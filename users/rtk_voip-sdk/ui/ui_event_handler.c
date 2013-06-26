#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/msg.h>
#include <pthread.h>
#include "ui_config.h"
#include "ui_event_handler.h"
#include "ioctl_timer.h"
#include "ioctl_keypad.h"
#include "ui_host.h"
#include "ui_call.h"

static int qidEventUI = -1;

/* *************************************************************** */
/* Initialize and Terminate */
/* *************************************************************** */
void InitializeEventHandler( void )
{
	key_t key;
	
	if( ( key = ftok( UI_CTRL_PATHNAME, UI_CTRL_PRJ_ID_OWNER_UI ) ) == -1 )
		debug_out( "File not exist?? " UI_CTRL_PATHNAME "\n" );
	
	if( ( qidEventUI = msgget( key, 0666 | IPC_CREAT ) ) == -1 ) {
		debug_out( "Create UI event queue fail.\n" );
	}
	
	debug_out( "qidEventUI: %d (K:%d)\n", qidEventUI, key );

#ifdef UI_EVENTS_USE_IPC_MODE
	extern void InitializeEventHandler_Send( void );
	
	InitializeEventHandler_Send();
#endif /* UI_EVENTS_USE_IPC_MODE */
}

void TerminateEventHandler( void )
{
	if( qidEventUI != -1 ) {
		if( msgctl( qidEventUI, IPC_RMID, 0 ) == -1 ) {
                debug_out( "msgctl(UI) fail: %d\n", errno );
        }
	}

#ifdef UI_EVENTS_USE_IPC_MODE 
	extern void TerminateEventHandler_Send( void );
	
	TerminateEventHandler_Send();
#endif /* UI_EVENTS_USE_IPC_MODE */ 
}

/* *************************************************************** */
/* Announce events to handler(this one), so use by SIG handler only*/
/* *************************************************************** */
/* for ioctl_timer only */
int AnnounceTimerSignalIsArrived( void )
{
	tlv_event_header_t tlv_header;
	
	tlv_header.type = UI_EVENT_TYPE_TIMER_SIG;
	tlv_header.len = 0;

	if( msgsnd( qidEventUI, &tlv_header, tlv_header.len + sizeof( tlv_header.len ), 0 ) == -1 ) {
		debug_out( "Fail to send timer signal.\n" );
		return 0;
	}
	
#if 0	// TODO: remvoe for test only
	tlv_header.type = UI_EVENT_TYPE_KEYPAD_SIG;
	tlv_header.len = 0;
	
	write( hCtrlFifo, &tlv_header, sizeof( tlv_header ) );
#endif
		
	return 1;
}

/* for ioctl_keypad only */
int AnnounceKeypadSignalIsArrived( void )
{
	tlv_event_header_t tlv_header;
	
	tlv_header.type = UI_EVENT_TYPE_KEYPAD_SIG;
	tlv_header.len = 0;

	if( msgsnd( qidEventUI, &tlv_header, tlv_header.len + sizeof( tlv_header.len ), 0 ) == -1 ) {
		debug_out( "Fail to send keypad signal.\n" );
		return 0;
	}

	return 1;
}

/* for exit only */
int AnnounceExitSignalIsArrived( void )
{
	tlv_event_header_t tlv_header;
	
	tlv_header.type = UI_EVENT_TYPE_EXIT;
	tlv_header.len = 0;

	if( msgsnd( qidEventUI, &tlv_header, tlv_header.len + sizeof( tlv_header.len ), 0 ) == -1 ) {
		debug_out( "Fail to send exit signal.\n" );
		return 0;
	}

	return 1;
}

/* *************************************************************** */
/* Announce events to handler(this one), and it announced by thread */
/* *************************************************************** */
/* for ping an ip */
static unsigned char bOwnByPingThread = 0;
static unsigned char szPingIpAddr[ MAX_LEN_OF_IP_ADDR + 1 ];
static unsigned long idPingThread = 1;

