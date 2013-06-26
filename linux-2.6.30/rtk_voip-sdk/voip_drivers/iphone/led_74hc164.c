#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/sched.h>
#include "../gpio/gpio.h"
#include "base_gpio.h"
#include "led_74hc164.h"

//-----------------------------
#if 1	//0: kernel used(udelay), 1: test program used(for(;;)) 
#define __test_program_used__
#else
#define __kernel_used__
#endif

//------------------------- LED_74HC164 -----------------------------------------
#define _LED_74HC164_	//GPIO pin
#define LED_74HC164_RATING_FACTOR	2	//scanning frequency
#define LED_CLK_FACTOR		1	//clk period
#define LED_MAX_POSITIVE_CLK		10	//max positive clk
#define LED_NUMBER		16	//amount of led
#define LED_POLARITY	 1	//1->led common-anode, 0->led common-cathode
//#include <linux/init.h>
//#include <linux/sched.h>
//-------------------------------------------------------------------------------


/************************ LED_74HC164 data struct *************************/
struct rtl8186_led_74hc164_dev_s
{
	unsigned int a;		//output 
	unsigned int clk;		//output
};

typedef struct rtl8186_led_74hc164_dev_s rtl8186_led_74hc164_dev_t;
/**************************************************************************/

#ifdef _LED_74HC164_
#if defined(CONFIG_RTK_VOIP_GPIO_IPP_100)
#define GPIO_LED_74HC164 "D"
#define LED_74HC164_A			GPIO_ID(GPIO_PORT_D,8)	//output
#define LED_74HC164_CLK			GPIO_ID(GPIO_PORT_D,7)	//output
#elif defined(CONFIG_RTK_VOIP_GPIO_IPP_101)
#define GPIO_LED_74HC164 "A"
#define LED_74HC164_A			GPIO_ID(GPIO_PORT_A,3)	//output
#define LED_74HC164_CLK			GPIO_ID(GPIO_PORT_A,2)	//output
#endif
#endif

/********************* LED_74HC164 data struct******************/
static rtl8186_led_74hc164_dev_t led_74hc164;
//struct timer_list led_74hc164_polling_timer;
//------------- led api-----------------------------------------//

void init_led_74hc164_gpio(void)
{
	printk("( GPIO %s )  ", GPIO_LED_74HC164);
	
	led_74hc164.a = LED_74HC164_A;
	led_74hc164.clk = LED_74HC164_CLK;
	//set led_74hc164.a and led_74hc164.clk output
	_rtl8186_initGpioPin(led_74hc164.a, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(led_74hc164.clk, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	
#if LED_POLARITY
	_rtl8186_setGpioDataBit(led_74hc164.a,1); //common-anode
#else
	_rtl8186_setGpioDataBit(led_74hc164.a,0); //common-cathode
#endif	
	_rtl8186_setGpioDataBit(led_74hc164.clk,0);
	
	printk("For LED port init=> OK !\n");
	return;
}

//led_on(16bit):1->on, 0->off. bit15:LED16, bit14:LED15, .....bit1:LED2, bit0:LED1
void led_shower(unsigned short led_on)
{
	unsigned char i, j, on_temp;
	unsigned int count = 0;
	unsigned long flags;
	save_flags(flags); cli();

	
#if LED_POLARITY
	led_on = ~led_on;
#endif	
	
	for (j=0; j<LED_NUMBER; j++) {
		_rtl8186_setGpioDataBit(led_74hc164.clk,0); //pull clk down
			
		if (j < LED_NUMBER)	
#if LED_POLARITY				
		_rtl8186_setGpioDataBit(led_74hc164.a,led_on>>(LED_NUMBER-j-1)); //pull a down, common-anode
#else
		_rtl8186_setGpioDataBit(led_74hc164.a,led_on>>(LED_NUMBER-j-1)); //pull a high, common-cathode
#endif							
		for (count=0; count<LED_CLK_FACTOR; count++);
		_rtl8186_setGpioDataBit(led_74hc164.clk,1); //pull clk high
			
			
#ifdef __kernel_used__				
		udelay(LED_MAX_POSITIVE_CLK);
#endif
#ifdef __test_program_used__				
		for (count=0; count<LED_MAX_POSITIVE_CLK; count++);
#endif			
		
	}


#if 0
	if (led_on == 0) {
		for (j=0; j<=LED_NUMBER; j++) {
			_rtl8186_setGpioDataBit(led_74hc164.clk,0); //pull clk down
			
			if (j == 0)
#if LED_POLARITY				
				_rtl8186_setGpioDataBit(led_74hc164.a,1); //pull a high, common-anode
#else
				_rtl8186_setGpioDataBit(led_74hc164.a,0); //pull a down, common-cathode
#endif							
			_rtl8186_setGpioDataBit(led_74hc164.clk,1); //pull clk high
		}
	}
	
	
	for (i=0; i<LED_NUMBER; i++) {
		
		on_temp = ((led_on>>i) & 0x01)? i:0xff;
		if (on_temp == 0xff)
			continue;
				
		for (j=0; j<=LED_NUMBER; j++) {
			_rtl8186_setGpioDataBit(led_74hc164.clk,0); //pull clk down
			
			if (j == 0)
#if LED_POLARITY				
				_rtl8186_setGpioDataBit(led_74hc164.a,0); //pull a down, common-anode
#else
				_rtl8186_setGpioDataBit(led_74hc164.a,1); //pull a high, common-cathode
#endif							
			for (count=0; count<LED_CLK_FACTOR; count++);
			_rtl8186_setGpioDataBit(led_74hc164.clk,1); //pull clk high
			
			if (j == on_temp)
#ifdef __kernel_used__				
				udelay(LED_MAX_POSITIVE_CLK*10);
#endif
#ifdef __test_program_used__				
				for (count=0; count<LED_MAX_POSITIVE_CLK*10000; count++);
#endif			
			else	
				for (count=0; count<LED_CLK_FACTOR; count++);	
	
			if (j == 0)
#if LED_POLARITY			 				
				_rtl8186_setGpioDataBit(led_74hc164.a,1); //pull a high, common-anode
#else
				_rtl8186_setGpioDataBit(led_74hc164.a,0); //pull a down, common-cathode
#endif							
		}	
	
	}
#endif
	
#if LED_POLARITY			 				
	_rtl8186_setGpioDataBit(led_74hc164.a,1); //pull a high, common-anode
#else
	_rtl8186_setGpioDataBit(led_74hc164.a,0); //pull a down, common-cathode
#endif
	_rtl8186_setGpioDataBit(led_74hc164.clk,0);
	
#if 0
	led_74hc164_polling_timer.expires = jiffies + LED_74HC164_RATING_FACTOR;
	led_74hc164_polling_timer.function = led_shower;
	add_timer(&led_74hc164_polling_timer);
#endif	
	restore_flags(flags);
	return;
}
#if 0
void led_74hc164_polling()
{
	init_led_74hc164_gpio();
	led_74hc164_display(0);
	init_timer(&led_74hc164_polling_timer);
	led_74hc164_polling_timer.expires = jiffies + LED_74HC164_RATING_FACTOR;
	led_74hc164_polling_timer.function = led_shower;
	add_timer(&led_74hc164_polling_timer);
	return;
}
module_init(led_74hc164_polling);
#endif



