//#include "rtpTypes.h"
#include "voip_init.h"
#include "voip_proc.h"
#include "rtpTools.h"
#include "Rtp.h"
#include "RtpReceiverXR.h"

#include "codec_descriptor.h"

#include "v152_api.h"

///////////////////////////////////////////////////////////////
// static function
//int updateSource (RtpPacket* p);
int addSource(RtpPacket* p);
void initSource (RtpPacket* p);
void removeSource (RtpSrc s, uint32 sid, int flag);
static RtpPacket* getPacket (void);
static void freePacket (RtpPacket* p);

///////////////////////////////////////////////////////////////
//static variable
//static BOOL doneCatchup = FALSE;    // is this need? - kle

static int cur_get;		/// point to the current free RX buffer

///////////////////////////////////////////////////////////////

RtpReceiver RtpRxInfo[MAX_DSP_RTK_SS_NUM];
extern unsigned char rfc2833_dtmf_pt_local[];
extern unsigned char rfc2833_fax_modem_pt_local[];
extern int g_dynamic_pt_local[];
extern int g_dynamic_pt_local_vbd[];
void RtpRx_rtp2ntp (uint32 sid, RtpTime rtpTime, NtpTime* ntpTime);

#if 1
static inline void rtcp_dd_write( const char *format, ... )	{}
#else
extern void rtcp_dd_write( const char *format, ... );
#endif

void RtpRx_Init(void)
{
	// no transmitter
	int i;
	cur_get = 0;
	for(i=0; i<DSP_RTK_SS_NUM; i++)
	{
		RtpRx_InitbyID(i);
	}
}

void RtpRx_InitbyID(uint32 sid)
{
	RtpReceiver* pInfo = NULL;

	if(sid >= DSP_RTK_SS_NUM)
		return;

	pInfo = &RtpRxInfo[sid];

	pInfo->sourceSet = FALSE;
	pInfo->ssrc = 0;
	pInfo->probationSet = FALSE;
	pInfo->srcProbation = 0;
	pInfo->probation = -2;
//	recvOpmode[i] = rtprecv_normal;
	pInfo->recvOpmode = rtprecv_droppacket;
	
	// inter jitter 
	pInfo->interjitter.First = 1;
	pInfo->interjitter.Di_1 = 0;
	pInfo->interjitter.Ji_Q4 = 0;
#if 0
	pInfo->transit = 0;
	pInfo->jitter = 0;
#endif
	
	
//		prevPacket = NULL;
#ifdef SUPPORT_RTCP
	/*pInfo->rtcpRecv = NULL;*/
	pInfo->rtcpRecv = RtcpRx_getInfo(sid);
#endif
}

void RtpRx_renewSession(uint32 sid)
{
	RtpReceiver* pInfo = NULL;

	if(sid >= DSP_RTK_SS_NUM)
		return;

	pInfo = &RtpRxInfo[sid];

	// no transmitter
	pInfo->sourceSet = FALSE;
	pInfo->ssrc = 0;
	pInfo->probationSet = FALSE;
	pInfo->srcProbation = 0;
	pInfo->probation = -2;
//	pInfo->recvOpmode = rtprecv_normal;
	
	// inter jitter 
	pInfo->interjitter.First = 1;
	pInfo->interjitter.Di_1 = 0;
	pInfo->interjitter.Ji_Q4 = 0;
#if 0
	pInfo->transit = 0;
	pInfo->jitter = 0;
#endif
}

#if 0
RtpReceiver_Close()
{
    if (freeStack)
    {
        delete myStack;
        myStack = NULL;
    }
    rtcpRecv = NULL;

#ifdef DEBUG_LOG
    cpLog (LOG_DEBUG_STACK, "Close receiver");
#endif
}
#endif

RtpReceiver* RtpRx_getInfo (uint32 sid)
{
    RtpReceiver* pInfo = NULL;

	if(sid >= DSP_RTK_SS_NUM)
		return NULL;

	pInfo = &RtpRxInfo[sid];
	return pInfo;
}

/* --- receive packet functions ------------------------------------ */

