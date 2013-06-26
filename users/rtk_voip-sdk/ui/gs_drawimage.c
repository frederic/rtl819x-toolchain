#include <string.h>
#include <stdio.h>
#include "ui_config.h"
#include "gs_vram.h"
#include "gs_basic.h"
#include "gs_def.h"
#include "res_image.h"
#include "gs_drawimage.h"

#ifndef _TEXT_MODE

#ifdef LCD_COL_ORIENTED
static inline void GS_DrawImage_1bpp_Col( int x, int y, const unsigned char *pbits, int cx, int cy )
{
	unsigned char *pvram;
	int cyByte;
	int mode;
	int i;

	if( x + cx <= 0 || x >= VRAM_WIDTH )		/* out of range */ 
		return;

	if( y >= VRAM_HEIGHT || y + cy <= 0 )		/* out of range */ 
		return;

	cyByte = ( cy + 7 ) >> 3;	/* ( cy + 7 ) / 8 */

	if( ( y & 7 ) == 0 ) {		/* y % 8 == 0 */
		if( ( cy & 7 ) == 0 )
			mode = 0;
		else 
			mode = 1;
	} else {
		if( ( cy & 7 ) == 0 )
			mode = 2;
		else 
			mode = 3;		
	}

	pvram = &vRam.pixels[ x * VRAM_HEIGHT_BYTES + ( y >> 3 ) ];

	if( mode == 0 ) {

		// draw a font 
		for( i = 0; i < cx; i ++ ) {
			if( x < 0 )
				goto label_skip_this_col;

			if( cy == 8 ) {
				*pvram = *pbits;

			} else {
				debug_out( "cy is not 8\n" );
			}

label_skip_this_col:
			x ++;
			pvram += VRAM_HEIGHT_BYTES;
			pbits += cyByte;
		}

	} else {
		debug_out( "Under developement\n" );
	}
}
#endif /* LCD_COL_ORIENTED */

#ifdef LCD_COL2_ORIENTED
static inline void GS_DrawImage_1bpp_Col_Type2( int x, int y, const unsigned char *pbits, int cx, int cy )
{
	unsigned char *pvram;
	int cyByte;
	int yt;
	int mode;
	int i;
	int offset;
	unsigned char mask;

	if( x + cx <= 0 || x >= VRAM_WIDTH )		/* out of range */ 
		return;

	if( y >= VRAM_HEIGHT || y + cy <= 0 )		/* out of range */ 
		return;

	cyByte = ( cy + 7 ) >> 3;	/* ( cy + 7 ) / 8 */

	if( ( y & 7 ) == 0 ) {		/* y % 8 == 0 */
		if( ( cy & 7 ) == 0 )
			mode = 0;
		else 
			mode = 1;
	} else {
		if( ( cy & 7 ) == 0 )
			mode = 2;
		else 
			mode = 3;		
	}

	pvram = &vRam.pixels[ x + ( y >> 3 ) * VRAM_WIDTH ];

	/* draw a image */ 
	if( mode == 0 ) {

		yt = y;

		for( i = 0; i < cyByte; i ++ ) {

			if( yt < 0 || yt + 8 >= VRAM_HEIGHT )
				goto label_mode0_skip_this_page;

			if( x < 0 )		/* head part out of left boundary */
				memcpy( pvram + ( -x ), pbits + ( -x ), cx - ( -x ) );
			else if( ( offset = VRAM_WIDTH - ( x + cx ) ) < 0 )	/* tail part out of right boundary */
				memcpy( pvram, pbits, cx - ( -offset ) );
			else
				memcpy( pvram, pbits, cx );

label_mode0_skip_this_page:
			pvram += VRAM_WIDTH;
			pbits += cx;
			yt += 8;
		}

	} else if( mode == 1 ) {

		yt = y;

		for( i = 0; i < cyByte - 1; i ++ ) {

			if( yt < 0 || yt + 8 >= VRAM_HEIGHT )
				goto label_mode1_skip_this_page;
			
			if( x < 0 )		/* head part out of left boundary */
				memcpy( pvram + ( -x ), pbits + ( -x ), cx - ( -x ) );
			else if( ( offset = VRAM_WIDTH - ( x + cx ) ) < 0 )	/* tail part out of right boundary */
				memcpy( pvram, pbits, cx - ( -offset ) );
			else
				memcpy( pvram, pbits, cx );

label_mode1_skip_this_page:
			pvram += VRAM_WIDTH;
			pbits += cx;
			yt += 8;
		}

		/* draw the last page */
		mask = bitsMask8[ ( cy & 7 ) - 1 ];	/* Now, (cy & 7 != 0) */

		if( x < 0 )		/* head part out of left boundary */
			memcpy_mask8( pvram + ( -x ), pbits + ( -x ), cx - ( -x ), mask );
		else if( ( offset = VRAM_WIDTH - ( x + cx ) ) < 0 )	/* tail part out of right boundary */
			memcpy_mask8( pvram, pbits, cx - ( -offset ), mask );
		else
			memcpy_mask8( pvram, pbits, cx, mask );

	} else if( mode == 2 || mode == 3 ) {
		/* 
		 * mode 3 is very similar to mode 2, excepting to font height. 
		 * In mode 2, we use OR operation to draw bitmap, so mode 3 can use same function. 
		 */

		yt = y;

		for( i = 0; i < cyByte; i ++ ) {
			if( x < 0 ) {		/* head part out of left boundary */
				if( yt >= 0 )
					memcpy_sh8( pvram + ( -x ), pbits + ( -x ), cx - ( -x ), yt & 7 );
				if( yt + 8 > 0 && yt + 8 < VRAM_HEIGHT )
					memcpy_sh8( pvram + ( -x ) + VRAM_WIDTH, pbits + ( -x ), cx - ( -x ), ( yt & 7 ) - 8 );
			} else if( ( offset = VRAM_WIDTH - ( x + cx ) ) < 0 ) {	/* tail part out of right boundary */
				if( yt >= 0 )
					memcpy_sh8( pvram, pbits, cx - ( -offset ), yt & 7 );
				if( yt + 8 > 0 && yt + 8 < VRAM_HEIGHT )
					memcpy_sh8( pvram + VRAM_WIDTH, pbits, cx - ( -offset ), ( yt & 7 ) - 8 );
			} else {
				if( yt >= 0 )
					memcpy_sh8( pvram, pbits, cx, yt & 7 );
				if( yt + 8 > 0 && yt + 8 < VRAM_HEIGHT )
					memcpy_sh8( pvram + VRAM_WIDTH, pbits, cx, ( yt & 7 ) - 8 );
			}

			pvram += VRAM_WIDTH;
			pbits += cx;
			yt += 8;

			if( yt >= VRAM_HEIGHT )
				goto label_skip_this_image;
		}

	} else {
		debug_out( "Under developement\n" );
	}

label_skip_this_image:
	;
}
#endif /* LCD_COL2_ORIENTED */

