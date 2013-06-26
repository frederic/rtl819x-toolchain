#include <stdio.h>
#include "ui_config.h"
#include "com_ime_def.h"
#include "res_ime_gb2312_wubihua.h"
#include "com_ime_wubihua.h"
#include "gs_drawtext.h"
#include "res_image.h"
#include "ui_vkey.h"

#ifdef LANG_GB2312

#pragma pack(1)

typedef union codes4to5_s {
	struct {
#ifdef _TEST_MODE
		unsigned char code5:4;
		unsigned char code4:4;
#else
		unsigned char code4:4;
		unsigned char code5:4;
#endif
	};
	unsigned char all;
} codes4to5_t;

typedef union codes_s {
	struct {
		union {
			struct {
				unsigned char code1;
				unsigned char code2;
				unsigned char code3;
			};
			struct {
				unsigned char code1;
				unsigned char code2;
				unsigned char code3;
			} code1to3;
		};
		union {
			unsigned char code4to5;
			struct {
#ifdef _TEST_MODE
			unsigned char code5:4;
			unsigned char code4:4;
#else
			unsigned char code4:4;
			unsigned char code5:4;
#endif
			};
		};
	};
	unsigned long all;
} codes_t;

#pragma pack()

CT_ASSERT( sizeof( codes4to5_t ) == sizeof( ( ( codes4to5_t *)0 ) ->all ) );
CT_ASSERT( sizeof( codes_t ) == sizeof( ( ( codes_t *)0 ) ->all ) );

static codes_t maskOneCode[ 5 ] = { 
	{ { { { 0, 0, 0 } }, { 0 } } },	
	{ { { { 0, 0, 0 } }, { 0 } } },
	{ { { { 0, 0, 0 } }, { 0 } } },
	{ { { { 0, 0, 0 } }, { 0 } } },
	{ { { { 0, 0, 0 } }, { 0 } } },
};

/* ============================================================================ */
/* IME WuBiHua Table Access Functions */
/*
 * If one want to modify these functions, you should know naming. 
 * (no suffix) ... reference to context variables, and modify
 * _CONST      ... reference to context variables, but not modify 
 * _STATIC     ... not reference to context variables 
 *
 * Type:
 *   isi       ... Index of Supra-Index 
 *   iwo       ... Index of Word Offset table 
 *   wo        ... Word Offset 
 */

#if 0
//static const unsigned char WuBiHuaCodeLevel2[ ] = {
//	'\x0', 1, 2, 3, 4, 5, 
//};
//
//#define WuBiHuaCodeLevel3		WuBiHuaCodeLevel2
//#define WuBiHuaCodeLevel4		WuBiHuaCodeLevel2
//#define WuBiHuaCodeLevel5		WuBiHuaCodeLevel2
//
//#define KINDS_OF_CODE2			( sizeof( WuBiHuaCodeLevel2 ) / sizeof( WuBiHuaCodeLevel2[ 0 ] ) )
//#define KINDS_OF_CODE3			( sizeof( WuBiHuaCodeLevel3 ) / sizeof( WuBiHuaCodeLevel3[ 0 ] ) )
//#define KINDS_OF_CODE4			( sizeof( WuBiHuaCodeLevel4 ) / sizeof( WuBiHuaCodeLevel4[ 0 ] ) )
//#define KINDS_OF_CODE5			( sizeof( WuBiHuaCodeLevel5 ) / sizeof( WuBiHuaCodeLevel5[ 0 ] ) )
//
//CT_ASSERT( KINDS_OF_CODE2 == 6 );
//CT_ASSERT( KINDS_OF_CODE3 == 6 );
//CT_ASSERT( KINDS_OF_CODE4 == 6 );
//CT_ASSERT( KINDS_OF_CODE5 == 6 );
#else
#define KINDS_OF_CODE2			6
#define KINDS_OF_CODE3			6
#define KINDS_OF_CODE4			6
#define KINDS_OF_CODE5			6
#endif

#define INVALID_CODES			0xFFFF
#define WILDCARD_CODE			6

typedef unsigned short isi_t;
typedef unsigned short iwo_t;
typedef unsigned short wo_t;

typedef struct {
	struct {
		iwo_t start;
		iwo_t end;
	} iwop;
	struct {
		wo_t start;
		wo_t end;
	} wop;
} qresult_t;

