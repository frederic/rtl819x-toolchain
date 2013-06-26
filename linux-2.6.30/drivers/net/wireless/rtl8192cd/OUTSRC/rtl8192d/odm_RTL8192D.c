/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *                                        
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/

//============================================================
// include files
//============================================================

#include "../odm_precomp.h"

//---------------PROTO TYPE---------------//

VOID
ODM_Write_DIG_DMSP(
	IN	PDM_ODM_T		pDM_Odm,
	IN	u1Byte			CurrentIGI
	);

VOID
ODM_Write_CCK_CCA_Thres_92D(
	IN	PDM_ODM_T		pDM_Odm,
	IN	u1Byte			CurCCK_CCAThres
	);

VOID
ODM_Write_CCK_CCA_DMSP(
	IN	PDM_ODM_T		pDM_Odm,
	IN	u1Byte			CurCCK_CCAThres
	);

VOID
odm_GetCCKFalseAlarm_92D(
	IN		PDM_ODM_T		pDM_Odm
	);

VOID
odm_ResetFACounter_92D(
	IN		PDM_ODM_T		pDM_Odm
	);

VOID 
odm_FalseAlarmCounterStatistics_ForSlaveOfDMSP(
	IN		PDM_ODM_T		pDM_Odm
	);

#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
__inline static void AcquireCCKAndRWPageAControl(IN PADAPTER pAdapter){}
__inline static void ReleaseCCKAndRWPageAControl(IN PADAPTER pAdapter){}
#endif

//---------------PROTO TYPE---------------//

VOID
ODM_Write_DIG_DMSP(
	IN	PDM_ODM_T		pDM_Odm,
	IN	u1Byte			CurrentIGI
	)
{
#if (DM_ODM_SUPPORT_TYPE == ODM_MP)
	PADAPTER	pAdapter = pDM_Odm->Adapter;
	pDIG_T		pDM_DigTable = &pDM_Odm->DM_DigTable;

	PADAPTER	BuddyAdapter = pAdapter->BuddyAdapter;
	
	//BOOLEAN		bGetValueFromOtherMac = dm_DualMacGetParameterFromBuddyAdapter(pAdapter);

	if(pDM_Odm->pBuddyAdapter == NULL)
	{	
		if(*pDM_Odm->pbMasterOfDMSP)
		{
			ODM_Write_DIG(pDM_Odm, CurrentIGI);
		}
		else
		{
			pDM_DigTable->CurIGValue = CurrentIGI;//pDM_DigTable->PreIGValue = pDM_DigTable->CurIGValue;
		}
		return;
	}

	if(*pDM_Odm->pbGetValueFromOtherMac)
	{
		if(pAdapter->DualMacDMSPControl.bWriteDigForAnotherMacOfDMSP)
		{
			pAdapter->DualMacDMSPControl.bWriteDigForAnotherMacOfDMSP = FALSE;
			ODM_Write_DIG(pDM_Odm, (u1Byte) pAdapter->DualMacDMSPControl.CurDigValueForAnotherMacOfDMSP);
		}
	}

	if( pDM_DigTable->CurIGValue != CurrentIGI)//if(pDM_DigTable->PreIGValue != pDM_DigTable->CurIGValue)
	{
		 if(!(*pDM_Odm->pbMasterOfDMSP))
		 {
		 	BuddyAdapter->DualMacDMSPControl.bWriteDigForAnotherMacOfDMSP = TRUE;
			//BuddyAdapter->DualMacDMSPControl.CurDigValueForAnotherMacOfDMSP =  pDM_DigTable->CurIGValue; 
			BuddyAdapter->DualMacDMSPControl.CurDigValueForAnotherMacOfDMSP =  CurrentIGI; 
		 }
		else
		{
			if(!(*pDM_Odm->pbGetValueFromOtherMac))
			{
				ODM_Write_DIG(pDM_Odm, CurrentIGI);
			}
		}
		//pDM_DigTable->PreIGValue = pDM_DigTable->CurIGValue;
	}
#endif
}