RtpPacket* Rtp_receive (void)
{
	extern RtpTime RtpRx_ntp2rtp (uint32 sid, NtpTime* ntpTime);
	RtpReceiver* pInfo = NULL;

	RtpPacket* p = NULL;
	int len = 0;
	uint32 sid;
#ifdef SUPPORT_RTCP
	uint32 absDij = 0;
	int32 Di;
	uint32 Ji_1_Q4 = 0;
	RtpTime Si, Ri;
#endif
#ifdef SUPPORT_RTCP
	//int packetTransit = 0;
	//int delay = 0;
	// empty network que
	NtpTime arrival;//, ntptime;
	Ntp_TimeInitNull(&arrival);
#endif
	while (1) // network empty or time to play return packet
	{
		p = getPacket();

		if (p == NULL)
			break;

		sid = p->sid;

		/* add by sandro*/
		pInfo = &RtpRxInfo[sid];

		if(!isRecvMode(sid))
		{
			freePacket(p);
			break;
		}

		// only play packets for valid sources
		if (pInfo->probation < 0)
		{
#ifdef DEBUG_LOG
			cpLog(LOG_ERR, "Packet from invalid source");
#endif
//			delete p;
			p->own = OWNBYDSP;
			p = NULL;
			continue;
        }
#ifdef SUPPORT_RTCP
		Ntp_getTime(&arrival);
#endif

		len = getPayloadUsage(p);
		if (len <= 0 || len > 1012)
		{
//#ifdef DEBUG_LOG
//			cpLog(LOG_DEBUG_STACK, "Got an invalid packet size");
			printk("Got an invalid packet size!\n");
//#endif
//			delete p;
			p->own = OWNBYDSP;
			p = NULL;
			continue;
		}

		// bypass jitterBuffer
//		if (jitterTime == -1)
		{
			// update counters
			pInfo->prevSeqRecv = getSequence(p);
			pInfo->prevSeqPlay = getSequence(p);
			if(pInfo->prevSeqRecv == 0)
				pInfo->recvCycles += RTP_SEQ_MOD;

			// update packet received
			pInfo->packetReceived++;
			pInfo->payloadReceived += getPayloadUsage(p);

#ifdef SUPPORT_RTCP
			// update jitter calculation
#if 0
			RtpRx_rtp2ntp(sid, getRtpTime(p), &ntptime);
			packetTransit = NTP_sub(&arrival, &ntptime);
			delay = packetTransit - pInfo->transit;
			pInfo->transit = packetTransit;
			if (delay < 0) 
				delay = -delay;
			pInfo->jitter += delay - ((pInfo->jitter + 8) >> 4);
#endif
			Si = getRtpTime(p);
			Ri = RtpRx_ntp2rtp( sid, &arrival );
			
			Di = Ri - Si;	// R(i) - S(i)
			
			if( pInfo ->interjitter.First ) {
				pInfo ->interjitter.First = 0;
			} else {
				// Dij = abs( D(i) - D(i-1) )
				absDij = ( Di > pInfo ->interjitter.Di_1 ?
							Di - pInfo ->interjitter.Di_1 :
							pInfo ->interjitter.Di_1 - Di );
				
				Ji_1_Q4 = pInfo ->interjitter.Ji_Q4;
				
				// J(i) = J(i-1) * 15/16 + Dij * 1/16
				//      = J(i-1) + ( Dij - J(i-1) ) * 1/16     (signed integer)
				//      = J(i-1) - J(i-1) * 1/16 + Dij * 1/16  (unsigned integer)
				// 
				// J_Q4(i) = J_Q4(i-1) - J_Q4(i-1) * 1/16 + Dij_Q4 * 1/16
				//         = J_Q4(i-1) - J_Q4(i-1) * 1/16 + Dij 
				//
				pInfo ->interjitter.Ji_Q4 = 
								pInfo ->interjitter.Ji_Q4 - 
								( ( pInfo ->interjitter.Ji_Q4 + 8 ) >> 4 ) +	// +8 to round up or down 
								( absDij );
			}
			
			pInfo ->interjitter.Di_1 = Di;	// store D(i) as D(i-1)
			
			if( absDij < 160 && ( getSequence( p ) & 0xFF ) != 9 )	// 9 is magic 
				goto label_skip_IJ_log;
			
			rtcp_dd_write( "IJ: %s:%d %lu \n"
						"\tgetRtpTime(p)=%lu, Si=%lu\n"
						"\tarrival(%X.%X), Ri=%lu\n"
						"\tDi    =%ld\n"
						"\tabsDij=%lu\n"
						"\tpInfo ->interjitter.Di_1=%ld\n"
						"\tpInfo ->interjitter.Ji_Q4=%lu\n"
						"\tJi_1_Q4=%lu\n", 
						__FUNCTION__, __LINE__, sid, 
						getRtpTime(p), Si,
						arrival.seconds, arrival.fractional, Ri, 
						Di,
						absDij,
						pInfo ->interjitter.Di_1,
						pInfo ->interjitter.Ji_Q4,
						Ji_1_Q4 );

label_skip_IJ_log:
			;
#endif	// SUPPORT_RTCP

#ifdef CONFIG_RTK_VOIP_RTCP_XR
			Rtp_receive_XR( p, &pInfo ->xr, absDij, Ri );
#endif

			return p;
		}
	}
	return NULL;
}

