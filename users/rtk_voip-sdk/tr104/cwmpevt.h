#ifndef CWMP_EVT_MSG_H
#define CWMP_EVT_MSG_H

#include "kernel_config.h"
#include "voip_feature.h"
#include "rtk_voip.h"

#define	MAX_PORTS			16	//CON_CH_NUM 	///MAX channel number

typedef enum{
	EVT_VOICEPROFILE_LINE_GET_STATUS,
	EVT_VOICEPROFILE_LINE_SET_STATUS
}cwmpEvt;

typedef struct voiceProfileLineStatusObj{
	int profileID;	// RFU 
	int line;		// RFU 
	int incomingCallsReceived;
	int incomingCallsAnswered;
	int incomingCallsConnected;
	int incomingCallsFailed;
	int outgoingCallsAttempted;
	int outgoingCallsAnswered;
	int outgoingCallsConnected;
	int outgoingCallsFailed;
}voiceProfileLineStatus;

typedef struct cwmpEvtMsgObj{
	cwmpEvt event;
	voiceProfileLineStatus voiceProfileLineStatusMsg[MAX_PORTS];	
}cwmpEvtMsg;

/*new the evt msg*/
cwmpEvtMsg *cwmpEvtMsgNew(void);

/* free the evt msg */
void cwmpEvtMsgFree(cwmpEvtMsg *msg);

/*set and get the msg event*/
void cwmpEvtMsgSetEvent(cwmpEvtMsg *msg, cwmpEvt event);
cwmpEvt cwmpEvtMsgGetEvent(cwmpEvtMsg *msg);

/* set and get the voiceProfileLineStatus */
void cwmpEvtMsgSetVPLineStatus(cwmpEvtMsg *msg, voiceProfileLineStatus *VPLineStatus, int nPort);
voiceProfileLineStatus *cwmpEvtMsgGetVPLineStatus(cwmpEvtMsg *msg, int nPort);

/* the size of the evt msg */
int cwmpEvtMsgSizeof(void);

#endif /*CWMP_EVT_MSG_H*/
