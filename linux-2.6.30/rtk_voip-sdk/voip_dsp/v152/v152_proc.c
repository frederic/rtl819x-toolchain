#include <linux/string.h>
#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_debug.h"
#include "voip_control.h"
#include "voip_params.h"
#include "voip_init.h"
#include "voip_proc.h"
#include "v152_api.h"
#include "v152_priv.h"

// state string 
static const char * const v152st_str[] = {
	"ST_AUDIO",
	"ST_AUDIO_ING",
	"ST_VBD",
	"ST_VBD_ING",
	"ST_ERR_SID",
};

#define SIZE_OF_STATE_STR		( sizeof( v152st_str ) / sizeof( v152st_str[ 0 ] ) )

CT_ASSERT( ST_TOTAL_NUM == SIZE_OF_STATE_STR );

// reason string 
#define _M_REASON( x )	{ #x, x }

static const struct {
	const char *str;
	v152reason reason;
} v152reason_str[] = {
	_M_REASON( REASON_NONE ),
	// reason to VBD
	_M_REASON( REASON_SIG_VBD_CED ),
	_M_REASON( REASON_SIG_VBD_PREAMBLE ),
	_M_REASON( REASON_SIG_VBD_CNG ),
	_M_REASON( REASON_RTP_VBD ),
	// reason to AUDIO
	_M_REASON( REASON_VOC_BI_SILENCE ),
	_M_REASON( REASON_TDM_VOICE ),
	_M_REASON( REASON_TDM_SIG_END ),
	_M_REASON( REASON_RTP_AUDIO	),
	_M_REASON( REASON_MGC_SIG ),
	_M_REASON( REASON_OFB_SIG ),
};
#define SIZE_OF_REASON_STR	( sizeof( v152reason_str ) / sizeof( v152reason_str[ 0 ] ) )

#undef _M_REASON

static const char *v152_st_string( v152st st )
{
	if( st < SIZE_OF_STATE_STR )
		return v152st_str[ st ];
	
	return "";
}

static const char *v152_reason_string( v152reason reason )
{
	int i;
	
	for( i = 0; i < SIZE_OF_REASON_STR; i ++ )
		if( v152reason_str[ i ].reason == reason )
			return v152reason_str[ i ].str;
	
	return "";
}

static int voip_v152_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
	extern int g_dynamic_pt_remote_vbd[];
	extern int g_dynamic_pt_local_vbd[];
	extern TstVoipPayLoadTypeConfig astVoipPayLoadTypeConfig[];
	int ss;
	int n = 0;

	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}
	
	if( IS_CH_PROC_DATA( data ) ) {
		//ch = CH_FROM_PROC_DATA( data );
		//n = sprintf( buf, "channel=%d\n", ch );
	} else {
		const v152_session_vars_t *pvar;
		
		ss = SS_FROM_PROC_DATA( data );
		pvar = &v152_vars.session[ ss ];
		
		n = sprintf( buf, "session=%d\n", ss );
		n += sprintf( buf + n, "Enable:\t%d\n", ( v152_vars.bitsEnable & ( 1 << ss ) ? 1 : 0 ) );
		n += sprintf( buf + n, "State:\t%s\n", v152_st_string( pvar ->state ) );
		n += sprintf( buf + n, "Reason-Audio:\t%s\t(%08lX)\n", v152_reason_string( pvar ->reasonAudio & 0xFF ), ( long )pvar ->reasonAudio );
		n += sprintf( buf + n, "Reason-VBD:\t%s\t(%08lX)\n", v152_reason_string( pvar ->reasonVBD & 0xFF ), ( long )pvar ->reasonVBD );
		n += sprintf( buf + n, "PT: remote:%d, local:%d, codec=%d\n", g_dynamic_pt_remote_vbd[ ss ], g_dynamic_pt_local_vbd[ ss ], astVoipPayLoadTypeConfig[ ss ].uPktFormat_vbd );
		n += sprintf( buf + n, "silence:\t%lu\n", pvar ->silence );
	}
	
	*eof = 1;
	return n;
}

int __init voip_v152_proc_init( void )
{
	//create_voip_channel_proc_read_entry( "v152", voip_v152_read_proc );
	create_voip_session_proc_read_entry( "v152", voip_v152_read_proc );

	return 0;
}

void __exit voip_v152_proc_exit( void )
{
	//remove_voip_channel_proc_entry( "v152" );
	remove_voip_session_proc_entry( "v152" );
}

voip_initcall_proc( voip_v152_proc_init );
voip_exitcall( voip_v152_proc_exit );

