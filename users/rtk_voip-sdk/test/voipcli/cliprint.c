#include <stdio.h>
#include <stdarg.h>

#include "cliprint.h"

static int bCliPrint = 1;

int CliPrintf( const char *format, ... )
{
	va_list marker;
	int ret;
	
	if( !bCliPrint )
		return 0;
	
	va_start( marker, format );
	
	ret = vprintf( format, marker );
	
	va_end( marker );
	
	return ret;
}

int EnableCliPrintf( int bEnable )
{
	int old = bCliPrint;
	
	bCliPrint = bEnable;
	
	return old;
}

