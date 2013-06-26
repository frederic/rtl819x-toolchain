#include <stdio.h>
#include "ui_config.h"
#include "com_ime_def.h"
#ifdef LANG_BIG5
#include "res_ime_big5_pinyin.h"
#elif defined( LANG_GB2312 )
#include "res_ime_gb2312_pinyin.h"
#endif
#include "com_ime_pinyin.h"
#include "gs_drawtext.h"
#include "ui_vkey.h"

#if defined( LANG_BIG5 ) || defined( LANG_GB2312 )

#pragma pack(1)

typedef union codes3to5_s {
	struct {
#ifdef _TEST_MODE
		unsigned char code3:3;
		unsigned char code4:3;
		unsigned char code5:2;
#else
		unsigned char code5:2;
		unsigned char code4:3;
		unsigned char code3:3;
#endif
	};
	unsigned char all;
} codes3to5_t;

typedef union codes_s {
	struct {
		union {
			unsigned short code1to2;
			struct {
				unsigned char code1;
				unsigned char code2;
			};
		};
		union {
			unsigned char code3to5;
			struct {
#ifdef _TEST_MODE
				unsigned char code3:3;
				unsigned char code4:3;
				unsigned char code5:2;
#else
				unsigned char code5:2;
				unsigned char code4:3;
				unsigned char code3:3;
#endif
			};
		};
		unsigned char code6;
	};
	unsigned long all;
} codes_t;

#pragma pack()

CT_ASSERT( sizeof( codes3to5_t ) == sizeof( ( ( codes3to5_t *)0 ) ->all ) );
CT_ASSERT( sizeof( codes_t ) == sizeof( ( ( codes_t *)0 ) ->all ) );

/* ============================================================================ */
/* IME PinYin Table Access Functions */
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

static const unsigned char PinYinCodeLevel2[ ] = {
#ifdef LANG_BIG5
	'\x0', 'a', 'e', 'g', 'h', 'i', 'n', 'o', 'r', 'u', 
#elif defined( LANG_GB2312 )
	'\x0', 'a', 'e', 'h', 'i', 'n', 'o', 'r', 'u', 'v',
#endif
};

static const unsigned char PinYinCodeLevel3[ ] = {
	'\x0', 'a', 'e', 'g', 'i', 'n', 'o', 'u',
};

static const unsigned char PinYinCodeLevel4[ ] = {
	'\x0', 'a', 'g', 'i', 'n', 'o', 'u', 
};

static const unsigned char PinYinCodeLevel5[ ] = {
	'\x0', 'g', 'i', 'n', 
};

static const unsigned char PinYinCodeLevel6[ ] = {
	'\x0', 'g', 
};

#define KINDS_OF_CODE2			( sizeof( PinYinCodeLevel2 ) / sizeof( PinYinCodeLevel2[ 0 ] ) )
#define KINDS_OF_CODE3			( sizeof( PinYinCodeLevel3 ) / sizeof( PinYinCodeLevel3[ 0 ] ) )
#define KINDS_OF_CODE4			( sizeof( PinYinCodeLevel4 ) / sizeof( PinYinCodeLevel4[ 0 ] ) )
#define KINDS_OF_CODE5			( sizeof( PinYinCodeLevel5 ) / sizeof( PinYinCodeLevel5[ 0 ] ) )
#define KINDS_OF_CODE6			( sizeof( PinYinCodeLevel6 ) / sizeof( PinYinCodeLevel6[ 0 ] ) )

CT_ASSERT( KINDS_OF_CODE2 == 10 );
CT_ASSERT( KINDS_OF_CODE3 == 8 );
CT_ASSERT( KINDS_OF_CODE4 == 7 );
CT_ASSERT( KINDS_OF_CODE5 == 4 );
CT_ASSERT( KINDS_OF_CODE6 == 2 );

#define INVALID_CODES			0xFFFF

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
		} wo[ 6 ];
		unsigned short total;		/* total candidates */
		unsigned short current;		/* between start and end */
		unsigned short previous;	/* number of previous retrieved candidiates, so that they are shown in screen */
	} candidate;
	struct {
		struct {
			iwo_t start;
			iwo_t end;
		} iwo[ 5 ];			/* don't need level 6! */
	} offset;
	codes_t codes;
	unsigned int level;		/* now, input how many codes */

	/* compose codes for UI functions */
	codes_cyclic_t cyclic;
} py;

