/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : GPIO Driver
* Abstract : 
* Author : 
*/

#include "gpio.h"


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

extern void gpioConfig (int gpio_num, int gpio_func);
extern void gpioSet(int gpio_num);
extern void gpioClear(int gpio_num);
extern int gpioRead(int gpio_num) ;

int gpio_debug = 0;
extern int spi_dbg;
// Goal : unifiy GPIO functions
//static uint32 _getGpio( enum GPIO_FUNC func, enum GPIO_PORT port, uint32 pin )
static uint32 _getGpio( unsigned char port, uint32 pin )
{
	return 0;
}
//static void _setGpio( enum GPIO_FUNC func, enum GPIO_PORT port, uint32 pin, uint32 data )
static void _setGpio( unsigned char port, uint32 pin, uint32 data )
{
	return 0;
}
//int32 _rtl867x_initGpioPin( uint32 gpioId, enum GPIO_DIRECTION direction, 
//                                           enum GPIO_INTERRUPT_TYPE interruptEnable )
int32 _rtl867x_initGpioPin(uint32 gpioId, unsigned char func)
{
    if(func == 0)
        gpioConfig(gpioId, 0x01);
    else
        gpioConfig(gpioId, 0x02);
}

int32 _rtl867x_getGpioDataBit( uint32 gpioId, uint32* pData )
{
//	unsigned int val;
//	if (gpioId >7)	return 0; //skip non-GPA

//	val = *(volatile int*)GPABCDDATA;

//	*pData = (val >> gpioId) & 0x1 ;
	*pData = gpioRead(gpioId);

}

int32 _rtl867x_setGpioDataBit( uint32 gpioId, uint32 data )
{

    if( data == 1)
        gpioSet(gpioId);
    else
        gpioClear(gpioId);

	return 0;
}


