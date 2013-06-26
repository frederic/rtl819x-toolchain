#ifndef __UI_EVENT_DEF_H__
#define __UI_EVENT_DEF_H__

#include "ui_limits.h"

#define UI_EVENTS_USE_IPC_MODE		/* use ipc (stand alone application), or combine with SIP */

/* event queue information */
//#define UI_CTRL_PATHNAME			"/mnt/nfs/ipphone/" "ui"
#define UI_CTRL_PATHNAME			"/bin/" "ui"
#define UI_CTRL_PRJ_ID_OWNER_UI		0x01
#define UI_CTRL_PRJ_ID_OWNER_SIP	0x02

/* type for SIP */
typedef unsigned long sid_t;	/* line id */

typedef enum {
	LSR_LINE1	= 0x0001,
	LSR_LINE2	= 0x0002,
	//LSR_LINE3	= 0x0004,	/* for future reference */
	//LSR_LINE4	= 0x0008,
	LSR_CONFERENCE	= LSR_LINE1 | LSR_LINE2,	/* conference -> select two lines */
} lsr_t;

typedef unsigned int fxom_t;	/* FXO line bit Mask */
#define FXOM_NONE		0x0000
#define FXOM_LINE1		0x0001
#define FXOM_LINE2		0x0002
//#define FXOM_LINE3		0x0004	/* for future reference */
//#define FXOM_LINE4		0x0008

typedef enum {
	SIP_NOT_REGISTER,	/* no register account */
	SIP_REGISTERING,	/* try to register */
	SIP_REGISTERED,		/* registered */
} register_status_t;

/* TLV definition */
typedef struct tlv_event_header_s {
	unsigned long type;
	unsigned long len;
} tlv_event_header_t;

typedef struct tlv_event_general_s {
	unsigned long type;
	unsigned long len;
	unsigned char value[ 1 ];
} tlv_event_general_t;

typedef struct tlv_event_phone_call_s {
	unsigned long type;
	unsigned long len;	
	sid_t		  sid;
	unsigned long connected;	/* for outgoing call only. */
	unsigned int  transfer;		/* for outgoing call only. call transfer */
	unsigned int  fxo;			/* for outgoing call only. FXO so ignore phonenumber */
	unsigned char phonenumber[ MAX_LEN_OF_PHONENUMBER + 1 ];
} tlv_event_phone_call_t;	/* for incoming/outgoing call */

typedef struct tlv_event_sid_s {
	unsigned long type;
	unsigned long len;	
	sid_t		  sid;
	unsigned long value;	/* different type has different meaning */
} tlv_event_sid_t;

typedef struct tlv_event_accept_s {
	unsigned long type;		/* UI_EVENT_TYPE_INCOMING_CALL_ACCEPT_REQ */
	unsigned long len;	
	sid_t		  sid;
} tlv_event_accept_t;

typedef struct tlv_event_reject_s {
	unsigned long type;		/* UI_EVENT_TYPE_INCOMING_CALL_REJECT_REQ */
	unsigned long len;
} tlv_event_reject_t;

typedef struct tlv_event_dtmf_s {
	unsigned long type;		/* UI_EVENT_TYPE_SEND_DTMF_REQ */
	unsigned long len;
	sid_t		  sid;
	unsigned char dtmf;
} tlv_event_dtmf_t;

typedef struct tlv_event_ping_s {
	unsigned long type;		/* UI_EVENT_TYPE_PING_ACK */
	unsigned long len;
	unsigned long idPing;	/* ping thread identifier */
	unsigned long result;	/* 1: ok, 0: fail */
} tlv_event_ping_t;

typedef struct tlv_event_line_s {
	unsigned long type;		/* UI_EVENT_TYPE_LINE_ALLOCATION_REQ */
	unsigned long len;
	sid_t		  sid;
	unsigned int  connected;
} tlv_event_line_t;

typedef struct tlv_event_release_s {
	unsigned long type;		/* UI_EVENT_TYPE_LINE_RELEASE_REQ */
	unsigned long len;
} tlv_event_release_t;

typedef struct tlv_event_fxo_s {
	unsigned long type;		/* UI_EVENT_TYPE_SWITCH_FXO_REQ */
	unsigned long len;
	sid_t		  sid;
	unsigned int  enable;	/* enable or disable */
	unsigned int  connected;
} tlv_event_fxo_t;

typedef struct tlv_event_line_switch_s {
	unsigned long type;		/* UI_EVENT_TYPE_LINE_SWITCHING_REQ */
	unsigned long len;
	lsr_t		  lsr;		/* type of Line Switching Request */
	unsigned int  transfer;
	unsigned int  back;		/* switch back, during dialing 2nd outgoing call */
	unsigned long value;
} tlv_event_line_switch_t;

typedef struct tlv_event_outgoing_cancel_s {
	unsigned long type;		/* UI_EVENT_TYPE_OUTGOING_CALL_CANCEL_REQ */
	unsigned long len;
	sid_t		  sid;
	unsigned int  connected;
} tlv_event_outgoing_cancel_t;

