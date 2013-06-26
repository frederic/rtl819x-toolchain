/*
 * voip_flash_mib.h: VoIP MIB Header
 *
 * Authors: Rock <shaofu@realtek.com.tw>
 *
 */

#ifndef __VOIP_FLASH_MIB_H
#define __VOIP_FLASH_MIB_H

/* Use MIB_VOIP_TABLE_LIST to inform init procedure that it is a table when loading from tlv to bin file. */
#ifdef VOIP_SUPPORT_TLV_CFG
#define MIB_VOIP_TABLE_LIST 0x8000
#else
#define MIB_VOIP_TABLE_LIST 0x0
#endif

// voip flash info
#define MIB_VOIP_SIGNATURE				1
#define MIB_VOIP_VERSION				2
#define MIB_VOIP_FEATURE				3
#define MIB_VOIP_EXTEND_FEATURE			4

// port setting
#define MIB_VOIP_PORT					(5|MIB_VOIP_TABLE_LIST)

// proxy setting - account
#define MIB_VOIP_PROXY_DISPLAY_NAME		6
#define MIB_VOIP_PROXY_NUMBER			7
#define MIB_VOIP_PROXY_LOGIN_ID			8
#define MIB_VOIP_PROXY_PASSWORD			9

// proxy setting - register server
#define MIB_VOIP_PROXY_ENABLE			10
#define MIB_VOIP_PROXY_ADDR				11
#define MIB_VOIP_PROXY_PORT				12
#define MIB_VOIP_PROXY_DOMAIN_NAME		13
#define MIB_VOIP_PROXY_REG_EXPIRE		14

// proxy setting - nat traversal server
#define MIB_VOIP_PROXY_OUTBOUND_ENABLE	15
#define MIB_VOIP_PROXY_OUTBOUND_ADDR	16
#define MIB_VOIP_PROXY_OUTBOUND_PORT	17

// NAT Traversal
#define MIB_VOIP_STUN_ENABLE			18
#define MIB_VOIP_STUN_ADDR				19
#define MIB_VOIP_STUN_PORT				20

// port setting - advanced
#define MIB_VOIP_SIP_PORT				21
#define MIB_VOIP_MEDIA_PORT				22
#define MIB_VOIP_DTMF_MODE				23
#define MIB_VOIP_DTMF_RFC2833_PAYLOAD_TYPE		24
#define MIB_VOIP_CALL_WAITING_ENABLE	25

// port setting - forward
#define MIB_VOIP_UC_FORWARD_ENABLE		26
#define MIB_VOIP_UC_FORWARD				27
#define MIB_VOIP_BUSY_FORWARD_ENABLE	28
#define MIB_VOIP_BUSY_FORWARD			29
#define MIB_VOIP_NA_FORWARD_ENABLE		30
#define MIB_VOIP_NA_FORWARD				31
#define MIB_VOIP_NA_FORWARD_TIME		32

// port setting - speed dial
#define MIB_VOIP_SPEED_DIAL				(33|MIB_VOIP_TABLE_LIST)
#define MIB_VOIP_SPEED_DIAL_NAME		34
#define MIB_VOIP_SPEED_DIAL_URL			35

// port setting - codec
#define MIB_VOIP_FRAME_SIZE				36
#define MIB_VOIP_PRECEDENCE				37
#define MIB_VOIP_VAD					38
#define MIB_VOIP_G7231_RATE				39

// port setting - DSP
#define MIB_VOIP_SLIC_TX_VOLUME			40
#define MIB_VOIP_SLIC_RX_VOLUME			41
						/*42-43 is used by daa volume*/
#define MIB_VOIP_MAX_DELAY				44
#define MIB_VOIP_ECHO_TAIL				45
#define MIB_VOIP_CALLER_ID_MODE			46
#define MIB_VOIP_CALL_WAITING_CID		47

// flash hook time
#define MIB_VOIP_FLASH_HOOK_TIME		48

// RFC flags
#define MIB_VOIP_RFC_FLAGS				49

// tone
#define MIB_VOIP_TONE_OF_COUNTRY		50
#define MIB_VOIP_TONE_OF_CUSTDIAL		51
#define MIB_VOIP_TONE_OF_CUSTRING		52
#define MIB_VOIP_TONE_OF_CUSTBUSY		53
#define MIB_VOIP_TONE_OF_CUSTWAITING	54
#define MIB_VOIP_TONE_OF_CUSTOMIZE		55