static isi_t isiFromCode1to2_STATIC( unsigned char code1, unsigned char code2 )
{
	int i;
	isi_t index;

	// code 1
	if( code1 < 'a' || code1 > 'z' )
		return INVALID_CODES;

	index = ( code1 - 'a' ) * KINDS_OF_CODE2;

	// code 2
	for( i = 0; i < KINDS_OF_CODE2; i ++ )
		if( code2 == PinYinCodeLevel2[ i ] )
			goto label_continue_to_get_index;

	return INVALID_CODES;

label_continue_to_get_index:

	return index + i;
}

static iwo_t iwoFromIsi_STATIC( isi_t isi )
{
	if( isi == 0 )
		return 0;

	isi --;

	if( isi >= PY_SUPRA_INDEX_BOUNDARY )
		return ( iwo_t )pySupraIndex[ isi ] | 0x0100;

	return ( iwo_t )pySupraIndex[ isi ];
}

static wo_t woFromIwo_STATIC( iwo_t iwo )
{
	int i;
	int startbyte;
	wo_t wo;

	if( iwo == 0 )
		return 0;

	iwo --;

	for( i = 0; i < PY_WORD_OFFSET_BOUNDARY_NUM; i ++ )
		if( iwo < pyWordOffsetBoundary[ i ] )
			break;

	startbyte = iwo / 2 * 3;

	if( ( iwo & 1 ) == 0 ) {
		wo = ( wo_t )pyWordOffset[ startbyte ] | 
						( ( ( wo_t )pyWordOffset[ startbyte + 1 ] << 8 ) & 0x0F00 );
	} else {
		wo = ( wo_t )pyWordOffset[ startbyte + 2 ] | 
						( ( ( wo_t )pyWordOffset[ startbyte + 1 ] << 4 ) & 0x0F00 );
	}

	wo |= ( i << 12 );

	return wo;
}

static codes3to5_t codes3to5FromCharCode_STATIC( unsigned char code, unsigned int level )
{
	/*
	 * NOTE: Pay attention to this function, because it can NOT discriminate between 
	 * 'not found' and '\x0' code. 
	 * The reason is that both of them are 0. 
	 */
	codes3to5_t codes3to5;
	const unsigned char * pcode;
	unsigned int kinds;
	unsigned int i;

	codes3to5.all = 0;

	switch( level ) {
	case 3:
		pcode = PinYinCodeLevel3;
		kinds = KINDS_OF_CODE3;
		break;
	case 4:
		pcode = PinYinCodeLevel4;
		kinds = KINDS_OF_CODE4;
		break;
	case 5:
		pcode = PinYinCodeLevel5;
		kinds = KINDS_OF_CODE5;
		break;
	default:
		debug_out( "codes3to5 process level between 3 and 5 only.\n" );
		return codes3to5;	/* return 0 */
	}

	for( i = 0; i < kinds; i ++ ) {
		if( pcode[ i ] == code )
			goto label_code_is_found;
	}

	return codes3to5;	/* return 0 */

label_code_is_found:

	switch( level ) {
	case 3:
		codes3to5.code3 = i;
		break;
	case 4:
		codes3to5.code4 = i;
		break;
	case 5:
		codes3to5.code5 = i;
		break;
	}

	return codes3to5;
}

