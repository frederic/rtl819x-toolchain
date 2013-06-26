#include "rtk_voip.h"
#include "voip_init.h"
#include "con_register.h"

#include "codec_descriptor.h"

#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
SampleRate_t VoipChannelSampleRate[ DSP_RTK_CH_NUM ] = {[0 ... DSP_RTK_CH_NUM-1] = SAMPLE_NARROW_BAND};
#endif

static int enable_dsp_rtk( voip_dsp_t *this, int enable )
{	
	if( enable ) {
		VoipChannelSampleRate[ this ->dch ] = 
			( enable == 2 ? SAMPLE_WIDE_BAND : SAMPLE_NARROW_BAND );
	}

	return 0;
}

int __init voip_dsp_rtk_init( void )
{
	// initialization is realted to binding 
	
	extern void dsp_rtk_isr_init_var( void );
	extern void dsp_rtk_handler_init_var( void );
	
	// init dsp 
	dsp_rtk_isr_init_var();
	dsp_rtk_handler_init_var();
	
	return 0;
}

voip_initcall( voip_dsp_rtk_init );

// --------------------------------------------------------
// channel mapping register  
// --------------------------------------------------------
extern void isr_bus_tx_start( voip_dsp_t *this );
extern void isr_bus_tx_process_pre( struct voip_dsp_s *this );
extern void isr_bus_tx_process( voip_dsp_t *this, uint16 *pcm_tx );
extern void isr_bus_tx_process_post( struct voip_dsp_s *this );
extern void isr_bus_rx_start( voip_dsp_t *this );
extern void isr_bus_rx_process( voip_dsp_t *this, uint16 *pcm_rx, const uint16 *lec_ref );

// misc. function  
extern int stop_type1_fsk_cid_gen_when_phone_offhook( voip_dsp_t *p_dsp );

// turn on/off dsp function in ISR 
extern void DisableDspInPcmIsr( voip_dsp_t *this );
extern void RestoreDspInPcmIsr( voip_dsp_t *this );

static voip_dsp_t dsp_rtk[ DSP_RTK_CH_NUM ];
static dsp_ops_t dsp_rtk_ops = {
	// common operation 
	.enable = enable_dsp_rtk,
	
	// isr handler 
	.isr_bus_tx_start = isr_bus_tx_start,
	.isr_bus_tx_process_pre = isr_bus_tx_process_pre,
	.isr_bus_tx_process = isr_bus_tx_process,
	.isr_bus_tx_process_post = isr_bus_tx_process_post,
	.isr_bus_rx_start = isr_bus_rx_start,
	.isr_bus_rx_process = isr_bus_rx_process,
		
	// misc. function  
	.stop_type1_fsk_cid_gen_when_phone_offhook = stop_type1_fsk_cid_gen_when_phone_offhook,

	// turn on/off dsp function in ISR 
	.disable_dsp_in_ISR = DisableDspInPcmIsr,
	.restore_dsp_in_ISR = RestoreDspInPcmIsr,
};

voip_dsp_t *get_voip_rtk_dsp( uint32 dch )
{
	// only for dsp_rtk_ module 
	return &dsp_rtk[ dch ];
}

int __init voip_dsp_rtk_register( void )
{
	int i;
	
	// register dsp 
	for( i = 0; i < DSP_RTK_CH_NUM; i ++ ) {
		dsp_rtk[ i ].dch = i;
		dsp_rtk[ i ].name = "rtk_dsp";
		dsp_rtk[ i ].dsp_type = DSP_TYPE_REALTEK;
#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
		dsp_rtk[ i ].band_mode_sup = BAND_MODE_8K | BAND_MODE_16K;
#else
		dsp_rtk[ i ].band_mode_sup = BAND_MODE_8K;
#endif
		dsp_rtk[ i ].dsp_ops = &dsp_rtk_ops;
	}
	
	register_voip_dsp( &dsp_rtk[ 0 ], DSP_RTK_CH_NUM );
	
	return 0;
}

voip_initcall_dsp( voip_dsp_rtk_register );

