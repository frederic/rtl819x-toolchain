
#ifndef _RTK_VOIP_H
#define _RTK_VOIP_H

#ifdef __KERNEL__
#include <linux/config.h>
#include <linux/version.h>
#endif

//#include "voip_version.h"	// reduce dependency 

#define PCM_HANDLER_USE_TASKLET

#ifdef PCM_HANDLER_USE_TASKLET
#define SUPPORT_PCM_FIFO
#ifndef CONFIG_RTK_VOIP_MODULE
#ifndef FEATURE_DMEM_STACK_CLI //add by timlee for compile warning
#define FEATURE_DMEM_STACK_CLI
#endif
#if !defined(CONFIG_RTK_VOIP_DRIVERS_PCM865xC) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) && !defined (CONFIG_RTK_VOIP_DRIVERS_PCM89xxC) &&!defined (CONFIG_RTK_VOIP_DRIVERS_PCM8672) &&!defined (CONFIG_RTK_VOIP_DRIVERS_PCM8676) && !defined (CONFIG_RTK_VOIP_DRIVERS_PCM89xxD)/* !RTL8952/62 && !RTL8972B/82B && !RTL89xxC && !RTL89xxD */
#define VOCODER_INT			/* Voice codec is interruptable by ISR in some conditions */
#endif

#endif /* CONFIG_RTK_VOIP_MODULE */
#define PCM_HANDLER_USE_CLI
#else // !PCM_HANDLER_USE_TASKLET
#define RTP_TX_USE_TASKLET		/* To avoid wlan_tx() called when interrupt is disabled */
#endif


//#define LEC_G168_ISR_SYNC_P
#define SUPPORT_LEC_G168_ISR		/* SUPPORT_LEC_G168_ISR is definded for ATA.*/
//#define SUPPORT_AES_ISR //for DAA channel

#ifdef SUPPORT_LEC_G168_ISR
#ifndef SUPPORT_PCM_FIFO
#define SUPPORT_PCM_FIFO
#endif
#ifdef CONFIG_AUDIOCODES_VOIP
//#ifndef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
#define LEC_USE_CIRC_BUF
//#endif
#else
#define LEC_USE_CIRC_BUF		/* using circular buffer in LEC, for out-of-order tx rx isr */
#endif
#endif

#ifdef SUPPORT_PCM_FIFO			/* If SUPPORT_PCM_FIFO is defined, following items must be defined. */
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
#define PCM_FIFO_SIZE 		16	// multiple of PCM_PERIOD because of LEC process need 
#else
#define PCM_FIFO_SIZE 		10
#endif
#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM865xC) || defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) || defined (CONFIG_RTK_VOIP_DRIVERS_PCM89xxC) || defined (CONFIG_RTK_VOIP_DRIVERS_PCM8672) || defined (CONFIG_RTK_VOIP_DRIVERS_PCM8676) || (CONFIG_RTK_VOIP_DRIVERS_PCM89xxD) /* RTL8952/62 || RTL8972B/82B || RTL89xxC || RTL89xxD */
#if defined (CONFIG_AUDIOCODES_VOIP) && defined (CONFIG_RTK_VOIP_DRIVERS_IP_PHONE)
#define PCM_PERIOD		2	/* Unit: 10ms */
#else
#define PCM_PERIOD		1	/* Unit: 10ms */
#endif
#else
#define PCM_PERIOD		2	/* Unit: 10ms */
#endif
#define PCM_PERIOD_10MS_SIZE	160	/* Unit: byte */
#define TX_FIFO_START_NUM 	(PCM_PERIOD)	/* PCM_PERIOD <= TX_FIFO_START_NUM < PCM_FIFO_SIZE */

#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
#define MAX_BAND_FACTOR		2
#else
#define MAX_BAND_FACTOR		1
#endif

#define REDUCE_PCM_FIFO_MEMCPY
//#ifdef CONFIG_RTK_VOIP_G7231
#if ! defined (CONFIG_AUDIOCODES_VOIP)
/**
 * PCM_handler() consumes and produces 10-ms voice.
 * DspProcess() consumes and produces one frame every time.
 * So, DspProcess() is called only if its frame is full. 
 */
