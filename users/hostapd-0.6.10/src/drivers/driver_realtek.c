/*
 * WPA Supplicant - driver interaction with realtek 802.11 driver
 * Copyright (c) 2004, Sam Leffler <sam@errno.com>
 * Copyright (c) 2004-2005, Jouni Malinen <j@w1.fi>
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
 * Please note that realtek supports WPA configuration via Linux wireless
 * extensions and if the kernel includes support for this, driver_wext.c should
 * be used instead of this driver wrapper.
 */

#include "includes.h"
#include <sys/ioctl.h>

#include "common.h"
#include "driver.h"
#include "driver_realtek_wext.h"
#include "eloop.h"
#include "ieee802_11_defs.h"
#include "wireless_copy.h"

#include "driver_realtek.h"

#include "../wpa_supplicant/config.h"
#include "../wpa_supplicant/wpa_supplicant_i.h"
#include "../wpa_supplicant/wps_supplicant.h"


/*
 * Avoid conflicts with wpa_supplicant definitions by undefining a definition.
 */
#undef WME_OUI_TYPE

//#include <include/compat.h>
#include <net80211/ieee80211.h>
#ifdef WME_NUM_AC
/* Assume this is built against BSD branch of realtek driver. */
#define realtek_BSD
#include <net80211/_ieee80211.h>
#endif /* WME_NUM_AC */
#include <net80211/ieee80211_crypto.h>
#include <net80211/ieee80211_ioctl.h>


#ifdef IEEE80211_IOCTL_SETWMMPARAMS
/* Assume this is built against realtek-ng */
#define realtek_NG
#endif /* IEEE80211_IOCTL_SETWMMPARAMS */

struct wpa_driver_realtek_data {
	void *wext; /* private data for driver_wext */
	void *ctx;
	char ifname[IFNAMSIZ + 1];
	int sock;
};

static int
set80211priv(struct wpa_driver_realtek_data *drv, int op, void *data, int len,
	     int show_err)
{
	struct iwreq iwr;

	printf("Wpa_supplicant: %s +++\n", __FUNCTION__);

	os_memset(&iwr, 0, sizeof(iwr));
	os_strlcpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);
	if (len < IFNAMSIZ &&
	    op != IEEE80211_IOCTL_SET_APPIEBUF) {
		/*
		 * Argument data fits inline; put it there.
		 */
		os_memcpy(iwr.u.name, data, len);
	} else {
		/*
		 * Argument data too big for inline transfer; setup a
		 * parameter block instead; the kernel will transfer
		 * the data for the driver.
		 */
		iwr.u.data.pointer = data;
		iwr.u.data.length = len;
	}


#ifdef RTK_INBAND
	if (inband_ioctl(op, &iwr) < 0)
#else
	if (ioctl(drv->sock, op, &iwr) < 0)