VOID
ODM_Write_CCK_CCA_Thres_92D(
	IN	PDM_ODM_T		pDM_Odm,
	IN	u1Byte			CurCCK_CCAThres
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_MP|ODM_CE))
	PADAPTER	pAdapter = pDM_Odm->Adapter;
	pDIG_T	pDM_DigTable = &pDM_Odm->DM_DigTable;

	if(!(pDM_Odm->SupportICType == ODM_RTL8192D))
		return;

	if(*pDM_Odm->pMacPhyMode == ODM_DMSP)
	{
		ODM_Write_CCK_CCA_DMSP(pDM_Odm, CurCCK_CCAThres);
		return;
	}
	else
	{
		if(pDM_DigTable->CurCCK_CCAThres!=CurCCK_CCAThres)		
		{
			AcquireCCKAndRWPageAControl(pAdapter);
//modify by Guo.Mingzhi 2011-12-29
			ODM_Write1Byte(pDM_Odm, 0xa0a, CurCCK_CCAThres);
			ReleaseCCKAndRWPageAControl(pAdapter);
		}
		pDM_DigTable->PreCCK_CCAThres = pDM_DigTable->CurCCK_CCAThres;
		pDM_DigTable->CurCCK_CCAThres = CurCCK_CCAThres;
	}
#endif
}

VOID
ODM_Write_CCK_CCA_DMSP(
	IN	PDM_ODM_T		pDM_Odm,
	IN	u1Byte			CurCCK_CCAThres
	)
{
#if (DM_ODM_SUPPORT_TYPE == ODM_MP)
	PADAPTER	pAdapter = pDM_Odm->Adapter;
	pDIG_T		pDM_DigTable = &pDM_Odm->DM_DigTable;
	PADAPTER	BuddyAdapter = pAdapter->BuddyAdapter;
	BOOLEAN		bGetValueFromOtherMac = dm_DualMacGetParameterFromBuddyAdapter(pAdapter);

		if(BuddyAdapter == NULL)
		{
			if(pAdapter->bSlaveOfDMSP)
			{
				pDM_DigTable->PreCCK_CCAThres = pDM_DigTable->CurCCK_CCAThres;
				pDM_DigTable->CurCCK_CCAThres = CurCCK_CCAThres;
			}
			else
			{	
				AcquireCCKAndRWPageAControl(pAdapter);
				ODM_Write1Byte(pDM_Odm, 0xa0a, CurCCK_CCAThres);
				ReleaseCCKAndRWPageAControl(pAdapter);
				pDM_DigTable->PreCCK_CCAThres = pDM_DigTable->CurCCK_CCAThres;
				pDM_DigTable->CurCCK_CCAThres = CurCCK_CCAThres;
			}
			return;
		}
	
		if(pAdapter->bSlaveOfDMSP)
		{
			BuddyAdapter->DualMacDMSPControl.bChangeCCKPDStateForAnotherMacOfDMSP = TRUE;
			BuddyAdapter->DualMacDMSPControl.CurCCKPDStateForAnotherMacOfDMSP = CurCCK_CCAThres;
		}
		else
		{
			if(!bGetValueFromOtherMac)
			{
				if(pDM_DigTable->CurCCK_CCAThres!=CurCCK_CCAThres)
				{
					AcquireCCKAndRWPageAControl(pAdapter);
					ODM_Write1Byte(pDM_Odm, 0xa0a, CurCCK_CCAThres);
					ReleaseCCKAndRWPageAControl(pAdapter);
				}
				pDM_DigTable->PreCCK_CCAThres = pDM_DigTable->CurCCK_CCAThres;
				pDM_DigTable->CurCCK_CCAThres = CurCCK_CCAThres;
			}
		}
#endif
}

