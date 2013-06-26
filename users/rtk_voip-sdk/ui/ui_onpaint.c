#include "ui_config.h"
#include "ui_onpaint.h"


//typedef void ( *pFnOnPaintHandler_t )( const rect_t *pInvalidRect );
//typedef int opid_t;

typedef struct onpaint_s {
	pFnOnPaintHandler_t handler;
} onpaint_t;

/* Register a handler */
int RegisterOnPaintHandler( pFnOnPaintHandler_t handler )
{
	return 0;
}

/* Unregister a handler */
int UnregisterOnPaintHandler( pFnOnPaintHandler_t handler )
{
	return 0;
}


void UnregisterAllOnPaintHandlers( void )
{
}


/* OnPaint Core function */
void DoInvalidRectangleOnPaint( const rect_t *pInvalidRect )
{
}

