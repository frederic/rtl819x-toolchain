#include "cwmpevt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*new the evt msg*/
cwmpEvtMsg *cwmpEvtMsgNew(void){
	cwmpEvtMsg *msg = NULL;
	
	if((msg = (cwmpEvtMsg*)malloc(sizeof(cwmpEvtMsg))) == NULL )
		return NULL;
	memset((void*)msg, 0, sizeof(cwmpEvtMsg));
	return msg;
}

/* free the evt msg */
void cwmpEvtMsgFree(cwmpEvtMsg *msg){
	if(msg != NULL)
		free(msg);
}

/*set the msg event*/
void cwmpEvtMsgSetEvent(cwmpEvtMsg *msg, cwmpEvt event){
	//assert(msg != NULL);
	msg->event = event;
}

/*get the msg event*/
cwmpEvt cwmpEvtMsgGetEvent(cwmpEvtMsg *msg){
	assert(msg != NULL);
	return msg->event;
}

/* set the voiceProfileLineStatus */
void cwmpEvtMsgSetVPLineStatus(cwmpEvtMsg *msg, voiceProfileLineStatus *VPLineStatus, int nPort){
	assert(msg != NULL && nPort >= 0 && nPort < MAX_PORTS);
	memcpy((void*)&msg->voiceProfileLineStatusMsg[nPort], (void*)VPLineStatus, sizeof(voiceProfileLineStatus));
}

/*get the voiceProfileLineStatus */
voiceProfileLineStatus *cwmpEvtMsgGetVPLineStatus(cwmpEvtMsg *msg, int nPort){
	assert(msg != NULL && nPort >= 0 && nPort < MAX_PORTS);
	return &(msg->voiceProfileLineStatusMsg[nPort]);
}

/* the size of the evt msg */
int cwmpEvtMsgSizeof(void){
	return sizeof(cwmpEvtMsg);
}
