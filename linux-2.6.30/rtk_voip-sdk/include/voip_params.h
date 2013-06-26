/**
 * @file voip_params.h
 * @brief VoIP control parameters
 */

#ifndef VOIP_PARAMS_H
#define VOIP_PARAMS_H

#include "rtk_voip.h"

/**
 * @ingroup VOIP_DSP_CODEC
 * Enumeration for supported rtp payload
 */
typedef enum
{
    rtpPayloadUndefined = -1,
    rtpPayloadPCMU = 0,
    rtpPayload1016 = 1,
    rtpPayloadG726_32 = 2,
    rtpPayloadGSM = 3,
    rtpPayloadG723 = 4,
    rtpPayloadDVI4_8KHz = 5,
    rtpPayloadDVI4_16KHz = 6,
    rtpPayloadLPC = 7,
    rtpPayloadPCMA = 8,
    rtpPayloadG722 = 9,
    rtpPayloadL16_stereo = 10,
    rtpPayloadL16_mono = 11,
    rtpPayloadQCELP = 12,
    rtpPayloadMPA = 14,
    rtpPayloadG728 = 15,
    rtpPayloadDVI4_11KHz = 16,
    rtpPayloadDVI4_22KHz = 17,
    rtpPayloadG729 = 18,
    rtpPayloadCN = 19,
    rtpPayloadG726_40    = 21,
    rtpPayloadG726_24    = 22,
    rtpPayloadG726_16    = 23,
    rtpPayloadCelB = 25,
    rtpPayloadJPEG = 26,
    rtpPayloadNV = 28,
    rtpPayloadH261 = 31,
    rtpPayloadMPV = 32,
    rtpPayloadMP2T = 33,
    rtpPayloadH263 = 34,
    // fake codec 
    rtpPayloadSilence = 35,
	// dynamic payload type in rtk_voip
    rtpPayloadDTMF_RFC2833 = 96,
	rtpPayload_iLBC = 97,
	rtpPayload_iLBC_20ms = 98,
    rtpPayloadFaxModem_RFC2833 = 101,
    rtpPayloadPCMU_WB = 102,
    rtpPayloadPCMA_WB = 103,
    rtpPayloadT38_Virtual = 110,
    rtpPayloadV152_Virtual = 111,
    rtpPayloadCiscoRtp = 121,
    rtpPayloadL16_8k_mono = 122,
    rtpPayload_AMR_NB = 123,
    rtpPayload_SPEEX_NB_RATE8 = 111,
    rtpPayload_SPEEX_NB_RATE2P15 = 112,
    rtpPayload_SPEEX_NB_RATE5P95 = 113,
    rtpPayload_SPEEX_NB_RATE11 = 114,
    rtpPayload_SPEEX_NB_RATE15 = 115,
    rtpPayload_SPEEX_NB_RATE18P2 = 116,
    rtpPayload_SPEEX_NB_RATE24P6 = 117,
    rtpPayload_SPEEX_NB_RATE3P95 = 118,
    rtpPayload_PCM_Linear_8K = 119,
    rtpPayload_PCM_Linear_16K = 120,
    rtpPayloadRtpRedundant = 121,
} RtpPayloadType;

/**
 * @ingroup VOIP_SESSION_RTP
 * Enumeration for rtp session state
 */
typedef enum
{
    rtp_session_unchange = -2,			        ///< RTP Session- Unchanged
    rtp_session_undefined = -1,				///< RTP Session- Undefined
    rtp_session_inactive = 0,                           ///< RTP Session- Inactive
    rtp_session_sendonly = 1,                           ///< RTP Session- Sending only
    rtp_session_recvonly = 2,                           ///< RTP Session- Receiving only
    rtp_session_sendrecv = 3                            ///< RTP Session- Send/Recv
} RtpSessionState;

#ifdef CONFIG_AUDIOCODES_VOIP
// country tone set
typedef enum
{
	DSPCODEC_COUNTRY_USA,
	DSPCODEC_COUNTRY_UK,
	DSPCODEC_COUNTRY_AUSTRALIA,
	DSPCODEC_COUNTRY_HK,
	DSPCODEC_COUNTRY_JP,
	DSPCODEC_COUNTRY_SE,
	DSPCODEC_COUNTRY_GR,
	DSPCODEC_COUNTRY_FR,
#if 1
	DSPCODEC_COUNTRY_TW,
#else
	DSPCODEC_COUNTRY_TR,
#endif
	DSPCODEC_COUNTRY_BE,
	DSPCODEC_COUNTRY_FL,
	DSPCODEC_COUNTRY_IT,
	DSPCODEC_COUNTRY_CN,
	DSPCODEC_COUNTRY_EX1,		///< extend country #1
	DSPCODEC_COUNTRY_EX2,		///< extend country #2
	DSPCODEC_COUNTRY_EX3,		///< extend country #3
	DSPCODEC_COUNTRY_EX4,		///< extend country #4
#ifdef COUNTRY_TONE_RESERVED
	DSPCODEC_COUNTRY_RESERVE,
#endif
	DSPCODEC_COUNTRY_CUSTOME
} DSPCODEC_COUNTRY;
#endif

