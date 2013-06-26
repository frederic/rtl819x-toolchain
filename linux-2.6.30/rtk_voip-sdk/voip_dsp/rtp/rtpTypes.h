#ifndef RTPTYPES_H
#define RTPTYPES_H

/* ====================================================================
 * The Vovida Software License, Version 1.0 
 * 
 * Copyright (c) 2000 Vovida Networks, Inc.  All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 
 * 3. The names "VOCAL", "Vovida Open Communication Application Library",
 *    and "Vovida Open Communication Application Library (VOCAL)" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact vocal@vovida.org.
 *
 * 4. Products derived from this software may not be called "VOCAL", nor
 *    may "VOCAL" appear in their name, without prior written
 *    permission of Vovida Networks, Inc.
 * 
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL VOVIDA
 * NETWORKS, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT DAMAGES
 * IN EXCESS OF $1,000, NOR FOR ANY INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 * 
 * ====================================================================
 * 
 * This software consists of voluntary contributions made by Vovida
 * Networks, Inc. and many individuals on behalf of Vovida Networks,
 * Inc.  For more information on Vovida Networks, Inc., please see
 * <http://www.vovida.org/>.
 *
 */

//static const char* const rtpTypes_h_Version =
//    "$Id: rtpTypes.h,v 1.3 2008-01-15 06:33:23 kennylin Exp $";

//#include <sys/types.h>
//#include <unistd.h>
//#ifdef KBUILD_BASENAME
#include <linux/types.h>
//#else
//#include <sys/types.h>
//#endif

#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_params.h"
#include "NtpTime.h"
#include "rtpTypesXR.h"
#include "codec_descriptor.h"
//typedef unsigned short		u_int16_t;
//typedef unsigned int		u_int32_t;


// these types defined in NtpTime.h
//#define _idiv32(x, y)		((x) / (y))
//#define _imul32(x, y)		((x) * (y))
//#define _imod32(x, y)		((x) % (y))


/// Version of this RTP, always 2
//const int RTP_VERSION = 2;
#define RTP_VERSION 2

#define RTP_SEQ_MOD_SHIFT	16
#define RTP_SEQ_MOD 		(1<<RTP_SEQ_MOD_SHIFT)
//const int RTP_SEQ_MOD = 1 << 16;

/// Maximum UDP packet size, 8129
//const int RTP_MTU = 8129;

/// 32-bit sequence number
typedef u_int16_t RtpSeqNumber;

/// Middle 32-bit of NTP
typedef u_int32_t RtpTime;

/// 32-bit source number
typedef u_int32_t RtpSrc;


#ifdef _LITTLE_ENDIAN
???
#else /*Big endian system */

#if 1 
//#ifdef __linux__
	#undef ntohs
	#undef ntohl
	#undef htons
	#undef htonl
	#define ntohs(x)   (x)
	#define ntohl(x)   (x) 
	#define htons(x)   (x)
	#define htonl(x)   (x)
	#define _LINUX_BYTEORDER_GENERIC_H
#endif /* __linux__ */	
	#define NTOHL(d) ((d) = ntohl(d))
	#define NTOHS(d) ((d) = ntohs((uint16)(d)))
	#define HTONL(d) ((d) = htonl(d))
	#define HTONS(d) ((d) = htons((uint16)(d)))
	#define GET_UINT16_UNALIGNED( ptr )  			GET_UINT16_BIG_ENDIAN_UNALIGNED(ptr)
	#define SET_UINT16_UNALIGNED(  u16value, ptr )	SET_UINT16_BIG_ENDIAN_UNALIGNED(  u16value, ptr )
	#define GET_UINT32_UNALIGNED( ptr )  			GET_UINT32_BIG_ENDIAN_UNALIGNED(ptr)
	#define SET_UINT32_UNALIGNED(  u32value, ptr )	SET_UINT32_BIG_ENDIAN_UNALIGNED(  u32value,ptr)
	#define PKTGET_UINT16_UNALIGNED( ptr )  			GET_UINT16_BIG_ENDIAN_UNALIGNED(ptr)
	#define PKTSET_UINT16_UNALIGNED(  u16value, ptr)	SET_UINT16_BIG_ENDIAN_UNALIGNED(  u16value, ptr )
	#define PKTGET_UINT32_UNALIGNED( ptr )  			GET_UINT32_BIG_ENDIAN_UNALIGNED(ptr)
	#define PKTSET_UINT32_UNALIGNED(  u32value, ptr)	SET_UINT32_BIG_ENDIAN_UNALIGNED(  u32value,ptr)	
