#include "rtk_voip.h"
#include "voip_types.h"
#include "rtpTypes.h"
#include "RtcpPacket.h"
#include "RtpPacket.h"
#include "NtpTime.h"
#include "Rtcp.h"
#include "codec_descriptor.h"
#include "lec.h"
#include "level_cluster.h"
#include "RtcpXR_RLE.h"
#include "RtpReceiverXR.h"

extern RtcpPacket RTCP_TX_DEC[RTCP_TX_DEC_NUM];
extern RtcpTransmitter RtcpTxInfo[MAX_DSP_RTK_SS_NUM];

extern RtpReceiver* RtpRx_getInfo (uint32 sid);

#if 0
extern uint32 RtcpXR_SignalDetection[ MAX_DSP_RTK_CH_NUM ];// = {[0 ... MAX_VOIP_CH_NUM-1] = 1};
extern uint32 RtcpXR_SignalLevel[ MAX_DSP_RTK_CH_NUM ];// = {[0 ... MAX_VOIP_CH_NUM-1] = 0};	// Q8
extern uint32 RtcpXR_NoiseLevel[ MAX_DSP_RTK_CH_NUM ];// = {[0 ... MAX_VOIP_CH_NUM-1] = 0};	// Q8
#endif

void RtcpTx_reset( uint32 chid, uint32 sid )
{
	// do DSP related reset 
	
	// signal level 
#if 0
	RtcpXR_SignalLevel[ chid ] = 0;
	RtcpXR_NoiseLevel[ chid ] = 0;
#endif
}

static int RtcpTx_addXR_LossRLE(uint32 sid, RtcpPacket* p, int npadSize)
{
	// rtcpXRBT_LossRLE = 1
	RtcpXRReportBlockLossRLE *LossRLE = 
			( RtcpXRReportBlockLossRLE * )(RtcpPkt_freeData(p));
	RtpReceiver * const rtpReceiver = RtpRx_getInfo( sid );
	RtpReceiverXR * const rtpReceiverXR = &rtpReceiver ->xr;
	uint16 data_bits;
	uint16 end_seq_real;
	uint32 chunks;
		
	// allocate block header
	RtcpPkt_allocData( p, sizeof( RtcpXRReportBlockLossRLE ) 
							- sizeof( RtcpXRChunk ) * 2 );
	
	// header 
	LossRLE ->blockType = rtcpXRBT_LossRLE;
	LossRLE ->rsvd = 1;
	LossRLE ->thinning = LOSS_THINNING;
	LossRLE ->blockLength = 2;
	LossRLE ->SSRC = rtpReceiver ->ssrc;
	
	// determine real end_seq  and data_bits 
	data_bits = ( uint16 )( rtpReceiverXR ->end_seq - 
							rtpReceiverXR ->begin_seq );
	
	if( data_bits > MAX_NUM_XR_LOSS ) {
		end_seq_real = rtpReceiverXR ->begin_seq + MAX_NUM_XR_LOSS;
		data_bits = MAX_NUM_XR_LOSS;
	} else {
		end_seq_real = rtpReceiverXR ->end_seq;
	}
	
	// header - seq 
	LossRLE ->begin_seq = rtpReceiverXR ->begin_seq;
	LossRLE ->end_seq = end_seq_real;
	
	// chunk data 
	chunks = RunLengthEncode( rtpReceiverXR ->loss_bits, 
								data_bits,
								LossRLE ->chunk );
	
	RtcpPkt_allocData( p, sizeof( RtcpXRChunk ) * chunks );
	LossRLE ->blockLength += chunks / 2;		
	
	return ( LossRLE ->blockLength + 1 ) * 4;	// +1 for header 
}

