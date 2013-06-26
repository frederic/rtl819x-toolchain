#include <linux/config.h>
#include <linux/poll.h>

#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_control.h"
#include "voip_init.h"
#include "voip_dev.h"

#include "voip_mgr_define.h"

#include "voip_mgr_do_protocol.h"
#include "voip_mgr_do_dsp.h"
#include "voip_mgr_do_driver.h"
#include "voip_mgr_do_misc.h"
#include "voip_mgr_do_ipphone.h"
#include "voip_mgr_do_debug.h"

typedef struct {
	uint32 DUMMY_F;
} DUMMY_T;

// DO NOT use these 
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
#define _M_BASE_NAME( x )				x, do_mgr_ ## x, # x
#else
#define _M_BASE_NAME( x )				x, do_mgr_ ## x 
#endif
#define _M_BASE_FIELD( typ, fld )		sizeof( typ ), sizeof( ( ( typ * )0 )->fld ), ( uint32 )&( ( ( typ * )0 )->fld )

// Use these: 
//                      Regular  Ethdsp:host          Ethdsp:dsp
//                      -------  -------------------  ---------------------
//  M_NORMAL_MGR        normal   fw + ch              normal
//  M_NORMAL_SND_MGR    normal   [fw + ch] | [body]   normal
//  M_NORMALBODY_MGR    normal   fw + ch + body       normal 
//  M_NOCHANNEL_MGR     normal   fw                   normal
//  M_NOCHANNEL_SND_MGR normal   fw + body            normal
//  M_FETCH_MGR         normal   fw + ch + ret        normal
//  M_FETCH_SND_MGR     normal   [fw+ch+ret] | [body] normal 
//  M_EVENT_MGR         normal   func call (no fw)    borrow ID to send event packet
//  M_PUREEVENT_MGR     undef    undef                borrow ID to send event packet
//  M_BODY_MGR          normal   func call fw         normal
//  M_COMPLEX_MGR       normal   complex flags        complex flags
//  M_UNDEF_MGR         undef    undef                undef 
//  M_STANDALONE_MGR    normal   undef                undef 
//  M_HOSTONLY_MGR      normal   func call (no fw)    undef  
//
#ifndef CONFIG_RTK_VOIP_IPC_ARCH
#define M_NORMAL_MGR( x, typ, fld )			_M_BASE_NAME( x )
#define M_NORMAL_SND_MGR( x, typ, fld )		_M_BASE_NAME( x )
#define M_NORMALBODY_MGR( x, typ, fld )		_M_BASE_NAME( x )
#define M_NOCHANNEL_MGR( x  )				_M_BASE_NAME( x )
#define M_NOCHANNEL_SND_MGR( x  )			_M_BASE_NAME( x )
#define M_FETCH_MGR( x, typ, fld )			_M_BASE_NAME( x )
#define M_FETCH_SND_MGR( x, typ, fld )		_M_BASE_NAME( x )
#define M_EVENT_MGR( x, typ, fld )			_M_BASE_NAME( x )
#define M_PUREEVENT_MGR( x )				M_UNDEF_MGR( x )
#define M_BODY_MGR( x, typ, fld )			_M_BASE_NAME( x )
#define M_COMPLEX_MGR( x, typ, fld, flg )	_M_BASE_NAME( x )
#define M_UNDEF_MGR( x )					x, NULL
#define M_STANDALONE_MGR( x )				_M_BASE_NAME( x )
#define M_HOSTONLY_MGR( x  )				_M_BASE_NAME( x )
#elif defined( CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST )
#define M_NORMAL_MGR( x, typ, fld )			_M_BASE_NAME( x ), 			\
											_M_BASE_FIELD( typ, fld ), 	\
											MF_AUTOFW | MF_CHANNEL
#define M_NORMAL_SND_MGR( x, typ, fld )		_M_BASE_NAME( x ), 			\
											_M_BASE_FIELD( typ, fld ), 	\
											MF_AUTOFW | MF_CHANNEL | MF_SNDCMD
#define M_NORMALBODY_MGR( x, typ, fld )		_M_BASE_NAME( x ), 			\
											_M_BASE_FIELD( typ, fld ), 	\
											MF_AUTOFW | MF_CHANNEL | MF_BODY
#define M_NOCHANNEL_MGR( x )				_M_BASE_NAME( x ), 			\
											_M_BASE_FIELD( DUMMY_T, DUMMY_F ), 	\
											MF_AUTOFW
#define M_NOCHANNEL_SND_MGR( x )			_M_BASE_NAME( x ), 			\
											_M_BASE_FIELD( DUMMY_T, DUMMY_F ), 	\
											MF_AUTOFW | MF_SNDCMD
#define M_FETCH_MGR( x, typ, fld )			_M_BASE_NAME( x ), 			\
											_M_BASE_FIELD( typ, fld ), 	\
											MF_AUTOFW | MF_CHANNEL | MF_FETCH
#define M_FETCH_SND_MGR( x, typ, fld )		_M_BASE_NAME( x ), 			\
											_M_BASE_FIELD( typ, fld ), 	\
											MF_AUTOFW | MF_CHANNEL | MF_FETCH | MF_SNDCMD
#define M_EVENT_MGR( x, typ, fld )			_M_BASE_NAME( x ), 			\
											_M_BASE_FIELD( typ, fld ), 	\
											MF_BODY
