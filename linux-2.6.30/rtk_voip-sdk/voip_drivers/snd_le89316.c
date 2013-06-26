#include "rtk_voip.h"
#include "voip_init.h"

#include "con_register.h"
#include "con_defer_init.h"
#include "snd_pin_cs.h"
#include "snd_help_ipc.h"

#include "zarlink_int.h"
#include "zarlink_api.h"

#include "gpio.h"
#include "spi.h"

static voip_snd_t			snd_zarlink_le89316[2*CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316_NR];
static rtl_spi_dev_t		spi_dev_le89316[CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316_NR];
/*
** Realtek memory space that API-2 requires 
** for the Device/Line contexts and objects
*/
static RTKDevObj	  	  	DevObj_le89316[CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316_NR]; //
static RTKLineObj	  	  	LineObj_le89316[2*CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316_NR]; //1FXS 1FXO

/*
** Application memory space that API-2 requires 
** for the Device/Line contexts and objects
*/
static Vp890DeviceObjectType  VpDevObj_le89316[CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316_NR];
static Vp890LineObjectType    VpLineObj_le89316[2*CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316_NR];

static VpDevCtxType           VpDevCtx_le89316[CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316_NR];

static VpLineCtxType          VpLineCtx_le89316[2*CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316_NR];

#if 1
static const uint32 * const pin_cs_le89316 = 
		&snd_pin_cs[ CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316_PIN_CS - 1 ];
#else
static uint32 pin_cs_le89316[] = {
	//PIN_CS1,
	PIN_CS2,
	PIN_CS3,
	PIN_CS4,
};

#define NUM_OF_PIN_CS_LE89316		( sizeof( pin_cs_le89316 ) / sizeof( pin_cs_le89316[ 0 ] ) )

CT_ASSERT( NUM_OF_PIN_CS_LE89316 >= CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316_NR );
#endif

static int SLIC_init_le89316(int pcm_mode, int initonly)
{
	int i;
	RTKDevObj *pDev;
	int PcnType;
   	int rtn;
   	//int ch;

	PRINT_MSG("================================================\n");
	PRINT_MSG("Zarlink API-II Lite Version %d.%d.%d\n", 
				VP_API_VERSION_MAJOR_NUM, 
				VP_API_VERSION_MINOR_NUM, 
				VP_API_VERSION_MINI_NUM);
			
	if( initonly )
		goto label_do_init_only;

	/* ******************* */
	/* Init Realtek obj    */
	/* ******************* */
		
	// setup GPIO for SPI 
	for( i = 0; i < CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316_NR; i ++ ) {
		#ifdef CONFIG_RTK_VOIP_8676_SHARED_SPI
		PRINT_MSG( "le89316[%d] CS=%08X\n", i, CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316_PIN_CS - 1 + i);
		init_spi_pins( &spi_dev_le89316[ i ], CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316_PIN_CS - 1 + i, PIN_CLK, PIN_DO, PIN_DI);
		#else
		PRINT_MSG( "le89316[%d] CS=%08X\n", i, pin_cs_le89316[ i ] );
		init_spi_pins( &spi_dev_le89316[ i ], pin_cs_le89316[ i ], PIN_CLK, PIN_DO, PIN_DI);
		#endif
	}
	
	// Create Dev & Line Object 
	for( i = 0; i < CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316_NR; i ++ ) {
		Ve890CreateDevObj( 
			DEV_FXSFXO, i * 2,
			&DevObj_le89316[ i ], &LineObj_le89316[ i * 2 ],
			&spi_dev_le89316[ i ], VP_DEV_890_SERIES, 
			&VpDevObj_le89316[ i ], &VpDevCtx_le89316[ i ], 
			&VpLineObj_le89316[ i * 2 ], &VpLineCtx_le89316[ i * 2 ]);

		// FXS
		Ve890CreateLineObj( rtkGetNewChID(),   0, LINE_FXS, DevObj_le89316[ i ].pLine[0], 
							pcm_mode, snd_zarlink_le89316[ i * 2 ].TS1 );

		// FXO
		Ve890CreateLineObj( rtkGetNewChID(), 1, LINE_FXO, DevObj_le89316[ i ].pLine[1], 
							pcm_mode, snd_zarlink_le89316[ i * 2 + 1 ].TS1 );
	}
label_do_init_only:

	/* ******************* */
	/* Init Zarlink API-II */
	/* ******************* */
	
	for( i = 0; i < CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316_NR; i ++ ) 
	{
		/* deviceId imply spi device id */
		pDev = &DevObj_le89316[ i ];
		PcnType = Ve890GetRev( pDev );
		
		if ( PcnType == FAILED ) {
			PRINT_R("Error (%d:x) %s Read version fail\n", i, __FUNCTION__);
			return FAILED;
		}
		
		/* Initialize API-2 device settings */
		rtn = zarlinkInitDevice( pDev );
		if ( rtn == FAILED ) {
			PRINT_R("Error (%d:x) %s\n", i, __FUNCTION__);
			return FAILED;
		}
	}
	
	return 0;
}

