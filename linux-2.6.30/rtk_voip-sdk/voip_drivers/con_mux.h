#ifndef __CON_MUX_H__
#define __CON_MUX_H__

#include "voip_types.h"
#include "con_register.h"

// provide global number to access 
extern int dsp_ch_num;
extern int dsp_ss_num;
extern int con_ch_num;

// bus fifo
//extern void bus_fifo_set_tx_mute_cch( uint32 cch, int enable );
//extern void bus_fifo_set_rx_mute_cch( uint32 cch, int enable );
//extern void bus_fifo_clean_tx_cch( uint32 cch );
//extern void bus_fifo_clean_rx_cch( uint32 cch );

// enable a control channel, so all binding channel will be enabled too. 
extern int con_enable_cch( uint32 cch, int enable );

// enable CPC gen 
extern void SLIC_CPC_Gen_cch( uint32 cch, unsigned int time_in_ms_of_cpc_signal );

// access snd type 
extern snd_type_t get_snd_type_cch( uint32 cch );
extern snd_type_t get_snd_type_con( voip_con_t *p_con );

// pluse dail 
extern void DAA_PulseGenProcess_msec(void);
extern void DAA_PulseGenKill_cch(unsigned int cch);

// IPC arch is host 
extern int snd_locate_host_cch( uint32 cch );
extern int API_get_DSP_info( int cmd, int host_cch, uint32 *dsp_cpuid, uint32 *dsp_cch );
extern int API_get_DSP_ID( int cmd, int host_cch );
extern int API_get_DSP_CH( int cmd, int host_cch );
extern int API_get_Host_CH( uint32 dsp_cpuid, int dsp_cch );
extern int API_get_DSP_NUM( void );

// IOC - LED/Relay 
extern int Set_LED_Display_cch( uint32 cch, uint32 led_id, 
						ioc_mode_t ioc_mode, ioc_state_t ioc_state );
extern int Set_SLIC_Relay_cch( uint32 cch, ioc_state_t ioc_state );

// bus
extern int Set_Bus_Data_Format_cch( uint32 cch, uint32 format );
extern int Set_PCM_Timeslot_cch( uint32 cch, uint32 TS1, uint32 TS2 );

#endif /* __CON_MUX_H__ */

