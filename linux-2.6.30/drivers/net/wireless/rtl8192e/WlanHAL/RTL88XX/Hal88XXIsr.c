/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal88XXIsr.c

Abstract:
	Defined RTL88XX HAL common Function

Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-04-30 Filen            Create.
--*/

#ifndef __ECOS
#include "HalPrecomp.h"
#else
#include "../HalPrecomp.h"
#endif

VOID
EnableIMR88XX(
    IN  HAL_PADAPTER    Adapter
)
{
    PHAL_DATA_TYPE              pHalData = _GET_HAL_DATA(Adapter);
    
    RT_TRACE_F(COMP_INIT, DBG_LOUD, ("\n"));

    HAL_RTL_W32(REG_HIMR0, pHalData->IntMask[0]);
    HAL_RTL_W32(REG_HIMR1, pHalData->IntMask[1]);
}


//
// Description:
//	Recognize the interrupt content by reading the interrupt register or content and masking interrupt mask (IMR)
//	if it is our NIC's interrupt. After recognizing, we may clear the all interrupts (ISR).
// Arguments:
//	[in] Adapter -
//		The adapter context.
//	[in] pContent -
//		Under PCI interface, this field is ignord.
//		Under USB interface, the content is the interrupt content pointer.
//		Under SDIO interface, this is the interrupt type which is Local interrupt or system interrupt.
//	[in] ContentLen -
//		The length in byte of pContent.
// Return:
//	If any interrupt matches the mask (IMR), return TRUE, and return FALSE otherwise.
//

HAL_IMEM
BOOLEAN
InterruptRecognized88XX(
    IN  HAL_PADAPTER        Adapter,
	IN	PVOID				pContent,
	IN	u4Byte				ContentLen
)
{
    PHAL_DATA_TYPE              pHalData = _GET_HAL_DATA(Adapter);

    pHalData->IntArray[0] = HAL_RTL_R32(REG_HISR0);
    pHalData->IntArray[0] &= pHalData->IntMask[0];
    HAL_RTL_W32(REG_HISR0, pHalData->IntArray[0]);

    pHalData->IntArray[1] = HAL_RTL_R32(REG_HISR1);
    pHalData->IntArray[1] &= pHalData->IntMask[1];
    HAL_RTL_W32(REG_HISR1, pHalData->IntArray[1]);

    return (pHalData->IntArray[0]!=0 || pHalData->IntArray[1]!=0);
}

