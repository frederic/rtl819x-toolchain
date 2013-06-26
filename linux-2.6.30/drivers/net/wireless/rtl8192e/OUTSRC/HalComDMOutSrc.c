//============================================================
// File Name: HalComDMOutSrc.c 
//
// Description:
//
// This file is for 92CE/92CU/88EE/88EU outsource dynamic mechanism for partner.
//
//
//============================================================

//============================================================
// include files
//============================================================
#include "Mp_Precomp.h"

//
// 20112/01/31 MH HW team will use ODM module to do all dynamic scheme.
//
#if 0

const u2Byte dB_Invert_Table[8][12] = {
	{	1,		1,		1,		2,		2,		2,		2,		3,		3,		3,		4,		4},
	{	4,		5,		6,		6,		7,		8,		9,		10,		11,		13,		14,		16},
	{	18,		20,		22,		25,		28,		32,		35,		40,		45,		50,		56,		63},
	{	71,		79,		89,		100,	112,	126,	141,	158,	178,	200,	224,	251},
	{	282,	316,	355,	398,	447,	501,	562,	631,	708,	794,	891,	1000},
	{	1122,	1259,	1413,	1585,	1778,	1995,	2239,	2512,	2818,	3162,	3548,	3981},
	{	4467,	5012,	5623,	6310,	7079,	7943,	8913,	10000,	11220,	12589,	14125,	15849},
	{	17783,	19953,	22387,	25119,	28184,	31623,	35481,	39811,	44668,	50119,	56234,	65535}};

// 20100515 Joseph: Add global variable to keep temporary scan list for antenna switching test.
//u1Byte			tmpNumBssDesc;
//RT_WLAN_BSS	tmpbssDesc[MAX_BSS_DESC];


static u4Byte edca_setting_DL[HT_IOT_PEER_MAX] = 
// UNKNOWN		REALTEK_90	REALTEK_92SE	BROADCOM		RALINK		ATHEROS		CISCO		MARVELL		92U_AP		SELF_AP(UpLink/Rx)
{ 0xa44f, 		0x5ea44f, 	0x5e4322, 		0x5ea42b, 		0xa44f, 		0x3ea42b, 		0x5ea630,	0xa44f,		0xa42b,		0xa42b};

static u4Byte edca_setting_DL_GMode[HT_IOT_PEER_MAX] = 
// UNKNOWN		REALTEK_90	REALTEK_92SE	BROADCOM		RALINK		ATHEROS		CISCO		MARVELL		92U_AP		SELF_AP
{ 0x4322, 		0xa44f, 		0x5e4322,		0xa42b, 			0x5e4322, 	0x4322, 		0xa42b,		0xa44f,		0x5e4322,	0x5ea42b};

static u4Byte edca_setting_UL[HT_IOT_PEER_MAX] = 
// UNKNOWN		REALTEK_90	REALTEK_92SE	BROADCOM		RALINK		ATHEROS		CISCO		MARVELL		92U_AP		SELF_AP(DownLink/Tx)
{ 0x5e4322, 		0xa44f, 		0x5e4322,		0x5ea32b,  		0x5ea422, 	0x5ea322,	0x3ea430,	0x5ea44f,	0x5e4322,	0x5e4322};

//============================================================


//============================================================
// Global var
//============================================================
u4Byte OFDMSwingTable[OFDM_TABLE_SIZE_92D] = {
	0x7f8001fe, // 0, +6.0dB
	0x788001e2, // 1, +5.5dB
	0x71c001c7, // 2, +5.0dB
	0x6b8001ae, // 3, +4.5dB
	0x65400195, // 4, +4.0dB
	0x5fc0017f, // 5, +3.5dB
	0x5a400169, // 6, +3.0dB
	0x55400155, // 7, +2.5dB
	0x50800142, // 8, +2.0dB
	0x4c000130, // 9, +1.5dB
	0x47c0011f, // 10, +1.0dB
	0x43c0010f, // 11, +0.5dB
	0x40000100, // 12, +0dB
	0x3c8000f2, // 13, -0.5dB
	0x390000e4, // 14, -1.0dB
	0x35c000d7, // 15, -1.5dB
	0x32c000cb, // 16, -2.0dB
	0x300000c0, // 17, -2.5dB
	0x2d4000b5, // 18, -3.0dB
	0x2ac000ab, // 19, -3.5dB
	0x288000a2, // 20, -4.0dB
	0x26000098, // 21, -4.5dB
	0x24000090, // 22, -5.0dB
	0x22000088, // 23, -5.5dB
	0x20000080, // 24, -6.0dB
	0x1e400079, // 25, -6.5dB
	0x1c800072, // 26, -7.0dB
	0x1b00006c, // 27. -7.5dB
	0x19800066, // 28, -8.0dB
	0x18000060, // 29, -8.5dB
	0x16c0005b, // 30, -9.0dB
	0x15800056, // 31, -9.5dB
	0x14400051, // 32, -10.0dB
	0x1300004c, // 33, -10.5dB
	0x12000048, // 34, -11.0dB
	0x11000044, // 35, -11.5dB
	0x10000040, // 36, -12.0dB
	0x0f00003c,// 37, -12.5dB
	0x0e400039,// 38, -13.0dB    
	0x0d800036,// 39, -13.5dB
	0x0cc00033,// 40, -14.0dB
	0x0c000030,// 41, -14.5dB
	0x0b40002d,// 42, -15.0dB	
};


u1Byte CCKSwingTable_Ch1_Ch13[CCK_TABLE_SIZE][8] = {
	{0x36, 0x35, 0x2e, 0x25, 0x1c, 0x12, 0x09, 0x04},	// 0, +0dB
	{0x33, 0x32, 0x2b, 0x23, 0x1a, 0x11, 0x08, 0x04},	// 1, -0.5dB
	{0x30, 0x2f, 0x29, 0x21, 0x19, 0x10, 0x08, 0x03},	// 2, -1.0dB
	{0x2d, 0x2d, 0x27, 0x1f, 0x18, 0x0f, 0x08, 0x03},	// 3, -1.5dB
	{0x2b, 0x2a, 0x25, 0x1e, 0x16, 0x0e, 0x07, 0x03},	// 4, -2.0dB 
	{0x28, 0x28, 0x22, 0x1c, 0x15, 0x0d, 0x07, 0x03},	// 5, -2.5dB
	{0x26, 0x25, 0x21, 0x1b, 0x14, 0x0d, 0x06, 0x03},	// 6, -3.0dB
	{0x24, 0x23, 0x1f, 0x19, 0x13, 0x0c, 0x06, 0x03},	// 7, -3.5dB
	{0x22, 0x21, 0x1d, 0x18, 0x11, 0x0b, 0x06, 0x02},	// 8, -4.0dB 
	{0x20, 0x20, 0x1b, 0x16, 0x11, 0x08, 0x05, 0x02},	// 9, -4.5dB
	{0x1f, 0x1e, 0x1a, 0x15, 0x10, 0x0a, 0x05, 0x02},	// 10, -5.0dB 
	{0x1d, 0x1c, 0x18, 0x14, 0x0f, 0x0a, 0x05, 0x02},	// 11, -5.5dB
	{0x1b, 0x1a, 0x17, 0x13, 0x0e, 0x09, 0x04, 0x02},	// 12, -6.0dB 
	{0x1a, 0x19, 0x16, 0x12, 0x0d, 0x09, 0x04, 0x02},	// 13, -6.5dB
	{0x18, 0x17, 0x15, 0x11, 0x0c, 0x08, 0x04, 0x02},	// 14, -7.0dB 
	{0x17, 0x16, 0x13, 0x10, 0x0c, 0x08, 0x04, 0x02},	// 15, -7.5dB
	{0x16, 0x15, 0x12, 0x0f, 0x0b, 0x07, 0x04, 0x01},	// 16, -8.0dB 
	{0x14, 0x14, 0x11, 0x0e, 0x0b, 0x07, 0x03, 0x02},	// 17, -8.5dB
	{0x13, 0x13, 0x10, 0x0d, 0x0a, 0x06, 0x03, 0x01},	// 18, -9.0dB 
	{0x12, 0x12, 0x0f, 0x0c, 0x09, 0x06, 0x03, 0x01},	// 19, -9.5dB
	{0x11, 0x11, 0x0f, 0x0c, 0x09, 0x06, 0x03, 0x01},	// 20, -10.0dB
	{0x10, 0x10, 0x0e, 0x0b, 0x08, 0x05, 0x03, 0x01},	// 21, -10.5dB
	{0x0f, 0x0f, 0x0d, 0x0b, 0x08, 0x05, 0x03, 0x01},	// 22, -11.0dB
	{0x0e, 0x0e, 0x0c, 0x0a, 0x08, 0x05, 0x02, 0x01},	// 23, -11.5dB
	{0x0d, 0x0d, 0x0c, 0x0a, 0x07, 0x05, 0x02, 0x01},	// 24, -12.0dB
	{0x0d, 0x0c, 0x0b, 0x09, 0x07, 0x04, 0x02, 0x01},	// 25, -12.5dB
	{0x0c, 0x0c, 0x0a, 0x09, 0x06, 0x04, 0x02, 0x01},	// 26, -13.0dB
	{0x0b, 0x0b, 0x0a, 0x08, 0x06, 0x04, 0x02, 0x01},	// 27, -13.5dB
	{0x0b, 0x0a, 0x09, 0x08, 0x06, 0x04, 0x02, 0x01},	// 28, -14.0dB
	{0x0a, 0x0a, 0x09, 0x07, 0x05, 0x03, 0x02, 0x01},	// 29, -14.5dB
	{0x0a, 0x09, 0x08, 0x07, 0x05, 0x03, 0x02, 0x01},	// 30, -15.0dB
	{0x09, 0x09, 0x08, 0x06, 0x05, 0x03, 0x01, 0x01},	// 31, -15.5dB
	{0x09, 0x08, 0x07, 0x06, 0x04, 0x03, 0x01, 0x01}	// 32, -16.0dB
};


u1Byte CCKSwingTable_Ch14 [CCK_TABLE_SIZE][8]= {
	{0x36, 0x35, 0x2e, 0x1b, 0x00, 0x00, 0x00, 0x00},	// 0, +0dB	
	{0x33, 0x32, 0x2b, 0x19, 0x00, 0x00, 0x00, 0x00},	// 1, -0.5dB 
	{0x30, 0x2f, 0x29, 0x18, 0x00, 0x00, 0x00, 0x00},	// 2, -1.0dB  
	{0x2d, 0x2d, 0x17, 0x17, 0x00, 0x00, 0x00, 0x00},	// 3, -1.5dB
	{0x2b, 0x2a, 0x25, 0x15, 0x00, 0x00, 0x00, 0x00},	// 4, -2.0dB  
	{0x28, 0x28, 0x24, 0x14, 0x00, 0x00, 0x00, 0x00},	// 5, -2.5dB
	{0x26, 0x25, 0x21, 0x13, 0x00, 0x00, 0x00, 0x00},	// 6, -3.0dB  
	{0x24, 0x23, 0x1f, 0x12, 0x00, 0x00, 0x00, 0x00},	// 7, -3.5dB  
	{0x22, 0x21, 0x1d, 0x11, 0x00, 0x00, 0x00, 0x00},	// 8, -4.0dB  
	{0x20, 0x20, 0x1b, 0x10, 0x00, 0x00, 0x00, 0x00},	// 9, -4.5dB
	{0x1f, 0x1e, 0x1a, 0x0f, 0x00, 0x00, 0x00, 0x00},	// 10, -5.0dB  
	{0x1d, 0x1c, 0x18, 0x0e, 0x00, 0x00, 0x00, 0x00},	// 11, -5.5dB
	{0x1b, 0x1a, 0x17, 0x0e, 0x00, 0x00, 0x00, 0x00},	// 12, -6.0dB  
	{0x1a, 0x19, 0x16, 0x0d, 0x00, 0x00, 0x00, 0x00},	// 13, -6.5dB 
	{0x18, 0x17, 0x15, 0x0c, 0x00, 0x00, 0x00, 0x00},	// 14, -7.0dB  
	{0x17, 0x16, 0x13, 0x0b, 0x00, 0x00, 0x00, 0x00},	// 15, -7.5dB
	{0x16, 0x15, 0x12, 0x0b, 0x00, 0x00, 0x00, 0x00},	// 16, -8.0dB  
	{0x14, 0x14, 0x11, 0x0a, 0x00, 0x00, 0x00, 0x00},	// 17, -8.5dB
	{0x13, 0x13, 0x10, 0x0a, 0x00, 0x00, 0x00, 0x00},	// 18, -9.0dB  
	{0x12, 0x12, 0x0f, 0x09, 0x00, 0x00, 0x00, 0x00},	// 19, -9.5dB
	{0x11, 0x11, 0x0f, 0x09, 0x00, 0x00, 0x00, 0x00},	// 20, -10.0dB
	{0x10, 0x10, 0x0e, 0x08, 0x00, 0x00, 0x00, 0x00},	// 21, -10.5dB
	{0x0f, 0x0f, 0x0d, 0x08, 0x00, 0x00, 0x00, 0x00},	// 22, -11.0dB
	{0x0e, 0x0e, 0x0c, 0x07, 0x00, 0x00, 0x00, 0x00},	// 23, -11.5dB
	{0x0d, 0x0d, 0x0c, 0x07, 0x00, 0x00, 0x00, 0x00},	// 24, -12.0dB
	{0x0d, 0x0c, 0x0b, 0x06, 0x00, 0x00, 0x00, 0x00},	// 25, -12.5dB
	{0x0c, 0x0c, 0x0a, 0x06, 0x00, 0x00, 0x00, 0x00},	// 26, -13.0dB
	{0x0b, 0x0b, 0x0a, 0x06, 0x00, 0x00, 0x00, 0x00},	// 27, -13.5dB
	{0x0b, 0x0a, 0x09, 0x05, 0x00, 0x00, 0x00, 0x00},	// 28, -14.0dB
	{0x0a, 0x0a, 0x09, 0x05, 0x00, 0x00, 0x00, 0x00},	// 29, -14.5dB
	{0x0a, 0x09, 0x08, 0x05, 0x00, 0x00, 0x00, 0x00},	// 30, -15.0dB
	{0x09, 0x09, 0x08, 0x05, 0x00, 0x00, 0x00, 0x00},	// 31, -15.5dB
	{0x09, 0x08, 0x07, 0x04, 0x00, 0x00, 0x00, 0x00}	// 32, -16.0dB
};	

//#pragma mark --DIG --
//3============================================================
//3 DIG
//3============================================================
/*-----------------------------------------------------------------------------
 * Function:	odm_DIGInit()
 *
 * Overview:	Set DIG scheme init value.
 *
 * Input:		NONE
 *
 * Output:		NONE
 *
 * Return:		NONE
 *
 * Revised History:
 *	When		Who		Remark
 *
 *---------------------------------------------------------------------------*/
