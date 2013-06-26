#ifndef __COM_IME_ENGLISH_H__
#define __COM_IME_ENGLISH_H__


/* static function for each IME */
extern void EnglishImeInitialization( void );
extern int  EnglishImeKeyProcessor( unsigned char key );
extern void EnglishImeTimerProcessor( void );
extern void EnglishImeTermination( void );

#endif /* __COM_IME_ENGLISH_H__ */
