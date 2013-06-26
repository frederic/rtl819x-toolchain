#ifndef __HAL_COM_REG_H__
#define __HAL_COM_REG_H__
/*-------------------------Modification Log-----------------------------------
    For Page0, it is based on Combo_And_WL_Only_Page0_Reg.xls SVN524
    The supported IC are 8723A, 8881A, 8723B, 8192E, 8881A
    8812A and 8188E is not included in page0 register
    
    For other pages, it is based on MAC_Register.doc SVN502
    Most IC is the same with 8812A
-------------------------Modification Log-----------------------------------*/

/*--------------------------Include File--------------------------------------*/
#include "HalHWCfg.h"
/*--------------------------Include File--------------------------------------*/

//-----------------------------------------------------
//
//  0x0000h ~ 0x00FFh   System Configuration
//
//-----------------------------------------------------

#if CONFIG_WLANREG_SUPPORT & ~(SUPPORT_CHIP_8812A|SUPPORT_CHIP_8188E)

    #define REG_SYS_ISO_CTRL            0x0000
    #define REG_SYS_FUNC_EN             0x0002
    #define REG_SYS_PW_CTRL             0x0004
    #define REG_SYS_CLK_CTRL            0x0008
    #define REG_SYS_EEPROM_CTRL         0x000A
    #define REG_EE_VPD                  0x000C
    #define REG_SYS_SWR_CTRL1           0x0010
    #define REG_SYS_SWR_CTRL2           0x0018
    #define REG_RSV_CTRL                0x001C
    #define REG_RF_CTRL                 0x001F
    #define REG_AFE_LDO_CTRL            0x0020
    #define REG_AFE_CTRL1               0x0024
    #define REG_AFE_CTRL2               0x0028
    #define REG_AFE_CTRL3               0x002C
    
    #define REG_EFUSE_CTRL              0x0030
    #define REG_LDO_EFUSE_CTRL          0x0034
    #define REG_PWR_OPTION_CTRL         0x0038
    #define REG_CAL_TIMER               0x003C
    #define REG_ACLK_MON                0x003E
    
    #define REG_GPIO_MUXCFG             0x0040
    #define REG_GPIO_PIN_CTRL           0x0044
    #define REG_GPIO_INTM               0x0048
    #define REG_LEDCFG                  0x004C
    
    #define REG_FSIMR                   0x0050
    #define REG_FSISR                   0x0054
    #define REG_HSIMR                   0x0058
    #define REG_HSISR                   0x005C
    
    #define REG_GPIO_EXT_CTRL           0x0060
    #define REG_PAD_CTRL1               0x0064
    #define REG_WL_BT_PWR_CTRL          0x0068
    #define REG_SDM_DEBUG               0x006C
    #define REG_SDIO_CTRL               0x0070
    #define REG_HCI_OPT_CTRL            0x0074
    #define REG_AFE_CTRL4               0x0078
    #define REG_LDO_SWR_CTRL            0x007C
    
    #define REG_8051FW_CTRL             0x0080
    #define REG_MCU_TST_CFG             0x0084
    
    #define REG_HMEBOX_E0_E1            0x0088			
    #define REG_HMEBOX_E2_E3            0x008C
    
    #define REG_WLLPS_CTRL              0x0090
    #define REG_RSVD_0x94               0x0094
    #define REG_GPIO_DEBOUNCE_CTRL      0x0098
    #define REG_RPWM2                   0x009E
    
    #define REG_SYSON_FSM_MON           0x00A0
    #define REG_RSVD_0xA4               0x00A4
    #define REG_PMC_DBG_CTRL1           0x00A8
    
    #define REG_HIMR0                   0x00B0
    #define REG_HISR0                   0x00B4
    #define REG_HIMR1                   0x00B8
    #define REG_HISR1                   0x00BC
    #define REG_DBG_PORT_SEL            0x00C0
    #define REG_PAD_CTRL2               0x00C4
    #define REG_RSVD_0xC8               0x00C8
    #define REG_PMC_DBG_CTRL2           0x00CC
    #define REG_BIST_CTRL               0x00D0
    #define REG_BIST_RPT                0x00D4
    #define REG_MEM_CTRL                0x00D8
    #define REG_RSVD_0xDC               0x00DC
    #define REG_USB_SIE_INTF            0x00E0
    #define REG_PCIE_MIO_INTF           0x00E4
    #define REG_PCIE_MIO_INTD           0x00E8
    #define REG_RSVD_0xEC               0x00EC
    #define REG_SYS_CFG1                0x00F0
    #define REG_SYS_STATUS1             0x00F4
    #define REG_SYS_STATUS2             0x00F8
    #define REG_SYS_CFG2                0x00FC

#endif 
//-----------------------------------------------------
//
//  0x0100h ~ 0x01FFh   MACTOP General Configuration
//  For other pages, most IC is the same with 8812
//
//-----------------------------------------------------
#define REG_CR                      0x0100
#define REG_PBP                     0x0104//Filen change to this ID according to DataSheet
#define REG_PKT_BUFF_ACCESS_CTRL    0x0106
#define REG_TSF_CLK_STATE           0x0109
#define REG_TRX_DMA_CTRL            0x010C
#define REG_TRXFF_BNDY              0x0114    //Filen change to this ID according to DataSheet
#define REG_TRXFF_STATUS            0x0118
#define REG_RXFF_PTR                0x011C

#if CONFIG_WLANREG_SUPPORT & (~SUPPORT_CHIP_8812A)
    #define REG_HIMR                    0x0120
    #define REG_HISR                    0x0124
#endif  
#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8812A)
    #define REG_8812A_FEIMR             0x0120
    #define REG_8812A_FEISR             0x0124
#endif  

#define REG_HIMRE                   0x0128
#define REG_HISRE                   0x012C
#define REG_CPWM                    0x012F

