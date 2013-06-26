#ifndef __GS_BASIC_H__
#define __GS_BASIC_H__

#if !defined( _TEXT_MODE ) && defined( LCD_COL2_ORIENTED )
extern const unsigned char bitsMask8[];
#endif

/* memcpy of mask part */
extern void memcpy_mask8( unsigned char *dst, const unsigned char *src, int len, unsigned char mask );

/* memcpy with shift ( sh < 0 -> right shift; sh > 0 -> left shift */
extern void memcpy_sh8( unsigned char *dst, const unsigned char *src, int len, int sh );

#endif /* __GS_BASIC_H__ */
