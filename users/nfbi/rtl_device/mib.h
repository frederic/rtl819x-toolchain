/*
  *   MIB header file for RTL8197B
  *
  *	Copyright (C)2008, Realtek Semiconductor Corp. All rights reserved.
  *
  *	$Id: mib.h,v 1.2 2010/02/05 07:18:18 marklee Exp $
  */

#ifndef INCLUDE_MIB_H
#define INCLUDE_MIB_H

/*================================================================*/
#define __PACK__  // no use 

/* Constant Definitions */
#define MAX_INTF_NAME 10
#define MAX_WLAN_INTF 5
#define MAX_ETH_INTF 6
#define VLAN_CONFIG_LEN 30
#define MAC_ADDR_LEN    13
#define VLAN_CONFIG_NUM 6


// MIB ID
#define LAST_ENTRY_ID 0 //LAST_ENTRY_ID

// WLAN MIB id
#define MIB_WLAN_SSID			1
#define MIB_WLAN_CHAN_NUM		2
#define MIB_WLAN_WEP			3
#define MIB_WLAN_WEP64_KEY1		4
#define MIB_WLAN_WEP64_KEY2		5
#define MIB_WLAN_WEP64_KEY3		6
#define MIB_WLAN_WEP64_KEY4		7
#define MIB_WLAN_WEP128_KEY1		8
#define MIB_WLAN_WEP128_KEY2		9
#define MIB_WLAN_WEP128_KEY3		10
#define MIB_WLAN_WEP128_KEY4		11
#define MIB_WLAN_WEP_KEY_TYPE		12
#define MIB_WLAN_WEP_DEFAULT_KEY	13
#define MIB_WLAN_FRAG_THRESHOLD		14
#define MIB_WLAN_SUPPORTED_RATE		15
#define MIB_WLAN_BEACON_INTERVAL	16
#define MIB_WLAN_PREAMBLE_TYPE		17
#define MIB_WLAN_BASIC_RATE		18
#define MIB_WLAN_RTS_THRESHOLD		19
#define MIB_WLAN_AUTH_TYPE		20
#define MIB_WLAN_HIDDEN_SSID		21
#define MIB_WLAN_DISABLED		22
#define MIB_ELAN_MAC_ADDR		23
#define MIB_WLAN_MAC_ADDR		24
#define MIB_WLAN_ENCRYPT		25
#define MIB_WLAN_ENABLE_SUPP_NONWPA	26
#define MIB_WLAN_SUPP_NONWPA		27
#define MIB_WLAN_WPA_AUTH		28
#define MIB_WLAN_WPA_CIPHER_SUITE	29
#define MIB_WLAN_WPA_PSK		30
#define MIB_WLAN_WPA_GROUP_REKEY_TIME	31
#define MIB_WLAN_RS_IP			32
#define MIB_WLAN_RS_PORT		33
#define MIB_WLAN_RS_PASSWORD		34
#define MIB_WLAN_ENABLE_1X		35
#define MIB_WLAN_WPA_PSK_FORMAT		36
#define MIB_WLAN_WPA2_PRE_AUTH		37
#define MIB_WLAN_WPA2_CIPHER_SUITE	38
#define MIB_WLAN_ACCOUNT_RS_ENABLED	39
#define MIB_WLAN_ACCOUNT_RS_IP		40
#define MIB_WLAN_ACCOUNT_RS_PORT	41
#define MIB_WLAN_ACCOUNT_RS_PASSWORD	42
#define MIB_WLAN_ACCOUNT_UPDATE_ENABLED	43
#define MIB_WLAN_ACCOUNT_UPDATE_DELAY	44
#define MIB_WLAN_ENABLE_MAC_AUTH	45
#define MIB_WLAN_RS_RETRY		46
#define MIB_WLAN_RS_INTERVAL_TIME	47
#define MIB_WLAN_ACCOUNT_RS_RETRY	48
#define MIB_WLAN_ACCOUNT_RS_INTERVAL_TIME 49

#define MIB_WLAN_INACTIVITY_TIME	50
#define MIB_WLAN_RATE_ADAPTIVE_ENABLED	51
#define MIB_WLAN_AC_ENABLED		52
#define MIB_WLAN_AC_NUM			53
#define MIB_WLAN_AC_ADDR		54
#define MIB_WLAN_AC_ADDR_ADD		55
#define MIB_WLAN_AC_ADDR_DEL		56
#define MIB_WLAN_AC_ADDR_DELALL		57
#define MIB_WLAN_DTIM_PERIOD		58
#define MIB_WLAN_MODE			59
#define MIB_WLAN_NETWORK_TYPE		60
#define MIB_WLAN_DEFAULT_SSID		61	// used while configured as Ad-hoc and no any other Ad-hoc could be joined
						// it will use this default SSID to start BSS