//FWIMR
#define REG_FWIMR                   0x0130
#define REG_WLANIMR0                0x0130
#define REG_WLANIMR1                0x0131
#define REG_WLANIMR2                0x0132
#define REG_WLANIMR3                0x0133

//FWISR
#define REG_FWISR                   0x0134
#define REG_WLANISR0                0x0134
#define REG_WLANISR1                0x0135
#define REG_WLANISR2                0x0136
#define REG_WLANISR3                0x0137

//FTIMR
#define REG_FTIMR                   0x0138
#define REG_FTIMR0                  0x0138
#define REG_FTIMR1                  0x0139
#define REG_FTIMR2                  0x013A
#define REG_FTIMR3                  0x013B

//FTISR
#define REG_FTISR                   0x013C
#define REG_FTISR0                  0x013C
#define REG_FTISR1                  0x013D
#define REG_FTISR2                  0x013E
#define REG_FTISR3                  0x013F

#define REG_PKTBUF_DBG_CTRL         0x0140//KaiYuan: In DataSheet, There are two items for 8812
#define REG_PKTBUF_DBG_DATA_L       0x0144
#define REG_PKTBUF_DBG_DATA_H       0x0148
#define REG_CPWM2                   0x014C
#define REG_TC0_CTRL                0x0150
#define REG_TC1_CTRL                0x0154
#define REG_TC2_CTRL                0x0158
#define REG_TC3_CTRL                0x015C
#define REG_TC4_CTRL                0x0160
#define REG_TCUNIT_BASE             0x0164

#if CONFIG_WLANREG_SUPPORT & ~(SUPPORT_CHIP_8723A|SUPPORT_CHIP_8188E)
    #define REG_TC5_CTRL      0x0168
    #define REG_TC6_CTRL      0x016C
#endif 

#if CONFIG_WLANREG_SUPPORT & (~SUPPORT_CHIP_8723A)
    #define REG_DRF_MBIST_FAIL      0x0170
#endif 

#if CONFIG_WLANREG_SUPPORT & (~SUPPORT_CHIP_8723A)//KaiYuan: Suggest to Use the same ID for 88E and 8812 in datasheet
    #define REG_MBIST_START         0x0174
    #define REG_MBIST_DONE          0x0178
    #define REG_MBIST_FAIL          0x017C
#endif 

#define REG_AES_DECRPT_DATA         0x0180
#define REG_AES_DECRPT_CFG          0x0184
#define REG_SHA1_DATA               0x0188
#define REG_SHA1_CFG                0x018C

//32K control reg
#define REG_TMCR                    0x0190
#define REG_32KCTRL1                0x0194//KaiYuan: Suggest to Use the same ID for 8723, 88e in datasheet
#define REG_32KCTRL2                0x0198

//0x1A0 ~ 0x1AF are used for C2H
#define REG_C2HEVT                  0x01A0

#if CONFIG_WLANREG_SUPPORT & (~SUPPORT_CHIP_8723A)
    #define REG_UPHY_CPTRIM_CTRL        0x01B0
    #define REG_EFUSE_HW_CTRL           0x01B4
#endif
//0x1B8 ~ 0x1BF: there are no control circuit. But these register can be R/W.
//0x1C0 ~ 0x1C7: there are no control circuit. But these register can be R/W.

#define REG_MCUDMSG_1               0x01C0
#define REG_MCUDMSG_2               0x01C4
#define REG_FMETHR                  0x01C8
#define REG_HMETFR                  0x01CC

//0x1D0 ~0x1DF are H2C message box
#define REG_HMEBOX_0                0x01D0
#define REG_HMEBOX_1                0x01D4
#define REG_HMEBOX_2                0x01D8
#define REG_HMEBOX_3                0x01DC
//LLT
#define REG_LLT_INIT                0x01E0
#define REG_LLT_INIT_HDATA          0x01E0
#define REG_LLT_INIT_ADDR           0x01E1
#define REG_LLT_INIT_PDATA          0x01E2
#define REG_LLTE_RWM                0x01E3
#define REG_GENERAL_TEST            0x01E4
//BB Indirect Access
#define REG_BB_ACCEESS_CTRL         0x01E8
#define REG_BB_ACCESS_DATA          0x01EC

//0x1F0~0x1FF are used for H2C extension
#define REG_HMEBOX_E0_V1            0x01F0
#define REG_HMEBOX_E1_V1            0x01F4
#define REG_HMEBOX_E2_V1            0x01F8
#define REG_HMEBOX_E3_V1            0x01FC

//-----------------------------------------------------
//
//  0x0200h ~ 0x027Fh   TXDMA Configuration
//
//-----------------------------------------------------
#define REG_RQPN                    0x0200
#define REG_FIFOPAGE                0x0204
#define REG_TDECTRL                 0x0208
#define REG_TXDMA_OFFSET_CHECK      0x020C
#define REG_TXDMA_STATUS            0x0210
#define REG_RQPN_NPQ                0x0214

#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8188E)
    #define REG_8188E_TXDMA_DEBUG_PORT    0x0218
#endif    

#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8723A)
    #define REG_8723A_TQPNT         0x0218
    #define REG_8723A_TQPNTC        0x0220
#endif 

#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8821A|SUPPORT_CHIP_8881A|SUPPORT_CHIP_8192E)
    #define REG_AUTO_LLT_INIT       0x0224
    #define REG_TDECTRL1            0x0228
#endif

#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8188E)
    #define REG_8188E_BIST_START_PAUSE  0x240
    #define REG_8188E_BIST_READY        0x244
    #define REG_8188E_BIST_FAIL         0x248
    #define REG_8188E_BIST_RPT          0x24C
    #define REG_8188E_MAC_SELECT        0x250
#endif

//-----------------------------------------------------
//
//  0x0280h ~ 0x02FFh   RXDMA Configuration
//
//-----------------------------------------------------
#define REG_RXDMA_AGG_PG_TH         0x0280
#define REG_WOW_RXPKT_NUM           0x0282
#define REG_RXPKT_NUM               0x0284
#define REG_RXDMA_STATUS            0x0288
#if CONFIG_WLANREG_SUPPORT & (~SUPPORT_CHIP_8723A)
    #define REG_RXDMA_DBG               0x028C
