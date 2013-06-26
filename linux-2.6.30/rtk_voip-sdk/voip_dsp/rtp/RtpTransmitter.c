
//static const char* const RtpTransmitter_cxx_Version =
//    "$Id: RtpTransmitter.c,v 1.8 2008-11-20 09:41:16 rock Exp $";

//#include <time.h>
//#include <sys/types.h>
//#include <unistd.h>
//#include <string.h>

#include <linux/string.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#ifdef DEBUG_LOG
#include "cpLog.h"
#endif
/* Kao
#include "vsock.hxx"
*/

#include "assert.h"
#include "rtpTypes.h"
#include "rtpTools.h"
#include "Rtp.h"
#include "rtk_voip.h"
#include "voip_init.h"
#include "voip_proc.h"
#include "voip_params.h"
//#include <debug.h>

#include "codec_descriptor.h"

#include "v152_api.h"

//#define RFC2833_TX_FLOW_DEBUG_PRINT
#define RFC2833_PROC_DEBUG_SUPPORT

#ifdef RFC2833_PROC_DEBUG_SUPPORT
static uint16 rfc2833_proc_event_cnt[DSP_RTK_SS_NUM][50] = {0}; /* 16 DTMF digit */
static uint16 rfc2833_proc_ret_cnt[DSP_RTK_SS_NUM][8][50] = {0};
static uint16 rfc2833_proc_add_cnt[DSP_RTK_SS_NUM][50] = {0};
static uint16 rfc2833_proc_fifo_read_cnt[DSP_RTK_SS_NUM][50] = {0};
extern int g_digit[];
#endif

///////////////////////////////////////////////////////////////////////

char rfc2833_period[MAX_DSP_RTK_CH_NUM]={0};        // hc+ for voice packet interleave in rfc2833 period issue
unsigned int gRfc2833_volume_dsp_auto[MAX_DSP_RTK_SS_NUM] = {0};
unsigned int gRfc2833_volume[MAX_DSP_RTK_SS_NUM] = {[0 ... MAX_DSP_RTK_SS_NUM-1] = 0x0A};
static int transmit(RtpPacket*, BOOL);


static int cur_send_dtmf;
static int cur_send;
static BOOL bCreate;

extern int dsp_rtk_ch_num;

#ifdef SUPPORT_MULTI_FRAME
extern unsigned char MFFrameNo[MAX_DSP_RTK_SS_NUM];
#endif

extern unsigned char rfc2833_dtmf_pt_remote[MAX_DSP_SS_NUM];    /* 0: Don't send 2833 packets. 96 or 101 or ..: send 2833 packets */	// move to dsp_define.c
extern unsigned char rfc2833_fax_modem_pt_remote[MAX_DSP_SS_NUM];
extern unsigned int CurrentRfc2833DtmfMode[MAX_DSP_RTK_CH_NUM];	/* 0 : current is not in DTMF RFC2833 mode, 1: in RFC2833 mode*/	// move to dsp_define.c
extern unsigned int CurrentRfc2833FaxModemMode[MAX_DSP_RTK_CH_NUM];	/* 0 : current is not in Fax/Modem RFC2833 mode, 1: in RFC2833 mode*/
static int send_2833_start[MAX_DSP_SS_NUM]={0};              /* 0: stop send 2833 packets   1: start  send 2833 packets */
static int send_marker_flag[MAX_DSP_SS_NUM] = {0};           /* 0: not send. 1: send */
static int send_2833_flag[MAX_DSP_SS_NUM] = {0};             /* 0: not send. 1: send */
static int send_edge_flag[MAX_DSP_SS_NUM] = {0};             /* 0: not send. 1: send */
static int timestamp_2833[MAX_DSP_SS_NUM] = {0};
static unsigned short edge_sequence_2833[MAX_DSP_SS_NUM] = {0};
int send_2833_by_ap[MAX_DSP_RTK_CH_NUM] = {0};		/* 0: by DSP 1: by AP */	
extern int g_dynamic_pt_remote[];
extern int g_dynamic_pt_remote_vbd[];


#if defined( SUPPORT_RFC_2833 ) && defined( SEND_RFC2833_ISR )
unsigned char send_2833_count_down[MAX_DSP_RTK_SS_NUM];
static unsigned char send_2833_count_down_dsp[MAX_DSP_RTK_SS_NUM] = {0};
#endif

#ifdef SUPPORT_RFC2833_PLAY_LIMIT
extern int bRfc2833_play_limit[MAX_DSP_RTK_SS_NUM]; // flag to turn on/off play time limit
extern int rfc2833_play_limit_ms[MAX_DSP_RTK_SS_NUM];
#endif

RtpTransmitter RtpTxInfo[MAX_DSP_RTK_SS_NUM];

#ifdef SUPPORT_RTP_REDUNDANT
// ------ RTP Redundancy - Audio ---------
#define MAX_AUDIO_REDUNDANT_NUM		2	// redundant audio packet number 
#define MAX_AUDIO_ELEMENT_DATA		( PKTDATA_SIZES / MAX_AUDIO_REDUNDANT_NUM )

typedef struct {
	unsigned char data[ MAX_AUDIO_ELEMENT_DATA ];
	int len;
	RtpTime timestamp;
	RtpPayloadType PT;
} RtpRedundantTxAudioElement_t;

typedef struct {
	RtpRedundantTxAudioElement_t element[ MAX_AUDIO_REDUNDANT_NUM ];	// preceding is older 
	int set_count;
	int max_count;
} RtpRedundantTxAudio_t;

// ------ RTP Redundancy - RFC 2833 ---------
#define MAX_RFC2833_ELEMENT_DATA	8	// when I check RFC2833, 8 bytes seems maximum 
#define MAX_RFC2833_REDUNDANT_R		( 5 + 1 ) 	// RFC2833 suggest 5 (but whitin 2.048 sec due to timestamp offset field)

typedef struct {
	RtpTime timestamp;
	RtpPayloadType PT;
	unsigned char data[ MAX_RFC2833_ELEMENT_DATA ];
	int len;
} RtpRedundantTxRFC2833Element_t;

typedef struct {
	RtpRedundantTxRFC2833Element_t element[ MAX_RFC2833_REDUNDANT_R ];	// preceding is older 
	int valid_element;
	int max_element;	// user's max + 1, for three 'end' 
} RtpRedundantTxRFC2833_t;

typedef struct {
	RtpRedundantTxAudio_t audio;
	RtpRedundantTxRFC2833_t rfc2833;
} RtpRedundantTx_t;

static RtpRedundantTx_t RtpRedundantTx[ DSP_RTK_SS_NUM ];
#endif // SUPPORT_RTP_REDUNDANT

/* ----------------------------------------------------------------- */
/* --- RtpTransmitter Constructor ---------------------------------- */
/* ----------------------------------------------------------------- */

void RtpTx_Init (void)
{
	int i;
	// set private variables
	cur_send = 0;
	cur_send_dtmf = 0;
	bCreate = FALSE;
	for(i=0; i<DSP_RTK_SS_NUM; i++)
	{
		RtpTx_InitbyID(i);
		rfc2833_dtmf_pt_remote[i] = 101; /* Init payload type to 101 */
		rfc2833_fax_modem_pt_remote[i] = 101;
	}
}

void RtpTx_InitbyID (uint32 sid)
{
	RtpTransmitter* pInfo = NULL;
	RtpRedundantTx_t *pRtpRedundantTx = &RtpRedundantTx[ sid ];

	if(sid >= DSP_RTK_SS_NUM)
		return;

	pInfo = &RtpTxInfo[sid];

	pInfo->ssrc = generateSRC();
/*	pInfo->seedNtpTime = Ntp_getTime(); */
#ifdef SUPPORT_RTCP
	Ntp_getTime(&pInfo->seedNtpTime);
        pInfo->prevNtpTime = pInfo->seedNtpTime;
#endif
	pInfo->seedRtpTime = generate32();
	pInfo->prevRtpTime = pInfo->seedRtpTime;
	pInfo->prevRtpTimeCalibration = 1;
	pInfo->prevSequence = generate32();
	pInfo->markerOnce = TRUE;

	pInfo->tranOpmode = rtptran_droppacket;

	// set counters
	pInfo->packetSent = 0;
	pInfo->payloadSent = 0;

#ifdef SUPPORT_RTP_REDUNDANT	
	pRtpRedundantTx ->audio.set_count = 0;
	pRtpRedundantTx ->audio.max_count = MAX_AUDIO_REDUNDANT_NUM;
	pRtpRedundantTx ->rfc2833.valid_element = 0;
	pRtpRedundantTx ->rfc2833.max_element = MAX_RFC2833_REDUNDANT_R;
#endif
}

