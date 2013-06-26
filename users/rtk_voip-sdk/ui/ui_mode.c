#include <stdio.h>
#include <string.h>
#include "ui_config.h"
#include "ui_mode.h"
#include "ui_host.h"
#include "ui_softkey.h"
#include "gs_lib.h"
#include "flash_layout.h"
#include "flash_rw_api.h"
#include "ioctl_codec.h"

fModeFlags_t fModeFlags;	CT_ASSERT( sizeof( fModeFlags ) == sizeof( fModeFlags.all ) );

unsigned char nModeOutVolume[ NUM_OF_INOUTVOL_TYPE ];
unsigned char nModeOutVolumeTemp;


#if MAX_VOLUME_VALUE == 255
  #define DEFAUT_MODE_OUT_VOL_SPEAKER	223
  #define DEFAUT_MODE_OUT_VOL_RECEIVER	223
  #define DEFAUT_MODE_IN_VOL_MIC_S		223	/* no field try */
  #define DEFAUT_MODE_IN_VOL_MIC_R		223	/* no field try */
#elif MAX_VOLUME_VALUE == 63
  #define DEFAUT_MODE_OUT_VOL_SPEAKER	32
  #define DEFAUT_MODE_OUT_VOL_RECEIVER	32
  #define DEFAUT_MODE_IN_VOL_MIC_S		32
  #define DEFAUT_MODE_IN_VOL_MIC_R		32
#else
	???
#endif

/* ================================================================== */
/* Load settings functions */
/* ================================================================== */
static const mode_info_t modeDefaultValue = {
	1,	/* keypress */
	DEFAUT_MODE_OUT_VOL_RECEIVER,	/* receiver volume */
	DEFAUT_MODE_OUT_VOL_SPEAKER,	/* speaker volume */
	DEFAUT_MODE_IN_VOL_MIC_R,		/* mic (R) */
	DEFAUT_MODE_IN_VOL_MIC_S,		/* mic (S) */
	DEF_AUTO_DIAL_TIME,				/* auto dial time */
	DEF_AUTO_ANSWER_TIME,			/* auto answer time */
	DEF_OFF_HOOK_ALARM_TIME,		/* off-hook alarm time */
	0,			/* hot line switch */
	"\xFF",		/* hot line phonenumber in bcd format */
};

void LoadModeSettingsFromStorage( int bLoadDefault )
{
	mode_info_t	mode;

	if( bLoadDefault ) {
		/* set default value */
		mode = modeDefaultValue;
	
		FlashWriteData( MY_FIELD_OFFSET( ui_falsh_layout_t, mode ),
						&mode, sizeof( mode ) );

		/* FlashValidateWriting() by caller */
	} else {
		
		FlashReadData( MY_FIELD_OFFSET( ui_falsh_layout_t, mode ),
						&mode, sizeof( mode ) );
	
		/* keypress tone */
		//bKeypressTone = mode.bKeypressTone;		
	}

	/* keypress tone */
	fModeFlags.b.keypressTone = ( mode.bKeypressTone ? 1 : 0 );
	
	/* auto dial */
	fModeFlags.b.autoDial = ( mode.nAutoDialTime ? 1 : 0 );
	
	/* auto answer */
	fModeFlags.b.autoAnswer = ( mode.nAutoAnswerTime ? 1 : 0 );

	/* off-hook alarm */
	fModeFlags.b.offHookAlarm = ( mode.nOffHookAlarmTime ? 1 : 0 );

	/* hot line */
	fModeFlags.b.hotLine = ( mode.bHotLine ? 1 : 0 );

	/* DAC volume */
	nModeOutVolume[ INOUTVOL_TYPE_RECEIVER ] = mode.nModeOutVolumeReceiver;
	nModeOutVolume[ INOUTVOL_TYPE_SPEAKER ] = mode.nModeOutVolumeSpeaker;
	nModeOutVolume[ INOUTVOL_TYPE_MIC_R ] = mode.nModeInVolumeMic_R;
	nModeOutVolume[ INOUTVOL_TYPE_MIC_S ] = mode.nModeInVolumeMic_S;

	SetVolumeOfDAC( nModeOutVolume[ INOUTVOL_TYPE_MIC_R ],
					nModeOutVolume[ INOUTVOL_TYPE_RECEIVER ] );	/* Set this DAC volume to hardware. */
}