void	odm_DIGInit(
	IN	PADAPTER	pAdapter
)
{
	pDIG_T	pDM_DigTable = &pAdapter->DM_DigTable;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);

	pDM_DigTable->Dig_Enable_Flag = TRUE;
	pDM_DigTable->Dig_Ext_Port_Stage = DIG_EXT_PORT_STAGE_MAX;
	
	pDM_DigTable->CurIGValue = 0x20;
	pDM_DigTable->PreIGValue = 0x0;

	pDM_DigTable->CurSTAConnectState = pDM_DigTable->PreSTAConnectState = DIG_STA_DISCONNECT;
	pDM_DigTable->CurMultiSTAConnectState = DIG_MultiSTA_DISCONNECT;

	pDM_DigTable->RssiLowThresh 	= DM_DIG_THRESH_LOW;
	pDM_DigTable->RssiHighThresh 	= DM_DIG_THRESH_HIGH;

	pDM_DigTable->FALowThresh	= DM_FALSEALARM_THRESH_LOW;
	pDM_DigTable->FAHighThresh	= DM_FALSEALARM_THRESH_HIGH;

	//vivi note! 92d and 92c has different value
	/*if(IS_HARDWARE_TYPE_8192D(pAdapter))
	{
		pDM_DigTable->rx_gain_range_max = DM_DIG_FA_UPPER;
		pDM_DigTable->rx_gain_range_min = DM_DIG_FA_LOWER;
	}*/
	//else
	//{
		pDM_DigTable->rx_gain_range_max = DM_DIG_MAX;
		pDM_DigTable->rx_gain_range_min = DM_DIG_MIN;
	//}
	
	pDM_DigTable->BackoffVal = DM_DIG_BACKOFF_DEFAULT;
	pDM_DigTable->BackoffVal_range_max = DM_DIG_BACKOFF_MAX;
	pDM_DigTable->BackoffVal_range_min = DM_DIG_BACKOFF_MIN;

	//vivi note! 92d and 92c has different value
	/*if(IS_HARDWARE_TYPE_8192D(pAdapter))
	{
		pDM_DigTable->PreCCKPDState = CCK_PD_STAGE_LowRssi;
		pDM_DigTable->CurCCKPDState = CCK_PD_STAGE_MAX;
		pDM_DigTable->ForbiddenIGI = DM_DIG_FA_LOWER;
	}*/
	//else
	//{
		pDM_DigTable->PreCCKPDState = CCK_PD_STAGE_MAX;
		pDM_DigTable->CurCCKPDState = CCK_PD_STAGE_LowRssi;
		pDM_DigTable->ForbiddenIGI = DM_DIG_MIN;
		pDM_DigTable->DeadZone = DM_DIG_MIN;
	//}
	pDM_DigTable->LargeFAHit = 0;
	pDM_DigTable->Recover_cnt = 0;
	
	pDM_DigTable->DIG_Dynamic_MIN_0 = 0x25;
	pDM_DigTable->DIG_Dynamic_MIN_1 = 0x25;
	pDM_DigTable->bMediaConnect_0 =FALSE;
	pDM_DigTable->bMediaConnect_1 =FALSE;

	pDM_DigTable->preearly= 2;
	pDM_DigTable->curearly= 1;	

	// The EDCCA value should be adjusted according to the DIG value. 2011.11.25. by tynli.
	pHalData->bPreEdccaEnable = FALSE;
	
}

VOID
ODM_Write_DIG(
	IN	PADAPTER	pAdapter
	)
{
	pDIG_T	pDM_DigTable = &pAdapter->DM_DigTable;

	//RT_TRACE(	COMP_DIG, DBG_LOUD, ("CurIGValue = 0x%x, PreIGValue = 0x%x, BackoffVal = %d\n", 
	//			pDM_DigTable->CurIGValue, pDM_DigTable->PreIGValue, pDM_DigTable->BackoffVal));
	RT_TRACE(	COMP_DIG, DBG_LOUD, ("CurIGValue = 0x%x, PreIGValue = 0x%x\n", 
				pDM_DigTable->CurIGValue, pDM_DigTable->PreIGValue));

#if 0 // Fix DIG for NdisTest in the house --------------------------
	//PHY_SetBBReg(pAdapter, rOFDM0_RxDetector1, 0x7f, 0x4c);
	//PHY_SetBBReg(pAdapter, rOFDM0_XAAGCCore1, 0x7f, 0x4e);
	//PHY_SetBBReg(pAdapter, rOFDM0_XBAGCCore1, 0x7f, 0x4e);
	return;
#endif // -------------------------------------------------


	if (pDM_DigTable->Dig_Enable_Flag == FALSE)
	{
		RT_TRACE(	COMP_DIG, DBG_LOUD, ("DIG is disabled\n"));
		//pDM_DigTable->PreIGValue = 0x17;
		return;
	}
	
	if(pDM_DigTable->PreIGValue != pDM_DigTable->CurIGValue)
	{
		// Set initial gain.
		// 20100211 Joseph: Set only BIT0~BIT6 for DIG. BIT7 is the function switch of Antenna diversity.
		// Just not to modified it for SD3 testing.
		//PHY_SetBBReg(pAdapter, rOFDM0_XAAGCCore1, bMaskByte0, DM_DigTable.CurIGValue);
		//PHY_SetBBReg(pAdapter, rOFDM0_XBAGCCore1, bMaskByte0, DM_DigTable.CurIGValue);
		PHY_SetBBReg(pAdapter, rOFDM0_XAAGCCore1, 0x7f, pDM_DigTable->CurIGValue);
		if(!IS_HARDWARE_TYPE_8188E(pAdapter))
		PHY_SetBBReg(pAdapter, rOFDM0_XBAGCCore1, 0x7f, pDM_DigTable->CurIGValue);
		// 2011/07/12 MH According to Luke.Lee's suggestion, we need to restore 0xc50 by driver 
		// after scan process.
		//if(pDM_DigTable->CurIGValue != 0x17)
			pDM_DigTable->PreIGValue = pDM_DigTable->CurIGValue;
	}

	// Adjust EDCCA.
	if(!IS_HARDWARE_TYPE_8188E(pAdapter))
	odm_DynamicEDCCA(pAdapter);
}

//#pragma mark -- Init Gain --
//3============================================================
//3 Initial Gain
//3============================================================
u1Byte
odm_initial_gain_MinPWDB(
	IN	PADAPTER	pAdapter
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	s4Byte			Rssi_val_min = 0;
#if 0
	pDIG_T	pDM_DigTable = &pAdapter->DM_DigTable;

	if(	(pDM_DigTable->CurMultiSTAConnectState == DIG_MultiSTA_CONNECT) &&
		(pDM_DigTable->CurSTAConnectState == DIG_STA_CONNECT) )
	{
		if(pHalData->EntryMinUndecoratedSmoothedPWDB != 0)
			Rssi_val_min  =  (pHalData->EntryMinUndecoratedSmoothedPWDB > pHalData->UndecoratedSmoothedPWDB)?
					pHalData->UndecoratedSmoothedPWDB:pHalData->EntryMinUndecoratedSmoothedPWDB;		
		else
			Rssi_val_min = pHalData->UndecoratedSmoothedPWDB;
	}
	else if(	pDM_DigTable->CurSTAConnectState == DIG_STA_CONNECT || 
			pDM_DigTable->CurSTAConnectState == DIG_STA_BEFORE_CONNECT) 
		Rssi_val_min = pHalData->UndecoratedSmoothedPWDB;
	else if(pDM_DigTable->CurMultiSTAConnectState == DIG_MultiSTA_CONNECT)
		Rssi_val_min = pHalData->EntryMinUndecoratedSmoothedPWDB;
#endif
	if(pHalData->EntryMinUndecoratedSmoothedPWDB != 0)
			Rssi_val_min  =  (pHalData->EntryMinUndecoratedSmoothedPWDB > pHalData->UndecoratedSmoothedPWDB)?
					pHalData->UndecoratedSmoothedPWDB:pHalData->EntryMinUndecoratedSmoothedPWDB;		
	else
		Rssi_val_min = pHalData->UndecoratedSmoothedPWDB;

	return (u1Byte)Rssi_val_min;
}


VOID
ODM_ChangeDynamicInitGainThresh(
	IN	PADAPTER	pAdapter,
	IN	INT32		DM_Type,
	IN	INT32		DM_Value)
{
	pDIG_T	pDM_DigTable = &pAdapter->DM_DigTable;

	if (DM_Type == DIG_TYPE_THRESH_HIGH)
	{
		pDM_DigTable->RssiHighThresh = DM_Value;		
	}
	else if (DM_Type == DIG_TYPE_THRESH_LOW)
	{
		pDM_DigTable->RssiLowThresh = DM_Value;
	}
	else if (DM_Type == DIG_TYPE_ENABLE)
	{
		pDM_DigTable->Dig_Enable_Flag	= TRUE;
	}	
	else if (DM_Type == DIG_TYPE_DISABLE)
	{
		pDM_DigTable->Dig_Enable_Flag = FALSE;
	}	
	else if (DM_Type == DIG_TYPE_BACKOFF)
	{
		if(DM_Value > 30)
			DM_Value = 30;
		pDM_DigTable->BackoffVal = (u1Byte)DM_Value;
	}
	else if(DM_Type == DIG_TYPE_RX_GAIN_MIN)
	{
		if(DM_Value == 0)
			DM_Value = 0x1;
		pDM_DigTable->rx_gain_range_min = (u1Byte)DM_Value;
	}
	else if(DM_Type == DIG_TYPE_RX_GAIN_MAX)
	{
		if(DM_Value > 0x50)
			DM_Value = 0x50;
		pDM_DigTable->rx_gain_range_max = (u1Byte)DM_Value;
	}
}	/* DM_ChangeDynamicInitGainThresh */

//#pragma mark --Tx Power --
//3============================================================
//3 Dynamic Tx Power
//3============================================================

VOID
odm_SavePowerIndex(IN	PADAPTER	Adapter)
{
	u1Byte			index;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u4Byte			Power_Index_REG[6] = {0xc90, 0xc91, 0xc92, 0xc98, 0xc99, 0xc9a};
	
	for(index = 0; index< 6; index++)
		pHalData->PowerIndex_backup[index] = PlatformEFIORead1Byte(Adapter, Power_Index_REG[index]);
}

VOID
odm_RestorePowerIndex(IN	PADAPTER	Adapter)
{
	u1Byte			index;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u4Byte			Power_Index_REG[6] = {0xc90, 0xc91, 0xc92, 0xc98, 0xc99, 0xc9a};
	
	for(index = 0; index< 6; index++)
		PlatformEFIOWrite1Byte(Adapter, Power_Index_REG[index], pHalData->PowerIndex_backup[index]);
}

VOID
odm_WritePowerIndex(
		IN	PADAPTER	Adapter, 
		IN 	u1Byte		Value)
{
	u1Byte			index;
	u4Byte			Power_Index_REG[6] = {0xc90, 0xc91, 0xc92, 0xc98, 0xc99, 0xc9a};
	
	for(index = 0; index< 6; index++)
		PlatformEFIOWrite1Byte(Adapter, Power_Index_REG[index], Value);
}
void odm_InitDynamicTxPower(IN	PADAPTER	Adapter)
{
	PMGNT_INFO			pMgntInfo = &Adapter->MgntInfo;
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);

#if DEV_BUS_TYPE==RT_USB_INTERFACE					
	if(RT_GetInterfaceSelection(Adapter) == INTF_SEL1_USB_High_Power)
	{
		odm_SavePowerIndex(Adapter);
		pMgntInfo->bDynamicTxPowerEnable = TRUE;
	}		
	else	
#else
	pMgntInfo->bDynamicTxPowerEnable = FALSE;
#endif
	
	if(IS_HARDWARE_TYPE_8192D(Adapter) && (Adapter->MgntInfo.FirmwareVersion > 0x14))
	{
		RT_TRACE(COMP_INIT, DBG_LOUD, ("Dynamic Tx Power is enabled by FW \n"));
		pMgntInfo->bDynamicTxPowerEnable = FALSE;
	}
	
	pHalData->LastDTPLvl = TxHighPwrLevel_Normal;
	pHalData->DynamicTxHighPowerLvl = TxHighPwrLevel_Normal;
}

void odm_DynamicTxPower (IN	PADAPTER	Adapter)
{
	PMGNT_INFO			pMgntInfo = &Adapter->MgntInfo;
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	s4Byte				UndecoratedSmoothedPWDB;


	// STA not connected and AP not connected
	if((!pMgntInfo->bMediaConnect) &&	
		(pHalData->EntryMinUndecoratedSmoothedPWDB == 0))
	{
		RT_TRACE(COMP_HIPWR, DBG_LOUD, ("Not connected to any \n"));
		pHalData->DynamicTxHighPowerLvl = TxHighPwrLevel_Normal;

		//the LastDTPlvl should reset when disconnect, 
		//otherwise the tx power level wouldn't change when disconnect and connect again.
		// Maddest 20091220.
		 pHalData->LastDTPLvl=TxHighPwrLevel_Normal;
		return;
	}

#if (INTEL_PROXIMITY_SUPPORT == 1)
	// Intel set fixed tx power 
	if(pMgntInfo->IntelProximityModeInfo.PowerOutput > 0)
	{
		switch(pMgntInfo->IntelProximityModeInfo.PowerOutput){
			case 1:
				pHalData->DynamicTxHighPowerLvl = TxHighPwrLevel_100;
				RT_TRACE(COMP_HIPWR, DBG_LOUD, ("TxHighPwrLevel_100\n"));
				break;
			case 2:
				pHalData->DynamicTxHighPowerLvl = TxHighPwrLevel_70;
				RT_TRACE(COMP_HIPWR, DBG_LOUD, ("TxHighPwrLevel_70\n"));
				break;
			case 3:
				pHalData->DynamicTxHighPowerLvl = TxHighPwrLevel_50;
				RT_TRACE(COMP_HIPWR, DBG_LOUD, ("TxHighPwrLevel_50\n"));
				break;
			case 4:
				pHalData->DynamicTxHighPowerLvl = TxHighPwrLevel_35;
				RT_TRACE(COMP_HIPWR, DBG_LOUD, ("TxHighPwrLevel_35\n"));
				break;
			case 5:
				pHalData->DynamicTxHighPowerLvl = TxHighPwrLevel_15;
				RT_TRACE(COMP_HIPWR, DBG_LOUD, ("TxHighPwrLevel_15\n"));
				break;
			default:
				pHalData->DynamicTxHighPowerLvl = TxHighPwrLevel_100;
				RT_TRACE(COMP_HIPWR, DBG_LOUD, ("TxHighPwrLevel_100\n"));
				break;
		}		
	}
	else
#endif		
	{ 
		if(	(pMgntInfo->bDynamicTxPowerEnable != TRUE) ||
			(pHalData->DMFlag & HAL_DM_HIPWR_DISABLE) ||
			pMgntInfo->IOTAction & HT_IOT_ACT_DISABLE_HIGH_POWER)
		{
			pHalData->DynamicTxHighPowerLvl = TxHighPwrLevel_Normal;
		}
		else
		{
			if(pMgntInfo->bMediaConnect)	// Default port
			{
				if(ACTING_AS_AP(Adapter) || ACTING_AS_IBSS(Adapter))
				{
					UndecoratedSmoothedPWDB = pHalData->EntryMinUndecoratedSmoothedPWDB;
					RT_TRACE(COMP_HIPWR, DBG_LOUD, ("AP Client PWDB = 0x%x \n", UndecoratedSmoothedPWDB));
				}
				else
				{
					UndecoratedSmoothedPWDB = pHalData->UndecoratedSmoothedPWDB;
					RT_TRACE(COMP_HIPWR, DBG_LOUD, ("STA Default Port PWDB = 0x%x \n", UndecoratedSmoothedPWDB));
				}
			}
			else // associated entry pwdb
			{	
				UndecoratedSmoothedPWDB = pHalData->EntryMinUndecoratedSmoothedPWDB;
				RT_TRACE(COMP_HIPWR, DBG_LOUD, ("AP Ext Port PWDB = 0x%x \n", UndecoratedSmoothedPWDB));
			}
				
			if(UndecoratedSmoothedPWDB >= TX_POWER_NEAR_FIELD_THRESH_LVL2)
			{
				pHalData->DynamicTxHighPowerLvl = TxHighPwrLevel_Level2;
				RT_TRACE(COMP_HIPWR, DBG_LOUD, ("TxHighPwrLevel_Level1 (TxPwr=0x0)\n"));
			}
			else if((UndecoratedSmoothedPWDB < (TX_POWER_NEAR_FIELD_THRESH_LVL2-3)) &&
				(UndecoratedSmoothedPWDB >= TX_POWER_NEAR_FIELD_THRESH_LVL1) )
			{
				pHalData->DynamicTxHighPowerLvl = TxHighPwrLevel_Level1;
				RT_TRACE(COMP_HIPWR, DBG_LOUD, ("TxHighPwrLevel_Level1 (TxPwr=0x10)\n"));
			}
			else if(UndecoratedSmoothedPWDB < (TX_POWER_NEAR_FIELD_THRESH_LVL1-5))
			{
				pHalData->DynamicTxHighPowerLvl = TxHighPwrLevel_Normal;
				RT_TRACE(COMP_HIPWR, DBG_LOUD, ("TxHighPwrLevel_Normal\n"));
			}
		}
	}
	if( pHalData->DynamicTxHighPowerLvl != pHalData->LastDTPLvl )
	{
		RT_TRACE(COMP_HIPWR, DBG_LOUD, ("PHY_SetTxPowerLevel8192C() Channel = %d \n" , pHalData->CurrentChannel));

#if DEV_BUS_TYPE != RT_SDIO_INTERFACE
		if(IS_HARDWARE_TYPE_8812(Adapter))
		{
			PHY_SetTxPowerLevel8812(Adapter, pHalData->CurrentChannel);
		}
		else
#endif
		if(IS_HARDWARE_TYPE_8188E(Adapter))
		{
			PHY_SetTxPowerLevel8188E(Adapter, pHalData->CurrentChannel);
		}
		else if(IS_HARDWARE_TYPE_8192C(Adapter))
		{
			PHY_SetTxPowerLevel8192C(Adapter, pHalData->CurrentChannel);
		}
		if(	(pHalData->DynamicTxHighPowerLvl == TxHighPwrLevel_Normal) &&
			(pHalData->LastDTPLvl == TxHighPwrLevel_Level1 || pHalData->LastDTPLvl == TxHighPwrLevel_Level2)) //TxHighPwrLevel_Normal
			odm_RestorePowerIndex(Adapter);
		else if(pHalData->DynamicTxHighPowerLvl == TxHighPwrLevel_Level1)
			odm_WritePowerIndex(Adapter, 0x14);
		else if(pHalData->DynamicTxHighPowerLvl == TxHighPwrLevel_Level2)
			odm_WritePowerIndex(Adapter, 0x10);
	}
	pHalData->LastDTPLvl = pHalData->DynamicTxHighPowerLvl;
}
//============================================================

