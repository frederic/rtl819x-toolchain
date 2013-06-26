#include <linux/kernel.h>
#include "voip_init.h"
#include "voip_debug.h"
#include "con_register.h"

#define TOTAL_SND_SHADOW_SLIC	( CONFIG_RTK_VOIP_DSP_DEVICE_NR * 	\
								CONFIG_RTK_VOIP_SLIC_CH_NR_PER_DSP )

#if TOTAL_SND_SHADOW_SLIC > 0

static voip_snd_t snd_shadow_slic[ TOTAL_SND_SHADOW_SLIC ];

// --------------------------------------------------------
// channel mapping architecture 
// --------------------------------------------------------

static int __init voip_snd_init_shadow_slic( void )
{
	int i;
	
	for( i = 0; i < TOTAL_SND_SHADOW_SLIC; i ++ ) {
		snd_shadow_slic[ i ].sch = i;
		snd_shadow_slic[ i ].name = "slic(s)";
		snd_shadow_slic[ i ].snd_type = SND_TYPE_FXS;
		snd_shadow_slic[ i ].bus_type_sup = BUS_TYPE_PCM;
		snd_shadow_slic[ i ].TS1 = 0;
		snd_shadow_slic[ i ].TS2 = 0;
		snd_shadow_slic[ i ].band_mode_sup = BAND_MODE_8K;
		snd_shadow_slic[ i ].fxs_ops = NULL;
		snd_shadow_slic[ i ].priv = NULL;
	}
	
	register_voip_snd( snd_shadow_slic, TOTAL_SND_SHADOW_SLIC );
	
	return 0;
}

voip_initcall_snd( voip_snd_init_shadow_slic );

#endif // TOTAL_SND_SHADOW_SLIC > 0 