#define M_PUREEVENT_MGR( x )				M_UNDEF_MGR( x )
#define M_BODY_MGR( x, typ, fld )			_M_BASE_NAME( x ), 			\
											_M_BASE_FIELD( typ, fld ), 	\
											MF_BODY
#define M_COMPLEX_MGR( x, typ, fld, flg )	_M_BASE_NAME( x ), 			\
											_M_BASE_FIELD( typ, fld ), 	\
											flg
#define M_UNDEF_MGR( x )					x, NULL 
#define M_STANDALONE_MGR( x )				M_UNDEF_MGR( x )
#define M_HOSTONLY_MGR( x )					_M_BASE_NAME( x ), 			\
											_M_BASE_FIELD( DUMMY_T, DUMMY_F ), 	\
											MF_BODY
#elif defined( CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP )
#define M_NORMAL_MGR( x, typ, fld )			_M_BASE_NAME( x ), 0
#define M_NORMAL_SND_MGR( x, typ, fld )		_M_BASE_NAME( x ), 0
#define M_NORMALBODY_MGR( x, typ, fld )		_M_BASE_NAME( x ), 0
#define M_NOCHANNEL_MGR( x )				_M_BASE_NAME( x ), 0
#define M_NOCHANNEL_SND_MGR( x )			_M_BASE_NAME( x ), 0
#define M_FETCH_MGR( x, typ, fld )			_M_BASE_NAME( x ), 0
#define M_FETCH_SND_MGR( x, typ, fld )		_M_BASE_NAME( x ), 0
#define M_EVENT_MGR( x, typ, fld )			x, NULL, MF_EVENTCMD
#define M_PUREEVENT_MGR( x )				x, NULL, MF_EVENTCMD
#define M_BODY_MGR( x, typ, fld )			_M_BASE_NAME( x ), 0
#define M_COMPLEX_MGR( x, typ, fld, flg )	_M_BASE_NAME( x ), flg 
#define M_UNDEF_MGR( x )					x, NULL 
#define M_STANDALONE_MGR( x )				M_UNDEF_MGR( x )
#define M_HOSTONLY_MGR( x )					M_UNDEF_MGR( x )
#else
#error ""
#endif

