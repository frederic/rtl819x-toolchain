#include "prmt_limit.h"
#include "prmt_voice_profile_line.h"
#include "prmt_capabilities.h"	/* for lstCodecs */
#include "mib_def.h"
#include "mib_tr104.h"
#include "str_mib.h"
#include "str_utility.h"
#include "voip_manager.h"

#include "cwmpevt.h"

static int getVoiceProfileLineCodecListEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
static int setVoiceProfileLineCodecListEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);
static int objVoiceProfileLineCodec(char *name, struct CWMP_LEAF *entity, int type, void *data);

static int getVoiceProfileLineStatusEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
static int setVoiceProfileLineStatusEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

static int getVoiceProfileLineSipEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
static int setVoiceProfileLineSipEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

static int getVoiceProfileLineEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
static int setVoiceProfileLineEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);


#if 0
struct sCWMP_ENTITY tVoiceProfileLineCodecListEntity[] = {
/*	{ name,					type,			flag,					accesslist,	getvalue,								setvalue,							next_table,	sibling } */
	{ "EntryId",			eCWMP_tUINT,	CWMP_READ,				NULL,		getVoiceProfileLineCodecListEntity, 	setVoiceProfileLineCodecListEntity,	NULL,		NULL },
	{ "Codec",				eCWMP_tSTRING,	CWMP_READ,				NULL,		getVoiceProfileLineCodecListEntity, 	setVoiceProfileLineCodecListEntity,	NULL,		NULL },
	{ "BitRate",			eCWMP_tUINT,	CWMP_READ,				NULL,		getVoiceProfileLineCodecListEntity, 	setVoiceProfileLineCodecListEntity,	NULL,		NULL },
	{ "PacketizationPeriod",eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileLineCodecListEntity, 	setVoiceProfileLineCodecListEntity,	NULL,		NULL },
	{ "SilenceSuppression",	eCWMP_tBOOLEAN,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileLineCodecListEntity, 	setVoiceProfileLineCodecListEntity,	NULL,		NULL },
	{ "Priority",			eCWMP_tUINT,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileLineCodecListEntity, 	setVoiceProfileLineCodecListEntity,	NULL,		NULL },
	{ "",					eCWMP_tNONE,	0,						NULL,		NULL,									NULL,								NULL,		NULL },
};
#endif

// leaf 
struct CWMP_OP tVoiceProfileLineCodecListEntityLeavessOP = {
	getVoiceProfileLineCodecListEntity, setVoiceProfileLineCodecListEntity, 
};

struct CWMP_PRMT tVoiceProfileLineCodecListEntityLeavesInfo[] = {
	/*(name,			type,		flag,			op)*/
	{ "EntryId",			eCWMP_tUINT,	CWMP_READ,	&tVoiceProfileLineCodecListEntityLeavessOP },
	{ "Codec",				eCWMP_tSTRING,	CWMP_READ,	&tVoiceProfileLineCodecListEntityLeavessOP },
	{ "BitRate",			eCWMP_tUINT,	CWMP_READ,	&tVoiceProfileLineCodecListEntityLeavessOP },
	{ "PacketizationPeriod",eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&tVoiceProfileLineCodecListEntityLeavessOP },
	{ "SilenceSuppression",	eCWMP_tBOOLEAN,	CWMP_READ | CWMP_WRITE,	&tVoiceProfileLineCodecListEntityLeavessOP },
	{ "Priority",			eCWMP_tUINT,	CWMP_READ | CWMP_WRITE,	&tVoiceProfileLineCodecListEntityLeavessOP },
};

enum {
	eEntryId,
	eCodec,
	eBitRate,
	ePacketizationPeriod,
	eSilenceSuppression,
	ePriority,
};

struct CWMP_LEAF tVoiceProfileLineCodecListEntityLeaves[] = {
	{ &tVoiceProfileLineCodecListEntityLeavesInfo[ eEntryId ] },
	{ &tVoiceProfileLineCodecListEntityLeavesInfo[ eCodec ] },
	{ &tVoiceProfileLineCodecListEntityLeavesInfo[ eBitRate ] },
	{ &tVoiceProfileLineCodecListEntityLeavesInfo[ ePacketizationPeriod ] },
	{ &tVoiceProfileLineCodecListEntityLeavesInfo[ eSilenceSuppression ] },
	{ &tVoiceProfileLineCodecListEntityLeavesInfo[ ePriority ] },
	{ NULL },
};

