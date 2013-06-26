#include "ui_config.h"
#include "gs_vram.h"
#include "gs_shape.h"
#include "gs_basic.h"
#include "gs_def.h"

#ifndef _TEXT_MODE


typedef enum {
	OP_OR,
	OP_NAND,
	OP_XOR,
} pixel_op_t;

#ifdef LCD_COL_ORIENTED
/* ======================================================================= */
/* LCD column oriented */
  #if VRAM_HEIGHT_BYTES == 2
	static const cols_t bitsMask[] = {
		0x0001, 0x0003, 0x0007, 0x000F, 0x001F, 0x003F, 0x007F, 0x00FF,
		0x01FF, 0x03FF, 0x07FF, 0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF,
		0x0001, 0x0003, 0x0007, 0x000F, 0x001F, 0x003F, 0x007F, 0x00FF,
		0x01FF, 0x03FF, 0x07FF, 0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF,
	};
  #else
    #error "???"
  #endif

  #if VRAM_HEIGHT_BYTES == 2
    #define ENDIAN( x )	( ( ( unsigned short )x >> 8 ) | ( ( unsigned short )x << 8 ) )
  #else
    #error "???"
  #endif
#elif defined( LCD_ROW_ORIENTED )
/* ======================================================================= */
/* LCD row oriented */
  #if VRAM_ROW_ATOM_BYTE == 4
	static const rows_atom_t bitsMask[] = {
		0x00000001, 0x00000003, 0x00000007, 0x0000000F, 0x0000001F, 0x0000003F, 0x0000007F, 0x000000FF,
		0x000001FF, 0x000003FF, 0x000007FF, 0x00000FFF, 0x00001FFF, 0x00003FFF, 0x00007FFF, 0x0000FFFF,
		0x0001FFFF, 0x0003FFFF, 0x0007FFFF, 0x000FFFFF, 0x001FFFFF, 0x003FFFFF, 0x007FFFFF, 0x00FFFFFF,
		0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF, 0x0FFFFFFF, 0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF,
	};

	#define ENDIAN_ROW_ATOM( x )	htonl( x )
  #else
    ???
  #endif
/* ======================================================================= */
#endif /* LCD_ROW_ORIENTED */

static inline int GS_NormalizeAndCheckForHorizontalLine( int *x, int *y, int *cx )
{
	/* Make *cx to be positive */

	if( *cx < 0 ) {
		*x = *x + *cx;
		*cx = ( -1 ) * *cx;
	}

	if( *x < 0 ) {
		*cx += *x;
		*x = 0;
	}

	if( *cx <= 0 )
		return 1;

	if( *x >= VRAM_WIDTH )
		return 2;		/* out of screen */ 

	if( *y < 0 || *y >= VRAM_HEIGHT )
		return 3;		/* out of screen */

	if( *x + *cx > VRAM_WIDTH )
		*cx = VRAM_WIDTH - *x;

	return 0;
}

static inline int GS_NormalizeAndCheckForVerticalLine( int *x, int *y, int *cy )
{
	/* Make *cy to be positive */

	if( *cy < 0 ) {
		*y = *y + *cy;
		*cy = ( -1 ) * *cy;
	}

	if( *y < 0 ) {
		*cy += *y;
		*y = 0;
	}

	if( *cy <= 0 )
		return 1;

	if( *y >= VRAM_HEIGHT )
		return 2;		/* out of screen */ 

	if( *x < 0 || *x >= VRAM_WIDTH )
		return 3;		/* out of screen */

	if( *y + *cy > VRAM_HEIGHT )
		*cy = VRAM_HEIGHT - *y;

	return 0;
}

