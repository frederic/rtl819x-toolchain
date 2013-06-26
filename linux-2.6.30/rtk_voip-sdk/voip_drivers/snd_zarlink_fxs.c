#include <linux/kernel.h>
#include "voip_types.h"
#include "voip_debug.h"
#include "con_register.h"
#include "snd_define.h"
#include "zarlink_api.h"
#include "snd_zarlink_common.h"

// --------------------------------------------------------
// zarlink fxs ops 
// --------------------------------------------------------

static void FXS_Ring_zarlink(voip_snd_t *this, unsigned char ringset )
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	ZarlinkFxsRing(pLine, ringset);
}

static unsigned char FXS_Check_Ring_zarlink(voip_snd_t *this)
{
	unsigned char ringer; //0: ring off, 1: ring on
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;

	ringer = ZarlinkCheckFxsRing(pLine);

	return ringer;
}

static void Adjust_SLIC_Tx_Gain_zarlink(voip_snd_t *this, int tx_gain)
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;

	ZarlinkAdjustSlicTxGain(pLine, tx_gain);
}

static void Adjust_SLIC_Rx_Gain_zarlink(voip_snd_t *this, int rx_gain)
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;

	ZarlinkAdjustSlicRxGain(pLine, rx_gain);
}

static void SLIC_Set_Ring_Cadence_zarlink(voip_snd_t *this, unsigned short OnMsec, unsigned short OffMsec)
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	ZarlinkSetRingCadence(pLine, OnMsec, OffMsec);
}

static void SLIC_Set_Ring_Freq_Amp_zarlink(voip_snd_t *this, char preset)
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	ZarlinkSetRingFreqAmp(pLine, preset);
}

static void SLIC_Set_Impendance_Country_zarlink(voip_snd_t *this, unsigned short country, unsigned short impd)
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	ZarlinkSetImpedenceCountry(pLine, (unsigned char)country);
}

static void SLIC_Set_Impendance_zarlink(voip_snd_t *this, unsigned short preset)
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	ZarlinkSetImpedence(pLine, preset);
}

#if 0
static void SLIC_GenProcessTone(unsigned int chid, genTone_struct *gen_tone)
{
}
#endif


static void OnHookLineReversal_zarlink(voip_snd_t *this, unsigned char bReversal) //0: Forward On-Hook Transmission, 1: Reverse On-Hook Transmission
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	ZarlinkSetOHT(pLine, bReversal);
}

static void SLIC_Set_LineVoltageZero_zarlink(voip_snd_t *this)
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	ZarlinkSetLineOpen(pLine);
	//ZarlinkSetLineState(pLine, VP_LINE_DISCONNECT);
}

static uint8 SLIC_CPC_Gen_zarlink(voip_snd_t *this)
{
#if 0	// con_polling.c: SLIC_CPC_Gen_cch() do this 
	extern void HookPollingDisable(int cch);

	if (slic_cpc[chid].cpc_start != 0)
	{
		PRINT_R("SLIC CPC gen not stop, ch=%d\n", chid);
		return;
	}
#endif

	uint8 pre_linefeed;
	
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	pre_linefeed = ZarlinkGetLineState( pLine ); // save current linefeed status

	ZarlinkSetLineOpen( pLine );
	//ZarlinkSetLineState( pLine, VP_LINE_DISCONNECT );

#if 0	// con_polling.c: SLIC_CPC_Gen_cch() do this 
	slic_cpc[chid].cpc_timeout = jiffies + (HZ*time_in_ms_of_cpc_signal/1000);
	slic_cpc[chid].cpc_start = 1;
	slic_cpc[chid].cpc_stop = 0;
	HookPollFlag[chid] = 0; // disable hook pooling
#endif
	
	return pre_linefeed;
}

static void SLIC_CPC_Check_zarlink(voip_snd_t *this, uint8 pre_linefeed)	// check in timer
{
#if 0	// con_polling.c: ENTRY_SLIC_CPC_Polling() do this 
	extern void HookPollingEnable(int cch);
	
	if (slic_cpc[chid].cpc_start == 0)
		return;
#endif

	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	// Stop wink function
#if 0
	if ((slic_cpc[chid].cpc_stop == 0) && (timetick_after(timetick, slic_cpc[chid].cpc_timeout)))
#endif
	{

		//printk("set linefeed=0x%x\n", slic_cpc[chid].pre_linefeed);
		ZarlinkSetLineState(pLine, pre_linefeed);

#if 0	// con_polling.c: ENTRY_SLIC_CPC_Polling() do this 
		slic_cpc[chid].cpc_timeout2 = jiffies + (HZ*200/1000);
		slic_cpc[chid].cpc_stop = 1;
#endif

	}
	
#if 0	// con_polling.c: ENTRY_SLIC_CPC_Polling() do this 
	if ((slic_cpc[chid].cpc_stop == 1) && (timetick_after(timetick, slic_cpc[chid].cpc_timeout2)))
	{
		slic_cpc[chid].cpc_start = 0;
		//HookPollFlag[chid] = 1; // enable hook pooling
		HookPollingEnable( chid );
	}
#endif
}