//#pragma mark --EDCA Turbo --
//3============================================================
//3 EDCA Turbo
//3============================================================
void
ODM_InitEdcaTurbo(
	IN	PADAPTER	Adapter
	)
{
	HAL_DATA_TYPE			*pHalData = GET_HAL_DATA(Adapter);

	pHalData->bCurrentTurboEDCA = FALSE;
	pHalData->bIsAnyNonBEPkts = FALSE;
	pHalData->bIsCurRDLState = FALSE;
	
}	// ODM_InitEdcaTurbo

void
odm_CheckEdcaTurbo(
	IN	PADAPTER	Adapter
	)
{
	PADAPTER 			pDefaultAdapter = GetDefaultAdapter(Adapter);
	PADAPTER 			pExtAdapter = NULL;
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	PMGNT_INFO			pMgntInfo = &Adapter->MgntInfo;
	PSTA_QOS			pStaQos = Adapter->MgntInfo.pStaQos;

	// Keep past Tx/Rx packet count for RT-to-RT EDCA turbo.
	//static u8Byte			lastTxOkCnt = 0;
	//static u8Byte			lastRxOkCnt = 0;
	u8Byte				curTxOkCnt = 0;
	u8Byte				curRxOkCnt = 0;	
	u4Byte				EDCA_BE_UL = 0x5ea42b;//Parameter suggested by Scott  //edca_setting_UL[pMgntInfo->IOTPeer];
	u4Byte				EDCA_BE_DL = 0x5ea42b;//Parameter suggested by Scott  //edca_setting_DL[pMgntInfo->IOTPeer];

	//[Win7 Count Tx/Rx statistic for Extension Port] odm_CheckEdcaTurbo's Adapter is always Default. 2009.08.20, by Bohn
	
	//static u8Byte 			Ext_lastTxOkCnt = 0;
	//static u8Byte 			Ext_lastRxOkCnt = 0;
	u8Byte				Ext_curTxOkCnt = 0;
	u8Byte				Ext_curRxOkCnt = 0;	
	//For future Win7  Enable Default Port to modify AMPDU size dynamically, 2009.08.20, Bohn.	
	u1Byte TwoPortStatus = (u1Byte)TWO_PORT_STATUS__WITHOUT_ANY_ASSOCIATE;


	pExtAdapter = GetFirstExtAdapter(Adapter);
	if(pExtAdapter == NULL) pExtAdapter = pDefaultAdapter;
	
#ifndef UNDER_CE		
	if(BT_DisableEDCATurbo(Adapter))
	{
		return;
	}
#endif	

	if(IS_HARDWARE_TYPE_8192D(Adapter))
	{
		if(IS_92D_SINGLEPHY(pHalData->VersionID))
		{
			EDCA_BE_UL = 0x60a42b;
			EDCA_BE_DL = 0x60a42b;
		}
		else
		{
			EDCA_BE_UL = 0x6ea42b;
			EDCA_BE_DL = 0x6ea42b;
		}

		// IOT.
#if DEV_BUS_TYPE==RT_PCI_INTERFACE		
		if(pMgntInfo->IOTPeer == HT_IOT_PEER_CISCO && IS_WIRELESS_MODE_N_24G(Adapter))
		{
			// Suggested by SD3 Cherry. Added by tynli. 2011.12.19.
			EDCA_BE_DL = edca_setting_DL[pMgntInfo->IOTPeer];
			EDCA_BE_UL = 0x5ea430;
		}
#endif		
	}
	else
	{
#if DEV_BUS_TYPE==RT_PCI_INTERFACE
		if(IS_92C_SERIAL(pHalData->VersionID))
		{
			EDCA_BE_UL = 0x60a42b;
			EDCA_BE_DL = 0x60a42b;
		}
		else
		{
			EDCA_BE_UL = 0x6ea42b;
			EDCA_BE_DL = 0x6ea42b;
		}
#endif
	}
	GetTwoPortSharedResource(Adapter,TWO_PORT_SHARED_OBJECT__STATUS,NULL,&TwoPortStatus);
	if(TwoPortStatus == TWO_PORT_STATUS__EXTENSION_ONLY)
	{
		EDCA_BE_UL = 0x5ea42b;//Parameter suggested by Scott  //edca_setting_UL[ExtAdapter->MgntInfo.IOTPeer];
		EDCA_BE_DL = 0x5ea42b;//Parameter suggested by Scott  //edca_setting_DL[ExtAdapter->MgntInfo.IOTPeer];
	}
	

	RT_TRACE(COMP_TURBO,DBG_TRACE,("==>odm_CheckEdcaTurbo, Now is %s Adapt.\n",(IsDefaultAdapter(Adapter))?"Def":"Ext"));
//	RT_TRACE(COMP_TURBO,DBG_TRACE,("Def_IOTPeer=%ld.Ext_IOTPeer=%ld\n",pDefaultAdapter->MgntInfo.IOTPeer, ExtAdapter->MgntInfo.IOTPeer));	

	//
	// Do not be Turbo if it's under WiFi config and Qos Enabled, because the EDCA parameters 
	// should follow the settings from QAP. By Bruce, 2007-12-07.
	//
	if(Adapter->MgntInfo.bWiFiConfg)
		goto dm_CheckEdcaTurbo_EXIT;
	
	// 1. We do not turn on EDCA turbo mode for some AP that has IOT issue
	// 2. User may disable EDCA Turbo mode with OID settings.
	if((pMgntInfo->IOTAction & HT_IOT_ACT_DISABLE_EDCA_TURBO) ||pHalData->bForcedDisableTurboEDCA)
		goto dm_CheckEdcaTurbo_EXIT;

#if (INTEL_PROXIMITY_SUPPORT == 1)
	if(pMgntInfo->IntelClassModeInfo.bEnableCA == TRUE)
	{
		EDCA_BE_UL = EDCA_BE_DL = 0xa44f;
	}
	else
#endif		
	{
		if((!pMgntInfo->bDisableFrameBursting) && 
			(pMgntInfo->IOTAction & (HT_IOT_ACT_FORCED_ENABLE_BE_TXOP|HT_IOT_ACT_AMSDU_ENABLE)))
		{// To check whether we shall force turn on TXOP configuration.
			if(!(EDCA_BE_UL & 0xffff0000))
				EDCA_BE_UL |= 0x005e0000; // Force TxOP limit to 0x005e for UL.
			if(!(EDCA_BE_DL & 0xffff0000))
				EDCA_BE_DL |= 0x005e0000; // Force TxOP limit to 0x005e for DL.
		}
		
		//92D txop can't be set to 0x3e for cisco1250
		if((!IS_HARDWARE_TYPE_8192D (Adapter)) && pMgntInfo->IOTPeer == HT_IOT_PEER_CISCO && IS_WIRELESS_MODE_N_24G(Adapter))
		{
			EDCA_BE_DL = edca_setting_DL[pMgntInfo->IOTPeer];
			EDCA_BE_UL = edca_setting_UL[pMgntInfo->IOTPeer];
		}
		else if(pMgntInfo->IOTPeer == HT_IOT_PEER_CISCO
			&& ( IS_WIRELESS_MODE_G(Adapter) ||IS_WIRELESS_MODE_B(Adapter) ||IS_WIRELESS_MODE_A(Adapter) ))
		{
			EDCA_BE_DL = edca_setting_DL_GMode[pMgntInfo->IOTPeer];
			RT_TRACE(COMP_TURBO,DBG_LOUD,("==>odm_CheckEdcaTurbo,HT_IOT_PEER_CISCO,G/B mode. set EDCA_BE_DL=0x%x\n ",EDCA_BE_DL));
		}
		else if(pMgntInfo->IOTPeer == HT_IOT_PEER_AIRGO && (IS_WIRELESS_MODE_G(Adapter) || IS_WIRELESS_MODE_A(Adapter)))
		{
			EDCA_BE_DL = 0xa630;
		}
		else if(pMgntInfo->IOTPeer == HT_IOT_PEER_MARVELL)
		{
			EDCA_BE_DL = edca_setting_DL[pMgntInfo->IOTPeer];
			EDCA_BE_UL = edca_setting_UL[pMgntInfo->IOTPeer];
		}
		else if(pMgntInfo->IOTPeer == HT_IOT_PEER_ATHEROS)
		{
			// Set DL EDCA for Atheros peer to 0x3ea42b. Suggested by SD3 Wilson for ASUS TP issue. 
			// 2011.11.28. by tynli.
			EDCA_BE_DL = edca_setting_DL[pMgntInfo->IOTPeer];
		}
		
	}

	if (pMgntInfo->IOTAction & HT_IOT_ACT_DISABLE_AC_TXOP)
	{
		//DbgPrint("HT_IOT_ACT_DISABLE_AC_TXOP return\n");
		return;		// Disable EDCA turbo for WNDAP 4500 broadcom 3T3R AP.
	}

	// Check if the status needs to be changed.
	if((!pHalData->bIsAnyNonBEPkts) && (!pMgntInfo->bDisableFrameBursting))
	{
		RTPRINT(FDM, DM_EDCA_Turbo, ("Turn on Turbo EDCA \n"));
		//
		// Turn On EDCA turbo here. 
		// In this point, 2 condition needs to be checked:
		// (1) What peer STA do we link to.
		// (2) Check Tx/Rx count to determine if it is in uplink/downlink.
		// Use specific EDCA parameter for each different combination.
		//
		curTxOkCnt = Adapter->TxStats.NumTxBytesUnicast - pMgntInfo->lastTxOkCnt;
		curRxOkCnt = Adapter->RxStats.NumRxBytesUnicast - pMgntInfo->lastRxOkCnt;
		Ext_curTxOkCnt = pExtAdapter->TxStats.NumTxBytesUnicast - pMgntInfo->Ext_lastTxOkCnt;
		Ext_curRxOkCnt = pExtAdapter->RxStats.NumRxBytesUnicast - pMgntInfo->Ext_lastRxOkCnt;
		RT_TRACE(COMP_TURBO,DBG_TRACE,("[Def]curTxOkCnt=%"i64fmt"d.",curTxOkCnt));
		RT_TRACE(COMP_TURBO,DBG_TRACE,("curRxOkCnt=%"i64fmt"d.\n",curRxOkCnt));
		RT_TRACE(COMP_TURBO,DBG_TRACE,("[Ext]curTxOkCnt=%"i64fmt"d.",Ext_curTxOkCnt));
		RT_TRACE(COMP_TURBO,DBG_TRACE,("curRxOkCnt=%"i64fmt"d.\n",Ext_curRxOkCnt));

		//For future Win7  Enable Default Port to modify AMPDU size dynamically, 2009.08.20, Bohn.
		{
			u1Byte TwoPortStatus = (u1Byte)TWO_PORT_STATUS__WITHOUT_ANY_ASSOCIATE;
			GetTwoPortSharedResource(Adapter,TWO_PORT_SHARED_OBJECT__STATUS,NULL,&TwoPortStatus);
			if(TwoPortStatus == TWO_PORT_STATUS__EXTENSION_ONLY)
			{
				curTxOkCnt = Ext_curTxOkCnt ;
				curRxOkCnt = Ext_curRxOkCnt ;
			}
		}

		// Modify EDCA parameters selection bias
		// For some APs, use downlink EDCA parameters for uplink+downlink 
		if(pMgntInfo->IOTAction & HT_IOT_ACT_EDCA_BIAS_ON_RX)
		{
			RTPRINT(FDM, DM_EDCA_Turbo, ("EDCA IOT state : BIAS on Rx\n"));
			if(curTxOkCnt > 4*curRxOkCnt)
			{// Uplink TP is present.
				RTPRINT(FDM, DM_EDCA_Turbo, ("EDCA Current Uplink state\n"));
				if(pHalData->bIsCurRDLState || !pHalData->bCurrentTurboEDCA)
				{
					PlatformEFIOWrite4Byte(Adapter, REG_EDCA_BE_PARAM, EDCA_BE_UL);
					pHalData->bIsCurRDLState = FALSE;
				}
			}
			else
			{// Balance TP is present.
				RTPRINT(FDM, DM_EDCA_Turbo, ("EDCA Current Balance state\n"));
				if(!pHalData->bIsCurRDLState || !pHalData->bCurrentTurboEDCA)
				{
					PlatformEFIOWrite4Byte(Adapter, REG_EDCA_BE_PARAM, EDCA_BE_DL);
					pHalData->bIsCurRDLState = TRUE;
				}
			}
			pHalData->bCurrentTurboEDCA = TRUE;
		}
		else
		{// For generic IOT Action.
			RTPRINT(FDM, DM_EDCA_Turbo, ("EDCA IOT state : Generic\n"));
			if(curRxOkCnt > 4*curTxOkCnt)
			{// Downlink TP is present.
				RTPRINT(FDM, DM_EDCA_Turbo, ("EDCA Current Downlink state\n"));
				if(!pHalData->bIsCurRDLState || !pHalData->bCurrentTurboEDCA)
				{
					PlatformEFIOWrite4Byte(Adapter, REG_EDCA_BE_PARAM, EDCA_BE_DL);
					pHalData->bIsCurRDLState = TRUE;
				}
			}
			else
			{// Balance TP is present.
				RTPRINT(FDM, DM_EDCA_Turbo, ("EDCA Current Balance state\n"));
				if(pHalData->bIsCurRDLState || !pHalData->bCurrentTurboEDCA)
				{
					PlatformEFIOWrite4Byte(Adapter, REG_EDCA_BE_PARAM, EDCA_BE_UL);
					pHalData->bIsCurRDLState = FALSE;
				}
			}
			pHalData->bCurrentTurboEDCA = TRUE;
		}
	}
	else
	{
		RTPRINT(FDM, DM_EDCA_Turbo, ("Turn off Turbo EDCA \n"));
		//
		// Turn Off EDCA turbo here.
		// Restore original EDCA according to the declaration of AP.
		//
		 if(pHalData->bCurrentTurboEDCA)
		{
			Adapter->HalFunc.SetHwRegHandler(Adapter, HW_VAR_AC_PARAM, GET_WMM_PARAM_ELE_SINGLE_AC_PARAM(pStaQos->WMMParamEle, AC0_BE) );
			pHalData->bCurrentTurboEDCA = FALSE;
		}
	}

dm_CheckEdcaTurbo_EXIT:
	// Set variables for next time.
	pHalData->bIsAnyNonBEPkts = FALSE;
	pMgntInfo->lastTxOkCnt = Adapter->TxStats.NumTxBytesUnicast;
	pMgntInfo->lastRxOkCnt = Adapter->RxStats.NumRxBytesUnicast;
	pMgntInfo->Ext_lastTxOkCnt = pExtAdapter->TxStats.NumTxBytesUnicast;
	pMgntInfo->Ext_lastRxOkCnt = pExtAdapter->RxStats.NumRxBytesUnicast;
}	// odm_CheckEdcaTurbo