static int RtcpTx_addXR_DupRLE(uint32 sid, RtcpPacket* p, int npadSize)
{
	// rtcpXRBT_DuplicateRLE = 2
	RtcpXRReportBlockDuplicateRLE *DupRLE = 
			( RtcpXRReportBlockDuplicateRLE * )(RtcpPkt_freeData(p));
	RtpReceiver * const rtpReceiver = RtpRx_getInfo( sid );
	RtpReceiverXR * const rtpReceiverXR = &rtpReceiver ->xr;
	uint16 data_bits;
	uint16 end_seq_real;
	uint32 chunks;
		
	// allocate block header
	RtcpPkt_allocData( p, sizeof( RtcpXRReportBlockDuplicateRLE ) 
							- sizeof( RtcpXRChunk ) * 2 );
	
	// header 
	DupRLE ->blockType = rtcpXRBT_DuplicateRLE;
	DupRLE ->rsvd = 1;
	DupRLE ->thinning = DUP_THINING;
	DupRLE ->blockLength = 2;
	DupRLE ->SSRC = rtpReceiver ->ssrc;
	
	// determine real end_seq  and data_bits 
	data_bits = ( uint16 )( rtpReceiverXR ->end_seq - 
							rtpReceiverXR ->begin_seq );
	
	if( data_bits > MAX_NUM_XR_DUP ) {
		end_seq_real = rtpReceiverXR ->begin_seq + MAX_NUM_XR_DUP;
		data_bits = MAX_NUM_XR_DUP;
	} else {
		end_seq_real = rtpReceiverXR ->end_seq;
	}
	
	// header - seq 
	DupRLE ->begin_seq = rtpReceiverXR ->begin_seq;
	DupRLE ->end_seq = end_seq_real;
	
	// chunk data 
	chunks = RunLengthEncode( rtpReceiverXR ->dup_bits, 
								data_bits,
								DupRLE ->chunk );
	
	RtcpPkt_allocData( p, sizeof( RtcpXRChunk ) * chunks );
	DupRLE ->blockLength += chunks / 2;		

	return ( DupRLE ->blockLength + 1 ) * 4;	// +1 for header 
}

static int RtcpTx_addXR_PacketReceiptTimes(uint32 sid, RtcpPacket* p, int npadSize)
{
	// rtcpXRBT_PacketReceiptTimes = 3
	RtcpXRReportBlockReceiptTimes *RecvTimes = 
			( RtcpXRReportBlockReceiptTimes * )(RtcpPkt_freeData(p));
	RtpReceiver * const rtpReceiver = RtpRx_getInfo( sid );
	RtpReceiverXR * const rtpReceiverXR = &rtpReceiver ->xr;
	uint16 data_count;
	uint16 end_seq_real;
	uint16 i;
		
	// allocate block header
	RtcpPkt_allocData( p, sizeof( RtcpXRReportBlockReceiptTimes ) 
							- sizeof( uint32 ) );
	
	// header 
	RecvTimes ->blockType = rtcpXRBT_PacketReceiptTimes;
	RecvTimes ->rsvd = 1;
	RecvTimes ->thinning = ARRIVE_THINING;
	RecvTimes ->blockLength = 2;
	RecvTimes ->SSRC = rtpReceiver ->ssrc;
	
	// determine real end_seq  and data_bits 
	data_count = ( uint16 )( rtpReceiverXR ->end_seq - 
							rtpReceiverXR ->begin_seq );
	
	if( data_count > MAX_NUM_XR_ARRIVE ) {
		end_seq_real = rtpReceiverXR ->begin_seq + MAX_NUM_XR_ARRIVE;
		data_count = MAX_NUM_XR_ARRIVE;
	} else {
		end_seq_real = rtpReceiverXR ->end_seq;
	}
	
	// header - seq 
	RecvTimes ->begin_seq = rtpReceiverXR ->begin_seq;
	RecvTimes ->end_seq = end_seq_real;
	RecvTimes ->blockLength += ( RecvTimes ->end_seq - RecvTimes ->begin_seq );
	
	// recv times data 
	RtcpPkt_allocData( p, sizeof( uint32 ) * data_count );

	for( i = 0; i < data_count; i ++ )
		RecvTimes ->receiptTime[ i ] = rtpReceiverXR ->arrive_timestamp[ i ];
	
	return ( RecvTimes ->blockLength + 1 ) * 4;	// +1 for header 
}