#endif    

//-----------------------------------------------------
//
// BT_COEX
//
//-----------------------------------------------------
#if CONFIG_WLANREG_SUPPORT & (~SUPPORT_CHIP_8723A)
    #define REG_BT_COEX_GLB_CTRL        0x02C0
    #define REG_BT_COEX_TBL             0x02C4
    #define REG_BT_SSI                  0x02D4
    #define REG_BT_CSR_ENH_INTF_CTRL    0x02D6
    #define REG_BT_ACT_STATISTICS       0x02D8
    #define REG_BT_CMD_WLAN_RPT         0x02E0
    #define REG_BT_CMD_BT_RPT           0x02E1
    #define REG_BT_CMD_LATCH            0x02E2
#endif

#if CONFIG_WLANREG_SUPPORT & ~(SUPPORT_CHIP_8188E|SUPPORT_CHIP_8723A)
    #define REG_DRV_FW_INTF_DATA0        0x02F0
    #define REG_DRV_FW_INTF_DATA1        0x02F4
    #define REG_DRV_FW_INTF_DATA2        0x02F8
    #define REG_DRV_FW_INTF_DATA3        0x02FC
#endif

//0x02F0~ 0x02FF: there are no control circuit. But these register can be R/W.

//1 KaiYuan 0x0300~0x03FF to be checked later
//-----------------------------------------------------
//
//  0x0300h ~ 0x03FFh   PCIe/LBus
//
//-----------------------------------------------------
// TODO: Check MAC_reg.doc in svn
/////////// SD7
#define	REG_PCIE_CTRL_REG           0x0300
#define	REG_INT_MIG                 0x0304	// Interrupt Migration 
#define	REG_BCNQ_DESA               0x0308	// TX Beacon Descriptor Address
#define	REG_HQ_DESA                 0x0310	// TX High Queue Descriptor Address
#define	REG_MGQ_DESA                0x0318	// TX Manage Queue Descriptor Address
#define	REG_VOQ_DESA                0x0320	// TX VO Queue Descriptor Address
#define	REG_VIQ_DESA                0x0328	// TX VI Queue Descriptor Address
#define	REG_BEQ_DESA                0x0330	// TX BE Queue Descriptor Address
#define	REG_BKQ_DESA                0x0338	// TX BK Queue Descriptor Address
#define	REG_RX_DESA                 0x0340	// RX Queue	Descriptor Address
//sherry added for DBI Read/Write  20091126
#define	REG_DBI_WDATA               0x0348	// Backdoor REG for Access Configuration
#define REG_DBI_RDATA               0x034C  //Backdoor REG for Access Configuration
#define REG_DBI_CTRL                0x0350  //Backdoor REG for Access Configuration
#define REG_DBI_FLAG                0x0352  //Backdoor REG for Access Configuration#define	REG_MDIO					0x0354	// MDIO for Access PCIE PHY
#define	REG_MDIO                    0x0354	// MDIO for Access PCIE PHY
#define	REG_DBG_SEL                 0x0360	// Debug Selection Register
#define	REG_PCIE_HRPWM              0x0361	//PCIe RPWM
#define	REG_PCIE_HCPWM              0x0363	//PCIe CPWM
#define	REG_WATCH_DOG               0x0368

// RTL8723 series -------------------------------
#define	REG_PCIE_HISR_EN			0x0394	//PCIE Local Interrupt Enable Register
#define	REG_PCIE_HISR				0x03A0
#define	REG_PCIE_HISRE				0x03A4
#define	REG_PCIE_HIMR				0x03A8
#define	REG_PCIE_HIMRE				0x03AC
// TODO: Check Address
#define	REG_PCIE_HRPWM2
#define	REG_PCIE_HCPWM2
#define REG_PCIE_H2C_MSG
#define REG_PCIE_C2H_MSG


///////////

#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8881A|SUPPORT_CHIP_8192E)

#if CONFIG_WLANREG_SUPPORT & SUPPORT_CHIP_8192E
#define REG_PCIE_CTRL1              0x0300  // 4 Bytes
#endif

#if CONFIG_WLANREG_SUPPORT & SUPPORT_CHIP_8881A
#define REG_LX_CTRL1                0x0300  // 4 Bytes
#endif

#define REG_INT_MIG_CFG             0x0304  // 4 Bytes
#define REG_BCNQ_TXBD_DESA          0x0308  // 8 Bytes
#define REG_MGQ_TXBD_DESA           0x0310  // 8 Bytes 
#define REG_VOQ_TXBD_DESA           0x0318  // 8 Bytes
#define REG_VIQ_TXBD_DESA           0x0320  // 8 Bytes
#define REG_BEQ_TXBD_DESA           0x0328  // 8 Bytes
#define REG_BKQ_TXBD_DESA           0x0330  // 8 Bytes
#define REG_RXQ_RXBD_DESA           0x0338  // 8 Bytes
#define REG_HI0Q_TXBD_DESA          0x0340  // 8 Bytes
#define REG_HI1Q_TXBD_DESA          0x0348  // 8 Bytes
#define REG_HI2Q_TXBD_DESA          0x0350  // 8 Bytes
#define REG_HI3Q_TXBD_DESA          0x0358  // 8 Bytes
#define REG_HI4Q_TXBD_DESA          0x0360  // 8 Bytes
#define REG_HI5Q_TXBD_DESA          0x0368  // 8 Bytes
#define REG_HI6Q_TXBD_DESA          0x0370  // 8 Bytes
#define REG_HI7Q_TXBD_DESA          0x0378  // 8 Bytes

