#ifndef __HALDEF_H__
#define __HALDEF_H__
/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	HalComDef.h
	
Abstract:
	Defined HAL common data structure & Define
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-03-23 Filen            Create.	
--*/


typedef enum _HW_VARIABLES{
        HW_VAR_ETHER_ADDR,
        HW_VAR_MULTICAST_REG,
        HW_VAR_BSSID,
        HW_VAR_MAC_IO_ENABLE,       //Set Only
        HW_VAR_MACREGFILE_START,    //Get Only
        HW_VAR_MACREGFILE_SIZE,     //Get Only
        HW_VAR_PHYREGFILE_START,    //Get Only
        HW_VAR_PHYREGFILE_SIZE,     //Get Only        
        HW_VAR_PHYREGFILE_1T_START,    //Get Only
        HW_VAR_PHYREGFILE_1T_SIZE,     //Get Only         
        HW_VAR_PHYREGFILE_MP_START, //Get Only
        HW_VAR_PHYREGFILE_MP_SIZE,  //Get Only 
        HW_VAR_PHYREGFILE_PG_START, //Get Only
        HW_VAR_PHYREGFILE_PG_SIZE,  //Get Only         
        HW_VAR_PHYREGFILE_AGC_START, //Get Only
        HW_VAR_PHYREGFILE_AGC_SIZE,  //Get Only        
        HW_VAR_RFREGFILE_RADIO_A_START, //Get Only
        HW_VAR_RFREGFILE_RADIO_A_SIZE,  //Get Only             
        HW_VAR_RFREGFILE_RADIO_B_START, //Get Only
        HW_VAR_RFREGFILE_RADIO_B_SIZE,  //Get Only        
        HW_VAR_FWFILE_START,        //Get Only
        HW_VAR_FWFILE_SIZE,         //Get Only        
        HW_VAR_TXPKTFWFILE_START,        //Get Only
        HW_VAR_TXPKTFWFILE_SIZE,         //Get Only
        HW_VAR_MEDIA_STATUS,
        HW_VAR_MAC_LOOPBACK_ENABLE, //Set Only
        HW_VAR_MAC_CONFIG,          //Set Only
        HW_VAR_EDCA,                //Set Only
        HW_VAR_CAM_RESET_ALL_ENTRY, //Set Only
        HW_VAR_SECURITY_CONFIG,
        HW_VAR_BEACON_INTERVAL,
        HW_VAR_ENABLE_BEACON_DMA,
        HW_VAR_TXPAUSE,
        HW_VAR_HIQ_NO_LMT_EN,
        HW_VAR_DRV_DBG,
        HW_VAR_NUM_TOTAL_RF_PATH,    //Get Only
        HW_VAR_NUM_RXDMA_STATUS,
        HW_VAR_NUM_TXDMA_STATUS,
}HW_VARIABLES;