typedef struct tlv_event_disconnect_s {
	unsigned long type;		/* UI_EVENT_TYPE_DISCONNECT_REQ */
	unsigned long len;
} tlv_event_disconnect_t;

typedef struct tlv_event_disc_ind_s {
	unsigned long type;		/* UI_EVENT_TYPE_DISCONNECT_IND */
	unsigned long len;	
	sid_t		  sid;
} tlv_event_disc_ind_t;

typedef struct tlv_event_solar_restart_s {
	unsigned long type;		/* UI_EVENT_TYPE_SOLAR_RESTART_IND */
	unsigned long len;
	unsigned int  reason;
} tlv_event_solar_restart_t;

typedef struct tlv_event_sip_register_s {
	unsigned long type;		/* UI_EVENT_TYPE_SIP_REGISTER_IND */
	unsigned long len;
	register_status_t status;
} tlv_event_sip_register_t;
	
typedef union tlv_event_super_s {
	tlv_event_header_t	header;
	tlv_event_general_t	general;
	tlv_event_phone_call_t	call;
	tlv_event_sid_t sid;
	tlv_event_accept_t accept;
	tlv_event_reject_t reject;
	tlv_event_dtmf_t dtmf;
	tlv_event_ping_t ping;
	tlv_event_line_t line;
	tlv_event_release_t release;
	tlv_event_line_switch_t lsr;	/* Line Switch Request */
	tlv_event_outgoing_cancel_t cancel;
	tlv_event_disconnect_t disconnect;	/* disconnect req. */
	tlv_event_disc_ind_t discInd;		/* disconnect ind. */
	tlv_event_solar_restart_t restart;
	tlv_event_sip_register_t registering;
} tlv_event_super_t;

#define SIZE_OF_TLV_EVENT_SUPER		( sizeof( tlv_event_super_t ) - sizeof( unsigned long ) )
#define TLV_EVENT_PAYLOAD_SIZE( t )	( sizeof( t ) - sizeof( tlv_event_header_t ) )

enum {	/* Type of TLV event */
	UI_EVENT_TYPE_EINVAL,
	/* Communicate with driver */
	UI_EVENT_TYPE_TIMER_SIG,
	UI_EVENT_TYPE_KEYPAD_SIG,
	
	/* UI Control Command */
	UI_EVENT_TYPE_EXIT,
	
	/* Communicate with solar */
	/* IND: UI <- solar -> UI */
	/* REQ: UI -> solar */
	/* NTI: UI -> solar (Notification) */
	UI_EVENT_TYPE_SOLAR_START,		/* ---- Start ---- */
		
	UI_EVENT_TYPE_INCOMING_CALL_IND,			/* incoming call */
	UI_EVENT_TYPE_INCOMING_CALL_ACCEPT_REQ,		/* accept incoming call */
	UI_EVENT_TYPE_INCOMING_CALL_REJECT_REQ,		/* reject incoming call */

	UI_EVENT_TYPE_LINE_ALLOCATION_REQ,			/* allocate a line for outgoing call */
	UI_EVENT_TYPE_LINE_RELEASE_REQ,				/* release a line from outgoing call */
	
	UI_EVENT_TYPE_OUTGOING_CALL_REQ,			/* do outgoing call */
	UI_EVENT_TYPE_OUTGOING_CALL_CANCEL_REQ,		/* cancel outgoing call */
	
	UI_EVENT_TYPE_CONNECTION_EST_IND,			/* connection */
	UI_EVENT_TYPE_SEND_DTMF_REQ,				/* snd DTMF */
	
	UI_EVENT_TYPE_DISCONNECT_REQ,				/* disconnect req. */
	UI_EVENT_TYPE_DISCONNECT_IND,				/* disconnect ind. */

	UI_EVENT_TYPE_LINE_SWITCHING_REQ,			/* line switching: hold/resume/conference */
	UI_EVENT_TYPE_HELD_IND,						/* hold ack or be held */
	UI_EVENT_TYPE_RESUME_IND,					/* resume indication */
	
	UI_EVENT_TYPE_SOLAR_RESTART_IND,			/* solar restart & flash update */
	UI_EVENT_TYPE_SIP_REGISTER_IND,				/* SIP register status */
	
	UI_EVENT_TYPE_SOLAR_END,		/* ---- End ---- */
	
	/* Communicate with flash */
	UI_EVENT_FLASH_WRITE,
	UI_EVENT_FLASH_WRITE_ACK,
	UI_EVENT_FLASH_READ,
	UI_EVENT_FLASH_READ_ACK,
	
	/* ping event */
	UI_EVENT_TYPE_PING_ACK,

	UI_EVENT_THE_LAST_ONE,	/* keep it to be last one */	
};

#endif /* __UI_EVENT_DEF_H__ */