void RtpTx_renewSession (uint32 sid, int randomly, RtpSrc SSRC, RtpSeqNumber seqno, RtpTime timestamp,
							int max_red_audio, int max_red_rfc2833)
{
	RtpTransmitter* pInfo = NULL;
	RtpRedundantTx_t *pRtpRedundantTx = &RtpRedundantTx[ sid ];

	if(sid >= DSP_RTK_SS_NUM)
		return;

	pInfo = &RtpTxInfo[sid];

	pInfo->ssrc = ( randomly ? generateSRC() : SSRC );
	pInfo->seedRtpTime = ( randomly ? generate32() : timestamp );
	pInfo->prevRtpTime = pInfo->seedRtpTime - 80;	// '-80' is not good style, but RtpTx_setFormat() will update again 
	pInfo->prevRtpTimeCalibration = 1;
	pInfo->prevSequence = ( randomly ? generate32() : seqno );
#ifdef CONFIG_RTK_VOIP_SRTP
	// Avoid losing ROC synchronization
	// Initial seguence number shall be less than 2^15
	pInfo->prevSequence &= 0x7FFF;
#endif	
	pInfo->markerOnce = TRUE;

	// set counters
	pInfo->packetSent = 0;
	pInfo->payloadSent = 0;
	
#ifdef SUPPORT_RTP_REDUNDANT
	pRtpRedundantTx ->audio.set_count = 0;
	pRtpRedundantTx ->audio.max_count = 
				( ( max_red_audio < 0 || max_red_audio > MAX_AUDIO_REDUNDANT_NUM ) ?
					MAX_AUDIO_REDUNDANT_NUM : max_red_audio );
	pRtpRedundantTx ->rfc2833.valid_element = 0;
	if( max_red_rfc2833 == 0 )
		pRtpRedundantTx ->rfc2833.max_element = 0;
	else
		pRtpRedundantTx ->rfc2833.max_element = 
				( ( max_red_rfc2833 < 0 || max_red_rfc2833 >= MAX_RFC2833_REDUNDANT_R ) ?
					MAX_RFC2833_REDUNDANT_R : max_red_rfc2833 + 1 );
#endif
}

#if 0
void setRemoteAddr (const NetworkAddress& theAddr)
{
	remoteAddr = theAddr;
}
#endif

RtpTransmitter* RtpTx_getInfo (uint32 sid)
{
    RtpTransmitter* pInfo = NULL;

	if(sid >= DSP_RTK_SS_NUM)
		return NULL;

	pInfo = &RtpTxInfo[sid];
	return pInfo;
}

/* --- send packet functions --------------------------------------- */

static RtpPacket* createPacket (uint32 chid, uint32 sid, int npadSize, int csrc_count)
{
	RtpTransmitter* pInfo = NULL;

	RtpPacket* packet;
	// create packet
	if(bCreate)
	{
		bCreate = FALSE;
		return NULL;
	}
	bCreate = TRUE;

	if(sid >= DSP_RTK_SS_NUM)
	{
		bCreate = FALSE;
		return NULL;
	}

	pInfo = &RtpTxInfo[sid];

	packet = &RTP_TX_DTMF[cur_send_dtmf];
	if(packet->own == OWNBYDSP)
	{
		bCreate = FALSE;
    	return NULL;
	}
	cur_send_dtmf++;
	cur_send_dtmf &= (RTP_TX_DTMF_NUM-1);
	RtpPacket_Init(packet, RECV_BUF/*pInfo->payloadSize*/, npadSize, csrc_count);
	assert (packet);
	// load packet
	setSSRC (packet, pInfo->ssrc);
	setPayloadType (packet, pInfo->payloadFormat/*apiFormat*/);
	packet->chid = chid;
	packet->sid = sid;
	
	packet->own = OWNBYDSP;
	bCreate = FALSE;

    return packet;
}

#ifdef SUPPORT_RTP_REDUNDANT
static void RtpRedundantReclaimRFC2833( int sid, RtpTime now )
{
	RtpRedundantTxRFC2833_t * const pRtpRedTx2833 = &RtpRedundantTx[ sid ].rfc2833;
	RtpRedundantTxRFC2833Element_t * pRtpRedTx2833Element;
	int elements = pRtpRedTx2833 ->valid_element;
	int i, j;
	
	// quickly return 
	if( elements == 0 )
		return;
	
	// check elements number 
	if( elements > pRtpRedTx2833 ->max_element ) {
		printk( "sid=%d elements > %d\n", sid, pRtpRedTx2833 ->max_element );
		elements = pRtpRedTx2833 ->max_element;
	}
	
	// loop to check element 
	for( i = 0; i < elements; i ++ ) {
		
		pRtpRedTx2833Element = &pRtpRedTx2833 ->element[ i ];
		
		// redundancy timestamp offset 14 bits --> 2^14 = 16384
		if( now - pRtpRedTx2833Element ->timestamp >= 16384 ) {
			// remove this!! 
		} else
			break;	// scan done! 
	}
	
	// no corrupt element 
	if( i == 0 )
		return;
	
	// update valid element 
	pRtpRedTx2833 ->valid_element -= i;
	
	// remove corrupt elements 
	for( j = 0; j < pRtpRedTx2833 ->max_element && i < elements; j ++, i ++ ) 
	{
		pRtpRedTx2833 ->element[ j ] = pRtpRedTx2833 ->element[ i ];
	}
}

static int RtpRedundantTransmitProcessRFC2833( RtpPacket* p )
{
	// Process: 
	//   move payload to redundancy primary payload 
	//   set (enlarge) payload total length 
	//   fill redundancy 'RFC2833' data 
	// Not process: 
	//   RTP payload type (by caller)
	
	//const int chid = p ->chid;
	const int sid = p ->sid;
	const int primLen = getPayloadUsage( p );
	const RtpPayloadType primPT = getPayloadType(p);
	const RtpTime primTimestamp = getRtpTime( p );
	
	RtpRedundantTxRFC2833_t * const pRtpRedTx2833 = &RtpRedundantTx[ sid ].rfc2833;
	RtpRedundantTxRFC2833Element_t * pRtpRedTx2833Element;
	int nTotalPayloadLen;	// redundant header + red/primary data
	unsigned char *pDst;
	int i, elements;
	int red_start, red_elements = 0;
	
	if( !GetRtpRedundantStatus( sid ) )
		return 0;
	
	if( pRtpRedTx2833 ->max_element == 0 )
		return 0;	// Don't redundancy (may audio redundancy only)
	
	// arrange rundancy RFC2833 data (valid_elements will change!!)
	// call this before enter!
	//RtpRedundantReclaimRFC2833( sid, primTimestamp );
	
	// calculate nTotalPayloadLen (1) - primary 
	nTotalPayloadLen = sizeof( RtpRedPrimaryHeader ) + primLen;
	
	//printk( "T:%d=%d+%d ", nTotalPayloadLen, sizeof( RtpRedPrimaryHeader ), len );
	
	// calculate nTotalPayloadLen (2) - redundancy  
	elements = pRtpRedTx2833 ->valid_element;
	
	for( i = 0; i < elements; i ++ ) {
		
		pRtpRedTx2833Element = &pRtpRedTx2833 ->element[ i ];
		
		if( pRtpRedTx2833Element ->timestamp == primTimestamp ) {
			continue;	// to avoid three continual 'end' 
		}
		
		red_elements ++;
		nTotalPayloadLen += sizeof( RtpRedHeader ) + 
							pRtpRedTx2833Element ->len;
	}
	
	// calculate nTotalPayloadLen (3) - minus first one, because we increases user's max 
	if( red_elements >= pRtpRedTx2833 ->max_element ) {
		nTotalPayloadLen -= ( sizeof( RtpRedHeader ) +
								pRtpRedTx2833 ->element[ 0 ].len );
		red_start = 1;
	} else
		red_start = 0;
	
	// calculate nTotalPayloadLen (4) - set to packet 
	setPayloadUsage (p, nTotalPayloadLen/*packetSize*/);
	
	// fill packet to transmit (1) - primary payload (memmove to tail)
	pDst = getPayloadLoc(p);
	
	memmove( pDst + nTotalPayloadLen - primLen, pDst, primLen );	// move primary data to tail 
	
	// fill packet to transmit (2) - redundancy header  
	for( i = red_start; i < elements; i ++ ) {
		
		pRtpRedTx2833Element = &pRtpRedTx2833 ->element[ i ];

		if( pRtpRedTx2833Element ->timestamp == primTimestamp )
			continue;	// to avoid three continual 'end' 
		
		( ( RtpRedHeader * )pDst ) ->F = 1;
		( ( RtpRedHeader * )pDst ) ->blockPT = pRtpRedTx2833Element ->PT;
		( ( RtpRedHeader * )pDst ) ->timestamp_offset = primTimestamp - pRtpRedTx2833Element ->timestamp;
		( ( RtpRedHeader * )pDst ) ->block_length = pRtpRedTx2833Element ->len;
		pDst += sizeof( RtpRedHeader );
	}

	// fill packet to transmit (3) - primary header 
	( ( RtpRedPrimaryHeader * )pDst ) ->F = 0;	// fill primary redundant header 
	( ( RtpRedPrimaryHeader * )pDst ) ->blockPT = getPayloadType( p );
	pDst += sizeof( RtpRedPrimaryHeader );
	
	// fill packet to transmit (4) - redundancy data   
	for( i = red_start; i < elements; i ++ ) {
		
		pRtpRedTx2833Element = &pRtpRedTx2833 ->element[ i ];
		
		if( pRtpRedTx2833Element ->timestamp == primTimestamp )
			continue;	// to avoid three continual 'end' 
		
		memcpy( pDst, pRtpRedTx2833Element ->data, pRtpRedTx2833Element ->len );
		pDst += pRtpRedTx2833Element ->len;
	}
	
	// pDst point to primary data 
	
	/////////////////////////////////////////////////////////////// 	
	// save primary for next redundant 
	// (now, we process event only, but not tone with freqency)
	
	if( (primPT != rfc2833_dtmf_pt_remote[sid] )
		&& (primPT != rfc2833_fax_modem_pt_remote[sid]))	// event 
		goto lable_process_modulation;
	
//label_pocess_event:
	// Process DTMF digits only  
	if( ( ( RtpEventDTMFRFC2833 * )pDst ) ->edge != 1 )
		goto lable_process_modulation;
	
	// check the newest element 
	pRtpRedTx2833Element = &pRtpRedTx2833 ->element[ elements - 1 ];
	
	if( pRtpRedTx2833Element ->timestamp == primTimestamp )
		goto label_save_primary_done;	// has been saved!! 
		
	// check len 
	if( primLen > MAX_RFC2833_ELEMENT_DATA ) {
		printk( "primary len %d > %d\n", primLen, MAX_RFC2833_ELEMENT_DATA );
		goto label_save_primary_done;
	}
	
	// full!! remove one for current! 
	if( elements == pRtpRedTx2833 ->max_element ) {	
		for( i = 0; i < pRtpRedTx2833 ->max_element - 1; i ++ )
			pRtpRedTx2833 ->element[ i ] = pRtpRedTx2833 ->element[ i + 1 ];
		
		elements = pRtpRedTx2833 ->max_element - 1;
	}
	
	// save it 
	pRtpRedTx2833Element = &pRtpRedTx2833 ->element[ elements ];
	
	pRtpRedTx2833Element ->timestamp = primTimestamp;
	pRtpRedTx2833Element ->PT = primPT;
	memcpy( pRtpRedTx2833Element ->data, pDst, primLen );
	pRtpRedTx2833Element ->len = primLen;
	
	pRtpRedTx2833 ->valid_element = elements + 1;

lable_process_modulation:
	;
		
label_save_primary_done:
	;
	
	return 1;
}
#endif

