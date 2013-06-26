#ifndef __PT6961_H__
#define __PT6961_H__

// Initialize PT6961 
extern void pt6961_Initialize( void );

// Clear LED Display 
// @clear 0: turn off all LED, 1: turn on all LED 
extern void pt6961_ClearDisplay( int clear );

// Set LED Display 
// @data 32bits data to represent LED  
extern void pt6961_SetDisplay( unsigned long led );

// Get hook status 
// @return 0->on-hook, 1->off-hook
extern int pt6961_GetHookStatus( void );


#endif /* __PT6961_H__ */