#define REG_MGQ_TXBD_NUM            0x0380  // 2 Bytes
#define REG_RX_RXBD_NUM             0x0382  // 2 Bytes
#define REG_VOQ_TXBD_NUM            0x0384  // 2 Bytes
#define REG_VIQ_TXBD_NUM            0x0386  // 2 Bytes
#define REG_BEQ_TXBD_NUM            0x0388  // 2 Bytes
#define REG_BKQ_TXBD_NUM            0x038A  // 2 Bytes
#define REG_HI0Q_TXBD_NUM           0x038C  // 2 Bytes
#define REG_HI1Q_TXBD_NUM           0x038E  // 2 Bytes
#define REG_HI2Q_TXBD_NUM           0x0390  // 2 Bytes
#define REG_HI3Q_TXBD_NUM           0x0392  // 2 Bytes
#define REG_HI4Q_TXBD_NUM           0x0394  // 2 Bytes
#define REG_HI5Q_TXBD_NUM           0x0396  // 2 Bytes
#define REG_HI6Q_TXBD_NUM           0x0398  // 2 Bytes
#define REG_HI7Q_TXBD_NUM           0x039A  // 2 Bytes

#define REG_BD_RWPTR_CLR            0x039C  // 4 Bytes
#define REG_VOQ_TXBD_IDX            0x03A0  // 4 Bytes
#define REG_VIQ_TXBD_IDX            0x03A4  // 4 Bytes
#define REG_BEQ_TXBD_IDX            0x03A8  // 4 Bytes
#define REG_BKQ_TXBD_IDX            0x03AC  // 4 Bytes
#define REG_MGQ_TXBD_IDX            0x03B0  // 4 Bytes
#define REG_RXQ_RXBD_IDX            0x03B4  // 4 Bytes
#define REG_HI0Q_TXBD_IDX           0x03B8  // 4 Bytes
#define REG_HI1Q_TXBD_IDX           0x03BC  // 4 Bytes
#define REG_HI2Q_TXBD_IDX           0x03C0  // 4 Bytes
#define REG_HI3Q_TXBD_IDX           0x03C4  // 4 Bytes
#define REG_HI4Q_TXBD_IDX           0x03C8  // 4 Bytes
#define REG_HI5Q_TXBD_IDX           0x03CC  // 4 Bytes
#define REG_HI6Q_TXBD_IDX           0x03D0  // 4 Bytes
#define REG_HI7Q_TXBD_IDX           0x03D4  // 4 Bytes

#if CONFIG_WLANREG_SUPPORT & SUPPORT_CHIP_8192E
#define REG_DBG_SEL_V1              0x03D8  // 1 Bytes
#endif

#define REG_PCIE_HRPWM1_V1          0x03D9  // 1 Bytes
#define REG_PCIE_HCPWM1_V1          0x03DA  // 1 Bytes

#if CONFIG_WLANREG_SUPPORT & SUPPORT_CHIP_8192E
#define REG_PCIE_CTRL2              0x03DB  // 1 Bytes
#endif

#if CONFIG_WLANREG_SUPPORT & SUPPORT_CHIP_8881A
#define REG_LX_CTRL2                0x03DB  // 1 Bytes
#endif

#define REG_PCIE_HRPWM2_V1          0x03DC  // 2 Bytes
#define REG_PCIE_HCPWM2_V1          0x03DE  // 2 Bytes
#define REG_PCIE_H2C_MSG_V1         0x03E0  // 4 Bytes
#define REG_PCIE_C2H_MSG_V1         0x03E4  // 4 Bytes

#if CONFIG_WLANREG_SUPPORT & SUPPORT_CHIP_8192E
#define REG_DBI_WDATA_V1            0x03E8  // 4 Bytes
#define REG_DBI_RDATA_V1            0x03EC  // 4 Bytes
#define REG_DBI_FLAG_V1             0x03F0  // 4 Bytes
#endif 

#if CONFIG_WLANREG_SUPPORT & SUPPORT_CHIP_8881A
#define REG_LX_DMA_ISR              0x03E8  // 4 Bytes
#define REG_LX_DMA_IMR              0x03EC  // 4 Bytes
#define REG_LX_DMA_DBG              0x03F0  // 4 Bytes
#endif 

#if CONFIG_WLANREG_SUPPORT & SUPPORT_CHIP_8192E
#define REG_MDIO_V1                 0x03F4  // 4 Bytes
#endif

#if CONFIG_WLANREG_SUPPORT & SUPPORT_CHIP_8192E
#define REG_PCIE_MIX_CFG            0x03F8  // 4 Bytes
#endif

#if CONFIG_WLANREG_SUPPORT & SUPPORT_CHIP_8881A
#define REG_BUS_MIX_CFG             0x03F8  // 4 Bytes
#define REG_BUS_MIX_CFG1            0x03FC  // 4 Bytes
#endif

#endif // endif CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8881A|SUPPORT_CHIP_8192E)
//-----------------------------------------------------
//
//  0x0400h ~ 0x047Fh   Protocol Configuration
//
//-----------------------------------------------------
#define REG_Q0_INFORMATION          0x0400
#define REG_Q1_INFORMATION          0x0404
#define REG_Q2_INFORMATION          0x0408
#define REG_Q3_INFORMATION          0x040C
#define REG_MGQ_INFORMATION         0x0410
#define REG_HGQ_INFORMATION         0x0414
#define REG_BCNQ_INFORMATION        0x0418
#define REG_TXPKT_EMPTY             0x041A
#define REG_CPU_MGQ_INFORMATION     0x041C
#define REG_FUNCTION_ENABLE         0x0420
#define REG_HWSEQ_CTRL              0x0423
#define REG_TXPKTBUF_BCNQ_BDNY      0x0424
#define REG_TXPKTBUF_MGQ_BDNY       0x0425
#define REG_LIFETIME_CTRL           0x0426
#define REG_FREE_TAIL               0x0427
#define REG_SPEC_SIFS               0x0428
#define REG_RL                      0x042A
#if CONFIG_WLANREG_SUPPORT & ~(SUPPORT_CHIP_8723A|SUPPORT_CHIP_8188E)
    #define REG_TXBF_CTRL               0x042C    