static int RtcpTx_addXR_ReceiverReferenceTime(uint32 sid, RtcpPacket* p, int npadSize)
{
	// rtcpXRBT_ReceiverReferenceTime = 4
	RtcpXRReportBlockReceiverReferenceTime *RecvRefTime = 
			(RtcpXRReportBlockReceiverReferenceTime*) (RtcpPkt_freeData(p));
	int usage = RtcpPkt_allocData(p, sizeof(RtcpXRReportBlockReceiverReferenceTime));
	NtpTime nowNtp;
	
	Ntp_getTime(&nowNtp);
	
	RecvRefTime ->blockType = rtcpXRBT_ReceiverReferenceTime;	// =4 
	RecvRefTime ->reserved = 0;
	RecvRefTime ->blockLength = 2;
	RecvRefTime ->NTP_MSW = nowNtp.seconds;
	RecvRefTime ->NTP_LSW = nowNtp.fractional;
	
	return ( 1 + 2 ) * 4;	// header + payload 
}

static int RtcpTx_addXR_DLRR(uint32 sid, RtcpPacket* p, int npadSize)
{
	// rtcpXRBT_DLRR = 5
	RtcpXRReportBlockDLRR *DLRR = ( RtcpXRReportBlockDLRR * )(RtcpPkt_freeData(p));
	int i;
	int nTrans = RtcpRx_getTranInfoCount(sid);
	NtpTime nowNtp;
	uint32 midNowNtp, midTempNtp;
	
	if( !nTrans )
		return 0;
	
	// fill header 
	DLRR ->blockType = rtcpXRBT_DLRR;
	DLRR ->reserved = 0;
	DLRR ->blockLength = sizeof( RtcpXRReportBlockDLRR_sb ) / 4 * nTrans;
	
	// now NTP 
	Ntp_getTime( &nowNtp );
	midNowNtp = NTP32_getNTP32( &nowNtp );
	
	// allocate block header
	RtcpPkt_allocData( p, sizeof( RtcpXRReportBlockDLRR ) - sizeof( RtcpXRReportBlockDLRR_sb ) );
	
	for (i = 0; i < nTrans; i++)
	{
		RtpTranInfo * tranInfo;
		RtcpXRReportBlockDLRR_sb * const sb = &DLRR ->sb[ i ];
		
		// transaction instance 
		tranInfo = RtcpRx_getTranInfoList(sid, i);
		
		// allocate sub block  
		RtcpPkt_allocData( p, sizeof( RtcpXRReportBlockDLRR_sb ) );
		
		// fill data 
		midTempNtp = NTP32_getNTP32( &tranInfo ->recvLastRRTimestamp );
		
		sb ->SSRC = tranInfo ->ssrc;
		sb ->LRR = midTempNtp;
		sb ->DLRR = midNowNtp - midTempNtp;
	}
	
	return ( DLRR ->blockLength + 1 ) * 4;	// +1 for header 
}

static int RtcpTx_addXR_StatisticsSummary(uint32 sid, RtcpPacket* p, int npadSize)
{
	// rtcpXRBT_StatisticsSummary = 6
	RtcpXRReportStatisticsSummary *pSummary = ( RtcpXRReportStatisticsSummary * )(RtcpPkt_freeData(p));
	RtpReceiver * const rtpReceiver = RtpRx_getInfo( sid );
	RtpReceiverXR * const rtpReceiverXR = &rtpReceiver ->xr;
	
	// allocate block header
	RtcpPkt_allocData( p, sizeof( RtcpXRReportStatisticsSummary ) );
	
	// header 
	pSummary ->blockType = rtcpXRBT_StatisticsSummary;
	pSummary ->lossReport = 1;
	pSummary ->duplicateReport = 1;
	pSummary ->jitterReport = 1;
	pSummary ->TTLorHopLimit = 1;	// 0: none, 1: IPv4, 2: IPv6, 3: reserved 
	pSummary ->reserved = 0;
	pSummary ->blockLength = 9;
	
	// SSRC
	pSummary ->SSRC = rtpReceiver ->ssrc;
	
	// seq
	pSummary ->begin_seq = rtpReceiverXR ->begin_seq;
	pSummary ->end_seq = rtpReceiverXR ->end_seq;
	
	// summary
	RtcpXR_FillSummaryBlock( pSummary, rtpReceiverXR );
	
	return ( pSummary ->blockLength + 1 ) * 4;	// +1 for header 
}