static RtpPacket* getPacket (void)
{
	RtpPacket* p;
	p = &RTP_RX_DEC[cur_get];
	if(p->own == OWNBYDSP)
		p = NULL;
	else
	{
		cur_get++;
		cur_get &= (RTP_RX_DEC_NUM - 1);
	}
//	printk("cur_get = %d\n", cur_get);

	return p;
}

static void freePacket (RtpPacket* p)
{
	if(p->own == OWNBYRTP)
		p->own = OWNBYDSP;
}

KnownPayloadType checkIfKnownPayloadType( uint32 sid, RtpPayloadType pt,
					uint32 bPrimary, /* Redundancy primary packet? Normal packet is also primary! */
					int bIncSIDcount /* Increase SID counter? If we call this function more than one times, set this argument */
					)
{
	extern uint16 SID_payload_type_local[ DSP_RTK_SS_NUM ];
	extern uint32 SID_count_rx[ DSP_RTK_SS_NUM ];
	
	if( ( !SID_payload_type_local[ sid ] && pt == 13 ) ||
		( SID_payload_type_local[ sid ] && 
		  SID_payload_type_local[ sid ] == pt ) )
	{
		// SID: equal to case 13 
		//setPayloadType(packet, pInfo->payloadFormat);	// caller do this 
		if( bIncSIDcount )
			SID_count_rx[ sid ] ++;
		return KPT_SID;		// Silence 
	}
	
#ifdef SUPPORT_RTP_REDUNDANT
	if( GetRtpRedundantStatus( sid ) ) {
		if( pt == 
			GetRtpRedundantPayloadType( sid, 1 /* local */ ) )
		{
			return KPT_REDUNDANCY;	// RTP redundancy 
		}
	} 
#endif

	if (pt == g_dynamic_pt_local[sid]) {
#ifdef SUPPORT_V152_VBD
		if( bPrimary ) {
			V152_ConfirmStateTransition( sid, ST_AUDIO_ING );
			
			if( V152_StateTransition( sid, REASON_RTP_AUDIO ) ) {
				V152_SwitchCodecIfNecessary();
			}
		}
#endif
		return KPT_RTP_NORMAL;	// Normal voice, or V.152 Audio 
	}

#ifdef SUPPORT_V152_VBD		
	if (pt == g_dynamic_pt_local_vbd[sid]) {

		if( bPrimary ) {
			V152_ConfirmStateTransition( sid, ST_VBD_ING );
			
			if( V152_StateTransition( sid, REASON_RTP_VBD ) ) {
				V152_SwitchCodecIfNecessary();
			}
		}
		return KPT_RTP_VBD;		// V.152 VBD 
	}
#endif
		
	//if( pt < 96 && GetCodecPayloadDesc( pt ) )	// do we still need this??? 
	//	return KPT_OTHERS;			// Exist in predefined table? (static payload type) 

	if ((pt == rfc2833_dtmf_pt_local[sid]) || (pt == rfc2833_fax_modem_pt_local[sid]))
		return KPT_RFC2833;		// RFC 2833
		
#ifdef DEBUG_LOG
	cpLog (LOG_ERR, "Unknown payload type");
#endif
	return KPT_UNKNOWN;	
}

