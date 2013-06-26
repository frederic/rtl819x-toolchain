/*
* Copyright c                  Realtek Semiconductor Corporation, 2009
* All rights reserved.
* 
* Program : 8972D V100 GPIO Header File
* Abstract :
* Author :
* 
*/
 
#ifndef __GPIO_8972D_V100_H__
#define __GPIO_8972D_V100_H__

////// For 8972D V100 ATA SLIC /////
#if defined(CONFIG_RTK_VOIP_DRIVERS_ATA_SLIC)

#define GPIO "ATA"
#define PIN_RESET1		GPIO_ID(GPIO_PORT_G,0)	//output
//#define PIN_RESET2		GPIO_ID(GPIO_PORT_D,6)	//output

#define PIN_CS1			GPIO_ID(GPIO_PORT_B,7)	//output 
#define PIN_CS2			GPIO_ID(GPIO_PORT_C,0)	//output 

#define PIN_CLK			GPIO_ID(GPIO_PORT_C,4)	//output
#define PIN_DI			GPIO_ID(GPIO_PORT_C,5) 	//input
#define PIN_DO 			GPIO_ID(GPIO_PORT_A,1) 	//output

/* LED */ 
/* V100 use below GPIO as SLIC CS */
#define PIN_VOIP0_LED	GPIO_ID(GPIO_PORT_G,5)    
#define PIN_VOIP1_LED	GPIO_ID(GPIO_PORT_G,6)    

#endif //CONFIG_RTK_VOIP_DRIVERS_ATA_SLIC


#endif /* __GPIO_8972D_V100_H__ */

