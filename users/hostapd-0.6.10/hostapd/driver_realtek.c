/*
 * hostapd / Driver interaction with Realtek 802.11 driver
 * Copyright (c) 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include "includes.h"
#include "common.h"
#include <net/if.h>
#include <sys/ioctl.h>

#include <net80211/ieee80211.h>
#ifdef WME_NUM_AC
/* Assume this is built against BSD branch of realtek driver. */
#define REALTEK_BSD
#include <net80211/_ieee80211.h>
#endif /* WME_NUM_AC */
#include <net80211/ieee80211_crypto.h>
#include <net80211/ieee80211_ioctl.h>

#ifdef IEEE80211_IOCTL_SETWMMPARAMS
/* Assume this is built against realtek-ng */
#define REALTEK_NG
#endif /* IEEE80211_IOCTL_SETWMMPARAMS */

#include <net/if_arp.h>
#include "wireless_copy.h"
//#include <linux/wireless.h>

//#include <netpacket/packet.h>

#include "hostapd.h"
#include "driver.h"
#include "ieee802_1x.h"
#include "eloop.h"
#include "priv_netlink.h"
#include "sta_info.h"
#include "l2_packet/l2_packet.h"

#include "eapol_sm.h"
#include "wpa.h"
#include "radius/radius.h"
#include "ieee802_11.h"
#include "accounting.h"
#include "common.h"
#include "wps_hostapd.h"

#ifndef EAP_WPS
#define EAP_WPS
#endif

#include "./driver_realtek.h"

#ifdef CONFIG_WPS
#ifdef IEEE80211_IOCTL_FILTERFRAME
//#include <netpacket/packet.h>
#ifndef __LINUX_IF_PACKET_H
//#ifndef __NETPACKET_PACKET_H
#include <linux/if_packet.h>
//#include <netpacket/packet.h>
#endif

#ifndef ETH_P_80211_RAW
#define ETH_P_80211_RAW 0x0019
#endif
#endif /* IEEE80211_IOCTL_FILTERFRAME */
#endif /* CONFIG_WPS */

/*
 * Avoid conflicts with hostapd definitions by undefining couple of defines
 * from madwifi header files.
 */
#undef RSN_VERSION
#undef WPA_VERSION
#undef WPA_OUI_TYPE
#undef WME_OUI_TYPE

#if defined(INBAND_CTRL)
#ifndef __IOH_H
#include "../../inband_lib/ioh.h"
#endif
#define INBAND_INTF		"br0"
#define INBAND_SLAVE	("001234567899")
#define INBAND_IOCTL_TYPE	0x8899
#define INBAND_NETLINK_TYPE 0x9000
#define INBAND_DEBUG 0
#define INBAND_IOCTLPKT_DUMP //hex_dump
#undef STAND_ALONE
#define IWREQ_LEN 32
#define INBAND_IOCTLTYPE_LEN	4
#define INBAND_IOCTLHDR_LEN	6
#define INBAND_PENDING_START(data) data+INBAND_IOCTLHDR_LEN+IWREQ_LEN
#define INBAND_IOCTLRET_PTR(data) data+INBAND_IOCTLTYPE_LEN
#define IOH_HDR_LEN sizeof(struct ioh_header)
#else
#define STAND_ALONE
#endif

//#define HOST_LITTLE_ENDIAN 1 //mark_endian

#ifdef RTK_MBSSID

struct i802_bss {
	//void *ctx;       //back pointer to hapd per bss data
	struct realtek_driver_data *drv;
	struct i802_bss *next;
	int ifindex;
	char ifname[IFNAMSIZ + 1];
	struct l2_packet_data *sock_xmit;	/* raw packet xmit socket */
	//unsigned int beacon_set:1;
};

#endif


struct realtek_driver_data {
	//struct driver_ops ops;			/* base class */
	struct hostapd_data *hapd;		/* back pointer */

	char	iface[IFNAMSIZ + 1];
	int     ifindex;
	struct l2_packet_data *sock_xmit;	/* raw packet xmit socket */
#ifdef RTK_MBSSID
	int sock_recv;
#else
	struct l2_packet_data *sock_recv;	/* raw packet recv socket */
#endif
	int	ioctl_sock;			/* socket for ioctl() use */
	int	wext_sock;			/* socket for wireless events */
	int	we_version;
	u8	acct_mac[ETH_ALEN];
	struct hostap_sta_driver_data acct_data;
#if defined(INBAND_CTRL)
	struct ioh_class netlink_ioh_obj;
#endif

#ifdef RTK_MBSSID
	struct i802_bss first_bss;
	int if_indices[RTK_MAX_IF_INDEX];
	int num_if_indices;
#endif
};

struct rtk_hapd_config rtk_config;

const struct wpa_driver_ops wpa_driver_realtek_ops;

static void realtek_deinit(void *priv);
static int realtek_sta_deauth(void *priv, const u8 *addr, int reason_code);

#ifdef EAP_WPS
static int realtek_set_wps_beacon_ie(const char *ifname, void *priv, const u8 *iebuf, size_t iebuflen);
static int realtek_set_wps_probe_resp_ie(const char *ifname, void *priv, const u8 *iebuf, size_t iebuflen);
static int realtek_set_wps_assoc_resp_ie(const char *ifname, void *priv, const u8 *iebuf, size_t iebuflen);
static int realtek_start_receive_prob_req(void *priv);
#endif /* EAP_WPS */

#ifdef RTK_HAPD
static int realtek_driver_on(void *priv, int on);
static int realtek_hapd_config(void *priv);
#endif

static int realtek_read_priv_vap_cfg(const char *ifname,void *priv, struct rtk_hapd_config* config);
static int realtek_config_rate(int *rate_list, unsigned int *rate_config);
static int realtek_read_hapd_cfg(struct hostapd_data *hapd,void *priv, struct rtk_hapd_config* config);


#ifdef MODIFIED_BY_SONY
static int wext_set_key(void *priv, int alg,
						const u8 *addr, int key_idx,
						int set_tx, const u8 *seq, size_t seq_len,
						const u8 *key, size_t key_len);
#endif /* MODIFIED_BY_SONY */

#ifdef RTK_MBSSID
static int add_ifidx(struct realtek_driver_data *drv, int ifidx)
{
	int i;
	wpa_printf(MSG_DEBUG, "nl80211: Add own interface ifindex %d",
		   ifidx);
	for (i = 0; i < drv->num_if_indices; i++) {
		if (drv->if_indices[i] == 0) {
			drv->if_indices[i] = ifidx;
			return;
		}
	}

	return -1;
}

static int have_ifidx(struct realtek_driver_data *drv, int ifidx)
{
	int i;	

	for (i = 0; i < drv->num_if_indices; i++)
		if (drv->if_indices[i] == ifidx)
			return 1;
	return 0;
}


static struct hostapd_data *find_hapd_by_ifname(void *priv,const char *ifname)
{
	struct realtek_driver_data *drv = priv;
	struct hostapd_data *hapd = drv->hapd,*bss_hapd=hapd;
	struct hostapd_iface *iface = hapd->iface;
	struct hostapd_bss_config *conf;
	int j;


	for (j = 0; j < iface->num_bss; j++)
	{
		bss_hapd = iface->bss[j] ;
		conf = bss_hapd->conf;	
		if(os_strcmp(conf->iface, ifname) == 0) //find the entry	
			break;
		
	}
	
	return bss_hapd;
}

static struct hostapd_data *find_hapd_by_ifindex(void *priv,int ifindex)
{
	struct realtek_driver_data *drv = priv;
	struct hostapd_data *hapd = drv->hapd;
	struct i802_bss *tbss = &drv->first_bss,*bss;	
	
	
	if( drv->ifindex == ifindex) //if  it's for Root AP
		goto end;	

	while (tbss) {  //find the VAP
		
			bss = tbss->next;

			if(bss == NULL)
				break;

			if( bss->ifindex == ifindex ) //find the entry
			{
				hapd = (struct hostapd_data *)find_hapd_by_ifname(drv,bss->ifname);
				break;
			}
			tbss = tbss->next;
	}
end:
	return hapd;
}

static struct hostapd_data *find_hapd_by_sta(void *priv,u8 *sta_addr)
{
	struct realtek_driver_data *drv = priv;
	struct hostapd_data *hapd = drv->hapd;
	struct hostapd_iface *iface = hapd->iface;
	int j;
		
	for (j = 0; j < iface->num_bss; j++) {
		if (ap_get_sta(iface->bss[j], sta_addr)) {
			hapd = iface->bss[j];
			break;
		}
	}

	return hapd;
}

static int rtk_free_bss_by_ifname(void *priv, const char *ifname)
{
	struct realtek_driver_data *drv = priv;
	struct i802_bss *tbss = &drv->first_bss,*bss;

	while (tbss) {
		
			bss = tbss->next;
			if(bss == NULL)
				break;
			if(os_strcmp(bss->ifname, ifname) == 0) //find the entry
			{
				if (bss->sock_xmit != NULL)  //deinit socket
					l2_packet_deinit(bss->sock_xmit);
		
				tbss->next = bss->next;
				os_free(bss);
				break;
			}
			tbss = tbss->next  ;
	}
	return 0;
}

static struct l2_packet_data *rtk_find_l2sock_by_ifname(void *priv, const char *ifname)
{
	struct realtek_driver_data *drv = priv;
	struct i802_bss *tbss = &drv->first_bss,*bss;
	struct l2_packet_data *eapol_sock=drv->sock_xmit;
	
	if(os_strcmp(drv->iface,ifname)==0) //if  it's for Root AP
		goto end; 
	

	while (tbss) {  //find the VAP
		
			bss = tbss->next;

			if(bss == NULL)
				break;

			if(os_strcmp(bss->ifname, ifname) == 0) //find the entry
			{
				eapol_sock = bss->sock_xmit;
				break;
			}
			tbss = tbss->next;
	}

end:
	return eapol_sock;
}

static void handle_eapol(int sock, void *eloop_ctx, void *sock_ctx)
{
	struct realtek_driver_data *drv = eloop_ctx;
	struct hostapd_data *hapd = drv->hapd;
	struct sockaddr_ll lladdr;
	unsigned char buf[3000];
	int len,j;
	socklen_t fromlen = sizeof(lladdr);
	struct hostapd_iface *iface = hapd->iface;
	unsigned char *sa;
	len = recvfrom(sock, buf, sizeof(buf), 0,
		       (struct sockaddr *)&lladdr, &fromlen);
	
	if (len < 0) {
		perror("recv");
		return;
	}
	sa = (unsigned char *)lladdr.sll_addr;
	//if (have_ifidx(drv, lladdr.sll_ifindex))  //mark_mbssid , if br0 rcv??
	//{
		for (j = 0; j < iface->num_bss; j++) {			
			if (ap_get_sta(iface->bss[j], sa)) {
			hapd = iface->bss[j];
			break;
			}
		}	

		/*printf("handle_eapol if=%s  src_mac=%x:%x:%x:%x:%x:%x\n",hapd->conf->iface,
								sa[0],sa[1],sa[2],sa[3],sa[4],sa[5]);*/
		ieee802_1x_receive(hapd, sa, buf, len);	
	//}	
}
#endif

#ifdef INBAND_CTRL // HOST_LITTLE_ENDIAN
static void  rtk_cfg_to_bigEndian(struct rtk_hapd_config *config_ptr)
{	
	config_ptr->band = htonl(config_ptr->band);	
	config_ptr->channel = htonl(config_ptr->channel);	
	config_ptr->bcnint = htonl(config_ptr->bcnint);	
	config_ptr->dtimperiod = htonl(config_ptr->dtimperiod);	
	config_ptr->stanum = htonl(config_ptr->stanum);	
	config_ptr->rtsthres = htonl(config_ptr->rtsthres);	
	config_ptr->fragthres = htonl(config_ptr->fragthres);	
	config_ptr->oprates = htonl(config_ptr->oprates);	
	config_ptr->basicrates = htonl(config_ptr->basicrates);	
	config_ptr->preamble = htonl(config_ptr->preamble);	
	config_ptr->aclmode = htonl(config_ptr->aclmode);	
	config_ptr->aclnum = htonl(config_ptr->aclnum);	
	config_ptr->hiddenAP = htonl(config_ptr->hiddenAP);	
	config_ptr->qos_enable = htonl(config_ptr->qos_enable);	
	config_ptr->expired_time = htonl(config_ptr->expired_time);	
	config_ptr->block_relay = htonl(config_ptr->block_relay);	
	config_ptr->shortGI20M = htonl(config_ptr->shortGI20M);	
	config_ptr->shortGI40M = htonl(config_ptr->shortGI40M); 
	//Above are for Hostapd owned configurations //====================================================		
	config_ptr->phyBandSelect = htonl(config_ptr->phyBandSelect);	
	config_ptr->ther = htonl(config_ptr->ther);	
	config_ptr->swcrypto = htonl(config_ptr->swcrypto);	
	config_ptr->regdomain = htonl(config_ptr->regdomain);	
	config_ptr->autorate = htonl(config_ptr->autorate);	
	config_ptr->fixrate = htonl(config_ptr->fixrate);	
	config_ptr->disable_protection = htonl(config_ptr->disable_protection);	
	config_ptr->disable_olbc = htonl(config_ptr->disable_olbc);	
	config_ptr->deny_legacy = htonl(config_ptr->deny_legacy);	
	config_ptr->opmode = htonl(config_ptr->opmode);		
	config_ptr->vap_enable = htonl(config_ptr->vap_enable);	
	config_ptr->use40M = htonl(config_ptr->use40M);	
	config_ptr->_2ndchoffset = htonl(config_ptr->_2ndchoffset);	
	config_ptr->ampdu = htonl(config_ptr->ampdu);	
	config_ptr->coexist = htonl(config_ptr->coexist);	
	config_ptr->rssi_dump = htonl(config_ptr->rssi_dump);	
	config_ptr->mp_specific = htonl(config_ptr->mp_specific);	
	config_ptr->use_ext_pa = htonl(config_ptr->use_ext_pa);		
	config_ptr->macPhyMode = htonl(config_ptr->macPhyMode);
	//Below are for RTK private configurations 	                       	                       	                                            	
	}
#endif


