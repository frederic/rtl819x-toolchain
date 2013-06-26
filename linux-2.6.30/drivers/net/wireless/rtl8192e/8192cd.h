/*
 *  Header file defines some private structures and macro
 *
 *  $Id: 8192cd.h,v 1.56.2.38 2011/01/13 14:38:36 pluswang Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef	_8192CD_H_
#define _8192CD_H_

#ifdef __KERNEL__
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#endif

#ifdef __DRAYTEK_OS__
#include <draytek/softimer.h>
#include <draytek/skbuff.h>
#include <draytek/wl_dev.h>
#endif

#include "./8192cd_cfg.h"

#if !defined(__KERNEL__) && !defined(__ECOS)
#include "./sys-support.h"
#endif

#ifdef __ECOS
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
#endif

#ifdef SUPPORT_SNMP_MIB
#include "./8192cd_mib.h"
#endif

#ifdef P2P_SUPPORT
#include "./8192cd_p2p.h"
#endif

#ifdef USE_OUT_SRC
#include "OUTSRC/odm_types.h"
#include "./odm_inc.h"
#include "OUTSRC/odm.h"
#endif

#define TRUE		1
#define FALSE		0

#define CONGESTED	2
#ifdef SUCCESS
#undef SUCCESS
#endif
#define SUCCESS		1
#define FAIL		0

#if 0
typedef unsigned char	UINT8;
typedef unsigned short	UINT16;
typedef unsigned long	UINT32;

typedef signed char		INT8;
typedef signed short	INT16;
typedef signed long		INT32;

typedef unsigned int	UINT;
typedef signed int		INT;

typedef unsigned long long	UINT64;
typedef signed long long	INT64;
#endif


#ifdef __KERNEL__
#include "./ieee802_mib.h"
#elif defined(__ECOS)
#include <cyg/io/eth/rltk/819x/wlan/ieee802_mib.h>
#endif

#include "./wifi.h"
#ifdef  CONFIG_WLAN_HAL
#include "./Wlan_TypeDef.h"
#include "./Wlan_QoSType.h"
#endif
#include "./8192cd_security.h"

#ifdef RTK_BR_EXT
#include "./8192cd_br_ext.h"
#endif

#include "./8192cd_hw.h"

#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
#include "./8192cd_psk.h"
#endif

#ifdef WIFI_SIMPLE_CONFIG
#ifdef INCLUDE_WPS
#include "./wps/wsc.h"
#endif
#endif

#ifdef WIFI_WPAS
#define MAX_WPA_IE_LEN		255+5
#define MAX_RSN_IE_LEN		255+5
#endif

#if defined(CONFIG_RTL_WAPI_SUPPORT)
#include "wapi_wai.h"
#include "wapiCrypto.h"
#endif

#ifdef CONFIG_RTK_MESH
#include "../mesh_ext/mesh.h"
#include "../mesh_ext/hash_table.h"
#include "../mesh_ext/mesh_route.h"
#include "../mesh_ext/mesh_security.h"
#endif
#define DWNGRADE_PROBATION_TIME		3
#define UPGRADE_PROBATION_TIME		3
#define TRY_RATE_FREQ				6

#ifdef CONFIG_RTL8190_PRIV_SKB
	#define MAX_PRE_ALLOC_SKB_NUM	160
#else
	#define MAX_PRE_ALLOC_SKB_NUM	32
#endif

#if defined(__LINUX_2_6__)
#if defined(CONFIG_PRINTK)
#define panic_printk         printk
#elif !defined(CONFIG_PANIC_PRINTK)
#define panic_printk		scrlog_printk
#endif
#endif

#ifdef HIGH_POWER_EXT_PA
#define HP_OFDM_POWER_MAX		(priv->pshare->rf_ft_var.hp_ofdm_pwr_max)
#define HP_CCK_POWER_MAX		(priv->pshare->rf_ft_var.hp_cck_pwr_max)
#define HP_OFDM_POWER_DEFAULT	20
#define HP_CCK_POWER_DEFAULT	16
#endif

#ifdef USB_POWER_SUPPORT
#define USB_HT_2S_DIFF			14
#define USB_RA_MASK				0x1e0fffff		// disable MCS 12, 11, 10, 9, 8

#endif

#ifdef PCIE_POWER_SAVING

#define PABCD_ISR  0xB8003510

enum ap_pwr_state {
	L0,
	L1,
	L2,
	ASPM_L0s_L1,
};

enum ps_enable {
	L1_en = BIT(0),
	L2_en = BIT(1),
	ASPM_en = BIT(2),
	_1x1_en = BIT(4),
	offload_en = BIT(5),
	stop_dma_en = BIT(6),
};

enum pwr_state_change {
// Renamed by Annie for ODM OUTSRC porting and conflict naming issue, 2011-09-22
	PWR_STATE_IN=1,
	PWR_STATE_OUT=2
};
#endif

// for packet aggregation
#define FG_AGGRE_MPDU				1
#define FG_AGGRE_MPDU_BUFFER_FIRST	2
#define FG_AGGRE_MPDU_BUFFER_MID	3
#define FG_AGGRE_MPDU_BUFFER_LAST	4
#define FG_AGGRE_MSDU_FIRST			5
#define FG_AGGRE_MSDU_MIDDLE		6
#define FG_AGGRE_MSDU_LAST			7

#define MANAGEMENT_AID				0

enum wifi_state {
	WIFI_NULL_STATE		=	0x00000000,
	WIFI_ASOC_STATE		=	0x00000001,
	WIFI_REASOC_STATE	=	0x00000002,
	WIFI_SLEEP_STATE	=	0x00000004,
	WIFI_STATION_STATE	=	0x00000008,
	WIFI_AP_STATE		=	0x00000010,
	WIFI_ADHOC_STATE	=	0x00000020,
	WIFI_AUTH_NULL		=	0x00000100,
	WIFI_AUTH_STATE1	= 	0x00000200,
	WIFI_AUTH_SUCCESS	=	0x00000400,
	WIFI_SITE_MONITOR	=	0x00000800,		//to indicate the station is under site surveying
#ifdef WDS
	WIFI_WDS			=	0x00001000,
	WIFI_WDS_RX_BEACON	=	0x00002000,		// already rx WDS AP beacon
#ifdef LAZY_WDS
	WIFI_WDS_LAZY			=	0x00004000,
#endif
#endif

#ifdef MP_TEST
	WIFI_MP_STATE					= 0x00010000,
	WIFI_MP_CTX_BACKGROUND			= 0x00020000,	// in continuous tx background
	WIFI_MP_CTX_BACKGROUND_PENDING	= 0x00040000,	// pending in continuous tx background due to out of skb
	WIFI_MP_CTX_PACKET				= 0x00080000,	// in packet mode
	WIFI_MP_CTX_ST					= 0x00100000,	// in continuous tx with single-tone
	WIFI_MP_CTX_SCR					= 0x00200000,	// in continuous tx with single-carrier
	WIFI_MP_CTX_CCK_CS				= 0x00400000,	// in cck continuous tx with carrier suppression
	WIFI_MP_CTX_OFDM_HW				= 0x00800000,	// in ofdm continuous tx
	WIFI_MP_RX							= 0x01000000,
	WIFI_MP_ARX_FILTER				= 0x02000000,
#if 1//def CONFIG_RTL8672
	WIFI_MP_CTX_BACKGROUND_STOPPING	= 0x04000000,	// stopping ctx
#endif
#endif

#ifdef WIFI_SIMPLE_CONFIG
#ifdef INCLUDE_WPS
	WIFI_WPS			=	0x02000000,
	WIFI_WPS_JOIN		=	0x04000000,
#else
	WIFI_WPS			= 0x01000000,
	WIFI_WPS_JOIN		= 0x02000000,
#endif
#endif

#ifdef A4_STA
	WIFI_A4_STA		=	0x04000000,
#endif


#ifdef CONFIG_RTL8672
    WIFI_WAIT_FOR_CHANNEL_SELECT    = 0x04000000,
#endif

#ifdef P2P_SUPPORT
	WIFI_P2P_SUPPORT	=	0x08000000
#endif
};

enum frag_chk_state {
	NO_FRAG		= 0x0,
	UNDER_FRAG	= 0x1,
	CHECK_FRAG	= 0x2,
};

enum led_type {
	LEDTYPE_HW_TX_RX,
	LEDTYPE_HW_LINKACT_INFRA,
	LEDTYPE_SW_LINK_TXRX,
	LEDTYPE_SW_LINKTXRX,
	LEDTYPE_SW_LINK_TXRXDATA,
	LEDTYPE_SW_LINKTXRXDATA,
	LEDTYPE_SW_ENABLE_TXRXDATA,
	LEDTYPE_SW_ENABLETXRXDATA,
	LEDTYPE_SW_ADATA_GDATA,
	LEDTYPE_SW_ENABLETXRXDATA_1,
	LEDTYPE_SW_CUSTOM1,
	LEDTYPE_SW_LED2_GPIO8_LINKTXRX,					// 11
	LEDTYPE_SW_LED2_GPIO8_ENABLETXRXDATA,			// 12
	LEDTYPE_SW_LED2_GPIO10_LINKTXRX,				// 13
	LEDTYPE_SW_RESERVED,							// 14, redirect to 52
	LEDTYPE_SW_LED2_GPIO8_LINKTXRXDATA,				// 15
	LEDTYPE_SW_LED2_GPIO8_ASOCTXRXDATA,  			// 16, mark_led	
	// Latest 92D customized LED types start from 50
	LEDTYPE_SW_LED2_GPIO10_ENABLETXRXDATA = 50,		// 50 for 92D, LED Control = 92C LED_TYPE 7
	LEDTYPE_SW_LED1_GPIO9_LINKTXRX_92D = 51,		// 51 for 92D, LED Control = 92C LED_TYPE 13
	LEDTYPE_SW_LED2_GPIO10_LINKTXRX_92D = 52,		// 52 for 92D, LED Control = 92C LED_TYPE 13
	LEDTYPE_SW_MAX,
};

enum Synchronization_Sta_State {
	STATE_Sta_Min				= 0,
	STATE_Sta_No_Bss			= 1,
	STATE_Sta_Bss				= 2,
	STATE_Sta_Ibss_Active		= 3,
	STATE_Sta_Ibss_Idle			= 4,
	STATE_Sta_Auth_Success		= 5,
	STATE_Sta_Roaming_Scan		= 6,
};

// Realtek proprietary IE
enum Realtek_capability_IE_bitmap {
	RTK_CAP_IE_TURBO_AGGR		= 0x01,
	RTK_CAP_IE_USE_LONG_SLOT	= 0x02,
	RTK_CAP_IE_USE_AMPDU		= 0x04,
#ifdef RTK_WOW
	RTK_CAP_IE_USE_WOW		= 0x08,
#endif
	RTK_CAP_IE_SOFTAP			= 0x10,
	RTK_CAP_IE_WLAN_8192SE	= 0x20,
	RTK_CAP_IE_WLAN_88C92C		= 0x40,
	RTK_CAP_IE_AP_CLIENT		= 0x80,
	RTK_CAP_IE_VIDEO_ENH		= 0x01,
};

enum CW_STATE {
	CW_STATE_NORMAL				= 0x00000000,
	CW_STATE_AGGRESSIVE			= 0x00010000,
	CW_STATE_DIFSEXT			= 0x00020000,
	CW_STATE_AUTO_TRUBO			= 0x01000000,
};

enum {TURBO_AUTO=0, TURBO_ON=1, TURBO_OFF=2};

enum NETWORK_TYPE {
	WIRELESS_11B = 1,
	WIRELESS_11G = 2,
	WIRELESS_11A = 4,
	WIRELESS_11N = 8,
	WIRELESS_11AC = 64
};

enum FREQUENCY_BAND {
	BAND_2G,
	BAND_5G
};

enum _HT_CHANNEL_WIDTH {
	HT_CHANNEL_WIDTH_20		= 0,
	HT_CHANNEL_WIDTH_20_40	= 1,
	HT_CHANNEL_WIDTH_80		= 2,
	HT_CHANNEL_WIDTH_160	= 3,
	HT_CHANNEL_WIDTH_10		= 4
};

#ifdef RTK_AC_SUPPORT
enum _HT_CHANNEL_WIDTH_AC {
	HT_CHANNEL_WIDTH_AC_20	= 0,
	HT_CHANNEL_WIDTH_AC_40	= 1, 
	HT_CHANNEL_WIDTH_AC_80 	= 2,
	HT_CHANNEL_WIDTH_AC_160	= 3,
	HT_CHANNEL_WIDTH_AC_10	= 4,
	HT_CHANNEL_WIDTH_AC_5 	= 5
	};
#endif

#ifdef RTK_AC_SUPPORT
enum _SUB_CHANNEL_INDEX_80M {
	_20_B_40_A = 1,
	_20_A_40_B = 2,
	_20_A_40_A = 3, 
	_20_B_40_B = 4,
	_40_A	= 9,
	_40_B	= 10
	};

#endif


enum SECONDARY_CHANNEL_OFFSET {
	HT_2NDCH_OFFSET_DONTCARE = 0,
	HT_2NDCH_OFFSET_BELOW    = 1,	// secondary channel is below primary channel, ex. primary:5 2nd:1
	HT_2NDCH_OFFSET_ABOVE    = 2	// secondary channel is above primary channel, ex. primary:5 2nd:9
};

enum AGGREGATION_METHOD {
	AGGRE_MTHD_NONE = 0,
	AGGRE_MTHD_MPDU = 1,
	AGGRE_MTHD_MSDU = 2
};

enum _HT_CURRENT_TX_INFO_ {
	TX_USE_40M_MODE		= BIT(0),
	TX_USE_SHORT_GI		= BIT(1),
	TX_USE_80M_MODE     = BIT(2),
	TX_USE_160M_MODE    = BIT(3)	
};

/*
enum _ADD_RATID_UPDATE_CONTENT_ {
	RATID_NONE_UPDATE = 0,
	RATID_GENERAL_UPDATE = 1,
	RATID_INFO_UPDATE = 2
};
*/

enum _DC_TH_CURRENT_STATE_ {
	DC_TH_USE_NONE	= 0,
	DC_TH_USE_UPPER	= 1,
	DC_TH_USE_LOWER	= 2
};

enum _H2C_CMD_ID_ {
 	_AP_OFFLOAD_CMD_ = 0 ,  /*0*/
 	_SETPWRMODE_CMD_,
 	_JOINBSSRPT_CMD_,
 	_RSVDPAGE_CMD_,
 	_SET_RSSI_4_CMD_,
	H2C_CMD_RSSI		= 5,
	H2C_CMD_MACID		= 6,
	H2C_CMD_PS		= 7,
	_P2P_PS_OFFLOAD_CMD_	= 8,
	SELECTIVE_SUSPEND_ROF = 9,
	AP_OFFLOAD_RFCTRL = 10,
	AP_REQ_RPT = 11,
	BT_COEX_DUTY_CYCLE = 12,
	H2C_CMD_INFO_PKT = 13,
	H2C_CMD_SMCC = 14,
	H2C_CMD_AP_WPS_CTRL = 64
};

enum _ANTENNA_ {
	ANTENNA_A		= 0x1,
	ANTENNA_B		= 0x2,
	ANTENNA_C		= 0x4,
	ANTENNA_D		= 0x8,
	ANTENNA_AC		= 0x5,
	ANTENNA_AB		= 0x3,
	ANTENNA_BD		= 0xA,
	ANTENNA_CD		= 0xC,
	ANTENNA_ABCD	= 0xF
};

enum qos_prio { BK, BE, VI, VO, VI_AG, VO_AG };

#ifdef WIFI_HAPD
enum HAPD_EVENT{
	HAPD_EXIRED = 0,
	HAPD_REGISTERED = 1,
	HAPD_MIC_FAILURE = 2,
	HAPD_TRAFFIC_STAT = 3,
	HAPD_PUSH_BUTTON = 4,
	HAPD_WPS_PROBEREQ =5,
	HAPD_WDS_SETWPA = 6
};
#endif

enum ACL_MODE{
	ACL_allow = 1,
	ACL_deny = 2
};

#ifdef WIFI_WPAS
enum WPAS_EVENT{
	WPAS_EXIRED = 10,
	WPAS_REGISTERED = 11,
	WPAS_MIC_FAILURE = 12,
	WPAS_ASSOC_INFO = 13,
	WPAS_SCAN_DONE = 14
};