#endif  
#define REG_DARFRC                  0x0430
#define REG_RARFRC                  0x0438
#define REG_RRSR                    0x0440
#define REG_ARFR0                   0x0444
#define REG_ARFR1                   0x044C
#define REG_CCK_CHECK               0x0454
#define REG_AMPDU_MAX_TIME          0x0456

#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8881A|SUPPORT_CHIP_8192E)
    #define REG_TXPKTBUF_BCNQ_BDNY1     0x0457
#endif

#define REG_AMPDU_MAX_LENGTH        0x0458
#if CONFIG_WLANREG_SUPPORT & (~SUPPORT_CHIP_8723A)
    #define REG_ACQ_STOP                0x045C
#endif
#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8188E)
    #define REG_8188E_WMAC_LBK_BUF_HD   0x045D
#endif
#if CONFIG_WLANREG_SUPPORT & (~SUPPORT_CHIP_8188E)
    #define REG_SEARCH_QUEUE_EN         0x045E
#endif
#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8188E)
    #define REG_8188E_TX_HANG_CTRL      0x045E
#endif

#if CONFIG_WLANREG_SUPPORT & ~(SUPPORT_CHIP_8723A|SUPPORT_CHIP_8188E)
    #define REG_NDPA_OPT_CTRL           0x045F
#endif  

#define REG_FAST_EDCA_CTRL          0x0460
#define REG_RD_RESP_PKT_TH          0x0463

#if CONFIG_WLANREG_SUPPORT & (~SUPPORT_CHIP_8188E)
    #define REG_Q4_INFORMATION          0x0468
    #define REG_Q5_INFORMATION          0x046C
    #define REG_Q6_INFORMATION          0x0470
    #define REG_Q7_INFORMATION          0x0474
    #define REG_SPCRPT_CTRL             0x047C
    #define REG_SPC_W_PTR               0x047E
    #define REG_SPC_R_PTR               0x047F
#endif

#define REG_INIRTS_RATE_SEL         0x0480
#define REG_INI_BASIC_CFEND_SEL     0x0481
#define REG_INI_STBC_CFEND_SEL      0x0482
#if CONFIG_WLANREG_SUPPORT & ~(SUPPORT_CHIP_8723A|SUPPORT_CHIP_8188E)
    #define REG_DATA_SC             0x0483
#endif
#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8188E)
    #define REG_8188E_MACID_NO_LINK       0x0484
    #define REG_8188E_MACID_PAUSE         0x048C
#endif
#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8723A)
    #define REG_8723A_INIDATA_RATE_SEL     0x0484
    #define REG_8723A_POWER_STATUS         0x048A
#endif
#if CONFIG_WLANREG_SUPPORT & ~(SUPPORT_CHIP_8723A|SUPPORT_CHIP_8188E)
    #define REG_MACID_PKT_SLEEP_3       0x0484
    #define REG_MACID_PKT_SLEEP_1       0x0488
    #define REG_ARFR2                   0x048C
    #define REG_ARFR3                   0x0494
    #define REG_ARFR4                   0x049C
    #define REG_ARFR5                   0x04A4
    #define REG_TXRPT_Start_Offset      0x04AC
    #define REG_TRYING_CNT_TH           0x04B0
#endif

#define REG_POWER_STAGE1            0x04B4
#define REG_POWER_STAGE2            0x04B8
#if CONFIG_WLANREG_SUPPORT & ~(SUPPORT_CHIP_8723A|SUPPORT_CHIP_8188E)
    #define REG_SW_AMPDU_BURST_MODE     0x04BC
#endif
#define REG_PKT_LIFE_TIME           0x04C0
#define REG_STBC_SETTING            0x04C4

#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8188E)
    #define REG_8188E_TRI_PKT           0x04C5
#endif  
#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8723A)
    #define REG_8723A_ACQ_SW_AMPDU_BURST_MODE_EN 0x04C5
#endif
#if CONFIG_WLANREG_SUPPORT & (~SUPPORT_CHIP_8188E)
    #define REG_QUEUE_CTRL              0x04C6    
#endif   

#define REG_PROT_MODE_CTRL          0x04C8
#define REG_BAR_MODE_CTRL           0x04CC
#define REG_RA_TRY_RATE_AGG_LMT     0x04CF

#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8188E)
    #define REG_8188E_EARLY_MODE           0x04D0
    #define REG_8188E_EARLY_MODE_RATE_EN   0x04D4
    #define REG_8188E_DMA_QUE_EN           0x04D5
#endif  

#if CONFIG_WLANREG_SUPPORT & (~SUPPORT_CHIP_8188E)
    #define REG_MACID_PKT_DROP          0x04D0
    #define REG_MACID_PKT_SLEEP         0x04D4
    #define REG_HW_SEQ0                 0x04D8
    #define REG_HW_SEQ1                 0x04DA    
#endif 

#define REG_NQOS_SEQ                0x04DC
#define REG_QOS_SEQ                 0x04DE

#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8188E)
    #define REG_8188E_PTCL_ERR_STATUS      0x04E0
#endif 

#if CONFIG_WLANREG_SUPPORT & (~SUPPORT_CHIP_8188E)
#define REG_PTCL_ERR_STATUS         0x04E2
#endif
#define REG_TX_PKT_NUM              0x04E3
#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8188E)
    #define REG_8188E_MISS_TXRPT           0x04E4
    #define REG_8188E_PTCL_DROP_PKT_NUM    0x04E8
    #define REG_8188E_SW_AMPDU_BURST_MODE_UNDERFLOW_NUM      0x04EA
    #define REG_8188E_TX_RPT_CTL           0x04EC
    #define REG_8188E_TX_RPT_TIM           0x04F0
    #define REG_8188E_TX_STS_SET           0x04F2
    #define REG_8188E_TX_STS_VLD           0x04F4
    #define REG_8188E_TX_STS_INF           0x04F8
