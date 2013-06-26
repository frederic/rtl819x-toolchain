#include "prmt_limit.h"
#include "prmt_voice_profile.h"
#include "prmt_voice_profile_line.h"
#include "mib_def.h"
#include "mib_tr104.h"
#include "str_utility.h"
#include "str_mib.h"

static int getVoiceProfileSipEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
static int setVoiceProfileSipEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

static int getServiceProviderInfoEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
static int setServiceProviderInfoEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

static int getVoiceProfileEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
static int setVoiceProfileEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

#if 0
struct sCWMP_ENTITY tVoiceProfileSipEntity[] = {
/*	{ name,								type,			flag,					accesslist,	getvalue,					setvalue,					next_table,	sibling } */
	{ "ProxyServer",					eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileSipEntity, 	setVoiceProfileSipEntity,	NULL,		NULL },
	{ "ProxyServerPort",				eCWMP_tUINT,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileSipEntity, 	setVoiceProfileSipEntity,	NULL,		NULL },
	{ "ProxyServerTransport",			eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileSipEntity, 	setVoiceProfileSipEntity,	NULL,		NULL },
	{ "RegistrationServer",				eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileSipEntity, 	setVoiceProfileSipEntity,	NULL,		NULL },
	{ "RegistrationServerPort",			eCWMP_tUINT,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileSipEntity, 	setVoiceProfileSipEntity,	NULL,		NULL },
	{ "RegistrationServerTransport",	eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileSipEntity, 	setVoiceProfileSipEntity,	NULL,		NULL },
	{ "UserAgentDomain",				eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileSipEntity, 	setVoiceProfileSipEntity,	NULL,		NULL },
	{ "UserAgentPort",					eCWMP_tUINT,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileSipEntity, 	setVoiceProfileSipEntity,	NULL,		NULL },
	{ "UserAgentTransport",				eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileSipEntity, 	setVoiceProfileSipEntity,	NULL,		NULL },
	{ "OutboundProxy",					eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileSipEntity, 	setVoiceProfileSipEntity,	NULL,		NULL },
	{ "OutboundProxyPort",				eCWMP_tUINT,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileSipEntity, 	setVoiceProfileSipEntity,	NULL,		NULL },
	{ "Organization",					eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileSipEntity, 	setVoiceProfileSipEntity,	NULL,		NULL },
	{ "RegistrationPeriod",				eCWMP_tUINT,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileSipEntity, 	setVoiceProfileSipEntity,	NULL,		NULL },
	{ "",								eCWMP_tNONE,	0,						NULL,		NULL,						NULL,						NULL,		NULL },
};
#endif

// leaf 
struct CWMP_OP tVoiceProfileSipEntityLeavesOP = {
	getVoiceProfileSipEntity, 	setVoiceProfileSipEntity, 
};

struct CWMP_PRMT tVoiceProfileSipEntityLeavesInfo[] = {
	/*(name,			type,		flag,			op)*/
	{ "ProxyServer",					eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&tVoiceProfileSipEntityLeavesOP },
	{ "ProxyServerPort",				eCWMP_tUINT,	CWMP_READ | CWMP_WRITE,	&tVoiceProfileSipEntityLeavesOP },
	{ "ProxyServerTransport",			eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&tVoiceProfileSipEntityLeavesOP },
	{ "RegistrationServer",				eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&tVoiceProfileSipEntityLeavesOP },
	{ "RegistrationServerPort",			eCWMP_tUINT,	CWMP_READ | CWMP_WRITE,	&tVoiceProfileSipEntityLeavesOP },
	{ "RegistrationServerTransport",	eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&tVoiceProfileSipEntityLeavesOP },
	{ "UserAgentDomain",				eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&tVoiceProfileSipEntityLeavesOP },
	{ "UserAgentPort",					eCWMP_tUINT,	CWMP_READ | CWMP_WRITE,	&tVoiceProfileSipEntityLeavesOP },
	{ "UserAgentTransport",				eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&tVoiceProfileSipEntityLeavesOP },
	{ "OutboundProxy",					eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&tVoiceProfileSipEntityLeavesOP },
	{ "OutboundProxyPort",				eCWMP_tUINT,	CWMP_READ | CWMP_WRITE,	&tVoiceProfileSipEntityLeavesOP },
	{ "Organization",					eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&tVoiceProfileSipEntityLeavesOP },
	{ "RegistrationPeriod",				eCWMP_tUINT,	CWMP_READ | CWMP_WRITE,	&tVoiceProfileSipEntityLeavesOP },
};

