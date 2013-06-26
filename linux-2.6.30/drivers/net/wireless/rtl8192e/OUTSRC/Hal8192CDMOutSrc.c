//============================================================
// File Name: Hal8192CDMOutSrc.c 
//
// Description:
//
// This file is for 92CE/92CU outsource dynamic mechanism for partner.
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

//============================================================
// Function predefine.
//============================================================
VOID	odm_2TPathDiversityInit_92C(	IN	PADAPTER	Adapter);
VOID	odm_1TPathDiversityInit_92C(	IN	PADAPTER	Adapter);
BOOLEAN	odm_IsConnected_92C(IN	PADAPTER	Adapter);
VOID	odm_PathDiversityAfterLink_92C(	IN	PADAPTER	Adapter);
VOID	odm_SetRespPath_92C(		IN	PADAPTER	Adapter, 	IN	u1Byte	DefaultRespPath);
VOID	odm_OFDMTXPathDiversity_92C(	IN	PADAPTER	Adapter);
VOID	odm_CCKTXPathDiversity_92C(	IN	PADAPTER	Adapter);
VOID	odm_ResetPathDiversity_92C(		IN	PADAPTER	Adapter);


#if 0
VOID 
odm_FalseAlarmCounterStatistics_92C(
	IN	PADAPTER	Adapter
	)
{
	u4Byte ret_value;
	PFALSE_ALARM_STATISTICS FalseAlmCnt = &(Adapter->FalseAlmCnt);
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	PMGNT_INFO		pMgntInfo = &(Adapter->MgntInfo);
	u1Byte TxRate = PlatformEFIORead1Byte(Adapter, REG_INIDATA_RATE_SEL);
	u1Byte BBReset;
	
	ret_value = PHY_QueryBBReg(Adapter, rOFDM_PHYCounter1, bMaskDWord);
       FalseAlmCnt->Cnt_Parity_Fail = ((ret_value&0xffff0000)>>16);	

       ret_value = PHY_QueryBBReg(Adapter, rOFDM_PHYCounter2, bMaskDWord);
	FalseAlmCnt->Cnt_Rate_Illegal = (ret_value&0xffff);
	FalseAlmCnt->Cnt_Crc8_fail = ((ret_value&0xffff0000)>>16);
	ret_value = PHY_QueryBBReg(Adapter, rOFDM_PHYCounter3, bMaskDWord);
	FalseAlmCnt->Cnt_Mcs_fail = (ret_value&0xffff);
	ret_value = PHY_QueryBBReg(Adapter, rOFDM0_FrameSync, bMaskDWord);
	FalseAlmCnt->Cnt_Fast_Fsync = (ret_value&0xffff);
	FalseAlmCnt->Cnt_SB_Search_fail = ((ret_value&0xffff0000)>>16);

	FalseAlmCnt->Cnt_Ofdm_fail = 	FalseAlmCnt->Cnt_Parity_Fail + FalseAlmCnt->Cnt_Rate_Illegal +
								FalseAlmCnt->Cnt_Crc8_fail + FalseAlmCnt->Cnt_Mcs_fail+
								FalseAlmCnt->Cnt_Fast_Fsync + FalseAlmCnt->Cnt_SB_Search_fail;

	
	//hold cck counter
	PHY_SetBBReg(Adapter, rCCK0_FalseAlarmReport, BIT14, 1);
	
	ret_value = PHY_QueryBBReg(Adapter, rCCK0_FACounterLower, bMaskByte0);
	FalseAlmCnt->Cnt_Cck_fail = ret_value;

	ret_value = PHY_QueryBBReg(Adapter, rCCK0_FACounterUpper, bMaskByte3);
	FalseAlmCnt->Cnt_Cck_fail +=  (ret_value& 0xff)<<8;
	
	FalseAlmCnt->Cnt_all = (	FalseAlmCnt->Cnt_Parity_Fail +
						FalseAlmCnt->Cnt_Rate_Illegal +
						FalseAlmCnt->Cnt_Crc8_fail +
						FalseAlmCnt->Cnt_Mcs_fail +
						FalseAlmCnt->Cnt_Cck_fail);	
	
	//reset false alarm counter registers
	PHY_SetBBReg(Adapter, rOFDM1_LSTF, 0x08000000, 1);
	PHY_SetBBReg(Adapter, rOFDM1_LSTF, 0x08000000, 0);
	//reset cck counter
	PHY_SetBBReg(Adapter, rCCK0_FalseAlarmReport, 0x0000c000, 0);
	//enable cck counter
	PHY_SetBBReg(Adapter, rCCK0_FalseAlarmReport, 0x0000c000, 2);

	if(!pMgntInfo->bMediaConnect)
	{
		BBReset = PlatformEFIORead1Byte(Adapter, 0x02);
		PlatformEFIOWrite1Byte(Adapter, 0x02, BBReset&(~BIT0));
		PlatformEFIOWrite1Byte(Adapter, 0x02, BBReset|BIT0);
	}

	RT_TRACE(	COMP_DIG, DBG_LOUD, ("Cnt_Parity_Fail = %d, Cnt_Rate_Illegal = %d, Cnt_Crc8_fail = %d, Cnt_Mcs_fail = %d\n", 
				FalseAlmCnt->Cnt_Parity_Fail, FalseAlmCnt->Cnt_Rate_Illegal, FalseAlmCnt->Cnt_Crc8_fail, FalseAlmCnt->Cnt_Mcs_fail) );	
	RT_TRACE(	COMP_DIG, DBG_LOUD, ("Cnt_Fast_Fsync = %d, Cnt_SB_Search_fail = %d\n", 
				FalseAlmCnt->Cnt_Fast_Fsync, FalseAlmCnt->Cnt_SB_Search_fail) );
	RT_TRACE(	COMP_DIG, DBG_LOUD, ("Cnt_Ofdm_fail = %d, Cnt_Cck_fail = %d, Cnt_all = %d\n", 
				FalseAlmCnt->Cnt_Ofdm_fail, FalseAlmCnt->Cnt_Cck_fail, FalseAlmCnt->Cnt_all) );		
	RT_TRACE(	COMP_DIG, DBG_LOUD, ("RSSI_A=%d, RSSI_B=%d, RSSI_Ave=%d, RSSI_CCK=%d\n",
		Adapter->RxStats.RxRSSIPercentage[0], Adapter->RxStats.RxRSSIPercentage[1], pHalData->UndecoratedSmoothedPWDB,pHalData->UndecoratedSmoothedCCK));
	RT_TRACE(	COMP_DIG, DBG_LOUD, ("RX Rate =  0x%x, TX Rate = 0x%x \n", pHalData->RxRate, TxRate));

}
#endif

VOID 
odm_FalseAlarmCounterStatistics_8192C(
	IN	PADAPTER	Adapter
	)
{
	u4Byte ret_value;
	PFALSE_ALARM_STATISTICS FalseAlmCnt = &(Adapter->FalseAlmCnt);
	//pDIG_T	pDM_DigTable = &Adapter->DM_DigTable;
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	PMGNT_INFO	pMgntInfo = &(Adapter->MgntInfo);
	u1Byte	BBReset;

	//hold ofdm counter
	PHY_SetBBReg(Adapter, rOFDM0_LSTF, BIT31, 1); //hold page C counter
	PHY_SetBBReg(Adapter, rOFDM1_LSTF, BIT31, 1); //hold page D counter
	
	ret_value = PHY_QueryBBReg(Adapter, rOFDM0_FrameSync, bMaskDWord);
	FalseAlmCnt->Cnt_Fast_Fsync = (ret_value&0xffff);
	FalseAlmCnt->Cnt_SB_Search_fail = ((ret_value&0xffff0000)>>16);		
	ret_value = PHY_QueryBBReg(Adapter, rOFDM_PHYCounter1, bMaskDWord);
    FalseAlmCnt->Cnt_Parity_Fail = ((ret_value&0xffff0000)>>16);	
    ret_value = PHY_QueryBBReg(Adapter, rOFDM_PHYCounter2, bMaskDWord);
	FalseAlmCnt->Cnt_Rate_Illegal = (ret_value&0xffff);
	FalseAlmCnt->Cnt_Crc8_fail = ((ret_value&0xffff0000)>>16);
	ret_value = PHY_QueryBBReg(Adapter, rOFDM_PHYCounter3, bMaskDWord);
	FalseAlmCnt->Cnt_Mcs_fail = (ret_value&0xffff);


	if(IS_HARDWARE_TYPE_8723A(Adapter))
	{// By pass Cnt_Fast_Fsync and Cnt_SB_Search_fail for 8723A temporarily.
		FalseAlmCnt->Cnt_Ofdm_fail = 	FalseAlmCnt->Cnt_Parity_Fail + FalseAlmCnt->Cnt_Rate_Illegal +
								FalseAlmCnt->Cnt_Crc8_fail + FalseAlmCnt->Cnt_Mcs_fail;
	}
	else
	{
	FalseAlmCnt->Cnt_Ofdm_fail = 	FalseAlmCnt->Cnt_Parity_Fail + FalseAlmCnt->Cnt_Rate_Illegal +
								FalseAlmCnt->Cnt_Crc8_fail + FalseAlmCnt->Cnt_Mcs_fail +
								FalseAlmCnt->Cnt_Fast_Fsync + FalseAlmCnt->Cnt_SB_Search_fail;
	}

	if(pHalData->CurrentBandType92D != BAND_ON_5G)
	{
		//hold cck counter
		AcquireCCKAndRWPageAControl(Adapter);
	//	PHY_SetBBReg(Adapter, rCCK0_FalseAlarmReport, BIT14, 1);
		
		ret_value = PHY_QueryBBReg(Adapter, rCCK0_FACounterLower, bMaskByte0);
		FalseAlmCnt->Cnt_Cck_fail = ret_value;

		ret_value = PHY_QueryBBReg(Adapter, rCCK0_FACounterUpper, bMaskByte3);
		FalseAlmCnt->Cnt_Cck_fail +=  (ret_value& 0xff)<<8;
		ReleaseCCKAndRWPageAControl(Adapter);
	}
	else
	{
		FalseAlmCnt->Cnt_Cck_fail = 0;
	}

#if 1	
	FalseAlmCnt->Cnt_all = (	FalseAlmCnt->Cnt_Fast_Fsync + 
						FalseAlmCnt->Cnt_SB_Search_fail +
						FalseAlmCnt->Cnt_Parity_Fail +
						FalseAlmCnt->Cnt_Rate_Illegal +
						FalseAlmCnt->Cnt_Crc8_fail +
						FalseAlmCnt->Cnt_Mcs_fail +
						FalseAlmCnt->Cnt_Cck_fail);	
#endif
#if 0 //Just for debug
	if(pDM_DigTable->CurIGValue < 0x25)
		FalseAlmCnt->Cnt_all = 12000;
	else if(pDM_DigTable->CurIGValue < 0x2A)
		FalseAlmCnt->Cnt_all = 20;
	else if(pDM_DigTable->CurIGValue < 0x2D)
		FalseAlmCnt->Cnt_all = 0;
#endif
	
	//reset false alarm counter registers
	PHY_SetBBReg(Adapter, rOFDM1_LSTF, 0x08000000, 1);
	PHY_SetBBReg(Adapter, rOFDM1_LSTF, 0x08000000, 0);
	//update ofdm counter
	PHY_SetBBReg(Adapter, rOFDM0_LSTF, BIT31, 0); //update page C counter
	PHY_SetBBReg(Adapter, rOFDM1_LSTF, BIT31, 0); //update page D counter
	if(pHalData->CurrentBandType92D != BAND_ON_5G)
	{
		//reset cck counter
		AcquireCCKAndRWPageAControl(Adapter);
	//	RT_TRACE(COMP_INIT,DBG_LOUD,("Acquiere mutex in dm_falsealarmcount 111 \n"));
		PHY_SetBBReg(Adapter, rCCK0_FalseAlarmReport, 0x0000c000, 0);
		//enable cck counter
		PHY_SetBBReg(Adapter, rCCK0_FalseAlarmReport, 0x0000c000, 2);
		ReleaseCCKAndRWPageAControl(Adapter);
	//	RT_TRACE(COMP_INIT,DBG_LOUD,("Release mutex in dm_falsealarmcount 111 \n"));

	}

	//BB Reset
	if(IS_HARDWARE_TYPE_8192D(Adapter))
	{
		if(pHalData->MacPhyMode92D == DUALMAC_SINGLEPHY)
		{
			if((pHalData->CurrentBandType92D == BAND_ON_2_4G) && Adapter->bMasterOfDMSP && (!pMgntInfo->bMediaConnect))
			{
				BBReset = PlatformEFIORead1Byte(Adapter, 0x02);
				PlatformEFIOWrite1Byte(Adapter, 0x02, BBReset&(~BIT0));
				PlatformEFIOWrite1Byte(Adapter, 0x02, BBReset|BIT0);
			}
		}
		else
		{
			if((pHalData->CurrentBandType92D == BAND_ON_2_4G) && (!pMgntInfo->bMediaConnect))
			{
				BBReset = PlatformEFIORead1Byte(Adapter, 0x02);
				PlatformEFIOWrite1Byte(Adapter, 0x02, BBReset&(~BIT0));
				PlatformEFIOWrite1Byte(Adapter, 0x02, BBReset|BIT0);
			}
		}
	}
	else if(!pMgntInfo->bMediaConnect)
	{
		BBReset = PlatformEFIORead1Byte(Adapter, 0x02);
		PlatformEFIOWrite1Byte(Adapter, 0x02, BBReset&(~BIT0));
		PlatformEFIOWrite1Byte(Adapter, 0x02, BBReset|BIT0);
	}
	
}

VOID
odm_EnableEDCCA(
	IN	PADAPTER	Adapter
)
{
	// Enable EDCCA. The value is suggested by SD3 Wilson.
	PlatformEFIOWrite1Byte(Adapter, rOFDM0_ECCAThreshold, 0x03);
	PlatformEFIOWrite1Byte(Adapter, rOFDM0_ECCAThreshold+2, 0x00);
}

VOID
odm_DisableEDCCA(
	IN	PADAPTER	Adapter
)
{	
	// Disable EDCCA..
	PlatformEFIOWrite1Byte(Adapter, rOFDM0_ECCAThreshold, 0x7f);
	PlatformEFIOWrite1Byte(Adapter, rOFDM0_ECCAThreshold+2, 0x7f);
}

//
// Description: According to initial gain value to determine to enable or disable EDCCA.
//
// Suggested by SD3 Wilson. Added by tynli. 2011.11.25.
//
VOID
odm_DynamicEDCCA(
	IN	PADAPTER	Adapter
)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u1Byte	RegC50, RegC58;
	
	RegC50 = (u1Byte)PHY_QueryBBReg(Adapter, rOFDM0_XAAGCCore1, bMaskByte0);
	RegC58 = (u1Byte)PHY_QueryBBReg(Adapter, rOFDM0_XBAGCCore1, bMaskByte0);

	if(RegC50 > 0x28 && RegC58 > 0x28)
	{
		if(!pHalData->bPreEdccaEnable)
		{
			odm_EnableEDCCA(Adapter);
			pHalData->bPreEdccaEnable = TRUE;
		}
		
	}
	else if(RegC50 < 0x25 && RegC58 < 0x25)
	{
		if(pHalData->bPreEdccaEnable)
		{
			odm_DisableEDCCA(Adapter);
			pHalData->bPreEdccaEnable = FALSE;
		}
	}
}


#if 0
VOID 
odm_CtrlInitGainByFA(
	IN	PADAPTER	pAdapter
)	
{
	pDIG_T	pDM_DigTable = &pAdapter->DM_DigTable;
	u1Byte	value_IGI = pDM_DigTable->CurIGValue;
	
	if(pAdapter->FalseAlmCnt.Cnt_all < DM_DIG_FA_TH0)	
		value_IGI --;
	else if(pAdapter->FalseAlmCnt.Cnt_all < DM_DIG_FA_TH1)	
		value_IGI += 0;
	else if(pAdapter->FalseAlmCnt.Cnt_all < DM_DIG_FA_TH2)	
		value_IGI ++;
	else if(pAdapter->FalseAlmCnt.Cnt_all >= DM_DIG_FA_TH2)	
		value_IGI +=2;
	
	if(value_IGI > DM_DIG_FA_UPPER)			
		value_IGI = DM_DIG_FA_UPPER;
	else if(value_IGI < DM_DIG_FA_LOWER)		
		value_IGI = DM_DIG_FA_LOWER;

	if(pAdapter->FalseAlmCnt.Cnt_all > 10000)
		value_IGI = 0x32;
	pDM_DigTable->CurIGValue = value_IGI;
	
	DM_Write_DIG(pAdapter);
}

VOID 
odm_CtrlInitGainByRssi(
	IN	PADAPTER	pAdapter
)	
{	
	pDIG_T	pDM_DigTable = &pAdapter->DM_DigTable;

	//modify DIG upper bound
	if((pDM_DigTable->Rssi_val_min + 20) > DM_DIG_MAX )
		pDM_DigTable->rx_gain_range_max = DM_DIG_MAX;
	else
		pDM_DigTable->rx_gain_range_max = pDM_DigTable->Rssi_val_min + 20;

	//modify DIG lower bound, deal with abnorally large false alarm
	if(pAdapter->FalseAlmCnt.Cnt_all > 10000)
	{
		RT_TRACE(COMP_DIG, DBG_LOUD, ("dm_DIG(): Abnornally false alarm case. \n"));

		pDM_DigTable->LargeFAHit++;
		if(pDM_DigTable->ForbiddenIGI < pDM_DigTable->CurIGValue)
		{
			pDM_DigTable->ForbiddenIGI = pDM_DigTable->CurIGValue;
			pDM_DigTable->LargeFAHit = 1;
		}

		if(pDM_DigTable->LargeFAHit >= 3)
		{
			if((pDM_DigTable->ForbiddenIGI+1) >pDM_DigTable->rx_gain_range_max)
				pDM_DigTable->rx_gain_range_min = pDM_DigTable->rx_gain_range_max;
			else
				pDM_DigTable->rx_gain_range_min = (pDM_DigTable->ForbiddenIGI + 1);
			pDM_DigTable->Recover_cnt = 3600; //3600=2hr
		}

	}
	else
	{
		//Recovery mechanism for IGI lower bound
		if(pDM_DigTable->Recover_cnt != 0)
			pDM_DigTable->Recover_cnt --;
		else
		{
			if(pDM_DigTable->LargeFAHit == 0 )
			{
				if((pDM_DigTable->ForbiddenIGI -1) < DM_DIG_MIN)
				{
					pDM_DigTable->ForbiddenIGI = DM_DIG_MIN;
					pDM_DigTable->rx_gain_range_min = DM_DIG_MIN;
				}
				else
				{
					pDM_DigTable->ForbiddenIGI --;
					pDM_DigTable->rx_gain_range_min = (pDM_DigTable->ForbiddenIGI + 1);
				}
			}
			else if(pDM_DigTable->LargeFAHit == 3 )
			{
				pDM_DigTable->LargeFAHit = 0;
			}
		}
	}

	
	RT_TRACE(	COMP_DIG, DBG_LOUD, ("DM_DigTable.ForbiddenIGI = 0x%x, DM_DigTable.LargeFAHit = 0x%x\n",
		pDM_DigTable->ForbiddenIGI, pDM_DigTable->LargeFAHit));
	RT_TRACE(	COMP_DIG, DBG_LOUD, ("DM_DigTable.rx_gain_range_max = 0x%x, DM_DigTable.rx_gain_range_min = 0x%x\n",
		pDM_DigTable->rx_gain_range_max, pDM_DigTable->rx_gain_range_min));
	

#if (DEV_BUS_TYPE==RT_USB_INTERFACE)
	if(pAdapter->FalseAlmCnt.Cnt_all < 250)
	{
#endif
		RT_TRACE(	COMP_DIG, DBG_LOUD, ("Enter DIG by SS mode\n"));
		if(pAdapter->FalseAlmCnt.Cnt_all > pDM_DigTable->FAHighThresh)
		{
			if((pDM_DigTable->BackoffVal -2) < pDM_DigTable->BackoffVal_range_min)
				pDM_DigTable->BackoffVal = pDM_DigTable->BackoffVal_range_min;
			else
				pDM_DigTable->BackoffVal -= 2; 
		}	
		else if(pAdapter->FalseAlmCnt.Cnt_all < pDM_DigTable->FALowThresh)
		{
			if((pDM_DigTable->BackoffVal+2) > pDM_DigTable->BackoffVal_range_max)
				pDM_DigTable->BackoffVal = pDM_DigTable->BackoffVal_range_max;
			else
				pDM_DigTable->BackoffVal +=2;
		}	
		
	if((pDM_DigTable->Rssi_val_min+10-pDM_DigTable->BackoffVal) > pDM_DigTable->rx_gain_range_max)
		pDM_DigTable->CurIGValue = pDM_DigTable->rx_gain_range_max;
	else if((pDM_DigTable->Rssi_val_min+10-pDM_DigTable->BackoffVal) < pDM_DigTable->rx_gain_range_min)
		pDM_DigTable->CurIGValue = pDM_DigTable->rx_gain_range_min;
	else
		pDM_DigTable->CurIGValue = pDM_DigTable->Rssi_val_min+10-pDM_DigTable->BackoffVal;

		RT_TRACE(	COMP_DIG, DBG_LOUD, ("RSSI = 0x%x, BackoffVal %d\n", 
				pDM_DigTable->Rssi_val_min, pDM_DigTable->BackoffVal));
#if (DEV_BUS_TYPE==RT_USB_INTERFACE)
	}
#endif
#if (DEV_BUS_TYPE==RT_USB_INTERFACE)
	else
	{
		RT_TRACE(	COMP_DIG, DBG_LOUD, ("Enter DIG by FA mode\n")); 
		RT_TRACE(	COMP_DIG, DBG_LOUD, ("RSSI = 0x%x", pDM_DigTable->Rssi_val_min));

		//Adjust initial gain by false alarm
		if(pAdapter->FalseAlmCnt.Cnt_all > 1000)
			pDM_DigTable->CurIGValue = pDM_DigTable->PreIGValue+2;
		else if (pAdapter->FalseAlmCnt.Cnt_all > 750)
			pDM_DigTable->CurIGValue = pDM_DigTable->PreIGValue+1;
		else if(pAdapter->FalseAlmCnt.Cnt_all < 500)
			pDM_DigTable->CurIGValue =pDM_DigTable->PreIGValue-1;	

		//Check initial gain by upper/lower bound
		if(pDM_DigTable->CurIGValue > pDM_DigTable->rx_gain_range_max)
			pDM_DigTable->CurIGValue = pDM_DigTable->rx_gain_range_max;
		else if(pDM_DigTable->CurIGValue < pDM_DigTable->rx_gain_range_min)
			pDM_DigTable->CurIGValue = pDM_DigTable->rx_gain_range_min;
			
	}
#endif
	DM_Write_DIG(pAdapter);

}

VOID
odm_initial_gain_Multi_STA(
	IN	PADAPTER	pAdapter)
{
	static u1Byte		binitialized = FALSE;
	PMGNT_INFO 		pMgntInfo = &(pAdapter->MgntInfo);
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	s4Byte 			rssi_strength =  pHalData->EntryMinUndecoratedSmoothedPWDB;	
	BOOLEAN			bMulti_STA = FALSE;
	pDIG_T	pDM_DigTable = &pAdapter->DM_DigTable;

	
	if(MgntActQuery_ApType(pAdapter) == RT_AP_TYPE_NORMAL || MgntActQuery_ApType(pAdapter) == RT_AP_TYPE_IBSS_EMULATED ) 
		bMulti_STA = TRUE;
		else if(IsAPModeExist(pAdapter))
		bMulti_STA = TRUE;	
	else if(pMgntInfo->mIbss)
		bMulti_STA = TRUE;
		
	if(	(bMulti_STA == FALSE) 
		|| (pDM_DigTable->CurSTAConnectState != DIG_STA_DISCONNECT))
	{
		binitialized = FALSE;
		pDM_DigTable->Dig_Ext_Port_Stage = DIG_EXT_PORT_STAGE_MAX;
		return;
	}	
	else if(binitialized == FALSE)
	{
		binitialized = TRUE;
		pDM_DigTable->Dig_Ext_Port_Stage = DIG_EXT_PORT_STAGE_0;
		pDM_DigTable->CurIGValue = 0x20;
		DM_Write_DIG(pAdapter);
	}

	// Initial gain control by ap mode
	if(pDM_DigTable->CurMultiSTAConnectState == DIG_MultiSTA_CONNECT)
	{
		if (	(rssi_strength < pDM_DigTable->RssiLowThresh) 	&& 
			(pDM_DigTable->Dig_Ext_Port_Stage != DIG_EXT_PORT_STAGE_1))
		{					
			// Set to dig value to 0x20 for Luke's opinion after disable dig
			if(pDM_DigTable->Dig_Ext_Port_Stage == DIG_EXT_PORT_STAGE_2)
			{
				pDM_DigTable->CurIGValue = 0x20;
				DM_Write_DIG(pAdapter);				
			}	
			pDM_DigTable->Dig_Ext_Port_Stage = DIG_EXT_PORT_STAGE_1;	
		}	
		else if (rssi_strength > pDM_DigTable->RssiHighThresh)
		{
			pDM_DigTable->Dig_Ext_Port_Stage = DIG_EXT_PORT_STAGE_2;
			odm_CtrlInitGainByFA(pAdapter);
		} 
	}	
	else if(pDM_DigTable->Dig_Ext_Port_Stage != DIG_EXT_PORT_STAGE_0)
	{
		pDM_DigTable->Dig_Ext_Port_Stage = DIG_EXT_PORT_STAGE_0;
		pDM_DigTable->CurIGValue = 0x20;
		DM_Write_DIG(pAdapter);
	}

	RT_TRACE(	COMP_DIG, DBG_LOUD, ("CurMultiSTAConnectState = %x Dig_Ext_Port_Stage %x\n", 
				pDM_DigTable->CurMultiSTAConnectState, pDM_DigTable->Dig_Ext_Port_Stage));
}


VOID 
odm_initial_gain_STA(
	IN	PADAPTER	pAdapter)
{
	pDIG_T	pDM_DigTable = &pAdapter->DM_DigTable;
	
	RT_TRACE(	COMP_DIG, DBG_LOUD, ("PreSTAConnectState = %x, CurSTAConnectState = %x\n", 
				pDM_DigTable->PreSTAConnectState, pDM_DigTable->CurSTAConnectState));


	if(pDM_DigTable->PreSTAConnectState ==pDM_DigTable->CurSTAConnectState||
	   pDM_DigTable->CurSTAConnectState == DIG_STA_BEFORE_CONNECT ||
	  pDM_DigTable->CurSTAConnectState == DIG_STA_CONNECT)
	{
		// beforeconnect -> beforeconnect or  connect -> connect
		// (dis)connect -> beforeconnect
		// disconnect -> connecct or beforeconnect -> connect
		if(	pDM_DigTable->CurSTAConnectState != DIG_STA_DISCONNECT)
		{
			pDM_DigTable->Rssi_val_min = odm_initial_gain_MinPWDB(pAdapter);
			odm_CtrlInitGainByRssi(pAdapter);
		}	
	}
	else	
	{		
		// connect -> disconnect or beforeconnect -> disconnect
		pDM_DigTable->Rssi_val_min = 0;
		pDM_DigTable->Dig_Ext_Port_Stage = DIG_EXT_PORT_STAGE_MAX;
		pDM_DigTable->BackoffVal = DM_DIG_BACKOFF_DEFAULT;
		pDM_DigTable->CurIGValue = 0x20;
		pDM_DigTable->PreIGValue = 0;	
		DM_Write_DIG(pAdapter);
	}

}
#endif

VOID
ODM_Write_CCK_CCA_Thres(
	IN	PADAPTER	pAdapter
	)
{
	pDIG_T	pDM_DigTable = &pAdapter->DM_DigTable;

	if(pDM_DigTable->PreCCKPDState != pDM_DigTable->CurCCKPDState)
	{
		PHY_SetBBReg(pAdapter, rCCK0_CCA, bMaskByte2, pDM_DigTable->CurCCKPDState);
		pDM_DigTable->PreCCKPDState = pDM_DigTable->CurCCKPDState;
	}
	RT_TRACE(	COMP_DIG, DBG_LOUD, ("Lanhsin-> CCK_CCAThres=0x%x\n",pDM_DigTable->CurCCKPDState));
}

void odm_CCK_PacketDetectionThresh_8192C(
	IN	PADAPTER	pAdapter)
{
	//HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	pDIG_T	pDM_DigTable = &pAdapter->DM_DigTable;
	PMGNT_INFO	pMgntInfo = &(pAdapter->MgntInfo);

	if(pMgntInfo->bMediaConnect)//if(pDM_DigTable->CurSTAConnectState == DIG_STA_CONNECT)
	{
		pDM_DigTable->Rssi_val_min = odm_initial_gain_MinPWDB(pAdapter);
		if(pDM_DigTable->PreCCKPDState == 0x83)//CCK_PD_STAGE_LowRssi)
		{
			if(pDM_DigTable->Rssi_val_min <= 25)
				pDM_DigTable->CurCCKPDState = 0x83;//CCK_PD_STAGE_LowRssi;
			else
				pDM_DigTable->CurCCKPDState = 0xcd;//CCK_PD_STAGE_HighRssi;
		}
		else{
			if(pDM_DigTable->Rssi_val_min <= 20)
				pDM_DigTable->CurCCKPDState = 0x83;//CCK_PD_STAGE_LowRssi;
			else
				pDM_DigTable->CurCCKPDState = 0xcd;//CCK_PD_STAGE_HighRssi;
		}
	}
	else
		pDM_DigTable->CurCCKPDState=0x83;//CCK_PD_STAGE_MAX;
	
	ODM_Write_CCK_CCA_Thres(pAdapter);
#if 0	
	if(pDM_DigTable->PreCCKPDState != pDM_DigTable->CurCCKPDState)
	{
		if((pDM_DigTable->CurCCKPDState == CCK_PD_STAGE_LowRssi)||
			(pDM_DigTable->CurCCKPDState == CCK_PD_STAGE_MAX))
		{
			PHY_SetBBReg(pAdapter, rCCK0_CCA, bMaskByte2, 0x83);
				
			//PHY_SetBBReg(pAdapter, rCCK0_System, bMaskByte1, 0x40);
			//if(IS_92C_SERIAL(pHalData->VersionID))
				//PHY_SetBBReg(pAdapter, rCCK0_FalseAlarmReport , bMaskByte2, 0xd7);
		}
		else
		{
			PHY_SetBBReg(pAdapter, rCCK0_CCA, bMaskByte2, 0xcd);
			//PHY_SetBBReg(pAdapter,rCCK0_System, bMaskByte1, 0x47);
			//if(IS_92C_SERIAL(pHalData->VersionID))
				//PHY_SetBBReg(pAdapter, rCCK0_FalseAlarmReport , bMaskByte2, 0xd3);
		}
		pDM_DigTable->PreCCKPDState = pDM_DigTable->CurCCKPDState;
	}
	RT_TRACE(	COMP_DIG, DBG_LOUD, ("CCK_CCAThres=0x%x\n",
		(pDM_DigTable->CurCCKPDState = CCK_PD_STAGE_HighRssi)?0xcd:0x83));
	//RT_TRACE(	COMP_DIG, DBG_LOUD, ("is92C=%x\n",IS_92C_SERIAL(pHalData->VersionID)));
#endif
}
#if 0
static	void
odm_CtrlInitGainByTwoPort(
	IN	PADAPTER	pAdapter)
{
	PMGNT_INFO		pMgntInfo = &(pAdapter->MgntInfo);
	pDIG_T	pDM_DigTable = &pAdapter->DM_DigTable;
	
	if(MgntScanInProgress(pMgntInfo))
		return;

	if(BT_DigByBtRssi(pAdapter))
		return;

	// Decide the current status and if modify initial gain or not
	if(pMgntInfo->bJoinInProgress)
		pDM_DigTable->CurSTAConnectState = DIG_STA_BEFORE_CONNECT;
	else if(MgntActQuery_ApType(pAdapter) == RT_AP_TYPE_NORMAL || MgntActQuery_ApType(pAdapter) == RT_AP_TYPE_IBSS_EMULATED ) 
		pDM_DigTable->CurSTAConnectState = DIG_STA_DISCONNECT;
	else if(pMgntInfo->mAssoc)
		pDM_DigTable->CurSTAConnectState = DIG_STA_CONNECT;
	else
		pDM_DigTable->CurSTAConnectState = DIG_STA_DISCONNECT;

	odm_initial_gain_STA(pAdapter);	
	odm_initial_gain_Multi_STA(pAdapter);
	odm_CCK_PacketDetectionThresh_8192C(pAdapter);
	
	pDM_DigTable->PreSTAConnectState = pDM_DigTable->CurSTAConnectState;
}
#endif

