#include <stdio.h>
#include <string.h>
#include "ui_config.h"
#include "ui_host.h"
#include "com_edit.h"
#include "gs_cursor.h"
#include "gs_lib.h"

/* static function for each editor */
static void SingleLineEditorInitialization( const params_for_editor_t *pParams );
static int  SingleLineEditorKeyProcessor( unsigned char key );
static int  SingleLineEditorTextProcessor( unsigned char text, editor_cmd_t cmd );
static int  SingleLineEditorGetText( unsigned char *pszText, int lenText );
static void SingleLineEditorTermination( void );

/* structure declare */
typedef void ( *pFnInitialization_t )( const params_for_editor_t *pParams );
typedef int  ( *pFnKeyProcessor_t )( unsigned char key );
typedef int  ( *pFnTextProcessor_t )( unsigned char text, editor_cmd_t cmd );
typedef int  ( *pFnGetText_t )( unsigned char *pszText, int lenText );
typedef void ( *pFnTermination_t )( void );

typedef struct editor_context_s {
	pFnInitialization_t		pFnInitializeation;
	pFnKeyProcessor_t		pFnKeyProcessor;
	pFnTextProcessor_t		pFnTextProcessor;
	pFnGetText_t			pFnGetText;
	pFnTermination_t		pFnTermination;
} editor_context_t;

#define M_EDITOR_FUNCTION_SET( e )		{ e ## Initialization, e ## KeyProcessor, e ## TextProcessor, e ## GetText, e ## Termination }

static const editor_context_t editor_context[] = {
	M_EDITOR_FUNCTION_SET( SingleLineEditor ),	/* EDITOR_ID_SINGLE_LINE */
};

#undef M_EDITOR_FUNCTION_SET

CT_ASSERT( ( sizeof( editor_context ) / sizeof( editor_context[ 0 ] ) ) == NUM_OF_TEXT_EDITOR );

/*
 * Editor Variables
 */
typedef struct single_line_s {
	text_type_t		tTextType;
	int				nNumberOfText;
	int				nMaxLengthOfText;
	int				nCandidateCount;
	int				nDisplayYaxis;
	int				nNotRefreshText;
	params_flags_t	fParams;
	unsigned char	nTextBuffer[ MAX_LENGTH_OF_SINGLE_LINE_EDITOR + 1 ];	/* +1 for null terminator */
} single_line_t;

struct editor_vars_s {
	editor_id_t				idEditor;
	const editor_context_t	*pEditorContext;
	union {
		single_line_t		single_line_vars;
	} share;
} editor_vars = { 
	NUM_OF_TEXT_EDITOR,
	NULL,
};

#if 1
  static single_line_t * const pSingleLineVars = &editor_vars.share.single_line_vars;
#else
  #define pSingleLineVars		( &editor_vars.share.single_line_vars )
#endif

/*
 * Assistant constant
 */
#define SPACE_STRING		( const unsigned char * )" "

/*
 * Functions attach to extern 
 */
void ActivateTextEditor( editor_id_t editor_id, const params_for_editor_t *pParams )
{
	if( editor_id < NUM_OF_TEXT_EDITOR ) {
		editor_vars.idEditor = editor_id;
		editor_vars.pEditorContext = &editor_context[ editor_id ];
	} else {
		editor_vars.idEditor = NUM_OF_TEXT_EDITOR;
		editor_vars.pEditorContext = NULL;
		debug_out( "Bad edit_id: %d\n", editor_id );
	}

	if( editor_vars.pEditorContext == NULL )
		return;

	( *( editor_vars.pEditorContext ->pFnInitializeation ) )( pParams );
}

void DeactivateTextEditor( void )
{
	if( editor_vars.pEditorContext == NULL )
		return;
	
	( *( editor_vars.pEditorContext ->pFnTermination ) )();

	editor_vars.pEditorContext = NULL;
	editor_vars.idEditor = NUM_OF_TEXT_EDITOR;
}

int SendTextToTextEditor( unsigned char text, editor_cmd_t cmd )
{
	if( editor_vars.pEditorContext == NULL )
		return 0;

	return ( *( editor_vars.pEditorContext ->pFnTextProcessor ) )( text, cmd );
}

int SendWideTextToTextEditor( unsigned char text1, unsigned char text2 )
{
	int ret;

	if( ( ret = SendTextToTextEditor( text1, EDITOR_CMD_NOT_REFRESH ) ) == 0 )
		return 0;

	return SendTextToTextEditor( text2, EDITOR_CMD_NORMAL );
}