#define REQIELEN		123
#define RESPIELEN		123

typedef struct _WPAS_ASSOCIATION_INFO
{
		unsigned short	ReqIELen;
		char			ReqIE[REQIELEN];
		unsigned short  RespIELen;
        char            RespIE[RESPIELEN];
} WPAS_ASSOCIATION_INFO;
#endif

static const struct ParaRecord rtl_ap_EDCA[] =
{
//ACM,AIFSN, ECWmin, ECWmax, TXOplimit
     {0,     7,      4,      10,     0},
     {0,     3,      4,      6,      0},
     {0,     1,      3,      4,      188},
     {0,     1,      2,      3,      102},
     {0,     1,      3,      4,      94},
     {0,     1,      2,      3,      47},
};

static const struct ParaRecord rtl_sta_EDCA[] =
{
//ACM,AIFSN, ECWmin, ECWmax, TXOplimit
     {0,     7,      4,      10,     0},
     {0,     3,      4,      10,     0},
     {0,     2,      3,      4,      188},
     {0,     2,      2,      3,      102},
     {0,     2,      3,      4,      94},
     {0,     2,      2,      3,      47},
};


struct pkt_queue {
	struct sk_buff	*pSkb[NUM_TXPKT_QUEUE];
	int	head;
	int	tail;
};

#if defined(WIFI_WMM) && defined(WMM_APSD)
struct apsd_pkt_queue {
	struct sk_buff	*pSkb[NUM_APSD_TXPKT_QUEUE];
	int				head;
	int				tail;
};
#endif

#if defined(WIFI_WMM)
struct dz_mgmt_queue{
	struct tx_insn	*ptx_insn[NUM_DZ_MGT_QUEUE];
	int				head;
	int				tail;
};

#ifdef DZ_ADDBA_RSP
// dz addba
struct dz_addba_info{
	unsigned char used;
	unsigned char dialog_token;
	unsigned char TID;
	unsigned short status_code;
	unsigned short timeout;
};
#endif
#endif

#if defined(GREEN_HILL) || defined(PACK_STRUCTURE) || defined(__ECOS)
#pragma pack(1)
#endif


/**
 *	@brief MAC Frame format - wlan_hdr (wireless LAN header)
 *
 *	Dcscription: 802.11 MAC Frame (header). See textbook P.46
 *	p.s : memory aligment by BYTE,
 *	      __PACK : not need 4 bytes aligment
 */
__PACK struct wlan_hdr {
	unsigned short	fmctrl;
	unsigned short	duration;
	unsigned char	addr1[MACADDRLEN];
	unsigned char	addr2[MACADDRLEN];
	unsigned char	addr3[MACADDRLEN];
	unsigned short	sequence;
	unsigned char	addr4[MACADDRLEN];
	unsigned short	qosctrl;
#if defined(CONFIG_RTK_MESH) && defined(RX_RL_SHORTCUT)
	struct lls_mesh_header  meshhdr;        // 11s, mesh header, 4~16 bytes
#endif
	unsigned char	iv[8];
} __WLAN_ATTRIB_PACK__;

#if defined(GREEN_HILL) || defined(PACK_STRUCTURE) || defined(__ECOS)
#pragma pack()
#endif

struct wlan_hdrnode {
	struct list_head	list;
#ifdef TX_EARLY_MODE
	unsigned char		em_info[8];		// early mode info
#endif		
	struct wlan_hdr		hdr;
};

struct wlan_hdr_poll {
	struct wlan_hdrnode hdrnode[PRE_ALLOCATED_HDR];
	int					count;
};

#if defined(GREEN_HILL) || defined(PACK_STRUCTURE) || defined(__ECOS)
#pragma pack(1)
#endif

__PACK struct wlanllc_hdr {
	struct wlan_hdr		wlanhdr;
#if defined(CONFIG_RTL_WAPI_SUPPORT)
	wpiSMS4Hdr			sms4hdr;		//always be used for wapi wlan hdr: iv [and mic]
#endif
	struct llc_snap		llcsnaphdr;
} __WLAN_ATTRIB_PACK__;

#if defined(GREEN_HILL) || defined(PACK_STRUCTURE) || defined(__ECOS)
#pragma pack()
#endif

struct wlanllc_node {
	struct list_head	list;
#ifdef TX_EARLY_MODE
	unsigned char		em_info[8];		// early mode info
#endif	
	struct wlanllc_hdr	hdr;

#ifdef CONFIG_RTK_MESH
	unsigned char amsdu_header[30];
#else
    unsigned char amsdu_header[14];
#endif

};

struct wlanllc_hdr_poll {
	struct wlanllc_node	hdrnode[PRE_ALLOCATED_HDR];
	int					count;
};

struct wlanbuf_node {
	struct list_head	list;
	unsigned int		buf[PRE_ALLOCATED_BUFSIZE]; // 4 bytes alignment!
};

struct wlanbuf_poll {
	struct wlanbuf_node	hdrnode[PRE_ALLOCATED_MMPDU];
	int					count;
};

struct wlanicv_node {
	struct list_head	list;
	unsigned int		icv[2];
};

struct wlanicv_poll {
	struct wlanicv_node	hdrnode[PRE_ALLOCATED_HDR];
	int					count;
};

struct wlanmic_node {
	struct list_head	list;
	unsigned int		mic[2];
};

struct wlanmic_poll {
	struct wlanmic_node	hdrnode[PRE_ALLOCATED_HDR];
	int					count;
};

struct wlan_acl_node {
	struct list_head	list;
	unsigned char		addr[MACADDRLEN];
	unsigned char		mode;
};

struct wlan_acl_poll {
	struct wlan_acl_node aclnode[NUM_ACL];
};

#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_)
struct mesh_acl_poll {
	struct wlan_acl_node meshaclnode[NUM_MESH_ACL];
};
#endif
struct tx_insn	{
	unsigned int		q_num;
	void				*pframe;
	unsigned char		*phdr;			//in case of mgt frame, phdr is wlan_hdr,
										//in case of data, phdr = wlan + llc
	unsigned int		hdr_len;
	unsigned int		fr_type;
	unsigned int		fr_len;
	unsigned int		frg_num;
	unsigned int		need_ack;
	unsigned int		frag_thrshld;
	unsigned int		rts_thrshld;
	unsigned int		privacy;
	unsigned int		iv;
	unsigned int		icv;
	unsigned int		mic;
	unsigned char		llc;
	unsigned char		tx_rate;
	unsigned char		lowest_tx_rate;
	unsigned char		fixed_rate;
	unsigned char		retry;
	unsigned char		aggre_en;
	unsigned char		tpt_pkt;
	unsigned char		one_txdesc;
#ifdef WDS
	int					wdsIdx;
#endif
	struct stat_info	*pstat;

#ifdef CONFIG_RTK_MESH
	unsigned char		is_11s;			// for transmitting 11s data frame (to rewrite 4 addresses)
	unsigned char		nhop_11s[MACADDRLEN]; // to record "da" in start_xmit
	unsigned char		prehop_11s[MACADDRLEN];
	struct  lls_mesh_header mesh_header;
#endif

#ifdef SUPPORT_TX_MCAST2UNI
	unsigned char		isMC2UC;
#endif

};

struct reorder_ctrl_entry
{
	struct sk_buff		*packet_q[RC_ENTRY_NUM];
	unsigned char		start_rcv;
	short				rc_timer_id;
	unsigned short		win_start;
	unsigned short		last_seq;
};

#ifdef SUPPORT_TX_MCAST2UNI
struct ip_mcast_info {
	int					used;
	unsigned char		mcmac[MACADDRLEN];
};
#endif

#ifdef TX_SHORTCUT
struct tx_sc_entry {
	struct tx_insn		txcfg;
	struct wlanllc_hdr 	wlanhdr;
#ifdef CONFIG_WLAN_HAL
//    PVOID   phw_desc1;
// 40 is SIZE_TXDESC_88XX
    u1Byte  hal_hw_desc[40];
#endif // CONFIG_WLAN_HAL
	struct tx_desc		hwdesc1;
	struct tx_desc		hwdesc2;
	struct tx_desc_info	swdesc1;
	struct tx_desc_info	swdesc2;
#ifdef TX_SCATTER
	struct tx_desc		hwdesc3;
	struct tx_desc_info	swdesc3;
	int	has_desc3;
#endif	
	int					sc_keyid;
	struct wlan_ethhdr_t	ethhdr;
	unsigned char		pktpri;
};
#endif

#ifdef SW_TX_QUEUE
#define CHECK_DEC_AGGN		0
#define CHECK_INC_AGGN		1
#define MAX_BACKOFF_CNT		8

struct sw_tx_q {
	struct sk_buff_head     be_queue;
    struct sk_buff_head     bk_queue;
    struct sk_buff_head     vi_queue;
	struct sk_buff_head     vo_queue;
	struct timer_list       beq_timer;
    struct timer_list       bkq_timer;
    struct timer_list       viq_timer;
    struct timer_list       voq_timer;
	int           			beq_empty;      //0:empty; 1:not empty
    int           			bkq_empty;
    int           			viq_empty;
    int           			voq_empty;
    int						q_aggnum[8];
	int						q_TOCount[8];
	unsigned char 			q_used[8];
	unsigned char			q_aggnumIncSlow[8];
};
#endif

struct stat_info {
	struct list_head	hash_list;	// always keep the has_list as the first item, to accelerat searching
	struct list_head	asoc_list;
	struct list_head	auth_list;
	struct list_head	sleep_list;
	struct list_head	defrag_list;
	struct list_head	wakeup_list;
	struct list_head	frag_list;
	struct list_head	addRAtid_list;	// to avoid add RAtid fail
	struct list_head	addrssi_list;
	struct list_head	addps_list;

	struct sk_buff_head	dz_queue;	// Queue for sleeping mode

#ifdef SW_TX_QUEUE
        struct sw_tx_q  swq;
#endif

#ifdef CONFIG_RTK_MESH
	struct list_head	mesh_mp_ptr;	// MESH MP list
#endif

#ifdef A4_STA
	struct list_head	a4_sta_list;
#endif

#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
	WPA_STA_INFO		*wpa_sta_info;
#endif

#ifdef TLN_STATS
	unsigned int		enterpise_wpa_info;
#endif

#ifdef WIFI_HAPD
	u_int8_t wpa_ie[256];
#ifndef HAPD_DRV_PSK_WPS
	u_int8_t wps_ie[256];
#endif
#endif

#if defined(WIFI_WMM) && defined(WMM_APSD)
	unsigned char		apsd_bitmap;	// bit 0: VO, bit 1: VI, bit 2: BK, bit 3: BE
	unsigned int		apsd_pkt_buffering;
	struct apsd_pkt_queue	*VO_dz_queue;
	struct apsd_pkt_queue	*VI_dz_queue;
	struct apsd_pkt_queue	*BE_dz_queue;
	struct apsd_pkt_queue	*BK_dz_queue;
#endif

#if defined(WIFI_WMM)
	struct dz_mgmt_queue	*MGT_dz_queue;
#ifdef DZ_ADDBA_RSP
	struct dz_addba_info	dz_addba;
#endif
#endif

	struct reorder_ctrl_entry	rc_entry[8];

	/****************************************************************
	 * from here on, data will be clear in init_stainfo() except "aid" and "hwaddr"  *
	 ****************************************************************/
	unsigned int		auth_seq;
	unsigned char		chg_txt[128];

	unsigned int		frag_to;
	unsigned int		frag_count;
	unsigned int		sleep_to;
	unsigned short		tpcache[8][TUPLE_WINDOW];
	unsigned short		tpcache_mgt;	// mgt cache number

#ifdef CLIENT_MODE
    unsigned short      tpcache_mcast;  // for client mode broadcast or multicast used
#endif

#ifdef _DEBUG_RTL8192CD_
	unsigned int		rx_amsdu_err;
	unsigned int		rx_amsdu_1pkt;
	unsigned int		rx_amsdu_2pkt;
	unsigned int		rx_amsdu_3pkt;
	unsigned int		rx_amsdu_4pkt;
	unsigned int		rx_amsdu_5pkt;
	unsigned int		rx_amsdu_gt5pkt;

	unsigned int		rx_rc_drop1;
	unsigned int		rx_rc_drop3;
	unsigned int		rx_rc_drop4;
	unsigned int		rx_rc_reorder3;
	unsigned int		rx_rc_reorder4;
	unsigned int		rx_rc_passup2;
	unsigned int		rx_rc_passup3;
	unsigned int		rx_rc_passup4;
	unsigned int		rx_rc_passupi;
#endif

#if defined(CLIENT_MODE) || defined(WDS)
	unsigned int		beacon_num;
#endif

#ifdef TX_SHORTCUT
	struct tx_sc_entry	tx_sc_ent[TX_SC_ENTRY_NUM];
	int					tx_sc_replace_idx;
	int 				protection;
	unsigned int		ht_protection;
#endif

#ifdef RX_SHORTCUT
	int					rx_payload_offset;
	int					rx_trim_pad;
	struct wlan_ethhdr_t	rx_ethhdr;
	struct wlanllc_hdr 	rx_wlanhdr;
	int					rx_privacy;
#endif

#ifdef SUPPORT_TX_AMSDU
	struct sk_buff_head	amsdu_tx_que[8];
	int					amsdu_timer_id[8];
	int					amsdu_size[8];
#endif

	/******************************************************************
	 * from here to end, data will be backup when doing FAST_RECOVERY *
	 ******************************************************************/
	unsigned short		aid;
#ifdef STA_EXT
	unsigned short		remapped_aid;// support up to 64 clients
#endif
	unsigned char		hwaddr[MACADDRLEN];
#ifdef WIFI_WMM
	unsigned int 		QosEnabled;
	unsigned short		AC_seq[8];
#endif
	enum wifi_state		state;
	unsigned int		AuthAlgrthm;		// could be open/shared key
	unsigned int		ieee8021x_ctrlport;	// 0 for blocked, 1 for open
	unsigned int		keyid;				// this could only be valid in legacy wep
	unsigned int		keylen;
	struct Dot11KeyMappingsEntry	dot11KeyMapping;
	unsigned char		bssrateset[32];
	unsigned int		bssratelen;
	unsigned int		useShortPreamble;
	unsigned int		expire_to;
	unsigned int 		idle_count;
	unsigned char		rssi;
	unsigned char		sq;
	unsigned char		rx_rate;
	unsigned char		rx_bw;
	unsigned char		rx_splcp;
	struct rf_misc_info	rf_info;
	unsigned short		seq_backup;
	unsigned char		rssi_backup;
	unsigned char		sq_backup;
	unsigned char		rx_rate_backup;
	unsigned char		rx_bw_backup;
	unsigned char		rx_splcp_backup;
	struct rf_misc_info rf_info_backup;
	int					cck_mimorssi_total[4];
	unsigned char		cck_rssi_num;
	unsigned char		highest_rx_rate;
	unsigned char		rssi_level;
	unsigned char		hp_level;
	unsigned char		is_realtek_sta;
#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
	unsigned char		is_apclient;
	unsigned char		is_rtl8192s_sta;
	unsigned char		is_rtl8192cd_apclient;
	unsigned char		is_rtl81xxc_sta;
	unsigned char		is_broadcom_sta;
	unsigned char		is_ralink_sta;
	unsigned char		is_marvell_sta;
	unsigned char		is_intel_sta;
#ifdef RTK_WOW
	unsigned char		is_rtk_wow_sta;
#endif

#endif	
	unsigned char		leave;
	unsigned char		no_rts;
	unsigned char		is_2t_mimo_sta;
	unsigned int		check_init_tx_rate;

	unsigned char		is_forced_ampdu;
	unsigned char		is_forced_rts;
	unsigned char		aggre_mthd;
	unsigned char		tx_bw;
	unsigned char		ht_current_tx_info;	// bit0: 0=20M mode, 1=40M mode; bit1: 0=longGI, 1=shortGI
	unsigned char		tmp_rmv_key;
	unsigned long		link_time;
	unsigned char		private_ie[32];
	unsigned int		private_ie_len;
	unsigned int		tx_ra_bitmap;