#if 0
struct sCWMP_ENTITY tVoiceProfileLineCodecList[] = {
/*	{ name,		type,			flag,								accesslist,	getvalue,	setvalue,	next_table,							sibling } */
	{ "0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL,		NULL,		NULL,		tVoiceProfileLineCodecListEntity, 	NULL },
};
#endif

// link node 
struct CWMP_PRMT tVoiceProfileLineCodecListLinkNodeInfo[] =
{
	/*(name,			type,		flag,		op)*/
	{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL},
};

struct CWMP_LINKNODE tVoiceProfileLineCodecListLinkNode[] =
{
	/*info,  										leaf, 					 	next,		sibling,		instnum)*/
	{&tVoiceProfileLineCodecListLinkNodeInfo[0],	tVoiceProfileLineCodecListEntityLeaves, NULL,		NULL,			0},
};

#if 0
struct sCWMP_ENTITY tVoiceProfileLineCodecEntity[] = {
/*	{ name,					type,			flag,		accesslist,	getvalue,					setvalue,					next_table,	sibling } */
	{ "List",				eCWMP_tOBJECT,	CWMP_READ,	NULL,		NULL, 						objVoiceProfileLineCodec,	NULL,		NULL },
	{ "",					eCWMP_tNONE,	0,			NULL,		NULL,						NULL,						NULL,		NULL },
};
#endif

// node 
struct CWMP_OP tVoiceProfileLineCodecEntityNodesOP = {
	NULL, objVoiceProfileLineCodec, 
};

struct CWMP_PRMT tVoiceProfileLineCodecEntityNodesInfo[] = {
	/*(name,				type,		flag,			op)*/
	{ "List",				eCWMP_tOBJECT,	CWMP_READ,	&tVoiceProfileLineCodecEntityNodesOP },
};

struct CWMP_NODE tVoiceProfileLineCodecEntityNodes[] = {
	/*info,  										leaf,			next)*/
	{ &tVoiceProfileLineCodecEntityNodesInfo[ 0 ], NULL, NULL },
	{ NULL },
};

#if 0
struct sCWMP_ENTITY tVoiceProfileLineStatusEntity[] = {
/*	{ name,						type,			flag,		accesslist,	getvalue,							setvalue,							next_table,	sibling } */
	{ "PacketsSent",			eCWMP_tUINT,	CWMP_READ,	NULL,		getVoiceProfileLineStatusEntity, 	setVoiceProfileLineStatusEntity,	NULL,		NULL },
	{ "PacketsReceived",		eCWMP_tUINT,	CWMP_READ,	NULL,		getVoiceProfileLineStatusEntity, 	setVoiceProfileLineStatusEntity,	NULL,		NULL },
	{ "BytesSent",				eCWMP_tUINT,	CWMP_READ,	NULL,		getVoiceProfileLineStatusEntity, 	setVoiceProfileLineStatusEntity,	NULL,		NULL },
	{ "BytesReceived",			eCWMP_tUINT,	CWMP_READ,	NULL,		getVoiceProfileLineStatusEntity, 	setVoiceProfileLineStatusEntity,	NULL,		NULL },
	{ "PacketsLost",			eCWMP_tUINT,	CWMP_READ,	NULL,		getVoiceProfileLineStatusEntity, 	setVoiceProfileLineStatusEntity,	NULL,		NULL },
	{ "IncomingCallsReceived",	eCWMP_tUINT,	CWMP_READ,	NULL,		getVoiceProfileLineStatusEntity, 	setVoiceProfileLineStatusEntity,	NULL,		NULL },
	{ "IncomingCallsAnswered",	eCWMP_tUINT,	CWMP_READ,	NULL,		getVoiceProfileLineStatusEntity, 	setVoiceProfileLineStatusEntity,	NULL,		NULL },
	{ "IncomingCallsConnected",	eCWMP_tUINT,	CWMP_READ,	NULL,		getVoiceProfileLineStatusEntity, 	setVoiceProfileLineStatusEntity,	NULL,		NULL },
	{ "IncomingCallsFailed",	eCWMP_tUINT,	CWMP_READ,	NULL,		getVoiceProfileLineStatusEntity, 	setVoiceProfileLineStatusEntity,	NULL,		NULL },
	{ "OutgoingCallsAttempted",	eCWMP_tUINT,	CWMP_READ,	NULL,		getVoiceProfileLineStatusEntity, 	setVoiceProfileLineStatusEntity,	NULL,		NULL },
	{ "OutgoingCallsAnswered",	eCWMP_tUINT,	CWMP_READ,	NULL,		getVoiceProfileLineStatusEntity, 	setVoiceProfileLineStatusEntity,	NULL,		NULL },
	{ "OutgoingCallsConnected",	eCWMP_tUINT,	CWMP_READ,	NULL,		getVoiceProfileLineStatusEntity, 	setVoiceProfileLineStatusEntity,	NULL,		NULL },
	{ "OutgoingCallsFailed",	eCWMP_tUINT,	CWMP_READ,	NULL,		getVoiceProfileLineStatusEntity, 	setVoiceProfileLineStatusEntity,	NULL,		NULL },
	{ "",						eCWMP_tNONE,	0,			NULL,		NULL,								NULL,								NULL,		NULL },
};
#endif

// leaf 
struct CWMP_OP tVoiceProfileLineStatusEntityLeavesOP = {
	getVoiceProfileLineStatusEntity, 	setVoiceProfileLineStatusEntity, 
};

struct CWMP_PRMT tVoiceProfileLineStatusEntityLeavesInfo[] = {
	/*(name,			type,		flag,			op)*/
	{ "PacketsSent",			eCWMP_tUINT,	CWMP_READ,	&tVoiceProfileLineStatusEntityLeavesOP },
	{ "PacketsReceived",		eCWMP_tUINT,	CWMP_READ,	&tVoiceProfileLineStatusEntityLeavesOP },
	{ "BytesSent",				eCWMP_tUINT,	CWMP_READ,	&tVoiceProfileLineStatusEntityLeavesOP },
	{ "BytesReceived",			eCWMP_tUINT,	CWMP_READ,	&tVoiceProfileLineStatusEntityLeavesOP },
	{ "PacketsLost",			eCWMP_tUINT,	CWMP_READ,	&tVoiceProfileLineStatusEntityLeavesOP },
	{ "IncomingCallsReceived",	eCWMP_tUINT,	CWMP_READ,	&tVoiceProfileLineStatusEntityLeavesOP },
	{ "IncomingCallsAnswered",	eCWMP_tUINT,	CWMP_READ,	&tVoiceProfileLineStatusEntityLeavesOP },
	{ "IncomingCallsConnected",	eCWMP_tUINT,	CWMP_READ,	&tVoiceProfileLineStatusEntityLeavesOP },
	{ "IncomingCallsFailed",	eCWMP_tUINT,	CWMP_READ,	&tVoiceProfileLineStatusEntityLeavesOP },
	{ "OutgoingCallsAttempted",	eCWMP_tUINT,	CWMP_READ,	&tVoiceProfileLineStatusEntityLeavesOP },
	{ "OutgoingCallsAnswered",	eCWMP_tUINT,	CWMP_READ,	&tVoiceProfileLineStatusEntityLeavesOP },
	{ "OutgoingCallsConnected",	eCWMP_tUINT,	CWMP_READ,	&tVoiceProfileLineStatusEntityLeavesOP },
	{ "OutgoingCallsFailed",	eCWMP_tUINT,	CWMP_READ,	&tVoiceProfileLineStatusEntityLeavesOP },
};

enum {
	ePacketsSent,
	ePacketsReceived,
	eBytesSent,
	eBytesReceived,
	ePacketsLost,
	eIncomingCallsReceived,
	eIncomingCallsAnswered,
	eIncomingCallsConnected,
	eIncomingCallsFailed,
	eOutgoingCallsAttempted,
	eOutgoingCallsAnswered,
	eOutgoingCallsConnected,
	eOutgoingCallsFailed,
};

struct CWMP_LEAF tVoiceProfileLineStatusEntityLeaves[] = {
	{ &tVoiceProfileLineStatusEntityLeavesInfo[ ePacketsSent ] },
	{ &tVoiceProfileLineStatusEntityLeavesInfo[ ePacketsReceived ] },
	{ &tVoiceProfileLineStatusEntityLeavesInfo[ eBytesSent ] },
	{ &tVoiceProfileLineStatusEntityLeavesInfo[ eBytesReceived ] },
	{ &tVoiceProfileLineStatusEntityLeavesInfo[ ePacketsLost ] },
	{ &tVoiceProfileLineStatusEntityLeavesInfo[ eIncomingCallsReceived ] },
	{ &tVoiceProfileLineStatusEntityLeavesInfo[ eIncomingCallsAnswered ] },
	{ &tVoiceProfileLineStatusEntityLeavesInfo[ eIncomingCallsConnected ] },
	{ &tVoiceProfileLineStatusEntityLeavesInfo[ eIncomingCallsFailed ] },
	{ &tVoiceProfileLineStatusEntityLeavesInfo[ eOutgoingCallsAttempted ] },
	{ &tVoiceProfileLineStatusEntityLeavesInfo[ eOutgoingCallsAnswered ] },
	{ &tVoiceProfileLineStatusEntityLeavesInfo[ eOutgoingCallsConnected ] },
	{ &tVoiceProfileLineStatusEntityLeavesInfo[ eOutgoingCallsFailed ] },
	{ NULL },
};

#if 0
struct sCWMP_ENTITY tVoiceProfileLineSipEntity[] = {
/*	{ name,					type,			flag,					accesslist,	getvalue,						setvalue,						next_table,	sibling } */
	{ "AuthUserName",		eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileLineSipEntity, 	setVoiceProfileLineSipEntity,	NULL,		NULL },
	{ "AuthPassword",		eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileLineSipEntity, 	setVoiceProfileLineSipEntity,	NULL,		NULL },
	{ "URI",				eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileLineSipEntity, 	setVoiceProfileLineSipEntity,	NULL,		NULL },
	{ "",					eCWMP_tNONE,	0,						NULL,		NULL,							NULL,							NULL,		NULL },
};
#endif

// leaf 
struct CWMP_OP tVoiceProfileLineSipEntityLeavesOP = {
	getVoiceProfileLineSipEntity, 	setVoiceProfileLineSipEntity, 
};

struct CWMP_PRMT tVoiceProfileLineSipEntityLeavesInfo[] = {
	/*(name,			type,		flag,			op)*/
	{ "AuthUserName",		eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&tVoiceProfileLineSipEntityLeavesOP },
	{ "AuthPassword",		eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&tVoiceProfileLineSipEntityLeavesOP },
	{ "URI",				eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&tVoiceProfileLineSipEntityLeavesOP },
};

enum {
	eAuthUserName,
	eAuthPassword,
	eURI,
};

struct CWMP_LEAF tVoiceProfileLineSipEntityLeaves[] = {
	{ &tVoiceProfileLineSipEntityLeavesInfo[ eAuthUserName ] },
	{ &tVoiceProfileLineSipEntityLeavesInfo[ eAuthPassword ] },
	{ &tVoiceProfileLineSipEntityLeavesInfo[ eURI ] },
	{ NULL },
};

#if 0
struct sCWMP_ENTITY tVoiceProfileLineEntity[] = {
/*	{ name,					type,			flag,					accesslist,	getvalue,					setvalue,					next_table,						sibling } */
	{ "Enable",				eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileLineEntity, 	setVoiceProfileLineEntity,	NULL,							NULL },
	{ "DirectoryNumber",	eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	NULL,		getVoiceProfileLineEntity, 	setVoiceProfileLineEntity,	NULL,							NULL },
	{ "Status",				eCWMP_tSTRING,	CWMP_READ,				NULL,		getVoiceProfileLineEntity, 	setVoiceProfileLineEntity,	NULL,							NULL },
	{ "SIP",				eCWMP_tOBJECT,	CWMP_READ,				NULL,		NULL, 						NULL,						tVoiceProfileLineSipEntity,		NULL },
	{ "Stats",				eCWMP_tOBJECT,	CWMP_READ,				NULL,		NULL, 						NULL,						tVoiceProfileLineStatusEntity,	NULL },
	{ "Codec",				eCWMP_tOBJECT,	CWMP_READ,				NULL,		NULL, 						NULL,						tVoiceProfileLineCodecEntity,	NULL },
	{ "",					eCWMP_tNONE,	0,						NULL,		NULL,						NULL,						NULL,		NULL },
};
#endif

// node 
struct CWMP_PRMT tVoiceProfileLineEntityNodesInfo[] = {
	/*(name,				type,		flag,			op)*/
	{ "SIP",				eCWMP_tOBJECT,	CWMP_READ,	NULL },
	{ "Stats",				eCWMP_tOBJECT,	CWMP_READ,	NULL },
	{ "Codec",				eCWMP_tOBJECT,	CWMP_READ,	NULL },
};

enum {
	eLineSIP,
	eLineStats,
	eLineCodec,
};

struct CWMP_NODE tVoiceProfileLineEntityNodes[] = {
	/*info,  									 leaf,			next)*/
	{ &tVoiceProfileLineEntityNodesInfo[ eLineSIP ], tVoiceProfileLineSipEntityLeaves, NULL },
	{ &tVoiceProfileLineEntityNodesInfo[ eLineStats ], tVoiceProfileLineStatusEntityLeaves, NULL },
	{ &tVoiceProfileLineEntityNodesInfo[ eLineCodec ], NULL, tVoiceProfileLineCodecEntityNodes },
	{ NULL },
};

// leaf 
struct CWMP_OP tVoiceProfileLineEntityLeavesOP = {
	getVoiceProfileLineEntity, 	setVoiceProfileLineEntity, 
};

struct CWMP_PRMT tVoiceProfileLineEntityLeavesInfo[] = {
	/*(name,			type,		flag,			op)*/
	{ "Enable",				eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&tVoiceProfileLineEntityLeavesOP },
	{ "DirectoryNumber",	eCWMP_tSTRING,	CWMP_READ | CWMP_WRITE,	&tVoiceProfileLineEntityLeavesOP },
	{ "Status",				eCWMP_tSTRING,	CWMP_READ,				&tVoiceProfileLineEntityLeavesOP },
};

enum {
	eEnable,
	eDirectoryNumber,
	eStatus,
};

struct CWMP_LEAF tVoiceProfileLineEntityLeaves[] = {
	{ &tVoiceProfileLineEntityLeavesInfo[ eEnable ] },
	{ &tVoiceProfileLineEntityLeavesInfo[ eDirectoryNumber ] },
	{ &tVoiceProfileLineEntityLeavesInfo[ eStatus ] },
	{ NULL },
};

#if 0
struct sCWMP_ENTITY tVoiceProfileLine[] = {
/*	{ name,		type,			flag,								accesslist,	getvalue,	setvalue,	next_table,				sibling } */
	{ "0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL,		NULL,		NULL,		tVoiceProfileLineEntity, 	NULL },
};
#endif

// link node
struct CWMP_PRMT tVoiceProfileLineLinkNodeInfo[] =
{
	/*(name,			type,		flag,		op)*/
	{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL},
};

struct CWMP_LINKNODE tVoiceProfileLineLinkNode[] =
{
	/*info,  							leaf, 					 		next,							sibling,		instnum)*/
	{&tVoiceProfileLineLinkNodeInfo[0],	tVoiceProfileLineEntityLeaves, tVoiceProfileLineEntityNodes,	NULL,			0},
};

static int getVoiceProfileLineCodecListEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	/* VoiceServer.{i}.VoiceProfile.{i}.SIP.Line.{i}.Codec.List.{i}. */
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nCodecListInstNum;
	unsigned int nNumberOfLine;
	union {
		char PacketizationPeriod[ 64 ];
		int SilenceSuppression;
		unsigned int Priority;
	} s;