// customize tone
#define MIB_VOIP_CUST_TONE				(56|MIB_VOIP_TABLE_LIST)
#define MIB_VOIP_CUST_TONE_TYPE			57
#define MIB_VOIP_CUST_TONE_CYCLE		58
#define MIB_VOIP_CUST_TONE_CAD_NUM		59
#define MIB_VOIP_CUST_TONE_CAD_ON0		60
#define MIB_VOIP_CUST_TONE_CAD_ON1		61
#define MIB_VOIP_CUST_TONE_CAD_ON2		62
#define MIB_VOIP_CUST_TONE_CAD_ON3		63
#define MIB_VOIP_CUST_TONE_CAD_OFF0		64
#define MIB_VOIP_CUST_TONE_CAD_OFF1		65
#define MIB_VOIP_CUST_TONE_CAD_OFF2		66
#define MIB_VOIP_CUST_TONE_CAD_OFF3		67
#define MIB_VOIP_CUST_TONE_PATTERN_OFF	68
#define MIB_VOIP_CUST_TONE_NUM			69
#define MIB_VOIP_CUST_TONE_FREQ1		70
#define MIB_VOIP_CUST_TONE_FREQ2		71
#define MIB_VOIP_CUST_TONE_FREQ3		72
#define MIB_VOIP_CUST_TONE_FREQ4		73
#define MIB_VOIP_CUST_TONE_GAIN1		74
#define MIB_VOIP_CUST_TONE_GAIN2		75
#define MIB_VOIP_CUST_TONE_GAIN3		76
#define MIB_VOIP_CUST_TONE_GAIN4		77
//#define MIB_VOIP_CUST_TONE_DUMMY		223

// ring
#define MIB_VOIP_RING_CAD				78
#define MIB_VOIP_RING_GROUP				79
#define MIB_VOIP_RING_PHONE_NUM			80
#define MIB_VOIP_RING_CADENCE_USE		81
#define MIB_VOIP_RING_CADENCE_SEL		82
#define MIB_VOIP_RING_CAD_ON			83
#define MIB_VOIP_RING_CAD_OFF			84

// function key
#define MIB_VOIP_FUNCKEY_PSTN			85
#define MIB_VOIP_FUNCKEY_TRANSFER		86

// other
#define MIB_VOIP_AUTO_DIAL			87
#define MIB_VOIP_OFF_HOOK_ALARM			88

// mib version
#define MIB_VOIP_MIB_VERSION			89

// auto config
#define MIB_VOIP_AUTO_CFG_VER			90
#define MIB_VOIP_AUTO_CFG_MODE			91
#define MIB_VOIP_AUTO_CFG_HTTP_ADDR		92
#define MIB_VOIP_AUTO_CFG_HTTP_PORT		93
#define MIB_VOIP_AUTO_CFG_FILE_PATH		94
#define MIB_VOIP_AUTO_CFG_EXPIRE		95

// port setting - DSP
#define MIB_VOIP_JITTER_DELAY			96

#define MIB_VOIP_VOICE_QOS			97

// flash hook min time
#define MIB_VOIP_FLASH_HOOK_TIME_MIN		98

//caller id dtmf start/end digit configruation
#define MIB_VOIP_CID_DTMF_MODE			99

// port setting - DIAL PLAN
#define MIB_VOIP_REPLACE_RULE_OPTION	100
#define MIB_VOIP_REPLACE_RULE_SOURCE	101
#define MIB_VOIP_REPLACE_RULE_TARGET	102
#define MIB_VOIP_DIAL_PLAN              103
#define MIB_VOIP_AUTO_PREFIX			104
#define MIB_VOIP_PREFIX_UNSET_PLAN		105

// sip info duration
#define MIB_VOIP_SIP_INFO_DURATION	      	106

//AGC config
#define MIB_VOIP_SPEAKERAGC			107
#define MIB_VOIP_SPK_AGC_LVL			108
#define MIB_VOIP_SPK_AGC_GU			109
#define MIB_VOIP_SPK_AGC_GD			110
#define MIB_VOIP_MICAGC				111
#define MIB_VOIP_MIC_AGC_LVL			112
#define MIB_VOIP_MIC_AGC_GU			113
#define MIB_VOIP_MIC_AGC_GD			114

