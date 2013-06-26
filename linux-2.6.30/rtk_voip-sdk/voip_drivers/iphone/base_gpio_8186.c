#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/string.h>

#include "base_gpio_8186.h"


#if defined(CONFIG_RTK_VOIP_GPIO_IPP_100)
#define IPONE_HOOK_GPIOD_DIR *((volatile unsigned int *)0xbd010134)
#define IPONE_HOOK_GPIOD_DATA *((volatile unsigned int *)0xbd010130)
#elif defined(CONFIG_RTK_VOIP_GPIO_IPP_101)
#define IPONE_HOOK_GPIOA_DIR *((volatile unsigned int *)0xbd010124)
#define IPONE_HOOK_GPIOA_DATA *((volatile unsigned int *)0xbd010120)
#endif
static void init_iphone_hook_gpio()
{
#if defined(CONFIG_RTK_VOIP_GPIO_IPP_100)
	//GPIOD bit 5 for iphone hook detection
	IPONE_HOOK_GPIOD_DIR &= 0xffdf;
#elif defined(CONFIG_RTK_VOIP_GPIO_IPP_101)
	//GPIOA bit 6 for iphone hook detection
	IPONE_HOOK_GPIOA_DIR &= 0xffbf;
#endif	
	return;
}

//return value 0->on-hook, 1->off-hook
unsigned char iphone_hook_detect()
{
	static unsigned char i = 0;
	if (!i) {
		init_iphone_hook_gpio();
		i = 1;
	}	
#if defined(CONFIG_RTK_VOIP_GPIO_IPP_100)
	return !((IPONE_HOOK_GPIOD_DATA>>21)&0x01);
#elif defined(CONFIG_RTK_VOIP_GPIO_IPP_101)
	return !((IPONE_HOOK_GPIOA_DATA>>6)&0x01);
#endif	
}