// The type used to query whether the interrupt in HAL is toggled.
typedef enum _HAL_INT_TYPE
{
	HAL_INT_TYPE_ANY,				// Any interrupt
	HAL_INT_TYPE_TBDOK,				// Tx Beacon OK
	HAL_INT_TYPE_TBDER,				// Tx Beacon error
	HAL_INT_TYPE_BcnInt,			// For 92C or later, it should be early beacon interrupt.
	HAL_INT_TYPE_PSTIMEOUT,			// PS timer interrupt by TSF
	HAL_INT_TYPE_PSTIMEOUT1,		// PS timer 1 interrupt by TSF	
	HAL_INT_TYPE_PSTIMEOUT2,		// PS timer 2 interrupt by TSF	
	HAL_INT_TYPE_C2HCMD,			// CPU to Host Command INT Status interrupt
	HAL_INT_TYPE_RXFOVW,			// Rx FIFO over flow
	HAL_INT_TYPE_VIDOK,				// VI queue DMA OK
	HAL_INT_TYPE_VODOK,				// VO queue DMA OK
	HAL_INT_TYPE_BEDOK,				// BE queue DMA OK
	HAL_INT_TYPE_BKDOK,				// BK queue DMA OK
	HAL_INT_TYPE_MGNTDOK,			// Mgnt queue DMA OK
	HAL_INT_TYPE_HIGHDOK,			// High queue DMA OK
	HAL_INT_TYPE_BDOK,				// Beacon queue DMA OK  , Note: Filen, this interrupt has removed in 8812 & later chip
	HAL_INT_TYPE_CPWM,				// CPU power Mode exchange INT Status
	HAL_INT_TYPE_TSF_BIT32_TOGGLE,	// TSF Timer BIT32 toggle indication interrupt
	HAL_INT_TYPE_RX_OK,				// Receive DMA OK
	HAL_INT_TYPE_RDU,
	HAL_INT_TYPE_BcnInt_MBSSID,		// For 92C or later, it should be early beacon interrupt.	
	HAL_INT_TYPE_BcnInt1,
	HAL_INT_TYPE_BcnInt2,
	HAL_INT_TYPE_BcnInt3,
	HAL_INT_TYPE_BcnInt4,
	HAL_INT_TYPE_BcnInt5,
	HAL_INT_TYPE_BcnInt6,
	HAL_INT_TYPE_BcnInt7,
	HAL_INT_TYPE_CTWEND,
	HAL_INT_TYPE_BCNDERR0,
	HAL_INT_TYPE_TXFOVW,            // Transmit packet buffer Overflow.
	HAL_INT_TYPE_RXERR,             // Rx Error Flag INT Status
	HAL_INT_TYPE_TXERR,             // Tx Error Flag INT Status
	#if 0   //Filen: Not used to AP Platform
	//==== SDIO Specified Interrupt=====//
	HAL_INT_TYPE_SDIO_ISR_IND,
	HAL_INT_TYPE_SDIO_GPIO12_0_INT,
	HAL_INT_TYPE_SDIO_SPS_OCP_INT,
	HAL_INT_TYPE_SDIO_RON_INT_EN,
	HAL_INT_TYPE_SDIO_PDNINT,
	HAL_INT_TYPE_SDIO_GPIO9_INT,
	#endif
}HAL_INT_TYPE, *PHAL_INT_TYPE;


typedef struct _MACCONFIG_PARA_ {
    u4Byte     AckTO;
    u4Byte     vap_enable;
    u4Byte     OP_Mode;
    u2Byte     dot11DTIMPeriod;
    // TODO:
} MACCONFIG_PARA, *PMACCONFIG_PARA;

typedef struct _EDCA_PARA_ {
    u4Byte              slot_time;
    u4Byte              sifs_time;
    struct ParaRecord   Para[AC_PARAM_SIZE];
}EDCA_PARA, *PEDCA_PARA;

typedef struct _CAM_ENTRY_CFG_ {
    BOOLEAN             bValid;
    u1Byte              KeyID;
    DOT11_ENC_ALGO      EncAlgo;
}CAM_ENTRY_CFG, *PCAM_ENTRY_CFG;


//-----------------------------------------------------------
//
//	Queue mapping
//
//-----------------------------------------------------------
//1.) used to TXPOLL
#define TXPOLL_BK_QUEUE						0
#define TXPOLL_BE_QUEUE						1
#define TXPOLL_VI_QUEUE						2
#define TXPOLL_VO_QUEUE						3
#define TXPOLL_BEACON_QUEUE					4
//#define TXPOLL_TXCMD_QUEUE				5
#define TXPOLL_MGNT_QUEUE					6
#define TXPOLL_HIGH_QUEUE					7
#define TXPOLL_HCCA_QUEUE					8

//2.) used to TXPAUSE
#define TXPAUSE_BK_QUEUE_BIT                    BIT0
#define TXPAUSE_BE_QUEUE_BIT                    BIT1
#define TXPAUSE_VI_QUEUE_BIT                    BIT2
#define TXPAUSE_VO_QUEUE_BIT                    BIT3
#define TXPAUSE_MGNT_QUEUE_BIT                  BIT4
#define TXPAUSE_HIGH_QUEUE_BIT                  BIT5
#define TXPAUSE_BCN_QUEUE_BIT                   BIT6
#define TXPAUSE_BCN_HI_MGNT_QUEUE_BIT           BIT7

#define TXPAUSE_ALL_QUEUE_BIT                   0xFF


//3 Initialization Related
typedef RT_STATUS
(*NicInitPONHandler)(
    INPUT	HAL_PADAPTER		Adapter
    );

