#ifndef __UI_STATE_CHECK_H__
#define __UI_STATE_CHECK_H__

/* Off-hook: It jump to UI_STATE_DIAL normally. */
extern int StateOnKeyProcessOffHook( void );

/* On-hook: It jump to UI_STATE_STANDBY normally. */
extern int StateOnKeyProcessOnHook( void );

/* Current state in connection? */
extern int CheckStatesInConnection( void );

#endif /* __UI_STATE_CHECK_H__ */

