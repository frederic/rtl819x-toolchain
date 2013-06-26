#ifndef __GS_LIB_H__
#define __GS_LIB_H__

#include "gs_types.h"
#include "gs_drawtext.h"
#include "gs_cursor.h"
#include "gs_shape.h"

/* initialize and terminate GS */
extern void GS_Initialization( void );
extern void GS_Termination( void );

/* draw off screen functions */
extern void GS_DrawOffScreen( void );
extern void GS_DrawOffScreenAndClearScreen( void );
extern void GS_DrawOnScreen( const rect_t *pInvalidRect );

/* draw off screen functions (supervisor mode) */
/* NOTE: These function should be pair and non-nested, or system will be abnormal. */
extern void GS_SupervisorDrawOffScreen( void );
extern void GS_SupervisorDrawOnScreen( const rect_t *pInvalidRect );

/* clear screen */
extern void GS_ClearScreen( void );

#endif /* __GS_LIB_H__ */

