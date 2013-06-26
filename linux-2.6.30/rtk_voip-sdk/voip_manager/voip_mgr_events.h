#ifndef __VOIP_MGR_EVENTS_H__
#define __VOIP_MGR_EVENTS_H__

#include "voip_types.h"
#include "voip_control.h"

typedef enum {
	VOIP_EVENT_IDX_DTMF_DIRTDM, 
	VOIP_EVENT_IDX_DTMF_DIRIP, 
	VOIP_EVENT_IDX_HOOK, 
	VOIP_EVENT_IDX_ENERGY, 
	VOIP_EVENT_IDX_DECT, 
	VOIP_EVENT_IDX_FAXMDM, 
	VOIP_EVENT_IDX_RFC2833_MID0, 
	VOIP_EVENT_IDX_RFC2833_MID1, 
	VOIP_EVENT_IDX_DSP_MID0, 
	VOIP_EVENT_IDX_DSP_MID1, 
	VOIP_EVENT_QUEUE_TOTAL_NUM,
} VoipEventQueueIdx;

// ------------------------------------ 
// event input (for kernel)
extern int voip_mgr_event_in( uint32 chid, VoipEventQueueIdx idx, VoipEventMask mask, 
						VoipEventID id, uint32 p0, uint32 p1 );

#define voip_event_dtmf_in( chid, dtmf, dir, energy, duration )							\
			voip_mgr_event_in( chid, 													\
					( dir ? VOIP_EVENT_IDX_DTMF_DIRIP : VOIP_EVENT_IDX_DTMF_DIRTDM ),	\
					( dir ? VEM_DIRIP : VEM_DIRTDM ),									\
					VEID_DTMF_WILDCARD | ( ( uint32 )( dtmf ) & 0x00FF ),				\
					energy, duration )

#define voip_event_hook_in( chid, hook )						\
			voip_mgr_event_in( chid, VOIP_EVENT_IDX_HOOK, 0,	\
					hook, 0, 0 )

#define voip_event_energy_in( chid, energy )					\
			voip_mgr_event_in( chid, VOIP_EVENT_IDX_ENERGY, 0,	\
					VEID_ENERGY, energy, 0 )

#define voip_event_dect_in( button )							\
			voip_mgr_event_in( 0, VOIP_EVENT_IDX_DECT, 0,		\
					button, 0, 0 )

#define voip_event_fax_modem_in( chid, faxmdm )							\
			do {														\
				extern int g_disable_announce_fax;						\
				extern int announce_SIP_event( const char *ev_str );	\
				extern uint32 fax_modem_det_mode[];						\
				extern int Set_Fax_Check_Flags_Timer( uint32 ch_id, uint32 input );	\
				if (g_disable_announce_fax!=0)							\
					announce_SIP_event( "ef\n" );						\
				if( ( faxmdm == VEID_FAXMDM_MODEM_LOCAL_DELAY ||		\
					  faxmdm == VEID_FAXMDM_FAX_DIS_RX ) && 			\
					( fax_modem_det_mode[ chid ] == 0 ) &&				\
					Set_Fax_Check_Flags_Timer( chid, faxmdm ) == 0 )	\
				{ /* do noghting */ } 									\
				else {													\
					voip_mgr_event_in( chid, VOIP_EVENT_IDX_FAXMDM, 0,	\
							faxmdm, 0, 0 );								\
				}														\
			} while( 0 )

#define voip_event_rfc2833_in( chid, sid, rfc2833_event )		\
			voip_mgr_event_in( chid, 							\
					( chid == sid ? VOIP_EVENT_IDX_RFC2833_MID0 : VOIP_EVENT_IDX_RFC2833_MID1 ),	\
					( chid == sid ? VEM_MID0 : VEM_MID1 ),		\
					VEID_RFC2833_RX_WILDCARD | ( ( uint32 )( rfc2833_event ) & 0x00FF ), 	\
					0, 0 )

#define voip_event_dsp_in( chid, sid, dsp_event )				\
			voip_mgr_event_in( chid,							\
					( chid == sid ? VOIP_EVENT_IDX_DSP_MID0 : VOIP_EVENT_IDX_DSP_MID1 ),	\
					( chid == sid ? VEM_MID0 : VEM_MID1 ),		\
					dsp_event, 0, 0 )

extern int voip_mgr_event_in_packed( const TstVoipEvent *pVoipEvent );

// ------------------------------------ 
// event output (for users ioctl) 
extern int voip_mgr_event_out( TstVoipEvent *pVoipEvent );

// ------------------------------------ 
// event flush (for users ioctl or kernel) 
extern int voip_mgr_event_flush( TstFlushVoipEvent *pFlushVoipEvent );

#define voip_event_flush_dtmf_fifo( chid, dir )								\
			{																\
				TstFlushVoipEvent FlushVoipEvent;							\
				FlushVoipEvent.ch_id = chid;								\
				FlushVoipEvent.type = VET_DTMF;								\
				FlushVoipEvent.mask = ( dir ? VEM_DIRIP : VEM_DIRTDM );		\
				voip_mgr_event_flush( &FlushVoipEvent );					\
			}

#define voip_event_flush_dect_fifo()										\
			{																\
				TstFlushVoipEvent FlushVoipEvent;							\
				FlushVoipEvent.ch_id = 0;									\
				FlushVoipEvent.type = VET_DECT;								\
				FlushVoipEvent.mask = 0;									\
				voip_mgr_event_flush( &FlushVoipEvent );					\
			}

#define voip_event_flush_fax_modem_fifo( chid, reset_chk )					\
			{																\
				extern void Reset_Fax_Check_Flags( uint32 );				\
				TstFlushVoipEvent FlushVoipEvent;							\
				if( reset_chk ) 											\
					Reset_Fax_Check_Flags( chid );							\
				FlushVoipEvent.ch_id = 0;									\
				FlushVoipEvent.type = VET_FAXMDM;							\
				FlushVoipEvent.mask = 0;									\
				voip_mgr_event_flush( &FlushVoipEvent );					\
			}


#endif /* __VOIP_MGR_EVENTS_H__ */