static uint32 CalculateMOSLQ_x10( uint32 factor_q15, uint32 nMosRef_q7,
								int32 dec_signal, int32 dec_noise )
{
	uint32 t_x10;
	uint32 MOS_LQ_x10;
	
	MOS_LQ_x10 = ( nMosRef_q7 * factor_q15 * 100 ) >> 22;	// Q7 * Q15 >> 22 = Q0 
	
#if 1	// use signal/noise level to modify MOS_LQ
	// DSLA signal/noise -> 57/19 
	if( dec_signal == 0 && dec_noise == 0 )
		goto label_signal_noise_ajdust_done;
	
	t_x10 = 0;
	
	if( dec_signal < 35 ) {
		if( dec_noise < 10 ) {
			// very very quiet. need modify?? (probability=1/4) 
			// '== 2' is magic number 
			//t_x10 = ( ( ( dec_signal + dec_noise ) & 0x03 ) == 2 ? 10 : 0 );	
			t_x10 = 3 + ( ( dec_signal + dec_noise ) & 0x07 );
		} else
			t_x10 = 10 + ( ( dec_signal + dec_noise ) & 0x03 );	// -0.1
	} else if( dec_signal < 40 )	// signal is too small
		t_x10 = 20 + ( ( dec_signal + dec_noise ) & 0x03 );	// -0.2
	else if( dec_signal < 50 )
		t_x10 = 10 + ( ( dec_signal + dec_noise ) & 0x03 );	// -0.1
	else if( dec_signal > 60 ) {
		if( dec_noise > 28 )
			t_x10 = 10 + ( ( dec_signal + dec_noise ) & 0x03 );	// too large noise while talking 
	}
	
	if( MOS_LQ_x10 >= t_x10 )
		MOS_LQ_x10 -= t_x10;
	else
		MOS_LQ_x10 = 100;
	
label_signal_noise_ajdust_done:
	;
#endif
	
	if( MOS_LQ_x10 < 100 )		// lowest is 1.0 
		MOS_LQ_x10 = 100;
	
	return MOS_LQ_x10;
}

