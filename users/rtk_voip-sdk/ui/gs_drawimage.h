#ifndef __GS_DRAW_IMAGE_H__
#define __GS_DRAW_IMAGE_H__

#ifndef _TEXT_MODE

/* Draw a 1 bpp image */
extern void GS_DrawImage_1bpp( int x, int y, const unsigned char *pbits, int cx, int cy );

/* Draw resource image */
extern void GS_DrawResImage( int x, int y, const unsigned char *pimage );

/* Get resource image size */
extern void GS_GetResImageSize( const unsigned char *pimage, int *pwidth, int *pheight );

#else

#define GS_DrawImage_1bpp( x, y, pbits, cx, cy )	

#define GS_DrawResImage( x, y, pimage )

#define GS_GetResImageSize( pimage, pwidth, pheight )

#endif

#endif /* __GS_DRAW_IMAGE_H__ */
