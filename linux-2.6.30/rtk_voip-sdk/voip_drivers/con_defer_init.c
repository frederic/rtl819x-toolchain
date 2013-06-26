#include <linux/kernel.h>
#include "voip_debug.h"
#include "con_defer_init.h"

static defer_init_t *defer_list_head = NULL;
static defer_init_t *defer_list_tail = NULL;

void complete_defer_initialization( void )
{
	defer_init_t *defer = defer_list_head;

#if 0	// test code: enable PCM clk 
	*( ( uint32 volatile * )0xb8008000 ) = 0x00001800;
	asm volatile ("nop\n\t");
	*( ( uint32 volatile * )0xb8008000 ) = 0;
	asm volatile ("nop\n\t");
	*( ( uint32 volatile * )0xb8008000 ) = 0x00001800;
	
	PRINT_Y( "test code: Enable PCM interface. PCMCR(%X) = 0x%X\n", 0xb8008000, *( ( uint32 volatile * )0xb8008000 ) );
#endif
	
	PRINT_MSG( "======= VoIP defer initialization ===\n" );
	
	while( defer ) {
		
		//printk( "defer %p\n", defer ->fn_defer_func );
		
		defer ->fn_defer_func( defer ->p0, defer ->p1 );
		
		defer = defer ->next;
	}
	
	PRINT_MSG( "---------------------------------------\n" );
}

void add_defer_initialization( defer_init_t *defer )
{
	defer ->next = NULL;
	
	if( defer_list_head == NULL ) {
		// first one
		defer_list_head = defer;
	} else {
		defer_list_tail ->next = defer;
	}
	
	defer_list_tail = defer;
}

