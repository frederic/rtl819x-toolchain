#include <linux/config.h>
#include <linux/kernel.h>
#include "rtk_voip.h"
#include "voip_init.h"

#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8186) || defined(CONFIG_RTK_VOIP_DRIVERS_PCM8671)
  #include "lcm_char16x2.h"
  #if defined(CONFIG_RTK_VOIP_DRIVERS_CODEC_WM8510)
    #include "WM8510.h"
  #elif defined(CONFIG_RTK_VOIP_DRIVERS_CODEC_ALC5621)
    #include "ALC5621.h"
  #endif
#else
  #include "base_gpio.h"
  #if defined( CONFIG_RTK_VOIP_GPIO_IPP_100 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_101 )
	#include "lcm_char16x2.h"
  #elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V00 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V01 )
	#include "lcm_ht1650.h"
  #elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8952_V00 )
	#include "lcm_epl65132.h"
	#include "led_directgpio.h"
	#include "keyscan_matrix.h"
  #elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8972B_V00 )
	#include "lcm_splc780d.h"
	#include "led_directgpio.h"
	#include "keyscan_matrix.h"
  #endif
  #if defined(CONFIG_RTK_VOIP_DRIVERS_CODEC_WM8510)
    #include "WM8510.h"
  #elif defined(CONFIG_RTK_VOIP_DRIVERS_CODEC_ALC5621)
    #include "ALC5621.h"
  #endif
#endif

static int __init ipphone_init( void /*int law*/ )
{	
	// check redundant initialization (pcm and iis)
	static int ipphone_init_done = 0;
	
	if( ipphone_init_done )
		return;
		
	ipphone_init_done = 1;
	
	//init LCD module
#if defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V00 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V01 ) 
	ht1650_LCM_init();	/* Initialize LCD first due to share PIN. */
	pt6961_Initialize();
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8952_V00 )
	epl65132_LCM_init();
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_100 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_101 )
	LCM_Software_Init();
	Write_string_to_LCM("Startup...",0,0,1);
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8972B_V00 )
	splc780d_InitLCM();
#endif
	
	//init LED, keypad 	
	extern void init_ipphone_hook_detect( void );

#ifdef CONFIG_RTK_VOIP_GPIO_IPP_8952_V00
	LED_DirectGPIO_Initialize();
	keyscan_matrix_init();
	init_ipphone_hook_detect();
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_100 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_101 )
	init_led_74hc164_gpio();
	tri_keypad_polling();
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8972B_V00 )
	LED_DirectGPIO_Initialize();
	init_ipphone_hook_detect();	
	keyscan_matrix_init();
#endif
}

voip_initcall( ipphone_init );

