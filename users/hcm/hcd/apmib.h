/*
 *      Header file of AP mib
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *
 *      $Id: apmib.h,v 1.2 2011/01/05 08:51:33 marklee Exp $
 *
 */


#ifndef INCLUDE_APMIB_H
#define INCLUDE_APMIB_H

/* Forrest, 2007.11.07.
 * If you want to let APMIB adopt shared memory mechanism, define it to 1.
 * Or define it to 0 will go back to original local copy mechanism.
 * Note: 
 *   1. I only use shared memory for HW Configuration (pHwSetting), Default 
 *      Configuration (pMibDef) and Current Configuration (pMib). There is
 *      no shared memory for each linkchain now.
 *   2. Because uClibc does not support POSIX inter-process semaphore, I have
 *      to use SYSTEM V semaphore and shared memory. So if you want to adopt
 *      shared memory mechanism, you must go to turn on CONFIG_SYSVIPC 
 *      kernel config to support it.
 */
#ifdef CONFIG_APMIB_SHARED 
	#define CONFIG_APMIB_SHARED_MEMORY	1
#else
	#define CONFIG_APMIB_SHARED_MEMORY	0
#endif

// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT
#include "voip_flash.h"
#include "voip_flash_mib.h"
#endif

#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT)
//#define LOGDEBUG_ENABLED
#endif


#if !defined(CONFIG_RTL8196C_CLIENT_ONLY)
#define MBSSID
#endif
//#ifdef HOME_GATEWAY
//#define GW_QOS_ENGINE
//#endif
#ifdef ENABLE_QOS // thru makefile
	#ifndef QOS_BY_BANDWIDTH
		#define GW_QOS_ENGINE
	#endif
#endif


#define NUM_WLAN_INTERFACE		1	// number of wlan interface supported


#if defined(CONFIG_RTL8196B)
#define NUM_WLAN_MULTIPLE_SSID	8	// number of wlan ssid support
#else
//!CONFIG_RTL8196B => rtl8651c+rtl8190
#define NUM_WLAN_MULTIPLE_SSID	5	// number of wlan ssid support
#endif

#ifdef MBSSID
#ifdef CONFIG_RTL8196C_AP_HCM
#define NUM_VWLAN		8	// number of virtual wlan interface supported
#else
#define NUM_VWLAN		4	// number of virtual wlan interface supported
#endif
#else
#define NUM_VWLAN		0
#endif

#ifdef UNIVERSAL_REPEATER
#define NUM_VWLAN_INTERFACE		NUM_VWLAN+1
#else
#define NUM_VWLAN_INTERFACE		NUM_VWLAN
#endif

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
#ifdef TLS_CLIENT
//sc_yang for client mode TLS
#define MIB_CERTROOT_NUM			79
#define MIB_CERTROOT			80
#define MIB_CERTROOT_ADD			81	
#define MIB_CERTROOT_DEL			82
#define MIB_CERTROOT_DELALL		83
#define MIB_CERTUSER_NUM			84
#define MIB_CERTUSER			85
#define MIB_CERTUSER_ADD			86	
#define MIB_CERTUSER_DEL			87
#define MIB_CERTUSER_DELALL		88
#endif

// for WMM
#define MIB_WLAN_WMM_ENABLED 89

#ifdef WLAN_EASY_CONFIG
#define MIB_WLAN_EASYCFG_ENABLED	90
#define MIB_WLAN_EASYCFG_MODE		91
#define MIB_WLAN_EASYCFG_SSID		92
#define MIB_WLAN_EASYCFG_KEY		93
#define MIB_WLAN_EASYCFG_ALG_REQ	94
#define MIB_WLAN_EASYCFG_ALG_SUPP	95
#define MIB_WLAN_EASYCFG_DIGEST		96
#define MIB_WLAN_EASYCFG_ROLE		97
#define MIB_WLAN_EASYCFG_SCAN_SSID	98
#define MIB_WLAN_EASYCFG_WLAN_MODE	99
#endif // WLAN_EASY_CONFIG


#ifdef HOME_GATEWAY
#define MIB_WAN_MAC_ADDR		100
#define MIB_WAN_IP_ADDR			101
#define MIB_WAN_SUBNET_MASK		102
#define MIB_WAN_DEFAULT_GATEWAY		103
#define MIB_WAN_DHCP			104
#define MIB_WAN_DNS_MODE		105
#define MIB_PPP_USER			106
#define MIB_PPP_PASSWORD		107
#define MIB_PPP_IDLE_TIME		108
#define MIB_PPP_CONNECT_TYPE		109
#define MIB_PORTFW_ENABLED		110
#define MIB_PORTFW_NUM			111
#define MIB_PORTFW			112
#define MIB_PORTFW_ADD			113
#define MIB_PORTFW_DEL			114
#define MIB_PORTFW_DELALL		115
#define MIB_IPFILTER_ENABLED		116
#define MIB_IPFILTER_NUM		117
#define MIB_IPFILTER			118
#define MIB_IPFILTER_ADD		119
#define MIB_IPFILTER_DEL		120
#define MIB_IPFILTER_DELALL		121
#define MIB_MACFILTER_ENABLED		122
#define MIB_MACFILTER_NUM		123
#define MIB_MACFILTER			124
#define MIB_MACFILTER_ADD		125
#define MIB_MACFILTER_DEL		126
#define MIB_MACFILTER_DELALL		127
#define MIB_PORTFILTER_ENABLED		128
#define MIB_PORTFILTER_NUM		129
#define MIB_PORTFILTER			130
#define MIB_PORTFILTER_ADD		131
#define MIB_PORTFILTER_DEL		132
#define MIB_PORTFILTER_DELALL		133
#define MIB_TRIGGERPORT_ENABLED		134
#define MIB_TRIGGERPORT_NUM		135
#define MIB_TRIGGERPORT			136
#define MIB_TRIGGERPORT_ADD		137
#define MIB_TRIGGERPORT_DEL		138
#define MIB_TRIGGERPORT_DELALL		139
#define MIB_DMZ_ENABLED			140
#define MIB_DMZ_HOST			141
#define MIB_UPNP_ENABLED		142
#define MIB_UPNP_IGD_NAME		143
#define MIB_PPP_MTU_SIZE		144
#define MIB_PPTP_IP			145
#define MIB_PPTP_SUBNET_MASK		146
#define MIB_PPTP_SERVER_IP		147
#define MIB_PPTP_USER			148
#define MIB_PPTP_PASSWORD		149
#define MIB_PPTP_MTU_SIZE		150
#define MIB_NTP_ENABLED 		151
#define MIB_NTP_SERVER_ID 		152
#define MIB_NTP_TIMEZONE 		153
#define MIB_NTP_SERVER_IP1		154
#define MIB_NTP_SERVER_IP2		155
#define MIB_PPTP_SECURITY_ENABLED 	156
#define MIB_FIXED_IP_MTU_SIZE	157
#define MIB_DHCP_MTU_SIZE		158
#define MIB_PPTP_MPPC_ENABLED	159

#ifdef VPN_SUPPORT
#define MIB_IPSECTUNNEL_ENABLED		160
#define MIB_IPSECTUNNEL_NUM		161
#define MIB_IPSECTUNNEL			162
#define MIB_IPSECTUNNEL_ADD		163
#define MIB_IPSECTUNNEL_DEL		165
#define MIB_IPSECTUNNEL_DELALL		166
#define MIB_IPSEC_NATT_ENABLED		167
#define MIB_IPSEC_RSA_FILE 		168
#endif
#endif // HOME_GATEWAY

#define MIB_IP_ADDR			170
#define MIB_SUBNET_MASK			171
#define MIB_DEFAULT_GATEWAY		172
#define MIB_DHCP			173
#define MIB_DHCP_CLIENT_START		174
#define MIB_DHCP_CLIENT_END		175
#define MIB_WAN_DNS1			176
#define MIB_WAN_DNS2			177
#define MIB_WAN_DNS3			178
#define MIB_STP_ENABLED			179
#define MIB_SUPER_NAME			180
#define MIB_SUPER_PASSWORD		181
#define MIB_USER_NAME			182
#define	MIB_USER_PASSWORD		183
#define MIB_LOG_ENABLED			184
#define MIB_AUTO_DISCOVERY_ENABLED	185
#define MIB_DEVICE_NAME			186

#ifdef HOME_GATEWAY			
#define MIB_DDNS_ENABLED		187
#define MIB_DDNS_TYPE			188
#define MIB_DDNS_DOMAIN_NAME		189
#define MIB_DDNS_USER			190
#define MIB_DDNS_PASSWORD		191
#endif
#define MIB_OP_MODE			192
#define MIB_WISP_WAN_ID			193

#ifdef HOME_GATEWAY
#define	WEB_WAN_ACCESS_ENABLED		194
#define	PING_WAN_ACCESS_ENABLED		195
#define MIB_HOST_NAME			197
#endif

#define MIB_DOMAIN_NAME			198


// Hardware setting MIB
#define MIB_HW_BOARD_VER		200
#define MIB_HW_NIC0_ADDR		201
#define MIB_HW_NIC1_ADDR		202
#define MIB_HW_WLAN_ADDR		203
#define MIB_HW_REG_DOMAIN		204
#define MIB_HW_RF_TYPE			205
#define MIB_HW_TX_POWER_CCK		206
#define MIB_HW_TX_POWER_OFDM		207
#define MIB_HW_ANT_DIVERSITY		208
#define MIB_HW_TX_ANT			209
#define MIB_HW_CCA_MODE			210
#define MIB_HW_PHY_TYPE			211
#define MIB_HW_LED_TYPE			212
#define MIB_HW_INIT_GAIN		213


#ifdef TLS_CLIENT
#define MIB_ROOT_IDX			214
#define MIB_USER_IDX			215
#endif
#ifdef ROUTE_SUPPORT
#define MIB_STATICROUTE_ENABLED         216
#define MIB_STATICROUTE                 217
#define MIB_STATICROUTE_ADD             218
#define MIB_STATICROUTE_DEL             219
#define MIB_STATICROUTE_DELALL          220
#define MIB_STATICROUTE_NUM             221
#define MIB_RIP_ENABLED			222
#define MIB_RIP_LAN_TX 			223
#define MIB_RIP_LAN_RX 			224
#define MIB_RIP_WAN_TX 			225
#define MIB_RIP_WAN_RX 			226
#endif

#define MIB_REMOTELOG_ENABLED 		227
#define MIB_REMOTELOG_SERVER		228

#ifdef HOME_GATEWAY
#ifdef DOS_SUPPORT
#define MIB_DOS_ENABLED 			229
#define MIB_DOS_SYSSYN_FLOOD 		230
#define MIB_DOS_SYSFIN_FLOOD 		231
#define MIB_DOS_SYSUDP_FLOOD 		232
#define MIB_DOS_SYSICMP_FLOOD 	233
#define MIB_DOS_PIPSYN_FLOOD 		234
#define MIB_DOS_PIPFIN_FLOOD 		235
#define MIB_DOS_PIPUDP_FLOOD 		236
#define MIB_DOS_PIPICMP_FLOOD 		237
#define MIB_DOS_BLOCK_TIME 		238
#endif
#define MIB_URLFILTER_ENABLED		239
#define MIB_URLFILTER_NUM			240
#define MIB_URLFILTER				241
#define MIB_URLFILTER_ADD			242
#define MIB_URLFILTER_DEL			243
#define MIB_URLFILTER_DELALL		244
#define VPN_PASSTHRU_IPSEC_ENABLED	245
#define VPN_PASSTHRU_PPTP_ENABLED	246
#define VPN_PASSTHRU_L2TP_ENABLED	247
#endif

//#ifdef SNMP_SUPPORT Keith remove
#define MIB_SNMP_RO_COMMUNITY           248
#define MIB_SNMP_RW_COMMUNITY           249
//#endif Keith remove
#ifdef UNIVERSAL_REPEATER
#define MIB_REPEATER_ENABLED1		250
#define MIB_REPEATER_SSID1		251
#define MIB_REPEATER_ENABLED2		252
#define MIB_REPEATER_SSID2		253
#endif

// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT
#define MIB_VOIP_CFG			254
#endif // VOIP_SUPPORT

#define MIB_WIFI_SPECIFIC		260

#ifdef HOME_GATEWAY
#define MIB_PPP_SERVICE			261
#endif

#define MIB_TURBO_MODE			262
#define MIB_WLAN_RF_POWER		263

#ifdef WIFI_SIMPLE_CONFIG
#define MIB_WSC_DISABLE 		270
#define MIB_WSC_METHOD			271
#define MIB_WSC_CONFIGURED		272
#define MIB_WSC_PIN				273
#define MIB_WSC_AUTH			274
#define MIB_WSC_ENC				275
#define MIB_WSC_MANUAL_ENABLED 	276
#define MIB_WSC_PSK				277
#define MIB_WSC_SSID			278
#define MIB_WSC_UPNP_ENABLED	279
#define MIB_WSC_REGISTRAR_ENABLED 	280
#define MIB_WSC_CONFIG_BY_EXT_REG 	281
#endif
//Brad addd
#define MIB_DAYLIGHT_SAVE		282
#define MIB_IGMP_PROXY_DISABLED		283
//Brad add for 11N
#define MIB_WLAN_CHANNEL_BONDING 284
#define MIB_WLAN_CONTROL_SIDEBAND 285
#define MIB_WLAN_AGGREGATION 286
#define MIB_WLAN_SHORT_GI 287
//Brad define 
#define MIB_WLAN_WEP64_KEY		288
#define MIB_WLAN_WEP128_KEY		289

