/*
* Copyright c                  Realtek Semiconductor Corporation, 2006  
* All rights reserved.
* 
* Program : GPIO Header File 
* Abstract : 
* Author :                
* 
*/

 
#ifndef __GPIO_8676_H
#define __GPIO_8676_H
#include <gpio.h>
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

#if defined (CONFIG_RTK_VOIP_GPIO_8676P_IAD_2LAYER_DEMO_BOARD_V01A) || defined (CONFIG_RTK_VOIP_GPIO_8676PN_IAD_2LAYER_DEMO_BOARD_V01)
#define N8676_GPIO_SIM
#ifdef N8676_GPIO_SIM
#define GPIO "ABCDEFGH"

/*slic used*/
#ifdef CONFIG_6028_IAD_BGA_PIN_DEF
//generic iad v.E
#define PIN_RESET1	30	//D6
#define PIN_CS1		5	//A5
#define PIN_CLK		21	//C5
#define PIN_DI		20	//C4
#define PIN_DO		22	//C6
#define PIN_RELAY	4	//A4
//#define PIN_RELAY	29	//D5
#endif

#ifdef CONFIG_8676_IAD_SILAB3217X
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3217x
#define PIN_RESET1  GPIO_H_1 //57  //H1
#define PIN_RESET2  GPIO_H_1 //57  //H1
#define PIN_CS1     GPIO_D_7 //31  //D7
#define PIN_CS2     GPIO_D_7 //31  //D7
#define PIN_CS3     GPIO_D_7 //31  //D7
#define PIN_CS4     GPIO_D_7 //31  //D7
#define PIN_CLK     GPIO_E_0 //32  //E0
#define PIN_DI      GPIO_E_2 //34  //E2
#define PIN_DO      GPIO_E_1 //33  //E1
#define PIN_RELAY   GPIO_D_6 //26  //D2
#elif defined CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226
#define PIN_RESET1  GPIO_H_1 //57  //H1
#define PIN_RESET2  GPIO_H_1 //57  //H1
#define PIN_CS1     GPIO_D_0 //31  //D7
#define PIN_CS2     GPIO_D_0 //31  //D7
#define PIN_CS3     GPIO_D_0 //31  //D7
#define PIN_CS4     GPIO_D_0 //31  //D7
#define PIN_CLK     GPIO_D_1 //32  //E0
#define PIN_DI      GPIO_D_3 //34  //E2
#define PIN_DO      GPIO_D_2 //33  //E1
#define PIN_RELAY   GPIO_D_6 //26  //D2
#endif
#endif

#ifdef CONFIG_8676_IAD_ZARLINK
#ifndef CONFIG_BOARD_011 //someone EVB can not use this reset pin, it share with other reset pin
#define PIN_RESET1  GPIO_H_1 //57 //H1 
#define PIN_RESET2  GPIO_H_1 //57 //H1 
#endif
#define PIN_CSEN    GPIO_F_5
#ifdef CONFIG_RTK_VOIP_8676_SPI_GPIO
#define PIN_CS1     PIN_CSEN
#else
#define PIN_CS1     GPIO_D_6 //30 //D6 
#endif
#define PIN_CS2     GPIO_D_7 //31 //D7
#define PIN_CS3     GPIO_D_5 //29 //D5 
#define PIN_CS4     GPIO_D_4 //28 //D4 
#define PIN_CLK     GPIO_E_0 //32 //E0 
#define PIN_DI      GPIO_E_2 //34 //E2 
#define PIN_DO      GPIO_E_1 //33 //E1 
#define PIN_RELAY   GPIO_A_5 //5  //A5, just temp. for the unuseless relay pin. put it on the spi_int.
#endif

#if 0
/* DAA used*/
#ifdef CONFIG_6028_IAD_BGA_PIN_DEF
#define PIN_RESET3_DAA		3	//A2
#define PIN_CS3_DAA			26	//D2
#define PIN_CLK_DAA			21	//C5
#define PIN_DI_DAA			20	//C4 	//input
#define PIN_DO_DAA			22	//C6	//output
#endif

#ifdef CONFIG_6166_IAD_SILAB3217X
#define PIN_RESET3_DAA      28  //D4  
#define PIN_CS3_DAA         8   //B0
#define PIN_CLK_DAA         7   //A7
#define PIN_DI_DAA          9   //B1    //input
#define PIN_DO_DAA          10  //B2    //output
#endif

#ifdef CONFIG_6166_IAD_ZARLINK
#define PIN_RESET3_DAA      28  //D4  
#define PIN_CS3_DAA         8   //B0
#define PIN_CLK_DAA         7   //A7
#define PIN_DI_DAA          9   //B1    //input
#define PIN_DO_DAA          10  //B2    //output
#endif
#endif

#else 
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
#endif //! N8676_GPIO_SIM
#endif //! CONFIG_RTK_VOIP_GPIO_8676P_IAD_2LAYER_DEMO_BOARD_V01A


/* LED */ 
#if defined (CONFIG_RTK_VOIP_GPIO_8676P_IAD_2LAYER_DEMO_BOARD_V01A) || defined (CONFIG_RTK_VOIP_GPIO_8676PN_IAD_2LAYER_DEMO_BOARD_V01)
/* 8676P IAD 1FXS EVB use below GPIO as SLIC CS */
#define PIN_VOIP0_LED	GPIO_H_3
#endif

/*====================END FOR RTL867x EVB gpio pin ==================*/

/*==================== FOR RTL8676 ==================*/
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8676
typedef struct rtl_8676_gpio
{
	unsigned int GPABCDCNR;
	unsigned int GPABCDPTYP;
	unsigned int GPABCDDIR;
	unsigned int GPABCDDATA;
	unsigned int GPABCDISR;
	unsigned int GPABIMR;
	unsigned int GPCDIMR;
	unsigned int GPEFGHCNR;
	unsigned int GPEFGHPTYP;
	unsigned int GPEFGHDIR;
	unsigned int GPEFGHDATA;
	unsigned int GPEFGHISR;
	unsigned int GPEFIMR;
	unsigned int GPGHIMR;
} rtl_8676_gpio_t ;
#endif
/*==================== FOR RTL867x ==================*/
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8676

/******** GPIO define ********/


/* define GPIO port */
enum GPIO_PORT
{
	GPIO_PORT_A = 0,
	GPIO_PORT_B,
	GPIO_PORT_MAX,
	GPIO_PORT_UNDEF,
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
//int32 _rtl867x_initGpioPin( uint32 gpioId, enum GPIO_DIRECTION direction, enum GPIO_INTERRUPT_TYPE interruptEnable );
int32 _rtl867x_initGpioPin( uint32 gpioId, unsigned char func );
int32 _rtl867x_getGpioDataBit( uint32 gpioId, uint32* pData );
int32 _rtl867x_setGpioDataBit( uint32 gpioId, uint32 data );

#endif//CONFIG_RTK_VOIP_DRIVERS_PCM8671

#endif/*__GPIO__*/