static int QueryCandidatesOfCodes1to2_STATIC( unsigned char code1, unsigned char code2, 
											  unsigned int level, qresult_t *pqresult )
{
	isi_t isiStart, isiEnd;
	iwo_t iwoStart, iwoEnd;

	if( level == 1 ) {
		isiStart = isiFromCode1to2_STATIC( code1, code2 );
		isiEnd = isiStart + KINDS_OF_CODE2;
	} else {
		isiStart = isiFromCode1to2_STATIC( code1, code2 );
		isiEnd = isiStart + 1;
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

static int QueryCandidatesOfCodes3to5_STATIC( iwo_t iwoStart, iwo_t iwoEnd, codes3to5_t codes3to5, 
											  unsigned int level, qresult_t *pqresult  )
{
	iwo_t i;
	codes3to5_t mask;
	int hit;

	mask.all = 0;

	switch( level ) {
	case 3:
		mask.code3 = 0xFF;
		break;
	case 4:
		mask.code3 = mask.code4 = 0xFF;
		break;
	case 5:
		mask.code3 = mask.code4 = mask.code5 = 0xFF;
		break;
	default:
		debug_out( "Bad codes level\n" );
		return 0;
	}

	/* search for range */
	hit = 0;

	for( i = iwoStart; i < iwoEnd; i ++ ) {
		if( ( ( ( codes3to5_t * )&pyCodes3to5[ i ] ) ->all & mask.all ) ==
			( codes3to5.all & mask.all ) )
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

static int QueryCandidatesOfCodes6_STATIC( codes_t codes, unsigned char code6, 
										   qresult_t *pqresult )
{
	int i;

	for( i = 0; i < PY_6CODES_ENTRIES; i ++ ) {
		if( codes.code1 == py6CodesEntries[ i ].code1 &&
			codes.code2 == py6CodesEntries[ i ].code2 &&
			codes.code3to5 == py6CodesEntries[ i ].code3to5 &&
			code6 == py6CodesEntries[ i ].code6 )
		{
			if( pqresult ) {
				pqresult ->wop.start = py6CodesEntries[ i ].woStart;
				pqresult ->wop.end = py6CodesEntries[ i ].woEnd;
			}

			return 1;
		}
	}

	return 0;
}

static int CheckIfCodesExistBeyondPreviousOne_CONST( unsigned char code )
{
	unsigned int newLevel;		/* current level of this code */
	unsigned int idxLevel;
	codes3to5_t codes3to5, codes3to5_mask;
	
	newLevel = py.level + 1;
	codes3to5_mask.all = 0;

	switch( newLevel ) {
	case 1:
		return QueryCandidatesOfCodes1to2_STATIC( code, '\x0', newLevel, NULL );
	case 2:
		return QueryCandidatesOfCodes1to2_STATIC( py.codes.code1, code, newLevel, NULL );

	case 5:
		codes3to5_mask.code4 = 0xFF;
	case 4:
		codes3to5_mask.code3 = 0xFF;
	case 3:
		codes3to5 = codes3to5FromCharCode_STATIC( code, newLevel );

		if( codes3to5.all == 0 )
			return 0;

		idxLevel = py.level - 1;
		codes3to5.all |= ( ( ( codes3to5_t * )&py.codes.code3to5 ) ->all & codes3to5_mask.all );

		return QueryCandidatesOfCodes3to5_STATIC( py.offset.iwo[ idxLevel ].start, 
											 py.offset.iwo[ idxLevel ].end, 
											 codes3to5,
											 newLevel,
											 NULL );
	case 6:
		return QueryCandidatesOfCodes6_STATIC( py.codes, code, NULL );
	}

	return 0;
}

static int ComposeCodesToRetrieveCandidatesCore( unsigned char code )
{
	unsigned int newLevel;		/* current level of this code */
	unsigned int idxLevel;
	qresult_t qresult;
	codes3to5_t codes3to5, codes3to5_mask;
	wo_t wostart, woend;

	newLevel = py.level + 1;
	codes3to5_mask.all = 0;

	switch( newLevel ) {
	case 1:
		if( QueryCandidatesOfCodes1to2_STATIC( code, '\x0', newLevel, &qresult ) ) {
			py.codes.code1 = code;
			
			goto label_process_iwo;
		}
		break;

	case 2:
		if( QueryCandidatesOfCodes1to2_STATIC( py.codes.code1, code, newLevel, &qresult ) ) {
			py.codes.code2 = code;

			goto label_process_iwo;
		}
		break;

	case 5:
		codes3to5_mask.code4 = 0xFF;
	case 4:
		codes3to5_mask.code3 = 0xFF;
	case 3:
		codes3to5 = codes3to5FromCharCode_STATIC( code, newLevel );

		if( codes3to5.all == 0 ) {
			debug_out( "Compose a non-existed code?\n" );
			return 0;
		}

		idxLevel = py.level - 1;
		codes3to5.all |= ( ( ( codes3to5_t * )&py.codes.code3to5 ) ->all & codes3to5_mask.all );

		if( QueryCandidatesOfCodes3to5_STATIC( py.offset.iwo[ idxLevel ].start, 
											   py.offset.iwo[ idxLevel ].end, 
											   codes3to5,
											   newLevel,
											   &qresult ) )
		{
			py.codes.code3to5 = *( ( unsigned char * )&codes3to5 );

			goto label_process_iwo;
		}
		break;

	case 6:
		if( QueryCandidatesOfCodes6_STATIC( py.codes, code, &qresult ) ) {
			py.codes.code6 = code;

			/*
			 * NOTE:
			 *   py.codes.offset.iwo[ 5 ].start 
			 *   py.codes.offset.iwo[ 5 ].end
			 * are not update!!
			*/

			wostart = qresult.wop.start;
			woend = qresult.wop.end;

			goto label_process_wo;
		}
		break;
	}

	debug_out( "Compose code error\n" );
	return 0;

label_process_iwo:
	py.offset.iwo[ py.level ].start = qresult.iwop.start;
	py.offset.iwo[ py.level ].end = qresult.iwop.end;

	wostart = woFromIwo_STATIC( qresult.iwop.start );
	woend = woFromIwo_STATIC( qresult.iwop.end );	/* woFromIwo_STATIC retrieve begining of offset */

label_process_wo:
	py.candidate.wo[ py.level ].start = wostart;
	py.candidate.wo[ py.level ].end = woend;

	py.candidate.total = woend - wostart;
	py.candidate.current = 0;
	py.candidate.previous = 0;

	py.level ++;

	return 1;
}

static int ComposeCodesToRetrieveCandidates( coop_t coop, unsigned char code )
{
	switch( coop ) {
	case COOP_ADD:
		return ComposeCodesToRetrieveCandidatesCore( code );
		break;
	case COOP_REPLACE:
		if( py.level ) {
			py.level --;
			return ComposeCodesToRetrieveCandidatesCore( code );
		}
		break;
	case COOP_DEL:
		if( py.level ) {
			py.level --;

			if( py.level )
				py.candidate.total = py.candidate.wo[ py.level - 1 ].end - 
											py.candidate.wo[ py.level - 1 ].start;
			else
				py.candidate.total = 0;
			py.candidate.current = 0;
			py.candidate.previous = 0;
			return 1;
		}
		break;
	}

	return 0;
}

static int GetRetrievedCandidates( unsigned char *pCandidates, int nWant )
{
	const unsigned char *pWordSrc;
	int i;
	int backward = 0;
	unsigned short start;

	if( nWant == 0 )
		return 0;

	if( nWant > 0 ) {
		/* decide start and nWant by current position */
		if( py.candidate.current + nWant > py.candidate.total ) {
			nWant = py.candidate.total - py.candidate.current;
		}
		start = py.candidate.current;
	} else {
		backward = 1;
		/* move current to correct position */
		if( py.candidate.current == 0 && py.candidate.previous )
			py.candidate.current = py.candidate.total - py.candidate.previous;
		else
			py.candidate.current -= py.candidate.previous;

		/* decide start and nWant by current position */
		if( py.candidate.current == 0 ) {
			if( nWant== -8 )
				i = ( py.candidate.total & 7 );
			else
				i = ( py.candidate.total % ( -nWant ) );
			nWant = ( ( i == 0 ) ? -nWant : i );
			start = py.candidate.total - nWant;
		} else {
			if( py.candidate.current + nWant < 0 ) {
				nWant = py.candidate.current;
				start = 0;
			} else {
				start = py.candidate.current + nWant;
				nWant = -nWant;
			}
		}
	}

	pWordSrc = &pyWordsList[ 2 * ( start + py.candidate.wo[ py.level - 1 ].start ) ];

	for( i = 0; i < nWant; i ++ ) {
		*pCandidates ++ = *pWordSrc ++;
		*pCandidates ++ = *pWordSrc ++;
	}
	
	/* update indicator */
	if( !backward ) {
		py.candidate.current += nWant;

		if( py.candidate.current >= py.candidate.total )
			py.candidate.current = 0;
	}

	py.candidate.previous = nWant; 

	return nWant;
}

/* ============================================================================ */
/* IME PinYin UI Functions */

static const unsigned char cyclic_codes_for_key2[] = {
	'a', 'b', 'c', 
};
static const unsigned char cyclic_codes_for_key3[] = {
	'd', 'e', 'f', 
};
static const unsigned char cyclic_codes_for_key4[] = {
	'g', 'h', 'i',
};
static const unsigned char cyclic_codes_for_key5[] = {
	'j', 'k', 'l',
};
static const unsigned char cyclic_codes_for_key6[] = {
	'm', 'n', 'o',
};
static const unsigned char cyclic_codes_for_key7[] = {
	'p', 'q', 'r', 's',
};
static const unsigned char cyclic_codes_for_key8[] = {
	't', 'u', 
#ifdef LANG_GB2312
	'v', 	/* PinYin of Traditional Chinese doesn't use 'v' */
#endif
};
static const unsigned char cyclic_codes_for_key9[] = {
	'w', 'x', 'y', 'z',
};

#define M_CYCLIC_KEYS_SET( k )		{ k, sizeof( k ) / sizeof( k[ 0 ] ), }

static const cyclic_keys_set_t pinyinCyclicCodesSet[] = {
	M_CYCLIC_KEYS_SET( cyclic_codes_for_key2 ),
	M_CYCLIC_KEYS_SET( cyclic_codes_for_key3 ),
	M_CYCLIC_KEYS_SET( cyclic_codes_for_key4 ),
	M_CYCLIC_KEYS_SET( cyclic_codes_for_key5 ),
	M_CYCLIC_KEYS_SET( cyclic_codes_for_key6 ),
	M_CYCLIC_KEYS_SET( cyclic_codes_for_key7 ),
	M_CYCLIC_KEYS_SET( cyclic_codes_for_key8 ),
	M_CYCLIC_KEYS_SET( cyclic_codes_for_key9 ),
};

static int GetCyclicCodesForInputKey( unsigned char key );
static void PinYinInitializeVariables( void );

static const cce_context_t pinyinCceContext = {
	GetCyclicCodesForInputKey,
	ComposeCodesToRetrieveCandidates,
	GetRetrievedCandidates,
	PinYinInitializeVariables,
	NULL,	/* Wide codes */
	NULL,	/* Images for codes */
};

static void PinYinInitializeVariables( void )
{
	py.level = 0;
	py.cyclic.key = 0;
}

void PinYinInitialization( void )
{
	PinYinInitializeVariables();

	ActivateCCEditor( &pinyinCceContext, &py.level, &py.cyclic, 
						CCE_FLAGS_KEY_2_TO_9 );
}

void PinYinTermination( void )
{
	DeactivateCCEditor();
}

static int GetCyclicCodesForInputKey( unsigned char key )
{
	const cyclic_keys_set_t *pCyclicSet;
	const unsigned char *pCodes;
	int nCodesNum;
	int i;
	unsigned char code;

	if( key < '2' || key > '9' )
		return 0;	/* not my key */

	/* Get cyclic info */
	pCyclicSet = &pinyinCyclicCodesSet[ key - '2' ];
	pCodes = pCyclicSet ->pCyclicKeys;
	nCodesNum = pCyclicSet ->nNumberOfKeys;

	/* reset possible cyclic info */
	py.cyclic.total = 0;
	py.cyclic.current = 0;

	for( i = 0; i < nCodesNum; i ++ ) {
		code = *( pCodes + i );
		if( CheckIfCodesExistBeyondPreviousOne_CONST( code ) ) {
			py.cyclic.codes[ py.cyclic.total ++ ] = code;
		} 
	}

	if( py.cyclic.total ) {
		py.cyclic.key = key;
		py.cyclic.time = GetUptimeInMillisecond();
	} else
		py.cyclic.key = 0;

	return py.cyclic.total;
}

int PinYinKeyProcessor( unsigned char key )
{
	/* Key is processed by KeyOwnCCEditor */
	return 0;
}

void PinYinTimerProcessor( void )
{
}

#endif /* LANG_BIG5 || LANG_GB2312 */
