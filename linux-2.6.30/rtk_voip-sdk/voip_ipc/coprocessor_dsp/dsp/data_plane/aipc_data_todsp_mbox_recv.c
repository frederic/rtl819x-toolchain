/*******************************************************************************
*	From CPUto DSP		eCos Mail Box Receive function
*******************************************************************************/
#ifdef WIN32
#include <string.h>
#else
#include <linux/string.h>
#endif

#include "aipc_data_dsp.h"

void ( *aipc_todsp_mbox_rx )( void *shm_pkt, unsigned long size);

/*
*	this could be called in DSP decoder DSR
*	it doesn't need to be called in ISR for reducing interrupt overhead
*/
int
aipc_todsp_mbox_recv( void ) 
{	
	void   *dp;
	
	while( (dp = aipc_todsp_mb_dequeue()) != NULL )
	{
		if( aipc_todsp_mbox_rx ) 
		{
			aipc_todsp_mbox_rx( dp, TODSP_MAIL_SIZE );
		}
			
		aipc_todsp_bc_enqueue( dp );
	}

	return OK;
}


void *	
aipc_todsp_mb_dequeue( void )
{
	void *retp;
	u32_t next_mb_del = ( aipc_data_shm.todsp_mbox.todsp_mb_del + 1 ) % TODSP_MAIL_TOTAL;
	
	if( aipc_data_shm.todsp_mbox.todsp_mb_del == 
		aipc_data_shm.todsp_mbox.todsp_mb_ins )
	{
		return NULL;  //mail box empty
	} else{
		retp = aipc_data_shm.todsp_mbox.todsp_mb[ aipc_data_shm.todsp_mbox.todsp_mb_del ];
		aipc_data_shm.todsp_mbox.todsp_mb_del = next_mb_del;
		return retp;
	}
}

void
aipc_todsp_bc_enqueue( void * dp )
{
	aipc_data_shm.todsp_mbox.todsp_bc[ aipc_data_shm.todsp_mbox.todsp_bc_ins ] = dp;
	aipc_data_shm.todsp_mbox.todsp_bc_ins = (aipc_data_shm.todsp_mbox.todsp_bc_ins+1) % TODSP_MAIL_TOTAL;
}