#define MIB_VOIP_CALLER_ID_DET_MODE		115
#define MIB_VOIP_CALLER_ID_AUTO_DET_SELECT	116
#define MIB_VOIP_PULSE_DIAL_GENERATE	148
#define MIB_VOIP_PULSE_GEN_PPS			149
#define MIB_VOIP_PULSE_GEN_MAKE_TIME		150
#define MIB_VOIP_PULSE_GEN_INTERDIGIT_PAUSE	151
#define	MIB_VOIP_PULSE_DIAL_DETECT			152
#define MIB_VOIP_PULSE_DET_PAUSE		153
#define MIB_VOIP_DAA_TX_VOLUME			42
#define MIB_VOIP_DAA_RX_VOLUME			43

// VLAN setting
#define MIB_VOIP_WAN_VLAN_ENABLE		117
#define MIB_VOIP_WAN_VLAN_ID_VOICE		118
#define MIB_VOIP_WAN_VLAN_PRIORITY_VOICE	119
#define MIB_VOIP_WAN_VLAN_CFI_VOICE		120

//fsk cid gen mode
#define MIB_VOIP_FSK_GEN_MODE			121

//T.38 config
#define MIB_VOIP_T38_USET38			122
#define MIB_VOIP_T38_PORT			123

//voice gain
#define MIB_VOIP_SPK_VOICE_GAIN			124
#define MIB_VOIP_MIC_VOICE_GAIN			125

// DSCP setting
#define MIB_VOIP_RTP_DSCP			126
#define MIB_VOIP_SIP_DSCP			127

// HOT Line
#define MIB_VOIP_HOTLINE_ENABLE			128
#define MIB_VOIP_HOTLINE_NUMBER			129

// DND
#define MIB_VOIP_DND_MODE			130
#define MIB_VOIP_DND_FROM_HOUR			131
#define MIB_VOIP_DND_FROM_MIN			132
#define MIB_VOIP_DND_TO_HOUR			133
#define MIB_VOIP_DND_TO_MIN			134

// VLAN setting part2
#define MIB_VOIP_WAN_VLAN_ID_DATA		135
#define MIB_VOIP_WAN_VLAN_PRIORITY_DATA		136
#define MIB_VOIP_WAN_VLAN_CFI_DATA		137

// VLAN setting part3
#define MIB_VOIP_WAN_VLAN_ID_VIDEO		138
#define MIB_VOIP_WAN_VLAN_PRIORITY_VIDEO	139
#define MIB_VOIP_WAN_VLAN_CFI_VIDEO		140

// port setting - DSP
#define MIB_VOIP_JITTER_FACTOR			141

// port setting - codec
#define MIB_VOIP_ILBC_MODE			142

// proxy
#define MIB_VOIP_PROXIES			(143|MIB_VOIP_TABLE_LIST)
#define MIB_VOIP_DEFAULT_PROXY			144

// security
#define MIB_VOIP_SECURITY_ENABLE		145
#define MIB_VOIP_TLS_ENABLE			146
#define MIB_VOIP_KEY_EXCHANGE_MODE		147

//fax modem det
#define MIB_VOIP_FAX_MODEM_DET			154

// off hook passwd
#define MIB_VOIP_OFFHOOK_PASSWD			155

// one stage dial
#define MIB_VOIP_ONE_STAGE_DIAL			156

// port setting - abbreviated dial
#define MIB_VOIP_ABBREVIATED_DIAL		(157|MIB_VOIP_TABLE_LIST)
#define MIB_VOIP_ABBREVIATED_DIAL_NAME		158
#define MIB_VOIP_ABBREVIATED_DIAL_URL		159

// port setting - alarm
#define MIB_VOIP_ALARM_ENABLE			160
#define MIB_VOIP_ALARM_TIME_HH			161
#define MIB_VOIP_ALARM_TIME_MM			162
#define MIB_VOIP_ALARM_RING_LAST_DAY		163
#define MIB_VOIP_ALARM_RING_DEFER		164

// port setting - PSTN Routing Prefix
#define MIB_VOIP_PSTN_ROUTING_PREFIX		188

