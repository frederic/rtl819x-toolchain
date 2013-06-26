/*******************************************************************************
*	From CPU to DSP		Linux Mail Box Send function
*******************************************************************************/
#ifdef WIN32
#include <string.h>
#else
#include <linux/string.h>
#endif

#include "aipc_data_cpu.h"


int
aipc_todsp_mbox_send( void *data )
{
	void *dp=NULL;
	
	if( !data )
		return NOK;

	if( (dp = aipc_todsp_bc_dequeue()) == NULL ) 
			return NOK;    //buffer unavailable, dequeue failed

	memcpy( dp , data , sizeof(TODSP_MAIL_SIZE) );

	aipc_todsp_mb_enqueue( dp );
	
	return OK;
}

void
aipc_todsp_mb_enqueue( void * dp )
{
	aipc_data_shm.todsp_mbox.todsp_mb[aipc_data_shm.todsp_mbox.todsp_mb_ins] = dp;
	aipc_data_shm.todsp_mbox.todsp_mb_ins = (aipc_data_shm.todsp_mbox.todsp_mb_ins+1) % TODSP_MAIL_TOTAL;
}

void *
aipc_todsp_bc_dequeue( void )
{
	void *retp;
	u32_t next_bc_del = ( aipc_data_shm.todsp_mbox.todsp_bc_del + 1 ) % TODSP_MAIL_TOTAL;
	
	if( aipc_data_shm.todsp_mbox.todsp_bc_del == 
		aipc_data_shm.todsp_mbox.todsp_bc_ins )
	{
		return NULL;  //Buffer Circulation is Empty
	} else{
		retp = aipc_data_shm.todsp_mbox.todsp_bc[ aipc_data_shm.todsp_mbox.todsp_bc_del ];
		aipc_data_shm.todsp_mbox.todsp_bc_del = next_bc_del;
		return retp;
	}
}

