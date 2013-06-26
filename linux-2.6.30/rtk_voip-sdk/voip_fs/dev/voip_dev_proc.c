#include "voip_init.h"
#include "voip_proc.h"

static int voip_devices_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
	extern int sprintf_voip_dev_entry( char *buff, const char *format );
	int len = 0;
	
	len += sprintf( buf + len, "minor nr name\n" );
	
	len += sprintf_voip_dev_entry( buf + len, "%4d  %2d %s\n" );

	*eof = 1;	
	return len;
}

static int __init voip_devices_proc_init( void )
{
	create_proc_read_entry( PROC_VOIP_DIR "/devices", 0, NULL, voip_devices_read_proc, NULL );
	
	return 0;
}

static void __exit voip_devices_proc_exit( void )
{
	remove_voip_proc_entry( PROC_VOIP_DIR "/devices", NULL );
}

voip_initcall_proc( voip_devices_proc_init );
voip_exitcall( voip_devices_proc_exit );

