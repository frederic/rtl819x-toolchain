/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal88XXGen.c

Abstract:
	Defined RTL88XX HAL common Function

Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-03-23 Filen            Create.
--*/

#ifndef __ECOS
#include "HalPrecomp.h"
#else
#include "../HalPrecomp.h"
#endif
MIMO_TR_STATUS
GetChipIDMIMO88XX(
    IN  HAL_PADAPTER Adapter
)
{
    u4Byte      value32;

    value32 = PlatformEFIORead4Byte(Adapter, REG_SYS_CFG1);

    if ( value32 & BIT27 )
    {
        return MIMO_2T2R;
    }
    else
    {
        return MIMO_1T1R;
    }
}


VOID
CAMEmptyEntry88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  u1Byte          index
)
{
    u4Byte  command=0, content=0;
    u4Byte  i;

    for (i = 0; i < HAL_CAM_CONTENT_COUNT; i++)
    {
        // polling bit, and No Write enable, and address
        command = HAL_CAM_CONTENT_COUNT*index+i;
        command = command | SECCAM_POLL |SECCAM_WE;
        // write content 0 is equal to mark invalid
        HAL_RTL_W32(REG_CAMWRITE, content);
        HAL_RTL_W32(REG_CAMCMD, command);
    }
}

u4Byte
CAMFindUsable88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          for_begin
)
{
    u4Byte command = 0, content = 0;
    u4Byte index;

    for (index=for_begin; index<HAL_TOTAL_CAM_ENTRY; index++)
    {
        // polling bit, and No Write enable, and address
        command = HAL_CAM_CONTENT_COUNT*index;
        HAL_RTL_W32(REG_CAMCMD, (SECCAM_POLL|command));

        // Check polling bit is clear
        while (1)
        {
            command = HAL_RTL_R32(REG_CAMCMD);
            if(command & SECCAM_POLL)
                continue;
            else
                break;
        }
        content = HAL_RTL_R32(REG_CAMREAD);

        // check valid bit. if not valid,
        if ((content & BIT15) == 0)
        {
            return index;
        }
    }

    return HAL_TOTAL_CAM_ENTRY;
}


VOID
CAMReadMACConfig88XX
(
    IN  HAL_PADAPTER    Adapter,
    IN  u1Byte          index,
    OUT pu1Byte         pMacad,
    OUT PCAM_ENTRY_CFG  pCfg
)
{
    u4Byte  command = 0, content = 0;
    u2Byte  TempConfig;

    // polling bit, and No Write enable, and address
    // cam address...
    // first 32-bit
    command = HAL_CAM_CONTENT_COUNT*index+0;
    command = command | SECCAM_POLL;
    HAL_RTL_W32(REG_CAMCMD, command);

    //Check polling bit is clear
    while (1)
    {
        command = HAL_RTL_R32(REG_CAMCMD);
        if(command & SECCAM_POLL)
            continue;
        else
            break;
    }

    content = HAL_RTL_R32(REG_CAMREAD);

    //first 32-bit is MAC address and CFG field
    *(pMacad+0) = (u1Byte)((content>>16)&0x000000FF);
    *(pMacad+1) = (u1Byte)((content>>24)&0x000000FF);

    TempConfig      = (u2Byte)(content&0x0000FFFF);
    pCfg->bValid    = (TempConfig & BIT15)? _TRUE : _FALSE;
    pCfg->KeyID     = TempConfig & 0x3;
    pCfg->EncAlgo   = (TempConfig & 0x1c)>>2;

    command = HAL_CAM_CONTENT_COUNT*index+1;
    command = command | SECCAM_POLL;
    HAL_RTL_W32(REG_CAMCMD, command);

    //Check polling bit is clear
    while (1)
    {
        command = HAL_RTL_R32(REG_CAMCMD);
        if(command & SECCAM_POLL)
            continue;
        else
            break;
    }
    content = HAL_RTL_R32(REG_CAMREAD);

    *(pMacad+5) = (u1Byte)((content>>24)&0x000000FF);
    *(pMacad+4) = (u1Byte)((content>>16)&0x000000FF);
    *(pMacad+3) = (u1Byte)((content>>8)&0x000000FF);
    *(pMacad+2) = (u1Byte)((content)&0x000000FF);
}


VOID
CAMProgramEntry88XX(
    IN	HAL_PADAPTER		Adapter,
    IN  u1Byte              index,
    IN  pu1Byte             macad,
    IN  pu1Byte             key128,
    IN  u2Byte              config
)
{
    u4Byte  target_command = 0, target_content = 0;
    u1Byte  entry_i = 0;

    for (entry_i=0; entry_i<HAL_CAM_CONTENT_USABLE_COUNT; entry_i++)
    {
        // polling bit, and write enable, and address
        target_command = entry_i + HAL_CAM_CONTENT_COUNT*index;
        target_command = target_command |SECCAM_POLL | SECCAM_WE;
        if (entry_i == 0)
        {
            //first 32-bit is MAC address and CFG field
            target_content = (u4Byte)(*(macad+0))<<16
                             |(u4Byte)(*(macad+1))<<24
                             |(u4Byte)config;
            target_content = target_content|config;
        }
        else if (entry_i == 1)
        {
            //second 32-bit is MAC address
            target_content = (u4Byte)(*(macad+5))<<24
                             |(u4Byte)(*(macad+4))<<16
                             |(u4Byte)(*(macad+3))<<8
                             |(u4Byte)(*(macad+2));
        }
        else
        {
            target_content = (u4Byte)(*(key128+(entry_i*4-8)+3))<<24
                             |(u4Byte)(*(key128+(entry_i*4-8)+2))<<16
                             |(u4Byte)(*(key128+(entry_i*4-8)+1))<<8
                             |(u4Byte)(*(key128+(entry_i*4-8)+0));
        }

        HAL_RTL_W32(REG_CAMWRITE, target_content);
        HAL_RTL_W32(REG_CAMCMD, target_command);
    }

    target_content = HAL_RTL_R32(REG_CR);
    if ((target_content & MAC_SEC_EN) == 0)
    {
        HAL_RTL_W32(REG_CR, (target_content | MAC_SEC_EN));
    }
}