#define MIB_HW_11N_XCAP			290
#define MIB_HW_11N_RXIMR		291
#define MIB_DHCPRSVDIP_ENABLED		292
#define MIB_DHCPRSVDIP			293
#define MIB_DHCPRSVDIP_ADD       	294
#define MIB_DHCPRSVDIP_DEL        	295
#define MIB_DHCPRSVDIP_DELALL	296
#define MIB_DHCPRSVDIP_NUM		297
#define MIB_HW_11N_LOFDMPWD    298
#define MIB_HW_11N_ANTPWD_B     299
#define MIB_HW_11N_ANTPWD_C     300
#define MIB_HW_11N_ANTPWD_D     301
#define MIB_HW_11N_THER_RFIC        302
#define MIB_HW_WLAN_ADDR1		303
#define MIB_HW_WLAN_ADDR2		304
#define MIB_HW_WLAN_ADDR3		305
#define MIB_HW_WLAN_ADDR4		306

// GW_QOS_ENGINE
#define MIB_QOS_ENABLED		       307
#define MIB_QOS_AUTO_UPLINK_SPEED      308
#define MIB_QOS_MANUAL_UPLINK_SPEED    309
#define MIB_QOS_RULE_NUM	       310
#define MIB_QOS_RULE		       311
#define MIB_QOS_ADD		       312
#define MIB_QOS_DEL		             313
#define MIB_QOS_DELALL		       314

#define MIB_WLAN_ACCESS			315
#define MIB_WLAN_PRIORITY		316

#define MIB_START_MP_DAEMON	317

#define MIB_TIME_YEAR	321
#define MIB_TIME_MONTH	322
#define MIB_TIME_DAY		323
#define MIB_TIME_HOUR	324
#define MIB_TIME_MIN		325
#define MIB_TIME_SEC		326

/* # keith: add l2tp support. 20080515 */
#define MIB_L2TP_IP					331
#define MIB_L2TP_SUBNET_MASK			332
#define MIB_L2TP_SERVER_IP			333
#define MIB_L2TP_USER				334
#define MIB_L2TP_PASSWORD			335
#define MIB_L2TP_MTU_SIZE			336
#define MIB_L2TP_CONNECTION_TYPE	337
#define MIB_L2TP_IDLE_TIME 			338

/*+++++added by Jack for Tr-069 configuration+++++*/
#ifdef CONFIG_CWMP_TR069
#define CWMP_ID						400
#define CWMP_PROVISIONINGCODE				CWMP_ID + 1	//069
#define CWMP_ACS_URL					CWMP_ID + 2			//069
#define CWMP_ACS_USERNAME				CWMP_ID + 3		//069
#define CWMP_ACS_PASSWORD				CWMP_ID + 4		//069
#define CWMP_INFORM_ENABLE				CWMP_ID + 5		//069
#define CWMP_INFORM_INTERVAL				CWMP_ID + 6		//069
#define CWMP_INFORM_TIME				CWMP_ID + 7
#define CWMP_CONREQ_USERNAME				CWMP_ID + 8
#define CWMP_CONREQ_PASSWORD				CWMP_ID + 9
#define CWMP_ACS_UPGRADESMANAGED			CWMP_ID + 10	//069
#define CWMP_LAN_CONFIGPASSWD				CWMP_ID + 11
#define CWMP_SERIALNUMBER				CWMP_ID + 12		//069
#define CWMP_DHCP_SERVERCONF				CWMP_ID + 13
#define CWMP_LAN_IPIFENABLE				CWMP_ID + 14
#define CWMP_LAN_ETHIFENABLE				CWMP_ID + 15
#define CWMP_WLAN_BASICENCRY				CWMP_ID + 16
#define CWMP_WLAN_WPAENCRY				CWMP_ID + 17
#define CWMP_DL_COMMANDKEY				CWMP_ID + 18
#define CWMP_DL_STARTTIME				CWMP_ID + 19
#define CWMP_DL_COMPLETETIME				CWMP_ID + 20
#define CWMP_DL_FAULTCODE				CWMP_ID + 21
#define CWMP_INFORM_EVENTCODE				CWMP_ID + 22
#define CWMP_RB_COMMANDKEY				CWMP_ID + 23
#define CWMP_ACS_PARAMETERKEY				CWMP_ID + 24
#define CWMP_CERT_PASSWORD				CWMP_ID + 25
#define CWMP_FLAG					CWMP_ID + 26
#define CWMP_SI_COMMANDKEY				CWMP_ID + 38	/*ScheduleInform's commandkey*/

/* define the len of the entities */
#define CWMP_PROVISION_CODE_LEN	64
#define CWMP_ACS_URL_LEN	256
#define CWMP_ACS_USERNAME_LEN	256
#define CWMP_ACS_PASSWD_LEN	256
#define CWMP_CONREQ_USERNAME_LEN	256
#define CWMP_CONREQ_PASSWD_LEN	256
#define CWMP_LANCONF_PASSWD_LEN	64
#define CWMP_SERIALNUMBER_LEN	64
#define CWMP_COMMAND_KEY_LEN	32
#define CWMP_CERT_PASSWD_LEN	64
/* define the cwmp_flag */
#define CWMP_FLAG_DEBUG_MSG	0x01
#define CWMP_FLAG_CERT_AUTH      0x02
#define CWMP_FLAG_SENDGETRPC    0x04
#define CWMP_FLAG_SKIPMREBOOT   0x08
#define CWMP_FLAG_DELAY         	0x10
#define CWMP_FLAG_AUTORUN          0x20
#endif /*CONFIG_CWMP_TR069*/

// SNMP, Forrest added, 2007.10.25.
#ifdef CONFIG_SNMP
#define MIB_SNMP_ENABLED		435
#define MIB_SNMP_NAME			436
#define MIB_SNMP_LOCATION		437
#define MIB_SNMP_CONTACT		438
#define MIB_SNMP_RWCOMMUNITY	439
#define MIB_SNMP_ROCOMMUNITY	440
#define MIB_SNMP_TRAP_RECEIVER1	441
#define MIB_SNMP_TRAP_RECEIVER2	442
#define MIB_SNMP_TRAP_RECEIVER3	443
#endif

/* # keith: add l2tp support. 20080515 */
#define MIB_L2TP_WAN_IP_DYNAMIC		501
#define MIB_L2TP_GATEWAY		502
//SCHEDULE
#define MIB_SCHEDULE						503
#define MIB_SCHEDULE_ENABLED		504
#define MIB_SCHEDULE_NUM			505
#define MIB_SCHEDULE_ADD			506
#define MIB_SCHEDULE_DEL				507
#define MIB_SCHEDULE_DELALL		508

#define MIB_PPTP_CONNECTION_TYPE	509
#define MIB_PPTP_IDLE_TIME 			510


//NewAdd For rtl8196B
#define MIB_HW_WLAN_ADDR5		511
#define MIB_HW_WLAN_ADDR6		512
#define MIB_HW_WLAN_ADDR7		513

#define MIB_HW_TX_POWER_OFDM_1S 514
#define MIB_HW_TX_POWER_OFDM_2S 515
#define MIB_HW_11N_LOFDMPWDA    516
#define MIB_HW_11N_LOFDMPWDB    517
#define MIB_HW_11N_TSSI1     518
#define MIB_HW_11N_TSSI2     519
#define MIB_HW_11N_THER 520
#define MIB_HW_11N_RESERVED1 521
#define MIB_HW_11N_RESERVED2 522
#define MIB_HW_11N_RESERVED3 523
#define MIB_HW_11N_RESERVED4 524
#define MIB_HW_11N_RESERVED5 525
#define MIB_HW_11N_RESERVED6 526
#define MIB_HW_11N_RESERVED7 527
#define MIB_HW_11N_RESERVED8 528

/*-----end-----*/

// SNMP maximum length of fields, Forrest added, 2007.10.25.
#ifdef CONFIG_SNMP
#define MAX_SNMP_NAME_LEN		64
#define MAX_SNMP_LOCATION_LEN		64
#define MAX_SNMP_CONTACT_LEN		64
#define MAX_SNMP_COMMUNITY_LEN          64
#endif

//=========add for MESH=========
//#ifdef CONFIG_RTK_MESH Keith remove
#define MIB_WLAN_MESH_ENABLE    644	//new feature:Mesh enable/disable
#define MIB_MESH_ROOT_ENABLE    551
#define MIB_MESH_ID				554
#define MIB_MESH_MAX_NEIGHTBOR	555
#define MIB_MESH_ENCRYPT		559
#define MIB_MESH_WPA_PSK_FORMAT	560
#define MIB_MESH_WPA_PSK		561
#define MIB_MESH_WPA_AUTH		562
#define MIB_MESH_WPA2_CIPHER_SUITE	563

//#ifdef _MESH_ACL_ENABLE_ Keith remove
#define MIB_MESH_ACL_ENABLED		580
#define MIB_MESH_ACL_NUM			581
#define MIB_MESH_ACL_ADDR			582
#define MIB_MESH_ACL_ADDR_ADD		583
#define MIB_MESH_ACL_ADDR_DEL		584
#define MIB_MESH_ACL_ADDR_DELALL	585
//#endif Keith remove

//#ifdef 	_11s_TEST_MODE_	 Keith remove
#define MIB_MESH_TEST_PARAM1		600
#define MIB_MESH_TEST_PARAM2		601
#define MIB_MESH_TEST_PARAM3		602
#define MIB_MESH_TEST_PARAM4		603
#define MIB_MESH_TEST_PARAM5		604
#define MIB_MESH_TEST_PARAM6		605
#define MIB_MESH_TEST_PARAM7		606
#define MIB_MESH_TEST_PARAM8		607
#define MIB_MESH_TEST_PARAM9		608
#define MIB_MESH_TEST_PARAMA		609
#define MIB_MESH_TEST_PARAMB		610
#define MIB_MESH_TEST_PARAMC		611
#define MIB_MESH_TEST_PARAMD		612
#define MIB_MESH_TEST_PARAME		613
#define MIB_MESH_TEST_PARAMF		614
#define MIB_MESH_TEST_PARAMSTR1		615
//#endif Keith remove
//#endif // CONFIG_RTK_MESH Keith remove
//=========add for MESH=========

#define MIB_VLANCONFIG_ENABLED		616
#define MIB_VLANCONFIG_NUM		617
#define MIB_VLANCONFIG			618
#define MIB_VLANCONFIG_ADD       	619
#define MIB_VLANCONFIG_DEL        	620
#define MIB_VLANCONFIG_DELALL	621

//#ifdef CONFIG_RTL_WAPI_SUPPORT Keith remove
//WAPI start from 630
#define MIB_WLAN_WAPI_PSK   				630
#define MIB_WLAN_WAPI_PSKLEN   			631
#define MIB_WLAN_WAPI_PSK_FORMAT		632
#define MIB_WLAN_WAPI_AUTH				633
#define MIB_WLAN_WAPI_ASIPADDR    			634
#define MIB_WLAN_WAPI_SEARCH_CERTINFO    635
#define MIB_WLAN_WAPI_SEARCH_CERTINDEX    636
#define MIB_WLAN_WAPI_MCAST_REKEYTYPE    637
#define MIB_WLAN_WAPI_MCAST_TIME    638
#define MIB_WLAN_WAPI_MCAST_PACKETS    639
#define MIB_WLAN_WAPI_UCAST_REKETTYPE    640
#define MIB_WLAN_WAPI_UCAST_TIME    641
#define MIB_WLAN_WAPI_UCAST_PACKETS    642
#define MIB_WLAN_WAPI_CA_INIT                 643

#define MIB_NAT_ENABLED 645
#define MIB_WLAN_11N_STBC 646
#define MIB_WLAN_11N_COEXIST 647

#define CERTS_DATABASE  "/var/myca/index.txt"
#define ONE_DAY_SECONDS 86400
#define USER_NAME_LEN 32
typedef struct _CertsDbEntry_ {
        unsigned char userName[USER_NAME_LEN];  //user name of this user cert
        unsigned long serial;                   //serial of this cert
        unsigned short validDays;               //total valid days of this cert
        unsigned short validDaysLeft;           //the left valid days of this cert
        unsigned char certType;                 //0(default): X.509; others: reserved
        unsigned char certStatus;               //0(default): valid; 1: expired; 2: revoked
} CERTS_DB_ENTRY_T, *CERTS_DB_ENTRY_Tp;

//#endif Keith remove
//WAPI END

// GW_QOS_ENGINE
#define MIB_QOS_AUTO_DOWNLINK_SPEED      650
#define MIB_QOS_MANUAL_DOWNLINK_SPEED      651

//11n onoff TKIP
#define MIB_WLAN_11N_ONOFF_TKIP		660

/*+++++added by Jack for Tr-069 configuration+++++*/
#ifdef CONFIG_CWMP_TR069
#define CWMP_ID						700 //The value of CWMP_ID is ON or OFF tr069
#define CWMP_PROVISIONINGCODE				CWMP_ID + 1	//069
#define CWMP_ACS_URL					CWMP_ID + 2			//069
#define CWMP_ACS_USERNAME				CWMP_ID + 3		//069
#define CWMP_ACS_PASSWORD				CWMP_ID + 4		//069
#define CWMP_INFORM_ENABLE				CWMP_ID + 5		//069
#define CWMP_INFORM_INTERVAL				CWMP_ID + 6		//069
#define CWMP_INFORM_TIME				CWMP_ID + 7
#define CWMP_CONREQ_USERNAME				CWMP_ID + 8
#define CWMP_CONREQ_PASSWORD				CWMP_ID + 9
#define CWMP_ACS_UPGRADESMANAGED			CWMP_ID + 10	//069
//#define CWMP_LAN_CONFIGPASSWD				CWMP_ID + 11
//#define CWMP_SERIALNUMBER				CWMP_ID + 12		//069
//#define CWMP_DHCP_SERVERCONF				CWMP_ID + 13
//#define CWMP_LAN_IPIFENABLE				CWMP_ID + 14
//#define CWMP_LAN_ETHIFENABLE				CWMP_ID + 15
//#define CWMP_WLAN_BASICENCRY				CWMP_ID + 16
//#define CWMP_WLAN_WPAENCRY				CWMP_ID + 17
#define CWMP_DL_COMMANDKEY				CWMP_ID + 18
#define CWMP_DL_STARTTIME				CWMP_ID + 19
#define CWMP_DL_COMPLETETIME				CWMP_ID + 20
#define CWMP_DL_FAULTCODE				CWMP_ID + 21
#define CWMP_INFORM_EVENTCODE				CWMP_ID + 22
#define CWMP_RB_COMMANDKEY				CWMP_ID + 23
//#define CWMP_ACS_PARAMETERKEY				CWMP_ID + 24
#define CWMP_CERT_PASSWORD				CWMP_ID + 25
#define CWMP_FLAG					CWMP_ID + 26
#define CWMP_SI_COMMANDKEY				CWMP_ID + 27	/*ScheduleInform's commandkey*/

