#include <stdio.h>
#include "ui_config.h"
#include "ioctl_misc.h"
#include "ioctl_kernel.h"
#include "voip_manager.h"
#include "mm_ring.h"
#include "ui_flags.h"

#define _DEV_TEST		/* check tone_id_t */

#if 1	/* use continual DTMF */
#define DSPCODEC_TONE_0				DSPCODEC_TONE_0_CONT
#define DSPCODEC_TONE_1				DSPCODEC_TONE_1_CONT
#define DSPCODEC_TONE_2				DSPCODEC_TONE_2_CONT
#define DSPCODEC_TONE_3				DSPCODEC_TONE_3_CONT
#define DSPCODEC_TONE_4				DSPCODEC_TONE_4_CONT
#define DSPCODEC_TONE_5				DSPCODEC_TONE_5_CONT
#define DSPCODEC_TONE_6				DSPCODEC_TONE_6_CONT
#define DSPCODEC_TONE_7				DSPCODEC_TONE_7_CONT
#define DSPCODEC_TONE_8				DSPCODEC_TONE_8_CONT
#define DSPCODEC_TONE_9				DSPCODEC_TONE_9_CONT
#define DSPCODEC_TONE_STARSIGN		DSPCODEC_TONE_STARSIGN_CONT
#define DSPCODEC_TONE_HASHSIGN		DSPCODEC_TONE_HASHSIGN_CONT
#endif

#ifdef _DEV_TEST
#define _M_TONE( x )	{ TONE_ID_ ##x, DSPCODEC_TONE_ ##x }
#else
#define _M_TONE( x )	DSPCODEC_TONE_ ##x
#endif

//#define DSPCODEC_TONE_IN_RING	DSPCODEC_TONE_RINGING	// TODO: to be modify 
#define DSPCODEC_TONE_IN_RING	DSPCODEC_TONE_RING

struct {
#ifdef _DEV_TEST
	tone_id_t idTone;
#endif
	DSPCODEC_TONE dspcodecTone;
} idTone2DspcodecTone[] = {
	_M_TONE( 0 ),
	_M_TONE( 1 ),
	_M_TONE( 2 ),
	_M_TONE( 3 ),
	_M_TONE( 4 ),
	_M_TONE( 5 ),
	_M_TONE( 6 ),
	_M_TONE( 7 ),
	_M_TONE( 8 ),
	_M_TONE( 9 ),
	_M_TONE( STARSIGN ),
	_M_TONE( HASHSIGN ),
	_M_TONE( DIAL ),
	_M_TONE( RING ),
	_M_TONE( IN_RING ),
	_M_TONE( BUSY ),
};

#undef _M_TONE

CT_ASSERT( NUM_OF_TONE_ID == ( sizeof( idTone2DspcodecTone ) / sizeof( idTone2DspcodecTone[ 0 ] ) ) );

int SetPlayTone( tone_id_t idTone, int bEnable, int bInvolvePCM )
{
	uint32 sid = ( fLineFlags[ 1 ].b.active ? 1 : IP_SID );
	
	if( idTone >= NUM_OF_TONE_ID ) {
		debug_out( "Tone ID is too large.\n" );
		return -1;
	}

#ifdef _DEV_TEST
	if( idTone2DspcodecTone[ idTone ].idTone != idTone )
		debug_out( "Tone ID is not match!!\n" );
#endif

	if( bInvolvePCM )
		rtk_eanblePCM( IP_CHID, ( bEnable ? 1 : 0 ) );

	return rtk_SetPlayTone( IP_CHID, sid, 
						idTone2DspcodecTone[ idTone ].dspcodecTone, 
						( bEnable ? 0x81 : 0x80 ), 	/* bit 0x80 indicates a IP phone tone */
						DSPCODEC_TONEDIRECTION_LOCAL );
}

int EnablePCMtoRunDSP( unsigned int bEnable )
{
	return rtk_eanblePCM( IP_CHID, bEnable );
}
