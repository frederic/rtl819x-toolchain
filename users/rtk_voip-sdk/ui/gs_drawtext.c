#include <string.h>
#include <stdio.h>
#include "ui_config.h"
#include "gs_vram.h"
#include "gs_font.h"
#include "gs_basic.h"
#include "gs_def.h"
#include "gs_drawimage.h"
#include "ui_softkey.h"


/* ======================================================================= */
/* Graphic Mode */
#ifndef _TEXT_MODE
//  #ifdef LCD_COL_ORIENTED
//static inline void GS_DrawText_Col( int x, int y, const unsigned char *pszText, int len )
//{
//	int fontWidth, fontHeight;
//	const unsigned char *pBitmap;
//#if 0
//	int fontHeightByte;
//	unsigned char *pvram;
//	int mode;
//	int i;
//#endif
//	int k;
//	int nch;
//
//	GS_GetFontMaxSize( &fontWidth, &fontHeight );
//
//	if( y >= VRAM_HEIGHT || y + fontHeight <= 0 )		/* out of range */ 
//		return;
//
//	for( k = 0; k < len; k += nch ) {
//
//		pBitmap = GS_GetFontBitmap( pszText, &nch, &fontWidth, &fontHeight );
//
//#if 1
//		GS_DrawImage_1bpp( x, y, pBitmap, fontWidth, fontHeight );
//
//		x += fontWidth;
//		pszText += nch;
//#else
//		fontHeightByte = ( fontHeight + 7 ) >> 3;	/* ( fontHeight + 7 ) / 8 */
//
//		if( ( y & 7 ) == 0 ) {		/* y % 8 == 0 */
//			if( ( fontHeight & 7 ) == 0 )
//				mode = 0;
//			else 
//				mode = 1;
//		} else {
//			if( ( fontHeight & 7 ) == 0 )
//				mode = 2;
//			else 
//				mode = 3;		
//		}
//
//		if( x + fontWidth <= 0 ) 
//			goto label_skip_this_character;
//		else 
//			pvram = &vRam.pixels[ x * VRAM_HEIGHT_BYTES + ( y >> 3 ) ];
//
//		if( mode == 0 ) {
//
//			// draw a font 
//			for( i = 0; i < fontWidth; i ++ ) {
//				if( x < 0 )
//					goto label_skip_this_col;
//
//				if( fontHeight == 8 ) {
//					*pvram = *pBitmap;
//
//				} else {
//					debug_out( "fontHeight is not 8\n" );
//				}
//
//label_skip_this_col:
//				x ++;
//				pvram += VRAM_HEIGHT_BYTES;
//				pBitmap += fontHeightByte;
//			}
//
//		} else {
//			debug_out( "Under developement\n" );
//		}
//
//label_skip_this_character:
//		pszText ++;
//#endif
//	} /* each word */
//}
//  #endif /* LCD_COL_ORIENTED */

