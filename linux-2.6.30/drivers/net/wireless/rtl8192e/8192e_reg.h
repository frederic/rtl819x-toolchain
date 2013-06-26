/*
 *  Header file of 8188E register
 *
 *	 $Id: 8188e_reg.h,v 1.1 2011/06/30 11:02:56 victoryman Exp $
 *
 *  Copyright (c) 2011 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef _8192E_REG_H_
#define _8192E_REG_H_

//============================================================
//       8188E Regsiter offset definition
//============================================================


/*
 *	Project RTL8188E follows most of registers in Project RTL8192c
 *	This file includes additional registers for RTL8188E only
 *	Header file of RTL8192C registers should always be included
 */

//
// 1. System Configure Register. (Offset 0x000 - 0x0FFh)
//
#define		REG_92E_BB_PAD_CTRL			0x64
#define		REG_92E_HMEBOX_E0			0x88
#define		REG_92E_HMEBOX_E1			0x8A
#define		REG_92E_HMEBOX_E2			0x8C
#define		REG_92E_HMEBOX_E3			0x8E
#define		REG_92E_WLLPS_CTRL			0x90
#define		REG_92E_RPWM2				0x9E
#define		REG_92E_HIMR				0xB0
#define		REG_92E_HISR				0xB4
#define		REG_92E_HIMRE				0xB8
#define		REG_92E_HISRE				0xBC
#define		REG_92E_EFUSE_DATA1			0xCC
#define		REG_92E_EFUSE_DATA0			0xCD
#define		REG_92E_EPPR				0xCF

#define		REG_92E_TDECTRL1			0x228

#ifdef NEW_92E_PAGE3_REG
#define		REG_92E_MGQ_DESA			0x310
#define		REG_92E_VOQ_DESA			0x318
#define		REG_92E_VIQ_DESA			0x320
#define		REG_92E_BEQ_DESA			0x328
#define		REG_92E_BKQ_DESA			0x330
#define 	REG_92E_RX_DESA 			0x338
#define		REG_92E_HI0Q_DESA			0x340
#define		REG_92E_HI1Q_DESA			0x348
#define		REG_92E_HI2Q_DESA			0x350
#define		REG_92E_HI3Q_DESA			0x358
#define		REG_92E_HI4Q_DESA			0x360
#define		REG_92E_HI5Q_DESA			0x368
#define		REG_92E_HI6Q_DESA			0x370
#define		REG_92E_HI7Q_DESA			0x378
#define		REG_92E_ACQ_DES_NUM0		0x384
#define		REG_92E_ACQ_DES_NUM1		0x388
#define		REG_92E_ACQ_DES_NUM2		0x380
#define		REG_92E_HQ_DES_NUM0			0x38C
#define		REG_92E_HQ_DES_NUM1			0x390
#define		REG_92E_HQ_DES_NUM2			0x394
#define		REG_92E_HQ_DES_NUM3			0x398
#define		REG_92E_TSFT_CLRQ			0x39C
#define		REG_92E_VOQ_IDX				0x3A0
#define		REG_92E_VIQ_IDX				0x3A4
#define		REG_92E_BEQ_IDX				0x3A8
#define		REG_92E_BKQ_IDX				0x3AC
#define		REG_92E_MGQ_IDX				0x3B0
#define		REG_92E_RXQ_IDX				0x3B4
#define		REG_92E_HI0Q_IDX			0x3B8
#define		REG_92E_HI1Q_IDX			0x3BC
#define		REG_92E_HI2Q_IDX			0x3C0
#define		REG_92E_HI3Q_IDX			0x3C4
#define		REG_92E_HI4Q_IDX			0x3C8
#define		REG_92E_HI5Q_IDX			0x3CC
#define		REG_92E_HI6Q_IDX			0x3D0
#define		REG_92E_HI7Q_IDX			0x3D4
#else
#define		REG_92E_WATCHDOG			0x35C
#define		REG_92E_VOQ_IDX				0x310
#define		REG_92E_VIQ_IDX				0x314
#define		REG_92E_MGQ_DESA			0x318
#define		REG_92E_VOQ_DESA			0x320
#define		REG_92E_VIQ_DESA			0x328
#define		REG_92E_BEQ_DESA			0x330
#define		REG_92E_BKQ_DESA			0x338
#define		REG_92E_PCIE_HRPWM			0x361
#define		REG_92E_PCIE_CLK_RECOVER	0x362
#define		REG_92E_PCIE_HCPWM			0x363
#define		REG_92E_BEQ_IDX				0x364
#define		REG_92E_BKQ_IDX				0x368
#define		REG_92E_MGQ_IDX				0x36C
#define		REG_92E_HI0Q_IDX			0x370
#define		REG_92E_HI1Q_IDX			0x374
#define		REG_92E_HI2Q_IDX			0x378
#define		REG_92E_HI3Q_IDX			0x37C
#define		REG_92E_PCIE_HRPWM2			0x380
#define		REG_92E_PCIE_HCPWM2			0x382
#define		REG_92E_HCI_PCIE_H2C_MSG	0x384
#define		REG_92E_HCI_PCIE_C2H_MSG	0x388
#define		REG_92E_RXQ_IDX				0x38C
#define		REG_92E_HI4Q_IDX			0x390
#define		REG_92E_HI5Q_IDX			0x394
#define		REG_92E_HI6Q_IDX			0x398
#define		REG_92E_HI7Q_IDX			0x39C
#define		REG_92E_HQ_DES_NUM0			0x3A0
#define		REG_92E_HQ_DES_NUM1			0x3A4
#define		REG_92E_HQ_DES_NUM2			0x3A8
#define		REG_92E_HQ_DES_NUM3			0x3AC
#define		REG_92E_TSFT_CLRQ			0x3B0
#define		REG_92E_ACQ_DES_NUM0		0x3B4
#define		REG_92E_ACQ_DES_NUM1		0x3B8
#define		REG_92E_ACQ_DES_NUM2		0x3BC
#define		REG_92E_HI0Q_DESA			0x3C0
#define		REG_92E_HI1Q_DESA			0x3C8
#define		REG_92E_HI2Q_DESA			0x3D0
#define		REG_92E_HI3Q_DESA			0x3D8
#define		REG_92E_HI4Q_DESA			0x3E0
#define		REG_92E_HI5Q_DESA			0x3E8
#define		REG_92E_HI6Q_DESA			0x3F0
#define		REG_92E_HI7Q_DESA			0x3F8
#endif

