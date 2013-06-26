#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <net/sock.h>

#include "rtk_voip.h"
#include "voip_feature.h"
#include "voip_types.h"
#include "voip_control.h"
#include "voip_params.h"
#include "voip_mgr_define.h"
#include "voip_mgr_netfilter.h"
#ifndef CONFIG_DEFAULTS_KERNEL_2_6
#include "voip_mgr_do_misc.h"
#endif

#ifdef VOIP_RESOURCE_CHECK
#include "voip_resource_check.h"
#endif

#ifdef CONFIG_RTK_VOIP_LED
extern volatile unsigned int sip_registed[CON_CH_NUM];//={0};     //register ok:1, no register:0
#endif

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
#include "con_mux.h"
#endif

#ifdef CONFIG_RTK_VOIP_IPC_ARCH
#include "ipc_arch_tx.h"
#endif

#ifdef CONFIG_RTK_VOIP_DEFER_SNDDEV_INIT
#include "con_defer_init.h"
#endif

/**
 * @ingroup VOIP_MISC
 * @brief SIP tell kernel register status, so LED can be its indicator 
 * @param TstVoipCfg.ch_id Channel ID 
 * @param TstVoipCfg.enable SIP is registered (1) or unregistered (0) 
 * @see VOIP_MGR_SIP_REGISTER TstVoipCfg 
 */
int do_mgr_VOIP_MGR_SIP_REGISTER( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	extern void led_state_watcher(unsigned int chid);
	TstVoipCfg stVoipCfg;
	int ret;
	
	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward 
#else

#ifdef CONFIG_RTK_VOIP_LED
	if ( sip_registed[stVoipCfg.ch_id] != stVoipCfg.enable )
	{
		sip_registed[stVoipCfg.ch_id] = stVoipCfg.enable;
		led_state_watcher(stVoipCfg.ch_id);
	}
#endif

#if 0
	save_flags(flags); cli();
	if (LED.device == 0)
		FXS_LED_STATE(LED.state);
	else if (LED.device == 1)
		fxo_led_state(LED.state);
	else if (LED.device == 2)
		SIP_LED_STATE(LED.state);
	else if (LED.device == 3)
		FXS_ONE_LED_STATE(LED.state);
	restore_flags(flags);
#endif
#endif
	return 0;
}

/**
 * @ingroup VOIP_MISC
 * @brief Provide DSP's capability and features 
 * @param [out] TstVoipCfg.cfg VoIP feature (RTK_VOIP_FEATURE) 
 * @see VOIP_MGR_GET_FEATURE TstVoipCfg 
 */
int do_mgr_VOIP_MGR_GET_FEATURE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	//unsigned long flags;
	TstVoipFeature stVoipFeature;

	// assert these features 
	CT_ASSERT( sizeof( gVoipFeature ) == 8 );	// make sure 8 bytes 
	CT_ASSERT( sizeof( TstVoipFeature ) == 8 );	// make sure 8 bytes 	

	COPY_FROM_USER(&stVoipFeature, (TstVoipFeature *)user, sizeof(TstVoipFeature));

	//save_flags(flags); cli();	// pkshih: why need cli?? 
	memcpy( &stVoipFeature, &gVoipFeature, sizeof( TstVoipFeature ) );
	//gVoipFeature = RTK_VOIP_FEATURE; //move to rtk_voip_mgr_init_module
	//restore_flags(flags);
	//PRINT_MSG("== VoIP Feature: 0x%llx ==\n", *( ( uint64 * )&gVoipFeature ));
	return COPY_TO_USER(user, &stVoipFeature, sizeof(TstVoipFeature), cmd, seq_no);
	//return 0;
}

/**
 * @ingroup VOIP_MISC
 * @brief Check whether CPU resource is enough 
 * @param TstVoipCfg.cfg Packet format (payload type) 
 * @param TstVoipCfg.enable Resource is available or not (@ref Voip_reosurce_t) 
 * @see VOIP_MGR_VOIP_RESOURCE_CHECK TstVoipCfg 
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_VOIP_RESOURCE_CHECK( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCfg stVoipCfg;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward 
	//stVoipCfg.enable = 1;
#else

#if ! defined (VOIP_RESOURCE_CHECK)
	stVoipCfg.enable = VOIP_RESOURCE_AVAILABLE;
#else
	stVoipCfg.enable = GetCurrentVoipResourceStatus(stVoipCfg.cfg);
#endif
#endif
	return COPY_TO_USER(user, &stVoipCfg, sizeof(TstVoipCfg), cmd, seq_no);
	//return 0;
}
#else
int do_mgr_VOIP_MGR_VOIP_RESOURCE_CHECK( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCfg stVoipCfg;
	
	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
	
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward 
#else

#if ! defined (VOIP_RESOURCE_CHECK)
	stVoipCfg.enable = VOIP_RESOURCE_AVAILABLE;
#else
  #if 0
	int chid;
	stVoipCfg.enable = VOIP_RESOURCE_UNAVAILABLE;

	for (chid=0; chid < ACMW_MAX_NUM_CH; chid++)
	{
		if (CHANNEL_STATE__IDLE == RtkAc49xGetChannelState(chid))
		{
			stVoipCfg.enable = VOIP_RESOURCE_AVAILABLE;
		}
	}
  #endif
	stVoipCfg.enable = VOIP_RESOURCE_AVAILABLE;
#endif
#endif
	return COPY_TO_USER(user, &stVoipCfg, sizeof(TstVoipCfg), cmd, seq_no);
	//return 0;
}
#endif

/**
 * @ingroup VOIP_MISC
 * @brief Announce to start firmware upgrade, and kernel will reboot in a certain period 
 * @param Reboot_Wait An 'int' variable. Reboot wait time. 
 * @see VOIP_MGR_SET_FW_UPDATE 
 */
