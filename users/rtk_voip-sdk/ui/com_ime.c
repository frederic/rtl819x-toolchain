#include <stdio.h>
#include "ui_config.h"
#include "com_edit.h"
#include "com_ime_def.h"
#include "com_ime_pinyin.h"
#include "com_ime_phonetic.h"
#include "com_ime_wubihua.h"
#include "com_ime_english.h"
#include "com_ime.h"
#include "ui_vkey.h"
#include "ui_softkey.h"
#include "ioctl_softrtc.h"
#include "gs_types.h"
#include "gs_shape.h"
#include "gs_lib.h"
#include "gs_drawtext.h"
#include "gs_drawimage.h"
#include "gs_font.h"
#include "res.h"
#include "res_image.h"

/*
 * IME Screen layout
 *
 *  +-------------------------------------------------+
 *  |                                                 |
 *  |                                                 |
 *  |________                                 ________|
 *  | Ins 1 | CODES                      ICON | Ins 2 |
 *  +-------------------------------------------------+
 */

/* structure decleare */
typedef void ( *pFnInitialization_t )( void );
typedef int  ( *pFnKeyProcessor_t )( unsigned char key );
typedef void ( *pFnTimerProessor_t )( void );
typedef void ( *pFnTermination_t )( void );

typedef struct ime_context_s {
	pFnInitialization_t	pFnInitialization;
	pFnKeyProcessor_t	pFnKeyProcessor;
	pFnTimerProessor_t	pFnTimerProcessor;
	pFnTermination_t	pFnTermination;
} ime_context_t;

#define M_IME_FUNCTION_SET( e )		{ e ## Initialization, e ## KeyProcessor, e ## TimerProcessor, e ## Termination }

static const ime_context_t ime_context[] = {
	M_IME_FUNCTION_SET( EnglishIme ),
#if defined( LANG_BIG5 ) || defined( LANG_GB2312 )
	M_IME_FUNCTION_SET( PinYin ),
#endif
#ifdef LANG_BIG5
	M_IME_FUNCTION_SET( Phonetic ),
#endif
#ifdef LANG_GB2312
	M_IME_FUNCTION_SET( WuBiHua ),
#endif
};

CT_ASSERT( NUM_OF_INPUT_METHOD_EDITOR == ( sizeof( ime_context ) / sizeof( ime_context[ 0 ] ) ) );

#undef M_IME_FUNCTION_SET

/* IME indicator icon */
#if !defined( _TEXT_MODE ) && ( defined( LANG_BIG5 ) || defined( LANG_GB2312 ) )
#define ENABLE_IME_INDICATOR
#endif

#ifdef ENABLE_IME_INDICATOR
#define IME_INDICATOR_ICON_MAX_WIDTH	16
#define IME_INDICATOR_ICON_MAX_HEIGHT	16

#define IME_INDICATOR_ICON_RIGHT		( VRAM_WIDTH - SOFTKEY_INS_TEXT_WIDTH * 2 - 3 )	/* *2 for two Chinese words */
#define IME_INDICATOR_ICON_LEFT			( IME_INDICATOR_ICON_RIGHT - IME_INDICATOR_ICON_MAX_WIDTH )
#define IME_INDICATOR_ICON_BOTTOM		( VRAM_HEIGHT )
#define IME_INDICATOR_ICON_TOP			( IME_INDICATOR_ICON_BOTTOM - IME_INDICATOR_ICON_MAX_HEIGHT )

static const rect_t rectImePromptIcon = {
	IME_INDICATOR_ICON_LEFT,
	IME_INDICATOR_ICON_TOP,
	IME_INDICATOR_ICON_RIGHT,
	IME_INDICATOR_ICON_BOTTOM,
};

static const unsigned char * const ime_indicator_icon[] = {
	Image_ime_indicator_English, 
#if defined( LANG_BIG5 ) || defined( LANG_GB2312 )
	Image_ime_indicator_PinYin, 
#endif
#ifdef LANG_BIG5
	Image_ime_indicator_Phonetic, 
#endif
#ifdef LANG_GB2312
	Image_ime_indicator_WuBiHua,
#endif
};

