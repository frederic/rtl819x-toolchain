#ifndef _DSP_MAIN_H
#define _DSP_MAIN_H

#include "rtk_voip.h"
#include "voip_types.h"
#include "../rtp/RtpPacket.h"
#ifdef SUPPORT_RTCP
#include "../rtp/RtcpPacket.h"
#endif
//#include "rtp/Rtp.h"

#define G723_RATE63 0
#define G723_RATE53 1

struct _rtp_config_s
{
	uint32 isTcp;
	uint32 remIp;
	uint16 remPort;
	uint32 extIp;
	uint16 extPort;

	uint32 chid;
	uint32 mid;

//	RtpPayloadType uPktFormat;
};
typedef struct _rtp_config_s rtp_config_t;

#ifdef SUPPORT_RTCP
struct _rtcp_config_s
{
	uint32 bOpenRtcp;
	uint16 rtcpRemPort;
	uint16 rtcpExtPort;

	uint32 chid;
	uint32 mid;
};
typedef struct _rtcp_config_s rtcp_config_t;

#endif

int DSP_init( void );
int32 DSP_rtpWrite( RtpPacket* pst );
int32 DSP_Read(uint32 chid, uint32 sid, uint8* pBuf, int32 nSize);
int32 DSP_Write(uint32 chid, uint32 sid, uint8* pBuf, int32 nSize,
				int bRedundancyPacket);
int32 DSP_CodecRestart(uint32 chid, uint32 sid, RtpPayloadType uPktFormat, uint32 nFramePerPacket, int32 nG723Type, bool bVAD, bool bPLC, uint32 nJitterDelay, uint32 nMaxDelay, uint32 nJitterFactor, uint32 G726Packing, uint32 nG7111Mode, uint32 nPcmMode);
void hc_SetPlayTone(uint32 chid, uint32 sid, uint32 nTone, uint bFlag, uint path);
uint32 API_OpenSid(uint32 chid, uint32 mid);
uint32 API_GetSid(uint32 chid, uint32 mid);
uint32 API_GetMid(uint32 chid, uint32 sid);
uint32 API_CloseSid(uint32 chid, uint32 mid);
void API_InitVar(void);

#ifdef SUPPORT_RTCP
int32 DSP_rtcpRead( uint32 chid, uint32 sid, const void* packet, uint32 pktLen);
int32 DSP_rtcpWrite( RtcpPacket* pst );
#endif

#ifdef SUPPORT_3WAYS_AUDIOCONF
uint32 chanInfo_SetConference(uint32 chid, uint32 bEnable);
uint32 chanInfo_IsConference(uint32 chid);
#endif
int32 chanInfo_SetTranSessionID(uint32 chid, uint32 sid);
int32 chanInfo_CloseSessionID(uint32 chid, uint32 sid);
uint32 chanInfo_GetTranSessionID(uint32 chid);
uint32 chanInfo_GetRegSessionNum(uint32 chid);
uint32 chanInfo_GetRegSessionID(uint32 chid, uint32 reg_idx);
int32 chanInfo_GetRegSessionRank(uint32 chid, uint32 rank[]);
uint32 chanInfo_GetChannelbySession(uint32 sid);
uint32 chanInfo_IsSessionFull(void);
uint32 chanInfo_IsActiveSession(uint32 chid, uint32 sid);
int32 DSP_pktRx( uint32 chid, uint32 mid, const void* packet, uint32 pktLen, uint32 flags );
#ifdef CONFIG_RTK_VOIP_IPC_ARCH
int32 Host_pktRx( uint32 chid, uint32 mid, void* packet, uint32 pktLen, uint32 flags );
unsigned int Dsp_T38Tx( unsigned int chid, unsigned int sid, void* packet, unsigned int pktLen);
#endif
#endif		//_DSP_MAIN_H
