#ifndef __COM_IME_PINYIN_H__
#define __COM_IME_PINYIN_H__

extern void PinYinInitialization( void );
extern void PinYinTermination( void );
extern int  PinYinKeyProcessor( unsigned char key );
extern void PinYinTimerProcessor( void );

#endif /* __COM_IME_PINYIN_H__ */
