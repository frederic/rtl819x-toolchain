#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>

#include "voip_manager.h"

#define DEMO_CHID			0
#define SELECT_WAIT_SEC		5

static void read_all_events_infinitely( void )
{
	extern int g_VoIP_Mgr_FD;
	
	TstFlushVoipEvent stFlushVoipEvent;
	
	int fd;
	fd_set fdset;
	struct timeval timeout;
	TstVoipEvent stVoipEvent;
	
	int ret;
	int i;
	
	// flush all voip events 
	stFlushVoipEvent.ch_id = DEMO_CHID;
	stFlushVoipEvent.type = VET_ALL;
	stFlushVoipEvent.mask = VEM_ALL;
	
	rtk_FlushVoIPEvent( &stFlushVoipEvent );
	
	// main loop 
	while( 1 ) {
		
		printf( "Wait %d seconds...", SELECT_WAIT_SEC );
		fflush( stdout );
		
		// set fd for select 
		FD_ZERO( &fdset );
		FD_SET( g_VoIP_Mgr_FD, &fdset );
		
		// set timeout for select
		timeout.tv_sec = SELECT_WAIT_SEC;
		timeout.tv_usec = 0;
		
		// select 
		ret = select( g_VoIP_Mgr_FD + 1, &fdset, NULL, NULL, &timeout );
		
		// print out information 
		if( ret == 0 ) {
			printf( "timeout\n" );
			continue;
		} 
		
		printf( "got it\n" );
		
		for( i = 0; ; i ++ ) {
			// fill input data & get it 
			stVoipEvent.ch_id = DEMO_CHID;
			stVoipEvent.type = VET_ALL;
			stVoipEvent.mask = VEM_ALL;
			
			rtk_GetVoIPEvent( &stVoipEvent );
			
			// no more data 
			if( stVoipEvent.id == VEID_NONE )
				break;
			
			// print out 
			printf( "[%d] on chid=%d\n", i, stVoipEvent.ch_id );
			printf( "\ttype=%08X mask=%08X\n", stVoipEvent.type, stVoipEvent.mask );
			printf( "\tID=%08X p0=%u p1=%u @%08X\n", stVoipEvent.id, 
							stVoipEvent.p0, stVoipEvent.p1, stVoipEvent.time );
		}	// read all events 
	
	}	// infinite loop 
}


int voip_event_main(int argc, char *argv[])
{
	read_all_events_infinitely();
	
	return 0;
}