// takes api RTP packet and send to network
// assumes payload size is already set
static int transmit(RtpPacket* packet, BOOL eventFlag )
{
	static int _2833_terminate[MAX_DSP_RTK_SS_NUM] = {0};
	
	RtpTransmitter* pInfo = NULL;
	if( !packet )
	{
#ifdef DEBUG_LOG
		cpLog(LOG_ERR,"Attempting to transmit a NULL rtp packet");
#endif
		return -1;
	}

	RtpPacket* p = packet;

	if(p->sid >= DSP_RTK_SS_NUM)
		return -1;

	pInfo = &RtpTxInfo[p->sid];

	if( !p->timestampSet )
	{
		setRtpTime( p, pInfo->prevRtpTime + pInfo->pktSampleSize/*network_pktSampleSize*/ );
		
		if (true == p->EventPktMarker)
		{
			timestamp_2833[p->sid] = pInfo->prevRtpTime + p->EventPktDuration;
			//timestamp_2833[p->sid] = pInfo->prevRtpTime + pInfo->pktSampleSize;
			//printk("tx: %u, sid=%d\n", timestamp_2833[p->sid], p->sid);
		}
	}
	
	if (eventFlag)
		setRtpTime( p, timestamp_2833[p->sid] );


	if( (!p->sequenceSet) )
	{
		if (eventFlag == 1)
		{
			setSequence( p, pInfo->prevSequence++ );
			
			if (p->EventPktEdge == true)
			{
				edge_sequence_2833[p->sid] = pInfo->prevSequence - 1;
			}
		}
		else
			setSequence( p, pInfo->prevSequence++ );
	}
	else
	{
		if ( (eventFlag == 1) && (p->EventPktEdge == true) )
		{
			static int cnt[MAX_DSP_RTK_SS_NUM] = {0};
			
			cnt[p->sid]++;
			//printk("->%d\n", cnt[p->sid]);

			if (cnt[p->sid] == 2)
			{
				_2833_terminate[p->sid] = 1;
				cnt[p->sid] = 0;
				//printk("1:%d\n", _2833_pkt_cnt[p->sid]);
			}
				
			setSequence( p, edge_sequence_2833[p->sid]);
		}
	}
		
	if( getPayloadUsage(p) < 0  ||  getPayloadUsage(p) > 1012 )
	{
#ifdef DEBUG_LOG
		cpLog(LOG_DEBUG_STACK,"Invalid data packet size %d", getPayloadUsage());
#endif
		return -1;
	}

    //set marker once
	if(( pInfo->markerOnce == TRUE ) && (eventFlag == 0))
	{
#ifdef DEBUG_LOG
		cpLog( LOG_DEBUG_STACK,"Setting marker bit once");
#endif
		setMarkerFlag(p, 1);
		pInfo->markerOnce = FALSE;
	}

#ifdef SUPPORT_RTP_REDUNDANT
	// reclaim redudnacy data 
	RtpRedundantReclaimRFC2833( p ->sid, getRtpTime( p ) );
	
	// RFC2833 need redudancy 
	if( p ->RFC2833 && RtpRedundantTransmitProcessRFC2833( p ) ) {	
		/* overwrite payload type with redundant one */
		setPayloadType (p, GetRtpRedundantPayloadType( p ->sid, 0 /* remote */ ) );
	}
#endif

	// for packet reuse
	p->timestampSet = FALSE;
	p->sequenceSet = FALSE;
	// transmit packet
	DSP_rtpWrite(p);

	// update counters
	pInfo->packetSent++;
	//prevSequence = getSequence(p);
	if( !eventFlag )
	{
		pInfo->payloadSent += getPayloadUsage(p);
#ifdef SUPPORT_RTCP
		Ntp_getTime(&pInfo->prevNtpTime);
#endif
		pInfo->prevRtpTime = getRtpTime(p);
	}
	else
	{
		if (_2833_terminate[p->sid] == 1)
		{		
			//printk("1: %u, sid=%d\n", pInfo->prevRtpTime, p->sid);
			pInfo->prevRtpTime +=  p->EventPktDuration;
			//printk("2: %u, sid=%d\n", pInfo->prevRtpTime, p->sid);

			_2833_terminate[p->sid] = 0;

			//printk("2:0%x\n", pInfo->prevRtpTime);
		}

		/*
		 *	voice rtp timestamp: x
		 *	2833 pkt 1 
		 *	2833 pkt 2 
		 *	..
		 *	2833 pkt N duration: y
		 *	voice rtp timestamp: x + y + voice rtp packet time
		 */
	}
	

	// set up return value
	int result = getPayloadUsage(p);

	// exit with success
	return result;
}

#ifdef SUPPORT_RTP_REDUNDANT
static void RtpRedundantReclaimAudio( uint32 sid, RtpTime now )
{
	RtpRedundantTxAudio_t * const pRedAudio = &RtpRedundantTx[ sid ].audio;
	RtpRedundantTxAudioElement_t * pElement;
	int i, j;
	int count = pRedAudio ->set_count;
	
	// check which one need reclaim 
	for( i = 0; i < count; i ++ ) {
		
		pElement = &pRedAudio ->element[ i ];
		
		// check if timestamp too large (timestamp offset 14 bits -->16384 )
		if( now - pElement ->timestamp < 16384 )
			break;
	}
	
	if( i == 0 )
		return;	// no need reclaim 

	pRedAudio ->set_count = count - i;
	
	for( j = 0; j < pRedAudio ->max_count - 1 && 
				i < count; 
												j ++, i ++ )
	{
		pRedAudio ->element[ j ] = pRedAudio ->element[ i ];
	}
}

static int RtpRedundantTransmitProcess( uint32 sid, char* data, int len,
								 RtpTransmitter* pInfo,
								 RtpPacket* p )
{
	// goal of this function is to: 
	//	setPayloadUsage (p, len/*packetSize*/);
	//	memcpy (getPayloadLoc(p), data, len);
	
	RtpRedundantTxAudio_t * const pRtpRedTx = &RtpRedundantTx[ sid ].audio;
	RtpRedundantTxAudioElement_t * pElement;
	int nTotalPayloadLen = 0;	// redundant header + red/primary data
	unsigned char *pDst;
	const RtpTime curTimestamp = pInfo->prevRtpTime + pInfo->pktSampleSize;
	int i;

	if( !GetRtpRedundantStatus( sid ) )
		return 0;
	
	if( pRtpRedTx ->max_count == 0 )
		return 0;	// Don't redundancy (may RFC 2833 redundancy only)
	
	// reclaim audio redundancy 
	RtpRedundantReclaimAudio( sid, curTimestamp );
	
	// fill packet to transmit - calculate nTotalPayloadLen 
	for( i = 0; i < pRtpRedTx ->set_count; i ++ ) {
		
		pElement = &pRtpRedTx ->element[ i ];
		
		nTotalPayloadLen += sizeof( RtpRedHeader ) + pElement ->len;
	}
	
	nTotalPayloadLen += sizeof( RtpRedPrimaryHeader ) + len;
	
	// fill packet to transmit - set payload usage 
	setPayloadUsage (p, nTotalPayloadLen/*packetSize*/);
	
	pDst = getPayloadLoc(p);
	
	// fill packet to transmit - fill redundant header 
	for( i = 0; i < pRtpRedTx ->set_count; i ++ ) {
		
		pElement = &pRtpRedTx ->element[ i ];
		
		( ( RtpRedHeader * )pDst ) ->F = 1;
		( ( RtpRedHeader * )pDst ) ->blockPT = pElement ->PT;
		( ( RtpRedHeader * )pDst ) ->timestamp_offset = curTimestamp - pElement ->timestamp;
		( ( RtpRedHeader * )pDst ) ->block_length = pElement ->len;
		pDst += sizeof( RtpRedHeader );
	}
	
	// fill packet to transmit - fill primary redundant header 
	( ( RtpRedPrimaryHeader * )pDst ) ->F = 0;
	( ( RtpRedPrimaryHeader * )pDst ) ->blockPT = getPayloadType( p );
	pDst += sizeof( RtpRedPrimaryHeader );
	
	// fill packet to transmit - fill redundant data 
	for( i = 0; i < pRtpRedTx ->set_count; i ++ ) {
		
		pElement = &pRtpRedTx ->element[ i ];
		
		memcpy( pDst, pElement ->data, pElement ->len );
		pDst += pElement ->len;
	}
	
	// fill packet to transmit - fill primary data 
	memcpy( pDst, data, len );
	pDst += len;
	
	// save current for next redundant - check length 
	if( len > MAX_AUDIO_ELEMENT_DATA ) {
		printk( "audio len %d > %d\n", len, MAX_AUDIO_ELEMENT_DATA );
		return 1;
	} 
	
	// save current for next redundant - free a space for current 
	if( pRtpRedTx ->set_count >= pRtpRedTx ->max_count ) {
		
		for( i = 0; i < pRtpRedTx ->max_count - 1; i ++ )
			pRtpRedTx ->element[ i ] = pRtpRedTx ->element[ i + 1 ];
		
		pRtpRedTx ->set_count = pRtpRedTx ->max_count - 1;
	}
	
	// save current for next redundant - save it!! 
	pElement = &pRtpRedTx ->element[ pRtpRedTx ->set_count ];
	
	pRtpRedTx ->set_count ++;	// increase count 
	
	memcpy( pElement ->data, data, len );
	pElement ->len = len;
	pElement ->timestamp = curTimestamp;
	pElement ->PT = getPayloadType( p );

	return 1;
}
#endif