/* ----------------------------------------------------------------------- */
//  #ifdef LCD_COL2_ORIENTED
//static inline void GS_DrawText_Col_Type2( int x, int y, const unsigned char *pszText, int len )
//{
//	int fontWidth, fontHeight;
//	const unsigned char *pBitmap;
//#if 0
//	int fontHeightByte;
//	unsigned char *pvram;
//	int mode;
//	int i, offset;
//	unsigned char mask;
//	int yt;
//#endif
//	int k;
//	int nch;
//
//	GS_GetFontMaxSize( &fontWidth, &fontHeight );
//
//	if( y >= VRAM_HEIGHT || y + fontHeight <= 0 )		/* out of range */ 
//		return;
//
//	for( k = 0; k < len; k += nch ) {
//
//		pBitmap = GS_GetFontBitmap( pszText, &nch, &fontWidth, &fontHeight );
//
//#if 1
//		GS_DrawImage_1bpp( x, y, pBitmap, fontWidth, fontHeight );
//
//		x += fontWidth;
//		pszText += nch;
//#else
//		fontHeightByte = ( fontHeight + 7 ) >> 3;	/* ( fontHeight + 7 ) / 8 */
//
//		if( ( y & 7 ) == 0 ) {		/* y % 8 == 0 */
//			if( ( fontHeight & 7 ) == 0 )
//				mode = 0;
//			else 
//				mode = 1;
//		} else {
//			if( ( fontHeight & 7 ) == 0 )
//				mode = 2;
//			else 
//				mode = 3;		
//		}
//
//		if( x + fontWidth <= 0 ) 
//			goto label_skip_this_character;
//		else if( x >= VRAM_WIDTH )
//			break;	/* out of right boundary */
//		else 
//			pvram = &vRam.pixels[ x + ( y >> 3 ) * VRAM_WIDTH ];
//
//		/* draw a font */ 
//		if( mode == 0 ) {
//
//			yt = y;
//
//			for( i = 0; i < fontHeightByte; i ++ ) {
//
//				if( yt < 0 || yt + 8 >= VRAM_HEIGHT )
//					goto label_mode0_skip_this_page;
//
//				if( x < 0 )		/* head part out of left boundary */
//					memcpy( pvram + ( -x ), pBitmap + ( -x ), fontWidth - ( -x ) );
//				else if( ( offset = VRAM_WIDTH - ( x + fontWidth ) ) < 0 )	/* tail part out of right boundary */
//					memcpy( pvram, pBitmap, fontWidth - ( -offset ) );
//				else
//					memcpy( pvram, pBitmap, fontWidth );
//
//label_mode0_skip_this_page:
//				pvram += VRAM_WIDTH;
//				pBitmap += fontWidth;
//				yt += 8;
//			}
//
//		} else if( mode == 1 ) {
//
//			yt = y;
//
//			for( i = 0; i < fontHeightByte - 1; i ++ ) {
//
//				if( yt < 0 || yt + 8 >= VRAM_HEIGHT )
//					goto label_mode1_skip_this_page;
//				
//				if( x < 0 )		/* head part out of left boundary */
//					memcpy( pvram + ( -x ), pBitmap + ( -x ), fontWidth - ( -x ) );
//				else if( ( offset = VRAM_WIDTH - ( x + fontWidth ) ) < 0 )	/* tail part out of right boundary */
//					memcpy( pvram, pBitmap, fontWidth - ( -offset ) );
//				else
//					memcpy( pvram, pBitmap, fontWidth );
//
//label_mode1_skip_this_page:
//				pvram += VRAM_WIDTH;
//				pBitmap += fontWidth;
//				yt += 8;
//			}
//
//			/* draw the last page */
//			mask = bitsMask8[ ( fontHeight & 7 ) - 1 ];	/* Now, (fontHeight & 7 != 0) */
//
//			if( x < 0 )		/* head part out of left boundary */
//				memcpy_mask8( pvram + ( -x ), pBitmap + ( -x ), fontWidth - ( -x ), mask );
//			else if( ( offset = VRAM_WIDTH - ( x + fontWidth ) ) < 0 )	/* tail part out of right boundary */
//				memcpy_mask8( pvram, pBitmap, fontWidth - ( -offset ), mask );
//			else
//				memcpy_mask8( pvram, pBitmap, fontWidth, mask );
//
//		} else if( mode == 2 || mode == 3 ) {
//			/* 
//			 * mode 3 is very similar to mode 2, excepting to font height. 
//			 * In mode 2, we use OR operation to draw bitmap, so mode 3 can use same function. 
//			 */
//
//			yt = y;
//
//			for( i = 0; i < fontHeightByte; i ++ ) {
//				if( x < 0 ) {		/* head part out of left boundary */
//					if( yt >= 0 )
//						memcpy_sh8( pvram + ( -x ), pBitmap + ( -x ), fontWidth - ( -x ), yt & 7 );
//					if( yt + 8 > 0 && yt + 8 < VRAM_HEIGHT )
//						memcpy_sh8( pvram + ( -x ) + VRAM_WIDTH, pBitmap + ( -x ), fontWidth - ( -x ), ( yt & 7 ) - 8 );
//				} else if( ( offset = VRAM_WIDTH - ( x + fontWidth ) ) < 0 ) {	/* tail part out of right boundary */
//					if( yt >= 0 )
//						memcpy_sh8( pvram, pBitmap, fontWidth - ( -offset ), yt & 7 );
//					if( yt + 8 > 0 && yt + 8 < VRAM_HEIGHT )
//						memcpy_sh8( pvram + VRAM_WIDTH, pBitmap, fontWidth - ( -offset ), ( yt & 7 ) - 8 );
//				} else {
//					if( yt >= 0 )
//						memcpy_sh8( pvram, pBitmap, fontWidth, yt & 7 );
//					if( yt + 8 > 0 && yt + 8 < VRAM_HEIGHT )
//						memcpy_sh8( pvram + VRAM_WIDTH, pBitmap, fontWidth, ( yt & 7 ) - 8 );
//				}
//
//				pvram += VRAM_WIDTH;
//				pBitmap += fontWidth;
//				yt += 8;
//
//				if( yt >= VRAM_HEIGHT )
//					goto label_skip_this_character;
//			}
//
//		} else {
//			debug_out( "Under developement\n" );
//		}
//
//label_skip_this_character:
//		x += fontWidth;
//		pszText += nch;
//#endif
//	} /* each word */
//}
//  #endif /* LCD_COL2_ORIENTED */

