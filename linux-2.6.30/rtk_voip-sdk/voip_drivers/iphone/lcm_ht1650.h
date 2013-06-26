#ifndef __LCM_HT1650_H__
#define __LCM_HT1650_H__

// initialize 
extern void ht1650_LCM_init( void );

// clear screen 
//extern void ht1650_ClearScreen( int color );

// write LCD data, start: 0 ~ 159 
extern void ht1650_WriteData( int start, const unsigned char *pdata, int len );

// turn on/off LCD 
extern void ht1650_DisplayOnOff( int onOff );

#endif /* __LCM_HT1650_H__ */

