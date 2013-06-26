#ifndef __MIB_SIM_H__
#define __MIB_SIM_H__
#include "mib_def.h"
#include "prmt_limit.h"

#define VOIP_SUPPORT 1
#include "apmib.h"
#include "voip_flash_client.h"

extern voip_flash_share_t *g_pVoIPShare;

#define PROFILE_NAME_LEN		20
/* the default value of voice profile*/
typedef int boolean;

typedef enum {
	/* -------------------- Argument type 1 ------------------------ */
	/* Need instance number of PROFILE */
	MIB_VOICE_PROFILE__ARG_TYPE1_START,
	MIB_VOICE_PROFILE__ENABLE,
	MIB_VOICE_PROFILE__RESET,
	MIB_VOICE_PROFILE__NUMBER_OF_LINES,
	MIB_VOICE_PROFILE__NAME,
	MIB_VOICE_PROFILE__SIGNALING_PROTOCOL,
	MIB_VOICE_PROFILE__MAX_SESSIONS,
	MIB_VOICE_PROFILE__DTMF_METHOD,
	MIB_VOICE_PROFILE__SERVICE_PROVIDE_INFO__NAME,
	MIB_VOICE_PROFILE__SIP__PROXY_SERVER,
	MIB_VOICE_PROFILE__SIP__PROXY_SERVER_PORT,
	MIB_VOICE_PROFILE__SIP__PROXY_SERVER_TRANSPORT,
	MIB_VOICE_PROFILE__SIP__REGISTRATION_SERVER,
	MIB_VOICE_PROFILE__SIP__REGISTRATION_SERVER_PORT,
	MIB_VOICE_PROFILE__SIP__REGISTRATION_SERVER_TRANSPORT,
	MIB_VOICE_PROFILE__SIP__USER_AGENT_DOMAIN,
	MIB_VOICE_PROFILE__SIP__USER_AGENT_PORT,
	MIB_VOICE_PROFILE__SIP__USER_AGENT_TRANSPORT,
	MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY,
	MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY_PORT,
	MIB_VOICE_PROFILE__SIP__ORGANIZATION,
	MIB_VOICE_PROFILE__SIP__REGISTRATION_PERIOD,
	/* -------------------- Arguments type 2 ------------------------ */
	/* Need instance number of PROFILE and LINE */
	MIB_VOICE_PROFILE__ARG_TYPE2_START,	
	MIB_VOICE_PROFILE__LINE__ENABLE,
	MIB_VOICE_PROFILE__LINE__DIRECTOR_NUMBER,
	MIB_VOICE_PROFILE__LINE__STATUS,
	MIB_VOICE_PROFILE__LINE__SIP__AUTH_USER_NAME,
	MIB_VOICE_PROFILE__LINE__SIP__AUTH_PASSWORD,
	MIB_VOICE_PROFILE__LINE__SIP__URI,
	MIB_VOICE_PROFILE__LINE__SIP__STATS__PACKETS_SENT,		/* unused */
	MIB_VOICE_PROFILE__LINE__SIP__STATS__PACKETS_RECEIVED,	/* unused */
	MIB_VOICE_PROFILE__LINE__SIP__STATS__BYTES_SENT,		/* unused */
	MIB_VOICE_PROFILE__LINE__SIP__STATS__BYTES_RECEIVED,	/* unused */
	MIB_VOICE_PROFILE__LINE__SIP__STATS__PACKETS_LOST,		/* unused */
	MIB_VOICE_PROFILE__LINE__SIP__STATS__INCOMING_CALLS_RECEIVED,	/* unused */
	MIB_VOICE_PROFILE__LINE__SIP__STATS__INCOMING_CALLS_ANSWERED,	/* unused */
	MIB_VOICE_PROFILE__LINE__SIP__STATS__INCOMING_CALLS_CONNECTED,	/* unused */
	MIB_VOICE_PROFILE__LINE__SIP__STATS__INCOMING_CALLS_FAILED,		/* unused */
	MIB_VOICE_PROFILE__LINE__SIP__STATS__OUTGOING_CALLS_ATTEMPTED,	/* unused */
	MIB_VOICE_PROFILE__LINE__SIP__STATS__OUTGOING_CALLS_ANSWERED,	/* unused */
	MIB_VOICE_PROFILE__LINE__SIP__STATS__OUTGOING_CALLS_CONNECTED,	/* unused */
	MIB_VOICE_PROFILE__LINE__SIP__STATS__OUTGOING_CALLS_FAILED,		/* unused */
	/* -------------------- Arguments type 3 ------------------------ */
	/* Need instance number of PROFILE, LINE and LIST */
	MIB_VOICE_PROFILE__ARG_TYPE3_START,	
	MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__ENTRY_ID,	/* unused */
	MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__CODEC,		/* unused */
	MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__BITRATE,		/* unused */
	MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__PACKETIZATION_PERIOD,
	MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__SILENCE_SUPPRESSION,
	MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__PRIORITY,
	/* -------------------- ---------------- ------------------------ */	
	MIB_VOICE_PROFILE__EOF,		/* keep it to be the last one */
} idMib_t;

extern int mib_get_type1( idMib_t idMib, unsigned int nVoiceProfileInstNum, 
				   void *pData );
extern int mib_set_type1( idMib_t idMib, unsigned int nVoiceProfileInstNum, 
				   void *pData );

extern int mib_get_type2( idMib_t idMib, unsigned int nVoiceProfileInstNum, 
				   unsigned int nLineInstNum,
				   void *pData );
extern int mib_set_type2( idMib_t idMib, unsigned int nVoiceProfileInstNum, 
				   unsigned int nLineInstNum,
				   void *pData );

extern int mib_get_type3( idMib_t idMib, unsigned int nVoiceProfileInstNum, 
				   unsigned int nLineInstNum, unsigned int nListInstNum,
				   void *pData );
extern int mib_set_type3( idMib_t idMib, unsigned int nVoiceProfileInstNum, 
				   unsigned int nLineInstNum, unsigned int nListInstNum,
				   void *pData );

#endif /* __MIB_SIM_H__ */

