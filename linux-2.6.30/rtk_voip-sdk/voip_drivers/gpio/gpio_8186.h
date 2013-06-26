/*
* Copyright c                  Realtek Semiconductor Corporation, 2006  
* All rights reserved.
* 
* Program : GPIO Header File 
* Abstract : 
* Author :                
* 
*/

 
#ifndef __GPIO_8186_H
#define __GPIO_8186_H

/*==================== FOR RTL8186 EVB gpio pin==================*/

//It's for Z-version board used. GPIO_C used
#define CONFIG_RTK_VOIP_DRIVERS_PCM8186V_Z 0	//0: R-version board, 1: Z-version board
#if CONFIG_RTK_VOIP_DRIVERS_PCM8186V_Z
#define RTL8186PV_RELAY_USE_Z 1	//0: R-version board, 1: Z-version board
#else
#define RTL8186PV_RELAY_USE_Z 0	//0: R-version board, 1: Z-version board
#endif

////// For 8186 PV V275 EV Board /////
#ifdef CONFIG_RTK_VOIP_GPIO_8186PV_V275
#define GPIO "C"
#if !CONFIG_RTK_VOIP_DRIVERS_PCM8186V_Z //R-version board USED
/*slic used*/
#define PIN_RESET1	GPIO_ID(GPIO_PORT_C,0)
#define PIN_CS1		GPIO_ID(GPIO_PORT_C,2)	//output
#define PIN_CLK		GPIO_ID(GPIO_PORT_C,3)	//output
#define PIN_DI		GPIO_ID(GPIO_PORT_C,4) 	//input
#define PIN_DO		GPIO_ID(GPIO_PORT_C,6)	//output
/* DAA used. chiminer*/
#define PIN_RESET3_DAA		GPIO_ID(GPIO_PORT_C,10)  //output
#define PIN_CS3_DAA		GPIO_ID(GPIO_PORT_C,11)	//output
#define PIN_CLK_DAA		GPIO_ID(GPIO_PORT_C,12)	//output
#define PIN_DI_DAA		GPIO_ID(GPIO_PORT_C,13) 	//input
#define PIN_DO_DAA		GPIO_ID(GPIO_PORT_C,14)	//output

#else //Z-version board USED
/*slic used*/
#define PIN_RESET1	GPIO_ID(GPIO_PORT_C,1)
#define PIN_CLK		GPIO_ID(GPIO_PORT_C,2)	//output
#define PIN_CS1		GPIO_ID(GPIO_PORT_C,3)	//output
#define PIN_DI		GPIO_ID(GPIO_PORT_C,5) 	//input
#define PIN_DO		GPIO_ID(GPIO_PORT_C,4)	//output
/* DAA used. chiminer*/
#define PIN_RESET3_DAA		GPIO_ID(GPIO_PORT_C,6)  //output
#define PIN_CLK_DAA		GPIO_ID(GPIO_PORT_C,7)	//output
#define PIN_CS3_DAA		GPIO_ID(GPIO_PORT_C,8)	//output
#define PIN_DI_DAA		GPIO_ID(GPIO_PORT_C,10) 	//input
#define PIN_DO_DAA		GPIO_ID(GPIO_PORT_C,9)	//output
#endif //end CONFIG_RTK_VOIP_DRIVERS_PCM8186V_Z
#endif

////// For 8186V/VA V100, V200, V210, V220, V221 EV Board /////
//#define CONFIG_RTK_VOIP_DRIVERS_PCM8186V_OC

#ifdef CONFIG_RTK_VOIP_GPIO_8186V_V100_V200_V210_C220

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186V_OC	//OC's board

#define GPIO "AD"
/*Slic used*/
#define PIN_RESET1	GPIO_ID(GPIO_PORT_D,8)
#define PIN_CS1		GPIO_ID(GPIO_PORT_D,6)	//output
#define PIN_CLK		GPIO_ID(GPIO_PORT_D,7)	//output
#define PIN_DI		GPIO_ID(GPIO_PORT_D,5)  //input
#define PIN_DO		GPIO_ID(GPIO_PORT_D,0)	//output

//DAA common GPIO pin
#define PIN_RESET3_DAA		GPIO_ID(GPIO_PORT_D,9)  //output
#define PIN_CLK_DAA		GPIO_ID(GPIO_PORT_A,3)	//output
#define PIN_DO_DAA		GPIO_ID(GPIO_PORT_A,2)	//output

//DAA 1 GPIO pin
#define PIN_CS3_DAA		GPIO_ID(GPIO_PORT_D,13)	//output
#define PIN_DI_DAA		GPIO_ID(GPIO_PORT_A,7) 	//input

//DAA 2 GPIO pin
#define PIN_CS3_DAA_2		GPIO_ID(GPIO_PORT_D,14)	//output
#define PIN_DI_DAA_2		GPIO_ID(GPIO_PORT_A,8) 	//input

#else	//RTK-version board

