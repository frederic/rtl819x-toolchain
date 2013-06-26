#include <sys/msg.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "ui_config.h"
#include "ui_exit.h"
#include "ui_event_def.h"
#include "ui_host.h"
#include "res.h"

static void IpPhoneUIExitHandler( int sig )
{
	extern int AnnounceExitSignalIsArrived( void );

	if( AnnounceExitSignalIsArrived() == 0 ) {
		/* fail to send message, so exit immediately. */
		exit( 0 );
	}
}

void InitializeExitHandler( void )
{
	//signal( SIGINT, IpPhoneUIExitHandler );	/* ctrl-C */
	//signal( SIGKILL, IpPhoneUIExitHandler );	/* another possible */
	signal( SIGTERM, IpPhoneUIExitHandler );
}

void DisplayExitMessage( void )
{
	SetupFrameWithCentralizedString( szRebooting );
}


