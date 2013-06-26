#include "rtk_voip.h"
#include "voip_types.h"
#include "rtpTypes.h"
#include "RtpReceiverXR.h"

// ----------------------------------------------------------- 
//typedef long TFract; /* 2 integer bits, 30 fractional bits */
typedef long TNatual; /* 32 integer bits */

TNatual NaturalSqrt(TNatual x)
{
	register unsigned long root, remHi, remLo, testDiv, count;
	
	root = 0; /* Clear root */
	remHi = 0; /* Clear high part of partial remainder */
	remLo = x; /* Get argument into low part of partial remainder */
	//count = 30; /* Load loop counter */
	count = 15; /* Load loop counter */
	do {
		remHi = (remHi<<2) | (remLo>>30); remLo <<= 2; /* get 2 bits of arg */
		root <<= 1; /* Get ready for the next bit in the root */
		testDiv = (root << 1) + 1; /* Test radical */
		if (remHi >= testDiv) {
			remHi -= testDiv;
			root++;
		}
	} while (count-- != 0);
	
	return(root);
}
// ----------------------------------------------------------- 

// The reference sequence number is an extended sequence number
// that serves as the basis for determining whether a new 16 bit
// sequence number comes earlier or later in the 32 bit sequence
// space.
//u_int32 _src_ref_seq;
//bool	_uninitialized_src_ref_seq;

// Place seq into a 32-bit sequence number space based upon a
// heuristic for its most likely location.
static uint32 extend_seq( RtpReceiverXR *pInfoXR, const uint16 seq )
{
	uint32 extended_seq, seq_a, seq_b, diff_a, diff_b; 
	uint32 _src_ref_seq = pInfoXR ->_src_ref_seq;
	
	if(pInfoXR ->_uninitialized_src_ref_seq) {
	
		// This is the first sequence number received.  Place
		// it in the middle of the extended sequence number
		// space.
		_src_ref_seq                = seq | 0x80000000u;
		pInfoXR ->_uninitialized_src_ref_seq  = 0;
		extended_seq                = _src_ref_seq;
	}
	else {
	
		// Prior sequence numbers have been received.
		// Propose two candidates for the extended sequence
		// number: seq_a is without wraparound, seq_b with
		// wraparound.
		seq_a = seq | (_src_ref_seq & 0xFFFF0000u);
		if(_src_ref_seq < seq_a) {
			seq_b  = seq_a - 0x00010000u;
			diff_a = seq_a - _src_ref_seq;
			diff_b = _src_ref_seq - seq_b;
		}
		else {
			seq_b  = seq_a + 0x00010000u;
			diff_a = _src_ref_seq - seq_a;
			diff_b = seq_b - _src_ref_seq;
		}
		
		// Choose the closer candidate.  If they are equally
		// close, the choice is somewhat arbitrary: we choose
		// the candidate for which no rollover is necessary.
		if(diff_a < diff_b) {
			extended_seq = seq_a;
		}
		else {
			extended_seq = seq_b;
		}
		
		// Set the reference sequence number to be this most
		// recently-received sequence number.
		_src_ref_seq = extended_seq;
	}

	pInfoXR ->_src_ref_seq = _src_ref_seq;
	
	// Return our best guess for a 32-bit sequence number that
	// corresponds to the 16-bit number we were given.
	return extended_seq;
}
// ----------------------------------------------------------- 

static void RtpRx_EventForBurstPacketLossCalc( RtpReceiverXR *pInfoXR,
			int packet_lost, int packet_discarded )
{
	if(packet_lost) {
		pInfoXR ->A2_loss_count++;
	}
	if(packet_discarded) {
		pInfoXR ->A2_discard_count++;
	}
	if(!packet_lost && !packet_discarded) {
		pInfoXR ->A2_pkt++;
	}
	else {
		if(pInfoXR ->A2_pkt >= XR_GMIN) {
			if(pInfoXR ->A2_lost == 1) {
				pInfoXR ->A2_c14++;
			}
			else {
				pInfoXR ->A2_c13++;
			}
			pInfoXR ->A2_lost = 1;
			pInfoXR ->A2_c11 += pInfoXR ->A2_pkt;
		}
		else {
			pInfoXR ->A2_lost++;
			if(pInfoXR ->A2_pkt == 0) {
				pInfoXR ->A2_c33++;
			}
			else {
				pInfoXR ->A2_c23++;
				pInfoXR ->A2_c22 += (pInfoXR ->A2_pkt - 1);
			}
		}
		pInfoXR ->A2_pkt = 0;
	}
}