void odm_DIG_8192C(
	IN	PADAPTER	pAdapter)
{
	PMGNT_INFO	pMgntInfo = &(pAdapter->MgntInfo);
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	pDIG_T	pDM_DigTable = &pAdapter->DM_DigTable;
	PFALSE_ALARM_STATISTICS FalseAlmCnt = &(pAdapter->FalseAlmCnt);
	pRXHP_T			pRX_HP_Table = &pAdapter->DM_RXHP_Table;
	u1Byte	DIG_Dynamic_MIN;
	BOOLEAN			FirstConnect;
#if DBG
	u1Byte TxRate = PlatformEFIORead1Byte(pAdapter, REG_INIDATA_RATE_SEL);
#endif
 	u1Byte	offset;


	// RXHP_flag=1 : Enter AGC RX HIgh Power Mode, DON'T execute DIG process!!!!
	if(pRX_HP_Table->RXHP_flag == 1)
	{
		//DbgPrint("Gary: AGC RX HP Mode, NO DIG\n");
		return;
	}


	RTPRINT(FDM, DM_Monitor, ("odm_DIG_8192C() ==>\n"));
	RT_TRACE(COMP_DIG, DBG_LOUD, ("odm_DIG_8192C() Upper=RSSI+10 ==>\n"));
	
	if(IS_HARDWARE_TYPE_8192D(pAdapter))
	{
		if(pHalData->MacPhyMode92D == DUALMAC_SINGLEPHY)
		{
			if(pAdapter->bMasterOfDMSP)
			{
				DIG_Dynamic_MIN = pDM_DigTable->DIG_Dynamic_MIN_0;
				FirstConnect = (pMgntInfo->bMediaConnect) && (pDM_DigTable->bMediaConnect_0 == FALSE);
				RT_TRACE(	COMP_DIG, DBG_LOUD, ("bMediaConnect_0=%d,  pMgntInfo->bMediaConnect=%d\n", 
					pDM_DigTable->bMediaConnect_0, pMgntInfo->bMediaConnect));	
			}
			else
			{
				DIG_Dynamic_MIN = pDM_DigTable->DIG_Dynamic_MIN_1;
				FirstConnect = (pMgntInfo->bMediaConnect) && (pDM_DigTable->bMediaConnect_1 == FALSE);
				RT_TRACE(	COMP_DIG, DBG_LOUD, ("bMediaConnect_1=%d,  pMgntInfo->bMediaConnect=%d\n", 
					pDM_DigTable->bMediaConnect_1, pMgntInfo->bMediaConnect));	
			}
			RT_TRACE(COMP_DIG, DBG_LOUD, ("pHalData->CurrentBandType92D = %s\n",
				(pHalData->CurrentBandType92D==BAND_ON_2_4G)?"2.4G":"5G"));
		}
		else
		{
			if(pHalData->CurrentBandType92D==BAND_ON_5G)
			{
				DIG_Dynamic_MIN = pDM_DigTable->DIG_Dynamic_MIN_0;
				FirstConnect = (pMgntInfo->bMediaConnect) && (pDM_DigTable->bMediaConnect_0 == FALSE);
				RT_TRACE(	COMP_DIG, DBG_LOUD, ("bMediaConnect_5G=%d,  pMgntInfo->bMediaConnect=%d\n", 
				pDM_DigTable->bMediaConnect_0, pMgntInfo->bMediaConnect));	
			}
			else
			{
				DIG_Dynamic_MIN = pDM_DigTable->DIG_Dynamic_MIN_1;
				FirstConnect = (pMgntInfo->bMediaConnect) && (pDM_DigTable->bMediaConnect_1 == FALSE);
				RT_TRACE(	COMP_DIG, DBG_LOUD, ("bMediaConnect_2.4G=%d,  pMgntInfo->bMediaConnect=%d\n", 
					pDM_DigTable->bMediaConnect_1, pMgntInfo->bMediaConnect));	
			}
		}
	}
	else
	{	
		DIG_Dynamic_MIN = pDM_DigTable->DIG_Dynamic_MIN_0;
		FirstConnect = (pMgntInfo->bMediaConnect) && (pDM_DigTable->bMediaConnect_0 == FALSE);
		RT_TRACE(	COMP_DIG, DBG_LOUD, ("bMediaConnect=%d,  pMgntInfo->bMediaConnect=%d\n", 
			pDM_DigTable->bMediaConnect_0, pMgntInfo->bMediaConnect));	
	}

	RT_TRACE(	COMP_DIG, DBG_LOUD, ("Cnt_Parity_Fail = %d, Cnt_Rate_Illegal = %d, Cnt_Crc8_fail = %d, Cnt_Mcs_fail = %d\n", 
				FalseAlmCnt->Cnt_Parity_Fail, FalseAlmCnt->Cnt_Rate_Illegal, FalseAlmCnt->Cnt_Crc8_fail, FalseAlmCnt->Cnt_Mcs_fail) );	
	RT_TRACE(	COMP_DIG, DBG_LOUD, ("Cnt_Fast_Fsync = %d, Cnt_SB_Search_fail = %d\n", 
				FalseAlmCnt->Cnt_Fast_Fsync, FalseAlmCnt->Cnt_SB_Search_fail) );
	RT_TRACE(	COMP_DIG, DBG_LOUD, ("Cnt_Ofdm_fail = %d, Cnt_Cck_fail = %d, Cnt_all = %d\n", 
				FalseAlmCnt->Cnt_Ofdm_fail, FalseAlmCnt->Cnt_Cck_fail, FalseAlmCnt->Cnt_all) );		
	RT_TRACE(	COMP_DIG, DBG_LOUD, ("RSSI_A=%d, RSSI_B=%d, RSSI_Ave=%d, RSSI_CCK=%d\n",
		pAdapter->RxStats.RxRSSIPercentage[0], pAdapter->RxStats.RxRSSIPercentage[1], pHalData->UndecoratedSmoothedPWDB,pHalData->UndecoratedSmoothedCCK));
	RT_TRACE(	COMP_DIG, DBG_LOUD, ("RX Rate =  0x%x, TX Rate = 0x%x \n", pHalData->RxRate, TxRate));

	
	if(pMgntInfo->bDMInitialGainEnable == FALSE)
		return;
	
	//if (DM_DigTable.Dig_Enable_Flag == FALSE)
		//return;

#if OS_WIN_FROM_WIN7(OS_VERSION)
	if(IsAPModeExist( pAdapter) && pAdapter->bInHctTest)
	    	return;
#endif

	if(MgntScanInProgress(pMgntInfo))
		return;

	RTPRINT(FDM, DM_Monitor, ("odm_DIG_8192C() progress \n"));

	//odm_CtrlInitGainByTwoPort(pAdapter);
	//RTPRINT(FDM, DM_Monitor, ("odm_DIG_8192C() <==\n"));
	
	//1 Boundary Decision
	if(pMgntInfo->bMediaConnect)
	{
		//2 Get minimum RSSI value among associated devices
		pDM_DigTable->Rssi_val_min = odm_initial_gain_MinPWDB(pAdapter);
		RT_TRACE(COMP_DIG, DBG_LOUD, ("pDM_DigTable->Rssi_val_min = 0x%x\n", pDM_DigTable->Rssi_val_min));

		//2 Modify DIG upper bound
		// Currently for 92DE combo/8723AE, we always set the condition the same
		// with BT busy, should be revised in the future.
		if( IS_HARDWARE_TYPE_8192DE(pAdapter) ||
			IS_HARDWARE_TYPE_8723A(pAdapter) )//if(BTDM_IsBTBusy(pAdapter))
			offset = 10;
		else
			offset = 20;
				
		if((pDM_DigTable->Rssi_val_min + offset) > DM_DIG_MAX )
			pDM_DigTable->rx_gain_range_max = DM_DIG_MAX;
		else if((pDM_DigTable->Rssi_val_min + offset) < DM_DIG_MIN )
			pDM_DigTable->rx_gain_range_max = DM_DIG_MIN;
		else
			pDM_DigTable->rx_gain_range_max = pDM_DigTable->Rssi_val_min + offset;

		//2 Modify DIG lower bound
		if((FalseAlmCnt->Cnt_all > 500)&&(DIG_Dynamic_MIN < 0x25))
			DIG_Dynamic_MIN++;
		else if(((pAdapter->FalseAlmCnt.Cnt_all < 500)||(pDM_DigTable->Rssi_val_min < 8))&&(DIG_Dynamic_MIN > DM_DIG_MIN))
			DIG_Dynamic_MIN--;
		//2011.09.26 LukeLee: add for DIG_Dynamic_MIN limitation
		if(DIG_Dynamic_MIN > pDM_DigTable->rx_gain_range_max)
			DIG_Dynamic_MIN = pDM_DigTable->rx_gain_range_max;
		
#if (DEV_BUS_TYPE == RT_USB_INTERFACE)
		//Modification for ext-LNA board
		if(pHalData->InterfaceSel == INTF_SEL1_USB_High_Power)
		{
			if((pDM_DigTable->Rssi_val_min + 20) > DM_DIG_MAX_HP )
				pDM_DigTable->rx_gain_range_max = DM_DIG_MAX_HP;
			else
				pDM_DigTable->rx_gain_range_max = pDM_DigTable->Rssi_val_min + 20;
			DIG_Dynamic_MIN = DM_DIG_MIN_HP;
		}
#endif
	}
	else
	{
		pDM_DigTable->rx_gain_range_max = DM_DIG_MAX;
		DIG_Dynamic_MIN = DM_DIG_MIN;
#if (DEV_BUS_TYPE == RT_USB_INTERFACE)
		//Modification for ext-LNA board
		if(pHalData->InterfaceSel == INTF_SEL1_USB_High_Power)
		{
			pDM_DigTable->rx_gain_range_max = DM_DIG_MAX_HP;
			DIG_Dynamic_MIN = DM_DIG_MIN_HP;
		}
#endif
	}
	
	//1 Modify DIG lower bound, deal with abnorally large false alarm
	if(FalseAlmCnt->Cnt_all > 10000)
	{
		RT_TRACE(COMP_DIG, DBG_LOUD, ("dm_DIG(): Abnornally false alarm case. \n"));

		if(pDM_DigTable->LargeFAHit < 0xFF)
		pDM_DigTable->LargeFAHit++;
		
		if(pDM_DigTable->ForbiddenIGI < pDM_DigTable->CurIGValue)
		{
			pDM_DigTable->ForbiddenIGI = pDM_DigTable->CurIGValue;
			pDM_DigTable->LargeFAHit = 1;
		}

		if(pDM_DigTable->LargeFAHit >= 3) //Enter Abnormal  Case
		{
			if(pDM_DigTable->ForbiddenIGI < pDM_DigTable->rx_gain_range_max)
				pDM_DigTable->DeadZone = pDM_DigTable->ForbiddenIGI;
			else
				pDM_DigTable->DeadZone = pDM_DigTable->rx_gain_range_max -1;		
			pDM_DigTable->Recover_cnt = 3600; //3600=2hr
		}

	}
	else
	{
		//2011.09.26 LukeLee: to prevent RSSI drop suddenly
		if( pDM_DigTable->DeadZone >= pDM_DigTable->rx_gain_range_max)
			pDM_DigTable->DeadZone = pDM_DigTable->rx_gain_range_max;
		
		if(pDM_DigTable->Recover_cnt != 0) //Stay in Abnormal Case
			pDM_DigTable->Recover_cnt --;
		else
		{
			if(pDM_DigTable->LargeFAHit >= 3 ) //Reset to Normal Case
				pDM_DigTable->LargeFAHit = 0;
			else if(pDM_DigTable->LargeFAHit == 0) //Normal Case
			{
				if(pDM_DigTable->DeadZone > DIG_Dynamic_MIN)
					pDM_DigTable->DeadZone--;
				else
					pDM_DigTable->DeadZone = DIG_Dynamic_MIN -1;
				pDM_DigTable->ForbiddenIGI = pDM_DigTable->DeadZone;
			}
		}
	}
	pDM_DigTable->rx_gain_range_min = ((pDM_DigTable->DeadZone+1) > DIG_Dynamic_MIN)?
		(pDM_DigTable->DeadZone+1):DIG_Dynamic_MIN;
	
	RT_TRACE(	COMP_DIG, DBG_LOUD, ("DM_DigTable.DeadZone = 0x%x, DM_DigTable.LargeFAHit = 0x%x\n",
		pDM_DigTable->DeadZone, pDM_DigTable->LargeFAHit));
	RT_TRACE(	COMP_DIG, DBG_LOUD, ("DM_DigTable.rx_gain_range_max = 0x%x, DM_DigTable.rx_gain_range_min = 0x%x\n",
		pDM_DigTable->rx_gain_range_max, pDM_DigTable->rx_gain_range_min));

	//1 Adjust initial gain by false alarm
	if(pMgntInfo->bMediaConnect)
	{
		if(FirstConnect)
		{
			pDM_DigTable->CurIGValue = pDM_DigTable->Rssi_val_min;
			RT_TRACE(	COMP_DIG, DBG_LOUD, ("DIG: First Connect\n"));
		}
		else
		{
			if(IS_HARDWARE_TYPE_8192D(pAdapter))
			{
				if(FalseAlmCnt->Cnt_all > DM_DIG_FA_TH2_92D)
					pDM_DigTable->CurIGValue = pDM_DigTable->PreIGValue+2;
				else if (FalseAlmCnt->Cnt_all > DM_DIG_FA_TH1_92D)
					pDM_DigTable->CurIGValue = pDM_DigTable->PreIGValue+1;
				else if(FalseAlmCnt->Cnt_all < DM_DIG_FA_TH0_92D)
					pDM_DigTable->CurIGValue =pDM_DigTable->PreIGValue-1;	
			}
			else
			{
				if(FalseAlmCnt->Cnt_all > DM_DIG_FA_TH2)
					pDM_DigTable->CurIGValue = pDM_DigTable->PreIGValue+2;
				else if (FalseAlmCnt->Cnt_all > DM_DIG_FA_TH1)
					pDM_DigTable->CurIGValue = pDM_DigTable->PreIGValue+1;
				else if(FalseAlmCnt->Cnt_all < DM_DIG_FA_TH0)
					pDM_DigTable->CurIGValue =pDM_DigTable->PreIGValue-1;	
			}
		}
	}	
	else
	{
		pDM_DigTable->CurIGValue = pDM_DigTable->rx_gain_range_min;
	}
	//1 Check initial gain by upper/lower bound
	if(pDM_DigTable->CurIGValue > pDM_DigTable->rx_gain_range_max)
		pDM_DigTable->CurIGValue = pDM_DigTable->rx_gain_range_max;
	if(pDM_DigTable->CurIGValue < pDM_DigTable->rx_gain_range_min)
		pDM_DigTable->CurIGValue = pDM_DigTable->rx_gain_range_min;
	
	
	//2 ADD by 8723
	// High power RSSI threshold
	if(IS_HARDWARE_TYPE_8723A(pAdapter) && 
		(pHalData->UndecoratedSmoothedPWDB > DM_DIG_HIGH_PWR_THRESHOLD))
	{
		// High power IGI lower bound
		RT_TRACE(COMP_DIG, DBG_LOUD, ("odm_DIG(): UndecoratedSmoothedPWDB(%#x)\n", pHalData->UndecoratedSmoothedPWDB));
		if(pDM_DigTable->CurIGValue < DM_DIG_HIGH_PWR_IGI_LOWER_BOUND)
		{
			RT_TRACE(COMP_DIG, DBG_LOUD, ("odm_DIG(): CurIGValue(%#x)\n", pDM_DigTable->CurIGValue));
			pDM_DigTable->CurIGValue = DM_DIG_HIGH_PWR_IGI_LOWER_BOUND;
		}
	}



	
	if(IS_HARDWARE_TYPE_8192D(pAdapter))
	{
		//sherry  delete DualMacSmartConncurrent 20110517
		if(pHalData->MacPhyMode92D == DUALMAC_SINGLEPHY)
		{
			DM_Write_DIG_DMSP(pAdapter);
			if(pAdapter->bMasterOfDMSP)
			{
				pDM_DigTable->bMediaConnect_0 = pMgntInfo->bMediaConnect;
				pDM_DigTable->DIG_Dynamic_MIN_0 = DIG_Dynamic_MIN;
			}
			else
			{
				pDM_DigTable->bMediaConnect_1 = pMgntInfo->bMediaConnect;
				pDM_DigTable->DIG_Dynamic_MIN_1 = DIG_Dynamic_MIN;
			}
		}
		else
		{
			DM_Write_DIG(pAdapter);
			if(pHalData->CurrentBandType92D==BAND_ON_5G)
			{
				pDM_DigTable->bMediaConnect_0 = pMgntInfo->bMediaConnect;
				pDM_DigTable->DIG_Dynamic_MIN_0 = DIG_Dynamic_MIN;
			}
			else
			{
				pDM_DigTable->bMediaConnect_1 = pMgntInfo->bMediaConnect;
				pDM_DigTable->DIG_Dynamic_MIN_1 = DIG_Dynamic_MIN;
			}
		}
	}
	else
	{
		DM_Write_DIG(pAdapter);
		pDM_DigTable->bMediaConnect_0 = pMgntInfo->bMediaConnect;
		pDM_DigTable->DIG_Dynamic_MIN_0 = DIG_Dynamic_MIN;
	}
	RTPRINT(FDM, DM_Monitor, ("odm_DIG_8192C() <==\n"));
	RT_TRACE(COMP_DIG, DBG_LOUD, ("odm_DIG_8192C() <==\n"));

}

#if(DEV_BUS_TYPE == RT_PCI_INTERFACE)||(DEV_BUS_TYPE == RT_USB_INTERFACE)

void	odm_RXHPInit(
	IN	PADAPTER	pAdapter)
{
	pRXHP_T			pRX_HP_Table = &pAdapter->DM_RXHP_Table;
   	u1Byte			index;

	pRX_HP_Table->RXHP_enable = TRUE;
	pRX_HP_Table->RXHP_flag = 0;
	pRX_HP_Table->PSD_func_trigger = 0;
	pRX_HP_Table->Pre_IGI = 0x20;
	pRX_HP_Table->Cur_IGI = 0x20;
	pRX_HP_Table->First_time_enter = FALSE;
	pRX_HP_Table->Cur_pw_th = pw_th_10dB;
	pRX_HP_Table->Pre_pw_th = pw_th_10dB;
	for(index=0; index<80; index++)
		pRX_HP_Table->PSD_bitmap_RXHP[index] = 0;

#if(DEV_BUS_TYPE == RT_USB_INTERFACE)
	pRX_HP_Table->TP_Mode = Idle_Mode;
#endif
	
}

void odm_RXHP(
	IN	PADAPTER	Adapter)
{
	PMGNT_INFO	pMgntInfo = &(Adapter->MgntInfo);
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	pDIG_T		pDM_DigTable = &Adapter->DM_DigTable;
	pRXHP_T		pRX_HP_Table = &Adapter->DM_RXHP_Table;
       PFALSE_ALARM_STATISTICS		FalseAlmCnt = &(Adapter->FalseAlmCnt);
	
	u1Byte              	i, j, sum;
	s1Byte              	Intf_diff_idx, MIN_Intf_diff_idx = 16;   
       s4Byte              	cur_channel;    
       u1Byte              	ch_map_intf_5M[17] = {0};     
       static u4Byte		FA_TH = 0;	
	static u1Byte      	psd_intf_flag = 0;
	static s4Byte      	curRssi = 0;                
       static s4Byte  		preRssi = 0;                                                                
	
	u1Byte			RX_HP_enable = (u1Byte)(PHY_QueryBBReg(Adapter, rOFDM0_XAAGCCore2, bMaskDWord)>>31);   // for debug!!
	static u1Byte		PSDTriggerCnt = 1;
#if(DEV_BUS_TYPE == RT_USB_INTERFACE)	
	static s8Byte  		lastTxOkCnt = 0, lastRxOkCnt = 0;  
       s8Byte			curTxOkCnt, curRxOkCnt;
	s8Byte			curTPOkCnt;
	s8Byte			TP_Acc3, TP_Acc5;
	static s8Byte		TP_Buff[5] = {0};
	static u1Byte		pre_state = 0, pre_state_flag = 0;
	static u1Byte		Intf_HighTP_flag = 0, De_counter = 16; 
	static u1Byte		TP_Degrade_flag = 0;
#endif	   
	static u1Byte		LatchCnt = 0;
	
     	if(IS_HARDWARE_TYPE_8723A(Adapter))
		return;

	//AGC RX High Power Mode is only applied on 2G band in 92D!!!
	if(IS_HARDWARE_TYPE_8192D(Adapter))
	{
		if(pHalData->CurrentBandType92D != BAND_ON_2_4G)
			return;
	}

	//RX HP ON/OFF
	if(RX_HP_enable == 1)
		pRX_HP_Table->RXHP_enable = FALSE;
	else
		pRX_HP_Table->RXHP_enable = TRUE;

	if(pRX_HP_Table->RXHP_enable == FALSE)
	{
		if(pRX_HP_Table->RXHP_flag == 1)
		{
			pRX_HP_Table->RXHP_flag = 0;
			psd_intf_flag = 0;
		}
		return;
	}
#if(DEV_BUS_TYPE == RT_USB_INTERFACE)	
	//2 Record current TP for USB interface
	curTxOkCnt = Adapter->TxStats.NumTxBytesUnicast - lastTxOkCnt;
	curRxOkCnt = Adapter->RxStats.NumRxBytesUnicast - lastRxOkCnt;
	lastTxOkCnt = Adapter->TxStats.NumTxBytesUnicast;
	lastRxOkCnt = Adapter->RxStats.NumRxBytesUnicast;

	curTPOkCnt = curTxOkCnt+curRxOkCnt;
	TP_Buff[0] = curTPOkCnt;    // current TP  
	//TP_Acc3 = (TP_Buff[1]+TP_Buff[2]+TP_Buff[3])/3;  
	TP_Acc3 = PlatformDivision64((TP_Buff[1]+TP_Buff[2]+TP_Buff[3]), 3);
	//TP_Acc5 = (TP_Buff[0]+TP_Buff[1]+TP_Buff[2]+TP_Buff[3]+TP_Buff[4])/5;
	TP_Acc5 = PlatformDivision64((TP_Buff[0]+TP_Buff[1]+TP_Buff[2]+TP_Buff[3]+TP_Buff[4]), 5);
	
	if(TP_Acc5 < 1000)
		pRX_HP_Table->TP_Mode = Idle_Mode;
	else if((1000 < TP_Acc5)&&(TP_Acc5 < 3750000))
		pRX_HP_Table->TP_Mode = Low_TP_Mode;
	else
		pRX_HP_Table->TP_Mode = High_TP_Mode;

	//DbgPrint("Gary-->TP_Mode = %d\n", pRX_HP_Table->TP_Mode);
	// Since TP result would be sampled every 2 sec, it needs to delay 4sec to wait PSD processing.
	// When LatchCnt = 0, we would Get PSD result.
	if(TP_Degrade_flag == 1)
	{
		LatchCnt--;
		if(LatchCnt == 0)
		{
			TP_Degrade_flag = 0;
		}
	}
	// When PSD function triggered by TP degrade 20%, and Interference Flag = 1
	// Set a De_counter to wait IGI = upper bound. If time is UP, the Interference flag will be pull down.
	if(Intf_HighTP_flag == 1)
	{
		De_counter--;
		if(De_counter == 0)
		{
			Intf_HighTP_flag = 0;
			psd_intf_flag = 0;
		}
	}
#endif
	//2 AGC RX High Power Mode by PSD only applied to STA Mode
	//3 NOT applied 1. Ad Hoc Mode.
	//3 NOT applied 2. AP Mode
	if ((pMgntInfo->mAssoc) && (!pMgntInfo->mIbss) && (!ACTING_AS_AP(Adapter)))
	{    
		cur_channel = PHY_QueryRFReg(Adapter, RF_PATH_A, RF_CHNLBW, 0x0fff) & 0x0f;
		curRssi = pHalData->UndecoratedSmoothedPWDB;
       	//DbgPrint("Gary: FA_all = %d\n", FalseAlmCnt->Cnt_all);
		//DbgPrint("Gary: RSSI = %d\n", curRssi);    
       	//DbgPrint("Gary: current CH = %d\n", cur_channel);
       	//2 PSD function would be triggered 
       	//3 1. Every 4 sec for PCIE
       	//3 2. Before TP Mode (Idle TP<4kbps) for USB
       	//3 3. After TP Mode (High TP) for USB 
		if((curRssi > 68) && (pRX_HP_Table->RXHP_flag == 0))	// Only RSSI>TH and RX_HP_flag=0 will Do PSD process 
		{
#if (DEV_BUS_TYPE == RT_USB_INTERFACE)
			//2 Before TP Mode ==> PSD would be trigger every 4 sec
			if(pRX_HP_Table->TP_Mode == Idle_Mode)		//2.1 less wlan traffic <4kbps
			{
#endif
				if(PSDTriggerCnt == 1)       
				{    	
					odm_PSDMonitor_8192C(Adapter);
					pRX_HP_Table->PSD_func_trigger = 1;
					PSDTriggerCnt = 0;
				}
				else
				{
             				PSDTriggerCnt++;
				}
#if(DEV_BUS_TYPE == RT_USB_INTERFACE)
			}	
			//2 After TP Mode ==> Check if TP degrade larger than 20% would trigger PSD function
			if(pRX_HP_Table->TP_Mode == High_TP_Mode)
			{
				if((pre_state_flag == 0)&&(LatchCnt == 0)) 
				{
					// TP var < 5%
					if((((curTPOkCnt-TP_Acc3)*20)<(TP_Acc3))&&(((curTPOkCnt-TP_Acc3)*20)>(-TP_Acc3)))
					{
						pre_state++;
						if(pre_state == 3)      // hit pre_state condition => consecutive 3 times
						{
							pre_state_flag = 1;
							pre_state = 0;
						}

					}
					else
					{
						pre_state = 0;
					}
				}
				//3 If pre_state_flag=1 ==> start to monitor TP degrade 20%
				if(pre_state_flag == 1)		
				{
					if(((TP_Acc3-curTPOkCnt)*5)>(TP_Acc3))      // degrade 20%
					{
						odm_PSDMonitor_8192C(Adapter);
						pRX_HP_Table->PSD_func_trigger = 1;
						TP_Degrade_flag = 1;
						LatchCnt = 2;
						pre_state_flag = 0;
					}
					else if(((TP_Buff[2]-curTPOkCnt)*5)>TP_Buff[2])
					{
						odm_PSDMonitor_8192C(Adapter);
						pRX_HP_Table->PSD_func_trigger = 1;
						TP_Degrade_flag = 1;
						LatchCnt = 2;
						pre_state_flag = 0;
					}
					else if(((TP_Buff[3]-curTPOkCnt)*5)>TP_Buff[3])
					{
						odm_PSDMonitor_8192C(Adapter);
						pRX_HP_Table->PSD_func_trigger = 1;
						TP_Degrade_flag = 1;
						LatchCnt = 2;
						pre_state_flag = 0;
					}
				}
			}
#endif	
		}

#if (DEV_BUS_TYPE == RT_USB_INTERFACE)
		for (i=0;i<4;i++)
		{
			TP_Buff[4-i] = TP_Buff[3-i];
		}
#endif
		//2 Update PSD bitmap according to PSD report 
		if((pRX_HP_Table->PSD_func_trigger == 1)&&(LatchCnt == 0))
    		{	
           		//2 Separate 80M bandwidth into 16 group with smaller 5M BW.
			for (i = 0 ; i < 16 ; i++)
           		{
				sum = 0;
				for(j = 0; j < 5 ; j++)
                			sum += pRX_HP_Table->PSD_bitmap_RXHP[5*i + j];
            
                		if(sum < 5)
                		{
                			ch_map_intf_5M[i] = 1;  // interference flag
                		}
           		}
			//2 Mask target channel 5M index
	    		for(i = 0; i < 4 ; i++)
           		{
				ch_map_intf_5M[cur_channel - 1 + i] = 0;  
           		}
           		psd_intf_flag = 0;
	    		for(i = 0; i < 16; i++)
           		{
         			if(ch_map_intf_5M[i] == 1)
	              	{
	              		psd_intf_flag = 1;            // interference is detected!!!	
	              		break;
         			}
	    		}
				
#if (DEV_BUS_TYPE == RT_USB_INTERFACE)
			if(pRX_HP_Table->TP_Mode!=Idle_Mode)
			{
				if(psd_intf_flag == 1)
				{
					Intf_HighTP_flag = 1;
					De_counter = 32;     // 0x1E -> 0x3E needs 32 times by each IGI step =1
				}
			}
#endif
      			//DbgPrint("Gary: psd_intf_flag = %d\n", psd_intf_flag);  
			/*for(i = 0; i< 16; i++)
			{
			     DbgPrint("Gary: ch_map_intf_5M[%d] = %d\n", i , ch_map_intf_5M[i]);
	    		}*/
			//2 Distance between target channel and interference
           		for(i = 0; i < 16; i++)
          		{
				if(ch_map_intf_5M[i] == 1)
                		{
					Intf_diff_idx = ((cur_channel-(i+1))>0) ? (s1Byte)(cur_channel-(i-2)) : (s1Byte)((i+1)-cur_channel);  
                      		if(Intf_diff_idx < MIN_Intf_diff_idx)
						MIN_Intf_diff_idx = Intf_diff_idx;    // the min difference index between interference and target
		  		}
	    		}
	    		//DbgPrint("Gary: MIN_Intf_diff_idx = %d\n", MIN_Intf_diff_idx); 
			//2 Choose False Alarm Threshold
			switch (MIN_Intf_diff_idx){
      				case 0: 
	   			case 1:
	        		case 2:
	        		case 3:	 	 
                 			FA_TH = FA_RXHP_TH1;  
                     		break;
	        		case 4:
	        		case 5:
		   			FA_TH = FA_RXHP_TH2;	
               			break;
	        		case 6:
	        		case 7:
		      			FA_TH = FA_RXHP_TH3;
                    			break; 
               		case 8:
	        		case 9:
		      			FA_TH = FA_RXHP_TH4;
                    			break; 	
	        		case 10:
	        		case 11:
	        		case 12:
	        		case 13:	 
	        		case 14:
	      			case 15:	 	
		      			FA_TH = FA_RXHP_TH5;
                    			break;  		
       		}	
			//DbgPrint("Gary: FA_TH = %d\n", FA_TH);
			pRX_HP_Table->PSD_func_trigger = 0;
		}
		//1 Monitor RSSI variation to choose the suitable IGI or Exit AGC RX High Power Mode
         	if(pRX_HP_Table->RXHP_flag == 1)
         	{
              	if ((curRssi > 80)&&(preRssi < 80))
              	{ 
                   		pRX_HP_Table->Cur_IGI = LNA_Low_Gain_1;
              	}
              	else if ((curRssi < 80)&&(preRssi > 80))
              	{
                   		pRX_HP_Table->Cur_IGI = LNA_Low_Gain_2;
			}
	       	else if ((curRssi > 72)&&(preRssi < 72))
	      		{
                		pRX_HP_Table->Cur_IGI = LNA_Low_Gain_2;
	       	}
              	else if ((curRssi < 72)&&( preRssi > 72))
	     		{
                   		pRX_HP_Table->Cur_IGI = LNA_Low_Gain_3;
	       	}
	       	else if (curRssi < 68)		 //RSSI is NOT large enough!!==> Exit AGC RX High Power Mode
	       	{
                   		pRX_HP_Table->Cur_pw_th = pw_th_10dB;
				pRX_HP_Table->RXHP_flag = 0;    // Back to Normal DIG Mode		  
				psd_intf_flag = 0;
			}
			preRssi = curRssi;   
		}
		else    // pRX_HP_Table->RXHP_flag == 0
		{
			//1 Decide whether to enter AGC RX High Power Mode
			if ((curRssi > 70) && (psd_intf_flag == 1) && (FalseAlmCnt->Cnt_all > FA_TH) &&     //2  AGC RX High Power Conditions
				(pDM_DigTable->CurIGValue == pDM_DigTable->rx_gain_range_max))
			{
             			if (curRssi > 80)
             			{
					pRX_HP_Table->Cur_IGI = LNA_Low_Gain_1;
				}
				else if (curRssi > 72) 
              		{
               			pRX_HP_Table->Cur_IGI = LNA_Low_Gain_2;
				}
             			else
            			{
                   			pRX_HP_Table->Cur_IGI = LNA_Low_Gain_3;
				}
           			pRX_HP_Table->Cur_pw_th = pw_th_16dB;		//RegC54[9:8]=2'b11: to enter AGC Flow 3
				pRX_HP_Table->First_time_enter = TRUE;
				pRX_HP_Table->RXHP_flag = 1;    //	RXHP_flag=1: AGC RX High Power Mode, RXHP_flag=0: Normal DIG Mode
			}
		}
		DM_Write_RXHP(Adapter);	
	}
}