/**
 * @ingroup VOIP_DSP_TONE
 * Enumeration for which tone to play
 */
typedef enum
{
	DSPCODEC_TONE_NONE = -1,
	DSPCODEC_TONE_0 = 0,
	DSPCODEC_TONE_1,
	DSPCODEC_TONE_2,
	DSPCODEC_TONE_3,
	DSPCODEC_TONE_4,
	DSPCODEC_TONE_5,
	DSPCODEC_TONE_6,
	DSPCODEC_TONE_7,
	DSPCODEC_TONE_8,
	DSPCODEC_TONE_9,
	DSPCODEC_TONE_STARSIGN,				// *
	DSPCODEC_TONE_HASHSIGN,				// #

#ifdef SUPPORT_TONE_PROFILE

	DSPCODEC_TONE_DIAL,				///< code = 66 in RFC2833
	DSPCODEC_TONE_STUTTERDIAL,			///< 
	DSPCODEC_TONE_MESSAGE_WAITING,			///< 
	DSPCODEC_TONE_CONFIRMATION,			///< 
	DSPCODEC_TONE_RINGING, 				///< ring back tone, code = 70 in RFC2833
	DSPCODEC_TONE_BUSY,				///< code = 72 in RFC2833
	DSPCODEC_TONE_CONGESTION,			///< 
	DSPCODEC_TONE_ROH,				///< 
	DSPCODEC_TONE_DOUBLE_RING,			///< 
	DSPCODEC_TONE_SIT_NOCIRCUIT,			///< 
	DSPCODEC_TONE_SIT_INTERCEPT,			///< 
	DSPCODEC_TONE_SIT_VACANT,			///< 
	DSPCODEC_TONE_SIT_REORDER,			///< 
	DSPCODEC_TONE_CALLING_CARD_WITHEVENT,		///< 
	DSPCODEC_TONE_CALLING_CARD,			///< 
	DSPCODEC_TONE_CALL_WAITING,			///< code = 79 in RFC2833
	DSPCODEC_TONE_CALL_WAITING_2,			///< 
	DSPCODEC_TONE_CALL_WAITING_3,			///< 
	DSPCODEC_TONE_CALL_WAITING_4,			///< 
	DSPCODEC_TONE_EXTEND_1,				///< extend tone #1
	DSPCODEC_TONE_EXTEND_2,				///< extend tone #2
	DSPCODEC_TONE_EXTEND_3,				///< extend tone #3
	DSPCODEC_TONE_EXTEND_4,				///< extend tone #4
	DSPCODEC_TONE_EXTEND_5,				///< extend tone #5
	DSPCODEC_TONE_INGRESS_RINGBACK,			///< 

	DSPCODEC_TONE_HOLD,				///< code = 76 in RFC2833
	DSPCODEC_TONE_OFFHOOKWARNING,			///< code = 88 in RFC2833
	DSPCODEC_TONE_RING,				///< code = 89 in RFC2833

#else
	DSPCODEC_TONE_DIAL,				///< code = 66 in RFC2833
	DSPCODEC_TONE_RINGING,				///< code = 70 in RFC2833
	DSPCODEC_TONE_BUSY,				///< code = 72 in RFC2833
	DSPCODEC_TONE_HOLD,				///< code = 76 in RFC2833

	DSPCODEC_TONE_CALL_WAITING,			///< code = 79 in RFC2833
	DSPCODEC_TONE_OFFHOOKWARNING,			///< code = 88 in RFC2833
	DSPCODEC_TONE_RING,				///< code = 89 in RFC2833

#endif // #ifdef SUPPORT_TONE_PROFILE

#ifdef SW_DTMF_CID
	// hc+ 1124 for DTMF CID =================
	DSPCODEC_TONE_A,					///< DTMF digit A (19)
	DSPCODEC_TONE_B,					///< DTMF digit B (20)
	DSPCODEC_TONE_C,					///< DTMF digit C (21)
	DSPCODEC_TONE_D,					///< DTMF digit D (22)
	//=========================================
#endif
	// sandro+ 2006/07/24 for SAS tone
	DSPCODEC_TONE_FSK_SAS,				///< alert signal (23)
	// hc+ 1229 for off hook FSK CID
	DSPCODEC_TONE_FSK_ALERT,				///< alert signal (23)
	// jwsyu+ 20111208 for off hook FSK CID mute voice
	DSPCODEC_TONE_FSK_MUTE,				///< mute voice when off hook CID
	
	DSPCODEC_TONE_NTT_IIT_TONE,			///< for NTT type 2 caller id		

	// thlin+ continous DTMF tone play for RFC2833
	DSPCODEC_TONE_0_CONT,				///< 
	DSPCODEC_TONE_1_CONT,                           ///< 
	DSPCODEC_TONE_2_CONT,                           ///< 
	DSPCODEC_TONE_3_CONT,                           ///< 
	DSPCODEC_TONE_4_CONT,                           ///< 
	DSPCODEC_TONE_5_CONT,                           ///< 
	DSPCODEC_TONE_6_CONT,                           ///< 
	DSPCODEC_TONE_7_CONT,                           ///< 
	DSPCODEC_TONE_8_CONT,                           ///< 
	DSPCODEC_TONE_9_CONT,                           ///< 
	DSPCODEC_TONE_STARSIGN_CONT,			///< 
	DSPCODEC_TONE_HASHSIGN_CONT,			///< 
	DSPCODEC_TONE_A_CONT,				///< 
	DSPCODEC_TONE_B_CONT,				///< 
	DSPCODEC_TONE_C_CONT,				///< 
	DSPCODEC_TONE_D_CONT,				///< 
	
	DSPCODEC_TONE_VBD_ANS,				///< index = 58
	DSPCODEC_TONE_VBD_ANSBAR,			///< index = 59
	DSPCODEC_TONE_VBD_ANSAM,				///< index = 60
	DSPCODEC_TONE_VBD_ANSAMBAR,			///< index = 61
	DSPCODEC_TONE_VBD_CNG,				///< index = 62
	DSPCODEC_TONE_VBD_CRE,				///< index = 63

	DSPCODEC_TONE_USER_DEFINE1,			///< user define tone #1
	DSPCODEC_TONE_USER_DEFINE2,			///< user define tone #2
	DSPCODEC_TONE_USER_DEFINE3,			///< user define tone #3
	DSPCODEC_TONE_USER_DEFINE4,			///< user define tone #4
	DSPCODEC_TONE_USER_DEFINE5,			///< user define tone #5
	
	DSPCODEC_TONE_KEY				///< the others key tone

} DSPCODEC_TONE;