VOID
SetHwReg88XX(
    IN	HAL_PADAPTER		Adapter,
    IN	u1Byte				variable,
    IN	pu1Byte				val
)
{
    // TODO:

    switch(variable)
    {

        case HW_VAR_ETHER_ADDR:
        {
            u1Byte	idx = 0;
            //For Endian Free.
            for(idx=0; idx <6 ; idx++)
            {
                PlatformEFIOWrite1Byte(Adapter, (REG_MACID+idx), val[idx]);
            }


            // Win8: Let the device port use the locally-administered mac address -----------------------------------------
            for(idx=0; idx < 6; idx++)
            {
                PlatformEFIOWrite1Byte(Adapter, (REG_MACID1 + idx), val[idx]);

                if(idx == 0)
                {
                    PlatformEFIOWrite1Byte(Adapter, (REG_MACID1 + idx), val[idx] | BIT1);
                }
            }
            // ---------------------------------------------------------------------------------------------------
        }
        break;

        case HW_VAR_MULTICAST_REG:
        {
            u1Byte	idx=0;
            //For endian free.
            for(idx = 0; idx < 8 ; idx++)
            {
                PlatformEFIOWrite1Byte(Adapter, (REG_MAR+idx), val[idx]);
            }
        }
        break;

        case  HW_VAR_BSSID:
        {
            u1Byte  idx = 0;
            for(idx = 0 ; idx < 6; idx++)
            {
                PlatformEFIOWrite1Byte(Adapter, (REG_BSSID+idx), val[idx]);
            }
        }
        break;

        case HW_VAR_MEDIA_STATUS:
        {
            RT_OP_MODE	OpMode = *((RT_OP_MODE *)(val));
            u1Byte		btMsr = PlatformEFIORead1Byte(Adapter, REG_CR+2);

            btMsr &= 0xfc;

            switch( OpMode )
            {
                case RT_OP_MODE_INFRASTRUCTURE:
                    btMsr |= MSR_INFRA;
                    break;

                case RT_OP_MODE_IBSS:
                    btMsr |= MSR_ADHOC;
                    break;

                case RT_OP_MODE_AP:
                    btMsr |= MSR_AP;
                    break;

                default:
                    btMsr |= MSR_NOLINK;
                    break;
            }

            PlatformEFIOWrite1Byte(Adapter, REG_CR+2, btMsr);
        }
        break;

        case HW_VAR_MAC_LOOPBACK_ENABLE:
        {
            // accept all packets
            HAL_RTL_W32(REG_RCR, HAL_RTL_R32(REG_RCR) | RCR_AAP);

            // enable MAC loopback
            HAL_RTL_W32(REG_CR, HAL_RTL_R32(REG_CR) | (LB_MAC_DLY&LBMODE_Mask)<<LBMODE_SHIFT);
        }
        break;

        case HW_VAR_MAC_CONFIG:
        {
            PMACCONFIG_PARA	pMacCfgPara = (MACCONFIG_PARA *)(val);

            HAL_RTL_W8(REG_INIRTS_RATE_SEL, 0x8); // 24M

            // 2007/02/07 Mark by Emily becasue we have not verify whether this register works
            //For 92C,which reg?
            //	RTL_W8(BWOPMODE, BW_20M);	//	set if work at 20m

            // Ack timeout.
            if ((pMacCfgPara->AckTO > 0) && (pMacCfgPara->AckTO < 0xff))
            {
                HAL_RTL_W8(REG_ACKTO, pMacCfgPara->AckTO);
            }
            else
            {
                HAL_RTL_W8(REG_ACKTO, 0x40);
            }

#if 0
            // clear for mbid beacon tx
            HAL_RTL_W8(MULTI_BCNQ_EN, 0);
            HAL_RTL_W8(MULTI_BCNQ_OFFSET, 0);
#else
            // TODO: Spec has changed, check design
#endif

            // add by Eric, set RateID table 10 for ARFR1 (1SS VHT)
            // RateID 9 is for ARFR0(2SS VHT)
            HAL_RTL_W32(REG_ARFR1, 0x00000015);	
            HAL_RTL_W32(REG_ARFR1+4, 0x003FF000);	


            /*
                        * Disable TXOP CFE
                        */
            HAL_RTL_W16(REG_RD_CTRL, HAL_RTL_R16(REG_RD_CTRL) | BIT10);

            /*
                        *	RA try rate aggr limit
                        */
            HAL_RTL_W8(REG_RA_TRY_RATE_AGG_LMT, 2);

            /*
                	 *	Max mpdu number per aggr
                	 */
            HAL_RTL_W16(REG_PROT_MODE_CTRL+2, 0x0909);

            //3 MAC Beacon Tming Related

            if (pMacCfgPara->vap_enable)
            {
                HAL_RTL_W32(REG_TBTT_PROHIBIT, 0x1df04);
            }
            else
            {
                HAL_RTL_W32(REG_TBTT_PROHIBIT, 0x40004);
            }

            HAL_RTL_W8(REG_DRVERLYINT,          10);
            HAL_RTL_W8(REG_BCNDMATIM,           1);
            HAL_RTL_W16(REG_ATIMWND,            0x3C);
            HAL_RTL_W8(REG_DTIM_COUNTER_ROOT,   pMacCfgPara->dot11DTIMPeriod-1);
            HAL_RTL_W32(REG_PKT_LIFETIME_CTRL,  HAL_RTL_R32(REG_PKT_LIFETIME_CTRL) & ~BIT(19));

#ifdef CFG_HAL_SUPPORT_MBSSID
            if (pMacCfgPara->vap_enable && HAL_NUM_VWLAN == 1 &&
                    (HAL_RTL_R16(REG_MBSSID_BCN_SPACE)< 30))
            {
                HAL_RTL_W8(REG_DRVERLYINT, 6);
            }
#endif  //CFG_HAL_SUPPORT_MBSSID

            HAL_RTL_W8(REG_PRE_DL_BCN_ITV, HAL_RTL_R8(REG_DRVERLYINT)+1);

            HAL_RTL_W8(REG_BCN_CTRL,            DIS_TSF_UPDATE_N|DIS_SUB_STATE_N);
            HAL_RTL_W8(REG_BCN_MAX_ERR,         0xff);
            HAL_RTL_W16(REG_TSFTR_SYN_OFFSET,   0);
            HAL_RTL_W8(REG_DUAL_TSF_RST,        3);
            if( RT_OP_MODE_INFRASTRUCTURE == pMacCfgPara->OP_Mode )
            {
                HAL_RTL_W8(REG_FUNCTION_ENABLE+2, HAL_RTL_R8(REG_FUNCTION_ENABLE+2)^BIT6);
            }

            HAL_RTL_W8(REG_BCN_CTRL, HAL_RTL_R8(REG_BCN_CTRL) | EN_BCN_FUNCTION | EN_TXBCN_RPT);

            if( RT_OP_MODE_AP == pMacCfgPara->OP_Mode )
            {
                HAL_RTL_W16(REG_BCNTCFG, 0x000C);
            }
            else
            {
                HAL_RTL_W16(REG_BCNTCFG, 0x0204);
            }
        }
        break;

        case HW_VAR_EDCA:
        {
            PEDCA_PARA pEDCA = (PEDCA_PARA)(val);
            u1Byte      QueueIdx;
            u4Byte      ACPara;

            for ( QueueIdx=0; QueueIdx<AC_PARAM_SIZE; QueueIdx++ )
            {

                ACPara = ((pEDCA->Para[QueueIdx].TXOPlimit) << 16) \
                         | ((pEDCA->Para[QueueIdx].ECWmax) << 12) \
                         | ((pEDCA->Para[QueueIdx].ECWmin) << 8) \
                         | (pEDCA->sifs_time + (pEDCA->Para[QueueIdx].AIFSN) * pEDCA->slot_time);

                switch(QueueIdx)
                {
                    case AC0_BE:
                        HAL_RTL_W32(REG_EDCA_BE_PARAM, ACPara);
                        break;

                    case AC1_BK:
                        HAL_RTL_W32(REG_EDCA_BK_PARAM, ACPara);
                        break;

                    case AC2_VI:
                        HAL_RTL_W32(REG_EDCA_VI_PARAM, ACPara);
                        break;

                    case AC3_VO:
                        HAL_RTL_W32(REG_EDCA_VO_PARAM, ACPara);
                        break;
                }
            }

            HAL_RTL_W8(REG_ACMHWCTRL, 0x00);
        }
        break;

        case HW_VAR_CAM_RESET_ALL_ENTRY:
        {
            u1Byte  index;

            HAL_RTL_W32(REG_CAMCMD, BIT30);

            for(index = 0; index < HAL_TOTAL_CAM_ENTRY; index++)
                CAMEmptyEntry88XX(Adapter, index);

            HAL_RTL_W32(REG_CR, HAL_RTL_R32(REG_CR) & (~MAC_SEC_EN));
        }
        break;

        case HW_VAR_SECURITY_CONFIG:
        {
            SECURITY_CONFIG_OPERATION SecCfg = *((PSECURITY_CONFIG_OPERATION)(val));
            u2Byte  SecCfgReg = 0;

            if (SecCfg & SCO_TXUSEDK) {
                SecCfgReg |= TXUSEDK;
            }

            if (SecCfg & SCO_RXUSEDK) {
                SecCfgReg |= RXUSEDK;
            }

            if (SecCfg & SCO_TXENC) {
                SecCfgReg |= TXENC;
            }

            if (SecCfg & SCO_RXDEC) {
                SecCfgReg |= RXDEC;
            }

            if (SecCfg & SCO_SKBYA2) {
                SecCfgReg |= SKBYA2;
            }

            if (SecCfg & SCO_NOSKMC) {
                SecCfgReg |= NOSKMC;
            }

            if (SecCfg & SCO_TXBCUSEDK) {
                SecCfgReg |= TXBCUSEDK;
            }

            if (SecCfg & SCO_RXBCUSEDK) {
                SecCfgReg |= RXBCUSEDK;
            }

            if (SecCfg & SCO_CHK_KEYID) {
                SecCfgReg |= CHK_KEYID;
            }

            HAL_RTL_W16(REG_SECCFG, SecCfgReg);
        }
        break;

        case HW_VAR_BEACON_INTERVAL:
        {
            u2Byte BcnInterval = *((pu2Byte)(val));
            HAL_RTL_W16(REG_MBSSID_BCN_SPACE, BcnInterval);
        }
        break;

        case HW_VAR_ENABLE_BEACON_DMA:
        {  
#if IS_RTL8881A_SERIES
            if ( IS_HARDWARE_TYPE_8881AEM(Adapter) )
            {
                u2Byte stop_queue = HAL_RTL_R16(REG_LX_CTRL1);
                stop_queue &= ~BIT_STOP_BCNQ;
                HAL_RTL_W16(REG_LX_CTRL1, stop_queue);
            }
#endif
#if IS_RTL8192E_SERIES
            if ( IS_HARDWARE_TYPE_8192E(Adapter) )
            {
                u2Byte stop_queue = HAL_RTL_R16(REG_PCIE_CTRL1);
                stop_queue &= ~BIT_STOP_BCNQ;
                HAL_RTL_W16(REG_PCIE_CTRL1, stop_queue);
            }
#endif
        }
        break;

        case HW_VAR_TXPAUSE:
        {
            u1Byte QueueIndexBIT = *((pu1Byte)(val));

            HAL_RTL_W8(REG_TXPAUSE, QueueIndexBIT);
        }
        break;


        case HW_VAR_HIQ_NO_LMT_EN:
        {
            u1Byte HiQNoLMTEn = *((pu1Byte)(val));            
            HAL_RTL_W8(REG_HIQ_NO_LMT_EN, HiQNoLMTEn);
        }
        break;            

        case HW_VAR_DRV_DBG:
        {
            u4Byte ErrorFlag = *((pu4Byte)(val));            
            HAL_RTL_W32(REGDUMP_DRV_ERR0, ErrorFlag);            
        }
        break;

        case HW_VAR_NUM_TXDMA_STATUS:
        {
            u4Byte RegTxDMA = *((pu4Byte)(val));            
            HAL_RTL_W32(REG_TXDMA_STATUS, RegTxDMA); 
        }
        break;

        case HW_VAR_NUM_RXDMA_STATUS:
        {
            u1Byte RegRxDMA = *((pu1Byte)(val));            
            HAL_RTL_W8(REG_RXDMA_STATUS, RegRxDMA);
        }
        break;
        
        default:
            RT_TRACE_F(COMP_IO, DBG_WARNING, ("Command ID(%d) not Supported\n", variable));
            break;
    }
}