#define SUPPORT_CUT_DSP_PROCESS
#endif
//#endif
#endif


#ifdef CONFIG_RTK_VOIP_MODULE
#define SYSTEM_IMEM 0			/* 1: enable set_system_imem() in DspProcess() and G.72x codec imem will be set every frame.
                                   	   0: disable set_system_imem  and G.72x codec imem will be set once if codec doesn't change.
                                   	 */
#else
#define SYSTEM_IMEM 1			/* 1: enable set_system_imem() in DspProcess() and G.72x codec imem will be set every frame.
                                   	   0: disable set_system_imem  and G.72x codec imem will be set once if codec doesn't change.
                                   	 */
#endif

#define SUPPORT_3WAYS_AUDIOCONF
#define SUPPORT_ADJUST_JITTER
#ifdef SUPPORT_ADJUST_JITTER
  #define SUPPORT_DYNAMIC_JITTER_DELAY
  #define SUPPORT_IDEAL_MODE_JITTER_DELAY	/* Ideal mode cause minimum delay */
#endif


#define SUPPORT_COMFORT_NOISE

#ifdef SUPPORT_COMFORT_NOISE
 #define SIMPLIFIED_COMFORT_NOISE	/* for g711/g726 only */
 #define SIMPLIFIED_CN_VER	3	/* 1: all zeros, 2: plc, 3: by NoiseLevel */

 /* If not defined CONFIG_RTK_VOIP_G729AB, we can use SIMPLIFIED_COMFORT_NOISE only. */
 #if !defined( CONFIG_RTK_VOIP_G729AB ) && !defined( SIMPLIFIED_COMFORT_NOISE )
  #undef SUPPORT_COMFORT_NOISE
 #endif
#endif


#define SUPPORT_TONE_PROFILE		/* support more tone of different countries. Please refer to "voip_params.h" for detail. */
//#define COUNTRY_TONE_RESERVED

#define SUPPORT_DETECT_LONG_TERM_NO_RTP

#if 0	// for backward compatible only 
#ifndef CONFIG_RTK_VOIP_SLIC_SI32176_NR
#define CONFIG_RTK_VOIP_SLIC_SI32176_NR	0
#endif
#ifndef CONFIG_RTK_VOIP_SLIC_SI32176_CS_NR
#define CONFIG_RTK_VOIP_SLIC_SI32176_CS_NR 0
#endif
#ifndef CONFIG_RTK_VOIP_SLIC_SI32178_NR
#define CONFIG_RTK_VOIP_SLIC_SI32178_NR	0
#endif
#ifndef CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_NR
#define CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_NR	0
#endif
#ifndef CONFIG_RTK_VOIP_SLIC_SI3226_NR
#define CONFIG_RTK_VOIP_SLIC_SI3226_NR 0
#endif
#ifndef CONFIG_RTK_VOIP_SLIC_SI3226x_NR
#define CONFIG_RTK_VOIP_SLIC_SI3226x_NR 0
#endif
#ifndef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221_NR		// 2S
#define CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221_NR	0
#endif
#ifndef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88111_NR		// 1S
#define CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88111_NR	0
#endif
#ifndef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116_NR		// 1S
#define CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116_NR	0
#endif
#ifndef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316_NR		// 1S1O
#define CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316_NR	0
#endif
#ifndef CONFIG_RTK_VOIP_DECT_DSPG_HS_NR
#define CONFIG_RTK_VOIP_DECT_DSPG_HS_NR 0
#endif
#ifndef CONFIG_RTK_VOIP_DECT_SITEL_HS_NR
#define CONFIG_RTK_VOIP_DECT_SITEL_HS_NR 0
#endif
#ifndef CONFIG_RTK_VOIP_IP_PHONE_CH_NR
#define CONFIG_RTK_VOIP_IP_PHONE_CH_NR	0
#endif
#ifndef CONFIG_RTK_VOIP_DSP_DEVICE_NR		// CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
#define CONFIG_RTK_VOIP_DSP_DEVICE_NR	0
#endif
#ifndef CONFIG_RTK_VOIP_SLIC_CH_NR_PER_DSP		// CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
#define CONFIG_RTK_VOIP_SLIC_CH_NR_PER_DSP	0
#endif
#ifndef CONFIG_RTK_VOIP_DAA_CH_NR_PER_DSP		// CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
#define CONFIG_RTK_VOIP_DAA_CH_NR_PER_DSP	0
#endif
#ifndef CONFIG_RTK_VOIP_DRIVERS_MIRROR_SLIC_NR
#define CONFIG_RTK_VOIP_DRIVERS_MIRROR_SLIC_NR	0
#endif
#ifndef CONFIG_RTK_VOIP_DRIVERS_MIRROR_DAA_NR
#define CONFIG_RTK_VOIP_DRIVERS_MIRROR_DAA_NR	0
#endif
#endif