static int
set80211priv(const char *ifname,struct realtek_driver_data *drv, int op, void *data, int len) 
{
	struct iwreq iwr;
        int do_inline = (len < IFNAMSIZ);

        /* Poorly thought out inteface -- certain ioctls MUST use
         * the non-inline method:
         */
        if (
            #ifdef IEEE80211_IOCTL_SET_APPIEBUF 
            op == IEEE80211_IOCTL_SET_APPIEBUF ||
            #endif
            #ifdef IEEE80211_IOCTL_FILTERFRAME
            op == IEEE80211_IOCTL_FILTERFRAME ||
            #endif
            0
            ) {
                do_inline = 0;
        }

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, ifname, IFNAMSIZ);
	
	if (do_inline) {
		memcpy(iwr.u.name, data, len);
	} else {
		/*
		 * Argument data MAY BE too big for inline transfer; setup a
		 * parameter block instead; the kernel will transfer
		 * the data for the driver.
		 */
		iwr.u.data.pointer = data;
		iwr.u.data.length = len;
	}

#ifdef STAND_ALONE
	if (ioctl(drv->ioctl_sock, op, &iwr) < 0) {
                {
                        int err = errno;
						perror("set80211priv ioctl failed");
                        wpa_printf(MSG_ERROR, "ioctl 0x%x failed errno=%d", 
                                op, err);
                }
		return -1;
	}
#endif
#if defined(INBAND_CTRL)
	INBAND_IOCTLPKT_DUMP(&iwr,sizeof(iwr));

	if ( inband_ioctl(op, &iwr) < 0) {
		perror("set80211priv ioctl failed");
		wpa_printf(MSG_ERROR, "ioctl 0x%x failed", op);
		return -1;
	}
#endif


	return 0;
}

static int
set80211param(const char *ifname,struct realtek_driver_data *drv, int op, int arg)
{
	struct iwreq iwr;

	memset(&iwr, 0, sizeof(iwr));	
	strncpy(iwr.ifr_name, ifname, IFNAMSIZ);  
#ifdef INBAND_CTRL //HOST_LITTLE_ENDIAN 
//  int need endian swap       
        op = htonl(op);	
	 arg = htonl(arg);
#endif	
	iwr.u.mode = op;
	memcpy(iwr.u.name+sizeof(__u32), &arg, sizeof(arg));

#ifdef STAND_ALONE
	if (ioctl(drv->ioctl_sock, IEEE80211_IOCTL_SETPARAM, &iwr) < 0) {
		perror("ioctl[IEEE80211_IOCTL_SETPARAM]");
		wpa_printf(MSG_DEBUG, "%s: Failed to set parameter (op %d "
			   "arg %d)", __func__, op, arg);
		return -1;
	}
#endif
#if defined(INBAND_CTRL)
	INBAND_IOCTLPKT_DUMP(&iwr,sizeof(iwr));
	if (inband_ioctl(IEEE80211_IOCTL_SETPARAM, &iwr) < 0) {
		perror("ioctl[IEEE80211_IOCTL_SETPARAM]");
		wpa_printf(MSG_DEBUG, "%s: Failed to set parameter (op %d "
			   "arg %d)", __func__, op, arg);
		return -1;
	}
#endif

	return 0;
}

static const char *
ether_sprintf(const u8 *addr)
{
	static char buf[sizeof(MACSTR)];

	if (addr != NULL)
		snprintf(buf, sizeof(buf), MACSTR, MAC2STR(addr));
	else
		snprintf(buf, sizeof(buf), MACSTR, 0,0,0,0,0,0);
	return buf;
}


static int
realtek_set_wpa(char *ifname,struct realtek_driver_data *drv, int wpa, int psk, int cipher)
{

	if(psk & WPA_KEY_MGMT_PSK)
		{//PSK mode, set PSK & cipher
			if (set80211param(ifname,drv, IEEE80211_PARAM_KEYMGTALGS, wpa)) 
				{
					wpa_printf(MSG_ERROR, "Unable to set key management algorithms");
					return -1;
				}					
			if (set80211param(ifname,drv, IEEE80211_PARAM_UCASTCIPHERS, cipher)) 
				{
					wpa_printf(MSG_ERROR, "Unable to set pairwise key ciphers");
					return -1;
				}
	
		}
	else
		{//Enterprise mode, Disable PSK & set cipher.	
			if (set80211param(ifname,drv, IEEE80211_PARAM_KEYMGTALGS, 0)) 
				{
					wpa_printf(MSG_ERROR, "Unable to set key management algorithms");
					return -1;
				}	
			if (set80211param(ifname,drv, IEEE80211_PARAM_UCASTCIPHERS, cipher)) 
				{
					wpa_printf(MSG_ERROR, "Unable to set pairwise key ciphers");
					return -1;
				}
	
		}
	
	if (set80211param(ifname,drv, IEEE80211_PARAM_WPA, wpa)) 
		{
			wpa_printf(MSG_ERROR, "Unable to set WPA");
			return -1;
		}

	return set80211param(ifname,drv, IEEE80211_PARAM_AUTHMODE, IEEE80211_AUTH_WPA);

}

/*
 * Configure WPA parameters.
 */
static int
realtek_configure_wpa(struct hostapd_data *hapd,struct realtek_driver_data *drv)
{	
	struct hostapd_bss_config *conf = hapd->conf;
	int v;

	//delete conf->wpa_group & conf->rsn_preauth related parts (like ralink)

	wpa_printf(MSG_DEBUG, "realtek_configure_wpa +++ wpa=0x%x, psk=0x%x, cipher=0x%x", 
			conf->wpa, conf->wpa_key_mgmt, conf->wpa_pairwise);

	v = 0;
	if (conf->wpa_pairwise & WPA_CIPHER_CCMP)
	v |= 1<<IEEE80211_CIPHER_AES_CCM;
	if (conf->wpa_pairwise & WPA_CIPHER_TKIP)
	v |= 1<<IEEE80211_CIPHER_TKIP;
	if (conf->wpa_pairwise & WPA_CIPHER_NONE)
	v |= 1<<IEEE80211_CIPHER_NONE;
					
	if((conf->wpa == 0) || (conf->wpa  > (HOSTAPD_WPA_VERSION_WPA|HOSTAPD_WPA_VERSION_WPA2)))
		return -1;
	else
		return realtek_set_wpa(conf->iface,drv, conf->wpa , conf->wpa_key_mgmt, v); 
		
}

static int
realtek_set_iface_hwMac(const char *ifname,void *priv, char *addr)
{
	struct realtek_driver_data *drv = priv;
	struct ifreq ifreq;

	if (drv->ioctl_sock < 0)
		return -1;

	os_memset(&ifreq, 0, sizeof(ifreq));	
	os_strlcpy(ifreq.ifr_name, ifname, IFNAMSIZ);

	memcpy(ifreq.ifr_hwaddr.sa_data, addr, ETH_ALEN);
	ifreq.ifr_hwaddr.sa_family = ARPHRD_ETHER;

#if defined(INBAND_CTRL)
	INBAND_IOCTLPKT_DUMP(&ifreq,sizeof(ifreq));
	if (inband_ioctl(SIOCSIFHWADDR, &ifreq) < 0) {
		perror("inband_ioctl[SIOCSIFHWADDR]");
		return -1;
	}
#else
	if (ioctl(drv->ioctl_sock, SIOCSIFHWADDR, &ifreq) != 0) {
		perror("ioctl[SIOCSIFHWADDR]");
		return -1;
	}
#endif


	return 0;
}

static int
realtek_set_iface_flags(const char *ifname,void *priv, int dev_up)
{
	struct realtek_driver_data *drv = priv;
	struct ifreq ifr;

	wpa_printf(MSG_DEBUG, "realtek_set_iface_flags +++  dev_up = %d", dev_up);

	if (drv->ioctl_sock < 0)
		return -1;

	if (dev_up) {
		memset(&ifr, 0, sizeof(ifr));
		snprintf(ifr.ifr_name, IFNAMSIZ, "%s", ifname);
		ifr.ifr_mtu = HOSTAPD_MTU;
#ifdef STAND_ALONE
		if (ioctl(drv->ioctl_sock, SIOCSIFMTU, &ifr) != 0) {
			perror("ioctl[SIOCSIFMTU]");
			printf("Setting MTU failed - trying to survive with "
			       		"current value\n");
		}
#endif
#if defined(INBAND_CTRL)
		INBAND_IOCTLPKT_DUMP(&ifr,sizeof(ifr));
		if (inband_ioctl(SIOCSIFMTU, &ifr) < 0) {
			perror("inband_ioctl[SIOCSIFMTU]");
			printf("Setting MTU failed - trying to survive with "
				   "current value\n");
		}
#endif
	}
	
	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, IFNAMSIZ, "%s", ifname);
#ifdef STAND_ALONE
	if (ioctl(drv->ioctl_sock, SIOCGIFFLAGS, &ifr) != 0) {
		perror("ioctl[SIOCGIFFLAGS]");
		return -1;
	}
#endif
#if defined(INBAND_CTRL)
	INBAND_IOCTLPKT_DUMP(&ifr,sizeof(ifr));
	if (inband_ioctl(SIOCGIFFLAGS, &ifr) < 0) {
		perror("inband_ioctl[SIOCGIFFLAGS]");
		printf("inband_ioctl[SIOCGIFFLAGS] fail");
		return -1;
	}
#endif
	
	if (dev_up)
		ifr.ifr_flags |= IFF_UP;
	else
		ifr.ifr_flags &= ~IFF_UP;
#ifdef STAND_ALONE
	if (ioctl(drv->ioctl_sock, SIOCSIFFLAGS, &ifr) != 0) {
		perror("ioctl[SIOCSIFFLAGS]");
		return -1;
	}
#endif
#if defined(INBAND_CTRL)
	INBAND_IOCTLPKT_DUMP(&ifr,sizeof(ifr));
	if (inband_ioctl(SIOCSIFFLAGS, &ifr) < 0) {
		perror("inband_ioctl[SIOCSIFFLAGS]");
		printf("inband_ioctl[SIOCGIFFLAGS] fail");
		return -1;
	}
#endif

	wpa_printf(MSG_DEBUG, "realtek_set_iface_flags ---");
	return 0;
} 

static int realtek_set_ieee8021x(const char *ifname, void *priv, int enabled)
{
	struct realtek_driver_data *drv = priv;
	struct hostapd_data *hapd = drv->hapd;
	struct hostapd_bss_config *conf = hapd->conf;
	struct hostapd_wep_keys wep = conf->ssid.wep;

#ifdef RTK_MBSSID //find the real VAP
	hapd =(struct hostapd_data *) find_hapd_by_ifname(priv,ifname);
	conf = hapd->conf;
	wep = conf->ssid.wep;
#endif

	wpa_printf(MSG_DEBUG, "realtek_set_ieee8021x+++ \n"
		   "enabled=%d, conf->auth_algs=%d, wep.keys_set=%d, conf->ieee802_1x=%d"
			,enabled, conf->auth_algs, wep.keys_set, conf->ieee802_1x);

	if(set80211param(ifname,priv, IEEE80211_PARAM_UCASTKEYLEN, conf->individual_wep_key_len))
		return -1;

	if (!enabled) {
		wpa_printf(MSG_DEBUG, "set WEP");
    	/* Set interface up flags after setting authentication modes,
    	   	    done atlast in realtek_commit() */   	   	    	
		if(conf->auth_algs==1 && wep.keys_set==0)
			return set80211param(ifname,priv, IEEE80211_PARAM_AUTHMODE, IEEE80211_AUTH_OPEN);
		else if(conf->auth_algs==1 && wep.keys_set==1){
			return set80211param(ifname,priv, IEEE80211_PARAM_AUTHMODE, IEEE80211_AUTH_OPEN);
			/*Fix for open wep  when run using hostapd*/
			//return set80211param(priv, IEEE80211_PARAM_PRIVACY, 1);			
		}		
		else if(conf->auth_algs==2)
			return set80211param(ifname,priv, IEEE80211_PARAM_AUTHMODE, IEEE80211_AUTH_SHARED);
		else if(conf->auth_algs==3)
			return set80211param(ifname,priv, IEEE80211_PARAM_AUTHMODE, IEEE80211_AUTH_AUTO);
		else if(conf->auth_algs==8) //RTK_HAPD, add auth_algs=BIT(3) as value of none authentication. 
			return set80211param(ifname,priv, IEEE80211_PARAM_AUTHMODE, IEEE80211_AUTH_NONE);
	}
	
	if (!conf->wpa && !conf->ieee802_1x) {
		hostapd_logger(hapd, NULL, HOSTAPD_MODULE_DRIVER,
			HOSTAPD_LEVEL_WARNING, "No 802.1X or WPA enabled!");
		return -1;
	}

	if((conf->wpa == 0) && (conf->ieee802_1x))
	{
		if(set80211param(ifname,priv, IEEE80211_PARAM_AUTHMODE, IEEE80211_AUTH_8021X))
			return -1;
	}

	if (conf->wpa) 
	{
		return realtek_configure_wpa(hapd,drv);
	}

	return 0;
}

static int
realtek_set_privacy(const char *ifname, void *priv, int enabled)
{
	struct realtek_driver_data *drv = priv;
	struct hostapd_data *hapd = drv->hapd;

	wpa_printf(MSG_DEBUG, "realtek_set_privacy +++");

	return set80211param(ifname,priv, IEEE80211_PARAM_PRIVACY, enabled);
}

static int
realtek_set_sta_authorized(void *priv, const u8 *addr, int authorized)
{
	struct realtek_driver_data *drv = priv;
	struct hostapd_data *hapd = drv->hapd;
	struct hostapd_bss_config *conf = hapd->conf;
	struct ieee80211req_mlme mlme;
	int ret;

#ifdef RTK_MBSSID
	hapd = (struct hostapd_data *)find_hapd_by_sta(priv,addr);
	conf = hapd->conf;	
#endif

	if (authorized)
		mlme.im_op = IEEE80211_MLME_AUTHORIZE;
	else
		mlme.im_op = IEEE80211_MLME_UNAUTHORIZE;
	mlme.im_reason = 0;
	memcpy(mlme.im_macaddr, addr, IEEE80211_ADDR_LEN);
	ret = set80211priv(conf->iface,priv, IEEE80211_IOCTL_SETMLME, &mlme,
			   sizeof(mlme));
	if (ret < 0) {
		wpa_printf(MSG_DEBUG, "%s: Failed to %sauthorize STA " MACSTR,
			   __func__, authorized ? "" : "un", MAC2STR(addr));
	}

	return ret;
}

