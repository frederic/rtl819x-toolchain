
//
// rtpterm.h
//

#ifndef _RTPTERM_H_
#define _RTPTERM_H_

#include "rtpTypes.h"
//#include "Rtp.h"
#include "rtk_voip.h"
#include "voip_control.h"

typedef struct CRtpConfig_T
{
#if 0
	char m_szRemoteHost[64];				// the name of remote host. Get from TSP
#endif
	//uint32 m_nRtpRemoteIP;					// the IP of remote host.
	//uint32 m_nRtpLocalIP;					// the IP of remote host.
	//uint32 m_nRtpRemotePort;					// the RTP port of remote host. Get from TSP
	//uint32 m_nRtpLocalPort;					// the RTP port of local host. Get from TSP
	//uint32 m_nRtcpRemotePort;					// the RTCP port of remote host. Get from TSP
	//uint32 m_nRtcpLocalPort;					// the RTCP port of local host. Get from TSP
	RtpPayloadType m_uPktFormat;			// the frame type. Get from TSP
	int m_nRecvFrameRate;					// the receiving rate of RTP frame. Get from TSP
	int m_nTranFrameRate;					// the transmitting rate of RTP frame. Get from TSP
	RtpSessionState m_uTRMode;			// sending or receiving mode

	/* add for 8651 only */
	//uint32 isTcp;
	//uint32 m_bOpenRtp;
	//uint32 m_bOpenRtcp;
	//uint32 m_nPeriod;					// ms
} CRtpConfig;

//extern CRtpConfig m_xConfig[MAX_DSP_RTK_SS_NUM];

void CRtpTerminal_Init(uint32 sid, CRtpConfig *pConfig);
//int32 RtpTerminal_Read(uint32 chid, uchar *pBuf, int32 nSize);
int32 RtpTerminal_Read(uint32* pchid, uint32* psid, uint8 *pBuf, int32 nSize);
int32 RtpTerminal_Write(uint32 chid, uint32 sid, uint8 *pBuf, int32 nSize);
void RtpTerminal_SetConfig(uint32 chid, uint32 sid, CRtpConfig *pConfig);
/*+++++ add by Jack for set session state+++++*/
void RtpTerminal_SetSessionState(uint32 sid, uint32 state);
/*-----end-----*/
//void RtpTerminal_GetConfig(uint32 sid, void *pConfig);
void RtpTerminal_TransmitData(void);

void RtpSession_setSessionState (uint32 chid, RtpSessionState state);
void RtpSession_renew(uint32 sid, const TstVoipMgrSession *rtpcfg, 
			const TstVoipRtcpSession *rtcpcfg);
//RESULT SendDTMFEvent(int32 nEvent, int duration);
#ifdef SUPPORT_RTCP
void RtpSession_processRTCP (uint32 sid);
#endif
void RFC2833_receiver_init(uint32 sid);

extern uint32 GetRtpRedundantStatus( uint32 sid );
extern RtpPayloadType GetRtpRedundantPayloadType( uint32 sid, int local );

#endif	// _RTPTERM_H_
