#ifndef __DSP_DEFINE_H__
#define __DSP_DEFINE_H__

#include "rtk_voip.h"
#include "voip_types.h"

#ifdef SUPPORT_SLIC_GAIN_CFG
extern uint32 g_txVolumneGain[MAX_DSP_CH_NUM];
extern uint32 g_rxVolumneGain[MAX_DSP_CH_NUM];
#endif

extern unsigned char rfc2833_payload_type_local[MAX_DSP_SS_NUM];
extern unsigned char rfc2833_payload_type_remote[MAX_DSP_SS_NUM];

#endif /* __DSP_DEFINE_H__ */

