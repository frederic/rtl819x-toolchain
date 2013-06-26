/*******************************************************************************
*	From DSP to CPU		eCos Send function
*******************************************************************************/
#include "aipc_data_dsp.h"


#if 0
int
aipc_int_sendto_cpu( u32_t int_id )		//only use 64 
{
	int retval = NOK;
	u32_t state;
	u32_t add_hiq = !(int_id & 0x01);	//add even to hi, odd to low queue
	
	if( invalid(int_id) ) 
		return retval;		
		
	if( add_hiq ){
		while( 1 ){
			cyg_interrupt_disable();

			if( aipc_int_cpu_hiq_full() ){
				cyg_interrupt_enable();
				continue;			
			}
			else{
				aipc_int_cpu_hiq_enqueue( int_id );
				assert( AIPC_CPU_INT_IP );
				retval = OK;
				cyg_interrupt_enable();
				
				break;
			}
		}
	}
	else{
		while( 1 ){
			cyg_interrupt_disable();

			if( aipc_int_cpu_lowq_full() ){
				cyg_interrupt_enable();
				continue;			
			}
			else{
				aipc_int_cpu_lowq_enqueue( int_id );
				assert( AIPC_CPU_INT_IP );
				retval = OK;
				cyg_interrupt_enable();
				
				break;
			}
		}
	}
	
	return retval;
}
#endif

int	
aipc_int_cpu_hiq_full( void )
{
	if((aipc_data_shm.tocpu_int_q.tocpu_int_hiq_del + TOCPU_HIQ_SIZE - aipc_data_shm.tocpu_int_q.tocpu_int_hiq_ins)
		%TOCPU_HIQ_SIZE == (TOCPU_HIQ_SIZE-1))
	{
		return TRUE;
	} else
		return FALSE;
}
	
int
aipc_int_cpu_lowq_full( void )
{
	if((aipc_data_shm.tocpu_int_q.tocpu_int_lowq_del + TOCPU_HIQ_SIZE - aipc_data_shm.tocpu_int_q.tocpu_int_lowq_ins)
		%TOCPU_HIQ_SIZE == (TOCPU_HIQ_SIZE-1))
	{
		return TRUE;
	} else
		return FALSE;
}

void
aipc_int_cpu_hiq_enqueue( u32_t int_id )
{
	aipc_data_shm.tocpu_int_q.tocpu_int_hiq[aipc_data_shm.tocpu_int_q.tocpu_int_hiq_ins] = int_id;
	aipc_data_shm.tocpu_int_q.tocpu_int_hiq_ins = (aipc_data_shm.tocpu_int_q.tocpu_int_hiq_ins+1)%TOCPU_HIQ_SIZE;
}

void
aipc_int_cpu_lowq_enqueue( u32_t int_id )
{
	aipc_data_shm.tocpu_int_q.tocpu_int_lowq[aipc_data_shm.tocpu_int_q.tocpu_int_lowq_ins] = int_id;
	aipc_data_shm.tocpu_int_q.tocpu_int_lowq_ins = (aipc_data_shm.tocpu_int_q.tocpu_int_lowq_ins+1)%TOCPU_LOWQ_SIZE;
}


