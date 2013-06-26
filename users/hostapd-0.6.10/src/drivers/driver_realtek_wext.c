/*
 * WPA Supplicant - driver interaction with generic Linux Wireless Extensions
 * Copyright (c) 2003-2007, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 *
 * This file implements a driver interface for the Linux Wireless Extensions.
 * When used with WE-18 or newer, this interface can be used as-is with number
 * of drivers. In addition to this, some of the common functions in this file
 * can be used by other driver interface implementations that use generic WE
 * ioctls, but require private ioctls for some of the functionality.
 */

#include "includes.h"
#include <sys/ioctl.h>
#include <net/if_arp.h>

#include "wireless_copy.h"
#include "common.h"
#include "driver.h"
#include "eloop.h"
#include "priv_netlink.h"
#include "driver_realtek_wext.h"
#include "ieee802_11_defs.h"
#include "wpa_common.h"

#include "driver_realtek.h"


#ifdef RTK_INBAND
#define INBAND_INTF		"br0"
#define INBAND_SLAVE	("001234567899")
#define INBAND_IOCTL_TYPE	0x8899
#define INBAND_NETLINK_TYPE 0x9000
#define INBAND_DEBUG 0
#define INBAND_IOCTLPKT_DUMP //hex_dump
#define IWREQ_LEN 32
#define INBAND_IOCTLTYPE_LEN	4
#define INBAND_IOCTLHDR_LEN	6
#define INBAND_PENDING_START(data) data+INBAND_IOCTLHDR_LEN+IWREQ_LEN
#define INBAND_IOCTLRET_PTR(data) data+INBAND_IOCTLTYPE_LEN
#define IOH_HDR_LEN sizeof(struct ioh_header)
#endif


static int wpa_driver_wext_flush_pmkid(void *priv);
static int wpa_driver_wext_get_range(void *priv);
static void wpa_driver_wext_finish_drv_init(struct wpa_driver_wext_data *drv);
static void wpa_driver_wext_disconnect(struct wpa_driver_wext_data *drv);


void convert_to_net (const char *name, void *data, int data_size)
{
	//_Eric ?? signed ?? unsigned ??
	
	if(data_size == 16)
	{	
		u16 *tmp = (u16 *)data;
		printf("htons +++ %s = 0x%x", name, *tmp);
		*tmp = htons(*tmp);
		printf("htons --- %s = 0x%x", name, *tmp);
	}
	else if(data_size == 32)
	{
		u32 *tmp = (u32 *)data;
		printf("htonl +++ %s = 0x%x", name, *tmp);
		*tmp = htonl(*tmp);
		printf("htonl --- %s = 0x%x", name, *tmp);
	}
	else
		printf("Unknown data type !!!! %s size = %d\n", name, data_size);
	
}


void convert_to_host (const char *name, void *data, int data_size)
{
	//_Eric ?? signed ?? unsigned ??
	
	if(data_size == 16)
	{	
		u16 *tmp = (u16 *)data;
		printf("ntohs +++ %s = 0x%x", name, *tmp);
		*tmp = ntohs(*tmp);
		printf("ntohs --- %s = 0x%x", name, *tmp);
	}
	else if(data_size == 32)
	{
		u32 *tmp = (u32 *)data;
		printf("ntohl +++ %s = 0x%x", name, *tmp);
		*tmp = ntohl(*tmp);
		printf("ntohl --- %s = 0x%x", name, *tmp);
	}
	else
		printf("Unknown data type !!!! %s size = %d\n", name, data_size);

}


//_Eric ??
static int wpa_driver_wext_send_oper_ifla(struct wpa_driver_wext_data *drv,
					  int linkmode, int operstate)
{
	struct {
		struct nlmsghdr hdr;
		struct ifinfomsg ifinfo;
		char opts[16];
	} req;
	struct rtattr *rta;
	static int nl_seq;
	ssize_t ret;

	os_memset(&req, 0, sizeof(req));

	req.hdr.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
	req.hdr.nlmsg_type = RTM_SETLINK;
	req.hdr.nlmsg_flags = NLM_F_REQUEST;
	req.hdr.nlmsg_seq = ++nl_seq;
	req.hdr.nlmsg_pid = 0;

	req.ifinfo.ifi_family = AF_UNSPEC;
	req.ifinfo.ifi_type = 0;
	req.ifinfo.ifi_index = drv->ifindex;
	req.ifinfo.ifi_flags = 0;
	req.ifinfo.ifi_change = 0;

	if (linkmode != -1) {
		rta = aliasing_hide_typecast(
			((char *) &req + NLMSG_ALIGN(req.hdr.nlmsg_len)),
			struct rtattr);
		rta->rta_type = IFLA_LINKMODE;
		rta->rta_len = RTA_LENGTH(sizeof(char));
		*((char *) RTA_DATA(rta)) = linkmode;
		req.hdr.nlmsg_len = NLMSG_ALIGN(req.hdr.nlmsg_len) +
			RTA_LENGTH(sizeof(char));
	}
	if (operstate != -1) {
		rta = (struct rtattr *)
			((char *) &req + NLMSG_ALIGN(req.hdr.nlmsg_len));
		rta->rta_type = IFLA_OPERSTATE;
		rta->rta_len = RTA_LENGTH(sizeof(char));
		*((char *) RTA_DATA(rta)) = operstate;
		req.hdr.nlmsg_len = NLMSG_ALIGN(req.hdr.nlmsg_len) +
			RTA_LENGTH(sizeof(char));
	}

	wpa_printf(MSG_DEBUG, "WEXT: Operstate: linkmode=%d, operstate=%d",
		   linkmode, operstate);

	ret = send(drv->event_sock, &req, req.hdr.nlmsg_len, 0); //_Eric ?? "Send" relates to driver or not ?
	if (ret < 0) {
		wpa_printf(MSG_DEBUG, "WEXT: Sending operstate IFLA failed: "
			   "%s (assume operstate is not supported)",
			   strerror(errno));
	}

	return ret < 0 ? -1 : 0;
}



/**
 * wpa_driver_wext_get_bssid - Get BSSID, SIOCGIWAP
 * @priv: Pointer to private wext data from wpa_driver_wext_init()
 * @bssid: Buffer for BSSID
 * Returns: 0 on success, -1 on failure
 */
int wpa_driver_wext_get_bssid(void *priv, u8 *bssid)
{
	struct wpa_driver_wext_data *drv = priv;
	struct iwreq iwr;
	int ret = 0;

	os_memset(&iwr, 0, sizeof(iwr));
	os_strlcpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);

#ifdef RTK_INBAND
	if (inband_ioctl(SIOCGIWAP, &iwr) < 0)
#else
	if (ioctl(drv->ioctl_sock, SIOCGIWAP, &iwr) < 0) 
#endif
	{
		perror("ioctl[SIOCGIWAP]");
		ret = -1;
	}
	os_memcpy(bssid, iwr.u.ap_addr.sa_data, ETH_ALEN);

	return ret;
}



/**
 * wpa_driver_wext_set_bssid - Set BSSID, SIOCSIWAP
 * @priv: Pointer to private wext data from wpa_driver_wext_init()
 * @bssid: BSSID
 * Returns: 0 on success, -1 on failure
 */
int wpa_driver_wext_set_bssid(void *priv, const u8 *bssid)
{
	struct wpa_driver_wext_data *drv = priv;
	struct iwreq iwr;
	int ret = 0;

	os_memset(&iwr, 0, sizeof(iwr));
	os_strlcpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);
	iwr.u.ap_addr.sa_family = ARPHRD_ETHER;
	if (bssid)
		os_memcpy(iwr.u.ap_addr.sa_data, bssid, ETH_ALEN);
	else
		os_memset(iwr.u.ap_addr.sa_data, 0, ETH_ALEN);

