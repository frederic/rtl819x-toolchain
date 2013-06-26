#ifndef __CALLER_H__
#define __CALLER_H__

#define DTMF_CLID_SIZE	21

typedef struct
{
	unsigned char bAuto_Ring;
	unsigned char bAuto_SLIC_action;
	unsigned char bBefore1stRing;
	unsigned char bAuto_StartEnd;
	unsigned char start_digit;
	unsigned char end_digit;
	unsigned int on_duration;		// unint: 1 meanes 10ms
	unsigned int pause_duration;		// unint: 1 meanes 10ms
	unsigned int pre_silence_duration;	// unint: 1 meanes 10ms
	unsigned int end_silence_duration;	// unint: 1 meanes 10ms
	unsigned char data[DTMF_CLID_SIZE];	
}
TstDtmfClid;

extern TstDtmfClid dtmf_cid_info[];
extern void dtmf_cid_init(unsigned int chid);
extern void fsk_gen_init(uint32 chid);

extern char ntt_skip_dc_loop[];

#endif
