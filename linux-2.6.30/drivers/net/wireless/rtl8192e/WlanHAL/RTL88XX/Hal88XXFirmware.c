/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal88XXFirmware.c
	
Abstract:
	Defined RTL88XX Firmware Related Function
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-04-11 Filen            Create.	
--*/
#ifndef __ECOS
#include "HalPrecomp.h"
#else
#include "../HalPrecomp.h"
#endif
//
//3 Download Firmware
//

static VOID
ReadFwHdr88XX(
    IN  HAL_PADAPTER    Adapter
)
{
#if 0
    PRTL88XX_FW_HDR pfw_hdr;
    pu1Byte     pFWStart;
    
    pfw_hdr = (PRTL88XX_FW_HDR)HALMalloc(Adapter, sizeof(RTL88XX_FW_HDR));
    if (NULL == pfw_hdr) {
        RT_TRACE(COMP_INIT, DBG_WARNING, ("ReadFwHdr88XX\n"));
        return;
    }

    GET_HAL_INTERFACE(Adapter)->GetHwRegHandler(Adapter, HW_VAR_FWFILE_START, (pu1Byte)&pFWStart);
    HAL_memcpy(pfw_hdr, pFWStart, RT_FIRMWARE_HDR_SIZE);
 
    //Register to HAL_DATA
    _GET_HAL_DATA(Adapter)->PFWHeader = pfw_hdr;
    RT_TRACE_F(COMP_INIT, DBG_TRACE ,("FW version = %x \n",pfw_hdr->version));
    RT_TRACE_F(COMP_INIT, DBG_TRACE ,("FW release at %x/%x \n",pfw_hdr->month,pfw_hdr->day));
    HAL_free(pfw_hdr);
#else
    PRTL88XX_FW_HDR pfw_hdr;
    pu1Byte     pFWStart;

    GET_HAL_INTERFACE(Adapter)->GetHwRegHandler(Adapter, HW_VAR_FWFILE_START, (pu1Byte)&pFWStart);
    //Register to HAL_DATA
    _GET_HAL_DATA(Adapter)->PFWHeader = (PVOID)pFWStart;
    pfw_hdr = (PRTL88XX_FW_HDR)pFWStart;
    
    RT_TRACE_F(COMP_INIT, DBG_TRACE ,("FW version = %x \n", pfw_hdr->version));
    RT_TRACE_F(COMP_INIT, DBG_TRACE ,("FW release at %x/%x \n", pfw_hdr->month, pfw_hdr->day));    
#endif
}

static VOID
WriteToFWSRAM88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  pu1Byte         pFWRealStart,
    IN  u4Byte          FWRealLen
)
{
    u4Byte      WriteAddr   = FW_DOWNLOAD_START_ADDRESS;
    u4Byte      CurPtr      = 0;
    u4Byte      Temp;
    
	while (CurPtr < FWRealLen) {
		if ((CurPtr+4) > FWRealLen) {
			// Reach the end of file.
			while (CurPtr < FWRealLen) {
				Temp = *(pFWRealStart + CurPtr);
				HAL_RTL_W8(WriteAddr, (u1Byte)Temp);
				WriteAddr++;
				CurPtr++;
			}
		} else {
			// Write FW content to memory.
			Temp = *((pu4Byte)(pFWRealStart + CurPtr));
			Temp = HAL_cpu_to_le32(Temp);
			HAL_RTL_W32(WriteAddr, Temp);
			WriteAddr += 4;

			if(WriteAddr == 0x2000) {
				u1Byte  tmp = HAL_RTL_R8(REG_8051FW_CTRL+2);
                
                //Switch to next page
				tmp += 1;
                //Reset Address
				WriteAddr = 0x1000;
                
				HAL_RTL_W8(REG_8051FW_CTRL+2, tmp);
			}
			CurPtr += 4;
		}
	}
}

static VOID
DownloadFWInit88XX(
    IN  HAL_PADAPTER    Adapter
)
{
   //Clear 0x80,0x81,0x82[0],0x82[1],0x82[2],0x82[3]
   HAL_RTL_W8(REG_8051FW_CTRL,0x0);
   HAL_RTL_W8(REG_8051FW_CTRL+1,0x0);

   // Enable MCU
   HAL_RTL_W8(REG_SYS_FUNC_EN+1, HAL_RTL_R8(REG_SYS_FUNC_EN+1) | BIT2);
   HAL_delay_ms(1);

   // Load SRAM
   HAL_RTL_W8(REG_8051FW_CTRL, HAL_RTL_R8(REG_8051FW_CTRL) | MCUFWDL_EN);
   HAL_delay_ms(1);

   //Clear ROM FPGA Related Parameter
   HAL_RTL_W32(REG_8051FW_CTRL, HAL_RTL_R32(REG_8051FW_CTRL) & 0xfff0ffff);
   delay_ms(1);
}