	unsigned int		tx_bytes;
	unsigned int		rx_bytes;
	unsigned int		tx_bytes_1m;
	unsigned int		rx_bytes_1m;
	unsigned int		tx_pkts;
	unsigned int		rx_pkts;
	unsigned int		tx_fail;
	unsigned int		tx_pkts_pre;
	unsigned int		rx_pkts_pre;
	unsigned int		tx_fail_pre;
	unsigned int		current_tx_rate;
	unsigned int		tx_byte_cnt;
	unsigned int		tx_avarage;
	unsigned int		rx_byte_cnt;
	unsigned int		rx_avarage;
	unsigned int		tx_conti_fail_cnt;	// continuous tx fail count; added by Annie, 2010-08-10.
	unsigned long		tx_last_good_time;	// up_time record for last good tx; added by Annie, 2010-08-10.

	// bcm old 11n chipset iot debug, and TXOP enlarge
	unsigned int		current_tx_bytes;
	unsigned int		current_rx_bytes;

#ifdef PREVENT_BROADCAST_STORM
	unsigned int		rx_pkts_bc;
#endif

#ifdef WDS
	int					wds_idx;
	unsigned int		wds_probe_done;
	unsigned int		idle_time;
#endif

#ifdef RTK_AC_SUPPORT
	struct vht_cap_elmt 	vht_cap_buf;
	unsigned int			vht_cap_len;
	struct vht_oper_elmt	vht_oper_buf;
	unsigned int			vht_oper_len;
	unsigned char			nss;
#endif
#if defined(RTK_AC_SUPPORT) || defined(CONFIG_WLAN_HAL)
	unsigned char	 		ratr_idx;
#endif 

	struct ht_cap_elmt	ht_cap_buf;
	unsigned int		ht_cap_len;
	struct ht_info_elmt	ht_ie_buf;
	unsigned int		ht_ie_len;
	unsigned char		cam_id;
	unsigned char		MIMO_ps;
	unsigned char		dialog_token;
	unsigned char		is_8k_amsdu;
	unsigned char		ADDBA_ready[8];
	unsigned char		ADDBA_req_num[8];
	unsigned char		ADDBA_sent[8];
	unsigned int		diffAmpduSz;
	unsigned int		amsdu_level;
	unsigned char		tmp_mic_key[8];

#ifdef GBWC
	unsigned char		GBWC_in_group;
#endif

	unsigned int		is_legacy_encrpt;			// 1: TKIP, 2: WEP

#ifdef  SUPPORT_TX_MCAST2UNI
	int					ipmc_num;
	struct ip_mcast_info	ipmc[MAX_IP_MC_ENTRY];
#endif

#ifdef USB_PKT_RATE_CTRL_SUPPORT
	unsigned int		change_toggle;
#endif

#ifdef STA_EXT
	int					sta_in_firmware;
#endif
#ifdef CONFIG_RTK_MESH
	struct MESH_Neighbor_Entry	mesh_neighbor_TBL;	//mesh_neighbor

	// Throughput statistics (sounder)
	//	struct flow_stats		f_stats;
#endif
#ifdef CONFIG_RTL_WAPI_SUPPORT
	wapiStaInfo		*wapiInfo;
#endif
#ifdef HW_ANT_SWITCH
		int hwRxAntSel[2];
		int AntRSSI[2];
		int cckPktCount[2];
		char CurAntenna;
#endif
	unsigned int		retry_inc;
#ifdef P2P_SUPPORT
		unsigned char is_p2p_client;
#endif

#ifdef TX_EARLY_MODE
	int empkt_num;	// how many pending packet in next
	int emextra_len;	// extra 802.11 length for each 802.3 pkt
	int empkt_len[10];	// packet length for next pending packet
#endif
	unsigned long def_expired_time;
	unsigned long def_expired_throughput;
#ifdef CONFIG_RTL_88E_SUPPORT
	unsigned char txpause_flag;
	unsigned long txpause_time;
#endif
#ifdef USE_OUT_SRC
	// Driver Write
//	BOOLEAN		bValid;				// record the sta status link or not?
	UINT8		WirelessMode;		// 
	UINT8		IOTPeer;			// Enum value.	HT_IOT_PEER_E

	// ODM Write
	//1 PHY_STATUS_INFO
	UINT8		RSSI_Path[4];		// 
	UINT8		RSSI_Ave;
	UINT8		RXEVM[4];
	UINT8		RXSNR[4];
	RSSI_STA	rssi_stat;
#endif
	unsigned int			low_tp_disable_ampdu;
};


/*
 *	Driver open, alloc whole amount of aid_obj, avoid memory fragmentation
 *	If STA Association , The aid_obj will chain to stat_info.
 */
struct aid_obj {
	struct stat_info	station;
	unsigned int		used;	// used == TRUE => has been allocated, used == FALSE => can be allocated
	struct rtl8192cd_priv *priv;
};

/* Note: always calculate the WLAN average throughput, if the throughput is larger than TP_HIGH_WATER_MARK,
             gCpuCanSuspend will be FALSE. If the throughput is smaller than TP_LOW_WATER_MARK,
             gCpuCanSuspend will be TRUE.
             However, you can undefine the CONFIG_RTL8190_THROUGHPUT. The gCpuCanSuspend will always be
             TRUE in this case.
 */
#define CONFIG_RTL8190_THROUGHPUT

struct extra_stats {
	unsigned long		tx_retrys;
	unsigned long		tx_drops;
	unsigned long		rx_retrys;
	unsigned long		rx_decache;
	unsigned long		rx_data_drops;
	unsigned long		beacon_ok;
	unsigned long		beacon_er;
	unsigned long 		beaconQ_sts;
#if defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8190_THROUGHPUT) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD) || defined(CONFIG_RTL8196C_EC)
	unsigned long		tx_peak;
	unsigned long		rx_peak;
#endif
	unsigned long		tx_dma_err;
	unsigned long		rx_dma_err;
	
	unsigned long		tx_byte_cnt;
	unsigned long		tx_avarage;
	unsigned long		rx_byte_cnt;
	unsigned long		rx_avarage;
	unsigned long		rx_fifoO;
	unsigned long		rx_rdu;
	unsigned long		freeskb_err;
	unsigned long		reused_skb;
#if defined(CONFIG_RTL_NOISE_CONTROL) || defined(CONFIG_RTL_NOISE_CONTROL_92C)
	unsigned int		tp_average_pre;
#endif

#if defined(__ECOS) && defined(_DEBUG_RTL8192CD_)
	unsigned long		tx_cnt_nosc;
	unsigned long		tx_cnt_sc1;
	unsigned long		tx_cnt_sc2;
	unsigned long		rx_cnt_nosc;
	unsigned long		rx_cnt_sc;
	unsigned long		br_cnt_nosc;
	unsigned long		br_cnt_sc;
#endif	
};


#ifdef TLN_STATS
struct tln_wifi_stats {
	unsigned long		connected_sta;
	unsigned long		max_sta;
	unsigned long		max_sta_timestamp;
	unsigned long		rejected_sta;
};


struct tln_ext_wifi_stats {
	unsigned long		rson_UNSPECIFIED_1;
	unsigned long		rson_AUTH_INVALID_2;
	unsigned long		rson_DEAUTH_STA_LEAVING_3;
	unsigned long		rson_INACTIVITY_4;
	unsigned long		rson_RESOURCE_INSUFFICIENT_5;
	unsigned long		rson_UNAUTH_CLS2FRAME_6;
	unsigned long		rson_UNAUTH_CLS3FRAME_7;
	unsigned long		rson_DISASSOC_STA_LEAVING_8;
	unsigned long		rson_ASSOC_BEFORE_AUTH_9;
	unsigned long		rson_INVALID_IE_13;
	unsigned long		rson_MIC_FAILURE_14;
	unsigned long		rson_4WAY_TIMEOUT_15;
	unsigned long		rson_GROUP_KEY_TIMEOUT_16;
	unsigned long		rson_DIFF_IE_17;
	unsigned long		rson_MCAST_CIPHER_INVALID_18;
	unsigned long		rson_UCAST_CIPHER_INVALID_19;
	unsigned long		rson_AKMP_INVALID_20;
	unsigned long		rson_UNSUPPORT_RSNIE_VER_21;
	unsigned long		rson_RSNIE_CAP_INVALID_22;
	unsigned long		rson_802_1X_AUTH_FAIL_23;
	unsigned long		rson_OUT_OF_SCOPE;

	unsigned long		status_FAILURE_1;
	unsigned long		status_CAP_FAIL_10;
	unsigned long		status_NO_ASSOC_11;
	unsigned long		status_OTHER_12;
	unsigned long		status_NOT_SUPPORT_ALG_13;
	unsigned long		status_OUT_OF_AUTH_SEQ_14;
	unsigned long		status_CHALLENGE_FAIL_15;
	unsigned long		status_AUTH_TIMEOUT_16;
	unsigned long		status_RESOURCE_INSUFFICIENT_17;
	unsigned long		status_RATE_FAIL_18;
	unsigned long		status_OUT_OF_SCOPE;
};
#endif


#ifdef WIFI_SIMPLE_CONFIG
#ifndef INCLUDE_WPS
struct wps_ie_info {
	unsigned char rssi;
	unsigned char data[MAX_WSC_IE_LEN];
};
#endif

#ifdef WIFI_WPAS
struct wpa_ie_info {
	unsigned char 	rssi;
	unsigned int	wpa_ie_len;
	unsigned char 	data[MAX_WPA_IE_LEN];
};
struct rsn_ie_info {
	unsigned char 	rssi;
	unsigned int	rsn_ie_len	;
	unsigned char 	data[MAX_RSN_IE_LEN];
};
#endif
#endif

struct ss_res {
	unsigned int		ss_channel;
	unsigned int		count;
	struct bss_desc		bss[MAX_BSS_NUM];
	unsigned int		count_backup;
	struct bss_desc		bss_backup[MAX_BSS_NUM];
	unsigned int		count_target;
	struct bss_desc		bss_target[MAX_BSS_NUM];
#ifdef WIFI_SIMPLE_CONFIG
	struct wps_ie_info	ie[MAX_BSS_NUM];
	struct wps_ie_info	ie_backup[MAX_BSS_NUM];
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
	//unsigned char		bk_nwtype;
	//unsigned int		bk_iqc[11];
#endif
#ifdef WIFI_WPAS
	struct wpa_ie_info	wpa_ie[MAX_BSS_NUM];
	struct wpa_ie_info	wpa_ie_backup[MAX_BSS_NUM];
	struct rsn_ie_info	rsn_ie[MAX_BSS_NUM];
	struct rsn_ie_info	rsn_ie_backup[MAX_BSS_NUM];
#endif
#ifdef CONFIG_RTL_NEW_AUTOCH
	unsigned int		to_scan_40M;
#endif
	unsigned int		hidden_ap_found;
};

#if defined(CONFIG_RTL_NEW_AUTOCH) && defined(SS_CH_LOAD_PROC)

struct ss_report{
	unsigned char	channel;
	unsigned char	is40M;
	unsigned char	rssi;
	unsigned short	fa_count;
	unsigned short	cca_count;
	unsigned int	ch_load;
};

#endif


//struct stat_info_cache {
//	struct stat_info	*pstat;
//	unsigned char		hwaddr[6];
//};

struct mc2u_flood_mac_entry 
{
	unsigned char macAddr[MACADDRLEN];
};

struct rf_finetune_var {
	unsigned char		ofdm_1ss_oneAnt;// for 2T2R
	unsigned char		pathB_1T; // for 1T2R, 1T1R
	unsigned char		rssi_dump;
	unsigned char		rxfifoO;
	unsigned char		raGoDownUpper;
	unsigned char		raGoDown20MLower;
	unsigned char		raGoDown40MLower;
	unsigned char		raGoUpUpper;
	unsigned char		raGoUp20MLower;
	unsigned char		raGoUp40MLower;
	unsigned char		dig_enable;
	unsigned char		digGoLowerLevel;
	unsigned char		digGoUpperLevel;
	unsigned char		dcThUpper;
	unsigned char		dcThLower;
	unsigned char		rssiTx20MUpper;
	unsigned char		rssiTx20MLower;
	unsigned char		rssi_expire_to;
#ifdef INTERFERENCE_CONTROL
	unsigned char		nbi_filter_enable;
#endif
	unsigned char		rts_init_rate;

	unsigned char		cck_pwr_max;
	unsigned char		cck_tx_pathB;
	unsigned char           min_pwr_idex;

	// dynamic Tx power control
	unsigned char		tx_pwr_ctrl;

	// 11n ap AES debug
	unsigned char		aes_check_th;

	// Tx power tracking
	unsigned int		tpt_period;

	// TXOP enlarge
	unsigned char		txop_enlarge_upper;
	unsigned char		txop_enlarge_lower;
#ifdef LOW_TP_TXOP
	unsigned char		low_tp_txop;
	unsigned int		low_tp_txop_thd_n;
	unsigned int		low_tp_txop_thd_g;
	unsigned int		low_tp_txop_thd_low;
	unsigned char		low_tp_txop_close;
	unsigned char		low_tp_txop_count;
	unsigned char		low_tp_txop_delay;
	unsigned int		cwmax_enhance_thd;
#endif

	// 2.3G support
	unsigned char		use_frq_2_3G;

	// for mp test
#ifdef MP_TEST
	unsigned char		mp_specific;
#endif

	//Support IP multicast->unicast
#ifdef SUPPORT_TX_MCAST2UNI
	unsigned char		mc2u_disable;
#ifdef IGMP_FILTER_CMO
	unsigned char		igmp_deny;
#endif
	unsigned char 		mc2u_drop_unknown;
	unsigned int		mc2u_flood_ctrl;	
	struct mc2u_flood_mac_entry mc2u_flood_mac[MAX_FLOODING_MAC_NUM];
	unsigned int		mc2u_flood_mac_num;

#endif

#ifdef	HIGH_POWER_EXT_PA
	unsigned char		hp_ofdm_pwr_max;
	unsigned char		hp_cck_pwr_max;
#endif

#ifdef PCIE_POWER_SAVING
	unsigned char		power_save;
#endif
	unsigned char		use_ext_pa;
	unsigned char		use_ext_lna;
	unsigned char           NDSi_support;
	unsigned char		edcca_thd;
#ifdef ADD_TX_POWER_BY_CMD
	char		txPowerPlus_cck_1;
	char		txPowerPlus_cck_2;
	char		txPowerPlus_cck_5;
	char		txPowerPlus_cck_11;
	char		txPowerPlus_ofdm_6;
	char		txPowerPlus_ofdm_9;
	char		txPowerPlus_ofdm_12;
	char		txPowerPlus_ofdm_18;
	char		txPowerPlus_ofdm_24;
	char		txPowerPlus_ofdm_36;
	char		txPowerPlus_ofdm_48;
	char		txPowerPlus_ofdm_54;
	char		txPowerPlus_mcs_0;
	char		txPowerPlus_mcs_1;
	char		txPowerPlus_mcs_2;
	char		txPowerPlus_mcs_3;
	char		txPowerPlus_mcs_4;
	char		txPowerPlus_mcs_5;
	char		txPowerPlus_mcs_6;
	char		txPowerPlus_mcs_7;
	char		txPowerPlus_mcs_8;
	char		txPowerPlus_mcs_9;
	char		txPowerPlus_mcs_10;
	char		txPowerPlus_mcs_11;
	char		txPowerPlus_mcs_12;
	char		txPowerPlus_mcs_13;
	char		txPowerPlus_mcs_14;
	char		txPowerPlus_mcs_15;
#endif

	unsigned char		rootFwBeacon;		// use FW to send beacon
	unsigned char		ledBlinkingFreq;

	unsigned char		diffAmpduSz;
	unsigned char		one_path_cca;		// 0: 2-path, 1: path-A, 2: path-B
#ifdef DFS
	unsigned char		dfsdbgmode;
	unsigned char		dfsdelayiqk;
	unsigned int		dfsdbgcnt;
	unsigned long		dfsrctime;
#endif

#ifdef SW_TX_QUEUE
	int             	swq_en_highthd;
	int             	swq_dis_lowthd;
	int             	swq_enable;
	int             	swq_dbg;
	int					swq_aggnum;
	int					timeout_thd;
	int					timeout_thd2;
	int					timeout_thd3;
#endif

#ifdef A4_STA
	unsigned char		a4_enable;
#endif

#ifdef SW_ANT_SWITCH
	unsigned char		antSw_enable;
#endif

#ifdef HW_ANT_SWITCH
	unsigned char		antHw_enable;
	unsigned char		CurAntenna;
#endif
#if defined(HW_ANT_SWITCH) || defined(SW_ANT_SWITCH)
	unsigned char		antSw_select;
	unsigned char		ant_dump;
#endif

#ifdef RTL8192D_INT_PA
	unsigned char		use_intpa92d;
#endif

#if defined(CONFIG_RTL_NOISE_CONTROL) || defined(CONFIG_RTL_NOISE_CONTROL_92C)
	unsigned char		dnc_enable;
#endif

