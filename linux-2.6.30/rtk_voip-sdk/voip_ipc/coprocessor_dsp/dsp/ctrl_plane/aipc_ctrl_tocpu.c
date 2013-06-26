#ifdef WIN32
#include <string.h>
#else
#include <linux/string.h>
#endif

#include "aipc_ctrl_dsp.h"

/*****************************************************************************
*	Global Variable
*****************************************************************************/
static int event_buf_idx = 0;

aipc_event_buf_t *aipc_event_alloc( event_type_t event_type )
{
	int i;
	
	for( i = 0; i < AIPC_EVENT_BUF_NUM; i ++ ) {
		
		if( aipc_event_buf[ event_buf_idx ].own != OWN_NONE ) {
			
			if( ++ event_buf_idx == AIPC_EVENT_BUF_NUM )
				event_buf_idx = 0;
			
			continue;
		}
		
		aipc_event_buf[event_buf_idx].cseq ++;
		aipc_event_buf[event_buf_idx].size = AIPC_EVENT_BUF_LEN;
		
		if( event_type == EVENT_POST ){
			aipc_event_buf[event_buf_idx].type	= EVENT_POST;
			aipc_event_buf[event_buf_idx].own	= OWN_DSP_ALLOC;
		}
		else{	//event_type == EVENT_POST_GET
			aipc_event_buf[event_buf_idx].type	= EVENT_POST_GET;
			aipc_event_buf[event_buf_idx].own	= OWN_DSP_ALLOC;
		}
		
		return &aipc_event_buf[event_buf_idx];
	}
	
	return NULL;
}

int aipc_event_xmit( aipc_event_buf_t *event_buf, event_type_t event_type, u8_t *post_get_buf )
{
	if( event_type>EVENT_MAX || event_type<EVENT_POST || !event_buf )
		return NOK;
	
	if( event_buf ->own != OWN_DSP_ALLOC )
		return NOK;
	
	if( event_type == EVENT_POST ){
		event_buf ->own = OWN_CPU;
	} else {	//event_type == EVENT_POST_GET
		event_buf ->own = OWN_CPU;
		
		while( ( volatile int )( event_buf ->own ) == OWN_CPU );
		
		memcpy( post_get_buf , event_buf ->buf , AIPC_EVENT_BUF_LEN );
		event_buf ->type	= EVENT_POST;
		event_buf ->own		= OWN_NONE;
	}
	
	return OK;
}

int aipc_event_send( event_type_t event_type , u8_t *event_buf )
{
	aipc_event_buf_t *aipc_event_buf;
	
	aipc_event_buf = aipc_event_alloc( event_type );
	
	if( !aipc_event_buf )
		return NOK;
	
	memcpy( aipc_event_buf ->buf, event_buf, AIPC_EVENT_BUF_LEN );
	
	return aipc_event_xmit( aipc_event_buf, event_type, event_buf );
}

static int ctrl_buf_idx = 0;
void ( *aipc_todsp_ctrl_rx )( void *shm_pkt, unsigned long size);

int aipc_ctrl_proc( void )
{
	int i;

	//while( aipc_ctrl_need_proc() )
	for( i = 0; i < AIPC_CTRL_BUF_NUM; i ++ )
	{
		if( aipc_ctrl_buf[ ctrl_buf_idx ].own != OWN_DSP )
			break;
		
		//idx = aipc_ctrl_least_cseq_buf();	//find least cseq buffer and process it
		//aipc_L2_pkt_proc( &aipc_ctrl_buf[idx].buf );

		if(aipc_ctrl_buf[ ctrl_buf_idx ].type == TYPE_POST )
		{
			if( aipc_todsp_ctrl_rx ) {
				aipc_todsp_ctrl_rx( &aipc_ctrl_buf[ ctrl_buf_idx ].buf, 
										aipc_ctrl_buf[ ctrl_buf_idx ].size );
			}

			aipc_ctrl_buf[ ctrl_buf_idx ].own = OWN_NONE;
		}
		else {  //aipc_ctrl_buf[idx].type == TYPE_POST_GET
			//get_responce_packet(&aipc_ctrl_buf[idx].buf, &dsp_id);
			if( aipc_todsp_ctrl_rx ) {
				aipc_todsp_ctrl_rx( &aipc_ctrl_buf[ ctrl_buf_idx ].buf, 
										aipc_ctrl_buf[ ctrl_buf_idx ].size );
			}

			aipc_ctrl_buf[ ctrl_buf_idx ].own = OWN_DSP;
		}
		
		//
		if( ++ ctrl_buf_idx == AIPC_CTRL_BUF_NUM )
			ctrl_buf_idx = 0;
	}
	
	return OK;
} 
