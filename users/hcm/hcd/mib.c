/*
  *   MIB access control for RTL8197B
  *
  *	Copyright (C)2008, Realtek Semiconductor Corp. All rights reserved.
  *
  *	$Id: mib.c,v 1.31 2011/02/14 07:50:28 marklee Exp $
  */
  
/*================================================================*/
/* Include Files */
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <netinet/in.h>


/*================================================================*/
/* Local Include Files */

#include "hcd.h"
#include "mib.h"
//#include "../../../linux-2.6.30/drivers/char/rtl_mdio/rtl_mdio.h"
#define FW_VERSION	"1.0" 

/*================================================================*/
/* Local Variables */
//struct wlan_config_mib wlan_mib;
static struct io_config_mib io_mib;
struct config_mib_all mib_all;

/*for internal use now....*/
struct config_cmd_entry io_cmd_table[]={
CMD_IO_DEF(cmd_timeout, 				BYTE_T,	"10"	,			1,			255		    ),
CMD_IO_DEF(mii_pause_enable, 			BYTE_T, 	"1"	,			0,			1			),
CMD_IO_DEF(eth_pause_enable, 			BYTE_T, 	"1"	,				0,			1			),
CMD_IO_DEF(cpu_suspend_enable,		    BYTE_T, 	"0"	,				0,			1			),
CMD_IO_DEF(phy_reg_poll_time, 			BYTE_T, 	"10"	,			1,			100	    ),		
{0}
};	

struct config_cmd_entry wlan_comm_config_cmd_table[] = {
CMD_WLAN_DEF(MIB_HW_RF_TYPE ,  "RFPowerScale", BYTE_T,	RFPowerScale	, "0" ,  0 , 1),
CMD_SYS_DEF(MIB_HW_REG_DOMAIN,  "regdomain", BYTE_T,	regDomain , "1" ,  1 , 10),	//move to sys_setting
CMD_WLAN_DEF(MIB_WLAN_CHAN_NUM ,  "channel", BYTE_T,	channel , "11" ,  0 , 250), //channel depend on regdomain
CMD_WLAN_DEF(MIB_WLAN_BAND ,  "band",	 BYTE_T,	wlanBand	, "11" ,  1 , 31),
CMD_WLAN_DEF(MIB_WLAN_CHANNEL_BONDING ,  "use40M", BYTE_T,	channelbonding	, "1" ,  0 , 1),
CMD_WLAN_DEF(MIB_WLAN_PREAMBLE_TYPE ,  "preamble", BYTE_T,	preambleType	, "1" ,  0 , 1),
CMD_WLAN_DEF(MIB_WLAN_CONTROL_SIDEBAND	,  "2ndchoffset",	 BYTE_T,	controlsideband ,"2" ,	1 , 2),
CMD_WLAN_DEF(MIB_WLAN_FIX_RATE	,  "fixrate",	 INT_BIT_T, fixedTxRate ,"1" ,	0 , 0xffffffff),
CMD_WLAN_DEF(MIB_WLAN_BASIC_RATE ,	"basicrates", WORD_T,	basicRates	, "15"	,  1 , 0xffff),
CMD_WLAN_DEF(MIB_WLAN_SUPPORTED_RATE ,	"oprates", WORD_T,	supportedRates	, "4095"  ,  1 , 0xffff),
CMD_WLAN_DEF(MIB_WLAN_RATE_ADAPTIVE_ENABLED ,  "autorate", BYTE_T,	rateAdaptiveEnabled, "1" ,	0 , 1),
CMD_WLAN_DEF(MIB_WLAN_PROTECTION_DISABLED ,  "disable_protection", BYTE_T,	protectionDisabled	, "1" ,  0 , 1),
CMD_WLAN_DEF(MIB_WLAN_BEACON_INTERVAL ,  "bcnint", WORD_T,	beaconInterval , "100" ,  20 , 1024),
CMD_WLAN_DEF(MIB_WLAN_DTIM_PERIOD ,  "dtimperiod", BYTE_T, dtimPeriod	, "1" ,  0 , 256),
CMD_WLAN_DEF(MIB_WLAN_RTS_THRESHOLD ,  "rtsthres", WORD_T,	rtsThreshold	, "2347" ,	0 , 2347),
CMD_WLAN_DEF(MIB_WLAN_FRAG_THRESHOLD	,  "fragthres",  WORD_T,	fragThreshold	,"2346" ,  256 , 2346),
CMD_WLAN_DEF(MIB_WLAN_SHORT_GI ,  "enable_shortGI", BYTE_T, shortgiEnabled	, "1"  ,  0 , 1),
CMD_WLAN_DEF(MIB_WLAN_AGGREGATION ,  "enable_AMPDU",  BYTE_T,	aggregation ,"3" ,	0 , 3),
CMD_WLAN_DEF(MIB_WLAN_11N_STBC ,  "enable_stbc",  BYTE_T,	STBCEnabled ,"0" ,	0 , 3),
CMD_WLAN_DEF(MIB_WLAN_11N_COEXIST ,  "enable_coexist",  BYTE_T,	CoexistEnabled ,"0" ,	0 , 3),
CMD_WLAN_DEF(MIB_WLAN_MODE ,	"wireless_mode",	BYTE_T, wlanMode ,"0" ,	0 , 7),
CMD_WLAN_DEF(MIB_WLAN_PHY_BAND_SELECT ,  "phy_band_select",  BYTE_T,	phyBandSelect ,"0" ,0 , 2), //mark_92d
CMD_WLAN_DEF(MIB_WLAN_MAC_PHY_MODE ,	"mac_phy_mode",	BYTE_T, macPhyMode ,"0" ,	0 , 2),
{0},
};

