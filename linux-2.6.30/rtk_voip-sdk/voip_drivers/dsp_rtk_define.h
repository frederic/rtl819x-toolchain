#ifndef __DSP_RTK_DEFINE_H__
#define __DSP_RTK_DEFINE_H__

#include "con_register.h"

// ========================================================
// useful macro   
// ========================================================
extern voip_dsp_t *get_voip_rtk_dsp( uint32 dch );

#define DECLARE_CON_FROM_RTK_DCH( dch )		\
	voip_dsp_t * const p_dsp = get_voip_rtk_dsp( dch );	\
	voip_con_t * const p_con = p_dsp ->con_ptr;			

// ========================================================
// dsp functions 
// ========================================================
extern uint32 * GetTxNoToneReadingBaseAddr( uint32 chid );
extern void TxNoToneBufferReadingDone( uint32 chid );

extern int pcm_set_LoopMode(unsigned char group, unsigned int mode, unsigned char main_chid, unsigned char mate_chid);

#endif /* __DSP_RTK_DEFINE_H__ */

