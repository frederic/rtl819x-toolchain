#ifdef CONFIG_DEFAULTS_KERNEL_2_6
#include <linux/interrupt.h>
#endif
#include "rtk_voip.h"
#include "voip_init.h"

#include "con_register.h"
#include "con_defer_init.h"
#include "snd_pin_cs.h"
#include "snd_proslic_type.h"

//static ctrl_S gSpiGciObj_3217x[ NUMBER_OF_HWINTF_3217X ];
//static ProslicContainer_t gProslicContainer_3217x[ TOTAL_NUM_OF_CH_3217X ];
//static voip_snd_t snd_proslic_3217x[ TOTAL_NUM_OF_CH_3217X ];

#ifdef CONFIG_RTK_VOIP_SLIC_SI32176		// Daisy Chain
static ctrl_S gSpiGciObj_32176[ 1 ];
static ProslicContainer_t gProslicContainer_32176[ CONFIG_RTK_VOIP_SLIC_SI32176_NR ];
static voip_snd_t snd_proslic_32176[ CONFIG_RTK_VOIP_SLIC_SI32176_NR ];
#endif

#ifdef CONFIG_RTK_VOIP_SLIC_SI32176_CS	// Chip Select 
static ctrl_S gSpiGciObj_32176_CS[ CONFIG_RTK_VOIP_SLIC_SI32176_CS_NR ];
static ProslicContainer_t gProslicContainer_32176_CS[ CONFIG_RTK_VOIP_SLIC_SI32176_CS_NR ];
static voip_snd_t snd_proslic_32176_CS[ CONFIG_RTK_VOIP_SLIC_SI32176_CS_NR ];
#endif

#ifdef CONFIG_RTK_VOIP_SLIC_SI32178		// Chip Select 
static ctrl_S gSpiGciObj_32178[ CONFIG_RTK_VOIP_SLIC_SI32178_NR ];
static ProslicContainer_t gProslicContainer_32178[ 2 * CONFIG_RTK_VOIP_SLIC_SI32178_NR ];
static voip_snd_t snd_proslic_32178[ 2 * CONFIG_RTK_VOIP_SLIC_SI32178_NR ];
static daa_det_t daa_det_32178[ CONFIG_RTK_VOIP_SLIC_SI32178_NR ];
#endif

#ifdef CONFIG_RTK_VOIP_SLIC_SI32176_SI32178	// Daisy Chain 
static ctrl_S gSpiGciObj_32176_32178[ 1 ];
static ProslicContainer_t gProslicContainer_32176_32178[ 2 + CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_NR ];
static voip_snd_t snd_proslic_32176_32178[ 2 + CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_NR ];
static daa_det_t daa_det_32176_32178[ 1 ];
#endif

static int SLIC_init_si3217x(int pcm_mode, int initonly);

#define ts2count( ts )	( 1 + ( ts ) * 8 )		// Time slot to silab's count 

#ifdef CONFIG_RTK_VOIP_SLIC_SI32176		// Daisy Chain
static const uint32 * const pin_cs_si32176 = 
		&snd_pin_cs[ CONFIG_RTK_VOIP_SLIC_SI32176_PIN_CS - 1 ];
#endif
#ifdef CONFIG_RTK_VOIP_SLIC_SI32176_CS	// Chip Select 
static const uint32 * const pin_cs_si32176_CS = 
		&snd_pin_cs[ CONFIG_RTK_VOIP_SLIC_SI32176_CS_PIN_CS - 1 ];
#endif
#ifdef CONFIG_RTK_VOIP_SLIC_SI32178		// Chip Select 
static const uint32 * const pin_cs_si32178 = 
		&snd_pin_cs[ CONFIG_RTK_VOIP_SLIC_SI32178_PIN_CS - 1 ];
#endif
#ifdef CONFIG_RTK_VOIP_SLIC_SI32176_SI32178	// Daisy Chain 
static const uint32 * const pin_cs_si32176_32178 = 
		&snd_pin_cs[ CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_PIN_CS - 1 ];
#endif

#if 0
static const uint32 pin_cs[] = {
	PIN_CS1, 
#ifdef PIN_CS2
	PIN_CS2, 
#ifdef PIN_CS4
	PIN_CS3, 
	PIN_CS4, 
#ifdef PIN_CS8
	PIN_CS5, 
	PIN_CS6, 
	PIN_CS7, 
	PIN_CS8, 
#endif
#endif
#endif
};

