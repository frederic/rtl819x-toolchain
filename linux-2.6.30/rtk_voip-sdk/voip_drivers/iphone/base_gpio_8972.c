#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/string.h>

#include "base_gpio_8972.h"

//return value 0->on-hook, 1->off-hook
unsigned char iphone_hook_detect( void )
{
#if 1
	extern unsigned char ALC5621_GetGpioStatus( void );
	
	return ( ALC5621_GetGpioStatus() ? 0 : 1 );
#else
	extern extern int pt6961_GetHookStatus( void );

	return pt6961_GetHookStatus();
#endif
}

