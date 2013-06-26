#ifndef __VOIP_TIMER_H__
#define __VOIP_TIMER_H__

// ==================================================
// voip time tick in unit of ms 
typedef unsigned long timetick_t;

#define TIMETICK_SEED	0xFFFF0000	// to make it wrap around after 65.536 seconds 

// global variable to hold time tick
extern timetick_t timetick;

// timetick compare function 
#define timetick_before( a, b )		\
		( ( long )( ( b ) - ( a ) ) > 0 ? 1 : 0 )
#define timetick_before_eq( a, b )		\
		( ( long )( ( b ) - ( a ) ) >= 0 ? 1 : 0 )
#define timetick_after( a, b )		\
		( ( long )( ( a ) - ( b ) ) > 0 ? 1 : 0 )
#define timetick_after_eq( a, b )		\
		( ( long )( ( a ) - ( b ) ) >= 0 ? 1 : 0 )


// ==================================================
// timer function definition 
typedef void ( *fn_timer_t )( void *priv );

// ==================================================
// register timer interfaces 
// period: > 0 means periodic timer, < 0 means one shoot timer 
extern int register_timer( fn_timer_t fn_timer, void *priv, long period );

#define register_timer_10ms( fn, priv )		register_timer( fn, priv, 10 )



#endif // __VOIP_TIMER_H__

