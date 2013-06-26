/*
* Copyright c                  Realtek Semiconductor Corporation, 2009
* All rights reserved.
* 
* Program : 8954C V200 GPIO Header File
* Abstract :
* Author :
* 
*/

#ifndef __GPIO_8954C_V200_H__
#define __GPIO_8954C_V200_H__

////// For 8954C V200 ATA SLIC /////
#if defined(CONFIG_RTK_VOIP_DRIVERS_ATA_SLIC)

#define GPIO "ATA"
#define PIN_RESET1		GPIO_ID(GPIO_PORT_D,6)	//output
#define PIN_RESET2		GPIO_ID(GPIO_PORT_D,6)	//output
#define PIN_CLK			GPIO_ID(GPIO_PORT_D,3)	//output
#define PIN_DO			GPIO_ID(GPIO_PORT_D,4) 	//input
#define PIN_DI 			GPIO_ID(GPIO_PORT_D,5) 	//output

#define PIN_CS1			GPIO_ID(GPIO_PORT_D,7)		//output 
#define PIN_CS2			GPIO_ID(GPIO_PORT_UNDEF,0) 	//undefined
#define PIN_CS3			GPIO_ID(GPIO_PORT_UNDEF,0) 	//undefined
#define PIN_CS4			GPIO_ID(GPIO_PORT_UNDEF,0) 	//undefined

#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_SILAB
	/* Silab */
	#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3217x
	/* SLIC Si3217x */
	#if defined( CONFIG_RTK_VOIP_SLIC_SI32178_NR ) &&	\
		( ( CONFIG_RTK_VOIP_SLIC_SI32178_NR == 8 ) ||	\
		  ( CONFIG_RTK_VOIP_SLIC_SI32178_NR == 4 ) )
	#undef PIN_CS1
	#undef PIN_CS2
	#undef PIN_CS3
	#undef PIN_CS4
	#define PIN_CS1			GPIO_ID(GPIO_PORT_D,7)	//output
	#define PIN_CS2			GPIO_ID(GPIO_PORT_E,0)	//output
	#define PIN_CS3			GPIO_ID(GPIO_PORT_E,1)	//output
	#define PIN_CS4			GPIO_ID(GPIO_PORT_D,0)	//output
	#define PIN_CS5			GPIO_ID(GPIO_PORT_E,3)	//output
	#define PIN_CS6			GPIO_ID(GPIO_PORT_E,4)	//output
	#define PIN_CS7			GPIO_ID(GPIO_PORT_E,5)	//output
	#define PIN_CS8			GPIO_ID(GPIO_PORT_E,6)	//output
	#else
	#ifdef CONFIG_RTK_VOIP_SLIC_SI32176_SI32178
	#else
	#undef PIN_CS1
	#define PIN_CS1			GPIO_ID(GPIO_PORT_D,7)	//output (Si32176)
	#if defined( CONFIG_RTK_VOIP_SLIC_SI32176 ) && defined( CONFIG_RTK_VOIP_SLIC_SI32178 )
	#undef PIN_CS2
	#define PIN_CS2			GPIO_ID(GPIO_PORT_E,0)	//output (Si32178)
	#endif
	#endif // 
	#endif // CONFIG_RTK_VOIP_SLIC_SI32178_NR == 8
	#endif //CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3217x	
#elif defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK) 
	/* Zarlink */
	#undef  PIN_RESET1
	#define PIN_RESET1 	GPIO_ID(GPIO_PORT_UNDEF,0) 	//NOT defined
#endif //CONFIG_RTK_VOIP_DRIVERS_SLIC_SILAB

/* PSTN Relay */
#define PIN_RELAY0		GPIO_ID(GPIO_PORT_D,1) 	//output
#define PIN_RELAY1		GPIO_ID(GPIO_PORT_D,2) 	//output

/* LED */ 
#define PIN_VOIP1_LED	GPIO_ID(GPIO_PORT_E,6)  //output
#define PIN_VOIP2_LED	GPIO_ID(GPIO_PORT_E,5)	//output
#define PIN_PSTN_LED	GPIO_ID(GPIO_PORT_E,4)	//output

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

#endif /* __GPIO_8954C_V200_H__ */