struct config_cmd_entry wlan_config_cmd_table[]={
/*WLAN config mib cmd----------------------------------------*/	
/*id , name, type, mib_name ,def,start, end, ----------------------------------------*/
CMD_WLANEXT_DEF(MIB_INTF_NAME ,  "name", STRING_T,	name, "wlan" ,  0, 10),
CMD_WLAN_DEF(MIB_WLAN_MAC_ADDR ,  "macaddr", STRING_T, wlanMacAddr	, "00e04c8196c0" ,  0 , 13),
CMD_WLAN_DEF(MIB_WLAN_SSID 	,  "ssid",	 STRING_T,	ssid	,"Realtek-inband-AP" ,  0 , 32),
CMD_WLAN_DEF(MIB_WLAN_AUTH_TYPE ,  "wep_authtype", BYTE_T,	authType	, "2"  ,  0 , 2),
CMD_WLAN_DEF(MIB_WLAN_ENCRYPT ,  "encmode", BYTE_T,	encrypt	, "0" ,  0 , 6),
CMD_WLAN_DEF(MIB_WLAN_WEP_DEFAULT_KEY ,  "wepdkeyid", BYTE_T,	wepDefaultKey	, "0" ,  0 , 3),
CMD_WLAN_DEF(MIB_WLAN_WPA_CIPHER_SUITE ,  "wpa_cipher", BYTE_T,	wpaCipher	, "0"  ,  0 , 8),
CMD_WLAN_DEF(MIB_WLAN_WPA2_CIPHER_SUITE ,  "wpa2_cipher", BYTE_T,	wpa2Cipher	, "0" ,  0 , 8),
CMD_WLAN_DEF(MIB_WLAN_WPA_PSK ,  "passphrase", STRING_T,	wpaPSK	, "" ,  1 , 32),
CMD_WLAN_DEF(MIB_WLAN_WPA_PSK_FORMAT,  "wpaPSKFormat", BYTE_T,	wpaPSKFormat, "0" ,	0 , 32),
CMD_WLAN_DEF(MIB_WLAN_PSK_ENABLE ,	"wpa_auth",	 BYTE_T,	wpaAuth	,"0" ,	0 , 2), 
CMD_WLAN_DEF(MIB_WLAN_WPA_GROUP_REKEY_TIME ,  "gk_rekey",	 INT_T,	wpaGroupRekeyTime	,"86400" ,  0 , 65535),
CMD_WLAN_DEF(MIB_WLAN_ENABLE_1X ,  "802_1x", BYTE_T,	enable1X	, "0"  ,  0 , 1),
CMD_WLAN_DEF(MIB_WLAN_WEP ,  "wep", BYTE_T,	wep	, "0"  ,  0 , 10),
CMD_WLAN_DEF(MIB_WLAN_WEP_KEY_TYPE,  "wepKeyType", BYTE_T, wepKeyType , "0"  ,  0 , 10),
CMD_WLAN_DEF(MIB_WLAN_WEP_DEFAULT_KEY,  "wepDefaultKey", BYTE_T, wepDefaultKey , "0"  ,	0 , 10),
CMD_WLAN_DEF(MIB_WLAN_WEP64_KEY1 ,  "wep64key1", BYTE5_T,	wep64Key1	, "" ,	5, 5),
CMD_WLAN_DEF(MIB_WLAN_WEP64_KEY2 ,  "wep64key2", BYTE5_T,	wep64Key2	, "" ,	5, 5),
CMD_WLAN_DEF(MIB_WLAN_WEP64_KEY3 ,  "wep64key3", BYTE5_T,	wep64Key3	, "" ,  5, 5),
CMD_WLAN_DEF(MIB_WLAN_WEP64_KEY4 ,  "wep64key4", BYTE5_T,	wep64Key4	, "" ,  5, 5),
CMD_WLAN_DEF(MIB_WLAN_WEP128_KEY1 ,  "wep128key1", BYTE13_T,	wep128Key1	, "" ,  13, 13),
CMD_WLAN_DEF(MIB_WLAN_WEP128_KEY2 ,  "wep128key2", BYTE13_T,	wep128Key2	, "" ,  13, 13),
CMD_WLAN_DEF(MIB_WLAN_WEP128_KEY3 ,  "wep128key3", BYTE13_T,	wep128Key3	, "" ,  13, 13),
CMD_WLAN_DEF(MIB_WLAN_WEP128_KEY4 ,  "wep128key4", BYTE13_T,	wep128Key4	, ""  ,  13, 13),
CMD_WLAN_DEF(MIB_WLAN_RS_IP ,	"auth_server_ip", INETADDR_T, rsIpAddr ,"127.0.0.1" , 0 , IP_ADDR_SLEN),
CMD_WLAN_DEF(MIB_WLAN_RS_PORT , "auth_server_port", WORD_T, rsPort ,"1812" , 0 , 65535),
CMD_WLAN_DEF(MIB_WLAN_RS_PASSWORD , "auth_server_shared_secret", STRING_T, rsPassword ,"realtek" , 0 , AUTH_SER_SECRE_LEN+1),
CMD_WLAN_DEF(MIB_WLAN_ACCOUNT_RS_IP ,	"acct_server_ip", INETADDR_T, accountRsIpAddr ,"127.0.0.1" , 0 , IP_ADDR_SLEN),
CMD_WLAN_DEF(MIB_WLAN_ACCOUNT_RS_PORT , "acct_server_port", WORD_T, accountRsPort ,"1812" , 0 , 65535),
CMD_WLAN_DEF(MIB_WLAN_RS_INTERVAL_TIME ,	"acct_server_shared_secret", STRING_T, accountRsPassword ,"" , 0 , 65535),
CMD_WLAN_DEF(MIB_WLAN_ACCOUNT_RS_INTERVAL_TIME ,	"radius_acct_interim_interval", INT_T, accountRsIntervalTime ,"600" , 0 , 65535),
CMD_WLAN_DEF(MIB_WLAN_BLOCK_RELAY ,  "block_relay", BYTE_T,	blockRelay	, "0" ,  0 , 2),
CMD_WLAN_DEF(MIB_WLAN_WMM_ENABLED ,  "enable_qos", BYTE_T, wmmEnabled	, "1"  ,  0 , 1),
//--------------------priv mib-------------
CMD_WLAN_DEF(MIB_WLAN_HIDDEN_SSID ,  "disable_hiddenAP", BYTE_T,	hiddenSSID	, "0" ,  0 , 1),
CMD_WLAN_DEF(MIB_WLAN_DISABLED ,  "disable_rf", BYTE_T,	wlanDisabled	, "0" ,  0 , 1),
#if 0 //WDS not enabled
CMD_WLAN_DEF(MIB_WLAN_WDS_ENABLED,  "wds_pure", BYTE_T,	channel	, "1" ,  1 , 11),
CMD_WLAN_DEF(MIB_WLAN_WDS,  "wds_priority", BYTE_T,	preambleType	, "1" ,  0 , 32),
CMD_WLAN_DEF(MIB_WLAN_WDS_ADD,  "wds_add",	 STRING_T,	ssid	,"MotoAP" ,  0 , 32),
CMD_WLAN_DEF(MIB_WLAN_WDS_ENCRYPT ,  "wds_encrypt", BYTE_T,	wdsEncrypt	, "0"  ,  0 , 32),
CMD_WLAN_DEF(MIB_WLAN_WDS_WEP_FORMAT ,	"wds_wepkey_format", BYTE_T,	wdsWepKeyFormat	, "0"  ,  0 , 32),
CMD_WLAN_DEF(MIB_WLAN_WDS_WEP_KEY ,  "wds_wepkey", STRING_T,	wdsWepKey	, "" ,  0 , WEP128_KEY_LEN*2+1),
CMD_WLAN_DEF(MIB_WLAN_WDS_PSK_FORMAT ,	"wds_wepkey_format", BYTE_T,	wdsPskFormat	, "0"  ,  0 , 32),
CMD_WLAN_DEF(MIB_WLAN_WDS_PSK ,  "wds_passphrase", STRING_T,	wdsPsk	, "" ,  0 , MAX_PSK_LEN+1),
#endif
CMD_WLAN_DEF(MIB_WLAN_AC_ENABLED ,	"aclmode", BYTE_T,	acEnabled	, "0" ,  0 , 2),
CMD_WLAN_DEF(MIB_WLAN_AC_ADDR, "acl_mac_list", STRING_T,	acAddrArray	, "" ,	0 , 6*2*MAX_WLAN_AC_NUM+ MAX_WLAN_AC_NUM), //mac_string * max MACs + max MACs ','
CMD_WLANEXT_DEF(MIB_VLAN_CONFIG ,  "VLAN", STRING_T,	vlan, "0,0,0,1,0,1" ,  0, 30),
CMD_WLAN_DEF(MIB_WPS_DISABLE ,	"disable_wps", BYTE_T,	wscDisable	, "1" ,  0 , 1),
CMD_WLAN_DEF(MIB_WPS_CONFIGURED ,	"wps_configured", BYTE_T,	wscConfigured	, "0" ,  0 , 9),
CMD_WLAN_DEF(MIB_WPS_METHOD ,	"wps_config_method", BYTE_T,	wscMethod	, "134" ,  0 , 0xff),
//WSC_AUTH_OPEN=1, WSC_AUTH_WPAPSK=2, WSC_AUTH_SHARED=4, WSC_AUTH_WPA=8, WSC_AUTH_WPA2=0x10, WSC_AUTH_WPA2PSK=0x20, WSC_AUTH_WPA2PSKMIXED=0x22
CMD_WLAN_DEF(MIB_WPS_AUTH , "wps_auth", BYTE_T, wscAuth	, "1" ,  0 , 32),
//WSC_ENCRYPT_NONE=1, WSC_ENCRYPT_WEP=2, WSC_ENCRYPT_TKIP=4, WSC_ENCRYPT_AES=8, WSC_ENCRYPT_TKIPAES=12
CMD_WLAN_DEF(MIB_WPS_ENC ,	"wps_enc", BYTE_T, wscEnc	, "1" ,  0 , 32),
CMD_WLAN_DEF(MIB_WPS_MANUAL_ENABLED ,	"wps_manualEnabled", BYTE_T, wscManualEnabled	, "0" ,  0 , 32),
CMD_WLAN_DEF(MIB_WPS_UPNP_ENABLED , "wps_upnp_support", BYTE_T, wscUpnpEnabled	, "1" ,  0 , 32),
CMD_WLAN_DEF(MIB_WPS_REGISTRAR_ENABLED ,	"wps_registrarEnabled", BYTE_T, wscRegistrarEnabled	, "1" ,  0 , 32),
CMD_WLAN_DEF(MIB_WPS_CONFIG_BY_EXT_REG ,	"wps_configbyextreg", BYTE_T, wscConfigByExtReg	, "1" ,  0 , 32),
CMD_WLAN_DEF(MIB_WLAN_WDS_ENABLED ,  "enable_wds", BYTE_T,	wdsEnabled	, "0"  ,  0 , 32),
CMD_WLAN_DEF(MIB_WLAN_IAPP_DISABLED ,	"disable_iapp",	BYTE_T, iappDisabled ,"0" ,	0 , 3),
CMD_WLANEXT_DEF(MIB_WLAN_STA_NUM ,  "stanum", INT_T,	stanum, "0" ,	0, MAX_IF_STA_NUM), //mark_sta
//CMD_WLAN_DEF(MIB_WLAN_STA_EXPIRETIME ,	"sta_expiretime", INT_T,	expiretime, "30000" ,  0, 65535),
CMD_WLAN_DEF(MIB_WLAN_ACCESS,  "nat_only", BYTE_T, access	, "0"  ,  0 , 1),
CMD_WLANEXT_DEF(MIB_MANUAL_EDCA ,  "enable_manual_edca", BYTE_T,	enable_manual_edca, "0" ,	0, 1),
CMD_WLANEXT_DEF(MIB_STA_BKQ_ACM ,  "sta_bkq_acm", BYTE_T,	sta_bkq_acm, "0" ,	0, 1),
CMD_WLANEXT_DEF(MIB_STA_BKQ_CWMIN , "sta_bkq_cwmin", BYTE_T,	sta_bkq_cwmin, "4" ,	1, 10),
CMD_WLANEXT_DEF(MIB_STA_BKQ_CWMAX , "sta_bkq_cwmax", BYTE_T,	sta_bkq_cwmax, "10" ,	1, 10),
CMD_WLANEXT_DEF(MIB_STA_BKQ_AIFSN , "sta_bkq_aifsn", BYTE_T,	sta_bkq_aifsn, "7" ,	1, 7),
CMD_WLANEXT_DEF(MIB_STA_BKQ_TXOPLIMIT , "sta_bkq_txoplimit", INT_T,	sta_bkq_txoplimit, "0" ,	0, 256),
CMD_WLANEXT_DEF(MIB_STA_BEQ_ACM ,  "sta_beq_acm", BYTE_T,	sta_beq_acm, "0" ,	0, 1),
CMD_WLANEXT_DEF(MIB_STA_BEQ_CWMIN , "sta_beq_cwmin", BYTE_T,	sta_beq_cwmin, "4" ,	1, 10),
CMD_WLANEXT_DEF(MIB_STA_BEQ_CWMAX , "sta_beq_cwmax", BYTE_T,	sta_beq_cwmax, "10" , 1, 10),
CMD_WLANEXT_DEF(MIB_STA_BEQ_AIFSN , "sta_beq_aifsn", BYTE_T,	sta_beq_aifsn, "3" ,	1, 7),
CMD_WLANEXT_DEF(MIB_STA_BEQ_TXOPLIMIT , "sta_beq_txoplimit", INT_T, sta_beq_txoplimit, "0" ,	0, 256),
CMD_WLANEXT_DEF(MIB_STA_VIQ_ACM ,  "sta_viq_acm", BYTE_T,	sta_viq_acm, "0" ,	0, 1),
CMD_WLANEXT_DEF(MIB_STA_VIQ_CWMIN , "sta_viq_cwmin", BYTE_T,	sta_viq_cwmin, "3" ,	1, 10),
CMD_WLANEXT_DEF(MIB_STA_VIQ_CWMAX , "sta_viq_cwmax", BYTE_T,	sta_viq_cwmax, "4" , 1, 10),
CMD_WLANEXT_DEF(MIB_STA_VIQ_AIFSN , "sta_viq_aifsn", BYTE_T,	sta_viq_aifsn, "2" ,	1, 7),
CMD_WLANEXT_DEF(MIB_STA_VIQ_TXOPLIMIT , "sta_viq_txoplimit", INT_T, sta_viq_txoplimit, "188" ,	0, 256),
CMD_WLANEXT_DEF(MIB_STA_VIQ_ACM ,  "sta_voq_acm", BYTE_T,	sta_voq_acm, "0" ,	0, 1),
CMD_WLANEXT_DEF(MIB_STA_VIQ_CWMIN , "sta_voq_cwmin", BYTE_T,	sta_voq_cwmin, "2" ,	1, 10),
CMD_WLANEXT_DEF(MIB_STA_VIQ_CWMAX , "sta_voq_cwmax", BYTE_T,	sta_voq_cwmax, "3" , 1, 10),
CMD_WLANEXT_DEF(MIB_STA_VIQ_AIFSN , "sta_voq_aifsn", BYTE_T,	sta_voq_aifsn, "2" ,	1, 7),
CMD_WLANEXT_DEF(MIB_STA_VIQ_TXOPLIMIT , "sta_voq_txoplimit", INT_T, sta_voq_txoplimit, "102" ,	0, 256),
CMD_WLANEXT_DEF(MIB_AP_BKQ_ACM ,  "ap_bkq_acm", BYTE_T,	ap_bkq_acm, "0" ,	0, 1),
CMD_WLANEXT_DEF(MIB_AP_BKQ_CWMIN , "ap_bkq_cwmin", BYTE_T,	ap_bkq_cwmin, "4" ,	1, 10),
CMD_WLANEXT_DEF(MIB_AP_BKQ_CWMAX , "ap_bkq_cwmax", BYTE_T,	ap_bkq_cwmax, "10" , 1, 10),
CMD_WLANEXT_DEF(MIB_AP_BKQ_AIFSN , "ap_bkq_aifsn", BYTE_T,	ap_bkq_aifsn, "7" ,	1, 7),
CMD_WLANEXT_DEF(MIB_AP_BKQ_TXOPLIMIT , "ap_bkq_txoplimit", INT_T, ap_bkq_txoplimit, "0" ,	0, 256),
CMD_WLANEXT_DEF(MIB_AP_BEQ_ACM ,  "ap_beq_acm", BYTE_T,	ap_beq_acm, "0" ,	0, 1),
CMD_WLANEXT_DEF(MIB_AP_BEQ_CWMIN , "ap_beq_cwmin", BYTE_T,	ap_beq_cwmin, "4" ,	1, 10),
CMD_WLANEXT_DEF(MIB_AP_BEQ_CWMAX , "ap_beq_cwmax", BYTE_T,	ap_beq_cwmax, "6" , 1, 10),
CMD_WLANEXT_DEF(MIB_AP_BEQ_AIFSN , "ap_beq_aifsn", BYTE_T,	ap_beq_aifsn, "3" ,	1, 7),
CMD_WLANEXT_DEF(MIB_AP_BEQ_TXOPLIMIT , "ap_beq_txoplimit", INT_T, ap_beq_txoplimit, "0" ,	0, 256),
CMD_WLANEXT_DEF(MIB_AP_VIQ_ACM ,  "ap_viq_acm", BYTE_T,	ap_viq_acm, "0" ,	0, 1),
CMD_WLANEXT_DEF(MIB_AP_VIQ_CWMIN , "ap_viq_cwmin", BYTE_T,	ap_viq_cwmin, "3" ,	1, 10),
CMD_WLANEXT_DEF(MIB_AP_VIQ_CWMAX , "ap_viq_cwmax", BYTE_T,	ap_viq_cwmax, "4" , 1, 10),
CMD_WLANEXT_DEF(MIB_AP_VIQ_AIFSN , "ap_viq_aifsn", BYTE_T,	ap_viq_aifsn, "2" ,	1, 7),
CMD_WLANEXT_DEF(MIB_AP_VIQ_TXOPLIMIT , "ap_viq_txoplimit", INT_T, ap_viq_txoplimit, "188" ,	0, 256),
CMD_WLANEXT_DEF(MIB_AP_VOQ_ACM ,  "ap_voq_acm", BYTE_T,	ap_voq_acm, "0" ,	0, 1),
CMD_WLANEXT_DEF(MIB_AP_VOQ_CWMIN , "ap_voq_cwmin", BYTE_T,	ap_voq_cwmin, "2" ,	1, 10),
CMD_WLANEXT_DEF(MIB_AP_VOQ_CWMAX , "ap_voq_cwmax", BYTE_T,	ap_voq_cwmax, "3" , 1, 10),
CMD_WLANEXT_DEF(MIB_AP_VOQ_AIFSN , "ap_voq_aifsn", BYTE_T,	ap_voq_aifsn, "2" ,	1, 7),
CMD_WLANEXT_DEF(MIB_AP_VOQ_TXOPLIMIT , "ap_voq_txoplimit", INT_T, ap_voq_txoplimit, "102" ,	0, 256),
{0}
};

