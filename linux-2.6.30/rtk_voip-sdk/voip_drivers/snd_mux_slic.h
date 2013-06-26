#ifndef __SND_MUX_SLIC_H__
#define __SND_MUX_SLIC_H__

#include "voip_types.h"

extern void SLIC_reset( uint32 cch, int codec_law );

extern void FXS_Ring( uint32 cch, unsigned char ring_set );
extern unsigned char FXS_Check_Ring( uint32 cch );

extern void Set_SLIC_Tx_Gain( uint32 cch, int tx_gain );
extern void Set_SLIC_Rx_Gain( uint32 cch, int rx_gain );
extern void SLIC_Set_Ring_Cadence( uint32 cch, unsigned short OnMsec, unsigned short OffMsec );
extern void SLIC_Set_Ring_Freq_Amp( uint32 cch, char preset );
extern void SLIC_Set_Impendance_Country( uint32 cch, unsigned short country, unsigned short impd );
extern void SLIC_Set_Impendance( uint32 cch, unsigned short preset );
extern void OnHookLineReversal( uint32 cch, unsigned char bReversal ); //0: Forward On-Hook Transmission, 1: Reverse On-Hook Transmission
extern void SLIC_Set_LineVoltageZero( uint32 cch );
//extern void SLIC_CPC_Gen( uint32 cch, unsigned int time_in_ms_of_cpc_signal );
//extern void SLIC_CPC_CheckAndStop( uint32 cch );	// check in timer
extern unsigned int FXS_Line_Check( uint32 cch );	// Note: this API may cause watch dog timeout. Should it disable WTD?

extern void SendNTTCAR( uint32 cch );
extern unsigned int SendNTTCAR_check( uint32 cch, unsigned long time_out );
extern void disableOscillators( uint32 cch );
extern void SetOnHookTransmissionAndBackupRegister( uint32 cch ); // use for DTMF caller id

extern void RestoreBackupRegisterWhenSetOnHookTransmission( uint32 cch ); // use for DTMF caller id
extern void SLIC_Set_PCM_state( uint32 cch, int enable );
extern unsigned char SLIC_Get_Hook_Status( uint32 cch, int directly );

extern void SLIC_read_reg( uint32 cch, unsigned int num, unsigned char * len, unsigned char *val );
extern void SLIC_write_reg( uint32 cch, unsigned int num, unsigned char * len, unsigned char *val );
extern void SLIC_read_ram( uint32 cch, unsigned short num, unsigned int *val );
extern void SLIC_write_ram( uint32 cch, unsigned short num, unsigned int val );
extern void SLIC_dump_reg( uint32 cch );
extern void SLIC_dump_ram( uint32 cch );
//extern void FXS_FXO_DTx_DRx_Loopback( uint32 cch, unsigned int enable );
extern void FXS_FXO_DTx_DRx_Loopback_greedy( uint32 cch, unsigned int enable );
extern void SLIC_OnHookTrans_PCM_start( uint32 cch );

#endif /* __SND_MUX_SLIC_H__ */

