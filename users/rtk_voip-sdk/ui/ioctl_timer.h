#ifndef __IOCTL_TIMER_H__
#define __IOCTL_TIMER_H__

extern void InitializeTimer( void );
extern int GetTimerEvent( void );
extern int ChangeTimerPeriod( unsigned long nMilliSecond );

#endif /* __IOCTL_TIMER_H__ */

