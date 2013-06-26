/*
* Copyright c                  Realtek Semiconductor Corporation, 2006 
* All rights reserved.
* 
* Program : Control  MDC/MDIO connected RTL8305Sx
* Abstract : 
* Author : Robin Zheng-bei Xing(robin_xing@realsil.com.cn)                
*  $Id: patch-1.1.0.8,v 1.1.2.1 2009/03/05 09:11:39 thlin Exp $
*/
/*	@doc MDCMDIO_API

	@module mdcmdio.c -  MDC/MDIO API documentation	|
	This document explains API to use two GPIO pin  to simulate MDC/MDIO signal which is 
	based on RTL8651B platform, if you use other platform, please modify this file.	
	@normal 

	Copyright <cp>2006 Realtek<tm> Semiconductor Cooperation, All Rights Reserved.

 	@head3 List of Symbols |
 	Here is a list of all functions and variables in this module.

 	@index | MDCMDIO_API

*/

#include <linux/kernel.h>
#include <linux/interrupt.h>
#include "gpio.h"
#include "gpio_8972b.h"


#define BOOST

void _smiGpioInit(void)
{
	printk(" Set GPIO D6,7 to MDC/IO pin.\n");
	*((volatile unsigned int *)0xB8000030) = 	// PIN Mux select
		*((volatile unsigned int *)0xB8000030) & ~0x00FFFF83 | 0x00500003;
	printk(" value of reg 0xb8000030 = %X\n", *((volatile unsigned int *)0xb8000030));
	
	*((volatile unsigned int *)0xB8003500) &= ~0xC0000000; // config as GPIO pin
	printk(" value of reg 0xB8003500 = %X\n", *((volatile unsigned int *)0xB8003500));

	*((volatile unsigned int *)0xb8003518) &= ~0xF0000000; //Disable D6, D7 interrupt
	printk(" value of reg 0xb8003518 = %X\n", *((volatile unsigned int *)0xb8003518));
	
	*((volatile unsigned int *)0xB8003508) |= 0xC0000000; //set D6, D7 output pin
	printk(" value of reg 0xB8003508 = %X\n", *((volatile unsigned int *)0xB8003508));
}


#ifndef BOOST
#if !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC)
#define smi_MDC         GPIO_ID(GPIO_PORT_F,0)  /* GPIO used for SMI Clock Generation */
#define smi_MDIO        GPIO_ID(GPIO_PORT_F,1)  /* GPIO used for SMI Data signal */
#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY)
//#define smi_MDC         GPIO_ID(GPIO_PORT_E,5)  /* GPIO used for SMI Clock Generation */
//#define smi_MDIO        GPIO_ID(GPIO_PORT_E,6)  /* GPIO used for SMI Data signal */
#define smi_MDC         GPIO_ID(GPIO_PORT_D,6)  /* GPIO used for SMI Clock Generation */
#define smi_MDIO        GPIO_ID(GPIO_PORT_D,7)  /* GPIO used for SMI Data signal */
#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC)
#error "not defined NEED FIXED ME"
#define smi_MDC         GPIO_ID(GPIO_PORT_D,6)  /* GPIO used for SMI Clock Generation */
#define smi_MDIO        GPIO_ID(GPIO_PORT_D,7)  /* GPIO used for SMI Data signal */
#endif

#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) || defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC)
#define DELAY		200
#else
#define DELAY		100
#endif

#define CLK_DURATION(clk)	{ uint32 i; for (i=0; i< (clk); i++); }

/*
Low speed smi is a general MDC/MDIO interface, it is realized by call gpio api function, could
specified any gpio pin as MDC/MDIO
*/

static void _smiGenReadClk(void) 
{
	#if !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC)
	CLK_DURATION(DELAY);
	_rtl865xC_setGpioDataBit(smi_MDC, 1);
	CLK_DURATION(DELAY);
	_rtl865xC_setGpioDataBit(smi_MDC, 0);
	#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY)
	CLK_DURATION(DELAY);
	_rtl8972B_setGpioDataBit(smi_MDC, 1);
	CLK_DURATION(DELAY);
	_rtl8972B_setGpioDataBit(smi_MDC, 0);
	#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC)
	CLK_DURATION(DELAY);
	_rtl8954C_setGpioDataBit(smi_MDC, 1);
	CLK_DURATION(DELAY);
	_rtl8954C_setGpioDataBit(smi_MDC, 0);
	#endif

}