enum {
	eProxyServer,
	eProxyServerPort,
	eProxyServerTransport,
	eRegistrationServer,
	eRegistrationServerPort,
	eRegistrationServerTransport,
	eUserAgentDomain,
	eUserAgentPort,
	eUserAgentTransport,
	eOutboundProxy,
	eOutboundProxyPort,
	eOrganization,
	eRegistrationPeriod,
};

struct CWMP_LEAF tVoiceProfileSipEntityLeaves[] = {
	{ &tVoiceProfileSipEntityLeavesInfo[ eProxyServer ] },
	{ &tVoiceProfileSipEntityLeavesInfo[ eProxyServerPort ] },
	{ &tVoiceProfileSipEntityLeavesInfo[ eProxyServerTransport ] },
	{ &tVoiceProfileSipEntityLeavesInfo[ eRegistrationServer ] },
	{ &tVoiceProfileSipEntityLeavesInfo[ eRegistrationServerPort ] },
	{ &tVoiceProfileSipEntityLeavesInfo[ eRegistrationServerTransport ] },
	{ &tVoiceProfileSipEntityLeavesInfo[ eUserAgentDomain ] },
	{ &tVoiceProfileSipEntityLeavesInfo[ eUserAgentPort ] },
	{ &tVoiceProfileSipEntityLeavesInfo[ eUserAgentTransport ] },
	{ &tVoiceProfileSipEntityLeavesInfo[ eOutboundProxy ] },
	{ &tVoiceProfileSipEntityLeavesInfo[ eOutboundProxyPort ] },
	{ &tVoiceProfileSipEntityLeavesInfo[ eOrganization ] },
	{ &tVoiceProfileSipEntityLeavesInfo[ eRegistrationPeriod ] },
	{ NULL },
};

#if 0
struct sCWMP_ENTITY tServiceProviderInfoEntity[] = {
/*	{ name,					type,			flag,					accesslist,	getvalue,						setvalue,						next_table,	sibling } */
	{ "Name",				eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getServiceProviderInfoEntity, 	setServiceProviderInfoEntity,	NULL,		NULL },
	{ "",					eCWMP_tNONE,	0,						NULL,		NULL,							NULL,							NULL,		NULL },
};
#endif

// leaf 
struct CWMP_OP tServiceProviderInfoEntityLeavesOP = {
	getServiceProviderInfoEntity, setServiceProviderInfoEntity, 
};

struct CWMP_PRMT tServiceProviderInfoEntityLeavesInfo[] = {
	/*(name,			type,		flag,			op)*/
	{ "Name",			eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&tServiceProviderInfoEntityLeavesOP },
};

struct CWMP_LEAF tServiceProviderInfoEntityLeaves[] = {
	{ &tServiceProviderInfoEntityLeavesInfo[ 0 ] },
	{ NULL },
};

#if 0
struct sCWMP_ENTITY tVoiceProfileEntity[] = {
/*	{ name,					type,			flag,					accesslist,	getvalue,				setvalue,				next_table,					sibling } */
	{ "Enable",				eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileEntity, 	setVoiceProfileEntity,	NULL,						NULL },
	{ "Reset",				eCWMP_tBOOLEAN,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileEntity, 	setVoiceProfileEntity,	NULL,						NULL },
	{ "NumberOfLines",		eCWMP_tUINT,	CWMP_READ,				NULL,		getVoiceProfileEntity, 	setVoiceProfileEntity,	NULL,						NULL },
	{ "Name",				eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileEntity, 	setVoiceProfileEntity,	NULL,						NULL },
	{ "SignallingProtocol",	eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileEntity, 	setVoiceProfileEntity,	NULL,						NULL },
	{ "MaxSessions",		eCWMP_tUINT,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileEntity, 	setVoiceProfileEntity,	NULL,						NULL },
	{ "DTMFMethod",			eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileEntity, 	setVoiceProfileEntity,	NULL,						NULL },
	{ "ServiceProviderInfo",eCWMP_tOBJECT,	CWMP_READ,				NULL,		NULL, 					NULL,					tServiceProviderInfoEntity,	NULL },
	{ "SIP",				eCWMP_tOBJECT,	CWMP_READ,				NULL,		NULL, 					NULL,					tVoiceProfileSipEntity,		NULL },
	{ "Line",				eCWMP_tOBJECT,	CWMP_READ,				NULL,		NULL, 					objVoiceProfileLine,	NULL,						NULL },
	{ "",					eCWMP_tNONE,	0,						NULL,		NULL,					NULL,					NULL,		NULL },
};
#endif

