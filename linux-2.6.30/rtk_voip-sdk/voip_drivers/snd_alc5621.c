#include "rtk_voip.h"
#include "voip_init.h"
#include "con_register.h"

static int enable_alc5621( voip_snd_t *this, int enable )
{
	extern void Stereo_dac_volume_control_MUTE(unsigned short int DACMUTE);
	
	Stereo_dac_volume_control_MUTE( ( enable ? 0 : 7 ) );
	
	return 0;
}


// --------------------------------------------------------
// channel mapping architecture 
// --------------------------------------------------------


static const snd_ops_ac_t snd_alc5621_ops = {
	// common operation 
	.enable = enable_alc5621,
	
	
};

#define ALC5621_NUM		1

static voip_snd_t snd_alc5621[ 1 ];//ALC5621_NUM ];

static int __init voip_snd_alc5621_init( void )
{
#ifdef CONFIG_RTK_VOIP_DRIVERS_IIS
	const int law = 0; 
#else
	extern int law;
#endif
	extern void ALC5621_init(int law);
	int i;
	
	// bring up ALC 5621 
	extern void init_i2c_gpio(void);

	init_i2c_gpio();
	ALC5621_init( law );
	
	// register snd 
	for( i = 0; i < ALC5621_NUM; i ++ ) {
		snd_alc5621[ i ].sch = i;
		snd_alc5621[ i ].name = "alc5621";
		snd_alc5621[ i ].snd_type = SND_TYPE_AC;
#ifdef CONFIG_RTK_VOIP_DRIVERS_IIS	// iis
		snd_alc5621[ i ].bus_type_sup = BUS_TYPE_IIS;
		snd_alc5621[ i ].TS1 = 0;
		snd_alc5621[ i ].TS2 = 0;
  #ifdef CONFIG_RTK_VOIP_DRIVERS_CODEC_ALC5621_WIDEBAND
		snd_alc5621[ i ].band_mode_sup = BAND_MODE_16K;
  #else
		snd_alc5621[ i ].band_mode_sup = BAND_MODE_8K;
  #endif
#else	// pcm 
		snd_alc5621[ i ].bus_type_sup = BUS_TYPE_PCM;
		snd_alc5621[ i ].TS1 = i * 2;
		snd_alc5621[ i ].TS2 = 0;
		snd_alc5621[ i ].band_mode_sup = BAND_MODE_8K;
#endif
		snd_alc5621[ i ].snd_ops = ( const snd_ops_t * )&snd_alc5621_ops;
	}
	
	register_voip_snd( &snd_alc5621[ 0 ], ALC5621_NUM );
	
	return 0;
}

voip_initcall_snd( voip_snd_alc5621_init );