#ifdef _PRMT_USERINTERFACE_						/*InternetGatewayDevice.UserInterface.*/
#define UIF_PW_REQUIRED					CWMP_ID + 28 	/*PasswordRequired*/
#define UIF_PW_USER_SEL					CWMP_ID + 29	/*PasswordUserSelectable*/
#define UIF_UPGRADE					CWMP_ID + 30	/*UpgradeAvailable*/
#define UIF_WARRANTYDATE				CWMP_ID + 31	/*WarrantyDate*/
#define UIF_AUTOUPDATESERVER				CWMP_ID + 32	/*AutoUpdateServer*/
#define UIF_USERUPDATESERVER				CWMP_ID + 33	/*UserUpdateServer*/
#endif /*_PRMT_USERINTERFACE_*/

#define CWMP_ACS_KICKURL				CWMP_ID + 74
#define CWMP_ACS_DOWNLOADURL				CWMP_ID + 75
#define CWMP_CONREQ_PORT				CWMP_ID + 76 /*port for connection request*/
#define CWMP_CONREQ_PATH				CWMP_ID + 77 /*path for connection request*/
#define CWMP_FLAG2					CWMP_ID + 78 

//#ifdef _PRMT_TR143_
#define TR143_UDPECHO_ENABLE				CWMP_ID + 79
#define TR143_UDPECHO_ITFTYPE				CWMP_ID + 80
#define TR143_UDPECHO_SRCIP				CWMP_ID + 81
#define TR143_UDPECHO_PORT				CWMP_ID + 82
#define TR143_UDPECHO_PLUS				CWMP_ID + 83
//#endif //_PRMT_TR143_
#define CWMP_MIB_END				CWMP_ID + 200	/* Reserve 200 mib for tr069*/

/* define the len of the entities */
#define CWMP_PROVISION_CODE_LEN	32 //64 in spec
#define CWMP_ACS_URL_LEN	64 //256 in spec
#define CWMP_ACS_USERNAME_LEN	32 //256 in spec
#define CWMP_ACS_PASSWD_LEN	32 //256 in spec
#define CWMP_CONREQ_USERNAME_LEN	32 //256 in spec
#define CWMP_CONREQ_PASSWD_LEN	32 //256 in spec
#define CONN_REQ_PATH_LEN	32 //32 in spec
#define CWMP_KICK_URL 32 //64 in spec
#define CWMP_DOWNLOAD_URL 32 //64 in spec
//#define CWMP_LANCONF_PASSWD_LEN	64
//#define CWMP_SERIALNUMBER_LEN	64
#define CWMP_COMMAND_KEY_LEN	32
#define CWMP_CERT_PASSWD_LEN	32 //64 in spec
#define IP_ADDR_LEN 4
/* define the cwmp_flag */
#define CWMP_FLAG_DEBUG_MSG	0x01
#define CWMP_FLAG_CERT_AUTH      0x02
#define CWMP_FLAG_SENDGETRPC    0x04
#define CWMP_FLAG_SKIPMREBOOT   0x08
#define CWMP_FLAG_DELAY         	0x10
#define CWMP_FLAG_AUTORUN          0x20
#define CWMP_FLAG_CTINFORMEXT	0x40
#define CWMP_FLAG_SELFREBOOT    0x80
/*flag for CWMP_FLAG2 setting*/
#define CWMP_FLAG2_DIS_CONREQ_AUTH		0x01  /*disable connection request authentication*/
#define CWMP_FLAG2_DEFAULT_WANIP_IN_INFORM	0x02  /*bring the default wan ip in the inform*/

/*action type for applying new values*/
#define CWMP_NONE		0
#define CWMP_START		1
#define CWMP_STOP		2
#define CWMP_RESTART		3

/*EC_xxxxx event must consist with those defined in cwmp_rpc.h*/
#define EC_X_CT_COM_ACCOUNT	0x10000	/*X_CT-COM_ACCOUNTCHANGE*/

#endif /*CONFIG_CWMP_TR069*/


/*new add since new platform rtl8196c or rtl8198*/
#define MIB_HW_TX_POWER_CCK_A 901
#define MIB_HW_TX_POWER_CCK_B 902
#define MIB_HW_TX_POWER_HT40_1S_A 903
#define MIB_HW_TX_POWER_HT40_1S_B 904
#define MIB_HW_TX_POWER_HT40_2S 905
#define MIB_HW_TX_POWER_HT20 906
#define MIB_HW_TX_POWER_DIFF_OFDM 907
#define MIB_HW_11N_RESERVED9 908
#define MIB_HW_11N_RESERVED10 909
#define MIB_HW_TX_POWER_5G_HT40_1S_A 910
#define MIB_HW_TX_POWER_5G_HT40_1S_B 911
#define MIB_HW_TX_POWER_DIFF_5G_HT40_2S 912
#define MIB_HW_TX_POWER_DIFF_5G_HT20 913
#define MIB_HW_TX_POWER_DIFF_5G_OFDM 914

/*new add end*/

#define MIB_VPN_PASSTHRU_IPV6	930

#define MIB_WLAN_PHY_BAND_SELECT        951
#define MIB_WLAN_MAC_PHY_MODE           952
#define MIB_WLAN_BAND2G5G_SELECT        953

// MIB value and constant
#define MAX_SSID_LEN			33
#define WEP64_KEY_LEN			5
#define WEP128_KEY_LEN			13
#define MAX_NAME_LEN			31
#define COMMENT_LEN			21
#define MAX_CCK_CHAN_NUM		14
#define MAX_OFDM_CHAN_NUM		162

#define MAX_2G_CHANNEL_NUM_MIB		14
#define MAX_5G_CHANNEL_NUM_MIB		196

#define MAX_PSK_LEN			64
#define MAX_RS_PASS_LEN			65
#define MAX_DOMAIN_LEN			51
#define MAX_NAME_LEN_LONG		129

#define TX_RATE_1M			0x01
#define TX_RATE_2M			0x02
#define TX_RATE_5M			0x04
#define TX_RATE_11M			0x08

#define TX_RATE_6M			0x10
#define TX_RATE_9M			0x20
#define TX_RATE_12M			0x40
#define TX_RATE_18M			0x80
#define TX_RATE_24M			0x100
#define TX_RATE_36M			0x200
#define TX_RATE_48M			0x400
#define TX_RATE_54M			0x800

#define MAX_WLAN_AC_NUM			20

//#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_) Keith remove
#define MAX_MESH_ACL_NUM	MAX_WLAN_AC_NUM
//#endif Keith remove

#define MAX_CERTROOT_NUM			5
#define MAX_CERTUSER_NUM			5	
#ifdef HOME_GATEWAY
#define MAX_FILTER_NUM			20
#define MAX_URLFILTER_NUM		8
#ifdef VPN_SUPPORT
#define MAX_TUNNEL_NUM			10
#define MAX_RSA_FILE_LEN		2048
#define MAX_RSA_KEY_LEN			380
#define MAX_ENCRKEY_LEN			49
#define MAX_AUTHKEY_LEN			41
#define MAX_SPI_LEN			5
#endif
#define MAX_QOS_RULE_NUM		10
#endif
#define MAX_ROUTE_NUM			10
#define MAX_DHCP_RSVD_IP_NUM 20
#define MAXFNAME			60

//#ifdef CONFIG_RTL8196B_GW_8M
//#define MAX_WDS_NUM			4
//#else
#define MAX_WDS_NUM			8
//#endif

#ifdef WLAN_EASY_CONFIG
#define MAX_ACF_KEY_LEN			64
#define MAX_ACF_DIGEST_LEN		32
#endif

//#ifdef SNMP_SUPPORT Keith remove
#define MAX_SNMP_COMMUNITY_LEN                  64
//#endif Keith remove

#ifdef WIFI_SIMPLE_CONFIG
#define PIN_LEN					8
#endif

#define IFNAMSIZE       16
#define MAX_IFACE_VLAN_CONFIG 10 /* no wds and vxd*/
/*Brad add for schedule*/
#define MAX_SCHEDULE_NUM 1
/* # keith: add l2tp support. 20080515 */
#define MAX_PPTP_HOST_NAME_LEN 64

#ifdef __mips__
#define FLASH_DEVICE_NAME		("/dev/mtd")
#define FLASH_DEVICE_NAME1		("/dev/mtdblock1")
#ifdef CONFIG_RTL_HW_SETTING_OFFSET
#define HW_SETTING_OFFSET		CONFIG_RTL_HW_SETTING_OFFSET
#else
#define HW_SETTING_OFFSET		0x6000
#endif

#if !defined(MOVE_OUT_DEFAULT_SETTING_FROM_FLASH)
#ifdef CONFIG_RTL_DEFAULT_SETTING_OFFSET
#define DEFAULT_SETTING_OFFSET          CONFIG_RTL_DEFAULT_SETTING_OFFSET
#else
#define DEFAULT_SETTING_OFFSET		0x8000
#endif
#endif

// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT
#define CURRENT_SETTING_OFFSET      0x10000		// ds: 0x8000 - 0x10000 = 32K, cs: 0x10000 - 0x18000 = 32K 
#define CODE_IMAGE_OFFSET           0x40000		// max linux = 0x150000 - 0x40000 = 1114112 Bytes
#define WEB_PAGE_OFFSET             0x20000		// max web = 0x40000 - 0x20000 = 128 K
#define ROOT_IMAGE_OFFSET			0x150000	// max rootfs = 0x3f0000 - 0x150000 = 2752512 Bytes
#define CERT_PAGE_OFFSET			0x3f0000
#else
#ifdef CONFIG_RTL_CURRENT_SETTING_OFFSET
#define CURRENT_SETTING_OFFSET		CONFIG_RTL_CURRENT_SETTING_OFFSET
#else
#define CURRENT_SETTING_OFFSET		0xc000
#endif
#ifdef CONFIG_RTL_CODE_IMAGE_OFFSET
#define CODE_IMAGE_OFFSET               CONFIG_RTL_CODE_IMAGE_OFFSET
#else
#define CODE_IMAGE_OFFSET		0x20000
#endif
#ifdef CONFIG_RTL_WEB_PAGES_OFFSET
#define WEB_PAGE_OFFSET			CONFIG_RTL_WEB_PAGES_OFFSET
#else
#define WEB_PAGE_OFFSET			0x10000
#endif
#ifdef CONFIG_RTL_ROOT_IMAGE_OFFSET
#define ROOT_IMAGE_OFFSET		CONFIG_RTL_ROOT_IMAGE_OFFSET
#else
#define ROOT_IMAGE_OFFSET		0xE0000
#endif

#ifdef HOME_GATEWAY
	#define CERT_PAGE_OFFSET	0x3f0000
#else
	#define CERT_PAGE_OFFSET	0x1f0000
#endif

#endif

#else // not MIPS platform (x86)
#define FLASH_DEVICE_NAME		("setting.bin")

#ifdef CONFIG_RTL_HW_SETTING_OFFSET
#define HW_SETTING_OFFSET		(CONFIG_RTL_HW_SETTING_OFFSET-CONFIG_RTL_HW_SETTING_OFFSET)
#else
#define HW_SETTING_OFFSET		0
#endif

#if !defined(MOVE_OUT_DEFAULT_SETTING_FROM_FLASH)
#ifdef CONFIG_RTL_DEFAULT_SETTING_OFFSET
#define DEFAULT_SETTING_OFFSET		(CONFIG_RTL_DEFAULT_SETTING_OFFSET-CONFIG_RTL_HW_SETTING_OFFSET)
#else
#define DEFAULT_SETTING_OFFSET		0x2000
#endif
#endif

// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT
#define CURRENT_SETTING_OFFSET		0x8000
#else

#ifdef CONFIG_RTL_CURRENT_SETTING_OFFSET
#define CURRENT_SETTING_OFFSET		(CONFIG_RTL_CURRENT_SETTING_OFFSET-CONFIG_RTL_HW_SETTING_OFFSET)
#else
#define CURRENT_SETTING_OFFSET		0x6000
#endif

#endif
#define CODE_IMAGE_OFFSET		0
#define WEB_PAGE_OFFSET			0
#define CERT_PAGE_OFFSET		0
#define ROOT_IMAGE_OFFSET		0
#endif
#if !defined(MOVE_OUT_DEFAULT_SETTING_FROM_FLASH)
#ifdef CONFIG_RTL_DEFAULT_SETTING_OFFSET
#define HW_SETTING_SECTOR_LEN		(CONFIG_RTL_DEFAULT_SETTING_OFFSET-CONFIG_RTL_HW_SETTING_OFFSET)
#define DEFAULT_SETTING_SECTOR_LEN	(CONFIG_RTL_CURRENT_SETTING_OFFSET-CONFIG_RTL_DEFAULT_SETTING_OFFSET)
#define CURRENT_SETTING_SECTOR_LEN	(CONFIG_RTL_WEB_PAGES_OFFSET-CONFIG_RTL_CURRENT_SETTING_OFFSET)
#else
#define HW_SETTING_SECTOR_LEN		(0x8000-0x6000)
#define DEFAULT_SETTING_SECTOR_LEN	(0xc000-0x8000)
#define CURRENT_SETTING_SECTOR_LEN	(0x10000-0xc000)
#endif
#else
#ifdef CONFIG_RTL_DEFAULT_SETTING_OFFSET
#define HW_SETTING_SECTOR_LEN		(CONFIG_RTL_DEFAULT_SETTING_OFFSET-CONFIG_RTL_HW_SETTING_OFFSET)
#define CURRENT_SETTING_SECTOR_LEN	(CONFIG_RTL_WEB_PAGES_OFFSET-CONFIG_RTL_CURRENT_SETTING_OFFSET)
#else
#define HW_SETTING_SECTOR_LEN		(0x8000-0x6000)
#define CURRENT_SETTING_SECTOR_LEN	(0x10000-0xc000)
#endif
#endif