// FwUpdate setting
#define MIB_VOIP_FW_UPDATE_MODE			189
#define MIB_VOIP_FW_UPDATE_TFTP_ADDR		190
#define MIB_VOIP_FW_UPDATE_HTTP_ADDR		191
#define MIB_VOIP_FW_UPDATE_HTTP_PORT		192
#define MIB_VOIP_FW_UPDATE_FTP_ADDR		193
#define MIB_VOIP_FW_UPDATE_FTP_USER		194
#define MIB_VOIP_FW_UPDATE_FTP_PASSWD		195
#define MIB_VOIP_FW_UPDATE_FILE_PATH		196
#define MIB_VOIP_FW_UPDATE_POWER_ON		197
#define MIB_VOIP_FW_UPDATE_SCHEDULING_DAY	198
#define MIB_VOIP_FW_UPDATE_SCHEDULING_TIME	199
#define MIB_VOIP_FW_UPDATE_AUTO			200
#define MIB_VOIP_FW_UPDATE_FILE_PREFIX		201
#define MIB_VOIP_FW_UPDATE_NEXT_TIME		202
#define MIB_VOIP_FW_UPDATE_FW_VERSION		203

// disconnect tone detect
#define	MIB_VOIP_DISTONE_NUM			166
#define	MIB_VOIP_D1FREQNUM			167
#define	MIB_VOIP_D1FREQ1			168
#define	MIB_VOIP_D1FREQ2			169
#define	MIB_VOIP_D1ACCUR			170
#define	MIB_VOIP_D1LEVEL			171
#define	MIB_VOIP_D1ONUP				172
#define	MIB_VOIP_D1ONLOW			173
#define	MIB_VOIP_D1OFFUP			174
#define	MIB_VOIP_D1OFFLOW			175
#define	MIB_VOIP_D2FREQNUM			176
#define	MIB_VOIP_D2FREQ1			177
#define	MIB_VOIP_D2FREQ2			178
#define	MIB_VOIP_D2ACCUR			179
#define	MIB_VOIP_D2LEVEL			180
#define	MIB_VOIP_D2ONUP				181
#define	MIB_VOIP_D2ONLOW			182
#define	MIB_VOIP_D2OFFUP			183
#define	MIB_VOIP_D2OFFLOW			184

#define MIB_VOIP_HWNAT_ENABLE			185
// auto_bypass_relay
#define MIB_VOIP_AUTO_BYPASS_RELAY		204

// two_stage_dial
#define MIB_VOIP_TWO_STAGE_DIAL			205

// direct ip call
#define MIB_VOIP_DIRECT_IP_CALL			206

/*+++++added by Jack for auto provision for tftp and ftp+++++*/
#define MIB_VOIP_AUTO_CFG_TFTP_ADDR		207
#define MIB_VOIP_AUTO_CFG_FTP_ADDR		208
#define MIB_VOIP_AUTO_CFG_FTP_USER		209
#define MIB_VOIP_AUTO_CFG_FTP_PASSWD		210
/*-----end-----*/

// port setting - codec
#define MIB_VOIP_G726_PACK			211

// bandwidth mgr
#define MIB_VOIP_BANDWIDTH_LANPORT0_EGRESS	212
#define MIB_VOIP_BANDWIDTH_LANPORT1_EGRESS	213
#define MIB_VOIP_BANDWIDTH_LANPORT2_EGRESS	214
#define MIB_VOIP_BANDWIDTH_LANPORT3_EGRESS	215
#define MIB_VOIP_BANDWIDTH_WANPORT_EGRESS	216
#define MIB_VOIP_BANDWIDTH_LANPORT0_INGRESS	217
#define MIB_VOIP_BANDWIDTH_LANPORT1_INGRESS	218
#define MIB_VOIP_BANDWIDTH_LANPORT2_INGRESS	219
#define MIB_VOIP_BANDWIDTH_LANPORT3_INGRESS	220
#define MIB_VOIP_BANDWIDTH_WANPORT_INGRESS	221

// port setting - speex nb codec
#define MIB_VOIP_SPEEX_NB_RATE			222

// customize tone
#define MIB_VOIP_CUST_TONE_DUMMY		223

// V.152
#define MIB_VOIP_USE_V152		224
#define MIB_VOIP_V152_PAYLOAD_TYPE	225
#define MIB_VOIP_V152_CODEC_TYPE	226

// RTP redundant
#define MIB_VOIP_RTP_RED_PAYLOAD_TYPE	227
#define MIB_VOIP_RTP_RED_CODEC		228