#endif

/* ----------------------------------------------------------------- */
/* --- RTP Structures ---------------------------------------------- */
/* ----------------------------------------------------------------- */

typedef void* RtpPacketPtr;
typedef void* RtpReceiverPtr;
typedef void* RtpTransmitterPtr;

struct stRtpReceiver;


/// RTP packet header
struct RtpHeaderStruct
{
#ifdef __LITTLE_ENDIAN
u_int32_t count:
    4;
u_int32_t extension:
    1;
u_int32_t padding:
    1;
u_int32_t version:
    2;
u_int32_t type:
    7;
u_int32_t marker:
    1;
#else //__BIG_ENDIAN
    /// protocal version
u_int32_t version:
    2;
    /// padding flag - for encryption
u_int32_t padding:
    1;
    /// header extension flag
u_int32_t extension:
    1;
    /// csrc count
u_int32_t count:
    4;
    /// marker bit - for profile
u_int32_t marker:
    1;
    /// payload type
u_int32_t type:
    7;
#endif

    /// sequence number of this packet
u_int32_t sequence :
    16;
    /// timestamp of this packet
    RtpTime timestamp;
    /// source of packet
    RtpSrc ssrc;
    /// list of contributing sources
    RtpSrc startOfCsrc;
};
typedef struct RtpHeaderStruct RtpHeader;

// RTP redundant additional header (big-endian only)
typedef struct {
	uint32 F: 1;		// 1 further header block follow, 0 last header block 
	uint32 blockPT: 7;	// payload type for this block 
	uint32 timestamp_offset: 14; // timestamp = primary - offset 
	uint32 block_length: 10;
} RtpRedHeader;

typedef struct {
	uint8 F: 1;		// 1 further header block follow, 0 last header block 
	uint8 blockPT: 7;	// payload type for this block 
} RtpRedPrimaryHeader;

// Transmitter errors
typedef enum
{
    tran_success = 0
} RtpTransmitterError;


// Receiver errors
typedef enum
{
    recv_success = 0,
    recv_bufferEmpty = 20,
    recv_lostPacket = 21
} RtpReceiverError;

// Operation mode
typedef enum
{
    rtprecv_normal,
    rtprecv_droppacket
} RtpReceiverMode;

typedef enum
{
    rtptran_normal,
    rtptran_droppacket
} RtpTransmitMode, RtcpTransmitMode;

typedef enum
{
	rtpCN_withCodec = 0,
	rtpCN_withAll
} RtpTranCNMode;

#if 1
/* ----------------------------------------------------------------- */
/* --- RTCP Structures --------------------------------------------- */
/* ----------------------------------------------------------------- */

typedef void* RtcpPacketPtr;
typedef void* RtcpReceiverPtr;
typedef void* RtcpTransmitterPtr;


// Supported RTCP types
typedef enum
{
    rtcpTypeSR = 200,
    rtcpTypeRR = 201,
    rtcpTypeSDES = 202,
    rtcpTypeBYE = 203,
    rtcpTypeAPP = 204,             // not implemented
#ifdef CONFIG_RTK_VOIP_RTCP_XR
    rtcpTypeXR = 207,		// RFC3611: RTCP XR
#endif
} RtcpType;


// Supported SDES types
typedef enum
{
    rtcpSdesEnd = 0,
    rtcpSdesCname = 1,
    rtcpSdesName = 2,
    rtcpSdesEmail = 3,
    rtcpSdesPhone = 4,
    rtcpSdesLoc = 5,
    rtcpSdesTool = 6,
    rtcpSdesNote = 7,
    rtcpSdesPriv = 8              // not implemented
} RtcpSDESType;



/// RTCP header
struct RtcpHeaderStruct
{
#ifdef __LITTLE_ENDIAN
u_int32_t count:
    5;
u_int32_t padding:
    1;
u_int32_t version:
    2;
#else //__BIG_ENDIAN
/// protocal version
u_int32_t version:
    2;
    /// padding flag
u_int32_t padding:
    1;
    /// depending on packet type
u_int32_t count:
    5;
#endif
    /// type of RTCP packet
u_int32_t type:
    8;
    /// lenght of RTCP packet in octlets minus 1
u_int32_t length:
    16;
};
typedef struct RtcpHeaderStruct RtcpHeader;