#if 0	// for backward compatible only 
#define SLIC_CH_NUM		( \
			CONFIG_RTK_VOIP_SLIC_SI32176_NR * 1	+			\
			CONFIG_RTK_VOIP_SLIC_SI32176_CS_NR * 1 +		\
			CONFIG_RTK_VOIP_SLIC_SI32178_NR * 1	+ 			\
			( CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_NR + !!CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_NR ) * 1	+ \
			CONFIG_RTK_VOIP_SLIC_SI3226_NR * 2	+			\
			CONFIG_RTK_VOIP_SLIC_SI3226x_NR * 2      +                       \
			CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221_NR * 2 +	\
			CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88111_NR * 1 +	\
			CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116_NR * 1 +	\
			CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316_NR * 1	+	\
			CONFIG_RTK_VOIP_IP_PHONE_CH_NR +				\
			CONFIG_RTK_VOIP_DSP_DEVICE_NR * CONFIG_RTK_VOIP_SLIC_CH_NR_PER_DSP +	\
			CONFIG_RTK_VOIP_DRIVERS_MIRROR_SLIC_NR			\
			)
#endif

//#if defined( SLIC_CH_NUM ) && (SLIC_CH_NUM > 4)
//#if (PCM_PERIOD == 1)
//#define PCM_PERIOD		2
//#define TX_FIFO_START_NUM 	(PCM_PERIOD)	/* PCM_PERIOD <= TX_FIFO_START_NUM < PCM_FIFO_SIZE */
//#endif
//#endif

//#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
//#define	SLIC_CH_NUM		1		/* Support PCM channel number, number range is 1~4. */
//#endif

//#if (VOIP_CH_NUM > 1)		// pkshih: comment to avoid compiler warning
//#define SIMPLIFIED_TWO_CHANNEL_729
//#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_DAA_SUPPORT
#define VIRTUAL_DAA_CH_NUM	0

#if 0	// for backward compatible only 
#define DAA_CH_NUM		( \
			CONFIG_RTK_VOIP_SLIC_SI32176_NR * 0	+			\
			CONFIG_RTK_VOIP_SLIC_SI32176_CS_NR * 0 +		\
			CONFIG_RTK_VOIP_SLIC_SI32178_NR * 1	+ 			\
			( !!CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_NR ) * 1	+ \
			CONFIG_RTK_VOIP_SLIC_SI3226_NR * 0	+			\
			CONFIG_RTK_VOIP_SLIC_SI3226x_NR * 0      +                       \
			CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221_NR * 0 +	\
			CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88111_NR * 0 +	\
			CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116_NR * 0 +	\
			CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316_NR * 1 +	\
			CONFIG_RTK_VOIP_DSP_DEVICE_NR * CONFIG_RTK_VOIP_DAA_CH_NR_PER_DSP + \
			CONFIG_RTK_VOIP_DRIVERS_MIRROR_DAA_NR			\
			)
#endif