#ifdef RTK_INBAND
	if (inband_ioctl(SIOCSIWAP, &iwr) < 0)
#else
	if (ioctl(drv->ioctl_sock, SIOCSIWAP, &iwr) < 0) 
#endif
	{
		perror("ioctl[SIOCSIWAP]");
		ret = -1;
	}

	return ret;
}



/**
 * wpa_driver_wext_get_ssid - Get SSID, SIOCGIWESSID
 * @priv: Pointer to private wext data from wpa_driver_wext_init()
 * @ssid: Buffer for the SSID; must be at least 32 bytes long
 * Returns: SSID length on success, -1 on failure
 */
int wpa_driver_wext_get_ssid(void *priv, u8 *ssid)
{
	struct wpa_driver_wext_data *drv = priv;
	struct iwreq iwr;
	int ret = 0;

	os_memset(&iwr, 0, sizeof(iwr));
	os_strlcpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);
	iwr.u.essid.pointer = (caddr_t) ssid;
	iwr.u.essid.length = 32;

#ifdef RTK_INBAND
	if (inband_ioctl(SIOCGIWESSID, &iwr) < 0)
#else
	if (ioctl(drv->ioctl_sock, SIOCGIWESSID, &iwr) < 0)
#endif
	{
		perror("ioctl[SIOCGIWESSID]");
		ret = -1;
	} else {
		ret = iwr.u.essid.length;
		if (ret > 32)
			ret = 32;
		/* Some drivers include nul termination in the SSID, so let's
		 * remove it here before further processing. WE-21 changes this
		 * to explicitly require the length _not_ to include nul
		 * termination. */
		if (ret > 0 && ssid[ret - 1] == '\0' &&
		    drv->we_version_compiled < 21)
			ret--;
	}

	return ret;
}



/**
 * wpa_driver_wext_set_ssid - Set SSID, SIOCSIWESSID
 * @priv: Pointer to private wext data from wpa_driver_wext_init()
 * @ssid: SSID
 * @ssid_len: Length of SSID (0..32)
 * Returns: 0 on success, -1 on failure
 */
int wpa_driver_wext_set_ssid(void *priv, const u8 *ssid, size_t ssid_len)
{
	struct wpa_driver_wext_data *drv = priv;
	struct iwreq iwr;
	int ret = 0;
	char buf[33];

	printf("wpa_driver_wext_set_ssid ssid_len %d \n ", ssid_len);

	if (ssid_len > 32)
		return -1;

	os_memset(&iwr, 0, sizeof(iwr));
	os_strlcpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);
	/* flags: 1 = ESSID is active, 0 = not (promiscuous) */
	iwr.u.essid.flags = (ssid_len != 0);
	os_memset(buf, 0, sizeof(buf));
	os_memcpy(buf, ssid, ssid_len);
	iwr.u.essid.pointer = (caddr_t) buf;
	if (drv->we_version_compiled < 21) {
		/* For historic reasons, set SSID length to include one extra
		 * character, C string nul termination, even though SSID is
		 * really an octet string that should not be presented as a C
		 * string. Some Linux drivers decrement the length by one and
		 * can thus end up missing the last octet of the SSID if the
		 * length is not incremented here. WE-21 changes this to
		 * explicitly require the length _not_ to include nul
		 * termination. */
		if (ssid_len)
			ssid_len++;
	}
	iwr.u.essid.length = ssid_len;

	printf("wpa_driver_wext_set_ssid len %d \n ", iwr.u.essid.length);

#ifdef RTK_INBAND
	if (inband_ioctl(SIOCSIWESSID, &iwr) < 0)
#else
	if (ioctl(drv->ioctl_sock, SIOCSIWESSID, &iwr) < 0) 
#endif
	{
		perror("ioctl[SIOCSIWESSID]");
		ret = -1;
	}

	return ret;
}



static void
wpa_driver_wext_event_wireless_custom(void *ctx, char *custom, u16 flags, size_t len)
{
	union wpa_event_data data;
	
	os_memset(&data, 0, sizeof(data));
	
	wpa_printf(MSG_DEBUG, "custom event =%d, len=%d", flags, len);
	
	switch(flags)
	{
		case WPAS_MIC_FAILURE:
		{
			unsigned char * unicast = (unsigned char *)custom;
			data.michael_mic_failure.unicast = unicast[0];
			wpa_supplicant_event(ctx, EVENT_MICHAEL_MIC_FAILURE, &data);
			break;
		}
		case WPAS_ASSOC_INFO:
		{
			struct _WPAS_ASSOCIATION_INFO *assoc_info = (struct _WPAS_ASSOCIATION_INFO *) custom;

			data.assoc_info.req_ies = NULL;
			data.assoc_info.resp_ies = NULL;
			data.assoc_info.beacon_ies = NULL;

#ifdef RTK_INBAND_LE
			assoc_info->ReqIELen = ntohs(assoc_info->ReqIELen);
			assoc_info->RespIELen = ntohs(assoc_info->RespIELen);			
#endif

			if((assoc_info->ReqIELen == 0) && (assoc_info->RespIELen == 0))
				goto done;

			data.assoc_info.req_ies_len = assoc_info->ReqIELen;
			data.assoc_info.resp_ies_len = assoc_info->RespIELen;

			if(data.assoc_info.req_ies_len > 0)
			{
				data.assoc_info.req_ies = os_malloc(assoc_info->ReqIELen);
			
				if (data.assoc_info.req_ies == NULL)
					goto done;
				
				os_memcpy(data.assoc_info.req_ies, assoc_info->ReqIE, assoc_info->ReqIELen);
			}

			
			if(data.assoc_info.resp_ies_len > 0)
			{

				data.assoc_info.resp_ies = os_malloc(assoc_info->RespIELen);
				if (data.assoc_info.resp_ies == NULL)
					goto done;
				
				os_memcpy(data.assoc_info.resp_ies, assoc_info->RespIE, assoc_info->RespIELen);
			}

			wpa_supplicant_event(ctx, EVENT_ASSOCINFO, &data);
	
done:

			if(data.assoc_info.req_ies_len > 0)
				os_free(data.assoc_info.req_ies);
			
			if(data.assoc_info.resp_ies_len > 0)
				os_free(data.assoc_info.resp_ies);
	
			break;
		}
		default:
			break;
	
	}
	
}



static int wpa_driver_wext_event_wireless_michaelmicfailure(
	void *ctx, const char *ev, size_t len)
{
	const struct iw_michaelmicfailure *mic;
	union wpa_event_data data;

	if (len < sizeof(*mic))
		return -1;

	mic = (const struct iw_michaelmicfailure *) ev;

	wpa_printf(MSG_DEBUG, "Michael MIC failure wireless event: "
		   "flags=0x%x src_addr=" MACSTR, mic->flags,
		   MAC2STR(mic->src_addr.sa_data));

	os_memset(&data, 0, sizeof(data));
	data.michael_mic_failure.unicast = !(mic->flags & IW_MICFAILURE_GROUP);
	wpa_supplicant_event(ctx, EVENT_MICHAEL_MIC_FAILURE, &data);

	return 0;
}