void ODM_Write_RXHP(
	IN	PADAPTER	Adapter)
{
	pRXHP_T		pRX_HP_Table = &Adapter->DM_RXHP_Table;

	if(pRX_HP_Table->First_time_enter == TRUE)
	{
		PHY_SetBBReg1Byte(Adapter, rOFDM0_XAAGCCore2, BIT8|BIT9, pRX_HP_Table->Cur_pw_th);    // RegC54[9:8]=2'b00:  AGC Flow 2
		PHY_SetBBReg1Byte(Adapter, rOFDM0_XAAGCCore1, bMaskByte0, pRX_HP_Table->Cur_IGI);
	     	PHY_SetBBReg1Byte(Adapter, rOFDM0_XBAGCCore1, bMaskByte0, pRX_HP_Table->Cur_IGI);	
		pRX_HP_Table->First_time_enter = FALSE;
	}
	else
	{
		if(pRX_HP_Table->RXHP_flag == 1)
		{
			if(pRX_HP_Table->Cur_IGI != pRX_HP_Table->Pre_IGI)
			{
				PHY_SetBBReg1Byte(Adapter, rOFDM0_XAAGCCore1, bMaskByte0, pRX_HP_Table->Cur_IGI);
	     			PHY_SetBBReg1Byte(Adapter, rOFDM0_XBAGCCore1, bMaskByte0, pRX_HP_Table->Cur_IGI);	
				pRX_HP_Table->Pre_IGI = pRX_HP_Table->Cur_IGI; 
			}	
			if(pRX_HP_Table->Cur_pw_th != pRX_HP_Table->Pre_pw_th)
				pRX_HP_Table->Pre_pw_th = pRX_HP_Table->Cur_pw_th;
		}
		else
		{
			if(pRX_HP_Table->Cur_pw_th != pRX_HP_Table->Pre_pw_th )
			{	
				PHY_SetBBReg1Byte(Adapter, rOFDM0_XAAGCCore2, BIT8|BIT9, pRX_HP_Table->Cur_pw_th);  // RegC54[9:8]=2'b11:  AGC Flow 3
				pRX_HP_Table->Pre_pw_th = pRX_HP_Table->Cur_pw_th;
			}
		}
	}
}
	
#endif

void odm_DynamicEarlyMode(
	IN	PADAPTER	pAdapter)
{
	PMGNT_INFO		pMgntInfo = &(pAdapter->MgntInfo);
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	pDIG_T	pDM_DigTable = &pAdapter->DM_DigTable;
	
	//For Cisco1252 IOT,by Cherry.2011-09-21
	if((IS_HARDWARE_TYPE_8192DE(pAdapter)) && (pHalData->bEarlyModeEnable) && (pHalData->MacPhyMode92D != DUALMAC_DUALPHY))
	{
		if((pMgntInfo->mAssoc)&&(pMgntInfo->IOTPeer == HT_IOT_PEER_CISCO))
		{
			RT_TRACE(COMP_DIG, DBG_LOUD, ("IOT_PEER = CISCO \n"));
			   if( pHalData->MinUndecoratedPWDBForDM < 50)
		{
			pDM_DigTable->curearly = 0;
			RT_TRACE(COMP_DIG|COMP_INIT, DBG_LOUD, ("for CISCO,Early Mode Off \n"));
		}
		else if(pHalData->MinUndecoratedPWDBForDM > 55)	
		{
			pDM_DigTable->curearly= 1;
			RT_TRACE(COMP_DIG|COMP_INIT, DBG_LOUD, ("for CISCO, Early Mode On \n"));
		}
		}
		else if(!(PlatformEFIORead1Byte(pAdapter, REG_EARLY_MODE_CONTROL)&0xF))
		{
				pDM_DigTable->curearly= 1;
				RT_TRACE(COMP_DIG, DBG_LOUD, ("Early Mode On \n"));
		}
		
		if(pDM_DigTable->curearly != pDM_DigTable->preearly)
		{
			if(pDM_DigTable->curearly == 0)
				PlatformEFIOWrite1Byte(pAdapter, REG_EARLY_MODE_CONTROL, 0x00);
			else
				PlatformEFIOWrite1Byte(pAdapter, REG_EARLY_MODE_CONTROL, 0x0f);

			pDM_DigTable->preearly = pDM_DigTable->curearly;
		}
			
	}
}

#if 0
//For 8192D DIG mechanism,by luke.
void odm_DIG_92D(
	IN	PADAPTER	pAdapter)
{
	PMGNT_INFO		pMgntInfo = &(pAdapter->MgntInfo);
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	pDIG_T	pDM_DigTable = &pAdapter->DM_DigTable;
	u1Byte	value_IGI = pDM_DigTable->CurIGValue;
	
	RT_TRACE(COMP_DIG, DBG_LOUD, ("dm_DIG() ==>\n"));

#if OS_WIN_FROM_WIN7(OS_VERSION)
	if(IsAPModeExist( pAdapter) && pAdapter->bInHctTest)
	    	return;
#endif

#if (DEV_BUS_TYPE==RT_PCI_INTERFACE)
	//For Cisco1252 IOT,by luke.2011-01-06
	if((IS_HARDWARE_TYPE_8192DE(pAdapter)) && pHalData->bEarlyModeEnable)
	{
		if((pMgntInfo->mAssoc)&&(pMgntInfo->IOTPeer == HT_IOT_PEER_CISCO))
		{
			RT_TRACE(COMP_DIG, DBG_LOUD, ("IOT_PEER = CISCO \n"));
			if(pHalData->LastMinUndecoratedPWDBForDM>=50 && pHalData->MinUndecoratedPWDBForDM < 50)
		{
			PlatformEFIOWrite1Byte(pAdapter, REG_EARLY_MODE_CONTROL, 0x00);
			RT_TRACE(COMP_DIG|COMP_INIT, DBG_LOUD, ("for CISCO,Early Mode Off \n"));
		}
		else if(pHalData->LastMinUndecoratedPWDBForDM<= 55 && pHalData->MinUndecoratedPWDBForDM > 55)
		{
			PlatformEFIOWrite1Byte(pAdapter, REG_EARLY_MODE_CONTROL, 0x0f);
			RT_TRACE(COMP_DIG|COMP_INIT, DBG_LOUD, ("for CISCO, Early Mode On \n"));
		}
		}
		else if(!(PlatformEFIORead1Byte(pAdapter, REG_EARLY_MODE_CONTROL)&0xF))
		{
				PlatformEFIOWrite1Byte(pAdapter, REG_EARLY_MODE_CONTROL, 0x0f);
				RT_TRACE(COMP_DIG, DBG_LOUD, ("Early Mode On \n"));
		}
		pHalData->LastMinUndecoratedPWDBForDM = pHalData->MinUndecoratedPWDBForDM;
	}
#endif
	
	if(pMgntInfo->bDMInitialGainEnable == FALSE)
		return;
	//if (pDM_DigTable->Dig_Enable_Flag == FALSE)
		//return;
	if(MgntScanInProgress(pMgntInfo))
		return;

	RT_TRACE(COMP_DIG, DBG_LOUD, ("dm_DIG() progress \n"));

	// Decide the current status and if modify initial gain or not
	if(pMgntInfo->bJoinInProgress)
		pDM_DigTable->CurSTAConnectState = DIG_STA_BEFORE_CONNECT;
	else if(MgntActQuery_ApType(pAdapter) == RT_AP_TYPE_NORMAL || MgntActQuery_ApType(pAdapter) == RT_AP_TYPE_IBSS_EMULATED ) 
		pDM_DigTable->CurSTAConnectState = DIG_STA_DISCONNECT;
	else if(pMgntInfo->mAssoc)
		pDM_DigTable->CurSTAConnectState = DIG_STA_CONNECT;
	else
		pDM_DigTable->CurSTAConnectState = DIG_STA_DISCONNECT;

	//adjust initial gain according to false alarm counter
	if(pAdapter->FalseAlmCnt.Cnt_all < DM_DIG_FA_TH0_92D)
		value_IGI --;
	else if(pAdapter->FalseAlmCnt.Cnt_all < DM_DIG_FA_TH1_92D)	
		value_IGI += 0;
	else if(pAdapter->FalseAlmCnt.Cnt_all < DM_DIG_FA_TH2_92D)	
		value_IGI ++;
	else if(pAdapter->FalseAlmCnt.Cnt_all >= DM_DIG_FA_TH2_92D)	
		value_IGI +=2;


	//DBG Report
	RT_TRACE(COMP_DIG, DBG_LOUD, ("dm_DIG() Before: LargeFAHit=%d, ForbiddenIGI=%x \n", 
				pDM_DigTable->LargeFAHit, pDM_DigTable->ForbiddenIGI));
	RT_TRACE(COMP_DIG, DBG_LOUD, ("dm_DIG() Before: Recover_cnt=%d, rx_gain_range_min=%x \n", 
				pDM_DigTable->Recover_cnt, pDM_DigTable->rx_gain_range_min));

	//deal with abnorally large false alarm
	if(pAdapter->FalseAlmCnt.Cnt_all > 10000)
	{
		RT_TRACE(COMP_DIG, DBG_LOUD, ("dm_DIG(): Abnornally false alarm case. \n"));

		pDM_DigTable->LargeFAHit++;
		if(pDM_DigTable->ForbiddenIGI < pDM_DigTable->CurIGValue)
		{
			pDM_DigTable->ForbiddenIGI = pDM_DigTable->CurIGValue;
			pDM_DigTable->LargeFAHit = 1;
		}
		if(pDM_DigTable->LargeFAHit >= 3)
		{
			if((pDM_DigTable->ForbiddenIGI+1) > DM_DIG_MAX)
				pDM_DigTable->rx_gain_range_min = DM_DIG_MAX;
			else
				pDM_DigTable->rx_gain_range_min = (pDM_DigTable->ForbiddenIGI + 1);
			pDM_DigTable->Recover_cnt = 3600; //3600=2hr
		}
	}
	else
	{
		//Recovery mechanism for IGI lower bound
		if(pDM_DigTable->Recover_cnt != 0)
			pDM_DigTable->Recover_cnt --;
		else
		{
			if(pDM_DigTable->LargeFAHit == 0 )
			{
				if((pDM_DigTable->ForbiddenIGI -1) < DM_DIG_FA_LOWER)
				{
					pDM_DigTable->ForbiddenIGI = DM_DIG_FA_LOWER;
					pDM_DigTable->rx_gain_range_min = DM_DIG_FA_LOWER;

				}
				else
				{
					pDM_DigTable->ForbiddenIGI --;
					pDM_DigTable->rx_gain_range_min = (pDM_DigTable->ForbiddenIGI + 1);
				}
			}
			else if(pDM_DigTable->LargeFAHit == 3 )
			{
				pDM_DigTable->LargeFAHit = 0;
			}
		}
	}

	//DBG Report
	RT_TRACE(COMP_DIG, DBG_LOUD, ("dm_DIG() After: LargeFAHit=%d, ForbiddenIGI=%x \n", 
				pDM_DigTable->LargeFAHit, pDM_DigTable->ForbiddenIGI));
	RT_TRACE(COMP_DIG, DBG_LOUD, ("dm_DIG() After: Recover_cnt=%d, rx_gain_range_min=%x \n", 
				pDM_DigTable->Recover_cnt, pDM_DigTable->rx_gain_range_min));
	
	if(value_IGI > DM_DIG_MAX)			
		value_IGI = DM_DIG_MAX;
	else if(value_IGI < pDM_DigTable->rx_gain_range_min)		
		value_IGI = pDM_DigTable->rx_gain_range_min;

	pDM_DigTable->CurIGValue = value_IGI;

//sherry  delete DualMacSmartConncurrent 20110517
	if(pHalData->MacPhyMode92D == DUALMAC_SINGLEPHY)
	{
		DM_Write_DIG_DMSP(pAdapter);
		if(pHalData->CurrentBandType92D != BAND_ON_5G)
			dm_CCK_PacketDetectionThresh_DMSP(pAdapter);
	}
	else
	{
		DM_Write_DIG(pAdapter);
		if(pHalData->CurrentBandType92D != BAND_ON_5G)
			dm_CCK_PacketDetectionThresh(pAdapter);
	}
		
	RT_TRACE(COMP_DIG, DBG_LOUD, ("dm_DIG() <<==\n"));
}
#endif
//
//sherry move from DUSC to here 20110517
//
VOID
odm_FindMinimumRSSI_Dmsp(
	IN	PADAPTER	pAdapter
)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	s4Byte		Rssi_val_min_back_for_mac0;
	BOOLEAN		bGetValueFromBuddyAdapter = dm_DualMacGetParameterFromBuddyAdapter(pAdapter);
	BOOLEAN		bRestoreRssi = FALSE;
	PADAPTER	BuddyAdapter = pAdapter->BuddyAdapter;

	if(pHalData->MacPhyMode92D == DUALMAC_SINGLEPHY)
	{
		if(BuddyAdapter!= NULL)
		{
			if(pAdapter->bSlaveOfDMSP)
			{
				RT_TRACE(COMP_EASY_CONCURRENT,DBG_LOUD,("bSlavecase of dmsp\n"));
				BuddyAdapter->DualMacDMSPControl.RssiValMinForAnotherMacOfDMSP = pHalData->MinUndecoratedPWDBForDM;
			}
			else
			{
				if(bGetValueFromBuddyAdapter)
				{
					RT_TRACE(COMP_EASY_CONCURRENT,DBG_LOUD,("get new RSSI\n"));
					bRestoreRssi = TRUE;
					Rssi_val_min_back_for_mac0 = pHalData->MinUndecoratedPWDBForDM;
					pHalData->MinUndecoratedPWDBForDM = pAdapter->DualMacDMSPControl.RssiValMinForAnotherMacOfDMSP;
				}
			}
		}
		
	}

	if(bRestoreRssi)
	{
		bRestoreRssi = FALSE;
		pHalData->MinUndecoratedPWDBForDM = Rssi_val_min_back_for_mac0;
	}
	
}



void
odm_FindMinimumRSSI_92D(
IN	PADAPTER	pAdapter
	)
{	
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	PMGNT_INFO	pMgntInfo = &(pAdapter->MgntInfo);
	//pPS_T pDM_PSTable = &pAdapter->DM_PSTable;

	//1 1.Determine the minimum RSSI 
	if((!pMgntInfo->bMediaConnect) &&	
		(pHalData->EntryMinUndecoratedSmoothedPWDB == 0))
	{
		pHalData->MinUndecoratedPWDBForDM = 0;
		RT_TRACE(COMP_BB_POWERSAVING, DBG_LOUD, ("Not connected to any \n"));
	}
	if(pMgntInfo->bMediaConnect)	// Default port
	{
		if(ACTING_AS_AP(pAdapter) || pMgntInfo->mIbss)
		{
			pHalData->MinUndecoratedPWDBForDM = pHalData->EntryMinUndecoratedSmoothedPWDB;
			RT_TRACE(COMP_BB_POWERSAVING, DBG_LOUD, ("AP Client PWDB = 0x%x \n", pHalData->MinUndecoratedPWDBForDM));
		}
		else
		{
			pHalData->MinUndecoratedPWDBForDM = pHalData->UndecoratedSmoothedPWDB;
			RT_TRACE(COMP_BB_POWERSAVING, DBG_LOUD, ("STA Default Port PWDB = 0x%x \n", pHalData->MinUndecoratedPWDBForDM));
		}
	}
	else // associated entry pwdb
	{	
		pHalData->MinUndecoratedPWDBForDM = pHalData->EntryMinUndecoratedSmoothedPWDB;
		RT_TRACE(COMP_BB_POWERSAVING, DBG_LOUD, ("AP Ext Port or disconnet PWDB = 0x%x \n", pHalData->MinUndecoratedPWDBForDM));
	}

	odm_FindMinimumRSSI_Dmsp(pAdapter);

	RT_TRACE(COMP_DIG, DBG_LOUD, ("MinUndecoratedPWDBForDM =%d\n",pHalData->MinUndecoratedPWDBForDM));
}

//============================================================


void odm_DynamicTxPower_92D(IN	PADAPTER	Adapter)
{
	PMGNT_INFO			pMgntInfo = &Adapter->MgntInfo;
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	s4Byte				UndecoratedSmoothedPWDB;

	PADAPTER	BuddyAdapter = Adapter->BuddyAdapter;
	BOOLEAN		bGetValueFromBuddyAdapter = dm_DualMacGetParameterFromBuddyAdapter(Adapter);
	u1Byte		HighPowerLvlBackForMac0 = TxHighPwrLevel_Level1;


	// If dynamic high power is disabled.
	if( (pMgntInfo->bDynamicTxPowerEnable != TRUE) ||
		(pHalData->DMFlag & HAL_DM_HIPWR_DISABLE) ||
		pMgntInfo->IOTAction & HT_IOT_ACT_DISABLE_HIGH_POWER)
	{
		pHalData->DynamicTxHighPowerLvl = TxHighPwrLevel_Normal;
		return;
	}

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
	
	if(pMgntInfo->bMediaConnect)	// Default port
	{
		if(ACTING_AS_AP(Adapter) || pMgntInfo->mIbss)
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
	
	if(IS_HARDWARE_TYPE_8192D(Adapter) && GET_HAL_DATA(Adapter)->CurrentBandType92D == 1){
		if(UndecoratedSmoothedPWDB >= 0x33)
		{
			pHalData->DynamicTxHighPowerLvl = TxHighPwrLevel_Level2;
			RT_TRACE(COMP_HIPWR, DBG_LOUD, ("5G:TxHighPwrLevel_Level2 (TxPwr=0x0)\n"));
		}
		else if((UndecoratedSmoothedPWDB <0x33) &&
			(UndecoratedSmoothedPWDB >= 0x2b) )
		{
			pHalData->DynamicTxHighPowerLvl = TxHighPwrLevel_Level1;
			RT_TRACE(COMP_HIPWR, DBG_LOUD, ("5G:TxHighPwrLevel_Level1 (TxPwr=0x10)\n"));
		}
		else if(UndecoratedSmoothedPWDB < 0x2b)
		{
			pHalData->DynamicTxHighPowerLvl = TxHighPwrLevel_Normal;
			RT_TRACE(COMP_HIPWR, DBG_LOUD, ("5G:TxHighPwrLevel_Normal\n"));
		}

	}
	else
	
	{
		if(UndecoratedSmoothedPWDB >= TX_POWER_NEAR_FIELD_THRESH_LVL2)
		{
			pHalData->DynamicTxHighPowerLvl = TxHighPwrLevel_Level1;
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

//sherry  delete flag 20110517
	if(bGetValueFromBuddyAdapter)
	{
		RT_TRACE(COMP_MLME,DBG_LOUD,("dm_DynamicTxPower() mac 0 for mac 1 \n"));
		if(Adapter->DualMacDMSPControl.bChangeTxHighPowerLvlForAnotherMacOfDMSP)
		{
			RT_TRACE(COMP_MLME,DBG_LOUD,("dm_DynamicTxPower() change value \n"));
			HighPowerLvlBackForMac0 = pHalData->DynamicTxHighPowerLvl;
			pHalData->DynamicTxHighPowerLvl = Adapter->DualMacDMSPControl.CurTxHighLvlForAnotherMacOfDMSP;
			PHY_SetTxPowerLevel8192C(Adapter, pHalData->CurrentChannel);
			pHalData->DynamicTxHighPowerLvl = HighPowerLvlBackForMac0;
			Adapter->DualMacDMSPControl.bChangeTxHighPowerLvlForAnotherMacOfDMSP = FALSE;
		}						
	}

	if( (pHalData->DynamicTxHighPowerLvl != pHalData->LastDTPLvl) )
	{
			RT_TRACE(COMP_HIPWR, DBG_LOUD, ("PHY_SetTxPowerLevel8192S() Channel = %d \n" , pHalData->CurrentChannel));
			if(Adapter->DualMacSmartConcurrent == TRUE)
			{
				if(BuddyAdapter == NULL)
				{
					RT_TRACE(COMP_MLME,DBG_LOUD,("dm_DynamicTxPower() BuddyAdapter == NULL case \n"));
					if(!Adapter->bSlaveOfDMSP)
					{
						PHY_SetTxPowerLevel8192C(Adapter, pHalData->CurrentChannel);
					}
				}
				else
				{
					if(pHalData->MacPhyMode92D == DUALMAC_SINGLEPHY)
					{
						RT_TRACE(COMP_MLME,DBG_LOUD,("dm_DynamicTxPower() BuddyAdapter DMSP \n"));
						if(Adapter->bSlaveOfDMSP)
						{
							RT_TRACE(COMP_MLME,DBG_LOUD,("dm_DynamicTxPower() bslave case  \n"));
							BuddyAdapter->DualMacDMSPControl.bChangeTxHighPowerLvlForAnotherMacOfDMSP = TRUE;
							BuddyAdapter->DualMacDMSPControl.CurTxHighLvlForAnotherMacOfDMSP = pHalData->DynamicTxHighPowerLvl;
						}
						else
						{
							RT_TRACE(COMP_MLME,DBG_LOUD,("dm_DynamicTxPower() master case  \n"));					
							if(!bGetValueFromBuddyAdapter)
							{
								RT_TRACE(COMP_MLME,DBG_LOUD,("dm_DynamicTxPower() mac 0 for mac 0 \n"));
								PHY_SetTxPowerLevel8192C(Adapter, pHalData->CurrentChannel);
							}
						}
					}
					else
					{
						RT_TRACE(COMP_MLME,DBG_LOUD,("dm_DynamicTxPower() BuddyAdapter DMDP\n"));
						PHY_SetTxPowerLevel8192C(Adapter, pHalData->CurrentChannel);
					}
				}
			}
			else
			{
				PHY_SetTxPowerLevel8192C(Adapter, pHalData->CurrentChannel);
			}

		}
	pHalData->LastDTPLvl = pHalData->DynamicTxHighPowerLvl;
}


//============================================================


//3============================================================
//3 Tx Power Tracking
//3============================================================

//091212 chiyokolin
VOID
odm_TXPowerTrackingCallback_ThermalMeter_92C(
            IN PADAPTER	Adapter)
{
#if ((RT_PLATFORM == PLATFORM_WINDOWS) || (RT_PLATFORM == PLATFORM_LINUX)) && (HAL_CODE_BASE==RTL8192_C)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u1Byte			ThermalValue = 0, delta, delta_LCK, delta_IQK, delta_HP, TimeOut = 100;
	s4Byte 			ele_A=0, ele_D, TempCCk, X, value32;
	s4Byte			Y, ele_C=0;
	s1Byte			OFDM_index[2], CCK_index=0, OFDM_index_old[2], CCK_index_old=0;
	int	    			i = 0;
	BOOLEAN			is2T = IS_92C_SERIAL(pHalData->VersionID);

#if MP_DRIVER == 1
	PMPT_CONTEXT	pMptCtx = &(Adapter->MptCtx);	
	pu1Byte			TxPwrLevel = pMptCtx->TxPwrLevel;
#endif	
	u1Byte			OFDM_min_index = 6, rf; //OFDM BB Swing should be less than +3.0dB, which is required by Arthur
#if 0	
	u4Byte			DPK_delta_mapping[2][DPK_DELTA_MAPPING_NUM] = {
					{0x1c, 0x1c, 0x1d, 0x1d, 0x1e, 
					 0x1f, 0x00, 0x00, 0x01, 0x01,
					 0x02, 0x02, 0x03},
					{0x1c, 0x1d, 0x1e, 0x1e, 0x1e,
					 0x1f, 0x00, 0x00, 0x01, 0x02,
					 0x02, 0x03, 0x03}};
#endif	
#if DEV_BUS_TYPE==RT_USB_INTERFACE		
	u1Byte			ThermalValue_HP_count = 0;
	u4Byte			ThermalValue_HP = 0;
	s1Byte			index_mapping_HP[index_mapping_HP_NUM] = {
					0,	1,	3,	4,	6,	
					7,	9,	10,	12,	13,	
					15,	16,	18,	19,	21
					};

	s1Byte			index_HP;
#endif

	if (ODM_CheckPowerStatus(Adapter) == FALSE)
		return;
	
	pHalData->TXPowerTrackingCallbackCnt++;	//cosa add for debug
	pHalData->bTXPowerTrackingInit = TRUE;

	RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD,("===>odm_TXPowerTrackingCallback_ThermalMeter_92C\n"));

	ThermalValue = (u1Byte)PHY_QueryRFReg(Adapter, RF_PATH_A, RF_T_METER, 0x1f);	// 0x24: RF Reg[4:0]	
	RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD,("Readback Thermal Meter = 0x%x pre thermal meter 0x%x EEPROMthermalmeter 0x%x\n", ThermalValue, pHalData->ThermalValue, pHalData->EEPROMThermalMeter));

	PHY_APCalibrate_8192C(Adapter, (ThermalValue - pHalData->EEPROMThermalMeter));

	if(is2T)
		rf = 2;
	else
		rf = 1;
	
	while(PlatformAtomicExchange(&Adapter->IntrCCKRefCount, TRUE) == TRUE) 
	{
		PlatformSleepUs(100);
		TimeOut--;
		if(TimeOut <= 0)
		{
			RTPRINT(FINIT, INIT_TxPower, 
			 ("!!!odm_TXPowerTrackingCallback_ThermalMeter_92C Wait for check CCK gain index too long!!!\n" ));
			break;
		}
	}
	
	if(ThermalValue)
	{
//		if(!pHalData->ThermalValue)
		{
			//Query OFDM path A default setting 		
			ele_D = PHY_QueryBBReg(Adapter, rOFDM0_XATxIQImbalance, bMaskDWord)&bMaskOFDM_D;
			for(i=0; i<OFDM_TABLE_SIZE_92C; i++)	//find the index
			{
				if(ele_D == (OFDMSwingTable[i]&bMaskOFDM_D))
				{
					OFDM_index_old[0] = (u1Byte)i;
					RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD,("Initial pathA ele_D reg0x%x = 0x%x, OFDM_index=0x%x\n", 
						rOFDM0_XATxIQImbalance, ele_D, OFDM_index_old[0]));
					break;
				}
			}

			//Query OFDM path B default setting 
			if(is2T)
			{
				ele_D = PHY_QueryBBReg(Adapter, rOFDM0_XBTxIQImbalance, bMaskDWord)&bMaskOFDM_D;
				for(i=0; i<OFDM_TABLE_SIZE_92C; i++)	//find the index
				{
					if(ele_D == (OFDMSwingTable[i]&bMaskOFDM_D))
					{
						OFDM_index_old[1] = (u1Byte)i;
						RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD,("Initial pathB ele_D reg0x%x = 0x%x, OFDM_index=0x%x\n", 
							rOFDM0_XBTxIQImbalance, ele_D, OFDM_index_old[1]));
						break;
					}
				}
			}

			//Query CCK default setting From 0xa24
			TempCCk = PHY_QueryBBReg(Adapter, rCCK0_TxFilter2, bMaskDWord)&bMaskCCK;
			for(i=0 ; i<CCK_TABLE_SIZE ; i++)
			{
				if(pHalData->bCCKinCH14)
				{
					if(PlatformCompareMemory((void*)&TempCCk, (void*)&CCKSwingTable_Ch14[i][2], 4)==0)
					{
						CCK_index_old =(u1Byte) i;
						RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD,("Initial reg0x%x = 0x%x, CCK_index=0x%x, ch 14 %d\n", 
							rCCK0_TxFilter2, TempCCk, CCK_index_old, pHalData->bCCKinCH14));
						break;
					}
				}
				else
				{
					if(PlatformCompareMemory((void*)&TempCCk, (void*)&CCKSwingTable_Ch1_Ch13[i][2], 4)==0)
					{
						CCK_index_old =(u1Byte) i;
						RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD,("Initial reg0x%x = 0x%x, CCK_index=0x%x, ch14 %d\n", 
							rCCK0_TxFilter2, TempCCk, CCK_index_old, pHalData->bCCKinCH14));
						break;
					}			
				}
			}	

			if(!pHalData->ThermalValue)
			{
				pHalData->ThermalValue = pHalData->EEPROMThermalMeter;
				pHalData->ThermalValue_LCK = ThermalValue;				
				pHalData->ThermalValue_IQK = ThermalValue;								
				pHalData->ThermalValue_DPK = pHalData->EEPROMThermalMeter;
				
#if DEV_BUS_TYPE==RT_USB_INTERFACE				
				for(i = 0; i < rf; i++)
					pHalData->OFDM_index_HP[i] = pHalData->OFDM_index[i] = OFDM_index_old[i];
				pHalData->CCK_index_HP = pHalData->CCK_index = CCK_index_old;
#else
				for(i = 0; i < rf; i++)
					pHalData->OFDM_index[i] = OFDM_index_old[i];
				pHalData->CCK_index = CCK_index_old;
#endif
			}	

#if DEV_BUS_TYPE==RT_USB_INTERFACE				
			if(RT_GetInterfaceSelection(Adapter) == INTF_SEL1_USB_High_Power)
			{
				pHalData->ThermalValue_HP[pHalData->ThermalValue_HP_index] = ThermalValue;
				pHalData->ThermalValue_HP_index++;
				if(pHalData->ThermalValue_HP_index == HP_THERMAL_NUM)
					pHalData->ThermalValue_HP_index = 0;

				for(i = 0; i < HP_THERMAL_NUM; i++)
				{
					if(pHalData->ThermalValue_HP[i])
					{
						ThermalValue_HP += pHalData->ThermalValue_HP[i];
						ThermalValue_HP_count++;
					}			
				}
		
				if(ThermalValue_HP_count)
					ThermalValue = (u1Byte)(ThermalValue_HP / ThermalValue_HP_count);
			}
#endif
		}
		
		delta = (ThermalValue > pHalData->ThermalValue)?(ThermalValue - pHalData->ThermalValue):(pHalData->ThermalValue - ThermalValue);
#if DEV_BUS_TYPE==RT_USB_INTERFACE				
		if(RT_GetInterfaceSelection(Adapter) == INTF_SEL1_USB_High_Power)
		{
			if(pHalData->bDoneTxpower)
				delta_HP = (ThermalValue > pHalData->ThermalValue)?(ThermalValue - pHalData->ThermalValue):(pHalData->ThermalValue - ThermalValue);
			else
				delta_HP = ThermalValue > pHalData->EEPROMThermalMeter?(ThermalValue - pHalData->EEPROMThermalMeter):(pHalData->EEPROMThermalMeter - ThermalValue);						
		}
		else
#endif	
		{
			delta_HP = 0;			
		}
		delta_LCK = (ThermalValue > pHalData->ThermalValue_LCK)?(ThermalValue - pHalData->ThermalValue_LCK):(pHalData->ThermalValue_LCK - ThermalValue);
		delta_IQK = (ThermalValue > pHalData->ThermalValue_IQK)?(ThermalValue - pHalData->ThermalValue_IQK):(pHalData->ThermalValue_IQK - ThermalValue);

		RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD,("Readback Thermal Meter = 0x%x pre thermal meter 0x%x EEPROMthermalmeter 0x%x delta 0x%x delta_LCK 0x%x delta_IQK 0x%x\n", ThermalValue, pHalData->ThermalValue, pHalData->EEPROMThermalMeter, delta, delta_LCK, delta_IQK));

		if(delta_LCK > 1)
		{
			pHalData->ThermalValue_LCK = ThermalValue;
			PHY_LCCalibrate_8192C(Adapter);
		}
		
		if((delta > 0 || delta_HP > 0)&& pHalData->TxPowerTrackControl)
		{
#if DEV_BUS_TYPE==RT_USB_INTERFACE		
			if(RT_GetInterfaceSelection(Adapter) == INTF_SEL1_USB_High_Power)
			{
				pHalData->bDoneTxpower = TRUE;
				delta_HP = ThermalValue > pHalData->EEPROMThermalMeter?(ThermalValue - pHalData->EEPROMThermalMeter):(pHalData->EEPROMThermalMeter - ThermalValue);						
				
				if(delta_HP > index_mapping_HP_NUM-1)					
					index_HP = index_mapping_HP[index_mapping_HP_NUM-1];
				else
					index_HP = index_mapping_HP[delta_HP];
				
				if(ThermalValue > pHalData->EEPROMThermalMeter)	//set larger Tx power
				{
					for(i = 0; i < rf; i++)
					 	OFDM_index[i] = pHalData->OFDM_index_HP[i] - index_HP;
					CCK_index = pHalData->CCK_index_HP -index_HP;						
				}
				else
				{
					for(i = 0; i < rf; i++)
						OFDM_index[i] = pHalData->OFDM_index_HP[i] + index_HP;
					CCK_index = pHalData->CCK_index_HP + index_HP;						
				}	
				
				delta_HP = (ThermalValue > pHalData->ThermalValue)?(ThermalValue - pHalData->ThermalValue):(pHalData->ThermalValue - ThermalValue);
				
			}
			else
#endif				
			{
				if(ThermalValue > pHalData->ThermalValue)
				{ 
					for(i = 0; i < rf; i++)
					 	pHalData->OFDM_index[i] -= delta;
					pHalData->CCK_index -= delta;
				}
				else
				{
					for(i = 0; i < rf; i++)			
						pHalData->OFDM_index[i] += delta;
					pHalData->CCK_index += delta;
				}
			}
			
			if(is2T)
			{
				RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD,("temp OFDM_A_index=0x%x, OFDM_B_index=0x%x, CCK_index=0x%x\n", 
					pHalData->OFDM_index[0], pHalData->OFDM_index[1], pHalData->CCK_index));			
			}
			else
			{
				RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD,("temp OFDM_A_index=0x%x, CCK_index=0x%x\n", 
					pHalData->OFDM_index[0], pHalData->CCK_index));			
			}
			
			//no adjust