// takes rawdata, buffers it, and send network packets
int RtpTx_transmitRaw (uint32 chid, uint32 sid, char* data, int len)
{
	RtpTransmitter* pInfo = NULL;
	// send out packets from buffer
	int result = 0;

	extern uint32 rtpConfigOK[];
#ifdef SUPPORT_COMFORT_NOISE
	extern int m_nSIDFrameLen[MAX_DSP_RTK_SS_NUM];                                            // the length of SID frame
#endif
	extern uint16 SID_payload_type_remote[ DSP_RTK_SS_NUM ];
	extern uint32 SID_count_tx[ DSP_RTK_SS_NUM ];
	// create packet
	RtpPacket* p = &RTP_TX_DEC[cur_send];

	if(chid >= dsp_rtk_ch_num)
		return 0;
	
	if(sid >= DSP_RTK_SS_NUM)
		return 0;

	if(rtpConfigOK[sid] == 0)
		return 0;

	if(p->own == OWNBYDSP)
		return 0;

	if(!isTranMode(sid))
		return 0;

	pInfo = &RtpTxInfo[sid];

	cur_send++;
	cur_send &= (RTP_TX_DEC_NUM-1);
	RtpPacket_Init(p, RECV_BUF/*pInfo->payloadSize*/, 0, 0);
	p->chid = chid;
	p->sid = sid;
	assert (p);
	setSSRC (p, pInfo->ssrc);
#ifdef SUPPORT_COMFORT_NOISE
	if ((len == m_nSIDFrameLen[sid]) && (g_dynamic_pt_remote[sid] != rtpPayload_AMR_NB) && (g_dynamic_pt_remote[sid] != rtpPayloadG729))
	{
		if( SID_payload_type_remote[ sid ] )
			setPayloadType( p, SID_payload_type_remote[ sid ] );
		else
			setPayloadType (p, 13);
		
		SID_count_tx[ sid ] ++;
	} else
#endif
	{
		RtpPayloadType type;

#ifdef SUPPORT_V152_VBD		
		if( V152_CheckIfSendVBD( sid ) )
			type = g_dynamic_pt_remote_vbd[sid];
		else
#endif
		{
			type = g_dynamic_pt_remote[sid];
		}
			
		setPayloadType (p, type); // support dynamic payload.
		//setPayloadType (p, pInfo->payloadFormat/*networkFormat*/);
	}

#ifdef SUPPORT_RTP_REDUNDANT
	if( RtpRedundantTransmitProcess( sid, data, len, pInfo, p ) ) {
		/* overwrite payload type with redundant one */
		setPayloadType (p, GetRtpRedundantPayloadType( sid, 0 /* remote */ ) );
	} else
#endif
	{
		setPayloadUsage (p, len/*packetSize*/);
	
		memcpy (getPayloadLoc(p), data, len);
	}
	p->own = OWNBYDSP;
	result += transmit(p, FALSE);
	p->own = OWNBYRTP;
	return result;
}

#ifdef SUPPORT_RFC_2833
int RtpTx_transmitEvent( uint32 chid, uint32 sid, int event, int delay_ms)
{
	extern char dtmf_mode[MAX_DSP_RTK_CH_NUM]/*_inband*/ ;
	int num,i;
	unsigned short n = 160;
	RtpTransmitter* pInfo = NULL;

        if(chid >= dsp_rtk_ch_num)
		return 0;

	if(sid >= DSP_RTK_SS_NUM)
		return 0;
		
	pInfo = &RtpTxInfo[sid];

    // Howard. 2004.12.30 for prevent to send dtmf digit using Outband when user enables the Inband
    if( dtmf_mode[chid]/*_inband*/ != 0 )	// Howard. 2005.2.24 when dtmf_mode is not RFC2833 , then we can't generate RFC2833 packet, so we return here.
		return ( -1 ) ;

	rfc2833_period[chid] = 1;	// hc+ for avoid voice packet interleave issue

	//sessionError = session_success;
	RtpPacket* eventPacket = createPacket(chid, sid, 0, 0);
	// Howard. 2005.3.16
	setPayloadType( eventPacket, rfc2833_dtmf_pt_remote[sid]);
	setPayloadUsage( eventPacket, sizeof( RtpEventDTMFRFC2833 ) );
	setMarkerFlag(eventPacket, 1);
	RtpEventDTMFRFC2833* eventPayload = (RtpEventDTMFRFC2833*)( getPayloadLoc(eventPacket) );

	// reset event payload
	eventPayload->event = event; 
	eventPayload->volume = gRfc2833_volume[sid];
	eventPayload->reserved = 0;
	eventPayload->edge = 0;
	eventPayload->duration = n;	//htons(n);

	// send onedge packet
	// jimmylin - let's send it for delay_ms
	num = (delay_ms/20) - 2;
	//Howard. 2005.3.16 to transmit 3 times first Digit ( Marker bit set to 1 )
	for( i = 0 ; i < 3 ; i++ )
	{
		eventPacket->timestampSet = true ;
		transmit( eventPacket, true ) ;
		eventPacket->sequenceSet = true ;
	}
	setMarkerFlag(eventPacket,0);
	udelay(20);

	for(i=0; i<num; i++)	
	{
		n += 160;
		eventPayload->duration = n;	//htons(n);
		eventPacket->timestampSet = true;
		transmit( eventPacket, true );
		udelay(20);
	}

	// send on packet
	eventPayload->edge = 1;
	// jimmylin - retransmit 3 times according to Rfc2833
	for(i=0; i<3; i++)
	{
		eventPacket->timestampSet = true;
		transmit( eventPacket, true );
		eventPacket->sequenceSet = true;
	}
	pInfo->prevRtpTime += 640;	// hc+ 1101 timestamp issue

	rfc2833_period[chid] = 0;	// hc+ for avoid voice packet interleave issue
	
	eventPacket->own = OWNBYRTP;
	return 0;
}


static unsigned short duration[DSP_RTK_SS_NUM] = {0};
static unsigned short duration_end[DSP_RTK_SS_NUM] = {0};
static unsigned int rfc2833_dtmf_interval[DSP_RTK_SS_NUM] = {[0 ... DSP_RTK_SS_NUM-1] = 10}; // default: 10ms
static unsigned int rfc2833_vbd_interval[DSP_RTK_SS_NUM] = {[0 ... DSP_RTK_SS_NUM-1] = 10}; // default: 10ms
static unsigned int rfc2833_cut_process[DSP_RTK_SS_NUM] = {[0 ... DSP_RTK_SS_NUM-1] = 0};

void SetDtmfRfc2833PktInterval(unsigned int sid, unsigned int interval)
{
	if ((interval%10) != 0)
	{
		PRINT_R("DTMF RFC2833 packet interval(=%d ms) is not multiple of 10ms, Set Fail!\n", interval);
		return;
	}
	
	rfc2833_dtmf_interval[sid] = interval;
	//PRINT_G("rfc2833_dtmf_interval = %d ms\n", rfc2833_dtmf_interval[sid]);
}

void SetFaxModemRfc2833PktInterval(unsigned int sid, unsigned int interval)
{
	if ((interval%10) != 0)
	{
		PRINT_R("Fax/Modem RFC2833 packet interval(=%d ms) is not multiple of 10ms, Set Fail!\n", interval);
		return;
	}
	
	rfc2833_vbd_interval[sid] = interval;
	//PRINT_G("rfc2833_vbd_interval = %d ms\n", rfc2833_vbd_interval[sid]);
}

unsigned int GetDtmfRfc2833PktInterval(unsigned int sid)
{
	return rfc2833_dtmf_interval[sid];
}

unsigned int GetFaxModemRfc2833PktInterval(unsigned int sid)
{
	return rfc2833_vbd_interval[sid];
}

unsigned int GetRfc2833PktInterval(unsigned int sid, unsigned int event)
{
	if ((event >= 0) && (event <= 16))
		return rfc2833_dtmf_interval[sid];
	else if ((event >= 32) && (event <= 49))
		return rfc2833_vbd_interval[sid];
}



#define EVENT_PKT_FIFO_NUM 10
static RtpPacket eventPkt_fifo[MAX_DSP_RTK_SS_NUM][EVENT_PKT_FIFO_NUM]={{{0}}};
static int eventPkt_fifo_r[MAX_DSP_RTK_SS_NUM]={0}, eventPkt_fifo_w[MAX_DSP_RTK_SS_NUM]={0};