static void wpa_driver_wext_event_assoc_ies(struct wpa_driver_wext_data *drv)
{
	union wpa_event_data data;

	if (drv->assoc_req_ies == NULL && drv->assoc_resp_ies == NULL)
		return;

	os_memset(&data, 0, sizeof(data));
	if (drv->assoc_req_ies) {
		data.assoc_info.req_ies = drv->assoc_req_ies;
		drv->assoc_req_ies = NULL;
		data.assoc_info.req_ies_len = drv->assoc_req_ies_len;
	}
	if (drv->assoc_resp_ies) {
		data.assoc_info.resp_ies = drv->assoc_resp_ies;
		drv->assoc_resp_ies = NULL;
		data.assoc_info.resp_ies_len = drv->assoc_resp_ies_len;
	}

	wpa_supplicant_event(drv->ctx, EVENT_ASSOCINFO, &data);

	os_free(data.assoc_info.req_ies);
	os_free(data.assoc_info.resp_ies);
}



static void wpa_driver_wext_event_wireless(struct wpa_driver_wext_data *drv,
					   void *ctx, char *data, int len)
{
	struct iw_event iwe_buf, *iwe = &iwe_buf;
	char *pos, *end, *custom, *buf;

	pos = data;
	end = data + len;

	while (pos + IW_EV_LCP_LEN <= end) {
		/* Event data may be unaligned, so make a local, aligned copy
		 * before processing. */
		os_memcpy(&iwe_buf, pos, IW_EV_LCP_LEN);

#ifdef RTK_INBAND_LE		
		iwe->len = ntohs(iwe->len); 	
		iwe->cmd = ntohs(iwe->cmd);
#endif
		
		wpa_printf(MSG_DEBUG, "Wireless event: cmd=0x%x len=%d",
			   iwe->cmd, iwe->len);

		if (iwe->len <= IW_EV_LCP_LEN)
			return;

		custom = pos + IW_EV_POINT_LEN;
		if (drv->we_version_compiled > 18 &&
		    (iwe->cmd == IWEVMICHAELMICFAILURE ||
		     iwe->cmd == IWEVCUSTOM ||
		     iwe->cmd == IWEVASSOCREQIE ||
		     iwe->cmd == IWEVASSOCRESPIE ||
		     iwe->cmd == IWEVPMKIDCAND)) {
			/* WE-19 removed the pointer from struct iw_point */
			char *dpos = (char *) &iwe_buf.u.data.length;
			int dlen = dpos - (char *) &iwe_buf;
			os_memcpy(dpos, pos + IW_EV_LCP_LEN,
				  sizeof(struct iw_event) - dlen);
		} else {
			os_memcpy(&iwe_buf, pos, sizeof(struct iw_event));
			custom += IW_EV_POINT_OFF;
			
#ifdef RTK_INBAND_LE    		        
			iwe->len = ntohs(iwe->len); 					   
			iwe->cmd = ntohs(iwe->cmd);
#endif

		}

		switch (iwe->cmd) {
		case SIOCGIWAP: //_Eric ?? sa_data is get from parsing ??
			wpa_printf(MSG_DEBUG, "Wireless event: new AP: "
				   MACSTR,
				   MAC2STR((u8 *) iwe->u.ap_addr.sa_data));
			if (is_zero_ether_addr(
				    (const u8 *) iwe->u.ap_addr.sa_data) ||
			    os_memcmp(iwe->u.ap_addr.sa_data,
				      "\x44\x44\x44\x44\x44\x44", ETH_ALEN) ==
			    0) {
				os_free(drv->assoc_req_ies);
				drv->assoc_req_ies = NULL;
				os_free(drv->assoc_resp_ies);
				drv->assoc_resp_ies = NULL;
				wpa_supplicant_event(ctx, EVENT_DISASSOC,
						     NULL);
			
			} else {
				wpa_driver_wext_event_assoc_ies(drv);
				wpa_supplicant_event(ctx, EVENT_ASSOC, NULL);
			}
			break;
		case IWEVCUSTOM:

#ifdef RTK_INBAND_LE			
			iwe->u.data.length = ntohs(iwe->u.data.length); 		
			iwe->u.data.flags = ntohs(iwe->u.data.flags);
#endif	

			if (custom + iwe->u.data.length > end) {
				wpa_printf(MSG_DEBUG, "WEXT: Invalid "
					   "IWEVCUSTOM length");
				return;
			}
			buf = os_malloc(iwe->u.data.length + 1);
			if (buf == NULL)
				return;
			os_memcpy(buf, custom, iwe->u.data.length);
			buf[iwe->u.data.length] = '\0';
			wpa_driver_wext_event_wireless_custom(ctx, buf, iwe->u.data.flags, iwe->u.data.length);
			os_free(buf);
			break;
		case SIOCGIWSCAN:
			drv->scan_complete_events = 1;
			eloop_cancel_timeout(wpa_driver_wext_scan_timeout,
					     drv, ctx);
			wpa_supplicant_event(ctx, EVENT_SCAN_RESULTS, NULL);
			break;
		}

		pos += iwe->len;
	}
}


static void wpa_driver_wext_event_rtm_newlink(struct wpa_driver_wext_data *drv,
					      void *ctx, struct nlmsghdr *h,
					      size_t len)
{
	struct ifinfomsg *ifi;
	int attrlen, nlmsg_len, rta_len;
	struct rtattr * attr;

	if (len < sizeof(*ifi))
		return;

	ifi = NLMSG_DATA(h);

#ifdef RTK_INBAND_LE
	ifi->ifi_index = ntohl(ifi->ifi_index);
#endif	


	if (drv->ifindex != ifi->ifi_index)
	 {
		wpa_printf(MSG_DEBUG, "Ignore event for foreign ifindex %d, drv->ifindex %d",
			   ifi->ifi_index, drv->ifindex);
		//_Eric ??  return;
	 }

	wpa_printf(MSG_DEBUG, "RTM_NEWLINK: operstate=%d ifi_flags=0x%x "
		   "(%s%s%s%s)",
		   drv->operstate, ifi->ifi_flags,
		   (ifi->ifi_flags & IFF_UP) ? "[UP]" : "",
		   (ifi->ifi_flags & IFF_RUNNING) ? "[RUNNING]" : "",
		   (ifi->ifi_flags & IFF_LOWER_UP) ? "[LOWER_UP]" : "",
		   (ifi->ifi_flags & IFF_DORMANT) ? "[DORMANT]" : "");

	nlmsg_len = NLMSG_ALIGN(sizeof(struct ifinfomsg));

	attrlen = h->nlmsg_len - nlmsg_len;
	if (attrlen < 0)
		return;

	attr = (struct rtattr *) (((char *) ifi) + nlmsg_len);

#ifdef RTK_INBAND_LE	
	attr->rta_len = ntohs(attr->rta_len);	
	attr->rta_type = ntohs(attr->rta_type);
#endif

	rta_len = RTA_ALIGN(sizeof(struct rtattr));
	while (RTA_OK(attr, attrlen)) 
		{
			if (attr->rta_type == IFLA_WIRELESS) 
			{
				wpa_driver_wext_event_wireless(
				drv, ctx, ((char *) attr) + rta_len,
				attr->rta_len - rta_len);
			} 
			
			attr = RTA_NEXT(attr, attrlen);
			
#ifdef RTK_INBAND_LE 
			attr->rta_len = ntohs(attr->rta_len);			
			attr->rta_type = ntohs(attr->rta_type);
#endif

		}
	
}



