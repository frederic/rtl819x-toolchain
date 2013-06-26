#ifndef _BASE_GPIO_8186_H_
#define _BASE_GPIO_8186_H_

#include "../gpio/gpio.h"

extern unsigned char iphone_hook_detect();

//======================= I2C ============================================
//----------------------- GPIO pin assignment ----------------------------------------------
#ifdef CONFIG_RTK_VOIP_DRIVERS_CODEC_WM8510
#if defined(CONFIG_RTK_VOIP_GPIO_IPP_100)
#define GPIO_I2C "D"
#define SCLK_PIN	GPIO_ID(GPIO_PORT_D,10)	//output
#define SDIO_PIN	GPIO_ID(GPIO_PORT_D,6)	//output or input
#elif defined(CONFIG_RTK_VOIP_GPIO_IPP_101)
#define GPIO_I2C "C"
#define SCLK_PIN	GPIO_ID(GPIO_PORT_C,13)	//output
#define SDIO_PIN	GPIO_ID(GPIO_PORT_C,14)	//output or input
#endif
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_CODEC_ALC5621
#if defined(CONFIG_RTK_VOIP_GPIO_IPP_100)
#define GPIO_I2C "D"
#define SCLK_PIN	GPIO_ID(GPIO_PORT_D,10)	//output
#define SDIO_PIN	GPIO_ID(GPIO_PORT_D,6)	//output or input
#elif defined(CONFIG_RTK_VOIP_GPIO_IPP_101)
#define GPIO_I2C "C"
#define SCLK_PIN	GPIO_ID(GPIO_PORT_C,13)	//output
#define SDIO_PIN	GPIO_ID(GPIO_PORT_C,14)	//output or input
#endif
#endif

//----------------------- GPIO API mapping ----------------------------------------------
#define __i2c_initGpioPin( id, dir, intt ) 	\
			_rtl8186_initGpioPin( id, dir, intt )
#define __i2c_getGpioDataBit( id, pd )		\
			_rtl8186_getGpioDataBit( id, pd )
#define __i2c_setGpioDataBit( id, d )		\
			_rtl8186_setGpioDataBit( id, d )

/*************** Define RTL8186 GPIO Register Set ************************/

//======================= LCM ============================================
//----------------------- GPIO API mapping ----------------------------------------------
#define __lcm_initGpioPin( id, dir, intt ) 	\
			_rtl8186_initGpioPin( id, dir, intt )
#define __lcm_getGpioDataBit( id, pd )		\
			_rtl8186_getGpioDataBit( id, pd )
#define __lcm_setGpioDataBit( id, d )		\
			_rtl8186_setGpioDataBit( id, d )

//======================= LED ============================================
//----------------------- GPIO API mapping ----------------------------------------------
#define __led_initGpioPin( id, dir, intt ) 	\
			_rtl8186_initGpioPin( id, dir, intt )
#define __led_getGpioDataBit( id, pd )		\
			_rtl8186_getGpioDataBit( id, pd )
#define __led_setGpioDataBit( id, d )		\
			_rtl8186_setGpioDataBit( id, d )

//======================= Keypad ============================================
//----------------------- GPIO API mapping ----------------------------------------------
#define __key_initGpioPin( id, dir, intt ) 	\
			_rtl8186_initGpioPin( id, dir, intt )
#define __key_getGpioDataBit( id, pd )		\
			_rtl8186_getGpioDataBit( id, pd )
#define __key_setGpioDataBit( id, d )		\
			_rtl8186_setGpioDataBit( id, d )


#endif	//_BASE_GPIO_8186_H_
