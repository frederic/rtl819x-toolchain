#include "ui_config.h"
#include "gs_basic.h"
#include <string.h>

#if !defined( _TEXT_MODE ) && defined( LCD_COL2_ORIENTED )
/* ======================================================================= */
/* LCD column oriented Type 2 */
const unsigned char bitsMask8[] = { 
	0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF, 
};

void memcpy_mask8( unsigned char *dst, const unsigned char *src, int len, unsigned char mask )
{
	int i;
	unsigned char ch;

	for( i = 0; i < len; i ++ ) {
		ch = *dst;
		ch &= ~mask;
		ch |= *src;
		*dst = ch;

		dst ++;
		src ++;
	}
}

void memcpy_sh8( unsigned char *dst, const unsigned char *src, int len, int sh )
{
	int i;
	unsigned char ch;
	unsigned char mask;

	if( sh == 0 ) {
		memcpy( dst, src, len );
		return;
	}

	if( sh <= -8 || sh >= 8 )
		return;

	if( sh > 0 ) {	/* left shift */

		mask = bitsMask8[ sh - 1 ];	/* 7 >= sh > 0 */

		for( i = 0; i < len; i ++ ) {
			ch = *dst;
			ch &= mask;
			ch |= *src << sh;
			*dst = ch;

			dst ++;
			src ++;
		}

	} else {		/* right shift */

		mask = ~bitsMask8[ 7 + sh ];		/* 7 >= -sh > 0 */
		sh = -sh;	/* abs( sh ) */

		for( i = 0; i < len; i ++ ) {
			ch = *dst;
			ch &= mask;
			ch |= *src >> sh;
			*dst = ch;

			dst ++;
			src ++;
		}
	}
}

#endif	/* !_TEXT_MODE && LCD_COL2_ORIENTED */
