#include <string.h>
#include <stdio.h>
#include "ui_config.h"
#include "ui_softkey.h"
#include "ui_vkey.h"
#include "gs_types.h"
#include "gs_drawtext.h"
#include "gs_shape.h"
#include "gs_font.h"
#include "res.h"

#ifdef SOFTKEY_SUPPORT

typedef enum {
	ALIGN_LEFT,
	ALIGN_CENTER,
	ALIGN_RIGHT,
} align_t;

#if KEYPAD_MAP_VENDOR == VENDOR_WCO2
#define SOFTKEY_NUM		8

static const struct {
	unsigned char vkey;
	rect_t rectInstText;
	align_t align;
} locInstText[] = {
	{ VKEY_INS_B1, { 0, SOFTKEY_TEXT_LOC_Y + 1, 32 - 1, VRAM_HEIGHT - 1 }, ALIGN_LEFT }, 
	{ VKEY_INS_B2, { 32, SOFTKEY_TEXT_LOC_Y + 1, 64 - 1, VRAM_HEIGHT - 1 }, ALIGN_LEFT }, 
	{ VKEY_INS_B3, { 64, SOFTKEY_TEXT_LOC_Y + 1, 96 - 1, VRAM_HEIGHT - 1 }, ALIGN_LEFT }, 
	{ VKEY_INS_B4, { 96, SOFTKEY_TEXT_LOC_Y + 1, VRAM_WIDTH - 1, VRAM_HEIGHT - 1 }, ALIGN_RIGHT }, 
};
#endif /* KEYPAD_MAP_VENDOR == VENDOR_WCO2 */

static struct {
	int nUsedKey;
	struct {
		unsigned char vkeySource;
		unsigned char vkeyTarget;
		const unsigned char *pszInstructionText;
	} map[ SOFTKEY_NUM ];
} softkey, backupSoftkey = { 0 };

#define INST_TEXT_NUM		( sizeof( locInstText ) / sizeof( locInstText[ 0 ] ) )

void ResetSoftkeyConfiguration( void )
{
	softkey.nUsedKey = 0;
}

void ClearSoftkeyInstructionTextRetangle( rect_t *prect )
{
	GS_DrawSolidRectangle( 0, SOFTKEY_TEXT_LOC_Y, VRAM_WIDTH, VRAM_HEIGHT, 0 );

	if( prect ) {
		prect ->left = 0;
		prect ->top = SOFTKEY_TEXT_LOC_Y;
		prect ->right = VRAM_WIDTH;
		prect ->bottom = VRAM_HEIGHT;
	}
}

static int SearchForSoftkeyIndex( unsigned char vkey )
{
	int i;

	for( i =0; i < softkey.nUsedKey; i ++ )
		if( softkey.map[ i ].vkeySource == vkey )
			return i;

	return -1;
}

void DoSoftkeyTranslation( unsigned char *pkey )
{
	int index;

	if( ( index = SearchForSoftkeyIndex( *pkey ) ) == -1 )
		return;

	*pkey = softkey.map[ index ].vkeyTarget;
}

static int ReplaceSoftkeyTranslationWithIndex( int index, unsigned char vkeySource, unsigned char vkeyTarget )
{
	if( index >= softkey.nUsedKey ) {
		debug_out( "Replace index too large!\n" );
		return -1;
	}

	softkey.map[ index ].vkeySource = vkeySource;
	softkey.map[ index ].vkeyTarget = vkeyTarget;
	softkey.map[ index ].pszInstructionText = NULL;

	return index;
}

static int AddSoftkeyTranslation( unsigned char vkeySource, unsigned char vkeyTarget )
{
	int index;

	if( softkey.nUsedKey >= SOFTKEY_NUM ) {
		debug_out( "Softkey is full!\n" );
		return -1;
	}

	index = softkey.nUsedKey;

	softkey.map[ index ].vkeySource = vkeySource;
	softkey.map[ index ].vkeyTarget = vkeyTarget;
	softkey.map[ index ].pszInstructionText = NULL;
	softkey.nUsedKey ++;

	return index;
}

static int DrawInstructionTextByVkey( unsigned char vkey, const unsigned char *pszInstructionText )
{
	int i;
	eng_font_id_t idEngFont;
	chn_font_id_t idChnFont;
	rect_t rect;
	int len, textWidth;

	for( i = 0; i < INST_TEXT_NUM; i ++ )
		if( locInstText[ i ].vkey == vkey )
			goto label_do_draw_text;

	return -1;

label_do_draw_text:
	idEngFont = GS_SelectEngFont( ENG_FONT_ID_VARIABLE );
	idChnFont = GS_SelectChnFont( CHN_FONT_ID_SMALL );

	rect = locInstText[ i ].rectInstText;
	len = strlen( ( const char * )pszInstructionText );
	GS_GetTextWidthHeight( pszInstructionText, len, &textWidth, NULL );

	switch( locInstText[ i ].align ) {
	case ALIGN_LEFT:
		rect.right = rect.left + textWidth + 1;
		break;

	case ALIGN_RIGHT:
		rect.left = rect.right - textWidth - 1;
		break;

	case ALIGN_CENTER:
		i = ( rect.right - rect.left + 1 - textWidth ) / 2;
		rect.left += i;
		rect.right -= i;
		break;
	};

	GS_DrawText( rect.left + 1, rect.top + 1, pszInstructionText, len );

	GS_SelectEngFont( idEngFont );
	GS_SelectChnFont( idChnFont );

	GS_DrawRectangle_rect( &rect );

	return i;
}