#if DEV_BUS_TYPE==RT_USB_INTERFACE					
			if(RT_GetInterfaceSelection(Adapter) != INTF_SEL1_USB_High_Power)
#endif				
			{
				if(ThermalValue > pHalData->EEPROMThermalMeter)
				{
					for(i = 0; i < rf; i++)			
						OFDM_index[i] = pHalData->OFDM_index[i]+1;
					CCK_index = pHalData->CCK_index+1;			
				}
				else
				{
					for(i = 0; i < rf; i++)			
						OFDM_index[i] = pHalData->OFDM_index[i];
					CCK_index = pHalData->CCK_index;						
				}

#if MP_DRIVER == 1
				for(i = 0; i < rf; i++)
				{
					if(TxPwrLevel[i] >=0 && TxPwrLevel[i] <=26)
					{
						if(ThermalValue > pHalData->EEPROMThermalMeter)
						{
							if (delta < 5)
								OFDM_index[i] -= 1;					
							else 
								OFDM_index[i] -= 2;					
						}
						else if(delta > 5 && ThermalValue < pHalData->EEPROMThermalMeter)
						{
							OFDM_index[i] += 1;
						}
					}
					else if (TxPwrLevel[i] >= 27 && TxPwrLevel[i] <= 32 && ThermalValue > pHalData->EEPROMThermalMeter)
					{
						if (delta < 5)
							OFDM_index[i] -= 1;					
						else 
							OFDM_index[i] -= 2;								
					}
					else if (TxPwrLevel[i] >= 32 && TxPwrLevel[i] <= 38 && ThermalValue > pHalData->EEPROMThermalMeter && delta > 5)
					{
						OFDM_index[i] -= 1;								
					}
				}

				{
					if(TxPwrLevel[i] >=0 && TxPwrLevel[i] <=26)
					{
						if(ThermalValue > pHalData->EEPROMThermalMeter)
						{
							if (delta < 5)
								CCK_index -= 1; 				
							else 
								CCK_index -= 2; 				
						}
						else if(delta > 5 && ThermalValue < pHalData->EEPROMThermalMeter)
						{
							CCK_index += 1;
						}
					}
					else if (TxPwrLevel[i] >= 27 && TxPwrLevel[i] <= 32 && ThermalValue > pHalData->EEPROMThermalMeter)
					{
						if (delta < 5)
							CCK_index -= 1; 				
						else 
							CCK_index -= 2; 							
					}
					else if (TxPwrLevel[i] >= 32 && TxPwrLevel[i] <= 38 && ThermalValue > pHalData->EEPROMThermalMeter && delta > 5)
					{
						CCK_index -= 1; 							
					}
				}
#endif				
			}
			
			for(i = 0; i < rf; i++)
			{
				if(OFDM_index[i] > (OFDM_TABLE_SIZE_92C-1))
					OFDM_index[i] = (OFDM_TABLE_SIZE_92C-1);
				else if (OFDM_index[i] < OFDM_min_index)
					OFDM_index[i] = OFDM_min_index;
			}
						
			if(CCK_index > (CCK_TABLE_SIZE-1))
				CCK_index = (CCK_TABLE_SIZE-1);
			else if (CCK_index < 0)
				CCK_index = 0;		

			if(is2T)
			{
				RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD,("new OFDM_A_index=0x%x, OFDM_B_index=0x%x, CCK_index=0x%x\n", 
					OFDM_index[0], OFDM_index[1], CCK_index));
			}
			else
			{
				RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD,("new OFDM_A_index=0x%x, CCK_index=0x%x\n", 
					OFDM_index[0], CCK_index));			
			}
		}

		if(pHalData->TxPowerTrackControl && (delta != 0 || delta_HP != 0))
		{
			//Adujst OFDM Ant_A according to IQK result
			ele_D = (OFDMSwingTable[OFDM_index[0]] & 0xFFC00000)>>22;
			X = pHalData->RegE94;
			Y = pHalData->RegE9C;		

			if(X != 0)
			{
				if ((X & 0x00000200) != 0)
					X = X | 0xFFFFFC00;
				ele_A = ((X * ele_D)>>8)&0x000003FF;
					
				//new element C = element D x Y
				if ((Y & 0x00000200) != 0)
					Y = Y | 0xFFFFFC00;
				ele_C = ((Y * ele_D)>>8)&0x000003FF;
				
				//wirte new elements A, C, D to regC80 and regC94, element B is always 0
				value32 = (ele_D<<22)|((ele_C&0x3F)<<16)|ele_A;
				PHY_SetBBReg(Adapter, rOFDM0_XATxIQImbalance, bMaskDWord, value32);
				
				value32 = (ele_C&0x000003C0)>>6;
				PHY_SetBBReg(Adapter, rOFDM0_XCTxAFE, bMaskH4Bits, value32);

				value32 = ((X * ele_D)>>7)&0x01;
				PHY_SetBBReg(Adapter, rOFDM0_ECCAThreshold, BIT31, value32);

				value32 = ((Y * ele_D)>>7)&0x01;
				PHY_SetBBReg(Adapter, rOFDM0_ECCAThreshold, BIT29, value32);
				
			}
			else
			{
				PHY_SetBBReg(Adapter, rOFDM0_XATxIQImbalance, bMaskDWord, OFDMSwingTable[OFDM_index[0]]);				
				PHY_SetBBReg(Adapter, rOFDM0_XCTxAFE, bMaskH4Bits, 0x00);
				PHY_SetBBReg(Adapter, rOFDM0_ECCAThreshold, BIT31|BIT29, 0x00);			
			}

			RTPRINT(FINIT, INIT_IQK, ("TxPwrTracking path A: X = 0x%x, Y = 0x%x ele_A = 0x%x ele_C = 0x%x ele_D = 0x%x\n", X, Y, ele_A, ele_C, ele_D));		

			//Adjust CCK according to IQK result
			if(!pHalData->bCCKinCH14){
				PlatformEFIOWrite1Byte(Adapter, 0xa22, CCKSwingTable_Ch1_Ch13[CCK_index][0]);
				PlatformEFIOWrite1Byte(Adapter, 0xa23, CCKSwingTable_Ch1_Ch13[CCK_index][1]);
				PlatformEFIOWrite1Byte(Adapter, 0xa24, CCKSwingTable_Ch1_Ch13[CCK_index][2]);
				PlatformEFIOWrite1Byte(Adapter, 0xa25, CCKSwingTable_Ch1_Ch13[CCK_index][3]);
				PlatformEFIOWrite1Byte(Adapter, 0xa26, CCKSwingTable_Ch1_Ch13[CCK_index][4]);
				PlatformEFIOWrite1Byte(Adapter, 0xa27, CCKSwingTable_Ch1_Ch13[CCK_index][5]);
				PlatformEFIOWrite1Byte(Adapter, 0xa28, CCKSwingTable_Ch1_Ch13[CCK_index][6]);
				PlatformEFIOWrite1Byte(Adapter, 0xa29, CCKSwingTable_Ch1_Ch13[CCK_index][7]);		
			}
			else{
				PlatformEFIOWrite1Byte(Adapter, 0xa22, CCKSwingTable_Ch14[CCK_index][0]);
				PlatformEFIOWrite1Byte(Adapter, 0xa23, CCKSwingTable_Ch14[CCK_index][1]);
				PlatformEFIOWrite1Byte(Adapter, 0xa24, CCKSwingTable_Ch14[CCK_index][2]);
				PlatformEFIOWrite1Byte(Adapter, 0xa25, CCKSwingTable_Ch14[CCK_index][3]);
				PlatformEFIOWrite1Byte(Adapter, 0xa26, CCKSwingTable_Ch14[CCK_index][4]);
				PlatformEFIOWrite1Byte(Adapter, 0xa27, CCKSwingTable_Ch14[CCK_index][5]);
				PlatformEFIOWrite1Byte(Adapter, 0xa28, CCKSwingTable_Ch14[CCK_index][6]);
				PlatformEFIOWrite1Byte(Adapter, 0xa29, CCKSwingTable_Ch14[CCK_index][7]);	
			}		

			if(is2T)
			{						
				ele_D = (OFDMSwingTable[OFDM_index[1]] & 0xFFC00000)>>22;
				
				//new element A = element D x X
				X = pHalData->RegEB4;
				Y = pHalData->RegEBC;
				
				if(X != 0){
					if ((X & 0x00000200) != 0)	//consider minus
						X = X | 0xFFFFFC00;
					ele_A = ((X * ele_D)>>8)&0x000003FF;
					
					//new element C = element D x Y
					if ((Y & 0x00000200) != 0)
						Y = Y | 0xFFFFFC00;
					ele_C = ((Y * ele_D)>>8)&0x00003FF;
					
					//wirte new elements A, C, D to regC88 and regC9C, element B is always 0
					value32=(ele_D<<22)|((ele_C&0x3F)<<16) |ele_A;
					PHY_SetBBReg(Adapter, rOFDM0_XBTxIQImbalance, bMaskDWord, value32);

					value32 = (ele_C&0x000003C0)>>6;
					PHY_SetBBReg(Adapter, rOFDM0_XDTxAFE, bMaskH4Bits, value32);	
					
					value32 = ((X * ele_D)>>7)&0x01;
					PHY_SetBBReg(Adapter, rOFDM0_ECCAThreshold, BIT27, value32);

					value32 = ((Y * ele_D)>>7)&0x01;
					PHY_SetBBReg(Adapter, rOFDM0_ECCAThreshold, BIT25, value32);

				}
				else{
					PHY_SetBBReg(Adapter, rOFDM0_XBTxIQImbalance, bMaskDWord, OFDMSwingTable[OFDM_index[1]]);										
					PHY_SetBBReg(Adapter, rOFDM0_XDTxAFE, bMaskH4Bits, 0x00);	
					PHY_SetBBReg(Adapter, rOFDM0_ECCAThreshold, BIT27|BIT25, 0x00);				
				}

				RTPRINT(FINIT, INIT_IQK, ("TxPwrTracking path B: X = 0x%x, Y = 0x%x ele_A = 0x%x ele_C = 0x%x ele_D = 0x%x\n", X, Y, ele_A, ele_C, ele_D));			
			}

			RTPRINT(FINIT, INIT_IQK, ("TxPwrTracking 0xc80 = 0x%x, 0xc94 = 0x%x RF 0x24 = 0x%x\n", PHY_QueryBBReg(Adapter, 0xc80, bMaskDWord), PHY_QueryBBReg(Adapter, 0xc94, bMaskDWord), PHY_QueryRFReg(Adapter, RF_PATH_A, 0x24, bRFRegOffsetMask)));
		}

#if MP_DRIVER == 1
		if(delta_IQK > 1)
#else
		if(delta_IQK > 3)
#endif			
		{
			pHalData->ThermalValue_IQK = ThermalValue;
			PHY_IQCalibrate_8192C(Adapter, FALSE);
		}

#if 1
		if(delta > 0 && IS_HARDWARE_TYPE_8723A(Adapter))
		{
			if(ThermalValue >= 15)
				PHY_SetBBReg(Adapter, REG_AFE_XTAL_CTRL, bMaskDWord, 0x038180fd );
			else
				PHY_SetBBReg(Adapter, REG_AFE_XTAL_CTRL, bMaskDWord, 0x0381808d );				
		}
#endif

		//update thermal meter value
		if(pHalData->TxPowerTrackControl)
			Adapter->HalFunc.SetHalDefVarHandler(Adapter, HAL_DEF_THERMAL_VALUE, &ThermalValue);
			
	}

	PlatformAtomicExchange(&Adapter->IntrCCKRefCount, FALSE);		
	pHalData->TXPowercount = 0;

	// 2011/08/23 MH Add for power tracking after S3/S4  turn off RF. In this case, we need to execute IQK again. Otherwise
	// The IQK scheme will use old value to save and cause incorrect BB value.
	{
		RT_RF_POWER_STATE 	rtState;

		Adapter->HalFunc.GetHwRegHandler(Adapter, HW_VAR_RF_STATE, (pu1Byte)(&rtState));	
		
		if(Adapter->bDriverStopped || Adapter->bDriverIsGoingToPnpSetPowerSleep || rtState == eRfOff)
		{
			RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD,("Incorrect pwrtrack point, re-iqk next time\n"));	
			pHalData->bIQKInitialized = FALSE;
		}
	}
	
	RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD,("<===odm_TXPowerTrackingCallback_ThermalMeter_92C\n"));	
#endif
}

//#if (RT_PLATFORM == PLATFORM_WINDOWS) && (HAL_CODE_BASE==RTL8192_C)
#if (HAL_CODE_BASE==RTL8192_C)
VOID
dm_RXGainTrackingCallback_ThermalMeter_92D(
	IN PADAPTER 	Adapter)
{
	u1Byte			index_mapping[Rx_index_mapping_NUM] = {
						0x0f,	0x0f,	0x0f,	0x0f,	0x0b,
						0x0a,	0x09,	0x08,	0x07,	0x06,
						0x05,	0x04,	0x04,	0x03,	0x02						
					};

	u1Byte			eRFPath;
	u4Byte			u4tmp;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	u4tmp = (index_mapping[(pHalData->EEPROMThermalMeter - pHalData->ThermalValue_RxGain)]) << 12;

	RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD,("===>dm_RXGainTrackingCallback_ThermalMeter_92D interface %u  Rx Gain %x\n", Adapter->interfaceIndex, u4tmp));
	
	for(eRFPath = RF_PATH_A; eRFPath <pHalData->NumTotalRFPath; eRFPath++)
		PHY_SetRFReg(Adapter, eRFPath, RF_RXRF_A3, bRFRegOffsetMask, (pHalData->RegRF3C[eRFPath]&(~(0xF000)))|u4tmp);

};	
#endif

//091212 chiyokolin
VOID
odm_TXPowerTrackingCallback_ThermalMeter_92D(
            IN PADAPTER	Adapter)
{
//#if (RT_PLATFORM == PLATFORM_WINDOWS) && (HAL_CODE_BASE==RTL8192_C)
#if (HAL_CODE_BASE==RTL8192_C)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u1Byte			ThermalValue = 0, delta, delta_LCK, delta_IQK, delta_RxGain, index, offset;
	u1Byte			ThermalValue_AVG_count = 0;
	u4Byte			ThermalValue_AVG = 0;	
	s4Byte 			ele_A=0, ele_D, TempCCk, X, value32;
	s4Byte			Y, ele_C=0;
	s1Byte			OFDM_index[2], CCK_index=0, OFDM_index_old[2], CCK_index_old=0;
	u4Byte			i = 0;
	BOOLEAN			is2T = (IS_92C_SERIAL(pHalData->VersionID) || IS_92D_SINGLEPHY(pHalData->VersionID)) ;
	BOOLEAN			bInteralPA = FALSE;

	u1Byte			OFDM_min_index = 6, OFDM_min_index_internalPA = 3, rf; //OFDM BB Swing should be less than +3.0dB, which is required by Arthur
	u1Byte			Indexforchannel = GetRightChnlPlaceforIQK(pHalData->CurrentChannel);
	u1Byte			index_mapping[5][index_mapping_NUM] = {	
					{0,	1,	3,	6,	8,	9,				//5G, path A/MAC 0, decrease power 
					11,	13,	14,	16,	17,	18, 18},	
					{0,	2,	4,	5,	7,	10,				//5G, path A/MAC 0, increase power 
					12,	14,	16,	18,	18,	18,	18},					
					{0,	2,	3,	6,	8,	9,				//5G, path B/MAC 1, decrease power
					11,	13,	14,	16,	17,	18,	18},		
					{0,	2,	4,	5,	7,	10,				//5G, path B/MAC 1, increase power
					13,	16,	16,	18,	18,	18,	18},					
					{0,	1,	2,	3,	4,	5,				//2.4G, for decreas power
					6,	7,	7,	8,	9,	10,	10},												
					};

u1Byte				index_mapping_internalPA[8][index_mapping_NUM] = { 
					{0, 	1,	2,	4,	6,	7,				//5G, path A/MAC 0, ch36-64, decrease power 
					9, 	11, 	12, 	14, 	15, 	16, 	16},	
					{0, 	2,	4,	5,	7,	10, 				//5G, path A/MAC 0, ch36-64, increase power 
					12, 	14, 	16, 	18, 	18, 	18, 	18},					
					{0, 	1,	2,	3,	5,	6,				//5G, path A/MAC 0, ch100-165, decrease power 
					8,	10, 	11, 	13, 	14, 	15, 	15},	
					{0, 	2,	4,	5,	7,	10, 				//5G, path A/MAC 0, ch100-165, increase power 
					12, 	14, 	16, 	18, 	18, 	18, 	18},						
					{0, 	1,	2,	4,	6,	7,				//5G, path B/MAC 1, ch36-64, decrease power
					9,	11, 	12, 	14, 	15, 	16, 	16},		
					{0, 	2,	4,	5,	7,	10, 				//5G, path B/MAC 1, ch36-64, increase power
					13, 	16, 	16, 	18, 	18, 	18, 	18},					
					{0, 	1,	2,	3,	5,	6,				//5G, path B/MAC 1, ch100-165, decrease power
					8,	9,	10, 	12, 	13, 	14, 	14},		
					{0, 	2,	4,	5,	7,	10, 				//5G, path B/MAC 1, ch100-165, increase power
					13, 	16, 	16, 	18, 	18, 	18, 	18},																						
				};

//#if MP_DRIVER != 1
//	return;
//#endif
	

	pHalData->TXPowerTrackingCallbackCnt++;	//cosa add for debug
	pHalData->bTXPowerTrackingInit = TRUE;

	RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD,("===>dm_TXPowerTrackingCallback_ThermalMeter_92D interface %u txpowercontrol %d\n", Adapter->interfaceIndex, pHalData->TxPowerTrackControl));

	ThermalValue = (u1Byte)PHY_QueryRFReg(Adapter, RF_PATH_A, RF_T_METER_92D, 0xf800);	//0x42: RF Reg[15:11] 92D

	RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD,("Readback Thermal Meter = 0x%x pre thermal meter 0x%x EEPROMthermalmeter 0x%x\n", ThermalValue, pHalData->ThermalValue, pHalData->EEPROMThermalMeter));

	PHY_APCalibrate_8192C(Adapter, (ThermalValue - pHalData->EEPROMThermalMeter));

//	if(!pHalData->TxPowerTrackControl)
//		return;

	if(is2T)
		rf = 2;
	else
		rf = 1;
	
	if(ThermalValue)
	{
//		if(!pHalData->ThermalValue)
		{
			//Query OFDM path A default setting 		
			ele_D = PHY_QueryBBReg(Adapter, rOFDM0_XATxIQImbalance, bMaskDWord)&bMaskOFDM_D;
			for(i=0; i<OFDM_TABLE_SIZE_92D; i++)	//find the index
			{
				if(ele_D == (OFDMSwingTable[i]&bMaskOFDM_D))
				{
					OFDM_index_old[0] = (u1Byte)i;
					RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD,("Initial pathA ele_D reg0x%x = 0x%x, OFDM_index=0x%x\n", 
						rOFDM0_XATxIQImbalance, ele_D, OFDM_index_old[0]));
					break;
				}
			}

			//Query OFDM path B default setting 
			if(is2T)
			{
				ele_D = PHY_QueryBBReg(Adapter, rOFDM0_XBTxIQImbalance, bMaskDWord)&bMaskOFDM_D;
				for(i=0; i<OFDM_TABLE_SIZE_92D; i++)	//find the index
				{
					if(ele_D == (OFDMSwingTable[i]&bMaskOFDM_D))
					{
						OFDM_index_old[1] = (u1Byte)i;
						RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD,("Initial pathB ele_D reg0x%x = 0x%x, OFDM_index=0x%x\n", 
							rOFDM0_XBTxIQImbalance, ele_D, OFDM_index_old[1]));
						break;
					}
				}
			}
			
			if(pHalData->CurrentBandType92D == BAND_ON_2_4G)
			{
				//Query CCK default setting From 0xa24
				TempCCk = pHalData->RegA24;

				for(i=0 ; i<CCK_TABLE_SIZE ; i++)
				{
					if(pHalData->bCCKinCH14)
					{
						if(PlatformCompareMemory((void*)&TempCCk, (void*)&CCKSwingTable_Ch14[i][2], 4)==0)
						{
							CCK_index_old =(u1Byte) i;
							RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD,("Initial reg0x%x = 0x%x, CCK_index=0x%x, ch 14 %d\n", 
								rCCK0_TxFilter2, TempCCk, CCK_index_old, pHalData->bCCKinCH14));
							break;
						}
					}
					else
					{
						if(PlatformCompareMemory((void*)&TempCCk, (void*)&CCKSwingTable_Ch1_Ch13[i][2], 4)==0)
						{
							CCK_index_old =(u1Byte) i;
							RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD,("Initial reg0x%x = 0x%x, CCK_index=0x%x, ch14 %d\n", 
								rCCK0_TxFilter2, TempCCk, CCK_index_old, pHalData->bCCKinCH14));
							break;
						}			
					}
				}
			}
			else
			{
				TempCCk = 0x090e1317;
				CCK_index_old = 12;
			}

			if(!pHalData->ThermalValue)
			{
				pHalData->ThermalValue = pHalData->EEPROMThermalMeter;
				pHalData->ThermalValue_LCK = ThermalValue;				
				pHalData->ThermalValue_IQK = ThermalValue;								
				pHalData->ThermalValue_RxGain = pHalData->EEPROMThermalMeter;		
				
				for(i = 0; i < rf; i++)
					pHalData->OFDM_index[i] = OFDM_index_old[i];
				pHalData->CCK_index = CCK_index_old;
			}			

			if(pHalData->bReloadtxpowerindex)
			{
				RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD,("reload ofdm index for band switch\n"));				
			}

			//calculate average thermal meter
			{
				pHalData->ThermalValue_AVG[pHalData->ThermalValue_AVG_index] = ThermalValue;
				pHalData->ThermalValue_AVG_index++;
				if(pHalData->ThermalValue_AVG_index == AVG_THERMAL_NUM)
					pHalData->ThermalValue_AVG_index = 0;

				for(i = 0; i < AVG_THERMAL_NUM; i++)
				{
					if(pHalData->ThermalValue_AVG[i])
					{
						ThermalValue_AVG += pHalData->ThermalValue_AVG[i];
						ThermalValue_AVG_count++;
					}
				}

				if(ThermalValue_AVG_count)
					ThermalValue = (u1Byte)(ThermalValue_AVG / ThermalValue_AVG_count);
			}			
		}

		if(pHalData->bReloadtxpowerindex)
		{
			delta = ThermalValue > pHalData->EEPROMThermalMeter?(ThermalValue - pHalData->EEPROMThermalMeter):(pHalData->EEPROMThermalMeter - ThermalValue);				
			pHalData->bReloadtxpowerindex = FALSE;	
			pHalData->bDoneTxpower = FALSE;
		}
		else if(pHalData->bDoneTxpower)
		{
			delta = (ThermalValue > pHalData->ThermalValue)?(ThermalValue - pHalData->ThermalValue):(pHalData->ThermalValue - ThermalValue);
		}
		else
		{
			delta = ThermalValue > pHalData->EEPROMThermalMeter?(ThermalValue - pHalData->EEPROMThermalMeter):(pHalData->EEPROMThermalMeter - ThermalValue);		
		}
		delta_LCK = (ThermalValue > pHalData->ThermalValue_LCK)?(ThermalValue - pHalData->ThermalValue_LCK):(pHalData->ThermalValue_LCK - ThermalValue);
		delta_IQK = (ThermalValue > pHalData->ThermalValue_IQK)?(ThermalValue - pHalData->ThermalValue_IQK):(pHalData->ThermalValue_IQK - ThermalValue);
		delta_RxGain = (ThermalValue > pHalData->ThermalValue_RxGain)?(ThermalValue - pHalData->ThermalValue_RxGain):(pHalData->ThermalValue_RxGain - ThermalValue);

		RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD,("interface %u Readback Thermal Meter = 0x%x pre thermal meter 0x%x EEPROMthermalmeter 0x%x delta 0x%x delta_LCK 0x%x delta_IQK 0x%x delta_RxGain 0x%x\n",  Adapter->interfaceIndex, ThermalValue, pHalData->ThermalValue, pHalData->EEPROMThermalMeter, delta, delta_LCK, delta_IQK, delta_RxGain));
		RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD,("interface %u pre thermal meter LCK 0x%x pre thermal meter IQK 0x%x delta_LCK_bound 0x%x delta_IQK_bound 0x%x\n",  Adapter->interfaceIndex, pHalData->ThermalValue_LCK, pHalData->ThermalValue_IQK, pHalData->Delta_LCK, pHalData->Delta_IQK));

		if((delta_LCK > pHalData->Delta_LCK) && (pHalData->Delta_LCK != 0))
		{
			pHalData->ThermalValue_LCK = ThermalValue;
			PHY_LCCalibrate_8192C(Adapter);
		}
		
		if(delta > 0 && pHalData->TxPowerTrackControl)
		{
			delta = ThermalValue > pHalData->EEPROMThermalMeter?(ThermalValue - pHalData->EEPROMThermalMeter):(pHalData->EEPROMThermalMeter - ThermalValue);		

			//calculate new OFDM / CCK offset	
			{
				if(pHalData->CurrentBandType92D == BAND_ON_2_4G)
				{
					offset = 4;
				
					if(delta > index_mapping_NUM-1)					
						index = index_mapping[offset][index_mapping_NUM-1];
					else
						index = index_mapping[offset][delta];
				
					if(ThermalValue > pHalData->EEPROMThermalMeter)
					{ 
						for(i = 0; i < rf; i++)
						 	OFDM_index[i] = pHalData->OFDM_index[i] -delta;
						CCK_index = pHalData->CCK_index -delta;
					}
					else
					{
						for(i = 0; i < rf; i++)			
							OFDM_index[i] = pHalData->OFDM_index[i] + index;
						CCK_index = pHalData->CCK_index + index;
					}
				}
				else if(pHalData->CurrentBandType92D == BAND_ON_5G)
				{
					for(i = 0; i < rf; i++)
					{
						if(pHalData->MacPhyMode92D == DUALMAC_DUALPHY &&
							Adapter->interfaceIndex == 1)		//MAC 1 5G
							bInteralPA = pHalData->InternalPA5G[1];
						else
							bInteralPA = pHalData->InternalPA5G[i];	
					
						if(bInteralPA)
						{
							if(Adapter->interfaceIndex == 1 || i == rf)
								offset = 4;
							else
								offset = 0;

							if(pHalData->CurrentChannel >= 100 && pHalData->CurrentChannel <= 165)
								offset += 2;													
						}
						else
						{					
						if(Adapter->interfaceIndex == 1 || i == rf)
							offset = 2;
						else
							offset = 0;
						}

						if(ThermalValue > pHalData->EEPROMThermalMeter)	//set larger Tx power
							offset++;		
						
						if(bInteralPA)
						{
							if(delta > index_mapping_NUM-1)					
								index = index_mapping_internalPA[offset][index_mapping_NUM-1];
							else
								index = index_mapping_internalPA[offset][delta];						
						}
						else
						{						
						if(delta > index_mapping_NUM-1)					
							index = index_mapping[offset][index_mapping_NUM-1];
						else
							index = index_mapping[offset][delta];
						}
						
						if(ThermalValue > pHalData->EEPROMThermalMeter)	//set larger Tx power
						{
							if(bInteralPA && ThermalValue > 0x12)
							{
								 OFDM_index[i] = pHalData->OFDM_index[i] -((delta/2)*3+(delta%2));							
							}
							else	
							{
							 OFDM_index[i] = pHalData->OFDM_index[i] -index;
						}
						}
						else
						{				
							OFDM_index[i] = pHalData->OFDM_index[i] + index;
						}
					}
				}
				
				if(is2T)
				{
					RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD,("temp OFDM_A_index=0x%x, OFDM_B_index=0x%x, CCK_index=0x%x\n", 
						pHalData->OFDM_index[0], pHalData->OFDM_index[1], pHalData->CCK_index));			
				}
				else
				{
					RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD,("temp OFDM_A_index=0x%x, CCK_index=0x%x\n", 
						pHalData->OFDM_index[0], pHalData->CCK_index));			
				}
				
				for(i = 0; i < rf; i++)
				{
					if(OFDM_index[i] > OFDM_TABLE_SIZE_92D-1)
					{
						OFDM_index[i] = OFDM_TABLE_SIZE_92D-1;
					}
					else if(bInteralPA || pHalData->CurrentBandType92D == BAND_ON_2_4G)
					{
						if (OFDM_index[i] < OFDM_min_index_internalPA)
							OFDM_index[i] = OFDM_min_index_internalPA;
					}
					else if (OFDM_index[i] < OFDM_min_index)
					{
						OFDM_index[i] = OFDM_min_index;
				}
				}

				if(pHalData->CurrentBandType92D == BAND_ON_2_4G)
				{
					if(CCK_index > CCK_TABLE_SIZE-1)
						CCK_index = CCK_TABLE_SIZE-1;
					else if (CCK_index < 0)
						CCK_index = 0;
				}

				if(is2T)
				{
					RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD,("new OFDM_A_index=0x%x, OFDM_B_index=0x%x, CCK_index=0x%x\n", 
						OFDM_index[0], OFDM_index[1], CCK_index));
				}
				else
				{
					RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD,("new OFDM_A_index=0x%x, CCK_index=0x%x\n", 
						OFDM_index[0], CCK_index));	
				}
			}

			//Config by SwingTable
			if(pHalData->TxPowerTrackControl && !pHalData->bNOPG)			
			{
				pHalData->bDoneTxpower = TRUE;			

				//Adujst OFDM Ant_A according to IQK result
				ele_D = (OFDMSwingTable[(u1Byte)OFDM_index[0]] & 0xFFC00000)>>22;
//				X = pHalData->RegE94;
//				Y = pHalData->RegE9C;		
				X = pHalData->IQKMatrixRegSetting[Indexforchannel].Value[0][0];
				Y = pHalData->IQKMatrixRegSetting[Indexforchannel].Value[0][1];

				if(X != 0 && pHalData->CurrentBandType92D == BAND_ON_2_4G)
				{
					if ((X & 0x00000200) != 0)
						X = X | 0xFFFFFC00;
					ele_A = ((X * ele_D)>>8)&0x000003FF;
						
					//new element C = element D x Y
					if ((Y & 0x00000200) != 0)
						Y = Y | 0xFFFFFC00;
					ele_C = ((Y * ele_D)>>8)&0x000003FF;
					
					//wirte new elements A, C, D to regC80 and regC94, element B is always 0
					value32 = (ele_D<<22)|((ele_C&0x3F)<<16)|ele_A;
					PHY_SetBBReg(Adapter, rOFDM0_XATxIQImbalance, bMaskDWord, value32);

					value32 = (ele_C&0x000003C0)>>6;
					PHY_SetBBReg(Adapter, rOFDM0_XCTxAFE, bMaskH4Bits, value32);

					value32 = ((X * ele_D)>>7)&0x01;
					PHY_SetBBReg(Adapter, rOFDM0_ECCAThreshold, BIT24, value32);
					
				}
				else
				{
					PHY_SetBBReg(Adapter, rOFDM0_XATxIQImbalance, bMaskDWord, OFDMSwingTable[(u1Byte)OFDM_index[0]]);				
					PHY_SetBBReg(Adapter, rOFDM0_XCTxAFE, bMaskH4Bits, 0x00);
					PHY_SetBBReg(Adapter, rOFDM0_ECCAThreshold, BIT24, 0x00);			
				}

				RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD, ("TxPwrTracking for interface %u path A: X = 0x%x, Y = 0x%x ele_A = 0x%x ele_C = 0x%x ele_D = 0x%x 0xe94 = 0x%x 0xe9c = 0x%x\n", 
					(u1Byte)Adapter->interfaceIndex, (u4Byte)X, (u4Byte)Y, (u4Byte)ele_A, (u4Byte)ele_C, (u4Byte)ele_D, (u4Byte)X, (u4Byte)Y));		

				
				if(pHalData->CurrentBandType92D == BAND_ON_2_4G)
				{
					//Adjust CCK according to IQK result
					if(!pHalData->bCCKinCH14){
						PlatformEFIOWrite1Byte(Adapter, 0xa22, CCKSwingTable_Ch1_Ch13[(u1Byte)CCK_index][0]);
						PlatformEFIOWrite1Byte(Adapter, 0xa23, CCKSwingTable_Ch1_Ch13[(u1Byte)CCK_index][1]);
						PlatformEFIOWrite1Byte(Adapter, 0xa24, CCKSwingTable_Ch1_Ch13[(u1Byte)CCK_index][2]);
						PlatformEFIOWrite1Byte(Adapter, 0xa25, CCKSwingTable_Ch1_Ch13[(u1Byte)CCK_index][3]);
						PlatformEFIOWrite1Byte(Adapter, 0xa26, CCKSwingTable_Ch1_Ch13[(u1Byte)CCK_index][4]);
						PlatformEFIOWrite1Byte(Adapter, 0xa27, CCKSwingTable_Ch1_Ch13[(u1Byte)CCK_index][5]);
						PlatformEFIOWrite1Byte(Adapter, 0xa28, CCKSwingTable_Ch1_Ch13[(u1Byte)CCK_index][6]);
						PlatformEFIOWrite1Byte(Adapter, 0xa29, CCKSwingTable_Ch1_Ch13[(u1Byte)CCK_index][7]);		
					}
					else{
						PlatformEFIOWrite1Byte(Adapter, 0xa22, CCKSwingTable_Ch14[(u1Byte)CCK_index][0]);
						PlatformEFIOWrite1Byte(Adapter, 0xa23, CCKSwingTable_Ch14[(u1Byte)CCK_index][1]);
						PlatformEFIOWrite1Byte(Adapter, 0xa24, CCKSwingTable_Ch14[(u1Byte)CCK_index][2]);
						PlatformEFIOWrite1Byte(Adapter, 0xa25, CCKSwingTable_Ch14[(u1Byte)CCK_index][3]);
						PlatformEFIOWrite1Byte(Adapter, 0xa26, CCKSwingTable_Ch14[(u1Byte)CCK_index][4]);
						PlatformEFIOWrite1Byte(Adapter, 0xa27, CCKSwingTable_Ch14[(u1Byte)CCK_index][5]);
						PlatformEFIOWrite1Byte(Adapter, 0xa28, CCKSwingTable_Ch14[(u1Byte)CCK_index][6]);
						PlatformEFIOWrite1Byte(Adapter, 0xa29, CCKSwingTable_Ch14[(u1Byte)CCK_index][7]);	
					}		
				}
				
				if(is2T)
				{						
					ele_D = (OFDMSwingTable[(u1Byte)OFDM_index[1]] & 0xFFC00000)>>22;
					
					//new element A = element D x X
//					X = pHalData->RegEB4;
//					Y = pHalData->RegEBC;
					X = pHalData->IQKMatrixRegSetting[Indexforchannel].Value[0][4];
					Y = pHalData->IQKMatrixRegSetting[Indexforchannel].Value[0][5];
					
					if(X != 0 && pHalData->CurrentBandType92D == BAND_ON_2_4G)
					{
						if ((X & 0x00000200) != 0)	//consider minus
							X = X | 0xFFFFFC00;
						ele_A = ((X * ele_D)>>8)&0x000003FF;
						
						//new element C = element D x Y
						if ((Y & 0x00000200) != 0)
							Y = Y | 0xFFFFFC00;
						ele_C = ((Y * ele_D)>>8)&0x00003FF;
						
						//wirte new elements A, C, D to regC88 and regC9C, element B is always 0
						value32=(ele_D<<22)|((ele_C&0x3F)<<16) |ele_A;
						PHY_SetBBReg(Adapter, rOFDM0_XBTxIQImbalance, bMaskDWord, value32);

						value32 = (ele_C&0x000003C0)>>6;
						PHY_SetBBReg(Adapter, rOFDM0_XDTxAFE, bMaskH4Bits, value32);	
						
						value32 = ((X * ele_D)>>7)&0x01;
						PHY_SetBBReg(Adapter, rOFDM0_ECCAThreshold, BIT28, value32);

					}
					else
					{
						PHY_SetBBReg(Adapter, rOFDM0_XBTxIQImbalance, bMaskDWord, OFDMSwingTable[(u1Byte)OFDM_index[1]]);										
						PHY_SetBBReg(Adapter, rOFDM0_XDTxAFE, bMaskH4Bits, 0x00);	
						PHY_SetBBReg(Adapter, rOFDM0_ECCAThreshold, BIT28, 0x00);				
					}

					RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD, ("TxPwrTracking path B: X = 0x%x, Y = 0x%x ele_A = 0x%x ele_C = 0x%x ele_D = 0x%x 0xeb4 = 0x%x 0xebc = 0x%x\n", 
						(u4Byte)X, (u4Byte)Y, (u4Byte)ele_A, (u4Byte)ele_C, (u4Byte)ele_D, (u4Byte)X, (u4Byte)Y));			
				}
				
				RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD, ("TxPwrTracking 0xc80 = 0x%x, 0xc94 = 0x%x RF 0x24 = 0x%x\n", PHY_QueryBBReg(Adapter, 0xc80, bMaskDWord), PHY_QueryBBReg(Adapter, 0xc94, bMaskDWord), PHY_QueryRFReg(Adapter, RF_PATH_A, 0x24, bRFRegOffsetMask)));
			}			
		}
		
		if((delta_IQK > pHalData->Delta_IQK) && (pHalData->Delta_IQK != 0))
		{
			PHY_ResetIQKResult(Adapter);		
			pHalData->ThermalValue_IQK = ThermalValue;
#if (DEV_BUS_TYPE == RT_PCI_INTERFACE)	
#if USE_WORKITEM
			PlatformAcquireMutex(&pHalData->mxChnlBwControl);
#else
			PlatformAcquireSpinLock(Adapter, RT_CHANNEL_AND_BANDWIDTH_SPINLOCK);
#endif
#elif((DEV_BUS_TYPE == RT_USB_INTERFACE) || (DEV_BUS_TYPE == RT_SDIO_INTERFACE))
			PlatformAcquireMutex(&pHalData->mxChnlBwControl);
#endif
			
			PHY_IQCalibrate_8192C(Adapter, FALSE);

#if (DEV_BUS_TYPE == RT_PCI_INTERFACE)	
#if USE_WORKITEM
			PlatformReleaseMutex(&pHalData->mxChnlBwControl);
#else
			PlatformReleaseSpinLock(Adapter, RT_CHANNEL_AND_BANDWIDTH_SPINLOCK);
#endif
#elif((DEV_BUS_TYPE == RT_USB_INTERFACE) || (DEV_BUS_TYPE == RT_SDIO_INTERFACE))
			PlatformReleaseMutex(&pHalData->mxChnlBwControl);
#endif

		}

		if(delta_RxGain > 0 && pHalData->CurrentBandType92D == BAND_ON_5G 
			&& ThermalValue <= pHalData->EEPROMThermalMeter && pHalData->bNOPG == FALSE)
		{
			pHalData->ThermalValue_RxGain = ThermalValue;		
			dm_RXGainTrackingCallback_ThermalMeter_92D(Adapter);
		}

		//update thermal meter value
		if(pHalData->TxPowerTrackControl)
		{
			Adapter->HalFunc.SetHalDefVarHandler(Adapter, HAL_DEF_THERMAL_VALUE, &ThermalValue);
		}
			
	}

	RT_TRACE(COMP_POWER_TRACKING, DBG_LOUD,("<===dm_TXPowerTrackingCallback_ThermalMeter_92D\n"));
	
	pHalData->TXPowercount = 0;