static int
realtek_sta_set_flags(void *priv, const u8 *addr, int flags_or, int flags_and)
{

	wpa_printf(MSG_DEBUG, "realtek_sta_set_flags +++");
	
	/* For now, only support setting Authorized flag */
	if (flags_or & WLAN_STA_AUTHORIZED)
		return realtek_set_sta_authorized(priv, addr, 1);
	if (!(flags_and & WLAN_STA_AUTHORIZED))
		return realtek_set_sta_authorized(priv, addr, 0);
	return 0;
}

static int
realtek_del_key(const char *ifname,void *priv, const u8 *addr, int key_idx)
{
	struct realtek_driver_data *drv = priv;
	struct hostapd_data *hapd = drv->hapd;
	struct ieee80211req_del_key wk;
	int ret;

	wpa_printf(MSG_DEBUG, "realtek_del_key +++");

	memset(&wk, 0, sizeof(wk));
	if (addr != NULL) {
		memcpy(wk.idk_macaddr, addr, IEEE80211_ADDR_LEN);
		wk.idk_keyix = (u8) IEEE80211_KEYIX_NONE;
	} else {
		wk.idk_keyix = key_idx;
	}

	ret = set80211priv(ifname ,priv, IEEE80211_IOCTL_DELKEY, &wk, sizeof(wk)); 
	if (ret < 0) {
		wpa_printf(MSG_DEBUG, "%s: Failed to delete key (addr %s"
			   " key_idx %d)", __func__, ether_sprintf(addr),
			   key_idx);
	}

	wpa_printf(MSG_DEBUG, "realtek_del_key ---");
	return ret;
}

static int
realtek_set_key(const char *ifname, void *priv, const char *alg,
		const u8 *addr, int key_idx,
		const u8 *key, size_t key_len, int txkey)
{
	struct realtek_driver_data *drv = priv;
	struct hostapd_data *hapd = drv->hapd;
	struct hostapd_bss_config *conf = hapd->conf;
	struct hostapd_wep_keys wep = conf->ssid.wep;
	struct ieee80211req_key wk;
	u_int8_t cipher;
	int ret;

#ifdef RTK_MBSSID //find the real VAP
	hapd = (struct hostapd_data *)find_hapd_by_ifname(priv,ifname);
	conf = hapd->conf;
	wep = conf->ssid.wep;
#endif

	wpa_printf(MSG_DEBUG, "realtek_set_key +++");
	wpa_printf(MSG_DEBUG, "alg = %s\n, addr = %s\n, key_index = %d", alg,  ether_sprintf(addr), key_idx);

	if (strcmp(alg, "none") == 0)
		return realtek_del_key(ifname,priv, addr, key_idx); 

	if ((strcmp(alg, "WEP") == 0) && ((wep.keys_set >= 1) && (wep.keys_set <= 4)))
		cipher = IEEE80211_CIPHER_WEP;
	else if (strcmp(alg, "TKIP") == 0)
		cipher = IEEE80211_CIPHER_TKIP;
	else if (strcmp(alg, "CCMP") == 0)
		cipher = IEEE80211_CIPHER_AES_CCM;
	else {
		printf("%s: unknown/unsupported algorithm %s\n",
			__func__, alg);
		return -1;
	}

	if (key_len > sizeof(wk.ik_keydata)) {
		printf("%s: key length %lu too big\n", __func__,
		       (unsigned long) key_len);
		return -3;
	}

	memset(&wk, 0, sizeof(wk));
	wk.ik_type = cipher;
	wk.ik_flags = IEEE80211_KEY_RECV | IEEE80211_KEY_XMIT;
	if (addr == NULL) {
		memset(wk.ik_macaddr, 0xff, IEEE80211_ADDR_LEN);
		wk.ik_keyix = key_idx;
		wk.ik_flags |= IEEE80211_KEY_DEFAULT;
	} else {
		memcpy(wk.ik_macaddr, addr, IEEE80211_ADDR_LEN);
		wk.ik_keyix = key_idx;
		//wk.ik_keyix = IEEE80211_KEYIX_NONE;
	}
	wk.ik_keylen = key_len;
	memcpy(wk.ik_keydata, key, key_len);

#ifdef INBAND_CTRL //HOST_LITTLE_ENDIAN 	
	// ik_keyix int16 need endian-swap 	
	wk.ik_keyix = htons(wk.ik_keyix);	
	wk.ik_keylen = htonl(key_len);
#endif
	ret = set80211priv(ifname,priv, IEEE80211_IOCTL_SETKEY, &wk, sizeof(wk));
	if (ret < 0) {
		wpa_printf(MSG_DEBUG, "%s: Failed to set key (addr %s"
			   " key_idx %d alg '%s' key_len %lu txkey %d)",
			   __func__, ether_sprintf(wk.ik_macaddr), key_idx,
			   alg, (unsigned long) key_len, txkey);
	}

	wpa_printf(MSG_DEBUG, "realtek_set_key ---");
	return ret;
}


static int
realtek_get_seqnum(const char *ifname, void *priv, const u8 *addr, int idx,
		   u8 *seq)
{
//do nothing like broadcom & ralink, even madwifi did not call back for hosapd.

	wpa_printf(MSG_DEBUG, "realtek_get_seqnum +++");
	wpa_printf(MSG_DEBUG, "realtek_get_seqnum ---");

	return 0;
}


static int 
realtek_flush(void *priv)
{
//do nothing like broadcom & ralink
	wpa_printf(MSG_DEBUG, "realtek_flush +++");
	wpa_printf(MSG_DEBUG, "realtek_flush ---");
	return 0;
}


static int
realtek_read_sta_driver_data(void *priv, struct hostap_sta_driver_data *data,
			     const u8 *addr)
{
#if 0 //not use now
	struct realtek_driver_data *drv = priv;
	wpa_printf(MSG_DEBUG, "realtek_read_sta_driver_data +++");

#ifdef REALTEK_BSD
	struct ieee80211req_sta_stats stats;

	memset(data, 0, sizeof(*data));

	/*
	 * Fetch statistics for station from the system.
	 */
	memset(&stats, 0, sizeof(stats));
	memcpy(stats.is_u.macaddr, addr, IEEE80211_ADDR_LEN);
	if (set80211priv(drv,
#ifdef REALTEK_NG
			 IEEE80211_IOCTL_STA_STATS,
#else /* REALTEK_NG */
			 IEEE80211_IOCTL_GETSTASTATS,
#endif /* REALTEK_NG */
			 &stats, sizeof(stats))) {
		wpa_printf(MSG_DEBUG, "%s: Failed to fetch STA stats (addr "
			   MACSTR ")", __func__, MAC2STR(addr));
		if (memcmp(addr, drv->acct_mac, ETH_ALEN) == 0) {
			memcpy(data, &drv->acct_data, sizeof(*data));
			return 0;
		}

		printf("Failed to get station stats information element.\n");
		return -1;
	}
#ifdef INBAND_CTRL //HOST_LITTLE_ENDIAN 	
	stats.is_stats.ns_rx_data = ntohl(stats.is_stats.ns_rx_data); 	
	stats.is_stats.ns_rx_bytes = ntohl(stats.is_stats.ns_rx_bytes); 	
	stats.is_stats.ns_tx_data = ntohl(stats.is_stats.ns_tx_data); 	
	stats.is_stats.ns_tx_bytes = ntohl(stats.is_stats.ns_tx_bytes);
#endif
	data->rx_packets = stats.is_stats.ns_rx_data;
	data->rx_bytes = stats.is_stats.ns_rx_bytes;
	data->tx_packets = stats.is_stats.ns_tx_data;
	data->tx_bytes = stats.is_stats.ns_tx_bytes;
	return 0;

#else /* REALTEK_BSD */

	char buf[1024], line[128], *pos;
	FILE *f;
	unsigned long val;

	memset(data, 0, sizeof(*data));
	snprintf(buf, sizeof(buf), "/proc/net/realtek/%s/" MACSTR,
		 drv->iface, MAC2STR(addr));

	f = fopen(buf, "r");
	if (!f) {
		if (memcmp(addr, drv->acct_mac, ETH_ALEN) != 0)
			return -1;
		memcpy(data, &drv->acct_data, sizeof(*data));
		return 0;
	}
	/* Need to read proc file with in one piece, so use large enough
	 * buffer. */
	setbuffer(f, buf, sizeof(buf));

	while (fgets(line, sizeof(line), f)) {
		pos = strchr(line, '=');
		if (!pos)
			continue;
		*pos++ = '\0';
		val = strtoul(pos, NULL, 10);
		if (strcmp(line, "rx_packets") == 0)
			data->rx_packets = val;
		else if (strcmp(line, "tx_packets") == 0)
			data->tx_packets = val;
		else if (strcmp(line, "rx_bytes") == 0)
			data->rx_bytes = val;
		else if (strcmp(line, "tx_bytes") == 0)
			data->tx_bytes = val;
	}

	fclose(f);

	return 0;
#endif /* REALTEK_BSD */
#endif
return 0;
}


static int
realtek_sta_clear_stats(void *priv, const u8 *addr)
{
#if 0 //not support now
#if defined(REALTEK_BSD) && defined(IEEE80211_MLME_CLEAR_STATS)
	struct realtek_driver_data *drv = priv;
	struct hostapd_data *hapd = drv->hapd;
	struct ieee80211req_mlme mlme;
	int ret;

	wpa_printf(MSG_DEBUG, "realtek_sta_clear_stats (BSD) +++");

	mlme.im_op = IEEE80211_MLME_CLEAR_STATS;
	memcpy(mlme.im_macaddr, addr, IEEE80211_ADDR_LEN);
	ret = set80211priv(priv, IEEE80211_IOCTL_SETMLME, &mlme,
			   sizeof(mlme));
	if (ret < 0) {
		wpa_printf(MSG_DEBUG, "%s: Failed to clear STA stats (addr "
			   MACSTR ")", __func__, MAC2STR(addr));
	}

	return ret;
#else /* REALTEK_BSD && IEEE80211_MLME_CLEAR_STATS */
	wpa_printf(MSG_DEBUG, "realtek_sta_clear_stats +++");
	return 0; /* FIX */
#endif /* REALTEK_BSD && IEEE80211_MLME_CLEAR_STATS */
#endif
return 0;
}


static int
realtek_set_opt_ie(const char *ifname, void *priv, const u8 *ie, size_t ie_len)
{
//do nothing like broadcom & ralink & madwifi

	wpa_printf(MSG_DEBUG, "realtek_set_opt_ie +++");
	wpa_printf(MSG_DEBUG, "realtek_set_opt_ie ---");
	return 0;
}

static int
realtek_is_ifup(const char *ifname,struct realtek_driver_data *drv)
{
	struct ifreq ifr;

	if (drv->ioctl_sock < 0)
		return 0;

	memset(&ifr, 0, sizeof(ifr));	
	snprintf(ifr.ifr_name, IFNAMSIZ, "%s", ifname);
#ifdef STAND_ALONE
	if (ioctl(drv->ioctl_sock, SIOCGIFFLAGS, &ifr) != 0) {
		perror("ioctl[SIOCGIFFLAGS]");
		return 0;
	}
#endif
#if defined(INBAND_CTRL)
	INBAND_IOCTLPKT_DUMP(&ifr,sizeof(ifr));
	if (inband_ioctl(SIOCGIFFLAGS, &ifr) < 0) {
		perror("ioctl[SIOCGIFFLAGS]");
		return 0;
	}
#endif

	return ((ifr.ifr_flags & IFF_UP) == IFF_UP);
}

static int
realtek_sta_deauth(void *priv, const u8 *addr, int reason_code)
{
	struct realtek_driver_data *drv = priv;
	struct hostapd_data *hapd = drv->hapd;
	struct hostapd_bss_config *conf = hapd->conf;
	struct ieee80211req_mlme mlme;
	int ret;

#ifdef RTK_MBSSID //find the sta  belong to which MBSSID
	hapd = (struct hostapd_data *)find_hapd_by_sta(priv,addr);
	conf = hapd->conf;	
#endif

	wpa_printf(MSG_DEBUG, "realtek_sta_deauth +++");

	mlme.im_op = IEEE80211_MLME_DEAUTH;
	mlme.im_reason = reason_code;
	memcpy(mlme.im_macaddr, addr, IEEE80211_ADDR_LEN);
	if (!realtek_is_ifup(conf->iface,drv)) {
		return EINVAL;
	}
#ifdef INBAND_CTRL //HOST_LITTLE_ENDIAN 
	//im_reason need swap 	
	mlme.im_reason = htons(mlme.im_reason);
#endif
	ret = set80211priv(conf->iface,priv, IEEE80211_IOCTL_SETMLME, &mlme, sizeof(mlme));
	if (ret < 0) {
		wpa_printf(MSG_DEBUG, "%s: Failed to deauth STA (addr " MACSTR
			   " reason %d)",
			   __func__, MAC2STR(addr), reason_code);
	}

	return ret;
}

static int
realtek_sta_disassoc(void *priv, const u8 *addr, int reason_code)
{
	struct realtek_driver_data *drv = priv;
	struct hostapd_data *hapd = drv->hapd;
	struct hostapd_bss_config *conf = hapd->conf;
	struct ieee80211req_mlme mlme;
	int ret;

#ifdef RTK_MBSSID //find the sta  belong to which MBSSID
	hapd = (struct hostapd_data *)find_hapd_by_sta(priv,addr);
	conf = hapd->conf;	
#endif

	wpa_printf(MSG_DEBUG, "realtek_sta_disassoc +++");

	mlme.im_op = IEEE80211_MLME_DISASSOC;
	mlme.im_reason = reason_code;
	memcpy(mlme.im_macaddr, addr, IEEE80211_ADDR_LEN);
#ifdef INBAND_CTRL //HOST_LITTLE_ENDIAN 
	//im_reason need swap 	
	mlme.im_reason = htons(mlme.im_reason);
#endif		
	ret = set80211priv(conf->iface,priv, IEEE80211_IOCTL_SETMLME, &mlme, sizeof(mlme));
	if (ret < 0) {
		wpa_printf(MSG_DEBUG, "%s: Failed to disassoc STA (addr "
			   MACSTR " reason %d)",
			   __func__, MAC2STR(addr), reason_code);
	}

	return ret;
}