#endif
	{
		if (show_err) {
#ifdef realtek_NG
			int first = IEEE80211_IOCTL_SETPARAM;
			int last = IEEE80211_IOCTL_KICKMAC;
			static const char *opnames[] = {
				"ioctl[IEEE80211_IOCTL_SETPARAM]",
				"ioctl[IEEE80211_IOCTL_GETPARAM]",
				"ioctl[IEEE80211_IOCTL_SETMODE]",
				"ioctl[IEEE80211_IOCTL_GETMODE]",
				"ioctl[IEEE80211_IOCTL_SETWMMPARAMS]",
				"ioctl[IEEE80211_IOCTL_GETWMMPARAMS]",
				"ioctl[IEEE80211_IOCTL_SETCHANLIST]",
				"ioctl[IEEE80211_IOCTL_GETCHANLIST]",
				"ioctl[IEEE80211_IOCTL_CHANSWITCH]",
				NULL,
				"ioctl[IEEE80211_IOCTL_SET_APPIEBUF]",
				"ioctl[IEEE80211_IOCTL_GETSCANRESULTS]",
				NULL,
				"ioctl[IEEE80211_IOCTL_GETCHANINFO]",
				"ioctl[IEEE80211_IOCTL_SETOPTIE]",
				"ioctl[IEEE80211_IOCTL_GETOPTIE]",
				"ioctl[IEEE80211_IOCTL_SETMLME]",
				NULL,
				"ioctl[IEEE80211_IOCTL_SETKEY]",
				NULL,
				"ioctl[IEEE80211_IOCTL_DELKEY]",
				NULL,
				"ioctl[IEEE80211_IOCTL_ADDMAC]",
				NULL,
				"ioctl[IEEE80211_IOCTL_DELMAC]",
				NULL,
				"ioctl[IEEE80211_IOCTL_WDSMAC]",
				NULL,
				"ioctl[IEEE80211_IOCTL_WDSDELMAC]",
				NULL,
				"ioctl[IEEE80211_IOCTL_KICKMAC]",
			};
#else /* realtek_NG */
			int first = IEEE80211_IOCTL_SETPARAM;
			int last = IEEE80211_IOCTL_CHANLIST;
			static const char *opnames[] = {
				"ioctl[IEEE80211_IOCTL_SETPARAM]",
				"ioctl[IEEE80211_IOCTL_GETPARAM]",
				"ioctl[IEEE80211_IOCTL_SETKEY]",
				"ioctl[IEEE80211_IOCTL_GETKEY]",
				"ioctl[IEEE80211_IOCTL_DELKEY]",
				NULL,
				"ioctl[IEEE80211_IOCTL_SETMLME]",
				NULL,
				"ioctl[IEEE80211_IOCTL_SETOPTIE]",
				"ioctl[IEEE80211_IOCTL_GETOPTIE]",
				"ioctl[IEEE80211_IOCTL_ADDMAC]",
				NULL,
				"ioctl[IEEE80211_IOCTL_DELMAC]",
				NULL,
				"ioctl[IEEE80211_IOCTL_CHANLIST]",
			};
#endif /* realtek_NG */
			int idx = op - first;
			if (first <= op && op <= last &&
			    idx < (int) (sizeof(opnames) / sizeof(opnames[0]))
			    && opnames[idx])
				perror(opnames[idx]);
			else
				perror("ioctl[unknown???]");
		}
		return -1;
	}
	return 0;
}

static int
set80211param(struct wpa_driver_realtek_data *drv, int op, int arg,
	      int show_err)
{
	struct iwreq iwr;

	printf("Wpa_supplicant: %s +++\n", __FUNCTION__);

#ifdef RTK_INBAND_LE	
   	op  = htonl(op); 
	arg = htonl(arg);
#endif

	os_memset(&iwr, 0, sizeof(iwr));
	os_strlcpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);
	iwr.u.mode = op;
	os_memcpy(iwr.u.name+sizeof(u32), &arg, sizeof(arg));


#ifdef RTK_INBAND
	if (inband_ioctl(IEEE80211_IOCTL_SETPARAM, &iwr) < 0)
#else
	if (ioctl(drv->sock, IEEE80211_IOCTL_SETPARAM, &iwr) < 0)
#endif
	{
		if (show_err) 
			perror("ioctl[IEEE80211_IOCTL_SETPARAM]");
		return -1;
	}

	return 0;
}