static int eventPkt_fifoWrite(int sid, RtpPacket *eventPkt)
{
	RtpPacket* p;
	
	if(((eventPkt_fifo_w[sid]+1)%EVENT_PKT_FIFO_NUM) == eventPkt_fifo_r[sid])
	{
		printk("RTP Event PKT FIFO Full\n");
		return 0;
	}
		
	p = &eventPkt_fifo[sid][eventPkt_fifo_w[sid]];
	memcpy(p, eventPkt, sizeof(RtpPacket));
	p->packetData = p->packetbuf;
	p->header = (RtpHeader*) (char*)(p->packetData);
	eventPkt_fifo_w[sid] = (eventPkt_fifo_w[sid] + 1)%EVENT_PKT_FIFO_NUM;
	//printk("w[%d]=%d\n", sid, eventPkt_fifo_w[sid]);
		
	return 1;
}

static int eventPkt_fifoRead(int sid, RtpPacket** eventPacket)
{

	if (eventPkt_fifo_r[sid] == eventPkt_fifo_w[sid])
	{
		//printk("RTP Event PKT FIFO Empty\n");
		//printk(".");
		return 0;
	}

	*eventPacket = &eventPkt_fifo[sid][eventPkt_fifo_r[sid]];
	
	return 1; 
	
}

static void eventPkt_fifoReadDone(int sid)
{
	unsigned long flags;

	save_flags(flags); cli();
	eventPkt_fifo_r[sid] = (eventPkt_fifo_r[sid]+1)%EVENT_PKT_FIFO_NUM;
	restore_flags(flags);
	// printk("r[%d]=%d\n", sid, eventPkt_fifo_r[sid]);
}

static void eventPkt_fifoFlush(int sid)
{
	eventPkt_fifo_r[sid] = 0;
	eventPkt_fifo_w[sid] = 0;
}

// Support FaxModem RTP Removal Flag
static uint32 Support_FaxModem_RtpRemoval[MAX_DSP_RTK_CH_NUM] = {0};

int SetFaxModem_RtpRemoval(uint32 chid, uint32 flag)
{
	Support_FaxModem_RtpRemoval[chid] = flag;
	return 0;
}

int GetFaxModem_RtpRemoval(uint32 chid)
{
	return Support_FaxModem_RtpRemoval[chid];
}

static int dtmf_three_end[MAX_DSP_RTK_SS_NUM] = {[0 ... MAX_DSP_RTK_SS_NUM-1] = 1};
static int fax_modem_three_end[MAX_DSP_RTK_SS_NUM] = {[0 ... MAX_DSP_RTK_SS_NUM-1] = 1};

int get_2833_three_end(uint32 sid)
{
	int chid, ret;
	extern uint32 chanInfo_GetChannelbySession(uint32 sid);
	
	chid = chanInfo_GetChannelbySession(sid);
	
	if (Support_FaxModem_RtpRemoval[chid] == 1)
	{
		ret = (dtmf_three_end[sid] & fax_modem_three_end[sid]);
		//printk("%d ", fax_modem_three_end[chid]);
	}
	else
	{
		ret = dtmf_three_end[sid];
		//printk("2:%d\n", ret);
	}
	
	return ret;
}

struct tasklet_struct	event_send_tasklet;

void event_send(unsigned long *dummy)
{
	static int end_cnt[MAX_DSP_RTK_SS_NUM] = {0};
	unsigned char sid;
	RtpPacket* eventPacket = NULL;
	RtpEventDTMFRFC2833* eventPayload = NULL;
	int eventPacket_EventPktEdge;
	int eventPayload_event;

	for(sid=0; sid < MAX_DSP_RTK_SS_NUM; sid++)
	{
		while(eventPkt_fifoRead(sid, &eventPacket))
		{
			eventPayload = getPayloadLoc(eventPacket);
			
			if (eventPacket->EventPktEdge == false)
			{
				end_cnt[sid] = 0;
				if ((eventPayload->event >= 0) && (eventPayload->event <= 16))
					dtmf_three_end[sid] = 0;
				else if ((eventPayload->event >= 32) && (eventPayload->event <= 49))
					fax_modem_three_end[sid] = 0;
			}

			// backup check variable for later use 
			eventPacket_EventPktEdge = eventPacket->EventPktEdge;
			eventPayload_event = eventPayload->event;

			transmit(eventPacket, true);
			
#ifdef RFC2833_PROC_DEBUG_SUPPORT
			rfc2833_proc_fifo_read_cnt[sid][eventPayload_event]++;
#endif
			eventPkt_fifoReadDone(sid);

			// transmit() may modify eventPacket and eventPayload, so 
			// don't use them 
			if (eventPacket_EventPktEdge == true)
			{
				end_cnt[sid]++;
				//printk("=> %d\n", end_cnt[chid]);
				if (end_cnt[sid] == 3)
				{
					if ((eventPayload_event >= 0) && (eventPayload_event <= 16))
						dtmf_three_end[sid] = 1;
					else if ((eventPayload_event >= 32) && (eventPayload_event <= 49))
						fax_modem_three_end[sid] = 1;
				}
			}
		}
	}
}

static unsigned int check_removal(uint32 sid, int event)
{
	if (send_2833_count_down_dsp[sid] > 0)
		return 1;
	else if (send_2833_count_down_dsp[sid] == 0)
		return 0;
}

int get_2833_send_flag(uint32 sid)
{
	extern int send_dtmf_flag[];
	extern int send_modem_fax_flag[];
	extern uint32 chanInfo_GetChannelbySession(uint32 sid);
	int chid, ret;
	
	chid = chanInfo_GetChannelbySession(sid);
	
	if (Support_FaxModem_RtpRemoval[chid] == 1)
		ret = (send_dtmf_flag[sid] ||send_modem_fax_flag[sid]);
	else
		ret = send_dtmf_flag[sid] ;
	
	return ret;
}

static void set_2833_send_flag(uint32 sid, int event, int flag)
{
	extern int send_dtmf_flag[];
	extern int send_modem_fax_flag[];
	extern unsigned char rfc2833_transfer_flag[];

	if ((event >= 0) && (event <= 16))		// DTMF Event
		send_dtmf_flag[sid] = flag;
	else if ((event >= 32) && (event <= 49))	// Modem/Fax Event
		send_modem_fax_flag[sid] = flag;
	
	if (flag == 0)
		rfc2833_transfer_flag[sid] = 0;
}

static int reset_2833_send_flag(uint32 sid)
{
	extern int send_dtmf_flag[];
	extern int send_modem_fax_flag[];
	
	send_dtmf_flag[sid] = 0;
	send_modem_fax_flag[sid] = 0;
	dtmf_three_end[sid] = 1;
	fax_modem_three_end[sid] = 1;
}

int Reset_RFC2833_Trasmit(uint32 sid)
{
#ifdef SUPPORT_RFC2833_TRANSFER
	extern unsigned char rfc2833_transfer_flag[];
	extern unsigned char remote_dtmf_transfer_flag[];
	
	rfc2833_transfer_flag[sid] = 0;
	remote_dtmf_transfer_flag[sid] = 0;
#endif
	eventPkt_fifoFlush(sid);
	reset_2833_send_flag(sid);
	rfc2833_cut_process[sid] = 0;
}

uint32 rfc2833_count_add(uint32 sid, uint32 cnt)
{
	send_2833_count_down_dsp[sid] += cnt;
	
	//printk("%d-s%d\n", send_2833_count_down_dsp[sid], sid);

#ifdef RFC2833_PROC_DEBUG_SUPPORT
	extern uint32 chanInfo_GetChannelbySession(uint32 sid);
	int chid;
	chid = chanInfo_GetChannelbySession(sid);
	rfc2833_proc_add_cnt[sid][g_digit[chid]] += cnt;
#endif

	return send_2833_count_down_dsp[sid];
}

uint32 rfc2833_count_sub(uint32 sid, uint32 cnt)
{
	if (send_2833_count_down_dsp[sid] >= cnt)
		send_2833_count_down_dsp[sid] -= cnt;
	else
		send_2833_count_down_dsp[sid] = 0;
	
	return send_2833_count_down_dsp[sid];
}

uint32 rfc2833_count_reset(uint32 sid)
{
	send_2833_count_down[sid] = 0;
	send_2833_count_down_dsp[sid] = 0;
	return 0;
}

uint32 rfc2833_count_get(uint32 chid, uint32 sid)
{
	if (send_2833_by_ap[chid] == 1)
		return send_2833_count_down[sid];
	else
		return send_2833_count_down_dsp[sid];
}

static uint32 rfc2833_count_down(uint32 chid, uint32 sid, uint32 rfc2833_pi)
{

	if (send_2833_by_ap[chid] == 1)
  	{
		if( send_2833_count_down[ sid ] )
		{
			if ( send_2833_count_down[ sid ] < rfc2833_pi )
			{
				duration_end[sid] = 80*send_2833_count_down[ sid ];
				send_2833_count_down[ sid ]  = 0;
				//PRINT_R("1? ");
			}
			else
			{
				duration_end[sid] = 80*rfc2833_pi;
				send_2833_count_down[ sid ] -= rfc2833_pi; /* dec. 2833 count */	
			}
		}
		//PRINT_Y("%d ", send_2833_count_down[ sid ]);
	}
	else
	{
		if( send_2833_count_down_dsp[ sid ] )
		{
			if ( send_2833_count_down_dsp[ sid ] < rfc2833_pi )
			{
				duration_end[sid] = 80*send_2833_count_down_dsp[ sid ];
				send_2833_count_down_dsp[ sid ] = 0;
				//PRINT_R("2? ");	
			}
			else
			{
				duration_end[sid] = 80*rfc2833_pi;
				send_2833_count_down_dsp[ sid ] -= rfc2833_pi; /* dec. 2833 count */
			}
		}
		
		//PRINT_Y("%d ", send_2833_count_down_dsp[ sid ]);
	}

}