#ifdef LCD_COL_ORIENTED
static void GS_DrawHorizontalLine_Core_1bpp_Col( int x, int y, int cx, pixel_op_t op )
{
	unsigned char *pvram;
	unsigned char bit;
	rect_t rect;
	int i;

	if( GS_NormalizeAndCheckForHorizontalLine( &x, &y, &cx ) )	
		return;

	/* (x, y) and (x+cx, y) are in screen, and cx is positive */
	pvram = &vRam.pixels[ x * VRAM_HEIGHT_BYTES + y / 8 ];

	bit = 1 << ( y & 0x07 );	/* ( y & 0x07 ) = ( y % 8 ) */

	switch( op ) {
	case OP_OR:
		for( i = 0; i < cx; i ++ ) {
			*pvram |= bit;
			pvram += VRAM_HEIGHT_BYTES;
		}
		break;

	case OP_NAND:
		bit = ~bit;
		for( i = 0; i < cx; i ++ ) {
			*pvram &= bit;
			pvram += VRAM_HEIGHT_BYTES;
		}
		break;

	case OP_XOR:
		for( i = 0; i < cx; i ++ ) {
			*pvram ^= bit;
			pvram += VRAM_HEIGHT_BYTES;
		}
		break;
	}

	/* output to LCD */
	if( !fDrawOffScreen.all ) {
		rect.left = x;
		rect.top = y;
		rect.right = x + cx;
		rect.bottom = y + 1;

		GS_VRamToLcdDevice( &rect );
	}
}
#endif /* LCD_COL_ORIENTED */

#ifdef LCD_COL2_ORIENTED
static void GS_DrawHorizontalLine_Core_1bpp_Col_Type2( int x, int y, int cx, pixel_op_t op )
{
	unsigned char *pvram;
	unsigned char bit;
	rect_t rect;
	int i;

	if( GS_NormalizeAndCheckForHorizontalLine( &x, &y, &cx ) )	
		return;

	/* (x, y) and (x+cx, y) are in screen, and cx is positive */
	pvram = &vRam.pixels[ x + ( y >> 3 ) * VRAM_WIDTH ];

	bit = 1 << ( y & 0x07 );	/* ( y & 0x07 ) = ( y % 8 ) */

	switch( op ) {
	case OP_OR:
		for( i = 0; i < cx; i ++ ) {
			*pvram |= bit;
			pvram ++;
		}
		break;

	case OP_NAND:
		bit = ~bit;
		for( i = 0; i < cx; i ++ ) {
			*pvram &= bit;
			pvram ++;
		}
		break;

	case OP_XOR:
		for( i = 0; i < cx; i ++ ) {
			*pvram ^= bit;
			pvram ++;
		}
		break;
	}

	/* output to LCD */
	if( !fDrawOffScreen.all ) {
		rect.left = x;
		rect.top = y;
		rect.right = x + cx;
		rect.bottom = y + 1;

		GS_VRamToLcdDevice( &rect );
	}
}
#endif /* LCD_COL2_ORIENTED */

#ifdef LCD_ROW_ORIENTED
static void GS_DrawHorizontalLine_Core_1bpp_Row( int x, int y, int cx, pixel_op_t op )
{
	rect_t rect;
	rows_atom_t *patom;
	rows_atom_t bits;
	int cx_remainder;

	if( GS_NormalizeAndCheckForHorizontalLine( &x, &y, &cx ) )	
		return;

	/* (x, y) and (x+cx, y) are in screen, and cx is positive */
	patom = &vRam.rows[ y ].atom[ x >> VRAM_ROW_ATOM_BIT_LOG ];

	if( ( x >> VRAM_ROW_ATOM_BIT_LOG ) == ( ( x + cx - 1 ) >> VRAM_ROW_ATOM_BIT_LOG ) ) {	
		/* first atom: line within an atom */
		bits = bitsMask[ cx - 1 ];	/* cx has to be less than 32 */
		cx_remainder = 0;
	} else {
		/* first atom: cross */
		bits = bitsMask[ VRAM_ROW_ATOM_BYTE * 4 - x - 1 ];
		cx_remainder = cx - ( VRAM_ROW_ATOM_BYTE * 4 - x );
	}

	bits <<= x;

#ifndef _TEST_MODE
	bits = ENDIAN_L( bits );
#endif

	switch( op ) {
	case OP_OR:
		*patom |= bits;

		while( cx_remainder >= VRAM_ROW_ATOM_BYTE * 4 ) {
			*patom |= VRAM_ROW_ATOM_BITS_MASK;

			cx_remainder -= VRAM_ROW_ATOM_BYTE * 4;
			patom ++;
		}

		if( cx_remainder > 0 ) {
			bits = bitsMask[ cx_remainder - 1 ];
#ifndef _TEST_MODE
			bits = ENDIAN_L( bits );
#endif
			*patom |= bits;
		}

		break;

	case OP_NAND:
		*patom &= ~bits;

		while( cx_remainder >= VRAM_ROW_ATOM_BYTE * 4 ) {
			*patom &= ~VRAM_ROW_ATOM_BITS_MASK;

			cx_remainder -= VRAM_ROW_ATOM_BYTE * 4;
			patom ++;
		}

		if( cx_remainder > 0 ) {
			bits = bitsMask[ cx_remainder - 1 ];
#ifndef _TEST_MODE
			bits = ENDIAN_L( bits );
#endif
			*patom &= ~bits;
		}
		break;

	case OP_XOR:
		*patom ^= bits;

		while( cx_remainder >= VRAM_ROW_ATOM_BYTE * 4 ) {
			*patom ^= VRAM_ROW_ATOM_BITS_MASK;

			cx_remainder -= VRAM_ROW_ATOM_BYTE * 4;
			patom ++;
		}

		if( cx_remainder > 0 ) {
			bits = bitsMask[ cx_remainder - 1 ];
#ifndef _TEST_MODE
			bits = ENDIAN_L( bits );
#endif
			*patom ^= bits;
		}
		break;
	}

	/* output to LCD */
	if( !fDrawOffScreen.all ) {
		rect.left = x;
		rect.top = y;
		rect.right = x + cx;
		rect.bottom = y + 1;

		GS_VRamToLcdDevice( &rect );
	}
}
#endif /* LCD_ROW_ORIENTED */

