/*
 * voip_flash.h: VoIP Flash Header
 *
 * Authors: Rock <shaofu@realtek.com.tw>
 *
 */

#ifndef __VOIP_FLASH_H
#define __VOIP_FLASH_H

#include "kernel_config.h"
#include "rtk_voip.h"
#include "voip_feature.h"
#ifdef CONFIG_RTK_VOIP_IP_PHONE
#include "ui_flash_layout.h"
#endif

/* Define below marco to  support TLV in VoIP configuration */
#define VOIP_SUPPORT_TLV_CFG 1

#ifndef CONFIG_RTK_VOIP_PACKAGE_867X //defined(CONFIG_RTK_VOIP_IP_PHONE) || defined(CONFIG_APP_TR104)
#define SUPPORT_VOIP_FLASH_WRITE	/* flash write module */
#endif

#define MIBDEF(_ctype,  _cname, _crepeat, _mib_name, _mib_type, _mib_parents_ctype, _default_value, _next_tbl ) \
    _ctype _cname _crepeat;
#define MIBARY MIBDEF
#define MIBLST MIBDEF
#define MIBSTR MIBDEF

//------------------------------------------------------------------
//			VOIP FLASH FEATURE
//------------------------------------------------------------------
#define VOIP_FLASH_SIGNATURE			0x766f6970		// voip
#define VOIP_CONFIG_SIGNATURE			0x08010806		// 8186

#define VOIP_FLASH_VER					60
// ver 2: add call_waiting_cid
// ver 3: add funckey_transfer, funckey_pstn, off_hook_alarm, auto_dial
// ver 4:
//	- change default sip port (couldn't be the same sip port)
//	- add speed dial
// ver 5:
//  - change speed dial length: 31->61
//	- hide credit_quota
//	- add auto config setting
//  - add mib version!
// ver 6:
//  - add jitter delay configuration
// ver 7:
//  - add QoS TOS/DSCP
// ver 8:
//  - add min flash hook time
// ver 9:
//  - add caller id dtmf start/end digit configruation
// ver 10:
//  - add dial plan
// ver 11:
//  - add sip info duration
// ver 12:
//  - modify dial plan layout: divide 256 bytes into many parts including
//    original dial plan, replace rule, and auto prefix.
// ver 13:
//  - add agc config
// ver 14:
//  - enlarge _CODEC_MAX
// ver 15:
//  - add caller id detection mode
// ver 16:
//  - add caller id auto detection selection
// ver 17:
//  - move daa volume to structure voipCfgParam_s
// ver 18:
//	- change feature to 32 bits
//  - move ports[] to last field of voipCfgParam_s for handling port number not match issue
// ver 19:
//  - add FSK caller id gen mode selection
// ver 20:
// - add T.38 config
// ver 21:
// - add voice gain config
// ver 22:
// - add rtpDscp, sipDscp
// ver 23:
// - add hotline, dnd
// ver 24:
// - add VLAN for Voice and Data traffic
// ver 25:
// - use hotline/dnd per port
// ver 26:
// - add video vlan tag
// ver 27:
// - add optimization factor of jitter buffer
// ver 28:
// - add iLBC
// - move flash hook time from voipCfgParam_s to voipCfgPortParam_s
// - support multi proxy
// ver 29:
// - add Security parameters SECURITY_ENABLE, TLS_ENABLE, SECURITY_KEY_EXCHANGE_MODE
// ver 30:
// - add pulse dial generation/detection config
// ver 31:
// - add fax modem det config
// ver 32:
// - add off-hook passwd
// - add one stage dialing
// ver 33:
// - add abbreviated dial (phonebook)
// - add alarm
// ver 34:
// - add PSTN routing prefix
// ver 35:
// - remove default_proxy
// ver 36:
// - restore default proxy
// ver 37:
// - auto bypass relay
// ver 38:
// - add two stage dialing option (VoIP -> PSTN)
// - add direct ip call option
// ver 39:
// - add disconnect tone detect
// ver 40:
// - add enable HW-NAT
// - add TFTP and FTP for auto configuration
// ver 42:
// - add g726 packing config
// ver 43:
// - refine auto cofig and fw update
// ver 44:
// - add Bandwidth Mgr
// ver 45:
// - add sip-tls enable to proxy
// ver 46:
// - add speex narrow band
// ver 47:
// - add V.152 parameters
// ver 48:
// - add RTP redundant
// ver 49:
// - add HW add SW VLAN
// ver 50:
// - add G711.1 (G711 Wideband)
// ver 51:
// - refine VLAN WAN TAG
// ver 51:
// - add T.38 parameters
// ver 52:
// - add PLC switch
// ver 53:
// - add RTCP XR switch
// ver 54:
// - add answert tone detect
// ver 55:
// - add LEC switch
// ver 56:
// - add NLP switch
// ver 57:
// - add CNG switch, VAD/CNG threshold
// ver 58:
// - add SID noise level configuration
// ver 59:
// - add Fax/Modem RFC2833 config
// ver 60:
// - add RTCP interval 
// ver 61:
// - add RFC2833 packet interval, Fax/Modem RFC2833 payload type

#define VOIP_FLASH_MIB_VER				0x0000

//#define	VOIP_FLASH_FEATURE				( uint32 )( ( RTK_VOIP_FEATURE ) >> 32 )
//#define VOIP_FLASH_EXT_FEATURE 			( uint32 )( ( RTK_VOIP_FEATURE ) & 0xFFFFFFFFUL )
#define VOIP_FLASH_2_SYSTEM_FEATURE(f,fe)	( uint64 )( ( ( uint64 )(f) << 32 ) | ( ( uint64 )(fe) ) )
#define VOIP_SYSTEM_2_FLASH_FEATURE(f)		( uint32 )( ( uint64 )( f ) >> 32 )
#define VOIP_SYSTEM_2_FLASH_EXT_FEATURE(f)	( uint32 )( ( uint64 )( f ) & 0xFFFFFFFFUL )

