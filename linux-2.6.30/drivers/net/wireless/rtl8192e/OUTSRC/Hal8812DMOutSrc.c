//============================================================
// File Name: Hal8812DMOutSrc.c 
//
// Description:
//
// This file is for 8812 outsource dynamic mechanism for partner.
//
//
//============================================================

//============================================================
// include files
//============================================================
#include "Mp_Precomp.h"

//============================================================
// Function predefine.
//============================================================

u1Byte    PSD_Report_RXHP[80];   // Add By Gary
u1Byte    PSD_func_flag;               // Add By Gary

VOID 
odm_FalseAlarmCounterStatistics_8812(
	IN	PADAPTER	Adapter
	)
{
	u4Byte ret_value;
	PFALSE_ALARM_STATISTICS FalseAlmCnt = &(Adapter->FalseAlmCnt);
	//pDIG_T	pDM_DigTable = &Adapter->DM_DigTable;
	//HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	//PMGNT_INFO	pMgntInfo = &(Adapter->MgntInfo);
	//u1Byte	BBReset;

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

	FalseAlmCnt->Cnt_Ofdm_fail = 	FalseAlmCnt->Cnt_Parity_Fail + FalseAlmCnt->Cnt_Rate_Illegal +
								FalseAlmCnt->Cnt_Crc8_fail + FalseAlmCnt->Cnt_Mcs_fail +
								FalseAlmCnt->Cnt_Fast_Fsync + FalseAlmCnt->Cnt_SB_Search_fail;
		
	ret_value = PHY_QueryBBReg(Adapter, rCCK0_FACounterLower, bMaskByte0);
	FalseAlmCnt->Cnt_Cck_fail = ret_value;

	ret_value = PHY_QueryBBReg(Adapter, rCCK0_FACounterUpper, bMaskByte3);
	FalseAlmCnt->Cnt_Cck_fail +=  (ret_value& 0xff)<<8;

#if 1	
	FalseAlmCnt->Cnt_all = (	FalseAlmCnt->Cnt_Fast_Fsync + 
						FalseAlmCnt->Cnt_SB_Search_fail +
						FalseAlmCnt->Cnt_Parity_Fail +
						FalseAlmCnt->Cnt_Rate_Illegal +
						FalseAlmCnt->Cnt_Crc8_fail +
						FalseAlmCnt->Cnt_Mcs_fail +
						FalseAlmCnt->Cnt_Cck_fail);	
#endif

	
	//reset false alarm counter registers
	//Page C counter
	PHY_SetBBReg(Adapter, rOFDM0_TRSWIsolation, BIT31, 1);
	PHY_SetBBReg(Adapter, rOFDM0_TRSWIsolation, BIT31, 0);
	//Page D counter
	PHY_SetBBReg(Adapter, rOFDM1_LSTF, 0x08000000, 1);
	PHY_SetBBReg(Adapter, rOFDM1_LSTF, 0x08000000, 0);
	//update ofdm counter
	PHY_SetBBReg(Adapter, rOFDM0_LSTF, BIT31, 0); //update page C counter
	PHY_SetBBReg(Adapter, rOFDM1_LSTF, BIT31, 0); //update page D counter
	
	//reset cck counter
	//	RT_TRACE(COMP_INIT,DBG_LOUD,("Acquiere mutex in dm_falsealarmcount 111 \n"));
	PHY_SetBBReg(Adapter, rCCK0_FalseAlarmReport, 0x0000c000, 0);
	//enable cck counter
	PHY_SetBBReg(Adapter, rCCK0_FalseAlarmReport, 0x0000c000, 2);
	//	RT_TRACE(COMP_INIT,DBG_LOUD,("Release mutex in dm_falsealarmcount 111 \n"));

	
/*
	//BB Reset
	if(!pMgntInfo->bMediaConnect)
	{
		BBReset = PlatformEFIORead1Byte(Adapter, 0x02);
		PlatformEFIOWrite1Byte(Adapter, 0x02, BBReset&(~BIT0));
		PlatformEFIOWrite1Byte(Adapter, 0x02, BBReset|BIT0);
	}
*/	
}