#ifdef LCD_COL_ORIENTED
static void GS_DrawVerticalLine_Core_1bpp_Col( int x, int y, int cy, pixel_op_t op )
{
	cols_t *pcols;
	cols_t bits;
	rect_t rect;

	if( GS_NormalizeAndCheckForVerticalLine( &x, &y, &cy ) )	
		return;

	/* (x, y) and (x, y+cy) are in screen, and cy is positive */
	pcols = &vRam.cols[ x ];
	bits = bitsMask[ cy ] << y;

#ifndef _TEST_MODE
	bits = ENDIAN( bits );
#endif

	switch( op ) {
	case OP_OR:
		*pcols |= bits;
		break;

	case OP_NAND:
		*pcols &= ~bits;
		break;

	case OP_XOR:
		*pcols ^= bits;
		break;
	}

	/* output to LCD */
	if( !fDrawOffScreen.all ) {
		rect.left = x;
		rect.top = y;
		rect.right = x + 1;
		rect.bottom = y + cy;

		GS_VRamToLcdDevice( &rect );
	}
}
#endif /* LCD_COL_ORIENTED */

#ifdef LCD_COL2_ORIENTED
static void GS_DrawVerticalLine_Core_1bpp_Col_Type2( int x, int y, int cy, pixel_op_t op )
{
	unsigned char *pvram;
	unsigned char bits8_head, bits8_tail;
	rect_t rect;
	int i, mid_len;

	if( GS_NormalizeAndCheckForVerticalLine( &x, &y, &cy ) )	
		return;

	/* (x, y) and (x, y+cy) are in screen, and cy is positive */
	pvram = &vRam.pixels[ x + ( y >> 3 ) * VRAM_WIDTH ];

	if( ( y >> 3 ) == ( ( y + cy - 1 ) >> 3 ) ) {
		/* cy <= 8 and the line within a byte */
		bits8_head = bitsMask8[ cy ] << ( y & 0x07 );
		bits8_tail = 0x00;
		mid_len = 0;
	} else {
		bits8_head = bitsMask8[ 7 - ( y & 0x07 ) ] << ( y & 0x07 );
		bits8_tail = bitsMask8[ ( y + cy - 1 ) & 0x07 ];
		mid_len = ( ( y + cy - 1 ) >> 3 ) - ( y >> 3 ) - 1;
	}

	switch( op ) {
	case OP_OR:
		*pvram |= bits8_head;
		pvram += VRAM_WIDTH;
		for( i = 0; i < mid_len; i ++ ) {
			//*pvram |= 0xFF;
			*pvram = 0xFF;
			pvram += VRAM_WIDTH;
		}
		*pvram |= bits8_tail;
		break;

	case OP_NAND:
		*pvram &= ~bits8_head;
		pvram += VRAM_WIDTH;
		for( i = 0; i < mid_len; i ++ ) {
			//*pvram &= ~0xFF;
			*pvram = 0;
			pvram += VRAM_WIDTH;
		}
		*pvram &= ~bits8_tail;
		break;

	case OP_XOR:
		*pvram ^= bits8_head;
		pvram += VRAM_WIDTH;
		for( i = 0; i < mid_len; i ++ ) {
			*pvram ^= 0xFF;
			pvram += VRAM_WIDTH;
		}
		*pvram ^= bits8_tail;
		break;
	}

	/* output to LCD */
	if( !fDrawOffScreen.all ) {
		rect.left = x;
		rect.top = y;
		rect.right = x + 1;
		rect.bottom = y + cy;

		GS_VRamToLcdDevice( &rect );
	}
}
#endif /* LCD_COL2_ORIENTED */

