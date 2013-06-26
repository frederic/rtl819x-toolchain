#ifndef __RTP_TYPES_XR_H__
#define __RTP_TYPES_XR_H__

#include "voip_types.h"

/* ----------------------------------------------------------------- */
/* --- RTCP XR Structures for Packet ------------------------------- */
/* ----------------------------------------------------------------- */

typedef struct {
	uint32 version	:2;		// = 2
	uint32 padding	:1;
	uint32 reserved	:5;
	uint32 type		:8;		// XR = 207
	uint32 length	:16;
	uint32 SSRC;
	// report blocks here 
} RtcpXRPacketHeader;	// this will be compatible with RtcpHeader 

typedef enum {
	rtcpXRBT_LossRLE = 1,
	rtcpXRBT_DuplicateRLE = 2,
	rtcpXRBT_PacketReceiptTimes = 3,
	rtcpXRBT_ReceiverReferenceTime = 4,
	rtcpXRBT_DLRR = 5,	// delay since the last Sender Report 
	rtcpXRBT_StatisticsSummary = 6,
	rtcpXRBT_VoIPMetrics = 7,
} RtcpXRBT;	// rtcp XR block type 

typedef union {
	struct {
		uint16	chunkType		:1;	// =0
		uint16	runType			:1;	// 0 = zeros, 1 = ones 
		uint16	runLength		:14;
	} runLength;
	struct {
		uint16	chunkType		:1;	// =1
		uint16	bitVector		:15;
	} bitVector;
	struct {
		uint16	chunkType		:1;
		uint16	others			:15;
	} general;
} RtcpXRChunk;

typedef struct {
	RtcpXRBT	blockType		:8;
	uint32		typeSpecific	:8;		// defined by each block type 
	uint32		blockLength		:16;
	uint32		blockContents[ 1 ];		// defined by each block type 
} RtcpXRReportBlockHeader;

typedef struct {
	RtcpXRBT	blockType		:8;		// BT=1 or BT=2
	uint32		rsvd			:4;
	uint32		thinning		:4;
	uint32		blockLength		:16;
	uint32		SSRC;
	uint32		begin_seq		:16;
	uint32		end_seq			:16;
	RtcpXRChunk	chunk[ 2 ];				// size should be power of 2 
} RtcpXRReportBlockLossRLE, RtcpXRReportBlockDuplicateRLE;

typedef struct {
	RtcpXRBT	blockType		:8;		// BT=3
	uint32		rsvd			:4;
	uint32		thinning		:4;
	uint32		blockLength		:16;
	uint32		SSRC;
	uint32		begin_seq		:16;
	uint32		end_seq			:16;
	uint32		receiptTime[ 1 ];
} RtcpXRReportBlockReceiptTimes;

typedef struct {
	RtcpXRBT	blockType		:8;		// BT=4
	uint32		reserved		:8;
	uint32		blockLength		:16;	// =2
	uint32		NTP_MSW;				// most significant word 
	uint32		NTP_LSW;				// least significant word 
} RtcpXRReportBlockReceiverReferenceTime;

typedef struct {
	uint32	SSRC;
	uint32	LRR;					// last RR
	uint32	DLRR;					// delay since last RR
} RtcpXRReportBlockDLRR_sb;

typedef struct {
	RtcpXRBT	blockType		:8;		// BT=5
	uint32		reserved		:8;
	uint32		blockLength		:16;
	RtcpXRReportBlockDLRR_sb sb[ 1 ];	// sub block
} RtcpXRReportBlockDLRR;	// delay since the last Sendr Report 

typedef struct {
	RtcpXRBT	blockType		:8;		// BT=6
	uint32		lossReport		:1;
	uint32		duplicateReport	:1;
	uint32		jitterReport	:1;
	uint32		TTLorHopLimit	:2;
	uint32		reserved		:3;
	uint32		blockLength		:16;	// =9
	uint32		SSRC;
	uint16		begin_seq;
	uint16		end_seq;
	uint32		lost_packets;
	uint32		dup_packets;
	uint32		min_jitter;
	uint32		max_jitter;
	uint32		mean_jitter;
	uint32		dev_jitter;
	uint8		min_ttl_or_hl;
	uint8		max_ttl_or_hl;
	uint8		mean_ttl_or_hl;
	uint8		dev_ttl_or_hl;
} RtcpXRReportStatisticsSummary;