//============================================================

//#pragma mark -- Tx Power Tracking --
//3============================================================
//3 Tx Power Tracking
//3============================================================


static	VOID
odm_InitializeTXPowerTracking_ThermalMeter(
	IN	PADAPTER		Adapter)
{
	PMGNT_INFO      	pMgntInfo = &Adapter->MgntInfo;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	pMgntInfo->bTXPowerTracking = TRUE;
	pHalData->TXPowercount       = 0;
	pHalData->bTXPowerTrackingInit = FALSE;
#if	MP_DRIVER != 1					//for mp driver, turn off txpwrtracking as default
	if(!IS_HARDWARE_TYPE_8188E(Adapter))		
		pHalData->TxPowerTrackControl = TRUE;		
#endif
	RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD, ("pMgntInfo->bTXPowerTracking = %d\n", pMgntInfo->bTXPowerTracking));
}


VOID
odm_InitializeTXPowerTracking(
	IN	PADAPTER		Adapter)
{
	odm_InitializeTXPowerTracking_ThermalMeter(Adapter);
}


VOID
odm_CheckTXPowerTracking_ThermalMeter(
	IN	PADAPTER		Adapter)
{
#if (HAL_CODE_BASE==RTL8192_C)
	PMGNT_INFO      		pMgntInfo = &Adapter->MgntInfo;
	//HAL_DATA_TYPE			*pHalData = GET_HAL_DATA(Adapter);
	static u1Byte			TM_Trigger = 0;
	//u1Byte					TxPowerCheckCnt = 5;	//10 sec

	if(!pMgntInfo->bTXPowerTracking /*|| (!pHalData->TxPowerTrackControl && pHalData->bAPKdone)*/)
	{
		return;
	}

	if(!TM_Trigger)		//at least delay 1 sec
	{
		if(IS_HARDWARE_TYPE_8192D(Adapter))
			PHY_SetRFReg(Adapter, RF_PATH_A, RF_T_METER_92D, BIT17 | BIT16, 0x03);
		else if(IS_HARDWARE_TYPE_8188E(Adapter))
			PHY_SetRFReg(Adapter, RF_PATH_A, RF_T_METER_88E, BIT17 | BIT16, 0x03);
		else if(IS_HARDWARE_TYPE_8812(Adapter))
			PHY_SetRFReg(Adapter, RF_PATH_A, RF_T_METER_88E, BIT16, 0x01);
		else
			PHY_SetRFReg(Adapter, RF_PATH_A, RF_T_METER, bRFRegOffsetMask, 0x60);
		RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD,("Trigger 92C Thermal Meter!!\n"));
		
		TM_Trigger = 1;
		return;
	}
	else
	{
		RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD,("Schedule TxPowerTracking direct call!!\n"));		
		odm_TXPowerTrackingDirectCall(Adapter); //Using direct call is instead, added by Roger, 2009.06.18.
		TM_Trigger = 0;
	}
#endif
}


//
// 2011/07/26 MH Add an API for testing IQK fail case.
//
BOOLEAN
ODM_CheckPowerStatus(
	IN	PADAPTER		Adapter)
{
	RT_RF_POWER_STATE 	rtState;
	PMGNT_INFO			pMgntInfo	= &(Adapter->MgntInfo);

	// 2011/07/27 MH We are not testing ready~~!! We may fail to get correct value when init sequence.
	if (pMgntInfo->init_adpt_in_progress == TRUE)
	{
		RT_TRACE(COMP_INIT, DBG_LOUD, ("ODM_CheckPowerStatus Return TRUE, due to initadapter"));
		return	TRUE;
	}
	
	//
	//	2011/07/19 MH We can not execute tx pwoer tracking/ LLC calibrate or IQK.
	//
	Adapter->HalFunc.GetHwRegHandler(Adapter, HW_VAR_RF_STATE, (pu1Byte)(&rtState));	
	if(Adapter->bDriverStopped || Adapter->bDriverIsGoingToPnpSetPowerSleep || rtState == eRfOff)
	{
		RT_TRACE(COMP_INIT, DBG_LOUD, ("ODM_CheckPowerStatus Return FALSE, due to %d/%d/%d\n", 
		Adapter->bDriverStopped, Adapter->bDriverIsGoingToPnpSetPowerSleep, rtState));
		return	FALSE;
	}

	return	TRUE;
}


VOID
ODM_CheckTXPowerTracking(
	IN	PADAPTER		Adapter)
{
	if (ODM_CheckPowerStatus(Adapter) == FALSE)
		return;
	
	odm_CheckTXPowerTracking_ThermalMeter(Adapter);
}	
//============================================================

//#pragma mark --Rate Adaptive --
//3============================================================
//3 Rate Adaptive
//3============================================================

VOID
odm_InitRateAdaptiveMask(
	IN	PADAPTER	Adapter	
	)
{
	PMGNT_INFO      			pMgntInfo = &Adapter->MgntInfo;
	PRATE_ADAPTIVE			pRA = (PRATE_ADAPTIVE)&pMgntInfo->RateAdaptive;

	pRA->RATRState = DM_RATR_STA_INIT;
	pRA->PreRATRState = DM_RATR_STA_INIT;

	if (pMgntInfo->DM_Type == DM_Type_ByDriver)
		pMgntInfo->bUseRAMask = TRUE;
	else
		pMgntInfo->bUseRAMask = FALSE;	
}

VOID
ODM_ApInitRateAdaptiveState(
	IN	PADAPTER	Adapter	,
	IN	PRT_WLAN_STA  pEntry
	)
{
	PRATE_ADAPTIVE	pRA = (PRATE_ADAPTIVE)&pEntry->RateAdaptive;

	pRA->RATRState = DM_RATR_STA_INIT;
	pRA->PreRATRState = DM_RATR_STA_INIT;
}

/*-----------------------------------------------------------------------------
 * Function:	odm_RefreshRateAdaptiveMask()
 *
 * Overview:	Update rate table mask according to rssi
 *
 * Input:		NONE
 *
 * Output:		NONE
 *
 * Return:		NONE
 *
 * Revised History:
 *	When		Who		Remark
 *	05/27/2009	hpfan	Create Version 0.  
 *
 *---------------------------------------------------------------------------*/
VOID
odm_RefreshRateAdaptiveMask(	IN	PADAPTER	pAdapter	)
{
	PADAPTER 				pTargetAdapter = NULL;
	HAL_DATA_TYPE			*pHalData = GET_HAL_DATA(pAdapter);
	PMGNT_INFO				pMgntInfo = GetDefaultMgntInfo(pAdapter);
	PRATE_ADAPTIVE			pRA = (PRATE_ADAPTIVE)&pMgntInfo->RateAdaptive;
	u4Byte					LowRSSIThreshForRA = 0, HighRSSIThreshForRA = 0;

	if(pAdapter->bDriverStopped)
	{
		RT_TRACE(COMP_RATR, DBG_TRACE, ("<---- odm_RefreshRateAdaptiveMask(): driver is going to unload\n"));
		return;
	}

	if(!pMgntInfo->bUseRAMask)
	{
		RT_TRACE(COMP_RATR, DBG_LOUD, ("<---- odm_RefreshRateAdaptiveMask(): driver does not control rate adaptive mask\n"));
		return;
	}

	// if default port is connected, update RA table for default port (infrastructure mode only)
	if(pAdapter->MgntInfo.mAssoc && (!ACTING_AS_AP(pAdapter)))
	{
		
		// decide rastate according to rssi
		switch (pRA->PreRATRState)
		{
			case DM_RATR_STA_HIGH:
				HighRSSIThreshForRA = 50;
				LowRSSIThreshForRA = 20;
				break;
			
			case DM_RATR_STA_MIDDLE:
				HighRSSIThreshForRA = 55;
				LowRSSIThreshForRA = 20;
				break;
			
			case DM_RATR_STA_LOW:
				HighRSSIThreshForRA = 50;
				LowRSSIThreshForRA = 25;
				break;

			default:
				HighRSSIThreshForRA = 50;
				LowRSSIThreshForRA = 20;
				break;
		}

		if(pHalData->UndecoratedSmoothedPWDB > (s4Byte)HighRSSIThreshForRA)
			pRA->RATRState = DM_RATR_STA_HIGH;
		else if(pHalData->UndecoratedSmoothedPWDB > (s4Byte)LowRSSIThreshForRA)
			pRA->RATRState = DM_RATR_STA_MIDDLE;
		else
			pRA->RATRState = DM_RATR_STA_LOW;

		if((pRA->PreRATRState != pRA->RATRState)||pMgntInfo->bSetTXPowerTrainingByOid)
		{
			RT_PRINT_ADDR(COMP_RATR, DBG_LOUD, ("Target AP addr : "), pMgntInfo->Bssid);
			RT_TRACE(COMP_RATR, DBG_LOUD, ("RSSI = %d\n", pHalData->UndecoratedSmoothedPWDB));
			RT_TRACE(COMP_RATR, DBG_LOUD, ("RSSI_LEVEL = %d\n", pRA->RATRState));
			RT_TRACE(COMP_RATR, DBG_LOUD, ("PreState = %d, CurState = %d\n", pRA->PreRATRState, pRA->RATRState));
			pAdapter->HalFunc.UpdateHalRAMaskHandler(
									pAdapter,
									FALSE,
									0,
									NULL,
									NULL,
									pRA->RATRState,
									RAMask_Normal);
			pRA->PreRATRState = pRA->RATRState;
		}
	}

	//
	// The following part configure AP/VWifi/IBSS rate adaptive mask.
	//

	if(pMgntInfo->mIbss)
	{
		// Target: AP/IBSS peer.
		pTargetAdapter = GetDefaultAdapter(pAdapter);
	}
	else
	{

		pTargetAdapter = GetDefaultAdapter(pAdapter);
		while(pTargetAdapter != NULL)
		{
			if(ACTING_AS_AP(pTargetAdapter ))
				break;
			pTargetAdapter = GetNextExtAdapter(pTargetAdapter);		
		}

	}

	// if extension port (softap) is started, updaet RA table for more than one clients associate
	if(pTargetAdapter != NULL)
	{
		int	i;
		PRT_WLAN_STA	pEntry;
		PRATE_ADAPTIVE     pEntryRA;

		for(i = 0; i < ASSOCIATE_ENTRY_NUM; i++)
		{
			if(	pTargetAdapter->MgntInfo.AsocEntry[i].bUsed && pTargetAdapter->MgntInfo.AsocEntry[i].bAssociated)
			{
				pEntry = pTargetAdapter->MgntInfo.AsocEntry+i;
				pEntryRA = &pEntry->RateAdaptive;

				switch (pEntryRA->PreRATRState)
				{
					case DM_RATR_STA_HIGH:
					{
						HighRSSIThreshForRA = 50;
						LowRSSIThreshForRA = 20;
					}
					break;
					
					case DM_RATR_STA_MIDDLE:
					{
						HighRSSIThreshForRA = 55;
						LowRSSIThreshForRA = 20;
					}
					break;
					
					case DM_RATR_STA_LOW:
					{
						HighRSSIThreshForRA = 50;
						LowRSSIThreshForRA = 25;
					}
					break;

					default:
					{
						HighRSSIThreshForRA = 50;
						LowRSSIThreshForRA = 20;
					}
				}

				if(pEntry->rssi_stat.UndecoratedSmoothedPWDB > (s4Byte)HighRSSIThreshForRA)
					pEntryRA->RATRState = DM_RATR_STA_HIGH;
				else if(pEntry->rssi_stat.UndecoratedSmoothedPWDB > (s4Byte)LowRSSIThreshForRA)
					pEntryRA->RATRState = DM_RATR_STA_MIDDLE;
				else
					pEntryRA->RATRState = DM_RATR_STA_LOW;

				if((pEntryRA->PreRATRState != pEntryRA->RATRState)||pMgntInfo->bSetTXPowerTrainingByOid)
				{
					RT_PRINT_ADDR(COMP_RATR, DBG_LOUD, ("AsocEntry addr : "), pEntry->MacAddr);
					RT_TRACE(COMP_RATR, DBG_LOUD, ("RSSI = %d\n", pEntry->rssi_stat.UndecoratedSmoothedPWDB));
					RT_TRACE(COMP_RATR, DBG_LOUD, ("RSSI_LEVEL = %d\n", pEntryRA->RATRState));
					RT_TRACE(COMP_RATR, DBG_LOUD, ("PreState = %d, CurState = %d\n", pEntryRA->PreRATRState, pEntryRA->RATRState));
					pAdapter->HalFunc.UpdateHalRAMaskHandler(
											pTargetAdapter,
											FALSE,
											pEntry->AID+1,
											pEntry->MacAddr,
											pEntry,
											pEntryRA->RATRState,
											RAMask_Normal);
					pEntryRA->PreRATRState = pEntryRA->RATRState;
				}

			}
		}
	}

	if(pMgntInfo->bSetTXPowerTrainingByOid)
		pMgntInfo->bSetTXPowerTrainingByOid = FALSE;	
}