//
// Description:
//	Check the interrupt content (read from previous process) in HAL.
// Arguments:
//	[in] pAdapter -
//		The adapter context pointer.
//	[in] intType -
//		The HAL interrupt type for querying.
// Return:
//	If the corresponding interrupt content (bit) is toggled, return TRUE.
//	If the input interrupt type isn't recognized or this corresponding
//	hal interupt isn't toggled, return FALSE.
// Note:
//	We don't perform I/O here to read interrupt such as ISR here, so the
//	interrupt content shall be read before this handler.
//
HAL_IMEM
BOOLEAN
GetInterrupt88XX(
	IN	HAL_PADAPTER	Adapter,
	IN	HAL_INT_TYPE	intType
	)
{
	HAL_DATA_TYPE	*pHalData   = _GET_HAL_DATA(Adapter);
	BOOLEAN			bResult     = FALSE;

	switch(intType)
	{
	default:
		// Unknown interrupt type, no need to alarm because this IC may not
		// support this interrupt.
		RT_TRACE_F(COMP_SYSTEM, DBG_WARNING, ("Unkown intType: %d!\n", intType));
		break;

	case HAL_INT_TYPE_ANY:
		bResult = (pHalData->IntArray[0] || pHalData->IntArray[1]) ? TRUE : FALSE;
		break;

	//4 // ========== DWORD 0 ==========
	case HAL_INT_TYPE_BCNDERR0:
		bResult = (pHalData->IntArray[0] & BIT_BCNDERR0) ? TRUE : FALSE;
		break;

	case HAL_INT_TYPE_TBDOK:
		bResult = (pHalData->IntArray[0] & BIT_TXBCNOK) ? TRUE : FALSE;
		break;
		
	case HAL_INT_TYPE_TBDER:
		bResult = (pHalData->IntArray[0] & BIT_TXBCNERR) ? TRUE : FALSE;
		break;
		
	case HAL_INT_TYPE_BcnInt:
		bResult = (pHalData->IntArray[0] & BIT_BCNDMAINT0) ? TRUE : FALSE;
		break;
		
	case HAL_INT_TYPE_PSTIMEOUT:
		bResult = (pHalData->IntArray[0] & BIT_PSTIMEOUT) ? TRUE : FALSE;
		break;

	case HAL_INT_TYPE_PSTIMEOUT1:
		bResult = (pHalData->IntArray[0] & BIT_TIMEOUT1) ? TRUE : FALSE;
		break;

	case HAL_INT_TYPE_PSTIMEOUT2:
		bResult = (pHalData->IntArray[0] & BIT_TIMEOUT2) ? TRUE : FALSE;
		break; 
		
	case HAL_INT_TYPE_C2HCMD:
		bResult = (pHalData->IntArray[0] & BIT_C2HCMD_INT) ? TRUE : FALSE;
		break;	
		
	case HAL_INT_TYPE_VIDOK:
		bResult = (pHalData->IntArray[0] & BIT_VIDOK) ? TRUE : FALSE;
		break;
		
	case HAL_INT_TYPE_VODOK:
		bResult = (pHalData->IntArray[0] & BIT_VODOK) ? TRUE : FALSE;
		break;
		
	case HAL_INT_TYPE_BEDOK:
		bResult = (pHalData->IntArray[0] & BIT_BEDOK) ? TRUE : FALSE;
		break;
		
	case HAL_INT_TYPE_BKDOK:
		bResult = (pHalData->IntArray[0] & BIT_BKDOK) ? TRUE : FALSE;
		break;

	case HAL_INT_TYPE_MGNTDOK:
		bResult = (pHalData->IntArray[0] & BIT_MGTDOK) ? TRUE : FALSE;
		break;

	case HAL_INT_TYPE_HIGHDOK:
		bResult = (pHalData->IntArray[0] & BIT_HIGHDOK) ? TRUE : FALSE;
		break;

    #if 0   //Filen: removed
	case HAL_INT_TYPE_BDOK:
		bResult = (pHalData->IntArray[0] & IMR_BCNDOK0_88E) ? TRUE : FALSE;
		break;
    #endif
		
	case HAL_INT_TYPE_CPWM:
		bResult = (pHalData->IntArray[0] & BIT_CPWM_INT) ? TRUE : FALSE;
		break;

	case HAL_INT_TYPE_TSF_BIT32_TOGGLE:
		bResult = (pHalData->IntArray[0] & BIT_TSF_BIT32_TOGGLE) ? TRUE : FALSE;
		break;

    case HAL_INT_TYPE_RX_OK:
		bResult = (pHalData->IntArray[0] & BIT_RXOK) ? TRUE : FALSE;
        break;
        
    case HAL_INT_TYPE_RDU:
		bResult = (pHalData->IntArray[0] & BIT_RDU) ? TRUE : FALSE;
        break;        

    case HAL_INT_TYPE_CTWEND:
		bResult = (pHalData->IntArray[0] & BIT_CTWEND) ? TRUE : FALSE;        
        break;

	//4 // ========== DWORD 1 ==========
	case HAL_INT_TYPE_RXFOVW:
		bResult = (pHalData->IntArray[1] & BIT_FOVW) ? TRUE : FALSE;
		break;

    case HAL_INT_TYPE_TXFOVW:
        bResult = (pHalData->IntArray[1] & BIT_TXFOVW) ? TRUE : FALSE;
		break;

    case HAL_INT_TYPE_RXERR:
        bResult = (pHalData->IntArray[1] & BIT_RXERR_INT) ? TRUE : FALSE;
		break;        
        
    case HAL_INT_TYPE_TXERR:
        bResult = (pHalData->IntArray[1] & BIT_TXERR_INT) ? TRUE : FALSE;
		break;        

    case HAL_INT_TYPE_BcnInt_MBSSID:
        bResult = ((pHalData->IntArray[1] & (BIT_BCNDMAINT1|BIT_BCNDMAINT2|BIT_BCNDMAINT3|BIT_BCNDMAINT4|\
                                    BIT_BCNDMAINT5|BIT_BCNDMAINT6|BIT_BCNDMAINT7)) ||
                    (pHalData->IntArray[0] & BIT_BCNDMAINT0)
                    ) ? TRUE : FALSE;
        break;

    case HAL_INT_TYPE_BcnInt1:
		bResult = (pHalData->IntArray[1] & BIT_BCNDMAINT1) ? TRUE : FALSE;              
        break;

    case HAL_INT_TYPE_BcnInt2:
		bResult = (pHalData->IntArray[1] & BIT_BCNDMAINT2) ? TRUE : FALSE;
        break;

    case HAL_INT_TYPE_BcnInt3:
		bResult = (pHalData->IntArray[1] & BIT_BCNDMAINT3) ? TRUE : FALSE;              
        break;

    case HAL_INT_TYPE_BcnInt4:
		bResult = (pHalData->IntArray[1] & BIT_BCNDMAINT4) ? TRUE : FALSE;
        break;

    case HAL_INT_TYPE_BcnInt5:
		bResult = (pHalData->IntArray[1] & BIT_BCNDMAINT5) ? TRUE : FALSE;              
        break;

    case HAL_INT_TYPE_BcnInt6:
		bResult = (pHalData->IntArray[1] & BIT_BCNDMAINT6) ? TRUE : FALSE;
        break;

    case HAL_INT_TYPE_BcnInt7:
		bResult = (pHalData->IntArray[1] & BIT_BCNDMAINT7) ? TRUE : FALSE;
        break;

	}

	return bResult;
}