static voip_mgr_entry_t voip_mgr_table[] = {
	// testing function 
	{ M_UNDEF_MGR( VOIP_MGR_SET_EBL ), },
	{ M_UNDEF_MGR( VOIP_MGR_INIT_GNET ), },
	{ M_UNDEF_MGR( VOIP_MGR_INIT_GNET2 ), },
	{ M_UNDEF_MGR( VOIP_MGR_LOOPBACK ), },
	{ M_UNDEF_MGR( VOIP_MGR_GNET ), },
	{ M_UNDEF_MGR( VOIP_MGR_SIGNALTEST ), },
	{ M_UNDEF_MGR( VOIP_MGR_DSPSETCONFIG ), },
	{ M_UNDEF_MGR( VOIP_MGR_DSPCODECSTART ), }, 

	// Protocol - RTP 
	//! @addtogroup VOIP_PROTOCOL_RTP
	//! @ingroup VOIP_CONTROL
	{ M_BODY_MGR( VOIP_MGR_SET_SESSION, TstVoipMgrSession, ch_id ), },
	{ M_BODY_MGR( VOIP_MGR_UNSET_SESSION, TstVoipCfg, ch_id ), },
	{ M_NORMAL_MGR( VOIP_MGR_SETRTPSESSIONSTATE, TstVoipRtpSessionState, ch_id ), }, 
	{ M_NORMAL_MGR( VOIP_MGR_RTP_CFG, TstVoipCfg, ch_id ), },
	{ M_NORMAL_MGR( VOIP_MGR_HOLD, TstVoipCfg, ch_id ), }, 
	{ M_NORMALBODY_MGR( VOIP_MGR_CTRL_RTPSESSION, TstVoipCfg, ch_id ), },
	{ M_FETCH_MGR( VOIP_MGR_CTRL_TRANSESSION_ID, TstVoipCfg, ch_id ), },
	{ M_NORMAL_MGR( VOIP_MGR_SETCONFERENCE, TstVoipMgr3WayCfg, ch_id ), },	
	{ M_BODY_MGR( VOIP_MGR_GET_RTP_STATISTICS, TstVoipRtpStatistics, ch_id ), },
	{ M_FETCH_MGR( VOIP_MGR_GET_SESSION_STATISTICS, TstVoipSessionStatistics, ch_id ), },

	// Protocol - RTCP  
	//! @addtogroup VOIP_PROTOCOL_RTCP
	//! @ingroup VOIP_CONTROL
	{ M_BODY_MGR( VOIP_MGR_SET_RTCP_SESSION, TstVoipRtcpSession, ch_id ), }, 
	{ M_BODY_MGR( VOIP_MGR_UNSET_RTCP_SESSION, TstVoipCfg, ch_id ), },
	//{ M_NORMAL_MGR( VOIP_MGR_SET_RTCP_TX_INTERVAL, TstVoipValue, ch_id ), },	
	{ M_FETCH_MGR( VOIP_MGR_GET_RTCP_LOGGER, TstVoipRtcpLogger, ch_id ), },	

	// DSP - General  
	//! @addtogroup VOIP_DSP_GENERAL
	//! @ingroup VOIP_CONTROL
	{ M_NORMALBODY_MGR( VOIP_MGR_ON_HOOK_RE_INIT, TstVoipCfg, ch_id ), }, 
	{ M_NORMAL_MGR( VOIP_MGR_SET_VOICE_GAIN, TstVoipValue, ch_id ), }, 
	{ M_FETCH_MGR( VOIP_MGR_ENERGY_DETECT, TstVoipValue, ch_id ), }, 
	{ M_HOSTONLY_MGR( VOIP_MGR_GET_VOIP_EVENT ), }, 
	{ M_NORMALBODY_MGR( VOIP_MGR_FLUSH_VOIP_EVENT, TstFlushVoipEvent, ch_id ), }, 
	
	// DSP - Codec 
	//! @addtogroup VOIP_DSP_CODEC
	//! @ingroup VOIP_CONTROL
	{ M_BODY_MGR( VOIP_MGR_SETRTPPAYLOADTYPE, TstVoipPayLoadTypeConfig, ch_id ), }, 
	{ M_BODY_MGR( VOIP_MGR_DSPCODECSTOP, TstVoipValue, ch_id ), }, 
	
	// DSP - FAX and Modem  
	//! @addtogroup VOIP_DSP_FAXMODEM
	//! @ingroup VOIP_CONTROL
	{ M_NORMAL_MGR( VOIP_MGR_FAX_OFFHOOK, TstVoipCfg, ch_id ), }, 
	{ M_FETCH_MGR( VOIP_MGR_FAX_END_DETECT, TstVoipCfg, ch_id ), }, 
	{ M_NORMALBODY_MGR( VOIP_MGR_SET_FAX_MODEM_DET, TstVoipCfg, ch_id ), }, 
	{ M_FETCH_MGR( VOIP_MGR_FAX_DIS_DETECT, TstVoipCfg, ch_id ), },
	{ M_HOSTONLY_MGR( VOIP_MGR_FAX_DIS_TX_DETECT ), /*TstVoipCfg, ch_id ),*/ },
	{ M_HOSTONLY_MGR( VOIP_MGR_FAX_DIS_RX_DETECT ), /*TstVoipCfg, ch_id ),*/ },
	{ M_FETCH_MGR( VOIP_MGR_FAX_DCN_DETECT, TstVoipCfg, ch_id ), },
	{ M_FETCH_MGR( VOIP_MGR_FAX_DCN_TX_DETECT, TstVoipCfg, ch_id ), },
	{ M_FETCH_MGR( VOIP_MGR_FAX_DCN_RX_DETECT, TstVoipCfg, ch_id ), },
	{ M_NORMAL_MGR( VOIP_MGR_SET_ANSWERTONE_DET, TstVoipCfg, ch_id ), }, 
	{ M_NORMAL_MGR( VOIP_MGR_SET_FAX_SILENCE_DET, TstVoipCfg, ch_id ), }, 
	
	// DSP - LEC  
	//! @addtogroup VOIP_DSP_LEC
	//! @ingroup VOIP_CONTROL
	{ M_NORMAL_MGR( VOIP_MGR_SET_ECHO_TAIL_LENGTH, TstVoipValue, ch_id ), }, 
	{ M_NORMAL_MGR( VOIP_MGR_SET_G168_LEC_CFG, TstVoipCfg, ch_id ), }, 
	{ M_NORMAL_MGR( VOIP_MGR_SET_VBD_EC, TstVoipCfg, ch_id ), }, 
	{ M_STANDALONE_MGR( VOIP_MGR_GET_EC_DEBUG ), }, 
		
	// DSP - DTMF  
	//! @addtogroup VOIP_DSP_DTMF
	//! @ingroup VOIP_CONTROL
	{ M_NORMAL_MGR( VOIP_MGR_DTMF_DET_PARAM, TstDtmfDetPara, ch_id ), },  
	{ M_NORMAL_MGR( VOIP_MGR_DTMF_CFG, TstVoipCfg, ch_id ), },  
	{ M_NORMAL_MGR( VOIP_MGR_SET_DTMF_MODE, TstVoipCfg, ch_id ), }, 
	{ M_NORMAL_MGR( VOIP_MGR_SEND_RFC2833_PKT_CFG, TstVoip2833, ch_id ), }, 
	{ M_NORMAL_MGR( VOIP_MGR_SEND_RFC2833_BY_AP, TstVoipCfg, ch_id ), },  
	{ M_NORMAL_MGR( VOIP_MGR_PLAY_SIP_INFO, TstVoipValue, ch_id ), },  
	{ M_NORMAL_MGR( VOIP_MGR_LIMIT_MAX_RFC2833_DTMF_DURATION, TstVoip2833, ch_id ), },
	{ M_NORMAL_MGR( VOIP_MGR_SET_RFC2833_TX_VOLUME, TstVoip2833, ch_id ), },

	// DSP - Caller ID  
	//! @addtogroup VOIP_DSP_CALLERID
	//! @ingroup VOIP_CONTROL
	{ M_NORMAL_MGR( VOIP_MGR_DTMF_CID_GEN_CFG, TstVoipCID, ch_id ), }, 
	{ M_FETCH_MGR( VOIP_MGR_GET_CID_STATE_CFG, TstVoipCID, ch_id ), }, 
	{ M_NORMAL_MGR( VOIP_MGR_FSK_CID_GEN_CFG, TstVoipCID, ch_id ), }, 
	{ M_NORMAL_MGR( VOIP_MGR_SET_FSK_VMWI_STATE, TstVoipCID, ch_id ), }, 
	{ M_NORMAL_MGR( VOIP_MGR_SET_FSK_AREA, TstVoipCID, ch_id ), }, 
	{ M_NORMAL_MGR( VOIP_MGR_FSK_ALERT_GEN_CFG, TstVoipCID, ch_id ), }, 
	{ M_NORMAL_MGR( VOIP_MGR_SET_CID_DTMF_MODE, TstVoipCID, ch_id ), }, 
	{ M_NORMAL_MGR( VOIP_MGR_SET_CID_DET_MODE, TstVoipCfg, ch_id ), }, 
	{ M_FETCH_MGR( VOIP_MGR_GET_FSK_CID_STATE_CFG, TstVoipCID, ch_id ), }, 
	{ M_UNDEF_MGR( VOIP_MGR_SET_CID_FSK_GEN_MODE ), }, 
	{ M_NORMAL_MGR( VOIP_MGR_FSK_CID_VMWI_GEN_CFG, TstVoipCID, ch_id ), }, 
	{ M_NORMAL_MGR( VOIP_MGR_SET_FSK_CLID_PARA, TstVoipFskPara, ch_id ), },
	{ M_FETCH_MGR( VOIP_MGR_GET_FSK_CLID_PARA, TstVoipFskPara, ch_id ), },
	{ M_NORMAL_MGR( VOIP_MGR_FSK_CID_MDMF_GEN, TstFskClid, ch_id ), },
	
	// DSP - Tone  
	//! @addtogroup VOIP_DSP_TONE
	//! @ingroup VOIP_CONTROL
	{ M_NORMAL_MGR( VOIP_MGR_SETPLAYTONE, TstVoipPlayToneConfig, ch_id ), }, 
	{ M_NOCHANNEL_MGR( VOIP_MGR_SET_COUNTRY ), },
	{ M_NOCHANNEL_SND_MGR( VOIP_MGR_SET_COUNTRY_IMPEDANCE ), },
	{ M_NOCHANNEL_MGR( VOIP_MGR_SET_COUNTRY_TONE ), },
	{ M_NOCHANNEL_MGR( VOIP_MGR_SET_TONE_OF_CUSTOMIZE ), },  
	{ M_NOCHANNEL_MGR( VOIP_MGR_SET_CUST_TONE_PARAM ), }, 	// FIXED: Types of Realtek and Audiocodes are different 
	{ M_NOCHANNEL_MGR( VOIP_MGR_USE_CUST_TONE ), }, 	// FIXED: Types of Realtek and Audiocodes are different 
	{ M_NOCHANNEL_MGR( VOIP_MGR_SET_DIS_TONE_DET ), }, 

	// DSP - AGC  
	//! @addtogroup VOIP_DSP_AGC
	//! @ingroup VOIP_CONTROL
	{ M_NORMAL_MGR( VOIP_MGR_SET_SPK_AGC, TstVoipValue, ch_id ), },  
	{ M_NORMAL_MGR( VOIP_MGR_SET_SPK_AGC_LVL, TstVoipValue, ch_id ), },  
	{ M_NORMAL_MGR( VOIP_MGR_SET_SPK_AGC_GUP, TstVoipValue, ch_id ), }, 
	{ M_NORMAL_MGR( VOIP_MGR_SET_SPK_AGC_GDOWN, TstVoipValue, ch_id ), }, 
	{ M_NORMAL_MGR( VOIP_MGR_SET_MIC_AGC, TstVoipValue, ch_id ), }, 
	{ M_NORMAL_MGR( VOIP_MGR_SET_MIC_AGC_LVL, TstVoipValue, ch_id ), }, 
	{ M_NORMAL_MGR( VOIP_MGR_SET_MIC_AGC_GUP, TstVoipValue, ch_id ), }, 
	{ M_NORMAL_MGR( VOIP_MGR_SET_MIC_AGC_GDOWN, TstVoipValue, ch_id ), }, 

	// DSP - Pluse Dial 
	//! @addtogroup VOIP_DSP_PLUSEDIAL
	//! @ingroup VOIP_CONTROL
	//TH: for pulse dial config
	{ M_NORMAL_MGR( VOIP_MGR_SET_PULSE_DIGIT_DET, TstVoipCfg, ch_id ), },  
	{ M_NORMAL_MGR( VOIP_MGR_SET_DIAL_MODE, TstVoipCfg, ch_id ), },  
	{ M_FETCH_MGR( VOIP_MGR_GET_DIAL_MODE, TstVoipCfg, ch_id ), }, 
	{ M_NORMAL_MGR( VOIP_MGR_PULSE_DIAL_GEN_CFG, TstVoipValue, ch_id ), },  
	{ M_NORMAL_MGR( VOIP_MGR_GEN_PULSE_DIAL, TstVoipValue, ch_id ), },  

	// DSP - IVR  
	//! @addtogroup VOIP_DSP_IVR
	//! @ingroup VOIP_CONTROL
	{ M_FETCH_MGR( VOIP_MGR_PLAY_IVR, TstVoipPlayIVR_Header, ch_id ), }, 
	{ M_FETCH_MGR( VOIP_MGR_POLL_IVR, TstVoipPollIVR, ch_id ), }, 
	{ M_NORMAL_MGR( VOIP_MGR_STOP_IVR, TstVoipStopIVR, ch_id ), }, 
	
	// Driver - PCM ctrl  
	//! @addtogroup VOIP_DRIVER_PCM
	//! @ingroup VOIP_CONTROL
	{ M_NORMAL_MGR( VOIP_MGR_PCM_CFG, TstVoipCfg, ch_id ), },  
	{ M_NORMAL_MGR( VOIP_MGR_SET_BUS_DATA_FORMAT, TstVoipBusDataFormat, ch_id ), },  
	{ M_NORMAL_MGR( VOIP_MGR_SET_PCM_TIMESLOT, TstVoipPcmTimeslot, ch_id ), },  
	{ M_STANDALONE_MGR( VOIP_MGR_SET_PCM_LOOP_MODE ), }, 
	
	// Driver - SLIC  
	//! @addtogroup VOIP_DRIVER_SLIC
	//! @ingroup VOIP_CONTROL
	{ M_NORMAL_SND_MGR( VOIP_MGR_SLIC_RING, TstVoipSlicRing, ch_id ), },  
	{ M_UNDEF_MGR( VOIP_MGR_SLIC_TONE ), }, 
	{ M_NORMAL_SND_MGR( VOIP_MGR_SLIC_RESTART, TstVoipSlicRestart, ch_id ), },  
	{ M_FETCH_SND_MGR( VOIP_MGR_GET_SLIC_REG_VAL, TstVoipSlicReg, ch_id ), },  
	{ M_NORMAL_MGR( VOIP_MGR_SET_SLIC_TX_GAIN, TstVoipValue, ch_id ), },  
	{ M_NORMAL_MGR( VOIP_MGR_SET_SLIC_RX_GAIN, TstVoipValue, ch_id ), },  
	{ M_NORMAL_MGR( VOIP_MGR_SET_FLASH_HOOK_TIME, TstVoipHook, ch_id ), },  
	{ M_NORMALBODY_MGR( VOIP_MGR_SET_SLIC_RING_CADENCE, TstVoipValue, ch_id ), },  
	{ M_NORMAL_SND_MGR( VOIP_MGR_SET_SLIC_RING_FRQ_AMP, TstVoipValue, ch_id ), },
	{ M_NOCHANNEL_SND_MGR( VOIP_MGR_SET_IMPEDANCE ), },
	{ M_FETCH_SND_MGR( VOIP_MGR_SET_SLIC_REG_VAL, TstVoipSlicReg, ch_id ), },  
	{ M_FETCH_SND_MGR( VOIP_MGR_GET_SLIC_STAT, TstVoipCfg, ch_id ), }, 
	{ M_NORMAL_MGR( VOIP_MGR_SLIC_ONHOOK_ACTION, TstVoipCfg, ch_id ), },  	// only used in AudioCodes
	{ M_NORMAL_MGR( VOIP_MGR_SLIC_OFFHOOK_ACTION, TstVoipCfg, ch_id ), },  	// only used in AudioCodes
	{ M_FETCH_SND_MGR( VOIP_MGR_LINE_CHECK, TstVoipValue, ch_id ), },  
	{ M_NORMAL_MGR( VOIP_MGR_HOOK_FIFO_IN, TstVoipCfg, ch_id ), },  
	{ M_FETCH_SND_MGR( VOIP_MGR_GET_SLIC_RAM_VAL, TstVoipSlicRam, ch_id ), }, 
	{ M_FETCH_SND_MGR( VOIP_MGR_SET_SLIC_RAM_VAL, TstVoipSlicRam, ch_id ), },  

	{ M_NOCHANNEL_MGR( VOIP_MGR_SET_RING_DETECTION ), }, 
	{ M_NORMAL_SND_MGR( VOIP_MGR_SET_FXS_FXO_LOOPBACK, TstVoipCfg, ch_id ), }, 
	{ M_NORMAL_SND_MGR( VOIP_MGR_SET_SLIC_ONHOOK_TRANS_PCM_START, TstVoipCfg, ch_id ), }, 

	{ M_NORMAL_SND_MGR( VOIP_MGR_SET_SLIC_LINE_VOLTAGE, TstVoipValue, ch_id ), },  
	{ M_NORMAL_MGR( VOIP_MGR_GEN_SLIC_CPC, TstVoipCfg, ch_id ), },

	// Driver - DAA  
	//! @addtogroup VOIP_DRIVER_DAA
	//! @ingroup VOIP_CONTROL
	{ M_FETCH_SND_MGR( VOIP_MGR_DAA_RING, TstVoipCfg, ch_id ), }, 
	{ M_FETCH_SND_MGR( VOIP_MGR_DAA_OFF_HOOK, TstVoipCfg, ch_id ), }, 
	{ M_NORMAL_SND_MGR( VOIP_MGR_DAA_ON_HOOK, TstVoipCfg, ch_id ), }, 
	{ M_FETCH_SND_MGR( VOIP_MGR_GET_DAA_LINE_VOLTAGE, TstVoipValue, ch_id ), },
	{ M_NOCHANNEL_SND_MGR( VOIP_MGR_SET_DAA_TX_GAIN ), }, 
	{ M_NOCHANNEL_SND_MGR( VOIP_MGR_SET_DAA_RX_GAIN ), }, 
	{ M_NORMAL_MGR( VOIP_MGR_SET_DAA_ISR_FLOW, TstVoipValue, ch_id ), }, 
	{ M_NORMAL_MGR( VOIP_MGR_GET_DAA_ISR_FLOW, TstVoipValue, ch_id ), }, 
	{ M_NORMAL_MGR( VOIP_MGR_SET_DAA_PCM_HOLD_CFG, TstVoipValue, ch_id ), }, 
	{ M_FETCH_MGR( VOIP_MGR_GET_DAA_BUSY_TONE_STATUS, TstVoipValue, ch_id ), }, 
	{ M_FETCH_MGR( VOIP_MGR_GET_DAA_CALLER_ID, TstVoipCID, daa_id ), }, 
	{ M_FETCH_MGR( VOIP_MGR_GET_DAA_USED_BY_WHICH_SLIC, TstVoipValue, ch_id ), }, 
	{ M_NORMAL_MGR( VOIP_MGR_FXO_ON_HOOK, TstVoipCfg, ch_id ), }, 
	{ M_NORMAL_MGR( VOIP_MGR_FXO_OFF_HOOK, TstVoipCfg, ch_id ), }, 

	// Driver - GPIO  
	//! @addtogroup VOIP_DRIVER_GPIO
	//! @ingroup VOIP_CONTROL
	{ M_HOSTONLY_MGR( VOIP_MGR_GPIO ), }, 
	{ M_NORMAL_MGR( VOIP_MGR_SET_LED_DISPLAY, TstVoipLedDisplay, ch_id ), }, 
	{ M_NORMAL_MGR( VOIP_MGR_SET_SLIC_RELAY, TstVoipSlicRelay, ch_id ), }, 
	
	// Driver - Networking  
	//! @addtogroup VOIP_DRIVER_NETWORK
	//! @ingroup VOIP_CONTROL
	{ M_HOSTONLY_MGR( VOIP_MGR_8305_SWITCH_VAL ), }, 
	{ M_HOSTONLY_MGR( VOIP_MGR_WAN_VLAN_TAG ), }, 
	{ M_HOSTONLY_MGR( VOIP_MGR_BRIDGE_MODE ), },  
	{ M_HOSTONLY_MGR( VOIP_MGR_SET_DSCP_PRIORITY ), }, 
	{ M_HOSTONLY_MGR( VOIP_MGR_WAN_2_VLAN_TAG ), }, 
	{ M_HOSTONLY_MGR( VOIP_MGR_WAN_3_VLAN_TAG ), }, 
	//add by Tim
	{ M_HOSTONLY_MGR( VOIP_MGR_SET_WAN_CLONE_MAC ), }, 
	{ M_HOSTONLY_MGR( VOIP_MGR_BANDWIDTH_MGR ), }, 
	{ M_HOSTONLY_MGR( VOIP_MGR_GET_PORT_LINK_STATUS ), }, 
	{ M_HOSTONLY_MGR( VOIP_MGR_SET_RTP_TOS ), }, 
	{ M_HOSTONLY_MGR( VOIP_MGR_SET_RTP_DSCP ), }, 	
	{ M_HOSTONLY_MGR( VOIP_MGR_SET_SIP_TOS ), }, 
	{ M_HOSTONLY_MGR( VOIP_MGR_SET_SIP_DSCP ), },
	{ M_HOSTONLY_MGR( VOIP_MGR_PORT_DISABLE ), }, 
	{ M_HOSTONLY_MGR( VOIP_MGR_PORT_PRIORITY ), }, 
	{ M_HOSTONLY_MGR( VOIP_MGR_PORT_DISABLE_FLOWCONTROL ), }, 
	
	// Driver - DECT  
	//! @addtogroup VOIP_DRIVER_DECT
	//! @ingroup VOIP_CONTROL
	{ M_HOSTONLY_MGR( VOIP_MGR_DECT_SET_POWER ), }, 
	{ M_HOSTONLY_MGR( VOIP_MGR_DECT_GET_POWER ), }, 
	{ M_HOSTONLY_MGR( VOIP_MGR_DECT_GET_PAGE ), }, 
	{ M_HOSTONLY_MGR( VOIP_MGR_DECT_SET_LED ), },
	
	// Miscellanous  
	//! @addtogroup VOIP_MISC
	//! @ingroup VOIP_CONTROL
	{ M_NORMAL_MGR( VOIP_MGR_SIP_REGISTER, TstVoipCfg, ch_id ), },
	{ M_HOSTONLY_MGR( VOIP_MGR_GET_FEATURE ), },
	{ M_FETCH_MGR( VOIP_MGR_VOIP_RESOURCE_CHECK, TstVoipCfg, ch_id ), },
	{ M_STANDALONE_MGR( VOIP_MGR_SET_FW_UPDATE ), },
		
	// IP Phone - keypad, LCM, Codec, LED and etc  
	//! @addtogroup VOIP_IPPHONE
	//! @ingroup VOIP_CONTROL
	{ M_STANDALONE_MGR( VOIP_MGR_CTL_KEYPAD ), }, 
	{ M_STANDALONE_MGR( VOIP_MGR_CTL_LCM ), }, 
	{ M_STANDALONE_MGR( VOIP_MGR_CTL_VOICE_PATH ), },
	{ M_STANDALONE_MGR( VOIP_MGR_CTL_LED ), },
	{ M_STANDALONE_MGR( VOIP_MGR_CTL_MISC ), },

	// Debug  
	//! @addtogroup VOIP_DEBUG
	//! @ingroup VOIP_CONTROL
	{ M_BODY_MGR( VOIP_MGR_DEBUG, TstVoipValue, ch_id ), },
	{ M_STANDALONE_MGR( VOIP_MGR_VOICE_PLAY ), },
	{ M_UNDEF_MGR( VOIP_MGR_GET_T38_PCMIN ), },
	{ M_UNDEF_MGR( VOIP_MGR_GET_T38_PACKETIN ), },
	{ M_STANDALONE_MGR( VOIP_MGR_SET_GETDATA_MODE ), },
	{ M_STANDALONE_MGR( VOIP_MGR_IPHONE_TEST ), },
	{ M_NOCHANNEL_MGR( VOIP_MGR_PRINT ), },
	{ M_NOCHANNEL_MGR( VOIP_MGR_COP3_CONIFG ), },
	
	// Ethernet DSP
	//! @addtogroup VOIP_ETHERNET_DSP
	//! @ingroup VOIP_CONTROL
	{ M_HOSTONLY_MGR( VOIP_MGR_SET_DSP_ID_TO_DSP ), },
	{ M_HOSTONLY_MGR( VOIP_MGR_SET_DSP_PHY_ID ), },
	{ M_HOSTONLY_MGR( VOIP_MGR_CHECK_DSP_ALL_SW_READY ), },
	{ M_HOSTONLY_MGR( VOIP_MGR_COMPLETE_DEFER_INIT ), },


	// DSP - Codec 
	//! @ingroup VOIP_CONTROL	
	{ M_NORMAL_MGR( VOIP_MGR_SET_VAD_CNG_THRESHOLD, TstVoipThresVadCngConfig, ch_id), },

	// DSP - DTMF  
	//! @addtogroup VOIP_DSP_DTMF
	//! @ingroup VOIP_CONTROL	
	{ M_NORMAL_MGR( VOIP_MGR_FAX_MODEM_RFC2833_CFG, TstVoipCfg, ch_id ), },
	{ M_NORMAL_MGR( VOIP_MGR_RFC2833_PKT_INTERVAL_CFG, TstVoipCfg, ch_id ), },
	
	// DSP - Caller ID  
	//! @addtogroup VOIP_DSP_CALLERID
	//! @ingroup VOIP_CONTROL
	{ M_NORMAL_MGR( VOIP_MGR_STOP_CID, TstVoipCfg, ch_id ), },
	
};