#ifdef LCD_ROW_ORIENTED
static void GS_DrawVerticalLine_Core_1bpp_Row( int x, int y, int cy, pixel_op_t op )
{
	unsigned char *pvram;
	unsigned char bit;
	rect_t rect;
	int i;

	if( GS_NormalizeAndCheckForHorizontalLine( &x, &y, &cy ) )	
		return;

	/* (x, y) and (x, y+cy) are in screen, and cy is positive */
	pvram = &vRam.pixels[ ( x >> VRAM_ROW_ATOM_BIT_LOG ) + y * VRAM_WIDTH_BYTES ];

	bit = 1 << ( x & 0x07 );	/* ( y & 0x07 ) = ( y % 8 ) */

	switch( op ) {
	case OP_OR:
		for( i = 0; i < cy; i ++ ) {
			*pvram |= bit;
			pvram += VRAM_WIDTH_BYTES;
		}
		break;

	case OP_NAND:
		bit = ~bit;
		for( i = 0; i < cy; i ++ ) {
			*pvram &= bit;
			pvram += VRAM_WIDTH_BYTES;
		}
		break;

	case OP_XOR:
		for( i = 0; i < cy; i ++ ) {
			*pvram ^= bit;
			pvram += VRAM_WIDTH_BYTES;
		}
		break;
	}

	/* output to LCD */
	if( !fDrawOffScreen.all ) {
		rect.left = x;
		rect.top = y;
		rect.right = x + 1;
		rect.bottom = y + cy;

		GS_VRamToLcdDevice( &rect );
	}
}
#endif /* LCD_ROW_ORIENTED */

static inline void GS_NormalizeRectangle( int *x1, int *y1, int *x2, int *y2 )
{
	int t;

	if( *x1 > *x2 ) {
		t = *x1;
		*x1 = *x2;
		*x2 = t;
	}

	if( *y1 > *y2 ) {
		t = *y1;
		*y1 = *y2;
		*y2 = t;
	}
}

static inline int GS_NormalizeAndCheckForSoildRectangle( int *x1, int *y1, int *x2, int *y2 )
{
	GS_NormalizeRectangle( x1, y1, x2, y2 );

	if( *x1 > VRAM_WIDTH || *y1 > VRAM_HEIGHT || *x2 < 0 || *y2 < 0 )	/* out of screen */
		return 1;

	if( *x1 < 0 )
		*x1 = 0;

	if( *y1 < 0 )
		*y1 = 0;

	if( *x2 > VRAM_WIDTH )
		*x2 = VRAM_WIDTH;

	if( *y2 > VRAM_HEIGHT )
		*y2 = VRAM_HEIGHT;

	if( *x1 == *x2 || *y1 == *y2 )	/* equal, so draw nothing */
		return 2;

	return 0;
}

