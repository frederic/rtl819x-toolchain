#ifndef __IPC_ARCH_HELP_HOST_H__
#define __IPC_ARCH_HELP_HOST_H__

//#define HOST_CH_NUM	VOIP_CH_NUM

#if 1
//#define DSP_DEVICE_NUM	CONFIG_RTK_VOIP_DSP_DEVICE_NR
#else
#if defined CONFIG_RTK_VOIP_DSP_DEVICE_NUM_1
#define DSP_DEVICE_NUM	1
#elif defined CONFIG_RTK_VOIP_DSP_DEVICE_NUM_2
#define DSP_DEVICE_NUM	2
#elif defined CONFIG_RTK_VOIP_DSP_DEVICE_NUM_3
#define DSP_DEVICE_NUM	3
#elif defined CONFIG_RTK_VOIP_DSP_DEVICE_NUM_4
#define DSP_DEVICE_NUM	4
#endif
#endif

#if 1
//#define CH_NUM_PER_DSP	( CONFIG_RTK_VOIP_SLIC_CH_NR_PER_DSP + CONFIG_RTK_VOIP_DAA_CH_NR_PER_DSP )
#else
#if defined CONFIG_RTK_VOIP_CH_NUM_PER_DSP_1
#define CH_NUM_PER_DSP	1
#elif defined CONFIG_RTK_VOIP_CH_NUM_PER_DSP_2
#define CH_NUM_PER_DSP	2
#elif defined CONFIG_RTK_VOIP_CH_NUM_PER_DSP_3
#define CH_NUM_PER_DSP	3
#elif defined CONFIG_RTK_VOIP_CH_NUM_PER_DSP_4
#define CH_NUM_PER_DSP	4
#elif defined CONFIG_RTK_VOIP_CH_NUM_PER_DSP_5
#define CH_NUM_PER_DSP	5
#elif defined CONFIG_RTK_VOIP_CH_NUM_PER_DSP_6
#define CH_NUM_PER_DSP	6
#elif defined CONFIG_RTK_VOIP_CH_NUM_PER_DSP_7
#define CH_NUM_PER_DSP	7
#elif defined CONFIG_RTK_VOIP_CH_NUM_PER_DSP_8
#define CH_NUM_PER_DSP	8
#endif
#endif

#if 0
extern unsigned int ChNumPerDsp[];
extern unsigned int SessNumPerDsp[];
#endif

#if 0
extern unsigned short API_get_DSP_ID(int cmd, int host_chid);
extern unsigned int API_get_DSP_CH(int cmd, int host_chid);
extern unsigned int API_get_Host_CH(unsigned short dsp_id, int dsp_chid);
extern unsigned int API_get_DSP_NUM(void);
#endif

#endif