#endif
}

//3============================================================
//3 BB Power Save
//3============================================================

void 
odm_1R_CCA_8192C(
	IN	PADAPTER	pAdapter
	)
{
	HAL_DATA_TYPE*	pHalData = GET_HAL_DATA(pAdapter);
	pPS_T	pDM_PSTable = &pAdapter->DM_PSTable;
//	pPD_T	pDM_PDTable = &pAdapter->DM_PDTable;

	if(pHalData->PathDivCfg == 0)
	{
	if(pDM_PSTable->Rssi_val_min != 0)
	{
		 
		if(pDM_PSTable->PreCCAState == CCA_2R)
		{
			if(pDM_PSTable->Rssi_val_min >= 35)
				pDM_PSTable->CurCCAState = CCA_1R;
			else
				pDM_PSTable->CurCCAState = CCA_2R;
			
		}
		else{
			if(pDM_PSTable->Rssi_val_min <= 30)
				pDM_PSTable->CurCCAState = CCA_2R;
			else
				pDM_PSTable->CurCCAState = CCA_1R;
		}
	}
	else
		pDM_PSTable->CurCCAState=CCA_MAX;
	}
	else
		pDM_PSTable->CurCCAState=CCA_2R;
	
	if(pDM_PSTable->PreCCAState != pDM_PSTable->CurCCAState)
	{
		if(pDM_PSTable->CurCCAState == CCA_1R)
		{
			if(pHalData->RF_Type==RF_2T2R)
			{
				PHY_SetBBReg(pAdapter, rOFDM0_TRxPathEnable  , bMaskByte0, 0x13);
				//PHY_SetBBReg(pAdapter, 0xe70, bMaskByte3, 0x20);
			}
			else
			{
				PHY_SetBBReg(pAdapter, rOFDM0_TRxPathEnable  , bMaskByte0, 0x23);
				//PHY_SetBBReg(pAdapter, 0xe70, 0x7fc00000, 0x10c); // Set RegE70[30:22] = 9b'100001100
			}
		}
		else
		{
			PHY_SetBBReg(pAdapter, rOFDM0_TRxPathEnable  , bMaskByte0, 0x33);
			//PHY_SetBBReg(pAdapter,0xe70, bMaskByte3, 0x63);
		}
		pDM_PSTable->PreCCAState = pDM_PSTable->CurCCAState;
	}
	RT_TRACE(	COMP_BB_POWERSAVING, DBG_LOUD, ("CCAStage = %s\n",(pDM_PSTable->CurCCAState==0)?"1RCCA":"2RCCA"));
}

void
ODM_RF_Saving_8192C(
	IN	PADAPTER	pAdapter,
	IN	u1Byte	bForceInNormal 
	)
{
	static u1Byte	initialize = 0;
	static u4Byte	Reg874,RegC70,Reg85C,RegA74;
	pPS_T	pDM_PSTable = &pAdapter->DM_PSTable;

	if(initialize == 0){
		Reg874 = (PHY_QueryBBReg(pAdapter, rFPGA0_XCD_RFInterfaceSW, bMaskDWord)&0x1CC000)>>14;
		RegC70 = (PHY_QueryBBReg(pAdapter, rOFDM0_AGCParameter1, bMaskDWord)&BIT3)>>3;
		Reg85C = (PHY_QueryBBReg(pAdapter, rFPGA0_XCD_SwitchControl, bMaskDWord)&0xFF000000)>>24;
		RegA74 = (PHY_QueryBBReg(pAdapter, 0xa74, bMaskDWord)&0xF000)>>12;
		//Reg818 = PHY_QueryBBReg(pAdapter, 0x818, bMaskDWord);
		initialize = 1;
	}

	if(!bForceInNormal)
	{
		if(pDM_PSTable->Rssi_val_min != 0)
		{
			 
			if(pDM_PSTable->PreRFState == RF_Normal)
			{
				if(pDM_PSTable->Rssi_val_min >= 30)
					pDM_PSTable->CurRFState = RF_Save;
				else
					pDM_PSTable->CurRFState = RF_Normal;
			}
			else{
				if(pDM_PSTable->Rssi_val_min <= 25)
					pDM_PSTable->CurRFState = RF_Normal;
				else
					pDM_PSTable->CurRFState = RF_Save;
			}
		}
		else
			pDM_PSTable->CurRFState=RF_MAX;
	}
	else
	{
		pDM_PSTable->CurRFState = RF_Normal;
	}
	
	if(pDM_PSTable->PreRFState != pDM_PSTable->CurRFState)
	{
		if(pDM_PSTable->CurRFState == RF_Save)
		{
			// <tynli_note> 8723 RSSI report will be wrong. Set 0x874[5]=1 when enter BB power saving mode.
			// Suggested by SD3 Yu-Nan. 2011.01.20.
			if(IS_HARDWARE_TYPE_8723A(pAdapter))
			{
				PHY_SetBBReg(pAdapter, rFPGA0_XCD_RFInterfaceSW  , BIT5, 0x1); //Reg874[5]=1b'1
			}
			PHY_SetBBReg(pAdapter, rFPGA0_XCD_RFInterfaceSW  , 0x1C0000, 0x2); //Reg874[20:18]=3'b010
			PHY_SetBBReg(pAdapter, rOFDM0_AGCParameter1, BIT3, 0); //RegC70[3]=1'b0
			PHY_SetBBReg(pAdapter, rFPGA0_XCD_SwitchControl, 0xFF000000, 0x63); //Reg85C[31:24]=0x63
			PHY_SetBBReg(pAdapter, rFPGA0_XCD_RFInterfaceSW, 0xC000, 0x2); //Reg874[15:14]=2'b10
			PHY_SetBBReg(pAdapter, 0xa74, 0xF000, 0x3); //RegA75[7:4]=0x3
			PHY_SetBBReg(pAdapter, 0x818, BIT28, 0x0); //Reg818[28]=1'b0
			PHY_SetBBReg(pAdapter, 0x818, BIT28, 0x1); //Reg818[28]=1'b1
			RT_TRACE(	COMP_BB_POWERSAVING, DBG_LOUD, (" RF_Save"));
		}
		else
		{
			PHY_SetBBReg(pAdapter, rFPGA0_XCD_RFInterfaceSW  , 0x1CC000, Reg874); 
			PHY_SetBBReg(pAdapter, rOFDM0_AGCParameter1, BIT3, RegC70); 
			PHY_SetBBReg(pAdapter, rFPGA0_XCD_SwitchControl, 0xFF000000, Reg85C);
			PHY_SetBBReg(pAdapter, 0xa74, 0xF000, RegA74); 
			PHY_SetBBReg(pAdapter, 0x818, BIT28, 0x0);  
			if(IS_HARDWARE_TYPE_8723A(pAdapter))
			{
				PHY_SetBBReg(pAdapter, rFPGA0_XCD_RFInterfaceSW  , BIT5, 0x0); //Reg874[5]=1b'0
			}
			RT_TRACE(	COMP_BB_POWERSAVING, DBG_LOUD, (" RF_Normal"));
		}
		pDM_PSTable->PreRFState =pDM_PSTable->CurRFState;
	}
}



//============================================================


u4Byte
GetPSDData_8192C(
	PADAPTER	Adapter,
	unsigned int 	point,
	u1Byte initial_gain_psd)
{
	//unsigned int	val, rfval;
	//int	psd_report;
	u4Byte	psd_report;
	
	//HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	//Debug Message
	//val = PHY_QueryBBReg(Adapter,0x908, bMaskDWord);
	//DbgPrint("Reg908 = 0x%x\n",val);
	//val = PHY_QueryBBReg(Adapter,0xDF4, bMaskDWord);
	//rfval = PHY_QueryRFReg(Adapter, RF_PATH_A, 0x00, bRFRegOffsetMask);
	//DbgPrint("RegDF4 = 0x%x, RFReg00 = 0x%x\n",val, rfval);
	//DbgPrint("PHYTXON = %x, OFDMCCA_PP = %x, CCKCCA_PP = %x, RFReg00 = %x\n",
		//(val&BIT25)>>25, (val&BIT14)>>14, (val&BIT15)>>15, rfval);

	//Set DCO frequency index, offset=(40MHz/SamplePts)*point
	PHY_SetBBReg(Adapter, 0x808, 0x3FF, point);

	//Start PSD calculation, Reg808[22]=0->1
	PHY_SetBBReg(Adapter, 0x808, BIT22, 1);
	//Need to wait for HW PSD report
	PlatformStallExecution(30);
	PHY_SetBBReg(Adapter, 0x808, BIT22, 0);
	//Read PSD report, Reg8B4[15:0]
	psd_report = PHY_QueryBBReg(Adapter,0x8B4, bMaskDWord) & 0x0000FFFF;
#if 1//(DEV_BUS_TYPE == RT_PCI_INTERFACE) && ( (RT_PLATFORM == PLATFORM_LINUX) || (RT_PLATFORM == PLATFORM_MACOSX))
	psd_report = (u4Byte) (ConvertTo_dB(psd_report))+(u4Byte)(initial_gain_psd-0x1c);
#else
	psd_report = (int) (20*log10((double)psd_report))+(int)(initial_gain_psd-0x1c);
#endif
	return psd_report;
	
}



VOID
PatchDCTone_8192C(
	PADAPTER	Adapter, 
	pu4Byte		PSD_report,
	u1Byte 		initial_gain_psd
)
{
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	u4Byte	psd_report;

	PHY_SetRFReg(Adapter, RF_PATH_A, 0x18, 0x3FF, 11);
	if(IS_HARDWARE_TYPE_8192D(Adapter))
	{
		if((pHalData->MacPhyMode92D == SINGLEMAC_SINGLEPHY)||(pHalData->MacPhyMode92D == DUALMAC_SINGLEPHY))
		{
			PHY_SetRFReg(Adapter, RF_PATH_B, RF_CHNLBW, 0x3FF, 11);
			PHY_SetRFReg(Adapter, RF_PATH_B, 0x25, 0xfffff, 0x643BC);
			PHY_SetRFReg(Adapter, RF_PATH_B, 0x26, 0xfffff, 0xFC038);
			PHY_SetRFReg(Adapter, RF_PATH_B, 0x27, 0xfffff, 0x77C1A);
			PHY_SetRFReg(Adapter, RF_PATH_B, 0x2B, 0xfffff, 0x41289);
			PHY_SetRFReg(Adapter, RF_PATH_B, 0x2C, 0xfffff, 0x01840);
		}
		else
		{
			PHY_SetRFReg(Adapter, RF_PATH_A, 0x25, 0xfffff, 0x643BC);
			PHY_SetRFReg(Adapter, RF_PATH_A, 0x26, 0xfffff, 0xFC038);
			PHY_SetRFReg(Adapter, RF_PATH_A, 0x27, 0xfffff, 0x77C1A);
			PHY_SetRFReg(Adapter, RF_PATH_A, 0x2B, 0xfffff, 0x41289);
			PHY_SetRFReg(Adapter, RF_PATH_A, 0x2C, 0xfffff, 0x01840);
		}
	}
	//Ch9 DC tone patch
	psd_report = GetPSDData_8192C(Adapter, 96, initial_gain_psd);
	PSD_report[50] = psd_report;
	//Ch13 DC tone patch
	psd_report = GetPSDData_8192C(Adapter, 32, initial_gain_psd);
	PSD_report[70] = psd_report;

	//2 Switch to CH3 to patch CH1 and CH5 DC tone
	PHY_SetRFReg(Adapter, RF_PATH_A, RF_CHNLBW, 0x3FF, 3);
	if(IS_HARDWARE_TYPE_8192D(Adapter))
	{
		if((pHalData->MacPhyMode92D == SINGLEMAC_SINGLEPHY)||(pHalData->MacPhyMode92D == DUALMAC_SINGLEPHY))
		{
			PHY_SetRFReg(Adapter, RF_PATH_B, RF_CHNLBW, 0x3FF, 3);
			//PHY_SetRFReg(Adapter, RF90_PATH_B, 0x25, 0xfffff, 0x643BC);
			//PHY_SetRFReg(Adapter, RF90_PATH_B, 0x26, 0xfffff, 0xFC038);
			PHY_SetRFReg(Adapter, RF_PATH_B, 0x27, 0xfffff, 0x07C1A);
			//PHY_SetRFReg(Adapter, RF90_PATH_B, 0x2B, 0xfffff, 0x61289);
			//PHY_SetRFReg(Adapter, RF90_PATH_B, 0x2C, 0xfffff, 0x01C41);
		}
		else
		{
			//PHY_SetRFReg(Adapter, RF_PATH_A, 0x25, 0xfffff, 0x643BC);
			//PHY_SetRFReg(Adapter, RF_PATH_A, 0x26, 0xfffff, 0xFC038);
			PHY_SetRFReg(Adapter, RF_PATH_A, 0x27, 0xfffff, 0x07C1A);
			//PHY_SetRFReg(Adapter, RF_PATH_A, 0x2B, 0xfffff, 0x61289);
			//PHY_SetRFReg(Adapter, RF_PATH_A, 0x2C, 0xfffff, 0x01C41);
		}
	}
	//Ch1 DC tone patch
	psd_report = GetPSDData_8192C(Adapter, 96, initial_gain_psd);
	PSD_report[10] = psd_report;
	//Ch5 DC tone patch
	psd_report = GetPSDData_8192C(Adapter, 32, initial_gain_psd);
	PSD_report[30] = psd_report;
	

}


VOID
GoodChannelDecision_8192C(
	PADAPTER	Adapter, 
	ps4Byte		PSD_report,
	pu1Byte		PSD_bitmap,
	u1Byte 		RSSI_BT,
	pu1Byte		PSD_bitmap_memory)
{
	pRXHP_T		pRX_HP_Table = &Adapter->DM_RXHP_Table;
	//s4Byte	TH1 =  SSBT-0x15;    // modify TH by Neil Chen
	s4Byte	TH1;
	s4Byte	TH2 = RSSI_BT+85;
	u2Byte    TH3;
//	s4Byte	RegB34;
	u1Byte	bitmap, Smooth_size[3], Smooth_TH[3];
	//u1Byte	psd_bit;
	u4Byte	i,n,j, byte_idx, bit_idx, good_cnt, good_cnt_smoothing, Smooth_Interval[3];
	int 		start_byte_idx,start_bit_idx,cur_byte_idx, cur_bit_idx,NOW_byte_idx ;
	
//	RegB34 = PHY_QueryBBReg(Adapter,0xB34, bMaskDWord)&0xFF;
	 if(IS_HARDWARE_TYPE_8192C(Adapter)||(IS_HARDWARE_TYPE_8192D(Adapter)))     // add by Gary
       {
            TH1 = RSSI_BT + 0x14;  
	}

	Smooth_size[0]=Smooth_Size_1;
	Smooth_size[1]=Smooth_Size_2;
	Smooth_size[2]=Smooth_Size_3;
	Smooth_TH[0]=Smooth_TH_1;
	Smooth_TH[1]=Smooth_TH_2;
	Smooth_TH[2]=Smooth_TH_3;
	Smooth_Interval[0]=16;
	Smooth_Interval[1]=15;
	Smooth_Interval[2]=13;
	good_cnt = 0;
	if(IS_HARDWARE_TYPE_8723AE(Adapter))
	{
		//2 Threshold  

		if(RSSI_BT >=41)
			TH1 = 113;	
		else if(RSSI_BT >=38)   // >= -15dBm
			TH1 = 105;                              //0x69
		else if((RSSI_BT >=33)&(RSSI_BT <38))
			TH1 = 99+(RSSI_BT-33);         //0x63
		else if((RSSI_BT >=26)&(RSSI_BT<33))
			TH1 = 99-(33-RSSI_BT)+2;     //0x5e
 		else if((RSSI_BT >=24)&(RSSI_BT<26))
			TH1 = 88-((RSSI_BT-24)*3);   //0x58
		else if((RSSI_BT >=18)&(RSSI_BT<24))
			TH1 = 77+((RSSI_BT-18)*2);
		else if((RSSI_BT >=14)&(RSSI_BT<18))
			TH1 = 63+((RSSI_BT-14)*2);
		else if((RSSI_BT >=8)&(RSSI_BT<14))
			TH1 = 58+((RSSI_BT-8)*2);
		else if((RSSI_BT >=3)&(RSSI_BT<8))
			TH1 = 52+(RSSI_BT-3);
		else
			TH1 = 51;
	}

	for (i = 0; i< 10; i++)
		PSD_bitmap[i] = 0;

	 // Add By Gary
       for (i=0; i<80; i++)
	   	pRX_HP_Table->PSD_bitmap_RXHP[i] = 0;
	// End



	if(IS_HARDWARE_TYPE_8723AE(Adapter))
	{
		TH1 =TH1-SIR_STEP_SIZE;
	}
	while (good_cnt < PSD_CHMIN)
	{
		good_cnt = 0;
		if(IS_HARDWARE_TYPE_8723AE(Adapter))
		{
			if(TH1 ==TH2)
				break;
			if((TH1+SIR_STEP_SIZE) < TH2)
				TH1 += SIR_STEP_SIZE;
			else
				TH1 = TH2;
		}
		else   // C-series D-series for RX high Power
		{
			if(TH1==(RSSI_BT+0x1E))
             		     break;    
   			if((TH1+2) < (RSSI_BT+0x1E))
				TH1+=3;
		     	else
				TH1 = RSSI_BT+0x1E;	
		
		}
		RT_TRACE(COMP_PSD,DBG_LOUD,("PSD: decision threshold is: %d", TH1));
			 
		for (i = 0; i< 80; i++)
		{
			if(PSD_report[i] < TH1)
			{
				byte_idx = i / 8;
				bit_idx = i -8*byte_idx;
				bitmap = PSD_bitmap[byte_idx];
				PSD_bitmap[byte_idx] = bitmap | (u1Byte) (1 << bit_idx);
			}
		}

#if DBG
	RT_TRACE(	COMP_PSD, DBG_LOUD,("PSD: before smoothing\n"));
	for(n=0;n<10;n++)
	{
		//DbgPrint("PSD_bitmap[%u]=%x\n", n, PSD_bitmap[n]);
		for (i = 0; i<8; i++)
			RT_TRACE(	COMP_PSD, DBG_LOUD,("PSD_bitmap[%u] =   %d\n", 2402+n*8+i, (PSD_bitmap[n]&BIT(i))>>i));
	}
#endif
	
//1 Start of smoothing function

	for (j=0;j<3;j++)
	{
		start_byte_idx=0;
		start_bit_idx=0;
		for(n=0; n<Smooth_Interval[j]; n++)
		{
			good_cnt_smoothing = 0;
			cur_bit_idx = start_bit_idx;
			cur_byte_idx = start_byte_idx;
			for ( i=0; i < Smooth_size[j]; i++)
			{
				NOW_byte_idx = cur_byte_idx + (i+cur_bit_idx)/8;
				if ( (PSD_bitmap[NOW_byte_idx]& BIT( (cur_bit_idx + i)%8)) != 0)
					good_cnt_smoothing++;

			}

			if( good_cnt_smoothing < Smooth_TH[j] )
			{
				cur_bit_idx = start_bit_idx;
				cur_byte_idx = start_byte_idx;
			for ( i=0; i< Smooth_size[j] ; i++)
				{	
				NOW_byte_idx = cur_byte_idx + (i+cur_bit_idx)/8;				
				PSD_bitmap[NOW_byte_idx] = PSD_bitmap[NOW_byte_idx] & (~BIT( (cur_bit_idx + i)%8));
				}
			}
			start_bit_idx =  start_bit_idx + Smooth_Step_Size;
			while ( (start_bit_idx)  > 7 )
			{
				start_byte_idx= start_byte_idx+start_bit_idx/8;
				start_bit_idx = start_bit_idx%8;
			}
		}

		RT_TRACE(	COMP_PSD, DBG_LOUD,("PSD: after %u smoothing", j+1));
		for(n=0;n<10;n++)
		{
			for (i = 0; i<8; i++)
			{
				RT_TRACE(	COMP_PSD, DBG_LOUD,("PSD_bitmap[%u] =   %d\n", 2402+n*8+i, (PSD_bitmap[n]&BIT(i))>>i));
				if ( ((PSD_bitmap[n]&BIT(i))>>i) ==1)  //----- Add By Gary
				{
                                   pRX_HP_Table->PSD_bitmap_RXHP[8*n+i] = 1;
				}                                                  // ------end by Gary
			}
		}

	}

	good_cnt = 0;
	for ( i = 0; i < 10; i++)
	{
		for (n = 0; n < 8; n++)
			if((PSD_bitmap[i]& BIT(n)) != 0)
				good_cnt++;
	}
	RT_TRACE(COMP_PSD, DBG_LOUD,("PSD: good channel cnt = %u",good_cnt));
	}

	//RT_TRACE(COMP_PSD, DBG_LOUD,("PSD: SSBT=%d, TH2=%d, TH1=%d",SSBT,TH2,TH1));
	for (i = 0; i <10; i++)
		RT_TRACE(COMP_PSD, DBG_LOUD,("PSD: PSD_bitmap[%u]=%x",i,PSD_bitmap[i]));
/*	
	//Update bitmap memory
	for(i = 0; i < 80; i++)
	{
		byte_idx = i / 8;
		bit_idx = i -8*byte_idx;
		psd_bit = (PSD_bitmap[byte_idx] & BIT(bit_idx)) >> bit_idx;
		bitmap = PSD_bitmap_memory[i]; 
		PSD_bitmap_memory[i] = (bitmap << 1) |psd_bit;
	}
*/
}


