#ifndef __VOIP_RESOURCE_CHECK_H__
#define __VOIP_RESOURCE_CHECK_H__

#define DEFAULT_WEIGHT	0
#define G711_WEIGHT		1
#define G726_WEIGHT		2
#define G729_WEIGHT		3
#define G723_WEIGHT		3
#define GSMFR_WEIGHT		3
#define AMRNB_WEIGHT		3
#define T38_WEIGHT		3
#define ILBC_WEIGHT		4
#define SPEEXNB_WEIGHT		4
#ifdef SUPPORT_G722_TYPE_WN
#ifdef SUPPORT_G722_ITU
#define G722_WEIGHT		4 // G722 16k mode + resampler
#else
#define G722_WEIGHT		3 // G722 16k mode + resampler
#endif
#else
#define G722_WEIGHT		2 // G722 8k mode
#endif
#define G711_WB_WEIGHT	5

/* Weight for Tone, Caller ID detection */
#ifdef CONFIG_RTK_VOIP_DRIVERS_SI3050
#define FXO_WEIGHT		1	//( DAA_CH_NUM >= 3 ? 2 : 1 )
//#ifdef CONFIG_RTK_VOIP_DAA_NUM_1
//#define FXO_WEIGHT 		1
//#elif defined CONFIG_RTK_VOIP_DAA_NUM_2
//#define FXO_WEIGHT	 	1
//#elif defined CONFIG_RTK_VOIP_DAA_NUM_3
//#define FXO_WEIGHT	 	2
//#elif defined CONFIG_RTK_VOIP_DAA_NUM_4
//#define FXO_WEIGHT	 	2
//#elif defined CONFIG_RTK_VOIP_DAA_NUM_8
//#define FXO_WEIGHT	 	2
//#else
//#error
//#endif
#else
#define FXO_WEIGHT		 0
#endif

/* Different SoC, different RES_WEIGHT_THS */
#if defined (CONFIG_RTK_VOIP_DRIVERS_PCM8651)
#define RES_WEIGHT_THS	( 7 - FXO_WEIGHT )
#elif defined (CONFIG_RTK_VOIP_DRIVERS_PCM865xC)
 #ifdef CONFIG_RTK_VOIP_GPIO_8962	// 400 MHz, can support two port G.729 3-way conf.
  #define RES_WEIGHT_THS	( 13 - FXO_WEIGHT )
 #elif CONFIG_RTK_VOIP_GPIO_8651C	// 280 MHz
  #define RES_WEIGHT_THS	( 8 - FXO_WEIGHT )
 #elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8952_V00 )	// 400 MHz, can support two port G.729 3-way conf.
  #define RES_WEIGHT_THS	( 13 - FXO_WEIGHT )
 #endif
#elif defined (CONFIG_RTK_VOIP_DRIVERS_PCM89xxD)
#define RES_WEIGHT_THS	( 34 - FXO_WEIGHT )
#elif defined (CONFIG_RTK_VOIP_DRIVERS_PCM89xxC)
#define RES_WEIGHT_THS	( 32 - FXO_WEIGHT )
#elif defined (CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY)
#define RES_WEIGHT_THS	( 13 - FXO_WEIGHT )
#elif defined (CONFIG_RTK_VOIP_DRIVERS_PCM8186)
#define RES_WEIGHT_THS	( 7 - FXO_WEIGHT )
#elif defined (CONFIG_RTK_VOIP_DRIVERS_PCM8671)
#define RES_WEIGHT_THS	( 7 - FXO_WEIGHT )
#elif defined (CONFIG_RTK_VOIP_DRIVERS_PCM8672)
#define RES_WEIGHT_THS	( 13 - FXO_WEIGHT )
#elif defined (CONFIG_RTK_VOIP_DRIVERS_PCM8676)
#define RES_WEIGHT_THS	( 32 - FXO_WEIGHT )

#endif

extern int GetCurrentVoipResourceStatus(int pkt_fmt);

extern int SetVoipResourceWeight(uint32 s_id, int pltype);

#endif // __VOIP_RESOURCE_CHECK_H__


