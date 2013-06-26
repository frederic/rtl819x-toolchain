/*
* Copyright c                  Realtek Semiconductor Corporation, 2006  
* All rights reserved.
* 
* Program : GPIO Header File 
* Abstract : 
* Author :                
* 
*/

 
#ifndef __GPIO_8972_H
#define __GPIO_8972_H

/*==================== FOR RTL867x EVB gpio pin ==================*/

////// For 8671V EV Board /////
#ifdef CONFIG_RTK_VOIP_GPIO_8671_QA_V1_1_V1_1_2_2
#define GPIO "AB"
/*slic used*/
#define PIN_RESET1	GPIO_ID(GPIO_PORT_B,3)  //output
#define PIN_CS1		GPIO_ID(GPIO_PORT_B,4)	//output
#define PIN_CLK		GPIO_ID(GPIO_PORT_B,5)	//output
#define PIN_DI		GPIO_ID(GPIO_PORT_B,6) 	//input
#define PIN_DO		GPIO_ID(GPIO_PORT_B,7)	//output
/* DAA used*/
#define PIN_RESET3_DAA		GPIO_ID(GPIO_PORT_A,3)  //output
#define PIN_CS3_DAA		GPIO_ID(GPIO_PORT_A,5)	//output
#define PIN_CLK_DAA		GPIO_ID(GPIO_PORT_A,4)	//output
#define PIN_DI_DAA		GPIO_ID(GPIO_PORT_B,7) 	//input
#define PIN_DO_DAA		GPIO_ID(GPIO_PORT_B,6)	//output
#endif

#ifdef CONFIG_RTK_VOIP_GPIO_8671_V1_2_EMI
#define GPIO "AB"
/*slic used*/
#define PIN_RESET1	GPIO_ID(GPIO_PORT_A,3)  //output
#define PIN_CS1		GPIO_ID(GPIO_PORT_A,4)	//output
#define PIN_CLK		GPIO_ID(GPIO_PORT_B,5)	//output
#define PIN_DI		GPIO_ID(GPIO_PORT_B,6) 	//input
#define PIN_DO		GPIO_ID(GPIO_PORT_B,7)	//output
/* DAA used*/
#define PIN_RESET3_DAA		GPIO_ID(GPIO_PORT_A,3)  //output
#define PIN_CS3_DAA		GPIO_ID(GPIO_PORT_A,5)	//output
#define PIN_CLK_DAA		GPIO_ID(GPIO_PORT_A,4)	//output
#define PIN_DI_DAA		GPIO_ID(GPIO_PORT_B,7) 	//input
#define PIN_DO_DAA		GPIO_ID(GPIO_PORT_B,6)	//output
#endif

#ifdef CONFIG_RTK_VOIP_GPIO_8671_V1_2
#define GPIO "AB"
/*slic used*/
#define PIN_RESET1	GPIO_ID(GPIO_PORT_A,3)  //output
#define PIN_CS1		GPIO_ID(GPIO_PORT_B,5)	//output
#define PIN_CLK		GPIO_ID(GPIO_PORT_A,4)	//output
#define PIN_DI		GPIO_ID(GPIO_PORT_B,7) 	//input
#define PIN_DO		GPIO_ID(GPIO_PORT_B,6)	//output
/* DAA used*/
#define PIN_RESET3_DAA		GPIO_ID(GPIO_PORT_A,3)  //output
#define PIN_CS3_DAA		GPIO_ID(GPIO_PORT_A,5)	//output
#define PIN_CLK_DAA		GPIO_ID(GPIO_PORT_A,4)	//output
#define PIN_DI_DAA		GPIO_ID(GPIO_PORT_B,7) 	//input
#define PIN_DO_DAA		GPIO_ID(GPIO_PORT_B,6)	//output
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
/* To be modified!! */
#define GPIO "AB"
/*slic used*/
#define PIN_RESET1	GPIO_ID(GPIO_PORT_A,3)  //output
#define PIN_CS1		GPIO_ID(GPIO_PORT_B,5)	//output
#define PIN_CLK		GPIO_ID(GPIO_PORT_A,4)	//output
#define PIN_DI		GPIO_ID(GPIO_PORT_B,7) 	//input
#define PIN_DO		GPIO_ID(GPIO_PORT_B,6)	//output
/* DAA used*/
#define PIN_RESET3_DAA		GPIO_ID(GPIO_PORT_A,3)  //output
#define PIN_CS3_DAA		GPIO_ID(GPIO_PORT_A,5)	//output
#define PIN_CLK_DAA		GPIO_ID(GPIO_PORT_A,4)	//output
#define PIN_DI_DAA		GPIO_ID(GPIO_PORT_B,7) 	//input
#define PIN_DO_DAA		GPIO_ID(GPIO_PORT_B,6)	//output
#endif

/*====================END FOR RTL867x EVB gpio pin ==================*/


/*==================== FOR RTL867x ==================*/
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8671

/******** GPIO define ********/


/* define GPIO port */
enum GPIO_PORT
{
	GPIO_PORT_A = 0,
	GPIO_PORT_B,
	GPIO_PORT_MAX,
};


/* define GPIO direction */
enum GPIO_DIRECTION
{
	GPIO_DIR_IN = 0,
	GPIO_DIR_OUT =1,
};

/* define GPIO Interrupt Type */
enum GPIO_INTERRUPT_TYPE
{
	GPIO_INT_DISABLE = 0,
	GPIO_INT_ENABLE,

};

/*************** Define RTL867x GPIO Register Set ************************/

#define	GPADIR		0xB9C01000
#define	GPADATA		0xB9C01004
#define	GPAISR		0xB9C01008
#define	GPAIMR		0xB9C0100C
#define	GPBDIR		0xB9C01010
#define	GPBDATA		0xB9C01014
#define	GPBISR		0xB9C01018
#define	GPBIMR		0xB9C0101C
/**************************************************************************/
/* Register access macro (REG*()).*/
#define REG32(reg) 			(*((volatile uint32 *)(reg)))
#define REG16(reg) 			(*((volatile uint16 *)(reg)))
#define REG8(reg) 			(*((volatile uint8 *)(reg)))

/*********************  Function Prototype in gpio.c  ***********************/
int32 _rtl867x_initGpioPin( uint32 gpioId, enum GPIO_DIRECTION direction, enum GPIO_INTERRUPT_TYPE interruptEnable );
int32 _rtl867x_getGpioDataBit( uint32 gpioId, uint32* pData );
int32 _rtl867x_setGpioDataBit( uint32 gpioId, uint32 data );

#endif//CONFIG_RTK_VOIP_DRIVERS_PCM8671

#endif/*__GPIO__*/
