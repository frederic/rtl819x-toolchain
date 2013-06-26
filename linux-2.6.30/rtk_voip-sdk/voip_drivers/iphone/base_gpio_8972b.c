#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/string.h>

#include "../gpio/gpio.h"
#include "base_gpio_8972b.h"

#ifdef CONFIG_RTK_VOIP_GPIO_IPP_8972B_V00
#define IPPHONE_PIN_HOOK	GPIO_ID(GPIO_PORT_E,6)	//input
#define INTERRUPT_HOOK
#else
#define IPPHONE_PIN_HOOK	GPIO_ID(GPIO_PORT_B,4)	//input
//#define INTERRUPT_HOOK
#endif

/************************* IP phone hook detection ****************************************/
void ipphone_hook_interrupt( int enable )
{
#if defined( IPPHONE_PIN_HOOK ) && defined( INTERRUPT_HOOK )
	const enum GPIO_INTERRUPT_TYPE int_type = ( enable ? GPIO_INT_BOTH_EDGE : GPIO_INT_DISABLE );
	
	_rtl8972B_initGpioPin( IPPHONE_PIN_HOOK, GPIO_CONT_GPIO, GPIO_DIR_IN, int_type );
#endif
}

void init_ipphone_hook_detect( void )
{
#ifdef IPPHONE_PIN_HOOK
  #ifdef INTERRUPT_HOOK
	ipphone_hook_interrupt( 1 );
  #else
	_rtl8972B_initGpioPin( IPPHONE_PIN_HOOK, GPIO_CONT_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE );
  #endif
#endif
}

//return value 0->on-hook, 1->off-hook
unsigned char iphone_hook_detect( void )
{
	uint32 data;

#ifdef CONFIG_RTK_VOIP_GPIO_IPP_8972B_V00
  #ifdef IPPHONE_PIN_HOOK
	_rtl8972B_getGpioDataBit( IPPHONE_PIN_HOOK, &data );
  #else
	extern unsigned char ALC5621_GetGpioStatus( void );
	
	data = ( ALC5621_GetGpioStatus() ? 1 : 0 );
  #endif
#else
	_rtl8972B_getGpioDataBit( IPPHONE_PIN_HOOK, &data );
#endif
	
	//printk( "HOOK:%d\n", data );

#if 0	// debug use only 
	return ( unsigned char )data;
#else	
	return ( unsigned char )( !data );
#endif
}


