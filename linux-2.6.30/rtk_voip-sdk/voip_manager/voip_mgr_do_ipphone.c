#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <net/sock.h>

#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_control.h"
#include "voip_params.h"
#include "voip_mgr_define.h"
#include "voip_mgr_netfilter.h"
#ifndef CONFIG_DEFAULTS_KERNEL_2_6
#include "voip_mgr_do_ipphone.h"
#endif

#ifdef CONFIG_RTK_VOIP_IP_PHONE
  #if defined(CONFIG_RTK_VOIP_DRIVERS_CODEC_WM8510)
    #include "../voip_drivers/iphone/WM8510.h"	/* for type of AI_AO_select() */
  #elif defined(CONFIG_RTK_VOIP_DRIVERS_CODEC_ALC5621)
    #include "../voip_drivers/iphone/ALC5621.h"
  #endif
  #if defined( CONFIG_RTK_VOIP_GPIO_IPP_100 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_101 )
  #elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V00 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V01 ) 
    #include "../voip_drivers/iphone/PT6961.h"
  #elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8952_V00 )
    #include "../voip_drivers/iphone/led_directgpio.h"
  #endif
#endif

/**
 * @ingroup VOIP_IPPHONE
 * @brief Initialize and retrieve keypad information 
 * @note This interface contains many functions. 
 * @see VOIP_MGR_CTL_KEYPAD TstKeypadCtl 
 */
int do_mgr_VOIP_MGR_CTL_KEYPAD( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef CONFIG_RTK_VOIP_IP_PHONE
 #ifndef CONFIG_RTK_VOIP_GPIO_IPP_8972B_V99
	extern void do_keypad_opt_ctl( void *pUser, unsigned int len );
	do_keypad_opt_ctl( user, len );
 #endif
#endif
	return 0;
}

/**
 * @ingroup VOIP_IPPHONE
 * @brief Initialize and control LCM display 
 * @note This interface contains many functions. 
 * @see VOIP_MGR_CTL_LCM TstLcmCtl 
 */
int do_mgr_VOIP_MGR_CTL_LCM( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef CONFIG_RTK_VOIP_IP_PHONE
 #ifndef CONFIG_RTK_VOIP_GPIO_IPP_8972B_V99
	extern void do_lcm_opt_ctl( void *pUser, unsigned int len );
	do_lcm_opt_ctl( user, len );
 #endif
#endif
	return 0;
}

/**
 * @ingroup VOIP_IPPHONE
 * @brief Control voice path 
 * @see VOIP_MGR_CTL_VOICE_PATH TstVoicePath_t 
 */
int do_mgr_VOIP_MGR_CTL_VOICE_PATH( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef CONFIG_RTK_VOIP_IP_PHONE
	TstVoicePath_t stVoicePath;
	extern void AI_AO_select(unsigned char type);
	static const unsigned char select_type[] =
		{ MIC1_SPEAKER, MIC2_MONO, SPEAKER_ONLY, MONO_ONLY };
	int ret;
	
	COPY_FROM_USER(&stVoicePath, (TstVoicePath_t *)user, sizeof(TstVoicePath_t));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

	AI_AO_select( select_type[ stVoicePath.vpath ] );
#endif
	return 0;
}

/**
 * @ingroup VOIP_IPPHONE
 * @brief Control LED display 
 * @see VOIP_MGR_CTL_LED TstLedCtl 
 */
int do_mgr_VOIP_MGR_CTL_LED( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef CONFIG_RTK_VOIP_IP_PHONE
	TstLedCtl stLedCtl;
	int ret;
	
	COPY_FROM_USER(&stLedCtl, user, sizeof(TstLedCtl));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#if defined( CONFIG_RTK_VOIP_GPIO_IPP_100 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_101 )
	led_shower( ( unsigned short )stLedCtl.led );
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V00 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V01 ) 
	pt6961_SetDisplay( stLedCtl.led );
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8952_V00 )
	LED_DirectGPIO_SetDisplay( stLedCtl.led );
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8972B_V00 )
	LED_DirectGPIO_SetDisplay( stLedCtl.led );
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8972B_V99 )

#else
	???
#endif		

#endif
	return 0;
}

/**
 * @ingroup VOIP_IPPHONE
 * @brief Retrieve build number and date 
 * @see VOIP_MGR_CTL_MISC TstMiscCtl_t 
 */
int do_mgr_VOIP_MGR_CTL_MISC( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef CONFIG_RTK_VOIP_IP_PHONE
	TstMiscCtl_t stMiscCtl;
	
	//copy_from_user(&stMiscCtl, user, sizeof(TstMiscCtl_t));
	extern const unsigned long buildno;
	extern const unsigned long builddate;

	stMiscCtl.buildno = buildno;
	stMiscCtl.builddate = builddate;
	
	return COPY_TO_USER(user, &stMiscCtl, sizeof(TstMiscCtl_t), cmd, seq_no);
#endif /* CONFIG_RTK_VOIP_IP_PHONE */
	return 0;
}


