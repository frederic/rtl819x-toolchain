#include <stdio.h>
#include "ui_config.h"
#include "gs_font.h"
#include "res_font.h"

#ifndef _TEXT_MODE

static const unsigned char * const ppLstEngFont[] = {
#if CHAR_WIDTH == 5 && CHAR_HEIGHT == 8 && defined( LCD_COL_ORIENTED )
	ASCII_TAB_Eng_5x8_col,
	ASCII_TAB_Eng_5x8_col,	// to be modify 
#elif CHAR_WIDTH == 8 && CHAR_HEIGHT == 16 && defined( LCD_COL2_ORIENTED )
	ASCII_TAB_Eng_8x16_col2,
	ASCII_TAB_Eng_Nx16_col2,
#elif CHAR_WIDTH == 8 && CHAR_HEIGHT == 16 && defined( LCD_ROW_ORIENTED )
	ASCII_TAB_Eng_8x16_row,
	ASCII_TAB_Eng_8x16_row,	// to be modify 
#endif
};

#ifdef LANG_BIG5
static const unsigned char * const ppLstChnFont[] = {
#if CHAR_WIDTH == 8 && CHAR_HEIGHT == 16 && defined( LCD_COL2_ORIENTED )
	Chinese_big5_16x16_col2,
	Chinese_big5_14x14_col2,
#else
	???
#endif
};
#elif defined( LANG_GB2312 )
#if CHAR_WIDTH == 8 && CHAR_HEIGHT == 16 && defined( LCD_COL2_ORIENTED )
static const unsigned char * const ppLstChnFont[] = {
	Chinese_gb2312_16x16_col2,
	Chinese_gb2312_14x14_col2,
#else
	???
#endif
};
#endif

#if CHAR_WIDTH == 5 && CHAR_HEIGHT == 8 && defined( LCD_COL_ORIENTED )
static const unsigned char pQuestionSignSymbol[] = {0x02,0x01,0x51,0x09,0x06,};  //3F '?'
static const int nQuestionSignSymbolWidth = 5;
static const int nQuestionSignSymbolHeight = 8;
#elif CHAR_WIDTH == 8 && CHAR_HEIGHT == 16 && defined( LCD_COL2_ORIENTED )
static const unsigned char pQuestionSignSymbol[] = {
	0x00, 0x38, 0x04, 0x04, 0x84, 0x48, 0x30, 0x00, 0x00, 0x00, 0x00, 0x36, 0x01, 0x00, 0x00, 0x00, /* 3F ? */
};
static const int nQuestionSignSymbolWidth = 8;
static const int nQuestionSignSymbolHeight = 16;
#elif CHAR_WIDTH == 8 && CHAR_HEIGHT == 16 && defined( LCD_ROW_ORIENTED )
#else
	???
#endif

#define NUM_OF_ENG_FONT_LIST		( sizeof( ppLstEngFont ) / sizeof( ppLstEngFont[ 0 ] ) )
CT_ASSERT( NUM_OF_ENG_FONT_LIST == NUM_OF_ENG_FONT_ID );

#if defined( LANG_BIG5 ) || defined( LANG_GB2312 )
#define NUM_OF_CHN_FONT_LIST		( sizeof( ppLstChnFont ) / sizeof( ppLstChnFont[ 0 ] ) )
CT_ASSERT( NUM_OF_CHN_FONT_LIST == NUM_OF_CHN_FONT_ID );
#endif

static struct {
	union {
		eng_font_id_t idEng;
		chn_font_id_t idChn;
	};
	const font_t * pFont;			/* point to whole font array */
	const unsigned char *pBits;		/* point to bitmap part */
	const nFontOffset_t *pOffset;	/* point to font offset array (for variable width only) */
	int nHeightBytes;				/* bytes of height (for variable width only) */
} engFont, chnFont;

void GS_InitializeFont( void )
{
	GS_SelectEngFont( ENG_FONT_ID_NORMAL );
	GS_SelectChnFont( CHN_FONT_ID_NORMAL );
}

eng_font_id_t GS_SelectEngFont( eng_font_id_t id )
{
	eng_font_id_t old_id;

	old_id = engFont.idEng;

	if( id >= NUM_OF_ENG_FONT_ID )
		id = ENG_FONT_ID_NORMAL;

	engFont.pFont = ( const font_t * )ppLstEngFont[ id ];
	engFont.idEng = id;
	engFont.nHeightBytes = ( engFont.pFont ->height + 7 ) >> 3;

	if( ( engFont.pFont ->flag & FONT_FLAG_WIDTH_MASK ) == FONT_FLAG_WIDTH_FIXED ) {
		engFont.pOffset = NULL;
		engFont.pBits = &engFont.pFont ->eng.bitmap[ 0 ];
	} else {
		engFont.pOffset = &engFont.pFont ->eng_var.offset[ 0 ];
		engFont.pBits = ( const unsigned char * )&engFont.pFont ->eng_var.offset[ engFont.pFont ->words ];
	}

	return old_id;
}

