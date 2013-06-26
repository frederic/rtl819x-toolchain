#include "prmt_limit.h"
#include "prmt_capabilities.h"
#include "str_utility.h"

static int getCapabilitiesCodecsEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
static int objCapabilitiesCodecs(char *name, struct CWMP_LEAF *entity, int type, void *data);

static int getCapabilitiesSipEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);

static int getCapabilitiesEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);

#if 0
struct sCWMP_ENTITY tCapabilitiesCodecsEntity[] = {
/*	{ name,					type,			flag,		accesslist,	getvalue,						setvalue,						next_table,	sibling } */
	{ "EntryId",			eCWMP_tUINT,	CWMP_READ,	NULL,		getCapabilitiesCodecsEntity, 	NULL,							NULL,		NULL },
	{ "Codec",				eCWMP_tSTRING,	CWMP_READ,	NULL,		getCapabilitiesCodecsEntity, 	NULL,							NULL,		NULL },
	{ "BitRate",			eCWMP_tUINT,	CWMP_READ,	NULL,		getCapabilitiesCodecsEntity, 	NULL,							NULL,		NULL },
	{ "PacketizationPeriod",eCWMP_tSTRING,	CWMP_READ,	NULL,		getCapabilitiesCodecsEntity, 	NULL,							NULL,		NULL },
	{ "SilenceSuppression",	eCWMP_tBOOLEAN,	CWMP_READ,	NULL,		getCapabilitiesCodecsEntity, 	NULL,							NULL,		NULL },
	{ "",					eCWMP_tNONE,	0,			NULL,		NULL,							NULL,							NULL,		NULL },
};
#endif

// leaf 
struct CWMP_OP tCapabilitiesCodecsEntityLeavesOP = {
	getCapabilitiesCodecsEntity, NULL, 
};

struct CWMP_PRMT tCapabilitiesCodecsEntityLeavesInfo[] = {
	/*(name,			type,		flag,			op)*/
	{ "EntryId",			eCWMP_tUINT,	CWMP_READ,	&tCapabilitiesCodecsEntityLeavesOP },
	{ "Codec",				eCWMP_tSTRING,	CWMP_READ,	&tCapabilitiesCodecsEntityLeavesOP },
	{ "BitRate",			eCWMP_tUINT,	CWMP_READ,	&tCapabilitiesCodecsEntityLeavesOP },
	{ "PacketizationPeriod",eCWMP_tSTRING,	CWMP_READ,	&tCapabilitiesCodecsEntityLeavesOP },
	{ "SilenceSuppression",	eCWMP_tBOOLEAN,	CWMP_READ,	&tCapabilitiesCodecsEntityLeavesOP },
};

enum {
	eEntryId,
	eCodec,
	eBitRate,
	ePacketizationPeriod,
	eSilenceSuppression,
};

struct CWMP_LEAF tCapabilitiesCodecsEntityLeaves[] = {
	{ &tCapabilitiesCodecsEntityLeavesInfo[ eEntryId ] },
	{ &tCapabilitiesCodecsEntityLeavesInfo[ eCodec ] },
	{ &tCapabilitiesCodecsEntityLeavesInfo[ eBitRate ] },
	{ &tCapabilitiesCodecsEntityLeavesInfo[ ePacketizationPeriod ] },
	{ &tCapabilitiesCodecsEntityLeavesInfo[ eSilenceSuppression ] },
	{ NULL }, 
};

