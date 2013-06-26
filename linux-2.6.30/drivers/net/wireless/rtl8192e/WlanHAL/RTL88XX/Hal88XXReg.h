#ifndef __RTL88XX_REG_H__
#define __RTL88XX_REG_H__

/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal88XXReg.h
	
Abstract:
	Defined RTL88XX Register Offset & Marco & Bit define
	    
Major Change History:
	When            Who                      What
	---------- ---------------  -------------------------------
	2012-03-23  Filen                      Create.	
	2012-03-29  Lun-Wu Yeh          Add Tx/Rx Desc Reg
--*/

/*--------------------------Define -------------------------------------------*/

#if 0 //Filen:
//-----------------------------------------------------
//
//  0x0000h ~ 0x00FFh   System Configuration
//
//-----------------------------------------------------
#define REG_SYS_ISO_CTRL            0x0000
#define REG_SYS_FUNC_EN             0x0002
#define REG_PSC_FSM                 0x0004
#define REG_SYS_CLKR                0x0008
#define REG_9346CR                  0x000A
#define REG_EE_VPD                  0x000C
#define REG_AFE_MISC                0x0010
#define REG_SPS0_CTRL               0x0011
#define REG_SPS1_CTRL               0x0018
#define REG_RSV_CTRL                0x001C
#define REG_RF_CTRL                 0x001F
#define REG_LDOV12D_CTRL            0x0021
#define REG_LDOHCI12_CTRL           0x0022 
#define REG_LPLDO_CTRL              0x0023
#define REG_AFE_XTAL_CTRL           0x0024
#define REG_AFE_PLL_CTRL            0x0028
#define REG_APE_PLL_CTRL_EXT        0x002C
#define REG_EFUSE_CTRL              0x0030
#define REG_EFUSE_TEST              0x0034
#define REG_PWR_DATA                0x0038
#define REG_CAL_TIMER               0x003C
#define REG_ACLK_MON                0x003E
#define REG_GPIO_MUXCFG             0x0040
#define REG_GPIO_PIN_CTRL           0x0044
#define REG_GPIO_INTM               0x0048
#define REG_LEDCFG                  0x004C

//FSIMR
#define REG_SYSIMR0                 0x0050
#define REG_SYSIMR1                 0x0051
#define REG_SYSIMR2                 0x0052
#define REG_SYSIMR3                 0x0053

//FSISR
#define REG_SYSISR0                 0x0054
#define REG_SYSISR1                 0x0055
#define REG_SYSISR2                 0x0056
#define REG_SYSISR3                 0x0057

#define REG_GSSR                    0x006C
#define REG_SYS_SDIO_CTRL           0x0070
#define REG_HCI_OPT_CTRL            0x0074
#define REG_AFE_XTAL_CTRL_EXT       0x0078

#define REG_8051FWDL                0x0080
#define REG_RPWM                    0x0083
#define REG_MCUTSTCFG               0x0084

#define REG_HMEBOX_E0               0x0088			
#define REG_HMEBOX_E1               0x008A
#define REG_HMEBOX_E2               0x008C
#define REG_HMEBOX_E3               0x008E

#define REG_WL_LPS_CTRL             0x0090
#define REG_RPWM2                   0x009E
#define REG_HIMR0                   0x00B0
#define REG_HISR0                   0x00B4
#define REG_HIMR1                   0x00B8
#define REG_HISR1                   0x00BC

#define REG_EFUSE_DATA0             0x00CC
#define REG_EFUSE_DATA1             0x00CD
#define REG_EPPR                    0x00CF
#define REG_BIST_SCAN               0x00D0
#define REG_BIST_ROM_RPT            0x00D8
#define REG_USB_SIE_INTF            0x00E0
#define REG_PCIE_MIO_INTF           0x00E4
#define REG_PCIE_MIO_INTD           0x00E8
#define REG_HPON_FSM                0x00EC
#define REG_SYS_CFG                 0x00F0
#define REG_SYS_STATUS_RPT	        0x00F4
#define REG_SYS_CFG1                0x00FC
#define REG_ROM_VERSION             0x00FD

//-----------------------------------------------------
//
//  0x0100h ~ 0x01FFh   MACTOP General Configuration
//
//-----------------------------------------------------
#define REG_CR                      0x0100//KaiYuan this name is the same with that in DataSheet, not function enable
#define REG_PBP                     0x0104//Filen change to this ID according to DataSheet
#define REG_PKT_BUFF_ACCESS_CTRL    0x0106
#define REG_PLAYFORM_CLOCK          0x0109
#define REG_TRX_DMA_CTRL            0x010C
#define REG_SMPS_ENABLE             0x0110
#define REG_TRXFF_BNDY              0x0114    //Filen change to this ID according to DataSheet
#define REG_TRXFF_STATUS            0x0118//KaiYuan change to this ID according to DataSheet
#define REG_RXFF_PTR                0x011C
#define REG_HIMR                    0x0120
#define REG_HISR                    0x0124
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
#define REG_FTIMR0                  0x0138
#define REG_FTIMR1                  0x0139
#define REG_FTIMR2                  0x013A
#define REG_FTIMR3                  0x013B

