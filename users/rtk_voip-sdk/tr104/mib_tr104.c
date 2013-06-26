#include <string.h>
#include <stdio.h>
#include "mib_tr104.h"
#include "voip_flash.h"
#include "prmt_capabilities.h"
#include "prmt_limit.h"
#include "prmt_igd.h"

#ifdef CONFIG_RTK_VOIP_PACKAGE_8186
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#else
#include "mibtbl.h"
#endif

/* voip share memory */
voip_flash_share_t *g_pVoIPShare = NULL;

/* get the sip uri. If domain is NULL, device LAN ip is replaced*/
static void getSIPURI(char *sipURI, char *username, char *domain){
	boolean buserName=TRUE;
	char addr[DNS_LEN];
	
	if(NULL == username || 0 == strlen(username)){
		buserName=FALSE;
	}
	if(NULL == domain || 0 == strlen(domain)){
#ifdef CONFIG_RTK_VOIP_PACKAGE_8186
		mib_client_get(MIB_IP_ADDR, addr, sizeof(addr));
		domain=inet_ntoa(*((struct in_addr *)addr));
#else
		getMIB2Str(MIB_GW_LAN_IP, addr);		
		domain=addr;
#endif
	}
	
	if(TRUE == buserName)
		sprintf(sipURI,"%s@%s",username,domain);
	else
		sprintf(sipURI,"%s",domain);
}

/* get the flash codec index */
static int getCodecIndex(const char *codecName, int bitRate){
	
	if( NULL == codecName)
		return -1;
	
	if(0 == strcmp("G.711MuLaw",codecName)){
		return _CODEC_G711U;
	}else if(0 == strcmp("G.711ALaw",codecName)){
		return _CODEC_G711A;
	}else if(0 == strcmp("G.729",codecName)){
		return _CODEC_G729;
	}else if(0 == strcmp("G.723.1",codecName) ){
		return _CODEC_G723;
	}else if(0 == strcmp("G.726",codecName) && 16000 == bitRate){
		return _CODEC_G726_16;
	}else if(0 == strcmp("G.726",codecName) && 24000 == bitRate){
		return _CODEC_G726_24;
	}else if(0 == strcmp("G.726",codecName) && 32000 == bitRate){
		return _CODEC_G726_32;
	}else if(0 == strcmp("G.726",codecName) && 40000 == bitRate){
		return _CODEC_G726_40;
	}else if(0 == strcmp("GSM-FR",codecName)){
		return _CODEC_GSMFR;
	}else if(0 == strcmp("iLBC",codecName)){
		return _CODEC_ILBC;
	}else if(0 == strcmp("G.722",codecName)){
		return _CODEC_G722;
	}
	
	return -1;
}