CT_ASSERT( NUM_OF_INPUT_METHOD_EDITOR == ( sizeof( ime_indicator_icon ) / sizeof( ime_indicator_icon[ 0 ] ) ) );
#endif /* ENABLE_IME_INDICATOR */

/*
 * Codes and Candidates Editor 
 */
#define NUM_OF_SCREEN_CANDIDATES		8
#define NUM_OF_SCREEN_CODES				6

typedef struct cce_candidates_s {
	unsigned short current;
	unsigned short total;
	unsigned char buffer[ NUM_OF_SCREEN_CANDIDATES * 2 ];
} cce_candidates_t;

typedef struct {
	const cce_context_t *pContext;
	const unsigned int *pLevel;
	codes_cyclic_t *pCyclic;
	cce_candidates_t candidates;
	cce_flags_t flags;
	unsigned char codes[ NUM_OF_SCREEN_CODES ];
} cce_t;

/*
 * Input Method Editor Variables
 */
struct ime_vars_s {
	ime_id_t				imeId;
	const ime_context_t *	pImeContext;
	cce_t cce;
	union {
		english_ime_t		englishImeVars;
	} share;
} ime_vars = {
	NUM_OF_INPUT_METHOD_EDITOR,		/* imeId */
	NULL,							/* pImeContext */
	{
		NULL, NULL, NULL,			/* CCE */
	},
};

#if 1
english_ime_t * const pEnglishImeVars = &ime_vars.share.englishImeVars;
#else
#define pEnglishImeVars		( &ime_vars.share.englishImeVars )
#endif

fImeFlags_t fImeFlags = { { 0 } };
CT_ASSERT( sizeof( fImeFlags ) == sizeof( fImeFlags.all ) );

#if defined( LANG_BIG5 ) || defined( LANG_GB2312 )
static inline int KeyOwnByCCEditor( unsigned char key );
static inline void CCEditorTimerEvent( void );
#else
#define KeyOwnByCCEditor( key )		0	
#define CCEditorTimerEvent()		
#endif

/*
 * Functions attach to extern 
 */
#ifdef ENABLE_IME_INDICATOR
static void DrawImeIndicatorIcon( int bDrawOrClear )
{
	int width, height;
	const unsigned char * const pImage = ime_indicator_icon[ ime_vars.imeId ];

	GS_GetResImageSize( pImage, &width, &height );

	GS_DrawResImage( IME_INDICATOR_ICON_RIGHT - width, 
					IME_INDICATOR_ICON_TOP + ( IME_INDICATOR_ICON_MAX_HEIGHT - height + 1 ) / 2, 
					pImage );
}
#else
#define DrawImeIndicatorIcon( b )
#endif

void ActivateInputMethodEditor( ime_id_t imeId )
{
	ime_vars.imeId = imeId;
	ime_vars.pImeContext = &ime_context[ imeId ];

	( *( ime_vars.pImeContext ->pFnInitialization ) )();

	DrawImeIndicatorIcon( 1 /* draw IME indicator icon */ );
}

void DeactivateInputMethodEditor( void )
{
	if( ime_vars.pImeContext == NULL )
		return;

	DeactivateCCEditor();

	( *( ime_vars.pImeContext ->pFnTermination ) )();

	ime_vars.imeId = NUM_OF_INPUT_METHOD_EDITOR;
	ime_vars.pImeContext = NULL;

	//DrawImeIndicatorIcon( 0 /* clear IME indicator icon */ );
}

int KeyOwnInputMethodEditor( unsigned char key )
{
	ime_id_t imeId;

	if( ime_vars.pImeContext == NULL )
		return 0;		/* not my key */

	if( key == VKEY_IME ) {		/* VKEY_IME is an alias */
		if( NUM_OF_INPUT_METHOD_EDITOR > 1 ) {
			/* next IME */
			imeId = ( ime_id_t )( ( int )ime_vars.imeId + 1 );

			if( imeId == NUM_OF_INPUT_METHOD_EDITOR )
				imeId = ( ime_id_t )0;

			/* ---------------------------------------------------------------- */
			fImeFlags.b.clearCCE = 1;	/* set clear CCE flag */

			GS_SupervisorDrawOffScreen();

			DeactivateInputMethodEditor();

			ActivateInputMethodEditor( imeId );

			GS_SupervisorDrawOnScreen( NULL );

			fImeFlags.b.clearCCE = 0;	/* unset clear CCE flag */
			/* ---------------------------------------------------------------- */

			return 1;
		}
	}

	if( KeyOwnByCCEditor( key ) )
		return 1;

	return ( *( ime_vars.pImeContext ->pFnKeyProcessor ) )( key );
}