/* ================================================================== */
/* Adjust volume realted functions */
/* ================================================================== */
static void GetMicSpkVolumePairByInOutVolType( inout_vol_type_t type, 
											   unsigned char *pVolMic,
											   unsigned char *pVolSpk )
{
	/*
	 * Using this function to get mic/spk pair volume, because 
	 * one only adjust one of mic/spk volume. 
	 * But our volume control API need provide volume pair. 
	 */
	static unsigned char idxVolmic[ NUM_OF_INOUTVOL_TYPE ] = {
		INOUTVOL_TYPE_MIC_R,
		INOUTVOL_TYPE_MIC_S,
		INOUTVOL_TYPE_MIC_R,
		INOUTVOL_TYPE_MIC_S,
	};
	
	static unsigned char idxVolspk[ NUM_OF_INOUTVOL_TYPE ] = {
		INOUTVOL_TYPE_RECEIVER,
		INOUTVOL_TYPE_SPEAKER,
		INOUTVOL_TYPE_RECEIVER,
		INOUTVOL_TYPE_SPEAKER,
	};

	*pVolMic = nModeOutVolume[ idxVolmic[ type ] ];
	*pVolSpk = nModeOutVolume[ idxVolspk[ type ] ];
}

int RefreshAdjustOutVolumeXxxFrame( int bDrawVolumeOnly, unsigned char key, 
									int bRefreshHardware, inout_vol_type_t type )
{
#if MAX_VOLUME_VALUE == 255
  #define VOLUME_INC	4
#elif MAX_VOLUME_VALUE == 63
  #define VOLUME_INC	1
#endif

	unsigned char *pModeOutVolumeXxx;
	int volume;
	unsigned char vol;
	unsigned char szBuffer[ 20 ];
	unsigned int tick;
	unsigned int i;
	unsigned char volmic, volspk;
	const unsigned char *pVolumePromptFormat;
#ifdef SOFTKEY_SUPPORT
	rect_t rect;
#endif

	/* refresh hardware or temporal */
	if( bRefreshHardware ) {
		if( type > NUM_OF_INOUTVOL_TYPE )
			debug_out( "Too large OUTVOL type 3.\n" );
	
		pModeOutVolumeXxx = &nModeOutVolume[ type ];
	} else
		pModeOutVolumeXxx = &nModeOutVolumeTemp;

	volume = *pModeOutVolumeXxx;

	if( key == VKEY_OUTVOL_PLUS )
		volume += VOLUME_INC;
	else if( key == VKEY_OUTVOL_MINUS )
		volume -= VOLUME_INC;
	else {
		if( key == '\x0' )
			;
		else {
			debug_out( "Give an invalid key.\n" );
			return 0;
		}
	}
	
	if( volume > MAX_VOLUME_VALUE || volume < 0 ) {
		vol = *pModeOutVolumeXxx;
		goto label_draw_volume;
	}

	vol = *pModeOutVolumeXxx = ( unsigned char )volume;

	if( bRefreshHardware ) {
		GetMicSpkVolumePairByInOutVolType( type, &volmic, &volspk );
		SetVolumeOfDAC( volmic, volspk );
	}

	/* draw volume */
label_draw_volume:

#ifdef SOFTKEY_SUPPORT
	GS_DrawOffScreen();
	GS_DrawSolidRectangle( 0, 0, VRAM_WIDTH, SOFTKEY_TEXT_LOC_Y, 0 );
#else
	GS_DrawOffScreenAndClearScreen();
#endif

	GS_SetCursorStatus( CURSOR_STATUS_OFF );

	switch( type ) {
	case INOUTVOL_TYPE_RECEIVER:
		pVolumePromptFormat = szVolumeReceiverWithDigitsFormat;
		break;
		
	case INOUTVOL_TYPE_SPEAKER:
		pVolumePromptFormat = szVolumeSpeakerWithDigitsFormat;
		break;
		
	case INOUTVOL_TYPE_MIC_R:
		pVolumePromptFormat = szVolumeMicRWithDigitsFormat;
		break;
		
	case INOUTVOL_TYPE_MIC_S:
	default:
		pVolumePromptFormat = szVolumeMicSWithDigitsFormat;
		break;
	}
		
	sprintf( ( char * )szBuffer, ( const char * )pVolumePromptFormat, vol );
	GS_TextOut( 0, 0, szBuffer, strlen( ( const char * )szBuffer ) );

	/* draw a [......] indicator */
	tick = ( unsigned int )vol * ( VRAM_WIDTH_IN_TEXT_UNIT - 2 ) / MAX_VOLUME_VALUE;

	GS_TextOut( 0, 1, ( const unsigned char * )"[", 1 );

	for( i = 0; i < tick; i ++ ) {
		GS_TextOut( 1 + i, 1, ( const unsigned char * )"*", 1 );
	}

	for( ; i < ( VRAM_WIDTH_IN_TEXT_UNIT - 2 ); i ++ )
		GS_TextOut( 1 + i, 1, ( const unsigned char * )".", 1 );

	GS_TextOut( VRAM_WIDTH_IN_TEXT_UNIT - 1, 1, ( const unsigned char * )"]", 1 );

#ifdef SOFTKEY_SUPPORT
	rect.left = 0;
	rect.top = 0; 
	rect.right = VRAM_WIDTH;
	rect.bottom = SOFTKEY_TEXT_LOC_Y;
	GS_DrawOnScreen( &rect );
#else
	GS_DrawOnScreen( NULL );
#endif

	return 1;

#undef VOLUME_INC
}

