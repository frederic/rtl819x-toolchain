#include <stdio.h>
#include "ui_config.h"
#include "com_ime_def.h"
#include "res_ime_big5_phonetic.h"
#include "com_ime_phonetic.h"

#ifdef LANG_BIG5

#pragma pack(1)

typedef union codes2to3_s {
	struct {
#ifdef _TEST_MODE
		unsigned char code3:4;
		unsigned char code2:4;
#else
		unsigned char code2:4;
		unsigned char code3:4;
#endif
	};
	unsigned char all;
} codes2to3_t;

typedef union codes_s {
	struct {
		unsigned char code1;
		union {
			unsigned char code2to3;
			struct {
#ifdef _TEST_MODE
				unsigned char code3:4;
				unsigned char code2:4;
#else
				unsigned char code2:4;
				unsigned char code3:4;
#endif
			};
		};
	};
	unsigned short all;
} codes_t;

#pragma pack()

CT_ASSERT( sizeof( codes2to3_t ) == sizeof( ( ( codes2to3_t *)0 ) ->all ) );
CT_ASSERT( sizeof( codes_t ) == sizeof( ( ( codes_t *)0 ) ->all ) );

/* ============================================================================ */
/* IME phonetic Table Access Functions */
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

static const unsigned char PhoneticCodeLevel2[ ] = {
	'\x0', 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 35, 36, 37, 
	/*     £«  £¬  £­  £®  £¯  £°  £±  £²  £³  £´  £µ  £¶  £¸  £¹  £º */
};

static const unsigned char PhoneticCodeLevel3[ ] = {
	'\x0', 22, 23, 25, 26, 27, 28, 29, 30, 31, 32, 33, 
	/*     £«  £¬  £®  £¯  £°  £±  £²  £³  £´  £µ  £¶ */
};


#define KINDS_OF_CODE2			( sizeof( PhoneticCodeLevel2 ) / sizeof( PhoneticCodeLevel2[ 0 ] ) )
#define KINDS_OF_CODE3			( sizeof( PhoneticCodeLevel3 ) / sizeof( PhoneticCodeLevel3[ 0 ] ) )

CT_ASSERT( KINDS_OF_CODE2 == 16 );
CT_ASSERT( KINDS_OF_CODE3 == 12 );

#define INVALID_CODES			0xFFFF

#define PHONETIC_CODE_START		1	/* £t */
#define PHONETIC_CODE_END		37	/* £º */

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
		} wo[ 3 ];
		unsigned short total;		/* total candidates */
		unsigned short current;		/* between start and end */
		unsigned short previous;	/* number of previous retrieved candidiates, so that they are shown in screen */
	} candidate;
	struct {
		struct {
			iwo_t start;
			iwo_t end;
		} iwo[ 3 ];			
	} offset;
	codes_t codes;
	unsigned int level;		/* now, input how many codes */

	/* compose codes for UI functions */
	codes_cyclic_t cyclic;
} ph;


static isi_t isiFromCode1_STATIC( unsigned char code1 )
{
	isi_t index;

	if( code1 < PHONETIC_CODE_START || code1 > PHONETIC_CODE_END )
		return INVALID_CODES;

	index = code1 - PHONETIC_CODE_START;

	return index;
}

static iwo_t iwoFromIsi_STATIC( isi_t isi )
{
	if( isi == 0 )
		return 0;

	isi --;

	if( isi >= PHONETIC_SUPRA_INDEX_BOUNDARY )
		return ( iwo_t )phoneticSupraIndex[ isi ] | 0x0100;

	return ( iwo_t )phoneticSupraIndex[ isi ];
}

static wo_t woFromIwo_STATIC( iwo_t iwo )
{
	int i;
	int startbyte;
	wo_t wo;

	if( iwo == 0 )
		return 0;

	iwo --;

	for( i = 0; i < PHONETIC_WORD_OFFSET_BOUNDARY_NUM; i ++ )
		if( iwo < phoneticWordOffsetBoundary[ i ] )
			break;

	startbyte = iwo / 2 * 3;

	if( ( iwo & 1 ) == 0 ) {
		wo = ( wo_t )phoneticWordOffset[ startbyte ] | 
						( ( ( wo_t )phoneticWordOffset[ startbyte + 1 ] << 8 ) & 0x0F00 );
	} else {
		wo = ( wo_t )phoneticWordOffset[ startbyte + 2 ] | 
						( ( ( wo_t )phoneticWordOffset[ startbyte + 1 ] << 4 ) & 0x0F00 );
	}

	wo |= ( i << 12 );

	return wo;
}