#ifndef WIN32
#define __PACK__			__attribute__ ((packed))
#else
#define __PACK__
#endif


/* Config/fw image file header */

typedef enum { HW_SETTING=1, DEFAULT_SETTING=2, CURRENT_SETTING=4 } CONFIG_DATA_T;

#if defined(CONFIG_RTL8196B)

	#if defined(CONFIG_RTL8198)
	
		// update tag
		#define HW_SETTING_HEADER_TAG		((char *)"H6")
		
		#if (defined(HOME_GATEWAY) && defined(VPN_SUPPORT))
		#define DEFAULT_SETTING_HEADER_TAG	((char *)"6V")
		#elif (defined(HOME_GATEWAY))
		#define DEFAULT_SETTING_HEADER_TAG	((char *)"6G")
		#else
		#define DEFAULT_SETTING_HEADER_TAG	((char *)"6A")
		#endif
		
		#if (defined(HOME_GATEWAY) && defined(VPN_SUPPORT))
		#define CURRENT_SETTING_HEADER_TAG	((char *)"6v")
		#elif (defined(HOME_GATEWAY))
		#define CURRENT_SETTING_HEADER_TAG	((char *)"6g")
		#else
		#define CURRENT_SETTING_HEADER_TAG	((char *)"6a")
		#endif
		// force tag
		#define HW_SETTING_HEADER_FORCE_TAG	((char *)"Hf")
		#define DEFAULT_SETTING_HEADER_FORCE_TAG ((char *)"Df")
		#define CURRENT_SETTING_HEADER_FORCE_TAG ((char *)"Cf")
		// upgrade
		#define HW_SETTING_HEADER_UPGRADE_TAG	((char *)"Hu")
		#define DEFAULT_SETTING_HEADER_UPGRADE_TAG ((char *)"Du")
		#define CURRENT_SETTING_HEADER_UPGRADE_TAG ((char *)"Cu")
	
	
	
	#elif defined(CONFIG_RTL8196C)
	
				// update tag
		#define HW_SETTING_HEADER_TAG		((char *)"H6")
		
		#if (defined(HOME_GATEWAY) && defined(VPN_SUPPORT))
		#define DEFAULT_SETTING_HEADER_TAG	((char *)"6V")
		#elif (defined(HOME_GATEWAY))
		#define DEFAULT_SETTING_HEADER_TAG	((char *)"6G")
		#else
		#define DEFAULT_SETTING_HEADER_TAG	((char *)"6A")
		#endif
		
		#if (defined(HOME_GATEWAY) && defined(VPN_SUPPORT))
		#define CURRENT_SETTING_HEADER_TAG	((char *)"6v")
		#elif (defined(HOME_GATEWAY))
		#define CURRENT_SETTING_HEADER_TAG	((char *)"6g")
		#else
		#define CURRENT_SETTING_HEADER_TAG	((char *)"6a")
		#endif
		// force tag
		#define HW_SETTING_HEADER_FORCE_TAG	((char *)"Hf")
		#define DEFAULT_SETTING_HEADER_FORCE_TAG ((char *)"Df")
		#define CURRENT_SETTING_HEADER_FORCE_TAG ((char *)"Cf")
		// upgrade
		#define HW_SETTING_HEADER_UPGRADE_TAG	((char *)"Hu")
		#define DEFAULT_SETTING_HEADER_UPGRADE_TAG ((char *)"Du")
		#define CURRENT_SETTING_HEADER_UPGRADE_TAG ((char *)"Cu")
	
	
	#else // if defined(CONFIG_RTL8196B)
	
		// update tag
		#define HW_SETTING_HEADER_TAG		((char *)"h6")
		
		#if (defined(HOME_GATEWAY) && defined(VPN_SUPPORT))
		#define DEFAULT_SETTING_HEADER_TAG	((char *)"6V")
		#elif (defined(HOME_GATEWAY))
		#define DEFAULT_SETTING_HEADER_TAG	((char *)"6G")
		#else
		#define DEFAULT_SETTING_HEADER_TAG	((char *)"6A")
		#endif
		
		#if (defined(HOME_GATEWAY) && defined(VPN_SUPPORT))
		#define CURRENT_SETTING_HEADER_TAG	((char *)"6v")
		#elif (defined(HOME_GATEWAY))
		#define CURRENT_SETTING_HEADER_TAG	((char *)"6g")
		#else
		#define CURRENT_SETTING_HEADER_TAG	((char *)"6a")
		#endif
	// force tag
		#define HW_SETTING_HEADER_FORCE_TAG	((char *)"Hf")
		#define DEFAULT_SETTING_HEADER_FORCE_TAG ((char *)"Df")
		#define CURRENT_SETTING_HEADER_FORCE_TAG ((char *)"Cf")
	// upgrade
		#define HW_SETTING_HEADER_UPGRADE_TAG	((char *)"Hu")
		#define DEFAULT_SETTING_HEADER_UPGRADE_TAG ((char *)"Du")
		#define CURRENT_SETTING_HEADER_UPGRADE_TAG ((char *)"Cu")
	
	
	
	#endif
	


#else // #if !defined(CONFIG_RTL8196B)

// update tag
#define HW_SETTING_HEADER_TAG		((char *)"hs")
#if (defined(HOME_GATEWAY) && defined(VPN_SUPPORT))
#define DEFAULT_SETTING_HEADER_TAG	((char *)"dv")
#elif (defined(HOME_GATEWAY))
#define DEFAULT_SETTING_HEADER_TAG	((char *)"dg")
#else
#define DEFAULT_SETTING_HEADER_TAG	((char *)"da")
#endif
#if (defined(HOME_GATEWAY) && defined(VPN_SUPPORT))
#define CURRENT_SETTING_HEADER_TAG	((char *)"cv")
#elif (defined(HOME_GATEWAY))
#define CURRENT_SETTING_HEADER_TAG	((char *)"cg")
#else
#define CURRENT_SETTING_HEADER_TAG	((char *)"ca")
#endif
// force tag
#define HW_SETTING_HEADER_FORCE_TAG	((char *)"hf")
#define DEFAULT_SETTING_HEADER_FORCE_TAG ((char *)"df")
#define CURRENT_SETTING_HEADER_FORCE_TAG ((char *)"cf")
// upgrade
#define HW_SETTING_HEADER_UPGRADE_TAG	((char *)"hu")
#define DEFAULT_SETTING_HEADER_UPGRADE_TAG ((char *)"du")
#define CURRENT_SETTING_HEADER_UPGRADE_TAG ((char *)"cu")
#endif //#if defined(CONFIG_RTL8196B)


#if 0//def CONFIG_RTL8196B_GW_8M
#undef DEFAULT_SETTING_HEADER_TAG	
#define DEFAULT_SETTING_HEADER_TAG	((char *)"61")

#undef CURRENT_SETTING_HEADER_TAG	
#define CURRENT_SETTING_HEADER_TAG	((char *)"62")
#endif


#define TAG_LEN				2

#if defined(CONFIG_RTL8196B)

	#if defined(CONFIG_RTL8198)
		#define HW_SETTING_VER			1	// hw setting version
		#define DEFAULT_SETTING_VER		1	// default setting version
		#define CURRENT_SETTING_VER		DEFAULT_SETTING_VER // current setting version
	
	#elif defined(CONFIG_RTL8196C)
		#define HW_SETTING_VER			1	// hw setting version
		#define DEFAULT_SETTING_VER		1	// default setting version
		#define CURRENT_SETTING_VER		DEFAULT_SETTING_VER // current setting version
		
	#else // 		if defined(CONFIG_RTL8196B)
		#define HW_SETTING_VER			1	// hw setting version
		#define DEFAULT_SETTING_VER		2	// default setting version
		#define CURRENT_SETTING_VER		DEFAULT_SETTING_VER // current setting version

	#endif

#else

#define HW_SETTING_VER			3	// hw setting version
#define DEFAULT_SETTING_VER		5	// default setting version
#define CURRENT_SETTING_VER		DEFAULT_SETTING_VER // current setting version

#endif

#if defined(CONFIG_RTL8196B)

	#if defined(CONFIG_RTL8198)
		#define FW_HEADER_WITH_ROOT	((char *)"cr6c")
		#define FW_HEADER			((char *)"cs6c")
	
	#elif defined(CONFIG_RTL8196C)
		#define FW_HEADER_WITH_ROOT	((char *)"cr6c")
		#define FW_HEADER			((char *)"cs6c")
		
	#else // 		if defined(CONFIG_RTL8196B)
		#define FW_HEADER_WITH_ROOT	((char *)"cr6b")
		#define FW_HEADER			((char *)"cs6b")
	#endif
	
#else
#define FW_HEADER_WITH_ROOT	((char *)"csro")
#define FW_HEADER			((char *)"csys")
#endif //#if defined(CONFIG_RTL8196B)

#if defined(CONFIG_RTL8196B)

	#if defined(CONFIG_RTL8198)
		#if (defined(HOME_GATEWAY) && defined(VPN_SUPPORT))
			#define WEB_HEADER			((char *)"w6cv")
		#elif (defined(HOME_GATEWAY))
			#define WEB_HEADER			((char *)"w6cg")
		#else
			#define WEB_HEADER			((char *)"w6ca")
		#endif
	
	#elif defined(CONFIG_RTL8196C)
		#if (defined(HOME_GATEWAY) && defined(VPN_SUPPORT))
			#define WEB_HEADER			((char *)"w6cv")
		#elif (defined(HOME_GATEWAY))
			#define WEB_HEADER			((char *)"w6cg")
		#else
			#define WEB_HEADER			((char *)"w6ca")
		#endif
		
	#else // 		if defined(CONFIG_RTL8196B)
		#if (defined(HOME_GATEWAY) && defined(VPN_SUPPORT))
			#define WEB_HEADER			((char *)"w6bv")
		#elif (defined(HOME_GATEWAY))
			#define WEB_HEADER			((char *)"w6bg")
		#else
			#define WEB_HEADER			((char *)"w6ba")
		#endif
	#endif
#else
#if (defined(HOME_GATEWAY) && defined(VPN_SUPPORT))
#define WEB_HEADER			((char *)"webv")
#elif (defined(HOME_GATEWAY))
#define WEB_HEADER			((char *)"webg")
#else
#define WEB_HEADER			((char *)"weba")
#endif
#endif //#if defined(CONFIG_RTL8196B)

#if defined(CONFIG_RTL8196B)
	#if defined(CONFIG_RTL8198)
		#define ROOT_HEADER			((char *)"r6cr")
	#elif defined(CONFIG_RTL8196C)
		#define ROOT_HEADER			((char *)"r6cr")
	#else
		#define ROOT_HEADER			((char *)"r6br")	
	#endif

#else
#define ROOT_HEADER			((char *)"root")
#endif //#if defined(CONFIG_RTL8196B)

#define CERT_HEADER			((char *)"cert")
#define BOOT_HEADER			((char *)"boot")
#define ALL_HEADER			((char *)"allp")
#define SIGNATURE_LEN			4

/* wlan driver ioctl id */
#define SIOCGIWRTLSTAINFO   		0x8B30	// get station table information
#define SIOCGIWRTLSTANUM		0x8B31	// get the number of stations in table
#define SIOCGIWRTLSCANREQ		0x8B33	// scan request
#define SIOCGIWRTLGETBSSDB		0x8B34	// get bss data base
#define SIOCGIWRTLJOINREQ		0x8B35	// join request
#define SIOCGIWRTLJOINREQSTATUS		0x8B36	// get status of join request
#define SIOCGIWRTLGETBSSINFO		0x8B37	// get currnet bss info
#define SIOCGIWRTLGETWDSINFO		0x8B38
#define SIOCGMISCDATA	0x8B48	// get misc data

//=========add for MESH=========
//#ifdef CONFIG_RTK_MESH Keith remove
// by GANTOE for site survey 2008/12/26 
#define SIOCJOINMESH 0x8B94 
#define SIOCCHECKMESHLINK 0x8B95
#define RTL8190_IOCTL_GET_MIB	0x89f2
//#endif Keith remove
//=========add for MESH=========

#define MAX_STA_NUM			64	// max support sta number

/* flag of sta info */
#define STA_INFO_FLAG_AUTH_OPEN     	0x01
#define STA_INFO_FLAG_AUTH_WEP      	0x02
#define STA_INFO_FLAG_ASOC          	0x04
#define STA_INFO_FLAG_ASLEEP        	0x08

// bit value for hw board id
#if 0
// Old code and no longer used
#define ETH_PHY_TYPE			1
#define BOOT_PORT_SELECT		2
#define USE_ETH0_WAN			4
#endif
#define WLAN_RF_2T2R			1

