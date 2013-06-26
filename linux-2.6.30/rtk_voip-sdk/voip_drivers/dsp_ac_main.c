#include "rtk_voip.h"
#include "voip_init.h"
#include "con_register.h"

static int enable_ac_rtk( voip_dsp_t *this, int enable )
{
	extern void dsp_ac_chanEnable( uint32 dch, char enable );
	
	dsp_ac_chanEnable( this ->dch, ( enable ? 1 : 0 ) );
	
	return 0;
}


// --------------------------------------------------------
// channel mapping register  
// --------------------------------------------------------
extern void isr_ac_bus_tx_start( voip_dsp_t *this );
extern void isr_ac_bus_tx_process_pre( struct voip_dsp_s *this );
extern void isr_ac_bus_tx_process( voip_dsp_t *this, uint16 *pcm_tx );
extern void isr_ac_bus_tx_process_post( struct voip_dsp_s *this );
extern void isr_ac_bus_rx_start( voip_dsp_t *this );
extern void isr_ac_bus_rx_process( voip_dsp_t *this, uint16 *pcm_rx, const uint16 *lec_ref );

// misc. function  

// turn on/off dsp function in ISR (dummy)
static void disable_dsp_in_ISR( struct voip_dsp_s *this ) {}
static void restore_dsp_in_ISR( struct voip_dsp_s *this ) {}

static voip_dsp_t dsp_ac[ DSP_AC_CH_NUM ];
static dsp_ops_t dsp_ac_ops = {
	// common operation 
	.enable = enable_ac_rtk,
	
	// isr handler 
	.isr_bus_tx_start = isr_ac_bus_tx_start,
	.isr_bus_tx_process_pre = isr_ac_bus_tx_process_pre,
	.isr_bus_tx_process = isr_ac_bus_tx_process,
	.isr_bus_tx_process_post = isr_ac_bus_tx_process_post,
	.isr_bus_rx_start = isr_ac_bus_rx_start,
	.isr_bus_rx_process = isr_ac_bus_rx_process,
		
	// misc. function  
	.stop_type1_fsk_cid_gen_when_phone_offhook = NULL,
	
	// turn on/off dsp function in ISR 
	.disable_dsp_in_ISR = disable_dsp_in_ISR,
	.restore_dsp_in_ISR = restore_dsp_in_ISR,
};

voip_dsp_t *get_voip_ac_dsp( uint32 dch )
{
	// only for dsp_ac_ module 
	return &dsp_ac[ dch ];
}

int __init voip_dsp_ac_init( void )
{
	int i;
	
	for( i = 0; i < DSP_AC_CH_NUM; i ++ ) {
		dsp_ac[ i ].dch = i;
		dsp_ac[ i ].name = "ac_dsp";
		dsp_ac[ i ].dsp_type = DSP_TYPE_AUDIOCODES;
		dsp_ac[ i ].band_mode_sup = BAND_MODE_8K;
		dsp_ac[ i ].dsp_ops = &dsp_ac_ops;
	}
	
	register_voip_dsp( &dsp_ac[ 0 ], DSP_AC_CH_NUM );
	
	return 0;
}

voip_initcall_dsp( voip_dsp_ac_init );


