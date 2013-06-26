#ifndef __RTP_RECEIVER_XR_H__
#define __RTP_RECEIVER_XR_H__

#include "rtpTypesXR.h"
#include "RtpPacket.h"

/* ------------------------------------------------ */
/* Record RTP information */
/* ------------------------------------------------ */
extern void RtpRx_InitXR( RtpReceiverXR *pInfoXR );

extern void RtpRx_ResetPeriodicXR( RtpReceiverXR *pInfoXR );

extern void Rtp_receive_XR( RtpPacket *p, RtpReceiverXR *pInfoXR,
							int jitter_delay, RtpTime recvRtpTime );

/* ------------------------------------------------ */
/* Fill report block */
/* ------------------------------------------------ */
extern void RtcpXR_FillSummaryBlock( RtcpXRReportStatisticsSummary *pReport, 
					const RtpReceiverXR *pInfoXR );

extern void RtpRx_FillBurstPacketLossCalc( uint32 sid, 
			RtpReceiverXR *pInfoXR,
			RtcpXRReportVoIPMetrics *pMetrics );

#endif /* __RTP_RECEIVER_XR_H__ */