//============================================================

//#pragma mark --BB Power Save --
//3============================================================
//3 BB Power Save
//3============================================================
void odm_InitDynamicBBPowerSaving(
	IN	PADAPTER	Adapter
	)
{
	pPS_T	pDM_PSTable = &Adapter->DM_PSTable;

	pDM_PSTable->PreCCAState = CCA_MAX;
	pDM_PSTable->CurCCAState = CCA_MAX;
	pDM_PSTable->PreRFState = RF_MAX;
	pDM_PSTable->CurRFState = RF_MAX;
	pDM_PSTable->Rssi_val_min = 0;
}


void
odm_DynamicBBPowerSaving(
IN	PADAPTER	pAdapter
	)
{	

	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	PMGNT_INFO	pMgntInfo = &(pAdapter->MgntInfo);
	pPS_T	pDM_PSTable = &pAdapter->DM_PSTable;

	//1 1.Determine the minimum RSSI 
	if((!pMgntInfo->bMediaConnect) &&	
		(pHalData->EntryMinUndecoratedSmoothedPWDB == 0))
	{
		pDM_PSTable->Rssi_val_min = 0;
		RT_TRACE(COMP_BB_POWERSAVING, DBG_LOUD, ("Not connected to any \n"));
	}
	if(pMgntInfo->bMediaConnect)	// Default port
	{
		if(ACTING_AS_AP(pAdapter) || pMgntInfo->mIbss)
		{
			pDM_PSTable->Rssi_val_min = pHalData->EntryMinUndecoratedSmoothedPWDB;
			RT_TRACE(COMP_BB_POWERSAVING, DBG_LOUD, ("AP Client PWDB = 0x%x \n", pDM_PSTable->Rssi_val_min));
		}
		else
		{
			pDM_PSTable->Rssi_val_min = pHalData->UndecoratedSmoothedPWDB;
			RT_TRACE(COMP_BB_POWERSAVING, DBG_LOUD, ("STA Default Port PWDB = 0x%x \n", pDM_PSTable->Rssi_val_min));
		}
	}
	else // associated entry pwdb
	{	
		pDM_PSTable->Rssi_val_min = pHalData->EntryMinUndecoratedSmoothedPWDB;
		RT_TRACE(COMP_BB_POWERSAVING, DBG_LOUD, ("AP Ext Port PWDB = 0x%x \n", pDM_PSTable->Rssi_val_min));
	}
	
	//1 2.Power Saving for 92C
	if(IS_92C_SERIAL(pHalData->VersionID))
	{
		odm_1R_CCA(pAdapter);
	}
	
	// 20100628 Joseph: Turn off BB power save for 88CE because it makesthroughput unstable.
	// 20100831 Joseph: Turn ON BB power save again after modifying AGC delay from 900ns ot 600ns.
	//1 3.Power Saving for 88C
	else
	{
		dm_RF_Saving(pAdapter, FALSE);
	}
}

//============================================================

//#pragma mark --Antenna Diversity --
//3============================================================
//3 Antenna Diversity
//3============================================================
//
// 20100514 Joseph: 
// Add new function to reset the state of antenna diversity before link.
//
VOID
ODM_SwAntDivResetBeforeLink(
	IN	PADAPTER	Adapter
	)
{
	pSWAT_T		pDM_SWAT_Table = &Adapter->DM_SWAT_Table;

	pDM_SWAT_Table->SWAS_NoLink_State = 0;
}

u1Byte
odm_SwAntDivSelectChkChnl(
	IN	PADAPTER	Adapter
	)
{
#if (RT_MEM_SIZE_LEVEL != RT_MEM_SIZE_MINIMUM)
	u1Byte 	index, target_chnl=0;
	u1Byte	chnl_peer_cnt[14] = {0};

	if(Adapter->MgntInfo.tmpNumBssDesc==0)
	{
		return 0;
	}
	else
	{		
		// 20100519 Joseph: Select checking channel from current scan list.
		// We just choose the channel with most APs to be the test scan channel.
		for(index=0; index<Adapter->MgntInfo.tmpNumBssDesc; index++)
		{
			// Add by hpfan: prevent access invalid channel number
			// TODO: Verify channel number by channel plan
			if(Adapter->MgntInfo.tmpbssDesc[index].ChannelNumber == 0 || 
				Adapter->MgntInfo.tmpbssDesc[index].ChannelNumber > 13)
				continue;
			
			chnl_peer_cnt[Adapter->MgntInfo.tmpbssDesc[index].ChannelNumber-1]++;
		}
		for(index=0; index<14; index++)
		{
			if(chnl_peer_cnt[index]>chnl_peer_cnt[target_chnl])
				target_chnl = index;
		}
		target_chnl+=1;
		RT_TRACE(COMP_SWAS, DBG_LOUD, 
			("odm_SwAntDivSelectChkChnl(): Channel %d is select as test channel.\n", target_chnl));

		return target_chnl;
	}
#else
	return	0;
#endif	
}

VOID
odm_SwAntDivConsructChkScanChnl(
	IN	PADAPTER	Adapter,
	IN	u1Byte		ChkChnl
	)
{
	PMGNT_INFO			pMgntInfo = &Adapter->MgntInfo;
	PRT_CHANNEL_LIST	pChannelList = GET_RT_CHANNEL_LIST(pMgntInfo);
	u1Byte				index;

	if(ChkChnl==0)
	{
		// 20100519 Joseph: Original antenna scanned nothing. 
		// Test antenna shall scan all channel with half period in this condition.
		RtActChannelList(Adapter, RT_CHNL_LIST_ACTION_CONSTRUCT_SCAN_LIST, NULL, NULL);
		for(index=0; index<pChannelList->ChannelLen; index++)
			pChannelList->ChannelInfo[index].ScanPeriod /= 2;
	}
	else
	{
		// The using of this CustomizedScanRequest is a trick to rescan the two channels 
		//	under the NORMAL scanning process. It will not affect MGNT_INFO.CustomizedScanRequest.
		CUSTOMIZED_SCAN_REQUEST CustomScanReq;

		CustomScanReq.bEnabled = TRUE;
		CustomScanReq.Channels[0] = ChkChnl;
		CustomScanReq.Channels[1] = pMgntInfo->dot11CurrentChannelNumber;
		CustomScanReq.nChannels = 2;
		CustomScanReq.ScanType = SCAN_ACTIVE;
		CustomScanReq.Duration = DEFAULT_ACTIVE_SCAN_PERIOD;

		RtActChannelList(Adapter, RT_CHNL_LIST_ACTION_CONSTRUCT_SCAN_LIST, &CustomScanReq, NULL);
	}
}


//
// 20100503 Joseph:
// Add new function SwAntDivCheck8192C().
// This is the main function of Antenna diversity function before link.
// Mainly, it just retains last scan result and scan again.
// After that, it compares the scan result to see which one gets better RSSI.
// It selects antenna with better receiving power and returns better scan result.
//

BOOLEAN
ODM_SwAntDivCheckBeforeLink(
	IN	PADAPTER	Adapter
	)
{
#if (RT_MEM_SIZE_LEVEL != RT_MEM_SIZE_MINIMUM)
	HAL_DATA_TYPE*	pHalData = GET_HAL_DATA(Adapter);
	PMGNT_INFO		pMgntInfo = &Adapter->MgntInfo;
	pSWAT_T		pDM_SWAT_Table = &Adapter->DM_SWAT_Table;

	s1Byte			Score = 0;
	PRT_WLAN_BSS	pTmpBssDesc;
	PRT_WLAN_BSS	pTestBssDesc;

	u1Byte			target_chnl = 0;
	u1Byte			index;

	// Condition that does not need to use antenna diversity.
	// 8723A need to use SW Ant
	if(IS_HARDWARE_TYPE_8192D(Adapter) ||IS_92C_SERIAL(pHalData->VersionID) ||
		(pHalData->AntDivCfg==0) || 
		pMgntInfo->AntennaTest )
	{
		RT_TRACE(COMP_SWAS, DBG_LOUD, 
				("ODM_SwAntDivCheckBeforeLink(): No AntDiv Mechanism.\n"));
		return FALSE;
	}

	//2 8723A may only one Ant
	if(IS_HARDWARE_TYPE_8723A(Adapter) && (pDM_SWAT_Table->ANTB_ON == FALSE))
	{
		RT_TRACE(COMP_SWAS, DBG_LOUD, 
				("ODM_SwAntDivCheckBeforeLink(): No AntDiv Mechanism, Antenna B is off\n"));
		return FALSE;
	}
	
	// Since driver is going to set BB register, it shall check if there is another thread controlling BB/RF.
	PlatformAcquireSpinLock(Adapter, RT_RF_STATE_SPINLOCK);
	if(pHalData->eRFPowerState!=eRfOn || pMgntInfo->RFChangeInProgress || pMgntInfo->bMediaConnect)
	{
		PlatformReleaseSpinLock(Adapter, RT_RF_STATE_SPINLOCK);
	
		RT_TRACE(COMP_SWAS, DBG_LOUD, 
				("ODM_SwAntDivCheckBeforeLink(): RFChangeInProgress(%x), eRFPowerState(%x)\n", 
				pMgntInfo->RFChangeInProgress,
				pHalData->eRFPowerState));
	
		pDM_SWAT_Table->SWAS_NoLink_State = 0;
		
		return FALSE;
	}
	else
	{
		PlatformReleaseSpinLock(Adapter, RT_RF_STATE_SPINLOCK);
	}

	//1 Run AntDiv mechanism "Before Link" part.
	if(pDM_SWAT_Table->SWAS_NoLink_State == 0)
	{
		//1 Prepare to do Scan again to check current antenna state.

		// Set check state to next step.
		pDM_SWAT_Table->SWAS_NoLink_State = 1;
	
		// Copy Current Scan list.
		Adapter->MgntInfo.tmpNumBssDesc = pMgntInfo->NumBssDesc;
		PlatformMoveMemory((PVOID)Adapter->MgntInfo.tmpbssDesc, (PVOID)pMgntInfo->bssDesc, sizeof(RT_WLAN_BSS)*MAX_BSS_DESC);
		
		// Switch Antenna to another one.
		pDM_SWAT_Table->PreAntenna = pDM_SWAT_Table->CurAntenna;
		pDM_SWAT_Table->CurAntenna = (pDM_SWAT_Table->CurAntenna==Antenna_A)?Antenna_B:Antenna_A;
		
		RT_TRACE(COMP_SWAS, DBG_LOUD, 
			("ODM_SwAntDivCheckBeforeLink(): Change to Ant(%s) for testing.\n", (pDM_SWAT_Table->CurAntenna==Antenna_A)?"A":"B"));
		//PHY_SetBBReg(Adapter, rFPGA0_XA_RFInterfaceOE, 0x300, DM_SWAT_Table.CurAntenna);
		pDM_SWAT_Table->SWAS_NoLink_BK_Reg860 = ((pDM_SWAT_Table->SWAS_NoLink_BK_Reg860 & 0xfffffcff) | (pDM_SWAT_Table->CurAntenna<<8));
		PHY_SetBBReg(Adapter, rFPGA0_XA_RFInterfaceOE, bMaskDWord, pDM_SWAT_Table->SWAS_NoLink_BK_Reg860);

		// Go back to scan function again.
		RT_TRACE(COMP_SWAS, DBG_LOUD, ("ODM_SwAntDivCheckBeforeLink(): Scan one more time\n"));
		pMgntInfo->ScanStep=0;
		target_chnl = odm_SwAntDivSelectChkChnl(Adapter);
		odm_SwAntDivConsructChkScanChnl(Adapter, target_chnl);
		HTReleaseChnlOpLock(Adapter);
		PlatformSetTimer(Adapter, &pMgntInfo->ScanTimer, 5);

		return TRUE;
	}
	else
	{
		//1 ScanComple() is called after antenna swiched.
		//1 Check scan result and determine which antenna is going
		//1 to be used.

		for(index=0; index<Adapter->MgntInfo.tmpNumBssDesc; index++)
		{
			pTmpBssDesc = &(Adapter->MgntInfo.tmpbssDesc[index]);
			pTestBssDesc = &(pMgntInfo->bssDesc[index]);

			if(PlatformCompareMemory(pTestBssDesc->bdBssIdBuf, pTmpBssDesc->bdBssIdBuf, 6)!=0)
			{
				RT_TRACE(COMP_SWAS, DBG_LOUD, ("ODM_SwAntDivCheckBeforeLink(): ERROR!! This shall not happen.\n"));
				continue;
			}

			if(pTmpBssDesc->RecvSignalPower > pTestBssDesc->RecvSignalPower)
			{
				RT_TRACE(COMP_SWAS, DBG_LOUD, ("ODM_SwAntDivCheckBeforeLink8192C: Compare scan entry: Score++\n"));
				RT_PRINT_STR(COMP_SWAS, DBG_LOUD, "SSID: ", pTestBssDesc->bdSsIdBuf, pTestBssDesc->bdSsIdLen);
				RT_TRACE(COMP_SWAS, DBG_LOUD, ("Original: %d, Test: %d\n", pTmpBssDesc->RecvSignalPower, pTestBssDesc->RecvSignalPower));
			
				Score++;
				PlatformMoveMemory(pTestBssDesc, pTmpBssDesc, sizeof(RT_WLAN_BSS));
			}
			else if(pTmpBssDesc->RecvSignalPower < pTestBssDesc->RecvSignalPower)
			{
				RT_TRACE(COMP_SWAS, DBG_LOUD, ("ODM_SwAntDivCheckBeforeLink: Compare scan entry: Score--\n"));
				RT_PRINT_STR(COMP_SWAS, DBG_LOUD, "SSID: ", pTestBssDesc->bdSsIdBuf, pTestBssDesc->bdSsIdLen);
				RT_TRACE(COMP_SWAS, DBG_LOUD, ("Original: %d, Test: %d\n", pTmpBssDesc->RecvSignalPower, pTestBssDesc->RecvSignalPower));
				Score--;
			}

		}

		if(pMgntInfo->NumBssDesc!=0 && Score<=0)
		{
			RT_TRACE(COMP_SWAS, DBG_LOUD,
				("ODM_SwAntDivCheckBeforeLink(): Using Ant(%s)\n", (pDM_SWAT_Table->CurAntenna==Antenna_A)?"A":"B"));

			pDM_SWAT_Table->PreAntenna = pDM_SWAT_Table->CurAntenna;
		}
		else
		{
			RT_TRACE(COMP_SWAS, DBG_LOUD, 
				("ODM_SwAntDivCheckBeforeLink(): Remain Ant(%s)\n", (pDM_SWAT_Table->CurAntenna==Antenna_A)?"B":"A"));

			pDM_SWAT_Table->CurAntenna = pDM_SWAT_Table->PreAntenna;

			//PHY_SetBBReg(Adapter, rFPGA0_XA_RFInterfaceOE, 0x300, DM_SWAT_Table.CurAntenna);
			pDM_SWAT_Table->SWAS_NoLink_BK_Reg860 = ((pDM_SWAT_Table->SWAS_NoLink_BK_Reg860 & 0xfffffcff) | (pDM_SWAT_Table->CurAntenna<<8));
			PHY_SetBBReg(Adapter, rFPGA0_XA_RFInterfaceOE, bMaskDWord, pDM_SWAT_Table->SWAS_NoLink_BK_Reg860);
		}

		// Check state reset to default and wait for next time.
		pDM_SWAT_Table->SWAS_NoLink_State = 0;

		return FALSE;
	}
#else
		return	FALSE;
#endif
	
}
//
// 20100514 Luke/Joseph:
// Add new function to reset antenna diversity state after link.
//
VOID
ODM_SwAntDivRestAfterLink(
	IN	PADAPTER	Adapter
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	pSWAT_T		pDM_SWAT_Table = &Adapter->DM_SWAT_Table;

	pHalData->RSSI_cnt_A = 0;
	pHalData->RSSI_cnt_B = 0;
	pHalData->RSSI_test = FALSE;
	pDM_SWAT_Table->try_flag = 0xff;
	pDM_SWAT_Table->RSSI_Trying = 0;
	pDM_SWAT_Table->SelectAntennaMap=0xAA;
}