#define  	REG_92E_TXPKTBUF_BCNQ_BDNY1	0x457
#define		REG_92E_MACID_NOLINK		0x484
#define		REG_92E_MACID_PAUSE			0x48C
#define		REG_92E_TXRPT_CTRL			0x4EC
#define		REG_92E_TXRPT_TIM			0x4F0
#define		REG_92E_TXRPT_STSSET		0x4F2
#define		REG_92E_TXRPT_STSVLD		0x4F4
#define		REG_92E_TXRPT_STSINF		0x4F8
#define		REG_92E_MBSSID_CTRL			0x526
#define 	REG_92E_PKT_LIFETIME_CTRL	0x528
#define 	REG_92E_ATIMWND1			0x570
#define 	REG_92E_PRE_DL_BCN_ITV		0x58F
#define 	REG_92E_ATIMWND2			0x5A0
#define 	REG_92E_ATIMWND3			0x5A1
#define 	REG_92E_ATIMWND4			0x5A2
#define 	REG_92E_ATIMWND5			0x5A3
#define 	REG_92E_ATIMWND6			0x5A4
#define 	REG_92E_ATIMWND7			0x5A5
#define 	REG_92E_ATIMUGT				0x5A6
#define 	REG_92E_HIQ_NO_LMT_EN		0x5A7
#define 	REG_92E_DTIM_COUNT_ROOT		0x5A8
#define 	REG_92E_DTIM_COUNT_VAP1		0x5A9
#define 	REG_92E_DTIM_COUNT_VAP2		0x5AA
#define 	REG_92E_DTIM_COUNT_VAP3		0x5AB
#define 	REG_92E_DTIM_COUNT_VAP4		0x5AC
#define 	REG_92E_DTIM_COUNT_VAP5		0x5AD
#define 	REG_92E_DTIM_COUNT_VAP6		0x5AE
#define 	REG_92E_DTIM_COUNT_VAP7		0x5AF
#define 	REG_92E_DIS_ATIM			0x5B0
#define 	REG_92E_UPD_HGQMD			0x604