CT_ASSERT( ( sizeof( pin_cs ) / sizeof( pin_cs[ 0 ] ) ) >= ( CONFIG_RTK_VOIP_SLIC_SI32178_NR + !!CONFIG_RTK_VOIP_SLIC_SI32176_NR ) );
#endif

static const proslic_args_t proslic_args_3217x;

static inline void _SLIC_init_si3217x(
		int i_size, int i_device, int i_channel, 
		ProslicContainer_t gProslicContainer_3217x[],
		voip_snd_t snd_proslic_3217x[],
		ctrl_S *gSpiGciObj_3217x, 
		uint32 pin_cs_3217x,
		int pcm_mode, 
		int initonly )
{
	// once call this function:
	//  - one control interface 
	//  - i_size total channels 
	//  - i_device chip number 
	//  - i_channel fxs channels
	int j;
	
	init_spi_pins( &gSpiGciObj_3217x ->spi_dev, pin_cs_3217x, PIN_CLK, PIN_DO, PIN_DI);
	
	if( initonly )
		goto label_do_init_only;
		
	// create objs
	for( j = 0; j < i_size; j ++ ) {
		gProslicContainer_3217x[ j ].spiGciObj = gSpiGciObj_3217x;
	}

	proslic_alloc_objs( &gProslicContainer_3217x[ 0 ], 
		i_size, i_device, i_channel, SI3217X_TYPE );
	//si3217x_alloc_objs( &gProslicContainer[ sidx ], 
	//	i_size, i_device, i_channel );

	// init proslic 
label_do_init_only:

	proslic_init_user_objs( &gProslicContainer_3217x[ 0 ], i_size, 
					SI3217X_TYPE );

	//si3217x_init( &snd_proslic[ sidx ], &gProslicContainer[ sidx ], 
	//				i_size, pcm_mode );
	proslic_init( &snd_proslic_3217x[ 0 ], &gProslicContainer_3217x[ 0 ], 
					i_size, 
					&proslic_args_3217x,
					pcm_mode );
}

