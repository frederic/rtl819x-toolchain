/*
* Copyright c                  Realtek Semiconductor Corporation, 2006  
* All rights reserved.
* 
* Program : GPIO Header File 
* Abstract : 
* Author :                
* 
*/

 
#ifndef __GPIO_8952_H
#define __GPIO_8952_H

////// For 8651C/CV EV Board /////
#ifdef CONFIG_RTK_VOIP_GPIO_8651C

#define GPIO "A"
/* Slic used */
#define PIN_RESET1	GPIO_ID(GPIO_PORT_A,6)  //output
#define PIN_CS1		GPIO_ID(GPIO_PORT_A,0)	//output
#define PIN_CLK		GPIO_ID(GPIO_PORT_A,1)	//output
#define PIN_DI		GPIO_ID(GPIO_PORT_A,3) 	//input
#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226) || defined (CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3217x) || defined (CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226x)
#define PIN_DO		GPIO_ID(GPIO_PORT_A,2)	//output  for Si3215 daugher board
#elif defined (CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK)
#define PIN_DO		GPIO_ID(GPIO_PORT_A,5)	//output  for Le88221 daughter board
#endif

/* DAA used (Reserved)*/ 
#define PIN_RESET3_DAA		GPIO_ID(GPIO_PORT_D,0)  //output
#define PIN_CS3_DAA		GPIO_ID(GPIO_PORT_D,1)	//output
#define PIN_CLK_DAA		GPIO_ID(GPIO_PORT_D,2)	//output
#define PIN_DI_DAA		GPIO_ID(GPIO_PORT_D,3) 	//input
#define PIN_DO_DAA		GPIO_ID(GPIO_PORT_D,4)	//output

#endif

////// For 8962 EV Board /////
#ifdef CONFIG_RTK_VOIP_GPIO_8962

#define GPIO "A"
/* SLIC Si3226 */
#define PIN_RESET1		GPIO_ID(GPIO_PORT_H,7)	//output
#define PIN_INT1		GPIO_ID(GPIO_PORT_E,2)  //input
#define PIN_CS1			GPIO_ID(GPIO_PORT_H,6)	//output
#define PIN_CLK			GPIO_ID(GPIO_PORT_E,1)	//output
#define PIN_DI			GPIO_ID(GPIO_PORT_G,3) 	//input
#define PIN_DO 			GPIO_ID(GPIO_PORT_G,2) 	//output

/* DAA used */ 
#define PIN_RESET3_DAA	GPIO_ID(GPIO_PORT_E,0)  //output
#define PIN_CS3_DAA		GPIO_ID(GPIO_PORT_G,0)	//output
#define PIN_CLK_DAA		GPIO_ID(GPIO_PORT_G,1)	//output
#define PIN_DI_DAA		GPIO_ID(GPIO_PORT_G,7) 	//input
#define PIN_DO_DAA		GPIO_ID(GPIO_PORT_G,6)	//output

/* LED */ 
#define PIN_VOIP1_LED	GPIO_ID(GPIO_PORT_F,4)  //output
#define PIN_VOIP2_LED	GPIO_ID(GPIO_PORT_F,2)	//output
#define PIN_PSTN_LED	GPIO_ID(GPIO_PORT_F,5)	//output

#endif

////// For IP Phone /////
#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
/* To be modified!! */
#define GPIO "A"
/* SLIC Si3226 */
#define PIN_RESET1		GPIO_ID(GPIO_PORT_H,7)	//output
#define PIN_INT1		GPIO_ID(GPIO_PORT_E,2)  //input
#define PIN_CS1			GPIO_ID(GPIO_PORT_H,6)	//output
#define PIN_CLK			GPIO_ID(GPIO_PORT_E,1)	//output
#define PIN_DI			GPIO_ID(GPIO_PORT_G,3) 	//input
#define PIN_DO 			GPIO_ID(GPIO_PORT_G,2) 	//output

/* DAA used */ 
#define PIN_RESET3_DAA	GPIO_ID(GPIO_PORT_E,0)  //output
#define PIN_CS3_DAA		GPIO_ID(GPIO_PORT_G,0)	//output
#define PIN_CLK_DAA		GPIO_ID(GPIO_PORT_G,1)	//output
#define PIN_DI_DAA		GPIO_ID(GPIO_PORT_G,7) 	//input
#define PIN_DO_DAA		GPIO_ID(GPIO_PORT_G,6)	//output

/* LED */ 
#define PIN_VOIP1_LED	GPIO_ID(GPIO_PORT_F,4)  //output
#define PIN_VOIP2_LED	GPIO_ID(GPIO_PORT_F,2)	//output
#define PIN_PSTN_LED	GPIO_ID(GPIO_PORT_F,5)	//output
#endif 

/*==================== FOR RTL865xC ==================*/
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM865xC

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
	GPIO_PORT_H,
	GPIO_PORT_MAX,
};

/* define GPIO dedicate peripheral pin */
enum GPIO_PERIPHERAL
{
	GPIO_PERI_GPIO = 0,
	GPIO_PERI_TYPE0,
	GPIO_PERI_TYPE1,
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
	GPIO_INT_FALLING_EDGE,
	GPIO_INT_RISING_EDGE,
	GPIO_INT_BOTH_EDGE,
};

/*************** Define RTL865xC GPIO Register Set ************************/
#define GPABCDCNR		0xB8003500
#define GPABCDPTYP		0xB8003504
#define	GPABCDDIR		0xB8003508
#define	GPABCDDATA		0xB800350C
#define	GPABCDISR		0xB8003510
#define	GPABIMR			0xB8003514
#define	GPCDIMR			0xB8003518
#define GPEFGHCNR		0xB800351C
#define GPEFGHPTYP		0xB8003520
#define	GPEFGHDIR		0xB8003524
#define	GPEFGHDATA		0xB8003528
#define	GPEFGHISR		0xB800352C
#define	GPEFIMR			0xB8003530
#define	GPGHIMR			0xB8003534
/**************************************************************************/
/* Register access macro (REG*()).*/
#define REG32(reg) 			(*((volatile uint32 *)(reg)))
#define REG16(reg) 			(*((volatile uint16 *)(reg)))
#define REG8(reg) 			(*((volatile uint8 *)(reg)))

/*********************  Function Prototype in gpio.c  ***********************/
int32 _rtl865xC_initGpioPin(uint32 gpioId, enum GPIO_PERIPHERAL dedicate, 
                                           enum GPIO_DIRECTION, 
                                           enum GPIO_INTERRUPT_TYPE interruptEnable );
int32 _rtl865xC_getGpioDataBit( uint32 gpioId, uint32* pData );
int32 _rtl865xC_setGpioDataBit( uint32 gpioId, uint32 data );

#endif//CONFIG_RTK_VOIP_DRIVERS_PCM865xC

#endif /* __GPIO_8952_H */