// TODO: Pedro, we can set several IMR combination for different scenario. Ex: 1) AP, 2) Client, 3) ....
// TODO: this can avoid to check non-necessary interrupt in __wlan_interrupt(..)...
VOID
AddInterruptMask88XX(
	IN	HAL_PADAPTER	Adapter,
	IN	HAL_INT_TYPE	intType
	)
{
	HAL_DATA_TYPE	*pHalData   = _GET_HAL_DATA(Adapter);

	switch(intType)
	{
    	default:
    		// Unknown interrupt type, no need to alarm because this IC may not
    		// support this interrupt.
    		RT_TRACE_F(COMP_SYSTEM, DBG_WARNING, ("Unkown intType: %d!\n", intType));
    		break;

    	case HAL_INT_TYPE_ANY:
            pHalData->IntMask[0] = 0xFFFFFFFF;
            pHalData->IntMask[1] = 0xFFFFFFFF;
    		break;

    	//4 // ========== DWORD 0 ==========
    	case HAL_INT_TYPE_TBDOK:
    		pHalData->IntMask[0] |= BIT_TXBCNOK;
    		break;
    		
    	case HAL_INT_TYPE_TBDER:
            pHalData->IntMask[0] |= BIT_TXBCNERR;
    		break;
    		
    	case HAL_INT_TYPE_BcnInt:
    		pHalData->IntMask[0] |= BIT_BCNDMAINT0;
    		break;
    		
    	case HAL_INT_TYPE_PSTIMEOUT:
            pHalData->IntMask[0] |= BIT_PSTIMEOUT;
    		break;

    	case HAL_INT_TYPE_PSTIMEOUT1:
    		pHalData->IntMask[0] |= BIT_TIMEOUT1;
    		break;

    	case HAL_INT_TYPE_PSTIMEOUT2:
    		pHalData->IntMask[0] |= BIT_TIMEOUT2;
    		break; 
    		
    	case HAL_INT_TYPE_C2HCMD:
    		pHalData->IntMask[0] |= BIT_C2HCMD_INT;
    		break;	
    		
    	case HAL_INT_TYPE_VIDOK:
    		pHalData->IntMask[0] |= BIT_VIDOK;
    		break;
    		
    	case HAL_INT_TYPE_VODOK:
    		pHalData->IntMask[0] |= BIT_VODOK;
    		break;
    		
    	case HAL_INT_TYPE_BEDOK:
    		pHalData->IntMask[0] |= BIT_BEDOK;
    		break;
    		
    	case HAL_INT_TYPE_BKDOK:
    		pHalData->IntMask[0] |= BIT_BKDOK;
    		break;

    	case HAL_INT_TYPE_MGNTDOK:
    		pHalData->IntMask[0] |= BIT_MGTDOK;
    		break;

    	case HAL_INT_TYPE_HIGHDOK:
    		pHalData->IntMask[0] |= BIT_HIGHDOK;
    		break;

        #if 0   //Filen: removed
    	case HAL_INT_TYPE_BDOK:
            pHalData->IntMask[0] |= IMR_BCNDOK0_88E;
    		break;
        #endif
    		
    	case HAL_INT_TYPE_CPWM:
    		pHalData->IntMask[0] |= BIT_CPWM_INT;
    		break;

    	case HAL_INT_TYPE_TSF_BIT32_TOGGLE:
    		pHalData->IntMask[0] |= BIT_TSF_BIT32_TOGGLE;
    		break;

        case HAL_INT_TYPE_RX_OK:
    		pHalData->IntMask[0] |= BIT_RXOK;
            break;

        case HAL_INT_TYPE_RDU:
    		pHalData->IntMask[0] |= BIT_RDU;
            break;

    	//4 // ========== DWORD 1 ==========
    	case HAL_INT_TYPE_RXFOVW:
    		pHalData->IntMask[1] |= BIT_FOVW;
    		break;

        case HAL_INT_TYPE_TXFOVW:
            pHalData->IntMask[1] |= BIT_TXFOVW;
            break;

        case HAL_INT_TYPE_RXERR:
            pHalData->IntMask[1] |= BIT_RXERR_INT;
            break;
            
        case HAL_INT_TYPE_TXERR:
            pHalData->IntMask[1] |= BIT_TXERR_INT;
            break;

	}
}