#ifdef RFC2833_PROC_DEBUG_SUPPORT
void rfc2833_proc_tx_cnt_reset(uint32 sid)
{
	memset( (unsigned char*)rfc2833_proc_event_cnt[sid], 0, sizeof(rfc2833_proc_event_cnt[0]) );
	memset( (unsigned char*)rfc2833_proc_ret_cnt[sid], 0, sizeof(rfc2833_proc_ret_cnt[0]));
	memset( (unsigned char*)rfc2833_proc_add_cnt[sid], 0, sizeof(rfc2833_proc_add_cnt[0]));
	memset( (unsigned char*)rfc2833_proc_fifo_read_cnt[sid], 0, sizeof(rfc2833_proc_fifo_read_cnt[0]));
}
#endif

int RtpTx_transmitEvent_ISR( uint32 chid, uint32 sid, int event)
{
	
	int i;
	unsigned int rfc2833_interval;
	unsigned int rfc2833_volume;
	unsigned char rfc2833_pt_remote = 0;
	uint32 current_cnt;
	static uint32 pre_cnt[MAX_DSP_RTK_SS_NUM]={0};
	
	//static int int_cnt = 0;
	
	//int_cnt++;
	
	RtpTransmitter* pInfo = NULL;
		
	if(chid >= dsp_rtk_ch_num)
	{
		set_2833_send_flag(sid, event, 0);
#ifdef RFC2833_PROC_DEBUG_SUPPORT
		rfc2833_proc_ret_cnt[sid][0][event]++;
#endif
		return 0;
	}

	if(sid >= DSP_RTK_SS_NUM)
	{
		set_2833_send_flag(sid, event, 0);
#ifdef RFC2833_PROC_DEBUG_SUPPORT
		rfc2833_proc_ret_cnt[sid][1][event]++;
#endif
		return 0;
	}
	
	if(!isTranMode(sid))
	{
#ifdef RFC2833_PROC_DEBUG_SUPPORT
		rfc2833_proc_ret_cnt[sid][2][event]++;
#endif
		return 0;
	}

	if ((event >= 0) && (event <= 16))		// DTMF 2833 event
		rfc2833_pt_remote = rfc2833_dtmf_pt_remote[sid];
	else if ((event >= 32) && (event <= 49))	// Fax/Modem 2833 event
		rfc2833_pt_remote = rfc2833_fax_modem_pt_remote[sid];
		
	if (rfc2833_pt_remote == 0) /* 0: Don't send 2833 packets. */
	{
		set_2833_send_flag(sid, event, 0);
#ifdef RFC2833_PROC_DEBUG_SUPPORT
		rfc2833_proc_ret_cnt[sid][3][event]++;
#endif
		return 0;
	}
	
	
	// update RFC2833 Interval
	if ((event >= 0) && (event <= 16))
		rfc2833_interval = rfc2833_dtmf_interval[sid]/10;
	else if ((event >= 32) && (event <= 49))
		rfc2833_interval = rfc2833_vbd_interval[sid]/10;


	//PRINT_G("cp%d\n", rfc2833_cut_process[sid]);
	
	if (rfc2833_cut_process[sid] != 0)
	{
		rfc2833_cut_process[sid] = (rfc2833_cut_process[sid]+1)%rfc2833_interval;
#ifdef RFC2833_PROC_DEBUG_SUPPORT
		rfc2833_proc_ret_cnt[sid][4][event]++;
#endif
		return 0;
	}
	else
	{
		rfc2833_cut_process[sid] = (rfc2833_cut_process[sid]+1)%rfc2833_interval;
	}


	if (!send_2833_start[sid])
	{
  		if(((send_2833_count_down[ sid ] != 0) && (send_2833_by_ap[chid] == 1) ) ||
  			((check_removal(sid, event) == 1) && (send_2833_by_ap[chid] == 0))) 
		{
			// Check if enough count
			if ((send_2833_count_down[sid] >= (rfc2833_interval+1)) || (send_2833_count_down_dsp[sid] >= (rfc2833_interval+1)))
			{
				rfc2833_count_down(chid, sid, rfc2833_interval);
				send_2833_start[sid] = 1;
				send_marker_flag[sid] = 1;
				pre_cnt[sid] = 0;

#ifdef RFC2833_TX_FLOW_DEBUG_PRINT
				printk("s%d\n", sid);
				//PRINT_R("int_cnt=%d\n", int_cnt);
#endif

			}
			else
			{
				current_cnt = rfc2833_count_get(chid, sid);
				
				if (pre_cnt[sid] == current_cnt)
				{
					rfc2833_count_reset(sid);
					pre_cnt[sid] = 0;
				}
				else
					pre_cnt[sid] = current_cnt;
				// Wait enough count
#ifdef RFC2833_TX_FLOW_DEBUG_PRINT
				printk("w%d\n", sid);
#endif
#ifdef RFC2833_PROC_DEBUG_SUPPORT
				rfc2833_proc_ret_cnt[sid][5][event]++;
#endif
				return 0;
			}
		}
		else
		{
			set_2833_send_flag(sid, event, 0);
#ifdef RFC2833_PROC_DEBUG_SUPPORT
			rfc2833_proc_ret_cnt[sid][6][event]++;
#endif
			return 0;
		}
	}
	else
	{
		rfc2833_count_down(chid, sid, rfc2833_interval);
		
  		if(((send_2833_count_down[ sid ] == 0) && (send_2833_by_ap[chid] == 1) ) ||
  			((check_removal(sid, event) == 0) && (send_2833_by_ap[chid] == 0))) 
		{
			send_edge_flag[sid] = 1;
			send_2833_flag[sid] = 0;
#ifdef RFC2833_TX_FLOW_DEBUG_PRINT
			printk("e%d\n", sid);	
#endif
		}
		else
		{
			send_2833_flag[sid] = 1;
			//printk("send_2833_flag[%d] = 1\n", sid);
			//PRINT_R("int_cnt=%d\n", int_cnt);
		}
	}


	pInfo = &RtpTxInfo[sid];


	RtpPacket* eventPacket = createPacket(chid, sid, 0, 0);
	setPayloadType( eventPacket, rfc2833_pt_remote);
	setPayloadUsage( eventPacket, sizeof( RtpEventDTMFRFC2833 ) );
	RtpEventDTMFRFC2833* eventPayload = (RtpEventDTMFRFC2833*)( getPayloadLoc(eventPacket) );

	extern char get_dtmf_dBFS(int chid, int dir, int reset);

	if ((event >= 0) && (event <= 16)) // DTMF 2833 event
	{
		// RFC2833 TX mode is sent by DSP, and volume is DSP auto.
		if ((send_2833_by_ap[chid] == 0) && (gRfc2833_volume_dsp_auto[sid] == 1))
		{
			rfc2833_volume = (-1)*get_dtmf_dBFS(chid, 0/*dir*/, 0/*reset*/);
			//printk("%d ", rfc2833_volume);
		}
		else
			rfc2833_volume = gRfc2833_volume[sid];
	}
	else
		rfc2833_volume = gRfc2833_volume[sid];

	// reset event payload
	eventPayload->event = event; 
	eventPayload->volume = rfc2833_volume;
	eventPayload->reserved = 0;
	eventPayload->edge = 0;
	eventPayload->duration = duration[sid];
	setMarkerFlag(eventPacket,0);
	eventPacket->RFC2833 = FALSE;
	eventPacket->EventPktMarker = false;
	eventPacket->EventPktBody = false;
	eventPacket->EventPktEdge = false;
	eventPacket->EventPktDuration = 0;


	/* Send Marker Packet */
	if (send_marker_flag[sid] == 1)
	{
		duration[sid] = 80*rfc2833_interval;
		setMarkerFlag(eventPacket, 1);
		eventPayload->duration = duration[sid];
		eventPacket->timestampSet = false ;
		eventPacket->sequenceSet = false ;
		eventPacket->EventPktMarker = true;
		eventPacket->RFC2833 = TRUE;
		eventPacket->EventPktDuration = duration[sid];
		//transmit( eventPacket, true ) ;
		eventPkt_fifoWrite(sid, eventPacket);
#ifdef RFC2833_PROC_DEBUG_SUPPORT
		rfc2833_proc_event_cnt[sid][event]++;
#endif
		
		send_marker_flag[sid] = 0;
#ifdef RFC2833_TX_FLOW_DEBUG_PRINT
		printk("ss%d\n", sid);
		//PRINT_R("int_cnt=%d\n", int_cnt);	
#endif
	}

#ifdef SUPPORT_RFC2833_PLAY_LIMIT
	if (bRfc2833_play_limit[sid] == 1)
	{
		if (duration[sid] >= (rfc2833_play_limit_ms[sid]*8 - 80*PCM_PERIOD))
		{
			send_edge_flag[sid] = 1;
			send_2833_flag[sid] = 0;
			//printk("==> stop 2833 event gen, sid=%d\n", sid);
		}	
	}		
#endif


	/* Send 2833 Event Packet */
	if (send_2833_flag[sid] == 1)
	{
		duration[sid] += 80*PCM_PERIOD*rfc2833_interval;
		eventPayload->duration = duration[sid];
		eventPacket->timestampSet = true;
		eventPacket->sequenceSet = false ;
		eventPacket->EventPktBody = true;
		eventPacket->RFC2833 = TRUE;
		eventPacket->EventPktDuration = duration[sid];
		//transmit( eventPacket, true );
		eventPkt_fifoWrite(sid, eventPacket);
#ifdef RFC2833_PROC_DEBUG_SUPPORT
		rfc2833_proc_event_cnt[sid][event]++;
#endif
#ifdef RFC2833_TX_FLOW_DEBUG_PRINT
		printk("st%d\n", sid);	
		//PRINT_R("int_cnt=%d\n", int_cnt);
#endif

	}
	

	/* Send Edge Packet */
	if (send_edge_flag[sid] == 1)
	{
		eventPayload->edge = 1;
		//duration[sid] += 80*PCM_PERIOD*rfc2833_interval;
		duration[sid] += duration_end[sid];
		eventPayload->duration = duration[sid];
		
		/*** send three the same sequence number edge packets ***/
		
		eventPacket->timestampSet = true;
		eventPacket->sequenceSet = false;
		eventPacket->EventPktEdge = true;
		eventPacket->RFC2833 = TRUE;
		eventPacket->EventPktDuration = duration[sid];
		//transmit( eventPacket, true );
		eventPkt_fifoWrite(sid, eventPacket);
#ifdef RFC2833_PROC_DEBUG_SUPPORT
		rfc2833_proc_event_cnt[sid][event]++;
#endif
		
		send_edge_flag[sid] = 0;	/* Must be set to 0 after send first edge packets */
		
		for(i=0; i<2; i++)
		{
			eventPacket->timestampSet = true;
			eventPacket->sequenceSet = true;
			eventPacket->EventPktEdge = true;
			eventPacket->RFC2833 = TRUE;
			//transmit( eventPacket, true );
			eventPkt_fifoWrite(sid, eventPacket);
#ifdef RFC2833_PROC_DEBUG_SUPPORT
			rfc2833_proc_event_cnt[sid][event]++;
#endif
		}
		
		send_2833_start[sid] = 0;
		set_2833_send_flag(sid, event, 0);
		rfc2833_cut_process[sid] = 0;
#ifdef RFC2833_TX_FLOW_DEBUG_PRINT
		printk("se%d\n", sid);
#endif
	}

	tasklet_hi_schedule(&event_send_tasklet);
	eventPacket->own = OWNBYRTP;
	
#ifdef RFC2833_PROC_DEBUG_SUPPORT
	rfc2833_proc_ret_cnt[sid][7][event]++;
#endif
	return 1;
}
#endif


