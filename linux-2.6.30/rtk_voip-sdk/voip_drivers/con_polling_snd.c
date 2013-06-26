#include <linux/string.h>
#include "rtk_voip.h"
#include "voip_ipc.h"
#include "con_register.h"
#include "con_ipc_mirror.h"
#include "con_ipc_mirror_host.h"

static int Polling_Mirror_SND_con( voip_con_t *p_con, voip_snd_t *p_snd )
{
	//int status;		/* 1:off-hook, 0:on-hook, -1: ignore */
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
	//unsigned char ring_state;
#ifdef HW_FXO_REVERSAL_DET
	//unsigned char daa_reversal;
#endif
#endif
	unsigned long flags;
	ipc_mirror_priv_data_t * const p_mirror_data = ( ipc_mirror_priv_data_t * )p_con ->ipc.priv;
	ipc_mirror_help_state_t * const p_mirror_hstate = &( p_mirror_data ->hstate );
	ipc_mirror_union_data_t mirror_data_backup;
	
	memcpy( &mirror_data_backup, &( p_mirror_data ->udata ), sizeof( ipc_mirror_union_data_t ) );
	
	// --------------------------------------------
	// get SND status in host side 
	save_flags(flags);
	cli();
	
	switch( p_snd ->snd_type ) {
	case SND_TYPE_FXS:
		// 1.9530
		// 1.8419 inline + dmem
		// 1.6065 (.5519) inline + dmem (gpio/desc)
		// 1.4192 (.7122) inline + no debug 
		p_mirror_data ->udata.slic.hook_status = 
				p_snd ->fxs_ops ->SLIC_Get_Hook_Status( p_snd, 1 ); /* 1:off-hook  0:on-hook */
		
		if( p_mirror_hstate ->slic.f_ring_chk ) {
			// RPC FXS_Ring() will make f_ring_chk = 1
			p_mirror_data ->udata.slic.ringing =
				p_snd ->fxs_ops ->FXS_Check_Ring( p_snd );
		}
		
		if( p_mirror_hstate ->slic.f_SendNTTCAR_chk ) {
			p_mirror_data ->udata.slic.SendNTTCAR_chk =
				p_snd ->fxs_ops ->SendNTTCAR_check( p_snd, p_mirror_hstate ->slic.SendNTTCAR_check_timeout );
		}
		
		break;
	
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
	case SND_TYPE_DAA:
		// 3.0881 (2.0226) inline + no debug 
		p_mirror_data ->udata.daa.hook_status =
				p_snd ->daa_ops ->DAA_Hook_Status( p_snd, 1 );
#ifdef HW_FXO_REVERSAL_DET
		p_mirror_data ->udata.daa.pol_rev_det =
				p_snd ->daa_ops ->DAA_Polarity_Reversal_Det( p_snd );
#endif
#ifdef HW_FXO_BAT_DROP_OUT
		p_mirror_data ->udata.daa.bat_dropout_det = 
				p_snd ->daa_ops ->DAA_Bat_DropOut_Det( p_snd );
#endif
		p_mirror_data ->udata.daa.pos_neg_ring =
				p_snd ->daa_ops ->DAA_Positive_Negative_Ring_Detect( p_snd );
		
		if( p_mirror_data ->udata.daa.hook_status ) {
			p_mirror_data ->udata.daa.polarity =
					p_snd ->daa_ops ->DAA_Get_Polarity( p_snd );
			p_mirror_data ->udata.daa.line_voltage =
					p_snd ->daa_ops ->DAA_Get_Line_Voltage( p_snd );
		}
		
		break;
#endif
	
	case SND_TYPE_VDAA:
		//status = p_snd ->vdaa_ops ->virtual_daa_hook_detect( p_snd ); /* 1:off-hook  0:on-hook */
		break;
		
	case SND_TYPE_DECT:	// dect process handle it
	case SND_TYPE_AC:	// ui process this key. 
	default:
		//status = -1;
		break;
	}
	
	restore_flags(flags);

	// --------------------------------------------
	// additional process out of cli() 
	switch( p_snd ->snd_type ) {
	case SND_TYPE_FXS:
		if( p_mirror_hstate ->slic.f_ring_chk ) {
			
			//printk( "FXS_Check_Ring: %d\n", p_mirror_data ->udata.slic.ringing );
			
			// make sure ring stete 0 -> 1 -> 0
			if( p_mirror_data ->udata.slic.ringing )
				p_mirror_hstate ->slic.f_ring_occur = 1;
			
			if( p_mirror_hstate ->slic.f_ring_occur &&
				p_mirror_data ->udata.slic.ringing == 0 )
			{
				// clean 
				p_mirror_hstate ->slic.f_ring_chk = 0;
				p_mirror_hstate ->slic.f_ring_occur = 0;
			}
		}
		
		if( p_mirror_hstate ->slic.f_SendNTTCAR_chk &&
			p_mirror_data ->udata.slic.SendNTTCAR_chk )
		{
			p_mirror_hstate ->slic.f_SendNTTCAR_chk = 0;
		}

		break;
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
	case SND_TYPE_DAA:
		break;
#endif
	case SND_TYPE_VDAA:
	case SND_TYPE_DECT:	// dect process handle it
	case SND_TYPE_AC:	// ui process this key. 
	default:
		break;
	}
		
	// --------------------------------------------
	// check SND data changed? 
	switch( p_snd ->snd_type ) {
	case SND_TYPE_FXS:
		if( memcmp( &mirror_data_backup.slic, &( p_mirror_data ->udata.slic ), 
					sizeof( p_mirror_data ->udata.slic ) ) )
		{
			p_con ->con_ops ->ipc_mirror_request( p_con, IPC_MIR_SLIC_ALL, 
					&( p_mirror_data ->udata.slic ), sizeof( p_mirror_data ->udata.slic ) );
		}
		break;
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
	case SND_TYPE_DAA:
		if( memcmp( &mirror_data_backup.daa, &( p_mirror_data ->udata.daa ), 
					sizeof( p_mirror_data ->udata.daa ) ) )
		{
			p_con ->con_ops ->ipc_mirror_request( p_con, IPC_MIR_DAA_ALL, 
					&( p_mirror_data ->udata.daa ), sizeof( p_mirror_data ->udata.daa ) );
		}
		break;
#endif	
	case SND_TYPE_VDAA:
	case SND_TYPE_DECT:	// dect process handle it
	case SND_TYPE_AC:	// ui process this key. 
	default:
		break;
	}		
}

void Polling_Mirror_SND(unsigned long data)
{
	int i;
	voip_con_t * const p_con_start = ( voip_con_t * )data;
	voip_con_t *p_con;
	voip_snd_t *p_snd;
	int status;
	
	for( i = 0, p_con = p_con_start; i < CON_CH_NUM; i ++, p_con ++ ) 
	{
		p_snd = p_con ->snd_ptr;
		
		if( !p_snd || !p_con ->ipc.priv || p_con ->ipc.priv == MAGIC_IPC_PRIV_SHADOW )
			continue;
		
		status = Polling_Mirror_SND_con( p_con, p_snd );
	}
}

