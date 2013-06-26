#include <linux/string.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_control.h"
#include "voip_params.h"
#include "voip_debug.h"
#include "voip_init.h"

#include "dsp_main.h"	// V152_SwitchCodecNow()
#include "RtpTransmitter.h"

#include "silence_det.h"

#include "../voip_manager/voip_mgr_events.h"

#include "v152_api.h"
#include "v152_priv.h"

v152_vars_t v152_vars;

// check if bit number is greater then session number 
CT_ASSERT( sizeof( v152_vars.bitsEnable ) * 8 >= DSP_RTK_SS_NUM );
CT_ASSERT( sizeof( v152_vars.bitsSwitchCodec ) * 8 >= DSP_RTK_SS_NUM );

CT_ASSERT( REASON_RTP_VBD != 0 );	// If assert is met, change REASON_RTP_VBD number to be non-zero. 

int V152_Initialize( void )
{
	memset( &v152_vars, 0, sizeof( v152_vars ) );
	
	return 0;
}

void V152_InitializeSession( int sid, int enable )
{
	if( sid >= DSP_RTK_SS_NUM )
		return;
		
	memset( &v152_vars.session[ sid ], 0, sizeof( v152_vars.session[ 0 ] ) );
	
	//CT_ASSERT( REASON_RTP_VBD != 0 ); 	// If assert is met, uncomment below code or change REASON_RTP_VBD number. 
	//v152_vars.session[ sid ].prevRtp = REASON_RTP_AUDIO;
	
	v152_vars.bitsSwitchCodec &= ~( 1 << sid );
	
	if( enable )
		v152_vars.bitsEnable |= ( 1 << sid );
	else
		v152_vars.bitsEnable &= ~( 1 << sid );
}

int V152_CheckSessionEnabled( int sid )
{
	v152_session_vars_t *pvar = &v152_vars.session[ sid ];
	int ret = 0;
	
	if( sid >= DSP_RTK_SS_NUM )
		return 0;
	
	// check enable
	if( !( v152_vars.bitsEnable & ( 1 << sid ) ) )
		return 0;
	
	return 1;
}

static inline int V152_ReasonNotification( int sid, v152reason reason )
{
	int chid;
	int i;
	
	static const struct {
		v152reason reason;
		VoipEventID event2;
	} reason2event[] = {
		{ REASON_RTP_VBD,			VEID_FAXMDM_V152_RTP_VBD },
		{ REASON_RTP_AUDIO,			VEID_FAXMDM_V152_RTP_AUDIO },
		{ REASON_SIG_VBD_CED,		VEID_FAXMDM_V152_SIG_CED },
		{ REASON_TDM_SIG_END,		VEID_FAXMDM_V152_TDM_SIG_END },
		{ REASON_VOC_BI_SILENCE,	VEID_FAXMDM_V152_BI_SILENCE },
	};
	
#define SIZE_REASON_2_EVENT		( sizeof( reason2event ) / sizeof( reason2event[ 0 ] ) )
	
	for( i = 0; i < SIZE_REASON_2_EVENT; i ++ ) {
		
		if( reason2event[ i ].reason == reason ) {
			
			chid = chanInfo_GetChannelbySession( sid );
			
			voip_event_fax_modem_in( chid, reason2event[ i ].event2 );
			
			return 1;
		}
	}
	
	return 0;
#undef SIZE_REASON_2_EVENT
}

