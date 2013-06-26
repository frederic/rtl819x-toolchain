#ifndef __DSP_RTP_DEF_H__
#define __DSP_RTP_DEF_H__

// This file define some types shared to enc/dec phases and RTP 
typedef enum {
	RTP_FRAME_NORMAL,		// normal 
	//RTP_FRAME_NORMAL_SID,	// normal + SID (enc only)
	RTP_FRAME_SID,			// SID
	//RTP_FRAME_NTX,		// Not TX (enc only)
} RtpFramesType;

#endif /* __DSP_RTP_DEF_H__ */