/**
 * @ingroup VOIP_DSP_TONE
 * Enumeration for which direction to play tone
 */
typedef enum									
{
	DSPCODEC_TONEDIRECTION_LOCAL,			///< local 
	DSPCODEC_TONEDIRECTION_REMOTE,			///< remote 
	DSPCODEC_TONEDIRECTION_BOTH			///< local and remote
} DSPCODEC_TONEDIRECTION;

/**
 * @ingroup VOIP_DSP_IVR
 * Play Types in IVR
 * We provide many types of input
 */
typedef enum {
	IVR_PLAY_TYPE_TEXT,				///< play a string, such as '192.168.0.0'
	IVR_PLAY_TYPE_G723_63,				///< play a G723 6.3k data (24 bytes per 30 ms)
	IVR_PLAY_TYPE_G729,				///< play a G729 data (10 bytes per 10ms) 
	IVR_PLAY_TYPE_G711A,				///< play a G711 a-law (80 bytes per 10ms) 
	IVR_PLAY_TYPE_LINEAR_8K,				///< play 8k linear (160bytes per 10ms)
	IVR_PLAY_TYPE_LINEAR_16K,			///< play 16k linear (320bytes per 10ms) 
} IvrPlayType_t;

/**
 * @ingroup VOIP_DSP_IVR
 * IVR play direction (for playing TEXT only) 
 * One can play IVR in local or remote. 
 */
typedef enum {
	IVR_DIR_LOCAL,					///< IVR local direction
	IVR_DIR_REMOTE,					///< IVR remote direction
} IvrPlayDir_t;

/**
 * @ingroup VOIP_DSP_IVR
 * Specified Text Type
 * Execpt to ASCII within 0~127, we define special speech above 128.
 */
enum {
	IVR_TEXT_ID_DHCP		= 128,
	IVR_TEXT_ID_FIX_IP,
	IVR_TEXT_ID_NO_RESOURCE,
	IVR_TEXT_ID_PLZ_ENTER_NUMBER,	
	IVR_TEXT_ID_PLEASE_ENTER_PASSWORD,
	///<<&&ID5&&>>	/* DON'T remove this line, it helps wizard to generate ID. */
	//IVR_TEXT_ID_xxx,	
};

#ifdef CONFIG_RTK_VOIP_IP_PHONE
/**
 * @ingroup VOIP_IPPHONE_KEYPAD 
 * We list keypad control in follow 
 */
typedef enum {
	KEYPAD_CMD_SET_TARGET,
	KEYPAD_CMD_SIG_DEBUG,
	KEYPAD_CMD_READ_KEY,
	KEYPAD_CMD_HOOK_STATUS,
} keypad_cmd_t;

/**
 * @ingroup VOIP_IPPHONE_LCM 
 * We list LCM control in follow 
 */