//FTISR
#define REG_FTISR0                  0x013C
#define REG_FTISR1                  0x013D
#define REG_FTISR2                  0x013E
#define REG_FTISR3                  0x013F

#define REG_PKTBUF_DBG_CTRL         0x0140
#define REG_PKTBUF_DBG_DATA_L       0x0144
#define REG_PKTBUF_DBG_DATA_H       0x0148
#define REG_CPWM2                   0x014C

//
//  General Purpose Timer (Offset: 0x0150 - 0x016Fh)
//
#define REG_TC0_CTRL                0x0150
#define REG_TC1_CTRL                0x0154
#define REG_TC2_CTRL                0x0158
#define REG_TC3_CTRL                0x015C
#define REG_TC4_CTRL                0x0160
#define REG_TCUNIT_BASE             0x0164

#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8723A|SUPPORT_CHIP_8188E)
//#if BIT1 & BIT1
//#if (BIT(1)|BIT(2)) & (BIT(1))
//#if (BIT1|BIT2|BIT3) & (BIT1|BIT2)
//#if (BIT1|BIT2|BIT3|BIT4) & (BIT1|BIT2)
//0x168 ~ 0x16F: there are no control circuit. But these register can be R/W.
#endif

#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8881A|SUPPORT_CHIP_8812A|SUPPORT_CHIP_8821A|SUPPORT_CHIP_8723B|SUPPORT_CHIP_8192E)
#define REG_TC5_CTRL                0x0168
#define REG_TC6_CTRL                0x016C
#endif

#define REG_DRF_MBIST_FAIL          0x0170
#define REG_MBIST_START             0x0174
#define REG_MBIST_DONE              0x0178
#define REG_MBIST_FAIL              0x017C
#define REG_AES_DECRPT_DATA         0x0180
#define REG_AES_DECRPT_CFG          0x0184
#define REG_SHA1_DATA               0x0188
#define REG_SHA1_CFG                0x018C

//32K control reg
#define REG_TMCR                    0x0190
#define REG_32KCTRL1                0x0194
#define REG_32KCTRL2                0x0198
#define REG_32KCTRL3                0x019C

//0x1A0 ~ 0x1AF are used for C2H
#define REG_C2H_HEADER              0x01A0
#define REG_C2H_TRIGGER             0x01AF

#define REG_UPHY_CPTRIM_CTRL        0x01B0
#define REG_EFUSE_HW_CTRL           0x01B4

//0x1B8 ~ 0x1BF: there are no control circuit. But these register can be R/W.
//0x1C0 ~ 0x1C7: there are no control circuit. But these register can be R/W.


#define REG_FMETHR                  0x01C8
#define REG_HMETFR                  0x01CC//KaiYuan Change name
#define REG_HOST_RCV_MESSAGE        0x01CF


//0x1D0 ~0x1DF are H2C message box
#define REG_HMEBOX_0                0x01D0//KaiYuan change name
#define REG_HMEBOX_1                0x01D4//KaiYuan change name
#define REG_HMEBOX_2                0x01D8//KaiYuan change name
#define REG_HMEBOX_3                0x01DC//KaiYuan change name

//LLT
#define REG_LLT_INIT                0x01E0
#define REG_LLT_INIT_HDATA          0x01E0
#define REG_LLT_INIT_ADDR           0x01E1
#define REG_LLT_INIT_PDATA          0x01E2
#define REG_LLTE_RWM                0x01E3

//BB Indirect Access
#define REG_BB_ACCEESS_CTRL         0x01E8 //KaiYuan Change name
#define REG_BB_ACCESS_DATA          0x01EC //KaiYuan Change name

//0x1F0~0x1FF are used for H2C extension
#define REG_H2CEXT_0                0x01F0
#define REG_H2CEXT_1                0x01F4
#define REG_H2CEXT_2                0x01F8
#define REG_H2CEXT_3                0x01FC

//-----------------------------------------------------
//
//  0x0200h ~ 0x027Fh   TXDMA Configuration
//
//-----------------------------------------------------
#define REG_RQPN                    0x0200
#define REG_FIFOPAGE                0x0204
#define REG_TDECTL                  0x0208
#define REG_TXDMA_OFFSET_CHECK      0x020C
#define REG_TXDMA_STATUS            0x0210
#define REG_RQPN_NPQ                0x0214

//-----------------------------------------------------
//
//  0x0280h ~ 0x02FFh   RXDMA Configuration
//
//-----------------------------------------------------
#define REG_RXDMA_AGG_PG_TH         0x0280
#define REG_WOW_RXPKT_NUM           0x0282
#define REG_RXPKT_NUM               0x0284
#define REG_RXDMA_STATUS            0x0288
#define REG_RXDMA_DBG               0x028C
#define REG_RXDMA_PRO               0x0290
//-----------------------------------------------------
//
// BT_COEX
//
//-----------------------------------------------------
#define REG_BT_COEX_GLB_CTRL        0x02C0
#define REG_BT_COEX_TBL             0x02C4
#define REG_BT_SSI                  0x02D4
#define REG_BT_CSR_ENH_INTF_CTRL    0x02D6
#define REG_BT_ACT_STATISTICS       0x02D8
#define REG_BT_CMD_WLAN_RPT         0x02E0
#define REG_BT_CMD_BT_RPT           0x02E1
#define REG_BT_CMD_LATCH            0x02E2

