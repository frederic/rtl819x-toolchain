#ifndef VOIP_MGR_NETFILTER_H
#define VOIP_MGR_NETFILTER_H

#include "../include/voip_types.h"
#include "../include/voip_params.h"
#include "../include/voip_control.h"
#include "../include/rtk_voip.h"
#ifndef AUDIOCODES_VOIP
#include "../voip_dsp/rtp/Rtp.h"
#include "../voip_dsp/dsp_r0/dspcodec_0.h"
#include "../voip_dsp/dsp_r1/include/dspcodec.h"
#endif
//#include "../voip_drivers/si3210init.h"
#include "../voip_drivers/snd_mux_slic.h"

#include "voip_resource_check.h"

extern char fsk_cid_state[];
extern char ntt_skip_dc_loop[];
extern volatile char fsk_alert_state[];
extern volatile char fsk_alert_time[];
extern int Is_DAA_Channel(int chid);

#endif