/* ----------------------------------------------------------------------- */
//  #ifdef LCD_ROW_ORIENTED
//static inline void GS_DrawText_Row( int x, int y, const unsigned char *pszText, int len )
//{
//	int fontWidth, fontHeight;
//	int fontWidthByte;
//	const unsigned char *pBitmap;
//	unsigned char *pvram;
//	int mode;
//	int i, k;
//	int lcd_y;
//	int nch;
//
//	GS_GetFontMaxSize( &fontWidth, &fontHeight );
//
//	if( y >= VRAM_HEIGHT || y + fontHeight <= 0 )		/* out of range */ 
//		return;
//
//	for( k = 0; k < len; k += nch ) {
//
//		pBitmap = GS_GetFontBitmap( pszText, &nch, &fontWidth, &fontHeight );
//
//		fontWidthByte = ( fontWidth + 7 ) >> 3;	/* ( fontWidth + 7 ) / 8 */
//
//		if( ( x & 7 ) == 0 ) {		/* x % 8 == 0 */
//			if( ( fontWidth & 7 ) == 0 )
//				mode = 0;
//			else 
//				mode = 1;
//		} else {
//			if( ( fontWidth & 7 ) == 0 )
//				mode = 2;
//			else 
//				mode = 3;		
//		}
//
//		if( x + fontWidth <= 0 ) 
//			goto label_skip_this_character;
//		else if( x >= VRAM_WIDTH )
//			break;	/* out of right boundary */
//		else 
//			pvram = &vRam.pixels[ ( x >> 3 ) + y * VRAM_WIDTH_BYTES ];
//
//		lcd_y = y;
//
//		if( mode == 0 ) {
//
//			/* draw a font */ 
//			for( i = 0; i < fontHeight; i ++ ) {
//				if( lcd_y < 0 )
//					goto label_skip_this_col;
//
//				if( fontWidth == 8 ) {
//					*pvram = *pBitmap;
//
//				} else {
//					debug_out( "fontWidth is not 8\n" );
//				}
//
//label_skip_this_col:
//				lcd_y ++;
//				pvram += VRAM_WIDTH_BYTES;
//				pBitmap += fontWidthByte;
//			}
//
//		} else {
//			debug_out( "Under developement\n" );
//		}
//
//label_skip_this_character:
//		x += fontWidth;
//		pszText ++;
//	} /* each word */
//}
//  #endif /* LCD_ROW_ORIENTED */

static inline void GS_DrawText_Core( int x, int y, const unsigned char *pszText, int len )
{
	int fontWidth, fontHeight;
	const unsigned char *pBitmap;
	int k;
	int nch;

	GS_GetFontMaxSize( &fontWidth, &fontHeight );

	if( y >= VRAM_HEIGHT || y + fontHeight <= 0 )		/* out of range */ 
		return;

	for( k = 0; k < len; k += nch ) {

		pBitmap = GS_GetFontBitmap( pszText, &nch, &fontWidth, &fontHeight );

		GS_DrawImage_1bpp( x, y, pBitmap, fontWidth, fontHeight );

		x += fontWidth;
		pszText += nch;
	}
}

void GS_GetTextWidthHeight( const unsigned char *pszText, int len, int *pTextWidth, int *pTextHeight )
{
	int nTotalWidth = 0, nMaxHeight = 0;
	int i;
	int nch, fontWidth, fontHeight;
	const unsigned char *pBitmap;

	for( i = 0; i < len; ) {
		pBitmap = GS_GetFontBitmap( pszText, &nch, &fontWidth, &fontHeight );

		/* width and height calculation */
		nTotalWidth += fontWidth;
		nMaxHeight = ( nMaxHeight < fontHeight ? fontHeight : nMaxHeight );

		/* post modifier */
		pszText += nch;
		i += nch;
	}

	if( pTextWidth )
		*pTextWidth = nTotalWidth;

	if( pTextHeight )
		*pTextHeight = nMaxHeight;
}