// node  
struct CWMP_OP tVoiceProfileEntityNodesOP = {
	NULL, objVoiceProfileLine, 
};

struct CWMP_PRMT tVoiceProfileEntityNodesInfo[] = {
	/*(name,				type,		flag,			op)*/
	{ "ServiceProviderInfo",eCWMP_tOBJECT,	CWMP_READ,	NULL },
	{ "SIP",				eCWMP_tOBJECT,	CWMP_READ,	NULL },
	{ "Line",				eCWMP_tOBJECT,	CWMP_READ,	&tVoiceProfileEntityNodesOP },
};

enum {
	eServiceProviderInfo,
	eSIP,
	eLine,
};

struct CWMP_NODE tVoiceProfileEntityNodes[] = {
	/*info,  												leaf,			next)*/
	{ &tVoiceProfileEntityNodesInfo[ eServiceProviderInfo ], &tServiceProviderInfoEntityLeaves, NULL },
	{ &tVoiceProfileEntityNodesInfo[ eSIP ], &tVoiceProfileSipEntityLeaves, NULL },
	{ &tVoiceProfileEntityNodesInfo[ eLine ], NULL, NULL },
	{ NULL },
};


// leaf 
struct CWMP_OP tVoiceProfileEntityLeavesOP = {
	getVoiceProfileEntity, setVoiceProfileEntity, 
};

struct CWMP_PRMT tVoiceProfileEntityLeavesInfo[] = {
	/*(name,			type,		flag,			op)*/
	{ "Enable",				eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&tVoiceProfileEntityLeavesOP },
	{ "Reset",				eCWMP_tBOOLEAN,	CWMP_READ | CWMP_WRITE,	&tVoiceProfileEntityLeavesOP },
	{ "NumberOfLines",		eCWMP_tUINT,	CWMP_READ,				&tVoiceProfileEntityLeavesOP },
	{ "Name",				eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&tVoiceProfileEntityLeavesOP },
	{ "SignallingProtocol",	eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&tVoiceProfileEntityLeavesOP },
	{ "MaxSessions",		eCWMP_tUINT,	CWMP_READ | CWMP_WRITE,	&tVoiceProfileEntityLeavesOP },
	{ "DTMFMethod",			eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&tVoiceProfileEntityLeavesOP },
};

enum {
	eEnable,
	eReset,
	eNumberOfLines,
	eName,
	eSignallingProtocol,
	eMaxSessions,
	eDTMFMethod,
};

struct CWMP_LEAF tVoiceProfileEntityLeaves[] = {
	{ &tVoiceProfileEntityLeavesInfo[ eEnable ] },
	{ &tVoiceProfileEntityLeavesInfo[ eReset ] },
	{ &tVoiceProfileEntityLeavesInfo[ eNumberOfLines ] },
	{ &tVoiceProfileEntityLeavesInfo[ eName ] },
	{ &tVoiceProfileEntityLeavesInfo[ eSignallingProtocol ] },
	{ &tVoiceProfileEntityLeavesInfo[ eMaxSessions ] },
	{ &tVoiceProfileEntityLeavesInfo[ eDTMFMethod ] },
	{ NULL },
};