static int
realtek_configure_wpa(struct wpa_driver_realtek_data *drv, struct wpa_driver_associate_params *params)
{
	int cipher = 0; 
	int wpa = 0;
	int psk = 0;

	wpa_printf(MSG_DEBUG, "realtek_configure_wpa +++ pairwise: 0x%x key_mgmt: 0x%x", 
					params->pairwise_suite, params->key_mgmt_suite);

	//wpa_hexdump(999, "wpa_ie", params->wpa_ie, params->wpa_ie_len); //_Eric
	
	if (params->pairwise_suite & CIPHER_CCMP)
	cipher |= 1<<IEEE80211_CIPHER_AES_CCM;
	if (params->pairwise_suite & CIPHER_TKIP)
	cipher |= 1<<IEEE80211_CIPHER_TKIP;
	if (params->pairwise_suite & CIPHER_NONE)
	cipher |= 1<<IEEE80211_CIPHER_NONE;

	if(params->wpa_ie[0] == 0x30)
		wpa = 2;
	else if (params->wpa_ie[0] == 0xdd)
		{
			if(params->wpa_ie[5] == 0x1)
				wpa = 1;
			else if(params->wpa_ie[5] == 0x4) //for WPS
				return wpa_driver_realtek_set_wpa_ie(drv, params->wpa_ie, params->wpa_ie_len);
		}
	
	wpa_printf(MSG_DEBUG, "wpa:%d, pairwise: 0x%x, cipher:0x%x", wpa, params->pairwise_suite, params->key_mgmt_suite);

	if(params->key_mgmt_suite & KEY_MGMT_PSK)
		{//PSK mode, set PSK & cipher
		
			if (set80211param(drv, IEEE80211_PARAM_KEYMGTALGS, wpa, 1)) 
				{
					wpa_printf(MSG_ERROR, "Unable to set key management algorithms");
					return -1;
				}	
			
			if (set80211param(drv, IEEE80211_PARAM_UCASTCIPHERS, cipher, 1)) 
				{
					wpa_printf(MSG_ERROR, "Unable to set pairwise key ciphers");
					return -1;
				}
	
		}
	else
		{//Enterprise mode, Disable PSK & set cipher.	
			if (set80211param(drv, IEEE80211_PARAM_KEYMGTALGS, 0, 1)) 
				{
					wpa_printf(MSG_ERROR, "Unable to set key management algorithms");
					return -1;
				}	
			if (set80211param(drv, IEEE80211_PARAM_UCASTCIPHERS, cipher, 1)) 
				{
					wpa_printf(MSG_ERROR, "Unable to set pairwise key ciphers");
					return -1;
				}
	
		}
	
	if (set80211param(drv, IEEE80211_PARAM_WPA, wpa, 1)) 
		{
			wpa_printf(MSG_ERROR, "Unable to set WPA");
			return -1;
		}

	if (set80211param(drv, IEEE80211_PARAM_AUTHMODE, IEEE80211_AUTH_WPA, 1))
		{
			wpa_printf(MSG_ERROR, "Unable to Authmode");
			return -1;
		}

	return wpa_driver_realtek_set_wpa_ie(drv, params->wpa_ie, params->wpa_ie_len);
					
		
}


static int
realtek_config_security(struct wpa_driver_realtek_data *drv, struct wpa_driver_associate_params *params)
{
	int wep_keyidx = params->wep_tx_keyidx;
	int wep_keylen = params->wep_key_len[wep_keyidx];

	printf("Wpa_supplicant: %s +++\n", __FUNCTION__);

	if(set80211param(drv, IEEE80211_PARAM_UCASTKEYLEN, wep_keylen, 1))
		return -1;

	if(params->wep_key_len[wep_keyidx] > 0)
	{

		struct rtk_wpas_config config;

		wpa_printf(MSG_DEBUG, "wep_key_len %d", params->wep_key_len[wep_keyidx]);
		
		memset(&config, 0, sizeof(config));

		config.type = WPAS_CONFIG_WEPKEY; 

		config.wep_keyidx = wep_keyidx;
		config.wep_keylen = wep_keylen;

		memcpy(config.wep_key, params->wep_key[wep_keyidx], wep_keylen);

#ifdef RTK_INBAND_LE
		config.wep_keyidx = htonl(config.wep_keyidx);
		config.wep_keylen = htonl(config.wep_keylen);
#endif
	
		if(set80211priv(drv, WPAS_IOCTL_CUSTOM, &config, sizeof(config), 1)) 
		{
			wpa_printf(MSG_ERROR, "%s: Failed to set Configurations", __func__);
			return -1;
		}
	}

	if (params->wpa_ie_len == 0) 
	{		
		wpa_driver_realtek_set_wpa_ie(drv, params->wpa_ie, params->wpa_ie_len);
	
		wpa_printf(MSG_DEBUG, "set WEP: auth_alg %d", params->auth_alg);
		/* Set interface up flags after setting authentication modes,
			done atlast in realtek_commit() */					
		if(params->auth_alg == AUTH_ALG_OPEN_SYSTEM)
			return set80211param(drv, IEEE80211_PARAM_AUTHMODE, IEEE80211_AUTH_OPEN, 1);		
		else if(params->auth_alg == AUTH_ALG_SHARED_KEY)
			return set80211param(drv, IEEE80211_PARAM_AUTHMODE, IEEE80211_AUTH_SHARED, 1);
		else if(params->auth_alg == 0x3)
			return set80211param(drv, IEEE80211_PARAM_AUTHMODE, IEEE80211_AUTH_AUTO, 1);
		else if(params->auth_alg == 0x8) //RTK_WPAS, add auth_algs=BIT(3) as value of none authentication. 
			return set80211param(drv, IEEE80211_PARAM_AUTHMODE, IEEE80211_AUTH_NONE, 1);
	}

	if (params->key_mgmt_suite & KEY_MGMT_802_1X )
	{
		wpa_printf(MSG_DEBUG, "set 8021X");
		if(set80211param(drv, IEEE80211_PARAM_AUTHMODE, IEEE80211_AUTH_8021X, 1))
			return -1;
	}
	else if (params->wpa_ie_len > 0 ) 
	{
		return realtek_configure_wpa(drv, params);
	}
	else
	{
		printf("No 802.1X or WPA enabled!");
		return -1;
	}

	return 0;

}