#define GPIO "D"
/*Slic used*/
#define PIN_RESET1	GPIO_ID(GPIO_PORT_D,7)
#define PIN_CS1		GPIO_ID(GPIO_PORT_D,10)	//output
#define PIN_CLK		GPIO_ID(GPIO_PORT_D,8)	//output
#define PIN_DI		GPIO_ID(GPIO_PORT_D,5) //input
#define PIN_DO		GPIO_ID(GPIO_PORT_D,0)	//output
/* DAA used */
#define PIN_RESET3_DAA		GPIO_ID(GPIO_PORT_D,13)  //output
#define PIN_CS3_DAA		GPIO_ID(GPIO_PORT_D,11)	//output
#define PIN_CLK_DAA		GPIO_ID(GPIO_PORT_D,12)	//output
#define PIN_DI_DAA		GPIO_ID(GPIO_PORT_D,9) 	//input
#define PIN_DO_DAA		GPIO_ID(GPIO_PORT_D,14)	//output

#endif

#endif

////// For 8186V IP Phone 100 /////
#ifdef CONFIG_RTK_VOIP_GPIO_IPP_100
#define GPIO "D"
/*slic used*/
#define PIN_RESET1	GPIO_ID(GPIO_PORT_C,0)
#define PIN_CS1		GPIO_ID(GPIO_PORT_C,2)	//output
#define PIN_CLK		GPIO_ID(GPIO_PORT_C,3)	//output
#define PIN_DI		GPIO_ID(GPIO_PORT_C,4) 	//input
#define PIN_DO		GPIO_ID(GPIO_PORT_C,6)	//output
/* DAA used. chiminer*/
#define PIN_RESET3_DAA		GPIO_ID(GPIO_PORT_D,13)  //output
#define PIN_CS3_DAA		GPIO_ID(GPIO_PORT_D,11)	//output
#define PIN_CLK_DAA		GPIO_ID(GPIO_PORT_D,12)	//output
#define PIN_DI_DAA		GPIO_ID(GPIO_PORT_D,14) 	//input
#define PIN_DO_DAA		GPIO_ID(GPIO_PORT_D,9)	//output
#endif


////// For 8186V IP Phone 101 /////
#ifdef CONFIG_RTK_VOIP_GPIO_IPP_101
#define GPIO "C"
/*slic used*/
#define PIN_RESET1	GPIO_ID(GPIO_PORT_C,0)
#define PIN_CS1		GPIO_ID(GPIO_PORT_C,2)	//output
#define PIN_CLK		GPIO_ID(GPIO_PORT_C,3)	//output
#define PIN_DI		GPIO_ID(GPIO_PORT_C,4) 	//input
#define PIN_DO		GPIO_ID(GPIO_PORT_C,6)	//output
/* DAA used. chiminer*/
#define PIN_RESET3_DAA		GPIO_ID(GPIO_PORT_A,1)  //output
#define PIN_CS3_DAA		GPIO_ID(GPIO_PORT_E,2)	//output
#define PIN_CLK_DAA		GPIO_ID(GPIO_PORT_E,1)	//output
#define PIN_DI_DAA		GPIO_ID(GPIO_PORT_E,3) 	//input
#define PIN_DO_DAA		GPIO_ID(GPIO_PORT_A,0)	//output
#endif

/*====================END FOR RTL8186 EVB gpio pin ==================*/


/*==================== FOR RTL8186 ==================*/

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186

/******** GPIO define ********/

/* define GPIO port */
enum GPIO_PORT
{
	GPIO_PORT_A = 0,
	GPIO_PORT_B,
	GPIO_PORT_C,
	GPIO_PORT_D,
	GPIO_PORT_E,
	GPIO_PORT_F,
	GPIO_PORT_G,
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

/*************** Define RTL8186 GPIO Register Set ************************/

#define	GPABDATA	0xBD010120
#define	GPABDIR		0xBD010124
#define	GPABIMR		0xBD010128
#define	GPABISR		0xBD01012C
#define	GPCDDATA	0xBD010130
#define	GPCDDIR		0xBD010134
#define	GPCDIMR		0xBD010138
#define	GPCDISR		0xBD01013C
#define	GPEFDATA	0xBD010140
#define	GPEFDIR		0xBD010144
#define	GPEFIMR		0xBD010148
#define	GPEFISR		0xBD01014C
#define	GPGDATA		0xBD010150
#define	GPGDIR		0xBD010154
#define	GPGIMR		0xBD010158
#define	GPGISR		0xBD01015C

/* Register access macro (REG*()).*/
#define REG32(reg) 			(*((volatile uint32 *)(reg)))
#define REG16(reg) 			(*((volatile uint16 *)(reg)))
#define REG8(reg) 			(*((volatile uint8 *)(reg)))



/*********************  Function Prototype in gpio.c  ***********************/
int32 _rtl8186_initGpioPin( uint32 gpioId, enum GPIO_DIRECTION direction, enum GPIO_INTERRUPT_TYPE interruptEnable );
int32 _rtl8186_getGpioDataBit( uint32 gpioId, uint32* pData );
int32 _rtl8186_setGpioDataBit( uint32 gpioId, uint32 data );

#endif //CONFIG_RTK_VOIP_DRIVERS_PCM8186

#endif /* __GPIO_8186_H */
