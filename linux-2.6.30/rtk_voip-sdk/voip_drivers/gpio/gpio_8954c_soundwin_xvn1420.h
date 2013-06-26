/*
* Copyright c                  Realtek Semiconductor Corporation, 2009
* All rights reserved.
* 
* Program : 8954C SOUNDWIN_XVN1420 GPIO Header File
* Abstract :
* Author :
* 
*/

#ifndef __GPIO_8954C_SOUNDWIN_XVN1420_H__
#define __GPIO_8954C_SOUNDWIN_XVN1420_H__

////// For 8954C SOUNDWIN XVN1420 ATA SLIC /////
#if defined(CONFIG_RTK_VOIP_DRIVERS_ATA_SLIC)

#define GPIO "ATA"
#define PIN_RESET1		GPIO_ID(GPIO_PORT_D,6)	//output
//#define PIN_RESET2		GPIO_ID(GPIO_PORT_D,6)	//output
#define PIN_CLK			GPIO_ID(GPIO_PORT_D,3)	//output
#define PIN_DO			GPIO_ID(GPIO_PORT_D,4) 	//input
#define PIN_DI 			GPIO_ID(GPIO_PORT_D,5) 	//output

#define PIN_CS1			GPIO_ID(GPIO_PORT_D,7)		//output 
#define PIN_CS2			GPIO_ID(GPIO_PORT_E,0) 	//undefined

/* LED */ 
#define PIN_VOIP0_LED	GPIO_ID(GPIO_PORT_F,2)  //output F2
#define PIN_VOIP1_LED	GPIO_ID(GPIO_PORT_F,1)	//output F1

#endif //CONFIG_RTK_VOIP_DRIVERS_ATA_SLIC

#endif /* __GPIO_8954C_SOUNDWIN_XVN1420_H__  */

