/*******************************************************************************
*	From CPUto DSP		Linux Send function
*******************************************************************************/
#include "aipc_data_cpu.h"

#if 0
spinlock_t aipc_lock ; 

int
aipc_int_sendto_dsp( u32_t int_id )		//only use 64 
{
	int retval = NOK;
	
	u32_t state;
	u32_t add_hiq = !(int_id & 0x01);	//add even to hi, odd to low queue
	
	if( invalid(int_id) ) 
		return retval;		
	
	if( add_hiq ){
		while( 1 ){
			spin_lock_irqsave( &aipc_lock , state );  
	
			if( aipc_int_dsp_hiq_full() ){
					spin_unlock_store( &aipc_lock , state );
					continue;			
				}
				else{
					aipc_int_dsp_hiq_enqueue( int_id );
					assert( AIPC_DSP_INT_IP );
					retval = OK;
					spin_unlock_store( &aipc_lock , state );
					
					break;
				}
			}		
	}
	else {
		while( 1 ){
			spin_lock_irqsave( &aipc_lock , state );	
			
			if( aipc_int_dsp_lowq_full() ){
				spin_unlock_store( &aipc_lock , state );
				continue;			
			}
			else{
				aipc_int_dsp_lowq_enqueue( int_id );
				assert( AIPC_DSP_INT_IP );
				retval = OK;
				spin_unlock_store( &aipc_lock , state );
				
				break;
			}
		}
	}
	
	return retval;	
}
#endif

int	
aipc_int_dsp_hiq_full( void )
{
	if((aipc_data_shm.todsp_int_q.todsp_int_hiq_del + TODSP_HIQ_SIZE - aipc_data_shm.todsp_int_q.todsp_int_hiq_ins)
		%TODSP_HIQ_SIZE == (TODSP_HIQ_SIZE-1))
	{
		return TRUE;
	} else
		return FALSE;
}
	
int
aipc_int_dsp_lowq_full( void )
{
	if((aipc_data_shm.todsp_int_q.todsp_int_lowq_del + TODSP_HIQ_SIZE - aipc_data_shm.todsp_int_q.todsp_int_lowq_ins)
		%TODSP_HIQ_SIZE == (TODSP_HIQ_SIZE-1))
	{
		return TRUE;
	} else
		return FALSE;
}

void
aipc_int_dsp_hiq_enqueue( u32_t int_id )
{
	aipc_data_shm.todsp_int_q.todsp_int_hiq[aipc_data_shm.todsp_int_q.todsp_int_hiq_ins] = int_id;
	aipc_data_shm.todsp_int_q.todsp_int_hiq_ins = (aipc_data_shm.todsp_int_q.todsp_int_hiq_ins+1)%TODSP_HIQ_SIZE;
}

void
aipc_int_dsp_lowq_enqueue( u32_t int_id )
{
	aipc_data_shm.todsp_int_q.todsp_int_lowq[aipc_data_shm.todsp_int_q.todsp_int_lowq_ins] = int_id;
	aipc_data_shm.todsp_int_q.todsp_int_lowq_ins = (aipc_data_shm.todsp_int_q.todsp_int_lowq_ins+1)%TODSP_LOWQ_SIZE;
}


