#include "rtk_voip.h"
#include "voip_types.h"
#include "rtpTypesXR.h"
#include "RtcpXR_RLE.h"

/*
 * This RLE (Run Length Encode) algorithm is greedy, because  
 * computation power is limited. 
 * 
 * Algorithm:
 * ~~~~~~~~~~
 * definition:
 *   m_state = 0: 0's, 1: 1's, 2: BV (Bit vector)
 * 
 * state transition:
 *   0/1 --> 0: 32 0's 
 *   0/1 --> 1: 32 1's
 *   0/1 --> 2: prefix (30/15) 0's ..... RLE (30/15) then others be bit vector 
 *   0/1 --> 2: prefix (30/15) 1's ..... RLE (30/15) then others be bit vector 
 *   0/1 --> 2: other cases
 *   2   --> 0: suffix 0's
 *   2   --> 1: suffix 1's
 *   2   --> 2: other cases
 * 
 * remaining bits:
 *   If data is not multiple of WORD, the last remaining bits will be
 *   treated as bit vector. 
 * 
 */

#define __inline	inline

const uint32 bits_mask[] = { 
#if 1
	0x80000000, 0xC0000000, 0xE0000000, 0xF0000000, 
	0xF8000000, 0xFC000000, 0xFE000000, 0xFF000000, 
	0xFF800000, 0xFFC00000, 0xFFE00000, 0xFFF00000, 
	0xFFF80000, 0xFFFC0000, 0xFFFE0000, 0xFFFF0000, 
	0xFFFF8000, 0xFFFFC000, 0xFFFFE000, 0xFFFFF000, 
	0xFFFFF800, 0xFFFFFC00, 0xFFFFFE00, 0xFFFFFF00, 
	0xFFFFFF80, 0xFFFFFFC0, 0xFFFFFFE0, 0xFFFFFFF0, 
	0xFFFFFFF8, 0xFFFFFFFC, 0xFFFFFFFE, 0xFFFFFFFF, 
#else
	0x00000001, 0x00000003, 0x00000007, 0x0000000F, 
	0x0000001F, 0x0000003F, 0x0000007F, 0x000000FF,
	0x000001FF, 0x000003FF, 0x000007FF, 0x00000FFF,
	0x00001FFF, 0x00003FFF, 0x00007FFF, 0x0000FFFF,
	0x0001FFFF, 0x0003FFFF, 0x0007FFFF, 0x000FFFFF, 
	0x001FFFFF, 0x003FFFFF, 0x007FFFFF, 0x00FFFFFF,
	0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF, 0x0FFFFFFF,
	0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF,
#endif
};

#define MAX_RUN_LENGTH		( 16383 )

#define M_32_1S( x )		( x == 0xFFFFFFFF )
#define M_32_0S( x )		( x == 0x00000000 )
#if 1
#define M_15_1S( x )		( ( x & 0xFFFE0000 ) == 0xFFFE0000 )
#define M_15_0S( x )		( ( x & 0xFFFE0000 ) == 0x00000000 )
#define M_30_1S( x )		( ( x & 0xFFFFFFFC ) == 0xFFFFFFFC )
#define M_30_0S( x )		( ( x & 0xFFFFFFFC ) == 0x00000000 )
#else
#define M_15_1S( x )		( ( x & 0x00007FFF ) == 0x00007FFF )
#define M_15_0S( x )		( ( x & 0x00007FFF ) == 0x00000000 )
#define M_30_1S( x )		( ( x & 0x3FFFFFFF ) == 0x3FFFFFFF )
#define M_30_0S( x )		( ( x & 0x3FFFFFFF ) == 0x00000000 )
#endif

static __inline int CheckSuffix1s( uint32 x, int used_bits )
{
	// used_bits = 1 ~ 32
	uint32 mask;

	mask = ~bits_mask[ used_bits - 1 ];

	if( ( x & mask ) == mask )
		return 1;

	return 0;
}

static __inline int CheckSuffix0s( uint32 x, int used_bits )
{
	// used_bits = 1 ~ 32
	uint32 mask;

	mask = ~bits_mask[ used_bits - 1 ];

	if( ( x & mask ) == 0 )
		return 1;

	return 0;
}

static __inline int GetPrefix1s( uint32 x )
{
	if( M_32_1S( x ) )
		return 32;

	if( M_30_1S( x ) )
		return 30;

	if( M_15_1S( x ) )
		return 15;

	return 0;
}

static __inline int GetPrefix0s( uint32 x )
{
	if( M_32_0S( x ) )
		return 32;

	if( M_30_0S( x ) )
		return 30;

	if( M_15_0S( x ) )
		return 15;

	return 0;
}

