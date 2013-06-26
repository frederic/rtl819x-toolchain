#ifndef __UI_EVENT_HANDLER_H__
#define __UI_EVENT_HANDLER_H__

#include "ui_event_def.h"

/* Message type for main loop */
typedef enum {
	RET_OK = 0,
	RET_NOT_INIT, 
	RET_ASK_EXIT,
} ui_launcher_ret_t;

/* extern functions */
extern void InitializeEventHandler( void );
extern void TerminateTimer( void );
extern void TerminateEventHandler( void );
extern ui_launcher_ret_t UIEventLauncher( int bNoWait );

#endif /* __UI_EVENT_HANDLER_H__ */

