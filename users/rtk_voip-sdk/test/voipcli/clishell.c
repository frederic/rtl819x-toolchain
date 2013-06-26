#include <stdio.h>
#include <string.h>

#include "cliprint.h"
#include "cmdcore.h"
#include "clishell.h"

static int run;

void PrintHelp( void )
{
	//CliPrintf( "Help page is not implement yet!\n" );
	CliPrintf( "Node command:\n" );
	CliPrintf( "	ls:		list all nodes\n" );
	CliPrintf( "	back | ..:	back one level\n" );
	CliPrintf( "	root | /:	back to root level\n" );
	CliPrintf( "	[nodes]:	enter the node\n" );
	CliPrintf( "Leaf command (in leaf node only):\n" );
	CliPrintf( "	args:		input args iteratively\n" );
	CliPrintf( "	run:		run voip control\n" );
	CliPrintf( "Program command\n" );
	CliPrintf( "	bye | exit:	exit this program\n" );
	CliPrintf( "	h | ? | help:	this page\n" );
}

static int GlobalCmdParser( const char * const pszcmd )
{
	if( strcmp( pszcmd, "bye" ) == 0 ||
		strcmp( pszcmd, "exit" ) == 0 )
	{
		run = 0;
		return 1;
	} else if( strcmp( pszcmd, "h" ) == 0 ||
			   strcmp( pszcmd, "?" ) == 0 ||
			   strcmp( pszcmd, "help" ) == 0 )
	{
		PrintHelp();
		return 1;
	} 
	
	return 0;
}

static void StripTailNewLine( char *buffer )
{
	int len;
	char ch;
	
	len = strlen( buffer );
	
	while( 1 ) {
		ch = buffer[ len - 1 ];
		
		if( ch == '\n' || ch == '\r' )
			buffer[ -- len ] = '\x0';
		else
			break;
	}
}

int clishell_main(int argc, char *argv[])
{
	char buffer[ 512 ];
		
	run = 1;
	
	InitializeCmdCore();
	
	while( run ) {
		
		/* print prompt */
		CliPrintf( "VoIP" );
		
		PrintCmdPromptPath();	/* more prompt here */
		
		CliPrintf( ">" );
		
		/* read stdin */
		/* If user use 'echo xxxx | cli' without this constraint, 
			it will enter infinite loop */ 
		if( fgets( buffer, 512, stdin ) == NULL )
			break;
		
		/* remove tail '\n'  */
		StripTailNewLine( buffer );
				
		/* parsing command */
		if( CmdCoreParser( buffer ) )
			;
		else if( GlobalCmdParser( buffer ) )
			;
		else {
			//CliPrintf( "Enter 'h', '?' or 'help' to show help.\n" );
		}
	}

	return 0;
}