static void wpa_driver_wext_event_receive(int sock, void *eloop_ctx,
					  void *sock_ctx)
{
	char buf[8192];
	int left;
	struct sockaddr_nl from;
	socklen_t fromlen;
	struct nlmsghdr *h;
	int max_events = 10;
	struct wpa_driver_wext_data *drv = eloop_ctx;

try_again:

#ifdef RTK_INBAND
	left = ioh_recv(&drv->netlink_ioh_obj, 3000);

	if (left < 0) {
		perror("recvfrom(rawsock)");
		return;
	}
	
	//hex_dump(drv->netlink_ioh_obj.rx_data,left);
	
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
		
#ifdef RTK_INBAND_LE
		h->nlmsg_len = ntohl(h->nlmsg_len); 		
		h->nlmsg_type = ntohs(h->nlmsg_type);			
		h->nlmsg_flags = ntohs(h->nlmsg_flags); 		
		h->nlmsg_seq = ntohl(h->nlmsg_seq); 		
		h->nlmsg_pid = ntohl(h->nlmsg_pid);
#endif


		len = h->nlmsg_len;
		plen = len - sizeof(*h);
		if (len > left || plen < 0) {
			wpa_printf(MSG_DEBUG, "Malformed netlink message: "
				   "len=%d left=%d plen=%d",
				   len, left, plen);
			break;
		}

		switch (h->nlmsg_type) {
		case RTM_NEWLINK:
			wpa_driver_wext_event_rtm_newlink(eloop_ctx, sock_ctx,
							  h, plen);
			break;
		}

		len = NLMSG_ALIGN(len);
		left -= len;
		h = (struct nlmsghdr *) ((char *) h + len);
	}

	if (left > 0) {
		wpa_printf(MSG_DEBUG, "%d extra bytes in the end of netlink "
			   "message", left);
	}

	if (--max_events > 0) {
		/*
		 * Try to receive all events in one eloop call in order to
		 * limit race condition on cases where AssocInfo event, Assoc
		 * event, and EAPOL frames are received more or less at the
		 * same time. We want to process the event messages first
		 * before starting EAPOL processing.
		 */
		goto try_again;
	}
}



static int wpa_driver_wext_get_ifflags_ifname(struct wpa_driver_wext_data *drv,
					      const char *ifname, int *flags)
{
	struct ifreq ifr;

	os_memset(&ifr, 0, sizeof(ifr));
	os_strlcpy(ifr.ifr_name, ifname, IFNAMSIZ);

#ifdef RTK_INBAND
	if (inband_ioctl(SIOCGIFFLAGS, (caddr_t) &ifr) < 0)
#else
	if (ioctl(drv->ioctl_sock, SIOCGIFFLAGS, (caddr_t) &ifr) < 0) 
#endif
	{
		perror("ioctl[SIOCGIFFLAGS]");
		return -1;
	}
	*flags = ifr.ifr_flags & 0xffff;
	return 0;
}



/**
 * wpa_driver_wext_get_ifflags - Get interface flags (SIOCGIFFLAGS)
 * @drv: driver_wext private data
 * @flags: Pointer to returned flags value
 * Returns: 0 on success, -1 on failure
 */
int wpa_driver_wext_get_ifflags(struct wpa_driver_wext_data *drv, int *flags)
{
	return wpa_driver_wext_get_ifflags_ifname(drv, drv->ifname, flags);
}



static int wpa_driver_wext_set_ifflags_ifname(struct wpa_driver_wext_data *drv,
					      const char *ifname, int flags)
{
	struct ifreq ifr;

	os_memset(&ifr, 0, sizeof(ifr));
	os_strlcpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ifr.ifr_flags = flags & 0xffff;

#ifdef RTK_INBAND
	if (inband_ioctl(SIOCSIFFLAGS, (caddr_t) &ifr) < 0)
#else
	if (ioctl(drv->ioctl_sock, SIOCSIFFLAGS, (caddr_t) &ifr) < 0) 
#endif
	{
		perror("SIOCSIFFLAGS");
		return -1;
	}
	return 0;
}



/**
 * wpa_driver_wext_set_ifflags - Set interface flags (SIOCSIFFLAGS)
 * @drv: driver_wext private data
 * @flags: New value for flags
 * Returns: 0 on success, -1 on failure
 */
int wpa_driver_wext_set_ifflags(struct wpa_driver_wext_data *drv, int flags)
{
	return wpa_driver_wext_set_ifflags_ifname(drv, drv->ifname, flags);
}



/**
 * wpa_driver_wext_init - Initialize WE driver interface
 * @ctx: context to be used when calling wpa_supplicant functions,
 * e.g., wpa_supplicant_event()
 * @ifname: interface name, e.g., wlan0
 * Returns: Pointer to private data, %NULL on failure
 */
void * wpa_driver_wext_init(void *ctx, const char *ifname)
{
	int s;
	struct sockaddr_nl local;
	struct wpa_driver_wext_data *drv;

	drv = os_zalloc(sizeof(*drv));
	if (drv == NULL)
		return NULL;
	drv->ctx = ctx;
	os_strlcpy(drv->ifname, ifname, sizeof(drv->ifname));

	drv->ioctl_sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (drv->ioctl_sock < 0) {
		perror("socket(PF_INET,SOCK_DGRAM)");
		os_free(drv);
		return NULL;
	}


#ifdef RTK_INBAND
	s = ioh_open(&drv->netlink_ioh_obj,INBAND_INTF,INBAND_SLAVE,INBAND_NETLINK_TYPE,INBAND_DEBUG);
	if (s < 0) {
		perror("socket(PF_PACKET,SOCK_RAW,INBAND_NETLINK_TYPE)");
		return -1;
	}
#else
	s = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (s < 0) {
		perror("socket(PF_NETLINK,SOCK_RAW,NETLINK_ROUTE)");
		close(drv->ioctl_sock);
		os_free(drv);
		return NULL;
	}

	os_memset(&local, 0, sizeof(local));
	local.nl_family = AF_NETLINK;
	local.nl_groups = RTMGRP_LINK;
	if (bind(s, (struct sockaddr *) &local, sizeof(local)) < 0) {
		perror("bind(netlink)");
		close(s);
		close(drv->ioctl_sock);
		os_free(drv);
		return NULL;
	}
#endif

#ifdef RTK_INBAND
	eloop_register_read_sock(drv->netlink_ioh_obj.sockfd, wpa_driver_wext_event_receive, drv, ctx);
	drv->event_sock = drv->netlink_ioh_obj.sockfd;
#else
	eloop_register_read_sock(s, wpa_driver_wext_event_receive, drv, ctx);
	drv->event_sock = s;
#endif

	drv->mlme_sock = -1;

	wpa_driver_wext_finish_drv_init(drv);

	return drv;
}



static void wpa_driver_wext_finish_drv_init(struct wpa_driver_wext_data *drv)
{
	int flags;

	if (wpa_driver_wext_get_ifflags(drv, &flags) != 0)
		printf("Could not get interface '%s' flags\n", drv->ifname);

	if (wpa_driver_wext_set_ifflags(drv, flags &= ~IFF_UP) != 0) 
		printf("Could not set interface '%s' DOWN\n", drv->ifname);
	
	if (wpa_driver_wext_set_mode(drv, 0) < 0) 
		printf("Could not configure driver to use managed mode\n");

	if (wpa_driver_wext_set_ifflags(drv, flags | IFF_UP) != 0)
		printf("Could not set interface '%s' UP\n", drv->ifname);

	wpa_driver_wext_get_range(drv);

	wpa_driver_wext_disconnect(drv);

	drv->ifindex = if_nametoindex(drv->ifname);

}



/**
 * wpa_driver_wext_deinit - Deinitialize WE driver interface
 * @priv: Pointer to private wext data from wpa_driver_wext_init()
 *
 * Shut down driver interface and processing of driver events. Free
 * private data buffer if one was allocated in wpa_driver_wext_init().
 */
