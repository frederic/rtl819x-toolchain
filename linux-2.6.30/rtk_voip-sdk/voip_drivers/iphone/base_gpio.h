#ifndef __BASE_GPIO_H__

#if defined( CONFIG_RTK_VOIP_GPIO_IPP_100 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_101 )
#include "base_gpio_8186.h"
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V00 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V01 )
#include "base_gpio_8972.h"
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8952_V00 )
#include "base_gpio_8952.h"
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8972B_V00 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_8972B_V99 )
#include "base_gpio_8972b.h"
#else
  #error "include which one?"
#endif

//
// common interfaces for IP phone hook 
//
// turn on / off interrupt 
extern void ipphone_hook_interrupt( int enable );

// initial hook GPIO 
extern void init_ipphone_hook_detect( void );

// get hook status form GPIO or other 
extern unsigned char iphone_hook_detect( void );	//return value 0->on-hook, 1->off-hook

#endif /* __BASE_GPIO_H__ */