VOID
GetHwReg88XX(
    IN      HAL_PADAPTER          Adapter,
    IN      u1Byte                variable,
    OUT     pu1Byte               val
)
{
    // TODO:


    switch(variable)
    {
        case HW_VAR_ETHER_ADDR:
        {
            *((pu4Byte)(val)) = PlatformEFIORead4Byte(Adapter, REG_MACID);
            *((pu2Byte)(val+4)) = PlatformEFIORead2Byte(Adapter, REG_MACID+4);
        }
        break;

        case HW_VAR_BSSID:
        {
            *((pu4Byte)(val)) = PlatformEFIORead4Byte(Adapter, REG_BSSID);
            *((pu2Byte)(val+4)) = PlatformEFIORead2Byte(Adapter, (REG_BSSID+4));
        }
        break;


        case HW_VAR_MAC_IO_ENABLE:
        {
            *((PBOOLEAN)val) = ((PlatformEFIORead2Byte(Adapter, REG_SYS_FUNC_EN) & (FEN_MREGEN|FEN_DCORE) ) == (FEN_MREGEN|FEN_DCORE));
        }
        break;

        case HW_VAR_MACREGFILE_START:
        {
#if IS_RTL8881A_SERIES
            if ( IS_HARDWARE_TYPE_8881AEM(Adapter) )
            {
                *((pu4Byte)(val)) = (u4Byte)data_MAC_REG_8881A_start;
            }
#endif
#if IS_RTL8192E_SERIES
            if ( IS_HARDWARE_TYPE_8192E(Adapter) )
            {
	            if(_GET_HAL_DATA(Adapter)->cutVersion	== ODM_CUT_B)
	                *((pu4Byte)(val)) = (u4Byte)data_MAC_REG_8192Eb_start;
				else
	                *((pu4Byte)(val)) = (u4Byte)data_MAC_REG_8192E_start;
            }
#endif
        }
        break;

        case HW_VAR_MACREGFILE_SIZE:
        {
#if IS_RTL8881A_SERIES
            if ( IS_HARDWARE_TYPE_8881A(Adapter) )
            {
                *((pu4Byte)(val)) = (u4Byte)(data_MAC_REG_8881A_end - data_MAC_REG_8881A_start);
            }
#endif
#if IS_RTL8192E_SERIES
            if ( IS_HARDWARE_TYPE_8192E(Adapter) )
            {
	            if(_GET_HAL_DATA(Adapter)->cutVersion	== ODM_CUT_B)
					*((pu4Byte)(val)) = (u4Byte)(data_MAC_REG_8192Eb_end - data_MAC_REG_8192Eb_start);
				else
	                *((pu4Byte)(val)) = (u4Byte)(data_MAC_REG_8192E_end - data_MAC_REG_8192E_start);
            }
#endif
        }
        break;

        case HW_VAR_PHYREGFILE_START:
        {
#if IS_RTL8881A_SERIES
            if ( IS_HARDWARE_TYPE_8881AEM(Adapter) )
            {
                *((pu4Byte)(val)) = (u4Byte)data_PHY_REG_8881A_start;
            }
#endif
#if IS_RTL8192E_SERIES
            if ( IS_HARDWARE_TYPE_8192E(Adapter) )
            {
	            if(_GET_HAL_DATA(Adapter)->cutVersion	== ODM_CUT_B)
					 *((pu4Byte)(val)) = (u4Byte)data_PHY_REG_8192Eb_start;
				else
	                *((pu4Byte)(val)) = (u4Byte)data_PHY_REG_8192E_start;
            }
#endif            
        }
        break;
            
        case HW_VAR_PHYREGFILE_SIZE:
        {
#if IS_RTL8881A_SERIES
            if ( IS_HARDWARE_TYPE_8881A(Adapter) )
            {
                *((pu4Byte)(val)) = (u4Byte)(data_PHY_REG_8881A_end - data_PHY_REG_8881A_start);
            }
#endif
#if IS_RTL8192E_SERIES
            if ( IS_HARDWARE_TYPE_8192E(Adapter) )
            {
            	if(_GET_HAL_DATA(Adapter)->cutVersion	== ODM_CUT_B)
	                *((pu4Byte)(val)) = (u4Byte)(data_PHY_REG_8192Eb_end - data_PHY_REG_8192Eb_start);
				else
	                *((pu4Byte)(val)) = (u4Byte)(data_PHY_REG_8192E_end - data_PHY_REG_8192E_start);					
            }
#endif            
        }    
        break;

        case HW_VAR_PHYREGFILE_1T_START:
        {
#if IS_RTL8881A_SERIES
            if ( IS_HARDWARE_TYPE_8881AEM(Adapter) )
            {
                *((pu4Byte)(val)) = (u4Byte)data_PHY_REG_1T_8881A_start;
            }
#endif
#if 0 //IS_RTL8192E_SERIES
            if ( IS_HARDWARE_TYPE_8192E(Adapter) )
            {
                *((pu4Byte)(val)) = (u4Byte)data_PHY_REG_1T_8192E_start;
            }
#endif            
        }
        break;
            
        case HW_VAR_PHYREGFILE_1T_SIZE:
        {
#if IS_RTL8881A_SERIES
            if ( IS_HARDWARE_TYPE_8881A(Adapter) )
            {
                *((pu4Byte)(val)) = (u4Byte)(data_PHY_REG_1T_8881A_end - data_PHY_REG_1T_8881A_start);
            }
#endif
#if 0 //IS_RTL8192E_SERIES
            if ( IS_HARDWARE_TYPE_8192E(Adapter) )
            {
                *((pu4Byte)(val)) = (u4Byte)(data_PHY_REG_1T_8192E_end - data_PHY_REG_1T_8192E_start);
            }
#endif            
        }    
        break;

        case HW_VAR_PHYREGFILE_MP_START:
        {
#if IS_RTL8881A_SERIES
            if ( IS_HARDWARE_TYPE_8881AEM(Adapter) )
            {
                *((pu4Byte)(val)) = (u4Byte)data_PHY_REG_MP_8881A_start;
            }
#endif
#if IS_RTL8192E_SERIES
            if ( IS_HARDWARE_TYPE_8192E(Adapter) )
            {
                *((pu4Byte)(val)) = (u4Byte)data_PHY_REG_MP_8192E_start;
            }
#endif            
        }
        break;
            
        case HW_VAR_PHYREGFILE_MP_SIZE:
        {
#if IS_RTL8881A_SERIES
            if ( IS_HARDWARE_TYPE_8881A(Adapter) )
            {
                *((pu4Byte)(val)) = (u4Byte)(data_PHY_REG_MP_8881A_end - data_PHY_REG_MP_8881A_start);
            }
#endif
#if IS_RTL8192E_SERIES
            if ( IS_HARDWARE_TYPE_8192E(Adapter) )
            {
                *((pu4Byte)(val)) = (u4Byte)(data_PHY_REG_MP_8192E_end - data_PHY_REG_MP_8192E_start);
            }
#endif            
        }    
        break;

        case HW_VAR_PHYREGFILE_PG_START:
        {
#if IS_RTL8881A_SERIES
            if ( IS_HARDWARE_TYPE_8881AEM(Adapter) )
            {
                *((pu4Byte)(val)) = (u4Byte)data_PHY_REG_PG_8881A_start;
            }
#endif
#if IS_RTL8192E_SERIES
            if ( IS_HARDWARE_TYPE_8192E(Adapter) )
            {
                *((pu4Byte)(val)) = (u4Byte)data_PHY_REG_PG_8192E_start;
            }
#endif            
        }
        break;
            
        case HW_VAR_PHYREGFILE_PG_SIZE:
        {
#if IS_RTL8881A_SERIES
            if ( IS_HARDWARE_TYPE_8881A(Adapter) )
            {
                *((pu4Byte)(val)) = (u4Byte)(data_PHY_REG_PG_8881A_end - data_PHY_REG_PG_8881A_start);
            }
#endif
#if IS_RTL8192E_SERIES
            if ( IS_HARDWARE_TYPE_8192E(Adapter) )
            {
                *((pu4Byte)(val)) = (u4Byte)(data_PHY_REG_PG_8192E_end - data_PHY_REG_PG_8192E_start);
            }
#endif            
        }    
        break;

        case HW_VAR_PHYREGFILE_AGC_START:
        {
#if IS_RTL8881A_SERIES
            if ( IS_HARDWARE_TYPE_8881AEM(Adapter) )
            {
                *((pu4Byte)(val)) = (u4Byte)data_AGC_TAB_8881A_start;
            }
#endif
#if IS_RTL8192E_SERIES
            if ( IS_HARDWARE_TYPE_8192E(Adapter) )
            {
                *((pu4Byte)(val)) = (u4Byte)data_AGC_TAB_8192E_start;
            }
#endif            
        }
        break;
            
        case HW_VAR_PHYREGFILE_AGC_SIZE:
        {
#if IS_RTL8881A_SERIES
            if ( IS_HARDWARE_TYPE_8881A(Adapter) )
            {
                *((pu4Byte)(val)) = (u4Byte)(data_AGC_TAB_8881A_end - data_AGC_TAB_8881A_start);
            }
#endif
#if IS_RTL8192E_SERIES
            if ( IS_HARDWARE_TYPE_8192E(Adapter) )
            {
                *((pu4Byte)(val)) = (u4Byte)(data_AGC_TAB_8192E_end - data_AGC_TAB_8192E_start);
            }
#endif            
        }    
        break; 

        case HW_VAR_RFREGFILE_RADIO_A_START:
        {
#if IS_RTL8881A_SERIES
            if ( IS_HARDWARE_TYPE_8881AEM(Adapter) )
            {
                *((pu4Byte)(val)) = (u4Byte)data_RadioA_8881A_start;
            }
#endif
#if IS_RTL8192E_SERIES
            if ( IS_HARDWARE_TYPE_8192E(Adapter) )
            {
            	if(_GET_HAL_DATA(Adapter)->cutVersion	== ODM_CUT_B)
	                *((pu4Byte)(val)) = (u4Byte)data_RadioA_8192Eb_start;
				else
	                *((pu4Byte)(val)) = (u4Byte)data_RadioA_8192E_start;					
            }
#endif            
        }
        break;
            
        case HW_VAR_RFREGFILE_RADIO_A_SIZE:
        {
#if IS_RTL8881A_SERIES
            if ( IS_HARDWARE_TYPE_8881A(Adapter) )
            {
                *((pu4Byte)(val)) = (u4Byte)(data_RadioA_8881A_end - data_RadioA_8881A_start);
            }
#endif
#if IS_RTL8192E_SERIES
            if ( IS_HARDWARE_TYPE_8192E(Adapter) )
            {
            	if(_GET_HAL_DATA(Adapter)->cutVersion	== ODM_CUT_B)
	                *((pu4Byte)(val)) = (u4Byte)(data_RadioA_8192Eb_end - data_RadioA_8192Eb_start);
				else
	                *((pu4Byte)(val)) = (u4Byte)(data_RadioA_8192E_end - data_RadioA_8192E_start);					
            }
#endif            
        }    
        break;

        case HW_VAR_RFREGFILE_RADIO_B_START:
        {
#if IS_RTL8192E_SERIES
            if ( IS_HARDWARE_TYPE_8192E(Adapter) )
            {
            	if(_GET_HAL_DATA(Adapter)->cutVersion	== ODM_CUT_B)
	                *((pu4Byte)(val)) = (u4Byte)data_RadioB_8192Eb_start;
				else
                *((pu4Byte)(val)) = (u4Byte)data_RadioB_8192E_start;					
            }
#endif            
        }
        break;
            
        case HW_VAR_RFREGFILE_RADIO_B_SIZE:
        {
#if IS_RTL8192E_SERIES
            if ( IS_HARDWARE_TYPE_8192E(Adapter) )
            {
            	if(_GET_HAL_DATA(Adapter)->cutVersion	== ODM_CUT_B)
	                *((pu4Byte)(val)) = (u4Byte)(data_RadioB_8192Eb_end - data_RadioB_8192Eb_start);
				else
	                *((pu4Byte)(val)) = (u4Byte)(data_RadioB_8192E_end - data_RadioB_8192E_start);					
            }
#endif            
        }    
        break;        

        case HW_VAR_FWFILE_START:
        {
#if IS_RTL8881A_SERIES
            if ( IS_HARDWARE_TYPE_8881A(Adapter))
            {
                if(IS_HAL_TEST_CHIP(Adapter))
                {
                    *((pu4Byte)(val)) = (u4Byte)data_RTL8881FW_Test_T_start;
                }
                else
                {
                    *((pu4Byte)(val)) = (u4Byte)data_RTL8881FW_A_CUT_T_start;
                }
            }
#endif
#if IS_RTL8192E_SERIES
            if ( IS_HARDWARE_TYPE_8192E(Adapter) )
            {
                *((pu4Byte)(val)) = (u4Byte)data_rtl8192Efw_start;
            }
#endif
        }
        break;

        case HW_VAR_FWFILE_SIZE:
        {
#if IS_RTL8881A_SERIES
            if ( IS_HARDWARE_TYPE_8881A(Adapter))
            {
                if(IS_HAL_TEST_CHIP(Adapter))
                {
                    *((pu4Byte)(val)) = (u4Byte)(data_RTL8881FW_Test_T_end - data_RTL8881FW_Test_T_start);
                }
                else
                {
                    *((pu4Byte)(val)) = (u4Byte)(data_RTL8881FW_A_CUT_T_end - data_RTL8881FW_A_CUT_T_start);
                }
            }
#endif
#if IS_RTL8192E_SERIES
            if ( IS_HARDWARE_TYPE_8192E(Adapter) )
            {
                *((pu4Byte)(val)) = (u4Byte)(data_rtl8192Efw_end - data_rtl8192Efw_start);
            }
#endif
        }
        break;

#if IS_RTL8881A_SERIES
        case HW_VAR_TXPKTFWFILE_START:
        {
             if ( IS_HARDWARE_TYPE_8881A(Adapter))
             {
                 if(IS_HAL_TEST_CHIP(Adapter))
                 {
                     *((pu4Byte)(val)) = (u4Byte)data_RTL8881TXBUF_Test_T_start;
                 }
                 else
                 {
                     *((pu4Byte)(val)) = (u4Byte)data_RTL8881TXBUF_A_CUT_T_start;
                 }
             }                      
        }
        break;

        case HW_VAR_TXPKTFWFILE_SIZE:
        {
              if ( IS_HARDWARE_TYPE_8881A(Adapter))
              {
                 if(IS_HAL_TEST_CHIP(Adapter))
                 {
                     *((pu4Byte)(val)) = (u4Byte)(data_RTL8881TXBUF_Test_T_end - data_RTL8881TXBUF_Test_T_start);                 
                 }
                 else
                 {
                     *((pu4Byte)(val)) = (u4Byte)(data_RTL8881TXBUF_A_CUT_T_end - data_RTL8881TXBUF_A_CUT_T_start);
                 }
              }    
        }
        break;
#endif

        case HW_VAR_MEDIA_STATUS:
        {
            val[0] = PlatformEFIORead1Byte(Adapter, REG_CR+2)&0x3;

            switch( val[0] )
            {
                case MSR_INFRA:
                    val[0] = RT_OP_MODE_INFRASTRUCTURE;
                    break;

                case MSR_ADHOC:
                    val[0] = RT_OP_MODE_IBSS;
                    break;

                case MSR_AP:
                    val[0] = RT_OP_MODE_AP;
                    break;

                default:
                    val[0] = RT_OP_MODE_NO_LINK;
                    break;
            }
        }
        break;

        case HW_VAR_SECURITY_CONFIG:
        {
            u2Byte                      SecCfgReg;
            SECURITY_CONFIG_OPERATION   SecCfg = 0;

            SecCfgReg = HAL_RTL_R16(REG_SECCFG);

            if (SecCfgReg & TXUSEDK) {
                SecCfg |= SCO_TXUSEDK;
            }

            if (SecCfgReg & RXUSEDK) {
                SecCfg |= SCO_RXUSEDK;
            }

            if (SecCfgReg & TXENC) {
                SecCfg |= SCO_TXENC;
            }

            if (SecCfgReg & RXDEC) {
                SecCfg |= SCO_RXDEC;
            }

            if (SecCfgReg & SKBYA2) {
                SecCfg |= SCO_SKBYA2;
            }

            if (SecCfgReg & NOSKMC) {
                SecCfg |= SCO_NOSKMC;
            }

            if (SecCfgReg & TXBCUSEDK) {
                SecCfg |= SCO_TXBCUSEDK;
            }

            if (SecCfgReg & RXBCUSEDK) {
                SecCfg |= SCO_RXBCUSEDK;
            }

            if (SecCfgReg & CHK_KEYID) {
                SecCfg |= SCO_CHK_KEYID;
            }

            *((PSECURITY_CONFIG_OPERATION)(val)) = SecCfg;
        }
        break;

        case HW_VAR_BEACON_INTERVAL:
        {
            *((pu2Byte)(val)) = PlatformEFIORead2Byte(Adapter, REG_MBSSID_BCN_SPACE);
        }
        break;

        case HW_VAR_TXPAUSE:
        {
            *((pu1Byte)(val)) = PlatformEFIORead1Byte(Adapter, REG_TXPAUSE);
        }
        break;

        case HW_VAR_HIQ_NO_LMT_EN:
        {          
            *((pu1Byte)(val)) = PlatformEFIORead1Byte(Adapter, REG_HIQ_NO_LMT_EN);
        }
        break;     

        case HW_VAR_DRV_DBG:
        {
            *((pu4Byte)(val)) = PlatformEFIORead4Byte(Adapter, REGDUMP_DRV_ERR0);         
        }
        break;        


        case HW_VAR_NUM_TXDMA_STATUS:
        {
            *((pu4Byte)(val)) = PlatformEFIORead4Byte(Adapter, REG_TXDMA_STATUS);    
        }
        break;

        case HW_VAR_NUM_RXDMA_STATUS:
        {
            *((pu1Byte)(val)) = PlatformEFIORead1Byte(Adapter, REG_RXDMA_STATUS);
        }
        break;        

        case HW_VAR_NUM_TOTAL_RF_PATH:
        {
#if IS_RTL8881A_SERIES
            if ( IS_HARDWARE_TYPE_8881A(Adapter) )
            {
                *((pu4Byte)(val)) = (u4Byte)1;
            }
#endif
#if IS_RTL8192E_SERIES
            if ( IS_HARDWARE_TYPE_8192E(Adapter) )
            {
                *((pu4Byte)(val)) = (u4Byte)2;
            }
#endif            
        }
        break;

        default:
            RT_TRACE_F(COMP_IO, DBG_WARNING, ("Command ID(%d) not Supported\n", variable));
            break;
    }
}