	const char *pszLastname = entity ->info ->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
	
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line_List( name, &nVoiceProfileInstNum, &nSipLineInstNum, &nCodecListInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;
	
	if( -- nCodecListInstNum >= MAX_CODEC_LIST )	/* convert to zero based */
		return -1;
	
	*type = entity ->info ->type;

	if( strcmp( pszLastname, "EntryId" ) == 0 ) {
		*data = uintdup( nCodecListInstNum + 1 );	/* 1 based */
	} else if( strcmp( pszLastname, "Codec" ) == 0 ) {
		*data = strdup( lstCodecs[ nCodecListInstNum ].pszCodec );
	} else if( strcmp( pszLastname, "BitRate" ) == 0 ) {
		*data = uintdup( lstCodecs[ nCodecListInstNum ].nBitRate );
	} else if( strcmp( pszLastname, "PacketizationPeriod" ) == 0 ) {
		mib_get_type3( MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__PACKETIZATION_PERIOD, 
					   nVoiceProfileInstNum, nSipLineInstNum, nCodecListInstNum,
					   &s.PacketizationPeriod );
		*data = strdup( s.PacketizationPeriod );
	} else if( strcmp( pszLastname, "SilenceSuppression" ) == 0 ) {
		mib_get_type3( MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__SILENCE_SUPPRESSION, 
					   nVoiceProfileInstNum, nSipLineInstNum, nCodecListInstNum,
					   &s.SilenceSuppression );
		*data = booldup( s.SilenceSuppression );
	} else if( strcmp( pszLastname, "Priority" ) == 0 ) {
		mib_get_type3( MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__PRIORITY, 
					   nVoiceProfileInstNum, nSipLineInstNum, nCodecListInstNum,
					   &s.Priority );
		*data = uintdup( s.Priority );
	} else {
		*data = NULL;
		return ERR_9005;
	}

	return 0;
}

static int setVoiceProfileLineCodecListEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	/* VoiceService.{i}.VoiceProfile.{i}.Line.{i}.Codec.List.{i}. */
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nCodecListInstNum;
	unsigned int nNumberOfLine;