/* verify the setting values */
static int verifySettingValues(idMib_t idMib, int nPort, voipCfgParam_t *voipConfig,void *pData){
	voipCfgProxy_t *proxy = &voipConfig->ports[nPort].proxies[voipConfig->ports[nPort].default_proxy];

	switch(idMib){
	case MIB_VOICE_PROFILE__SIP__PROXY_SERVER:
	case MIB_VOICE_PROFILE__SIP__REGISTRATION_SERVER: /*pass through*/
		if(proxy->enable && (0 == strlen((char *)pData) || NULL == pData)){
			CWMPDBG( 0, ( stderr, "ERROR:Proxy server name should not be empty\n") );
			return VERIFY_ERROR;
		}
		break;
		
	case MIB_VOICE_PROFILE__SIP__PROXY_SERVER_PORT:
	case MIB_VOICE_PROFILE__SIP__REGISTRATION_SERVER_PORT: /*pass through*/
	case MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY_PORT: /*pass through*/
		if(*( ( unsigned int * )pData ) < 0 || *( ( unsigned int * )pData ) > 65535){
			CWMPDBG( 0, ( stderr, "ERROR:Port should be in the range 0~65535\n") );
			return VERIFY_ERROR;
		}
		break;
		
	case MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY:
		if(proxy->outbound_enable && (0 == strlen((char *)pData) || NULL == pData)){
			CWMPDBG( 0, ( stderr, "ERROR:Outbound proxy name should not be empty\n") );
			return VERIFY_ERROR;
		}
		break;

	case MIB_VOICE_PROFILE__SIP__REGISTRATION_PERIOD:
		if(*( ( unsigned int * )pData ) < 10 || *( ( unsigned int * )pData )  > 86400){
			CWMPDBG( 0, ( stderr, "ERROR:Registration period should be in range 10~86400\n") );
			return VERIFY_ERROR;
		}
		break;

#ifdef CONFIG_RTK_VOIP_SLIC_NUM_2
	case MIB_VOICE_PROFILE__SIP__USER_AGENT_PORT:
	{
		int otherPort=0;
		otherPort = (nPort == 0) ? 1 : 0;
		if(*( ( unsigned int * )pData ) == voipConfig->ports[otherPort].sip_port){
			CWMPDBG( 0, ( stderr, "ERROR:Sip port of this line should not be as same as that of other line\n") );
			return VERIFY_ERROR;
		}
	}
		break;
#endif /*CONFIG_RTK_VOIP_SLIC_NUM_2*/

	case MIB_VOICE_PROFILE__SIP__USER_AGENT_DOMAIN:
	{
		char addr[DNS_LEN];
		char *ptrTempAddr=NULL;
#ifdef CONFIG_RTK_VOIP_PACKAGE_8186
		mib_client_get(MIB_IP_ADDR, addr, sizeof(addr));
		ptrTempAddr = inet_ntoa(*((struct in_addr *)addr));
		if(0 == strcmp(ptrTempAddr,(char*)pData)){
#else
		getMIB2Str(MIB_GW_LAN_IP, addr);
		if(0 == strcmp(addr,(char*)pData)){
#endif
			CWMPDBG( 0, ( stderr, "ERROR:Domain should not be the LAN IP\n") );
			return VERIFY_ERROR;
		}
	}	
		break;
	default:
		return VERIFY_ERROR;

	}
	return VERIFY_OK;
}

int mib_get_type1( idMib_t idMib, unsigned int nVoiceProfileInstNum, 
				   void *pData )
{	
	/* VoiceProfile.{ i }. */
	//voipCfgParam_t voipConfig;
	voipCfgParam_t * const pVoipConfig = &g_pVoIPShare ->voip_cfg;
	voipCfgProxy_t *proxy;
	int nPort = 0;
	
	if( g_pVoIPShare == NULL ) {
		printf( "g_pVoIPShare is NULL in mib_get_type1\n" );
		return 0;
	}

	if( nVoiceProfileInstNum >= MAX_PROFILE_COUNT )
		return 0;
	nPort = nVoiceProfileInstNum;
	printf("+++++debug:mib_get_type1+++++\n");
//#ifdef CONFIG_RTK_VOIP_PACKAGE_8186
//	mib_client_get(MIB_VOIP_CFG, (void*)&voipConfig, sizeof(voipConfig));
//#else /*CONFIG_RTK_VOIP_PACKAGE_8186*/
//	mib_client_get(CWMP_VOIP, (void*)&voipConfig);
//#endif
 	
#if 1
	if( nVoiceProfileInstNum >= VOIP_PORTS )
		printf( "nVoiceProfileInstNum=%d >= VOIP_PORTS\n", nVoiceProfileInstNum );
#endif
 	
 	proxy = &pVoipConfig ->ports[nPort].proxies[pVoipConfig ->ports[nPort].default_proxy];

	switch( idMib ) {
	case MIB_VOICE_PROFILE__ENABLE:
		*( ( enable_t * )pData ) = TR104_ENABLE;
		break;
		
	case MIB_VOICE_PROFILE__RESET:
		*( ( boolean * )pData ) = FALSE;
		break;
		
	case MIB_VOICE_PROFILE__NUMBER_OF_LINES:
		*( ( unsigned int * )pData ) = MAX_LINE_PER_PROFILE;
		break;
		
	case MIB_VOICE_PROFILE__NAME:
	{
		char profileName[PROFILE_NAME_LEN];
		sprintf(profileName,"%s%d","profile",nVoiceProfileInstNum + 1);
		strcpy( ( char * )pData, profileName);
	}
		break;
		
	case MIB_VOICE_PROFILE__SIGNALING_PROTOCOL:
		*( ( signaling_protocol_t * )pData ) = SIP;
		break;
		
	case MIB_VOICE_PROFILE__MAX_SESSIONS:
		*( ( unsigned int * )pData ) = PROFILE_MAX_SESSION;
		break;
	
	case MIB_VOICE_PROFILE__DTMF_METHOD:
		*( ( DTMF_method_t * )pData ) = pVoipConfig ->ports[nPort].dtmf_mode;
		break;
		
	case MIB_VOICE_PROFILE__SERVICE_PROVIDE_INFO__NAME:
		strcpy( ( char * )pData, SERVICE_PROVIDER );
		break;

	case MIB_VOICE_PROFILE__SIP__PROXY_SERVER:
		strcpy( ( char * )pData, proxy->addr);
		break;
		
	case MIB_VOICE_PROFILE__SIP__PROXY_SERVER_PORT:
		*( ( unsigned int * )pData ) = proxy->port;
		break;
		
	case MIB_VOICE_PROFILE__SIP__PROXY_SERVER_TRANSPORT:
		*( ( transport_t * )pData ) = UDP;
		break;
		
	case MIB_VOICE_PROFILE__SIP__REGISTRATION_SERVER:
		strcpy( ( char * )pData, proxy->addr);
		break;
		
	case MIB_VOICE_PROFILE__SIP__REGISTRATION_SERVER_PORT:
		*( ( unsigned int * )pData ) = proxy->port;
		break;
		
	case MIB_VOICE_PROFILE__SIP__REGISTRATION_SERVER_TRANSPORT:
		*( ( transport_t * )pData ) = UDP;
		break;
		
	case MIB_VOICE_PROFILE__SIP__USER_AGENT_DOMAIN:
		if(NULL == proxy->domain_name || 0 == strlen(proxy->domain_name)){
			char addr[DNS_LEN];
#ifdef CONFIG_RTK_VOIP_PACKAGE_8186
			printf( "sizeof(addr):%d\n", sizeof(addr) );
			mib_client_get(MIB_IP_ADDR, addr, sizeof(addr));
			strcpy( ( char * )pData, inet_ntoa(*((struct in_addr *)addr)));
#else
			getMIB2Str(MIB_GW_LAN_IP, addr);
			strcpy( ( char * )pData, addr);
#endif
		}else{
			strcpy( ( char * )pData, proxy->domain_name);
		}
		break;

	case MIB_VOICE_PROFILE__SIP__USER_AGENT_PORT:
		*( ( unsigned int * )pData ) = pVoipConfig ->ports[nPort].sip_port;
		break;

	case MIB_VOICE_PROFILE__SIP__USER_AGENT_TRANSPORT:
		*( ( transport_t * )pData ) = UDP;
		break;

	case MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY:
		strcpy( ( char * )pData, proxy->outbound_addr);
		break;

	case MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY_PORT:
		*( ( unsigned int * )pData ) = proxy->outbound_port;
		break;

	case MIB_VOICE_PROFILE__SIP__ORGANIZATION:
		strcpy( ( char * )pData, ORGANIZATION_HEADER );
		break;

	case MIB_VOICE_PROFILE__SIP__REGISTRATION_PERIOD:
		*( ( unsigned int * )pData ) = proxy->reg_expire;
		break;
		
	default:
		return 0;	/* range error */
		break;
	}

	return 1;
}

int mib_set_type1( idMib_t idMib, unsigned int nVoiceProfileInstNum, 
				   void *pData )
{	
	/* VoiceProfile.{ i }. */
	//voipCfgParam_t voipConfig;
	voipCfgParam_t * const pVoipConfig = &g_pVoIPShare ->voip_cfg;
	voipCfgProxy_t *proxy;
	int nPort=0;
	boolean bDirtyBit = FALSE;
	
	printf("+++++debug:mib_set_type1+++++\n");
	if( nVoiceProfileInstNum >= MAX_PROFILE_COUNT )
		return 0;
	nPort = nVoiceProfileInstNum;
//#ifdef CONFIG_RTK_VOIP_PACKAGE_8186
//	mib_client_get(MIB_VOIP_CFG, (void*)&voipConfig, sizeof(voipConfig));
//#else /*CONFIG_RTK_VOIP_PACKAGE_8186*/
//	mib_client_get(CWMP_VOIP, (void*)&voipConfig);
//#endif
 	proxy = &pVoipConfig ->ports[nPort].proxies[pVoipConfig ->ports[nPort].default_proxy];
	
	switch( idMib ) {	
	case MIB_VOICE_PROFILE__ENABLE:
		// unable to set
		break;
		
	case MIB_VOICE_PROFILE__RESET:
		// unable to set
		break;
		
	case MIB_VOICE_PROFILE__NUMBER_OF_LINES:
		// unable to set
		break;
		
	case MIB_VOICE_PROFILE__NAME:
		// unable to set
		break;

	case MIB_VOICE_PROFILE__SIGNALING_PROTOCOL:
		// unable to set
		break;
		
	case MIB_VOICE_PROFILE__MAX_SESSIONS:
		// unable to set
		break;
		
	case MIB_VOICE_PROFILE__DTMF_METHOD:
		pVoipConfig ->ports[nPort].dtmf_mode = *( ( DTMF_method_t * )pData );
		bDirtyBit = TRUE;
		break;

	case MIB_VOICE_PROFILE__SERVICE_PROVIDE_INFO__NAME:
		// unable to set
		break;

	case MIB_VOICE_PROFILE__SIP__PROXY_SERVER:
		if(VERIFY_OK == verifySettingValues(idMib, nPort, pVoipConfig, pData)){
			strcpy( proxy->addr, ( char * )pData );
			bDirtyBit = TRUE;
		}
		break;

	case MIB_VOICE_PROFILE__SIP__PROXY_SERVER_PORT:
		if(VERIFY_OK == verifySettingValues(idMib, nPort, pVoipConfig, pData)){
			proxy->port = *( ( unsigned int * )pData );
			bDirtyBit = TRUE;
		}
		break;

	case MIB_VOICE_PROFILE__SIP__PROXY_SERVER_TRANSPORT:
		// unable to set
		break;

	case MIB_VOICE_PROFILE__SIP__REGISTRATION_SERVER:
		if(VERIFY_OK == verifySettingValues(idMib, nPort, pVoipConfig, pData)){
			strcpy( proxy->addr, ( char * )pData );
			bDirtyBit = TRUE;
		}
		break;

	case MIB_VOICE_PROFILE__SIP__REGISTRATION_SERVER_PORT:
		if(VERIFY_OK == verifySettingValues(idMib, nPort, pVoipConfig, pData)){
			proxy->port = *( ( unsigned int * )pData );
			bDirtyBit = TRUE;
		}
		break;

	case MIB_VOICE_PROFILE__SIP__REGISTRATION_SERVER_TRANSPORT:
		// unable to set
		break;
		
	case MIB_VOICE_PROFILE__SIP__USER_AGENT_DOMAIN:
		if(VERIFY_OK == verifySettingValues(idMib, nPort, pVoipConfig, pData)){
			strcpy( proxy->domain_name, ( char * )pData );
			bDirtyBit = TRUE;
		}
		break;

	case MIB_VOICE_PROFILE__SIP__USER_AGENT_PORT:
		if(VERIFY_OK == verifySettingValues(idMib, nPort, pVoipConfig, pData)){
			pVoipConfig ->ports[nPort].sip_port = *( ( unsigned int * )pData );
			bDirtyBit = TRUE;
		}
		break;

	case MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY:
		if(VERIFY_OK == verifySettingValues(idMib, nPort, pVoipConfig, pData)){
			strcpy( proxy->outbound_addr, ( char * )pData );
			bDirtyBit = TRUE;
		}
		break;

	case MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY_PORT:
		if(VERIFY_OK == verifySettingValues(idMib, nPort, pVoipConfig, pData)){
			proxy->outbound_port = *( ( unsigned int * )pData );
			bDirtyBit = TRUE;
		}
		break;

	case MIB_VOICE_PROFILE__SIP__ORGANIZATION:
		//unable to set
		break;
		
	case MIB_VOICE_PROFILE__SIP__REGISTRATION_PERIOD:
		if(VERIFY_OK == verifySettingValues(idMib, nPort, pVoipConfig, pData)){
			proxy->reg_expire = *( ( unsigned int * )pData );
			bDirtyBit = TRUE;
		}
		break;
		
	default:
		return 0;	/* range error */
		break;
	}
	if(TRUE == bDirtyBit)
#ifdef CONFIG_RTK_VOIP_PACKAGE_8186
		mib_client_set(MIB_VOIP_CFG, (void*)pVoipConfig, sizeof(voipCfgParam_t));
#else
		mib_client_set(CWMP_VOIP, (void*)pVoipConfig);
#endif
	return 1;
}

int mib_get_type2( idMib_t idMib, unsigned int nVoiceProfileInstNum, 
				   unsigned int nLineInstNum,
				   void *pData )
{
	/* VoiceProfile.{ i }.Line.{i}. */
	//voipCfgParam_t voipConfig;
	voipCfgParam_t * const pVoipConfig = &g_pVoIPShare ->voip_cfg;
	voipCfgProxy_t *proxy;
	int nPort = 0;
	char sipURI[100];

	printf("+++++debug:mib_get_type2+++++\n");
	if( ( nVoiceProfileInstNum >= MAX_PROFILE_COUNT ) ||
		( nLineInstNum >= MAX_LINE_PER_PROFILE ) )
	{
		return 0;
	}
	
	nPort = nVoiceProfileInstNum;
//#ifdef CONFIG_RTK_VOIP_PACKAGE_8186
//	mib_client_get(MIB_VOIP_CFG, (void*)&voipConfig, sizeof(voipConfig));
//#else /*CONFIG_RTK_VOIP_PACKAGE_8186*/
//	mib_client_get(CWMP_VOIP, (void*)&voipConfig);
//#endif
 	proxy = &pVoipConfig ->ports[nPort].proxies[pVoipConfig ->ports[nPort].default_proxy];

	getSIPURI(sipURI, proxy->login_id, proxy->domain_name);
	
	switch( idMib ) {
	case MIB_VOICE_PROFILE__LINE__ENABLE:
		*( ( enable_t * )pData ) = TR104_ENABLE;
		break;

	case MIB_VOICE_PROFILE__LINE__DIRECTOR_NUMBER:
		strcpy( pData, proxy->number);
		break;

	case MIB_VOICE_PROFILE__LINE__STATUS:
		*( ( line_status_t * )pData ) = UP;
		break;

	case MIB_VOICE_PROFILE__LINE__SIP__AUTH_USER_NAME:
		strcpy( (char *)pData, proxy->login_id);
		break;

	case MIB_VOICE_PROFILE__LINE__SIP__AUTH_PASSWORD:
		strcpy( (char *)pData, proxy->password);
		break;

	case MIB_VOICE_PROFILE__LINE__SIP__URI:
		strcpy( (char *)pData, sipURI );
		break;

	case MIB_VOICE_PROFILE__LINE__SIP__STATS__PACKETS_SENT:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__PACKETS_RECEIVED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__BYTES_SENT:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__BYTES_RECEIVED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__PACKETS_LOST:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__INCOMING_CALLS_RECEIVED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__INCOMING_CALLS_ANSWERED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__INCOMING_CALLS_CONNECTED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__INCOMING_CALLS_FAILED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__OUTGOING_CALLS_ATTEMPTED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__OUTGOING_CALLS_ANSWERED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__OUTGOING_CALLS_CONNECTED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__OUTGOING_CALLS_FAILED:
		break;
		
	default:
		return 0;	/* range error */
		break;
	}

	return 1;
}

int mib_set_type2( idMib_t idMib, unsigned int nVoiceProfileInstNum, 
				   unsigned int nLineInstNum,
				   void *pData )
{
	/* VoiceProfile.{ i }.Line.{i}. */
	//voipCfgParam_t voipConfig;
	voipCfgParam_t * const pVoipConfig = &g_pVoIPShare ->voip_cfg;
	voipCfgProxy_t *proxy;
	int nPort=0;
	boolean bDirtyBit=FALSE;
	
	printf("+++++debug:mib_set_type2+++++\n");
	if( ( nVoiceProfileInstNum >= MAX_PROFILE_COUNT ) ||
		( nLineInstNum >= MAX_LINE_PER_PROFILE ) )
	{
		return 0;
	}
	nPort = nVoiceProfileInstNum;
//#ifdef CONFIG_RTK_VOIP_PACKAGE_8186
//	mib_client_get(MIB_VOIP_CFG, (void*)&voipConfig, sizeof(voipConfig));
//#else /*CONFIG_RTK_VOIP_PACKAGE_8186*/
//	mib_client_get(CWMP_VOIP, (void*)&voipConfig);
//#endif
 	proxy = &pVoipConfig ->ports[nPort].proxies[pVoipConfig ->ports[nPort].default_proxy];
	
	switch( idMib ) {
		case MIB_VOICE_PROFILE__LINE__ENABLE:
		// unable to set	
		break;

	case MIB_VOICE_PROFILE__LINE__DIRECTOR_NUMBER:
		strcpy( proxy->number, pData );
		bDirtyBit=TRUE;
		break;

	case MIB_VOICE_PROFILE__LINE__STATUS:
		// unable to set
		break;

	case MIB_VOICE_PROFILE__LINE__SIP__AUTH_USER_NAME:
		strcpy( proxy->login_id, pData );
		bDirtyBit=TRUE;
		break;

	case MIB_VOICE_PROFILE__LINE__SIP__AUTH_PASSWORD:
		strcpy( proxy->password, pData );
		bDirtyBit=TRUE;
		break;

	case MIB_VOICE_PROFILE__LINE__SIP__URI:
		// unable to set
		break;
		
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__PACKETS_SENT:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__PACKETS_RECEIVED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__BYTES_SENT:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__BYTES_RECEIVED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__PACKETS_LOST:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__INCOMING_CALLS_RECEIVED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__INCOMING_CALLS_ANSWERED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__INCOMING_CALLS_CONNECTED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__INCOMING_CALLS_FAILED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__OUTGOING_CALLS_ATTEMPTED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__OUTGOING_CALLS_ANSWERED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__OUTGOING_CALLS_CONNECTED:
	case MIB_VOICE_PROFILE__LINE__SIP__STATS__OUTGOING_CALLS_FAILED:
		break;
		
	default:
		return 0;	/* range error */
		break;
	}
	if(TRUE == bDirtyBit)
#ifdef CONFIG_RTK_VOIP_PACKAGE_8186
		mib_client_set(MIB_VOIP_CFG, (void*)pVoipConfig, sizeof(voipCfgParam_t));
#else
		mib_client_set(CWMP_VOIP, (void*)pVoipConfig);
#endif
	return 1;
}

int mib_get_type3( idMib_t idMib, unsigned int nVoiceProfileInstNum, 
				   unsigned int nLineInstNum, unsigned int nListInstNum,
				   void *pData )
{
	/* VoiceProfile.{ i }.Line.{i}.Codec.List.{i}. */
	//voipCfgParam_t voipConfig;
	voipCfgParam_t * const pVoipConfig = &g_pVoIPShare ->voip_cfg;
	int nPort=0;
	
	printf("+++++debug:mib_get_type3+++++\n");
	if( ( nVoiceProfileInstNum >= MAX_PROFILE_COUNT ) ||
		( nLineInstNum >= MAX_LINE_PER_PROFILE ) ||
		( nListInstNum >= MAX_CODEC_LIST ) )
	{
		return 0;
	}
	nPort = nVoiceProfileInstNum;
//#ifdef CONFIG_RTK_VOIP_PACKAGE_8186
//	mib_client_get(MIB_VOIP_CFG, (void*)&voipConfig, sizeof(voipConfig));
//#else /*CONFIG_RTK_VOIP_PACKAGE_8186*/
//	mib_client_get(CWMP_VOIP, (void*)&voipConfig);
//#endif
	
	switch( idMib ) {
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__ENTRY_ID:
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__CODEC:
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__BITRATE:
		break;
		
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__PACKETIZATION_PERIOD:
		strcpy( pData, lstCodecs[nListInstNum].pszPacketizationPeriod );
		break;
		
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__SILENCE_SUPPRESSION:
		*( ( boolean * )pData ) = lstCodecs[nListInstNum].bSilenceSupression;
		break;
		
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__PRIORITY:
	{
		int codecIndex=0;
		codecIndex = getCodecIndex(lstCodecs[nListInstNum].pszCodec, lstCodecs[nListInstNum].nBitRate);
		if(-1 != codecIndex)
			*( ( unsigned int * )pData ) = pVoipConfig ->ports[nPort].precedence[codecIndex];
	}
		break;
		
	default:
		return 0;	/* range error */
		break;
	}
	return 1;
}

int mib_set_type3( idMib_t idMib, unsigned int nVoiceProfileInstNum, 
				   unsigned int nLineInstNum, unsigned int nListInstNum,
				   void *pData )
{
	/* VoiceProfile.{ i }.Line.{i}.Codec.List.{i}. */
	//voipCfgParam_t voipConfig;
	voipCfgParam_t * const pVoipConfig = &g_pVoIPShare ->voip_cfg;
	int nPort=0;
	boolean bDirtyBit=FALSE;
	
	printf("+++++debug:mib_set_type3+++++\n");
	nPort = nVoiceProfileInstNum;
//#ifdef CONFIG_RTK_VOIP_PACKAGE_8186
//	mib_client_get(MIB_VOIP_CFG, (void*)&voipConfig, sizeof(voipConfig));
//#else /*CONFIG_RTK_VOIP_PACKAGE_8186*/
//	mib_client_get(CWMP_VOIP, (void*)&voipConfig);
//#endif
	switch( idMib ) {
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__ENTRY_ID:
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__CODEC:
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__BITRATE:
		break;
		
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__PACKETIZATION_PERIOD:
		//unable to set
		break;
		
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__SILENCE_SUPPRESSION:
		//unable to set
		break;
		
	case MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__PRIORITY:
	{
		int codecIndex=0, swapIndex=0, i;
	
		codecIndex = getCodecIndex(lstCodecs[nListInstNum].pszCodec, lstCodecs[nListInstNum].nBitRate);
		if(-1 != codecIndex){
			for(i=_CODEC_G711U; i <  _CODEC_MAX; i++){
				if(pVoipConfig ->ports[nPort].precedence[i]==*(( unsigned int * )pData)){
					swapIndex = i;
					break;
				}
			}
			pVoipConfig ->ports[nPort].precedence[swapIndex] = pVoipConfig ->ports[nPort].precedence[codecIndex];
			pVoipConfig ->ports[nPort].precedence[codecIndex]=*( ( unsigned int * )pData );
			bDirtyBit=TRUE;
		}
	}
		break;
		
	default:
		return 0;	/* range error */
		break;
	}
	if(TRUE == bDirtyBit)
#ifdef CONFIG_RTK_VOIP_PACKAGE_8186
		mib_client_set(MIB_VOIP_CFG, (void*)pVoipConfig, sizeof(voipCfgParam_t));
#else
		mib_client_set(CWMP_VOIP, (void*)pVoipConfig);
#endif
	
	return 1;
}