typedef RT_STATUS
(*NicInitMACHandler)(
    INPUT	HAL_PADAPTER		Adapter
    );

typedef VOID
(*NicInitIMRHandler)(
    INPUT	HAL_PADAPTER		Adapter,
    INPUT   RT_OP_MODE          OPMode
    );

typedef RT_STATUS
(*NicInitFirmwareHandler)(
    INPUT	HAL_PADAPTER		Adapter
    );

typedef RT_STATUS
(*NicInitHCIDMAMemHandler)(
    INPUT	HAL_PADAPTER		Adapter
    );

typedef RT_STATUS
(*NicInitHCIDMARegHandler)(
    INPUT	HAL_PADAPTER		Adapter
    );

typedef VOID
(*NicInitMBSSIDHandler)(
    INPUT	HAL_PADAPTER		Adapter
    );

typedef VOID
(*NicStopMBSSIDHandler)(
    INPUT	HAL_PADAPTER		Adapter
    );

typedef VOID
(*NicInitVAPIMRHandler)(
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          VapSeq
    );


//3 Stop Related
typedef VOID
(*NicDisableVXDAPHandler)(
    INPUT	HAL_PADAPTER		Adapter
    );


//3 ISR Related 
typedef VOID
(*NicEnableIMRHandler)(
    INPUT	HAL_PADAPTER		Adapter
    );

typedef BOOLEAN
(*NicInterruptRecognizedHandler)(
    INPUT   HAL_PADAPTER    Adapter,
    INPUT   PVOID           pContent,
    INPUT   u4Byte          ContentLen
	);

typedef BOOLEAN
(*NicGetInterruptHandler)(
    IN  HAL_PADAPTER    Adapter,
    IN  HAL_INT_TYPE	intType
	);

typedef VOID
(*NicAddInterruptMaskHandler)(
    IN  HAL_PADAPTER    Adapter,
    IN  HAL_INT_TYPE	intType
	);

typedef VOID
(*NicRemoveInterruptMaskHandler)(
    IN  HAL_PADAPTER    Adapter,
    IN  HAL_INT_TYPE	intType
	);

typedef VOID
(*NicDisableRxRelatedInterruptHandler)(
    IN  HAL_PADAPTER    Adapter
	);

typedef VOID
(*NicEnableRxRelatedInterruptHandler)(
    IN  HAL_PADAPTER    Adapter
	);


//3 Tx Related
typedef RT_STATUS
(*NicPrepareTxBufferDescriptorHandler)(
    INPUT	HAL_PADAPTER		Adapter
    );

typedef VOID
(*NicTxPollingHandler)(
	INPUT	HAL_PADAPTER		Adapter,
	INPUT	u1Byte  			QueueIndex
	);

typedef VOID
(*NicSigninBeaconTXBDHandler)(
    INPUT	HAL_PADAPTER        Adapter,
    INPUT   pu4Byte             beaconbuf,
    INPUT   u2Byte              frlen
	);

typedef u2Byte
(*NicGetTxQueueHWIdxHandler)(
    IN	HAL_PADAPTER        Adapter,
    IN  u4Byte              q_num       //enum _TX_QUEUE_
);

typedef u4Byte
(*NicMappingTxQueueHandler)(
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          TxQNum      //enum _TX_QUEUE_
	);

typedef BOOLEAN
(*NicQueryTxConditionMatchHandler)(
    IN	HAL_PADAPTER    Adapter
    );

typedef BOOLEAN
(*NicFillTxHwCtrlHandler)(
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN  PVOID           pDescData
    );

typedef RT_STATUS
(*NicSyncSWTXBDHostIdxToHWHandler) (
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          queueIndex  //HCI_TX_DMA_QUEUE_88XX
    );

typedef PVOID
(*NicGetShortCutTxDescriptorHandler) (
    IN  HAL_PADAPTER    Adapter
    );

typedef VOID
(*NicReleaseShortCutTxDescriptorHandler)(
    IN  HAL_PADAPTER    Adapter,
    IN  PVOID           pTxDesc
    );

typedef VOID
(*NicSetShortCutTxBuffSizeHandler) (
    IN  HAL_PADAPTER    Adapter,
    IN  PVOID           pTxDesc,
    IN  u2Byte          txPktSize
    );

