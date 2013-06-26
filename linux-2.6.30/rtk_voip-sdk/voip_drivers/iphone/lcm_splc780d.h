#ifndef __LCM_SPLC780D_H__
#define __LCM_SPLC780D_H__

// Initialize 
extern void splc780d_InitLCM( void );

// Display, cursor and blink ON/OFF
extern void splc780d_DisplayOnOff( unsigned char display, 
							unsigned char cursor,
							unsigned char blink );

// Move cursor 
extern void splc780d_MoveCursorPosition( unsigned char x, unsigned char y );

// Draw text 
extern void splc780d_DrawText( unsigned char x, unsigned char y, 
						unsigned char *pszText, int len );



#endif /* __LCM_SPLC780D_H__ */

