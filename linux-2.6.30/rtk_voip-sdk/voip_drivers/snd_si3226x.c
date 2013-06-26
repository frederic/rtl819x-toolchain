#include <linux/interrupt.h>
#include "rtk_voip.h"
#include "voip_init.h"

#include "con_register.h"
#include "snd_pin_cs.h"
#include "snd_proslic_type.h"

// We consider diasy chain case only. 

#define CHAN_PER_DEVICE 		2	// one 3226x contains 2 FXS

#define TOTAL_NUM_OF_3226x_CH	( CONFIG_RTK_VOIP_SLIC_SI3226x_NR * 2 )
#define NUMBER_OF_PROSLIC_3226x	( TOTAL_NUM_OF_3226x_CH / CHAN_PER_DEVICE )
#if 1
#define NUMBER_OF_HWINTF_3226x	1
#define NUMBER_OF_CHAN_3226x		TOTAL_NUM_OF_3226x_CH
#else
// CS mode may use this 
#define NUMBER_OF_HWINTF_3226x	( TOTAL_NUM_OF_3226x_CH / CHAN_PER_DEVICE )
#endif

static ctrl_S gSpiGciObj_3226x[ NUMBER_OF_HWINTF_3226x ];
static ProslicContainer_t gProslicContainer_3226x[ TOTAL_NUM_OF_3226x_CH ];

static voip_snd_t snd_proslic_3226x[ TOTAL_NUM_OF_3226x_CH ];

#if 1
static const uint32 * const pin_cs = 
		&snd_pin_cs[ CONFIG_RTK_VOIP_SLIC_SI3226x_PIN_CS - 1 ];
#else
static const uint32 pin_cs[] = {
	PIN_CS1, 
};

CT_ASSERT( ( sizeof( pin_cs ) / sizeof( pin_cs[ 0 ] ) ) >= NUMBER_OF_HWINTF_3226x );
#endif

static const proslic_args_t proslic_args_3226x;

static int SLIC_init_si3226x(int pcm_mode, int initonly)
{
	int i, sidx;
	int j;
	int i_size, i_device, i_channel;
	//rtl_spi_dev_t *spi_devs[ NUMBER_OF_HWINTF_3226x ];
	
	printk ("\n<<<<<<<<<<< Si3226x Driver Version %s >>>>>>>>>>\n", ProSLIC_Version());
	
	//if( !initonly ) {
	//	// init spi first, because reset pin will affect all SLIC 
	//	printk( "Preparing spi channel for SLIC...\n" );
	//	
	//	for( i = 0; i < NUMBER_OF_HWINTF_3226x; i ++ ) {
	//		spi_devs[ i ] = &gSpiGciObj_3226x[ i ].spi_dev;
	//	}
	//
	//	init_spi_channels( NUMBER_OF_HWINTF_3226x, spi_devs, pin_cs, PIN_RESET1, PIN_CLK, PIN_DO, PIN_DI);
	//}
	
	for( i = 0, sidx = 0; i < NUMBER_OF_HWINTF_3226x; i ++ ) {
		
		printk( "--------------------------------------\n" );
		printk( "SLIC HW intf %d starting at %d CS=%08X\n", i, sidx, pin_cs[ i ] );
		
		init_spi_pins( &gSpiGciObj_3226x[ i ].spi_dev, pin_cs[ i ], PIN_CLK, PIN_DO, PIN_DI);
		
		i_size = NUMBER_OF_CHAN_3226x;
		i_device = NUMBER_OF_PROSLIC_3226x;
		i_channel = NUMBER_OF_CHAN_3226x;
		
		if( initonly )
			goto label_do_init_only;

		// create objs
		for( j = 0; j < i_size; j ++ ) {
			gProslicContainer_3226x[ sidx + j ].spiGciObj = &gSpiGciObj_3226x[ i ];			
		}
		
		proslic_alloc_objs( &gProslicContainer_3226x[ sidx ], 
			i_size, i_device, i_channel, SI3226X_TYPE );
				
		// init proslic 
label_do_init_only:
		
		proslic_init_user_objs( &gProslicContainer_3226x[ sidx ], i_size, 
						SI3226X_TYPE );
		
		proslic_init( &snd_proslic_3226x[ sidx ], &gProslicContainer_3226x[ sidx ], 
						i_size, 
						&proslic_args_3226x,
						pcm_mode );
		
		sidx += i_size;
	}
	
	return 0;
}

