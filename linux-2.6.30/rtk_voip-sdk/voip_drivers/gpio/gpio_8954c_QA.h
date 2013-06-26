/*
* Copyright c                  Realtek Semiconductor Corporation, 2009
* All rights reserved.
* 
* Program : 8954C QA board GPIO Header File
* Abstract :
* Author :
* 
*/
 
#ifndef __GPIO_8954C_QA_H__
#define __GPIO_8954C_QA_H__

////// For 8964C QA Board /////

#define GPIO "SLIC"
/* Slic used */
#define PIN_RESET1	GPIO_ID(GPIO_PORT_B,0)  //output
#define PIN_RESET2	GPIO_ID(GPIO_PORT_B,0)  //output
#define PIN_CS1		GPIO_ID(GPIO_PORT_C,3)	//output
#define PIN_CLK		GPIO_ID(GPIO_PORT_C,2)	//output
#define PIN_DI		GPIO_ID(GPIO_PORT_C,1) 	//input

#define PIN_CS2			GPIO_ID(GPIO_PORT_UNDEF,0)	//undefined
#define PIN_CS3			GPIO_ID(GPIO_PORT_UNDEF,0)	//undefined
#define PIN_CS4			GPIO_ID(GPIO_PORT_UNDEF,0)	//undefined

#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215) || \
	defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226) || \
	defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226x) || \
	defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3217x)
	#define PIN_DO		GPIO_ID(GPIO_PORT_B,1)	//output  for Si3215 daugher board
#elif defined CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK
	#define PIN_DO		GPIO_ID(GPIO_PORT_C,0)	//output  for Le88221 daughter board
#endif

/* DAA used (Reserved)*/ 
#define PIN_RESET3_DAA	GPIO_ID(GPIO_PORT_D,0)  //output
#define PIN_CS3_DAA		GPIO_ID(GPIO_PORT_D,1)	//output
#define PIN_CLK_DAA		GPIO_ID(GPIO_PORT_D,2)	//output
#define PIN_DI_DAA		GPIO_ID(GPIO_PORT_D,3) 	//input
#define PIN_DO_DAA		GPIO_ID(GPIO_PORT_D,4)	//output

/* Unused */
#define PIN_CS2			0x0
#define PIN_CS3			0x0
#define PIN_CS4			0x0

#endif /* __GPIO_8954C_QA_H__ */