void InputMethodEditorTimerEvent( void )
{
	if( ime_vars.pImeContext == NULL )
		return;

	CCEditorTimerEvent();

	( *( ime_vars.pImeContext ->pFnTimerProcessor ) )();
}

/* ============================================================================ */
/* IME Codes and Candiates UI */
/* We provide these functions to complex IME such as Chinese PinYin and phonetic */
#if defined( LANG_BIG5 ) || defined( LANG_GB2312 )

#define CCE_RECT_LEFT					( 0 )
#define CCE_RECT_TOP					( 2 * CHAR_HEIGHT )
#define CCE_RECT_RIGHT					( VRAM_WIDTH )
#define CCE_RECT_BOTTOM					( VRAM_HEIGHT )
#define CANDIDATES_RECT_LEFT			CCE_RECT_LEFT
#define CANDIDATES_RECT_TOP				CCE_RECT_TOP
#define CANDIDATES_RECT_RIGHT			CCE_RECT_RIGHT
#define CANDIDATES_RECT_BOTTOM			( CANDIDATES_RECT_TOP + CHAR_HEIGHT )
#define CODES_RECT_LEFT					( SOFTKEY_INS_TEXT_WIDTH * 2 + 3 )	/* *2 for two Chinese word */
#define CODES_RECT_TOP					CANDIDATES_RECT_BOTTOM
#define CODES_RECT_RIGHT				( CODES_RECT_LEFT + CHAR_WIDTH * 6 )	/* *6 for six codes */
#define CODES_RECT_BOTTOM				CCE_RECT_BOTTOM

CT_ASSERT( IME_INDICATOR_ICON_LEFT > CODES_RECT_RIGHT );	/* prevent overlap */

static const rect_t rectCCE = {
	CCE_RECT_LEFT,
	CCE_RECT_TOP,
	CCE_RECT_RIGHT,
	CCE_RECT_BOTTOM,
};

static const rect_t rectCandidates = {
	CANDIDATES_RECT_LEFT,
	CANDIDATES_RECT_TOP,
	CANDIDATES_RECT_RIGHT,
	CANDIDATES_RECT_BOTTOM,
}; 

static const rect_t rectCodes = {
	CODES_RECT_LEFT,
	CODES_RECT_TOP,
	CODES_RECT_RIGHT,
	CODES_RECT_BOTTOM,
};

static void ClearCodesAndCandidatesRectangle( void );

void ActivateCCEditor( const cce_context_t *pCceContext, 
					   const unsigned int *pLevel, 
					   codes_cyclic_t *pCodesCyclic,
					   cce_flags_t flags )
{
	if( ime_vars.cce.pContext )
		DeactivateCCEditor();

	ime_vars.cce.pContext = pCceContext;
	ime_vars.cce.pLevel = pLevel;
	ime_vars.cce.pCyclic = pCodesCyclic;
	ime_vars.cce.flags = flags;

	/* Softkey save */
	SoftkeyContextSave();
}

void DeactivateCCEditor( void )
{
	if( !ime_vars.cce.pContext )
		return;

	if( fImeFlags.b.clearCCE )
		ClearCodesAndCandidatesRectangle();

	ime_vars.cce.pContext = NULL;
}

static void DrawCandidatesSelectedRectangle( unsigned char key, rect_t *pRect )
{
	const cce_candidates_t * const pcandidates = &ime_vars.cce.candidates;

	rect_t rect;
	int fontWidth, fontHeight;

	GS_GetChnFontMaxSize( &fontWidth, &fontHeight );

	rect.top = CANDIDATES_RECT_TOP;
	rect.bottom = CANDIDATES_RECT_BOTTOM;
	rect.left = CANDIDATES_RECT_LEFT + fontWidth * pcandidates ->current;
	rect.right = rect.left + fontWidth;

	if( pRect )
		*pRect = rect;		/* save rectangle for new selected candidate */

	GS_DrawSolidRectangleInverse_rect( &rect );	/* select new candidate */

	if( key == VKEY_LEFT ) {
		rect.left += fontWidth;
		rect.right += fontWidth;
		if( pRect )
			pRect ->right = rect.right;		/* enlarge to cover old candidate */
	} else if( key == VKEY_RIGHT ) {
		rect.left -= fontWidth;
		rect.right -= fontWidth;
		if( pRect )
			pRect ->left = rect.left;		/* enlarge to cover old candidate */
	} else {
		/* Other key: draw new candidate only */
		return;
	}

	GS_DrawSolidRectangleInverse_rect( &rect );	/* unselect old candidate */
}