//----------------------------------------------------------------------------
//       8192E REG_92E_HIMR bits				(Offset 0xB0-B3, 32 bits)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//       8192E REG_92E_HISR bits				(Offset 0xB4-B7, 32 bits)
//----------------------------------------------------------------------------
#define	HIMR_92E_TXCCK					BIT(30)		// TXRPT interrupt when CCX bit of the packet is set	
#define	HIMR_92E_PSTIMEOUT				BIT(29)		// Power Save Time Out Interrupt
#define	HIMR_92E_GTINT4					BIT(28)		// When GTIMER4 expires, this bit is set to 1	
#define	HIMR_92E_GTINT3					BIT(27)		// When GTIMER3 expires, this bit is set to 1	
#define	HIMR_92E_TBDER					BIT(26)		// Transmit Beacon0 Error			
#define	HIMR_92E_TBDOK					BIT(25)		// Transmit Beacon0 OK, ad hoc only
#define	HIMR_92E_TSF_BIT32_TOGGLE		BIT(24)		// TSF Timer BIT32 toggle indication interrupt			
#define	HIMR_92E_BcnInt					BIT(20)		// Beacon DMA Interrupt 0			
#define	HIMR_92E_BDERR0					BIT(16)		// Beacon Queue DMA OK0			
#define	HIMR_92E_HSISR_IND_ON_INT		BIT(15)		// HSISR Indicator (HSIMR & HSISR is true, this bit is set to 1)			
#define	HIMR_92E_BCNDMAINT_E			BIT(14)		// Beacon DMA Interrupt Extension for Win7			
#define	HIMR_92E_ATIMEND				BIT(12)		// CTWidnow End or ATIM Window End
#define	HIMR_92E_HISR1_IND_INT			BIT(11)		// HISR1 Indicator (HISR1 & HIMR1 is true, this bit is set to 1)
#define	HIMR_92E_C2HCMD					BIT(10)		// CPU to Host Command INT Status, Write 1 clear	
#define	HIMR_92E_CPWM2					BIT(9)		// CPU power Mode exchange INT Status, Write 1 clear	
#define	HIMR_92E_CPWM					BIT(8)		// CPU power Mode exchange INT Status, Write 1 clear	
#define	HIMR_92E_HIGHDOK				BIT(7)		// High Queue DMA OK	
#define	HIMR_92E_MGNTDOK				BIT(6)		// Management Queue DMA OK	
#define	HIMR_92E_BKDOK					BIT(5)		// AC_BK DMA OK		
#define	HIMR_92E_BEDOK					BIT(4)		// AC_BE DMA OK	
#define	HIMR_92E_VIDOK					BIT(3)		// AC_VI DMA OK		
#define	HIMR_92E_VODOK					BIT(2)		// AC_VO DMA OK	
#define	HIMR_92E_RDU					BIT(1)		// Rx Descriptor Unavailable	
#define	HIMR_92E_ROK					BIT(0)		// Receive DMA OK

//----------------------------------------------------------------------------
//       8192E REG_92E_HIMRE bits			(Offset 0xB8-BB, 32 bits)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//       8192E REG_92E_HIMSE bits			(Offset 0xBC-BF, 32 bits)
//----------------------------------------------------------------------------
#define	HIMRE_92E_BCNDMAINT7			BIT(27)		// Beacon DMA Interrupt 7
#define	HIMRE_92E_BCNDMAINT6			BIT(26)		// Beacon DMA Interrupt 6
#define	HIMRE_92E_BCNDMAINT5			BIT(25)		// Beacon DMA Interrupt 5
#define	HIMRE_92E_BCNDMAINT4			BIT(24)		// Beacon DMA Interrupt 4
#define	HIMRE_92E_BCNDMAINT3			BIT(23)		// Beacon DMA Interrupt 3
#define	HIMRE_92E_BCNDMAINT2			BIT(22)		// Beacon DMA Interrupt 2
#define	HIMRE_92E_BCNDMAINT1			BIT(21)		// Beacon DMA Interrupt 1
#define	HIMRE_92E_BCNDOK7				BIT(20)		// Beacon Queue DMA OK Interrup 7
#define	HIMRE_92E_BCNDOK6				BIT(19)		// Beacon Queue DMA OK Interrup 6
#define	HIMRE_92E_BCNDOK5				BIT(18)		// Beacon Queue DMA OK Interrup 5
#define	HIMRE_92E_BCNDOK4				BIT(17)		// Beacon Queue DMA OK Interrup 4
#define	HIMRE_92E_BCNDOK3				BIT(16)		// Beacon Queue DMA OK Interrup 3
#define	HIMRE_92E_BCNDOK2				BIT(15)		// Beacon Queue DMA OK Interrup 2
#define	HIMRE_92E_BCNDOK1				BIT(14)		// Beacon Queue DMA OK Interrup 1
#define	HIMRE_92E_ATIMEND_E				BIT(13)		// ATIM Window End Extension for Win7
#define	HIMRE_92E_TXERR					BIT(11)		// Tx Error Flag Interrupt Status, write 1 clear.
#define	HIMRE_92E_RXERR					BIT(10)		// Rx Error Flag INT Status, Write 1 clear
#define	HIMRE_92E_TXFOVW				BIT(9)		// Transmit FIFO Overflow
#define	HIMRE_92E_RXFOVW				BIT(8)		// Receive FIFO Overflow