int isValid (RtpPacket* packet, KnownPayloadType *pKnownPT)
{
	RtpReceiver* pInfo = NULL;
	RtpPayloadType pt;
	int sid;

	if ( getTotalUsage (packet) <= 0)
		return 0;

	// check version
	if (getVersion(packet) != RTP_VERSION)
	{
#ifdef DEBUG_LOG
		cpLog(LOG_DEBUG_STACK, "Wrong RTP version");
#endif
		return 0;
	}

	if(packet->sid >= DSP_RTK_SS_NUM)
		return 0 ;

	pInfo = &RtpRxInfo[packet->sid];
	
	pt = getPayloadType(packet);
	sid = packet->sid;

	// check if known payload type
	switch( ( *pKnownPT = checkIfKnownPayloadType( sid, pt, 1, 1 ) ) ) {
	case KPT_UNKNOWN:
		return 0;

	case KPT_SID:			// Silence 
		setPayloadType(packet, pInfo->payloadFormat);
		break;
		
	case KPT_RTP_NORMAL:	// Normal voice, or V.152 Audio 
	case KPT_RTP_VBD:		// V.152 VBD 
	case KPT_RFC2833:		// RFC 2833 
	case KPT_REDUNDANCY:	// RTP redundancy 
	case KPT_OTHERS:		// Exist in predefined table? (static payload type) 
	default:
		break;
	}

	return 1;
}

int updateSource (RtpPacket* p)
{
	RtpReceiver* pInfo = NULL;
	int bSourceSet;

	if(p->sid >= DSP_RTK_SS_NUM)
		return 1;

	pInfo = &RtpRxInfo[p->sid];

	// check if ssrc in probation list
	if (pInfo->sourceSet && getSSRC(p) == pInfo->srcProbation && pInfo->probationSet)
	{
		// old probation packets still in que
		return 1;
	}

	// new source found or resync old source
	if (!pInfo->sourceSet || getSSRC(p) != pInfo->ssrc)
	{
		bSourceSet = pInfo->sourceSet;	/* sourceSet will be modified in addSource(p) */

#if 0	// avoid some abnormal RFC 2833 packet with SSRC == 0 
		if( bSourceSet &&
			getSSRC(p) == 0 &&
			getPayloadType(p) == rfc2833_dtmf_pt_local[p->sid] )
		{
			return 1;
		}
#endif
	
		if (addSource(p))
			return 2;
		else if( bSourceSet ) {
			/* When we enter this statement, it means SSRC is set and different. */
			extern void DspcodecWriteSnyc( uint32 sid );
			DspcodecWriteSnyc( p->sid );
		}
	}	

	// no vaild source yet
	assert (pInfo->probation >= 0);

	//if ((getPayloadType(p) != pInfo->payloadFormat) &&
	//	(getPayloadType(p) != rfc2833_payload_type_local[p->sid]))
	//{
	//	return 3;
	//}
	return 0;
}

int addSource(RtpPacket* p)
{
	RtpReceiver* pInfo = NULL;

	if(p->sid >= DSP_RTK_SS_NUM)
		return 1;

	pInfo = &RtpRxInfo[p->sid];

    // don't allow ssrc changes without removing first
	if (pInfo->sourceSet)
    {
		if (pInfo->probation < 4)
        {
			pInfo->probation ++;
#ifdef DEBUG_LOG
			cpLog(LOG_ERR, "Rejecting new transmitter %d, keeping ",
					getSSRC(p), pInfo->ssrc);
#endif
			return 1;
        }
		else
		{
			removeSource(p->sid, pInfo->ssrc, 0);
		}
	}

	// check if ssrc in probation list
	if (pInfo->sourceSet && getSSRC(p) == pInfo->srcProbation && pInfo->probationSet)
		return 1;

	pInfo->sourceSet = TRUE;
	pInfo->ssrc = getSSRC(p);
#ifdef DEBUG_LOG
	cpLog(LOG_DEBUG_STACK, "Received ssrc = %d", ssrc[sid]);
#endif
	pInfo->probation = 0;
	pInfo->packetReceived = 0;
	pInfo->payloadReceived = 0;

#ifdef CONFIG_RTK_VOIP_RTCP_XR
	RtpRx_InitXR( &pInfo ->xr );
#endif

	// init SDES and RTCP fields
//	if (rtcpRecv) rtcpRecv->addTranInfo(ssrc[sid], this);
#ifdef SUPPORT_RTCP
	RtcpRx_addTranInfo(p->sid, pInfo->ssrc, pInfo);
#endif
	initSource (p);

	return 0;
}

