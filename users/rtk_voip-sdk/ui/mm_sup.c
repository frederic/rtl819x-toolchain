#include "ui_config.h"
#include "ui_vkey.h"
#include "ui_mode.h"
#include "mm_ring.h"
#include "mm_sup.h"
#include "ioctl_codec.h"

/* ------------------------------------------------------------------ */
/* Key Press Tone */
/* ------------------------------------------------------------------ */
/* rename tone */
#define TONE_ID_STAR		TONE_ID_STARSIGN
#define TONE_ID_POUND		TONE_ID_HASHSIGN

#define _M_KEY_TONE( x )	{ VKEY_ ##x, TONE_ID_ ##x }

static const struct {
	unsigned char key;
	tone_id_t tone;
} keyToneMap[] = {
	_M_KEY_TONE( 0 ),
	_M_KEY_TONE( 1 ),
	_M_KEY_TONE( 2 ),
	_M_KEY_TONE( 3 ),
	_M_KEY_TONE( 4 ),
	_M_KEY_TONE( 5 ),
	_M_KEY_TONE( 6 ),
	_M_KEY_TONE( 7 ),
	_M_KEY_TONE( 8 ),
	_M_KEY_TONE( 9 ),
	_M_KEY_TONE( STAR ),
	_M_KEY_TONE( POUND ),
};

#undef _M_KEY_TONE

#define NUM_OF_KEY_TONE_MAP		( sizeof( keyToneMap ) / sizeof( keyToneMap[ 0 ] ) )

void PlayKeypressTone( unsigned char key )
{
	int i;

	for( i = 0; i < NUM_OF_KEY_TONE_MAP; i ++ )
		if( keyToneMap[ i ].key == key ) {
			MM_StartTonePlaying( keyToneMap[ i ].tone, KEYPRESS_TONE_PERIOD );
			return;
		}
}

/* ------------------------------------------------------------------ */
/* Voice Path and Its Volume */
/* ------------------------------------------------------------------ */
CT_ASSERT( VOC_PATH_MIC1_SPEAKER == 0 );
CT_ASSERT( VOC_PATH_MIC2_MONO == 1 );
CT_ASSERT( VOC_PATH_SPEAKER_ONLY == 2 );
CT_ASSERT( VOC_PATH_MONO_ONLY == 3 );

void SetVoicePathAndItsVolume( vpMicSpeacker_t path )
{
	static const inout_vol_type_t OutTypeMap_Spk[ NUM_OF_VOC_PATH ] = {
		INOUTVOL_TYPE_SPEAKER,
		INOUTVOL_TYPE_RECEIVER,
		INOUTVOL_TYPE_SPEAKER,
		INOUTVOL_TYPE_RECEIVER,		
	};

	static const inout_vol_type_t OutTypeMap_Mic[ NUM_OF_VOC_PATH ] = {
		INOUTVOL_TYPE_MIC_S,
		INOUTVOL_TYPE_MIC_R,
		INOUTVOL_TYPE_MIC_S,
		INOUTVOL_TYPE_MIC_R,		
	};

	SetCodecVoicePath( path );
	
	SetVolumeOfDAC( nModeOutVolume[ OutTypeMap_Mic[ path ] ],
					nModeOutVolume[ OutTypeMap_Spk[ path ] ] );
}