void RtpTx_setFormat (uint32 sid, RtpPayloadType newtype, int frameRate)
{
	const codec_payload_desc_t *pCodecPayloadDesc;
	RtpTransmitter* pInfo = NULL;
	RtpRedundantTx_t *pRtpRedundantTx = &RtpRedundantTx[ sid ];
	if(sid >= DSP_RTK_SS_NUM)	// Sandro+
		return;

	pInfo = &RtpTxInfo[sid];

	pInfo->payloadFormat = newtype;
	pInfo->pCodecPayloadDesc = GetCodecPayloadDesc( newtype );

	pCodecPayloadDesc = pInfo->pCodecPayloadDesc;
	
	if( pCodecPayloadDesc ) { 
		pInfo->bitRate = pCodecPayloadDesc ->nTimestampRate;
		pInfo->payloadSize = 
			_imul32(pCodecPayloadDesc ->nFrameBytes, 
					_idiv32(frameRate, pCodecPayloadDesc ->nTranFrameRate));
	#ifdef SUPPORT_MULTI_FRAME
		pInfo->pktSampleSize = pCodecPayloadDesc ->nFrameTimestamp * MFFrameNo[sid];
	#else
		pInfo->pktSampleSize = pCodecPayloadDesc ->nFrameTimestamp;
	#endif		
	} else {
		pInfo->bitRate = 8000;
		pInfo->payloadSize = 160;
		pInfo->pktSampleSize = 160;
	}
	
	// setFormat can know timestamp period, so update again 
	if( pInfo->prevRtpTimeCalibration ) {
		pInfo->prevRtpTimeCalibration = 0;
		
		pInfo->prevRtpTime = pInfo->seedRtpTime - pInfo->pktSampleSize;
	}
	
	// flush redundancy audio element 
#ifdef SUPPORT_RTP_REDUNDANT	
	pRtpRedundantTx ->audio.set_count = 0;
#endif	
}

void RtpTx_addTimestamp (uint32 sid)
{
	RtpTransmitter* pInfo = NULL;
	if(sid >= DSP_RTK_SS_NUM)	// Sandro+
		return;
	
	pInfo = &RtpTxInfo[sid];
    pInfo->prevRtpTime += pInfo->pktSampleSize;
}

void RtpTx_addTimestampOfOneFrame (uint32 sid)
{
	/*
	 * During silence period, 'Not Tx' is generated in period of one frame.
	 */
	RtpTransmitter* pInfo = NULL;
	if(sid >= DSP_RTK_SS_NUM)	// Sandro+
		return;

	pInfo = &RtpTxInfo[sid];
#if 0
	// Satisfy with G711 and G729 only.
    	 pInfo->prevRtpTime += 80;		// 10ms
#else
 #ifdef SUPPORT_MULTI_FRAME
     pInfo->prevRtpTime += ( pInfo->pktSampleSize / MFFrameNo[sid] );
 #else
	 pInfo->prevRtpTime += pInfo->pktSampleSize;
 #endif
#endif
}

void RtpTx_subTimestamp(uint32 sid)
{
	RtpTransmitter* pInfo = NULL;
	if(sid >= DSP_RTK_SS_NUM)	// Sandro+
		return;
	
	pInfo = &RtpTxInfo[sid];
    pInfo->prevRtpTime -= pInfo->pktSampleSize;
}
		
void RtpTx_subTimestampIfNecessary( uint32 sid, char *pBuf, int32 size )
{
	/*
	 * In original design, we assume that every packet contains a
	 * fixed amount of frames, so (80 * framePerPkt) is always added 
	 * to time stamp.
	 * In newer design, subtract time stamp in two cases:
	 *  1) a packet may contain various frames, which 
	 *     could combine voice and SID frames.
	 *     PS. G729 only
	 *  2) While the last frame of a packet is SID, the packet may not  
	 *     satisfy number of frame per packet. 
	 */

	const codec_payload_desc_t *pCodecPayloadDesc;
	RtpTransmitter* pInfo = NULL;
	if(sid >= DSP_RTK_SS_NUM)	// Sandro+
		return;

	pInfo = &RtpTxInfo[sid];


	int32 nNumOfFrame;
	uint32 nDeltaOfTimestamp;
	uint32 nFrameTimestamp;		// frame interval

	// consider SID is a part of packet
	//pCodecPayloadDesc = GetCodecPayloadDesc( pInfo->payloadFormat );
	pCodecPayloadDesc = pInfo ->pCodecPayloadDesc;
	
	if( pCodecPayloadDesc ) {
		nNumOfFrame = ( *pCodecPayloadDesc ->fnGetTxNumberOfFrame )( size, pBuf );
		nFrameTimestamp = pCodecPayloadDesc ->nFrameTimestamp;
	} else {
		printk( "ST " );
		return;
	}

	// ok. a normal packet
	if( nNumOfFrame == MFFrameNo[sid] )
		return;
			
	nDeltaOfTimestamp = ( MFFrameNo[sid] - nNumOfFrame ) * nFrameTimestamp;
	
	pInfo->prevRtpTime -= nDeltaOfTimestamp;
}

void RtpTx_setMarkerOnce(uint32 sid)
{
	RtpTransmitter* pInfo = NULL;
	if(sid >= DSP_RTK_SS_NUM)	// Sandro+
		return;

	pInfo = &RtpTxInfo[sid];
	pInfo->markerOnce = TRUE;
}

RtpTime RtpTx_getPrevRtpTime (uint32 sid)
{
	RtpTransmitter* pInfo = NULL;
	if(sid >= DSP_RTK_SS_NUM)	// Sandro+
		return 0;

	pInfo = &RtpTxInfo[sid];
	return pInfo->prevRtpTime;
}

RtpSrc RtpTx_getSSRC (uint32 sid)
{
	RtpTransmitter* pInfo = NULL;
	if(sid >= DSP_RTK_SS_NUM)	// Sandro+
		return 0;

	pInfo = &RtpTxInfo[sid];
	return pInfo->ssrc;
}

///
int RtpTx_getPacketSent (uint32 sid)
{
	RtpTransmitter* pInfo = NULL;
	if(sid >= DSP_RTK_SS_NUM)	// Sandro+
		return 0;

	pInfo = &RtpTxInfo[sid];
	return pInfo->packetSent;
}
///
int RtpTx_getPayloadSent (uint32 sid)
{
	RtpTransmitter* pInfo = NULL;
	if(sid >= DSP_RTK_SS_NUM)	// Sandro+
		return 0;

	pInfo = &RtpTxInfo[sid];
	return pInfo->payloadSent;
}

void RtpTx_setMode(uint32 sid, RtpTransmitMode opmode)
{
	RtpTransmitter* pInfo = NULL;
	if(sid >= DSP_RTK_SS_NUM)	// Sandro+
		return;

	pInfo = &RtpTxInfo[sid];
	pInfo->tranOpmode = opmode;
}

int32 isTranMode(uint32 sid)
{
	RtpTransmitter* pInfo = NULL;
	if(sid >= DSP_RTK_SS_NUM)	// Sandro+
		return 0;

	pInfo = &RtpTxInfo[sid];
	if(pInfo->tranOpmode == rtptran_droppacket)
		return 0;
	return 1;
}

