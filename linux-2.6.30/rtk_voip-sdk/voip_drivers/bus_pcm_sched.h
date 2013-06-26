#ifndef __BUS_PCM_SCHED_H__
#define __BUS_PCM_SCHED_H__

#include "rtk_voip.h"

#ifdef PCM_HANDLER_USE_CLI
// disable All GIMR except PCM and Timer
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
unsigned short old_pcm_gimr_value;
#define GIMR_PCM (0xBD010000)
#ifdef AUDIOCODES_VOIP
#define GIMR_MASK_PCM (0xB1)
#else
#define GIMR_MASK_PCM (0x081)
#endif
#elif defined( CONFIG_RTK_VOIP_DRIVERS_PCM8651 )
unsigned long old_pcm_gimr_value;
#define GIMR_PCM (0xBD012000)
#define GIMR_MASK_PCM (0x80040000)
#elif defined( CONFIG_RTK_VOIP_DRIVERS_PCM8671 )
unsigned short old_pcm_gimr_value;
#define GIMR_PCM (0xb9c03010)
#define GIMR_MASK_PCM (0x4020)
#elif defined( CONFIG_RTK_VOIP_DRIVERS_PCM8672 ) || defined( CONFIG_RTK_VOIP_DRIVERS_PCM8676 )
unsigned long old_pcm_gimr_value;
#define GIMR_PCM (0xb8003000)
#define GIMR_MASK_PCM (0x00080000) //only PCM
#elif defined( CONFIG_RTK_VOIP_DRIVERS_PCM865xC )
unsigned long old_pcm_gimr_value;
#define GIMR_PCM (0xB8003000)
#define GIMR_MASK_PCM (0x80100)
#elif defined( CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY )
unsigned long old_pcm_gimr_value;
#define GIMR_PCM (0xB8003000)
#define GIMR_MASK_PCM (0x80100)
#elif defined( CONFIG_RTK_VOIP_DRIVERS_PCM89xxC )
unsigned long old_pcm_gimr_value;
#define GIMR_PCM (0xB8003000)
#define GIMR_MASK_PCM (0x80100)
#elif defined( CONFIG_RTK_VOIP_DRIVERS_PCM89xxD )
unsigned long old_pcm_gimr_value;
#define GIMR_PCM (0xB8003000)
#define GIMR_MASK_PCM (0x80100)
#else
# error
#endif

static inline void cli_pcm(void){
	unsigned long flags;
	save_flags(flags); cli();

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
	old_pcm_gimr_value = *(volatile unsigned short *)GIMR_PCM;
	*(volatile unsigned short *)GIMR_PCM = GIMR_MASK_PCM;
#elif defined( CONFIG_RTK_VOIP_DRIVERS_PCM8651 )
	old_pcm_gimr_value = *(volatile unsigned long *)GIMR_PCM;
	*(volatile unsigned long *)GIMR_PCM = GIMR_MASK_PCM;
#elif defined( CONFIG_RTK_VOIP_DRIVERS_PCM8671 )
	old_pcm_gimr_value = *(volatile unsigned short *)GIMR_PCM;
	*(volatile unsigned short *)GIMR_PCM = GIMR_MASK_PCM;
#elif defined( CONFIG_RTK_VOIP_DRIVERS_PCM8672 ) ||  defined( CONFIG_RTK_VOIP_DRIVERS_PCM8676 )
	old_pcm_gimr_value = *(volatile unsigned long *)GIMR_PCM;
	*(volatile unsigned long *)GIMR_PCM = GIMR_MASK_PCM;
#elif defined( CONFIG_RTK_VOIP_DRIVERS_PCM865xC )
	old_pcm_gimr_value = *(volatile unsigned long *)GIMR_PCM;
	*(volatile unsigned long *)GIMR_PCM = GIMR_MASK_PCM;
	*(volatile unsigned long *)GIMR_PCM; // force GIMR updated.
#elif defined( CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY )
	old_pcm_gimr_value = *(volatile unsigned long *)GIMR_PCM;
	*(volatile unsigned long *)GIMR_PCM = GIMR_MASK_PCM;
	*(volatile unsigned long *)GIMR_PCM; // force GIMR updated.
#elif defined( CONFIG_RTK_VOIP_DRIVERS_PCM89xxC )
	old_pcm_gimr_value = *(volatile unsigned long *)GIMR_PCM;
	*(volatile unsigned long *)GIMR_PCM = GIMR_MASK_PCM;
	*(volatile unsigned long *)GIMR_PCM; // force GIMR updated.
#elif defined( CONFIG_RTK_VOIP_DRIVERS_PCM89xxD )
	old_pcm_gimr_value = *(volatile unsigned long *)GIMR_PCM;
	*(volatile unsigned long *)GIMR_PCM = GIMR_MASK_PCM;
	*(volatile unsigned long *)GIMR_PCM; // force GIMR updated.
#endif
	restore_flags(flags);
}

static inline void sti_pcm(void){
	unsigned long flags;
	save_flags(flags); cli();

	// restore GIMR
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
	*(volatile unsigned short *)GIMR_PCM = old_pcm_gimr_value;
#elif defined( CONFIG_RTK_VOIP_DRIVERS_PCM8651 )
	*(volatile unsigned long *)GIMR_PCM = old_pcm_gimr_value;
#elif defined( CONFIG_RTK_VOIP_DRIVERS_PCM8671 )
	*(volatile unsigned short *)GIMR_PCM = old_pcm_gimr_value;
#elif defined( CONFIG_RTK_VOIP_DRIVERS_PCM8672 ) || defined( CONFIG_RTK_VOIP_DRIVERS_PCM8676 )
	*(volatile unsigned long *)GIMR_PCM = old_pcm_gimr_value;
#elif defined( CONFIG_RTK_VOIP_DRIVERS_PCM865xC )
	*(volatile unsigned long *)GIMR_PCM = old_pcm_gimr_value;
#elif defined( CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY )
	*(volatile unsigned long *)GIMR_PCM = old_pcm_gimr_value;
#elif defined( CONFIG_RTK_VOIP_DRIVERS_PCM89xxC )
	*(volatile unsigned long *)GIMR_PCM = old_pcm_gimr_value;
#elif defined( CONFIG_RTK_VOIP_DRIVERS_PCM89xxD )
	*(volatile unsigned long *)GIMR_PCM = old_pcm_gimr_value;
#endif

	restore_flags(flags);
}

#endif /* PCM_HANDLER_USE_CLI */

#endif	/* __BUS_PCM_SCHED_H__ */