void initSource (RtpPacket* p)
{
	RtpReceiver* pInfo = NULL;

    if(p->sid >= DSP_RTK_SS_NUM)
    	return;

	pInfo = &RtpRxInfo[p->sid];

    assert (pInfo->ssrc == getSSRC(p));


#ifdef DEBUG_LOG
    cpLog(LOG_DEBUG_STACK, "InitSource %d with sequence %d", ssrc[sid], getSequence(p));
#endif
	pInfo->seedSeq = getSequence(p);
#ifdef SUPPORT_RTCP
	Ntp_getTime(&pInfo->seedNtpTime);
        pInfo->seedRtpTime = getRtpTime(p);
#endif

//    inPos = 0;
//    playPos = 0;

    // set timing information
//    prevRtpTime = getRtpTime(p) - pktSampleSize/*network_pktSampleSize*/;
//    prevNtpTime = rtp2ntp(getRtpTime(p)) - _idiv32(_imul32(pktSampleSize, 1000), bitRate);
    //prevNtpTime = rtp2ntp(getRtpTime(p)) - _idiv32(_imul32(network_pktSampleSize, 1000), networkFormat_bitRate);
    pInfo->prevSeqRecv = getSequence(p) - 1;
    pInfo->prevSeqPlay = getSequence(p) - 1;
    pInfo->recvCycles = 0;
    pInfo->playCycles = 0;

//    transit = 0;
//    jitter = 0;
//    jitterTime = jitterSeed;

    // set up next gotime
//    gotime = rtp2ntp (getRtpTime(p)) + jitterTime;
}

void removeSource (uint32 sid, RtpSrc s, int flag)
{
	RtpReceiver* pInfo = NULL;

    if(sid >= DSP_RTK_SS_NUM)
    	return;

	pInfo = &RtpRxInfo[sid];


#ifdef DEBUG_LOG
    if (s != pInfo->ssrc)
        cpLog(LOG_DEBUG_STACK, "Removing non active source: %d", s);
#endif

    // no longer listen to this source
    pInfo->probationSet = TRUE;
    pInfo->srcProbation = s;

    // no transmitter
    pInfo->sourceSet = FALSE;
    pInfo->ssrc = 0;
    pInfo->probation = -2;

    // remove from RTCP receiver
//    if (rtcpRecv && !flag) rtcpRecv->removeTranInfo (s, 1);
#ifdef SUPPORT_RTCP
	RtcpRx_removeTranInfo (sid, s, 1);
#endif

#ifdef DEBUG_LOG
    cpLog (LOG_DEBUG_STACK, "Removing source: %d", s);
#endif
}

#ifdef SUPPORT_RTCP
/*NtpTime RtpRx_rtp2ntp (uint32 sid, RtpTime rtpTime)*/
void RtpRx_rtp2ntp (uint32 sid, RtpTime rtpTime, NtpTime* ntpTime)
{
	RtpReceiver* pInfo = NULL;
/*    if(sid >= SESS_NUM)
    	return 0; */
	pInfo = &RtpRxInfo[sid];
/*    NtpTime ntptime;*/
	NTP_addms(&pInfo->seedNtpTime, _idiv32(_imul32((rtpTime - pInfo->seedRtpTime), 1000), pInfo->bitRate/*apiFormat_bitRate*/), ntpTime);
/*    return ntptime;*/
}
#endif

#ifdef SUPPORT_RTCP
RtpTime RtpRx_ntp2rtp (uint32 sid, NtpTime* ntpTime)
{
	RtpReceiver* pInfo = NULL;
	int delta_ms;
	RtpTime /*rtpNow,*/ delta_rtp;
	
	pInfo = &RtpRxInfo[sid];
	
	//delta_ms = NTP_sub( &pInfo->seedNtpTime, ntpTime );
	delta_ms = NTP_sub( ntpTime, &pInfo->seedNtpTime );
	
	delta_rtp = delta_ms * ( pInfo->bitRate / 1000 );
	
	return pInfo->seedRtpTime + delta_rtp;
}
#endif

/* --- Private Information for RTCP -------------------------------- */
#ifdef SUPPORT_RTCP
void RtpRx_setRTCPrecv (uint32 sid, RtcpReceiver* s)
{
	RtpReceiver* pInfo = NULL;

    if(sid >= DSP_RTK_SS_NUM)
    	return;

	pInfo = &RtpRxInfo[sid];
    pInfo->rtcpRecv = s;
}
#endif

