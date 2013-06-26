#include <linux/kernel.h>
#include "ipc_arch_help_dsp.h"

static unsigned int gDSP_cpuid = 0;

unsigned int Get_DSP_CPUID(void)
{	
	return gDSP_cpuid;
}

int Set_DSP_CPUID(unsigned int dsp_cpuid)
{	
	if (( dsp_cpuid < 0 ) || ( dsp_cpuid > 15))
	{
		printk("%s: NOT support dsp_id=%d\n", __FUNCTION__, dsp_cpuid);
		return 0;
	}
		
	gDSP_cpuid = dsp_cpuid;
	printk("Set DSP ID to %d\n", dsp_cpuid);
	
	return 1;
}