//0x02F0~ 0x02FF: there are no control circuit. But these register can be R/W.


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
#define REG_PICE_CTRL2              0x03DB  // 1 Bytes
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
#define REG_TX_ACQ_EMPTY            0x041A
#define REG_TX_MGQ_EMPTY            0x041B
#define REG_CPU_MGQ_INFORMATION     0x041C
#define REG_FUNCTION_ENABLE         0x0420
#define REG_HWSEQ_CTRL              0x0423
#define REG_TXPKTBUF_BCNQ_BDNY      0x0424
#define REG_TXPKTBUF_MGQ_BDNY       0x0425
#define REG_LIFETIME_CTRL           0x0426
#define REG_FREE_TAIL               0x0427
#define REG_SPEC_SIFS               0x0428
#define REG_RL                      0x042A
#define REG_TXBF_CTRL               0x042C
#define REG_DARFRC                  0x0430
#define REG_RARFRC                  0x0438
#define REG_RRSR                    0x0440
#define REG_ARFR0                   0x0444
#define REG_ARFR1                   0x0448
#define REG_ARFR2                   0x044C
#define REG_ARFR3                   0x0450
#define REG_CCK_CHECK               0x0454
#define REG_AMPDU_MAX_TIME          0x0456
#define REG_AMPDU_MAX_LENGTH        0x0458
#define REG_ACQ_STOP                0x045C
#define REG_WMAC_LBK_BUF_HD         0x045D
#define REG_TX_HANG_CTRL            0x045E
#define REG_FAST_EDCA_CTRL          0x0460
#define REG_RD_RESP_PKT_TH          0x0463
#define REG_Q4_INFORMATION          0x0468
#define REG_Q5_INFORMATION          0x046C
#define REG_Q6_INFORMATION          0x0470
#define REG_Q7_INFORMATION          0x0474
#define REG_SPCRPT_CTRL             0x047C
#define REG_SPC_W_PTR               0x047E
#define REG_SPC_R_PTR               0x047F
#define REG_INIRTS_RATE_SEL         0x0480
#define REG_INI_BASIC_CFEND_SEL     0x0481
#define REG_INI_STBC_CFEND_SEL      0x0482
#define REG_MACID_PKT_SLEEP_3       0x0484
#define REG_MACID_PKT_SLEEP_1       0x0488
#define REG_ARFR2                   0x048C
#define REG_ARFR3                   0x0494
#define REG_ARFR4                   0x049C
#define REG_ARFR5                   0x04A4
#define REG_TXRPT_Start_Offset      0x04AC
#define REG_TRYING_CNT_TH           0x04B0
#define REG_POWER_STAGE1            0x04B4
#define REG_POWER_STAGE2            0x04B8
#define REG_SW_AMPDU_BURST_MODE     0x04BC
#define REG_PKT_LIFE_TIME           0x04C0
#define REG_STBC_SETTING            0x04C4
#define REG_QUEUE_CTRL              0x04C6
#define REG_PROT_MODE_CTRL          0x04C8
#define REG_BAR_MODE_CTRL           0x04CC
#define REG_RA_TRY_RATE_AGG_LMT     0x04CF
#define REG_EARLY_MODE_CTRL         0x04D0//KaiYuan change name

#define REG_HW_SEQ0                 0x04D8
#define REG_HW_SEQ1                 0x04DA
#define REG_NQOS_SEQ                0x04DC//KaiYuan change name
#define REG_QOS_SEQ                 0x04DE//KaiYuan change name
#define REG_NEED_CPU_HANDLE         0x04E0
#define REG_PTCL_ERR_STATUS         0x04E2
#define REG_TX_PKT_NUM              0x04E3
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
#define REG_BCNDROPCTRL	            0x0574
#define REG_HGQ_TIMEOUT_PERIOD      0x0575

#define REG_PS_TIMER                0x0580
#define REG_TIMER0                  0x0584//KaiYuan change name
#define REG_TIMER1                  0x0588//KaiYuan change name
#define REG_TBTT_CTN_AREA           0x058C
#define REG_FORCE_BCN_IFS           0x058E
#define REG_ACMHWCTRL               0x05C0
#define REG_ACMRSTCTRL              0x05C1
#define REG_ACMAVG                  0x05C2
#define REG_VO_ADMTIME              0x05C4
#define REG_VI_ADMTIME              0x05C6
#define REG_BE_ADMTIME              0x05C8
#define REG_EDCA_RANDOM_GEN         0x05CC
#define REG_TXCMD_SEL               0x05CF
#define REG_SCH_TXCMD               0x05F8
#define REG_PAGE5_DUMMY             0x05FC