static struct {
	struct {
		struct {
			wo_t start;	/* reference to word list */
			wo_t end;		/* not include this code */
		} wo[ 5 ];
		unsigned short total;		/* total candidates */
		unsigned short current;		/* between start and end */
		unsigned short previous;	/* number of previous retrieved candidiates, so that they are shown in screen */
		union {
			struct {
				unsigned long forward:1;	/* previous is forward retrieve */
			};
			unsigned long all;
		} b;
	} candidate;
	struct {
		struct {
			iwo_t start;
			iwo_t end;
		} iwo[ 5 ];			
	} offset;
	struct {
		struct {
			struct {
				unsigned short total;
			} wo[ 5 ];
		} candidate;
		struct {
			qresult_t qresult;	/* for back use only */
			codes_t codes;
		} first[ 5 ];
		codes_t codes;			/* user's input codes (ie. '6' can present) */
		unsigned short mask;	/* wildcard mask for each level */
	} wildcard;
	codes_t codes;			/* current codes (ie. '6' will not present) */
	unsigned int level;		/* now, input how many codes */

	/* compose codes for UI functions */
	codes_cyclic_t cyclic;
} wbh;

static isi_t isiFromCode1to3_STATIC( unsigned char code1, unsigned char code2, unsigned char code3 )
{
	isi_t index;

	if( code1 < 1 || code1 > 5 || code2 > 5 || code3 > 5 )
		return INVALID_CODES;

	index = ( code1 - 1 ) * KINDS_OF_CODE2 * KINDS_OF_CODE3;
	index += code2 * KINDS_OF_CODE3;
	index += code3;

	return index;
}

static iwo_t iwoFromIsi_STATIC( isi_t isi )
{
	int i;

	if( isi == 0 )
		return 0;

	isi --;

	for ( i = 0; i < WBH_SUPRA_INDEX_BOUNDARY_NUM; i ++ )
		if( isi < wbhSupraIndexBoundary[ i ] )
			break;

	return ( iwo_t )wbhSupraIndex[ isi ] | ( ( iwo_t )i << 8 );
}

static wo_t woFromIwo_STATIC( iwo_t iwo )
{
	int i;
	int startbyte;
	wo_t wo;

	if( iwo == 0 )
		return 0;

	iwo --;

#if WBH_WORD_OFFSET_BOUNDARY_NUM == 1
	if( iwo < wbhWordOffsetBoundary[ 0 ] )
		i = 0;
	else
		i = 1;
#else
	for( i = 0; i < WBH_WORD_OFFSET_BOUNDARY_NUM; i ++ )
		if( iwo < wbhWordOffsetBoundary[ i ] )
			break;
#endif

	startbyte = iwo / 2 * 3;

	if( ( iwo & 1 ) == 0 ) {
		wo = ( wo_t )wbhWordOffset[ startbyte ] | 
						( ( ( wo_t )wbhWordOffset[ startbyte + 1 ] << 8 ) & 0x0F00 );
	} else {
		wo = ( wo_t )wbhWordOffset[ startbyte + 2 ] | 
						( ( ( wo_t )wbhWordOffset[ startbyte + 1 ] << 4 ) & 0x0F00 );
	}

	wo |= ( i << 12 );

	return wo;
}

static codes4to5_t codes4to5FromCharCode_STATIC( unsigned char code, unsigned int level )
{
	/*
	 * NOTE: Pay attention to this function, because it can NOT discriminate between 
	 * 'not found' and '\x0' code. 
	 * The reason is that both of them are 0. 
	 */
	codes4to5_t codes4to5;

	codes4to5.all = 0;

	if( code > 5 )	/* code range: 0~5 */
		return codes4to5;

	switch( level ) {
	case 4:
		codes4to5.code4 = code;
		break;
	case 5:
		codes4to5.code5 = code;
		break;
	default:
		debug_out( "codes4to5 process level between 4 and 5 only.\n" );
		return codes4to5;	/* return 0 */
	}

	return codes4to5;	/* return 0 */
}

static int QueryCandidatesOfCodes1to3_STATIC( unsigned char code1, unsigned char code2, 
											  unsigned char code3, 
											  unsigned int level, qresult_t *pqresult )
{
	isi_t isiStart, isiEnd;
	iwo_t iwoStart, iwoEnd;

	/* normal case */
	isiStart = isiFromCode1to3_STATIC( code1, code2, code3 );

	switch( level ) {
	case 1:
		isiEnd = isiStart + KINDS_OF_CODE2 * KINDS_OF_CODE3;
		break;
	case 2:
		isiEnd = isiStart + KINDS_OF_CODE2;
		break;
	case 3:
		isiEnd = isiStart + 1;
		break;
	}

	if( isiStart == INVALID_CODES )
		return 0;
	else {
		iwoStart = iwoFromIsi_STATIC( isiStart );
		iwoEnd = iwoFromIsi_STATIC( isiEnd );
		if( iwoStart == iwoEnd )	/* No this code */
			return 0;

		if( pqresult ) {
			pqresult ->iwop.start = iwoStart;
			pqresult ->iwop.end = iwoEnd;
		}

		return 1;
	}	
}