void wpa_driver_wext_deinit(void *priv)
{
	struct wpa_driver_wext_data *drv = priv;
	int flags;

	eloop_cancel_timeout(wpa_driver_wext_scan_timeout, drv, drv->ctx);

	/*
	 * Clear possibly configured driver parameters in order to make it
	 * easier to use the driver after wpa_supplicant has been terminated.
	 */
	wpa_driver_wext_disconnect(drv);

	wpa_driver_wext_send_oper_ifla(priv, 0, IF_OPER_UP);

	eloop_unregister_read_sock(drv->event_sock);
	if (drv->mlme_sock >= 0)
		eloop_unregister_read_sock(drv->mlme_sock);

	if (wpa_driver_wext_get_ifflags(drv, &flags) == 0)
		(void) wpa_driver_wext_set_ifflags(drv, flags & ~IFF_UP);

	close(drv->event_sock);
	close(drv->ioctl_sock);
	if (drv->mlme_sock >= 0)
		close(drv->mlme_sock);
	os_free(drv->assoc_req_ies);
	os_free(drv->assoc_resp_ies);
	os_free(drv);
}



/**
 * wpa_driver_wext_scan_timeout - Scan timeout to report scan completion
 * @eloop_ctx: Unused
 * @timeout_ctx: ctx argument given to wpa_driver_wext_init()
 *
 * This function can be used as registered timeout when starting a scan to
 * generate a scan completed event if the driver does not report this.
 */
void wpa_driver_wext_scan_timeout(void *eloop_ctx, void *timeout_ctx)
{
	wpa_printf(MSG_DEBUG, "Scan timeout - try to get results");
	wpa_supplicant_event(timeout_ctx, EVENT_SCAN_RESULTS, NULL);
}



/**
 * wpa_driver_wext_scan - Request the driver to initiate scan
 * @priv: Pointer to private wext data from wpa_driver_wext_init()
 * @ssid: Specific SSID to scan for (ProbeReq) or %NULL to scan for
 *	all SSIDs (either active scan with broadcast SSID or passive
 *	scan
 * @ssid_len: Length of the SSID
 * Returns: 0 on success, -1 on failure
 */
int wpa_driver_wext_scan(void *priv, const u8 *ssid, size_t ssid_len)
{
	struct wpa_driver_wext_data *drv = priv;
	struct iwreq iwr;
	int ret = 0, timeout;
	struct iw_scan_req req;

	if (ssid_len > IW_ESSID_MAX_SIZE) {
		wpa_printf(MSG_DEBUG, "%s: too long SSID (%lu)",
			   __FUNCTION__, (unsigned long) ssid_len);
		return -1;
	}

	os_memset(&iwr, 0, sizeof(iwr));
	os_strlcpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);

	if (ssid && ssid_len) {
		os_memset(&req, 0, sizeof(req));
		req.essid_len = ssid_len;
		req.bssid.sa_family = ARPHRD_ETHER;
		os_memset(req.bssid.sa_data, 0xff, ETH_ALEN);
		os_memcpy(req.essid, ssid, ssid_len);
		iwr.u.data.pointer = (caddr_t) &req;
		iwr.u.data.length = sizeof(req);
		iwr.u.data.flags = IW_SCAN_THIS_ESSID;
	}

#ifdef RTK_INBAND
	if (inband_ioctl(SIOCSIWSCAN, &iwr) < 0)
#else
	if (ioctl(drv->ioctl_sock, SIOCSIWSCAN, &iwr) < 0) 
#endif
	{
		perror("ioctl[SIOCSIWSCAN]");
		ret = -1;
	}

	/* Not all drivers generate "scan completed" wireless event, so try to
	 * read results after a timeout. */
	timeout = 5;
	if (drv->scan_complete_events) {
		/*
		 * The driver seems to deliver SIOCGIWSCAN events to notify
		 * when scan is complete, so use longer timeout to avoid race
		 * conditions with scanning and following association request.
		 */
		timeout = 30;
	}
	wpa_printf(MSG_DEBUG, "Scan requested (ret=%d) - scan timeout %d "
		   "seconds", ret, timeout);
	eloop_cancel_timeout(wpa_driver_wext_scan_timeout, drv, drv->ctx);
	eloop_register_timeout(timeout, 0, wpa_driver_wext_scan_timeout, drv,
			       drv->ctx);

	return ret;
}



static u8 * wpa_driver_wext_giwscan(struct wpa_driver_wext_data *drv,
				    size_t *len)
{
	struct iwreq iwr;
	u8 *res_buf;
	size_t res_buf_len;

	res_buf_len = IW_SCAN_MAX_DATA;
	for (;;) {
		res_buf = os_malloc(res_buf_len);
		if (res_buf == NULL)
			return NULL;
		os_memset(&iwr, 0, sizeof(iwr));
		os_strlcpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);
		iwr.u.data.pointer = res_buf;
		iwr.u.data.length = res_buf_len;

#ifdef RTK_INBAND
		if (inband_ioctl(SIOCGIWSCAN, &iwr) == 0)
#else
		if (ioctl(drv->ioctl_sock, SIOCGIWSCAN, &iwr) == 0)
#endif
			break;

		if (errno == E2BIG && res_buf_len < 65535) {
			os_free(res_buf);
			res_buf = NULL;
			res_buf_len *= 2;
			if (res_buf_len > 65535)
				res_buf_len = 65535; /* 16-bit length field */
			wpa_printf(MSG_DEBUG, "Scan results did not fit - "
				   "trying larger buffer (%lu bytes)",
				   (unsigned long) res_buf_len);
		} else {
			perror("ioctl[SIOCGIWSCAN]");
			os_free(res_buf);
			return NULL;
		}
	}

	if (iwr.u.data.length > res_buf_len) {
		os_free(res_buf);
		return NULL;
	}
	*len = iwr.u.data.length;

	return res_buf;
}



/*
 * Data structure for collecting WEXT scan results. This is needed to allow
 * the various methods of reporting IEs to be combined into a single IE buffer.
 */
struct wext_scan_data {
	struct wpa_scan_res res;
	u8 *ie;
	size_t ie_len;
	u8 ssid[32];
	size_t ssid_len;
	int maxrate;
};



static void wext_get_scan_mode(struct iw_event *iwe,
			       struct wext_scan_data *res)
{
	if (iwe->u.mode == IW_MODE_ADHOC)
		res->res.caps |= IEEE80211_CAP_IBSS;
	else if (iwe->u.mode == IW_MODE_MASTER || iwe->u.mode == IW_MODE_INFRA)
		res->res.caps |= IEEE80211_CAP_ESS;
}



static void wext_get_scan_ssid(struct iw_event *iwe,
			       struct wext_scan_data *res, char *custom,
			       char *end)
{
	int ssid_len = iwe->u.essid.length;
	if (custom + ssid_len > end)
		return;
	if (iwe->u.essid.flags &&
	    ssid_len > 0 &&
	    ssid_len <= IW_ESSID_MAX_SIZE) {
		os_memcpy(res->ssid, custom, ssid_len);
		res->ssid_len = ssid_len;
	}
}



static void wext_get_scan_freq(struct iw_event *iwe,
			       struct wext_scan_data *res)
{
	int divi = 1000000, i;

	if (iwe->u.freq.e == 0) {
		/*
		 * Some drivers do not report frequency, but a channel.
		 * Try to map this to frequency by assuming they are using
		 * IEEE 802.11b/g.  But don't overwrite a previously parsed
		 * frequency if the driver sends both frequency and channel,
		 * since the driver may be sending an A-band channel that we
		 * don't handle here.
		 */

		if (res->res.freq)
			return;

		if (iwe->u.freq.m >= 1 && iwe->u.freq.m <= 13) {
			res->res.freq = 2407 + 5 * iwe->u.freq.m;
			return;
		} else if (iwe->u.freq.m == 14) {
			res->res.freq = 2484;
			return;
		}
	}

