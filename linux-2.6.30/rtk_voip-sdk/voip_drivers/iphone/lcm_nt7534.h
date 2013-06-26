#ifndef __LCM_NT7534_H__
#define __LCM_NT7534_H__

// Initialize 
extern void nt7534_LCM_init( void );

// Display ON/OFF (On: 1, Off: 0)
extern void nt7534_DisplayOnOff( int onOff );

// wirte on (page, col) in size of 'len' (wrap-around writing by software) 
extern void nt7534_WriteData( unsigned char page, unsigned char col, const unsigned char *pdata, int len );

// Clear Screen (color = 1 --> all black, color = 0 --> all white)
extern void nt7534_ClearScreen( int color );

// Draw a text on screen 
extern void nt7534_DrawText( unsigned char page, unsigned char col, unsigned char *pszText );


/*
         ==== (Page, Col) Layout ==== 

     +---------------------------------+
     |           Page 0                |
     +---------------------------------+
     |           Page 1                |
     +---------------------------------+
     :               :                 :
     +---------------------------------+
     |           Page 8                |
     +---------------------------------+
 Col  0 .......................... 131 (83h)

   * A page contains 8 * 132 bits
   * After writing, 'col' is increasing and stops with 83h. 

*/

#endif /* __LCM_NT7534_H__ */

