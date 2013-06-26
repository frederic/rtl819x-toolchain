#ifndef __LCM_RTS2615_H__
#define __LCM_RTS2615_H__

// M-Square RTS26151 is a drive IC, so CPU need to refresh liquid in 50 to 150 Hz. 
// Since 8952/62 platform doesn't have DMA and other hardware to offload CPU, 
// this IC won't be chose. 

// initialize RTS26151 
extern void rts26151_LCM_init( void );

// write LCD data, start = 0 ~ 1024 for 128x64  
extern void rts26151_WriteData( int start, const unsigned char *pdata, int len );


#endif /* __LCM_RTS2615_H__ */