/// report block
struct RtcpReportStruct
{
    /// source being reported
    RtpSrc ssrc;

    /* endian problem here? - kle */
    /// fraction lost since last report
u_int32_t fracLost:
    8;
    /// cumulative packet lost - signed
    unsigned char cumLost[3];

    /// number of cycles
u_int32_t recvCycles:
    16;
    /// last seq number received
u_int32_t lastSeqRecv:
    16;
    /// interval jitter (timestamp units)
    u_int32_t jitter;
    /// last SR (LSR) packet received from ssrc (NTP32: 32 bits out of 64 bits NTP)
    u_int32_t lastSRTimeStamp;
    /// delay since last SR (DLSR) packet (NTP32: 1/65536 seconds, same as LSR)
    u_int32_t lastSRDelay;
};
typedef struct RtcpReportStruct RtcpReport;


/// sender info
struct RtcpSenderStruct
{
    /// source of sender
    RtpSrc ssrc;
    /// seconds of NTP
    RtpTime ntpTimeSec;
    /// fractional seconds of NTP
    RtpTime ntpTimeFrac;
    /// transmitter RTP timestamp
    RtpTime rtpTime;
    /// number of packets sent
    u_int32_t packetCount;
    /// number of octlets sent
    u_int32_t octetCount;
};
typedef struct RtcpSenderStruct RtcpSender;


/// bye reason item
struct RtcpByeStruct
{
    /// lenght of text
    u_int8_t length;
    /// reason for leaving, not null-term
    char startOfText;
};
typedef struct RtcpByeStruct RtcpBye;


/// source descrp item
struct RtcpSDESItemStruct
{
    /// type of description
    u_int8_t type;
    /// lenght of item
    u_int8_t length;
    /// text description not null-term
    char startOfText;
};
typedef struct RtcpSDESItemStruct RtcpSDESItem;


/// source descrp chunk
struct RtcpChunkStruct
{
    /// source being described
    RtpSrc ssrc;
    /// list of SDES information, ending with rtcpSdesEnd
    RtcpSDESItem startOfItem;
};
typedef struct RtcpChunkStruct RtcpChunk;


/// SDES information
struct SDESdataStruct
{
    /// CNAME for this source
    char cname [256];
    /// NAME for this source
    char name [256];
    /// EMAIL for this source
    char email [256];
    /// PHONE for this source
    char phone [256];
    /// LOC for this source
    char loc [256];
    /// TOOL for this source
    char tool [256];
    /// NOTE for this source
    char note [256];
};
typedef struct SDESdataStruct SDESdata;

#endif

#if 1
/// receiver information
struct RtpTranInfoStruct
{
	int balloc;

    /// SSRC number for recv
    RtpSrc ssrc;

    struct stRtpReceiver* recv;  /* pointer to receiver for specific information */
    /// number packets expected in last interval
    int expectedPrior;
    /// number of packets actually received in last RTCP interval
    int receivedPrior;

    /// SDES information
    SDESdata SDESInfo;

    /// LSR timestamp which will be one of the fields of the next SR sent out
    u_int32_t lastSRTimestamp;	// (NTP32: 32 bits out of 64 bits NTP)

    /// receiveing time of the last SR received
    NtpTime recvLastSRTimestamp;

#ifdef CONFIG_RTK_VOIP_RTCP_XR    
	// last RR information for XR 
	NtpTime recvLastRRTimestamp;
#endif
};
typedef struct RtpTranInfoStruct RtpTranInfo;
#endif



/* ----------------------------------------------------------------- */
/* --- RTP Session ------------------------------------------------- */
/* ----------------------------------------------------------------- */




// Session errors
typedef enum
{
    session_success = 0,
    session_wrongState = 20
} RtpSessionError;


/* ----------------------------------------------------------------- */
/* --- RTP Events -------------------------------------------------- */
/* ----------------------------------------------------------------- */


