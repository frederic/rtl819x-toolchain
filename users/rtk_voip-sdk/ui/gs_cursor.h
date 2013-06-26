#ifndef __GS_CURSOR_H__
#define __GS_CURSOR_H__

typedef enum {
	CURSOR_STATUS_OFF,
	CURSOR_STATUS_OFF_NODRAW,	/* Almost equal to CURSOR_STATUS_OFF, but no erase cursor. */
	CURSOR_STATUS_ON,
	CURSOR_STATUS_SQUARE,
} cursor_status_t;

extern void GS_InitializeCursor( void );
extern void GS_CursorBlinkTimerEvent( void );
extern void GS_SetCursorPosition( int x, int y );
extern void GS_SetCursorStatus( cursor_status_t status );

#endif /* __GS_CURSOR_H__ */

