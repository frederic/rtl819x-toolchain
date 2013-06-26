#ifndef __BASE_GPIO_8972B_H__
#define __BASE_GPIO_8972B_H__

#include "../gpio/gpio.h"

//======================= I2C ============================================
//----------------------- GPIO pin assignment ----------------------------------------------
#if defined(CONFIG_RTK_VOIP_DRIVERS_CODEC_ALC5621)
#define I2C_SCLK_PIN		GPIO_ID(GPIO_PORT_F,0)	//output
#define I2C_SDIO_PIN		GPIO_ID(GPIO_PORT_F,1)	//output or input
#endif

//----------------------- GPIO API mapping ----------------------------------------------
#define __i2c_initGpioPin( id, dir, intt ) 	\
			_rtl8972B_initGpioPin( id, GPIO_CONT_GPIO, dir, intt )
#define __i2c_getGpioDataBit( id, pd )		\
			_rtl8972B_getGpioDataBit( id, pd )
#define __i2c_setGpioDataBit( id, d )		\
			_rtl8972B_setGpioDataBit( id, d )

//======================= LCM ============================================
//----------------------- GPIO API mapping ----------------------------------------------
#define __lcm_initGpioPin( id, dir, intt ) 	\
			_rtl8972B_initGpioPin( id, GPIO_CONT_GPIO, dir, intt )
#define __lcm_getGpioDataBit( id, pd )		\
			_rtl8972B_getGpioDataBit( id, pd )
#define __lcm_setGpioDataBit( id, d )		\
			_rtl8972B_setGpioDataBit( id, d )

//======================= LED ============================================
//----------------------- GPIO API mapping ----------------------------------------------
#define __led_initGpioPin( id, dir, intt ) 	\
			_rtl8972B_initGpioPin( id, GPIO_CONT_GPIO, dir, intt )
#define __led_getGpioDataBit( id, pd )		\
			_rtl8972B_getGpioDataBit( id, pd )
#define __led_setGpioDataBit( id, d )		\
			_rtl8972B_setGpioDataBit( id, d )

//======================= Keypad ============================================
//----------------------- GPIO API mapping ----------------------------------------------
#define __key_initGpioPin( id, dir, intt ) 	\
			_rtl8972B_initGpioPin( id, GPIO_CONT_GPIO, dir, intt )
#define __key_getGpioDataBit( id, pd )		\
			_rtl8972B_getGpioDataBit( id, pd )
#define __key_setGpioDataBit( id, d )		\
			_rtl8972B_setGpioDataBit( id, d )

#endif /* __BASE_GPIO_8972B_H__ */

