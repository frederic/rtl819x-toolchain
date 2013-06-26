#ifndef __UI_ON_PAINT_H__
#define __UI_ON_PAINT_H__

#include "gs_types.h"

typedef void ( *pFnOnPaintHandler_t )( const rect_t *pInvalidRect );
typedef int opid_t;

/* Register a handler */
extern int RegisterOnPaintHandler( pFnOnPaintHandler_t handler );

/* Unregister a handler */
extern int UnregisterOnPaintHandler( pFnOnPaintHandler_t handler );

extern void UnregisterAllOnPaintHandlers( void );

/* OnPaint Core function */
extern void DoInvalidRectangleOnPaint( const rect_t *pInvalidRect );

#endif /* __UI_ON_PAINT_H__ */
