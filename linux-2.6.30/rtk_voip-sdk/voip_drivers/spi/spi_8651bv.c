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


/*
@func int32 | _rtl865x_spi_init | Initialize SPI device
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
int32 _rtl865x_spi_init( rtl_spi_dev_t* pDev, uint32 gpioSCLK, uint32 gpioCS_, uint32 gpioSDI, uint32 gpioSDO)
{
	pDev->gpioSCLK = gpioSCLK;
	pDev->gpioCS_ = gpioCS_;
	pDev->gpioSDI = gpioSDI;
	pDev->gpioSDO = gpioSDO;

	_rtl865x_initGpioPin( gpioSCLK, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE );
	_rtl865x_initGpioPin( gpioCS_, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE );
	_rtl865x_initGpioPin( gpioSDI, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE );
	_rtl865x_initGpioPin( gpioSDO, GPIO_PERI_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE );
	
	return SUCCESS;
}



/*
@func int32 | _rtl865x_spi_exit | Called when a SPI device is released
@parm rtl_spi_dev_t* | pDev | Structure containing device information
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
int32 _rtl865x_spi_exit( rtl_spi_dev_t* pDev )
{
	return SUCCESS;
}


void init_channel(int channel, uint32 pin_cs, uint32 pin_reset, uint32 pin_clk, uint32 pin_do, uint32 pin_di, uint32 pin_led)
{
	int i;
	_rtl865x_initGpioPin(pin_cs,    GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl865x_initGpioPin(pin_reset, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl865x_spi_init(&spi_dev[channel], pin_clk, pin_cs, pin_do, pin_di);
	/* jason++ 2005/11/15 add for led support */	
	_rtl865x_initGpioPin(pin_led, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl865x_setGpioDataBit(pin_led, 1);	// turn off during the initial time

	#if speed_booting_rating
		for(i=0;i<50000000;i++);
	#else
	cyg_thread_delay(15);
	#endif
	_rtl865x_setGpioDataBit(pin_cs, 1); /* CS_ */
	_rtl865x_setGpioDataBit(pin_clk, 1); /* SCLK */
	
	_rtl865x_setGpioDataBit(pin_reset, 1);
	#if speed_booting_rating
		for(i=0;i<5000000;i++);
	#else
	cyg_thread_delay(15);
	#endif
	
	_rtl865x_setGpioDataBit(pin_reset, 0);
	#if speed_booting_rating
		for(i=0;i<5000000;i++);
	#else
		cyg_thread_delay(15);
	#endif						/* wait more than 100ms */
	_rtl865x_setGpioDataBit(pin_reset, 1);
	#if speed_booting_rating
		for(i=0;i<5000000;i++);
	#else
	cyg_thread_delay(15);
	#endif

}

void init_spi(int ch_spi)
{
	static unsigned int relay_flag = 0;
	if(relay_flag == 0)
		_rtl865x_initGpioPin(PIN_RELAY, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);

	/*Slic 0*/
	if(ch_spi==SLIC0_SPI_DEV)
	{
		printk("Init gpio for spi of SLIC0.\n");
		init_channel(0, PIN_CS1, PIN_RESET1, PIN_CLK, PIN_DO, PIN_DI, PIN_LED1);
	}
	/*Slic 1*/
	else if(ch_spi==SLIC1_SPI_DEV)
	{
		printk("Init gpio for spi of SLIC1.\n");
		init_channel(1, PIN_CS2, PIN_RESET2, PIN_CLK, PIN_DO, PIN_DI, PIN_LED2);
	}
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651_2S_OC
	else if (ch_spi == DAA0_SPI_DEV)
	{
		printk("for DAA[%d]...", ch_spi);
		init_channel(ch_spi, PIN_CS3_DAA, PIN_RESET3_DAA, PIN_CLK_DAA, PIN_DO_DAA, PIN_DI_DAA, PIN_LED1);
		printk("=> OK !\n");
	}
	else if (ch_spi == DAA1_SPI_DEV)
	{
		printk("for DAA[%d]...", ch_spi);
		init_channel(ch_spi, PIN_CS3_DAA_2, PIN_RESET3_DAA, PIN_CLK_DAA, PIN_DO_DAA, PIN_DI_DAA_2, PIN_LED1);
		printk("=> OK !\n");
	}
#endif

	if(relay_flag == 0)
		_rtl865x_setGpioDataBit(PIN_RELAY, 1);
	relay_flag = 1;

}

void set_865x_slic_channel(int channel)
{
	cur_channel = channel;
}