//_Eric ?? static int error on PC ??
int
wpa_driver_realtek_set_wpa_ie(struct wpa_driver_realtek_data *drv, const u8 *wpa_ie, size_t wpa_ie_len)
{
	struct iwreq iwr;

	printf("Wpa_supplicant: %s +++\n", __FUNCTION__);

	os_memset(&iwr, 0, sizeof(iwr));
	os_strlcpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);
	/* NB: SETOPTIE is not fixed-size so must not be inlined */
	iwr.u.data.pointer = (void *) wpa_ie;
	iwr.u.data.length = wpa_ie_len;

#ifdef RTK_INBAND
	if (inband_ioctl(IEEE80211_IOCTL_SETOPTIE, &iwr) < 0)
#else
	if (ioctl(drv->sock, IEEE80211_IOCTL_SETOPTIE, &iwr) < 0) 
#endif
	{
		perror("ioctl[IEEE80211_IOCTL_SETOPTIE]");
		return -1;
	}
	
	return 0;
}

static int
wpa_driver_realtek_del_key(struct wpa_driver_realtek_data *drv, int key_idx,
			   const u8 *addr)
{
	struct ieee80211req_del_key wk;

	printf("Wpa_supplicant: %s +++\n", __FUNCTION__);

	wpa_printf(MSG_DEBUG, "%s: keyidx=%d", __FUNCTION__, key_idx);
	os_memset(&wk, 0, sizeof(wk));

	wk.idk_keyix = key_idx;

	if (addr != NULL)
		os_memcpy(wk.idk_macaddr, addr, IEEE80211_ADDR_LEN);

	return set80211priv(drv, IEEE80211_IOCTL_DELKEY, &wk, sizeof(wk), 1);
	
}