typedef enum {
	LCM_CMD_DISPLAY_ON_OFF,
	LCM_CMD_MOVE_CURSOR_POS,
	LCM_CMD_DRAW_TEXT,
	LCM_CMD_WRITE_DATA,
	LCM_CMD_WRITE_DATA2,
	LCM_CMD_DIRTY_MMAP,
	LCM_CMD_DIRTY_MMAP2,
} lcm_cmd_t;

/**
 * @ingroup VOIP_IPPHONE_OTHERS 
 * Control voice path 
 */
typedef enum {
	VPATH_MIC1_SPEAKER,
	VPATH_MIC2_MONO,
	VPATH_SPEAKER_ONLY,
	VPATH_MONO_ONLY,
} vpath_t;
#endif /* CONFIG_RTK_VOIP_IP_PHONE */

/**
 * @brief For VoIP resourece parameters
 */
typedef enum 
{
	VOIP_RESOURCE_UNAVAILABLE=0,
	VOIP_RESOURCE_AVAILABLE,
}Voip_reosurce_t;

/**
 * @brief For FSK caller id mode.
 */
typedef enum
{
	FSK_Bellcore = 0,
	FSK_ETSI,
	FSK_BT,
	FSK_NTT
}TfskArea;

/**
 * @brief For FSK caller ID parameters
 */
typedef enum
{
	FSK_PARAM_NULL = 0,				///< 
	FSK_PARAM_DATEnTIME = 0x01,			///< Date and Time
	FSK_PARAM_CLI = 0x02,				///< Calling Line Identify (CLI)
	FSK_PARAM_CLI_ABS = 0x04,			///< Reason for absence of CLI
	FSK_PARAM_CLI_NAME = 0x07,			///< Calling Line Identify (CLI) Name
	FSK_PARAM_CLI_NAME_ABS = 0x08,			///< Reason for absence of (CLI) Name
	FSK_PARAM_MW = 0x0b,				///< Message Waiting

}TfskParaType;

/**
 * @brief For FSK caller ID gain parameters
 */
typedef enum
{
	FSK_CLID_GAIN_8DB_UP = 0,			///< 
	FSK_CLID_GAIN_7DB_UP,                           ///< 
	FSK_CLID_GAIN_6DB_UP,                           ///< 
	FSK_CLID_GAIN_5DB_UP,                           ///< 
	FSK_CLID_GAIN_4DB_UP,                           ///< 
	FSK_CLID_GAIN_3DB_UP,                           ///< 
	FSK_CLID_GAIN_2DB_UP,                           ///< 
	FSK_CLID_GAIN_1DB_UP,                           ///< 
	FSK_CLID_GAIN_0DB,                              ///< 
	FSK_CLID_GAIN_1DB_DOWN,                         ///< 
	FSK_CLID_GAIN_2DB_DOWN,                         ///< 
	FSK_CLID_GAIN_3DB_DOWN,                         ///< 
	FSK_CLID_GAIN_4DB_DOWN,                         ///< 
	FSK_CLID_GAIN_5DB_DOWN,                         ///< 
	FSK_CLID_GAIN_6DB_DOWN,                         ///< 
	FSK_CLID_GAIN_7DB_DOWN,                         ///< 
	FSK_CLID_GAIN_8DB_DOWN,                         ///< 
	FSK_CLID_GAIN_9DB_DOWN,                         ///< 
	FSK_CLID_GAIN_10DB_DOWN,                        ///< 
	FSK_CLID_GAIN_11DB_DOWN,                        ///< 
	FSK_CLID_GAIN_12DB_DOWN,                        ///< 
	FSK_CLID_GAIN_13DB_DOWN,                        ///< 
	FSK_CLID_GAIN_14DB_DOWN,                        ///< 
	FSK_CLID_GAIN_15DB_DOWN,                        ///< 
	FSK_CLID_GAIN_16DB_DOWN,                        ///< 
	FSK_CLID_GAIN_MAX                               ///< 

}TfskClidGain;

/**
 * @brief For LED display
 */
typedef enum {
	LED_OFF,						///< LED turn off
	LED_ON,						///< LED turn On
	LED_BLINKING,			///< LED blinking
} LedDisplayMode;

/**
 * @ingroup VOIP_PHONE
 * Enumeration for RFC2833 volume mode
 */
typedef enum
{
	RFC2833_VOLUME_DSP_NON_ATUO = 0,		///<
	RFC2833_VOLUME_DSP_ATUO = 1,			///<
	
}RFC2833_VOLUME_MODE;

/**
 * @ingroup VOIP_PHONE
 * Type definition for RFC2833 volume (n indicates -n dBm)
 */
typedef unsigned int RFC2833_VOLUME;

/**
 * @ingroup VOIP_DSP_GENERAL
 * Specified VoIP EVENT type 
 */
