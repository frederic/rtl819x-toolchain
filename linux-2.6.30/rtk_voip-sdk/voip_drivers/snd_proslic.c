#include <linux/kernel.h>


// --------------------------------------------------------
// channel mapping architecture 
// --------------------------------------------------------

#include "voip_init.h"
#include "con_register.h"

#define PROSLIC_NUM		3

static voip_snd_t snd_proslic[ PROSLIC_NUM ];
static snd_ops_t snd_proslic_ops = {
};

static int __init voip_snd_proslic_init( void )
{
	int i;
	
	for( i = 0; i < PROSLIC_NUM; i ++ ) {
		snd_proslic[ i ].sch = i;
		snd_proslic[ i ].name = "proslic";
		snd_proslic[ i ].snd_type = SND_TYPE_FXS;
		snd_proslic[ i ].bus_type_sup = BUS_TYPE_PCM;
		snd_proslic[ i ].TS1 = i * 2;
		snd_proslic[ i ].TS2 = 0;
		snd_proslic[ i ].band_mode_sup = BAND_MODE_8K;
		snd_proslic[ i ].snd_ops = &snd_proslic_ops;
	}
	
	// hack 
	snd_proslic[ 2 ].snd_type = SND_TYPE_DAA;
	
	register_voip_snd( &snd_proslic[ 0 ], PROSLIC_NUM );
	
	return 0;
}

//voip_initcall_snd( voip_snd_proslic_init );