static int
wpa_driver_realtek_set_key(void *priv, wpa_alg alg,
			   const u8 *addr, int key_idx, int set_tx,
			   const u8 *seq, size_t seq_len,
			   const u8 *key, size_t key_len)
{
	struct wpa_driver_realtek_data *drv = priv;
	struct ieee80211req_key wk;
	char *alg_name;
	u_int8_t cipher;


	if (alg == WPA_ALG_NONE)
		return wpa_driver_realtek_del_key(drv, key_idx, addr);

	printf("Wpa_supplicant: %s +++\n", __FUNCTION__);

	switch (alg) {
	case WPA_ALG_WEP:
		alg_name = "WEP";
		cipher = IEEE80211_CIPHER_WEP;
		break;
	case WPA_ALG_TKIP:
		alg_name = "TKIP";
		cipher = IEEE80211_CIPHER_TKIP;
		break;
	case WPA_ALG_CCMP:
		alg_name = "CCMP";
		cipher = IEEE80211_CIPHER_AES_CCM;
		break;
	default:
		wpa_printf(MSG_DEBUG, "%s: unknown/unsupported algorithm %d",
			__FUNCTION__, alg);
		return -1;
	}

	wpa_printf(MSG_DEBUG, "%s: alg=%s key_idx=%d set_tx=%d seq_len=%lu "
		   "key_len=%lu", __FUNCTION__, alg_name, key_idx, set_tx,
		   (unsigned long) seq_len, (unsigned long) key_len);

	if (seq_len > sizeof(u_int64_t)) {
		wpa_printf(MSG_DEBUG, "%s: seq_len %lu too big",
			   __FUNCTION__, (unsigned long) seq_len);
		return -2;
	}
	if (key_len > sizeof(wk.ik_keydata)) {
		wpa_printf(MSG_DEBUG, "%s: key length %lu too big",
			   __FUNCTION__, (unsigned long) key_len);
		return -3;
	}

	os_memset(&wk, 0, sizeof(wk));
	wk.ik_type = cipher;
	wk.ik_flags = IEEE80211_KEY_RECV;
	if (addr == NULL ||
	    os_memcmp(addr, "\xff\xff\xff\xff\xff\xff", ETH_ALEN) == 0)
		wk.ik_flags |= IEEE80211_KEY_GROUP;
	if (set_tx) {
		wk.ik_flags |= IEEE80211_KEY_XMIT | IEEE80211_KEY_DEFAULT;
		os_memcpy(wk.ik_macaddr, addr, IEEE80211_ADDR_LEN);
	} else
		os_memset(wk.ik_macaddr, 0, IEEE80211_ADDR_LEN);
	wk.ik_keyix = key_idx;
	wk.ik_keylen = key_len;
#ifdef WORDS_BIGENDIAN //_Eric ?? Endian problem ? HAPD not identify ?
#define WPA_KEY_RSC_LEN 8
	{
		size_t i;
		u8 tmp[WPA_KEY_RSC_LEN];
		os_memset(tmp, 0, sizeof(tmp));
		for (i = 0; i < seq_len; i++)
			tmp[WPA_KEY_RSC_LEN - i - 1] = seq[i];
		os_memcpy(&wk.ik_keyrsc, tmp, WPA_KEY_RSC_LEN);
	}
#else /* WORDS_BIGENDIAN */
	os_memcpy(&wk.ik_keyrsc, seq, seq_len);
#endif /* WORDS_BIGENDIAN */

	os_memcpy(wk.ik_keydata, key, key_len);

#ifdef RTK_INBAND_LE
	wk.ik_keyix = htons(wk.ik_keyix);
	wk.ik_keyrsc = htonll(wk.ik_keyrsc);
#endif

	return set80211priv(drv, IEEE80211_IOCTL_SETKEY, &wk, sizeof(wk), 1);
}

static int
wpa_driver_realtek_set_countermeasures(void *priv, int enabled)
{
	struct wpa_driver_realtek_data *drv = priv;

	printf("Wpa_supplicant: %s +++\n", __FUNCTION__);
	wpa_printf(MSG_DEBUG, "%s: enabled=%d", __FUNCTION__, enabled);
	return set80211param(drv, IEEE80211_PARAM_COUNTERMEASURES, enabled, 1);
}


static int
wpa_driver_realtek_set_drop_unencrypted(void *priv, int enabled)
{
	struct wpa_driver_realtek_data *drv = priv;

	printf("Wpa_supplicant: %s +++\n", __FUNCTION__);
	wpa_printf(MSG_DEBUG, "%s: enabled=%d", __FUNCTION__, enabled);
	return set80211param(drv, IEEE80211_PARAM_DROPUNENCRYPTED, enabled, 1);
}

static int
wpa_driver_realtek_deauthenticate(void *priv, const u8 *addr, int reason_code)
{
	struct wpa_driver_realtek_data *drv = priv;
	struct ieee80211req_mlme mlme;

	printf("Wpa_supplicant: %s +++\n", __FUNCTION__);
	wpa_printf(MSG_DEBUG, "%s", __FUNCTION__);
	mlme.im_op = IEEE80211_MLME_DEAUTH;
	mlme.im_reason = reason_code;
	os_memcpy(mlme.im_macaddr, addr, IEEE80211_ADDR_LEN);

#ifdef RTK_INBAND_LE
	mlme.im_reason = htons(mlme.im_reason);
#endif
	return set80211priv(drv, IEEE80211_IOCTL_SETMLME, &mlme, sizeof(mlme), 1);
}

