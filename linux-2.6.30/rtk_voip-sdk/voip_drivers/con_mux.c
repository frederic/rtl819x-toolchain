#include <linux/errno.h>
#include "rtk_voip.h"
#include "voip_types.h"
#include "con_register.h"
#include "con_mux.h"

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
#include "con_ipc_help_host.h"
#endif

// retrieve con_ptr, only *mux* can use this!! 
extern voip_con_t *get_voip_con_ptr( uint32 cch );

uint32 dch_from_cch( uint32 cch )
{
	voip_con_t * const p_con = get_voip_con_ptr( cch );
	
	return ( p_con ->dsp_ptr ? p_con ->dsp_ptr ->dch : -1 );
}

#if 0
void bus_fifo_set_tx_mute_cch( uint32 cch, int enable )
{
	voip_con_t * const p_con = get_voip_con_ptr( cch );
	
	p_con ->con_ops ->bus_fifo_set_tx_mute( p_con, enable );
}

void bus_fifo_set_rx_mute_cch( uint32 cch, int enable )
{
	voip_con_t * const p_con = get_voip_con_ptr( cch );
	
	p_con ->con_ops ->bus_fifo_set_rx_mute( p_con, enable );
}

void bus_fifo_clean_tx_cch( uint32 cch )
{
	voip_con_t * const p_con = get_voip_con_ptr( cch );
	
	p_con ->con_ops ->bus_fifo_clean_tx( p_con );
}

void bus_fifo_clean_rx_cch( uint32 cch )
{
	voip_con_t * const p_con = get_voip_con_ptr( cch );
	
	p_con ->con_ops ->bus_fifo_clean_rx( p_con );
}
#endif

int con_enable_cch( uint32 cch, int enable )
{
	voip_con_t * const p_con = get_voip_con_ptr( cch );
	
	return p_con ->con_ops ->enable( p_con, enable );
}

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
void SLIC_CPC_Gen_cch( uint32 cch, unsigned int time_in_ms_of_cpc_signal )
{
	extern void SLIC_CPC_Gen_con( const voip_con_t *p_con, unsigned int time_in_ms_of_cpc_signal );
	voip_con_t * const p_con = get_voip_con_ptr( cch );
	
	return SLIC_CPC_Gen_con( p_con, time_in_ms_of_cpc_signal );
}
#endif

snd_type_t get_snd_type_con( voip_con_t *p_con )
{
	voip_snd_t * const p_snd = p_con ->snd_ptr;
	
	if( !p_snd )
		return SND_TYPE_NONE;
	
	return p_snd ->snd_type;	
}

snd_type_t get_snd_type_cch( uint32 cch )
{
	voip_con_t * const p_con = get_voip_con_ptr( cch );
	
	if( cch >= CON_CH_NUM )
		return SND_TYPE_NONE;
	
	return get_snd_type_con( p_con );
}

// ---------------------------------------------------------
// pluse dail 
// ---------------------------------------------------------
#ifdef PULSE_DIAL_GEN
void DAA_PulseGenProcess_msec(void)
{
	extern void DAA_PulseGenProcess_msec_con( const voip_con_t * const p_con );
	uint32 cch;
	
	for( cch = 0; cch < CON_CH_NUM; cch ++ ) {
		DAA_PulseGenProcess_msec_con( get_voip_con_ptr( cch ) );
	}
}

void DAA_PulseGenKill_cch(unsigned int cch)
{
	extern void DAA_PulseGenKill_con( voip_con_t * p_con );
	
	voip_con_t * const p_con = get_voip_con_ptr( cch );
	
	DAA_PulseGenKill_con( p_con );	
}

int pulse_dial_in_dch(uint32 dch, char input)
{
	extern int pulse_dial_in_cch(uint32 ch_id, char input);
	const uint32 cch = dch_from_cch( dch );
	
	return pulse_dial_in_cch( cch, input );
}
#endif /* PULSE_DIAL_GEN */

// ---------------------------------------------------------
// IPC 
// ---------------------------------------------------------

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST

int snd_locate_host_cch( uint32 cch )
{
	voip_con_t *p_con = get_voip_con_ptr( cch );
	
	if( p_con == NULL )
		return 0;
	
	if( p_con ->snd_ptr )
		return 1;
	
	return 0;
}