//----------------------------------------------------------------------------
//       8192E REG_92E_HQ_DES_NUM0 bits		(Offset 0x3A0-3A3, 32 bits)
//----------------------------------------------------------------------------
#define	ACQ_92E_H1Q_DESCS_MODE_8SEG		BIT(31)	
#define ACQ_92E_H1Q_DESC_NUM_MASK		0xfff
#define ACQ_92E_H1Q_DESC_NUM_SHIFT		16
#define	ACQ_92E_H0Q_DESCS_MODE_8SEG		BIT(15)	
#define ACQ_92E_H0Q_DESC_NUM_MASK		0xfff
#define ACQ_92E_H0Q_DESC_NUM_SHIFT		0

//----------------------------------------------------------------------------
//       8192E REG_92E_HQ_DES_NUM1 bits		(Offset 0x3A4-3A7, 32 bits)
//----------------------------------------------------------------------------
#define	ACQ_92E_H3Q_DESCS_MODE_8SEG		BIT(31)	
#define ACQ_92E_H3Q_DESC_NUM_MASK		0xfff
#define ACQ_92E_H3Q_DESC_NUM_SHIFT		16
#define	ACQ_92E_H2Q_DESCS_MODE_8SEG		BIT(15)	
#define ACQ_92E_H2Q_DESC_NUM_MASK		0xfff
#define ACQ_92E_H2Q_DESC_NUM_SHIFT		0

//----------------------------------------------------------------------------
//       8192E REG_92E_HQ_DES_NUM2 bits		(Offset 0x3A8-3AB, 32 bits)
//----------------------------------------------------------------------------
#define	ACQ_92E_H5Q_DESCS_MODE_8SEG		BIT(31)	
#define ACQ_92E_H5Q_DESC_NUM_MASK		0xfff
#define ACQ_92E_H5Q_DESC_NUM_SHIFT		16
#define	ACQ_92E_H4Q_DESCS_MODE_8SEG		BIT(15)	
#define ACQ_92E_H4Q_DESC_NUM_MASK		0xfff
#define ACQ_92E_H4Q_DESC_NUM_SHIFT		0

//----------------------------------------------------------------------------
//       8192E REG_92E_HQ_DES_NUM3 bits		(Offset 0x3AC-3AF, 32 bits)
//----------------------------------------------------------------------------
#define	ACQ_92E_H7Q_DESCS_MODE_8SEG		BIT(31)	
#define ACQ_92E_H7Q_DESC_NUM_MASK		0xfff
#define ACQ_92E_H7Q_DESC_NUM_SHIFT		16
#define	ACQ_92E_H6Q_DESCS_MODE_8SEG		BIT(15)	
#define ACQ_92E_H6Q_DESC_NUM_MASK		0xfff
#define ACQ_92E_H6Q_DESC_NUM_SHIFT		0