#ifdef LCD_ROW_ORIENTED
static inline void GS_DrawImage_1bpp_Row( int x, int y, const unsigned char *pbits, int cx, int cy )
{
	unsigned char *pvram;
	int cxByte;
	int mode;
	int i;
	int lcd_y;

	if( x + cx <= 0 || x >= VRAM_WIDTH )		/* out of range */ 
		return;

	if( y >= VRAM_HEIGHT || y + cy <= 0 )		/* out of range */ 
		return;

	cxByte = ( cx + 7 ) >> 3;	/* ( cx + 7 ) / 8 */

	if( ( x & 7 ) == 0 ) {		/* x % 8 == 0 */
		if( ( cx & 7 ) == 0 )
			mode = 0;
		else 
			mode = 1;
	} else {
		if( ( cx & 7 ) == 0 )
			mode = 2;
		else 
			mode = 3;		
	}

	pvram = &vRam.pixels[ ( x >> 3 ) + y * VRAM_WIDTH_BYTES ];

	lcd_y = y;

	if( mode == 0 ) {

		/* draw a font */ 
		for( i = 0; i < cy; i ++ ) {
			if( lcd_y < 0 )
				goto label_skip_this_col;

			if( cx == 8 ) {
				*pvram = *pbits;

			} else {
				debug_out( "fontWidth is not 8\n" );
			}

label_skip_this_col:
			lcd_y ++;
			pvram += VRAM_WIDTH_BYTES;
			pbits += cxByte;
		}

	} else {
		debug_out( "Under developement\n" );
	}
}
#endif /* LCD_ROW_ORIENTED */

void GS_DrawImage_1bpp( int x, int y, const unsigned char *pbits, int cx, int cy )
{
	rect_t rect;

#ifdef LCD_COL_ORIENTED
	GS_DrawImage_1bpp_Col( x, y, pbits, cx, cy );
#elif defined( LCD_COL2_ORIENTED )
	GS_DrawImage_1bpp_Col_Type2( x, y, pbits, cx, cy );
#elif defined( LCD_ROW_ORIENTED )
	GS_DrawImage_1bpp_Row( x, y, pbits, cx, cy );
#endif

	if( !fDrawOffScreen.all ) {
		rect.left = x;
		rect.top = y;
		rect.right = rect.left + cx;
		rect.bottom = rect.top + cy;

		GS_VRamToLcdDevice( &rect );
	}
}

void GS_DrawResImage( int x, int y, const unsigned char *pimage )
{
	const image_t * const pimage_res = ( const image_t * )pimage;

	if( ( pimage_res ->flags & IAMGE_FLAGS_BPP_MASK ) != IAMGE_FLAGS_1BPP ) {
		debug_out( "BPP not support\n" );
		return;
	}

	GS_DrawImage_1bpp( x, y, pimage_res ->data, pimage_res ->width, pimage_res ->height );
}

void GS_GetResImageSize( const unsigned char *pimage, int *pwidth, int *pheight )
{
	const image_t * const pimage_res = ( const image_t * )pimage;

	*pwidth = pimage_res ->width;
	*pheight = pimage_res ->height;
}

#endif /* !_TEXT_MODE */
