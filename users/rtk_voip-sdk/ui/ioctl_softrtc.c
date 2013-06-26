#include <sys/time.h>
#include <stdio.h>
#include "ui_config.h"
#include "ioctl_softrtc.h"

/*
 * This file has NO complete function of soft RTC, and only 
 * provide a program uptime.
 * This is to make up our requirement, which LINUX does not provide.
 */

uptime_t GetUptimeInMillisecond( void )
{
	uptime_t now_millisecond;
	struct timeval now_tv; 
	
	if( gettimeofday( &now_tv, NULL ) != 0 ) {
		printf( "gettimeofday error\n" );
		return 0;
	}

	now_millisecond = now_tv.tv_sec * 1000 +
					  now_tv.tv_usec / 1000;
	
	return now_millisecond;
}

unsigned long GetUptimeInSecond( void )
{
	struct timeval now_tv; 
	
	if( gettimeofday( &now_tv, NULL ) != 0 ) {
		printf( "gettimeofday error\n" );
		return 0;
	}

	return now_tv.tv_sec;	
}

