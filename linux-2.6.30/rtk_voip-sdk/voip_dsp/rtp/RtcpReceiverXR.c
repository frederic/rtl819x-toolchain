#include "rtk_voip.h"
#include "RtcpReceiverXR.h"

static void RtcpRx_ReportBlock_DLRR( uint32 sid, 
							const RtcpXRReportBlockHeader *pReportBlock )
{
}

static void RtcpRx_ReportBlock_VoIPMetrics( uint32 sid, 
							const RtcpXRReportBlockHeader *pReportBlock )
{
	extern RtcpReceiver* RtcpRx_getInfo (uint32 sid);
	extern void rtcp_logger_unit_new_data( RtcpStatisticsUnit *pUnit,
								unsigned long data );
	
	const RtcpXRReportVoIPMetrics * const pVoIPMetrics = 
				( const RtcpXRReportVoIPMetrics * )pReportBlock;
	RtcpReceiver* pRxInfo;
	
	if( ( pRxInfo = RtcpRx_getInfo( sid ) ) == NULL )
		return;
	
	if( pRxInfo ->rxLogger.round_trip_delay.count == 0 &&
		pVoIPMetrics ->roundTripDelay == 0 )
	{
		// count == 0 && delay == 0.... ignore it! 
	} else
		rtcp_logger_unit_new_data( &pRxInfo ->rxLogger.round_trip_delay,
									pVoIPMetrics ->roundTripDelay );
	
	rtcp_logger_unit_new_data( &pRxInfo ->rxLogger.MOS_LQ,
								pVoIPMetrics ->MOS_LQ );
	
}

void RtcpRx_readXR (uint32 sid, RtcpHeader* head)
{
	uint32 total_length = head ->length * 4;	// in bytes   
	uint32 offset = 0;
	uint8 *middle = ( uint8 * )head;
	RtcpXRReportBlockHeader *pReportBlock;
	
	offset = sizeof( RtcpHeader ) + 4;	// +4 for SSRC 
	
	while( offset < total_length ) {
		
		pReportBlock = ( RtcpXRReportBlockHeader * )( middle + offset );
		
		switch( pReportBlock ->blockType ) {
		case rtcpXRBT_LossRLE:
		case rtcpXRBT_DuplicateRLE:
		case rtcpXRBT_PacketReceiptTimes:
		case rtcpXRBT_ReceiverReferenceTime:
			break;
			
		case rtcpXRBT_DLRR:
			RtcpRx_ReportBlock_DLRR( sid, pReportBlock );
			break;
			
		case rtcpXRBT_StatisticsSummary:
			break;
			
		case rtcpXRBT_VoIPMetrics:
			RtcpRx_ReportBlock_VoIPMetrics( sid, pReportBlock );
			break;
			
		default:
			printk( "Bad XR block report type\n" );
			break;
		}
				
		offset += pReportBlock ->blockLength * 4 + 4;	// +4 for BT header 
	}
}