static void RefreshCandidatesSelectedRectangle( unsigned char key )
{
	rect_t rectSelected;

	GS_DrawOffScreen();

	DrawCandidatesSelectedRectangle( key, &rectSelected );

	GS_DrawOnScreen( &rectSelected );
}

#define DRAW_CODES				0x01
#define DRAW_CANDIDATES			0x02

static void DrawCodesAndCandidatesRectangle( unsigned int maskDraw )
{
	codes_cyclic_t * const pcyclic = ime_vars.cce.pCyclic;
	cce_candidates_t * const pcandidates = &ime_vars.cce.candidates;
	const cce_flags_t flags = ime_vars.cce.flags;
	const unsigned char * const pwidecodes = ime_vars.cce.pContext ->pWideCodesContent;
	const unsigned char * const * const ppimage = ime_vars.cce.pContext ->ppImagesForCodes;
	rect_t rect;
	unsigned int i;

	if( maskDraw & DRAW_CODES ) {
		/* clear codes rectangle */
		GS_DrawSolidRectangle_rect( &rectCodes, 0 );

		/* draw codes */
		switch( flags & CCE_FLAGS_DRAW_CODES_MASK ) {
		case CCE_FLAGS_DRAW_WIDE:
			for( i = 0; i < *ime_vars.cce.pLevel; i ++ )
			{
				GS_DrawText( CODES_RECT_LEFT + i * CHAR_WIDTH * 2, 
							 CODES_RECT_TOP, 
							 &pwidecodes[ ( ime_vars.cce.codes[ i ] - 1 ) * 2 ], 2 );
			}
			break;

		case CCE_FLAGS_DRAW_IMAGE:
			for( i = 0; i < *ime_vars.cce.pLevel; i ++ )
			{
				GS_DrawResImage( CODES_RECT_LEFT + i * CHAR_WIDTH, 
								 CODES_RECT_TOP, 
								 ppimage[ ime_vars.cce.codes[ i ] - 1 ] );

			}
			break;

		default:
			GS_DrawText( CODES_RECT_LEFT, CODES_RECT_TOP, ime_vars.cce.codes, *ime_vars.cce.pLevel );
			break;
		}

		/* 
		 * WuBiHua uses flag CCE_FLAGS_NO_CYCLIC_CODES but also set key to '\x0'. 
		 * Thus, we don't need to check this flag. 
		 */
		if( pcyclic ->key ) {
			/* inverse cyclic code */
			rect = rectCodes;

			switch( flags & CCE_FLAGS_DRAW_CODES_MASK ) {
			case CCE_FLAGS_DRAW_WIDE:
				rect.left += ( *ime_vars.cce.pLevel - 1 ) * 2 * CHAR_WIDTH;
				rect.right = rect.left + 2 * CHAR_WIDTH;
				break;

			case CCE_FLAGS_DRAW_IMAGE:
			default:
				rect.left += ( *ime_vars.cce.pLevel - 1 ) * CHAR_WIDTH;
				rect.right = rect.left + CHAR_WIDTH;
				break;
			}

			GS_DrawSolidRectangleInverse_rect( &rect );
		}
	}

	if( maskDraw & DRAW_CANDIDATES ) {
		GS_DrawSolidRectangle_rect( &rectCandidates, 0 );
		GS_DrawText( CANDIDATES_RECT_LEFT, CANDIDATES_RECT_TOP, pcandidates ->buffer, pcandidates ->total * 2 );
		DrawCandidatesSelectedRectangle( '\x0', NULL );
	}
}

