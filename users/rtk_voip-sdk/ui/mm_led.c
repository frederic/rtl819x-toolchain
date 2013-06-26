#include "ui_config.h"
#include "ioctl_led.h"
#include "ui_host.h"
#include "ui_flags.h"
#include "mm_led.h"

static unsigned long ledBlinkingMask = 0;
static unsigned short swBlinking = 0;
static uptime_t nLedBlinkingTime;

void MM_StartLedBlinking( unsigned long mask )
{
	fHostFlags.b.ledBlinking = 1;

	ledBlinkingMask = mask;
	swBlinking = 1;

	MM_LedBlinkingTimerEvent( 1 /* immediately */ );
}

void MM_StopLedBlinking( void )
{
	fHostFlags.b.ledBlinking = 0;

	ledBlinkingMask = 0;
}

void MM_LedBlinkingTimerEvent( int bImmediately )
{
	if( !fHostFlags.b.ledBlinking )
		return;

	if( !ledBlinkingMask )
		return;

	if( bImmediately ) {
		nLedBlinkingTime = GetUptimeInMillisecond();
		goto label_blinking_led_immediately;
	}

	if( CheckIfTimeoutInMillisecond( &nLedBlinkingTime, LED_BLINKING_PERIOD /* 500 ms*/  ) == 0 ) {

label_blinking_led_immediately:
		if( swBlinking ) {
			swBlinking = 0;
			TurnOnLEDThroughGPIO( ledBlinkingMask );
		} else {
			swBlinking = 1;
			TurnOffLEDThroughGPIO( ledBlinkingMask );
		}
	}
}
