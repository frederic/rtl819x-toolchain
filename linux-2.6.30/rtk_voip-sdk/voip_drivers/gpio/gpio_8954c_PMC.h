/*
* Copyright c                  Realtek Semiconductor Corporation, 2009
* All rights reserved.
* 
* Program : 8954C V400 GPIO Header File
* Abstract :
* Author :
* 
*/
 
#ifndef __GPIO_8954C_PMC_H__
#define __GPIO_8954C_PMC_H__

////// For 8954C PMC GPON /////

#define PIN_CS1			GPIO_ID(GPIO_PORT_A,4)	//output 
#define PIN_CLK			GPIO_ID(GPIO_PORT_G,6)	//output
#define PIN_DO			GPIO_ID(GPIO_PORT_G,1) 	//output G1
#define PIN_DI 			GPIO_ID(GPIO_PORT_A,0) 	//input


/* LED */ 
#define PIN_VOIP0_LED	GPIO_ID(GPIO_PORT_C,0)    	
#define PIN_VOIP1_LED	GPIO_ID(GPIO_PORT_C,4)    	

#endif /* __GPIO_8954C_PMC_H__ */