static BOOLEAN
VerifyDownloadStatus88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  pu1Byte         pFWRealStart,
    IN  u4Byte          FWRealLen
)
{
    u4Byte      WriteAddr   = FW_DOWNLOAD_START_ADDRESS;
    u4Byte      CurPtr      = 0;
    u4Byte      binTemp;
    u4Byte      ROMTemp;    
    u1Byte      u1ByteTmp;


    // first clear page number 

    u1ByteTmp =  HAL_RTL_R8(REG_8051FW_CTRL+2);
    u1ByteTmp &= ~BIT0;
    u1ByteTmp &= ~BIT1;
    u1ByteTmp &= ~BIT2;    
    HAL_RTL_W8(REG_8051FW_CTRL+2, u1ByteTmp);
	delay_ms(1);

    // then compare FW image and download content
    CurPtr = 0;
         
	while (CurPtr < FWRealLen) {
		if ((CurPtr+4) > FWRealLen) {
			// Reach the end of file.
			while (CurPtr < FWRealLen) {
                if(HAL_RTL_R8(WriteAddr)!=((u1Byte)*(pFWRealStart + CurPtr)))
    			{
        		    RT_TRACE_F(COMP_INIT, DBG_LOUD,("Verify download fail at [%x] \n",WriteAddr));
                    return _FALSE;
    			}
				WriteAddr++;
				CurPtr++;
			}
		} else {
			// Comapre Download code with original binary

            binTemp = *((pu4Byte)(pFWRealStart + CurPtr));

			binTemp = HAL_cpu_to_le32(binTemp);
			ROMTemp = HAL_RTL_R32(WriteAddr);

			if(binTemp != ROMTemp)
			{
    		   RT_TRACE_F(COMP_INIT, DBG_LOUD,("Verify download fail at [0x%x] binTemp=%x,ROMTemp=%x \n",
                    WriteAddr,binTemp,ROMTemp));
                return _FALSE;
			}
			WriteAddr += 4;

			if(WriteAddr == 0x2000) {
				u1Byte  tmp = HAL_RTL_R8(REG_8051FW_CTRL+2);
                
                //Switch to next page
				tmp += 1;
                //Reset Address
				WriteAddr = 0x1000;
                
				HAL_RTL_W8(REG_8051FW_CTRL+2, tmp);
			}
			CurPtr += 4;
		}
	}
    return _TRUE;
}

static BOOLEAN
LoadFirmware88XX(
    IN  HAL_PADAPTER    Adapter
)
{
    pu1Byte     pFWRealStart;
    u4Byte      FWRealLen;
    u1Byte      u1ByteTmp;
    u1Byte      wait_cnt = 0;

    
    RT_TRACE_F(COMP_INIT, DBG_LOUD, ("\n"));

    GET_HAL_INTERFACE(Adapter)->GetHwRegHandler(Adapter, HW_VAR_FWFILE_START, (pu1Byte)&pFWRealStart);
    pFWRealStart += RT_FIRMWARE_HDR_SIZE;

    GET_HAL_INTERFACE(Adapter)->GetHwRegHandler(Adapter, HW_VAR_FWFILE_SIZE, (pu1Byte)&FWRealLen);
    FWRealLen -= RT_FIRMWARE_HDR_SIZE;

    DownloadFWInit88XX(Adapter);

    WriteToFWSRAM88XX(Adapter, pFWRealStart, FWRealLen);

    u1ByteTmp = HAL_RTL_R8(REG_8051FW_CTRL);

    if ( u1ByteTmp & FWDL_CHKSUM_RPT ) {
        RT_TRACE_F(COMP_INIT, DBG_TRACE , ("CheckSum Pass\n"));
    }
    else {
        RT_TRACE_F(COMP_INIT, DBG_WARNING, ("CheckSum Failed\n"));
        return _FALSE;        
    }

#if CFG_FW_VERIFICATION    
    if(VerifyDownloadStatus88XX(Adapter,pFWRealStart,FWRealLen)==_TRUE)
    {
        RT_TRACE_F(COMP_INIT, DBG_TRACE,("download verify ok!\n"));
    }
    else
    {
        RT_TRACE_F(COMP_INIT, DBG_WARNING,("download verify fail!\n"));
        return _FALSE;
    }
#endif //#if CFG_FW_VERIFICATION

    // download and verify ok, clear download enable bit, and set MCU DL ready
    u1ByteTmp &= ~MCUFWDL_EN;
    u1ByteTmp |= MCUFWDL_RDY;
	HAL_RTL_W8(REG_8051FW_CTRL, u1ByteTmp);
	HAL_delay_ms(1);
    
    // reset MCU, 8051 will jump to RAM code    
	HAL_RTL_W8(REG_8051FW_CTRL+1, 0x00);
    HAL_RTL_W8(REG_SYS_FUNC_EN+1, HAL_RTL_R8(REG_SYS_FUNC_EN+1)&(~BIT2));
    HAL_delay_ms(1);    
    HAL_RTL_W8(REG_SYS_FUNC_EN+1, HAL_RTL_R8(REG_SYS_FUNC_EN+1) | BIT2);
  //  RT_TRACE_F(COMP_INIT, DBG_WARNING , ("After download RAM reset MCU\n"));

	// Check if firmware RAM Code is ready
    while (!(HAL_RTL_R8(REG_8051FW_CTRL) & WINTINI_RDY)) {
        if (++wait_cnt > CHECK_FW_RAMCODE_READY_TIMES) {        
            RT_TRACE_F(COMP_INIT, DBG_WARNING, ("RAMCode Failed\n"));
            return _FALSE;
		}
        
        RT_TRACE_F(COMP_INIT, DBG_WARNING, ("Firmware is not ready, wait\n"));
        HAL_delay_ms(CHECK_FW_RAMCODE_READY_DELAY_MS);
    }

    return _TRUE;
}


