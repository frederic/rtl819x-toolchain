/*
  *   MIB header file for RTL8197B
  *
  *	Copyright (C)2008, Realtek Semiconductor Corp. All rights reserved.
  *
  *	$Id: mib.h,v 1.13 2011/02/14 07:50:28 marklee Exp $
  */

#ifndef INCLUDE_MIB_H
#define INCLUDE_MIB_H

#include "apmib.h"

/*================================================================*/
//#define __PACK__  // no use 

/* Constant Definitions */
#define IP_ADDR_SLEN 16
#define MAX_INTF_NAME 10

#define MAX_WLAN_AP_INTF 8 
#define MAX_WLAN_CLIENT_INTF 1
#define MAX_WLAN_INTF (MAX_WLAN_AP_INTF + MAX_WLAN_CLIENT_INTF)
#define WLAN_VXD_INDEX  MAX_WLAN_INTF - 1

#define MAX_ETH_INTF 6
#define VLAN_CONFIG_LEN 30
#define MAC_ADDR_SLEN    13
#define VLAN_CONFIG_NUM 6
#define ACL_LIST_NUM 32
#define AUTH_SER_SECRE_LEN 30
#define PIN_SLEN 8+1
#define MAX_IF_STA_NUM 256 

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
#define MIB_WLAN_STA_NUM 	79 //mark_sta
// for WMM
#define MIB_WLAN_WMM_ENABLED 89

#define MIB_IP_ADDR			170
#define MIB_SUBNET_MASK			171


// for WPS
#define MIB_WPS_DISABLE 		270
#define MIB_WPS_METHOD			271
#define MIB_WPS_CONFIGURED		272
#define MIB_WPS_PIN				273
#define MIB_WPS_AUTH			274
#define MIB_WPS_ENC				275
#define MIB_WPS_MANUAL_ENABLED 	276
#define MIB_WPS_PSK				277
#define MIB_WPS_SSID			278
#define MIB_WPS_UPNP_ENABLED	279
#define MIB_WPS_REGISTRAR_ENABLED 	280
#define MIB_WPS_CONFIG_BY_EXT_REG 	281
#define MIB_WPS_DEVICENAME 	282


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

#define MIB_WLAN_11N_STBC 646
#define MIB_WLAN_11N_COEXIST 647

#define MIB_WLAN_STA_EXPIRETIME 915

#define MIB_INIT_DEFAULT	916

#define MIB_HOST_MAC 1000
#define MIB_HOST_IP_ADDR 1001
#define MIB_SYSTEM_MODE 1002

enum MIB_MANUAL_EDCA {
	MIB_MANUAL_EDCA = 1010,
	MIB_STA_BKQ_ACM,
	MIB_STA_BKQ_CWMIN,
	MIB_STA_BKQ_CWMAX,
	MIB_STA_BKQ_AIFSN,
	MIB_STA_BKQ_TXOPLIMIT,
	MIB_STA_BEQ_ACM,
	MIB_STA_BEQ_CWMIN,
	MIB_STA_BEQ_CWMAX,
	MIB_STA_BEQ_AIFSN,
	MIB_STA_BEQ_TXOPLIMIT,
	MIB_STA_VIQ_ACM,
	MIB_STA_VIQ_CWMIN,
	MIB_STA_VIQ_CWMAX,
	MIB_STA_VIQ_AIFSN,
	MIB_STA_VIQ_TXOPLIMIT,
	MIB_STA_VOQ_ACM,
	MIB_STA_VOQ_CWMIN,
	MIB_STA_VOQ_CWMAX,
	MIB_STA_VOQ_AIFSN,
	MIB_STA_VOQ_TXOPLIMIT,
	MIB_AP_BKQ_ACM,
	MIB_AP_BKQ_CWMIN,
	MIB_AP_BKQ_CWMAX,
	MIB_AP_BKQ_AIFSN,
	MIB_AP_BKQ_TXOPLIMIT,
	MIB_AP_BEQ_ACM,
	MIB_AP_BEQ_CWMIN,
	MIB_AP_BEQ_CWMAX,
	MIB_AP_BEQ_AIFSN,
	MIB_AP_BEQ_TXOPLIMIT,
	MIB_AP_VIQ_ACM,
	MIB_AP_VIQ_CWMIN,
	MIB_AP_VIQ_CWMAX,
	MIB_AP_VIQ_AIFSN,
	MIB_AP_VIQ_TXOPLIMIT,
	MIB_AP_VOQ_ACM,
	MIB_AP_VOQ_CWMIN,
	MIB_AP_VOQ_CWMAX,
	MIB_AP_VOQ_AIFSN,
	MIB_AP_VOQ_TXOPLIMIT,
};//MIB_AP_VOQ_TXOPLIMIT = 1050



