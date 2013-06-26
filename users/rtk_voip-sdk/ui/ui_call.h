#ifndef __UI_CALL_H__
#define __UI_CALL_H__

#include "ui_event_def.h"	/* sid_t */
#include "ui_event_send.h"

#define IP_SID_INIT		0	/* used for outgoing call */
#define IP_SID_TEMP		0

/* Enter Dial State */
extern void DoEditDialNumberWithStateTransition( int bHandfree, unsigned char key );

/* ---------------------------------------------------------------------------- */
/* SIP to UI events */
/* ---------------------------------------------------------------------------- */
/* Incoming call. It is possible standby state or in-connection state */
extern void API_IncomingCallToUI( sid_t sid, const unsigned char *pszPhonenumber );

/* Connection est. indication */
extern void API_ConnectionEstToUI( sid_t sid );

/* Disconnection indication */
extern void API_DisconnectToUI( sid_t sid );

/* Seession be held indication */
extern void API_LineHeldToUI( sid_t sid );

/* Line resume indication */
extern void API_LineResumeToUI( sid_t sid );

/* ---------------------------------------------------------------------------- */
/* UI to SIP events */
/* ---------------------------------------------------------------------------- */
/* Allocate / release line in STANDBY state. */
extern void API_AllocateLineForCall( sid_t sid, unsigned int bConnected );
extern void API_ReleaseLineForCall( void );

/* Allocate line in CONNECTION state */
extern void API_ActivateLineInConnection( sid_t sid, unsigned int bHoldResume );

/* Accept / reject incoming call in INCOMING_CALL state */
extern void API_AcceptIncomingCall( sid_t sid );
extern void API_RejectIncomingCall( void );

/* Do outgoing call in STANDBY state. */
extern void API_OutgoingCall( sid_t sid, const unsigned char *pszPhonenumber, ocf_t ocf );

/* Cancel outgoing call in OUTGOING_CALL state. */
extern void API_CancelOutgoingCall( sid_t sid, unsigned int bConnected );

/* Line switching (hold/resume/conference) in CONNECTION state. */
extern void API_LineSwitching( lsr_t lsr, unsigned int bCallTransfer, unsigned int bSwitchBack );

/* Send DTMF in CONNECTION state. */
extern void API_SendDTMF( sid_t sid, unsigned char dtmf );

/* Disconnect request in CONNECTION state. */
extern void API_DisconnectCall( void );

#endif /* __UI_CALL_H__ */

