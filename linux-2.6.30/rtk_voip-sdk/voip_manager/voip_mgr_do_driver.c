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
#include "voip_mgr_do_driver.h"
#endif
#include "voip_mgr_events.h"

#include "../voip_drivers/gpio/gpio.h"
#ifdef CONFIG_DEFAULTS_KERNEL_2_6
#include <linux/wait.h>
#include <linux/spinlock_types.h>
#endif
//#include "../voip_drivers/pcm_interface.h"
#include "../voip_drivers/dsp_rtk_define.h"
#include "../voip_drivers/led.h"
//#include "../voip_drivers/Daa_api.h"

#ifdef CONFIG_RTK_VOIP_DRIVERS_ATA_DECT
#include "../voip_drivers/dect_common/dect_ctrl.h"
#endif

#ifdef CONFIG_RTK_VOIP_QOS
#include "voip_qos.h"
#endif

#include "con_mux.h"
#include "snd_mux_slic.h"
#include "snd_mux_daa.h"
#include "snd_define.h"
#include "snd_help.h"
#include "dsp_define.h"

#if ! defined (AUDIOCODES_VOIP)

#ifdef FXO_BUSY_TONE_DET
#include "../voip_dsp/include/tone_det_i.h"
#endif
#ifdef FXO_CALLER_ID_DET
#include "../voip_dsp/include/fsk_det.h"
#include "../voip_dsp/include/dtmf_dec.h"
#endif

#else

#include "RTK_AC49xApi_Interface.h"

#endif

#if ! defined (AUDIOCODES_VOIP)

//choice 8305SC or 8306SD
#if 0
#define _8305SC_
#else
#define _8306SD_
#endif

//unsigned char pcm_flag[MAX_VOIP_CH_NUM] = {0};


#ifdef FXO_CALLER_ID_DET
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
int fsk_cid_valid[CON_CH_NUM] = {0};
int dtmf_cid_valid[CON_CH_NUM] = {0};
#endif
#endif

#ifdef SUPPORT_SLIC_GAIN_CFG
extern uint32 g_txVolumneGain[];
extern uint32 g_rxVolumneGain[];
#endif

#else

//choice 8305SC or 8306SD
#if 0
#define _8305SC_
#else
#define _8306SD_
#endif

//static unsigned char pcm_flag[MAX_VOIP_CH_NUM] = {0};

//uint32 g_txVolumneGain[MAX_VOIP_CH_NUM];	// move to dsp_define.c 
//uint32 g_rxVolumneGain[MAX_VOIP_CH_NUM];	// move to dsp_define.c 

#endif /* AUDIOCODES_VOIP */

#ifdef CONFIG_RTK_VOIP_IPC_ARCH
#include "ipc_arch_tx.h"
#endif

/* Default Ring Off cadence 4sec (The same to the SLIC init Ring Cadnece) */
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
int gRingCadOff[CON_CH_NUM] = {[0 ... CON_CH_NUM-1] = 4000 };
unsigned char ioctrl_ring_set[CON_CH_NUM] = {0};	// cch
#endif
extern unsigned int flash_hook_time[];
extern unsigned int flash_hook_min_time[];
static unsigned int external_mode = 0;
extern int hook_out(uint32 ch_id, char * output);
//#ifdef CONFIG_RTK_VOIP_DRIVERS_SI3050
//extern uint32 g_txVolumneGain_DAA;
//extern uint32 g_rxVolumneGain_DAA;
//#endif


#ifdef CONFIG_RTK_VOIP_LED
volatile unsigned int sip_registed[CON_CH_NUM]={0};     //register ok:1, no register:0
volatile unsigned int daa_hook_status[CON_CH_NUM]={0};  //off-hook:1 ,on-hook:0
volatile unsigned int slic_hook_status[CON_CH_NUM]={0}; //off-hook:1 ,on-hook:0 ,flash-hook:2
volatile unsigned int fxs_ringing[CON_CH_NUM]={0};                      //no ring:0 ,voip incoming ring:1
volatile unsigned int daa_ringing=0;
#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA
unsigned int pstn_ringing=0;	//pstn incoming ring:1 ,no ring and voip incoming ring:0
extern char relay_2_PSTN_flag[CON_CH_NUM]; /* 1: relay to PSTN , 0: relay to SLIC */
#elif defined CONFIG_RTK_VOIP_DRIVERS_DAA_SUPPORT
unsigned int pstn_ringing[CON_CH_NUM] = {0};
#endif
#endif

#ifdef SUPPORT_DSCP
int rtp_tos = 0;
int sip_tos = 0;
#endif

#if 0
int sid_owner[MAX_DSP_RTK_SS_NUM]= {0};   /* Session own by 0: SLIC, 1: DAA */
#endif

extern int Is_DAA_Channel(int chid);

#ifdef CONFIG_RTK_VOIP_IP_PHONE
  #if defined(CONFIG_RTK_VOIP_DRIVERS_CODEC_ALC5621)
extern void Stereo_dac_volume_control_MUTE(unsigned short int DACMUTE);
  #endif
#endif

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
//static unsigned int mgr_chid;
#endif

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
#ifdef CONFIG_RTK_VOIP_LED
void led_state_watcher(unsigned int chid)
{
	// FXS
	if (Is_DAA_Channel(chid) == 0)
	{
		if (fxs_ringing[chid] || slic_hook_status[chid])
		{
			fxs_led_state(chid, 2); // blinking
		}
		else
		{
			if (sip_registed[chid])
			{
				fxs_led_state(chid, 1); // on
			}
			else
			{
				fxs_led_state(chid, 0); // off
			}
		}
	}

	// FXO
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA
	if (pstn_ringing || relay_2_PSTN_flag[0] || relay_2_PSTN_flag[1])
#else
	if ( (pstn_ringing[chid] || daa_hook_status[chid]) && (Is_DAA_Channel(chid) == 1))
#endif
	{
		fxo_led_state(chid, 2); // blinking
		//PRINT_MSG("===> daa led blinking...\n");
	}
#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA
	else
#else
	else if (Is_DAA_Channel(chid) == 1)
#endif
	{
#if CON_CH_NUM == 2
		if (sip_registed[0] && sip_registed[1])
		{
			fxo_led_state(chid, 0); // off
			//PRINT_MSG("===> daa led off...\n");
		}
		else
		{
			fxo_led_state(chid, 1); // on
			//PRINT_MSG("===> daa led on...\n");
		}
#elif CON_CH_NUM == 1
		if (sip_registed[chid])
		{
			fxo_led_state(chid, 0); // off
			//PRINT_MSG("===> daa led off...\n");
		}
		else
		{
			fxo_led_state(chid, 1); // on
			//PRINT_MSG("===> daa led on...\n");
		}
#else
		//#error "Please fix this error in voip_mgr_netfilter.c"
#endif
	}
#endif //CONFIG_RTK_VOIP_DRIVERS_FXO
}
#endif
#endif

/**
 * @ingroup VOIP_DRIVER_PCM
 * @brief Enable / Disable PCM controller 
 * @param TstVoipCfg.ch_id Channel ID
 * @param TstVoipCfg.enable Enable (1: narrow band, 2: wideband) or disable (0) PCM controller 
 * @see VOIP_MGR_PCM_CFG TstVoipCfg
 * @note Re-entry will cause system crash!! 
 */
int do_mgr_VOIP_MGR_PCM_CFG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	unsigned long flags;
#endif
	TstVoipCfg stVoipCfg;
	int ret;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
	
	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

	PRINT_MSG("VOIP_MGR_PCM_CFG:ch_id = %d, enable = %d\n", stVoipCfg.ch_id, stVoipCfg.enable);

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward 
#else

	save_flags(flags); cli();
	
	if( con_enable_cch( stVoipCfg.ch_id, stVoipCfg.enable ) < 0 ) {
		PRINT_MSG(" ===> Double PCM Restart(%d)\n", stVoipCfg.ch_id);
		restore_flags(flags);
		copy_to_user(user, &stVoipCfg, sizeof(TstVoipCfg));
		return 0;
	}
	
	restore_flags(flags);

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
	/* pkshih: move them out of cli() */
	PRINT_MSG("stVoipCfg.ch_id=%d\n",stVoipCfg.ch_id); //chiminer modify
	PRINT_MSG("PCMCHCNR=%x\n",PCM_CHCNR);//chiminer modify
#endif
#endif
	
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_PCM
 * @brief Set bus data format 
 * @param TstVoipBusDataFormat.ch_id Channel ID
 * @param TstVoipBusDataFormat.format Data format @ref AP_BUS_DATA_FORMAT
 * @see VOIP_MGR_SET_BUS_DATA_FORMAT TstVoipBusDataFormat
 */