static int RtcpTx_addXR_VoIPMetrics(uint32 sid, RtcpPacket* p, int npadSize)
{
	// rtcpXRBT_VoIPMetrics = 7
	extern int tx_jit_buf_high_threshold[MAX_DSP_RTK_SS_NUM];
	extern uint32 GetSessionFramePeriod( uint32 ssid );
	extern int JbcRtcpXR_GetNominalDelay( uint32 ssid );
	extern uint32 JbcStatistics_GetPlayoutDelay( uint32 ssid );
	extern unsigned char MFFrameNo[MAX_DSP_RTK_SS_NUM];
	extern int RtcpRx_getAvgRoundTripDelay(uint32 sid);
	extern uint32 chanInfo_GetChannelbySession(uint32 sid);
	extern const codec_algo_desc_t *ppNowCodecAlgorithmDesc[MAX_DSP_RTK_SS_NUM];
	extern uint32 JbcStatistics_GetMosFactor( uint32 ssid );
	extern RtcpTransmitter* RtcpTx_getInfo (uint32 sid);
	extern void rtcp_logger_unit_new_data( RtcpStatisticsUnit *pUnit,
								unsigned long data );
	extern int32 get_energy_det_max( int chid );
	
	RtcpXRReportVoIPMetrics *pMetrics = ( RtcpXRReportVoIPMetrics * )(RtcpPkt_freeData(p));
	RtpReceiver * const rtpReceiver = RtpRx_getInfo( sid );
	RtpReceiverXR * const rtpReceiverXR = &rtpReceiver ->xr;
	const uint32 frame_period = GetSessionFramePeriod( sid );
	NtpTime nowNtp;
	uint32 delta;
	int t;
	int32 max_energy, signal_energy, noise_energy, dec_signal, dec_noise;
	unsigned long MOS_LQ_x10 = 100;	// 1.0
	
	uint32 chid = chanInfo_GetChannelbySession( sid );
	const codec_algo_desc_t * const pCodecAlgoDesc = ppNowCodecAlgorithmDesc[ sid ];
	//uint32 delta_seq;
	RtcpTransmitter * pTxInfo = RtcpTx_getInfo( sid );
	
	// allocate block header
	RtcpPkt_allocData( p, sizeof( RtcpXRReportVoIPMetrics ) );
	
	// header 
	pMetrics ->blockType = rtcpXRBT_VoIPMetrics;
	pMetrics ->reserved = 0;
	pMetrics ->blockLength = 8;

	// SSRC
	pMetrics ->SSRC = rtpReceiver ->ssrc;
	
	// packet loss and discard metrics 
	// (lossRate, discardRate, burstDensity, gapDensity, burstDuration, gapDuration)
	RtpRx_FillBurstPacketLossCalc( sid, rtpReceiverXR, pMetrics );
	
#if 0
	delta_seq = rtpReceiverXR ->end_seq - rtpReceiverXR ->global_begin_seq;

	if( delta_seq ) {
		pMetrics ->lossRate = 
			( rtpReceiverXR ->global_loss_count * 256 ) / delta_seq;
		pMetrics ->discardRate = 
			( rtpReceiverXR ->global_discard_count * 256 ) / delta_seq;
	}
#endif
	
	// delay metrics 
	pMetrics ->roundTripDelay = RtcpRx_getAvgRoundTripDelay( sid );
	
	pMetrics ->endSystemDelay = 
		PCM_PERIOD * 10 * 3 / 2 +
		1 * frame_period +
		MFFrameNo[ sid ] * frame_period +
		PCM_PERIOD * 10 * 3 / 2 +
		JbcStatistics_GetPlayoutDelay( sid ) * frame_period / 10 +
		1 * frame_period +
		5;
	
	// signal related metrics 
	max_energy = get_energy_det_max( chid ); 	// energy = 91 - 20 * log( |x| )
	
	if( chid < MAX_DSP_RTK_CH_NUM /*&& RtcpXR_SignalDetection[ chid ]*/ ) {
		
		get_level_cluster( sid, &signal_energy, &noise_energy, 
								&dec_signal, &dec_noise );
#if 0
		signal_energy = RtcpXR_SignalLevel[ chid ] >> 8;
		noise_energy = RtcpXR_NoiseLevel[ chid ] >> 8;
#endif
		
		// level = 10 log 10( |x| )
		pMetrics ->signalLevel = ( ( int )signal_energy - max_energy ) / 2;
		pMetrics ->noiseLevel = ( ( int )noise_energy - max_energy ) / 2;
	} else {
		signal_energy = 0;
		noise_energy = 0;
		dec_signal = 0;
		dec_noise = 0;
		
		pMetrics ->signalLevel = 127;
		pMetrics ->noiseLevel = 127;
	}

#if 0//def CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
	pMetrics ->RERL = 127; // TODO not support yet
#else
	pMetrics ->RERL = LEC_g168_Get_ERLE( chid ) + 12;
#endif
	
	// call quality or transmission quality metrics (not implement)
	pMetrics ->Rfactor = 127;		// ITU-T G.107 and ETSI TS 101 329-5
	pMetrics ->extRfactor = 127;	// ITU-T G.107 and ETSI TS 101 329-5 
	pMetrics ->MOS_LQ = 127;
	pMetrics ->MOS_CQ = 127;

	if( pCodecAlgoDesc ) {
		// ===== MOS-LQ
		uint32 factor = JbcStatistics_GetMosFactor( sid );	// Q15 
		
		MOS_LQ_x10 = CalculateMOSLQ_x10( factor, pCodecAlgoDesc ->nMosRef,
						dec_signal, dec_noise );
		
		pMetrics ->MOS_LQ = MOS_LQ_x10 / 10;
		
		if( pMetrics ->MOS_LQ < 10 )	// lowest is 1.0 
			pMetrics ->MOS_LQ = 0;
		
		// ===== MOS-CQ
		pMetrics ->MOS_CQ = pMetrics ->MOS_LQ;

#if 1	// use round trip delay to modify MOS_CQ 
		t = 0;
		
		if( pMetrics ->roundTripDelay < 200 )
			;	// do nothing 
		else if( pMetrics ->roundTripDelay < 300 )	// -0.1 ~ -0.5
			t = pMetrics ->roundTripDelay / 100	- 1;
		else if( pMetrics ->roundTripDelay < 500 )	// -0.5 ~ -1.8
			t = pMetrics ->roundTripDelay * 13 / 200 - 14;
		else if( pMetrics ->roundTripDelay < 1000 )	// -1.8 ~ -3.7
			t = pMetrics ->roundTripDelay * 19 / 500 - 1;
		else	// - 4.0
			t = 40;
		
		if( pMetrics ->MOS_CQ >= t )
			pMetrics ->MOS_CQ -= t;
		else
			pMetrics ->MOS_CQ = 0;
#endif
					
		if( pMetrics ->MOS_CQ < 10 )	// lowest is 1.0 
			pMetrics ->MOS_CQ = 10;
		
		// ===== R-factor 
		t = ( pMetrics ->MOS_CQ ) * 4 - 72;
		if( t < 10 )
			t = 10;
		else if( t > 100 )
			t = 100;
		pMetrics ->Rfactor = t;
		
		//printk( "pMetrics ->MOS_CQ=%d, pMetrics ->Rfactor=%d\n\n", 
		//			pMetrics ->MOS_CQ, pMetrics ->Rfactor );
	}
	
	// configuration parameters 
	pMetrics ->Gmin = XR_GMIN;		// gap threshold. ETSI TS 101 329-5 
	pMetrics ->RXconfig.PLC = 0x03;	// 11: standard, 10: enhanced, 01: disabled, 00: unspecified 
	pMetrics ->RXconfig.JBA = 0x03;	// 11: adaptive, 10: non-adaptive, 01: reserved, 00: unknown
	pMetrics ->RXconfig.JBrate = 0;	// // rate = 0 - 15, adjustment time = 2 * J * frame size (ms)
	
	// jitter buffer parameters 
	pMetrics ->JBnominal = JbcRtcpXR_GetNominalDelay( sid ) * frame_period;
	pMetrics ->JBmaximum = tx_jit_buf_high_threshold[ sid ] * frame_period;
	pMetrics ->JBabsMax = 30 * frame_period;
	
	// logger tx data 
	if( pTxInfo ) {
		if( pTxInfo ->txLogger.round_trip_delay.count == 0 &&
			pMetrics ->roundTripDelay == 0 )
		{
			// count == 0 && delay == 0.... ignore it! 
		} else
			rtcp_logger_unit_new_data( &pTxInfo ->txLogger.round_trip_delay,
										pMetrics ->roundTripDelay );
		
		rtcp_logger_unit_new_data( &pTxInfo ->txLogger.MOS_LQ,
									MOS_LQ_x10 );
	}
	
	return ( pMetrics ->blockLength + 1 ) * 4;	// +1 for header 
}

