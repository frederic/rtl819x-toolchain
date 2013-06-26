#ifndef __DSP_AC_DEFINED_H__
#define __DSP_AC_DEFINED_H__

#include "con_register.h"

// ========================================================
// useful macro   
// ========================================================
extern voip_dsp_t *get_voip_ac_dsp( uint32 dch );

#define DECLARE_CON_FROM_AC_DCH( dch )		\
	voip_dsp_t * const p_dsp = get_voip_ac_dsp( dch );	\
	voip_con_t * const p_con = p_dsp ->con_ptr;			


#endif // __DSP_AC_DEFINED_H__