#if 0
struct sCWMP_ENTITY tCapabilitiesCodecs[] = {
/*	{ name,		type,			flag,								accesslist,	getvalue,	setvalue,	next_table,					sibling } */
	{ "0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL,		NULL,		NULL,		tCapabilitiesCodecsEntity,	NULL },
};
#endif

// link node 
struct CWMP_PRMT tCapabilitiesCodecsInfo[] =
{
	/*(name,			type,		flag,		op)*/
	{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL},
};

struct CWMP_LINKNODE tCapabilitiesCodecsLinkNode[] =
{
	/*info,  						leaf, 							 next,		sibling,		instnum)*/
	{&tCapabilitiesCodecsInfo[0],	tCapabilitiesCodecsEntityLeaves, NULL,		NULL,			0},
};

#if 0
struct sCWMP_ENTITY tCapabilitiesSipEntity[] = {
/*	{ name,					type,			flag,		accesslist,	getvalue,					setvalue,					next_table,	sibling } */
	{ "Role",				eCWMP_tSTRING,	CWMP_READ,	NULL,		getCapabilitiesSipEntity, 	NULL,						NULL,		NULL },
	{ "Extensions",			eCWMP_tSTRING,	CWMP_READ,	NULL,		getCapabilitiesSipEntity, 	NULL,						NULL,		NULL },
	{ "Transports",			eCWMP_tSTRING,	CWMP_READ,	NULL,		getCapabilitiesSipEntity, 	NULL,						NULL,		NULL },
	{ "URISchemes",			eCWMP_tSTRING,	CWMP_READ,	NULL,		getCapabilitiesSipEntity, 	NULL,						NULL,		NULL },
	{ "",					eCWMP_tNONE,	0,			NULL,		NULL,						NULL,						NULL,		NULL },
};
#endif

// leaf
struct CWMP_OP tCapabilitiesSipEntityLeavesOP = {
	getCapabilitiesSipEntity, NULL, 
};

struct CWMP_PRMT tCapabilitiesSipEntityLeavesInfo[] = {
	/*(name,			type,		flag,			op)*/
	{ "Role",			eCWMP_tSTRING,	CWMP_READ,	&tCapabilitiesSipEntityLeavesOP }, 
	{ "Extensions",		eCWMP_tSTRING,	CWMP_READ,	&tCapabilitiesSipEntityLeavesOP }, 
	{ "Transports",		eCWMP_tSTRING,	CWMP_READ,	&tCapabilitiesSipEntityLeavesOP }, 
	{ "URISchemes",		eCWMP_tSTRING,	CWMP_READ,	&tCapabilitiesSipEntityLeavesOP }, 
};

enum {
	eRole,
	eExtensions,
	eTransports,
	eURISchemes,
};

struct CWMP_LEAF tCapabilitiesSipEntityLeaves[] = {
	{ &tCapabilitiesSipEntityLeavesInfo[ eRole ] }, 
	{ &tCapabilitiesSipEntityLeavesInfo[ eExtensions ] }, 
	{ &tCapabilitiesSipEntityLeavesInfo[ eTransports ] }, 
	{ &tCapabilitiesSipEntityLeavesInfo[ eURISchemes ] }, 
	{ NULL }, 
};

#if 0
struct sCWMP_ENTITY tCapabilitiesEntity[] = {
/*	{ name,					type,			flag,		accesslist,	getvalue,				setvalue,				next_table,				sibling } */
	{ "MaxProfileCount",	eCWMP_tUINT,	CWMP_READ,	NULL,		getCapabilitiesEntity, 	NULL,					NULL,					NULL },
	{ "MaxSessionCount",	eCWMP_tUINT,	CWMP_READ,	NULL,		getCapabilitiesEntity, 	NULL,					NULL,					NULL },
	{ "SignallingProtocols",eCWMP_tSTRING,	CWMP_READ,	NULL,		getCapabilitiesEntity, 	NULL,					NULL,					NULL },
	{ "FaxT38",				eCWMP_tBOOLEAN,	CWMP_READ,	NULL,		getCapabilitiesEntity, 	NULL,					NULL,					NULL },
	{ "FaxPassThrough",		eCWMP_tBOOLEAN,	CWMP_READ,	NULL,		getCapabilitiesEntity, 	NULL,					NULL,					NULL },
	{ "ModemPassThrough",	eCWMP_tBOOLEAN,	CWMP_READ,	NULL,		getCapabilitiesEntity, 	NULL,					NULL,					NULL },
	{ "ToneGeneration",		eCWMP_tBOOLEAN,	CWMP_READ,	NULL,		getCapabilitiesEntity, 	NULL,					NULL,					NULL },
	{ "RingGeneration",		eCWMP_tBOOLEAN,	CWMP_READ,	NULL,		getCapabilitiesEntity, 	NULL,					NULL,					NULL },
	{ "VoicePortTests",		eCWMP_tBOOLEAN,	CWMP_READ,	NULL,		getCapabilitiesEntity, 	NULL,					NULL,					NULL },
	{ "SIP",				eCWMP_tOBJECT,	CWMP_READ,	NULL,		NULL, 					NULL,					tCapabilitiesSipEntity,	NULL },
	{ "Codecs",				eCWMP_tOBJECT,	CWMP_READ,	NULL,		NULL, 					objCapabilitiesCodecs,	NULL,					NULL },
	{ "",					eCWMP_tNONE,	0,			NULL,		NULL,					NULL,					NULL,					NULL },
};
#endif

// nodes 
struct CWMP_OP tCapabilitiesEntityNodesOP = {
	NULL, objCapabilitiesCodecs, 
};

struct CWMP_PRMT tCapabilitiesEntityNodesInfo[] = {
	/*(name,			type,		flag,			op)*/
	{ "SIP",			eCWMP_tOBJECT,	CWMP_READ,	NULL, },
	{ "Codecs",			eCWMP_tOBJECT,	CWMP_READ,	&tCapabilitiesEntityNodesOP },
};

enum {
	eSIP,
	eCodecs,
};

struct CWMP_NODE tCapabilitiesEntityNodes[] = {
	/*info,  								leaf,			next)*/
	{ &tCapabilitiesEntityNodesInfo[ eSIP ], tCapabilitiesSipEntityLeaves, NULL },
	{ &tCapabilitiesEntityNodesInfo[ eCodecs ], NULL, NULL },
	{ NULL },
};

// leaf 
struct CWMP_OP tCapabilitiesEntityLeavesOP = {
	getCapabilitiesEntity, NULL, 
};

struct CWMP_PRMT tCapabilitiesEntityLeavesInfo[] = {
	/*(name,			type,		flag,			op)*/
	{ "MaxProfileCount",	eCWMP_tUINT,	CWMP_READ,	&tCapabilitiesEntityLeavesOP },
	{ "MaxSessionCount",	eCWMP_tUINT,	CWMP_READ,	&tCapabilitiesEntityLeavesOP },
	{ "SignallingProtocols",eCWMP_tSTRING,	CWMP_READ,	&tCapabilitiesEntityLeavesOP },
	{ "FaxT38",				eCWMP_tBOOLEAN,	CWMP_READ,	&tCapabilitiesEntityLeavesOP },
	{ "FaxPassThrough",		eCWMP_tBOOLEAN,	CWMP_READ,	&tCapabilitiesEntityLeavesOP },
	{ "ModemPassThrough",	eCWMP_tBOOLEAN,	CWMP_READ,	&tCapabilitiesEntityLeavesOP },
	{ "ToneGeneration",		eCWMP_tBOOLEAN,	CWMP_READ,	&tCapabilitiesEntityLeavesOP },
	{ "RingGeneration",		eCWMP_tBOOLEAN,	CWMP_READ,	&tCapabilitiesEntityLeavesOP },
	{ "VoicePortTests",		eCWMP_tBOOLEAN,	CWMP_READ,	&tCapabilitiesEntityLeavesOP },
};

enum {
	eMaxProfileCount,
	eMaxSessionCount,
	eSignallingProtocols,
	eFaxT38,
	eFaxPassThrough,
	eModemPassThrough,
	eToneGeneration,
	eRingGeneration,
	eVoicePortTests,
};

struct CWMP_LEAF tCapabilitiesEntityLeaves[] = {
	{ &tCapabilitiesEntityLeavesInfo[ eMaxProfileCount ] },
	{ &tCapabilitiesEntityLeavesInfo[ eMaxSessionCount ] },
	{ &tCapabilitiesEntityLeavesInfo[ eSignallingProtocols ] },
	{ &tCapabilitiesEntityLeavesInfo[ eFaxT38 ] },
	{ &tCapabilitiesEntityLeavesInfo[ eFaxPassThrough ] },
	{ &tCapabilitiesEntityLeavesInfo[ eModemPassThrough ] },
	{ &tCapabilitiesEntityLeavesInfo[ eToneGeneration ] },
	{ &tCapabilitiesEntityLeavesInfo[ eRingGeneration ] },
	{ &tCapabilitiesEntityLeavesInfo[ eVoicePortTests ] },
	{ NULL },	
};

/* It must be listed as same as that of voip_flash.h */
const lstCodecs_t lstCodecs[] = {
	{ "G.711MuLaw",	80 * 100 * 8,		"10,20,30",	1 },
	{ "G.711ALaw",	80 * 100 * 8,		"10,20,30",	1 },
#ifdef  CONFIG_RTK_VOIP_G729AB
	{ "G.729",		10 * 100 * 8,		"10,20,30",	1 },
#endif /*CONFIG_RTK_VOIP_G729AB*/
#ifdef CONFIG_RTK_VOIP_G7231
	{ "G.723.1",	5300,				"30,60",	1 },
	{ "G.723.1",	6300,				"30,60",	1 },
#endif /*CONFIG_RTK_VOIP_G7231*/
#ifdef  CONFIG_RTK_VOIP_G726
	{ "G.726",		16 * 1000,			"10,20,30",	1 },
	{ "G.726",		24 * 1000,			"10,20,30",	1 },
	{ "G.726",		32 * 1000,			"10,20,30",	1 },
	{ "G.726",		40 * 1000,			"10,20,30",	1 },
#endif /*CONFIG_RTK_VOIP_G726*/
#ifdef CONFIG_RTK_VOIP_GSMFR
	{ "GSM-FR",		33 * 100 * 8 / 2,	"20,40",	0 },
#endif /*CONFIG_RTK_VOIP_GSMFR*/
#ifdef CONFIG_RTK_VOIP_ILBC
	{ "iLBC",	13330,		"30",		0 },
	{ "iLBC",	15200,		"20",		0 },
#endif /*CONFIG_RTK_VOIP_ILBC*/
#ifdef CONFIG_RTK_VOIP_G722
	{ "G.722",	64000,		"10,20,30",		1 },
#endif /*CONFIG_RTK_VOIP_G722*/
};

#define NUM_OF_LIST_CODEC		( sizeof( lstCodecs ) / sizeof( lstCodecs[ 0 ] ) )

CT_ASSERT( NUM_OF_LIST_CODEC == MAX_CODEC_LIST );

static int getCapabilitiesCodecsEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	const char *pszLastname = entity ->info ->name;
	unsigned int nObjectNum, nObjectIdx;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	if( !GetOneBasedInstanceNumber_Capabilities_Codecs( name, &nObjectNum ) )
		return ERR_9007;
	