void odm_CCK_PacketDetectionThresh_8812(
	IN	PADAPTER	pAdapter)
{
	pDIG_T	pDM_DigTable = &pAdapter->DM_DigTable;

	if(pDM_DigTable->CurSTAConnectState == DIG_STA_CONNECT)
	{
		pDM_DigTable->Rssi_val_min = odm_initial_gain_MinPWDB(pAdapter);
		if(pDM_DigTable->PreCCKPDState == CCK_PD_STAGE_LowRssi)
		{
			if(pDM_DigTable->Rssi_val_min <= 25)
				pDM_DigTable->CurCCKPDState = CCK_PD_STAGE_LowRssi;
			else
				pDM_DigTable->CurCCKPDState = CCK_PD_STAGE_HighRssi;
		}
		else{
			if(pDM_DigTable->Rssi_val_min <= 20)
				pDM_DigTable->CurCCKPDState = CCK_PD_STAGE_LowRssi;
			else
				pDM_DigTable->CurCCKPDState = CCK_PD_STAGE_HighRssi;
		}
	}
	else
		pDM_DigTable->CurCCKPDState=CCK_PD_STAGE_MAX;
	
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
}


void odm_DIG_8812(
	IN	PADAPTER	pAdapter)
{
	PMGNT_INFO	pMgntInfo = &(pAdapter->MgntInfo);
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	pDIG_T			pDM_DigTable = &pAdapter->DM_DigTable;
	PFALSE_ALARM_STATISTICS FalseAlmCnt = &(pAdapter->FalseAlmCnt);
	static u1Byte	DIG_Dynamic_MIN_0 = 0x25;
	u1Byte	DIG_Dynamic_MIN;
	static BOOLEAN	bMediaConnect_0 = FALSE;
	BOOLEAN			FirstConnect;
#if DBG
	u1Byte TxRate = PlatformEFIORead1Byte(pAdapter, REG_INIDATA_RATE_SEL);
#endif
        //------- For AGC RX High Power Mode ---------
       //--------Add by Gary 2011-08-24---------
	u1Byte              	i, j, sum;    
       s1Byte              	Intf_diff_idx, MIN_Intf_diff_idx = 16;   
       s4Byte              	cur_channel;    
       u1Byte              	ch_map_intf_5M[16] = {0};     
       u4Byte              	FA_TH=FA_RXHP_TH1;
	static u1Byte      	psd_intf_flag = 0;
	static s4Byte      	curRssi = 0;                
       static s4Byte  		preRssi = 0;                                                      
       static u1Byte  		RX_HP_flag = 0;           
       u1Byte			RX_HP_enable = (u1Byte)(PHY_QueryBBReg(pAdapter, rOFDM0_XAAGCCore2, bMaskDWord)>>31);   
       //-------- End for RX High Power Mode ---------
       
if (RX_HP_flag == 0)   // Normal DIG mode, if RX_HP_flag=1 --> Don't adjust IGI!!!
{
	RTPRINT(FDM, DM_Monitor, ("odm_DIG_8812() ==>\n"));
	RT_TRACE(COMP_DIG, DBG_LOUD, ("odm_DIG_8812() ==>\n"));
	
	
	DIG_Dynamic_MIN = DIG_Dynamic_MIN_0;
	FirstConnect = (pMgntInfo->bMediaConnect) && (bMediaConnect_0 == FALSE);
	RT_TRACE(	COMP_DIG, DBG_LOUD, ("bMediaConnect=%d,  pMgntInfo->bMediaConnect=%d\n", 
		bMediaConnect_0, pMgntInfo->bMediaConnect));	
	

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

	RTPRINT(FDM, DM_Monitor, ("odm_DIG_8812() progress \n"));

	//odm_CtrlInitGainByTwoPort(pAdapter);
	//RTPRINT(FDM, DM_Monitor, ("odm_DIG_8812() <==\n"));
	
	//1 Boundary Decision
	if(pMgntInfo->bMediaConnect)
	{
		//2 Get minimum RSSI value among associated devices
		pDM_DigTable->Rssi_val_min = odm_initial_gain_MinPWDB(pAdapter);
		RT_TRACE(COMP_DIG, DBG_LOUD, ("pDM_DigTable->Rssi_val_min = 0x%x\n", pDM_DigTable->Rssi_val_min));

		//2 Modify DIG upper bound
		if((pDM_DigTable->Rssi_val_min + 20) > DM_DIG_MAX )
			pDM_DigTable->rx_gain_range_max = DM_DIG_MAX;
		else
			pDM_DigTable->rx_gain_range_max = pDM_DigTable->Rssi_val_min + 20;
		//2 Modify DIG lower bound
		if((FalseAlmCnt->Cnt_all > 500)&&(DIG_Dynamic_MIN < 0x25))
			DIG_Dynamic_MIN++;
		else if(((pAdapter->FalseAlmCnt.Cnt_all < 500)||(pDM_DigTable->Rssi_val_min < 8))&&(DIG_Dynamic_MIN > DM_DIG_MIN))
			DIG_Dynamic_MIN--;
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
				if((pDM_DigTable->ForbiddenIGI -1) < DIG_Dynamic_MIN) //DM_DIG_MIN)
				{
					pDM_DigTable->ForbiddenIGI = DIG_Dynamic_MIN; //DM_DIG_MIN;
					pDM_DigTable->rx_gain_range_min = DIG_Dynamic_MIN; //DM_DIG_MIN;
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

			if(FalseAlmCnt->Cnt_all > DM_DIG_FA_TH2)
				pDM_DigTable->CurIGValue = pDM_DigTable->PreIGValue+2;
			else if (FalseAlmCnt->Cnt_all > DM_DIG_FA_TH1)
				pDM_DigTable->CurIGValue = pDM_DigTable->PreIGValue+1;
			else if(FalseAlmCnt->Cnt_all < DM_DIG_FA_TH0)
				pDM_DigTable->CurIGValue =pDM_DigTable->PreIGValue-1;	
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
	
	

	DM_Write_DIG(pAdapter);
	bMediaConnect_0 = pMgntInfo->bMediaConnect;
	DIG_Dynamic_MIN_0 = DIG_Dynamic_MIN;
	RTPRINT(FDM, DM_Monitor, ("odm_DIG_8812() <==\n"));
	RT_TRACE(COMP_DIG, DBG_LOUD, ("odm_DIG_8812() <==\n"));
}

 //1 AGC RX High Power Mode By PSD
//--- AGC RX High Power only be implemented for client mode ---
//------------Add by Gary 2011-08-24 ----------------
if ((pMgntInfo->mAssoc) && (!pMgntInfo->mIbss) && (!ACTING_AS_AP(pAdapter)))
{     
       curRssi = pHalData->UndecoratedSmoothedPWDB;
       cur_channel = PHY_QueryRFReg(pAdapter, RF_PATH_A, 0x18, bRFRegOffsetMask) & 0x0f;
    if (PSD_func_flag == 1)      //  when PSD function update the PSD report, it needs to do the following work
    {
	    //for (i = 0 ; i < 80 ; i++)
	    //{
           //}
           // Separate 80M bandwidth into 16 group with smaller 5M BW.
           for (i = 0 ; i < 16 ; i++)
           {
			sum = 0;
                for(j = 0; j < 5 ; j++)
                	sum+=PSD_Report_RXHP[5*i + j];
            
                if(sum < 5)
                {
                	ch_map_intf_5M[i] = 1;  // interference flag
                }
           }
	
           // Mask target channel 5M index
	    for(i = 0; i < 4 ; i++)
           {
           		ch_map_intf_5M[cur_channel - 1 + i] = 0;  
           }
           psd_intf_flag = 0;
	    for(i = 0; i < 16; i++)
           {
                if(ch_map_intf_5M[i] == 1)
	               psd_intf_flag = 1;            // interference is detected!!!		
	    }

	    //for(i = 0; i< 16; i++)
           //{
	    //}

           for(i = 0; i < 16; i++)
          {
                 if(ch_map_intf_5M[i] == 1)
                {
                      Intf_diff_idx = ((cur_channel-(i+1))>0) ? (s1Byte)(cur_channel-(i-2)) : (s1Byte)((i+1)-cur_channel);  
                      if(Intf_diff_idx < MIN_Intf_diff_idx)
                           MIN_Intf_diff_idx = Intf_diff_idx;    // the difference index between interference and target
		  }
	    }

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

    }
    //1 Decide when Entering AGC RX High Power Mode
    if (RX_HP_enable == 0)      // AGC RX High Power Mode  ON/OFF   default is ON!!!
    {
         if ((psd_intf_flag == 1) && ( FalseAlmCnt->Cnt_all > FA_TH) && (curRssi > 70) &&                          // AGC RX High Power Conditions
	 	(pDM_DigTable->CurIGValue == pDM_DigTable->rx_gain_range_max) && (RX_HP_flag == 0))
         {
              if (curRssi > 80)
              {
                   PHY_SetBBReg1Byte(pAdapter, rOFDM0_XAAGCCore1, bMaskByte0, LNA_Low_Gain_1);
	            PHY_SetBBReg1Byte(pAdapter, rOFDM0_XBAGCCore1, bMaskByte0, LNA_Low_Gain_1);			  
		}
              else if ( curRssi > 72) 
              {
                   PHY_SetBBReg1Byte(pAdapter, rOFDM0_XAAGCCore1, bMaskByte0, LNA_Low_Gain_2);
	            PHY_SetBBReg1Byte(pAdapter, rOFDM0_XBAGCCore1, bMaskByte0, LNA_Low_Gain_2);	
              }
              else
              {
                   PHY_SetBBReg1Byte(pAdapter, rOFDM0_XAAGCCore1, bMaskByte0, LNA_Low_Gain_3);
	            PHY_SetBBReg1Byte(pAdapter, rOFDM0_XBAGCCore1, bMaskByte0, LNA_Low_Gain_3);	     
	       }
              PHY_SetBBReg1Byte(pAdapter, rOFDM0_XAAGCCore2, BIT8|BIT9, 0x3); 
	       RX_HP_flag = 1;    // conditions are OK!	
         }
         //3 Monitor RSSI variation to choose the suitable IGI or Exit AGC RX High Power Mode
	  if(RX_HP_flag == 1)
         {
              if ((curRssi > 80)&&(preRssi < 80))
              { 
                   PHY_SetBBReg1Byte(pAdapter, rOFDM0_XAAGCCore1, bMaskByte0, LNA_Low_Gain_1);
	            PHY_SetBBReg1Byte(pAdapter, rOFDM0_XBAGCCore1, bMaskByte0, LNA_Low_Gain_1);		    
              }
              else if ((curRssi < 80)&&(preRssi > 80))
              {
                   PHY_SetBBReg1Byte(pAdapter, rOFDM0_XAAGCCore1, bMaskByte0, LNA_Low_Gain_2);
	            PHY_SetBBReg1Byte(pAdapter, rOFDM0_XBAGCCore1, bMaskByte0, LNA_Low_Gain_2);	    
		}
	       else if ((curRssi > 72)&&(preRssi < 72))
	       {
                   PHY_SetBBReg1Byte(pAdapter, rOFDM0_XAAGCCore1, bMaskByte0, LNA_Low_Gain_2);
	            PHY_SetBBReg1Byte(pAdapter, rOFDM0_XBAGCCore1, bMaskByte0, LNA_Low_Gain_2);	
	       }
              else if ((curRssi < 72)&&( preRssi > 72))
	       {
                   PHY_SetBBReg1Byte(pAdapter, rOFDM0_XAAGCCore1, bMaskByte0, LNA_Low_Gain_3);
	            PHY_SetBBReg1Byte(pAdapter, rOFDM0_XBAGCCore1, bMaskByte0, LNA_Low_Gain_3);	
	       }
	       else if (curRssi < 68)//||(psd_intf_flag=0))       //RSSI is NOT large enough!!==> Exit RX HP Mode
	       {
                   PHY_SetBBReg1Byte(pAdapter, rOFDM0_XAAGCCore2, BIT8|BIT9, 0x0);
	            RX_HP_flag = 0;    // Exit RX High Power Mode flag		  
	       }
	   }
	   preRssi = curRssi;   
      }
}

}

//============================================================

//3============================================================
//3 Tx Power Tracking
//3============================================================

//091212 chiyokolin
VOID
odm_TXPowerTrackingCallback_ThermalMeter_8812(
            IN PADAPTER	Adapter)
{
}

//3============================================================
//3 BB Power Save
//3============================================================

void 
odm_1R_CCA_8812(
	IN	PADAPTER	pAdapter
	)
{
	HAL_DATA_TYPE*	pHalData = GET_HAL_DATA(pAdapter);
	pPS_T	pDM_PSTable = &pAdapter->DM_PSTable;

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


//============================================================
//3 PSD Monitor
//============================================================


int
GetPSDData_8812(
	PADAPTER	Adapter,
	unsigned int 	point,
	u1Byte initial_gain_psd)
{
	//unsigned int	val, rfval;
	int	psd_report;
       u2Byte psd_report_in;    //Add by Gary
	//HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	//Debug Message
	//val = PHY_QueryBBReg(Adapter,0x908, bMaskDWord);
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
	psd_report_in = (u2Byte)(PHY_QueryBBReg(Adapter,0x8B4, bMaskDWord) & 0x0000FFFF);    // changed!!! psd_report -> psd_report_in

#if(DEV_BUS_TYPE == RT_PCI_INTERFACE &&  RT_PLATFORM == PLATFORM_WINDOWS)
	psd_report = (int) (20*log10((double)psd_report_in))+(int)(initial_gain_psd-0x1c);   // changed!!! psd_report -> psd_report_in
#else
	psd_report = (int) (ConvertTo_dB(psd_report_in))+(int)(initial_gain_psd-0x1c);
#endif                               

	return psd_report;
	
}



VOID
PatchDCTone_8812(
	PADAPTER	Adapter, 
	ps4Byte		PSD_report,
	u1Byte 		initial_gain_psd
)
{
	int	psd_report;

	PHY_SetRFReg(Adapter, RF_PATH_A, 0x18, 0x3FF, 11);
	//Ch9 DC tone patch
	psd_report = GetPSDData_8812(Adapter, 96, initial_gain_psd);
	PSD_report[50] = psd_report;
	//Ch13 DC tone patch
	psd_report = GetPSDData_8812(Adapter, 32, initial_gain_psd);
	PSD_report[70] = psd_report;
	
	PHY_SetRFReg(Adapter, RF_PATH_A, 0x18, 0x3FF, 3);
	//Ch1 DC tone patch
	psd_report = GetPSDData_8812(Adapter, 96, initial_gain_psd);
	PSD_report[10] = psd_report;
	//Ch5 DC tone patch
	psd_report = GetPSDData_8812(Adapter, 32, initial_gain_psd);
	PSD_report[30] = psd_report;
	

}


VOID
GoodChannelDecision_8812(
	PADAPTER	Adapter, 
	ps4Byte		PSD_report,
	pu1Byte		PSD_bitmap,
	u1Byte 		SSBT,
	pu1Byte		PSD_bitmap_memory,
	pu1Byte         PSD_Report_RXHP )
{
       s4Byte	TH1 = SSBT -0x15;
	s4Byte	TH2 = PSD_TH2;
//	s4Byte	RegB34;
	u1Byte	bitmap, Smooth_size[3], Smooth_TH[3];
	//u1Byte	psd_bit;
	u4Byte	i,n,j, byte_idx, bit_idx, good_cnt, good_cnt_smoothing, Smooth_Interval[3];
	int 		start_byte_idx,start_bit_idx,cur_byte_idx, cur_bit_idx,NOW_byte_idx ;

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
	for (i = 0; i< 10; i++)
		PSD_bitmap[i] = 0;

       // Add By Gary
       for (i=0; i<80; i++)
	   	PSD_Report_RXHP[i] = 0;
	// End

	
	while (good_cnt < PSD_CHMIN)
	{
	
		good_cnt = 0;

              if(IS_HARDWARE_TYPE_8192C(Adapter))     // Add by Gary
              {
                   if(TH1==(SSBT+0x07))
                          break;    
                   if((TH1-3) > (SSBT+0x07))
			     TH1 -=3;
		     else
			     TH1 = SSBT+0x07;	
		}
			  
              //DbgPrint("PSD:TH1 = %d\n", TH1);

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

	RT_TRACE(	COMP_PSD, DBG_LOUD,("PSD: before smoothing\n"));
	for(n=0;n<10;n++)
	{
		//DbgPrint("PSD_bitmap[%u]=%x\n", n, PSD_bitmap[n]);
		for (i = 0; i<8; i++)
			RT_TRACE(	COMP_PSD, DBG_LOUD,("PSD_bitmap[%u] =   %d\n", 2402+n*8+i, (PSD_bitmap[n]&BIT(i))>>i));
	}

	
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
                                   PSD_Report_RXHP[8*n+i] = 1;
				}                                                  // ------end by Gary
			}
		}

	}

       /*for(i=0 ; i<80 ; i++)   // Gary Just for Debug
       {
             DbgPrint("Gary PSD : PSD_Report_RXHP[%d] = %d\n", i, PSD_Report_RXHP[i]);
	}*/

	
	good_cnt = 0;
	for ( i = 0; i < 10; i++)
	{
		for (n = 0; n < 8; n++)
			if((PSD_bitmap[i]& BIT(n)) != 0)
				good_cnt++;
	}
	RT_TRACE(COMP_PSD, DBG_LOUD,("PSD: good channel cnt = %u",good_cnt));
       }

	RT_TRACE(COMP_PSD, DBG_LOUD,("PSD: SSBT=%d, TH2=%d, TH1=%d",SSBT,TH2,TH1));
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
odm_PSD_Monitor_8812(
	PADAPTER	Adapter
)
{
       unsigned int 		pts, start_point, stop_point, initial_gain ;
	static u1Byte		PSD_bitmap_memory[80], init_memory = 0;
	static u1Byte 		psd_cnt=0;
	static s4Byte		PSD_report[80], PSD_report_tmp;
	//u1Byte                  PSD_Report_RXHP[80];  // Add By Gary
	static u8Byte		lastTxOkCnt=0, lastRxOkCnt=0;
	u1Byte 			H2C_PSD_DATA[5]={0,0,0,0,0};
	u1Byte			idx[20]={96,99,102,106,109,112,115,118,122,125,
					0,3,6,10,13,16,19,22,26,29};
	u1Byte			n, i, channel, BBReset,tone_idx;
	u1Byte			PSD_bitmap[10], SSBT=0,initial_gain_psd=0, RSSI_BT=0, initialGainUpper;
	s4Byte    			PSD_skip_start, PSD_skip_stop;
	u4Byte			CurrentChannel, RXIQI, RxIdleLowPwr, wlan_channel;
	u4Byte			ReScan, Interval, Is40MHz;
	u8Byte			curTxOkCnt, curRxOkCnt;
	int 				cur_byte_idx, cur_bit_idx;
	PMGNT_INFO		pMgntInfo = &(Adapter->MgntInfo);
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
       //------------ Add by Gary for fix RSSI_psd setting ------------
       s4Byte                 psd_result = 0;
	u1Byte                 RSSI_BT_new = (u1Byte) PHY_QueryBBReg(Adapter, 0xB9C, 0xFF);
       u1Byte                 rssi_ctrl = (u1Byte) PHY_QueryBBReg(Adapter, 0xB38, 0xFF);
       //-------------------------------------------------------
	
	if(pMgntInfo->bScanInProgress)
		return;

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
	if (IS_HARDWARE_TYPE_8192C(Adapter))       // Add by Gary
           RSSI_BT = RSSI_BT_new;
	
	
	RT_TRACE(COMP_PSD, DBG_LOUD,("PSD: RSSI_BT= %d\n", RSSI_BT));
	if(RSSI_BT<0x17)
		RSSI_BT +=3;
	RT_TRACE(COMP_PSD, DBG_LOUD,("PSD: RSSI_BT= %d\n", RSSI_BT));
	
	if(IS_HARDWARE_TYPE_8192C(Adapter))       // Add by Gary
       {
	     if(rssi_ctrl==1)
	         initial_gain_psd = RSSI_BT_new ; //gary
            else
	         initial_gain_psd = (u1Byte)pHalData->UndecoratedSmoothedPWDB; 
       } 

	  
	if(pHalData->bUserAssignLevel)
	{
		pHalData->bUserAssignLevel = FALSE;
		initialGainUpper = 0x7f;
	}
	else
	{
		initialGainUpper = 0x54;
	}
	if (initial_gain_psd < 0x1a)
		initial_gain_psd = 0x1a;
	if (initial_gain_psd > initialGainUpper)
	 	initial_gain_psd = initialGainUpper;
  
	if(IS_HARDWARE_TYPE_8192C(Adapter))    // Add by Gary
	       SSBT = initial_gain_psd;

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
       if((curRxOkCnt+curTxOkCnt) > 1000)
	{
		PSD_skip_start = (wlan_channel-1)*5 -Is40MHz*10;
		PSD_skip_stop = PSD_skip_start + (1+Is40MHz)*20;
		
	}   
	RT_TRACE(COMP_PSD,DBG_LOUD,("PSD: Skip tone from %d to %d \n", PSD_skip_start, PSD_skip_stop));

 	for (n=0;n<80;n++)
 	{
 		if((n%20)==0)
 		{
			channel = (n/20)*4 + 1;
			PHY_SetRFReg(Adapter, RF_PATH_A, 0x18, 0x3FF, channel);
		}
		tone_idx = n%20;
		if ((n>=PSD_skip_start) && (n<PSD_skip_stop))
		{	
			PSD_report[n] = SSBT;
			RT_TRACE(COMP_PSD,DBG_LOUD,("PSD:Tone %d skipped \n", n));
		}
		else
		{
			PSD_report_tmp =  GetPSDData_8812(Adapter, idx[tone_idx], initial_gain_psd);

			if ( PSD_report_tmp > PSD_report[n])
				PSD_report[n] = PSD_report_tmp;
				
		}
	}

	PatchDCTone_8812(Adapter, PSD_report, initial_gain_psd);
      
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
	PHY_SetRFReg(Adapter, RF_PATH_A, 0x18, bRFRegOffsetMask, CurrentChannel);
	//Turn on CCA
	PHY_SetBBReg(Adapter, 0xC14, bMaskDWord, RXIQI);
	//Restore RX idle low power
	if(RxIdleLowPwr == TRUE)
		PHY_SetBBReg(Adapter, 0x818, BIT28, 1);
	
	psd_cnt++;
	RT_TRACE(COMP_PSD, DBG_LOUD,("PSD:psd_cnt = %d \n",psd_cnt));
	if (psd_cnt < ReScan)
		PlatformSetTimer( Adapter, &pHalData->PSDTimer, Interval); //ms
	else
	{
		psd_cnt = 0;
		for(i=0;i<80;i++)		 
		{
	           psd_result = PSD_report[i];    
	           RT_TRACE(COMP_PSD, DBG_LOUD,("psd_report[%d]=     %d \n", 2402+i, psd_result));
                  //DbgPrint("psd_report[%d]=     %d \n", 2402+i, psd_result);    // Add By Gary
		}

		GoodChannelDecision_8812(Adapter, PSD_report, PSD_bitmap,SSBT, PSD_bitmap_memory, PSD_Report_RXHP);

	cur_byte_idx=0;
	cur_bit_idx=0;

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
	
		FillH2CCmd88E(Adapter, H2C_88E_PSD_RESULT, 5, H2C_PSD_DATA);
		RT_TRACE(	COMP_PSD, DBG_LOUD,("Leave dm_PSD_Monitor\n"));
	}
}