#endif 

#define REG_PAGE4_DUMMY             0x04FC

//-----------------------------------------------------
//
//  0x0500h ~ 0x05FFh   EDCA Configuration
//
//-----------------------------------------------------
#define REG_EDCA_VO_PARAM           0x0500
#define REG_EDCA_VI_PARAM           0x0504
#define REG_EDCA_BE_PARAM           0x0508
#define REG_EDCA_BK_PARAM           0x050C
#define REG_BCNTCFG                 0x0510
#define REG_PIFS                    0x0512
#define REG_RDG_PIFS                0x0513
#define REG_SIFS                    0x0514
#define REG_TSFTR_SYN_OFFSET        0x0518
#define REG_AGGR_BREAK_TIME         0x051A
#define REG_SLOT                    0x051B
#define REG_TX_PTCL_CTRL            0x0520
#define REG_TXPAUSE                 0x0522
#define REG_DIS_TXREQ_CLR           0x0523
#define REG_RD_CTRL                 0x0524
#define REG_TX_PTCL_CTRL1           0x0525
#define REG_MBSSID_CTRL             0x0526
#define REG_P2PPS_CTRL              0x0527
#define REG_PKT_LIFETIME_CTRL       0x0528
#define REG_P2PPS_SPEC_STATE        0x052B
#define REG_TBTT_PROHIBIT           0x0540
#define REG_P2PPS_STATE             0x0543  
#define REG_RD_NAV_NXT              0x0544
#define REG_NAV_PROT_LEN            0x0546
#define REG_BCN_CTRL                0x0550
#define REG_BCN_CTRL1               0x0551
#define REG_MBID_NUM                0x0552
#define REG_DUAL_TSF_RST            0x0553
#define REG_MBSSID_BCN_SPACE        0x0554
#define REG_DRVERLYINT              0x0558
#define REG_BCNDMATIM               0x0559
#define REG_ATIMWND                 0x055A
#define REG_USTIME_TSF              0x055C
#define REG_BCN_MAX_ERR             0x055D
#define REG_RXTSF_OFFSET_CCK        0x055E
#define REG_RXTSF_OFFSET_OFDM       0x055F

#define REG_TSFTR                   0x0560
#define REG_TSFTR1                  0x0568
#define REG_ATIMWND1                0x0570
#define REG_CTWND                   0x0572
#define REG_BCNIVLCUNT              0x0573
#if CONFIG_WLANREG_SUPPORT & (~SUPPORT_CHIP_8723A)
    #define REG_BCNDROPCTRL	            0x0574    
#endif  
#define REG_HGQ_TIMEOUT_PERIOD      0x0575

#define REG_PS_TIMER                0x0580
#define REG_TIMER0                  0x0584
#define REG_TIMER1                  0x0588
#define REG_TBTT_CTN_AREA           0x058C
#define REG_FORCE_BCN_IFS           0x058E

#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8812A|SUPPORT_CHIP_8821A|SUPPORT_CHIP_8192E|SUPPORT_CHIP_8881A)
#define REG_PRE_DL_BCN_ITV          0x058F
#endif

#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8188E)
    #define REG_8188E_PRE_TX_CTRL       0x0590
    #define REG_8188E_TXOP_CTRL         0x0592
#endif

#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8881A|SUPPORT_CHIP_8192E)
    #define REG_ATIMWND2            0x05A0
    #define REG_ATIMWND3            0x05A1
    #define REG_ATIMWND4            0x05A2    
    #define REG_ATIMWND5            0x05A3    
    #define REG_ATIMWND6            0x05A4    
    #define REG_ATIMWND7            0x05A5
    #define REG_ATIMUGT             0x05A6
    #define REG_HIQ_NO_LMT_EN       0x05A7
    #define REG_DTIM_COUNTER_ROOT   0x05A8
    #define REG_DTIM_COUNTER_VAP1   0x05A9    
    #define REG_DTIM_COUNTER_VAP2   0x05AA    
    #define REG_DTIM_COUNTER_VAP3   0x05AB    
    #define REG_DTIM_COUNTER_VAP4   0x05AC    
    #define REG_DTIM_COUNTER_VAP5   0x05AD    
    #define REG_DTIM_COUNTER_VAP6   0x05AE    
    #define REG_DTIM_COUNTER_VAP7   0x05AF
    #define REG_DIS_ATIM            0x05B0
#endif

#define REG_ACMHWCTRL               0x05C0
#define REG_ACMRSTCTRL              0x05C1
#define REG_ACMAVG                  0x05C2
#define REG_VO_ADMTIME              0x05C4
#define REG_VI_ADMTIME              0x05C6
#define REG_BE_ADMTIME              0x05C8
#define REG_EDCA_RANDOM_GEN         0x05CC
#define REG_TXCMD_SEL               0x05CF
#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8188E)
    #define REG_8188E_NOA_PARAM               0x05E0
#endif
#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8723A)
    #define REG_8723A_NOA_PARAM               0x05E0
#endif

#define REG_SCH_TXCMD               0x05F8
//#define REG_PAGE5_DUMMY             0x05FC
#define REG_DRV_DBG                 0x05FC

//-----------------------------------------------------
//
//  0x0600h ~ 0x07FFh   WMAC Configuration
//
//-----------------------------------------------------
#if CONFIG_WLANREG_SUPPORT & (~SUPPORT_CHIP_8723A)
    #define REG_WMAC_CR                 0x0600    
#endif
#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8723A)
    #define REG_8723A_APSD_CTRL          0x0600
    #define REG_8723A_BWOPMODE           0x0603
#endif    

#define REG_TCR                     0x0604
#define REG_RCR                     0x0608
#define REG_RX_PKT_LIMIT            0x060C
#define REG_RX_DLK_TIME             0x060D
#define REG_RX_DRVINFO_SZ           0x060F
#define REG_MACID                   0x0610
#define REG_BSSID                   0x0618
#define REG_MAR                     0x0620
#define REG_MBIDCAMCFG              0x0628