	if (iwe->u.freq.e > 6) {
		wpa_printf(MSG_DEBUG, "Invalid freq in scan results (BSSID="
			   MACSTR " m=%d e=%d)",
			   MAC2STR(res->res.bssid), iwe->u.freq.m,
			   iwe->u.freq.e);
		return;
	}

	for (i = 0; i < iwe->u.freq.e; i++)
		divi /= 10;
	res->res.freq = iwe->u.freq.m / divi;
}



static void wext_get_scan_qual(struct iw_event *iwe,
			       struct wext_scan_data *res)
{
	res->res.qual = iwe->u.qual.qual;
	res->res.noise = iwe->u.qual.noise;
	res->res.level = iwe->u.qual.level;
}



static void wext_get_scan_encode(struct iw_event *iwe,
				 struct wext_scan_data *res)
{
	if (!(iwe->u.data.flags & IW_ENCODE_DISABLED))
		res->res.caps |= IEEE80211_CAP_PRIVACY;
}


static void wext_get_scan_rate(struct iw_event *iwe,
			       struct wext_scan_data *res, char *pos,
			       char *end)
{
	int maxrate;
	char *custom = pos + IW_EV_LCP_LEN;
	struct iw_param p;
	size_t clen;

	clen = iwe->len;
	if (custom + clen > end)
		return;
	maxrate = 0;
	while (((ssize_t) clen) >= (ssize_t) sizeof(struct iw_param)) {
		/* Note: may be misaligned, make a local, aligned copy */
		os_memcpy(&p, custom, sizeof(struct iw_param));
		if (p.value > maxrate)
			maxrate = p.value;
		clen -= sizeof(struct iw_param);
		custom += sizeof(struct iw_param);
	}

	/* Convert the maxrate from WE-style (b/s units) to
	 * 802.11 rates (500000 b/s units).
	 */
	res->maxrate = maxrate / 500000;
}



static void wext_get_scan_iwevgenie(struct iw_event *iwe,
				    struct wext_scan_data *res, char *custom,
				    char *end)
{
	char *genie, *gpos, *gend;
	u8 *tmp;

	if (iwe->u.data.length == 0)
		return;

	gpos = genie = custom;
	gend = genie + iwe->u.data.length;
	if (gend > end) {
		wpa_printf(MSG_INFO, "IWEVGENIE overflow");
		return;
	}

	tmp = os_realloc(res->ie, res->ie_len + gend - gpos);
	if (tmp == NULL)
		return;
	os_memcpy(tmp + res->ie_len, gpos, gend - gpos);
	res->ie = tmp;
	res->ie_len += gend - gpos;
}



static void wext_get_scan_custom(struct iw_event *iwe,
				 struct wext_scan_data *res, char *custom,
				 char *end)
{
	size_t clen;
	u8 *tmp;

	clen = iwe->u.data.length;
	if (custom + clen > end)
		return;

	if (clen > 7 && os_strncmp(custom, "wpa_ie=", 7) == 0) {
		char *spos;
		int bytes;
		spos = custom + 7;
		bytes = custom + clen - spos;
		if (bytes & 1 || bytes == 0)
			return;
		bytes /= 2;
		tmp = os_realloc(res->ie, res->ie_len + bytes);
		if (tmp == NULL)
			return;
		hexstr2bin(spos, tmp + res->ie_len, bytes);
		res->ie = tmp;
		res->ie_len += bytes;
	} else if (clen > 7 && os_strncmp(custom, "rsn_ie=", 7) == 0) {
		char *spos;
		int bytes;
		spos = custom + 7;
		bytes = custom + clen - spos;
		if (bytes & 1 || bytes == 0)
			return;
		bytes /= 2;
		tmp = os_realloc(res->ie, res->ie_len + bytes);
		if (tmp == NULL)
			return;
		hexstr2bin(spos, tmp + res->ie_len, bytes);
		res->ie = tmp;
		res->ie_len += bytes;
	} else if (clen > 4 && os_strncmp(custom, "tsf=", 4) == 0) {
		char *spos;
		int bytes;
		u8 bin[8];
		spos = custom + 4;
		bytes = custom + clen - spos;
		if (bytes != 16) {
			wpa_printf(MSG_INFO, "Invalid TSF length (%d)", bytes);
			return;
		}
		bytes /= 2;
		hexstr2bin(spos, bin, bytes);
		res->res.tsf += WPA_GET_BE64(bin);
	}
}



static int wext_19_iw_point(struct wpa_driver_wext_data *drv, u16 cmd)
{
	return drv->we_version_compiled > 18 &&
		(cmd == SIOCGIWESSID || cmd == SIOCGIWENCODE ||
		 cmd == IWEVGENIE || cmd == IWEVCUSTOM);
}



static void wpa_driver_wext_add_scan_entry(struct wpa_scan_results *res,
					   struct wext_scan_data *data)
{
	struct wpa_scan_res **tmp;
	struct wpa_scan_res *r;
	size_t extra_len;
	u8 *pos, *end, *ssid_ie = NULL, *rate_ie = NULL;

	/* Figure out whether we need to fake any IEs */
	pos = data->ie;
	end = pos + data->ie_len;
	while (pos && pos + 1 < end) {
		if (pos + 2 + pos[1] > end)
			break;
		if (pos[0] == WLAN_EID_SSID)
			ssid_ie = pos;
		else if (pos[0] == WLAN_EID_SUPP_RATES)
			rate_ie = pos;
		else if (pos[0] == WLAN_EID_EXT_SUPP_RATES)
			rate_ie = pos;
		pos += 2 + pos[1];
	}

	extra_len = 0;
	if (ssid_ie == NULL)
		extra_len += 2 + data->ssid_len;
	if (rate_ie == NULL && data->maxrate)
		extra_len += 3;

	r = os_zalloc(sizeof(*r) + extra_len + data->ie_len);
	if (r == NULL)
		return;
	os_memcpy(r, &data->res, sizeof(*r));
	r->ie_len = extra_len + data->ie_len;
	pos = (u8 *) (r + 1);
	if (ssid_ie == NULL) {
		/*
		 * Generate a fake SSID IE since the driver did not report
		 * a full IE list.
		 */
		*pos++ = WLAN_EID_SSID;
		*pos++ = data->ssid_len;
		os_memcpy(pos, data->ssid, data->ssid_len);
		pos += data->ssid_len;
	}
	if (rate_ie == NULL && data->maxrate) {
		/*
		 * Generate a fake Supported Rates IE since the driver did not
		 * report a full IE list.
		 */
		*pos++ = WLAN_EID_SUPP_RATES;
		*pos++ = 1;
		*pos++ = data->maxrate;
	}
	if (data->ie)
		os_memcpy(pos, data->ie, data->ie_len);

	tmp = os_realloc(res->res,
			 (res->num + 1) * sizeof(struct wpa_scan_res *));
	if (tmp == NULL) {
		os_free(r);
		return;
	}
	tmp[res->num++] = r;
	res->res = tmp;
}

					  

/**
 * wpa_driver_wext_get_scan_results - Fetch the latest scan results
 * @priv: Pointer to private wext data from wpa_driver_wext_init()
 * Returns: Scan results on success, -1 on failure
 */
