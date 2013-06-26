#ifndef __BUS_IIS_SCHED_H__
#define __BUS_IIS_SCHED_H__

#include "rtk_voip.h"

#ifdef PCM_HANDLER_USE_CLI

// disable All GIMR except PCM and Timer
#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) || defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC)
unsigned long old_iis_gimr_value;
#define GIMR_PCM_IIS (0xb8003000)
#define GIMR_MASK_PCM_IIS (0x04080000) //only PCM AND IIS
#endif

static inline void cli_iis(void){
	unsigned long flags;
	save_flags(flags); cli();

#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) || defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC)
	old_iis_gimr_value = *(volatile unsigned long *)GIMR_PCM_IIS;
	*(volatile unsigned long *)GIMR_PCM_IIS = GIMR_MASK_PCM_IIS;
	*(volatile unsigned long *)GIMR_PCM_IIS; // force GIMR updated.
#endif
	restore_flags(flags);
}

static inline void sti_iis(void){
	unsigned long flags;
	save_flags(flags); cli();

	// restore GIMR
#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) || defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC)
	*(volatile unsigned long *)GIMR_PCM_IIS = old_iis_gimr_value;
#endif

	restore_flags(flags);
}

#endif /* PCM_HANDLER_USE_CLI */

#endif /* __BUS_IIS_SCHED_H__ */