void HelpSetupMenuConfigOutVolumeXxxFrame( inout_vol_type_t type )
{
	rect_t rect;

	if( type >= NUM_OF_INOUTVOL_TYPE )
		debug_out( "outvol_type is too large 1.\n" );

	nModeOutVolumeTemp = nModeOutVolume[ type ];

	RefreshAdjustOutVolumeXxxFrame( 0 /* draw entire */, 
									0 /* key */, 
									0 /* refresh temporal */, 
									type );

	/* Softkey configuration */
	GS_DrawOffScreen();

	ClearSoftkeyInstructionTextRetangle( &rect );

	SoftkeyStandardConfiguration_OkBack();

	GS_DrawOnScreen( &rect );
}

int HelpStateMenuConfigOutVolumeXxxOnKey( unsigned char key, inout_vol_type_t type )
{
#if 0
	unsigned char volmic, volspk;
#endif
	
	if( type >= NUM_OF_INOUTVOL_TYPE )
		debug_out( "outvol_type is too large 2.\n" );

	switch( key ) {
#if KEYPAD_MAP_VENDOR != VENDOR_BCO
	case VKEY_LEFT:
#endif
#if KEYPAD_MAP_VENDOR == VENDOR_WCO
	case VKEY_UP:
#endif
	case VKEY_OUTVOL_MINUS:
		RefreshAdjustOutVolumeXxxFrame( 1 /* draw volume only */, 
										VKEY_OUTVOL_MINUS, 
										0 /* refresh temporal */, 
										type );
		break;

#if KEYPAD_MAP_VENDOR != VENDOR_BCO
	case VKEY_RIGHT:
#endif
#if KEYPAD_MAP_VENDOR == VENDOR_WCO
	case VKEY_DOWN:
#endif
	case VKEY_OUTVOL_PLUS:
		RefreshAdjustOutVolumeXxxFrame( 1 /* draw volume only */, 
										VKEY_OUTVOL_PLUS, 
										0 /* refresh temporal */, 
										type );
		break;

	case VKEY_OK:
		nModeOutVolume[ type ] = nModeOutVolumeTemp;
		
#if 0
		/* Don't need to update immediately. */
		GetMicSpkVolumePairByInOutVolType( type, &volmic, &volspk );
		SetVolumeOfDAC( volmic, volspk );
#endif
		
		return 1;	/* OK. show prompt */
	}
	
	return 0;
}

/* ================================================================== */
/* Auto Dial realted functions */
/* ================================================================== */
unsigned long ReadAutoDialTimeInMillisecond( void )
{
	unsigned char time;

	time = FlashReadOneByte( MY_FIELD_OFFSET( ui_falsh_layout_t, mode.nAutoDialTime ) );
	
	if( time == 0 )
		;
	else if( time < MIN_AUTO_DIAL_TIME || time > MAX_AUTO_DIAL_TIME )
		time = DEF_AUTO_DIAL_TIME;
		
	return ( unsigned long )time * 1000;	/* convert sec. to ms. */
}

/* ================================================================== */
/* Auto Answer realted functions */
/* ================================================================== */
unsigned long ReadAutoAnswerTimeInMillisecond( void )
{
	unsigned char time;

	time = FlashReadOneByte( MY_FIELD_OFFSET( ui_falsh_layout_t, mode.nAutoAnswerTime ) );
	
	if( time == 0 )
		;
	else if( time < MIN_AUTO_ANSWER_TIME || time > MAX_AUTO_ANSWER_TIME )
		time = DEF_AUTO_ANSWER_TIME;
		
	return ( unsigned long )time * 1000;	/* convert sec. to ms. */
}

/* ================================================================== */
/* Off-hook Alarm Time realted functions */
/* ================================================================== */
unsigned long ReadOffHookAlarmTimeInMillisecond( void )
{
	unsigned char time;

	time = FlashReadOneByte( MY_FIELD_OFFSET( ui_falsh_layout_t, mode.nOffHookAlarmTime ) );
	
	if( time == 0 )
		;
	else if( time < MIN_OFF_HOOK_ALARM_TIME || time > MAX_OFF_HOOK_ALARM_TIME )
		time = DEF_OFF_HOOK_ALARM_TIME;
		
	return ( unsigned long )time * 1000;	/* convert sec. to ms. */
}
