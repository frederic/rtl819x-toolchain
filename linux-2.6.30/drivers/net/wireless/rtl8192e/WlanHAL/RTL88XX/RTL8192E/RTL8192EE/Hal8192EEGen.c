/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal88XXGen.c
	
Abstract:
	Defined RTL8192EE HAL Function
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-03-23 Filen            Create.	
--*/

#ifndef __ECOS
#include "HalPrecomp.h"
#else
#include "../../../HalPrecomp.h"
#endif

RT_STATUS
InitPON8192EE(
    IN  HAL_PADAPTER Adapter
)
{
    u32     bytetmp;
    u32     retry;
    
    RT_TRACE_F( COMP_INIT, DBG_LOUD, ("\n"));

	HAL_RTL_W8(REG_RSV_CTRL, 0x00);
    
	if (!HalPwrSeqCmdParsing88XX(Adapter, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK,
			PWR_INTF_PCI_MSK, rtl8192E_card_enable_flow))
    {
        RT_TRACE( COMP_INIT, DBG_SERIOUS, ("%s %d, HalPwrSeqCmdParsing init fail!!!\n", __FUNCTION__, __LINE__));
        return RT_STATUS_FAILURE;
    }

	// Power on when re-enter from IPS/Radio off/card disable
    HAL_RTL_W8(REG_AFE_XTAL_CTRL, HAL_RTL_R8(REG_AFE_XTAL_CTRL) | BIT0);

    bytetmp = HAL_RTL_R16(REG_SYS_PW_CTRL);
    bytetmp &= 0xE7ff;
    bytetmp |= 0x0800;
    HAL_RTL_W16(REG_SYS_PW_CTRL, bytetmp);

    while (!((bytetmp = HAL_RTL_R32(REG_SYS_PW_CTRL)) & 0x00020000));

    bytetmp = HAL_RTL_R16(REG_SYS_PW_CTRL);
    bytetmp &= 0x7FFF;
    HAL_RTL_W16(REG_SYS_PW_CTRL, bytetmp);

    bytetmp = HAL_RTL_R16(REG_SYS_PW_CTRL);
    bytetmp &= 0xE7ff;
    bytetmp |= 0x0000;
    HAL_RTL_W16(REG_SYS_PW_CTRL, bytetmp);

    HAL_delay_ms(1);

	// auto enable WLAN
	// Power On Reset for MAC Block
	bytetmp = HAL_RTL_R8(REG_SYS_PW_CTRL+1) | BIT(0);
	HAL_delay_us(2);
	HAL_RTL_W8(REG_SYS_PW_CTRL+1, bytetmp);
	HAL_delay_us(2);

	bytetmp = HAL_RTL_R8(REG_SYS_PW_CTRL+1);
	HAL_delay_us(2);
	retry = 0;
	while((bytetmp & BIT(0)) && retry < 1000){
		retry++;
		HAL_delay_us(50);
		bytetmp = HAL_RTL_R8(REG_SYS_PW_CTRL+1);
		HAL_delay_us(50);
	}
	
    RT_TRACE(COMP_INIT, DBG_WARNING, ("%s: RTL_R8(APS_FSMCO+1) retry times=%d\n", (char *)__FUNCTION__, retry) );

	{
		int val;
		val = HAL_RTL_R32(REG_SYS_PW_CTRL);
        RT_TRACE(COMP_INIT, DBG_TRACE, ("FSMCO11=0x%x\n", val) );
	}
	
	if (bytetmp & BIT(0)) {
        RT_TRACE(COMP_INIT, DBG_SERIOUS, ("%s ERROR: auto enable WLAN failed!!(0x%02X)\n", __FUNCTION__, bytetmp) );
	}	

    HAL_RTL_W16(REG_SYS_FUNC_EN, HAL_RTL_R16(REG_SYS_FUNC_EN) & ~FEN_CPUEN);
    
    HAL_delay_us(2);

    return  RT_STATUS_SUCCESS;    
}


RT_STATUS
StopHW8192EE(
    IN  HAL_PADAPTER Adapter
)
{
    // TODO:

    return RT_STATUS_SUCCESS;
}


