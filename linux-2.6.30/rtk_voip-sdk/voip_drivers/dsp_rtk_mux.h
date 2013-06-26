#ifndef __DSP_RTK_MUX_H__
#define __DSP_RTK_MUX_H__

#include "voip_types.h"

// provide global number to access 
extern int dsp_rtk_ch_num;
extern int dsp_rtk_ss_num;

extern void bus_fifo_clean_tx_dch( uint32 dch );

#endif /* __DSP_RTK_MUX_H__ */

