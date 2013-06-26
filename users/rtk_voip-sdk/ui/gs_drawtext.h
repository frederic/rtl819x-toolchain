#ifndef __GS_DRAWTEXT_H__
#define __GS_DRAWTEXT_H__

/* Draw text (x and y in unit of pixel, so graphic mode only) */
extern void GS_DrawText( int x, int y, const unsigned char *pszText, int len );

/* Draw text in center (x and y in unit of pixel, so graphic mode only) */
extern void GS_DrawTextInCenter( int y, const unsigned char *pszText );

/* Draw text (x and y in unit of character, but textual and graphic can use it) */
extern void GS_TextOut( int x, int y, const unsigned char *pszText, int len );

/* Draw text in center (x and y in unit of character, but textual and graphic can use it) */
extern void GS_TextOutInCenter( int y, const unsigned char *pszText );

/* Get width and height of specified text string (Graphic mode only) */
extern void GS_GetTextWidthHeight( const unsigned char *pszText, int len, int *pTextWidth, int *pTextHeight );

/* Get length fitting specified text string (Graphic mode only) */
extern int GS_GetTextLengthToFitWidth( const unsigned char *pszText, int len, int width );

/* Get nch of last character in a text string (Graphic mode only) */
extern int GS_GetLastNchOfTextString( const unsigned char *pszText, int len );


#endif /* __GS_DRAWTEXT_H__ */