//------------------------------------------------------------------
//			VOIP FLASH Control
//------------------------------------------------------------------
#define VOIP_PATHNAME					"/bin/solar"	// use for ftok
#define VOIP_SEM_EVENT					0				// number in sem_set = 0
#define VOIP_SEM_MUTEX					1				// number in sem_set = 1
#define VOIP_SHARE_NOPORTS_SIZE (sizeof(voip_flash_share_t) - sizeof(voipCfgPortParam_t) * VOIP_PORTS)
#define VOIP_SHARE_SIZE(n) (VOIP_SHARE_NOPORTS_SIZE + sizeof(voipCfgPortParam_t) * (n))

//------------------------------------------------------------------
//			VOIP FLASH STRUCT
//------------------------------------------------------------------

#define	VOIP_PORTS				CON_CH_NUM			///< channel number
#define	MAX_VOIP_PORTS			16	//PCM_CH_NUM			///< max channel numbera

#define DNS_LEN					40		///< max DNS length
#define FW_LEN					40		///< max forward URI length
#define PASSWD_LEN				10		///< max passwd length
#define	DEF_SIP_PROXY_PORT		5060
#define	DEF_SIP_LOCAL_PORT		5060
#define	DEF_RTP_PORT			9000
#define	DEF_OUTBOUND_PROXY_PORT		5060
#define	DEF_STUN_SERVER_PORT		  3478
#define FUNC_KEY_LENGTH			3		///< max function key length
#define MAX_SPEED_DIAL			10		///< 0..9
#define MAX_SPEED_DIAL_NAME		11
#define MAX_SPEED_DIAL_URL		61
#define MAX_AUTO_CONFIG_PATH	61

#define MAX_FW_UPDATE_PATH		61
#define MAX_FW_UPDATE_FILE_PREFIX	61
#define MAX_FW_UPDATE_FTP		20
#define MAX_FW_VERSION			20

#define MAX_REPLACE_RULE_SOURCE	80
#define MAX_REPLACE_RULE_TARGET	10
#define MAX_DIALPLAN_LENGTH		80
#define MAX_AUTO_PREFIX			5
#define MAX_PREFIX_UNSET_PLAN	80
#define DEF_T38_PORT			(DEF_RTP_PORT + VOIP_PORTS * 4)
#define MAX_PROXY				2
#define MAX_ABBR_DIAL_NUM		5
#define MAX_ABBR_DIAL_NAME		5
#define MAX_ABBR_DIAL_URL		61
#define PSTN_ROUTING_PREFIX_LEN	128

// proxy types
enum __PROXY_ENABLE__
{
	PROXY_DISABLED = 0,
	PROXY_ENABLED = 0x0001,
	PROXY_NORTEL = 0x0002,
};

/**
 * @ingroup VOIP_RTP_SESSION
 * Enumeration for codec type
 */
enum __CODEC_TYPE__
{
	_CODEC_NOTSUPPORT = -1,
	_CODEC_G711U = 0,
	_CODEC_G711A,
	_CODEC_G729,
	_CODEC_G723,
	_CODEC_G726_16,
	_CODEC_G726_24,
	_CODEC_G726_32,
	_CODEC_G726_40,
	_CODEC_GSMFR,
	_CODEC_ILBC,
	_CODEC_G722,
	_CODEC_SPEEX_NB,
	_CODEC_G711U_WB,
	_CODEC_G711A_WB,
	_CODEC_MAX
};

enum
{
	SUPPORTED_CODEC_G711U = 0,
	SUPPORTED_CODEC_G711A,
#ifdef CONFIG_RTK_VOIP_G729AB
	SUPPORTED_CODEC_G729,
#endif
#ifdef CONFIG_RTK_VOIP_G7231
	SUPPORTED_CODEC_G723,
#endif
#ifdef CONFIG_RTK_VOIP_G726
	SUPPORTED_CODEC_G726_16,
	SUPPORTED_CODEC_G726_24,
	SUPPORTED_CODEC_G726_32,
	SUPPORTED_CODEC_G726_40,
#endif
#ifdef CONFIG_RTK_VOIP_GSMFR
	SUPPORTED_CODEC_GSMFR,
#endif
#ifdef CONFIG_RTK_VOIP_ILBC
	SUPPORTED_CODEC_ILBC,
#endif
#ifdef CONFIG_RTK_VOIP_G722
	SUPPORTED_CODEC_G722,
#endif
#ifdef CONFIG_RTK_VOIP_SPEEX_NB
	SUPPORTED_CODEC_SPEEX_NB,
#endif
#ifdef CONFIG_RTK_VOIP_G7111
	SUPPORTED_CODEC_G711U_WB,
	SUPPORTED_CODEC_G711A_WB,
#endif
	SUPPORTED_CODEC_MAX
};

/**
 * @ingroup VOIP_RTP_SESSION
 * Enumeration for G723 type
 */
enum __G7231_RATE__
{
	G7231_RATE63 = 0,
	G7231_RATE53,
};

/**
 * @ingroup VOIP_RTP_SESSION
 * Enumeration for G726 packing type
 */
enum __G726_PACKING__
{
	G726_PACK_NONE = 0,
	G726_PACK_LEFT,
	G726_PACK_RIGHT,
	G726_PACK_MAX
};

/**
 * @ingroup VOIP_RTP_SESSION
 * Enumeration for iLBC mode
 */
enum __ILBC_MODE_
{
	ILBC_30MS = 0,
	ILBC_20MS
};

/**
 * @ingroup VOIP_RTP_SESSION
 * Enumeration for speex nb rate
 */
