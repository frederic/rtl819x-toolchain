#include <linux/string.h>
#include <linux/version.h>
#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_init.h"
#include "voip_ipc.h"

#ifdef CONFIG_RTK_VOIP_COPROCESS_DSP_IS_HOST
#include "aipc_ctrl_cpu.h"
#include "aipc_data_cpu.h"
#else
#include "aipc_ctrl_dsp.h"
#include "aipc_data_dsp.h"
#endif

typedef enum {
	ASHM_2DSP_CTRL,
	ASHM_2CPU_EVENT,
	ASHM_2DSP_MBOX,
	ASHM_2CPU_MBOX,
} aipc_shm_t;

static inline aipc_shm_t get_aipc_shm_type( uint8 protocol )
{
	switch( protocol )
	{
#ifdef CONFIG_RTK_VOIP_COPROCESS_DSP_IS_HOST
	case IPC_PROT_CTRL:					// a) host --> dsp 
	case IPC_PROT_ACK:					// b) host --> dsp 
	case IPC_PROT_MIRROR_ACK:			// c) host --> dsp
	case IPC_PROT_RPC_ACK:				// d) host --> dsp	
		return ASHM_2DSP_CTRL;
		
	case IPC_PROT_VOICE_TO_DSP:			//    host --> dsp 
	case IPC_PROT_T38_TO_DSP:			//    host --> dsp 
	case IPC_PROT_MIRROR:				// c) host <-- dsp
	case IPC_PROT_RPC:					// d) host <-- dsp
	default:
		return ASHM_2DSP_MBOX;
		
#elif defined( CONFIG_RTK_VOIP_COPROCESS_DSP_IS_DSP )
	case IPC_PROT_RESP:					// a) host <-- dsp 
	case IPC_PROT_EVENT:				// b) host <-- dsp 
		return ASHM_2CPU_EVENT;

	case IPC_PROT_VOICE_TO_HOST:		//    host <-- dsp 
	case IPC_PROT_T38_TO_HOST:			//    host <-- dsp 
	default:
		return ASHM_2CPU_MBOX;
#endif
	}
}

ipc_ctrl_pkt_t *coprocessor_dsp_tx_allocate( unsigned int *len, 
											void **ipc_priv, uint8 protocol )
{
	// ipc_priv will pass to coprocessor_start_xmit() 
	switch( get_aipc_shm_type( protocol ) ) {
#ifdef CONFIG_RTK_VOIP_COPROCESS_DSP_IS_HOST
	case ASHM_2DSP_CTRL:
		if( ( *ipc_priv = aipc_ctrl_alloc( TYPE_POST ) ) == NULL )
			return NULL;
			
		return ( ipc_ctrl_pkt_t * )
				( ( ( aipc_ctrl_buf_t * )( *ipc_priv ) ) ->buf );
		
	case ASHM_2DSP_MBOX:
		*ipc_priv = aipc_todsp_bc_dequeue();
		return ( ipc_ctrl_pkt_t * )( *ipc_priv );
#else
	case ASHM_2CPU_EVENT:
		if( ( *ipc_priv = aipc_event_alloc( EVENT_POST ) ) == NULL )
			return NULL;
			
		return ( ipc_ctrl_pkt_t * )
				( ( ( aipc_event_buf_t * )( *ipc_priv ) ) ->buf );
		
	case ASHM_2CPU_MBOX:
		*ipc_priv = aipc_tocpu_bc_dequeue();
		return ( ipc_ctrl_pkt_t * )( *ipc_priv );
#endif
	
	default:
		break;
	}

	return NULL;
}

void coprocessor_dsp_fill_tx_header( ipc_ctrl_pkt_t *ipc_pkt,
								const TstTxPktCtrl* txCtrl, uint8 protocol )
{
#if 0	// nothing! 
	switch( get_aipc_shm_type( protocol ) ) {
#ifdef CONFIG_RTK_VOIP_COPROCESS_DSP_IS_HOST
	case ASHM_2DSP_CTRL:
		break;
	case ASHM_2DSP_MBOX:
		break;
#else
	case ASHM_2CPU_EVENT:
		break;
	case ASHM_2CPU_MBOX:
		break;
#endif
	}
#endif 
}

void coprocessor_start_xmit( void *ipc_priv, uint8 protocol )
{
	switch( get_aipc_shm_type( protocol ) ) {
#ifdef CONFIG_RTK_VOIP_COPROCESS_DSP_IS_HOST
	case ASHM_2DSP_CTRL:
		aipc_ctrl_xmit( ipc_priv, TYPE_POST, NULL );
		break;
		
	case ASHM_2DSP_MBOX:
		aipc_todsp_mb_enqueue( ipc_priv );
		break;
#else
	case ASHM_2CPU_EVENT:
		aipc_event_xmit( ipc_priv, EVENT_POST, NULL );
		break;
		
	case ASHM_2CPU_MBOX:
		aipc_tocpu_mb_enqueue( ipc_priv );
		break;
#endif

	default:
		break;
	}
}

