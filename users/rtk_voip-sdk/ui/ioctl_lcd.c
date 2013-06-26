#include "ui_config.h"
#include "ioctl_lcd.h"
#include "ioctl_kernel.h"

/* associated with LCD status */
#define LCD_STATUS_DISPLAY		0x00000001	/* bit 0: display on/off */
#define LCD_STATUS_CURSOR		0x00000002	/* bit 1: cursor on/off */
#define LCD_STATUS_CUR_BLINK	0x00000004	/* bit 0: cursor blink */

/* It is hardware status */
static unsigned long fLcdStatus;

void InitializeLCD( void )
{
	fLcdStatus = 0;

	//rtk_SetLcdDrawText( 0, 0, "Ready...", 8 );
	LCD_DisplayOnOff( 1 /* on */ );
	
	//rtk_SetLcdMoveCursorPosition( 0, 0, 1 );
}

void TerminateLCD( void )
{
	/* keep it display something */
	//LCD_DisplayOnOff( 0 /* off */ );
}

void LCD_DisplayOnOff( unsigned char bOnOff )
{
	if( bOnOff )
		fLcdStatus |= LCD_STATUS_DISPLAY;
	else
		fLcdStatus &= ~LCD_STATUS_DISPLAY;
	
	rtk_SetLcdDisplayOnOff( fLcdStatus & LCD_STATUS_DISPLAY,
							fLcdStatus & LCD_STATUS_CURSOR,
							fLcdStatus & LCD_STATUS_CUR_BLINK );
}

/* ===================================================================== */
#ifdef _HARDWARE_CURSOR
void LCD_CursorOnOff( unsigned char bOnOff, unsigned char bBlink )
{
	/*
	 * NOTE: we change cursor's behavior.
	 */
	if( bOnOff && bBlink ) {
		fLcdStatus |= LCD_STATUS_CUR_BLINK;
		fLcdStatus &= ~LCD_STATUS_CURSOR;
	} else if( bOnOff & !bBlink ) {
		fLcdStatus |= LCD_STATUS_CURSOR;
		fLcdStatus &= ~LCD_STATUS_CUR_BLINK;		
	} else
		fLcdStatus &= ~( LCD_STATUS_CURSOR | LCD_STATUS_CUR_BLINK );
		
#if 0
	if( bOnOff )
		fLcdStatus |= LCD_STATUS_CURSOR;
	else
		fLcdStatus &= ~LCD_STATUS_CURSOR;	

	if( bBlink )
		fLcdStatus |= LCD_STATUS_CUR_BLINK;
	else
		fLcdStatus &= ~LCD_STATUS_CUR_BLINK;
#endif
	
	rtk_SetLcdDisplayOnOff( fLcdStatus & LCD_STATUS_DISPLAY,
							fLcdStatus & LCD_STATUS_CURSOR,
							fLcdStatus & LCD_STATUS_CUR_BLINK );
}

void LCD_MoveCursor( int x, int y )
{
	rtk_SetLcdMoveCursorPosition( x, y );
}
#endif /* _HARDWARE_CURSOR */

/* ===================================================================== */
#ifdef _TEXT_MODE
void LCD_DrawText( int x, int y, unsigned char *pszText, int len )
{
	rtk_SetLcdDrawText( x, y, pszText, len );
}
/* ===================================================================== */
#else
  #ifdef LCD_COL2_ORIENTED
    #if defined( VRAM_MMAP ) && VRAM_MMAP == 2
void LCD_DirtyMmap2( int page, int col, int len, int rows )
{
	rtk_SetLcdDirtyMmap2( page, col, len, rows );
}
    #else
void LCD_WriteData2( int page, int col, const unsigned char *pdata, int len )
{
	rtk_SetLcdWriteData2( page, col, pdata, len );
}
    #endif /* VRAM_MMAP && VRAM_MMAP == 2 */
  #else
void LCD_WriteData( int start, const unsigned char *pdata, int len )
{
	rtk_SetLcdWriteData( start, pdata, len );
}
  #endif
#endif /* _TEXT_MODE */
