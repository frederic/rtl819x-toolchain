#include "ui_config.h"
#include "ioctl_kernel.h"
#include "ioctl_led.h"
#include <stdio.h>

void TurnOnOffLEDThruoghGPIO( unsigned long mask, unsigned long set )
{
	static unsigned long pre_set = 0x0000;
	unsigned long cur_set;
	
	cur_set = ( pre_set & ~mask ) | ( set & mask );
	
	if( cur_set && cur_set == pre_set )
		return;
	
#if 1
	rtk_SetLedOnOff( cur_set );
#else
	//rtk_Set_IPhone( 4, ( unsigned short )cur_set, 0 /* ignore */ );
#endif
	
	pre_set = cur_set;
}