RT_STATUS
InitFirmware88XX(
    IN  HAL_PADAPTER    Adapter
)
{
    PHAL_DATA_TYPE      pHalData    = _GET_HAL_DATA(Adapter);    
    u4Byte              dwnRetry    = DOWNLOAD_FIRMWARE_RETRY_TIMES;
    BOOLEAN             bfwStatus   = _FALSE;

    ReadFwHdr88XX(Adapter);

    while(dwnRetry-- && !bfwStatus) {
        bfwStatus = LoadFirmware88XX(Adapter);
    }

    if ( _TRUE == bfwStatus ) {
        RT_TRACE_F(COMP_INIT, DBG_WARNING, ("LoadFirmware88XX is Successful\n"));
        pHalData->bFWReady = _TRUE;
    }
    else {
        RT_TRACE_F(COMP_INIT, DBG_WARNING, ("LoadFirmware88XX failed\n"));
        pHalData->bFWReady = _FALSE;
        return RT_STATUS_FAILURE;
    }
    
    return RT_STATUS_SUCCESS;
}

//
//3 H2C Command
//
#if 0
BOOLEAN
IsH2CBufOccupy88XX(
    IN  HAL_PADAPTER    Adapter
)
{
    PHAL_DATA_TYPE              pHalData = _GET_HAL_DATA(Adapter);
   
    if ( HAL_RTL_R8(REG_HMETFR) & BIT(pHalData->H2CBufPtr88XX) ) {
        return _TRUE;
    }
    else {
        RT_TRACE(COMP_DBG, DBG_WARNING, ("H2CBufOccupy(%d) !!\n", 
                                            pHalData->H2CBufPtr88XX) );
        return _FALSE;
    }
}


BOOLEAN
SigninH2C88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  PH2C_CONTENT    pH2CContent
)
{
    PHAL_DATA_TYPE      pHalData = _GET_HAL_DATA(Adapter);
    u4Byte              DelayCnt = H2CBUF_OCCUPY_DELAY_CNT;
    
    //Check if h2c cmd signin buffer is occupied
    while( _TRUE == IsH2CBufOccupy88XX(Adapter) ) {
       HAL_delay_us(H2CBUF_OCCUPY_DELAY_US);
       DelayCnt--;

       if ( 0 == DelayCnt ) {
            RT_TRACE(COMP_DBG, DBG_WARNING, ("H2CBufOccupy retry timeout\n") );
            return _FALSE;
       }
       else {
            //Continue to check H2C Buf
       }
       
    }

    //signin reg in order to fit hw requirement
    if ( pH2CContent->content & BIT7 ) {
        HAL_RTL_W16(REG_HMEBOX_E0_E1 + (pHalData->H2CBufPtr88XX*2), pH2CContent->ext_content);
    }

    HAL_RTL_W16(REG_HMEBOX_0 + (pHalData->H2CBufPtr88XX*4), pH2CContent->content);

	//printk("(smcc) sign in h2c %x\n", HMEBOX_0+(priv->pshare->fw_q_fifo_count*4));
    RT_TRACE(COMP_DBG, DBG_LOUD, ("sign in h2c(%d) 0x%x\n", 
                                        pHalData->H2CBufPtr88XX, 
                                        REG_HMEBOX_0 + (pHalData->H2CBufPtr88XX*4)) );

    //rollover ring buffer count
    if (++pHalData->H2CBufPtr88XX > 3) {
        pHalData->H2CBufPtr88XX = 0;
    }

    return _TRUE;
}