	const char *pszLastname = entity ->info ->name;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
	
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line_List( name, &nVoiceProfileInstNum, &nSipLineInstNum, &nCodecListInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;
	
	if( -- nCodecListInstNum >= MAX_CODEC_LIST )	/* convert to zero based */
		return -1;

	if( entity ->info ->type != type )
		return ERR_9006;

	if( strcmp( pszLastname, "EntryId" ) == 0 ) {
		return ERR_9008;
	} else if( strcmp( pszLastname, "Codec" ) == 0 ) {
		return ERR_9008;
	} else if( strcmp( pszLastname, "BitRate" ) == 0 ) {
		return ERR_9008;
	} else if( strcmp( pszLastname, "PacketizationPeriod" ) == 0 ) {
		mib_set_type3( MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__PACKETIZATION_PERIOD, 
					   nVoiceProfileInstNum, nSipLineInstNum, nCodecListInstNum,
					   data );
		return 1;
	} else if( strcmp( pszLastname, "SilenceSuppression" ) == 0 ) {
		mib_set_type3( MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__SILENCE_SUPPRESSION, 
					   nVoiceProfileInstNum, nSipLineInstNum, nCodecListInstNum,
					   data );
		return 1;
	} else if( strcmp( pszLastname, "Priority" ) == 0 ) {
		mib_set_type3( MIB_VOICE_PROFILE__LINE__SIP__CODEC__LIST__PRIORITY, 
					   nVoiceProfileInstNum, nSipLineInstNum, nCodecListInstNum,
					   data );
		return 1;
	} else {
		return ERR_9005;
	}

	return 0;
}

static int objVoiceProfileLineCodec(char *name, struct CWMP_LEAF *entity, int type, void *data)
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
			