typedef enum {
	VET_DTMF	= 0x00010000,	///< bit 16: DTMF event 
	VET_HOOK	= 0x00020000,	///< bit 17: hook event 
	VET_ENERGY	= 0x00040000,	///< bit 18: energy event 
	VET_DECT	= 0x00080000,	///< bit 19: DECT event 
	VET_FAXMDM	= 0x00100000,	///< bit 20: FAX/MODEM event 
	VET_RFC2833	= 0x00200000,	///< bit 21: RFC 2833
	VET_DSP		= 0x00400000,	///< bit 22: other DSP event 
	
	VET_MASK	= 0xFFFF0000,	///< mask bits 16-31
	VET_ALL		= 0xFFFF0000,	///< all event type, bits 16-31
} VoipEventType;

/**
 * @ingroup VOIP_DSP_GENERAL
 * VoIP EVENT mask is used to mask (provide) some events for user 
 */
typedef enum {
	VEM_MID0	= 0x00000001,	///< bit 0: mask media 0 (RFC2833/DSP)
	VEM_MID1	= 0x00000002,	///< bit 1: mask media 1 (RFC2833/DSP)
	VEM_DIRTDM	= 0x00000010,	///< bit 4: mask direction TDM (DTMF) 
	VEM_DIRIP	= 0x00000020,	///< bit 5: mask direction IP (DTMF) 
	
	VEM_ALL		= 0xFFFFFFFF,	///< bit 0-31: mask all 
} VoipEventMask;

/**
 * @ingroup VOIP_DSP_GENERAL
 * Specified VoIP EVENT ID 
 */