#define VOIP_MGR_TABLE_SIZE		( sizeof( voip_mgr_table ) / sizeof( voip_mgr_table[ 0 ] ) )

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
int last_process_seqno = -1;
/* Note: the addr of para should be 4 byte-align to avoid emulate opcode xxx */ 
       int do_voip_mgr_set_ctl(unsigned short cmd, unsigned char* para, unsigned short length, unsigned short seq_no)
#else
static int do_voip_mgr_set_ctl(int cmd, void *user, unsigned int len)
#endif
{
#ifdef 	CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	extern int do_voip_mgr_ctl_in_host( int cmd, void *user, unsigned int len, const voip_mgr_entry_t *p_entry );
#endif
	int index;
	const voip_mgr_entry_t *p_entry; 
	
	if( cmd <= VOIP_MGR_BASE_CTL || cmd >= VOIP_MGR_SET_MAX ) 
		goto label_not_voip_mgr_cmd;
	
	index = cmd - ( VOIP_MGR_BASE_CTL + 1 );
	
	if( index >= VOIP_MGR_TABLE_SIZE ) {
		PRINT_MSG( "cmd is not within table\n" );
		
		goto label_not_voip_mgr_cmd;
	}
	
	p_entry = &voip_mgr_table[ index ];

#if 0	// move to check_voip_mgr_table() 
	if( p_entry ->cmd != cmd ) {
		PRINT_MSG( "cmd is not match with table's one\n" );
		goto label_not_voip_mgr_cmd;
	}
#endif 
	
#ifdef 	CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	return do_voip_mgr_ctl_in_host( cmd, user, len, p_entry );
#else
	if( p_entry ->do_mgr ) {
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
		return p_entry ->do_mgr( cmd, para, length, seq_no );
		last_process_seqno = seq_no;
#else
		return p_entry ->do_mgr( cmd, user, len, 0 );
#endif
	}
#endif
	
	PRINT_Y("IOCTL command %d has no do_mgr\n", cmd);
	
	return -ENXIO;

label_not_voip_mgr_cmd:

	PRINT_MSG("IOCTL no %d command meet\n", cmd);
	
	return -EINVAL;
}

