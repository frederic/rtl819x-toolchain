#include "rtk_voip.h"
#include "con_register.h"
#include "voip_init.h"
#include "voip_proc.h"

#ifndef CONFIG_RTK_VOIP_QOS
#error "Build this file only if CONFIG_RTK_VOIP_QOS defined!!"
#endif

extern voip_con_t *get_voip_cons_ptr( void );

static int Check_Any_ChanEnabled(void)
{
	// call by networking driver 
	int i;
	voip_con_t *p_con = get_voip_cons_ptr();
	
	for( i = 0; i < CON_CH_NUM; i ++, p_con ++ )
		if( p_con ->enabled )
			return 1;
	
	return 0;
}


static __init int init_check_voip_channel_loading( void )
{
	extern int ( *check_voip_channel_loading )( void );
	
	check_voip_channel_loading = Check_Any_ChanEnabled;
	
	return 0;
}

voip_initcall( init_check_voip_channel_loading );

// --------------------------------------------------------
// proc 
// --------------------------------------------------------
static int voip_check_channel_loading_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
	extern int ( *check_voip_channel_loading )( void );
	int n = 0;

	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}
	
	n += sprintf( buf + n, "callback=%p\n", check_voip_channel_loading );
	if( check_voip_channel_loading )
		n += sprintf( buf + n, "\tvalue=%d\n", check_voip_channel_loading() );
	
	*eof = 1;
	return n;
}

int __init voip_check_channel_loading_proc_init( void )
{
	create_proc_read_entry( PROC_VOIP_DIR "/load_chk", 0, NULL, voip_check_channel_loading_read_proc, NULL );

	return 0;
}

void __exit voip_check_channel_loading_proc_exit( void )
{
	remove_proc_entry( PROC_VOIP_DIR "/load_chk", NULL );
}

voip_initcall_proc( voip_check_channel_loading_proc_init );
voip_exitcall( voip_check_channel_loading_proc_exit );

