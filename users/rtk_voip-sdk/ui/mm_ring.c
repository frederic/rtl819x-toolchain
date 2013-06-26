#include "ui_config.h"
#include "ui_host.h"
#include "ui_flags.h"
#include "mm_ring.h"
#include "mm_sup.h"
#include "ioctl_softrtc.h"
#include "ioctl_misc.h"
#include "ioctl_codec.h"

typedef union fMMRing_s {
	struct {
		unsigned long	forcePlaying:1;		/* 1: enable PCM to play tone */
		unsigned long	speakerOut:1;		/* 1: reset voice path, after stopping */
	} b;
	unsigned long all;
} fMMRing_t;		CT_ASSERT( MY_FIELD_SIZE( fMMRing_t, all ) == sizeof( fMMRing_t ) );

static struct {
	tone_id_t idTone;
	unsigned int nPeriod;	/* in unit of 100ms */
	uptime_t uptimeStart;
	fMMRing_t flags;		/* flags */
} mm_vars;

void MM_Initialize( void )
{
	fHostFlags.b.tonePlaying = 0;
	
	mm_vars.flags.all = 0;
}

void MM_Terminate( void )
{
	MM_StopTonePlaying();
}

void MM_TimerEvent( void )
{
	if( !fHostFlags.b.tonePlaying )
		return;
	
	if( mm_vars.nPeriod == 0 )	/* repeat */
		return;

	if( CheckIfTimeoutInMillisecond( &mm_vars.uptimeStart, 
										mm_vars.nPeriod ) == 0 )
	{
		MM_StopTonePlaying();
	}
}

static int DoStopTonePlaying( void )
{
	SetPlayTone( mm_vars.idTone, 0 /* Stop */, ( mm_vars.flags.b.forcePlaying ? 1 : 0 ) );
	
	if( mm_vars.flags.b.speakerOut ) {
		if( fHostFlags.b.handfree )
			SetVoicePathAndItsVolume( VOC_PATH_MIC1_SPEAKER );
		else
			SetVoicePathAndItsVolume( VOC_PATH_MIC2_MONO );
	}

	mm_vars.flags.b.forcePlaying = 0;
	mm_vars.flags.b.speakerOut = 0;
	fHostFlags.b.tonePlaying = 0;
	
	return 0;
}

int MM_StartTonePlayingEx( tone_id_t idTone, unsigned int period /* ms */, 
						   int bForce, int bSpeakerOut )
{
	if( fHostFlags.b.tonePlaying )
		DoStopTonePlaying();

	mm_vars.idTone = idTone;
	mm_vars.nPeriod = period;
	mm_vars.uptimeStart = GetUptimeInMillisecond();
	
	if( bSpeakerOut ) {
		mm_vars.flags.b.speakerOut = 1;
		SetVoicePathAndItsVolume( VOC_PATH_SPEAKER_ONLY );
	} else
		mm_vars.flags.b.speakerOut = 0;
	
	SetPlayTone( idTone, 1 /* start */, bForce );
	
	if( bForce )
		mm_vars.flags.b.forcePlaying = 1;
	
	fHostFlags.b.tonePlaying = 1;
	
	/* Playing tone need faster timer event. */
	ChangeTimerPeriodForApplication();
	
	return 1;	
}

int MM_StartTonePlaying( tone_id_t idTone, unsigned int period )
{
	return MM_StartTonePlayingEx( idTone, period, 
								  0 /* force */, 
								  0 /* speaker out */ );
}

int MM_StopTonePlaying( void )
{
	if( !fHostFlags.b.tonePlaying )
		return 0;

	DoStopTonePlaying();
	
	/* Recall timer period */
	ChangeTimerPeriodForApplication();

	return 1;
}