#if 0
struct sCWMP_ENTITY tVoiceProfile[] = {
/*	{ name,		type,			flag,								accesslist,	getvalue,	setvalue,	next_table,				sibling } */
	{ "0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL,		NULL,		NULL,		tVoiceProfileEntity, 	NULL },
};
#endif

// link node 
struct CWMP_PRMT tVoiceProfileLinkNodeInfo[] =
{
	/*(name,			type,		flag,		op)*/
	{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL},
};

struct CWMP_LINKNODE tVoiceProfileLinkNode[] =
{
	/*info,  						leaf, 					 	next,		sibling,		instnum)*/
	{&tVoiceProfileLinkNodeInfo[0],	tVoiceProfileEntityLeaves, tVoiceProfileEntityNodes,		NULL,			0},
};


static int getVoiceProfileSipEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	/* VoiceServer.{i}.VoiceProfile.{i}.SIP */
	unsigned int nVoiceProfileInstNum;
	union {
		char ProxyServer[ 256 ];
		unsigned int ProxyServerPort;
		transport_t ProxyServerTransport;
		char RegistrationServer[ 256 ];
		unsigned int RegistrationServerPort;
		transport_t RegistrationServerTransport;
		char UserAgentDomain[ 256 ];
		unsigned int UserAgentPort;
		transport_t UserAgentTransport;
		char OutboundProxy[ 256 ];
		unsigned int OutboundProxyPort;
		char Organization[ 256 ];
		unsigned int RegistrationPeriod;
	} s;
	const char *pszLastname = entity ->info ->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	*type = entity ->info ->type;	
	
	if( strcmp( pszLastname, "ProxyServer" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__PROXY_SERVER, nVoiceProfileInstNum, &s.ProxyServer );
		*data = strdup( s.ProxyServer );
	} else if( strcmp( pszLastname, "ProxyServerPort" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__PROXY_SERVER_PORT, nVoiceProfileInstNum, &s.ProxyServerPort );
		*data = uintdup( s.ProxyServerPort );
	} else if( strcmp( pszLastname, "ProxyServerTransport" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__PROXY_SERVER_TRANSPORT, nVoiceProfileInstNum, &s.ProxyServerTransport );
		*data = strdup_Transport( s.ProxyServerTransport );
	} else if( strcmp( pszLastname, "RegistrationServer" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__REGISTRATION_SERVER, nVoiceProfileInstNum, &s.RegistrationServer );
		*data = strdup( s.RegistrationServer );
	} else if( strcmp( pszLastname, "RegistrationServerPort" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__REGISTRATION_SERVER_PORT, nVoiceProfileInstNum, &s.RegistrationServerPort );
		*data = uintdup( s.RegistrationServerPort );
	} else if( strcmp( pszLastname, "RegistrationServerTransport" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__REGISTRATION_SERVER_TRANSPORT, nVoiceProfileInstNum, &s.RegistrationServerTransport );
		*data = strdup_Transport( s.RegistrationServerTransport );
	} else if( strcmp( pszLastname, "UserAgentDomain" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__USER_AGENT_DOMAIN, nVoiceProfileInstNum, &s.UserAgentDomain );
		*data = strdup( s.UserAgentDomain );
	} else if( strcmp( pszLastname, "UserAgentPort" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__USER_AGENT_PORT, nVoiceProfileInstNum, &s.UserAgentPort );
		*data = uintdup( s.UserAgentPort );
	} else if( strcmp( pszLastname, "UserAgentTransport" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__USER_AGENT_TRANSPORT, nVoiceProfileInstNum, &s.UserAgentTransport );
		*data = strdup_Transport( s.UserAgentTransport );
	} else if( strcmp( pszLastname, "OutboundProxy" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY, nVoiceProfileInstNum, &s.OutboundProxy );
		*data = strdup( s.OutboundProxy );
	} else if( strcmp( pszLastname, "OutboundProxyPort" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY_PORT, nVoiceProfileInstNum, &s.OutboundProxyPort );
		*data = uintdup( s.OutboundProxyPort );
	} else if( strcmp( pszLastname, "Organization" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__ORGANIZATION, nVoiceProfileInstNum, &s.Organization );
		*data = strdup( s.Organization );
	} else if( strcmp( pszLastname, "RegistrationPeriod" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIP__REGISTRATION_PERIOD, nVoiceProfileInstNum, &s.RegistrationPeriod );
		*data = uintdup( s.RegistrationPeriod );
	} else {
		*data = NULL;
		return ERR_9005;
	}
	
	return 0;
}