struct config_cmd_entry eth_config_cmd_table[]={
CMD_ETH_DEF(MIB_INTF_NAME ,  "name", STRING_T,	name, "eth" ,  0, 10),
CMD_ETH_DEF(MIB_VLAN_CONFIG ,  "VLAN", STRING_T,	vlan, "0,0,0,1,0,1" ,  0, 30),
{0}
};

struct config_cmd_entry hw_config_cmd_table[]={
CMD_HW_DEF(MIB_HW_BOARD_VER,  "board", BYTE_T,	boardVer, "1" ,  0, 10),
{0}
};

struct config_cmd_entry sys_config_cmd_table[]={
CMD_SYS_DEF(MIB_VAP_NUM ,  "vap_number", BYTE_T,	vap_number, "0" ,	0, 7),
CMD_SYS_DEF(MIB_INIT_DEFAULT , "init_default", BYTE_T, init_default, "0", 0, 1),
CMD_SYS_DEF(MIB_ENABLE_EFUSE_CONFIG , "enabel_efuse_config", BYTE_T, enable_efuse_config, "0", 0, 1),
CMD_SYS_DEF(MIB_ETH_MAC ,  "eth_mac", STRING_T,	eth_mac, "001234567899" ,	0, 13),
CMD_SYS_DEF(MIB_VLAN_ENABLE ,  "vlan_enable", BYTE_T,	vlan_enable, "0" ,  0, 1),
CMD_SYS_DEF(MIB_IP_ADDR ,  "ip", STRING_T,	ip_addr, "192.168.1.254" ,	0, 15+1),
CMD_SYS_DEF(MIB_SUBNET_MASK ,  "netmask", STRING_T,	netmask, "255.255.255.0" ,	0, 15+1),
CMD_SYS_DEF(MIB_HOST_MAC ,  "host_mac", STRING_T,	host_mac, "000000000000" ,	0, 13),
CMD_SYS_DEF(MIB_HOST_IP_ADDR ,  "host_ip", STRING_T,	host_ip, "192.168.1.254" ,	0, 15+1),
CMD_SYS_DEF(MIB_WPS_PIN ,	"wps_pin", STRING_T,	wscPin , "12345670" ,	0 , 8), //legacy in hw settings
CMD_SYS_DEF(MIB_WPS_DEVICENAME ,	"wps_device_name", STRING_T,	wps_devicename , "" ,	0 , 100), //legacy in hw settings
CMD_SYS_DEF(MIB_SYSTEM_MODE ,  "mode", BYTE_T,	mode, "0" ,	0, 7), // 0:AP-VAP 1:REAPTER+AP 2:PURE REPTER 3:CLIENT 4: WDS
{0}
};