	unsigned char		pwr_by_rate;
#ifdef _TRACKING_TABLE_FILE
	unsigned char		pwr_track_file;
#endif
#ifdef DPK_92D
	unsigned char		dpk_on;
#endif

#if defined(TXPWR_LMT)
	unsigned char		disable_txpwrlmt;
#endif
#ifdef CONFIG_RTL_92D_DMDP
	unsigned char		peerReinit;
#endif
#ifdef WIFI_WMM
	unsigned char		wifi_beq_iot;
#endif
	unsigned char		bcast_to_dzq;
#ifdef TLN_STATS
	unsigned int		stats_time_interval;
#endif
#ifdef TX_EARLY_MODE
	unsigned char		em_enable;
#endif
#ifdef CLIENT_MODE
	unsigned char		sta_mode_ps;
#endif
#ifdef CONFIG_RTL_WLAN_DOS_FILTER
	unsigned int		dos_block_time;
#endif

	unsigned int		intel_rtylmt_tp_margin;
#ifdef CONFIG_RTL_88E_SUPPORT
	unsigned char		disable_pkt_pause;
	unsigned char		disable_pkt_nolink;
	unsigned char		max_pkt_fail;
	unsigned char		min_pkt_fail;	
#endif
	unsigned char		low_tp_no_aggr;	
#ifdef RTK_AC_SUPPORT  //vht rate 
//	unsigned char		dot11OperationalRateSet_AC[40]; //(MCS0 - MCS9) X (nss1 - nss4) = 10 * 4 = 40
//	unsigned int		dot11OperationalRateSetLen_AC; //??  useless
//	unsigned int		dot11SupportedRates_AC;	// 1-nss1, 2-nss2, 3-nss3, 4-nss4
//	unsigned int		dot11BasicRates_AC;		// 1-nss1, 2-nss2, 3-nss3, 4-nss4
	unsigned char		rxldpc;	
	unsigned char		txldpc;		
#endif
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A)
//	unsigned char		update_rainfo;
#ifdef FOR_VHT5G_PF2
	unsigned char		use_cca;
	unsigned char		cca_rts;
#endif
#endif
};

//Filen
//		MESH		WDS			Type		ACCESS_SWAP
//          [9:8]		[7:4]             [3:2]		[1:0]
//

/* Bit mask value of type */
#define ACCESS_SWAP_IO		0x01	/* Do bye-swap in access IO register */
#define ACCESS_SWAP_MEM		0x02	/* Do byte-swap in access memory space */

#define ACCESS_MASK			0x3
#define TYPE_SHIFT			2
#define TYPE_MASK			0x3

#ifdef WDS
#define WDS_SHIFT			4
#define WDS_MASK			0xf
#define WDS_NUM_CFG			NUM_WDS
#ifdef LAZY_WDS
#define WDS_LAZY_ENABLE			2
#endif

#else
#define WDS_SHIFT			0
#define WDS_NUM_CFG			0
#endif


enum {
	TYPE_EMBEDDED	= 0,	/* embedded wlan controller */
	TYPE_PCI_DIRECT	= 1,	/* PCI wlan controller and enable PCI bridge directly */
	TYPE_PCI_BIOS	= 2		/* PCI wlan controller and enable PCI by BIOS */
};


enum {
	DRV_STATE_INIT	 = 1,	/* driver has been init */
	DRV_STATE_OPEN	= 2,	/* driver is opened */
#ifdef UNIVERSAL_REPEATER
	DRV_STATE_VXD_INIT = 4,	/* vxd driver has been opened */
	DRV_STATE_VXD_AP_STARTED	= 8, /* vxd ap has been started */
#endif
};

#ifdef RX_BUFFER_GATHER
enum {
	GATHER_STATE_NO = 0,		/* no data pending */
	GATHER_STATE_FIRST = 1, 	/* first segment */
	GATHER_STATE_MIDDLE = 2, 	/* niether first nor last segment */
	GATHER_STATE_LAST = 3		/* last segment */
};
#endif

#ifdef CHECK_TX_HANGUP
#define PENDING_PERIOD		40	// max time of pending period


struct desc_check_info {
	int pending_tick;	// tick value when pending is detected
	int pending_tail;	// descriptor tail number when pending is detected
	int idle_tick;		// tick value when detect idle (tx desc is not free)
};
#endif


// MAC access control log definition
#define MAX_AC_LOG		32	// max number of log entry
#define AC_LOG_TIME		300	// log time in sec
#define AC_LOG_EXPIRE	RTL_SECONDS_TO_JIFFIES(3)	// entry expire time

struct ac_log_info {
	int	used;		// <>0: in use
	int	cur_cnt;	// current attack counter
	int	last_cnt;	// counter of last time log
	unsigned long last_attack_time; // jiffies time of last attack
	unsigned char addr[MACADDRLEN];	// mac address
};

#ifdef WIFI_SIMPLE_CONFIG
struct wsc_probe_request_info {
	unsigned char addr[MACADDRLEN]; // mac address
	char ProbeIE[PROBEIELEN];
	unsigned short ProbeIELen;
	unsigned long time_stamp; // jiffies time of last probe request
	unsigned char used;
	unsigned char pbcactived;			/* 0528pbc */
};
#endif

struct reorder_ctrl_timer
{
	struct rtl8192cd_priv	*priv;
	struct stat_info	*pstat;
	unsigned char		tid;
	unsigned int		timeout;
};

#ifdef RTK_QUE
struct ring_que {
	int qlen;
	int qmax;
	int head;
	int tail;
	struct sk_buff *ring[MAX_PRE_ALLOC_SKB_NUM+1];
};
#endif

#ifdef USE_TXQUEUE
struct txq_node {
	struct list_head list;
	struct sk_buff *skb;
	struct net_device *dev;
};
struct txq_list_head {
	struct list_head list;
	unsigned int qlen;
};

#define init_txq_head(_q) \
	do { \
		INIT_LIST_HEAD(&((_q)->list)); \
		(_q)->qlen = 0; \
	}while(0)
#define is_txq_empty(_q) ((_q)->qlen ? 0 : 1)
#define txq_len(_q) ((_q)->qlen)
#define add_txq_tail(_q, _n) \
	do { \
		list_add_tail(&((_n)->list), &((_q)->list)); \
		(_q)->qlen ++; \
	}while(0)
#define add_txq_head(_q, _n) \
	do { \
		list_add(&((_n)->list), &((_q)->list)); \
		(_q)->qlen ++; \
	}while(0)
#define unlink_txq(_q, _n) \
	do { \
		list_del(&((_n)->list)); \
		(_q)->qlen --; \
	}while(0)
#define deq_txq(_q) \
	( (_q)->qlen ? (struct txq_node *)((_q)->list.next) : NULL ); \
	unlink_txq(_q, (struct txq_node *)((_q)->list.next));
#endif

#ifdef A4_STA
#define A4_STA_HASH_BITS		3
#define A4_STA_HASH_SIZE		(1 << A4_STA_HASH_BITS)
#define A4_STA_AGEING_TIME	300
#define MAX_A4_TBL_NUM			32

struct a4_sta_db_entry {
	struct a4_sta_db_entry	*next_hash;
	struct a4_sta_db_entry	**pprev_hash;
	unsigned char			mac[6];
	unsigned long			ageing_timer;
	struct stat_info		*stat;
};

struct a4_tbl_entry {
	int used;
	struct a4_sta_db_entry	entry;
};
#endif

#ifdef SW_ANT_SWITCH

// 20100503 Joseph:
// Add new function SwAntDivCheck8192C().
// This is the main function of Antenna diversity function before link.
// Mainly, it just retains last scan result and scan again.
// After that, it compares the scan result to see which one gets better RSSI.
// It selects antenna with better receiving power and returns better scan result.
//
#define	TP_MODE			0
#define	RSSI_MODE		1
#define	TRAFFIC_LOW		0
#define	TRAFFIC_HIGH	1
#define	TRAFFIC_THRESHOLD	 6600000	// 16 Mbps
#define	TP_MODE_THD			  200000	//500 kbps

#define SELANT_MAP_SIZE	8

// 20100514 Joseph: Add definition for antenna switching test after link.
// This indicates two different the steps.
// In SWAW_STEP_PEAK, driver needs to switch antenna and listen to the signal on the air.
// In SWAW_STEP_DETERMINE, driver just compares the signal captured in SWAW_STEP_PEAK
// with original RSSI to determine if it is necessary to switch antenna.
#define SWAW_STEP_PEAK		0
#define SWAW_STEP_DETERMINE	1
#define SWAW_STEP_RESET		0xff

typedef struct _SW_Antenna_Switch_
{
	unsigned char 		try_flag;
	unsigned char		CurAntenna;
	unsigned char		RSSI_Trying;
	unsigned char		TestMode;
	int					SelectAntennaMap[2][SELANT_MAP_SIZE];
	unsigned char 		mapIndex;
}SWAT_T;

#endif

#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
#if defined(SW_ANT_SWITCH) || defined(HW_ANT_SWITCH)

typedef enum tag_SW_Antenna_Switch_Definition
{
	Antenna_L = 1,
	Antenna_R = 2,
	Antenna_MAX = 3,
}DM_SWAS_E;
#endif
#endif


#if defined(CONFIG_RTL_88E_SUPPORT) && defined(TXREPORT)
typedef struct StationInfoRAStruct
{
	unsigned char RateID;	// old rate id, by every conn per station
	unsigned int RateMask;	// old rate mask
	unsigned int RAUseRate;
	unsigned char RateSGI;	// use SGI by decision
	unsigned char RssiStaRA;
	unsigned char PreRssiStaRA;
	unsigned char SGIEnable;	// set if station support SGI, by every conn per station
	unsigned char DecisionRate;	// update txrate info for desc setting
	unsigned char PreRate;
	unsigned char HighestRate;
	unsigned char LowestRate;
	unsigned int NscUp;
	unsigned int NscDown;
	unsigned short RTY[5];
	unsigned int TOTAL;
	unsigned short DROP;
	unsigned char Active;
	unsigned short RptTime;
#if 1
	unsigned char RAWaitingCounter;
	unsigned char RAPendingCounter;
#endif
#if 0
	unsigned char TryingState;
	unsigned char RateBeforeTrying;
#endif
	struct stat_info *pstat;
} STATION_RA_INFO,*PSTATION_RA_INFO;
#endif


#if defined(CONFIG_RTL_88E_SUPPORT) && !defined(CALIBRATE_BY_ODM) //for 88e tx power tracking

#define IQK_Matrix_REG_NUM	8

typedef struct _IQK_MATRIX_REGS_SETTING{
	char 	bIQKDone;
	int		Value[1][IQK_Matrix_REG_NUM];
}IQK_MATRIX_REGS_SETTING,*PIQK_MATRIX_REGS_SETTING;

#endif



// common private structure which info are shared between root interface and virtual interface
struct priv_shared_info {
	unsigned int			type;
	unsigned int			ioaddr;
	unsigned int			version_id;
#ifdef IO_MAPPING
	unsigned int			io_mapping;
#endif

#ifdef USE_CHAR_DEV
	struct rtl8192cd_chr_priv	*chr_priv;
#endif
#ifdef USE_PID_NOTIFY
	pid_t					wlanapp_pid;
#endif

#ifdef CONFIG_RTL_WAPI_SUPPORT
	pid_t					wlanwapi_pid;
#endif

#ifdef CONFIG_NET_PCI
	struct pci_dev			*pdev;
#endif

#if	defined(CONCURRENT_MODE) || defined(CONFIG_RTL_92D_SUPPORT)
	int						wlandev_idx;
#endif

#ifdef __KERNEL__
	struct tasklet_struct	rx_tasklet;
	struct tasklet_struct	tx_tasklet;
	struct tasklet_struct	oneSec_tasklet;
#ifdef PCIE_POWER_SAVING
	struct tasklet_struct	ps_tasklet;
	unsigned int rf_phy_bb_backup[23];
#endif
#endif

	struct wlan_hdr_poll	*pwlan_hdr_poll;
	struct list_head		wlan_hdrlist;

	struct wlanllc_hdr_poll	*pwlanllc_hdr_poll;
	struct list_head		wlanllc_hdrlist;

	struct wlanbuf_poll		*pwlanbuf_poll;
	struct list_head		wlanbuf_list;

	struct wlanicv_poll		*pwlanicv_poll;
	struct list_head		wlanicv_list;

	struct wlanmic_poll		*pwlanmic_poll;
	struct list_head		wlanmic_list;

	struct rtl8192cd_hw		*phw;
	struct rtl8192cd_tx_desc_info 	*pdesc_info;
	unsigned int			have_hw_mic;

	struct aid_obj			*aidarray[NUM_STAT];
	unsigned int			max_fw_macid;

#ifdef _11s_TEST_MODE_
	struct Galileo_poll 	*galileo_poll;
	struct list_head		galileo_list;
#endif

	unsigned char			agc_tab_buf[AGC_TAB_SIZE];
	unsigned char			mac_reg_buf[MAC_REG_SIZE];
	unsigned char			phy_reg_buf[PHY_REG_SIZE];

#ifdef MP_TEST
	unsigned char			phy_reg_mp_buf[PHY_REG_SIZE];
#endif

	unsigned char			phy_reg_pg_buf[PHY_REG_PG_SIZE];

#ifdef TXPWR_LMT
	unsigned char			txpwr_lmt_buf[MAC_REG_SIZE];
	unsigned int			txpwr_lmt_CCK;
	unsigned int			txpwr_lmt_OFDM;
	unsigned int			txpwr_lmt_HT1S;
	unsigned int			txpwr_lmt_HT2S;
	unsigned char			ch_pwr_lmtCCK[SUPPORT_CH_NUM];
	unsigned char			ch_pwr_lmtOFDM[SUPPORT_CH_NUM];
	unsigned char			ch_pwr_lmtHT20_1S[SUPPORT_CH_NUM];
	unsigned char			ch_pwr_lmtHT20_2S[SUPPORT_CH_NUM];
	unsigned char			ch_pwr_lmtHT40_1S[SUPPORT_CH_NUM];
	unsigned char			ch_pwr_lmtHT40_2S[SUPPORT_CH_NUM];
	unsigned char			ch_tgpwr_CCK[SUPPORT_CH_NUM];
	unsigned char			ch_tgpwr_OFDM[SUPPORT_CH_NUM];
	unsigned char			ch_tgpwr_HT20_1S[SUPPORT_CH_NUM];
	unsigned char			ch_tgpwr_HT20_2S[SUPPORT_CH_NUM];
	unsigned char			ch_tgpwr_HT40_1S[SUPPORT_CH_NUM];
	unsigned char			ch_tgpwr_HT40_2S[SUPPORT_CH_NUM];
	unsigned int			tgpwr_CCK;
	unsigned int			tgpwr_OFDM;
	unsigned int			tgpwr_HT1S;
	unsigned int			tgpwr_HT2S;
#endif

#ifdef _TRACKING_TABLE_FILE
	unsigned char			txpwr_tracking_2G_CCK[4][index_mapping_NUM_MAX];
	unsigned char			txpwr_tracking_2G_OFDM[4][index_mapping_NUM_MAX];
	unsigned char			txpwr_tracking_5GL[4][index_mapping_NUM_MAX];
	unsigned char			txpwr_tracking_5GM[4][index_mapping_NUM_MAX];
	unsigned char			txpwr_tracking_5GH[4][index_mapping_NUM_MAX];
#endif

//	unsigned char			phy_reg_2to1[PHY_REG_1T2R];
	unsigned short			fw_IMEM_len;
	unsigned short			fw_EMEM_len;
	unsigned short			fw_DMEM_len;

#ifdef __KERNEL__
	spinlock_t				lock;
#endif

#ifdef SMP_SYNC
	spinlock_t				lock_xmit;
	spinlock_t				lock_skb;
	spinlock_t				lock_buf;
	spinlock_t				lock_recv;
#endif

	// for RF fine tune
	struct rf_finetune_var	rf_ft_var;

	// bcm 11n chipset iot debug
	unsigned int			reg_810_state;	// 0=default, 1=set for bcm

	unsigned int			skip_mic_chk;
	// TXOP enlarge
	unsigned int			txop_enlarge;	// 0=no txop, 1=half txop enlarged, 2=full txop enlarged, e=for Intel, f=for 8192S sta

	struct stat_info		*highTP_found_pstat;
	struct stat_info                *txpause_pstat;
	unsigned long                   txpause_time;
	unsigned char			rssi_min;
#ifdef WIFI_WMM
	unsigned char			iot_mode_enable;
	unsigned int			iot_mode_VI_exist;
	unsigned int			iot_mode_VO_exist;	
	unsigned char			wifi_beq_lower;
#ifdef WMM_VIBE_PRI
	unsigned int            iot_mode_BE_exist;
#endif
#ifdef WMM_BEBK_PRI
	unsigned int            iot_mode_BK_exist;
#endif
#endif

#ifdef LOW_TP_TXOP
	unsigned char			BE_cwmax_enhance;
#endif

#ifdef RTK_AC_SUPPORT  
	char					txsc_20;    // { 1, 2, 3, 4 }
	char					txsc_40;    // { 9, 10 }
#endif

	int						is_40m_bw;
	int						is_40m_bw_bak;
	char						offset_2nd_chan;
//	int						is_giga_exist;

#ifdef CONFIG_RTK_MESH
	struct MESH_Share		meshare; 	// mesh share data
	spinlock_t				lock_queue;	// lock for DOT11_EnQueue2/DOT11_DeQueue2
	spinlock_t				lock_Rreq;	// lock for rreq_retry. Some function like aodv_expire/tx use lock_queue simultaneously
#endif

	unsigned int			curr_band;				// remember the current band to save switching time
	unsigned short		fw_signature;
	unsigned char			fw_category;
	unsigned char			fw_function;
	unsigned short			fw_version;
//	unsigned short		fw_src_version;
	unsigned short			fw_sub_version;
	unsigned char			fw_date_month;
	unsigned char			fw_date_day;
	unsigned char			fw_date_hour;
	unsigned char			fw_date_minute;
	unsigned int			CamEntryOccupied;		// how many entries in CAM?

	unsigned char			rtk_ie_buf[16];
	unsigned int			rtk_ie_len;
	unsigned char			*rtk_cap_ptr;
	unsigned short			rtk_cap_val;
	unsigned char			use_long_slottime;

	// for Tx power control
	unsigned char			working_channel;
	unsigned char			ra40MLowerMinus;
	unsigned char			raThdHP_Minus;

#ifdef	INCLUDE_WPS
	struct wsc_context		WSC_CONT_S;
#endif

	unsigned int			current_num_tx_desc;

#ifdef USE_OUT_SRC
	DM_ODM_T				_dmODM;
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
#ifdef SUPPORT_RTL8188E_TC
	unsigned int			rtl8188e_testchip_checked;
#endif
#if defined(TXREPORT) && !defined(RATEADAPTIVE_BY_ODM)
	STATION_RA_INFO  RaInfo[RTL8188E_NUM_STAT];
#endif
#endif

#if defined(USE_OUT_SRC) 
		unsigned char		use_outsrc;
#endif

		unsigned char		use_hal;

#ifdef DFS
	unsigned int			dfsSwitchChannel;
#endif

	/*********************************************************
	 * from here on, data will be clear in rtl8192cd_init_sw() *
	 *********************************************************/

	// for SW LED
	struct timer_list		LED_Timer;
	unsigned int			LED_Interval;
	unsigned char			LED_Toggle;
	unsigned char			LED_ToggleStart;
	unsigned int			LED_tx_cnt_log;
	unsigned int			LED_rx_cnt_log;
	unsigned int			LED_tx_cnt;
	unsigned int			LED_rx_cnt;
	unsigned char			LED_cnt_mgn_pkt;

	//for TxPwrTracking

	unsigned char			Thermal_idx;
	unsigned char			Thermal_log[8];

	unsigned char			ThermalValue;
	unsigned char			ThermalValue_LCK;
	unsigned char			ThermalValue_IQK;
	unsigned char			CCK_index;
	unsigned char			OFDM_index[2];
	unsigned char			CCK_index0;
	unsigned char			OFDM_index0[2];
#ifdef CONFIG_RTL_92D_SUPPORT
	unsigned char			Delta_IQK;
	unsigned char			Delta_LCK;
	unsigned char			pwr_trk_ongoing;
#ifdef RX_GAIN_TRACK_92D
	unsigned int			RegRF3C[2];	//pathA / pathB
	unsigned char			ThermalValue_RxGain;
#endif
	unsigned char			ThermalValue_Crystal;
#ifdef DPK_92D
	unsigned char			ThermalValue_DPK;
	unsigned char			ThermalValue_DPKstore;
	unsigned char			ThermalValue_DPKtrack;
	unsigned char			bDPKworking;
	struct timer_list		DPKTimer;
	unsigned char			bDPKdone[2];
	unsigned char			bDPKstore;
	short					index_mapping_DPK_current[4][index_mapping_DPK_NUM];
	unsigned char			OFDM_min_index_internalPA_DPK[2];
	unsigned char			TxPowerLevelDPK[2];
#endif
	unsigned int			RegRF18[2];
	unsigned int			RegRF28[2];
#endif // CONFIG_RTL_92D_SUPPORT

#if  defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A)
	unsigned char 			No_RF_Write;
	unsigned char			No_BB_Write;
#endif


	unsigned int			RegE94;
	unsigned int			RegE9C;
	unsigned int			RegEB4;
	unsigned int			RegEBC;

	//for IQK
	unsigned int			RegC04;
	unsigned int			Reg874;
	unsigned int			RegC08;
	unsigned int			ADDA_backup[IQK_ADDA_REG_NUM];
	unsigned int			IQK_MAC_backup[IQK_MAC_REG_NUM];

#ifdef CONFIG_RTL_88E_SUPPORT
	//for 8188E IQK
	unsigned int			IQK_BB_backup[IQK_BB_REG_NUM];
	unsigned int 			IQK_BB_backup_recover[IQK_BB_REG_NUM];
	unsigned char			bRfPiEnable;
	unsigned char			IQK_88E_done;
#endif 	

#ifdef CONFIG_RTL_88E_SUPPORT //for 88e tx power tracking

	unsigned char	Power_tracking_on_88E;

	unsigned int 	TXPowerTrackingCallbackCnt; //cosa add for debug
	unsigned int	RegA24; // for TempCCK
	unsigned char	EEPROMThermalMeter;			// EEPROM default ThermalMeter value.

	//u1Byte bTXPowerTracking;
	unsigned char	TXPowercount;
	char 			bTXPowerTrackingInit; 
	char 			bTXPowerTracking;
	unsigned char	TxPowerTrackControl; //for mp mode, turn off txpwrtracking as default
	unsigned char	TM_Trigger;
	unsigned char	InternalPA5G[2];	//pathA / pathB

	unsigned char	ThermalMeter[2];	// ThermalMeter, index 0 for RFIC0, and 1 for RFIC1	
	unsigned char	ThermalValue_AVG[AVG_THERMAL_NUM];
	unsigned char	ThermalValue_AVG_index; 	
	unsigned char	ThermalValue_RxGain;
	unsigned char	ThermalValue_Crystal;
	unsigned char	ThermalValue_DPKstore;
	unsigned char	ThermalValue_DPKtrack;
	char 			TxPowerTrackingInProgress;
	char 			bDPKenable;

	char 			bReloadtxpowerindex;	
	//unsigned char	bRfPiEnable;
	//unsigned int	TXPowerTrackingCallbackCnt; //cosa add for debug

	unsigned char	bCCKinCH14;
	char 			bDoneTxpower;
		
	unsigned char	ThermalValue_HP[HP_THERMAL_NUM];
	unsigned char	ThermalValue_HP_index;
	IQK_MATRIX_REGS_SETTING IQKMatrixRegSetting[IQK_Matrix_Settings_NUM];

	unsigned char	Delta_IQK;
	unsigned char	Delta_LCK;

#endif

#ifdef CONFIG_RTL_8812_SUPPORT //for 8812 tx power tracking
	unsigned char	Power_tracking_on_8812;
	unsigned char	ThermalValue_AVG_8812[AVG_THERMAL_NUM_8812];
	unsigned char	ThermalValue_AVG_index_8812; 	
#endif

#if 0 //def SMART_CONCURRENT_92D
	unsigned int			bcnCount;
#endif

//#ifdef MBSSID
	struct rtl8192cd_priv	*bcnDOk_priv;
//#endif

#if defined(TXREPORT) && (defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)) || defined(USE_OUT_SRC)
		int sta_query_idx;
#endif

	// for Rx dynamic tasklet
	unsigned int			rxInt_useTsklt;
	unsigned int			rxInt_data_delta;

#ifdef CHECK_HANGUP
#ifdef CHECK_TX_HANGUP
	struct desc_check_info	Q_info[5];
#endif
#ifdef CHECK_RX_DMA_ERROR
	unsigned short			rxff_pkt;
	unsigned short			rxff_rdptr;
	unsigned int			rx_byte_cnt;
#endif
#ifdef CHECK_RX_HANGUP
	unsigned int			rx_hang_checking;
	unsigned int			rx_cntreg_log;
	unsigned int			rx_stop_pending_tick;
	struct rtl8192cd_priv		*selected_priv;
#endif
#ifdef CHECK_BEACON_HANGUP
	unsigned int			beacon_ok_cnt;
	unsigned int			beacon_pending_cnt;
	unsigned int			beacon_wait_cnt;
#endif
	unsigned int			reset_monitor_cnt_down;
	unsigned int			reset_monitor_pending;
	unsigned int			reset_monitor_rx_pkt_cnt;
#endif

#ifdef MP_TEST
	unsigned char			mp_datarate;
	unsigned char			mp_antenna_tx;
	unsigned char			mp_antenna_rx;
	unsigned char			mp_txpwr_patha;
	unsigned char			mp_txpwr_pathb;
	unsigned char			mp_cck_txpwr_odd;

	void 					*skb_pool_ptr;
	struct sk_buff 			*skb_pool[NUM_MP_SKB];
	int						skb_head;
	int						skb_tail;

	unsigned int			mp_ctx_count;
	unsigned int			mp_ctx_pkt;
	unsigned char			mp_rssi;
	unsigned char			mp_sq;
	struct rf_misc_info		mp_rf_info;

	unsigned char			mp_ofdm_swing_idx;
	unsigned char			mp_cck_swing_idx;
	unsigned char			mp_txpwr_tracking;
	int					mp_pkt_len;

#ifdef MP_SWITCH_LNA
	unsigned char			rx_packet_ss_a;
	unsigned char			rx_packet_ss_b;
#endif

#ifdef B2B_TEST
	volatile unsigned long	mp_rx_ok, mp_rx_sequence, mp_rx_lost_packet, mp_rx_dup;
	volatile unsigned short	mp_cached_seq;
	int 					mp_rx_waiting;
	volatile unsigned int	mp_mac_changed;
	unsigned long			txrx_elapsed_time;
	unsigned long			txrx_start_time;
#endif
//arx parameters
	unsigned int			mp_filter_flag;
	unsigned char 		mp_filter_DA[6];
	unsigned char 		mp_filter_SA[6];
	unsigned char 		mp_filter_BSSID[6];

#endif // MP_TEST

	// monitor Tx and Rx
	unsigned long			tx_packets_pre;
	unsigned long			rx_packets_pre;

	// bcm old 11n chipset iot debug, and TXOP enlarge
	unsigned long			current_tx_bytes;
	unsigned long			current_rx_bytes;

#ifdef USE_OUT_SRC
	u8Byte					NumTxBytesUnicast;
	u8Byte					NumRxBytesUnicast;
	unsigned char			bScanInProcess;
	u8Byte					dummy;
#endif	

	unsigned char			CurrentChannelBW;
	unsigned char			*txcmd_buf;
	unsigned long			cmdbuf_phyaddr;
	unsigned long			InterruptMask;
	unsigned long			InterruptMaskExt;
	unsigned int			rx_rpt_ofdm;
	unsigned int			rx_rpt_cck;
	unsigned int			rx_rpt_ht;
	unsigned int			successive_bb_hang;
#ifdef CLIENT_MODE
	int					AP_BW;
#endif

	unsigned long			rxFiFoO_pre;
//	unsigned int			pkt_in_hiQ;

#ifdef RTK_QUE
	struct ring_que 		skb_queue;
#else
	struct sk_buff_head		skb_queue;
#endif

	struct timer_list			rc_sys_timer;
	struct reorder_ctrl_timer	rc_timer[64];
	unsigned short				rc_timer_head;
	unsigned short				rc_timer_tail;
	unsigned short				rc_timer_tick;

	struct reorder_ctrl_timer	amsdu_timer[64];
	unsigned short				amsdu_timer_head;
	unsigned short				amsdu_timer_tail;

	// ht associated client statistic
#ifdef WIFI_WMM
	unsigned int			ht_sta_num;
	unsigned int			mimo_ps_dynamic_sta;
#ifdef CONFIG_RTL_88E_SUPPORT
	unsigned int			mimo_ps_dynamic_sta_88e_hw_ext;
#endif
#ifdef STA_EXT
	unsigned int			mimo_ps_dynamic_sta_ext;
#endif
#endif

	unsigned int			set_led_in_progress;

	struct stat_info*		CurPstat[4]; // for tx desc break field
//	unsigned int			has_2r_sta; // Used when AP is 2T2R. bitmap of 2R aid
	int						has_triggered_rx_tasklet;
	int						has_triggered_tx_tasklet;
#ifdef __ECOS
	int						has_triggered_reorder_ctrl_timeout;
	int						has_triggered_process_mcast_dzqueue;
#if defined(TXREPORT) && (defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT))
	int						has_triggered_C2H_isr;
#endif
#endif

#ifdef STA_EXT
	unsigned char			fw_free_space;
	unsigned char			remapped_aidarray[NUM_STAT];
#endif

	// for DIG
	unsigned char			DIG_on;
	unsigned char			restore;
	unsigned short			threshold0;
	unsigned short			threshold1;
	unsigned short			threshold2;
	unsigned char			FA_upper;
	unsigned char			FA_lower;
	unsigned short			ofdm_FA_cnt1;
	unsigned short			ofdm_FA_cnt2;
	unsigned short			ofdm_FA_cnt3;
	unsigned short			ofdm_FA_cnt4;
	unsigned short			cck_FA_cnt;
#ifdef INTERFERENCE_CONTROL
	unsigned int			ofdm_FA_total_cnt;
#endif
	unsigned int			FA_total_cnt;
	unsigned int			CCA_total_cnt;

#if defined(CONFIG_RTL_NOISE_CONTROL) || defined(CONFIG_RTL_NOISE_CONTROL_92C)
	unsigned char			DNC_on;
#endif

#if defined(CONFIG_RTL_92D_SUPPORT) && defined(CONFIG_RTL_NOISE_CONTROL)
	unsigned char			DNC_chk_cnt;
	unsigned char			DNC_chk;
//	unsigned short			F90_cnt;
	unsigned short			F94_cnt;
	unsigned short			F94_cntOK;
	unsigned int			Reg664_cnt;
	unsigned int			Reg664_cntOK;
#endif

	int 					digDownCount;
	int 					digDeadPoint;
	int 					digDeadPointCandidate;
	int 					digDeadPointHitCount;

	unsigned char			cca_path;
	unsigned char			fw_q_fifo_count;

	unsigned int			current_tx_rate;
	unsigned char			ht_current_tx_info;	// bit0: 0=20M mode, 1=40M mode; bit1: 0=longGI, 1=shortGI

#ifdef DOT11D
	unsigned int			countryTabIdx;
	unsigned int			countryBandUsed;	// 0 == 2.4G ; 1 == 5G
#endif


#if defined(RESERVE_TXDESC_FOR_EACH_IF) && (defined(UNIVERSAL_REPEATER) || defined(MBSSID))
	unsigned int			num_txdesc_cnt;		// less or equal max available tx desc
	unsigned int			num_txdesc_upper_limit;
	unsigned int			num_txdesc_lower_limit;
#ifdef USE_TXQUEUE
	unsigned int			num_txq_cnt;
	unsigned int			num_txq_upper_limit;
	unsigned int			num_txq_lower_limit;
#endif
#endif
#ifdef __ECOS
	unsigned int			isr_status, isr_status_ext;
#endif

	// Retry Limit register content
	unsigned char				RLShort;		// Short Retry Limit
	unsigned char				RLLong;		// Long Retry Limit

#ifdef SW_ANT_SWITCH
	SWAT_T				DM_SWAT_Table;
	struct	stat_info  *RSSI_target;
	struct timer_list	swAntennaSwitchTimer;

	unsigned int		RSSI_sum_R;
	unsigned int		RSSI_cnt_R;
	unsigned int		RSSI_sum_L;
	unsigned int		RSSI_cnt_L;
	unsigned int 		RSSI_test;
 	unsigned int 		lastTxOkCnt;
	unsigned int 		lastRxOkCnt;
 	unsigned int 		TXByteCnt_R;
	unsigned int 		TXByteCnt_L;
	unsigned int 		RXByteCnt_R;
	unsigned int 		RXByteCnt_L;
 	unsigned int 		TrafficLoad;
#endif

#ifdef DETECT_STA_EXISTANCE
	struct timer_list			rl_recover_timer;
	unsigned char				bRLShortened;
#endif

#ifdef DFS
	unsigned int			dfsSwitchChCountDown;
	unsigned int			dfsSwCh_ongoing;
#endif

#ifdef RX_BUFFER_GATHER
	unsigned int 			gather_state;
	struct list_head		gather_list;
	int					gather_len;
#ifdef CONFIG_WLAN_HAL
    int                     pkt_total_len;
#endif // CONFIG_WLAN_HAL
#endif

#ifdef USE_TXQUEUE
	struct txq_list_head	txq_list[7];
	unsigned int			txq_isr;
	unsigned int			txq_stop;
	unsigned int			txq_check;
	struct list_head		txq_pool;
	unsigned char			*txq_pool_addr;
#endif

	unsigned char  	Reg_RRSR_2;
	unsigned char  	Reg_81b;
	unsigned int 	marvellMapBit;
#ifdef CONFIG_RTL_88E_SUPPORT
	unsigned int 	marvellMapBit_88e_hw_ext;
#endif
#ifdef STA_EXT
	unsigned int 	marvellMapBitExt;
#endif

#ifdef TX_EARLY_MODE
	unsigned int	em_waitq_on;
	unsigned int	em_tx_byte_cnt;
	unsigned int	reach_tx_limit_cnt;
#ifdef CONFIG_RTL_88E_SUPPORT
	unsigned short  aggrmax_bak;
#endif	
#endif

	unsigned int	iqk_2g_done;	
#ifdef CONFIG_RTL_92D_SUPPORT	
	unsigned int	iqk_5g_done;
#endif	

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A)
	unsigned char 	pre_channel;
#endif

	unsigned int	intel_active_sta;
	unsigned int	intel_rty_lmt;

#if defined(CONFIG_RTL_88E_SUPPORT) && defined(TXREPORT)
	unsigned int total_assoc_num;
	unsigned int txRptMacid;
#endif

#ifdef SMART_REPEATER_MODE
	struct timer_list	check_vxd_ap;
	unsigned int		switch_chan_rp;
	unsigned int		switch_2ndchoff_rp;
	unsigned int		band_width_rp;
#endif	
};

#ifdef CONFIG_RTL8186_KB
typedef struct guestmac {
	unsigned char			macaddr[6];
	unsigned char			valid;
} GUESTMAC_T;
#endif


typedef struct rtl8192cd_priv {

#ifdef NETDEV_NO_PRIV
	struct rtl8192cd_priv*	wlan_priv;		//This element shall be put at top of this struct
#endif

#ifdef CONFIG_WLAN_HAL
    void *                           HalFunc;
    void *                           HalData;
#endif
	int						drv_state;		// bit0 - init, bit1 - open/close
	struct net_device		*dev;
	struct wifi_mib 		*pmib;

	struct wlan_acl_poll	*pwlan_acl_poll;
	struct list_head		wlan_aclpolllist;	// this is for poll management
	struct list_head		wlan_acl_list;		// this is for auth checking

	DOT11_QUEUE				*pevent_queue;
#ifdef CONFIG_RTL_WAPI_SUPPORT
	DOT11_QUEUE				*wapiEvent_queue;
#endif
	DOT11_EAP_PACKET		*Eap_packet;
#ifdef INCLUDE_WPS
#ifndef CONFIG_MSC
	DOT11_EAP_PACKET		*upnp_packet;
#endif
#endif

#ifdef _INCLUDE_PROC_FS_
	struct proc_dir_entry	*proc_root;
	unsigned int			txdesc_num;
	unsigned char			*phypara_file_start;
	unsigned char			*phypara_file_end;
#endif

#ifdef CONFIG_RTK_MESH
	DOT11_QUEUE2			*pathsel_queue;		// pathselection QUEUE
#ifdef _11s_TEST_MODE_
	DOT11_QUEUE2			*receiver_queue;	// pathselection QUEUE
	struct list_head		mtb_list;
#endif
#endif

#ifdef ENABLE_RTL_SKB_STATS
	atomic_t 				rtl_tx_skb_cnt;
	atomic_t				rtl_rx_skb_cnt;
#endif

	struct priv_shared_info	*pshare;		// pointer of shared info, david

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	struct rtl8192cd_priv		*proot_priv;	// ptr of private structure of root interface
#ifdef RESERVE_TXDESC_FOR_EACH_IF
	unsigned int			use_txdesc_cnt[7];
#ifdef USE_TXQUEUE
	unsigned int			use_txq_cnt[7];
#endif
#endif
#endif
#ifdef UNIVERSAL_REPEATER
	struct rtl8192cd_priv		*pvxd_priv;		// ptr of private structure of virtual interface
#endif
#ifdef MBSSID
	struct rtl8192cd_priv		*pvap_priv[RTL8192CD_NUM_VWLAN];	// ptr of private structure of vap interface
	short					vap_id;
	short					vap_init_seq;
	unsigned char			func_off_already;
	int						bcn_period_bak;
#endif

#ifdef DFS
	/*
	 *	timer for the blocked channel if radar detected
	 */
	struct timer_list		ch52_timer;
	struct timer_list		ch56_timer;
	struct timer_list		ch60_timer;
	struct timer_list		ch64_timer;
	struct timer_list		ch100_timer;
	struct timer_list		ch104_timer;
	struct timer_list		ch108_timer;
	struct timer_list		ch112_timer;
	struct timer_list		ch116_timer;
	struct timer_list		ch120_timer;
	struct timer_list		ch124_timer;
	struct timer_list		ch128_timer;
	struct timer_list		ch132_timer;
	struct timer_list		ch136_timer;
	struct timer_list		ch140_timer;

	/*
	 *	blocked channel will be removed from available_chnl[32] and placed in this list
	 */
	unsigned int			NOP_chnl[15];
	unsigned int			NOP_chnl_num;

	unsigned int			Not_DFS_chnl[32];
	unsigned int			Not_DFS_chnl_num;
#endif

#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
	WPA_GLOBAL_INFO			*wpa_global_info;
#endif

#ifdef CHECK_HANGUP
	int						reset_hangup;
	unsigned int			check_cnt_tx;
	unsigned int			check_cnt_rx;
	unsigned int			check_cnt_isr;
	unsigned int			check_cnt_bcn;
	unsigned int			check_cnt_rst;
	unsigned int			check_cnt_bb;
	unsigned int			check_cnt_cca;
#endif

#ifdef WDS
	struct net_device_stats	wds_stats[NUM_WDS];
	struct net_device		*wds_dev[NUM_WDS];
#endif
	unsigned int			auto_channel;			// 0: not auto, 1: auto select this time, 2: auto select next time
	unsigned int			auto_channel_backup;
	unsigned long			up_time;

#ifdef CONFIG_RTK_MESH
	unsigned char			RreqMAC[AODV_RREQ_TABLE_SIZE][MACADDRLEN];
	unsigned int			RreqBegin;
	unsigned int			RreqEnd;

	struct hash_table		*proxy_table, *mesh_rreq_retry_queue;
#ifdef PU_STANDARD
	struct hash_table		*proxyupdate_table;
#endif
	struct hash_table		*pathsel_table; // add by chuangch 2007.09.13
	struct mpp_tb			*pann_mpp_tb;

	struct MESH_FAKE_MIB_T	mesh_fake_mib;
	unsigned char			root_mac[MACADDRLEN];		// Tree Base root MAC

/*
	dev->priv->base_addr = 0 is wds
	dev->priv->base_addr = 1 is mesh
	We provide only one mesh device now. Although it is possible that more than one
	mesh devices bind with one physical interface simultaneously. RTL8186 shares the
	same MAC address with multiple virtual devices. Hence, the mesh data frame can't
	be handled (rx) by mesh devices correctly.
*/

	struct net_device		*mesh_dev;

#ifdef _MESH_ACL_ENABLE_
	struct mesh_acl_poll	*pmesh_acl_poll;
	struct list_head		mesh_aclpolllist;	// this is for poll management
	struct list_head		mesh_acl_list;		// this is for auth checking
#endif

#endif // CONFIG_RTK_MESH

#ifdef MESH_USE_METRICOP
	UINT32                          toMeshMetricAuto; // timeout, check mesh_fake_mib for further description
#endif

#ifdef EN_EFUSE
	unsigned char			AutoloadFailFlag;
	unsigned char			EfuseMap[2][EFUSE_MAP_LEN];
	unsigned short			EfuseUsedBytes;
#endif


#ifdef CONFIG_RTL_WAPI_SUPPORT
	uint8				*wapiCachedBuf;
	uint8				wapiNMK[WAPI_KEY_LEN];
	uint8				txMCast[WAPI_PN_LEN];
	uint8				rxMCast[WAPI_PN_LEN];
	uint8				keyNotify[WAPI_PN_LEN];
	uint8				aeWapiIE[256];
	uint8				wapiCachedLen;
	uint8				aeWapiIELength;
	uint8				wapiMCastKeyId:1;
	uint8				wapiMCastKeyUpdateAllDone:1;
	uint8				wapiMCastKeyUpdate:1;
	uint8				wapiMCastNeedInit:1;
	uint16				wapiWaiTxSeq;
	wapiKey				wapiMCastKey[2];
	unsigned long			wapiMCastKeyUpdateCnt;
	struct timer_list		waiMCastKeyUpdateTimer;
#endif

#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
	struct	net_device	*pWlanDev;
#endif
#if defined(CONFIG_RTL_NEW_AUTOCH) && defined(SS_CH_LOAD_PROC)
	unsigned int			ch_ss_rpt_cnt;
	struct ss_report		ch_ss_rpt[MAX_BSS_NUM];
#endif
#ifdef P2P_SUPPORT
	struct timer_list		p2p_listen_timer_t;
	struct timer_list		p2p_search_timer_t;
	struct p2p_context*		p2pPtr;
#endif

	/*********************************************************
	 * from here on, data will be clear in rtl8192cd_init_sw() *
	 *********************************************************/

	struct net_device_stats	net_stats;
	struct extra_stats		ext_stats;
#ifdef TLN_STATS
	struct tln_wifi_stats		wifi_stats;
	struct tln_ext_wifi_stats	ext_wifi_stats;
	unsigned int			stats_time_countdown;
#endif

	struct timer_list		frag_to_filter;
	unsigned int			frag_to;

	struct timer_list		expire_timer;
#ifdef PCIE_POWER_SAVING
struct timer_list			ps_timer;
#endif

	unsigned int			auth_to;	// second. time to expire in authenticating
	unsigned int			assoc_to;	// second. time to expire before associating
	unsigned int			expire_to;	// second. time to expire after associating

	struct timer_list		ss_timer;	//ss_timer: site_survey timer
	struct ss_res			site_survey;
	unsigned int			site_survey_times;
	unsigned char			ss_ssid[32];
	unsigned int			ss_ssidlen;
	unsigned char			ss_req_ongoing;
#ifdef CONFIG_RTL_COMAPI_WLTOOLS
	wait_queue_head_t		ss_wait;
#endif

	unsigned int			auth_seq;
	unsigned char			chg_txt[128];

	struct list_head		stat_hash[NUM_STAT];
	struct list_head		asoc_list;
	struct list_head		auth_list;
	struct list_head		defrag_list;
	struct list_head		sleep_list;
	struct list_head		wakeup_list;
	struct list_head		addRAtid_list;	// to avoid add RAtid fail
	struct list_head		addrssi_list;
	struct list_head		addps_list;

#ifdef WIFI_SIMPLE_CONFIG
	unsigned int			beaconbuf[MAX_WSC_IE_LEN];
	struct wsc_probe_request_info wsc_sta[MAX_WSC_PROBE_STA];
	unsigned int 			wps_issue_join_req;
	unsigned int			recover_join_req;
	struct bss_desc			dot11Bss_original;
	int						hidden_ap_mib_backup;
	unsigned	char		*pbeacon_ssid;

	unsigned	char 		orig_SSID[33];
	int 					orig_SSID_LEN;

#else
	unsigned int			beaconbuf[128];
#endif

	struct ht_cap_elmt		ht_cap_buf;
	unsigned int			ht_cap_len;
	struct ht_info_elmt		ht_ie_buf;
	unsigned int			ht_ie_len;
	unsigned int			ht_legacy_obss_to;	
	unsigned int			ht_nomember_legacy_sta_to;
	unsigned int			ht_legacy_sta_num;
	unsigned int			ht_protection;
	unsigned int			dc_th_current_state;

#ifdef RTK_AC_SUPPORT
	struct vht_cap_elmt		vht_cap_buf;
	unsigned int			vht_cap_len;
	struct vht_oper_elmt 	vht_oper_buf;
	unsigned int			vht_oper_len;
#endif

	// to avoid add RAtid fail
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)
	struct timer_list		add_RATid_timer;
	struct timer_list		add_rssi_timer;
#endif
	struct timer_list		add_ps_timer;
#if defined(CONFIG_RTL_92D_SUPPORT) && defined(CONFIG_RTL_NOISE_CONTROL)
	struct timer_list		dnc_timer;
#endif
#ifdef SMART_CONCURRENT_92D
	struct timer_list		smcc_prb_timer;
#endif

	unsigned short			timoffset;
	unsigned char			dtimcount;
	unsigned char			pkt_in_dtimQ;
	unsigned char			pkt_in_hiQ;

	//struct stat_info_cache	stainfo_cache;
	struct stat_info		*pstat_cache;

	struct list_head		rx_datalist;
	struct list_head		rx_mgtlist;
	struct list_head		rx_ctrllist;
#ifdef SMP_SYNC
	spinlock_t				rx_datalist_lock;
	spinlock_t				rx_mgtlist_lock;
	spinlock_t				rx_ctrllist_lock;
#endif

	int						assoc_num;		// association client number
	int						link_status;

	unsigned short			*pBeaconCapability;		// ptr of capability field in beacon buf
	unsigned char			*pBeaconErp;			// ptr of ERP field in beacon buf

	unsigned int			available_chnl[76];		// all available channel we can use
	unsigned int			available_chnl_num;		// record the number

#ifdef CONFIG_RTL_NEW_AUTOCH
	unsigned int			chnl_ss_fa_count[76];	// record FA count while ss
	unsigned int			chnl_ss_cca_count[76];	// record CCA count while ss
#ifdef SS_CH_LOAD_PROC
	unsigned char			chnl_ss_load[76];	// record noise level while ss
#endif
	unsigned int			chnl_ss_mac_rx_count[76];
	unsigned int			chnl_ss_mac_rx_count_40M[76];
#endif
#ifdef P2P_SUPPORT
	unsigned int	back_available_chnl[76];		// all available channel we can use
	unsigned int	back_available_chnl_num;		// record the number
#endif


	//unsigned int			oper_band;				// remember the operating bands
	unsigned int			supported_rates;
	unsigned int			basic_rates;

	// for MIC check
	struct timer_list		MIC_check_timer;
	struct timer_list		assoc_reject_timer;
	unsigned int			MIC_timer_on;
	unsigned int			assoc_reject_on;
	unsigned char                   assoc_reject_mac[6];

#ifdef CLIENT_MODE
	struct timer_list		reauth_timer;
	unsigned int			reauth_count;

	struct timer_list		reassoc_timer;
	unsigned int			reassoc_count;

	struct timer_list		idle_timer;
#ifdef DFS
	struct timer_list		dfs_cntdwn_timer;
#endif

	unsigned int			join_req_ongoing;
	int						join_index;
	unsigned long			jiffies_pre;
	unsigned int			ibss_tx_beacon;
	unsigned int			rxBeaconNumInPeriod;
	unsigned int			rxBeaconCntArray[ROAMING_DECISION_PERIOD_ARRAY];
	unsigned int			rxBeaconCntArrayIdx;	// current index of array
	unsigned int			rxBeaconCntArrayWindow;	// Is slide windows full
	unsigned int			rxBeaconPercentage;
	unsigned int			rxDataNumInPeriod;
	unsigned int			rxDataCntArray[ROAMING_DECISION_PERIOD_ARRAY];
	unsigned int			rxMlcstDataNumInPeriod;
	unsigned int			rxDataNumInPeriod_pre;
	unsigned int			rxMlcstDataNumInPeriod_pre;
	unsigned int			dual_band;
	unsigned int			supported_rates_alt;
	unsigned int			basic_rates_alt;
	unsigned char 			rx_timestamp[8];
	UINT64					prev_tsf;
#endif

	int						authModeToggle;		// auth mode toggle referred when auto-auth mode is set under client mode, david
	int						authModeRetry;		// auth mode retry sequence when auto-auth mode is set under client mode

	int						acLogCountdown;		// log count-down time
	struct ac_log_info		acLog[MAX_AC_LOG];

	struct tx_desc			*amsdu_first_desc;

	unsigned int			bcnTxAGC;
	unsigned int            bcnTxAGC_bak;
	unsigned short          tx_beacon_len;

#ifdef WIFI_11N_2040_COEXIST
	struct obss_scan_para_elmt		obss_scan_para_buf;
	unsigned int					obss_scan_para_len;
	unsigned int					bg_ap_timeout;
#ifdef CLIENT_MODE
	unsigned int					bg_ap_timeout_ch[14];
	unsigned int					intolerant_timeout;
	unsigned int					coexist_connection;
#endif
	unsigned int					force_20_sta;
	unsigned int					switch_20_sta;
#ifdef CONFIG_RTL_88E_SUPPORT
	unsigned int					force_20_sta_88e_hw_ext;
	unsigned int					switch_20_sta_88e_hw_ext;
#endif
#ifdef STA_EXT
	unsigned int					force_20_sta_ext;
	unsigned int					switch_20_sta_ext;
#endif
#endif

	/*********************************************************************
	 * from here on till EE_Cached will be backup during hang up reset   *
	 *********************************************************************/
#ifdef CLIENT_MODE
	unsigned int			join_res;
	unsigned int			beacon_period;
	unsigned short			aid;
	unsigned int			ps_state;
#if defined(WIFI_WMM) && defined (WMM_APSD)
	unsigned int			uapsd_assoc;
#endif
#ifdef RTK_BR_EXT
	unsigned int			macclone_completed;
	struct nat25_network_db_entry	*nethash[NAT25_HASH_SIZE];
	int						pppoe_connection_in_progress;
	unsigned char			pppoe_addr[MACADDRLEN];
	unsigned char			scdb_mac[MACADDRLEN];
	unsigned char			scdb_ip[4];
	struct nat25_network_db_entry	*scdb_entry;
	unsigned char			br_mac[MACADDRLEN];
	unsigned char			br_ip[4];
	unsigned char			ukpro_mac[MACADDRLEN];	// mac address of unknown protocol
	unsigned char			ukpro_mac_valid;		// if the above entry is valid
#endif
	unsigned char 			up_flag;
#endif

#ifdef MICERR_TEST
	unsigned int			micerr_flag;
#endif

#ifdef SMART_CONCURRENT_92D
	unsigned int			smcc_state;
	struct SMCC_MAC_Info_Tbl *MAC_info;
#endif

#ifdef DFS
	struct timer_list		DFS_timer;			/* timer for radar detection */
	struct timer_list		ch_avail_chk_timer;	/* timer for channel availability check */
	struct timer_list		dfs_chk_timer;	/* timer for dfs trigger */
#endif

#ifdef GBWC
	struct timer_list		GBWC_timer;
	struct pkt_queue		GBWC_tx_queue;
	struct pkt_queue		GBWC_rx_queue;
	unsigned int			GBWC_tx_count;
	unsigned int			GBWC_rx_count;
	unsigned int			GBWC_consuming_Q;
#endif

#ifdef SUPPORT_SNMP_MIB
	struct mib_snmp			snmp_mib;
#endif

	struct pkt_queue		dz_queue;	// Queue for multicast pkts when there is sleeping sta
	unsigned char			release_mcast;

#ifndef USE_RTL8186_SDK
	unsigned int			amsdu_first_dma_desc;
	unsigned int			ampdu_first_dma_desc;
#endif
	int						amsdu_len;
	int						ampdu_len;

#ifdef CHECK_RX_HANGUP
	unsigned long			rx_packets_pre1;
	unsigned long			rx_packets_pre2;
	unsigned int 			rx_start_monitor_running;
#endif

#ifdef USB_PKT_RATE_CTRL_SUPPORT
	unsigned int			change_toggle;
	unsigned int			pre_pkt_cnt;
	unsigned int			pkt_nsec_diff;
	unsigned int			poll_usb_cnt;
	unsigned int			auto_rate_mask;
#endif

#ifdef CONFIG_RTL8186_KB
	GUESTMAC_T				guestMac[MAX_GUEST_NUM];
#endif

#ifdef CONFIG_RTK_VLAN_SUPPORT
	int						global_vlan_enable;
	struct vlan_info	vlan_setting;
#endif

#ifdef CONFIG_RTK_MESH
	struct net_device_stats	mesh_stats;

#ifdef _11s_TEST_MODE_
	char 					rvTestPacket[3000];
#endif

	UINT8					mesh_Version;
	// WLAN Mesh Capability
	INT16					mesh_PeerCAP_cap;		// peer capability-Cap number (Signed!)
	UINT8					mesh_PeerCAP_flags;		// peer capability-flags
	UINT8					mesh_PowerSaveCAP;		// Power Save capability
	UINT8					mesh_SyncCAP;			// Synchronization capability
	UINT8					mesh_MDA_CAP;			// MDA capability
	UINT32					mesh_ChannelPrecedence;	// Channel Precedence

	UINT8					mesh_HeaderFlags;		// mesh header in mesh flags field

	// MKD domain element [MKDDIE]
	UINT8					mesh_MKD_DomainID[6];
	UINT8					mesh_MKDDIE_SecurityConfiguration;

	// EMSA Handshake element [EMSAIE]
	UINT8					mesh_EMSAIE_ANonce[32];
	UINT8					mesh_EMSAIE_SNonce[32];
	UINT8					mesh_EMSAIE_MA_ID[6];
	UINT16					mesh_EMSAIE_MIC_Control;
	UINT8					mesh_EMSAIE_MIC[16];

#ifdef MESH_BOOTSEQ_AUTH
	struct timer_list		mesh_auth_timer;		///< for unestablish (And establish to unestablish) MP mesh_auth_hdr

	// mesh_auth_hdr:
	//  It is a list structure, only stores unAuth MP entry
	//  Each entry is a pointer pointing to an entry in "stat_info->mesh_mp_ptr"
	//  and removed by successful "Auth" or "Expired"
	struct list_head		mesh_auth_hdr;
#endif

	struct timer_list		mesh_peer_link_timer;	///< for unestablish (And establish to unestablish) MP mesh_unEstablish_hdr

	// mesh_unEstablish_hdr:
	//  It is a list structure, only stores unEstablish (or Establish -> unEstablish [MP_HOLDING])MP entry
	//  Each entry is a pointer pointing to an entry in "stat_info->mesh_mp_ptr"
	//  and removed by successful "Peer link setup" or "Expired"
	struct list_head		mesh_unEstablish_hdr;

	// mesh_mp_hdr:
	//  It is a list of MP/MAP/MPP who has already passed "Peer link setup"
	//  Each entry is a pointer pointing to an entry in "stat_info->mesh_mp_ptr"
	//  Every entry is inserted by "successful peer link setup"
	//  and removed by "Expired"
	struct list_head		mesh_mp_hdr;

	struct MESH_Profile		mesh_profile[1];	// Configure by WEB in the future, Maybe delete, Preservation before delete


#ifdef MESH_BOOTSEQ_STRESS_TEST
	unsigned long			mesh_stressTestCounter;
#endif	// MESH_BOOTSEQ_STRESS_TEST

	// Throughput statistics (sounder)
	unsigned int			mesh_log;
	unsigned long			log_time;

#ifdef _MESH_ACL_ENABLE_
	unsigned char			meshAclCacheAddr[MACADDRLEN];
	unsigned char			meshAclCacheMode;
#endif
#endif // CONFIG_RTK_MESH

#ifdef SUPPORT_TX_MCAST2UNI
	int 							stop_tx_mcast2uni;
#endif

#ifdef PCIE_POWER_SAVING
	unsigned int offload_ctrl;
	unsigned char pwr_state;
	unsigned char ps_ctrl;
#ifdef PCIE_POWER_SAVING_DEBUG
	char firstPkt;
#endif
#endif

#ifdef TEMP_MP_92D
	unsigned char	mp_start;
	unsigned int	mp_txrate;
#endif

#ifdef SW_TX_QUEUE
    unsigned char   record_mac[6];
    int             record_qnum;
    int             swq_txmac_chg;
    int             swq_en;
    unsigned short  record_aid;
    unsigned long   swqen_keeptime;    
#endif

#ifdef A4_STA
	struct list_head			a4_sta_list;
	struct a4_sta_db_entry		*machash[A4_STA_HASH_SIZE];
	struct a4_tbl_entry 		a4_ent[MAX_A4_TBL_NUM];
#endif

#ifdef WIFI_WPAS
	unsigned char 	wpas_manual_assoc; //_Eric ??
#endif
	int update_bcn_period;

#ifdef SUPPORT_MULTI_PROFILE
	int	profile_idx;			// indicate next used profile. 
	int mask_n_band;
#endif
#ifdef SWITCH_CHAN
	int	chan_backup;
	int	bw_backup;
	int	offset_backup;
	int func_backup;
#endif
#if defined(CONFIG_RTL_8812_SUPPORT)||defined(CONFIG_WLAN_HAL_8881A)
	char bFWReady;
#endif	
}RTL8192CD_PRIV, *PRTL8192CD_PRIV;

struct rtl8192cd_chr_priv {
	unsigned int			major;
	unsigned int			minor;
	struct rtl8192cd_priv*	wlan_priv;
	struct fasync_struct*	asoc_fasync;	// asynch notification
};

#ifdef NETDEV_NO_PRIV
struct rtl8192cd_wds_priv {
	struct rtl8192cd_priv*	wlan_priv;      //This element shall be put at top of this struct
};
#endif

/* station info, reported to web server */
typedef struct _sta_info_2_web {
	unsigned short	aid;
	unsigned char	addr[6];
	unsigned long	tx_packets;
	unsigned long	rx_packets;
	unsigned long	expired_time;	// 10 msec unit
	unsigned short	flags;
	unsigned char	TxOperaRate;
	unsigned char	rssi;
	unsigned long	link_time;		// 1 sec unit
	unsigned long	tx_fail;
	unsigned long	tx_bytes;
	unsigned long	rx_bytes;
	unsigned char	network;
	unsigned char	ht_info;		// bit0: 0=20M mode, 1=40M mode; bit1: 0=longGI, 1=shortGI
#ifdef TLN_STATS
	unsigned char		RxOperaRate;
	unsigned char		auth_type;
	unsigned char		enc_type;
	unsigned char 	resv[3];
#else
	unsigned char 	resv[6];
#endif
} sta_info_2_web;

#define NULL_MAC_ADDR		("\x0\x0\x0\x0\x0\x0")

// Macros
#define GET_MIB(priv)		(priv->pmib)
#define GET_HW(priv)		(priv->pshare->phw)

#define AP_BSSRATE			((GET_MIB(priv))->dot11StationConfigEntry.dot11OperationalRateSet)
#define AP_BSSRATE_LEN		((GET_MIB(priv))->dot11StationConfigEntry.dot11OperationalRateSetLen)

#ifdef RTK_AC_SUPPORT  //vht rate 
#define AP_BSSRATE_AC			(priv->pshare->rf_ft_var.dot11OperationalRateSet_AC)
#define AP_BSSRATE_LEN_AC		(priv->pshare->rf_ft_var.dot11OperationalRateSetLen_AC)
#endif


#define STAT_OPRATE			(pstat->bssrateset)
#define STAT_OPRATE_LEN		(pstat->bssratelen)

#define BSSID			((GET_MIB(priv))->dot11StationConfigEntry.dot11Bssid)

#define SSID			((GET_MIB(priv))->dot11StationConfigEntry.dot11DesiredSSID)

#define SSID_LEN		((GET_MIB(priv))->dot11StationConfigEntry.dot11DesiredSSIDLen)

#define OPMODE			((GET_MIB(priv))->dot11OperationEntry.opmode)

#define HIDDEN_AP		((GET_MIB(priv))->dot11OperationEntry.hiddenAP)

#define RTSTHRSLD		((GET_MIB(priv))->dot11OperationEntry.dot11RTSThreshold)

#define FRAGTHRSLD		((GET_MIB(priv))->dot11OperationEntry.dot11FragmentationThreshold)

#define EXPIRETIME		((GET_MIB(priv))->dot11OperationEntry.expiretime)

#define LED_TYPE		((GET_MIB(priv))->dot11OperationEntry.ledtype)

#ifdef RTL8190_SWGPIO_LED
#define LED_ROUTE		((GET_MIB(priv))->dot11OperationEntry.ledroute)
#endif

#define IAPP_ENABLE		((GET_MIB(priv))->dot11OperationEntry.iapp_enable)

#define SWCRYPTO		((GET_MIB(priv))->dot11StationConfigEntry.dot11swcrypto)

#define IEEE8021X_FUN	((GET_MIB(priv))->dot118021xAuthEntry.dot118021xAlgrthm)

#define ACCT_FUN		((GET_MIB(priv))->dot118021xAuthEntry.acct_enabled)

#define ACCT_FUN_TIME	((GET_MIB(priv))->dot118021xAuthEntry.acct_timeout_period)

#define ACCT_FUN_TP		((GET_MIB(priv))->dot118021xAuthEntry.acct_timeout_throughput)

#define ACCT_TP_INT		60

#define SHORTPREAMBLE	((GET_MIB(priv))->dot11RFEntry.shortpreamble)

#define SSID2SCAN		((GET_MIB(priv))->dot11StationConfigEntry.dot11SSIDtoScan)

#define SSID2SCAN_LEN	((GET_MIB(priv))->dot11StationConfigEntry.dot11SSIDtoScanLen)

#define RX_BUF_LEN		((GET_MIB(priv))->dot11nConfigEntry.dot11nAMSDURecvMax?(MAX_RX_BUF_LEN):(MIN_RX_BUF_LEN))

#ifdef WIFI_WMM
#define QOS_ENABLE 		((GET_MIB(priv))->dot11QosEntry.dot11QosEnable)

#define APSD_ENABLE 	((GET_MIB(priv))->dot11QosEntry.dot11QosAPSD)

#define GET_WMM_IE		((GET_MIB(priv))->dot11QosEntry.WMM_IE)

#define GET_WMM_PARA_IE		((GET_MIB(priv))->dot11QosEntry.WMM_PARA_IE)

#define GET_EDCA_PARA_UPDATE 	((GET_MIB(priv))->dot11QosEntry.EDCAparaUpdateCount)

#define GET_STA_AC_BE_PARA	((GET_MIB(priv))->dot11QosEntry.STA_AC_BE_paraRecord)

#define GET_STA_AC_BK_PARA	((GET_MIB(priv))->dot11QosEntry.STA_AC_BK_paraRecord)

#define GET_STA_AC_VI_PARA	((GET_MIB(priv))->dot11QosEntry.STA_AC_VI_paraRecord)

#define GET_STA_AC_VO_PARA	((GET_MIB(priv))->dot11QosEntry.STA_AC_VO_paraRecord)
#endif

#ifdef DOT11D
#define COUNTRY_CODE_ENABLED 	((GET_MIB(priv))->dot11dCountry.dot11CountryCodeSwitch)
#endif

