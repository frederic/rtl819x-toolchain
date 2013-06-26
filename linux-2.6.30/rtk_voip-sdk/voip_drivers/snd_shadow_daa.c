#include <linux/kernel.h>
#include "voip_init.h"
#include "voip_debug.h"
#include "con_register.h"

#define TOTAL_SND_SHADOW_DAA	( CONFIG_RTK_VOIP_DSP_DEVICE_NR * 	\
								CONFIG_RTK_VOIP_DAA_CH_NR_PER_DSP )

#if TOTAL_SND_SHADOW_DAA > 0

static voip_snd_t snd_shadow_daa[ TOTAL_SND_SHADOW_DAA ];

// --------------------------------------------------------
// channel mapping architecture 
// --------------------------------------------------------

static int __init voip_snd_init_shadow_daa( void )
{
	int i;
	
	for( i = 0; i < TOTAL_SND_SHADOW_DAA; i ++ ) {
		snd_shadow_daa[ i ].sch = i;
		snd_shadow_daa[ i ].name = "daa(s)";
		snd_shadow_daa[ i ].snd_type = SND_TYPE_DAA;
		snd_shadow_daa[ i ].bus_type_sup = BUS_TYPE_PCM;
		snd_shadow_daa[ i ].TS1 = 0;
		snd_shadow_daa[ i ].TS2 = 0;
		snd_shadow_daa[ i ].band_mode_sup = BAND_MODE_8K;
		snd_shadow_daa[ i ].fxs_ops = NULL;
		snd_shadow_daa[ i ].priv = NULL;
	}
	
	register_voip_snd( snd_shadow_daa, TOTAL_SND_SHADOW_DAA );
	
	return 0;
}

voip_initcall_snd( voip_snd_init_shadow_daa );

#endif // TOTAL_SND_SHADOW_DAA > 0 