static int QueryCandidatesOfCodes4to5_STATIC( iwo_t iwoStart, iwo_t iwoEnd, codes4to5_t codes4to5, 
											  unsigned int level, qresult_t *pqresult  )
{
	iwo_t i;
	codes4to5_t mask;
	int hit;

	mask.all = 0;

	switch( level ) {
	case 4:
		mask.code4 = 0xFF;
		break;
	case 5:
		mask.code4 = mask.code5 = 0xFF;
		break;
	default:
		debug_out( "Bad codes level\n" );
		return 0;
	}

	/* search for range */
	hit = 0;

	for( i = iwoStart; i < iwoEnd; i ++ ) {
		if( ( ( ( codes4to5_t * )&wbhCodes4to5[ i ] ) ->all & mask.all ) ==
			( codes4to5.all & mask.all ) )
		{
			if( hit )
				continue;

			hit = 1;
			if( pqresult )
				pqresult ->iwop.start = i;
			else
				return 1;
		} else {

			if( hit )
				break;
		}
	}

	if( hit && pqresult )
		pqresult ->iwop.end = i;

	return hit;
}

static int QueryCandidatesOfCodes1to5_STATIC( codes_t codes, 
											  unsigned int level, qresult_t *pqresult )
{
	qresult_t iwoQresult;
	codes4to5_t codes4to5;

	static struct {
		codes_t codes;
		qresult_t qresult;
	} cache = {
		{ { { { 0, 0, 0 } }, { 0 } } },
		{ { 0, 0 }, { 0, 0 } },
	};
	static const codes_t maskCodes1to3 = { { { { 0xFF, 0xFF, 0xFF } }, { 0 } } };

	switch( level ) {
	case 1:
		return QueryCandidatesOfCodes1to3_STATIC( codes.code1, '\x0', '\x0', level, pqresult );
	case 2:
		return QueryCandidatesOfCodes1to3_STATIC( codes.code1, codes.code2, '\x0', level, pqresult );
	case 3:
		return QueryCandidatesOfCodes1to3_STATIC( codes.code1, codes.code2, codes.code3, level, pqresult );
	case 4:
	case 5:

		/* check cache */ 
		if( ( codes.all & maskCodes1to3.all ) == ( cache.codes.all & maskCodes1to3.all ) ) {
			iwoQresult = cache.qresult;
			goto label_iwo_qresult_ok;
		}

		if( QueryCandidatesOfCodes1to3_STATIC( codes.code1, codes.code2, codes.code3, 
												3, &iwoQresult ) == 0 )
		{
			return 0;
		} else {
			/* fill last record to cache */
			cache.codes.code1to3 = codes.code1to3;
			cache.qresult = iwoQresult;
		}

label_iwo_qresult_ok:
		codes4to5.code4 = codes.code4;
		codes4to5.code5 = codes.code5;

		return QueryCandidatesOfCodes4to5_STATIC( iwoQresult.iwop.start, 
												  iwoQresult.iwop.end, 
												  codes4to5, 
												  level,
												  pqresult );

	}

	return 1;
}

static inline int FindNextWildcardPermutation_STATIC( unsigned int newLevel, 
													codes_t *pcodes, codes_t wildcard_codes )
{
	int carry = 0, inc = 1;

#define DO_PERMUTATION( x )		case x:																	\
								if( wildcard_codes.code ## x == WILDCARD_CODE ) {						\
									if( inc || carry ) {												\
										inc = 0; carry = 0;												\
										if( ++ pcodes ->code ## x == WILDCARD_CODE ) {						\
											pcodes ->code ## x = 1;										\
											carry = 1;													\
										} 																\
									}																	\
								}

	switch( newLevel ) {
	DO_PERMUTATION( 5 );
	DO_PERMUTATION( 4 );
	DO_PERMUTATION( 3 );
	DO_PERMUTATION( 2 );
	DO_PERMUTATION( 1 );
		break;
	}
#undef DO_PERMUTATION

	if( carry )		/* no more permutation */
		return 0;

	return 1;
}