static int setVoiceProfileSipEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	/* VoiceServer.{i}.VoiceProfile.{i}.SIP */
	unsigned int nVoiceProfileInstNum;
	union {
		char ProxyServer[ 256 ];
		unsigned int ProxyServerPort;
		transport_t ProxyServerTransport;
		char RegistrationServer[ 256 ];
		unsigned int RegistrationServerPort;
		transport_t RegistrationServerTransport;
		char UserAgentDomain[ 256 ];
		unsigned int UserAgentPort;
		transport_t UserAgentTransport;
		char OutboundProxy[ 256 ];
		unsigned int OutboundProxyPort;
		char Organization[ 256 ];
		unsigned int RegistrationPeriod;
	} s;
	const char *pszLastname = entity ->info ->name;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
		
	if( entity ->info ->type != type )
		return ERR_9006;
	
	if( strcmp( pszLastname, "ProxyServer" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SIP__PROXY_SERVER, nVoiceProfileInstNum, data );
		return 1;
	} else if( strcmp( pszLastname, "ProxyServerPort" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SIP__PROXY_SERVER_PORT, nVoiceProfileInstNum, data );
		return 1;
	} else if( strcmp( pszLastname, "ProxyServerTransport" ) == 0 ) {
		if( str2id_SignalingProtocol( data, &s.ProxyServerTransport ) ){
			mib_set_type1( MIB_VOICE_PROFILE__SIP__PROXY_SERVER_TRANSPORT, nVoiceProfileInstNum, &s.ProxyServerTransport );
			return 1;
		}else{
			return ERR_9007;
		}
	} else if( strcmp( pszLastname, "RegistrationServer" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SIP__REGISTRATION_SERVER, nVoiceProfileInstNum, data );
		return 1;
	} else if( strcmp( pszLastname, "RegistrationServerPort" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SIP__REGISTRATION_SERVER_PORT, nVoiceProfileInstNum, data );
		return 1;
	} else if( strcmp( pszLastname, "RegistrationServerTransport" ) == 0 ) {
		if( str2id_SignalingProtocol( data, &s.RegistrationServerTransport ) ){
			mib_set_type1( MIB_VOICE_PROFILE__SIP__REGISTRATION_SERVER_TRANSPORT, nVoiceProfileInstNum, &s.RegistrationServerTransport );
			return 1;
		}else{
			return ERR_9007;
		}
	} else if( strcmp( pszLastname, "UserAgentDomain" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SIP__USER_AGENT_DOMAIN, nVoiceProfileInstNum, data );
		return 1;
	} else if( strcmp( pszLastname, "UserAgentPort" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SIP__USER_AGENT_PORT, nVoiceProfileInstNum, data );
		return 1;
	} else if( strcmp( pszLastname, "UserAgentTransport" ) == 0 ) {
		if( str2id_SignalingProtocol( data, &s.UserAgentTransport ) ){
			mib_set_type1( MIB_VOICE_PROFILE__SIP__USER_AGENT_TRANSPORT, nVoiceProfileInstNum, &s.UserAgentTransport );
			return 1;
		}else{
			return ERR_9007;
		}
	} else if( strcmp( pszLastname, "OutboundProxy" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY, nVoiceProfileInstNum, data );
		return 1;
	} else if( strcmp( pszLastname, "OutboundProxyPort" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SIP__OUTBOUND_PROXY_PORT, nVoiceProfileInstNum, data );
		return 1;
	} else if( strcmp( pszLastname, "Organization" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SIP__ORGANIZATION, nVoiceProfileInstNum, data );
		return 1;
	} else if( strcmp( pszLastname, "RegistrationPeriod" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SIP__REGISTRATION_PERIOD, nVoiceProfileInstNum, data );
		return 1;
	} else {
		return ERR_9005;
	}	
	
	return 0;
}

static int getServiceProviderInfoEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	/* VoiceServer.{i}.VoiceProfile.{i}.ServiceProviderInfo */
	unsigned int nVoiceProfileInstNum;
	char szName[ 256 ];
	const char *pszLastname = entity ->info ->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	*type = entity ->info ->type;	
		
	if( strcmp( pszLastname, "Name" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SERVICE_PROVIDE_INFO__NAME, nVoiceProfileInstNum, &szName );
		*data = strdup( szName );
	} else {
		*data = NULL;
		return ERR_9005;
	}
	
	return 0;
}