#if defined( LANG_BIG5 ) || defined( LANG_GB2312 )
chn_font_id_t GS_SelectChnFont( chn_font_id_t id )
{
	chn_font_id_t old_id;

	old_id = chnFont.idChn;

	if( id >= NUM_OF_CHN_FONT_ID )
		id = CHN_FONT_ID_NORMAL;

	chnFont.pFont = ( const font_t * )ppLstChnFont[ id ];
	chnFont.idChn = id;
	chnFont.nHeightBytes = ( chnFont.pFont ->height + 7 ) >> 3;

	chnFont.pOffset = NULL;
#ifdef LANG_BIG5
	chnFont.pBits = ( const unsigned char * )&chnFont.pFont ->big5.bitmap[ 0 ];
#elif defined( LANG_GB2312 )
	chnFont.pBits = ( const unsigned char * )&chnFont.pFont ->gb2312.bitmap[ 0 ];
#else
	???
#endif

	return old_id;
}
#endif /* LANG_BIG5 || LANG_GB2312 */

static const unsigned char *GS_GetEngFontBitmap( unsigned char ch, int *pnWidth, int *pnHeight )
{
	short delta;
	nFontOffset_t offset;

	if( engFont.pFont == NULL ) {
		debug_out( "Font is not initialized!!\n" );
		goto label_use_question_sign_symbol;
	}

	delta = ( short )ch - ( short )engFont.pFont ->eng.start_ascii;

	/* check font support range */
	if( delta < 0 )
		goto label_out_of_range;

	if( engFont.pFont ->eng.start_ascii + engFont.pFont ->words <= ch )
		goto label_out_of_range;

	/* get font bitmap */
	if( ( engFont.pFont ->flag & FONT_FLAG_WIDTH_MASK ) == FONT_FLAG_WIDTH_FIXED ) {
		/* fixed width font */
		*pnWidth = engFont.pFont ->width;
		*pnHeight = engFont.pFont ->height;
		return &engFont.pBits[ delta * engFont.pFont ->bytes ];
	} else {
		/* variable width font */
		offset = ( delta == 0 ? 0 : engFont.pOffset[ delta - 1 ] );

		*pnWidth = ( engFont.pOffset[ delta ] - offset ) / engFont.nHeightBytes;
		*pnHeight = engFont.pFont ->height;
		return &engFont.pBits[ offset ];
	}

label_out_of_range:
label_use_question_sign_symbol:

	*pnWidth = nQuestionSignSymbolWidth;
	*pnHeight = nQuestionSignSymbolHeight;
	return pQuestionSignSymbol;
}