#elif defined (CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA)
#define DAA_CH_NUM		0
#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA
#define VIRTUAL_DAA_CH_NUM	1
#elif defined (CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA_2_RELAY_SUPPORT)
#define VIRTUAL_DAA_CH_NUM	2
#endif
#else
#define DAA_CH_NUM		0
#define VIRTUAL_DAA_CH_NUM	0
#endif

#if 0	// for backward compatible only 
#define DECT_CH_NUM			( \
			CONFIG_RTK_VOIP_DECT_DSPG_HS_NR +	\
			CONFIG_RTK_VOIP_DECT_SITEL_HS_NR )
#endif

#if 1	//defined (CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) || defined (CONFIG_RTK_VOIP_DRIVERS_PCM8672) || defined (CONFIG_RTK_VOIP_DRIVERS_PCM89xxC) || defined (CONFIG_RTK_VOIP_DRIVERS_PCM8676)
//#define PCM_CH_NUM 		16		// old naming /* Support PCM channel number, Max number is 8. */
//#define MAX_VOIP_CH_NUM		16	// old naming 
#define CON_CH_NUM			CONFIG_RTK_VOIP_CON_CH_NUM		// 16
#define BUS_PCM_CH_NUM		CONFIG_RTK_VOIP_BUS_PCM_CH_NUM	// 8
#define BUS_IIS_CH_NUM		CONFIG_RTK_VOIP_BUS_IIS_CH_NUM	// 1
#ifndef CONFIG_AUDIOCODES_VOIP
#define MAX_DSP_RTK_CH_NUM	16
#define MAX_DSP_RTK_SS_NUM	( MAX_DSP_RTK_CH_NUM * 2 )
#define DSP_RTK_CH_NUM		16	// less or equal to MAX_DSP_RTK_CH_NUM
#define DSP_RTK_SS_NUM		( DSP_RTK_CH_NUM * 2 )
#else
#define MAX_DSP_AC_CH_NUM	4
#define MAX_DSP_AC_SS_NUM	( MAX_DSP_AC_CH_NUM * 2 )
#define DSP_AC_CH_NUM		4	// less or equal to MAX_DSP_AC_CH_NUM
#define DSP_AC_SS_NUM		( DSP_AC_CH_NUM * 2 )
#endif
#else
//#define PCM_CH_NUM 		4		/* Support PCM channel number, Max number is 4. */
//#define MAX_VOIP_CH_NUM		4
#define BUS_PCM_CH_NUM		4
#define MAX_RTK_DSP_CH_NUM	4
#define CON_CH_NUM			4
#endif

#ifdef CONFIG_AUDIOCODES_VOIP
#define MAX_DSP_CH_NUM		MAX_DSP_AC_CH_NUM
#define MAX_DSP_SS_NUM		MAX_DSP_AC_SS_NUM
#define DSP_CH_NUM			DSP_AC_CH_NUM
#define DSP_SS_NUM			DSP_AC_SS_NUM
#else
#define MAX_DSP_CH_NUM		MAX_DSP_RTK_CH_NUM
#define MAX_DSP_SS_NUM		MAX_DSP_RTK_SS_NUM
#define DSP_CH_NUM			DSP_RTK_CH_NUM
#define DSP_SS_NUM			DSP_RTK_SS_NUM
#endif

//#define VOIP_CH_NUM		(SLIC_CH_NUM + DAA_CH_NUM + DECT_CH_NUM)

//#define MAX_SESS_NUM 		2*MAX_VOIP_CH_NUM
//#define SESS_NUM		2*VOIP_CH_NUM

#if ! defined (CONFIG_AUDIOCODES_VOIP)

#else
//#include "acmw_userdef.h"
//#define AUDIOCODES_VOTING_MECHANISM
#define ACMWPCM_HANDLER	1
#define ACMW_PLAYBACK	// define: use AudioCodes IVR, not define: use RTK IVR
#define ACMW_MODEM_RX_BEFORE_LEC 0
/* AudioCodes recommend ACMWModemRx will be after the LEC process, in order to get echo-free input data (especially for DTMF CID)*/
#endif /*CONFIG_AUDIOCODES_VOIP*/

