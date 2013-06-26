/*******************************************************************************
*	CPU		Linux AIPC Header File
*******************************************************************************/

#ifndef __AIPC_DATA_CPU_H__
#define __AIPC_DATA_CPU_H__

#include "soc_type.h"
#include "aipc_shm.h"

/*****************************************************************************
*   Macro Definitions
*****************************************************************************/


/*****************************************************************************
*   Data Structure
*****************************************************************************/


/*****************************************************************************
*   Export Function
*****************************************************************************/
/*	
*	Function name:
*		aipc_todsp_mbox_send
*	Description:
*		Send Data throughput mbox
*		This function is process context
*	Parameters:
*		void *data	:	Data start address
*	Return:
*		OK	: success
*		NOK	: fail
*/
int		aipc_todsp_mbox_send( void *data );

/*	
*	Function name:
*		aipc_todsp_bc_dequeue
*	Description:
*		Get available buffer from mailbox(todsp direction)
*		This function is process context
*	Parameters:
*		None
*	Return:
*		None NULL	: start address of available mail (available buffer address)
*		NULL	 	: no available buffer
*/
void *	aipc_todsp_bc_dequeue( void );

/*	
*	Function name:
*		aipc_todsp_mb_enqueue
*	Description:
*		Return buffer to mailbox(todsp direction)
*		This function is process context
*	Parameters:
*		void *dp	: buffer which will be return to mailbox
*	Return:
*		None
*/
void	aipc_todsp_mb_enqueue( void * dp );



/*	
*	Function name:
*		aipc_tocpu_mbox_recv
*	Description:
*		Receive Data from mbox
*		This function is ISR context. Receiving RTP packets in CPU<-DSP direction
*		uses interrupt mechanism. DSP sending RTP packets in constant time and 
*		frequency is small. DSP notifies CPU about packet arriving only when need.
*	Parameters:
*		//u32_t int_id:	parameters from ISR. doesn't need to handle
*		//void *data	:	parameters from ISR. doesn't need to handle
*	Return:
*		OK	: success
*		NOK	: fail
*/
int		aipc_tocpu_mbox_recv( void );


/*	
*	Function name:
*		aipc_tocpu_mb_dequeue
*	Description:
*		Get available buffer from mailbox(tocpu direction)
*		This function is interrupt context (to be invoked in ISR or DSR)
*	Parameters:
*		None
*	Return:
*		None NULL	: start address of available mail (available buffer address)
*		NULL	 	: no available buffer
*/
void *	aipc_tocpu_mb_dequeue( void );

/*	
*	Function name:
*		aipc_todsp_mb_enqueue
*	Description:
*		Return buffer to mailbox(tocpu direction)
*		This function is interrupt context (to be invoked in ISR or DSR)
*	Parameters:
*		void *dp	: buffer which will be return to mailbox
*	Return:
*		None
*/
void	aipc_tocpu_bc_enqueue( void * dp );


/*****************************************************************************
*   Function
*****************************************************************************/
void *	aipc_tocpu_mb_dequeue( void );
void	aipc_tocpu_bc_enqueue( void * dp );
void *	aipc_todsp_bc_dequeue( void );
void	aipc_todsp_mb_enqueue( void * dp );
int		aipc_int_sendto_dsp( u32_t int_id );
void	aipc_int_dsp_hiq_enqueue( u32_t int_id );
void	aipc_int_dsp_lowq_enqueue( u32_t int_id );
int		aipc_int_cpu_hiq_dequeue( u32_t *fid );
int		aipc_int_cpu_lowq_dequeue( u32_t *fid );
int		aipc_int_cpu_hiq_empty( void );
int		aipc_int_cpu_lowq_empty( void );
int		aipc_int_dsp_hiq_full( void );
int		aipc_int_dsp_lowq_full( void );
//void 	aipc_exec_callback( u32_t cmd , void *data );


//callback function
#if 0
typedef int ( *aipc_callback )( u32_t cmd , void *data );

typedef struct {
	u32_t			cmd;
	aipc_callback	do_mgr;
} aipc_entry_t;

#define AIPC_MGR_BASE  0		//It can be started from any number

enum {
	AIPC_MGR_TOCPU_MBOX_RECV = AIPC_MGR_BASE+1,
	AIPC_MGR_MAX
}

static aipc_entry_t aipc_callback_table[] = {
	//DSP send RTP packet to CPU using interrupt
	{ AIPC_MGR_TOCPU_MBOX_RECV 	, aipc_tocpu_mbox_recv  	},
	{ AIPC_MGR_MAX , NULL }
}
#endif


/*****************************************************************************
*   External Function
*****************************************************************************/
extern void aipc_L2_pkt_proc(unsigned char* pkt);


/*****************************************************************************
*   Debug Function
*****************************************************************************/

#endif