static void _smiGenWriteClk(void) 
{
	#if !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC)
	CLK_DURATION(DELAY);
	_rtl865xC_setGpioDataBit(smi_MDC, 0);
	CLK_DURATION(DELAY);
	_rtl865xC_setGpioDataBit(smi_MDC, 1);
	#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY)
	CLK_DURATION(DELAY);
	_rtl8972B_setGpioDataBit(smi_MDC, 0);
	CLK_DURATION(DELAY);
	_rtl8972B_setGpioDataBit(smi_MDC, 1);
	#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC)
	CLK_DURATION(DELAY);
	_rtl8954C_setGpioDataBit(smi_MDC, 0);
	CLK_DURATION(DELAY);
	_rtl8954C_setGpioDataBit(smi_MDC, 1);
	#endif

}

static void _smiZbit(void)
{	
	#if !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC)
	_rtl865xC_initGpioPin(smi_MDIO, GPIO_PERI_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);
	_rtl865xC_setGpioDataBit(smi_MDC, 0);
	_rtl865xC_setGpioDataBit(smi_MDIO, 0);
	#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY)
	_rtl8972B_initGpioPin(smi_MDIO, GPIO_CONT_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);
	_rtl8972B_setGpioDataBit(smi_MDC, 0);
	_rtl8972B_setGpioDataBit(smi_MDIO, 0);
	#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC)
	_rtl8954C_initGpioPin(smi_MDIO, GPIO_CONT_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);
	_rtl8954C_setGpioDataBit(smi_MDC, 0);
	_rtl8954C_setGpioDataBit(smi_MDIO, 0);
	#endif
	CLK_DURATION(DELAY);
}

/* Generate  1 -> 0 transition and sampled at 1 to 0 transition time,
should not sample at 0->1 transition because some chips stop outputing
at the last bit at rising edge*/
static void _smiReadBit(uint32 * pdata) 
{
	uint32 u;
	#if !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC)
	_rtl865xC_initGpioPin(smi_MDIO, GPIO_PERI_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);
	_rtl865xC_setGpioDataBit(smi_MDC, 1);
	CLK_DURATION(DELAY);
	_rtl865xC_setGpioDataBit(smi_MDC, 0);
	_rtl865xC_getGpioDataBit(smi_MDIO, &u);
	#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY)
	_rtl8972B_initGpioPin(smi_MDIO, GPIO_CONT_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);
	_rtl8972B_setGpioDataBit(smi_MDC, 1);
	CLK_DURATION(DELAY);
	_rtl8972B_setGpioDataBit(smi_MDC, 0);
	_rtl8972B_getGpioDataBit(smi_MDIO, &u);
	#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC)
	_rtl8954C_initGpioPin(smi_MDIO, GPIO_CONT_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);
	_rtl8954C_setGpioDataBit(smi_MDC, 1);
	CLK_DURATION(DELAY);
	_rtl8954C_setGpioDataBit(smi_MDC, 0);
	_rtl8954C_getGpioDataBit(smi_MDIO, &u);
	#endif
	*pdata = u;
}