//-----------------------------------------------------
//
//  0x0600h ~ 0x07FFh   WMAC Configuration
//
//-----------------------------------------------------
#define REG_WMAC_CR                 0x0600
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
#define REG_PS_RX_INFO              0x0692
#define REG_WMMPS_UAPSD_TID         0x0693
#define REG_WKFMCAM_NUM             0x0698
#define REG_RXFLTMAP0               0x06A0
#define REG_RXFLTMAP1               0x06A2
#define REG_RXFLTMAP2               0x06A4
#define REG_BCN_PSR_RPT             0x06A8
#define REG_CALB32K_CTRL            0x06AC //spec no description, check RTL!!
#define REG_PKT_MON_CTRL            0x06B4
#define REG_BT_COEX_TABLE           0x06C0
#define REG_WMAC_RESP_TXINFO        0x06D8

#define REG_MACID1                  0x0700
#define REG_BSSID1                  0x0708

#define REG_RX_CSI_RPT_INFO         0x071C


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
#endif


//2 SYS_FUNC_EN
#define FEN_BBRSTB				BIT(0)
#define FEN_BB_GLB_RSTn			BIT(1)
#define FEN_USBA				BIT(2)
#define FEN_UPLL				BIT(3)
#define FEN_USBD				BIT(4)
#define FEN_DIO_PCIE			BIT(5)
#define FEN_PCIEA				BIT(6)
#define FEN_PPLL				BIT(7)
#define FEN_PCIED				BIT(8)
#define FEN_DIOE				BIT(9)
#define FEN_CPUEN				BIT(10)
#define FEN_DCORE				BIT(11)
#define FEN_ELDR				BIT(12)
#define FEN_DIO_RF				BIT(13)
#define FEN_HWPDN				BIT(14)
#define FEN_MREGEN				BIT(15)


//----------------------------------------------------------------------------
//       8192C (MSR) Media Status Register	(Offset 0x4C, 8 bits)  
//----------------------------------------------------------------------------
/*
Network Type
00: No link
01: Link in ad hoc network
10: Link in infrastructure network
11: AP mode
Default: 00b.
*/
#define	MSR_NOLINK				0x00
#define	MSR_ADHOC				0x01
#define	MSR_INFRA				0x02
#define	MSR_AP					0x03

//----------------------------------------------------------------------------
//       8192C MCUFWDL bits						(Offset 0x80-83, 32 bits)
//----------------------------------------------------------------------------
#define		RPWM_SHIFT		24	// Host Request Power State.
#define		RPWM_Mask			0x0FF
#define		CPRST				BIT(23)	// 8051 Reset Status.
#define		ROM_DLEN			BIT(19)	// ROM Download Enable (8051 Core will be reseted) FPGA only.
#define		ROM_PGE_SHIFT		16	// ROM Page (FPGA only).
#define		ROM_PGE_Mask		0x07
#define		MAC1_RFINI_RDY	BIT(10)	// 92D_REG, MAC1 MCU Initial RF ready
#define		MAC1_BBINI_RDY	BIT(9)	// 92D_REG, MAC1 MCU Initial BB ready
#define		MAC1_MACINI_RDY	BIT(8)	// 92D_REG, MAC1 MCU Initial MAC ready
#define		MCU_STATUS		BIT(7)	// 92D_REG, 1: SRAM, 0: ROM
#define		WINTINI_RDY		BIT(6)	// WLAN Interrupt Initial ready.
#define		MAC0_RFINI_RDY	BIT(5)	// MAC0 MCU Initial RF ready.
#define		MAC0_BBINI_RDY	BIT(4)	// MAC0 MCU Initial BB ready.
#define		MAC0_MACINI_RDY	BIT(3)	// MAC0 MCU Initial MAC ready.
#define		FWDL_CHKSUM_RPT	BIT(2)	// FWDL CheckSum report, 1: OK, 0 : Faill.
#define		MCUFWDL_RDY		BIT(1)	// Driver set this bit to notify MCU FW Download OK.
#define		MCUFWDL_EN		BIT(0)	// MCU Firmware download enable. 1:Enable, 0:Disable.


