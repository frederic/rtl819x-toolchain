#include "prmt_limit.h"
#include "prmt_voiceservice.h"
#include "prmt_capabilities.h"
#include "prmt_voice_profile.h"

static int getVSEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);

#if 0
struct sCWMP_ENTITY tVoiceServiceEntity[] = {
/*	{ name,								type,			flag,		accesslist,	getvalue,		setvalue,		next_table,				sibling } */
	{ "VoiceProfileNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,	NULL,		getVSEntity, 	NULL,			NULL,					NULL },
	{ "Capabilities",					eCWMP_tOBJECT,	CWMP_READ,	NULL,		NULL,			NULL,			tCapabilitiesEntity,	NULL },
	{ "VoiceProfile",					eCWMP_tOBJECT,	CWMP_READ,	NULL,		NULL,			objVoiceProfile,NULL,					NULL },
	{ "",								eCWMP_tNONE,	0,			NULL,		NULL,			NULL,			NULL,					NULL },
};
#endif

// node 
struct CWMP_OP tVoiceServiceEntityNodeOP = {
	NULL, objVoiceProfile, 
};

struct CWMP_PRMT tVoiceServiceEntityNodeInfo[] = {
	/*(name,			type,		flag,			op)*/
	{ "Capabilities",	eCWMP_tOBJECT,	CWMP_READ,	NULL },
	{ "VoiceProfile",	eCWMP_tOBJECT,	CWMP_READ,	&tVoiceServiceEntityNodeOP },
};

enum {
	eVoiceServiceCapabilities, 
	eVoiceServiceVoiceProfile, 
};

struct CWMP_NODE tVoiceServiceEntityNodes[] = {
	/*info,  				leaf,			next)*/
	{ &tVoiceServiceEntityNodeInfo[ eVoiceServiceCapabilities ], tCapabilitiesEntityLeaves, tCapabilitiesEntityNodes },
	{ &tVoiceServiceEntityNodeInfo[ eVoiceServiceVoiceProfile ], NULL, NULL },	
	{ NULL },
};

// leaf 
struct CWMP_OP tVoiceServiceEntityLeafOP = {
	getVSEntity, NULL, 
};

struct CWMP_PRMT tVoiceServiceEntityLeafInfo[] = {
	/*(name,			type,		flag,			op)*/
	{ "VoiceProfileNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,	&tVoiceServiceEntityLeafOP },
};

enum eVoiceServiceEntityLeaf {
	eVoiceProfileNumberOfEntries,
};

struct CWMP_LEAF tVoiceServiceEntityLeaves[] = {
	{ &tVoiceServiceEntityLeafInfo[ eVoiceProfileNumberOfEntries ] },
	{ NULL },
};

#if 0
struct sCWMP_ENTITY tVoiceService[] = {
/*	{ name,		type,			flag,								accesslist,	getvalue,	setvalue,	next_table,				sibling } */
	{ "0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL,		NULL,		NULL,		tVoiceServiceEntity, 	NULL },
};
#endif

struct CWMP_PRMT tVoiceServiceInfo[] =
{
/*(name,			type,		flag,		op)*/
{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL},
};

struct CWMP_LINKNODE tVoiceService[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tVoiceServiceInfo[0],	tVoiceServiceEntityLeaves, tVoiceServiceEntityNodes,		NULL,			0},
};

int objVoiceService(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	switch( type ) {
	case eCWMP_tINITOBJ:
	{
		int num=0;
		//struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;
		struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		num = 1;	/* Now, we has one voice service only. */
		//int create_Object( struct CWMP_LINKNODE **table, struct CWMP_LINKNODE ori_table[], int size, unsigned int num, unsigned int from);

		if( create_Object( c, tVoiceService, sizeof(tVoiceService), 1, 1 ) < 0 )
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

static int getVSEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	*type = entity->info->type;
	*data = uintdup( MAX_PROFILE_COUNT );

	return 0;
}

