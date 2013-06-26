#ifndef __COM_IME_PHONETIC_H__
#define __COM_IME_PHONETIC_H__

extern void PhoneticInitialization( void );
extern void PhoneticTermination( void );
extern int  PhoneticKeyProcessor( unsigned char key );
extern void PhoneticTimerProcessor( void );

#endif /* __COM_IME_PHONETIC_H__ */