//----------------------------------------------------------------------------
//       REG_HIMR bits				(Offset 0xB0-B3, 32 bits)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//       REG_HISR bits				(Offset 0xB4-B7, 32 bits)
//----------------------------------------------------------------------------
#define	HIMR_TXCCK				BIT(30)		// TXRPT interrupt when CCX bit of the packet is set	
#define	HIMR_PSTIMEOUT			BIT(29)		// Power Save Time Out Interrupt
#define	HIMR_GTINT4				BIT(28)		// When GTIMER4 expires, this bit is set to 1	
#define	HIMR_GTINT3				BIT(27)		// When GTIMER3 expires, this bit is set to 1	
#define	HIMR_TBDER				BIT(26)		// Transmit Beacon0 Error			
#define	HIMR_TBDOK				BIT(25)		// Transmit Beacon0 OK, ad hoc only
#define	HIMR_TSF_BIT32_TOGGLE	BIT(24)		// TSF Timer BIT32 toggle indication interrupt			
#define	HIMR_BcnInt				BIT(20)		// Beacon DMA Interrupt 0			
#define	HIMR_BDOK				BIT(16)		// Beacon Queue DMA OK0			
#define	HIMR_HSISR_IND_ON_INT	BIT(15)		// HSISR Indicator (HSIMR & HSISR is true, this bit is set to 1)			
#define	HIMR_BCNDMAINT_E		BIT(14)		// Beacon DMA Interrupt Extension for Win7			
#define	HIMR_ATIMEND			BIT(12)		// CTWidnow End or ATIM Window End
#define	HIMR_HISR1_IND_INT		BIT(11)		// HISR1 Indicator (HISR1 & HIMR1 is true, this bit is set to 1)
#define	HIMR_C2HCMD				BIT(10)		// CPU to Host Command INT Status, Write 1 clear	
#define	HIMR_CPWM2				BIT(9)		// CPU power Mode exchange INT Status, Write 1 clear	
#define	HIMR_CPWM				BIT(8)		// CPU power Mode exchange INT Status, Write 1 clear	
#define	HIMR_HIGHDOK			BIT(7)		// High Queue DMA OK	
#define	HIMR_MGNTDOK			BIT(6)		// Management Queue DMA OK	
#define	HIMR_BKDOK				BIT(5)		// AC_BK DMA OK		
#define	HIMR_BEDOK				BIT(4)		// AC_BE DMA OK	
#define	HIMR_VIDOK				BIT(3)		// AC_VI DMA OK		
#define	HIMR_VODOK				BIT(2)		// AC_VO DMA OK	
#define	HIMR_RDU				BIT(1)		// Rx Descriptor Unavailable	
#define	HIMR_ROK				BIT(0)		// Receive DMA OK


//----------------------------------------------------------------------------
//       REG_HIMRE bits			(Offset 0xB8-BB, 32 bits)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//       REG_HIMSE bits			(Offset 0xBC-BF, 32 bits)
//----------------------------------------------------------------------------
#define	HIMRE_BCNDMAINT7		BIT(27)		// Beacon DMA Interrupt 7
#define	HIMRE_BCNDMAINT6		BIT(26)		// Beacon DMA Interrupt 6
#define	HIMRE_BCNDMAINT5		BIT(25)		// Beacon DMA Interrupt 5
#define	HIMRE_BCNDMAINT4		BIT(24)		// Beacon DMA Interrupt 4
#define	HIMRE_BCNDMAINT3		BIT(23)		// Beacon DMA Interrupt 3
#define	HIMRE_BCNDMAINT2		BIT(22)		// Beacon DMA Interrupt 2
#define	HIMRE_BCNDMAINT1		BIT(21)		// Beacon DMA Interrupt 1
#define	HIMRE_BCNDOK7			BIT(20)		// Beacon Queue DMA OK Interrup 7
#define	HIMRE_BCNDOK6			BIT(19)		// Beacon Queue DMA OK Interrup 6
#define	HIMRE_BCNDOK5			BIT(18)		// Beacon Queue DMA OK Interrup 5
#define	HIMRE_BCNDOK4			BIT(17)		// Beacon Queue DMA OK Interrup 4
#define	HIMRE_BCNDOK3			BIT(16)		// Beacon Queue DMA OK Interrup 3
#define	HIMRE_BCNDOK2			BIT(15)		// Beacon Queue DMA OK Interrup 2
#define	HIMRE_BCNDOK1			BIT(14)		// Beacon Queue DMA OK Interrup 1
#define	HIMRE_ATIMEND_E			BIT(13)		// ATIM Window End Extension for Win7
#define	HIMRE_TXERR				BIT(11)		// Tx Error Flag Interrupt Status, write 1 clear.
#define	HIMRE_RXERR				BIT(10)		// Rx Error Flag INT Status, Write 1 clear
#define	HIMRE_TXFOVW			BIT(9)		// Transmit FIFO Overflow
#define	HIMRE_RXFOVW			BIT(8)		// Receive FIFO Overflow



//----------------------------------------------------------------------------
//       8192C CR bits						(Offset 0x100-103, 32 bits)
//----------------------------------------------------------------------------
#define		LBMODE_SHIFT		24	// Loopback mode.
#define		LBMODE_Mask		0x0F
#define		NETYPE_SHIFT		16	// Network Type.
#define		NETYPE_Mask		0x03
#define		MAC_SEC_EN		BIT(9)	// Enable MAC security engine.
#define		ENSWBCN			BIT(8)	// Enable SW TX beacon.
#define		MACRXEN			BIT(7)	// MAC Receiver Enable.
#define		MACTXEN			BIT(6)	// MAC Transmitter Enable.
#define		SCHEDULE_EN		BIT(5)	// Schedule Enable.
#define		PROTOCOL_EN		BIT(4)	// protocol Block Function Enable.
#define		RXDMA_EN			BIT(3)	// RXDMA Function Enable.
#define		TXDMA_EN			BIT(2)	// TXDMA Function Enable.
#define		HCI_RXDMA_EN		BIT(1)	// HCI to RXDMA Interface Enable.
#define		HCI_TXDMA_EN		BIT(0)	// HCI to TXDMA Interface Enable.
// Loopback mode.
#define		LB_NORMAL			0x00
#define		LB_MAC				0x0B
#define		LB_MAC_DLY			0x03
#define		LB_PHY				0x01
#define		LB_DMA				0x07
#define		LB_DUAL_MAC		0x1B	// 92D_REG
// Network Type.
#define		NETYPE_NOLINK		0x00
#define		NETYPE_ADHOC		0x01
#define		NETYPE_INFRA		0x02
#define		NETYPE_AP			0x03