typedef u2Byte
(*NicGetShortCutTxBuffSizeHandler)(
    IN  HAL_PADAPTER    Adapter,
    IN  PVOID           pTxDesc
    );

typedef PVOID
(*NicCopyShortCutTxDescriptorHandler)(
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          queueIndex, //HCI_TX_DMA_QUEUE_88XX
    IN  PVOID           pTxDesc,
    IN  u4Byte          direction    
    );

typedef BOOLEAN
(*NicFillShortCutTxHwCtrlHandler)(
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN  PVOID           pDescData,
    IN  PVOID           pTxDesc,
    IN  u4Byte          direction
    );

//3 Rx Related
typedef RT_STATUS
(*NicPrepareRxBufferDescriptorHandler)(
    INPUT	HAL_PADAPTER		Adapter,
    INPUT   u2Byte              bufferLen,
    INPUT   PVOID               Callback
    );

typedef RT_STATUS
(*NicQueryRxDescriptorHandler) (
    INPUT   HAL_PADAPTER    Adapter,
    INPUT   u4Byte          queueIndex,
    INPUT   pu1Byte         pBufAddr,
    OUTPUT  PVOID           pRxDescStatus    
    );

typedef VOID 
(*NicUpdateRxBufferDescriptorInfoHandler)(
    IN      HAL_PADAPTER    Adapter,
    IN      u4Byte          queueIndex,  //HCI_RX_DMA_QUEUE_88XX
    IN      u1Byte          rxbd_idx,
    IN      pu1Byte         pBuf,
    IN      PVOID           Callback,    // callback function    
    IN      BOOLEAN         bInit
);

typedef u4Byte	
(*NicReadableRxBufferDescCountHandler)(
    INPUT   HAL_PADAPTER	    Adapter,
	INPUT   u4Byte		        queueIndex
    );

typedef VOID
(*NicUpdateRXBDHostIdxHandler)(
    IN      HAL_PADAPTER    Adapter,
    IN      u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN      u4Byte          Count
    );

typedef u2Byte	
(*NicUpdateRXBDHWIdxHandler)(
    IN  HAL_PADAPTER    Adapter,
	IN  u4Byte		    queueIndex  //HCI_TX_DMA_QUEUE_88XX
    );

//3 General operation
typedef MIMO_TR_STATUS
(*NicGetChipIDMIMOHandler)(
	INPUT	HAL_PADAPTER		Adapter
	);

typedef VOID
(*NicSetHwRegHandler)(
	INPUT	HAL_PADAPTER		Adapter,
	INPUT	u1Byte				RegName,
	INPUT	pu1Byte				val
	);

typedef VOID
(*NicGetHwRegHandler)(
	INPUT	HAL_PADAPTER		Adapter,
	INPUT	u1Byte				RegName,
	OUTPUT	pu1Byte				val
	);

typedef VOID
(*NicCAMReadMACConfigHandler)(
	INPUT	HAL_PADAPTER		Adapter,
    INPUT   u1Byte              index, 
    OUTPUT  pu1Byte             pMacad,
    OUTPUT  PCAM_ENTRY_CFG      pCfg
	);


//3 Security Related    
typedef VOID
(*NicCAMEmptyEntryHandler)(
	INPUT	HAL_PADAPTER		Adapter,
    INPUT   u1Byte              index
	);

typedef u4Byte
(*NicCAMFindUsableHandler)(
	INPUT	HAL_PADAPTER		Adapter,
    INPUT   u4Byte              for_begin
	);

typedef VOID
(*NicCAMProgramEntryHandler)(
	INPUT	HAL_PADAPTER		Adapter,
    INPUT   u1Byte              index,
    INPUT   pu1Byte             macad,
    INPUT   pu1Byte             key128,
    INPUT   u2Byte              config
	);

typedef RT_STATUS
(*NicStopHWHandler)(
	INPUT	HAL_PADAPTER		Adapter
	);

typedef RT_STATUS
(*NicStopSWHandler)(
	INPUT	HAL_PADAPTER		Adapter
	);


