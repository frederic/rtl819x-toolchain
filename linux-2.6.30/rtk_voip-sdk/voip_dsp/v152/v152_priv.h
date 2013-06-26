#ifndef __V152_PRIV_H__
#define __V152_PRIV_H__

#include "voip_types.h"
#include "v152_api.h"

typedef struct {
	v152st	state;
	uint32	reasonAudio;	// reason back to audio (a history list)
	uint32	reasonVBD;		// reason back to VBD (a history list)
	v152reason	prevRtp;	// previous RTP is audio or VBD. only check REASON_RTP_VBD, others seen as audio 
	uint32	silence;		// bidirectional silence counter 
} v152_session_vars_t;

typedef struct {
	v152_session_vars_t session[ DSP_RTK_SS_NUM ];
	uint32 bitsEnable;			// support VBD in sip negotiation ? 
	uint32 bitsSwitchCodec;		// each bit represents a session 
} v152_vars_t; 

extern v152_vars_t v152_vars;

#endif // __V152_PRIV_H__

