#ifndef __SND_HELP_H__
#define __SND_HELP_H__

#include "snd_define.h"	// stPulseDetParam

#ifndef AUDIOCODES_VOIP
#include "../voip_dsp/include/fskcid_gen.h"
extern void SLIC_gen_FSK_CID(unsigned int chid, char mode, unsigned char msg_type, TstFskClidData *clid_data);
extern int SLIC_gen_VMWI(unsigned int chid, TstFskClidData *clid_data);
#endif

extern void ring_det_cad_set( unsigned int ring_on_msec, unsigned int first_ringoff_msec, unsigned int ring_off_msec);
extern void ring_times_set(unsigned int chid, unsigned int ringOn, unsigned int ringOff);
extern void vir_daa_ring_det_set(unsigned int on_ths, unsigned int off_ths);
#ifdef PULSE_DIAL_DET
void set_pulse_det(unsigned int chid, unsigned int enable, stPulseDetParam* param);
unsigned int get_pulse_det(unsigned int chid);
#endif

#endif // __SND_HELP_H__