//----------------------------------------------------------------------------
//       8192E REG_92E_CLRQ bits				(Offset 0x3B0-3B4, 32 bits)
//----------------------------------------------------------------------------
#define CLRQ_92E_ALL_IDX				0x3FFF3FFF
#define	CLRQ_92E_HI7Q_HW_IDX			BIT(29)
#define	CLRQ_92E_HI6Q_HW_IDX			BIT(28)
#define	CLRQ_92E_HI5Q_HW_IDX			BIT(27)
#define	CLRQ_92E_HI4Q_HW_IDX			BIT(26)
#define	CLRQ_92E_HI3Q_HW_IDX			BIT(25)
#define	CLRQ_92E_HI2Q_HW_IDX			BIT(24)
#define	CLRQ_92E_HI1Q_HW_IDX			BIT(23)
#define	CLRQ_92E_HI0Q_HW_IDX			BIT(22)
#define	CLRQ_92E_BKQ_HW_IDX				BIT(21)
#define	CLRQ_92E_BEQ_HW_IDX				BIT(20)
#define	CLRQ_92E_VIQ_HW_IDX				BIT(19)
#define	CLRQ_92E_VOQ_HW_IDX				BIT(18)
#define	CLRQ_92E_MGQ_HW_IDX				BIT(17)
#define	CLRQ_92E_RXQ_HW_IDX				BIT(16)
#define	CLRQ_92E_HI7Q_HOST_IDX			BIT(13)
#define	CLRQ_92E_HI6Q_HOST_IDX			BIT(12)
#define	CLRQ_92E_HI5Q_HOST_IDX			BIT(11)
#define	CLRQ_92E_HI4Q_HOST_IDX			BIT(10)
#define	CLRQ_92E_HI3Q_HOST_IDX			BIT(9)
#define	CLRQ_92E_HI2Q_HOST_IDX			BIT(8)
#define	CLRQ_92E_HI1Q_HOST_IDX			BIT(7)
#define	CLRQ_92E_HI0Q_HOST_IDX			BIT(6)
#define	CLRQ_92E_BKQ_HOST_IDX			BIT(5)
#define	CLRQ_92E_BEQ_HOST_IDX			BIT(4)
#define	CLRQ_92E_VIQ_HOST_IDX			BIT(3)
#define	CLRQ_92E_VOQ_HOST_IDX			BIT(2)
#define	CLRQ_92E_MGQ_HOST_IDX			BIT(1)
#define	CLRQ_92E_RXQ_HOST_IDX			BIT(0)

//----------------------------------------------------------------------------
//       8192E REG_92E_ACQ_DES_NUM0 bits		(Offset 0x3B4-3B7, 32 bits)
//----------------------------------------------------------------------------
#define	ACQ_92E_VIQ_DESCS_MODE_8SEG		BIT(31)	
#define ACQ_92E_VIQ_DESC_NUM_MASK		0xfff
#define ACQ_92E_VIQ_DESC_NUM_SHIFT		16
#define	ACQ_92E_VOQ_DESCS_MODE_8SEG		BIT(15)	
#define ACQ_92E_VOQ_DESC_NUM_MASK		0xfff
#define ACQ_92E_VOQ_DESC_NUM_SHIFT		0

//----------------------------------------------------------------------------
//       8192E REG_92E_ACQ_DES_NUM1 bits		(Offset 0x3B8-3BB, 32 bits)
//----------------------------------------------------------------------------
#define	ACQ_92E_BKQ_DESCS_MODE_8SEG		BIT(31)	
#define ACQ_92E_BKQ_DESC_NUM_MASK		0xfff
#define ACQ_92E_BKQ_DESC_NUM_SHIFT		16
#define	ACQ_92E_BEQ_DESCS_MODE_8SEG		BIT(15)	
#define ACQ_92E_BEQ_DESC_NUM_MASK		0xfff
#define ACQ_92E_BEQ_DESC_NUM_SHIFT		0

//----------------------------------------------------------------------------
//       8192E REG_92E_ACQ_DES_NUM2 bits		(Offset 0x3BC-3BF, 32 bits)
//----------------------------------------------------------------------------
#define ACQ_92E_RXQ_DESC_NUM_MASK		0xfff
#define ACQ_92E_RXQ_DESC_NUM_SHIFT		16
#define	ACQ_92E_MGQ_DESCS_MODE_8SEG		BIT(15)	
#define ACQ_92E_MGQ_DESC_NUM_MASK		0xfff
#define ACQ_92E_MGQ_DESC_NUM_SHIFT		0

//----------------------------------------------------------------------------
//       8192E MBID_NUM bits					(Offset 0x552, 8 bits)
//----------------------------------------------------------------------------
#define	MBID_NUM_92E_EN_PREDOWN_BCN		BIT(3)	

#endif