static int setServiceProviderInfoEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	/* VoiceServer.{i}.VoiceProfile.{i}.ServiceProviderInfo */
	unsigned int nVoiceProfileInstNum;
	const char *pszLastname = entity ->info ->name;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
		
	if( entity ->info ->type != type )
		return ERR_9006;
	
	if( strcmp( pszLastname, "Name" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__SERVICE_PROVIDE_INFO__NAME, nVoiceProfileInstNum, data );
	} else {
		return ERR_9005;
	}	
	
	return 0;
}

static int getVoiceProfileEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	/* VoiceServer.{i}.VoiceProfile.{i}. */
	unsigned int nVoiceProfileInstNum;
	union {
		enable_t Enable;
		int Reset;
		unsigned int NumberOfLines;
		char Name[ 40 ];
		signaling_protocol_t SignallingProtocol;
		unsigned int MaxSessions;
		DTMF_method_t DTMFMethod;
	} s;

	const char *pszLastname = entity ->info ->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;

	*type = entity ->info ->type;

	if( strcmp( pszLastname, "Enable" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__ENABLE, nVoiceProfileInstNum, &s.Enable );
		*data = strdup_Enable( s.Enable );
	} else if( strcmp( pszLastname, "Reset" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__RESET, nVoiceProfileInstNum, &s.Reset );
		*data = booldup( s.Reset );
	} else if( strcmp( pszLastname, "NumberOfLines" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &s.NumberOfLines );
		*data = uintdup( s.NumberOfLines );
	} else if( strcmp( pszLastname, "Name" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__NAME, nVoiceProfileInstNum, &s.Name );
		*data = strdup( s.Name );
	} else if( strcmp( pszLastname, "SignallingProtocol" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__SIGNALING_PROTOCOL, nVoiceProfileInstNum, &s.SignallingProtocol );
		*data = strdup_SignalingProtocol( s.SignallingProtocol );
	} else if( strcmp( pszLastname, "MaxSessions" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__MAX_SESSIONS, nVoiceProfileInstNum, &s.MaxSessions );
		*data = uintdup( s.MaxSessions );
	} else if( strcmp( pszLastname, "DTMFMethod" ) == 0 ) {
		mib_get_type1( MIB_VOICE_PROFILE__DTMF_METHOD, nVoiceProfileInstNum, &s.DTMFMethod );
		*data = strdup_DTMFMethod( s.DTMFMethod );	
	} else {
		*data = NULL;
		return ERR_9005;
	}

	return 0;
}

static int setVoiceProfileEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	/* VoiceServer.{i}.VoiceProfile.{i}. */
	unsigned int nVoiceProfileInstNum;
	union {
		enable_t Enable;
		int Reset;
		unsigned int NumberOfLines;
		char Name[ 40 ];
		signaling_protocol_t SignallingProtocol;
		unsigned int MaxSessions;
		DTMF_method_t DTMFMethod;
	} s;

	const char *pszLastname = entity ->info ->name;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile( name, &nVoiceProfileInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
		
	if( entity ->info ->type != type )
		return ERR_9006;

	if( strcmp( pszLastname, "Enable" ) == 0 ) {
		if( str2id_Enable( data, &s.Enable ) ){
			mib_set_type1( MIB_VOICE_PROFILE__ENABLE, nVoiceProfileInstNum, &s.Enable );
			return 1;
		}else{
			return ERR_9007;
		}
	} else if( strcmp( pszLastname, "Reset" ) == 0 ) {
		s.Reset = *( ( int * )data );
		mib_set_type1( MIB_VOICE_PROFILE__RESET, nVoiceProfileInstNum, &s.Reset );
		return 1;
	} else if( strcmp( pszLastname, "NumberOfLines" ) == 0 ) {
		return ERR_9008;	/* Read only */
	} else if( strcmp( pszLastname, "Name" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__NAME, nVoiceProfileInstNum, data );
		return 1;
	} else if( strcmp( pszLastname, "SignallingProtocol" ) == 0 ) {
		if( str2id_SignalingProtocol( data, &s.SignallingProtocol ) ){
			mib_set_type1( MIB_VOICE_PROFILE__SIGNALING_PROTOCOL, nVoiceProfileInstNum, &s.SignallingProtocol );
			return 1;
		}else{
			return ERR_9007;
		}
	} else if( strcmp( pszLastname, "MaxSessions" ) == 0 ) {
		mib_set_type1( MIB_VOICE_PROFILE__MAX_SESSIONS, nVoiceProfileInstNum, data );
		return 1;
	} else if( strcmp( pszLastname, "DTMFMethod" ) == 0 ) {
		if( str2id_DTMFMethod( data, &s.DTMFMethod ) ){
			mib_set_type1( MIB_VOICE_PROFILE__DTMF_METHOD, nVoiceProfileInstNum, &s.DTMFMethod );
			return 1;
		}else{
			return ERR_9007;
		}
	} else {
		return ERR_9005;
	}
	
	return 1;
}

