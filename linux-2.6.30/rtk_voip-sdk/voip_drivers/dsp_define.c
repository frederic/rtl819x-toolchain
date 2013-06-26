#include "rtk_voip.h"
#include "dsp_define.h"

// This file define some variables for all DSP. 

char fsk_cid_state[MAX_DSP_CH_NUM]={0};		// for FSK CID (dch)

#ifdef SUPPORT_SLIC_GAIN_CFG
uint32 g_txVolumneGain[MAX_DSP_CH_NUM];
uint32 g_rxVolumneGain[MAX_DSP_CH_NUM];
#endif

unsigned char rfc2833_dtmf_pt_local[MAX_DSP_SS_NUM];
unsigned char rfc2833_dtmf_pt_remote[MAX_DSP_SS_NUM];
unsigned char rfc2833_fax_modem_pt_local[MAX_DSP_SS_NUM];
unsigned char rfc2833_fax_modem_pt_remote[MAX_DSP_SS_NUM];
unsigned int CurrentRfc2833DtmfMode[MAX_DSP_RTK_CH_NUM];	/* 0 : current is not in DTMF RFC2833 mode, 1: in RFC2833 mode*/
unsigned int CurrentRfc2833FaxModemMode[MAX_DSP_RTK_CH_NUM];	/* 0 : current is not in Fax/Modem RFC2833 mode, 1: in RFC2833 mode*/

