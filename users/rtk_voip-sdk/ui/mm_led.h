#ifndef __MM_LED_H__
#define __MM_LED_H__

/* Start blinking LED */
extern void MM_StartLedBlinking( unsigned long mask );

/* Stop blinking */
extern void MM_StopLedBlinking( void );

/* Timer event to blinking */
extern void MM_LedBlinkingTimerEvent( int bImmediately );

#endif /* __MM_LED_H__ */