//----------------------------------------------------------------------------
//       8192C PBP bits						(Offset 0x104-107, 32 bits)
//----------------------------------------------------------------------------
#define		PSTX_SHIFT			4	// Page size of transmit packet buffer.
#define		PSTX_Mask			0x0F
#define		PSRX_SHIFT			0	// Page size of receive packet buffer and C2HCMD buffer.
#define		PSRX_Mask			0x0F
// Page size
#define		PBP_64B             0x00
#define		PBP_128B			0x01
#define		PBP_256B			0x02
#define		PBP_512B			0x03
#define		PBP_1024B			0x04

#define     PBP_UNIT            128

//----------------------------------------------------------------------------
//       8192C TRXDMA_CTRL bits				(Offset 0x10C-10D, 16 bits)
//----------------------------------------------------------------------------
#define		HPQ_SEL_SHIFT		8	// High Priority Queue Selection.
#define		HPQ_SEL_Mask		0x03F
#define		RXDMA_AGG_EN		BIT(2)	//
#define		RXSHFT_EN			BIT(1)	// When this bit is set, RX shift to fit alignment is enable.
#define		RXDMA_ARBBW_EN	BIT(0)	// Enable RXDMA Arbitrator priority for Host interface.
// High Priority Queue Selection.
#define		HPQ_SEL_VOQ		BIT(0)
#define		HPQ_SEL_VIQ		BIT(1)
#define		HPQ_SEL_BEQ		BIT(2)
#define		HPQ_SEL_BKQ		BIT(3)
#define		HPQ_SEL_MGQ		BIT(4)
#define		HPQ_SEL_HIQ		BIT(5)


//----------------------------------------------------------------------------
//       8192C TRXFF_BNDY bits				(Offset 0x114-117, 32 bits)
//----------------------------------------------------------------------------
#define		RXFF0_BNDY_SHIFT			16	// upper boundary of RXFF0.
#define		RXFF0_BNDY_Mask			0x0FFFF
#define		TXPKTBUF_PGBNDY_SHIFT	0	// From FWHW offload, sets the max pages controlled by TXDMA.
#define		TXPKTBUF_PGBNDY_Mask		0x0FF


//----------------------------------------------------------------------------
//       8192C SPEC_SIFS bits					(Offset 0x428-429, 16 bits)
//----------------------------------------------------------------------------
#define		SPEC_SIFS_OFDM_SHIFT	8	// spec SIFS value for duration calculation.
#define		SPEC_SIFS_OFDM_Mask	0x0FF
#define		SPEC_SIFS_CCK_SHIFT	0	// spec SIFS value for duration calculation.
#define		SPEC_SIFS_CCK_Mask	0x0FF


//----------------------------------------------------------------------------
//       8192C RL bits							(Offset 0x42A-42B, 16 bits)
//----------------------------------------------------------------------------
#define		SRL_SHIFT			8	// Short Retry Limit.
#define		SRL_Mask			0x03F
#define		LRL_SHIFT			0	// Long Retry Limit.
#define		LRL_Mask			0x03F


//----------------------------------------------------------------------------
//       8192C SIFS_CCK bits						(Offset 0x514-515, 16 bits)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//       8192C SIFS_OFDM bits					(Offset 0x516-517, 16 bits)
//----------------------------------------------------------------------------
#define		SIFS_TRX_SHIFT		8	// SIFS time required for any immediate response.
#define		SIFS_TRX_Mask		0x0FF
#define		SIFS_CTX_SHIFT		0	// SIFS time required for consecutive TX events.
#define		SIFS_CTX_Mask		0x0FF


//----------------------------------------------------------------------------
//       8192C BCN_CTRL bits					(Offset 0x550, 8 bits)
//----------------------------------------------------------------------------

#define 	DIS_SUB_STATE		BIT(4)
#define 	DIS_SUB_STATE_N		BIT(1)
#define 	DIS_TSF_UPDATE		BIT(5)
#define 	DIS_TSF_UPDATE_N	BIT(4)
#define 	DIS_ATIM			BIT(0)


#define		BCN0_AUTO_SYNC	BIT(5)	// When this bit is set, TSFTR will update the timestamp in Beacon matched BSSID.
#define		DIS_TSF_UPT		BIT(4)	// 92D_REG, When this bit is set, tsf will not update
#define		EN_BCN_FUNCTION	BIT(3)	// bit=1, TSF and other beacon related functions are then enabled.
#define		EN_TXBCN_RPT		BIT(2)	//
#define		EN_MBSSID			BIT(1)	//
#define		PBCNQSEL			BIT(0)	//

