#include <linux/config.h>
#include "codec_mem.h"

extern void g726_dmen_memcpy( unsigned char rate);
extern int imem_size;
extern int dmem_size;
extern unsigned long* __imem_type;
extern unsigned long __iram;
extern unsigned long __nat_speedup_start;

void set_g726_dmem(unsigned char rate)
{
#ifdef CONFIG_RTK_VOIP_G726
	#ifdef CONFIG_RTK_VOIP_MODULE
	#else
	g726_dmen_memcpy(rate);
	#endif

#else

#endif
}

void set_imem_size(void)
{
	imem_size = IMEM_SIZE;
}

void set_dmem_size(void)
{
	dmem_size = DMEM_SIZE;
}

void set_system_imem_type(void)
{
#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) || defined(CONFIG_RTK_VOIP_DRIVERS_PCM8671) || defined(CONFIG_RTK_VOIP_DRIVERS_PCM865xC) ||defined(CONFIG_RTK_VOIP_DRIVERS_PCM8672) || defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) || defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC) ||defined(CONFIG_RTK_VOIP_DRIVERS_PCM8676)
	__imem_type = &__iram;
#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM8186
	__imem_type = &__nat_speedup_start;
#endif
}
