#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/string.h>
#include <linux/delay.h>
#include "rtk_voip.h"
#include "gpio/gpio.h"
#include "spi.h"

extern rtl_spi_dev_t spi_dev[SPI_DEV_NUM];


/************************************* Set GPIO Pin to SPI ***********************************************************/
/*
@func int32 | _rtl8972B_spi_init | Initialize SPI device
@parm rtl_spi_dev_t* | pDev | Structure to store device information
@parm uint32 | gpioSCLK | GPIO ID of SCLK
@parm uint32 | gpioCS_ | GPIO ID of CS_
@parm uint32 | gpioSDI | GPIO ID of SDI
@parm uint32 | gpioSDO | GPIO ID of SDO
@parm uint32 | maxSpeed | how fast SPI driver can generate the SCLK signal (unit: HZ)
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
//#ifdef CONFIG_RTK_VOIP_DECT_SPI_SUPPORT
int32 __rtl8972B_spi_init( rtl_spi_dev_t* pDev, uint32 gpioSCLK, uint32 gpioCS_, uint32 gpioSDO, uint32 gpioSDI, int32 gpioINT )
//#else
///*static*/ int32 _rtl8972B_spi_init( rtl_spi_dev_t* pDev, uint32 gpioSCLK, uint32 gpioCS_, uint32 gpioSDO, uint32 gpioSDI)
//#endif
{
	
	pDev->gpioSCLK = gpioSCLK;
	pDev->gpioCS_ = gpioCS_;
	pDev->gpioSDI = gpioSDI;
	pDev->gpioSDO = gpioSDO;
#ifdef CONFIG_RTK_VOIP_DECT_SPI_SUPPORT
	pDev->gpioINT = gpioINT;
#endif
	//pDev->SClkDelayLoop = SysClock / maxSpeed;
	

	_rtl8972B_initGpioPin( gpioSCLK, GPIO_CONT_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE );
	_rtl8972B_initGpioPin( gpioCS_, GPIO_CONT_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE );
	_rtl8972B_initGpioPin( gpioSDI, GPIO_CONT_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE );
	_rtl8972B_initGpioPin( gpioSDO, GPIO_CONT_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE );
#ifdef CONFIG_RTK_VOIP_DECT_SPI_SUPPORT
	_rtl8972B_initGpioPin( gpioINT, GPIO_CONT_GPIO, GPIO_DIR_IN, 0x3 ); //Enable both falling or rising edge interrupt
#endif
	
	return SUCCESS;
}


/*
@func int32 | _rtl8972B_spi_exit | Called when a SPI device is released
@parm rtl_spi_dev_t* | pDev | Structure containing device information
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
int32 _rtl8972B_spi_exit( rtl_spi_dev_t* pDev )
{
	return SUCCESS;
}
 

void _rtl8972B_init_spi_channels(int size, rtl_spi_dev_t* pDev[], uint32 pin_cs[], uint32 pin_reset, uint32 pin_clk, uint32 pin_do, uint32 pin_di)
{
//This API is used for SLIC gpio init and SLIC reset
#ifndef CONFIG_RTK_VOIP_DECT_SPI_SUPPORT
	int i;
	_rtl8972B_initGpioPin(pin_reset, GPIO_CONT_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	for( i = 0; i < size; i ++ ) {
		_rtl8972B_initGpioPin(pin_cs[i], GPIO_CONT_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
		_rtl8972B_spi_init(pDev[i], pin_clk, pin_cs[i], pin_do, pin_di);
	}
	
	#if speed_booting_rating
		for(i=0;i<50000;i++);
	#else
		for(i=0;i<80000000;i++);	//~ 500 msec
		//cyg_thread_delay(50);
	#endif
	for( i = 0; i < size; i ++ )
		_rtl8972B_setGpioDataBit(pin_cs[i], 1); 	/* CS_ preset to high state*/
	_rtl8972B_setGpioDataBit(pin_clk, 1); 	/* SCLK preset to high state*/
	
	_rtl8972B_setGpioDataBit(pin_reset, 1);	// reset high
	#if speed_booting_rating
		for(i=0;i<50000;i++);
	#else
		for(i=0;i<80000000;i++);
		//cyg_thread_delay(50);
	#endif
	_rtl8972B_setGpioDataBit(pin_reset, 0);	// set reset low, PCLK and FS id present and stable.
	#if speed_booting_rating
		for(i=0;i<50000;i++);
	#else
		for(i=0;i<80000000;i++);
		//cyg_thread_delay(50);
	#endif
	_rtl8972B_setGpioDataBit(pin_reset, 1);	// release reset
	

	#if speed_booting_rating
		for(i=0;i<50000;i++);
	#else
		for(i=0;i<80000000;i++);
		//cyg_thread_delay(50);
	#endif				// wait more than 100ms 