VOID
odm_PSD_Monitor_8192C(
	PADAPTER	Adapter
)
{
	unsigned int 		pts, start_point, stop_point, initial_gain ;
	static u1Byte		PSD_bitmap_memory[80], init_memory = 0;
	static u1Byte 		psd_cnt=0;
	static u4Byte		PSD_report[80], PSD_report_tmp;
	static u8Byte		lastTxOkCnt=0, lastRxOkCnt=0;
	u1Byte 			H2C_PSD_DATA[5]={0,0,0,0,0};
	static u1Byte		H2C_PSD_DATA_last[5] ={0,0,0,0,0};
	u1Byte			idx[20]={96,99,102,106,109,112,115,118,122,125,
					0,3,6,10,13,16,19,22,26,29};
	u1Byte			n, i, channel, BBReset,tone_idx;
	u1Byte			PSD_bitmap[10], SSBT,initial_gain_psd, RSSI_BT, initialGainUpper;
	s4Byte    			PSD_skip_start, PSD_skip_stop;
	u4Byte			CurrentChannel, RXIQI, RxIdleLowPwr, wlan_channel;
	u4Byte			ReScan, Interval, Is40MHz;
	u8Byte			curTxOkCnt, curRxOkCnt;
	int 				cur_byte_idx, cur_bit_idx;
	PMGNT_INFO		pMgntInfo = &(Adapter->MgntInfo);
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	//--------------2G band synthesizer for 92D switch RF channel using----------------- 
	u1Byte			group_idx=0;
	u4Byte			SYN_RF25=0, SYN_RF26=0, SYN_RF27=0, SYN_RF2B=0, SYN_RF2C=0;
	u4Byte			SYN[5] = {0x25, 0x26, 0x27, 0x2B, 0x2C};    // synthesizer RF register for 2G channel
	u4Byte			SYN_group[3][5] = {{0x643BC, 0xFC038, 0x77C1A, 0x41289, 0x01840},     // For CH1,2,4,9,10.11.12   {0x643BC, 0xFC038, 0x77C1A, 0x41289, 0x01840}
									    {0x643BC, 0xFC038, 0x07C1A, 0x41289, 0x01840},     // For CH3,13,14
									    {0x243BC, 0xFC438, 0x07C1A, 0x4128B, 0x0FC41}};   // For Ch5,6,7,8
       //--------------------- Add by Gary for Debug setting ----------------------
       s4Byte 			psd_result = 0;
	u1Byte             	RSSI_BT_new = (u1Byte) PHY_QueryBBReg(Adapter, 0xB9C, 0xFF);
       u1Byte             	rssi_ctrl = (u1Byte) PHY_QueryBBReg(Adapter, 0xB38, 0xFF);
       //---------------------------------------------------------------------
	
	if(pMgntInfo->bScanInProgress)
	{
		if(IS_HARDWARE_TYPE_8723AE(Adapter))
		{	//pHalData->bPSDactive=FALSE;
			PlatformSetTimer( Adapter, &pHalData->PSDTimer, 900); //ms	
			//psd_cnt=0;
		}
		return;
	}

	ReScan = PSD_RESCAN;
	Interval = SCAN_INTERVAL;


	//1 Initialization
	if(init_memory == 0)
	{
		RT_TRACE(	COMP_PSD, DBG_LOUD,("Init memory\n"));
		for(i = 0; i < 80; i++)
			PSD_bitmap_memory[i] = 0xFF; // channel is always good
		init_memory = 1;
	}
	if(psd_cnt == 0)
	{
		RT_TRACE(COMP_PSD, DBG_LOUD,("Enter dm_PSD_Monitor\n"));
		for(i = 0; i < 80; i++)
			PSD_report[i] = 0;
	}
#if 0	//for test only
	DbgPrint("cosa odm_PSD_Monitor call()\n");
	DbgPrint("cosa pHalData->RSSI_BT = %d\n", pHalData->RSSI_BT);
	DbgPrint("cosa pHalData->bUserAssignLevel = %d\n", pHalData->bUserAssignLevel);
#if 0
	psd_cnt++;
	if (psd_cnt < ReScan)
		PlatformSetTimer( Adapter, &pHalData->PSDTimer, Interval); //ms
	else
		psd_cnt = 0;
	return;
#endif
#endif
	//1 Backup Current Settings
	CurrentChannel = PHY_QueryRFReg(Adapter, RF_PATH_A, 0x18, bRFRegOffsetMask);
	if(IS_HARDWARE_TYPE_8192D(Adapter))
	{
		//2 Record Current synthesizer parameters based on current channel
		if((pHalData->MacPhyMode92D == SINGLEMAC_SINGLEPHY)||(pHalData->MacPhyMode92D == DUALMAC_SINGLEPHY))
		{
			SYN_RF25 = PHY_QueryRFReg(Adapter, RF_PATH_B, 0x25, bMaskDWord);
			SYN_RF26 = PHY_QueryRFReg(Adapter, RF_PATH_B, 0x26, bMaskDWord);
			SYN_RF27 = PHY_QueryRFReg(Adapter, RF_PATH_B, 0x27, bMaskDWord);
			SYN_RF2B = PHY_QueryRFReg(Adapter, RF_PATH_B, 0x2B, bMaskDWord);
			SYN_RF2C = PHY_QueryRFReg(Adapter, RF_PATH_B, 0x2C, bMaskDWord);
       	}
		else     // DualMAC_DualPHY 2G
		{
			SYN_RF25 = PHY_QueryRFReg(Adapter, RF_PATH_A, 0x25, bMaskDWord);
			SYN_RF26 = PHY_QueryRFReg(Adapter, RF_PATH_A, 0x26, bMaskDWord);
			SYN_RF27 = PHY_QueryRFReg(Adapter, RF_PATH_A, 0x27, bMaskDWord);
			SYN_RF2B = PHY_QueryRFReg(Adapter, RF_PATH_A, 0x2B, bMaskDWord);
			SYN_RF2C = PHY_QueryRFReg(Adapter, RF_PATH_A, 0x2C, bMaskDWord);
		}
	}
	RXIQI = PHY_QueryBBReg(Adapter, 0xC14, bMaskDWord);
	RxIdleLowPwr = (PHY_QueryBBReg(Adapter, 0x818, bMaskDWord)&BIT28)>>28;
	Is40MHz = pMgntInfo->pHTInfo->bCurBW40MHz;
	RT_TRACE(	COMP_PSD, DBG_LOUD,("PSD Scan Start\n"));
	//1 Turn off CCK
	PHY_SetBBReg(Adapter, rFPGA0_RFMOD, BIT24, 0);
	//1 Turn off TX
	//Pause TX Queue
	PlatformEFIOWrite1Byte(Adapter, REG_TXPAUSE, 0xFF);
	//Force RX to stop TX immediately
	PHY_SetRFReg(Adapter, RF_PATH_A, RF_AC, bRFRegOffsetMask, 0x32E13);
	//1 Turn off RX
	//Rx AGC off  RegC70[0]=0, RegC7C[20]=0
	PHY_SetBBReg(Adapter, 0xC70, BIT0, 0);
	PHY_SetBBReg(Adapter, 0xC7C, BIT20, 0);
	//Turn off CCA
	PHY_SetBBReg(Adapter, 0xC14, bMaskDWord, 0x0);
	//BB Reset
	BBReset = PlatformEFIORead1Byte(Adapter, 0x02);
	PlatformEFIOWrite1Byte(Adapter, 0x02, BBReset&(~BIT0));
	PlatformEFIOWrite1Byte(Adapter, 0x02, BBReset|BIT0);
	//1 Leave RX idle low power
	PHY_SetBBReg(Adapter, 0x818, BIT28, 0x0);
	//1 Fix initial gain
	if (IS_HARDWARE_TYPE_8723AE(Adapter))
	    RSSI_BT = pHalData->RSSI_BT;  
       else if((IS_HARDWARE_TYPE_8192C(Adapter))||(IS_HARDWARE_TYPE_8192D(Adapter)))      // Add by Gary
           RSSI_BT = RSSI_BT_new;
	RT_TRACE(COMP_PSD, DBG_LOUD,("PSD: RSSI_BT= %d\n", RSSI_BT));
	
	if(IS_HARDWARE_TYPE_8723A(Adapter))
	{
		//Neil add--2011--10--12
		//2 Initial Gain index 
		if(RSSI_BT >=35)   // >= -15dBm
			initial_gain_psd = RSSI_BT*2;
		else if((RSSI_BT >=33)&(RSSI_BT<35))
			initial_gain_psd = RSSI_BT*2+6;
		else if((RSSI_BT >=24)&(RSSI_BT<33))
			initial_gain_psd = 70-(31-RSSI_BT);
 		else if((RSSI_BT >=19)&(RSSI_BT<24))
			initial_gain_psd = 64-((24-RSSI_BT)*4);
		else if((RSSI_BT >=14)&(RSSI_BT<19))
			initial_gain_psd = 44-((18-RSSI_BT)*2);
		else if((RSSI_BT >=8)&(RSSI_BT<14))
			initial_gain_psd = 35-(14-RSSI_BT);
		else
			initial_gain_psd = 0x1B;
	}
	else
	{
		if(rssi_ctrl == 1)        // just for debug!!
	      		initial_gain_psd = RSSI_BT_new ; 
           	else
	         	initial_gain_psd = (u1Byte)pHalData->UndecoratedSmoothedPWDB;    // PSD report based on RSSI
	}
	//if(RSSI_BT<0x17)
	//	RSSI_BT +=3;
	//DbgPrint("PSD: RSSI_BT= %d\n", RSSI_BT);
	RT_TRACE(COMP_PSD, DBG_LOUD,("PSD: RSSI_BT= %d\n", RSSI_BT));
	
	if(pHalData->bUserAssignLevel)
	{
		pHalData->bUserAssignLevel = FALSE;
		initialGainUpper = 0x7f;
	}
	else
	{
		initialGainUpper = 0x54;
	}
	/*
	if (initial_gain_psd < 0x1a)
		initial_gain_psd = 0x1a;
	if (initial_gain_psd > initialGainUpper)
		initial_gain_psd = initialGainUpper;
	*/
	if(IS_HARDWARE_TYPE_8723AE(Adapter))
		SSBT = RSSI_BT  * 2 +0x3E;
	else if((IS_HARDWARE_TYPE_8192C(Adapter))||(IS_HARDWARE_TYPE_8192D(Adapter)))   // Add by Gary
	{
		RSSI_BT = initial_gain_psd;
		SSBT = RSSI_BT;
	}
	RT_TRACE(	COMP_PSD, DBG_LOUD,("PSD: SSBT= %d\n", SSBT));
	RT_TRACE(	COMP_PSD, DBG_LOUD,("PSD: initial gain= 0x%x\n", initial_gain_psd));
	//DbgPrint("PSD: SSBT= %d", SSBT);
	pMgntInfo->bDMInitialGainEnable = FALSE;
	initial_gain = PHY_QueryBBReg(Adapter, 0xc50, bMaskDWord) & 0x7F;
	PHY_SetBBReg(Adapter, 0xc50, 0x7F, initial_gain_psd);	
	//1 Turn off 3-wire
	PHY_SetBBReg(Adapter, 0x88c, BIT20|BIT21|BIT22|BIT23, 0xF);

	//pts value = 128, 256, 512, 1024
	pts = 128;

	if(pts == 128)
	{
		PHY_SetBBReg(Adapter, 0x808, BIT14|BIT15, 0x0);
		start_point = 64;
		stop_point = 192;
	}
	else if(pts == 256)
	{
		PHY_SetBBReg(Adapter, 0x808, BIT14|BIT15, 0x1);
		start_point = 128;
		stop_point = 384;
	}
	else if(pts == 512)
	{
		PHY_SetBBReg(Adapter, 0x808, BIT14|BIT15, 0x2);
		start_point = 256;
		stop_point = 768;
	}
	else
	{
		PHY_SetBBReg(Adapter, 0x808, BIT14|BIT15, 0x3);
		start_point = 512;
		stop_point = 1536;
	}
	

//3 Skip WLAN channels if WLAN busy
	curTxOkCnt = Adapter->TxStats.NumTxBytesUnicast - lastTxOkCnt;
	curRxOkCnt = Adapter->RxStats.NumRxBytesUnicast - lastRxOkCnt;
	lastTxOkCnt = Adapter->TxStats.NumTxBytesUnicast;
	lastRxOkCnt = Adapter->RxStats.NumRxBytesUnicast;	

	PSD_skip_start=80;
	PSD_skip_stop = 0;
	wlan_channel = CurrentChannel & 0x0f;

	RT_TRACE(COMP_PSD,DBG_LOUD,("PSD: current channel: %x, BW:%d \n", wlan_channel, Is40MHz));
	if(IS_HARDWARE_TYPE_8723AE(Adapter))
	{
		if((curRxOkCnt+curTxOkCnt) > 10)
		{
			if(Is40MHz)
			{
				PSD_skip_start = ((wlan_channel-1)*5 -Is40MHz*10)-2;  // Modify by Neil to add 10 chs to mask
				PSD_skip_stop = (PSD_skip_start + (1+Is40MHz)*20)+4;
			}
			else
			{
				PSD_skip_start = ((wlan_channel-1)*5 -Is40MHz*10)-10;  // Modify by Neil to add 10 chs to mask
				PSD_skip_stop = (PSD_skip_start + (1+Is40MHz)*20)+18; 
			}
		
			if(PSD_skip_start < 0)
				PSD_skip_start = 0;
			if(PSD_skip_stop >80)
				PSD_skip_stop = 80;
		}
	}
	else   // for C/D series RX High Power
	{
		if((curRxOkCnt+curTxOkCnt) > 1000)
		{
			PSD_skip_start = (wlan_channel-1)*5 -Is40MHz*10;
			PSD_skip_stop = PSD_skip_start + (1+Is40MHz)*20;
		}
	}
	RT_TRACE(COMP_PSD,DBG_LOUD,("PSD: Skip tone from %d to %d \n", PSD_skip_start, PSD_skip_stop));

 	for (n=0;n<80;n++)
 	{
 		if((n%20)==0)
 		{
			channel = (n/20)*4 + 1;
			if(IS_HARDWARE_TYPE_8192D(Adapter))
			{
				switch(channel)
				{
					case 1: 
					case 9:
						group_idx = 0;
						break;
					case 5:
						group_idx = 2;
						break;
					case 13:
				 		group_idx = 1;
						break;
				}
				
				if((pHalData->MacPhyMode92D == SINGLEMAC_SINGLEPHY)||(pHalData->MacPhyMode92D == DUALMAC_SINGLEPHY))   
				{
					for(i = 0; i < SYN_Length; i++)
						PHY_SetRFReg(Adapter, RF_PATH_B, SYN[i], bMaskDWord, SYN_group[group_idx][i]);

					PHY_SetRFReg(Adapter, RF_PATH_A, RF_CHNLBW, 0x3FF, channel);
					PHY_SetRFReg(Adapter, RF_PATH_B, RF_CHNLBW, 0x3FF, channel);
				}
				else  // DualMAC_DualPHY 2G
				{
					for(i = 0; i < SYN_Length; i++)
						PHY_SetRFReg(Adapter, RF_PATH_A, SYN[i], bMaskDWord, SYN_group[group_idx][i]);   
					
					PHY_SetRFReg(Adapter, RF_PATH_A, RF_CHNLBW, 0x3FF, channel);
				}
			}
			else
				PHY_SetRFReg(Adapter, RF_PATH_A, 0x18, 0x3FF, channel);
		}
		tone_idx = n%20;
		if ((n>=PSD_skip_start) && (n<PSD_skip_stop))
		{	
			PSD_report[n] = SSBT;
			RT_TRACE(COMP_PSD,DBG_LOUD,("PSD:Tone %d skipped", n));
		}
		else
		{
			PSD_report_tmp =  GetPSDData_8192C(Adapter, idx[tone_idx], initial_gain_psd);

			if ( PSD_report_tmp > PSD_report[n])
				PSD_report[n] = PSD_report_tmp;
				
		}
	}

	PatchDCTone_8192C(Adapter, PSD_report, initial_gain_psd);
      /*
       //Add by Neil Chen to report PSD dB result---2011--07--06
       for (n=0;n<80;n++)
 	{
 	    RT_TRACE(COMP_PSD,DBG_LOUD,("PSD Tone:%d, PSD SCAN Result:%d \n",n, PSD_report[n]));
       }
       */
       //RT_TRACE( COMP_PSD, DBG_LOUD,("psd_report[%d]=     %d", 2402+i, PSD_report[i]));

       //----end
	//1 Turn on RX
	//Rx AGC on
	PHY_SetBBReg(Adapter, 0xC70, BIT0, 1);
	PHY_SetBBReg(Adapter, 0xC7C, BIT20, 1);
	//CCK on
	PHY_SetBBReg(Adapter, rFPGA0_RFMOD, BIT24, 1);
	//1 Turn on TX
	//Resume TX Queue
	PlatformEFIOWrite1Byte(Adapter, REG_TXPAUSE, 0x00);
	//Turn on 3-wire
	PHY_SetBBReg(Adapter, 0x88c, BIT20|BIT21|BIT22|BIT23, 0x0);
	//1 Restore Current Settings
	//Resume DIG
	pMgntInfo->bDMInitialGainEnable = TRUE;
	PHY_SetBBReg(Adapter, 0xc50, 0x7F, initial_gain);
	// restore originl center frequency
	PHY_SetRFReg(Adapter, RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask, CurrentChannel);
	if(IS_HARDWARE_TYPE_8192D(Adapter))
	{
		if((pHalData->MacPhyMode92D == SINGLEMAC_SINGLEPHY)||(pHalData->MacPhyMode92D == DUALMAC_SINGLEPHY))
		{
			PHY_SetRFReg(Adapter, RF_PATH_B, RF_CHNLBW, bMaskDWord, CurrentChannel);
			PHY_SetRFReg(Adapter, RF_PATH_B, 0x25, bMaskDWord, SYN_RF25);
			PHY_SetRFReg(Adapter, RF_PATH_B, 0x26, bMaskDWord, SYN_RF26);
			PHY_SetRFReg(Adapter, RF_PATH_B, 0x27, bMaskDWord, SYN_RF27);
			PHY_SetRFReg(Adapter, RF_PATH_B, 0x2B, bMaskDWord, SYN_RF2B);
			PHY_SetRFReg(Adapter, RF_PATH_B, 0x2C, bMaskDWord, SYN_RF2C);
		}
		else     // DualMAC_DualPHY
		{
			PHY_SetRFReg(Adapter, RF_PATH_A, 0x25, bMaskDWord, SYN_RF25);
			PHY_SetRFReg(Adapter, RF_PATH_A, 0x26, bMaskDWord, SYN_RF26);
			PHY_SetRFReg(Adapter, RF_PATH_A, 0x27, bMaskDWord, SYN_RF27);
			PHY_SetRFReg(Adapter, RF_PATH_A, 0x2B, bMaskDWord, SYN_RF2B);
			PHY_SetRFReg(Adapter, RF_PATH_A, 0x2C, bMaskDWord, SYN_RF2C);
		}
	}
	//Turn on CCA
	PHY_SetBBReg(Adapter, 0xC14, bMaskDWord, RXIQI);
	//Restore RX idle low power
	if(RxIdleLowPwr == TRUE)
		PHY_SetBBReg(Adapter, 0x818, BIT28, 1);
	
	psd_cnt++;
	RT_TRACE(COMP_PSD, DBG_LOUD,("PSD:psd_cnt = %d",psd_cnt));
	if (psd_cnt < ReScan)
		PlatformSetTimer( Adapter, &pHalData->PSDTimer, Interval); //ms
	else
	{
		psd_cnt = 0;
		for(i=0;i<80;i++)
			//DbgPrint("psd_report[%d]=     %d \n", 2402+i, PSD_report[i]);
			RT_TRACE(	COMP_PSD, DBG_LOUD,("psd_report[%d]=     %d \n", 2402+i, PSD_report[i]));

		GoodChannelDecision_8192C(Adapter, PSD_report, PSD_bitmap, RSSI_BT, PSD_bitmap_memory);

		if(IS_HARDWARE_TYPE_8723AE(Adapter))
		{
			cur_byte_idx=0;
			cur_bit_idx=0;

			//2 Restore H2C PSD Data to Last Data
  			H2C_PSD_DATA_last[0] = H2C_PSD_DATA[0];
			H2C_PSD_DATA_last[1] = H2C_PSD_DATA[1];
			H2C_PSD_DATA_last[2] = H2C_PSD_DATA[2];
			H2C_PSD_DATA_last[3] = H2C_PSD_DATA[3];
			H2C_PSD_DATA_last[4] = H2C_PSD_DATA[4];

	
			//2 Translate 80bit channel map to 40bit channel	
			for ( i=0;i<5;i++)
			{
				for(n=0;n<8;n++)
				{
					cur_byte_idx = i*2 + n/4;
					cur_bit_idx = (n%4)*2;
					if ( ((PSD_bitmap[cur_byte_idx]& BIT(cur_bit_idx)) != 0) && ((PSD_bitmap[cur_byte_idx]& BIT(cur_bit_idx+1)) != 0))
						H2C_PSD_DATA[i] = H2C_PSD_DATA[i] | (u1Byte) (1 << n);
				}
				RT_TRACE(	COMP_PSD, DBG_LOUD,("H2C_PSD_DATA[%d]=0x%x\n" ,i, H2C_PSD_DATA[i]));
			}
	
		//3 	To Compare the difference
			for ( i=0;i<5;i++)
			{
				if(H2C_PSD_DATA[i] !=H2C_PSD_DATA_last[i])
				{
					FillH2CCmd92C(Adapter, H2C_92C_PSD_RESULT, 5, H2C_PSD_DATA);
					RT_TRACE(	COMP_PSD, DBG_LOUD,("Need to Update the AFH Map \n"));
					break;
				}
				else
				{
					if(i==5)
						RT_TRACE(	COMP_PSD, DBG_LOUD,("Not need to Update\n"));	
				}
			}
			//pHalData->bPSDactive=FALSE;
			PlatformSetTimer( Adapter, &pHalData->PSDTimer, 900); //ms	
			RT_TRACE(	COMP_PSD, DBG_LOUD,("Leave dm_PSD_Monitor\n"));	
		}
	}
}	

VOID
odm_PSDMonitor_8192C(
	IN	PADAPTER	Adapter
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	
	if(IS_HARDWARE_TYPE_8723AE(Adapter))
	{
		if(pHalData->bBTTurnOff == TRUE)
		{
			pHalData->bPSDactive=FALSE;
			return; 
		}	
		RT_TRACE(COMP_PSD, DBG_LOUD, ("odm_PSDMonitor\n"));
		pHalData->bPSDinProcess = TRUE;
		pHalData->bPSDactive=TRUE;
		odm_PSD_Monitor(Adapter);
		pHalData->bPSDinProcess = FALSE;
	}
}

//2 8723A ANT DETECT
//
// Description:
//	Implement IQK single tone for RF DPK loopback and BB PSD scanning. 
//	This function is cooperated with BB team Neil. 
//
// Added by Roger, 2011.12.15
//
BOOLEAN
ODM_SingleDualAntennaDetection(
	IN	PADAPTER	Adapter
	)
{

	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	pSWAT_T		pDM_SWAT_Table = &Adapter->DM_SWAT_Table;
	u4Byte		CurrentChannel;
	u1Byte		n, i;
	u4Byte		Reg88c, Regc08, Reg874, Regc50;
	u1Byte		initial_gain = 0x5a;
	u4Byte		PSD_report_tmp;
	u4Byte		AntA_report = 0x0, AntB_report = 0x0;
	BOOLEAN		bResult = TRUE;
	u4Byte		AFE_Backup[16];
	u4Byte		AFE_REG_8723A[16] = {
					rRx_Wait_CCA, 	rTx_CCK_RFON, 
					rTx_CCK_BBON, 	rTx_OFDM_RFON,
					rTx_OFDM_BBON, 	rTx_To_Rx,
					rTx_To_Tx, 		rRx_CCK, 
					rRx_OFDM, 		rRx_Wait_RIFS, 
					rRx_TO_Rx,		rStandby,
					rSleep,			rPMPD_ANAEN, 	
					rFPGA0_XCD_SwitchControl, rBlue_Tooth};


	if(!IS_HARDWARE_TYPE_8723A(Adapter) ||(pHalData->AntDivCfg==0))
		return bResult;
	
	
	//1 Backup Current RF/BB Settings	
	
	CurrentChannel = PHY_QueryRFReg(Adapter, RF_PATH_A, 0x18, bRFRegOffsetMask);

	PHY_SetBBReg(Adapter, rFPGA0_XA_RFInterfaceOE, 0x300, Antenna_A);  // change to Antenna A
	// Step 1: USE IQK to transmitter single tone 

	PlatformStallExecution(10);
	
	//Store A Path Register 88c, c08, 874, c50
	Reg88c = PHY_QueryBBReg(Adapter, rFPGA0_AnalogParameter4, bMaskDWord);
	Regc08 = PHY_QueryBBReg(Adapter, rOFDM0_TRMuxPar, bMaskDWord);
	Reg874 = PHY_QueryBBReg(Adapter, rFPGA0_XCD_RFInterfaceSW, bMaskDWord);
	Regc50 = PHY_QueryBBReg(Adapter, rOFDM0_XAAGCCore1, bMaskDWord);	
	
	// Store AFE Registers
	_PHY_SaveAFERegisters(Adapter, AFE_REG_8723A, AFE_Backup, 16);	
	
	PHY_SetBBReg(Adapter, rFPGA0_PSDFunction, BIT14|BIT15, 0x0);  //128 pts
	
	// To SET CH1 to do
	PHY_SetRFReg(Adapter, RF_PATH_A, 0x18, bRFRegOffsetMask, 0x01);     //Channel 1
	
	// AFE all on step
	PHY_SetBBReg(Adapter, rRx_Wait_CCA, bMaskDWord, 0x6FDB25A4);
	PHY_SetBBReg(Adapter, rTx_CCK_RFON, bMaskDWord, 0x6FDB25A4);
	PHY_SetBBReg(Adapter, rTx_CCK_BBON, bMaskDWord, 0x6FDB25A4);
	PHY_SetBBReg(Adapter, rTx_OFDM_RFON, bMaskDWord, 0x6FDB25A4);
	PHY_SetBBReg(Adapter, rTx_OFDM_BBON, bMaskDWord, 0x6FDB25A4);
	PHY_SetBBReg(Adapter, rTx_To_Rx, bMaskDWord, 0x6FDB25A4);
	PHY_SetBBReg(Adapter, rTx_To_Tx, bMaskDWord, 0x6FDB25A4);
	PHY_SetBBReg(Adapter, rRx_CCK, bMaskDWord, 0x6FDB25A4);
	PHY_SetBBReg(Adapter, rRx_OFDM, bMaskDWord, 0x6FDB25A4);
	PHY_SetBBReg(Adapter, rRx_Wait_RIFS, bMaskDWord, 0x6FDB25A4);
	PHY_SetBBReg(Adapter, rRx_TO_Rx, bMaskDWord, 0x6FDB25A4);
	PHY_SetBBReg(Adapter, rStandby, bMaskDWord, 0x6FDB25A4);
	PHY_SetBBReg(Adapter, rSleep, bMaskDWord, 0x6FDB25A4);
	PHY_SetBBReg(Adapter, rPMPD_ANAEN, bMaskDWord, 0x6FDB25A4);
	PHY_SetBBReg(Adapter, rFPGA0_XCD_SwitchControl, bMaskDWord, 0x6FDB25A4);
	PHY_SetBBReg(Adapter, rBlue_Tooth, bMaskDWord, 0x6FDB25A4);

	// 3 wire Disable
	PHY_SetBBReg(Adapter, rFPGA0_AnalogParameter4, bMaskDWord, 0xCCF000C0);
	
	//BB IQK Setting
	PHY_SetBBReg(Adapter, rOFDM0_TRMuxPar, bMaskDWord, 0x000800E4);
	PHY_SetBBReg(Adapter, rFPGA0_XCD_RFInterfaceSW, bMaskDWord, 0x22208000);

	//IQK setting tone@ 4.34Mhz
	PHY_SetBBReg(Adapter, rTx_IQK_Tone_A, bMaskDWord, 0x10008C1C);
	PHY_SetBBReg(Adapter, rTx_IQK, bMaskDWord, 0x01007c00);	


	//Page B init
	PHY_SetBBReg(Adapter, rConfig_AntA, bMaskDWord, 0x00080000);
	PHY_SetBBReg(Adapter, rConfig_AntA, bMaskDWord, 0x0f600000);
	PHY_SetBBReg(Adapter, rRx_IQK, bMaskDWord, 0x01004800);
	PHY_SetBBReg(Adapter, rRx_IQK_Tone_A, bMaskDWord, 0x10008c1f);
	PHY_SetBBReg(Adapter, rTx_IQK_PI_A, bMaskDWord, 0x82150008);
	PHY_SetBBReg(Adapter, rRx_IQK_PI_A, bMaskDWord, 0x28150008);
	PHY_SetBBReg(Adapter, rIQK_AGC_Rsp, bMaskDWord, 0x001028d0);	

	//RF loop Setting
	PHY_SetRFReg(Adapter, RF_PATH_A, 0x0, 0xFFFFF, 0x50008);	
	
	//IQK Single tone start
	PHY_SetBBReg(Adapter, rFPGA0_IQK, bMaskDWord, 0x80800000);
	PHY_SetBBReg(Adapter, rIQK_AGC_Pts, bMaskDWord, 0xf8000000);
	PlatformStallExecution(1000);

	for (n=0;n<10;n++)
 	{
 		PSD_report_tmp =  GetPSDData_8192C(Adapter, 14, initial_gain);	
		if(PSD_report_tmp >AntA_report)
			AntA_report=PSD_report_tmp;
	}

	PSD_report_tmp=0x0;

	PHY_SetBBReg(Adapter, rFPGA0_XA_RFInterfaceOE, 0x300, Antenna_B);  // change to Antenna B
	PlatformStallExecution(10);	

	
	for (n=0 ; n<10 ; n++)
 	{
 		PSD_report_tmp =  GetPSDData_8192C(Adapter, 14, initial_gain);	
		if(PSD_report_tmp > AntB_report)
			AntB_report=PSD_report_tmp;
	}

	//Close IQK Single Tone function
	PHY_SetBBReg(Adapter, rFPGA0_IQK, bMaskDWord, 0x00000000);
	PSD_report_tmp = 0x0;

	//1 Return to antanna A
	PHY_SetBBReg(Adapter, rFPGA0_XA_RFInterfaceOE, 0x300, Antenna_A);	
	PHY_SetBBReg(Adapter, rFPGA0_AnalogParameter4, bMaskDWord, Reg88c);
	PHY_SetBBReg(Adapter, rOFDM0_TRMuxPar, bMaskDWord, Regc08);
	PHY_SetBBReg(Adapter, rFPGA0_XCD_RFInterfaceSW, bMaskDWord, Reg874);
	PHY_SetBBReg(Adapter, rOFDM0_XAAGCCore1, 0x7F, 0x40);
	PHY_SetBBReg(Adapter, rOFDM0_XAAGCCore1, bMaskDWord, Regc50);
	PHY_SetRFReg(Adapter, RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask,CurrentChannel);

	//Reload AFE Registers
	_PHY_ReloadAFERegisters(Adapter, AFE_REG_8723A, AFE_Backup, 16);	

	RT_TRACE(COMP_ANTENNA, DBG_LOUD, ("psd_report_A[%d]= %d \n", 2416, AntA_report));	
	RT_TRACE(COMP_ANTENNA, DBG_LOUD, ("psd_report_B[%d]= %d \n", 2416, AntB_report));	

	if(AntA_report >=	100)
	{
		if(AntB_report > (AntA_report+1))
		{
			pDM_SWAT_Table->ANTB_ON=FALSE;
			RT_TRACE(COMP_ANTENNA, DBG_LOUD, ("ODM_SingleDualAntennaDetection(): Single Antenna A\n"));		
		}	
		else
		{
			pDM_SWAT_Table->ANTB_ON=TRUE;
			RT_TRACE(COMP_ANTENNA, DBG_LOUD, ("ODM_SingleDualAntennaDetection(): Dual Antenna is A and B\n"));	
		}	
	}
	else
	{
		RT_TRACE(COMP_ANTENNA, DBG_LOUD, ("ODM_SingleDualAntennaDetection(): Need to check again\n"));
		pDM_SWAT_Table->ANTB_ON=FALSE; // Set Antenna B off as default 
		bResult = FALSE;
	}

	return bResult;

}

VOID
_PHY_SaveAFERegisters(
	IN	PADAPTER	pAdapter,
	IN	pu4Byte		AFEReg,
	IN	pu4Byte		AFEBackup,
	IN	u4Byte		RegisterNum
	)
{
	u4Byte	i;
	
	RTPRINT(FINIT, INIT_IQK, ("Save ADDA parameters.\n"));
	for( i = 0 ; i < RegisterNum ; i++){
		AFEBackup[i] = PHY_QueryBBReg(pAdapter, AFEReg[i], bMaskDWord);
	}
}

VOID
_PHY_ReloadAFERegisters(
	IN	PADAPTER	pAdapter,
	IN	pu4Byte		AFEReg,
	IN	pu4Byte		AFEBackup,
	IN	u4Byte		RegiesterNum
	)
{
	u4Byte	i;

	RTPRINT(FINIT, INIT_IQK, ("Reload ADDA power saving parameters !\n"));
	for(i = 0 ; i < RegiesterNum; i++)
	{
	
		PHY_SetBBReg(pAdapter, AFEReg[i], bMaskDWord, AFEBackup[i]);
	}
}

//2 8723A ANT DETECT END




//3 ============================================================
//3 Path Diversity
//3 ============================================================
//2011.09.05  Add by Luke Lee for path diversity


VOID
odm_PathDiversityInit_92C(
	IN	PADAPTER	Adapter
)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	pPD_T	pDM_PDTable = &Adapter->DM_PDTable;
#if RT_PLATFORM == PLATFORM_WINDOWS
	pHalData->PathDivCfg = Adapter->pNdisCommon->bPathDivEnable;
#endif

	if(!IS_92C_SERIAL(pHalData->VersionID))
	{
		RT_TRACE(	COMP_SWAS, DBG_LOUD, ("No ODM_TXPathDiversity()\n"));
		return;
	}
	
	if(pHalData->PathDivCfg == 2)
		odm_2TPathDiversityInit_92C(Adapter);
	else if(pHalData->PathDivCfg == 1)
	{
		odm_1TPathDiversityInit_92C(Adapter);
		//DbgPrint("ODM_1TPathDiversity()\n");
	}
	else
	{
		//DbgPrint(	"No PathDiversity()\n");
	}
	pDM_PDTable->OFDMTXPath = 0;
	pDM_PDTable->CCKTXPath = 0;
	pDM_PDTable->DefaultRespPath = 0;
	pDM_PDTable->TrainingState = 0;
	pDM_PDTable->OFDM_Pkt_Cnt = 0;
	pDM_PDTable->CCK_Pkt_Cnt = 0;
	pDM_PDTable->PathDiv_NoLink_State = 0;
	pDM_PDTable->CCKPathDivEnable = TRUE;
	pHalData->CCK_Pkt_Cnt = 0;
	pHalData->OFDM_Pkt_Cnt = 0;
}