//
// 20100514 Luke/Joseph:
// Add new function for antenna diversity after link.
// This is the main function of antenna diversity after link.
// This function is called in HalDmWatchDog() and ODM_SwAntDivChkAntSwitchCallback().
// HalDmWatchDog() calls this function with SWAW_STEP_PEAK to initialize the antenna test.
// In SWAW_STEP_PEAK, another antenna and a 500ms timer will be set for testing.
// After 500ms, ODM_SwAntDivChkAntSwitchCallback() calls this function to compare the signal just
// listened on the air with the RSSI of original antenna.
// It chooses the antenna with better RSSI.
// There is also a aged policy for error trying. Each error trying will cost more 5 seconds waiting 
// penalty to get next try.
//
VOID
odm_SwAntDivChkAntSwitch(
	PADAPTER		Adapter,
	u1Byte			Step
)
{
	PMGNT_INFO		pMgntInfo = &(Adapter->MgntInfo);
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	pSWAT_T		pDM_SWAT_Table = &Adapter->DM_SWAT_Table;
	s4Byte			curRSSI=100, RSSI_A, RSSI_B;
	u1Byte			nextAntenna=Antenna_B;
	static u8Byte		lastTxOkCnt=0, lastRxOkCnt=0;
	u8Byte			curTxOkCnt, curRxOkCnt;
	static u8Byte		TXByteCnt_A=0, TXByteCnt_B=0, RXByteCnt_A=0, RXByteCnt_B=0;
	u8Byte			CurByteCnt=0, PreByteCnt=0;
	static u1Byte		TrafficLoad = TRAFFIC_LOW;
	u1Byte			Score_A=0, Score_B=0;
	u1Byte			i;
	// sleep state for 8723
	u1Byte			CHK_A=0, CHK_B=0;
	static u1Byte		PeningCount=0;
	static u1Byte		NeedtoChk=1;
	static BOOLEAN	TryingOK=FALSE;

	// Condition that does not need to use antenna diversity.
	// 8723A need to use SW ANT
	if(IS_92C_SERIAL(pHalData->VersionID) ||(pHalData->AntDivCfg==0) || pMgntInfo->AntennaTest)
	{
		RT_TRACE(COMP_SWAS, DBG_LOUD, ("odm_SwAntDivChkAntSwitch(): No AntDiv Mechanism.\n"));
		return;
	}

	//2 8723A may only one ANT
	if(IS_HARDWARE_TYPE_8723A(Adapter) && 
		(pDM_SWAT_Table->ANTB_ON == FALSE))
	{
		RT_TRACE(COMP_SWAS, DBG_LOUD, 
				("odm_SwAntDivChkAntSwitch(): No AntDiv Mechanism, Antenna B is off\n"));
		return;
	}


	// Radio off: Status reset to default and return.
	if(pHalData->eRFPowerState==eRfOff)
	{
		ODM_SwAntDivRestAfterLink(Adapter);
		return;
	}


	// Handling step mismatch condition.
	// Peak step is not finished at last time. Recover the variable and check again.
	if(	Step != pDM_SWAT_Table->try_flag	)
	{
		ODM_SwAntDivRestAfterLink(Adapter);
	}

	//8723A Sleep State
	if(IS_HARDWARE_TYPE_8723A(Adapter))
	{
		if((PeningCount >0)&&(TryingOK==TRUE))
		{
			PeningCount--;
			NeedtoChk=0;
			//DbgPrint("SwAnt is in Sleeping State");
			return;
		}
	
       	// Add to Steady State to avoid busy traing
       	// to Check SelectAntennaMap first
       	if((NeedtoChk)&&(TryingOK==TRUE))
       	{
       		for(i=0;i<8;i++)
       		{
       			if(((pDM_SWAT_Table->SelectAntennaMap>>i)&BIT0) == 1)
					CHK_A++;
				else
					CHK_B++;
       		}
			if((CHK_A >=8)||(CHK_B>=8))
			{
				PeningCount=0x1E;
				//DbgPrint("SwAnt go into Sleeping State");
				return;
			}
	 	}
	 	else
	 	{
	 		NeedtoChk=1;
			//DbgPrint("SwAnt Out to Sleeping State");
	 	}  
	}

	if(pDM_SWAT_Table->try_flag == 0xff)
	{
		// Select RSSI checking target
		if(pMgntInfo->mAssoc && !ACTING_AS_AP(Adapter))
		{
			// Target: Infrastructure mode AP.
			pHalData->RSSI_target = NULL;
			RT_TRACE(COMP_SWAS, DBG_LOUD, ("odm_SwAntDivChkAntSwitch(): RSSI_target is DEF AP!\n"));
		}
		else
		{
			u1Byte			index = 0;
			PRT_WLAN_STA	pEntry = NULL;
			PADAPTER		pTargetAdapter = NULL;
		
			if(pMgntInfo->mIbss )
			{
				// Target: AP/IBSS peer.
				pTargetAdapter = Adapter;
			}
			else
			{
		
				pTargetAdapter = GetDefaultAdapter(Adapter);
				while(pTargetAdapter != NULL)
			{
					if(ACTING_AS_AP(pTargetAdapter ))
						break;
					pTargetAdapter = GetNextExtAdapter(pTargetAdapter);		
				}

			}

			if(pTargetAdapter != NULL)
			{
				for(index=0; index<ASSOCIATE_ENTRY_NUM; index++)
				{
					pEntry = AsocEntry_EnumStation(pTargetAdapter, index);
					if(pEntry != NULL)
					{
						if(pEntry->bAssociated)
							break;			
					}
				}
			}

			if(pEntry == NULL)
			{
				ODM_SwAntDivRestAfterLink(Adapter);
				RT_TRACE(COMP_SWAS, DBG_LOUD, ("odm_SwAntDivChkAntSwitch(): No Link.\n"));
				return;
			}
			else
			{
				pHalData->RSSI_target = pEntry;
				RT_TRACE(COMP_SWAS, DBG_LOUD, ("odm_SwAntDivChkAntSwitch(): RSSI_target is PEER STA\n"));
			}
		}
			

		pHalData->RSSI_cnt_A = 0;
		pHalData->RSSI_cnt_B = 0;
		pDM_SWAT_Table->try_flag = 0;
		RT_TRACE(COMP_SWAS, DBG_LOUD, ("odm_SwAntDivChkAntSwitch(): Set try_flag to 0 prepare for peak!\n"));
		return;
	}
	else
	{
		curTxOkCnt = Adapter->TxStats.NumTxBytesUnicast - lastTxOkCnt;
		curRxOkCnt = Adapter->RxStats.NumRxBytesUnicast - lastRxOkCnt;
		lastTxOkCnt = Adapter->TxStats.NumTxBytesUnicast;
		lastRxOkCnt = Adapter->RxStats.NumRxBytesUnicast;
		TryingOK=FALSE; //Neil  -----> for 8723A sleep State using
	
		if(pDM_SWAT_Table->try_flag == 1)
		{
			if(pDM_SWAT_Table->CurAntenna == Antenna_A)
			{
				TXByteCnt_A += curTxOkCnt;
				RXByteCnt_A += curRxOkCnt;
			}
			else
			{
				TXByteCnt_B += curTxOkCnt;
				RXByteCnt_B += curRxOkCnt;
			}
		
			nextAntenna = (pDM_SWAT_Table->CurAntenna == Antenna_A)? Antenna_B : Antenna_A;
			pDM_SWAT_Table->RSSI_Trying--;
			RT_TRACE(COMP_SWAS, DBG_LOUD, ("RSSI_Trying = %d\n",pDM_SWAT_Table->RSSI_Trying));
			if(pDM_SWAT_Table->RSSI_Trying == 0)
			{
				CurByteCnt = (pDM_SWAT_Table->CurAntenna == Antenna_A)? (TXByteCnt_A+RXByteCnt_A) : (TXByteCnt_B+RXByteCnt_B);
				PreByteCnt = (pDM_SWAT_Table->CurAntenna == Antenna_A)? (TXByteCnt_B+RXByteCnt_B) : (TXByteCnt_A+RXByteCnt_A);
				
				if(TrafficLoad == TRAFFIC_HIGH)
					CurByteCnt = PlatformDivision64(CurByteCnt, 9);
				else if(TrafficLoad == TRAFFIC_LOW)
					CurByteCnt = PlatformDivision64(CurByteCnt, 2);

				if(pHalData->RSSI_cnt_A > 0)
					RSSI_A = pHalData->RSSI_sum_A/pHalData->RSSI_cnt_A; 
				else
					RSSI_A = 0;
				if(pHalData->RSSI_cnt_B > 0)
					RSSI_B = pHalData->RSSI_sum_B/pHalData->RSSI_cnt_B; 
		else
					RSSI_B = 0;
				curRSSI = (pDM_SWAT_Table->CurAntenna == Antenna_A)? RSSI_A : RSSI_B;
				pDM_SWAT_Table->PreRSSI =  (pDM_SWAT_Table->CurAntenna == Antenna_A)? RSSI_B : RSSI_A;
				RT_TRACE(COMP_SWAS, DBG_LOUD, ("Luke:PreRSSI = %d, CurRSSI = %d\n",pDM_SWAT_Table->PreRSSI, curRSSI));
				RT_TRACE(COMP_SWAS, DBG_LOUD, ("SWAS: preAntenna= %s, curAntenna= %s \n", 
				(pDM_SWAT_Table->PreAntenna == Antenna_A?"A":"B"), (pDM_SWAT_Table->CurAntenna == Antenna_A?"A":"B")));
				RT_TRACE(COMP_SWAS, DBG_LOUD, ("Luke:RSSI_A= %d, RSSI_cnt_A = %d, RSSI_B= %d, RSSI_cnt_B = %d\n",
					RSSI_A, pHalData->RSSI_cnt_A, RSSI_B, pHalData->RSSI_cnt_B));
			}

		}
		else
		{
		
			if(pHalData->RSSI_cnt_A > 0)
				RSSI_A = pHalData->RSSI_sum_A/pHalData->RSSI_cnt_A; 
			else
				RSSI_A = 0;
			if(pHalData->RSSI_cnt_B > 0)
				RSSI_B = pHalData->RSSI_sum_B/pHalData->RSSI_cnt_B; 
			else
				RSSI_B = 0;
			curRSSI = (pDM_SWAT_Table->CurAntenna == Antenna_A)? RSSI_A : RSSI_B;
			pDM_SWAT_Table->PreRSSI =  (pDM_SWAT_Table->PreAntenna == Antenna_A)? RSSI_A : RSSI_B;
			RT_TRACE(COMP_SWAS, DBG_LOUD, ("Ekul:PreRSSI = %d, CurRSSI = %d\n", pDM_SWAT_Table->PreRSSI, curRSSI));
		RT_TRACE(COMP_SWAS, DBG_LOUD, ("SWAS: preAntenna= %s, curAntenna= %s \n", 
			(pDM_SWAT_Table->PreAntenna == Antenna_A?"A":"B"), (pDM_SWAT_Table->CurAntenna == Antenna_A?"A":"B")));

			RT_TRACE(COMP_SWAS, DBG_LOUD, ("Ekul:RSSI_A= %d, RSSI_cnt_A = %d, RSSI_B= %d, RSSI_cnt_B = %d\n",
				RSSI_A, pHalData->RSSI_cnt_A, RSSI_B, pHalData->RSSI_cnt_B));
			//RT_TRACE(COMP_SWAS, DBG_LOUD, ("Ekul:curTxOkCnt = %d\n", curTxOkCnt));
			//RT_TRACE(COMP_SWAS, DBG_LOUD, ("Ekul:curRxOkCnt = %d\n", curRxOkCnt));
		}

		//1 Trying State
		if((pDM_SWAT_Table->try_flag == 1)&&(pDM_SWAT_Table->RSSI_Trying == 0))
		{

			if(pDM_SWAT_Table->TestMode == TP_MODE)
			{
				RT_TRACE(COMP_SWAS, DBG_LOUD, ("SWAS: TestMode = TP_MODE"));
				RT_TRACE(COMP_SWAS, DBG_LOUD, ("TRY:CurByteCnt = %"i64fmt"d,", CurByteCnt));
				RT_TRACE(COMP_SWAS, DBG_LOUD, ("TRY:PreByteCnt = %"i64fmt"d\n",PreByteCnt));		
				if(CurByteCnt < PreByteCnt)
				{
					if(pDM_SWAT_Table->CurAntenna == Antenna_A)
						pDM_SWAT_Table->SelectAntennaMap=pDM_SWAT_Table->SelectAntennaMap<<1;
					else
						pDM_SWAT_Table->SelectAntennaMap=(pDM_SWAT_Table->SelectAntennaMap<<1)+1;
				}
				else
				{
					if(pDM_SWAT_Table->CurAntenna == Antenna_A)
						pDM_SWAT_Table->SelectAntennaMap=(pDM_SWAT_Table->SelectAntennaMap<<1)+1;
					else
						pDM_SWAT_Table->SelectAntennaMap=pDM_SWAT_Table->SelectAntennaMap<<1;
				}
				for (i= 0; i<8; i++)
				{
					if(((pDM_SWAT_Table->SelectAntennaMap>>i)&BIT0) == 1)
						Score_A++;
					else
						Score_B++;
				}
				RT_TRACE(COMP_SWAS, DBG_LOUD, ("SelectAntennaMap=%x\n ",pDM_SWAT_Table->SelectAntennaMap));
				RT_TRACE(COMP_SWAS, DBG_LOUD, ("Score_A=%d, Score_B=%d\n", Score_A, Score_B));
			
				if(pDM_SWAT_Table->CurAntenna == Antenna_A)
				{
					nextAntenna = (Score_A > Score_B)?Antenna_A:Antenna_B;
				}
				else
				{
					nextAntenna = (Score_B > Score_A)?Antenna_B:Antenna_A;
				}
				//RT_TRACE(COMP_SWAS, DBG_LOUD, ("nextAntenna=%s\n",(nextAntenna==Antenna_A)?"A":"B"));
				//RT_TRACE(COMP_SWAS, DBG_LOUD, ("preAntenna= %s, curAntenna= %s \n", 
				//(DM_SWAT_Table.PreAntenna == Antenna_A?"A":"B"), (DM_SWAT_Table.CurAntenna == Antenna_A?"A":"B")));

				if(nextAntenna != pDM_SWAT_Table->CurAntenna)
				{
					RT_TRACE(COMP_SWAS, DBG_LOUD, ("SWAS: Switch back to another antenna"));
				}
				else
				{
					RT_TRACE(COMP_SWAS, DBG_LOUD, ("SWAS: current anntena is good\n"));
				}	
				// for 8723A Sleep State using
				TryingOK=TRUE;
			}

			if(pDM_SWAT_Table->TestMode == RSSI_MODE)
			{	
				RT_TRACE(COMP_SWAS, DBG_LOUD, ("SWAS: TestMode = RSSI_MODE"));
				pDM_SWAT_Table->SelectAntennaMap=0xAA;
				if(curRSSI < pDM_SWAT_Table->PreRSSI) //Current antenna is worse than previous antenna
				{
					RT_TRACE(COMP_SWAS, DBG_LOUD, ("SWAS: Switch back to another antenna"));
					nextAntenna = (pDM_SWAT_Table->CurAntenna == Antenna_A)? Antenna_B : Antenna_A;
				}
				else // current anntena is good
				{
					nextAntenna =pDM_SWAT_Table->CurAntenna;
					RT_TRACE(COMP_SWAS, DBG_LOUD, ("SWAS: current anntena is good\n"));
				}
			}
			pDM_SWAT_Table->try_flag = 0;
			pHalData->RSSI_test = FALSE;
			pHalData->RSSI_sum_A = 0;
			pHalData->RSSI_cnt_A = 0;
			pHalData->RSSI_sum_B = 0;
			pHalData->RSSI_cnt_B = 0;
			TXByteCnt_A = 0;
			TXByteCnt_B = 0;
			RXByteCnt_A = 0;
			RXByteCnt_B = 0;
			
		}

		//1 Normal State
		else if(pDM_SWAT_Table->try_flag == 0)
		{
			if(TrafficLoad == TRAFFIC_HIGH)
			{
				if(PlatformDivision64(curTxOkCnt+curRxOkCnt, 2) > 1875000)
					TrafficLoad = TRAFFIC_HIGH;
				else
					TrafficLoad = TRAFFIC_LOW;
			}
			else if(TrafficLoad == TRAFFIC_LOW)
				{
				if(PlatformDivision64(curTxOkCnt+curRxOkCnt, 2) > 1875000)
					TrafficLoad = TRAFFIC_HIGH;
				else
					TrafficLoad = TRAFFIC_LOW;
			}
			if(TrafficLoad == TRAFFIC_HIGH)
				pDM_SWAT_Table->bTriggerAntennaSwitch = 0;
			//RT_TRACE(COMP_SWAS, DBG_LOUD, ("Normal:TrafficLoad = %llu\n", curTxOkCnt+curRxOkCnt));

			//Prepare To Try Antenna		
					nextAntenna = (pDM_SWAT_Table->CurAntenna == Antenna_A)? Antenna_B : Antenna_A;
					pDM_SWAT_Table->try_flag = 1;
					pHalData->RSSI_test = TRUE;
			if((curRxOkCnt+curTxOkCnt) > 1000)
			{
				pDM_SWAT_Table->RSSI_Trying = 4;
				pDM_SWAT_Table->TestMode = TP_MODE;
				}
				else
				{
				pDM_SWAT_Table->RSSI_Trying = 2;
				pDM_SWAT_Table->TestMode = RSSI_MODE;

			}
			RT_TRACE(COMP_SWAS, DBG_LOUD, ("SWAS: Normal State -> Begin Trying!\n"));
			
			
			pHalData->RSSI_sum_A = 0;
			pHalData->RSSI_cnt_A = 0;
			pHalData->RSSI_sum_B = 0;
			pHalData->RSSI_cnt_B = 0;
		}
	}

	//1 4.Change TRX antenna
	if(nextAntenna != pDM_SWAT_Table->CurAntenna)
	{
		RT_TRACE(COMP_SWAS, DBG_LOUD, ("SWAS: Change TX Antenna!\n "));
		PHY_SetBBReg(Adapter, rFPGA0_XA_RFInterfaceOE, 0x300, nextAntenna);
	}

	//1 5.Reset Statistics
	pDM_SWAT_Table->PreAntenna = pDM_SWAT_Table->CurAntenna;
	pDM_SWAT_Table->CurAntenna = nextAntenna;
	pDM_SWAT_Table->PreRSSI = curRSSI;

	//1 6.Set next timer

	if(pDM_SWAT_Table->RSSI_Trying == 0)
		return;

	if(pDM_SWAT_Table->RSSI_Trying%2 == 0)
	{
		if(pDM_SWAT_Table->TestMode == TP_MODE)
		{
			if(TrafficLoad == TRAFFIC_HIGH)
			{
				PlatformSetTimer( Adapter, &pHalData->SwAntennaSwitchTimer, 10 ); //ms
				RT_TRACE(COMP_SWAS, DBG_LOUD, ("dm_SW_AntennaSwitch(): Test another antenna for 10 ms\n"));
			}
			else if(TrafficLoad == TRAFFIC_LOW)
			{
				PlatformSetTimer( Adapter, &pHalData->SwAntennaSwitchTimer, 50 ); //ms
				RT_TRACE(COMP_SWAS, DBG_LOUD, ("dm_SW_AntennaSwitch(): Test another antenna for 50 ms\n"));
			}
		}
		else
		{
			PlatformSetTimer( Adapter, &pHalData->SwAntennaSwitchTimer, 500 ); //ms
			RT_TRACE(COMP_SWAS, DBG_LOUD, ("dm_SW_AntennaSwitch(): Test another antenna for 500 ms\n"));
		}
	}
	else
	{
		if(pDM_SWAT_Table->TestMode == TP_MODE)
		{
			if(TrafficLoad == TRAFFIC_HIGH)
				PlatformSetTimer( Adapter, &pHalData->SwAntennaSwitchTimer, 90 ); //ms
			else if(TrafficLoad == TRAFFIC_LOW)
				PlatformSetTimer( Adapter, &pHalData->SwAntennaSwitchTimer, 100 ); //ms
		}
		else
			PlatformSetTimer( Adapter, &pHalData->SwAntennaSwitchTimer, 500 ); //ms
	}
}