VOID
RemoveInterruptMask88XX(
	IN	HAL_PADAPTER	Adapter,
	IN	HAL_INT_TYPE	intType
	)
{
	HAL_DATA_TYPE	*pHalData   = _GET_HAL_DATA(Adapter);

	switch(intType)
	{
    	default:
    		// Unknown interrupt type, no need to alarm because this IC may not
    		// support this interrupt.
    		RT_TRACE_F(COMP_SYSTEM, DBG_WARNING, ("Unkown intType: %d!\n", intType));
    		break;

    	case HAL_INT_TYPE_ANY:
            pHalData->IntMask[0] &= ~0xFFFFFFFF;
            pHalData->IntMask[1] &= ~0xFFFFFFFF;
    		break;

    	//4 // ========== DWORD 0 ==========
    	case HAL_INT_TYPE_TBDOK:
    		pHalData->IntMask[0] &= ~BIT_TXBCNOK;
    		break;
    		
    	case HAL_INT_TYPE_TBDER:
            pHalData->IntMask[0] &= ~BIT_TXBCNERR;
    		break;
    		
    	case HAL_INT_TYPE_BcnInt:
    		pHalData->IntMask[0] &= ~BIT_BCNDMAINT0;
    		break;
    		
    	case HAL_INT_TYPE_PSTIMEOUT:
            pHalData->IntMask[0] &= ~BIT_PSTIMEOUT;
    		break;

    	case HAL_INT_TYPE_PSTIMEOUT1:
    		pHalData->IntMask[0] &= ~BIT_TIMEOUT1;
    		break;

    	case HAL_INT_TYPE_PSTIMEOUT2:
    		pHalData->IntMask[0] &= ~BIT_TIMEOUT2;
    		break; 
    		
    	case HAL_INT_TYPE_C2HCMD:
    		pHalData->IntMask[0] &= ~BIT_C2HCMD_INT;
    		break;	
    		
    	case HAL_INT_TYPE_VIDOK:
    		pHalData->IntMask[0] &= ~BIT_VIDOK;
    		break;
    		
    	case HAL_INT_TYPE_VODOK:
    		pHalData->IntMask[0] &= ~BIT_VODOK;
    		break;
    		
    	case HAL_INT_TYPE_BEDOK:
    		pHalData->IntMask[0] &= ~BIT_BEDOK;
    		break;
    		
    	case HAL_INT_TYPE_BKDOK:
    		pHalData->IntMask[0] &= ~BIT_BKDOK;
    		break;

    	case HAL_INT_TYPE_MGNTDOK:
    		pHalData->IntMask[0] &= ~BIT_MGTDOK;
    		break;

    	case HAL_INT_TYPE_HIGHDOK:
    		pHalData->IntMask[0] &= ~BIT_HIGHDOK;
    		break;

        #if 0   //Filen: removed
    	case HAL_INT_TYPE_BDOK:
            pHalData->IntMask[0] &= ~IMR_BCNDOK0_88E;
    		break;
        #endif
    		
    	case HAL_INT_TYPE_CPWM:
    		pHalData->IntMask[0] &= ~BIT_CPWM_INT;
    		break;

    	case HAL_INT_TYPE_TSF_BIT32_TOGGLE:
    		pHalData->IntMask[0] &= ~BIT_TSF_BIT32_TOGGLE;
    		break;

        case HAL_INT_TYPE_RX_OK:
    		pHalData->IntMask[0] &= ~BIT_RXOK;
            break;

        case HAL_INT_TYPE_RDU:
    		pHalData->IntMask[0] &= ~BIT_RDU;
            break;            

    	//4 // ========== DWORD 1 ==========
    	case HAL_INT_TYPE_RXFOVW:
    		pHalData->IntMask[1] &= ~BIT_FOVW;
    		break;

        case HAL_INT_TYPE_TXFOVW:
            pHalData->IntMask[1] &= ~BIT_TXFOVW;
            break;

        case HAL_INT_TYPE_RXERR:
            pHalData->IntMask[1] &= ~BIT_RXERR_INT;
            break;
            
        case HAL_INT_TYPE_TXERR:
            pHalData->IntMask[1] &= ~BIT_TXERR_INT;
            break;
	}
}