#ifdef LCD_COL_ORIENTED
static void GS_DrawSolidRectangle_Core_1bpp_Col( int x1, int y1, int x2, int y2, pixel_op_t op )
{
	cols_t *pcols;
	cols_t bits;
	rect_t rect;
	int i;

	if( GS_NormalizeAndCheckForSoildRectangle( &x1, &y1, &x2, &y2 ) )
		return;

	/* (x1, y1) and (x2, y2) are in screen */
	pcols = &vRam.cols[ x1 ];
	bits = bitsMask[ y2 - y1 ] << y1;

#ifndef _TEST_MODE
	bits = ENDIAN( bits );
#endif

	switch( op ) {
	case OP_OR:
		for( i = x1; i < x2; i ++ ) {
			*pcols |= bits;
			pcols ++;
		}
		break;

	case OP_NAND:
		bits = ~bits;
		for( i = x1; i < x2; i ++ ) {
			*pcols &= bits;
			pcols ++;
		}
		break;

	case OP_XOR:
		for( i = x1; i < x2; i ++ ) {
			*pcols ^= bits;
			pcols ++;
		}
		break;
	}

	/* output to LCD */
	if( !fDrawOffScreen.all ) {
		rect.left = x1;
		rect.top = y1;
		rect.right = x2;
		rect.bottom = y2;

		GS_VRamToLcdDevice( &rect );
	}
}
#endif /* LCD_COL_ORIENTED */

#ifdef LCD_COL2_ORIENTED
static void GS_DrawSolidRectangle_Core_1bpp_Col_Type2( int x1, int y1, int x2, int y2, pixel_op_t op )
{
	unsigned char *pvram, *pvram2;
	unsigned char bits8_head, bits8_tail;
	rect_t rect;
	int i, j, mid_len;

	if( GS_NormalizeAndCheckForSoildRectangle( &x1, &y1, &x2, &y2 ) )
		return;

	/* (x1, y1) and (x2, y2) are in screen */
	pvram = &vRam.pixels[ x1 + ( y1 >> 3 ) * VRAM_WIDTH ];

	if( ( y1 >> 3 ) == ( ( y2 - 1 ) >> 3 ) ) {
		/* cy <= 8 and the line within a byte */
		bits8_head = bitsMask8[ y2 - y1 ] << ( y1 & 0x07 );
		bits8_tail = 0x00;
		mid_len = 0;
	} else {
		bits8_head = bitsMask8[ 7 - ( y1 & 0x07 ) ] << ( y1 & 0x07 );
		bits8_tail = bitsMask8[ ( y2 - 1 ) & 0x07 ];
		mid_len = ( ( y2 - 1 ) >> 3 ) - ( y1 >> 3 ) - 1;
	}

	switch( op ) {
	case OP_OR:
		pvram2 = pvram;
		for( i = x1; i < x2; i ++ ) {
			*pvram2 |= bits8_head;
			pvram2 ++;
		}

		pvram += VRAM_WIDTH;
		pvram2 = pvram;
		for( j = 0; j < mid_len; j ++ ) {
			for( i = x1; i < x2; i ++ ) {
				//*pvram2 |= 0xFF;
				*pvram2 = 0xFF;
				pvram2 ++;
			}
			pvram += VRAM_WIDTH;
			pvram2 = pvram;
		}

		for( i = x1; i < x2; i ++ ) {
			*pvram2 |= bits8_tail;
			pvram2 ++;
		}
		break;

	case OP_NAND:
		pvram2 = pvram;
		for( i = x1; i < x2; i ++ ) {
			*pvram2 &= ~bits8_head;
			pvram2 ++;
		}

		pvram += VRAM_WIDTH;
		pvram2 = pvram;
		for( j = 0; j < mid_len; j ++ ) {
			for( i = x1; i < x2; i ++ ) {
				//*pvram2 &= ~0xFF;
				*pvram2 = 0;
				pvram2 ++;
			}
			pvram += VRAM_WIDTH;
			pvram2 = pvram;
		}

		for( i = x1; i < x2; i ++ ) {
			*pvram2 &= ~bits8_tail;
			pvram2 ++;
		}
		break;

	case OP_XOR:
		pvram2 = pvram;
		for( i = x1; i < x2; i ++ ) {
			*pvram2 ^= bits8_head;
			pvram2 ++;
		}

		pvram += VRAM_WIDTH;
		pvram2 = pvram;
		for( j = 0; j < mid_len; j ++ ) {
			for( i = x1; i < x2; i ++ ) {
				*pvram2 ^= 0xFF;
				pvram2 ++;
			}
			pvram += VRAM_WIDTH;
			pvram2 = pvram;
		}

		for( i = x1; i < x2; i ++ ) {
			*pvram2 ^= bits8_tail;
			pvram2 ++;
		}
		break;
	}

	/* output to LCD */
	if( !fDrawOffScreen.all ) {
		rect.left = x1;
		rect.top = y1;
		rect.right = x2;
		rect.bottom = y2;

		GS_VRamToLcdDevice( &rect );
	}
}
#endif /* LCD_COL2_ORIENTED */