static void RefreshCandidatesRectangle( void )
{
	GS_DrawOffScreen();

	DrawCodesAndCandidatesRectangle( DRAW_CANDIDATES );

	GS_DrawOnScreen( &rectCandidates );
}

static void RefreshCodesRectangle( void )
{
	GS_DrawOffScreen();

	DrawCodesAndCandidatesRectangle( DRAW_CODES );

	GS_DrawOnScreen( &rectCodes );
}

static void RefreshCodesAndCandidatesRectangle( int bDrawInstructionText )
{
	GS_DrawOffScreen();

	DrawCodesAndCandidatesRectangle( DRAW_CODES | DRAW_CANDIDATES );

	if( bDrawInstructionText )
		RightSoftkeyConfigurationWithInstructionText( VKEY_CLEAR, szInsClear );

	GS_DrawOnScreen( &rectCCE );
}

static void ClearCodesAndCandidatesRectangle( void )
{
	GS_DrawOffScreen();

	GS_DrawSolidRectangle_rect( &rectCodes, 0 );
	GS_DrawSolidRectangle_rect( &rectCandidates, 0 );

	/* Softkey restore */
	SoftkeyContextRestore( 1 /* draw instruction text */ );

	GS_DrawOnScreen( &rectCCE );
}

static inline int KeyOwnByCCEditor_Core( unsigned char key )
{
	/* Const variables to make it easy to study */
	codes_cyclic_t * const pcyclic = ime_vars.cce.pCyclic;
	cce_candidates_t * const pcandidates = &ime_vars.cce.candidates;
	pFnGetCyclicCodesForInputKey_t const pFnGetCyclicCodesForInputKey = ime_vars.cce.pContext ->pFnGetCyclicCodesForInputKey;
	pFnComposeCodesToRetrieveCandidates_t const pFnComposeCodesToRetrieveCandidates = ime_vars.cce.pContext ->pFnComposeCodesToRetrieveCandidates;
	pFnGetRetrievedCandidates_t const pFnGetRetrievedCandidates = ime_vars.cce.pContext ->pFnGetRetrievedCandidates;
	pFnResetImeVariables_t const pFnResetImeVariables = ime_vars.cce.pContext ->pFnResetImeVariables;
	const cce_flags_t flags = ime_vars.cce.flags;

	int bDrawInstructionText = 0;
	int bDrawCodes = 0;

	if( *ime_vars.cce.pLevel == 0 )		/* No any codes, so don't operate candidates */
		goto label_compose_codes_and_candidates_start;

	/* Candidates move and selection function */
	if( key == VKEY_OK ) {
		SendWideTextToTextEditor( pcandidates ->buffer[ pcandidates ->current * 2 ],
								  pcandidates ->buffer[ pcandidates ->current * 2 + 1 ] );

		( *pFnResetImeVariables )();

		goto label_enter_initial_state_so_clear_candidates;
	} else if( key == VKEY_LEFT ) {
		if( pcandidates ->current == 0 ) {
			key = VKEY_UP;
			goto label_do_candidates_page_up_down;
		}
		pcandidates ->current --;
		goto label_refresh_candidates_selected_rectangle;
	} else if( key == VKEY_RIGHT ) {
		if( pcandidates ->current + 1 >= pcandidates ->total ) {
			key = VKEY_DOWN;
			goto label_do_candidates_page_up_down;
		}
		pcandidates ->current ++;
		goto label_refresh_candidates_selected_rectangle;
	} else if( key == VKEY_DOWN || key == VKEY_UP ) {
label_do_candidates_page_up_down:
		pcandidates ->total = ( *pFnGetRetrievedCandidates )( pcandidates ->buffer, 
					( key == VKEY_DOWN ? NUM_OF_SCREEN_CANDIDATES : -NUM_OF_SCREEN_CANDIDATES ) );
		pcandidates ->current = ( key == VKEY_DOWN ? 0 : pcandidates ->total - 1 );
		RefreshCandidatesRectangle();
		return 1;
	} else if( key == VKEY_CLEAR ) {
		if( ( *pFnComposeCodesToRetrieveCandidates )
										( COOP_DEL, pcyclic ->codes[ pcyclic ->current ] )  )
		{
			pcyclic ->key = 0;

			if( *ime_vars.cce.pLevel == 0 )
				goto label_enter_initial_state_so_clear_candidates;
			else
				goto label_draw_get_retrieved_candidates_and_draw_it;
		}
	}

label_compose_codes_and_candidates_start:
	switch( flags & CCE_FLAGS_KEY_MASK ) {
	case CCE_FLAGS_KEY_0_TO_9:
		if( key < '0' || key > '9' )
			return 0;	/* not my key */
		break;
	case CCE_FLAGS_KEY_1_TO_9:
		if( key < '1' || key > '9' )
			return 0;	/* not my key */
		break;
	case CCE_FLAGS_KEY_2_TO_9:
		if( key < '2' || key > '9' )
			return 0;	/* not my key */
		break;
	case CCE_FLAGS_KEY_1_TO_6:
		if( key < '1' || key > '6' )
			return 0;	/* not my key */
		break;
	case CCE_FLAGS_KEY_1_TO_5:
		if( key < '1' || key > '5' )
			return 0;	/* not my key */
		break;
	}

	/* Add/Delete/Cyclic Codes */
	if( pcyclic ->key == 0 ) {
label_search_for_new_cyclic:
		/* search for new cyclic codes set */
		if( ( *pFnGetCyclicCodesForInputKey )( key ) == 0 ) {
			if( bDrawCodes )
				goto label_refresh_codes_rectangle;	/* press a new key, but no candidates */
			else
				goto label_do_nothing_for_this_key;
		}

		ime_vars.cce.codes[ *ime_vars.cce.pLevel ] = pcyclic ->codes[ pcyclic ->current ];
		( *pFnComposeCodesToRetrieveCandidates )( COOP_ADD, pcyclic ->codes[ pcyclic ->current ] );

		/* Draw right softkey as 'clear' */
		if( *ime_vars.cce.pLevel == 1 )
			bDrawInstructionText = 1;

		goto label_draw_get_retrieved_candidates_and_draw_it;
	} else if( pcyclic ->key == key ) {
		/* cyclic the key */
		pcyclic ->time = GetUptimeInMillisecond();

		if( pcyclic ->total <= 1 )
			goto label_do_nothing_for_this_key;
			
		if( ++ pcyclic ->current == pcyclic ->total )
			pcyclic ->current = 0;

		ime_vars.cce.codes[ *ime_vars.cce.pLevel - 1 ] = pcyclic ->codes[ pcyclic ->current ];
		( *pFnComposeCodesToRetrieveCandidates )( COOP_REPLACE, pcyclic ->codes[ pcyclic ->current ] );

		goto label_draw_get_retrieved_candidates_and_draw_it;
	} else if( pcyclic ->key != key ) {
		pcyclic ->key = 0;
		bDrawCodes = 1;		/* We need to clear inversed rectangle */
		goto label_search_for_new_cyclic;
	}

label_do_nothing_for_this_key:
	return 1;

label_draw_get_retrieved_candidates_and_draw_it:

	pcandidates ->total = ( *pFnGetRetrievedCandidates )( pcandidates ->buffer, NUM_OF_SCREEN_CANDIDATES );
	pcandidates ->current = 0;
	RefreshCodesAndCandidatesRectangle( bDrawInstructionText );

	return 1;

label_enter_initial_state_so_clear_candidates:
	ClearCodesAndCandidatesRectangle();
	return 1;

label_refresh_candidates_selected_rectangle:
	RefreshCandidatesSelectedRectangle( key );
	return 1;

label_refresh_codes_rectangle:
	RefreshCodesRectangle();
	return 1;
}

static inline int KeyOwnByCCEditor( unsigned char key )
{
	if( !ime_vars.cce.pContext )
		return 0;

	return KeyOwnByCCEditor_Core( key );
}

static inline void CCEditorTimerEvent( void )
{
	codes_cyclic_t * const pcyclic = ime_vars.cce.pCyclic;
	
	if( !ime_vars.cce.pContext )
		return;

	if( pcyclic ->key &&
		CheckIfTimeoutInMillisecond( &pcyclic ->time, CYCLIC_CODES_TIME ) == 0 ) 
	{
		pcyclic ->key = 0;
		RefreshCodesRectangle();
	}
}

#endif /* LANG_BIG5 || LANG_GB2312 */