typedef enum {
	VEID_MASK_ID	= 0x0000FFFF,	///< evnet ID mask 0-15
	VEID_MASK_TYPE	= VET_MASK,		///< event type mask 16-31
	
	VEID_NONE		= 0x00000000,	///< no event 
	
	// DTMF (0x00010000)
	VEID_DTMF_WILDCARD		= VET_DTMF,		///< to combine with ASCII '0'-'9' '*' '#'
	VEID_DTMF_DIGIT_MASK	= 0x000000FF,	///< mask to retrieve ASCII digit 
	
	VEID_DTMF_0		= VET_DTMF | '0',	///< DTMF '0'
	VEID_DTMF_1		= VET_DTMF | '1',	///< DTMF '1'
	VEID_DTMF_2		= VET_DTMF | '2',	///< DTMF '2'
	VEID_DTMF_3		= VET_DTMF | '3',	///< DTMF '3'
	VEID_DTMF_4		= VET_DTMF | '4',	///< DTMF '4'
	VEID_DTMF_5		= VET_DTMF | '5',	///< DTMF '5'
	VEID_DTMF_6		= VET_DTMF | '6',	///< DTMF '6'
	VEID_DTMF_7		= VET_DTMF | '7',	///< DTMF '7'
	VEID_DTMF_8		= VET_DTMF | '8',	///< DTMF '8'
	VEID_DTMF_9		= VET_DTMF | '9',	///< DTMF '9'
	VEID_DTMF_STAR	= VET_DTMF | '*',	///< DTMF '*'
	VEID_DTMF_SHARP	= VET_DTMF | '#',	///< DTMF '#'
	VEID_DTMF_A		= VET_DTMF | 'A',	///< DTMF 'A'
	VEID_DTMF_B		= VET_DTMF | 'B',	///< DTMF 'B'
	VEID_DTMF_C		= VET_DTMF | 'C',	///< DTMF 'C'
	VEID_DTMF_D		= VET_DTMF | 'D',	///< DTMF 'D'
	VEID_DTMF_ENERGY= VET_DTMF | 'E',	///< indicate 'E'nergy
	
	// Hook (0x00020000)
	VEID_HOOK_PHONE_ON_HOOK = VET_HOOK,	///< On hook 
	VEID_HOOK_PHONE_OFF_HOOK,			///< Off hook
	VEID_HOOK_PHONE_FLASH_HOOK,			///< Flash hook
	VEID_HOOK_PHONE_STILL_ON_HOOK,		///< Still on hook 
	VEID_HOOK_PHONE_STILL_OFF_HOOK,		///< Still off hook 
	//VEID_HOOK_PHONE_UNKNOWN,			///< Unknown??
	VEID_HOOK_FXO_ON_HOOK,				///< FXO on hook
	VEID_HOOK_FXO_OFF_HOOK,				///< FXO off hook
	VEID_HOOK_FXO_FLASH_HOOK,			///< FXO flash hook 
	VEID_HOOK_FXO_STILL_ON_HOOK,		///< FXO still on hook 
	VEID_HOOK_FXO_STILL_OFF_HOOK,		///< FXO still off hook
	VEID_HOOK_FXO_RING_ON,				///< FXO ring on 
	VEID_HOOK_FXO_RING_OFF,				///< FXO ring off 
	VEID_HOOK_FXO_BUSY_TONE,			///< FXO busy tone 
	VEID_HOOK_FXO_CALLER_ID,			///< FXO caller ID 
	VEID_HOOK_FXO_RING_TONE_ON,			///< FXO ring tone on 
	VEID_HOOK_FXO_RING_TONE_OFF,		///< FXO ring tone off 
	VEID_HOOK_FXO_POLARITY_REVERSAL,	///< FXO resersal 
	VEID_HOOK_FXO_CURRENT_DROP,			///< FXO current drop 
	VEID_HOOK_FXO_DIS_TONE,				///< FXO disconnect tone 
	//VEID_HOOK_FXO_UNKNOWN,			///< unknown?? 
	VEID_HOOK_OUTBAND_FLASH_EVENT,		///< RTP outband flash event 
	
	// Energy (0x00040000)
	VEID_ENERGY = VET_ENERGY,			///< Energy 
	
	// DECT (0x00080000)
	VEID_DECT_BUTTON_PAGE = VET_DECT, 	///< Page button 
	VEID_DECT_BUTTON_REGISTRATION_MODE,	///< Register button 
	VEID_DECT_BUTTON_DEL_HS,			///< Delete button 
	VEID_DECT_BUTTON_NOT_DEFINED,		///< Undefined button 
	
	// FAXMDM (0x00100000)
	VEID_FAXMDM_AUDIOCODES_FAX		= VET_FAXMDM | 1,	///< FAX event (Audiocodes)
	VEID_FAXMDM_AUDIOCODES_MODEM	= VET_FAXMDM | 2,	///< Modem event (Audiocodes)
	VEID_FAXMDM_LEC_AUTO_RESTORE	= VET_FAXMDM | 20,	///< 
	VEID_FAXMDM_LEC_AUTO_ON			= VET_FAXMDM | 21,	///< 
	VEID_FAXMDM_LEC_AUTO_OFF		= VET_FAXMDM | 22,	///< 
	VEID_FAXMDM_FAX_CED				= VET_FAXMDM | 30,	///< 
	VEID_FAXMDM_FAX_DIS_TX			= VET_FAXMDM | 31,	///< 
	VEID_FAXMDM_FAX_DIS_RX			= VET_FAXMDM | 32,	///< 
	VEID_FAXMDM_FAX_DCN_TX			= VET_FAXMDM | 33,	///< 
	VEID_FAXMDM_FAX_DCN_RX			= VET_FAXMDM | 34,	///< 
	VEID_FAXMDM_MODEM_LOCAL			= VET_FAXMDM | 35,	///< 
	VEID_FAXMDM_MODEM_LOCAL_DELAY	= VET_FAXMDM | 36,	///< This event means that DSP need to check if DIS_RX event occur within 4~6 sec after modem tone event is detected.
	VEID_FAXMDM_MODEM_REMOTE		= VET_FAXMDM | 37,	///< MODEM_REMOTE must right after MODEM_LOCAL_DELAY. see ced.c
	VEID_FAXMDM_ANSTONE_CNG_LOCAL	= VET_FAXMDM | 40,	///< 
	VEID_FAXMDM_ANSTONE_ANS_LOCAL	= VET_FAXMDM | 41,	///< 
	VEID_FAXMDM_ANSTONE_ANSAM_LOCAL	= VET_FAXMDM | 42,	///< 
	VEID_FAXMDM_ANSTONE_ANSBAR_LOCAL	= VET_FAXMDM | 43,	///< 
	VEID_FAXMDM_ANSTONE_ANSAMBAR_LOCAL	= VET_FAXMDM | 44,	///< 
	VEID_FAXMDM_ANSTONE_BELLANS_LOCAL	= VET_FAXMDM | 45,	///< 
	VEID_FAXMDM_ANSTONE_V22_LOCAL		= VET_FAXMDM | 46,	///< 
	VEID_FAXMDM_ANSTONE_V8BIS_LOCAL	= VET_FAXMDM | 47,	///< 
	VEID_FAXMDM_V21FLAG_LOCAL		= VET_FAXMDM | 48,	///< 
	VEID_FAXMDM_HS_FAX_SEND_V21FLAG_LOCAL = VET_FAXMDM | 49,	///< itu v.8 sender, V.21 CM 
	VEID_FAXMDM_HS_FAX_RECV_V21FLAG_LOCAL = VET_FAXMDM | 50,	///< itu v.8 receiver, V.21 JM
	VEID_FAXMDM_ANSTONE_OFF_LOCAL		= VET_FAXMDM | 51,	///< 
	                                                       
	VEID_FAXMDM_ANSTONE_CNG_REMOTE	= VET_FAXMDM | 60,		///< 
	VEID_FAXMDM_ANSTONE_ANS_REMOTE	= VET_FAXMDM | 61,		///< 
	VEID_FAXMDM_ANSTONE_ANSAM_REMOTE	= VET_FAXMDM | 62,	///< 
	VEID_FAXMDM_ANSTONE_ANSBAR_REMOTE	= VET_FAXMDM | 63,	///< 
	VEID_FAXMDM_ANSTONE_ANSAMBAR_REMOTE	= VET_FAXMDM | 64,	///< 
	VEID_FAXMDM_ANSTONE_BELLANS_REMOTE	= VET_FAXMDM | 65,	///< 
	VEID_FAXMDM_ANSTONE_V22_REMOTE	= VET_FAXMDM | 66,		///< 
	VEID_FAXMDM_ANSTONE_V8BIS_REMOTE	= VET_FAXMDM | 67,	///< 
	VEID_FAXMDM_V21FLAG_REMOTE		= VET_FAXMDM | 68,		///< 
	VEID_FAXMDM_HS_FAX_SEND_V21FLAG_REMOTE	= VET_FAXMDM | 69,	///< itu v.8 sender, V.21 CM 
	VEID_FAXMDM_HS_FAX_RECV_V21FLAG_REMOTE	= VET_FAXMDM | 70,	///< itu v.8 receiver, V.21 JM 
	VEID_FAXMDM_ANSTONE_OFF_REMOTE	= VET_FAXMDM | 71,		///< 
	
	VEID_FAXMDM_FAST_FAXTONE_LOCAL		= VET_FAXMDM | 80,	///<
	VEID_FAXMDM_FAST_MODEMTONE_LOCAL	= VET_FAXMDM | 81,	///<
	VEID_FAXMDM_FAST_FAXTONE_REMOTE		= VET_FAXMDM | 85,	///<
	VEID_FAXMDM_FAST_MODEMTONE_REMOTE	= VET_FAXMDM | 86,	///<
	                                                       
	VEID_FAXMDM_V152_RTP_VBD		= VET_FAXMDM | 90,	///< Switch to V.152 VBD due to receive VBD RTP
	VEID_FAXMDM_V152_RTP_AUDIO		= VET_FAXMDM | 91,	///< Switch to V.152 voice due to receive voice RTP
	VEID_FAXMDM_V152_SIG_CED		= VET_FAXMDM | 92,	///< Switch to V.152 VBD due to CED signal 
	VEID_FAXMDM_V152_TDM_SIG_END	= VET_FAXMDM | 93,	///< Switch to V.152 voice due to END signal 
	VEID_FAXMDM_V152_BI_SILENCE		= VET_FAXMDM | 94,	///< Switch to V.152 voice due to silence 
	                                                       
	VEID_FAXMDM_SILENCE_TDM		= VET_FAXMDM | 100,	///< Silence in TDM side 
	VEID_FAXMDM_SILENCE_IP		= VET_FAXMDM | 101,	///< Silence in IP side 
	VEID_FAXMDM_SILENCE_TDM_IP	= VET_FAXMDM | 102,	///< Silence in both sides 
	
	// RFC2833 RTP RX (Receive RFC2833 from RTP packet) (0x00200000)
	VEID_RFC2833_RX_WILDCARD	= VET_RFC2833,		///< To combine with 8 bits evnet encoding 
	VEID_RFC2833_RX_MASK		= 0x000000FF,		///< To obtain 8 bits event encoding 
	
	VEID_RFC2833_RX_DTMF_0		= VET_RFC2833 | 0,	///< RFC2833 - DTMF 0
	VEID_RFC2833_RX_DTMF_1,							///< RFC2833 - DTMF 1
	VEID_RFC2833_RX_DTMF_2,							///< RFC2833 - DTMF 2
	VEID_RFC2833_RX_DTMF_3,							///< RFC2833 - DTMF 3
	VEID_RFC2833_RX_DTMF_4,							///< RFC2833 - DTMF 4
	VEID_RFC2833_RX_DTMF_5,							///< RFC2833 - DTMF 5
	VEID_RFC2833_RX_DTMF_6,							///< RFC2833 - DTMF 6
	VEID_RFC2833_RX_DTMF_7,							///< RFC2833 - DTMF 7
	VEID_RFC2833_RX_DTMF_8,							///< RFC2833 - DTMF 8
	VEID_RFC2833_RX_DTMF_9,							///< RFC2833 - DTMF 9
	VEID_RFC2833_RX_DTMF_STAR,						///< RFC2833 - DTMF *
	VEID_RFC2833_RX_DTMF_POUND,						///< RFC2833 - DTMF #
	VEID_RFC2833_RX_DTMF_A,							///< RFC2833 - DTMF A
	VEID_RFC2833_RX_DTMF_B,							///< RFC2833 - DTMF B
	VEID_RFC2833_RX_DTMF_C,							///< RFC2833 - DTMF C
	VEID_RFC2833_RX_DTMF_D,							///< RFC2833 - DTMF D
	VEID_RFC2833_RX_DTMF_FLASH,						///< RFC2833 - DTMF flash 

	VEID_RFC2833_RX_DATA_FAX_ANS = VET_RFC2833 | 32,	///< RFC2833 - Data FAX ANS
	VEID_RFC2833_RX_DATA_FAX_ANSBAR,				///< RFC2833 - Data FAX /ANS
	VEID_RFC2833_RX_DATA_FAX_ANSAM,					///< RFC2833 - Data FAX ANS AM
	VEID_RFC2833_RX_DATA_FAX_ANSAMBAR,				///< RFC2833 - Data FAX /SNA AM
	VEID_RFC2833_RX_DATA_FAX_CNG,					///< RFC2833 - Data FAX CNG
	VEID_RFC2833_RX_DATA_FAX_V21_CH1_B0,			///< RFC2833 - Data FAX V21 CH1 B0
	VEID_RFC2833_RX_DATA_FAX_V21_CH1_B1,			///< RFC2833 - Data FAX V21 CH1 B1
	VEID_RFC2833_RX_DATA_FAX_V21_CH2_B0,			///< RFC2833 - Data FAX V21 CH2 B0
	VEID_RFC2833_RX_DATA_FAX_V21_CH2_B1,			///< RFC2833 - Data FAX V21 CH2 B1 
	VEID_RFC2833_RX_DATA_FAX_CRDI,					///< RFC2833 - Data FAX CRDI
	VEID_RFC2833_RX_DATA_FAX_CRDR,					///< RFC2833 - Data FAX CRDR
	VEID_RFC2833_RX_DATA_FAX_CRE,					///< RFC2833 - Data FAX CRE
	VEID_RFC2833_RX_DATA_FAX_ESI,					///< RFC2833 - Data FAX ESI
	VEID_RFC2833_RX_DATA_FAX_ESR,					///< RFC2833 - Data FAX ESR 
	VEID_RFC2833_RX_DATA_FAX_MRDI,					///< RFC2833 - Data FAX MRDI 
	VEID_RFC2833_RX_DATA_FAX_MRDR,					///< RFC2833 - Data FAX MRDR 
	VEID_RFC2833_RX_DATA_FAX_MRE,					///< RFC2833 - Data FAX MRE 
	VEID_RFC2833_RX_DATA_FAX_CT,					///< RFC2833 - Data FAX CT 
	
	// DSP Event (0x00400000)
	VEID_DSP_DTMF_CLID_END			= VET_DSP | 10,	///< DTMF CLID END Event 
	VEID_DSP_FSK_CLID_END			= VET_DSP | 11,	///< FSK CLID END Event 
	VEID_DSP_FSK_CLID_TYPE2_NO_ACK	= VET_DSP | 12,	///< FSK TYPE2 CLID NO ACK Event
	VEID_DSP_LOCAL_TONE_END			= VET_DSP | 20,	///< Local tone stop event when the tone duration expired. 
	VEID_DSP_REMOTE_TONE_END		= VET_DSP | 21,	///< Remote tone stop event when the tone duration expired. 
	VEID_DSP_LOCAL_TONE_USER_STOP	= VET_DSP | 22,	///< Local tone stop event by user
	VEID_DSP_REMOTE_TONE_USER_STOP	= VET_DSP | 23,	///< Remote tone stop event by user	
	
} VoipEventID;

