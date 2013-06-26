#ifdef WIN32
#include <string.h>
#else
#include <linux/string.h>
#endif

#include "aipc_ctrl_cpu.h"

/*****************************************************************************
*	Global Variable
*****************************************************************************/
static int cmd_buf_idx = 0;

aipc_ctrl_buf_t *aipc_ctrl_alloc( cmd_type_t cmd_type )
{
	int i;
	
	for( i = 0; i < AIPC_CTRL_BUF_NUM; i ++ ) {

		if( aipc_ctrl_buf[ cmd_buf_idx ].own != OWN_NONE ) {

			if( ++ cmd_buf_idx == AIPC_CTRL_BUF_NUM )
				cmd_buf_idx = 0;

			continue;
		}

		aipc_ctrl_buf[cmd_buf_idx].cseq ++;
		aipc_ctrl_buf[cmd_buf_idx].size  = AIPC_CTRL_BUF_LEN/*cmd_size*/;
		
		if( cmd_type == TYPE_POST ){
			aipc_ctrl_buf[cmd_buf_idx].type	= TYPE_POST;
			aipc_ctrl_buf[cmd_buf_idx].own	= OWN_CPU_ALLOC;
		}
		else{	//cmd_type == TYPE_POST_GET
			aipc_ctrl_buf[cmd_buf_idx].type	= TYPE_POST_GET;
			aipc_ctrl_buf[cmd_buf_idx].own	= OWN_CPU_ALLOC;
		}

		return &aipc_ctrl_buf[cmd_buf_idx];
	}

	return NULL;
}

int aipc_ctrl_xmit( aipc_ctrl_buf_t *ctrl_buf, cmd_type_t cmd_type, u8_t *post_get_buf )
{
	if( cmd_type>TYPE_MAX || cmd_type<TYPE_POST || !ctrl_buf )
		return NOK;

	if( ctrl_buf ->own != OWN_CPU_ALLOC )
		return NOK;
	
	if( cmd_type == TYPE_POST ){
		//ctrl_buf ->type	= TYPE_POST;	// by allocate 
		ctrl_buf ->own	= OWN_DSP;
	}
	else{	//cmd_type == TYPE_POST_GET
		//ctrl_buf ->type	= TYPE_POST_GET;	// by allocate 
		ctrl_buf ->own	= OWN_DSP;
		while( ( volatile int )( ctrl_buf ->own ) == OWN_DSP );
		
		memcpy( post_get_buf, ctrl_buf ->buf, AIPC_CTRL_BUF_LEN );
        ctrl_buf ->type	= TYPE_POST;
        ctrl_buf ->own	= OWN_NONE;
	}

	return OK;
}

int aipc_ctrl_send( cmd_type_t cmd_type, u8_t *cmd_buf )
{
	aipc_ctrl_buf_t *ctrl_buff;
	
	ctrl_buff = aipc_ctrl_alloc( cmd_type );

	if( !ctrl_buff )
		return NOK;
	
	memcpy( ctrl_buff ->buf, cmd_buf , AIPC_CTRL_BUF_LEN );
	
	return aipc_ctrl_xmit( ctrl_buff, cmd_type, cmd_buf );
} 

static int event_buf_idx = 0;
void ( *aipc_tocpu_event_rx )( void *shm_pkt, unsigned long size);

int aipc_event_proc( void )
{
	int i;
	//unsigned short dsp_id;

	//while( aipc_event_need_proc() )
	for( i = 0; i < AIPC_EVENT_BUF_NUM; i ++ )
	{
		if( aipc_event_buf[ event_buf_idx ].own != OWN_CPU )
			break;
		
		if( aipc_event_buf[event_buf_idx].type == EVENT_POST ) {
			
			if( aipc_tocpu_event_rx )
			{
				aipc_tocpu_event_rx( &aipc_event_buf[ event_buf_idx ].buf, 
										aipc_event_buf[ event_buf_idx ].size );
			}
			
			aipc_event_buf[event_buf_idx].own = OWN_NONE;
		}
		else {  //aipc_event_buf[idx].type == EVENT_POST_GET
			//get_responce_packet(&aipc_event_buf[idx].buf, &dsp_id);
			aipc_event_buf[event_buf_idx].own = OWN_CPU;
		}
		
		// move to next 
		if( ++ event_buf_idx == AIPC_EVENT_BUF_NUM )
			event_buf_idx = 0;
	}
	
	return OK;
} 

int aipc_ctrl_init( void )
{
	cmd_buf_idx = 0;
	
	event_buf_idx = 0;

	return 0;
}