typedef struct {
	RtcpXRBT	blockType		:8;		// BT=7
	uint32		reserved		:8;
	uint32		blockLength		:16;	// =8
	uint32		SSRC;
	uint8		lossRate;
	uint8		discardRate;
	uint8		burstDensity;
	uint8		gapDensity;
	uint16		burstDuration;
	uint16		gapDuration;
	uint16		roundTripDelay;
	uint16		endSystemDelay;
	int8		signalLevel;
	int8		noiseLevel;
	uint8		RERL;					// residual echo return loss 
	uint8		Gmin;					// gap threshold. ETSI TS 101 329-5 
	uint8		Rfactor;				// ITU-T G.107 and ETSI TS 101 329-5
	uint8		extRfactor;				// ITU-T G.107 and ETSI TS 101 329-5 
	uint8		MOS_LQ;
	uint8		MOS_CQ;
	union {
		uint8	all;
		struct {
			uint8	PLC			:2;		// 11: standard, 10: enhanced, 01: disabled, 00: unspecified 
			uint8	JBA			:2;		// 11: adaptive, 10: non-adaptive, 01: reserved, 00: unknown
			uint8	JBrate		:4;		// rate = 0 - 15, adjustment time = 2 * J * frame size (ms)
		};
	} RXconfig;
	uint8		reserved2;
	uint16		JBnominal;
	uint16		JBmaximum;
	uint16		JBabsMax;
} RtcpXRReportVoIPMetrics;


/* ----------------------------------------------------------------- */
/* --- RTCP XR Structures for Management --------------------------- */
/* ----------------------------------------------------------------- */

#define MAX_NUM_XR_LOSS		1024
#define MAX_NUM_XR_DUP		1024	// should less or equal to loss  
#define MAX_NUM_XR_ARRIVE	64		// should be small (occupies 64*4 bytes in packet)

#define LOSS_THINNING		0		// report 2 ^ T (T=0~15)
#define DUP_THINING			0		// report 2 ^ T (T=0~15)
#define ARRIVE_THINING		0		// report 2 ^ T (T=0~15)

#define XR_GMIN				16

typedef struct {	// This is a memeber of RtpReceiver 
	// ----------------------------------------------------
	// periodic statistics (4.1, 4.2, 4.3, 4.6)
	// ----------------------------------------------------
	// flag
	int set_periodic;	// determine first one rtp or not - begin_seq ~ end_seq 
	
	// sequence
	uint32 begin_seq;
	uint32 end_seq;
	
	// arrive 
	uint32 arrive_timestamp[ MAX_NUM_XR_ARRIVE ];	// use loss_bit to check valid 
	
	// loss  
	uint32 loss_bits[ MAX_NUM_XR_LOSS / 32 ];	// 1: arrive, 0: loss
	uint32 loss_count;
	
	// duplicate 
	uint32 dup_bits[ MAX_NUM_XR_DUP / 32 ];		// 0: duplicate 
	uint32 dup_count;
	
	// jitter 
	uint32 min_jitter;
	uint32 max_jitter;
	uint32 sum_jitter;
	uint32 squ_jitter;
	
	// TTL 
	int min_ttl;
	int max_ttl;
	uint32 sum_ttl;
	uint32 squ_ttl;

	// ----------------------------------------------------
	// global - A.1. extend seq use 
	// ----------------------------------------------------
	uint32 _src_ref_seq;
	int _uninitialized_src_ref_seq;

	// ----------------------------------------------------
	// global - A.2. burst packet loss calculation  
	// ----------------------------------------------------
	uint32 A2_loss_count;
	uint32 A2_discard_count;
	uint32 A2_pkt;
	uint32 A2_lost;
	uint32 A2_c11, A2_c13, A2_c14, A2_c22, A2_c23, A2_c33;

	// ----------------------------------------------------
	// global statistics (4.7)
	// ----------------------------------------------------
	// flag
	int set_global;
	
	// sequence
	uint32 global_begin_seq;
	
	// loss
	//uint32 global_loss_count;
	
	// discard
	//uint32 global_discard_count;
		
} RtpReceiverXR;

#endif /* __RTP_TYPES_XR_H__ */