void DrawAllInstructionTextByContext( void )
{
	int i;

	for( i = 0; i < softkey.nUsedKey; i ++ ) 
		DrawInstructionTextByVkey( softkey.map[ i ].vkeySource, softkey.map[ i ].pszInstructionText );
}

int SoftkeyConfiguration( unsigned char vkeySource, unsigned char vkeyTarget )
{
	int index;

	if( ( index = SearchForSoftkeyIndex( vkeySource ) ) == -1 ) {
		/* source vkey doesn't appear in mapping table */
		if( ( index = AddSoftkeyTranslation( vkeySource, vkeyTarget ) ) == -1 ) {
			/* mapping table is full */
		}
	} else {
		/* softkey is exist, so replace it */
		ReplaceSoftkeyTranslationWithIndex( index, vkeySource, vkeyTarget );
	}

	return index;
}

void SoftkeyConfigurationWithInstructionText( unsigned char vkeySource, unsigned char vkeyTarget, const unsigned char *pszInstructionText )
{
	int index;

	/* Config mapping table */
	if( ( index = SoftkeyConfiguration( vkeySource, vkeyTarget ) ) == -1 )
		return;

	softkey.map[ index ].pszInstructionText = pszInstructionText;

	/* Mapping table is ok, so we are going to draw instruction text now. */
	if( ( index = DrawInstructionTextByVkey( vkeySource, pszInstructionText ) ) == -1 ) 
		return;		/* this key has to instruction text location */
}

/* ---------------------------------------------------------------------------- */
/* Context switch */
void SoftkeyContextSave( void )
{
	backupSoftkey = softkey;
}

void SoftkeyContextRestore( int bDraw )
{
	softkey = backupSoftkey;

	if( bDraw )
		DrawAllInstructionTextByContext();	
}

/* ---------------------------------------------------------------------------- */
/* Specialize softkey indirect */
/* Left softkey --> VKEY_INS_B1 */
void LeftSoftkeyConfigurationWithInstructionText( unsigned char vkeyTarget, const unsigned char *pszInstructionText )
{
	SoftkeyConfigurationWithInstructionText( VKEY_INS_B1, vkeyTarget, pszInstructionText );
}

/* Middle softkey --> VKEY_INS_B2 (allow long instruction text) */
void MiddleSoftkeyConfigurationWithInstructionText( unsigned char vkeyTarget, const unsigned char *pszInstructionText )
{
	SoftkeyConfigurationWithInstructionText( VKEY_INS_B2, vkeyTarget, pszInstructionText );
}

/* Middle2 softkey --> VKEY_INS_B3 (don't allow long instruction text) */
void Middle2SoftkeyConfigurationWithInstructionText( unsigned char vkeyTarget, const unsigned char *pszInstructionText )
{
	SoftkeyConfigurationWithInstructionText( VKEY_INS_B3, vkeyTarget, pszInstructionText );
}

/* Right softkey --> VKEY_INS_B4 */
void RightSoftkeyConfigurationWithInstructionText( unsigned char vkeyTarget, const unsigned char *pszInstructionText )
{
	SoftkeyConfigurationWithInstructionText( VKEY_INS_B4, vkeyTarget, pszInstructionText );
}

void SoftkeyStandardConfiguration_OkBack( void )
{
	LeftSoftkeyConfigurationWithInstructionText( VKEY_OK, szInsOK );
	RightSoftkeyConfigurationWithInstructionText( VKEY_CANCEL, szInsBack );
}

void SoftkeyStandardConfiguration_OkClear( void )
{
	LeftSoftkeyConfigurationWithInstructionText( VKEY_OK, szInsOK );
	RightSoftkeyConfigurationWithInstructionText( VKEY_CLEAR, szInsClear );
}

void SoftkeyStandardConfiguration_Back( void )
{
	RightSoftkeyConfigurationWithInstructionText( VKEY_CANCEL, szInsBack );
}

void SoftkeyStandardConfiguration_YesNo( void )
{
	LeftSoftkeyConfigurationWithInstructionText( VKEY_OK, szInsYes );
	RightSoftkeyConfigurationWithInstructionText( VKEY_CANCEL, szInsNo );
}

void SoftkeyStandardConfiguration_DetailBack( void )
{
	LeftSoftkeyConfigurationWithInstructionText( VKEY_OK, szInsDetail );
	RightSoftkeyConfigurationWithInstructionText( VKEY_CANCEL, szInsBack );
}

#endif /* SOFTKEY_SUPPORT */