struct wpa_scan_results * wpa_driver_wext_get_scan_results(void *priv)
{
	struct wpa_driver_wext_data *drv = priv;
	size_t ap_num = 0, len;
	int first;
	u8 *res_buf;
	struct iw_event iwe_buf, *iwe = &iwe_buf;
	char *pos, *end, *custom;
	struct wpa_scan_results *res;
	struct wext_scan_data data;
#ifdef RTK_INBAND_LE
	unsigned char cur_ap_num = 0;
	unsigned char prev_ap_num = 0;
#endif


	res_buf = wpa_driver_wext_giwscan(drv, &len);
	
	if (res_buf == NULL)
		return NULL;

	ap_num = 0;
	first = 1;

	res = os_zalloc(sizeof(*res));
	if (res == NULL) {
		os_free(res_buf);
		return NULL;
	}

	pos = (char *) res_buf;
	end = (char *) res_buf + len;
	os_memset(&data, 0, sizeof(data));

	while (pos + IW_EV_LCP_LEN <= end) {
		/* Event data may be unaligned, so make a local, aligned copy
		 * before processing. */
		os_memcpy(&iwe_buf, pos, IW_EV_LCP_LEN);

#ifdef RTK_INBAND_LE
		iwe->len = ntohs(iwe->len);
		iwe->cmd = ntohs(iwe->cmd);
#endif

		if (iwe->len <= IW_EV_LCP_LEN)
			break;

		custom = pos + IW_EV_POINT_LEN;
		if (wext_19_iw_point(drv, iwe->cmd)) {

#ifdef RTK_INBAND_LE
		//iwe_buf.u.data.length = ntohs(iwe_buf.u.data.length);
		//iwe_buf.u.data.flags = ntohs(iwe_buf.u.data.flags);	
#endif

			/* WE-19 removed the pointer from struct iw_point */
			char *dpos = (char *) &iwe_buf.u.data.length;
			int dlen = dpos - (char *) &iwe_buf;
			os_memcpy(dpos, pos + IW_EV_LCP_LEN,
				  sizeof(struct iw_event) - dlen);
		} else {
			os_memcpy(&iwe_buf, pos, sizeof(struct iw_event));
			custom += IW_EV_POINT_OFF;

#ifdef RTK_INBAND_LE
			iwe->len = ntohs(iwe->len);
			iwe->cmd = ntohs(iwe->cmd);
#endif
		}


#ifdef RTK_INBAND_LE

		if(cur_ap_num > prev_ap_num)
		{

			struct wext_scan_data* scan_data = (struct wext_scan_data*)&data;

			if(first)
				scan_data->ie_len = ntohl(scan_data->ie_len);

			scan_data->ssid_len = ntohl(scan_data->ssid_len);
			scan_data->maxrate = ntohl(scan_data->maxrate);
			scan_data->res.freq = ntohl(scan_data->res.freq);
			scan_data->res.beacon_int= ntohs(scan_data->res.beacon_int);
			scan_data->res.caps = ntohs(scan_data->res.caps);
			scan_data->res.qual = ntohl(scan_data->res.qual);
			scan_data->res.noise = ntohl(scan_data->res.noise);
			scan_data->res.level = ntohl(scan_data->res.level);
			scan_data->res.tsf = ntohll(scan_data->res.tsf);
			scan_data->res.ie_len = ntohl(scan_data->res.ie_len);

			prev_ap_num = cur_ap_num;

		}
	
		iwe_buf.u.data.length = ntohs(iwe_buf.u.data.length);
		iwe_buf.u.data.flags = ntohs(iwe_buf.u.data.flags); 
	
#endif

		switch (iwe->cmd) {
		case SIOCGIWAP:

#ifdef RTK_INBAND_LE
			if (!first)
				cur_ap_num ++; 
#endif
			
			if (!first)
				wpa_driver_wext_add_scan_entry(res, &data);
			
			first = 0;
			os_free(data.ie);

			os_memset(&data, 0, sizeof(data));
			
			os_memcpy(data.res.bssid,
				  iwe->u.ap_addr.sa_data, ETH_ALEN);

#ifdef RTK_INBAND_LE //_Eric ?? ????
			data.res.bssid[2] = iwe->u.ap_addr.sa_data[3];
			data.res.bssid[3] = iwe->u.ap_addr.sa_data[2];
			data.res.bssid[4] = iwe->u.ap_addr.sa_data[5];
			data.res.bssid[5] = iwe->u.ap_addr.sa_data[4];
#endif
			
			break;
		case SIOCGIWMODE:
			wext_get_scan_mode(iwe, &data);
			break;
		case SIOCGIWESSID:
			wext_get_scan_ssid(iwe, &data, custom, end);
			break;
		case SIOCGIWFREQ:
			wext_get_scan_freq(iwe, &data);
			break;
		case IWEVQUAL:
			wext_get_scan_qual(iwe, &data);
			break;
		case SIOCGIWENCODE:
			wext_get_scan_encode(iwe, &data);
			break;
		case SIOCGIWRATE: //_Eric ?? rate genie custom & NO name
			wext_get_scan_rate(iwe, &data, pos, end);
			break;
		case IWEVGENIE:
			wext_get_scan_iwevgenie(iwe, &data, custom, end);
			break;
		case IWEVCUSTOM:
			wext_get_scan_custom(iwe, &data, custom, end);
			break;
		}

		pos += iwe->len;
	}
	os_free(res_buf);
	res_buf = NULL;
	if (!first)
		wpa_driver_wext_add_scan_entry(res, &data);
	os_free(data.ie);

	wpa_printf(MSG_DEBUG, "Received %lu bytes of scan results (%lu BSSes)",
		   (unsigned long) len, (unsigned long) res->num);

	return res;
}



static int wpa_driver_wext_get_range(void *priv)
{
	struct wpa_driver_wext_data *drv = priv;
	struct iw_range *range;
	struct iwreq iwr;
	int minlen;
	size_t buflen;

	/*
	 * Use larger buffer than struct iw_range in order to allow the
	 * structure to grow in the future.
	 */
	buflen = sizeof(struct iw_range) + 500;
	range = os_zalloc(buflen);
	if (range == NULL)
		return -1;

	os_memset(&iwr, 0, sizeof(iwr));
	os_strlcpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);
	iwr.u.data.pointer = (caddr_t) range;
	iwr.u.data.length = buflen;

	minlen = ((char *) &range->enc_capa) - (char *) range +
		sizeof(range->enc_capa);

#ifdef RTK_INBAND
	if (inband_ioctl(SIOCGIWRANGE, &iwr) < 0)
#else
	if (ioctl(drv->ioctl_sock, SIOCGIWRANGE, &iwr) < 0) 
#endif
	{
		perror("ioctl[SIOCGIWRANGE]");
		os_free(range);
		return -1;
	}

#ifdef RTK_INBAND_LE
	//_Eric ?? Only convert what used below
	range->enc_capa = ntohl(range->enc_capa);