int GS_GetTextLengthToFitWidth( const unsigned char *pszText, int len, int width )
{
	int nTotalWidth = 0;
	int i;
	int nch, fontWidth, fontHeight;
	const unsigned char *pBitmap;

	for( i = 0; i < len; ) {
		pBitmap = GS_GetFontBitmap( pszText, &nch, &fontWidth, &fontHeight );

		/* width and height calculation */
		nTotalWidth += fontWidth;

		if( nTotalWidth > width )
			break;

		/* post modifier */
		pszText += nch;
		i += nch;
	}

	return i;
}

int GS_GetLastNchOfTextString( const unsigned char *pszText, int len )
{
	int i;
	int nch, fontWidth, fontHeight;

	if( len <= 0 )
		return 0;

	for( i = 0; i < len; ) {
		GS_GetFontBitmap( pszText, &nch, &fontWidth, &fontHeight );

		/* post modifier */
		pszText += nch;
		i += nch;
	}

	return nch;
}

void GS_DrawText( int x, int y, const unsigned char *pszText, int len )
{
#if defined( LANG_BIG5 ) || defined( LANG_GB2312 )
	int textWidth, textHeight;
#else
	int fontWidth, fontHeight;
#endif
	rect_t rect;


#if 1
	GS_DrawText_Core( x, y, pszText, len );
#else
//#ifdef LCD_COL_ORIENTED
//	GS_DrawText_Col( x, y, pszText, len );
//#elif defined( LCD_COL2_ORIENTED )
//	GS_DrawText_Col_Type2( x, y, pszText, len );
//#elif defined( LCD_ROW_ORIENTED )
//	GS_DrawText_Row( x, y, pszText, len );
//#endif
#endif

	/* output to LCD */
	if( !fDrawOffScreen.all ) {

#if defined( LANG_BIG5 ) || defined( LANG_GB2312 )
		GS_GetTextWidthHeight( pszText, len, &textWidth, &textHeight );

		rect.left = x; 
		rect.right = rect.left + textWidth;
		rect.top = y; 
		rect.bottom = rect.top + textHeight;
#else
		GS_GetEngFontMaxSize( &fontWidth, &fontHeight );

		rect.left = x;
		rect.top = y;
		rect.right = x + len * fontWidth;
		rect.bottom = y + fontHeight;
#endif
		GS_VRamToLcdDevice( &rect );
	}
}

void GS_TextOut( int x, int y, const unsigned char *pszText, int len )
{
	GS_DrawText( x * CHAR_WIDTH, y * CHAR_HEIGHT, pszText, len );
}

void GS_TextOutInCenter( int y, const unsigned char *pszText )
{
	int len;
	int width;

	len = strlen( ( char * )pszText );

	GS_GetTextWidthHeight( pszText, len, &width, NULL );

	GS_DrawText( ( VRAM_WIDTH - width ) / 2, y * CHAR_HEIGHT, pszText, len );
}
#else
/* ======================================================================= */
/* Text Mode */
void GS_TextOut( int x, int y, const unsigned char *pszText, int len )
{
	rect_t rect;
	int abs_x, index;
	
	if( x >= VRAM_WIDTH || y >= VRAM_HEIGHT || y < 0 || len <= 0 )
		return;
		
	if( x < 0 ) {
		abs_x = x * ( -1 );
		
		if( abs_x <= len )		/* too left */
			return;

		x = 0;
		pszText += abs_x;
		len -= abs_x;
	}
	
	if( len > VRAM_WIDTH )		/* too long */
		len = VRAM_WIDTH;
	
	index = x + y * VRAM_WIDTH;
	
	/* write to vRAM */
	while( len -- )
		vRam.ch[ index ++ ] = *pszText ++;
	
	/* output to LCD */
	if( !fDrawOffScreen.all ) {
		// FIXME: fill rect
		GS_VRamToLcdDevice( &rect );
	}
}

void GS_TextOutInCenter( int y, const unsigned char *pszText )
{
	int len;
	int x;

	len = strlen( ( char * )pszText );

	if( len >= VRAM_WIDTH_IN_TEXT_UNIT )
		x = 0;
	else
		x = ( VRAM_WIDTH_IN_TEXT_UNIT - len ) / 2;

	GS_TextOut( x, y, pszText, len );
}
#endif /* !_TEXT_MODE */