#ifdef LCD_ROW_ORIENTED
static void GS_DrawSolidRectangle_Core_1bpp_Row( int x1, int y1, int x2, int y2, pixel_op_t op )
{
	rows_atom_t *patom;
	rows_atom_t bits_first, bits_last;
	int mid_count;
	rect_t rect;
	int i, j;
	int cx;

	if( GS_NormalizeAndCheckForSoildRectangle( &x1, &y1, &x2, &y2 ) )
		return;

	/* (x1, y1) and (x2, y2) are in screen */
	cx = x2 - x1;
	if( ( x1 >> VRAM_ROW_ATOM_BIT_LOG ) == ( ( x2 - 1 ) >> VRAM_ROW_ATOM_BIT_LOG ) ) {
		/* all within one atom */
		bits_first = bitsMask[ cx - 1 ] << x1;
		cx = 0;
	} else {
		/* cross many atom */
		bits_first = bitsMask[ VRAM_ROW_ATOM_BYTE * 32 - x1 - 1 ] << x1;
		cx -= ( VRAM_ROW_ATOM_BYTE * 32 - x1 );
	}

#ifndef _TEST_MODE
	bits_first = ENDIAN_ROW_ATOM( bits_first );
#endif

	mid_count = cx >> VRAM_ROW_ATOM_BIT_LOG;	/* mid_count = cx / 32 */
	cx &= VRAM_ROW_ATOM_REMANDER;				/* cx = cx % 32 */

	if( cx ) {
		bits_last = bitsMask[ cx - 1 ];
	} else 
		bits_last = 0;

#ifndef _TEST_MODE
	bits_last = ENDIAN_ROW_ATOM( bits_last );
#endif

	switch( op ) {
	case OP_OR:
		for( i = y1; i < y2; i ++ ) {
			patom = &vRam.rows[ i ].atom[ x1 >> VRAM_ROW_ATOM_BIT_LOG ];

			*patom ++ |= bits_first;

			for( j = 0; j < mid_count; j ++ )
				*patom ++ |= VRAM_ROW_ATOM_BITS_MASK;

			if( bits_last )
				*patom ++ |= bits_last;
		}
		break;

	case OP_NAND:
		bits_first = ~bits_first;

		for( i = y1; i < y2; i ++ ) {
			patom = &vRam.rows[ i ].atom[ x1 >> VRAM_ROW_ATOM_BIT_LOG ];

			*patom ++ &= bits_first;

			for( j = 0; j < mid_count; j ++ )
				*patom ++ &= ~VRAM_ROW_ATOM_BITS_MASK;

			if( bits_last )
				*patom ++ &= ~bits_last;
		}
		break;

	case OP_XOR:
		for( i = y1; i < y2; i ++ ) {
			patom = &vRam.rows[ i ].atom[ x1 >> VRAM_ROW_ATOM_BIT_LOG ];

			*patom ++ ^= bits_first;

			for( j = 0; j < mid_count; j ++ )
				*patom ++ ^= VRAM_ROW_ATOM_BITS_MASK;

			if( bits_last )
				*patom ++ ^= bits_last;
		}
		break;
	}

	/* output to LCD */
	if( !fDrawOffScreen.all ) {
		rect.left = x1;
		rect.top = y1;
		rect.right = x2;
		rect.bottom = y2;

		GS_VRamToLcdDevice( &rect );
	}
}
#endif /* LCD_ROW_ORIENTED */

/* ============================================================================ */
/* External functions */
/* ============================================================================ */
void GS_DrawHorizontalLine( int x, int y, int cx, int color )
{
#ifdef LCD_COL_ORIENTED
	GS_DrawHorizontalLine_Core_1bpp_Col( x, y, cx, ( color ? OP_OR : OP_NAND ) );
#elif defined( LCD_COL2_ORIENTED )
	GS_DrawHorizontalLine_Core_1bpp_Col_Type2( x, y, cx, ( color ? OP_OR : OP_NAND ) );
#elif defined( LCD_ROW_ORIENTED )
	GS_DrawHorizontalLine_Core_1bpp_Row( x, y, cx, ( color ? OP_OR : OP_NAND ) );
#endif
}