#ifdef LANG_BIG5 
static const unsigned char *GS_GetChnFontBitmap( const unsigned char *pch, int *pnWidth, int *pnHeight )
{
#define NUM_OF_TAB_BIG5		( sizeof( tabBig5 ) / sizeof( tabBig5[ 0 ] ) )

	static const struct {
		unsigned short chStart;
		unsigned short chEnd;
		unsigned short nChNum;
		big5_segment maskSeg;
	} tabBig5[] = {
		{ 0x8140, 0xA0FE, 5024, BIG5_SEG0_USER1 },		/* 0x8140-0xA0FE (5024) 保留給使用者自定義字元（造字區） */
		{ 0xA140, 0xA3BF, 408, BIG5_SEG1_SYMBOL },		/* 0xA140-0xA3BF (408)  標點符號、希臘字母及特殊符號，包括在0xA259-0xA261，安放了雙音節度量衡單位用字：兙兛兞兝兡兣嗧瓩糎。 */
		{ 0xA3C0, 0xA3FE, 63, BIG5_SEG2_RESERVED },		/* 0xA3C0-0xA3FE (63)   保留。此區沒有開放作造字區用。 */
		{ 0xA440, 0xC67E, 5401, BIG5_SEG3_COMMON1 },	/* 0xA440-0xC67E (5401) 常用漢字，先按筆劃再按部首排序。 */
		{ 0xC6A1, 0xC8FE, 408, BIG5_SEG4_USER2 },		/* 0xC6A1-0xC8FE (408)  保留給使用者自定義字元（造字區） */
		{ 0xC940, 0xF9D5, 7652, BIG5_SEG5_COMMON2 },	/* 0xC940-0xF9D5 (7652) 次常用漢字，亦是先按筆劃再按部首排序。 */
		{ 0xF9D6, 0xFEFE, 826, BIG5_SEG6_USER3 },		/* 0xF9D6-0xFEFE (826)  保留給使用者自定義字元（造字區） */
	};

	unsigned char segment;
	unsigned char hi, lo;
	unsigned char lo2;
	unsigned short wch;
	unsigned short offset;
	unsigned short t;
	int i;

	/* 「高位位元組」使用了0x81-0xFE，「低位位元組」使用了0x40-0x7E，及0xA1-0xFE。 */
	hi = *pch;
	lo = *( pch + 1 );

	if( hi < 0x81 || hi > 0xFE )
		goto label_not_big5_character;

	if( lo < 0x40 || ( lo > 0x7E && lo < 0xA1 ) || lo > 0xFE )
		goto label_not_big5_character;

	/* decide offset */
	segment = chnFont.pFont ->big5.segment;
	wch = ( ( unsigned short )hi << 8 ) | lo;
	offset = 0;

	for( i = 0; i < NUM_OF_TAB_BIG5; i ++ ) {
		if( tabBig5[ i ].chStart <= wch && wch <= tabBig5[ i ].chEnd ) {
			/* ok. found! */
			if( ( segment & tabBig5[ i ].maskSeg ) == 0 ) {
				/* but no this bitmap */
				goto label_this_font_has_no_this_bitmap;
			} else {
				/* calculate offset */
				t = ( wch - tabBig5[ i ].chStart ) >> 8;
				offset += t * ( ( 0x7e - 0x40 + 1 ) + ( 0xfe - 0xa1 + 1 ) );

				lo2 = tabBig5[ i ].chStart & 0xFF;

				if( lo > lo2 ) {
					if( lo > 0x7e && lo2 <= 0x7e )
						offset += ( 0x7e - lo2 + 1 ) + ( lo - 0xa1 + 1 ) - 1;
					else
						offset += ( lo - lo2 + 1 ) - 1;
				} else if( lo < lo2 ) {
					if( lo <= 0x7e && lo2 > 0x7e )
						offset += ( 0xfe - lo2 + 1 ) + ( lo - 0x40 + 1 ) - 1;
					else 
						offset += ( 0x7e - 0x40 + 1 ) + ( 0xfe - 0xa1 + 1 ) - ( lo2 - lo + 1 ) - 1;
				}

				goto label_extract_this_character;
			}
		}

		if( segment & tabBig5[ i ].maskSeg )
			offset += tabBig5[ i ].nChNum;
	}

	debug_out( "An unexpected big5(%02X%02X)\n", hi, lo );
	return NULL;

label_extract_this_character:
	*pnWidth = chnFont.pFont ->width;
	*pnHeight = chnFont.pFont ->height;
	return &chnFont.pBits[ offset * chnFont.pFont ->bytes ];;

label_this_font_has_no_this_bitmap:
	*pnWidth = nQuestionSignSymbolWidth;
	*pnHeight = nQuestionSignSymbolHeight;
	return pQuestionSignSymbol;

label_not_big5_character:
	return NULL;

#undef NUM_OF_TAB_BIG5
}
#endif /* LANG_BIG5 */