#define MAX_RTP_TRAP_SESSION	( 2*DSP_SS_NUM )

/* rtcp definition */
/* define SUPPORT_RTCP to support RTCP.
 * It also need to define it in voip_manger.c for user space.
 * Thlin add 2006-07-04
 */
#define SUPPORT_RTCP
#define SUPPORT_RTCP_XR	// Make DSP to support RFC3611 - RTCP XR
/* rtcp mid offset*/
#define RTCP_SID_OFFSET		( DSP_SS_NUM )


// dtmf definition
#define CH_TONE 2			/* number of channel of playtone function */
#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
#define TONE_BUFF_SIZE 480  /* Borrow to do upsampler, so 30ms is 480samples. Unit: Word16 */
#else
#define TONE_BUFF_SIZE 320  /* Unit: Word16 */
#endif

// dynamic payload & multi-frame
#define SUPPORT_DYNAMIC_PAYLOAD
#define SUPPORT_MULTI_FRAME
#define MULTI_FRAME_BUFFER_SIZE	480	/* 6 frames per packet (maximum size of one frame is 80 when g711 codec) */
#define SUPPORT_BASEFRAME_10MS		/* undefine this MACRO will not work! */
//#define SUPPORT_FORCE_VAD
#ifdef SUPPORT_DYNAMIC_PAYLOAD
//#define DYNAMIC_PAYLOAD_VER1
#endif

#if defined( SUPPORT_DYNAMIC_PAYLOAD ) && !defined( DYNAMIC_PAYLOAD_VER1 )
#define SUPPORT_APPENDIX_SID		/* some packets contain voice in font of SID */
#define RESERVE_SPACE_FOR_SLOW		/* to place out of order packet into correct position */
#endif

#define NEW_JITTER_BUFFER_WI_DESIGN
#define CLEAN_JITTER_BUFFER_PARAMS

//#define SUPPORT_CODEC_DESCRIPTOR

//#define SUPPORT_CUSTOMIZE_FRAME_SIZE	/* turn on 'frame per packet' option in web configuration */

/* ================== DTMF DETECTION ==================== */
#if ! defined (CONFIG_AUDIOCODES_VOIP)
#define DTMF_DEC_ISR
#endif

#ifdef DTMF_DEC_ISR
#define DTMF_REMOVAL_ISR
#define DTMF_REMOVAL_FORWARD
#endif

#ifdef DTMF_REMOVAL_FORWARD
#define DTMF_REMOVAL_FORWARD_SIZE 3	/* removal length is (3 + PCM_PERIOD) */
					/*
					 * Forward remove DTMF_REMOVAL_FORWARD_SIZE*10 ms.
					 * The larger size, DTMF removal more clean, but longer delay.
					 */
#define CUSTOMIZE_DTMF_MINIMUM_ON_TIME /* user can set dtmf minium on time default 30ms */
#endif

//#define DTMF_DET_PRIOR_LEC		/* local DTMF tone detect in pcm_rx prior or post LEC  */

#define DTMF_DET_DURATION_HIGH_ACCURACY

/* ==================== FAX DETECTION ==================== */
#ifdef DTMF_DEC_ISR
#define SUPPORT_FAX_PASS_ISR
#endif

/* ================= PULSE DIAL GENERATION/DETECTION ================ */
#ifdef CONFIG_RTK_VOIP_DRIVERS_DAA_SUPPORT
#define PULSE_DIAL_GEN
#define OUTBAND_AUTO_PULSE_DIAL_GEN	// auto gen pulse dial for FXO when reveive outband DTMF signal.
#endif
#define PULSE_DIAL_DET
// Note: there is no pulse dial for phone key * and #

/* ================== RFC2833 SEND =================== */
#define SUPPORT_RFC_2833
#define SUPPORT_RFC2833_PLAY_LIMIT
#define SUPPORT_RFC2833_TRANSFER