int do_mgr_VOIP_MGR_SET_BUS_DATA_FORMAT( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifndef CONFIG_RTK_VOIP_ETHERNET_DSP_IS_HOST
#define _M_FMT_MAP( x )		{ AP_ ## x, x }
	static const struct {
		AP_BUS_DATA_FORMAT ap_format;
		BUS_DATA_FORMAT format;
	} format_map[] = {
		_M_FMT_MAP( BUSDATFMT_PCM_LINEAR ),
		_M_FMT_MAP( BUSDATFMT_PCM_ALAW ),
		_M_FMT_MAP( BUSDATFMT_PCM_ULAW ),
		_M_FMT_MAP( BUSDATFMT_PCM_WIDEBAND_LINEAR ),
		_M_FMT_MAP( BUSDATFMT_PCM_WIDEBAND_ALAW ),
		_M_FMT_MAP( BUSDATFMT_PCM_WIDEBAND_ULAW ),
	};
#undef _M_FMT_MAP
	
	#define FORMAT_MAP_SIZE		( sizeof( format_map ) / sizeof( format_map[ 0 ] ) )
	int i;
#endif
	TstVoipBusDataFormat stVoipBusDataFormat;
	int ret;

	COPY_FROM_USER(&stVoipBusDataFormat, (TstVoipBusDataFormat *)user, sizeof(TstVoipBusDataFormat));
	
	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

	PRINT_MSG("VOIP_MGR_SET_BUS_DATA_FORMAT:ch_id = %d, format = %d\n", 
					stVoipBusDataFormat.ch_id, stVoipBusDataFormat.format);

#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP_IS_HOST
	// Host auto forward 
#else
	
	for( i = 0; i < FORMAT_MAP_SIZE; i ++ ) { 
		if( format_map[ i ].ap_format != stVoipBusDataFormat.format )
			continue;
			
		Set_Bus_Data_Format_cch( stVoipBusDataFormat.ch_id, format_map[ i ].format );
		break;
	}

#endif
	
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_PCM
 * @brief Set PCM timeslot 
 * @param TstVoipBusDataFormat.ch_id Channel ID
 * @param TstVoipBusDataFormat.timeslot1 Timeslot 1 
 * @param TstVoipBusDataFormat.timeslot2 Timeslot 2
 * @see VOIP_MGR_SET_PCM_TIMESLOT TstVoipPcmTimeslot
 */
int do_mgr_VOIP_MGR_SET_PCM_TIMESLOT( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipPcmTimeslot stVoipPcmTimeslot;
	int ret;

	COPY_FROM_USER(&stVoipPcmTimeslot, (TstVoipPcmTimeslot *)user, sizeof(TstVoipPcmTimeslot));
	
	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

	PRINT_MSG("VOIP_MGR_SET_PCM_TIMESLOT:ch_id = %d, ts = %d, %d\n", 
					stVoipPcmTimeslot.ch_id, 
					stVoipPcmTimeslot.timeslot1, stVoipPcmTimeslot.timeslot2);

#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP_IS_HOST
	// Host auto forward 
#else
	
	Set_PCM_Timeslot_cch( stVoipPcmTimeslot.ch_id,
					stVoipPcmTimeslot.timeslot1, stVoipPcmTimeslot.timeslot2 );

#endif
	
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_PCM
 * @brief Set PCM loopback mode 
 * @param TstVoipValue.value Loop mode. (0: Not loop mode, 1: loop mode, 2: loop mode with VoIP)
 * @param TstVoipValue.value1 Main channel ID
 * @param TstVoipValue.value2 Mate channel ID 
 * @see VOIP_MGR_SET_PCM_LOOP_MODE TstVoipValue pcm_set_LoopMode()
 */
int do_mgr_VOIP_MGR_SET_PCM_LOOP_MODE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifndef CONFIG_AUDIOCODES_VOIP
#ifdef PCM_LOOP_MODE_DRIVER
	TstVoipValue stVoipValue;
	int ret;
	
	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
	
	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward 
#else
	pcm_set_LoopMode(stVoipValue.value, stVoipValue.value1, stVoipValue.value2, stVoipValue.value3);
#endif
#endif
#endif
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_SLIC
 * @brief Set FXS FXO loopback mode 
 * @param TstVoipCfg.ch_id SLIC channel ID
 * @param TstVoipCfg.enable Enable(1) or disbale(0) FXS FXO loopback
 * @see VOIP_MGR_SET_FXS_FXO_LOOPBACK TstVoipCfg
 * @note This IO control is only for SLIC Si32177/8
 */
int do_mgr_VOIP_MGR_SET_FXS_FXO_LOOPBACK( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCfg stVoipCfg;
	int ret;
	
	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_FULLY_OFFLOAD
	// Host auto forward (fully offload)
#else
	FXS_FXO_DTx_DRx_Loopback_greedy(stVoipCfg.ch_id, stVoipCfg.enable);
#endif
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_SLIC
 * @brief Set SLIC to On Hook Transmission and PCM On
 * @param TstVoipCfg.ch_id SLIC channel ID
 * @see VOIP_MGR_SET_SLIC_ONHOOK_TRANS_PCM_START TstVoipCfg
 */
int do_mgr_VOIP_MGR_SET_SLIC_ONHOOK_TRANS_PCM_START( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCfg stVoipCfg;
	int ret;
	
	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_FULLY_OFFLOAD
	// Host auto forward (fully offload) 
#else
	SLIC_OnHookTrans_PCM_start(stVoipCfg.ch_id);
#endif
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_SLIC
 * @brief Make a FSX rings   
 * @see VOIP_MGR_SLIC_RING TstVoipSlicRing 
 */
int do_mgr_VOIP_MGR_SLIC_RING( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipSlicRing stVoipSlicRing;
	int ret;
	
	COPY_FROM_USER(&stVoipSlicRing, (TstVoipSlicRing *)user, sizeof(TstVoipSlicRing));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

	PRINT_MSG("VOIP_MGR_SLIC_RING:ch_id =%d, ring_set = %d\n", stVoipSlicRing.ch_id, stVoipSlicRing.ring_set);
	
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_FULLY_OFFLOAD
	// Host auto forward (fully offload)
#else

	if ( 1 == Is_DAA_Channel(stVoipSlicRing.ch_id))
	{
		return 0;
	}

#ifdef CONFIG_RTK_VOIP_LED
	if (stVoipSlicRing.ring_set == 1 && daa_ringing == 0)
		fxs_ringing[stVoipSlicRing.ch_id] = 1;
	if (stVoipSlicRing.ring_set == 0)
		fxs_ringing[stVoipSlicRing.ch_id] = 0;
	led_state_watcher(stVoipSlicRing.ch_id);
#endif
	//FXS_Ring( ( ring_struct * )&stVoipSlicRing);
	FXS_Ring( stVoipSlicRing.ch_id, stVoipSlicRing.ring_set );

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	/* bit0: ring set value,
	   bit1: set flag, once this bit is read, it will be clear to zeroa
	 */
	ioctrl_ring_set[stVoipSlicRing.ch_id] = stVoipSlicRing.ring_set+ (0x1<<1);
	PRINT_MSG("ioctrl_ring_set[%d]=%d\n", stVoipSlicRing.ch_id, ioctrl_ring_set[stVoipSlicRing.ch_id]);
#endif
#endif
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_SLIC
 * @brief Play a tone by SLIC
 * @see VOIP_MGR_SLIC_TONE TstVoipSlicTone 
 * @warning Now, we use @ref do_mgr_VOIP_MGR_SETPLAYTONE of DSP to generate a tone.
 */
int do_mgr_VOIP_MGR_SLIC_TONE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#if 0 // Using DSP tone generation instead!
	unsigned long flags;
	TstVoipSlicTone stVoipSlicTone;

	PRINT_MSG("VOIP_MGR_SLIC_TONE\n");
	copy_from_user(&stVoipSlicTone, (TstVoipSlicTone *)user, sizeof(TstVoipSlicTone));
	save_flags(flags); cli();
	SLIC_GenProcessTone(stVoipSlicTone.ch_id, &stVoipSlicTone);
	restore_flags(flags);
#endif
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_SLIC
 * @brief Reset or restart a SLIC
 * @see VOIP_MGR_SLIC_RESTART TstVoipSlicRestart SLIC_reset() 
 */
int do_mgr_VOIP_MGR_SLIC_RESTART( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_FULLY_OFFLOAD
	unsigned long flags;
#endif
	TstVoipSlicRestart stVoipSlicRestart;
	int ret;
	
	COPY_FROM_USER(&stVoipSlicRestart, (TstVoipSlicRestart*)user, sizeof(TstVoipSlicRestart));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

	PRINT_MSG("VOIP_MGR_SLIC_RESTART:ch_id:%d, codec_law=%d\n", stVoipSlicRestart.ch_id, stVoipSlicRestart.codec_law);
		
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_FULLY_OFFLOAD
	// Host auto forward (fully offload)
#else
	save_flags(flags); cli();
	SLIC_reset(stVoipSlicRestart.ch_id, stVoipSlicRestart.codec_law);
	restore_flags(flags);
#endif
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_SLIC
 * @brief Get SLIC's register 
 * @see VOIP_MGR_GET_SLIC_REG_VAL TstVoipSlicReg SLIC_read_reg() 
 * @see do_mgr_VOIP_MGR_SET_SLIC_REG_VAL()
 */
int do_mgr_VOIP_MGR_GET_SLIC_REG_VAL( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipSlicReg stSlicReg;
	
	COPY_FROM_USER(&stSlicReg, (TstVoipSlicReg *)user, sizeof(TstVoipSlicReg));

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_FULLY_OFFLOAD
	// Host auto forward (fully offload)
#else
	//SLIC_read_reg(stSlicReg.ch_id, stSlicReg.reg_num, 1, &stSlicReg.reg_ary);
	if( get_snd_type_cch( stSlicReg.ch_id ) == SND_TYPE_FXS ) {
		SLIC_read_reg(stSlicReg.ch_id, stSlicReg.reg_num, &stSlicReg.reg_len, stSlicReg.reg_ary);
	} else {
  #ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
		DAA_read_reg(stSlicReg.ch_id, stSlicReg.reg_num, &stSlicReg.reg_len, stSlicReg.reg_ary);
  #endif
	}
	//PRINT_MSG("VOIP_MGR_GET_SLIC_REG_VAL: ch%d read %d = %x\n", stSlicReg.ch_id, stSlicReg.reg_num, stSlicReg.reg_ary);
#endif

	return COPY_TO_USER(user, &stSlicReg, sizeof(TstVoipSlicReg), cmd, seq_no);

	//return 0;
}

/**
 * @ingroup VOIP_DRIVER_SLIC
 * @brief Set SLIC's Tx gain  
 * @param TstVoipValue.ch_id Channel ID
 * @param TstVoipValue.value SLIC Tx Gain. (x -6 = dB)
 * @see VOIP_MGR_SET_SLIC_TX_GAIN TstVoipValue Set_SLIC_Tx_Gain() 
 */
int do_mgr_VOIP_MGR_SET_SLIC_TX_GAIN( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#if 1 //def SUPPORT_SLIC_GAIN_CFG

	TstVoipValue stVoipValue;
	int ret;
	
	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward 
#else
	//g_txVolumneGain[stVoipValue.ch_id] = stVoipValue.value;
	//Set_SLIC_Tx_Gain(stVoipValue.ch_id, g_txVolumneGain[stVoipValue.ch_id]+1);
	Set_SLIC_Tx_Gain(stVoipValue.ch_id, stVoipValue.value);
#endif

	if (stVoipValue.value>0)
		PRINT_MSG("Adjust SLIC%d Tx Gain +%ddB\n", stVoipValue.ch_id, stVoipValue.value);
	else
		PRINT_MSG("Adjust SLIC%d Tx Gain %ddB\n", stVoipValue.ch_id, stVoipValue.value);

#else
	return NO_COPY_TO_USER( cmd, seq_no );
#endif
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_SLIC
 * @brief Set SLIC's Rx gain  
 * @param TstVoipValue.ch_id Channel ID
 * @param TstVoipValue.value SLIC Tx Gain. (x -6 = dB)
 * @see VOIP_MGR_SET_SLIC_RX_GAIN TstVoipValue Set_SLIC_Rx_Gain() 
 */
int do_mgr_VOIP_MGR_SET_SLIC_RX_GAIN( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#if 1 //def SUPPORT_SLIC_GAIN_CFG

	TstVoipValue stVoipValue;
	int ret;
	
	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward 
#else
	//g_rxVolumneGain[stVoipValue.ch_id] = stVoipValue.value;
	//Set_SLIC_Rx_Gain(stVoipValue.ch_id, g_rxVolumneGain[stVoipValue.ch_id]+1);
	Set_SLIC_Rx_Gain(stVoipValue.ch_id, stVoipValue.value);
#endif

	if (stVoipValue.value>0)
		PRINT_MSG("Adjust SLIC%d Rx Gain +%ddB\n", stVoipValue.ch_id, stVoipValue.value);
	else
		PRINT_MSG("Adjust SLIC%d Rx Gain %ddB\n", stVoipValue.ch_id, stVoipValue.value);

#else
	return NO_COPY_TO_USER( cmd, seq_no );
#endif
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_SLIC
 * @brief Set SLIC line voltage
 * @param TstVoipValue.ch_id Channel ID
 * @param TstVoipValue.value SLIC line voltage flag. 0: zero voltage, 1: normal voltage, 2: reverse voltage
 * @see VOIP_MGR_SET_SLIC_LINE_VOLTAGE  TstVoipValue OnHookLineReversal() SLIC_Set_LineVoltageZero()
 */
int do_mgr_VOIP_MGR_SET_SLIC_LINE_VOLTAGE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;
	
	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_FULLY_OFFLOAD
	// Host auto forward (fully offload) 
#else
	switch (stVoipValue.value)
	{
		case 0: //zero voltage
			SLIC_Set_LineVoltageZero(stVoipValue.ch_id);
			PRINT_MSG("Set SLIC line zero voltage, ch=%d\n", stVoipValue.ch_id);
			break;

		case 1: //normal voltage
			OnHookLineReversal(stVoipValue.ch_id, 0);
			PRINT_MSG("Set SLIC line normal voltage, ch=%d\n", stVoipValue.ch_id);
			break;

		case 2: //reverse voltage
			OnHookLineReversal(stVoipValue.ch_id, 1);
			PRINT_MSG("Set SLIC line reverse voltage, ch=%d\n", stVoipValue.ch_id);
			break;
	}
#endif
	return 0;
}


/**
 * @ingroup VOIP_DRIVER_SLIC
 * @brief Generate SLIC CPC signal
 * @param TstVoipCfg.ch_id Channel ID
 * @param TstVoipCfg.cfg SLIC CPC signal period (unit in ms)
 * @see VOIP_MGR_GEN_SLIC_CPC  TstVoipCfg SLIC_CPC_Gen()
 */
int do_mgr_VOIP_MGR_GEN_SLIC_CPC( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCfg stVoipCfg;
	int ret;
	
	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward 
#else
	SLIC_CPC_Gen_cch(stVoipCfg.ch_id, stVoipCfg.cfg);
	PRINT_MSG("Gen SLIC CPC signal, %d ms, ch=%d\n", stVoipCfg.cfg, stVoipCfg.ch_id);
#endif
	return 0;
}


/**
 * @ingroup VOIP_DRIVER_SLIC
 * @brief Set flash hook time (in unit of ms)  
 * @see VOIP_MGR_SET_FLASH_HOOK_TIME TstVoipHook  
 */
int do_mgr_VOIP_MGR_SET_FLASH_HOOK_TIME( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipHook stHookTime;
	int ret;
	
	COPY_FROM_USER(&stHookTime, (TstVoipHook *)user, sizeof(TstVoipHook));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward 
#else

	#ifdef EVENT_POLLING_TIMER
	flash_hook_time[stHookTime.ch_id] = (stHookTime.flash_time)/10;
	flash_hook_min_time[stHookTime.ch_id] = (stHookTime.flash_time_min)/10;
	#else
	#ifdef PCM_PERIOD
	flash_hook_time[stHookTime.ch_id] = (stHookTime.flash_time)/(10*PCM_PERIOD);
	flash_hook_min_time[stHookTime.ch_id] = (stHookTime.flash_time_min)/(10*PCM_PERIOD);
	#else
	flash_hook_time[stHookTime.ch_id] = (stHookTime.flash_time)/10;
	flash_hook_min_time[stHookTime.ch_id] = (stHookTime.flash_time_min)/10;
	#endif /*PCM_PERIOD*/
    	//PRINT_MSG("Set Flash Hook Min Time = %d ms\n", stHookTime.flash_time_min);
    	//PRINT_MSG("Set Flash Hook Time = %d ms\n", stHookTime.flash_time);
	#endif /*EVENT_POLLING_TIMER*/
	
	if ( (flash_hook_time[stHookTime.ch_id] < 10) || (flash_hook_time[stHookTime.ch_id] > 200) )
	{
		flash_hook_time[stHookTime.ch_id] = 30;
		PRINT_R("flash_hook_time[%d] setting is out of range. Set to default: 300 ms\n", stHookTime.ch_id);
	}
	
	if( flash_hook_min_time[stHookTime.ch_id] < 8)
	{
		flash_hook_min_time[stHookTime.ch_id] = 10;
		PRINT_R("flash_hook_min_time[%d] setting is out of range. Set to default: 100 ms\n", stHookTime.ch_id);
	}
	
	if (flash_hook_min_time[stHookTime.ch_id] >= flash_hook_time[stHookTime.ch_id])
	{
		flash_hook_time[stHookTime.ch_id] = 30;
		flash_hook_min_time[stHookTime.ch_id] = 10;
		PRINT_R("Error: flash_hook_min_time[%d] is larger or equal to flash_hook_time[%d]. Reset default.\n", stHookTime.ch_id, stHookTime.ch_id);
	}
#endif
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_SLIC
 * @brief Set SLIC ring cadence (in unit of ms)
 * @param TstVoipValue.ch_id Channel ID
 * @param TstVoipValue.value5 Time for ring on 
 * @param TstVoipValue.value6 Time for ring off 
 * @see VOIP_MGR_SET_SLIC_RING_CADENCE TstVoipValue  
 */
int do_mgr_VOIP_MGR_SET_SLIC_RING_CADENCE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;
	
	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

	PRINT_MSG("Set Ring Cadence (%d, %d)\n", stVoipValue.value5, stVoipValue.value6);
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward, and run this body 
#endif

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_FULLY_OFFLOAD
	/* For FXS Ring Cadence */
	if( get_snd_type_cch( stVoipValue.ch_id ) == SND_TYPE_FXS )
		SLIC_Set_Ring_Cadence(stVoipValue.ch_id, stVoipValue.value5 /* msec */, stVoipValue.value6 /* msec */);
#endif

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	gRingCadOff[stVoipValue.ch_id] = stVoipValue.value6;// After gen caller ID, for ring FXS usage.
#ifndef FXO_RING_NO_DET_CADENCE
	/* For FXO Ring Detection */
	ring_det_cad_set(stVoipValue.value5 /* msec */, stVoipValue.value6 /* msec */, stVoipValue.value6 /* msec */);
#endif
#endif

	return 0;
}


/**
 * @ingroup VOIP_DRIVER_SLIC
 * @brief Set SLIC ring frequency, amputide
 * @param TstVoipValue.ch_id Channel ID
 * @param TstVoipValue.value The Ring frequency, amputide preset table index
  * @see VOIP_MGR_SET_SLIC_RING_FRQ_AMP TstVoipValue  
 */
int do_mgr_VOIP_MGR_SET_SLIC_RING_FRQ_AMP( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;
	
	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

	PRINT_MSG("Set Ring Freq, Amp, ch=%d, preset=%d\n", stVoipValue.ch_id, stVoipValue.value);	
	
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_FULLY_OFFLOAD
	// Host auto forward (fully offload)
#else
	SLIC_Set_Ring_Freq_Amp(stVoipValue.ch_id, stVoipValue.value);
#endif

	return 0;
}

/**
 * @ingroup VOIP_DRIVER_SLIC
 * @brief Set the impedance  from preset table 
 * @param TstVoipValue.value preset table index 
 * @see VOIP_MGR_SET_IMPEDANCE TstVoipValue 
 */
int do_mgr_VOIP_MGR_SET_IMPEDANCE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_FULLY_OFFLOAD
	uint32 chid;
#endif

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_FULLY_OFFLOAD
	// Host auto forward (fully offload) 
#else
	//for (chid=0; chid<SLIC_CH_NUM; chid++)
	for (chid=0; chid<CON_CH_NUM; chid++) {
		if( get_snd_type_cch( chid ) != SND_TYPE_FXS )
			continue;
			
		SLIC_Set_Impendance(chid, stVoipValue.value);
	}

#endif
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_SLIC
 * @brief Set SLIC's register 
 * @see VOIP_MGR_SET_SLIC_REG_VAL TstVoipSlicReg SLIC_write_reg() 
 * @see do_mgr_VOIP_MGR_GET_SLIC_REG_VAL() 
 */
int do_mgr_VOIP_MGR_SET_SLIC_REG_VAL( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipSlicReg stSlicReg;
	
	COPY_FROM_USER(&stSlicReg, (TstVoipSlicReg *)user, sizeof(TstVoipSlicReg));

	//PRINT_MSG("VOIP_MGR_SET_SLIC_REG_VAL: ch%d write %d = %x\n", stSlicReg.ch_id, stSlicReg.reg_num, stSlicReg.reg_ary);
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_FULLY_OFFLOAD
	// Host auto forward (fully offload)
#else
	//SLIC_write_reg(stSlicReg.ch_id, stSlicReg.reg_num, 1, stSlicReg.reg_ary);
	if( get_snd_type_cch( stSlicReg.ch_id ) == SND_TYPE_FXS ) {
		SLIC_write_reg(stSlicReg.ch_id, stSlicReg.reg_num, &stSlicReg.reg_len, stSlicReg.reg_ary);
	} else {
  #ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
		DAA_write_reg(stSlicReg.ch_id, stSlicReg.reg_num, &stSlicReg.reg_len, stSlicReg.reg_ary);
  #endif
	}
	//stSlicReg.ret_val = 0;
#endif
	return COPY_TO_USER(user, &stSlicReg, sizeof(TstVoipSlicReg), cmd, seq_no);
	//return 0;
}

/**
 * @ingroup VOIP_DRIVER_SLIC
 * @brief Get SLIC's status 
 * @param TstVoipCfg.ch_id Channel ID
 * @param TstVoipCfg.cfg Hook status. (0: on-hook, 1: off-hook) 
 * @see VOIP_MGR_GET_SLIC_STAT TstVoipCfg  
 */
int do_mgr_VOIP_MGR_GET_SLIC_STAT( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_FULLY_OFFLOAD
	unsigned long flags;
#endif
	TstVoipCfg stVoipCfg;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_FULLY_OFFLOAD
	// Host auto forward (fully offload)
#else
	save_flags(flags); cli();
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
	if ( 1 == Is_DAA_Channel(stVoipCfg.ch_id))
	{
		stVoipCfg.cfg = DAA_Hook_Status(stVoipCfg.ch_id, 1);
	}
	else
#endif
	{
		stVoipCfg.cfg = SLIC_Get_Hook_Status(stVoipCfg.ch_id, 1);
	}
	
	restore_flags(flags);
#endif
	return COPY_TO_USER(user, &stVoipCfg, sizeof(TstVoipCfg), cmd, seq_no);
	//return 0;
}

/**
 * @ingroup VOIP_DRIVER_SLIC
 * @brief Do actions after SLIC on-hook  
 * @param TstVoipCfg.ch_id Channel ID
 * @see VOIP_MGR_SLIC_ONHOOK_ACTION TstVoipCfg 
 * @note AudioCodes only
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SLIC_ONHOOK_ACTION( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	// only used in AudioCodes
#if 1
	return NO_COPY_TO_USER( cmd, seq_no );
#else	
	TstVoipCfg stVoipCfg;
	int ret;
	
	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

	return 0; 
#endif
}
#else
int do_mgr_VOIP_MGR_SLIC_ONHOOK_ACTION( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCfg stVoipCfg;
	int ret;
	
	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;
	
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// host auto forward 
#else
	// Disable DTMF Detection Before Close ACMW Channel
	if ( 0 == Is_DAA_Channel(stVoipCfg.ch_id))
	{
		RtkAc49xApiSetDtmfDetection(stVoipCfg.ch_id, CONTROL__DISABLE);
		RtkAc49xApiOnhookAction(stVoipCfg.ch_id);
	}
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_DRIVER_SLIC
 * @brief Do actions after SLIC off-hook  
 * @param TstVoipCfg.ch_id Channel ID
 * @see VOIP_MGR_SLIC_OFFHOOK_ACTION TstVoipCfg 
 * @note AudioCodes only
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SLIC_OFFHOOK_ACTION( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	// only used in AudioCodes
#if 1
	return NO_COPY_TO_USER( cmd, seq_no );
#else	
	TstVoipCfg stVoipCfg;
	int ret;
	
	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

	return 0; 
#endif
}
#else
int do_mgr_VOIP_MGR_SLIC_OFFHOOK_ACTION( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCfg stVoipCfg;
	int ret;
	
	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward 
#else
	/* FXO channel should always in offhook action state, it's set when init, not by IO ctrl. */
	if ( 0 == Is_DAA_Channel(stVoipCfg.ch_id))
	{
		RtkAc49xApiOffhookAction(stVoipCfg.ch_id);
		// Enable DTMF Detection After Open ACMW Channel
		RtkAc49xApiSetDtmfDetection(stVoipCfg.ch_id, CONTROL__ENABLE);
	}
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_DRIVER_SLIC
 * @brief Determine whether FXS or FXO is connected, not connected or busy 
 * @param TstVoipValue.ch_id Channel ID
 * @param TstVoipValue.value FXS or FXO connection status. (0: connect, 1: not connect, 2: busy)
 * @note It can not check virtual DAA
 * @see VOIP_MGR_LINE_CHECK TstVoipValue 
 */
int do_mgr_VOIP_MGR_LINE_CHECK( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	
	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(stVoipValue));

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_FULLY_OFFLOAD
	// Host auto forward 
#else

#if defined (CONFIG_RTK_VOIP_DRIVERS_FXO) && !defined (CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA)
	if ( 1 == Is_DAA_Channel(stVoipValue.ch_id))
	{
		/* for FXO: 0: connect, 1: not connect, 2: busy */
		stVoipValue.value = DAA_Check_Line_State(stVoipValue.ch_id);
	}
	else
#endif
	{
		/*	for FXS:
		0: Phone dis-connect, 
		1: Phone connect, 
		2: Phone off-hook, 
		3: Check time out ( may connect too many phone set => view as connect),
		4: Can not check, Linefeed should be set to active state first.
		*/
		stVoipValue.value = FXS_Line_Check(stVoipValue.ch_id);
	}
#endif
	return COPY_TO_USER(user, &stVoipValue, sizeof(TstVoipValue), cmd, seq_no);
	//return 0;
}

/**
 * @ingroup VOIP_DRIVER_SLIC
 * @brief Add a hook event into fifo 
 * @param TstVoipCfg.ch_id Channel ID
 * @param TstVoipCfg.cfg Hook event 
 * @param TstVoipCfg.cfg2 Hook event for new event queue 
 * @see VOIP_MGR_HOOK_FIFO_IN TstVoipCfg 
 */
int do_mgr_VOIP_MGR_HOOK_FIFO_IN( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
	TstVoipCfg stVoipCfg;
	int ret;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward 
#else
	if ((Is_DAA_Channel(stVoipCfg.ch_id) == 1) && (stVoipCfg.cfg2== VEID_HOOK_FXO_BUSY_TONE))
	{
#ifdef PULSE_DIAL_GEN
		DAA_PulseGenKill_cch(stVoipCfg.ch_id);
#endif			
		DAA_On_Hook(stVoipCfg.ch_id);
	}
	voip_event_hook_in( stVoipCfg.ch_id, stVoipCfg.cfg2 );
#endif
#else
	return NO_COPY_TO_USER( cmd, seq_no );
#endif
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_SLIC
 * @brief Get SLIC' RAM value  
 * @see VOIP_MGR_GET_SLIC_RAM_VAL TstVoipSlicRam 
 * @see do_mgr_VOIP_MGR_SET_SLIC_RAM_VAL()
 */
int do_mgr_VOIP_MGR_GET_SLIC_RAM_VAL( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipSlicRam stSlicRam;
	
	COPY_FROM_USER(&stSlicRam, (TstVoipSlicRam *)user, sizeof(TstVoipSlicRam));

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_FULLY_OFFLOAD
	// Host auto forward (fully offload)
#else
	if( get_snd_type_cch( stSlicRam.ch_id ) == SND_TYPE_FXS ) {
		SLIC_read_ram(stSlicRam.ch_id, stSlicRam.reg_num, &stSlicRam.reg_val);
	} else {
  #ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
		DAA_read_ram(stSlicRam.ch_id, stSlicRam.reg_num, &stSlicRam.reg_val);
  #endif
	}
	//PRINT_MSG("VOIP_MGR_GET_SLIC_RAM_VAL: ch%d read %d = %x\n", stSlicRam.ch_id, stSlicRam.reg_num, stSlicRam.reg_val);
#endif
	return COPY_TO_USER(user, &stSlicRam, sizeof(TstVoipSlicRam), cmd, seq_no);
	//return 0;
}

/**
 * @ingroup VOIP_DRIVER_SLIC
 * @brief Set SLIC' RAM value  
 * @see VOIP_MGR_SET_SLIC_RAM_VAL TstVoipSlicRam 
 * @see do_mgr_VOIP_MGR_GET_SLIC_RAM_VAL()
 */
int do_mgr_VOIP_MGR_SET_SLIC_RAM_VAL( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipSlicRam stSlicRam;
	
	COPY_FROM_USER(&stSlicRam, (TstVoipSlicRam *)user, sizeof(TstVoipSlicRam));

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_FULLY_OFFLOAD
	// Host auto forward (fully offload)
#else
	//PRINT_MSG("VOIP_MGR_SET_SLIC_RAM_VAL: ch%d write %d = %x\n", stSlicReg.ch_id, stSlicReg.reg_num, stSlicReg.reg_val);
	if( get_snd_type_cch( stSlicRam.ch_id ) == SND_TYPE_FXS ) {
		SLIC_write_ram(stSlicRam.ch_id, stSlicRam.reg_num, stSlicRam.reg_val);
	} else {
  #ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
		DAA_write_ram(stSlicRam.ch_id, stSlicRam.reg_num, stSlicRam.reg_val);
  #endif
	}
#endif
	return COPY_TO_USER(user, &stSlicRam, sizeof(TstVoipSlicRam), cmd, seq_no);
	//return 0;
}

/**
 * @ingroup VOIP_DRIVER_SLIC
 * @brief Set cadence for FXO ring detection (in unit of ms)
 * @param TstVoipValue.value5 Time for ring on 
 * @param TstVoipValue.value6 Time for first ring off 
 * @param TstVoipValue.value7 Time for ring off 
 * @see VOIP_MGR_SET_RING_DETECTION TstVoipValue 
 */
int do_mgr_VOIP_MGR_SET_RING_DETECTION( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
	TstVoipValue stVoipValue;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward 
#else

#ifdef FXO_RING_NO_DET_CADENCE
	PRINT_MSG("VOIP_MGR_SET_RING_DETECTION\n");
	/* For FXO Ring Detection */
	ring_det_cad_set(stVoipValue.value5 /* msec */, stVoipValue.value6 /* msec */, stVoipValue.value7 /* msec */);
#endif
#endif
#else
	return NO_COPY_TO_USER( cmd, seq_no );
#endif
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_DAA
 * @brief Check whether DAA is ringing
 * @param TstVoipCfg.ch_id Channel ID
 * @param [out] TstVoipCfg.enable Ring state (0:Not ring state  1:Ringing now)
 * @see VOIP_MGR_DAA_RING TstVoipCfg 
 */
int do_mgr_VOIP_MGR_DAA_RING( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCfg stVoipCfg;
	
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_FULLY_OFFLOAD
	int daa_ring;
#endif
	
	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_FULLY_OFFLOAD
	// Host auto forward (fully offload)
#else
	//static unsigned int counter = 0, counter_1 = 0;;

	#ifndef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA
	daa_ring = DAA_Ring_Detection(stVoipCfg.ch_id);

	/*
	   Because only pcm channel 0 is connected to DAA is implemented(2006-10-13),
	   when DAA incomimg ring is detected, we only ring the phone 0 (pcm channel 0).
	   Once pcm channel N connect to DAA is finished, it is needed to also ring
	   the phone N (pcm channel N).
	*/
	#else
	/* 0:Not ring state  1:Ringing now */
	if (stVoipCfg.ch_id == 0)
		daa_ring = virtual_daa_ring_det();
	else if (stVoipCfg.ch_id == 1)
#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA_2_RELAY_SUPPORT
	daa_ring = virtual_daa_ring_det();
#else
	daa_ring = 0;
#endif
	else
		daa_ring = 0;
	/*
	   Because V210 only have one Relay for phone 0 (pcm channel 0), when DAA
	   incomimg ring is detected, we only ring the phone 0 (pcm channel 0).
	   Once Relay for phone N is attached, it is needed to also ring
	   the phone N (pcm channel N).
	*/
	#endif

	stVoipCfg.enable = daa_ring;
#endif
	return COPY_TO_USER(user, &stVoipCfg, sizeof(TstVoipCfg), cmd, seq_no);
#else
	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
	stVoipCfg.enable = 0;
	return COPY_TO_USER(user, &stVoipCfg, sizeof(TstVoipCfg), cmd, seq_no);
#endif
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_DAA
 * @brief Off-hook DAA line 
 * @param TstVoipCfg.ch_id Channel ID
 * @param [out] TstVoipCfg.enable Result. (1: success, 0xff: line not connect or busy or not support)
 * @see VOIP_MGR_DAA_OFF_HOOK TstVoipCfg 
 * @see do_mgr_VOIP_MGR_DAA_ON_HOOK()
 */
int do_mgr_VOIP_MGR_DAA_OFF_HOOK( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCfg stVoipCfg;
	
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
	// for virtual DAA off-hook(channel ID is FXS channel)
	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_FULLY_OFFLOAD
	// Host auto forward (fully offload)
#else
	/* 1: success, 0xff: line not connect or busy or not support */
	stVoipCfg.enable = DAA_Off_Hook(stVoipCfg.ch_id);
	//PRINT_MSG( "stVoipCfg.enable=%u\n", stVoipCfg.enable );
#endif
	PRINT_MSG("VOIP_MGR_DAA_OFF_HOOK(%d)\n", stVoipCfg.ch_id);
	return COPY_TO_USER(user, &stVoipCfg, sizeof(TstVoipCfg), cmd, seq_no);
#else
	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
	stVoipCfg.enable = 0xff;
	return COPY_TO_USER(user, &stVoipCfg, sizeof(TstVoipCfg), cmd, seq_no);
#endif
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_DAA
 * @brief On-hook DAA line 
 * @param TstVoipCfg.ch_id Channel ID 
 * @see VOIP_MGR_DAA_ON_HOOK TstVoipCfg 
 * @see do_mgr_VOIP_MGR_DAA_OFF_HOOK()
 * @note This function is for virutal DAA 
 */
int do_mgr_VOIP_MGR_DAA_ON_HOOK( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
	TstVoipCfg stVoipCfg;
	int ret;
	
	// for virtual DAA on-hook(channel is FXS channel)
	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
	
	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_FULLY_OFFLOAD
	// Host auto forward (fully offload) 
#else
	DAA_On_Hook(stVoipCfg.ch_id);
#endif
	PRINT_MSG("VOIP_MGR_DAA_ON_HOOK(%d)\n", stVoipCfg.ch_id);
#else
	return NO_COPY_TO_USER( cmd, seq_no );
#endif
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_DAA
 * @brief Set DAA Tx Gain 
 * @param TstVoipValue.value DAA Tx gain. (x -7 = dB)
 * @see VOIP_MGR_SET_DAA_TX_GAIN TstVoipValue 
 * @see do_mgr_VOIP_MGR_SET_DAA_RX_GAIN()
 * @note This function is for virutal DAA 
 */
int do_mgr_VOIP_MGR_SET_DAA_TX_GAIN( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
	extern void DAA_Set_Tx_Gain(int chid, unsigned char tx_gain);
	TstVoipValue stVoipValue;
	int ret;
#if !defined( CONFIG_RTK_VOIP_IPC_ARCH_FULLY_OFFLOAD ) &&	\
	!defined( CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA )
	int cch;
#endif
	
	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_FULLY_OFFLOAD
	// Host auto forward, and run this body 
#else
	#ifndef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA
	//#ifdef CONFIG_RTK_VOIP_DRIVERS_SI3050
	//g_txVolumneGain_DAA = stVoipValue.value;
	//#endif
	for( cch = 0; cch < CON_CH_NUM; cch ++ ) {
		if( get_snd_type_cch( cch ) == SND_TYPE_DAA )
			DAA_Set_Tx_Gain(cch, stVoipValue.value);
	}

	PRINT_MSG("Set DAA Tx Gain = %ddB \n", stVoipValue.value-7);
	#endif
#endif
#else
	return NO_COPY_TO_USER( cmd, seq_no );
#endif
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_DAA
 * @brief Set DAA Rx Gain 
 * @param TstVoipValue.value DAA Tx gain. (x -7 = dB)
 * @see VOIP_MGR_SET_DAA_RX_GAIN TstVoipValue 
 * @see do_mgr_VOIP_MGR_SET_DAA_TX_GAIN()
 */
int do_mgr_VOIP_MGR_SET_DAA_RX_GAIN( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
	TstVoipValue stVoipValue;
	int ret;
#if !defined( CONFIG_RTK_VOIP_IPC_ARCH_FULLY_OFFLOAD ) &&	\
	!defined( CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA )
	int cch;
#endif

	// thlin +
	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_FULLY_OFFLOAD
	// Host auto forward, and run this body 
#else
	#ifndef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA
	//#ifdef CONFIG_RTK_VOIP_DRIVERS_SI3050
	//g_rxVolumneGain_DAA = stVoipValue.value;
	//#endif
	for( cch = 0; cch < CON_CH_NUM; cch ++ ) {
		if( get_snd_type_cch( cch ) == SND_TYPE_DAA )
			DAA_Set_Rx_Gain(cch, stVoipValue.value);
	}

	PRINT_MSG("Set DAA Rx Gain = %ddB\n", stVoipValue.value-7);
	#endif
#endif
#else
	return NO_COPY_TO_USER( cmd, seq_no );
#endif
	return 0;
}


/**
 * @ingroup VOIP_DRIVER_DAA
 * @brief Get DAA Line Voltage
 * @param TstVoipValue.ch_id Channel ID
   @param [out] TstVoipValue.value5 DAA Line Voltage
 * @see VOIP_MGR_GET_DAA_LINE_VOLTAGE TstVoipValue 
 * @see do_mgr_VOIP_MGR_GET_DAA_LINE_VOLTAGE()
 */
int do_mgr_VOIP_MGR_GET_DAA_LINE_VOLTAGE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;

#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
	// thlin +
	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_FULLY_OFFLOAD
	// Host auto forward (fully offload) 
#else

	#ifdef CONFIG_RTK_VOIP_DRIVERS_SI3050
	stVoipValue.value5 = DAA_Get_Line_Voltage(stVoipValue.ch_id);
	#endif
#endif
	return COPY_TO_USER(user, &stVoipValue, sizeof(TstVoipValue), cmd, seq_no);
#else
	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
	stVoipValue.value5 = 0;
	return COPY_TO_USER(user, &stVoipValue, sizeof(TstVoipValue), cmd, seq_no);	
#endif
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_DAA
 * @brief Set DAA ISR flow 
 * @param TstVoipValue.ch_id Channel ID
 * @param TstVoipValue.m_id Media ID 
 * @see VOIP_MGR_SET_DAA_ISR_FLOW TstVoipValue 
 * @see do_mgr_VOIP_MGR_GET_DAA_ISR_FLOW()
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_DAA_ISR_FLOW( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;
	
	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;
#if 0
//#ifdef CONFIG_RTK_VOIP_DRIVERS_SI3050
	save_flags(flags); cli();
	DAA_ISR_FLOW[stVoipValue.ch_id] = stVoipValue.value;
	s_id = API_GetSid(stVoipValue.ch_id, stVoipValue.m_id);

	if (stVoipValue.value != DRIVER_FLOW_NORMAL)// not normal flow
		sid_owner[s_id] = 1; //DAA own
	else
		sid_owner[s_id] = 0; //SLIC own
	restore_flags(flags);
	PRINT_MSG("rtk_Set_DAA_ISR_FLOW: chid=%d, flow=%d, mid=%d, sid=%d\n", stVoipValue.ch_id, stVoipValue.value, stVoipValue.m_id, s_id);
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_DAA_ISR_FLOW( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;
	
	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;
#if 0
//#ifdef CONFIG_RTK_VOIP_DRIVERS_SI3050
	save_flags(flags); cli();
	DAA_ISR_FLOW[stVoipValue.ch_id] = stVoipValue.value;
	s_id = 2*stVoipValue.ch_id+ PROTOCOL__RTP - 1;
	
	if (stVoipValue.value != DRIVER_FLOW_NORMAL)// not normal flow
		sid_owner[s_id] = 1; //DAA own
	else
		sid_owner[s_id] = 0; //SLIC own
	restore_flags(flags);
	PRINT_MSG("rtk_Set_DAA_ISR_FLOW: chid=%d, flow=%d, mid=%d, sid=%d\n", stVoipValue.ch_id, stVoipValue.value, stVoipValue.m_id, s_id);
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_DRIVER_DAA
 * @brief Get DAA ISR flow 
 * @param TstVoipValue.ch_id Channel ID 
 * @param [out] TstVoipValue.value DAA ISR flow. 
 * @see VOIP_MGR_GET_DAA_ISR_FLOW TstVoipValue 
 * @see do_mgr_VOIP_MGR_SET_DAA_ISR_FLOW()
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_GET_DAA_ISR_FLOW( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;
	
	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;
#if 0
//#ifdef CONFIG_RTK_VOIP_DRIVERS_SI3050
	save_flags(flags); cli();
	stVoipValue.value = DAA_ISR_FLOW[stVoipValue.ch_id];
	restore_flags(flags);
	copy_to_user(user, &stVoipValue, sizeof(TstVoipValue));
	PRINT_MSG("rtk_Get_DAA_ISR_FLOW: chid=%d, flow=%d\n", stVoipValue.ch_id, stVoipValue.value);
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_GET_DAA_ISR_FLOW( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;
	
	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;
#if 0
//#ifdef CONFIG_RTK_VOIP_DRIVERS_SI3050
	save_flags(flags); cli();
	stVoipValue.value = DAA_ISR_FLOW[stVoipValue.ch_id];
	restore_flags(flags);
	copy_to_user(user, &stVoipValue, sizeof(TstVoipValue));
	PRINT_MSG("rtk_Get_DAA_ISR_FLOW: chid=%d, flow=%d\n", stVoipValue.ch_id, stVoipValue.value);
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_DRIVER_DAA
 * @brief Hold DAA channel  
 * @warning We don't use this function now. 
 * @see VOIP_MGR_SET_DAA_PCM_HOLD_CFG TstVoipValue 
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_DAA_PCM_HOLD_CFG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;
	
	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;
#if 0
//#ifdef CONFIG_RTK_VOIP_DRIVERS_SI3050
	save_flags(flags); cli();
	if (stVoipValue.value == 1)
	{
		pcm_Hold_DAA(stVoipValue.value2); /* hold daa used pcm channel */
	
		if ( g_DAA_used[stVoipValue.ch_id] == DAA_USE)
			g_DAA_used[stVoipValue.ch_id] = DAA_USE_HOLD; // hold
	}
	else
	{
		pcm_UnHold_DAA(stVoipValue.value2);
	
		if ( g_DAA_used[stVoipValue.ch_id] == DAA_USE_HOLD)
			g_DAA_used[stVoipValue.ch_id] = DAA_USE; // un-hold
	}
	restore_flags(flags);
	PRINT_MSG("VOIP_MGR_SET_DAA_PCM_HOLD_CFG: chid=%d, enable=%d\n", stVoipValue.ch_id, stVoipValue.value);
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_DAA_PCM_HOLD_CFG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;
	
	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;
#if 0
//#ifdef CONFIG_RTK_VOIP_DRIVERS_SI3050
	save_flags(flags); cli();
	if (stVoipValue.value == 1)
	{
		pcm_Hold_DAA(stVoipValue.value2); /* hold daa used pcm channel */
	
		if ( g_DAA_used[stVoipValue.ch_id] == DAA_USE)
			g_DAA_used[stVoipValue.ch_id] = DAA_USE_HOLD; // hold
	}
	else
	{
		pcm_UnHold_DAA(stVoipValue.value2);
	
		if ( g_DAA_used[stVoipValue.ch_id] == DAA_USE_HOLD)
			g_DAA_used[stVoipValue.ch_id] = DAA_USE; // un-hold
	}
	restore_flags(flags);
	        PRINT_MSG("VOIP_MGR_SET_DAA_PCM_HOLD_CFG: chid=%d, enable=%d\n", stVoipValue.ch_id, stVoipValue.value);
#endif
	return 0;	
}
#endif

/**
 * @ingroup VOIP_DRIVER_DAA
 * @brief Check whether DAA is in busy tone status 
 * @param TstVoipValue.ch_id Channel ID
 * @param [out] TstVoipValue.value Busy tone is 1: Detected, 0: NOT detected. 
 * @see VOIP_MGR_GET_DAA_BUSY_TONE_STATUS TstVoipValue 
 * @warning We don't use this function anymore. 
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_GET_DAA_BUSY_TONE_STATUS( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	
#ifdef FXO_BUSY_TONE_DET
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	unsigned long flags;
#endif

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
	printk(AC_FORE_RED "VOIP_MGR_GET_DAA_BUSY_TONE_STATUS: Error! This IO control should not be used. To get busy tone event should poll event FIFO by channel.\n" AC_RESET);
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward 
#else
	save_flags(flags); cli();
	stVoipValue.value = busy_tone_flag_get(stVoipValue.ch_id);/* Busy tone is  1: Detected, 0: NOT detected. */
	restore_flags(flags);
#endif
	return COPY_TO_USER(user, &stVoipValue, sizeof(TstVoipValue), cmd, seq_no);
#else
	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
	stVoipValue.value = 0;
	return COPY_TO_USER(user, &stVoipValue, sizeof(TstVoipValue), cmd, seq_no);
#endif

	return 0;
}
#else
int do_mgr_VOIP_MGR_GET_DAA_BUSY_TONE_STATUS( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	
#ifdef FXO_BUSY_TONE_DET
	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	printk(AC_FORE_RED "VOIP_MGR_GET_DAA_BUSY_TONE_STATUS: Error! This IO control should not be used. To get busy tone event should poll event FIFO by channel.\n" AC_RESET);
	return COPY_TO_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue), cmd, seq_no);
#else
	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
	stVoipValue.value = 0;
	return COPY_TO_USER(user, &stVoipValue, sizeof(TstVoipValue), cmd, seq_no);
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_DRIVER_DAA
 * @brief Get caller ID from DAA  
 * @param TstVoipCID.daa_id DAA ID 
 * @param [out] TstVoipCID.string Caller ID - phonenumber 
 * @param [out] TstVoipCID.string2 Caller ID - date 
 * @param [out] TstVoipCID.cid_name Caller ID - name. (If string2 is a null string, ignore this parameter.)
 * @see VOIP_MGR_GET_DAA_CALLER_ID TstVoipCID 
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_GET_DAA_CALLER_ID( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCID stCIDstr;

#ifdef FXO_CALLER_ID_DET
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	unsigned long flags;
	extern TstVoipciddet dtmf_cid_res[];
#endif

	COPY_FROM_USER(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward 
#else

	save_flags(flags); cli();
	dtmf_cid_valid[stCIDstr.daa_id] = 0;
	fsk_cid_valid[stCIDstr.daa_id] = 0;

	if (dtmf_cid_res[stCIDstr.daa_id].cid_valid == 1)
	{
		dtmf_cid_valid[stCIDstr.daa_id] = 1;
	}
	else if (stVoipciddet[stCIDstr.daa_id].cid_valid == 1)// stVoipciddet is global variable in cid_det.c
	{
		fsk_cid_valid[stCIDstr.daa_id] = 1;
	}

	if (dtmf_cid_valid[stCIDstr.daa_id] == 1) // DTMF
	{
		strcpy(stCIDstr.string, dtmf_cid_res[stCIDstr.daa_id].number); // caller id
		stCIDstr.string2[0] = 0;
		dmtf_cid_det_init(stCIDstr.daa_id);
		dtmf_cid_valid[stCIDstr.daa_id] = 0;
	}
	else if (fsk_cid_valid[stCIDstr.daa_id] == 1) // FSK
	{
		strcpy(stCIDstr.string, stVoipciddet[stCIDstr.daa_id].number);	// caller id
		strcpy(stCIDstr.string2, stVoipciddet[stCIDstr.daa_id].date);	// date
		strcpy(stCIDstr.cid_name, stVoipciddet[stCIDstr.daa_id].cid_name);	// caller id name
		init_cid_det_si3500(stCIDstr.daa_id);
		fsk_cid_valid[stCIDstr.daa_id] = 0;
	}
	else
	{
		stCIDstr.string[0] = 0;
		stCIDstr.string2[0] = 0;
	}

	restore_flags(flags);

	if (dtmf_cid_valid[stCIDstr.daa_id] == 1) // DTMF
		PRINT_MSG("-->dtmf get cid (%d)\n", stCIDstr.daa_id);
	else if (fsk_cid_valid[stCIDstr.daa_id] == 1) // FSK
		PRINT_MSG("-->fsk get cid (%d)\n", stCIDstr.daa_id);

	if (fsk_cid_valid[stCIDstr.daa_id] == 1) // FSK
	{
		int temp_index;
		PRINT_MSG("-->the rude data is: ");
		for(temp_index=0 ; temp_index<20 ; temp_index++)/*only print out first 20 data, total data is 256byte.*/
			PRINT_MSG(" %02x ",stVoipciddet[stCIDstr.daa_id].byte_buf[temp_index]);
	}
#endif
	return COPY_TO_USER(user, &stCIDstr, sizeof(TstVoipCID), cmd, seq_no);
#else
	COPY_FROM_USER(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));
	stCIDstr.string[0] = 0;
	stCIDstr.string2[0] = 0;
	return COPY_TO_USER(user, &stCIDstr, sizeof(TstVoipCID), cmd, seq_no);	
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_GET_DAA_CALLER_ID( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef FXO_CALLER_ID_DET
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	extern TstCidDet cid_res[];
#endif
#endif
	TstVoipCID stCIDstr;
	
#ifdef FXO_CALLER_ID_DET
	unsigned long flags;

	COPY_FROM_USER(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));
	
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward 
#else
	save_flags(flags); cli();
	
	if (cid_res[stCIDstr.daa_id].dtmf_cid_valid == 1) // DTMF
	{
		strcpy(stCIDstr.string, cid_res[stCIDstr.daa_id].number); // caller id
#if 0	
		printk("CID = ");
		int i;
		for (i=0; i< strlen(stCIDstr.string); i++)
		{
			printk("%c ", stCIDstr.string[i]);
		}
		printk("\n");
#endif
		stCIDstr.string2[0] = 0;
		cid_res[stCIDstr.daa_id].dtmf_cid_valid = 0;
	}
	else if (cid_res[stCIDstr.daa_id].fsk_cid_valid == 1) // FSK
	{
		strcpy(stCIDstr.string, cid_res[stCIDstr.daa_id].number);	// caller id
		strcpy(stCIDstr.string2, cid_res[stCIDstr.daa_id].date);	// date and time
		strcpy(stCIDstr.cid_name, cid_res[stCIDstr.daa_id].cid_name);	// name
		
		cid_res[stCIDstr.daa_id].fsk_cid_valid = 0;
#if 0	                        
        i=0;
        printk("Number = ");
        for (i=0; i < (strlen(stCIDstr.string)); i++)
        {
        	printk("%c ", stCIDstr.string[i]);
        }
        printk("\n");
        
        printk("Date and Time = ");
        for (i=0; i < (strlen(stCIDstr.string2)); i++)
        {
        	printk("%c ", stCIDstr.string2[i]);
        }
        printk("\n");
        
        printk("Name = ");
        for (i=0; i < (strlen(stCIDstr.cid_name)); i++)
        {
        	printk("%c ", stCIDstr.cid_name[i]);
        }
        printk("\n");
#endif
	}
	else
	{
		stCIDstr.string[0] = 0;
		stCIDstr.string2[0] = 0;
	}

	restore_flags(flags);

#if 0
	if (dtmf_cid_valid[stCIDstr.daa_id] == 1) // DTMF
		PRINT_MSG("-->dtmf get cid (%d)\n", stCIDstr.daa_id);
	else if (fsk_cid_valid[stCIDstr.daa_id] == 1) // FSK
		PRINT_MSG("-->fsk get cid (%d)\n", stCIDstr.daa_id);
#endif

	//stCIDstr.ret_val = 0;
#endif
	return COPY_TO_USER(user, &stCIDstr, sizeof(TstVoipCID), cmd, seq_no);
#else
	COPY_FROM_USER(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));
	stCIDstr.string[0] = 0;
	stCIDstr.string2[0] = 0;
	return COPY_TO_USER(user, &stCIDstr, sizeof(TstVoipCID), cmd, seq_no);	
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_DRIVER_DAA
 * @brief Determine DAA used by which SLIC
 * @param TstVoipValue.ch_id Channel ID
 * @param [out] TstVoipValue.value DAA is 0: NOT used, 1: used by SLIC N(use channel n), 2: used and held.
 * @see VOIP_MGR_GET_DAA_USED_BY_WHICH_SLIC TstVoipValue 
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_GET_DAA_USED_BY_WHICH_SLIC( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;

#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	unsigned long flags;
#endif
	
	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward 
#else

	save_flags(flags); cli();
	stVoipValue.value = g_DAA_used[stVoipValue.ch_id];
	restore_flags(flags);
#endif
	return COPY_TO_USER(user, &stVoipValue, sizeof(TstVoipValue), cmd, seq_no);
#else
	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
	stVoipValue.value = 0;
	return COPY_TO_USER(user, &stVoipValue, sizeof(TstVoipValue), cmd, seq_no);	
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_GET_DAA_USED_BY_WHICH_SLIC( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;

#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
	unsigned long flags;
	
	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
	
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward 
#else
	save_flags(flags); cli();
	stVoipValue.value = g_DAA_used[stVoipValue.ch_id];
	restore_flags(flags);
#endif
	return COPY_TO_USER(user, &stVoipValue, sizeof(TstVoipValue), cmd, seq_no);
#else
	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
	stVoipValue.value = 0;
	return COPY_TO_USER(user, &stVoipValue, sizeof(TstVoipValue), cmd, seq_no);	
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_DRIVER_DAA
 * @brief On-hook FXO line  
 * @param TstVoipCfg.ch_id Channel ID 
 * @see VOIP_MGR_FXO_ON_HOOK TstVoipCfg 
 * @see do_mgr_VOIP_MGR_FXO_OFF_HOOK()
 * @note This function is for real DAA 
 */
int do_mgr_VOIP_MGR_FXO_ON_HOOK( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
	extern void dis_tone_det_init(int);
	TstVoipCfg stVoipCfg;
	int ret;

	// for real DAA on-hook(channel is FXO channel)
	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;
	
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward 
#else

#ifdef PULSE_DIAL_GEN
	DAA_PulseGenKill_cch(stVoipCfg.ch_id);
#endif
	DAA_On_Hook(stVoipCfg.ch_id);
#if defined (FXO_BUSY_TONE_DET) && !defined (AUDIOCODES_VOIP)
	busy_tone_det_init(stVoipCfg.ch_id);
	dis_tone_det_init(stVoipCfg.ch_id);
#endif
#endif
	PRINT_MSG("VOIP_MGR_FXO_ON_HOOK(%d)\n", stVoipCfg.ch_id);
#else
	return NO_COPY_TO_USER( cmd, seq_no );
#endif
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_DAA
 * @brief Off-hook FXO line  
 * @param TstVoipCfg.ch_id Channel ID 
 * @see VOIP_MGR_FXO_OFF_HOOK TstVoipCfg 
 * @see do_mgr_VOIP_MGR_FXO_ON_HOOK()
 * @note This function is for real DAA 
 */
int do_mgr_VOIP_MGR_FXO_OFF_HOOK( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
	TstVoipCfg stVoipCfg;
	int ret;

	// for real DAA off-hook(channel is FXO channel)
	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward 
#else
	voip_event_flush_dtmf_fifo(stVoipCfg.ch_id, 0);//To avoid error events from other phone of the same PSTN line
	voip_event_flush_dtmf_fifo(stVoipCfg.ch_id, 1);
	DAA_Off_Hook(stVoipCfg.ch_id);
#endif
	PRINT_MSG("VOIP_MGR_FXO_OFF_HOOK(%d)\n", stVoipCfg.ch_id);
#else
	return NO_COPY_TO_USER( cmd, seq_no );
#endif
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_GPIO
 * @brief Initialize / read / write GPIO 
 * @see VOIP_MGR_GPIO TstVoipGPIO 
 */
int do_mgr_VOIP_MGR_GPIO( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#if 1//ndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	unsigned long flags;
	int i;
#endif
	
	TstVoipGPIO stVoipGPIO;
	
	// rock 2008.1.16
	COPY_FROM_USER(&stVoipGPIO, (TstVoipGPIO *) user, sizeof(TstVoipGPIO));

#if 0//def CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	//PRINT_MSG("VOIP_MGR_GPIO: not support for Host\n");
	//stVoipGPIO.ret_val = 0;
#else

	PRINT_MSG("VOIP_MGR_GPIO: action=%d, address:0x%08X, value:0x%08X\n",
		stVoipGPIO.action, stVoipGPIO.pid, stVoipGPIO.value);

	i = -1;
	switch (stVoipGPIO.action)
	{
	case 0: // init
		save_flags(flags); cli();
		i = RTK_GPIO_INIT(
			stVoipGPIO.pid,
			stVoipGPIO.value & 0x03,			// GPIO_PERIPHERAL
			stVoipGPIO.value >> 2 & 0x01,		// GPIO_DIRECTION
			stVoipGPIO.value >> 3 & 0x01		// GPIO_INTERRUPT_TYPE
		);
		restore_flags(flags);
		break;
	case 1: // read
		save_flags(flags); cli();
		i = RTK_GPIO_GET(
			stVoipGPIO.pid,
			&stVoipGPIO.value
		);
		restore_flags(flags);
		break;
	case 2: // write
		save_flags(flags); cli();
		i = RTK_GPIO_SET(
			stVoipGPIO.pid,
			stVoipGPIO.value
		);
		restore_flags(flags);
		break;
	default:
		printk("VOIP_MGR_GPIO: unknown action\n");
		break;
	}
	stVoipGPIO.result = i;
#endif
	return COPY_TO_USER(user, &stVoipGPIO, sizeof(TstVoipGPIO), cmd, seq_no);
	//return 0;
}


/**
 * @brief Structure for LED display  
 * @param ch_id Channel ID 
 * @param led_id Each channel has at most 2 LED, and we use 0 normally. 
 * @param mode LED display mode. It may be off/on/blinking 
 * @see VOIP_MGR_SET_LED_DISPLAY TstVoipLedDisplay LedDisplayMode
 */
int do_mgr_VOIP_MGR_SET_LED_DISPLAY( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipLedDisplay stVoipLedDisplay;
	int ret;
	ioc_mode_t ioc_mode;
	ioc_state_t ioc_state;	
	
	COPY_FROM_USER(&stVoipLedDisplay, (TstVoipLedDisplay *) user, sizeof(TstVoipLedDisplay));
	
	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;	
	
	// mode convert 
	switch( stVoipLedDisplay.mode )	{
	case LED_OFF:
		ioc_mode = IOC_MODE_NORMAL;
		ioc_state = IOC_STATE_LED_OFF;
		break;
	
	case LED_ON:
		ioc_mode = IOC_MODE_NORMAL;
		ioc_state = IOC_STATE_LED_ON;
		break;
		
	case LED_BLINKING:
		ioc_mode = IOC_MODE_BLINKING;
		ioc_state = IOC_STATE_LED_ON;
		break;
		
	default:
		return -1;
	}
	
	return Set_LED_Display_cch( stVoipLedDisplay.ch_id, 
								stVoipLedDisplay.led_id, 
								ioc_mode, ioc_state );
}

/**
 * @brief Structure for SLIC relay 
 * @param ch_id Channel ID 
 * @param close A boolean value to indicate relay is close(1) or open(0)
 * @see VOIP_MGR_SET_SLIC_RELAY TstVoipSlicRelay
 */
int do_mgr_VOIP_MGR_SET_SLIC_RELAY( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipSlicRelay stVoipSlicRelay;
	int ret;
	ioc_state_t ioc_state;	
	
	COPY_FROM_USER(&stVoipSlicRelay, (TstVoipSlicRelay *) user, sizeof(TstVoipSlicRelay));
	
	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;	
	
	// state convert 
	if( stVoipSlicRelay.close )
		ioc_state = IOC_STATE_RELAY_CLOSE;
	else
		ioc_state = IOC_STATE_RELAY_OPEN;
	
	return Set_SLIC_Relay_cch( stVoipSlicRelay.ch_id, ioc_state );
}

/**
 * @ingroup VOIP_DRIVER_NETWORK
 * @brief Control 8305 / 8306 switch 
 * @see VOIP_MGR_8305_SWITCH_VAL TstVoipSwitch 
 */
int do_mgr_VOIP_MGR_8305_SWITCH_VAL( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef CONFIG_RTK_VOIP_DRIVERS_8186V_ROUTER
	unsigned long flags;
	TstVoipSwitch switch_value;

	copy_from_user(&switch_value, (TstVoipSwitch *)user, sizeof(TstVoipSwitch));
	save_flags(flags); cli();
	if (switch_value.read_write == 0)
		MII_read(switch_value.phy,switch_value.reg,0);
	else if (switch_value.read_write == 1)
		MII_write(switch_value.phy,switch_value.reg,switch_value.value,0);
#ifdef _8305SC_
	else if (switch_value.read_write == 2)
		rtl8305_init();
#endif
	else if (switch_value.read_write == 3)
		rtl8306SD_init();
	else if (switch_value.read_write == 4)
		rtl8306_debug();
	else if (switch_value.read_write == 5)
		rtl8306_reset();
#ifdef _8305SC_
	else if (switch_value.read_write == 6)
		rtl8305_wan_vlan(0x6ab,0x05,1);
#endif
	else if (switch_value.read_write == 7)
		rtl8306_wan_vlan(0x6ab,0x05,1);
#if 0
#ifdef _8305SC_
	else if (switch_value.read_write == 8)
		rtl8305_WAN_LAN_VLAN_TAG(0x6ab,0x7cd);
#endif
	else if (switch_value.read_write == 9)
		rtl8306_WAN_LAN_VLAN_TAG(0x6ab,0x7cd);
#endif
#ifdef _8305SC_
	else if (switch_value.read_write == 10) {
		rtl8305_restore_switch();
		external_mode = 1;
	}
#endif
	else if (switch_value.read_write == 11) {
		rtl8306_restore_switch();
		external_mode = 1;
	}
	/************ For Testing ************/
	else if (switch_value.read_write == 12)
		print_pcm();
#ifdef CONFIG_RTK_VOIP_DRIVERS_SI3050
	else if (switch_value.read_write == 13)
		going_off_hook(0);
	else if (switch_value.read_write == 14)
		DAA_On_Hook(0);
#endif
	/**************************************/
	else if (switch_value.read_write == 21)
		rtl8306_bandwidth_control(4000,1);
	else if (switch_value.read_write == 22)
		rtl8306_bandwidth_control(8000,0);
	else if (switch_value.read_write == 23)
		rtl8306_QoS_restore();
	else if (switch_value.read_write == 24)
		rtl8306_QoS_mechanism();
	else if (switch_value.read_write == 25) {
		//rtl8306_wan_multi_vlan(0x6ab,0x7cd);
		rtl8306_wan_2_vlan(
					1707, 5, 0,
					1997, 0, 0);
	} else if (switch_value.read_write == 26)
                    rtl8306_wan_3_vlan(
					1707, 7, 0,
					1997, 0, 0,
					2287, 5, 0);
	else if (switch_value.read_write == 27)
		rtl8306_setAsicQosPortRate(5,0x9c,1,1);
	else if (switch_value.read_write == 28)
		rtl8306_setAsicQosPortRate(5,0x9c,1,0);
	else if (switch_value.read_write == 29)
		rtl8306_setAsicQosPortRate(0,0x4e,0,1);
	else if (switch_value.read_write == 30)
		rtl8306_setAsicQosPortRate(0,0x4e,0,0);
	else if (switch_value.read_write == 31)
		rtl8306_vlan_dump();

	restore_flags(flags);
#endif
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_NETWORK
 * @brief Control 8305 / 8306 VLAN tag  
 * @see VOIP_MGR_WAN_VLAN_TAG TstVoipSwitch_VLAN_tag 
 */
int do_mgr_VOIP_MGR_WAN_VLAN_TAG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	unsigned long flags;
	TstVoipSwitch_VLAN_tag wan_vlan_tag;

	copy_from_user(&wan_vlan_tag, (TstVoipSwitch_VLAN_tag *)user, sizeof(TstVoipSwitch_VLAN_tag));
#if 1 // debug only
	PRINT_MSG("wan_vlan_tag.enable =%d\n", wan_vlan_tag.enable);
	PRINT_MSG("wan_vlan_tag.vlanId =%d\n", wan_vlan_tag.vlanId);
	PRINT_MSG("wan_vlan_tag.priority =%d\n", wan_vlan_tag.priority);
	PRINT_MSG("wan_vlan_tag.cfi =%d\n", wan_vlan_tag.cfi);
#endif
	save_flags(flags); cli();
	if (external_mode == 0) {
		if (wan_vlan_tag.enable == 1)
		{
		#ifdef CONFIG_RTK_VOIP_DRIVERS_8186V_ROUTER
		#ifdef _8305SC_
			rtl8305_wan_vlan(wan_vlan_tag.vlanId,wan_vlan_tag.priority,wan_vlan_tag.cfi);
		#endif
		#ifdef _8306SD_
			rtl8306_wan_vlan(wan_vlan_tag.vlanId,wan_vlan_tag.priority,wan_vlan_tag.cfi);
		#endif
		#endif
		} else // if (wan_vlan_tag.enable == 2)
		{
		#ifdef CONFIG_RTK_VOIP_DRIVERS_8186V_ROUTER
		#ifdef _8305SC_
			rtl8305_init();
		#endif
		#ifdef _8306SD_
			rtl8306SD_init();
		#endif
		#endif
		}
	}
	restore_flags(flags);
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_NETWORK
 * @brief Control 8305 / 8306 bridge mode 
 * @param bridge_mode An 'unsigned char' variable. (1: restore switch, 2: initial)
 * @see VOIP_MGR_BRIDGE_MODE  
 */
int do_mgr_VOIP_MGR_BRIDGE_MODE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef CONFIG_RTK_VOIP_DRIVERS_8186V_ROUTER
	unsigned long flags;
	unsigned char bridge_mode;

	copy_from_user(&bridge_mode, (unsigned char *)user, sizeof(unsigned char));
	save_flags(flags); cli();
	if (bridge_mode == 1)
	{
	#ifdef _8305SC_
		rtl8305_restore_switch();
	#endif
	#ifdef _8306SD_
		rtl8306_restore_switch();
	#endif
	} else if (bridge_mode == 2)
	{
	#ifdef _8305SC_
		rtl8305_init();
	#endif
	#ifdef _8306SD_
		rtl8306SD_init();
	#endif
	}
	restore_flags(flags);
#endif /*CONFIG_RTK_VOIP_DRIVERS_8186V_ROUTER*/
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_NETWORK
 * @brief Configure DSCP    
 * @param dscp An 'int' variable. <br>
 *        bit 0-7: RTP DSCP <br> 
 *        bit 7-15: SIP DSCP <br>
 * @see VOIP_MGR_SET_DSCP_PRIORITY  
 */
int do_mgr_VOIP_MGR_SET_DSCP_PRIORITY( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef SUPPORT_DSCP
#ifdef CONFIG_DEFAULTS_KERNEL_2_6
#ifdef CONFIG_RTK_VOIP_QOS
	extern int32 rtl8651_setAsicDscpPriority( uint32 dscp, uint32 priority);
	extern int32 rtl8651_reset_dscp_priority( void );
	extern int voip_qos;
	int value;
	int dscp, priority;
	
	copy_from_user(&value, (int *)user, sizeof(int));

	dscp = (value & 0xFF00)>>8;
	priority = (value & 0xFF);

	if(priority == 0xFF){
		PRINT_MSG("reset all dscp priority to 0 \n");
		rtl8651_reset_dscp_priority();
	}
	else{
		if(voip_qos & VOIP_QOS_RX_HW_HIGH_QUEUE)
		{
			PRINT_MSG("dscp = 0x%x, priority = %d\n", dscp, priority);
			rtl8651_setAsicDscpPriority(dscp, priority);
		}
	}
#endif 
#else // CONFIG_DEFAULTS_KERNEL_2_6
#ifdef CONFIG_RTK_VOIP_DRIVERS_8186V_ROUTER
	rtl8306_set_DSCP(sip_dscp,rtp_dscp);
#endif
#ifdef CONFIG_RTK_VOIP_865xC_QOS
	#if 0
	extern void rtl865xC_set_DSCP(int SIP, int RTP);
	rtl865xC_set_DSCP(sip_dscp, rtp_dscp);
	#else
	PRINT_MSG("%s is not supported.\n", VOIP_MGR_SET_DSCP_PRIORITY);
	#endif	
#endif
#endif
#endif
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_NETWORK
 * @brief Configure second VLAN tag    
 * @see VOIP_MGR_WAN_2_VLAN_TAG TstVoipSwitch_2_VLAN_tag
 */
int do_mgr_VOIP_MGR_WAN_2_VLAN_TAG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	unsigned long flags;
	TstVoipSwitch_2_VLAN_tag wan_2_vlan_tag;
	
	copy_from_user(&wan_2_vlan_tag, (TstVoipSwitch_2_VLAN_tag *)user, sizeof(TstVoipSwitch_2_VLAN_tag));
#if 1 // debug only
	PRINT_MSG("wan_2_vlan_tag.enable =%d\n", wan_2_vlan_tag.enable);
	PRINT_MSG("wan_2_vlan_tag.vlanIdVoice =%d\n", wan_2_vlan_tag.vlanIdVoice);
	PRINT_MSG("wan_2_vlan_tag.priorityVoice =%d\n", wan_2_vlan_tag.priorityVoice);
	PRINT_MSG("wan_2_vlan_tag.cfiVoice =%d\n", wan_2_vlan_tag.cfiVoice);
	PRINT_MSG("wan_2_vlan_tag.vlanIdData =%d\n", wan_2_vlan_tag.vlanIdData);
	PRINT_MSG("wan_2_vlan_tag.priorityData =%d\n", wan_2_vlan_tag.priorityData);
	PRINT_MSG("wan_2_vlan_tag.cfiData =%d\n", wan_2_vlan_tag.cfiData);
#endif
	save_flags(flags); cli();
	if (external_mode == 0) {
		if (wan_2_vlan_tag.enable == 1)
		{
		#ifdef CONFIG_RTK_VOIP_DRIVERS_8186V_ROUTER
			rtl8306_wan_2_vlan(
				wan_2_vlan_tag.vlanIdVoice, wan_2_vlan_tag.priorityVoice, wan_2_vlan_tag.cfiVoice,
				wan_2_vlan_tag.vlanIdData, wan_2_vlan_tag.priorityData, wan_2_vlan_tag.cfiData
			);
		#endif
		} else // if (wan_2_vlan_tag.enable == 2)
		{
		#ifdef CONFIG_RTK_VOIP_DRIVERS_8186V_ROUTER
			rtl8306SD_init();
		#endif
		}
	}
	restore_flags(flags);
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_NETWORK
 * @brief Configure third VLAN tag    
 * @see VOIP_MGR_WAN_3_VLAN_TAG TstVoipSwitch_3_VLAN_tag
 */
int do_mgr_VOIP_MGR_WAN_3_VLAN_TAG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	extern void rtl865xC_wan_3_vlan(
            unsigned int id_proto,
            unsigned int priority_proto,
            unsigned int cfi_proto,
            unsigned int id_data,
            unsigned int priority_data,
            unsigned int cfi_data,
            unsigned int id_video,
            unsigned int priority_video,
            unsigned int cfi_video
            );
	extern void rtl865xC_vlan_init( void );
	
	unsigned long flags;
	TstVoipSwitch_3_VLAN_tag wan_3_vlan_tag;

	copy_from_user(&wan_3_vlan_tag, (TstVoipSwitch_3_VLAN_tag *)user, sizeof(TstVoipSwitch_3_VLAN_tag));
	PRINT_MSG("wan_3_vlan_tag.enable =%d\n", wan_3_vlan_tag.enable);
	PRINT_MSG("wan_3_vlan_tag.vlanIdVoice =%d\n", wan_3_vlan_tag.vlanIdVoice);
	PRINT_MSG("wan_3_vlan_tag.priorityVoice =%d\n", wan_3_vlan_tag.priorityVoice);
	PRINT_MSG("wan_3_vlan_tag.cfiVoice =%d\n", wan_3_vlan_tag.cfiVoice);
	PRINT_MSG("wan_3_vlan_tag.vlanIdData =%d\n", wan_3_vlan_tag.vlanIdData);
	PRINT_MSG("wan_3_vlan_tag.priorityData =%d\n", wan_3_vlan_tag.priorityData);
	PRINT_MSG("wan_3_vlan_tag.cfiData =%d\n", wan_3_vlan_tag.cfiData);
	PRINT_MSG("wan_3_vlan_tag.vlanIdVideo =%d\n", wan_3_vlan_tag.vlanIdVideo);
	PRINT_MSG("wan_3_vlan_tag.priorityVideo =%d\n", wan_3_vlan_tag.priorityVideo);
	PRINT_MSG("wan_3_vlan_tag.cfiVideo =%d\n", wan_3_vlan_tag.cfiVideo);

	save_flags(flags); cli();
	if (external_mode == 0) {
		if (wan_3_vlan_tag.enable == 1)
		{
		#ifdef CONFIG_RTK_VOIP_DRIVERS_8186V_ROUTER
			rtl8306_wan_3_vlan(
				wan_3_vlan_tag.vlanIdVoice, wan_3_vlan_tag.priorityVoice, wan_3_vlan_tag.cfiVoice,
				wan_3_vlan_tag.vlanIdData, wan_3_vlan_tag.priorityData, wan_3_vlan_tag.cfiData,
				wan_3_vlan_tag.vlanIdVideo, wan_3_vlan_tag.priorityVideo, wan_3_vlan_tag.cfiVideo
			);
		#endif
		#if defined(CONFIG_RTL865XC) && defined(CONFIG_RTK_VOIP_WAN_VLAN)
			rtl865xC_wan_3_vlan(
				wan_3_vlan_tag.vlanIdVoice, wan_3_vlan_tag.priorityVoice, wan_3_vlan_tag.cfiVoice,
				wan_3_vlan_tag.vlanIdData, wan_3_vlan_tag.priorityData, wan_3_vlan_tag.cfiData,
				wan_3_vlan_tag.vlanIdVideo, wan_3_vlan_tag.priorityVideo, wan_3_vlan_tag.cfiVideo
			);
		#endif
			
		} 
		else
		{
		#ifdef CONFIG_RTK_VOIP_DRIVERS_8186V_ROUTER
			rtl8306SD_init();
		#endif
		#if defined(CONFIG_RTL865XC) && defined(CONFIG_RTK_VOIP_WAN_VLAN)
			rtl865xC_vlan_init();
		#endif
			
		}
	}
	restore_flags(flags);
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_NETWORK
 * @brief Set WAN's clone MAC     
 * @see VOIP_MGR_SET_WAN_CLONE_MAC TstVoipCloneMAC
 */
int do_mgr_VOIP_MGR_SET_WAN_CLONE_MAC( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
//added by Tim 1/8/2008 CLONE_MAC
#ifdef CONFIG_RTK_VOIP_CLONE_MAC
	unsigned long flags;
	TstVoipCloneMAC stVoipCloneMAC;
	
	copy_from_user(&stVoipCloneMAC, (TstVoipCloneMAC *)user, sizeof(TstVoipCloneMAC));
	PRINT_MSG("CONFIG_RTK_VOIP_CLONE_MAC: MAC = %x%x%x%x%x%x\n",
		(unsigned char*)stVoipCloneMAC.CloneMACAddress[0],
		(unsigned char*)stVoipCloneMAC.CloneMACAddress[1],
		(unsigned char*)stVoipCloneMAC.CloneMACAddress[2],
		(unsigned char*)stVoipCloneMAC.CloneMACAddress[3],
		(unsigned char*)stVoipCloneMAC.CloneMACAddress[4],
		(unsigned char*)stVoipCloneMAC.CloneMACAddress[5]);
	save_flags(flags); cli();
	rtl8306SD_CloneMac((void*)stVoipCloneMAC.CloneMACAddress);
	restore_flags(flags);
#endif
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_NETWORK
 * @brief Configure bandwith      
 * @see VOIP_MGR_BANDWIDTH_MGR TstVoipBandwidthMgr
 */
int do_mgr_VOIP_MGR_BANDWIDTH_MGR( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	extern int32 rtl8651_setAsicPortIngressBandwidth( int /*enum PORTID*/ port, uint32 bandwidth );
	extern int32 rtl8651_setAsicPortEgressBandwidth( int /*enum PORTID*/ port, uint32 bandwidth );
	
#ifdef CONFIG_RTK_VOIP_QOS
	extern int voip_qos;
	extern int32 rtl8651_cpu_tx_fc(int enable);
#endif
	unsigned long flags;
	TstVoipBandwidthMgr stVoipBandwidthMgr;
	
	copy_from_user(&stVoipBandwidthMgr, (TstVoipBandwidthMgr *)user, sizeof(TstVoipBandwidthMgr));
	/*PRINT_MSG("VOIP_MGR_BANDWIDTH_MGR: param = %d %d %d\n",
	stVoipBandwidthMgr.port,
	stVoipBandwidthMgr.dir,
	stVoipBandwidthMgr.ban);*/

	save_flags(flags); cli();
#if defined (CONFIG_RTL865XC) || defined(CONFIG_RTL_819X)
	if(stVoipBandwidthMgr.dir == 0)//ingress
	{
		rtl8651_setAsicPortIngressBandwidth(stVoipBandwidthMgr.port, stVoipBandwidthMgr.ban);//disable bandwidth mgr
	}
	else //egress
	{
		if(stVoipBandwidthMgr.ban == 0)
			rtl8651_setAsicPortEgressBandwidth(stVoipBandwidthMgr.port, 0x3fff);//disable bandwidth mgr
		else
			//Rate = (1+ban)*64kbps, unlimit: ban = 0x3fff
			rtl8651_setAsicPortEgressBandwidth(stVoipBandwidthMgr.port, stVoipBandwidthMgr.ban-1);
	#ifdef CONFIG_DEFAULTS_KERNEL_2_6
		#ifdef CONFIG_RTK_VOIP_QOS
		#ifdef CONFIG_RTL_819X
			if(stVoipBandwidthMgr.port == 4 )
		#else //CONFIG_RTL865XC
			if(stVoipBandwidthMgr.port == 0 )
		#endif
			{
				if((voip_qos&VOIP_QOS_TX_DISABLE_FC)&&stVoipBandwidthMgr.ban < 161 && stVoipBandwidthMgr.ban > 0)
				{
					//turn off WAN port Queue 0 flow control to avoid voip packet dropped
					//rtl8651_setAsicQueueFlowControlConfigureRegister(0,0,0);
					rtl8651_cpu_tx_fc(0);
				}
				else
				{
					//turn on WAN port Queue 0 flow control
					//rtl8651_setAsicQueueFlowControlConfigureRegister(0,0,1);
					rtl8651_cpu_tx_fc(1);
				}
			}
		#endif

	#else
		#ifdef CONFIG_RTK_VOIP_865xC_QOS
			extern void Enable_VOIP_QoS(int enable);
			#ifdef	CONFIG_FIX_WAN_TO_4
				if(stVoipBandwidthMgr.port == 4 )
				{
					if(stVoipBandwidthMgr.ban < 40)
						Enable_VOIP_QoS(1);
					else
						Enable_VOIP_QoS(0);
				}	
			#else
				if(stVoipBandwidthMgr.port == 0 )
				{
					if(stVoipBandwidthMgr.ban < 40 && stVoipBandwidthMgr.ban > 0)
						Enable_VOIP_QoS(1);
					else
						Enable_VOIP_QoS(0);
				}
			#endif
		#endif
	#endif
	}
#endif
	restore_flags(flags);
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_NETWORK
 * @brief Configure port disable 
 * @see VOIP_MGR_PORT_DISABLE TstVoipPortConfig
 */
int do_mgr_VOIP_MGR_PORT_DISABLE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#if !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC) && !defined (CONFIG_RTK_VOIP_DRIVERS_PCM8672) && !defined (CONFIG_RTK_VOIP_DRIVERS_PCM8676)
	extern int32 rtl8651_setAsicEthernetPHYPowerDown( uint32 port, uint32 pwrDown );
	TstVoipPortConfig stVoipPortConfig;

	copy_from_user(&stVoipPortConfig, (TstVoipPortConfig *)user, sizeof(TstVoipPortConfig));

	rtl8651_setAsicEthernetPHYPowerDown(stVoipPortConfig.port,stVoipPortConfig.config);
#endif
	return 0;	
}

/**
 * @ingroup VOIP_DRIVER_NETWORK
 * @brief Configure high priority port 
 * @see VOIP_MGR_PORT_PRIORITY TstVoipPortConfig
 */
int do_mgr_VOIP_MGR_PORT_PRIORITY( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#if !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC) && !defined (CONFIG_RTK_VOIP_DRIVERS_PCM8672) && !defined (CONFIG_RTK_VOIP_DRIVERS_PCM8676) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxD)
	extern int32 rtl8651_setAsicPortPriority( uint32 port, uint32 priority );
	TstVoipPortConfig stVoipPortConfig;
	copy_from_user(&stVoipPortConfig, (TstVoipPortConfig *)user, sizeof(TstVoipPortConfig));
	rtl8651_setAsicPortPriority(stVoipPortConfig.port,stVoipPortConfig.config);     
#endif
        return 0;
}

/**
 * @ingroup VOIP_DRIVER_NETWORK
 * @brief Configure port flow control 
 * @see VOIP_MGR_PORT_DISABLE_FLOWCONTROL TstVoipPortConfig
 */
int do_mgr_VOIP_MGR_PORT_DISABLE_FLOWCONTROL( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef CONFIG_RTL865X_LAYERED_ASIC_DRIVER
	extern int32 rtl8651_setPortFlowControlConfigureRegister(uint32 port,uint32 enable);
	TstVoipPortConfig stVoipPortConfig;
	copy_from_user(&stVoipPortConfig, (TstVoipPortConfig *)user, sizeof(TstVoipPortConfig));

	if( stVoipPortConfig.config)
		rtl8651_setPortFlowControlConfigureRegister(stVoipPortConfig.port,0);
	else
		rtl8651_setPortFlowControlConfigureRegister(stVoipPortConfig.port,1);
#endif
	return 0;	

}

/**
 * @ingroup VOIP_DRIVER_NETWORK
 * @brief Get port link status 
 * @see VOIP_MGR_GET_PORT_LINK_STATUS TstVoipPortLinkStatus
 */
int do_mgr_VOIP_MGR_GET_PORT_LINK_STATUS( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef CONFIG_RTK_VOIP_DRIVERS_8186V_ROUTER
	extern int32 rtl8306_getPHYLinkStatus(uint32 phy, uint32 *linkUp);
#elif (defined(CONFIG_RTL865XC)|| defined(CONFIG_RTL_819X) )&& defined (CONFIG_RTK_VOIP_PORT_LINK)
	extern int32 rtl8651_getAsicEthernetLinkStatus(uint32 port, uint8 *linkUp);
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_8186V_ROUTER
	uint32 temp;
#elif (defined(CONFIG_RTL865XC)|| defined(CONFIG_RTL_819X) )&& defined (CONFIG_RTK_VOIP_PORT_LINK)
	extern int32 rtl8651_getAsicEthernetLinkStatus(uint32 port, uint8 *linkUp);
#endif
#if defined( CONFIG_RTK_VOIP_DRIVERS_8186V_ROUTER ) || defined( CONFIG_RTL865XC ) ||  defined(CONFIG_RTL_819X)
	int i;
	uint8 linkstatus;
#endif
	TstVoipPortLinkStatus stVoipPortLinkStatus;

  	copy_from_user(&stVoipPortLinkStatus, (TstVoipPortLinkStatus *)user, sizeof(TstVoipPortLinkStatus));
	//PRINT_MSG( "VOIP_MGR_GET_PORT_LINK_STATUS\n" );
		
#ifdef CONFIG_RTK_VOIP_DRIVERS_8186V_ROUTER
	stVoipPortLinkStatus.status = 0;
	
	/* LAN detection */
	for( i = 0; i < 4; i ++ ) {
		rtl8306_getPHYLinkStatus(i, &temp);
		
		if( temp )
			stVoipPortLinkStatus.status |= ( PORT_LINK_LAN0 << i );
	}
	
	/* WAN detection */
	rtl8306_getPHYLinkStatus( 4, &temp );
	
	if( temp )
		stVoipPortLinkStatus.status |= PORT_LINK_WAN;
#elif (defined(CONFIG_RTL865XC)|| defined(CONFIG_RTL_819X) )&& defined (CONFIG_RTK_VOIP_PORT_LINK)
	stVoipPortLinkStatus.status = 0;

	/* LAN detection */
  #if defined(CONFIG_RTL_819X) && !defined (CONFIG_RTL_89xxD)
	for( i = 0; i <= 3; i ++ ) 
  #else
	for( i = 1; i <= 4; i ++ ) 
  #endif
	{
		rtl8651_getAsicEthernetLinkStatus(i, &linkstatus);
		if( linkstatus )
			stVoipPortLinkStatus.status |= ( PORT_LINK_LAN0 << i );
	}

	/* WAN detection */
  #if defined(CONFIG_RTL_819X) && !defined (CONFIG_RTL_89xxD)
	rtl8651_getAsicEthernetLinkStatus( 4, &linkstatus );
  #else
	rtl8651_getAsicEthernetLinkStatus( 0, &linkstatus );
  #endif
	if( linkstatus )
		stVoipPortLinkStatus.status |= PORT_LINK_WAN;
#else
	stVoipPortLinkStatus.status = PORT_LINK_LAN_ALL | PORT_LINK_WAN;
	
	PRINT_MSG( "I don't know how to detect network status\n" );
#endif		
  	copy_to_user( user, &stVoipPortLinkStatus, sizeof(TstVoipPortLinkStatus));

	return 0;
}
	
/**
 * @ingroup VOIP_DRIVER_NETWORK
 * @brief Set RTP TOS 
 * @param tos An 'int' variable. <br>
 *        RTP tos = tos << 2 <br>
 * @see VOIP_MGR_SET_RTP_TOS 
 */
int do_mgr_VOIP_MGR_SET_RTP_TOS( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef SUPPORT_DSCP
	int tos;
	copy_from_user(&tos, (int *)user, sizeof(int));
	PRINT_MSG("rtp_tos = 0x%x\n", tos);
	rtp_tos = tos;
#ifdef CONFIG_RTK_VOIP_QOS_HEAVYLOAD
        extern int rtk_voip_update_setting(void);
        rtk_voip_update_setting();
#endif
#endif
	return 0;
}
/**
 * @ingroup VOIP_DRIVER_NETWORK
 * @brief Set RTP DSCP 
 * @param dscp An 'int' variable.
 * @see VOIP_MGR_SET_RTP_TOS 
 */
int do_mgr_VOIP_MGR_SET_RTP_DSCP( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef SUPPORT_DSCP
	int dscp;
	copy_from_user(&dscp, (int *)user, sizeof(int));
	PRINT_MSG("rtp_dscp = 0x%x\n", dscp);
	rtp_tos = dscp << 2;
#ifdef CONFIG_RTK_VOIP_QOS_HEAVYLOAD
        extern int rtk_voip_update_setting(void);
        rtk_voip_update_setting();
#endif
#endif
	return 0;
}
	
/**
 * @ingroup VOIP_DRIVER_NETWORK
 * @brief inform kernel SIP TOS 
 * @param tos An 'int' variable. <br>
 *        RTP tos = tos << 2 <br>
 * @see VOIP_MGR_SET_SIP_TOS 
 */
int do_mgr_VOIP_MGR_SET_SIP_TOS( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef SUPPORT_DSCP
	int tos;
	copy_from_user(&tos, (int *)user, sizeof(int));
	PRINT_MSG("sip_tos = 0x%x\n", tos);
	sip_tos = tos;
#ifdef CONFIG_RTK_VOIP_QOS_HEAVYLOAD
        extern int rtk_voip_update_setting(void);
        rtk_voip_update_setting();
#endif
#endif
	return 0;
}
/**
 * @ingroup VOIP_DRIVER_NETWORK
 * @brief inform kernel SIP DSCP 
 * @param dscp An 'int' variable.
 * @see VOIP_MGR_SET_SIP_TOS 
 */
int do_mgr_VOIP_MGR_SET_SIP_DSCP( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef SUPPORT_DSCP
	int dscp;
	copy_from_user(&dscp, (int *)user, sizeof(int));
	PRINT_MSG("sip_dscp = 0x%x\n", dscp);
	sip_tos = dscp << 2;
#ifdef CONFIG_RTK_VOIP_QOS_HEAVYLOAD
	extern int rtk_voip_update_setting(void);
	rtk_voip_update_setting();
#endif
#endif
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_DECT
 * @brief Set DECT power 
 * @param TstVoipSingleValue.value Value for power GPIO, 0: active, 1: inactive
 * @see VOIP_MGR_DECT_SET_POWER TstVoipSingleValue
 */
int do_mgr_VOIP_MGR_DECT_SET_POWER( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef CONFIG_RTK_VOIP_DRIVERS_ATA_DECT
	TstVoipSingleValue stVoipSingleValue;
	
	copy_from_user( &stVoipSingleValue, (int *)user, sizeof( TstVoipSingleValue ) );
	
	voip_dect_ctrl_set_power( stVoipSingleValue.value );
	
	PRINT_MSG( "DECT-power=%d\n", stVoipSingleValue.value );
#endif
	return 0;
}

/**
 * @ingroup VOIP_DRIVER_DECT
 * @brief Get DECT power 
 * @param TstVoipSingleValue.value Value for power GPIO, 0: active, 1: inactive
 * @see VOIP_MGR_DECT_GET_POWER TstVoipSingleValue
 */
int do_mgr_VOIP_MGR_DECT_GET_POWER( int cmd, void *user, unsigned int len, unsigned short seq_no )
{	
#ifdef CONFIG_RTK_VOIP_DRIVERS_ATA_DECT
	TstVoipSingleValue stVoipSingleValue;

	stVoipSingleValue.value = voip_dect_ctrl_get_power();

  	copy_to_user( user, &stVoipSingleValue, sizeof(TstVoipSingleValue));
  	
  	PRINT_MSG( "DECT-power=%d\n", stVoipSingleValue.value );
#endif

	return 0;
}

/**
 * @ingroup VOIP_DRIVER_DECT
 * @brief Get DECT page 
 * @param TstVoipSingleValue.value Value for page GPIO, 0: active, 1: inactive
 * @see VOIP_MGR_DECT_GET_PAGE TstVoipSingleValue
 */
int do_mgr_VOIP_MGR_DECT_GET_PAGE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{	
#ifdef CONFIG_RTK_VOIP_DRIVERS_ATA_DECT
	TstVoipSingleValue stVoipSingleValue;

	stVoipSingleValue.value = voip_dect_ctrl_get_page();

  	copy_to_user( user, &stVoipSingleValue, sizeof(TstVoipSingleValue));
  	
  	//PRINT_MSG( "DECT-page=%d\n", stVoipSingleValue.value );
#endif

	return 0;
}

/**
 * @ingroup VOIP_DRIVER_DECT
 * @brief Set DECT LED
 * @param TstVoipValue.ch_id Channel ID
 * @param TstVoipValue.value  Value for set LED, 0: off, 1: on, 2:blinking
 * @see VOIP_MGR_DECT_SET_LED TstVoipSingleValue
 */
int do_mgr_VOIP_MGR_DECT_SET_LED( int cmd, void *user, unsigned int len, unsigned short seq_no )
{	
#ifdef CONFIG_RTK_VOIP_DRIVERS_ATA_DECT
	unsigned long flags;
	TstVoipValue stVoipValue;
	copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	printk("Set DECT LED, ch%d, st=%d ", stVoipValue.ch_id, stVoipValue.value);
	save_flags(flags); cli();
	voip_dect_ctrl_set_led(stVoipValue.ch_id, stVoipValue.value);
	restore_flags(flags);

	//stVoipValue.ret_val = 0;
	copy_to_user(user, &stVoipValue, sizeof(TstVoipValue));
#endif

	return 0;
}