VOID
odm_2TPathDiversityInit_92C(
	IN	PADAPTER	Adapter
)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	if(!IS_92C_SERIAL(pHalData->VersionID))
		return;

	//DbgPrint("ODM_2TPathDiversity()\n");
	//3 CCK Settings
	PHY_SetBBReg(Adapter, rCCK0_AFESetting  , bMaskByte3, 0xc1);		//RegA07=0xc1
	PHY_SetBBReg(Adapter, rCCK0_RxAGC2  , bMaskByte1, 0x9b);			//RegA11=0x9b
	PHY_SetBBReg(Adapter, rCCK0_TxFilter1  , bMaskByte0, 0x10);			//RegA20=0x10
	PHY_SetBBReg(Adapter, rCCK0_FalseAlarmReport  , bMaskByte2, 0xdf);	//RegA2E=0xdf
	PHY_SetBBReg(Adapter, rCCK0_FalseAlarmReport  , bMaskByte3, 0x10);	//RegA2F=0x10
	PHY_SetBBReg(Adapter, 0xa74  , BIT8, 1);					//RegA75=0x01
	//PHY_SetBBReg(Adapter, rFPGA0_TxGainStage, BIT31, 0);				//Reg80C[31]=1'b0
	
	//3 OFDM Settings
	PHY_SetBBReg(Adapter, rOFDM0_XBTxAFE  , bMaskDWord, 0xa0e40000);	//RegC8C=0xa0e40000
	PHY_SetBBReg(Adapter, rFPGA1_TxInfo  , 0x0FFFFFFF, 0x03321333);	//Reg90C=0x83321333
	//PHY_SetBBReg(Adapter, rFPGA1_TxInfo  , BIT30, 1);					//Reg90C[30]=1'b1

	//3 Response TX Settings
	PlatformEFIOWrite1Byte(Adapter, 0x6D8, 0x3F);						//Reg6D8[5:0]=6b'111111

}

VOID
odm_1TPathDiversityInit_92C(
	IN	PADAPTER	Adapter
)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	if(!IS_92C_SERIAL(pHalData->VersionID))
		return;

	//DbgPrint("ODM_2TPathDiversity()\n");
	//3 CCK Settings
	PHY_SetBBReg(Adapter, rCCK0_AFESetting  , bMaskByte3, 0xc1);		//RegA07=0xc1
	PHY_SetBBReg(Adapter, rCCK0_RxAGC2  , bMaskByte1, 0x9b);			//RegA11=0x9b
	PHY_SetBBReg(Adapter, rCCK0_TxFilter1  , bMaskByte0, 0x10);			//RegA20=0x10
	PHY_SetBBReg(Adapter, rCCK0_FalseAlarmReport  , bMaskByte2, 0xdf);	//RegA2E=0xdf
	PHY_SetBBReg(Adapter, rCCK0_FalseAlarmReport  , bMaskByte3, 0x10);	//RegA2F=0x10
	PHY_SetBBReg(Adapter, 0xa74  , BIT8, 1);					//RegA75=0x01
	PHY_SetBBReg(Adapter, rFPGA0_TxGainStage, BIT31, 1);				//Reg80C[31]=1'b1
	
	//3 OFDM Settings
	PHY_SetBBReg(Adapter, rOFDM0_XBTxAFE  , bMaskDWord, 0xa0e40000);	//RegC8C=0xa0e40000
	//PHY_SetBBReg(Adapter, rFPGA1_TxInfo  , 0x0FFFFFFF, 0x03321333);	//Reg90C=0x83321333
	PHY_SetBBReg(Adapter, rFPGA1_TxInfo  , BIT30, 1);					//Reg90C[30]=1'b1

	//3 Response TX Settings
	PlatformEFIOWrite1Byte(Adapter, 0x6D8, 0x15);						//Reg6D8[5:0]=6b'010101
}

BOOLEAN
ODM_PathDiversityBeforeLink92C(
	IN	PADAPTER	Adapter
	)
{
#if (RT_MEM_SIZE_LEVEL != RT_MEM_SIZE_MINIMUM)
	HAL_DATA_TYPE*	pHalData = GET_HAL_DATA(Adapter);
	PMGNT_INFO		pMgntInfo = &Adapter->MgntInfo;
	//pSWAT_T		pDM_SWAT_Table = &Adapter->DM_SWAT_Table;
	pPD_T			pDM_PDTable = &Adapter->DM_PDTable;

	s1Byte			Score = 0;
	PRT_WLAN_BSS	pTmpBssDesc;
	PRT_WLAN_BSS	pTestBssDesc;

	u1Byte			target_chnl = 0;
	u1Byte			index;

	// Condition that does not need to use path diversity.
	if((!IS_92C_SERIAL(pHalData->VersionID)) || (pHalData->PathDivCfg!=1) || pMgntInfo->AntennaTest )
	{
		RT_TRACE(COMP_SWAS, DBG_LOUD, 
				("ODM_PathDiversityBeforeLink92C(): No PathDiv Mechanism before link.\n"));
		return FALSE;
	}

	// Since driver is going to set BB register, it shall check if there is another thread controlling BB/RF.
	PlatformAcquireSpinLock(Adapter, RT_RF_STATE_SPINLOCK);
	if(pHalData->eRFPowerState!=eRfOn || pMgntInfo->RFChangeInProgress || pMgntInfo->bMediaConnect)
	{
		PlatformReleaseSpinLock(Adapter, RT_RF_STATE_SPINLOCK);
	
		RT_TRACE(COMP_SWAS, DBG_LOUD, 
				("ODM_PathDiversityBeforeLink92C(): RFChangeInProgress(%x), eRFPowerState(%x)\n", 
				pMgntInfo->RFChangeInProgress,
				pHalData->eRFPowerState));
	
		//pDM_SWAT_Table->SWAS_NoLink_State = 0;
		pDM_PDTable->PathDiv_NoLink_State = 0;
		
		return FALSE;
	}
	else
	{
		PlatformReleaseSpinLock(Adapter, RT_RF_STATE_SPINLOCK);
	}

	//1 Run AntDiv mechanism "Before Link" part.
	//if(pDM_SWAT_Table->SWAS_NoLink_State == 0)
	if(pDM_PDTable->PathDiv_NoLink_State == 0)
	{
		//1 Prepare to do Scan again to check current antenna state.

		// Set check state to next step.
		//pDM_SWAT_Table->SWAS_NoLink_State = 1;
		pDM_PDTable->PathDiv_NoLink_State = 1;
	
		// Copy Current Scan list.
		Adapter->MgntInfo.tmpNumBssDesc = pMgntInfo->NumBssDesc;
		PlatformMoveMemory((PVOID)Adapter->MgntInfo.tmpbssDesc, (PVOID)pMgntInfo->bssDesc, sizeof(RT_WLAN_BSS)*MAX_BSS_DESC);

		// Switch Antenna to another one.
		if(pDM_PDTable->DefaultRespPath == 0)
		{
			PHY_SetBBReg(Adapter, rCCK0_AFESetting  , 0x0F000000, 0x05); // TRX path = PathB
			odm_SetRespPath_92C(Adapter, 1);
			pDM_PDTable->OFDMTXPath = 0xFFFFFFFF;
			pDM_PDTable->CCKTXPath = 0xFFFFFFFF;
		}
		else
		{
			PHY_SetBBReg(Adapter, rCCK0_AFESetting  , 0x0F000000, 0x00); // TRX path = PathA
			odm_SetRespPath_92C(Adapter, 0);
			pDM_PDTable->OFDMTXPath = 0x0;
			pDM_PDTable->CCKTXPath = 0x0;
		}
#if 0	

		pDM_SWAT_Table->PreAntenna = pDM_SWAT_Table->CurAntenna;
		pDM_SWAT_Table->CurAntenna = (pDM_SWAT_Table->CurAntenna==Antenna_A)?Antenna_B:Antenna_A;
		
		RT_TRACE(COMP_SWAS, DBG_LOUD, 
			("ODM_PathDiversityBeforeLink92C: Change to Ant(%s) for testing.\n", (pDM_SWAT_Table->CurAntenna==Antenna_A)?"A":"B"));
		//PHY_SetBBReg(Adapter, rFPGA0_XA_RFInterfaceOE, 0x300, DM_SWAT_Table.CurAntenna);
		pDM_SWAT_Table->SWAS_NoLink_BK_Reg860 = ((pDM_SWAT_Table->SWAS_NoLink_BK_Reg860 & 0xfffffcff) | (pDM_SWAT_Table->CurAntenna<<8));
		PHY_SetBBReg(Adapter, rFPGA0_XA_RFInterfaceOE, bMaskDWord, pDM_SWAT_Table->SWAS_NoLink_BK_Reg860);
#endif

		// Go back to scan function again.
		RT_TRACE(COMP_SWAS, DBG_LOUD, ("ODM_PathDiversityBeforeLink92C: Scan one more time\n"));
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
				RT_TRACE(COMP_SWAS, DBG_LOUD, ("ODM_PathDiversityBeforeLink92C(): ERROR!! This shall not happen.\n"));
				continue;
			}

			if(pTmpBssDesc->RecvSignalPower > pTestBssDesc->RecvSignalPower)
			{
				RT_TRACE(COMP_SWAS, DBG_LOUD, ("ODM_PathDiversityBeforeLink92C: Compare scan entry: Score++\n"));
				RT_PRINT_STR(COMP_SWAS, DBG_LOUD, "SSID: ", pTestBssDesc->bdSsIdBuf, pTestBssDesc->bdSsIdLen);
				RT_TRACE(COMP_SWAS, DBG_LOUD, ("Original: %d, Test: %d\n", pTmpBssDesc->RecvSignalPower, pTestBssDesc->RecvSignalPower));
			
				Score++;
				PlatformMoveMemory(pTestBssDesc, pTmpBssDesc, sizeof(RT_WLAN_BSS));
			}
			else if(pTmpBssDesc->RecvSignalPower < pTestBssDesc->RecvSignalPower)
			{
				RT_TRACE(COMP_SWAS, DBG_LOUD, ("ODM_PathDiversityBeforeLink92C: Compare scan entry: Score--\n"));
				RT_PRINT_STR(COMP_SWAS, DBG_LOUD, "SSID: ", pTestBssDesc->bdSsIdBuf, pTestBssDesc->bdSsIdLen);
				RT_TRACE(COMP_SWAS, DBG_LOUD, ("Original: %d, Test: %d\n", pTmpBssDesc->RecvSignalPower, pTestBssDesc->RecvSignalPower));
				Score--;
			}

		}

		if(pMgntInfo->NumBssDesc!=0 && Score<=0)
		{
			RT_TRACE(COMP_SWAS, DBG_LOUD,
				("ODM_PathDiversityBeforeLink92C(): DefaultRespPath=%d\n", pDM_PDTable->DefaultRespPath));

			//pDM_SWAT_Table->PreAntenna = pDM_SWAT_Table->CurAntenna;
		}
		else
		{
			RT_TRACE(COMP_SWAS, DBG_LOUD, 
				("ODM_PathDiversityBeforeLink92C(): DefaultRespPath=%d\n", pDM_PDTable->DefaultRespPath));

			if(pDM_PDTable->DefaultRespPath == 0)
			{
				pDM_PDTable->OFDMTXPath = 0xFFFFFFFF;
				pDM_PDTable->CCKTXPath = 0xFFFFFFFF;
				odm_SetRespPath_92C(Adapter, 1);
			}
			else
			{
				pDM_PDTable->OFDMTXPath = 0x0;
				pDM_PDTable->CCKTXPath = 0x0;
				odm_SetRespPath_92C(Adapter, 0);
			}
			PHY_SetBBReg(Adapter, rCCK0_AFESetting  , 0x0F000000, 0x01); // RX path = PathAB

			//pDM_SWAT_Table->CurAntenna = pDM_SWAT_Table->PreAntenna;

			//PHY_SetBBReg(Adapter, rFPGA0_XA_RFInterfaceOE, 0x300, DM_SWAT_Table.CurAntenna);
			//pDM_SWAT_Table->SWAS_NoLink_BK_Reg860 = ((pDM_SWAT_Table->SWAS_NoLink_BK_Reg860 & 0xfffffcff) | (pDM_SWAT_Table->CurAntenna<<8));
			//PHY_SetBBReg(Adapter, rFPGA0_XA_RFInterfaceOE, bMaskDWord, pDM_SWAT_Table->SWAS_NoLink_BK_Reg860);
		}

		// Check state reset to default and wait for next time.
		//pDM_SWAT_Table->SWAS_NoLink_State = 0;
		pDM_PDTable->PathDiv_NoLink_State = 0;

		return FALSE;
	}
#else
		return	FALSE;
#endif
	
}

BOOLEAN
odm_IsConnected_92C(
	IN	PADAPTER	Adapter
)
{
	PRT_WLAN_STA	pEntry;
	PMGNT_INFO		pMgntInfo = &(Adapter->MgntInfo);
	u4Byte		i;
	BOOLEAN		bConnected=FALSE;
	
	if(pMgntInfo->mAssoc)
	{
		bConnected = TRUE;
	}
	else
	{
		for(i = 0; i < ASSOCIATE_ENTRY_NUM; i++)
		{
			if(IsAPModeExist(Adapter)  && GetFirstExtAdapter(Adapter) != NULL)
				pEntry = AsocEntry_EnumStation(GetFirstExtAdapter(Adapter), i);
			else
				pEntry = AsocEntry_EnumStation(GetDefaultAdapter(Adapter), i);

			if(pEntry!=NULL)
			{
				if(pEntry->bAssociated)
				{
					bConnected = TRUE;
					break;
				}
			}
			else
			{
				break;
			}
		}
	}
	return	bConnected;
}


VOID
odm_PathDiversityAfterLink_92C(
	IN	PADAPTER	Adapter
)
{
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	pPD_T		pDM_PDTable = &Adapter->DM_PDTable;
	u1Byte		DefaultRespPath=0;

	if((!IS_92C_SERIAL(pHalData->VersionID)) || (pHalData->PathDivCfg != 1) || (pHalData->eRFPowerState == eRfOff))
	{
		if(pHalData->PathDivCfg == 0)
		{
			RT_TRACE(	COMP_SWAS, DBG_LOUD, ("No ODM_TXPathDiversity()\n"));
		}
		else
		{
			RT_TRACE(	COMP_SWAS, DBG_LOUD, ("2T ODM_TXPathDiversity()\n"));
		}
		return;
	}
	if(!odm_IsConnected_92C(Adapter))
	{
		RT_TRACE(	COMP_SWAS, DBG_LOUD, ("ODM_TXPathDiversity(): No Connections\n"));
		return;
	}
	
	
	if(pDM_PDTable->TrainingState == 0)
	{
		RT_TRACE(	COMP_SWAS, DBG_LOUD, ("ODM_TXPathDiversity() ==>\n"));
		odm_OFDMTXPathDiversity_92C(Adapter);

		if((pDM_PDTable->CCKPathDivEnable == TRUE) && (pDM_PDTable->OFDM_Pkt_Cnt < 100))
		{
			//RT_TRACE(	COMP_SWAS, DBG_LOUD, ("odm_CCKTXPathDiversity_92C: TrainingState=0\n"));
			
			if(pDM_PDTable->CCK_Pkt_Cnt > 300)
				pDM_PDTable->Timer = 20;
			else if(pDM_PDTable->CCK_Pkt_Cnt > 100)
				pDM_PDTable->Timer = 60;
			else
				pDM_PDTable->Timer = 250;
			RT_TRACE(	COMP_SWAS, DBG_LOUD, ("odm_CCKTXPathDiversity_92C: timer=%d\n",pDM_PDTable->Timer));

			PHY_SetBBReg(Adapter, rCCK0_AFESetting  , 0x0F000000, 0x00); // RX path = PathA
			pDM_PDTable->TrainingState = 1;
			pHalData->RSSI_test = TRUE;
			PlatformSetTimer( Adapter, &pHalData->CCKPathDiversityTimer, pDM_PDTable->Timer); //ms
		}
		else
		{
			pDM_PDTable->CCKTXPath = pDM_PDTable->OFDMTXPath;
			DefaultRespPath = pDM_PDTable->OFDMDefaultRespPath;
			RT_TRACE(	COMP_SWAS, DBG_LOUD, ("odm_SetRespPath_92C: Skip odm_CCKTXPathDiversity_92C, DefaultRespPath is OFDM\n"));
			odm_SetRespPath_92C(Adapter, DefaultRespPath);
			odm_ResetPathDiversity_92C(Adapter);
			RT_TRACE(	COMP_SWAS, DBG_LOUD, ("ODM_TXPathDiversity() <==\n"));
		}
	}
	else if(pDM_PDTable->TrainingState == 1)
	{
		//RT_TRACE(	COMP_SWAS, DBG_LOUD, ("odm_CCKTXPathDiversity_92C: TrainingState=1\n"));
		PHY_SetBBReg(Adapter, rCCK0_AFESetting  , 0x0F000000, 0x05); // RX path = PathB
		pDM_PDTable->TrainingState = 2;
		PlatformSetTimer( Adapter, &pHalData->CCKPathDiversityTimer, pDM_PDTable->Timer); //ms
	}
	else
	{
		//RT_TRACE(	COMP_SWAS, DBG_LOUD, ("odm_CCKTXPathDiversity_92C: TrainingState=2\n"));
		pDM_PDTable->TrainingState = 0;	
		odm_CCKTXPathDiversity_92C(Adapter); 
		if(pDM_PDTable->OFDM_Pkt_Cnt != 0)
		{
			DefaultRespPath = pDM_PDTable->OFDMDefaultRespPath;
			RT_TRACE(	COMP_SWAS, DBG_LOUD, ("odm_SetRespPath_92C: DefaultRespPath is OFDM\n"));
		}
		else
		{
			DefaultRespPath = pDM_PDTable->CCKDefaultRespPath;
			RT_TRACE(	COMP_SWAS, DBG_LOUD, ("odm_SetRespPath_92C: DefaultRespPath is CCK\n"));
		}
		odm_SetRespPath_92C(Adapter, DefaultRespPath);
		odm_ResetPathDiversity_92C(Adapter);
		RT_TRACE(	COMP_SWAS, DBG_LOUD, ("ODM_TXPathDiversity() <==\n"));
	}

}

VOID
odm_SetRespPath_92C(
	IN	PADAPTER	Adapter,
	IN	u1Byte	DefaultRespPath )
{
	pPD_T	pDM_PDTable = &Adapter->DM_PDTable;

	RT_TRACE(	COMP_SWAS, DBG_LOUD, ("odm_SetRespPath_92C: Select Response Path=%d\n",DefaultRespPath));
	if(DefaultRespPath != pDM_PDTable->DefaultRespPath)
	{
		if(DefaultRespPath == 0)
		{
			PlatformEFIOWrite1Byte(Adapter, 0x6D8, (PlatformEFIORead1Byte(Adapter, 0x6D8)&0xc0)|0x15);	
		}
		else
		{
			PlatformEFIOWrite1Byte(Adapter, 0x6D8, (PlatformEFIORead1Byte(Adapter, 0x6D8)&0xc0)|0x2A);
		}	
	}
	pDM_PDTable->DefaultRespPath = DefaultRespPath;
}

VOID
odm_OFDMTXPathDiversity_92C(
	IN	PADAPTER	Adapter)
{
//	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	PMGNT_INFO		pMgntInfo = &(Adapter->MgntInfo);
	PRT_WLAN_STA	pEntry;
	u1Byte	i, DefaultRespPath = 0;
	s4Byte	MinRSSI = 0xFF;
	pPD_T	pDM_PDTable = &Adapter->DM_PDTable;
	pDM_PDTable->OFDMTXPath = 0;
	
	//1 Default Port
	if(pMgntInfo->mAssoc)
	{
		RT_TRACE(	COMP_SWAS, DBG_LOUD, ("odm_OFDMTXPathDiversity_92C: Default port RSSI[0]=%d, RSSI[1]=%d\n",
			Adapter->RxStats.RxRSSIPercentage[0], Adapter->RxStats.RxRSSIPercentage[1]));
		if(Adapter->RxStats.RxRSSIPercentage[0] > Adapter->RxStats.RxRSSIPercentage[1])
		{
			pDM_PDTable->OFDMTXPath = pDM_PDTable->OFDMTXPath & (~BIT0);
			MinRSSI =  Adapter->RxStats.RxRSSIPercentage[1];
			DefaultRespPath = 0;
			RT_TRACE(	COMP_SWAS, DBG_LOUD, ("odm_OFDMTXPathDiversity_92C: Default port Select Path-0\n"));
		}
		else
		{
			pDM_PDTable->OFDMTXPath =  pDM_PDTable->OFDMTXPath | BIT0;
			MinRSSI =  Adapter->RxStats.RxRSSIPercentage[0];
			DefaultRespPath = 1;
			RT_TRACE(	COMP_SWAS, DBG_LOUD, ("odm_OFDMTXPathDiversity_92C: Default port Select Path-1\n"));
		}
		//RT_TRACE(	COMP_SWAS, DBG_LOUD, ("pDM_PDTable->OFDMTXPath =0x%x\n",pDM_PDTable->OFDMTXPath));
	}
	//1 Extension Port
	for(i = 0; i < ASSOCIATE_ENTRY_NUM; i++)
	{
		if(IsAPModeExist(Adapter)  && GetFirstExtAdapter(Adapter) != NULL)
			pEntry = AsocEntry_EnumStation(GetFirstExtAdapter(Adapter), i);
		else
			pEntry = AsocEntry_EnumStation(GetDefaultAdapter(Adapter), i);

		if(pEntry!=NULL)
		{
			if(pEntry->bAssociated)
			{
				RT_TRACE(	COMP_SWAS, DBG_LOUD, ("odm_OFDMTXPathDiversity_92C: MACID=%d, RSSI_0=%d, RSSI_1=%d\n", 
					pEntry->AID+1, pEntry->rssi_stat.RxRSSIPercentage[0], pEntry->rssi_stat.RxRSSIPercentage[1]));
				
				if(pEntry->rssi_stat.RxRSSIPercentage[0] > pEntry->rssi_stat.RxRSSIPercentage[1])
				{
					pDM_PDTable->OFDMTXPath = pDM_PDTable->OFDMTXPath & ~(BIT(pEntry->AID+1));
					//pHalData->TXPath = pHalData->TXPath & ~(1<<(pEntry->AID+1));
					RT_TRACE(	COMP_SWAS, DBG_LOUD, ("odm_OFDMTXPathDiversity_92C: MACID=%d Select Path-0\n", pEntry->AID+1));
					if(pEntry->rssi_stat.RxRSSIPercentage[1] < MinRSSI)
					{
						MinRSSI = pEntry->rssi_stat.RxRSSIPercentage[1];
						DefaultRespPath = 0;
					}
				}
				else
				{
					pDM_PDTable->OFDMTXPath = pDM_PDTable->OFDMTXPath | BIT(pEntry->AID+1);
					//pHalData->TXPath = pHalData->TXPath | (1 << (pEntry->AID+1));
					RT_TRACE(	COMP_SWAS, DBG_LOUD, ("odm_OFDMTXPathDiversity_92C: MACID=%d Select Path-1\n", pEntry->AID+1));
					if(pEntry->rssi_stat.RxRSSIPercentage[0] < MinRSSI)
					{
						MinRSSI = pEntry->rssi_stat.RxRSSIPercentage[0];
						DefaultRespPath = 1;
					}
				}
			}
		}
		else
		{
			break;
		}
	}

	pDM_PDTable->OFDMDefaultRespPath = DefaultRespPath;
}


VOID
odm_CCKTXPathDiversity_92C(
	IN	PADAPTER	Adapter
)
{
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	PMGNT_INFO		pMgntInfo = &(Adapter->MgntInfo);
	PRT_WLAN_STA	pEntry;
	s4Byte	MinRSSI = 0xFF;
	u1Byte	i, DefaultRespPath = 0;
//	BOOLEAN	bBModePathDiv = FALSE;
	pPD_T	pDM_PDTable = &Adapter->DM_PDTable;

	//1 Default Port
	if(pMgntInfo->mAssoc)
	{
		if(pHalData->OFDM_Pkt_Cnt == 0)
		{
			for(i=0; i<2; i++)
			{
				if(pDM_PDTable->RSSI_CCK_Path_cnt[i] > 1) //Because the first packet is discarded
					pDM_PDTable->RSSI_CCK_Path[i] = pDM_PDTable->RSSI_CCK_Path[i] / (pDM_PDTable->RSSI_CCK_Path_cnt[i]-1);
				else
					pDM_PDTable->RSSI_CCK_Path[i] = 0;
			}
			RT_TRACE(	COMP_SWAS, DBG_LOUD, ("odm_CCKTXPathDiversity_92C: pDM_PDTable->RSSI_CCK_Path[0]=%d, pDM_PDTable->RSSI_CCK_Path[1]=%d\n",
				pDM_PDTable->RSSI_CCK_Path[0], pDM_PDTable->RSSI_CCK_Path[1]));
			RT_TRACE(	COMP_SWAS, DBG_LOUD, ("odm_CCKTXPathDiversity_92C: pDM_PDTable->RSSI_CCK_Path_cnt[0]=%d, pDM_PDTable->RSSI_CCK_Path_cnt[1]=%d\n",
				pDM_PDTable->RSSI_CCK_Path_cnt[0], pDM_PDTable->RSSI_CCK_Path_cnt[1]));
		
			if(pDM_PDTable->RSSI_CCK_Path[0] > pDM_PDTable->RSSI_CCK_Path[1])
			{
				pDM_PDTable->CCKTXPath = pDM_PDTable->CCKTXPath & (~BIT0);
				MinRSSI =  pDM_PDTable->RSSI_CCK_Path[1];
				DefaultRespPath = 0;
				RT_TRACE(	COMP_SWAS, DBG_LOUD, ("odm_CCKTXPathDiversity_92C: Default port Select CCK Path-0\n"));
			}
			else if(pDM_PDTable->RSSI_CCK_Path[0] < pDM_PDTable->RSSI_CCK_Path[1])
			{
				pDM_PDTable->CCKTXPath =  pDM_PDTable->CCKTXPath | BIT0;
				MinRSSI =  pDM_PDTable->RSSI_CCK_Path[0];
				DefaultRespPath = 1;
				RT_TRACE(	COMP_SWAS, DBG_LOUD, ("odm_CCKTXPathDiversity_92C: Default port Select CCK Path-1\n"));
			}
			else
			{
				if((pDM_PDTable->RSSI_CCK_Path[0] != 0) && (pDM_PDTable->RSSI_CCK_Path[0] < MinRSSI))
				{
					pDM_PDTable->CCKTXPath = pDM_PDTable->CCKTXPath & (~BIT0);
					RT_TRACE(	COMP_SWAS, DBG_LOUD, ("odm_CCKTXPathDiversity_92C: Default port Select CCK Path-0\n"));
					MinRSSI =  pDM_PDTable->RSSI_CCK_Path[1];
					DefaultRespPath = 0;
				}
				else
				{
					RT_TRACE(	COMP_SWAS, DBG_LOUD, ("odm_CCKTXPathDiversity_92C: Default port unchange CCK Path\n"));
				}
			}
		}
		else //Follow OFDM decision
		{
			pDM_PDTable->CCKTXPath = (pDM_PDTable->CCKTXPath & (~BIT0)) | (pDM_PDTable->OFDMTXPath &BIT0);
			RT_TRACE(	COMP_SWAS, DBG_LOUD, ("odm_CCKTXPathDiversity_92C: Follow OFDM decision, Default port Select CCK Path-%d\n",
				pDM_PDTable->CCKTXPath &BIT0));
		}
	}
	//1 Extension Port
	for(i = 0; i < ASSOCIATE_ENTRY_NUM; i++)
	{
		if(IsAPModeExist(Adapter)  && GetFirstExtAdapter(Adapter) != NULL)
			pEntry = AsocEntry_EnumStation(GetFirstExtAdapter(Adapter), i);
		else
			pEntry = AsocEntry_EnumStation(GetDefaultAdapter(Adapter), i);

		if(pEntry!=NULL)
		{
			if(pEntry->bAssociated)
			{
				if(pEntry->rssi_stat.OFDM_Pkt_Cnt == 0)
				{
					for(i=0; i<2; i++)
					{
						if(pEntry->rssi_stat.RSSI_CCK_Path_cnt[i] > 1)
							pEntry->rssi_stat.RSSI_CCK_Path[i] = pEntry->rssi_stat.RSSI_CCK_Path[i] / (pEntry->rssi_stat.RSSI_CCK_Path_cnt[i]-1);
						else
							pEntry->rssi_stat.RSSI_CCK_Path[i] = 0;
					}
					RT_TRACE(	COMP_SWAS, DBG_LOUD, ("odm_CCKTXPathDiversity_92C: MACID=%d, RSSI_CCK0=%d, RSSI_CCK1=%d\n", 
						pEntry->AID+1, pEntry->rssi_stat.RSSI_CCK_Path[0], pEntry->rssi_stat.RSSI_CCK_Path[1]));
					
					if(pEntry->rssi_stat.RSSI_CCK_Path[0] >pEntry->rssi_stat.RSSI_CCK_Path[1])
					{
						pDM_PDTable->CCKTXPath = pDM_PDTable->CCKTXPath & ~(BIT(pEntry->AID+1));
						RT_TRACE(	COMP_SWAS, DBG_LOUD, ("odm_CCKTXPathDiversity_92C: MACID=%d Select CCK Path-0\n", pEntry->AID+1));
						if(pEntry->rssi_stat.RSSI_CCK_Path[1] < MinRSSI)
						{
							MinRSSI = pEntry->rssi_stat.RSSI_CCK_Path[1];
							DefaultRespPath = 0;
						}
					}
					else if(pEntry->rssi_stat.RSSI_CCK_Path[0] <pEntry->rssi_stat.RSSI_CCK_Path[1])
					{
						pDM_PDTable->CCKTXPath = pDM_PDTable->CCKTXPath | BIT(pEntry->AID+1);
						RT_TRACE(	COMP_SWAS, DBG_LOUD, ("odm_CCKTXPathDiversity_92C: MACID=%d Select CCK Path-1\n", pEntry->AID+1));
						if(pEntry->rssi_stat.RSSI_CCK_Path[0] < MinRSSI)
						{
							MinRSSI = pEntry->rssi_stat.RSSI_CCK_Path[0];
							DefaultRespPath = 1;
						}
					}
					else
					{
						if((pEntry->rssi_stat.RSSI_CCK_Path[0] != 0) && (pEntry->rssi_stat.RSSI_CCK_Path[0] < MinRSSI))
						{
							pDM_PDTable->CCKTXPath = pDM_PDTable->CCKTXPath & ~(BIT(pEntry->AID+1));
							MinRSSI = pEntry->rssi_stat.RSSI_CCK_Path[1];
							DefaultRespPath = 0;
							RT_TRACE(	COMP_SWAS, DBG_LOUD, ("odm_CCKTXPathDiversity_92C: MACID=%d Select CCK Path-0\n", pEntry->AID+1));
						}
						else
						{
							RT_TRACE(	COMP_SWAS, DBG_LOUD, ("odm_CCKTXPathDiversity_92C: MACID=%d unchange CCK Path\n", pEntry->AID+1));
						}
					}
				}
				else //Follow OFDM decision
				{
					pDM_PDTable->CCKTXPath = (pDM_PDTable->CCKTXPath & (~(BIT(pEntry->AID+1)))) | (pDM_PDTable->OFDMTXPath & BIT(pEntry->AID+1));
					RT_TRACE(	COMP_SWAS, DBG_LOUD, ("odm_CCKTXPathDiversity_92C: Follow OFDM decision, MACID=%d Select CCK Path-%d\n",
						pEntry->AID+1, (pDM_PDTable->CCKTXPath & BIT(pEntry->AID+1))>>(pEntry->AID+1)));
				}
			}
		}
		else
		{
			break;
		}
	}

	RT_TRACE(	COMP_SWAS, DBG_LOUD, ("odm_CCKTXPathDiversity_92C:MinRSSI=%d\n",MinRSSI));

	if(MinRSSI == 0xFF)
		DefaultRespPath = pDM_PDTable->CCKDefaultRespPath;

	pDM_PDTable->CCKDefaultRespPath = DefaultRespPath;
}

VOID
ODM_CCKTXPathDiversityCallback(
	PRT_TIMER		pTimer
)
{
#if USE_WORKITEM
       PADAPTER	Adapter = (PADAPTER)pTimer->Adapter;
       HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
#else
	PADAPTER	Adapter = (PADAPTER)pTimer->Adapter;
#endif

#if DEV_BUS_TYPE==RT_PCI_INTERFACE
#if USE_WORKITEM
	PlatformScheduleWorkItem(&pHalData->CCKPathDiversityWorkitem);
#else
	odm_PathDiversityAfterLink_92C(Adapter);
#endif
#else
	PlatformScheduleWorkItem(&pHalData->CCKPathDiversityWorkitem);
#endif

}

VOID
odm_CCKTXPathDiversityWorkItemCallback(
    IN PVOID            pContext
    )
{
	PADAPTER	Adapter = (PADAPTER)pContext;

	odm_CCKTXPathDiversity_92C(Adapter);
}