//WAN VLAN TAG
#define MIB_VOIP_VLAN_ENABLE             229
#define MIB_VOIP_VLAN_TAG                230
#define MIB_VOIP_VLAN_BRIDGE_ENABLE      231
#define MIB_VOIP_VLAN_BRIDGE_TAG         232
#define MIB_VOIP_VLAN_BRIDGE_PORT        233
#define MIB_VOIP_VLAN_BRIDGE_MULTICAST_ENABLE 234
#define MIB_VOIP_VLAN_BRIDGE_MULTICAST_TAG    235
#define MIB_VOIP_VLAN_HOST_ENABLE          236
#define MIB_VOIP_VLAN_HOST_TAG          237
#define MIB_VOIP_VLAN_HOST_PRI          238
#define MIB_VOIP_VLAN_WIFI_ENABLE       239
#define MIB_VOIP_VLAN_WIFI_TAG          240
#define MIB_VOIP_VLAN_WIFI_PRI          241
#define MIB_VOIP_VLAN_WIFI_VAP0_ENABLE  242
#define MIB_VOIP_VLAN_WIFI_VAP0_TAG     243
#define MIB_VOIP_VLAN_WIFI_VAP0_PRI     244
#define MIB_VOIP_VLAN_WIFI_VAP1_ENABLE  245
#define MIB_VOIP_VLAN_WIFI_VAP1_TAG     246
#define MIB_VOIP_VLAN_WIFI_VAP1_PRI     247
#define MIB_VOIP_VLAN_WIFI_VAP2_ENABLE  248
#define MIB_VOIP_VLAN_WIFI_VAP2_TAG     249
#define MIB_VOIP_VLAN_WIFI_VAP2_PRI     250
#define MIB_VOIP_VLAN_WIFI_VAP3_ENABLE  251
#define MIB_VOIP_VLAN_WIFI_VAP3_TAG     252
#define MIB_VOIP_VLAN_WIFI_VAP3_PRI     253

// reserve 254-269 for VLAN

#define MIB_VOIP_ECHO_LEC		270

// T.38 parameters
#define MIB_VOIP_T38_PARAM_ENABLE		271
#define MIB_VOIP_T38_MAX_BUFFER			272
#define MIB_VOIP_T38_RATE_MGT			273
#define MIB_VOIP_T38_MAX_RATE			274
#define MIB_VOIP_T38_ENABLE_ECM			275
#define MIB_VOIP_T38_ECC_SIGNAL			276
#define MIB_VOIP_T38_ECC_DATA			277

#define MIB_VOIP_PLC				278
#define MIB_VOIP_RTCPXR				279
#define MIB_VOIP_ANSTONE			280
#define MIB_VOIP_ECHO_NLP			281
#define MIB_VOIP_VAD_THRESHOLD			282
#define MIB_VOIP_CNG				283
#define MIB_VOIP_CNG_THRESHOLD			284
#define MIB_VOIP_G7111_PRECEDENCE		285
#define MIB_VOIP_SID_MODE			286
#define MIB_VOIP_SID_LEVEL			287
#define MIB_VOIP_SID_GAIN			288
#define MIB_VOIP_FAX_MODEM_RFC2833		289
#define MIB_VOIP_RTCP_INTERVAL			290
#define MIB_VOIP_FAXMODEM_RFC2833_PT		291
#define MIB_VOIP_DTMF_RFC2833_PI		292
#define MIB_VOIP_FAXMODEM_RFC2833_PI		293
#define MIB_VOIP_FAXMODEM_RFC2833_PT_SAME_DTMF	294
#define MIB_VOIP_T38_ENABLE_SPOOF	295		// T.38 parameters 
#define MIB_VOIP_T38_DUPLICATE_NUM	296		// T.38 parameters
#define MIB_VOIP_ROOT				297		

// Config Setting
#define	VOIP_NONE_SETTING		0
#define	VOIP_CURRENT_SETTING	1
#define	VOIP_DEFAULT_SETTING	2
#define	VOIP_ALL_SETTING		(VOIP_CURRENT_SETTING | VOIP_DEFAULT_SETTING)