#if 0
#if ! defined (AUDIOCODES_VOIP)
int do_voip_mgr_get_ctl(struct sock *sk, int cmd, void *user, int *len)
{
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
  struct RTK_TRAP_profile *myptr;
  TstVoipMgrSession stVoipMgrSession;
#endif
  switch(cmd)
  {
	case VOIP_MGR_GET_SESSION:
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
		PRINT_MSG("GET SESSION\n");
		copy_from_user(&stVoipMgrSession, (TstVoipMgrSession *)user, sizeof(TstVoipMgrSession));
		if( get_filter(stVoipMgrSession.ch_id, myptr)!=0 ) {
			stVoipMgrSession.result = -1;
      			copy_to_user(user, &stVoipMgrSession, sizeof(TstVoipMgrSession));
			break;
		}

		stVoipMgrSession.ip_src_addr = myptr->ip_src_addr;
		stVoipMgrSession.ip_dst_addr = myptr->ip_dst_addr;
		stVoipMgrSession.udp_src_port= myptr->udp_src_port;
		stVoipMgrSession.udp_dst_port= myptr->udp_dst_port;
		stVoipMgrSession.protocol = myptr->protocol;
		stVoipMgrSession.result = 0;
		copy_to_user(user, &stVoipMgrSession, sizeof(TstVoipMgrSession));
		break;
#else
		PRINT_MSG("Not defined in 865x platform\n");
#endif
	default:
		break;
  }
  return 0;
 }

