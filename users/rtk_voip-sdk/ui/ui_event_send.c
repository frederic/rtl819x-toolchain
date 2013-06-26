#include <stdio.h>
#include <errno.h>
#include <sys/msg.h>
#include <string.h>
#include "ui_config.h"
#include "ui_event_def.h"
#include "ui_event_send.h"

#define IP_SID_TEMP		0

/* *************************************************************** */
/* Initialize and Terminate */
/* *************************************************************** */
#ifdef UI_EVENTS_USE_IPC_MODE
static int qidEventSip;

void InitializeEventHandler_Send( void )
{
	key_t key;
	
	key = ftok( UI_CTRL_PATHNAME, UI_CTRL_PRJ_ID_OWNER_SIP );
	
	if( ( qidEventSip = msgget( key, 0666 | IPC_CREAT ) ) == -1 ) {
		debug_out( "Create Sip event queue fail.\n" );
	}

	debug_out( "qidEventSip: %d (K:%d)\n", qidEventSip, key );
}

void TerminateEventHandler_Send( void )
{
	if( qidEventSip != -1 ) {
		if( msgctl( qidEventSip, IPC_RMID, 0 ) == -1 ) {
                debug_out( "msgctl(Sip) fail: %d\n", errno );
        }
	}
}
#endif /* UI_EVENTS_USE_IPC_MODE */

/* *************************************************************** */
/* Send Event Functions */
/* *************************************************************** */

/* Incoming call */
void API_AcceptIncomingCall( sid_t sid )
{
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_INCOMING_CALL_ACCEPT_REQ */
	tlv_event_accept_t tlv_event;
	
	tlv_event.type = UI_EVENT_TYPE_INCOMING_CALL_ACCEPT_REQ;
	tlv_event.len = TLV_EVENT_PAYLOAD_SIZE( tlv_event_accept_t );
	tlv_event.sid = sid;
	
	if( msgsnd( qidEventSip, &tlv_event, tlv_event.len + sizeof( tlv_event.len ), 0 ) == -1 ) {
		debug_out( "Fail to send incoming call accept.\n" );
		return;
	}
#else
	
#endif
}

void API_RejectIncomingCall( void )
{
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_INCOMING_CALL_REJECT_REQ */
	tlv_event_reject_t tlv_event;
	
	tlv_event.type = UI_EVENT_TYPE_INCOMING_CALL_REJECT_REQ;
	tlv_event.len = TLV_EVENT_PAYLOAD_SIZE( tlv_event_reject_t );
	
	if( msgsnd( qidEventSip, &tlv_event, tlv_event.len + sizeof( tlv_event.len ), 0 ) == -1 ) {
		debug_out( "Fail to send incoming call reject.\n" );
		return;
	}
#else
	
#endif
}

/* Allocate a line for outgoing call */
void API_AllocateLineForCall( sid_t sid, unsigned int bConnected )
{
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_LINE_ALLOCATION_REQ */
	tlv_event_line_t tlv_event;
	
	tlv_event.type = UI_EVENT_TYPE_LINE_ALLOCATION_REQ;
	tlv_event.len = TLV_EVENT_PAYLOAD_SIZE( tlv_event_line_t );
	tlv_event.sid = sid;
	tlv_event.connected = bConnected;

	if( msgsnd( qidEventSip, &tlv_event, tlv_event.len + sizeof( tlv_event.len ), 0 ) == -1 ) {
		debug_out( "Fail to send line allocation req.\n" );
		return;
	}	
#else
#endif
}

/* Release a line from outgoing call */
void API_ReleaseLineForCall( void )
{
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_LINE_RELEASE_REQ */
	tlv_event_release_t tlv_event;
	
	tlv_event.type = UI_EVENT_TYPE_LINE_RELEASE_REQ;
	tlv_event.len = TLV_EVENT_PAYLOAD_SIZE( tlv_event_release_t );

	if( msgsnd( qidEventSip, &tlv_event, tlv_event.len + sizeof( tlv_event.len ), 0 ) == -1 ) {
		debug_out( "Fail to send line release req.\n" );
		return;
	}	
#else
#endif	
}

#if 0
/* Switch a line for FXO outgoing call */
void API_SwitchLineToFXO( sid_t sid, unsigned int bEnable, unsigned int bConnected )
{
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_SWITCH_FXO_REQ */
	tlv_event_fxo_t tlv_event;
	
	tlv_event.type = UI_EVENT_TYPE_SWITCH_FXO_REQ;
	tlv_event.len = TLV_EVENT_PAYLOAD_SIZE( tlv_event_fxo_t );
	tlv_event.sid = sid;
	tlv_event.enable = bEnable;
	tlv_event.connected = bConnected;

	if( msgsnd( qidEventSip, &tlv_event, tlv_event.len + sizeof( tlv_event.len ), 0 ) == -1 ) {
		debug_out( "Fail to send line allocation req.\n" );
		return;
	}

	printf( "API_SwitchLineToFXO\n" );
#else
#endif	
}
#endif 