//3 Firmware CMD IO related
typedef RT_STATUS
(*NicFillH2CCmdHandler)(
	IN  HAL_PADAPTER    Adapter,
	IN	u1Byte 		    ElementID,
	IN	u4Byte 		    CmdLen,
	IN	pu1Byte		    pCmdBuffer
    );

typedef VOID
(*NicUpdateHalRAMaskHandler)(
	IN HAL_PADAPTER         Adapter,	
	HAL_PSTAINFO            pEntry,
	u1Byte				    rssi_level
    );

typedef VOID
(*NicUpdateHalMSRRPTHandler)(
	IN HAL_PADAPTER     Adapter,
	u2Byte              aid,
	u1Byte              opmode
    );

typedef VOID
(*DumpRxBDescHandler)(
	IN HAL_PADAPTER     Adapter,
	u4Byte              q_num
    );

typedef VOID
(*DumpTxBDescHandler)(
	IN HAL_PADAPTER     Adapter,
	u4Byte              q_num
    );


// 4. RF setting related

typedef VOID
(*NicPHYSetCCKTxPowerHandler)(
    IN  HAL_PADAPTER    Adapter, 
    IN  u1Byte          channel
    );

typedef VOID
(*NicPHYSetOFDMTxPowerHandler)(
    IN  HAL_PADAPTER    Adapter, 
    IN  u1Byte          channel
    );

typedef VOID
(*NicPHYUpdateBBRFValHandler)(
    IN  HAL_PADAPTER    Adapter, 
    IN  u1Byte          channel
    );

typedef struct _HAL_INTERFACE_COMMON_{


    //
    // WLAN Device operations. 
    //

    //3 Initialization Related
    NicInitPONHandler               InitPONHandler;
    NicInitMACHandler               InitMACHandler;
    NicInitIMRHandler               InitIMRHandler;
    NicInitFirmwareHandler          InitFirmwareHandler;
    NicInitHCIDMAMemHandler         InitHCIDMAMemHandler;
    NicInitHCIDMARegHandler         InitHCIDMARegHandler;    
    NicInitMBSSIDHandler            InitMBSSIDHandler;
    NicInitVAPIMRHandler            InitVAPIMRHandler;

    //3 Stop Related
    NicStopMBSSIDHandler            StopMBSSIDHandler;
    NicStopHWHandler                StopHWHandler;
    NicStopSWHandler                StopSWHandler;
    NicDisableVXDAPHandler          DisableVXDAPHandler;

    //3 ISR Related 
    NicEnableIMRHandler                     EnableIMRHandler;
    NicInterruptRecognizedHandler           InterruptRecognizedHandler;
    NicGetInterruptHandler                  GetInterruptHandler;
    NicAddInterruptMaskHandler              AddInterruptMaskHandler;
    NicRemoveInterruptMaskHandler           RemoveInterruptMaskHandler;
    NicDisableRxRelatedInterruptHandler     DisableRxRelatedInterruptHandler;
    NicEnableRxRelatedInterruptHandler      EnableRxRelatedInterruptHandler;

    //3 General operation
    NicGetChipIDMIMOHandler     GetChipIDMIMOHandler;
    NicSetHwRegHandler          SetHwRegHandler;
    NicGetHwRegHandler          GetHwRegHandler;

    //3 Security Related	
    //CAM
    NicCAMReadMACConfigHandler  CAMReadMACConfigHandler;
    NicCAMEmptyEntryHandler     CAMEmptyEntryHandler;
    NicCAMFindUsableHandler     CAMFindUsableHandler;
    NicCAMProgramEntryHandler   CAMProgramEntryHandler;      

    //3 PHY/RF Related

    //3 Firmware CMD IO related

    //
    //  Special Operation for each Chip type
    //
#if IS_RTL88XX_GENERATION

#if (DEV_BUS_TYPE & (RT_EMBEDDED_INTERFACE | RT_PCI_INTERFACE))
    //3 Tx Related
    NicTxPollingHandler                     TxPollingHandler;
    NicSigninBeaconTXBDHandler              SigninBeaconTXBDHandler;
    NicGetTxQueueHWIdxHandler               GetTxQueueHWIdxHandler;
    NicMappingTxQueueHandler                MappingTxQueueHandler;
    NicQueryTxConditionMatchHandler         QueryTxConditionMatchHandler;
    NicPrepareTxBufferDescriptorHandler     PrepareTXBDHandler;
    NicFillTxHwCtrlHandler                  FillTxHwCtrlHandler;
    NicSyncSWTXBDHostIdxToHWHandler         SyncSWTXBDHostIdxToHWHandler;
//    NicGetShortCutTxDescriptorHandler       GetShortCutTxDescHandler;
    NicReleaseShortCutTxDescriptorHandler   ReleaseShortCutTxDescHandler;
    NicGetShortCutTxBuffSizeHandler         GetShortCutTxBuffSizeHandler;
    NicSetShortCutTxBuffSizeHandler         SetShortCutTxBuffSizeHandler;
    NicCopyShortCutTxDescriptorHandler      CopyShortCutTxDescHandler;
    NicFillShortCutTxHwCtrlHandler          FillShortCutTxHwCtrlHandler;

    //3 Rx Related
    NicPrepareRxBufferDescriptorHandler     PrepareRXBDHandler;
    NicQueryRxDescriptorHandler             QueryRxDescHandler;
    NicUpdateRxBufferDescriptorInfoHandler  UpdateRXBDInfoHandler;
    NicReadableRxBufferDescCountHandler     ReadableRxBufferDescCountHandler;
    NicUpdateRXBDHWIdxHandler               UpdateRXBDHWIdxHandler;
    NicUpdateRXBDHostIdxHandler             UpdateRXBDHostIdxHandler;

    //3 Firmware CMD IO related
    NicFillH2CCmdHandler                    FillH2CCmdHandler;
    NicUpdateHalRAMaskHandler               UpdateHalRAMaskHandler;
    NicUpdateHalMSRRPTHandler               UpdateHalMSRRPTHandler;

    //4 SetPhy BB parameter related
    NicPHYSetCCKTxPowerHandler              PHYSetCCKTxPowerHandler;
    NicPHYSetOFDMTxPowerHandler             PHYSetOFDMTxPowerHandler;
    NicPHYUpdateBBRFValHandler              PHYUpdateBBRFValHandler;
#endif

#if     IS_RTL8192E_SERIES
    PVOID               PHalFunc8192E;
#endif

#if     IS_RTL8881A_SERIES
    PVOID               PHalFunc8881A;
#endif

#endif  //IS_RTL88XX_GENERATION    
	DumpRxBDescHandler	DumpRxBDescTestHandler;
	DumpTxBDescHandler	DumpTxBDescTestHandler;
} HAL_INTERFACE_COMMON, *PHAL_INTERFACE_COMMON;