static codes2to3_t codes2to3FromCharCode_STATIC( unsigned char code, unsigned int level )
{
	/*
	 * NOTE: Pay attention to this function, because it can NOT discriminate between 
	 * 'not found' and '\x0' code. 
	 * The reason is that both of them are 0. 
	 */
	codes2to3_t codes2to3;
	const unsigned char * pcode;
	unsigned int kinds;
	unsigned int i;

	codes2to3.all = 0;

	switch( level ) {
	case 2:
		pcode = PhoneticCodeLevel2;
		kinds = KINDS_OF_CODE2;
		break;
	case 3:
		pcode = PhoneticCodeLevel3;
		kinds = KINDS_OF_CODE3;
		break;
	default:
		debug_out( "codes2to3 process level between 2 and 3 only.\n" );
		return codes2to3;	/* return 0 */
	}

	for( i = 0; i < kinds; i ++ ) {
		if( pcode[ i ] == code )
			goto label_code_is_found;
	}

	return codes2to3;	/* return 0 */

label_code_is_found:

	switch( level ) {
	case 2:
		codes2to3.code2 = i;
		break;
	case 3:
		codes2to3.code3 = i;
		break;
	}

	return codes2to3;
}

static int QueryCandidatesOfCodes1_STATIC( unsigned char code1, qresult_t *pqresult )
{
	isi_t isiStart, isiEnd;
	iwo_t iwoStart, iwoEnd;

	isiStart = isiFromCode1_STATIC( code1 );
	isiEnd = isiStart + 1;

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

static int QueryCandidatesOfCodes2to3_STATIC( iwo_t iwoStart, iwo_t iwoEnd, codes2to3_t codes2to3, 
											  unsigned int level, qresult_t *pqresult  )
{
	iwo_t i;
	codes2to3_t mask;
	int hit;

	mask.all = 0;

	switch( level ) {
	case 2:
		mask.code2 = 0xFF;
		break;
	case 3:
		mask.code3 = 0xFF;
		break;
	default:
		debug_out( "Bad codes level\n" );
		return 0;
	}

	/* search for range */
	hit = 0;

	for( i = iwoStart; i < iwoEnd; i ++ ) {
		if( ( ( ( codes2to3_t * )&phoneticCodes2to3[ i ] ) ->all & mask.all ) ==
			( codes2to3.all & mask.all ) )
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

static int CheckIfCodesExistBeyondPreviousOne_CONST( unsigned char code )
{
	unsigned int newLevel;		/* current level of this code */
	unsigned int idxLevel;
	codes2to3_t codes2to3, codes2to3_mask;
	
	newLevel = ph.level + 1;
	codes2to3_mask.all = 0;

	switch( newLevel ) {
	case 1:
		return QueryCandidatesOfCodes1_STATIC( code, NULL );

	case 3:
		codes2to3_mask.code2 = 0xFF;
	case 2:
		codes2to3 = codes2to3FromCharCode_STATIC( code, newLevel );

		if( codes2to3.all == 0 )
			return 0;

		idxLevel = ph.level - 1;
		codes2to3.all |= ( ( ( codes2to3_t * )&ph.codes.code2to3 ) ->all & codes2to3_mask.all );

		return QueryCandidatesOfCodes2to3_STATIC( ph.offset.iwo[ idxLevel ].start, 
											 ph.offset.iwo[ idxLevel ].end, 
											 codes2to3,
											 newLevel,
											 NULL );
	}

	return 0;
}

static int ComposeCodesToRetrieveCandidatesCore( unsigned char code )
{
	unsigned int newLevel;		/* current level of this code */
	unsigned int idxLevel;
	qresult_t qresult;
	codes2to3_t codes2to3, codes2to3_mask;
	wo_t wostart, woend;

	newLevel = ph.level + 1;
	codes2to3_mask.all = 0;

	switch( newLevel ) {
	case 1:
		if( QueryCandidatesOfCodes1_STATIC( code, &qresult ) ) {
			ph.codes.code1 = code;
			
			goto label_process_iwo;
		}
		break;

	case 3:
		codes2to3_mask.code2 = 0xFF;
	case 2:
		codes2to3 = codes2to3FromCharCode_STATIC( code, newLevel );

		if( codes2to3.all == 0 ) {
			debug_out( "Compose a non-existed code?\n" );
			return 0;
		}

		idxLevel = ph.level - 1;
		codes2to3.all |= ( ( ( codes2to3_t * )&ph.codes.code2to3 ) ->all & codes2to3_mask.all );

		if( QueryCandidatesOfCodes2to3_STATIC( ph.offset.iwo[ idxLevel ].start, 
											   ph.offset.iwo[ idxLevel ].end, 
											   codes2to3,
											   newLevel,
											   &qresult ) )
		{
			ph.codes.code2to3 = *( ( unsigned char * )&codes2to3 );

			goto label_process_iwo;
		}
		break;

	}

	debug_out( "Compose code error\n" );
	return 0;

label_process_iwo:
	ph.offset.iwo[ ph.level ].start = qresult.iwop.start;
	ph.offset.iwo[ ph.level ].end = qresult.iwop.end;

	wostart = woFromIwo_STATIC( qresult.iwop.start );
	woend = woFromIwo_STATIC( qresult.iwop.end );	/* woFromIwo_STATIC retrieve begining of offset */

//label_process_wo:
	ph.candidate.wo[ ph.level ].start = wostart;
	ph.candidate.wo[ ph.level ].end = woend;

	ph.candidate.total = woend - wostart;
	ph.candidate.current = 0;
	ph.candidate.previous = 0;

	ph.level ++;

	return 1;
}

static int ComposeCodesToRetrieveCandidates( coop_t coop, unsigned char code )
{
	switch( coop ) {
	case COOP_ADD:
		return ComposeCodesToRetrieveCandidatesCore( code );
		break;
	case COOP_REPLACE:
		if( ph.level ) {
			ph.level --;
			return ComposeCodesToRetrieveCandidatesCore( code );
		}
		break;
	case COOP_DEL:
		if( ph.level ) {
			ph.level --;

			if( ph.level )
				ph.candidate.total = ph.candidate.wo[ ph.level - 1 ].end - 
											ph.candidate.wo[ ph.level - 1 ].start;
			else
				ph.candidate.total = 0;
			ph.candidate.current = 0;
			ph.candidate.previous = 0;
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
		if( ph.candidate.current + nWant > ph.candidate.total ) {
			nWant = ph.candidate.total - ph.candidate.current;
		}
		start = ph.candidate.current;
	} else {
		backward = 1;
		/* move current to correct position */
		if( ph.candidate.current == 0 && ph.candidate.previous )
			ph.candidate.current = ph.candidate.total - ph.candidate.previous;
		else
			ph.candidate.current -= ph.candidate.previous;

		/* decide start and nWant by current position */
		if( ph.candidate.current == 0 ) {
			if( nWant== -8 )
				i = ( ph.candidate.total & 7 );
			else
				i = ( ph.candidate.total % ( -nWant ) );
			nWant = ( ( i == 0 ) ? -nWant : i );
			start = ph.candidate.total - nWant;
		} else {
			if( ph.candidate.current + nWant < 0 ) {
				nWant = ph.candidate.current;
				start = 0;
			} else {
				start = ph.candidate.current + nWant;
				nWant = -nWant;
			}
		}
	}

	pWordSrc = &phoneticWordsList[ 2 * ( start + ph.candidate.wo[ ph.level - 1 ].start ) ];

	for( i = 0; i < nWant; i ++ ) {
		*pCandidates ++ = *pWordSrc ++;
		*pCandidates ++ = *pWordSrc ++;
	}
	
	/* update indicator */
	if( !backward ) {
		ph.candidate.current += nWant;

		if( ph.candidate.current >= ph.candidate.total )
			ph.candidate.current = 0;
	}

	ph.candidate.previous = nWant; 

	return nWant;
}

/* ============================================================================ */
/* IME phonetic UI Functions */
static const unsigned char cyclic_codes_for_key1[] = {
	1, 2, 3, 4,
/*  £t £u £v £w */
};

static const unsigned char cyclic_codes_for_key2[] = {
	5, 6, 7, 8,
/*  £x £y £z £{ */
};
static const unsigned char cyclic_codes_for_key3[] = {
	9, 10, 11,
/*  £| £}  £~ */
};
static const unsigned char cyclic_codes_for_key4[] = {
	12, 13, 14,
/*  £¡  £¢  ££ */
};
static const unsigned char cyclic_codes_for_key5[] = {
	15, 16, 17, 18,
/*  £¤  £¥  £¦  £§ */
};
static const unsigned char cyclic_codes_for_key6[] = {
	19, 20, 21,
/*  £¨  £©  £ª */
};
static const unsigned char cyclic_codes_for_key7[] = {
	22, 23, 24, 25, 
/*  £«  £¬  £­  £® */
};
static const unsigned char cyclic_codes_for_key8[] = {
	26, 27, 28, 29, 
/*  £¯  £°  £±  £² */
};
static const unsigned char cyclic_codes_for_key9[] = {
	30, 31, 32, 33, 34, 
/*  £³  £´  £µ  £¶  £· */
};

static const unsigned char cyclic_codes_for_key0[] = {
	35, 36, 37,
/*  £¸  £¹  £º */
};

#define M_CYCLIC_KEYS_SET( k )		{ k, sizeof( k ) / sizeof( k[ 0 ] ), }

static const cyclic_keys_set_t phoneticCyclicCodesSet[] = {
	M_CYCLIC_KEYS_SET( cyclic_codes_for_key0 ),
	M_CYCLIC_KEYS_SET( cyclic_codes_for_key1 ),
	M_CYCLIC_KEYS_SET( cyclic_codes_for_key2 ),
	M_CYCLIC_KEYS_SET( cyclic_codes_for_key3 ),
	M_CYCLIC_KEYS_SET( cyclic_codes_for_key4 ),
	M_CYCLIC_KEYS_SET( cyclic_codes_for_key5 ),
	M_CYCLIC_KEYS_SET( cyclic_codes_for_key6 ),
	M_CYCLIC_KEYS_SET( cyclic_codes_for_key7 ),
	M_CYCLIC_KEYS_SET( cyclic_codes_for_key8 ),
	M_CYCLIC_KEYS_SET( cyclic_codes_for_key9 ),
};

static const unsigned char phoneticWideCodes[] = { 
	"£t£u£v£w£x£y£z£{£|£}£~£¡£¢£££¤£¥£¦£§£¨£©£ª£«£¬£­£®£¯£°£±£²£³£´£µ£¶£·£¸£¹£º" 
};

static int GetCyclicCodesForInputKey( unsigned char key );
static void PhoneticInitializeVariables( void );

static const cce_context_t phoneticCceContext = {
	GetCyclicCodesForInputKey,
	ComposeCodesToRetrieveCandidates,
	GetRetrievedCandidates,
	PhoneticInitializeVariables,
	phoneticWideCodes,	/* Wide codes */
	NULL,	/* Images for codes */
};

static void PhoneticInitializeVariables( void )
{
	ph.level = 0;
	ph.cyclic.key = 0;
}

void PhoneticInitialization( void )
{
	PhoneticInitializeVariables();

	ActivateCCEditor( &phoneticCceContext, &ph.level, &ph.cyclic, 
						CCE_FLAGS_KEY_0_TO_9 | CCE_FLAGS_DRAW_WIDE );
}

void PhoneticTermination( void )
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

	if( key < '0' || key > '9' )
		return 0;	/* not my key */

	/* Get cyclic info */
	pCyclicSet = &phoneticCyclicCodesSet[ key - '0' ];
	pCodes = pCyclicSet ->pCyclicKeys;
	nCodesNum = pCyclicSet ->nNumberOfKeys;

	/* reset possible cyclic info */
	ph.cyclic.total = 0;
	ph.cyclic.current = 0;

	for( i = 0; i < nCodesNum; i ++ ) {
		code = *( pCodes + i );
		if( CheckIfCodesExistBeyondPreviousOne_CONST( code ) ) {
			ph.cyclic.codes[ ph.cyclic.total ++ ] = code;
		} 
	}

	if( ph.cyclic.total ) {
		ph.cyclic.key = key;
		ph.cyclic.time = GetUptimeInMillisecond();
	} else
		ph.cyclic.key = 0;

	return ph.cyclic.total;
}

int PhoneticKeyProcessor( unsigned char key )
{
	/* Key is processed by KeyOwnCCEditor */
	return 0;
}

void PhoneticTimerProcessor( void )
{
}

#endif /* LANG_BIG5 */