struct RtpEventDTMFRFC2833Struct
{
u_int32_t event:
    8;
#ifdef __LITTLE_ENDIAN
u_int32_t volume:
    6;
u_int32_t reserved:
    1;
u_int32_t edge:
    1;
#else
u_int32_t edge:
    1;
u_int32_t reserved:
    1;
u_int32_t volume:
    6;
#endif
u_int32_t duration:
    16;
};
typedef struct RtpEventDTMFRFC2833Struct RtpEventDTMFRFC2833;

#if 0
struct RtpEventDTMFCiscoRtp
{
u_int32_t sequence:
    8;
#if __BYTE_ORDER == __LITTLE_ENDIAN
u_int32_t level:
    5;
u_int32_t reserved0:
    3;
u_int32_t edge:
    5;
u_int32_t digitType:
    3;
u_int32_t digitCode:
    5;
u_int32_t reserved1:
    3;
#elif __BYTE_ORDER == __BIG_ENDIAN
u_int32_t reserved0:
    3;
u_int32_t level:
    5;
u_int32_t digitType:
    3;
u_int32_t edge:
    5;
u_int32_t reserved1:
    3;
u_int32_t digitCode:
    5;
#else
#error "Problem in <endian.h>"
#endif
};
typedef struct RtpEventDTMFCiscoRtpStruct RtpEventCiscoRtp;
#endif
        
enum DTMFEvent
{
    DTMFEventNULL = -1,
    DTMFEventDigit0,
    DTMFEventDigit1,
    DTMFEventDigit2,
    DTMFEventDigit3,
    DTMFEventDigit4,
    DTMFEventDigit5,
    DTMFEventDigit6,
    DTMFEventDigit7,
    DTMFEventDigit8,
    DTMFEventDigit9,
    DTMFEventDigitStar,
    DTMFEventDigitHash
};

enum KeyEvent
{
    KeyEventNULL,
    KeyEventOn,
    KeyEventEdge,
    KeyEventOff
};


/* ----------------------------------------------------------------- */
/* --- RTCP Receiver and Transmitter Statistics Logger Struct ---------------- */
/* ----------------------------------------------------------------- */
typedef struct {
	unsigned long max;
	unsigned long min;
	unsigned long sum;		// avg = sum / count 
	unsigned long count;
	unsigned long last;
} RtcpStatisticsUnit;

typedef struct {
	unsigned long packet_count;
	RtcpStatisticsUnit fraction_loss;	// RTCP 
	RtcpStatisticsUnit inter_jitter;	// RTCP 
	RtcpStatisticsUnit round_trip_delay;	// RTCP XR 4.7.3 
	RtcpStatisticsUnit MOS_LQ;				// RTCP XR 4.7.5 
} RtcpStatisticsLogger;

/* ----------------------------------------------------------------- */
/* --- RTP and RTCP Receiver and Transmitter Struct ---------------- */
/* ----------------------------------------------------------------- */
#define MAX_TRANINFO_LIST 2	//16	/* I think that each seesion has only one SSRC */

///Struct to transmit RTCP packets
typedef struct stRtcpReceiver
{
	/* list of known sources */
/*	map < RtpSrc, RtpTranInfo* > tranInfoList;*/
	struct RtpTranInfoStruct tranInfoList[MAX_TRANINFO_LIST];
	int tranInfoCnt;

	/* my UDP stack */
/*	UdpStack* myStack;*/
	int localPort;
	int remotePort;

	unsigned long packetReceived;
	
	// one way delay (correct only if two peers sync with wall clock)
	unsigned long accumOneWayDelay;				// ms
	unsigned long accumOneWayDelayCount;
	unsigned long avgOneWayDelay;
	
	// round trip delay 
	unsigned long accumRoundTripDelay;			// ms 
	unsigned long accumRoundTripDelayCount;
	unsigned long avgRoundTripDelay;
	
	RtcpStatisticsLogger rxLogger;
	
}RtcpReceiver;