int API_get_DSP_info( int cmd, int host_cch, uint32 *dsp_cpuid, uint32 *dsp_cch )
{
	// get dsp cpuid 
	voip_con_t *p_con;
	
	// get con object 
	if( ( p_con = get_voip_con_ptr( host_cch ) ) == NULL )
		goto label_error;
	
	// check ipc 
	if( !p_con ->ipc.priv )
		goto label_error;
	
	// store 
	if( dsp_cpuid )
		*dsp_cpuid = p_con ->ipc.dsp_cpuid;
	
	if( dsp_cch )
		*dsp_cch = p_con ->ipc.dsp_cch;
	
	return 0;
	
label_error:
	printk( "cmd=%d host_cch=%d get dsp_cpuid/dsp_cch error\n", cmd, host_cch );
	
	return -1;
}

int API_get_DSP_ID( int cmd, int host_cch )
{
	int dsp_cpuid;
	
	if( API_get_DSP_info( cmd, host_cch, &dsp_cpuid, NULL ) == 0 )
		return dsp_cpuid;
	
	return -1;
}

int API_get_DSP_CH( int cmd, int host_cch )
{
	int dsp_cch;
	
	if( API_get_DSP_info( cmd, host_cch, NULL, &dsp_cch ) == 0 )
		return dsp_cch;
	
	return -1;
}

int API_get_Host_CH( uint32 dsp_cpuid, int dsp_cch )
{
	uint32 host_cch;
	
	host_cch = ipc_get_host_cch_from_invert_table( dsp_cpuid, dsp_cch );
	
	if( host_cch >= CON_CH_NUM )
		return -1;
	
	return host_cch;
}

int API_get_DSP_NUM( void )
{
	return CONFIG_RTK_VOIP_DSP_DEVICE_NR;
}

#endif // CONFIG_RTK_VOIP_IPC_ARCH

// ---------------------------------------------------------
// IOC - LED/Relay 
// ---------------------------------------------------------
int Set_LED_Display_cch( uint32 cch, uint32 led_id, 
						ioc_mode_t ioc_mode, ioc_state_t ioc_state )
{
	extern int Set_LED_Display_ioc( voip_ioc_t *p_ioc, 
						ioc_mode_t ioc_mode, ioc_state_t ioc_state );
	voip_con_t * const p_con = get_voip_con_ptr( cch );
	voip_ioc_t * p_ioc;
	
	if( !p_con )
		return -ENXIO;
	
	if( led_id == 0 && ( p_ioc = p_con ->ioc_led0_ptr ) )
		return Set_LED_Display_ioc( p_ioc, ioc_mode, ioc_state );

	if( led_id == 1 && ( p_ioc = p_con ->ioc_led1_ptr ) )
		return Set_LED_Display_ioc( p_ioc, ioc_mode, ioc_state );
	
	return -ENXIO;
}

int Set_SLIC_Relay_cch( uint32 cch, ioc_state_t ioc_state )
{
	extern int Set_SLIC_Relay_ioc( voip_ioc_t *p_ioc, ioc_state_t ioc_state );
	voip_con_t * const p_con = get_voip_con_ptr( cch );
	voip_ioc_t * p_ioc;
	
	if( !p_con )
		return -ENXIO;
	
	p_ioc = p_con ->ioc_relay_ptr;
	
	if( !p_ioc )
		return -ENXIO;
	
	return Set_SLIC_Relay_ioc( p_ioc, ioc_state );
}

// ---------------------------------------------------------
// Bus 
// ---------------------------------------------------------
int Set_Bus_Data_Format_cch( uint32 cch, uint32 format )
{
	voip_con_t * const p_con = get_voip_con_ptr( cch );
	voip_bus_t * p_bus;
	
	if( !p_con )
		return -ENXIO;
	
	// bus 1
	p_bus = p_con ->bus_ptr;
	
	if( !p_bus )
		return -ENXIO;
	
	p_bus ->bus_ops ->set_format( p_bus, format );
	
	// bus 2
	p_bus = p_con ->bus2_ptr;
	
	if( p_bus )
		p_bus ->bus_ops ->set_format( p_bus, format );
	
	return 0;
}

int Set_PCM_Timeslot_cch( uint32 cch, uint32 TS1, uint32 TS2 )
{
	voip_con_t * const p_con = get_voip_con_ptr( cch );
	voip_bus_t * p_bus, *p_bus2;
	
	if( !p_con )
		return -ENXIO;
	
	// bus 1
	p_bus = p_con ->bus_ptr;
	
	if( !p_bus )
		return -ENXIO;
	
	// bus 2
	p_bus2 = p_con ->bus2_ptr;
	
	if( p_bus && p_bus2 ) {	// 8k+ mode 
		p_bus ->bus_ops ->set_timeslot( p_bus, TS1, 0 );
		p_bus2 ->bus_ops ->set_timeslot( p_bus2, TS2, 0 );
	} else {
		p_bus ->bus_ops ->set_timeslot( p_bus, TS1, TS2 );
	}
	
	return 0;
}

