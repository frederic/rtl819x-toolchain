#include "rtk_voip.h"

#include "voip_ipc.h"

#include "snd_mirror_define.h"

#include "con_register.h"

void ipc_mirror_parser( ipc_ctrl_pkt_t *ipc_ctrl )
{
	// retrieve con_ptr, only *mux* can use this!! 
	extern voip_con_t *get_voip_con_ptr( uint32 cch );
	
	extern int ipcSentMirrorAckPacket( unsigned short category, uint16 seq_no, void* mirror_ack_data, unsigned short mirror_ack_len );

	const mirror_all_content_t * const mirror_content = ( const mirror_all_content_t * )ipc_ctrl ->content;
	const uint16 cch = mirror_content ->cch;
	const mirror_slic_priv_t * const mirror_slic_priv = ( const mirror_slic_priv_t * )mirror_content ->data;
	const mirror_daa_priv_t * const mirror_daa_priv = ( const mirror_daa_priv_t * )mirror_content ->data;
	voip_con_t *p_con;
	voip_snd_t *p_snd;

#if 0
	int i;
	
	printk( "cch=%d data=", cch );
	
	for( i = 2; i < ipc_ctrl ->cont_len; i ++ )	// =2 to skip cch 
		printk( "%02X ", ipc_ctrl ->content[ i ] );
		
	printk( "\n" );
#endif
	
	// mirror ack 
	ipcSentMirrorAckPacket( ipc_ctrl ->category, ipc_ctrl ->sequence, NULL, 0 );
	
	// check cch 
	if( cch >= CON_CH_NUM )
		goto label_error;
	
	// retrieve con ptr 
	p_con = get_voip_con_ptr( cch );
	p_snd = p_con ->snd_ptr;
	
	switch( ipc_ctrl ->category ) {
	// --------------------------------------------
	// SLIC Mirror 
	// --------------------------------------------
	case IPC_MIR_SLIC_ALL:			// all 
		if( p_snd && p_snd ->fxs_ops && p_snd ->fxs_ops ->Mirror_SLIC_All )
			p_snd ->fxs_ops ->Mirror_SLIC_All( p_snd, mirror_slic_priv );
		break;
		
	// --------------------------------------------
	// DAA Mirror 
	// --------------------------------------------		
	case IPC_MIR_DAA_ALL:			// all 
		if( p_snd && p_snd ->daa_ops && p_snd ->daa_ops ->Mirror_DAA_All )
			p_snd ->daa_ops ->Mirror_DAA_All( p_snd, mirror_daa_priv );
		break;
	
	default:
		printk( "ipc_mirror_parser unexpected category: %u\n", ipc_ctrl ->category );
		break;
	}
	
	return;
	
label_error:
	return;
}


