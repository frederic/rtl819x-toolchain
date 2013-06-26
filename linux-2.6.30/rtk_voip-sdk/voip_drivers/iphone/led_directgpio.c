#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/string.h>
#include "led_directgpio.h"
#include "../gpio/gpio.h"
#include "base_gpio.h"
#include <linux/delay.h>	//udelay(),mdelay()

// GPIO PIN definition 
// ------------------------------------------------------ 
#ifdef CONFIG_RTK_VOIP_GPIO_IPP_8952_V00

#define DG_PIN_LED0		GPIO_ID(GPIO_PORT_D,0)	//output
#define DG_PIN_LED1		GPIO_ID(GPIO_PORT_D,1)	//output
#define DG_PIN_LED2		GPIO_ID(GPIO_PORT_D,2)	//output
#define DG_PIN_LED3		GPIO_ID(GPIO_PORT_D,3)	//output
#define DG_PIN_LED4		GPIO_ID(GPIO_PORT_D,4)	//output
#define DG_PIN_LED5		GPIO_ID(GPIO_PORT_D,5)	//output
#define DG_PIN_LED6		GPIO_ID(GPIO_PORT_D,6)	//output
#define DG_PIN_LED7		GPIO_ID(GPIO_PORT_D,7)	//output

#define DG_PIN_LED8		GPIO_ID(GPIO_PORT_C,6)	//output
#define DG_PIN_LED9		GPIO_ID(GPIO_PORT_C,7)	//output

#define DG_PIN_LED10	GPIO_ID(GPIO_PORT_E,0)	//output

#define DG_PIN_LED11	GPIO_ID(GPIO_PORT_E,2)	//output
#define DG_PIN_LED12	GPIO_ID(GPIO_PORT_E,3)	//output
#define DG_PIN_LED13	GPIO_ID(GPIO_PORT_E,4)	//output
#define DG_PIN_LED14	GPIO_ID(GPIO_PORT_E,5)	//output
#define DG_PIN_LED15	GPIO_ID(GPIO_PORT_E,6)	//output
#define DG_PIN_LED16	GPIO_ID(GPIO_PORT_E,7)	//output

#define DG_PIN_LED17	GPIO_ID(GPIO_PORT_F,3)	//output

#define DG_PIN_LED18	GPIO_ID(GPIO_PORT_F,6)	//output

#define DG_PIN_LED19	GPIO_ID(GPIO_PORT_G,4)	//output
#define DG_PIN_LED20	GPIO_ID(GPIO_PORT_G,5)	//output

// ------------------------------------------------------ 
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8972B_V00 )

#define DG_PIN_LED_HF		GPIO_ID(GPIO_PORT_F,3)	//output
#define DG_PIN_LED_VOIP1	GPIO_ID(GPIO_PORT_D,1)	//output

#endif

// other definition 
//#define LED_POLARITY_ANODE

// variable 
static const uint32 led_gpio_map[] = {
#ifdef CONFIG_RTK_VOIP_GPIO_IPP_8952_V00
	DG_PIN_LED0,
	DG_PIN_LED1, 
	DG_PIN_LED2, 
	DG_PIN_LED3, 
	DG_PIN_LED4, 
	DG_PIN_LED5, 
	DG_PIN_LED6, 
	DG_PIN_LED7, 
	DG_PIN_LED8, 
	DG_PIN_LED9, 
	DG_PIN_LED10, 
	DG_PIN_LED11, 
	DG_PIN_LED12, 
	DG_PIN_LED13, 
	DG_PIN_LED14, 
	DG_PIN_LED15, 
	DG_PIN_LED16, 
	DG_PIN_LED17, 
	DG_PIN_LED18, 
	DG_PIN_LED19, 
	DG_PIN_LED20, 
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8972B_V00 )
	DG_PIN_LED_HF,
	DG_PIN_LED_VOIP1,
#endif
};

#define LED_MAP_SIZE	( sizeof( led_gpio_map ) / sizeof( led_gpio_map[ 0 ] ) )

// variables 
static unsigned long old_led = 0x00000000L;

// internal functions 
static void LED_DirectGPIO_init_gpio( void )
{
	int i;
	
	for( i = 0; i < LED_MAP_SIZE; i ++ ) {
		__led_initGpioPin( led_gpio_map[ i ], GPIO_DIR_OUT, GPIO_INT_DISABLE );
#ifdef LED_POLARITY_ANODE
		__led_setGpioDataBit( led_gpio_map[ i ], 0 );
#else
		__led_setGpioDataBit( led_gpio_map[ i ], 1 );
#endif
	}
}

// API 
void LED_DirectGPIO_SetDisplay( unsigned long led )
{
	unsigned long bit = 0x00000001L;
	int i;
	
	for( i = 0; i < LED_MAP_SIZE; i ++ ) {
		if( ( led & bit ) != ( old_led & bit ) ) {
			// update different part only 
#ifdef LED_POLARITY_ANODE
			__led_setGpioDataBit( led_gpio_map[ i ], ( ( led & bit ) ? 1 : 0 ) );
#else
			__led_setGpioDataBit( led_gpio_map[ i ], ( ( led & bit ) ? 0 : 1 ) );
#endif
		}
		
		bit <<= 1;
	}
	
	// save LED display status 
	old_led = led;
}

void LED_DirectGPIO_Initialize( void )
{
	LED_DirectGPIO_init_gpio();
	
#if 0
	while( 1 ) {
		LED_DirectGPIO_SetDisplay( 0xFFFFFFFF );
		mdelay( 1000 );
		
		LED_DirectGPIO_SetDisplay( 0x0 );
		mdelay( 1000 );		
	}
#endif
}