int do_mgr_VOIP_MGR_SET_FW_UPDATE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	//add by timlee 20081118
#if !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC) && !defined (CONFIG_RTK_VOIP_DRIVERS_PCM8672) && !defined (CONFIG_RTK_VOIP_DRIVERS_PCM8676) &&  !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxD)
	int Reboot_Wait;
	int ret;

	extern void VOIP_FWUPDATE_REBOOT(int reboot);	
	COPY_FROM_USER(&Reboot_Wait, (int*)user, sizeof(int));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

	VOIP_FWUPDATE_REBOOT(Reboot_Wait);
	//printk("Reboot_Wait: %d \n",Reboot_Wait);
#endif
	return 0;
}


/**
 * @ingroup VOIP_ETHERNET_DSP
 * @brief Tell DSP its CPUID in host via MDC/IO by Host
 * @param stVoipValue.value DSP's CPU ID 
 * @see VOIP_MGR_SET_DSP_ID_TO_DSP 
 */
int do_mgr_VOIP_MGR_SET_DSP_ID_TO_DSP( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP
	extern void SetCurrentPhyId(unsigned char phy_id);
	extern unsigned char SetDspIdToDsp(int dsp_id);

	TstVoipValue stVoipValue;
	int ret;
	
	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

	SetCurrentPhyId(stVoipValue.value);
	SetDspIdToDsp(stVoipValue.value);
#endif
#endif
	return 0;
}

/**
 * @ingroup VOIP_ETHERNET_DSP
 * @brief Set DSP's PHY ID used by MDC/IO 
 * @param stVoipValue.value DSP CPUID (Valid values are 0/1 and corresponding to PHY ID 8/16 respectively.)
 * @see VOIP_MGR_SET_DSP_PHY_ID 
 */
int do_mgr_VOIP_MGR_SET_DSP_PHY_ID( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP
	extern void SetCurrentPhyId(unsigned char phy_id);
	
	TstVoipValue stVoipValue;
	int ret;
	
	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

	//PRINT_MSG("VOIP_MGR_SET_DSP_PHY_ID, Phy ID=%d\n", stVoipValue.value);
	SetCurrentPhyId(stVoipValue.value);
#endif
#endif
	return 0;
}

/**
 * @ingroup VOIP_ETHERNET_DSP
 * @brief Check whether DSP is ready 
 * @param stVoipValue.value DSP CPUID 
 * @see VOIP_MGR_CHECK_DSP_ALL_SW_READY 
 */
int do_mgr_VOIP_MGR_CHECK_DSP_ALL_SW_READY( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP
	extern void SetCurrentPhyId(unsigned char phy_id);
	extern unsigned short CheckDspIfAllSoftwareReady(void);
	
	TstVoipValue stVoipValue;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
	//PRINT_MSG("VOIP_MGR_CHECK_DSP_ALL_SW_READY, Phy ID=%d\n", stVoipValue.value);
	SetCurrentPhyId(stVoipValue.value);
	stVoipValue.value1 = CheckDspIfAllSoftwareReady();
	ret = COPY_TO_USER(user, &stVoipValue, sizeof(TstVoipValue), cmd, seq_no);	
	return ret;
#endif
#endif		
	return 0;
}

/**
 * @ingroup VOIP_ETHERNET_DSP
 * @brief Complete defer initialization. 
 *        In case of ethernet DSP architecture, and host side controls SLIC/DAA and etc. 
 *        DSP side owns PCM controller, but normally host try to bring up SLIC/DAA ago. 
 *        We try to defer host's some initialization, so now we should complete them. 
 * @see VOIP_MGR_COMPLETE_DEFER_INIT 
 */
int do_mgr_VOIP_MGR_COMPLETE_DEFER_INIT( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef CONFIG_RTK_VOIP_DEFER_SNDDEV_INIT
	//printk( "do_mgr_VOIP_MGR_COMPLETE_DEFER_INIT ok!!\n" );
	complete_defer_initialization();
#endif
	return 0;
}

