#include "ipc_arch_help_dsp.h"

int Set_Ethernet_DSP_ID(unsigned char dsp_id)
{
	return Set_DSP_CPUID( dsp_id );
}

unsigned int Get_Ethernet_DSP_ID(void)
{
	return Get_DSP_CPUID();
}