enum {
	V_INT = 0,
	V_UINT,
	// 1 byte
	V_CHAR,
	V_BYTE,
	// 2 byte
	V_SHORT,
	V_WORD,
	// 4 byte
	V_LONG,
	V_DWORD,
	// other
	V_IP4,
	V_MAC6,
	V_MIB,
	//--------------------------------------
	//------------- Array type -------------
	//--------------------------------------
	V_STRING, // string is a special byte array
	V_BYTE_ARRAY,
	//--------------------------------------
	//------------- List Type --------------
	//--------------------------------------
	V_INT_LIST,
	V_UINT_LIST,
	V_CHAR_LIST,
	V_BYTE_LIST,
	V_SHORT_LIST,
	V_WORD_LIST,
	V_LONG_LIST,
	V_DWORD_LIST,
	V_IP4_LIST,
	V_MAC6_LIST,
	V_MIB_LIST,
	V_END
};

typedef struct voipMibEntry_S voipMibEntry_T;

struct voipMibEntry_S {
	int id;
	char name[32];
	int type;
	int offset;
	int total_size;						// if type is V_XXX_LIST (total_size)
	int unit_size;						
	voipMibEntry_T *next_mib_table;		// if type is V_MIB or V_MIB_LIST
    const unsigned char *default_value;
};

#ifndef FIELD_OFFSET
#define FIELD_OFFSET(type, field)	((unsigned long)(long *)&(((type *)0)->field))
#endif

#ifndef NULL
#define NULL ((void *) 0)
#endif

#define VOIP_OFFSET(field)					((int)FIELD_OFFSET(voipCfgParam_t,field))
#define VOIP_SIZE(field)					sizeof(((voipCfgParam_t *)0)->field)
#define VOIP_FIELD(field)					VOIP_OFFSET(field), VOIP_SIZE(field), 0, NULL
#define VOIP_MIB_FIELD(field, mib)			VOIP_OFFSET(field), VOIP_SIZE(field), 0, mib
#define VOIP_LIST_FIELD(field)				VOIP_OFFSET(field), VOIP_SIZE(field), \
											VOIP_SIZE(field[0]), NULL
#define VOIP_MIB_LIST_FIELD(field, mib)		VOIP_OFFSET(field), VOIP_SIZE(field), \
											VOIP_SIZE(field[0]), mib
#define VOIP_STRING_FIELD					VOIP_LIST_FIELD
#define VOIP_ARRAY_FIELD					VOIP_LIST_FIELD

#define VOIP_PORT_OFFSET(field)					((int)FIELD_OFFSET(voipCfgPortParam_t,field))
#define VOIP_PORT_SIZE(field)					sizeof(((voipCfgPortParam_t *)0)->field)
#define VOIP_PORT_FIELD(field)					VOIP_PORT_OFFSET(field), VOIP_PORT_SIZE(field), 0, NULL
#define VOIP_PORT_MIB_FIELD(field, mib)			VOIP_PORT_OFFSET(field), VOIP_PORT_SIZE(field), 0, mib
#define VOIP_PORT_LIST_FIELD(field)				VOIP_PORT_OFFSET(field), VOIP_PORT_SIZE(field), \
												VOIP_PORT_SIZE(field[0]), NULL
#define VOIP_PORT_MIB_LIST_FIELD(field, mib)	VOIP_PORT_OFFSET(field), VOIP_PORT_SIZE(field), \
												VOIP_PORT_SIZE(field[0]), mib
#define VOIP_PORT_STRING_FIELD					VOIP_PORT_LIST_FIELD
#define VOIP_PORT_ARRAY_FIELD					VOIP_PORT_LIST_FIELD

/* speed dial */
#define VOIP_SPEED_DIAL_OFFSET(field)			((int)FIELD_OFFSET(SpeedDialCfg_t,field))
#define VOIP_SPEED_DIAL_SIZE(field)				sizeof(((SpeedDialCfg_t *)0)->field)
#define VOIP_SPEED_DIAL_FIELD(field)			VOIP_SPEED_DIAL_OFFSET(field), VOIP_SPEED_DIAL_SIZE(field), 0, NULL
#define VOIP_SPEED_DIAL_LIST_FIELD(field)		VOIP_SPEED_DIAL_OFFSET(field), VOIP_SPEED_DIAL_SIZE(field), \
												VOIP_SPEED_DIAL_SIZE(field[0]), NULL
#define VOIP_SPEED_DIAL_STRING_FIELD			VOIP_SPEED_DIAL_LIST_FIELD