int V152_StateTransition( int sid, v152reason reason )
{
	v152_session_vars_t *pvar = &v152_vars.session[ sid ];
	int ret = 0;
	
	if( sid >= DSP_RTK_SS_NUM )
		return 0;
	
	// check enable
	if( !( v152_vars.bitsEnable & ( 1 << sid ) ) )
		return 0;
	
	// avoid transition erroneously (P.20)
	switch( pvar ->state ) {
	case ST_AUDIO_ING:
	case ST_VBD_ING:
		return 0;
		
	default:
		break;
	}
	
	// save & check rtp reason  
	switch( reason ) {
	case REASON_RTP_VBD:
		if( pvar ->prevRtp == REASON_RTP_VBD )
			return 0;
		break;
		
	case REASON_RTP_AUDIO:
		if( pvar ->prevRtp != REASON_RTP_VBD )	// don't check REASON_RTP_AUDIO, because inital value is 0 
			return 0;
		break;
		
	default:
		goto label_save_and_check_rtp_reason_done;
		break;
	}

	pvar ->prevRtp = reason;

label_save_and_check_rtp_reason_done:
	
	// check to do state transition 
	switch( pvar ->state ) {
	case ST_AUDIO:
		if( reason <= REASON_SEPARATOR ) {
			pvar ->state = ST_VBD_ING;
			pvar ->reasonVBD = ( pvar ->reasonVBD << 8 ) | ( reason & 0xFF );
			v152_vars.bitsSwitchCodec |= ( 1 << sid );
			ret = 1;
		}
		break;
		
	case ST_VBD:
		if( reason > REASON_SEPARATOR ) {
			pvar ->state = ST_AUDIO_ING;
			pvar ->reasonAudio = ( pvar ->reasonAudio << 8 ) | ( reason & 0xFF );
			v152_vars.bitsSwitchCodec |= ( 1 << sid );
			ret = 1;
		}
		break;
		
	default:
		break;
	}
	
	// send reason as FAX/modem events to users 
	if( ret == 1 )
		V152_ReasonNotification( sid, reason );
	
	return ret;
}

int V152_ConfirmStateTransition( int sid, v152st state )
{
	v152_session_vars_t *pvar = &v152_vars.session[ sid ];
	
	if( sid >= DSP_RTK_SS_NUM )
		return 0;
	
	// check enable
	if( !( v152_vars.bitsEnable & ( 1 << sid ) ) )
		return 0;
	
	// filter state 
	switch( state ) {
	case ST_AUDIO_ING:
	case ST_VBD_ING:
		break;
		
	default:
		return 0;
	}
	
	// confirm state 
	if( pvar ->state != state )
		return 0;
	
	// state transition 
	switch( pvar ->state ) {
	case ST_AUDIO_ING:
		pvar ->state = ST_AUDIO;
		break;
		
	case ST_VBD_ING:
		pvar ->state = ST_VBD;
		pvar ->silence = 0;
		break;
		
	default:
		return 0;
	}
	
	return 1;
}

#if 0	// use silence detection now
int V152_RecordBidirectionalSilence( int sid, int32 energy_in, int32 energy_out )
{
	v152_session_vars_t *pvar = &v152_vars.session[ sid ];
	
	if( sid >= DSP_RTK_SS_NUM )
		return 0;
	
	// check enable
	if( !( v152_vars.bitsEnable & ( 1 << sid ) ) )
		return 0;
	
	// check state 
	if( pvar ->state != ST_VBD )
		return 0;
	
	// check energy 
	if( energy_in < 35 && energy_out < 35 )
	{
		pvar ->silence ++;
		
		//printk( "%d ", pvar ->silence );
	} else {
		//if( pvar ->silence > 100 )
		//	printk( "%d-%d ", energy_in, energy_out );
			
		pvar ->silence = 0;	// check continual silence 
	}
	
	return 1;
}

int V152_CheckBidirectionalSilence( int sid )
{
	v152_session_vars_t *pvar = &v152_vars.session[ sid ];
	
	if( sid >= DSP_RTK_SS_NUM )
		return 0;
	
	// check enable
	if( !( v152_vars.bitsEnable & ( 1 << sid ) ) )
		return 0;
	
	// check state 
	if( pvar ->state != ST_VBD )
		return 0;
	
	// check silence period (T.30 T2 = 6 +- 1 sec)
	if( pvar ->silence > 10 * 100 ) {	// 10 sec (to be more safe) 
		pvar ->silence = 0;
		return 1;
	} 
	
	return 0;
}
#endif