#ifdef DTMF_DEC_ISR
#define SEND_RFC2833_ISR
//#define RTP_SNED_TASKLET		/* To avoid wlan_tx() called when interrupt is disabled */
								/* Thlin: Enable rtp send tasklet to send RFC2833 packets in tasklet. */
#endif

/* ====================================================== */


/* software DTMF CID generate */
#define SW_DTMF_CID
#define FSK_TYPE2_ACK_CHECK

#define SUPPORT_USERDEFINE_TONE

#define CHANNEL_NULL	0xff
#define SESSION_NULL	0xff

#ifdef SUPPORT_3WAYS_AUDIOCONF
	#define CONF_OFF	0xff
#endif


#define EVENT_POLLING_TIMER		/* Init a timer for Hook Polling usage. Accuracy: 10 ms */

//#define SUPPORT_VOICE_QOS // replaced by SUPPORT_DSCP.
#define SUPPORT_DSCP //  Move SIP and RTP QoS setting UI to VoIP "Other" page and support dynamic DSCP settings for SIP and RTP.

#ifdef CONFIG_RTK_VOIP_DRIVERS_8186V_ROUTER
#define CONFIG_RTK_VOIP_WAN_VLAN // Support VLAN setting of WAN port on Web UI!
#define CONFIG_RTK_VOIP_CLONE_MAC // Support WAN MAC CLONE for RTL8306
//#define SUPPORT_IP_ADDR_QOS // Note: SUPPORT_IP_ADDR_QOS will casue packet lost temporarily!
#endif

//#if defined (CONFIG_RTK_VOIP_DRIVERS_PCM8186) || defined (CONFIG_RTK_VOIP_GPIO_8962) || defined (CONFIG_RTK_VOIP_DRIVERS_PCM8671) || defined( CONFIG_RTK_VOIP_GPIO_8972B ) || defined( CONFIG_RTK_VOIP_GPIO_8954C_V100) || defined( CONFIG_RTK_VOIP_GPIO_8954C_V200)
//#ifndef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
//#if !defined( CONFIG_RTK_VOIP_SLIC_NUM_8 ) || !defined( CONFIG_RTK_VOIP_DAA_NUM_8 )
//#define CONFIG_RTK_VOIP_LED	       /* V210 EV Board LED Control */
//#endif
//#endif
//#endif

#if defined (CONFIG_RTK_VOIP_DRIVERS_PCM8672) || defined (CONFIG_RTK_VOIP_DRIVERS_PCM8676)
#define VOIP_CPU_CACHE_WRITE_BACK
#endif

#if defined (CONFIG_RTK_VOIP_DRIVERS_FXO) && !defined (CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA)

/****** For SLIC and DAA Negotiation *****/
#ifdef CONFIG_RTK_VOIP_DRIVERS_DAA_SUPPORT
#define CH_NUM_DAA	1 		// The number of the DAA which can support negotiation with SLIC
#define DAA_RX_DET
#define FXO_CALLER_ID_DET		// RTK,AC middleware use the same define
#define FXO_BUSY_TONE_DET		// RTK,AC middleware use the same define
#if ! defined (CONFIG_AUDIOCODES_VOIP)
//#define SUPPORT_BANDPASS_FOR_DAA	//add 200-3400hz bandpass for DAA RX
#endif
#define FXO_RING_NO_DET_CADENCE
#define HW_FXO_REVERSAL_DET
//#define HW_FXO_BAT_DROP_OUT
//#define SW_FXO_REVERSAL_DET
#else
#define CH_NUM_DAA	0
#endif
/*****************************************/
#else
#define CH_NUM_DAA	0
#endif


/********** For voice record DEBUG ******/
//#define RTK_VOICE_RECORD
#ifdef RTK_VOICE_RECORD
#define DATAGETBUFSIZE	(10*1120)	//10*1120byte = 700*80short 700ms voice data
#define EACH_DATAGETBUFSIZE 1120
#endif //#ifdef RTK_VOICE_RECORD