#define HAL_INTERFACE           HAL_INTERFACE_COMMON
#define PHAL_INTERFACE          PHAL_INTERFACE_COMMON
#define GET_HAL_INTERFACE(__pAdapter)	((HAL_INTERFACE_COMMON *)((__pAdapter)->HalFunc))

typedef struct _HAL_DATA_MV_
{
    u1Byte  test;
    
}HAL_DATA_MV, *PHAL_DATA_PMV;


typedef u4Byte RT_INT_REG, *PRT_INT_REG;


// Variable: AccessSwapCtrl
#define HAL_ACCESS_SWAP_IO      BIT0	/* Do bye-swap in access IO register */
#define HAL_ACCESS_SWAP_MEM     BIT1    /* Do byte-swap in access memory space */

typedef struct _HAL_DATA_COMMON_
{
    u2Byte              HardwareType;
    BOOLEAN             bTestChip;      // 1: TestChip, 0:MP
	u1Byte				cutVersion;

    //IO/MEM Swap
    u4Byte              AccessSwapCtrl;

    //Mapping driver variable
    HAL_DATA_MV         MappingVariable;

    //ISR
    //u4Byte              InterruptMask;
    //u4Byte              InterruptMaskExt;
    RT_INT_REG          IntArray[2];
    RT_INT_REG			IntMask[2];
    RT_INT_REG			IntMask_RxINTBackup[2]; //Backup for Rx IMR Control

#if IS_RTL88XX_GENERATION

    //Firmware
    u1Byte              H2CBufPtr88XX;      //88XX range: 0~3
    BOOLEAN             bFWReady;
    PVOID               PFWHeader;

    //TRX DESC
    PVOID               PRxDescData88XX;
    PVOID               PRxDescStatus88XX;

#if (DEV_BUS_TYPE & (RT_EMBEDDED_INTERFACE | RT_PCI_INTERFACE))
    PVOID               PTxDMA88XX;
    PVOID               PRxDMA88XX;
    u4Byte              ring_dma_addr;	//rx_dma_addr_start.
    u4Byte              ring_buf_len;
    u4Byte              ring_virt_addr;
    u4Byte              alloc_dma_buf;
    u4Byte              txBD_dma_ring_addr[14]; //there are 14 queues in system, including BCN queue
    u4Byte              txDesc_dma_ring_addr[14]; //there are 14 queues in system, including BCN queue
#endif

#if IS_RTL8881A_SERIES
    PVOID               PHalData8881A;
#endif  //IS_RTL8881A_SERIES

#if IS_RTL8192E_SERIES
    PVOID               PHalData8192E;
#endif  //IS_RTL8192E_SERIES

#endif  //IS_RTL88XX_GENERATION

}HAL_DATA_COMMON, *PHAL_DATA_COMMON;

