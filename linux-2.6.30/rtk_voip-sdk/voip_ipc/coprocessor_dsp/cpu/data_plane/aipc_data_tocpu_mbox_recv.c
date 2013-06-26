/*******************************************************************************
*	From DSP to CPU		Linux Mail Box Receive function
*******************************************************************************/
#ifdef WIN32
#include <string.h>
#else
#include <linux/string.h>
#endif

#include "aipc_data_cpu.h"

void ( *aipc_tocpu_mbox_rx )( void *shm_pkt, unsigned long size);

/*
*	For CPU<-DSP sending RTP packet case, DSP needs to send interrupt to notify CPU
*	about packet arrival. CPU receive RTP packet ing ISR context.
*	In this case, this function should be called in AIPC ISR.
*/
int
aipc_tocpu_mbox_recv( void )
{
	void   *dp;

	while( (dp = aipc_tocpu_mb_dequeue()) != NULL )
	{
		if( aipc_tocpu_mbox_rx ) 
		{
			aipc_tocpu_mbox_rx( dp, TOCPU_MAIL_SIZE );
		}
			
		aipc_tocpu_bc_enqueue( dp );
	}

	return 0;
}

void *	
aipc_tocpu_mb_dequeue( void )
{
	void *retp;
	u32_t next_mb_del = ( aipc_data_shm.tocpu_mbox.tocpu_mb_del + 1 ) % TOCPU_MAIL_TOTAL;
	
	if( aipc_data_shm.tocpu_mbox.tocpu_mb_del == 
		aipc_data_shm.tocpu_mbox.tocpu_mb_ins )
	{
		return NULL;  //mail box empty
	} else{
		retp = aipc_data_shm.tocpu_mbox.tocpu_mb[ aipc_data_shm.tocpu_mbox.tocpu_mb_del ];
		aipc_data_shm.tocpu_mbox.tocpu_mb_del = next_mb_del;
		return retp;
	}
}

void
aipc_tocpu_bc_enqueue( void * dp )
{
	aipc_data_shm.tocpu_mbox.tocpu_bc[ aipc_data_shm.tocpu_mbox.tocpu_bc_ins ] = dp;
	aipc_data_shm.tocpu_mbox.tocpu_bc_ins = (aipc_data_shm.tocpu_mbox.tocpu_bc_ins+1) % TOCPU_MAIL_TOTAL;
}