#define     TBTT_prohibit_hold_SHT   8
#define     TBTT_prohibit_hold_Mask  0xfff


//----------------------------------------------------------------------------
//       8192C MBID_NUM bits					(Offset 0x552, 8 bits)
//----------------------------------------------------------------------------
#define		MBID_BCN_NUM_SHIFT	0	// num of virtual interface num excluding the root.
#define		MBID_BCN_NUM_Mask	0x07


//----------------------------------------------------------------------------
//       8192C MBSSID_BCN_SPACE bits			(Offset 0x554-557, 32 bits)
//----------------------------------------------------------------------------
#define		BCN_SPACE2_SHIFT	16	//
#define		BCN_SPACE2_Mask	0x0FFFF
#define		BCN_SPACE1_SHIFT	0	//
#define		BCN_SPACE1_Mask	0x0FFFF


//----------------------------------------------------------------------------
//       8192C TCR bits							(Offset 0x604-607, 32 bits)
//----------------------------------------------------------------------------
#define		TSFT_CMP_SHIFT	16		// TSFT insertion compensation value.
#define		TSFT_CMP_Mask		0x0FF
#define		WMAC_TCR_ERRSTEN3		BIT(15)	// 92D_REG, Use phytxend_ps to reset mactx state machine
#define		WMAC_TCR_ERRSTEN2		BIT(14)	// 92D_REG, If txd fifo underflow when txtype is cmpba, reset mactx state machine
#define		WMAC_TCR_ERRSTEN1		BIT(13)	// 92D_REG, If txd fifo underflow, reset mactx state machine
#define		WMAC_TCR_ERRSTEN0		BIT(12)	// 92D_REG, Phytxend_ps comes but mactx still active, reset mactx state machine
#define		WMAC_TCR_TXSK_PERPKT		BIT(11)	// 92D_REG, Serche key for each mpdu
#define		ICV					BIT(10)	// Integrity Check Value.
#define		CFE_FORM			BIT(9)	// CF-End Frame Format.
#define		CRC					BIT(8)	// Append 32-bit Cyclic Redundancy Check.
#define		PWRBIT_OW_EN		BIT(7)	// bit=1, MAC overwrite pwr bit according to PWR_ST for data frame.
#define		PWR_ST				BIT(6)	// MAC will overwrite pwr bit accroding to PWR_ST for data frame.
#define		HW_DTIM				BIT(5)	// Enable mactx to update DTIM count and group bit of beacon TIM
#define		HIQ_HW_MD			BIT(4)	// Enable mactx to update more data subfield of high queue packet
#define		PAD_SEL			BIT(2)	// AMPDU Padding pattern selection.
#define		DIS_GCLK			BIT(1)	// Disable MACTX clock gating control.
#define		TSFRST				BIT(0)	// Reset TSF Timer to zero.


//----------------------------------------------------------------------------
//       8192C RCR bits							(Offset 0x608-60B, 32 bits)
//----------------------------------------------------------------------------
#define		RCR_APP_FCS		BIT(31)	// wmac RX will append FCS after payload.
#define		RCR_APP_MIC		BIT(30)	// bit=1, MACRX will retain the MIC at the bottom of the packet.
#define		RCR_APP_ICV		BIT(29)	// bit=1, MACRX will retain the ICV at the bottom of the packet.
#define		RCR_APP_PHYSTS	BIT(28)	// Append RXFF0 PHY Status Enable.
#define		RCR_APP_BASSN		BIT(27)	// Append SSN of previous TXBA Enable.
#define		RCR_MBID_EN		BIT(24)	// Enable Multiple BssId.
#define		RCR_LSIGEN			BIT(23)	// Enable LSIG TXOP Protection function.
#define		RCR_MFBEN			BIT(22)	// Enable immediate MCS Feedback function.
#define		RCR_BM_DATA_EN	BIT(17)	// BM_DATA_EN.
#define		RCR_UC_DATA_EN	BIT(16)	// Unicast data packet interrupt enable.
#define		RCR_HTC_LOC_CTRL	BIT(14)	// 1: HTC -> MFC, 0: MFC-> HTC.
#define		RCR_AMF			BIT(13)	// Accept Management Frame.
#define		RCR_ACF			BIT(12)	// Accept Control Frame.
#define		RCR_ADF			BIT(11)	// Accept Data Frame.
#define		RCR_AICV			BIT(9)	// Accept Integrity Check Value Error packets.
#define		RCR_ACRC32			BIT(8)	// Accept CRC32 Error packets.

#define		RCR_CBSSID_ADHOC		(BIT(6)|BIT(7))	// Check BSSID.
#define		RCR_CBSSID			BIT(6)	// Check BSSID.
#define		RCR_APWRMGT		BIT(5)	// Accept Power Management Packet.
#define		RCR_ADD3			BIT(4)	// Accept Address 3 Match Packets.
#define		RCR_AB				BIT(3)	// Accept Broadcast packets.
#define		RCR_AM				BIT(2)	// Accept Multicast packets.
#define		RCR_APM			BIT(1)	// Accept Physical Match packets.
#define		RCR_AAP			BIT(0)	// Accept Destination Address packets.


