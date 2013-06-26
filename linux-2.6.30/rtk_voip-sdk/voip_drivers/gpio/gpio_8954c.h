/*
* Copyright c                  Realtek Semiconductor Corporation, 2009
* All rights reserved.
* 
* Program : GPIO Header File
* Abstract :
* Author :
* 
*/

 
#ifndef __GPIO_8954C_H__
#define __GPIO_8954C_H__

/* For 8964C QA Board */
#if defined (CONFIG_RTK_VOIP_GPIO_8964C_QA)
#include "gpio_8954c_QA.h"
#endif 

/* For 8954C V100 EV Board */
#if defined (CONFIG_RTK_VOIP_GPIO_8954C_V100) 
#include "gpio_8954c_v100.h"
#endif 

/* For 8954C V200 EV Board */
#if defined (CONFIG_RTK_VOIP_GPIO_8954C_V200)
#include "gpio_8954c_v200.h"
#endif

/* For 8954C V400 EV Board */
#if defined (CONFIG_RTK_VOIP_GPIO_8954C_V400)
#include "gpio_8954c_v400.h"
#endif

/* For 8954C PMC GPON EV Board */
#if defined (CONFIG_RTK_VOIP_GPIO_8954C_PMC)
#include "gpio_8954c_PMC.h"
#endif

/* For 8954C SOUNDWIN  XVN1420 EV Board */
#if defined (CONFIG_RTK_VOIP_GPIO_8954C_SOUNDWIN_XVN1420)
#include "gpio_8954c_soundwin_xvn1420.h"
#endif

/* For IP Phone */
#if defined (CONFIG_RTK_VOIP_DRIVERS_IP_PHONE)
/* NOT support yet */
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM89xxC

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
	GPIO_PORT_UNDEF,
};

/* define GPIO control pin */
enum GPIO_CONTROL
{
	GPIO_CONT_GPIO = 0,
	GPIO_CONT_PERI = 0x1,
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

/*************** Define RTL8954C Family GPIO Register Set ************************/
#define GPABCDCNR		0xB8003500

#define	GPABCDDIR		0xB8003508
#define	GPABCDDATA		0xB800350C
#define	GPABCDISR		0xB8003510
#define	GPABIMR			0xB8003514
#define	GPCDIMR			0xB8003518
#define GPEFGHCNR		0xB800351C

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
int32 _rtl8954C_initGpioPin(uint32 gpioId, enum GPIO_CONTROL dedicate, 
                                           enum GPIO_DIRECTION, 
                                           enum GPIO_INTERRUPT_TYPE interruptEnable );
int32 _rtl8954C_getGpioDataBit( uint32 gpioId, uint32* pData );
int32 _rtl8954C_setGpioDataBit( uint32 gpioId, uint32 data );

#endif /* CONFIG_RTK_VOIP_DRIVERS_PCM89xxC */

#endif /* __GPIO_8954C_H__ */

