#include "ui_config.h"
#include "ui_host.h"
#include "ui_flags.h"
#include "ioctl_timer.h"
#include "ioctl_softrtc.h"
#include "ioctl_misc.h"
#include "ioctl_led.h"
#include "mm_ring.h"
#include "mm_animate.h"
#include "mm_sup.h"
#include "flash_rw_api.h"

/* ****************************************************************************
 * Timer assistant
 * ****************************************************************************/
#define STATE_TIMER_PERIOD_SLEEP	( 30 * 1000 )	/* 30 seconds */
#define STATE_TIMER_PERIOD_STANDBY	( 5 * 1000 )	/* 5 seconds */
#define STATE_TIMER_PERIOD_STANDBYT	( 1 * 1000 )	/* 1 second */
#define STATE_TIMER_PERIOD_CURSOR	100				/* 0.1 second */
#define STATE_TIMER_PERIOD_MENU		500				/* 0.5 seconds */
#define STATE_TIMER_PERIOD_PROMPT	100				/* 0.1 second */
#define STATE_TIMER_PERIOD_CONFIRM	500				/* 0.5 second */
#define STATE_TIMER_PERIOD_CALL		1000			/* 1 second */

static const unsigned long nPredefinedStateTimerPeriod[] = {
	STATE_TIMER_PERIOD_SLEEP,
	STATE_TIMER_PERIOD_STANDBY,
	STATE_TIMER_PERIOD_STANDBYT,
	STATE_TIMER_PERIOD_CURSOR,
	STATE_TIMER_PERIOD_MENU,
	STATE_TIMER_PERIOD_PROMPT,
	STATE_TIMER_PERIOD_CONFIRM,
	STATE_TIMER_PERIOD_CALL,
};

CT_ASSERT( ( sizeof( nPredefinedStateTimerPeriod ) / sizeof( nPredefinedStateTimerPeriod[ 0 ] ) ) == NUM_OF_STATE_TIMER_IDX );

static unsigned long nNowTimerPeriod = 0;

void ChangeTimerPeriodForApplication( void )
{
	extern const ui_state_context_t uiStateContext[];

	unsigned long nMilliSecond;
	
	if( uiStateContext[ uiState ].nStateTimerIdx < NUM_OF_STATE_TIMER_IDX )
		nMilliSecond = nPredefinedStateTimerPeriod[ uiStateContext[ uiState ].nStateTimerIdx ];
	else
		nMilliSecond = STATE_TIMER_PERIOD_STANDBY;
	
	/* tone is playing */
	if( fHostFlags.b.tonePlaying ) {
		if( nMilliSecond > MM_TONE_PLAY_PERIOD_UNIT )	/* 100ms */
			nMilliSecond = MM_TONE_PLAY_PERIOD_UNIT;
	}

	/* animate is playing */
	if( fHostFlags.b.animatePlaying ) {
		if( nMilliSecond > MM_ANIMATE_PLAY_PERIOD_UNIT )	/* 500ms */
			nMilliSecond = MM_ANIMATE_PLAY_PERIOD_UNIT;
	}

	/* LED blinking */
	if( fHostFlags.b.ledBlinking ) {
		if( nMilliSecond > LED_BLINKING_PERIOD )		/* 500ms */
			nMilliSecond = LED_BLINKING_PERIOD;
	}
	
	if( nNowTimerPeriod != nMilliSecond ) 
		ChangeTimerPeriod( ( nNowTimerPeriod = nMilliSecond ) );
}

uptime_t CheckIfTimeoutInMillisecond( uptime_t *pPrevUptime, 
												unsigned long nTimeoutMS )
{
	uptime_t now_uptime, delta_uptime;
	
	now_uptime = GetUptimeInMillisecond();
	delta_uptime = now_uptime - *pPrevUptime;
	
	if( delta_uptime >= nTimeoutMS ) {
		*pPrevUptime = now_uptime;
		return 0;
	}
		
	return ( nTimeoutMS - delta_uptime );
}

/* ****************************************************************************
 * BCD conversion
 * ****************************************************************************/

static const unsigned char chBCD[] = { 
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '*', '#', 'P'
};

#define NUM_OF_EXTEND_BCD		( sizeof( chBCD ) / sizeof( chBCD[ 0 ] ) )

static unsigned char CharFromBcd( unsigned char bcd )
{
	if( bcd < NUM_OF_EXTEND_BCD )
		return chBCD[ bcd ];

	return '\x0';
}

static unsigned char BcdFromChar( unsigned char ch )
{
	unsigned char i;

	for( i = 0; i < NUM_OF_EXTEND_BCD; i ++ )
		if( chBCD[ i ] == ch )
			return i;

	return '\xF';
}