#else
BOOLEAN
CheckFwReadLastH2C88XX(
	IN  HAL_PADAPTER    Adapter,
	IN  u1Byte          BoxNum
)
{
	u1Byte      valHMETFR;
	BOOLEAN     Result = FALSE;
	
	valHMETFR = HAL_RTL_R8(REG_HMETFR);

	if(((valHMETFR>>BoxNum)&BIT0) == 0)
		Result = TRUE;
		
	return Result;
}


RT_STATUS
FillH2CCmd88XX(
	IN  HAL_PADAPTER    Adapter,
	IN	u1Byte 		    ElementID,
	IN	u4Byte 		    CmdLen,
	IN	pu1Byte		    pCmdBuffer
)
{
    PHAL_DATA_TYPE      pHalData = _GET_HAL_DATA(Adapter);
	u1Byte	            BoxNum;
	u2Byte	            BOXReg=0, BOXExtReg=0;
	BOOLEAN             bFwReadClear=FALSE;
	u1Byte	            BufIndex=0;
	u1Byte	            WaitH2cLimmit=0;
	u1Byte	            BoxContent[4], BoxExtContent[4];
	u1Byte	            idx=0;

    if(!pHalData->bFWReady) {
        RT_TRACE(COMP_DBG, DBG_WARNING, ("H2C bFWReady=False !!\n") );
        return RT_STATUS_FAILURE;
    }

	// 1. Find the last BOX number which has been writen.
	BoxNum = pHalData->H2CBufPtr88XX;	//pHalData->LastHMEBoxNum;
	switch(BoxNum)
	{
		case 0:
			BOXReg = REG_HMEBOX_0;
			BOXExtReg = REG_HMEBOX_E0_V1;
			break;
		case 1:
			BOXReg = REG_HMEBOX_1;
			BOXExtReg = REG_HMEBOX_E1_V1;
			break;
		case 2:
			BOXReg = REG_HMEBOX_2;
			BOXExtReg = REG_HMEBOX_E2_V1;
			break;
		case 3:
			BOXReg = REG_HMEBOX_3;
			BOXExtReg = REG_HMEBOX_E3_V1;
			break;
		default:
			break;
	}

	// 2. Check if the box content is empty.
	while(!bFwReadClear)
	{
		bFwReadClear = CheckFwReadLastH2C88XX(Adapter, BoxNum);
		if(WaitH2cLimmit++ >= 100)
		{
			return RT_STATUS_FAILURE;
		}
		else if(!bFwReadClear)
		{	
			HAL_delay_us(10); //us
		}
	}

	// 4. Fill the H2C cmd into box 	
	HAL_memset(BoxContent, 0, sizeof(BoxContent));
	HAL_memset(BoxExtContent, 0, sizeof(BoxExtContent));
	
	BoxContent[0] = ElementID; // Fill element ID
//	RTPRINT(FFW, FW_MSG_H2C_CONTENT, ("[FW], Write ElementID BOXReg(%4x) = %2x \n", BOXReg, ElementID));

	switch(CmdLen)
	{
	case 1:
	case 2:
	case 3:
	{
		//BoxContent[0] &= ~(BIT7);
		HAL_memcpy((pu1Byte)(BoxContent)+1, pCmdBuffer+BufIndex, CmdLen);
		//For Endian Free.
		for(idx= 0; idx < 4; idx++)
		{
			HAL_RTL_W8(BOXReg+idx, BoxContent[idx]);
		}
		break;
	}
	case 4: 
	case 5:
	case 6:
	case 7:
	{
		//BoxContent[0] |= (BIT7);
		HAL_memcpy((pu1Byte)(BoxExtContent), pCmdBuffer+BufIndex+3, (CmdLen-3));
		HAL_memcpy((pu1Byte)(BoxContent)+1, pCmdBuffer+BufIndex, 3);
		//For Endian Free.
		for(idx = 0 ; idx < 4 ; idx ++)
		{
			HAL_RTL_W8(BOXExtReg+idx, BoxExtContent[idx]);
		}		
		for(idx = 0 ; idx < 4 ; idx ++)
		{
			HAL_RTL_W8(BOXReg+idx, BoxContent[idx]);
		}
		break;
	}
	
	default:
//RTPRINT(FFW, FW_MSG_H2C_STATE, ("[FW], Invalid command len=%d!!!\n", CmdLen));
		return RT_STATUS_FAILURE;
	}

	if (++pHalData->H2CBufPtr88XX > 3)
		pHalData->H2CBufPtr88XX = 0;

//RTPRINT(FFW, FW_MSG_H2C_CONTENT, ("[FW], pHalData->LastHMEBoxNum = %d\n", pHalData->LastHMEBoxNum));
	return RT_STATUS_SUCCESS;
}
#endif

