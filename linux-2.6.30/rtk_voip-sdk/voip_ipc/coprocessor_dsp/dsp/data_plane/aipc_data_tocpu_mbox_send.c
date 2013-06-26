/*******************************************************************************
*	From DSP to CPU		eCos Mail Box  Send function
*******************************************************************************/
#ifdef WIN32
#include <string.h>
#else
#include <linux/string.h>
#endif

#include "aipc_data_dsp.h"


int
aipc_tocpu_mbox_send( u32_t int_id , void *data )
{
	void *dp=NULL;
	
	if( !data )
		return NOK;

	if( (dp = aipc_tocpu_bc_dequeue()) == NULL ) 
			return NOK;    //buffer unavailable, dequeue failed

	memcpy( dp , data , TOCPU_MAIL_SIZE );
	
	aipc_tocpu_mb_enqueue( dp );
	
	//aipc_int_sendto_cpu( int_id );
	
	return OK;
}

void
aipc_tocpu_mb_enqueue( void * dp )
{
	aipc_data_shm.tocpu_mbox.tocpu_mb[ aipc_data_shm.tocpu_mbox.tocpu_mb_ins ] = dp;
	aipc_data_shm.tocpu_mbox.tocpu_mb_ins = (aipc_data_shm.tocpu_mbox.tocpu_mb_ins+1) % TOCPU_MAIL_TOTAL;
}

void *
aipc_tocpu_bc_dequeue( void )
{
	void *retp;
	u32_t next_bc_del = ( aipc_data_shm.tocpu_mbox.tocpu_bc_del + 1 ) % TOCPU_MAIL_TOTAL;
	
	if( aipc_data_shm.tocpu_mbox.tocpu_bc_del == 
		aipc_data_shm.tocpu_mbox.tocpu_bc_ins )
	{
		return NULL;  //Buffer Circulation is Empty
	} else{
		retp = aipc_data_shm.tocpu_mbox.tocpu_bc[ aipc_data_shm.tocpu_mbox.tocpu_bc_del ];
		aipc_data_shm.tocpu_mbox.tocpu_bc_del = next_bc_del;
		return retp;
	}
}