static int SLIC_init_si3217x(int pcm_mode, int initonly)
{
	//int ch;
#if defined( CONFIG_RTK_VOIP_SLIC_SI32178) || defined(CONFIG_RTK_VOIP_SLIC_SI32176_CS)
	int i;
#endif
	//int i, j, sidx;
	//int i_size, i_device, i_channel;
	//rtl_spi_dev_t *spi_devs[ NUMBER_OF_HWINTF_3217X ];
	
	printk ("\n<<<<<<<<<<< Si3217x Driver Version %s >>>>>>>>>>\n", ProSLIC_Version());
	
	//if( !initonly ) {
	//	// init spi first, because reset pin will affect all SLIC 
	//	printk( "Preparing spi channel for SLIC...\n" );
	//	
	//	for( i = 0; i < NUMBER_OF_HWINTF_3217X; i ++ ) {
	//		spi_devs[ i ] = &gSpiGciObj_3217x[ i ].spi_dev;
	//	}
	//
	//	init_spi_channels( NUMBER_OF_HWINTF_3217X, spi_devs, pin_cs, PIN_RESET1, PIN_CLK, PIN_DO, PIN_DI);
	//}

#if 1
	
#ifdef CONFIG_RTK_VOIP_SLIC_SI32176		// Daisy Chain
	printk( "------------------------------\n" );
	printk( "SLIC 32176 HW intf %d CS=%X\n", 0, pin_cs_si32176[ 0 ] );
		
	_SLIC_init_si3217x( 
			CONFIG_RTK_VOIP_SLIC_SI32176_NR, 
			CONFIG_RTK_VOIP_SLIC_SI32176_NR,
			CONFIG_RTK_VOIP_SLIC_SI32176_NR,
			gProslicContainer_32176,
			snd_proslic_32176, 
			&gSpiGciObj_32176[ 0 ], 
			pin_cs_si32176[ 0 ],
			pcm_mode,
			initonly );
#endif

#ifdef CONFIG_RTK_VOIP_SLIC_SI32176_CS	// Chip Select 
	for( i = 0; i < CONFIG_RTK_VOIP_SLIC_SI32176_CS_NR; i ++ ) {
		printk( "------------------------------\n" );
		printk( "SLIC 32176 HW intf %d CS=%X\n", 0, pin_cs_si32176_CS[ i ] );
		
		_SLIC_init_si3217x( 
				1, 1, 1,
				&gProslicContainer_32176_CS[ i ],
				&snd_proslic_32176_CS[ i ],
				&gSpiGciObj_32176_CS[ i ], 
				pin_cs_si32176_CS[ i ],
				pcm_mode, 
				initonly );
	}
#endif

#ifdef CONFIG_RTK_VOIP_SLIC_SI32178		// Chip Select 
	for( i = 0; i < CONFIG_RTK_VOIP_SLIC_SI32178_NR; i ++ ) {
		
		printk( "------------------------------\n" );
		printk( "SLIC 31278 HW intf %d CS=%X\n", i, pin_cs_si32178[ i ] );
		
		_SLIC_init_si3217x( 
				2, 1, 1,
				&gProslicContainer_32178[ i * 2 ],
				&snd_proslic_32178[ i * 2 ],
				&gSpiGciObj_32178[ i ], 
				pin_cs_si32178[ i ],	// consider 32176 
				pcm_mode, 
				initonly );
		
	}
#endif

#ifdef CONFIG_RTK_VOIP_SLIC_SI32176_SI32178	// Daisy Chain 
	printk( "------------------------------\n" );
	printk( "SLIC 32176/78 HW intf %d CS=%X\n", 0, pin_cs_si32176_32178[ 0 ] );
	
	_SLIC_init_si3217x(
			2 + CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_NR,
			1 + CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_NR,
			1 + CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_NR,
			gProslicContainer_32176_32178,
			snd_proslic_32176_32178, 
			&gSpiGciObj_32176_32178[ 0 ],
			pin_cs_si32176_32178[ 0 ],
			pcm_mode,
			initonly );
#endif
		
#elif 0
	for( i = 0, sidx = 0; i < NUMBER_OF_HWINTF_3217X; i ++ ) {
		
		printk( "------------------------------\n" );
		printk( "SLIC HW intf %d starting at %d\n", i, sidx );
		
		init_spi_pins( &gSpiGciObj_3217x[ i ].spi_dev, pin_cs[ i ], PIN_CLK, PIN_DO, PIN_DI);
		
  #if defined( CONFIG_RTK_VOIP_MULTIPLE_SI32178 )
		i_size = CHAN_PER_DEVICE_3217X + 1;	// number of FXS/FXO port 
		i_device = 1;					// number of chip 
		i_channel = CHAN_PER_DEVICE_3217X;	// number of FXS port 
  #elif defined( CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_CS )
  		// This assume CS of 32176 at preceding index 
		i_size = ( i == NUMBER_OF_HWINTF_3217X - 1 ? 
					CHAN_PER_DEVICE_3217X + 1 :
					CHAN_PER_DEVICE_3217X );
		i_device = 1;
		i_channel = CHAN_PER_DEVICE_3217X;
  #else
		i_size = TOTAL_NUM_OF_CH_3217X;
		i_device = NUMBER_OF_PROSLIC_3217X;
		i_channel = NUMBER_OF_CHAN_3217X;
  #endif
		
		if( initonly )
			goto label_do_init_only;
			
		// create objs
		//init_spi_channel( &gSpiGciObj[ i ].spi_dev, pin_cs[ i ], PIN_RESET1, PIN_CLK, PIN_DO, PIN_DI);
		
		for( j = 0; j < i_size; j ++ ) {
			gProslicContainer_3217x[ sidx + j ].spiGciObj = &gSpiGciObj_3217x[ i ];
		}

		proslic_alloc_objs( &gProslicContainer_3217x[ sidx ], 
			i_size, i_device, i_channel, SI3217X_TYPE );
		//si3217x_alloc_objs( &gProslicContainer[ sidx ], 
		//	i_size, i_device, i_channel );

		// init proslic 
label_do_init_only:

		proslic_init_user_objs( &gProslicContainer_3217x[ sidx ], i_size, 
						SI3217X_TYPE );

		//si3217x_init( &snd_proslic[ sidx ], &gProslicContainer[ sidx ], 
		//				i_size, pcm_mode );
		proslic_init( &snd_proslic_3217x[ sidx ], &gProslicContainer_3217x[ sidx ], 
						i_size, 
						&proslic_args_3217x,
						pcm_mode );
		
		sidx += i_size;
	}
#else
	// create objs
	//gSpiGciObj[ 0 ].portID = 0;
	init_spi_channel( &gSpiGciObj_3217x[ 0 ].spi_dev, PIN_CS1, PIN_RESET1, PIN_CLK, PIN_DO, PIN_DI);
	
	for( i = 0; i < TOTAL_NUM_OF_CH_3217X; i ++ )
		gProslicContainer_3217x[ i ].spiGciObj = &gSpiGciObj_3217x[ 0 ];
	
	si3217x_alloc_objs( gProslicContainer_3217x, 
		TOTAL_NUM_OF_CH_3217X, NUMBER_OF_PROSLIC_3217X, TOTAL_NUM_OF_CH_3217X - NUM_OF_DAA_3217X );
	si3217x_init( snd_proslic, gProslicContainer_3217x, TOTAL_NUM_OF_CH_3217X, pcm_mode );
#endif

	printk("<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>\n\n");
	
	return 0;
}