//0x634 ~ 0x637: there are no control circuit. But these register can be R/W.

#define REG_USTIME_EDCA             0x0638
#define REG_MAC_SPEC_SIFS           0x063A
#define REG_RESP_SIFS_CCK           0x063C
#define REG_RESP_SIFS_OFDM          0x063E
#define REG_ACKTO                   0x0640
#define REG_CTS2TO                  0x0641
#define REG_EIFS                    0x0642

//WMA, BA, CCX
#define REG_NAV_CTRL                0x0650
#define REG_BACAMCMD                0x0654
#define REG_BACAMCONTENT            0x0658
#define REG_LBDLY                   0x0660
#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8723A)
    #define REG_8723A_FWDLY          0x0661
#endif 
#define REG_RXERR_RPT               0x0664
#define REG_WMAC_TRXPTCL_CTL        0x0668

// Security
#define REG_CAMCMD                  0x0670
#define REG_CAMWRITE                0x0674
#define REG_CAMREAD                 0x0678
#define REG_CAMDBG                  0x067C
#define REG_SECCFG                  0x0680

// Power
#define REG_WOW_CTRL                0x0690
#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8723A)
    #define REG_8723A_PSSTATUS       0x0691
#endif 
#define REG_PS_RX_INFO              0x0692
#define REG_WMMPS_UAPSD_TID         0x0693
#define REG_WKFMCAM_NUM             0x0698
#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8723A)
    #define REG_8723A_WAKEUP_FRAME_RW_DATA   0x069C
#endif 
#define REG_RXFLTMAP0               0x06A0
#define REG_RXFLTMAP1               0x06A2
#define REG_RXFLTMAP2               0x06A4
#define REG_BCN_PSR_RPT             0x06A8

#if CONFIG_WLANREG_SUPPORT & ~(SUPPORT_CHIP_8723A|SUPPORT_CHIP_8188E)
    #define REG_FLC_RPC             0x06AC
    #define REG_FLC_RPCT            0x06AD
    #define REG_FLC_PTS             0x06AE
    #define REG_FLC_TRPC            0x06AF
#endif 

#define REG_BT_COEX_TABLE           0x06C0
#define REG_WMAC_RESP_TXINFO        0x06D8

#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8723A)
    #define REG_8723A_P2P_RX_BCN_NOA  0x06E0
#endif

#define REG_MACID1                  0x0700
#define REG_BSSID1                  0x0708

#define REG_BCN_PSR_RPT1                  0x0710

#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8723A)
    #define REG_8723A_NS_ARP_CTRL           0x0720
    #define REG_8723A_NS_ARP_IPADDR         0x0728
    #define REG_8723A_NS_ARP_IPV6_MYADDR    0x0730
    #define REG_8723A_NS_ARP_IPV6_MCADDR    0x0740
#endif

#if CONFIG_WLANREG_SUPPORT & (~SUPPORT_CHIP_8723A)
    #define REG_SW_AES                      0x0750
    #define REG_SW_AES_CFG                  0x0760
    #define REG_WLAN_ACT_MSK_CTRL           0x0768
    #define REG_BT_STATISTICS_CTRL          0x076E//KaiYuan: Duplicated in datasheet
    #define REG_BT_ACT_STATISTICS           0x0770
    #define REG_BT_STATISTICS_OTH_CTRL      0x0778
    #define REG_BT_CMD_ID                   0x077C
    #define REG_BT_STATUS_RPT               0x077D
    #define REG_BT_DATA                     0x0780//KaiYuan: Duplicated in datasheet
    #define REG_BT_ISOLATION_TABLE          0x0785
    #define REG_BT_TDMA_BT_T                0x0790
    #define REG_BT_CH_INFO                  0x0794
    #define REG_BT_STATIC_INFO_EXT          0x0795
    
#endif    

#define REG_LTR_IDLE_LATENCY                0x0798
#define REG_LTR_ACTIVE_LATENCY              0x079C
#define REG_OBFF_CTRL                       0x07A0
#define REG_LTR_CTRL                        0x07A4//KaiYuan: Duplicated in datasheet
#define REG_ANTTRN_CTRL                     0x07B0

#define	REG_CNT_FORMAT			    0x07B8
#define	REG_FUNCTRL			    0x07BC

//-----------------------------------------------------
//
//  0xFB00h ~ 0xFCFFh   TX/RX packet buffer affress
//
//-----------------------------------------------------
#define REG_RXPKTBUF_STARTADDR      0xFB00
#define REG_TXPKTBUF_STARTADDR      0xFC00




//-----------------------------------------------------
//
//  0xFD00h ~ 0xFDFFh   8051 CPU Local REG
//
//-----------------------------------------------------
#define REG_SYS_CTRL            0xFD00
#define REG_PONSTS_RPT1         0xFD01
#define REG_PONSTS_RPT2         0xFD02
#define REG_PONSTS_RPT3         0xFD03
#define REG_PONSTS_RPT4         0xFD04  //0x84
#define REG_PONSTS_RPT5         0xFD05  //0x85
#define REG_8051ERRFLAG         0xFD08  
#define REG_8051ERRFLAG_MASK    0xFD09
#define REG_TXADDRH             0xFD10  //Tx Packet High Address
#define REG_RXADDRH             0xFD11  //Rx Packet High Address
#define REG_TXADDRH_EXT         0xFD12  //0xFD12[0] : for 8051 access txpktbuf high64k as external register

#define REG_U3_STATE            0xFD48  //(Read only) [7:4] : usb3 changed last state. [3:0]  usb3 state


//for MAILBOX
#define REG_OUTDATA0            0xFD50
#define REG_OUTDATA1            0xFD54
#define REG_OUTRDY              0xFD58  //bit[0] : OutReady, bit[1] : OutEmptyIntEn

#define REG_INDATA0             0xFD60
#define REG_INDATA1             0xFD64
#define REG_INRDY               0xFD68  //bit[0] : InReady, bit[1] : InRdyIntEn