/*================================================================*/
/* Routine Implementations */

unsigned char convert_atob(char *data, int base)
{
	char tmpbuf[10];
	int bin;

	memcpy(tmpbuf, data, 2);
	tmpbuf[2]='\0';
	if (base == 16)
		sscanf(tmpbuf, "%02x", &bin);
	else
		sscanf(tmpbuf, "%02d", &bin);
	return((unsigned char)bin);
}

/*
  *	check if more than one bits is asserted.    
  */
static int is_more_bit_asserted(int val)
{
	int i, num=0;
	
	for (i=0; i<32; i++) {
		if (BIT(i) & val)
			num++;
	}
	if (num > 1)
		return 1;
	else
		return 0;
}

#ifdef CMD_LINE
void dump_mib( int all , char *nameid)
{
	int i = 0, val, index=0;
	unsigned char *ptr, tmpbuf[256], bVal;
	unsigned short wVal;
	unsigned char *pmib ;
	struct config_cmd_entry *config_cmd_table;
	char *intf, *tmp;

	/*
	intf = strchr(nameid,' ');
	tmp = (char *)malloc(intf-nameid)+1;
	memset(tmp,0,intf-nameid+1);
	memcpy(tmp, nameid, intf-nameid);
	tmp[intf-nameid] = '\0';
	intf = tmp;
	nameid = strchr(nameid,' ')+1;
	*/

	intf = nameid;
	nameid = strchr(nameid,'_')+1;
	intf[nameid-intf-1] = '\0';
	
	if( select_mib_table(intf, &pmib, &config_cmd_table) < 0) {
		DEBUG_ERR("MIB not found!\n");
		return -1;
	}

	while (config_cmd_table[i].id != LAST_ENTRY_ID) {
		if( (all ==1) || ( (all !=1) && (!strcmp(config_cmd_table[i].name, nameid))) ){
		
		switch (config_cmd_table[i].type) {
			case BYTE_T:
				memcpy(&bVal, ((unsigned char *)pmib)+config_cmd_table[i].offset, 1);
				printf("%s=%d\n", config_cmd_table[i].name, (int)bVal);
				break;				

			case WORD_T:
				memcpy(&wVal, ((unsigned char *)pmib)+config_cmd_table[i].offset, 2);
				printf("%s=%d\n", config_cmd_table[i].name, (int)wVal);
				break;				
			
			case INT_T:
			case INT_BIT_T:
				memcpy(&val, ((unsigned char *)pmib)+config_cmd_table[i].offset, 4);
				printf("%s=%d\n", config_cmd_table[i].name, val);
				break;				
				
			case STRING_T:
				ptr = ((unsigned char *)pmib)+config_cmd_table[i].offset;
				if (strlen(ptr) > 0)
					strcpy(tmpbuf, ptr);
				else
					tmpbuf[0] = '\0';
				printf("%s=%s\n", config_cmd_table[i].name, ptr);				
				break;				

			case BYTE5_T:
				ptr = ((unsigned char *)pmib)+config_cmd_table[i].offset;
				printf("%s=%02x%02x%02x%02x%02x\n", config_cmd_table[i].name, 
							*ptr, *(ptr+1),  *(ptr+2),  *(ptr+3),  *(ptr+4) );
				break;

			case BYTE6_T:
				ptr = ((unsigned char *)pmib)+config_cmd_table[i].offset;
				printf("%s=%02x%02x%02x%02x%02x%02x\n", config_cmd_table[i].name, 
							*ptr, *(ptr+1),  *(ptr+2),  *(ptr+3),  *(ptr+4),  *(ptr+5));
				break;
				
			case BYTE13_T:				
				ptr = ((unsigned char *)pmib)+config_cmd_table[i].offset;
				printf("%s=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
							config_cmd_table[i].name, 
							*ptr, *(ptr+1),  *(ptr+2),  *(ptr+3),  *(ptr+4),  *(ptr+5), *(ptr+6), 
							*(ptr+7),  *(ptr+8),  *(ptr+9),  *(ptr+10),  *(ptr+11), *(ptr+12));
				break;				
			
			default:
				printf("Invalid mib type!\n");
				return;
			}
			if(!all) break; 
		}
		//printf(">>> [%s] read (%02x)%s as %d\n",__FUNCTION__,((unsigned char *)pmib)+config_cmd_table[i].offset,config_cmd_table[i].name,*(((unsigned char *)pmib)+config_cmd_table[i].offset));
		i++;
	}
}
#endif