#endif

	if (iwr.u.data.length >= minlen &&
		  range->we_version_compiled >= 18) {
		wpa_printf(MSG_DEBUG, "SIOCGIWRANGE: WE(compiled)=%d "
			   "WE(source)=%d enc_capa=0x%x",
			   range->we_version_compiled,
			   range->we_version_source,
			   range->enc_capa);
		drv->has_capability = 1;
		drv->we_version_compiled = range->we_version_compiled;
		if (range->enc_capa & IW_ENC_CAPA_WPA) {
			drv->capa.key_mgmt |= WPA_DRIVER_CAPA_KEY_MGMT_WPA |
				WPA_DRIVER_CAPA_KEY_MGMT_WPA_PSK;
		}
		if (range->enc_capa & IW_ENC_CAPA_WPA2) {
			drv->capa.key_mgmt |= WPA_DRIVER_CAPA_KEY_MGMT_WPA2 |
				WPA_DRIVER_CAPA_KEY_MGMT_WPA2_PSK;
		}
		drv->capa.enc |= WPA_DRIVER_CAPA_ENC_WEP40 |
			WPA_DRIVER_CAPA_ENC_WEP104;
		if (range->enc_capa & IW_ENC_CAPA_CIPHER_TKIP)
			drv->capa.enc |= WPA_DRIVER_CAPA_ENC_TKIP;
		if (range->enc_capa & IW_ENC_CAPA_CIPHER_CCMP)
			drv->capa.enc |= WPA_DRIVER_CAPA_ENC_CCMP;
		if (range->enc_capa & IW_ENC_CAPA_4WAY_HANDSHAKE)
			drv->capa.flags |= WPA_DRIVER_FLAGS_4WAY_HANDSHAKE;
		drv->capa.auth = WPA_DRIVER_AUTH_OPEN |
			WPA_DRIVER_AUTH_SHARED |
			WPA_DRIVER_AUTH_LEAP;

		wpa_printf(MSG_DEBUG, "  capabilities: key_mgmt 0x%x enc 0x%x "
			   "flags 0x%x",
			   drv->capa.key_mgmt, drv->capa.enc, drv->capa.flags);
	} else {
		wpa_printf(MSG_DEBUG, "SIOCGIWRANGE: too old (short) data - "
			   "assuming WPA is not supported");
	}

	os_free(range);
	return 0;
}



static void wpa_driver_wext_disconnect(struct wpa_driver_wext_data *drv)
{
	struct iwreq iwr;
	const u8 null_bssid[ETH_ALEN] = { 0, 0, 0, 0, 0, 0 };
	u8 ssid[32];
	int i;

	/*
	 * Only force-disconnect when the card is in infrastructure mode,
	 * otherwise the driver might interpret the cleared BSSID and random
	 * SSID as an attempt to create a new ad-hoc network.
	 */
	os_memset(&iwr, 0, sizeof(iwr));
	os_strlcpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);

#ifdef RTK_INBAND
	if (inband_ioctl(SIOCGIWMODE, &iwr) < 0)
#else
	if (ioctl(drv->ioctl_sock, SIOCGIWMODE, &iwr) < 0) 
#endif
	{
		perror("ioctl[SIOCGIWMODE]");
		iwr.u.mode = IW_MODE_INFRA;
	}

	if (iwr.u.mode == IW_MODE_INFRA) {
		/*
		 * Clear the BSSID selection and set a random SSID to make sure
		 * the driver will not be trying to associate with something
		 * even if it does not understand SIOCSIWMLME commands (or
		 * tries to associate automatically after deauth/disassoc).
		 */
		wpa_driver_wext_set_bssid(drv, null_bssid);

		for (i = 0; i < 32; i++)
			ssid[i] = rand() & 0xFF;
		wpa_driver_wext_set_ssid(drv, ssid, 32);
	}
}



/**
 * wpa_driver_wext_set_mode - Set wireless mode (infra/adhoc), SIOCSIWMODE
 * @priv: Pointer to private wext data from wpa_driver_wext_init()
 * @mode: 0 = infra/BSS (associate with an AP), 1 = adhoc/IBSS
 * Returns: 0 on success, -1 on failure
 */
int wpa_driver_wext_set_mode(void *priv, int mode)
{
	struct wpa_driver_wext_data *drv = priv;
	struct iwreq iwr;
	int ret = -1, flags;
	unsigned int new_mode = mode ? IW_MODE_ADHOC : IW_MODE_INFRA;

	os_memset(&iwr, 0, sizeof(iwr));
	os_strlcpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);
	iwr.u.mode = new_mode;

#ifdef RTK_INBAND
	if (inband_ioctl(SIOCSIWMODE, &iwr) == 0)
#else
	if (ioctl(drv->ioctl_sock, SIOCSIWMODE, &iwr) == 0) 
#endif
	{
		ret = 0;
		goto done;
	}

	if (errno != EBUSY) {
		perror("ioctl[SIOCSIWMODE]");
		goto done;
	}

	/* mac80211 doesn't allow mode changes while the device is up, so if
	 * the device isn't in the mode we're about to change to, take device
	 * down, try to set the mode again, and bring it back up.
	 */

#ifdef RTK_INBAND
	if (inband_ioctl(SIOCGIWMODE, &iwr) < 0)
#else
	if (ioctl(drv->ioctl_sock, SIOCGIWMODE, &iwr) < 0)
#endif
	{
		perror("ioctl[SIOCGIWMODE]");
		goto done;
	}

	if (iwr.u.mode == new_mode) {
		ret = 0;
		goto done;
	}

	if (wpa_driver_wext_get_ifflags(drv, &flags) == 0) {
		(void) wpa_driver_wext_set_ifflags(drv, flags & ~IFF_UP);

		/* Try to set the mode again while the interface is down */
		iwr.u.mode = new_mode;

#ifdef RTK_INBAND
		if (inband_ioctl(SIOCSIWMODE, &iwr) < 0)
#else
		if (ioctl(drv->ioctl_sock, SIOCSIWMODE, &iwr) < 0)
#endif
			perror("ioctl[SIOCSIWMODE]");
		else
			ret = 0;

		/* Ignore return value of get_ifflags to ensure that the device
		 * is always up like it was before this function was called.
		 */
		(void) wpa_driver_wext_get_ifflags(drv, &flags);
		(void) wpa_driver_wext_set_ifflags(drv, flags | IFF_UP);
	}

done:
	return ret;
}



int wpa_driver_wext_set_operstate(void *priv, int state)
{
	struct wpa_driver_wext_data *drv = priv;

	wpa_printf(MSG_DEBUG, "%s: operstate %d->%d (%s)",
		   __func__, drv->operstate, state, state ? "UP" : "DORMANT");
	drv->operstate = state;
	return wpa_driver_wext_send_oper_ifla(
		drv, -1, state ? IF_OPER_UP : IF_OPER_DORMANT);
}



const struct wpa_driver_ops wpa_driver_wext_ops = {
	.name = "wext",
	.desc = "Linux wireless extensions (generic)",
	.get_bssid = wpa_driver_wext_get_bssid,
	.get_ssid = wpa_driver_wext_get_ssid,
	//.set_wpa = wpa_driver_wext_set_wpa,
	//.set_key = wpa_driver_wext_set_key,
	//.set_countermeasures = wpa_driver_wext_set_countermeasures,
	//.set_drop_unencrypted = wpa_driver_wext_set_drop_unencrypted,
	.scan = wpa_driver_wext_scan,
	.get_scan_results2 = wpa_driver_wext_get_scan_results,
	//.deauthenticate = wpa_driver_wext_deauthenticate,
	//.disassociate = wpa_driver_wext_disassociate,
	//.set_mode = wpa_driver_wext_set_mode,
	//.associate = wpa_driver_wext_associate,
	//.set_auth_alg = wpa_driver_wext_set_auth_alg,
	.init = wpa_driver_wext_init,
	.deinit = wpa_driver_wext_deinit,
	//.add_pmkid = wpa_driver_wext_add_pmkid,
	//.remove_pmkid = wpa_driver_wext_remove_pmkid,
	//.flush_pmkid = wpa_driver_wext_flush_pmkid,
	//.get_capa = wpa_driver_wext_get_capa,
	.set_operstate = wpa_driver_wext_set_operstate,
};
