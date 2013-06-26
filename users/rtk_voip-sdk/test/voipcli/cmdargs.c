#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef __ECOS 
#include <sys/types.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "cliprint.h"
#include "cmdtree.h"
#include "cmdargs.h"

static void PrintArgsValue_Num( const unsigned char *pArgsBuffer, 
								const args_t *pArgs )
{
	unsigned long val;

	switch( pArgs ->size ) {
	case 1:
		val = pArgsBuffer[ pArgs ->offset ];
		break;
	case 2:
		val = *( ( unsigned short * )&pArgsBuffer[ pArgs ->offset ] );
		break;
	case 4:
		val = *( ( unsigned long * )&pArgsBuffer[ pArgs ->offset ] );
		break;
	default:
		val = 12563478;
		break;
	}
	
	CliPrintf( "%lu", val );
}

static void PrintArgsValue_Str( const unsigned char *pArgsBuffer, 
								const args_t *pArgs )
{
	CliPrintf( "%s", &pArgsBuffer[ pArgs ->offset ] );
}

static void PrintArgsValue_IA4( const unsigned char *pArgsBuffer, 
								const args_t *pArgs )
{
	union {
		unsigned long all;
		struct {
			unsigned char a1;
			unsigned char a2;
			unsigned char a3;
			unsigned char a4;
		};
	} ia4;

	ia4.all = *( ( unsigned long * )&pArgsBuffer[ pArgs ->offset ] );
	
	CliPrintf( "%u.%u.%u.%u", ia4.a1, ia4.a2, ia4.a3, ia4.a4 );
}

void PrintArgsValue( const unsigned char *pArgsBuffer, 
					 const args_t *pArgs )
{
	switch( pArgs ->type ) {
	case ATYPE_IN_STR:
		PrintArgsValue_Str( pArgsBuffer, pArgs );
		break;
	
	case ATYPE_IN_IA4:
		PrintArgsValue_IA4( pArgsBuffer, pArgs );
		break;
		
	case ATYPE_IN:
	case ATYPE_OUT:
	default:
		PrintArgsValue_Num( pArgsBuffer, pArgs );
		break;
	}
}

static void WriteArgsValue_Num( unsigned char *pArgsBuffer, 
								const args_t *pArgs,
								const unsigned char *pInput )
{
	unsigned long val;
	
	val = atol( pInput );
	
	switch( pArgs ->size ) {
	case 1:
		pArgsBuffer[ pArgs ->offset ] = val;
		break;
	case 2:
		*( ( unsigned short * )&pArgsBuffer[ pArgs ->offset ] ) = val;
		break;
	case 4:
		*( ( unsigned long * )&pArgsBuffer[ pArgs ->offset ] ) = val;
		break;
	default:
		break;
	}
}

static void WriteArgsValue_Str( unsigned char *pArgsBuffer, 
								const args_t *pArgs,
								const unsigned char *pInput )
{
	unsigned char *pstr;
	
	pstr = &pArgsBuffer[ pArgs ->offset ];
	
	strncpy( pstr, pInput, pArgs ->size );
	pstr[ pArgs ->size - 1 ] = '\x0';
}

static void WriteArgsValue_IA4( unsigned char *pArgsBuffer, 
								const args_t *pArgs,
								const unsigned char *pInput )
{
	unsigned long ia4;
	
	ia4 = inet_addr( pInput );
	
	*( ( unsigned long * )&pArgsBuffer[ pArgs ->offset ] ) = ia4;
}

void WriteArgsValue( unsigned char *pArgsBuffer, 
					 const args_t *pArgs,
					 const unsigned char *pInput )
{
	switch( pArgs ->type ) {
	case ATYPE_IN_STR:
		WriteArgsValue_Str( pArgsBuffer, pArgs, pInput );
		break;
		
	case ATYPE_IN_IA4:
		WriteArgsValue_IA4( pArgsBuffer, pArgs, pInput );
		break;
		
	case ATYPE_IN:
	case ATYPE_OUT:
	default:
		WriteArgsValue_Num( pArgsBuffer, pArgs, pInput );
		break;
	}
}

