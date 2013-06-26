#include <stdio.h>

#include "cliprint.h"
#include "clishell.h"

int voip_cli_main(int argc, char *argv[])
{
	if( argc >= 2 && argv[ 1 ][ 0 ] == 'm' )
		EnableCliPrintf( 0 );	/* Disable print */
	
	clishell_main( argc, argv );
	
	return 0;
}

