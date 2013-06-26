#ifndef __SND_MUX_DAA_H__
#define __SND_MUX_DAA_H__

#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA
#include "virtual_daa.h"
#elif defined CONFIG_RTK_VOIP_DRIVERS_SI3050
//#include "daa.h"
#endif

#define DAA_NOT_USE	0
#define DAA_USE		1
#define DAA_USE_HOLD	2

extern int g_DAA_used[];

/*Daa_api.c function prototype*/
//int DAA_Init(int cch, int pcm_mode);
void DAA_Set_Rx_Gain(int cch, unsigned char rx_gain);
void DAA_Set_Tx_Gain(int cch, unsigned char tx_gain);
int DAA_Check_Line_State(int cch);
void DAA_On_Hook(int cch);
int DAA_Off_Hook(int cch);
unsigned char DAA_Hook_Status(int cch, int directly);
int DAA_Ring_Detection(int cch);
unsigned int DAA_Positive_Negative_Ring_Detect(int cch);
unsigned int DAA_Get_Polarity(int cch);
unsigned short DAA_Get_Line_Voltage(int cch);
void DAA_OnHook_Line_Monitor_Enable(int cch);
void DAA_Set_Dial_Mode(unsigned int cch, unsigned int mode);
unsigned int DAA_Get_Dial_Mode(unsigned int cch);
void DAA_Set_PulseDial_cch(unsigned int cch, unsigned int pulse_enable);
int DAA_PulseDial_Gen_Cfg(unsigned int pps, unsigned int make_duration, unsigned int interdigit_duration);
#ifdef CONFIG_RTK_VOIP_PULSE_DIAL_GEN_TIMER
//void DAA_PollPulseGenFifo(unsigned int cch);
//void DAA_PulseGenProcess_msec(void);
#else
//void DAA_PulseGenProcess(unsigned int cch);
#endif
//void DAA_PulseGenKill(unsigned int cch);
unsigned char DAA_Polarity_Reversal_Det(int cch);
unsigned char DAA_Bat_DropOut_Det(int cch);

extern void DAA_read_reg( int cch, unsigned int num, unsigned char *len, unsigned char *val );
extern void DAA_write_reg( int cch, unsigned int num, unsigned char *len, unsigned char *val );
extern void DAA_read_ram( int cch, unsigned short num, unsigned int *val );
extern void DAA_write_ram( int cch, unsigned short num, unsigned int val );
extern void DAA_dump_reg( int cch );
extern void DAA_dump_ram( int cch );

#endif // __SND_MUX_DAA_H__