//private IO MIB id
#define id_cmd_timeout						0x201
#define id_mii_pause_enable   		        0x202
#define id_eth_pause_enable   		        0x203
#define id_cpu_suspend_enable             	0x204
#define id_phy_reg_poll_time			    0x205
#define id_set_host_pid						0x100		// set host daemon pid

             
#define FIELD_OFFSET(type, field)	((unsigned long)(long *)&(((type *)0)->field))

//#define WLAN_MIB_OFFSET(field)	((int)FIELD_OFFSET(struct wlan_config_mib,field))
#define WLAN_HW_OFFSET(field)	((int)FIELD_OFFSET(struct hw_setting,field))
#define WLAN_MIB_OFFSET(field)	((int)FIELD_OFFSET(struct config_wlan_setting,field))
#define WLAN_MIBEXT_OFFSET(field)	((int)FIELD_OFFSET(struct wlan_config_mib,field))
#define WLAN_COMM_MIB_OFFSET(field) ((int)FIELD_OFFSET(struct wlan_comm_config_mib,field))
#define ETH_MIB_OFFSET(field)	((int)FIELD_OFFSET(struct eth_config_mib,field))
#define SYS_MIB_OFFSET(field)	((int)FIELD_OFFSET(struct sys_config_mib,field))

#define IO_MIB_OFFSET(field)	((int)FIELD_OFFSET(struct io_config_mib,field))

#define CMD_HW_DEF(id,name, type, mib_name ,def,start, end) \
	{id, name, type, WLAN_HW_OFFSET(mib_name), def ,start, end, ACT_WLAN_MIB_RW}

#define CMD_WLAN_DEF(id,name, type, mib_name ,def,start, end) \
	{id, name, type, WLAN_MIB_OFFSET(mib_name), def ,start, end, ACT_WLAN_MIB_RW}

#define CMD_WLANEXT_DEF(id,name, type, mib_name ,def,start, end) \
	{id, name, type, WLAN_MIBEXT_OFFSET(mib_name), def ,start, end, ACT_WLAN_MIB_RW}

#define CMD_WLAN_COMM_DEF(id,name, type, mib_name ,def,start, end) \
	{id, name, type, WLAN_COMM_MIB_OFFSET(mib_name), def ,start, end, ACT_WLAN_MIB_RW}

#define CMD_ETH_DEF(id,name, type, mib_name ,def,start, end) \
	{id, name, type, ETH_MIB_OFFSET(mib_name), def ,start, end, ACT_SYS_MIB_RW}

#define CMD_SYS_DEF(id,name, type, mib_name ,def,start, end) \
	{id, name, type, SYS_MIB_OFFSET(mib_name), def ,start, end, ACT_SYS_MIB_RW}

#define CMD_IO_DEF(name, type ,def,start, end) \
	{id_##name, #name, type, IO_MIB_OFFSET(name),def, start, end, ACT_IO_MIB_RW}
             
// access mib flag definition
enum ACCESS_TYPE_T {
	ACCESS_MIB_SET=0x01, 
	ACCESS_MIB_GET=0x02, 
	ACCESS_MIB_BY_NAME=0x04, 
	ACCESS_MIB_BY_ID=0x08,
	ACCESS_MIB_SYNC=0x10,
};

/*
enum { WSC_AUTH_OPEN=1, WSC_AUTH_WPAPSK=2, WSC_AUTH_SHARED=4, WSC_AUTH_WPA=8, WSC_AUTH_WPA2=0x10, WSC_AUTH_WPA2PSK=0x20, WSC_AUTH_WPA2PSKMIXED=0x22 };
enum { WSC_ENCRYPT_NONE=1, WSC_ENCRYPT_WEP=2, WSC_ENCRYPT_TKIP=4, WSC_ENCRYPT_AES=8, WSC_ENCRYPT_TKIPAES=12 };
enum { CONFIG_METHOD_ETH=0x2, CONFIG_METHOD_PIN=0x4, CONFIG_METHOD_PBC=0x80 };
enum { CONFIG_BY_INTERNAL_REGISTRAR=1, CONFIG_BY_EXTERNAL_REGISTRAR=2};
*/


// MIB value and constant
// priv use
//#define MAX_PSK_LEN				64
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
	unsigned char init_default __PACK__;
	unsigned char enable_efuse_config __PACK__;
	unsigned char vlan_enable __PACK__;
	unsigned char eth_mac[MAC_ADDR_SLEN] __PACK__;
	unsigned char ip_addr[IP_ADDR_SLEN] __PACK__;
	unsigned char netmask[IP_ADDR_SLEN] __PACK__;
	unsigned char host_mac[MAC_ADDR_SLEN] __PACK__;
	unsigned char host_ip[IP_ADDR_SLEN] __PACK__;
	unsigned char vap_number __PACK__;
#ifdef WIFI_SIMPLE_CONFIG
	unsigned char wscPin[PIN_LEN+1] __PACK__;
	unsigned char wps_devicename[100] __PACK__; //from H
#endif
	unsigned char mode __PACK__;
	unsigned char regDomain; //move from hw_setting
};