/********** For voice play DEBUG ******/
//#define RTK_VOICE_PLAY
#ifdef RTK_VOICE_PLAY
#define DATAPUTBUFSIZE	(10*1120)	//10*1120byte = 700*80short 700ms voice data
#define EACH_DATAPUTBUFSIZE 1120
#define RTK_VOICE_PLAY_WAIT_CODEC_START
#endif //#ifdef RTK_VOICE_PLAY
/********** For new EC 128ms **********/
//#define CONFIG_DEFAULT_NEW_EC128	1
/********** For experimental AEC ******/
//#define EXPER_AEC
/********** For experimental NR  *****/
//#define EXPER_NR


#define OPTIMIZATION

#define ENERGY_DET_PCM_IN

//#include "voip_feature.h"
#include "voip_debug.h"
//#define SUPPORT_SLIC_GAIN_CFG // define this to enable SLIC gain config (include DTMF compensation)

#ifdef CONFIG_RTK_VOIP_T38
 #define T38_STAND_ALONE_HANDLER
#endif

#define VOIP_RESOURCE_CHECK	// Define VOIP_RESOURCE_CHECK to enable VoIP resource check. 
				// Max. resource is two encode/decode channel.
				// VOIP_RESOURCE_CHECK is only for Realtek Solution, AudioCodes always check resource(2 channel).

// pkshih: move to voip_flash.h 
//#if defined(CONFIG_RTK_VOIP_IP_PHONE) || defined(CONFIG_CWMP_TR069)
//#define SUPPORT_VOIP_FLASH_WRITE	/* flash write module */
//#endif

//#define SUPPORT_IVR_HOLD_TONE	/* Use IVR G.723 to play HOLD tone */

/* For voice gain adjust object (note: adjust object only can adjust once) */
#define VOICE_GAIN_ADJUST_IVR
#define VOICE_GAIN_ADJUST_VOICE
//#define VOICE_GAIN_ADJUST_TONE_VOICE
//#define VOICE_GAIN_ADJUST_IVR_TONE_VOICE

#define PCM_LOOP_MODE_DRIVER
//#define PCM_LOOP_MODE_CONTROL

#define SUPPORT_G722_ITU	// Support ITU Fixed G722. Note: Support G722 ITU, user need to change the dsp_r1/Makefile.
//#define SUPPORT_G722_TYPE_NN		// G722 8k mode  
#define SUPPORT_G722_TYPE_WW		// G722 16k mode  
//#define SUPPORT_G722_TYPE_WN		// G722 16k mode + resampler

#define SUPPORT_FAX_V21_DETECT	1	// support fax preamble or DIS/DCN detect

#define NEW_TONE_ENTRY_ARCH			/* provide buffer to move both local/remote tone outside */
//#define NEW_REMOTE_TONE_ENTRY		/* mix remote tone in PCM RX (recommend: disable) */
#define NEW_LOCAL_TONE_ENTRY		/* mix local tone in PCM TX (recommend: enable) */

//#ifndef CONFIG_RTK_VOIP_IP_PHONE
//#define DISABLE_NEW_REMOTE_TONE	/* disable new remote tone, so we suggest to comment it */
//#endif

//#define ANSTONE_DET_PRIOR_LEC		/* local answer tone detect in pcm_rx prior or post LEC  */

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
#define IPC_ARCH_DEBUG_HOST
#endif
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
#define IPC_ARCH_DEBUG_DSP
#endif

#ifdef __KERNEL__
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
#define save_flags(x) local_irq_save(x)
#define cli(format, ...)
#define restore_flags(x) local_irq_restore(x)
#endif
#endif

#ifndef CONFIG_AUDIOCODES_VOIP
#define SUPPORT_V152_VBD	1		// support V.152 
#endif

#define SUPPORT_RTP_REDUNDANT	1	// support RTP redundant 

#ifdef CONFIG_RTK_VOIP_G7111
#define G7111_10MS_BASE		// G.711.1 frame is 5ms. This define will see it as 10ms
#endif

#define SUPPORT_VOIP_DBG_COUNTER	// Support VoIP Debug Counter

#endif //_RTK_VOIP_H