// 		MACID Setting Register. (Offset 0x610 - 0x62Fh)
//----------------------------------------------------------------------------
//       8192C MBIDCAMCFG bits					(Offset 0x628-62F, 64 bits)
//----------------------------------------------------------------------------
#define		MBIDCAM_POLL		BIT(31)	// Pooling bit.
#define		MBIDWRITE_EN		BIT(30)	// Write Enable.
#define		MBIDCAM_ADDR_SHIFT	24	// CAM Address.
#define		MBIDCAM_ADDR_Mask	0x01F
#define		MBIDCAM_VALID		BIT(23)	// CAM Valid bit.



//----------------------------------------------------------------------------
//       8192C CAMCMD bits						(Offset 0x670-673, 32 bits)
//----------------------------------------------------------------------------
#define		SECCAM_POLL			BIT(31)	// Security CAM Polling.
#define		SECCAM_CLR				BIT(30)	// Set 1 to clear all valid bits in CAM.
#define		MFBCAM_CLR			BIT(29)	// Write 1 to clear all MFB value in CAM.
#define		SECCAM_WE				BIT(16)	// Security CAM Write Enable.
#define		SECCAM_ADDR_SHIFT	0	// Security CAM Address Offset.
#define		SECCAM_ADDR_Mask		0x0FF



//----------------------------------------------------------------------------
//       8192C SECCFG bits						(Offset 0x680, 8 bits)
//----------------------------------------------------------------------------
#define		CHK_KEYID			BIT(8)	// Key search engine need to check if key ID matched
#define		RXBCUSEDK			BIT(7)	// Force RX Broadcast packets Use Default Key
#define		TXBCUSEDK			BIT(6)	// Force Tx Broadcast packets Use Default Key
#define		NOSKMC				BIT(5)	// No Key Search for Multicast.
#define		SKBYA2				BIT(4)	// Search Key by A2.
#define		RXDEC				BIT(3)	// Enable Rx Decryption.
#define		TXENC				BIT(2)	// Enable Tx Encryption.
#define		RXUSEDK			BIT(1)	// Force Rx Use Default Key.
#define		TXUSEDK			BIT(0)	// Force Tx Use Default Key.

//----------------------------------------------------------------------------
//       8192E REG_92E_CNT_FORMAT bits           (Offset 0x7B8-7BB, 32 bits)
//----------------------------------------------------------------------------
#define     CNT_EN				BIT(0)
#define     CNT_RST				BIT(1)
#define     CNT_BSSIDMAP_MASK	0xf
#define     CNT_BSSIDMAP_SHIFT	4
#define     CNT_BSSIDMAP_CNT0	0
#define     CNT_BSSIDMAP_CNT1   1
#define     CNT_BSSIDMAP_CNT2   2
#define     CNT_BSSIDMAP_CNT3   3
#define     CNT_BSSIDMAP_CNT4   4
#define     CNT_BSSIDMAP_CNT5   5
#define     CNT_BSSIDMAP_CNT6   6
#define     CNT_BSSIDMAP_CNT7   7
#define     CNT_TXCTRL_MASK		0xffff
#define     CNT_TXCTRL_SHIFT	0
#define     CNT_RXCTRL_MASK		0xffff
#define     CNT_RXCTRL_SHIFT	16

//----------------------------------------------------------------------------
//       8192E REG_92E_FUNCTRL bits           (Offset 0x7BC-7BD, 16 bits)
//----------------------------------------------------------------------------	
#define     FUNCTRL_ADDR_MASK		0xff
#define     FUNCTRL_ADDR_SHIFT		0
#define     FUNCTRL_ALLCNTEN		BIT(8)
#define     FUNCTRL_ALLCNTRST		BIT(9)
#define     FUNCTRL_ADDR_CNT0CTRL	0x00
#define     FUNCTRL_ADDR_CNT0TRX	0x01
#define     FUNCTRL_ADDR_CNT1CTRL   0x10
#define     FUNCTRL_ADDR_CNT1TRX    0x11
#define     FUNCTRL_ADDR_CNT2CTRL   0x20
#define     FUNCTRL_ADDR_CNT2TRX    0x21
#define     FUNCTRL_ADDR_CNT3CTRL   0x30
#define     FUNCTRL_ADDR_CNT3TRX    0x31
#define     FUNCTRL_ADDR_CNT4CTRL   0x40
#define     FUNCTRL_ADDR_CNT4TRX    0x41
#define     FUNCTRL_ADDR_CNT5CTRL   0x50
#define     FUNCTRL_ADDR_CNT5TRX    0x51
#define     FUNCTRL_ADDR_CNT6CTRL   0x60
#define     FUNCTRL_ADDR_CNT6TRX    0x61
#define     FUNCTRL_ADDR_CNT7CTRL   0x70
#define     FUNCTRL_ADDR_CNT7TRX    0x71


#endif  //__RTL88XX_REG_H__