int KeyOwnByTextEditor( unsigned char key )
{
	if( editor_vars.pEditorContext == NULL )
		return 0;	/* not my key */

	return ( *( editor_vars.pEditorContext ->pFnKeyProcessor ) )( key );	
}

int GetTextEditorTextContent( unsigned char *pszText, int lenText )
{
	/*
	 * If text is a string, lenText exclusive null-terminator.
	 * If text is a bcd string, lenText include 0xF terminator.
	 */
	if( editor_vars.pEditorContext == NULL )
		return 0;

	return ( *( editor_vars.pEditorContext ->pFnGetText ) )( pszText, lenText );
}

/* ============================================================================ */
/* ============================================================================ */
/* ============================================================================ */
/*
 * EDITOR_ID_SINGLE_LINE
 */
#define DEFAULT_Y_AXIS_OF_SINGLE_LINE_EDITOR		1

static void DrawSingleLineEditorScreen( int idx_start, int idx_end )
{
	int len, space_len;

	if( pSingleLineVars ->nNumberOfText >= idx_end ) {
		len = idx_end - idx_start;
		space_len = 0;
	} else {
		len = pSingleLineVars ->nNumberOfText - idx_start;
		space_len = idx_end - pSingleLineVars ->nNumberOfText;

		/* borrow idx_end to indicate starting index to draw space */
		idx_end = pSingleLineVars ->nNumberOfText;
	}

	GS_TextOut( idx_start, pSingleLineVars ->nDisplayYaxis, 
				 &pSingleLineVars ->nTextBuffer[ idx_start ], len );

	while( space_len -- ) {
		GS_TextOut( idx_end, pSingleLineVars ->nDisplayYaxis, SPACE_STRING, 1 );
		idx_end ++;
	}
}

static void RefreshSingleLineEditorScreen( int idx_start, int idx_end )
{
	rect_t rect;

	GS_DrawOffScreen();
	GS_SetCursorStatus( CURSOR_STATUS_OFF );

	DrawSingleLineEditorScreen( idx_start, idx_end );

	rect.top = ( pSingleLineVars ->nDisplayYaxis ) * CHAR_HEIGHT;
	rect.bottom = ( rect.top + 1 ) * CHAR_HEIGHT;
	rect.left = idx_start * CHAR_WIDTH;
	rect.right = idx_end * CHAR_WIDTH;

	GS_DrawOnScreen( &rect );

	/* Set cursor position */
	if( pSingleLineVars ->nCandidateCount ) {
		GS_SetCursorPosition( pSingleLineVars ->nNumberOfText - 1, pSingleLineVars ->nDisplayYaxis );
		GS_SetCursorStatus( CURSOR_STATUS_SQUARE );
	} else {
		GS_SetCursorPosition( pSingleLineVars ->nNumberOfText, pSingleLineVars ->nDisplayYaxis );
		GS_SetCursorStatus( CURSOR_STATUS_ON );
	}
}

static void SingleLineEditorInitialization( const params_for_editor_t *pParams )
{
	int nDefaultLength;
	int nMaxLength;
	const params_for_single_line_editor_t * const pParamsSingle = &pParams ->single_line;

	/* decide default text */
	if( pParamsSingle ->tTextType == TEXT_TYPE_BCD ) {
		/* BCD */
		if( pParamsSingle ->pszDefaultText ) {
			nDefaultLength = pSingleLineVars ->nNumberOfText = 
					GetTextStringFromBcd( pSingleLineVars ->nTextBuffer,
										  MAX_LENGTH_OF_SINGLE_LINE_EDITOR,
									      pParamsSingle ->pszDefaultText );
		} else {
			nDefaultLength = pSingleLineVars ->nNumberOfText = 0;
			pSingleLineVars ->nTextBuffer[ 0 ] = '\x0';
		}
	} else {
		/* Normal text or IP */
		nDefaultLength = pParamsSingle ->nDefaultTextLength;

		if( nDefaultLength > 0 ) {
			if( nDefaultLength > MAX_LENGTH_OF_SINGLE_LINE_EDITOR )
				nDefaultLength = MAX_LENGTH_OF_SINGLE_LINE_EDITOR;
			
			pSingleLineVars ->nNumberOfText = nDefaultLength;
			memcpy( pSingleLineVars ->nTextBuffer, 
					pParamsSingle ->pszDefaultText,
					nDefaultLength );

		} else {
			pSingleLineVars ->nNumberOfText = 0;
		}
	}

	/* decide max text length */
	nMaxLength = pParamsSingle ->nMaxTextLength;

	if( nMaxLength == 0 || nMaxLength > MAX_LENGTH_OF_SINGLE_LINE_EDITOR )
		nMaxLength = MAX_LENGTH_OF_SINGLE_LINE_EDITOR;

	pSingleLineVars ->nMaxLengthOfText = nMaxLength;

	/* initialize others */
	pSingleLineVars ->nCandidateCount = 0;
	pSingleLineVars ->nDisplayYaxis = DEFAULT_Y_AXIS_OF_SINGLE_LINE_EDITOR;
	pSingleLineVars ->tTextType = pParamsSingle ->tTextType;
	pSingleLineVars ->fParams = pParams ->single_line.fParams;
	pSingleLineVars ->nNotRefreshText = 0;

	if( nDefaultLength > 0 )
		DrawSingleLineEditorScreen( 0, pSingleLineVars ->nNumberOfText );

	GS_SetCursorPosition( pSingleLineVars ->nNumberOfText, pSingleLineVars ->nDisplayYaxis );
}

