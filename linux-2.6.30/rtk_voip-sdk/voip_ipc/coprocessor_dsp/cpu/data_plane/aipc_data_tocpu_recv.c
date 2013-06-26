/*******************************************************************************
*	From DSP to CPU		Linux ISR function
*******************************************************************************/
#include "aipc_data_cpu.h"

#if 0
int
aipc_exec_callback( u32_t cmd , void *data )
{
	int index;
	aipc_entry_t *p_entry; 
	
	if( cmd <= AIPC_MGR_BASE || cmd >= AIPC_MGR_MAX ){
		return NOK;	
	}
	
	index = cmd - ( AIPC_MGR_BASE + 1 );
	
	p_entry = &aipc_callback_table[ index ];
	
	if( p_entry ->do_mgr ) {
		return p_entry ->do_mgr( cmd, data );
	}
}
#endif

//#define MAX_HANDLE_CNT 		//needs to specify this number
#ifndef MAX_HANDLE_CNT
#error "Need to define MAX_HANDLE_CNT"
#endif

#if 0
int
aipc_ISR( u32_t irq , void *data )
{
	int n = MAX_HANDLE_CNT;
	u32_t fid;

	interrupt_mask( irq );
	deassert( AIPC_CPU_INT_IP );

	while(1) {
		if( !aipc_int_cpu_hiq_empty() ){	//handle elements in hi queue
			for( ; n>0 ; n-- )
			{
				if( !aipc_int_cpu_hiq_empty() ){  //For hi queue	
					//Get element from hi queue
					if( OK == aipc_int_cpu_hiq_dequeue( &fid ))
						;//aipc_exec_callback( fid , data );  //Run callback
					else
						break;
				}
			}
		}
	
		if( !aipc_int_cpu_lowq_empty() ){	//handle elements in low queue
			for( ; n>0 ; n-- )
			{
				if( !aipc_int_cpu_lowq_empty() ){  //For low queue	
					//Get element from low queue
					if( OK == aipc_int_cpu_lowq_dequeue( &fid ) )
						;//aipc_exec_callback( fid , data );  //Run callback
					else
						break;
				}
			}
		}

		if( read_register( AIPC_CPU_INT_IP ) )
			deassert( AIPC_CPU_INT_IP );
		else
			break;
	}

	interrupt_unmask( irq );

	return IRQ_HANDLED;
}
#endif

int	
aipc_int_cpu_hiq_empty( void )
{
	if((aipc_data_shm.tocpu_int_q.tocpu_int_hiq_ins + TOCPU_HIQ_SIZE - aipc_data_shm.tocpu_int_q.tocpu_int_hiq_del)
			%TOCPU_HIQ_SIZE == 0)
		return TRUE;
	else
		return FALSE;
}

int
aipc_int_cpu_lowq_empty( void )
{
	if((aipc_data_shm.tocpu_int_q.tocpu_int_lowq_ins + TOCPU_LOWQ_SIZE - aipc_data_shm.tocpu_int_q.tocpu_int_lowq_del)
			%TOCPU_LOWQ_SIZE == 0)
		return TRUE;
	else
		return FALSE;
}

int
aipc_int_cpu_hiq_dequeue( u32_t *fid )
{
	if( !aipc_int_cpu_hiq_empty() ){
		*fid = aipc_data_shm.tocpu_int_q.tocpu_int_hiq[ aipc_data_shm.tocpu_int_q.tocpu_int_hiq_del ];
		aipc_data_shm.tocpu_int_q.tocpu_int_hiq_del = (aipc_data_shm.tocpu_int_q.tocpu_int_hiq_del-1)%TOCPU_HIQ_SIZE;
		return OK;
		}
	else
		return NOK;
}

int
aipc_int_cpu_lowq_dequeue( u32_t *fid )
{
	if( !aipc_int_cpu_lowq_empty() ){
		*fid = aipc_data_shm.tocpu_int_q.tocpu_int_lowq[ aipc_data_shm.tocpu_int_q.tocpu_int_lowq_del ];
		aipc_data_shm.tocpu_int_q.tocpu_int_lowq_del = (aipc_data_shm.tocpu_int_q.tocpu_int_lowq_del-1)%TOCPU_LOWQ_SIZE;
		return OK;
		}
	else
		return NOK;
}