#else /*AUDIOCODES_VOIP*/

int do_voip_mgr_get_ctl(struct sock *sk, int cmd, void *user, int *len)
{
 	switch (cmd)
  	{

		default:
		break;

	}
	return 0;
}

#endif /*AUDIOCODES_VOIP*/
#endif

// ------------------------------------------------------------------
// ------------------------------------------------------------------
static int __init check_voip_mgr_table( void )
{
	int index, cmd;
	const voip_mgr_entry_t *p_entry;
#if defined( CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST )
	int type_size = 0, type_size_auto = 0;
#endif
	
	// set initial values for checking  
	cmd = VOIP_MGR_BASE_CTL + 1;
	index = cmd - ( VOIP_MGR_BASE_CTL + 1 );
	
	for( ; cmd < VOIP_MGR_SET_MAX; index ++, cmd ++ ) {
		
		p_entry = &voip_mgr_table[ index ];
		
		// check table cmd and index 
		if( p_entry ->cmd != cmd )
			PRINT_MSG( "voip_mgr_table bad cmd %d on %d\n", cmd, index );
		
#if defined( CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST )
		// max type_size
		if( type_size < p_entry ->type_size )
			type_size = p_entry ->type_size;
		// max type_size of auto
		if( ( p_entry ->flags & MF_AUTOFW ) &&
			type_size_auto < p_entry ->type_size )
		{
			type_size_auto = p_entry ->type_size;
		}
#endif
	}
	
	// print some message 
	PRINT_MSG( "Check voip_mgr_table done.\n" );
	PRINT_MSG( "\ttable size=%d, range=%d-%d\n", 
					VOIP_MGR_TABLE_SIZE, 
					VOIP_MGR_BASE_CTL + 1, VOIP_MGR_SET_MAX );
#if defined( CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST )
	PRINT_MSG( "\tmax type_size=%d\n", type_size );
	PRINT_MSG( "\tmax type_size_auto=%d\n", type_size_auto );
#endif
	
	return 0;
}

