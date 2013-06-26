#ifndef __SND_HELP_IPC_H__
#define __SND_HELP_IPC_H__

#include "con_register.h"

#define SND_PER_DSPCPU( total_snd, total_dspcpu )		\
		( ( ( total_snd ) + ( total_dspcpu ) - 1 ) / ( total_dspcpu ) )

static inline int fill_ipc_dsp_cpuid( 
	voip_snd_t p_snd[],
	int n_fxs, int m_daa, 
	int ordinal_snd,	// n-st snd is this 
	int snd_per_dspcpu	// each dsp occuies how much snd device 
	)
{
	const int total_channel = n_fxs + m_daa;
	const int dsp_cpuid = ordinal_snd / snd_per_dspcpu;
	int i;
	
	for( i = 0; i < total_channel; i ++ ) {
		p_snd[ i ].ipc.f_dsp_cpuid = 1;
		p_snd[ i ].ipc.n_dsp_cpuid = dsp_cpuid;
	}
	
	if( ( ordinal_snd + 1 ) % snd_per_dspcpu == 0 ) {
		return 1;	// TS rewind 
	} else
		return 0;
}

#endif /* __SND_HELP_IPC_H__ */