/* Generate  0 -> 1 transition and put data ready during 0 to 1 whole period */
static void _smiWriteBit(uint32 data) 
{
	#if !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC)
	_rtl865xC_initGpioPin(smi_MDIO, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	if (data) { /*write 1*/
			_rtl865xC_setGpioDataBit(smi_MDIO, 1);
			_rtl865xC_setGpioDataBit(smi_MDC, 0);
			CLK_DURATION(DELAY);
			_rtl865xC_setGpioDataBit(smi_MDC, 1);
	} else {
			_rtl865xC_setGpioDataBit(smi_MDIO, 0);
			_rtl865xC_setGpioDataBit(smi_MDC, 0);
			CLK_DURATION(DELAY);
			_rtl865xC_setGpioDataBit(smi_MDC, 1);				 
	}
	#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY)
	_rtl8972B_initGpioPin(smi_MDIO, GPIO_CONT_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	if (data) { /*write 1*/
			_rtl8972B_setGpioDataBit(smi_MDIO, 1);
			_rtl8972B_setGpioDataBit(smi_MDC, 0);
			CLK_DURATION(DELAY);
			_rtl8972B_setGpioDataBit(smi_MDC, 1);
	} else {
			_rtl8972B_setGpioDataBit(smi_MDIO, 0);
			_rtl8972B_setGpioDataBit(smi_MDC, 0);
			CLK_DURATION(DELAY);
			_rtl8972B_setGpioDataBit(smi_MDC, 1);				 
	}
	#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC)
	_rtl8954C_initGpioPin(smi_MDIO, GPIO_CONT_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	if (data) { /*write 1*/
			_rtl8954C_setGpioDataBit(smi_MDIO, 1);
			_rtl8954C_setGpioDataBit(smi_MDC, 0);
			CLK_DURATION(DELAY);
			_rtl8954C_setGpioDataBit(smi_MDC, 1);
	} else {
			_rtl8954C_setGpioDataBit(smi_MDIO, 0);
			_rtl8954C_setGpioDataBit(smi_MDC, 0);
			CLK_DURATION(DELAY);
			_rtl8954C_setGpioDataBit(smi_MDC, 1);				 
	}
	#endif
}

/*
@func int32 | smiRead | read data from phy register
@parm uint32 | phyad | PHY address (0~31)
@parm uint32 | regad | Register address (0 ~31)
@parm uint32* | data | Register value
@comm 
This function could read register through MDC/MDIO serial 
interface, and it is platform  related. It use two GPIO pins 
to simulate MDC/MDIO timing. MDC is sourced by the Station Management 
entity to the PHY as the timing reference for transfer of information
on the MDIO signal. MDC is an aperiodic signal that has no maximum high 
or low times. The minimum high and low times for MDC shall be 160 ns each, 
and the minimum period for MDC shall be 400 ns. Obeying frame format defined
by IEEE802.3 standard, you could access Phy registers. If you want to 
port it to other CPU, please modify static functions which are called 
by this function.
*/

int32 smiRead(uint32 phyad, uint32 regad, uint16 * data) 
{

	int32 i;
	uint32 readBit;
	//unsigned long flags;

	if ((phyad > 31) || (regad > 31) || (data == NULL))  return	FAILED;

      /*it lock the resource to ensure that SMI opertion is atomic, 
       the API is based on RTL865X, it is used to disable CPU interrupt,
       if porting to other platform, please rewrite it to realize the same function*/
      	rtlglue_drvMutexLock();   
      	//save_flags(flags); cli();

	/* 32 continuous 1 as preamble*/
	for(i=0; i<32; i++)
		_smiWriteBit(1);
	/* ST: Start of Frame, <01>*/
		_smiWriteBit(0);
		_smiWriteBit(1);
	/* OP: Operation code, read is <10> */
		_smiWriteBit(1);
		_smiWriteBit(0);
	/* PHY Address */
	for(i=4; i>=0; i--) 
		_smiWriteBit((phyad>>i)&0x1);
	/* Register Address */
	for(i=4; i>=0; i--) 
		_smiWriteBit((regad>>i)&0x1);
	/* TA: Turnaround <z0>, zbit has no clock in order to steal a clock to sample data at clock falling edge */
		_smiZbit();
		_smiReadBit(&readBit);
	/* Data */
	*data = 0;
	for(i=15; i>=0; i--) {
		_smiReadBit(&readBit);
		*data = (*data<<1) | readBit;
	}

     /*add  an extra clock cycles for robust reading , ensure partner stop output signal
        and not affect the next read operation, because TA steal a clock*/     
#if 1	// Thlin
	_smiGenReadClk();
#else
	_smiWriteBit(1);
#endif
	_smiZbit();
    
	rtlglue_drvMutexUnlock();	/*unlock the source, enable interrupt*/
	//restore_flags(flags);
    
	return	SUCCESS;
}

/*
@func int32 | smiWrite | Write data to Phy register
@parm uint32 | phyad | Phy address (0~31)
@parm uint32 | regad | Register address (0~31)
@parm uint32 | data | data to be written into Phy register
@comm 
This function could write register through MDC/MDIO serial 
interface, and it is platform  related. It use two GPIO pins 
to simulate MDC/MDIO timing. MDC is sourced by the Station Management 
entity to the PHY as the timing reference for transfer of information
on the MDIO signal. MDC is an aperiodic signal that has no maximum high 
or low times. The minimum high and low times for MDC shall be 160 ns each, 
and the minimum period for MDC shall be 400 ns. Obeying frame format defined
by IEEE802.3 standard, you could access Phy registers. If you want to 
port it to other CPU, please modify static functions which are called 
by this function.
*/

int32 smiWrite(uint32 phyad, uint32 regad, uint16 data)
{
	int32 i;
	unsigned long flags;

	if ((phyad > 31) || (regad > 31) || (data > 0xFFFF))  return	FAILED;

      /*it lock the resource to ensure that SMI opertion is atomic, 
       the API is based on RTL865X, it is used to disable CPU interrupt,
       if porting to other platform, please rewrite it to realize the same function*/
      	rtlglue_drvMutexLock(); 
      	//save_flags(flags); cli(); 


	/* 32 continuous 1 as preamble*/
	for(i=0; i<32; i++)
		_smiWriteBit(1);
	/* ST: Start of Frame, <01>*/
		_smiWriteBit(0);
		_smiWriteBit(1);
	/* OP: Operation code, write is <01> */
		_smiWriteBit(0);
		_smiWriteBit(1);
	/* PHY Address */
	for(i=4; i>=0; i--) 
		_smiWriteBit((phyad>>i)&0x1);
	/* Register Address */
	for(i=4; i>=0; i--) 
		_smiWriteBit((regad>>i)&0x1);
	/* TA: Turnaround <10> */
		_smiWriteBit(1);
		_smiWriteBit(0);
	/* Data */
	for(i=15; i>=0; i--) 
		_smiWriteBit((data>>i)&0x1);
	_smiGenWriteClk();
	_smiZbit();

	rtlglue_drvMutexUnlock();	/*unlock the source, enable interrupt*/      
	//restore_flags(flags);
            
	return 	SUCCESS;	
}


/*
int32 | smiInit | Init Rtl8651B smi interface
uint32 | port | Specify Rtl8651B GPIO port
uint32 | pinMDC | Set which gpio pin as MDC 
uint32 | pinMDIO | Set which gpio pin as MDIO
This function is only for Rtl8651B, use it to specify
GPIO pins as MDC/MDIO signal. It should be called at first.
*/
int32 smiInit()
{
	#if !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC)
	/* Initialize GPIO smi_MDC  as SMI MDC signal */
	_rtl865xC_initGpioPin(smi_MDC, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	
	/* Initialize GPIO smi_MDIO  as SMI MDIO signal */
	_rtl865xC_initGpioPin(smi_MDIO, GPIO_PERI_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);
	#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY)
	/* Initialize GPIO smi_MDC  as SMI MDC signal */
	_rtl8972B_initGpioPin(smi_MDC, GPIO_CONT_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	
	/* Initialize GPIO smi_MDIO  as SMI MDIO signal */
	_rtl8972B_initGpioPin(smi_MDIO, GPIO_CONT_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);
	#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC)
	/* Initialize GPIO smi_MDC  as SMI MDC signal */
	_rtl8954C_initGpioPin(smi_MDC, GPIO_CONT_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	
	/* Initialize GPIO smi_MDIO  as SMI MDIO signal */
	_rtl8954C_initGpioPin(smi_MDIO, GPIO_CONT_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);
	#endif
	
	return SUCCESS;
}

#else  /*high speed smi, , */

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY

//#define GPIO_E56
#define GPIO_D67	// D6: MDC, D7: MDIO

#endif

/*
high speed smi is a special interface, it is realsized by configuring rtl865x gpio register directly,
it specifies GPIO port F pin0 as MDC, pin 1 as MDIO
*/

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY
#define DELAY		50
#else
#define DELAY		100
#endif

static void _smiGenReadClk(void) 
{
	unsigned short i;
	#ifndef CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY
	#error "Need implement for _smiGenReadClk"
	#else
	REG32(GPABCDDIR) = (REG32(GPABCDDIR)& 0x3FFFFFFF) | 0x40000000;		// MDIO=IN, MDC=OUT
	REG32(GPABCDDATA) = (REG32(GPABCDDATA) & 0x3FFFFFFF) | 0x40000000;	// MDC=1, MDIO=0
	for(i=0; i< DELAY; i++);
	REG32(GPABCDDATA) = (REG32(GPABCDDATA) & 0x3FFFFFFF);			// MDC=0
	#endif

}

static void _smiGenWriteClk(void) 
{
	unsigned short i;
	#ifndef CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY
	#error "Need implement for _smiGenWriteClk"
	#else
	for(i=0; i< DELAY; i++);
	REG32(GPABCDDATA) = (REG32(GPABCDDATA) & 0x3FFFFFFF);			// MDC=0, MDIO=0
	for(i=0; i< DELAY; i++);
	REG32(GPABCDDATA) = (REG32(GPABCDDATA) & 0xBFFFFFFF) | 0x40000000;	// MDC=1
	#endif

}

/* Change clock to 1 */
static void _smiZBit(void) {
	unsigned short i;
#ifndef CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY
	REG32(GPEFGHDIR) = (REG32(GPEFGHDIR) & 0xFFFFFCFF) | 0x100;
	REG32(GPEFGHDATA) = (REG32(GPEFGHDATA) & 0xFFFFFCFF);
#else
    #ifdef GPIO_E56
	REG32(GPEFGHDIR) = (REG32(GPEFGHDIR) & 0xFFFFFF9F) | 0x20;
	REG32(GPEFGHDATA) = (REG32(GPEFGHDATA) & 0xFFFFFF9F);
    #elif defined GPIO_D67
    	REG32(GPABCDDIR) = (REG32(GPABCDDIR) & 0x3FFFFFFF) | 0x40000000;// MDIO=IN, MDC=OUT
	REG32(GPABCDDATA) = (REG32(GPABCDDATA) & 0x3FFFFFFF);	// MDIO=0, MDC=0
    #endif
#endif
	for(i=0; i< DELAY; i++);
}

/* Generate  1 -> 0 transition and sampled at 1 to 0 transition time,
should not sample at 0->1 transition because some chips stop outputing
at the last bit at rising edge*/

static void _smiReadBit(unsigned short * pdata) {
	unsigned short i;
#ifndef CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY
	REG32(GPEFGHDIR) = (REG32(GPEFGHDIR)& 0xFFFFFCFF) | 0x100;
	REG32(GPEFGHDATA) = (REG32(GPEFGHDATA) & 0xFFFFFCFF) | 0x100;
	for(i=0; i< DELAY; i++);
	REG32(GPEFGHDATA) = (REG32(GPEFGHDATA) & 0xFFFFFCFF);
	*pdata = (REG32(GPEFGHDATA) & 0x200)?1:0; 
#else
    #ifdef GPIO_E56
	REG32(GPEFGHDIR) = (REG32(GPEFGHDIR)& 0xFFFFFF9F) | 0x20;
	REG32(GPEFGHDATA) = (REG32(GPEFGHDATA) & 0xFFFFFF9F) | 0x20;
	for(i=0; i< DELAY; i++);
	REG32(GPEFGHDATA) = (REG32(GPEFGHDATA) & 0xFFFFFF9F);
	*pdata = (REG32(GPEFGHDATA) & 0x40)?1:0; 
    #elif defined GPIO_D67
	REG32(GPABCDDIR) = (REG32(GPABCDDIR)& 0x3FFFFFFF) | 0x40000000;	// MDIO=IN, MDC=OUT
	REG32(GPABCDDATA) = (REG32(GPABCDDATA) & 0x3FFFFFFF) | 0x40000000; // MDC=1, MDIO=0
	for(i=0; i< DELAY; i++);
	REG32(GPABCDDATA) = (REG32(GPABCDDATA) & 0x3FFFFFFF);	// MDC=0
	*pdata = (REG32(GPABCDDATA) & 0x80000000)?1:0; // Get MDIO value
    #endif
#endif
}

/* Generate  0 -> 1 transition and put data ready during 0 to 1 whole period */
static void _smiWriteBit(unsigned short data) {
	unsigned short i;
#ifndef CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY
	REG32(GPEFGHDIR) = REG32(GPEFGHDIR) | 0x300;
	if(data) {/* Write 1 */
		REG32(GPEFGHDATA) = (REG32(GPEFGHDATA) & 0xFFFFFCFF) | 0x200;
		for(i=0; i< DELAY; i++);
		REG32(GPEFGHDATA) = (REG32(GPEFGHDATA) & 0xFFFFFCFF) | 0x300;
	} else {
		REG32(GPEFGHDATA) = (REG32(GPEFGHDATA) & 0xFFFFFCFF);
		for(i=0; i< DELAY; i++);
		REG32(GPEFGHDATA) = (REG32(GPEFGHDATA) & 0xFFFFFCFF) | 0x100;
	}
#else
    #ifdef GPIO_E56
	REG32(GPEFGHDIR) = REG32(GPEFGHDIR) | 0x60;
	if(data) {/* Write 1 */
		REG32(GPEFGHDATA) = (REG32(GPEFGHDATA) & 0xFFFFFCFF) | 0x40;
		for(i=0; i< DELAY; i++);
		REG32(GPEFGHDATA) = (REG32(GPEFGHDATA) & 0xFFFFFCFF) | 0x60;
	} else {
		REG32(GPEFGHDATA) = (REG32(GPEFGHDATA) & 0xFFFFFCFF);
		for(i=0; i< DELAY; i++);
		REG32(GPEFGHDATA) = (REG32(GPEFGHDATA) & 0xFFFFFCFF) | 0x20;
	}
    #elif defined GPIO_D67
    	REG32(GPABCDDIR) = REG32(GPABCDDIR) | 0xC0000000; // set MDIO, MDC to output
	if(data) {/* Write 1 */
		REG32(GPABCDDATA) = (REG32(GPABCDDATA) & 0x3FFFFFFF) | 0x80000000; // MDC=0, MDIO=1
		for(i=0; i< DELAY; i++);
		REG32(GPABCDDATA) = (REG32(GPABCDDATA) & 0xBFFFFFFF) | 0x40000000; // MDC=1
	} else {
		REG32(GPABCDDATA) = (REG32(GPABCDDATA) & 0x3FFFFFFF);		// MDC=0, MDIO=0
		for(i=0; i< DELAY; i++);
		REG32(GPABCDDATA) = (REG32(GPABCDDATA) & 0xBFFFFFFF) | 0x40000000;// MDC=1
	}
    #endif
#endif
}

int smiRead(unsigned char phyad, unsigned short regad, unsigned short * data) {
	int i;
	unsigned short readBit;

	/* Configure port C pin 1, 0 to be GPIO and disable interrupts of these two pins */
#ifndef CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY
	REG32(GPEFGHCNR) = REG32(GPEFGHCNR) & 0xFFFFFCFF;
	REG32(GPEFIMR) = REG32(GPEFIMR) & 0xFFFFFFF;
#else
    #ifdef GPIO_E56
	REG32(GPEFGHCNR) = REG32(GPEFGHCNR) & 0xFFFFFF9F;
	REG32(GPEFIMR) = REG32(GPEFIMR) & 0xFFFFFFF;
    #elif defined GPIO_D67
    	//REG32(GPABCDCNR) = REG32(GPABCDCNR) & 0xFFFFFF9F;
	//REG32(GPCDIMR) = REG32(GPCDIMR) & 0xFFFFFFF;
    #endif
#endif
	/* 32 continuous 1 as preamble*/
	for(i=0; i<32; i++)
		_smiWriteBit(1);
	/* ST: Start of Frame, <01>*/
	_smiWriteBit(0);
	_smiWriteBit(1);
	/* OP: Operation code, read is <10> */
	_smiWriteBit(1);
	_smiWriteBit(0);
	/* PHY Address */
	for(i=4; i>=0; i--) 
		_smiWriteBit((phyad>>i)&0x1);
	/* Register Address */
	for(i=4; i>=0; i--) 
		_smiWriteBit((regad>>i)&0x1);
	/* TA: Turnaround <z0> */
	_smiZBit();
	_smiReadBit(&readBit);
	/* Data */
	*data = 0;
	for(i=15; i>=0; i--) {
		_smiReadBit(&readBit);
		*data = (*data<<1) | readBit;
	}
        /*add  an extra clock cycles for robust reading , ensure partner stop output signal
        and not affect the next read operation, because TA steal a clock*/     
	_smiWriteBit(1);
	_smiZBit();
    
	return SUCCESS ;
}

int smiWrite(unsigned char phyad, unsigned short regad, unsigned short data) {
	int i;

	/* Configure port C pin 1, 0 to be GPIO and disable interrupts of these two pins */
#ifndef CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY
	REG32(GPEFGHCNR) = REG32(GPEFGHCNR) & 0xFFFFFCFF;
	REG32(GPEFIMR) = REG32(GPEFIMR) & 0xFFFFFFF;
#else
    #ifdef GPIO_E56
	REG32(GPEFGHCNR) = REG32(GPEFGHCNR) & 0xFFFFFF9F;
	REG32(GPEFIMR) = REG32(GPEFIMR) & 0xFFFFFFF;
    #elif defined GPIO_D67
    	//REG32(GPABCDCNR) = REG32(GPABCDCNR) & 0xFFFFFF9F;
	//REG32(GPCDIMR) = REG32(GPCDIMR) & 0xFFFFFFF;
    #endif
#endif
	/* 32 continuous 1 as preamble*/
	for(i=0; i<32; i++)
		_smiWriteBit(1);
	/* ST: Start of Frame, <01>*/
	_smiWriteBit(0);
	_smiWriteBit(1);
	/* OP: Operation code, write is <01> */
	_smiWriteBit(0);
	_smiWriteBit(1);
	/* PHY Address */
	for(i=4; i>=0; i--) 
		_smiWriteBit((phyad>>i)&0x1);
	/* Register Address */
	for(i=4; i>=0; i--) 
		_smiWriteBit((regad>>i)&0x1);
	/* TA: Turnaround <10> */
	_smiWriteBit(1);
	_smiWriteBit(0);
	/* Data */
	for(i=15; i>=0; i--) 
		_smiWriteBit((data>>i)&0x1);
	_smiGenWriteClk();
	_smiZBit();


	return SUCCESS;
}


#endif /* if 	SMI_HIGHSPEED*/

/*
@func int32 | smiReadBit | read one bit of PHY register
@parm uint32 | phyad | Phy address (0~31)
@parm uint32 | regad | Register address (0~31)
@parm uint32 | bit | Register bit (0~15)
@parm uint32* | pdata | Bit value
*/
int smiReadBit(unsigned char phyad, unsigned short regad, unsigned short bit, unsigned short * pdata) 
{
	unsigned short regData;

	if ((phyad > 31) || (regad > 31) || (bit > 15) || (pdata == NULL) )  return	FAILED;
	if(bit>=16)
		* pdata = 0;
	else {
		smiRead(phyad, regad, &regData);
		if(regData & (1<<bit)) 
			* pdata = 1;
		else
			* pdata = 0;
	}
	
	return SUCCESS;
}

/*
@func int32 | smiWriteBit | read one bit of PHY register
@parm uint32 | phyad | Phy address (0~31)
@parm uint32 | regad | Register address (0~31)
@parm uint32 | bit | Register bit (0~15)
@parm uint32 | data | Bit value to be written
*/

int smiWriteBit(unsigned char phyad, unsigned short regad, unsigned short bit, unsigned short data) 
{
	unsigned short regData;
	
	if ((phyad > 31) || (regad > 31) || (bit > 15) || (data > 1) ) 	return	FAILED;
	smiRead(phyad, regad, &regData);
	if(data) 
		regData = regData | (1<<bit);
	else
		regData = regData & ~(1<<bit);
	smiWrite(phyad, regad, regData);
	
	return SUCCESS;
}




