/*  return value:
	0: Phone dis-connect, 
	1: Phone connect, 
	2: Phone off-hook, 
	3: Check time out ( may connect too many phone set => view as connect),
	4: Can not check, Linefeed should be set to active state first.
*/
//static inline unsigned char SLIC_Get_Hook_Status( int chid );

static inline unsigned int FXS_Line_Check_zarlink( voip_snd_t *this )	// Note: this API may cause watch dog timeout. Should it disable WTD?
{
	//unsigned long flags;
	//unsigned int v_tip, v_ring, tick=0;
	//unsigned int v_tip, v_ring, tick = 0;
	//unsigned int connect_flag = 0, time_out_flag = 0;
	//unsigned char linefeed, rev_linefeed;

	if ( 1 == this ->fxs_ops ->SLIC_Get_Hook_Status( this, 1 ) )
	{
		//PRINT_MSG("%s: Phone 0ff-hook\n",__FUNCTION__);
		return 2;
	}

	return 4;
}


static void SendNTTCAR_zarlink( voip_snd_t *this )
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	ZarlinkSendNTTCAR(pLine);
}

static unsigned int SendNTTCAR_check_zarlink(voip_snd_t *this, unsigned long time_out)
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	return ZarlinkSendNTTCAR_Check(pLine, time_out);
}

static void disableOscillators_zarlink(voip_snd_t *this)
{
	printk("Not implemented!\n");
}

static void SetOnHookTransmissionAndBackupRegister_zarlink(voip_snd_t *this) // use for DTMF caller id
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	ZarlinkSetOHT(pLine, 0);
}

static inline void RestoreBackupRegisterWhenSetOnHookTransmission_zarlink(voip_snd_t *this) // use for DTMF caller id
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	ZarlinkGetLineState(pLine);//thlin test
}

#define PCMLAW_OFFSET	3

CT_ASSERT( BUSDATFMT_PCM_WIDEBAND_LINEAR - BUSDATFMT_PCM_LINEAR == PCMLAW_OFFSET );
CT_ASSERT( BUSDATFMT_PCM_WIDEBAND_ALAW - BUSDATFMT_PCM_ALAW == PCMLAW_OFFSET );
CT_ASSERT( BUSDATFMT_PCM_WIDEBAND_ULAW - BUSDATFMT_PCM_ULAW == PCMLAW_OFFSET );

static void SLIC_Set_PCM_state_zarlink(voip_snd_t *this, int enable)
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	/* By the limition of API-II, LE89 series, PCM tx/rx can not be mute at the same time. */
	ZarlinkSetPcmTxOnly(pLine, ( enable ? 0 : 1 ));// mute phone SPK
	//ZarlinkSetPcmRxOnly(chid, ( enable ? 0 : 1 ));// mute phone MIC
}

static unsigned char SLIC_Get_Hook_Status_zarlink(voip_snd_t *this, int directly)
{
	unsigned char status;
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;

	status = ZarlinkGetFxsHookStatus(pLine, directly);
	
	return status;
}

static void SLIC_read_reg_zarlink(voip_snd_t *this, unsigned int num, unsigned char *len, unsigned char *val)
{
	RTKLineObj * pLine = (RTKLineObj * )this ->priv;

	/* check if dump all */
	if (num == 890 || num == 880) {
		*len = 0;
		ZarlinkDumpDevReg(pLine);

	} else  if (num%2==0) {
		/* Zarlink user odd number if register as read register */
		*len = 0; 
		return;

	}else{
		ZarlinkRWDevReg(pLine,num,len,val);
	}

	return;
}

static void SLIC_write_reg_zarlink(voip_snd_t *this, unsigned int num, unsigned char *len, unsigned char *val)
{
	RTKLineObj * pLine = (RTKLineObj * )this ->priv;

	/* Zarlink user even number of register as write register */
	if (num%2==1) {
		*len = 0;
		return;
	}

	ZarlinkRWDevReg(pLine,num,len,val);
	return;
}

static void SLIC_read_ram_zarlink(voip_snd_t *this, unsigned short num, unsigned char len, unsigned int *val)
{
	printk("%s(%d)Not support yet!\n",__FUNCTION__,__LINE__);
}