static VOID
SetBcnCtrlReg88XX(
	IN HAL_PADAPTER     Adapter,	
	IN	u1Byte		    SetBits,
	IN	u1Byte		    ClearBits
	)
{
	u1Byte tmp = HAL_RTL_R8(REG_BCN_CTRL);

	tmp |=  SetBits;
	tmp &= ~ClearBits;

	HAL_RTL_W8(REG_BCN_CTRL, tmp);
}

static u1Byte
MRateIdxToARFRId88XX(
	IN HAL_PADAPTER     Adapter,
	u1Byte  			RateIdx,    //RATR_TABLE_MODE
	u1Byte	    		RfType
)
{
	u1Byte Ret = 0;
	
	switch(RateIdx){

	case RATR_INX_WIRELESS_NGB:
		if(RfType == MIMO_1T1R)
			Ret = 1;
		else 
			Ret = 0;
		break;

	case RATR_INX_WIRELESS_N:
	case RATR_INX_WIRELESS_NG:
		if(RfType == MIMO_1T1R)
			Ret = 5;
		else
			Ret = 4;
		break;

	case RATR_INX_WIRELESS_NB:
		if(RfType == MIMO_1T1R)
			Ret = 3;
		else 
			Ret = 2;
		break;

	case RATR_INX_WIRELESS_GB:
		Ret = 6;
		break;

	case RATR_INX_WIRELESS_G:
		Ret = 7;
		break;	

	case RATR_INX_WIRELESS_B:
		Ret = 8;
		break;

	case RATR_INX_WIRELESS_MC:
		if (!(HAL_VAR_NETWORK_TYPE & WIRELESS_11A))
			Ret = 6;
		else
			Ret = 7;
		break;
#if CFG_HAL_RTK_AC_SUPPORT
	case RATR_INX_WIRELESS_AC_N:
		if(RfType == MIMO_1T1R)
			Ret = 10;
		else
			Ret = 9;
		break;
#endif

	default:
		Ret = 0;
		break;
	}	

	return Ret;
}

static u1Byte
Get_RA_BW88XX(
	BOOLEAN 	bCurTxBW80MHz, 
	BOOLEAN		bCurTxBW40MHz
)
{
	u1Byte	BW = HT_CHANNEL_WIDTH_20;
    
	if(bCurTxBW80MHz)
		BW = HT_CHANNEL_WIDTH_80;
	else if(bCurTxBW40MHz)
		BW = HT_CHANNEL_WIDTH_20_40;
	else
		BW = HT_CHANNEL_WIDTH_20;

	return BW;
}

static u1Byte
Get_VHT_ENI88XX(
	u4Byte			IOTAction,
	u1Byte			WirelessMode,
	u4Byte			ratr_bitmap 
	)
{
	u1Byte Ret = 0;

#ifdef CONFIG_WLAN_HAL_8192EE
	if (1) return 0;
#endif

	if(WirelessMode < WIRELESS_MODE_N_24G)
		Ret =  0;
	else if(WirelessMode == WIRELESS_MODE_N_24G || WirelessMode == WIRELESS_MODE_N_5G)
	{

//if(IOTAction == HT_IOT_VHT_HT_MIX_MODE)
		{
			if(ratr_bitmap & BIT20)	// Mix , 2SS
				Ret = 3;
			else 					// Mix, 1SS
				Ret = 2;
		}

	}
	else if(WirelessMode == WIRELESS_MODE_AC_5G)
		Ret = 1;						// VHT

	return (Ret << 4);
}