VOID
odm_PSDMonitor_8812(
	IN	PADAPTER	Adapter
	)
{
}




//3============================================================
//3 Export Interface
//3============================================================


VOID
ODM_DMWatchdog_8812(
	IN	PADAPTER	Adapter
	)
{
	//------------- Add By Gary for RX HP using ----------------
       static u1Byte         psd_counter=1, psd_counter_nxt=1;
       static u8Byte  		lastTxOkCnt=0, lastRxOkCnt=0;  // Add by gary
       u8Byte			curTxOkCnt, curRxOkCnt;

       psd_counter = psd_counter_nxt;
	curTxOkCnt = Adapter->TxStats.NumTxBytesUnicast - lastTxOkCnt;
	curRxOkCnt = Adapter->RxStats.NumRxBytesUnicast - lastRxOkCnt;
	lastTxOkCnt = Adapter->TxStats.NumTxBytesUnicast;
	lastRxOkCnt = Adapter->RxStats.NumRxBytesUnicast;	
       //---------------------------------------------2011.08.24

	//
	// Dynamic Initial Gain mechanism.
	//
	//odm_RSSIMonitorCheck(Adapter);
	odm_FalseAlarmCounterStatistics_8812(Adapter);

#if 0
#if (DEV_BUS_TYPE == RT_PCI_INTERFACE)|(DEV_BUS_TYPE == RT_USB_INTERFACE)
	if(psd_counter==1)       // every 4 sec do PSD monitor once
	{    
		if((DEV_BUS_TYPE == RT_PCI_INTERFACE) || ((DEV_BUS_TYPE == RT_USB_INTERFACE)&&((curTxOkCnt+curRxOkCnt)<1000)))
	     	{   
			odm_PSDMonitor_8812(Adapter);           // Add by Gary
                       	PSD_func_flag = 1;
                  	}
			psd_counter_nxt--;			  
       }
       else
       {
             	psd_counter_nxt++;
                  	PSD_func_flag = 0;
	}
#endif
#endif
	odm_DIG_8812(Adapter);
	odm_CCK_PacketDetectionThresh_8812(Adapter);
#if 0
	//
	//Dynamic BB Power Saving Mechanism
	//
	odm_DynamicBBPowerSaving(Adapter);

	//
	// Dynamic Tx Power mechanism.
	//
	odm_DynamicTxPower (Adapter);

	//
	//
	// Rate Adaptive by Rx Signal Strength mechanism.
	//
	odm_RefreshRateAdaptiveMask(Adapter);
#endif
	// EDCA turbo
	odm_CheckEdcaTurbo(Adapter);
#if 0

	//
	// Software Antenna diversity
	//
	//When traffic load is high, skip once SW antenna switch
	odm_SwAntDivChkAntSwitch(Adapter, SWAW_STEP_PEAK);
#endif
}