#ifdef WIFI_SIMPLE_CONFIG
enum { WSC_AUTH_OPEN=1, WSC_AUTH_WPAPSK=2, WSC_AUTH_SHARED=4, WSC_AUTH_WPA=8, WSC_AUTH_WPA2=0x10, WSC_AUTH_WPA2PSK=0x20, WSC_AUTH_WPA2PSKMIXED=0x22 };
enum { WSC_ENCRYPT_NONE=1, WSC_ENCRYPT_WEP=2, WSC_ENCRYPT_TKIP=4, WSC_ENCRYPT_AES=8, WSC_ENCRYPT_TKIPAES=12 };
enum { CONFIG_METHOD_ETH=0x2, CONFIG_METHOD_PIN=0x4, CONFIG_METHOD_PBC=0x80 };
enum { CONFIG_BY_INTERNAL_REGISTRAR=1, CONFIG_BY_EXTERNAL_REGISTRAR=2};
#endif
typedef enum { ENCRYPT_DISABLED=0, ENCRYPT_WEP=1, ENCRYPT_WPA=2, ENCRYPT_WPA2=4, ENCRYPT_WPA2_MIXED=6 ,ENCRYPT_WAPI=7} ENCRYPT_T;
typedef enum { WDS_ENCRYPT_DISABLED=0, WDS_ENCRYPT_WEP64=1, WDS_ENCRYPT_WEP128=2, WDS_ENCRYPT_TKIP=3, WDS_ENCRYPT_AES=4} WDS_ENCRYPT_T;
typedef enum { SUPP_NONWPA_NONE=0,SUPP_NONWPA_WEP=1,SUPP_NONWPA_1X=2} SUPP_NONWAP_T;
typedef enum { WPA_AUTH_AUTO=1, WPA_AUTH_PSK=2 } WPA_AUTH_T;
typedef enum { WAPI_AUTH_AUTO=1, WAPI_AUTH_PSK=2 } WAPI_AUTH_T;
typedef enum { WPA_CIPHER_TKIP=1, WPA_CIPHER_AES=2, WPA_CIPHER_MIXED=3 } WPA_CIPHER_T;
typedef enum { WEP_DISABLED=0, WEP64=1, WEP128=2 } WEP_T;
typedef enum { KEY_ASCII=0, KEY_HEX } KEY_TYPE_T;
typedef enum { LONG_PREAMBLE=0, SHORT_PREAMBLE=1 } PREAMBLE_T;
typedef enum { DHCP_DISABLED=0, DHCP_CLIENT=1, DHCP_SERVER=2, PPPOE=3, PPTP=4, L2TP=6, DHCP_AUTO=15 } DHCP_T; /* # keith: add l2tp support. 20080515 */
typedef enum { DHCP_LAN_NONE=0, DHCP_LAN_CLIENT=1, DHCP_LAN_SERVER=2, DHCP_LAN_RELAY=3 } DHCP_TYPE_T; //keith add. LAN SIDE DHCP TYPE
typedef enum { GATEWAY_MODE=0, BRIDGE_MODE=1, WISP_MODE=2 } OPMODE_T;
typedef enum { DISABLE_MODE=0, RIP1_MODE=1, RIP2_MODE=2 } RIP_OPMODE_T; 
typedef enum { FCC=1, IC, ETSI, SPAIN, FRANCE, MKK } REG_DOMAIN_T;
typedef enum { AUTH_OPEN=0, AUTH_SHARED, AUTH_BOTH } AUTH_TYPE_T;
typedef enum { DNS_AUTO=0, DNS_MANUAL } DNS_TYPE_T;
typedef enum { CONTINUOUS=0, CONNECT_ON_DEMAND, MANUAL } PPP_CONNECT_TYPE_T;
typedef enum { RF_INTERSIL=1, RF_RFMD=2, RF_PHILIP=3, RF_MAXIM=4, RF_GCT=5,
 		RF_MAXIM_AG=6, RF_ZEBRA=7, RF_8255=8 } RF_TYPE_T;
typedef enum { LED_TX_RX=0, LED_LINK_TXRX=1, LED_LINKTXRX=2 } LED_TYPE_T;

//=========add for MESH=========
#ifdef CONFIG_RTK_MESH
typedef enum { RANN=0, PREQ=1 } TREEMECH_T;
#ifdef CONFIG_NEW_MESH_UI
typedef enum { AP_MODE=0, CLIENT_MODE=1, WDS_MODE=2, AP_WDS_MODE=3, AP_MESH_MODE=4, MESH_MODE=5} WLAN_MODE_T;
#else
typedef enum { AP_MODE=0, CLIENT_MODE=1, WDS_MODE=2, AP_WDS_MODE=3, AP_MPP_MODE=4, MPP_MODE=5, MAP_MODE=6, MP_MODE=7	} WLAN_MODE_T;
#endif
#else
typedef enum { AP_MODE=0, CLIENT_MODE=1, WDS_MODE=2, AP_WDS_MODE=3	} WLAN_MODE_T;
#endif // CONFIG_RTK_MESH
//=========add for MESH=========

typedef enum { INFRASTRUCTURE=0, ADHOC=1 } NETWORK_TYPE_T;
typedef enum { BAND_11B=1, BAND_11G=2, BAND_11BG=3, BAND_11A=4, BAND_11N=8 } BAND_TYPE_T;
typedef enum { DISABLED=0, A_MPDU=1, A_MSDU=2, A_MIXED=3} AGGREGATION_MODE_T;	// GANTOE & epopen: DISABLED=0 original is DISABLE=0, Because conflict with ../../auth/include/1x_common.h in AP/net-snmp-5.x.x

typedef enum { PHYBAND_OFF=0, PHYBAND_2G=1, PHYBAND_5G=2 } PHYBAND_TYPE_T; //mark_add
typedef enum { SMACSPHY=0, DMACSPHY=1, DMACDPHY=2 } MACPHYMODE_TYPE_T;

#ifdef HOME_GATEWAY
#ifdef VPN_SUPPORT
typedef enum { IKE_MODE=0, MANUAL_MODE=1} KEY_MODE_T;
typedef enum { SINGLE_ADDR=0, SUBNET_ADDR=1, ANY_ADDR=2, NATT_ADDR=3} ADDRESS_MODE_T;
typedef enum { INITIATOR=0, RESPONDER=1} CONN_TYPE_T;
typedef enum { MD5_ALGO=0, AUTH_ALGO=1} AUTH_MODE_T;
typedef enum { ESP_PROTO=0, AH_PROTO=1} IPSEC_PROTO_T;
typedef enum { TRI_DES_ALGO=0, AES_ALGO=1, NONE_ALGO=2} ENCR_MODE_T;
// DH1=768 bits, DH2=1024 bits, DH5= 1536
typedef enum { DH1_GRP=0, DH2_GRP=1, DH5_GRP=2} KEY_GROUP_T;
#endif // VPN_SUPPORT
typedef enum { PROTO_BOTH=3, PROTO_TCP=1, PROTO_UDP=2 } PROTO_TYPE_T;
#endif // HOME_GATEWAY

#ifdef WLAN_EASY_CONFIG
enum { MODE_BUTTON=1, MODE_QUESTION=2 };
enum {
	ACF_ALGORITHM_WEP64	= 0x01,
	ACF_ALGORITHM_WEP128	= 0x02,
	ACF_ALGORITHM_WPA_TKIP	= 0x04,
	ACF_ALGORITHM_WPA_AES	= 0x08,
	ACF_ALGORITHM_WPA2_TKIP	= 0x10,
	ACF_ALGORITHM_WPA2_AES	= 0x20,
};
enum {	ROLE_SERVER=1, ROLE_CLIENT=2, ROLE_ADHOC=4};
#endif // WLAN_EASY_CONFIG

enum {TURBO_AUTO=0, TURBO_ON=1, TURBO_OFF=2};

#define DWORD_SWAP(v) ( (((v&0xff)<<24)&0xff000000) | ((((v>>8)&0xff)<<16)&0xff0000) | \
				((((v>>16)&0xff)<<8)&0xff00) | (((v>>24)&0xff)&0xff) )
#define WORD_SWAP(v) ((unsigned short)(((v>>8)&0xff) | ((v<<8)&0xff00)))

/* scramble saved configuration data */
#define ENCODE_DATA(data,len) { \
	int i; \
	for (i=0; i<len; i++) \
		data[i] = ~ ( data[i] + 0x38); \
}

#define DECODE_DATA(data,len) { \
	int i; \
	for (i=0; i<len; i++) \
		data[i] = ~data[i] - 0x38;	\
}

/* Do checksum and verification for configuration data */
#ifndef WIN32
static inline unsigned char CHECKSUM(unsigned char *data, int len)
#else
__inline unsigned char CHECKSUM(unsigned char *data, int len)
#endif
{
	int i;
	unsigned char sum=0;

	for (i=0; i<len; i++)
		sum += data[i];

	sum = ~sum + 1;
	return sum;
}
#ifndef WIN32
static inline int CHECKSUM_OK(unsigned char *data, int len)
#else
__inline int CHECKSUM_OK(unsigned char *data, int len)
#endif
{
	int i;
	unsigned char sum=0;

	for (i=0; i<len; i++)
		sum += data[i];

	if (sum == 0)
		return 1;
	else
		return 0;
}

/* WLAN sta info structure */
typedef struct wlan_sta_info {
	unsigned short	aid;
	unsigned char	addr[6];
	unsigned long	tx_packets;
	unsigned long	rx_packets;
	unsigned long	expired_time;	// 10 msec unit
	unsigned short	flag;
	unsigned char	txOperaRates;
	unsigned char	rssi;
	unsigned long	link_time;		// 1 sec unit
	unsigned long	tx_fail;
	unsigned long tx_bytes;
	unsigned long rx_bytes;
	unsigned char network;
	unsigned char ht_info;	// bit0: 0=20M mode, 1=40M mode; bit1: 0=longGI, 1=shortGI
	unsigned char 	resv[6];
} WLAN_STA_INFO_T, *WLAN_STA_INFO_Tp;

typedef struct wlan_rate{
unsigned int id;
unsigned char rate[20];
}WLAN_RATE_T, *WLAN_RATE_Tp;
typedef enum { 
	MCS0=0x80, 
	MCS1=0x81, 
	MCS2=0x82,
	MCS3=0x83,
	MCS4=0x84,
	MCS5=0x85,
	MCS6=0x86,
	MCS7=0x87,
	MCS8=0x88,
	MCS9=0x89,
	MCS10=0x8a,
	MCS11=0x8b,
	MCS12=0x8c,
	MCS13=0x8d,
	MCS14=0x8e,
	MCS15=0x8f
	} RATE_11N_T;

#ifdef WIN32
#pragma pack(1)
#endif


//zj: eco featrue
#define ECO_LEDDIM_MASK		0x08
#define ECO_TIMER_MASK			0x04
#define ECO_EVERYDAY_MASK		0x02
#define ECO_24HOURS_MASK			0x01

#define ECO_DAY_MASK			0x0000007F
#define ECO_SUNDAY_MASK		0x00000001
#define ECO_MONDAY_MASK		0x00000002
#define ECO_TUESDAY_MASK		0x00000004
#define ECO_WEDNESDAY_MASK	0x00000008
#define ECO_THURSDAY_MASK		0x00000010
#define ECO_FRIDAY_MASK		0x00000020
#define ECO_SATURDAY_MASK		0x00000040

#define SCHEDULE_NAME_LEN		20
typedef struct schedule_entry {
	unsigned char text[SCHEDULE_NAME_LEN] __PACK__; /* name */
	unsigned short eco __PACK__; /* enabled:normal:everyday:24hours */
	unsigned short fTime __PACK__; /* in minute. 0~1440 */
	unsigned short tTime __PACK__;	/* in minute. 0~1440 */
	unsigned short day __PACK__; /* bitmap 0-6 : Sunday to Saturday */
} SCHEDULE_T, *SCHEDULE_Tp;



typedef struct macfilter_entry {
	unsigned char macAddr[6] __PACK__;
	unsigned char comment[COMMENT_LEN] __PACK__;
} MACFILTER_T, *MACFILTER_Tp;

#ifdef HOME_GATEWAY
typedef struct urlfilter_entry {	
	unsigned char urlAddr[31] __PACK__;	
	//unsigned char comment[COMMENT_LEN] __PACK__;
} URLFILTER_T, *URLFILTER_Tp;
typedef struct portfw_entry {
	unsigned char ipAddr[4] __PACK__;
	unsigned short fromPort __PACK__;
	unsigned short toPort __PACK__;
	unsigned char protoType __PACK__;
	unsigned char comment[COMMENT_LEN] __PACK__;
} PORTFW_T, *PORTFW_Tp;

typedef struct ipfilter_entry {
	unsigned char ipAddr[4] __PACK__;
	unsigned char protoType __PACK__;
	unsigned char comment[COMMENT_LEN] __PACK__;
} IPFILTER_T, *IPFILTER_Tp;

typedef struct portfilter_entry {
	unsigned short fromPort __PACK__;
	unsigned short toPort __PACK__;
	unsigned char protoType __PACK__;
	unsigned char comment[COMMENT_LEN] __PACK__;
} PORTFILTER_T, *PORTFILTER_Tp;

typedef struct triggerport_entry {
	unsigned short tri_fromPort __PACK__;	// trigger-from port
	unsigned short tri_toPort __PACK__;	// trigger-to port
	unsigned char tri_protoType __PACK__;	// trigger proto type
	unsigned short inc_fromPort __PACK__;	// incomming-from port
	unsigned short inc_toPort __PACK__;	// incoming-to port
	unsigned char inc_protoType __PACK__;	// incoming proto type
	unsigned char comment[COMMENT_LEN] __PACK__;	// comment
} TRIGGERPORT_T, *TRIGGERPORT_Tp;

#ifdef GW_QOS_ENGINE
#define MAX_QOS_NAME_LEN    15
typedef struct qos_entry {
	unsigned char entry_name[MAX_QOS_NAME_LEN+1] __PACK__;
	unsigned char enabled __PACK__;
	unsigned char priority __PACK__;
	unsigned short protocol __PACK__;
	unsigned char local_ip_start[4] __PACK__;
	unsigned char local_ip_end[4] __PACK__;
	unsigned short local_port_start __PACK__;
	unsigned short local_port_end __PACK__;
	unsigned char remote_ip_start[4] __PACK__;
	unsigned char remote_ip_end[4] __PACK__;
	unsigned short remote_port_start __PACK__;
	unsigned short remote_port_end __PACK__;
} QOS_T, *QOS_Tp;
#endif

