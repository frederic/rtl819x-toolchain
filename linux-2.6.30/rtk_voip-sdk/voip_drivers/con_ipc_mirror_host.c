#include <linux/kernel.h>
#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_init.h"
#include "voip_proc.h"
#include "voip_ipc.h"
#include "con_register.h"
#include "con_ipc_mirror_host.h"

void ipc_mirror_request( voip_con_t *this, uint16 category, 
							void *data, uint16 len )
{
	extern int ipcSentMirrorPacket(unsigned short cmd, unsigned int host_cch, void* mirror_data, unsigned short mirror_len);
	
#if 0	// print debug message 
	int i;
	
	printk( "cch=%d cate=%d data=", this ->cch, category );
	
	for( i = 0; i < len; i ++ )
		printk( "%02X ", *( ( uint8 * )data + i ) );
	
	printk( "\n" );
#endif
	
	ipcSentMirrorPacket( category, this ->cch, data, len );
}

void ipc_mirror_ack_parser( const ipc_ctrl_pkt_t *ipc_ctrl )
{
	// TODO: do something to check mirror_ack 
	
}