typedef struct stRtpReceiver
{
	RtpSeqNumber prevSeqRecv;		/* previous sequence number received */
	RtpSeqNumber prevSeqPlay;		/* previous sequence numer played */
	BOOL sourceSet;					/* source found flag */
	RtpSrc ssrc;					/* SRC number for this source */
	BOOL probationSet;				/* probation set flag */
	RtpSrc srcProbation;			/* wouldn't listen to this source */
	int probation;					/* probation, 0 source valid */
	RtpSeqNumber seedSeq;			/* inital seqence number */

#if 1
	NtpTime seedNtpTime;			/* inital NTP timestamp */
	RtpTime seedRtpTime;        	/* inital RTP timestamp */
#endif

#if 0
	RtpTime sampleRate;				/* rtp interval */
	int baseSampleRate;				/* payload specific sample rate */
	RtpPacket* prevPacket;			/* previous packet */
	NtpTime gotime;					/* next packet play time */
#endif

	int packetReceived;				/* number of packets received */
	int payloadReceived;			/* number of bytes of payload received */

#if 0
	RtpTime prevRtpTime;			/* last RtpTime play */
	NtpTime prevNtpTime;        	/* last NtpTime play */
	RtpSeqNumber prevSeqRecv;		/* previous sequence number received */
	RtpSeqNumber prevSeqPlay;		/* previous sequence numer played */
#endif

	int recvCycles;					/* number of received sequence number cycles */
	int playCycles;					/* number of played sequence number cycles */

#if 1	// RFC 3550, page 33: interarrival jitter 
	struct {
		int First;					/* first RTP? */
		int32 Di_1;					/* D(i-1) = R(i-1) - S(i-1) */
		uint32 Ji_Q4;				/* J(i) */
	} interjitter;
	
#if 0	
	int transit;					/* relative transmission time for prev packet */
	int jitter;						/* estimated jitter */
#endif
	//int jitterTime;					/* jitter time in ms time */
	/* map<RtpSeqNumber, RtpPacket*> jitterBuffer; */ /* jitter buffer */
#endif

	RtcpReceiver* rtcpRecv;			/* additional SDES and RTCP information */
	int bitRate;					/* payload specific sample rate */
	RtpPayloadType payloadFormat; 	/* format of payload for stack */
	//int pktSampleSize;				/* number of samples per RTP packet on api (typical 160 or 240) */
	int payloadSize;				/* payload sizes */

#ifdef CONFIG_RTK_VOIP_RTCP_XR
	RtpReceiverXR xr;
#endif

#if 0
	int jitterSeed;					/* inital jitter size */
#endif
	RtpReceiverMode recvOpmode;
}RtpReceiver;

typedef struct stRtpTransmitter
{
	RtpSrc ssrc;					/* local SRC number */
	NtpTime seedNtpTime;			/* inital NTP time */
	RtpTime seedRtpTime;			/* inital RTP time */
	RtpTime prevRtpTime;			/* RTP time of previous packet */
	int     prevRtpTimeCalibration;	/* Calibrate RTP time when RtpTx_setFormat() */
	NtpTime prevNtpTime;			/* NTP time of previous packet */
	RtpSeqNumber prevSequence;		/* previous packet's sequence */
	int packetSent;					/* numbers of packet sent */
	int payloadSent;				/* total bytes of payload sent */
	RtpPayloadType payloadFormat;	/* format of payload for stack */
	const codec_payload_desc_t *pCodecPayloadDesc;	/* corresponding to payloadFormat (efficient access) */
	int pktSampleSize;				/* timestamp per tx packet */ /* WRONG: number of samples per RTP packet on api (typical 160 or 240) */
	int payloadSize;				/* payload sizes */
	BOOL markerOnce;				/* marker once */
	RtpTransmitMode tranOpmode;
	int bitRate;					/* number of bytes per sample  (typical 1 or 2, used for endian conversion) */

}RtpTransmitter;

///Struct to transmit RTCP packets
typedef struct stRtcpTransmitter
{

	/* Next time to submit RTCP packet */
	NtpTime nextInterval;
	
	unsigned int txInterval;	// ms 

	/** Transmitter SDES information
	 *  data stored as null-term strings
	**/
	SDESdata* SDESInfo;
	SDESdata SDESdataBuf[1];

	RtpTransmitter* tran;

	RtpReceiver* recv;

	RtcpReceiver* rtcpRecv;

	/* my UDP stack */
/*	UdpStack* myStack; */
	int localPort;
	int remotePort;
	
	RtcpStatisticsLogger txLogger;
	
	RtcpTransmitMode txMode;
	
/*	NetworkAddress remoteAddr; */
}RtcpTransmitter;


#endif // RTPTYPES_H