struct eth_config_mib {
	unsigned char port_name[3] __PACK__;
	unsigned char name[MAX_INTF_NAME] __PACK__;
	unsigned char vlan[VLAN_CONFIG_LEN] __PACK__;
};


enum qos_prio { BK, BE, VI, VO, VI_AG, VO_AG };

struct wlan_config_mib {
	CONFIG_WLAN_SETTING_T legacy_flash_settings __PACK__;
	unsigned char name[MAX_INTF_NAME] __PACK__; //from Hw
	unsigned char vlan[VLAN_CONFIG_LEN] __PACK__;
	unsigned int stanum __PACK__; //mark_sta
	/* Manual EDCA parameters */
	unsigned char enable_manual_edca __PACK__;
	unsigned char sta_bkq_acm __PACK__;
	unsigned char sta_bkq_cwmin __PACK__;
	unsigned char sta_bkq_cwmax __PACK__;
	unsigned char sta_bkq_aifsn __PACK__;
	unsigned int sta_bkq_txoplimit __PACK__;
	unsigned char sta_beq_acm __PACK__;
	unsigned char sta_beq_cwmin __PACK__;
	unsigned char sta_beq_cwmax __PACK__;
	unsigned char sta_beq_aifsn __PACK__;
	unsigned int sta_beq_txoplimit __PACK__;
	unsigned char sta_viq_acm __PACK__;
	unsigned char sta_viq_cwmin __PACK__;
	unsigned char sta_viq_cwmax __PACK__;
	unsigned char sta_viq_aifsn __PACK__;
	unsigned int sta_viq_txoplimit __PACK__;
	unsigned char sta_voq_acm __PACK__;
	unsigned char sta_voq_cwmin __PACK__;
	unsigned char sta_voq_cwmax __PACK__;
	unsigned char sta_voq_aifsn __PACK__;
	unsigned int sta_voq_txoplimit __PACK__;
	unsigned char ap_bkq_acm __PACK__;
	unsigned char ap_bkq_cwmin __PACK__;
	unsigned char ap_bkq_cwmax __PACK__;
	unsigned char ap_bkq_aifsn __PACK__;
	unsigned int ap_bkq_txoplimit __PACK__;
	unsigned char ap_beq_acm __PACK__;
	unsigned char ap_beq_cwmin __PACK__;
	unsigned char ap_beq_cwmax __PACK__;
	unsigned char ap_beq_aifsn __PACK__;
	unsigned int ap_beq_txoplimit __PACK__;
	unsigned char ap_viq_acm __PACK__;
	unsigned char ap_viq_cwmin __PACK__;
	unsigned char ap_viq_cwmax __PACK__;
	unsigned char ap_viq_aifsn __PACK__;
	unsigned int ap_viq_txoplimit __PACK__;
	unsigned char ap_voq_acm __PACK__;
	unsigned char ap_voq_cwmin __PACK__;
	unsigned char ap_voq_cwmax __PACK__;
	unsigned char ap_voq_aifsn __PACK__;
	unsigned int ap_voq_txoplimit __PACK__;
};

struct config_mib_all {
	HW_SETTING_T hw_config;
	struct sys_config_mib sys_config;
	struct eth_config_mib eth[MAX_ETH_INTF];
	struct wlan_config_mib wlan[MAX_WLAN_INTF];
};

typedef enum  {                 
	AP_VAP_MODE,							
	REPEATER_AP_MODE,						
	PURE_REPEATER_MODE,						
	PURE_CLIENT_MODE,					
	RTK_WDS_MODE,						
} SYSMODE_T;   
                                                  
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
	INETADDR_T,
	ACMACARRAY_T,
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