#endif
}

void init_channel(int channel, uint32 pin_cs, uint32 pin_reset, uint32 pin_clk, uint32 pin_do, uint32 pin_di)
{
	rtl_spi_dev_t * spi_devs[ 1 ] = { &spi_dev[channel] };
	
	_rtl8972B_init_spi_channels( 1, spi_devs, &pin_cs, pin_reset, pin_clk, pin_do, pin_di );
}

//------------------------------------------
void init_spi(int ch_spi)
{
	
	printk("Init GPIO %s as SPI and do reset ", GPIO );

#ifndef CONFIG_RTK_VOIP_DRIVERS_ATA_DECT
#ifdef CONFIG_RTK_VOIP_MULTIPLE_SI32178
	if (ch_spi == 0)
	{
		printk("for SLIC[%d]...", ch_spi);
		init_channel(0, PIN_CS1, PIN_RESET1, PIN_CLK, PIN_DO, PIN_DI);
		
		printk("=> OK !\n");
	}
	else if (ch_spi == 1)
	{
		printk("for SLIC[%d]...", ch_spi);
		init_channel(1, PIN_CS2, PIN_RESET1, PIN_CLK, PIN_DO, PIN_DI);
		
		printk("=> OK !\n");
	}
	else if (ch_spi == 2)
	{
		printk("for SLIC[%d]...", ch_spi);
		init_channel(2, PIN_CS3, PIN_RESET1, PIN_CLK, PIN_DO, PIN_DI);
		
		printk("=> OK !\n");
	}
	else if (ch_spi == 3)
	{
		printk("for SLIC[%d]...", ch_spi);
		init_channel(3, PIN_CS4, PIN_RESET1, PIN_CLK, PIN_DO, PIN_DI);
		
		printk("=> OK !\n");
	}
	else 
	{
		printk("No GPIO Pin assign for any device -> Can't initialize any device\n");
		
	}
#else
	if (ch_spi == 0)
	{
		printk("for SLIC[%d]...", ch_spi);
		init_channel(0, PIN_CS1, PIN_RESET1, PIN_CLK, PIN_DO, PIN_DI);
		
		printk("=> OK !\n");
	}
	else if (ch_spi == 1)
	{
		printk("for DAA[%d]...", ch_spi);
		init_channel(1, PIN_CS3_DAA, PIN_RESET3_DAA, PIN_CLK_DAA, PIN_DO_DAA, PIN_DI_DAA);
		printk("=> OK !\n");
	}
	else 
	{
		printk("No GPIO Pin assign for any device -> Can't initialize any device\n");
		
	}
#endif
#endif//!CONFIG_RTK_VOIP_DECT_SPI_SUPPORT

#if 0	
	#define GPIOGTNR *((volatile unsigned int *)0xb800351c)
	#define GPIOGDIR *((volatile unsigned int *)0xb8003524)
	#define GPIOGDATA *((volatile unsigned int *)0xb8003528)
	GPIOGTNR &= 0xffdfffff;
	GPIOGDIR |= 0x00200000;
	while (1) {
	GPIOGDATA &= 0xffdfffff;;
	 GPIOGDATA |= 0x00200000;
	//_rtl8972B_setGpioDataBit(spi_dev[0].gpioSCLK, 0);
	//_rtl8972B_setGpioDataBit(spi_dev[0].gpioSCLK, 1);	
	};
#endif
	
}