void RtpRx_setFormat (uint32 sid, RtpPayloadType newtype, int frameRate)
{
	RtpReceiver* pInfo = NULL;
	const codec_payload_desc_t *pCodecPayloadDesc;

    if(sid >= DSP_RTK_SS_NUM)
    	return;

	pInfo = &RtpRxInfo[sid];

	pInfo->payloadFormat = newtype;

	pCodecPayloadDesc = GetCodecPayloadDesc( newtype );
	
	if( pCodecPayloadDesc ) {
		pInfo->bitRate = pCodecPayloadDesc ->nTimestampRate;
		pInfo->payloadSize = 
			_imul32(pCodecPayloadDesc ->nFrameBytes, 
					_idiv32(frameRate, pCodecPayloadDesc ->nRecvFrameRate));
	} else {
		pInfo->bitRate = 8000;
		pInfo->payloadSize = 160;
	}

	//pInfo->pktSampleSize = _idiv32(_imul32(pInfo->bitRate, frameRate), 1000000);
}

void RtpRx_setMode(uint32 sid, RtpReceiverMode opmode)
{
	RtpReceiver* pInfo = NULL;

    if(sid >= DSP_RTK_SS_NUM)
    	return;

	pInfo = &RtpRxInfo[sid];
	pInfo->recvOpmode = opmode;
}

int32 isRecvMode(uint32 sid)
{
	RtpReceiver* pInfo = NULL;

    if(sid >= DSP_RTK_SS_NUM)
    	return -1;

	pInfo = &RtpRxInfo[sid];
	if(pInfo->recvOpmode == rtprecv_droppacket)
		return 0;
	return 1;
}

// ------------------------------------------------------------------
// proc 
// ------------------------------------------------------------------

static int voip_rtp_receiver_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
	int ss;
	int n = 0;
	const RtpReceiver * pRtpReceiver;

	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}
	
	if( IS_CH_PROC_DATA( data ) ) {
		//ch = CH_FROM_PROC_DATA( data );
		//n = sprintf( buf, "channel=%d\n", ch );
	} else {
		ss = SS_FROM_PROC_DATA( data );
		pRtpReceiver = &RtpRxInfo[ ss ];
		
		n = sprintf( buf, "session=%d\n", ss );
		
		n += sprintf( buf + n, "prev seq: recv=%u play=%u\n", 
						pRtpReceiver ->prevSeqRecv, pRtpReceiver ->prevSeqPlay );
		n += sprintf( buf + n, "SSRC: %u, set=%d\n",
						pRtpReceiver ->ssrc, pRtpReceiver ->sourceSet );
		n += sprintf( buf + n, "Probation: ssrc=%u set=%d count=%d\n",
						pRtpReceiver ->srcProbation, pRtpReceiver ->probationSet,
						pRtpReceiver ->probation );
		n += sprintf( buf + n, "Seed: seq=%u ntp=(%u,%u), rtp=%u\n", 
						pRtpReceiver ->seedSeq, 
						pRtpReceiver ->seedNtpTime.seconds, 
						pRtpReceiver ->seedNtpTime.fractional, 
						pRtpReceiver ->seedRtpTime );
		n += sprintf( buf + n, "Received: packet=%u, byte=%u\n",
						pRtpReceiver ->packetReceived, pRtpReceiver ->payloadReceived );
		n += sprintf( buf + n, "Cycles: recv=%u, play=%u\n",
						pRtpReceiver ->recvCycles, pRtpReceiver ->playCycles );
		n += sprintf( buf + n, "Format: bitrate=%d, payloadtype=%d, "
								"payloadsize=%d\n",
						pRtpReceiver ->bitRate, pRtpReceiver ->payloadFormat, 
						pRtpReceiver ->payloadSize );
		n += sprintf( buf + n, "RecvMode: %u\n", pRtpReceiver ->recvOpmode );
	}
	
	*eof = 1;
	return n;
}

int __init voip_rtp_receiver_proc_init( void )
{
	create_voip_session_proc_read_entry( "rtpreceiver", voip_rtp_receiver_read_proc );

	return 0;
}

void __exit voip_rtp_receiver_proc_exit( void )
{
	remove_voip_session_proc_entry( "rtpreceiver" );
}

voip_initcall_proc( voip_rtp_receiver_proc_init );
voip_exitcall( voip_rtp_receiver_proc_exit );


