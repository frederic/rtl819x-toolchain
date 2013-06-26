#include <stdio.h>
#include <string.h>
#include "ui_config.h"
#include "ui_limits.h"
#include "ioctl_net.h"

int GetDefaultGatewayText( unsigned char *pszGateway )
{
	FILE *fp;
	unsigned char szBuffer[ 256 ];
	struct {
		unsigned char Iface[ 10 ];
		unsigned long Destination;
		unsigned long Gateway;
		unsigned int Flags;
		unsigned int RefCnt;
		unsigned int Use;
		unsigned int Metric;
		unsigned long Mask;
		unsigned int MTU;
		unsigned int Window;
		unsigned int IRTT;
	} field;
	unsigned int bFound = 0;
	
	if( !( fp = fopen( "/proc/net/route", "r" ) ) )
		return 0;
		
	/*
	 * Iface   Destination     Gateway         Flags   RefCnt  Use     Metric  Mask            MTU     WindowIRTT
	 * br0     C0A80100        00000000        0001    0       0       0       FFFFFF00        40      0     0
	 * br0     00000000        C0A80121        0003    0       0       0       00000000        40      0     0
	 */
	while( !feof( fp ) ) {
		fgets( szBuffer, 256, fp );
		
		if( sscanf( szBuffer, "%s %lX %lX %X %X %X %X %lX %X %X %X",
					field.Iface, &field.Destination, &field.Gateway, 
					&field.Flags, &field.RefCnt, &field.Use, 
					&field.Metric, &field.Mask, &field.MTU, 
					&field.Window, &field.IRTT ) == 11 )
		{
			/* /usr/src/linux/include/linux/route.h define flags */
			if( field.Flags & 0x0002 ) {	/* RTF_GATEWAY == 0x0002 */
				bFound = 1;
				break;
			}
		}
	}

	fclose( fp );
	
	/* analyze done */
	if( !bFound )
		return 0;
		
	sprintf( pszGateway, "%u.%u.%u.%u", 
						*( ( unsigned char *)&field.Gateway + 0 ),
						*( ( unsigned char *)&field.Gateway + 1 ),
						*( ( unsigned char *)&field.Gateway + 2 ),
						*( ( unsigned char *)&field.Gateway + 3 ) );

	return 1;
}

int GetFirstDNSText( unsigned char *pszDNS )
{
	FILE *fp;
	unsigned char szBuffer[ 256 ];
	unsigned char szDNS[ 256 ];
	unsigned int bFound = 0;
	
	if( !( fp = fopen( "/etc/resolv.conf", "r" ) ) )
		return 0;	
	
	/*
	 * search localdomain
	 * nameserver 172.21.1.10
	 */
	while( !feof( fp ) ) {
		fgets( szBuffer, 256, fp );
		
		if( sscanf( szBuffer, "nameserver %s", szDNS ) == 1 ) {
			if( strlen( szDNS ) > MAX_LEN_OF_IP_ADDR )
				continue;
			
			bFound = 1;
			break;
		}
	}

	fclose( fp );	
	
	/* analyze done */
	if( !bFound )
		return 0;
		
	strcpy( pszDNS, szDNS );
	
	return 1;
}

