#include <stdio.h>
#include <string.h>
#include "kernel_config.h"
#include "rtk_voip.h"
#ifdef CONFIG_RTK_VOIP_IP_PHONE
#include "ui_flash_layout.h"
#endif


int main( int argc, const char **argv )
{
	int output = 3;

	if( argc > 1 ) {
		if( strcmp( argv[ 1 ], "SLIC_NUM" ) == 0 )
			output = 1;
		else if( strcmp( argv[ 1 ], "CON_CH_NUM" ) == 0 )
			output = 2;
	}
		
	if( output & 1 )
		;//printf( "#define SLIC_NUM	%d", SLIC_NUM );

	if( ( output & 3  ) == 3 )
		printf( "\\n" );

	if( output & 2 )
		printf( "#define CON_CH_NUM	%d", CON_CH_NUM );

	return 0;
}