	nObjectIdx = nObjectNum - 1;	/* to zero-based */
	
	if( nObjectIdx >= NUM_OF_LIST_CODEC )
		return ERR_9007;
			
	*type = entity ->info ->type;

	if( strcmp( pszLastname, "EntryId" ) == 0 ) {
		*data = uintdup( nObjectNum );
	} else if( strcmp( pszLastname, "Codec" ) == 0 ) {
		*data = strdup( lstCodecs[ nObjectIdx ].pszCodec );
	} else if( strcmp( pszLastname, "BitRate" ) == 0 ) {
		*data = uintdup( lstCodecs[ nObjectIdx ].nBitRate );
	} else if( strcmp( pszLastname, "PacketizationPeriod" ) == 0 ) {
		*data = strdup( lstCodecs[ nObjectIdx ].pszPacketizationPeriod );
	} else if( strcmp( pszLastname, "SilenceSuppression" ) == 0 ) {
		*data = booldup( lstCodecs[ nObjectIdx ].bSilenceSupression );
	} else {
		*data = NULL;
		return ERR_9005;
	}	

	return 0;
}

static int objCapabilitiesCodecs(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	switch( type ) {
	case eCWMP_tINITOBJ:
	{
		struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		if( create_Object( c, tCapabilitiesCodecsLinkNode, sizeof(tCapabilitiesCodecsLinkNode), NUM_OF_LIST_CODEC, 1 ) < 0 )
			return -1;
			
		add_objectNum( name, 1 );
		return 0;
	}
		break;
		
	case eCWMP_tADDOBJ:
	case eCWMP_tDELOBJ:
	case eCWMP_tUPDATEOBJ:
		break;
	}
	
	return -1;
}