//
// 20100514 Luke/Joseph:
// Callback function for 500ms antenna test trying.
//
VOID
ODM_SwAntDivChkAntSwitchCallback(
	PRT_TIMER		pTimer
)
{
	PADAPTER		Adapter = (PADAPTER)pTimer->Adapter;
#if USE_WORKITEM
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
#endif
#if DEV_BUS_TYPE==RT_PCI_INTERFACE
#if USE_WORKITEM
	PlatformScheduleWorkItem(&pHalData->SwAntennaSwitchWorkitem);
#else
	odm_SwAntDivChkAntSwitch(Adapter, SWAW_STEP_DETERMINE);
#endif
#else
	PlatformScheduleWorkItem(&pHalData->SwAntennaSwitchWorkitem);
#endif
}

VOID
ODM_SwAntDivChkAntSwitchWorkitemCallback(
    IN PVOID            pContext
    )
{
	PADAPTER	pAdapter = (PADAPTER)pContext;

	if(pAdapter->bDriverStopped)
		return;
	odm_SwAntDivChkAntSwitch(pAdapter, SWAW_STEP_DETERMINE);
}

//
// 20100514 Luke/Joseph:
// This function is used to gather the RSSI information for antenna testing.
// It selects the RSSI of the peer STA that we want to know.
//
VOID
ODM_SwAntDivChkPerPktRssi(
	PADAPTER		Adapter,
	BOOLEAN			bIsDefPort,
	BOOLEAN			bMatchBSSID,
	PRT_WLAN_STA	pEntry,
	PRT_RFD			pRfd
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	BOOLEAN			bCount = FALSE;
	pSWAT_T		pDM_SWAT_Table = &Adapter->DM_SWAT_Table;

	if(pHalData->RSSI_target==NULL && bIsDefPort && bMatchBSSID)
		bCount = TRUE;
	else if(pHalData->RSSI_target!=NULL && pEntry!=NULL && pHalData->RSSI_target==pEntry)
		bCount = TRUE;

	if(bCount)
	{
		//1 RSSI for SW Antenna Switch
		if(pDM_SWAT_Table->CurAntenna == Antenna_A)
		{
			pHalData->RSSI_sum_A += pRfd->Status.RxPWDBAll;
			pHalData->RSSI_cnt_A++;
		}
		else
		{
			pHalData->RSSI_sum_B += pRfd->Status.RxPWDBAll;
			pHalData->RSSI_cnt_B++;

		}
	}
}


VOID
odm_SwAntDivInit(
	IN	PADAPTER	Adapter
	)
{
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	pSWAT_T		pDM_SWAT_Table = &Adapter->DM_SWAT_Table;

	RT_TRACE(COMP_SWAS, DBG_LOUD, ("SWAS:Init SW Antenna Switch\n"));
	pHalData->RSSI_sum_A = 0;
	pHalData->RSSI_cnt_A = 0;
	pHalData->RSSI_sum_B = 0;
	pHalData->RSSI_cnt_B = 0;
	pDM_SWAT_Table->CurAntenna = Antenna_A;
	pDM_SWAT_Table->PreAntenna = Antenna_A;
	pDM_SWAT_Table->try_flag = 0xff;
	pDM_SWAT_Table->PreRSSI = 0;
	pDM_SWAT_Table->SWAS_NoLink_State = 0;
	pDM_SWAT_Table->bTriggerAntennaSwitch = 0;
	pDM_SWAT_Table->SelectAntennaMap=0xAA;
	pDM_SWAT_Table->SWAS_NoLink_BK_Reg860 = PlatformEFIORead4Byte(Adapter, 0x860);
}
//============================================================

//#pragma mark --RSSI Monitor --

//3============================================================
//3 RSSI Monitor
//3============================================================
VOID
odm_RSSIMonitorInit(
	IN	PADAPTER	Adapter
)
{

}

VOID
odm_RSSIMonitorCheck(
	IN	PADAPTER	Adapter
)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	PRT_WLAN_STA	pEntry;
	u1Byte			i;
	s4Byte			tmpEntryMaxPWDB=0, tmpEntryMinPWDB=0xffff;

	RTPRINT(FDM, DM_PWDB, ("pHalData->UndecoratedSmoothedPWDB = 0x%x( %d)\n", 
		pHalData->UndecoratedSmoothedPWDB,
		pHalData->UndecoratedSmoothedPWDB));

	for(i = 0; i < ASSOCIATE_ENTRY_NUM; i++)
	{
		if(IsAPModeExist(Adapter)  && GetFirstExtAdapter(Adapter) != NULL)
		{
			pEntry = AsocEntry_EnumStation(GetFirstExtAdapter(Adapter), i);
		}
		else
		{
			pEntry = AsocEntry_EnumStation(GetDefaultAdapter(Adapter), i);
		}

		if(pEntry!=NULL)
		{
			if(pEntry->bAssociated)
			{
				RTPRINT_ADDR(FDM, DM_PWDB, ("pEntry->MacAddr ="), pEntry->MacAddr);
				RTPRINT(FDM, DM_PWDB, ("pEntry->rssi = 0x%x(%d)\n", 
					pEntry->rssi_stat.UndecoratedSmoothedPWDB,
					pEntry->rssi_stat.UndecoratedSmoothedPWDB));
				if(pEntry->rssi_stat.UndecoratedSmoothedPWDB < tmpEntryMinPWDB)
					tmpEntryMinPWDB = pEntry->rssi_stat.UndecoratedSmoothedPWDB;
				if(pEntry->rssi_stat.UndecoratedSmoothedPWDB > tmpEntryMaxPWDB)
					tmpEntryMaxPWDB = pEntry->rssi_stat.UndecoratedSmoothedPWDB;
			}
		}
		else
		{
			break;
		}
	}

	if(tmpEntryMaxPWDB != 0)	// If associated entry is found
	{
		pHalData->EntryMaxUndecoratedSmoothedPWDB = tmpEntryMaxPWDB;
		RTPRINT(FDM, DM_PWDB, ("EntryMaxPWDB = 0x%x(%d)\n", 
			tmpEntryMaxPWDB, tmpEntryMaxPWDB));
	}
	else
	{
		pHalData->EntryMaxUndecoratedSmoothedPWDB = 0;
	}
	if(tmpEntryMinPWDB != 0xffff) // If associated entry is found
	{
		pHalData->EntryMinUndecoratedSmoothedPWDB = tmpEntryMinPWDB;
		RTPRINT(FDM, DM_PWDB, ("EntryMinPWDB = 0x%x(%d)\n", 
					tmpEntryMinPWDB, tmpEntryMinPWDB));
	}
	else
	{
		pHalData->EntryMinUndecoratedSmoothedPWDB = 0;
	}

	// Indicate Rx signal strength to FW.
	if(Adapter->MgntInfo.bUseRAMask)
	{
		u1Byte	H2C_Parameter[3] ={0};
	//	DbgPrint("RxSS: %lx =%ld\n", pHalData->UndecoratedSmoothedPWDB, pHalData->UndecoratedSmoothedPWDB);
		H2C_Parameter[2] = (u1Byte)(pHalData->UndecoratedSmoothedPWDB & 0xFF);
		H2C_Parameter[1] = 0x20;   // fw v12 cmdid 5:use max macid ,for nic ,default macid is 0 ,max macid is 1
		
		FillH2CCmd(Adapter, H2C_RSSI_REPORT(Adapter), 3, H2C_Parameter);
	}
	else
	{
		PlatformEFIOWrite1Byte(Adapter, 0x4fe, (u1Byte)pHalData->UndecoratedSmoothedPWDB);
		//DbgPrint("0x4fe write %x %d\n", pHalData->UndecoratedSmoothedPWDB, pHalData->UndecoratedSmoothedPWDB);
	}
}

VOID
odm_Init_RSSIForDM(
	PADAPTER					Adapter
)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	pHalData->PacketMap = 0;
	pHalData->ValidBit = 0;
}