static RT_STATUS
LLT_table_init88XX(
    IN  HAL_PADAPTER    Adapter
)
{
    u32 i, count = 0;
    u32 txpktbufSz, bufBd;

    if (TXPKTBUF_LLT_PAGECNT >=TXPKTBUF_TOTAL_PAGECNT) {
        RT_TRACE(COMP_INIT, DBG_SERIOUS, ("LLT init fail, size not match, error !!! \n") );
        return RT_STATUS_FAILURE;        
    }

    if (TX_PAGE_CNT_RSV + TXPKTBUF_LLT_PAGECNT > TXPKTBUF_TOTAL_PAGECNT) {
        RT_TRACE(COMP_INIT, DBG_SERIOUS, ("LLT init fail, size not match, error !!! \n") );
        return RT_STATUS_FAILURE;
    }

    txpktbufSz  = TXPKTBUF_LLT_PAGECNT;
    bufBd       = TXPKTBUF_TOTAL_PAGECNT - 1;

    // Set reserved page for each queue
    /* normal queue init MUST be previous of RQPN enable */
    HAL_RTL_W8(REG_RQPN_NPQ, TX_PAGE_CNT_NPQ);
    HAL_RTL_W8(REG_RQPN_NPQ+2, TX_PAGE_CNT_EPQ);
    HAL_RTL_W32(REG_RQPN, BIT31 | (TX_PAGE_CNT_PUBQ << 16) | (TX_PAGE_CNT_LPQ << 8) | (TX_PAGE_CNT_HPQ));

    HAL_RTL_W8(REG_TXPKTBUF_BCNQ_BDNY, txpktbufSz);
    HAL_RTL_W8(REG_TXPKTBUF_MGQ_BDNY, txpktbufSz);
    HAL_RTL_W8(REG_TDECTRL+1, txpktbufSz);

    HAL_RTL_W8(REG_TXPKTBUF_BCNQ_BDNY1, txpktbufSz + SECOND_BCN_PAGE_OFFSET);
    
#if CFG_HAL_SUPPORT_MBSSID
    HAL_RTL_W8(REG_TDECTRL1+1, txpktbufSz + SECOND_BCN_PAGE_OFFSET);    
#else
    HAL_RTL_W8(REG_TDECTRL1+1, txpktbufSz);
#endif // CFG_HAL_SUPPORT_MBSSID


#if 0
    for (i = 0; i < txpktbufSz-1; i++)
    {
        HAL_RTL_W32(REG_LLT_INIT, ((LLTE_RWM_WR&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)|(i&LLTINI_ADDR_Mask)<<LLTINI_ADDR_SHIFT
                    |((i+1)&LLTINI_HDATA_Mask)<<LLTINI_HDATA_SHIFT);

        count = 0;
        do
        {
            if (!(HAL_RTL_R32(REG_LLT_INIT) & ((LLTE_RWM_RD&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)))
                break;
            if (++count >= LLT_TABLE_INIT_POLLING_CNT)
            {
                RT_TRACE(COMP_INIT, DBG_WARNING, ("LLT_init, section 01, i=%d\n", i) );
                RT_TRACE(COMP_INIT, DBG_WARNING, ("LLT Polling failed 01 !!!\n") );

                return RT_STATUS_FAILURE;
            }
        }
        while(count < LLT_TABLE_INIT_POLLING_CNT);
    }

    HAL_RTL_W32(REG_LLT_INIT, ((LLTE_RWM_WR&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)
                |((txpktbufSz-1)&LLTINI_ADDR_Mask)<<LLTINI_ADDR_SHIFT|(255&LLTINI_HDATA_Mask)<<LLTINI_HDATA_SHIFT);

    count = 0;
    do
    {
        if (!(HAL_RTL_R32(REG_LLT_INIT) & ((LLTE_RWM_RD&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)))
            break;
        if (++count >= LLT_TABLE_INIT_POLLING_CNT)
        {
            RT_TRACE(COMP_INIT, DBG_WARNING, ("LLT Polling failed 02 !!!\n") );
            return RT_STATUS_FAILURE;
        }
    }
    while(count < LLT_TABLE_INIT_POLLING_CNT);

    for (i = txpktbufSz; i < bufBd; i++)
    {
        HAL_RTL_W32(REG_LLT_INIT, ((LLTE_RWM_WR&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)|(i&LLTINI_ADDR_Mask)<<LLTINI_ADDR_SHIFT
                    |((i+1)&LLTINI_HDATA_Mask)<<LLTINI_HDATA_SHIFT);

        do
        {
            if (!(HAL_RTL_R32(REG_LLT_INIT) & ((LLTE_RWM_RD&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)))
                break;
            if (++count >= LLT_TABLE_INIT_POLLING_CNT)
            {
                RT_TRACE(COMP_INIT, DBG_WARNING, ("LLT Polling failed 03 !!!\n") );
                return RT_STATUS_FAILURE;
            }
        }
        while(count < LLT_TABLE_INIT_POLLING_CNT);
    }

    HAL_RTL_W32(REG_LLT_INIT, ((LLTE_RWM_WR&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)|(bufBd&LLTINI_ADDR_Mask)<<LLTINI_ADDR_SHIFT
                |(txpktbufSz&LLTINI_HDATA_Mask)<<LLTINI_HDATA_SHIFT);

    count = 0;
    do
    {
        if (!(HAL_RTL_R32(REG_LLT_INIT) & ((LLTE_RWM_RD&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)))
            break;
        if(++count >= LLT_TABLE_INIT_POLLING_CNT)
        {
            RT_TRACE(COMP_INIT, DBG_WARNING, ("LLT Polling failed 04 !!!\n") );
            return RT_STATUS_FAILURE;
        }
    }
    while(count < LLT_TABLE_INIT_POLLING_CNT);
#else
    HAL_RTL_W32(REG_AUTO_LLT_INIT, HAL_RTL_R32(REG_AUTO_LLT_INIT) | BIT16);

    count = 0;
    do
    {
        if ((HAL_RTL_R32(REG_AUTO_LLT_INIT) & BIT16) == 0 ) {
            // Success
            break;
        }

        if(++count >= LLT_TABLE_INIT_POLLING_CNT)
        {
            RT_TRACE(COMP_INIT, DBG_WARNING, ("Auto LLT Polling failed !!!\n") );
            return RT_STATUS_FAILURE;
        }
    }
    while(1);
#endif

    return RT_STATUS_SUCCESS;
}

RT_STATUS
InitMAC88XX(
    IN  HAL_PADAPTER Adapter
)
{
    u4Byte  errorFlag = 0;
    
    RT_TRACE(COMP_INIT, DBG_LOUD, ("===>%s\n", __FUNCTION__));

    //Clear RegDumpErr 
    GET_HAL_INTERFACE(Adapter)->SetHwRegHandler(Adapter, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
    
    // Release MAC IO register reset
    HAL_RTL_W32(REG_CR, HAL_RTL_R32(REG_CR)|MACRXEN|MACTXEN|SCHEDULE_EN|PROTOCOL_EN
                |RXDMA_EN|TXDMA_EN|HCI_RXDMA_EN|HCI_TXDMA_EN);

    //System init
    if ( RT_STATUS_SUCCESS != LLT_table_init88XX(Adapter) ) {
        RT_TRACE(COMP_INIT, DBG_SERIOUS, ("LLT_table_init Failed \n") );
        return RT_STATUS_FAILURE;
    }
    else {
        RT_TRACE(COMP_INIT, DBG_TRACE, ("LLT_table_init OK \n") );  
    }

    // Clear interrupt and enable interrupt
    HAL_RTL_W32(REG_HISR0, 0xFFFFFFFF);
    HAL_RTL_W32(REG_HISR1, 0xFFFFFFFF);

#if IS_EXIST_RTL8881AEM
    if ( IS_HARDWARE_TYPE_8881AEM(Adapter) )
    {
        if (IS_HAL_TEST_CHIP(Adapter) )
        {
            // TODO:
//            HAL_RTL_W32(REG_LBUS_DMA_ISR, 0xFFFFFFFF);
        }
        else
        {
            // TODO:
//            HAL_RTL_W32(REG_LBUS_DMA_ISR, 0xFFFFFFFF);
        }
    }
#endif  //IS_EXIST_RTL8881AEM

    // RXFF
    HAL_RTL_W32(REG_TRXFF_BNDY, (HAL_RTL_R32(REG_TRXFF_BNDY)&0x0000FFFF)|(MAC_RXFF_SIZE&RXFF0_BNDY_Mask)<<RXFF0_BNDY_SHIFT);

    // TRX DMA Queue Mapping
    HAL_RTL_W16(REG_TRX_DMA_CTRL, (TRX_DMA_QUEUE_MAP_PARA | RXSHFT_EN | RXDMA_ARBBW_EN));

    // Set Network type: ap mode
    HAL_RTL_W32(REG_CR, HAL_RTL_R32(REG_CR) | ((NETYPE_AP & NETYPE_Mask) << NETYPE_SHIFT));

    // Set SLOT time
    HAL_RTL_W8(REG_SLOT, 0x09);

    // Set Tx/Rx page size (Tx must be 256 Bytes, Rx can be 64,128,256,512,1024 bytes)
    HAL_RTL_W8(REG_PBP, ((PBP_PSTX_SIZE/PBP_UNIT)&PSTX_Mask)<<PSTX_SHIFT | \
                            ((PBP_PSRX_SIZE/PBP_UNIT)&PSRX_Mask)<<PSRX_SHIFT);

    // Set RCR register
    HAL_RTL_W32(REG_RCR, RCR_APP_FCS|RCR_APP_MIC|RCR_APP_ICV|RCR_APP_PHYSTS|RCR_HTC_LOC_CTRL
                |RCR_AMF|RCR_ADF|RCR_AICV|RCR_ACRC32|RCR_AB|RCR_AM|RCR_APM|RCR_AAP);

    // Set Driver info size
    HAL_RTL_W8(REG_RX_DRVINFO_SZ, 4);

    // This part is not in WMAC InitMAC()
    // Set SEC register
    HAL_RTL_W16(REG_SECCFG, HAL_RTL_R16(REG_SECCFG) & ~(RXUSEDK | TXUSEDK));

    // Set TCR register
    HAL_RTL_W32(REG_TCR, HAL_RTL_R32(REG_TCR)|CFE_FORM);

    // Set TCR to avoid deadlock
    HAL_RTL_W32(REG_TCR, HAL_RTL_R32(REG_TCR)|BIT15|BIT14|BIT13|BIT12);

    // Set TCR to enable mactx update DTIM count, group bit, and moreData bit
    HAL_RTL_W8(REG_TCR, HAL_RTL_R8(REG_TCR)|HW_DTIM|HIQ_HW_MD);


    HAL_RTL_W16(REG_RRSR, 0xFFFF);
    HAL_RTL_W8(REG_RRSR+2, 0xFF);

    // Set Spec SIFS (used in NAV)
    // Joseph test
    HAL_RTL_W16(REG_SPEC_SIFS, (0x0A&SPEC_SIFS_OFDM_Mask)<<SPEC_SIFS_OFDM_SHIFT
                |(0x0A&SPEC_SIFS_CCK_Mask)<<SPEC_SIFS_CCK_SHIFT);

    // Set SIFS for CCK
    // Joseph test
    HAL_RTL_W16(REG_SIFS, (0x0A&SIFS_TRX_Mask)<<SIFS_TRX_SHIFT|(0x0A&SIFS_CTX_Mask)<<SIFS_CTX_SHIFT);

    // Set SIFS for OFDM
    // Joseph test
    HAL_RTL_W16(REG_SIFS+2, (0x0A&SIFS_TRX_Mask)<<SIFS_TRX_SHIFT|(0x0A&SIFS_CTX_Mask)<<SIFS_CTX_SHIFT);

    // EIFS
    HAL_RTL_W16(REG_EIFS, 0x0040);	// eifs = 40 us

    // Set retry limit
    HAL_VAR_RETRY_LIMIT_SHORT           = RETRY_LIMIT_SHORT_AP;
    HAL_VAR_RETRY_LIMIT_LONG            = RETRY_LIMIT_LONG_AP;

#if IS_EXIST_RTL8192EE // TODO: just for TP tuning 
    // TODO: Joseph test
    HAL_VAR_RETRY_LIMIT_SHORT           = 0x30;//RETRY_LIMIT_SHORT_AP;
    HAL_VAR_RETRY_LIMIT_LONG            = 0x30;//RETRY_LIMIT_LONG_AP;
#endif

#if CFG_HAL_SUPPORT_CLIENT_MODE
    HAL_VAR_RETRY_LIMIT_SHORT           = RETRY_LIMIT_SHORT_CLIENT;
    HAL_VAR_RETRY_LIMIT_LONG            = RETRY_LIMIT_LONG_CLIENT;    
#endif
    HAL_RTL_W16(REG_RL, (HAL_VAR_RETRY_LIMIT_SHORT&SRL_Mask)<<SRL_SHIFT| \
                        (HAL_VAR_RETRY_LIMIT_LONG&LRL_Mask)<<LRL_SHIFT);

    // disable BT_enable
    HAL_RTL_W8(REG_GPIO_MUXCFG, 0);


#if 1 // TODO: Filen, Because 8881A Pin Mux issue 
    if ( IS_HARDWARE_TYPE_8881AEM(Adapter) ) {
        HAL_RTL_W32(REG_LEDCFG, HAL_RTL_R32(REG_LEDCFG) & ~(BIT23 | BIT22));
    }
#endif


    RT_TRACE(COMP_INIT, DBG_LOUD, ("%s===>\n", __FUNCTION__) );

    return RT_STATUS_SUCCESS;
}

VOID
InitIMR88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  RT_OP_MODE      OPMode
)
{
    PHAL_DATA_TYPE              pHalData = _GET_HAL_DATA(Adapter);

    RT_TRACE_F(COMP_INIT, DBG_LOUD, ("\n"));

    pHalData->IntMask[0]        = BIT_RXOK | HIMR_HISR1_IND_INT | BIT_CTWEND | BIT_RDU;
#if CFG_HAL_MP_TEST
    if (HAL_VAR_MP_SPECIFIC)
        pHalData->IntMask[0]    |= BIT_BEDOK;
#endif
    pHalData->IntMask[1]        = BIT_FOVW | BIT_TXFOVW | BIT_RXERR_INT | BIT_TXERR_INT;

    switch(OPMode)
    {
        case RT_OP_MODE_AP:
            pHalData->IntMask[0]     |= BIT_BCNDMAINT0 | BIT_TXBCNOK | BIT_TXBCNERR;
#ifdef TXREPORT
			pHalData->IntMask[0]     |= BIT_C2HCMD_INT;
#endif
            break;

#if CFG_HAL_SUPPORT_CLIENT_MODE
        case RT_OP_MODE_INFRASTRUCTURE:
            break;

        case RT_OP_MODE_IBSS:
            pHalData->IntMask[0]     |= BIT_RXOK | HIMR_HISR1_IND_INT;
            break;
#endif  //CFG_HAL_SUPPORT_CLIENT_MODE

        default:
            break;
    }
}


VOID
InitVAPIMR88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          VapSeq
)
{
    PHAL_DATA_TYPE      pHalData = _GET_HAL_DATA(Adapter);
    
    pHalData->IntMask[1]    |=  BIT_BCNDMAINT1<<(VapSeq - 1);

    // TODO: Filen, we should add code for relative opeartion belw
    #if 0
    pHalData->IntMask[1]    |=  BIT_BCNDERR1<<(VapSeq - 1);
    #endif

    HAL_RTL_W32(REG_HIMR1, pHalData->IntMask[1]);
}



RT_STATUS
InitHCIDMAMem88XX(
    IN  HAL_PADAPTER Adapter
)
{   
    PHCI_TX_DMA_MANAGER_88XX    ptx_dma=NULL;
    PHCI_RX_DMA_MANAGER_88XX    prx_dma=NULL;

#ifdef CONFIG_NET_PCI
    unsigned char* page_ptr=NULL;
    u4Byte ring_dma_addr=0;
    u4Byte page_align_phy=0;

    unsigned int dma_len= DESC_DMA_PAGE_SIZE_MAX;
  
    if ((((Adapter->pshare->type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS)) {
        page_ptr = pci_alloc_consistent(Adapter->pshare->pdev, dma_len, (dma_addr_t *)&ring_dma_addr);
        if (NULL == page_ptr){
            RT_TRACE_F( COMP_INIT, DBG_SERIOUS, ("Allocate HAL Memory-TX Failed\n") );
            return RT_STATUS_FAILURE;
        }
        
        _GET_HAL_DATA(Adapter)->ring_dma_addr = ring_dma_addr;
        _GET_HAL_DATA(Adapter)->alloc_dma_buf = (u4Byte)page_ptr;

#if defined(NOT_RTK_BSP)		
        page_align_phy = (HAL_PAGE_SIZE - (((u4Byte)page_ptr) & (HAL_PAGE_SIZE-1)));
#endif	
        page_ptr = (unsigned char *)
                    (((u4Byte)page_ptr) + (HAL_PAGE_SIZE - (((u4Byte)page_ptr) & (PAGE_SIZE-1))));
		_GET_HAL_DATA(Adapter)->ring_buf_len = _GET_HAL_DATA(Adapter)->alloc_dma_buf + dma_len - ((u4Byte)page_ptr);

#if defined(NOT_RTK_BSP)
		_GET_HAL_DATA(Adapter)->ring_dma_addr = _GET_HAL_DATA(Adapter)->ring_dma_addr + page_align_phy;  
#else
		_GET_HAL_DATA(Adapter)->ring_dma_addr = HAL_VIRT_TO_BUS(page_ptr);
#endif

#ifdef __MIPSEB__
		page_ptr = (unsigned char *)KSEG1ADDR(page_ptr);
#endif

		_GET_HAL_DATA(Adapter)->ring_virt_addr = (u4Byte)page_ptr;
        printk("page_ptr=%08x, size=%d, ring_dma_addr:%08x, alloc_dma_buf:%08x, ring_virt_addr:%08x \n",
            (u4Byte)page_ptr, dma_len, _GET_HAL_DATA(Adapter)->ring_dma_addr,  
            _GET_HAL_DATA(Adapter)->alloc_dma_buf, _GET_HAL_DATA(Adapter)->ring_virt_addr );
    }
#endif

    ptx_dma = (PHCI_TX_DMA_MANAGER_88XX)HALMalloc(Adapter, sizeof(HCI_TX_DMA_MANAGER_88XX));
    if (NULL == ptx_dma) {
        RT_TRACE_F( COMP_INIT, DBG_SERIOUS, ("Allocate HAL Memory-TX Failed\n") );
        return RT_STATUS_FAILURE;
    }
    prx_dma = (PHCI_RX_DMA_MANAGER_88XX)HALMalloc(Adapter, sizeof(HCI_RX_DMA_MANAGER_88XX));
    if (NULL == prx_dma) {
        HAL_free(ptx_dma);
        RT_TRACE_F( COMP_INIT, DBG_SERIOUS, ("Allocate HAL Memory-RX Failed\n") );
        return RT_STATUS_FAILURE;
    }
    else {
        PlatformZeroMemory(ptx_dma, sizeof(HCI_TX_DMA_MANAGER_88XX));
        PlatformZeroMemory(prx_dma, sizeof(HCI_RX_DMA_MANAGER_88XX));
    }

    //Register to HAL_DATA
    _GET_HAL_DATA(Adapter)->PTxDMA88XX = ptx_dma;
    _GET_HAL_DATA(Adapter)->PRxDMA88XX = prx_dma;

    return RT_STATUS_SUCCESS;
}


RT_STATUS
InitHCIDMAReg88XX(
    IN  HAL_PADAPTER Adapter
)
{
    u4Byte                      value32 = 0;
    u4Byte                      RXBDReg;

#if IS_EXIST_RTL8192EE
    if ( IS_HARDWARE_TYPE_8192EE(Adapter) ) {
        value32 = HAL_RTL_R32(REG_PCIE_CTRL1);
        //Clear Bit
        value32 = value32 & ~((BIT_MASK_PCIE_MAX_RXDMA<<BIT_SHIFT_PCIE_MAX_RXDMA) | (BIT_MASK_PCIE_MAX_TXDMA<<BIT_SHIFT_PCIE_MAX_TXDMA));

#if RXBD_READY_CHECK_METHOD
        // RXTAG, Do Nothing, HW default value
        value32 |= BIT15;
#else
        // TOTALRXPKTSIZE
        value32 &= ~BIT15;
#endif  //RXBD_READY_CHECK_METHOD
        
        //Set Bit
        value32 |= BIT_PCIE_MAX_RXDMA(PCIE_RXDMA_BURST_SIZE) | BIT_PCIE_MAX_TXDMA(PCIE_TXDMA_BURST_SIZE) | BIT_STOP_BCNQ;

        HAL_RTL_W32(REG_PCIE_CTRL1, value32);

        // Disable TX/RX DMA pre-fetch
        HAL_RTL_W8(REG_PCIE_CTRL2, HAL_RTL_R8(REG_PCIE_CTRL2) | BIT_DIS_RXDMA_PRE | BIT_DIS_TXDMA_PRE);        
    }
#endif  //IS_EXIST_RTL8192EE

#if IS_EXIST_RTL8881AEM
    if ( IS_HARDWARE_TYPE_8881AEM(Adapter) ) {
        value32 = HAL_RTL_R32(REG_LX_CTRL1);
        //Clear Bit
        value32 = value32 & ~((BIT_MASK_MAX_RXDMA<<BIT_SHIFT_MAX_RXDMA) | (BIT_MASK_MAX_TXDMA<<BIT_SHIFT_MAX_TXDMA));
    #if RXBD_READY_CHECK_METHOD
        // RXTAG, Do Nothing, HW default value
        value32 |= BIT15;
    #else
        // TOTALRXPKTSIZE
        value32 &= ~BIT15;
    #endif  //RXBD_READY_CHECK_METHOD
        //Set Bit
        value32 |= BIT_MAX_RXDMA(LBUS_RXDMA_BURST_SIZE) | BIT_MAX_TXDMA(LBUS_TXDMA_BURST_SIZE) | BIT_STOP_BCNQ;

        HAL_RTL_W32(REG_LX_CTRL1, value32);
    }
#endif  //IS_EXIST_RTL8881AEM
    
    //3 ===Set TXBD Mode and Number===    
    HAL_RTL_W16(REG_MGQ_TXBD_NUM, BIT_MAX_MGQ_DESC_MODE(TX_MGQ_TXBD_MODE_SEL) | \
                                    BIT_MAX_MGQ_DESC_NUM(TX_MGQ_TXBD_NUM));
    HAL_RTL_W16(REG_VOQ_TXBD_NUM, BIT_MAX_VOQ_DESC_MODE(TX_VOQ_TXBD_MODE_SEL) | \
                                    BIT_MAX_VOQ_DESC_NUM(TX_VOQ_TXBD_NUM));
    HAL_RTL_W16(REG_VIQ_TXBD_NUM, BIT_MAX_VIQ_DESC_MODE(TX_VIQ_TXBD_MODE_SEL) | \
                                    BIT_MAX_VIQ_DESC_NUM(TX_VIQ_TXBD_NUM));
    HAL_RTL_W16(REG_BEQ_TXBD_NUM, BIT_MAX_BEQ_DESC_MODE(TX_BEQ_TXBD_MODE_SEL) | \
                                    BIT_MAX_BEQ_DESC_NUM(TX_BEQ_TXBD_NUM));
    HAL_RTL_W16(REG_BKQ_TXBD_NUM, BIT_MAX_BKQ_DESC_MODE(TX_BKQ_TXBD_MODE_SEL) | \
                                    BIT_MAX_BKQ_DESC_NUM(TX_BKQ_TXBD_NUM));
    
    HAL_RTL_W16(REG_HI0Q_TXBD_NUM, BIT_MAX_HI0Q_DESC_MODE(TX_HI0Q_TXBD_MODE_SEL) | \
                                    BIT_MAX_HI0Q_DESC_NUM(TX_HI0Q_TXBD_NUM));
    HAL_RTL_W16(REG_HI1Q_TXBD_NUM, BIT_MAX_HI1Q_DESC_MODE(TX_HI1Q_TXBD_MODE_SEL) | \
                                    BIT_MAX_HI1Q_DESC_NUM(TX_HI1Q_TXBD_NUM));
    HAL_RTL_W16(REG_HI2Q_TXBD_NUM, BIT_MAX_HI2Q_DESC_MODE(TX_HI2Q_TXBD_MODE_SEL) | \
                                    BIT_MAX_HI2Q_DESC_NUM(TX_HI2Q_TXBD_NUM));
    HAL_RTL_W16(REG_HI3Q_TXBD_NUM, BIT_MAX_HI3Q_DESC_MODE(TX_HI3Q_TXBD_MODE_SEL) | \
                                    BIT_MAX_HI3Q_DESC_NUM(TX_HI3Q_TXBD_NUM));
    HAL_RTL_W16(REG_HI4Q_TXBD_NUM, BIT_MAX_HI4Q_DESC_MODE(TX_HI4Q_TXBD_MODE_SEL) | \
                                    BIT_MAX_HI4Q_DESC_NUM(TX_HI4Q_TXBD_NUM));
    HAL_RTL_W16(REG_HI5Q_TXBD_NUM, BIT_MAX_HI5Q_DESC_MODE(TX_HI5Q_TXBD_MODE_SEL) | \
                                    BIT_MAX_HI5Q_DESC_NUM(TX_HI5Q_TXBD_NUM));
    HAL_RTL_W16(REG_HI6Q_TXBD_NUM, BIT_MAX_HI6Q_DESC_MODE(TX_HI6Q_TXBD_MODE_SEL) | \
                                    BIT_MAX_HI6Q_DESC_NUM(TX_HI6Q_TXBD_NUM));
    HAL_RTL_W16(REG_HI7Q_TXBD_NUM, BIT_MAX_HI7Q_DESC_MODE(TX_HI7Q_TXBD_MODE_SEL) | \
                                    BIT_MAX_HI7Q_DESC_NUM(TX_HI7Q_TXBD_NUM));

    //3 ===Set Beacon Mode: 2, 4, or 8 segment each descriptor===
    RXBDReg = HAL_RTL_R16(REG_RX_RXBD_NUM);
    RXBDReg = (RXBDReg & ~(BIT_MASK_BCNQ_DESC_MODE << BIT_SHIFT_BCNQ_DESC_MODE)) | \
                            BIT_MAX_BCNQ_DESC_MODE(TX_BCNQ_TXBD_MODE_SEL);
    HAL_RTL_W16(REG_RX_RXBD_NUM, RXBDReg);

    //3 ===Set RXBD Number===
    RXBDReg = (RXBDReg & ~BIT_MASK_RXQ_DESC_NUM) | BIT_MAX_RXQ_DESC_NUM(RX_Q_RXBD_NUM);
    HAL_RTL_W16(REG_RX_RXBD_NUM, RXBDReg);

    //3 ===Set 32Bit / 64 Bit System===
    RXBDReg = (RXBDReg & ~BIT_SYS_32_64) | (TXBD_SEG_32_64_SEL << BIT_SHIFT_SYS_32_64);
    HAL_RTL_W16(REG_RX_RXBD_NUM, RXBDReg);


#if IS_EXIST_RTL8881AEM
    if ( IS_HARDWARE_TYPE_8881AEM(Adapter) ) {
        value32 = HAL_RTL_R32(REG_LX_CTRL1);
        value32 = value32 & (~BIT_STOP_RXQ);
        HAL_RTL_W32(REG_LX_CTRL1, value32);
    }
#endif
        
#if IS_EXIST_RTL8192EE
    if ( IS_HARDWARE_TYPE_8192EE(Adapter) ) {
        value32 = HAL_RTL_R32(REG_PCIE_CTRL1);
        //value32 = value32 & (~BIT_STOP_RXQ);
        value32 = value32 & (~0x7fff);
        HAL_RTL_W32(REG_PCIE_CTRL1, value32);
    }        
#endif

    return RT_STATUS_SUCCESS;
}


VOID
StopHCIDMASW88XX(
    IN  HAL_PADAPTER Adapter
)
{
#ifdef CONFIG_NET_PCI
    if (((Adapter->pshare->type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS && _GET_HAL_DATA(Adapter)->alloc_dma_buf != NULL) {
        unsigned int dma_len= DESC_DMA_PAGE_SIZE_MAX;

        u4Byte page_align_phy = (HAL_PAGE_SIZE - (((u4Byte)_GET_HAL_DATA(Adapter)->alloc_dma_buf) & (HAL_PAGE_SIZE-1)));
   
	    pci_free_consistent(Adapter->pshare->pdev, dma_len, (void*)_GET_HAL_DATA(Adapter)->alloc_dma_buf,
				(dma_addr_t)((_GET_HAL_DATA(Adapter)->ring_dma_addr)-page_align_phy));
    }
#endif
    
    //Free TRX DMA Manager Memory
    if ( _GET_HAL_DATA(Adapter)->PTxDMA88XX ) {
        HAL_free(_GET_HAL_DATA(Adapter)->PTxDMA88XX);
    }

    if ( _GET_HAL_DATA(Adapter)->PRxDMA88XX ) {
        HAL_free(_GET_HAL_DATA(Adapter)->PRxDMA88XX);
    }   
}


VOID
StopHCIDMAHW88XX(
    IN  HAL_PADAPTER Adapter
)
{
    u4Byte     value32 ;
    
    //TRX DMA Stop
#if IS_EXIST_RTL8881AEM
    if ( IS_HARDWARE_TYPE_8881AEM(Adapter) ) {
        value32 = HAL_RTL_R32(REG_LX_CTRL1);
        value32 = value32 | (0x7FFF);
        HAL_RTL_W32(REG_LX_CTRL1, value32);
    }
#endif
        
#if IS_EXIST_RTL8192EE
    if ( IS_HARDWARE_TYPE_8192EE(Adapter) ) {
        value32 = HAL_RTL_R32(REG_PCIE_CTRL1);
        value32 = value32 | (0x7FFF);
        HAL_RTL_W32(REG_PCIE_CTRL1, value32);
    }        
#endif

    //Sugested by DD-TimChen
    // Reason: make sure thar TRX DMA operation is done (To avoid transaction error in LBUS)
    HAL_delay_ms(5);

}



#if CFG_HAL_SUPPORT_MBSSID
VOID
InitMBSSID88XX(
    IN  HAL_PADAPTER Adapter
)
{
    s4Byte      i, j;
    u4Byte      camData[2];
    pu1Byte     macAddr = HAL_VAR_MY_HWADDR;
    u4Byte      bcn_early_time ;
    u4Byte      tbtt_hold;
    u4Byte      vap_bcn_offset;

	if (HAL_IS_ROOT_INTERFACE(Adapter))
	{
		camData[0] = MBIDCAM_POLL | MBIDWRITE_EN | MBIDCAM_VALID | (macAddr[5] << 8) | macAddr[4];
		camData[1] = (macAddr[3] << 24) | (macAddr[2] << 16) | (macAddr[1] << 8) | macAddr[0];

		for (j=1; j>=0; j--) {
			HAL_RTL_W32((REG_MBIDCAMCFG+4)-4*j, camData[j]);
		}

		// clear the rest area of CAM
		camData[1] = 0;
		for (i=1; i<8; i++) {
			camData[0] = MBIDCAM_POLL | MBIDWRITE_EN | (i&MBIDCAM_ADDR_Mask)<<MBIDCAM_ADDR_SHIFT;
			for (j=1; j>=0; j--) {
				HAL_RTL_W32((REG_MBIDCAMCFG+4)-4*j, camData[j]);
			}
		}

		// set MBIDCTRL & MBID_BCN_SPACE by cmd
		HAL_RTL_W32(REG_MBSSID_BCN_SPACE,
			(HAL_VAR_BCN_INTERVAL & BCN_SPACE2_Mask)<<BCN_SPACE2_SHIFT
			|(HAL_VAR_BCN_INTERVAL & BCN_SPACE1_Mask)<<BCN_SPACE1_SHIFT);

        HAL_RTL_W8(REG_HIQ_NO_LMT_EN, 0xff);
		HAL_RTL_W8(REG_BCN_CTRL, 0);
		HAL_RTL_W8(REG_DUAL_TSF_RST, 1);

        HAL_RTL_W8(REG_BCN_CTRL, EN_BCN_FUNCTION | DIS_SUB_STATE_N | DIS_TSF_UPDATE_N|EN_TXBCN_RPT);

		HAL_RTL_W32(REG_RCR, HAL_RTL_R32(REG_RCR) | RCR_MBID_EN);	// MBSSID enable
		
	}
	else if (HAL_IS_VAP_INTERFACE(Adapter))
	{
		HAL_VAR_VAP_INIT_SEQ = HAL_RTL_R8(REG_MBID_NUM) & MBID_BCN_NUM_Mask;


    // Add odd number of AP issue, If previous time add more one AP, decrease one.
    if ((HAL_RTL_R8(REG_MBSSID_CTRL) & (1<<HAL_VAR_VAP_INIT_SEQ)) == 0)
	{
		HAL_VAR_VAP_INIT_SEQ--;
	}

		HAL_VAR_VAP_INIT_SEQ++;

        RT_TRACE(COMP_INIT, DBG_LOUD, ("init swq=%d\n", HAL_VAR_VAP_INIT_SEQ));

		switch (HAL_VAR_VAP_INIT_SEQ)
		{
			case 1:
				HAL_RTL_W16(REG_ATIMWND1, 0x3C);		
				HAL_RTL_W8(REG_DTIM_COUNTER_VAP1, HAL_VAR_DTIM_PERIOD-1);
                HAL_RTL_W16(REG_FUNCTRL, FUNCTRL_ADDR_CNT1CTRL);
				HAL_RTL_W8(REG_CNT_FORMAT, CNT_EN|(CNT_BSSIDMAP_CNT1<<CNT_BSSIDMAP_SHIFT));
				break;
			case 2:
				HAL_RTL_W8(REG_ATIMWND2, 0x3C);
				HAL_RTL_W8(REG_DTIM_COUNTER_VAP2, HAL_VAR_DTIM_PERIOD-1);
                HAL_RTL_W16(REG_FUNCTRL, FUNCTRL_ADDR_CNT2CTRL);
				HAL_RTL_W8(REG_CNT_FORMAT, CNT_EN|(CNT_BSSIDMAP_CNT2<<CNT_BSSIDMAP_SHIFT));                
				break;
			case 3:
				HAL_RTL_W8(REG_ATIMWND3, 0x3C);
				HAL_RTL_W8(REG_DTIM_COUNTER_VAP3, HAL_VAR_DTIM_PERIOD-1);
                HAL_RTL_W16(REG_FUNCTRL, FUNCTRL_ADDR_CNT3CTRL);
				HAL_RTL_W8(REG_CNT_FORMAT, CNT_EN|(CNT_BSSIDMAP_CNT3<<CNT_BSSIDMAP_SHIFT));                
				break;
			case 4:
				HAL_RTL_W8(REG_ATIMWND4, 0x3C);
				HAL_RTL_W8(REG_DTIM_COUNTER_VAP4, HAL_VAR_DTIM_PERIOD-1);
                HAL_RTL_W16(REG_FUNCTRL, FUNCTRL_ADDR_CNT4CTRL);
				HAL_RTL_W8(REG_CNT_FORMAT, CNT_EN|(CNT_BSSIDMAP_CNT4<<CNT_BSSIDMAP_SHIFT));                
				break;
			case 5:
				HAL_RTL_W8(REG_ATIMWND5, 0x3C);
				HAL_RTL_W8(REG_DTIM_COUNTER_VAP5, HAL_VAR_DTIM_PERIOD-1);
                HAL_RTL_W16(REG_FUNCTRL, FUNCTRL_ADDR_CNT5CTRL);
				HAL_RTL_W8(REG_CNT_FORMAT, CNT_EN|(CNT_BSSIDMAP_CNT5<<CNT_BSSIDMAP_SHIFT));
				break;
			case 6:
				HAL_RTL_W8(REG_ATIMWND6, 0x3C);
				HAL_RTL_W8(REG_DTIM_COUNTER_VAP6, HAL_VAR_DTIM_PERIOD-1);
                HAL_RTL_W16(REG_FUNCTRL, FUNCTRL_ADDR_CNT6CTRL);
				HAL_RTL_W8(REG_CNT_FORMAT, CNT_EN|(CNT_BSSIDMAP_CNT6<<CNT_BSSIDMAP_SHIFT));
				break;
			case 7:
				HAL_RTL_W8(REG_ATIMWND7, 0x3C);
				HAL_RTL_W8(REG_DTIM_COUNTER_VAP7, HAL_VAR_DTIM_PERIOD-1);
                HAL_RTL_W16(REG_FUNCTRL, FUNCTRL_ADDR_CNT7CTRL);
				HAL_RTL_W8(REG_CNT_FORMAT, CNT_EN|(CNT_BSSIDMAP_CNT7<<CNT_BSSIDMAP_SHIFT));
				break;
            default:
                RT_TRACE(COMP_INIT, DBG_WARNING, ("Invalid init swq=%d\n", HAL_VAR_VAP_INIT_SEQ));
                break;
		}
		
		camData[0] = MBIDCAM_POLL | MBIDWRITE_EN | MBIDCAM_VALID |
				(HAL_VAR_VAP_INIT_SEQ & MBIDCAM_ADDR_Mask)<<MBIDCAM_ADDR_SHIFT |
				(macAddr[5] << 8) | macAddr[4];
		camData[1] = (macAddr[3] << 24) | (macAddr[2] << 16) | (macAddr[1] << 8) | macAddr[0];
		for (j=1; j>=0; j--) {
			HAL_RTL_W32((REG_MBIDCAMCFG+4)-4*j, camData[j]);
		}

        // if odd number of AP, open one more AP.
		if ((HAL_VAR_VAP_INIT_SEQ % 2) == 0) {
			vap_bcn_offset = HAL_VAR_BCN_INTERVAL/(HAL_VAR_VAP_INIT_SEQ+2);
		}
		else {
			vap_bcn_offset = HAL_VAR_BCN_INTERVAL/(HAL_VAR_VAP_INIT_SEQ+1);
		}
		if (vap_bcn_offset > 200)
			vap_bcn_offset = 200;
		HAL_RTL_W32(REG_MBSSID_BCN_SPACE, (vap_bcn_offset & BCN_SPACE2_Mask)<<BCN_SPACE2_SHIFT
			|(HAL_VAR_BCN_INTERVAL & BCN_SPACE1_Mask)<<BCN_SPACE1_SHIFT);


		HAL_RTL_W8(REG_BCN_CTRL, 0);
		HAL_RTL_W8(REG_DUAL_TSF_RST, 1);

        HAL_RTL_W8(REG_BCN_CTRL, EN_BCN_FUNCTION | DIS_SUB_STATE_N | DIS_TSF_UPDATE_N|EN_TXBCN_RPT);

        // if odd number of AP, open one more AP. add close this additional AP
        if ((HAL_VAR_VAP_INIT_SEQ % 2) == 0) {
            HAL_RTL_W8(REG_MBID_NUM, (HAL_RTL_R8(REG_MBID_NUM) & ~MBID_BCN_NUM_Mask) | ((HAL_VAR_VAP_INIT_SEQ+1) & MBID_BCN_NUM_Mask));
            HAL_RTL_W8(REG_MBSSID_CTRL, (HAL_RTL_R8(REG_MBSSID_CTRL) | (1 << (HAL_VAR_VAP_INIT_SEQ))));
            HAL_RTL_W8(REG_MBSSID_CTRL, (HAL_RTL_R8(REG_MBSSID_CTRL) & (~(1 << (HAL_VAR_VAP_INIT_SEQ+1)))));
        }
        else {
        HAL_RTL_W8(REG_MBID_NUM, (HAL_RTL_R8(REG_MBID_NUM) & ~MBID_BCN_NUM_Mask) | (HAL_VAR_VAP_INIT_SEQ & MBID_BCN_NUM_Mask));
            HAL_RTL_W8(REG_MBSSID_CTRL, (HAL_RTL_R8(REG_MBSSID_CTRL) | (1 << (HAL_VAR_VAP_INIT_SEQ))));
        }
        {
            bcn_early_time = HAL_RTL_R8(REG_DRVERLYINT);

            if ((HAL_VAR_VAP_INIT_SEQ % 2) == 0) {
                tbtt_hold = (HAL_VAR_BCN_INTERVAL/(HAL_VAR_VAP_INIT_SEQ+2))*2 - bcn_early_time -2;
            }
            else {
                tbtt_hold = (HAL_VAR_BCN_INTERVAL/(HAL_VAR_VAP_INIT_SEQ+1))*2 - bcn_early_time -2;
            }
            if (tbtt_hold > 16)
                tbtt_hold = 16;
            
            HAL_RTL_W32(REG_TBTT_PROHIBIT, HAL_RTL_R8(REG_TBTT_PROHIBIT) | (((tbtt_hold*1024/32)& TBTT_prohibit_hold_Mask)<<TBTT_prohibit_hold_SHT));            
        }

        RT_TRACE(COMP_INIT, DBG_LOUD, ("REG_MBID_NUM(0x%x),HAL_VAR_VAP_INIT_SEQ(0x%x)\n", HAL_RTL_R8(REG_MBID_NUM), HAL_VAR_VAP_INIT_SEQ));
        
		HAL_RTL_W32(REG_RCR, HAL_RTL_R32(REG_RCR) & ~RCR_MBID_EN);
		HAL_RTL_W32(REG_RCR, HAL_RTL_R32(REG_RCR) | RCR_MBID_EN);	// MBSSID enable
	}
}


VOID
StopMBSSID88XX(
    IN  HAL_PADAPTER Adapter
)
{
    s4Byte          i, j;
    u4Byte          camData[2];
    
    camData[1] = 0;

    if (HAL_IS_ROOT_INTERFACE(Adapter))
    {
        // clear the rest area of CAM
        for (i=0; i<8; i++) {
            camData[0] = MBIDCAM_POLL | MBIDWRITE_EN | (i&MBIDCAM_ADDR_Mask)<<MBIDCAM_ADDR_SHIFT;
            for (j=1; j>=0; j--) {
                HAL_RTL_W32((REG_MBIDCAMCFG+4)-4*j, camData[j]);
            }
        }

        HAL_RTL_W32(REG_RCR, HAL_RTL_R32(REG_RCR) & ~RCR_MBID_EN);  // MBSSID disable
        HAL_RTL_W32(REG_MBSSID_BCN_SPACE,
            (HAL_VAR_BCN_INTERVAL & BCN_SPACE1_Mask)<<BCN_SPACE1_SHIFT);

        HAL_RTL_W8(REG_BCN_CTRL, 0);
        HAL_RTL_W8(0x553, 1);
        HAL_RTL_W8(REG_BCN_CTRL, EN_BCN_FUNCTION | DIS_SUB_STATE_N | DIS_TSF_UPDATE_N| EN_TXBCN_RPT);

    }
    else if (HAL_IS_VAP_INTERFACE(Adapter) && (HAL_VAR_VAP_INIT_SEQ >= 0))
    {
        camData[0] = MBIDCAM_POLL | MBIDWRITE_EN |
                        (HAL_VAR_VAP_INIT_SEQ & MBIDCAM_ADDR_Mask)<<MBIDCAM_ADDR_SHIFT;
        for (j=1; j>=0; j--) {
            HAL_RTL_W32((REG_MBIDCAMCFG+4)-4*j, camData[j]);
        }

        if (HAL_RTL_R8(REG_MBID_NUM) & MBID_BCN_NUM_Mask) {
            HAL_RTL_W8(REG_MBID_NUM, ((HAL_RTL_R8(REG_MBID_NUM) & MBID_BCN_NUM_Mask)-1) & MBID_BCN_NUM_Mask);
            
            HAL_RTL_W32(REG_MBSSID_BCN_SPACE,
            ((HAL_VAR_BCN_INTERVAL-
            ((HAL_VAR_BCN_INTERVAL/((HAL_RTL_R8(REG_MBID_NUM) & MBID_BCN_NUM_Mask)+1))*(HAL_RTL_R8(REG_MBID_NUM)&MBID_BCN_NUM_Mask)))
            & BCN_SPACE2_Mask)<<BCN_SPACE2_SHIFT
            |((HAL_VAR_BCN_INTERVAL/((HAL_RTL_R8(REG_MBID_NUM) & MBID_BCN_NUM_Mask)+1)) & BCN_SPACE1_Mask)
            <<BCN_SPACE1_SHIFT);

            HAL_RTL_W8(REG_BCN_CTRL, 0);
            HAL_RTL_W8(0x553, 1);
            HAL_RTL_W8(REG_BCN_CTRL, EN_BCN_FUNCTION | DIS_SUB_STATE_N | DIS_TSF_UPDATE_N| EN_TXBCN_RPT);
        }
        
        HAL_RTL_W32(REG_RCR, HAL_RTL_R32(REG_RCR) & ~RCR_MBID_EN);
        HAL_RTL_W32(REG_RCR, HAL_RTL_R32(REG_RCR) | RCR_MBID_EN);
        HAL_VAR_VAP_INIT_SEQ = -1;
    }
}
#endif  //CFG_HAL_SUPPORT_MBSSID


RT_STATUS
StopHW88XX(
    IN  HAL_PADAPTER Adapter
)
{   
    HAL_RTL_W32(REG_HIMR0, 0);
    HAL_RTL_W32(REG_HIMR1, 0);

    //MCU reset
    HAL_RTL_W16(REG_SYS_FUNC_EN, HAL_RTL_R16(REG_SYS_FUNC_EN) & ~BIT10);

    //Stop HCI DMA
    StopHCIDMAHW88XX(Adapter);

#if IS_EXIST_RTL8192EE
    if ( IS_HARDWARE_TYPE_8192EE(Adapter) ) {
        HalPwrSeqCmdParsing88XX(Adapter, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, 
            PWR_INTF_PCI_MSK, rtl8192E_enter_lps_flow);
    }
#endif  //IS_EXIST_RTL8192EE
    
#if IS_EXIST_RTL8881AEM
    if ( IS_HARDWARE_TYPE_8881AEM(Adapter) ) {
        HalPwrSeqCmdParsing88XX(Adapter, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, 
            PWR_INTF_PCI_MSK, rtl8881A_enter_lps_flow);
    }
#endif  //IS_EXIST_RTL8881AEM

#if IS_EXIST_RTL8192EE
        if ( IS_HARDWARE_TYPE_8192EE(Adapter) ) {
            RT_TRACE(COMP_INIT, DBG_LOUD, ("rtl8192E_card_disable_flow\n"));
            HalPwrSeqCmdParsing88XX(Adapter, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_PCI_MSK, rtl8192E_card_disable_flow);            
        }
#endif  //IS_EXIST_RTL8192EE

#if IS_EXIST_RTL8881AEM
        if ( IS_HARDWARE_TYPE_8881AEM(Adapter) ) {
            RT_TRACE(COMP_INIT, DBG_LOUD, ("rtl8881A_card_disable_flow\n"));
            HalPwrSeqCmdParsing88XX(Adapter, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_PCI_MSK, rtl8881A_card_disable_flow);
        }
#endif  //IS_EXIST_RTL8881AEM

    // Reset IO Wraper
    HAL_RTL_W8(REG_RSV_CTRL+1, HAL_RTL_R8(REG_RSV_CTRL+1) & ~BIT(3));
    HAL_RTL_W8(REG_RSV_CTRL+1, HAL_RTL_R8(REG_RSV_CTRL+1) | BIT(3));

    HAL_RTL_W8(REG_RSV_CTRL, 0x0e);                // lock ISO/CLK/Power control register
    
    return RT_STATUS_SUCCESS;
}


RT_STATUS
StopSW88XX(
    IN  HAL_PADAPTER Adapter
)
{
    StopHCIDMASW88XX(Adapter);
}


VOID
DisableVXDAP88XX(
    IN  HAL_PADAPTER Adapter
)
{
	HAL_DATA_TYPE	*pHalData   = _GET_HAL_DATA(Adapter);    
    RT_OP_MODE      OP_Mode     = RT_OP_MODE_NO_LINK;


    pHalData->IntMask[0] &= ~(BIT_BCNDMAINT0 | BIT_TXBCNOK | BIT_TXBCNERR);
    HAL_RTL_W32(REG_HIMR0, pHalData->IntMask[0]);
    
    GET_HAL_INTERFACE(Adapter)->SetHwRegHandler(Adapter, HW_VAR_MEDIA_STATUS, (pu1Byte)&OP_Mode);
}
  




