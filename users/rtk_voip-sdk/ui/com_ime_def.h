#ifndef __COM_IME_DEF_H__
#define __COM_IME_DEF_H__

#include "com_edit.h"
#include "ioctl_softrtc.h"

/* ============================================================================ */
/* English IME */
#define ON_SPOT_EDITING_ACCEPT_TIME		2000	/* 2 seconds */

/* structure for cyclic keys */
typedef struct cyclic_keys_set_s {
	const unsigned char *pCyclicKeys;
	int nNumberOfKeys;
} cyclic_keys_set_t;

/*
 * Input Method Editor Variables
 */
typedef struct english_ime_s {
	unsigned char				preStrokeKey;
	editor_cmd_t				preEditorCmd;
	uptime_t					preStrokeKeyTime;
	const cyclic_keys_set_t *	pCyclicKeysSet;
	int							nthKeyInCyclic;
} english_ime_t;

extern english_ime_t * const pEnglishImeVars;

/* ============================================================================ */
/* Chinese IME */

#define MAX_CYCLIC_CODES_NUM	5	/* maximum possible codes of a key is 5 */
#define CYCLIC_CODES_TIME		1000	/* accept cyclic codes in period of 1 second */

/* structure for complex IME, which use codes to compose word */
typedef struct codes_cyclic_s {
	unsigned char key;
	uptime_t time;
	unsigned char codes[ MAX_CYCLIC_CODES_NUM ];	/* possible codes for a certain key */
	int current;
	int total;
} codes_cyclic_t;

/* COOP is short for COmpose OPeration */
typedef enum {
	COOP_ADD,
	COOP_REPLACE,
	COOP_DEL,
} coop_t;

/* ============================================================================ */
/* CCE: Codes and Candidates Editor */

/* CCE flags */
typedef unsigned long cce_flags_t;

#define CCE_FLAGS_KEY_MASK			0x00000007	/* bit 0-2: IME use key set */
#define CCE_FLAGS_KEY_2_TO_9		0x00000000	/* bit 0-2=0: key 2 to 9 for PinYin */
#define CCE_FLAGS_KEY_0_TO_9		0x00000001	/* bit 0-2=1: key 0 to 9 for Phonetic */
#define CCE_FLAGS_KEY_1_TO_9		0x00000002	/* bit 0-2=2: key 1 to 9 */
#define CCE_FLAGS_KEY_1_TO_6		0x00000003	/* bit 0-2=3: key 1 to 6 for WuBiHua with widecard */
#define CCE_FLAGS_KEY_1_TO_5		0x00000004	/* bit 0-2=4: key 1 to 5 for WuBiHua */
#define CCE_FLAGS_NO_CYCLIC_CODES	0x00000008	/* bit 3: no cyclic codes (e.g. WuBiHua) */
#define CCE_FLAGS_DRAW_CODES_MASK	0x00000030	/* bit 4-5: How to draw codes */
#define CCE_FLAGS_DRAW_NORMAL		0x00000000	/* bit 4-5=0: normal (e.g. PinYin) */
#define CCE_FLAGS_DRAW_WIDE			0x00000010	/* bit 4-5=1: wide codes (e.g. Phonetic) */
#define CCE_FLAGS_DRAW_IMAGE		0x00000020	/* bit 4-5=2: image codes (e.g. WuBiHua) */

/* Context function */
typedef int ( *pFnGetCyclicCodesForInputKey_t )( unsigned char key );
typedef int ( *pFnComposeCodesToRetrieveCandidates_t )( coop_t coop, unsigned char code );
typedef int ( *pFnGetRetrievedCandidates_t )( unsigned char *pCandidates, int nWant );
typedef void ( *pFnResetImeVariables_t )( void );

typedef struct cce_context_s {
	/* Get clclic codes for user's input, such as 'abc' for key 2. */
	pFnGetCyclicCodesForInputKey_t			pFnGetCyclicCodesForInputKey;
	/* Add, replace or delete a code */
	pFnComposeCodesToRetrieveCandidates_t	pFnComposeCodesToRetrieveCandidates;
	/* After above operation, you can retrieve candidates. */
	pFnGetRetrievedCandidates_t				pFnGetRetrievedCandidates;
	/* Reset variables related with IME */
	pFnResetImeVariables_t					pFnResetImeVariables;
	/* Wide codes content (CCE_FLAGS_DRAW_WIDE) */
	const unsigned char *					pWideCodesContent;
	/* Image for codes (CCE_FLAGS_DRAW_IMAGE) */
	const unsigned char * const *			ppImagesForCodes;
} cce_context_t;

#if defined( LANG_BIG5 ) || defined( LANG_GB2312 )
extern void ActivateCCEditor( const cce_context_t *pCceContext, 
							  const unsigned int *pLevel, 
							  codes_cyclic_t *pCodesCyclic,
							  cce_flags_t flags );
extern void DeactivateCCEditor( void );
#else
#define ActivateCCEditor( pc, pl, pcc, fgs )	
#define DeactivateCCEditor()	
#endif

/* ============================================================================ */
/* IME assistant flags */
typedef union {		/* flags for IME assistant */
	struct {
		unsigned long	clearCCE:1;		/* clear CCE rectangle when deactivating CCE */
	} b;
	unsigned long all;
} fImeFlags_t;

extern fImeFlags_t fImeFlags;


#endif /* __COM_IME_DEF_H__ */
