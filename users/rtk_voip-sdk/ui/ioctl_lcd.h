#ifndef __IOCTL_LCD_H__
#define __IOCTL_LCD_H__

#include "ioctl_kernel.h"

extern void InitializeLCD( void );
extern void TerminateLCD( void );
extern void LCD_DisplayOnOff( unsigned char bOnOff );
extern void LCD_CursorOnOff( unsigned char bOnOff, unsigned char bBlink );
extern void LCD_MoveCursor( int x, int y );

#ifndef _TEXT_MODE
  #ifdef LCD_COL2_ORIENTED
    #if defined( VRAM_MMAP ) && VRAM_MMAP == 2
	extern void LCD_DirtyMmap2( int page, int col, int len, int rows );
    #else
    extern void LCD_WriteData2( int page, int col, const unsigned char *pdata, int len );
    #endif
  #else
    extern void LCD_WriteData( int start, const unsigned char *pdata, int len );
  #endif
#else
extern void LCD_DrawText( int x, int y, unsigned char *pszText, int len );
#endif

#define LCD_SCREEN_WIDTH	16
#define LCD_SCREEN_HEIGHT	2

#endif /* __IOCTL_LCD_H__ */