int objVoiceProfile(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	switch( type ) {
	case eCWMP_tINITOBJ:
	{
		int num=0,i;
		struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;
		//MIB_CE_IP_ROUTE_T *p,route_entity;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		num = 1;//mib_chain_total( MIB_IP_ROUTE_TBL );
		for( i=0; i<num;i++ )
		{
			//p = &route_entity;
			//if( !mib_chain_get( MIB_IP_ROUTE_TBL, i, (void*)p ) )
			//	continue;
			
			//if( p->InstanceNum==0 ) //maybe createn by web or cli
			//{
			//	MaxInstNum++;
			//	p->InstanceNum = MaxInstNum;
			//	mib_chain_update( MIB_IP_ROUTE_TBL, (unsigned char*)p, i );
			//}else
			//	MaxInstNum = p->InstanceNum;
			
			if( create_Object( c, tVoiceProfileLinkNode, sizeof(tVoiceProfileLinkNode), MAX_PROFILE_COUNT, 1 ) < 0 )
				return -1;
			//c = & (*c)->sibling;
		}
		add_objectNum( name, 1 );
		return 0;
	}
#if 0
	// TODO: Not implement add/del/update yet.  
	case eCWMP_tADDOBJ:
	     {
	     	int ret;
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
		ret = add_Object( name, &entity->next_table,  tForwarding, sizeof(tForwarding), data );
		if( ret >= 0 )
		{
			MIB_CE_IP_ROUTE_T fentry;
			memset( &fentry, 0, sizeof( MIB_CE_IP_ROUTE_T ) );
			fentry.InstanceNum = *(unsigned int*)data;
			fentry.FWMetric = -1;
			fentry.ifIndex = 0xff;
			mib_chain_add( MIB_IP_ROUTE_TBL, (unsigned char*)&fentry );
		}
		return ret;
	     }
	case eCWMP_tDELOBJ:
	     {
	     	int ret, id;
	     	
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
	     	
	     	id = getChainID( entity->next_table, *(int*)data  );
	     	if(id==-1) return ERR_9005;
	     	mib_chain_delete( MIB_IP_ROUTE_TBL, id );	
		ret = del_Object( name, &entity->next_table, *(int*)data );
		if( ret == 0 ) return 1;
		return ret;
	     }
	case eCWMP_tUPDATEOBJ:	
	     {     	int num=0,i;
	     	struct sCWMP_ENTITY *old_table;
	     	num = mib_chain_total( MIB_IP_ROUTE_TBL );
	     	old_table = entity->next_table;
	     	entity->next_table = NULL;
	     	for( i=0; i<num;i++ )
	     	{
	     		struct sCWMP_ENTITY *remove_entity=NULL;
			MIB_CE_IP_ROUTE_T *p,route_entity;

			p = &route_entity;
			if( !mib_chain_get( MIB_IP_ROUTE_TBL, i, (void*)p ) )
				continue;			
			remove_entity = remove_SiblingEntity( &old_table, p->InstanceNum );
			if( remove_entity!=NULL )
			{
				add_SiblingEntity( &entity->next_table, remove_entity );
			}else{ 
				unsigned int MaxInstNum=p->InstanceNum;			
				add_Object( name, &entity->next_table,  tForwarding,     							sizeof(tForwarding), &MaxInstNum );
				if(MaxInstNum!=p->InstanceNum)
				{     p->InstanceNum = MaxInstNum;
				      mib_chain_update( MIB_IP_ROUTE_TBL, (unsigned char*)p, i );
				}
			}	
	     	}
	     	if( old_table ) 	destroy_ParameterTable( old_table );	     	
	     	return 0;
	     }
#endif
	}
	return -1;
}