static void SLIC_reset_si3217x(voip_snd_t *this, int codec_law)
{
	// This function will cause system reset, if watch dog is enable!
	// Because calibration need mdelay(1900).
	unsigned long flags;
	save_flags(flags); cli();
	*(volatile unsigned long *)(0xB800311c) &=  0xFFFFFF;	// Disable watch dog
	*(volatile unsigned long *)(0xB800311c) |=  0xA5000000;
	restore_flags(flags);
	
	SLIC_init_si3217x( codec_law, 1 /* don't allocate */ );
	//si3217x_init(snd_proslic, gProslicContainer, TOTAL_NUM_OF_CH_3217X, law);
	
	save_flags(flags); cli();
	*(volatile unsigned long *)(0xB800311c) &=  0xFFFFFF;	// Enable watch dog
	*(volatile unsigned long *)(0xB800311c) |=  1 << 23;
	restore_flags(flags);
}

/* This API is workable only for Si32178, chid is SLIC's chid. */
#if defined( CONFIG_RTK_VOIP_SLIC_SI32178 ) || defined( CONFIG_RTK_VOIP_SLIC_SI32176_SI32178 )
static void FXS_FXO_DTx_DRx_Loopback_si3217x(voip_snd_t *this, voip_snd_t *daa_snd, unsigned int enable)
{
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	proslicChanType * const pfxs = container ->ProObj;
	ProslicContainer_t * const daa_container = ( ProslicContainer_t * )daa_snd ->priv;
	vdaaChanType * const daas = daa_container ->daas;
	//proslicChanType *pfxs;	
	//pfxs = ports[chid].ProObj;
	const unsigned char chid = this ->sch;
	
	if( container ->ProSLICDevices != daa_container ->ProSLICDevices )
		printk( "Si3217x different devices loopback\n" );
	
	ProSLIC_SO_DTRx_Loopback(pfxs, enable);

	if (enable == 1)
	{
		//ProSLIC_PCMTimeSlotSetup(pfxs, 1+(pfxs->channel*8), 1+(pfxs->channel+DAA_CHANNEL_OFFSET)*8);
		ProSLIC_PCMTimeSlotSetup(pfxs, ts2count( this ->TS1 ), ts2count( daa_snd ->TS1 ));
		
#if 0//def CONFIG_RTK_VOIP_MULTIPLE_SI32178
		Vdaa_PCMTimeSlotSetup(daas[chid], 1+(pfxs->channel+DAA_CHANNEL_OFFSET)*8, 1+(pfxs->channel*8));
#else
		//Vdaa_PCMTimeSlotSetup(daas, 1+(pfxs->channel+DAA_CHANNEL_OFFSET)*8, 1+(pfxs->channel*8));
		Vdaa_PCMTimeSlotSetup(daas, ts2count( daa_snd ->TS1 ), ts2count( this ->TS1 ));
#endif
		//printk("fxo-%d: 0x%p, %d, %d\n", chid, daas[0], 1, 65);
		PRINT_MSG("Set SI32178 FXS/O loopback mode for FXS port%d\n", chid);
	}
	else if (enable == 0)
	{
		ProSLIC_PCMTimeSlotSetup(pfxs, ts2count( this ->TS1 ), ts2count( this ->TS1 ));
#if 0 //def CONFIG_RTK_VOIP_MULTIPLE_SI32178
		Vdaa_PCMTimeSlotSetup(daas[chid], 1+(pfxs->channel+DAA_CHANNEL_OFFSET)*8, 1+(pfxs->channel+DAA_CHANNEL_OFFSET)*8);
#else
		Vdaa_PCMTimeSlotSetup(daas, ts2count( daa_snd ->TS1 ), ts2count( daa_snd ->TS1 ));
#endif
		//printk("fxo-%d: 0x%p, %d, %d\n", chid, daas[chid], 1+(pfxs->channel+DAA_CHANNEL_OFFSET)*8, 1+(pfxs->channel+DAA_CHANNEL_OFFSET)*8);
		//Vdaa_PCMTimeSlotSetup(daas[0], 1+(pfxs->channel+DAA_CHANNEL_OFFSET)*8, 1+(pfxs->channel+DAA_CHANNEL_OFFSET)*8);
		PRINT_MSG("Disable SI32178 FXS/O loopback mode for FXS port%d\n", chid);
	}
}
#endif