//MCU ERROR debug REG
#define REG_MCUERR_PCLSB        0xFD90  //PC[7:0]
#define REG_MCUERR_PCMSB        0xFD91  //PC[15:8]
#define REG_MCUERR_ACC          0xFD92
#define REG_MCUERR_B            0xFD93
#define REG_MCUERR_DPTRLSB      0xFD94  //DPTR[7:0]
#define REG_MCUERR_DPTRMSB      0xFD95  //DPTR[15:8]
#define REG_MCUERR_SP           0xFD96  //SP[7:0]
#define REG_MCUERR_IE           0xFD97  //IE[7:0]
#define REG_MCUERR_EIE          0xFD98  //EIE[7:0]
#define REG_VERA_SIM            0xFD9F
//0xFD99~0xFD9F are reserved.. 


//-----------------------------------------------------
//
//  0xFE00h ~ 0xFEFFh   USB Configuration
//
//-----------------------------------------------------

/* RTS5101 USB Register Definition */
#define REG_USB_SETUP_DEC_INT           0xFE00
#define REG_USB_DMACTL                  0xFE01
#define REG_USB_IRQSTAT0                0xFE02
#define REG_USB_IRQSTAT1                0xFE03
#define REG_USB_IRQEN0                  0xFE04
#define REG_USB_IRQEN1                  0xFE05
#define REG_USB_AUTOPTRL                0xFE06
#define REG_USB_AUTOPTRH                0xFE07
#define REG_USB_AUTODAT                 0xFE08

#define REG_USB_SCRATCH0                0xFE09
#define REG_USB_SCRATCH1                0xFE0A
#define REG_USB_SEEPROM                 0xFE0B
#define REG_USB_GPIO0                   0xFE0C
#define REG_USB_GPIO0DIR                0xFE0D
#define REG_USB_CLKSEL                  0xFE0E
#define REG_USB_BOOTCTL                 0xFE0F

#define REG_USB_USBCTL                  0xFE10
#define REG_USB_USBSTAT                 0xFE11
#define REG_USB_DEVADDR                 0xFE12
#define REG_USB_USBTEST                 0xFE13
#define REG_USB_FNUM0                   0xFE14
#define REG_USB_FNUM1                   0xFE15

#define REG_USB_EP_IDX                  0xFE20
#define REG_USB_EP_CFG                  0xFE21
#define REG_USB_EP_CTL                  0xFE22
#define REG_USB_EP_STAT                 0xFE23
#define REG_USB_EP_IRQ                  0xFE24
#define REG_USB_EP_IRQEN                0xFE25
#define REG_USB_EP_MAXPKT0              0xFE26
#define REG_USB_EP_MAXPKT1              0xFE27
#define REG_USB_EP_DAT                  0xFE28
#define REG_USB_EP_BC0                  0xFE29
#define REG_USB_EP_BC1                  0xFE2A
#define REG_USB_EP_TC0                  0xFE2B
#define REG_USB_EP_TC1                  0xFE2C
#define REG_USB_EP_TC2                  0xFE2D
#define REG_USB_EP_CTL2                 0xFE2E

#define REG_USB_INFO                    0xFE17
#define REG_USB_SPECIAL_OPTION          0xFE55
#define REG_USB_DMA_AGG_TO              0xFE5B
#define REG_USB_AGG_TO                  0xFE5C
#define REG_USB_AGG_TH                  0xFE5D

#define REG_USB_VID                     0xFE60
#define REG_USB_PID                     0xFE62
#define REG_USB_OPT                     0xFE64
#define REG_USB_CONFIG                  0xFE65  //RX EP setting. 0xFE65 Bit[3:0] : RXQ, Bit[7:4] : INTQ
                                                //TX EP setting. 0xFE66 Bit[3:0] : TXQ0, Bit[7:4] : TXQ1, 0xFE67 Bit[3:0] : TXQ2
#define REG_USB_PHY_PARA1               0xFE68  //Bit[7:4]: XCVR_SEN  (USB PHY 0xE2[7:4]), Bit[3:0]: XCVR_SH    (USB PHY 0xE2[3:0])
#define REG_USB_PHY_PARA2               0xFE69  //Bit[7:5]: XCVR_BG   (USB PHY 0xE3[5:3]), Bit[4:2]: XCVR_DR    (USB PHY 0xE3[2:0]), Bit[1]: SE0_LVL       (USB PHY 0xE5[7]), Bit[0]:  FORCE_XTL_ON  (USB PHY 0xE5[1])
#define REG_USB_PHY_PARA3               0xFE6A  //Bit[7:5]: XCVR_SRC  (USB PHY 0xE5[4:2]), Bit[4]: LATE_DLLEN   (USB PHY 0xF0[4]), Bit[3]: HS_LP_MODE   (USB PHY 0xF0[3]), Bit[2]: UTMI_POS_OUT (USB PHY 0xF1 [7]), Bit[1:0]: TX_DELAY   (USB PHY 0xF1 [2:1])
#define REG_USB_PHY_PARA4               0xFE6B  //(USB PHY 0xE7[7:0])
#define REG_USB_OPT2                    0xFE6C  
#define REG_USB_MAC_ADDR                0xFE70  //0xFE70~0xFE75
#define REG_USB_MANUFACTURE_SETTING     0xFE80  //0xFE80~0xFE90  Max : 32 bytes
#define REG_USB_PRODUCT_STRING          0xFEA0  //0xFEA0~0xFECF  Max : 48 bytes
#define REG_USB_SERIAL_NUMBER_STRING    0xFED0  //0xFED0~0xFEDF  Max : 12 bytes

#define REG_USB_ALTERNATE_SETTING       0xFE4F 
#define REG_USB_INT_BINTERVAL           0xFE6E
#define REG_USB_GPS_EP_CONFIG           0xFE6D

#endif  //__HAL_COM_REG_H__