/* customer tone */
#define VOIP_CUST_TONE_OFFSET(field)		((int)FIELD_OFFSET(st_ToneCfgParam,field))
#define VOIP_CUST_TONE_SIZE(field)			sizeof(((st_ToneCfgParam *)0)->field)
#define VOIP_CUST_TONE_FIELD(field)			VOIP_CUST_TONE_OFFSET(field), VOIP_CUST_TONE_SIZE(field), 0, NULL
#define VOIP_CUST_TONE_LIST_FIELD(field)	VOIP_CUST_TONE_OFFSET(field), VOIP_CUST_TONE_SIZE(field), \
											VOIP_CUST_TONE_SIZE(field[0]), NULL
#define VOIP_CUST_TONE_STRING_FIELD			VOIP_CUST_TONE_LIST_FIELD

/* voip proxy */
#define VOIP_PROXY_OFFSET(field)			((int)FIELD_OFFSET(voipCfgProxy_t,field))
#define VOIP_PROXY_SIZE(field)				sizeof(((voipCfgProxy_t *)0)->field)
#define VOIP_PROXY_FIELD(field)				VOIP_PROXY_OFFSET(field), VOIP_PROXY_SIZE(field), 0, NULL
#define VOIP_PROXY_LIST_FIELD(field)		VOIP_PROXY_OFFSET(field), VOIP_PROXY_SIZE(field), \
											VOIP_PROXY_SIZE(field[0]), NULL
#define VOIP_PROXY_STRING_FIELD				VOIP_PROXY_LIST_FIELD

#define VOIP_ABBREVIATED_DIAL_OFFSET(field)			((int)FIELD_OFFSET(abbrDialCfg_t,field))
#define VOIP_ABBREVIATED_DIAL_SIZE(field)				sizeof(((abbrDialCfg_t *)0)->field)
#define VOIP_ABBREVIATED_DIAL_FIELD(field)			VOIP_ABBREVIATED_DIAL_OFFSET(field), VOIP_ABBREVIATED_DIAL_SIZE(field), 0, NULL
#define VOIP_ABBREVIATED_DIAL_LIST_FIELD(field)		VOIP_ABBREVIATED_DIAL_OFFSET(field), VOIP_ABBREVIATED_DIAL_SIZE(field), \
													VOIP_ABBREVIATED_DIAL_SIZE(field[0]), NULL
#define VOIP_ABBREVIATED_DIAL_STRING_FIELD			VOIP_ABBREVIATED_DIAL_LIST_FIELD

#ifdef __cplusplus
extern "C" {
#endif

// check mib table is correct
int voip_mibtbl_check(int *size);
// write mib table to fd
int voip_mibtbl_write(const void *data, int fd, int mode);
// read mib entry from line
int voip_mibtbl_read_line(voipCfgAll_t *cfg_all, char *line);
// get mib entry, and output result to fd
int voip_mibtbl_get(const char *mib_name, voipCfgAll_t *cfg_all, int fd);
// set mib entry, output result to fd
int voip_mibtbl_set(const char *mib_name, const char *mib_value, voipCfgAll_t *cfg_all, int fd);
// swap if exchange mib data between two different endian system
int voip_mibtbl_swap_value(void *data);

// mib operations
int voip_mibline_to(char *line, const char *name, const char *value);
int voip_mibline_from(const char *line, char *name, char *value);
int voip_mib_from(const voipMibEntry_T *mib_tbl, const char *name,
	const voipMibEntry_T **ppmib, int *offset);
int voip_mib_read(void *data, const char *name, const char *value);

#ifdef VOIP_SUPPORT_TLV_CFG
int voip_mib_write_to_tlv(const voipMibEntry_T *mib_tbl, void *data, unsigned char *pfile, unsigned int *idx);
unsigned int voip_mib_cfg_init_from_tlv(const voipMibEntry_T *mib_root_tbl, unsigned char *pdata_array, void *pfile, unsigned int data_size, unsigned int *pmib_root_offset);
#endif /* VOIP_SUPPORT_TLV_CFG */

// extern variables
extern voipMibEntry_T mibtbl_voip[];
#ifdef VOIP_SUPPORT_TLV_CFG
extern voipMibEntry_T mibtbl_voip_root[];
#endif

#ifdef __cplusplus
}
#endif

#endif // __VOIP_FLASH_MIB_H