static BOOLEAN 
Get_RA_ShortGI88XX(	
	IN HAL_PADAPTER         Adapter,
	struct stat_info *		pEntry,
	IN	WIRELESS_MODE		WirelessMode,
	IN	u1Byte				ChnlBW
)
{	
	BOOLEAN	bShortGI;

	BOOLEAN	bShortGI20MHz = FALSE,bShortGI40MHz = FALSE, bShortGI80MHz = FALSE;
	
	if(	WirelessMode == WIRELESS_MODE_N_24G || 
		WirelessMode == WIRELESS_MODE_N_5G )
	{
		if (pEntry->ht_cap_buf.ht_cap_info & HAL_cpu_to_le16(_HTCAP_SHORTGI_40M_)
			&& Adapter->pmib->dot11nConfigEntry.dot11nShortGIfor40M) {
			bShortGI40MHz = TRUE;
		} else if (pEntry->ht_cap_buf.ht_cap_info & HAL_cpu_to_le16(_HTCAP_SHORTGI_20M_) &&
			Adapter->pmib->dot11nConfigEntry.dot11nShortGIfor20M) {
			bShortGI20MHz = TRUE;
		}
	}
#if CFG_HAL_RTK_AC_SUPPORT
	else if(WirelessMode == WIRELESS_MODE_AC_5G)
	{
		{
			if( HAL_cpu_to_le32(pEntry->vht_cap_buf.vht_cap_info) & BIT(SHORT_GI80M_E))
				bShortGI80MHz = TRUE;
		}
	}
#endif

	switch(ChnlBW){
		case HT_CHANNEL_WIDTH_20_40:
			bShortGI = bShortGI40MHz;
			break;
#if CFG_HAL_RTK_AC_SUPPORT            
		case HT_CHANNEL_WIDTH_80:
			bShortGI = bShortGI80MHz;
			break;
#endif
		default:case HT_CHANNEL_WIDTH_20:
			bShortGI = bShortGI20MHz;
			break;
	}		
	return bShortGI;
}

static u4Byte
  RateToBitmap_VHT88XX(
	pu1Byte			pVHTRate,
	u1Byte 		rf_mimo_mode
	
)
{

	u1Byte	i,j , tmpRate,rateMask;
	u4Byte	RateBitmap = 0;

    if(rf_mimo_mode == MIMO_1T1R)
		rateMask = 2;
	else
    	rateMask = 4;
    
	for(i = j= 0; i < rateMask; i+=2, j+=10)
	{
        // now support for 1ss and 2ss
		tmpRate = (pVHTRate[0] >> i) & 3;

		switch(tmpRate){
		case 2:
			RateBitmap = RateBitmap | (0x03ff << j);
			break;
		case 1:
			RateBitmap = RateBitmap | (0x01ff << j);
		break;

		case 0:
			RateBitmap = RateBitmap | (0x00ff << j);
		break;

		default:
			break;
		}
	}

	return RateBitmap;
}

u4Byte
Get_VHT_HT_Mix_Ratrbitmap(
	u4Byte					IOTAction,
	WIRELESS_MODE			WirelessMode,
	u4Byte					HT_ratr_bitmap,
	u4Byte					VHT_ratr_bitmap
	)
{
	u4Byte	ratr_bitmap = 0;
	if(WirelessMode == WIRELESS_MODE_N_24G || WirelessMode == WIRELESS_MODE_N_5G)
	{
/*	
		if(IOTAction == HT_IOT_VHT_HT_MIX_MODE)
			ratr_bitmap = HT_ratr_bitmap | BIT28 | BIT29;
		else
			ratr_bitmap =  HT_ratr_bitmap;
*/		
	}
	else
		ratr_bitmap =  VHT_ratr_bitmap;

	return ratr_bitmap;
}