VOID
odm_GetCCKFalseAlarm_92D(
	IN		PDM_ODM_T		pDM_Odm
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_MP|ODM_CE))
	PADAPTER		pAdapter	= pDM_Odm->Adapter;
	u4Byte ret_value;
	PFALSE_ALARM_STATISTICS FalseAlmCnt = &(pDM_Odm->FalseAlmCnt);
	
	if(*pDM_Odm->pBandType != ODM_BAND_5G)
	{
		AcquireCCKAndRWPageAControl(pAdapter);

		//hold cck counter
		ODM_SetBBReg(pDM_Odm, 0xa2c, BIT12, 1); 
		ODM_SetBBReg(pDM_Odm, 0xa2c, BIT14, 1); 
		
		ret_value = ODM_GetBBReg(pDM_Odm, 0xa5c, bMaskByte0);
		FalseAlmCnt->Cnt_Cck_fail = ret_value;

		ret_value = ODM_GetBBReg(pDM_Odm, 0xa58, bMaskByte3);
		FalseAlmCnt->Cnt_Cck_fail +=  (ret_value& 0xff)<<8;

		ret_value = ODM_GetBBReg(pDM_Odm, 0xa60, bMaskDWord);
		FalseAlmCnt->Cnt_CCK_CCA = ((ret_value&0xFF)<<8) |((ret_value&0xFF00)>>8);
		
		ReleaseCCKAndRWPageAControl(pAdapter);
	}
	else
	{
		FalseAlmCnt->Cnt_Cck_fail = 0;
		FalseAlmCnt->Cnt_CCK_CCA = 0;
	}
#endif	
}

VOID
odm_ResetFACounter_92D(
	IN		PDM_ODM_T		pDM_Odm
	)
{

#if (DM_ODM_SUPPORT_TYPE & ( ODM_MP| ODM_CE))
	PADAPTER		pAdapter	= pDM_Odm->Adapter;
	u1Byte	BBReset;

	//reset false alarm counter registers
	ODM_SetBBReg(pDM_Odm, 0xd00, BIT27, 1);
	ODM_SetBBReg(pDM_Odm, 0xd00, BIT27, 0);
	//update ofdm counter
	ODM_SetBBReg(pDM_Odm, 0xc00, BIT31, 0); //update page C counter
	ODM_SetBBReg(pDM_Odm, 0xd00, BIT31, 0); //update page D counter
	if(*pDM_Odm->pBandType != ODM_BAND_5G)
	{
		AcquireCCKAndRWPageAControl(pAdapter);
		//reset CCK CCA counter
		ODM_SetBBReg(pDM_Odm, 0xa2c, BIT13|BIT12, 0); 
		ODM_SetBBReg(pDM_Odm, 0xa2c, BIT13|BIT12, 2); 
		//reset CCK FA counter
		ODM_SetBBReg(pDM_Odm, 0xa2c, BIT15|BIT14, 0); 
		ODM_SetBBReg(pDM_Odm, 0xa2c, BIT15|BIT14, 2); 
		ReleaseCCKAndRWPageAControl(pAdapter);
	}

	//BB Reset
	if(*pDM_Odm->pMacPhyMode == ODM_DMSP)
	{
		if((*pDM_Odm->pBandType == ODM_BAND_2_4G) && *pDM_Odm->pbMasterOfDMSP && (!pDM_Odm->bLinked))
		{
			BBReset = ODM_Read1Byte(pDM_Odm, 0x02);
			ODM_Write1Byte(pDM_Odm, 0x02, BBReset&(~BIT0));
			ODM_Write1Byte(pDM_Odm, 0x02, BBReset|BIT0);
		}
	}
	else
	{
		if((*pDM_Odm->pBandType == ODM_BAND_2_4G) && (!pDM_Odm->bLinked))
		{
			BBReset = ODM_Read1Byte(pDM_Odm, 0x02);
			ODM_Write1Byte(pDM_Odm, 0x02, BBReset&(~BIT0));
			ODM_Write1Byte(pDM_Odm, 0x02, BBReset|BIT0);
		}
	}
#endif
}