static int
wpa_driver_realtek_disassociate(void *priv, const u8 *addr, int reason_code)
{
	struct wpa_driver_realtek_data *drv = priv;
	struct ieee80211req_mlme mlme;

	printf("Wpa_supplicant: %s +++\n", __FUNCTION__);

	wpa_printf(MSG_DEBUG, "%s", __FUNCTION__);
	mlme.im_op = IEEE80211_MLME_DISASSOC;
	mlme.im_reason = reason_code;
	os_memcpy(mlme.im_macaddr, addr, IEEE80211_ADDR_LEN);
	
#ifdef RTK_INBAND_LE
		mlme.im_reason = htons(mlme.im_reason);
#endif
	return set80211priv(drv, IEEE80211_IOCTL_SETMLME, &mlme, sizeof(mlme), 1);
}

static int
wpa_driver_realtek_associate(void *priv,
			     struct wpa_driver_associate_params *params)
{
	struct wpa_driver_realtek_data *drv = priv;
	struct ieee80211req_mlme mlme;
	int ret = 0, privacy = 1;

	printf("Wpa_supplicant: %s +++\n", __FUNCTION__);

	wpa_printf(MSG_DEBUG, "%s", __FUNCTION__);

	//_Eric ?? if ret = -1, why still run continuously?

	if(realtek_config_security(drv, params) < 0)
		ret = -9;

	if (params->bssid == NULL) {

		//_Eric ?? it means no MAC but have SSID(name), driver shall try to find this AP automatically?
		
		if (set80211param(drv, IEEE80211_PARAM_ROAMING, 0, 1) < 0)
			ret = -2;

		if (wpa_driver_wext_set_ssid(drv->wext, params->ssid,
					     params->ssid_len) < 0)
			ret = -3;
		
	} else {
		if (set80211param(drv, IEEE80211_PARAM_ROAMING, 2, 1) < 0)
			ret = -4;
		if (wpa_driver_wext_set_ssid(drv->wext, params->ssid,
					     params->ssid_len) < 0)
			ret = -5;
		os_memset(&mlme, 0, sizeof(mlme));
		mlme.im_op = IEEE80211_MLME_ASSOC;
		os_memcpy(mlme.im_macaddr, params->bssid, IEEE80211_ADDR_LEN);

		printf("Try to assoc %02x:%02x:%02x:%02x:%02x:%02x \n", 
			params->bssid[0], params->bssid[1], params->bssid[2],
			params->bssid[3], params->bssid[4], params->bssid[5]);

		if (set80211priv(drv, IEEE80211_IOCTL_SETMLME, &mlme,
				 sizeof(mlme), 1) < 0) {
			wpa_printf(MSG_DEBUG, "%s: SETMLME[ASSOC] failed",
				   __func__);
			ret = -1;
		}
	}

	printf("Wpa_supplicant: %s --- ret = %d\n", __FUNCTION__, ret);
	
	return ret;
	
}

static int
wpa_driver_realtek_set_auth_alg(void *priv, int auth_alg)
{
	struct wpa_driver_realtek_data *drv = priv;
	int authmode;

	printf("Wpa_supplicant: %s +++ auth_alg = %d\n", __FUNCTION__, auth_alg);

	if ((auth_alg & AUTH_ALG_OPEN_SYSTEM) &&
	    (auth_alg & AUTH_ALG_SHARED_KEY))
		authmode = IEEE80211_AUTH_AUTO;
	else if (auth_alg & AUTH_ALG_SHARED_KEY)
		authmode = IEEE80211_AUTH_SHARED;
	else
		authmode = IEEE80211_AUTH_OPEN;

	return set80211param(drv, IEEE80211_PARAM_AUTHMODE, authmode, 1);
}

static int
wpa_driver_realtek_scan(void *priv, const u8 *ssid, size_t ssid_len)
{
	struct wpa_driver_realtek_data *drv = priv;
	struct iwreq iwr;
	int ret = 0;

	printf("Wpa_supplicant: %s +++\n", __FUNCTION__);

	os_memset(&iwr, 0, sizeof(iwr));
	os_strlcpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);

	if (wpa_driver_wext_set_ssid(drv->wext, ssid, ssid_len) < 0)
		ret = -1;
	
#ifdef RTK_INBAND
	if (inband_ioctl(SIOCSIWSCAN, &iwr) < 0)
#else
	if (ioctl(drv->sock, SIOCSIWSCAN, &iwr) < 0) 
#endif
	{
		perror("ioctl[SIOCSIWSCAN]");
		ret = -1;
	}

	eloop_cancel_timeout(wpa_driver_wext_scan_timeout, drv->wext,
			     drv->ctx);
	eloop_register_timeout(30, 0, wpa_driver_wext_scan_timeout, drv->wext,
			       drv->ctx);

	return ret;
}

