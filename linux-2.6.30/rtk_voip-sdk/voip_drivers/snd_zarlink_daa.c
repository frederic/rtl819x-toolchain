#include "rtk_voip.h"
#include "voip_types.h"
#include "con_register.h"
#include "zarlink_api.h"
#include "snd_zarlink_common.h"

// --------------------------------------------------------
// zarlink daa ops 
// --------------------------------------------------------

static void DAA_Set_Rx_Gain_zarlink(voip_snd_t *this, unsigned char rx_gain)
{
	//obsolete API
	#ifndef CONFIG_VOIP_SDK
	PRINT_Y("%s(%d)Not support yet!\n",__FUNCTION__,__LINE__);
	#endif
}

static void DAA_Set_Tx_Gain_zarlink(voip_snd_t *this, unsigned char tx_gain)
{
	//obsolete API
	#ifndef CONFIG_VOIP_SDK
	PRINT_Y("%s(%d)Not support yet!\n",__FUNCTION__,__LINE__);
	#endif
}

static int DAA_Check_Line_State_zarlink(voip_snd_t *this)	/* 0: connect, 1: not connect, 2: busy*/
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;

	return ZarlinkGetFxoLineStatus(pLine, 1); // not support busy
}

static void DAA_On_Hook_zarlink(voip_snd_t *this)
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	ZarlinkSetFxoLineOnHook(pLine);
}

static int DAA_Off_Hook_zarlink(voip_snd_t *this)
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;

	ZarlinkSetFxoLineOffHook(pLine);
	return 1;
}

static unsigned char DAA_Hook_Status_zarlink(voip_snd_t *this, int directly)
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	return ZarlinkGetFxoHookStatus(pLine);
}


/* 0: has not changed states.
 * 1: has transitioned from 0 to 1, or from 1 to 0, 
 * indicating the polarity of  TIP and RING is switched.
 * This API will be polling when ATA is up
 */
static unsigned char DAA_Polarity_Reversal_Det_zarlink(voip_snd_t *this)
{

	//PRINT_Y("%s(%d)Not support yet!\n",__FUNCTION__,__LINE__);
	return 0;//ZarlinkGetFxoLineStatus(pLine, 2);
}

// 0: has not changed states.
// 1: battery drop out
static unsigned char DAA_Bat_DropOut_Det_zarlink(voip_snd_t *this)
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	return ZarlinkGetFxoLineStatus(pLine, 3);
}

static int DAA_Ring_Detection_zarlink(voip_snd_t *this)
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	return ZarlinkGetFxoLineStatus(pLine, 0);
}

static unsigned int DAA_Positive_Negative_Ring_Detect_zarlink(voip_snd_t *this)
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	return ZarlinkGetFxoLineStatus(pLine, 0);
}

static unsigned int DAA_Get_Polarity_zarlink(voip_snd_t *this)
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	return ZarlinkGetFxoLineStatus(pLine, 2);// 0: normal, 1: reverse
}

static unsigned short DAA_Get_Line_Voltage_zarlink(voip_snd_t *this)
{
	// Will be polling when off-hook
	//PRINT_Y("%s(%d)Not support yet!\n",__FUNCTION__,__LINE__);
	return 0;
}

static void DAA_OnHook_Line_Monitor_Enable_zarlink(voip_snd_t *this)
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	
	ZarlinkSetFxoLineOHT(pLine);
}

static void DAA_Set_PulseDial_zarlink(voip_snd_t *this, unsigned int pulse_enable)
{
	#ifndef CONFIG_VOIP_SDK
	PRINT_Y("%s(%d)Not support yet!\n",__FUNCTION__,__LINE__);
	#endif
#ifdef PULSE_DIAL_GEN // deinfe in rtk_voip.h
#endif // PULSE_DIAL_GEN
}

static void DAA_read_reg_zarlink(voip_snd_t *this, unsigned int num, unsigned char *len, unsigned char *val)
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

	} else {
		ZarlinkRWDevReg(pLine,num,len,val);
	}
}

static void DAA_write_reg_zarlink(voip_snd_t *this, unsigned int num, unsigned char *len, unsigned char *val)
{
	RTKLineObj * pLine = (RTKLineObj * )this ->priv;

	/* Zarlink user even number of register as write register */
	if (num%2==1) {
		*len = 0;
		return;
	}

	ZarlinkRWDevReg(pLine,num,len,val);
}

static void DAA_read_ram_zarlink(voip_snd_t *this, unsigned short num, unsigned int *val)
{
	PRINT_Y("%s(%d)Not support yet!\n",__FUNCTION__,__LINE__);
}


static void DAA_write_ram_zarlink(voip_snd_t *this, unsigned short num, unsigned int val)
{
	PRINT_Y("%s(%d)Not support yet!\n",__FUNCTION__,__LINE__);
}

static void DAA_dump_reg_zarlink(voip_snd_t *this)
{
	RTKLineObj * pLine = (RTKLineObj * )this ->priv;
	ZarlinkDumpDevReg(pLine);
}

static void DAA_dump_ram_zarlink(voip_snd_t *this)
{
	PRINT_Y("%s(%d)Not support yet!\n",__FUNCTION__,__LINE__);
}

static int enable_daa_zarlink( voip_snd_t *this, int enable )
{

	SOLAC_PCMSetup_priv_ops( this, enable );

	//why cause daa off-hook????
	//this ->fxs_ops->SLIC_Set_PCM_state( this, enable );
	
	return 0;
}

// --------------------------------------------------------
// channel mapping architecture 
// --------------------------------------------------------


const snd_ops_daa_t snd_zarlink_daa_ops = {
	// common operation 
	.enable 							= enable_daa_zarlink,
	
	// for each snd_type 
	.DAA_Set_Rx_Gain 					= DAA_Set_Rx_Gain_zarlink,
	.DAA_Set_Tx_Gain 					= DAA_Set_Tx_Gain_zarlink,
	.DAA_Check_Line_State 				= DAA_Check_Line_State_zarlink,
	.DAA_On_Hook 						= DAA_On_Hook_zarlink,
	.DAA_Off_Hook 						= DAA_Off_Hook_zarlink,

	.DAA_Hook_Status 					= DAA_Hook_Status_zarlink,
	.DAA_Polarity_Reversal_Det 			= DAA_Polarity_Reversal_Det_zarlink,
	.DAA_Bat_DropOut_Det 				= DAA_Bat_DropOut_Det_zarlink,
	.DAA_Ring_Detection 				= DAA_Ring_Detection_zarlink,
	.DAA_Positive_Negative_Ring_Detect 	= DAA_Positive_Negative_Ring_Detect_zarlink,
	.DAA_Get_Polarity 					= DAA_Get_Polarity_zarlink,
	.DAA_Get_Line_Voltage 				= DAA_Get_Line_Voltage_zarlink,
	.DAA_OnHook_Line_Monitor_Enable 	= DAA_OnHook_Line_Monitor_Enable_zarlink,
	.DAA_Set_PulseDial 					= DAA_Set_PulseDial_zarlink,
	
	// read/write register/ram
	.DAA_read_reg 						= DAA_read_reg_zarlink,
	.DAA_write_reg 						= DAA_write_reg_zarlink,
	.DAA_read_ram 						= DAA_read_ram_zarlink,
	.DAA_write_ram 						= DAA_write_ram_zarlink,
	.DAA_dump_reg 						= DAA_dump_reg_zarlink,
	.DAA_dump_ram 						= DAA_dump_ram_zarlink,
};