static int getCapabilitiesSipEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	const char *pszLastname = entity ->info ->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity ->info ->type;

	if( strcmp( pszLastname, "Role" ) == 0 ) {
		*data = strdup( SIP_ROLE );
	} else if( strcmp( pszLastname, "Extensions" ) == 0 ) {
		*data = strdup( "" );
	} else if( strcmp( pszLastname, "Transports" ) == 0 ) {
		*data = strdup( SIP_TRANSPORTS );
	} else if( strcmp( pszLastname, "URISchemes" ) == 0 ) {
		*data = strdup(SIP_URI_SCHEMES);
	} else {
		*data = NULL;
		return ERR_9005;
	}

	return 0;
}

static int getCapabilitiesEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	const char *pszLastname = entity ->info ->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity ->info ->type;

	if( strcmp( pszLastname, "MaxProfileCount" ) == 0 ) {
		*data = uintdup( MAX_PROFILE_COUNT );
	} else if( strcmp( pszLastname, "MaxSessionCount" ) == 0 ) {
		*data = uintdup( MAX_SESSION_COUNT );
	} else if( strcmp( pszLastname, "SignallingProtocols" ) == 0 ) {
		*data = strdup( SIGNALING_PROTOCOLS );	/* "SIP" */
	} else if( strcmp( pszLastname, "FaxT38" ) == 0 ) {
#ifdef CONFIG_RTK_VOIP_T38
		*data = booldup(TRUE);
	} else if( strcmp( pszLastname, "FaxPassThrough" ) == 0 ) {
		*data = booldup(TRUE);
	} else if( strcmp( pszLastname, "ModemPassThrough" ) == 0 ) {
		*data = booldup(TRUE);
#else
		*data = booldup(FALSE);
	} else if( strcmp( pszLastname, "FaxPassThrough" ) == 0 ) {
		*data = booldup(FALSE);
	} else if( strcmp( pszLastname, "ModemPassThrough" ) == 0 ) {
		*data = booldup(FALSE);
#endif
	} else if( strcmp( pszLastname, "ToneGeneration" ) == 0 ) {
		*data = booldup( SUPPORT_TONE_GENERATION_OBJECT );	/* Off -> No this object */
	} else if( strcmp( pszLastname, "RingGeneration" ) == 0 ) {
		*data = booldup( SUPPORT_RING_GENERATION_OBJECT );	/* Off -> No this object */
	} else if( strcmp( pszLastname, "VoicePortTests" ) == 0 ) {
		*data = booldup( SUPPORT_VOICE_LINE_TESTS_OBJECT );	/* Off -> No this object */
	} else {
		*data = NULL;
		return ERR_9005;
	}

	return 0;
}