typedef int( *fnRtcpXR_ReportBlock )( uint32 sid, RtcpPacket* p, int npadSize );

typedef struct {
	fnRtcpXR_ReportBlock	fn;
	//const char *			name;
} RtcpXR_ReportBlock_t;

//#define _M_XRRB( x )	{ x, #x }
#define _M_XRRB( x )	{ x }

static const RtcpXR_ReportBlock_t RtcpXR_ReportBlock[] = {
	_M_XRRB( RtcpTx_addXR_LossRLE ),
	_M_XRRB( RtcpTx_addXR_DupRLE ),
	_M_XRRB( RtcpTx_addXR_PacketReceiptTimes ),
	_M_XRRB( RtcpTx_addXR_ReceiverReferenceTime ),
	_M_XRRB( RtcpTx_addXR_DLRR ),
	_M_XRRB( RtcpTx_addXR_StatisticsSummary ),
	_M_XRRB( RtcpTx_addXR_VoIPMetrics ),
};

#undef _M_XRRB

#define KINDS_OF_XR_REPORT_BLOCK		( sizeof( RtcpXR_ReportBlock ) / sizeof( RtcpXR_ReportBlock[ 0 ] ) )


int RtcpTx_addXR (uint32 sid, RtcpPacket* p, int npadSize)
{
	// header
	RtcpXRPacketHeader* header = (RtcpXRPacketHeader*) (RtcpPkt_freeData(p));
	int usage = RtcpPkt_allocData(p, sizeof(RtcpXRPacketHeader));
	RtcpTransmitter *pInfo;
	NtpTime nowNtp, thenNtp;
	int i;
	
	if(sid >= DSP_RTK_SS_NUM)
		return -1;
	pInfo = &RtcpTxInfo[sid];
	
	header->version = RTP_VERSION;
	header->padding = (npadSize > 0) ? 1 : 0;
	header->reserved = 0;
	header->type = rtcpTypeXR;		// =207 
	header->length = 0;	// modify later 
	header->SSRC = pInfo->tran->ssrc;
	
	// add XR report block 
	for( i = 0; i < KINDS_OF_XR_REPORT_BLOCK; i ++ )
		usage += ( *RtcpXR_ReportBlock[ i ].fn )( sid, p, npadSize );
	
	// reset statstics 
	RtpRx_ResetPeriodicXR( &RtpRx_getInfo( sid ) ->xr );
	
	// padding
	if (npadSize > 0)
	{
		// future: not implemented
		assert (0);
	}
	
	// overall packet must ends on 32-bit count
	assert (usage % 4 == 0);

#if 0
	header->length = htons((usage / 4) - 1);
#else
	header->length = (usage / 4) - 1;
#endif
	//cpLog (LOG_DEBUG_STACK, "RTCP:  SR/RR packet used %d bytes/ %d words",
	//       usage, usage/4);
	return usage;
}

int rtcp_tx_logger_MOSLQ_string( char *buff, uint32 sid )
{
	extern const codec_algo_desc_t *ppNowCodecAlgorithmDesc[MAX_DSP_RTK_SS_NUM];
	extern uint32 JbcStatistics_GetMosFactor( uint32 ssid );
	
	const codec_algo_desc_t * const pCodecAlgoDesc = ppNowCodecAlgorithmDesc[ sid ];
	uint32 factor = JbcStatistics_GetMosFactor( sid );	// Q15 
	int32 signal_energy, noise_energy, dec_signal, dec_noise;
	uint32 MOS_LQ_x10;
	int n = 0;
	
	if( pCodecAlgoDesc == NULL ) {	// prevent core dump 
		MOS_LQ_x10 = 90;	// give a magic number 
		goto label_done;
	}
	
	get_level_cluster( sid, &signal_energy, &noise_energy, 
								&dec_signal, &dec_noise );
								
	MOS_LQ_x10 = CalculateMOSLQ_x10( factor, pCodecAlgoDesc ->nMosRef,
								dec_signal, dec_noise );

label_done:	
	n = sprintf( buff + n, "Snapshot: MOS-LQ*10=%u\n", MOS_LQ_x10 );
	
	return n;
}

