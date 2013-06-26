#ifndef __IOCTL_KEYPAD_H__
#define __IOCTL_KEYPAD_H__

extern void InitializeKeypad( void );
extern void TerminateKeypad( void );
extern int GetKeypadInput( unsigned char *pKey );
extern int ClearKeypadInput( void );
extern int GetKeypadHookStatus( void );		/* 0: off hook, others: on hook */
extern void HookStatusIsAcknowledgeByUI( int status );	/* status, 1: on-hook, 0: off-hook */

#endif /* __IOCTL_KEYPAD_H__ */

