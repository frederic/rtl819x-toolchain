#include <linux/sched.h>
#include <linux/interrupt.h>
#include "rtk_voip.h"
#include "con_bus_handler.h"
#include "section_def.h"

#include "bus_pcm_sched.h"
#include "bus_iis_sched.h"

static inline void cli_bus( void )
{
#ifdef PCM_HANDLER_USE_CLI
#ifdef CONFIG_RTK_VOIP_DRIVERS_IIS
	cli_iis();
#else
	cli_pcm();
#endif
#endif

}

static inline void sti_bus( void )
{
#ifdef PCM_HANDLER_USE_CLI
#ifdef CONFIG_RTK_VOIP_DRIVERS_IIS
	sti_iis();
#else
	sti_pcm();
#endif
#endif		
}

#ifdef PCM_HANDLER_USE_TASKLET
__pcmifdata20 struct tasklet_struct bus_handler_tasklet;
#endif

void bus_handler_2(unsigned long dummy)
{
	extern void dsp_rtk_bus_handler( void );
	extern void dsp_ac_bus_handler( void );
	extern void bus_pcm_processor_in_tasklet( void );
	extern void bus_iis_processor_in_tasklet( void );
	
	cli_bus();
	
#ifdef AUDIOCODES_VOIP
	dsp_ac_bus_handler();
#else
	dsp_rtk_bus_handler();
#endif

	sti_bus();

#ifdef CONFIG_RTK_VOIP_DRIVERS_IIS
	bus_iis_processor_in_tasklet();
#else
	bus_pcm_processor_in_tasklet();
#endif
}

#if defined (AUDIOCODES_VOIP)
__pcmImem void bus_handler_3(void)
#else
void bus_handler_3(void)
#endif
{
#ifdef PCM_HANDLER_USE_TASKLET
	tasklet_hi_schedule(&bus_handler_tasklet);	
#else
	bus_handler_2(NULL);
#endif
}

#ifdef PCM_HANDLER_USE_TASKLET
void dsp_schedule_bus_handler( void )
{
	tasklet_hi_schedule(&bus_handler_tasklet);
}
#endif

void isr_schedule_bus_handler( void )
{
#ifdef PCM_HANDLER_USE_TASKLET
	tasklet_hi_schedule(&bus_handler_tasklet);	
#else
	bus_handler_2(NULL);
#endif
}

void kill_bus_handler( void )
{
#ifdef PCM_HANDLER_USE_TASKLET
	tasklet_kill(&bus_handler_tasklet);
#endif	
}

void bus_handler_tasklet_init( void )
{
#ifdef PCM_HANDLER_USE_TASKLET
	tasklet_init(&bus_handler_tasklet, bus_handler_2, (unsigned long)NULL);
	BOOT_MSG("=== Bus Handler Tasklet Initialization ===\n");
#endif	
}

