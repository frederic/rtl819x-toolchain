#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include "ui_config.h"
#include "ioctl_timer.h"

/* to ensure there is only one timer event */
static unsigned char timer_r = 0, timer_w = 0;

#define DEFAULT_TIMER_PERIOD	500		/* 0.5 seconds */

static void TimerSignalHandler( int sig );

void InitializeTimer( void )
{	
	/* set signal handler */
	signal( SIGALRM, TimerSignalHandler );
	
	/* start timer */
	if( ChangeTimerPeriod( DEFAULT_TIMER_PERIOD ) == 0 )
		debug_out( "Initialize timer fail\n" );
}

void TerminateTimer( void )
{
	/* default signal handler */
	signal( SIGALRM, SIG_DFL );
	
	/* turn off timer */
	ChangeTimerPeriod( 0 );
}

/* Make sure that exist a timer event and mark it as read */
int GetTimerEvent( void )
{
	if( timer_r == timer_w )
		return 0;	/* Now it has no timer event?? */

	timer_r = timer_w;
	
	return 1;	/* ok */
}

int ChangeTimerPeriod( unsigned long nMilliSecond )
{
	struct itimerval interval;
	
	debug_out( "Timer Period: %lu(ms)\n", nMilliSecond );

	interval.it_interval.tv_sec = nMilliSecond / 1000;
	interval.it_interval.tv_usec = ( nMilliSecond % 1000 ) * 1000;
	interval.it_value = interval.it_interval;
	
	return ( setitimer( ITIMER_REAL, &interval, NULL ) == 0 );
}

static void TimerSignalHandler( int sig )
{
	extern int AnnounceTimerSignalIsArrived( void );
	
	if( timer_w != timer_r )
		return;		/* previous timer is not process yet. */
		
	if( AnnounceTimerSignalIsArrived() )
		timer_w = ( timer_w + 1 ) & 0x01;
}