static void SLIC_show_ID_si3217x( voip_snd_t *this )
{
	unsigned char reg_val, reg_len;
	
	reg_len = sizeof(reg_val);

	//reg_val = R_reg_dev(&spiGciObj ->spi_dev, i, 0);
	this ->fxs_ops ->SLIC_read_reg( this, 0, &reg_len, &reg_val );
	
	switch ((reg_val&0x38)>>3)
	{
		case 0:
			printk("Si32171 ");
			break;
		case 3:
			printk("Si32175 ");
			break;
		case 4:
			printk("Si32176 ");
			break;
		case 5:
			printk("Si32177 ");
			break;
		case 6:
			printk("Si32178 ");
			break;
		default:
			PRINT_R("Unknow SLIC ");
			break;
	}
	
	
	if ((reg_val&0x07) == 0)
		printk("Revision A\n");
	else if ((reg_val&0x07) == 1)
		printk("Revision B\n");
	else if ((reg_val&0x07) == 2)
		printk("Revision C\n");
	else
		PRINT_R("Unknow Revision\n");
}


// --------------------------------------------------------
// channel mapping architecture 
// --------------------------------------------------------

__attribute__ ((section(".snd_desc_data")))
static snd_ops_fxs_t snd_si3217x_fxs_ops;

static const proslic_args_t proslic_args_3217x = {
	.ring_setup_preset = 2,	// 20Hz, 48VRMS
};

static void __init fill_3217x_register_info( 
	voip_snd_t snd_proslic_3217x[],
	int n_fxs, int m_daa, uint16 TS_base,
	ProslicContainer_t gProslicContainer_3217x[], 
	daa_det_t *daa_det)
{
	// once call this function:
	//  - one control interface 
	//  - n fxs
	//  - m daa 
	int sch;
	int daa = 0;

#ifdef CONFIG_RTK_VOIP_DRIVERS_SI3050
	extern const snd_ops_daa_t snd_si3050_daa_ops;
#endif
	
	for( sch = 0; sch < n_fxs + m_daa; sch ++ ) {
	
		if( sch == n_fxs )
			daa = 1;

		snd_proslic_3217x[ sch ].sch = sch;
		snd_proslic_3217x[ sch ].name = "si3217x";
		snd_proslic_3217x[ sch ].snd_type = SND_TYPE_FXS;
		snd_proslic_3217x[ sch ].bus_type_sup = BUS_TYPE_PCM;
		snd_proslic_3217x[ sch ].TS1 = TS_base + sch * 2;
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3217x_WIDEBAND
		snd_proslic_3217x[ sch ].TS2 = ( daa || TS_base + sch * 2 >= 16 ? 0 : TS_base + ( sch + 8 ) * 2 );
		snd_proslic_3217x[ sch ].band_mode_sup = ( daa ? BAND_MODE_8K : BAND_MODE_8K | BAND_MODE_16K );
#else
		snd_proslic_3217x[ sch ].TS2 = 0;
		snd_proslic_3217x[ sch ].band_mode_sup = BAND_MODE_8K;
#endif
		snd_proslic_3217x[ sch ].snd_ops = ( const snd_ops_t * )&snd_si3217x_fxs_ops;
		snd_proslic_3217x[ sch ].priv = &gProslicContainer_3217x[ sch ];
		
		// DAA port  
		if( daa ) {
			gProslicContainer_3217x[ sch ].daa_det = daa_det;
			
			snd_proslic_3217x[ sch ].snd_type = SND_TYPE_DAA;
#ifdef CONFIG_RTK_VOIP_DRIVERS_SI3050
			snd_proslic_3217x[ sch ].snd_ops = ( const snd_ops_t * )&snd_si3050_daa_ops;
#else
			printk( "No snd_ops for DAA!!\n" );
#endif
		}		
	}
}