static inline int FindPrevWildcardPermutation_STATIC( unsigned int newLevel, 
													codes_t *pcodes, codes_t wildcard_codes )
{
	int carry = 0, dec = 1;

#define DO_PERMUTATION( x )		case x:																	\
								if( wildcard_codes.code ## x == WILDCARD_CODE ) {						\
									if( dec || carry ) {												\
										dec = 0; carry = 0;												\
										if( -- pcodes ->code ## x == 0 ) {								\
											pcodes ->code ## x = 5;										\
											carry = 1;													\
										} 																\
									}																	\
								}

	switch( newLevel ) {
	DO_PERMUTATION( 5 );
	DO_PERMUTATION( 4 );
	DO_PERMUTATION( 3 );
	DO_PERMUTATION( 2 );
	DO_PERMUTATION( 1 );
		break;
	}
#undef DO_PERMUTATION

	if( carry )		/* no more permutation */
		return 0;

	return 1;
}

static inline void FindFirstWildcardPermutation_STATIC( unsigned int newLevel, 
													codes_t *pcodes, codes_t *pwildcard_codes,
													unsigned char code )
{
#define M_WILDCARD_CODE_REPLACE( x )	case x:												\
										if( newLevel == x ) {								\
											pwildcard_codes ->code ## x = code;				\
											pcodes ->code ## x = code;						\
										}													\
										if( pwildcard_codes ->code ## x == WILDCARD_CODE )	\
											pcodes ->code ## x = 1
	switch( newLevel ) {
		M_WILDCARD_CODE_REPLACE( 5 );
		M_WILDCARD_CODE_REPLACE( 4 );
		M_WILDCARD_CODE_REPLACE( 3 );
		M_WILDCARD_CODE_REPLACE( 2 );
		M_WILDCARD_CODE_REPLACE( 1 );
		break;
	}

#undef M_WILDCARD_CODE_REPLACE
}

static inline int CheckIfNormalCodesExistBeyondPreviousOne_CONST( unsigned char code )
{
	const unsigned int newLevel = wbh.level + 1;		/* current level of this code */
	unsigned int idxLevel;
	codes4to5_t codes4to5, codes4to5_mask;
	
	codes4to5_mask.all = 0;

	switch( newLevel ) {
	case 1:
		return QueryCandidatesOfCodes1to3_STATIC( code, '\x0', '\x0', newLevel, NULL );
	case 2:
		return QueryCandidatesOfCodes1to3_STATIC( wbh.codes.code1, code, '\x0', newLevel, NULL );
	case 3:
		return QueryCandidatesOfCodes1to3_STATIC( wbh.codes.code1, wbh.codes.code2, code, newLevel, NULL );

	case 5:
		codes4to5_mask.code4 = 0xFF;
	case 4:
		codes4to5 = codes4to5FromCharCode_STATIC( code, newLevel );

		if( codes4to5.all == 0 )
			return 0;

		idxLevel = wbh.level - 1;
		codes4to5.all |= ( ( ( codes4to5_t * )&wbh.codes.code4to5 ) ->all & codes4to5_mask.all );

		return QueryCandidatesOfCodes4to5_STATIC( wbh.offset.iwo[ idxLevel ].start, 
											 wbh.offset.iwo[ idxLevel ].end, 
											 codes4to5,
											 newLevel,
											 NULL );
	}

	return 0;
}

static int GetWildcardCodesAmountBeyondPreviousOne_CONST( unsigned char code, 
														  unsigned short *pTotal,
														  codes_t *pFirstCodes )
{
	const unsigned int newLevel = wbh.level + 1;		/* current level of this code */
	codes_t codes, wildcard_codes;
	qresult_t qresult;
	unsigned short total = 0;
	int bFirstCodes = 1;

	if( newLevel >= 6 ) 
		return 0;

	/* set first codes */
	codes = wildcard_codes = wbh.wildcard.codes;

	FindFirstWildcardPermutation_STATIC( newLevel, &codes, &wildcard_codes, code );

	/* try permutation */
	while( 1 ) {
		if( pTotal ) {
			if( QueryCandidatesOfCodes1to5_STATIC( codes, newLevel, &qresult ) ) {

				total += woFromIwo_STATIC( qresult.iwop.end ) - 
								woFromIwo_STATIC( qresult.iwop.start  );

				if( bFirstCodes ) {
					bFirstCodes = 0;
					pFirstCodes ->all = codes.all;
				}
			}
		} else {
			if( QueryCandidatesOfCodes1to5_STATIC( codes, newLevel, NULL ) )
				return 1;
		}

		/* try next permutation */
		if( FindNextWildcardPermutation_STATIC( newLevel, &codes, wildcard_codes ) == 0 )
			break;
	}

	if( pTotal ) {
		if( ( *pTotal = total ) )
			return 1;
	}

	return 0;
}

static inline int CheckIfWildcardCodesExistBeyondPreviousOne_CONST( unsigned char code )
{
	return GetWildcardCodesAmountBeyondPreviousOne_CONST( code, NULL, NULL );
}

static int CheckIfCodesExistBeyondPreviousOne_CONST( unsigned char code )
{
	if( code == WILDCARD_CODE || wbh.wildcard.mask ) {
		/* wildcard codes */
		return CheckIfWildcardCodesExistBeyondPreviousOne_CONST( code );
	}

	/* normal codes */
	return CheckIfNormalCodesExistBeyondPreviousOne_CONST( code );
}

static inline void FillCandidatesInformationRelyOnQueryResult( const qresult_t *pqresult )
{
	/* NOTE: This function increases level!! */
	wo_t wostart, woend;

	wbh.offset.iwo[ wbh.level ].start = pqresult ->iwop.start;
	wbh.offset.iwo[ wbh.level ].end = pqresult ->iwop.end;

	wostart = woFromIwo_STATIC( pqresult ->iwop.start );
	woend = woFromIwo_STATIC( pqresult ->iwop.end );	/* woFromIwo_STATIC retrieve begining of offset */

	wbh.candidate.wo[ wbh.level ].start = wostart;
	wbh.candidate.wo[ wbh.level ].end = woend;

	wbh.candidate.total = woend - wostart;
	wbh.candidate.current = 0;
	wbh.candidate.previous = 0;
	wbh.candidate.b.all = 0;

	wbh.level ++;
}

static inline int ComposeNormalCodesToRetrieveCandidatesCore( unsigned char code )
{
	const unsigned int newLevel = wbh.level + 1;		/* current level of this code */
	unsigned int idxLevel;
	qresult_t qresult;
	codes4to5_t codes4to5, codes4to5_mask;

	codes4to5_mask.all = 0;

	switch( newLevel ) {
	case 1:
		if( QueryCandidatesOfCodes1to3_STATIC( code, '\x0', '\x0', newLevel, &qresult ) ) {
			wbh.codes.code1 = wbh.wildcard.codes.code1 = code;
			
			goto label_process_iwo;
		}
		break;

	case 2:
		if( QueryCandidatesOfCodes1to3_STATIC( wbh.codes.code1, code, '\x0', newLevel, &qresult ) ) {
			wbh.codes.code2 = wbh.wildcard.codes.code2 = code;

			goto label_process_iwo;
		}
		break;

	case 3:
		if( QueryCandidatesOfCodes1to3_STATIC( wbh.codes.code1, wbh.codes.code2, code, newLevel, &qresult ) ) {
			wbh.codes.code3 = wbh.wildcard.codes.code3 = code;

			goto label_process_iwo;
		}
		break;

	case 5:
		codes4to5_mask.code4 = 0xFF;
	case 4:
		codes4to5 = codes4to5FromCharCode_STATIC( code, newLevel );

		if( codes4to5.all == 0 ) {
			debug_out( "Compose a non-existed code?\n" );
			return 0;
		}

		idxLevel = wbh.level - 1;
		codes4to5.all |= ( ( ( codes4to5_t * )&wbh.codes.code4to5 ) ->all & codes4to5_mask.all );

		if( QueryCandidatesOfCodes4to5_STATIC( wbh.offset.iwo[ idxLevel ].start, 
											   wbh.offset.iwo[ idxLevel ].end, 
											   codes4to5,
											   newLevel,
											   &qresult ) )
		{
			wbh.codes.code4to5 = wbh.wildcard.codes.code4to5 = *( ( unsigned char * )&codes4to5 );

			goto label_process_iwo;
		}
		break;
	}

	debug_out( "Compose code error\n" );
	return 0;

label_process_iwo:
	FillCandidatesInformationRelyOnQueryResult( &qresult );

	return 1;
}

static inline int ComposeWildcardCodesToRetrieveCandidatesCore( unsigned char code )
{
	unsigned short total;
	codes_t firstcodes;
	qresult_t qresult;
	const unsigned int newLevel = wbh.level + 1;

	if( GetWildcardCodesAmountBeyondPreviousOne_CONST( code, &total, &firstcodes ) == 0 ) {
		debug_out( "Wildcard has no candidates?\n" );
		return 0;
	}

	if( QueryCandidatesOfCodes1to5_STATIC( firstcodes, newLevel, &qresult ) == 0 ) {
		debug_out( "Query first code error\n" );
		return 0;
	}

	/* fill codes */
	wbh.codes.all = firstcodes.all;

	/* fill wildcard relative data */
	switch( newLevel ) {
	case 1:
		wbh.wildcard.codes.code1 = code;
		break;
	case 2:
		wbh.wildcard.codes.code2 = code;
		break;
	case 3:
		wbh.wildcard.codes.code3 = code;
		break;
	case 4:
		wbh.wildcard.codes.code4 = code;
		break;
	case 5:
		wbh.wildcard.codes.code5 = code;
		break;
	}

	if( code == WILDCARD_CODE )
		wbh.wildcard.mask |= ( 1 << wbh.level );

	wbh.wildcard.candidate.wo[ wbh.level ].total = total;
	wbh.wildcard.first[ wbh.level ].qresult = qresult;
	wbh.wildcard.first[ wbh.level ].codes = firstcodes;

	/* fill common data. (This function increase wbh.level) */
	FillCandidatesInformationRelyOnQueryResult( &qresult );

	return 1;
}

static int ComposeCodesToRetrieveCandidatesCore( unsigned char code )
{
	if( code == WILDCARD_CODE || wbh.wildcard.mask ) {
		/* wildcard codes */
		return ComposeWildcardCodesToRetrieveCandidatesCore( code );
	}

	/* normal codes */
	return ComposeNormalCodesToRetrieveCandidatesCore( code );
}

static int ComposeCodesToRetrieveCandidates( coop_t coop, unsigned char code )
{
	switch( coop ) {
	case COOP_ADD:
		return ComposeCodesToRetrieveCandidatesCore( code );
		break;
	case COOP_REPLACE:
		if( wbh.level ) {
			wbh.level --;
			return ComposeCodesToRetrieveCandidatesCore( code );
		}
		break;
	case COOP_DEL:
		if( wbh.level ) {
			wbh.level --;

			/* clean codes */
			wbh.codes.all &= ~maskOneCode[ wbh.level ].all;
			wbh.wildcard.codes.all &= ~maskOneCode[ wbh.level ].all;
			wbh.wildcard.mask &= ~( 1 << wbh.level );

			if( wbh.wildcard.mask ) {
				/* wildcard case: recalculate candidates */
				FillCandidatesInformationRelyOnQueryResult( &wbh.wildcard.first[ -- wbh.level ].qresult );
				wbh.codes = wbh.wildcard.first[ wbh.level - 1 ].codes;
				return 1;
			}

			if( wbh.level ) 
				wbh.candidate.total = wbh.candidate.wo[ wbh.level - 1 ].end -
										wbh.candidate.wo[ wbh.level - 1 ].start;
			else
				wbh.candidate.total = 0;
			wbh.candidate.current = 0;
			wbh.candidate.previous = 0;
			wbh.candidate.b.all = 0;
			return 1;
		}
		break;
	}

	return 0;
}

static inline int GetRetrievedNormalCandidates( unsigned char *pCandidates, int nWant )
{
	const unsigned char *pWordSrc;
	int i;
	int backward = 0;
	unsigned short start;

	if( nWant == 0 )
		return 0;

	if( nWant > 0 ) {
		/* decide start and nWant by current position */
		if( wbh.candidate.current + nWant > wbh.candidate.total ) {
			nWant = wbh.candidate.total - wbh.candidate.current;
		}
		start = wbh.candidate.current;
	} else {
		backward = 1;
		/* move current to correct position */
		if( wbh.candidate.current == 0 && wbh.candidate.previous )
			wbh.candidate.current = wbh.candidate.total - wbh.candidate.previous;
		else
			wbh.candidate.current -= wbh.candidate.previous;

		/* decide start and nWant by current position */
		if( wbh.candidate.current == 0 ) {
			if( nWant== -8 )
				i = ( wbh.candidate.total & 7 );
			else
				i = ( wbh.candidate.total % ( -nWant ) );
			nWant = ( ( i == 0 ) ? -nWant : i );
			start = wbh.candidate.total - nWant;
		} else {
			if( wbh.candidate.current + nWant < 0 ) {
				nWant = wbh.candidate.current;
				start = 0;
			} else {
				start = wbh.candidate.current + nWant;
				nWant = -nWant;
			}
		}
	}

	pWordSrc = &wbhWordsList[ 2 * ( start + wbh.candidate.wo[ wbh.level - 1 ].start ) ];

	for( i = 0; i < nWant; i ++ ) {
		*pCandidates ++ = *pWordSrc ++;
		*pCandidates ++ = *pWordSrc ++;
	}
	
	/* update indicator */
	if( !backward ) {
		wbh.candidate.current += nWant;

		if( wbh.candidate.current >= wbh.candidate.total )
			wbh.candidate.current = 0;
	}

	wbh.candidate.previous = nWant; 
	wbh.candidate.b.forward = !backward;

	return nWant;
}

static int GetNextOrPreviousPermutationForCandidates( int forward )
{
	int carryCheck = 0;
	qresult_t qresult;

	while( 1 ) {	
		if( forward ) {
			/* next permutation */
			if( FindNextWildcardPermutation_STATIC( wbh.level, &wbh.codes, wbh.wildcard.codes ) == 0 ) {
				carryCheck ++;
			}
		} else {
			/* previous permutation */
			if( FindPrevWildcardPermutation_STATIC( wbh.level, &wbh.codes, wbh.wildcard.codes ) == 0 ) {
				carryCheck ++;
			}
		}

		if( QueryCandidatesOfCodes1to5_STATIC( wbh.codes, wbh.level, &qresult ) ) 
			break;	/* ok, find some candidates */

		if( carryCheck >= 2 ) {	/* prevent infinite loop */
			debug_out( "Can't find wildcard candidates\n" );
			break;
		}
	}

	/* Below function will increase wbh.level */
	wbh.level --;

	FillCandidatesInformationRelyOnQueryResult( &qresult );

	if( !forward )	/* backward */
		wbh.candidate.current = wbh.candidate.total;

	return carryCheck;
}

static inline int GetRetrievedWildcardCandidates( unsigned char *pCandidates, int nWant )
{
	const unsigned char *pWordSrc;
	unsigned char *pWordDst;
	int i;
	const int forward = ( nWant > 0 ? 1 : 0 );
	const int forward_previous = ( wbh.candidate.b.forward ? 1 : 0 );
	unsigned short start;

	int nRemain = ( nWant > 0 ? nWant : -nWant );
	int nCopied = 0, nCopy;
	unsigned short candidate_previous = wbh.candidate.previous;
	int carryCheck = 0;
	int loopTimes;
	int fixWant = 1;

	if( nWant == 0 )
		return 0;

	/* 
	 * In wildcard case, wbh.candidate.current is place on the LAST retrieved position, 
	 * so the positions are difference from forward and backward retrieve. 
	 * NOTE: WILDCARD CASE ONLY!! 
	 */

	/* If current and previous retrieve directions are the same, we can tetrieve right now. */
	if( ( forward ^ forward_previous ) == 0 || candidate_previous == 0 )
		goto label_start_to_retrieve_candidates;

	/* move wbh.candidate.current to correct position according to retrieve forward or backward. */
	while( 1 ) {
		if( forward ) {	/* forward */
			if( wbh.candidate.current + candidate_previous <= wbh.candidate.total ) {
				wbh.candidate.current += candidate_previous;
				break;
			} else
				candidate_previous -= ( wbh.candidate.total - wbh.candidate.current );
		} else {		/* backward */
			if( wbh.candidate.current >= candidate_previous ) {
				wbh.candidate.current -= candidate_previous;
				break;
			} else
				candidate_previous -= wbh.candidate.current;
		}

		carryCheck = GetNextOrPreviousPermutationForCandidates( forward );
	}

	if( carryCheck )
		debug_out( "Carry??\n" );

label_start_to_retrieve_candidates:
	/* ok. Now use wbh.candidate.current to retrieve candidates */
	for( loopTimes = 0; ; loopTimes ++ ) {
		if( forward ) {
			/* decide start and nCopy */
			if( wbh.candidate.current + nRemain > wbh.candidate.total ) {
				nCopy = wbh.candidate.total - wbh.candidate.current;
			} else
				nCopy = nRemain;
			start = wbh.candidate.current;
		} else {
			/* If backward carry present, modify nWant and nRemain */
			if( carryCheck && fixWant ) {
				if( nWant == -8 )
					i = ( wbh.wildcard.candidate.wo[ wbh.level - 1 ].total & 7 );
				else
					i = ( wbh.wildcard.candidate.wo[ wbh.level - 1 ].total % ( -nWant ) );

				if( i ) {
					nWant = -i;
					nRemain = i;
				}

				fixWant = 0;
			} 

			/* decide start and nCopy */
			if( wbh.candidate.current >= nRemain ) {
				start = wbh.candidate.current - nRemain;
				nCopy = nRemain;
			} else {
				start = 0;
				nCopy = wbh.candidate.current;
			}
		}

		/* Use start and nCopy to copy data */
		if( nCopy ) {
			pWordSrc = &wbhWordsList[ 2 * ( start + wbh.candidate.wo[ wbh.level - 1 ].start ) ];
			if( forward )
				pWordDst = &pCandidates[ 2 * nCopied ];
			else
				pWordDst = &pCandidates[ 2 * ( -nWant - nCopied - nCopy ) ];

			for( i = 0; i < nCopy; i ++ ) {
				*pWordDst ++ = *pWordSrc ++;
				*pWordDst ++ = *pWordSrc ++;
			}
			
			/* update indicator */
			if( forward )
				wbh.candidate.current += nCopy;
			else
				wbh.candidate.current -= nCopy;

			nRemain -= nCopy;
			nCopied += nCopy;
		}

		if( nRemain )
			;
		else
			goto label_copy_get_candidates_done;

		/* Try next or previous permutation */
		carryCheck = GetNextOrPreviousPermutationForCandidates( forward );

		/* If some candidates have been retrieved, and met the last codes permutation. */
		if( carryCheck && nCopied )		/* forward case */
			goto label_copy_get_candidates_done;

	}	/* loopTimes */

label_copy_get_candidates_done:
	wbh.candidate.previous = nCopied; 
	wbh.candidate.b.forward = forward;

	return nCopied;
}

static int GetRetrievedCandidates( unsigned char *pCandidates, int nWant )
{
	if( wbh.wildcard.mask )
		return GetRetrievedWildcardCandidates( pCandidates, nWant );

	return GetRetrievedNormalCandidates( pCandidates, nWant );
}

/* ============================================================================ */
/* IME WuBiHua UI Functions */

static int GetCyclicCodesForInputKey( unsigned char key );
static void WuBiHuaInitializeVariables( void );

static const unsigned char * const ppImagesForWuBiHuaCodes[] = {
	Image_ime_indicator_WuBiHua_root1, 
	Image_ime_indicator_WuBiHua_root2, 
	Image_ime_indicator_WuBiHua_root3, 
	Image_ime_indicator_WuBiHua_root4, 
	Image_ime_indicator_WuBiHua_root5, 
	Image_ime_indicator_WuBiHua_root6, 
};

static const cce_context_t wubihuaCceContext = {
	GetCyclicCodesForInputKey,
	ComposeCodesToRetrieveCandidates,
	GetRetrievedCandidates,
	WuBiHuaInitializeVariables,
	NULL,	/* Wide codes */
	ppImagesForWuBiHuaCodes,	/* Image for codes */
};

static void WuBiHuaInitializeVariables( void )
{
	wbh.level = 0;
	wbh.cyclic.key = 0;
	wbh.wildcard.codes.all = 0;
	wbh.wildcard.mask = 0;

	if( maskOneCode[ 0 ].all == 0 ) {
		maskOneCode[ 0 ].code1 = 0xFF;
		maskOneCode[ 1 ].code2 = 0xFF;
		maskOneCode[ 2 ].code3 = 0xFF;
		maskOneCode[ 3 ].code4 = 0xFF;
		maskOneCode[ 4 ].code5 = 0xFF;
	}
}

void WuBiHuaInitialization( void )
{
	WuBiHuaInitializeVariables();

	ActivateCCEditor( &wubihuaCceContext, &wbh.level, &wbh.cyclic, 
						CCE_FLAGS_KEY_1_TO_6 | CCE_FLAGS_NO_CYCLIC_CODES | CCE_FLAGS_DRAW_IMAGE );
}

void WuBiHuaTermination( void )
{
	DeactivateCCEditor();
}

static int GetCyclicCodesForInputKey( unsigned char key )
{
	unsigned char code;

	if( key < '1' || key > '6' )
		return 0;	/* not my key */

	/* reset possible cyclic info */
	wbh.cyclic.total = 0;
	wbh.cyclic.current = 0;

	code = key - '1' + 1;

	if( CheckIfCodesExistBeyondPreviousOne_CONST( code ) ) {
		wbh.cyclic.codes[ wbh.cyclic.total ++ ] = code;
	} 

	wbh.cyclic.key = 0;		/* no cyclic */
	wbh.cyclic.time = GetUptimeInMillisecond();

	return wbh.cyclic.total;
}

int WuBiHuaKeyProcessor( unsigned char key )
{
	/* Key is processed by KeyOwnCCEditor */
	return 0;
}

void WuBiHuaTimerProcessor( void )
{
}

#endif /* LANG_GB2312 */
