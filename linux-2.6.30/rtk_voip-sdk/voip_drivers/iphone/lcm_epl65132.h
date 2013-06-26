#ifndef __LCM_EPL65132_H__
#define __LCM_EPL65132_H__

// Initialize 
extern void epl65132_LCM_init( void );

// Display ON/OFF (On: 1, Off: 0)
extern void epl65132_DisplayOnOff( int onOff );

// wirte on (page, col) in size of 'len' (wrap-around writing by software) 
extern void epl65132_WriteData( unsigned char page, unsigned char col, const unsigned char *pdata, int len );

// dirty MMAP starting at (page, col) and ending at (page+len, col+rows)
extern void epl65132_DirtyMmap( unsigned char page, unsigned char col, unsigned char len, unsigned char rows );

// Clear Screen (color = 1 --> all black, color = 0 --> all white)
extern void epl65132_ClearScreen( int color );

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

#endif /* __LCM_EPL65132_H__ */