#ifdef RTK_HAPD
//Announce driver to remove sta list for IAPP function
static int
realtek_sta_remove(void *priv, const u8 *addr)
{
	struct realtek_driver_data *drv = priv;
	struct hostapd_data *hapd = drv->hapd;
	struct hostapd_bss_config *conf = hapd->conf;
	unsigned char para[32];
	struct iwreq wrq;

#ifdef RTK_MBSSID //find the sta  belong to which MBSSID
	hapd = (struct hostapd_data *)find_hapd_by_sta(priv,addr);
	conf = hapd->conf;	
#endif
	wpa_printf(MSG_DEBUG, "realtek_sta_remove +++");
	memset(para, 0, 32);
	sprintf(para, "%02x%02x%02x%02x%02x%02x", addr[0], addr[1],
	addr[2], addr[3], addr[4], addr[5]);
	wrq.u.data.pointer = para;
	wrq.u.data.length = strlen(para);
	strncpy(wrq.ifr_name, conf->iface, IFNAMSIZ);
#ifdef STAND_ALONE
	ioctl(drv->ioctl_sock, RTL8192CD_IOCTL_DEL_STA, &wrq);
#endif
#ifdef INBAND_CTRL
	inband_ioctl(RTL8192CD_IOCTL_DEL_STA, &wrq);
#endif

	wpa_printf(MSG_DEBUG, "realtek_sta_remove ---");
	
	return 0;
}
#endif

static int
realtek_del_sta(struct hostapd_data *hapd,struct realtek_driver_data *drv, u8 addr[IEEE80211_ADDR_LEN])
{
	struct sta_info *sta;

	hostapd_logger(hapd, addr, HOSTAPD_MODULE_IEEE80211,
		HOSTAPD_LEVEL_INFO, "disassociated");

	sta = ap_get_sta(hapd, addr);
	if (sta != NULL) {
		sta->flags &= ~(WLAN_STA_AUTH | WLAN_STA_ASSOC);
		wpa_auth_sm_event(sta->wpa_sm, WPA_DISASSOC);
		sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_USER_REQUEST;
		ieee802_1x_notify_port_enabled(sta->eapol_sm, 0);
		ap_free_sta(hapd, sta);
	}
	return 0;
}

static int
realtek_process_wpa_ie(struct hostapd_data *hapd,struct realtek_driver_data *drv, struct sta_info *sta)
{	
	struct ieee80211req_wpaie ie;
	int ielen, res;
	u8 *iebuf = NULL;
	struct hostapd_bss_config *conf = hapd->conf;

	wpa_printf(MSG_DEBUG, "realtek_process_wpa_ie +++");

	/*
	 * Fetch negotiated WPA/RSN parameters from the system.
	 */
	memset(&ie, 0, sizeof(ie));
	memcpy(ie.wpa_macaddr, sta->addr, IEEE80211_ADDR_LEN);
	
	if (set80211priv(conf->iface,drv, HAPD_IOCTL_GETWPAIE, &ie, sizeof(ie))) 
	{
		wpa_printf(MSG_ERROR, "%s: Failed to get WPA/RSN IE", __func__);
		printf("Failed to get WPA/RSN information element.\n");
		return -1;
	}

	//wpa_printf(MSG_DEBUG, "get wpa_ie = 0x%02x, 0x%02x, 0x%02x", ie.wpa_ie[0], ie.wpa_ie[1], ie.wpa_ie[2]);
	//wpa_printf(MSG_DEBUG, "get wps_ie = 0x%02x, 0x%02x, 0x%02x", ie.wps_ie[0], ie.wps_ie[1], ie.wps_ie[2]);

	do {
		iebuf = 0; ielen = 0;
		if (conf->wpa & HOSTAPD_WPA_VERSION_WPA) {
			iebuf = ie.wpa_ie; ielen = 0;
			if ((iebuf[0] == WLAN_EID_VENDOR_SPECIFIC) && iebuf[1]) {
				wpa_printf(MSG_DEBUG, "get wpa_ie");
				ielen = iebuf[1];
				break;
			}
		}
		if (conf->wpa & HOSTAPD_WPA_VERSION_WPA2) {
			iebuf = ie.rsn_ie; ielen = 0;
			if ((iebuf[0] == WLAN_EID_RSN) && iebuf[1]) {
				wpa_printf(MSG_DEBUG, "get rsn_ie");
				ielen = iebuf[1];
				break;
			}
		}
	} while (0);

	wpabuf_free(sta->wps_ie);
	if ((ie.wps_ie[0] == WLAN_EID_VENDOR_SPECIFIC) && ie.wps_ie[1]) 
	{
		//Test rtl8192su usb-dongle wps_pbc/wps_pin ok via here.
		 wpa_printf(MSG_DEBUG, "get wps_ie");
		 ielen = ie.wps_ie[1] + 2;
		 
		 sta->wps_ie = wpabuf_alloc(0);
		 sta->wps_ie->size = ielen;
		 sta->wps_ie->used = ielen;
		 sta->wps_ie->ext_data = os_malloc(ielen);
		 memcpy(sta->wps_ie->ext_data, ie.wps_ie, ielen);

		 sta->flags |= WLAN_STA_WPS;
		 return 0;
	}
	else
	{		
		sta->wps_ie = wpabuf_alloc(0);
		sta->wps_ie->used = 0;
		sta->wps_ie->ext_data = NULL;

		sta->flags &= ~WLAN_STA_WPS;

		
		if((ielen == 0) && (conf->wps_state)){
			wpa_printf(MSG_DEBUG, "STA did not include WPA/RSN/WPS IE in (Re)Association Request "
				"but WPS enabled at hostapd conf file - possible WPS use");
			sta->flags |= WLAN_STA_MAYBE_WPS;	// Test dwa140 and wn111 wps_pbc/wps_pin ok because of here!!!! --zj
		}
		else{
			sta->flags &= ~WLAN_STA_MAYBE_WPS;
		}
	}

	if(ielen != 0) //_For WPS, neglect wpa_ie??
	{
		if (sta->wpa_sm == NULL)
		sta->wpa_sm = wpa_auth_sta_init(hapd->wpa_auth, sta->addr);
	
		if (sta->wpa_sm == NULL) 
		{
			wpa_printf(MSG_ERROR, "Failed to initialize WPA state machine");
			return -1;
		}
	
		ielen += 2;
		res = wpa_validate_wpa_ie(hapd->wpa_auth, sta->wpa_sm, iebuf, ielen, NULL, 0);

		if (res != WPA_IE_OK) 
		{
			wpa_printf(MSG_ERROR, "WPA/RSN information element rejected? (res %u)", res);
			return -1;	
		}
		else
			wpa_printf(MSG_DEBUG, "WPA_IE_OK");
	}
	

	return 0;
}

static int
realtek_issue_asocrsp(char *ifname,struct realtek_driver_data *drv, struct sta_info *sta)
{
    int retVal = 0;
	struct iwreq iwr;
	DOT11_ASSOCIATION_RSP 	Association_Rsp;

	wpa_printf(MSG_DEBUG, "realtek_issue_asocrsp +++");

	strncpy(iwr.ifr_name, ifname, IFNAMSIZ); 

	iwr.u.data.pointer = (caddr_t)&Association_Rsp;
	iwr.u.data.length = sizeof(DOT11_ASSOCIATION_RSP);

	Association_Rsp.EventId = DOT11_EVENT_ASSOCIATION_RSP;
	
	//?? Re-Association case??
	//Association_Rsp.EventId = DOT11_EVENT_REASSOCIATION_RSP;
	
	Association_Rsp.IsMoreEvent = FALSE;
	Association_Rsp.Status = _STATS_SUCCESSFUL_;
	memcpy(&Association_Rsp.MACAddr, sta->addr, IEEE80211_ADDR_LEN);

#ifdef STAND_ALONE
	if(ioctl(drv->ioctl_sock, SIOCGIWIND, &iwr) < 0)
    	retVal = -1;
    else
        retVal = 0;
#endif
#if defined(INBAND_CTRL)
	INBAND_IOCTLPKT_DUMP(&iwr,sizeof(iwr));
	if (inband_ioctl(SIOCGIWIND, &iwr) < 0)
		retVal = -1;
	else
		retVal = 0;
#endif
    return retVal;

}


static int
realtek_new_sta(struct hostapd_data *hapd,struct realtek_driver_data *drv, u8 addr[IEEE80211_ADDR_LEN])
{
	struct hostapd_bss_config *conf = hapd->conf;
	struct sta_info *sta;
	int new_assoc;

	hostapd_logger(hapd, addr, HOSTAPD_MODULE_IEEE80211,
		HOSTAPD_LEVEL_INFO, "associated");

	sta = ap_get_sta(hapd, addr);
	if (sta) {
		accounting_sta_stop(hapd, sta);
	} else {
		//printf("realtek_new_sta hapd=%s \n",conf->iface);
		sta = ap_sta_add(hapd, addr);
		if (sta == NULL)
			return -1;
	}
	//mark_mbssid , acct_mac ? per VAP?
	if (memcmp(addr, drv->acct_mac, ETH_ALEN) == 0) {
		/* Cached accounting data is not valid anymore. */
		memset(drv->acct_mac, 0, ETH_ALEN);
		memset(&drv->acct_data, 0, sizeof(drv->acct_data));
	}
	accounting_sta_get_id(hapd, sta);

	if (hapd->conf->wpa || hapd->conf->wps_state) {
		if (realtek_process_wpa_ie(hapd,drv, sta)) 
			return -1;
		if((sta->wps_ie->used == 0) && ((conf->wpa_key_mgmt & WPA_KEY_MGMT_PSK) == 0) && (conf->ieee802_1x != 0))
			{//do issue Association response just for RADIUS authentication.
				if(realtek_issue_asocrsp(hapd->conf->iface,drv, sta))
					return -1;
			}
	}
	else
	{
		if(conf->ieee802_1x != 0)
			if(realtek_issue_asocrsp(hapd->conf->iface,drv, sta))
					return -1;
	}

	/*
	 * Now that the internal station state is setup
	 * kick the authenticator into action.
	 */

	wpa_printf(MSG_DEBUG, "Try to kick the authenticator into action");
	
	new_assoc = (sta->flags & WLAN_STA_ASSOC) == 0;
	sta->flags |= WLAN_STA_AUTH | WLAN_STA_ASSOC;
	wpa_auth_sm_event(sta->wpa_sm, WPA_ASSOC);
	hostapd_new_assoc_sta(hapd, sta, !new_assoc);
	ieee802_1x_notify_port_enabled(sta->eapol_sm, 1);
	return 0;
}



int realtek_set_wds(void *priv, struct rtk_wds_config wds)
{
	struct realtek_driver_data *drv = priv;
	struct hostapd_data *hapd = drv->hapd;
	struct hostapd_bss_config *conf = hapd->conf; 
	struct iwreq iwr;
	int op = 0;

	wpa_printf(MSG_DEBUG, "realtek_set_wds +++");

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, drv->iface, IFNAMSIZ);

	iwr.u.data.pointer = &wds;
	iwr.u.data.length = sizeof(struct rtk_wds_config);

	if(wds.wdsEnabled)
		op = IEEE80211_IOCTL_WDSADDMAC;
	else
		op = IEEE80211_IOCTL_WDSDELMAC;
#ifdef STAND_ALONE
	if (ioctl(drv->ioctl_sock, op, &iwr) < 0) 
	{
                {
                        int err = errno;
						perror("set WDS ioctl failed");
                        wpa_printf(MSG_ERROR, "ioctl 0x%x failed errno=%d", 
                                op, err);
                }
		return -1;
	}
#endif
#if defined(INBAND_CTRL)
	INBAND_IOCTLPKT_DUMP(&iwr,sizeof(iwr));
	if (inband_ioctl(op, &iwr) < 0)
	{
				{
						perror("set80211priv ioctl failed");
						wpa_printf(MSG_ERROR, "ioctl 0x%x failed", op);
				}
		return -1;
	}
#endif

	
	return 0;
}


