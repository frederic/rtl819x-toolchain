#ifndef __COM_IME_WUBIHUA_H__
#define __COM_IME_WUBIHUA_H__

extern void WuBiHuaInitialization( void );
extern void WuBiHuaTermination( void );
extern int  WuBiHuaKeyProcessor( unsigned char key );
extern void WuBiHuaTimerProcessor( void );

#endif /* __COM_IME_PINYIN_H__ */