uint32 RunLengthEncode( uint32 *src, uint32 bits, RtcpXRChunk *chunk )
{
#define NEW_RUN_LENGTH( rt )			\
	do {								\
	chunk ++; m_chunk_num ++;			\
	chunk ->runLength.chunkType = 0;	\
	chunk ->runLength.runType = rt;		\
	chunk ->runLength.runLength = 0;	\
	} while( 0 )

#define NEW_BIT_VECTOR()				\
	do {								\
	chunk ++; m_chunk_num ++;			\
	chunk ->bitVector.chunkType = 1;	\
	chunk ->bitVector.bitVector = 0;	\
	} while( 0 )

#define NEW_NULL_CHUNK()				\
	do {								\
	chunk ++; m_chunk_num ++;			\
	chunk ->general.chunkType = 0;		\
	chunk ->general.others = 0;		\
	} while( 0 )

#define INC_RUN_LENGTH( rt, len )									\
	do {															\
		int __tt;													\
		__tt = ( int )chunk ->runLength.runLength + len;			\
																	\
		if( __tt > MAX_RUN_LENGTH ) {								\
			chunk ->runLength.runLength = MAX_RUN_LENGTH;			\
																	\
			NEW_RUN_LENGTH( rt );									\
			chunk ->runLength.runLength = __tt - MAX_RUN_LENGTH;	\
		} else														\
			chunk ->runLength.runLength = __tt;						\
	} while( 0 )

	int m_state = 2;		// 0: 0's, 1: 1's, 2: bv 
	int m_chunk_num = 0;	// chunk num 
	int m_blank_dst = 0;	// for bv blank number 
	int m_used_src = 0;		// src used bits (fragement uint32)

	int m_not_inc_src = 0;

	int n, t;
	
	uint32 src_uint32 = bits / 32;
	uint32 remain_bits = bits & 0x1F;	// treat remain bit as special 
	
	chunk --;	// minus one to fit state transition 
	
	while( src_uint32 ) {

		switch( m_state ) {
		case 0:
		case 1:
			// 1's 
			if( ( n = GetPrefix1s( *src ) ) ) {

				if( m_state == 0 )		// 0 --> 1
					NEW_RUN_LENGTH( 1 );

				INC_RUN_LENGTH( 1, n );

				if( n == 32 )
					m_state = 1;
				else {
					m_used_src = n;
					m_blank_dst = 0;
					m_state = 2;	m_not_inc_src = 1;
				}
				break;
			} 
			
			// 0's 
			if( ( n = GetPrefix0s( *src ) ) ) {

				if( m_state == 1 )		// 1 --> 0
					NEW_RUN_LENGTH( 0 );

				INC_RUN_LENGTH( 0, n );

				if( n == 32 ) 
					m_state = 0;
				else {
					m_used_src = n;
					m_blank_dst = 0;
					m_state = 2;	m_not_inc_src = 1;
				}
				break;
			} 

			// other .. pass to state 2
			m_used_src = 0;
			m_blank_dst = 0;
			m_state = 2;	m_not_inc_src = 1;
			break;

		case 2:
			// current vector is full 
			if( m_blank_dst == 0 ) {
				if( CheckSuffix1s( *src, m_used_src ) )
					t = 1;
				else if( CheckSuffix0s( *src, m_used_src ) )
					t = 0;
				else
					t = -1;

				if( t >= 0 ) {
					NEW_RUN_LENGTH( t );
					chunk ->runLength.runLength = 32 - m_used_src;
					m_state = t;
					break;
				}

				NEW_BIT_VECTOR();
				m_blank_dst = 15;
			}

			// fill current vector 
			n = ( m_blank_dst > 32 - m_used_src ? 32 - m_used_src : m_blank_dst );

#if 1
			chunk ->bitVector.bitVector |= 
				( ( *src & ( bits_mask[ n ] >> m_used_src ) ) << m_used_src ) >> ( 17 + 15 - m_blank_dst );
#else
			chunk ->bitVector.bitVector |= 
				( ( *src & ( bits_mask[ n ] << m_used_src ) ) >> m_used_src ) << ( 15 - m_blank_dst );
#endif

			m_used_src += n;
			m_blank_dst -= n;

			// keep in state 2
			m_state = 2;	
			
			if( m_used_src != 32 ) {
				m_not_inc_src = 1;
			}

			break;
		}
			
		if( !m_not_inc_src ) {
			src_uint32 --;
			src ++;
			m_used_src = 0;
		} else
			m_not_inc_src = 0;
	}

	// process remain bits (use bit vector)
	while( remain_bits ) {
		if( m_state == 2 && m_blank_dst )
			;
		else {
			NEW_BIT_VECTOR();
			m_blank_dst = 15;
		}

		// fill current vector 
		n = ( m_blank_dst > 32 - m_used_src ? 32 - m_used_src : m_blank_dst );
		if( n > remain_bits )
			n = remain_bits;

#if 1
		chunk ->bitVector.bitVector |= 
			( ( *src & ( bits_mask[ n ] >> m_used_src ) ) << m_used_src ) >> ( 17 + 15 - m_blank_dst );
#else
		chunk ->bitVector.bitVector |= 
			( ( *src & ( bits_mask[ n ] << m_used_src ) ) >> m_used_src ) << ( 15 - m_blank_dst );
#endif

		m_used_src += n;
		m_blank_dst -= n;
		remain_bits -= n;
	}

	// add null chunk 
	NEW_NULL_CHUNK();

	if( m_chunk_num & 0x01 ) {
		NEW_NULL_CHUNK();
	}
		
	return m_chunk_num;	// it MUST be even 
}