voip_initcall( check_voip_mgr_table );

// ------------------------------------------------------------------
// ------------------------ IOCTL interface -------------------------
// ------------------------------------------------------------------
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP

#define MGR_IOCTL_DEV_NAME	"mgr-ioc"
#define MGR_IOCTL_PRIV_NR	1		// for future extension 

typedef struct {
	wait_queue_head_t wq;
	int event_exist;
} mgr_priv_t;

static mgr_priv_t mgr_priv[ MGR_IOCTL_PRIV_NR ];

void mgr_wakeup( void )
{
	// VoIP event should wakeup wait queue 
	mgr_priv[ 0 ].event_exist = 1;
	wake_up_interruptible( &mgr_priv[ 0 ].wq );
}

static int mgr_open( struct inode *node, struct file *filp )
{
	filp->private_data = &mgr_priv[ 0 ];
	
	//printk( "mgr_open\n" );
	
	return 0;
}

static int mgr_close( struct inode *node, struct file *filp )
{
	//mgr_priv_t * const priv = filp->private_data;
	
	//printk( "mgr_close\n" );
	
	return 0;
}

static int mgr_ioctl( struct inode *node, struct file *filp,
	unsigned int cmd, unsigned long arg )
{
	//mgr_priv_t * const priv = filp->private_data;
	const unsigned int nr = VOIP_MGR_IOC_NR( cmd );
	const unsigned int size = VOIP_MGR_IOC_SIZE( cmd );
	
	//printk( "mgr_ioctl cmd=%08x (nr=%04x,size=%04x) arg=%08lx\n", 
	//			cmd, nr, size, arg );
	
	return do_voip_mgr_set_ctl( nr, ( void * )arg, size );
}