static int __init voip_snd_proslic_init_3217x( void )
{
	extern int law;	// FIXME: chmap 
	extern const snd_ops_fxs_t snd_proslic_fxs_ops;
#if defined( CONFIG_RTK_VOIP_SLIC_SI32178) || defined(CONFIG_RTK_VOIP_SLIC_SI32176_CS)
	int i;//, sch, daa;
#endif
	int TS_base;
#ifdef CONFIG_RTK_VOIP_DEFER_SNDDEV_INIT
	static defer_init_t si3217x_defer;
#endif
	
	// si3217x override proslic base ops 
	snd_si3217x_fxs_ops = snd_proslic_fxs_ops;

#if defined( CONFIG_RTK_VOIP_SLIC_SI32178 ) || defined( CONFIG_RTK_VOIP_SLIC_SI32176_SI32178 )	
	snd_si3217x_fxs_ops.FXS_FXO_DTx_DRx_Loopback = FXS_FXO_DTx_DRx_Loopback_si3217x;
#endif
	snd_si3217x_fxs_ops.SLIC_reset = SLIC_reset_si3217x;
	snd_si3217x_fxs_ops.SLIC_show_ID = SLIC_show_ID_si3217x;
	
	// get TS base 
	TS_base = get_snd_free_timeslot();
	if( TS_base < 0 )
		TS_base = 0;
	
	// common port definition 
#ifdef CONFIG_RTK_VOIP_SLIC_SI32176
	fill_3217x_register_info( snd_proslic_32176, 
			CONFIG_RTK_VOIP_SLIC_SI32176_NR, 0, TS_base,
			gProslicContainer_32176, 
			NULL );
	
	register_voip_snd( &snd_proslic_32176[ 0 ], CONFIG_RTK_VOIP_SLIC_SI32176_NR );	
	
	TS_base += CONFIG_RTK_VOIP_SLIC_SI32176_NR * 2;
#endif

#ifdef CONFIG_RTK_VOIP_SLIC_SI32176_CS
	for( i = 0; i < CONFIG_RTK_VOIP_SLIC_SI32176_CS_NR; i ++ ) {
		
		fill_3217x_register_info( &snd_proslic_32176_CS[ i ], 
				1 /* fxs */, 0 /* daa */, TS_base,
				&gProslicContainer_32176_CS[ i ], 
				NULL);
		
		register_voip_snd( &snd_proslic_32176_CS[ i ], 1 );	
		
		TS_base += 2;
	}	
#endif

#ifdef CONFIG_RTK_VOIP_SLIC_SI32178
	for( i = 0; i < CONFIG_RTK_VOIP_SLIC_SI32178_NR; i ++ ) {
		
		fill_3217x_register_info( &snd_proslic_32178[ i * 2 ], 
				1 /* fxs */, 1 /* daa */, TS_base,
				&gProslicContainer_32178[ i * 2 ],
				&daa_det_32178[ i ] );
		
		register_voip_snd( &snd_proslic_32178[ i * 2 ], 2 );	
		
		TS_base += 4;
	}
#endif

#ifdef CONFIG_RTK_VOIP_SLIC_SI32176_SI32178
	fill_3217x_register_info( snd_proslic_32176_32178, 
			CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_NR + 1, 1, TS_base,
			gProslicContainer_32176_32178,
			daa_det_32176_32178 );
	
	register_voip_snd( &snd_proslic_32176_32178[ 0 ], 2 + CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_NR );		
	
	TS_base += ( CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_NR + 2 ) * 2;
#endif

	// SLIC init use ops 
#ifdef CONFIG_RTK_VOIP_DEFER_SNDDEV_INIT
	si3217x_defer.fn_defer_func = ( fn_defer_func_t )SLIC_init_si3217x;
	si3217x_defer.p0 = law;
	si3217x_defer.p1 = 0;
	
	add_defer_initialization( &si3217x_defer );
#else
	SLIC_init_si3217x( law, 0 /* allocate */ );
#endif
	
	return 0;
}

voip_initcall_snd( voip_snd_proslic_init_3217x );