static void
realtek_wireless_event_wireless_custom(struct hostapd_data *hapd,struct realtek_driver_data *drv, u16 flags, char * custom, size_t len)
{	
	wpa_printf(MSG_DEBUG, "custom event =%d, len=%d", flags, len);

	switch(flags)
	{
		case HAPD_MIC_FAILURE:
		{
			unsigned char * mac = (unsigned char *)custom;
			wpa_printf(MSG_DEBUG, "MIC failed sta: %02x %02x %02x %02x %02x %02x"
					, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			ieee80211_michael_mic_failure(hapd, mac, 1);
			break;
		}
		case HAPD_WPS_PROBEREQ:
		{
			struct _DOT11_PROBE_REQUEST_IND *wps_ie = (struct _DOT11_PROBE_REQUEST_IND *) custom;
			wpa_printf(MSG_DEBUG, "IsMoreEvent =%d, ProbeIELen =%d", wps_ie->IsMoreEvent, wps_ie->ProbeIELen);
			hostapd_wps_probe_req_rx(hapd, wps_ie->MACAddr, wps_ie->ProbeIE, wps_ie->ProbeIELen);
			break;
		}
	}

}


static void
realtek_wireless_event_wireless(struct hostapd_data *hapd,struct realtek_driver_data *drv,
					    u8 *data, int len)
{	
	struct iw_event iwe_buf, *iwe = &iwe_buf;
	u8 *pos, *end, *custom, *buf;
        u8 macaddr[ETH_ALEN] = {};

	pos = data;
	end = data + len;

	wpa_printf(MSG_DEBUG, "realtek_wireless_event_wireless +++");

	while (pos + IW_EV_LCP_LEN <= end) {
		/* Event data may be unaligned, so make a local, aligned copy
		 * before processing. */
		memcpy(&iwe_buf, pos, IW_EV_LCP_LEN);

#ifdef INBAND_CTRL //HOST_LITTLE_ENDIAN		
		iwe->len = ntohs(iwe->len);		
		iwe->cmd = ntohs(iwe->cmd);
#endif
		if (iwe->len <= IW_EV_LCP_LEN)
			return;

		custom = pos + IW_EV_POINT_LEN;
		if (drv->we_version > 18 &&
		    (iwe->cmd == IWEVMICHAELMICFAILURE ||
                     iwe->cmd == SIOCGIWESSID ||
                     iwe->cmd == SIOCGIWENCODE ||
                     iwe->cmd == IWEVGENIE ||
		     iwe->cmd == IWEVASSOCREQIE ||
		     iwe->cmd == IWEVASSOCRESPIE ||
		     iwe->cmd == IWEVCUSTOM)) {
			/* WE-19 removed the pointer from struct iw_point */
			char *dpos = (char *) &iwe_buf.u.data.length;
			int dlen = dpos - (char *) &iwe_buf;
			memcpy(dpos, pos + IW_EV_LCP_LEN,
			       sizeof(struct iw_event) - dlen);
		} else {
			memcpy(&iwe_buf, pos, sizeof(struct iw_event));
			custom += IW_EV_POINT_OFF;
#ifdef INBAND_CTRL //HOST_LITTLE_ENDIAN       		        
			iwe->len = ntohs(iwe->len);                        
			iwe->cmd = ntohs(iwe->cmd);
#endif			
		}
	
		switch (iwe->cmd) {
		case IWEVEXPIRED:
			wpa_printf(MSG_DEBUG, "case IWEVEXPIRED");
			realtek_del_sta(hapd,drv, (u8 *) iwe->u.addr.sa_data);
			break;
		case IWEVREGISTERED:
			wpa_printf(MSG_DEBUG, "case IWEVREGISTERED");
			realtek_new_sta(hapd,drv, (u8 *) iwe->u.addr.sa_data);
			break;
		case IWEVCUSTOM:
		case IWEVGENIE:
			wpa_printf(MSG_DEBUG, "case IWEVCUSTOM|IWEVGENIE");
#ifdef INBAND_CTRL //HOST_LITTLE_ENDIAN			
			iwe->u.data.length = ntohs(iwe->u.data.length);			
			iwe->u.data.flags = ntohs(iwe->u.data.flags);
#endif			
			if (custom + iwe->u.data.length > end)
				return;
			buf = malloc(iwe->u.data.length + 1);
			if (buf == NULL)
				return;		/* XXX */
			memcpy(buf, custom, iwe->u.data.length);
			buf[iwe->u.data.length] = '\0';
			realtek_wireless_event_wireless_custom(hapd,drv, iwe->u.data.flags, buf, iwe->u.data.length);
			free(buf);
			break;
		}
		pos += iwe->len;
	}
}


static void
realtek_wireless_event_rtm_newlink(struct realtek_driver_data *drv,
					       struct nlmsghdr *h, int len)
{
	struct ifinfomsg *ifi;
	int attrlen, nlmsg_len, rta_len;
	struct rtattr * attr;
	struct hostapd_data *hapd = drv->hapd;

	wpa_printf(MSG_DEBUG, "realtek_wireless_event_rtm_newlink +++");

	if (len < (int) sizeof(*ifi))
		return;

	ifi = NLMSG_DATA(h);
#ifdef INBAND_CTRL //HOST_LITTLE_ENDIAN	
	ifi->ifi_index = ntohl(ifi->ifi_index);
#endif		
#ifdef RTK_MBSSID
	if(!have_ifidx(drv, ifi->ifi_index))
		return;
	hapd = find_hapd_by_ifindex(drv,ifi->ifi_index);	
#else
	if (ifi->ifi_index != drv->ifindex)
		return;	
#endif
	nlmsg_len = NLMSG_ALIGN(sizeof(struct ifinfomsg));

	attrlen = h->nlmsg_len - nlmsg_len;
	if (attrlen < 0)
		return;

	attr = (struct rtattr *) (((char *) ifi) + nlmsg_len);

#ifdef INBAND_CTRL //HOST_LITTLE_ENDIAN	
	attr->rta_len = ntohs(attr->rta_len);	
	attr->rta_type = ntohs(attr->rta_type);
#endif	
	rta_len = RTA_ALIGN(sizeof(struct rtattr));
	while (RTA_OK(attr, attrlen)) {
		if (attr->rta_type == IFLA_WIRELESS) {
			realtek_wireless_event_wireless(hapd,
				drv, ((u8 *) attr) + rta_len,
				attr->rta_len - rta_len);
		}
		attr = RTA_NEXT(attr, attrlen);
#ifdef INBAND_CTRL //HOST_LITTLE_ENDIAN 
		attr->rta_len = ntohs(attr->rta_len);        	
		attr->rta_type = ntohs(attr->rta_type);
#endif
	}
}


static void
realtek_wireless_event_receive(int sock, void *eloop_ctx, void *sock_ctx)
{
	//char buf[256];
	char buf[1024], cmd_type=0x01; //increse size for IWEVGENIE event
	int left;
	struct sockaddr_nl from;
	socklen_t fromlen;
	struct nlmsghdr *h;
	struct realtek_driver_data *drv = eloop_ctx;
#ifdef INBAND_CTRL
	left = ioh_recv(&drv->netlink_ioh_obj, 3000);
	if (left < 0) {
			perror("recvfrom(rawsock)");
		return;
	}
	INBAND_IOCTLPKT_DUMP(drv->netlink_ioh_obj.rx_data,left);
	left -= IOH_HDR_LEN;

	h = (struct nlmsghdr *)drv->netlink_ioh_obj.rx_data ;
#else
	fromlen = sizeof(from);
	left = recvfrom(sock, buf, sizeof(buf), MSG_DONTWAIT,
			(struct sockaddr *) &from, &fromlen);
	if (left < 0) {
		if (errno != EINTR && errno != EAGAIN)
			perror("recvfrom(netlink)");
		return;
	}

	h = (struct nlmsghdr *) buf;
#endif
	while (left >= (int) sizeof(*h)) {
		int len, plen;
#ifdef INBAND_CTRL //HOST_LITTLE_ENDIAN //mark_x86	        
		h->nlmsg_len = ntohl(h->nlmsg_len);        	
		h->nlmsg_type = ntohs(h->nlmsg_type);	        
		h->nlmsg_flags = ntohs(h->nlmsg_flags);        	
		h->nlmsg_seq = ntohl(h->nlmsg_seq);	        
		h->nlmsg_pid = ntohl(h->nlmsg_pid);
#endif
		len = h->nlmsg_len;
		plen = len - sizeof(*h);
		if (len > left || plen < 0) {
			printf("Malformed netlink message: "
			       "len=%d left=%d plen=%d\n",
			       len, left, plen);
			break;
		}

		switch (h->nlmsg_type) {
		case RTM_NEWLINK:
			realtek_wireless_event_rtm_newlink(drv, h, plen);
			break;
		}

		len = NLMSG_ALIGN(len);
		left -= len;
		h = (struct nlmsghdr *) ((char *) h + len);
	}

	if (left > 0) {
		printf("%d extra bytes in the end of netlink message\n", left);
	}
}


static int
realtek_get_we_version(struct realtek_driver_data *drv)
{
	struct iw_range *range;
	struct iwreq iwr;
	int minlen;
	size_t buflen;

	drv->we_version = 0;

	/*
	 * Use larger buffer than struct iw_range in order to allow the
	 * structure to grow in the future.
	 */
	buflen = sizeof(struct iw_range) + 500;
	range = os_zalloc(buflen);
	if (range == NULL)
		return -1;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, drv->iface, IFNAMSIZ);
	iwr.u.data.pointer = (caddr_t) range;
	iwr.u.data.length = buflen;

	minlen = ((char *) &range->enc_capa) - (char *) range +
		sizeof(range->enc_capa);
#ifdef STAND_ALONE
	if (ioctl(drv->ioctl_sock, SIOCGIWRANGE, &iwr) < 0) {
		perror("ioctl[SIOCGIWRANGE]");
		free(range);
		return -1;
	}
	else if (iwr.u.data.length >= minlen &&
		   range->we_version_compiled >= 18) {
		wpa_printf(MSG_DEBUG, "SIOCGIWRANGE: WE(compiled)=%d "
			   "WE(source)=%d enc_capa=0x%x",
			   range->we_version_compiled,
			   range->we_version_source,
			   range->enc_capa);
		drv->we_version = range->we_version_compiled;
	}
#endif
#if defined(INBAND_CTRL)
	INBAND_IOCTLPKT_DUMP(&iwr,sizeof(iwr));
	if (inband_ioctl(SIOCGIWRANGE, &iwr) < 0) {
		perror("inband_ioctl[SIOCGIWRANGE]");
		free(range);
		return -1;
	} else if (iwr.u.data.length >= minlen &&
		   range->we_version_compiled >= 18) {
		wpa_printf(MSG_DEBUG, "SIOCGIWRANGE: WE(compiled)=%d "
			   "WE(source)=%d enc_capa=0x%x",
			   range->we_version_compiled,
			   range->we_version_source,
			   range->enc_capa);
		drv->we_version = range->we_version_compiled;
	}
#endif


	free(range);
	return 0;
}


static int
realtek_wireless_event_init(void *priv)
{
	struct realtek_driver_data *drv = priv;
	int s;
	struct sockaddr_nl local;

	realtek_get_we_version(drv);

	drv->wext_sock = -1;
#ifdef INBAND_CTRL
	s = ioh_open(&drv->netlink_ioh_obj,INBAND_INTF,INBAND_SLAVE,INBAND_NETLINK_TYPE,INBAND_DEBUG);
	if (s < 0) {
		perror("socket(PF_PACKET,SOCK_RAW,INBAND_NETLINK_TYPE)");
		return -1;
	}
#else
	s = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (s < 0) {
		perror("socket(PF_NETLINK,SOCK_RAW,NETLINK_ROUTE)");
		return -1;
	}

	memset(&local, 0, sizeof(local));
	local.nl_family = AF_NETLINK;
	local.nl_groups = RTMGRP_LINK;
	if (bind(s, (struct sockaddr *) &local, sizeof(local)) < 0) {
		perror("bind(netlink)");
		close(s);
		return -1;
	}
#endif

#ifdef INBAND_CTRL
	eloop_register_read_sock(drv->netlink_ioh_obj.sockfd, realtek_wireless_event_receive, drv, NULL);
#else
	eloop_register_read_sock(s, realtek_wireless_event_receive, drv, NULL);
#endif
#ifdef INBAND_CTRL
	drv->wext_sock = drv->netlink_ioh_obj.sockfd;
#else
	drv->wext_sock = 0;
#endif

	return 0;
}


static void
realtek_wireless_event_deinit(void *priv)
{
	struct realtek_driver_data *drv = priv;

	wpa_printf(MSG_DEBUG, "realtek_wireless_event_deinit +++");

	if (drv != NULL) {
		if (drv->wext_sock < 0)
			return;
		eloop_unregister_read_sock(drv->wext_sock);
		close(drv->wext_sock);
	}
}


static int
realtek_send_eapol(void *priv, const u8 *addr, const u8 *data, size_t data_len,
		   int encrypt, const u8 *own_addr)
{
	struct realtek_driver_data *drv = priv;
	unsigned char buf[3000];
	unsigned char *bp = buf;
	struct l2_ethhdr *eth;
	size_t len;
	int status;
	struct l2_packet_data *eapol_sock=drv->sock_xmit;
#ifdef RTK_MBSSID
#ifndef INBAND_CTRL
	struct hostapd_data *hapd = drv->hapd;	

	hapd = (struct hostapd_data *)find_hapd_by_sta(priv,addr);	
	eapol_sock = (struct l2_packet_data *)rtk_find_l2sock_by_ifname(priv,hapd->conf->iface);	
#endif		
#endif
	wpa_printf(MSG_DEBUG, "realtek_send_eapol +++");

	/*
	 * Prepend the Ethernet header.  If the caller left us
	 * space at the front we could just insert it but since
	 * we don't know we copy to a local buffer.  Given the frequency
	 * and size of frames this probably doesn't matter.
	 */
	len = data_len + sizeof(struct l2_ethhdr);
	if (len > sizeof(buf)) {
		bp = malloc(len);
		if (bp == NULL) {
			printf("EAPOL frame discarded, cannot malloc temp "
			       "buffer of size %lu!\n", (unsigned long) len);
			return -1;
		}
	}
	eth = (struct l2_ethhdr *) bp;
	memcpy(eth->h_dest, addr, ETH_ALEN);
	memcpy(eth->h_source, drv->hapd->own_addr, ETH_ALEN);	
	eth->h_proto = htons(ETH_P_EAPOL);
	memcpy(eth+1, data, data_len);

	//mark wpa_hexdump because both ralink & broadcom NOT call this function
	//wpa_hexdump(MSG_MSGDUMP, "TX EAPOL", bp, len);

	status = l2_packet_send(eapol_sock, addr, ETH_P_EAPOL, bp, len);
	if (bp != buf)
		free(bp);
	return status;
}

static void
handle_read(void *ctx, const u8 *src_addr, const u8 *buf, size_t len)
{
	struct realtek_driver_data *drv = ctx;
	struct hostapd_data *hapd = drv->hapd;
	struct sta_info *sta;

	//printf("handle_read +++"); 
#ifndef RTK_MBSSID	
	sta = ap_get_sta(hapd, src_addr);
	if (!sta || !(sta->flags & WLAN_STA_ASSOC)) {
		printf("Data frame from not associated STA %s\n",
		       ether_sprintf(src_addr));
		/* XXX cannot happen */
		return;
	}
	
	ieee802_1x_receive(hapd, src_addr, buf + sizeof(struct l2_ethhdr),
			   len - sizeof(struct l2_ethhdr));
#else
	return; // do nothing here , all handle in handle_eapol
#endif
}

