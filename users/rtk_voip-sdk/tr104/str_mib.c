#include <string.h>
#include "str_mib.h"

/*
 * In order to use macro M_IMPLEMENT_STRDUP_AND_STR2ID_XXX() that can
 * generate functions automatically, we have to follow naming rule. 
 * 1. Structure has two members, which are ID and its corresponding string. 
 *    ID named 'idXxxxYyyy', and string named 'pszXxxxYyyy'. 
 *    Name of structure is 'id2str_xxxx_yyyy'. 
 * 2. Array of conversion table is named 'id2str_xxxx_yyyy'.
 * 3. We define a macro, naming 'SIZE_OF_ID2STR_XXXX_YYYY', to know 
 *    array size of conversion table. 
 * 4. Finally, add a implement macro entry, such as 
 *    M_IMPLEMENT_STRDUP_AND_STR2ID_XXX( XXXX_YYYY, XxxxYyyy, xxxx_yyyy ).
 *
 * In above description, 'xxxx' and 'yyyy' refer to two words, such as 
 * 'line' and 'status'. 
 * Upper case of 'X' and 'Y' indicate upper case alphabets. 
 * Take 'line' and 'status' for example, 
 * 'XXXX_YYYY', 'XxxxYyyy' and 'xxxx_yyyy' will be 
 * 'LINE_STATUS', 'LineStatus' and 'line_status'. 
 */
typedef struct id2str_enable_s {
	enable_t idEnable;
	const char *pszEnable;
} id2str_enable_t;

typedef struct id2str_signaling_protocol_s {
	signaling_protocol_t idSignalingProtocol;
	const char *pszSignalingProtocol;
} id2str_signaling_protocol_t;

typedef struct id2str_DTMF_method_s {
	DTMF_method_t idDTMFMethod;
	const char *pszDTMFMethod;
} id2str_DTMF_method_t;

typedef struct id2str_transport_s {
	transport_t	idTransport;
	const char *pszTransport;
} id2str_transport_t;

typedef struct id2str_line_status_s {
	line_status_t idLineStatus;
	const char *pszLineStatus;
} id2str_line_status_t;

static id2str_enable_t id2str_enable[] = {
	{ TR104_DISABLE,		"Disabled" },
	{ TR104_QUIESCENT0,	"Quiescent" },
	{ TR104_ENABLE,		"Enabled" },
};

static id2str_signaling_protocol_t id2str_signaling_protocol[] = {
	{ SIP,		"SIP" },
	{ MGCP,		"MGCP" },
	{ MGCP_NCS,	"MGCP-NCS" },
	{ H_248,	"H.248" },
	{ H_323,	"H.323" },
};

/* It must follow the web page settings*/
static id2str_DTMF_method_t id2str_DTMF_method[] = {
	{ IN_BAND,	"Inband" },
	{ RFC2833,	"RFC2833" },
	{ SIP_INFO,	"SIP INFO" },
};

static id2str_transport_t id2str_transport[] = {
	{ UDP,		"UDP" },
	{ TCP,		"TCP" },
	{ TLS,		"TLS" },
	{ SCTP,		"SCTP" },
};

static id2str_line_status_t id2str_line_status[] = {
	{ UP,				"Up" },
	{ INITIALIZING,		"Initializing" },
	{ REGISTERING,		"Registering" },
	{ UNREGISTERING,	"Unregistering" },
	{ ERROR,			"Error" },
	{ TESTING,			"Testing" },
	{ QUIESCENT1,		"Quiescent" },
	{ TR104_DISABLED,	"Disabled" },
};

#define SIZE_OF_ID2STR_ENABLE	( sizeof( id2str_enable ) / sizeof( id2str_enable[ 0 ] ) )
#define SIZE_OF_ID2STR_SIGNALING_PROTOCOL	( sizeof( id2str_signaling_protocol ) / sizeof( id2str_signaling_protocol[ 0 ] ) )
#define SIZE_OF_ID2STR_DTMF_METHOD	( sizeof( id2str_DTMF_method ) / sizeof( id2str_DTMF_method[ 0 ] ) )
#define SIZE_OF_ID2STR_TRANSPORT	( sizeof( id2str_transport ) / sizeof( id2str_transport[ 0 ] ) )
#define SIZE_OF_ID2STR_LINE_STATUS	( sizeof( id2str_line_status ) / sizeof( id2str_line_status[ 0 ] ) )

/*
 * ====================================================================
 * Functions that convert ID to string (strdup), and reverse conversion. 
 * ====================================================================
 */
#define M_IMPLEMENT_STRDUP_AND_STR2ID_XXX( ALL_U, INIT_U, ALL_L )	\
char *strdup_##INIT_U( ALL_L##_t INIT_U )							\
{																	\
	int __i;														\
	for( __i = 0; __i < SIZE_OF_ID2STR_##ALL_U; __i ++ )			\
		if( id2str_##ALL_L[ __i ].id##INIT_U == INIT_U )			\
			return strdup( id2str_##ALL_L[ __i ].psz##INIT_U );		\
	return NULL;													\
}																	\
int str2id_##INIT_U( const char *psz##INIT_U, ALL_L##_t *pid##INIT_U )			\
{																				\
	int __i;																	\
	for( __i = 0; __i < SIZE_OF_ID2STR_##ALL_U; __i ++ )						\
		if( strcmp( id2str_##ALL_L[ __i ].psz##INIT_U, psz##INIT_U ) == 0 ) {	\
			*pid##INIT_U = id2str_##ALL_L[ __i ].id##INIT_U;					\
			return 1;															\
		}																		\
	return 0;																	\
}

M_IMPLEMENT_STRDUP_AND_STR2ID_XXX( ENABLE, Enable, enable )
M_IMPLEMENT_STRDUP_AND_STR2ID_XXX( SIGNALING_PROTOCOL, SignalingProtocol, signaling_protocol )
M_IMPLEMENT_STRDUP_AND_STR2ID_XXX( DTMF_METHOD, DTMFMethod, DTMF_method )
M_IMPLEMENT_STRDUP_AND_STR2ID_XXX( TRANSPORT, Transport, transport )
M_IMPLEMENT_STRDUP_AND_STR2ID_XXX( LINE_STATUS, LineStatus, line_status )

#undef M_IMPLEMENT_STRDUP_XXX