void GS_DrawVerticalLine( int x, int y, int cy, int color )
{
#ifdef LCD_COL_ORIENTED
	GS_DrawVerticalLine_Core_1bpp_Col( x, y, cy, ( color ? OP_OR : OP_NAND ) );
#elif defined( LCD_COL2_ORIENTED )
	GS_DrawVerticalLine_Core_1bpp_Col_Type2( x, y, cy, ( color ? OP_OR : OP_NAND ) );
#elif defined( LCD_ROW_ORIENTED )
	GS_DrawVerticalLine_Core_1bpp_Row( x, y, cy, ( color ? OP_OR : OP_NAND ) );
#endif
}

void GS_DrawHorizontalLineInverse( int x, int y, int cx )
{
#ifdef LCD_COL_ORIENTED
	GS_DrawHorizontalLine_Core_1bpp_Col( x, y, cx, OP_XOR );
#elif defined( LCD_COL2_ORIENTED )
	GS_DrawHorizontalLine_Core_1bpp_Col_Type2( x, y, cx, OP_XOR );
#elif defined( LCD_ROW_ORIENTED )
	GS_DrawHorizontalLine_Core_1bpp_Row( x, y, cx, OP_XOR );
#endif
}

void GS_DrawVerticalLineInverse( int x, int y, int cy )
{
#ifdef LCD_COL_ORIENTED
	GS_DrawVerticalLine_Core_1bpp_Col( x, y, cy, OP_XOR );
#elif defined( LCD_COL2_ORIENTED )
	GS_DrawVerticalLine_Core_1bpp_Col_Type2( x, y, cy, OP_XOR );
#elif defined( LCD_ROW_ORIENTED )
	GS_DrawVerticalLine_Core_1bpp_Row( x, y, cy, OP_XOR );
#endif
}

void GS_DrawRectangle( int x1, int y1, int x2, int y2 )
{
	GS_DrawHorizontalLine( x1, y1, x2 - x1, 1 );
	GS_DrawHorizontalLine( x1, y2, x2 - x1, 1 );

	GS_DrawVerticalLine( x1, y1, y2 - y1, 1 );
	GS_DrawVerticalLine( x2, y1, y2 - y1 + 1, 1 );
}

void GS_DrawRectangle_rect( const rect_t *prect )
{
	GS_DrawRectangle( prect ->left, prect ->top, prect ->right, prect ->bottom );
}

void GS_DrawSolidRectangle( int x1, int y1, int x2, int y2, int color )
{
#ifdef LCD_COL_ORIENTED
	GS_DrawSolidRectangle_Core_1bpp_Col( x1, y1, x2, y2, ( color ? OP_OR : OP_NAND ) );
#elif defined( LCD_COL2_ORIENTED )
	GS_DrawSolidRectangle_Core_1bpp_Col_Type2( x1, y1, x2, y2, ( color ? OP_OR : OP_NAND ) );
#elif defined( LCD_ROW_ORIENTED )
	GS_DrawSolidRectangle_Core_1bpp_Row( x1, y1, x2, y2, ( color ? OP_OR : OP_NAND ) );
#endif
}

void GS_DrawSolidRectangle_rect( const rect_t *prect, int color )
{
	GS_DrawSolidRectangle( prect ->left, prect ->top, prect ->right, prect ->bottom, color );
}

void GS_DrawSolidRectangleInverse( int x1, int y1, int x2, int y2 )
{
#ifdef LCD_COL_ORIENTED
	GS_DrawSolidRectangle_Core_1bpp_Col( x1, y1, x2, y2, OP_XOR );
#elif defined( LCD_COL2_ORIENTED )
	GS_DrawSolidRectangle_Core_1bpp_Col_Type2( x1, y1, x2, y2, OP_XOR );
#elif defined( LCD_ROW_ORIENTED )
	GS_DrawSolidRectangle_Core_1bpp_Row( x1, y1, x2, y2, OP_XOR );
#endif
}

void GS_DrawSolidRectangleInverse_rect( const rect_t *prect )
{
	GS_DrawSolidRectangleInverse( prect ->left, prect ->top, prect ->right, prect ->bottom );
}

#endif /* !_TEXT_MODE */
