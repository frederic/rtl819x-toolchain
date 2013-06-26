/*******************************************************************************
*	CPU		Linux AIPC Control Plane Header File
*******************************************************************************/

#ifndef __AIPC_CTRL_CPU_H__
#define __AIPC_CTRL_CPU_H__

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
*		aipc_ctrl_send
*	Description:
*		Send command from CPU to DSP
*		This function is process context
*		This function composes of aipc_ctrl_alloc() + aipc_ctrl_xmit() 
*	Parameters:
*		cmd_type_t cmd_type	: 	POST command or POST_GET command
*		u8_t *cmd_buf		:	Command buffer start address
*	Return:
*		OK	: success
*		NOK	: fail
*/
int aipc_ctrl_send( cmd_type_t cmd_type, u8_t *cmd_buf );

/*	
*	Function name:
*		aipc_ctrl_alloc
*	Description:
*		Allocate ctrl buffer to fill 
*		This function is process context
*	Parameters:
*		cmd_type_t cmd_type	: 	POST command or POST_GET command
*	Return:
*		Control buffer context 
*	Comments:
*		Free buffer action is automatically done by aipc_ctrl_proc()
*/
aipc_ctrl_buf_t *aipc_ctrl_alloc( cmd_type_t cmd_type );

/*	
*	Function name:
*		aipc_ctrl_xmit
*	Description:
*		Transmit ctrl buffer  
*		This function is process context
*	Parameters:
*		aipc_ctrl_buf_t *ctrl_buf : Point to ctrl buffer 
*		cmd_type_t cmd_type	: 	POST command or POST_GET command
*		u8_t *post_get_buf : If cmd_type is POST_GET, it will copy buf to here 
*	Return:
*		OK	: success
*		NOK	: fail
*/
int aipc_ctrl_xmit( aipc_ctrl_buf_t *ctrl_buf, cmd_type_t cmd_type, u8_t *post_get_buf );

/*	
*	Function name:
*		aipc_event_proc
*	Description:
*		Process event from DSP to CPU
*		This function is process context
*	Parameters:
*		None
*	Return:
*		OK	: success
*		NOK	: fail
*/
int aipc_event_proc( void );

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