static void SingleLineEditorTermination( void )
{
}

static int SingleLineEditorKeyProcessor( unsigned char key )
{
	int start_idx;

	if( key == VKEY_BACKSPACE && pSingleLineVars ->nNumberOfText ) {
		goto label_single_line_editor_erase_a_text;
	} else if( pSingleLineVars ->tTextType == TEXT_TYPE_IP ) {
		if( IsNumberKey( key ) ) {
			SingleLineEditorTextProcessor( key, EDITOR_CMD_NORMAL );
			goto label_single_line_editor_key_processor_done;
		} else if( key == VKEY_STAR ) {
			SingleLineEditorTextProcessor( '.', EDITOR_CMD_NORMAL );
			goto label_single_line_editor_key_processor_done;
		} else
			return 0;	/* not my key */
	} else {
		if( IsDialKey( key ) ) {
			SingleLineEditorTextProcessor( key, EDITOR_CMD_NORMAL );
			goto label_single_line_editor_key_processor_done;
		} else
			return 0;	/* not my key */
	}

	

#if 0
	if( pSingleLineVars ->nNumberOfText < pSingleLineVars ->nMaxLengthOfText ) {

		pSingleLineVars ->nTextBuffer[ pSingleLineVars ->nNumberOfText ] = key;
		start_idx = pSingleLineVars ->nNumberOfText;
		pSingleLineVars ->nNumberOfText ++;

		RefreshSingleLineEditorScreen( start_idx, pSingleLineVars ->nNumberOfText );
	}
#endif

label_single_line_editor_erase_a_text:
	start_idx = pSingleLineVars ->nNumberOfText;
#ifdef _TEXT_MODE
	pSingleLineVars ->nNumberOfText --;
#else
	pSingleLineVars ->nNumberOfText -= GS_GetLastNchOfTextString( 
							pSingleLineVars ->nTextBuffer, pSingleLineVars ->nNumberOfText );
#endif

	RefreshSingleLineEditorScreen( pSingleLineVars ->nNumberOfText, start_idx + 1 );	/* +1 for cursor */

label_single_line_editor_key_processor_done:

	if( pSingleLineVars ->fParams.b.keyBypass )
		return 0;	/* If bypass, it will seen as not my key. */

	return 1;	/* my key */
}