static int wpa_driver_realtek_get_bssid(void *priv, u8 *bssid)
{
	struct wpa_driver_realtek_data *drv = priv;
	printf("Wpa_supplicant: %s +++\n", __FUNCTION__);
	return wpa_driver_wext_get_bssid(drv->wext, bssid);
}


static int wpa_driver_realtek_get_ssid(void *priv, u8 *ssid)
{
	struct wpa_driver_realtek_data *drv = priv;
	printf("Wpa_supplicant: %s +++\n", __FUNCTION__);
	return wpa_driver_wext_get_ssid(drv->wext, ssid);
}


static struct wpa_scan_results *
wpa_driver_realtek_get_scan_results(void *priv)
{
	struct wpa_driver_realtek_data *drv = priv;
	printf("Wpa_supplicant: %s +++\n", __FUNCTION__);
	return wpa_driver_wext_get_scan_results(drv->wext);
}


static int wpa_driver_realtek_set_operstate(void *priv, int state)
{
	struct wpa_driver_realtek_data *drv = priv;
	printf("Wpa_supplicant: %s +++\n", __FUNCTION__);
	return wpa_driver_wext_set_operstate(drv->wext, state);
}


static int wpa_driver_realtek_mlme_setprotection(void *priv, const u8 *addr, int protect_type, int key_type)
{
	struct wpa_driver_realtek_data *drv = priv;
	struct ieee80211req_mlme mlme;

	printf("Wpa_supplicant: %s +++\n", __FUNCTION__);

	if(protect_type == MLME_SETPROTECTION_KEY_TYPE_PAIRWISE)
	{
		if(protect_type != MLME_SETPROTECTION_PROTECT_TYPE_NONE)
			mlme.im_op = IEEE80211_MLME_AUTHORIZE;
		else
			mlme.im_op = IEEE80211_MLME_UNAUTHORIZE;
	}
	else
		return 0;

	os_memcpy(mlme.im_macaddr, addr, IEEE80211_ADDR_LEN);
	
	return set80211priv(drv, IEEE80211_IOCTL_SETMLME, &mlme, sizeof(mlme), 1);

}


static int wpa_driver_realtek_set_probe_req_ie(void *priv, const u8 *ies,
					       size_t ies_len)
{
	struct ieee80211req_getset_appiebuf *probe_req_ie;
	int ret;

	printf("Wpa_supplicant: %s +++\n", __FUNCTION__);

	probe_req_ie = os_malloc(sizeof(*probe_req_ie) + ies_len);
	if (probe_req_ie == NULL)
		return -1;

	probe_req_ie->app_frmtype = IEEE80211_APPIE_FRAME_PROBE_REQ;
	probe_req_ie->app_buflen = ies_len;
	os_memcpy(probe_req_ie->app_buf, ies, ies_len);

#ifdef RTK_INBAND_LE
	probe_req_ie->app_frmtype = htonl(probe_req_ie->app_frmtype);
	probe_req_ie->app_buflen = htonl(probe_req_ie->app_buflen);
#endif

	ret = set80211priv(priv, IEEE80211_IOCTL_SET_APPIEBUF, probe_req_ie,
			   sizeof(struct ieee80211req_getset_appiebuf) +
			   ies_len, 1);

	os_free(probe_req_ie);

	return ret;
}