#ifdef QOS_BY_BANDWIDTH
#define QOS_RESTRICT_MIN	0x01
#define QOS_RESTRICT_MAX	0x02
#define QOS_RESTRICT_IP		0x04
#define QOS_RESTRICT_MAC	0x08
#define MAX_QOS_NAME_LEN    15
#define MAC_ADDR_LEN    6

typedef struct qos_entry {
	unsigned char entry_name[MAX_QOS_NAME_LEN+1] __PACK__;
	unsigned char enabled __PACK__;
	unsigned char mac[MAC_ADDR_LEN] __PACK__;
	unsigned char mode __PACK__;
	unsigned char local_ip_start[4] __PACK__;
	unsigned char local_ip_end[4] __PACK__;
	unsigned long bandwidth __PACK__; //Up link
	unsigned long bandwidth_downlink __PACK__; //Down link
} IPQOS_T, *IPQOS_Tp;
#endif

#ifdef VPN_SUPPORT
typedef struct ipsectunnel_entry {
	unsigned char tunnelId	__PACK__;   // Tunnel Id , compare pattern
	unsigned char authType __PACK__;  // psk or rsa
	//local info
	unsigned char lcType __PACK__;  // local site address type
	unsigned char lc_ipAddr[4] __PACK__; // local ip address
	unsigned char lc_maskLen __PACK__; // local ip mask length
	//remote Info
	unsigned char rtType __PACK__; // remote site address type 
	unsigned char rt_ipAddr[4] __PACK__; // remote ip address
	unsigned char rt_maskLen __PACK__;  // remote mask length
	unsigned char rt_gwAddr[4] __PACK__; // remote gw address
	// Key mode common
	unsigned char keyMode __PACK__; // IKE or manual mode
	//unsigned char espAh __PACK__;   // select esp or Ah 
	unsigned char espEncr __PACK__; // esp encryption algorithm select
	unsigned char espAuth __PACK__; // esp authentication algorithm select
	//unsigned char ahAuth __PACK__;  // AH authentication algorithm select
	//IKE mode
	unsigned char conType __PACK__;
	unsigned char psKey[MAX_NAME_LEN] __PACK__; // preshared key
	unsigned char rsaKey[MAX_RSA_KEY_LEN] __PACK__; // rsa key
	//Manual Mode
	unsigned char spi[MAX_SPI_LEN] __PACK__; // ipsec spi base (hex string)
	unsigned char  encrKey[MAX_ENCRKEY_LEN]  __PACK__; // Encryption Key 
	unsigned char  authKey[MAX_AUTHKEY_LEN]  __PACK__; // Authentication Key 
	// tunnel info
	unsigned char enable __PACK__;	 // tunnel enable
	unsigned char connName[MAX_NAME_LEN] __PACK__;  // Connection Name
	unsigned char lcIdType __PACK__;  // local Id
	unsigned char rtIdType __PACK__;  // remote Id
	unsigned char lcId[MAX_NAME_LEN] __PACK__;  // local Id
	unsigned char rtId[MAX_NAME_LEN] __PACK__;  // Remote Id
	// ike Advanced setup
	unsigned long ikeLifeTime __PACK__;
	unsigned char ikeEncr __PACK__;
	unsigned char ikeAuth __PACK__;
	unsigned char ikeKeyGroup __PACK__;
	unsigned long ipsecLifeTime  __PACK__;
	unsigned char ipsecPfs __PACK__;

} IPSECTUNNEL_T, *IPSECTUNNEL_Tp;
#endif // VPN_SUPPORT
#endif // HOME_GATEWAY
#ifdef TLS_CLIENT
typedef struct certroot_entry {
	unsigned char comment[COMMENT_LEN] __PACK__;
} CERTROOT_T, *CERTROOT_Tp;
typedef struct certUser_entry {
	unsigned char comment[COMMENT_LEN] __PACK__;
	unsigned char pass[MAX_RS_PASS_LEN] __PACK__;
} CERTUSER_T, *CERTUSER_Tp;
#endif
#ifdef HOME_GATEWAY
#ifdef ROUTE_SUPPORT
typedef struct staticRoute_entry {
	unsigned char dstAddr[4] __PACK__; // destination ip address
	unsigned char netmask[4] __PACK__; // destination ip address
	unsigned char gateway[4] __PACK__; // destination ip address
	unsigned char _interface_ __PACK__; //interface
	unsigned char metric __PACK__; //metric
} STATICROUTE_T, *STATICROUTE_Tp;
#endif
#endif
typedef struct dhcpRsvdIP_entry {	
	unsigned char ipAddr[4] __PACK__;	
	unsigned char macAddr[6] __PACK__;
	unsigned char hostName[32] __PACK__;	
} DHCPRSVDIP_T, *DHCPRSVDIP_Tp;

typedef struct wlanwds_entry {
	unsigned char macAddr[6] __PACK__;
	unsigned int fixedTxRate __PACK__;
	unsigned char comment[COMMENT_LEN] __PACK__;
} WDS_T, *WDS_Tp;


#ifdef HOME_GATEWAY //defined(VLAN_CONFIG_SUPPORTED) Keith modify
typedef struct vlan_lanconfig_entry {	
	unsigned char enabled __PACK__; //0-disable, 1-enable
	unsigned char netIface[IFNAMSIZE] __PACK__;	//net interface name
	unsigned char tagged __PACK__; //0-disable tagged, 1-enable tagged
	//unsigned char untagged __PACK__; //0-disable un-tagged, 1-enable un-tagged
	unsigned char priority __PACK__; //0~7
	unsigned char cfi __PACK__;//Canonical Format Indicator
	//unsigned char groupId __PACK__;
	unsigned short vlanId __PACK__;//0~4090
} VLAN_CONFIG_T, *VLAN_CONFIG_Tp;
#endif


