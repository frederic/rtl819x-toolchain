#ifndef _RTP_MAIN_H
#define _RTP_MAIN_H

void rtp_main(void);
void rtp_SetConfig(uint32 chid, uint32 sid, RtpPayloadType uPktFormat, uint32 nFramePerPacket);
int32 rtp_Session_Config(uint32 chid, uint32 sid, CRtpConfig* pCfg);
int32 rtp_Session_UnConfig(uint32 chid, uint32 sid);

#endif