static void SLIC_reset_si3226x(voip_snd_t *this, int codec_law)
{
	// This function will cause system reset, if watch dog is enable!
	// Because calibration need mdelay(1900).
	unsigned long flags;
	save_flags(flags); cli();
	*(volatile unsigned long *)(0xB800311c) &=  0xFFFFFF;	// Disable watch dog
	*(volatile unsigned long *)(0xB800311c) |=  0xA5000000;
	restore_flags(flags);
	
	SLIC_init_si3226x(codec_law, 1 /* don't allocate */);
	
	save_flags(flags); cli();
	*(volatile unsigned long *)(0xB800311c) &=  0xFFFFFF;	// Enable watch dog
	*(volatile unsigned long *)(0xB800311c) |=  1 << 23;
	restore_flags(flags);
}

static void SLIC_show_ID_si3226x( voip_snd_t *this )
{
	unsigned char reg_val, reg_len;
	
	reg_len = sizeof(reg_val);
	
	//reg_val = R_reg_dev(&spiGciObj ->spi_dev, i, 0);
	this ->fxs_ops ->SLIC_read_reg( this, 0, &reg_len, &reg_val );
	
	if (((reg_val&0x38)>>3) == 0)
		printk("Si3226x\n");
	else if (((reg_val&0x38)>>3) == 1)
		printk("Si3227\n");
	else
		PRINT_R("Unknow SLIC\n");
}

// --------------------------------------------------------
// channel mapping architecture 
// --------------------------------------------------------

static snd_ops_fxs_t snd_proslic_3226x_ops;

static const proslic_args_t proslic_args_3226x = {
	.ring_setup_preset = 0,	// 20Hz, 48V
};

static int __init voip_snd_si3226x_init( void )
{
	extern int law;	// FIXME: chmap 
	extern const snd_ops_fxs_t snd_proslic_fxs_ops;
	int i, sch;
	int TS_base;
	
	// 3226x override proslic base ops 
	snd_proslic_3226x_ops = snd_proslic_fxs_ops;
	
	snd_proslic_3226x_ops.SLIC_reset = SLIC_reset_si3226x;
	snd_proslic_3226x_ops.SLIC_show_ID = SLIC_show_ID_si3226x;
	
	// get TS base 
	TS_base = get_snd_free_timeslot();	
	if( TS_base < 0 )
		TS_base = 0;
	
	// common port definition 
	for( i = 0, sch = 0; i < TOTAL_NUM_OF_3226x_CH; i ++, sch ++ ) {

		snd_proslic_3226x[ i ].sch = sch;
		snd_proslic_3226x[ i ].name = "si3226x";
		snd_proslic_3226x[ i ].snd_type = SND_TYPE_FXS;
		snd_proslic_3226x[ i ].bus_type_sup = BUS_TYPE_PCM;
		snd_proslic_3226x[ i ].TS1 = TS_base + i * 2;
#ifdef CONFIG_RTK_VOIP_SLIC_SI3226x_WIDEBAND
		snd_proslic_3226x[ i ].TS2 = ( TS_base + i * 2 >= 16 ? 0 : TS_base + ( i + 8 ) * 2 );
		snd_proslic_3226x[ i ].band_mode_sup = BAND_MODE_8K | BAND_MODE_16K;
#else
		snd_proslic_3226x[ i ].TS2 = 0;
		snd_proslic_3226x[ i ].band_mode_sup = BAND_MODE_8K;
#endif
		snd_proslic_3226x[ i ].snd_ops = ( const snd_ops_t * )&snd_proslic_3226x_ops;
		snd_proslic_3226x[ i ].priv = &gProslicContainer_3226x[ i ];
	}
	
	register_voip_snd( &snd_proslic_3226x[ 0 ], TOTAL_NUM_OF_3226x_CH );
	
	// SLIC init use ops 
	SLIC_init_si3226x( law, 0 /* allocate */ );
	
	return 0;
}

voip_initcall_snd( voip_snd_si3226x_init );

