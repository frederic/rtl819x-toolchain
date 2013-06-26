#include <linux/timer.h>
#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_init.h"
#include "voip_ipc.h"

#include "aipc_shm.h"
#ifdef CONFIG_RTK_VOIP_COPROCESS_DSP_IS_HOST
#include "aipc_ctrl_cpu.h"
#include "aipc_data_cpu.h"
#endif
#ifdef CONFIG_RTK_VOIP_COPROCESS_DSP_IS_DSP
#include "aipc_ctrl_dsp.h"
#include "aipc_data_dsp.h"
#endif

#include "ipc_arch_rx.h"

static struct timer_list coprocessor_rx_timer;

static void coprocessor_dsp_rx( void *shm_pkt, unsigned long size)
{
	ipc_pkt_rx_entry( ( ipc_ctrl_pkt_t * )shm_pkt, size );
}

#if 0
aipc_data_shm_t aipc_data_shm;
aipc_ctrl_buf_t aipc_ctrl_buf[AIPC_CTRL_BUF_NUM];
aipc_event_buf_t aipc_event_buf[AIPC_EVENT_BUF_NUM];
int apic_shm_end;
#endif

static void coprocessor_share_memory_init( void )
{
	extern int apic_shm_end;
	int error = 0;
	
#ifdef CONFIG_RTK_VOIP_COPROCESS_DSP_IS_HOST
	// initialize share memory in host side 
	extern int apic_init_shm( void );
	
	apic_init_shm();
#endif

	// check share memory region
	if( ( unsigned long )( void * )&aipc_event_buf +
		sizeof( aipc_event_buf ) > 
		( unsigned long )( void * )&aipc_ctrl_buf )
	{
		error |= 0x01;
	}
	
	if( ( unsigned long )( void * )&aipc_ctrl_buf +
		sizeof( aipc_ctrl_buf ) > 
		( unsigned long )( void * )&aipc_data_shm )
	{
		error |= 0x02;
	}

	if( ( unsigned long )( void * )&aipc_data_shm +
		sizeof( aipc_data_shm ) > 
		( unsigned long )( void * )&apic_shm_end )
	{
		error |= 0x04;
	}
	
	printk( "aipc_event_buf = %p\n", &aipc_event_buf );
	printk( "aipc_ctrl_buf  = %p (%c)\n", &aipc_ctrl_buf, ( error & 0x01 ? 'x' : 'o' ) );
	printk( "aipc_data_shm  = %p (%c)\n", &aipc_data_shm, ( error & 0x02 ? 'x' : 'o' ) );
	printk( "apic_shm_end   = %p (%c)\n", &apic_shm_end, ( error & 0x04 ? 'x' : 'o' ) );

#if 1
	if( error )
		while( 1 );
#endif
} 

static void coprocessor_rx_timer_timer( unsigned long data )
{
	// do polling action 
	
#ifdef CONFIG_RTK_VOIP_COPROCESS_DSP_IS_HOST
	// process mbox packet from dsp to cpu 
	aipc_tocpu_mbox_recv();
	
	// process event packet from dsp to cpu 
	aipc_event_proc();
#elif defined( CONFIG_RTK_VOIP_COPROCESS_DSP_IS_DSP )
	// process mbox packet from cpu to dsp 
	aipc_todsp_mbox_recv();
		
	// process ctrl packet from cpu to dsp 
	aipc_ctrl_proc();
#endif	
	
	// add to timer again 
	coprocessor_rx_timer.expires = jiffies + 1;
	add_timer( &coprocessor_rx_timer );
}

static void __init coprocessor_rx_timer_init( void )
{
	init_timer( &coprocessor_rx_timer );
	coprocessor_rx_timer.expires = jiffies + 1;
	coprocessor_rx_timer.data = 0;
	coprocessor_rx_timer.function = coprocessor_rx_timer_timer;
	add_timer( &coprocessor_rx_timer );
}
	
static int __init coprocessor_dsp_rx_init( void )
{
#ifdef CONFIG_RTK_VOIP_COPROCESS_DSP_IS_HOST
	extern void ( *aipc_tocpu_mbox_rx )( void *shm_pkt, unsigned long size);
	extern void ( *aipc_tocpu_event_rx )( void *shm_pkt, unsigned long size);
	
	aipc_tocpu_mbox_rx = aipc_tocpu_event_rx = coprocessor_dsp_rx;
#elif defined( CONFIG_RTK_VOIP_COPROCESS_DSP_IS_DSP )
	extern void ( *aipc_todsp_mbox_rx )( void *shm_pkt, unsigned long size);
	extern void ( *aipc_todsp_ctrl_rx )( void *shm_pkt, unsigned long size);
	
	aipc_todsp_mbox_rx = aipc_todsp_ctrl_rx = coprocessor_dsp_rx;
#else
	???
#endif
	
	coprocessor_share_memory_init();
	
	coprocessor_rx_timer_init();
	
	return 0;
}

voip_initcall( coprocessor_dsp_rx_init );