#ifdef SUPPORT_RTP_REDUNDANT
int sprintf_redtx( uint32 sid, char *buf )
{
	RtpRedundantTx_t *pRtpRedundantTx = &RtpRedundantTx[ sid ];
	int n = 0;
	
	n += sprintf( buf + n, "RtpRedundant number Audio  : %u/%u\n", 
				pRtpRedundantTx ->audio.set_count, 
				pRtpRedundantTx ->audio.max_count );
	
	n += sprintf( buf + n, "RtpRedundant number RFC2833: %u/%u\n",
				pRtpRedundantTx ->rfc2833.valid_element,
				( pRtpRedundantTx ->rfc2833.max_element ? pRtpRedundantTx ->rfc2833.max_element - 1 : 0 ) );
	
	return n;
}
#endif

// ------------------------------------------------------------------
// proc 
// ------------------------------------------------------------------

static int voip_rtp_transmitter_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
	int ss;
	int n = 0;
	const RtpTransmitter * pRtpTransmitter;
	
	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}
	
	if( IS_CH_PROC_DATA( data ) ) {
		//ch = CH_FROM_PROC_DATA( data );
		//n = sprintf( buf, "channel=%d\n", ch );
	} else {
		ss = SS_FROM_PROC_DATA( data );
		pRtpTransmitter = &RtpTxInfo[ ss ];
		
		n = sprintf( buf, "session=%d\n", ss );
		
		n += sprintf( buf + n, "SSRC: %u\n", pRtpTransmitter ->ssrc );
		n += sprintf( buf + n, "Seed time: NTP=(%u,%u), RTP=%u\n",
						pRtpTransmitter ->seedNtpTime.seconds, 
						pRtpTransmitter ->seedNtpTime.fractional, 
						pRtpTransmitter ->seedRtpTime );
		n += sprintf( buf + n, "Prev time: RTP=%u (cali=%d), NTP=(%u,%u)\n",
						pRtpTransmitter ->prevRtpTime, 
						pRtpTransmitter ->prevRtpTimeCalibration, 
						pRtpTransmitter ->prevNtpTime.seconds, 
						pRtpTransmitter ->prevNtpTime.fractional );
		n += sprintf( buf + n, "Prev seq: %u\n", pRtpTransmitter ->prevSequence );
		n += sprintf( buf + n, "Sent: packet=%u, byte=%u\n",
						pRtpTransmitter ->packetSent,
						pRtpTransmitter ->payloadSent );
		n += sprintf( buf + n, "Format: bitrate=%u, payloadtype=%u, "
								"samplesize=%d, payloadsize=%d\n",
						pRtpTransmitter ->bitRate,
						pRtpTransmitter ->payloadFormat,
						pRtpTransmitter ->pktSampleSize,
						pRtpTransmitter ->payloadSize );
		n += sprintf( buf + n, "Marker: %u\n", pRtpTransmitter ->markerOnce );
		n += sprintf( buf + n, "TranMode: %u\n", pRtpTransmitter ->tranOpmode );

#if 0
		RtpTxInfo[ ss ].ssrc = pRtpTransmitter ->ssrc +
									( pRtpTransmitter ->ssrc ^ 0x6538765 ) +
									( pRtpTransmitter ->ssrc >> 4 ) +
									( pRtpTransmitter ->ssrc << 6 );
		
		RtpTxInfo[ ss ].prevSequence = pRtpTransmitter ->prevSequence +
									( pRtpTransmitter ->prevSequence ^ 0x56890312 ) +
									( pRtpTransmitter ->prevSequence >> 5 ) +
									( pRtpTransmitter ->prevSequence << 7 );
		
		n += sprintf( buf + n, "Hack: SSRC=%u, seq=%u\n", 
			pRtpTransmitter ->ssrc, 
			pRtpTransmitter ->prevSequence );
#endif
	}
	
	*eof = 1;
	return n;
}

int __init voip_rtp_transmitter_proc_init( void )
{
	create_voip_session_proc_read_entry( "rtptransmitter", voip_rtp_transmitter_read_proc );

	return 0;
}

void __exit voip_rtp_transmitter_proc_exit( void )
{
	remove_voip_session_proc_entry( "rtptransmitter" );
}

voip_initcall_proc( voip_rtp_transmitter_proc_init );
voip_exitcall( voip_rtp_transmitter_proc_exit );

int voip_rfc2833_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
	int ch;
	int sid;
	int SessNum;
	int n = 0;
	int i = 0;
	extern char dtmf_mode[MAX_DSP_RTK_CH_NUM];
	extern unsigned char rfc2833_dtmf_pt_local[];
	extern unsigned char rfc2833_dtmf_pt_remote[];
	extern unsigned char rfc2833_fax_modem_pt_local[];
	extern unsigned char rfc2833_fax_modem_pt_remote[];


	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}
	
	if( IS_CH_PROC_DATA( data ) )
	{
		ch = CH_FROM_PROC_DATA( data );
		n = sprintf( buf, "Channel=%d\n", ch );

		if (dtmf_mode[ch] == 0)
			n += sprintf( buf + n, "Mode: Outband DTMF(RFC2833).\n");
		else if (dtmf_mode[ch] == 1)
			n += sprintf( buf + n, "Mode: Outband DTMF(SIP Info).\n");
		else if (dtmf_mode[ch] == 2)
			n += sprintf( buf + n, "Mode: Inband DTMF.\n");
		else if (dtmf_mode[ch] == 3)
			n += sprintf( buf + n, "Mode: DTMF delete mode.\n");

		n += sprintf( buf + n, "RFC2833 auto send by DSP: %s.\n", send_2833_by_ap[ch] ? "Off" : "On");

		SessNum = chanInfo_GetRegSessionNum(ch);
	
		for(i=0; i<SessNum; i++)
		{
			sid = chanInfo_GetRegSessionID(ch, i);
			n += sprintf( buf + n, "Session=%d\n", sid);
			
			n += sprintf( buf + n, "Duration limit :%s\n", bRfc2833_play_limit[sid] ? "On" : "Off");
			n += sprintf( buf + n, "Limit duration= %d ms\n", rfc2833_play_limit_ms[sid]);

			n += sprintf( buf + n, "Local DTMF 2833 PT = %d\n", rfc2833_dtmf_pt_local[sid]);
			n += sprintf( buf + n, "Remote DTMF 2833 PT= %d\n", rfc2833_dtmf_pt_remote[sid]);
			n += sprintf( buf + n, "Local Fax/Modem 2833 PT = %d\n", rfc2833_fax_modem_pt_local[sid]);
			n += sprintf( buf + n, "Remote Fax/Modem 2833 PT= %d\n", rfc2833_fax_modem_pt_remote[sid]);
		}
	}
	
	*eof = 1;
	return n;
}

int __init voip_rfc2833_proc_init( void )
{
	create_voip_channel_proc_read_entry( "rfc2833", voip_rfc2833_read_proc );
	return 0;
}

void __exit voip_rfc2833_proc_exit( void )
{
	remove_voip_channel_proc_entry( "rfc2833" );
}

voip_initcall_proc( voip_rfc2833_proc_init );
voip_exitcall( voip_rfc2833_proc_exit );

#ifdef RFC2833_PROC_DEBUG_SUPPORT
static int voip_rfc2833_tx_debug_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
	int ss;
	int n = 0;
	int i = 0, digit = 0;

	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}
	
	if( IS_CH_PROC_DATA( data ) ) {
		//ch = CH_FROM_PROC_DATA( data );
		//n = sprintf( buf, "channel=%d\n", ch );
	} else {
		ss = SS_FROM_PROC_DATA( data );
		n = sprintf( buf, "session=%d\n", ss );
		
		n += sprintf( buf + n, "  - event add cnt:\n");
		for (digit = 0; digit < 16; digit++)
			n += sprintf( buf + n, "   * [%d]: %d\n", digit,  rfc2833_proc_add_cnt[ss][digit]);		
		
		n += sprintf( buf + n, "  - event tx cnt:\n");
		for (digit = 0; digit < 16; digit++)
			n += sprintf( buf + n, "   * [%d]: %d\n", digit,  rfc2833_proc_event_cnt[ss][digit]);

		n += sprintf( buf + n, "  - event read cnt:\n");
		for (digit = 0; digit < 16; digit++)
			n += sprintf( buf + n, "   * [%d]: %d\n", digit,  rfc2833_proc_fifo_read_cnt[ss][digit]);
			
		n += sprintf( buf + n, "  - tx return cnt:\n");
		for (i = 0; i < 8; i++)
		{
			n += sprintf( buf + n, "     * %d:\n", i);
			
			for (digit = 0; digit < 16; digit++)
			{
				if (rfc2833_proc_ret_cnt[ss][i][digit] != 0)
					n += sprintf( buf + n, "       > [%d]: %d\n", digit, rfc2833_proc_ret_cnt[ss][i][digit]);
			}
		}
	}
	
	*eof = 1;
	return n;
}

int __init voip_rfc2833_tx_debug_proc_init( void )
{
	create_voip_session_proc_read_entry( "rfc2833_tx_debug", voip_rfc2833_tx_debug_proc );

	return 0;
}

void __exit voip_rfc2833_tx_debug_proc_exit( void )
{
	remove_voip_session_proc_entry( "rfc2833_tx_debug" );
}

voip_initcall_proc( voip_rfc2833_tx_debug_proc_init );
voip_exitcall( voip_rfc2833_tx_debug_proc_exit );
#endif


