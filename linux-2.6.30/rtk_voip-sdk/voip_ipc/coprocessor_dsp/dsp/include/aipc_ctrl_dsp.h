/*******************************************************************************
*   DSP       eCos AIPC Control Plane Header File
*******************************************************************************/

#ifndef __AIPC_CTRL_DSP_H__
#define __AIPC_CTRL_DSP_H__

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
*		aipc_event_send
*	Description:
*		Send event from DSP to CPU
*		This function is process context
*	Parameters:
*		event_type_t event_type	: 	POST event or POST_GET event
*		u8_t *event_buf			:	Event buffer start address
*	Return:
*		OK	: success
*		NOK	: fail
*/
int aipc_event_send( event_type_t event_type , u8_t *event_buf );

/*	
*	Function name:
*		aipc_event_alloc
*	Description:
*		Get event context to send from DSP to CPU
*		This function is process context
*	Parameters:
*		event_type_t event_type	: 	POST event or POST_GET event
*	Return:
*		Event context 
*	Comments:
*		Free buffer action is automatically done by aipc_ctrl_proc()
*/
aipc_event_buf_t *aipc_event_alloc( event_type_t event_type );

/*	
*	Function name:
*		aipc_event_xmit
*	Description:
*		Transmit event buffer 
*		This function is process context
*	Parameters:
*		aipc_event_buf_t *event_buf : Point to event buffer 
*		event_type_t event_type	: 	POST command or POST_GET command
*		u8_t *post_get_buf : If cmd_type is POST_GET, it will copy buf to here 
*	Return:
*		OK	: success
*		NOK	: fail
*/
int aipc_event_xmit( aipc_event_buf_t *event_buf, event_type_t event_type, u8_t *post_get_buf );

/*	
*	Function name:
*		aipc_ctrl_proc
*	Description:
*		Process command from CPU to DSP
*		This function is process context
*	Parameters:
*		None
*	Return:
*		OK	: success
*		NOK	: fail
*/
int aipc_ctrl_proc( void );

/*****************************************************************************
*   Function
*****************************************************************************/
//void aipc_L2_pkt_proc(unsigned char* pkt);	//modify voip_dsp_L2_pkt_rx()


/*****************************************************************************
*   External Function
*****************************************************************************/


/*****************************************************************************
*   Debug Function
*****************************************************************************/


#endif

