/*******************************************************************************
*	From CPUto DSP		eCos ISR function
*******************************************************************************/
#include "aipc_data_dsp.h"

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

#if 0
//#define MAX_HANDLE_CNT 		//needs to specify this number
#ifndef MAX_HANDLE_CNT
#error "Need to define MAX_HANDLE_CNT"
#endif

int
aipc_ISR( u32_t irq , void *data )
{
	int n = MAX_HANDLE_CNT;
	u32_t fid;

	interrupt_mask( irq );
	deassert( AIPC_DSP_INT_IP );

	while(1) {
		if( !aipc_int_dsp_hiq_empty() ){	//handle elements in hi queue
			for( ; n>0 ; n-- )
			{
				if( !aipc_int_dsp_hiq_empty() ){  //For hi queue
					//Get element from hi queue
					if( OK == aipc_int_dsp_hiq_dequeue( &fid ))
						aipc_exec_callback( fid , data );  //Run callback
					else
						break;
				}
			}
		}
		if( !aipc_int_dsp_lowq_empty() ){	//handle elements in low queue
			for( ; n>0 ; n-- )
			{
				if( !aipc_int_dsp_lowq_empty() ){  //For low queue	
					//Get element from low queue
					if( OK == aipc_int_dsp_lowq_dequeue( &fid ))
						aipc_exec_callback( fid , data );  //Run callback
					else
						break;
				}
			}
		}

		if( read_register( AIPC_DSP_INT_IP ) )
			deassert( AIPC_DSP_INT_IP );
		else
			break;
	}
	interrupt_unmask( irq );

	return CYG_ISR_HANDLED;
}
#endif


int	
aipc_int_dsp_hiq_empty( void )
{
	if((aipc_data_shm.todsp_int_q.todsp_int_hiq_ins + TODSP_HIQ_SIZE - aipc_data_shm.todsp_int_q.todsp_int_hiq_del)
			%TODSP_HIQ_SIZE == 0)
		return TRUE;
	else
		return FALSE;
}

int
aipc_int_dsp_lowq_empty( void )
{
	if((aipc_data_shm.todsp_int_q.todsp_int_lowq_ins + TODSP_LOWQ_SIZE - aipc_data_shm.todsp_int_q.todsp_int_lowq_del)
			%TODSP_LOWQ_SIZE == 0)
		return TRUE;
	else
		return FALSE;
}

int
aipc_int_dsp_hiq_dequeue( u32_t *fid )
{
	if( !aipc_int_dsp_hiq_empty() ){
		*fid = aipc_data_shm.todsp_int_q.todsp_int_hiq[ aipc_data_shm.todsp_int_q.todsp_int_hiq_del ];
		aipc_data_shm.todsp_int_q.todsp_int_hiq_del = (aipc_data_shm.todsp_int_q.todsp_int_hiq_del-1)%TODSP_HIQ_SIZE;
		return OK;
		}
	else
		return NOK;
}

int
aipc_int_dsp_lowq_dequeue( u32_t *fid )
{
	if( !aipc_int_dsp_lowq_empty() ){
		*fid = aipc_data_shm.todsp_int_q.todsp_int_lowq[ aipc_data_shm.todsp_int_q.todsp_int_lowq_del ];
		aipc_data_shm.todsp_int_q.todsp_int_lowq_del = (aipc_data_shm.todsp_int_q.todsp_int_lowq_del-1)%TODSP_LOWQ_SIZE;
		return OK;
		}
	else
		return NOK;
}