#define MIB_WLAN_IAPP_DISABLED		62
#define MIB_WLAN_WDS_ENABLED		63
#define MIB_WLAN_WDS_NUM		64
#define MIB_WLAN_WDS			65
#define MIB_WLAN_WDS_ADD		66
#define MIB_WLAN_WDS_DEL		67
#define MIB_WLAN_WDS_DELALL		68
#define MIB_WLAN_WDS_ENCRYPT		69
#define MIB_WLAN_WDS_WEP_FORMAT		70
#define MIB_WLAN_WDS_WEP_KEY		71
#define MIB_WLAN_WDS_PSK_FORMAT		72
#define MIB_WLAN_WDS_PSK		73
#define MIB_WLAN_BAND			74
#define MIB_WLAN_FIX_RATE		75
#define MIB_WLAN_BLOCK_RELAY		76
#define MIB_WLAN_NAT25_MAC_CLONE	77
#define MIB_WLAN_PROTECTION_DISABLED 	78
// for WMM
#define MIB_WLAN_WMM_ENABLED 89

//11n
#define MIB_WLAN_CHANNEL_BONDING 284
#define MIB_WLAN_CONTROL_SIDEBAND 285
#define MIB_WLAN_AGGREGATION 286
#define MIB_WLAN_SHORT_GI 287

//priv MIB
#define MIB_WLAN_PSK_ENABLE  300
#define MIB_WLAN_SHORT_RETRY 301
#define MIB_WLAN_LONG_RETRY 302
// HW 
#define MIB_HW_REG_DOMAIN		204
#define MIB_HW_RF_TYPE 205

#define MIB_INTF_NAME 400
#define MIB_VLAN_ENABLE 401
#define MIB_VLAN_CONFIG 402
#define MIB_VAP_NUM 404
#define MIB_ETH_MAC 405
#define MIB_ENABLE_EFUSE_CONFIG 406


//private IO MIB id
#define id_cmd_timeout						0x201
#define id_mii_pause_enable   		        0x202
#define id_eth_pause_enable   		        0x203
#define id_cpu_suspend_enable             	0x204
#define id_phy_reg_poll_time			    0x205
#define id_set_host_pid						0x100		// set host daemon pid

             
#define FIELD_OFFSET(type, field)	((unsigned long)(long *)&(((type *)0)->field))

#define WLAN_MIB_OFFSET(field)	((int)FIELD_OFFSET(struct wlan_config_mib,field))
#define WLAN_COMM_MIB_OFFSET(field) ((int)FIELD_OFFSET(struct wlan_comm_config_mib,field))
#define ETH_MIB_OFFSET(field)	((int)FIELD_OFFSET(struct eth_config_mib,field))
#define SYS_MIB_OFFSET(field)	((int)FIELD_OFFSET(struct sys_config_mib,field))

#define IO_MIB_OFFSET(field)	((int)FIELD_OFFSET(struct io_config_mib,field))

#define CMD_WLAN_DEF(id,name, type, mib_name ,def,start, end) \
	{id, name, type, WLAN_MIB_OFFSET(mib_name), def ,start, end, ACT_WLAN_MIB_RW}

#define CMD_WLAN_COMM_DEF(id,name, type, mib_name ,def,start, end) \
	{id, name, type, WLAN_COMM_MIB_OFFSET(mib_name), def ,start, end, ACT_WLAN_MIB_RW}

#define CMD_ETH_DEF(id,name, type, mib_name ,def,start, end) \
	{id, name, type, ETH_MIB_OFFSET(mib_name), def ,start, end, ACT_SYS_MIB_RW}

#define CMD_SYS_DEF(id,name, type, mib_name ,def,start, end) \
	{id, name, type, SYS_MIB_OFFSET(mib_name), def ,start, end, ACT_SYS_MIB_RW}