void RtpRx_FillBurstPacketLossCalc( uint32 sid, 
			RtpReceiverXR *pInfoXR,
			RtcpXRReportVoIPMetrics *pMetrics )
{
#define SDIV( x, y )	( ( y ) ? ( ( x ) / ( y ) ) : 0 )

	extern unsigned short rx_frames_per_packet[MAX_DSP_RTK_SS_NUM];
	extern uint32 GetSessionFramePeriod( uint32 ssid );
	
	uint32 ctotal;
	uint32 p32_q8, p23_q8;
	uint32 c31, c32;
	
	uint32 m;
	
	uint32 burst_density, gap_density, gap_length, burst_length;
	uint32 loss_rate, discard_rate;
	
#if 0
	// A2_c11=0, A2_c13=0, A2_c14=0, A2_c22=0, A2_c23=1, A2_c33=0 l=0,d=1 
	printk( "A2_c11=%u, A2_c13=%u, A2_c14=%u, A2_c22=%u, A2_c23=%u, A2_c33=%u\n",
		pInfoXR ->A2_c11, pInfoXR ->A2_c13, pInfoXR ->A2_c14, 
		pInfoXR ->A2_c22, pInfoXR ->A2_c23, pInfoXR ->A2_c33 );
	printk( "l=%u,d=%u\n", pInfoXR ->A2_loss_count, pInfoXR ->A2_discard_count );
#endif	
	
	// Calculate additional transition counts.
	c31 = pInfoXR ->A2_c13;
	c32 = pInfoXR ->A2_c23;
	ctotal = pInfoXR ->A2_c11 + pInfoXR ->A2_c14 + pInfoXR ->A2_c13 + 
				pInfoXR ->A2_c22 + pInfoXR ->A2_c23 + 
				c31 + c32 + pInfoXR ->A2_c33;
	
	if( ctotal == 0 ) { 			// no loss or discard 
		pMetrics ->lossRate = 0;
		pMetrics ->discardRate = 0;
		pMetrics ->burstDensity = 0;
		pMetrics ->gapDensity = 0;
		pMetrics ->burstDuration = 0;
		pMetrics ->gapDuration = 0;
		return;
	}
	
	// Calculate burst and densities.
	p32_q8 = SDIV( ( c32 << 8 ), (c31 + c32 + pInfoXR ->A2_c33) );
	if((pInfoXR ->A2_c22 + pInfoXR ->A2_c23) < 1) {
		p23_q8 = 1 << 8;
	}
	else {
		p23_q8 = ( 1 << 8 ) - SDIV( ( pInfoXR ->A2_c22 << 8 ), (pInfoXR ->A2_c22 + pInfoXR ->A2_c23) );
	}
	burst_density = SDIV( 256 * p23_q8, (p23_q8 + p32_q8) );
	gap_density = SDIV( 256 * pInfoXR ->A2_c14, (pInfoXR ->A2_c11 + pInfoXR ->A2_c14) );
	
	// Calculate burst and gap durations in ms
	//m = frameDuration_in_ms * framesPerRTPPkt;
	//m = 10 * 1; 
	m = GetSessionFramePeriod( sid ) * rx_frames_per_packet[ sid ];
	gap_length = SDIV( (pInfoXR ->A2_c11 + pInfoXR ->A2_c14 + pInfoXR ->A2_c13) * m, pInfoXR ->A2_c13 );
	burst_length = SDIV( ctotal * m, pInfoXR ->A2_c13 ) - gap_length/*lgap*/;
	
	/* calculate loss and discard rates */
	loss_rate = 256 * pInfoXR ->A2_loss_count / ctotal;
	discard_rate = 256 * pInfoXR ->A2_discard_count / ctotal;
	
	// fill data
	pMetrics ->lossRate = loss_rate;
	pMetrics ->discardRate = discard_rate;
	pMetrics ->burstDensity = burst_density;
	pMetrics ->gapDensity = gap_density;
	pMetrics ->burstDuration = burst_length;
	pMetrics ->gapDuration = gap_length;

#undef SDIV
}


// ----------------------------------------------------------- 