#define HAL_DATA_TYPE                   HAL_DATA_COMMON
#define PHAL_DATA_TYPE                  PHAL_DATA_COMMON
#define _GET_HAL_DATA(__pAdapter)	    ((HAL_DATA_TYPE *)((__pAdapter)->HalData))


#define HAL_HW_TYPE_ID_8723A				0x01
#define HAL_HW_TYPE_ID_8188E				0x02
#define HAL_HW_TYPE_ID_8881A				0x03
#define HAL_HW_TYPE_ID_8812A				0x04
#define HAL_HW_TYPE_ID_8723B				0x05
#define HAL_HW_TYPE_ID_8821A				0x06
#define HAL_HW_TYPE_ID_8192E				0x07

typedef enum _HARDWARE_TYPE{
	HARDWARE_TYPE_RTL8192SE,
	HARDWARE_TYPE_RTL8192SU,
	HARDWARE_TYPE_RTL8192CE,
	HARDWARE_TYPE_RTL8192CU,
	HARDWARE_TYPE_RTL8192DE,
	HARDWARE_TYPE_RTL8192DU,
	HARDWARE_TYPE_RTL8723AE,
	HARDWARE_TYPE_RTL8723AU,
	HARDWARE_TYPE_RTL8723AS,
	HARDWARE_TYPE_RTL8188EE,
	HARDWARE_TYPE_RTL8188EU,
	HARDWARE_TYPE_RTL8188ES,
	HARDWARE_TYPE_RTL8812E,
	HARDWARE_TYPE_RTL8821E,
	HARDWARE_TYPE_RTL8812AU,
	HARDWARE_TYPE_RTL8821U,
	HARDWARE_TYPE_RTL8881AEM,
	HARDWARE_TYPE_RTL8192EE,
    HARDWARE_TYPE_RTL8192EU,
	HARDWARE_TYPE_MAX,
}HARDWARE_TYPE;

#define IS_HAL_TEST_CHIP(_Adapter)              (_GET_HAL_DATA(_Adapter)->bTestChip==_TRUE)

//
// RTL8192E Series
//
#define IS_HARDWARE_TYPE_8192EE(_Adapter)	(_GET_HAL_DATA(_Adapter)->HardwareType==HARDWARE_TYPE_RTL8192EE)
#define IS_HARDWARE_TYPE_8192EU(_Adapter)	(_GET_HAL_DATA(_Adapter)->HardwareType==HARDWARE_TYPE_RTL8192EU)
#define IS_HARDWARE_TYPE_8192E(_Adapter)			\
    (IS_HARDWARE_TYPE_8192EE(_Adapter) || IS_HARDWARE_TYPE_8192EU(_Adapter) )

//
// RTL8881A Series
//
#define IS_HARDWARE_TYPE_8881AEM(_Adapter)	(_GET_HAL_DATA(_Adapter)->HardwareType==HARDWARE_TYPE_RTL8881AEM)
#define IS_HARDWARE_TYPE_8881A(_Adapter)			\
    (IS_HARDWARE_TYPE_8881AEM(_Adapter))



#endif  //__HALDEF_H__