static int init_config_mib_default()
{
	int i = 0, val, j, max_len, index=0;
	char *ptr, bVal;
	unsigned short wVal;
	//struct wlan_config_mib *pmib = &wlan_mib;
	APMIB_Tp flash_current_settings;

	i = 0;
	while (sys_config_cmd_table[i].id != LAST_ENTRY_ID) {
		switch (sys_config_cmd_table[i].type) {
			case BYTE_T:
				if (sys_config_cmd_table[i].def) {				
					bVal = (unsigned char) atoi(sys_config_cmd_table[i].def);				
					memcpy(((unsigned char *)&mib_all.sys_config)+sys_config_cmd_table[i].offset, &bVal, 1);
				}
				break;
				
			case WORD_T:			
				if (sys_config_cmd_table[i].def) {				
					wVal = (unsigned short)atoi(sys_config_cmd_table[i].def);				
					memcpy(((unsigned char *)&mib_all.sys_config)+sys_config_cmd_table[i].offset, &wVal, 2);
				}
				break;							
			
			case INT_T:
			case INT_BIT_T:
				if (sys_config_cmd_table[i].def) {				
					val = atoi(sys_config_cmd_table[i].def);				
					memcpy(((unsigned char *)&mib_all.sys_config)+sys_config_cmd_table[i].offset, &val, 4);
				}
				break;				
				
			case STRING_T:
				if (sys_config_cmd_table[i].def) {
					strcpy(((unsigned char *)&mib_all.sys_config)+sys_config_cmd_table[i].offset, sys_config_cmd_table[i].def);
				}
				break;				
			case BYTE5_T:
			case BYTE6_T:
			case BYTE13_T:				
				if (sys_config_cmd_table[i].def) {
					if (sys_config_cmd_table[i].type ==BYTE5_T)
						max_len = 5;
					else if (sys_config_cmd_table[i].type ==BYTE6_T)
						max_len = 6;
					else
						max_len = 13;					
					for (j=0, ptr=sys_config_cmd_table[i].def; *ptr && j<max_len; j++, ptr+=2) {
						if ( !isxdigit((int)*ptr) || !isxdigit((int)*(ptr+1)) ) {
							printf("Invalid BYTE_T vlaue!\n");
							return -1;
						}				
						*(((unsigned char *)&mib_all.sys_config)+sys_config_cmd_table[i].offset+j) = convert_atob(ptr, 16);
					}					
				}
				break;	
				
			default:
				printf("Invalid mib type! mib_name:%s\n",sys_config_cmd_table[i].name);
				return -1;
		}
		i++;
	}

	for( index=0; index<MAX_ETH_INTF; index++) {
		i = 0;
		while (eth_config_cmd_table[i].id != LAST_ENTRY_ID) {
			switch (eth_config_cmd_table[i].type) {
				case BYTE_T:
					if (eth_config_cmd_table[i].def) {				
						bVal = (unsigned char)atoi(eth_config_cmd_table[i].def);				
						memcpy(((unsigned char *)&mib_all.eth[index])+eth_config_cmd_table[i].offset, &bVal, 1);
					}
					break;
					
				case WORD_T:			
					if (eth_config_cmd_table[i].def) {				
						wVal = (unsigned short)atoi(eth_config_cmd_table[i].def);				
						memcpy(((unsigned char *)&mib_all.eth[index])+eth_config_cmd_table[i].offset, &wVal, 2);
					}
					break;							
				
				case INT_T:
				case INT_BIT_T:
					if (eth_config_cmd_table[i].def) {				
						val = atoi(eth_config_cmd_table[i].def);				
						memcpy(((unsigned char *)&mib_all.eth[index])+eth_config_cmd_table[i].offset, &val, 4);
					}
					break;				
					
				case STRING_T:
					if (eth_config_cmd_table[i].def) {
						strcpy(((unsigned char *)&mib_all.eth[index])+eth_config_cmd_table[i].offset, eth_config_cmd_table[i].def);
					}
					break;				
				case BYTE5_T:
				case BYTE6_T:
				case BYTE13_T:				
					if (eth_config_cmd_table[i].def) {
						if (eth_config_cmd_table[i].type ==BYTE5_T)
							max_len = 5;
						else if (eth_config_cmd_table[i].type ==BYTE6_T)
							max_len = 6;
						else
							max_len = 13;					
						for (j=0, ptr=eth_config_cmd_table[i].def; *ptr && j<max_len; j++, ptr+=2) {
							if ( !isxdigit((int)*ptr) || !isxdigit((int)*(ptr+1)) ) {
								printf("Invalid BYTE_T vlaue!\n");
								return -1;
							}				
							*(((unsigned char *)&mib_all.eth[index])+eth_config_cmd_table[i].offset+j) = convert_atob(ptr, 16);
						}					
					}
					break;	
					
				default:
					printf("Invalid mib type! mib_name:%s\n",eth_config_cmd_table[i].name);
					return -1;
			}
			i++;
		}
	}
#ifdef CONFIG_HCD_FLASH_SUPPORT
	if ((flash_current_settings=(APMIB_Tp)apmib_csconf()) == NULL) {
		return 0;
	}
#endif
	for( index=0; index<MAX_WLAN_INTF; index++) {
#ifdef CONFIG_HCD_FLASH_SUPPORT
		if( !mib_all.sys_config.init_default ) {
			memcpy(&mib_all.wlan[index].legacy_flash_settings,&flash_current_settings->wlan[0][index],sizeof(mib_all.wlan[index].legacy_flash_settings));
		} else {
#endif
			i = 0;
			while (wlan_comm_config_cmd_table[i].id != LAST_ENTRY_ID) {
				switch (wlan_comm_config_cmd_table[i].type) {
					case BYTE_T:
						if (wlan_comm_config_cmd_table[i].def) {				
							bVal = (unsigned char)atoi(wlan_comm_config_cmd_table[i].def);				
							memcpy(((unsigned char *)&(mib_all.wlan[index].legacy_flash_settings))+wlan_comm_config_cmd_table[i].offset, &bVal, 1);
						}
						break;
						
					case WORD_T:			
						if (wlan_comm_config_cmd_table[i].def) {				
							wVal = (unsigned short)atoi(wlan_comm_config_cmd_table[i].def);				
							memcpy(((unsigned char *)&(mib_all.wlan[index].legacy_flash_settings))+wlan_comm_config_cmd_table[i].offset, &wVal, 2);
						}
						break;							
					
					case INT_T:
					case INT_BIT_T:
						if (wlan_comm_config_cmd_table[i].def) {				
							val = atoi(wlan_comm_config_cmd_table[i].def);				
							memcpy(((unsigned char *)&(mib_all.wlan[index].legacy_flash_settings))+wlan_comm_config_cmd_table[i].offset, &val, 4);
						}
						break;				
						
					case STRING_T:
						if (wlan_comm_config_cmd_table[i].def) {
							strcpy(((unsigned char *)&(mib_all.wlan[index].legacy_flash_settings))+wlan_comm_config_cmd_table[i].offset, wlan_comm_config_cmd_table[i].def);
						}
						break;				
					case BYTE5_T:
					case BYTE6_T:
					case BYTE13_T:				
						if (wlan_comm_config_cmd_table[i].def) {
							if (wlan_comm_config_cmd_table[i].type ==BYTE5_T)
								max_len = 5;
							else if (wlan_comm_config_cmd_table[i].type ==BYTE6_T)
								max_len = 6;
							else
								max_len = 13;					
							for (j=0, ptr=wlan_comm_config_cmd_table[i].def; *ptr && j<max_len; j++, ptr+=2) {
								if ( !isxdigit((int)*ptr) || !isxdigit((int)*(ptr+1)) ) {
									printf("Invalid BYTE_T vlaue!\n");
									return -1;
								}				
								*(((unsigned char *)&(mib_all.wlan[index].legacy_flash_settings))+wlan_comm_config_cmd_table[i].offset+j) = convert_atob(ptr, 16);
							}					
						}
						break;
					case INETADDR_T:
						if (wlan_comm_config_cmd_table[i].def) {
							//unsigned int serv_ip;
							inet_aton(wlan_comm_config_cmd_table[i].def,(struct in_addr *)(((unsigned char *)&(mib_all.wlan[index].legacy_flash_settings))+wlan_comm_config_cmd_table[i].offset));
							//serv_ip = *(int *)(((unsigned char *)&(mib_all.wlan[index].legacy_flash_settings))+wlan_comm_config_cmd_table[i].offset);
							//printf("set radius server ip:%d.%d.%d.%d, (%s)\n",serv_ip>>24,(serv_ip>>16)&0xff,(serv_ip>>8)&0xff,serv_ip&0xff,wlan_comm_config_cmd_table[i].def);
						}
						break;
						
					default:
						printf("Invalid mib type! mib_name:%s\n",wlan_comm_config_cmd_table[i].name);
						return -1;
				}
				//printf(">>> [%s] set (%02x)%s to deafult:%d\n",__FUNCTION__,((unsigned char *)&mib_all.wlan_comm)+wlan_comm_config_cmd_table[i].offset,wlan_comm_config_cmd_table[i].name,*(((unsigned char *)&mib_all.wlan_comm)+wlan_comm_config_cmd_table[i].offset));
				i++;
			}

			i = 0;
			while (wlan_config_cmd_table[i].id != LAST_ENTRY_ID) {
				/*
				if( strcmp(wlan_config_cmd_table[i].name,"VLAN") == 0 ){
					unsigned char tmpbuf[100] = {'\0'};
					while( (ptr = strstr(wlan_config_cmd_table[i].def,',')) ){
						*ptr = '\0';
						ptr = wlan_config_cmd_table[i].def;
						val = atoi(ptr);
						sprintf(tmpbuf,"%s %d ",tmpbuf,val);
						wlan_config_cmd_table[i].def = strstr(wlan_config_cmd_table[i].def,'\0')+1;
					}
					printf(">>> Default VLAN config:%s\n",tmpbuf);
					i++;
					continue;
				}
				*/
				
				switch (wlan_config_cmd_table[i].type) {
					case BYTE_T:
						if (wlan_config_cmd_table[i].def) {				
							bVal = (unsigned char)atoi(wlan_config_cmd_table[i].def);				
							memcpy(((unsigned char *)&(mib_all.wlan[index].legacy_flash_settings))+wlan_config_cmd_table[i].offset, &bVal, 1);
						}
						break;
						
					case WORD_T:			
						if (wlan_config_cmd_table[i].def) {				
							wVal = (unsigned short)atoi(wlan_config_cmd_table[i].def);				
							memcpy(((unsigned char *)&(mib_all.wlan[index].legacy_flash_settings))+wlan_config_cmd_table[i].offset, &wVal, 2);
						}
						break;							
					
					case INT_T:
					case INT_BIT_T:
						if (wlan_config_cmd_table[i].def) {				
							val = atoi(wlan_config_cmd_table[i].def);				
							memcpy(((unsigned char *)&(mib_all.wlan[index].legacy_flash_settings))+wlan_config_cmd_table[i].offset, &val, 4);
						}
						break;				
						
					case STRING_T:
						if (wlan_config_cmd_table[i].def) {
							if( index != 0 ) {	//for virtual interface
								char tmp[32] = {0};
								if( strcmp(wlan_config_cmd_table[i].name,"ssid") == 0 ) {
									if(index == WLAN_VXD_INDEX )
										sprintf(tmp,"819x_Repter_mode");
									else	
										sprintf(tmp,"Slave-VAP-%02x",index);

									strcpy(((unsigned char *)&(mib_all.wlan[index].legacy_flash_settings))+wlan_config_cmd_table[i].offset,tmp);
								} else if( strcmp(wlan_config_cmd_table[i].name,"macaddr") == 0 ) {
									sprintf(tmp,"00e04c00fb0%d",index);
									strcpy(((unsigned char *)&(mib_all.wlan[index].legacy_flash_settings))+wlan_config_cmd_table[i].offset,tmp);
								} else {
									strcpy(((unsigned char *)&(mib_all.wlan[index].legacy_flash_settings))+wlan_config_cmd_table[i].offset, wlan_config_cmd_table[i].def);
								}
							} else {	//for root interface
								strcpy(((unsigned char *)&(mib_all.wlan[index].legacy_flash_settings))+wlan_config_cmd_table[i].offset, wlan_config_cmd_table[i].def);
							}
						}
						break;				
					case BYTE5_T:
					case BYTE6_T:
					case BYTE13_T:				
						if (wlan_config_cmd_table[i].def) {
							if (wlan_config_cmd_table[i].type ==BYTE5_T)
								max_len = 5;
							else if (wlan_config_cmd_table[i].type ==BYTE6_T)
								max_len = 6;
							else
								max_len = 13;					
							for (j=0, ptr=wlan_config_cmd_table[i].def; *ptr && j<max_len; j++, ptr+=2) {
								if ( !isxdigit((int)*ptr) || !isxdigit((int)*(ptr+1)) ) {
									printf("Invalid BYTE_T vlaue!\n");
									return -1;
								}				
								*(((unsigned char *)&(mib_all.wlan[index].legacy_flash_settings))+wlan_config_cmd_table[i].offset+j) = convert_atob(ptr, 16);	
							}
						}
						break;	
					case INETADDR_T:
						if (wlan_config_cmd_table[i].def) {
							//unsigned int serv_ip;
							inet_aton(wlan_config_cmd_table[i].def,(struct in_addr *)(((unsigned char *)&(mib_all.wlan[index].legacy_flash_settings))+wlan_config_cmd_table[i].offset));
							//serv_ip = *(int *)(((unsigned char *)&(mib_all.wlan[index].legacy_flash_settings))+wlan_config_cmd_table[i].offset);
							//printf("set radius server ip:%d.%d.%d.%d, (%s)\n",serv_ip>>24,(serv_ip>>16)&0xff,(serv_ip>>8)&0xff,serv_ip&0xff,wlan_config_cmd_table[i].def);
						}
						break;
						
					default:
						printf("Invalid mib type! mib_name:%s\n",wlan_config_cmd_table[i].name);
						return -1;
				}
				//printf(">>> [%s] set %s to deafult:%d\n",__FUNCTION__,wlan_config_cmd_table[i].name,*(((unsigned char *)&mib_all.wlan[index])+wlan_config_cmd_table[i].offset));
				i++;
			}
#ifdef CONFIG_HCD_FLASH_SUPPORT
		}
#endif
	}

	for( index=0;index<MAX_ETH_INTF;index++){
		sprintf(mib_all.eth[index].port_name, "p%d",index);
		sprintf(mib_all.eth[index].name, "eth%d",index);
	}

	for( index=0;index<MAX_WLAN_AP_INTF;index++){
		if( index == 0 )
			sprintf(mib_all.wlan[index].name, "wlan0");
		else
			sprintf(mib_all.wlan[index].name, "wlan0-va%d",index-1);
	}
	//mark_vxd
	sprintf(mib_all.wlan[WLAN_VXD_INDEX].name, "wlan0-vxd");
	
	return 0;
}