#ifdef INBAND_CTRL
static int rtk_l2_packet_get_own_addr(struct realtek_driver_data *drv, u8 *addr)
{
	unsigned char cmd[100];
	struct ifreq ifr;

	os_memset(&ifr, 0, sizeof(ifr));
	os_strlcpy(ifr.ifr_name, INBAND_INTF, sizeof(INBAND_INTF));

	if(ioctl(drv->ioctl_sock,SIOCGIFHWADDR, &ifr ) < 0){
		return -1;
	}

	sprintf(cmd,"echo %d:%d:%d:%d:%d:%d > /proc/br_hostmac",ifr.ifr_hwaddr.sa_data[0],
		ifr.ifr_hwaddr.sa_data[1],ifr.ifr_hwaddr.sa_data[2],ifr.ifr_hwaddr.sa_data[3],
		ifr.ifr_hwaddr.sa_data[4],ifr.ifr_hwaddr.sa_data[5]);

	if (inband_remote_cmd(cmd) < 0)
		return -1;

	os_strlcpy(ifr.ifr_name, drv->iface, sizeof(ifr.ifr_name));
	if (inband_ioctl(SIOCGIFHWADDR,&ifr) < 0)
		return -1;
	else
		os_memcpy(addr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
	return 0;
}
#endif

#ifdef RTK_MBSSID
static int realtek_bss_add(void *priv, const char *ifname, const u8 *bssid)
{	
	struct realtek_driver_data *drv = priv;
	struct i802_bss *new_bss = NULL;	
	struct ifreq ifr;
      	struct rtk_hapd_config config;
	struct hostapd_data *hapd = drv->hapd;		
	
	//printf("realtek_bss_add : ifname = %s\n",ifname);

	new_bss = os_zalloc(sizeof(*new_bss));
	if (new_bss == NULL)
		return -1;
	

	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", ifname);
#ifdef STAND_ALONE
	if (ioctl(drv->ioctl_sock, SIOCGIFINDEX, &ifr) != 0) {
		perror("ioctl(SIOCGIFINDEX)");
		goto bad;
	}
#endif
#if defined(INBAND_CTRL)
	INBAND_IOCTLPKT_DUMP(&ifr,sizeof(ifr));
	if (inband_ioctl(SIOCGIFINDEX, &ifr) < 0) {
		perror("inband_ioctl(SIOCGIFINDEX)");
		goto bad;
	}
#endif	
	os_strlcpy(new_bss->ifname, ifname, IFNAMSIZ);
	
#ifdef INBAND_CTRL
	new_bss->sock_xmit = drv->sock_xmit; 
#else
	new_bss->sock_xmit = l2_packet_init(new_bss->ifname, NULL, ETH_P_EAPOL, handle_read, drv, 1);
#endif
	if (new_bss->sock_xmit == NULL)
		goto bad;
	
	//do we  nee to set HW addr to VAP??	
	realtek_set_iface_flags(new_bss->ifname,drv, 0); //down the interface ...... , bring up in  realtek_commit!! 

	if (bssid[0] | bssid[1] | bssid[2] | bssid[3] | bssid[4] | bssid[5]) 
		realtek_set_iface_hwMac(new_bss->ifname,drv, bssid);
	else
		printf("Warning !!! VAP no HW addr setting \n");
	
      	//memset(&config, 0, sizeof(struct rtk_hapd_config));
      	os_memcpy(&config, &rtk_config, sizeof(struct rtk_hapd_config)); //get some val from root
	hapd =(struct hostapd_data *) find_hapd_by_ifname(priv,ifname);

	realtek_set_privacy(ifname, drv, 0); /* default to no privacy */

        if(realtek_read_hapd_cfg(hapd,priv, &config))
		goto bad;

	//if(realtek_read_priv_cfg(priv, &config)) //already read in init
	//	goto bad;
	if(realtek_read_priv_vap_cfg(ifname,priv, &config)) //already read in init
		goto bad;


#ifdef INBAND_CTRL //HOST_LITTLE_ENDIAN		
	//   rtk_hapd_config need to swap  , write a function to do it	
	rtk_cfg_to_bigEndian(&config);
#endif

	if(set80211priv(ifname,drv, HAPD_IOCTL_SETCONFIG, &config, sizeof(config))) 
	{
		wpa_printf(MSG_ERROR, "%s: Failed to set Configurations", __func__);
		goto bad;
	}

	/*if(realtek_read_wds_cfg(priv, &wds)) //mark_mbssid , wds for vap ??
		return -1;*/

	if((hapd->conf->wpa == 0) && (hapd->conf->ieee802_1x == 0 ))
		realtek_set_ieee8021x(ifname, drv, 0);	
	
	new_bss->ifindex =  ifr.ifr_ifindex;
	new_bss->drv = drv;
	new_bss->next = drv->first_bss.next;
	drv->first_bss.next = new_bss;
	add_ifidx(drv, new_bss->ifindex);

	return 0;
	
bad :
	os_free(new_bss);
	return -1;
}

static int realtek_bss_remove(void *priv, const char *ifname)
{	
	
	//printf("realtek_bss_remove : ifname = %s\n",ifname);	

	rtk_free_bss_by_ifname(priv,ifname);

	realtek_set_iface_flags(ifname,priv, 0); //down VAP	
	
	return 0;
}

static void *
realtek_driver_init(struct hostapd_data *hapd)
{
	struct realtek_driver_data *drv;
	struct ifreq ifr;
	struct iwreq iwr;
	struct i802_bss *bss;

	wpa_printf(MSG_DEBUG, "realtek_init +++");
	
	drv = os_zalloc(sizeof(struct realtek_driver_data));
	if (drv == NULL) {
		printf("Could not allocate memory for realtek driver data\n");
		goto bad;
	}

	drv->hapd = hapd;
	bss = &drv->first_bss;
	bss->drv = drv;
	os_strlcpy(bss->ifname, hapd->conf->iface, sizeof(bss->ifname));
	
	drv->ioctl_sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (drv->ioctl_sock < 0) {
		perror("socket[PF_INET,SOCK_DGRAM]");
		goto bad;
	}
	memcpy(drv->iface, hapd->conf->iface, sizeof(drv->iface));

	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", drv->iface);
#ifdef STAND_ALONE
	if (ioctl(drv->ioctl_sock, SIOCGIFINDEX, &ifr) != 0) {
		perror("ioctl(SIOCGIFINDEX)");
		goto bad;
	}
#endif
#if defined(INBAND_CTRL)
	INBAND_IOCTLPKT_DUMP(&ifr,sizeof(ifr));
	if (inband_ioctl(SIOCGIFINDEX, &ifr) < 0) {
		perror("inband_ioctl(SIOCGIFINDEX)");
		goto bad;
	}
#endif
	drv->ifindex = ifr.ifr_ifindex;
	bss->ifindex = drv->ifindex;
#ifdef INBAND_CTRL
	drv->sock_xmit = l2_packet_init(INBAND_INTF, NULL, ETH_P_EAPOL, handle_read, drv, 1);
#else
	drv->sock_xmit = l2_packet_init(drv->iface, NULL, ETH_P_EAPOL, handle_read, drv, 1);
#endif
	if (drv->sock_xmit == NULL)
		goto bad;
#ifdef INBAND_CTRL
	if( rtk_l2_packet_get_own_addr(drv,hapd->own_addr) )
#else
	if (l2_packet_get_own_addr(drv->sock_xmit, hapd->own_addr))
#endif
		goto bad;

       //mark_mbssid , now recv all EAPOL from one socket
	drv->sock_recv = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_EAPOL));
	if (drv->sock_recv < 0) {
		perror("socket(PF_PACKET, SOCK_DGRAM, ETH_P_EAPOL)");
		goto bad;
	}
	if (eloop_register_read_sock(drv->sock_recv, handle_eapol, drv, NULL))
	{
		printf("Could not register read socket for eapol\n");
		goto bad;
	}
	
	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, drv->iface, IFNAMSIZ);

	realtek_set_iface_flags(drv->iface,drv, 0);	/* mark down during setup */
	realtek_set_privacy(drv->iface, drv, 0); /* default to no privacy */

	if(realtek_hapd_config(drv)) 
		goto bad;

	if((hapd->conf->wpa == 0) && (hapd->conf->ieee802_1x == 0 ))
		realtek_set_ieee8021x(drv->iface, drv, 0);

	wpa_printf(MSG_DEBUG, "realtek_init ---");
	return bss;
bad:
        wpa_printf(MSG_ERROR, "realtek_init failed!");

	if (drv != NULL)
		realtek_deinit(drv);
	
	return NULL;
}

static void *
realtek_init(struct hostapd_data *hapd)
{
	struct realtek_driver_data *drv;
	struct i802_bss *bss;
	int i;

	bss = realtek_driver_init(hapd);
	
	if (bss == NULL)
		return NULL;

	drv = bss->drv;	

	drv->num_if_indices = RTK_MAX_IF_INDEX;
	for(i=0;i<drv->num_if_indices;i++)
		drv->if_indices[i]=0;

	add_ifidx(drv, drv->ifindex);	

	//below maybe add bridge control in the future	
	//return bss;
	return drv;

}

#else //mark_mbssid , remove below in future

static void *
realtek_init(struct hostapd_data *hapd)
{
	struct realtek_driver_data *drv;
	struct ifreq ifr;
	struct iwreq iwr;

	wpa_printf(MSG_DEBUG, "realtek_init +++");
	
	drv = os_zalloc(sizeof(struct realtek_driver_data));
	if (drv == NULL) {
		printf("Could not allocate memory for realtek driver data\n");
		goto bad;
	}

	drv->hapd = hapd;
	drv->ioctl_sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (drv->ioctl_sock < 0) {
		perror("socket[PF_INET,SOCK_DGRAM]");
		goto bad;
	}
	memcpy(drv->iface, hapd->conf->iface, sizeof(drv->iface));

	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", drv->iface);
#ifdef STAND_ALONE
	if (ioctl(drv->ioctl_sock, SIOCGIFINDEX, &ifr) != 0) {
		perror("ioctl(SIOCGIFINDEX)");
		goto bad;
	}
#endif
#if defined(INBAND_CTRL)
	INBAND_IOCTLPKT_DUMP(&ifr,sizeof(ifr));
	if (inband_ioctl(SIOCGIFINDEX, &ifr) < 0) {
		perror("inband_ioctl(SIOCGIFINDEX)");
		goto bad;
	}
#endif
	drv->ifindex = ifr.ifr_ifindex;
#ifdef INBAND_CTRL
	drv->sock_xmit = l2_packet_init(INBAND_INTF, NULL, ETH_P_EAPOL, handle_read, drv, 1);
#else
	drv->sock_xmit = l2_packet_init(drv->iface, NULL, ETH_P_EAPOL, handle_read, drv, 1);
#endif
	if (drv->sock_xmit == NULL)
		goto bad;
#ifdef INBAND_CTRL
	if( rtk_l2_packet_get_own_addr(drv,hapd->own_addr) )
#else
	if (l2_packet_get_own_addr(drv->sock_xmit, hapd->own_addr))
#endif
		goto bad;
	if (hapd->conf->bridge[0] != '\0') {
		drv->sock_recv = l2_packet_init(hapd->conf->bridge, NULL,
						ETH_P_EAPOL, handle_read, drv,
						1); //fix receive sock??
		if (drv->sock_recv == NULL)
			goto bad;
	} else
		drv->sock_recv = drv->sock_xmit;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, drv->iface, IFNAMSIZ);

	realtek_set_iface_flags(drv->iface,drv, 0);	/* mark down during setup */
	realtek_set_privacy(drv->iface, drv, 0); /* default to no privacy */

	if(realtek_hapd_config(drv))
		goto bad;

	if((hapd->conf->wpa == 0) && (hapd->conf->ieee802_1x == 0 ))
		realtek_set_ieee8021x(drv->iface, drv, 0);

	wpa_printf(MSG_DEBUG, "realtek_init ---");
	return drv;
bad:
        wpa_printf(MSG_ERROR, "realtek_init failed!");

	if (drv != NULL)
		realtek_deinit(drv);
	
	return -1;
}
#endif

static void
realtek_deinit(void *priv)
{
	struct realtek_driver_data *drv = priv;

	wpa_printf(MSG_DEBUG, "realtek_deinit +++");

	drv->hapd->driver = NULL;

	realtek_set_iface_flags(drv->iface,drv, 0);
        #if 0   /* OLD */
	if (drv->probe_recv != NULL)
		l2_packet_deinit(drv->probe_recv);
        #endif
	if (drv->ioctl_sock >= 0)
		close(drv->ioctl_sock);
#ifdef RTK_MBSSID
	if (drv->sock_recv >= 0) {
		eloop_unregister_read_sock(drv->sock_recv);
		close(drv->sock_recv);
	}
#else
	if (drv->sock_recv != NULL && drv->sock_recv != drv->sock_xmit) 
		l2_packet_deinit(drv->sock_recv);
#endif
	if (drv->sock_xmit != NULL)
		l2_packet_deinit(drv->sock_xmit);
	free(drv);

	wpa_printf(MSG_DEBUG, "realtek_deinit ---");
}

static int
realtek_set_ssid(const char *ifname, void *priv, const u8 *buf, int len)
{
	struct realtek_driver_data *drv = priv;	
	struct iwreq iwr;

	wpa_printf(MSG_DEBUG, "realtek_set_ssid +++");

	memset(&iwr, 0, sizeof(iwr));	
	strncpy(iwr.ifr_name, ifname, IFNAMSIZ);
	iwr.u.essid.flags = 1; /* SSID active */
	iwr.u.essid.pointer = (caddr_t) buf;
	iwr.u.essid.length = len + 1;
#ifdef STAND_ALONE
	if (ioctl(drv->ioctl_sock, SIOCSIWESSID, &iwr) < 0) {
		perror("ioctl[SIOCSIWESSID]");
		printf("len=%d\n", len);
		return -1;
	}
#endif
#if defined(INBAND_CTRL)
	INBAND_IOCTLPKT_DUMP(&iwr,sizeof(iwr));
	if (inband_ioctl(SIOCSIWESSID, &iwr) < 0) {
		perror("inband_ioctl[SIOCSIWESSID]");
		printf("len=%d\n", len);
		return -1;
	}
#endif
	return 0;
}

static int
realtek_get_ssid(const char *ifname, void *priv, u8 *buf, int len)
{
	struct realtek_driver_data *drv = priv;	
	struct iwreq iwr;
	int ret = 0;

	wpa_printf(MSG_DEBUG, "realtek_get_ssid +++");
	
	memset(&iwr, 0, sizeof(iwr));	
	strncpy(iwr.ifr_name, ifname, IFNAMSIZ);
	iwr.u.essid.pointer = (caddr_t) buf;
	iwr.u.essid.length = len;
#ifdef STAND_ALONE
	if (ioctl(drv->ioctl_sock, SIOCGIWESSID, &iwr) < 0) {
		perror("ioctl[SIOCGIWESSID]");
		ret = -1;
	} else
		ret = iwr.u.essid.length;
#endif
#if defined(INBAND_CTRL)
	INBAND_IOCTLPKT_DUMP(&iwr,sizeof(iwr));
	if (inband_ioctl(SIOCGIWESSID, &iwr) < 0) {
		perror("ioctl[SIOCGIWESSID]");
		ret = -1;
	}
#endif
	return ret;
}

static int
realtek_set_countermeasures(void *priv, int enabled)
{
	struct realtek_driver_data *drv = priv;
	wpa_printf(MSG_DEBUG, "realtek_set_countermeasures +++");
	wpa_printf(MSG_DEBUG, "%s: enabled=%d", __FUNCTION__, enabled);
	return set80211param(drv->iface,drv, IEEE80211_PARAM_COUNTERMEASURES, enabled);
}

#ifdef INBAND_CTRL
/* inband ioctl start */
inline void realtek_priviwr(struct iwreq *iwr, unsigned char *param, void *value)
{
	unsigned char buf[1024] = {0};

	sprintf(buf,"%s=%d",param,*(unsigned int *)value);
	iwr->u.data.length = os_strlen(buf);
	iwr->u.data.pointer = os_malloc(iwr->u.data.length+1);
	if( iwr->u.data.pointer )
		os_memcpy(iwr->u.data.pointer,buf,iwr->u.data.length+1);
	else
		printf("Err: Alloc memory failed while %s\n",__FUNCTION__);
}
#endif