static void RtpRx_InitXR_optional( RtpReceiverXR *pInfoXR, int all )
{
	int i;

	// ----------------------------------------------------
	// periodic statistics (4.1, 4.2, 4.3, 4.6)
	// ----------------------------------------------------	
	// flag
	pInfoXR ->set_periodic = 0;
	
	// sequence
	pInfoXR ->begin_seq = 0x12123434;
	pInfoXR ->end_seq = 0x56567878;

	// arrive 
	for( i = 0; i < MAX_NUM_XR_ARRIVE; i ++ ) {
		pInfoXR ->arrive_timestamp[ i ] = 0;
	}

	// loss  
	for( i = 0; i < MAX_NUM_XR_LOSS / 32; i ++ )
		pInfoXR ->loss_bits[ i ] = 0;
		
	pInfoXR ->loss_count = 0;
	
	// duplicate 
	for( i = 0; i < MAX_NUM_XR_DUP / 32; i ++ )
		pInfoXR ->dup_bits[ i ] = 0xFFFFFFFF;
	
	pInfoXR ->dup_count = 0;
	
	// jitter 
	pInfoXR ->min_jitter = -1;
	pInfoXR ->max_jitter = 0;
	pInfoXR ->sum_jitter = 0;
	pInfoXR ->squ_jitter = 0;
	
	// ttl 
	pInfoXR ->min_ttl = 0x7FFFFFFF;
	pInfoXR ->max_ttl = 0;
	pInfoXR ->sum_ttl = 0;
	pInfoXR ->squ_ttl = 0;
	
	if( !all )
		return;
	
	// ----------------------------------------------------
	// global - A.1. extend seq use 
	// ----------------------------------------------------
	pInfoXR ->_src_ref_seq = 0x12121212;
	pInfoXR ->_uninitialized_src_ref_seq = 1;

	// ----------------------------------------------------
	// global - A.2. burst packet loss calculation  
	// ----------------------------------------------------
	pInfoXR ->A2_loss_count = 0;
	pInfoXR ->A2_discard_count = 0;
	pInfoXR ->A2_pkt = 0;
	pInfoXR ->A2_lost = 0;
	pInfoXR ->A2_c11 = 0;
	pInfoXR ->A2_c13 = 0;
	pInfoXR ->A2_c14 = 0;
	pInfoXR ->A2_c22 = 0;
	pInfoXR ->A2_c23 = 0;
	pInfoXR ->A2_c33 = 0;
	
	// ----------------------------------------------------
	// global statistics (4.7)
	// ----------------------------------------------------
	// flag
	pInfoXR ->set_global = 0;
	
	// sequence
	pInfoXR ->global_begin_seq = 0x34343434;
	
	// loss
	//pInfoXR ->global_loss_count = 0;
	
	// discard
	//pInfoXR ->global_discard_count = 0;
	
}

void RtpRx_InitXR( RtpReceiverXR *pInfoXR )
{
	RtpRx_InitXR_optional( pInfoXR, 1 );
}

void RtpRx_ResetPeriodicXR( RtpReceiverXR *pInfoXR )
{
	RtpRx_InitXR_optional( pInfoXR, 0 );
}