unsigned char *string_to_hex(unsigned char *hex, unsigned char *str, int len)
{
	int i=0;
	
	for( i=0;i<len*2;i++ ){
		if( !(str[i] < '0') && str[i] < 'a' )
			hex[i/2] |= (str[i]-'0') << (((i+1)%2)*4);
		else if( str[i] > '9' && str[i] < 'g' )
			hex[i/2] |= ((str[i]-'a')+10) << (((i+1)%2)*4);
	}
	return hex;
}

static int _is_hex(char c)
{
    return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||
            ((c >= 'a') && (c <= 'f')));
}


int a_to_i(char s)
{
	int v;
	
	if ( s >= '0' && s <= '9')
		v = s - '0';
	else if ( s >= 'a' && s <= 'f')
		v = s - 'a' + 10;
	else if ( s >= 'A' && s <= 'F')
		v = s - 'A' + 10;
	else {
		printf("error hex format!\n");
		return 0;
	}	
	return v;
}


int string_to_mac(unsigned char *hex, unsigned char *str, int len)
{
	int i;

    for(i=0;i<len*2;i++)
	{
		if(!_is_hex(str[i]))
			return 0;	
		if( (i%2) == 0 )
			hex[i/2] = a_to_i(str[i])*16; 
		else	
			hex[i/2] += a_to_i(str[i]); 
    }
	return 1;
}

unsigned int get_vap_num(void)
{
	return mib_all.sys_config.vap_number;
}

unsigned char get_sys_mode(void)
{
	return mib_all.sys_config.mode;
}

unsigned char *get_host_mac(void)
{
	return mib_all.sys_config.host_mac;
}

unsigned int get_host_ip(void)
{
	//return mib_all.sys_config.ip_addr; 
	return mib_all.sys_config.host_ip; //mark_issue , now we return host_ip
}

unsigned int get_sys_netmask(void)
{
	return mib_all.sys_config.netmask;
}

unsigned char *get_macaddr(unsigned char *ifname)
{
	int i=0;
	unsigned char mac[7]={'\0'}, name[20];

	for( i=0; i<MAX_WLAN_INTF; i++ ){
		if( strcmp(mib_all.wlan[i].name,ifname) == 0 ){
			return string_to_hex(mac,mib_all.wlan[i].legacy_flash_settings.wlanMacAddr,6);
		}
	}
	return NULL;
}

CONFIG_WLAN_SETTING_Tp fecth_wlanmib(int index)
{
	return &(mib_all.wlan[index].legacy_flash_settings);
}

HW_SETTING_Tp fecth_hwmib(void)
{
#ifdef CONFIG_HCD_FLASH_SUPPORT
	return apmib_hwconf();
#else
	return &mib_all.hw_config;
#endif
}

void update_wps_pincode(unsigned char *pincode)
{
	memcpy(mib_all.sys_config.wscPin,pincode,PIN_LEN);
}

int select_mib_table(char *intf, unsigned char **mib, struct config_cmd_entry **config_cmd_table)
//int select_mib(char *intf, char *mib, int *table_switch)
{
	int index = 0;

	/*
	if( strncmp(intf,"eth",3) == 0 ) {
		for( index=0;index<MAX_ETH_INTF;index++) {
			if( strcmp(mib_all.eth[index].name,intf) == 0 ){
				*mib = &mib_all.eth[index];
				*config_cmd_table = eth_config_cmd_table;
				return 0;
			}
		}
	}
	*/
	if( strncmp(intf,"p",1) == 0 ) {
		for( index=0;index<MAX_ETH_INTF;index++) {
			if( strcmp(mib_all.eth[index].port_name,intf) == 0 ){
				*mib = &mib_all.eth[index];
				*config_cmd_table = eth_config_cmd_table;
				return 0;
			}
		}
	}

	if( strncmp(intf,"wlan",4) == 0 ) {
		if( strlen(intf) == 4 ) {
			//*mib = &mib_all.wlan_comm;
			*mib = &mib_all.wlan[0];
			*config_cmd_table = wlan_comm_config_cmd_table;
			return 1;
		}

		for(index=0 ;index<MAX_WLAN_INTF;index++) {
			if( strcmp(mib_all.wlan[index].name,intf) == 0 ){
				//*mib = &mib_all.wlan[index];
				*mib = &mib_all.wlan[index].legacy_flash_settings;
				*config_cmd_table = wlan_config_cmd_table;
				return 0;
			}
		}
	}

	if( strcmp(intf,"sys") == 0 ) {
		*mib = &mib_all.sys_config;
		*config_cmd_table = sys_config_cmd_table;
		return 0;
	}

	if( strcmp(intf,"hw") == 0 ) {
		*mib = &mib_all.hw_config;
		*config_cmd_table = hw_config_cmd_table;
		return 0;
	}

	return -1;
}

