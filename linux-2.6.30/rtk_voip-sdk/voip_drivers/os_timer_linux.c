#include <linux/timer.h>
#include "os_timer_core.h"

#if ( 1000 % HZ ) != 0 
#error "Not support this HZ"
#endif

static struct timer_list os_linux_timer;

static void os_linux_timer_func( unsigned long data )
{
#if ( 1000 / HZ ) < 10
	// jiffies period < 10ms ==> announce timer in 10 ms 
	increase_timetick( 10 /* 10ms */ );
	
	mod_timer( &os_linux_timer, jiffies + 10 / ( 1000 / HZ ) );
#else
	// jiffies period >= 10ms 
	increase_timetick( 1000 / HZ );
	
	mod_timer( &os_linux_timer, jiffies + 1 );
#endif
}

void start_os_timer( void )
{
	init_timer( &os_linux_timer );
	
	os_linux_timer.expires = jiffies + 1;
	os_linux_timer.function = os_linux_timer_func;
	
	add_timer( &os_linux_timer );
}