			if( create_Object( c, tVoiceProfileLineCodecListLinkNode, sizeof(tVoiceProfileLineCodecListLinkNode), MAX_CODEC_LIST, 1 ) < 0 )
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

static int getVoiceProfileLineStatusEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	/* VoiceServer.{i}.VoiceProfile.{i}.SIP.Line.{i}.Stats. */
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nNumberOfLine;
	unsigned int chid;
	extern cwmpEvtMsg pEvtMsg;
	union {
		TstVoipRtpStatistics rtpStatistics;
	} s;

	const char *pszLastname = entity ->info ->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line( name, &nVoiceProfileInstNum, &nSipLineInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;
	
	*type = entity ->info ->type;

#if 0
	/*
	 * FIXME: Now, instance number of 'Line' is seens as channel ID,  
	 *        but specification indicate thta DirectoryNumber or PhyReferenceList 
	 *        is used to identify or associate with physical interface. 
	 */
	chid = nSipLineInstNum;
#else
	chid = nVoiceProfileInstNum;
#endif
	
	if( strcmp( pszLastname, "PacketsSent" ) == 0 ) {
		if( rtk_Get_Rtp_Statistics( chid, 0, &s.rtpStatistics ) )
			return ERR_9002;	/* Internal error */
		*data = uintdup( s.rtpStatistics.nTxRtpStatsCountPacket );
	} else if( strcmp( pszLastname, "PacketsReceived" ) == 0 ) {
		if( rtk_Get_Rtp_Statistics( chid, 0, &s.rtpStatistics ) )
			return ERR_9002;	/* Internal error */
		*data = uintdup( s.rtpStatistics.nRxRtpStatsCountPacket );
	} else if( strcmp( pszLastname, "BytesSent" ) == 0 ) {
		if( rtk_Get_Rtp_Statistics( chid, 0, &s.rtpStatistics ) )
			return ERR_9002;	/* Internal error */
		*data = uintdup( s.rtpStatistics.nTxRtpStatsCountByte );
	} else if( strcmp( pszLastname, "BytesReceived" ) == 0 ) {
		if( rtk_Get_Rtp_Statistics( chid, 0, &s.rtpStatistics ) )
			return ERR_9002;	/* Internal error */
		*data = uintdup( s.rtpStatistics.nRxRtpStatsCountByte );
	} else if( strcmp( pszLastname, "PacketsLost" ) == 0 ) {
		if( rtk_Get_Rtp_Statistics( chid, 0, &s.rtpStatistics ) )
			return ERR_9002;	/* Internal error */
		*data = uintdup( s.rtpStatistics.nRxRtpStatsLostPacket );
	} else if( strcmp( pszLastname, "IncomingCallsReceived" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsReceived);
		printf("@@@@@Line %d IncomingCallsReceived:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsReceived);
	} else if( strcmp( pszLastname, "IncomingCallsAnswered" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsAnswered);
		printf("@@@@@Line %d IncomingCallsAnswered:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsAnswered);
	} else if( strcmp( pszLastname, "IncomingCallsConnected" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsConnected);
		printf("@@@@@Line %d IncomingCallsConnected:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsConnected);
	} else if( strcmp( pszLastname, "IncomingCallsFailed" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsFailed);
		printf("@@@@@Line %d IncomingCallsFailed:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].incomingCallsFailed);
	} else if( strcmp( pszLastname, "OutgoingCallsAttempted" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsAttempted);
		printf("@@@@@Line %d OutgoingCallsAttempted:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsAttempted);
	} else if( strcmp( pszLastname, "OutgoingCallsAnswered" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsAnswered);
		printf("@@@@@Line %d OutgoingCallsAnswered:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsAnswered);
	} else if( strcmp( pszLastname, "OutgoingCallsConnected" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsConnected);
		printf("@@@@@Line %d OutgoingCallsConnected:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsConnected);
	} else if( strcmp( pszLastname, "OutgoingCallsFailed" ) == 0 ) {
		*data = uintdup( pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsFailed);
		printf("@@@@@Line %d OutgoingCallsFailed:%d\n",nVoiceProfileInstNum, pEvtMsg.voiceProfileLineStatusMsg[nVoiceProfileInstNum].outgoingCallsFailed);
	} else {
		*data = NULL;
		return ERR_9005;
	}

	return 0;
}

static int setVoiceProfileLineStatusEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	return ERR_9008;	/* Parameters are read only */
}

static int getVoiceProfileLineSipEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	/* VoiceServer.{i}.VoiceProfile.{i}.SIP.Line.{i}.SIP. */
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nNumberOfLine;
	union {
		char AuthUserName[ 128 ];
		char AuthPassword[ 128 ];
		char URI[ 389 ];
	} s;

	const char *pszLastname = entity ->info ->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line( name, &nVoiceProfileInstNum, &nSipLineInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;
	
	*type = entity ->info ->type;

	if( strcmp( pszLastname, "AuthUserName" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__SIP__AUTH_USER_NAME, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.AuthUserName );
		*data = strdup( s.AuthUserName );
	} else if( strcmp( pszLastname, "AuthPassword" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__SIP__AUTH_PASSWORD, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   s.AuthPassword );
		*data = strdup( s.AuthPassword );
	} else if( strcmp( pszLastname, "URI" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__SIP__URI, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.URI );
		*data = strdup( s.URI );
	} else {
		*data = NULL;
		return ERR_9005;
	}

	return 0;
}

static int setVoiceProfileLineSipEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	/* VoiceServer.{i}.VoiceProfile.{i}.SIP.Line.{i}.SIP. */
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nNumberOfLine;

	const char *pszLastname = entity ->info ->name;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line( name, &nVoiceProfileInstNum, &nSipLineInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;

	if( entity ->info ->type != type )
		return ERR_9006;

	if( strcmp( pszLastname, "AuthUserName" ) == 0 ) {
		mib_set_type2( MIB_VOICE_PROFILE__LINE__SIP__AUTH_USER_NAME, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   data );
		return 1;
	} else if( strcmp( pszLastname, "AuthPassword" ) == 0 ) {
		mib_set_type2( MIB_VOICE_PROFILE__LINE__SIP__AUTH_PASSWORD, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   data );
		return 1;
	} else if( strcmp( pszLastname, "URI" ) == 0 ) {
		mib_set_type2( MIB_VOICE_PROFILE__LINE__SIP__URI, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   data );
		return 1;
	} else {
		return ERR_9005;
	}

	return 0;
}

static int getVoiceProfileLineEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	/* VoiceServer.{i}.VoiceProfile.{i}.SIP.Line.{i}. */
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nNumberOfLine;
	union {
		enable_t Enable;
		char DirectoryNumber[ 32 ];
		line_status_t Status;
	} s;
	
	const char *pszLastname = entity ->info ->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line( name, &nVoiceProfileInstNum, &nSipLineInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;
	
	*type = entity ->info ->type;

	if( strcmp( pszLastname, "Enable" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__ENABLE, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.Enable );
		*data = strdup_Enable( s.Enable );
	} else if( strcmp( pszLastname, "DirectoryNumber" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__DIRECTOR_NUMBER, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   s.DirectoryNumber );
		*data = strdup( s.DirectoryNumber );
	} else if( strcmp( pszLastname, "Status" ) == 0 ) {
		mib_get_type2( MIB_VOICE_PROFILE__LINE__STATUS, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.Status );
		*data = strdup_LineStatus( s.Status );
	} else {
		*data = NULL;
		return ERR_9005;
	}

	return 0;
}

static int setVoiceProfileLineEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	/* VoiceServer.{i}.VoiceProfile.{i}.SIP.Line.{i}. */
	unsigned int nVoiceProfileInstNum;
	unsigned int nSipLineInstNum;
	unsigned int nNumberOfLine;
	union {
		enable_t Enable;
		char DirectoryNumber[ 32 ];
		line_status_t Status;
	} s;
	
	const char *pszLastname = entity ->info ->name;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
		
	if( !GetOneBasedInstanceNumber_VoiceProfile_Line( name, &nVoiceProfileInstNum, &nSipLineInstNum ) )
		return -1;
	
	if( -- nVoiceProfileInstNum >= MAX_PROFILE_COUNT )	/* convert to zero based */
		return -1;
	
	if( !mib_get_type1( MIB_VOICE_PROFILE__NUMBER_OF_LINES, nVoiceProfileInstNum, &nNumberOfLine ) )
		return -1;
	
	if( -- nSipLineInstNum >= nNumberOfLine )	/* convert to zero based */
		return -1;
	
	if( entity ->info ->type != type )
		return ERR_9006;

	if( strcmp( pszLastname, "Enable" ) == 0 ) {
		str2id_Enable( data, &s.Enable );
		mib_set_type2( MIB_VOICE_PROFILE__LINE__ENABLE, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   &s.Enable );
		return 1;
	} else if( strcmp( pszLastname, "DirectoryNumber" ) == 0 ) {
		mib_set_type2( MIB_VOICE_PROFILE__LINE__DIRECTOR_NUMBER, 
					   nVoiceProfileInstNum, nSipLineInstNum,
					   data );
		return 1;
	} else if( strcmp( pszLastname, "Status" ) == 0 ) {
		return ERR_9008;
	} else {
		return ERR_9005;
	}

	return 0;
}

int objVoiceProfileLine(char *name, struct CWMP_LEAF *entity, int type, void *data)
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
			
			if( create_Object( c, tVoiceProfileLineLinkNode, sizeof(tVoiceProfileLineLinkNode), MAX_LINE_PER_PROFILE, 1 ) < 0 )
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