int GetTextStringFromBcd( unsigned char *pszText, int maxLen, const unsigned char *pBCD )
{
	/* maxlen exclusive null terminator */
	unsigned char ch;
	int len = 0;

	while( 1 ) {
		if( ( ch = CharFromBcd( ( *pBCD & 0xF0 ) >> 4 ) ) == '\x0' )
			break;
		*pszText ++ = ch;
		if( ++ len >= maxLen )
			break;

		if( ( ch = CharFromBcd( *pBCD & 0x0F ) ) == '\x0' )
			break;
		*pszText ++ = ch;
		if( ++ len >= maxLen )
			break;

		pBCD ++;
	}

	*pszText ++ = '\x0';

	return len;
}

int GetBcdStringFromText( unsigned char *pBCD, int maxBytes, const unsigned char *pszText )
{
	/* maxBytes include nibble of '\xF'*/
	unsigned char bcd =0xFF, nibble;
	int len = 0;

	if( pszText[ 0 ] == '\x0' ) {
		pBCD[ 0 ] = 0xFF;
		return 0;
	}

	while( *pszText != '\x0' ) {

		if( ( nibble = BcdFromChar( *pszText ) ) == '\xF' )
			break;
		bcd = nibble << 4;
		pszText ++;

		if( ( nibble = BcdFromChar( *pszText ) ) == '\xF' || len == maxBytes - 1 ) {
			bcd |= 0x0F;
			break;
		}
		bcd |= nibble;
		pszText ++;
	
		*pBCD ++ = bcd;
		bcd = 0xFF;
		len ++;
	}

	*pBCD ++ = bcd;
	len ++;

	return len;
}

/* ****************************************************************************
 * Flash
 * ****************************************************************************/
int CheckFlashVersionFeatureSize( void )
{
	unsigned long version;
	unsigned long features;
	unsigned long size;
	unsigned long signature;
	
	/* check version */
	FlashReadData( MY_FIELD_OFFSET( ui_falsh_layout_t, version ), 
				   &version, sizeof( version ) ); 
	
	if( version != FLASH_VERSION )
		return 0;
	
	/* check features */
	FlashReadData( MY_FIELD_OFFSET( ui_falsh_layout_t, features ), 
				   &features, sizeof( features ) ); 
	
	if( features != FLASH_FEATURES )
		return 0;
	
	/* check size */
	FlashReadData( MY_FIELD_OFFSET( ui_falsh_layout_t, size ), 
				   &size, sizeof( size ) ); 
	
	if( size != MAX_ADDR_OF_FLASH )
		return 0;
	
	/* check signature */
	FlashReadData( MY_FIELD_OFFSET( ui_falsh_layout_t, signature ), 
				   &signature, sizeof( signature ) ); 
	
	if( signature != FLASH_SIGNATURE )
		return 0;

	return 1;
}

void WriteFlashVersionFeatureSize( void )
{
	unsigned long version;
	unsigned long features;
	unsigned long size;
	unsigned long signature;

	/* write version */
	version = FLASH_VERSION;
	FlashWriteData( MY_FIELD_OFFSET( ui_falsh_layout_t, version ), 
				    &version, sizeof( version ) ); 

	/* write features */
	features = FLASH_FEATURES;
	FlashWriteData( MY_FIELD_OFFSET( ui_falsh_layout_t, features ), 
				    &features, sizeof( features ) ); 
	
	/* write size */
	size = MAX_ADDR_OF_FLASH;
	FlashWriteData( MY_FIELD_OFFSET( ui_falsh_layout_t, size ), 
				    &size, sizeof( size ) ); 
	
	/* write signature */
	signature = FLASH_SIGNATURE;
	FlashWriteData( MY_FIELD_OFFSET( ui_falsh_layout_t, signature ), 
				    &signature, sizeof( signature ) ); 

	/* FlashValidateWriting() by caller */
}

/* ****************************************************************************
 * Hardware handfree 
 * ****************************************************************************/
int EnableHandfreeFunctionalityAndFlag( int bEnable )
{
	if( bEnable ) {
		/* Set voice path and volume */
		SetVoicePathAndItsVolume( VOC_PATH_MIC1_SPEAKER );	/* voice path may change by other */

		if( fHostFlags.b.handfree )
			return 0;

		/* flag and Enable PCM */
		fHostFlags.b.handfree = 1;
		EnablePCMtoRunDSP( 1 /* enable */ );
#ifdef LED_BIT_HANDFREE
		TurnOnLEDThroughGPIO( LED_BIT_HANDFREE );
#endif
	} else {
		/* Set voice path and volume */
		SetVoicePathAndItsVolume( VOC_PATH_MIC2_MONO );		/* voice path may change by other */

		if( !fHostFlags.b.handfree )
			return 0;

		/* flag and Enable PCM */
		fHostFlags.b.handfree = 0;
		EnablePCMtoRunDSP( 0 /* disable */ );
#ifdef LED_BIT_HANDFREE
		TurnOffLEDThroughGPIO( LED_BIT_HANDFREE );
#endif
	}

	return 1;
}
