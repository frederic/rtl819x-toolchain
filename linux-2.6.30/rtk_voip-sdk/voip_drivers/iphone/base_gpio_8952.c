#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/string.h>

#include "../gpio/gpio.h"
#include "base_gpio_8952.h"

#define IPPHONE_PIN_HOOK	GPIO_ID(GPIO_PORT_B,4)	//output

/************************* IP phone hook detection ****************************************/
void init_ipphone_hook_detect( void )
{
	_rtl865xC_initGpioPin( IPPHONE_PIN_HOOK, GPIO_PERI_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE );
}

//return value 0->on-hook, 1->off-hook
unsigned char iphone_hook_detect( void )
{
	uint32 data;
	
	_rtl865xC_getGpioDataBit( IPPHONE_PIN_HOOK, &data );

#if 0	// debug use only 
	return ( unsigned char )data;
#else	
	return ( unsigned char )( !data );
#endif
}