VOID
ODM_CCKPathDiversityChkPerPktRssi(
	PADAPTER		Adapter,
	BOOLEAN			bIsDefPort,
	BOOLEAN			bMatchBSSID,
	PRT_WLAN_STA	pEntry,
	PRT_RFD			pRfd,
	pu1Byte			pDesc
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	BOOLEAN			bCount = FALSE;
	pPD_T	pDM_PDTable = &Adapter->DM_PDTable;
	BOOLEAN	isCCKrate = RX_HAL_IS_CCK_RATE(pDesc);

	if((pHalData->PathDivCfg != 1) || (pHalData->RSSI_test == FALSE))
		return;
		
	if(pHalData->RSSI_target==NULL && bIsDefPort && bMatchBSSID)
		bCount = TRUE;
	else if(pHalData->RSSI_target!=NULL && pEntry!=NULL && pHalData->RSSI_target==pEntry)
		bCount = TRUE;

	if(bCount && isCCKrate)
	{
		if(pDM_PDTable->TrainingState == 1 )
		{
			if(pEntry)
			{
				if(pEntry->rssi_stat.RSSI_CCK_Path_cnt[0] != 0)
					pEntry->rssi_stat.RSSI_CCK_Path[0] += pRfd->Status.RxPWDBAll;
				pEntry->rssi_stat.RSSI_CCK_Path_cnt[0]++;
			}
			else
			{
				if(pDM_PDTable->RSSI_CCK_Path_cnt[0] != 0)
					pDM_PDTable->RSSI_CCK_Path[0] += pRfd->Status.RxPWDBAll;
				pDM_PDTable->RSSI_CCK_Path_cnt[0]++;
			}
		}
		else if(pDM_PDTable->TrainingState == 2 )
		{
			if(pEntry)
			{
				if(pEntry->rssi_stat.RSSI_CCK_Path_cnt[1] != 0)
					pEntry->rssi_stat.RSSI_CCK_Path[1] += pRfd->Status.RxPWDBAll;
				pEntry->rssi_stat.RSSI_CCK_Path_cnt[1]++;
			}
			else
			{
				if(pDM_PDTable->RSSI_CCK_Path_cnt[1] != 0)
					pDM_PDTable->RSSI_CCK_Path[1] += pRfd->Status.RxPWDBAll;
				pDM_PDTable->RSSI_CCK_Path_cnt[1]++;
			}
		}
	}
}

VOID
ODM_FillTXPathInTXDESC(
		IN	PADAPTER	Adapter,
		IN	PRT_TCB		pTcb,
		IN	pu1Byte		pDesc
)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u4Byte	TXPath;
	pPD_T	pDM_PDTable = &Adapter->DM_PDTable;

	//2011.09.05  Add by Luke Lee for path diversity
	if(pHalData->PathDivCfg == 1)
	{	
		TXPath = (pDM_PDTable->OFDMTXPath >> pTcb->macId) & BIT0;
		//RT_TRACE(	COMP_SWAS, DBG_LOUD, ("Fill TXDESC: macID=%d, TXPath=%d\n", pTcb->macId, TXPath));
		//SET_TX_DESC_TX_ANT_CCK(pDesc,TXPath);
		if(TXPath == 0)
		{
			SET_TX_DESC_TX_ANTL_92C(pDesc,1);
			SET_TX_DESC_TX_ANT_HT_92C(pDesc,1);
		}
		else
		{
			SET_TX_DESC_TX_ANTL_92C(pDesc,2);
			SET_TX_DESC_TX_ANT_HT_92C(pDesc,2);
		}
		TXPath = (pDM_PDTable->CCKTXPath >> pTcb->macId) & BIT0;
		if(TXPath == 0)
		{
			SET_TX_DESC_TX_ANT_CCK_92C(pDesc,1);
		}
		else
		{
			SET_TX_DESC_TX_ANT_CCK_92C(pDesc,2);
		}
	}
}

VOID
odm_ResetPathDiversity_92C(
		IN	PADAPTER	Adapter
)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	pPD_T	pDM_PDTable = &Adapter->DM_PDTable;
	PRT_WLAN_STA	pEntry;
	u4Byte	i;

	pHalData->RSSI_test = FALSE;
	pDM_PDTable->CCK_Pkt_Cnt = 0;
	pDM_PDTable->OFDM_Pkt_Cnt = 0;
	pHalData->CCK_Pkt_Cnt =0;
	pHalData->OFDM_Pkt_Cnt =0;
	
	if(pDM_PDTable->CCKPathDivEnable == TRUE)	
		PHY_SetBBReg(Adapter, rCCK0_AFESetting  , 0x0F000000, 0x01); //RX path = PathAB

	for(i=0; i<2; i++)
	{
		pDM_PDTable->RSSI_CCK_Path_cnt[i]=0;
		pDM_PDTable->RSSI_CCK_Path[i] = 0;
	}
	for(i = 0; i < ASSOCIATE_ENTRY_NUM; i++)
	{
		if(IsAPModeExist(Adapter)  && GetFirstExtAdapter(Adapter) != NULL)
			pEntry = AsocEntry_EnumStation(GetFirstExtAdapter(Adapter), i);
		else
			pEntry = AsocEntry_EnumStation(GetDefaultAdapter(Adapter), i);

		if(pEntry!=NULL)
		{
			pEntry->rssi_stat.CCK_Pkt_Cnt = 0;
			pEntry->rssi_stat.OFDM_Pkt_Cnt = 0;
			for(i=0; i<2; i++)
			{
				pEntry->rssi_stat.RSSI_CCK_Path_cnt[i] = 0;
				pEntry->rssi_stat.RSSI_CCK_Path[i] = 0;
			}
		}
		else
			break;
	}
}



//Neil Chen---2011--06--22
//----92D Path Diversity----//
//#ifdef PathDiv92D
//==================================
//3 Path Diversity 
//==================================
//
//
// 20100503 Joseph:
// Add new function SwAntDivCheck8192C().
// This is the main function of Antenna diversity function before link.
// Mainly, it just retains last scan result and scan again.
// After that, it compares the scan result to see which one gets better RSSI.
// It selects antenna with better receiving power and returns better scan result.
//
//
// 20100514 Luke/Joseph:
// This function is used to gather the RSSI information for antenna testing.
// It selects the RSSI of the peer STA that we want to know.
//
VOID
ODM_PathDivChkPerPktRssi(
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



//
// 20100514 Luke/Joseph:
// Add new function to reset antenna diversity state after link.
//
VOID
ODM_PathDivRestAfterLink(
	IN	PADAPTER	Adapter
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	pSWAT_T		pDM_SWAT_Table = &Adapter->DM_SWAT_Table;

	pHalData->RSSI_cnt_A = 0;
	pHalData->RSSI_cnt_B = 0;
	pHalData->RSSI_test = FALSE;
	pDM_SWAT_Table->try_flag = 0x0;       // NOT 0xff
	pDM_SWAT_Table->RSSI_Trying = 0;
	pDM_SWAT_Table->SelectAntennaMap=0xAA;
	pDM_SWAT_Table->CurAntenna = Antenna_A;  
}


//
// 20100514 Luke/Joseph:
// Callback function for 500ms antenna test trying.
//
VOID
ODM_PathDivChkAntSwitchCallback(
	PRT_TIMER		pTimer
)
{
	PADAPTER		Adapter = (PADAPTER)pTimer->Adapter;
#if USE_WORKITEM
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
#endif

#if DEV_BUS_TYPE==RT_PCI_INTERFACE

#if USE_WORKITEM
	PlatformScheduleWorkItem(&pHalData->PathDivSwitchWorkitem);
#else
	odm_PathDivChkAntSwitch(Adapter, SWAW_STEP_DETERMINE);
#endif
#else
	PlatformScheduleWorkItem(&pHalData->PathDivSwitchWorkitem);
#endif

//odm_SwAntDivChkAntSwitch(Adapter, SWAW_STEP_DETERMINE);

}


VOID
ODM_PathDivChkAntSwitchWorkitemCallback(
    IN PVOID            pContext
    )
{
	PADAPTER	pAdapter = (PADAPTER)pContext;

	odm_PathDivChkAntSwitch(pAdapter, SWAW_STEP_DETERMINE);
}


 //MAC0_ACCESS_PHY1

// 2011-06-22 Neil Chen & Gary Hsin
// Refer to Jr.Luke's SW ANT DIV
// 92D Path Diversity Main function
// refer to 88C software antenna diversity
// 
VOID
odm_PathDivChkAntSwitch(
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
       // Neil Chen
       static u1Byte        pathdiv_para=0x0;     
       static u1Byte        switchfirsttime=0x00;
       u1Byte                 regB33 = (u1Byte) PHY_QueryBBReg(Adapter, 0xB30,BIT27);
       //u1Byte                 reg637 =0x0;   
       static u1Byte        fw_value=0x0;         
       PADAPTER            BuddyAdapter = Adapter->BuddyAdapter;     // another adapter MAC
        // Path Diversity   //Neil Chen--2011--06--22
	u1Byte                 PathDiv_Trigger = (u1Byte) PHY_QueryBBReg(Adapter, 0xBA0,BIT31);
	u1Byte                 PathDiv_Enable = pHalData->bPathDiv_Enable;

	//DbgPrint("Path Div PG Value:%x \n",PathDiv_Enable);	
       if((BuddyAdapter==NULL)||(!PathDiv_Enable)||(PathDiv_Trigger)||(pHalData->CurrentBandType92D == BAND_ON_2_4G))
       {
           return;
       }
	RT_TRACE(COMP_ANTENNA,DBG_LOUD,("===================>odm_PathDivChkAntSwitch()\n"));

       // The first time to switch path excluding 2nd, 3rd, ....etc....
	if(switchfirsttime==0)
	{
	    if(regB33==0)
	    {
	       pDM_SWAT_Table->CurAntenna = Antenna_A;    // Default MAC0_5G-->Path A (current antenna)     
	    }	    
	}

	// Condition that does not need to use antenna diversity.
	if(IS_8723_SERIES(pHalData->VersionID) ||IS_92C_SERIAL(pHalData->VersionID) )
	{
		RT_TRACE(COMP_ANTENNA, DBG_LOUD, ("odm_PathDiversityMechanims(): No PathDiv Mechanism.\n"));
		return;
	}

	// Radio off: Status reset to default and return.
	if(pHalData->eRFPowerState==eRfOff)
	{
		//ODM_SwAntDivRestAfterLink(Adapter);
		return;
	}

       /*
	// Handling step mismatch condition.
	// Peak step is not finished at last time. Recover the variable and check again.
	if(	Step != pDM_SWAT_Table->try_flag	)
	{
		ODM_SwAntDivRestAfterLink(Adapter);
	} */
	
	if(pDM_SWAT_Table->try_flag == 0xff)
	{
		// Select RSSI checking target
		if(pMgntInfo->mAssoc && !ACTING_AS_AP(Adapter))
		{
			// Target: Infrastructure mode AP.
			pHalData->RSSI_target = NULL;
			RT_TRACE(COMP_ANTENNA, DBG_LOUD, ("odm_PathDivMechanism(): RSSI_target is DEF AP!\n"));
		}
		else
		{
			u1Byte			index = 0;
			PRT_WLAN_STA	pEntry = NULL;
			PADAPTER		pTargetAdapter = NULL;
		
			if(	pMgntInfo->mIbss || ACTING_AS_AP(Adapter) )
			{
				// Target: AP/IBSS peer.
				pTargetAdapter = Adapter;
			}
			else if(IsAPModeExist(Adapter)  && GetFirstExtAdapter(Adapter) != NULL)
			{
				// Target: VWIFI peer.
				pTargetAdapter = GetFirstExtAdapter(Adapter);
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
				ODM_PathDivRestAfterLink(Adapter);
				RT_TRACE(COMP_ANTENNA, DBG_LOUD, ("odm_SwAntDivChkAntSwitch(): No Link.\n"));
				return;
			}
			else
			{
				pHalData->RSSI_target = pEntry;
				RT_TRACE(COMP_ANTENNA, DBG_LOUD, ("odm_SwAntDivChkAntSwitch(): RSSI_target is PEER STA\n"));
			}
		}
			
		pHalData->RSSI_cnt_A = 0;
		pHalData->RSSI_cnt_B = 0;
		pDM_SWAT_Table->try_flag = 0;
		RT_TRACE(COMP_ANTENNA, DBG_LOUD, ("odm_SwAntDivChkAntSwitch(): Set try_flag to 0 prepare for peak!\n"));
		return;
	}
	else
	{
	       // 1st step
		curTxOkCnt = Adapter->TxStats.NumTxBytesUnicast - lastTxOkCnt;
		curRxOkCnt = Adapter->RxStats.NumRxBytesUnicast - lastRxOkCnt;
		lastTxOkCnt = Adapter->TxStats.NumTxBytesUnicast;
		lastRxOkCnt = Adapter->RxStats.NumRxBytesUnicast;
	
		if(pDM_SWAT_Table->try_flag == 1)   // Training State
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
			RT_TRACE(COMP_ANTENNA,DBG_LOUD,("=PATH DIV=: RSSI_Trying = %d\n",pDM_SWAT_Table->RSSI_Trying));
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
				RT_TRACE(COMP_ANTENNA,DBG_LOUD,("=PATH DIV=: PreRSSI = %d, CurRSSI = %d\n",pDM_SWAT_Table->PreRSSI, curRSSI));
				RT_TRACE(COMP_ANTENNA,DBG_LOUD,("=PATH DIV=: preAntenna= %s, curAntenna= %s \n", 
				(pDM_SWAT_Table->PreAntenna == Antenna_A?"A":"B"), (pDM_SWAT_Table->CurAntenna == Antenna_A?"A":"B")));
				RT_TRACE(COMP_ANTENNA,DBG_LOUD,("=PATH DIV=: RSSI_A= %d, RSSI_cnt_A = %d, RSSI_B= %d, RSSI_cnt_B = %d\n",
					RSSI_A, pHalData->RSSI_cnt_A, RSSI_B, pHalData->RSSI_cnt_B));
			}

		}
		else   // try_flag=0
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
			RT_TRACE(COMP_ANTENNA,DBG_LOUD, ("=PATH DIV=: PreRSSI = %d, CurRSSI = %d\n", pDM_SWAT_Table->PreRSSI, curRSSI));
		       RT_TRACE(COMP_ANTENNA,DBG_LOUD, ("=PATH DIV=: preAntenna= %s, curAntenna= %s \n", 
			(pDM_SWAT_Table->PreAntenna == Antenna_A?"A":"B"), (pDM_SWAT_Table->CurAntenna == Antenna_A?"A":"B")));

			RT_TRACE(COMP_ANTENNA, DBG_LOUD, ("=PATH DIV=: RSSI_A= %d, RSSI_cnt_A = %d, RSSI_B= %d, RSSI_cnt_B = %d\n",
				RSSI_A, pHalData->RSSI_cnt_A, RSSI_B, pHalData->RSSI_cnt_B));
			//RT_TRACE(COMP_SWAS, DBG_LOUD, ("Ekul:curTxOkCnt = %d\n", curTxOkCnt));
			//RT_TRACE(COMP_SWAS, DBG_LOUD, ("Ekul:curRxOkCnt = %d\n", curRxOkCnt));
		}

		//1 Trying State
		if((pDM_SWAT_Table->try_flag == 1)&&(pDM_SWAT_Table->RSSI_Trying == 0))
		{

			if(pDM_SWAT_Table->TestMode == TP_MODE)
			{
				RT_TRACE(COMP_ANTENNA, DBG_LOUD,("=PATH=: TestMode = TP_MODE"));
				RT_TRACE(COMP_ANTENNA, DBG_LOUD,("=PATH= TRY:CurByteCnt = %"i64fmt"d,", CurByteCnt));
				RT_TRACE(COMP_ANTENNA, DBG_LOUD,("=PATH= TRY:PreByteCnt = %"i64fmt"d\n",PreByteCnt));		
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
				RT_TRACE(COMP_ANTENNA, DBG_LOUD,("SelectAntennaMap=%x\n ",pDM_SWAT_Table->SelectAntennaMap));
				RT_TRACE(COMP_ANTENNA, DBG_LOUD,("=PATH=: Score_A=%d, Score_B=%d\n", Score_A, Score_B));
			
				if(pDM_SWAT_Table->CurAntenna == Antenna_A)
				{
					nextAntenna = (Score_A >= Score_B)?Antenna_A:Antenna_B;
				}
				else
				{
					nextAntenna = (Score_B >= Score_A)?Antenna_B:Antenna_A;
				}
				RT_TRACE(COMP_ANTENNA, DBG_LOUD,("=PATH=: nextAntenna=%s\n",(nextAntenna==Antenna_A)?"A":"B"));
				RT_TRACE(COMP_ANTENNA, DBG_LOUD, ("=PATH=: preAntenna= %s, curAntenna= %s \n", 
				(pDM_SWAT_Table->PreAntenna == Antenna_A?"A":"B"), (pDM_SWAT_Table->CurAntenna == Antenna_A?"A":"B")));

				if(nextAntenna != pDM_SWAT_Table->CurAntenna)
				{
					RT_TRACE(COMP_ANTENNA, DBG_LOUD,("=PATH=: Switch back to another antenna"));
				}
				else
				{
					RT_TRACE(COMP_ANTENNA, DBG_LOUD,("=PATH=: current anntena is good\n"));
				}	
			}

                    
			if(pDM_SWAT_Table->TestMode == RSSI_MODE)
			{	
				RT_TRACE(COMP_ANTENNA, DBG_LOUD,("=PATH=: TestMode = RSSI_MODE"));
				pDM_SWAT_Table->SelectAntennaMap=0xAA;
				if(curRSSI < pDM_SWAT_Table->PreRSSI) //Current antenna is worse than previous antenna
				{
					//RT_TRACE(COMP_SWAS, DBG_LOUD, ("SWAS: Switch back to another antenna"));
					nextAntenna = (pDM_SWAT_Table->CurAntenna == Antenna_A)? Antenna_B : Antenna_A;
				}
				else // current anntena is good
				{
					nextAntenna =pDM_SWAT_Table->CurAntenna;
					//RT_TRACE(COMP_SWAS, DBG_LOUD, ("SWAS: current anntena is good\n"));
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
#if DEV_BUS_TYPE==RT_PCI_INTERFACE
	                    pDM_SWAT_Table->RSSI_Trying = 4;                           
#else
	                    pDM_SWAT_Table->RSSI_Trying = 2;
#endif
				pDM_SWAT_Table->TestMode = TP_MODE;
			}
			else
			{
				pDM_SWAT_Table->RSSI_Trying = 2;
				pDM_SWAT_Table->TestMode = RSSI_MODE;

			}
                          
			//RT_TRACE(COMP_SWAS, DBG_LOUD, ("SWAS: Normal State -> Begin Trying!\n"));			
			pHalData->RSSI_sum_A = 0;
			pHalData->RSSI_cnt_A = 0;
			pHalData->RSSI_sum_B = 0;
			pHalData->RSSI_cnt_B = 0;
		} // end of try_flag=0
	}
	
	//1 4.Change TRX antenna
	if(nextAntenna != pDM_SWAT_Table->CurAntenna)
	{
	
		RT_TRACE(COMP_ANTENNA, DBG_LOUD,("=PATH=: Change TX Antenna!\n "));
		//PHY_SetBBReg(Adapter, rFPGA0_XA_RFInterfaceOE, 0x300, nextAntenna); for 88C
		if(nextAntenna==Antenna_A)
		{
		    RT_TRACE(COMP_ANTENNA, DBG_LOUD,("=PATH=: Next Antenna is RF PATH A\n "));
		    pathdiv_para = 0x02;   //02 to switchback to RF path A
		    fw_value = 0x03;
#if DEV_BUS_TYPE==RT_PCI_INTERFACE
                 odm_PathDiversity_8192D(Adapter, pathdiv_para);
#else
                 FillH2CCmd92C(Adapter, H2C_92C_PathDiv,1,(pu1Byte)(&fw_value));	
#endif
		}	
	       else if(nextAntenna==Antenna_B)
	       {
	           RT_TRACE(COMP_ANTENNA, DBG_LOUD,("=PATH=: Next Antenna is RF PATH B\n "));
	           if(switchfirsttime==0)  // First Time To Enter Path Diversity
	           {
	               switchfirsttime=0x01;
                      pathdiv_para = 0x00;
			  fw_value=0x00;    // to backup RF Path A Releated Registers		  
					  
#if DEV_BUS_TYPE==RT_PCI_INTERFACE
                     odm_PathDiversity_8192D(Adapter, pathdiv_para);
#else
                     FillH2CCmd92C(Adapter, H2C_92C_PathDiv,1,(pu1Byte)(&fw_value));
                     //for(u1Byte n=0; n<80,n++)
                     //{
                     delay_us(500);
			   
                     odm_PathDiversity_8192D(Adapter, pathdiv_para);
			 		 
			 fw_value=0x01;   	// to backup RF Path A Releated Registers		 
                     FillH2CCmd92C(Adapter, H2C_92C_PathDiv,1,(pu1Byte)(&fw_value));	
#endif	
			  RT_TRACE(COMP_ANTENNA, DBG_LOUD,("=PATH=: FIRST TIME To DO PATH SWITCH!\n "));	
	           }		   
		    else
		    {
		        pathdiv_para = 0x01;
			 fw_value = 0x02;	
#if DEV_BUS_TYPE==RT_PCI_INTERFACE
                     odm_PathDiversity_8192D(Adapter, pathdiv_para);
#else
                     FillH2CCmd92C(Adapter, H2C_92C_PathDiv,1,(pu1Byte)(&fw_value));	
#endif	
		    }		
	       }
           //   odm_PathDiversity_8192D(Adapter, pathdiv_para);
	}

	//1 5.Reset Statistics
	pDM_SWAT_Table->PreAntenna = pDM_SWAT_Table->CurAntenna;
	pDM_SWAT_Table->CurAntenna = nextAntenna;
	pDM_SWAT_Table->PreRSSI = curRSSI;
       //lastTxOkCnt = Adapter->TxStats.NumTxBytesUnicast;
       //lastRxOkCnt = Adapter->RxStats.NumRxBytesUnicast;
	   
	//1 6.Set next timer

	if(pDM_SWAT_Table->RSSI_Trying == 0)
		return;

	if(pDM_SWAT_Table->RSSI_Trying%2 == 0)
	{
		if(pDM_SWAT_Table->TestMode == TP_MODE)
		{
			if(TrafficLoad == TRAFFIC_HIGH)
			{
#if DEV_BUS_TYPE==RT_PCI_INTERFACE
	                    PlatformSetTimer( Adapter, &pHalData->PathDivSwitchTimer, 10 ); //ms
				RT_TRACE(COMP_ANTENNA, DBG_LOUD,("=PATH=: Test another antenna for 10 ms\n"));
#else
				PlatformSetTimer( Adapter, &pHalData->PathDivSwitchTimer, 20 ); //ms
				RT_TRACE(COMP_ANTENNA, DBG_LOUD,("=PATH=: Test another antenna for 20 ms\n"));
#endif				
			}
			else if(TrafficLoad == TRAFFIC_LOW)
			{
				PlatformSetTimer( Adapter, &pHalData->PathDivSwitchTimer, 50 ); //ms
				RT_TRACE(COMP_ANTENNA, DBG_LOUD,("=PATH=: Test another antenna for 50 ms\n"));
			}
		}
		else   // TestMode == RSSI_MODE
		{
			PlatformSetTimer( Adapter, &pHalData->PathDivSwitchTimer, 500 ); //ms
			RT_TRACE(COMP_ANTENNA, DBG_LOUD,("=PATH=: Test another antenna for 500 ms\n"));
		}
	}
	else
	{
		if(pDM_SWAT_Table->TestMode == TP_MODE)
		{
			if(TrafficLoad == TRAFFIC_HIGH)
				
#if DEV_BUS_TYPE==RT_PCI_INTERFACE
	                    PlatformSetTimer( Adapter, &pHalData->PathDivSwitchTimer, 90 ); //ms
				//RT_TRACE(COMP_ANTENNA, DBG_LOUD,("=PATH=: Test another antenna for 90 ms\n"));
#else		
				PlatformSetTimer( Adapter, &pHalData->PathDivSwitchTimer, 180); //ms
#endif				
			else if(TrafficLoad == TRAFFIC_LOW)
				PlatformSetTimer( Adapter, &pHalData->PathDivSwitchTimer, 100 ); //ms
		}
		else
			PlatformSetTimer( Adapter, &pHalData->PathDivSwitchTimer, 500 ); //ms
	}
}

//==================================================
//3 PathDiv End
//==================================================

//3============================================================
//3 Export Interface
//3============================================================

VOID 
odm_FalseAlarmCounterStatistics_ForSlaveOfDMSP(
	IN	PADAPTER	Adapter
)
{
	PADAPTER	BuddyAdapter = Adapter->BuddyAdapter;
	PFALSE_ALARM_STATISTICS FalseAlmCnt = &(Adapter->FalseAlmCnt);
	PFALSE_ALARM_STATISTICS	FlaseAlmCntBuddyAdapter;

	if(BuddyAdapter == NULL)
		return;

	if (Adapter->DualMacSmartConcurrent == FALSE)
		return;

	FlaseAlmCntBuddyAdapter = &(BuddyAdapter->FalseAlmCnt);
	
	FalseAlmCnt->Cnt_Fast_Fsync =FlaseAlmCntBuddyAdapter->Cnt_Fast_Fsync;
	FalseAlmCnt->Cnt_SB_Search_fail =FlaseAlmCntBuddyAdapter->Cnt_SB_Search_fail;		
    	FalseAlmCnt->Cnt_Parity_Fail = FlaseAlmCntBuddyAdapter->Cnt_Parity_Fail;	
	FalseAlmCnt->Cnt_Rate_Illegal = FlaseAlmCntBuddyAdapter->Cnt_Rate_Illegal;
	FalseAlmCnt->Cnt_Crc8_fail = FlaseAlmCntBuddyAdapter->Cnt_Crc8_fail;
	FalseAlmCnt->Cnt_Mcs_fail = FlaseAlmCntBuddyAdapter->Cnt_Mcs_fail;

	FalseAlmCnt->Cnt_Ofdm_fail = 	FalseAlmCnt->Cnt_Parity_Fail + FalseAlmCnt->Cnt_Rate_Illegal +
								FalseAlmCnt->Cnt_Crc8_fail + FalseAlmCnt->Cnt_Mcs_fail +
								FalseAlmCnt->Cnt_Fast_Fsync + FalseAlmCnt->Cnt_SB_Search_fail;

	
	//hold cck counter
	FalseAlmCnt->Cnt_Cck_fail = FlaseAlmCntBuddyAdapter->Cnt_Cck_fail;

	FalseAlmCnt->Cnt_all = (	FalseAlmCnt->Cnt_Fast_Fsync + 
						FalseAlmCnt->Cnt_SB_Search_fail +
						FalseAlmCnt->Cnt_Parity_Fail +
						FalseAlmCnt->Cnt_Rate_Illegal +
						FalseAlmCnt->Cnt_Crc8_fail +
						FalseAlmCnt->Cnt_Mcs_fail +
						FalseAlmCnt->Cnt_Cck_fail);	

/*
	RT_TRACE(	COMP_DIG, DBG_LOUD, ("Cnt_Fast_Fsync = %d, Cnt_SB_Search_fail = %d\n", 
				FalseAlmCnt->Cnt_Fast_Fsync , FalseAlmCnt->Cnt_SB_Search_fail) );	
	RT_TRACE(	COMP_DIG, DBG_LOUD, ("Cnt_Parity_Fail = %d, Cnt_Rate_Illegal = %d, Cnt_Crc8_fail = %d, Cnt_Mcs_fail = %d\n", 
				FalseAlmCnt->Cnt_Parity_Fail, FalseAlmCnt->Cnt_Rate_Illegal, FalseAlmCnt->Cnt_Crc8_fail, FalseAlmCnt->Cnt_Mcs_fail) );	
	RT_TRACE(	COMP_DIG, DBG_LOUD, ("Cnt_Ofdm_fail = %d, Cnt_Cck_fail = %d, Cnt_all = %d\n", 
				FalseAlmCnt->Cnt_Ofdm_fail, FalseAlmCnt->Cnt_Cck_fail, FalseAlmCnt->Cnt_all) );		
*/
}

/************************************************************************
TRUE: Use parameter from BuddyAdapter
FALSE: USe Parameter from itself
************************************************************************/

VOID
ODM_DMWatchdog_8192C(
	IN	PADAPTER	Adapter
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	
	//
	// Dynamic Initial Gain mechanism.
	//
	if(IS_HARDWARE_TYPE_8192D(Adapter))
	{

//sherry delete flag 20110517		
#if (DEV_BUS_TYPE == RT_PCI_INTERFACE)
			ACQUIRE_GLOBAL_SPINLOCK(&GlobalSpinlockForGlobalAdapterList);
#else
			ACQUIRE_GLOBAL_MUTEX(GlobalMutexForGlobalAdapterList);
#endif
			if(Adapter->bSlaveOfDMSP)
			{
				odm_FalseAlarmCounterStatistics_ForSlaveOfDMSP(Adapter);
			}
			else
			{
				odm_FalseAlarmCounterStatistics_8192C(Adapter);
			}
#if (DEV_BUS_TYPE == RT_PCI_INTERFACE)
			RELEASE_GLOBAL_SPINLOCK(&GlobalSpinlockForGlobalAdapterList);
#else
			RELEASE_GLOBAL_MUTEX(GlobalMutexForGlobalAdapterList);
#endif
			odm_RSSIMonitorCheck(Adapter);
			odm_FindMinimumRSSI_92D(Adapter);
#if (BT_30_SUPPORT==1)
			if(BT_Operation(Adapter))
			{
				BT_DigByBtRssi(Adapter);
			}
			else
#endif				
			{
				odm_DIG_8192C(Adapter);
				if(pHalData->MacPhyMode92D == DUALMAC_SINGLEPHY)
				{
					if(pHalData->CurrentBandType92D != BAND_ON_5G)
						dm_CCK_PacketDetectionThresh_DMSP(Adapter);
				}
				else
				{
					if(pHalData->CurrentBandType92D != BAND_ON_5G)
						dm_CCK_PacketDetectionThresh(Adapter);
				}
			}
#if (DEV_BUS_TYPE==RT_PCI_INTERFACE)
			odm_DynamicEarlyMode(Adapter);
#endif

			//
			// Dynamic Tx Power mechanism.
			//
			odm_DynamicTxPower_92D(Adapter);

			//same function with 92c, vivi note
			if(!Adapter->bSlaveOfDMSP || Adapter->DualMacSmartConcurrent == FALSE)
				ODM_CheckTXPowerTracking(Adapter);
			//
			// Rate Adaptive by Rx Signal Strength mechanism.
			//
			odm_RefreshRateAdaptiveMask(Adapter);

			// EDCA turbo
			odm_CheckEdcaTurbo(Adapter);

			 //================================================================
                    //Neil Chen----2011---06--16---
                    //================================================================
                    //DbgPrint("=PATH=counter's value==>%d\n",counter);
			
                    if((pHalData->CurrentBandType92D == BAND_ON_5G )&&(pHalData->MacPhyMode92D == DUALMAC_DUALPHY))
                    { 
                        RT_TRACE(COMP_ANTENNA,DBG_LOUD,("++++++++++++++++=PATH=:  Path Diversity Start\n"));
                        odm_PathDivChkAntSwitch(Adapter, SWAW_STEP_PEAK);	   
                    }
	}
	else
	{
		odm_RSSIMonitorCheck(Adapter);
		odm_FalseAlarmCounterStatistics_8192C(Adapter);
#ifndef UNDER_CE		
		if(BT_Operation(Adapter))
		{
			BT_DigByBtRssi(Adapter);
		}
		else
#endif		
		{
#if (DEV_BUS_TYPE == RT_PCI_INTERFACE) | (DEV_BUS_TYPE == RT_USB_INTERFACE)
			odm_RXHP(Adapter);
#endif
			odm_DIG_8192C(Adapter);
			odm_CCK_PacketDetectionThresh_8192C(Adapter);
		}

		//
		//Dynamic BB Power Saving Mechanism
		//
		odm_DynamicBBPowerSaving(Adapter);

		//
		// Dynamic Tx Power mechanism.
		//
		odm_DynamicTxPower(Adapter);

		//
		// Tx Power Tracking.
		//
		//same function with 92d, vivi note
		if(!Adapter->bSlaveOfDMSP || Adapter->DualMacSmartConcurrent == FALSE)
			ODM_CheckTXPowerTracking(Adapter);

		//
		// Rate Adaptive by Rx Signal Strength mechanism.
		//
		odm_RefreshRateAdaptiveMask(Adapter);


		// EDCA turbo
		odm_CheckEdcaTurbo(Adapter);

		//
		// Software Antenna diversity
		//
		//When traffic load is high, skip once SW antenna switch
		odm_SwAntDivChkAntSwitch(Adapter, SWAW_STEP_PEAK);

		odm_PathDiversityAfterLink_92C(Adapter);

	}
}

#endif
