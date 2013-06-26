#include <stdio.h>
#include <string.h>
#include "ui_config.h"
#include "flash_rw_api.h"
#include "ioctl_codec.h"
#include "ui_flags.h"

#ifndef _TEST_MODE
//#define _RAM_SIM
#define _NOT_SIM
#else
#define _FILE_SIM
#endif

/* ****************************************************************************
 * Use File to simulate flash 
 * **************************************************************************** */
#ifdef _FILE_SIM

#ifdef __GNUC__
  #define FLASH_FILENAME	"/mnt/nfs/ipphone/flash.dat"
  #define debug_flash( args ... )
  //#define debug_flash		printf
#else
  #define FLASH_FILENAME	"./flash.dat"
  #define debug_flash		//printf
#endif

FILE *fp;
static unsigned long nNotValidateWritingCount;

/* Initialize flash r/w interface */
void InitializeFlash( void )
{
	nNotValidateWritingCount = 0;

	fp = fopen( FLASH_FILENAME, "r+b" );

	if( fp == 0 ) {
		debug_out( "Flash not ready!\n" );
		return;
	}
}

/* Write to flash */
void FlashWriteData( unsigned long addr, const void *pData, unsigned int len )
{
	debug_flash( "Write: %d %d\n", addr, len );

	if( fp == 0 ) {
		debug_out( "Flash not ready!\n" );
		return;
	}

	nNotValidateWritingCount ++;

	fseek( fp, addr, SEEK_SET );

	fwrite( pData, len, 1, fp );
}

/* Validate writing */
void FlashValidateWriting( unsigned int bImmediately )
{
	/* Does it need validate writing? */
	if( nNotValidateWritingCount == 0 )
		return;
	
	/* write directly, or wait for idle */
	if( bImmediately ) {
		nNotValidateWritingCount = 0;
		printf( "FlashValidateWriting()\n" );
	} else
		fLongJobFlags.b.flashWrite = 1;
}

/* Read from flash */
void FlashReadData( unsigned long addr, void *pData, unsigned int len )
{
	debug_flash( "Read: %d %d\n", addr, len );

	if( fp == 0 ) {
		debug_out( "Flash not ready!\n" );
		return;
	}

	fseek( fp, addr, SEEK_SET );

	fread( pData, len, 1, fp );
}

/* Terminate flash r/w interface */
void TerminateFlash( void )
{
	if( fp )
		fclose( fp );
}
#endif /* _FILE_SIM */


/* ****************************************************************************
 * Use RAM to simulate flash 
 * **************************************************************************** */
#ifdef _RAM_SIM
ui_falsh_layout_t	ui_flash_data;