static void SLIC_reset_le89316( voip_snd_t *this, int codec_law )
{
	extern int law;	// FIXME: chmap 
	
	SLIC_init_le89316( law, 1 /* init only */ );
}

static void SLIC_show_ID_le89316( voip_snd_t *this )
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	RTKDevObj * const pDev = pLine ->pDev;
	
	Ve890GetRev( pDev );
}

// --------------------------------------------------------
// channel mapping architecture 
// --------------------------------------------------------
__attribute__ ((section(".snd_desc_data")))
static snd_ops_fxs_t snd_le89316_fxs_ops;

static void __init fill_le89316_register_info( 
	voip_snd_t snd_zarlink_le89xxx[],
	int n_fxs, int m_daa, uint16 TS_base,
	RTKLineObj LineObj_le89xxx[] 
	)
{
	extern const snd_ops_daa_t snd_zarlink_daa_ops;
	// once call this function:
	//  - one control interface 
	//  - n fxs
	//  - m daa 
	int sch;
	int daa = 0;
	
	for( sch = 0; sch < n_fxs + m_daa; sch ++ ) {
	
		if( sch == n_fxs )
			daa = 1;

		snd_zarlink_le89xxx[ sch ].sch = sch;
		snd_zarlink_le89xxx[ sch ].name = "le89316";
		snd_zarlink_le89xxx[ sch ].snd_type = SND_TYPE_FXS;
		snd_zarlink_le89xxx[ sch ].bus_type_sup = BUS_TYPE_PCM;
		snd_zarlink_le89xxx[ sch ].TS1 = TS_base + sch * 2;
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316_WIDEBAND
		snd_zarlink_le89xxx[ sch ].TS2 = ( daa || TS_base + sch * 2 >= 16 ? 0 : TS_base + ( sch + 8 ) * 2 );
		snd_zarlink_le89xxx[ sch ].band_mode_sup = ( daa ? BAND_MODE_8K : BAND_MODE_8K | BAND_MODE_16K );
#else
		snd_zarlink_le89xxx[ sch ].TS2 = 0;
		snd_zarlink_le89xxx[ sch ].band_mode_sup = BAND_MODE_8K;
#endif
		snd_zarlink_le89xxx[ sch ].snd_flags.b.attenuateTx_6dB = ( daa ? 1 : 0 );
		snd_zarlink_le89xxx[ sch ].snd_ops = ( const snd_ops_t * )&snd_le89316_fxs_ops;
		snd_zarlink_le89xxx[ sch ].priv = &LineObj_le89xxx[ sch ];
		
		// DAA port  
		if( daa ) {
			snd_zarlink_le89xxx[ sch ].snd_type = SND_TYPE_DAA;
#if 1
			snd_zarlink_le89xxx[ sch ].snd_ops = ( const snd_ops_t * )&snd_zarlink_daa_ops;
#else
			printk( "No snd_ops for DAA!!\n" );
#endif
		}		
	}
}

static int __init voip_snd_zarlink_init_le89316( void )
{
	extern int law;	// FIXME: chmap 
	extern const snd_ops_fxs_t snd_zarlink_fxs_ops;
	int i;//, sch, daa;
	int TS_base, TS_cur;
#ifdef CONFIG_RTK_VOIP_DEFER_SNDDEV_INIT
	static defer_init_t le89316_defer;
#endif

	// le89316 override proslic base ops 
	snd_le89316_fxs_ops = snd_zarlink_fxs_ops;

	snd_le89316_fxs_ops.SLIC_reset = SLIC_reset_le89316;
	snd_le89316_fxs_ops.SLIC_show_ID = SLIC_show_ID_le89316;
	
	// get TS base 
	TS_base = get_snd_free_timeslot();
	if( TS_base < 0 ) 
		TS_base = 0;
	
	TS_cur = TS_base;
	
	// common port definition 
	for( i = 0; i < CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316_NR; i ++ ) {
		
		fill_le89316_register_info( &snd_zarlink_le89316[ i * 2 ], 
				1 /* fxs */, 1 /* daa */, (uint16) TS_cur,
				&LineObj_le89316[ i * 2 ] );

#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116_OWNER_ALL_DSP
		if( fill_ipc_dsp_cpuid( &snd_zarlink_le89316[ i * 2 ], 
				1 /* fxs */, 1 /* daa */, 
				i, 
				SND_PER_DSPCPU( CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316_NR,
								CONFIG_RTK_VOIP_DSP_DEVICE_NR ) ) )
		{
			TS_cur = TS_base;
		} else
#endif
		{
			TS_cur += 4;
		}
		
		register_voip_snd( &snd_zarlink_le89316[ i * 2 ], 2 );
	}
	
	// SLIC init use ops 
#ifdef CONFIG_RTK_VOIP_DEFER_SNDDEV_INIT
	le89316_defer.fn_defer_func = ( fn_defer_func_t )SLIC_init_le89316;
	le89316_defer.p0 = law;
	le89316_defer.p1 = 0;
	
	add_defer_initialization( &le89316_defer );
#else
	SLIC_init_le89316( law, 0 /* allocate */ );
#endif

	return 0;
}

voip_initcall_snd( voip_snd_zarlink_init_le89316 );

