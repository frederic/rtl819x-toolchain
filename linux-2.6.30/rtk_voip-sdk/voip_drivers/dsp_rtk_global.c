#include "rtk_voip.h"
#include "voip_debug.h"
#include "con_register.h"

int dsp_rtk_ch_num = DSP_RTK_CH_NUM;	// to be modified after binding 
int dsp_rtk_ss_num = DSP_RTK_SS_NUM;	// to be modified after binding 

// --------------------------------------------------------
// global num 
// --------------------------------------------------------
void dsp_rtk_init_global_num_after_binding( int cch_num, int *p_dsp_ch_num, int *p_dsp_ss_num )
{
	int dch_num = 0;
	
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	extern voip_dsp_t *get_voip_rtk_dsp( uint32 dch );
	
	int i;
	voip_dsp_t *p_dsp;
	
	for( i = 0; i < DSP_RTK_CH_NUM; i ++ ) {
		p_dsp = get_voip_rtk_dsp( i );
		
		if( p_dsp ->con_ptr )
			dch_num ++;
	}
	
	if( cch_num != dch_num ) {
		PRINT_R( "cch and dch number = %d and %d\n", cch_num, dch_num );
	}
#else
	dch_num = cch_num;	// IPC host use cch_num as dch_num 
#endif
	
	dsp_rtk_ch_num = *p_dsp_ch_num = dch_num;
	dsp_rtk_ss_num = *p_dsp_ss_num = dch_num * 2;
}

int add_dsp_rtk_global_num_string( char *buff )
{
	int n = 0;
	
	n += sprintf( buff + n, "dsp_rtk_ch_num = %d\n", dsp_rtk_ch_num );
	n += sprintf( buff + n, "dsp_rtk_ss_num = %d\n", dsp_rtk_ss_num );
	
	return n;
}