static void * wpa_driver_realtek_init(void *ctx, const char *ifname)
{
	struct wpa_driver_realtek_data *drv;
	struct iwreq iwr;
	struct rtk_wpas_config config;
	struct wpa_supplicant *wpa_s = (struct wpa_supplicant *)ctx;

	printf("Wpa_supplicant: %s +++\n", __FUNCTION__);

	drv = os_zalloc(sizeof(*drv));
	if (drv == NULL)
		return NULL;

	drv->ctx = ctx;
	os_strlcpy(drv->ifname, ifname, sizeof(drv->ifname));
	drv->sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (drv->sock < 0)
		goto fail;

	if (set80211param(drv, IEEE80211_PARAM_ROAMING, 2, 1) < 0) {
		wpa_printf(MSG_DEBUG, "%s: failed to set wpa_supplicant-based "
			   "roaming", __FUNCTION__);
		goto fail;
	}

	if (set80211param(drv, IEEE80211_PARAM_WPA, 3, 1) < 0) {
		wpa_printf(MSG_DEBUG, "%s: failed to enable WPA support",
			   __FUNCTION__);
		goto fail;
	}

		
	memset(&config, 0, sizeof(config));

	config.is_hapd = 0;
	config.type = WPAS_CONFIG_MIB;
	config.bandmode = wpa_s->conf->macPhyMode;
	config.phymode	= wpa_s->conf->phyBandSelect;

	if(set80211priv(drv, WPAS_IOCTL_CUSTOM, &config, sizeof(config), 1)) {
		wpa_printf(MSG_ERROR, "%s: Failed to set Configurations", 
				__FUNCTION__);
		goto fail;
		}

	drv->wext = wpa_driver_wext_init(ctx, ifname);
	if (drv->wext == NULL)
		goto fail;

	printf("Wait 5 seconds for driver init ...\n");
	sleep(5);

	os_memset(&iwr, 0, sizeof(iwr));
	os_strlcpy(iwr.ifr_name, drv->ifname, IFNAMSIZ);

#ifdef RTK_INBAND
	if (inband_ioctl(SIOCSIWSCAN, &iwr) < 0)
#else
	if (ioctl(drv->sock, SIOCSIWSCAN, &iwr) < 0) 
#endif
	{
		perror("ioctl[SIOCSIWSCAN] of init");
		goto fail;
	}
	else{
		printf("Wait 5 seconds for scanning ...\n");
		sleep(5);
	}

	return drv;


fail:
	close(drv->sock);
	os_free(drv);
	return NULL;
}


static void wpa_driver_realtek_deinit(void *priv)
{
	struct wpa_driver_realtek_data *drv = priv;

	printf("Wpa_supplicant: %s +++\n", __FUNCTION__);

	if (wpa_driver_realtek_set_wpa_ie(drv, NULL, 0) < 0) {
		wpa_printf(MSG_DEBUG, "%s: failed to clear WPA IE",
			   __FUNCTION__);
	}
	if (set80211param(drv, IEEE80211_PARAM_ROAMING, 0, 1) < 0) {
		wpa_printf(MSG_DEBUG, "%s: failed to enable driver-based "
			   "roaming", __FUNCTION__);
	}
	if (set80211param(drv, IEEE80211_PARAM_PRIVACY, 0, 1) < 0) {
		wpa_printf(MSG_DEBUG, "%s: failed to disable forced Privacy "
			   "flag", __FUNCTION__);
	}
	if (set80211param(drv, IEEE80211_PARAM_WPA, 0, 1) < 0) {
		wpa_printf(MSG_DEBUG, "%s: failed to disable WPA",
			   __FUNCTION__);
	}

	wpa_driver_wext_deinit(drv->wext);

	close(drv->sock);
	os_free(drv);
}


const struct wpa_driver_ops wpa_driver_realtek_ops = {
	.name			= "realtek",
	.desc			= "realtek 802.11 support (Atheros, etc.)",
	.get_bssid		= wpa_driver_realtek_get_bssid,
	.get_ssid		= wpa_driver_realtek_get_ssid,
	.set_key		= wpa_driver_realtek_set_key,
	.init			= wpa_driver_realtek_init,
	.deinit			= wpa_driver_realtek_deinit,
	.set_countermeasures	= wpa_driver_realtek_set_countermeasures,
	.set_drop_unencrypted	= wpa_driver_realtek_set_drop_unencrypted,
	.scan			= wpa_driver_realtek_scan,
	.get_scan_results2	= wpa_driver_realtek_get_scan_results,
	.deauthenticate		= wpa_driver_realtek_deauthenticate,
	.disassociate		= wpa_driver_realtek_disassociate,
	.associate		= wpa_driver_realtek_associate,
	.set_auth_alg		= wpa_driver_realtek_set_auth_alg,
	.set_operstate		= wpa_driver_realtek_set_operstate,
	.mlme_setprotection = wpa_driver_realtek_mlme_setprotection,
	.set_probe_req_ie	= wpa_driver_realtek_set_probe_req_ie,
};