typedef struct hw_wlan_setting {
	unsigned char macAddr[6] __PACK__;
	unsigned char macAddr1[6] __PACK__;
	unsigned char macAddr2[6] __PACK__;
	unsigned char macAddr3[6] __PACK__;
	unsigned char macAddr4[6] __PACK__;
#if defined(CONFIG_RTL8196B)
#if defined(CONFIG_RTL8198) || defined(CONFIG_RTL8196C)
	unsigned char macAddr5[6] __PACK__;
	unsigned char macAddr6[6] __PACK__;
	unsigned char macAddr7[6] __PACK__;
	unsigned char pwrlevelCCK_A[MAX_2G_CHANNEL_NUM_MIB] __PACK__; // CCK Tx power for each channel
	unsigned char pwrlevelCCK_B[MAX_2G_CHANNEL_NUM_MIB] __PACK__; // CCK Tx power for each channel
	unsigned char pwrlevelHT40_1S_A[MAX_2G_CHANNEL_NUM_MIB] __PACK__; 
	unsigned char pwrlevelHT40_1S_B[MAX_2G_CHANNEL_NUM_MIB] __PACK__; 
	unsigned char pwrdiffHT40_2S[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiffHT20[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiffOFDM[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char regDomain __PACK__; // regulation domain
	unsigned char rfType __PACK__; // RF module type
	unsigned char ledType __PACK__; // LED type, see LED_TYPE_T for definition
	unsigned char xCap __PACK__; 
	unsigned char TSSI1 __PACK__; 
	unsigned char TSSI2 __PACK__; 
	unsigned char Ther __PACK__; 
	unsigned char Reserved1 __PACK__; 
	unsigned char Reserved2 __PACK__; 
	unsigned char Reserved3 __PACK__; 
	unsigned char Reserved4 __PACK__; 
	unsigned char Reserved5 __PACK__; 
	unsigned char Reserved6 __PACK__; 
	unsigned char Reserved7 __PACK__; 
	unsigned char Reserved8 __PACK__; 
	unsigned char Reserved9 __PACK__; 
	unsigned char Reserved10 __PACK__; 
	unsigned char pwrlevel5GHT40_1S_A[MAX_5G_CHANNEL_NUM_MIB];
	unsigned char pwrlevel5GHT40_1S_B[MAX_5G_CHANNEL_NUM_MIB];
	unsigned char pwrdiff5GHT40_2S[MAX_5G_CHANNEL_NUM_MIB];
	unsigned char pwrdiff5GHT20[MAX_5G_CHANNEL_NUM_MIB];
	unsigned char pwrdiff5GOFDM[MAX_5G_CHANNEL_NUM_MIB];
#else
	unsigned char macAddr5[6] __PACK__;
	unsigned char macAddr6[6] __PACK__;
	unsigned char macAddr7[6] __PACK__;
	unsigned char txPowerCCK[MAX_CCK_CHAN_NUM] __PACK__; // CCK Tx power for each channel
	unsigned char txPowerOFDM_HT_OFDM_1S[MAX_OFDM_CHAN_NUM] __PACK__; // OFDM Tx power for each channel
	unsigned char txPowerOFDM_HT_OFDM_2S[MAX_OFDM_CHAN_NUM] __PACK__; // OFDM Tx power for each channel
	unsigned char regDomain __PACK__; // regulation domain
	unsigned char rfType __PACK__; // RF module type
	unsigned char ledType __PACK__; // LED type, see LED_TYPE_T for definition
	unsigned char xCap __PACK__; 
	unsigned char LOFDMPwDiffA __PACK__; 
	unsigned char LOFDMPwDiffB __PACK__; 
	unsigned char TSSI1 __PACK__; 
	unsigned char TSSI2 __PACK__; 
	unsigned char Ther __PACK__; 
	unsigned char Reserved1 __PACK__; 
	unsigned char Reserved2 __PACK__; 
	unsigned char Reserved3 __PACK__; 
	unsigned char Reserved4 __PACK__; 
	unsigned char Reserved5 __PACK__; 
	unsigned char Reserved6 __PACK__; 
	unsigned char Reserved7 __PACK__; 
	unsigned char Reserved8 __PACK__; 

#endif
#else
	//!CONFIG_RTL8196B ==> rtl8651c+rtl8190
	unsigned char txPowerCCK[MAX_CCK_CHAN_NUM] __PACK__; // CCK Tx power for each channel
	unsigned char txPowerOFDM[MAX_OFDM_CHAN_NUM] __PACK__; // OFDM Tx power for each channel
	unsigned char regDomain __PACK__; // regulation domain
	unsigned char rfType __PACK__; // RF module type
	unsigned char antDiversity __PACK__; // rx antenna diversity on/off
	unsigned char txAnt __PACK__; // select tx antenna
	unsigned char ccaMode __PACK__;	// 0, 1, 2
	unsigned char ledType __PACK__; // LED type, see LED_TYPE_T for definition
	unsigned char initGain __PACK__; // baseband initial gain
	unsigned char xCap __PACK__; // for 11n
	unsigned char LOFDMPwDiff __PACK__; // for 11n
//	unsigned char AntPwDiff_B __PACK__; // for 11n
	unsigned char AntPwDiff_C __PACK__; // for 11n
//	unsigned char AntPwDiff_D __PACK__; // for 11n
	unsigned char TherRFIC __PACK__; // for 11n
#endif

#ifdef WIFI_SIMPLE_CONFIG
	unsigned char wscPin[PIN_LEN+1] __PACK__;
#endif

} HW_WLAN_SETTING_T, *HW_WLAN_SETTING_Tp;

typedef struct hw_setting {
	unsigned char boardVer __PACK__;	// h/w board version
	unsigned char nic0Addr[6] __PACK__;
	unsigned char nic1Addr[6] __PACK__;
	HW_WLAN_SETTING_T wlan[NUM_WLAN_INTERFACE];	
} HW_SETTING_T, *HW_SETTING_Tp;

typedef struct config_wlan_setting {
	unsigned char ssid[MAX_SSID_LEN] __PACK__ ; // SSID
	unsigned char channel __PACK__ ;// current channel
#ifdef RTK_FLASH_MIB_MAP	
	unsigned char wlanMacAddr[6] __PACK__ ; // WLAN MAC address
#else
	unsigned char wlanMacAddr[13] __PACK__ ; // WLAN MAC address
#endif
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
	unsigned short fragThreshold __PACK__ ;
	unsigned short rtsThreshold __PACK__ ;
	unsigned short supportedRates __PACK__ ;
	unsigned short basicRates __PACK__ ;
	unsigned short beaconInterval __PACK__ ;
	unsigned char preambleType __PACK__; // preamble type, 0 - long preamble, 1 - short preamble
	unsigned char authType __PACK__; // authentication type, 0 - open-system, 1 - shared-key, 2 - both
#if !defined(CONFIG_RTL8196C_CLIENT_ONLY)	
	unsigned char acEnabled __PACK__; // enable/disable WLAN access control
	unsigned char acNum __PACK__; // WLAN access control entry number

	MACFILTER_T acAddrArray[MAX_WLAN_AC_NUM] __PACK__; // WLAN access control array
#endif
	unsigned char hiddenSSID __PACK__ ;
	unsigned char wlanDisabled __PACK__; // enabled/disabled wlan interface. For vap interface, 0 - disable, 1 - enable
	unsigned long inactivityTime __PACK__; // wlan client inactivity time
	unsigned char rateAdaptiveEnabled __PACK__; // enable/disable rate adaptive
	unsigned char dtimPeriod __PACK__; // DTIM period
	unsigned char wlanMode __PACK__; // wireless mode - AP, Ethernet bridge 
	unsigned char networkType __PACK__; // adhoc or Infrastructure
	unsigned char iappDisabled __PACK__; // disable IAPP
	unsigned char protectionDisabled __PACK__; // disable g mode protection
	unsigned char defaultSsid[MAX_SSID_LEN]__PACK__ ; // default SSID
	unsigned char blockRelay __PACK__; // block/un-block the relay between wireless client
	unsigned char maccloneEnabled __PACK__; // enable NAT2.5 MAC Clone
	unsigned char wlanBand __PACK__; // wlan band, bit0-11B, bit1-11G, bit2-11A
	unsigned int fixedTxRate __PACK__; // fixed wlan tx rate, used when rate adaptive is disabled
	unsigned char turboMode __PACK__; // turbo mode, 0 - auto, 1 - always, 2 - off
	unsigned char RFPowerScale __PACK__; // RF output power scale, 0:100%, 1:50%, 2:25%, 3:10%, 45%

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
	unsigned char rsPassword[MAX_RS_PASS_LEN] __PACK__; // radius server password
	unsigned char enable1X __PACK__; // enable/disable 802.1x
	unsigned char wpaPSKFormat __PACK__; // PSK format 0 - passphrase, 1 - hex
	unsigned char accountRsEnabled __PACK__; // enable/disable accounting server
	unsigned char accountRsIpAddr[4] __PACK__; // accounting radius server IP address
	unsigned short accountRsPort __PACK__; // accounting radius server port number
	unsigned char accountRsPassword[MAX_RS_PASS_LEN] __PACK__; // accounting radius server password
	unsigned char accountRsUpdateEnabled __PACK__; // enable/disable accounting server update
	unsigned short accountRsUpdateDelay __PACK__; // account server update delay time in sec
	unsigned char macAuthEnabled __PACK__; // mac authentication enabled/disabled
	unsigned char rsMaxRetry __PACK__; // radius server max try
	unsigned short rsIntervalTime __PACK__; // radius server timeout
	unsigned char accountRsMaxRetry __PACK__; // accounting radius server max try
	unsigned short accountRsIntervalTime __PACK__; // accounting radius server timeout
	unsigned char wpa2PreAuth __PACK__; // wpa2 Preauthtication support
	unsigned char wpa2Cipher __PACK__; // wpa2 Unicast cipher
	
#if !defined(CONFIG_RTL8196C_CLIENT_ONLY)
	// WDS stuffs
	unsigned char wdsEnabled __PACK__; // wds enable/disable
	unsigned char wdsNum __PACK__; // number of wds entry existed
	WDS_T wdsArray[MAX_WDS_NUM] __PACK__; // wds array
	unsigned char wdsEncrypt __PACK__; // wds encrypt flag
	unsigned char wdsWepKeyFormat __PACK__; // 0 - ASCII, 1 - hex
	unsigned char wdsWepKey[WEP128_KEY_LEN*2+1] __PACK__;
	unsigned char wdsPskFormat __PACK__;	// 0 - passphrase, 1 - hex
	unsigned char wdsPsk[MAX_PSK_LEN+1] __PACK__;
#endif
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
	unsigned char channelbonding __PACK__; 
	unsigned char controlsideband __PACK__; 
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
    unsigned char phyBandSelect __PACK__;//mark_92d
    unsigned char macPhyMode __PACK__;//mark_92d
} CONFIG_WLAN_SETTING_T, *CONFIG_WLAN_SETTING_Tp;

typedef struct config_setting {
	// TCP/IP stuffs
	unsigned char ipAddr[4] __PACK__;
	unsigned char subnetMask[4] __PACK__;
	unsigned char defaultGateway[4] __PACK__;
	unsigned char dhcp __PACK__; // DHCP flag, 0 - disabled, 1 - client, 2 - server
	unsigned char dhcpClientStart[4] __PACK__; // DHCP client start address
	unsigned char dhcpClientEnd[4] __PACK__; // DHCP client end address
	unsigned char elanMacAddr[6] __PACK__ ; // Ethernet Lan MAC address	
	//Brad add for static dhcp
	unsigned char dhcpRsvdIpEnabled __PACK__; // DHCP Reserved IP enable flag
	unsigned char dhcpRsvdIpNum __PACK__;
	DHCPRSVDIP_T dhcpRsvdIpArray[MAX_DHCP_RSVD_IP_NUM] __PACK__;	
	unsigned char dns1[4], dns2[4], dns3[4] __PACK__;
	unsigned char stpEnabled; // Spanning tree protocol flag, 0 - disabled, 1 - enabled
	unsigned char deviceName[MAX_NAME_LEN] __PACK__; // device logical name	
	unsigned char scrlogEnabled __PACK__; // enable security log
	unsigned char autoDiscoveryEnabled __PACK__; // enable/disable auto-discovery
	unsigned char domainName[MAX_NAME_LEN] __PACK__; // dhcp server domain name

	// Supervisor of web server account
	unsigned char superName[MAX_NAME_LEN] __PACK__ ; // supervisor name
	unsigned char superPassword[MAX_NAME_LEN] __PACK__; // supervisor assword

	// web server account
	unsigned char userName[MAX_NAME_LEN] __PACK__; // supervisor name
	unsigned char userPassword[MAX_NAME_LEN] __PACK__; // supervisor assword
	CONFIG_WLAN_SETTING_T wlan[NUM_WLAN_INTERFACE][NUM_VWLAN_INTERFACE+1] __PACK__; // arrary of wlan setting
	unsigned char scheduleRuleEnabled __PACK__;
	unsigned char scheduleRuleNum __PACK__; // number of schedule rule entry existed
	SCHEDULE_T scheduleRuleArray[MAX_SCHEDULE_NUM] __PACK__; //
#ifdef HOME_GATEWAY
	unsigned char wanMacAddr[6] __PACK__ ; // MAC address of WAN port in used
	unsigned char wanDhcp __PACK__; // DHCP flag for WAN port, 0 - disabled, 1 - DHCP client
	unsigned char wanIpAddr[4] __PACK__;
	unsigned char wanSubnetMask[4] __PACK__;
	unsigned char wanDefaultGateway[4] __PACK__;
	unsigned char pppUserName[MAX_NAME_LEN_LONG] __PACK__;
	unsigned char pppPassword[MAX_NAME_LEN_LONG] __PACK__;
	DNS_TYPE_T dnsMode __PACK__;
	unsigned short pppIdleTime __PACK__;
	unsigned char pppConnectType __PACK__;

	unsigned char portFwEnabled __PACK__;
	unsigned char portFwNum __PACK__; // number of port-forwarding entry existed
	PORTFW_T portFwArray[MAX_FILTER_NUM] __PACK__; // port-forwarding array

	unsigned char ipFilterEnabled __PACK__;
	unsigned char ipFilterNum __PACK__; // number of ip-filter entry existed
	IPFILTER_T ipFilterArray[MAX_FILTER_NUM] __PACK__; // ip-filter array

	unsigned char portFilterEnabled __PACK__;
	unsigned char portFilterNum __PACK__; // number of port-filter entry existed
	PORTFILTER_T portFilterArray[MAX_FILTER_NUM] __PACK__; // ip-filter array

	unsigned char macFilterEnabled __PACK__;
	unsigned char macFilterNum __PACK__; // number of mac filter entry existed
	MACFILTER_T macFilterArray[MAX_FILTER_NUM] __PACK__; // mac-filter array

	unsigned char triggerPortEnabled __PACK__;
	unsigned char triggerPortNum __PACK__; // number of trigger port entry existed
	TRIGGERPORT_T triggerPortArray[MAX_FILTER_NUM] __PACK__; // trigger port array

	unsigned char dmzEnabled __PACK__;
	unsigned char dmzHost[4] __PACK__; // DMZ host
	unsigned char upnpEnabled __PACK__; // upnp enable/disable
	unsigned short pppMtuSize __PACK__; // pppoe MTU size
	unsigned char pptpIpAddr[4] __PACK__; // pptp local ip address
	unsigned char pptpSubnetMask[4] __PACK__; // pptp local ip address
	unsigned char pptpServerIpAddr[4] __PACK__; // pptp server ip address
	unsigned char pptpUserName[MAX_NAME_LEN_LONG] __PACK__;
	unsigned char pptpPassword[MAX_NAME_LEN_LONG] __PACK__;
	unsigned short pptpMtuSize __PACK__; // pptp MTU size

	/* # keith: add l2tp support. 20080515 */
	unsigned char l2tpIpAddr[4] __PACK__; // l2tp local ip address
	unsigned char l2tpSubnetMask[4] __PACK__; //l2tp local ip address
	unsigned char l2tpServerIpAddr[MAX_PPTP_HOST_NAME_LEN] __PACK__; // l2tp server ip address
	unsigned char l2tpGateway[4] __PACK__;
	unsigned char l2tpUserName[MAX_NAME_LEN_LONG] __PACK__;
	unsigned char l2tpPassword[MAX_NAME_LEN_LONG] __PACK__;
	unsigned short l2tpMtuSize __PACK__; // l2tp MTU size
	unsigned short l2tpIdleTime __PACK__; //l2tp idle time 
	unsigned char l2tpConnectType __PACK__; //l2tp connection type 
	unsigned char L2tpwanIPMode __PACK__; //l2tp ip mode

	unsigned char ntpEnabled __PACK__; // ntp client enabled
	unsigned char daylightsaveEnabled __PACK__; //day light saving enabled
	unsigned char ntpServerId __PACK__; // ntp Server Index
	unsigned char ntpTimeZone[8] __PACK__; // ntp  Time Zone 
	unsigned char ntpServerIp1[4] __PACK__; // ntp  server ip address
	unsigned char ntpServerIp2[4] __PACK__; // ntp server ip address
#ifdef VPN_SUPPORT
	unsigned char ipsecTunnelEnabled __PACK__;
	unsigned char ipsecTunnelNum __PACK__; // number of ipsec tunnel entry existed
	IPSECTUNNEL_T ipsecTunnelArray[MAX_TUNNEL_NUM] __PACK__; // ipsec tunnel array
	unsigned char ipsecNattEnabled __PACK__;
	unsigned char ipsecRsaKeyFile[MAX_RSA_FILE_LEN] __PACK__;
#endif
	unsigned char	ddnsEnabled	__PACK__; // ddns Enabled
	unsigned char	ddnsType	__PACK__; // ddnsService Provider
	unsigned char	ddnsDomainName[MAX_DOMAIN_LEN]	__PACK__; // Domain Name
	unsigned char   ddnsUser[MAX_DOMAIN_LEN]    __PACK__; // User
	unsigned char   ddnsPassword[MAX_NAME_LEN]    __PACK__; // Password
	unsigned short fixedIpMtuSize __PACK__; // fixed-IP MTU size	
	unsigned short dhcpMtuSize __PACK__; // dhcp MTU size
#endif // HOME_GATEWAY
	unsigned char   opMode __PACK__; // lan,wan opration mode
	unsigned char   wispWanId  __PACK__; // wisp WAN interface
#ifdef TLS_CLIENT	
	unsigned char certRootNum __PACK__; // number of certroot entry existed
	CERTROOT_T certRootArray[MAX_CERTROOT_NUM] __PACK__; // cert ca array
	unsigned char certUserNum __PACK__; // number of CERTUSER entry existed
	CERTUSER_T certUserArray[MAX_CERTUSER_NUM] __PACK__; // cert pr array	
	unsigned char rootIdx;
	unsigned char userIdx;
#endif
#ifdef HOME_GATEWAY
	unsigned char wanAccessEnabled __PACK__;
	unsigned char pingAccessEnabled __PACK__;
	unsigned char hostName[MAX_NAME_LEN] __PACK__; // dhcp client host name
#endif
	unsigned char rtLogEnabled __PACK__;
	unsigned char rtLogServer[4] __PACK__;

#ifdef UNIVERSAL_REPEATER
	// for wlan0 interface
	unsigned char repeaterEnabled1 __PACK__; // universal repeater enable/disable
	unsigned char repeaterSSID1[MAX_SSID_LEN] __PACK__;  // ssid on virtual interface

	// for wlan1 interface
	unsigned char repeaterEnabled2 __PACK__; // universal repeater enable/disable
	unsigned char repeaterSSID2[MAX_SSID_LEN] __PACK__;  // ssid on virtual interface
#endif

	unsigned char wifiSpecific __PACK__;

#ifdef HOME_GATEWAY
	unsigned char pppServiceName[41] __PACK__;
#ifdef DOS_SUPPORT
	unsigned long dosEnabled __PACK__;	
	unsigned short syssynFlood __PACK__;	
	unsigned short sysfinFlood __PACK__;	
	unsigned short sysudpFlood __PACK__;	
	unsigned short sysicmpFlood __PACK__;	
	unsigned short pipsynFlood __PACK__;	
	unsigned short pipfinFlood __PACK__;	
	unsigned short pipudpFlood __PACK__;	
	unsigned short pipicmpFlood __PACK__;	
	unsigned short blockTime __PACK__;
#endif
	unsigned char urlFilterEnabled __PACK__;	
	unsigned char urlFilterNum __PACK__; // number of url filter entry existed	
	URLFILTER_T urlFilterArray[MAX_URLFILTER_NUM] __PACK__; // url-filter array	
	unsigned char vpnPassthruIPsecEnabled __PACK__;	
	unsigned char vpnPassthruPPTPEnabled __PACK__;	
	unsigned char vpnPassthruL2TPEnabled __PACK__;
	unsigned char pptpSecurityEnabled __PACK__;
	unsigned char igmpproxyDisabled __PACK__;
	unsigned char pptpMppcEnabled __PACK__;
	unsigned short pptpIdleTime __PACK__; //pptp idle time 
	unsigned char pptpConnectType __PACK__; //pptp connection type 
#endif

// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT
	voipCfgParam_t voipCfgParam __PACK__;
#endif

#ifdef GW_QOS_ENGINE
	unsigned char	qosEnabled	__PACK__;
	unsigned char	qosAutoUplinkSpeed	__PACK__;
	unsigned long qosManualUplinkSpeed    __PACK__;
	unsigned char qosAutoDownLinkSpeed    __PACK__; 
	unsigned long qosManualDownLinkSpeed    __PACK__;

	unsigned char qosRuleNum __PACK__;
	QOS_T qosRuleArray[MAX_QOS_RULE_NUM] __PACK__;
#endif

#ifdef QOS_BY_BANDWIDTH
	unsigned char	qosEnabled	__PACK__;
	unsigned char	qosAutoUplinkSpeed	__PACK__;
	unsigned long qosManualUplinkSpeed    __PACK__;
	unsigned char qosAutoDownLinkSpeed    __PACK__; 
	unsigned long qosManualDownLinkSpeed    __PACK__;
	
	unsigned char qosRuleNum __PACK__;
	IPQOS_T qosRuleArray[MAX_QOS_RULE_NUM] __PACK__;
#endif

	unsigned char startMp __PACK__;

/*+++++added by Jack for Tr-069 configuration+++++*/
#ifdef CONFIG_CWMP_TR069
	unsigned char	cwmp_onoff __PACK__;
	unsigned char	cwmp_ProvisioningCode[CWMP_PROVISION_CODE_LEN] __PACK__;
	unsigned char	cwmp_ACSURL[CWMP_ACS_URL_LEN] __PACK__;
	unsigned char	cwmp_ACSUserName[CWMP_ACS_USERNAME_LEN] __PACK__;
	unsigned char	cwmp_ACSPassword[CWMP_ACS_PASSWD_LEN] __PACK__;
	unsigned char	cwmp_InformEnable __PACK__;
	unsigned int	cwmp_InformInterval __PACK__;
	unsigned int	cwmp_InformTime __PACK__;
	unsigned char	cwmp_ConnReqUserName[CWMP_CONREQ_USERNAME_LEN] __PACK__;
	unsigned char	cwmp_ConnReqPassword[CWMP_CONREQ_PASSWD_LEN] __PACK__;
	unsigned char	cwmp_UpgradesManaged __PACK__;
//	unsigned char	cwmp_LANConfPassword[CWMP_LANCONF_PASSWD_LEN] __PACK__;
//	unsigned char	cwmp_SerialNumber[CWMP_SERIALNUMBER_LEN] __PACK__;
//	unsigned char	cwmp_DHCP_ServerConf __PACK__;
//	unsigned char	cwmp_LAN_IPIFEnable __PACK__;
//	unsigned char	cwmp_LAN_EthIFEnable __PACK__;
//	unsigned char	cwmp_WLAN_BasicEncry __PACK__; /*0:none, 1:Wep*/
//	unsigned char	cwmp_WLAN_WPAEncry __PACK__; /*0:tkip, 1:aes, 2:tkip&aes*/
	unsigned char	cwmp_DL_CommandKey[CWMP_COMMAND_KEY_LEN+1] __PACK__;
	unsigned int	cwmp_DL_StartTime __PACK__;
	unsigned int	cwmp_DL_CompleteTime __PACK__;
	unsigned int	cwmp_DL_FaultCode __PACK__;
	unsigned int	cwmp_Inform_EventCode __PACK__;
	unsigned char	cwmp_RB_CommandKey[CWMP_COMMAND_KEY_LEN+1] __PACK__;
//	unsigned char	cwmp_ACS_ParameterKey[CWMP_COMMAND_KEY_LEN+1] __PACK__;
	unsigned char cwmp_CERT_Password[CWMP_CERT_PASSWD_LEN+1] __PACK__;
	unsigned char	cwmp_Flag __PACK__;
	unsigned char	cwmp_SI_CommandKey[CWMP_COMMAND_KEY_LEN+1] __PACK__;
#ifdef _PRMT_USERINTERFACE_
	unsigned char	 UIF_PW_Required __PACK__;
	unsigned char	 UIF_PW_User_Sel __PACK__;
	unsigned char	 UIF_Upgrade __PACK__;
	unsigned int	 UIF_WarrantyDate __PACK__;
	unsigned char	 UIF_AutoUpdateServer[256] __PACK__;
	unsigned char	 UIF_UserUpdateServer[256] __PACK__;
#endif //_PRMT_USERINTERFACE_

	unsigned char	cwmp_ACS_KickURL[CWMP_KICK_URL] __PACK__;
	unsigned char	cwmp_ACS_DownloadURL[CWMP_DOWNLOAD_URL] __PACK__;
	unsigned int 	cwmp_ConnReqPort __PACK__;
	unsigned char 	cwmp_ConnReqPath[CONN_REQ_PATH_LEN] __PACK__;	
	unsigned char	cwmp_Flag2 __PACK__;
#ifdef _PRMT_TR143_
	unsigned char	tr143_udpecho_enable __PACK__;
	unsigned char	tr143_udpecho_itftype __PACK__;
	unsigned char	tr143_udpecho_srcip[4] __PACK__;
	unsigned short	tr143_udpecho_port __PACK__;
	unsigned char	tr143_udpecho_plus __PACK__;
#endif //_PRMT_TR143_	
#endif /*CONFIG_CWMP_TR069*/

	// SNMP, Forrest added, 2007.10.25.
#ifdef CONFIG_SNMP
	unsigned char snmpEnabled __PACK__;
	unsigned char snmpName[MAX_SNMP_NAME_LEN] __PACK__;
	unsigned char snmpLocation[MAX_SNMP_LOCATION_LEN] __PACK__;
	unsigned char snmpContact[MAX_SNMP_CONTACT_LEN] __PACK__;
	unsigned char snmpRWCommunity[MAX_SNMP_COMMUNITY_LEN] __PACK__;
	unsigned char snmpROCommunity[MAX_SNMP_COMMUNITY_LEN] __PACK__;
	unsigned char snmpTrapReceiver1[4] __PACK__;
	unsigned char snmpTrapReceiver2[4] __PACK__;
	unsigned char snmpTrapReceiver3[4] __PACK__;
#endif

	unsigned short system_time_year __PACK__;
	unsigned char system_time_month __PACK__;
	unsigned char system_time_day __PACK__;
	unsigned char system_time_hour __PACK__;
	unsigned char system_time_min __PACK__;
	unsigned char system_time_sec __PACK__;
	
//=========add for MESH=========
//#ifdef CONFIG_RTK_MESH Keith remove
	unsigned char meshEnabled __PACK__;
	unsigned char meshRootEnabled __PACK__;
	unsigned char meshID[33] __PACK__;
	unsigned short meshMaxNumOfNeighbors __PACK__; 

// for backbone security
	unsigned char meshEncrypt __PACK__; // encrypt type
	unsigned char meshWpaPSKFormat __PACK__; // PSK format 0 - passphrase, 1 - hex
	unsigned char meshWpaPSK[MAX_PSK_LEN+1] __PACK__; // WPA pre-shared key
	unsigned char meshWpaAuth __PACK__; 		//  RADIUS / psk
	unsigned char meshWpa2Cipher __PACK__; 		// TKIP / AES
	
//#ifdef	_MESH_ACL_ENABLE_ Keith remove
	unsigned char meshAclEnabled __PACK__; // enable/disable MESH access control list
	unsigned char meshAclNum __PACK__; // MESH  access control entry number
	MACFILTER_T   meshAclAddrArray[MAX_MESH_ACL_NUM] __PACK__; // MESH access control array
//#endif Keith remove

#ifdef 	_11s_TEST_MODE_		
	unsigned short meshTestParam1 __PACK__;
	unsigned short meshTestParam2 __PACK__;
	unsigned short meshTestParam3 __PACK__;
	unsigned short meshTestParam4 __PACK__;
	unsigned short meshTestParam5 __PACK__;
	unsigned short meshTestParam6 __PACK__;
	unsigned short meshTestParam7 __PACK__;
	unsigned short meshTestParam8 __PACK__;
	unsigned short meshTestParam9 __PACK__;
	unsigned short meshTestParama __PACK__;
	unsigned short meshTestParamb __PACK__;
	unsigned short meshTestParamc __PACK__;
	unsigned short meshTestParamd __PACK__;
	unsigned short meshTestParame __PACK__;
	unsigned short meshTestParamf __PACK__;
	unsigned char meshTestParamStr1[16] __PACK__;
#endif
//#endif Keith remove
//=========add for MESH=========

//#ifdef SNMP_SUPPORT Keith remove
        unsigned char snmpROcommunity[MAX_SNMP_COMMUNITY_LEN] __PACK__;  // snmp agent Read-Only community
        unsigned char snmpRWcommunity[MAX_SNMP_COMMUNITY_LEN] __PACK__;  // snmp agent Read-Write community
//#endif Keith remove
/*-----end-----*/

#ifdef HOME_GATEWAY // defined(VLAN_CONFIG_SUPPORTED) Keith modify
unsigned char VlanConfigEnabled __PACK__;	
unsigned char VlanConfigNum __PACK__; // number of vlan config entry	
VLAN_CONFIG_T VlanConfigArray[MAX_IFACE_VLAN_CONFIG] __PACK__; // vlan config	
#endif

unsigned char wlan11nOnOffTKIP __PACK__;	//difu suggest
#ifdef HOME_GATEWAY
#ifdef ROUTE_SUPPORT
	unsigned char staticRouteEnabled __PACK__;
	unsigned char staticRouteNum __PACK__; // number of ip-filter entry existed
	STATICROUTE_T staticRouteArray[MAX_ROUTE_NUM] __PACK__; // ip-filter array
	unsigned char ripEnabled __PACK__;
	unsigned char ripLanTx __PACK__;
	unsigned char ripLanRx __PACK__;
	unsigned char ripWanTx __PACK__;
	unsigned char ripWanRx __PACK__;
	unsigned char natEnabled __PACK__;
#endif	
#endif // #ifdef HOME_GATEWAY
	unsigned char vpnPassthruIPv6 __PACK__;	
} APMIB_T, *APMIB_Tp;

/* Config file header */
typedef struct param_header {
	unsigned char signature[SIGNATURE_LEN] __PACK__;  // Tag + version
	unsigned short len __PACK__;
} PARAM_HEADER_T, *PARAM_HEADER_Tp;

/* Firmware image file header */
typedef struct img_header {
	unsigned char signature[SIGNATURE_LEN] __PACK__;
	unsigned int startAddr __PACK__;
	unsigned int burnAddr __PACK__;
	unsigned int len __PACK__;
} IMG_HEADER_T, *IMG_HEADER_Tp;

/* Web page file header */
typedef IMG_HEADER_T WEB_HEADER_T;
typedef IMG_HEADER_Tp WEB_HEADER_Tp;
#ifdef TLS_CLIENT
typedef IMG_HEADER_T CERT_HEADER_T;
typedef IMG_HEADER_Tp CERT_HEADER_Tp;
#endif
typedef struct _file_entry {
	char name[MAXFNAME] __PACK__;
	unsigned int size __PACK__;
} FILE_ENTRY_T, *FILE_ENTRY_Tp;

#ifdef WIN32
#pragma pack()
#endif


//////////////////////////////////////////////////////////
int apmib_init_HW(void);
int apmib_init(void);
int apmib_reinit(void);
char *apmib_hwconf(void);
char *apmib_csconf(void);
char *apmib_dsconf(void);
int apmib_get(int id, void *value);
int apmib_getDef(int id, void *value);
int apmib_set(int id, void *value);
int apmib_setDef(int id, void *value);
int apmib_update(CONFIG_DATA_T type);
int apmib_updateDef(void);
int apmib_updateFlash(CONFIG_DATA_T type, char *data, int len, int force, int ver);
int update_linkchain(int fmt, void *Entry_old, void *Entry_new, int type_size);
void apmib_default_setting(APMIB_Tp pMib);
extern APMIB_Tp pMib, pMibDef;
extern HW_SETTING_Tp pHwSetting;
extern PARAM_HEADER_T hsHeader, dsHeader, csHeader;
extern int wlan_idx;
extern int vwlan_idx;
#ifdef GW_QOS_ENGINE
extern void getVal12(char *value, char **p1, char **p2, char **p3, char **p4, char **p5, char **p6, char **p7,
                                char **p8, char **p9, char **p10, char **p11, char **p12);
#define QOS_FORMAT 	("%d, %d, %d, %s, %s, %d, %d, %s, %s, %d, %d, %s")
#endif

#ifdef QOS_BY_BANDWIDTH
//#define QOS_FORMAT 	("%d, %s, %d, %s, %s, %d, %s")
#define QOS_FORMAT 	("%d, %02x%02x%02x%02x%02x%02x, %d, %s, %s, %d, %d, %s")
#endif

#if CONFIG_APMIB_SHARED_MEMORY == 1
#define HWCONF_SHM_KEY	0
#define DSCONF_SHM_KEY	1
#define CSCONF_SHM_KEY	2

int apmib_sem_lock(void);
int apmib_sem_unlock(void);
int apmib_shm_free(void *shm_memory, int shm_key);
#endif


#ifdef CONFIG_CWMP_TR069

/* Keith add for tr069 --start */
#undef mib_get
#undef mib_set
#undef mib_update

#define mib_get(S, T)  apmib_get(S, T)
#define mib_set(S, T)  apmib_set(S, T)
#define mib_update(S)  apmib_update(S)
#define LANDEVNAME2BR0(a) do{ if(a && (strncmp(a, "eth0", 4)==0||strncmp(a, "wlan0", 5)==0||strncmp(a, "usb0", 4)==0)) strcpy(a, "br0"); }while(0)
/* Keith add for tr069 --end */

#define TIME_ZONE
#define WLAN_SUPPORT
#ifdef MBSSID
#define WLAN_MBSSID
#endif
#ifdef UNIVERSAL_REPEATER
#define WLAN_REPEATER
#endif
#define MAC_FILTER
#define ENABLE_WPAAES_WPA2TKIP

#endif //#ifdef CONFIG_CWMP_TR069

#endif // INCLUDE_APMIB_H