#if NUM_OF_CALL_RECORD_UNITS == 10
static const unsigned char pSort[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
#else
???
#endif

/* Initialize flash r/w interface */
void InitializeFlash( void )
{
	int i;
	
	/* version */
	ui_flash_data.version = FLASH_VERSION;

	/* phonebook */
	ui_flash_data.phonebook.nNumberOfPhonebookRecord = 1;
	strcpy( ( char * )ui_flash_data.phonebook.records[ 0 ].szName, "ABCD" );
	strcpy( ( char * )ui_flash_data.phonebook.records[ 0 ].bcdNumber, "\x21\x43\x65\xFF" );

	for( i = 0; i < NUM_OF_PHONEBOOK_RECORD; i ++ )
		ui_flash_data.phonebook.riFromViPhonebookRecordSort[ i ] = i;

	/* mode settings */
	/* keypree tone */
	ui_flash_data.bKeypressTone = 1;
	
#if MAX_VOLUME_VALUE == 255
	ui_flash_data.nModeOutVolumeReceiver = 223;	/* experimental value */
	ui_flash_data.nModeOutVolumeSpeaker = 251;
#elif MAX_VOLUME_VALUE == 63
	ui_flash_data.nModeOutVolumeReceiver = 50;
	ui_flash_data.nModeOutVolumeSpeaker = 50;
	ui_flash_data.nModeInVolumeMic_R = 32;
	ui_flash_data.nModeInVolumeMic_S = 32;
#else
	???
#endif

	/* call records */
	ui_flash_data.callrecord.missed.used = 0;
	ui_flash_data.callrecord.incoming.used = 0;
	ui_flash_data.callrecord.outgoing.used = 0;

	memcpy( ui_flash_data.callrecord.missed.sort, pSort, NUM_OF_CALL_RECORD_UNITS );
	memcpy( ui_flash_data.callrecord.incoming.sort, pSort, NUM_OF_CALL_RECORD_UNITS );
	memcpy( ui_flash_data.callrecord.outgoing.sort, pSort, NUM_OF_CALL_RECORD_UNITS );

}

/* Write to flash */
void FlashWriteData( unsigned long addr, const void *pData, unsigned int len )
{
	if( addr >= MAX_ADDR_OF_FLASH || len == 0 )
		return;
	else if( addr + len >= MAX_ADDR_OF_FLASH )
		len = MAX_ADDR_OF_FLASH - addr;

	memcpy( ( unsigned char * )&ui_flash_data + addr, pData, len );
}

/* Validate writing */
void FlashValidateWriting( unsigned int bImmediately )
{
}

/* Read from flash */
void FlashReadData( unsigned long addr, void *pData, unsigned int len )
{
	if( addr >= MAX_ADDR_OF_FLASH || len == 0 )
		return;
	else if( addr + len >= MAX_ADDR_OF_FLASH )
		len = MAX_ADDR_OF_FLASH - addr;

	memcpy( pData, ( unsigned char * )&ui_flash_data + addr, len );
}

/* Terminate flash r/w interface */
void TerminateFlash( void )
{
}
#endif /* _RAM_SIM */

/* ****************************************************************************
 * Real flash read/write functions  
 * **************************************************************************** */
#ifdef _NOT_SIM
#if CONFIG_RTK_VOIP_PACKAGE_865X
	#include "rtl_cfgmgr.h"
	#include "rtl_board.h"
	#include <sys/shm.h>
#elif CONFIG_RTK_VOIP_PACKAGE_867X
	#include <sys/shm.h>
	#include "mib.h"
#else
	#include "../../goahead-2.1.1/LINUX/apmib.h"
#endif

#include "voip_flash_client.h"

static voip_flash_share_t *g_pVoIPShare;
static unsigned long nNotValidateWritingCount;

/* Initialize flash r/w interface */
void InitializeFlash( void )
{
	nNotValidateWritingCount = 0;
	
	/* share memory */
	if (voip_flash_client_init(&g_pVoIPShare, VOIP_FLASH_WRITE_CLIENT_IPPHONE) == -1)
	{
		debug_out( "voip_flash_client_init failed.\n" );
		return;
	}
}

/* Write to flash */
void FlashWriteData( unsigned long addr, const void *pData, unsigned int len )
{
	/* convert addr base from ui_falsh_layout_t to voipCfgParam_t */
	if( len == 0 )
		return;
	
	if( addr >= MAX_ADDR_OF_FLASH ) {
		debug_out( "Flash Write over\n" );
		return;
	}
	
	nNotValidateWritingCount ++;	/* count ++ */
	
	if( addr + len > MAX_ADDR_OF_FLASH ) {
		len = MAX_ADDR_OF_FLASH - addr;
		debug_out( "Flash Write truncate\n" );
	}
	
	addr += MY_FIELD_OFFSET( voipCfgParam_t, ui );
	
	/* voip_flash_client_write() will do this memcpy() */
	//memcpy( ( unsigned char * )&( g_pVoIPShare ->voip_cfg ) + addr, pData, len );

	voip_flash_client_write( addr, len, pData, 1 /* ignore interrupt signal */ );
}

/* Validate writing */
void FlashValidateWriting( unsigned int bImmediately )
{
	/* Does it need validate writing? */
	if( nNotValidateWritingCount == 0 )
		return;
	
	/* write directly, or wait for idle */
	if( bImmediately ) {
		nNotValidateWritingCount = 0;
		voip_flash_client_validate_writing( 1 /* ignore interrupt signal */ );
	} else
		fLongJobFlags.b.flashWrite = 1;
}

/* Read from flash */
void FlashReadData( unsigned long addr, void *pData, unsigned int len )
{
	/* convert addr base from ui_falsh_layout_t to voipCfgParam_t */
	if( len == 0 )
		return;
	
	if( addr >= MAX_ADDR_OF_FLASH ) {
		debug_out( "Flash Read over\n" );
		return;
	}
	
	if( addr + len > MAX_ADDR_OF_FLASH ) {
		len = MAX_ADDR_OF_FLASH - addr;
		debug_out( "Flash Read truncate\n" );
	}
	
	addr += MY_FIELD_OFFSET( voipCfgParam_t, ui );

	memcpy( pData, ( unsigned char * )&( g_pVoIPShare ->voip_cfg ) + addr, len );
}

/* Terminate flash r/w interface */
void TerminateFlash( void )
{
	/* share memory */
	voip_flash_client_close();
}
#endif /* _NOT_SIM */

unsigned char FlashReadOneByte( unsigned long addr )
{
	unsigned char data;
	
	FlashReadData( addr, &data, sizeof( data ) );
	
	return data;
}

unsigned short FlashReadTwoBytes( unsigned long addr )
{
	unsigned short data;
	
	FlashReadData( addr, &data, sizeof( data ) );
	
	return data;
}

unsigned long FlashReadFourBytes( unsigned long addr )
{
	unsigned long data;
	
	FlashReadData( addr, &data, sizeof( data ) );
	
	return data;
}

void FlashWriteOneByte( unsigned long addr, unsigned char data )
{
	FlashWriteData( addr, &data, sizeof( data ) );
}

void FlashWriteTwoBytes( unsigned long addr, unsigned short data )
{
	FlashWriteData( addr, &data, sizeof( data ) );
}

void FlashWriteFourBytes( unsigned long addr, unsigned long data )
{
	FlashWriteData( addr, &data, sizeof( data ) );
}