static int
realtek_commit(void *priv)
{
	struct realtek_driver_data *drv = priv;
#ifdef RTK_MBSSID
	struct i802_bss *tbss = &drv->first_bss,*bss;
	wpa_printf(MSG_DEBUG, "realtek_commit +++");

	//up root interface here
	if(realtek_set_iface_flags(drv->iface,priv, 1) < 0)
		return -1;
	//up other VAP
	while (tbss) {
		
			bss = tbss->next;
			if(bss == NULL)
				break;			
			
			if(realtek_set_iface_flags(bss->ifname,priv, 1)<0)
				return -1;
			tbss = tbss->next  ;
	}
	return 0;
#else
	return realtek_set_iface_flags(drv->iface,priv, 1);
#endif
}

#ifdef EAP_WPS
static int
realtek_set_wps_ie(const char *ifname,void *priv, u8 *iebuf, int iebuflen, u32 frametype)
{
	struct realtek_driver_data *drv = priv;	
	u8 buf[256];
	struct ieee80211req_getset_appiebuf * ie;
	// int i;

	ie = (struct ieee80211req_getset_appiebuf *) buf;
	ie->app_frmtype = frametype;
	ie->app_buflen = iebuflen;
        if (iebuflen > 0)
	        os_memcpy(&(ie->app_buf[0]), iebuf, iebuflen);

#ifdef INBAND_CTRL //HOST_LITTLE_ENDIAN 
	// int32  need to swap 	
	ie->app_frmtype = htonl(ie->app_frmtype);	
	ie->app_buflen = htonl(ie->app_buflen);
#endif		
	return set80211priv(ifname,priv, IEEE80211_IOCTL_SET_APPIEBUF, ie,
			sizeof(struct ieee80211req_getset_appiebuf) + iebuflen);
}


static int 
realtek_set_wps_beacon_ie(const char *ifname, void *priv, const u8 *iebuf, size_t iebuflen)
{
	wpa_printf(MSG_DEBUG, "realtek_set_wps_beacon_ie +++");
	return realtek_set_wps_ie(ifname,priv, iebuf, iebuflen, 
			IEEE80211_APPIE_FRAME_BEACON);
}


static int 
realtek_set_wps_probe_resp_ie(const char *ifname, void *priv, const u8 *iebuf, size_t iebuflen)
{
	wpa_printf(MSG_DEBUG, "realtek_set_wps_probe_resp_ie +++");
	return realtek_set_wps_ie(ifname,priv, iebuf, iebuflen, 
			IEEE80211_APPIE_FRAME_PROBE_RESP);
}


static int 
realtek_set_wps_assoc_resp_ie(const char *ifname, void *priv, const u8 *iebuf, size_t iebuflen)
{
	wpa_printf(MSG_DEBUG, "realtek_set_wps_assoc_resp_ie +++");
	return realtek_set_wps_ie(ifname,priv, iebuf, iebuflen, 
			IEEE80211_APPIE_FRAME_ASSOC_RESP);
}


/* Ask to receive copies of all probe requests received.
 */
static int
realtek_start_receive_prob_req(void *priv)
{
#if 0 //this function is not used now!!
	struct ieee80211req_set_filter filt;

	wpa_printf(MSG_DEBUG, "%s Enter\n", __FUNCTION__);
	filt.app_filterype = IEEE80211_FILTER_TYPE_PROBE_REQ;
#ifdef INBAND_CTRL //HOST_LITTLE_ENDIAN 
	// int32  need to swap 	
	filt.app_filterype = htonl(filt.app_filterype);	
#endif
	return set80211priv(priv, IEEE80211_IOCTL_FILTERFRAME, &filt,
            			sizeof(struct ieee80211req_set_filter));
#endif
	return 0;
}

#endif /* EAP_WPS */

#ifdef RTK_HAPD
//Turn ON|OFF driver for hostapd reload.
//mark_mbssid , realtek_driver_on , it's only for WPS in root hapd reload
static int realtek_driver_on(void *priv, int on)
{
	struct realtek_driver_data *drv = priv;
	struct hostapd_data *hapd = drv->hapd;
	
	wpa_printf(MSG_DEBUG, "realtek_driver_on = %d +++", on);

	if((hapd->conf->wpa == 0) && (hapd->conf->ieee802_1x ==0 ) && (on == 1))
		realtek_set_ieee8021x(drv->iface, drv, 0);
	
	return realtek_set_iface_flags(drv->iface,priv, on);
}

static int realtek_config_rate(int *rate_list, unsigned int *rate_config)
{
	// bit mask value. bit0-bit11 as 1,2,5.5,11,6,9,12,18,24,36,48,54
	int temp = 0;
	
	while(1)
		{
			int rate; 
			rate = rate_list[temp];
			if((rate == -1) || (rate == 0))
				break;
			
			if(rate == 10)
				*rate_config |= BIT(0); 
			else if(rate == 20)
				*rate_config |= BIT(1); 
			else if(rate == 55)
				*rate_config |= BIT(2); 
			else if(rate == 110)
				*rate_config |= BIT(3); 
			else if(rate == 60)
				*rate_config |= BIT(4); 
			else if(rate == 90)
				*rate_config |= BIT(5); 
			else if(rate == 120)
				*rate_config |= BIT(6); 
			else if(rate == 180)
				*rate_config |= BIT(7); 
			else if(rate == 240)
				*rate_config |= BIT(8); 
			else if(rate == 360)
				*rate_config |= BIT(9); 
			else if(rate == 480)
				*rate_config |= BIT(10); 
			else if(rate == 540)
				*rate_config |= BIT(11); 

			temp++;
			
			if(temp > 12)
				{
					wpa_printf(MSG_ERROR, "Config Rates NUM > 12!!!");
					return -1;
				}
			
		}

	return 0;

}

static int realtek_parse_pwrlevel(unsigned char* pwr_list, char *val)
{
	int count;
	char *pos, *end;

	pos = val;
	count = 0;
	while (*pos != '\0') {
		if (*pos == ' ')
			count++;
		pos++;
	}

	pos = val;
	count = 0;
	
	while (*pos != '\0') {
		end = os_strchr(pos, ' ');
		if (end)
			*end = '\0';

		pwr_list[count] = atoi(pos);
		
		if (!end)
			break;
		
		count ++;
		if(count >=MAX_2G_CHANNEL_NUM)
			break;
		
		pos = end + 1;
	}

	return 0;
}


static int realtek_read_hapd_cfg(struct hostapd_data *hapd,void *priv, struct rtk_hapd_config* config)
{
	struct realtek_driver_data *drv = priv;	
	struct hostapd_config *iconf = hapd->iconf;
	struct hostapd_bss_config *conf = hapd->conf;

	int temp = 0;
	
	wpa_printf(MSG_DEBUG, "realtek_read_hapd_cfg +++");

	config->band |= BIT(iconf->hw_mode);
	if(iconf->ieee80211n)
		config->band |= BIT(3);
	
	config->channel = iconf->channel;

	config->bcnint = iconf->beacon_int;

	config->dtimperiod = conf->dtim_period;

	//if(conf->max_num_sta > RTK_MAX_STA)//mark_mbssid ,issue how to sync conf and rtk real support
	config->stanum = conf->max_num_sta;		

	config->rtsthres = iconf->rts_threshold;

	config->fragthres = iconf->fragm_threshold;

	if(realtek_config_rate(iconf->supported_rates, &(config->oprates)))
		return -1;
	
	if(realtek_config_rate(iconf->basic_rates, &(config->basicrates)))
		return -1;

	config->preamble = iconf->preamble;

	config->aclmode = conf->macaddr_acl; 

	if(config->aclmode == ACCEPT_UNLESS_DENIED)
		{
			int x = 0, y =0;
			config->aclnum = conf->num_deny_mac;

			if(config->aclnum > 0)
				{
					for(x=0; x < config->aclnum; x++)
					{
						struct mac_acl_entry *deny_mac = &conf->deny_mac[x];
						for(y=0; y<MACADDRLEN; y++)
						{
							config->acladdr[x][y] = deny_mac->addr[y];
						}
						wpa_printf(MSG_DEBUG, "DENY ACL# %d = %s", x, ether_sprintf(config->acladdr[x]));
					}
				}
		}
	else if(config->aclmode == DENY_UNLESS_ACCEPTED)
		{
			int x = 0, y = 0;
			config->aclnum = conf->num_accept_mac;

			if(config->aclnum > 0)
				{
					for(x=0; x<config->aclnum; x++)
					{
						struct mac_acl_entry *accept_mac = &conf->accept_mac[x];
						for(y=0; y<MACADDRLEN; y++)
						{
								config->acladdr[x][y] = accept_mac->addr[y];
						}
						wpa_printf(MSG_DEBUG, "ACCEPT ACL# %d = %s", x, ether_sprintf(config->acladdr[x]));
					}
				}
		}

	config->hiddenAP = conf->ignore_broadcast_ssid;

	config->qos_enable = conf->wmm_enabled;

	config->expired_time = conf->ap_max_inactivity;

	config->block_relay = iconf->bridge_packets; 

	if(iconf->ht_capab &= HT_CAP_INFO_SHORT_GI20MHZ)
	config->shortGI20M = 1;

	if(iconf->ht_capab &= HT_CAP_INFO_SHORT_GI40MHZ)
	config->shortGI40M = 1;	  
	
	wpa_printf(MSG_DEBUG, "realtek_read_hapd_cfg ---");

	return 0;

}

static int realtek_read_priv_vap_cfg(const char *ifname,void *priv, struct rtk_hapd_config* config)
{

	struct realtek_driver_data *drv = priv;
	FILE *f;
	char buf[256], *pos;
	int errors = 0;
	int line = 0;	
	int in_bss_section=0;

	f = fopen(drv->hapd->iface->config_fname, "r");
	if (f == NULL) 
	{
		wpa_printf(MSG_ERROR, "Could not open configuration file '%s' "
		   			"for reading.", drv->hapd->iface->config_fname);
		return -1;
	}

	while (fgets(buf, sizeof(buf), f)) 
	{
		line++;
		if (buf[0] == '#')
			continue;
		pos = buf;
		while (*pos != '\0') 
		{
			if (*pos == '\n') 
			{
				*pos = '\0';
				break;
			}
			pos++;
		}
		
		if (buf[0] == '\0')
			continue;

		pos = os_strchr(buf, '=');
		if (pos == NULL) 
		{
			wpa_printf(MSG_ERROR, "Line %d: invalid line '%s'", line, buf);
			errors++;
			continue;
		}
		*pos = '\0';
		pos++;
		if (os_strcmp(buf, "bss") == 0) {  

			if(in_bss_section) //already in section , then scan finished;
				break;
			if(os_strcmp(pos, ifname) == 0)
			{
				in_bss_section = 1;				
			}
			continue;
		} 
		//below is the rtk parameter for per VAP
		if(in_bss_section){			
			/*else if (os_strcmp(buf, "autorate") == 0) { //mark_issue
			config->autorate = atoi(pos);
			}*/
			if (os_strcmp(buf, "fixrate") == 0) { 
				unsigned int select=0;
				select =  atoi(pos);
				if(select == 0)
				{
					config->autorate = 1;
					config->fixrate = 0;
				}
				else
				{
					config->autorate = 0;
					config->fixrate =    (1 << (select-1));
				}			
			//mark_issue , need to validate the rate with hw_mode(G? , N?)
			}else if (os_strcmp(buf, "guest_access") == 0) {
				config->guest_access= atoi(pos);
			}	
		}
	}

	fclose(f);

	if (errors) 
		wpa_printf(MSG_ERROR, "%d errors found in configuration file "
		   "'%s'", errors, drv->hapd->iface->config_fname);
	
	return errors;

}