/**
 * @ingroup VOIP_DRIVER_PCM
 * Bus data format 
 */
typedef enum
{
	AP_BUSDATFMT_PCM_LINEAR,			///< PCM linear 
	AP_BUSDATFMT_PCM_ALAW,				///< PCM A law  
	AP_BUSDATFMT_PCM_ULAW,				///< PCM u law
	AP_BUSDATFMT_PCM_WIDEBAND_LINEAR,	///< PCM wideband - linear
	AP_BUSDATFMT_PCM_WIDEBAND_ALAW,		///< PCM wideband - A law
	AP_BUSDATFMT_PCM_WIDEBAND_ULAW,		///< PCM wideband - u law
	AP_BUSDATFMT_PCM_UNKNOWN,			///< Unknown PCM mode 
} AP_BUS_DATA_FORMAT;

/**
 * @ingroup VOIP_PORT_LINK_STATUS
 * The bits definition related to TstVoipPortLinkStatus.status
 */
#define PORT_LINK_LAN_ALL	0x0000000F	/* bits 0-3, active LAN */
#define PORT_LINK_LAN0		0x00000001	
#define PORT_LINK_LAN1		0x00000002
#define PORT_LINK_LAN2		0x00000004
#define PORT_LINK_LAN3		0x00000008
#define PORT_LINK_RESERVED1	0x000000F0	/* bit 4-7: reserved for more LAN */
#define PORT_LINK_WAN		0x00000100	/* bit 8, active WAN */

#endif