HAL_IMEM
VOID
DisableRxRelatedInterrupt88XX(
	IN	HAL_PADAPTER	Adapter
    )
{
    PHAL_DATA_TYPE      pHalData = _GET_HAL_DATA(Adapter);
    HAL_PADAPTER        priv     = Adapter;
    ULONG               flags;

#if 0
    HAL_SAVE_INT_AND_CLI(flags);

    pHalData->IntMask_RxINTBackup[0] = pHalData->IntMask[0];
    pHalData->IntMask_RxINTBackup[1] = pHalData->IntMask[1];

    pHalData->IntMask[0] &= ~BIT_RXOK;
    pHalData->IntMask[1] &= ~BIT_FOVW;

    HAL_RESTORE_INT(flags);

    HAL_RTL_W32(REG_HIMR0, pHalData->IntMask[0]);
    HAL_RTL_W32(REG_HIMR1, pHalData->IntMask[1]);
#else
    HAL_RTL_W32(REG_HIMR0, pHalData->IntMask[0] & ~BIT_RXOK);
    HAL_RTL_W32(REG_HIMR1, pHalData->IntMask[1] & ~BIT_FOVW);
#endif

}

HAL_IMEM
VOID
EnableRxRelatedInterrupt88XX(
	IN	HAL_PADAPTER	Adapter
    )
{
    PHAL_DATA_TYPE      pHalData    = _GET_HAL_DATA(Adapter);
    HAL_PADAPTER        priv        = Adapter;
    ULONG               flags;

#if 0
    HAL_SAVE_INT_AND_CLI(flags);

    pHalData->IntMask[0] = pHalData->IntMask_RxINTBackup[0];
    pHalData->IntMask[1] = pHalData->IntMask_RxINTBackup[1];

    HAL_RESTORE_INT(flags);
#endif
    HAL_RTL_W32(REG_HIMR0, pHalData->IntMask[0]);
    HAL_RTL_W32(REG_HIMR1, pHalData->IntMask[1]);

}