static int realtek_read_priv_cfg(void *priv, struct rtk_hapd_config* config)
{

	struct realtek_driver_data *drv = priv;
	FILE *f;
	char buf[256], *pos;
	int errors = 0;
	int line = 0;

	wpa_printf(MSG_DEBUG, "realtek_read_priv_cfg +++");

	f = fopen(drv->hapd->iface->config_fname, "r");
	if (f == NULL) 
	{
		wpa_printf(MSG_ERROR, "Could not open configuration file '%s' "
		   			"for reading.", drv->hapd->iface->config_fname);
		return -1;
	}

	while (fgets(buf, sizeof(buf), f)) 
	{
		line++;
		if (buf[0] == '#')
			continue;
		pos = buf;
		while (*pos != '\0') 
		{
			if (*pos == '\n') 
			{
				*pos = '\0';
				break;
			}
			pos++;
		}
		
		if (buf[0] == '\0')
			continue;

		pos = os_strchr(buf, '=');
		if (pos == NULL) 
		{
			wpa_printf(MSG_ERROR, "Line %d: invalid line '%s'", line, buf);
			errors++;
			continue;
		}
		*pos = '\0';
		pos++;
		if (os_strcmp(buf, "bss") == 0) {  
			break; //only read  interface  section
		}	
		if (os_strcmp(buf, "pwrlevelCCK_A") == 0) {
			realtek_parse_pwrlevel(config->pwrlevelCCK_A, pos);
		} 
		else if (os_strcmp(buf, "pwrlevelCCK_B") == 0) {
			realtek_parse_pwrlevel(config->pwrlevelCCK_B, pos);
		} 	
		else if (os_strcmp(buf, "pwrlevelHT40_1S_A") == 0) {
			realtek_parse_pwrlevel(config->pwrlevelHT40_1S_A, pos);
		} 
		else if (os_strcmp(buf, "pwrlevelHT40_1S_B") == 0) {
			realtek_parse_pwrlevel(config->pwrlevelHT40_1S_B, pos);
		} 
		else if (os_strcmp(buf, "pwrdiffHT40_2S") == 0) {
			realtek_parse_pwrlevel(config->pwrdiffHT40_2S, pos);
		} 
		else if (os_strcmp(buf, "pwrdiffHT20") == 0) {
			realtek_parse_pwrlevel(config->pwrdiffHT20, pos);
		}
		else if (os_strcmp(buf, "pwrdiffOFDM") == 0) {
			realtek_parse_pwrlevel(config->pwrdiffOFDM, pos);
		}
		else if (os_strcmp(buf, "phyBandSelect") == 0) {
			config->phyBandSelect = atoi(pos);
		}
		else if (os_strcmp(buf, "macPhyMode") == 0) {
			config->macPhyMode = atoi(pos);
		}
		else if (os_strcmp(buf, "ther") == 0) {
			config->ther = atoi(pos);
		}
		else if (os_strcmp(buf, "swcrypto") == 0) {
			config->swcrypto = atoi(pos);
		}
		else if (os_strcmp(buf, "regdomain") == 0) {
			config->regdomain = atoi(pos);
		}
		/*else if (os_strcmp(buf, "autorate") == 0) { //mark_issue
			config->autorate = atoi(pos);
		}*/
		else if (os_strcmp(buf, "fixrate") == 0) { 
			unsigned int select=0;
			select =  atoi(pos);
			if(select == 0)
			{
				config->autorate = 1;
				config->fixrate = 0;
			}
			else
			{
				config->autorate = 0;
				config->fixrate =    (1 << (select-1));
			}
			//mark_issue , need to validate the rate with hw_mode(G? , N?)
		}
		else if (os_strcmp(buf, "disable_protection") == 0) {
			config->disable_protection = atoi(pos);
		}
		else if (os_strcmp(buf, "disable_olbc") == 0) {
			config->disable_olbc = atoi(pos);
		}
		else if (os_strcmp(buf, "deny_legacy") == 0) {
			config->deny_legacy = atoi(pos);
		}
		else if (os_strcmp(buf, "opmode") == 0) {
			config->opmode = atoi(pos);
		}
		else if (os_strcmp(buf, "vap_enable") == 0) {
			config->vap_enable = atoi(pos);
		}
		else if (os_strcmp(buf, "use40M") == 0) {
			config->use40M = atoi(pos);
		}
		else if (os_strcmp(buf, "2ndchoffset") == 0) {
			config->_2ndchoffset = atoi(pos);
		}
		else if (os_strcmp(buf, "ampdu") == 0) {
			config->ampdu = atoi(pos);
		}
		else if (os_strcmp(buf, "coexist") == 0) {
			config->coexist = atoi(pos);
		}
		else if (os_strcmp(buf, "rssi_dump") == 0) {
			config->rssi_dump = atoi(pos);
		}
		else if (os_strcmp(buf, "mp_specific") == 0) {
			config->mp_specific = atoi(pos);
		}
		else if (os_strcmp(buf, "use_ext_pa") == 0) {
			config->use_ext_pa = atoi(pos);
		}
		else if (os_strcmp(buf, "guest_access") == 0) {
			config->guest_access= atoi(pos);
		}
		
	}

	fclose(f);

	if (errors) 
		wpa_printf(MSG_ERROR, "%d errors found in configuration file "
		   "'%s'", errors, drv->hapd->iface->config_fname);

	wpa_printf(MSG_DEBUG, "realtek_read_priv_cfg ---");
	
	return errors;

}



static int realtek_read_wds_cfg(void *priv, struct rtk_wds_config* wds)
{

	struct realtek_driver_data *drv = priv;
	FILE *f;
	char buf[256], *pos;
	int errors = 0;
	int line = 0;

	wpa_printf(MSG_DEBUG, "realtek_read_wds_cfg +++");

	f = fopen(drv->hapd->iface->config_fname, "r");
	if (f == NULL) 
	{
		wpa_printf(MSG_ERROR, "Could not open configuration file '%s' "
		   			"for reading.", drv->hapd->iface->config_fname);
		return -1;
	}

	while (fgets(buf, sizeof(buf), f)) 
	{
		line++;
		if (buf[0] == '#')
			continue;
		pos = buf;
		while (*pos != '\0') 
		{
			if (*pos == '\n') 
			{
				*pos = '\0';
				break;
			}
			pos++;
		}
		
		if (buf[0] == '\0')
			continue;

		pos = os_strchr(buf, '=');
		if (pos == NULL) 
		{
			wpa_printf(MSG_ERROR, "Line %d: invalid line '%s'", line, buf);
			errors++;
			continue;
		}
		*pos = '\0';
		pos++;

		if (strcmp(buf, "wds_enable") == 0) {
			wds->wdsEnabled = atoi(pos);
		} 
		else if (strcmp(buf, "wds_num") == 0) {
			wds->wdsNum	= atoi(pos);
		} 
		else if (strcmp(buf, "wds_mac1") == 0) {
			if (hwaddr_aton(pos, wds->macAddr[0])) {
				wpa_printf(MSG_ERROR, "Line %d: invalid MAC address '%s'\n", line, pos);
				errors++;
				}
		} 
		else if (strcmp(buf, "wds_mac2") == 0) {
			if (hwaddr_aton(pos, wds->macAddr[1])) {
				wpa_printf(MSG_ERROR, "Line %d: invalid MAC address '%s'\n", line, pos);
				errors++;
				}
		} 
		else if (strcmp(buf, "wds_mac3") == 0) {
			if (hwaddr_aton(pos, wds->macAddr[2])) {
				wpa_printf(MSG_ERROR, "Line %d: invalid MAC address '%s'\n", line, pos);
				errors++;
				}
		} 
		else if (strcmp(buf, "wds_mac4") == 0) {
			if (hwaddr_aton(pos, wds->macAddr[3])) {
				wpa_printf(MSG_ERROR, "Line %d: invalid MAC address '%s'\n", line, pos);
				errors++;
				}
		} 
		else if (strcmp(buf, "wds_ssid") == 0) {
			int ssid_len = strlen(pos);
			if (ssid_len > HOSTAPD_MAX_SSID_LEN || ssid_len < 1) {
			wpa_printf(MSG_ERROR, "Line %d: invalid SSID '%s'\n", line, pos);
			errors++;
			} else {
			memcpy(wds->ssid, pos, ssid_len);
			wds->ssid[ssid_len] = '\0';
			} 
		}
		else if (strcmp(buf, "wds_enc_type") == 0) {
			wds->wdsPrivacy = atoi(pos);
		} 
		else if (strcmp(buf, "wds_wepkey") == 0) {
			int len = strlen(pos);
			int tmp_error = 0;
			
			//free(wds->wdsWepKey); 
			wds->wdsWepKey = NULL;
			wds->wdsWepKeyLen = 0;
							
			if (pos[0] == '"') {
				if (len < 2 || pos[len - 1] != '"')
				tmp_error++;
				else{
				len -= 2;
				if (len > 0) {
					wds->wdsWepKey = malloc(len);
					if (wds->wdsWepKey == NULL)
					tmp_error++;
					else
					memcpy(wds->wdsWepKey, pos + 1, len);
					}
				}
			}
			else
			{
			if (len & 1)
				tmp_error++;
			
				len /= 2;			
				if (len > 0) 
				{		
					wds->wdsWepKey = malloc(len);
					if (wds->wdsWepKey == NULL)
					tmp_error++;
								
					if (hexstr2bin(pos, wds->wdsWepKey, len) < 0) 
						tmp_error++;
				}
			}
					
			wds->wdsWepKeyLen = len;
					
			if(tmp_error)
				{
					wpa_printf(MSG_ERROR, "Line %d: invalid wds_wepkey '%s'\n", line, pos);
					errors++;
				}
		} 
		else if (strcmp(buf, "wds_passphrase") == 0) {
			int len = strlen(pos);
			//free(wds->wdsPskPassPhrase); 
			wds->wdsPskPassPhrase = NULL;
			if (len < 8 || len > 63) {
				wpa_printf(MSG_ERROR, "Line %d: invalid WPA passphrase length"
							" %d (expected 8..63)\n", line, len);
				errors++;
				} else {
				if ((wds->wdsPskPassPhrase = strdup(pos)) == NULL)
				errors++;
				}	
		}
		
	}

	fclose(f);

	if (errors)
		wpa_printf(MSG_ERROR, "%d errors found in configuration file "
		   	"'%s'", errors, drv->hapd->iface->config_fname);

	wpa_printf(MSG_DEBUG, "realtek_read_wds_cfg ---");
	
	return errors;

}

static int realtek_hapd_config(void *priv)
{
	struct realtek_driver_data *drv = priv;
	struct hostapd_data *hapd = drv->hapd;
	struct rtk_hapd_config *config=&rtk_config;
	struct rtk_wds_config wds;

	wpa_printf(MSG_DEBUG, "realtek_hapd_config +++");

	memset(config, 0, sizeof(struct rtk_hapd_config));
	
	config->is_hapd = 1; //RTK_WPAS
	
	if(realtek_read_hapd_cfg(hapd,priv, config))
		return -1;

	if(realtek_read_priv_cfg(priv, config))
		return -1;

#ifdef INBAND_CTRL //HOST_LITTLE_ENDIAN		
	//   rtk_hapd_config need to swap  , write a function to do it	
	rtk_cfg_to_bigEndian(config);
#endif

	if(set80211priv(drv->iface,drv, HAPD_IOCTL_SETCONFIG, config, sizeof(struct rtk_hapd_config))) 
	{
		wpa_printf(MSG_ERROR, "%s: Failed to set Configurations", __func__);
		return -1;
	}

	if(realtek_read_wds_cfg(priv, &wds)) //mark_mbssid , wds for vap ??
		return -1;

	/*
	if(realtek_set_wds(priv, wds))
		return -1;
	*/

	wpa_printf(MSG_DEBUG, "realtek_hapd_config ---");

	return 0;
	
}


#endif


const struct wpa_driver_ops wpa_driver_realtek_ops = {
	.name					= "realtek",
	.init					= realtek_init,
	.deinit					= realtek_deinit,
	.set_ieee8021x			= realtek_set_ieee8021x,
	.set_privacy			= realtek_set_privacy,
	.set_encryption			= realtek_set_key,
	.get_seqnum				= realtek_get_seqnum,
	.flush					= realtek_flush,
	.set_generic_elem		= realtek_set_opt_ie,
	.wireless_event_init	= realtek_wireless_event_init,
	.wireless_event_deinit	= realtek_wireless_event_deinit,
	.sta_set_flags			= realtek_sta_set_flags,
	//mark read_sta_data because both ralink & broadcom NOT support
	//.read_sta_data		= realtek_read_sta_driver_data,
	.send_eapol				= realtek_send_eapol,
	.sta_disassoc			= realtek_sta_disassoc,
	.sta_deauth				= realtek_sta_deauth,
#ifdef RTK_MBSSID	
	.bss_add = realtek_bss_add,
	.bss_remove = realtek_bss_remove,
#endif	
#ifdef RTK_HAPD
	.sta_remove				= realtek_sta_remove,
#endif
	.set_ssid				= realtek_set_ssid,
	.get_ssid				= realtek_get_ssid,
	.set_countermeasures	= realtek_set_countermeasures,
	//mark sta_clear_stats because both ralink & broadcom NOT support
	//.sta_clear_stats        = realtek_sta_clear_stats,
	.commit					= realtek_commit,
#ifdef EAP_WPS
	.set_wps_beacon_ie		= realtek_set_wps_beacon_ie,
	.set_wps_probe_resp_ie	= realtek_set_wps_probe_resp_ie,
	//.set_wps_assoc_resp_ie	= realtek_set_wps_assoc_resp_ie,
#endif /* EAP_WPS */
#ifdef RTK_HAPD
	.driver_on				= realtek_driver_on,
#endif
};

#ifdef MODIFIED_BY_SONY
int wext_set_key(void *priv, int alg,
				 const u8 *addr, int key_idx,
				 int set_tx, const u8 *seq, size_t seq_len,
				 const u8 *key, size_t key_len)
{
	struct realtek_driver_data *drv = priv;
	struct hostapd_data *hapd = drv->hapd;
	struct hostapd_bss_config *conf = hapd->conf;
	struct iwreq iwr;
	int ret = 0;
	int ioctl_sock;

	wpa_printf(MSG_DEBUG,"%s: alg=%d key_idx=%d set_tx=%d seq_len=%lu "
		   "key_len=%lu", __FUNCTION__, alg, key_idx, set_tx,
		   (unsigned long) seq_len, (unsigned long) key_len);

	ioctl_sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (ioctl_sock < 0) {
		perror("socket(PF_INET,SOCK_DGRAM)");
		return -1;
	}

	os_memset(&iwr, 0, sizeof(iwr));
	os_strncpy(iwr.ifr_name, drv->iface, IFNAMSIZ);
	iwr.u.encoding.flags = key_idx + 1;
	if (alg == WPA_ALG_NONE)
		iwr.u.encoding.flags |= IW_ENCODE_DISABLED;
	if (conf->auth_algs & HOSTAPD_AUTH_OPEN)
		iwr.u.encoding.flags |= IW_ENCODE_OPEN;
	if (conf->auth_algs & HOSTAPD_AUTH_SHARED_KEY)
		iwr.u.encoding.flags |= IW_ENCODE_RESTRICTED;
	iwr.u.encoding.pointer = (caddr_t) key;
	iwr.u.encoding.length = key_len;
#ifdef STAND_ALONE
	if (ioctl(ioctl_sock, SIOCSIWENCODE, &iwr) < 0) {
		perror("ioctl[SIOCSIWENCODE]");
		ret = -1;
	}
#endif
#if defined(INBAND_CTRL)
	INBAND_IOCTLPKT_DUMP(&iwr,sizeof(iwr));
	if (inband_ioctl(&drv->ioctl_ioh_obj,SIOCSIWENCODE, &iwr) < 0) {
		perror("inband_ioctl[SIOCSIWENCODE]");
		ret = -1;
	}
#endif

	if (set_tx && alg != WPA_ALG_NONE) {
		os_memset(&iwr, 0, sizeof(iwr));
		os_strncpy(iwr.ifr_name, drv->iface, IFNAMSIZ);
		iwr.u.encoding.flags = key_idx + 1;
		iwr.u.encoding.pointer = (caddr_t) key;
		iwr.u.encoding.length = 0;
#ifdef STAND_ALONE
		if (ioctl(ioctl_sock, SIOCSIWENCODE, &iwr) < 0) {
			perror("ioctl[SIOCSIWENCODE] (set_tx)");
			ret = -1;
		}
#endif
#if defined(INBAND_CTRL)
		INBAND_IOCTLPKT_DUMP(&iwr,sizeof(iwr));
		if (inband_ioctl(&drv->ioctl_ioh_obj,SIOCSIWENCODE, &iwr) < 0) {
			perror("inband_ioctl[SIOCSIWENCODE] (set_tx)");
			ret = -1;
		}
#endif

	}
	close(ioctl_sock);
	return ret;
}
#endif /* MODIFIED_BY_SONY */