static int SingleLineEditorTextProcessor( unsigned char text, editor_cmd_t cmd )
{
	int start_idx, end_idx;
	int bTextBufferFull;

	if( pSingleLineVars ->nNumberOfText < pSingleLineVars ->nMaxLengthOfText )
		bTextBufferFull = 0;
	else
		bTextBufferFull = 1;

	switch( cmd ) {
	case EDITOR_CMD_NORMAL:		/* add text to buffer */
		if( bTextBufferFull ) {
			pSingleLineVars ->nNumberOfText -= pSingleLineVars ->nNotRefreshText;
			pSingleLineVars ->nNotRefreshText = 0;
			goto label_single_line_editor_text_processor_do_nothing;
		} else
			goto label_single_line_editor_text_processor_add_a_text;
		break;

	/* on spot editing */
	case EDITOR_CMD_CANDIDATE:	/* this text is a candidate */
		if( bTextBufferFull )
			goto label_single_line_editor_text_processor_do_nothing;
		else {
			pSingleLineVars ->nCandidateCount ++;
			goto label_single_line_editor_text_processor_add_a_text;
		}
		break;

	case EDITOR_CMD_REPLACE:		/* replace candidate */
		if( pSingleLineVars ->nCandidateCount ) {
			start_idx = pSingleLineVars ->nNumberOfText - 1;
			end_idx = pSingleLineVars ->nNumberOfText;
			pSingleLineVars ->nTextBuffer[ start_idx ] = text;
			goto label_single_line_editor_text_processor_refresh_screen;
		} else {
			printf( "Single line editor (REPLACE): no candidate count??\n" );
			goto label_single_line_editor_text_processor_do_nothing;
		}
		break;

	case EDITOR_CMD_ACCEPT:		/* accept candidate (no text) */
		if( pSingleLineVars ->nCandidateCount ) {
			start_idx = pSingleLineVars ->nNumberOfText - pSingleLineVars ->nCandidateCount;
			end_idx = pSingleLineVars ->nNumberOfText;
			pSingleLineVars ->nCandidateCount = 0;
			goto label_single_line_editor_text_processor_refresh_screen;
		} else {
			printf( "Single line editor (ACCEPT): no candidate count?? \n" );
			goto label_single_line_editor_text_processor_do_nothing;
		}
		break;

	case EDITOR_CMD_CANCEL:		/* cancel candidate (no text) */
		if( pSingleLineVars ->nCandidateCount ) {
			end_idx = pSingleLineVars ->nNumberOfText;
			pSingleLineVars ->nNumberOfText -= pSingleLineVars ->nCandidateCount;
			start_idx = pSingleLineVars ->nNumberOfText;
			pSingleLineVars ->nCandidateCount = 0;
			goto label_single_line_editor_text_processor_refresh_screen;
		} else {
			printf( "Single line editor (CANCEL): no candidate count?? \n" );
			goto label_single_line_editor_text_processor_do_nothing;
		}
		break;

	/* for wide-char */
	case EDITOR_CMD_NOT_REFRESH:	/* refresh unit receive next character */
		pSingleLineVars ->nNotRefreshText ++;
		goto label_single_line_editor_text_processor_add_a_text;
		break;

	default:
		printf( "Part of Single Line Editor is not implement!\n" );
		goto label_single_line_editor_text_processor_do_nothing;
		break;
	}

label_single_line_editor_text_processor_add_a_text:
	/* Add text to buffer */
	pSingleLineVars ->nTextBuffer[ pSingleLineVars ->nNumberOfText ] = text;
	start_idx = pSingleLineVars ->nNumberOfText - pSingleLineVars ->nNotRefreshText;
	pSingleLineVars ->nNumberOfText ++;
	end_idx = pSingleLineVars ->nNumberOfText;

	if( cmd == EDITOR_CMD_NOT_REFRESH ) 
		goto label_single_line_editor_text_processor_done;

label_single_line_editor_text_processor_refresh_screen:
	pSingleLineVars ->nNotRefreshText = 0;
	RefreshSingleLineEditorScreen( start_idx, end_idx );

label_single_line_editor_text_processor_do_nothing:
label_single_line_editor_text_processor_done:
	return pSingleLineVars ->nNumberOfText;	/* length of text */
}

static int SingleLineEditorGetText( unsigned char *pszText, int lenText )
{
	if( pszText == NULL )
		goto label_return_length_of_text;

	if( pSingleLineVars ->tTextType == TEXT_TYPE_BCD ) {

		pSingleLineVars ->nTextBuffer[ pSingleLineVars ->nNumberOfText ] = '\x0';
		GetBcdStringFromText( pszText, lenText, pSingleLineVars ->nTextBuffer );
	} else {
		/* pSingleLineVars ->tTextType == TEXT_TYPE_NORMAL */
		memcpy( pszText, pSingleLineVars ->nTextBuffer, pSingleLineVars ->nNumberOfText );
		pszText[ pSingleLineVars ->nNumberOfText ] = '\x0';
	}

label_return_length_of_text:
	return pSingleLineVars ->nNumberOfText;	/* length of text */
}

/* ============================================================================ */
/* ============================================================================ */
/* ============================================================================ */