static inline void V152_SwitchCodecNow( int s_id, int stAudio )
{
	extern TstVoipPayLoadTypeConfig astVoipPayLoadTypeConfig[];
	const TstVoipPayLoadTypeConfig *pstVoipPayLoadTypeConfig;
	
	uint32 ch_id = chanInfo_GetChannelbySession( s_id );
	unsigned long flags;
	
	pstVoipPayLoadTypeConfig = &astVoipPayLoadTypeConfig[ s_id ];
	
	save_flags(flags); cli();
	
	if( stAudio ) {	// Audio need do more (check do_mgr_VOIP_MGR_ON_HOOK_RE_INIT())
		extern uint32 chanInfo_GetChannelbySession(uint32 sid);
		extern void Init_CED_Det(unsigned char CH);	//thlin+ 2006-02-08
		extern void AEC_re_init(unsigned int chid);
		extern void NLP_g168_init(unsigned int chid);
		extern void NR_re_init(unsigned int chid);
		extern void LEC_re_init(unsigned char chid);
		extern int reinit_answer_tone_det(unsigned int chid);
		
		uint32 chid = chanInfo_GetChannelbySession( s_id );

		Init_CED_Det( chid );
		reinit_answer_tone_det( chid );
		
#ifdef SUPPORT_LEC_G168_ISR
		LEC_re_init( chid );
#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
		NLP_g168_init( chid );
#endif
//#ifdef EXPER_AEC
		AEC_re_init( chid );
//#endif
#ifdef EXPER_NR
		NR_re_init( chid );
#endif
	}
	
	RtpTx_addTimestamp( s_id );	// add older codec's packet timestamp 
	DSP_CodecRestart(ch_id, s_id,
					 ( stAudio ? pstVoipPayLoadTypeConfig ->uPktFormat : pstVoipPayLoadTypeConfig ->uPktFormat_vbd ),
					 ( stAudio ? pstVoipPayLoadTypeConfig ->nFramePerPacket : pstVoipPayLoadTypeConfig ->nFramePerPacket_vbd ),	
					 pstVoipPayLoadTypeConfig ->nG723Type,
					 ( stAudio ? pstVoipPayLoadTypeConfig ->bVAD : 0 ),	// turn off VAD
					 pstVoipPayLoadTypeConfig ->bPLC,
					 pstVoipPayLoadTypeConfig ->nJitterDelay,
					 pstVoipPayLoadTypeConfig ->nMaxDelay,
					 ( stAudio ? pstVoipPayLoadTypeConfig ->nJitterFactor : 13 ),	// turn off JBC
					 pstVoipPayLoadTypeConfig ->nG726Packing,
					 pstVoipPayLoadTypeConfig ->nG7111Mode,
					 pstVoipPayLoadTypeConfig ->nPcmMode);
	RtpTx_subTimestamp( s_id );	// sub newer codec's packet timestamp
	
	enable_silence_det( s_id, ( stAudio ? 0 : 1 ) );
	
	restore_flags(flags);
}

void V152_SwitchCodecIfNecessary( void )
{
	// This function will be called in period of 10ms, so it should be
	// very concise!! 
	
	int i;
	uint32 bit;

	if( v152_vars.bitsSwitchCodec == 0 )
		return;
		
	for( i = 0, bit = 0x01L; i < DSP_RTK_SS_NUM; i ++, bit <<= 1 )
		if( v152_vars.bitsSwitchCodec & bit ) {
			const int stAudio = 
				( v152_vars.session[ i ].state == ST_AUDIO ||
				  v152_vars.session[ i ].state == ST_AUDIO_ING ? 1 : 0 );
			V152_SwitchCodecNow( i, stAudio );
		}
		
	v152_vars.bitsSwitchCodec = 0;
}

int V152_CheckIfSendVBD( int sid )
{
	v152_session_vars_t *pvar = &v152_vars.session[ sid ];
	int ret;
	
	if( sid >= DSP_RTK_SS_NUM )
		return 0;
		
	if( !( v152_vars.bitsEnable & ( 1 << sid ) ) )
		return 0;
		
	switch( pvar ->state ) {
	case ST_AUDIO:
	case ST_AUDIO_ING:
		ret = 0;
		break;
		
	case ST_VBD:
	case ST_VBD_ING:
		ret = 1;
		break;
		
	default:
		return 0;
		break;
	}
	
	if( v152_vars.bitsSwitchCodec & ( 1 << sid ) )	// still switching 
		ret ^= 1;
	
	return ret;
}

voip_initcall( V152_Initialize );