enum __SPEEX_NB_RATE__
{
	SPEEX_RATE2P15 = 0,
	SPEEX_RATE5P95,
	SPEEX_RATE8,
	SPEEX_RATE11,
	SPEEX_RATE15,
	SPEEX_RATE18P2,
	SPEEX_RATE24P6,
	SPEEX_RATE3P95
};

/**
 * @ingroup VOIP_RTP_SESSION
 * Enumeration for G711-WB mode
 */
enum __G7111_MODE__
{
	G7111_R1 = 1,
	G7111_R2A,
	G7111_R2B,
	G7111_R3,
	G7111_MODES = G7111_R3
};

/**
 * @ingroup VOIP_RTP_DTMF
 * Enumeration for DTMF type
 */
enum __DTMF_TYPE__
{
	DTMF_RFC2833 = 0,
	DTMF_SIPINFO,
	DTMF_INBAND,
	DTMF_DELETE,
	DTMF_MAX
};

/**
 * @ingroup VOIP_PHONE_CALLERID
 * Enumeration for CID area
 */
enum _CID_AREA_
{
    CID_FSK_BELLCORE = 0,
    CID_FSK_ETSI,
    CID_FSK_BT,
    CID_FSK_NTT,
    CID_DTMF,
    CID_MAX
};

/**
 * @ingroup VOIP_PHONE_CALLERID
 * Enumeration for CID DTMF start/end digit
 */
enum _CID_DTMF_
{
    CID_DTMF_A = 0,
    CID_DTMF_B,
    CID_DTMF_C,
    CID_DTMF_D,
    CID_DTMF_MAX
};

enum _FAX_MODEM_DET_CFG_
{
	FAX_MODEM_DET_AUTO = 0,
	FAX_MODEM_DET_FAX,
	FAX_MODEM_DET_MODEM,
	FAX_MODEM_DET_AUTO2,
	FAX_MODEM_DET_MAX
};

/**
 * @ingroup VOIP_PHONE_TONE
 * Enumeration for Tone Country
 */
enum _TONE_COUNTRY_
{
    TONE_USA = 0,
    TONE_UK,
    TONE_AUSTRALIA,
    TONE_HK,
    TONE_JAPAN,
    TONE_SWEDEN,
    TONE_GERMANY,
    TONE_FRANCE,
#if 0
    TONE_TR57,
#else
    TONE_TW,
#endif
    TONE_BELGIUM,
    TONE_FINLAND,
    TONE_ITALY,
    TONE_CHINA,
    TONE_EXT1,
    TONE_EXT2,
    TONE_EXT3,
    TONE_EXT4,
#ifdef COUNTRY_TONE_RESERVED
    TONE_RESERVE,
#endif
    TONE_CUSTOMER,
    TONE_MAX
};

/**
 * @ingroup VOIP_PHONE_TONE
 * Enumeration for Tone Type
 */
enum _TONE_TYPE_
{
    TONE_TYPE_ADDITIVE = 0,
    TONE_TYPE_MODULATED,
    TONE_TYPE_SUCC,
    //TONE_TYPE_SUCC_ADD,
    TONE_TYPE_MAX
};

/**
 * @ingroup VOIP_PHONE_TONE
 * Enumeration for Tone Cycle
 */
enum _TONE_CYCLE_
{
    TONE_CYCLE_CONTINUOUS = 0,
    TONE_CYCLE_BURST,
    TONE_CYCLE_CADENCE,
    TONE_CYCLE_MAX
};

/*
 * @ingroup VOIP_CONFIG
 * Enumeration for Tone Customer
 */
enum _TONE_CUSTOMER_
{
    TONE_CUSTOMER_1 = 0,
    TONE_CUSTOMER_2,
    TONE_CUSTOMER_3,
    TONE_CUSTOMER_4,
    TONE_CUSTOMER_5,
    TONE_CUSTOMER_6,
    TONE_CUSTOMER_7,
    TONE_CUSTOMER_8,
    TONE_CUSTOMER_MAX
};


/*
 * @ingroup VOIP_CONFIG
 * Enumeration for Disconnect Tone Number
 */
enum _DIS_CONNECT_TONE_NUM_
{
    DIS_CONNECT_TONE_0 = 0,
    DIS_CONNECT_TONE_1,
    DIS_CONNECT_TONE_2,
    DIS_CONNECT_TONE_MAX
};

/*
 * @ingroup VOIP_CONFIG
 * Enumeration for Disconnect Tone Frequency Number
 */
enum _DIS_CONNECT_TONE_FREQ_NUM_
{
    DIS_CONNECT_TONE_FREQ_0 = 0,
    DIS_CONNECT_TONE_FREQ_1,
    DIS_CONNECT_TONE_FREQ_2,
    DIS_CONNECT_TONE_FREQ_MAX
};


/*
 * @ingroup VOIP_CONFIG
 * Enumeration for Ring Group
 */
enum _RING_GROUP_
{
    RING_GROUP_1 = 0,
    RING_GROUP_2,
    RING_GROUP_3,
    RING_GROUP_4,
    RING_GROUP_MAX
};


/*
 * @ingroup VOIP_CONFIG
 * Enumeration for Ring Customer
 */
enum _RING_CADENCE_
{
    RING_CADENCE_1 = 0,
    RING_CADENCE_2,
    RING_CADENCE_3,
    RING_CADENCE_4,
    RING_CADENCE_5,
    RING_CADENCE_6,
    RING_CADENCE_7,
    RING_CADENCE_8,
    RING_CADENCE_MAX
};

/*
 * @ingroup VOIP_CONFIG
 * Enumeration for Pulse Dial Generation PPS
 */
enum _PULSE_GEN_PPS_
{
    PULSE_GEN_PPS_10 = 0,
    PULSE_GEN_PPS_20,
    PULSE_GEN_PPS_MAX
};

/*
 * @ingroup VOIP_CONFIG
 * Enumeration for Pulse Dial Generation Make Duration
 */
enum _PULSE_GEN_MAKE_
{
    PULSE_GEN_MAKE_10MSEC = 0,
    PULSE_GEN_MAKE_20MSEC,
    PULSE_GEN_MAKE_30MSEC,
    PULSE_GEN_MAKE_40MSEC,
    PULSE_GEN_MAKE_50MSEC,
    PULSE_GEN_MAKE_60MSEC,
    PULSE_GEN_MAKE_70MSEC,
    PULSE_GEN_MAKE_80MSEC,
    PULSE_GEN_MAKE_90MSEC,
    PULSE_GEN_MAKE_MAX
};

typedef enum {
	NET_DHCP_DISABLED = 0,
	NET_DHCP_CLIENT
} net_dhcp_t;

typedef enum {
	NET_CFG_DHCP,
	NET_CFG_IP,
	NET_CFG_NETMASK,
	NET_CFG_GATEWAY,
	NET_CFG_DNS,
} net_cfg_t;

/**
 * @ingroup VOIP_CONFIG
 * Enumeration for Voice QoS TOS/DSCP Class
 */
enum _DSCP_CS_
{
    DSCP_CS0 = 0,
    DSCP_CS1,
    DSCP_CS2,
    DSCP_CS3,
    DSCP_CS4,
    DSCP_CS5,
    DSCP_CS6,
    DSCP_CS7,
    DSCP_EF,
    DSCP_MAX
};

/*
 * Auto Dial:
 *	- Auto Dial Time: 0x0001~0x000f (1~15 sec)
 *	- AUTO_DIAL_ALWAYS (Not check HASH) mask: 0x0010
 */
enum {
	AUTO_DIAL_DISABLE	= 0x0000,
	AUTO_DIAL_TIME		= 0x000f,
	AUTO_DIAL_ALWAYS	= 0x0010
};

/* +++++Add by Jack for VoIP security 240108+++++ */
//#ifdef CONFIG_RTK_VOIP_SRTP
enum{
	KEY_EXCHANGE_SDES,
	KEY_EXCHANGE_MIKEY
};
//#endif /* CONFIG_RTK_VOIP_SRTP */
/*-----end-----*/

/*
 * Auto bypass relay
 */
enum {
	AUTO_BYPASS_RELAY_ENABLE	= 0x01,
	AUTO_BYPASS_WARNING_ENABLE	= 0x10
};

#pragma pack(push, 1)

typedef struct ToneCfgParam	st_ToneCfgParam;
typedef struct SpeedDialCfg_s SpeedDialCfg_t;
typedef struct voipCfgProxy_s voipCfgProxy_t;
typedef struct voipCfgPortParam_s voipCfgPortParam_t;
typedef struct voipCfgParam_s voipCfgParam_t;

/*
 * @ingroup VOIP_PHONE_RING
 * Enumeration for Tone Customize
 */

struct ToneCfgParam
{
#if 1
#define MIB_VOIP_TONE_IMPORT
#include "voip_mibdef.h"
#undef MIB_VOIP_TONE_IMPORT
#else
	unsigned long	toneType;
	unsigned short	cycle;

	unsigned short	cadNUM;

	unsigned long	CadOn0;
	unsigned long	CadOn1;
	unsigned long	CadOn2;
	unsigned long	CadOn3;
	unsigned long	CadOff0;
	unsigned long	CadOff1;
	unsigned long	CadOff2;
	unsigned long	CadOff3;

	unsigned long PatternOff;
	unsigned long ToneNUM;

	unsigned long	Freq1;
	unsigned long	Freq2;
	unsigned long	Freq3;
	unsigned long	Freq4;

	long Gain1;
	long Gain2;
	long Gain3;
	long Gain4;
	//int dummy;	//thlin add for sync with struct TstVoipToneCfg in voip_control.h
#endif
};

struct SpeedDialCfg_s
{
#if 1 
#define MIB_VOIP_SPEED_DIAL_IMPORT
#include "voip_mibdef.h"
#undef MIB_VOIP_SPEED_DIAL_IMPORT
#else
	char			name[MAX_SPEED_DIAL_NAME];
	char			url[MAX_SPEED_DIAL_URL];
#endif
};

/*
 * @ingroup VOIP_CONFIG
 * @brief Structure for UI
 */
typedef struct ui_falsh_layout1_s {
	unsigned long	test;
} ui_falsh_layout1_t;

/*
 * @ingroup VOIP_CONFIG
 * @brief Structure for Proxy
 */
struct voipCfgProxy_s
{
#if 1 
#define MIB_VOIP_PROXY_IMPORT
#include "voip_mibdef.h"
#undef MIB_VOIP_PROXY_IMPORT
#else
	// account
	char display_name[DNS_LEN];				///< display name @sa DNS_LEN
	char number[DNS_LEN];					///< username @sa DNS_LEN
	char login_id[DNS_LEN];					///< login id @sa DNS_LEN
	char password[DNS_LEN];					///< password @sa DNS_LEN
	// register server
	unsigned short enable;					///< enable/disable register server setting, see __PROXY_ENABLE__
	char addr[DNS_LEN];						///< register server address
	unsigned short port;					///< default: 5060
	char domain_name[DNS_LEN];				///< register domain name, used in "from", "to" like user@domain
	unsigned int reg_expire;				///< default: 60 sec
	// nat traversal server
	unsigned short outbound_enable;			///< enable/disable outbound proxy setting
	char outbound_addr[DNS_LEN];			///< outbound proxy addr
	unsigned short outbound_port;			///< outbound proxy port
	// sip tls for security
	unsigned short siptls_enable;				///<enable/disable sip tls
#endif
};

/*
 * @ingroup VOIP_CONFIG
 * @brief Structure for Abbreviated Dial
 */
typedef struct abbrDialCfg_s {
#if 1 
#define MIB_VOIP_ABBR_DIAL_IMPORT
#include "voip_mibdef.h"
#undef MIB_VOIP_ABBR_DIAL_IMPORT
#else
	char name[ MAX_ABBR_DIAL_NAME ];
	char url[ MAX_ABBR_DIAL_URL ];
#endif
} abbrDialCfg_t;

/*
 * @ingroup VOIP_CONFIG
 * @brief Structure for sip handling
 */
struct voipCfgPortParam_s
{
#if 1 
#define MIB_VOIP_PORT_IMPORT
#include "voip_mibdef.h"
#undef MIB_VOIP_PORT_IMPORT
#else
	// general
	voipCfgProxy_t proxies[MAX_PROXY];	///< proxy server setting
	unsigned char default_proxy;

	// nat
	unsigned char stun_enable;				///< stun enable
	char stun_addr[DNS_LEN];				///< stun server
	unsigned short stun_port;				///< stun port

	// advanced
	unsigned short	sip_port;				///< default: 5060
	unsigned short	media_port;				///< default: 9000
	unsigned char	dtmf_mode;				///< default: DTMF_INBAND @sa __DTMF_TYPE__
	unsigned short	dtmf_2833_pt;				///< DTMF RFC2833 event payload type in RTP if enale RFC2833
	unsigned short	dtmf_2833_pi;				///< DTMF RFC2833 event packet interval
	unsigned char	fax_modem_2833_pt_same_dtmf;		///< Fax/Modem RFC2833 PT is the same to DTMF or not
	unsigned short	fax_modem_2833_pt;			///< Fax/Modem RFC2833 event payload type in RTP if enale RFC2833
	unsigned short	fax_modem_2833_pi;			///< Fax/Modem RFC2833 event packet interval
	unsigned short	sip_info_duration;			///< sip info duration if enable SIP Info
	unsigned char	call_waiting_enable;			///< call waiting option
	unsigned char 	direct_ip_call;				///< 0: reject direct ip call, 1: accept direct ip call

	// forward
	unsigned char	uc_forward_enable;			///< unconditional forward option
	char		uc_forward[FW_LEN];			///< unconditional forward address
	unsigned char	busy_forward_enable;			///< busy forward option
	char		busy_forward[FW_LEN];			///< busy forward address
	unsigned char	na_forward_enable;			///< no answer forward option
	char		na_forward[FW_LEN];			///< no answer forward address
	unsigned short	na_forward_time;			///< no answer forward time setting

	// speed dial
	SpeedDialCfg_t	speed_dial[MAX_SPEED_DIAL];

	// dial plan
	unsigned char	replace_rule_option;							///< replace rule option
	unsigned char	replace_rule_source[ MAX_REPLACE_RULE_SOURCE ];	///< replace rule source plan
	unsigned char	replace_rule_target[ MAX_REPLACE_RULE_TARGET ];	///< replace rule target
	unsigned char	dialplan[MAX_DIALPLAN_LENGTH];					///< dialplan
	unsigned char 	auto_prefix[ MAX_AUTO_PREFIX ];					///< auto prefix
	unsigned char 	prefix_unset_plan[ MAX_PREFIX_UNSET_PLAN ];		///< prefix unset plan

	// codec
	unsigned char 	frame_size[_CODEC_MAX];			///< 0: 1 frame per packet
	unsigned char	precedence[_CODEC_MAX];			///< default: 2, 0, 1, 3 (G729,G711u,G711a,G723) @sa __CODEC_TYPE__
	unsigned char 	vad;					///< 0: disable 1: enable
	unsigned char 	vad_thr;				///< vad threshold
	unsigned char 	cng;					///< 0: disable 1: enable
	unsigned char 	cng_thr;				///< cng threshold
	unsigned char 	sid_gainmode;				///< SID noise gain mode, default: 0(disable configurable), 1(fixed noise gain), 2(based on CN payload)
	unsigned char 	sid_noiselevel;				///< SID noise level value, default: 70(-70dBov), 0~127( 0dB ~ -127dBov)
	signed char 	sid_noisegain;				///< SID noise gain value, default: 0(0dB), -127~127 (negative is gain down and vice versa)
	unsigned char 	PLC;					///< 0: disable 1: enable
	unsigned char	RTCP_Interval;			///< 0: disable others: interval (second)
	unsigned char 	RTCP_XR;				///< 0: disable 1: enable
	unsigned char	g7231_rate;				///< 5.3k or 6.3k
	unsigned char	iLBC_mode;				///< 30ms or 20ms
	unsigned char	speex_nb_rate;				///< 2.15k, 5.95k, 8k, 11k, 15k, 18.2k, 24.6k or 3.95k
	unsigned char	g726_packing;				///< 0: none, 1: packing left, 2: packing right
	unsigned char	g7111_precedence[G7111_MODES];	///< default: R3, R2B, R2A, R1

	// RTP redundant
	unsigned char	rtp_redundant_payload_type;			// 96~127
	signed char		rtp_redundant_codec;				///< -1: none, 0: u-law, 8: a-law, 18: G.729

	// DSP
	unsigned char	slic_txVolumne;				///< 0..9
	unsigned char	slic_rxVolumne;				///< 0..9
	unsigned char	jitter_delay;			///< jitter delay in unit of 10ms
	unsigned char	maxDelay;				///< jitter-buffer max delay 5~10 : 50~100 ms, def:9
	unsigned char	jitter_factor;			///< jitter factor, 1: optimize to delay, 12: optimize to quality, 13: fix delay for FAX
	unsigned char 	lec;					///< LEC on/off switch, 0: disable 1: enable
	unsigned char 	nlp;					///< NLP on/off switch, 0: disable 1: enable
	unsigned char	echoTail;				///< echo-tail 0~5 : 1, 2, 4, 8, 16, 32 ms, def:3
	unsigned char	caller_id_mode;				///< DTMF(1) or FSK(0)
	unsigned char	call_waiting_cid;			///< use call waiting caller ID or not.0: disable 1: enable
	unsigned char	cid_dtmf_mode;				///< configure the DTMF caller id start/end digit.
	unsigned char	speaker_agc;				///< 0: disable 1: enable
	unsigned char	spk_agc_lvl;				///< 0..8
	unsigned char	spk_agc_gu;				///< 0..8
	unsigned char	spk_agc_gd;				///< 0..8
	unsigned char	mic_agc;				///< 0: disable 1: enable
	unsigned char	mic_agc_lvl;				///< 0..8
	unsigned char	mic_agc_gu;				///< 0..8
	unsigned char	mic_agc_gd;				///< 0..8
	unsigned char	cid_fsk_gen_mode;			///< 0:hardware, 1: software dsp gen
	char		spk_voice_gain;				///< -32~31, Mute:-32,
	char		mic_voice_gain;				///< -32~31, Mute:-32,
	unsigned int	anstone;				///< answer tone detect config
	unsigned char	faxmodem_rfc2833;			///< fax/modem rfc2833 support config

	// QoS
	unsigned char	voice_qos;

	//T.38
	unsigned char useT38;					//enable T.38
	unsigned short T38_port;				//T.38 port
	unsigned char fax_modem_det;				// fax_modem_det config

	//T.38 parameters
	unsigned char	T38ParamEnable;			///< enable T.38 parameters customization
	unsigned short	T38MaxBuffer;			///< default: 500. 200~600.
	unsigned char	T38RateMgt;				///< default: 2. 1: local TCF, 2: remote TCF
	unsigned char	T38MaxRate;				///< default: 5. 0~5: 2400, 4800, 7200, 9600, 12000, 14400
	unsigned char	T38EnableECM;			///< default: 1. 1: enable, 0: disable
	unsigned char	T38ECCSignal;			///< default: 5. 0~7
	unsigned char	T38ECCData;				///< default: 2. 0~2 
	unsigned char	T38EnableSpoof;			///< default: 1. 1: enable, 0: disable 
	unsigned char	T38DuplicateNum;				///< default: 0. 0~2 

	// V.152
	unsigned char	useV152;
	unsigned char	v152_payload_type;		// 96~127
	unsigned char	v152_codec_type;		// 0 (u-law) or 8 (a-law)

	// Hotline
	unsigned char 	hotline_enable;				// 0: disable, 1: enable
	char			hotline_number[DNS_LEN];

	// DND
	unsigned char	dnd_mode;			// 0: disable, 1: dnd period, 2: dnd_always
	unsigned char	dnd_from_hour;		// 0..23
	unsigned char	dnd_from_min;		// 0..59
	unsigned char	dnd_to_hour;		// 0..23
	unsigned char	dnd_to_min;			// 0..59

	// flash-hook time
	unsigned short	flash_hook_time;
	unsigned short	flash_hook_time_min;

	/* +++++Add by Jack for VoIP security 240108+++++ */
	// Security
//#ifdef CONFIG_RTK_VOIP_SRTP
	unsigned char security_enable;	//0:disable, 1:enable
	unsigned char	key_exchange_mode; // SDES, MIKEY
//#endif
	/*-----end-----*/

	// auth
	char offhook_passwd[PASSWD_LEN];	///< do authentication when off-hook

	// abbreviated dial (phonebook)
	abbrDialCfg_t	abbr_dial[ MAX_ABBR_DIAL_NUM ];

	// alarm
	unsigned char 	alarm_enable;
	unsigned char 	alarm_time_hh;
	unsigned char 	alarm_time_mm;
#if 0
	unsigned char 	alarm_ring_last_day;
	unsigned char	alarm_ring_defer;
#endif

	// PSTN Routing Prefix
	unsigned char	PSTN_routing_prefix[ PSTN_ROUTING_PREFIX_LEN ];
#endif
};

struct voipCfgParam_s {

#if 1 
#define MIB_VOIP_CFG_IMPORT
#include "voip_mibdef.h"
#undef MIB_VOIP_CFG_IMPORT
#else
	// voip flash check
	unsigned long	signature;				// voip flash check
	unsigned short	version;				// update if big change (size/offset not match..)
	unsigned long	feature;				// 32 bit feature
	unsigned long	extend_feature;			// 32 extend feature

	unsigned short mib_version; // update if some mib name change  (ex: VOIP.PORT[0].NUMBER => VOIP.PORT[0].USER.NUMBER)
								// used in import/export flash setting
								// if mib version not match, then import will fail

	// RFC flags
	unsigned int	rfc_flags;				///< RFC flags

	//tone			///< unit: 10ms
	unsigned char 	tone_of_country;
	unsigned char 	tone_of_custdial;
	unsigned char 	tone_of_custring;
	unsigned char 	tone_of_custbusy;
	unsigned char 	tone_of_custwaiting;
	unsigned char 	tone_of_customize;
	st_ToneCfgParam	cust_tone_para[TONE_CUSTOMER_MAX];			// for customized tone (up to 8 tones)

	//disconnect tone det
	unsigned char	distone_num;
	unsigned char	d1freqnum;
	unsigned short	d1Freq1;
	unsigned short	d1Freq2;
	unsigned char	d1Accur;
	unsigned short	d1Level;
	unsigned short	d1ONup;
	unsigned short	d1ONlow;
	unsigned short	d1OFFup;
	unsigned short	d1OFFlow;
	unsigned char	d2freqnum;
	unsigned short	d2Freq1;
	unsigned short	d2Freq2;
	unsigned char	d2Accur;
	unsigned short	d2Level;
	unsigned short	d2ONup;
	unsigned short	d2ONlow;
	unsigned short	d2OFFup;
	unsigned short	d2OFFlow;

	//ring
	unsigned char	ring_cad;
	unsigned char	ring_group;
	unsigned int	ring_phone_num[RING_GROUP_MAX];
	unsigned char	ring_cadence_use[RING_GROUP_MAX];
	unsigned char	ring_cadence_sel;
	unsigned short	ring_cadon[RING_CADENCE_MAX];
	unsigned short	ring_cadoff[RING_CADENCE_MAX];

	// function key
	char			funckey_pstn[FUNC_KEY_LENGTH];				// default is *0
	char			funckey_transfer[FUNC_KEY_LENGTH];			// default is *1

	// other
	unsigned short	auto_dial;									// 0, 3~9 sec, default is 5 sec, 0 is disable
	unsigned short	off_hook_alarm;								// 0, 10~60 sec, default is 30 sec, 0 is disable
	unsigned short  cid_auto_det_select;						// caller id auto detection selection
	unsigned short	caller_id_det_mode;							// caller id detection mode
	unsigned char	one_stage_dial;								// 0: disable
																// 1: enable one stage dial
																//		- VoIP -> FXO -> PSTN
																//		- PSTN number is assigned by "To" filed
	unsigned char	two_stage_dial;								// 0: disable
																// 1: enable two stage dial
																//		- VoIP -> FXO -> IVR prompt -> PSTN
	unsigned char	auto_bypass_relay;							// bit0: enable/disable bypass relay
																//		- SIP Register failed
																//		- TODO: Network failed
																// bit1: enable/disable warning tone

	// pulse dial
	unsigned char	pulse_dial_gen;								// 0: disable, 1: enable pulse dial generation for FXO port
	unsigned char	pulse_gen_pps;
	unsigned char	pulse_gen_make_time;
	unsigned short	pulse_gen_interdigit_pause;
	unsigned char	pulse_dial_det;								// 0: disable, 1: enable pulse dial detection for FXS port
	unsigned short	pulse_det_pause;

	// auto config
	unsigned short	auto_cfg_ver;								// load config if version is newer
	unsigned char	auto_cfg_mode;								// 0: disable, 1: http
	char			auto_cfg_http_addr[DNS_LEN];
	unsigned short	auto_cfg_http_port;
/*+++++added by Jack for auto provision for tftp and ftp+++++*/
	char			auto_cfg_tftp_addr[DNS_LEN];
	char 			auto_cfg_ftp_addr[DNS_LEN];
	char			auto_cfg_ftp_user[20];
	char 			auto_cfg_ftp_passwd[20];
/*-----end-----*/
	char			auto_cfg_file_path[MAX_AUTO_CONFIG_PATH];
	unsigned short	auto_cfg_expire;							// 0..365 days

	// fw update setting				// load config if version is newer
	unsigned char	fw_update_mode;			// 0: disable, 1: TFTP 2: FTP 3: HTTP
	char		fw_update_tftp_addr[DNS_LEN];
	char		fw_update_http_addr[DNS_LEN];
	unsigned short	fw_update_http_port;

	char		fw_update_ftp_addr[DNS_LEN];
	char		fw_update_ftp_user[MAX_FW_UPDATE_FTP];
	char		fw_update_ftp_passwd[MAX_FW_UPDATE_FTP];
	char		fw_update_file_path[MAX_FW_UPDATE_PATH];

	unsigned char	fw_update_power_on;
	unsigned short	fw_update_scheduling_day;
	unsigned char	fw_update_scheduling_time;
	unsigned char	fw_update_auto;

	char		fw_update_file_prefix[MAX_FW_UPDATE_FILE_PREFIX];

	unsigned long	fw_update_next_time;
	char		fw_update_fw_version[MAX_FW_VERSION];

	// VLAN setting of WAN Port
	unsigned char	wanVlanEnable;
	// VLAN for Voice and protocl packets (SIP, STUN, DHCP, ARP, etc.)
	unsigned short	wanVlanIdVoice;
	unsigned char	wanVlanPriorityVoice;
	unsigned char	wanVlanCfiVoice;
	// VLAN for Data
	unsigned short	wanVlanIdData;
	unsigned char	wanVlanPriorityData;
	unsigned char	wanVlanCfiData;
	// VLAN for Video
	unsigned short	wanVlanIdVideo;
	unsigned char	wanVlanPriorityVideo;
	unsigned char	wanVlanCfiVideo;


	//HW VLAN
	// VLAN setting of WAN Port
	unsigned char   vlan_enable;
	unsigned short  vlan_tag;

	unsigned char   vlan_bridge_enable;
	unsigned short  vlan_bridge_tag;
	unsigned short  vlan_bridge_port;
	unsigned char  vlan_bridge_multicast_enable;
	unsigned short  vlan_bridge_multicast_tag;

	unsigned char vlan_host_enable;
	unsigned short vlan_host_tag;
	unsigned short vlan_host_pri;
	unsigned char vlan_wifi_enable;
	unsigned short vlan_wifi_tag;
	unsigned short vlan_wifi_pri;
	unsigned char vlan_wifi_vap0_enable;
	unsigned short vlan_wifi_vap0_tag;
	unsigned short vlan_wifi_vap0_pri;
	unsigned char vlan_wifi_vap1_enable;
	unsigned short vlan_wifi_vap1_tag;
	unsigned short vlan_wifi_vap1_pri;
	unsigned char vlan_wifi_vap2_enable;
	unsigned short vlan_wifi_vap2_tag;
	unsigned short vlan_wifi_vap2_pri;
	unsigned char vlan_wifi_vap3_enable;
	unsigned short vlan_wifi_vap3_tag;
	unsigned short vlan_wifi_vap3_pri;



	// enable HW-NAT
	unsigned char   hwnat_enable;

	// bandwidth Mgr
	unsigned short	bandwidth_LANPort0_Egress;
	unsigned short	bandwidth_LANPort1_Egress;
	unsigned short	bandwidth_LANPort2_Egress;
	unsigned short	bandwidth_LANPort3_Egress;
	unsigned short	bandwidth_WANPort_Egress;

	unsigned short	bandwidth_LANPort0_Ingress;
	unsigned short	bandwidth_LANPort1_Ingress;
	unsigned short	bandwidth_LANPort2_Ingress;
	unsigned short	bandwidth_LANPort3_Ingress;
	unsigned short	bandwidth_WANPort_Ingress;

	// FXO valume
	unsigned char	daa_txVolumne;				///< 1..10
	unsigned char	daa_rxVolumne;				///< 1..10

	// DSCP
	unsigned char	rtpDscp;
	unsigned char	sipDscp;

	voipCfgPortParam_t ports[VOIP_PORTS];

	// UI
#ifdef CONFIG_RTK_VOIP_IP_PHONE
	ui_falsh_layout_t ui;
#endif

#endif // upper was excluded

};

//
// config header
//
// valid import by config header check:
// 	1. mib_version have to match (mib name ok)
//	2. feature and extend feature match, or
//     current(flash) version == config(file) version
//	   (It will skip the not match part on import).
//
// config file struct:
// | ------------------- |
// | config file header  |
// | ------------------- |
// | voip encode config  |
// | ------------------- |
// | checksum (1 byte)   |
// | ------------------- |
typedef struct VOIP_CFG_HEADER_S VOIP_CFG_HEADER;

struct VOIP_CFG_HEADER_S {
    unsigned long signature; // config file signature
	unsigned short len;
};

typedef struct flash_netcfg_s {
	int dhcp;
	unsigned long ip;
	unsigned long netmask;
	unsigned long gateway;
	unsigned long dns;
	int ivr_lan;
	char ivr_interface[10];
	char ivr_lan_interface[ 10 ];
} flash_netcfg_t;

typedef struct voip_flash_share_s {
	flash_netcfg_t net_cfg;
	voipCfgParam_t voip_cfg;
} voip_flash_share_t;

typedef struct {
	int mode;
	voipCfgParam_t current_setting;
	voipCfgParam_t default_setting;
} voipCfgAll_t;

#pragma pack(pop)

//------------------------------------------------------------------
//			VOIP FLASH Write
//------------------------------------------------------------------
typedef enum {
	VOIP_FLASH_WRITE_CLIENT_IPPHONE,
	VOIP_FLASH_WRITE_CLIENT_TR104,
	//VOIP_FLASH_WRITE_CLIENT_RESERVE_0,	/* other application can use these reserved IDs */
	//VOIP_FLASH_WRITE_CLIENT_RESERVE_1,
	//VOIP_FLASH_WRITE_CLIENT_RESERVE_2,
	//VOIP_FLASH_WRITE_CLIENT_RESERVE_3,
	MAX_VOIP_FLASH_WRITE_CLIENT,
	/* If a client doesn't use writing function, let it to be a invalid ID. */
	VOIP_FLASH_WRITE_CLIENT_SOLAR = MAX_VOIP_FLASH_WRITE_CLIENT,
	VOIP_FLASH_WRITE_CLIENT_IVRSERVER = MAX_VOIP_FLASH_WRITE_CLIENT,
	VOIP_FLASH_WRITE_CLIENT_FWUPDATE = MAX_VOIP_FLASH_WRITE_CLIENT,
} cid_t;

// ---------------------------------------------------------------
// Flash Interface
// ---------------------------------------------------------------

int flash_voip_default(voipCfgParam_t *pVoIPCfg);

int flash_voip_check(voipCfgParam_t *pVoIPCfg);

int flash_voip_cmd(int param_cnt, char *param_var[]);

/**
 * @ingroup VOIP_FLASH
 * @brief get voip configurations from flash
 * @param cfg_all The voip configuration.
 * @param mode used in 8186 only (VOIP_CURRENT_SETTING or VOIP_DEFAULT_SETTING)
 * @retval 0 Success
 */
int voip_flash_read(voipCfgAll_t *cfg_all, int mode);

/**
 * @ingroup VOIP_FLASH
 * @brief set voip configurations to flash
 * @param cfg_all The voip configuration.
 * @retval 0 Success
 */
int voip_flash_write(voipCfgAll_t *cfg_all);

/**
 * @ingroup VOIP_FLASH
 * @brief get voip configuration from temporary memory
 * @param ppVoIPCfg The voip configuration.
 * @retval 0 Success
 * @note It is workable after voip_flash_read or web has started
 */
int voip_flash_get(voipCfgParam_t **ppVoIPCfg);
int voip_flash_get_default(voipCfgParam_t **ppVoIPCfg);

/**
 * @ingroup VOIP_FLASH
 * @brief set voip configuration via temporary memory
 * @param pVoIPCfg The voip configuration.
 * @retval 0 Success
 */
int voip_flash_set(voipCfgParam_t *pVoIPCfg);

int voip_flash_server_read(voip_flash_share_t *pVoIPShare);
int voip_flash_server_write(voip_flash_share_t *pVoIPShare);

int voip_flash_server_start(void);
void voip_flash_server_stop(void);
int voip_flash_server_update(void);

int voip_flash_client_init(voip_flash_share_t **ppVoIPShare, cid_t cid);
void voip_flash_client_close(void);
int voip_flash_client_update(void);

extern unsigned char GetDhcpValueToSetFixedIP( void );
extern int net_cfg_flash_read( net_cfg_t cfg, void *pCfgData );
extern int GetGatewayOperationMode( void );

// ---------------------------------------------------------------
// Flash Variables
// ---------------------------------------------------------------

extern voipCfgParam_t voipCfgParamDefault;
#ifdef CONFIG_RTK_VOIP_PACKAGE_867X
extern voipCfgParam_t VoipEntry;
#endif

#endif