static unsigned int mgr_poll( struct file *filp, struct poll_table_struct *wait )
{
	mgr_priv_t * const priv = filp->private_data;
	
	//printk( "mgr_poll, filp=%p, &priv ->=%p, wait=%p, priv ->event_exist=%d\n", 
	//			filp, &priv ->wq, wait, priv ->event_exist );
	
	poll_wait( filp, &priv ->wq, wait );
	
	if( priv ->event_exist ) {
		priv ->event_exist = 0;
		return POLLIN | POLLRDNORM;	// readable 
	} else
		return 0;
}

static struct file_operations mgr_fops = {
	open:		mgr_open,
	release:	mgr_close,
	ioctl:		mgr_ioctl,
	poll:		mgr_poll,
};

static int __init voip_mgr_dev_init( void )
{
	int ret;
	int i;
	
	// initial priv data 
	for( i = 0; i < MGR_IOCTL_PRIV_NR; i ++ ) {
		init_waitqueue_head( &mgr_priv[ i ].wq );
		mgr_priv[ i ].event_exist = 0;
	}
	
	// register dev 
	ret = register_voip_chrdev( VOIP_DEV_MGR_IOCTL, 1, MGR_IOCTL_DEV_NAME, &mgr_fops );
	
	if( ret < 0 )
		printk( "register data dump dev fail\n" );
	
	return 0;
}

static void __exit voip_mgr_dev_exit( void )
{
	unregister_voip_chrdev( VOIP_DEV_MGR_IOCTL, 1 );
}

voip_initcall( voip_mgr_dev_init );
voip_exitcall( voip_mgr_dev_exit );

#endif // !CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP

