/*
* Copyright c                  Realtek Semiconductor Corporation, 2009
* All rights reserved.
* 
* Program : 8954C V400 GPIO Header File
* Abstract :
* Author :
* 
*/
 
#ifndef __GPIO_8954C_V400_H__
#define __GPIO_8954C_V400_H__

////// For 8954C V400 ATA SLIC /////
#if defined(CONFIG_RTK_VOIP_DRIVERS_ATA_SLIC)

#define GPIO "ATA"
#define PIN_RESET1		GPIO_ID(GPIO_PORT_D,6)	//output
#define PIN_RESET2		GPIO_ID(GPIO_PORT_D,6)	//output

#define PIN_CS1			GPIO_ID(GPIO_PORT_D,7)	//output 
#define PIN_CS2			GPIO_ID(GPIO_PORT_E,6)	//output share with PIN_VOIP2_LED
#define PIN_CS3			GPIO_ID(GPIO_PORT_E,5)	//output share with PIN_VOIP1_LED
#define PIN_CS4			GPIO_ID(GPIO_PORT_E,4)	//output share with PIN_VOIP0_LED

#define PIN_CLK			GPIO_ID(GPIO_PORT_D,3)	//output
#define PIN_DO			GPIO_ID(GPIO_PORT_D,4) 	//input
#define PIN_DI 			GPIO_ID(GPIO_PORT_D,5) 	//output

#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_SILAB
	/* SILAB SLIC */
	#if defined( CONFIG_RTK_VOIP_MULTIPLE_SI32178 ) || \
		defined( CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_CS )
	//vincent TODO check setting
	#define PIN_CS2			GPIO_ID(GPIO_PORT_E,0)	//output (Si32178)
	#endif
#elif defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK) 
	/* Zarlink SLIC */
	#undef  PIN_RESET1
	#define PIN_RESET1 	GPIO_ID(GPIO_PORT_UNDEF,0) 	//undefined
#endif //CONFIG_RTK_VOIP_DRIVERS_SLIC_SILAB

/* LED */ 
/* V400 use below GPIO as SLIC CS */
#define PIN_VOIP0_LED	GPIO_ID(GPIO_PORT_E,4)    	//share with PIN_CS4
#define PIN_VOIP1_LED	GPIO_ID(GPIO_PORT_E,5)    	//share with PIN_CS3
#define PIN_VOIP2_LED	GPIO_ID(GPIO_PORT_E,6)    	//share with PIN_CS2

#endif //CONFIG_RTK_VOIP_DRIVERS_ATA_SLIC

#ifdef CONFIG_RTK_VOIP_DRIVERS_ATA_DECT
	#define PIN_DECT_RESET1		GPIO_ID(GPIO_PORT_D,0)	//output
	#define PIN_DECT_PAGE		GPIO_ID(GPIO_PORT_E,3) 	//intput
#if defined( CONFIG_RTK_VOIP_DECT_SPI_SUPPORT )
	/* DECT SPI */
	#define GPIO "DECT"
	#define PIN_DECT_CS1		GPIO_ID(GPIO_PORT_E,3)	//output
	#define PIN_DECT_CLK		GPIO_ID(GPIO_PORT_E,1)	//output
	#define PIN_DECT_DI			GPIO_ID(GPIO_PORT_E,5) 	//input
	#define PIN_DECT_DO 		GPIO_ID(GPIO_PORT_E,4) 	//output
	#define PIN_DECT_INT		GPIO_ID(GPIO_PORT_E,6) 	//intput
#endif
#endif //CONFIG_RTK_VOIP_DRIVERS_ATA_DECT

#endif /* __GPIO_8954C_V400_H__ */