#ifdef LANG_GB2312
static const unsigned char *GS_GetChnFontBitmap( const unsigned char *pch, int *pnWidth, int *pnHeight )
{
#define NUM_OF_TAB_GB2312		( sizeof( tabGB2312 ) / sizeof( tabGB2312[ 0 ] ) )

	static const struct {
		unsigned short chStart;
		unsigned short chEnd;
		unsigned short nChNum;
		gb2312_segment maskSeg;
	} tabGB2312[] = {
		{ 0xA1A1, 0xA9FE, 94 * 9, GB2312_SEG0_SYMBOL },		/* 0xA1A1-0xA9FE 01-09區為特殊符號 */
		{ 0xAAA1, 0xAFFE, 94 * 6, GB2312_SEG1_UNUSED1 },	/* 0xAAA1-0xAFFE 10-15區未有編碼 */
		{ 0xB0A1, 0xD7FE, 94 * 40, GB2312_SEG2_COMMON1 },	/* 0xB0A1-0xD7FE 16-55區為一級漢字，按拼音排序 */
		{ 0xD8A1, 0xF7FE, 94 * 32, GB2312_SEG3_COMMON2 },	/* 0xD8A1-0xF7FE 56-87區為二級漢字，按部首/筆畫排序 */
		{ 0xF8A1, 0xFEFE, 94 * 7, GB2312_SEG4_UNUSED2 },	/* 0xF8A1-0xFEFE 88-94區則未有編碼 */
	};

	unsigned char segment;
	unsigned char hi, lo;
	unsigned char lo2;
	unsigned short wch;
	unsigned short offset;
	unsigned short t;
	int i;

	/* 「高位位元組」使用了0xA1-0xF7(把01-87區的區號加上0xA0)，「低位位元組」使用了0xA1-0xFE(把01-94加上0xA0)。 */
	hi = *pch;
	lo = *( pch + 1 );

	if( hi < 0xA1 || hi > 0xFE )
		goto label_not_gb2312_character;

	if( lo < 0xA1 || lo > 0xFE )
		goto label_not_gb2312_character;

	/* decide offset */
	segment = chnFont.pFont ->gb2312.segment;
	wch = ( ( unsigned short )hi << 8 ) | lo;
	offset = 0;

	for( i = 0; i < NUM_OF_TAB_GB2312; i ++ ) {
		if( tabGB2312[ i ].chStart <= wch && wch <= tabGB2312[ i ].chEnd ) {
			/* ok. found! */
			if( ( segment & tabGB2312[ i ].maskSeg ) == 0 ) {
				/* but no this bitmap */
				goto label_this_font_has_no_this_bitmap;
			} else {
				/* calculate offset */
				t = ( wch - tabGB2312[ i ].chStart ) >> 8;
				offset += t * ( 0xfe - 0xa1 + 1 );

				lo2 = tabGB2312[ i ].chStart & 0xFF;

				if( lo > lo2 ) {
					offset += ( lo - lo2 + 1 ) - 1;
				} else if( lo < lo2 ) {
					offset += ( 0xfe - lo2 + 1 ) + ( lo - 0xa1 + 1 ) - 1;
				}

				goto label_extract_this_character;
			}
		}

		if( segment & tabGB2312[ i ].maskSeg )
			offset += tabGB2312[ i ].nChNum;
	}

	debug_out( "An unexpected GB2312(%02X%02X)\n", hi, lo );
	return NULL;

label_extract_this_character:
	*pnWidth = chnFont.pFont ->width;
	*pnHeight = chnFont.pFont ->height;
	return &chnFont.pBits[ offset * chnFont.pFont ->bytes ];;

label_this_font_has_no_this_bitmap:
	*pnWidth = nQuestionSignSymbolWidth;
	*pnHeight = nQuestionSignSymbolHeight;
	return pQuestionSignSymbol;

label_not_gb2312_character:
	return NULL;

#undef NUM_OF_TAB_GB2312
}
#endif /* LANG_GB2312 */

const unsigned char *GS_GetFontBitmap( const unsigned char *pch, int *pnCh, int *pnWidth, int *pnHeight )
{
#if defined( LANG_BIG5 ) || defined( LANG_GB2312 )
	const unsigned char *pbits;
#endif

	/* try Chinese font */
#if defined( LANG_BIG5 ) || defined( LANG_GB2312 )
	if( ( pbits = GS_GetChnFontBitmap( pch, pnWidth, pnHeight ) ) ) {
		*pnCh = 2;
		return pbits;
	}
#endif

	/* try English font */
	*pnCh = 1;

	return GS_GetEngFontBitmap( *pch, pnWidth, pnHeight );
}

void GS_GetEngFontMaxSize( int *pWidth, int *pHeight )
{
	if( engFont.pFont == NULL ) {
		*pWidth = 0;
		*pHeight = 0;
		debug_out( "Select an English font first!!\n" );
		return;
	}

	*pWidth = engFont.pFont ->width;
	*pHeight = engFont.pFont ->height;
}

#if defined( LANG_BIG5 ) || defined( LANG_GB2312 )
void GS_GetChnFontMaxSize( int *pWidth, int *pHeight )
{
	if( chnFont.pFont == NULL ) {
		*pWidth = 0;
		*pHeight = 0;
		debug_out( "Select a Chinese font first!!\n" );
		return;
	}

	*pWidth = chnFont.pFont ->width;
	*pHeight = chnFont.pFont ->height;
}
#endif /* LANG_BIG5 || LANG_GB2312 */

void GS_GetFontMaxSize( int *pWidth, int *pHeight )
{
#if defined( LANG_BIG5 ) || defined( LANG_GB2312 )
	int width, height;
#endif

	GS_GetEngFontMaxSize( pWidth, pHeight );

#if defined( LANG_BIG5 ) || defined( LANG_GB2312 )
	GS_GetChnFontMaxSize( &width, &height );

	if( *pWidth < width )
		*pWidth = width;

	if( *pHeight < height )
		*pHeight = height;
#endif
}

#endif /* !_TEXT_MODE */