#define CMD_IO_DEF(name, type ,def,start, end) \
	{id_##name, #name, type, IO_MIB_OFFSET(name),def, start, end, ACT_IO_MIB_RW}
             
// access mib flag definition
enum {
	ACCESS_MIB_SET=0x01, 
	ACCESS_MIB_GET=0x02, 
	ACCESS_MIB_BY_NAME=0x04, 
	ACCESS_MIB_BY_ID=0x08,
};

// MIB value and constant
// priv use
#define MAX_PSK_LEN				(64+1)
#define MAX_WEP_KEY_LEN	13
#define WEP64_KEY_LEN	5
#define WEP128_KEY_LEN	13
#define MAX_SSID_LEN			33

//security
//auth
#define AUTH_OPEN 0
#define AUTH_SHARED_KEY 1
#define AUTH_AUTO 2
//encrpyt
#define ENC_NONE				0
#define ENC_WEP64				1
#define ENC_TKIP				2
#define ENC_AES				4
#define ENC_WEP128			5

//psk_enable
#define PSK_DISABLE 0
#define PSK_WPA 1
#define PSK_WPA2 2

//chiper
#define CIPHER_TKIP 2
#define CIPHER_AES 8

//aggregation type
#define AGGREGATE_OFF 0
#define AGGREGATE_AMPDU 1
#define AGGREGATE_AMSDU 2
#define AGGREGATE_BOTH 3

						 
/*================================================================*/
/* Structure Definitions */
             
struct mac_addr {
	unsigned char addr[6];
};           

struct io_config_mib {   
/*system mib - control interface */	
	unsigned char cmd_timeout;
	unsigned char mii_pause_enable;
	unsigned char eth_pause_enable;
	unsigned char cpu_suspend_enable;
	unsigned char phy_reg_poll_time;	
};

struct vlan_info {
	int global_vlan;	// 0/1 - global vlan disable/enable
	int is_lan;				// 1: eth-lan/wlan port, 0: wan port
	int vlan;					// 0/1: disable/enable vlan
	int tag;					// 0/1: disable/enable tagging
	int id;						// 1~4090: vlan id
	int pri;						// 0~7: priority;
	int cfi;						// 0/1: cfi
};

struct sys_config_mib {
	unsigned char enable_efuse_config __PACK__;
	unsigned char vlan_enable __PACK__;
	unsigned char eth_mac[MAC_ADDR_LEN] __PACK__;
};


struct eth_config_mib {
	unsigned char port_name[3] __PACK__;
	unsigned char name[MAX_INTF_NAME] __PACK__;
	unsigned char vlan[VLAN_CONFIG_LEN] __PACK__;
};

struct wlan_comm_config_mib {
/*from HW wlan mib */
	unsigned char vap_number __PACK__;
	unsigned char RFPowerScale __PACK__; // RF output power scale, 0:100%, 1:50%, 2:25%, 3:10%, 45%
	unsigned char regDomain __PACK__; //from Hw
	unsigned char channel __PACK__ ;// current channel
	unsigned char channelbonding __PACK__; 
	unsigned char controlsideband __PACK__; 
	unsigned char wlanBand __PACK__; // wlan band, bit0-11B, bit1-11G, bit2-11A
	unsigned short basicRates __PACK__ ;
	unsigned int fixedTxRate __PACK__; // fixed wlan tx rate, used when rate adaptive is disabled
	unsigned char protectionDisabled __PACK__; // disable g mode protection
	unsigned short beaconInterval __PACK__ ;
	unsigned char dtimPeriod __PACK__; // DTIM period
	unsigned short rtsThreshold __PACK__ ;
	unsigned short fragThreshold __PACK__ ;
};

struct wlan_config_mib {
	unsigned char name[MAX_INTF_NAME] __PACK__; //from Hw
/*priv wlan mib*/	
	unsigned char shortretry __PACK__; 
	unsigned char longretry __PACK__; 
	unsigned char psk_enable  __PACK__; 
/*general wlan mib */
	unsigned char macaddr[MAC_ADDR_LEN]  __PACK__;
	unsigned char ssid[MAX_SSID_LEN] __PACK__ ; // SSID
//	unsigned char wlanMacAddr[6] __PACK__ ; // WLAN MAC address
	unsigned char wep __PACK__ ; // WEP flag, 0 - disabled, 1 - 64bits, 2 128 bits
	//unsigned char wep64Key[WEP64_KEY_LEN] __PACK__ ;
	unsigned char wep64Key1[WEP64_KEY_LEN] __PACK__ ;
	unsigned char wep64Key2[WEP64_KEY_LEN] __PACK__ ;
	unsigned char wep64Key3[WEP64_KEY_LEN] __PACK__ ;
	unsigned char wep64Key4[WEP64_KEY_LEN] __PACK__ ;
	//unsigned char wep128Key[WEP128_KEY_LEN] __PACK__ ;
	unsigned char wep128Key1[WEP128_KEY_LEN] __PACK__ ;
	unsigned char wep128Key2[WEP128_KEY_LEN] __PACK__ ;
	unsigned char wep128Key3[WEP128_KEY_LEN] __PACK__ ;
	unsigned char wep128Key4[WEP128_KEY_LEN] __PACK__ ;
	unsigned char wepDefaultKey __PACK__ ;
	unsigned char wepKeyType __PACK__ ;
	unsigned short supportedRates __PACK__ ;
	unsigned char preambleType __PACK__; // preamble type, 0 - long preamble, 1 - short preamble
	unsigned char authType __PACK__; // authentication type, 0 - open-system, 1 - shared-key, 2 - both
	unsigned char acEnabled __PACK__; // enable/disable WLAN access control
	unsigned char acNum __PACK__; // WLAN access control entry number
	//MACFILTER_T acAddrArray[MAX_WLAN_AC_NUM] __PACK__; // WLAN access control array

	unsigned char hiddenSSID __PACK__ ;
	unsigned char wlanDisabled __PACK__; // enabled/disabled wlan interface. For vap interface, 0 - disable, 1 - enable
	unsigned long inactivityTime __PACK__; // wlan client inactivity time
	unsigned char rateAdaptiveEnabled __PACK__; // enable/disable rate adaptive
	unsigned char wlanMode __PACK__; // wireless mode - AP, Ethernet bridge 
	unsigned char networkType __PACK__; // adhoc or Infrastructure
	unsigned char iappDisabled __PACK__; // disable IAPP
	unsigned char defaultSsid[MAX_SSID_LEN]__PACK__ ; // default SSID
	unsigned char blockRelay __PACK__; // block/un-block the relay between wireless client
	unsigned char maccloneEnabled __PACK__; // enable NAT2.5 MAC Clone
	unsigned char turboMode __PACK__; // turbo mode, 0 - auto, 1 - always, 2 - off

	// WPA stuffs
	unsigned char encrypt __PACK__; // encrypt type, defined as ENCRYPT_t
	unsigned char enableSuppNonWpa __PACK__; // enable/disable nonWPA client support
	unsigned char suppNonWpa __PACK__; // which kind of non-wpa client is supported (wep/1x)
	unsigned char wpaAuth __PACK__; // WPA authentication type (auto or psk)
	unsigned char wpaCipher __PACK__; // WPA unicast cipher suite
	unsigned char wpaPSK[MAX_PSK_LEN+1] __PACK__; // WPA pre-shared key
	unsigned long wpaGroupRekeyTime __PACK__; // group key rekey time in second
	unsigned char rsIpAddr[4] __PACK__; // radius server IP address
	unsigned short rsPort __PACK__; // radius server port number
	//unsigned char rsPassword[MAX_RS_PASS_LEN] __PACK__; // radius server password
	unsigned char enable1X __PACK__; // enable/disable 802.1x
	unsigned char wpaPSKFormat __PACK__; // PSK format 0 - passphrase, 1 - hex
	unsigned char accountRsEnabled __PACK__; // enable/disable accounting server
	unsigned char accountRsIpAddr[4] __PACK__; // accounting radius server IP address
	unsigned short accountRsPort __PACK__; // accounting radius server port number
	//unsigned char accountRsPassword[MAX_RS_PASS_LEN] __PACK__; // accounting radius server password
	unsigned char accountRsUpdateEnabled __PACK__; // enable/disable accounting server update
	unsigned short accountRsUpdateDelay __PACK__; // account server update delay time in sec
	unsigned char macAuthEnabled __PACK__; // mac authentication enabled/disabled
	unsigned char rsMaxRetry __PACK__; // radius server max try
	unsigned short rsIntervalTime __PACK__; // radius server timeout
	unsigned char accountRsMaxRetry __PACK__; // accounting radius server max try
	unsigned short accountRsIntervalTime __PACK__; // accounting radius server timeout
	unsigned char wpa2PreAuth __PACK__; // wpa2 Preauthtication support
	unsigned char wpa2Cipher __PACK__; // wpa2 Unicast cipher

	// WDS stuffs
	unsigned char wdsEnabled __PACK__; // wds enable/disable
	unsigned char wdsNum __PACK__; // number of wds entry existed
	//WDS_T wdsArray[MAX_WDS_NUM] __PACK__; // wds array
	unsigned char wdsEncrypt __PACK__; // wds encrypt flag
	unsigned char wdsWepKeyFormat __PACK__; // 0 - ASCII, 1 - hex
	unsigned char wdsWepKey[WEP128_KEY_LEN*2+1] __PACK__;
	unsigned char wdsPskFormat __PACK__;	// 0 - passphrase, 1 - hex
	unsigned char wdsPsk[MAX_PSK_LEN+1] __PACK__;

	// for WMM
	unsigned char wmmEnabled __PACK__; // WMM enable/disable

#ifdef WLAN_EASY_CONFIG
	unsigned char acfEnabled __PACK__;
	unsigned char acfMode __PACK__;
	unsigned char acfSSID[MAX_SSID_LEN] __PACK__ ; 
	unsigned char acfKey[MAX_ACF_KEY_LEN+1] __PACK__; 
	unsigned char acfDigest[MAX_ACF_DIGEST_LEN+1] __PACK__; 	
	unsigned char acfAlgReq __PACK__;
	unsigned char acfAlgSupp __PACK__;		
	unsigned char acfRole __PACK__;	
	unsigned char acfScanSSID[MAX_SSID_LEN] __PACK__ ; 	
	unsigned char acfWlanMode __PACK__;
#endif

#ifdef WIFI_SIMPLE_CONFIG
	unsigned char wscDisable __PACK__;
	unsigned char wscMethod __PACK__;
	unsigned char wscConfigured __PACK__;
	unsigned char wscAuth __PACK__;
	unsigned char wscEnc __PACK__;
	unsigned char wscManualEnabled __PACK__;
	unsigned char wscUpnpEnabled __PACK__;
	unsigned char wscRegistrarEnabled __PACK__;	
	unsigned char wscSsid[MAX_SSID_LEN] __PACK__ ;
	unsigned char wscPsk[MAX_PSK_LEN+1] __PACK__;
	unsigned char wscConfigByExtReg __PACK__;
#endif
//for 11N
	unsigned char aggregation __PACK__; 
	unsigned char shortgiEnabled __PACK__; 

	unsigned char access  __PACK__; // 0 - lan+wan, 1 - wan
	unsigned char priority __PACK__; // wan access priority
// for WAPI
#if CONFIG_RTL_WAPI_SUPPORT
	unsigned char wapiPsk[MAX_PSK_LEN+1] __PACK__; //password
	unsigned char wapiPskLen __PACK__; //password
	unsigned char wapiAuth __PACK__;//0:AS 1:pre-shared key
	unsigned char wapiPskFormat __PACK__; // WAPI unicast cipher suite
	unsigned char wapiAsIpAddr[4] __PACK__; // as server IP address
	unsigned char wapiMcastkey  __PACK__; //0:time 1 packets
	unsigned long wapiMcastRekeyTime __PACK__; // 300 -31536000
	unsigned long wapiMcastRekeyPackets __PACK__; //1048576
	unsigned char wapiUcastkey  __PACK__; //0:time 1 packets
	unsigned long wapiUcastRekeyTime __PACK__; // 300 -31536000
	unsigned long wapiUcastRekeyPackets __PACK__; //1048576
//internal use
	unsigned char wapiSearchCertInfo[32] __PACK__; //search info
	unsigned char wapiSearchIndex __PACK__; // search type index
	unsigned char wapiCAInit   __PACK__; //init CA
#endif
	unsigned char STBCEnabled __PACK__; //new add for Wifi
	unsigned char CoexistEnabled __PACK__; //new add for Wifi
	unsigned char vlan[VLAN_CONFIG_LEN] __PACK__; //new add for VLAN
};

struct config_mib_all {
	struct sys_config_mib sys_config;
	struct eth_config_mib eth[MAX_ETH_INTF];
	struct wlan_comm_config_mib wlan_comm;
	struct wlan_config_mib wlan[MAX_WLAN_INTF];
};
                                                  
//                                                  
typedef enum  {                 
	BYTE_T,							// byte
	WORD_T,						// word	
	INT_T,							// int
	INT_BIT_T,					// int in bitmask value
	BYTE5_T,						// 6 bytes
	BYTE6_T,						// 6 bytes
	STRING_T,						// string
	BYTE13_T,					// 13 bytes
	LAST_ENTRY
} TYPE_T;   

typedef enum  {                 
	ACT_SYS_MIB_RW,				// system MIB read/write	
	ACT_WLAN_MIB_RW,				// wlan MIB read/write		
	ACT_IO_MIB_RW 	// MIB read/write + ioctl , usually it's control interface mib and need sync to IF driver immedeately		
} ACTION_T;                                                  

// CMD table for mib config access
struct config_cmd_entry {                          
	int 				id;				// cmd id
	char 			*name;		// cmd name in string                 	
	TYPE_T 		type;			// cmd value type
	int			 	offset;		// offset in struct mib      
	char 			*def;			// default value
	int				start, end;	// value range 
	ACTION_T	action;		// action flag
};    	

#endif // INCLUDE_MIB_H