VOID
UpdateHalRAMask88XX(
	IN HAL_PADAPTER         Adapter,
	HAL_PSTAINFO            pEntry,
	u1Byte                  rssi_level
	)
{

	u1Byte		WirelessMode= WIRELESS_MODE_A;
	u1Byte		BW = HT_CHANNEL_WIDTH_20;
	u2Byte		RateSet=0, i;
	u1Byte		MimoPs=MIMO_PS_NOLIMIT, ratr_index = 8, H2CCommand[7] ={ 0};
	u4Byte		EntryRateSet = 0xffffffff, ratr_bitmap = 0, IOTAction = 0;
	BOOLEAN		bShortGI=FALSE, bCurTxBW80MHz=FALSE, bCurTxBW40MHz=FALSE;
	struct 		stat_info *pstat = pEntry;
	u1Byte 		rf_mimo_mode = get_rf_mimo_mode(Adapter);

	if(pEntry == NULL)		
	{
		return;
	}
	{
		if(pEntry->MIMO_ps & _HT_MIMO_PS_STATIC_)
            MimoPs = MIMO_PS_STATIC;
		else if(pEntry->MIMO_ps & _HT_MIMO_PS_DYNAMIC_)
			MimoPs = MIMO_PS_DYNAMIC;
	
		add_RATid(Adapter, pEntry);		
		ratr_bitmap =  0xfffffff;

#if CFG_HAL_RTK_AC_SUPPORT
		if(pstat->vht_cap_len && (HAL_VAR_NETWORK_TYPE & WIRELESS_11AC)) {
			WirelessMode = WIRELESS_MODE_AC_5G;
			if(((HAL_le32_to_cpu(pstat->vht_cap_buf.vht_support_mcs[0])>>2)&3)==3)
				rf_mimo_mode = MIMO_1T1R;
		} 
        else 
#endif
        if ((HAL_VAR_NETWORK_TYPE & WIRELESS_11N) && pstat->ht_cap_len && (!should_restrict_Nrate(Adapter, pstat))) {
			if(HAL_VAR_NETWORK_TYPE & WIRELESS_11A)
				WirelessMode = WIRELESS_MODE_N_5G;
			else
				WirelessMode = WIRELESS_MODE_N_24G;
            
			if((pstat->tx_ra_bitmap & 0xff00000) == 0)
				rf_mimo_mode = MIMO_1T1R;
		}
		else if (((HAL_VAR_NETWORK_TYPE & WIRELESS_11G) && isErpSta(pstat))){
				WirelessMode = WIRELESS_MODE_G;		
		}
        else if ((HAL_VAR_NETWORK_TYPE & WIRELESS_11A) &&
				((HAL_OPMODE & WIFI_AP_STATE) || (Adapter->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G))) {
				WirelessMode = WIRELESS_MODE_A;		
		}
		else if(HAL_VAR_NETWORK_TYPE & WIRELESS_11B){
			WirelessMode = WIRELESS_MODE_B;		
		}

#if CFG_HAL_RTK_AC_SUPPORT
		if(WirelessMode == WIRELESS_MODE_AC_5G) {
			ratr_bitmap &= 0xfff;
			ratr_bitmap |= RateToBitmap_VHT88XX(&(pstat->vht_cap_buf.vht_support_mcs[0]),rf_mimo_mode) << 12;
//			ratr_bitmap &= 0x3FCFFFFF;
			if(rf_mimo_mode == MIMO_1T1R)
				ratr_bitmap &= 0x003fffff;
			else
				ratr_bitmap &= 0x3FCFFFFF;			// Test Chip...	2SS MCS7

			if(pstat->tx_bw==HT_CHANNEL_WIDTH_80)
				bCurTxBW80MHz = TRUE;
		}
#endif

		if (Adapter->pshare->is_40m_bw && (pstat->tx_bw == HT_CHANNEL_WIDTH_20_40))
			bCurTxBW40MHz = TRUE;
	}

	// assign band mask and rate bitmap
	switch (WirelessMode)
	{
		case WIRELESS_MODE_B:
		{
			ratr_index = RATR_INX_WIRELESS_B;
			if(ratr_bitmap & 0x0000000c)		//11M or 5.5M enable				
				ratr_bitmap &= 0x0000000d;
			else
				ratr_bitmap &= 0x0000000f;
		}
		break;

		case WIRELESS_MODE_G:
		{
			ratr_index = RATR_INX_WIRELESS_GB;
			
			if(rssi_level == 1)
				ratr_bitmap &= 0x00000f00;
			else if(rssi_level == 2)
				ratr_bitmap &= 0x00000ff0;
			else
				ratr_bitmap &= 0x00000ff5;
		}
		break;
			
		case WIRELESS_MODE_A:
		{
			ratr_index = RATR_INX_WIRELESS_G;
			ratr_bitmap &= 0x00000ff0;
		}
		break;
			
		case WIRELESS_MODE_N_24G:
		case WIRELESS_MODE_N_5G:
		{
			if(WirelessMode == WIRELESS_MODE_N_24G)
				ratr_index = RATR_INX_WIRELESS_NGB;
			else
				ratr_index = RATR_INX_WIRELESS_NG;

//			if(MimoPs <= MIMO_PS_DYNAMIC)
			if(MimoPs < MIMO_PS_DYNAMIC)
			{
				if(rssi_level == 1)
					ratr_bitmap &= 0x00070000;
				else if(rssi_level == 2)
					ratr_bitmap &= 0x0007f000;
				else
					ratr_bitmap &= 0x0007f005;
			}
			else
			{
				if (rf_mimo_mode == MIMO_1T1R)
				{
					if (bCurTxBW40MHz)
					{
						if(rssi_level == 1)
							ratr_bitmap &= 0x000f0000;
						else if(rssi_level == 2)
							ratr_bitmap &= 0x000ff000;
						else
							ratr_bitmap &= 0x000ff015;
					}
					else
					{
						if(rssi_level == 1)
							ratr_bitmap &= 0x000f0000;
						else if(rssi_level == 2)
							ratr_bitmap &= 0x000ff000;
						else
							ratr_bitmap &= 0x000ff005;
					}	
				}
				else
				{
					if (bCurTxBW40MHz)
					{
						if(rssi_level == 1)
							ratr_bitmap &= 0x0f8f0000;
						else if(rssi_level == 2) {
							//ratr_bitmap &= 0x0f8ff000;
							ratr_bitmap &= 0x0ffff000;
						}
						else {
							//ratr_bitmap &= 0x0f8ff015;
							ratr_bitmap &= 0x0ffff015;
						}
					}
					else
					{
						if(rssi_level == 1)
							ratr_bitmap &= 0x0f8f0000;
						else if(rssi_level == 2) {
							//ratr_bitmap &= 0x0f8ff000;
							ratr_bitmap &= 0x0ffff000;
						}
						else {
							//ratr_bitmap &= 0x0f8ff005;
							ratr_bitmap &= 0x0ffff005;
						}
					}
				}
			}
		}
		break;

#if CFG_HAL_RTK_AC_SUPPORT
		case WIRELESS_MODE_AC_5G:
		{
			ratr_index = RATR_INX_WIRELESS_AC_N;

			if (rf_mimo_mode == MIMO_1T1R)
				ratr_bitmap &= 0x003ff010;
			else
				ratr_bitmap &= 0xfffff010;
		}
		break;
#endif

		default:
			ratr_index = RATR_INX_WIRELESS_NGB;
			
			if(rf_mimo_mode == MIMO_1T1R)
				ratr_bitmap &= 0x000ff0ff;
			else
				ratr_bitmap &= 0x0f8ff0ff;
			break;
	}

	BW = Get_RA_BW88XX(bCurTxBW80MHz, bCurTxBW40MHz);
	bShortGI = Get_RA_ShortGI88XX(Adapter, pEntry, WirelessMode, BW);

	pstat->ratr_idx = MRateIdxToARFRId88XX(Adapter, ratr_index, rf_mimo_mode) ;

	H2CCommand[0] = (pstat->aid);
	H2CCommand[1] =  (pstat->ratr_idx)| (bShortGI?0x80:0x00) ;	
	H2CCommand[2] = BW |Get_VHT_ENI88XX(IOTAction, WirelessMode, ratr_bitmap);
	

	H2CCommand[2] |= BIT6;			// DisableTXPowerTraining

	H2CCommand[3] = (u1Byte)(ratr_bitmap & 0x000000ff);
	H2CCommand[4] = (u1Byte)((ratr_bitmap & 0x0000ff00) >>8);
	H2CCommand[5] = (u1Byte)((ratr_bitmap & 0x00ff0000) >> 16);
	H2CCommand[6] = (u1Byte)((ratr_bitmap & 0xff000000) >> 24);
	
	FillH2CCmd88XX(Adapter, H2C_88XX_RA_MASK, 7, H2CCommand);

	SetBcnCtrlReg88XX(Adapter, BIT3, 0);
/*
	panic_printk("UpdateHalRAMask88XX(): bitmap = %x ratr_index = %1x, MacID:%x, ShortGI:%x, MimoPs=%d\n", 
		ratr_bitmap, pstat->ratr_idx,  (pstat->aid), bShortGI, MimoPs);
	
	panic_printk("Cmd: %02x, %02x, %02x, %02x, %02x, %02x, %02x  \n",
		H2CCommand[0] ,H2CCommand[1], H2CCommand[2],
		H2CCommand[3] ,H2CCommand[4], H2CCommand[5], H2CCommand[6]		);
*/		
}

void
UpdateHalMSRRPT88XX(
	IN HAL_PADAPTER     Adapter,
	u2Byte              aid,
	u1Byte              opmode
	)
{
	u1Byte		H2CCommand[3] ={0};
	
	H2CCommand[0] = opmode & 0x01;
	H2CCommand[1] = aid & 0xff;
	H2CCommand[2] = 0;
	FillH2CCmd88XX(Adapter, H2C_88XX_MSRRPT, 3, H2CCommand);
	
//	panic_printk("UpdateHalMSRRPT88XX Cmd: %02x, %02x, %02x  \n",
//		H2CCommand[0] ,H2CCommand[1], H2CCommand[2]);
}