VOID
Process_RSSIForDM(
	PADAPTER					Adapter,
	PRT_RFD						pRfd,
	PRT_WLAN_STA				pEntry,
	pu1Byte						pDesc	
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	s4Byte			UndecoratedSmoothedPWDB, UndecoratedSmoothedCCK, UndecoratedSmoothedOFDM, RSSI_Ave;
	s4Byte			RxRSSIPercentage[2];
	u1Byte			isCCKrate=0;	
	u1Byte			RSSI_max, RSSI_min, i;
	u4Byte			OFDM_pkt=0; 
	u4Byte			Weighting=0;
	pPD_T	pDM_PDTable = &Adapter->DM_PDTable;
	isCCKrate = RX_HAL_IS_CCK_RATE(Adapter, pDesc);


	if(!pRfd->Status.bPacketMatchBSSID)
	{
		return;
	}

	if(pEntry)
	{
		UndecoratedSmoothedCCK = pEntry->rssi_stat.UndecoratedSmoothedCCK;
		UndecoratedSmoothedOFDM = pEntry->rssi_stat.UndecoratedSmoothedOFDM;
		UndecoratedSmoothedPWDB = pEntry->rssi_stat.UndecoratedSmoothedPWDB;
		for (i=0; i<2; i++)
		RxRSSIPercentage[i] = pEntry->rssi_stat.RxRSSIPercentage[i];
	}
	else
	{
		UndecoratedSmoothedCCK = pHalData->UndecoratedSmoothedCCK;
		UndecoratedSmoothedOFDM = pHalData->UndecoratedSmoothedOFDM;
		UndecoratedSmoothedPWDB = pHalData->UndecoratedSmoothedPWDB;
	}

	if(pRfd->Status.bPacketToSelf || pRfd->Status.bPacketBeacon)
	{

		if(!isCCKrate)
		{
			pDM_PDTable->OFDM_Pkt_Cnt++;
			if(pRfd->Status.RxMIMOSignalStrength[1] == 0)
				RSSI_Ave = 	pRfd->Status.RxMIMOSignalStrength[0];
			else
			{
				//DbgPrint("pRfd->Status.RxMIMOSignalStrength[0] = %d, pRfd->Status.RxMIMOSignalStrength[1] = %d \n", 
					//pRfd->Status.RxMIMOSignalStrength[0], pRfd->Status.RxMIMOSignalStrength[1]);

			
				if(pRfd->Status.RxMIMOSignalStrength[0] > pRfd->Status.RxMIMOSignalStrength[1])
				{
					RSSI_max = pRfd->Status.RxMIMOSignalStrength[0];
					RSSI_min = pRfd->Status.RxMIMOSignalStrength[1];
				}
				else
				{
					RSSI_max = pRfd->Status.RxMIMOSignalStrength[1];
					RSSI_min = pRfd->Status.RxMIMOSignalStrength[0];
				}
				if((RSSI_max -RSSI_min) < 3)
					RSSI_Ave = RSSI_max;
				else if((RSSI_max -RSSI_min) < 6)
					RSSI_Ave = RSSI_max - 1;
				else if((RSSI_max -RSSI_min) < 10)
					RSSI_Ave = RSSI_max - 2;
				else
					RSSI_Ave = RSSI_max - 3;
			}
					
			//1 Process OFDM RSSI
			if(UndecoratedSmoothedOFDM <= 0)	// initialize
			{
				UndecoratedSmoothedOFDM = pRfd->Status.RxPWDBAll;
			}
			else
			{
				if(pRfd->Status.RxPWDBAll > (u4Byte)UndecoratedSmoothedOFDM)
				{
					UndecoratedSmoothedOFDM = 	
							( ((UndecoratedSmoothedOFDM)*(Rx_Smooth_Factor-1)) + 
							(RSSI_Ave)) /(Rx_Smooth_Factor);
					UndecoratedSmoothedOFDM = UndecoratedSmoothedOFDM + 1;
				}
				else
				{
					UndecoratedSmoothedOFDM = 	
							( ((UndecoratedSmoothedOFDM)*(Rx_Smooth_Factor-1)) + 
							(RSSI_Ave)) /(Rx_Smooth_Factor);
				}
			}						
			
			if(pEntry)
			{
				pEntry->rssi_stat.PacketMap = (pEntry->rssi_stat.PacketMap<<1) | BIT0;
				pEntry->rssi_stat.OFDM_Pkt_Cnt++;
			}
			else
			{
				pHalData->PacketMap = (pHalData->PacketMap<<1) | BIT0;
				pHalData->OFDM_Pkt_Cnt++;
			}

			//2011.09.05 Luke Lee: Process two path RSSI for 92C path diversity
			if(pEntry)
			{
				//RT_TRACE(	COMP_DIG, DBG_LOUD, ("pEntry: Per Path RSSI\n"));
				for (i=0; i<2; i++)
				{
					if(RxRSSIPercentage[i] <= 0)	// initialize
					{
						RxRSSIPercentage[i] = pRfd->Status.RxMIMOSignalStrength[i];
					}
					else
					{
						if(pRfd->Status.RxMIMOSignalStrength[i] > (u4Byte) RxRSSIPercentage[i])
						{
							RxRSSIPercentage[i] = 	
									( ((RxRSSIPercentage[i])*(Rx_Smooth_Factor-1)) + 
									(pRfd->Status.RxMIMOSignalStrength[i])) /(Rx_Smooth_Factor);
							RxRSSIPercentage[i] = RxRSSIPercentage[i] + 1;
						}
						else
						{
							RxRSSIPercentage[i] = 	
									( ((RxRSSIPercentage[i])*(Rx_Smooth_Factor-1)) + 
									(pRfd->Status.RxMIMOSignalStrength[i])) /(Rx_Smooth_Factor);
						}
					}
					pEntry->rssi_stat.RxRSSIPercentage[i] = RxRSSIPercentage[i];
				}
			}
										
		}
		else
		{
			pDM_PDTable->CCK_Pkt_Cnt++;
			RSSI_Ave = pRfd->Status.RxPWDBAll;

			//1 Process CCK RSSI
			if(UndecoratedSmoothedCCK <= 0)	// initialize
			{
				UndecoratedSmoothedCCK = pRfd->Status.RxPWDBAll;
			}
			else
			{
				if(pRfd->Status.RxPWDBAll > (u4Byte)UndecoratedSmoothedCCK)
				{
					UndecoratedSmoothedCCK = 	
							( ((UndecoratedSmoothedCCK)*(Rx_Smooth_Factor-1)) + 
							(pRfd->Status.RxPWDBAll)) /(Rx_Smooth_Factor);
					UndecoratedSmoothedCCK = UndecoratedSmoothedCCK + 1;
				}
				else
				{
					UndecoratedSmoothedCCK = 	
							( ((UndecoratedSmoothedCCK)*(Rx_Smooth_Factor-1)) + 
							(pRfd->Status.RxPWDBAll)) /(Rx_Smooth_Factor);
				}
			}
			
			if(pEntry)
			{
				pEntry->rssi_stat.PacketMap = pEntry->rssi_stat.PacketMap<<1;
				pEntry->rssi_stat.CCK_Pkt_Cnt++;
			}
			else
			{
				pHalData->PacketMap = pHalData->PacketMap<<1;
				pHalData->CCK_Pkt_Cnt++;
			}
		}

		if(pEntry)
		{
			//2011.07.28 LukeLee: modified to prevent unstable CCK RSSI
			if(pEntry->rssi_stat.ValidBit >= 64)
				pEntry->rssi_stat.ValidBit = 64;
			else
				pEntry->rssi_stat.ValidBit++;
			for(i=0; i<pEntry->rssi_stat.ValidBit; i++)
				OFDM_pkt += (u1Byte)(pEntry->rssi_stat.PacketMap>>i)&BIT0;
			if(pHalData->ValidBit == 64)
			{
				Weighting = ((OFDM_pkt<<4) > 64)?64:(OFDM_pkt<<4);
				UndecoratedSmoothedPWDB = (Weighting*UndecoratedSmoothedOFDM+(64-Weighting)*UndecoratedSmoothedCCK)>>6;
			}
			else
			{
				if(pHalData->ValidBit != 0)
					UndecoratedSmoothedPWDB = (OFDM_pkt*UndecoratedSmoothedOFDM+(pHalData->ValidBit-OFDM_pkt)*UndecoratedSmoothedCCK)/pHalData->ValidBit;
				else
					UndecoratedSmoothedPWDB = 0;
			}

			pEntry->rssi_stat.UndecoratedSmoothedCCK = UndecoratedSmoothedCCK;
			pEntry->rssi_stat.UndecoratedSmoothedOFDM = UndecoratedSmoothedOFDM;
			pEntry->rssi_stat.UndecoratedSmoothedPWDB = UndecoratedSmoothedPWDB;
			//DbgPrint("OFDM_pkt=%d, Weighting=%d\n", OFDM_pkt, Weighting);
			//DbgPrint("UndecoratedSmoothedOFDM=%d, UndecoratedSmoothedPWDB=%d, UndecoratedSmoothedCCK=%d\n", 
			//	UndecoratedSmoothedOFDM, UndecoratedSmoothedPWDB, UndecoratedSmoothedCCK);
			
		}
		else
		{
			//2011.07.28 LukeLee: modified to prevent unstable CCK RSSI
			if(pHalData->ValidBit >= 64)
				pHalData->ValidBit = 64;
			else
				pHalData->ValidBit++;
			for(i=0; i<pHalData->ValidBit; i++)
				OFDM_pkt += (u1Byte)(pHalData->PacketMap>>i)&BIT0;
			if(pHalData->ValidBit == 64)
			{
				Weighting = ((OFDM_pkt<<4) > 64)?64:(OFDM_pkt<<4);
				UndecoratedSmoothedPWDB = (Weighting*UndecoratedSmoothedOFDM+(64-Weighting)*UndecoratedSmoothedCCK)>>6;
			}
			else
			{
				if(pHalData->ValidBit != 0)
					UndecoratedSmoothedPWDB = (OFDM_pkt*UndecoratedSmoothedOFDM+(pHalData->ValidBit-OFDM_pkt)*UndecoratedSmoothedCCK)/pHalData->ValidBit;
				else
					UndecoratedSmoothedPWDB = 0;
			}
			//DbgPrint("ValidBit = 0x%x\n",pHalData->ValidBit);
			//DbgPrint("OFDM_pkt=%d, Weighting=%d, UndecoratedSmoothedPWDB=%d\n", 
			//	OFDM_pkt, Weighting, UndecoratedSmoothedPWDB);
			pHalData->UndecoratedSmoothedCCK = UndecoratedSmoothedCCK;
			pHalData->UndecoratedSmoothedOFDM = UndecoratedSmoothedOFDM;
			pHalData->UndecoratedSmoothedPWDB = UndecoratedSmoothedPWDB;
		}
	
	}
}

//#pragma mark --PSD Monitor --
//3============================================================
//3 PSD Monitor
//3============================================================

VOID
odm_PSDMonitorInit(
	PADAPTER Adapter)
{
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);

	//PSD function is only applied in 2.4G in 92D -------add by Gary
	if(IS_HARDWARE_TYPE_8192D(Adapter))
	{
		if(pHalData->CurrentBandType92D != BAND_ON_2_4G)
			return;
	}
	//PSD Monitor Setting
	//Which path in ADC/DAC is turnned on for PSD: both I/Q
	PHY_SetBBReg(Adapter, 0x808, BIT10|BIT11, 0x3);
	//Ageraged number: 8
	PHY_SetBBReg(Adapter, 0x808, BIT12|BIT13, 0x1);
	pHalData->bPSDinProcess = FALSE;
	pHalData->bPSDactive = FALSE;
	pHalData->bBTTurnOff = FALSE;
	
	//Set Debug Port
	//PHY_SetBBReg(Adapter, 0x908, bMaskDWord, 0x803);
	//PHY_SetBBReg(Adapter, 0xB34, bMaskByte0, 0x00); // pause PSD
	//PHY_SetBBReg(Adapter, 0xB38, bMaskByte0, 10); //rescan
	//PHY_SetBBReg(Adapter, 0xB38, bMaskByte1, 0x32); // PSDDelay
	//PHY_SetBBReg(Adapter, 0xB38, bMaskByte2|bMaskByte3, 100); //interval

	//PlatformSetTimer( Adapter, &pHalData->PSDTriggerTimer, 0); //ms
	
}

u4Byte ConvertTo_dB(u4Byte Value)
{
	u1Byte i;
	u1Byte j;
	u4Byte dB;

	Value = Value & 0xFFFF;
	
	for (i=0;i<8;i++)
	{
		if (Value <= dB_Invert_Table[i][11])
		{
			break;
		}
	}

	if (i >= 8)
	{
		return (96);	// maximum 96 dB
	}

	for (j=0;j<12;j++)
	{
		if (Value <= dB_Invert_Table[i][j])
		{
			break;
		}
	}

	dB = i*12 + j + 1;

	return (dB);
}

VOID
odm_PSDMonitorCallback(
	PRT_TIMER		pTimer
)
{
#if USE_WORKITEM
       PADAPTER	Adapter = (PADAPTER)pTimer->Adapter;        //   Add by Gary
       HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
#else
	PADAPTER	Adapter = (PADAPTER)pTimer->Adapter;
#endif

#if USE_WORKITEM  &&  ( DEV_BUS_TYPE == RT_PCI_INTERFACE ||  DEV_BUS_TYPE == RT_USB_INTERFACE )
	PlatformScheduleWorkItem(&pHalData->PSDMonitorWorkitem);
#else
	odm_PSDMonitor(Adapter);
#endif
}

VOID
odm_PSDMonitorWorkItemCallback(
    IN PVOID            pContext
    )
{
	PADAPTER	Adapter = (PADAPTER)pContext;

	odm_PSDMonitor(Adapter);
}


VOID
ODM_PSDDbgControl(
	IN	PADAPTER	Adapter,
	IN	u4Byte		mode,
	IN	u4Byte		btRssi
	)
{
#if (DEV_BUS_TYPE == RT_PCI_INTERFACE)
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);

	RT_TRACE(COMP_PSD, DBG_LOUD, (" Monitor mode=%d, btRssi=%d\n", mode, btRssi));
	if(mode)
	{
		pHalData->RSSI_BT = (u1Byte)btRssi;
		pHalData->bUserAssignLevel = TRUE;
		PlatformSetTimer( Adapter, &pHalData->PSDTimer, 0); //ms		
	}
	else
	{
		PlatformCancelTimer(Adapter, &pHalData->PSDTimer);
	}
#endif
}


//#pragma mark --Export Interface --
//3============================================================
//3 Export Interface
//3============================================================
VOID
ODM_DMInit(
	IN	PADAPTER	Adapter
	)
{
	//same, vivi
  #if (DEV_BUS_TYPE == RT_PCI_INTERFACE)|(DEV_BUS_TYPE == RT_USB_INTERFACE)
  	odm_PSDMonitorInit(Adapter);

	if(IS_HARDWARE_TYPE_8192C(Adapter) || IS_HARDWARE_TYPE_8192D(Adapter) || IS_HARDWARE_TYPE_8723A(Adapter))
	  	odm_RXHPInit(Adapter);		// Add by Gary

 #endif
	odm_DIGInit(Adapter);
	odm_InitDynamicTxPower(Adapter);
	ODM_InitEdcaTurbo(Adapter);
	odm_InitRateAdaptiveMask(Adapter);		
	odm_InitializeTXPowerTracking(Adapter);
	odm_RSSIMonitorInit(Adapter);
	//different following
	if(!IS_HARDWARE_TYPE_8192D(Adapter))
	{
		odm_InitDynamicBBPowerSaving(Adapter);
		odm_SwAntDivInit(Adapter);	
	}
	odm_Init_RSSIForDM(Adapter);

	if(IS_HARDWARE_TYPE_8192C(Adapter) || IS_HARDWARE_TYPE_8192D(Adapter) || IS_HARDWARE_TYPE_8723A(Adapter))
		odm_PathDiversityInit_92C(Adapter);

}



#endif

