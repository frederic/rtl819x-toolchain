#ifndef __GS_SHAPE_H__
#define __GS_SHAPE_H__

#ifndef _TEXT_MODE

#include "gs_types.h"

/* Draw a horizontal line */
extern void GS_DrawHorizontalLine( int x, int y, int cx, int color );

/* Draw a vertical line */
extern void GS_DrawVerticalLine( int x, int y, int cy, int color );

/* Inverse a horizontal line */
extern void GS_DrawHorizontalLineInverse( int x, int y, int cx );

/* Inverse a vertical line */
extern void GS_DrawVerticalLineInverse( int x, int y, int cy );

/* Draw a rectangle */
extern void GS_DrawRectangle( int x1, int y1, int x2, int y2 );
extern void GS_DrawRectangle_rect( const rect_t *prect );

/* Draw a solid rectangle */
extern void GS_DrawSolidRectangle( int x1, int y1, int x2, int y2, int color );
extern void GS_DrawSolidRectangle_rect( const rect_t *prect, int color );

/* Inverse a solid rectangle */
extern void GS_DrawSolidRectangleInverse( int x1, int y1, int x2, int y2 );
extern void GS_DrawSolidRectangleInverse_rect( const rect_t *prect );

#endif /* !_TEXT_MODE */

#endif /* __GS_SHAPE_H__ */