static void SLIC_write_ram_zarlink(voip_snd_t *this, unsigned short num, unsigned int val)
{
	printk("%s(%d)Not support yet!\n",__FUNCTION__,__LINE__);
}

static void SLIC_dump_reg_zarlink(voip_snd_t *this)
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	ZarlinkDumpDevReg(pLine);
}

static void SLIC_dump_ram_zarlink(voip_snd_t *this)
{
	printk("%s(%d)Not support yet!\n",__FUNCTION__,__LINE__);
}

static void FXS_FXO_DTx_DRx_Loopback_zarlink(voip_snd_t *this, voip_snd_t *daa_snd, unsigned int enable)
{
	printk( "Not implement FXS_FXO loopback\n" );
}

static void SLIC_OnHookTrans_PCM_start_zarlink(voip_snd_t *this)
{
	this ->fxs_ops ->SLIC_Set_PCM_state(this, SLIC_PCM_ON);
	this ->fxs_ops ->OnHookLineReversal(this, 0);		//Forward On-Hook Transmission
	PRINT_MSG("SLIC_OnHookTrans_PCM_start, ch = %d\n", this ->sch);
}

static int enable_zarlink( voip_snd_t *this, int enable )
{
	SOLAC_PCMSetup_priv_ops( this, enable );
	this ->fxs_ops ->SLIC_Set_PCM_state( this, enable );
	
	return 0;
}

// --------------------------------------------------------
// channel mapping architecture 
// --------------------------------------------------------

const snd_ops_fxs_t snd_zarlink_fxs_ops = {
	// common operation 
	.enable = enable_zarlink,
	
	// for each snd_type 
	//.SLIC_reset = SLIC_reset_zarlink,
	.FXS_Ring = FXS_Ring_zarlink,
	.FXS_Check_Ring = FXS_Check_Ring_zarlink,
	.FXS_Line_Check = FXS_Line_Check_zarlink,	// Note: this API may cause watch dog timeout. Should it disable WTD?
	.SLIC_Set_PCM_state = SLIC_Set_PCM_state_zarlink,
	.SLIC_Get_Hook_Status = SLIC_Get_Hook_Status_zarlink,
	
	.Set_SLIC_Tx_Gain = Adjust_SLIC_Tx_Gain_zarlink,
	.Set_SLIC_Rx_Gain = Adjust_SLIC_Rx_Gain_zarlink,
	.SLIC_Set_Ring_Cadence = SLIC_Set_Ring_Cadence_zarlink,
	.SLIC_Set_Ring_Freq_Amp = SLIC_Set_Ring_Freq_Amp_zarlink,
	.SLIC_Set_Impendance_Country = SLIC_Set_Impendance_Country_zarlink, 
	.SLIC_Set_Impendance = SLIC_Set_Impendance_zarlink,
	.OnHookLineReversal = OnHookLineReversal_zarlink,	//0: Forward On-Hook Transmission, 1: Reverse On-Hook Transmission
	.SLIC_Set_LineVoltageZero = SLIC_Set_LineVoltageZero_zarlink,
	
	.SLIC_CPC_Gen = SLIC_CPC_Gen_zarlink,
	.SLIC_CPC_Check = SLIC_CPC_Check_zarlink,	// check in timer
	
	.SendNTTCAR = SendNTTCAR_zarlink,
	.SendNTTCAR_check = SendNTTCAR_check_zarlink,
	
	.disableOscillators = disableOscillators_zarlink,
	
	.SetOnHookTransmissionAndBackupRegister = SetOnHookTransmissionAndBackupRegister_zarlink,	// use for DTMF caller id
	.RestoreBackupRegisterWhenSetOnHookTransmission = RestoreBackupRegisterWhenSetOnHookTransmission_zarlink,	// use for DTMF caller id
	
	.FXS_FXO_DTx_DRx_Loopback = FXS_FXO_DTx_DRx_Loopback_zarlink,
	.SLIC_OnHookTrans_PCM_start = SLIC_OnHookTrans_PCM_start_zarlink,
	
	// read/write register/ram
	.SLIC_read_reg = SLIC_read_reg_zarlink,
	.SLIC_write_reg = SLIC_write_reg_zarlink,
	.SLIC_read_ram = SLIC_read_ram_zarlink,
	.SLIC_write_ram = SLIC_write_ram_zarlink,
	.SLIC_dump_reg = SLIC_dump_reg_zarlink,
	.SLIC_dump_ram = SLIC_dump_ram_zarlink,
	
	//.SLIC_show_ID = ??
};