void Rtp_receive_XR( RtpPacket *p, RtpReceiverXR *pInfoXR, 
					int jitter_delay, RtpTime recvRtpTime )
{
	// call by Rtp_receive() 

	// p is an information of current (not raw packet data) 
	// pInfoXR is an corresponding information 
	uint32 seq, end_seq_prev;
	uint16 offset, offset_byte;
	uint32 bit_mask;
	uint16 delta_seq;
	uint32 i;
	
	// sequence
	seq = extend_seq( pInfoXR, getSequence( p ) );
	//seq = getSequence( p );
	
	if( !pInfoXR ->set_periodic ) {
		pInfoXR ->begin_seq = seq;
		end_seq_prev = seq;
	} else {
		end_seq_prev = pInfoXR ->end_seq;
	}
	pInfoXR ->end_seq = seq + 1;
	
	if( !pInfoXR ->set_global )
		pInfoXR ->global_begin_seq = seq;
	
	// calculate offset 
	offset = seq - pInfoXR ->begin_seq;
	offset_byte = offset >> 5;
	
	// arrive 
	if( offset < MAX_NUM_XR_ARRIVE ) {
		pInfoXR ->arrive_timestamp[ offset ] = recvRtpTime;
	}
	
	RtpRx_EventForBurstPacketLossCalc( pInfoXR, 0, 0 );
	
	// duplicate 
#if 1
	bit_mask =  0x80000000UL >> ( offset & 0x1F ); 
#else
	bit_mask =  1 << ( offset & 0x1F ); 
#endif
	
	if( offset < MAX_NUM_XR_DUP &&
		/*offset < MAX_NUM_XR_LOSS &&*/	// MAX_NUM_XR_DUP <= MAX_NUM_XR_LOSS
		pInfoXR ->loss_bits[ offset_byte ] & bit_mask ) 
	{
		pInfoXR ->dup_bits[ offset_byte ] &= ~bit_mask;
		pInfoXR ->dup_count ++;
	}
	
	// loss 
	if( offset < MAX_NUM_XR_LOSS )
		pInfoXR ->loss_bits[ offset_byte ] |= bit_mask;
	
	if( ( delta_seq = ( uint16 )( seq - end_seq_prev ) ) < 32767 ) {
		pInfoXR ->loss_count += delta_seq;
		//pInfoXR ->global_loss_count += delta_seq;
		for( i = 0; i < delta_seq; i ++ )
			RtpRx_EventForBurstPacketLossCalc( pInfoXR, 1, 0 );
	} 		
		
	// jitter 
	if( pInfoXR ->min_jitter > jitter_delay )
		pInfoXR ->min_jitter = jitter_delay;
	if( pInfoXR ->max_jitter < jitter_delay )
		pInfoXR ->max_jitter = jitter_delay;
	pInfoXR ->sum_jitter += jitter_delay;
	pInfoXR ->squ_jitter += jitter_delay * jitter_delay;
	
	// ttl 
	if( pInfoXR ->min_ttl > p ->ip_ttl )
		pInfoXR ->min_ttl = p ->ip_ttl;
	if( pInfoXR ->max_ttl < p ->ip_ttl )
		pInfoXR ->max_ttl = p ->ip_ttl;
	pInfoXR ->sum_ttl += p ->ip_ttl;
	pInfoXR ->squ_ttl += p ->ip_ttl * p ->ip_ttl;
		
	// flag (keep the last one)
	pInfoXR ->set_periodic = 1;
	pInfoXR ->set_global = 1;
}

void RtpRx_IncDiscard( uint32 sid )
{
	extern RtpReceiver* RtpRx_getInfo (uint32 sid);
	RtpReceiver * const rtpReceiver = RtpRx_getInfo( sid );
	RtpReceiverXR * const rtpReceiverXR = &rtpReceiver ->xr;

	//rtpReceiverXR ->global_discard_count ++;
	RtpRx_EventForBurstPacketLossCalc( rtpReceiverXR, 0, 1 );
}
	
void RtcpXR_FillSummaryBlock( RtcpXRReportStatisticsSummary *pReport, 
					const RtpReceiverXR *pInfoXR )
{
	uint16 delta_seq = pInfoXR ->end_seq - pInfoXR ->begin_seq;
	uint32 mean, t;
	
	// loss
	pReport ->lost_packets = pInfoXR ->loss_count;
	
	// dup 
	pReport ->dup_packets = pInfoXR ->dup_count;	
	
	// jitter 
	pReport ->min_jitter = pInfoXR ->min_jitter;
	pReport ->max_jitter = pInfoXR ->max_jitter;
	
	mean = pInfoXR ->sum_jitter / delta_seq;
	pReport ->mean_jitter = mean;
	
	t = pInfoXR ->squ_jitter / delta_seq - mean * mean;
	pReport ->dev_jitter = NaturalSqrt( t );
	
	// ttl
	pReport ->min_ttl_or_hl = pInfoXR ->min_ttl;
	pReport ->max_ttl_or_hl = pInfoXR ->max_ttl;
	
	mean = pInfoXR ->sum_ttl / delta_seq;
	pReport ->mean_ttl_or_hl = mean;
	
	if( pReport ->min_ttl_or_hl == pReport ->max_ttl_or_hl )
		pReport ->dev_ttl_or_hl = 0;
	else 
	{
		t = pInfoXR ->squ_ttl / delta_seq - mean * mean;
		pReport ->dev_ttl_or_hl = NaturalSqrt( t );
	}
}