RT_STATUS	
hal_Associate_8192EE(
    HAL_PADAPTER        Adapter,
    BOOLEAN                 IsDefaultAdapter
)
{
    PHAL_INTERFACE              pHalFunc = GET_HAL_INTERFACE(Adapter);
    PHAL_DATA_TYPE              pHalData = _GET_HAL_DATA(Adapter);

    //
    //Initialization Related
    //
    pHalData->AccessSwapCtrl        = HAL_ACCESS_SWAP_MEM;
    pHalFunc->InitPONHandler        = InitPON8192EE;
    pHalFunc->InitMACHandler        = InitMAC88XX;
    pHalFunc->InitFirmwareHandler   = InitFirmware88XX;
    pHalFunc->InitHCIDMAMemHandler  = InitHCIDMAMem88XX;
    pHalFunc->InitHCIDMARegHandler  = InitHCIDMAReg88XX;    
#if CFG_HAL_SUPPORT_MBSSID    
    pHalFunc->InitMBSSIDHandler     = InitMBSSID88XX;
#endif  //CFG_HAL_SUPPORT_MBSSID
    pHalFunc->InitVAPIMRHandler     = InitVAPIMR88XX;


    //
    //Stop Related
    //
#if CFG_HAL_SUPPORT_MBSSID        
    pHalFunc->StopMBSSIDHandler     = StopMBSSID88XX;
#endif  //CFG_HAL_SUPPORT_MBSSID
    pHalFunc->StopHWHandler         = StopHW88XX;
    pHalFunc->StopSWHandler         = StopSW88XX;
    pHalFunc->DisableVXDAPHandler   = DisableVXDAP88XX;


    //
    //ISR Related
    //
    pHalFunc->InitIMRHandler                    = InitIMR88XX;
    pHalFunc->EnableIMRHandler                  = EnableIMR88XX;
    pHalFunc->InterruptRecognizedHandler        = InterruptRecognized88XX;
    pHalFunc->GetInterruptHandler               = GetInterrupt88XX;
    pHalFunc->AddInterruptMaskHandler           = AddInterruptMask88XX;
    pHalFunc->RemoveInterruptMaskHandler        = RemoveInterruptMask88XX;
    pHalFunc->DisableRxRelatedInterruptHandler  = DisableRxRelatedInterrupt88XX;
    pHalFunc->EnableRxRelatedInterruptHandler   = EnableRxRelatedInterrupt88XX;


    //
    //Tx Related
    //
    pHalFunc->PrepareTXBDHandler            = PrepareTXBD88XX;    
    pHalFunc->FillTxHwCtrlHandler           = FillTxHwCtrl88XX;
    pHalFunc->SyncSWTXBDHostIdxToHWHandler  = SyncSWTXBDHostIdxToHW88XX;
    pHalFunc->TxPollingHandler              = TxPolling88XX;
    pHalFunc->SigninBeaconTXBDHandler       = SigninBeaconTXBD88XX;
    pHalFunc->GetTxQueueHWIdxHandler        = GetTxQueueHWIdx88XX;
    pHalFunc->MappingTxQueueHandler         = MappingTxQueue88XX;
    pHalFunc->QueryTxConditionMatchHandler  = QueryTxConditionMatch88XX;
#if CFG_HAL_TX_SHORTCUT
//    pHalFunc->GetShortCutTxDescHandler      = GetShortCutTxDesc88XX;
//    pHalFunc->ReleaseShortCutTxDescHandler  = ReleaseShortCutTxDesc88XX;
    pHalFunc->GetShortCutTxBuffSizeHandler  = GetShortCutTxBuffSize88XX;
    pHalFunc->SetShortCutTxBuffSizeHandler  = SetShortCutTxBuffSize88XX;
    pHalFunc->CopyShortCutTxDescHandler     = CopyShortCutTxDesc88XX;
    pHalFunc->FillShortCutTxHwCtrlHandler   = FillShortCutTxHwCtrl88XX;    
#endif // CFG_HAL_TX_SHORTCUT

    //
    //Rx Related
    //
    pHalFunc->PrepareRXBDHandler            = PrepareRXBD88XX;
    pHalFunc->QueryRxDescHandler            = QueryRxDesc88XX;
    pHalFunc->UpdateRXBDInfoHandler         = UpdateRXBDInfo88XX;
    pHalFunc->UpdateRXBDHWIdxHandler        = UpdateRXBDHWIdx88XX;
    pHalFunc->UpdateRXBDHostIdxHandler      = UpdateRXBDHostIdx88XX;    

    //
    // General operation
    //
    pHalFunc->GetChipIDMIMOHandler          =   GetChipIDMIMO88XX;
    pHalFunc->SetHwRegHandler               =   SetHwReg88XX;
    pHalFunc->GetHwRegHandler               =   GetHwReg88XX;


    //
    // Security Related     
    //
    pHalFunc->CAMReadMACConfigHandler       =   CAMReadMACConfig88XX;
    pHalFunc->CAMEmptyEntryHandler          =   CAMEmptyEntry88XX;
    pHalFunc->CAMFindUsableHandler          =   CAMFindUsable88XX;
    pHalFunc->CAMProgramEntryHandler        =   CAMProgramEntry88XX;


    //
    // PHY/RF Related
    //
    pHalFunc->PHYSetCCKTxPowerHandler       = PHYSetCCKTxPower8192E;
    pHalFunc->PHYSetOFDMTxPowerHandler      = PHYSetOFDMTxPower8192E;


    //
    // Firmware CMD IO related
    //
    pHalData->H2CBufPtr88XX     = 0;
    pHalData->bFWReady          = _FALSE;
    pHalFunc->FillH2CCmdHandler             = FillH2CCmd88XX;
    pHalFunc->UpdateHalRAMaskHandler        = UpdateHalRAMask88XX;
    pHalFunc->UpdateHalMSRRPTHandler        = UpdateHalMSRRPT88XX;
    pHalFunc->DumpRxBDescTestHandler = DumpRxBDesc88XX;
    pHalFunc->DumpTxBDescTestHandler = DumpTxBDesc88XX;
    
    return  RT_STATUS_SUCCESS;    
}


void 
InitMAC8192EE(
    IN  HAL_PADAPTER Adapter
)
{














    
}