int sync_config_from_wlanroot(struct config_cmd_entry *cmd_table_entry, unsigned char *wlanroot)
{
	int ret=0, index=1, j=0, flag=0;
	unsigned char *pmib, *ptr;

	for(index=1;index<MAX_WLAN_INTF;index++) {
		pmib = &mib_all.wlan[index].legacy_flash_settings;
		
		switch(cmd_table_entry->type){
			case BYTE_T:
				//printf("sync %s %s from %d to %d\n",mib_all.wlan[index].name, cmd_table_entry->name, *(((unsigned char *)pmib)+cmd_table_entry->offset), *(((unsigned char *)wlanroot)+cmd_table_entry->offset) );
				memcpy(((unsigned char *)pmib)+cmd_table_entry->offset,((unsigned char *)wlanroot)+cmd_table_entry->offset , 1);
				break;				
				
			case WORD_T:
				//printf("sync %s %s from %d to %d\n",mib_all.wlan[index].name, cmd_table_entry->name, *(((unsigned char *)pmib)+cmd_table_entry->offset), *(((unsigned char *)wlanroot)+cmd_table_entry->offset));
				memcpy(((unsigned char *)pmib)+cmd_table_entry->offset, ((unsigned char *)wlanroot)+cmd_table_entry->offset, 2);
				break;												
			case INT_T:
			case INT_BIT_T:
				//printf("sync %s %s from %d to %d\n",mib_all.wlan[index].name, cmd_table_entry->name, *(((unsigned char *)pmib)+cmd_table_entry->offset), *(((unsigned char *)wlanroot)+cmd_table_entry->offset));
				memcpy(((unsigned char *)pmib)+cmd_table_entry->offset, ((unsigned char *)wlanroot)+cmd_table_entry->offset, 4);
				break;				
			
			case STRING_T:
					strcpy(((unsigned char *)pmib)+cmd_table_entry->offset, ((unsigned char *)wlanroot)+cmd_table_entry->offset);
					ret = strlen(((unsigned char *)wlanroot)+cmd_table_entry->offset);
				break;
			case BYTE5_T:
			case BYTE6_T:
			case BYTE13_T:				
				for (j=0, ptr=((unsigned char *)wlanroot)+cmd_table_entry->offset; *ptr && j<cmd_table_entry->start; j++, ptr+=2)
					*(((unsigned char *)pmib)+cmd_table_entry->offset+j) = convert_atob(ptr, 16);

				ret = 13;
				break;
			case INETADDR_T:
				memcpy(((unsigned char *)pmib)+cmd_table_entry->offset,((unsigned char *)wlanroot)+cmd_table_entry->offset,4);
				ret = 4;
			
			default:
				DEBUG_ERR("Invalid mib type!\n");
				ret = -RET_CANNOT_ACCESS;
				goto sync_end;
		}
	}
sync_end:
	return ret;
}

unsigned char *get_acl_count(void)
{
	return &mib_all.wlan[0].legacy_flash_settings.acNum;
}

unsigned char *update_acl_count(void)
{
	return &mib_all.wlan[0].legacy_flash_settings.acNum;
}


int acl_operation(int flag,unsigned char *buf,struct config_cmd_entry *cmd_entry, unsigned char *offset)
{
	unsigned char *mac, *next=NULL, num;
	unsigned char *comment;

	printf("%s get command:%s\n",__FUNCTION__,buf);

	if ((flag & ACCESS_MIB_SET) && *buf) {
		do {
			if( next )
				buf = next+1;
			mac = buf;
			comment = strchr(buf,',');

			num = mib_all.wlan[0].legacy_flash_settings.acNum;

			if( comment ){
				*comment = '\0';
				comment++;
			}

			next = strchr(comment,'+');
			if(next && *(next+1) != '\0') {
				*next = '\0';
				next++;
			} else
				next = NULL;

			/*
			string_to_hex(mac,mac,6);
			memcpy(((MACFILTER_Tp)offset+num)->macAddr,mac,6);
			strcpy(((MACFILTER_Tp)offset+num)->comment,comment);
			mib_all.wlan[0].legacy_flash_settings.acNum++;
			printf("insert ACL rule:");
			hex_dump(((MACFILTER_Tp)offset+num)->macAddr,6);
			printf(",%s\n",((MACFILTER_Tp)offset+num)->comment);
			*/
		} while(next);
	} /* else if ((flag & ACCESS_MIB_GET) && *buf) {
		
	}*/

	return 0;
}
int mac_acl_get(unsigned char *mac_list_string, unsigned char *pmib)
{
	CONFIG_WLAN_SETTING_Tp wlan_mib;
	char mac_buf[14];
	int i;
	wlan_mib = (CONFIG_WLAN_SETTING_Tp)pmib;

	strcat(mac_list_string,"=");	
	
	for(i=0;i<wlan_mib->acNum;i++)
	{
		sprintf(mac_buf,"%02x%02x%02x%02x%02x%02x",wlan_mib->acAddrArray[i].macAddr[0],
					wlan_mib->acAddrArray[i].macAddr[1],wlan_mib->acAddrArray[i].macAddr[2],
					wlan_mib->acAddrArray[i].macAddr[3],wlan_mib->acAddrArray[i].macAddr[4],
					wlan_mib->acAddrArray[i].macAddr[5]);

		if(i!= (wlan_mib->acNum-1))
			strcat(mac_buf,",");

		strcat(mac_list_string,mac_buf);
	}	
	return 0;
}

int mac_acl_add(int flag,unsigned char *mac_list_string, unsigned char *pmib)
{
	unsigned char *mac, *next=NULL, num;
	unsigned char *dot;
	CONFIG_WLAN_SETTING_Tp wlan_mib;
	unsigned char *pmib_acl_offset;
	unsigned char acl_mac[6],finished=0;

	wlan_mib = (CONFIG_WLAN_SETTING_Tp)pmib;
	pmib_acl_offset = (unsigned char *)&wlan_mib->acAddrArray[0];

	//printf("%s get command:%s\n",__FUNCTION__,mac_list_string);
	//reset mac_array
	memset( pmib_acl_offset,0,sizeof(MACFILTER_T)*MAX_WLAN_AC_NUM);
	wlan_mib->acNum =0;	
   //example , =112233445566,223344556677,010203040506'\0'.....
    mac = mac_list_string;      
	if ((flag & ACCESS_MIB_SET) && *mac_list_string) {
		do { //find mac token			
			
			dot = strchr(mac,',');
			if(dot)
			{
				*dot = '\0';				
				if(strlen(mac) != 12)
					goto error;
				
				if(!string_to_mac(&acl_mac[0],mac,6))
					goto error;

				//printf("acl_mac1\n");
				//hex_dump(acl_mac, 6);
				memcpy(&wlan_mib->acAddrArray[wlan_mib->acNum].macAddr,acl_mac,6);				
				wlan_mib->acNum++;	
				dot = dot +1;
				mac = dot;
				
			}
			else 
			{				
				if(strlen(mac) != 12)
					goto error;

				if(!string_to_mac(&acl_mac[0],mac,6))
					goto error;

				//printf("acl_mac2\n");
				//hex_dump(acl_mac, 6);
				memcpy(&wlan_mib->acAddrArray[wlan_mib->acNum].macAddr,acl_mac,6);
				wlan_mib->acNum++;
				finished =1;
			}				
			
 	   } while(!finished);
	} 
	/* else if ((flag & ACCESS_MIB_GET) && *buf) {
		
	}*/
	return 0;

error:
	wlan_mib->acNum =0; //
	return -1;
	
}