#ifdef P2P_SUPPORT
#define P2PMODE			((GET_MIB(priv))->p2p_mib.p2p_type)
#define P2P_STATE		((GET_MIB(priv))->p2p_mib.p2p_state)
#define P2P_DISCOVERY		((GET_MIB(priv))->p2p_mib.p2p_on_discovery)
#define P2P_EVENT_INDICATE		((GET_MIB(priv))->p2p_mib.p2p_event_indiate)

#endif


#define AMPDU_ENABLE	((GET_MIB(priv))->dot11nConfigEntry.dot11nAMPDU)
#define AMSDU_ENABLE	((GET_MIB(priv))->dot11nConfigEntry.dot11nAMSDU)

#define TSF_LESS(a, b)	(((a - b) & 0x80000000) != 0)
#define TSF_DIFF(a, b)	((a >= b)? (a - b):(0xffffffff - b + a + 1))

#define GET_GROUP_MIC_KEYLEN	((GET_MIB(priv))->dot11GroupKeysTable.dot11EncryptKey.dot11TMicKeyLen)
#define GET_GROUP_TKIP_MIC1_KEY	((GET_MIB(priv))->dot11GroupKeysTable.dot11EncryptKey.dot11TMicKey1.skey)
#define GET_GROUP_TKIP_MIC2_KEY	((GET_MIB(priv))->dot11GroupKeysTable.dot11EncryptKey.dot11TMicKey2.skey)

#define GET_UNICAST_MIC_KEYLEN		(pstat->dot11KeyMapping.dot11EncryptKey.dot11TMicKeyLen)
#define GET_UNICAST_TKIP_MIC1_KEY	(pstat->dot11KeyMapping.dot11EncryptKey.dot11TMicKey1.skey)
#define GET_UNICAST_TKIP_MIC2_KEY	(pstat->dot11KeyMapping.dot11EncryptKey.dot11TMicKey2.skey)

#define GET_GROUP_ENCRYP_KEY		((GET_MIB(priv))->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey)
#define GET_UNICAST_ENCRYP_KEY		(pstat->dot11KeyMapping.dot11EncryptKey.dot11TTKey.skey)

#define GET_GROUP_ENCRYP_KEYLEN			((GET_MIB(priv))->dot11GroupKeysTable.dot11EncryptKey.dot11TTKeyLen)
#define GET_UNICAST_ENCRYP_KEYLEN		(pstat->dot11KeyMapping.dot11EncryptKey.dot11TTKeyLen)

#define GET_MY_HWADDR		((GET_MIB(priv))->dot11OperationEntry.hwaddr)

#define GET_GROUP_ENCRYP_PN			(&((GET_MIB(priv))->dot11GroupKeysTable.dot11EncryptKey.dot11TXPN48))
#define GET_UNICAST_ENCRYP_PN		(&(pstat->dot11KeyMapping.dot11EncryptKey.dot11TXPN48))

#define SET_SHORTSLOT_IN_BEACON_CAP		\
	do {	\
		if (priv->pBeaconCapability != NULL)	\
			*priv->pBeaconCapability |= cpu_to_le16(BIT(10));	\
	} while(0)

#define RESET_SHORTSLOT_IN_BEACON_CAP	\
	do {	\
		if (priv->pBeaconCapability != NULL)	\
			*priv->pBeaconCapability &= ~cpu_to_le16(BIT(10));	\
	} while(0)

#define IS_DRV_OPEN(priv) ((priv==NULL) ? 0 : ((priv->drv_state & DRV_STATE_OPEN) ? 1 : 0))

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
#define GET_ROOT_PRIV(priv)			(priv->proot_priv)
#define IS_ROOT_INTERFACE(priv)	 	(GET_ROOT_PRIV(priv) ? 0 : 1)
#define GET_ROOT(priv)				((priv->proot_priv) ? priv->proot_priv : priv)
#else
#define GET_ROOT(priv)		(priv)
#endif
#ifdef UNIVERSAL_REPEATER
#define GET_VXD_PRIV(priv)			(priv->pvxd_priv)
#ifdef MBSSID
#define IS_VXD_INTERFACE(priv)		((GET_ROOT_PRIV(priv) ? 1 : 0) && (priv->vap_id < 0))
#else
#define IS_VXD_INTERFACE(priv)		(GET_ROOT_PRIV(priv) ? 1 : 0)
#endif
#endif // UNIVERSAL_REPEATER
#ifdef MBSSID
#define IS_VAP_INTERFACE(priv)		(!IS_ROOT_INTERFACE(priv) && (priv->vap_id >= 0))
#endif

#define MANAGE_QUE_NUM		MGNT_QUEUE

#ifdef DFS
#define DFS_TO					RTL_10MILISECONDS_TO_JIFFIES(priv->pmib->dot11DFSEntry.DFS_timeout)
#define NONE_OCCUPANCY_PERIOD	RTL_10MILISECONDS_TO_JIFFIES(priv->pmib->dot11DFSEntry.NOP_timeout)
#endif

#ifdef TX_EARLY_MODE
#define GET_TX_EARLY_MODE			(priv->pshare->rf_ft_var.em_enable)
#define GET_EM_SWQ_ENABLE			(priv->pshare->em_waitq_on)
#define MAX_EM_QUE_NUM				40//32
#define EM_TP_UP_BOUND				6000000 /* 48M */
#define EM_TP_LOW_BOUND			2000000 /* 16M */
#define WAIT_TP_TIME				3	/* wait TP limit for this period in sec */
#endif


#ifdef __KERNEL__
#ifdef _DEBUG_RTL8192CD_
#define ASSERT(expr) \
        if(!(expr)) {					\
  			printk( "\033[33;41m%s:%d: assert(%s)\033[m\n",	\
	        __FILE__,__LINE__,#expr);		\
        }
#else
	#define ASSERT(expr)
#endif
#endif

#ifdef USE_OUT_SRC
#define	ODMPTR					(&(priv->pshare->_dmODM))
#endif

#ifdef RTL8190_SWGPIO_LED
/* =====================================================================
		LED route configuration:

			Currently, LOW 10 bits of this MIB are used.

			+---+---+---+---+---+---+---+---+---+---+
			| E1| H1|  Route 1  | E0| H0|  Route 0  |
			+---+---+---+---+---+---+---+---+---+---+

			E0		: Indicates if the field route 0 is valid or not.
			E1		: Indicates if the field route 1 is valid or not.
			H0		: Indicate the GPIO indicated by route 0 is Active HIGH or Active LOW. ( 0: Active LOW, 1: Active HIGH)
			H1		: Indicate the GPIO indicated by route 1 is Active HIGH or Active LOW. ( 0: Active LOW, 1: Active HIGH)
			Route0	: The GPIO number (0~6) which used by LED0. Only used when E0=0b'1
			Route1	: The GPIO number (0~6) which used by LED1. Only used when E1=0b'1

			Unused bits	: reserved for further extension, must set to 0.

			Currently RTL8185 AP driver supports LED0/LED1, and RTL8185 has 7 GPIOs in it.
			So we need a routing-mechanism to decide what GPIO is used for LED0/LED1.
			The bit-field pairs {E0, H0, Route0}, {E1, H0, Route1} is used to set it.
			Ex.
				One customer only use GPIO0 for LED1 (Active LOW) and don't need LED0,
				he can set the route being:
				----------------------------------
				E0 = 0
				E1 = 1
				H0 = 0 ( Driver would ignore it )
				H1 = 0  ( Driver would ignore it )
				Route 0 = 0 ( Driver would ignore it )
				Route 1 = 0 ( GPIO0 )

				ledroute = 0x10 << 5;		: LED1 -Active LOW, GPIO0
				ledroute |= 0;				: LED0 -Disabled
				----------------------------------
     ===================================================================== */
#define	SWLED_GPIORT_CNT			2					/* totally we have max 3 GPIOs reserved for LED route usage */
#define	SWLED_GPIORT_RTBITMSK		0x07				/* bit mask of routing field = 0b'111 */
#define	SWLED_GPIORT_HLMSK			0x08				/* bit mask of Active high/low field = 0b'1000 */
#define	SWLED_GPIORT_ENABLEMSK		0x10				/* bit mask of enable filed = 0b'10000 */
#define	SWLED_GPIORT_ITEMBITCNT		5					/* total bit count of each item */
#define	SWLED_GPIORT_ITEMBITMASK	(	SWLED_GPIORT_RTBITMSK |\
										SWLED_GPIORT_HLMSK |\
										SWLED_GPIORT_ENABLEMSK)	/* bit mask of each item */
#define	SWLED_GPIORT_ITEM(ledroute, x)	(((ledroute) >> ((x)*SWLED_GPIORT_ITEMBITCNT)) & SWLED_GPIORT_ITEMBITMASK)
#endif // RTL8190_SWGPIO_LED

#ifdef SUPPORT_SNMP_MIB
#define SNMP_MIB(f)					(priv->snmp_mib.f)
#define SNMP_MIB_ASSIGN(f,v)		(SNMP_MIB(f)=v)
#define SNMP_MIB_COPY(f,v,len)		(memcpy(&SNMP_MIB(f), v, len))
#define SNMP_MIB_INC(f,v)			(SNMP_MIB(f)+=v)
#define SNMP_MIB_DEC(f,v)			(SNMP_MIB(f)-=v)

#else

#define SNMP_MIB(f)
#define SNMP_MIB_ASSIGN(f,v)
#define SNMP_MIB_COPY(f,v,len)
#define SNMP_MIB_INC(f,v)
#define SNMP_MIB_DEC(f,v)
#endif //SUPPORT_SNMP_MIB


#ifdef USB_PKT_RATE_CTRL_SUPPORT
	typedef unsigned int (*usb_pktCnt_fn)(void);
	typedef unsigned int (*register_usb_pkt_cnt_fn)(void *);
	extern register_usb_pkt_cnt_fn register_usb_hook;
#endif


#ifdef CONFIG_RTK_VLAN_SUPPORT
#if defined(CONFIG_RTK_VLAN_NEW_FEATURE)
extern int  rx_vlan_process(struct net_device *dev, struct VlanConfig *info_ori, struct sk_buff *skb, struct sk_buff **new_skb);
extern int  tx_vlan_process(struct net_device *dev, struct VlanConfig *info_ori, struct sk_buff *skb, int wlan_pri);
#else
extern int  rx_vlan_process(struct net_device *dev, struct VlanConfig *info, struct sk_buff *skb);
extern int  tx_vlan_process(struct net_device *dev, struct VlanConfig *info, struct sk_buff *skb, int wlan_pri);
#endif
#endif


//#ifdef  SUPPORT_TX_MCAST2UNI
#define IP_MCAST_MAC(mac)		((mac[0]==0x01)&&(mac[1]==0x00)&&(mac[2]==0x5e))
#define IPV6_MCAST_MAC(mac)	((mac[0]==0x33)&&(mac[1]==0x33))

/*match is  (1)ipv4 && (2)(IGMP control/management packet) */
#define IS_IGMP_PROTO(mac)	((mac[12]==0x08) && (mac[13]==0x00) && (mac[23]==0x02))

#define IS_ICMPV6_PROTO(mac)		( (mac[12]==0x86)&&(mac[13]==0xdd) && ((mac[20]==0x3a)||(mac[54]==0x3a)))
#define IS_MDNSV4_MAC(mac) ((mac[0]==0x01)&&(mac[1]==0x00)&&(mac[2]==0x5e)&& (mac[3]==0x00)&&(mac[4]==0x00)&&(mac[5]==0xFB)&&(mac[12]==0x08)&&(mac[13]==0x00))
#define IS_MDNSV6_MAC(mac) ((mac[0]==0x33)&&(mac[1]==0x33)&&(mac[2]==0x00)&& (mac[3]==0x00)&&(mac[4]==0x00)&&(mac[5]==0xFB)&&(mac[12]==0x86)&&(mac[13]==0xdd))

//#ifdef	TX_SUPPORT_IPV6_MCAST2UNI
#define ICMPV6_MCAST_MAC(mac)	((mac[0]==0x33)&&(mac[1]==0x33)&&(mac[2]!=0xff))

#define ICMPV6_PROTO1A_VALN(mac)		( (mac[12+4]==0x86)&&(mac[13+4]==0xdd)&& (mac[54+4]==0x3a))
#define ICMPV6_PROTO1B_VALN(mac)		( (mac[12+4]==0x86)&&(mac[13+4]==0xdd)&& (mac[20+4]==0x3a))
#define ICMPV6_PROTO2X_VALN(mac)		( (mac[12+4]==0x86)&&(mac[13+4]==0xdd)&& (mac[54+4]==0x3a || mac[20+4]==0x3a))

#define ICMPV6_PROTO1A(mac)		( (mac[12]==0x86)&&(mac[13]==0xdd)&& (mac[54]==0x3a))
#define ICMPV6_PROTO1B(mac)		( (mac[12]==0x86)&&(mac[13]==0xdd)&& (mac[20]==0x3a))
#define ICMPV6_PROTO2X(mac)		( (mac[12]==0x86)&&(mac[13]==0xdd)&& (mac[54]==0x3a || mac[20]==0x3a))

//#endif
//#endif


#ifdef DOT11D
#define A_BAND_MAX_CHANNEL_NUMBER	24
#define COUNTRYNUMBER				100

typedef struct PerChannelSet {
	unsigned char	firstChannel;
	unsigned char	numberOfChannel;
	unsigned char	maxTxDbm;
} PER_CHANNEL_ENTRY;

typedef struct G_BAND_TABLE_ELEMENT {
	unsigned char		region;
	PER_CHANNEL_ENTRY 	channel_set;
} G_BAND_TABLE_ELEMENT_T;

typedef struct A_BAND_TABLE_ELEMENT {
	unsigned char		region;
	unsigned char		setNumber;
	PER_CHANNEL_ENTRY 	channel_set[A_BAND_MAX_CHANNEL_NUMBER];
} A_BAND_TABLE_ELEMENT_T;

typedef struct countryIE {
	unsigned int		countryNumber;
	unsigned char 		countryA2[3];
	unsigned char		A_Band_Region;	//if support 5G A band? ;  0 == no support ; aBandRegion == real region domain
	unsigned char		G_Band_Region;	//if support 2.4G G band? ;  0 == no support ; bBandRegion == real region domain
	unsigned char 		countryName[24];
} COUNTRY_IE_ELEMENT;
#endif

#if defined(TXREPORT) && (defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)) || defined(CONFIG_WLAN_HAL)
struct tx_rpt{
	unsigned short		txfail;
	unsigned short      txok;
	unsigned short      macid;
	unsigned char     	initil_tx_rate;	//add for 8812	
};
#endif

struct _device_info_ {
	int type;
	unsigned long conf_addr;
	unsigned long base_addr;
	int irq;
	struct rtl8192cd_priv *priv;
};

#define HIDE_AP_FOUND			1
#define HIDE_AP_FOUND_DO_ACTIVE_SSAN	2

static __inline__ int is_passive_channel(int domain, int chan)
{
	if ((domain == DOMAIN_GLOBAL || domain == DOMAIN_WORLD_WIDE) &&
			(chan == 12 || chan == 13 || chan == 14))
		return 1;
	
	return 0;
}

#ifdef GBWC
#define GBWC_MODE_DISABLE			0
#define GBWC_MODE_LIMIT_MAC_INNER	1 // limit bw by mac address
#define GBWC_MODE_LIMIT_MAC_OUTTER	2 // limit bw by excluding the mac
#define GBWC_MODE_LIMIT_IF_TX		3 // limit bw by interface tx
#define GBWC_MODE_LIMIT_IF_RX		4 // limit bw by interface rx
#define GBWC_MODE_LIMIT_IF_TRX		5 // limit bw by interface tx/rx
#endif

// andrew, define a compatible data macro
#if defined(LINUX_2_6_22_)
#define SKB_MAC_HEADER(s) skb_mac_header(s)
#define SKB_IP_HEADER(s) (struct iphdr *)(skb_mac_header(s) + ETH_HLEN);
#else // older 2.6 header
#define SKB_MAC_HEADER(s) (s)->mac.raw
#define SKB_IP_HEADER(s) (struct iphdr *)((s)->mac.raw + ETH_HLEN);
#endif

#ifdef __ECOS
extern struct _device_info_ wlan_device[];
#endif

#ifdef     CONFIG_WLAN_HAL
#include "WlanHAL/Output/HalLib.h"
#endif  //CONFIG_WLAN_HAL

#endif // _8192CD_H_