static void PingCoreThread( void *ptr )
{
	unsigned char szCommand[ 256 ];
	unsigned long id;
	int ret;
	tlv_event_ping_t tlv_event_ping;
	
	if( !bOwnByPingThread )
		return;

	/* copy data */
	id = idPingThread;
	if( ++ idPingThread == 0 )
		idPingThread = 1;

	sprintf( ( char * )szCommand, "ping %s -c 1", szPingIpAddr );

	bOwnByPingThread = 0;

	/* do ping */
	debug_out( "execute: %s\n", szCommand );
	
	ret = system( szCommand );
	
	/* ok. send message */
	tlv_event_ping.type = UI_EVENT_TYPE_PING_ACK;
	tlv_event_ping.len = TLV_EVENT_PAYLOAD_SIZE( tlv_event_ping );
	tlv_event_ping.idPing = id;	/* ping thread identifier */
	tlv_event_ping.result = ( ( ret == 0 ) ? 1 : 0 );	/* 1: ok, 0: fail */
	
	if( msgsnd( qidEventUI, &tlv_event_ping, tlv_event_ping.len + sizeof( tlv_event_ping.len ), 0 ) == -1 ) {
		debug_out( "Fail to send ping ack.\n" );
	}	
}

unsigned long AnnouncePingRequestThread( const unsigned char *pszIp )
{
	pthread_t thread_ping;
	unsigned long id;
	
	if( bOwnByPingThread )
		return 0;
	
	strcpy( szPingIpAddr, pszIp );
	id = idPingThread;
	
	bOwnByPingThread = 1;

	pthread_create( &thread_ping, NULL,
                    (void*)&PingCoreThread, (void*) NULL);

	return id;
}

/* *************************************************************** */
/* Communication with ioctl */
/* *************************************************************** */


/* *************************************************************** */
/* Communication with main loop */
/* *************************************************************** */
ui_launcher_ret_t UIEventLauncher( int bNoWait )
{
	extern void API_PingAckToUI( unsigned long idPing, unsigned long result );
	extern void API_SipRegisterToUI( register_status_t status );

	tlv_event_super_t tlv_event;
	unsigned char event_key;

	int ret;

	/* Not initialize yet, or initialize fail */
	if( qidEventUI == -1 )
		return RET_NOT_INIT;	

	while( 1 ) {

		/* Try to get a message */
		ret = msgrcv( qidEventUI, &tlv_event, SIZE_OF_TLV_EVENT_SUPER, 0, ( bNoWait ? IPC_NOWAIT : 0 ) );
		
		//debug_out( "MR:%d,%ld\n", ret, tlv_event.header.type );
		
		if( ret == -1 )	/* no more data */
			break;
		
		/* Now, header is correct and event placed in event_buffer */
		switch( tlv_event.header.type ) {
		case UI_EVENT_TYPE_TIMER_SIG:
			if( GetTimerEvent() )
				UI_TimerEvent();
			break;
			
		case UI_EVENT_TYPE_KEYPAD_SIG:
			if( GetKeypadInput( &event_key ) ) {
				UI_KeypadInput( event_key );
			}
			break;
			
		case UI_EVENT_TYPE_EXIT:
			return RET_ASK_EXIT;
			break;
			
		case UI_EVENT_TYPE_INCOMING_CALL_IND:	/* incoming call */
			API_IncomingCallToUI( tlv_event.call.sid, tlv_event.call.phonenumber );
			break;
	
		case UI_EVENT_TYPE_CONNECTION_EST_IND:	/* connection */
			API_ConnectionEstToUI( tlv_event.sid.sid );
			break;
			
		case UI_EVENT_TYPE_DISCONNECT_IND:		/* disconnect ind. */
			API_DisconnectToUI( tlv_event.discInd.sid );
			break;

		case UI_EVENT_TYPE_HELD_IND:			/* held ind. */
			API_LineHeldToUI( tlv_event.sid.sid );
			break;

		case UI_EVENT_TYPE_RESUME_IND:			/* resume ind. */
			API_LineResumeToUI( tlv_event.sid.sid );
			break;
			
		case UI_EVENT_TYPE_PING_ACK:
			API_PingAckToUI( tlv_event.ping.idPing, tlv_event.ping.result );
			break;
			
		case UI_EVENT_TYPE_SOLAR_RESTART_IND:
			debug_out( "Rcv UI_EVENT_TYPE_SOLAR_RESTART_IND\n" );
			break;
			
		case UI_EVENT_TYPE_SIP_REGISTER_IND:
			API_SipRegisterToUI( tlv_event.registering.status );
			break;

		default:
			debug_out( "Unexpected message type:%lu\n", tlv_event.header.type );
			break;
		}		
		
	}

	return RET_OK;	/* OK */
}

