#include "ui_config.h"
#include "gs_lib.h"
#include "gs_cursor.h"
#include "ioctl_lcd.h"
#include "ioctl_softrtc.h"
/*
 * This is to provide a common interfaces to show cursor, no matter 
 * hardware or software cursor.
 */

//#define CURSOR_BLINK_PERIOD		400	/* 0.4 second (Because hardware blink in this period) */
#define CURSOR_BLINK_PERIOD		500	/* 0.5 second */

#ifndef _HARDWARE_CURSOR
#define CURSOR_WIDTH				CHAR_WIDTH
#define CURSOR_HEIGHT				CHAR_HEIGHT
#endif

typedef struct {
	int locCursorX;
	int locCursorY;
	cursor_status_t cursorStatus;
	int bCursorBlinkAction:1;	/* used when CURSOR_STATUS_ON */
	int bCursorBlinkStatus:1;	/* used when CURSOR_STATUS_ON */
	uptime_t cursorTime;
} cursor_vars_t;

static cursor_vars_t cursor_vars;

#ifndef _HARDWARE_CURSOR
static void GS_DrawCursor( int x, int y, cursor_status_t status );
static void GS_EraseCursor( int x, int y, cursor_status_t status );
#endif

void GS_InitializeCursor( void )
{
	cursor_vars.locCursorX = 0;
	cursor_vars.locCursorY = 0;
	cursor_vars.cursorStatus = CURSOR_STATUS_OFF;
	cursor_vars.bCursorBlinkAction = 0;
	cursor_vars.bCursorBlinkStatus = 0;
}

void GS_CursorBlinkTimerEvent( void )
{
	if( cursor_vars.cursorStatus != CURSOR_STATUS_ON &&
#ifdef _HARDWARE_CURSOR
		1 )
#else
		cursor_vars.cursorStatus != CURSOR_STATUS_SQUARE )
#endif
	{
		return;
	}
		
	if(	CheckIfTimeoutInMillisecond( &cursor_vars.cursorTime, 
									 CURSOR_BLINK_PERIOD ) == 0 )
	{
		if( cursor_vars.bCursorBlinkAction ) {
			if( cursor_vars.bCursorBlinkStatus ) {
				cursor_vars.bCursorBlinkStatus = 0;
#ifndef _HARDWARE_CURSOR
				GS_EraseCursor( cursor_vars.locCursorX, cursor_vars.locCursorY, cursor_vars.cursorStatus );
#endif
			} else {
				cursor_vars.bCursorBlinkStatus = 1;
#ifndef _HARDWARE_CURSOR
				GS_DrawCursor( cursor_vars.locCursorX, cursor_vars.locCursorY, cursor_vars.cursorStatus );
#endif
			}

#ifdef _HARDWARE_CURSOR
			LCD_CursorOnOff( cursor_vars.bCursorBlinkStatus, 0 );
#endif
		}
	}
}

void GS_SetCursorPosition( int x, int y )
{
	cursor_vars.locCursorX = x;
	cursor_vars.locCursorY = y;
	
	if( cursor_vars.cursorStatus != CURSOR_STATUS_OFF ) {
#ifdef _HARDWARE_CURSOR
		LCD_MoveCursor( cursor_vars.locCursorX, cursor_vars.locCursorY );
#endif
	}
}

void GS_SetCursorStatus( cursor_status_t status )
{	
	/*
	 * NOTE: We change cursor's behavior again.
	 */
	if( cursor_vars.cursorStatus == status )
		return;		/* unchange status */

	if( status == CURSOR_STATUS_OFF_NODRAW ) {
		if( cursor_vars.cursorStatus == CURSOR_STATUS_OFF )
			return;

		status = CURSOR_STATUS_OFF;
		goto label_no_erase_cursor;
	}

#ifndef _HARDWARE_CURSOR
	if( cursor_vars.bCursorBlinkStatus )
		GS_EraseCursor( cursor_vars.locCursorX, cursor_vars.locCursorY, cursor_vars.cursorStatus );
#endif

label_no_erase_cursor:

	switch( cursor_vars.cursorStatus = status ) {
	case CURSOR_STATUS_OFF:
		cursor_vars.bCursorBlinkAction = 0;
		
#ifdef _HARDWARE_CURSOR
		LCD_CursorOnOff( 0, 0 );
#endif
		break;
		
	case CURSOR_STATUS_SQUARE:
#ifdef _HARDWARE_CURSOR
		cursor_vars.bCursorBlinkAction = 0;
		
		LCD_MoveCursor( cursor_vars.locCursorX, cursor_vars.locCursorY );
		LCD_CursorOnOff( 1, 1 );
#else
		cursor_vars.bCursorBlinkAction = 1;
		cursor_vars.bCursorBlinkStatus = 1;
		cursor_vars.cursorTime = GetUptimeInMillisecond();

		GS_DrawCursor( cursor_vars.locCursorX, cursor_vars.locCursorY, CURSOR_STATUS_SQUARE );
#endif
		break;
		
	case CURSOR_STATUS_ON:
	default:
		cursor_vars.bCursorBlinkAction = 1;
		cursor_vars.bCursorBlinkStatus = 1;
		cursor_vars.cursorTime = GetUptimeInMillisecond();
		
#ifdef _HARDWARE_CURSOR
		LCD_MoveCursor( cursor_vars.locCursorX, cursor_vars.locCursorY );
		LCD_CursorOnOff( 1, 0 );
#else
		GS_DrawCursor( cursor_vars.locCursorX, cursor_vars.locCursorY, CURSOR_STATUS_ON );
#endif
		break;
	}	
}

#ifndef _HARDWARE_CURSOR
static void GS_DrawCursor_XorCore( int x, int y, cursor_status_t status )
{
	int x2, y2;

	switch( status ) {
	case CURSOR_STATUS_ON:
		x = x * CHAR_WIDTH;
		y = y * CHAR_HEIGHT + ( CHAR_HEIGHT - 1 );

		GS_DrawHorizontalLineInverse( x, y, CHAR_WIDTH );
		break;

	case CURSOR_STATUS_SQUARE:
		x = x * CHAR_WIDTH;
		y = y * CHAR_HEIGHT;
		x2 = x + CHAR_WIDTH;
		y2 = y + CHAR_HEIGHT;

		GS_DrawSolidRectangleInverse( x, y, x2, y2 );
		break;

	default:
		break;
	}
}

static void GS_DrawCursor( int x, int y, cursor_status_t status )
{
	GS_DrawCursor_XorCore( x, y, status );
}

static void GS_EraseCursor( int x, int y, cursor_status_t status )
{
	GS_DrawCursor_XorCore( x, y, status );
}
#endif /* _HARDWARE_CURSOR */