VOID 
odm_FalseAlarmCounterStatistics_ForSlaveOfDMSP(
	IN		PDM_ODM_T		pDM_Odm
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_MP|ODM_CE))
	PFALSE_ALARM_STATISTICS FalseAlmCnt = &(pDM_Odm->FalseAlmCnt);
	PFALSE_ALARM_STATISTICS	FlaseAlmCntBuddyAdapter =  &(pDM_Odm->FlaseAlmCntBuddyAdapter);

	if(pDM_Odm->pBuddyAdapter == NULL)
		return;

	if (pDM_Odm->bDualMacSmartConcurrent == FALSE)
		return;

	//FlaseAlmCntBuddyAdapter = &(BuddyAdapter->FalseAlmCnt);
	
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
#endif
}

VOID 
ODM_DynamicEarlyMode(
	IN		PDM_ODM_T		pDM_Odm
	)
{
#if (RTL8192DE_SUPPORT ==1) 
		int i;

	if(!(pDM_Odm->SupportAbility&ODM_MAC_EARLY_MODE))	return;

	// IS_HARDWARE_TYPE_8192DE
	if(! ((pDM_Odm->SupportICType == ODM_RTL8192D)&&(pDM_Odm->SupportInterface ==ODM_ITRF_PCIE)))	return;
	
		
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_EARLY_MODE,ODM_DBG_LOUD,("pDM_Odm->pMacPhyMode=0x%x",*(pDM_Odm->pMacPhyMode)));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_EARLY_MODE,ODM_DBG_LOUD,("pDM_Odm->SupportInterface)=0x%x",pDM_Odm->SupportInterface));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_EARLY_MODE,ODM_DBG_LOUD,("pDM_Odm->SupportICType=0x%x",pDM_Odm->SupportICType));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_EARLY_MODE,ODM_DBG_LOUD,("pDM_Odm->SupportAbility=0x%x",pDM_Odm->SupportAbility));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_EARLY_MODE,ODM_DBG_LOUD, ("Early mode:RSSI_Min= 0x%x",pDM_Odm->RSSI_Min));
	
	for(i=0;i<ODM_ASSOCIATE_ENTRY_NUM;i++){
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_EARLY_MODE,ODM_DBG_LOUD,  ("Early mode  IOT Peer[%x]:= 0x%x  be Used:%x",i,pDM_Odm->pODM_StaInfo[i]->IOTPeer,IS_STA_VALID(pDM_Odm->pODM_StaInfo[i])?TRUE:FALSE));
		if(IS_STA_VALID(pDM_Odm->pODM_StaInfo[i])&&(pDM_Odm->pODM_StaInfo[i]->IOTPeer==HT_IOT_PEER_CISCO))
			break;			
     	}

	if(i==ODM_ASSOCIATE_ENTRY_NUM)
	{
		 if(!(ODM_Read1Byte(pDM_Odm, REG_EARLY_MODE_CONTROL)&0xF))
		{
			ODM_Write1Byte(pDM_Odm, REG_EARLY_MODE_CONTROL, 0x0f);
			ODM_RT_TRACE(pDM_Odm,	ODM_COMP_EARLY_MODE,ODM_DBG_LOUD, ("Early mode on"));
		}
		return;
	}


// need to be re-checked when comm_info is ok  by Mingzhi
//RT_TRACE to be re-checked. Just for CISCO AP
	if((*(pDM_Odm->pMacPhyMode)!=DUALMAC_DUALPHY))
	{	
		if(pDM_Odm->RSSI_Min< 50)
		{
			ODM_Write1Byte(pDM_Odm, REG_EARLY_MODE_CONTROL, 0x00);
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_EARLY_MODE,ODM_DBG_LOUD, ("Early mode off:RSSI_Min= 0x%x",	pDM_Odm->RSSI_Min));
		}
		else if(pDM_Odm->RSSI_Min> 55)
		{
			ODM_Write1Byte(pDM_Odm, REG_EARLY_MODE_CONTROL, 0x0f);
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_EARLY_MODE,ODM_DBG_LOUD, ("Early mode on:RSSI_Min= 0x%x",	pDM_Odm->RSSI_Min));
		}		
		
	}
#endif
}



#if (DM_ODM_SUPPORT_TYPE == ODM_MP)

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



//091212 chiyokolin
VOID
odm_TXPowerTrackingCallback_ThermalMeter_92D(
	IN PADAPTER	Adapter
	)
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


#endif