int access_config_mib(int flag, char *nameid, void *data1, char *intf)
{
	int val, i = 0, ret = RET_OK, sync=0;
	
	int j;
	char *ptr;
	unsigned long dwVal;
	unsigned short wVal;
	unsigned char tmpbuf[256] = {'\0'}, bVal;
	void *data=NULL;
	unsigned char *pmib ;
	struct config_cmd_entry *config_cmd_table;

	if (nameid == NULL) {
		DEBUG_ERR("nameid == NULL!\n");
		ret = -1;
		goto access_mib_done;
	}

	sync = select_mib_table(intf, &pmib, &config_cmd_table);
	if( sync < 0) {
		DEBUG_ERR("MIB not found!\n");
		ret = -1;
		goto access_mib_done;
	}	
	while (config_cmd_table[i].id != LAST_ENTRY_ID){
		if (((flag & ACCESS_MIB_BY_NAME) && !strcmp(config_cmd_table[i].name, nameid)) ||
				 ((flag & ACCESS_MIB_BY_ID) && (config_cmd_table[i].id == *((int *)nameid)))) {
			// Do MIB R/W			
				switch (config_cmd_table[i].type) {
					case BYTE_T:
						if (flag & ACCESS_MIB_SET) {
							if 	(flag & ACCESS_MIB_BY_NAME)
								bVal = (unsigned char)atoi(data1);
							else
								bVal = ((unsigned char *)data1)[0];

							if ((((int)bVal) < config_cmd_table[i].start || ((int)bVal) > config_cmd_table[i].end)) {
								DEBUG_ERR("Invalid BYTE_T cmd range [%d, %d, %d])!\n", bVal, config_cmd_table[i].start, config_cmd_table[i].end);
								ret = -RET_INVALID_RANGE;
							} else {
								//printf("%s from %d to %d\n", config_cmd_table[i].name, *(((unsigned char *)pmib)+config_cmd_table[i].offset), bVal);
								memcpy(((unsigned char *)pmib)+config_cmd_table[i].offset, &bVal, 1);
								ret = 1;
							}
						}
						else { //GET
							memcpy(&bVal, ((unsigned char *)pmib)+config_cmd_table[i].offset, 1);
							sprintf(intf,"%s_%s=%d",intf,nameid,bVal);
							ret = strlen(intf);
							//memcpy(data1, &bVal, 1);
							//ret = 1;
							//printf("get %s vlaue:%d\n", config_cmd_table[i].name, *(char *)data1);
						}					
						break;				
						
					case WORD_T:
						if (flag & ACCESS_MIB_SET) {			
							if 	(flag & ACCESS_MIB_BY_NAME)						
								wVal = (unsigned short)atoi(data1);
							else
								memcpy(&wVal, data1, 2);

							if ((((int)wVal) < config_cmd_table[i].start || ((int)wVal) > config_cmd_table[i].end)) {
								DEBUG_ERR("Invalid WORD_T cmd range [%d, %d, %d])!\n", val, config_cmd_table[i].start, config_cmd_table[i].end);
								return -RET_INVALID_RANGE;
							} else {
								//printf("%s from %d to %d\n", config_cmd_table[i].name, *(((unsigned char *)pmib)+config_cmd_table[i].offset), wVal);
								memcpy(((unsigned char *)pmib)+config_cmd_table[i].offset, &wVal, 2);
								ret = 2;
							}
						} else {
							memcpy(&wVal, ((unsigned char *)pmib)+config_cmd_table[i].offset, 2);	
							//memcpy(data1, &wVal, 2);
							sprintf(intf,"%s_%s=%d",intf,nameid,wVal);
							ret = strlen(intf);
							//ret = 2;
							//printf("get %s value:%d\n", config_cmd_table[i].name, wVal);
						}					
						break;												
					case INT_T:
					case INT_BIT_T:
						if (flag & ACCESS_MIB_SET) {					
							if 	(flag & ACCESS_MIB_BY_NAME)							
								val = atoi(data1);
							else
								memcpy(&val, data1, 4);							
							
							if (((unsigned int)val <(unsigned int) config_cmd_table[i].start || (unsigned int)val > (unsigned int)config_cmd_table[i].end)) {
								DEBUG_ERR("Invalid INT_T cmd range [%d, %d, %d])!\n", val, config_cmd_table[i].start, config_cmd_table[i].end);
								ret = -RET_INVALID_RANGE;
							}						
							if ((config_cmd_table[i].type == INT_BIT_T) && is_more_bit_asserted(val)) {
								DEBUG_ERR("Invalid cmd range [%d, %d, %d])!\n", val, config_cmd_table[i].start, config_cmd_table[i].end);
								ret = -RET_INVALID_RANGE;
							} else {
								//printf("%s from %d to %d\n", config_cmd_table[i].name, *(((unsigned char *)pmib)+config_cmd_table[i].offset), val);
								memcpy(((unsigned char *)pmib)+config_cmd_table[i].offset, &val, 4);
								ret = 4;
							}
						} else { //GET
							memcpy(&val, ((unsigned char *)pmib)+config_cmd_table[i].offset, 4);			
							//memcpy(data1, &val, 4);	
							sprintf(intf,"%s_%s=%d\n",intf,nameid,val);
							ret = strlen(intf);
							//ret = 4;
							//printf("get %s value:%d\n", config_cmd_table[i].name, val);
						}					
						break;				
					
					case STRING_T:
						if (flag & ACCESS_MIB_SET) {
							if ((strlen(data1) > 0) && (strlen(data1) < config_cmd_table[i].start || strlen(data1) > config_cmd_table[i].end)) {
								DEBUG_ERR("Invalid STRINT_T cmd range [%d, %d, %d])!\n", strlen(data1), config_cmd_table[i].start, config_cmd_table[i].end);
								ret = -RET_INVALID_RANGE;
							} else if(!strcmp(config_cmd_table[i].name,"acl_mac_list")) {
								ret = mac_acl_add(flag,data1,pmib);
								if(ret >=0 )								
								ret = strlen(data1);
								else
									ret = -RET_INVALID_ARG;
									
							} else {
								//printf("%s from %s to %s\n", config_cmd_table[i].name, ((unsigned char *)pmib)+config_cmd_table[i].offset, data1);
								strcpy(((unsigned char *)pmib)+config_cmd_table[i].offset, data1);
								ret = strlen(data1);
							}
						} else { //GET
							if(!strcmp(config_cmd_table[i].name,"acl_mac_list")) {
								mac_acl_get(intf,pmib);								
								ret = strlen(intf);
							} else {
								//strcpy((unsigned char *)data1, ((unsigned char *)pmib)+config_cmd_table[i].offset);
								//printf("get %s value:%s\n", config_cmd_table[i].name, ((unsigned char *)pmib)+config_cmd_table[i].offset);
								sprintf(intf,"%s_%s=%s\n",intf,nameid,((unsigned char *)pmib)+config_cmd_table[i].offset);
								ret = strlen(intf);
							}
						}
						break;
					case BYTE5_T:
					case BYTE6_T:
					case BYTE13_T:				
						if (flag & ACCESS_MIB_SET) {
							if (flag & ACCESS_MIB_BY_NAME) {						
								if (strlen(data1) == config_cmd_table[i].start*2) { //HEX ,ex -> 112233445566
									//DEBUG_ERR("Invalid BYTE cmd length [%d, %d])!\n", strlen(data1), config_cmd_table[i].start);
									//ret = -RET_INVALID_RANGE;
								//}
									for (j=0, ptr=data1; *ptr && j<config_cmd_table[i].start; j++, ptr+=2) {
										if (!isxdigit((int)*ptr) || !isxdigit((int)*(ptr+1)) ) {
											DEBUG_ERR("%s: Invalid BYTE_T vlaue!\n", __FUNCTION__);
											ret = -RET_INVALID_RANGE;
										}				
										*(((unsigned char *)pmib)+config_cmd_table[i].offset+j) = convert_atob(ptr, 16);
									}
								} else {
									memcpy(((unsigned char *)pmib)+config_cmd_table[i].offset, data1, config_cmd_table[i].start);
								}
							} else {
								memcpy(((unsigned char *)pmib)+config_cmd_table[i].offset, data1, config_cmd_table[i].start);
							}
							ret = config_cmd_table[i].start;
						} else { //GET
#if 0
							if (flag & ACCESS_MIB_BY_NAME) {
								ptr = ((unsigned char *)pmib)+config_cmd_table[i].offset;
								if (config_cmd_table[i].type ==BYTE6_T)  //5T 
									ret = printf("%s=%02x%02x%02x%02x%02x%02x\n", config_cmd_table[i].name, 
										*ptr, *(ptr+1),  *(ptr+2),  *(ptr+3),  *(ptr+4),  *(ptr+5));
								else							
									ret = printf("%s=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
											config_cmd_table[i].name,
											*ptr, *(ptr+1),  *(ptr+2),  *(ptr+3),  *(ptr+4),  *(ptr+5), *(ptr+6),
											*(ptr+7),  *(ptr+8),  *(ptr+9),  *(ptr+10),  *(ptr+11), *(ptr+12));
							}
							//else 
#endif		
							{
								memcpy(data1, ((unsigned char *)pmib)+config_cmd_table[i].offset,  config_cmd_table[i].start);	
								ret =  config_cmd_table[i].start;	
							}
						}					
						break;
					case INETADDR_T:
						if (flag & ACCESS_MIB_SET) {
							inet_aton(data1,((unsigned char *)pmib)+config_cmd_table[i].offset);
							ret = strlen(data1);
						} else {
							sprintf(intf,"%s_%s=%s\n",intf,nameid,inet_ntoa(*(struct in_addr *)((unsigned char *)pmib+config_cmd_table[i].offset)));
							ret = strlen(intf);
						}
						break;

					default:
						DEBUG_ERR("Invalid mib type!\n");
						ret = -RET_NOT_NOW;
				}
				if( ret )
					goto access_mib_done;
		} else {
			i++;
		}
	}

	if( ret == 0 ) {
		DEBUG_ERR("Can't find mib!\n");
		ret = -RET_INVALID_CMD_ID;
	}
access_mib_done:
	if( (flag & ACCESS_MIB_SET) && sync && (ret > 0) ){
		//printf("parameter should be synced\n");
		sync_config_from_wlanroot(&config_cmd_table[i],pmib);
	}
	
	return ret;
}

int is_8021x_mode(CONFIG_WLAN_SETTING_Tp wlan_config)
{
	int ret=0;
	//if encmode = open or wep 
   if((wlan_config->encrypt == 0) || (wlan_config->encrypt == 1) )
   {
   	  if(wlan_config->enable1X)
	  	ret =1;
   }// if encmode = WPA , WPA2 , WPA2MIX  
   else if((wlan_config->encrypt == 2) || (wlan_config->encrypt == 4)
   			|| (wlan_config->encrypt == 6))
   {
   	  if(wlan_config->wpaAuth == 1)
	  	ret =1;
   }
   
   return ret;
}

void init_config_mib() 
{
	memset(&mib_all, '\0', sizeof(mib_all)); 	
	//internal use
	io_mib.cmd_timeout = 10;
	io_mib.mii_pause_enable= 1;
	io_mib.eth_pause_enable= 1;
	io_mib.cpu_suspend_enable= 0;
	io_mib.phy_reg_poll_time= 10;

	//init wlan config mib to default
	init_config_mib_default(); 
}