/* Outgoing call */
void API_OutgoingCall( sid_t sid, const unsigned char *pszPhonenumber, ocf_t ocf )
{
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_OUTGOING_CALL_REQ */
	int i;
	tlv_event_phone_call_t tlv_event;
	
	tlv_event.type = UI_EVENT_TYPE_OUTGOING_CALL_REQ;
	tlv_event.len = TLV_EVENT_PAYLOAD_SIZE( tlv_event_phone_call_t );
	tlv_event.sid = sid;
	tlv_event.connected = ( ( ocf & OCF_CONNECTED ) ? 1 : 0 );
	tlv_event.transfer = ( ( ocf & OCF_TRANSFER ) ? 1 : 0 );
	tlv_event.fxo = ( ( ocf & OCF_FXO ) ? 1 : 0 );
	
	strcpy( tlv_event.phonenumber, pszPhonenumber );
	
	// FIXME: convert '*' to '.' unconditionally. 
	for( i = 0; i < tlv_event.len; i ++ )
		if( tlv_event.phonenumber[ i ] == '*' )
			tlv_event.phonenumber[ i ] = '.';
	
	if( msgsnd( qidEventSip, &tlv_event, tlv_event.len + sizeof( tlv_event.len ), 0 ) == -1 ) {
		debug_out( "Fail to send outgoing call req.\n" );
		return;
	}
#else
	
#endif
}

void API_CancelOutgoingCall( sid_t sid, unsigned int bConnected )
{
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_OUTGOING_CALL_CANCEL_REQ */
	tlv_event_outgoing_cancel_t tlv_event;
	
	tlv_event.type = UI_EVENT_TYPE_OUTGOING_CALL_CANCEL_REQ;
	tlv_event.len = TLV_EVENT_PAYLOAD_SIZE( tlv_event_outgoing_cancel_t );
	tlv_event.sid = sid;
	tlv_event.connected = bConnected;
	
	if( msgsnd( qidEventSip, &tlv_event, tlv_event.len + sizeof( tlv_event.len ), 0 ) == -1 ) {
		debug_out( "Fail to send cancel outgoing call.\n" );
		return;
	}
#else
	
#endif
}

/* In connection */
void API_LineSwitching( lsr_t lsr, unsigned int bCallTransfer, unsigned int bSwitchBack )
{
	/* It assumes that this switching is appropriate. */
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_LINE_SWITCHING_REQ */
	tlv_event_line_switch_t tlv_event;

	tlv_event.type = UI_EVENT_TYPE_LINE_SWITCHING_REQ;
	tlv_event.len = TLV_EVENT_PAYLOAD_SIZE( tlv_event_line_switch_t );
	tlv_event.lsr = lsr;
	tlv_event.transfer = bCallTransfer;
	tlv_event.back = bSwitchBack;
	tlv_event.value = 0;	/* ignore */
	
	if( msgsnd( qidEventSip, &tlv_event, tlv_event.len + sizeof( tlv_event.len ), 0 ) == -1 ) {
		debug_out( "Fail to send line switcing.\n" );
		return;
	}	
#else
#endif
}

#if 0
void API_ConferenceCall( void )
{
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_CONFERENCE_CALL_REQ */
	tlv_event_header_t tlv_event;
	
	tlv_event.type = UI_EVENT_TYPE_CONFERENCE_CALL_REQ;
	tlv_event.len = 0;
	
	if( msgsnd( qidEventSip, &tlv_event, tlv_event.len + sizeof( tlv_event.len ), 0 ) == -1 ) {
		debug_out( "Fail to send conference call.\n" );
		return;
	}
#else
#endif
}

void API_AcceptCallWaiting( sid_t sid )
{
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_CALL_WAITING_ACCEPT_REQ */
	tlv_event_sid_t tlv_event;
	
	tlv_event.type = UI_EVENT_TYPE_CALL_WAITING_ACCEPT_REQ;
	tlv_event.len = TLV_EVENT_PAYLOAD_SIZE( tlv_event_sid_t );
	tlv_event.sid = sid;
	tlv_event.value = 0;	/* ignore */
	
	if( msgsnd( qidEventSip, &tlv_event, tlv_event.len + sizeof( tlv_event.len ), 0 ) == -1 ) {
		debug_out( "Fail to send accept call waiting.\n" );
		return;
	}
#else
#endif
}

void API_SessionHoldReq( sid_t sid )
{
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_HOLD_REQ */
	tlv_event_sid_t tlv_event;
	
	tlv_event.type = UI_EVENT_TYPE_HOLD_REQ;
	tlv_event.len = TLV_EVENT_PAYLOAD_SIZE( tlv_event_sid_t );
	tlv_event.sid = sid;
	tlv_event.value = 0;	/* ignore */
	
	if( msgsnd( qidEventSip, &tlv_event, tlv_event.len + sizeof( tlv_event.len ), 0 ) == -1 ) {
		debug_out( "Fail to send session hold req.\n" );
		return;
	}
#else
#endif
}
#endif

void API_SendDTMF( sid_t sid, unsigned char dtmf )
{
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_SEND_DTMF_REQ */
	tlv_event_dtmf_t tlv_event;
	
	tlv_event.type = UI_EVENT_TYPE_SEND_DTMF_REQ;
	tlv_event.len = TLV_EVENT_PAYLOAD_SIZE( tlv_event_dtmf_t );
	tlv_event.sid = sid;
	tlv_event.dtmf = dtmf;
	
	if( msgsnd( qidEventSip, &tlv_event, tlv_event.len + sizeof( tlv_event.len ), 0 ) == -1 ) {
		debug_out( "Fail to send DTMF.\n" );
		return;
	}
#else
	
#endif
}

/* Disconnection */
void API_DisconnectCall( void )
{
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_DISCONNECT_REQ */
	tlv_event_disconnect_t tlv_event;
	
	tlv_event.type = UI_EVENT_TYPE_DISCONNECT_REQ;
	tlv_event.len = TLV_EVENT_PAYLOAD_SIZE( tlv_event_disconnect_t );
	
	if( msgsnd( qidEventSip, &tlv_event, tlv_event.len + sizeof( tlv_event.len ), 0 ) == -1 ) {
		debug_out( "Fail to send disconnect req.\n" );
		return;
	}
#else
	
#endif
}

