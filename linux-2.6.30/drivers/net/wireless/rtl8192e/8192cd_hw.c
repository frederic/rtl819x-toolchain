/*
 *  Routines to access hardware
 *
 *  $Id: 8192cd_hw.c,v 1.107.2.43 2011/01/17 13:21:01 victoryman Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_HW_C_

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <asm/unistd.h>
#elif defined(__ECOS)
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
#include <cyg/io/eth/rltk/819x/wrapper/wrapper.h>
#endif

#include "./8192cd_cfg.h"
#include "./8192cd.h"
#include "./8192cd_hw.h"
#include "./8192cd_headers.h"
#include "./8192cd_debug.h"
#ifdef CONFIG_RTL_88E_SUPPORT
#include "Hal8188EPwrSeq.h"
#endif

#ifdef CONFIG_RTL_8812_SUPPORT
#include "Hal8812PwrSeq.h"
#endif


#ifdef __KERNEL__
#ifdef __LINUX_2_6__
#include <linux/syscalls.h>
#else
#include <linux/fs.h>
#endif
#endif

#ifdef USE_RTL8186_SDK
#ifdef CONFIG_RTL8672
	#ifdef USE_RLX_BSP
	#include <bspchip.h>

	#ifdef CONFIG_RTL_8196C
	#undef CONFIG_RTL_8196C
	#endif
	#ifdef CONFIG_RTL8196C_REVISION_B
	#undef CONFIG_RTL8196C_REVISION_B
	#endif

	#else
	#include <platform.h>
	#endif
#else

#if defined(__LINUX_2_6__)
#include <bsp/bspchip.h>
#else
#ifndef __ECOS
	#include <asm/rtl865x/platform.h>
#endif
#endif
#endif

#if defined(CONFIG_RTL865X_WTDOG) || defined(CONFIG_RTL_WTDOG)
	#if defined(__LINUX_2_6__)
#define _WDTCNR_			BSP_WDTCNR
#else
#define _WDTCNR_			WDTCNR
#endif
#endif

#endif // USE_RTL8186_SDK

#if defined(CONFIG_RTL865X_WTDOG) || defined(CONFIG_RTL_WTDOG)
#ifdef CONFIG_NET_PCI
#ifndef CONFIG_RTL_8198B
        #define BSP_WDTCNR 0xB800311C
#endif
#endif
#endif

#define MAX_CONFIG_FILE_SIZE (20*1024) // for 8192, added to 20k

int rtl8192cd_fileopen(const char *filename, int flags, int mode);
void selectMinPowerIdex(struct rtl8192cd_priv *priv);
void PHY_RF6052SetOFDMTxPower(struct rtl8192cd_priv *priv, unsigned int channel);
void PHY_RF6052SetCCKTxPower(struct rtl8192cd_priv *priv, unsigned int channel);
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
static void rtl8192cd_ReadFwHdr(struct rtl8192cd_priv * priv);
#endif
#ifdef CONFIG_RTL_92C_SUPPORT
static int Load_92C_Firmware(struct rtl8192cd_priv *priv);
#endif

/*
#ifdef ADD_TX_POWER_BY_CMD
#define ASSIGN_TX_POWER_OFFSET(offset, setting) { \
	if (setting != 0x7f) \
		offset = setting; \
}
#endif
*/

#define VAR_MAPPING(dst,src) \
unsigned char *data_##dst##_start = &data_##src[0]; \
unsigned char *data_##dst##_end   = &data_##src[sizeof(data_##src)]; \

#ifdef CONFIG_RTL_92D_SUPPORT
#include "data_PHY_REG_n.c"
#include "data_AGC_TAB_n.c"
#include "data_AGC_TAB_2G_n.c"
#include "data_AGC_TAB_5G_n.c"
#include "data_radio_a_n.c"
#include "data_radio_b_n.c"
#ifdef RTL8192D_INT_PA
#ifdef USB_POWER_SUPPORT

#if defined (RTL8192D_INT_PA_GAIN_TABLE_NEW)
#include "data_radio_a_intPA_GM_new.c"
#include "data_radio_b_intPA_GM_new.c"
#elif defined (RTL8192D_INT_PA_GAIN_TABLE_NEW1)
#include "data_radio_a_intPA_GM_new1.c"
#include "data_radio_b_intPA_GM_new1.c"
#else
#include "data_radio_a_intPA_GM.c"
#include "data_radio_b_intPA_GM.c"
#endif

#else // USB_POWER_SUPPORT

#if defined (RTL8192D_INT_PA_GAIN_TABLE_NEW)
#include "data_radio_a_intPA_new.c"
#include "data_radio_b_intPA_new.c"
#else
#include "data_radio_a_intPA.c"
#include "data_radio_b_intPA.c"
#endif

#endif // USB_POWER_SUPPORT
#endif // RTL8192D_INT_PA
//_TXPWR_REDEFINE
#ifdef HIGH_POWER_EXT_PA
#include "data_AGC_TAB_n_92d_hp.c"
#include "data_PHY_REG_n_92d_hp.c"
#include "data_radio_a_n_92d_hp.c"
#include "data_radio_b_n_92d_hp.c"
#include "data_PHY_REG_PG_92d_hp.c"
#endif
#include "data_PHY_REG_PG.c"
#include "data_PHY_REG_PG_FCC.c"
#include "data_PHY_REG_PG_CE.c"
#ifdef TXPWR_LMT
#include "data_TXPWR_LMT.c"
#include "data_TXPWR_LMT_FCC.c"
#include "data_TXPWR_LMT_CE.c"
#endif
#ifdef _TRACKING_TABLE_FILE
#include "data_REG_TXPWR_TRK_n_92d.c"
#include "data_REG_TXPWR_TRK_n_92d_hp.c"
#endif
#include "data_PHY_REG_MP_n.c"
#include "data_MACPHY_REG.c"
#include "data_rtl8192dfw_n.c"
VAR_MAPPING(PHY_REG_n, PHY_REG_n);
VAR_MAPPING(AGC_TAB_n, AGC_TAB_n);
VAR_MAPPING(AGC_TAB_2G_n, AGC_TAB_2G_n);
VAR_MAPPING(AGC_TAB_5G_n, AGC_TAB_5G_n);
VAR_MAPPING(radio_a_n, radio_a_n);
VAR_MAPPING(radio_b_n, radio_b_n);
#ifdef RTL8192D_INT_PA
#ifdef USB_POWER_SUPPORT

#if defined (RTL8192D_INT_PA_GAIN_TABLE_NEW)
VAR_MAPPING(radio_a_intPA_GM_new, radio_a_intPA_GM_new);
VAR_MAPPING(radio_b_intPA_GM_new, radio_b_intPA_GM_new);
#elif defined (RTL8192D_INT_PA_GAIN_TABLE_NEW1)
VAR_MAPPING(radio_a_intPA_GM_new1, radio_a_intPA_GM_new1);
VAR_MAPPING(radio_b_intPA_GM_new1, radio_b_intPA_GM_new1);
#else
VAR_MAPPING(radio_a_intPA_GM, radio_a_intPA_GM);
VAR_MAPPING(radio_b_intPA_GM, radio_b_intPA_GM);
#endif

#else // USB_POWER_SUPPORT

#if defined (RTL8192D_INT_PA_GAIN_TABLE_NEW)
VAR_MAPPING(radio_a_intPA_new, radio_a_intPA_new);
VAR_MAPPING(radio_b_intPA_new, radio_b_intPA_new);
#else
VAR_MAPPING(radio_a_intPA, radio_a_intPA);
VAR_MAPPING(radio_b_intPA, radio_b_intPA);
#endif

#endif // USB_POWER_SUPPORT
#endif // RTL8192D_INT_PA
//_TXPWR_REDEFINE
#ifdef HIGH_POWER_EXT_PA
VAR_MAPPING(AGC_TAB_n_92d_hp, AGC_TAB_n_92d_hp);
VAR_MAPPING(PHY_REG_n_92d_hp, PHY_REG_n_92d_hp);
VAR_MAPPING(radio_a_n_92d_hp, radio_a_n_92d_hp);
VAR_MAPPING(radio_b_n_92d_hp, radio_b_n_92d_hp);
VAR_MAPPING(PHY_REG_PG_92d_hp, PHY_REG_PG_92d_hp);
#endif
VAR_MAPPING(PHY_REG_PG, PHY_REG_PG);
VAR_MAPPING(PHY_REG_PG_FCC, PHY_REG_PG_FCC);
VAR_MAPPING(PHY_REG_PG_CE, PHY_REG_PG_CE);
#ifdef TXPWR_LMT
VAR_MAPPING(TXPWR_LMT, TXPWR_LMT);
VAR_MAPPING(TXPWR_LMT_FCC, TXPWR_LMT_FCC);
VAR_MAPPING(TXPWR_LMT_CE, TXPWR_LMT_CE);
#endif
#ifdef _TRACKING_TABLE_FILE
VAR_MAPPING(REG_TXPWR_TRK_n_92d, REG_TXPWR_TRK_n_92d);
VAR_MAPPING(REG_TXPWR_TRK_n_92d_hp, REG_TXPWR_TRK_n_92d_hp);
#endif
VAR_MAPPING(PHY_REG_MP_n, PHY_REG_MP_n);
VAR_MAPPING(MACPHY_REG, MACPHY_REG);
VAR_MAPPING(rtl8192dfw_n, rtl8192dfw_n);
#endif // CONFIG_RTL_92D_SUPPORT

#ifdef CONFIG_RTL_92C_SUPPORT

#ifdef TESTCHIP_SUPPORT
#include "data_AGC_TAB.c"
#include "data_PHY_REG_1T.c"
#include "data_PHY_REG_2T.c"
#include "data_radio_a_1T.c"
#include "data_radio_a_2T.c"
#include "data_radio_b_2T.c"
#include "data_rtl8192cfw.c"
#endif

#include "data_AGC_TAB_n_92C.c"
#include "data_PHY_REG_1T_n.c"
#include "data_PHY_REG_2T_n.c"
#include "data_radio_a_2T_n.c"
#include "data_radio_b_2T_n.c"
#include "data_radio_a_1T_n.c"
#include "data_rtl8192cfwn.c"
#include "data_rtl8192cfwua.c"


#include "data_PHY_REG_PG_92C.c"
#include "data_MACPHY_REG_92C.c"
#include "data_PHY_REG_MP_n_92C.c"

#include "data_AGC_TAB_n_hp.c"
#include "data_PHY_REG_2T_n_hp.c"
#include "data_radio_a_2T_n_lna.c"
#include "data_radio_b_2T_n_lna.c"
#include "data_PHY_REG_1T_n_hp.c"

#ifdef HIGH_POWER_EXT_PA
#include "data_radio_a_2T_n_hp.c"
#include "data_radio_b_2T_n_hp.c"
#include "data_PHY_REG_PG_hp.c"
#endif

#ifdef _TRACKING_TABLE_FILE
#include "data_REG_TXPWR_TRK.c"
#include "data_REG_TXPWR_TRK_hp.c"
#endif

#ifdef TXPWR_LMT 
#include "data_TXPWR_LMT_92c.c"
#include "data_TXPWR_LMT_92c_FCC.c"
#include "data_TXPWR_LMT_92c_CE.c"
#endif



#define VAR_MAPPING(dst,src) \
	unsigned char *data_##dst##_start = &data_##src[0]; \
	unsigned char *data_##dst##_end   = &data_##src[sizeof(data_##src)]; \

#ifdef TESTCHIP_SUPPORT
VAR_MAPPING(AGC_TAB, AGC_TAB);
VAR_MAPPING(PHY_REG_1T, PHY_REG_1T);
VAR_MAPPING(PHY_REG_2T, PHY_REG_2T);
VAR_MAPPING(radio_a_1T, radio_a_1T);
VAR_MAPPING(radio_a_2T, radio_a_2T);
VAR_MAPPING(radio_b_2T, radio_b_2T);
VAR_MAPPING(rtl8192cfw, rtl8192cfw);
#endif

VAR_MAPPING(AGC_TAB_n_92C, AGC_TAB_n_92C);
VAR_MAPPING(PHY_REG_1T_n, PHY_REG_1T_n);
VAR_MAPPING(PHY_REG_2T_n, PHY_REG_2T_n);
VAR_MAPPING(radio_a_1T_n, radio_a_1T_n);
VAR_MAPPING(radio_a_2T_n, radio_a_2T_n);
VAR_MAPPING(radio_b_2T_n, radio_b_2T_n);
VAR_MAPPING(rtl8192cfw_n, rtl8192cfwn);
VAR_MAPPING(rtl8192cfw_ua, rtl8192cfwua);

VAR_MAPPING(MACPHY_REG_92C, MACPHY_REG_92C);
VAR_MAPPING(PHY_REG_PG_92C, PHY_REG_PG_92C);
VAR_MAPPING(PHY_REG_MP_n_92C, PHY_REG_MP_n_92C);

VAR_MAPPING(AGC_TAB_n_hp, AGC_TAB_n_hp);
VAR_MAPPING(PHY_REG_2T_n_hp, PHY_REG_2T_n_hp);
VAR_MAPPING(PHY_REG_1T_n_hp, PHY_REG_1T_n_hp);
VAR_MAPPING(radio_a_2T_n_lna, radio_a_2T_n_lna);
VAR_MAPPING(radio_b_2T_n_lna, radio_b_2T_n_lna);

#ifdef HIGH_POWER_EXT_PA
VAR_MAPPING(radio_a_2T_n_hp, radio_a_2T_n_hp);
VAR_MAPPING(radio_b_2T_n_hp, radio_b_2T_n_hp);
VAR_MAPPING(PHY_REG_PG_hp, PHY_REG_PG_hp);
#endif

#ifdef _TRACKING_TABLE_FILE
VAR_MAPPING(REG_TXPWR_TRK, REG_TXPWR_TRK);
VAR_MAPPING(REG_TXPWR_TRK_hp, REG_TXPWR_TRK_hp);
#endif

#ifdef TXPWR_LMT
VAR_MAPPING(TXPWR_LMT_92c, TXPWR_LMT_92c);
VAR_MAPPING(TXPWR_LMT_92c_FCC, TXPWR_LMT_92c_FCC);
VAR_MAPPING(TXPWR_LMT_92c_CE, TXPWR_LMT_92c_CE);
#endif


#endif // CONFIG_RTL_92C_SUPPORT

#ifdef CONFIG_RTL_88E_SUPPORT
#include "data_PHY_REG_PG_88E.c"
#include "data_PHY_REG_MP_88E.c"
VAR_MAPPING(PHY_REG_PG_88E, PHY_REG_PG_88E);
VAR_MAPPING(PHY_REG_MP_88E, PHY_REG_MP_88E);

#ifndef CONFIG_MACBBRF_BY_ODM
#include "data_AGC_TAB_1T_88E.c"
#include "data_MAC_REG_88E.c"
#include "data_PHY_REG_1T_88E.c"
#include "data_radio_a_1T_88E.c"
#ifdef SUPPORT_RTL8188E_TC
#include "data_MAC_REG_88E_TC.c"
#include "data_PHY_REG_1T_88E_TC.c"
#include "data_radio_a_1T_88E_TC.c"
#endif

VAR_MAPPING(AGC_TAB_1T_88E, AGC_TAB_1T_88E);
VAR_MAPPING(MAC_REG_88E, MAC_REG_88E);
VAR_MAPPING(PHY_REG_1T_88E, PHY_REG_1T_88E);
VAR_MAPPING(radio_a_1T_88E, radio_a_1T_88E);
#ifdef SUPPORT_RTL8188E_TC
VAR_MAPPING(MAC_REG_88E_TC, MAC_REG_88E_TC);
VAR_MAPPING(PHY_REG_1T_88E_TC, PHY_REG_1T_88E_TC);
VAR_MAPPING(radio_a_1T_88E_TC, radio_a_1T_88E_TC);
#endif
#endif
#endif // CONFIG_RTL_88E_SUPPORT

#ifdef CONFIG_RTL_8812_SUPPORT
#include "data_AGC_TAB_8812.c"
#include "data_MAC_REG_8812.c"
#include "data_PHY_REG_8812.c"
#include "data_PHY_REG_MP_8812.c"
#include "data_PHY_REG_PG_8812.c"
#include "data_RadioA_8812.c"
#include "data_RadioB_8812.c"
#include "data_rtl8812fw.c"
VAR_MAPPING(rtl8812fw, rtl8812fw);
VAR_MAPPING(AGC_TAB_8812, AGC_TAB_8812);
VAR_MAPPING(MAC_REG_8812, MAC_REG_8812);
VAR_MAPPING(PHY_REG_8812, PHY_REG_8812);
VAR_MAPPING(PHY_REG_MP_8812, PHY_REG_MP_8812);
VAR_MAPPING(PHY_REG_PG_8812, PHY_REG_PG_8812);
VAR_MAPPING(RadioA_8812, RadioA_8812);
VAR_MAPPING(RadioB_8812, RadioB_8812);

//FOR_8812_MP_CHIP
#include "data_MAC_REG_8812_n.c"
#include "data_AGC_TAB_8812_n_default.c"
#include "data_PHY_REG_8812_n_default.c"
#include "data_RadioA_8812_n_default.c"
#include "data_RadioB_8812_n_default.c"
#include "data_rtl8812fw_n.c"
VAR_MAPPING(MAC_REG_8812_n, MAC_REG_8812_n);
VAR_MAPPING(rtl8812fw_n, rtl8812fw_n);
VAR_MAPPING(AGC_TAB_8812_n_default, AGC_TAB_8812_n_default);
VAR_MAPPING(PHY_REG_8812_n_default, PHY_REG_8812_n_default);
VAR_MAPPING(RadioA_8812_n_default, RadioA_8812_n_default);
VAR_MAPPING(RadioB_8812_n_default, RadioB_8812_n_default);
#include "data_AGC_TAB_8812_n_extlna.c"
#include "data_PHY_REG_8812_n_extlna.c"
#include "data_RadioA_8812_n_extlna.c"
#include "data_RadioB_8812_n_extlna.c"
VAR_MAPPING(AGC_TAB_8812_n_extlna, AGC_TAB_8812_n_extlna);
VAR_MAPPING(PHY_REG_8812_n_extlna, PHY_REG_8812_n_extlna);
VAR_MAPPING(RadioA_8812_n_extlna, RadioA_8812_n_extlna);
VAR_MAPPING(RadioB_8812_n_extlna, RadioB_8812_n_extlna);
#include "data_AGC_TAB_8812_n_extpa.c"
#include "data_PHY_REG_8812_n_extpa.c"
#include "data_RadioA_8812_n_extpa.c"
#include "data_RadioB_8812_n_extpa.c"
VAR_MAPPING(AGC_TAB_8812_n_extpa, AGC_TAB_8812_n_extpa);
VAR_MAPPING(PHY_REG_8812_n_extpa, PHY_REG_8812_n_extpa);
VAR_MAPPING(RadioA_8812_n_extpa, RadioA_8812_n_extpa);
VAR_MAPPING(RadioB_8812_n_extpa, RadioB_8812_n_extpa);

#ifdef HIGH_POWER_EXT_PA //FOR_8812_HP
#include "data_AGC_TAB_8812_hp.c"
#include "data_RadioA_8812_hp.c"
#include "data_RadioB_8812_hp.c"
VAR_MAPPING(AGC_TAB_8812_hp, AGC_TAB_8812_hp);
VAR_MAPPING(RadioA_8812_hp, RadioA_8812_hp);
VAR_MAPPING(RadioB_8812_hp, RadioB_8812_hp);

//FOR_8812_MP_CHIP
#include "data_AGC_TAB_8812_n_hp.c"
#include "data_PHY_REG_8812_n_hp.c"
#include "data_RadioA_8812_n_hp.c"
#include "data_RadioB_8812_n_hp.c"
VAR_MAPPING(AGC_TAB_8812_n_hp, AGC_TAB_8812_n_hp);
VAR_MAPPING(PHY_REG_8812_n_hp, PHY_REG_8812_n_hp);
VAR_MAPPING(RadioA_8812_n_hp, RadioA_8812_n_hp);
VAR_MAPPING(RadioB_8812_n_hp, RadioB_8812_n_hp);

#endif
#endif 

#ifdef __ECOS
static unsigned char config_buf[MAX_CONFIG_FILE_SIZE];
#endif

/*-----------------------------------------------------------------------------
 * Function:	PHYCheckIsLegalRfPath8192cPci()
 *
 * Overview:	Check different RF type to execute legal judgement. If RF Path is illegal
 *			We will return false.
 *
 * Input:		NONE
 *
 * Output:		NONE
 *
 * Return:		NONE
 *
 * Revised History:
 *	When		Who		Remark
 *	11/15/2007	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
int PHYCheckIsLegalRfPath8192cPci(struct rtl8192cd_priv *priv, unsigned int eRFPath)
{
	unsigned int rtValue = TRUE;

	if (get_rf_mimo_mode(priv) == MIMO_2T2R) {
		if ((eRFPath == RF92CD_PATH_A) || (eRFPath == RF92CD_PATH_B))
			rtValue = TRUE;
		else
			rtValue = FALSE;
	}
	else if (get_rf_mimo_mode(priv) == MIMO_1T1R) {
		if (eRFPath == RF92CD_PATH_A)
			rtValue = TRUE;
		else
			rtValue = FALSE;
	}
	else {
		rtValue = FALSE;
	}

	return rtValue;
}
#if defined(CONFIG_RTL_8196CS)
void setBaseAddressRegister(void)
{
	int tmp32=0, status;
	while(++tmp32 < 100) {
		REG32(0xb8b00004) = 0x00100007;
		REG32(0xb8b10004) = 0x00100007;
		REG32(0xb8b10010) = 0x18c00001;
		REG32(0xb8b10018) = 0x19000004;
		status = (REG32(0xb8b10010) ^ 0x18c00001) |( REG32(0xb8b10018) ^ 0x19000004);
		if(!status)
			break;
		else {
			printk("set BAR fail,%x\n", status);
			printk("%x %x %x %x \n",
			REG32(0xb8b00004) , REG32(0xb8b10004) ,REG32(0xb8b10010),  REG32(0xb8b10018) );
		}
	} ;
}
#endif
/**
* Function:	phy_CalculateBitShift
*
* OverView:	Get shifted position of the BitMask
*
* Input:
*			u4Byte		BitMask,
*
* Output:	none
* Return:		u4Byte		Return the shift bit bit position of the mask
*/
unsigned int phy_CalculateBitShift(unsigned int BitMask)
{
	unsigned int i;

	for (i=0; i<=31; i++) {
		if (((BitMask>>i) & 0x1) == 1)
			break;
	}

	return (i);
}

/**
* Function:	PHY_QueryBBReg
*
* OverView:	Read "sepcific bits" from BB register
*
* Input:
*			PADAPTER		Adapter,
*			u4Byte			RegAddr,		//The target address to be readback
*			u4Byte			BitMask		//The target bit position in the target address
*										//to be readback
* Output:	None
* Return:		u4Byte			Data			//The readback register value
* Note:		This function is equal to "GetRegSetting" in PHY programming guide
*/
unsigned int PHY_QueryBBReg(struct rtl8192cd_priv *priv, unsigned int RegAddr, unsigned int BitMask)
{
  	unsigned int ReturnValue = 0, OriginalValue, BitShift;
#ifdef CONFIG_RTL_92D_DMDP
	unsigned char reg;
	reg = MAC_PHY_CTRL_MP;
	if (!(RTL_R8(reg) & BIT(1)) && priv->pshare->wlandev_idx == 1){
		return DMDP_PHY_QueryBBReg(0, RegAddr, BitMask);
	}
#endif

	OriginalValue = RTL_R32(RegAddr);
	BitShift = phy_CalculateBitShift(BitMask);
	ReturnValue = (OriginalValue & BitMask) >> BitShift;

	return (ReturnValue);
}


/**
* Function:	PHY_SetBBReg
*
* OverView:	Write "Specific bits" to BB register (page 8~)
*
* Input:
*			PADAPTER		Adapter,
*			u4Byte			RegAddr,		//The target address to be modified
*			u4Byte			BitMask		//The target bit position in the target address
*										//to be modified
*			u4Byte			Data			//The new register value in the target bit position
*										//of the target address
*
* Output:	None
* Return:		None
* Note:		This function is equal to "PutRegSetting" in PHY programming guide
*/
void PHY_SetBBReg(struct rtl8192cd_priv *priv, unsigned int RegAddr, unsigned int BitMask, unsigned int Data)
{
	unsigned int OriginalValue, BitShift, NewValue;

#ifdef FPGA_VERIFICATION
    return;
#endif
    
#ifdef CONFIG_RTL_92D_DMDP
	unsigned char reg;
	reg = MAC_PHY_CTRL_MP;

	if (!(RTL_R8(reg) & BIT(1)) && priv->pshare->wlandev_idx == 1){
		DMDP_PHY_SetBBReg(0, RegAddr, BitMask, Data);
		return;
	}
#endif

	if (BitMask != bMaskDWord)
	{//if not "double word" write
	 //_TXPWR_REDEFINE ?? if have original value, how to count tx power index from PG file ??
		OriginalValue = RTL_R32(RegAddr);
		BitShift = phy_CalculateBitShift(BitMask);
		NewValue = ((OriginalValue & (~BitMask)) | (Data << BitShift));
		RTL_W32(RegAddr, NewValue);
	}
	else
		RTL_W32(RegAddr, Data);

	return;
}



/**
* Function:	phy_RFSerialWrite
*
* OverView:	Write data to RF register (page 8~)
*
* Input:
*			PADAPTER		Adapter,
*			RF92CD_RADIO_PATH_E	eRFPath,	//Radio path of A/B/C/D
*			u4Byte			Offset,		//The target address to be read
*			u4Byte			Data			//The new register Data in the target bit position
*										//of the target to be read
*
* Output:	None
* Return:		None
* Note:		Threre are three types of serial operations: (1) Software serial write
*			(2) Hardware LSSI-Low Speed Serial Interface (3) Hardware HSSI-High speed
*			serial write. Driver need to implement (1) and (2).
*			This function is equal to the combination of RF_ReadReg() and  RFLSSIRead()
 *
 * Note: 		  For RF8256 only
 *			 The total count of RTL8256(Zebra4) register is around 36 bit it only employs
 *			 4-bit RF address. RTL8256 uses "register mode control bit" (Reg00[12], Reg00[10])
 *			 to access register address bigger than 0xf. See "Appendix-4 in PHY Configuration
 *			 programming guide" for more details.
 *			 Thus, we define a sub-finction for RTL8526 register address conversion
 *		       ===========================================================
 *			 Register Mode		RegCTL[1]		RegCTL[0]		Note
 *								(Reg00[12])		(Reg00[10])
 *		       ===========================================================
 *			 Reg_Mode0				0				x			Reg 0 ~15(0x0 ~ 0xf)
 *		       ------------------------------------------------------------------
 *			 Reg_Mode1				1				0			Reg 16 ~30(0x1 ~ 0xf)
 *		       ------------------------------------------------------------------
 *			 Reg_Mode2				1				1			Reg 31 ~ 45(0x1 ~ 0xf)
 *		       ------------------------------------------------------------------
*/
void phy_RFSerialWrite(struct rtl8192cd_priv *priv, RF92CD_RADIO_PATH_E eRFPath, unsigned int Offset, unsigned int Data)
{
	struct rtl8192cd_hw			*phw = GET_HW(priv);
	unsigned int				DataAndAddr = 0;
	BB_REGISTER_DEFINITION_T	*pPhyReg = &phw->PHYRegDef[eRFPath];
	unsigned int				NewOffset;

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_WLAN_HAL)
	if ((GET_CHIP_VER(priv) == VERSION_8188E) || (IS_HAL_CHIP(priv)))
		Offset &= 0xff;
	else
#endif
		Offset &= 0x7f;
	//
	// Switch page for 8256 RF IC
	//
		NewOffset = Offset;

	//
	// Put write addr in [5:0]  and write data in [31:16]
	//
	//DataAndAddr = (Data<<16) | (NewOffset&0x3f);
	DataAndAddr = ((NewOffset<<20) | (Data&0x000fffff)) & 0x0fffffff;	// T65 RF

	//
	// Write Operation
	//
	PHY_SetBBReg(priv, pPhyReg->rf3wireOffset, bMaskDWord, DataAndAddr);
}


/**
* Function:	phy_RFSerialRead
*
* OverView:	Read regster from RF chips
*
* Input:
*			PADAPTER		Adapter,
*			RF92CD_RADIO_PATH_E	eRFPath,	//Radio path of A/B/C/D
*			u4Byte			Offset,		//The target address to be read
*			u4Byte			dbg_avoid,	//set bitmask in reg 0 to prevent RF switchs to debug mode
*
* Output:	None
* Return:		u4Byte			reback value
* Note:		Threre are three types of serial operations: (1) Software serial write
*			(2) Hardware LSSI-Low Speed Serial Interface (3) Hardware HSSI-High speed
*			serial write. Driver need to implement (1) and (2).
*			This function is equal to the combination of RF_ReadReg() and  RFLSSIRead()
*/
unsigned int phy_RFSerialRead(struct rtl8192cd_priv *priv, RF92CD_RADIO_PATH_E eRFPath, unsigned int Offset, unsigned int dbg_avoid)
{
	struct rtl8192cd_hw			*phw = GET_HW(priv);
	unsigned int 				tmplong, tmplong2;
	unsigned int				retValue = 0;
	BB_REGISTER_DEFINITION_T	*pPhyReg = &phw->PHYRegDef[eRFPath];
	unsigned int				NewOffset;

	//
	// Make sure RF register offset is correct
	//
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_WLAN_HAL)
	if ((GET_CHIP_VER(priv) == VERSION_8188E) || (IS_HAL_CHIP(priv)))
		Offset &= 0xff;
	else
#endif
		Offset &= 0x7f;

	//
	// Switch page for 8256 RF IC
	//
	NewOffset = Offset;


#ifdef CONFIG_WLAN_HAL_8192EE
	if (GET_CHIP_VER(priv) == VERSION_8192E) {
		int RfPiEnable = 0;
		
		if(eRFPath == RF_PATH_A)
		{
			tmplong2 = PHY_QueryBBReg(priv, rFPGA0_XA_HSSIParameter2, bMaskDWord);;
			tmplong2 = (tmplong2 & (~bLSSIReadAddress)) | (NewOffset<<23) | bLSSIReadEdge;	//T65 RF
			PHY_SetBBReg(priv, rFPGA0_XA_HSSIParameter2, bMaskDWord, tmplong2&(~bLSSIReadEdge));	
		}
		else
		{
			tmplong2 = PHY_QueryBBReg(priv, rFPGA0_XB_HSSIParameter2, bMaskDWord);
			tmplong2 = (tmplong2 & (~bLSSIReadAddress)) | (NewOffset<<23) | bLSSIReadEdge;	//T65 RF
			PHY_SetBBReg(priv, rFPGA0_XB_HSSIParameter2, bMaskDWord, tmplong2&(~bLSSIReadEdge));	
		}
		
		tmplong2 = PHY_QueryBBReg(priv, rFPGA0_XA_HSSIParameter2, bMaskDWord);
		PHY_SetBBReg(priv, rFPGA0_XA_HSSIParameter2, bMaskDWord, tmplong2 & (~bLSSIReadEdge)); 		
		PHY_SetBBReg(priv, rFPGA0_XA_HSSIParameter2, bMaskDWord, tmplong2 | bLSSIReadEdge);		
		
		delay_us(20);
	

	}else 
#endif

{

	// For 92S LSSI Read RFLSSIRead
	// For RF A/B write 0x824/82c(does not work in the future)
	// We must use 0x824 for RF A and B to execute read trigger
	tmplong = RTL_R32(rFPGA0_XA_HSSIParameter2);
	tmplong2 = RTL_R32(pPhyReg->rfHSSIPara2);
	tmplong2 = (tmplong2 & (~bLSSIReadAddress)) | ((NewOffset<<23) | bLSSIReadEdge);	//T65 RF

	RTL_W32(rFPGA0_XA_HSSIParameter2,tmplong&(~bLSSIReadEdge));
	delay_us(20);
	RTL_W32(pPhyReg->rfHSSIPara2,tmplong2);
	delay_us(20);
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
	if (
#ifdef CONFIG_RTL_92C_SUPPORT
		(GET_CHIP_VER(priv)==VERSION_8192C) || (GET_CHIP_VER(priv)==VERSION_8188C) 
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
#ifdef CONFIG_RTL_92C_SUPPORT
		|| 
#endif
		(GET_CHIP_VER(priv)==VERSION_8192D)
#endif
		) {
		RTL_W32(rFPGA0_XA_HSSIParameter2,tmplong|bLSSIReadEdge);
		delay_us(20);
		RTL_W32(rFPGA0_XA_HSSIParameter2,tmplong&(~bLSSIReadEdge));
	}
#endif

}

	//Read from BBreg8a0, 12 bits for 8190, 20 bits for T65 RF
	if (((eRFPath == RF92CD_PATH_A) && (RTL_R32(0x820)&BIT(8)))
		|| ((eRFPath == RF92CD_PATH_B) && (RTL_R32(0x828)&BIT(8))))
		retValue = PHY_QueryBBReg(priv, pPhyReg->rfLSSIReadBackPi, bLSSIReadBackData);
	else
		retValue = PHY_QueryBBReg(priv, pPhyReg->rfLSSIReadBack, bLSSIReadBackData);

	return retValue;
}


#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A)

unsigned int PHY_QueryRFReg_8812(struct rtl8192cd_priv *priv, RF92CD_RADIO_PATH_E eRFPath,
				unsigned int RegAddr, unsigned int BitMask, unsigned int dbg_avoid)
{
	unsigned int Original_Value, Readback_Value, BitShift;
	unsigned int dwTmp;	

	PHY_SetBBReg(priv, 0x838, 0xf, 0xc);		//		CCA off
				
	RTL_W8(0x8b0, RegAddr);
	
	if (IS_TEST_CHIP(priv)){	//for_8812_mp_chip
	dwTmp = RTL_R32(0x8b0);
	dwTmp &= ~BIT(8);
	RTL_W32(0x8b0, dwTmp); 
	//printk("0x8b0 = 0x%x\n", dwTmp);
	dwTmp |= BIT(8);
	RTL_W32(0x8b0, dwTmp); 
	//printk("0x8b0 = 0x%x\n", dwTmp);
	}

	delay_us(10);

	if(eRFPath == RF92CD_PATH_A)
		Original_Value = RTL_R32(0xd04);
	else if(eRFPath == RF92CD_PATH_B)
		Original_Value = RTL_R32(0xd44); 

	//printk("_eric_8812 rf_%x = 0x%x \n", RegAddr, Original_Value);

	Original_Value &= 0xfffff;
	
	BitShift =	phy_CalculateBitShift(BitMask);
	Readback_Value = (Original_Value & BitMask) >> BitShift;

	PHY_SetBBReg(priv, 0x838, 0xf, 0x4);		//		CCA on
	
	return (Readback_Value);
}

void phy_RFSerialWrite_8812(struct rtl8192cd_priv *priv, RF92CD_RADIO_PATH_E eRFPath, unsigned int Offset, unsigned int Data)
{
	unsigned int dwTmp = 0;
	unsigned int value = ((Offset << 20) | Data);

	//printk("_eric_8182 phy_RFSerialWrite_8812 +++ \n");

	PHY_SetBBReg(priv, 0x838, 0xf, 0xc);		//		CCA off

	if(eRFPath == RF92CD_PATH_A)
	{
		dwTmp = RTL_R32(0xc90);
		dwTmp &= 0xf0000000;
		dwTmp |= value;
		//printk("_eric_8812 0xc90 = 0x%x \n", dwTmp);
		RTL_W32(0xc90, dwTmp);
	}
	else if(eRFPath == RF92CD_PATH_B)
	{
		dwTmp = RTL_R32(0xe90);
		dwTmp &= 0xf0000000;
		dwTmp |= value;
		//printk("_eric_8812 0xe90 = 0x%x \n", dwTmp);
		RTL_W32(0xe90, dwTmp);
	}

	PHY_SetBBReg(priv, 0x838, 0xf, 0x4);		//		CCA on
}

#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
void PHY_SetRFReg_8812(struct rtl8192cd_priv *priv, RF92CD_RADIO_PATH_E eRFPath, unsigned int RegAddr,
				unsigned int BitMask, unsigned int Data)
{
	unsigned int Original_Value, BitShift, New_Value;
	unsigned long flags;
#if 1
	if(priv->pshare->No_RF_Write == 1
#ifdef MP_TEST
		&& (!priv->pshare->rf_ft_var.mp_specific)
#endif
	)
		return;
#endif
	SAVE_INT_AND_CLI(flags);
	if (BitMask != bMask20Bits) {
		Original_Value = PHY_QueryRFReg_8812(priv, eRFPath, RegAddr, bMask20Bits, 0);
		BitShift = phy_CalculateBitShift(BitMask);
		New_Value = ((Original_Value & (~BitMask)) | (Data << BitShift));
		phy_RFSerialWrite_8812(priv, eRFPath, RegAddr, New_Value);
	} else {
		phy_RFSerialWrite_8812(priv, eRFPath, RegAddr, Data);
	}

	RESTORE_INT(flags);
}

#endif


/**
* Function:	PHY_QueryRFReg
*
* OverView:	Query "Specific bits" to RF register (page 8~)
*
* Input:
*			PADAPTER		Adapter,
*			RF92CD_RADIO_PATH_E	eRFPath,	//Radio path of A/B/C/D
*			u4Byte			RegAddr,		//The target address to be read
*			u4Byte			BitMask		//The target bit position in the target address
*										//to be read
*			u4Byte			dbg_avoid	//set bitmask in reg 0 to prevent RF switchs to debug mode
*
* Output:	None
* Return:		u4Byte			Readback value
* Note:		This function is equal to "GetRFRegSetting" in PHY programming guide
*/
unsigned int PHY_QueryRFReg(struct rtl8192cd_priv *priv, RF92CD_RADIO_PATH_E eRFPath,
				unsigned int RegAddr, unsigned int BitMask, unsigned int dbg_avoid)
{
	unsigned int	Original_Value, Readback_Value, BitShift;

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A)
	if ((GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8881A)) {
		return PHY_QueryRFReg_8812(priv, eRFPath, RegAddr, BitMask, dbg_avoid);
	}
#endif
	
#ifdef CONFIG_RTL_92D_DMDP
	unsigned char reg;
	reg = MAC_PHY_CTRL_MP;
	if (!(RTL_R8(reg) & BIT(1)) && priv->pshare->wlandev_idx == 1){
		return DMDP_PHY_QueryRFReg(0,eRFPath,RegAddr,BitMask,dbg_avoid);
	}
#endif

	Original_Value = phy_RFSerialRead(priv, eRFPath, RegAddr, dbg_avoid);
   	BitShift =  phy_CalculateBitShift(BitMask);
   	Readback_Value = (Original_Value & BitMask) >> BitShift;

	return (Readback_Value);
}


/**
* Function:	PHY_SetRFReg
*
* OverView:	Write "Specific bits" to RF register (page 8~)
*
* Input:
*			PADAPTER		Adapter,
*			RF92CD_RADIO_PATH_E	eRFPath,	//Radio path of A/B/C/D
*			u4Byte			RegAddr,		//The target address to be modified
*			u4Byte			BitMask		//The target bit position in the target address
*										//to be modified
*			u4Byte			Data			//The new register Data in the target bit position
*										//of the target address
*
* Output:	None
* Return:		None
* Note:		This function is equal to "PutRFRegSetting" in PHY programming guide
*/
#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
void PHY_SetRFReg(struct rtl8192cd_priv *priv, RF92CD_RADIO_PATH_E eRFPath, unsigned int RegAddr,
				unsigned int BitMask, unsigned int Data)
{
	unsigned int Original_Value, BitShift, New_Value;
	unsigned long flags;

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A)
	if ((GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8881A)) {
		return PHY_SetRFReg_8812(priv, eRFPath, RegAddr, BitMask, Data);
	}
#endif

	
#ifdef CONFIG_RTL_92D_DMDP
	unsigned char reg;
	reg = MAC_PHY_CTRL_MP;

	if (!(RTL_R8(reg) & BIT(1)) && priv->pshare->wlandev_idx == 1){
		DMDP_PHY_SetRFReg(0, eRFPath, RegAddr, BitMask, Data);
		return;
	}
#endif

	SAVE_INT_AND_CLI(flags);

	if (BitMask != bMask20Bits) {
		Original_Value = phy_RFSerialRead(priv, eRFPath, RegAddr, 1);
		BitShift = phy_CalculateBitShift(BitMask);
		New_Value = ((Original_Value & (~BitMask)) | (Data << BitShift));

		phy_RFSerialWrite(priv, eRFPath, RegAddr, New_Value);
	} else {
		phy_RFSerialWrite(priv, eRFPath, RegAddr, Data);
	}

	RESTORE_INT(flags);
}


static int is_hex(char s)
{
	if (( s >= '0' && s <= '9') || ( s >= 'a' && s <= 'f') || (s >= 'A' && s <= 'F') || (s == 'x' || s == 'X'))
		return 1;
	else
		return 0;
}


static int is_item(char s)
{
	if (s=='t' || s=='a' || s=='b' || s=='l' || s=='e'  || s==':')
		return 1;
	else
		return 0;
}

static unsigned char *get_digit(unsigned char **data)
{
	unsigned char *buf=*data;
	int i=0;

	while(buf[i] && ((buf[i] == ' ') || (buf[i] == '\t')))
		i++;
	*data = &buf[i];

	while(buf[i]) {
		if ((buf[i] == ' ') || (buf[i] == '\t')) {
			buf[i] = '\0';
			break;
		}
		if (buf[i]>='A' && buf[i]<='Z')
			buf[i] += 32;

		if (!is_hex(buf[i])&&!is_item(buf[i]))
			return NULL;
		i++;
	}
	if (i == 0)
		return NULL;
	else
		return &buf[i+1];
}

#ifdef TXPWR_LMT
static unsigned char *get_digit_dot(unsigned char **data)
{
	unsigned char *buf=*data;
	int i=0;

	while(buf[i] && ((buf[i] == ' ') || (buf[i] == '\t')))
		i++;
	
	*data = &buf[i];

	while(buf[i]) {
		if(buf[i] == '.'){
			while((buf[i]==' ') ||(buf[i]=='\t') || (buf[i]=='\0') || (buf[i]=='/'))
				i++;

			i++;
		}
		
		if ((buf[i] == ' ') || (buf[i] == '\t')) {
			buf[i] = '\0';
			break;
		}
		if (buf[i]>='A' && buf[i]<='Z')
			buf[i] += 32;
		
		if (!is_hex(buf[i])&&!is_item(buf[i]))
			return NULL;
		i++;
	}
	if (i == 0)
		return NULL;
	else
		{
			return &buf[i+1];
		}
}


static int get_chnl_lmt_dot(unsigned char *line_head, unsigned int *ch_start, unsigned int *ch_end, unsigned int *limit, unsigned int *target)
{
	unsigned char *next, *next2;
	int base, idx;
	int num=0;
	unsigned char *ch;
	extern int _atoi(char *s, int base);

	*ch_start = *ch_start = '\0';

	// remove comments
	ch = line_head;
	while (1) {
		if ((*ch == '\0') || (*ch == '\n') || (*ch == '\r'))
			break;
		else if (*ch == '/') {
			*ch = '\0';
			break;
		} else {
			ch++;
		}
	}

	next = get_digit_dot(&line_head);
	if (next == NULL)
		return num;
	num++;

	if (!memcmp(line_head,"table",5)) {
		*ch_start = 0;
		*ch_end = 0;
	} else {
//		char *s;
		int format = 0;
		int idx2;

		if ((!memcmp(line_head, "0x", 2)) || (!memcmp(line_head, "0X", 2))) {
			base = 16;
			idx = 2;
		}
		else {
			base = 10;
			idx = 0;
		}
		idx2 = idx;
		while (line_head[idx2]!='\0'){
			//printk("(%c)",line_head[idx2]);
			if (line_head[idx2] == ':'){
				line_head[idx2] = '\0';
				format = 1; // format - start:end
				break;
			}
			idx2++;
		}
		*ch_start = _atoi((char *)&line_head[idx], base);
		if (format==0){
			*ch_end = *ch_start;
		}else{
			*ch_end = _atoi((char *)&line_head[idx2+1], base);
		}
	}

	
	*limit = 0;
	if (next) {
		if (!(next2 = get_digit_dot(&next)))
			return num;
		num++;

		base = 10;
		idx = 0;

		if( (*ch_start == 0) && (*ch_end == 0) )
			*limit = _atoi((char *)&next[idx], base); //In this condition, 'limit' represents "# of table"
		else
			*limit = _convert_2_pwr_dot((char *)&next[idx], base);
		}

	
	*target = 0;
	if (next2) {
		if (!get_digit_dot(&next2))
			return num;
		num++;

			base = 10;
			idx = 0;

		*target = _convert_2_pwr_dot((char *)&next2[idx], base);
		}


	return num;
}
#endif

static int get_offset_val(unsigned char *line_head, unsigned int *u4bRegOffset, unsigned int *u4bRegValue)
{
	unsigned char *next;
	int base, idx;
	int num=0;
	unsigned char *ch;
	extern int _atoi(char *s, int base);

	*u4bRegOffset = *u4bRegValue = '\0';

	ch = line_head;
	while (1) {
		if ((*ch == '\0') || (*ch == '\n') || (*ch == '\r'))
			break;
		else if (*ch == '/') {
			*ch = '\0';
			break;
		} else {
			ch++;
		}
	}

	next = get_digit(&line_head);
	if (next == NULL)
		return num;
	num++;
	if ((!memcmp(line_head, "0x", 2)) || (!memcmp(line_head, "0X", 2))) {
		base = 16;
		idx = 2;
	}
	else {
		base = 10;
		idx = 0;
	}
	*u4bRegOffset = _atoi((char *)&line_head[idx], base);

	if (next) {
		if (!get_digit(&next))
			return num;
		num++;
		if ((!memcmp(next, "0x", 2)) || (!memcmp(next, "0X", 2))) {
			base = 16;
			idx = 2;
		}
		else {
			base = 10;
			idx = 0;
		}
		*u4bRegValue = _atoi((char *)&next[idx], base);
	}
	else
		*u4bRegValue = 0;

	return num;
}


static int get_offset_mask_val(unsigned char *line_head, unsigned int *u4bRegOffset, unsigned int *u4bMask, unsigned int *u4bRegValue)
{
	unsigned char *next, *n1;
	int base, idx;
	int num=0;
	unsigned char *ch;
	extern int _atoi(char *s, int base);

	*u4bRegOffset = *u4bRegValue = *u4bMask = '\0';

	ch = line_head;
	while (1) {
		if ((*ch == '\0') || (*ch == '\n') || (*ch == '\r'))
			break;
		else if (*ch == '/') {
			*ch = '\0';
			break;
		} else {
			ch++;
		}
	}

	next = get_digit(&line_head);
	if (next == NULL)
		return num;
	num++;
	if ((!memcmp(line_head, "0x", 2)) || (!memcmp(line_head, "0X", 2))) {
		base = 16;
		idx = 2;
	}
	else {
		base = 10;
		idx = 0;
	}
	*u4bRegOffset = _atoi((char *)&line_head[idx], base);

	if (next) {
		n1 = get_digit(&next);
		if (n1 == NULL)
			return num;
		num++;
		if ((!memcmp(next, "0x", 2)) || (!memcmp(next, "0X", 2))) {
			base = 16;
			idx = 2;
		}
		else {
			base = 10;
			idx = 0;
		}
		*u4bMask = _atoi((char *)&next[idx], base);

		if (n1) {
			if (!get_digit(&n1))
				return num;
			num++;
			if ((!memcmp(n1, "0x", 2)) || (!memcmp(n1, "0X", 2))) {
				base = 16;
				idx = 2;
			}
			else {
				base = 10;
				idx = 0;
			}
			*u4bRegValue = _atoi((char *)&n1[idx], base);
		}
		else
			*u4bRegValue = 0;
	}
	else
		*u4bMask = 0;

	return num;
}


unsigned char *get_line(unsigned char **line)
{
	unsigned char *p=*line;

	while (*p && ((*p == '\n') || (*p == '\r')))
		p++;

	if (*p == '\0') {
		*line = NULL;
		return NULL;
	}
	*line = p;

	while (*p && (*p != '\n') && (*p != '\r'))
			p++;

	*p = '\0';
	return p+1;
}

#ifdef TXPWR_LMT

int ch2idx(int ch)
{
	int val=-1;
	// |1~14|36, 38, 40, ..., 64|100, 102, ..., 140|149, 151, ..., 165|
	if (ch<=14)
		val = ch-1;
	else if (ch<=64)
		val = ((ch-36)>>1)+14;
	else if (ch<=140)
		val = ((ch-100)>>1)+29;
	else if (ch<=165)
		val = ((ch-149)>>1)+50;

	return val;
}

void find_pwr_limit(struct rtl8192cd_priv *priv)
{
	int tmp = ch2idx(priv->pshare->working_channel);
	int tmp2 = ch2idx(priv->pmib->dot11RFEntry.dot11channel); //for CCK, OFDM & 20M bw

	//printk("tmp %d tmp2 %d working_channel %d dot11channel %d\n", 
		//tmp, tmp2, priv->pshare->working_channel, priv->pmib->dot11RFEntry.dot11channel);

	if ((tmp>=0) && (tmp2>=0)) {
			priv->pshare->txpwr_lmt_CCK = priv->pshare->ch_pwr_lmtCCK[tmp2];
			priv->pshare->txpwr_lmt_OFDM = priv->pshare->ch_pwr_lmtOFDM[tmp2];
			priv->pshare->tgpwr_CCK = priv->pshare->ch_tgpwr_CCK[tmp2];
			priv->pshare->tgpwr_OFDM = priv->pshare->ch_tgpwr_OFDM[tmp2];
			
		if (priv->pshare->is_40m_bw){
			priv->pshare->txpwr_lmt_HT1S = priv->pshare->ch_pwr_lmtHT40_1S[tmp];
			priv->pshare->txpwr_lmt_HT2S = priv->pshare->ch_pwr_lmtHT40_2S[tmp];
			priv->pshare->tgpwr_HT1S = priv->pshare->ch_tgpwr_HT40_1S[tmp];
			priv->pshare->tgpwr_HT2S = priv->pshare->ch_tgpwr_HT40_2S[tmp];
		} else { //if 20M bw, tmp == tmp2 ??
			priv->pshare->txpwr_lmt_HT1S = priv->pshare->ch_pwr_lmtHT20_1S[tmp2];
			priv->pshare->txpwr_lmt_HT2S = priv->pshare->ch_pwr_lmtHT20_2S[tmp2];
			priv->pshare->tgpwr_HT1S = priv->pshare->ch_tgpwr_HT20_1S[tmp2];
			priv->pshare->tgpwr_HT2S = priv->pshare->ch_tgpwr_HT20_2S[tmp2];
		}
	} else {
		priv->pshare->txpwr_lmt_CCK = 0;
		priv->pshare->txpwr_lmt_OFDM = 0;
		priv->pshare->txpwr_lmt_HT1S = 0;
		priv->pshare->txpwr_lmt_HT2S = 0;
		
		printk("Cannot map current working channel to find power limit!\n");
	}

	//printk("txpwr_lmt_OFDM %d tgpwr_OFDM %d\n", priv->pshare->txpwr_lmt_OFDM, priv->pshare->tgpwr_OFDM);
	
}


int PHY_ConfigTXLmtWithParaFile(struct rtl8192cd_priv *priv)
{
	int read_bytes, num, len=0;
	unsigned int  ch_start, ch_end, limit, target;
	unsigned char *mem_ptr, *line_head, *next_head;
	int	idx=0, tbl_idx[6], set_en=0, type;
	
	priv->pshare->txpwr_lmt_CCK = 0;
	priv->pshare->txpwr_lmt_OFDM = 0;
	priv->pshare->txpwr_lmt_HT1S = 0;
	priv->pshare->txpwr_lmt_HT2S = 0;

	struct TxPwrLmtTable *reg_table = (struct TxPwrLmtTable *)priv->pshare->txpwr_lmt_buf;

	if ((GET_CHIP_VER(priv)!=VERSION_8192D) && (GET_CHIP_VER(priv)!=VERSION_8192C) && (GET_CHIP_VER(priv)!=VERSION_8188C)) {
		printk("[%s]NOT support! TXPWR_LMT is for RTL8192D & 92C/88C ONLY!\n",__FUNCTION__);
		return -1;
	}

	if((mem_ptr = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
		printk("PHY_ConfigMACWithParaFile(): not enough memory\n");
		return -1;
	}


	tbl_idx[0] = 1;
	tbl_idx[1] = 2;
	tbl_idx[2] = 3;
	tbl_idx[3] = 4;
	tbl_idx[4] = 5;
	tbl_idx[5] = 6;


	DEBUG_INFO("regdomain=%d tbl_idx=%d,%d\n",priv->pmib->dot11StationConfigEntry.dot11RegDomain, tbl_idx[0],tbl_idx[1]);


	memset(mem_ptr, 0, MAX_CONFIG_FILE_SIZE); // clear memory

	printk("[%s][TXPWR_LMT]\n",__FUNCTION__);


#ifdef CONFIG_RTL_92C_SUPPORT
	if((GET_CHIP_VER(priv)==VERSION_8192C) || (GET_CHIP_VER(priv)==VERSION_8188C))
	{
		 if(priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_FCC){
			 //printk("\nFCC LMT!!!\n");
			 next_head = data_TXPWR_LMT_92c_FCC_start;
			 read_bytes = (int)(data_TXPWR_LMT_92c_FCC_end - data_TXPWR_LMT_92c_FCC_start);
		 }
		 else if(priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_ETSI){
			 //printk("\nCE LMT!!!\n");
			 next_head = data_TXPWR_LMT_92c_CE_start;
			 read_bytes = (int)(data_TXPWR_LMT_92c_CE_end - data_TXPWR_LMT_92c_CE_start);
		 }
		 else{
			 //printk("\nOther !!!\n");
		 next_head = data_TXPWR_LMT_92c_start;
		 read_bytes = (int)(data_TXPWR_LMT_92c_end - data_TXPWR_LMT_92c_start);
		 }
	}
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	if(GET_CHIP_VER(priv)==VERSION_8192D)
	{
	if(priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_FCC){
		//printk("\nFCC LMT!!!\n");
		next_head = data_TXPWR_LMT_FCC_start;
		read_bytes = (int)(data_TXPWR_LMT_FCC_end - data_TXPWR_LMT_FCC_start);
	}
	else if(priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_ETSI){
		//printk("\nCE LMT!!!\n");
		next_head = data_TXPWR_LMT_CE_start;
		read_bytes = (int)(data_TXPWR_LMT_CE_end - data_TXPWR_LMT_CE_start);
	}
	else{
		//printk("\nOther !!!\n");
	next_head = data_TXPWR_LMT_start;
	read_bytes = (int)(data_TXPWR_LMT_end - data_TXPWR_LMT_start);
	}
	}
#endif

	memcpy(mem_ptr, next_head, read_bytes);

	next_head = mem_ptr;
	while (1) {
		line_head = next_head;
		next_head = get_line(&line_head);

		if (line_head == NULL)
			break;

		if (line_head[0] == '/')
			continue;

		num = get_chnl_lmt_dot(line_head, &ch_start, &ch_end, &limit, &target);

		if (num > 0) {
			reg_table[len].start = ch_start;
			reg_table[len].end = ch_end;
			reg_table[len].limit = limit;
			reg_table[len].target = target;	

			len++;
			if ((len * sizeof(struct TxPwrLmtTable)) > MAC_REG_SIZE)
				break;
		}
	}

	reg_table[len].start = 0xff;

	kfree(mem_ptr);

	if ((len * sizeof(struct TxPwrLmtTable)) > MAC_REG_SIZE) {
		printk("TXPWR_LMT table buffer not large enough!\n");
		return -1;
	}

	num = 0;
	while (1) {
		ch_start = reg_table[num].start;
		ch_end = reg_table[num].end;
		limit = reg_table[num].limit;
		target = reg_table[num].target;

		//printk(">> %d-%d-%d-%d\n",ch_start,ch_end,limit,target);
		if (ch_start == 0xff)
			break;

		if (ch_start== 0 && ch_end == 0){
			if (limit == tbl_idx[0]) {
				set_en=1;
				type = 0;
				memset(priv->pshare->ch_pwr_lmtCCK, 0, SUPPORT_CH_NUM);
				memset(priv->pshare->ch_tgpwr_CCK, 0, SUPPORT_CH_NUM);
			}else if (limit == tbl_idx[1]) {
				set_en=1;
				type = 1;
				memset(priv->pshare->ch_pwr_lmtOFDM, 0, SUPPORT_CH_NUM);
				memset(priv->pshare->ch_tgpwr_OFDM, 0, SUPPORT_CH_NUM);
			} else if (limit == tbl_idx[2]) {
				set_en=1;
				type = 2;
				memset(priv->pshare->ch_pwr_lmtHT20_1S, 0, SUPPORT_CH_NUM);
				memset(priv->pshare->ch_tgpwr_HT20_1S, 0, SUPPORT_CH_NUM);
			} else if (limit == tbl_idx[3]) {
				set_en=1;
				type = 3;
				memset(priv->pshare->ch_pwr_lmtHT20_2S, 0, SUPPORT_CH_NUM);
				memset(priv->pshare->ch_tgpwr_HT20_2S, 0, SUPPORT_CH_NUM);
			} else if (limit == tbl_idx[4]) {
				set_en=1;
				type = 4;
				memset(priv->pshare->ch_pwr_lmtHT40_1S, 0, SUPPORT_CH_NUM);
				memset(priv->pshare->ch_tgpwr_HT40_1S, 0, SUPPORT_CH_NUM);
			} else if (limit == tbl_idx[5]) {
				set_en=1;
				type = 5;
				memset(priv->pshare->ch_pwr_lmtHT40_2S, 0, SUPPORT_CH_NUM);
				memset(priv->pshare->ch_tgpwr_HT40_2S, 0, SUPPORT_CH_NUM);
			} else {
				set_en = 0;
			}
		}

		if (set_en && ch_start){
			int j;
			for (j=ch2idx(ch_start);j<=ch2idx(ch_end);j++){
				if (j<0||j>=SUPPORT_CH_NUM){
					panic_printk("channel out of bound!!\n");
					break;
				}
				
				if (type == 0){
					priv->pshare->ch_pwr_lmtCCK[j] = limit;
					priv->pshare->ch_tgpwr_CCK[j] = target;
				}else if (type == 1){
					priv->pshare->ch_pwr_lmtOFDM[j] = limit;
					priv->pshare->ch_tgpwr_OFDM[j] = target;
				}else if (type==2){
					priv->pshare->ch_pwr_lmtHT20_1S[j] = limit;
					priv->pshare->ch_tgpwr_HT20_1S[j] = target;
				}else if (type==3){
					priv->pshare->ch_pwr_lmtHT20_2S[j] = limit;
					priv->pshare->ch_tgpwr_HT20_2S[j] = target;
				}else if (type==4){
					priv->pshare->ch_pwr_lmtHT40_1S[j] = limit;
					priv->pshare->ch_tgpwr_HT40_1S[j] = target;
				}else if (type==5){
					priv->pshare->ch_pwr_lmtHT40_2S[j] = limit;
					priv->pshare->ch_tgpwr_HT40_2S[j] = target;
				}
			}
		}

		num++;
	}

	return 0;
}

#endif


#ifdef _TRACKING_TABLE_FILE


#ifndef TXPWR_LMT

int _convert_2_pwr_tracking(char *s, int base)
{
	int k = 0;

	k = 0;
	if (base == 10) {
		while (*s >= '0' && *s <= '9') {
			k = 10 * k + (*s - '0');
			s++;
		}
	}
	else
		return 0;

	return k;
}


static unsigned char *get_digit_tracking(unsigned char **data)
{
	unsigned char *buf=*data;
	int i=0;
	
	*data = &buf[i];

	while(1)
	{
		if((buf[i] == '\n') || (buf[i] == '\r'))
		{
			return NULL;
		}

		if((buf[i] == ' ') || (buf[i] == '\t'))
			break;

		i++;
	}

	while(1) 
	{		

		if((buf[i] == '\n') || (buf[i] == '\r'))
		{
			return NULL;
		}

		if((buf[i]>= '0') && (buf[i] <= '9'))
		{
			//printk("found buf[i] = %c \n", buf[i]);
			return &buf[i];
		}

		i++;
	}
	
}

#endif


void input_tracking_value(struct rtl8192cd_priv *priv, int offset, int num, int value)
{

	switch (offset) {
		case CCKA_P:
		case CCKA_N:
		case CCKB_P:
		case CCKB_N:
			offset = (offset%4);
			priv->pshare->txpwr_tracking_2G_CCK[offset][num] = value;
			break;
		case A_P:
		case A_N:
		case B_P:
		case B_N:
			offset = (offset%4);
			priv->pshare->txpwr_tracking_2G_OFDM[offset][num] = value;
			break;
		case LA_P:
		case LA_N:
		case LB_P:
		case LB_N:
			offset = (offset%4);
			priv->pshare->txpwr_tracking_5GL[offset][num] = value;
			break;
		case MA_P:
		case MA_N:
		case MB_P:
		case MB_N:
			offset = (offset%4);
			priv->pshare->txpwr_tracking_5GM[offset][num] = value;
			break;
		case HA_P:
		case HA_N:
		case HB_P:
		case HB_N:
			offset = (offset%4);
			priv->pshare->txpwr_tracking_5GH[offset][num] = value;
			break;
	default:
			break;

	}


}



static int get_tracking_table(struct rtl8192cd_priv *priv, unsigned char *line_head)
{
	unsigned char *next, *next2;
	int base, idx;
	int num=0;
	int offset = 0;
	unsigned char *swim;
	extern int _atoi(char *s, int base);

	// remove comments
	swim = line_head+1;
	
	for(offset=0; offset<TXPWR_TRACKING_NAME_NUM; offset++)
	{
		if(!memcmp(line_head, TXPWR_TRACKING_NAME[offset], strlen(TXPWR_TRACKING_NAME[offset])))
			break;
	}

	if(offset >= TXPWR_TRACKING_NAME_NUM)
		return offset;

	//printk("_Eric offset = %d \n", offset);
	//printk("_Eric line_head = %s \n", line_head);

	next = get_digit_tracking(&swim);

	while(1)
	{
		
		if (next == NULL)
			break;
		
		if (next) 
		{
			
			base = 10;
			idx = 0;
			
			//printk("num#%d = %d \n", num, _convert_2_pwr_tracking((char *)&next[idx], base));
			input_tracking_value(priv, offset, num, _convert_2_pwr_tracking((char *)&next[idx], base));

			num++;
		}
		else
			break;

		if(num >= index_mapping_NUM_MAX)
			break;

		next = get_digit_tracking(&next);

	}


	return offset;
	
}


void check_tracking_table(struct rtl8192cd_priv *priv)
{

	int tmp = 0;
	int tmp2 = 0;

	for(tmp = 0; tmp<4; tmp++)
	{
		for(tmp2 = 1; tmp2<index_mapping_NUM_MAX; tmp2 ++)
		{
			if(priv->pshare->txpwr_tracking_2G_CCK[tmp][tmp2] < priv->pshare->txpwr_tracking_2G_CCK[tmp][tmp2-1])
				priv->pshare->txpwr_tracking_2G_CCK[tmp][tmp2] = priv->pshare->txpwr_tracking_2G_CCK[tmp][tmp2-1];
		}
	}

	for(tmp = 0; tmp<4; tmp++)
	{
		for(tmp2 = 1; tmp2<index_mapping_NUM_MAX; tmp2 ++)
		{
			if(priv->pshare->txpwr_tracking_2G_OFDM[tmp][tmp2] < priv->pshare->txpwr_tracking_2G_OFDM[tmp][tmp2-1])
				priv->pshare->txpwr_tracking_2G_OFDM[tmp][tmp2] = priv->pshare->txpwr_tracking_2G_OFDM[tmp][tmp2-1];
		}
	}

	for(tmp = 0; tmp<4; tmp++)
	{
		for(tmp2 = 1; tmp2<index_mapping_NUM_MAX; tmp2 ++)
		{
			if(priv->pshare->txpwr_tracking_5GL[tmp][tmp2] < priv->pshare->txpwr_tracking_5GL[tmp][tmp2-1])
				priv->pshare->txpwr_tracking_5GL[tmp][tmp2] = priv->pshare->txpwr_tracking_5GL[tmp][tmp2-1];
		}
	}

	for(tmp = 0; tmp<4; tmp++)
	{
		for(tmp2 = 1; tmp2<index_mapping_NUM_MAX; tmp2 ++)
		{
			if(priv->pshare->txpwr_tracking_5GM[tmp][tmp2] < priv->pshare->txpwr_tracking_5GM[tmp][tmp2-1])
				priv->pshare->txpwr_tracking_5GM[tmp][tmp2] = priv->pshare->txpwr_tracking_5GM[tmp][tmp2-1];
		}
	}

	for(tmp = 0; tmp<4; tmp++)
	{
		for(tmp2 = 1; tmp2<index_mapping_NUM_MAX; tmp2 ++)
		{
			if(priv->pshare->txpwr_tracking_5GH[tmp][tmp2] < priv->pshare->txpwr_tracking_5GH[tmp][tmp2-1])
				priv->pshare->txpwr_tracking_5GH[tmp][tmp2] = priv->pshare->txpwr_tracking_5GH[tmp][tmp2-1];
		}
	}

}

int PHY_ConfigTXPwrTrackingWithParaFile(struct rtl8192cd_priv *priv)
{
	int read_bytes, num, len=0;
	unsigned int  ch_start, ch_end, limit, target;
	unsigned char *mem_ptr, *line_head, *next_head;
	int	idx=0, tbl_idx[6], set_en=0, type;

	memset(priv->pshare->txpwr_tracking_2G_CCK, 0, (4*index_mapping_NUM_MAX));
	memset(priv->pshare->txpwr_tracking_2G_OFDM, 0, (4*index_mapping_NUM_MAX));
	memset(priv->pshare->txpwr_tracking_5GL, 0, (4*index_mapping_NUM_MAX));
	memset(priv->pshare->txpwr_tracking_5GM, 0, (4*index_mapping_NUM_MAX));
	memset(priv->pshare->txpwr_tracking_5GH, 0, (4*index_mapping_NUM_MAX));

	if((mem_ptr = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
		printk("PHY_ConfigMACWithParaFile(): not enough memory\n");
		return -1;
	}

	memset(mem_ptr, 0, MAX_CONFIG_FILE_SIZE); // clear memory


	if((GET_CHIP_VER(priv)== VERSION_8188C) || (GET_CHIP_VER(priv)== VERSION_8192C)) //_Eric_?? any other IC types ??
	{
#ifdef HIGH_POWER_EXT_PA
		printk("[%s][REG_TXPWR_TRK_hp]\n",__FUNCTION__);
		next_head = data_REG_TXPWR_TRK_hp_start;
		read_bytes = (int)(data_REG_TXPWR_TRK_hp_end - data_REG_TXPWR_TRK_hp_start);
#else
		printk("[%s][REG_TXPWR_TRK]\n",__FUNCTION__);
		next_head = data_REG_TXPWR_TRK_start;
		read_bytes = (int)(data_REG_TXPWR_TRK_end - data_REG_TXPWR_TRK_start);
#endif
	}
	else if(GET_CHIP_VER(priv)== VERSION_8192D)
	{
#ifdef HIGH_POWER_EXT_PA
		printk("[%s][REG_TXPWR_TRK_n_92d_hp]\n",__FUNCTION__);
		next_head = data_REG_TXPWR_TRK_n_92d_hp_start;
		read_bytes = (int)(data_REG_TXPWR_TRK_n_92d_hp_end - data_REG_TXPWR_TRK_n_92d_hp_start);
#else
		printk("[%s][REG_TXPWR_TRK_n_92d]\n",__FUNCTION__);
		next_head = data_REG_TXPWR_TRK_n_92d_start;
		read_bytes = (int)(data_REG_TXPWR_TRK_n_92d_end - data_REG_TXPWR_TRK_n_92d_start);
#endif
	}
	else
	{
		printk("[%s][NOT SUPPORT]\n",__FUNCTION__);
		return -1;
	}

	memcpy(mem_ptr, next_head, read_bytes);

	next_head = mem_ptr;
	
	while (1) {
		line_head = next_head;
		next_head = get_line(&line_head);

		if (line_head == NULL)
			break;

		if (line_head[0] == '/')
			continue;

		num = get_tracking_table(priv, line_head);

	}

	check_tracking_table(priv); 

#if 0

{
	int tmp = 0;
	int tmp2 = 0;

	for(tmp = 0; tmp<4; tmp++)
	{
		printk("txpwr_tracking_2G_CCK #%d = ", tmp);
		for(tmp2 = 0; tmp2<index_mapping_NUM_MAX; tmp2 ++)
		{
			printk("%d ", priv->pshare->txpwr_tracking_2G_CCK[tmp][tmp2]); 
		}
		printk("\n");
	}

	for(tmp = 0; tmp<4; tmp++)
	{
		printk("txpwr_tracking_2G_OFDM #%d = ", tmp);
		for(tmp2 = 0; tmp2<index_mapping_NUM_MAX; tmp2 ++)
		{
			printk("%d ", priv->pshare->txpwr_tracking_2G_OFDM[tmp][tmp2]); 
		}
		printk("\n");
	}

	for(tmp = 0; tmp<4; tmp++)
	{
		printk("txpwr_tracking_5GL #%d = ", tmp);
		for(tmp2 = 0; tmp2<index_mapping_NUM_MAX; tmp2 ++)
		{
			printk("%d ", priv->pshare->txpwr_tracking_5GL[tmp][tmp2]); 
		}
		printk("\n");
	}

	for(tmp = 0; tmp<4; tmp++)
	{
		printk("txpwr_tracking_5GM #%d = ", tmp);
		for(tmp2 = 0; tmp2<index_mapping_NUM_MAX; tmp2 ++)
		{
			printk("%d ", priv->pshare->txpwr_tracking_5GM[tmp][tmp2]); 
		}
		printk("\n");
	}

	for(tmp = 0; tmp<4; tmp++)
	{
		printk("txpwr_tracking_5GH #%d = ", tmp);
		for(tmp2 = 0; tmp2<index_mapping_NUM_MAX; tmp2 ++)
		{
			printk("%d ", priv->pshare->txpwr_tracking_5GH[tmp][tmp2]); 
		}
		printk("\n");
	}

}

#endif

	kfree(mem_ptr);

	return 0;
}
#endif


#ifdef _DEBUG_RTL8192CD_

//_TXPWR_REDEFINE
void Read_PG_File(struct rtl8192cd_priv *priv, int reg_file, int table_number, 
				char *MCSTxAgcOffset_A, char *MCSTxAgcOffset_B, char *OFDMTxAgcOffset_A,
				char *OFDMTxAgcOffset_B, char *CCKTxAgc_A, char *CCKTxAgc_B)
{
	int                read_bytes=0, num, len=0;
	unsigned int       u4bRegOffset, u4bRegValue, u4bRegMask;
	unsigned char      *mem_ptr, *line_head, *next_head=NULL;
	struct PhyRegTable *phyreg_table=NULL;
	struct MacRegTable *macreg_table=NULL;
	unsigned short     max_len=0;
	int                file_format=TWO_COLUMN;
#ifdef CONFIG_RTL_92D_SUPPORT
	int				idx=0, pg_tbl_idx=table_number, write_en=0;

	int tmp_rTxAGC_A_CCK1_Mcs32= 0;
	int tmp_rTxAGC_B_CCK5_1_Mcs32 = 0;
	int prev_reg = 0;
#endif

	//printk("PHYREG_PG = %d\n", PHYREG_PG);

	if (reg_file == PHYREG_PG) {
		//printk("[%s][PHY_REG_PG]\n",__FUNCTION__);
		
#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8192D){

			if(priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_FCC){
				//printk("\nFCC PG!!!\n");
				next_head = data_PHY_REG_PG_FCC_start;
				read_bytes = (int)(data_PHY_REG_PG_FCC_end - data_PHY_REG_PG_FCC_start);
			}
			else if(priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_ETSI){
				//printk("\nCE PG!!!\n");
				next_head = data_PHY_REG_PG_CE_start;
				read_bytes = (int)(data_PHY_REG_PG_CE_end - data_PHY_REG_PG_CE_start);
			}
			else{
				//printk("\nOTHER PG!!!\n");
				next_head = data_PHY_REG_PG_start;
				read_bytes = (int)(data_PHY_REG_PG_end - data_PHY_REG_PG_start);
			}

#ifdef HIGH_POWER_EXT_PA
			if( priv->pshare->rf_ft_var.use_ext_pa) {
				//printk("[%s][data_PHY_REG_PG_92d_hp]\n", __FUNCTION__);
				next_head = data_PHY_REG_PG_92d_hp_start;
				read_bytes = (int)(data_PHY_REG_PG_92d_hp_end - data_PHY_REG_PG_92d_hp_start);
			}
#endif

		}
#endif //CONFIG_RTL_92D_SUPPORT

#ifdef CONFIG_RTL_92C_SUPPORT
		if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)){
#ifdef HIGH_POWER_EXT_PA
			if( priv->pshare->rf_ft_var.use_ext_pa) {
				//printk("[%s][data_PHY_REG_PG_hp]\n", __FUNCTION__);
				next_head = data_PHY_REG_PG_hp_start;
				read_bytes = (int)(data_PHY_REG_PG_hp_end - data_PHY_REG_PG_hp_start);

			} else
#endif
			{
				//printk("[%s][data_PHY_REG_PG_92C]\n", __FUNCTION__);
				next_head = data_PHY_REG_PG_92C_start;
				read_bytes = (int)(data_PHY_REG_PG_92C_end - data_PHY_REG_PG_92C_start);
			}
		}
#endif //CONFIG_RTL_92C_SUPPORT
		macreg_table = (struct MacRegTable *)priv->pshare->phy_reg_pg_buf;
		max_len = PHY_REG_PG_SIZE;
		file_format = THREE_COLUMN;
	}

#ifdef CONFIG_RTL_92D_SUPPORT
	else if (reg_file == PHYREG) {
		if (GET_CHIP_VER(priv)==VERSION_8192D) {
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_buf;
			//printk("[%s][PHY_REG_n]\n",__FUNCTION__);
			next_head = data_PHY_REG_n_start;
			read_bytes = (int)(data_PHY_REG_n_end - data_PHY_REG_n_start);
			max_len = PHY_REG_SIZE;
		}
	}
#endif // CONFIG_RTL_92D_SUPPORT

#ifdef MP_TEST
	else if (reg_file == PHYREG_MP) {
#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8192D) {
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_mp_buf;
			//printk("[%s][PHY_REG_MP_n]\n",__FUNCTION__);
			next_head = data_PHY_REG_MP_n_start;
			read_bytes = (int)(data_PHY_REG_MP_n_end - data_PHY_REG_MP_n_start);
			max_len = PHY_REG_SIZE;
		}
#endif
#ifdef CONFIG_RTL_92C_SUPPORT
		if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)){
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_mp_buf;
			next_head = data_PHY_REG_MP_n_92C_start;
			read_bytes = (int)(data_PHY_REG_MP_n_92C_end - data_PHY_REG_MP_n_92C_start);
			max_len = PHY_REG_SIZE;
		}
#endif
	}
#endif

	{
		if((mem_ptr = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
			printk("PHY_ConfigBBWithParaFile(): not enough memory\n");
#ifdef __ECOS
			return;
#else
			return -1;
#endif
		}

		memset(mem_ptr, 0, MAX_CONFIG_FILE_SIZE); // clear memory
		memcpy(mem_ptr, next_head, read_bytes);

		next_head = mem_ptr;
		while (1) {
			line_head = next_head;
			next_head = get_line(&line_head);
			if (line_head == NULL)
				break;

			if (line_head[0] == '/')
				continue;

			if (file_format == TWO_COLUMN) {
				num = get_offset_val(line_head, &u4bRegOffset, &u4bRegValue);
				if (num > 0) {
					phyreg_table[len].offset = u4bRegOffset;
					phyreg_table[len].value = u4bRegValue;
					len++;
#if defined(CONFIG_RTL865X_WTDOG) || defined(CONFIG_RTL_WTDOG)
#if (defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)) && defined(CONFIG_RTL_92D_SUPPORT)
					//if ((len&0x7ff)==0)
						//REG32(BSP_WDTCNR) |=  1 << 23;
#endif
#endif
					if (u4bRegOffset == 0xff)
						break;
					if ((len * sizeof(struct PhyRegTable)) > max_len)
						break;
				}
			}
			else {
				num = get_offset_mask_val(line_head, &u4bRegOffset, &u4bRegMask ,&u4bRegValue);
				if (num > 0) {
					macreg_table[len].offset = u4bRegOffset;
					macreg_table[len].mask = u4bRegMask;
					macreg_table[len].value = u4bRegValue;
					len++;
					if (u4bRegOffset == 0xff)
						break;
					if ((len * sizeof(struct MacRegTable)) > max_len)
						break;
#if defined(CONFIG_RTL865X_WTDOG) || defined(CONFIG_RTL_WTDOG)
#if (defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)) && defined(CONFIG_RTL_92D_SUPPORT)
					//if ((len&0x7ff)==0)
						//REG32(BSP_WDTCNR) |=  1 << 23;
#endif
#endif
				}
			}
		}
#ifdef __KERNEL__
		kfree(mem_ptr);
#endif
		if ((len * sizeof(struct PhyRegTable)) > max_len) {
			printk("PHY REG table buffer not large enough!\n");
#ifdef __ECOS
			return;
#else
			return -1;
#endif
		}
	}

	num = 0;
	while (1) {
		if (file_format == THREE_COLUMN) {
			u4bRegOffset = macreg_table[num].offset;
			u4bRegValue = macreg_table[num].value;
			u4bRegMask = macreg_table[num].mask;
		}
		else {
			u4bRegOffset = phyreg_table[num].offset;
			u4bRegValue = phyreg_table[num].value;
		}

		if (u4bRegOffset == 0xff)
			break;
		else if (file_format == THREE_COLUMN){

#ifdef CONFIG_RTL_92D_SUPPORT
			if (reg_file == PHYREG_PG && GET_CHIP_VER(priv)==VERSION_8192D) {
				if (u4bRegOffset==0xe00){
					if (idx == pg_tbl_idx)
						write_en=1;
					idx++;
				}
				if (write_en){
					//PHY_SetBBReg(priv, u4bRegOffset, u4bRegMask, u4bRegValue);
					//printk("3C- 92D %x %x %x \n", u4bRegOffset, u4bRegMask, u4bRegValue);
					if(u4bRegMask != bMaskDWord)
					{
						int BitShift = phy_CalculateBitShift(u4bRegMask);
						u4bRegValue = (u4bRegValue << BitShift);
					}

					//=== PATH A ===
						
						if(u4bRegOffset == rTxAGC_A_Mcs03_Mcs00)
							*(unsigned int *)(&MCSTxAgcOffset_A[0]) = cpu_to_be32(u4bRegValue); 
						if(u4bRegOffset == rTxAGC_A_Mcs07_Mcs04)
							*(unsigned int *)(&MCSTxAgcOffset_A[4]) = cpu_to_be32(u4bRegValue); 
						if(u4bRegOffset == rTxAGC_A_Mcs11_Mcs08)
							*(unsigned int *)(&MCSTxAgcOffset_A[8]) = cpu_to_be32(u4bRegValue);
						if(u4bRegOffset == rTxAGC_A_Mcs15_Mcs12)
							*(unsigned int *)(&MCSTxAgcOffset_A[12]) = cpu_to_be32(u4bRegValue);
						
						if(u4bRegOffset == rTxAGC_A_Rate18_06)
							*(unsigned int *)(&OFDMTxAgcOffset_A[0]) = cpu_to_be32(u4bRegValue);
						if(u4bRegOffset == rTxAGC_A_Rate54_24)
							*(unsigned int *)(&OFDMTxAgcOffset_A[4]) = cpu_to_be32(u4bRegValue);

						if(u4bRegOffset == rTxAGC_A_CCK1_Mcs32)
						{
							tmp_rTxAGC_A_CCK1_Mcs32 = ((u4bRegValue & 0xff00) >> phy_CalculateBitShift(0xff00));
							prev_reg = rTxAGC_A_CCK1_Mcs32;
						}
						
						if(u4bRegOffset == rTxAGC_A_CCK11_2_B_CCK11)
						{
							if(prev_reg == rTxAGC_A_CCK1_Mcs32)
							{
								//printk("\n%x %x %x\n", tmp_rTxAGC_A_CCK1_Mcs32, u4bRegValue, cpu_to_be32((u4bRegValue & 0xffffff00) | tmp_rTxAGC_A_CCK1_Mcs32));
								
								*(unsigned int *)(&CCKTxAgc_A[0]) = 
									cpu_to_be32((u4bRegValue & 0xffffff00) | tmp_rTxAGC_A_CCK1_Mcs32);
							}
						}

					//=== PATH B ===

						if(u4bRegOffset == rTxAGC_B_Mcs03_Mcs00)
							*(unsigned int *)(&MCSTxAgcOffset_B[0]) = cpu_to_be32(u4bRegValue); 
						if(u4bRegOffset == rTxAGC_B_Mcs07_Mcs04)
							*(unsigned int *)(&MCSTxAgcOffset_B[4]) = cpu_to_be32(u4bRegValue); 
						if(u4bRegOffset == rTxAGC_B_Mcs11_Mcs08)
							*(unsigned int *)(&MCSTxAgcOffset_B[8]) = cpu_to_be32(u4bRegValue);
						if(u4bRegOffset == rTxAGC_B_Mcs15_Mcs12)
							*(unsigned int *)(&MCSTxAgcOffset_B[12]) = cpu_to_be32(u4bRegValue);
						
						if(u4bRegOffset == rTxAGC_B_Rate18_06)
							*(unsigned int *)(&OFDMTxAgcOffset_B[0]) = cpu_to_be32(u4bRegValue);
						if(u4bRegOffset == rTxAGC_B_Rate54_24)
							*(unsigned int *)(&OFDMTxAgcOffset_B[4]) = cpu_to_be32(u4bRegValue);

						if(u4bRegOffset == rTxAGC_B_CCK5_1_Mcs32)
						{
							tmp_rTxAGC_B_CCK5_1_Mcs32 = u4bRegValue;
							prev_reg = rTxAGC_B_CCK5_1_Mcs32;
						}

						if(u4bRegOffset == rTxAGC_A_CCK11_2_B_CCK11)
						{
							if(prev_reg == rTxAGC_B_CCK5_1_Mcs32)
							{
								//printk("\n%x %x %x\n", tmp_rTxAGC_B_CCK5_1_Mcs32, u4bRegValue, cpu_to_be32((u4bRegValue << 24) | (tmp_rTxAGC_B_CCK5_1_Mcs32 >> 8)));
								*(unsigned int *)(&CCKTxAgc_B[0]) = cpu_to_be32((u4bRegValue << 24) | (tmp_rTxAGC_B_CCK5_1_Mcs32 >> 8));
							}
						}
								
					if (u4bRegOffset==0x868){
						write_en = 0;
						break;
					}
				}
			}else
#endif
			{
				//PHY_SetBBReg(priv, u4bRegOffset, u4bRegMask, u4bRegValue);
				//printk("3C - 92C %x %x %x \n", u4bRegOffset, u4bRegMask, u4bRegValue);
			}
		}
		else
		{
			//printk("Not 3C - %x %x %x \n", u4bRegOffset, bMaskDWord, u4bRegValue);
			//PHY_SetBBReg(priv, u4bRegOffset, bMaskDWord, u4bRegValue);
		}
		num++;
	}

#ifdef __ECOS
	return;
#else
	return 0;
#endif
}

#endif

/*-----------------------------------------------------------------------------
 * Function:    PHY_ConfigBBWithParaFile()
 *
 * Overview:    This function read BB parameters from general file format, and do register
 *			  Read/Write
 *
 * Input:      	PADAPTER		Adapter
 *			ps1Byte 			pFileName
 *
 * Output:      NONE
 *
 * Return:      RT_STATUS_SUCCESS: configuration file exist
 *
 *---------------------------------------------------------------------------*/
int PHY_ConfigBBWithParaFile(struct rtl8192cd_priv *priv, int reg_file)
{
	int                read_bytes=0, num, len=0;
	unsigned int       u4bRegOffset, u4bRegValue, u4bRegMask=0;
	int 			   file_format=TWO_COLUMN;
	unsigned char      *mem_ptr, *line_head, *next_head=NULL;
	struct PhyRegTable *phyreg_table=NULL;
	struct MacRegTable *macreg_table=NULL;
	unsigned short     max_len=0;
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
	int				idx=0, pg_tbl_idx=BGN_2040_ALL, write_en=0;
#endif

	if (reg_file == AGCTAB) {
		phyreg_table = (struct PhyRegTable *)priv->pshare->agc_tab_buf;
#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8192D) {
#ifdef HIGH_POWER_EXT_PA //_eric_?? DMDP & SMSP ??
				if(priv->pshare->rf_ft_var.use_ext_pa)
				{
					//printk("[%s][AGC_TAB_n_92d_hp]\n",__FUNCTION__);
					next_head = data_AGC_TAB_n_92d_hp_start;
					read_bytes = (int)(data_AGC_TAB_n_92d_hp_end - data_AGC_TAB_n_92d_hp_start);
				}
				else
				{
					//printk("[%s][AGC_TAB_n]\n",__FUNCTION__);
					next_head = data_AGC_TAB_n_start;
					read_bytes = (int)(data_AGC_TAB_n_end - data_AGC_TAB_n_start);
				}
#else //HIGH_POWER_EXT_PA
#ifdef CONFIG_RTL_92D_DMDP
			if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY && priv->pshare->wlandev_idx==1) {
				if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
					//printk("[%s][AGC_TAB_5G_n]\n",__FUNCTION__);
					next_head = data_AGC_TAB_5G_n_start;
					read_bytes = (int)(data_AGC_TAB_5G_n_end - data_AGC_TAB_5G_n_start);
				} else {
					//printk("[%s][AGC_TAB_2G_n]\n",__FUNCTION__);
					next_head = data_AGC_TAB_2G_n_start;
					read_bytes = (int)(data_AGC_TAB_2G_n_end - data_AGC_TAB_2G_n_start);
				}
			}else
#endif
			{
				//printk("[%s][AGC_TAB_n]\n",__FUNCTION__);
				next_head = data_AGC_TAB_n_start;
				read_bytes = (int)(data_AGC_TAB_n_end - data_AGC_TAB_n_start);
			}
#endif //HIGH_POWER_EXT_PA
		}
#endif //CONFIG_RTL_92D_SUPPORT

#ifdef CONFIG_RTL_92C_SUPPORT
		if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)){
#ifdef TESTCHIP_SUPPORT
			if (IS_TEST_CHIP(priv)) {
				next_head = data_AGC_TAB_start;
				read_bytes = (int)(data_AGC_TAB_end - data_AGC_TAB_start);
			} else
#endif
			if (priv->pshare->rf_ft_var.use_ext_lna
#ifdef HIGH_POWER_EXT_PA
				|| priv->pshare->rf_ft_var.use_ext_pa
#endif
			) {
				next_head = data_AGC_TAB_n_hp_start;
				read_bytes = (int)(data_AGC_TAB_n_hp_end - data_AGC_TAB_n_hp_start);
			}
			else
			{
				next_head = data_AGC_TAB_n_92C_start;
				read_bytes = (int)(data_AGC_TAB_n_92C_end - data_AGC_TAB_n_92C_start);
			}
		}
#endif

#if !defined(CONFIG_MACBBRF_BY_ODM) && defined(CONFIG_RTL_88E_SUPPORT)
		if (GET_CHIP_VER(priv) == VERSION_8188E) {
			DEBUG_INFO("[%s][AGC_TAB_1T_88E]\n",__FUNCTION__);
			next_head = data_AGC_TAB_1T_88E_start;
			read_bytes = (int)(data_AGC_TAB_1T_88E_end - data_AGC_TAB_1T_88E_start);
		}
#endif

#ifdef CONFIG_RTL_8812_SUPPORT //8812 agc
		if (GET_CHIP_VER(priv) == VERSION_8812E) {
			if (IS_TEST_CHIP(priv)){
#ifdef HIGH_POWER_EXT_PA //FOR_8812_HP
				if( priv->pshare->rf_ft_var.use_ext_pa)
				{
					printk("[%s][AGC_TAB_8812_hp]\n",__FUNCTION__);
					next_head = data_AGC_TAB_8812_hp_start;
					read_bytes = (int)(data_AGC_TAB_8812_hp_end - data_AGC_TAB_8812_hp_start);
				}
				else
#endif
				{
					printk("[%s][AGC_TAB_8812]\n",__FUNCTION__);
					next_head = data_AGC_TAB_8812_start;
					read_bytes = (int)(data_AGC_TAB_8812_end - data_AGC_TAB_8812_start);
				}
			}
			else{ //for_8812_mp_chip
 #ifdef HIGH_POWER_EXT_PA //FOR_8812_HP
				if( priv->pshare->rf_ft_var.use_ext_pa)
				{
					printk("[%s][AGC_TAB_8812_n_hp]\n",__FUNCTION__);
					next_head = data_AGC_TAB_8812_n_hp_start;
					read_bytes = (int)(data_AGC_TAB_8812_n_hp_end - data_AGC_TAB_8812_n_hp_start);
				}
				else
#endif
				{
					printk("[%s][AGC_TAB_8812_n_default]\n",__FUNCTION__);
					next_head = data_AGC_TAB_8812_n_default_start;
					read_bytes = (int)(data_AGC_TAB_8812_n_default_end - data_AGC_TAB_8812_n_default_start);
				}

			}
		}		
		
#endif

#ifdef CONFIG_WLAN_HAL
        if (IS_HAL_CHIP(priv)) {
#ifdef HIGH_POWER_EXT_PA
            if( priv->pshare->rf_ft_var.use_ext_pa)
            {
                printk("[%s][AGC_TAB_HAL_hp]\n",__FUNCTION__);

                #if 0   // TODO: Filen, modify code below if HAL chip is exist HP issue
                GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_AGC_HP_SIZE, (pu1Byte)&read_bytes);
                GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_AGC_HP_START, &next_head);                                
                //next_head = data_AGC_TAB_8812_hp_start;
                //read_bytes = (int)(data_AGC_TAB_8812_hp_end - data_AGC_TAB_8812_hp_start);
                #endif
            }
            else
#endif
            {
                printk("[%s][AGC_TAB_HAL]\n",__FUNCTION__);
                GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_AGC_SIZE, (pu1Byte)&read_bytes);
                GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_AGC_START, &next_head);
            }
        }
#endif //CONFIG_WLAN_HAL

		max_len = AGC_TAB_SIZE;
	} else if (reg_file == PHYREG_PG) {
		//printk("[%s][PHY_REG_PG]\n",__FUNCTION__);
#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8192D){

			if(priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_FCC){
				//printk("\nFCC PG!!!\n");
				next_head = data_PHY_REG_PG_FCC_start;
				read_bytes = (int)(data_PHY_REG_PG_FCC_end - data_PHY_REG_PG_FCC_start);
			}
			else if(priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_ETSI){
				//printk("\nCE PG!!!\n");
				next_head = data_PHY_REG_PG_CE_start;
				read_bytes = (int)(data_PHY_REG_PG_CE_end - data_PHY_REG_PG_CE_start);
			}
			else{
				//printk("\nOTHER PG!!!\n");
			next_head = data_PHY_REG_PG_start;
			read_bytes = (int)(data_PHY_REG_PG_end - data_PHY_REG_PG_start);
			}

#ifdef HIGH_POWER_EXT_PA
			if( priv->pshare->rf_ft_var.use_ext_pa) {
				//printk("[%s][data_PHY_REG_PG_92d_hp]\n", __FUNCTION__);
				next_head = data_PHY_REG_PG_92d_hp_start;
				read_bytes = (int)(data_PHY_REG_PG_92d_hp_end - data_PHY_REG_PG_92d_hp_start);
			}
#endif

//_TXPWR_REDEFINE ?? Why 5G no need working channel ??
//_TXPWR_REDEFINE in MP Tool, 3 Groups: 36-99 100-148 149-165 
			if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G){
				if (priv->pshare->is_40m_bw == 0){
					if (priv->pmib->dot11RFEntry.dot11channel<=99)
						pg_tbl_idx = AN_20_CH_36_64;
					else if (priv->pmib->dot11RFEntry.dot11channel<=148)
						pg_tbl_idx = AN_20_CH_100_140;
					else
						pg_tbl_idx = AN_20_CH_149_165;
				}else{
					//_TXPWR_REDEFINE ??
					int val = priv->pmib->dot11RFEntry.dot11channel;
					
					if (priv->pshare->offset_2nd_chan == 1)
						val -= 2;
					else
						val += 2;

					if (priv->pmib->dot11RFEntry.dot11channel<=99)
						pg_tbl_idx = AN_40_CH_36_64;
					else if (priv->pmib->dot11RFEntry.dot11channel<=148)
						pg_tbl_idx = AN_40_CH_100_140;
					else
						pg_tbl_idx = AN_40_CH_149_165;
				}
			}else{
				if (priv->pshare->is_40m_bw == 0)
				{
					if (priv->pmib->dot11RFEntry.dot11channel<=3)
						pg_tbl_idx = BGN_20_CH1_3;
					else if (priv->pmib->dot11RFEntry.dot11channel<=9)
						pg_tbl_idx = BGN_20_CH4_9;
					else
						pg_tbl_idx = BGN_20_CH10_14;
				}else{
					int val = priv->pmib->dot11RFEntry.dot11channel;
					
					if (priv->pshare->offset_2nd_chan == 1)
						val -= 2;
					else
						val += 2;

					if (val<=3)
						pg_tbl_idx = BGN_40_CH1_3;
					else if (val<=9)
						pg_tbl_idx = BGN_40_CH4_9;
					else
						pg_tbl_idx = BGN_40_CH10_14;
				}
			}
#ifdef MP_TEST
			//In Noraml Driver mode, and if mib 'pwr_by_rate' = 0 >> Use default power by rate table 
			if( (priv->pshare->rf_ft_var.mp_specific == 0) && (priv->pshare->rf_ft_var.pwr_by_rate == 0) )
				pg_tbl_idx=BGN_2040_ALL;
#endif	
			DEBUG_INFO("channel=%d pg_tbl_idx=%d\n",priv->pmib->dot11RFEntry.dot11channel, pg_tbl_idx);

		}
#endif //CONFIG_RTL_92D_SUPPORT

#ifdef CONFIG_RTL_92C_SUPPORT
		if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)){
#ifdef HIGH_POWER_EXT_PA
			if( priv->pshare->rf_ft_var.use_ext_pa) {
				//printk("[%s][data_PHY_REG_PG_hp]\n", __FUNCTION__);
				next_head = data_PHY_REG_PG_hp_start;
				read_bytes = (int)(data_PHY_REG_PG_hp_end - data_PHY_REG_PG_hp_start);

			} else
#endif
			{
				//printk("[%s][data_PHY_REG_PG_92C]\n", __FUNCTION__);
				next_head = data_PHY_REG_PG_92C_start;
				read_bytes = (int)(data_PHY_REG_PG_92C_end - data_PHY_REG_PG_92C_start);
			}
		}
#endif //CONFIG_RTL_92C_SUPPORT

#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8188E) {
#ifdef SUPPORT_RTL8188E_TC
			if (IS_TEST_CHIP(priv))
				return 0;
#endif
			DEBUG_INFO("[%s][PHY_REG_PG_88E]\n",__FUNCTION__);
			next_head = data_PHY_REG_PG_88E_start;
			read_bytes = (int)(data_PHY_REG_PG_88E_end - data_PHY_REG_PG_88E_start);

			/* In Noraml Driver mode, and if mib 'pwr_by_rate' = 0 >> Use default power by rate table  */
			if (
#ifdef MP_TEST
				priv->pshare->rf_ft_var.mp_specific || 
#endif
				priv->pshare->rf_ft_var.pwr_by_rate) {
				if (priv->pshare->is_40m_bw == 0) {
					if (priv->pmib->dot11RFEntry.dot11channel<=3)
						pg_tbl_idx = BGN_20_CH1_3;
					else if (priv->pmib->dot11RFEntry.dot11channel<=9)
						pg_tbl_idx = BGN_20_CH4_9;
					else
						pg_tbl_idx = BGN_20_CH10_14;
				} else {
					int val = priv->pmib->dot11RFEntry.dot11channel;
					
					if (priv->pshare->offset_2nd_chan == 1)
						val -= 2;
					else
						val += 2;

					if (val<=3)
						pg_tbl_idx = BGN_40_CH1_3;
					else if (val<=9)
						pg_tbl_idx = BGN_40_CH4_9;
					else
						pg_tbl_idx = BGN_40_CH10_14;
				}
			}

#ifdef MP_TEST
			if(priv->pshare->rf_ft_var.mp_specific)
				pg_tbl_idx = 0;
#endif

			DEBUG_INFO("channel=%d pg_tbl_idx=%d\n",priv->pmib->dot11RFEntry.dot11channel, pg_tbl_idx);
		}
#endif

#ifdef CONFIG_RTL_8812_SUPPORT //eric_8812 pg
		if (GET_CHIP_VER(priv) == VERSION_8812E) {

			next_head = data_PHY_REG_PG_8812_start;
			read_bytes = (int)(data_PHY_REG_PG_8812_end - data_PHY_REG_PG_8812_start);

			/* In Noraml Driver mode, and if mib 'pwr_by_rate' = 0 >> Use default power by rate table  */
			if (
#ifdef MP_TEST
				priv->pshare->rf_ft_var.mp_specific || 
#endif
				priv->pshare->rf_ft_var.pwr_by_rate) {
				if (priv->pshare->is_40m_bw == 0) {
					if (priv->pmib->dot11RFEntry.dot11channel<=3)
						pg_tbl_idx = BGN_20_CH1_3;
					else if (priv->pmib->dot11RFEntry.dot11channel<=9)
						pg_tbl_idx = BGN_20_CH4_9;
					else
						pg_tbl_idx = BGN_20_CH10_14;
				} else {
					int val = priv->pmib->dot11RFEntry.dot11channel;
					
					if (priv->pshare->offset_2nd_chan == 1)
						val -= 2;
					else
						val += 2;

					if (val<=3)
						pg_tbl_idx = BGN_40_CH1_3;
					else if (val<=9)
						pg_tbl_idx = BGN_40_CH4_9;
					else
						pg_tbl_idx = BGN_40_CH10_14;
				}
			}

#ifdef MP_TEST
			if(priv->pshare->rf_ft_var.mp_specific)
				pg_tbl_idx = 0;
#endif

			DEBUG_INFO("channel=%d pg_tbl_idx=%d\n",priv->pmib->dot11RFEntry.dot11channel, pg_tbl_idx);
		}	
#endif //CONFIG_RTL_8812_SUPPORT

#ifdef CONFIG_WLAN_HAL
                if ( IS_HAL_CHIP(priv) ) {
                    printk("[%s][PHY_REG_PG_HAL]\n",__FUNCTION__);
                    GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_PG_SIZE, (pu1Byte)&read_bytes);
                    GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_PG_START, &next_head);
        
                    /* In Noraml Driver mode, and if mib 'pwr_by_rate' = 0 >> Use default power by rate table  */
                    if (
#ifdef MP_TEST
                        priv->pshare->rf_ft_var.mp_specific || 
#endif
                        priv->pshare->rf_ft_var.pwr_by_rate) {
                        if (priv->pshare->is_40m_bw == 0) {
                            if (priv->pmib->dot11RFEntry.dot11channel<=3)
                                pg_tbl_idx = BGN_20_CH1_3;
                            else if (priv->pmib->dot11RFEntry.dot11channel<=9)
                                pg_tbl_idx = BGN_20_CH4_9;
                            else
                                pg_tbl_idx = BGN_20_CH10_14;
                        } else {
                            int val = priv->pmib->dot11RFEntry.dot11channel;
                            
                            if (priv->pshare->offset_2nd_chan == 1)
                                val -= 2;
                            else
                                val += 2;
        
                            if (val<=3)
                                pg_tbl_idx = BGN_40_CH1_3;
                            else if (val<=9)
                                pg_tbl_idx = BGN_40_CH4_9;
                            else
                                pg_tbl_idx = BGN_40_CH10_14;
                        }
                    }
        
#ifdef MP_TEST
                    if(priv->pshare->rf_ft_var.mp_specific)
                        pg_tbl_idx = 0;
#endif
        
                    DEBUG_INFO("channel=%d pg_tbl_idx=%d\n",priv->pmib->dot11RFEntry.dot11channel, pg_tbl_idx);
                }   
#endif //CONFIG_WLAN_HAL

		macreg_table = (struct MacRegTable *)priv->pshare->phy_reg_pg_buf;
		max_len = PHY_REG_PG_SIZE;
		file_format = THREE_COLUMN;
	}
#if 0
	else if (reg_file == PHYREG_1T2R) {
		macreg_table = (struct MacRegTable *)priv->pshare->phy_reg_2to1;
		max_len = PHY_REG_1T2R;
		file_format = THREE_COLUMN;
		if (priv->pshare->rf_ft_var.pathB_1T == 0) { // PATH A
			next_head = __PHY_to1T2R_start;
			read_bytes = (int)(__PHY_to1T2R_end - __PHY_to1T2R_start);
		}
		else { // PATH B
			next_head = __PHY_to1T2R_b_start;
			read_bytes = (int)(__PHY_to1T2R_b_end - __PHY_to1T2R_b_start);
		}
	}
#endif
#if defined(CONFIG_RTL_92D_SUPPORT) || defined (CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
	else if (reg_file == PHYREG) {
#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8192D) {
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_buf;
#ifdef HIGH_POWER_EXT_PA
			//printk("[%s][PHY_REG_n_92d_hp]\n",__FUNCTION__);
			next_head = data_PHY_REG_n_92d_hp_start;
			read_bytes = (int)(data_PHY_REG_n_92d_hp_end - data_PHY_REG_n_92d_hp_start);
#else
			//printk("[%s][PHY_REG_n]\n",__FUNCTION__);
			next_head = data_PHY_REG_n_start;
			read_bytes = (int)(data_PHY_REG_n_end - data_PHY_REG_n_start);
#endif
			max_len = PHY_REG_SIZE;
		}
#endif
#ifdef CONFIG_RTL_8812_SUPPORT //8812 phy
		if (GET_CHIP_VER(priv)==VERSION_8812E) {
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_buf;
			if (IS_TEST_CHIP(priv)){ //for_8812_mp_chip
				printk("[%s][PHY_REG_8812]\n",__FUNCTION__);
				next_head = data_PHY_REG_8812_start;
				read_bytes = (int)(data_PHY_REG_8812_end - data_PHY_REG_8812_start);
			}
			else{
#ifdef HIGH_POWER_EXT_PA //FOR_8812_HP
				if( priv->pshare->rf_ft_var.use_ext_pa){
					 printk("[%s][PHY_REG_8812_n_hp]\n",__FUNCTION__);
					 next_head = data_PHY_REG_8812_n_hp_start;
					 read_bytes = (int)(data_PHY_REG_8812_n_hp_end - data_PHY_REG_8812_n_hp_start);
				}
				else
#endif
				{
					printk("[%s][PHY_REG_8812_n_default]\n",__FUNCTION__);
					next_head = data_PHY_REG_8812_n_default_start;
					read_bytes = (int)(data_PHY_REG_8812_n_default_end - data_PHY_REG_8812_n_default_start);
				}
			}
			max_len = PHY_REG_SIZE;
		}
#endif

#ifdef CONFIG_WLAN_HAL
        if ( IS_HAL_CHIP(priv) ) {
            phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_buf;
			printk("[%s][PHY_REG_HAL]\n",__FUNCTION__);            
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_SIZE, (pu1Byte)&read_bytes);
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_START, &next_head);
            max_len = PHY_REG_SIZE;                               
        }
#endif //CONFIG_WLAN_HAL        

	}
#endif // CONFIG_RTL_92D_SUPPORT
#ifdef MP_TEST
	else if (reg_file == PHYREG_MP) {
#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8192D) {
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_mp_buf;
			//printk("[%s][PHY_REG_MP_n]\n",__FUNCTION__);
			next_head = data_PHY_REG_MP_n_start;
			read_bytes = (int)(data_PHY_REG_MP_n_end - data_PHY_REG_MP_n_start);
			max_len = PHY_REG_SIZE;
		}
#endif
#ifdef CONFIG_RTL_92C_SUPPORT
		if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)){
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_mp_buf;
			next_head = data_PHY_REG_MP_n_92C_start;
			read_bytes = (int)(data_PHY_REG_MP_n_92C_end - data_PHY_REG_MP_n_92C_start);
			max_len = PHY_REG_SIZE;
		}
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8188E){
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_mp_buf;
			next_head = data_PHY_REG_MP_88E_start;
			read_bytes = (int)(data_PHY_REG_MP_88E_end - data_PHY_REG_MP_88E_start);
			max_len = PHY_REG_SIZE;
		}
#endif

#ifdef CONFIG_RTL_8812_SUPPORT //8812 phy mp
		if (GET_CHIP_VER(priv) == VERSION_8812E){
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_mp_buf;
			printk("[%s][PHY_REG_MP_8812]\n",__FUNCTION__);
			next_head = data_PHY_REG_MP_8812_start;
			read_bytes = (int)(data_PHY_REG_MP_8812_end - data_PHY_REG_MP_8812_start);
			max_len = PHY_REG_SIZE;
		}
#endif

#ifdef CONFIG_WLAN_HAL
        if ( IS_HAL_CHIP(priv) ) {
            phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_mp_buf;
            printk("[%s][PHY_REG_MP_HAL]\n",__FUNCTION__);
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_MP_SIZE, (pu1Byte)&read_bytes);
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_MP_START, &next_head);
            max_len = PHY_REG_SIZE;                               
        }
#endif //CONFIG_WLAN_HAL 

	}
#endif
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_WLAN_HAL)
	else if (reg_file == PHYREG_1T1R) { // PATH A
		phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_buf;
#ifdef CONFIG_RTL_92C_SUPPORT
		if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)) {
#ifdef TESTCHIP_SUPPORT
			if( IS_TEST_CHIP(priv) ) {
				next_head = data_PHY_REG_1T_start;
				read_bytes = (int)(data_PHY_REG_1T_end - data_PHY_REG_1T_start);
			} else
#endif
			{

				if (priv->pshare->rf_ft_var.use_ext_lna
#ifdef HIGH_POWER_EXT_PA
				|| priv->pshare->rf_ft_var.use_ext_pa
#endif
				) {
					//printk("[%s][PHY_REG_1T_n_hp]\n", __FUNCTION__);
					next_head = data_PHY_REG_1T_n_hp_start;
					read_bytes = (int)(data_PHY_REG_1T_n_hp_end - data_PHY_REG_1T_n_hp_start);
				} else {
					//printk("[%s][PHY_REG_1T_n]\n", __FUNCTION__);
					next_head = data_PHY_REG_1T_n_start;
					read_bytes = (int)(data_PHY_REG_1T_n_end - data_PHY_REG_1T_n_start);
				}
			}
		}
#endif
#if !defined(CONFIG_MACBBRF_BY_ODM) && defined(CONFIG_RTL_88E_SUPPORT)	
		if (GET_CHIP_VER(priv) == VERSION_8188E) {
#ifdef SUPPORT_RTL8188E_TC
			if (IS_TEST_CHIP(priv)) {
				DEBUG_INFO("[%s][PHY_REG_1T_88E_TC]\n",__FUNCTION__);
				next_head = data_PHY_REG_1T_88E_TC_start;
				read_bytes = (int)(data_PHY_REG_1T_88E_TC_end - data_PHY_REG_1T_88E_TC_start);
			} else
#endif
			{
				DEBUG_INFO("[%s][PHY_REG_1T_88E]\n",__FUNCTION__);
				next_head = data_PHY_REG_1T_88E_start;
				read_bytes = (int)(data_PHY_REG_1T_88E_end - data_PHY_REG_1T_88E_start);
			}
		}
#endif

#ifdef CONFIG_WLAN_HAL
        if ( IS_HAL_CHIP(priv) ) {
            printk("[%s][PHY_REG_1T_HAL]\n",__FUNCTION__);            
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_1T_SIZE, (pu1Byte)&read_bytes);
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_1T_START, &next_head);
        }
#endif //CONFIG_WLAN_HAL

		max_len = PHY_REG_SIZE;
#if 0
		if(priv->pshare->rf_ft_var.pathB_1T == 0) {
			next_head = __PHY_to1T1R_start;
			read_bytes = (int)(__PHY_to1T1R_end - __PHY_to1T1R_start);
		}
		else { // PATH B
			next_head = __PHY_to1T1R_b_start;
			read_bytes = (int)(__PHY_to1T1R_b_end - __PHY_to1T1R_b_start);
		}
#endif
	}
#endif
#ifdef CONFIG_RTL_92C_SUPPORT
	else if (reg_file == PHYREG_2T2R) {
#ifdef TESTCHIP_SUPPORT
		if (IS_TEST_CHIP(priv)) {
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_buf;
			next_head = data_PHY_REG_2T_start;
			read_bytes = (int)(data_PHY_REG_2T_end - data_PHY_REG_2T_start);
		} else
#endif
		{
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_buf;
			if (priv->pshare->rf_ft_var.use_ext_lna
#ifdef HIGH_POWER_EXT_PA
			|| priv->pshare->rf_ft_var.use_ext_pa
#endif
			) {
				//printk("[%s][PHY_REG_2T_n_hp]\n", __FUNCTION__);
				next_head = data_PHY_REG_2T_n_hp_start;
				read_bytes = (int)(data_PHY_REG_2T_n_hp_end - data_PHY_REG_2T_n_hp_start);
			}
			else
			{
				//printk("[%s][PHY_REG_2T_n]\n", __FUNCTION__);
				next_head = data_PHY_REG_2T_n_start;
				read_bytes = (int)(data_PHY_REG_2T_n_end - data_PHY_REG_2T_n_start);
			}
		}
		max_len = PHY_REG_SIZE;
	}
#endif

	{
#ifdef __KERNEL__
		if((mem_ptr = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
			printk("PHY_ConfigBBWithParaFile(): not enough memory\n");
			return -1;
		}
#endif
#ifdef __ECOS
		mem_ptr = config_buf;
#endif

		memset(mem_ptr, 0, MAX_CONFIG_FILE_SIZE); // clear memory
		memcpy(mem_ptr, next_head, read_bytes);

		next_head = mem_ptr;
		while (1) {
			line_head = next_head;
			next_head = get_line(&line_head);
			if (line_head == NULL)
				break;

			if (line_head[0] == '/')
				continue;

			if (file_format == TWO_COLUMN) {
				num = get_offset_val(line_head, &u4bRegOffset, &u4bRegValue);
				if (num > 0) {
					phyreg_table[len].offset = u4bRegOffset;
					phyreg_table[len].value = u4bRegValue;
					len++;

#if defined(CONFIG_RTL865X_WTDOG) || defined(CONFIG_RTL_WTDOG)
#if (defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)) && defined(CONFIG_RTL_92D_SUPPORT)
					if ((len&0x7ff)==0)
						REG32(BSP_WDTCNR) |=  1 << 23;
#elif defined (CONFIG_RTL_8198B) && defined(CONFIG_RTL_92D_SUPPORT)
					if ((len&0x7ff)==0)
						REG32(BSP_WDTCNTRR) |= BSP_WDT_KICK;
#endif
#endif

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT)|| defined(CONFIG_WLAN_HAL) //_eric_8812 ?? u4bRegOffset
					if ((GET_CHIP_VER(priv)==VERSION_8188E) || (GET_CHIP_VER(priv)==VERSION_8812E) || (IS_HAL_CHIP(priv))) {
						if (u4bRegOffset == 0xffff)
							break;
					} else
#endif
					{
						if (u4bRegOffset == 0xff)
							break;
					}
					if ((len * sizeof(struct PhyRegTable)) > max_len)
						break;
				}
			} else {
				num = get_offset_mask_val(line_head, &u4bRegOffset, &u4bRegMask ,&u4bRegValue);
				if (num > 0) {
					macreg_table[len].offset = u4bRegOffset;
					macreg_table[len].mask = u4bRegMask;
					macreg_table[len].value = u4bRegValue;
					len++;
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
					if ((GET_CHIP_VER(priv)==VERSION_8188E) || (GET_CHIP_VER(priv)==VERSION_8812E) || (IS_HAL_CHIP(priv))) {
						if (u4bRegOffset == 0xffff)
							break;
					} else
#endif
					{
						if (u4bRegOffset == 0xff)
							break;
					}
					if ((len * sizeof(struct MacRegTable)) > max_len)
						break;
#if defined(CONFIG_RTL865X_WTDOG) || defined(CONFIG_RTL_WTDOG)
#if (defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)) && defined(CONFIG_RTL_92D_SUPPORT)
					if ((len&0x7ff)==0)
						REG32(BSP_WDTCNR) |=  1 << 23;
#elif defined (CONFIG_RTL_8198B) && defined(CONFIG_RTL_92D_SUPPORT)
					if ((len&0x7ff)==0)
						REG32(BSP_WDTCNTRR) |= BSP_WDT_KICK;
#endif
#endif
				}
			}
		}
#ifdef __KERNEL__
		kfree(mem_ptr);
#endif
		if ((len * sizeof(struct PhyRegTable)) > max_len) {
			printk("PHY REG table buffer not large enough!\n");
			return -1;
		}
	}

	num = 0;
	while (1) {
		if (file_format == THREE_COLUMN) {
			u4bRegOffset = macreg_table[num].offset;
			u4bRegValue = macreg_table[num].value;
			u4bRegMask = macreg_table[num].mask;
		}
		else {
			u4bRegOffset = phyreg_table[num].offset;
			u4bRegValue = phyreg_table[num].value;
		}

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
		if ((GET_CHIP_VER(priv)==VERSION_8188E) || (GET_CHIP_VER(priv)==VERSION_8812E) || (IS_HAL_CHIP(priv))) {
			if (u4bRegOffset == 0xffff)
				break;
		} else
#endif
		{
			if (u4bRegOffset == 0xff)
				break;
		}

		if (file_format == THREE_COLUMN){
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
			if (reg_file == PHYREG_PG && (
#ifdef CONFIG_RTL_92D_SUPPORT
				(GET_CHIP_VER(priv)==VERSION_8192D) 
#endif
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
#ifdef CONFIG_RTL_92D_SUPPORT
				|| 
#endif
				(GET_CHIP_VER(priv)==VERSION_8188E) || (GET_CHIP_VER(priv)==VERSION_8812E) || (IS_HAL_CHIP(priv))
#endif
				)) {
				if (u4bRegOffset==0xe00){
					if (idx == pg_tbl_idx)
						write_en=1;
					idx++;
				}
				if (write_en){
					PHY_SetBBReg(priv, u4bRegOffset, u4bRegMask, u4bRegValue);
					if (u4bRegOffset==0x868){
						write_en = 0;
						break;
					}
				}
			}else
#endif
			{
				PHY_SetBBReg(priv, u4bRegOffset, u4bRegMask, u4bRegValue);
			}
		}
		else
			PHY_SetBBReg(priv, u4bRegOffset, bMaskDWord, u4bRegValue);
		num++;
	}

	return 0;
}


/*-----------------------------------------------------------------------------
 * Function:    PHY_ConfigRFWithParaFile()
 *
 * Overview:    This function read RF parameters from general file format, and do RF 3-wire
 *
 * Input:      	PADAPTER			Adapter
 *			ps1Byte 				pFileName
 *			RF92CD_RADIO_PATH_E	eRFPath
 *
 * Output:      NONE
 *
 * Return:      RT_STATUS_SUCCESS: configuration file exist
 *
 * Note:		Delay may be required for RF configuration
 *---------------------------------------------------------------------------*/
int PHY_ConfigRFWithParaFile(struct rtl8192cd_priv *priv,
				unsigned char *start, int read_bytes,
				RF92CD_RADIO_PATH_E eRFPath)
{
	int           num;
	unsigned int  u4bRegOffset, u4bRegValue;
	unsigned char *mem_ptr, *line_head, *next_head;

#ifdef __KERNEL__
	if ((mem_ptr = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
		printk("PHY_ConfigRFWithParaFile(): not enough memory\n");
		return -1;
	}
#endif
#ifdef __ECOS
	mem_ptr = config_buf;
#endif
	memset(mem_ptr, 0, MAX_CONFIG_FILE_SIZE); // clear memory
	memcpy(mem_ptr, start, read_bytes);

	next_head = mem_ptr;
	while (1) {
		line_head = next_head;
		next_head = get_line(&line_head);
		if (line_head == NULL)
			break;

		if (line_head[0] == '/')
			continue;

		num = get_offset_val(line_head, &u4bRegOffset, &u4bRegValue);
		if (num > 0) {
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL) //_eric_8812 ?? rf paras ??
			if ((GET_CHIP_VER(priv)==VERSION_8188E) || (GET_CHIP_VER(priv)==VERSION_8812E) || (IS_HAL_CHIP(priv))) {
				if (u4bRegOffset == 0xffff) {
					break;
				} else if ((u4bRegOffset == 0xfe) || (u4bRegOffset == 0xffe)) {
					delay_ms(50);	// Delay 50 ms. Only RF configuration require delay
				} else if (num == 2) {
					PHY_SetRFReg(priv, eRFPath, u4bRegOffset, bMask20Bits, u4bRegValue);
					delay_ms(1);
				}
			} else
#endif
			{
				if (u4bRegOffset == 0xff) {
					break;
				} else if (u4bRegOffset == 0xfe) {
					delay_ms(50);	// Delay 50 ms. Only RF configuration require delay
				} else if (num == 2) {
					PHY_SetRFReg(priv, eRFPath, u4bRegOffset, bMask20Bits, u4bRegValue);
					delay_ms(1);
				}
			}
		}
#if defined(CONFIG_RTL865X_WTDOG) || defined(CONFIG_RTL_WTDOG)
#if (defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)) && defined(CONFIG_RTL_92D_SUPPORT)
		REG32(BSP_WDTCNR) |=  1 << 23;
#elif defined (CONFIG_RTL_8198B) && defined(CONFIG_RTL_92D_SUPPORT)
		REG32(BSP_WDTCNTRR) |= BSP_WDT_KICK;
#endif
#endif
	}
#ifdef __KERNEL__
	kfree(mem_ptr);
#endif
	return 0;
}

//#if !defined(CONFIG_MACBBRF_BY_ODM) || !defined(CONFIG_RTL_88E_SUPPORT)	
#if 1
int PHY_ConfigMACWithParaFile(struct rtl8192cd_priv *priv)
{
	int read_bytes, num, len=0;
	unsigned int  u4bRegOffset, u4bRegValue;
	unsigned char *mem_ptr, *line_head, *next_head;
	struct PhyRegTable *reg_table = (struct PhyRegTable *)priv->pshare->mac_reg_buf;

	{
#ifdef __KERNEL__
		if((mem_ptr = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
			printk("PHY_ConfigMACWithParaFile(): not enough memory\n");
			return -1;
		}
#endif
#ifdef __ECOS
		mem_ptr = config_buf;
#endif
		memset(mem_ptr, 0, MAX_CONFIG_FILE_SIZE); // clear memory
#ifdef CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {
            u8 *pMACRegStart;
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_MACREGFILE_SIZE, (pu1Byte)&read_bytes);
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_MACREGFILE_START,(pu1Byte)&pMACRegStart);
            memcpy(mem_ptr, pMACRegStart, read_bytes);
        } 
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8192D) {
			printk("[%s][MACPHY_REG]\n",__FUNCTION__);
			read_bytes = (int)(data_MACPHY_REG_end - data_MACPHY_REG_start);
			memcpy(mem_ptr, data_MACPHY_REG_start, read_bytes);
		}
#endif
#ifdef CONFIG_RTL_92C_SUPPORT
		if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)){
			printk("[%s][MACPHY_REG_92C]\n",__FUNCTION__);
			read_bytes = (int)(data_MACPHY_REG_92C_end - data_MACPHY_REG_92C_start);
			memcpy(mem_ptr, data_MACPHY_REG_92C_start, read_bytes);
		}
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8188E) {
#ifdef SUPPORT_RTL8188E_TC
			if (IS_TEST_CHIP(priv)) {
				DEBUG_INFO("[%s][MAC_REG_88E_TC]\n",__FUNCTION__);
				read_bytes = (int)(data_MAC_REG_88E_TC_end - data_MAC_REG_88E_TC_start);
				memcpy(mem_ptr, data_MAC_REG_88E_TC_start, read_bytes);
			} else
#endif
			{
				DEBUG_INFO("[%s][MAC_REG_88E]\n",__FUNCTION__);
				read_bytes = (int)(data_MAC_REG_88E_end - data_MAC_REG_88E_start);
				memcpy(mem_ptr, data_MAC_REG_88E_start, read_bytes);
			}
		}
#endif

#ifdef CONFIG_RTL_8812_SUPPORT //mac reg
		if (GET_CHIP_VER(priv) == VERSION_8812E) {
			if (IS_TEST_CHIP(priv)) {
				printk("[%s][MAC_REG_8812]\n",__FUNCTION__);
			read_bytes = (int)(data_MAC_REG_8812_end - data_MAC_REG_8812_start);
			memcpy(mem_ptr, data_MAC_REG_8812_start, read_bytes);
			} else {
				printk("[%s][MAC_REG_8812_n]\n",__FUNCTION__);
				read_bytes = (int)(data_MAC_REG_8812_n_end - data_MAC_REG_8812_n_start);
				memcpy(mem_ptr, data_MAC_REG_8812_n_start, read_bytes);
			}
		}
#endif
		next_head = mem_ptr;
		while (1) {
			line_head = next_head;
			next_head = get_line(&line_head);
			if (line_head == NULL)
				break;

			if (line_head[0] == '/')
				continue;

			num = get_offset_val(line_head, &u4bRegOffset, &u4bRegValue);
			if (num > 0) {
				reg_table[len].offset = u4bRegOffset;
				reg_table[len].value = u4bRegValue;
				len++;
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
				if ((GET_CHIP_VER(priv)==VERSION_8188E) || (GET_CHIP_VER(priv)==VERSION_8812E) || (IS_HAL_CHIP(priv))) {
					if (u4bRegOffset == 0xffff)
						break;
				} else
#endif
				{
					if (u4bRegOffset == 0xff)
						break;
				}
				if ((len * sizeof(struct MacRegTable)) > MAC_REG_SIZE)
					break;
			}
		}
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
		if ((GET_CHIP_VER(priv)==VERSION_8188E) || (GET_CHIP_VER(priv)==VERSION_8812E) || (IS_HAL_CHIP(priv)))
			reg_table[len].offset = 0xffff;
		else
#endif
			reg_table[len].offset = 0xff;
#ifdef __KERNEL__
		kfree(mem_ptr);
#endif
		if ((len * sizeof(struct MacRegTable)) > MAC_REG_SIZE) {
			printk("MAC REG table buffer not large enough!\n");
			return -1;
		}
	}

	num = 0;
	while (1) {
		u4bRegOffset = reg_table[num].offset;
		u4bRegValue = reg_table[num].value;

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
		if ((GET_CHIP_VER(priv)==VERSION_8188E) || (GET_CHIP_VER(priv)==VERSION_8812E) || (IS_HAL_CHIP(priv))) {
			if (u4bRegOffset == 0xffff)
				break;
		} else
#endif
		{
			if (u4bRegOffset == 0xff)
				break;
		}

		RTL_W8(u4bRegOffset, u4bRegValue);
		num++;
	}

	return 0;
}
#endif

#ifdef UNIVERSAL_REPEATER
static struct rtl8192cd_priv *get_another_interface_priv(struct rtl8192cd_priv *priv)
{
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv)))
		return GET_VXD_PRIV(priv);
	else if (IS_DRV_OPEN(GET_ROOT_PRIV(priv)))
		return GET_ROOT_PRIV(priv);
	else
		return NULL;
}


static int get_shortslot_for_another_interface(struct rtl8192cd_priv *priv)
{
	struct rtl8192cd_priv *p_priv;

	p_priv = get_another_interface_priv(priv);
	if (p_priv) {
		if (p_priv->pmib->dot11OperationEntry.opmode & WIFI_AP_STATE)
			return (p_priv->pmib->dot11ErpInfo.shortSlot);
		else {
			if (p_priv->pmib->dot11OperationEntry.opmode & WIFI_ASOC_STATE)
				return (p_priv->pmib->dot11ErpInfo.shortSlot);
		}
	}
	return -1;
}
#endif // UNIVERSAL_REPEATER


void set_slot_time(struct rtl8192cd_priv *priv, int use_short)
{
#ifdef UNIVERSAL_REPEATER
	int is_short;
	is_short = get_shortslot_for_another_interface(priv);
	if (is_short != -1) { // not abtained
		use_short &= is_short;
	}
#endif

	if (use_short)
		RTL_W8(SLOT_TIME, 0x09);
	else
		RTL_W8(SLOT_TIME, 0x14);
}

void SwChnl(struct rtl8192cd_priv *priv, unsigned char channel, int offset)
{
	unsigned int val = channel, eRFPath, curMaxRFPath;

#if defined(CONFIG_RTL_8812_SUPPORT) 
	if (GET_CHIP_VER(priv) == VERSION_8812E) {
		priv->pshare->No_RF_Write = 0;
		UpdateBBRFVal8812(priv, channel);
	}
#endif


#if defined(CONFIG_WLAN_HAL_8881A)
	if (GET_CHIP_VER(priv) == VERSION_8881A) {
		priv->pshare->No_RF_Write = 0;
        //UpdateBBRFVal8812(priv, channel);
       GET_HAL_INTERFACE(priv)->PHYUpdateBBRFValHandler(priv, channel);
	}
#endif


#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8192D){
#ifdef MP_TEST		
		if( (priv->pshare->rf_ft_var.mp_specific == 0) || (priv->pshare->rf_ft_var.pwr_by_rate == 1) )
			reload_txpwr_pg(priv);
#endif			
	}
#endif

#ifdef CONFIG_RTL_92D_DMDP
	if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY)
		curMaxRFPath = RF92CD_PATH_B;
	else
#endif
		curMaxRFPath = RF92CD_PATH_MAX;

	if (channel > 14)
		priv->pshare->curr_band = BAND_5G;
	else
		priv->pshare->curr_band = BAND_2G;

//_TXPWR_REDEFINE ?? Working channel also apply to 5G ?? what if channel = 165 + 2 or 36 -2 ??
#ifdef RTK_AC_SUPPORT //todo, find working channel for 80M
	if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_80)
	{
		if(channel <= 48)
			val = 42;
		else if(channel <= 64)
			val = 58;
		else if(channel <= 112)
			val = 106;
		else if(channel <= 128)
			val = 122;
		else if(channel <= 144)
			val = 138;
		else if(channel <= 161)
			val = 155;
		else if(channel <= 177)
			val = 171;	
	}
	else
#endif
	if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20_40) {
		if (offset == 1)
			val -= 2;
		else
			val += 2;
	}

#ifdef CONFIG_RTL_92D_SUPPORT
	if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G)
#endif
		if (priv->pshare->rf_ft_var.use_frq_2_3G)
			val += 14;

	for(eRFPath = RF92CD_PATH_A; eRFPath < curMaxRFPath; eRFPath++)	{

#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8192D) {
			priv->pshare->RegRF18[eRFPath] = RTL_SET_MASK(priv->pshare->RegRF18[eRFPath],0xff,val,0);
			//PHY_SetRFReg(priv, eRFPath, rRfChannel, 0xff, val);
			if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
				/*
				 *	Set Bit18 when channel >= 100, for 5G only
				 */
				if (val >= 100)
					priv->pshare->RegRF18[eRFPath] |= BIT(18);
					//PHY_SetRFReg(priv, eRFPath, rRfChannel, BIT(18), 1);
				else
					priv->pshare->RegRF18[eRFPath] &= (~(BIT(18)));
					//PHY_SetRFReg(priv, eRFPath, rRfChannel, BIT(18), 0);

				priv->pshare->RegRF18[eRFPath] |= BIT(16);
				//PHY_SetRFReg(priv, eRFPath, rRfChannel, BIT(16), 1);
				priv->pshare->RegRF18[eRFPath] |= BIT(8);
				//PHY_SetRFReg(priv, eRFPath, rRfChannel, BIT(8), 1);
				// CLOAD for RF paht_A/B (MP-chip)
				if (val < 149)
					PHY_SetRFReg(priv, eRFPath, 0xB, BIT(16)|BIT(15)|BIT(14), 0x7);
				else
					PHY_SetRFReg(priv, eRFPath, 0xB, BIT(16)|BIT(15)|BIT(14), 0x2);
			} else {
				priv->pshare->RegRF18[eRFPath] &= (~(BIT(18)));
				//PHY_SetRFReg(priv, eRFPath, rRfChannel, BIT(18, 0);
				priv->pshare->RegRF18[eRFPath] &= (~(BIT(16)));
				//PHY_SetRFReg(priv, eRFPath, rRfChannel, BIT(16), 0);
				priv->pshare->RegRF18[eRFPath] &= (~(BIT(8)));
				//PHY_SetRFReg(priv, eRFPath, rRfChannel, BIT(8), 0);
				// CLOAD for RF paht_A/B (MP-chip)
				PHY_SetRFReg(priv, eRFPath, 0xB, BIT(16)|BIT(15)|BIT(14), 0x7);
			}

			PHY_SetRFReg(priv,eRFPath,rRfChannel,bMask20Bits,priv->pshare->RegRF18[eRFPath]);
			//printk("%s(%d) RF 18 = 0x%05x[0x%05x]\n",__FUNCTION__,__LINE__,priv->pshare->RegRF18[eRFPath],
				//PHY_QueryRFReg(priv,eRFPath,rRfChannel,bMask20Bits,1));
#ifdef RX_GAIN_TRACK_92D
			priv->pshare->RegRF3C[eRFPath] = PHY_QueryRFReg(priv, eRFPath, 0x3C, bMask20Bits, 1);
#endif
	    }else 
#endif
		{
			PHY_SetRFReg(priv, eRFPath, rRfChannel, 0xff, val);
		}

	}

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8192D) {
		SetSYN_para(priv, val);
#ifdef SW_LCK_92D
		phy_ReloadLCKSetting(priv);
#endif
		SetIMR_n(priv, val);

		Update92DRFbyChannel(priv, val);

	}
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G)
#endif
		if (priv->pshare->rf_ft_var.use_frq_2_3G)
			val -= 14;

	priv->pshare->working_channel = val;

#ifdef TXPWR_LMT
{	
	if (!priv->pshare->rf_ft_var.disable_txpwrlmt){
		find_pwr_limit(priv);
	}
}
#endif

#if defined(CONFIG_RTL_8812_SUPPORT) 
	if (GET_CHIP_VER(priv) == VERSION_8812E) {
		PHY_SetOFDMTxPower_8812(priv, val);
			
		if (priv->pshare->curr_band == BAND_2G)
		{
			PHY_SetCCKTxPower_8812(priv, val);
		}
		priv->pshare->No_RF_Write = 1;
		return;
	}
#endif

#if defined(CONFIG_WLAN_HAL_8881A)
if (GET_CHIP_VER(priv) == VERSION_8881A) {        
        GET_HAL_INTERFACE(priv)->PHYSetOFDMTxPowerHandler(priv, val);
			
		if (priv->pshare->curr_band == BAND_2G)
		{
			GET_HAL_INTERFACE(priv)->PHYSetCCKTxPowerHandler(priv, val);
		}
		priv->pshare->No_RF_Write = 1;
		return;
	}
#endif //#if defined(CONFIG_WLAN_HAL_8881A)


#if defined(CONFIG_WLAN_HAL_8192EE)
    if (GET_CHIP_VER(priv) == VERSION_8192E) {        
            GET_HAL_INTERFACE(priv)->PHYSetOFDMTxPowerHandler(priv, val);
                
            if (priv->pshare->curr_band == BAND_2G)
            {
                GET_HAL_INTERFACE(priv)->PHYSetCCKTxPowerHandler(priv, val);
            }
            selectMinPowerIdex(priv);
            return;
        }
#endif //#if defined(CONFIG_WLAN_HAL_8192EE)

	PHY_RF6052SetOFDMTxPower(priv, val);
	if (priv->pshare->curr_band == BAND_2G)
		PHY_RF6052SetCCKTxPower(priv, val);

	selectMinPowerIdex(priv);

	return;
}

#if 0
// switch 1 spatial stream path
//antPath: 01 for PathA,10 for PathB, 11for Path AB
void Switch_1SS_Antenna(struct rtl8192cd_priv *priv, unsigned int antPath )
{
	unsigned int dword = 0;
	if(get_rf_mimo_mode(priv) != MIMO_2T2R)
		return;

	switch(antPath){
	case 1:
		dword = RTL_R32(0x90C);
		if((dword & 0x0ff00000) == 0x01100000)
			goto switch_1ss_end;
		dword &= 0xf00fffff;
		dword |= 0x01100000; // Path A
		RTL_W32(0x90C, dword);
		break;
	case 2:
		dword = RTL_R32(0x90C);
		if((dword & 0x0ff00000) == 0x02200000)
			goto switch_1ss_end;
		dword &= 0xf00fffff;
		dword |= 0x02200000;	// Path B
		RTL_W32(0x90C, dword);
		break;

	case 3:
		if(priv->pshare->rf_ft_var.ofdm_1ss_oneAnt == 1)// use one ANT for 1ss
			goto switch_1ss_end;// do nothing
		dword = RTL_R32(0x90C);
		if((dword & 0x0ff00000) == 0x03300000)
			goto switch_1ss_end;
		dword &= 0xf00fffff;
		dword |= 0x03300000; // Path A,B
		RTL_W32(0x90C, dword);
		break;

	default:// do nothing
		break;
	}
switch_1ss_end:
	return;

}

// switch OFDM path
//antPath: 01 for PathA,10 for PathB, 11for Path AB
void Switch_OFDM_Antenna(struct rtl8192cd_priv *priv, unsigned int antPath )
{
	unsigned int dword = 0;
	if(get_rf_mimo_mode(priv) != MIMO_2T2R)
		return;

	switch(antPath){
	case 1:
		dword = RTL_R32(0x90C);
		if((dword & 0x000000f0) == 0x00000010)
			goto switch_OFDM_end;
		dword &= 0xffffff0f;
		dword |= 0x00000010; // Path A
		RTL_W32(0x90C, dword);
		break;
	case 2:
		dword = RTL_R32(0x90C);
		if((dword & 0x000000f0) == 0x00000020)
			goto switch_OFDM_end;
		dword &= 0xffffff0f;
		dword |= 0x00000020;	// Path B
		RTL_W32(0x90C, dword);
		break;

	case 3:
		if(priv->pshare->rf_ft_var.ofdm_1ss_oneAnt == 1)// use one ANT for 1ss
			goto switch_OFDM_end;// do nothing
		dword = RTL_R32(0x90C);
		if((dword & 0x000000f0) == 0x00000030)
			goto switch_OFDM_end;
		dword &= 0xffffff0f;
		dword |= 0x00000030; // Path A,B
		RTL_W32(0x90C, dword);
		break;

	default:// do nothing
		break;
	}
switch_OFDM_end:
	return;

}


#endif

void enable_hw_LED(struct rtl8192cd_priv *priv, unsigned int led_type)
{

#if defined(HW_ANT_SWITCH) || defined(SW_ANT_SWITCH)
	int b23 = RTL_R32(LEDCFG) & BIT(23);
#endif

#ifdef  CONFIG_WLAN_HAL
    // TODO: we should check register then set
	if (IS_HAL_CHIP(priv))
		return;
#endif
	switch (led_type) {
	case LEDTYPE_HW_TX_RX:
#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8192D)
			RTL_W32(LEDCFG, LED_TX_RX_EVENT_ON<<LED1CM_SHIFT_92D | LED1DIS_92D);
#endif
#ifdef CONFIG_RTL_92C_SUPPORT
		if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C))
			RTL_W32(LEDCFG, LED_RX_EVENT_ON<<LED1CM_SHIFT | LED_TX_EVENT_ON<<LED0CM_SHIFT);
#endif

		break;
	case LEDTYPE_HW_LINKACT_INFRA:
		RTL_W32(LEDCFG, LED_TX_RX_EVENT_ON<<LED0CM_SHIFT);
		if ((OPMODE & WIFI_AP_STATE) || (OPMODE & WIFI_STATION_STATE))
			RTL_W32(LEDCFG, RTL_R32(LEDCFG) & 0x0ff);
		else
			RTL_W32(LEDCFG, (RTL_R32(LEDCFG) & 0xfffff0ff) | LED1SV);
		break;
	default:
		break;
	}

#if defined(HW_ANT_SWITCH) || defined(SW_ANT_SWITCH)
	RTL_W32(LEDCFG, b23 | RTL_R32(LEDCFG));
#endif

}


/**
* Function:	phy_InitBBRFRegisterDefinition
*
* OverView:	Initialize Register definition offset for Radio Path A/B/C/D
*
* Input:
*			PADAPTER		Adapter,
*
* Output:	None
* Return:		None
* Note:		The initialization value is constant and it should never be changes
*/
void phy_InitBBRFRegisterDefinition(struct rtl8192cd_priv *priv)
{
	struct rtl8192cd_hw *phw = GET_HW(priv);

	// RF Interface Sowrtware Control
	phw->PHYRegDef[RF92CD_PATH_A].rfintfs = rFPGA0_XAB_RFInterfaceSW; // 16 LSBs if read 32-bit from 0x870
	phw->PHYRegDef[RF92CD_PATH_B].rfintfs = rFPGA0_XAB_RFInterfaceSW; // 16 MSBs if read 32-bit from 0x870 (16-bit for 0x872)

	// RF Interface Readback Value
	phw->PHYRegDef[RF92CD_PATH_A].rfintfi = rFPGA0_XAB_RFInterfaceRB; // 16 LSBs if read 32-bit from 0x8E0
	phw->PHYRegDef[RF92CD_PATH_B].rfintfi = rFPGA0_XAB_RFInterfaceRB;// 16 MSBs if read 32-bit from 0x8E0 (16-bit for 0x8E2)

	// RF Interface Output (and Enable)
	phw->PHYRegDef[RF92CD_PATH_A].rfintfo = rFPGA0_XA_RFInterfaceOE; // 16 LSBs if read 32-bit from 0x860
	phw->PHYRegDef[RF92CD_PATH_B].rfintfo = rFPGA0_XB_RFInterfaceOE; // 16 LSBs if read 32-bit from 0x864

	// RF Interface (Output and)  Enable
	phw->PHYRegDef[RF92CD_PATH_A].rfintfe = rFPGA0_XA_RFInterfaceOE; // 16 MSBs if read 32-bit from 0x860 (16-bit for 0x862)
	phw->PHYRegDef[RF92CD_PATH_B].rfintfe = rFPGA0_XB_RFInterfaceOE; // 16 MSBs if read 32-bit from 0x864 (16-bit for 0x866)

	//Addr of LSSI. Wirte RF register by driver
	phw->PHYRegDef[RF92CD_PATH_A].rf3wireOffset = rFPGA0_XA_LSSIParameter; //LSSI Parameter
	phw->PHYRegDef[RF92CD_PATH_B].rf3wireOffset = rFPGA0_XB_LSSIParameter;

	// RF parameter
	phw->PHYRegDef[RF92CD_PATH_A].rfLSSI_Select = rFPGA0_XAB_RFParameter;  //BB Band Select
	phw->PHYRegDef[RF92CD_PATH_B].rfLSSI_Select = rFPGA0_XAB_RFParameter;

	// Tx AGC Gain Stage (same for all path. Should we remove this?)
	phw->PHYRegDef[RF92CD_PATH_A].rfTxGainStage = rFPGA0_TxGainStage; //Tx gain stage
	phw->PHYRegDef[RF92CD_PATH_B].rfTxGainStage = rFPGA0_TxGainStage; //Tx gain stage

	// Tranceiver A~D HSSI Parameter-1
	phw->PHYRegDef[RF92CD_PATH_A].rfHSSIPara1 = rFPGA0_XA_HSSIParameter1;  //wire control parameter1
	phw->PHYRegDef[RF92CD_PATH_B].rfHSSIPara1 = rFPGA0_XB_HSSIParameter1;  //wire control parameter1

	// Tranceiver A~D HSSI Parameter-2
	phw->PHYRegDef[RF92CD_PATH_A].rfHSSIPara2 = rFPGA0_XA_HSSIParameter2;  //wire control parameter2
	phw->PHYRegDef[RF92CD_PATH_B].rfHSSIPara2 = rFPGA0_XB_HSSIParameter2;  //wire control parameter2

	// RF switch Control
	phw->PHYRegDef[RF92CD_PATH_A].rfSwitchControl = rFPGA0_XAB_SwitchControl; //TR/Ant switch control
	phw->PHYRegDef[RF92CD_PATH_B].rfSwitchControl = rFPGA0_XAB_SwitchControl;

	// AGC control 1
	phw->PHYRegDef[RF92CD_PATH_A].rfAGCControl1 = rOFDM0_XAAGCCore1;
	phw->PHYRegDef[RF92CD_PATH_B].rfAGCControl1 = rOFDM0_XBAGCCore1;

	// AGC control 2
	phw->PHYRegDef[RF92CD_PATH_A].rfAGCControl2 = rOFDM0_XAAGCCore2;
	phw->PHYRegDef[RF92CD_PATH_B].rfAGCControl2 = rOFDM0_XBAGCCore2;

	// RX AFE control 1
	phw->PHYRegDef[RF92CD_PATH_A].rfRxIQImbalance = rOFDM0_XARxIQImbalance;
	phw->PHYRegDef[RF92CD_PATH_B].rfRxIQImbalance = rOFDM0_XBRxIQImbalance;

	// RX AFE control 1
	phw->PHYRegDef[RF92CD_PATH_A].rfRxAFE = rOFDM0_XARxAFE;
	phw->PHYRegDef[RF92CD_PATH_B].rfRxAFE = rOFDM0_XBRxAFE;

	// Tx AFE control 1
	phw->PHYRegDef[RF92CD_PATH_A].rfTxIQImbalance = rOFDM0_XATxIQImbalance;
	phw->PHYRegDef[RF92CD_PATH_B].rfTxIQImbalance = rOFDM0_XBTxIQImbalance;

	// Tx AFE control 2
	phw->PHYRegDef[RF92CD_PATH_A].rfTxAFE = rOFDM0_XATxAFE;
	phw->PHYRegDef[RF92CD_PATH_B].rfTxAFE = rOFDM0_XBTxAFE;

	// Tranceiver LSSI Readback SI mode
	phw->PHYRegDef[RF92CD_PATH_A].rfLSSIReadBack = rFPGA0_XA_LSSIReadBack;
	phw->PHYRegDef[RF92CD_PATH_B].rfLSSIReadBack = rFPGA0_XB_LSSIReadBack;

	// Tranceiver LSSI Readback PI mode
	phw->PHYRegDef[RF92CD_PATH_A].rfLSSIReadBackPi = TransceiverA_HSPI_Readback;
	phw->PHYRegDef[RF92CD_PATH_B].rfLSSIReadBackPi = TransceiverB_HSPI_Readback;
}


void check_chipID_MIMO(struct rtl8192cd_priv *priv)
{

#ifdef CONFIG_RTL_8812_SUPPORT //FOR_8812_MP_CHIP
		if (GET_CHIP_VER(priv) == VERSION_8812E)	{
			unsigned int val32;
			
			val32 = RTL_R32(SYS_CFG);
			if (val32 & BIT(23))
			{
				panic_printk("8812 test chip !! \n");
				priv->pshare->version_id |= 0x100; //is 8812 test chip
			}
			else
			{
				panic_printk("8812 mp chip !! \n");
			}
			
			priv->pshare->phw->MIMO_TR_hw_support = MIMO_2T2R;
			goto exit_func;
		}
#endif

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8188E)	{
		priv->pshare->phw->MIMO_TR_hw_support = MIMO_1T1R;
		goto exit_func;
	}
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY)
			priv->pshare->phw->MIMO_TR_hw_support = MIMO_1T1R;
		else
			priv->pshare->phw->MIMO_TR_hw_support = MIMO_2T2R;
		goto exit_func;
	}
#endif

#ifdef CONFIG_RTL_92C_SUPPORT
	{
		unsigned int val32;
		val32 = RTL_R32(SYS_CFG);
		if (val32 & BIT(27)) {
			priv->pshare->version_id = VERSION_8192C;
			priv->pshare->phw->MIMO_TR_hw_support = MIMO_2T2R;
		} else {
			priv->pshare->version_id = VERSION_8188C;
			priv->pshare->phw->MIMO_TR_hw_support = MIMO_1T1R;

			if ((0x3 &(RTL_R32(0xec)>>22))== 0x01)
				priv->pshare->version_id |= 0x200;		// 88RE
		}

		if (val32 & BIT(23)) {
			priv->pshare->version_id |= 0x100;
		}
		if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)) {
			if (val32 & BIT(19)) {
				priv->pshare->version_id |= 0x400;			// UMC
				priv->pshare->version_id |= (0xf0 & (val32>>8));	//	0:  a-cut
			}
			if(((0x0f & (val32>>16)) ==0) && ((0x0f & (val32>>12)) ==1)) {		//6195B
				priv->pshare->version_id |= 0x400;
				priv->pshare->version_id |= 0x10;					//	0x10:	b-cut
			}
		}
	}
	
#endif

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT)
exit_func:
#endif
	return;
}

void selectMinPowerIdex(struct rtl8192cd_priv *priv)
{
	int i=0,idx, pwr_min = 0xff;
	unsigned int val32;
	unsigned int pwr_regA[] = {0xe00,0xe04,0xe08,0x86c,0xe10,0xe14,0xe18,0xe1c};
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
	unsigned int pwr_regB[] = {0x830,0x834,0x838,0x86c,0x83c,0x848,0x84c,0x868};
#endif

	for (idx=0 ; idx < 8 ; idx++)
	{
		val32 = RTL_R32(pwr_regA[idx]);
		switch (pwr_regA[idx]) {
		case 0xe08:
			pwr_min = POWER_MIN_CHECK(pwr_min,(val32 >> 8) & 0xff);
			break;

		case 0x86c:
			for (i=8 ; i < 32 ; i+=8)
				pwr_min = POWER_MIN_CHECK(pwr_min,(val32 >> i) & 0xff);
			break;

		default:
			for (i=0 ; i < 32 ; i+=8)
				pwr_min = POWER_MIN_CHECK(pwr_min,(val32 >> i) & 0xff);
			break;
		}
	}

#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
	if (
#ifdef CONFIG_RTL_92C_SUPPORT
		(GET_CHIP_VER(priv) == VERSION_8192C) 
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
#ifdef CONFIG_RTL_92C_SUPPORT
		|| 
#endif
		((GET_CHIP_VER(priv) == VERSION_8192D)
#ifdef CONFIG_RTL_92D_DMDP
		&& (priv->pmib->dot11RFEntry.macPhyMode == SINGLEMAC_SINGLEPHY)
#endif
		)
#endif
		) {
		for (idx=0 ; idx < 8 ; idx++) {
			val32 = RTL_R32(pwr_regB[idx]);
			switch (pwr_regB[idx]) {
			case 0x86c:
				pwr_min = POWER_MIN_CHECK(pwr_min,val32 & 0xff);
				break;

			case 0x838:
				for (i=8 ; i < 32 ; i+=8)
					pwr_min = POWER_MIN_CHECK(pwr_min,(val32 >> i) & 0xff);
				break;

			default:
				for (i=0 ; i < 32 ; i+=8)
					pwr_min = POWER_MIN_CHECK(pwr_min,(val32 >> i) & 0xff);
				break;
			}
		}
	}
#endif

	priv->pshare->rf_ft_var.min_pwr_idex = pwr_min;
}

#ifdef POWER_PERCENT_ADJUSTMENT
char PwrPercent2PwrLevel(int percentage)
{
	#define ARRAYSIZE(x)	(sizeof(x)/sizeof((x)[0]))
	
	const int percent_threshold[] = {95, 85, 75, 67, 60, 54, 48, 43, 38, 34, 30, 27, 24, 22, 19, 17, 15, 14, 12, 11, 10};
	const char pwrlevel_diff[9] = {-40, -34, -30, -28, -26, -24, -23, -22, -21};	// for < 10% case
	int i;

	for (i = 0; i < ARRAYSIZE(percent_threshold); ++i) {
		if (percentage >= percent_threshold[i]) {
			return (char)-i;
		}
	}
	
	if (percentage < 1) percentage = 1;
	
	return pwrlevel_diff[percentage-1];
}
#endif

void PHY_RF6052SetOFDMTxPower(struct rtl8192cd_priv *priv, unsigned int channel)
{
	unsigned int writeVal, defValue =0x28 ;
	unsigned char  offset;
	char base, byte0, byte1, byte2, byte3;
	unsigned char pwrlevelHT40_1S_A = priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[channel-1];
	unsigned char pwrlevelHT40_1S_B = priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[channel-1];
	unsigned char pwrdiffHT40_2S = priv->pmib->dot11RFEntry.pwrdiffHT40_2S[channel-1];
	unsigned char pwrdiffHT20 = priv->pmib->dot11RFEntry.pwrdiffHT20[channel-1];
	unsigned char pwrdiffOFDM = priv->pmib->dot11RFEntry.pwrdiffOFDM[channel-1];


#ifdef USB_POWER_SUPPORT
//_TXPWR_REDEFINE
	unsigned char pwrlevelHT40_6dBm_1S_A;
	unsigned char pwrlevelHT40_6dBm_1S_B;
	unsigned char pwrdiffHT40_6dBm_2S;
	unsigned char pwrdiffHT20_6dBm;
	unsigned char pwrdiffOFDM_6dBm;
	unsigned char offset_6dBm;
	char base_6dBm;


    
#endif
#ifdef POWER_PERCENT_ADJUSTMENT
	char pwrdiff_percent = PwrPercent2PwrLevel(priv->pmib->dot11RFEntry.power_percent);
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
	unsigned int ori_channel = channel; //Keep the original channel setting
#endif

// debug, 92e use default power index

#if	0 //defined(CONFIG_WLAN_HAL_8192EE)

	if(GET_CHIP_VER(priv) == VERSION_8192E) {
		pwrlevelHT40_1S_A = pwrlevelHT40_1S_B = 0;
		pwrdiffHT40_2S = pwrdiffHT20 = pwrdiffOFDM = 0;
		defValue = 0x20;
	}
#endif	


#ifdef CONFIG_RTL_88E_SUPPORT
	/* for testchip only */
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
		defValue =0x21;

		
#if defined(CALIBRATE_BY_ODM)
#ifdef MP_TEST
		if ((priv->pshare->rf_ft_var.mp_specific) && priv->pshare->mp_txpwr_patha)
			defValue = priv->pshare->mp_txpwr_patha;
#endif
		defValue += (ODMPTR->BbSwingIdxOfdm[RF_PATH_A] - ODMPTR->BbSwingIdxOfdmBase);
#endif	
	}
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8192D) {
#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8198B)
		if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G)
			defValue=0x28;
		else
			defValue=0x2d;
#else
		if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G)
			defValue=0x26;
		else
			defValue=0x30;
#endif

		//TXPWR_REDEFINE
		//FLASH GROUP [36-99] [100-148] [149-165] 
		//Special Cases: [34-2, 34, 34+2,  36-2, 165+2]:No DATA , [149-2]:FLASH DATA OF Channel-146-6dBm
		//Use Flash data of channel 36 & 140 & 165 for these special cases.
		if((channel > 30) && (channel < 36))
			channel = 36;
		else if (channel == (149-2))
			channel = 140;
		else if(channel > 165)
			channel = 165;
		
		if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
			pwrlevelHT40_1S_A = priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[channel-1];
			pwrlevelHT40_1S_B = priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[channel-1];
			pwrdiffHT40_2S = priv->pmib->dot11RFEntry.pwrdiff5GHT40_2S[channel-1];
			pwrdiffHT20 = priv->pmib->dot11RFEntry.pwrdiff5GHT20[channel-1];
			pwrdiffOFDM = priv->pmib->dot11RFEntry.pwrdiff5GOFDM[channel-1];
		}

#ifdef USB_POWER_SUPPORT
//_TXPWR_REDEFINE
//MCS 8 - 15: No Power By Rate
//Others: Power by Rate (Add Power)
//Remove PWR_5G_DIFF

//?? phyBandSelect will auto swtich or 2G | 5G ??
		{
			int i;
			
			if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
				pwrlevelHT40_6dBm_1S_A = priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[channel];
				pwrlevelHT40_6dBm_1S_B = priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[channel];
				pwrdiffHT40_6dBm_2S = priv->pmib->dot11RFEntry.pwrdiff5GHT40_2S[channel];
				pwrdiffHT20_6dBm = priv->pmib->dot11RFEntry.pwrdiff5GHT20[channel];
				pwrdiffOFDM_6dBm = priv->pmib->dot11RFEntry.pwrdiff5GOFDM[channel];
			} else {
				pwrlevelHT40_6dBm_1S_A = priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[channel-1];
				pwrlevelHT40_6dBm_1S_B = priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[channel-1];
				pwrdiffHT40_6dBm_2S = priv->pmib->dot11RFEntry.pwrdiff5GHT40_2S[channel-1];
				pwrdiffHT20_6dBm = priv->pmib->dot11RFEntry.pwrdiff5GHT20[channel-1];
				pwrdiffOFDM_6dBm = priv->pmib->dot11RFEntry.pwrdiff5GOFDM[channel-1];
			}
		}
#endif
			

#ifdef CONFIG_RTL_92D_DMDP
		if (priv->pmib->dot11RFEntry.macPhyMode==DUALMAC_DUALPHY &&
			priv->pshare->wlandev_idx == 1) {
			if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G)
				pwrlevelHT40_1S_A = priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[channel-1];
			else {
				pwrlevelHT40_1S_A = priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[channel-1];
			}
//_TXPWR_REDEFINE
#ifdef USB_POWER_SUPPORT
			if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
				pwrlevelHT40_6dBm_1S_A = priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[channel];
			} else {
				pwrlevelHT40_6dBm_1S_A = priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[channel-1];
			}	
#endif
		}
#endif

		channel = ori_channel; //_TXPWR_REDEFINE Restore the channel setting

#ifdef TXPWR_LMT
		if (!priv->pshare->rf_ft_var.disable_txpwrlmt){
			int i;
			int max_idx;

			if (!priv->pshare->txpwr_lmt_OFDM || !priv->pshare->tgpwr_OFDM){
				//printk("No limit for OFDM TxPower\n");
				max_idx=255;
			}else{
				// maximum additional power index
				max_idx = (priv->pshare->txpwr_lmt_OFDM - priv->pshare->tgpwr_OFDM);
			}

			for (i=0; i<=7; i++) {
				priv->pshare->phw->OFDMTxAgcOffset_A[i] = POWER_MIN_CHECK(priv->pshare->phw->OFDMTxAgcOffset_A[i], max_idx);
				priv->pshare->phw->OFDMTxAgcOffset_B[i] = POWER_MIN_CHECK(priv->pshare->phw->OFDMTxAgcOffset_B[i], max_idx);
				//printk("priv->pshare->phw->OFDMTxAgcOffset_A[%d]=%x\n",i, priv->pshare->phw->OFDMTxAgcOffset_A[i]);
				//printk("priv->pshare->phw->OFDMTxAgcOffset_B[%d]=%x\n",i, priv->pshare->phw->OFDMTxAgcOffset_B[i]);
			}

			if (!priv->pshare->txpwr_lmt_HT1S || !priv->pshare->tgpwr_HT1S){
				//printk("No limit for HT1S TxPower\n");
				max_idx = 255;
			}else{
				// maximum additional power index 
				max_idx = (priv->pshare->txpwr_lmt_HT1S - priv->pshare->tgpwr_HT1S);
			}
			
			for (i=0; i<=7; i++) {
				priv->pshare->phw->MCSTxAgcOffset_A[i] = POWER_MIN_CHECK(priv->pshare->phw->MCSTxAgcOffset_A[i], max_idx);
				priv->pshare->phw->MCSTxAgcOffset_B[i] = POWER_MIN_CHECK(priv->pshare->phw->MCSTxAgcOffset_B[i], max_idx);
				//printk("priv->pshare->phw->MCSTxAgcOffset_A[%d]=%x\n",i, priv->pshare->phw->MCSTxAgcOffset_A[i]);
				//printk("priv->pshare->phw->MCSTxAgcOffset_B[%d]=%x\n",i, priv->pshare->phw->MCSTxAgcOffset_B[i]);
			}
			
			if (!priv->pshare->txpwr_lmt_HT2S || !priv->pshare->tgpwr_HT2S){
				//printk("No limit for HT2S TxPower\n");
				max_idx = 255;
			}else{
				// maximum additional power index 
				max_idx = (priv->pshare->txpwr_lmt_HT2S - priv->pshare->tgpwr_HT2S);
			}
			
			for (i=8; i<=15; i++) {
				priv->pshare->phw->MCSTxAgcOffset_A[i] = POWER_MIN_CHECK(priv->pshare->phw->MCSTxAgcOffset_A[i], max_idx);
				priv->pshare->phw->MCSTxAgcOffset_B[i] = POWER_MIN_CHECK(priv->pshare->phw->MCSTxAgcOffset_B[i], max_idx);
				//printk("priv->pshare->phw->MCSTxAgcOffset_A[%d]=%x\n",i, priv->pshare->phw->MCSTxAgcOffset_A[i]);
				//printk("priv->pshare->phw->MCSTxAgcOffset_B[%d]=%x\n",i, priv->pshare->phw->MCSTxAgcOffset_B[i]);
			}
		}
#endif
	}
#endif

	if ((pwrlevelHT40_1S_A == 0) 
#if defined(MP_TEST) && defined(CONFIG_RTL_88E_SUPPORT) && defined(CALIBRATE_BY_ODM)
				|| ((priv->pshare->rf_ft_var.mp_specific) || (OPMODE & WIFI_MP_STATE))
#endif	
	)

	{	// use default value

#ifdef HIGH_POWER_EXT_PA
		if (priv->pshare->rf_ft_var.use_ext_pa)
			defValue = HP_OFDM_POWER_DEFAULT ;
#endif
#ifndef ADD_TX_POWER_BY_CMD
		writeVal = (defValue<<24)|(defValue<<16)|(defValue<<8)|(defValue);
		RTL_W32(rTxAGC_A_Rate18_06, writeVal);
		RTL_W32(rTxAGC_A_Rate54_24, writeVal);
		RTL_W32(rTxAGC_A_Mcs03_Mcs00, writeVal);
		RTL_W32(rTxAGC_A_Mcs07_Mcs04, writeVal);
		RTL_W32(rTxAGC_B_Rate18_06, writeVal);
		RTL_W32(rTxAGC_B_Rate54_24, writeVal);
		RTL_W32(rTxAGC_B_Mcs03_Mcs00, writeVal);
		RTL_W32(rTxAGC_B_Mcs07_Mcs04, writeVal);

#ifdef USB_POWER_SUPPORT
//_TXPWR_REDEFINE, pwrlevelHT40_1S_A == 0 >> No 6dBm Power >> default value >> so USB = def - 14
			writeVal = POWER_RANGE_CHECK(defValue - USB_HT_2S_DIFF);
			writeVal |= (writeVal<<24)|(writeVal<<16)|(writeVal<<8);
#endif
		RTL_W32(rTxAGC_A_Mcs11_Mcs08, writeVal);
		RTL_W32(rTxAGC_A_Mcs15_Mcs12, writeVal);
		RTL_W32(rTxAGC_B_Mcs11_Mcs08, writeVal);
		RTL_W32(rTxAGC_B_Mcs15_Mcs12, writeVal);
#else
		base = defValue;
		byte0 = byte1 = byte2 = byte3 = 0;
		ASSIGN_TX_POWER_OFFSET(byte0, priv->pshare->rf_ft_var.txPowerPlus_ofdm_18);
		ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_ofdm_12);
		ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_ofdm_9);
		ASSIGN_TX_POWER_OFFSET(byte3, priv->pshare->rf_ft_var.txPowerPlus_ofdm_6);

		byte0 = POWER_RANGE_CHECK(base + byte0);
		byte1 = POWER_RANGE_CHECK(base + byte1);
		byte2 = POWER_RANGE_CHECK(base + byte2);
		byte3 = POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
		RTL_W32(rTxAGC_A_Rate18_06, writeVal);
		RTL_W32(rTxAGC_B_Rate18_06, writeVal);

		byte0 = byte1 = byte2 = byte3 = 0;
		ASSIGN_TX_POWER_OFFSET(byte0, priv->pshare->rf_ft_var.txPowerPlus_ofdm_54);
		ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_ofdm_48);
		ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_ofdm_36);
		ASSIGN_TX_POWER_OFFSET(byte3, priv->pshare->rf_ft_var.txPowerPlus_ofdm_24);
		byte0 = POWER_RANGE_CHECK(base + byte0);
		byte1 = POWER_RANGE_CHECK(base + byte1);
		byte2 = POWER_RANGE_CHECK(base + byte2);
		byte3 = POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
		RTL_W32(rTxAGC_A_Rate54_24, writeVal);
		RTL_W32(rTxAGC_B_Rate54_24, writeVal);

		byte0 = byte1 = byte2 = byte3 = 0;
		ASSIGN_TX_POWER_OFFSET(byte0, priv->pshare->rf_ft_var.txPowerPlus_mcs_3);
		ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_mcs_2);
		ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_mcs_1);
		ASSIGN_TX_POWER_OFFSET(byte3, priv->pshare->rf_ft_var.txPowerPlus_mcs_0);
		byte0 = POWER_RANGE_CHECK(base + byte0);
		byte1 = POWER_RANGE_CHECK(base + byte1);
		byte2 = POWER_RANGE_CHECK(base + byte2);
		byte3 = POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
		RTL_W32(rTxAGC_A_Mcs03_Mcs00, writeVal);
		RTL_W32(rTxAGC_B_Mcs03_Mcs00, writeVal);

		byte0 = byte1 = byte2 = byte3 = 0;
		ASSIGN_TX_POWER_OFFSET(byte0, priv->pshare->rf_ft_var.txPowerPlus_mcs_7);
		ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_mcs_6);
		ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_mcs_5);
		ASSIGN_TX_POWER_OFFSET(byte3, priv->pshare->rf_ft_var.txPowerPlus_mcs_4);
		byte0 = POWER_RANGE_CHECK(base + byte0);
		byte1 = POWER_RANGE_CHECK(base + byte1);
		byte2 = POWER_RANGE_CHECK(base + byte2);
		byte3 = POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
		RTL_W32(rTxAGC_A_Mcs07_Mcs04, writeVal);
		RTL_W32(rTxAGC_B_Mcs07_Mcs04, writeVal);

//_TXPWR_REDEFINE
#ifdef USB_POWER_SUPPORT
			byte0 = byte1 = byte2 = byte3 = -USB_HT_2S_DIFF;
#else
			byte0 = byte1 = byte2 = byte3 = 0;
			ASSIGN_TX_POWER_OFFSET(byte0, priv->pshare->rf_ft_var.txPowerPlus_mcs_11);
			ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_mcs_10);
			ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_mcs_9);
			ASSIGN_TX_POWER_OFFSET(byte3, priv->pshare->rf_ft_var.txPowerPlus_mcs_8);
#endif

		byte0 = POWER_RANGE_CHECK(base + byte0);
		byte1 = POWER_RANGE_CHECK(base + byte1);
		byte2 = POWER_RANGE_CHECK(base + byte2);
		byte3 = POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
		RTL_W32(rTxAGC_A_Mcs11_Mcs08, writeVal);
		RTL_W32(rTxAGC_B_Mcs11_Mcs08, writeVal);

//_TXPWR_REDEFINE
#ifdef USB_POWER_SUPPORT
			byte0 = byte1 = byte2 = byte3 = -USB_HT_2S_DIFF;
#else
			byte0 = byte1 = byte2 = byte3 = 0;
			ASSIGN_TX_POWER_OFFSET(byte0, priv->pshare->rf_ft_var.txPowerPlus_mcs_15);
			ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_mcs_14);
			ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_mcs_13);
			ASSIGN_TX_POWER_OFFSET(byte3, priv->pshare->rf_ft_var.txPowerPlus_mcs_12);
#endif

		byte0 = POWER_RANGE_CHECK(base + byte0);
		byte1 = POWER_RANGE_CHECK(base + byte1);
		byte2 = POWER_RANGE_CHECK(base + byte2);
		byte3 = POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
		RTL_W32(rTxAGC_A_Mcs15_Mcs12, writeVal);
		RTL_W32(rTxAGC_B_Mcs15_Mcs12, writeVal);

#endif // ADD_TX_POWER_BY_CMD
		return; // use default
	}

	/******************************  PATH A  ******************************/
	base = pwrlevelHT40_1S_A;
	offset = (pwrdiffOFDM & 0x0f);
#if defined(CONFIG_RTL_92D_SUPPORT)&& defined(CONFIG_RTL_92D_DMDP)
//_TXPWR_REDEFINE??
	if (priv->pmib->dot11RFEntry.macPhyMode==DUALMAC_DUALPHY && priv->pshare->wlandev_idx == 1) {
		offset = ((pwrdiffOFDM & 0xf0) >> 4);
	}
#endif	
	base = COUNT_SIGN_OFFSET(base, offset);
#ifdef POWER_PERCENT_ADJUSTMENT
	base = base + pwrdiff_percent;
#endif

#if defined(CONFIG_RTL_88E_SUPPORT) && defined(CALIBRATE_BY_ODM)
	if (GET_CHIP_VER(priv) == VERSION_8188E)
		base += (ODMPTR->BbSwingIdxOfdm[RF_PATH_A] - ODMPTR->BbSwingIdxOfdmBase);
#endif

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[0]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[1]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[2]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[3]);
	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	RTL_W32(rTxAGC_A_Rate18_06, writeVal);

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[4]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[5]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[6]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[7]);
	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	RTL_W32(rTxAGC_A_Rate54_24, writeVal);

	base = pwrlevelHT40_1S_A;
	if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20) {
		offset = (pwrdiffHT20 & 0x0f);
#if defined(CONFIG_RTL_92D_SUPPORT)&& defined(CONFIG_RTL_92D_DMDP)
//_TXPWR_REDEFINE??
		if (priv->pmib->dot11RFEntry.macPhyMode==DUALMAC_DUALPHY && priv->pshare->wlandev_idx == 1) {
			offset = ((pwrdiffHT20 & 0xf0) >> 4);
		}
#endif	
		base = COUNT_SIGN_OFFSET(base, offset);
	}
#ifdef POWER_PERCENT_ADJUSTMENT
	base = base + pwrdiff_percent;
#endif

#if defined(CONFIG_RTL_88E_SUPPORT) && defined(CALIBRATE_BY_ODM)
	if (GET_CHIP_VER(priv) == VERSION_8188E)
		base += (ODMPTR->BbSwingIdxOfdm[RF_PATH_A] - ODMPTR->BbSwingIdxOfdmBase);
#endif

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[0]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[1]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[2]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[3]);
	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	RTL_W32(rTxAGC_A_Mcs03_Mcs00, writeVal);

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[4]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[5]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[6]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[7]);
	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	RTL_W32(rTxAGC_A_Mcs07_Mcs04, writeVal);

	offset = (pwrdiffHT40_2S & 0x0f);	
#if defined(CONFIG_RTL_92D_SUPPORT)&& defined(CONFIG_RTL_92D_DMDP)
//_TXPWR_REDEFINE??
	if (priv->pmib->dot11RFEntry.macPhyMode==DUALMAC_DUALPHY && priv->pshare->wlandev_idx == 1) {
		offset = ((pwrdiffHT40_2S & 0xf0) >> 4);
	}
#endif	
	base = COUNT_SIGN_OFFSET(base, offset);

//_TXPWR_REDEFINE
#ifdef USB_POWER_SUPPORT

	base_6dBm = pwrlevelHT40_6dBm_1S_A;

	if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20) {
		offset_6dBm = (pwrdiffHT20_6dBm & 0x0f);
		
#if defined(CONFIG_RTL_92D_SUPPORT)&& defined(CONFIG_RTL_92D_DMDP)
		if (priv->pmib->dot11RFEntry.macPhyMode==DUALMAC_DUALPHY && priv->pshare->wlandev_idx == 1) {
			offset_6dBm = ((pwrdiffHT20_6dBm & 0xf0) >> 4);
		}
#endif	

		base_6dBm = COUNT_SIGN_OFFSET(base_6dBm, offset_6dBm);
	}
	
	offset_6dBm = (pwrdiffHT40_6dBm_2S & 0x0f);
	
#if defined(CONFIG_RTL_92D_SUPPORT)&& defined(CONFIG_RTL_92D_DMDP)
	if (priv->pmib->dot11RFEntry.macPhyMode==DUALMAC_DUALPHY && priv->pshare->wlandev_idx == 1) {
		offset_6dBm = ((pwrdiffHT40_6dBm_2S & 0xf0) >> 4);
	}
#endif	

	base_6dBm = COUNT_SIGN_OFFSET(base_6dBm, offset_6dBm);

	if ((pwrlevelHT40_6dBm_1S_A != 0) && (pwrlevelHT40_6dBm_1S_A != pwrlevelHT40_1S_A))
		byte0 = byte1 = byte2 = byte3 =	base_6dBm;
	else if((base - USB_HT_2S_DIFF) > 0)
		byte0 = byte1 = byte2 = byte3 =	POWER_RANGE_CHECK(base -USB_HT_2S_DIFF);
	else
		byte0 = byte1 = byte2 = byte3 =	POWER_RANGE_CHECK(defValue - USB_HT_2S_DIFF);

#else
//_TXPWR_REDEFINE ?? MCS 8 - 11, shall NOT add power by rate even NOT USB power ??
		byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[8]);
		byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[9]);
		byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[10]);
		byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[11]);
#endif

	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;

	//DEBUG_INFO("debug e18:%x,%x,[%x,%x,%x,%x],%x\n", offset, base, byte0, byte1, byte2, byte3, writeVal);
	RTL_W32(rTxAGC_A_Mcs11_Mcs08, writeVal);

#ifdef USB_POWER_SUPPORT
//_TXPWR_REDEFINE
	if ((pwrlevelHT40_6dBm_1S_A != 0) && (pwrlevelHT40_6dBm_1S_A != pwrlevelHT40_1S_A))
		byte0 = byte1 = byte2 = byte3 =	base_6dBm;
	else if((base - USB_HT_2S_DIFF) > 0)
		byte0 = byte1 = byte2 = byte3 =	POWER_RANGE_CHECK(base -USB_HT_2S_DIFF);
	else
		byte0 = byte1 = byte2 = byte3 =	POWER_RANGE_CHECK(defValue - USB_HT_2S_DIFF);

#else
		byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[12]);
		byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[13]);
		byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[14]);
		byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[15]);
#endif

	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	RTL_W32(rTxAGC_A_Mcs15_Mcs12, writeVal);

	/******************************  PATH B  ******************************/
	base = pwrlevelHT40_1S_B;
	offset = ((pwrdiffOFDM & 0xf0) >> 4);
	base = COUNT_SIGN_OFFSET(base, offset);
#ifdef POWER_PERCENT_ADJUSTMENT
	base = base + pwrdiff_percent;
#endif

#if defined(CONFIG_RTL_88E_SUPPORT) && defined(CALIBRATE_BY_ODM)
	if (GET_CHIP_VER(priv) == VERSION_8188E)
		base += (ODMPTR->BbSwingIdxOfdm[RF_PATH_A] - ODMPTR->BbSwingIdxOfdmBase);
#endif

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[0]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[1]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[2]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[3]);
	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	RTL_W32(rTxAGC_B_Rate18_06, writeVal);

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[4]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[5]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[6]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[7]);
	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	RTL_W32(rTxAGC_B_Rate54_24, writeVal);

	base = pwrlevelHT40_1S_B;
	if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20) {
		offset = ((pwrdiffHT20 & 0xf0) >> 4);
		base = COUNT_SIGN_OFFSET(base, offset);
	}
#ifdef POWER_PERCENT_ADJUSTMENT
	base = base + pwrdiff_percent;
#endif

#if defined(CONFIG_RTL_88E_SUPPORT) && defined(CALIBRATE_BY_ODM)
	if (GET_CHIP_VER(priv) == VERSION_8188E)
		base += (ODMPTR->BbSwingIdxOfdm[RF_PATH_A] - ODMPTR->BbSwingIdxOfdmBase);
#endif

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[0]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[1]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[2]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[3]);
	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	RTL_W32(rTxAGC_B_Mcs03_Mcs00, writeVal);

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[4]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[5]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[6]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[7]);
	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	RTL_W32(rTxAGC_B_Mcs07_Mcs04, writeVal);

	offset = ((pwrdiffHT40_2S & 0xf0) >> 4);
	base = COUNT_SIGN_OFFSET(base, offset);

#ifdef USB_POWER_SUPPORT
//_TXPWR_REDEFINE ?? 2.4G
	base_6dBm = pwrlevelHT40_6dBm_1S_B;
	if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20) {
		offset_6dBm = ((pwrdiffHT20_6dBm & 0xf0) >> 4);
		base_6dBm = COUNT_SIGN_OFFSET(base_6dBm, offset_6dBm);
	}

	offset_6dBm = ((pwrdiffHT40_6dBm_2S & 0xf0) >> 4);
	base_6dBm = COUNT_SIGN_OFFSET(base_6dBm, offset_6dBm);
	
	if (( pwrlevelHT40_6dBm_1S_B != 0 ) && (pwrlevelHT40_6dBm_1S_B != pwrlevelHT40_1S_B))
		byte0 = byte1 = byte2 = byte3 = base_6dBm;
	else if((base - USB_HT_2S_DIFF) > 0)
		byte0 = byte1 = byte2 = byte3 = POWER_RANGE_CHECK(base -USB_HT_2S_DIFF);
	else
		byte0 = byte1 = byte2 = byte3 =	POWER_RANGE_CHECK(defValue - USB_HT_2S_DIFF);

#else
		byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[8]);
		byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[9]);
		byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[10]);
		byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[11]);
#endif

	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	RTL_W32(rTxAGC_B_Mcs11_Mcs08, writeVal);

#ifdef USB_POWER_SUPPORT
//_TXPWR_REDEFINE ?? 2.4G
	if (( pwrlevelHT40_6dBm_1S_B != 0 ) && (pwrlevelHT40_6dBm_1S_B != pwrlevelHT40_1S_B))
		byte0 = byte1 = byte2 = byte3 = base_6dBm;
	else if((base - USB_HT_2S_DIFF) > 0)
		byte0 = byte1 = byte2 = byte3 = POWER_RANGE_CHECK(base -USB_HT_2S_DIFF);
	else
		byte0 = byte1 = byte2 = byte3 =	POWER_RANGE_CHECK(defValue - USB_HT_2S_DIFF);

#else
		byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[12]);
		byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[13]);
		byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[14]);
		byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[15]);
#endif

	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	RTL_W32(rTxAGC_B_Mcs15_Mcs12, writeVal);
}	/* PHY_RF6052SetOFDMTxPower */


void PHY_RF6052SetCCKTxPower(struct rtl8192cd_priv *priv, unsigned int channel)
{
	unsigned int writeVal = 0;
	char byte, byte1, byte2;
	char pwrlevelCCK_A = priv->pmib->dot11RFEntry.pwrlevelCCK_A[channel-1];
	char pwrlevelCCK_B = priv->pmib->dot11RFEntry.pwrlevelCCK_B[channel-1];
#ifdef POWER_PERCENT_ADJUSTMENT
	char pwrdiff_percent = PwrPercent2PwrLevel(priv->pmib->dot11RFEntry.power_percent);
#endif

// debug

#if	0 //defined(CONFIG_WLAN_HAL_8192EE)

	if(GET_CHIP_VER(priv) == VERSION_8192E) {
		pwrlevelCCK_A = pwrlevelCCK_B = 0;
	}
#endif	



#if defined(CONFIG_RTL_92D_SUPPORT) && defined(CONFIG_RTL_92D_DMDP)
	if (GET_CHIP_VER(priv)==VERSION_8192D) {
		if (priv->pmib->dot11RFEntry.macPhyMode==DUALMAC_DUALPHY &&
			priv->pshare->wlandev_idx == 1){
			if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G)  {
				pwrlevelCCK_A = priv->pmib->dot11RFEntry.pwrlevelCCK_B[channel-1];
			}
		}
	}
#endif
#ifdef TXPWR_LMT
	if (!priv->pshare->rf_ft_var.disable_txpwrlmt){
		int max_idx, i;		
		if (!priv->pshare->txpwr_lmt_CCK || !priv->pshare->tgpwr_CCK){
			DEBUG_INFO("No limit for CCK TxPower\n");
			max_idx=255;
		}else{
			// maximum additional power index
			max_idx = (priv->pshare->txpwr_lmt_CCK - priv->pshare->tgpwr_CCK);
		}

		for (i=0; i<=3; i++) {
			priv->pshare->phw->CCKTxAgc_A[i] = POWER_MIN_CHECK(priv->pshare->phw->CCKTxAgc_A[i], max_idx);
			priv->pshare->phw->CCKTxAgc_B[i] = POWER_MIN_CHECK(priv->pshare->phw->CCKTxAgc_B[i], max_idx);
			//printk("priv->pshare->phw->CCKTxAgc_A[%d]=%x\n",i, priv->pshare->phw->CCKTxAgc_A[i]);
			//printk("priv->pshare->phw->CCKTxAgc_A[%d]=%x\n",i, priv->pshare->phw->CCKTxAgc_A[i]);
		}
	}
#endif

	if (priv->pshare->rf_ft_var.cck_pwr_max) {
		//byte = POWER_RANGE_CHECK(priv->pshare->rf_ft_var.cck_pwr_max);
		byte = (priv->pshare->rf_ft_var.cck_pwr_max > 0x3f)? 0x3f : priv->pshare->rf_ft_var.cck_pwr_max;
		writeVal = byte;
		PHY_SetBBReg(priv, rTxAGC_A_CCK1_Mcs32, 0x0000ff00, writeVal);
		writeVal = (byte << 16) | (byte << 8) | byte;
		PHY_SetBBReg(priv, rTxAGC_B_CCK5_1_Mcs32, 0xffffff00, writeVal);
		writeVal = (byte << 24) | (byte << 16) | (byte << 8) | byte;
		PHY_SetBBReg(priv, rTxAGC_A_CCK11_2_B_CCK11, 0xffffffff, writeVal);
		return;
	}

	if ((pwrlevelCCK_A == 0) 
#if defined(MP_TEST) && defined(CONFIG_RTL_88E_SUPPORT) && defined(CALIBRATE_BY_ODM)
		|| ((priv->pshare->rf_ft_var.mp_specific) || (OPMODE & WIFI_MP_STATE))
#endif					
	)
	{	// use default value
#ifdef HIGH_POWER_EXT_PA
		if (priv->pshare->rf_ft_var.use_ext_pa)
			byte = HP_CCK_POWER_DEFAULT;
		else
#endif
			byte = 0x24;

#ifdef CONFIG_RTL_88E_SUPPORT
	/* for testchip only */
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
		byte =0x21;

#if defined(CALIBRATE_BY_ODM)
#ifdef MP_TEST
		if ((priv->pshare->rf_ft_var.mp_specific) && priv->pshare->mp_txpwr_patha)
			byte = priv->pshare->mp_txpwr_patha;
#endif

		byte += (ODMPTR->BbSwingIdxCck - ODMPTR->BbSwingIdxCckBase);
#endif	
	}
#endif

#ifndef ADD_TX_POWER_BY_CMD
		writeVal = byte;
		PHY_SetBBReg(priv, rTxAGC_A_CCK1_Mcs32, 0x0000ff00, writeVal);
		writeVal = (byte << 16) | (byte << 8) | byte;
		PHY_SetBBReg(priv, rTxAGC_B_CCK5_1_Mcs32, 0xffffff00, writeVal);
		writeVal = (byte << 24) | (byte << 16) | (byte << 8) | byte;
		PHY_SetBBReg(priv, rTxAGC_A_CCK11_2_B_CCK11, 0xffffffff, writeVal);
#else
		pwrlevelCCK_A = pwrlevelCCK_B = byte;
		byte = 0;
		ASSIGN_TX_POWER_OFFSET(byte, priv->pshare->rf_ft_var.txPowerPlus_cck_1);
		writeVal = POWER_RANGE_CHECK(pwrlevelCCK_A + byte);
		PHY_SetBBReg(priv, rTxAGC_A_CCK1_Mcs32, 0x0000ff00, writeVal);

		byte = byte1 = byte2 = 0;
		ASSIGN_TX_POWER_OFFSET(byte, priv->pshare->rf_ft_var.txPowerPlus_cck_1);
		ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_cck_2);
		ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_cck_5);
		byte  = POWER_RANGE_CHECK(pwrlevelCCK_B + byte);
		byte1 = POWER_RANGE_CHECK(pwrlevelCCK_B + byte1);
		byte2 = POWER_RANGE_CHECK(pwrlevelCCK_B + byte2);
		writeVal = ((byte2 << 16) | (byte1 << 8) | byte);
		PHY_SetBBReg(priv, rTxAGC_B_CCK5_1_Mcs32, 0xffffff00, writeVal);

		byte = byte1 = byte2 = 0;
		ASSIGN_TX_POWER_OFFSET(byte, priv->pshare->rf_ft_var.txPowerPlus_cck_2);
		ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_cck_5);
		ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_cck_11);
		byte  = POWER_RANGE_CHECK(pwrlevelCCK_A + byte);
		byte1 = POWER_RANGE_CHECK(pwrlevelCCK_A + byte1);
		byte2 = POWER_RANGE_CHECK(pwrlevelCCK_A + byte2);
		writeVal = ((byte2 << 24) | (byte1 << 16) | (byte << 8) | byte2);
		PHY_SetBBReg(priv, rTxAGC_A_CCK11_2_B_CCK11, 0xffffffff, writeVal);
#endif
		return; // use default
	}
#ifdef POWER_PERCENT_ADJUSTMENT
	pwrlevelCCK_A += pwrdiff_percent;
	pwrlevelCCK_B += pwrdiff_percent;
#endif

#if defined(CONFIG_RTL_88E_SUPPORT) && defined(CALIBRATE_BY_ODM)
	if (GET_CHIP_VER(priv) == VERSION_8188E)
		pwrlevelCCK_A += (ODMPTR->BbSwingIdxCck - ODMPTR->BbSwingIdxCckBase);
#endif

	writeVal = POWER_RANGE_CHECK(pwrlevelCCK_A + priv->pshare->phw->CCKTxAgc_A[3]);
	PHY_SetBBReg(priv, rTxAGC_A_CCK1_Mcs32, 0x0000ff00, writeVal);
	writeVal = (POWER_RANGE_CHECK(pwrlevelCCK_B + priv->pshare->phw->CCKTxAgc_B[1]) << 16) |
	           (POWER_RANGE_CHECK(pwrlevelCCK_B + priv->pshare->phw->CCKTxAgc_B[2]) << 8)  |
	            POWER_RANGE_CHECK(pwrlevelCCK_B + priv->pshare->phw->CCKTxAgc_B[3]);
	PHY_SetBBReg(priv, rTxAGC_B_CCK5_1_Mcs32, 0xffffff00, writeVal);
	writeVal = (POWER_RANGE_CHECK(pwrlevelCCK_A + priv->pshare->phw->CCKTxAgc_A[0]) << 24) |
	           (POWER_RANGE_CHECK(pwrlevelCCK_A + priv->pshare->phw->CCKTxAgc_A[1]) << 16) |
	           (POWER_RANGE_CHECK(pwrlevelCCK_A + priv->pshare->phw->CCKTxAgc_A[2]) << 8)  |
	            POWER_RANGE_CHECK(pwrlevelCCK_B + priv->pshare->phw->CCKTxAgc_B[0]);
	PHY_SetBBReg(priv, rTxAGC_A_CCK11_2_B_CCK11, 0xffffffff, writeVal);
}


#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A)

int PHY_CheckBBWithParaFile(struct rtl8192cd_priv *priv, int reg_file)
{
	int                read_bytes=0, num, len=0;
	unsigned int       u4bRegOffset, u4bRegValue, u4bRegMask;
	int 			   file_format=TWO_COLUMN;
	unsigned char      *mem_ptr, *line_head, *next_head=NULL;
	struct PhyRegTable *phyreg_table=NULL;
	struct MacRegTable *macreg_table=NULL;
	unsigned short     max_len=0;
	
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A)
	int				idx=0, pg_tbl_idx=BGN_2040_ALL, write_en=0;
#endif


	if (reg_file == PHYREG) {
#ifdef CONFIG_WLAN_HAL_8881A
        if(GET_CHIP_VER(priv)==VERSION_8881A) {
            phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_buf;
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_SIZE, (pu1Byte)&read_bytes);
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_START, &next_head);
            max_len = PHY_REG_SIZE;
        }
#endif //CONFIG_WLAN_HAL
        
#ifdef CONFIG_RTL_8812_SUPPORT //8812 phy
		if (GET_CHIP_VER(priv)==VERSION_8812E) {
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_buf;
			max_len = PHY_REG_SIZE;

			if (IS_TEST_CHIP(priv)){ //for_8812_mp_chip
				printk("[%s][PHY_REG_8812]\n",__FUNCTION__);
				next_head = data_PHY_REG_8812_start;
				read_bytes = (int)(data_PHY_REG_8812_end - data_PHY_REG_8812_start);
			}
			else{
#ifdef HIGH_POWER_EXT_PA //FOR_8812_HP
				if( priv->pshare->rf_ft_var.use_ext_pa){
					 printk("[%s][PHY_REG_8812_n_hp]\n",__FUNCTION__);
					 next_head = data_PHY_REG_8812_n_hp_start;
					 read_bytes = (int)(data_PHY_REG_8812_n_hp_end - data_PHY_REG_8812_n_hp_start);
				}
				else
#endif
				{
					printk("[%s][PHY_REG_8812_n_default]\n",__FUNCTION__);
					next_head = data_PHY_REG_8812_n_default_start;
					read_bytes = (int)(data_PHY_REG_8812_n_default_end - data_PHY_REG_8812_n_default_start);
				}
			}
		}
#endif
	}

	{
		if((mem_ptr = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
			printk("PHY_ConfigBBWithParaFile(): not enough memory\n");
			return -1;
		}

		memset(mem_ptr, 0, MAX_CONFIG_FILE_SIZE); // clear memory

		memcpy(mem_ptr, next_head, read_bytes);

		next_head = mem_ptr;
		while (1) {
			line_head = next_head;
			next_head = get_line(&line_head);
			if (line_head == NULL)
				break;

			if (line_head[0] == '/')
				continue;

			if (file_format == TWO_COLUMN) {
				num = get_offset_val(line_head, &u4bRegOffset, &u4bRegValue);
				if (num > 0) {
					phyreg_table[len].offset = u4bRegOffset;
					phyreg_table[len].value = u4bRegValue;
					len++;

#if defined(CONFIG_RTL865X_WTDOG) || defined(CONFIG_RTL_WTDOG)
#if (defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)) && defined(CONFIG_RTL_92D_SUPPORT)
					if ((len&0x7ff)==0)
						REG32(BSP_WDTCNR) |=  1 << 23;
#endif
#endif
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
					if ((GET_CHIP_VER(priv)==VERSION_8188E) || (GET_CHIP_VER(priv)==VERSION_8812E) || (IS_HAL_CHIP(priv))) {
						if (u4bRegOffset == 0xffff)
							break;
					} else
#endif
					{
						if (u4bRegOffset == 0xff)
							break;
					}
					if ((len * sizeof(struct PhyRegTable)) > max_len)
						break;
				}
			} else {
				num = get_offset_mask_val(line_head, &u4bRegOffset, &u4bRegMask ,&u4bRegValue);
				if (num > 0) {
					macreg_table[len].offset = u4bRegOffset;
					macreg_table[len].mask = u4bRegMask;
					macreg_table[len].value = u4bRegValue;
					len++;
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
					if ((GET_CHIP_VER(priv)==VERSION_8188E) || (GET_CHIP_VER(priv)==VERSION_8812E) || (IS_HAL_CHIP(priv))) {
						if (u4bRegOffset == 0xffff)
							break;
					} else
#endif
					{
						if (u4bRegOffset == 0xff)
							break;
					}
					if ((len * sizeof(struct MacRegTable)) > max_len)
						break;

#if defined(CONFIG_RTL865X_WTDOG) || defined(CONFIG_RTL_WTDOG)
#if (defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)) && defined(CONFIG_RTL_92D_SUPPORT)
					if ((len&0x7ff)==0)
						REG32(BSP_WDTCNR) |=  1 << 23;
#endif
#endif
				}
			}
		}

		kfree(mem_ptr);

		if ((len * sizeof(struct PhyRegTable)) > max_len) {

			printk("PHY REG table buffer not large enough!\n");

			return -1;
		}
	}

	num = 0;
	while (1) {
		if (file_format == THREE_COLUMN) {
			u4bRegOffset = macreg_table[num].offset;
			u4bRegValue = macreg_table[num].value;
			u4bRegMask = macreg_table[num].mask;
		}
		else {
			u4bRegOffset = phyreg_table[num].offset;
			u4bRegValue = phyreg_table[num].value;
		}

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
		if ((GET_CHIP_VER(priv)==VERSION_8188E) || (GET_CHIP_VER(priv)==VERSION_8812E) || (IS_HAL_CHIP(priv))) {
			if (u4bRegOffset == 0xffff)
				break;
		} else
#endif
		{
			if (u4bRegOffset == 0xff)
				break;
		}

		if (file_format == THREE_COLUMN){
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
			if (reg_file == PHYREG_PG && (
#ifdef CONFIG_RTL_92D_SUPPORT
				(GET_CHIP_VER(priv)==VERSION_8192D) 
#endif
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
#ifdef CONFIG_RTL_92D_SUPPORT
				|| 
#endif
				(GET_CHIP_VER(priv)==VERSION_8188E) || (GET_CHIP_VER(priv)==VERSION_8812E) || (IS_HAL_CHIP(priv))
#endif
				)) {
				if (u4bRegOffset==0xe00){
					if (idx == pg_tbl_idx)
						write_en=1;
					idx++;
				}
				if (write_en){
					PHY_SetBBReg(priv, u4bRegOffset, u4bRegMask, u4bRegValue);
					if (u4bRegOffset==0x868){
						write_en = 0;
						break;
					}
				}
			}else
#endif
			{
				PHY_SetBBReg(priv, u4bRegOffset, u4bRegMask, u4bRegValue);
			}
		}
		else
		{
			unsigned int tmp = RTL_R32(u4bRegOffset);
			//PHY_SetBBReg(priv, u4bRegOffset, bMaskDWord, u4bRegValue);

			if(tmp != u4bRegValue)
			{
				printk("[0x%x] 0x%08x 0x%08x \n", u4bRegOffset, u4bRegValue, tmp);
				PHY_SetBBReg(priv, u4bRegOffset, bMaskDWord, u4bRegValue);
			}
		}
		num++;
	}

	return 0;
}


static void phy_BB8192CD_Check_ParaFile(struct rtl8192cd_priv *priv)
{
	int rtStatus=0;
	unsigned short val16;
	unsigned int val32;

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A)
	if ((GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8881A))
		rtStatus = PHY_CheckBBWithParaFile(priv, PHYREG);
#endif
}

#endif

#ifdef CONFIG_WLAN_HAL
// TODO: Filen, comfirm register setting below
static int phy_BB88XX_Config_ParaFile(struct rtl8192cd_priv *priv)
{
	int rtStatus=0;
	unsigned short val16;
	unsigned int val32;

	phy_InitBBRFRegisterDefinition(priv);

	// Enable BB and RF
	val16 = RTL_R16(REG_SYS_FUNC_EN);
	RTL_W16(REG_SYS_FUNC_EN, val16|BIT(13)|BIT(0)|BIT(1));

	RTL_W8(REG_RF_CTRL, RF_EN|RF_RSTB|RF_SDMRSTB);

    if (GET_CHIP_VER(priv) == VERSION_8192E) {
        //RTL_W32(REG_AFE_XTAL_CTRL, 0x350f81fb);
        RTL_W32(REG_AFE_XTAL_CTRL, 0x000f81fb);
    }
    else {
    	val32 = RTL_R32(REG_AFE_XTAL_CTRL);
    	val32 = (val32 & (~(BIT(11) | BIT(14)))) | (BIT(18) | BIT(19) | BIT(21) | BIT(22));
    	RTL_W32(REG_AFE_XTAL_CTRL, val32);
	}
    
	/*----Check chip ID and hw TR MIMO config----*/
//	check_chipID_MIMO(priv);

#ifdef CONFIG_WLAN_HAL
    if ( IS_HAL_CHIP(priv) ) {

#ifdef CONFIG_WLAN_HAL_8192EE
        if (GET_CHIP_VER(priv) == VERSION_8192E) {
#if defined(CONFIG_MACBBRF_BY_ODM)
            printk("%s(%d), HAL PHY_ConfigBBWithParaFile\n", __FUNCTION__, __LINE__);
            rtStatus = ODM_ConfigBBWithHeaderFile(ODMPTR, CONFIG_BB_PHY_REG);
#else
            printk("%s(%d), HAL PHY_ConfigBBWithParaFile\n", __FUNCTION__, __LINE__);
            rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG);
#endif            
        }
#endif //CONFIG_WLAN_HAL_8192EE        

#ifdef CONFIG_WLAN_HAL_8881A
        if (GET_CHIP_VER(priv) == VERSION_8881A) {
    		printk("%s(%d), HAL PHY_ConfigBBWithParaFile\n", __FUNCTION__, __LINE__);
    		rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG);
        }
#endif //CONFIG_WLAN_HAL_8881A        
    }
#endif  //CONFIG_WLAN_HAL

#if 1   //Filen, BB have no release these files
#ifdef MP_TEST
	if (priv->pshare->rf_ft_var.mp_specific) {
		delay_ms(10);
		rtStatus |= PHY_ConfigBBWithParaFile(priv, PHYREG_MP);
	}
#endif

	if (rtStatus) {
		printk("phy_BB88XX_Config_ParaFile(): PHYREG_MP Reg Fail!!\n");
		return rtStatus;
	}

	/*----If EEPROM or EFUSE autoload OK, We must config by PHY_REG_PG.txt----*/
	rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG_PG);
	if(rtStatus){
		printk("phy_BB88XX_Config_ParaFile():BB_PG Reg Fail!!\n");
		return rtStatus;
	}
#endif    

	/*----BB AGC table Initialization----*/
	rtStatus = PHY_ConfigBBWithParaFile(priv, AGCTAB);

	if (rtStatus) {
		printk("phy_BB8192CD_Config_ParaFile(): Write BB AGC Table Fail!!\n");
		return rtStatus;
	}

#ifdef CONFIG_WLAN_HAL_8192EE
    if(GET_CHIP_VER(priv)==VERSION_8192E)
    {
        PHY_SetBBReg(priv, rFPGA0_RFMOD, bOFDMEn, 1);
        PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, 1);
    }
#endif //CONFIG_WLAN_HAL_8192EE     

	DEBUG_INFO("PHY-BB Initialization Success\n");

	return 0;
}
#endif  //#ifdef CONFIG_WLAN_HAL

static int phy_BB8192CD_Config_ParaFile(struct rtl8192cd_priv *priv)
{
	int rtStatus=0;
	unsigned short val16;
	unsigned int val32;

	phy_InitBBRFRegisterDefinition(priv);

	// Enable BB and RF
	val16 = RTL_R16(REG_SYS_FUNC_EN);
	RTL_W16(REG_SYS_FUNC_EN, val16|BIT(13)|BIT(0)|BIT(1));

	// 20090923 Joseph: Advised by Steven and Jenyu. Power sequence before init RF.
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
	if (
#ifdef CONFIG_RTL_92C_SUPPORT
		(GET_CHIP_VER(priv)==VERSION_8192C) || (GET_CHIP_VER(priv)==VERSION_8188C) 
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
#ifdef CONFIG_RTL_92C_SUPPORT
		|| 
#endif
		(GET_CHIP_VER(priv)==VERSION_8192D)
#endif
		) {
		RTL_W8(REG_AFE_PLL_CTRL, 0x83);
		RTL_W8(REG_AFE_PLL_CTRL+1, 0xdb);
	}
#endif

	RTL_W8(REG_RF_CTRL, RF_EN|RF_RSTB|RF_SDMRSTB);

#ifdef CONFIG_RTL_8812_SUPPORT 
	if (GET_CHIP_VER(priv) == VERSION_8812E)
		RTL_W8(0x76, 0x7); //enable RF Path B
#endif

	//RTL_W8(REG_SYS_FUNC_EN, FEN_PPLL|FEN_PCIEA|FEN_DIO_PCIE|FEN_USBA|FEN_BB_GLB_RST|FEN_BBRSTB);
	//RTL_W8(REG_LDOHCI12_CTRL, 0x1f);
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
	if (
#ifdef CONFIG_RTL_92C_SUPPORT
		(GET_CHIP_VER(priv)==VERSION_8192C) || (GET_CHIP_VER(priv)==VERSION_8188C) 
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
#ifdef CONFIG_RTL_92C_SUPPORT
		|| 
#endif
		(GET_CHIP_VER(priv)==VERSION_8192D)
#endif
		)
		RTL_W8(REG_AFE_XTAL_CTRL+1, 0x80);
#endif

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv) != VERSION_8188E)
#endif
	{
	val32 = RTL_R32(REG_AFE_XTAL_CTRL);
	val32 = (val32 & (~(BIT(11) | BIT(14)))) | (BIT(18) | BIT(19) | BIT(21) | BIT(22));
	RTL_W32(REG_AFE_XTAL_CTRL, val32);
	}
	/*----Check chip ID and hw TR MIMO config----*/
//	check_chipID_MIMO(priv);

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8192D) {
		printk(">>> 92D load PHYREG \n");
		rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG);
	}
#endif
#ifdef CONFIG_RTL_92C_SUPPORT
	if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)){
		if (get_rf_mimo_mode(priv) == MIMO_2T2R)
			rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG_2T2R);
		else if (get_rf_mimo_mode(priv) == MIMO_1T1R)
			rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG_1T1R);
	}
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8188E) {
#if defined(CONFIG_MACBBRF_BY_ODM)
		rtStatus = ODM_ConfigBBWithHeaderFile(ODMPTR, CONFIG_BB_PHY_REG);
#else
		rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG_1T1R);
#endif
	}
#endif

#ifdef CONFIG_RTL_8812_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8812E)
		rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG);
#endif

#ifdef MP_TEST
	if ((priv->pshare->rf_ft_var.mp_specific)
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
		&& (
#ifdef CONFIG_RTL_92C_SUPPORT
		!IS_TEST_CHIP(priv)
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
#ifdef CONFIG_RTL_92C_SUPPORT
		|| 
#endif
		(GET_CHIP_VER(priv)==VERSION_8192D)
#endif
		)
#endif
		) {
		delay_ms(10);
		rtStatus |= PHY_ConfigBBWithParaFile(priv, PHYREG_MP);
	}
#endif

	if (rtStatus) {
		printk("phy_BB8192CD_Config_ParaFile(): Write BB Reg Fail!!\n");
		return rtStatus;
	}

	/*----If EEPROM or EFUSE autoload OK, We must config by PHY_REG_PG.txt----*/
	rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG_PG);
	if(rtStatus){
		printk("phy_BB8192CD_Config_ParaFile():BB_PG Reg Fail!!\n");
		return rtStatus;
	}

	/*----BB AGC table Initialization----*/
#ifdef CONFIG_RTL_92D_SUPPORT

    if (GET_CHIP_VER(priv)==VERSION_8192D)
	PHY_SetBBReg(priv, rFPGA0_RFMOD, bOFDMEn | bCCKEn, 0);
#endif

	if (GET_CHIP_VER(priv)==VERSION_8188E)  {
#if defined(CONFIG_MACBBRF_BY_ODM) && defined(CONFIG_RTL_88E_SUPPORT)
		rtStatus = ODM_ConfigBBWithHeaderFile(ODMPTR, CONFIG_BB_AGC_TAB);
#else
		rtStatus = PHY_ConfigBBWithParaFile(priv, AGCTAB);
#endif
	} else {
		rtStatus = PHY_ConfigBBWithParaFile(priv, AGCTAB);
	}


#ifdef CONFIG_RTL_8812_SUPPORT
	if(GET_CHIP_VER(priv)!=VERSION_8812E)
#endif
	{
		PHY_SetBBReg(priv, rFPGA0_RFMOD, bOFDMEn, 1);
#ifdef CONFIG_RTL_92D_SUPPORT
		if (!(priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G))

#endif
			PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, 1);
	}

	if (rtStatus) {
		printk("phy_BB8192CD_Config_ParaFile(): Write BB AGC Table Fail!!\n");
		return rtStatus;
	}

#if 0
	/*----For 1T2R Config----*/
	if (get_rf_mimo_mode(priv) == MIMO_1T2R) {
		rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG_1T2R);
		if (rtStatus) {
			printk("phy_BB8192CD_Config_ParaFile(): Write BB Reg for 1T2R Fail!!\n");
			return rtStatus;
		}
	}else if (get_rf_mimo_mode(priv) == MIMO_1T1R){
		delay_ms(100);
		rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG_1T1R);
		if (rtStatus) {
			printk("phy_BB8192CD_Config_ParaFile(): Write BB Reg for 1T1R Fail!!\n");
			return rtStatus;
		}
	}
#endif

	DEBUG_INFO("PHY-BB Initialization Success\n");
	return 0;
}

#if 1
//#if !defined(CONFIG_MACBBRF_BY_ODM) || !defined(CONFIG_RTL_88E_SUPPORT)	

int phy_RF6052_Config_ParaFile(struct rtl8192cd_priv *priv)
{
	int rtStatus=0;
	RF92CD_RADIO_PATH_E eRFPath;
	BB_REGISTER_DEFINITION_T *pPhyReg;
	unsigned int  u4RegValue = 0;

#ifdef CONFIG_RTL_92D_SUPPORT
	if ((GET_CHIP_VER(priv)==VERSION_8192D) && (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY))
		priv->pshare->phw->NumTotalRFPath = 1;
	else
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E)
		priv->pshare->phw->NumTotalRFPath = 1;
	else
#endif
#ifdef CONFIG_WLAN_HAL
    if ( IS_HAL_CHIP(priv)) {
        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_NUM_TOTAL_RF_PATH, (pu1Byte)&priv->pshare->phw->NumTotalRFPath);
    }
    else
#endif //CONFIG_WLAN_HAL
		priv->pshare->phw->NumTotalRFPath = 2;

	for (eRFPath = RF92CD_PATH_A; eRFPath<priv->pshare->phw->NumTotalRFPath; eRFPath++)
	{
		pPhyReg = &priv->pshare->phw->PHYRegDef[eRFPath];

		/*----Store original RFENV control type----*/
		switch(eRFPath)
		{
		case RF92CD_PATH_A:
			u4RegValue = PHY_QueryBBReg(priv, pPhyReg->rfintfs, bRFSI_RFENV);
			break;
		case RF92CD_PATH_B :
			u4RegValue = PHY_QueryBBReg(priv, pPhyReg->rfintfs, bRFSI_RFENV<<16);
			break;
		case RF92CD_PATH_MAX:
			break;
		}

		/*----Set RF_ENV enable----*/
		PHY_SetBBReg(priv, pPhyReg->rfintfe, bRFSI_RFENV<<16, 0x1);

		/*----Set RF_ENV output high----*/
		PHY_SetBBReg(priv, pPhyReg->rfintfo, bRFSI_RFENV, 0x1);

		/* Set bit number of Address and Data for RF register */
		PHY_SetBBReg(priv, pPhyReg->rfHSSIPara2, b3WireAddressLength, 0x0);
		PHY_SetBBReg(priv, pPhyReg->rfHSSIPara2, b3WireDataLength, 0x0);

		/*----Initialize RF fom connfiguration file----*/
#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8192D) {
		  	switch (eRFPath) {
			case RF92CD_PATH_A:
#ifdef CONFIG_RTL_92D_DMDP
				if ((priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY) &&
					(priv->pshare->wlandev_idx==1)){
#ifdef RTL8192D_INT_PA
					if (priv->pshare->rf_ft_var.use_intpa92d){
#ifdef USB_POWER_SUPPORT

#if defined (RTL8192D_INT_PA_GAIN_TABLE_NEW)
						printk("[%s][radio_b_intPA_GM_new]\n",__FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_intPA_GM_new_start,
							(int)(data_radio_b_intPA_GM_new_end - data_radio_b_intPA_GM_new_start), eRFPath);										
#elif defined (RTL8192D_INT_PA_GAIN_TABLE_NEW1)
						printk("[%s][radio_b_intPA_GM_new1]\n",__FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_intPA_GM_new1_start,
							(int)(data_radio_b_intPA_GM_new1_end - data_radio_b_intPA_GM_new1_start), eRFPath);						
#else
						printk("[%s][radio_b_intPA_GM]\n",__FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_intPA_GM_start,
							(int)(data_radio_b_intPA_GM_end - data_radio_b_intPA_GM_start), eRFPath);
#endif
						
#else //USB_POWER_SUPPORT

#if defined (RTL8192D_INT_PA_GAIN_TABLE_NEW)
						printk("[%s][radio_b_intPA_new]\n",__FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_intPA_new_start,
							(int)(data_radio_b_intPA_new_end - data_radio_b_intPA_new_start), eRFPath);
#else
						printk("[%s][radio_b_intPA]\n",__FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_intPA_start,
							(int)(data_radio_b_intPA_end - data_radio_b_intPA_start), eRFPath);
#endif

#endif //USB_POWER_SUPPORT

					} else
#endif
					{
#ifdef HIGH_POWER_EXT_PA
						if (priv->pshare->rf_ft_var.use_ext_pa)
						{
							printk("[%s][radio_b_n_92d_hp]\n",__FUNCTION__);
							rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_n_92d_hp_start,
								(int)(data_radio_b_n_92d_hp_end - data_radio_b_n_92d_hp_start), eRFPath);
						}
						else
#endif
						{
							printk("[%s] [radio_b_n]\n",__FUNCTION__);
							rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_n_start,
								(int)(data_radio_b_n_end - data_radio_b_n_start), eRFPath);
						}
					}
				} else
#endif
				{
#ifdef RTL8192D_INT_PA
					if (priv->pshare->rf_ft_var.use_intpa92d)
					{
#ifdef USB_POWER_SUPPORT

#if defined (RTL8192D_INT_PA_GAIN_TABLE_NEW)	
						printk("[%s][radio_a_intPA_GM_new]\n",__FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_intPA_GM_new_start,
							(int)(data_radio_a_intPA_GM_new_end - data_radio_a_intPA_GM_new_start), eRFPath);						
#elif defined (RTL8192D_INT_PA_GAIN_TABLE_NEW1)
						printk("[%s][radio_a_intPA_GM_new1]\n",__FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_intPA_GM_new1_start,
							(int)(data_radio_a_intPA_GM_new1_end - data_radio_a_intPA_GM_new1_start), eRFPath);						
#else
						printk("[%s][radio_a_intPA_GM]\n",__FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_intPA_GM_start,
							(int)(data_radio_a_intPA_GM_end - data_radio_a_intPA_GM_start), eRFPath);
#endif

#else //USB_POWER_SUPPORT

#if defined (RTL8192D_INT_PA_GAIN_TABLE_NEW)

						printk("[%s][radio_a_intPA_new]\n",__FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_intPA_new_start,
							(int)(data_radio_a_intPA_new_end - data_radio_a_intPA_new_start), eRFPath);

#else
						printk("[%s][radio_a_intPA]\n",__FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_intPA_start,
							(int)(data_radio_a_intPA_end - data_radio_a_intPA_start), eRFPath);
#endif

#endif //USB_POWER_SUPPORT
					} else
#endif
					{
//_TXPWR_REDEFINE
#ifdef HIGH_POWER_EXT_PA
						if (priv->pshare->rf_ft_var.use_ext_pa)
						{
							printk("[%s][radio_a_n_92d_hp]\n",__FUNCTION__);
							rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_n_92d_hp_start,
								(int)(data_radio_a_n_92d_hp_end - data_radio_a_n_92d_hp_start), eRFPath);
						}
						else
#endif
						{
							printk("[%s][radio_a_n]\n",__FUNCTION__);
							rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_n_start,
								(int)(data_radio_a_n_end - data_radio_a_n_start), eRFPath);
						}
					}
				}
				break;
			case RF92CD_PATH_B:
#ifdef RTL8192D_INT_PA
				if (priv->pshare->rf_ft_var.use_intpa92d){
#ifdef USB_POWER_SUPPORT

#if defined (RTL8192D_INT_PA_GAIN_TABLE_NEW)	
					printk("[%s][radio_b_intPA_GM_new]\n",__FUNCTION__);
					rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_intPA_GM_new_start,
						(int)(data_radio_b_intPA_GM_new_end - data_radio_b_intPA_GM_new_start), eRFPath);					
#elif defined (RTL8192D_INT_PA_GAIN_TABLE_NEW1)
					printk("[%s][radio_b_intPA_GM_new1]\n",__FUNCTION__);
					rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_intPA_GM_new1_start,
						(int)(data_radio_b_intPA_GM_new1_end - data_radio_b_intPA_GM_new1_start), eRFPath);					
#else
					printk("[%s][radio_b_intPA_GM]\n",__FUNCTION__);
					rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_intPA_GM_start,
						(int)(data_radio_b_intPA_GM_end - data_radio_b_intPA_GM_start), eRFPath);
#endif


#else //USB_POWER_SUPPORT

#if defined (RTL8192D_INT_PA_GAIN_TABLE_NEW)	
					printk("[%s][radio_b_intPA_new]\n",__FUNCTION__);
					rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_intPA_new_start,
						(int)(data_radio_b_intPA_new_end - data_radio_b_intPA_new_start), eRFPath);
#else
					printk("[%s][radio_b_intPA]\n",__FUNCTION__);
					rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_intPA_start,
						(int)(data_radio_b_intPA_end - data_radio_b_intPA_start), eRFPath);
#endif

#endif //USB_POWER_SUPPORT
				} else
#endif
				{
#ifdef HIGH_POWER_EXT_PA
					if (priv->pshare->rf_ft_var.use_ext_pa)
					{
						printk("[%s][radio_b_n_92d_hp]\n",__FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_n_92d_hp_start,
							(int)(data_radio_b_n_92d_hp_end - data_radio_b_n_92d_hp_start), eRFPath);
					}
					else
#endif
					{
						printk("[%s][radio_b_n]\n",__FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_n_start,
							(int)(data_radio_b_n_end - data_radio_b_n_start), eRFPath);
					}
				}
				break;
			default:
				break;
		  	}
		}
#endif //!CONFIG_RTL_92D_SUPPORT
#if defined(CONFIG_RTL_92C_SUPPORT)
	if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)) {
		switch (eRFPath)
		{
		case RF92CD_PATH_A:
			if (get_rf_mimo_mode(priv) == MIMO_2T2R) {
#ifdef TESTCHIP_SUPPORT
				if (IS_TEST_CHIP(priv))
					rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_2T_start,
								(int)(data_radio_a_2T_end - data_radio_a_2T_start), eRFPath);
				else
#endif
				{
#ifdef HIGH_POWER_EXT_PA
					if (priv->pshare->rf_ft_var.use_ext_pa)
					{
						//printk("[%s][data_radio_a_2T_n_hp]\n", __FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_2T_n_hp_start,
										(int)(data_radio_a_2T_n_hp_end - data_radio_a_2T_n_hp_start), eRFPath);
					}
					else
#endif
					{
					if (priv->pshare->rf_ft_var.use_ext_lna)
						{
							//printk("[%s][data_radio_a_2T_n_lna]\n", __FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_2T_n_lna_start,
										(int)(data_radio_a_2T_n_lna_end - data_radio_a_2T_n_lna_start), eRFPath);
						}
					else
						{
							//printk("[%s][data_radio_a_2T_n]\n", __FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_2T_n_start,
										(int)(data_radio_a_2T_n_end - data_radio_a_2T_n_start), eRFPath);
				}
					}
				}
			} else if (get_rf_mimo_mode(priv) == MIMO_1T1R)	{
#ifdef TESTCHIP_SUPPORT
				if (IS_TEST_CHIP(priv))
					rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_1T_start,
									(int)(data_radio_a_1T_end - data_radio_a_1T_start), eRFPath);
				else
#endif
				{
#ifdef HIGH_POWER_EXT_PA
						if (priv->pshare->rf_ft_var.use_ext_pa)
						{
							//printk("[%s][data_radio_a_2T_n_hp]\n", __FUNCTION__);
							rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_2T_n_hp_start,
											(int)(data_radio_a_2T_n_hp_end - data_radio_a_2T_n_hp_start), eRFPath);
						}
						else
#endif
						{
							//printk("[%s][data_radio_a_1T_n]\n", __FUNCTION__);
							rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_1T_n_start,
									(int)(data_radio_a_1T_n_end - data_radio_a_1T_n_start), eRFPath);
						}

				}
			}
			break;
		case RF92CD_PATH_B:
			if (get_rf_mimo_mode(priv) == MIMO_2T2R) {
#ifdef TESTCHIP_SUPPORT
				if (IS_TEST_CHIP(priv))
					rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_2T_start,
								(int)(data_radio_b_2T_end - data_radio_b_2T_start), eRFPath);
				else
#endif
				{
#ifdef HIGH_POWER_EXT_PA
					if (priv->pshare->rf_ft_var.use_ext_pa)
					{
						//printk("[%s][data_radio_b_2T_n_hp]\n", __FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_2T_n_hp_start,
									(int)(data_radio_b_2T_n_hp_end - data_radio_b_2T_n_hp_start), eRFPath);
					}
					else
#endif
					{
					if (priv->pshare->rf_ft_var.use_ext_lna)
						{
							//printk("[%s][data_radio_b_2T_n_lna]\n", __FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_2T_n_lna_start,
										(int)(data_radio_b_2T_n_lna_end - data_radio_b_2T_n_lna_start), eRFPath);
						}
					else
						{
							//printk("[%s][data_radio_b_2T_n]\n", __FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_2T_n_start,
									(int)(data_radio_b_2T_n_end - data_radio_b_2T_n_start), eRFPath);
				}
					}
				}
			} else if (get_rf_mimo_mode(priv) == MIMO_1T1R)
				rtStatus=0;
			break;
		default:
			break;
		}
	}
#endif

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8188E) {
		switch (eRFPath) {
		case RF92CD_PATH_A:
			if (get_rf_mimo_mode(priv) == MIMO_1T1R) {
#ifdef SUPPORT_RTL8188E_TC
				if (IS_TEST_CHIP(priv)) {
					DEBUG_INFO("[%s][radio_a_1T_88E_TC]\n",__FUNCTION__);
					rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_1T_88E_TC_start,
					(int)(data_radio_a_1T_88E_TC_end - data_radio_a_1T_88E_TC_start), eRFPath);
				} else
#endif
				{
					DEBUG_INFO("[%s][radio_a_1T_88E]\n",__FUNCTION__);
					rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_1T_88E_start,
					(int)(data_radio_a_1T_88E_end - data_radio_a_1T_88E_start), eRFPath);
				}
			}
			break;
		default:
			DEBUG_ERR("%s Line %d, wrong Ant settings\n", __FUNCTION__, __LINE__);
			break;
		}
	}
#endif

#ifdef CONFIG_RTL_8812_SUPPORT //data radio
	if (GET_CHIP_VER(priv) == VERSION_8812E) {
		switch (eRFPath) {
			case RF92CD_PATH_A://for_8812_mp_chip
				if (IS_TEST_CHIP(priv)){
#ifdef HIGH_POWER_EXT_PA //FOR_8812_HP
				if( priv->pshare->rf_ft_var.use_ext_pa)	
				{
					printk("[%s][RadioA_8812_hp]\n",__FUNCTION__);
					rtStatus = PHY_ConfigRFWithParaFile(priv, data_RadioA_8812_hp_start,
					(int)(data_RadioA_8812_hp_end - data_RadioA_8812_hp_start), eRFPath);
				}
				else
#endif
				{
				printk("[%s][RadioA_8812]\n",__FUNCTION__);
				rtStatus = PHY_ConfigRFWithParaFile(priv, data_RadioA_8812_start,
				(int)(data_RadioA_8812_end - data_RadioA_8812_start), eRFPath);
				}
				}
				else{
#ifdef HIGH_POWER_EXT_PA //FOR_8812_HP
					if( priv->pshare->rf_ft_var.use_ext_pa)	
					{
						printk("[%s][RadioA_8812_n_hp]\n",__FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_RadioA_8812_n_hp_start,
						(int)(data_RadioA_8812_n_hp_end - data_RadioA_8812_n_hp_start), eRFPath);
					}
					else
#endif
					{
						printk("[%s][RadioA_8812_n_default]\n",__FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_RadioA_8812_n_default_start,
						(int)(data_RadioA_8812_n_default_end - data_RadioA_8812_n_default_start), eRFPath);
					}
				}
				break;
			case RF92CD_PATH_B:
 				if (IS_TEST_CHIP(priv)){
#ifdef HIGH_POWER_EXT_PA //FOR_8812_HP
				if( priv->pshare->rf_ft_var.use_ext_pa) 
				{
					printk("[%s][RadioB_8812_hp]\n",__FUNCTION__);
					rtStatus = PHY_ConfigRFWithParaFile(priv, data_RadioB_8812_hp_start,
					(int)(data_RadioB_8812_hp_end - data_RadioB_8812_hp_start), eRFPath);
				}
				else
#endif
				{
				printk("[%s][RadioB_8812]\n",__FUNCTION__);
				rtStatus = PHY_ConfigRFWithParaFile(priv, data_RadioB_8812_start,
				(int)(data_RadioB_8812_end - data_RadioB_8812_start), eRFPath);
				}
				}
				else{
#ifdef HIGH_POWER_EXT_PA //FOR_8812_HP
					if( priv->pshare->rf_ft_var.use_ext_pa)	
					{
						printk("[%s][RadioB_8812_n_hp]\n",__FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_RadioB_8812_n_hp_start,
						(int)(data_RadioB_8812_n_hp_end - data_RadioB_8812_n_hp_start), eRFPath);
					}
					else
#endif
					{
						printk("[%s][RadioB_8812_n_default]\n",__FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_RadioB_8812_n_default_start,
						(int)(data_RadioB_8812_n_default_end - data_RadioB_8812_n_default_start), eRFPath);
					}
				}
				break;
			}
		}
#endif

#ifdef CONFIG_WLAN_HAL
        if (IS_HAL_CHIP(priv)) {
            pu4Byte FileStartPtr;
            u4Byte  FileLength;
     
            switch (eRFPath) {
                case RF92CD_PATH_A:
#ifdef HIGH_POWER_EXT_PA
                    #if 0    
                    if( priv->pshare->rf_ft_var.use_ext_pa) 
                    {
                        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_A_HP_SIZE, (pu1Byte)&FileLength);
                        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_A_HP_START, &FileStartPtr);
                        printk("[%s][RadioA_HAL_hp]\n",__FUNCTION__);
                        rtStatus = PHY_ConfigRFWithParaFile(priv, FileStartPtr,
                        (int)FileLength, eRFPath);
                    }
                    else
                    #endif    
#endif
                    {
                    GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_A_SIZE, (pu1Byte)&FileLength);
                    GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_A_START, &FileStartPtr);
                    printk("[%s][RadioA_HAL]\n",__FUNCTION__);
                    rtStatus = PHY_ConfigRFWithParaFile(priv, FileStartPtr, (int)FileLength, eRFPath);
                    }
                    break;
                case RF92CD_PATH_B:
#ifdef HIGH_POWER_EXT_PA
                    #if 0
                    if( priv->pshare->rf_ft_var.use_ext_pa) 
                    {
                        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_B_HP_SIZE, (pu1Byte)&FileLength);
                        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_B_HP_START, &FileStartPtr);
                        printk("[%s][RadioB_HAL_hp]\n",__FUNCTION__);
                        rtStatus = PHY_ConfigRFWithParaFile(priv, FileStartPtr,
                        (int)FileLength, eRFPath);
                    }
                    else
                    #endif    
#endif
                    {
                    GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_B_SIZE, (pu1Byte)&FileLength);
                    GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_B_START, &FileStartPtr);                        
                    printk("[%s][RadioB_HAL]\n",__FUNCTION__);
                    rtStatus = PHY_ConfigRFWithParaFile(priv, FileStartPtr,
                    (int)FileLength, eRFPath);
                    }
                    break;
                }
            }
#endif



		/*----Restore RFENV control type----*/;
		switch(eRFPath)
		{
		case RF92CD_PATH_A:
			PHY_SetBBReg(priv, pPhyReg->rfintfs, bRFSI_RFENV, u4RegValue);
			break;
		case RF92CD_PATH_B :
			PHY_SetBBReg(priv, pPhyReg->rfintfs, bRFSI_RFENV<<16, u4RegValue);
			break;
		case RF92CD_PATH_MAX:
			break;
		}
	}

	DEBUG_INFO("PHY-RF Initialization Success\n");

	return rtStatus;
}
#endif

//
// Description:
//	Set the MAC offset [0x09] and prevent all I/O for a while (about 20us~200us, suggested from SD4 Scott).
//	If the protection is not performed well or the value is not set complete, the next I/O will cause the system hang.
// Note:
//	This procudure is designed specifically for 8192S and references the platform based variables
//	which violates the stucture of multi-platform.
//	Thus, we shall not extend this procedure to common handler.
// By Bruce, 2009-01-08.
//
unsigned char
HalSetSysClk8192CD(	struct rtl8192cd_priv *priv,	unsigned char Data)
{
	RTL_W8((SYS_CLKR + 1), Data);
	delay_us(200);
	return TRUE;
}


#ifdef CONFIG_RTL_8812_SUPPORT

#define MAX_RX_DMA_BUFFER_SIZE_8812	0x3E80   //0x3FFF	// RX 16K


static void LLT_table_init_8812(struct rtl8192cd_priv *priv)
{
	unsigned int	i, maxPage = 255;
	unsigned int	count = 0;
	unsigned int	value32;	//High+low page number
	unsigned char   txpktbuf_bndy = 0xfc, bufBd = 0xff;
	

	printk("=====>LLT_table_init_8812\n");

	// 12.	TXRKTBUG_PG_BNDY 0x114[31:0] = 0x27FF00F6	//TXRKTBUG_PG_BNDY	
	RTL_W8(TRXFF_BNDY, txpktbuf_bndy);
	
	RTL_W16(TRXFF_BNDY+2, MAX_RX_DMA_BUFFER_SIZE_8812-1);

	// 13.	TDECTRL[15:8] 0x209[7:0] = 0xF6				// Beacon Head for TXDMA
	RTL_W8(TDECTRL+1, txpktbuf_bndy);

	// 14.	BCNQ_PGBNDY 0x424[7:0] =  0xF6				//BCNQ_PGBNDY
	// 2009/12/03 Why do we set so large boundary. confilct with document V11.
	RTL_W8(TXPKTBUF_BCNQ_BDNY, txpktbuf_bndy);
	RTL_W8(TXPKTBUF_MGQ_BDNY, txpktbuf_bndy);

	// 15.	WMAC_LBK_BF_HD 0x45D[7:0] =  0xF6			//WMAC_LBK_BF_HD
	RTL_W8(TXPKTBUF_WMAC_LBK_BF_HD, txpktbuf_bndy);
	
	// Set Tx/Rx page size (Tx must be 128 Bytes, Rx can be 64,128,256,512,1024 bytes)
	// 16.	PBP [7:0] = 0x11								// TRX page size
	RTL_W8(PBP, 0x31);		

	// 17.	DRV_INFO_SZ = 0x04
	RTL_W8(RX_DRVINFO_SZ, 0x4);	

	// 18.	LLT_table_init(Adapter); 
	for ( i = 0; i < txpktbuf_bndy-1; i++) {
		RTL_W32(LLT_INI, ((LLTE_RWM_WR&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)|(i&LLTINI_ADDR_Mask)<<LLTINI_ADDR_SHIFT
			|((i+1)&LLTINI_HDATA_Mask)<<LLTINI_HDATA_SHIFT);

		count = 0;
		do {
			if (!(RTL_R32(LLT_INI) & ((LLTE_RWM_RD&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)))
				break;
			if (++count >= 100) {
				printk("LLT_init, section 01, i=%d\n", i);
				printk("LLT Polling failed 01 !!!\n");
				return;
			}
		} while(count < 100);
	}

	RTL_W32(LLT_INI, ((LLTE_RWM_WR&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)
		|((txpktbuf_bndy-1)&LLTINI_ADDR_Mask)<<LLTINI_ADDR_SHIFT|(255&LLTINI_HDATA_Mask)<<LLTINI_HDATA_SHIFT);

	count = 0;
	do {
		if (!(RTL_R32(LLT_INI) & ((LLTE_RWM_RD&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)))
			break;
		if (++count >= 100) {
			printk("LLT Polling failed 02 !!!\n");
			return;
		}
	} while(count < 100);


	for (i = txpktbuf_bndy; i < bufBd; i++) {
		RTL_W32(LLT_INI, ((LLTE_RWM_WR&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)|(i&LLTINI_ADDR_Mask)<<LLTINI_ADDR_SHIFT
			|((i+1)&LLTINI_HDATA_Mask)<<LLTINI_HDATA_SHIFT);

		do{
			if (!(RTL_R32(LLT_INI) & ((LLTE_RWM_RD&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)))
				break;
			if (++count >= 100) {
				printk("LLT Polling failed 03 !!!\n");
				return;
			}
		}while(count < 100);
	}

	RTL_W32(LLT_INI, ((LLTE_RWM_WR&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)|(bufBd&LLTINI_ADDR_Mask)<<LLTINI_ADDR_SHIFT
		|(txpktbuf_bndy&LLTINI_HDATA_Mask)<<LLTINI_HDATA_SHIFT);

	count = 0;
	do {
		if (!(RTL_R32(LLT_INI) & ((LLTE_RWM_RD&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)))
			break;
		if(++count >= 100) {
			printk("LLT Polling failed 04 !!!\n");
			return;
		}
	} while(count < 100);

	
	// Set reserved page for each queue
	// 11.	RQPN 0x200[31:0]	= 0x80BD1C1C				// load RQPN

	if (priv->pmib->dot11OperationEntry.wifi_specific != 0) {
		RTL_W8(RQPN_NPQ, 0x29);
		RTL_W32(RQPN, 0x80a92004);
	}
	else {
		RTL_W32(RQPN, 0x80EB0808);//0x80cb1010);//0x80711010);//0x80cb1010);
		RTL_W8(RQPN_NPQ, 0x0);
	}

	printk("LLT_table_init_8812<=====\n");

	return;
	
}

#endif //CONFIG_RTL_8812_SUPPORT

static void LLT_table_init(struct rtl8192cd_priv *priv)
{

	unsigned int i, count = 0;

#if 1
	unsigned txpktbufSz, bufBd;
#ifdef CONFIG_RTL_92D_DMDP
	if (priv->pmib->dot11RFEntry.macPhyMode != SINGLEMAC_SINGLEPHY) {
		txpktbufSz = 120; //0x7C
		bufBd = 127;
	} else
#endif
	{
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8188E) {
#ifdef DRVMAC_LB
			txpktbufSz = 83; // 0x53
			bufBd = 87;
#else
			txpktbufSz = 171; // 0xAB
			bufBd = 175;
#endif
		} else
#endif
		{
			txpktbufSz = 246; // 0xF6
			bufBd = 255;
		}
	}
#else
	unsigned txpktbufSz = 252; //174(0xAE) 120(0x78) 252(0xFC)
#endif

	for ( i = 0; i < txpktbufSz-1; i++) {
		RTL_W32(LLT_INI, ((LLTE_RWM_WR&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)|(i&LLTINI_ADDR_Mask)<<LLTINI_ADDR_SHIFT
			|((i+1)&LLTINI_HDATA_Mask)<<LLTINI_HDATA_SHIFT);

		count = 0;
		do {
			if (!(RTL_R32(LLT_INI) & ((LLTE_RWM_RD&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)))
				break;
			if (++count >= 100) {
				printk("LLT_init, section 01, i=%d\n", i);
				printk("LLT Polling failed 01 !!!\n");
				return;
			}
		} while(count < 100);
	}

	RTL_W32(LLT_INI, ((LLTE_RWM_WR&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)
		|((txpktbufSz-1)&LLTINI_ADDR_Mask)<<LLTINI_ADDR_SHIFT|(255&LLTINI_HDATA_Mask)<<LLTINI_HDATA_SHIFT);

	count = 0;
	do {
		if (!(RTL_R32(LLT_INI) & ((LLTE_RWM_RD&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)))
			break;
		if (++count >= 100) {
			printk("LLT Polling failed 02 !!!\n");
			return;
		}
	} while(count < 100);


	for (i = txpktbufSz; i < bufBd; i++) {
		RTL_W32(LLT_INI, ((LLTE_RWM_WR&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)|(i&LLTINI_ADDR_Mask)<<LLTINI_ADDR_SHIFT
			|((i+1)&LLTINI_HDATA_Mask)<<LLTINI_HDATA_SHIFT);

		do{
			if (!(RTL_R32(LLT_INI) & ((LLTE_RWM_RD&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)))
				break;
			if (++count >= 100) {
				printk("LLT Polling failed 03 !!!\n");
				return;
			}
		}while(count < 100);
	}

	RTL_W32(LLT_INI, ((LLTE_RWM_WR&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)|(bufBd&LLTINI_ADDR_Mask)<<LLTINI_ADDR_SHIFT
		|(txpktbufSz&LLTINI_HDATA_Mask)<<LLTINI_HDATA_SHIFT);

	count = 0;
	do {
		if (!(RTL_R32(LLT_INI) & ((LLTE_RWM_RD&LLTE_RWM_Mask)<<LLTE_RWM_SHIFT)))
			break;
		if(++count >= 100) {
			printk("LLT Polling failed 04 !!!\n");
			return;
		}
	} while(count < 100);

// Set reserved page for each queue

#if 1
	/* normal queue init MUST be previoius of RQPN enable */
	//RTL_W8(RQPN_NPQ, 4);		//RQPN_NPQ
#ifdef CONFIG_RTL_92D_DMDP
	if (priv->pmib->dot11RFEntry.macPhyMode != SINGLEMAC_SINGLEPHY )
	{
		if (priv->pmib->dot11OperationEntry.wifi_specific == 1) {
			RTL_W8(RQPN_NPQ, 0x29);
			RTL_W32(RQPN, 0x802f1c04);
		} else {
			//RTL_W32(RQPN, 0x80501010);
			RTL_W8(RQPN_NPQ, 0x10);
			//RTL_W32(RQPN, 0x80630410);
			RTL_W32(RQPN, 0x80600404);
		}
	}
	else
#endif
	{
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8188E) {
			//RTL_W8(RQPN_NPQ, 4);
			RTL_W8(RQPN_NPQ, 0x29);
#ifdef DRVMAC_LB
			RTL_W32(RQPN, 0x80460404);
#else
			//RTL_W32(RQPN, 0x805d2029);
			//RTL_W32(RQPN, 0x805d2004);
			if (priv->pmib->dot11OperationEntry.wifi_specific == 1)
				RTL_W32(RQPN, 0x805d2004);
			else
				RTL_W32(RQPN, 0x807d0004);
#endif
		} else
#endif
		{
			RTL_W8(RQPN_NPQ, 0x29);
			//RTL_W32(RQPN, 0x809f2929);
			//RTL_W32(RQPN, 0x80a82029);
#ifdef DRVMAC_LB
			RTL_W8(RQPN_NPQ+2, 0x4);
			RTL_W32(RQPN, 0x80380404);
#else			
			if (priv->pmib->dot11OperationEntry.wifi_specific == 0) {
				RTL_W32(RQPN, 0x80C50404);
			}
			else{
				RTL_W32(RQPN, 0x80a92004);
			}
#endif				
		}
	}
#else
	if(txpktbufSz == 120)
		RTL_W32(RQPN, 0x80272828);
	else if(txpktbufSz == 252)
	{
		//RTL_W32(RQPN, 0x80c31c1c);

		// Joseph test
		//RTL_W32(RQPN, 0x80838484);
		RTL_W32(RQPN, 0x80bd1c1c);
	}
	else
		RTL_W32(RQPN, 0x80393a3a);
#endif

	//RTL_W32(TDECTRL, RTL_R32(TDECTRL)|(txpktbufSz&BCN_HEAD_Mask)<<BCN_HEAD_SHIFT);
	RTL_W8(TXPKTBUF_BCNQ_BDNY, txpktbufSz);
	RTL_W8(TXPKTBUF_MGQ_BDNY, txpktbufSz);
	RTL_W8(TRXFF_BNDY, txpktbufSz);
	RTL_W8(TDECTRL+1, txpktbufSz);

	RTL_W8(0x45D, txpktbufSz);
}


#ifdef CONFIG_RTL_8812_SUPPORT
static void MacInit_8812(struct rtl8192cd_priv *priv)
{
	volatile unsigned char bytetmp;
	volatile unsigned int Qbytetmp;
	unsigned short retry;
	printk("CP: MacInit_8812===>>");

	RTL_W8(RSV_CTRL0, 0x00);

	//Auto Power Down to CHIP-off State
	bytetmp = RTL_R8(APS_FSMCO+1);
	bytetmp &= (~BIT(7)); //PlatformEFIORead1Byte(Adapter, REG_APS_FSMCO_8812+1) & (~BIT7)
	RTL_W8(APS_FSMCO+1, bytetmp);

	printk("_eric HalPwrSeqCmdParsing +++ \n");
	//HW power on sequence
	if (!HalPwrSeqCmdParsing(priv, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, 
				PWR_INTF_PCI_MSK, rtl8812_card_enable_flow))
		panic_printk("%s %d, HalPwrSeqCmdParsing init fail!!!\n", __FUNCTION__, __LINE__);

	printk("_eric HalPwrSeqCmdParsing --- \n");

#if 0
	RTL_W32(0x500, 0x7f);
	RTL_W32(0x504, 0x7f);
	RTL_W32(0x508, 0x7f);
	RTL_W32(0x50c, 0x7f);

	printk("0x%x, 0x%x, 0x%x, 0x%x \n", RTL_R32(0x500), RTL_R32(0x504),RTL_R32(0x508), RTL_R32(0x50c));
#endif

	// Release MAC IO register reset
	RTL_W32(CR, RTL_R32(CR)|MACRXEN|MACTXEN|SCHEDULE_EN|PROTOCOL_EN
		|RXDMA_EN|TXDMA_EN|HCI_RXDMA_EN|HCI_TXDMA_EN);
	delay_ms(2);

	RTL_W8(HWSEQ_CTRL, 0x7f);
	delay_ms(2);


	// Add for wakeup online
	bytetmp = RTL_R8(SYS_CLKR);
	bytetmp |= BIT(3);
	RTL_W8(SYS_CLKR, bytetmp);
	
	bytetmp = RTL_R8(GPIO_MUXCFG+1);
	bytetmp &= (~BIT(4));
	RTL_W8(GPIO_MUXCFG+1, bytetmp);


	// Release MAC IO register reset
	// 9.	CR 0x100[7:0]	= 0xFF;
	// 10.	CR 0x101[1]	= 0x01; // Enable SEC block
	RTL_W16(CR, 0x2ff);


	//System init
	LLT_table_init_8812(priv);

	//printk("\n\n 0x%.x, 0x%.8x \n\n\n", RTL_R32(0x200),  RTL_R32(0x204));

	// Enable interrupt
	RTL_W32(REG_HISR0_8812, 0xffffffff);
	RTL_W32(REG_HISR1_8812, 0xffffffff);
	
	Qbytetmp = RTL_R16(REG_TRXDMA_CTRL_8812);
	Qbytetmp &= 0xf;
	Qbytetmp |= 0xF5B1;
	RTL_W16(REG_TRXDMA_CTRL_8812, Qbytetmp);


	// Reported Tx status from HW for rate adaptive.
	// 2009/12/03 MH This should be realtive to power on step 14. But in document V11  
	// still not contain the description.!!!
	RTL_W8(REG_FWHW_TXQ_CTRL_8812+1, 0x1F);

	// disable earlymode
	RTL_W8(0x4d0, 0x0);

#if 0		// 2012-07-06
#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
		RTL_W32(TRXFF_BNDY, (RTL_R32(TRXFF_BNDY)&0x0000FFFF)|(0x13ff&RXFF0_BNDY_Mask)<<RXFF0_BNDY_SHIFT);
	} else
#endif
	{
		// Set Rx FF0 boundary : 9K/10K
		RTL_W32(TRXFF_BNDY, (RTL_R32(TRXFF_BNDY)&0x0000FFFF)|(0x27FF&RXFF0_BNDY_Mask)<<RXFF0_BNDY_SHIFT);
	}
#endif
//	RTL_W8(TDECTRL, 0x11);	// need to confirm

	// Set Network type: ap mod
	//RTL_W32(CR, RTL_R32(CR) | BIT(8));

	RTL_W32(CR, RTL_R32(CR) | ((NETYPE_AP & NETYPE_Mask) << NETYPE_SHIFT));

	// Set SLOT time
	RTL_W8(SLOT_TIME, 0x09);

	// Set AMPDU min space
	RTL_W8(AMPDU_MIN_SPACE, 0);	//	need to confirm

	// Set Tx/Rx page size (Tx must be 128 Bytes, Rx can be 64,128,256,512,1024 bytes)
	//RTL_W8(PBP, (PBP_256B&PSTX_Mask)<<PSTX_SHIFT|(PBP_256B&PSRX_Mask)<<PSRX_SHIFT);

	// Set RCR register
	RTL_W32(RCR, RCR_APP_FCS|RCR_APP_MIC|RCR_APP_ICV|RCR_APP_PHYSTS|RCR_HTC_LOC_CTRL
		|RCR_AMF|RCR_ADF|RCR_AICV|RCR_ACRC32|RCR_AB|RCR_AM|RCR_APM|RCR_AAP);

	// Set Driver info size
	RTL_W8(RX_DRVINFO_SZ, 4);

	// This part is not in WMAC InitMAC()
	// Set SEC register
	RTL_W16(SECCFG, RTL_R16(SECCFG) & ~(RXUSEDK | TXUSEDK));

	// Set TCR register
//	RTL_W32(TCR, RTL_R32(TCR)|CRC|CFE_FORM);
	RTL_W32(TCR, RTL_R32(TCR)|CFE_FORM);

	// Set TCR to avoid deadlock
 	RTL_W32(TCR, RTL_R32(TCR)|BIT(15)|BIT(14)|BIT(13)|BIT(12));

	// Set RRSR (response rate set reg)
	//SetResponseRate();
	// Set RRSR (response rate set reg)
	// Set RRSR to all legacy rate and HT rate
	// CCK rate is supported by default.
	// CCK rate will be filtered out only when associated AP does not support it.
	// Only enable ACK rate to OFDM 24M


	 {
		/*
		 *	Set RRSR at here before MACPHY_REG.txt is ready
		 */
		if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
			/*
			 *	PHY_BAND_5G
			 */
			RTL_W16(RRSR, 0x150);
		} else{
			/*
			 *	PHY_BAND_2G
			 */
			RTL_W16(RRSR, 0x15F); //Set 0x15F for NDSi Client Connection Issue
		}
		RTL_W8(RRSR+2, 0);

	} 

	// Set Spec SIFS (used in NAV)
	// Joseph test
	RTL_W16(SPEC_SIFS_A, (0x10&SPEC_SIFS_OFDM_Mask)<<SPEC_SIFS_OFDM_SHIFT
		|(0x0A&SPEC_SIFS_CCK_Mask)<<SPEC_SIFS_CCK_SHIFT);

	// Set SIFS for CCK
	// Joseph test
	RTL_W16(SIFS_CCK, (0x0E&SIFS_TRX_Mask)<<SIFS_TRX_SHIFT|(0x0A&SIFS_CTX_Mask)<<SIFS_CTX_SHIFT);

	// Set SIFS for OFDM
	// Joseph test
	RTL_W16(SIFS_OFDM, (0x0E&SIFS_TRX_Mask)<<SIFS_TRX_SHIFT|(0x0A&SIFS_CTX_Mask)<<SIFS_CTX_SHIFT);

	// Set retry limit
	// Joseph test
	priv->pshare->RLShort = 0x10;//0x30;
	priv->pshare->RLLong = 0x10; //0x30;

#ifdef CLIENT_MODE 
    if (priv->pmib->dot11OperationEntry.opmode & WIFI_STATION_STATE) 
    { 
	    priv->pshare->RLShort = 0x30; 
        priv->pshare->RLLong = 0x30; 
    } 
#endif 

	RTL_W16(RL, (priv->pshare->RLShort&SRL_Mask)<<SRL_SHIFT|(priv->pshare->RLLong&LRL_Mask)<<LRL_SHIFT);



#if 0 //eric_8812 ??

	RTL_W16(TRXDMA_CTRL, (0xC660 | RXSHFT_EN | RXDMA_ARBBW_EN));
	//RTL_W8(PBP, (PBP_256B&PSTX_Mask)<<PSTX_SHIFT|(PBP_256B&PSRX_Mask)<<PSRX_SHIFT);

#endif



		//Set Desc Address
#if 0 //def CONFIG_RTL_8812_SUPPORT
		RTL_W32(BCNQ_DESA, priv->pshare->phw->tx_bufringB_addr);
		RTL_W32(REG_92E_MGQ_DESA, priv->pshare->phw->tx_bufring0_addr);
		RTL_W32(REG_92E_VOQ_DESA, priv->pshare->phw->tx_bufring4_addr);
		RTL_W32(REG_92E_VIQ_DESA, priv->pshare->phw->tx_bufring3_addr);
		RTL_W32(REG_92E_BEQ_DESA, priv->pshare->phw->tx_bufring2_addr);
		RTL_W32(REG_92E_BKQ_DESA, priv->pshare->phw->tx_bufring1_addr);
		RTL_W32(REG_92E_HI0Q_DESA, priv->pshare->phw->tx_bufring5_addr);
	
		RTL_W32(REG_92E_HI1Q_DESA, priv->pshare->phw->tx_bufring6_addr);	
		RTL_W32(REG_92E_HI2Q_DESA, priv->pshare->phw->tx_bufring7_addr);	
		RTL_W32(REG_92E_HI3Q_DESA, priv->pshare->phw->tx_bufring8_addr);	
		RTL_W32(REG_92E_HI4Q_DESA, priv->pshare->phw->tx_bufring9_addr);	
		RTL_W32(REG_92E_HI5Q_DESA, priv->pshare->phw->tx_bufring10_addr);	
		RTL_W32(REG_92E_HI6Q_DESA, priv->pshare->phw->tx_bufring11_addr);	
		RTL_W32(REG_92E_HI7Q_DESA, priv->pshare->phw->tx_bufring12_addr);	
		
		RTL_W32(RX_DESA, priv->pshare->phw->rx_ring_addr);
		printk("0x%x,0x%x,0x%x,0x%x,0x%x\n", priv->pshare->phw->tx_bufring0_addr, priv->pshare->phw->tx_bufring1_addr, priv->pshare->phw->tx_bufring2_addr, priv->pshare->phw->tx_bufring3_addr, priv->pshare->phw->tx_bufring4_addr);
		printk("1clean hw host point,0x%x,0x%x\n", priv->pshare->phw->tx_bufring5_addr, priv->pshare->phw->tx_bufringB_addr);
		printk("2clean hw host point,0x%x,0x%x\n", priv->pshare->phw->tx_bufring6_addr, priv->pshare->phw->tx_bufring7_addr);
		printk("3clean hw host point,0x%x,0x%x\n", priv->pshare->phw->tx_bufring8_addr, priv->pshare->phw->tx_bufring9_addr);
		printk("4clean hw host point,0x%x,0x%x\n", priv->pshare->phw->tx_bufring10_addr, priv->pshare->phw->tx_bufring11_addr);
		printk("5clean hw host point,0x%x,0x%x\n", priv->pshare->phw->tx_bufring12_addr, priv->pshare->phw->tx_bufringB_addr);
		RTL_W32(REG_92E_ACQ_DES_NUM0, (((NUM_TX_DESC & ACQ_92E_VIQ_DESC_NUM_MASK)<<ACQ_92E_VIQ_DESC_NUM_SHIFT)|((NUM_TX_DESC & ACQ_92E_VOQ_DESC_NUM_MASK)<<ACQ_92E_VOQ_DESC_NUM_SHIFT)));
		RTL_W32(REG_92E_ACQ_DES_NUM1, (((NUM_TX_DESC & ACQ_92E_BKQ_DESC_NUM_MASK)<<ACQ_92E_BKQ_DESC_NUM_SHIFT)|((NUM_TX_DESC & ACQ_92E_BEQ_DESC_NUM_MASK)<<ACQ_92E_BEQ_DESC_NUM_SHIFT)));
		RTL_W32(REG_92E_HQ_DES_NUM0,  (((NUM_TX_DESC & ACQ_92E_H1Q_DESC_NUM_MASK)<<ACQ_92E_H1Q_DESC_NUM_SHIFT)|((NUM_TX_DESC & ACQ_92E_H0Q_DESC_NUM_MASK)<<ACQ_92E_H0Q_DESC_NUM_SHIFT)));
		RTL_W32(REG_92E_HQ_DES_NUM1,  (((NUM_TX_DESC & ACQ_92E_H3Q_DESC_NUM_MASK)<<ACQ_92E_H3Q_DESC_NUM_SHIFT)|((NUM_TX_DESC & ACQ_92E_H2Q_DESC_NUM_MASK)<<ACQ_92E_H2Q_DESC_NUM_SHIFT)));
		RTL_W32(REG_92E_HQ_DES_NUM2,  (((NUM_TX_DESC & ACQ_92E_H5Q_DESC_NUM_MASK)<<ACQ_92E_H5Q_DESC_NUM_SHIFT)|((NUM_TX_DESC & ACQ_92E_H4Q_DESC_NUM_MASK)<<ACQ_92E_H4Q_DESC_NUM_SHIFT)));
		RTL_W32(REG_92E_HQ_DES_NUM3,  (((NUM_TX_DESC & ACQ_92E_H7Q_DESC_NUM_MASK)<<ACQ_92E_H7Q_DESC_NUM_SHIFT)|((NUM_TX_DESC & ACQ_92E_H6Q_DESC_NUM_MASK)<<ACQ_92E_H6Q_DESC_NUM_SHIFT)));
		
		RTL_W32(REG_92E_ACQ_DES_NUM2, (((NUM_RX_DESC & ACQ_92E_RXQ_DESC_NUM_MASK)<<ACQ_92E_RXQ_DESC_NUM_SHIFT)|((NUM_TX_DESC & ACQ_92E_MGQ_DESC_NUM_MASK)<<ACQ_92E_MGQ_DESC_NUM_SHIFT)));
		//RTL_W32(0x3be, 64);
		RTL_W32(REG_92E_TSFT_CLRQ, CLRQ_92E_ALL_IDX);
		RTL_W8(REG_92E_UPD_HGQMD, RTL_R8(REG_92E_UPD_HGQMD)|BIT(5)|BIT(4));
		//RTL_W16(0x3be, 64);
		//RTL_W16(0x3b8, 64);
		//RTL_W8(0x229, 0xf6);
		//RTL_W8(0x457, 0xf6);
		//RTL_W8(0x5a7,0xff);
#else	
		RTL_W32(BCNQ_DESA, priv->pshare->phw->tx_ringB_addr);
		RTL_W32(MGQ_DESA, priv->pshare->phw->tx_ring0_addr);
		RTL_W32(VOQ_DESA, priv->pshare->phw->tx_ring4_addr);
		RTL_W32(VIQ_DESA, priv->pshare->phw->tx_ring3_addr);
		RTL_W32(BEQ_DESA, priv->pshare->phw->tx_ring2_addr);
		printk("BEQ_DESA = 0x%08x 0x%08x \n", RTL_R32(BEQ_DESA),priv->pshare->phw->tx_ring2_addr);
		RTL_W32(BKQ_DESA, priv->pshare->phw->tx_ring1_addr);
		RTL_W32(HQ_DESA, priv->pshare->phw->tx_ring5_addr);
		RTL_W32(RX_DESA, priv->pshare->phw->ring_dma_addr);
#endif

//	RTL_W32(RCDA, priv->pshare->phw->rxcmd_ring_addr);
//	RTL_W32(TCDA, priv->pshare->phw->txcmd_ring_addr);
//	RTL_W32(TCDA, phw->tx_ring5_addr);
	// 2009/03/13 MH Prevent incorrect DMA write after accident reset !!!
//	RTL_W16(CMDR, 0x37FC);

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8192D)
		RTL_W32(PCIE_CTRL_REG, (RTL_R32(PCIE_CTRL_REG) & 0x00ffffff)|(0x03&MAX_RXDMA_Mask)<<MAX_RXDMA_SHIFT
			|(0x03&MAX_TXDMA_Mask)<<MAX_TXDMA_SHIFT | BCNQSTOP);
	else
#endif
		RTL_W32(PCIE_CTRL_REG, RTL_R32(PCIE_CTRL_REG)|(0x07&MAX_RXDMA_Mask)<<MAX_RXDMA_SHIFT
			|(0x07&MAX_TXDMA_Mask)<<MAX_TXDMA_SHIFT | BCNQSTOP);

	// 20090928 Joseph
	// Reconsider when to do this operation after asking HWSD.
	RTL_W8(APSD_CTRL, RTL_R8(APSD_CTRL) & ~ BIT(6));
	retry = 0;
	do{
		retry++;
		bytetmp = RTL_R8(APSD_CTRL);
	} while((retry<200) && (bytetmp&BIT(7))); //polling until BIT7 is 0. by tynli
	
	if (bytetmp & BIT(7)) {
		DEBUG_ERR("%s ERROR: APSD_CTRL=0x%02X\n", __FUNCTION__, bytetmp);
	}
	// disable BT_enable
	RTL_W8(GPIO_MUXCFG, 0);

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8192D){
		RTL_W16(TCR, RTL_R16(TCR) | WMAC_TCR_ERRSTEN3 | WMAC_TCR_ERRSTEN2
 	                         | WMAC_TCR_ERRSTEN1 | WMAC_TCR_ERRSTEN0);
	/*
	 *	For 92DE,Mac0 and Mac1 power off.
	 *	0x1F	BIT6: 0 mac0 off, 1: mac0 on
	 *		BIT7: 0 mac1 off, 1: mac1 on.
	 */
#ifdef CONFIG_RTL_92D_DMDP
	 	 if (priv->pshare->wlandev_idx == 0)
#endif
		{
			RTL_W8(RSV_MAC0_CTRL, RTL_R8(RSV_MAC0_CTRL)|MAC0_EN);

			if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G)
				RTL_W8(RSV_MAC0_CTRL, RTL_R8(RSV_MAC0_CTRL) & (~BAND_STAT));
			else
				RTL_W8(RSV_MAC0_CTRL, RTL_R8(RSV_MAC0_CTRL) | BAND_STAT);
		}

#ifdef CONFIG_RTL_92D_DMDP
		if (priv->pshare->wlandev_idx ==1)
		{
			RTL_W8(RSV_MAC1_CTRL, RTL_R8(RSV_MAC1_CTRL)|MAC1_EN);
			if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G)
				RTL_W8(RSV_MAC1_CTRL, RTL_R8(RSV_MAC1_CTRL) & (~BAND_STAT));
			else
				RTL_W8(RSV_MAC1_CTRL, RTL_R8(RSV_MAC1_CTRL) | BAND_STAT);
		}
#endif
	}
#endif //CONFIG_RTL_92D_SUPPORT

	printk("DONE\n");
}
#endif

static void MacInit(struct rtl8192cd_priv *priv)
{
	volatile unsigned char bytetmp;
	volatile unsigned int Qbytetmp;
	unsigned short retry;

	DEBUG_INFO("CP: MacInit===>>");

	RTL_W8(RSV_CTRL0, 0x00);

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
		if (!HalPwrSeqCmdParsing(priv, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, 
				PWR_INTF_PCI_MSK, rtl8188E_card_enable_flow))
			panic_printk("%s %d, HalPwrSeqCmdParsing init fail!!!\n", __FUNCTION__, __LINE__);

#ifdef SUPPORT_RTL8188E_TC
		check_RTL8188E_testChip(priv);
#endif
	}
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8192D) {
		RTL_W8(SYS_FUNC_EN, FEN_PPLL | FEN_PCIEA | FEN_DIO_PCIE);

		/* Enable PLL Power (LDOA15V) */
		RTL_W8(LDOA15_CTRL, LDA15_OBUF | LDA15_EN);

		/* advise by MAC team */
		RTL_W8(LDOHCI12_CTRL, 0x1f);

#ifndef NOT_RTK_BSP
#if !defined(CONFIG_RTL_8198) && !defined(CONFIG_RTL_819XD) && !defined(CONFIG_RTL_8196E) && !defined(CONFIG_RTL_8198B)
		{
			/* temp modifying, for 96c pocket ap better performance */
			volatile unsigned int Qbytetmp;
			Qbytetmp = REG32(0xb8000048);
			Qbytetmp &= ~(BIT(10) | BIT(8));
			Qbytetmp |= BIT(19);
			REG32(0xb8000048) = Qbytetmp;
		}
#endif
#endif
	}

#endif

	// Power on when re-enter from IPS/Radio off/card disable
#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
		RTL_W8(AFE_XTAL_CTRL, RTL_R8(AFE_XTAL_CTRL) | BIT(0));

		Qbytetmp = RTL_R16(APS_FSMCO);
		Qbytetmp &= 0xE7ff;
		Qbytetmp |= 0x0800;
		RTL_W16(APS_FSMCO, Qbytetmp);

		while (!((Qbytetmp = RTL_R32(APS_FSMCO)) & 0x00020000));

		Qbytetmp = RTL_R16(APS_FSMCO);
		Qbytetmp &= 0x7FFF;
		RTL_W16(APS_FSMCO, Qbytetmp);

		Qbytetmp = RTL_R16(APS_FSMCO);
		Qbytetmp &= 0xE7ff;
		Qbytetmp |= 0x0000;
		RTL_W16(APS_FSMCO, Qbytetmp);
	} else
#endif
	{
#ifdef CONFIG_RTL8672
		RTL_W8(AFE_XTAL_CTRL, RTL_R8(AFE_XTAL_CTRL) | BIT(0));	// enable XTAL
#else
		/* just don't change BIT(1),Crystal engine setting refine*/ 
		//RTL_W8(AFE_XTAL_CTRL, 0x0d);	// enable XTAL		// clk inverted
#endif
		RTL_W8(SPS0_CTRL, 0x2b);		// enable SPS into PWM
	}
	delay_ms(1);
	
#if 0
	// Enable AFE BANDGAP
	RTL_W8(AFE_MISC, RTL_R8(AFE_MISC)|AFE_BGEN);
	DEBUG_INFO("AFE_MISC = 0x%02x\n", RTL_R8(AFE_MISC));

	// Enable AFE MBIAS
	RTL_W8(AFE_MISC, RTL_R8(AFE_MISC)|AFE_MBEN);
	DEBUG_INFO("AFE_MISC = 0x%02x\n", RTL_R8(AFE_MISC));

	// Enable PLL Power (LDOA15V)
#ifdef CONFIG_RTL_92C_SUPPORT //#ifndef CONFIG_RTL_92D_SUPPORT
	if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C))
		RTL_W8(LDOA15_CTRL, RTL_R8(LDOA15_CTRL)|LDA15_EN);
#endif

	// Enable VDDCORE (LDOD12V)
	RTL_W8(LDOV12D_CTRL, RTL_R8(LDOV12D_CTRL)|LDV12_EN);

	// Release XTAL Gated for AFE PLL
//	RTL_W32(AFE_XTAL_CTRL, RTL_R32(AFE_XTAL_CTRL)|XTAL_GATE_AFE);
	RTL_W32(AFE_XTAL_CTRL, RTL_R32(AFE_XTAL_CTRL) & ~XTAL_GATE_AFE);

	// Enable AFE PLL
	RTL_W32(AFE_PLL_CTRL, RTL_R32(AFE_PLL_CTRL)|APLL_EN);

	// Release Isolation AFE PLL & MD
	RTL_W16(SYS_ISO_CTRL, RTL_R16(SYS_ISO_CTRL) & ~ISO_MD2PP);

	// Enable WMAC Clock
	RTL_W16(SYS_CLKR, RTL_R16(SYS_CLKR)|MAC_CLK_EN|SEC_CLK_EN);

	// Release WMAC reset & register reset
	RTL_W16(SYS_FUNC_EN, RTL_R16(SYS_FUNC_EN)|FEN_MREGEN|FEN_DCORE);

	// Release IMEM Isolation
	RTL_W16(SYS_ISO_CTRL, RTL_R16(SYS_ISO_CTRL) & ~(BIT(10)|ISO_DIOR));	//	need to confirm

/*	// need double setting???
	// Enable MAC IO registers
	RTL_W16(SYS_FUNC_EN, RTL_R16(SYS_FUNC_EN)|FEN_MREGEN);
*/

	// Switch HWFW select
	RTL_W16(SYS_CLKR, (RTL_R16(SYS_CLKR)|CLKR_80M_SSC_DIS) & ~BIT(6));	//	need to confirm
#else
	// auto enable WLAN

	// Power On Reset for MAC Block
	bytetmp = RTL_R8(APS_FSMCO+1) | BIT(0);
	delay_us(2);
	RTL_W8(APS_FSMCO+1, bytetmp);
	delay_us(2);

	bytetmp = RTL_R8(APS_FSMCO+1);
	delay_us(2);
	retry = 0;
	while((bytetmp & BIT(0)) && retry < 1000){
		retry++;
		delay_us(50);
		bytetmp = RTL_R8(APS_FSMCO+1);
		delay_us(50);
	}
	
	if (bytetmp & BIT(0)) {
		DEBUG_ERR("%s ERROR: auto enable WLAN failed!!(0x%02X)\n", __FUNCTION__, bytetmp);
	}
	
#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E) 
		RTL_W16(SYS_FUNC_EN, RTL_R16(SYS_FUNC_EN) & ~FEN_CPUEN);
	else		/*Enable Radio off, GPIO, and LED function*/
#endif
		RTL_W16(APS_FSMCO, 0x1012);			// when enable HWPDN
	
	// release RF digital isolation
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
	if (
#ifdef CONFIG_RTL_92C_SUPPORT
		(GET_CHIP_VER(priv)==VERSION_8192C) || (GET_CHIP_VER(priv)==VERSION_8188C) 
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
#ifdef CONFIG_RTL_92C_SUPPORT
		|| 
#endif
		(GET_CHIP_VER(priv)==VERSION_8192D)
#endif
		)
		RTL_W8(SYS_ISO_CTRL+1, 0x82);
#endif

	delay_us(2);
#endif

	// Release MAC IO register reset
	RTL_W32(CR, RTL_R32(CR)|MACRXEN|MACTXEN|SCHEDULE_EN|PROTOCOL_EN
		|RXDMA_EN|TXDMA_EN|HCI_RXDMA_EN|HCI_TXDMA_EN);

	//System init
	LLT_table_init(priv);

	// Clear interrupt and enable interrupt
#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
		RTL_W32(REG_88E_HISR, 0xFFFFFFFF);
		RTL_W32(REG_88E_HISRE, 0xFFFFFFFF);
	} else
#endif
	{
		RTL_W32(HISR, 0xFFFFFFFF);
		RTL_W16(HISRE, 0xFFFF);
	}

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8192D) {

		switch(priv->pmib->dot11RFEntry.macPhyMode) {
		case SINGLEMAC_SINGLEPHY:
			RTL_W8(MAC_PHY_CTRL_T, 0xfc); 
			RTL_W8(MAC_PHY_CTRL_MP, 0xfc); //enable super mac
			RTL_W32(AGGLEN_LMT, 0xb972a841);
			break;
		case DUALMAC_SINGLEPHY:
			RTL_W8(MAC_PHY_CTRL_T, 0xf1); 			
			RTL_W8(MAC_PHY_CTRL_MP, 0xf1); //enable supermac
			RTL_W32(AGGLEN_LMT, 0x54325521);
			break;
		case DUALMAC_DUALPHY:
			RTL_W8(MAC_PHY_CTRL_T, 0xf3); 
			RTL_W8(MAC_PHY_CTRL_MP, 0xf3); //DMDP
			RTL_W32(AGGLEN_LMT, 0x54325521);
			break;
		default:
			DEBUG_ERR("Unknown 92D macPhyMode selection!\n");
		}
	/*
	 *    Set Rx FF0 boundary, half sized for testchip & dual MAC
	 */
#ifdef CONFIG_RTL_92D_DMDP
		if (priv->pmib->dot11RFEntry.macPhyMode!=SINGLEMAC_SINGLEPHY)
			RTL_W32(TRXFF_BNDY, (RTL_R32(TRXFF_BNDY)&0x0000FFFF)|(0x13ff&RXFF0_BNDY_Mask)<<RXFF0_BNDY_SHIFT);
		else
#endif
			RTL_W32(TRXFF_BNDY, (RTL_R32(TRXFF_BNDY)&0x0000FFFF)|(0x27ff&RXFF0_BNDY_Mask)<<RXFF0_BNDY_SHIFT);

	} else
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
		RTL_W32(TRXFF_BNDY, (RTL_R32(TRXFF_BNDY)&0x0000FFFF)|(0x25ff&RXFF0_BNDY_Mask)<<RXFF0_BNDY_SHIFT);
	} else
#endif
	{
		// Set Rx FF0 boundary : 9K/10K
		RTL_W32(TRXFF_BNDY, (RTL_R32(TRXFF_BNDY)&0x0000FFFF)|(0x27FF&RXFF0_BNDY_Mask)<<RXFF0_BNDY_SHIFT);
	}

#ifdef CONFIG_RTL_92C_SUPPORT
	if (IS_TEST_CHIP(priv)) {
		// Set High priority queue select : HPQ:BC/H/VO/VI/MG, LPQ:BE/BK
		// [5]:H, [4]:MG, [3]:BK, [2]:BE, [1]:VI, [0]:VO
		RTL_W16(TRXDMA_CTRL, ((HPQ_SEL_HIQ|HPQ_SEL_MGQ|HPQ_SEL_VIQ|HPQ_SEL_VOQ)&HPQ_SEL_Mask)<<HPQ_SEL_SHIFT);

		/*
		 * Enable ampdu rx check error, and enable rx byte shift
		 */
		RTL_W8(TRXDMA_CTRL, RTL_R8(TRXDMA_CTRL) | RXSHFT_EN | RXDMA_ARBBW_EN);
	} else
#endif
	{
#if 0//def CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8188E)
			RTL_W16(TRXDMA_CTRL, (/*0xF5B1*/ 0xB5B1 | RXSHFT_EN | RXDMA_ARBBW_EN));
		else
#endif
			//RTL_W16(TRXDMA_CTRL, (0xB770 | RXSHFT_EN | RXDMA_ARBBW_EN));

		#if defined(CONFIG_RTL_ULINKER_BRSC)
			RTL_W16(TRXDMA_CTRL, (0x5660 | RXDMA_ARBBW_EN)); //disable IP(layer3) auto aligne to 4bytes
		#else
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8188E && priv->pmib->dot11OperationEntry.wifi_specific != 1)
			RTL_W16(TRXDMA_CTRL, (0x56a0 | RXSHFT_EN | RXDMA_ARBBW_EN));
		else
#endif
			RTL_W16(TRXDMA_CTRL, (0x5660 | RXSHFT_EN | RXDMA_ARBBW_EN));
		#endif
	}


//	RTL_W8(TDECTRL, 0x11);	// need to confirm

	// Set Network type: ap mode
	RTL_W32(CR, RTL_R32(CR) | ((NETYPE_AP & NETYPE_Mask) << NETYPE_SHIFT));

	// Set SLOT time
	RTL_W8(SLOT_TIME, 0x09);

	// Set AMPDU min space
	RTL_W8(AMPDU_MIN_SPACE, 0);	//	need to confirm

	// Set Tx/Rx page size (Tx must be 128 Bytes, Rx can be 64,128,256,512,1024 bytes)
	RTL_W8(PBP, (PBP_128B&PSTX_Mask)<<PSTX_SHIFT|(PBP_128B&PSRX_Mask)<<PSRX_SHIFT);

	// Set RCR register
	RTL_W32(RCR, RCR_APP_FCS|RCR_APP_MIC|RCR_APP_ICV|RCR_APP_PHYSTS|RCR_HTC_LOC_CTRL
		|RCR_AMF|RCR_ADF|RCR_AICV|RCR_ACRC32|RCR_AB|RCR_AM|RCR_APM|RCR_AAP);

	// Set Driver info size
	RTL_W8(RX_DRVINFO_SZ, 4);

	// This part is not in WMAC InitMAC()
	// Set SEC register
	RTL_W16(SECCFG, RTL_R16(SECCFG) & ~(RXUSEDK | TXUSEDK));

	// Set TCR register
//	RTL_W32(TCR, RTL_R32(TCR)|CRC|CFE_FORM);
	RTL_W32(TCR, RTL_R32(TCR)|CFE_FORM);

	// Set TCR to avoid deadlock
 	RTL_W32(TCR, RTL_R32(TCR)|BIT(15)|BIT(14)|BIT(13)|BIT(12));

	// Set RRSR (response rate set reg)
	//SetResponseRate();
	// Set RRSR (response rate set reg)
	// Set RRSR to all legacy rate and HT rate
	// CCK rate is supported by default.
	// CCK rate will be filtered out only when associated AP does not support it.
	// Only enable ACK rate to OFDM 24M
#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8192D) {
		/*
		 *	Set RRSR at here before MACPHY_REG.txt is ready
		 */
		if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
			/*
			 *	PHY_BAND_5G
			 */
			RTL_W16(RRSR, 0x150);
		} else{
			/*
			 *	PHY_BAND_2G
			 */
			RTL_W16(RRSR, 0x15F); //Set 0x15F for NDSi Client Connection Issue
		}
		RTL_W8(RRSR+2, 0);

		RTL_W8(RCR, 0x0e);		//follow 92c MACPHY_REG
		RTL_W8(RCR+1, 0x2a); 	//follow 92c MACPHY_REG
	} else 
#endif
	{
		RTL_W16(RRSR, 0xFFFF);
		RTL_W8(RRSR+2, 0xFF);
	}

	// Set Spec SIFS (used in NAV)
	// Joseph test
	RTL_W16(SPEC_SIFS_A, (0x0A&SPEC_SIFS_OFDM_Mask)<<SPEC_SIFS_OFDM_SHIFT
		|(0x0A&SPEC_SIFS_CCK_Mask)<<SPEC_SIFS_CCK_SHIFT);

	// Set SIFS for CCK
	// Joseph test
	RTL_W16(SIFS_CCK, (0x0A&SIFS_TRX_Mask)<<SIFS_TRX_SHIFT|(0x0A&SIFS_CTX_Mask)<<SIFS_CTX_SHIFT);

	// Set SIFS for OFDM
	// Joseph test
	RTL_W16(SIFS_OFDM, (0x0A&SIFS_TRX_Mask)<<SIFS_TRX_SHIFT|(0x0A&SIFS_CTX_Mask)<<SIFS_CTX_SHIFT);

	// Set retry limit
	// Joseph test
#ifdef NOT_RTK_BSP
	priv->pshare->RLShort = 0x3e;
	priv->pshare->RLLong = 0x3e;
#else
	priv->pshare->RLShort = 0x10;//0x30;
	priv->pshare->RLLong = 0x10; //0x30;
#endif

#ifdef CLIENT_MODE 
    if (priv->pmib->dot11OperationEntry.opmode & WIFI_STATION_STATE) 
    { 
	    priv->pshare->RLShort = 0x30; 
        priv->pshare->RLLong = 0x30; 
    } 
#endif 

	RTL_W16(RL, (priv->pshare->RLShort&SRL_Mask)<<SRL_SHIFT|(priv->pshare->RLLong&LRL_Mask)<<LRL_SHIFT);

	//Set Desc Address
	RTL_W32(BCNQ_DESA, priv->pshare->phw->tx_ringB_addr);
	RTL_W32(MGQ_DESA, priv->pshare->phw->tx_ring0_addr);
	RTL_W32(VOQ_DESA, priv->pshare->phw->tx_ring4_addr);
	RTL_W32(VIQ_DESA, priv->pshare->phw->tx_ring3_addr);
	RTL_W32(BEQ_DESA, priv->pshare->phw->tx_ring2_addr);
	RTL_W32(BKQ_DESA, priv->pshare->phw->tx_ring1_addr);
	RTL_W32(HQ_DESA, priv->pshare->phw->tx_ring5_addr);
	RTL_W32(RX_DESA, priv->pshare->phw->ring_dma_addr);
//	RTL_W32(RCDA, priv->pshare->phw->rxcmd_ring_addr);
//	RTL_W32(TCDA, priv->pshare->phw->txcmd_ring_addr);
//	RTL_W32(TCDA, phw->tx_ring5_addr);
	// 2009/03/13 MH Prevent incorrect DMA write after accident reset !!!
//	RTL_W16(CMDR, 0x37FC);

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8192D)
		RTL_W32(PCIE_CTRL_REG, (RTL_R32(PCIE_CTRL_REG) & 0x00ffffff)|(0x03&MAX_RXDMA_Mask)<<MAX_RXDMA_SHIFT
			|(0x03&MAX_TXDMA_Mask)<<MAX_TXDMA_SHIFT | BCNQSTOP);
	else
#endif
		RTL_W32(PCIE_CTRL_REG, RTL_R32(PCIE_CTRL_REG)|(0x07&MAX_RXDMA_Mask)<<MAX_RXDMA_SHIFT
			|(0x07&MAX_TXDMA_Mask)<<MAX_TXDMA_SHIFT | BCNQSTOP);

	// 20090928 Joseph
	// Reconsider when to do this operation after asking HWSD.
	RTL_W8(APSD_CTRL, RTL_R8(APSD_CTRL) & ~ BIT(6));
	retry = 0;
	do{
		retry++;
		bytetmp = RTL_R8(APSD_CTRL);
	} while((retry<200) && (bytetmp&BIT(7))); //polling until BIT7 is 0. by tynli
	
	if (bytetmp & BIT(7)) {
		DEBUG_ERR("%s ERROR: APSD_CTRL=0x%02X\n", __FUNCTION__, bytetmp);
	}
	// disable BT_enable
	RTL_W8(GPIO_MUXCFG, 0);

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8192D){
		RTL_W16(TCR, RTL_R16(TCR) | WMAC_TCR_ERRSTEN3 | WMAC_TCR_ERRSTEN2
 	                         | WMAC_TCR_ERRSTEN1 | WMAC_TCR_ERRSTEN0);
	/*
	 *	For 92DE,Mac0 and Mac1 power off.
	 *	0x1F	BIT6: 0 mac0 off, 1: mac0 on
	 *		BIT7: 0 mac1 off, 1: mac1 on.
	 */
#ifdef CONFIG_RTL_92D_DMDP
	 	 if (priv->pshare->wlandev_idx == 0)
#endif
		{
			RTL_W8(RSV_MAC0_CTRL, RTL_R8(RSV_MAC0_CTRL)|MAC0_EN);

			if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G)
				RTL_W8(RSV_MAC0_CTRL, RTL_R8(RSV_MAC0_CTRL) & (~BAND_STAT));
			else
				RTL_W8(RSV_MAC0_CTRL, RTL_R8(RSV_MAC0_CTRL) | BAND_STAT);
		}

#ifdef CONFIG_RTL_92D_DMDP
		if (priv->pshare->wlandev_idx ==1)
		{
			RTL_W8(RSV_MAC1_CTRL, RTL_R8(RSV_MAC1_CTRL)|MAC1_EN);
			if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G)
				RTL_W8(RSV_MAC1_CTRL, RTL_R8(RSV_MAC1_CTRL) & (~BAND_STAT));
			else
				RTL_W8(RSV_MAC1_CTRL, RTL_R8(RSV_MAC1_CTRL) | BAND_STAT);
		}
#endif
	}
#endif //CONFIG_RTL_92D_SUPPORT

	DEBUG_INFO("DONE\n");
}	//	MacInit


static void MacConfig(struct rtl8192cd_priv *priv)
{
	//RTL_W8(INIRTS_RATE_SEL, 0x8); // 24M
	priv->pshare->phw->RTSInitRate_Candidate = priv->pshare->phw->RTSInitRate = 0x8; // 24M
	RTL_W8(INIRTS_RATE_SEL, priv->pshare->phw->RTSInitRate);

	// 2007/02/07 Mark by Emily becasue we have not verify whether this register works
	//For 92C,which reg?
	//	RTL_W8(BWOPMODE, BW_20M);	//	set if work at 20m

	// Ack timeout.
	if ((priv->pmib->miscEntry.ack_timeout > 0) && (priv->pmib->miscEntry.ack_timeout < 0xff))
		RTL_W8(ACKTO, priv->pmib->miscEntry.ack_timeout);
	else
	RTL_W8(ACKTO, 0x40);

	// clear for mbid beacon tx
	RTL_W8(MULTI_BCNQ_EN, 0);
	RTL_W8(MULTI_BCNQ_OFFSET, 0);

#ifdef CONFIG_RTL_92D_SUPPORT
	if ((GET_CHIP_VER(priv)==VERSION_8192D) && (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G)){
		RTL_W32(ARFR0, 0xFF010);	// 40M mode
		RTL_W32(ARFR1, 0xFF010);	// 20M mode
	} else
#endif
	{
		// set user defining ARFR table for 11n 1T
		RTL_W32(ARFR0, 0xFF015);	// 40M mode
		RTL_W32(ARFR1, 0xFF005);	// 20M mode
	}
	/*
 	 * Disable TXOP CFE
 	 */
	RTL_W16(RD_CTRL, RTL_R16(RD_CTRL) | DIS_TXOP_CFE);

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8192D) {
		RTL_W8(MAC_SEL, 0);
		RTL_W8(0x526, 0xff);		/* enable all MBID interface beacon */

		/*
		 *	Protection mode control for hw RTS
		 */
		RTL_W16(PROT_MODE_CTRL, 0xff0D);
	}
#endif

	/*
	 *	RA try rate aggr limit
	 */
	RTL_W8(RA_TRY_RATE_AGG_LMT, 0);

	/*
	 *	Max mpdu number per aggr
	 */
	RTL_W16(PROT_MODE_CTRL+2, 0x0909);

#if (defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT)) && defined(TX_EARLY_MODE)
	if (
#ifdef CONFIG_RTL_92D_SUPPORT
       (GET_CHIP_VER(priv) == VERSION_8192D)
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
#ifdef CONFIG_RTL_92D_SUPPORT
       ||
#endif
       (GET_CHIP_VER(priv) == VERSION_8188E)
#endif
	)
		disable_em(priv);
#endif

#if defined(CONFIG_RTL_88E_SUPPORT) && defined(TXREPORT)
	if (GET_CHIP_VER(priv)==VERSION_8188E)
		RTL8188E_EnableTxReport(priv);
#endif
}


unsigned int get_mean_of_2_close_value(unsigned int *val_array)
{
	unsigned int tmp1, tmp2;

	//printk("v1 %08x v2 %08x v3 %08x\n", val_array[0], val_array[1], val_array[2]);
	if (val_array[0] > val_array[1]) {
		tmp1 = val_array[1];
		val_array[1] = val_array[0];
		val_array[0] = tmp1;
	}
	if (val_array[1] > val_array[2]) {
		tmp1 = val_array[2];
		val_array[2] = val_array[1];
		val_array[1] = tmp1;
	}
	if (val_array[0] > val_array[1]) {
		tmp1 = val_array[1];
		val_array[1] = val_array[0];
		val_array[0] = tmp1;
	}
	//printk("v1 %08x v2 %08x v3 %08x\n", val_array[0], val_array[1], val_array[2]);

	tmp1 = val_array[1] - val_array[0];
	tmp2 = val_array[2] - val_array[1];
	if (tmp1 < tmp2)
		tmp1 = (val_array[0] + val_array[1]) / 2;
	else
		tmp1 = (val_array[1] + val_array[2]) / 2;

	//printk("final %08x\n", tmp1);
	return tmp1;
}

#ifdef CONFIG_RTL_92C_SUPPORT

void PHY_APCalibrate(struct rtl8192cd_priv *priv)
{
#ifdef HIGH_POWER_EXT_PA
	if (!priv->pshare->rf_ft_var.use_ext_pa)
#endif
	if(!IS_TEST_CHIP(priv)) {
		if(GET_CHIP_VER(priv) == VERSION_8192C)
			APK_MAIN(priv, 1);
		else if(GET_CHIP_VER(priv) == VERSION_8188C)
			APK_MAIN(priv, 0);
	}
}

#ifdef CONFIG_RTL_92C_SUPPORT
#ifndef CONFIG_RTL_NEW_IQK
static void IQK_92CD(struct rtl8192cd_priv *priv)
{
	unsigned int cal_num=0, cal_retry=0, Oldval=0, temp_c04=0, temp_c08=0, temp_874=0, temp_eac;
	unsigned int cal_e94, cal_e9c, cal_ea4, cal_eac, cal_eb4, cal_ebc, cal_ec4, cal_ecc, adda_on_reg;
	unsigned int X, Y, val_e94[3], val_e9c[3], val_ea4[3], val_eac[3], val_eb4[3], val_ebc[3], val_ec4[3], val_ecc[3];
#ifdef HIGH_POWER_EXT_PA
	unsigned int temp_870=0, temp_860=0, temp_864=0;
#endif

	// step 1: save ADDA power saving parameters
	unsigned int temp_85c = RTL_R32(0x85c);
	unsigned int temp_e6c = RTL_R32(0xe6c);
	unsigned int temp_e70 = RTL_R32(0xe70);
	unsigned int temp_e74 = RTL_R32(0xe74);
	unsigned int temp_e78 = RTL_R32(0xe78);
	unsigned int temp_e7c = RTL_R32(0xe7c);
	unsigned int temp_e80 = RTL_R32(0xe80);
	unsigned int temp_e84 = RTL_R32(0xe84);
	unsigned int temp_e88 = RTL_R32(0xe88);
	unsigned int temp_e8c = RTL_R32(0xe8c);
	unsigned int temp_ed0 = RTL_R32(0xed0);
	unsigned int temp_ed4 = RTL_R32(0xed4);
	unsigned int temp_ed8 = RTL_R32(0xed8);
	unsigned int temp_edc = RTL_R32(0xedc);
	unsigned int temp_ee0 = RTL_R32(0xee0);
	unsigned int temp_eec = RTL_R32(0xeec);

#ifdef MP_TEST
	if (!priv->pshare->rf_ft_var.mp_specific)
#endif
	{
		if (priv->pshare->iqk_2g_done)
			return;
		priv->pshare->iqk_2g_done = 1;
	}

printk(">> %s \n",__FUNCTION__);

#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		temp_870 = RTL_R32(0x870);
		temp_860 = RTL_R32(0x860);
		temp_864 = RTL_R32(0x864);
	}
#endif

	// step 2: Path-A ADDA all on
	adda_on_reg = 0x04db25a4;

	RTL_W32(0x85c, adda_on_reg);
	RTL_W32(0xe6c, adda_on_reg);
	RTL_W32(0xe70, adda_on_reg);
	RTL_W32(0xe74, adda_on_reg);
	RTL_W32(0xe78, adda_on_reg);
	RTL_W32(0xe7c, adda_on_reg);
	RTL_W32(0xe80, adda_on_reg);
	RTL_W32(0xe84, adda_on_reg);
	RTL_W32(0xe88, adda_on_reg);
	RTL_W32(0xe8c, adda_on_reg);
	RTL_W32(0xed0, adda_on_reg);
	RTL_W32(0xed4, adda_on_reg);
	RTL_W32(0xed8, adda_on_reg);
	RTL_W32(0xedc, adda_on_reg);
	RTL_W32(0xee0, adda_on_reg);
	RTL_W32(0xeec, adda_on_reg);

	// step 3: IQ&LO calibration Setting
		// BB switch to PI mode
	//RTL_W32(0x820, 0x01000100);
	//RTL_W32(0x828, 0x01000100);
		//BB setting
	temp_c04 = RTL_R32(0xc04);
	temp_c08 = RTL_R32(0xc08);
	temp_874 = RTL_R32(0x874);
	RTL_W32(0xc04, 0x03a05600);
	RTL_W32(0xc08, 0x000800e4);
	RTL_W32(0x874, 0x00204000);

#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		PHY_SetBBReg(priv, 0x870, BIT(10), 1);
		PHY_SetBBReg(priv, 0x870, BIT(26), 1);
		PHY_SetBBReg(priv, 0x860, BIT(10), 0);
		PHY_SetBBReg(priv, 0x864, BIT(10), 0);
	}
#endif
	RTL_W32(0x840, 0x00010000);
	RTL_W32(0x844, 0x00010000);

	//AP or IQK
	RTL_W32(0xb68 , 0x00080000);
	RTL_W32(0xb6c , 0x00080000);

		// IQK setting
	RTL_W32(0xe28, 0x80800000);
	RTL_W32(0xe40, 0x01007c00);
	RTL_W32(0xe44, 0x01004800);
		// path-A IQK setting
	RTL_W32(0xe30, 0x10008c1f);
	RTL_W32(0xe34, 0x10008c1f);
	RTL_W32(0xe38, 0x82140102);
	RTL_W32(0xe3c, 0x28160202);
		// path-B IQK setting
	RTL_W32(0xe50, 0x10008c22);
	RTL_W32(0xe54, 0x10008c22);
	RTL_W32(0xe58, 0x82140102);
	RTL_W32(0xe5c, 0x28160202);
		// LO calibration setting
	RTL_W32(0xe4c, 0x001028d1);

	// delay to ensure Path-A IQK success
	delay_ms(300);

	// step 4: One shot, path A LOK & IQK
	while (cal_num < 3) {
			// One shot, path A LOK & IQK
		RTL_W32(0xe48, 0xf9000000);
		RTL_W32(0xe48, 0xf8000000);
			// delay 1ms
		delay_ms(10);

		// check fail bit and check abnormal condition, then fill BB IQ matrix
		cal_e94 = (RTL_R32(0xe94) >> 16) & 0x3ff;
		cal_e9c = (RTL_R32(0xe9c) >> 16) & 0x3ff;
		cal_ea4 = (RTL_R32(0xea4) >> 16) & 0x3ff;
		temp_eac = RTL_R32(0xeac);
		cal_eac = (temp_eac >> 16) & 0x3ff;
		if (!(temp_eac & BIT(28)) && !(temp_eac & BIT(27)) &&
			(cal_e94 != 0x142) && (cal_e9c != 0x42) &&
			(cal_ea4 != 0x132) && (cal_eac != 0x36)) {
			val_e94[cal_num] = cal_e94;
			val_e9c[cal_num] = cal_e9c;
			val_ea4[cal_num] = cal_ea4;
			val_eac[cal_num] = cal_eac;
			cal_num++;
		} else {
			if (++cal_retry >= 10) {
				printk("%s Path-A Check\n",__FUNCTION__);
				break;
			}
		}
	}

	if (cal_num == 3) {
		cal_e94 = get_mean_of_2_close_value(val_e94);
		cal_e9c = get_mean_of_2_close_value(val_e9c);
		cal_ea4 = get_mean_of_2_close_value(val_ea4);
		cal_eac = get_mean_of_2_close_value(val_eac);

		priv->pshare->RegE94=cal_e94;
		priv->pshare->RegE9C=cal_e9c;

		Oldval = (RTL_R32(0xc80) >> 22) & 0x3ff;

		X = cal_e94;
		PHY_SetBBReg(priv, 0xc80, 0x3ff, X * (Oldval / 0x100));
		PHY_SetBBReg(priv, 0xc4c, BIT(24), ((X * Oldval) >> 7) & 0x1);

		Y = cal_e9c;
		PHY_SetBBReg(priv, 0xc94, 0xf0000000, ((Y * (Oldval / 0x100)) >> 6) & 0xf);
		PHY_SetBBReg(priv, 0xc80, 0x003f0000, (Y * (Oldval / 0x100)) & 0x3f);
		PHY_SetBBReg(priv, 0xc4c, BIT(26), ((Y * Oldval) >> 7) & 0x1);

		PHY_SetBBReg(priv, 0xc14, 0x3ff, cal_ea4);

		PHY_SetBBReg(priv, 0xc14, 0xfc00, cal_eac & 0x3f);

		PHY_SetBBReg(priv, 0xca0, 0xf0000000, (cal_eac >> 6) & 0xf);
	}else {
		priv->pshare->RegE94=0x100;
		priv->pshare->RegE9C=0x00;
	}

	// step 5: Path-A standby mode
	RTL_W32(0xe28, 0);
	RTL_W32(0x840, 0x00010000);
	RTL_W32(0xe28, 0x80800000);

	// step 6: Path-B ADDA all on
	adda_on_reg = 0x0b1b25a4;

	RTL_W32(0x85c, adda_on_reg);
	RTL_W32(0xe6c, adda_on_reg);
	RTL_W32(0xe70, adda_on_reg);
	RTL_W32(0xe74, adda_on_reg);
	RTL_W32(0xe78, adda_on_reg);
	RTL_W32(0xe7c, adda_on_reg);
	RTL_W32(0xe80, adda_on_reg);
	RTL_W32(0xe84, adda_on_reg);
	RTL_W32(0xe88, adda_on_reg);
	RTL_W32(0xe8c, adda_on_reg);
	RTL_W32(0xed0, adda_on_reg);
	RTL_W32(0xed4, adda_on_reg);
	RTL_W32(0xed8, adda_on_reg);
	RTL_W32(0xedc, adda_on_reg);
	RTL_W32(0xee0, adda_on_reg);
	RTL_W32(0xeec, adda_on_reg);

	// step 7: One shot, path B LOK & IQK
	cal_num = 0;
	cal_retry = 0;
	while (cal_num < 3) {
			// One shot, path B LOK & IQK
		RTL_W32(0xe60, 2);
		RTL_W32(0xe60, 0);
			// delay 1ms
		delay_ms(10);

		// check fail bit and check abnormal condition, then fill BB IQ matrix
		cal_eb4 = (RTL_R32(0xeb4) >> 16) & 0x3ff;
		cal_ebc = (RTL_R32(0xebc) >> 16) & 0x3ff;
		cal_ec4 = (RTL_R32(0xec4) >> 16) & 0x3ff;
		cal_ecc = (RTL_R32(0xecc) >> 16) & 0x3ff;
		temp_eac = RTL_R32(0xeac);
		if (!(temp_eac & BIT(31)) && !(temp_eac & BIT(30)) &&
			(cal_eb4 != 0x142) && (cal_ebc != 0x42) &&
			(cal_ec4 != 0x132) && (cal_ecc != 0x36)) {
			val_eb4[cal_num] = cal_eb4;
			val_ebc[cal_num] = cal_ebc;
			val_ec4[cal_num] = cal_ec4;
			val_ecc[cal_num] = cal_ecc;
			cal_num++;
		} else {
			if (++cal_retry >= 10) {
				printk("%s Path-B Check\n",__FUNCTION__);
				break;
			}
		}
	}

	if (cal_num == 3) {
		cal_eb4 = get_mean_of_2_close_value(val_eb4);
		cal_ebc = get_mean_of_2_close_value(val_ebc);
		cal_ec4 = get_mean_of_2_close_value(val_ec4);
		cal_ecc = get_mean_of_2_close_value(val_ecc);

		priv->pshare->RegEB4=cal_eb4;
		priv->pshare->RegEBC=cal_ebc;

		Oldval = (RTL_R32(0xc88) >> 22) & 0x3ff;

		X = cal_eb4;
		PHY_SetBBReg(priv, 0xc88, 0x3ff, X * (Oldval / 0x100));
		PHY_SetBBReg(priv, 0xc4c, BIT(28), ((X * Oldval) >> 7) & 0x1);

		Y = cal_ebc;
		PHY_SetBBReg(priv, 0xc9c, 0xf0000000, ((Y * (Oldval / 0x100)) >> 6) & 0xf);
		PHY_SetBBReg(priv, 0xc88, 0x003f0000, (Y * (Oldval / 0x100)) & 0x3f);
		PHY_SetBBReg(priv, 0xc4c, BIT(30), ((Y * Oldval) >> 7) & 0x1);

		PHY_SetBBReg(priv, 0xc1c, 0x3ff, cal_ec4);

		PHY_SetBBReg(priv, 0xc1c, 0xfc00, cal_ecc & 0x3f);

		PHY_SetBBReg(priv, 0xc78, 0xf000, (cal_ecc >> 6) & 0xf);
	}else {
		priv->pshare->RegEB4=0x100;
		priv->pshare->RegEBC=0x00;
	}


	// step 8: back to BB mode, load original values
	RTL_W32(0xc04, temp_c04);
	RTL_W32(0x874, temp_874);
	RTL_W32(0xc08, temp_c08);
	RTL_W32(0xe28, 0);
	RTL_W32(0x840, 0x32ed3);
	RTL_W32(0x844, 0x32ed3);
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		RTL_W32(0x870, temp_870);
		RTL_W32(0x860, temp_860);
		RTL_W32(0x864, temp_864);
	}
#endif
		// return to SI mode
	//RTL_W32(0x820, 0x01000000);
	//RTL_W32(0x828, 0x01000000);

	// step 9: reload ADDA power saving parameters
	RTL_W32(0x85c, temp_85c);
	RTL_W32(0xe6c, temp_e6c);
	RTL_W32(0xe70, temp_e70);
	RTL_W32(0xe74, temp_e74);
	RTL_W32(0xe78, temp_e78);
	RTL_W32(0xe7c, temp_e7c);
	RTL_W32(0xe80, temp_e80);
	RTL_W32(0xe84, temp_e84);
	RTL_W32(0xe88, temp_e88);
	RTL_W32(0xe8c, temp_e8c);
	RTL_W32(0xed0, temp_ed0);
	RTL_W32(0xed4, temp_ed4);
	RTL_W32(0xed8, temp_ed8);
	RTL_W32(0xedc, temp_edc);
	RTL_W32(0xee0, temp_ee0);
	RTL_W32(0xeec, temp_eec);

}


static void IQK_88C(struct rtl8192cd_priv *priv)
{
	unsigned int cal_num=0, cal_retry=0;
	unsigned int Oldval_0=0, temp_c04=0, temp_c08=0, temp_874=0;
	unsigned int cal_e94, cal_e9c, cal_ea4, cal_eac, temp_eac;
	unsigned int X, Y, val_e94[3], val_e9c[3], val_ea4[3], val_eac[3];

#ifdef HIGH_POWER_EXT_PA
	unsigned int temp_870=0, temp_860=0, temp_864=0;
#endif
	// step 1: save ADDA power saving parameters
	unsigned int temp_85c = RTL_R32(0x85c);
	unsigned int temp_e6c = RTL_R32(0xe6c);
	unsigned int temp_e70 = RTL_R32(0xe70);
	unsigned int temp_e74 = RTL_R32(0xe74);
	unsigned int temp_e78 = RTL_R32(0xe78);
	unsigned int temp_e7c = RTL_R32(0xe7c);
	unsigned int temp_e80 = RTL_R32(0xe80);
	unsigned int temp_e84 = RTL_R32(0xe84);
	unsigned int temp_e88 = RTL_R32(0xe88);
	unsigned int temp_e8c = RTL_R32(0xe8c);
	unsigned int temp_ed0 = RTL_R32(0xed0);
	unsigned int temp_ed4 = RTL_R32(0xed4);
	unsigned int temp_ed8 = RTL_R32(0xed8);
	unsigned int temp_edc = RTL_R32(0xedc);
	unsigned int temp_ee0 = RTL_R32(0xee0);
	unsigned int temp_eec = RTL_R32(0xeec);

#ifdef MP_TEST
	if (!priv->pshare->rf_ft_var.mp_specific)
#endif
	{
		if (priv->pshare->iqk_2g_done)
			return;
		priv->pshare->iqk_2g_done = 1;
	}

#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		temp_870 = RTL_R32(0x870);
		temp_860 = RTL_R32(0x860);
		temp_864 = RTL_R32(0x864);
	}
#endif

	// step 2: ADDA all on
	RTL_W32(0x85c, 0x0b1b25a0);
	RTL_W32(0xe6c, 0x0bdb25a0);
	RTL_W32(0xe70, 0x0bdb25a0);
	RTL_W32(0xe74, 0x0bdb25a0);
	RTL_W32(0xe78, 0x0bdb25a0);
	RTL_W32(0xe7c, 0x0bdb25a0);
	RTL_W32(0xe80, 0x0bdb25a0);
	RTL_W32(0xe84, 0x0bdb25a0);
	RTL_W32(0xe88, 0x0bdb25a0);
	RTL_W32(0xe8c, 0x0bdb25a0);
	RTL_W32(0xed0, 0x0bdb25a0);
	RTL_W32(0xed4, 0x0bdb25a0);
	RTL_W32(0xed8, 0x0bdb25a0);
	RTL_W32(0xedc, 0x0bdb25a0);
	RTL_W32(0xee0, 0x0bdb25a0);
	RTL_W32(0xeec, 0x0bdb25a0);

	// step 3: start IQK
		// BB switch to PI mode
	//RTL_W32(0x820, 0x01000100);
	//RTL_W32(0x828, 0x01000100);
		//BB setting
	temp_c04 = RTL_R32(0xc04);
	temp_c08 = RTL_R32(0xc08);
	temp_874 = RTL_R32(0x874);
	RTL_W32(0xc04, 0x03a05600);
	RTL_W32(0xc08, 0x000800e4);
	RTL_W32(0x874, 0x00204000);
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		PHY_SetBBReg(priv, 0x870, BIT(10), 1);
		PHY_SetBBReg(priv, 0x870, BIT(26), 1);
		PHY_SetBBReg(priv, 0x860, BIT(10), 0);
		PHY_SetBBReg(priv, 0x864, BIT(10), 0);
	}
#endif

	//AP or IQK
//	RTL_W32(0xb68, 0x0f600000);
	RTL_W32(0xb68, 0x00080000);

		// IQK setting
	RTL_W32(0xe28, 0x80800000);
	RTL_W32(0xe40, 0x01007c00);
	RTL_W32(0xe44, 0x01004800);
		// path-A IQK setting
	RTL_W32(0xe30, 0x10008c1f);
	RTL_W32(0xe34, 0x10008c1f);
	RTL_W32(0xe38, 0x82140102);
	//RTL_W32(0xe3c, 0x28160502);
	RTL_W32(0xe3c, 0x28160202);

		// LO calibration setting
	RTL_W32(0xe4c, 0x001028d1);

	while (cal_num < 3) {
			// One shot, path A LOK & IQK
		RTL_W32(0xe48, 0xf9000000);
		RTL_W32(0xe48, 0xf8000000);
			// delay 1ms
		delay_ms(150);

		// step 4: check fail bit and check abnormal condition, then fill BB IQ matrix
		cal_e94 = (RTL_R32(0xe94) >> 16) & 0x3ff;
		cal_e9c = (RTL_R32(0xe9c) >> 16) & 0x3ff;
		cal_ea4 = (RTL_R32(0xea4) >> 16) & 0x3ff;
		temp_eac = RTL_R32(0xeac);
		cal_eac = (temp_eac >> 16) & 0x3ff;
		if (!(temp_eac & BIT(28)) && !(temp_eac & BIT(27)) &&
			(cal_e94 != 0x142) && (cal_e9c != 0x42) &&
			(cal_ea4 != 0x132) && (cal_eac != 0x36)) {
			val_e94[cal_num] = cal_e94;
			val_e9c[cal_num] = cal_e9c;
			val_ea4[cal_num] = cal_ea4;
			val_eac[cal_num] = cal_eac;
			cal_num++;
		} else {
			if (++cal_retry >= 10) {
				printk("IQK Check\n");
				break;
			}
		}
	}

	if (cal_num == 3) {
		cal_e94 = get_mean_of_2_close_value(val_e94);
		cal_e9c = get_mean_of_2_close_value(val_e9c);
		cal_ea4 = get_mean_of_2_close_value(val_ea4);
		cal_eac = get_mean_of_2_close_value(val_eac);

		priv->pshare->RegE94=cal_e94;
		priv->pshare->RegE9C=cal_e9c;

		Oldval_0 = (RTL_R32(0xc80) >> 22) & 0x3ff;

		X = cal_e94;
		PHY_SetBBReg(priv, 0xc80, 0x3ff, X * (Oldval_0 / 0x100));
		PHY_SetBBReg(priv, 0xc4c, BIT(24), ((X * Oldval_0) >> 7) & 0x1);

		Y = cal_e9c;
		PHY_SetBBReg(priv, 0xc94, 0xf0000000, ((Y * (Oldval_0 / 0x100)) >> 6) & 0xf);
		PHY_SetBBReg(priv, 0xc80, 0x003f0000, (Y * (Oldval_0 / 0x100)) & 0x3f);
		PHY_SetBBReg(priv, 0xc4c, BIT(26), ((Y * Oldval_0) >> 7) & 0x1);

		PHY_SetBBReg(priv, 0xc14, 0x3ff, cal_ea4);

		PHY_SetBBReg(priv, 0xc14, 0xfc00, cal_eac & 0x3f);

		PHY_SetBBReg(priv, 0xca0, 0xf0000000, (cal_eac >> 6) & 0xf);
	}else {
		priv->pshare->RegE94=0x100;
		priv->pshare->RegE9C=0x00;
	}

		// back to BB mode
	RTL_W32(0xc04, temp_c04);
	RTL_W32(0x874, temp_874);
	RTL_W32(0xc08, temp_c08);
	RTL_W32(0xe28, 0);
	RTL_W32(0x840, 0x00032ed3);
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		RTL_W32(0x870, temp_870);
		RTL_W32(0x860, temp_860);
		RTL_W32(0x864, temp_864);
	}
#endif
		// return to SI mode
	//RTL_W32(0x820, 0x01000000);
	//RTL_W32(0x828, 0x01000000);

	// step 5: reload ADDA power saving parameters
	RTL_W32(0x85c, temp_85c);
	RTL_W32(0xe6c, temp_e6c);
	RTL_W32(0xe70, temp_e70);
	RTL_W32(0xe74, temp_e74);
	RTL_W32(0xe78, temp_e78);
	RTL_W32(0xe7c, temp_e7c);
	RTL_W32(0xe80, temp_e80);
	RTL_W32(0xe84, temp_e84);
	RTL_W32(0xe88, temp_e88);
	RTL_W32(0xe8c, temp_e8c);
	RTL_W32(0xed0, temp_ed0);
	RTL_W32(0xed4, temp_ed4);
	RTL_W32(0xed8, temp_ed8);
	RTL_W32(0xedc, temp_edc);
	RTL_W32(0xee0, temp_ee0);
	RTL_W32(0xeec, temp_eec);
}	// IQK
#endif
#endif

#endif

void PHY_IQCalibrate(struct rtl8192cd_priv *priv)
{
#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D){
#ifdef CONFIG_RTL_92D_DMDP
		if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY) {
			if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
#ifdef DFS
		    if ((priv->pshare->rf_ft_var.dfsdelayiqk) &&
		            (OPMODE & WIFI_AP_STATE) &&
		            !priv->pmib->dot11DFSEntry.disable_DFS &&
		            (timer_pending(&priv->ch_avail_chk_timer) ||
		             priv->pmib->dot11DFSEntry.disable_tx))
		             return;
#endif
				IQK_92D_5G_phy0_n(priv);
			}
			else
				IQK_92D_2G_phy1(priv);
		}
		else
#endif
		{
			if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
#ifdef DFS
                if ((priv->pshare->rf_ft_var.dfsdelayiqk) &&
                        (OPMODE & WIFI_AP_STATE) &&
                        !priv->pmib->dot11DFSEntry.disable_DFS &&
                        (timer_pending(&priv->ch_avail_chk_timer) ||
                         priv->pmib->dot11DFSEntry.disable_tx))
                         return;
#endif
				IQK_92D_5G_n(priv);
			}
			else
				IQK_92D_2G(priv);
		}
	}
#endif
#ifdef CONFIG_RTL_92C_SUPPORT
	if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)) {
#ifdef CONFIG_RTL_NEW_IQK
		PHY_IQCalibrate_92C(priv);
#else
		if( IS_UMC_A_CUT_88C(priv)
#ifdef HIGH_POWER_EXT_PA
			||(priv->pshare->rf_ft_var.use_ext_pa)
#endif
		) {
		 PHY_IQCalibrate_92C(priv);
		} else {
			if (GET_CHIP_VER(priv) == VERSION_8192C)
				IQK_92CD(priv);
			else
				IQK_88C(priv);
		}
#endif
	}
#endif

#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8188E) 
#ifdef USE_OUT_SRC
			PHY_IQCalibrate_8188E(ODMPTR, FALSE);
#else
			PHY_IQCalibrate_8188E(priv, FALSE);
#endif
#endif

#ifdef CONFIG_RTL_8812_SUPPORT //FOR_8812_IQK
#ifdef USE_OUT_SRC
		if (GET_CHIP_VER(priv) == VERSION_8812E) 
		{	
			panic_printk("\nDO 8812 IQK !!!!\n");
			priv->pshare->No_RF_Write = 0;
			phy_IQCalibrate_8812A(ODMPTR);
			priv->pshare->No_RF_Write = 1;
			panic_printk("Done 8812 IQK !!!!\n\n");
		}
#endif
#endif

#ifdef CONFIG_WLAN_HAL_8881A
#ifdef USE_OUT_SRC
		if (GET_CHIP_VER(priv) == VERSION_8881A) 
		{	
			panic_printk("\nDO 8881A IQK !!!!\n");
			priv->pshare->No_RF_Write = 0;
            // TODO: Filen, check with BB/RF Team
            panic_printk("\nError, No 8881A IQK func\n\n");
			//phy_IQCalibrate_8881A(ODMPTR);
			priv->pshare->No_RF_Write = 1;
			panic_printk("Done 8881A IQK !!!!\n\n");
		}
#endif
#endif

#ifdef CONFIG_WLAN_HAL_8192EE
#ifdef USE_OUT_SRC
		if (GET_CHIP_VER(priv) == VERSION_8192E) 
		{	
//			panic_printk("\nDO 8192E IQK !!!!\n");
            // TODO: Filen, check with BB/RF Team
//            panic_printk("\nError, No 8192E IQK func\n\n");
			//phy_IQCalibrate_8192E(ODMPTR);
//			panic_printk("Done 8192E IQK !!!!\n\n");
		}
#endif
#endif
}


#ifdef ADD_TX_POWER_BY_CMD
static void assign_txpwr_offset(struct rtl8192cd_priv *priv)
{
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_A[0], priv->pshare->rf_ft_var.txPowerPlus_cck_11);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_A[1], priv->pshare->rf_ft_var.txPowerPlus_cck_5);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_A[2], priv->pshare->rf_ft_var.txPowerPlus_cck_2);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_A[3], priv->pshare->rf_ft_var.txPowerPlus_cck_1);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_B[0], priv->pshare->rf_ft_var.txPowerPlus_cck_11);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_B[1], priv->pshare->rf_ft_var.txPowerPlus_cck_5);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_B[2], priv->pshare->rf_ft_var.txPowerPlus_cck_2);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_B[3], priv->pshare->rf_ft_var.txPowerPlus_cck_1);

	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[0], priv->pshare->rf_ft_var.txPowerPlus_ofdm_18);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[1], priv->pshare->rf_ft_var.txPowerPlus_ofdm_12);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[2], priv->pshare->rf_ft_var.txPowerPlus_ofdm_9);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[3], priv->pshare->rf_ft_var.txPowerPlus_ofdm_6);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[0], priv->pshare->rf_ft_var.txPowerPlus_ofdm_18);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[1], priv->pshare->rf_ft_var.txPowerPlus_ofdm_12);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[2], priv->pshare->rf_ft_var.txPowerPlus_ofdm_9);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[3], priv->pshare->rf_ft_var.txPowerPlus_ofdm_6);

	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[4], priv->pshare->rf_ft_var.txPowerPlus_ofdm_54);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[5], priv->pshare->rf_ft_var.txPowerPlus_ofdm_48);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[6], priv->pshare->rf_ft_var.txPowerPlus_ofdm_36);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[7], priv->pshare->rf_ft_var.txPowerPlus_ofdm_24);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[4], priv->pshare->rf_ft_var.txPowerPlus_ofdm_54);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[5], priv->pshare->rf_ft_var.txPowerPlus_ofdm_48);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[6], priv->pshare->rf_ft_var.txPowerPlus_ofdm_36);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[7], priv->pshare->rf_ft_var.txPowerPlus_ofdm_24);

	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[0], priv->pshare->rf_ft_var.txPowerPlus_mcs_3);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[1], priv->pshare->rf_ft_var.txPowerPlus_mcs_2);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[2], priv->pshare->rf_ft_var.txPowerPlus_mcs_1);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[3], priv->pshare->rf_ft_var.txPowerPlus_mcs_0);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[0], priv->pshare->rf_ft_var.txPowerPlus_mcs_3);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[1], priv->pshare->rf_ft_var.txPowerPlus_mcs_2);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[2], priv->pshare->rf_ft_var.txPowerPlus_mcs_1);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[3], priv->pshare->rf_ft_var.txPowerPlus_mcs_0);

	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[4], priv->pshare->rf_ft_var.txPowerPlus_mcs_7);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[5], priv->pshare->rf_ft_var.txPowerPlus_mcs_6);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[6], priv->pshare->rf_ft_var.txPowerPlus_mcs_5);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[7], priv->pshare->rf_ft_var.txPowerPlus_mcs_4);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[4], priv->pshare->rf_ft_var.txPowerPlus_mcs_7);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[5], priv->pshare->rf_ft_var.txPowerPlus_mcs_6);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[6], priv->pshare->rf_ft_var.txPowerPlus_mcs_5);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[7], priv->pshare->rf_ft_var.txPowerPlus_mcs_4);

	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[8], priv->pshare->rf_ft_var.txPowerPlus_mcs_11);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[9], priv->pshare->rf_ft_var.txPowerPlus_mcs_10);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[10], priv->pshare->rf_ft_var.txPowerPlus_mcs_9);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[11], priv->pshare->rf_ft_var.txPowerPlus_mcs_8);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[8], priv->pshare->rf_ft_var.txPowerPlus_mcs_11);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[9], priv->pshare->rf_ft_var.txPowerPlus_mcs_10);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[10], priv->pshare->rf_ft_var.txPowerPlus_mcs_9);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[11], priv->pshare->rf_ft_var.txPowerPlus_mcs_8);

	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[12], priv->pshare->rf_ft_var.txPowerPlus_mcs_15);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[13], priv->pshare->rf_ft_var.txPowerPlus_mcs_14);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[14], priv->pshare->rf_ft_var.txPowerPlus_mcs_13);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[15], priv->pshare->rf_ft_var.txPowerPlus_mcs_12);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[12], priv->pshare->rf_ft_var.txPowerPlus_mcs_15);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[13], priv->pshare->rf_ft_var.txPowerPlus_mcs_14);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[14], priv->pshare->rf_ft_var.txPowerPlus_mcs_13);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[15], priv->pshare->rf_ft_var.txPowerPlus_mcs_12);
}
#endif


void reload_txpwr_pg(struct rtl8192cd_priv *priv)
{
	PHY_ConfigBBWithParaFile(priv, PHYREG_PG);

#if 0 //def HIGH_POWER_EXT_PA
	if (!priv->pshare->rf_ft_var.use_ext_pa)
#endif
	{
	// get default Tx AGC offset
	//_TXPWR_REDEFINE ?? CCKTxAgc_A[1] [2] [3] ??
	*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[0])	= cpu_to_be32(RTL_R32(rTxAGC_A_Mcs03_Mcs00));
	*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[4])	= cpu_to_be32(RTL_R32(rTxAGC_A_Mcs07_Mcs04));
	*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[8])	= cpu_to_be32(RTL_R32(rTxAGC_A_Mcs11_Mcs08));
	*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[12]) = cpu_to_be32(RTL_R32(rTxAGC_A_Mcs15_Mcs12));
	*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[0]) = cpu_to_be32(RTL_R32(rTxAGC_A_Rate18_06));
	*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[4]) = cpu_to_be32(RTL_R32(rTxAGC_A_Rate54_24));
	*(unsigned int *)(&priv->pshare->phw->CCKTxAgc_A[0]) = cpu_to_be32((RTL_R32(rTxAGC_A_CCK11_2_B_CCK11) & 0xffffff00)
		| RTL_R8(rTxAGC_A_CCK1_Mcs32 + 1));
	
#if defined(CONFIG_RTL_92D_SUPPORT)&& defined(CONFIG_RTL_92D_DMDP)
	if (priv->pmib->dot11RFEntry.macPhyMode==DUALMAC_DUALPHY && priv->pshare->wlandev_idx == 1) {
		*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[0])	= cpu_to_be32(RTL_R32(rTxAGC_B_Mcs03_Mcs00));
		*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[4])	= cpu_to_be32(RTL_R32(rTxAGC_B_Mcs07_Mcs04));
		*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[8])	= cpu_to_be32(RTL_R32(rTxAGC_B_Mcs11_Mcs08));
		*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[12]) = cpu_to_be32(RTL_R32(rTxAGC_B_Mcs15_Mcs12));
		*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[0]) = cpu_to_be32(RTL_R32(rTxAGC_B_Rate18_06));
		*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[4]) = cpu_to_be32(RTL_R32(rTxAGC_B_Rate54_24));
		*(unsigned int *)(&priv->pshare->phw->CCKTxAgc_A[0]) = cpu_to_be32((RTL_R8(rTxAGC_A_CCK11_2_B_CCK11) << 24)
			| (RTL_R32(rTxAGC_B_CCK5_1_Mcs32) >> 8));
	}
#endif	

	*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[0])	= cpu_to_be32(RTL_R32(rTxAGC_B_Mcs03_Mcs00));
	*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[4])	= cpu_to_be32(RTL_R32(rTxAGC_B_Mcs07_Mcs04));
	*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[8])	= cpu_to_be32(RTL_R32(rTxAGC_B_Mcs11_Mcs08));
	*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[12]) = cpu_to_be32(RTL_R32(rTxAGC_B_Mcs15_Mcs12));
	*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_B[0]) = cpu_to_be32(RTL_R32(rTxAGC_B_Rate18_06));
	*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_B[4]) = cpu_to_be32(RTL_R32(rTxAGC_B_Rate54_24));
	*(unsigned int *)(&priv->pshare->phw->CCKTxAgc_B[0]) = cpu_to_be32((RTL_R8(rTxAGC_A_CCK11_2_B_CCK11) << 24)
		| (RTL_R32(rTxAGC_B_CCK5_1_Mcs32) >> 8));

	}
#ifdef ADD_TX_POWER_BY_CMD
	assign_txpwr_offset(priv);
#endif
}

#ifdef USE_OUT_SRC

void ODM_software_init(struct rtl8192cd_priv *priv)
{	
	unsigned long ability;
	priv->pshare->_dmODM.priv = priv;

	//
	// Init Value
	//
	// 1. u1Byte SupportPlatform
	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_PLATFORM, ODM_AP);

	// 2. u4Byte SupportAbility
	ability =	\
			ODM_BB_DIG				|
			ODM_BB_RA_MASK		|
			ODM_BB_FA_CNT			|
			ODM_BB_RATE_ADAPTIVE	|
			ODM_MAC_EDCA_TURBO	|
			ODM_RF_RX_GAIN_TRACK	|
			ODM_RF_CALIBRATION		|
			ODM_BB_DYNAMIC_TXPWR	|
			ODM_RF_TX_PWR_TRACK |
			0;

#ifdef CONFIG_RTL_8812_SUPPORT
	if(GET_CHIP_VER(priv) == VERSION_8812)
	ability =	\
		ODM_BB_DIG				|
		ODM_BB_FA_CNT			|
		ODM_MAC_EDCA_TURBO	|
		ODM_BB_RSSI_MONITOR |
		0;
#endif

#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
        // TODO:  

#ifdef CONFIG_WLAN_HAL_8881A
		ability = ODM_MAC_EDCA_TURBO;
#endif

#ifdef CONFIG_WLAN_HAL_8192EE
		ability = \
		ODM_BB_DIG				|
		ODM_BB_RA_MASK			|
		ODM_BB_FA_CNT			|
		ODM_BB_RSSI_MONITOR		|
		ODM_MAC_EDCA_TURBO		|
		ODM_BB_DYNAMIC_TXPWR    |
		ODM_BB_CCK_PD;
#endif

	}
#endif
	
//	if(GET_CHIP_VER(priv)!=VERSION_8188E)
//		ability |= ODM_BB_DYNAMIC_TXPWR;
	
#if defined(HW_ANT_SWITCH)
	if (priv->pshare->rf_ft_var.antHw_enable)
		ability |= ODM_BB_ANT_DIV;
#endif
#if defined(SW_ANT_SWITCH) 
	if (priv->pshare->rf_ft_var.antSw_enable)
		ability |= ODM_BB_ANT_DIV;
#endif
#ifdef TX_EARLY_MODE
	ability |= ODM_MAC_EARLY_MODE;
#endif
		
	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_ABILITY, ability);
	ODM_CmnInfoUpdate(ODMPTR, ODM_CMNINFO_ABILITY, ability);

	// 3. u1Byte SupportInterface
	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_INTERFACE, ODM_ITRF_PCIE);

	// 4. u4Byte SupportICType
	if((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C))
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_IC_TYPE, ODM_RTL8192C);
	else if(GET_CHIP_VER(priv) == VERSION_8192D)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_IC_TYPE, ODM_RTL8192D);	
	else if(GET_CHIP_VER(priv) == VERSION_8188E)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_IC_TYPE, ODM_RTL8188E);	
	else if(GET_CHIP_VER(priv) == VERSION_8812E)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_IC_TYPE, ODM_RTL8812);	
    else if(GET_CHIP_VER(priv) == VERSION_8881A)
        ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_IC_TYPE, ODM_RTL8881A);  
    else if(GET_CHIP_VER(priv) == VERSION_8192E)
        ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_IC_TYPE, ODM_RTL8192E);  

	// 5. u1Byte CutVersion
	if (GET_CHIP_VER(priv) == VERSION_8188E)  {
		if(IS_TEST_CHIP(priv) ) {
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_CUT_VER, ODM_CUT_TEST);
		}
		else {
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_CUT_VER, ODM_CUT_A);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_MP_TEST_CHIP, 1);
		}
	}
	else {
#ifdef CONFIG_RTL_92C_SUPPORT		
		if(IS_UMC_B_CUT_88C(priv))
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_CUT_VER, ODM_CUT_B);
		else
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_CUT_VER, ODM_CUT_A);
#endif		
	}

	// 6. u1Byte FabVersion
#ifdef CONFIG_RTL_92C_SUPPORT		
	if(IS_UMC_A_CUT_88C(priv) || IS_UMC_B_CUT_88C(priv))
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_FAB_VER, ODM_UMC);
	else
#endif		
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_FAB_VER, ODM_TSMC);

	// 7. u1Byte RFType
	if(get_rf_mimo_mode(priv)== MIMO_2T2R)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_RF_TYPE, ODM_2T2R);
	else
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_RF_TYPE, ODM_1T1R);

	// 8. u1Byte BoardType
#ifdef HIGH_POWER_EXT_PA		
	if(priv->pshare->rf_ft_var.use_ext_pa) {
		priv->pmib->dot11RFEntry.trswitch=1;
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_BOARD_TYPE, ODM_BOARD_HIGHPWR);
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_EXT_PA, priv->pshare->rf_ft_var.use_ext_pa);		
	} else 
#endif
	{
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_BOARD_TYPE, ODM_BOARD_NORMAL );
	}

	// 9. u1Byte ExtLNA
	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_EXT_LNA, priv->pshare->rf_ft_var.use_ext_lna);

	// 10. u1Byte ExtPA

	// 11. u1Byte ExtTRSW, ODM_CMNINFO_EXT_TRSW:
	// follows variable "trswitch" which is modified in rtl8192cd_init_hw_PCI().
	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_EXT_TRSW, priv->pmib->dot11RFEntry.trswitch);

	// 12. u1Byte PatchID
	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_PATCH_ID, 0);

	// 13. BOOLEAN bInHctTest
	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_BINHCT_TEST, FALSE);

	// 14. BOOLEAN bWIFITest
	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_BWIFI_TEST, (priv->pmib->dot11OperationEntry.wifi_specific>0));

	// 15. BOOLEAN bDualMacSmartConcurrent
	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_SMART_CONCURRENT, FALSE);

	//
	// Dynamic Value
	//

	// 1. u1Byte *pMacPhyMode
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_MAC_PHY_MODE, &priv->pmib->dot11RFEntry.macPhyMode);

	// 2. u8Byte *pNumTxBytesUnicast
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_TX_UNI, &priv->pshare->NumTxBytesUnicast);

	// 3. u8Byte *pNumRxBytesUnicast
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_RX_UNI, &priv->pshare->NumRxBytesUnicast);

	// 4. u1Byte *pWirelessMode
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_WM_MODE, &priv->pmib->dot11BssType.net_work_type);

	// 5. u1Byte *pBandType
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_BAND, &priv->pmib->dot11RFEntry.phyBandSelect);

	// 6. u1Byte *pSecChOffset
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_SEC_CHNL_OFFSET, &priv->pshare->offset_2nd_chan);

	// 7. u1Byte *pSecurity
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_SEC_MODE, &priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm);

	// 8. u1Byte *pBandWidth
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_BW, &priv->pshare->CurrentChannelBW);

	// 9. u1Byte *pChannel
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_CHNL, &priv->pshare->working_channel);


	// 10. BOOLEAN *pbMasterOfDMSP
//	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_DMSP_IS_MASTER, NULL);
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_DMSP_IS_MASTER, &priv->pshare->dummy);	

	
	// 11. BOOLEAN *pbGetValueFromOtherMac
//	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_DMSP_GET_VALUE, NULL);
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_DMSP_GET_VALUE, &priv->pshare->dummy);	
	
	// 12. PADAPTER *pBuddyAdapter
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_BUDDY_ADAPTOR, NULL);

	// 13. BOOLEAN *pbBTOperation
//	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_BT_OPERATION, NULL);
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_BT_OPERATION, &priv->pshare->dummy);	
	
	// 14. BOOLEAN *pbBTDisableEDCATurbo
//	ODM_CmnInfoHook(pOdm, ODM_CMNINFO_BT_DISABLE_EDCA, NULL);
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_BT_DISABLE_EDCA, &priv->pshare->dummy);	


	// 15. BOOLEAN *pbScanInProcess
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_SCAN, &priv->pshare->bScanInProcess);	


	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_POWER_SAVING, &priv->pshare->dummy);	


	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_ONE_PATH_CCA, &priv->pshare->rf_ft_var.one_path_cca);	


// dummy

	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_DRV_STOP, &priv->pshare->dummy);	
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_PNP_IN, &priv->pshare->dummy);	
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_INIT_ON, &priv->pshare->dummy);	
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_BT_BUSY, &priv->pshare->dummy);	
//	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_ANT_DIV, &priv->pshare->dummy);	
	
// DM parameters init
//	ODM_DMInit(ODMPTR);

	ODM_InitAllTimers(ODMPTR);
}

#endif

int rtl8192cd_init_hw_PCI(struct rtl8192cd_priv *priv)
{
	struct wifi_mib *pmib;
	unsigned int opmode;
	unsigned long val32;
	unsigned short val16;
	int i;
#ifdef CONFIG_WLAN_HAL		
    unsigned long errorFlag;
#endif	
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT)
	unsigned int fwStatus=0, dwnRetry=5;
#endif

	pmib = GET_MIB(priv);
	opmode = priv->pmib->dot11OperationEntry.opmode;

	DBFENTER;

//1 For Test, Firmware Downloading

//	MacConfigBeforeFwDownload(priv);

#if 0 	//	==========>> later
	// ===========================================================================================
	// Download Firmware
	// allocate memory for tx cmd packet
	if((priv->pshare->txcmd_buf = (unsigned char *)kmalloc((LoadPktSize), GFP_ATOMIC)) == NULL) {
		printk("not enough memory for txcmd_buf\n");
		return -1;
	}

	priv->pshare->cmdbuf_phyaddr = get_physical_addr(priv, priv->pshare->txcmd_buf,
			LoadPktSize, PCI_DMA_TODEVICE);

	if(LoadFirmware(priv) == FALSE){
//		panic_printk("Load Firmware Fail!\n");
		panic_printk("Load Firmware check!\n");
		return -1;
	}else {
//		delay_ms(20);
		PRINT_INFO("Load firmware successful! \n");
	}
#endif
//	MacConfigAfterFwDownload(priv);

#if 0 // defined(CONFIG_RTL_92D_SUPPORT) && defined(MP_TEST)  // 92D MP DUAL-PHY SETTING
	if (priv->pshare->rf_ft_var.mp_specific && (GET_CHIP_VER(priv) == VERSION_8192D))
		priv->pmib->dot11RFEntry.macPhyMode = DUALMAC_DUALPHY;
		//priv->pmib->dot11RFEntry.macPhyMode = SINGLEMAC_SINGLEPHY;
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8192D) {
		check_chipID_MIMO(priv);
	}
#endif
#if 0 //def CONFIG_RTL_92D_SUPPORT
	if (check_MacPhyMode(priv)<0){
		printk("Check macPhyMode Fail!\n");
		return -1;
	}
#endif

#ifdef USE_OUT_SRC
	if(IS_OUTSRC_CHIP(priv))
		ODM_software_init(priv);
#endif

#ifdef SMART_CONCURRENT_92D
	if (priv->pmib->dot11RFEntry.smcc==1 && priv->pmib->dot11RFEntry.macPhyMode != SINGLEMAC_SINGLEPHY
		&& (priv->pshare->wlandev_idx == 0)
		){
		printk("Switch to dual-mac-single-phy!!!\n");
		priv->pmib->dot11RFEntry.macPhyMode = DUALMAC_SINGLEPHY;
	}
#endif

#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {

#if defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_819XD)
		if(GET_CHIP_VER(priv)== VERSION_8192E) {
			if ((REG32(0xb8000008)&0x2000000)!=0x2000000){
//				panic_printk("25M....\n");
				RTL_W8(REG_AFE_PLL_CTRL, RTL_R8(REG_AFE_PLL_CTRL) | 5);			// 25MHz
			}else{
//				panic_printk("40M....\n");
				RTL_W8(REG_AFE_PLL_CTRL, 1);   // 40MHz
			}
		}
#else
		if(GET_CHIP_VER(priv)== VERSION_8192E) {
#ifdef CONFIG_PHY_EAT_40MHZ
				RTL_W8(REG_AFE_PLL_CTRL, 1);   // 40MHz
#else
			//RTL_W8(REG_AFE_PLL_CTRL, RTL_R8(REG_AFE_PLL_CTRL) | 5);			// 25MHz
			RTL_W8(REG_AFE_PLL_CTRL, 1);   // 40MHz, fixed clock now, fix me
#endif	
		}
#endif		

	    if ( RT_STATUS_SUCCESS != GET_HAL_INTERFACE(priv)->InitPONHandler(priv) ) {
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
            errorFlag |= DRV_ER_INIT_PON;
            GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
	        panic_printk("InitPON Failed\n");
	    }
	    else {            
            printk("InitPON OK\n");
	    }

	    if ( RT_STATUS_SUCCESS != GET_HAL_INTERFACE(priv)->InitMACHandler(priv) ) {
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);            
            errorFlag |= DRV_ER_INIT_MAC;
            GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);              
	        panic_printk("InitMAC Failed\n");
	    }
	    else {
            
	        printk("InitMAC OK\n");
	    }
   
	    if ( RT_STATUS_SUCCESS != GET_HAL_INTERFACE(priv)->InitHCIDMARegHandler(priv) ) {
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);            
            errorFlag |= DRV_ER_INIT_HCIDMA;
            GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag); 
            panic_printk("InitHCIDMAReg Failed\n");
	    }
	    else {
	        printk("InitHCIDMAReg OK\n");
	    }

#ifdef CONFIG_WLAN_HAL_8881A
        //Filen, according to RLE0538 setting
        if (GET_CHIP_VER(priv)==VERSION_8881A) {
            //set RF register as PI mode
            RTL_W8(0xc00, RTL_R8(0xc00)|BIT(2));
            RTL_W8(0xe00, RTL_R8(0xe00)|BIT(2));            
        }
#endif // CONFIG_WLAN_HAL_8881A  

// patch for AES chariot 3pairs
#ifdef __ECOS
		if(priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm)
			RTL_W8(AGGR_BK_TIME, 0x06);
		else
			RTL_W8(AGGR_BK_TIME, 0x10);
#endif		
	} else	
#endif
	{
		MacInit(priv);
	}

//	RTL_W32(AGGLEN_LMT, RTL_R32(AGGLEN_LMT)|(0x0F&AGGLMT_MCS15S_Mask)<<AGGLMT_MCS15S_SHIFT
//		|(0x0F&AGGLMT_MCS15_Mask)<<AGGLMT_MCS15_SHIFT);

//	RTL_W8(AGGR_BK_TIME, 0x10);

	//
	// 2. Initialize MAC/PHY Config by MACPHY_reg.txt
	//

#if defined(CONFIG_MACBBRF_BY_ODM) && defined(CONFIG_RTL_88E_SUPPORT)
	if (GET_CHIP_VER(priv)==VERSION_8188E)	{		
		ODM_ConfigMACWithHeaderFile(ODMPTR);		
	} else
#endif
	if(PHY_ConfigMACWithParaFile(priv) < 0) {
        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);            
        errorFlag |= DRV_ER_INIT_MACPHYREGFILE;
        GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);           
		printk("Initialize MAC/PHY Config failure\n");
		return -1;
	}

	//
	// 3. Initialize BB After MAC Config PHY_reg.txt, AGC_Tab.txt
	//

#ifdef DRVMAC_LB
	if (!priv->pmib->miscEntry.drvmac_lb)
#endif
	{
#ifdef  CONFIG_WLAN_HAL
    	if (IS_HAL_CHIP(priv)) {                
            val32 = phy_BB88XX_Config_ParaFile(priv);

            if(val32)
            {
                GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);            
                errorFlag |= DRV_ER_INIT_BBEGFILE;
                GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);                   
                printk("Initialize phy_BB88XX_Config_ParaFile failure\n");
            }
    	}
        else
#endif	        
		val32 = phy_BB8192CD_Config_ParaFile(priv);
		if (val32)
			return -1;
	}

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8192D) {
		if(priv->pmib->dot11RFEntry.xcap != 0xff) {
			PHY_SetBBReg(priv, 0x24, 0xF0, priv->pmib->dot11RFEntry.xcap & 0x0F);
			PHY_SetBBReg(priv, 0x28, 0xF0000000, ((priv->pmib->dot11RFEntry.xcap & 0xF0) >> 4));
		}
	}
#endif

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT)
	if((GET_CHIP_VER(priv) == VERSION_8188E) || (GET_CHIP_VER(priv) == VERSION_8812E)){
		if(priv->pmib->dot11RFEntry.xcap > 0 && priv->pmib->dot11RFEntry.xcap < 0x3F) {
			PHY_SetBBReg(priv, 0x24, BIT(11)|BIT(12)|BIT(13)|BIT(14)|BIT(15)|BIT(16), priv->pmib->dot11RFEntry.xcap & 0x3F);
			PHY_SetBBReg(priv, 0x24, BIT(17)|BIT(18)|BIT(19)|BIT(20)|BIT(21)|BIT(22), priv->pmib->dot11RFEntry.xcap & 0x3F);
		}
	}
#endif

	// support up to MCS7 for 1T1R, modify rx capability here
	/*
	if (get_rf_mimo_mode(priv) == MIMO_1T1R
#ifdef SMART_CONCURRENT_92D  //-- fwdebug
		&& priv->pmib->dot11RFEntry.smcc==0
#endif
		)
		pmib->dot11nConfigEntry.dot11nSupportedMCS &= 0x00ff;

	*/
/*
	// Set NAV protection length
	// CF-END Threshold
	if (priv->pmib->dot11OperationEntry.wifi_specific) {
		RTL_W16(NAV_PROT_LEN, 0x80);
//		RTL_W8(CFEND_TH, 0x2);
	}
	else {
		RTL_W16(NAV_PROT_LEN, 0x01C0);
//		RTL_W8(CFEND_TH, 0xFF);
	}
*/
	//
	// 4. Initiailze RF RAIO_A.txt RF RAIO_B.txt
	//
	// 2007/11/02 MH Before initalizing RF. We can not use FW to do RF-R/W.
	// close loopback, normal mode

	// For RF test only from Scott's suggestion
//	RTL_W8(0x27, 0xDB);	//	==========>> ???
//	RTL_W8(0x1B, 0x07); // ACUT

/*
	// set RCR: RX_SHIFT and disable ACF
//	RTL_W8(0x48, 0x3e);
//	RTL_W32(0x48, RTL_R32(0x48) & ~ RCR_ACF  & ~RCR_ACRC32);
	RTL_W16(RCR, RCR_AAP | RCR_APM | RCR_ACRC32);
	RTL_W32(RCR, RTL_R32(RCR) & ~ RCR_ACF  & ~RCR_ACRC32);
	// for debug by victoryman, 20081119
//	RTL_W32(RCR, RTL_R32(RCR) | RCR_APP_PHYST_RXFF);
	RTL_W32(RCR, RTL_R32(RCR) | RCR_APP_PHYSTS);
*/
#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8192D) {
#if defined(RTL8192D_INT_PA)
			u8 c9 = priv->pmib->dot11RFEntry.trsw_pape_C9;
			u8 cc = priv->pmib->dot11RFEntry.trsw_pape_CC;
			if ((c9 == 0xAA && cc == 0xA0) ||	// Type 2 : 5G TRSW+Int. PA, 2G TR co-matched
				(c9 == 0xAA && cc == 0xAF) || 	// Type 3 : 5G SP3TSW+ Ext.(or Int.)PA, 2G TR co-matched
				(c9 == 0x00 && cc == 0xA0)) {	// Type 4 : 5G TRSW+ Ext( or Int.)PA, 2G TRSW+ Ext.(or Int.)PA

				panic_printk("\n**********************************\n");
				panic_printk("\n** NOTE!! RTL8192D INTERNAL PA! **\n");
				panic_printk("\n**********************************\n");

				
				priv->pshare->rf_ft_var.use_intpa92d = 1;
				priv->pshare->phw->InternalPA5G[0]=1;
				priv->pshare->phw->InternalPA5G[1]=1;
			} else {
				// if  using default setting, set as external PA for safe.
				priv->pshare->rf_ft_var.use_intpa92d = 0;
				priv->pmib->dot11RFEntry.trsw_pape_C9 = 0x00;
				priv->pmib->dot11RFEntry.trsw_pape_CC = 0xFF;
				priv->pshare->phw->InternalPA5G[0]=0;
				priv->pshare->phw->InternalPA5G[1]=0;
			} 
#else
			// to ignore flash setting for external PA
			priv->pmib->dot11RFEntry.trsw_pape_C9 = 0x00;
			priv->pmib->dot11RFEntry.trsw_pape_CC = 0xFF;
			priv->pshare->phw->InternalPA5G[0]=0;
			priv->pshare->phw->InternalPA5G[1]=0;
#endif
		}
#endif

#ifdef DRVMAC_LB
	if (priv->pmib->miscEntry.drvmac_lb) {
#ifdef  CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {
	        RT_OP_MODE  OP_Mode = RT_OP_MODE_NO_LINK;
	        GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_MEDIA_STATUS, &OP_Mode);
		} else
#endif
		{
			RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_NOLINK & NETYPE_Mask) << NETYPE_SHIFT));
		}
		drvmac_loopback(priv);
	}
	else
#endif
	{
//#ifdef CHECK_HANGUP
#if 0
		if (!priv->reset_hangup
#ifdef CLIENT_MODE
				|| (!(OPMODE & WIFI_AP_STATE) &&
						(priv->join_res != STATE_Sta_Bss) && (priv->join_res != STATE_Sta_Ibss_Active))
#endif
			)
#endif
		{
#ifdef  CONFIG_WLAN_HAL
			if (IS_HAL_CHIP(priv)) {

#ifdef CONFIG_WLAN_HAL_8881A
				if (GET_CHIP_VER(priv)==VERSION_8881A) {
                    val32 = phy_RF6052_Config_ParaFile(priv);

                    if(val32)
                    {
                        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);            
                        errorFlag |= DRV_ER_INIT_PHYRF;
                        GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);                           
                    }
                    
                    priv->pshare->No_RF_Write = 1;                    
				}                
#endif
                
#ifdef CONFIG_WLAN_HAL_8192EE
				if (GET_CHIP_VER(priv)==VERSION_8192E) {
#if 0
//#if defined(CONFIG_MACBBRF_BY_ODM)
  					val32 = ODM_ConfigRFWithHeaderFile(ODMPTR, ODM_RF_PATH_A, ODM_RF_PATH_A);
                    val32 |= ODM_ConfigRFWithHeaderFile(ODMPTR, ODM_RF_PATH_B, ODM_RF_PATH_B);
#else
    				val32 = phy_RF6052_Config_ParaFile(priv);                   
#endif
				}                
#endif

                if (val32)
					return -1;
			} 
            else
#endif
			{
#if defined(CONFIG_MACBBRF_BY_ODM) && defined(CONFIG_RTL_88E_SUPPORT)	
				if (GET_CHIP_VER(priv)==VERSION_8188E) {
					priv->pshare->phw->NumTotalRFPath = 1;
					val32 = ODM_ConfigRFWithHeaderFile(ODMPTR, ODM_RF_PATH_A, ODM_RF_PATH_A);
				} else
#endif
				val32 = phy_RF6052_Config_ParaFile(priv);
	

	#ifdef CONFIG_RTL_8812_SUPPORT 		 //eric_test
				if (GET_CHIP_VER(priv) == VERSION_8812E)
					priv->pshare->No_RF_Write = 1;
	#endif
				if (val32)
					return -1;

	#ifdef CONFIG_RTL_92C_SUPPORT
				if(IS_UMC_A_CUT_88C(priv))	{
					PHY_SetRFReg(priv, RF92CD_PATH_A, RF_RX_G1, bMask20Bits, 0x30255);
					PHY_SetRFReg(priv, RF92CD_PATH_A, RF_RX_G2, bMask20Bits, 0x50a00);
				} else if(IS_UMC_B_CUT_88C(priv))	{

					PHY_SetRFReg(priv, RF92CD_PATH_A, 0x1e, bMask20Bits, 0x03 |(PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x1e, bMaskDWord, 1)&0xff0f0));
					PHY_SetRFReg(priv, RF92CD_PATH_A, 0x1f, bMask20Bits, 0x200|(PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x1f, bMaskDWord, 1)&0xff0ff));
	#if 0
					PHY_SetRFReg(priv, RF92CD_PATH_A, 0x0c, bMask20Bits, 0x0008992f);
					PHY_SetRFReg(priv, RF92CD_PATH_A, 0x0a, bMask20Bits, 0x0001aef1);
					PHY_SetRFReg(priv, RF92CD_PATH_A, 0x15, bMask20Bits, 0x0008f425);
	#endif
				}
#endif
			}
		}
	}

#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
        //Do nothing   
        //MacConfig() is integrated in code below
        //Don't enable BB here
	} 
    else
#endif
	{
    #ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8192D) {
			unsigned int eRFPath, curMaxRFPath=((priv->pmib->dot11RFEntry.macPhyMode==SINGLEMAC_SINGLEPHY)?RF92CD_PATH_MAX : RF92CD_PATH_B);
			for(eRFPath = RF92CD_PATH_A; eRFPath <curMaxRFPath; eRFPath++) {
				priv->pshare->RegRF18[eRFPath] = PHY_QueryRFReg(priv, eRFPath, 0x18, bMask20Bits, 1);
				priv->pshare->RegRF28[eRFPath] = PHY_QueryRFReg(priv, eRFPath, 0x28, bMask20Bits, 1);
			}
			UpdateBBRFVal8192DE(priv);
		}
	#endif
	#ifdef CONFIG_RTL_8812_SUPPORT
		if(GET_CHIP_VER(priv)!=VERSION_8812E)
	#endif
		{
			/*---- Set CCK and OFDM Block "ON"----*/
			PHY_SetBBReg(priv, rFPGA0_RFMOD, bOFDMEn, 1);
		#if defined(CONFIG_RTL_92D_SUPPORT)
			if (!(priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G))
		#endif
				PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, 1);

			MacConfig(priv);
		}

		/*
		 *	Force CCK CCA for High Power products
		 */
		if (priv->pshare->rf_ft_var.use_ext_lna)
			RTL_W8(0xa0a, 0xcd);
	}

//	RTL_W8(BW_OPMODE, BIT(2)); // 40Mhz:0 20Mhz:1
//	RTL_W32(MACIDR,0x0);

	// under loopback mode
//	RTL_W32(MACIDR,0xffffffff);		//	need to confirm
/*
#ifdef CONFIG_NET_PCI
	if (IS_PCIBIOS_TYPE)
		pci_unmap_single(priv->pshare->pdev, priv->pshare->cmdbuf_phyaddr,
			(LoadPktSize), PCI_DMA_TODEVICE);
#endif
*/
#if	0
//	RTL_W32(0x230, 0x40000000);	//for test
////////////////////////////////////////////////////////////

	printk("init_hw: 1\n");

	RTL_W16(SIFS_OFDM, 0x1010);
	RTL_W8(SLOT_TIME, 0x09);

	RTL_W8(MSR, MSR_AP);

//	RTL_W8(MSR,MSR_INFRA);
	// for test, loopback
//	RTL_W8(MSR, MSR_NOLINK);
//	RTL_W8(LBKMD_SEL, BIT(0)| BIT(1) );
//	RTL_W16(LBDLY, 0xffff);

	//beacon related
	RTL_W16(BCN_INTERVAL, pmib->dot11StationConfigEntry.dot11BeaconPeriod);
	RTL_W16(ATIMWND, 2); //0
	RTL_W16(BCN_DRV_EARLY_INT, 10<<4); // 2
	RTL_W16(BCN_DMATIME, 256); // 0xf
	RTL_W16(SIFS_OFDM, 0x0e0e);
	RTL_W8(SLOT_TIME, 0x10);

//	CamResetAllEntry(priv);
	RTL_W16(SECR, 0x0000);

// By H.Y. advice
//	RTL_W16(_BCNTCFG_, 0x060a);
//	if (OPMODE & WIFI_AP_STATE)
//		RTL_W16(BCNTCFG, 0x000a);
//	else
// for debug
//	RTL_W16(_BCNTCFG_, 0x060a);
//	RTL_W16(BCNTCFG, 0x0204);

	init_beacon(priv);

	priv->pshare->InterruptMask = (IMR_ROK | IMR_VODOK | IMR_VIDOK | IMR_BEDOK | IMR_BKDOK |		\
	IMR_HCCADOK | IMR_MGNTDOK | IMR_COMDOK | IMR_HIGHDOK | 					\
	IMR_BDOK | /*IMR_RXCMDOK | IMR_TIMEOUT0 |*/ IMR_RDU | IMR_RXFOVW	|			\
	IMR_BcnInt/* | IMR_TXFOVW*/ /*| IMR_TBDOK | IMR_TBDER*/) ;// IMR_ROK | IMR_BcnInt | IMR_RDU | IMR_RXFOVW | IMR_RXCMDOK;

//	priv->pshare->InterruptMask = IMR_ROK| IMR_BDOK | IMR_BcnInt | IMR_MGNTDOK | IMR_TBDOK | IMR_RDU ;
//	priv->pshare->InterruptMask  = 0;
	priv->pshare->InterruptMaskExt = 0;
	RTL_W32(IMR, priv->pshare->InterruptMask);
	RTL_W32(IMR+4, priv->pshare->InterruptMaskExt);

//////////////////////////////////////////////////////////////
	printk("end of init hw\n");

	return 0;

#endif
// clear TPPoll
//	RTL_W16(TPPoll, 0x0);
// Should 8192SE do this initialize? I don't know yet, 080812. Joshua
	// PJ 1-5-2007 Reset PHY parameter counters
//	RTL_W32(0xD00, RTL_R32(0xD00)|BIT(27));
//	RTL_W32(0xD00, RTL_R32(0xD00)&(~(BIT(27))));
/*
	// configure timing parameter
	RTL_W8(ACK_TIMEOUT, 0x30);
	RTL_W8(PIFS_TIME,0x13);
//	RTL_W16(LBDLY, 0x060F);
//	RTL_W16(SIFS_OFDM, 0x0e0e);
//	RTL_W8(SLOT_TIME, 0x10);
	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) {
		RTL_W16(SIFS_OFDM, 0x0a0a);
		RTL_W8(SLOT_TIME, 0x09);
	}
	else if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) {
		RTL_W16(SIFS_OFDM, 0x0a0a);
		RTL_W8(SLOT_TIME, 0x09);
	}
	else if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) {
		RTL_W16(SIFS_OFDM, 0x0a0a);
		RTL_W8(SLOT_TIME, 0x09);
	}
	else { // WIRELESS_11B
		RTL_W16(SIFS_OFDM, 0x0a0a);
		RTL_W8(SLOT_TIME, 0x14);
	}
*/

#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
        RT_OP_MODE      OP_Mode;
        MACCONFIG_PARA  MacCfgPara;

        if (opmode & WIFI_AP_STATE) {
            DEBUG_INFO("AP-mode enabled...\n");
#if defined(CONFIG_RTK_MESH)	//Mesh Mode but mesh not enable
    		if (priv->pmib->dot11WdsInfo.wdsPure || priv->pmib->dot1180211sInfo.meshSilence )
#else
    		if (priv->pmib->dot11WdsInfo.wdsPure)
#endif
            {
                OP_Mode = RT_OP_MODE_NO_LINK;
    		}
    		else {
                OP_Mode = RT_OP_MODE_AP;
    		}            
        }
        else if (opmode & WIFI_STATION_STATE) {
        	DEBUG_INFO("Station-mode enabled...\n");
            OP_Mode = RT_OP_MODE_INFRASTRUCTURE;
        }
        else if (opmode & WIFI_ADHOC_STATE) {
    		DEBUG_INFO("Adhoc-mode enabled...\n");
            OP_Mode = RT_OP_MODE_IBSS;
        }
        else
    	{
    		printk("Operation mode error!\n");
    		return 2;
    	}

        GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_MEDIA_STATUS, (u8 *)&OP_Mode);

        MacCfgPara.AckTO                = priv->pmib->miscEntry.ack_timeout;
        MacCfgPara.vap_enable           = GET_ROOT(priv)->pmib->miscEntry.vap_enable;
        MacCfgPara.OP_Mode              = OP_Mode;
        MacCfgPara.dot11DTIMPeriod      = priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod;
        u2Byte beaconPeriod             = pmib->dot11StationConfigEntry.dot11BeaconPeriod;

        GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_BEACON_INTERVAL, (u8 *)&beaconPeriod);
        GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_MAC_CONFIG, &MacCfgPara);
    }
#endif

#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
#ifdef _OUTSRC_COEXIST
	if(!IS_OUTSRC_CHIP(priv))
#endif
	init_EDCA_para(priv, pmib->dot11BssType.net_work_type);
#endif

	// we don't have EEPROM yet, Mark this for FPGA Platform
//	RTL_W8(_9346CR_, CR9346_CFGRW);

//	92SE Windows driver does not set the PCIF as 0x77, seems HW bug?
	// Set Tx and Rx DMA burst size
//	RTL_W8(PCIF, 0x77);
	// Enable byte shift
//	RTL_W8(_PCIF_+2, 0x01);

/*
	// Retry Limit
	if (priv->pmib->dot11OperationEntry.dot11LongRetryLimit)
		val16 = priv->pmib->dot11OperationEntry.dot11LongRetryLimit & 0xff;
	else {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)
			val16 = 0x30;
		else
			val16 = 0x06;
	}
	if (priv->pmib->dot11OperationEntry.dot11ShortRetryLimit)
		val16 |= ((priv->pmib->dot11OperationEntry.dot11ShortRetryLimit & 0xff) << 8);
	else {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)
			val16 |= (0x30 << 8);
		else
			val16 |= (0x06 << 8);
	}
	RTL_W16(RETRY_LIMIT, val16);

	This should be done later, but Windows Driver not done yet.
	// Response Rate Set
	// let ACK sent by highest of 24Mbps
	val32 = 0x1ff;
	if (pmib->dot11RFEntry.shortpreamble)
		val32 |= BIT(23);
	RTL_W32(_RRSR_, val32);
*/




//	panic_printk("0x2c4 = bitmap = 0x%08x\n", (unsigned int)val32);
//	panic_printk("0x2c0 = cmd | macid | band = 0x%08x\n", 0xfd0000a2 | (1<<9 | (sta_band & 0xf))<<8);
//	panic_printk("Add id %d val %08x to ratr\n", 0, (unsigned int)val32);

/*	for (i = 0; i < 8; i++)
		RTL_W32(ARFR0+i*4, val32 & 0x1f0ff0f0);
*/

	//settting initial rate for control frame to 24M
//	RTL_W8(INIRTSMCS_SEL, 8);	//	==========>> later

#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
	    u8  MulticastAddr[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	    GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_MULTICAST_REG, MulticastAddr);

	    GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_BSSID, pmib->dot11OperationEntry.hwaddr);
	} else
#endif
	{
		//setting MAR
		RTL_W32(MAR, 0xffffffff);
		RTL_W32((MAR+4), 0xffffffff);

		//setting BSSID, not matter AH/AP/station
		memcpy((void *)&val32, (pmib->dot11OperationEntry.hwaddr), 4);
		memcpy((void *)&val16, (pmib->dot11OperationEntry.hwaddr + 4), 2);
		RTL_W32(BSSIDR, cpu_to_le32(val32));
		RTL_W16((BSSIDR + 4), cpu_to_le16(val16));
	//	RTL_W32(BSSIDR, 0x814ce000);
	//	RTL_W16((BSSIDR + 4), 0xee92);
	}

	//setting TCR
	//TCR, use default value

	//setting RCR // set in MacConfigAfterFwDownload
//	RTL_W32(_RCR_, _APWRMGT_ | _AMF_ | _ADF_ | _AICV_ | _ACRC32_ | _AB_ | _AM_ | _APM_);
//	if (pmib->dot11OperationEntry.crc_log)
//		RTL_W32(_RCR_, RTL_R32(_RCR_) | _ACRC32_);

#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
        //3 Integrated into SetHwRegHandler(priv, HW_VAR_MAC_CONFIG, &MacCfgPara);
	} else
#endif
	{
		// setting network type
		if (opmode & WIFI_AP_STATE)
		{
			DEBUG_INFO("AP-mode enabled...\n");

	#if defined(CONFIG_RTK_MESH)	//Mesh Mode but mesh not enable
			if (priv->pmib->dot11WdsInfo.wdsPure || priv->pmib->dot1180211sInfo.meshSilence )
	#else
			if (priv->pmib->dot11WdsInfo.wdsPure)
	#endif
				RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_NOLINK & NETYPE_Mask) << NETYPE_SHIFT));
			else
				RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_AP & NETYPE_Mask) << NETYPE_SHIFT));
	// Move init beacon after f/w download
	#if 0
			if (priv->auto_channel == 0) {
				DEBUG_INFO("going to init beacon\n");
				init_beacon(priv);
			}
	#endif
		}
	#ifdef CLIENT_MODE
		else if (opmode & WIFI_STATION_STATE)
		{
			DEBUG_INFO("Station-mode enabled...\n");
			RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_INFRA & NETYPE_Mask) << NETYPE_SHIFT));
		}
		else if (opmode & WIFI_ADHOC_STATE)
		{
			DEBUG_INFO("Adhoc-mode enabled...\n");
			RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_ADHOC & NETYPE_Mask) << NETYPE_SHIFT));
		}
	#endif
		else
		{
			printk("Operation mode error!\n");
			return 2;
		}
	}


    //3 Security Related

	CamResetAllEntry(priv);
#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
        SECURITY_CONFIG_OPERATION  SCO = 0;            
        GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_SECURITY_CONFIG, (u16 *)&SCO);
    } else
#endif
	{
		RTL_W16(SECCFG, 0);
	}
	if ((OPMODE & (WIFI_AP_STATE|WIFI_STATION_STATE|WIFI_ADHOC_STATE)) &&
		!priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm &&
			(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_ ||
				pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_)) {
		pmib->dot11GroupKeysTable.dot11Privacy = pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm;
		if (pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_)
			i = 5;
		else
			i = 13;
#ifdef USE_WEP_DEFAULT_KEY
		memcpy(pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey,
			&priv->pmib->dot11DefaultKeysTable.keytype[pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex].skey[0], i);
		pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKeyLen = i;
		pmib->dot11GroupKeysTable.keyid = pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex;
		pmib->dot11GroupKeysTable.keyInCam = 0;
#ifdef  CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {
            SECURITY_CONFIG_OPERATION  SCO;
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_SECURITY_CONFIG, (u16 *)&SCO);
            SCO |= SCO_NOSKMC;
            GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_SECURITY_CONFIG, (u16 *)&SCO);
        } else
#endif
		{
			RTL_W16(SECCFG, RTL_R16(SECCFG) | NOSKMC);	// no search multicast
		}
#else
#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
		memcpy(pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey,
							&priv->pmib->dot11DefaultKeysTable.keytype[pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex].skey[0], i);
#else
		memcpy(pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey,
							&priv->pmib->dot11DefaultKeysTable.keytype[0].skey[0], i);
#endif
		pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKeyLen = i;
		pmib->dot11GroupKeysTable.keyid = pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex;
		pmib->dot11GroupKeysTable.keyInCam = 0;
#endif
	}

// for debug
#if 0
// when hangup reset, re-init TKIP/AES key in ad-hoc mode
#ifdef CLIENT_MODE
	if ((OPMODE & WIFI_ADHOC_STATE) && pmib->dot11OperationEntry.keep_rsnie &&
		(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _TKIP_PRIVACY_ ||
			pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _CCMP_PRIVACY_)) {
		DOT11_SET_KEY Set_Key;
		Set_Key.KeyType = DOT11_KeyType_Group;
		Set_Key.EncType = pmib->dot11GroupKeysTable.dot11Privacy;
		DOT11_Process_Set_Key(priv->dev, NULL, &Set_Key, pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey);
	}
	else
#endif
//-------------------------------------- david+2006-06-30
	// restore group key if it has been set before open, david
	if (pmib->dot11GroupKeysTable.keyInCam) {
		int retVal;
		retVal = CamAddOneEntry(priv, (unsigned char *)"\xff\xff\xff\xff\xff\xff",
					pmib->dot11GroupKeysTable.keyid,
					pmib->dot11GroupKeysTable.dot11Privacy<<2,
					0, pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey);
		if (retVal)
			priv->pshare->CamEntryOccupied++;
		else {
			DEBUG_ERR("Add group key failed!\n");
		}
	}
#endif
	//here add if legacy WEP
	// if 1x is enabled, do not set default key, david
#ifdef USE_WEP_DEFAULT_KEY
#ifdef MBSSID
	if (!(OPMODE & WIFI_AP_STATE) || !priv->pmib->miscEntry.vap_enable)
#endif
	{
		if(!SWCRYPTO && !IEEE8021X_FUN &&
			(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_ ||
			 pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_))
			init_DefaultKey_Enc(priv, NULL, 0);
	}
#endif


    //3 MAC Beacon Tming Related

#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
        //Do Nothing
        // Integrated into code above
        //3      Integrated into SetHwRegHandler(priv, HW_VAR_MAC_CONFIG, &MacCfgPara);
	} else
#endif
	{
	
	//Setup Beacon Interval/interrupt interval, ATIM-WIND ATIM-Interrupt
	RTL_W32(MBSSID_BCN_SPACE, pmib->dot11StationConfigEntry.dot11BeaconPeriod);
	//Setting BCNITV is done by firmware now
//	set_fw_reg(priv, (0xF1000000 | (pmib->dot11StationConfigEntry.dot11BeaconPeriod << 8)), 0, 0);
	// Set max AMPDU aggregation time
//	int max_aggre_time = 0xc0; // in 4us
//	set_fw_reg(priv, (0xFD0000B1|((max_aggre_time << 8) & 0xff00)), 0 ,0);

//	RTL_W32(0x2A4, 0x00006300);
//	RTL_W32(0x2A0, 0xb026007C);
//	delay_ms(1);
//	while(RTL_R32(0x2A0) != 0){};
	//RTL_W16(TBTT_PROHIBIT, 0xc804);
	if ((GET_ROOT(priv)->pmib->miscEntry.vap_enable) || (priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod <= 40))
		RTL_W32(TBTT_PROHIBIT, 0x1df04);
	else
		RTL_W32(TBTT_PROHIBIT, 0x40004);

#ifdef SMART_CONCURRENT_92D
	if (priv->pmib->dot11RFEntry.smcc==1 && priv->pmib->dot11RFEntry.macPhyMode != SINGLEMAC_SINGLEPHY){
		if (priv->pmib->dot11RFEntry.phyBandSelect==PHY_BAND_5G)
			RTL_W8(DRVERLYINT, 3);
		else
			RTL_W8(DRVERLYINT, 3);
	} else
#endif
		RTL_W8(DRVERLYINT, 10);
	RTL_W8(BCNDMATIM, 1);
/*
	if (priv->pshare->rf_ft_var.bcast_to_dzq)
		RTL_W16(ATIMWND, 0x0a);
	else
		RTL_W16(ATIMWND, 5);
*/

#if defined(CONFIG_RTL_8812_SUPPORT)
	if(GET_CHIP_VER(priv)==VERSION_8812E)
	{
		RTL_W16(ATIMWND, 0x3C);
		RTL_W8(REG_92E_DTIM_COUNT_ROOT, priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod);
		RTL_W32(REG_92E_PKT_LIFETIME_CTRL, RTL_R32(REG_92E_PKT_LIFETIME_CTRL) & ~BIT(19));
	}
	else
#endif
	{
		RTL_W16(ATIMWND, 1);
	}
	
/*
	if (!((OPMODE & WIFI_AP_STATE)
#if defined(WDS) && defined(CONFIG_RTK_MESH)
		&& ((priv->pmib->dot11WdsInfo.wdsEnabled && priv->pmib->dot11WdsInfo.wdsPure) || priv->pmib->dot1180211sInfo.meshSilence))
#elif defined(WDS)
		&& priv->pmib->dot11WdsInfo.wdsEnabled && priv->pmib->dot11WdsInfo.wdsPure )
#elif defined(CONFIG_RTK_MESH)	//Mesh Mode but mesh not enable
		&& priv->pmib->dot1180211sInfo.meshSilence )
#else
		)
#endif
	)

		RTL_W16(BCN_DRV_EARLY_INT, RTL_R16(BCN_DRV_EARLY_INT)|BIT(15)); // sw beacon
*/
#ifdef MBSSID
	if (priv->pmib->miscEntry.vap_enable && RTL8192CD_NUM_VWLAN == 1 &&
					priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod < 30)
		//RTL_W16(BCN_DRV_EARLY_INT, (RTL_R16(BCN_DRV_EARLY_INT)&0xf00f) | ((6<<4)&0xff0));
		RTL_W8(DRVERLYINT, 6);
#endif

#if defined(CONFIG_RTL_8812_SUPPORT)
	if(GET_CHIP_VER(priv)==VERSION_8812E)
		RTL_W8(REG_92E_PRE_DL_BCN_ITV, RTL_R8(DRVERLYINT)+1);
#endif
	
#ifdef CONFIG_RTL_92C_SUPPORT
	if (IS_TEST_CHIP(priv) && ((GET_CHIP_VER(priv)==VERSION_8188C) || (GET_CHIP_VER(priv)==VERSION_8192C))) {
		RTL_W8(BCN_CTRL, 0);
		RTL_W8(0x553, 1);
		RTL_W8(BCN_CTRL, EN_BCN_FUNCTION);
	//	RTL_W16(BCN_DMATIME, 0x400); // 1ms

	// for debug
#ifdef CLIENT_MODE
		if (OPMODE & WIFI_ADHOC_STATE)
			RTL_W8(BCN_MAX_ERR, 0xff);
#endif
	} else 
#endif
	{
		RTL_W8(BCN_CTRL, DIS_TSF_UPDATE_N|  DIS_SUB_STATE_N  );
		RTL_W8(BCN_MAX_ERR, 0xff);
		RTL_W16(0x518, 0);
		RTL_W8(0x553, 3);
		if(OPMODE & WIFI_STATION_STATE)
			RTL_W8(0x422, RTL_R8(0x422)^BIT(6));

		if((priv->pmib->dot11WdsInfo.wdsPure == 0)
#ifdef MP_TEST
			&& (!priv->pshare->rf_ft_var.mp_specific)
#endif
  		)
		RTL_W8(BCN_CTRL, RTL_R8(BCN_CTRL) | EN_BCN_FUNCTION | EN_TXBCN_RPT);
	}

//--------------

// By H.Y. advice
//	RTL_W16(_BCNTCFG_, 0x060a);
	if (OPMODE & WIFI_AP_STATE)
		RTL_W16(BCNTCFG, 0x0001);
	else
// for debug
//	RTL_W16(_BCNTCFG_, 0x060a);
	RTL_W16(BCNTCFG, 0x0204);
}

   //3 IMR Related
   //3 Download Firmware Related
	// Ack ISR, and then unmask IMR
#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
	    if (opmode & WIFI_AP_STATE) {
	        GET_HAL_INTERFACE(priv)->InitIMRHandler(priv, RT_OP_MODE_AP);
	    }
		else if (opmode & WIFI_STATION_STATE) {
			GET_HAL_INTERFACE(priv)->InitIMRHandler(priv, RT_OP_MODE_INFRASTRUCTURE);
		}
	    else if (opmode & WIFI_ADHOC_STATE) {
	        GET_HAL_INTERFACE(priv)->InitIMRHandler(priv, RT_OP_MODE_IBSS);
	    }

	    // TODO: Filen, no need to sync !?
	    priv->pshare->InterruptMask = _GET_HAL_DATA(priv)->IntMask[0];
	    priv->pshare->InterruptMask = _GET_HAL_DATA(priv)->IntMask[1];

        // TODO: Filen, check download 8051 firmware
        //Download Firmware

       
        if ( RT_STATUS_SUCCESS != GET_HAL_INTERFACE(priv)->InitFirmwareHandler(priv)) {
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);            
            errorFlag |= DRV_ER_INIT_DLFW;
            GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);               
            printk("InitDownload FW Fail\n");                                                 
        }
        else {         
            printk("InitDownload FW OK \n");
        }
	} else
    
#endif
	{
#if 1
	// Ack ISR, and then unmask IMR
#if 0
	RTL_W32(ISR, RTL_R32(ISR));
	RTL_W32(ISR+4, RTL_R16(ISR+4));
	RTL_W32(IMR, 0x0);
	RTL_W32(IMR+4, 0x0);
	priv->pshare->InterruptMask = _ROK_ | _BCNDMAINT_ | _RDU_ | _RXFOVW_ | _RXCMDOK_;
	priv->pshare->InterruptMask = (IMR_ROK | IMR_VODOK | IMR_VIDOK | IMR_BEDOK | IMR_BKDOK |		\
	IMR_HCCADOK | IMR_MGNTDOK | IMR_COMDOK | IMR_HIGHDOK | 					\
	IMR_BDOK | IMR_RXCMDOK | /*IMR_TIMEOUT0 |*/ IMR_RDU | IMR_RXFOVW	|			\
	IMR_BcnInt/* | IMR_TXFOVW*/ /*| IMR_TBDOK | IMR_TBDER*/);// IMR_ROK | IMR_BcnInt | IMR_RDU | IMR_RXFOVW | IMR_RXCMDOK;
#endif
	//priv->pshare->InterruptMask = HIMR_ROK | HIMR_BCNDMA0 | HIMR_RDU | HIMR_RXFOVW;

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
		priv->pshare->InterruptMask = HIMR_88E_ROK | HIMR_88E_HISR1_IND_INT;
		priv->pshare->InterruptMaskExt = HIMRE_88E_RXFOVW;
	} else
#endif
#if defined(CONFIG_RTL_8812_SUPPORT)
	if (GET_CHIP_VER(priv)==VERSION_8812E) {
		priv->pshare->InterruptMask = HIMR_92E_ROK | HIMR_92E_HISR1_IND_INT;
		priv->pshare->InterruptMaskExt = HIMRE_92E_RXFOVW;
	} else
#endif
	{
		priv->pshare->InterruptMask = HIMR_ROK | HIMR_BCNDMA0 | HIMR_RXFOVW;
		priv->pshare->InterruptMaskExt = 0;
	}

#ifdef MP_TEST
	if (priv->pshare->rf_ft_var.mp_specific) {
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8188E)
			priv->pshare->InterruptMask |= HIMR_88E_BEDOK;
		else
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8812E)
			priv->pshare->InterruptMask |= IMR_BEDOK_8812;
		else
#endif
#ifdef CONFIG_WLAN_HAL
        //3 Integrated into HAL code
#endif
			priv->pshare->InterruptMask	|= HIMR_BEDOK;
	}
#endif

	if (opmode & WIFI_AP_STATE) {
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8188E)
			priv->pshare->InterruptMask |= HIMR_88E_BcnInt | HIMR_88E_TBDOK | HIMR_88E_TBDER;
		else
#endif
#if defined(CONFIG_RTL_92E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT)
		if ((GET_CHIP_VER(priv)==VERSION_8192E) || (GET_CHIP_VER(priv)==VERSION_8812E))
			priv->pshare->InterruptMask |= HIMR_92E_BcnInt | HIMR_92E_TBDOK | HIMR_92E_TBDER;
		else
#endif
			priv->pshare->InterruptMask |= HIMR_BCNDOK0 | HIMR_TXBCNERR;
	}
#ifdef CLIENT_MODE
	else if (opmode & WIFI_ADHOC_STATE) {
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8188E)
			priv->pshare->InterruptMaskExt |= HIMR_88E_BcnInt | HIMR_88E_TBDOK | HIMR_88E_TBDER;
		else
#endif
#if defined(CONFIG_RTL_8812_SUPPORT)
		if (GET_CHIP_VER(priv)==VERSION_8812E)
			priv->pshare->InterruptMaskExt |= HIMR_92E_BcnInt | HIMR_92E_TBDOK | HIMR_92E_TBDER;
		else
#endif
			priv->pshare->InterruptMaskExt |= (HIMR_TXBCNERR | HIMR_TXBCNOK);
	}
#endif

#endif

	// FGPA does not have eeprom now
//	RTL_W8(_9346CR_, 0);
/*
	// ===========================================================================================
	// Download Firmware
	// allocate memory for tx cmd packet
	if((priv->pshare->txcmd_buf = (unsigned char *)kmalloc((LoadPktSize), GFP_ATOMIC)) == NULL) {
		printk("not enough memory for txcmd_buf\n");
		return -1;
	}

	priv->pshare->cmdbuf_phyaddr = get_physical_addr(priv, priv->pshare->txcmd_buf,
			LoadPktSize, PCI_DMA_TODEVICE);
*/

#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
	if (
#ifdef CONFIG_RTL_92C_SUPPORT
		(GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
#ifdef CONFIG_RTL_92C_SUPPORT
		||
#endif
		(GET_CHIP_VER(priv)==VERSION_8192D)
#endif
		) {
		/* currently need not to download fw	*/
		rtl8192cd_ReadFwHdr(priv);

		while(dwnRetry-- && !fwStatus) {
#ifdef CONFIG_RTL_92D_SUPPORT
			if (GET_CHIP_VER(priv)==VERSION_8192D)
				fwStatus = Load_92D_Firmware(priv);
#endif
#ifdef CONFIG_RTL_92C_SUPPORT
			if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C))
				fwStatus = Load_92C_Firmware(priv);
#endif
			delay_ms(20);
		};
		if(fwStatus) {
			DEBUG_INFO("Load firmware successful!\n");
		} else {
			DEBUG_INFO("Load firmware check!\n");
#ifdef PCIE_POWER_SAVING
			priv->pshare->rf_ft_var.power_save &= ~( L1_en|L2_en);
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
			if (GET_CHIP_VER(priv)==VERSION_8192D){
				if (RTL_R8(0x1c5)==0xE0){
					DEBUG_INFO("RTL8192D part number failed!!\n");
					return -1;
				}
			}
#endif
		}
	}
#endif


#ifdef CONFIG_RTL_8812_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8812E) {
		extern RT_STATUS FirmwareDownload8812(struct rtl8192cd_priv *priv);
		priv->bFWReady = ! (0 ^ FirmwareDownload8812(priv));
	}
#endif

	}
	
	printk("errorFlag:%08x \n", errorFlag);
	
/*
	MacConfigAfterFwDownload(priv);
*/

	// Adaptive Rate Table for Basic Rate
	val32 = 0;
	for (i=0; i<32; i++) {
		if (AP_BSSRATE[i]) {
			if (AP_BSSRATE[i] & 0x80)
				val32 |= get_bit_value_from_ieee_value(AP_BSSRATE[i] & 0x7f);
		}
	}
	val32 |= (priv->pmib->dot11nConfigEntry.dot11nBasicMCS << 12);

#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
        // Do Nothing
        // Filen: it is not necessary to check !?
	} else
#endif
	{
		unsigned int delay_count = 10;
		do {
			if (!is_h2c_buf_occupy(priv))
				break;
			delay_us(5);
			delay_count--;
		} while (delay_count);
	}

#ifdef P2P_SUPPORT
	if(OPMODE&WIFI_P2P_SUPPORT){
		P2P_DEBUG("managment frame G only \n");
		set_RATid_cmd(priv, 0, ARFR_G_ONLY, val32);	// under P2P mode
	}else
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8192D) {
		if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
#ifdef USB_POWER_SUPPORT
			val32 &= USB_RA_MASK;
#endif
			set_RATid_cmd(priv, 0, ARFR_Band_A_BMC, val32);
		}
		else
			set_RATid_cmd(priv, 0, ARFR_BMC, val32);
	}
#ifdef CONFIG_RTL_92C_SUPPORT
	else if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)){
                 set_RATid_cmd(priv, 0, ARFR_BMC, val32);
        }
#endif
        else
#endif
	{
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)	
		set_RATid_cmd(priv, 0, ARFR_BMC, val32);
#endif
	}

//	kfree(priv->pshare->txcmd_buf);

#ifdef MP_TEST
	if (!priv->pshare->rf_ft_var.mp_specific)
#endif
	if (opmode & WIFI_AP_STATE)
	{
		if (priv->auto_channel == 0) {
			DEBUG_INFO("going to init beacon\n");
			init_beacon(priv);
		}
	}

    //3 Enable IMR
#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
        #if 0   //Filen: defer to open after drv_open
    	GET_HAL_INTERFACE(priv)->EnableIMRHandler(priv);
        #endif
	} else
#endif
	{
	//enable interrupt
#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
		RTL_W32(REG_88E_HIMR, priv->pshare->InterruptMask);
		RTL_W32(REG_88E_HIMRE, priv->pshare->InterruptMaskExt);
	} else
#endif

#ifdef CONFIG_RTL_8812_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8812E) {
#ifdef DRVMAC_LB
		priv->pshare->InterruptMask &= ~HIMR_92E_BcnInt;
#endif		
		RTL_W32(REG_HIMR0_8812, priv->pshare->InterruptMask);
		RTL_W32(REG_HIMR1_8812, priv->pshare->InterruptMaskExt);
	} else
#endif
	{
		RTL_W32(HIMR, priv->pshare->InterruptMask);
	}
//	RTL_W32(IMR+4, priv->pshare->InterruptMaskExt);
//	RTL_W32(IMR, 0xffffffff);
//	RTL_W8(IMR+4, 0x3f);
	}

	// ===========================================================================================

#ifdef CHECK_HANGUP
	if (priv->reset_hangup)
		priv->pshare->CurrentChannelBW = priv->pshare->is_40m_bw;
	else
#endif
	{
		if (opmode & WIFI_AP_STATE)
			priv->pshare->CurrentChannelBW = priv->pshare->is_40m_bw;
		else
			priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_20;
	}

#ifdef SUPPORT_RTL8188E_TC
	if (!((GET_CHIP_VER(priv)==VERSION_8188E) && IS_TEST_CHIP(priv)))
#endif
	{
#ifdef HIGH_POWER_EXT_PA
		if (!priv->pshare->rf_ft_var.use_ext_pa)
#endif
		{
			// get default Tx AGC offset
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[0])  = cpu_to_be32(RTL_R32(rTxAGC_A_Mcs03_Mcs00));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[4])  = cpu_to_be32(RTL_R32(rTxAGC_A_Mcs07_Mcs04));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[8])  = cpu_to_be32(RTL_R32(rTxAGC_A_Mcs11_Mcs08));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[12]) = cpu_to_be32(RTL_R32(rTxAGC_A_Mcs15_Mcs12));
			*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[0]) = cpu_to_be32(RTL_R32(rTxAGC_A_Rate18_06));
			*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[4]) = cpu_to_be32(RTL_R32(rTxAGC_A_Rate54_24));
			*(unsigned int *)(&priv->pshare->phw->CCKTxAgc_A[0]) = cpu_to_be32((RTL_R32(rTxAGC_A_CCK11_2_B_CCK11) & 0xffffff00)
				| RTL_R8(rTxAGC_A_CCK1_Mcs32 + 1));

#if defined(CONFIG_RTL_92D_SUPPORT)&& defined(CONFIG_RTL_92D_DMDP)
			if (priv->pmib->dot11RFEntry.macPhyMode==DUALMAC_DUALPHY && priv->pshare->wlandev_idx == 1) {
				*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[0])	= cpu_to_be32(RTL_R32(rTxAGC_B_Mcs03_Mcs00));
				*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[4])	= cpu_to_be32(RTL_R32(rTxAGC_B_Mcs07_Mcs04));
				*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[8])	= cpu_to_be32(RTL_R32(rTxAGC_B_Mcs11_Mcs08));
				*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[12]) = cpu_to_be32(RTL_R32(rTxAGC_B_Mcs15_Mcs12));
				*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[0]) = cpu_to_be32(RTL_R32(rTxAGC_B_Rate18_06));
				*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[4]) = cpu_to_be32(RTL_R32(rTxAGC_B_Rate54_24));
				*(unsigned int *)(&priv->pshare->phw->CCKTxAgc_A[0]) = cpu_to_be32((RTL_R8(rTxAGC_A_CCK11_2_B_CCK11) << 24)
					| (RTL_R32(rTxAGC_B_CCK5_1_Mcs32) >> 8));
			}
#endif	

			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[0])  = cpu_to_be32(RTL_R32(rTxAGC_B_Mcs03_Mcs00));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[4])  = cpu_to_be32(RTL_R32(rTxAGC_B_Mcs07_Mcs04));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[8])  = cpu_to_be32(RTL_R32(rTxAGC_B_Mcs11_Mcs08));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[12]) = cpu_to_be32(RTL_R32(rTxAGC_B_Mcs15_Mcs12));
			*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_B[0]) = cpu_to_be32(RTL_R32(rTxAGC_B_Rate18_06));
			*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_B[4]) = cpu_to_be32(RTL_R32(rTxAGC_B_Rate54_24));
			*(unsigned int *)(&priv->pshare->phw->CCKTxAgc_B[0]) = cpu_to_be32((RTL_R8(rTxAGC_A_CCK11_2_B_CCK11) << 24)
				| (RTL_R32(rTxAGC_B_CCK5_1_Mcs32) >> 8));

		}
	}
#ifdef ADD_TX_POWER_BY_CMD
	assign_txpwr_offset(priv);
#endif

#ifdef TXPWR_LMT
	if (!priv->pshare->rf_ft_var.disable_txpwrlmt)
		PHY_ConfigTXLmtWithParaFile(priv);
#endif

#ifdef _TRACKING_TABLE_FILE
		PHY_ConfigTXPwrTrackingWithParaFile(priv);
#endif


#ifdef CONFIG_RTL_88E_SUPPORT //for 88e tx power tracking
	if(GET_CHIP_VER(priv) == VERSION_8188E){
		if ((priv->pmib->dot11RFEntry.ther < 0x07) || (priv->pmib->dot11RFEntry.ther > 0x32)) {
			DEBUG_ERR("TPT: unreasonable target ther %d, disable tpt\n", priv->pmib->dot11RFEntry.ther);
			priv->pmib->dot11RFEntry.ther = 0;
		}
	}
	else
#endif
	if ((priv->pmib->dot11RFEntry.ther < 0x07) || (priv->pmib->dot11RFEntry.ther > 0x1d)) {
		DEBUG_ERR("TPT: unreasonable target ther %d, disable tpt\n", priv->pmib->dot11RFEntry.ther);
		priv->pmib->dot11RFEntry.ther = 0;
	}

/*
	if (opmode & WIFI_AP_STATE)
	{
		if (priv->auto_channel == 0) {
			DEBUG_INFO("going to init beacon\n");
			init_beacon(priv);
		}
	}
*/

#ifdef CONFIG_WLAN_HAL
    if(IS_HAL_CHIP(priv)) {
        #ifdef CONFIG_WLAN_HAL_8881A        
        if (GET_CHIP_VER(priv) == VERSION_8881A) {
            //Don't enable BB here
        }
        #endif // CONFIG_WLAN_HAL_8881A         

        #ifdef CONFIG_WLAN_HAL_8192EE
        if (GET_CHIP_VER(priv) == VERSION_8192E) {
            // TODO: Filen, check 8192E BB/RF control
            //Don't enable BB here
        }
        #endif //CONFIG_WLAN_HAL_8192EE
    }
    else
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
    if(GET_CHIP_VER(priv)!= VERSION_8812E)
#endif
    {
    	/*---- Set CCK and OFDM Block "ON"----*/
    	PHY_SetBBReg(priv, rFPGA0_RFMOD, bOFDMEn, 1);
#if defined(CONFIG_RTL_92D_SUPPORT)
    	if (!(priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G))
#endif
    		PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, 1);
    }

	delay_ms(2);

#ifdef MP_TEST
	if (priv->pshare->rf_ft_var.mp_specific) {
#ifdef CONFIG_WLAN_HAL
        if( IS_HAL_CHIP(priv) ) {
            u4Byte  MACAddr = 0x87654321;
            GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_ETHER_ADDR, (pu1Byte)&MACAddr);
        }
        else
#endif //CONFIG_WLAN_HAL
	    RTL_W32(MACID, 0x87654321);

		delay_ms(150);
	}
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8192D) {
#ifdef SW_LCK_92D
		PHY_LCCalibrate_92D(priv);
#endif
		SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
		SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);

		if (priv->pmib->dot11RFEntry.macPhyMode == SINGLEMAC_SINGLEPHY) 			
			clnt_ss_check_band(priv, priv->pmib->dot11RFEntry.dot11channel);			

	/*
	 *	IQK
	 */
		PHY_IQCalibrate(priv);

#ifdef DPK_92D
		if (priv->pmib->dot11RFEntry.phyBandSelect==PHY_BAND_5G && priv->pshare->rf_ft_var.dpk_on)
			PHY_DPCalibrate(priv);
#endif

#ifdef SMART_CONCURRENT_92D
		if (priv->pmib->dot11RFEntry.smcc==1 && priv->pmib->dot11RFEntry.macPhyMode != SINGLEMAC_SINGLEPHY){
			if((priv->MAC_info = (struct SMCC_MAC_Info_Tbl*)kmalloc(sizeof(struct SMCC_MAC_Info_Tbl), GFP_ATOMIC)) == NULL) {
					printk("kmalloc SMCC_MAC_Info_Tbl not enough memory\n");
					return -1;
			}
			memset(priv->MAC_info, 0, sizeof(struct SMCC_MAC_Info_Tbl));
			smcc_92D_fill_MAC_info(priv, priv->MAC_info);

			//if (priv->pmib->dot11RFEntry.smcc==1 && priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_SINGLEPHY)
			//		priv->pmib->dot11RFEntry.macPhyMode = DUALMAC_DUALPHY;

			/* 	Because our init flow is wlan0 5G enabled -> wlan1 2G enabled,
			 * 	SMCC collects IQC parameters by enabling wlan0 through 2x2A0->1x1A0 first.
			 *	Then we enable wlan1 through 1x1A0G1->2x2G1->1x1A0G1. -- Chris
			 */
			if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G) {
				smcc_92D_enable1x1_5G(priv,0);
				smcc_92D_fill_MAC_info(priv, priv->MAC_info);
				smcc_dump_MAC_info(priv, priv->MAC_info);
				smcc_signin_MAC_info(priv, priv->MAC_info);
			}
			else {
				smcc_92D_enable2x2_2G(priv);
				smcc_92D_fill_MAC_info(priv, priv->MAC_info);
				smcc_dump_MAC_info(priv, priv->MAC_info);
				smcc_signin_MAC_info(priv, priv->MAC_info);
				// Restore 1x1_A0G1
				smcc_92D_enable1x1_5G(priv,1);
			}

			priv->smcc_state = 0;
			smcc_signin_linkstate(priv, 1, priv->pmib->dot11RFEntry.smcc_t, priv->smcc_state);
			priv->pmib->dot11RFEntry.macPhyMode = DUALMAC_SINGLEPHY;
			priv->pshare->phw->MIMO_TR_hw_support = MIMO_2T2R;

			if (priv->pshare->wlandev_idx == 1){
				RTL_W8(0x553, 1); // rst bcn0
				DMDP_RTL_W8(0, 0x553, 1); // rst bcn0
				printk("MAC0-TSF %x MAC1-TSF %x\n", DMDP_RTL_R32(0, TSFTR), RTL_R32(TSFTR));
				DMDP_RTL_W8(0, 0x1c2, 0x11); // enable mac0 smart concurrent
				RTL_W8(0x1c2, 0x11); // enable mac1 smart concurrent
			}
		}
#endif
	}
#endif // CONFIG_RTL_92D_SUPPORT

#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT)
	if (
#ifdef CONFIG_RTL_92C_SUPPORT
		(GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
#ifdef CONFIG_RTL_92C_SUPPORT
		||
#endif
		(GET_CHIP_VER(priv) == VERSION_8188E)
#endif
		) {
#ifdef CONFIG_RTL_88E_SUPPORT
		if(GET_CHIP_VER(priv) == VERSION_8188E) {
			// switch to channel 7 before doing IQK
			printk("Switch to channel 7 before doing 88E IQK\n");
			SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
			SwChnl(priv, 7, priv->pshare->offset_2nd_chan);
		}
#endif
		PHY_IQCalibrate(priv);		// IQK_92C IQK_88c IQK_88e

#if defined(CONFIG_RTL865X_WTDOG) || defined(CONFIG_RTL_WTDOG)
#if defined(__LINUX_2_6__) && !defined(NOT_RTK_BSP)
#ifdef CONFIG_RTL_8198B
		REG32(BSP_WDTCNTRR) |= BSP_WDT_KICK;
#else
		REG32(_WDTCNR_) |=  1 << 23;
#endif
#endif
#endif

		//Do NOT perform APK fot RF team's request
		//PHY_APCalibrate(priv);		// APK_92C  APK_88C
#ifdef CALIBRATE_BY_ODM
		if(GET_CHIP_VER(priv) == VERSION_8188E) {
			PHY_LCCalibrate_8188E(ODMPTR);
		} else
#endif			
		{
			PHY_LCCalibrate(priv);
		}

		SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
		SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);

	/*
		 *	Set RF & RRSR depends on band in use
		 */
		if (priv->pmib->dot11BssType.net_work_type & (WIRELESS_11G |WIRELESS_11N)) {
			if ((priv->pmib->dot11StationConfigEntry.autoRate) || !(priv->pmib->dot11StationConfigEntry.fixedTxRate & 0xf)) {
#ifdef CONFIG_RTL_92C_SUPPORT
				if( IS_UMC_A_CUT_88C(priv) || GET_CHIP_VER(priv) == VERSION_8192C )
					PHY_SetRFReg(priv, 0, 0x26, bMask20Bits, 0x4f000);
				else
#endif
					PHY_SetRFReg(priv, 0, 0x26, bMask20Bits, 0x4f200);

//				RTL_W32(RRSR, RTL_R32(RRSR) & ~(0x0c));
			} else {
				PHY_SetRFReg(priv, 0, 0x26, bMask20Bits, 0x0f400);
			}
		} else {
			PHY_SetRFReg(priv, 0, 0x26, bMask20Bits, 0x0f400);
		}
	}
#endif // CONFIG_RTL_92C_SUPPORT || CONFIG_RTL_88E_SUPPORT
/*
	if(priv->pshare->rf_ft_var.ofdm_1ss_oneAnt == 1){// use one PATH for ofdm and 1SS
		Switch_1SS_Antenna(priv, 2);
		Switch_OFDM_Antenna(priv, 2);
	}
*/

#if defined(CONFIG_RTL865X_WTDOG) || defined(CONFIG_RTL_WTDOG)
#if defined(__LINUX_2_6__) && !defined(NOT_RTK_BSP)
#ifdef CONFIG_RTL_8198B
		REG32(BSP_WDTCNTRR) |= BSP_WDT_KICK;
#else
		REG32(_WDTCNR_) |=  1 << 23;
#endif
#endif
#endif
	delay_ms(100);


	//RTL_W32(0x100, RTL_R32(0x100) | BIT(14)); //for 8190 fw debug

	// init DIG variables
	val32 = 0x40020064;	// 0x20010020
	priv->pshare->threshold0 = (unsigned short)(val32&0x000000FF);
	priv->pshare->threshold1 = (unsigned short)((val32&0x000FFF00)>>8);
	priv->pshare->threshold2 = (unsigned short)((val32&0xFFF00000)>>20);
	priv->pshare->digDownCount = 0;
	priv->pshare->digDeadPoint = 0;
	priv->pshare->digDeadPointHitCount = 0;


//#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A)
//if((GET_CHIP_VER(priv)== VERSION_8812E) || (GET_CHIP_VER(priv)== VERSION_8881A)) {
	
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
if((GET_CHIP_VER(priv)== VERSION_8812E) || IS_HAL_CHIP(priv)) {

    //Do Nothing
}
else
#endif
{
// 2009.09.10
	if (priv->pshare->rf_ft_var.cck_tx_pathB) {
		RTL_W8(0xa07, 0x40);	// 0x80 -> 0x40    CCK path B Tx
		RTL_W8(0xa0b, 0x84);	// 0x88 -> 0x84    CCK path B Tx
	}
	

#ifdef INTERFERENCE_CONTROL
	set_DIG_state(priv, 1);		// DIG on

	if (priv->pshare->rf_ft_var.nbi_filter_enable) {
		priv->pshare->phw->nbi_filter_on = 1;
		RTL_W16(rOFDM0_RxDSP, RTL_R16(rOFDM0_RxDSP) | BIT(9));		// NBI on
	}
#endif

// CCK path A Tx
#ifdef CONFIG_POCKET_ROUTER_SUPPORT
#ifdef CONFIG_RTL_92D_SUPPORT
	if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G)
#endif
	{
		RTL_W8(0xa07, (RTL_R8(0xa07) & 0xbf));
		RTL_W8(0xa0b, (RTL_R8(0xa0b) & 0xfb));
	}
#endif

#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8198B)
	RTL_W8(AGGR_BK_TIME, 0x18);
	RTL_W16(0x4ca, 0x0a0a);
//	RTL_W32(RESP_SIFS_CCK, 0x0e0e0a0a);
	//RTL_W32(ACKTO, 0x40001440);
	RTL_W16(ACKTO, 0x1440);
	RTL_W16(RXFLTMAP2, 0xffff);
	//RTL_W16(RCR, RTL_R16(RCR)&(~ BIT(11)));
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	//CBN debug
	if (GET_CHIP_VER(priv)==VERSION_8192D) {
#ifndef SMART_CONCURRENT_92D
//		RTL_W32(RD_CTRL, RTL_R32(RD_CTRL)|BIT(13)); // enable force tx beacon
		RTL_W8(BCN_MAX_ERR, 0); // tx beacon error threshold
#endif
//		RTL_W16(EIFS, 0x0040);	// eifs < tbtt_prohibit
		if (opmode & WIFI_AP_STATE)
			RTL_W16(rFPGA0_RFTiming1, 0x5388);
	}
#endif

#ifdef CONFIG_WLAN_HAL
    if (IS_HAL_CHIP(priv)) {
    }
    else
#endif //CONFIG_WLAN_HAL
    {
        RTL_W16(EIFS, 0x0040);	// eifs = 40 us
    	RTL_W32(0x350, RTL_R32(0x350) | BIT(26));	// tx status check
    }

#ifdef HIGH_POWER_EXT_PA
	if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)){
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		priv->pmib->dot11RFEntry.trswitch = 1;
		PHY_SetBBReg(priv, 0x870, BIT(10), 0);
		if (GET_CHIP_VER(priv) == VERSION_8192C)
			PHY_SetBBReg(priv, 0x870, BIT(26), 0);
	}
	}
#endif

#if defined(SW_ANT_SWITCH) || defined(HW_ANT_SWITCH)
//	priv->pmib->dot11RFEntry.trswitch = 1;
#endif

	if (priv->pmib->dot11RFEntry.trswitch)
		RTL_W8(GPIO_MUXCFG, RTL_R8(GPIO_MUXCFG) | TRSW0EN);
	else
		RTL_W8(GPIO_MUXCFG, RTL_R8(GPIO_MUXCFG) & ~TRSW0EN);

#ifdef DFS
        if (!priv->pmib->dot11DFSEntry.disable_DFS) {
                RTL_W8(0xc50, 0x24);
                delay_us(10);
                RTL_W8(0xc50, 0x20);
        }
#endif

	if (get_rf_mimo_mode(priv) == MIMO_2T2R)
	{
		if (priv->pmib->dot11RFEntry.tx2path)
		{
			RTL_W32(0x90C, 0x83321333);
			RTL_W32(0x80C, RTL_R32(0x80C) & ~BIT(31));
			RTL_W8(0x6D8, RTL_R8(0x6D8) | 0x3F);

			RTL_W8(0xA07, 0xC1);
			RTL_W8(0xA11, RTL_R8(0xA11) & ~BIT(5));
			RTL_W8(0xA20, (RTL_R8(0xA20) & ~BIT(5)) | BIT(4));
			RTL_W8(0xA2E, RTL_R8(0xA2E) | BIT(3) | BIT(2));
			RTL_W8(0xA2F, (RTL_R8(0xA2F) & ~BIT(5)) | BIT(4));
			RTL_W8(0xA75, RTL_R8(0xA75) | BIT(0));
			RTL_W32(0xC8C, 0xa0240000);
			RTL_W8(0x800, RTL_R8(0x800) & ~BIT(1));
		}

		// TX Beamforming
		if (priv->pmib->dot11RFEntry.txbf)
			PHY_SetBBReg(priv, 0x90C, BIT(30), 1);
		else
			PHY_SetBBReg(priv, 0x90C, BIT(30), 0);
	}
	else {
		if (priv->pmib->dot11RFEntry.tx2path) {
			DEBUG_INFO("Not 2T2R, disable tx2path\n");
			priv->pmib->dot11RFEntry.tx2path = 0;
		}
		if (priv->pmib->dot11RFEntry.txbf) {
			DEBUG_INFO("Not 2T2R, disable txbf\n");
			priv->pmib->dot11RFEntry.txbf = 0;
		}
	}
}

#ifdef DRVMAC_LB
if (!priv->pmib->miscEntry.drvmac_lb)
#endif
{
#ifdef USE_OUT_SRC
	if(IS_OUTSRC_CHIP(priv))
        ODM_DMInit(ODMPTR);			// DM parameters init
#endif

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A)
	if ((GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8881A)) {

        phy_BB8192CD_Check_ParaFile(priv);
 
		priv->pshare->No_RF_Write = 0;
		SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
		SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);
		priv->pshare->No_RF_Write = 1;
		if(priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
			RTL_W32(0x808, RTL_R32(0x808)|BIT(29));
			RTL_W8(0x454, RTL_R8(0x454) | BIT(7));
		}
		else {
			RTL_W32(0x808, RTL_R32(0x808)|BIT(29)|BIT(28));
			RTL_W8(0x454, RTL_R8(0x454) & ~ BIT(7));
		}
		RTL_W16(0x4ca, 0x1f1f);
		
       if (get_rf_mimo_mode(priv) == MIMO_2T2R) {
    		RTL_W8(0x44b, 0x3c);		// 2SS MCS7 enable, 1T1R no use
       }
        
//		RTL_W32(0x2c, 0x28a3e200);

        if(GET_CHIP_VER(priv) == VERSION_8812E) {
            RTL_W32(0x2c, 0x4103e200);
        }
        else if(GET_CHIP_VER(priv) == VERSION_8881A) {
            #if 0
            u4Byte  TempValue;

            //Test chip default value: 0x300007c4
            TempValue = RTL_R32(0x2c);

            //Clear XIN / XOUT value
            TempValue = TempValue & ~(0xFFF000);

            //Set XIN / XOUT value
            TempValue = TempValue | (0x20<<12) | (0x20<<18);
            
            RTL_W32(0x2c, TempValue);
            #endif
    		//RTL_W32(0x2c, 0x4103e200);
            printk("\n0x2C[31:0]=0x%x\n", RTL_R32(0x2c));
        } 
        else {
        }
            
		RTL_W32(0x864, 0x796a6c32);

#ifdef FOR_VHT5G_PF2 //8812 cca
		if(((GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8881A))
            && priv->pshare->rf_ft_var.use_cca)
		{
			RTL_W32(0x4c4, (RTL_R32(0x4c4)|BIT(19)));
			RTL_W32(0x838, (RTL_R32(0x838)|BIT(0)));
			RTL_W8(0x577, RTL_R8(0x577)|3);
			printk("0x838 = 0x%x, 0x4c4= 0x%x\n", RTL_R32(0x838), RTL_R32(0x4c4));
		}
#endif

#if 1
//	Path A only
		if(get_rf_mimo_mode(priv) == MIMO_1T1R) {
			RTL_W8(0x808, 0x11);
			RTL_W16(0x80c, 0x1111);
			//RTL_W8(0xa07, (RTL_R8(0xa07) & 0xf3)| 0x04 );	// [3:2] = 2'b 01
			RTL_W32(0x8bc, (RTL_R32(0x8bc)&0x3FFFFF9F)|BIT(30) );
			RTL_W8(0xe00, (RTL_R8(0xe00) & 0xf0)| 0x04 );
			RTL_W8(0xe90, 0);
			RTL_W32(0xe60, 0);
			RTL_W32(0xe64, 0);
		}
#endif

	PHY_IQCalibrate(priv); //FOR_8812_IQK

#if 1 //for 8812 IOT issue with 11N NICs
	printk("0x838 B(1)= 0, 0x456 = 0x32 \n");
	RTL_W8(0x838, (RTL_R8(0x838)& ~ BIT(0))); //Disbale CCA 
	RTL_W8(0x456, 0x32); //Refine AMPDU MAX duration
	//RTL_W8(0x44b, 0x3c); //Disble MCS7 of RA
#endif

	}
#endif
}

#if	defined(CONFIG_WLAN_HAL_8192EE)

	if(GET_CHIP_VER(priv) == VERSION_8192E) {
		PHY_SetRFReg(priv, RF92CD_PATH_A, 0xb1, bMask20Bits, 0x55418);		// LCK
		PHY_IQCalibrate(priv);
//
//		PHY_ConfigBBWithParaFile(priv, PHYREG);
		SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
		SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);
	}
#endif


#if 0 // TODO: Filen, Because AGC ,temp setting
#ifdef CONFIG_WLAN_HAL_8881A
    if (GET_CHIP_VER(priv) == VERSION_8881A) {
        RTL_W8(0x813, (RTL_R8(0x813)&~0xF0) | 0x20);
    }
#endif //CONFIG_WLAN_HAL_8881A
#endif

	DBFEXIT;

	return 0;

}


#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
static void rtl8192cd_ReadFwHdr(struct rtl8192cd_priv *priv)
{
	struct __RTL8192C_FW_HDR__ *pFwHdr = NULL;
	unsigned char *swap_arr;

#ifdef MP_TEST
	if (priv->pshare->rf_ft_var.mp_specific)
		return;
#endif
#ifdef __ECOS
	unsigned char hdr_buf[RT_8192CD_FIRMWARE_HDR_SIZE];
	swap_arr = hdr_buf;
#endif
#ifdef __KERNEL__
	swap_arr = kmalloc(RT_8192CD_FIRMWARE_HDR_SIZE, GFP_ATOMIC);
	if (swap_arr == NULL)
		return;
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8192D){
		memcpy(swap_arr, data_rtl8192dfw_n_start, RT_8192CD_FIRMWARE_HDR_SIZE);
	}
#endif

#ifdef CONFIG_RTL_92C_SUPPORT
	if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)){
#ifdef TESTCHIP_SUPPORT
		if (IS_TEST_CHIP(priv))
			memcpy(swap_arr, data_rtl8192cfw_start, RT_8192CD_FIRMWARE_HDR_SIZE);
		else
#endif
		{
			if( IS_UMC_A_CUT_88C(priv) )
				memcpy(swap_arr, data_rtl8192cfw_ua_start, RT_8192CD_FIRMWARE_HDR_SIZE);
			else
				memcpy(swap_arr, data_rtl8192cfw_n_start, RT_8192CD_FIRMWARE_HDR_SIZE);
		}
	}
#endif

	pFwHdr = (struct __RTL8192C_FW_HDR__ *)swap_arr;
#ifdef _BIG_ENDIAN_
	pFwHdr->signature		= le16_to_cpu(pFwHdr->signature);
	pFwHdr->version		= le16_to_cpu(pFwHdr->version);
	pFwHdr->year		= le16_to_cpu(pFwHdr->year);	// ready on after v33.1
#endif

	priv->pshare->fw_signature	= pFwHdr->signature;
	priv->pshare->fw_category		= pFwHdr->category;
	priv->pshare->fw_function		= pFwHdr->function;
	priv->pshare->fw_version		= pFwHdr->version;
	priv->pshare->fw_sub_version	= pFwHdr->subversion;
	priv->pshare->fw_date_month	= pFwHdr->month;
	priv->pshare->fw_date_day	= pFwHdr->day;
	priv->pshare->fw_date_hour	= pFwHdr->hour;
	priv->pshare->fw_date_minute	= pFwHdr->minute;
#ifdef __KERNEL__
	kfree(swap_arr);
#endif
/*
	printk("fw_signature: ");
	if (priv->pshare->fw_signature == RTL8192C_TEST_CHIP)
		printk("92C_TEST_CHIP");
	else if (priv->pshare->fw_signature == RTL8188C_TEST_CHIP)
		printk("88C_TEST_CHIP");
	else if (priv->pshare->fw_signature == RTL8192C_MP_CHIP_A)
		printk("92C_MP_CHIP_A");
	else if (priv->pshare->fw_signature == RTL8188C_MP_CHIP_A)
		printk("88C_MP_CHIP_A");
	else if (priv->pshare->fw_signature == RTL8192C_MP_CHIP_B)
		printk("92C_MP_CHIP_B");
	else if (priv->pshare->fw_signature == RTL8188C_MP_CHIP_B)
		printk("88C_MP_CHIP_B");
	printk(", ");

	printk("fw_category: ");
	if (priv->pshare->fw_category == RTL8192C_NIC_PCIE)
		printk("92C_NIC_PCIE");
	else if (priv->pshare->fw_category == RTL8192C_NIC_USB)
		printk("92C_NIC_USB");
	else if (priv->pshare->fw_category == RTL8192C_AP_PCIE)
		printk("92C_AP_PCIE");
	else if (priv->pshare->fw_category == RTL8192C_AP_USB)
		printk("92C_AP_USB");
	printk(", ");

	printk("fw_function: ");
	if (priv->pshare->fw_function == RTL8192C_NIC_NORMAL)
		printk("92C_NIC_NORMAL");
	else if (priv->pshare->fw_function == RTL8192C_NIC_WWLAN)
		printk("92C_NIC_WWLAN");
	else if (priv->pshare->fw_function == RTL8192C_AP_NORMAL)
		printk("92C_AP_NORMAL");
	else if (priv->pshare->fw_function == RTL8192C_AP_SUSPEND)
		printk("92C_AP_SUSPEND");
	printk("\n");

	printk("fw_version: %d.%d, ", priv->pshare->fw_version, priv->pshare->fw_sub_version);
	printk("fw_date: %02x-%02x %02x:%02x\n", priv->pshare->fw_date_month, priv->pshare->fw_date_day,
		priv->pshare->fw_date_hour, priv->pshare->fw_date_minute);
*/
}
#endif


#ifdef CONFIG_RTL_92C_SUPPORT
static int Load_92C_Firmware(struct rtl8192cd_priv *priv)
{
	int fw_len, wait_cnt=0;
	unsigned int CurPtr=0;
	unsigned int WriteAddr;
	unsigned int Temp;
	unsigned char *ptmp;

#ifdef CONFIG_RTL8672
	printk("val=%x\n", RTL_R8(0x80));
#endif

#ifdef MP_TEST
	if (priv->pshare->rf_ft_var.mp_specific)
		return TRUE;
#endif

	printk("===> %s\n", __FUNCTION__);

#ifdef TESTCHIP_SUPPORT
	if (IS_TEST_CHIP(priv)) {
		ptmp = data_rtl8192cfw_start + 32;
		fw_len = (int)(data_rtl8192cfw_end - ptmp);

	} else
#endif
	{
		if( IS_UMC_A_CUT_88C(priv) ) {
			ptmp = data_rtl8192cfw_ua_start + 32;
			fw_len = (int)(data_rtl8192cfw_ua_end - ptmp);

		} else {
		ptmp = data_rtl8192cfw_n_start + 32;
		fw_len = (int)(data_rtl8192cfw_n_end - ptmp);
	}
	}

	// Disable SIC
	RTL_W8(0x41, 0x40);
	delay_ms(1);

	// Enable MCU

	RTL_W8(SYS_FUNC_EN+1, RTL_R8(SYS_FUNC_EN+1) | 0x04);
	delay_ms(1);

#ifdef CONFIG_RTL8672
	RTL_W8(0x04, RTL_R8(0x04) | 0x02);
	delay_ms(1);  //czyao
#endif

	// Load SRAM
	WriteAddr = 0x1000;
	RTL_W8(MCUFWDL, RTL_R8(MCUFWDL) | MCUFWDL_EN);
	delay_ms(1);

//	if (IS_TEST_CHIP(priv))
//		RTL_W8(0x82, RTL_R8(0x82) & 0xf7);
//	else
		RTL_W32(MCUFWDL, RTL_R32(MCUFWDL) & 0xfff0ffff);

	delay_ms(1);

	while (CurPtr < fw_len) {
		if ((CurPtr+4) > fw_len) {
			// Reach the end of file.
			while (CurPtr < fw_len) {
				Temp = *(ptmp + CurPtr);
				RTL_W8(WriteAddr, (unsigned char)Temp);
				WriteAddr++;
				CurPtr++;
			}
		} else {
			// Write FW content to memory.
			Temp = *((unsigned int *)(ptmp + CurPtr));
			Temp = cpu_to_le32(Temp);
			RTL_W32(WriteAddr, Temp);
			WriteAddr += 4;

			if((IS_TEST_CHIP(priv)==0) && (WriteAddr == 0x2000)) {
				unsigned char tmp = RTL_R8(MCUFWDL+2);
				tmp += 1;
				WriteAddr = 0x1000;
				RTL_W8(MCUFWDL+2, tmp) ;
				delay_ms(10);
//				printk("\n[CurPtr=%x, 0x82=%x]\n", CurPtr, RTL_R8(0x82));
			}
			CurPtr += 4;
		}
	}

	Temp = RTL_R8(0x80);
	Temp &= 0xfe;
	Temp |= 0x02;
	RTL_W8(0x80, (unsigned char)Temp);
	delay_ms(1);
	RTL_W8(0x81, 0x00);

	printk("<=== %s\n", __FUNCTION__);

	// check if firmware is ready
	while (!(RTL_R8(MCUFWDL) & WINTINI_RDY)) {
		if (++wait_cnt > 10) {
			return FALSE;
		}
		printk("8192c firmware not ready\n");
		delay_ms(1);
	}
#ifdef CONFIG_RTL8672
	printk("val=%x\n",RTL_R8(MCUFWDL));
#endif

	return TRUE;
}
#endif


#define	SET_RTL8192CD_RF_HALT(priv)						\
{ 														\
	unsigned char u1bTmp;								\
														\
	do													\
	{													\
		u1bTmp = RTL_R8(LDOV12D_CTRL);					\
		u1bTmp |= BIT(0); 								\
		RTL_W8(LDOV12D_CTRL, u1bTmp);					\
		RTL_W8(SPS1_CTRL, 0x0);							\
		RTL_W8(TXPAUSE, 0xFF);							\
		RTL_W16(CMDR, 0x57FC);							\
		delay_us(100);									\
		RTL_W16(CMDR, 0x77FC);							\
		RTL_W8(PHY_CCA, 0x0);							\
		delay_us(10);									\
		RTL_W16(CMDR, 0x37FC);							\
		delay_us(10);									\
		RTL_W16(CMDR, 0x77FC);							\
		delay_us(10);									\
		RTL_W16(CMDR, 0x57FC);							\
		RTL_W16(CMDR, 0x0000);							\
		u1bTmp = RTL_R8((SYS_CLKR + 1));				\
		if (u1bTmp & BIT(7))							\
		{												\
			u1bTmp &= ~(BIT(6) | BIT(7));				\
			if (!HalSetSysClk8192CD(priv, u1bTmp))		\
			break;										\
		}												\
		RTL_W8(0x03, 0x71);								\
		RTL_W8(0x09, 0x70);								\
		RTL_W8(0x29, 0x68);								\
		RTL_W8(0x28, 0x00);								\
		RTL_W8(0x20, 0x50);								\
		RTL_W8(0x26, 0x0E);								\
	} while (FALSE);									\
}

void Unlock_MCU(struct rtl8192cd_priv *priv)
{
	// 1. To clear C2H
	RTL_W8(C2H_SYNC_BYTE, 0x0);
	// 2. Unlock Overall MCU while(1)
	RTL_W8(MCU_UNLOCK, 0xFF);
}

void FirmwareSelfReset(struct rtl8192cd_priv *priv)
{
	unsigned char u1bTmp;
	unsigned int  Delay = 1000;
	if(priv->pshare->fw_version > 0x21
#ifdef CONFIG_RTL_92D_SUPPORT
		|| GET_CHIP_VER(priv) == VERSION_8192D
#endif
		)	{
		RTL_W32(FWIMR, 0x20);
		RTL_W8(REG_HMETFR+3, 0x20);
	 	u1bTmp = RTL_R8( REG_SYS_FUNC_EN+1);
	 	while(u1bTmp& BIT(2)) {
		  	Delay--;
		  	DEBUG_INFO("polling 0x03[2] Delay = %d \n", Delay);
		  	if(Delay == 0)
		   		break;
		  	delay_us(50);
			Unlock_MCU(priv);
		  	u1bTmp = RTL_R8( REG_SYS_FUNC_EN+1);
		}
		// restore MCU internal while(1) loop
		RTL_W8(MCU_UNLOCK, 0);
		if(u1bTmp& BIT(2)) {
			DEBUG_ERR("FirmwareSelfReset fail: 0x03=%02x, 0x1EB=0x%02x\n", u1bTmp, RTL_R8(0x1EB));
		}else{
			DEBUG_INFO("FirmwareSelfReset success: 0x03 = %x\n", u1bTmp);
		}
	}
}

//Return Value:
//	1: MAC I/O Registers Enable
//	0: MAC I/O Registers Disable
int check_MAC_IO_Enable(struct rtl8192cd_priv *priv)
{
	//Check PON register to decide
	return ( (RTL_R16(SYS_FUNC_EN) & (FEN_MREGEN|FEN_DCORE))==(FEN_MREGEN|FEN_DCORE) );
}
#ifdef CONFIG_RTL8672
extern unsigned char clk_src_40M;
#endif

int rtl8192cd_stop_hw(struct rtl8192cd_priv *priv)
{
#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
#ifdef TXREPORT
		RTL8188E_DisableTxReport(priv);
#endif
		RTL_W32(REG_88E_HIMR, 0);
		RTL_W32(REG_88E_HIMRE, 0);
		HalPwrSeqCmdParsing(priv, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, 
			PWR_INTF_PCI_MSK, rtl8188E_leave_lps_flow);
	} else
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8812E) {
		RTL_W32(REG_HIMR0_8812, 0);
		RTL_W32(REG_HIMR1_8812, 0);
		HalPwrSeqCmdParsing(priv, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, 
			PWR_INTF_PCI_MSK, rtl8812_enter_lps_flow);
	} else
#endif
	{
		RTL_W32(HIMR, 0);
		RTL_W16(HIMRE, 0);
		RTL_W16(HIMRE+2, 0);
		RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_NOLINK & NETYPE_Mask) << NETYPE_SHIFT));
	}

	RTL_W8(RCR, 0);
	RTL_W8(TXPAUSE, 0xff);								// Pause MAC TX queue
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
	if (
#ifdef CONFIG_RTL_92C_SUPPORT
		(GET_CHIP_VER(priv)==VERSION_8192C) || (GET_CHIP_VER(priv)==VERSION_8188C)
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
#ifdef CONFIG_RTL_92C_SUPPORT
		|| 
#endif
		(GET_CHIP_VER(priv)==VERSION_8192D)
#endif
		) {
		//	RTL_W8(CR, RTL_R8(CR) & ~(MACTXEN|MACRXEN));
			RTL_W8(CR, 0);
		}
#endif

#ifdef CONFIG_RTL_92D_DMDP
	if (priv->pshare->wlandev_idx == 0)
		RTL_W8(RSV_MAC0_CTRL, RTL_R8(RSV_MAC0_CTRL)&(~MAC0_EN));
	else
		RTL_W8(RSV_MAC1_CTRL, RTL_R8(RSV_MAC1_CTRL)&(~MAC1_EN));

	if ((RTL_R8(RSV_MAC0_CTRL)& MAC0_EN) || (RTL_R8(RSV_MAC1_CTRL)& MAC1_EN)) { // check if another interface exists
		DEBUG_INFO("Another MAC exists, cannot stop hw!!\n");
	} else
#endif
	{
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
		if (
#ifdef CONFIG_RTL_92C_SUPPORT
			(GET_CHIP_VER(priv)==VERSION_8192C) || (GET_CHIP_VER(priv)==VERSION_8188C)
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
#ifdef CONFIG_RTL_92C_SUPPORT
			|| 
#endif
			(GET_CHIP_VER(priv)==VERSION_8192D)
#endif
			) {
			//3 2.) ==== RF Off Sequence ====
			phy_InitBBRFRegisterDefinition(priv);		// preparation for read/write RF register
			
			PHY_SetRFReg(priv, RF92CD_PATH_A, 0x00, bMask20Bits, 0x00);	// disable RF
			RTL_W8(RF_CTRL, 0x00);
			RTL_W8(APSD_CTRL, 0x40);
			RTL_W8(SYS_FUNC_EN, 0xe2);		// reset BB state machine
			RTL_W8(SYS_FUNC_EN, 0xe0);		// reset BB state machine



			//3 3.) ==== Reset digital sequence ====
			if (RTL_R8(MCUFWDL) & BIT(1)) {
				//Make sure that Host Recovery Interrupt is handled by 8051 ASAP.
				RTL_W32(FSIMR, 0);				// clear FSIMR
				RTL_W32(FWIMR, 0x20);			// clear FWIMR except HRCV_INT
				RTL_W32(FTIMR, 0);				// clear FTIMR
				FirmwareSelfReset(priv);

				//Clear FWIMR to guarantee if 8051 runs in ROM, it is impossible to run FWISR Interrupt handler
				RTL_W32(FWIMR, 0x0);			// clear All FWIMR
			} else {
				//Critical Error.
				//the operation that reset 8051 is necessary to be done by 8051
				DEBUG_ERR("%s %d ERROR: (RTL_R8(MCUFWDL) & BIT(1))=0\n", __FUNCTION__, __LINE__);
				DEBUG_ERR("%s %d ERROR: the operation that reset 8051 is necessary to be done by 8051,%d\n", __FUNCTION__, __LINE__, RTL_R8(MCUFWDL));
			}
		}
#endif

		// ==== Reset digital sequence ====

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT)
		if ((GET_CHIP_VER(priv)==VERSION_8188E) || (GET_CHIP_VER(priv)==VERSION_8812E))
			RTL_W8(SYS_FUNC_EN+1, RTL_R8(SYS_FUNC_EN+1) & ~BIT(2));
		else			
#endif
			RTL_W8(SYS_FUNC_EN+1, 0x51);								// reset MCU, MAC register, DCORE
		RTL_W8(MCUFWDL, 0);											// reset MCU ready status

#ifdef CONFIG_RTL_8812_SUPPORT
	if(GET_CHIP_VER(priv)==VERSION_8812E)
	{
		unsigned char u1bTmp = 0;
		// 0x1F[7:0] = 0		// turn off RF
		RTL_W8(REG_RF_CTRL_8812, 0x00);

		// Reset MCU. Suggested by Filen. 2011.01.26. by tynli.
		u1bTmp = RTL_R8(REG_SYS_FUNC_EN_8812+1);
		RTL_W8(REG_SYS_FUNC_EN_8812+1, (u1bTmp&(~BIT(2))));

		// MCUFWDL 0x80[1:0]=0				// reset MCU ready status
		RTL_W8(REG_MCUFWDL_8812, 0x00);
	}
#endif

#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8188E) {
			HalPwrSeqCmdParsing(priv, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_PCI_MSK, rtl8188E_card_disable_flow);
		} else
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8812E) {
			HalPwrSeqCmdParsing(priv, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_PCI_MSK, rtl8812_card_disable_flow);
		} else
#endif
		{
			//3 4.) ==== Disable analog sequence ====
			RTL_W8(AFE_PLL_CTRL, 0x80);			// disable PLL

#if defined(CONFIG_RTL_92C_SUPPORT) && defined(CONFIG_RTL_92D_SUPPORT)
			if (GET_CHIP_VER(priv) == VERSION_8192C) {
				RTL_W8(SPS0_CTRL, 0x2b);
			} else
#endif
			{
#ifdef CONFIG_RTL_92C_SUPPORT
				if(IS_UMC_B_CUT_88C(priv))
					RTL_W8(SPS0_CTRL, 0x2b);
				else	
#endif
					RTL_W8(SPS0_CTRL, 0x23);
			}
		}
#ifdef CONFIG_RTL8672
		if (!clk_src_40M)
		RTL_W8(AFE_XTAL_CTRL, RTL_R8(AFE_XTAL_CTRL)&~BIT(0));		// only for ADSL platform because 40M crystal is only used by WiFi chip // disable XTAL, if No BT COEX
#endif

#ifdef CONFIG_RTL_8812_SUPPORT
		if(GET_CHIP_VER(priv)==VERSION_8812E)
		{
			unsigned char u1bTmp = 0;

			// Reset MCU IO Wrapper
			u1bTmp = RTL_R8(REG_RSV_CTRL_8812+1);
			RTL_W8(REG_RSV_CTRL_8812+1, (u1bTmp&(~BIT(0)))); 
			u1bTmp = RTL_R8(REG_RSV_CTRL_8812+1);
			RTL_W8(REG_RSV_CTRL_8812+1, u1bTmp|BIT(0));
	
			// RSV_CTRL 0x1C[7:0] = 0x0E			// lock ISO/CLK/Power control register
			RTL_W8(REG_RSV_CTRL_8812, 0x0e);
		}
#endif
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT)
		if ((GET_CHIP_VER(priv)==VERSION_8188E) || (GET_CHIP_VER(priv)==VERSION_8812E)) {
			// Reset MCU IO Wrapper
			RTL_W8(RSV_CTRL0+1, RTL_R8(RSV_CTRL0+1) & ~BIT(3));
			RTL_W8(RSV_CTRL0+1, RTL_R8(RSV_CTRL0+1) | BIT(3));
		} else
#endif
		{
			RTL_W8(APS_FSMCO+1, 0x10);
		}
		RTL_W8(RSV_CTRL0, 0x0e);				// lock ISO/CLK/Power control register

		//3 5.) ==== interface into suspend ====
//		RTL_W16(APS_FSMCO, (RTL_R16(APS_FSMCO) & 0x00ff) | (0x18 << 8));	// PCIe suspend mode

#ifdef CONFIG_RTL8672
		// 6.) Switch to XTAL_BSEL: NAND
		RTL_W8(AFE_XTAL_CTRL, RTL_R8(AFE_XTAL_CTRL) & ~ BIT(1));
#endif
	}
	return SUCCESS;
}


#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A)
void SwBWMode_AC(struct rtl8192cd_priv *priv, unsigned int bandwidth, int offset)
{
		unsigned char bTmp = 0, bTmp2 = 0;
		unsigned int dwTmp = 0, dwTmp2 = 0, tmp_rf = 0;
		unsigned char primary_channel = 0;
		unsigned int eRFPath, curMaxRFPath;

//		printk("SwBWMode_AC +++ BW = %d txsc_20 = %d txsc_40 = %d\n", bandwidth, priv->pshare->txsc_20, priv->pshare->txsc_40);
	
		curMaxRFPath = RF92CD_PATH_MAX;

		primary_channel = priv->pshare->txsc_20;

//========== <1> Set rf_mode 0x8ac & 0x668
		dwTmp = RTL_R32(0x8ac);
		dwTmp2 = RTL_R32(0x668);
		
		dwTmp &= ~(BIT(0)|BIT(1)|BIT(6)|BIT(7)|BIT(8)|BIT(9)|BIT(20)|BIT(21));
		dwTmp2 &= ~(BIT(7)|BIT(8));
		
		switch (bandwidth)
		{
		case HT_CHANNEL_WIDTH_AC_5:
			dwTmp |= (BIT(6)|BIT(20));
			RTL_W32(0x8ac, dwTmp);
			
			RTL_W32(0x668, dwTmp2); 
			break;
		case HT_CHANNEL_WIDTH_AC_10:
			dwTmp |= (BIT(7)|BIT(8)|BIT(21));
			RTL_W32(0x8ac, dwTmp);

			RTL_W32(0x668, dwTmp2);
			break;
		case HT_CHANNEL_WIDTH_AC_20:
			dwTmp |= (BIT(9)|BIT(20)|BIT(21));
			RTL_W32(0x8ac, dwTmp);

			RTL_W32(0x668, dwTmp2);
			break;
		case HT_CHANNEL_WIDTH_AC_40:
			dwTmp |= (BIT(0)|BIT(9)|BIT(20)|BIT(21));
			RTL_W32(0x8ac, dwTmp);

			dwTmp2 |= BIT(7);
			RTL_W32(0x668, dwTmp2);
			break;
		case HT_CHANNEL_WIDTH_AC_80:
			dwTmp |= (BIT(1)|BIT(9)|BIT(20)|BIT(21));
			RTL_W32(0x8ac, dwTmp);

			dwTmp2 |= BIT(8);
			RTL_W32(0x668, dwTmp2);
			break;
		}	

//========== <2> Set primary channel 0x8ac & 0xa00, txsc 0x483

		dwTmp = RTL_R32(0x8ac);
		dwTmp &= ~(BIT(2)|BIT(3)|BIT(4)|BIT(5));
		
		bTmp = RTL_R8(0xa00);
		bTmp2= RTL_R8(0x483);

		switch (bandwidth)
		{
		case HT_CHANNEL_WIDTH_AC_5:
		case HT_CHANNEL_WIDTH_AC_10:
		case HT_CHANNEL_WIDTH_AC_20:
			break;
		case HT_CHANNEL_WIDTH_AC_40:
			dwTmp |= (primary_channel << 2);
			RTL_W32(0x8ac, dwTmp);
			RTL_W32(0x838, (RTL_R32(0x838)&0x0fffffff)|(primary_channel<<28));

			if(primary_channel == 1)
				bTmp |= BIT(4);
			else
				bTmp &= ~(BIT(4));
			
			RTL_W8(0xa00, bTmp);

			bTmp2 &= 0xf0;
			bTmp2 |= priv->pshare->txsc_20;
			RTL_W8(0x483, bTmp2);
			
			break;
		case HT_CHANNEL_WIDTH_AC_80:
			dwTmp |= (primary_channel << 2);
			RTL_W32(0x8ac, dwTmp);
			RTL_W32(0x838, (RTL_R32(0x838)&0x0fffffff)|(primary_channel<<28));

			bTmp2 = ( priv->pshare->txsc_20 | (priv->pshare->txsc_40 << 4));
			RTL_W8(0x483, bTmp2);

			break;
		}	

//========== <3> Set RRSR_RSC 0x440

		dwTmp = RTL_R32(0x440);
		dwTmp &= ~(BIT(21)|BIT(22));

		switch (bandwidth)
		{
		case HT_CHANNEL_WIDTH_AC_5:
		case HT_CHANNEL_WIDTH_AC_10:
		case HT_CHANNEL_WIDTH_AC_20:
			break;
		case HT_CHANNEL_WIDTH_AC_40:
			RTL_W32(0x440, dwTmp);			
			break;
		case HT_CHANNEL_WIDTH_AC_80:

			//dwTmp |= (0x3 << 21);
			//dwTmp |= (0x2 << 21);		// duplicate ?
			RTL_W32(0x440, dwTmp);
			break;
		}	

//========== <4> Set RF TRX_BW rf_0x18

		switch (bandwidth)
		{
		case HT_CHANNEL_WIDTH_AC_5:
		case HT_CHANNEL_WIDTH_AC_10:
		case HT_CHANNEL_WIDTH_AC_20:
			tmp_rf = 0x03;
			break;
		case HT_CHANNEL_WIDTH_AC_40:
			tmp_rf = 0x01;
			break;
		case HT_CHANNEL_WIDTH_AC_80:
			tmp_rf = 0x00;
			break;
		}

		for(eRFPath = RF92CD_PATH_A; eRFPath < curMaxRFPath; eRFPath++) {
			unsigned int orig_val = PHY_QueryRFReg(priv, eRFPath, rRfChannel, bMask20Bits, 1);
			orig_val &= ~(BIT(10)|BIT(11));
			orig_val |= (tmp_rf << 10); 
			PHY_SetRFReg(priv, eRFPath, rRfChannel, bMask20Bits, orig_val);
		}

}
#endif

#if	defined(CONFIG_WLAN_HAL_8192EE)
void SwBWMode_92e(struct rtl8192cd_priv *priv, unsigned int bandwidth, int offset)
{
	unsigned char regBwOpMode, regRRSR_RSC, nCur40MhzPrimeSC;
	unsigned int eRFPath, curMaxRFPath, val;

	DEBUG_INFO("SwBWMode(): Switch to %s bandwidth\n", bandwidth?"40MHz":"20MHz");

	curMaxRFPath = RF92CD_PATH_MAX;

	if (offset == 1)
		nCur40MhzPrimeSC = 2;
	else
		nCur40MhzPrimeSC = 1;

	//3 <1> Set MAC register
//	regBwOpMode = RTL_R8(BWOPMODE);
	regRRSR_RSC = RTL_R8(RRSR+2);

	switch (bandwidth)
	{
	case HT_CHANNEL_WIDTH_20:
		regBwOpMode |= BW_OPMODE_20MHZ;
//		RTL_W8(BWOPMODE, regBwOpMode);
		break;
	case HT_CHANNEL_WIDTH_20_40:
//		regBwOpMode &= ~BW_OPMODE_20MHZ;
//		RTL_W8(BWOPMODE, regBwOpMode);
		regRRSR_RSC = (regRRSR_RSC&0x90) | (nCur40MhzPrimeSC<<5);
		RTL_W8(RRSR+2, regRRSR_RSC);

		// Let 812cd_rx, re-assign value
		if (priv->pshare->is_40m_bw){
			priv->pshare->Reg_RRSR_2 = 0;
			priv->pshare->Reg_81b = 0;
		}
		break;
	default:
		DEBUG_ERR("SwBWMode(): bandwidth mode error!\n");
		return;
		break;
	}

	//3 <2> Set PHY related register
	switch (bandwidth)
	{
	case HT_CHANNEL_WIDTH_20:
		PHY_SetBBReg(priv, rFPGA0_RFMOD, bRFMOD, 0x0);
		PHY_SetBBReg(priv, rFPGA1_RFMOD, bRFMOD, 0x0);
		val = 3;
		break;
	case HT_CHANNEL_WIDTH_20_40:
		PHY_SetBBReg(priv, rFPGA0_RFMOD, bRFMOD, 0x1);
		PHY_SetBBReg(priv, rFPGA1_RFMOD, bRFMOD, 0x1);
		// Set Control channel to upper or lower. These settings are required only for 40MHz
		PHY_SetBBReg(priv, rCCK0_System, bCCKSideBand, (nCur40MhzPrimeSC>>1));
		PHY_SetBBReg(priv, rOFDM1_LSTF, 0xC00, nCur40MhzPrimeSC);
		val = 1;

		PHY_SetBBReg(priv, 0x818, (BIT(26)|BIT(27)), (nCur40MhzPrimeSC==2)?1:2);
		break;
	default:
		DEBUG_ERR("SwBWMode(): bandwidth mode error! %d\n", __LINE__);
		return;
		break;
	}

	

	for(eRFPath = RF92CD_PATH_A; eRFPath < curMaxRFPath; eRFPath++) 
		PHY_SetRFReg(priv, eRFPath, rRfChannel, (BIT(11)|BIT(10)), val);


}
#endif

void SwBWMode(struct rtl8192cd_priv *priv, unsigned int bandwidth, int offset)
{
	unsigned char regBwOpMode, regRRSR_RSC, nCur40MhzPrimeSC;
	unsigned int eRFPath, curMaxRFPath, val;

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A)
	if ((GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8881A)){
		priv->pshare->No_RF_Write = 0;
		get_txsc_8812(priv, priv->pmib->dot11RFEntry.dot11channel);		
		SwBWMode_AC(priv, bandwidth, offset); 
		priv->pshare->No_RF_Write = 1;
		return;
	}
#endif
#if	defined(CONFIG_WLAN_HAL_8192EE)
	if(GET_CHIP_VER(priv) == VERSION_8192E) {
		SwBWMode_92e(priv, bandwidth, offset);
		return;
	}
#endif

	DEBUG_INFO("SwBWMode(): Switch to %s bandwidth\n", bandwidth?"40MHz":"20MHz");

#ifdef CONFIG_RTL_92D_DMDP
	if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY)
		curMaxRFPath = RF92CD_PATH_B;
	else
#endif
		curMaxRFPath = RF92CD_PATH_MAX;

	if (offset == 1)
		nCur40MhzPrimeSC = 2;
	else
		nCur40MhzPrimeSC = 1;

	//3 <1> Set MAC register
	regBwOpMode = RTL_R8(BWOPMODE);
	regRRSR_RSC = RTL_R8(RRSR+2);

	switch (bandwidth)
	{
	case HT_CHANNEL_WIDTH_20:
		regBwOpMode |= BW_OPMODE_20MHZ;
		RTL_W8(BWOPMODE, regBwOpMode);
		break;
	case HT_CHANNEL_WIDTH_20_40:
		regBwOpMode &= ~BW_OPMODE_20MHZ;
		RTL_W8(BWOPMODE, regBwOpMode);
		regRRSR_RSC = (regRRSR_RSC&0x90) | (nCur40MhzPrimeSC<<5);
		RTL_W8(RRSR+2, regRRSR_RSC);

		// Let 812cd_rx, re-assign value
		if (priv->pshare->is_40m_bw){
			priv->pshare->Reg_RRSR_2 = 0;
			priv->pshare->Reg_81b = 0;
		}
		break;
	default:
		DEBUG_ERR("SwBWMode(): bandwidth mode error!\n");
		return;
		break;
	}

	//3 <2> Set PHY related register
	switch (bandwidth)
	{
	case HT_CHANNEL_WIDTH_20:
		PHY_SetBBReg(priv, rFPGA0_RFMOD, bRFMOD, 0x0);
		PHY_SetBBReg(priv, rFPGA1_RFMOD, bRFMOD, 0x0);
#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8192D) {
			PHY_SetBBReg(priv, rFPGA0_AnalogParameter2, BIT(11) | BIT(10), 3);// SET BIT10 BIT11  for receive cck
		}
#endif
#ifdef CONFIG_RTL_92C_SUPPORT
		if ((GET_CHIP_VER(priv)==VERSION_8192C) || (GET_CHIP_VER(priv)==VERSION_8188C)) {
			PHY_SetBBReg(priv, rFPGA0_AnalogParameter2, BIT(10), 1);
		}
#endif
		break;
	case HT_CHANNEL_WIDTH_20_40:
		PHY_SetBBReg(priv, rFPGA0_RFMOD, bRFMOD, 0x1);
		PHY_SetBBReg(priv, rFPGA1_RFMOD, bRFMOD, 0x1);
		// Set Control channel to upper or lower. These settings are required only for 40MHz
		PHY_SetBBReg(priv, rCCK0_System, bCCKSideBand, (nCur40MhzPrimeSC>>1));
		PHY_SetBBReg(priv, rOFDM1_LSTF, 0xC00, nCur40MhzPrimeSC);
#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8192D) {
			PHY_SetBBReg(priv, rFPGA0_AnalogParameter2, BIT(11) | BIT(10), 0);// SET BIT10 BIT11  for receive cck
		}
#endif
#ifdef CONFIG_RTL_92C_SUPPORT
		if ((GET_CHIP_VER(priv)==VERSION_8192C) || (GET_CHIP_VER(priv)==VERSION_8188C)) {
			PHY_SetBBReg(priv, rFPGA0_AnalogParameter2, BIT(10), 0);
		}
#endif
		PHY_SetBBReg(priv, 0x818, (BIT(26)|BIT(27)), (nCur40MhzPrimeSC==2)?1:2);
		break;
	default:
		DEBUG_ERR("SwBWMode(): bandwidth mode error! %d\n", __LINE__);
		return;
		break;
	}

	//3<3> Set RF related register
	switch (bandwidth)
	{
	case HT_CHANNEL_WIDTH_20:
		{
#ifdef CONFIG_RTL_88E_SUPPORT		
		if (GET_CHIP_VER(priv) == VERSION_8188E)		
		val = 3;
		else
#endif	
		val = 1;
		}
		break;
	case HT_CHANNEL_WIDTH_20_40:
		val = 0;
		break;
	default:
		DEBUG_ERR("SwBWMode(): bandwidth mode error! %d\n", __LINE__);
		return;
		break;
	}

	for(eRFPath = RF92CD_PATH_A; eRFPath < curMaxRFPath; eRFPath++)	{
#ifdef CONFIG_RTL_92C_SUPPORT
		if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)){
			PHY_SetRFReg(priv, eRFPath, rRfChannel, (BIT(11)|BIT(10)), val);
		}
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8192D){
			priv->pshare->RegRF18[eRFPath] = RTL_SET_MASK(priv->pshare->RegRF18[eRFPath],(BIT(11)|BIT(10)),val,10);
			PHY_SetRFReg(priv, eRFPath, rRfChannel, bMask20Bits, priv->pshare->RegRF18[eRFPath]);
			//PHY_SetRFReg(priv, eRFPath, rRfChannel, (BIT(11)|BIT(10)), val);
		}
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8188E) {
			PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)eRFPath, rRfChannel, (BIT(11)|BIT(10)), val);
		}
#endif
	}

#if 0
	if (priv->pshare->rf_ft_var.use_frq_2_3G)
		PHY_SetRFReg(priv, RF90_PATH_C, 0x2c, 0x60, 0);
#endif
#ifdef TX_EARLY_MODE
#ifdef CONFIG_RTL_88E_SUPPORT	
	if (GET_CHIP_VER(priv) == VERSION_8188E) {
		if ((bandwidth == HT_CHANNEL_WIDTH_20) && GET_TX_EARLY_MODE) {
			GET_TX_EARLY_MODE = 0;
			//printk("[%s:%d] 88E 20M mpde ===> turn off early mode!\n", __FUNCTION__, __LINE__);
		}
	}
#endif		
#endif
}

#ifdef SMART_CONCURRENT_92D
void setup_timer1(struct rtl8192cd_priv *priv, int timeout)
{
	unsigned int current_value=RTL_R32(TSFTR);

	if (TSF_LESS(timeout, current_value))
		timeout = current_value+20;

	RTL_W32(TIMER0, timeout);
	RTL_W32(HIMR, RTL_R32(HIMR) | HIMR_TIMEOUT1);
}


void cancel_timer1(struct rtl8192cd_priv *priv)
{
	RTL_W32(HIMR, RTL_R32(HIMR) & ~HIMR_TIMEOUT1);
}
#endif
#if defined(SMART_CONCURRENT_92D) || defined(SUPPORT_TX_AMSDU)
void setup_timer2(struct rtl8192cd_priv *priv, unsigned int timeout)
{
	unsigned int current_value=RTL_R32(TSFTR);

	if (TSF_LESS(timeout, current_value))
		timeout = current_value+20;

	RTL_W32(TIMER1, timeout);

#ifdef CONFIG_WLAN_HAL
    if( IS_HAL_CHIP(priv) ) {
	    GET_HAL_INTERFACE(priv)->AddInterruptMaskHandler(priv, HAL_INT_TYPE_PSTIMEOUT2);
    }
    else
#endif //CONFIG_WLAN_HAL
#ifdef CONFIG_RTL_8812_SUPPORT
	if(GET_CHIP_VER(priv)== VERSION_8812E)
	    RTL_W32(REG_HIMR0_8812,	RTL_R32(REG_HIMR0_8812) | IMR_TIMER2_8812);
	else
#endif
	    RTL_W32(HIMR, RTL_R32(HIMR) | HIMR_TIMEOUT2);

}


void cancel_timer2(struct rtl8192cd_priv *priv)
{

#ifdef CONFIG_WLAN_HAL
    if( IS_HAL_CHIP(priv) ) {
	    GET_HAL_INTERFACE(priv)->RemoveInterruptMaskHandler(priv, HAL_INT_TYPE_PSTIMEOUT2);
    }
    else
#endif //CONFIG_WLAN_HAL    
#ifdef CONFIG_RTL_8812_SUPPORT
	if(GET_CHIP_VER(priv)== VERSION_8812E)
	RTL_W32(REG_HIMR0_8812, RTL_R32(REG_HIMR0_8812) & ~IMR_TIMER2_8812);
	else
#endif
	RTL_W32(HIMR, RTL_R32(HIMR) & ~HIMR_TIMEOUT2);

}
#endif



#if 0
void tx_path_by_rssi(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char enable){

	if((get_rf_mimo_mode(priv) != MIMO_2T2R))
		return; // 1T2R, 1T1R; do nothing

	if(pstat == NULL)
		return;

#ifdef	STA_EXT
	if ((pstat->remapped_aid == FW_NUM_STAT-1) ||
		(priv->pshare->has_2r_sta & BIT(pstat->remapped_aid)))// 2r STA
#else
	if (priv->pshare->has_2r_sta & BIT(pstat->aid))// 2r STA
#endif
		return; // do nothing

	// for debug, by victoryman 20090623
	if (pstat->tx_ra_bitmap & 0xff00000) {
		// this should be a 2r station!!!
		return;
	}

	if (pstat->tx_ra_bitmap & 0xffff000){// 11n 1R client
		if(enable){
			if(pstat->rf_info.mimorssi[0] > pstat->rf_info.mimorssi[1])
				Switch_1SS_Antenna(priv, 1);
			else
				Switch_1SS_Antenna(priv, 2);
		}
		else
			Switch_1SS_Antenna(priv, 3);
  }
	else if (pstat->tx_ra_bitmap & 0xff0){// 11bg client
		if(enable){
			if(pstat->rf_info.mimorssi[0] > pstat->rf_info.mimorssi[1])
				Switch_OFDM_Antenna(priv, 1);
			else
				Switch_OFDM_Antenna(priv, 2);
		}
		else
			Switch_OFDM_Antenna(priv, 3);
  }

#if 0  // original  setup
	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N){ // for 11n 1ss sta
		if(enable){
			if(pstat->rf_info.mimorssi[0] > pstat->rf_info.mimorssi[1])
				Switch_1SS_Antenna(priv, 1);
			else
				Switch_1SS_Antenna(priv, 2);
		}
		else
			Switch_1SS_Antenna(priv, 3);
	}
	else if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G){ // for 11g
		if(enable){
			if(pstat->rf_info.mimorssi[0] > pstat->rf_info.mimorssi[1])
				Switch_OFDM_Antenna(priv, 1);
			else
				Switch_OFDM_Antenna(priv, 2);
		}
		else
			Switch_OFDM_Antenna(priv, 3);
	}
#endif


}
//#endif


// dynamic Rx path selection by signal strength
void rx_path_by_rssi(struct rtl8192cd_priv *priv, struct stat_info *pstat, int enable)
{
	unsigned char highest_rssi=0, higher_rssi=0, under_ss_th_low=0;
	RF92CD_RADIO_PATH_E eRFPath, eRFPath_highest=0, eRFPath_higher=0;
	int ant_on_processing=0;
#ifdef _DEBUG_RTL8192CD_
	char path_name[] = {'A', 'B'};
#endif

	if (enable == FALSE) {
		if (priv->pshare->phw->ant_off_num) {
			priv->pshare->phw->ant_off_num = 0;
			priv->pshare->phw->ant_off_bitmap = 0;
			RTL_W8(rOFDM0_TRxPathEnable, 0x0f);
			RTL_W8(rOFDM1_TRxPathEnable, 0x0f);
			DEBUG_INFO("More than 1 sta, turn on all path\n");
		}
		return;
	}

	for (eRFPath = RF92CD_PATH_A; eRFPath<priv->pshare->phw->NumTotalRFPath; eRFPath++) {
		if (priv->pshare->phw->ant_off_bitmap & BIT(eRFPath))
			continue;

		if (pstat->rf_info.mimorssi[eRFPath] > highest_rssi) {
			higher_rssi = highest_rssi;
			eRFPath_higher = eRFPath_highest;
			highest_rssi = pstat->rf_info.mimorssi[eRFPath];
			eRFPath_highest = eRFPath;
		}

		else if (pstat->rf_info.mimorssi[eRFPath] > higher_rssi) {
			higher_rssi = pstat->rf_info.mimorssi[eRFPath];
			eRFPath_higher = eRFPath;
		}

		if (pstat->rf_info.mimorssi[eRFPath] < priv->pshare->rf_ft_var.ss_th_low)
			under_ss_th_low = 1;
	}

	// for OFDM
	if (priv->pshare->phw->ant_off_num > 0)
	{
		for (eRFPath = RF92CD_PATH_A; eRFPath<priv->pshare->phw->NumTotalRFPath; eRFPath++) {
			if (!(priv->pshare->phw->ant_off_bitmap & BIT(eRFPath)))
				continue;

			if (highest_rssi >= priv->pshare->phw->ant_on_criteria[eRFPath]) {
				priv->pshare->phw->ant_off_num--;
				priv->pshare->phw->ant_off_bitmap &= (~BIT(eRFPath));
				RTL_W8(rOFDM0_TRxPathEnable, ~(priv->pshare->phw->ant_off_bitmap) & 0x0f);
				RTL_W8(rOFDM1_TRxPathEnable, ~(priv->pshare->phw->ant_off_bitmap) & 0x0f);
				DEBUG_INFO("Path %c is on due to >= %d%%\n",
					path_name[eRFPath], priv->pshare->phw->ant_on_criteria[eRFPath]);
				ant_on_processing = 1;
			}
		}
	}

	if (!ant_on_processing)
	{
		if (priv->pshare->phw->ant_off_num < 2) {
			for (eRFPath = RF92CD_PATH_A; eRFPath<priv->pshare->phw->NumTotalRFPath; eRFPath++) {
				if ((eRFPath == eRFPath_highest) || (priv->pshare->phw->ant_off_bitmap & BIT(eRFPath)))
					continue;

				if ((pstat->rf_info.mimorssi[eRFPath] < priv->pshare->rf_ft_var.ss_th_low) &&
					((highest_rssi - pstat->rf_info.mimorssi[eRFPath]) > priv->pshare->rf_ft_var.diff_th)) {
					priv->pshare->phw->ant_off_num++;
					priv->pshare->phw->ant_off_bitmap |= BIT(eRFPath);
					priv->pshare->phw->ant_on_criteria[eRFPath] = highest_rssi + 5;
					RTL_W8(rOFDM0_TRxPathEnable, ~(priv->pshare->phw->ant_off_bitmap) & 0x0f);
					RTL_W8(rOFDM1_TRxPathEnable, ~(priv->pshare->phw->ant_off_bitmap) & 0x0f);
					DEBUG_INFO("Path %c is off due to under th_low %d%% and diff %d%%, will be on at %d%%\n",
						path_name[eRFPath], priv->pshare->rf_ft_var.ss_th_low,
						(highest_rssi - pstat->rf_info.mimorssi[eRFPath]),
						priv->pshare->phw->ant_on_criteria[eRFPath]);
					break;
				}
			}
		}
	}

	// For CCK
	if (priv->pshare->rf_ft_var.cck_sel_ver == 1) {
		if (under_ss_th_low && (pstat->rx_pkts > 20)) {
			if (priv->pshare->phw->ant_cck_sel != ((eRFPath_highest << 2) | eRFPath_higher)) {
				priv->pshare->phw->ant_cck_sel = ((eRFPath_highest << 2) | eRFPath_higher);
				RTL_W8(0xa07, (RTL_R8(0xa07) & 0xf0) | priv->pshare->phw->ant_cck_sel);
				DEBUG_INFO("CCK select default: path %c, optional: path %c\n",
					path_name[eRFPath_highest], path_name[eRFPath_higher]);
			}
		}
	}
}


// dynamic Rx path selection by signal strength
void rx_path_by_rssi_cck_v2(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int highest_rssi=-1000, higher_rssi=-1000;
	RF92CD_RADIO_PATH_E eRFPath, eRFPath_highest=0, eRFPath_higher=0;
#ifdef _DEBUG_RTL8192CD_
	char path_name[] = {'A', 'B'};
#endif

	for (eRFPath = RF92CD_PATH_A; eRFPath<priv->pshare->phw->NumTotalRFPath; eRFPath++) {
		if (pstat->cck_mimorssi_total[eRFPath] > highest_rssi) {
			higher_rssi = highest_rssi;
			eRFPath_higher = eRFPath_highest;
			highest_rssi = pstat->cck_mimorssi_total[eRFPath];
			eRFPath_highest = eRFPath;
		}

		else if (pstat->cck_mimorssi_total[eRFPath] > higher_rssi) {
			higher_rssi = pstat->cck_mimorssi_total[eRFPath];
			eRFPath_higher = eRFPath;
		}
	}

	if (priv->pshare->phw->ant_cck_sel != ((eRFPath_highest << 2) | eRFPath_higher)) {
		priv->pshare->phw->ant_cck_sel = ((eRFPath_highest << 2) | eRFPath_higher);
		RTL_W8(0xa07, (RTL_R8(0xa07) & 0xf0) | priv->pshare->phw->ant_cck_sel);
		DEBUG_INFO("CCK rssi A:%d B:%d C:%d D:%d accu %d pkts\n", pstat->cck_mimorssi_total[0],
			pstat->cck_mimorssi_total[1], pstat->cck_mimorssi_total[2], pstat->cck_mimorssi_total[3], pstat->cck_rssi_num);
		DEBUG_INFO("CCK select default: path %c, optional: path %c\n",
			path_name[eRFPath_highest], path_name[eRFPath_higher]);
	}
}


// Tx power control
void tx_power_control(struct rtl8192cd_priv *priv, struct stat_info *pstat, int enable)
{
	if (enable) {
		if (!priv->pshare->phw->lower_tx_power) {
			// TX High power enable
//			set_fw_reg(priv, 0xfd000009, 0, 0);
			if (!priv->pshare->bcnTxAGC)
				RTL_W8(0x364, RTL_R8(0x364) | FW_REG364_HP);
			priv->pshare->phw->lower_tx_power++;

		if ((!priv->pshare->is_40m_bw || (pstat->tx_bw == HT_CHANNEL_WIDTH_20)) &&
			(!pstat->is_rtl8190_sta && !pstat->is_broadcom_sta && !pstat->is_marvell_sta && !pstat->is_intel_sta))
			set_fw_reg(priv, 0xfd004314, 0, 0);
		else
			set_fw_reg(priv, 0xfd000015, 0, 0);
		}
	}
	else {
		if (priv->pshare->phw->lower_tx_power) {
			//TX High power disable
//			set_fw_reg(priv, 0xfd000008, 0, 0);
			RTL_W8(0x364, RTL_R8(0x364) & ~FW_REG364_HP);
			priv->pshare->phw->lower_tx_power = 0;
		}
	}
}


void tx_power_tracking(struct rtl8192cd_priv *priv)
{
	if (priv->pmib->dot11RFEntry.ther) {
		DEBUG_INFO("TPT: triggered(every %d seconds)\n", priv->pshare->rf_ft_var.tpt_period);

		// enable rf reg 0x24 power and trigger, to get ther value in 1 second
		PHY_SetRFReg(priv, RF92CD_PATH_A, 0x24, bMask20Bits, 0x60);
		mod_timer(&priv->pshare->phw->tpt_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(1000)); // 1000ms
	}
}


void rtl8192cd_tpt_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	unsigned int val32;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (timer_pending(&priv->pshare->phw->tpt_timer))
		del_timer_sync(&priv->pshare->phw->tpt_timer);

	if (priv->pmib->dot11RFEntry.ther) {
		// query rf reg 0x24[4:0], for thermal meter value
		val32 = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x24, bMask20Bits, 1) & 0x01f;

		if (val32) {
			set_fw_reg(priv, 0xfd000019|(priv->pmib->dot11RFEntry.ther & 0xff)<<8|val32<<16, 0, 0);
			DEBUG_INFO("TPT: finished once (ther: current=0x%02x, target=0x%02x)\n",
				val32, priv->pmib->dot11RFEntry.ther);
		}
		else{
			DEBUG_WARN("TPT: cannot finish, since wrong current ther value report\n");
		}
	}
}
#endif


/*
 *
 * CAM related functions
 *
 */

/*******************************************************/
/*CAM related utility                                  */
/*CamAddOneEntry                                       */
/*CamDeleteOneEntry                                    */
/*CamResetAllEntry                                     */
/*******************************************************/
#define TOTAL_CAM_ENTRY 32

#define CAM_CONTENT_COUNT 8
#define CAM_CONTENT_USABLE_COUNT 6

#define CFG_VALID        BIT(15)


static UCHAR CAM_find_usable(struct rtl8192cd_priv *priv)
{
	unsigned long command = 0, content = 0;
	unsigned char index;
	int for_begin = 4;
#ifdef CONFIG_WLAN_HAL
        u4Byte retVal;
#endif // CONFIG_WLAN_HAL

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8188E) {
#ifdef CONFIG_RTL_HW_WAPI_SUPPORT
		for_begin = 3;
#else
		for_begin = 2;
#endif
#ifdef MBSSID
		if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
			for_begin = 0;
#endif
#ifdef UNIVERSAL_REPEATER
		if (IS_VXD_INTERFACE(priv)) {
			for_begin = 0;
		} else {
			if (IS_ROOT_INTERFACE(priv)) {
				if (IS_DRV_OPEN(GET_VXD_PRIV(priv)))
					for_begin = 0;
			}
		}
#endif
	}
#endif

#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
	    retVal = GET_HAL_INTERFACE(priv)->CAMFindUsableHandler(priv, for_begin);
            return (unsigned char)retVal;
	} else
#endif
	{
		for (index=for_begin; index<TOTAL_CAM_ENTRY; index++) {
			// polling bit, and No Write enable, and address
			command = CAM_CONTENT_COUNT*index;
			RTL_W32(CAMCMD, (SECCAM_POLL|command));

		   	// Check polling bit is clear
			while (1) {
				command = RTL_R32(CAMCMD);
				if(command & SECCAM_POLL)
					continue;
				else
					break;
			}
			content = RTL_R32(CAMREAD);

			// check valid bit. if not valid,
			if ((content & CFG_VALID) == 0) {
				return index;
			}
		}
	}
	return TOTAL_CAM_ENTRY;
}


static void CAM_program_entry(struct rtl8192cd_priv *priv, unsigned char index, unsigned char* macad,
	unsigned char* key128, unsigned short config)
{
	unsigned long target_command = 0, target_content = 0;
	unsigned char entry_i = 0;
	struct stat_info *pstat;

#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
	    GET_HAL_INTERFACE(priv)->CAMProgramEntryHandler(
	                                priv, 
	                                index,
	                                macad,
	                                key128,
	                                config
	                                );
	} else
#endif
	{
		for (entry_i=0; entry_i<CAM_CONTENT_USABLE_COUNT; entry_i++)
		{
			// polling bit, and write enable, and address
			target_command = entry_i + CAM_CONTENT_COUNT*index;
			target_command = target_command |SECCAM_POLL | SECCAM_WE;
			if (entry_i == 0) {
			    //first 32-bit is MAC address and CFG field
			    target_content = (ULONG)(*(macad+0))<<16
								|(ULONG)(*(macad+1))<<24
								|(ULONG)config;
			    target_content = target_content|config;
		    }
			else if (entry_i == 1) {
				//second 32-bit is MAC address
				target_content = (ULONG)(*(macad+5))<<24
								|(ULONG)(*(macad+4))<<16
								|(ULONG)(*(macad+3))<<8
								|(ULONG)(*(macad+2));
			}
			else {
				target_content = (ULONG)(*(key128+(entry_i*4-8)+3))<<24
								|(ULONG)(*(key128+(entry_i*4-8)+2))<<16
								|(ULONG)(*(key128+(entry_i*4-8)+1))<<8
								|(ULONG)(*(key128+(entry_i*4-8)+0));
			}

			RTL_W32(CAMWRITE, target_content);
			RTL_W32(CAMCMD, target_command);
		}

		target_content = RTL_R32(CR);
		if ((target_content & MAC_SEC_EN) == 0)
			RTL_W32(CR, (target_content | MAC_SEC_EN));
	}
	
	pstat = get_stainfo(priv, macad);
	if (pstat) {
		pstat->cam_id = index;
	}
// move above
#if 0
	target_content = RTL_R32(CR);
	if ((target_content & MAC_SEC_EN) == 0)
		RTL_W32(CR, (target_content | MAC_SEC_EN));
#endif		
}


int CamAddOneEntry(struct rtl8192cd_priv *priv,unsigned char *pucMacAddr, unsigned long keyId,
	unsigned long encAlg, unsigned long useDK, unsigned char *pKey)
{
	unsigned char retVal = 0, camIndex = 0, wpaContent = 0;
	unsigned short usConfig = 0;
	unsigned int set_dk_margin = 4;

    //use Hardware Polling to check the valid bit.
    //in reality it should be done by software link-list
	if ((!memcmp(pucMacAddr, "\xff\xff\xff\xff\xff\xff", 6)) || (useDK
#if defined(CONFIG_RTL_HW_WAPI_SUPPORT)
	&&((encAlg>>2)!=DOT11_ENC_WAPI)
#endif
		))
		camIndex = keyId;
	else
		camIndex = CAM_find_usable(priv);

    if (camIndex==TOTAL_CAM_ENTRY)
    	return retVal;

	usConfig = usConfig|CFG_VALID|((USHORT)(encAlg))|(UCHAR)keyId;

	#if defined(CONFIG_RTL_HW_WAPI_SUPPORT)
	if((encAlg>>2) == DOT11_ENC_WAPI)
	{
		//ulUseDK is used to diff Parwise and Group
		if(camIndex<4) //is group key
			usConfig |= BIT(6);

		if(useDK==1)  // ==0 sec key; == 1mic key
			usConfig |= BIT(5);

		useDK = 0;
	}
	#endif

    CAM_program_entry(priv, camIndex, pucMacAddr, pKey, usConfig);

	if (priv->pshare->CamEntryOccupied == 0) {
#ifdef  CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {
	        SECURITY_CONFIG_OPERATION  SCO;
        
	        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_SECURITY_CONFIG, (u16 *)&SCO);
			if (useDK == 1) {
				SCO |= SCO_RXUSEDK | SCO_TXUSEDK;
			}
	        SCO |= SCO_RXDEC | SCO_TXENC | SCO_NOSKMC | SCO_CHK_KEYID;
	        GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_SECURITY_CONFIG, (u16 *)&SCO);
		} else 
#endif
		{
			if (useDK == 1)
				wpaContent = RXUSEDK | TXUSEDK;
			RTL_W16(SECCFG, RTL_R16(SECCFG) | RXDEC | TXENC | wpaContent | NOSKMC | CHK_KEYID);
		}
	}

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8188E) {
#ifdef CONFIG_RTL_HW_WAPI_SUPPORT
		set_dk_margin = 3;
#else
		set_dk_margin = 2;
#endif
#ifdef MBSSID
		if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
			set_dk_margin = 0;
#endif
#ifdef UNIVERSAL_REPEATER
		if (IS_VXD_INTERFACE(priv)) {
			set_dk_margin = 0;
		} else {
			if (IS_ROOT_INTERFACE(priv)) {
				if (IS_DRV_OPEN(GET_VXD_PRIV(priv)))
					set_dk_margin = 0;
			}
		}
#endif
	}
#endif

	if (camIndex < set_dk_margin) {
#ifdef  CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {
	        SECURITY_CONFIG_OPERATION  SCO;
	        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_SECURITY_CONFIG, (u16 *)&SCO);
	        SCO = (SCO & ~SCO_NOSKMC) | (SCO_RXBCUSEDK | SCO_TXBCUSEDK);
	        GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_SECURITY_CONFIG, (u16 *)&SCO);
		} else
#endif
		{
			RTL_W16(SECCFG, (RTL_R16(SECCFG) & ~NOSKMC) | (RXBCUSEDK | TXBCUSEDK));
		}
	}

    return 1;
}


void CAM_read_mac_config(struct rtl8192cd_priv *priv, unsigned char index, unsigned char* pMacad,
	unsigned short* pTempConfig)
{
	unsigned long command = 0, content = 0;

	// polling bit, and No Write enable, and address
	// cam address...
	// first 32-bit
	command = CAM_CONTENT_COUNT*index+0;
	command = command | SECCAM_POLL;
	RTL_W32(CAMCMD, command);

   	//Check polling bit is clear
	while (1) {
		command = RTL_R32(CAMCMD);
		if(command & SECCAM_POLL)
			continue;
		else
			break;
	}
	content = RTL_R32(CAMREAD);

	//first 32-bit is MAC address and CFG field
	*(pMacad+0) = (UCHAR)((content>>16)&0x000000FF);
	*(pMacad+1) = (UCHAR)((content>>24)&0x000000FF);
	*pTempConfig  = (USHORT)(content&0x0000FFFF);

	command = CAM_CONTENT_COUNT*index+1;
	command = command | SECCAM_POLL;
	RTL_W32(CAMCMD, command);

   	//Check polling bit is clear
	while (1) {
		command = RTL_R32(CAMCMD);
		if(command & SECCAM_POLL)
			continue;
		else
			break;
	}
	content = RTL_R32(CAMREAD);

	*(pMacad+5) = (UCHAR)((content>>24)&0x000000FF);
	*(pMacad+4) = (UCHAR)((content>>16)&0x000000FF);
	*(pMacad+3) = (UCHAR)((content>>8)&0x000000FF);
	*(pMacad+2) = (UCHAR)((content)&0x000000FF);
}


#if 0
void CAM_mark_invalid(struct rtl8192cd_priv *priv,UCHAR ucIndex)
{
	ULONG ulCommand=0;
	ULONG ulContent=0;

	// polling bit, and No Write enable, and address
	ulCommand = CAM_CONTENT_COUNT*ucIndex;
	ulCommand = ulCommand | _CAM_POLL_ |_CAM_WE_;
	// write content 0 is equall to mark invalid
	RTL_W32(_CAM_W_, ulContent);
	RTL_W32(_CAMCMD_, ulCommand);
}
#endif


static void CAM_empty_entry(struct rtl8192cd_priv *priv,unsigned char index)
{
	unsigned long command=0, content=0;
	unsigned int i;

	for (i = 0; i < CAM_CONTENT_COUNT; i++) {
		// polling bit, and No Write enable, and address
		command = CAM_CONTENT_COUNT*index+i;
		command = command | SECCAM_POLL |SECCAM_WE;
		// write content 0 is equal to mark invalid
		RTL_W32(CAMWRITE, content);
		RTL_W32(CAMCMD, command);
	}
}


int CamDeleteOneEntry(struct rtl8192cd_priv *priv, unsigned char *pMacAddr, unsigned long keyId, unsigned int useDK)
{
	unsigned char ucIndex;
	unsigned char ucTempMAC[6];
	unsigned short usTempConfig=0;
	int for_begin=4;

#ifdef  CONFIG_WLAN_HAL
    CAM_ENTRY_CFG   CamEntryCfg;
#endif

	// group key processing
	if ((!memcmp(pMacAddr, "\xff\xff\xff\xff\xff\xff", 6)) || (useDK)) {
#ifdef  CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {
	        GET_HAL_INTERFACE(priv)->CAMReadMACConfigHandler(priv, keyId, ucTempMAC, &CamEntryCfg);
	        if ( _TRUE == CamEntryCfg.bValid ) {
	            GET_HAL_INTERFACE(priv)->CAMEmptyEntryHandler(priv, keyId);

	            if (priv->pshare->CamEntryOccupied == 1) {
	                SECURITY_CONFIG_OPERATION  SCO = 0;
                GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_SECURITY_CONFIG, (u16 *)&SCO);
	            }

	            return 1;
	        }
	        else {
	            return 0;
	        }
		} else
#endif
		{
			CAM_read_mac_config(priv, keyId, ucTempMAC, &usTempConfig);
			if (usTempConfig&CFG_VALID) {
				CAM_empty_entry(priv, keyId);
				if (priv->pshare->CamEntryOccupied == 1)
					RTL_W16(SECCFG, 0);
				return 1;
			}
			else
				return 0;
		}
	}
#ifdef  CONFIG_WLAN_HAL
    // TODO:    check 8881A desgin below
#endif  //CONFIG_WLAN_HAL

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8188E) {
#ifdef CONFIG_RTL_HW_WAPI_SUPPORT
		for_begin = 3;
#else
		for_begin = 2;
#endif
#ifdef MBSSID
		if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
			for_begin = 0;
#endif
#ifdef UNIVERSAL_REPEATER
		if (IS_VXD_INTERFACE(priv)) {
			for_begin = 0;
		} else {
			if (IS_ROOT_INTERFACE(priv)) {
				if (IS_DRV_OPEN(GET_VXD_PRIV(priv)))
					for_begin = 0;
			}
		}
#endif
	}
#endif

	// unicast key processing
	// key processing for RTL818X(B) series
	for(ucIndex = for_begin; ucIndex < TOTAL_CAM_ENTRY; ucIndex++) {
#ifdef  CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {
	        GET_HAL_INTERFACE(priv)->CAMReadMACConfigHandler(priv, ucIndex, ucTempMAC, &CamEntryCfg);
	        if(!memcmp(pMacAddr, ucTempMAC, 6)) {
	            #if defined(CONFIG_RTL_HW_WAPI_SUPPORT)
	            if ( (CamEntryCfg.EncAlgo == DOT11_ENC_WAPI) && (CamEntryCfg.KeyID != keyId)) {
	                continue;
	            }
	            #endif            

	            GET_HAL_INTERFACE(priv)->CAMEmptyEntryHandler(priv, ucIndex);

	            if (priv->pshare->CamEntryOccupied == 1) {
	                SECURITY_CONFIG_OPERATION  SCO = 0;
                GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_SECURITY_CONFIG, (u16 *)&SCO);
	            }
	            return 1;
	        }
		} else
#endif
		{
			CAM_read_mac_config(priv, ucIndex, ucTempMAC, &usTempConfig);
			if(!memcmp(pMacAddr, ucTempMAC, 6)) {

	#if defined(CONFIG_RTL_HW_WAPI_SUPPORT)
				if((((usTempConfig & 0x1c) >> 2) == DOT11_ENC_WAPI)&&((usTempConfig & 0x3) != keyId))
					continue;
	#endif
				CAM_empty_entry(priv, ucIndex);	// reset MAC address, david+2007-1-15

				if (priv->pshare->CamEntryOccupied == 1)
					RTL_W16(SECCFG, 0);

				return 1;
			}
		}
	}
	return 0;
}


/*now use empty to fill in the first 4 entries*/
void CamResetAllEntry(struct rtl8192cd_priv *priv)
{

#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
    	GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_CAM_RESET_ALL_ENTRY, NULL);
	}
#endif
	{
		unsigned char index;

		RTL_W32(CAMCMD, SECCAM_CLR);

		for(index = 0; index < TOTAL_CAM_ENTRY; index++)
			CAM_empty_entry(priv,index);

		RTL_W32(CR, RTL_R32(CR) & (~MAC_SEC_EN));
	}
	priv->pshare->CamEntryOccupied = 0;
	priv->pmib->dot11GroupKeysTable.keyInCam = 0;

//	RTL_W32(CR, RTL_R32(CR) & (~MAC_SEC_EN));
}


void CAM_read_entry(struct rtl8192cd_priv *priv, unsigned char index, unsigned char* macad,
	unsigned char* key128, unsigned short* config)
{
	unsigned long  target_command = 0, target_content = 0;
	unsigned char entry_i = 0;
	unsigned long status;

	for (entry_i=0; entry_i<CAM_CONTENT_USABLE_COUNT; entry_i++)
	{
		// polling bit, and No Write enable, and address
		target_command = (unsigned long)(entry_i+CAM_CONTENT_COUNT*index);
		target_command = target_command | SECCAM_POLL;

		RTL_W32(CAMCMD, target_command);
	   	//Check polling bit is clear
		while(1) {
			status = RTL_R32(CAMCMD);
			if(status & SECCAM_POLL)
				continue;
			else
				break;
		}
		target_content = RTL_R32(CAMREAD);

		if (entry_i==0) {
			//first 32-bit is MAC address and CFG field
		    *(config)  = (unsigned short)((target_content)&0x0000FFFF);
		    *(macad+0) = (unsigned char)((target_content>>16)&0x000000FF);
		    *(macad+1) = (unsigned char)((target_content>>24)&0x000000FF);
		}
		else if (entry_i==1) {
			*(macad+5) = (unsigned char)((target_content>>24)&0x000000FF);
		    *(macad+4) = (unsigned char)((target_content>>16)&0x000000FF);
	    	*(macad+3) = (unsigned char)((target_content>>8)&0x000000FF);
	    	*(macad+2) = (unsigned char)((target_content)&0x000000FF);
    		}
		else {
	    	*(key128+(entry_i*4-8)+3) = (unsigned char)((target_content>>24)&0x000000FF);
	    	*(key128+(entry_i*4-8)+2) = (unsigned char)((target_content>>16)&0x000000FF);
	    	*(key128+(entry_i*4-8)+1) = (unsigned char)((target_content>>8)&0x000000FF);
	    	*(key128+(entry_i*4-8)+0) = (unsigned char)(target_content&0x000000FF);
		}

		target_content = 0;
	}
}


#if 0
void debug_cam(UCHAR*TempOutputMac, UCHAR*TempOutputKey, USHORT TempOutputCfg)
{
	printk("MAC Address\n");
	printk(" %X %X %X %X %X %X\n",*TempOutputMac
					    ,*(TempOutputMac+1)
					    ,*(TempOutputMac+2)
					    ,*(TempOutputMac+3)
					    ,*(TempOutputMac+4)
					    ,*(TempOutputMac+5));
	printk("Config:\n");
	printk(" %X\n",TempOutputCfg);

	printk("Key:\n");
	printk("%X %X %X %X,%X %X %X %X,\n%X %X %X %X,%X %X %X %X\n"
	      ,*TempOutputKey,*(TempOutputKey+1),*(TempOutputKey+2)
	      ,*(TempOutputKey+3),*(TempOutputKey+4),*(TempOutputKey+5)
	      ,*(TempOutputKey+6),*(TempOutputKey+7),*(TempOutputKey+8)
	      ,*(TempOutputKey+9),*(TempOutputKey+10),*(TempOutputKey+11)
	      ,*(TempOutputKey+12),*(TempOutputKey+13),*(TempOutputKey+14)
	      ,*(TempOutputKey+15));
}


void CamDumpAll(struct rtl8192cd_priv *priv)
{
	UCHAR TempOutputMac[6];
	UCHAR TempOutputKey[16];
	USHORT TempOutputCfg=0;
	unsigned long flags;
	int i;

	SAVE_INT_AND_CLI(flags);
	for (i=0; i<TOTAL_CAM_ENTRY; i++)
	{
		printk("%X-", i);
		CAM_read_entry(priv,i, TempOutputMac, TempOutputKey, &TempOutputCfg);
		debug_cam(TempOutputMac, TempOutputKey, TempOutputCfg);
		printk("\n\n");
	}
	RESTORE_INT(flags);
}


void CamDump4(struct rtl8192cd_priv *priv)
{
	UCHAR TempOutputMac[6];
	UCHAR TempOutputKey[16];
	USHORT TempOutputCfg=0;
	unsigned long flags;
	int i;

	SAVE_INT_AND_CLI(flags);
	for (i=0; i<4; i++)
	{
		printk("%X", i);
		CAM_read_entry(priv, i, TempOutputMac, TempOutputKey, &TempOutputCfg);
		debug_cam(TempOutputMac, TempOutputKey, TempOutputCfg);
		printk("\n\n");
	}
	RESTORE_INT(flags);
}
#endif



/*
 *
 * Power Saving related functions
 *
 */
#ifdef PCIE_POWER_SAVING

#ifdef CONFIG_RTL_92D_DMDP
extern u32 if_priv[];
#endif

#ifdef CONFIG_RTL_92D_DMDP

void Sw_PCIE_Func2(int func)
{
#if (RTL_USED_PCIE_SLOT==1)
	int reg = 0xb8b2100c;
#else
	int reg = 0xb8b0100c;
#endif

	REG32(reg) &= ~(1);
	REG32(reg) |= func; // switch to function #
}
#endif

#if defined(__LINUX_2_6__)
extern void HostPCIe_SetPhyMdioWrite(unsigned int , unsigned int , unsigned short );
#endif
#ifdef ASPM_ENABLE
void ASPM_on_off(struct rtl8192cd_priv *priv) ;
#endif

#ifndef CONFIG_RTL_8198B
#define CLK_MANAGE	0xb8000010
#endif
#ifdef PCIE_POWER_SAVING_DEBUG
int PCIE_PowerDown(struct rtl8192cd_priv *priv, unsigned char *data)
{
//  #define PCIE_PHY0  0xb8b01008
	
	#define dprintf printk
	int tmp, mode, portnum=0;
	unsigned int PCIE_PHY0, linkstatus;
	unsigned int haddr, saddr;
	
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		haddr = CFG_92D_SLOTH;
		saddr = CFG_92D_SLOTS;
	} else {
		haddr = CFG_92C_SLOTH;
		saddr = CFG_92C_SLOTS;
	}

  	PCIE_PHY0 = haddr + 0x1008;	  
  	linkstatus = haddr + 0x728;

#ifdef CONFIG_RTL_92D_DMDP
	Sw_PCIE_Func2(priv->pshare->wlandev_idx);
#endif

#if defined(CONFIG_RTL_92D_SUPPORT)
	portnum = RTL_USED_PCIE_SLOT;
#endif

	mode = _atoi(data, 16);

	if(strlen(data)==0) {
		dprintf("epdn mode.\n");
		dprintf("epdn 0: D0 ->L0 \n");
		dprintf("epdn 3: D3hot ->L1 \n");
		dprintf("epdn 4: board cast PME_TurnOff \n");
		dprintf("epdn 7: enable aspm and L0 entry \n");
		dprintf("epdn 8: enable aspm and L1 entry \n");
		dprintf("epdn 9: diable  aspm \n");
		dprintf("epdn 5a: pcie reset \n");
		dprintf("epdn 6a: L0 -> L2 \n");
		dprintf("epdn 6b: L2 -> L0\n");
		dprintf("epdn 6c: L0 -> L1 \n");
		dprintf("epdn 6d: L1 -> L0\n");
		dprintf("epdn 6e: download probe rsp\n");
		dprintf("epdn a3: wake pin test\n");
		dprintf("epdn b: bar\n");
		dprintf("epdn b1: offload enable \n");
		dprintf("epdn b2: offload disable\n");
		dprintf("epdn c1: swith to 1T\n");
		dprintf("epdn c2: switch to 2T\n");
		dprintf("Link status=%x \n", REG32(linkstatus)&0x1f );
		return 0;
	}

	if(mode==0) {

#ifdef CONFIG_RTL_92D_DMDP
	 	if (GET_CHIP_VER(priv) == VERSION_8192D) {
			if(priv->pshare->wlandev_idx !=0) {
				dprintf("not Root Interface!! \n");
				return 0;
			}			
			Sw_PCIE_Func2(0);
	 	}
#endif

#ifdef SAVING_MORE_PWR
		HostPCIe_SetPhyMdioWrite(portnum, 0xf, 0x0f0f);
#endif
		tmp = REG32(0xb8b10044) &( ~(3)); 
		REG32(0xb8b10044) = tmp|	(0);  //D0
		delay_ms(1);
		REG32(0xb8b10044) = tmp|	(0);  //D0
		dprintf("D0 \n");
		priv->pwr_state = L0;

#if defined(CONFIG_RTL_92D_SUPPORT)
		if (GET_CHIP_VER(priv) == VERSION_8192D) {
#ifdef CONFIG_RTL_92D_DMDP			
//		   	if(priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY)  
	   		{
				Sw_PCIE_Func2(1);	   		
#ifdef SAVING_MORE_PWR
				HostPCIe_SetPhyMdioWrite(portnum, 0xf, 0x0f0f);
#endif	   		
				tmp = REG32(0xb8b10044) &( ~(3));  //D0
				REG32(0xb8b10044) = tmp|	(0);  //D0
				delay_ms(1);
				REG32(0xb8b10044) = tmp|	(0);  //D0
				dprintf("D0 wlan1\n");
				((struct rtl8192cd_priv *)if_priv[1])->pwr_state = L0;
		   	}
			Sw_PCIE_Func2(0);
#endif
			delay_ms(1);
	  }
#endif
		
	 }

	 if(mode==3) {

#if defined(CONFIG_RTL_92D_SUPPORT)
		if (GET_CHIP_VER(priv) == VERSION_8192D){
#ifdef CONFIG_RTL_92D_DMDP
			if(priv->pshare->wlandev_idx !=0) {
				dprintf("not Root Interface!! \n");
				return 0;
			}
//			if(priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY) 
			{

				dprintf("DMDP, disable wlan1 !!\n");
				Sw_PCIE_Func2(1);
#ifdef SAVING_MORE_PWR
		  		REG32(0xb8b10080)|= (0x100);	//enable clock PM
#endif			
				tmp = REG32(0xb8b10044) &( ~(3));  
				REG32(0xb8b10044) = tmp|	(3);  //D3

				((struct rtl8192cd_priv *)if_priv[1])->pwr_state = L1;
#ifdef SAVING_MORE_PWR
	  			HostPCIe_SetPhyMdioWrite(portnum, 0xf, 0x0708);
#endif				
			}
			Sw_PCIE_Func2(0);
#endif
			delay_ms(1);

		}
#endif

#ifdef SAVING_MORE_PWR
	  	REG32(0xb8b10080)|= (0x100);	//enable clock PM
#endif
	   	tmp = REG32(0xb8b10044) &( ~(3));  
	  	REG32(0xb8b10044) = tmp|	(3);  //D3
	  	//HostPCIe_SetPhyMdioWrite(0xd, 0x15a6);
	  	dprintf("D3 hot \n");
		priv->pwr_state = L1;
#ifdef SAVING_MORE_PWR
	  	HostPCIe_SetPhyMdioWrite(portnum, 0xf, 0x0708);
#endif
	 }

	  if(mode==4) {

		RTL_W8(0x1c, 0xe1); 	// reg lock, dis_prst
		RTL_W8(0x1c, 0xe1);

#ifdef SAVING_MORE_PWR
	   HostPCIe_SetPhyMdioWrite(portnum, 0xf, 0x0f0f);
#endif

	   REG32(0xb8b01008) |= (0x200);
	   dprintf("Host boardcase PME_TurnOff \n");
		 priv->pwr_state = L2;
	  }


	 if(mode==0xd) {

		 RTL_W8(0x1c, 0x0);
		 priv->pshare->phw->cur_rx =0;
#ifdef DELAY_REFILL_RX_BUF
		 priv->pshare->phw->cur_rx_refill =0;
#endif
		memset(&(priv->pshare->phw->txhead0), 0, sizeof(int)*12);

		 RTL_W8(SPS0_CTRL, 0x2b);
		 RTL_W32(BCNQ_DESA, priv->pshare->phw->tx_ringB_addr);
		 RTL_W32(MGQ_DESA, priv->pshare->phw->tx_ring0_addr);
		 RTL_W32(VOQ_DESA, priv->pshare->phw->tx_ring4_addr);
		 RTL_W32(VIQ_DESA, priv->pshare->phw->tx_ring3_addr);
		 RTL_W32(BEQ_DESA, priv->pshare->phw->tx_ring2_addr);
		 RTL_W32(BKQ_DESA, priv->pshare->phw->tx_ring1_addr);
		 RTL_W32(HQ_DESA, priv->pshare->phw->tx_ring5_addr);
		 RTL_W32(RX_DESA, priv->pshare->phw->ring_dma_addr);

	 }

	 if(mode==7)	 {
	  REG32(0xb8b1070c) &= ~  ((0x7 <<27)|(0x7<<24));
	  REG32(0xb8b1070c) |=	((3)<<27) | ((1)<<24);	 //L1=8us, L0s=2us

	  REG32(0xb8b00080) &= ~(0x3);
	  REG32(0xb8b10080) &= ~(0x3);

	  REG32(0xb8b00080) |= 1;	//L0s
	  REG32(0xb8b10080) |= 1;
	 priv->pwr_state=ASPM_L0s_L1;
	 }

	 if(mode==8)	 {
	  REG32(0xb8b1070c) &= ~  ((0x7 <<27)|(0x7<<24));
	  REG32(0xb8b1070c) |=	((1)<<27) | ((3)<<24);	 //L1=2us, L0s=4us

	  REG32(0xb8b00080) &= ~(0x3);
	  REG32(0xb8b10080) &= ~(0x3);

	  REG32(0xb8b00080) |= 3;	//L1
	  REG32(0xb8b10080) |= 3; //L1
	  priv->pwr_state=ASPM_L0s_L1;

	 }

	 if(mode==9)	 {
	  REG32(0xb8b00080) &= ~(0x3);
	  REG32(0xb8b10080) &= ~(0x3);
	  priv->pwr_state=L0;
	 }


	 if(mode==0x6a)	{
		priv->ps_ctrl = 1 | 32 |0x80;
	  	 mod_timer(&priv->ps_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(100));
	 }

	 if(mode==0x6c)	{
		 priv->ps_ctrl = 1 | 16 |0x80;
//	  	mod_timer(&priv->ps_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(100));
		 PCIe_power_save_tasklet((unsigned long)priv);
	 }

	 if((mode==0x6b) || (mode==0x6d))		{
		priv->ps_ctrl = 0x82 | (priv->pwr_state<<4);
		priv->pshare->rf_ft_var.power_save &=0xf0;

#ifdef CONFIG_RTL_92D_DMDP
		((struct rtl8192cd_priv *)if_priv[1])->pshare->rf_ft_var.power_save &=0xf0;
#endif			
		PCIe_power_save_tasklet((unsigned long)priv);
		signin_h2c_cmd(priv, _AP_OFFLOAD_CMD_ , 0 );
#ifdef CONFIG_RTL_92D_DMDP
		signin_h2c_cmd(((struct rtl8192cd_priv *)if_priv[1]), _AP_OFFLOAD_CMD_ , 0 );
#endif					
	}

	if(mode==0x6e) {
		priv->offload_ctrl = 1;
		RTL_W16(0x100 , RTL_R16(0x100) | BIT(8));		// enable sw beacon
		tasklet_hi_schedule(&priv->pshare->rx_tasklet);
	}

	if(mode==0xc1)	{
//		PHY_ConfigBBWithParaFile(priv,	PATHB_OFF);
		switch_to_1x1(priv, PWR_STATE_IN);

	}

	 if(mode==0xc2)	 {
//		 PHY_ConfigBBWithParaFile(priv,  PATHB_ON);
		switch_to_1x1(priv, PWR_STATE_OUT);
 	}

	 if(mode==0xb)	 {
		 REG32(0xb8b00004) = 0x00100007;
		 REG32(0xb8b10004) = 0x00100007;
		 REG32(0xb8b10010) = 0x18c00001;
		 REG32(0xb8b10018) = 0x19000004;
		printk("b1-00=%x, b0-04=%x, b1-04=%x, b1-10=%x, b1-18=%x\n", REG32(0xb8b10000),
			REG32(0xb8b00004),REG32(0xb8b10004), REG32(0xb8b10010), REG32(0xb8b10018) );
	 }

	 if(mode==0xb1)	 {
		 unsigned int  cmd = _AP_OFFLOAD_CMD_ | (1<<8) |(HIDDEN_AP<<16) | ((GET_MIB(priv))->dot11OperationEntry.deny_any)<<24;
		int page = ((priv->offload_ctrl)>>7)&0xff;
		int cmd2=0, cmd2e=0;

		if(!page) {
			page = 2;
		}
			
		if(GET_CHIP_VER(priv) != VERSION_8192D) {
			cmd2= (_RSVDPAGE_CMD_ | page<<8) ;
		} else {
			cmd2 = ( _RSVDPAGE_CMD_ | BIT(7) | (page<<8));
			cmd2e= (page<<8)| (page) ;
		}
			
//		RTL_W16(PCIE_CTRL_REG, 0xff00 );
		REG32(saddr+0x44) |= 0x8108;

		printk("cmd: %x %x\n", cmd2, cmd2e);		
		signin_h2c_cmd(priv, cmd2, cmd2e);
		delay_ms(10);
		signin_h2c_cmd(priv,cmd, 0 );
		printk("sign in h2c cmd:%x, 0x284=%x\n", cmd, RTL_R32(0x284));

#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
		REG32(0xb8003000) |= BIT(16);		// GIMR
#else
		REG32(0xb8003000) |= BIT(9);		// GIMR
#endif
		
		delay_ms(10);
	 }

	 if(mode==0xb2)	 {
		signin_h2c_cmd(priv, 0x0000, 0);	// offload disable
		RTL_W8(0x423, 0x0); 		// mac seq disable
		RTL_W8(0x286, 0);			// RW_RELEASE_ENABLE		
		RTL_W16(PCIE_CTRL_REG, 0x00ff );
	 }

	 //static unsigned int Buffer[9];

	 if(mode==0xa3) {
		unsigned char tmp;
#ifdef RTL8676_WAKE_GPIO
				int gpio_num, irq_num;
		
				get_wifi_wake_pin(&gpio_num);
				irq_num = gpioGetBspIRQNum(gpio_num);
				
				gpioConfig(gpio_num, GPIO_FUNC_INPUT);
				gpioSetIMR(gpio_num, EN_FALL_EDGE_ISR); 	// enable interrupt in falling-edge
				REG32(BSP_GIMR) |= BIT(irq_num);
		
#else

#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)

     if(GET_CHIP_VER(priv) == VERSION_8192D) {
        REG32(0xb8000044) |= BIT(16)| BIT(17);                  //LEDPHASE1
        REG32(0xb8003500) &= ~(BIT(17));                        //PABCD_CNR , gpio pin
        REG32(0xb8003508) &= ~(BIT(17));                        //PABCD_DIR
        REG32(0xb8003518) &= (~(0x03 << 2));
        REG32(0xb8003518) |= (0x01 << 2);                       // PCD_IMR       
      } else {
        REG32(0xb8000044) |= BIT(24);                           //LEDPHASE4
        REG32(0xb8003500) &= ~(BIT(20));                        //PABCD_CNR , gpio pin
        REG32(0xb8003508) &= ~(BIT(20));                        //PABCD_DIR
        REG32(0xb8003518) &= (~(0x03 << 8));
        REG32(0xb8003518) |= (0x01 << 8);                       // PCD_IMR      
      }

		REG32(0xb8003000) |= BIT(16);		// GIMR
#else
		REG32(0xb8000040) |= 0x0c00;		//LEDPHASE1 :GPIOB7
		REG32(0xb8003500) &= ~(BIT(15));;	//PABCD_CNR , gpio pin
		REG32(0xb8003508) &= ~(BIT(15));	//PABCD_DIR
		REG32(0xb8003514) &= (~(0x03 << 30));
		REG32(0xb8003514) |= (0x01 << 30);	// PAB_IMR		// enable interrupt in falling-edge
		REG32(0xb8003000) |= BIT(9);		// GIMR
#endif

#endif
		// clear wake pin status
#ifdef CONFIG_RTL_92D_DMDP
		Sw_PCIE_Func2(priv->pshare->wlandev_idx);
#endif

		REG32(saddr+0x44) = 0x8108;
		tmp = RTL_R8(0x690);
		if(tmp&1)		{
			tmp ^=0x1;
			RTL_W8(0x690, tmp);
		}
		dprintf("0xb8b10044=%x,690=%x,3000=%x, 3514=%x\n",	REG32(saddr+0x44), RTL_R8(0x690), REG32(0xb8003000),REG32(0xb8003514) );
		RTL_W8(0x690, tmp |0x1 );
		dprintf("0xb8b10044=%x,690=%x\n",	REG32(saddr+0x44), RTL_R8(0x690) );
	 }

	if(mode==0x5a)
		PCIE_reset_procedure3(priv);

	//-------------------------------------------------------------
	if(mode==0x010) {	//L0->L1->L0		
		tmp = REG32(0xb8b10044) &( ~(3));  //D0
		REG32(0xb8b10044) = tmp| (3);  //D3
		REG32(0xb8b10044) = tmp| (0);  //D0, wakeup

		while(1) {
			if((REG32(linkstatus)&0x1f)==0x11)	 //wait to L0
				break;
		}

	  dprintf("DID/VID=%x\n", REG32(0xb8b10000));
	 }
	 //-------------------------------------------------------------
	 if(mode==0x020) {		//L0->L2->L0				 
	  tmp = REG32(0xb8b10044) &( ~(3));  //D0

	  REG32(0xb8b10044) = tmp|	(3);  //D3
	  delay_ms(100);

	  REG32(0xb8b01008) |= (0x200);
		delay_ms(100);

	 //wakeup
		 REG32(CLK_MANAGE) &= ~(1<<12);	  //perst=0 off.
				//dprintf("CLK_MANAGE=%x \n",  REG32(CLK_MANAGE));
			delay_ms(100);
			delay_ms(100);
			delay_ms(100);

		REG32(CLK_MANAGE) |=  (1<<12);	//PERST=1
		//prom_printf("\nCLK_MANAGE(0x%x)=0x%x\n\n",CLK_MANAGE,READ_MEM32(CLK_MANAGE));

			//4. PCIE PHY Reset
		REG32(PCIE_PHY0) = 0x01; //bit7 PHY reset=0   bit0 Enable LTSSM=1
		REG32(PCIE_PHY0) = 0x81;   //bit7 PHY reset=1	bit0 Enable LTSSM=1

	 while(1)	 {
	  if( (REG32(linkstatus)&0x1f)==0x11)
		 break;
	 }

	  dprintf("DID/VID=%x\n", REG32(0xb8b10000));
	 }

	  dprintf("Link status=%x\n", READ_MEM32(linkstatus)&0x1f/*, READ_MEM32(linkstatus), REG32(linkstatus)*/ );

return 0;
}
#endif


void switch_to_1x1(struct rtl8192cd_priv *priv, int mode)
{

	if(mode==PWR_STATE_IN) 	{

		priv->pshare->rf_phy_bb_backup[21]= RTL_R32(0x88c);

		priv->pshare->rf_phy_bb_backup[0]= RTL_R32(0x844);
		priv->pshare->rf_phy_bb_backup[1]= RTL_R32(0x85c);
		priv->pshare->rf_phy_bb_backup[2]= RTL_R32(0xe6c);

		priv->pshare->rf_phy_bb_backup[3]= RTL_R32(0xe70);
		priv->pshare->rf_phy_bb_backup[4]= RTL_R32(0xe74);
		priv->pshare->rf_phy_bb_backup[5]= RTL_R32(0xe78);
		priv->pshare->rf_phy_bb_backup[6]= RTL_R32(0xe7c);

		priv->pshare->rf_phy_bb_backup[7] = RTL_R32(0xe80);
		priv->pshare->rf_phy_bb_backup[8] = RTL_R32(0xe84);
		priv->pshare->rf_phy_bb_backup[9] = RTL_R32(0xe88);
		priv->pshare->rf_phy_bb_backup[10]= RTL_R32(0xe8c);

		priv->pshare->rf_phy_bb_backup[11]= RTL_R32(0xed0);
		priv->pshare->rf_phy_bb_backup[12]= RTL_R32(0xed4);
		priv->pshare->rf_phy_bb_backup[13]= RTL_R32(0xed8);
		priv->pshare->rf_phy_bb_backup[14]= RTL_R32(0xedc);

		priv->pshare->rf_phy_bb_backup[15]= RTL_R32(0xee0);
		priv->pshare->rf_phy_bb_backup[16]= RTL_R32(0xeec);

		priv->pshare->rf_phy_bb_backup[17]= RTL_R32(0xc04);
		priv->pshare->rf_phy_bb_backup[18]= RTL_R32(0xd04);
		priv->pshare->rf_phy_bb_backup[19]= RTL_R32(0x90c);
		priv->pshare->rf_phy_bb_backup[20]= RTL_R32(0x804);
		priv->pshare->rf_phy_bb_backup[22]= RTL_R32(0xa04);
		
#ifdef CONFIG_RTL_92D_SUPPORT
		if ((GET_CHIP_VER(priv)==VERSION_8192D)) {
			unsigned int mask = 0xB4FFFFFF, path=0x11;
			if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G)	{		
				mask = 0xDB3FFFFF;
				path=0x22;
				RTL_W8(0xa07, 0x45);
			}
			 PHY_SetBBReg(priv, 0x85c, bMaskDWord, priv->pshare->rf_phy_bb_backup[1] & mask);
			 PHY_SetBBReg(priv, 0xe6c, bMaskDWord, priv->pshare->rf_phy_bb_backup[2] & mask);

			 PHY_SetBBReg(priv, 0xe70, bMaskDWord, priv->pshare->rf_phy_bb_backup[3] & mask);
			 PHY_SetBBReg(priv, 0xe74, bMaskDWord, priv->pshare->rf_phy_bb_backup[4] & mask);
			 PHY_SetBBReg(priv, 0xe78, bMaskDWord, priv->pshare->rf_phy_bb_backup[5] & mask);
			 PHY_SetBBReg(priv, 0xe7c, bMaskDWord, priv->pshare->rf_phy_bb_backup[6] & mask);

			 PHY_SetBBReg(priv, 0xe80, bMaskDWord, priv->pshare->rf_phy_bb_backup[7] & mask);
			 PHY_SetBBReg(priv, 0xe84, bMaskDWord, priv->pshare->rf_phy_bb_backup[8] & mask);
			 PHY_SetBBReg(priv, 0xe88, bMaskDWord, priv->pshare->rf_phy_bb_backup[9] & mask);
			 PHY_SetBBReg(priv, 0xe8c, bMaskDWord, priv->pshare->rf_phy_bb_backup[10] & mask);

			 PHY_SetBBReg(priv, 0xed0, bMaskDWord, priv->pshare->rf_phy_bb_backup[11] & mask);
			 PHY_SetBBReg(priv, 0xed4, bMaskDWord, priv->pshare->rf_phy_bb_backup[12] & mask);
			 PHY_SetBBReg(priv, 0xed8, bMaskDWord, priv->pshare->rf_phy_bb_backup[13] & mask);
			 PHY_SetBBReg(priv, 0xedc, bMaskDWord, priv->pshare->rf_phy_bb_backup[14] & mask);

			 PHY_SetBBReg(priv, 0xee0, bMaskDWord, priv->pshare->rf_phy_bb_backup[15] & mask);
			 PHY_SetBBReg(priv, 0xeec, bMaskDWord, priv->pshare->rf_phy_bb_backup[16] & mask);

	 		 PHY_SetBBReg(priv, 0xc04, 0x000000ff, path);
			 PHY_SetBBReg(priv, 0xd04, 0x0000000f, path &0x01);
			 PHY_SetBBReg(priv, 0x90c, 0x000000ff, path);
			 PHY_SetBBReg(priv, 0x90c, 0x0ff00000, path);
		 
		}else
#endif
		{
			PHY_SetBBReg(priv, 0x88c, 0x00c00000 , 0x3);

#if 1
	// standby
			PHY_SetBBReg(priv, 0x844, bMaskDWord, 0x00010000);
#else
	// power off
			PHY_SetBBReg(priv, 0x844, bMaskDWord, 0x00000000);
#endif

			PHY_SetBBReg(priv, 0x85c, bMaskDWord, 0x00db25a4);
			PHY_SetBBReg(priv, 0xe6c, bMaskDWord, 0x20db25a4);

			PHY_SetBBReg(priv, 0xe70, bMaskDWord, 0x20db25a4);
			PHY_SetBBReg(priv, 0xe74, bMaskDWord, 0x041b25a4);
			PHY_SetBBReg(priv, 0xe78, bMaskDWord, 0x041b25a4);
			PHY_SetBBReg(priv, 0xe7c, bMaskDWord, 0x041b25a4);

			PHY_SetBBReg(priv, 0xe80, bMaskDWord, 0x041b25a4);
			PHY_SetBBReg(priv, 0xe84, bMaskDWord, 0x63db25a4);
			PHY_SetBBReg(priv, 0xe88, bMaskDWord, 0x041b25a4);
			PHY_SetBBReg(priv, 0xe8c, bMaskDWord, 0x20db25a4);

			PHY_SetBBReg(priv, 0xed0, bMaskDWord, 0x20db25a4);
			PHY_SetBBReg(priv, 0xed4, bMaskDWord, 0x20db25a4);
			PHY_SetBBReg(priv, 0xed8, bMaskDWord, 0x20db25a4);
			PHY_SetBBReg(priv, 0xedc, bMaskDWord, 0x001b25a4);

			PHY_SetBBReg(priv, 0xee0, bMaskDWord, 0x001b25a4);
			PHY_SetBBReg(priv, 0xeec, bMaskDWord, 0x24db25a4);

			PHY_SetBBReg(priv, 0xc04, 0x000000ff , 0x11);
			PHY_SetBBReg(priv, 0xd04, 0x0000000f , 0x1);
			PHY_SetBBReg(priv, 0x90c, 0x000000ff , 0x11);
			PHY_SetBBReg(priv, 0x90c, 0x0ff00000 , 0x11);
			
			PHY_SetBBReg(priv, 0x804, 0x000000f , 0x1);
		}
	} else if(mode==PWR_STATE_OUT)	 {

#ifdef CONFIG_RTL_92C_SUPPORT
		 if (GET_CHIP_VER(priv)==VERSION_8192C) {
		 	PHY_SetBBReg(priv, 0x88c, bMaskDWord, priv->pshare->rf_phy_bb_backup[21]);
		 	PHY_SetBBReg(priv, 0x844, bMaskDWord, priv->pshare->rf_phy_bb_backup[0]);
		 }		 
#endif
		 PHY_SetBBReg(priv, 0x85c, bMaskDWord, priv->pshare->rf_phy_bb_backup[1]);
		 PHY_SetBBReg(priv, 0xe6c, bMaskDWord, priv->pshare->rf_phy_bb_backup[2]);

		 PHY_SetBBReg(priv, 0xe70, bMaskDWord, priv->pshare->rf_phy_bb_backup[3]);
		 PHY_SetBBReg(priv, 0xe74, bMaskDWord, priv->pshare->rf_phy_bb_backup[4]);
		 PHY_SetBBReg(priv, 0xe78, bMaskDWord, priv->pshare->rf_phy_bb_backup[5]);
		 PHY_SetBBReg(priv, 0xe7c, bMaskDWord, priv->pshare->rf_phy_bb_backup[6]);

		 PHY_SetBBReg(priv, 0xe80, bMaskDWord, priv->pshare->rf_phy_bb_backup[7]);
		 PHY_SetBBReg(priv, 0xe84, bMaskDWord, priv->pshare->rf_phy_bb_backup[8]);
		 PHY_SetBBReg(priv, 0xe88, bMaskDWord, priv->pshare->rf_phy_bb_backup[9]);
		 PHY_SetBBReg(priv, 0xe8c, bMaskDWord, priv->pshare->rf_phy_bb_backup[10]);

		 PHY_SetBBReg(priv, 0xed0, bMaskDWord, priv->pshare->rf_phy_bb_backup[11]);
		 PHY_SetBBReg(priv, 0xed4, bMaskDWord, priv->pshare->rf_phy_bb_backup[12]);
		 PHY_SetBBReg(priv, 0xed8, bMaskDWord, priv->pshare->rf_phy_bb_backup[13]);
		 PHY_SetBBReg(priv, 0xedc, bMaskDWord, priv->pshare->rf_phy_bb_backup[14]);

		 PHY_SetBBReg(priv, 0xee0, bMaskDWord, priv->pshare->rf_phy_bb_backup[15]);
		 PHY_SetBBReg(priv, 0xeec, bMaskDWord, priv->pshare->rf_phy_bb_backup[16]);

		 PHY_SetBBReg(priv, 0xc04, bMaskDWord , priv->pshare->rf_phy_bb_backup[17]);
		 PHY_SetBBReg(priv, 0xd04, bMaskDWord , priv->pshare->rf_phy_bb_backup[18]);
		 PHY_SetBBReg(priv, 0x90c, bMaskDWord , priv->pshare->rf_phy_bb_backup[19]);
		 PHY_SetBBReg(priv, 0xa04, bMaskDWord , priv->pshare->rf_phy_bb_backup[22]);
		 
#ifdef CONFIG_RTL_92C_SUPPORT
	  	if (GET_CHIP_VER(priv)==VERSION_8192C) 	  
			PHY_SetBBReg(priv, 0x804, bMaskDWord , priv->pshare->rf_phy_bb_backup[20]);
#endif
 	}
}

#ifdef __LINUX_2_6__
irqreturn_t gpio_wakeup_isr(int irq, void *dev_instance);
#else
void gpio_wakeup_isr(int irq, void *dev_instance, struct pt_regs *regs);
#endif

//const unsigned int CLK_MANAGE =	0xb8000010;
//const unsigned int SYS_PCIE_PHY0   =(0xb8000000 +0x50);
void PCIE_reset_procedure3(struct rtl8192cd_priv *priv)

{

 	//PCIE Register
	unsigned int PCIE_PHY0_REG, PCIE_PHY0, linkstatus, haddr;
	int status=0, counter=0;
	
	if (GET_CHIP_VER(priv) == VERSION_8192D) 
		haddr = CFG_92D_SLOTH;
	 else 
		haddr = CFG_92C_SLOTH;

	PCIE_PHY0_REG  = haddr + 0x1000;
  	PCIE_PHY0 = haddr + 0x1008;	  
  	linkstatus = haddr + 0x728;


#if 0
	REG32(CLK_MANAGE) &= ~(1<<12);   //perst=0 off.
	//dprintf("CLK_MANAGE=%x \n",  REG32(CLK_MANAGE));
	delay_ms(3);
	delay_ms(3);

	REG32(CLK_MANAGE) |=  (1<<12);	//PERST=1
	//prom_printf("\nCLK_MANAGE(0x%x)=0x%x\n\n",CLK_MANAGE,READ_MEM32(CLK_MANAGE));

	//4. PCIE PHY Reset
	REG32(PCIE_PHY0) = 0x01; //bit7 PHY reset=0   bit0 Enable LTSSM=1
	REG32(PCIE_PHY0) = 0x81;   //bit7 PHY reset=1	bit0
	//
	delay_ms(3);

#else
	do{
		//2.Active LX & PCIE Clock
		REG32(CLK_MANAGE) &=  (~(1<<11));        //enable active_pcie0
		delay_ms(2);

		//4. PCIE PHY Reset
		REG32(PCIE_PHY0) = 0x1; //bit7 PHY reset=0   bit0 Enable LTSSM=1
		delay_ms(2);

		REG32(CLK_MANAGE) &= ~(1<<12);    //perst=0 off.
		delay_ms(5);

		REG32(PCIE_PHY0) = 0x81;   //bit7 PHY reset=1   bit0 Enable LTSSM=1
		delay_ms(5);

		REG32(CLK_MANAGE) |=  (1<<11);		  //enable active_pcie0

		//---------------------------------------
		// 6. PCIE Device Reset

		delay_ms(5);
		REG32(CLK_MANAGE) |=  (1<<12);   //PERST=1
		delay_ms(5);
		status = REG32(linkstatus)&0x1f;

		if( status == 0x11 ) {
			break;
		}else  {
			delay_ms(100);
//			printk("status=%x\n", status);
			if( ++counter>1000) {
//				panic_printk("PCIe reset fail!!!!\n");
				break;
			}
		}
	}while(1);



#endif

//	printk("PCIE_reset_procedure3\t devid=%x\n",REG32(0xb8b10000));

}

#ifdef ASPM_ENABLE
void ASPM_on_off(struct rtl8192cd_priv *priv)
{
	unsigned long flags;
	SAVE_INT_AND_CLI(flags);
	unsigned int haddr, saddr;
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		haddr = CFG_92D_SLOTH;
		saddr = CFG_92D_SLOTS;
	} else {
		haddr = CFG_92C_SLOTH;
		saddr = CFG_92C_SLOTS;
	}
#ifdef CONFIG_RTL_92D_DMDP
	Sw_PCIE_Func2(priv->pshare->wlandev_idx);
#endif

	if(priv->pshare->rf_ft_var.power_save &ASPM_en) {
		if(priv->pwr_state==L0) {
			
			 REG32(haddr+ 0x70c) &= ~  ((0x7 <<27)|(0x7<<24));
			 REG32(haddr+ 0x70c) |=  ((3)<<27) | ((1)<<24);	//L1=8us, L0s=2us
			 
			 REG32(haddr+ 0x80) &= ~(0x3);
			 REG32(saddr+ 0x80) &= ~(0x3);
			 REG32(haddr+ 0x80) |= 1;   //L0s
			 REG32(saddr+ 0x80) |= 1;
			 
			priv->pwr_state=ASPM_L0s_L1;
		}
	} else if(priv->pwr_state==ASPM_L0s_L1) {
		REG32(haddr+ 0x80) &= ~(0x3);
		REG32(saddr+ 0x80) &= ~(0x3);
		priv->pwr_state=L0;
	}

	RESTORE_INT(flags);
}
#endif

#ifdef GPIO_WAKEPIN
int request_irq_for_wakeup_pin(struct net_device *dev)
{
#ifdef NETDEV_NO_PRIV
	struct rtl8192cd_priv *priv = ((struct rtl8192cd_priv *)netdev_priv(dev))->wlan_priv;
#else
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
#endif
	unsigned int saddr;

	if (GET_CHIP_VER(priv) == VERSION_8192D)
		saddr = CFG_92D_SLOTS;
	else
		saddr = CFG_92C_SLOTS;

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)

	#if defined(CONFIG_RTL_ULINKER)
	{
		// GPIO B5, b8000044 [10:9] = 2'b 11
		REG32(0xb8000044) = (REG32(0xb8000044) & ~0x600) | 0x600;       //LEDPHASE1
		REG32(0xb8003500) &= ~(BIT(13));			//PABCD_CNR , gpio pin
		REG32(0xb8003508) &= ~(BIT(13));			//PABCD_DIR
		REG32(0xb8003514) &= (~(0x03 << 26));
		REG32(0xb8003514) |= (0x01 << 26);			// enable interrupt in falling-edge
		REG32(PABCD_ISR) = BIT(13) ;				// clear int status
	}
	#else

#if defined(CONFIG_RTL_92D_SUPPORT) && defined(CONFIG_RTL_8197D)
	if(GET_CHIP_VER(priv) == VERSION_8192D) {
		// GPIO B7, b8000044 [17:15] =3'b 100
		REG32(0xb8000044) = (REG32(0xb8000044) & ~0x00038000) | BIT(17);       //LEDPHASE1
		REG32(0xb8003500) &= ~(BIT(15));			//PABCD_CNR , gpio pin
		REG32(0xb8003508) &= ~(BIT(15));			//PABCD_DIR
		REG32(0xb8003514) &= (~(0x03 << 30));
		REG32(0xb8003514) |= (0x01 << 30);			// enable interrupt in falling-edge
		REG32(PABCD_ISR) = BIT(15) ;				// clear int status
		
	} else 
#endif
	{
		// GPIO A4, b8000040 [2:0] = 3'b 110
		REG32(0xb8000040) = (REG32(0xb8000040) & ~0x7) | 0x6;                //JTAG_TMS
		REG32(0xb8003500) &= ~(BIT(4));			//PABCD_CNR , gpio pin
		REG32(0xb8003508) &= ~(BIT(4));			//PABCD_DIR
		REG32(0xb8003514) &= (~(0x03 << 8));
		REG32(0xb8003514) |= (0x01 << 8);			// enable interrupt in falling-edge
		REG32(PABCD_ISR) = BIT(4) ;					// clear int status
	}
	#endif /* #if defined(CONFIG_RTL_ULINKER) */

  	REG32(0xb8003000) |= BIT(16);                   // GIMR

#elif defined(CONFIG_RTL_8198)
#ifdef CONFIG_RTL_92D_SUPPORT
	if(GET_CHIP_VER(priv) == VERSION_8192D) {
// GPIO C1
		REG32(0xb8000044) |= BIT(16)|BIT(17);       //LEDPHASE1
		REG32(0xb8003500) &= ~(BIT(17));			//PABCD_CNR , gpio pin
		REG32(0xb8003508) &= ~(BIT(17));			//PABCD_DIR
		REG32(0xb8003518) &= (~(0x03 << 2));
		REG32(0xb8003518) |= (0x01 << 2);			// enable interrupt in falling-edge
		REG32(PABCD_ISR) = BIT(17) ;				// clear int status
		
	} else 
#endif
	{
// GPIO C4
		REG32(0xb8000044) |= BIT(24);                //LEDPHASE4
		REG32(0xb8003500) &= ~(BIT(20));			//PABCD_CNR , gpio pin
		REG32(0xb8003508) &= ~(BIT(20));			//PABCD_DIR
		REG32(0xb8003518) &= (~(0x03 << 8));
		REG32(0xb8003518) |= (0x01 << 8);			// enable interrupt in falling-edge
		REG32(PABCD_ISR) = BIT(20) ;					// clear int status
	}
  	REG32(0xb8003000) |= BIT(16);                   // GIMR
#else
// GPIO B7
	REG32(0xb8000040) |= 0x0c00;				//LEDPHASE1 :GPIOB7
	REG32(0xb8003500) &= ~(BIT(15));;			//PABCD_CNR , gpio pin
	REG32(0xb8003508) &= ~(BIT(15));			//PABCD_DIR
	REG32(0xb8003514) &= (~(0x03 << 30));
	REG32(0xb8003514) |= (0x01 << 30);			// PAB_IMR		// enable interrupt in falling-edge
	REG32(0xb8003000) |= BIT(9);				// GIMR

	REG32(PABCD_ISR) = BIT(15) ;		// clear int status
#endif

#ifdef CONFIG_RTL_92D_DMDP
	Sw_PCIE_Func2(priv->pshare->wlandev_idx);
#endif	

	REG32(saddr+0x44) = 0x8108; 						// clear pme status

#ifdef CONFIG_RTL_92D_DMDP
	Sw_PCIE_Func2(0);
#endif	
	
#if defined(__LINUX_2_6__)
    return request_irq(BSP_GPIO_ABCD_IRQ, gpio_wakeup_isr, IRQF_SHARED, "rtk_gpio", dev);
#else
	return request_irq(1, gpio_wakeup_isr, SA_SHIRQ, "rtl_gpio", dev);
#endif

}
#endif


void init_pcie_power_saving(struct rtl8192cd_priv *priv)
{

	unsigned int saddr;
	if (GET_CHIP_VER(priv) == VERSION_8192D)
		saddr = CFG_92D_SLOTS;
	else
		saddr = CFG_92C_SLOTS;
	
// Jason : clk req
#if 0
		REG32(0xb9000354)=8;
		REG32(0xb9000358)=0x30;
#endif

#ifdef FIB_96C
   if (REG32(SYSTEM_BASE ) == 0x80000001) {
#if defined(__LINUX_2_6__)
#else
   	   extern void HostPCIe_SetPhyMdioWrite(unsigned int , unsigned int , unsigned short );
#endif
   	   HostPCIe_SetPhyMdioWrite(0, 8, 0x18d5);	// 18dd -> 18d5
	   HostPCIe_SetPhyMdioWrite(0, 0xd, 0x15a6);	// 15b6 -> 15a6
   }
#endif

// Jason , for ASPM read_reg
	if ((GET_CHIP_VER(priv) == VERSION_8192C)  || (GET_CHIP_VER(priv) == VERSION_8188C)) {
		RTL_W16(0x354, 0x18e);
		RTL_W16(0x358, 0x23);

		if ((GET_CHIP_VER(priv) == VERSION_8188C) ) {
		    RTL_W16(0x354, 0x20eb);
		    RTL_W16(0x358, 0x3d);
		}
	}

#ifdef CONFIG_RTL_92D_DMDP
	Sw_PCIE_Func2(priv->pshare->wlandev_idx);
#endif

	REG32(saddr+ 0x80)|= 0x0100;
#ifdef ASPM_ENABLE
	REG32(saddr+ 0x80)|= 0x43;
#endif
	REG32(saddr+ 0x0778)|= BIT(5)<<8;

#ifdef CONFIG_RTL_92C_SUPPORT

// 92c backdoor
	if ((GET_CHIP_VER(priv) == VERSION_8188C) || (GET_CHIP_VER(priv) == VERSION_8192C)) {
	   	REG32(saddr+ 0x70c)|= BIT(7)<<24;
	   	REG32(saddr+ 0x718)|= (BIT(3) | BIT(4))<<8;
//	   	dprintf("70f=%x,719=%x\n",  REG32(0xb8b1070f), REG32(0xb8b10719) );
	}
#endif

	RTL_W8(0x690, 0x2); 	// WoW
	RTL_W8(0x302, 0x2); 	// sw L123
	RTL_W8(0x5, 0x0);		// AFSM_PCIE
	RTL_W16(PCIE_CTRL_REG, 0xff );

//	RTL_W16(0x558, 0x040a);
//	RTL_W16(0x100 , RTL_R16(0x100) | BIT(8));		// enable sw beacon

#ifdef CONFIG_RTL_92C_SUPPORT
	if (IS_TEST_CHIP(priv)) {
		priv->pshare->rf_ft_var.power_save &= (~ L2_en);
		priv->pshare->rf_ft_var.power_save &= (~ASPM_en);
	} else 
#endif
	{
		RTL_W8(0x08, RTL_R8(0x08) | BIT(3));		// WAKEPAN_EN
#ifdef CONFIG_RTL_92C_SUPPORT		
		if(IS_UMC_A_CUT_88C(priv))
			priv->pshare->rf_ft_var.power_save &= (~ASPM_en);
#endif			

	}
#ifdef ASPM_ENABLE
	ASPM_on_off(priv);
#endif

#ifdef CONFIG_RTL_92D_DMDP
	Sw_PCIE_Func2(1);
#endif

}


int isPSConditionMatch(struct rtl8192cd_priv *priv)
{

// temporary disable Active ECO when 1 interfcae is disabled
#ifdef CONFIG_RTL_92D_DMDP
	if( (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY) &&
		(!IS_DRV_OPEN(((struct rtl8192cd_priv *)if_priv[1&(priv->pshare->wlandev_idx^1)]))))
		return 0;
#endif

	if(!IS_DRV_OPEN(priv))
		return 1;

	if( (priv->assoc_num==0) 
		&& (priv->pshare->rf_ft_var.power_save & (L1_en|L2_en))
#ifdef MBSSID
		&& (!priv->pmib->miscEntry.vap_enable)
#endif
#ifdef WDS
		&& (!priv->pmib->dot11WdsInfo.wdsEnabled)
#endif
#ifdef UNIVERSAL_REPEATER
		&& (!IS_DRV_OPEN(GET_VXD_PRIV(priv)))
#endif
#ifdef CLIENT_MODE
		&& !((OPMODE & WIFI_STATION_STATE) ||(OPMODE & WIFI_ADHOC_STATE))
#endif
	)
		return 1;
	else
		return 0;

}

#ifdef __KERNEL__
void PCIe_power_save_timer(unsigned long task_priv)
#elif defined(__ECOS)
void PCIe_power_save_timer(void *task_priv)
#endif
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	char force;
	force = priv->ps_ctrl & 0x80;
	priv->ps_ctrl &= 0x7f;

#ifdef MP_TEST
	if (priv->pshare->rf_ft_var.mp_specific)
		return ;
#endif

	if(!IS_DRV_OPEN(priv))
		return;

	if(force==0) {
#ifdef CONFIG_RTL_92D_DMDP		
		if(isPSConditionMatch((struct rtl8192cd_priv *)if_priv[0]) && isPSConditionMatch((struct rtl8192cd_priv *)if_priv[1]))
#else
		if(isPSConditionMatch(priv))
#endif
		{
			if(priv->pwr_state !=L1 && priv->pwr_state !=L2)
			{
				if((priv->offload_ctrl>>7)&&(priv->offload_ctrl&1)==0) {
				if (priv->pshare->rf_ft_var.power_save & L2_en)
					priv->ps_ctrl = 0x21;
					else
						priv->ps_ctrl = 0x11;
#ifdef CONFIG_RTL_92D_DMDP				
						if((priv->pshare->wlandev_idx ==1) && 
						(!IS_DRV_OPEN(((struct rtl8192cd_priv *)if_priv[0])))  ) {
							((struct rtl8192cd_priv *)if_priv[0])->ps_ctrl = priv->ps_ctrl;
							tasklet_schedule(&((struct rtl8192cd_priv *)if_priv[0])->pshare->ps_tasklet);
					} else	
					if((priv->pshare->wlandev_idx ==0) && 
						((!IS_DRV_OPEN(((struct rtl8192cd_priv *)if_priv[1]))) || (((struct rtl8192cd_priv *)if_priv[1])->offload_ctrl>>7)))
#endif					
						tasklet_schedule(&priv->pshare->ps_tasklet);
				} else {
					priv->offload_ctrl = 1;
					RTL_W16(CR , RTL_R16(CR) | ENSWBCN);		// enable sw beacon
					mod_timer(&priv->ps_timer, jiffies + RTL_SECONDS_TO_JIFFIES(1));
					return;
				}
			}
		}else {
			priv->offload_ctrl = 0;
		}
		
	}else {
		if(priv->pwr_state == L0)
			tasklet_schedule(&priv->pshare->ps_tasklet);
	}
	mod_timer(&priv->ps_timer, jiffies + POWER_DOWN_T0);

#ifdef ASPM_ENABLE
	ASPM_on_off(priv);
#endif

}

void setBaseAddressRegister(void)
{
	int tmp32=0, status;
	while(++tmp32 < 100) {
		REG32(0xb8b00004) = 0x00100007;
		REG32(0xb8b10004) = 0x00100007;
		REG32(0xb8b10010) = 0x18c00001;
		REG32(0xb8b10018) = 0x19000004;
		status = (REG32(0xb8b10010) ^ 0x18c00001) |( REG32(0xb8b10018) ^ 0x19000004);
		if(!status)
			break;
		else {
			printk("set BAR fail,%x\n", status);
			printk("%x %x %x %x \n",
			REG32(0xb8b00004) , REG32(0xb8b10004) ,REG32(0xb8b10010),  REG32(0xb8b10018) );
		}
	} ;
}

void PCIe_power_save_tasklet(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	char in_out, L1_L2;
	unsigned int tmp32=0,status=0,page=0, ctr;
#ifdef CONFIG_RTL_92D_SUPPORT
	unsigned int portnum=0, i;
#endif
	unsigned long flags;

	unsigned int saddr, haddr;
#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		saddr = CFG_92D_SLOTS;
		haddr = CFG_92D_SLOTH;

		portnum = RTL_USED_PCIE_SLOT;
	} else 
#endif
	{
		saddr = CFG_92C_SLOTS;
		haddr = CFG_92C_SLOTH;
	}

	priv->ps_ctrl &= 0x7f;
	in_out = priv->ps_ctrl & 0xf;
	L1_L2 = (priv->ps_ctrl>>4) & 0x7;

#if defined(CONFIG_RTL_92D_DMDP)
	if((in_out== PWR_STATE_IN) && (priv->pshare->wlandev_idx ==0) && (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY)) {
		struct rtl8192cd_priv *priv1 = ((struct rtl8192cd_priv *)if_priv[1]);
		priv1->ps_ctrl = priv->ps_ctrl;
		PCIe_power_save_tasklet((unsigned long )priv1);		
	}
#endif

	DEBUG_INFO("%s, %s, L%d\n", __FUNCTION__, (in_out==PWR_STATE_IN ? "in" : "out") , L1_L2);

	if( in_out == PWR_STATE_IN) 	{
		SAVE_INT_AND_CLI(flags);

#ifdef CONFIG_RTL_92D_DMDP
		Sw_PCIE_Func2(priv->pshare->wlandev_idx);

		if(!IS_DRV_OPEN(priv)) {
#ifdef ASPM_ENABLE
			if( priv->pwr_state==ASPM_L0s_L1) {
			  REG32(haddr+0x80) &= ~(0x3);
			  REG32(saddr+0x80) &= ~(0x3);
			}
#endif			
			RTL_W8(PCIE_CTRL_REG+2, 0x2);   // sw L123
			RTL_W16(PCIE_CTRL_REG, 0xff00 );
			REG32(saddr+0x44) |= 0x8108;

			if (L1_L2 == L1) {				
				priv->pwr_state = L1;				
			} else {
				RTL_W8(RSV_CTRL0, 0xe1);		// reg lock, dis_prst
				RTL_W8(RSV_CTRL0, 0xe1);
				priv->pwr_state = L2;
			}
			RESTORE_INT(flags);
			return;
		}
#endif

#ifdef ASPM_ENABLE
		if( priv->pwr_state==ASPM_L0s_L1) {
		  REG32(haddr+0x80) &= ~(0x3);
		  REG32(saddr+0x80) &= ~(0x3);
		}
#endif

//		RTL_W8(0x286,  4);
		RTL_W8(PCIE_CTRL_REG+2, 0x2);   // sw L123
		REG32(saddr+0x44) |= 0x8108; 		// clear pme status

		RTL_W16(NQOS_SEQ, priv->pshare->phw->seq);
		RTL_W8(HWSEQ_CTRL, 0x7f);			// mac seq

		ctr=3000;
		do {
			  delay_us(100);
			if(!RTL_R8(PCIE_CTRL_REG)) {
				RTL_W8(PCIE_CTRL_REG+1, 0xfe );
				break;
			}
		} while(--ctr);
		if(!ctr) {
			status = 1;
		}
		ctr=3000;
		do {
			  delay_us(100);
			if( ((RTL_R32(0x200) ^ RTL_R32(0x204))==0) && 
				(((RTL_R32(VOQ_INFO) |RTL_R32(VIQ_INFO)|RTL_R32(BEQ_INFO)|RTL_R32(BKQ_INFO)
				|RTL_R32(MGQ_INFO)|RTL_R32(HIQ_INFO))&0xffff00)==0)
				) { 				
				break;
			}
		} while(--ctr); 
		if(!ctr) {
			status = 1;
		}
		RTL_W8(TXPAUSE, 0x2f);
		delay_ms(1);

#ifdef PCIE_L2_ENABLE
		if (L1_L2 == L2) {
			int 	tx_head, tx_tail, q_num;
			struct tx_desc		*phdesc, *pdesc;
			for(q_num=MGNT_QUEUE; q_num<=HIGH_QUEUE; q_num++) {
				tx_head 	= get_txhead(GET_HW(priv), q_num);
				tx_tail 	= get_txtail(GET_HW(priv), q_num);
				phdesc		= get_txdesc(GET_HW(priv), q_num);
				while (tx_tail != tx_head) {
					pdesc	  = phdesc + (tx_tail);
					pdesc->Dword0 &= set_desc(~TX_OWN);
					tx_tail = (tx_tail + 1) % CURRENT_NUM_TX_DESC;
				}
			}
#ifdef SMP_SYNC
			if (!priv->pshare->has_triggered_tx_tasklet) {
				tasklet_schedule(&priv->pshare->tx_tasklet);
				priv->pshare->has_triggered_tx_tasklet = 1;
			}
#else
			rtl8192cd_tx_dsr((unsigned long)priv);
#endif
			rtl8192cd_rx_isr(priv);
		}
#endif
		if ((get_rf_mimo_mode(priv) == MIMO_2T2R) && (priv->pshare->rf_ft_var.power_save&_1x1_en))
			switch_to_1x1(priv,	PWR_STATE_IN);

		page = ((priv->offload_ctrl)>>7)&0xff;
		RTL_W16(RCR, RTL_R16(RCR) & ~(BIT(11)|BIT(12)|BIT(13)));
		RTL_W16(RXFLTMAP0, RTL_R16(RXFLTMAP0) |BIT(11)|BIT(4));

		if(priv->pshare->rf_ft_var.power_save&offload_en)  {

			if(GET_CHIP_VER(priv) != VERSION_8192D)
				status |= signin_h2c_cmd(priv, _RSVDPAGE_CMD_ | page<<8, 0);
			else
				status |= signin_h2c_cmd(priv, _RSVDPAGE_CMD_ | BIT(7) | (page<<8), (page<<8)| (page) );

			status |= signin_h2c_cmd(priv, _AP_OFFLOAD_CMD_ | (1<<8) |(HIDDEN_AP<<16) | ((GET_MIB(priv))->dot11OperationEntry.deny_any)<<24, 0 );
#if defined(__LINUX_2_6__)
			RTL_W32(HIMR, 0);
#endif
			DEBUG_INFO("%s, LINE:%d, h2c %x, %x, %x\n", __FUNCTION__, __LINE__,
				(_RSVDPAGE_CMD_ | page<<8), (_RSVDPAGE_CMD_ | BIT(7) | (page<<8), (page<<8)| (page)),
				(_AP_OFFLOAD_CMD_ | (1<<8) |(HIDDEN_AP<<16) | ((GET_MIB(priv))->dot11OperationEntry.deny_any)<<24)
				);
				ctr=3000;
				do {
					  delay_us(10);
					page = RTL_R8(RXPKT_NUM+2)&6 ;
					if(page==6)
						break;
				} while(--ctr);
				
			if(status || (page!=6)) {
				DEBUG_INFO("signin_h2c_cmd fail(ap offload), 286=%x\n", page);				
#if defined(__LINUX_2_6__)
			RTL_W32(HIMR, priv->pshare->InterruptMask);
#endif
				if ((get_rf_mimo_mode(priv) == MIMO_2T2R) && (priv->pshare->rf_ft_var.power_save&_1x1_en))
					switch_to_1x1(priv, PWR_STATE_OUT);
				RTL_W8(HWSEQ_CTRL, 0x0); 		// mac seq disable
				RTL_W8(RXPKT_NUM+2, 0);

				RTL_W16(RCR, RTL_R16(RCR) | (BIT(11)|BIT(13)));
				RTL_W16(RXFLTMAP0, RTL_R16(RXFLTMAP0) & ~(BIT(11)|BIT(4))); 

				RTL_W8(PCIE_CTRL_REG+1, 0x00);	// enable DMA
				RTL_W8(TXPAUSE, 0x0);

				priv->offload_ctrl =0;
				RESTORE_INT(flags);
				return;
			}
		}
		RTL_W16(CR , RTL_R16(CR) | ENSWBCN);		// enable sw beacon

		if( L1_L2 == L1){
  		
#ifdef CONFIG_RTL_92D_DMDP
			if((priv->pshare->wlandev_idx) && (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY)
				&&  IS_DRV_OPEN(((struct rtl8192cd_priv *)if_priv[0])))	{
				RESTORE_INT(flags);
				return ;
			}		  
			Sw_PCIE_Func2(1);
			tmp32 = REG32(saddr+0x44) &( ~(3));
			REG32(saddr+0x44) = tmp32|		(3);  //D3

			if(!priv->pshare->wlandev_idx)
				((struct rtl8192cd_priv *)if_priv[1])->pwr_state = L1;
			Sw_PCIE_Func2(0);
			delay_ms(1);
#endif				
			priv->pwr_state = L1;

		  tmp32= REG32(saddr+0x44) &( ~(3));  //D0
		  REG32(saddr+0x44) = tmp32|	(3);  //D3
		  //HostPCIe_SetPhyMdioWrite(0xd, 0x15a6);
		  printk("D3 hot -> L1\n");
		  delay_ms(1);
#if 0 //saving more power
		  HostPCIe_SetPhyMdioWrite(portnum, 0xf, 0x0708);
#endif
		}
		RESTORE_INT(flags);


#ifdef PCIE_L2_ENABLE
		if( L1_L2 == L2) {
#if 0 //saving more power   leave L1 write
		  HostPCIe_SetPhyMdioWrite(portnum, 0xf, 0x0f0f);
#endif
			RTL_W8(RSV_CTRL0, 0xe1);		// reg lock, dis_prst
			RTL_W8(RSV_CTRL0, 0xe1);
			priv->pwr_state = L2;
#ifdef CONFIG_RTL_92D_DMDP
			if(!priv->pshare->wlandev_idx)
#endif
			REG32(haddr+0x1008) |= (0x200);
			printk("PME turn off -> L2\n");		  
		}
#endif
#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
		REG32(0xb8003000) |= BIT(16);		// GIMR
#else
		REG32(0xb8003000) |= BIT(9);		// GIMR
#endif
	}
	else if(in_out==PWR_STATE_OUT) {
#ifdef PCIE_L2_ENABLE
		if( L1_L2 == L2){

#ifdef CONFIG_RTL_92D_DMDP
			Sw_PCIE_Func2(priv->pshare->wlandev_idx);
			if(!priv->pshare->wlandev_idx ) 			
#endif
				PCIE_reset_procedure3(priv);
			setBaseAddressRegister();

			SAVE_INT_AND_CLI(flags);
			priv->pwr_state = L0;
#ifdef CONFIG_RTL_92D_DMDP			
			((struct rtl8192cd_priv *)if_priv[1])->pwr_state = L0;
#endif			
			RTL_W8(RSV_CTRL0, 0x0);
			tmp32 = 0;
			while(1) {
				if( !(RTL_R8(SPS0_CTRL) & BIT(3)) || (++tmp32 > 20) ) {
					RTL_W8(SPS0_CTRL, 0x2b);
					break;
				}
			}
			DEBUG_INFO("SPS0_CTRL=%d !!\n", RTL_R8(SPS0_CTRL));
		} else
#endif
		{
			
			 SAVE_INT_AND_CLI(flags);			
			 if( priv->pwr_state == L1) {
			 
#ifdef CONFIG_RTL_92D_DMDP
				for(i=0; i<2; i++) {
					Sw_PCIE_Func2(i);
#endif
					ctr=3000;
#if 0 //saving more power, leave L1 write
					  HostPCIe_SetPhyMdioWrite(portnum, 0xf, 0x0f0f);
#endif
					  tmp32 = REG32(saddr+0x44) &( ~(3));  //D0
					do {
						  REG32(saddr+0x44) = tmp32|	(0);  //D0
						  delay_us(1);
						  REG32(saddr+0x44) = tmp32|	(0);  //D0
						status = REG32(haddr+0x728)&0x1f;
						if(status == 0x11)
							break;
					} while(--ctr);

					if(status != 0x11)
						panic_printk("change to L0 fail!!!, status=%x, MAC0\n", status);
					else
#ifdef CONFIG_RTL_92D_DMDP
					((struct rtl8192cd_priv *)if_priv[i])->pwr_state = L0;
				}
#else
			  priv->pwr_state = L0;
#endif
			}
		}


#ifdef CONFIG_RTL_92D_DMDP
		if((priv->pshare->wlandev_idx ==0) && (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY) && !IS_DRV_OPEN(priv) ) {
				goto OEPN_MAC1;
		}
#endif		

		if(priv->pshare->rf_ft_var.power_save&offload_en) {
			
#if defined(__LINUX_2_6__)
		RTL_W32(HIMR, priv->pshare->InterruptMask);
#endif
			signin_h2c_cmd(priv, _AP_OFFLOAD_CMD_ , 0 );		// offload
			delay_ms(2);		
		}

		if ((get_rf_mimo_mode(priv) == MIMO_2T2R) &&(priv->pshare->rf_ft_var.power_save&_1x1_en))
			switch_to_1x1(priv,	PWR_STATE_OUT);
#ifdef PCIE_L2_ENABLE
		if( L1_L2 == L2) {
			priv->pshare->phw->cur_rx =0;
#ifdef DELAY_REFILL_RX_BUF
			priv->pshare->phw->cur_rx_refill =0;
#endif
			memset(&(priv->pshare->phw->txhead0), 0, sizeof(int)*12);
			RTL_W32(BCNQ_DESA, priv->pshare->phw->tx_ringB_addr);
			RTL_W32(MGQ_DESA, priv->pshare->phw->tx_ring0_addr);
			RTL_W32(VOQ_DESA, priv->pshare->phw->tx_ring4_addr);
			RTL_W32(VIQ_DESA, priv->pshare->phw->tx_ring3_addr);
			RTL_W32(BEQ_DESA, priv->pshare->phw->tx_ring2_addr);
			RTL_W32(BKQ_DESA, priv->pshare->phw->tx_ring1_addr);
			RTL_W32(HQ_DESA, priv->pshare->phw->tx_ring5_addr);
			RTL_W32(RX_DESA, priv->pshare->phw->ring_dma_addr);
		}
#endif
		// wait until FW stop parsing packet
		ctr = 1000;
		do {
			if(!(RTL_R8(FWIMR) & FWIMR_RXDONE))
				break;
			delay_us(200);
		} while(--ctr) ;
		if(!ctr)
			DEBUG_ERR("stop offload fail\n");

		RTL_W8(HWSEQ_CTRL, 0x0); 		// mac seq disable
		RTL_W8(RXPKT_NUM+2, 0);			// RW_RELEASE_ENABLE
		RTL_W16(RCR, RTL_R16(RCR) | (BIT(11)|BIT(13)));
		RTL_W16(RXFLTMAP0, RTL_R16(RXFLTMAP0) & ~(BIT(11)|BIT(4)));	
		RTL_W8(PCIE_CTRL_REG+1, 0x00);	// enable DMA
		RTL_W8(PCIE_CTRL_REG+2, 0x3);	// sw L123		
		RTL_W8(TXPAUSE, 0x0);
		RTL_W16(CR , RTL_R16(CR) & ~ENSWBCN);

#ifdef CONFIG_RTL_92D_DMDP
		if((priv->pshare->wlandev_idx ==0) && (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY)) {
			struct rtl8192cd_priv *priv1;
OEPN_MAC1:			
			priv1 = ((struct rtl8192cd_priv *)if_priv[1]);
			if(IS_DRV_OPEN(priv1) ) {
				priv1->ps_ctrl = priv->ps_ctrl;
				PCIe_power_save_tasklet((unsigned long )priv1);
			}
		}
#endif

#if defined(RX_TASKLET)
		if(IS_DRV_OPEN(priv)) {
			tasklet_hi_schedule(&priv->pshare->rx_tasklet);
		}
#endif

#ifdef ASPM_ENABLE
		ASPM_on_off(priv);
#endif
		RESTORE_INT(flags);
	}
}


void PCIeWakeUp(struct rtl8192cd_priv *priv, unsigned int expTime)
{
	unsigned long flags;
	SAVE_INT_AND_CLI(flags);

	if( (priv->pwr_state == L1) || (priv->pwr_state == L2)) {

		if (timer_pending(&priv->ps_timer))
			del_timer_sync(&priv->ps_timer);

#ifdef CONFIG_RTL_92D_DMDP	
		if(priv->pshare->wlandev_idx ==1) {
			struct rtl8192cd_priv *priv0= ((struct rtl8192cd_priv *)if_priv[0]);
			PCIeWakeUp(priv0, expTime);
		} else 
#endif
		{
			priv->ps_ctrl = 0x02 | (priv->pwr_state<<4);
			PCIe_power_save_tasklet((unsigned long)priv);
		}
		
		mod_timer(&priv->ps_timer, jiffies + expTime);
		priv->offload_ctrl =0;
#ifdef CONFIG_RTL_92D_DMDP	
		((struct rtl8192cd_priv *)if_priv[priv->pshare->wlandev_idx^1])->offload_ctrl = 0;
#endif
	}
	RESTORE_INT(flags);
}

#ifdef __LINUX_2_6__
irqreturn_t gpio_wakeup_isr(int irq, void *dev_instance)
#else
void gpio_wakeup_isr(int irq, void *dev_instance, struct pt_regs *regs)
#endif
{
	struct net_device *dev = NULL;
	struct rtl8192cd_priv *priv = NULL;

	unsigned int saddr;

	dev = (struct net_device *)dev_instance;
	priv = (struct rtl8192cd_priv *)dev->priv;

	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		saddr = CFG_92D_SLOTS;
	} else {
		saddr = CFG_92C_SLOTS;
	}

#ifdef CONFIG_RTL_92D_DMDP
	Sw_PCIE_Func2(priv->pshare->wlandev_idx);
#endif

	DEBUG_INFO("%s, PABCD_ISR=%x\t", dev->name, REG32(PABCD_ISR));
	
#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)

	#if defined(CONFIG_RTL_ULINKER)
		REG32(PABCD_ISR) = BIT(13) ; 	// clear int status
	#else
#if defined(CONFIG_RTL_92D_SUPPORT) && defined(CONFIG_RTL_8197D)
	if(GET_CHIP_VER(priv) == VERSION_8192D)		
		REG32(PABCD_ISR) = BIT(15) ;
	else 
#endif
		REG32(PABCD_ISR) = BIT(4) ;		// clear int status
	#endif /* #if defined(CONFIG_RTL_ULINKER) */

#elif defined(CONFIG_RTL_8198)
#ifdef CONFIG_RTL_92D_SUPPORT
	if(GET_CHIP_VER(priv) == VERSION_8192D)		
		REG32(PABCD_ISR) = BIT(17) ;
	else 
#endif
		REG32(PABCD_ISR) = BIT(20) ;		// clear int status
#else
	REG32(PABCD_ISR) = BIT(15) ;			// clear int status
#endif

	DEBUG_INFO(", PABCD_ISR=%x,0xb8100044=%x\n", REG32(PABCD_ISR), REG32(saddr+0x44));

#ifdef GPIO_WAKEPIN
#ifdef PCIE_POWER_SAVING_DEBUG
	priv->firstPkt = 1;
#endif
	if (timer_pending(&priv->ps_timer))
		del_timer_sync(&priv->ps_timer);

	if( priv->pwr_state == L1 || priv->pwr_state == L2) {
		priv->ps_ctrl = 0x02 | (priv->pwr_state<<4);
		PCIe_power_save_tasklet((unsigned long)priv);
	}		
	
	priv->offload_ctrl =0;
	mod_timer(&priv->ps_timer, jiffies + (POWER_DOWN_T0));

#endif
#ifdef __LINUX_2_6__
	return IRQ_HANDLED;
#endif
}


void radio_off(struct rtl8192cd_priv *priv)
{
#if 0
	extern	void HostPCIe_Close(void);
	printk("Radio Off======>\n");
	HostPCIe_Close();
#endif
}
#endif


#ifdef EN_EFUSE
#define VOLTAGE_V25		0x03

#if defined(CONFIG_RTL_8812_SUPPORT)
#define MAX_2G_CHNLGRP	5
#define MAX_5G_CHNLGRP	14

struct channel_group {
        unsigned char low;
        unsigned char high;
};

static struct channel_group chnl_pwrlvlgp_2G[] =
{	//decrease 1 for array index
    {0, 1},
    {2, 4},
    {5, 7},
    {8, 10},
    {11, 12},
    {13}
};

static struct channel_group chnl_pwrlvlgp_5G[] =
{	//decrease 1 for array index
    {35, 39},
    {43, 47},
    {51, 55},
    {59, 63},
    {99, 103},
    {107, 111},
    {115, 119},
    {123, 127},
    {131, 135},
    {139, 143},
    {148, 152},
    {156, 160},
    {164, 168},
    {172, 176}
};

int find_2gchnlgroup(int chnl)
{
        int index=0;

        for(index=0;index<MAX_2G_CHNLGRP;index++){
            if( (chnl >= chnl_pwrlvlgp_2G[index].low)
				&& (chnl <= chnl_pwrlvlgp_2G[index].high) )
                    return index;
        }

        return -1;
}

int find_5gchnlgroup(int chnl)
{
        int index=0;

        for(index=0;index<MAX_5G_CHNLGRP;index++){
            if( (chnl >= chnl_pwrlvlgp_5G[index].low)
				&&  (chnl <= chnl_pwrlvlgp_5G[index].high) )
                    return index;
        }

        return -1;
}

int read_efusemap_2gtxpwrdiff(struct rtl8192cd_priv *priv, unsigned int chnl)
{
        unsigned char *hwinfo = &(priv->EfuseMap[EFUSE_INIT_MAP][0]);

        if(GET_CHIP_VER(priv) == VERSION_8812E) {
			priv->pmib->dot11RFEntry.pwrdiff_20BW1S_OFDM1T_A[chnl]
			    = hwinfo[EEPROM_2G_HT201S_TxPowerDiff+PATHA_OFFSET];
			priv->pmib->dot11RFEntry.pwrdiff_40BW2S_20BW2S_A[chnl]
			    = hwinfo[EEPROM_2G_HT402S_TxPowerDiff+PATHA_OFFSET];
			priv->pmib->dot11RFEntry.pwrdiff_OFDM2T_CCK2T_A[chnl]
			    = hwinfo[EEPROM_2G_OFDM2T_TxPowerDiff+PATHA_OFFSET];
			
			priv->pmib->dot11RFEntry.pwrdiff_20BW1S_OFDM1T_B[chnl]
			    = hwinfo[EEPROM_2G_HT201S_TxPowerDiff+PATHB_OFFSET];
			priv->pmib->dot11RFEntry.pwrdiff_40BW2S_20BW2S_B[chnl]
			    = hwinfo[EEPROM_2G_HT402S_TxPowerDiff+PATHB_OFFSET];
			priv->pmib->dot11RFEntry.pwrdiff_OFDM2T_CCK2T_B[chnl]
			    = hwinfo[EEPROM_2G_OFDM2T_TxPowerDiff+PATHB_OFFSET];
        }
}

int read_efusemap_5gtxpwrdiff(struct rtl8192cd_priv *priv, unsigned int chnl)
{
    unsigned char *hwinfo = &(priv->EfuseMap[EFUSE_INIT_MAP][0]);

    if(GET_CHIP_VER(priv) == VERSION_8812E) {
		priv->pmib->dot11RFEntry.pwrdiff_5G_20BW1S_OFDM1T_A[chnl]
		    = hwinfo[EEPROM_5G_HT201S_TxPowerDiff+PATHA_OFFSET];
		priv->pmib->dot11RFEntry.pwrdiff_5G_40BW2S_20BW2S_A[chnl]
		    = hwinfo[EEPROM_5G_HT402S_TxPowerDiff+PATHA_OFFSET];
		priv->pmib->dot11RFEntry.pwrdiff_5G_80BW1S_160BW1S_A[chnl]
		    = hwinfo[EEPROM_5G_HT801S_TxPowerDiff+PATHA_OFFSET];
		priv->pmib->dot11RFEntry.pwrdiff_5G_80BW2S_160BW2S_A[chnl]
		    = hwinfo[EEPROM_5G_HT802S_TxPowerDiff+PATHA_OFFSET];

		priv->pmib->dot11RFEntry.pwrdiff_5G_20BW1S_OFDM1T_B[chnl]
		    = hwinfo[EEPROM_5G_HT201S_TxPowerDiff+PATHB_OFFSET];
		priv->pmib->dot11RFEntry.pwrdiff_5G_40BW2S_20BW2S_B[chnl]
		    = hwinfo[EEPROM_5G_HT402S_TxPowerDiff+PATHB_OFFSET];
		priv->pmib->dot11RFEntry.pwrdiff_5G_80BW1S_160BW1S_B[chnl]
		    = hwinfo[EEPROM_5G_HT801S_TxPowerDiff+PATHB_OFFSET];
		priv->pmib->dot11RFEntry.pwrdiff_5G_80BW2S_160BW2S_B[chnl]
		    = hwinfo[EEPROM_5G_HT802S_TxPowerDiff+PATHB_OFFSET];
    }
}

int clear_5g_pwr_params(struct rtl8192cd_priv *priv,int index)
{
    if(GET_CHIP_VER(priv) == VERSION_8812E) {
        priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[index] = 0;
        priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[index] = 0;
        priv->pmib->dot11RFEntry.pwrdiff_5G_20BW1S_OFDM1T_A[index] = 0;
        priv->pmib->dot11RFEntry.pwrdiff_5G_40BW2S_20BW2S_A[index] = 0;
        priv->pmib->dot11RFEntry.pwrdiff_5G_80BW1S_160BW1S_A[index] = 0;
        priv->pmib->dot11RFEntry.pwrdiff_5G_80BW2S_160BW2S_A[index] = 0;

        priv->pmib->dot11RFEntry.pwrdiff_5G_20BW1S_OFDM1T_B[index] = 0;
        priv->pmib->dot11RFEntry.pwrdiff_5G_40BW2S_20BW2S_B[index] = 0;
        priv->pmib->dot11RFEntry.pwrdiff_5G_80BW1S_160BW1S_B[index] = 0;
        priv->pmib->dot11RFEntry.pwrdiff_5G_80BW2S_160BW2S_B[index] = 0;
    }

    return 0;
}
#endif

/*-----------------------------------------------------------------------------
 * Function:	efuse_PowerSwitch
 *
 * Overview:	When we want to enable write operation, we should change to
 *				pwr on state. When we stop write, we should switch to 500k mode
 *				and disable LDO 2.5V.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/17/2008 	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
static	void efuse_PowerSwitch(struct rtl8192cd_priv *priv, unsigned char bWrite, unsigned char PwrState)
{
	unsigned char tempval;
	short tmpV16;
	if (PwrState == TRUE) {
#if defined(CONFIG_RTL_8812_SUPPORT)
        if(GET_CHIP_VER(priv) == VERSION_8812E) {
                tempval = 0x69;
                RTL_W8(REG_PG_PASSWD_8812, tempval);
        }
#endif
        // 1.2V Power: From VDDON with Power Cut(0x0000h[15]), defualt valid
#if defined(CONFIG_RTL_8812_SUPPORT)
        if(GET_CHIP_VER(priv) != VERSION_8812E) {
                tmpV16 = RTL_R16(SYS_ISO_CTRL);
                if (!(tmpV16 & PWC_EV12V)) {
                        tmpV16 |= PWC_EV12V;
                        RTL_W16(SYS_ISO_CTRL, tmpV16);
                }
        }
#endif
		// Reset: 0x0000h[28], default valid
		tmpV16 = RTL_R16(REG_SYS_FUNC_EN);
		if (!(tmpV16 & FEN_ELDR)) {
			tmpV16 |= FEN_ELDR;
			RTL_W16(REG_SYS_FUNC_EN, tmpV16);
		}
		// Clock: Gated(0x0008h[5]) 8M(0x0008h[1]) clock from ANA, default valid
		tmpV16 = RTL_R16(SYS_CLKR);
		if ((!(tmpV16 & LOADER_CLK_EN)) || (!(tmpV16 & ANA8M))) {
			tmpV16 |= (LOADER_CLK_EN |ANA8M);
			RTL_W16(SYS_CLKR, tmpV16);
		}

		if (bWrite == TRUE) {
			// Enable LDO 2.5V before read/write action
			tempval = RTL_R8(EFUSE_TEST+3);
#if defined(CONFIG_RTL_8812_SUPPORT)
			if(GET_CHIP_VER(priv) == VERSION_8812E) {
					tempval &= ~(BIT3|BIT4|BIT5|BIT6);
					tempval |= (VOLTAGE_V25 << 3);
			} else
#endif
			{
				tempval &= 0x0F;
				tempval |= (VOLTAGE_V25 << 4);
				RTL_W8(EFUSE_TEST+3, (tempval | 0x80));
			}
		}
	} else {
#if defined(CONFIG_RTL_8812_SUPPORT)
        if(GET_CHIP_VER(priv) == VERSION_8812E) {
                tempval = 0x0;
                RTL_W8(REG_PG_PASSWD_8812, tempval);
        }
#endif

		if (bWrite == TRUE) {
			// Disable LDO 2.5V after read/write action
			tempval = RTL_R8(EFUSE_TEST+3);
			RTL_W8(EFUSE_TEST+3, (tempval & 0x7F));
		}
	}
}	/* efuse_PowerSwitch */

static void ReadEFuseByte(struct rtl8192cd_priv *priv, unsigned short _offset, unsigned char *pbuf)
{
	unsigned int   	value32;
	unsigned char 	readbyte;
	unsigned short 	retry;

	//Write Address
	RTL_W8(EFUSE_CTRL+1, (_offset & 0xff));
	readbyte = RTL_R8(EFUSE_CTRL+2);
	RTL_W8(EFUSE_CTRL+2, ((_offset >> 8) & 0x03) | (readbyte & 0xfc));

	//Write bit 32 0
	readbyte = RTL_R8(EFUSE_CTRL+3);
	RTL_W8(EFUSE_CTRL+3, 0x72); //read cmd	
	//RTL_W8(EFUSE_CTRL+3, (readbyte & 0x7f));

	//Check bit 32 read-ready
	retry = 0;
	value32 = RTL_R32(EFUSE_CTRL);

	while (!(((value32 >> 24) & 0xff) & 0x80) && (retry<10000)) {
		value32 = RTL_R32(EFUSE_CTRL);
		retry++;
	}

	// 20100205 Joseph: Add delay suggested by SD1 Victor.
	// This fix the problem that Efuse read error in high temperature condition.
	// Designer says that there shall be some delay after ready bit is set, or the
	// result will always stay on last data we read.
	delay_us(50);
	value32 = RTL_R32(EFUSE_CTRL);
	
	*pbuf = (unsigned char)(value32 & 0xff);
}	/* ReadEFuseByte */


//
//	Description:
//		1. Execute E-Fuse read byte operation according as map offset and
//		    save to E-Fuse table.
//		2. Refered from SD1 Richard.
//
//	Assumption:
//		1. Boot from E-Fuse and successfully auto-load.
//		2. PASSIVE_LEVEL (USB interface)
//
//	Created by Roger, 2008.10.21.
//	2008/12/12 MH 	1. Reorganize code flow and reserve bytes. and add description.
//					2. Add efuse utilization collect.
//	2008/12/22 MH	Read Efuse must check if we write section 1 data again!!! Sec1
//					write addr must be after sec5.
//
static void ReadEFuse(struct rtl8192cd_priv *priv, unsigned short _offset, int _size_byte, unsigned char *pbuf)
{
	unsigned char  	efuseTbl[EFUSE_MAP_LEN];
	unsigned char  	rtemp8[1];
	unsigned short 	eFuse_Addr = 0;
	unsigned char  	offset, wren;
	unsigned short  i, j;
	unsigned short 	eFuseWord[EFUSE_MAX_SECTION][EFUSE_MAX_WORD_UNIT];
	unsigned short	efuse_utilized = 0;

	//
	// Do NOT excess total size of EFuse table. Added by Roger, 2008.11.10.
	//
	if ((_offset + _size_byte) > EFUSE_MAP_LEN)
	{// total E-Fuse table is 128bytes
		DEBUG_INFO("ReadEFuse(): Invalid offset(%#x) with read bytes(%#x)!!\n", _offset, _size_byte);
		return;
	}

	// 0. Refresh efuse init map as all oxFF.
	for (i = 0; i < EFUSE_MAX_SECTION; i++)
		for (j = 0; j < EFUSE_MAX_WORD_UNIT; j++)
			eFuseWord[i][j] = 0xFFFF;

	//
	// 1. Read the first byte to check if efuse is empty!!!
	//
	//
	ReadEFuseByte(priv, eFuse_Addr, rtemp8);
	if (*rtemp8 != 0xFF) {
		efuse_utilized++;
		DEBUG_INFO("ReadEFuse, Addr=%x, %02x\n", eFuse_Addr, *rtemp8);
		eFuse_Addr++;
	} else {
		//RTPRINT(FEEPROM, EFUSE_READ_ALL, ("EFUSE is empty efuse_Addr-%d efuse_data=%x\n", eFuse_Addr, *rtemp8));
		return;
	}

	//
	// 2. Read real efuse content. Filter PG header and every section data.
	//
	while ((*rtemp8 != 0xFF) && (eFuse_Addr < EFUSE_REAL_CONTENT_LEN))
	{

#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT)
		// Check PG header for section num.
		if (((GET_CHIP_VER(priv) == VERSION_8192D)||(GET_CHIP_VER(priv) == VERSION_8812E)) && (*rtemp8 & 0x1F) == 0x0F)		//extended header
		{
			unsigned char u1temp = ((*rtemp8 & 0xE0) >> 5);
			ReadEFuseByte(priv, eFuse_Addr, rtemp8);
			if ((*rtemp8 & 0x0F) == 0x0F) {
				eFuse_Addr++;
				ReadEFuseByte(priv, eFuse_Addr, rtemp8);
				if (*rtemp8 != 0xFF && (eFuse_Addr < EFUSE_REAL_CONTENT_LEN)) {
					eFuse_Addr++;
				}
				continue;
			} else {
				offset = ((*rtemp8 & 0xF0) >> 1) | u1temp;
				wren = (*rtemp8 & 0x0F);
				eFuse_Addr++;
			}
		}
		else
#endif
		{
			offset = ((*rtemp8 >> 4) & 0x0f);
			wren = (*rtemp8 & 0x0f);
		}

		if (offset < EFUSE_MAX_SECTION) {
			for (i=0; i<EFUSE_MAX_WORD_UNIT; i++) {
				// Check word enable condition in the section
				if (!(wren & 0x01)) {
					ReadEFuseByte(priv, eFuse_Addr, rtemp8);
					DEBUG_INFO("ReadEFuse, Addr=%x, %02x\n", eFuse_Addr, *rtemp8);
					eFuse_Addr++;
					efuse_utilized++;
					eFuseWord[offset][i] = (*rtemp8 & 0xff);
					if (eFuse_Addr >= EFUSE_REAL_CONTENT_LEN)
						break;
					ReadEFuseByte(priv, eFuse_Addr, rtemp8);
					DEBUG_INFO("ReadEFuse, Addr=%x, %02x\n", eFuse_Addr, *rtemp8);
					eFuse_Addr++;
					efuse_utilized++;
					eFuseWord[offset][i] |= (((UINT16)*rtemp8 << 8) & 0xff00);
					if (eFuse_Addr >= EFUSE_REAL_CONTENT_LEN)
						break;
				}
				wren >>= 1;
			}
		}

		// Read next PG header
		ReadEFuseByte(priv, eFuse_Addr, rtemp8);
		if (*rtemp8 != 0xFF && (eFuse_Addr < EFUSE_REAL_CONTENT_LEN)) {
			efuse_utilized++;
			DEBUG_INFO("ReadEFuse, Addr=%x, %02x\n", eFuse_Addr, *rtemp8);
			eFuse_Addr++;
		}
	}

	//
	// 3. Collect 16 sections and 4 word unit into Efuse map.
	//
	for (i=0; i<EFUSE_MAX_SECTION; i++) {
		for (j=0; j<EFUSE_MAX_WORD_UNIT; j++) {
			efuseTbl[(i<<3)+(j<<1)] = (eFuseWord[i][j] & 0xff);
			efuseTbl[(i<<3)+(j<<1)+1] = ((eFuseWord[i][j] >> 8) & 0xff);
		}
	}

	//
	// 4. Copy from Efuse map to output pointer memory!!!
	//
	for (i=0; i<_size_byte; i++) {
		pbuf[i] = efuseTbl[_offset+i];
	}

	//
	// 5. Calculate Efuse utilization.
	//
	//efuse_usage = (unsigned char)((efuse_utilized*100)/EFUSE_REAL_CONTENT_LEN);
	priv->EfuseUsedBytes = efuse_utilized;

}


/*-----------------------------------------------------------------------------
 * Function:	efuse_ReadAllMap
 *
 * Overview:	Read All Efuse content
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/11/2008 	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
static void efuse_ReadAllMap(struct rtl8192cd_priv *priv, unsigned char *Efuse)
{
	//
	// We must enable clock and LDO 2.5V otherwise, read all map will be fail!!!!
	//
	efuse_PowerSwitch(priv, FALSE, TRUE);
	ReadEFuse(priv, 0, EFUSE_MAP_LEN, Efuse);
	efuse_PowerSwitch(priv, FALSE, FALSE);
}


/*-----------------------------------------------------------------------------
 * Function:	EFUSE_ShadowMapUpdate
 *
 * Overview:	Transfer current EFUSE content to shadow init and modify map.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/13/2008 	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
static void EFUSE_ShadowMapUpdate(struct rtl8192cd_priv *priv)
{
/*
	if (priv->AutoloadFailFlag == TRUE )	{
		DEBUG_INFO("AutoloadFailFlag=TRUE");
		memset((&priv->EfuseMap[EFUSE_INIT_MAP][0]), 0xFF, 128);
	} else
*/
	{
		efuse_ReadAllMap(priv, &priv->EfuseMap[EFUSE_INIT_MAP][0]);
	}

	memcpy(&priv->EfuseMap[EFUSE_MODIFY_MAP][0],
	       &priv->EfuseMap[EFUSE_INIT_MAP][0], EFUSE_MAP_LEN);

}


static int isPGValueValid(struct rtl8192cd_priv *priv, unsigned char *hwinfo)
{
	int j;
#ifdef CONFIG_RTL_92C_SUPPORT
	if ((GET_CHIP_VER(priv) == VERSION_8188C) || (GET_CHIP_VER(priv) == VERSION_8192C)) {
		for (j=EEPROM_TxPowerCCK; j<EEPROM_TxPowerCCK+3; j++) {
			if (hwinfo[j] > 63)
				return 0;
		}
		for (j=EEPROM_TxPowerHT40_1S; j<EEPROM_TxPowerHT40_1S+3; j++) {
			if (hwinfo[j] > 63)
				return 0;
		}
	}
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D){
		for (j=EEPROM_2G_TxPowerCCK; j<EEPROM_2G_TxPowerCCK+3; j++) {
			if (hwinfo[j] > 63)
				return 0;
		}
		for (j=EEPROM_2G_TxPowerHT40_1S; j<EEPROM_2G_TxPowerHT40_1S+3; j++) {
			if (hwinfo[j] > 63)
				return 0;
		}
		for (j=EEPROM_5GL_TxPowerHT40_1S; j<EEPROM_5GL_TxPowerHT40_1S+3; j++) {
			if (hwinfo[j] > 63)
				return 0;
		}
		for (j=EEPROM_5GM_TxPowerHT40_1S; j<EEPROM_5GM_TxPowerHT40_1S+3; j++) {
			if (hwinfo[j] > 63)
				return 0;
		}
		for (j=EEPROM_5GH_TxPowerHT40_1S; j<EEPROM_5GH_TxPowerHT40_1S+3; j++) {
			if (hwinfo[j] > 63)
				return 0;
		}
	}
#endif
	return 1;
}


void ReadTxPowerInfoFromHWPG(struct rtl8192cd_priv *priv)
{
	unsigned char *hwinfo = &(priv->EfuseMap[EFUSE_INIT_MAP][0]);
	int i;

	if (!isPGValueValid(priv, hwinfo))
		return;

	if (/*priv->AutoloadFailFlag==FALSE &&*/ priv->pmib->efuseEntry.enable_efuse==1) {
		u8 TxPwrCCK=0, TxPwrHT40_1S=0, TxPwrHT40_2SDiff=0, TxPwrHT20Diff=0, TxPwrOFDMDiff=0;
#ifdef CONFIG_RTL_92C_SUPPORT
		if ((GET_CHIP_VER(priv) == VERSION_8188C) || (GET_CHIP_VER(priv) == VERSION_8192C)) {
			TxPwrCCK = EEPROM_TxPowerCCK;
			TxPwrHT40_1S = EEPROM_TxPowerHT40_1S;
			TxPwrHT40_2SDiff = EEPROM_TxPowerHT40_2SDiff;
			TxPwrHT20Diff = EEPROM_TxPowerHT20Diff;
			TxPwrOFDMDiff = EEPROM_TxPowerOFDMDiff;
		}
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8192D){
			// 2.4G Setting
			TxPwrCCK = EEPROM_2G_TxPowerCCK;
			TxPwrHT40_1S = EEPROM_2G_TxPowerHT40_1S;
			TxPwrHT40_2SDiff = EEPROM_2G_TxPowerHT40_2SDiff;
			TxPwrHT20Diff = EEPROM_2G_TxPowerHT20Diff;
			TxPwrOFDMDiff = EEPROM_2G_TxPowerOFDMDiff;

			// 5G Setting
			for (i=0; i<MAX_5G_CHANNEL_NUM; i++) {
				if (i >= 35 && i <= 63) { // ch 36 ~ 64
					if (i >= 35 && i <= 43) { // ch 36 ~ 44
						priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = hwinfo[EEPROM_5GL_TxPowerHT40_1S];
						priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = hwinfo[EEPROM_5GL_TxPowerHT40_1S+3];

						priv->pmib->dot11RFEntry.pwrdiff5GHT40_2S[i]= hwinfo[EEPROM_5GL_TxPowerHT40_2SDiff];
						priv->pmib->dot11RFEntry.pwrdiff5GHT20[i] = hwinfo[EEPROM_5GL_TxPowerHT20Diff];
						priv->pmib->dot11RFEntry.pwrdiff5GOFDM[i] = hwinfo[EEPROM_5GL_TxPowerOFDMDiff];
					} else if (i >= 45 && i <= 53) { // ch 46 ~ 54
						priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = hwinfo[EEPROM_5GL_TxPowerHT40_1S+1];
						priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = hwinfo[EEPROM_5GL_TxPowerHT40_1S+4];

						priv->pmib->dot11RFEntry.pwrdiff5GHT40_2S[i]= hwinfo[EEPROM_5GL_TxPowerHT40_2SDiff+1];
						priv->pmib->dot11RFEntry.pwrdiff5GHT20[i] = hwinfo[EEPROM_5GL_TxPowerHT20Diff+1];
						priv->pmib->dot11RFEntry.pwrdiff5GOFDM[i] = hwinfo[EEPROM_5GL_TxPowerOFDMDiff+1];
					} else if (i >= 55 && i <= 63) { // ch 56 ~ 64
						priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = hwinfo[EEPROM_5GL_TxPowerHT40_1S+2];
						priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = hwinfo[EEPROM_5GL_TxPowerHT40_1S+5];

						priv->pmib->dot11RFEntry.pwrdiff5GHT40_2S[i]= hwinfo[EEPROM_5GL_TxPowerHT40_2SDiff+2];
						priv->pmib->dot11RFEntry.pwrdiff5GHT20[i] = hwinfo[EEPROM_5GL_TxPowerHT20Diff+2];
						priv->pmib->dot11RFEntry.pwrdiff5GOFDM[i] = hwinfo[EEPROM_5GL_TxPowerOFDMDiff+2];
					} else {
						priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = 0;
						priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = 0;

						priv->pmib->dot11RFEntry.pwrdiff5GHT40_2S[i]= 0;
						priv->pmib->dot11RFEntry.pwrdiff5GHT20[i] = 0;
						priv->pmib->dot11RFEntry.pwrdiff5GOFDM[i] = 0;
					}
				} else if (i >= 99 && i <= 139) { // ch 100 ~ 140
					if (i >= 99 && i <= 111) { // ch 100 ~ 112
						priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = hwinfo[EEPROM_5GM_TxPowerHT40_1S];
						priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = hwinfo[EEPROM_5GM_TxPowerHT40_1S+3];

						priv->pmib->dot11RFEntry.pwrdiff5GHT40_2S[i]= hwinfo[EEPROM_5GM_TxPowerHT40_2SDiff];
						priv->pmib->dot11RFEntry.pwrdiff5GHT20[i] = hwinfo[EEPROM_5GM_TxPowerHT20Diff];
						priv->pmib->dot11RFEntry.pwrdiff5GOFDM[i] = hwinfo[EEPROM_5GM_TxPowerOFDMDiff];
					} else if (i >= 113 && i <= 125) { // ch 114 ~ 126
						priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = hwinfo[EEPROM_5GM_TxPowerHT40_1S+1];
						priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = hwinfo[EEPROM_5GM_TxPowerHT40_1S+4];

						priv->pmib->dot11RFEntry.pwrdiff5GHT40_2S[i]= hwinfo[EEPROM_5GM_TxPowerHT40_2SDiff+1];
						priv->pmib->dot11RFEntry.pwrdiff5GHT20[i] = hwinfo[EEPROM_5GM_TxPowerHT20Diff+1];
						priv->pmib->dot11RFEntry.pwrdiff5GOFDM[i] = hwinfo[EEPROM_5GM_TxPowerOFDMDiff+1];
					} else if (i >= 127 && i <= 139) { // ch 128 ~ 140
						priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = hwinfo[EEPROM_5GM_TxPowerHT40_1S+2];
						priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = hwinfo[EEPROM_5GM_TxPowerHT40_1S+5];

						priv->pmib->dot11RFEntry.pwrdiff5GHT40_2S[i]= hwinfo[EEPROM_5GM_TxPowerHT40_2SDiff+2];
						priv->pmib->dot11RFEntry.pwrdiff5GHT20[i] = hwinfo[EEPROM_5GM_TxPowerHT20Diff+2];
						priv->pmib->dot11RFEntry.pwrdiff5GOFDM[i] = hwinfo[EEPROM_5GM_TxPowerOFDMDiff+2];
					} else {
						priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = 0;
						priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = 0;

						priv->pmib->dot11RFEntry.pwrdiff5GHT40_2S[i]= 0;
						priv->pmib->dot11RFEntry.pwrdiff5GHT20[i] = 0;
						priv->pmib->dot11RFEntry.pwrdiff5GOFDM[i] = 0;
					}
				} else if (i >= 148 && i <= 164 ) { // ch 149 ~ 165
					if (i >= 148 && i <= 152) { // ch 149 ~ 153
						priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = hwinfo[EEPROM_5GH_TxPowerHT40_1S];
						priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = hwinfo[EEPROM_5GH_TxPowerHT40_1S+3];

						priv->pmib->dot11RFEntry.pwrdiff5GHT40_2S[i]= hwinfo[EEPROM_5GH_TxPowerHT40_2SDiff];
						priv->pmib->dot11RFEntry.pwrdiff5GHT20[i] = hwinfo[EEPROM_5GH_TxPowerHT20Diff];
						priv->pmib->dot11RFEntry.pwrdiff5GOFDM[i] = hwinfo[EEPROM_5GH_TxPowerOFDMDiff];
					} else if (i >= 154 && i <= 158) { // ch 155 ~ 159
						priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = hwinfo[EEPROM_5GH_TxPowerHT40_1S+1];
						priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = hwinfo[EEPROM_5GH_TxPowerHT40_1S+4];

						priv->pmib->dot11RFEntry.pwrdiff5GHT40_2S[i]= hwinfo[EEPROM_5GH_TxPowerHT40_2SDiff+1];
						priv->pmib->dot11RFEntry.pwrdiff5GHT20[i] = hwinfo[EEPROM_5GH_TxPowerHT20Diff+1];
						priv->pmib->dot11RFEntry.pwrdiff5GOFDM[i] = hwinfo[EEPROM_5GH_TxPowerOFDMDiff+1];
					} else if (i >= 160 && i <= 164) { // ch 161 ~ 165
						priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = hwinfo[EEPROM_5GH_TxPowerHT40_1S+2];
						priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = hwinfo[EEPROM_5GH_TxPowerHT40_1S+5];

						priv->pmib->dot11RFEntry.pwrdiff5GHT40_2S[i]= hwinfo[EEPROM_5GH_TxPowerHT40_2SDiff+2];
						priv->pmib->dot11RFEntry.pwrdiff5GHT20[i] = hwinfo[EEPROM_5GH_TxPowerHT20Diff+2];
						priv->pmib->dot11RFEntry.pwrdiff5GOFDM[i] = hwinfo[EEPROM_5GH_TxPowerOFDMDiff+2];
					} else {
						priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = 0;
						priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = 0;

						priv->pmib->dot11RFEntry.pwrdiff5GHT40_2S[i]= 0;
						priv->pmib->dot11RFEntry.pwrdiff5GHT20[i] = 0;
						priv->pmib->dot11RFEntry.pwrdiff5GOFDM[i] = 0;
					}
				}
			}
		}
#endif

#if defined(CONFIG_RTL_8812_SUPPORT)
		if ( GET_CHIP_VER(priv)==VERSION_8812E ) {
			for (i=0; i<MAX_2G_CHANNEL_NUM; i++) {
				int ch_gp = -1;
				ch_gp = find_2gchnlgroup(i);

				if(ch_gp > -1) {
					priv->pmib->dot11RFEntry.pwrlevelCCK_A[i]= hwinfo[EEPROM_2G_CCK1T_TxPower+PATHA_OFFSET+ch_gp];
					priv->pmib->dot11RFEntry.pwrlevelCCK_B[i]= hwinfo[EEPROM_2G_CCK1T_TxPower+PATHB_OFFSET+ch_gp];
					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] = hwinfo[EEPROM_2G_HT401S_TxPower+PATHA_OFFSET+ch_gp];
					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] = hwinfo[EEPROM_2G_HT401S_TxPower+PATHB_OFFSET+ch_gp];

					read_efusemap_2gtxpwrdiff(priv,i);
				}
			}

			for (i=0; i<MAX_5G_CHANNEL_NUM; i++) {
				int ch_gp = -1;
				ch_gp = find_5gchnlgroup(i);

				if(ch_gp > -1) {
					priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = hwinfo[EEPROM_5G_HT401S_TxPower+PATHA_OFFSET+ch_gp];
					priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = hwinfo[EEPROM_5G_HT401S_TxPower+PATHB_OFFSET+ch_gp];

					read_efusemap_5gtxpwrdiff(priv,i);
				} else {
					clear_5g_pwr_params(priv,i);
				}
			}
		} else
#endif //#if defined(CONFIG_RTL_8812_SUPPORT)
		{

			for (i=0; i<MAX_2G_CHANNEL_NUM; i++) {
				if (i < 3) {
					priv->pmib->dot11RFEntry.pwrlevelCCK_A[i]= hwinfo[TxPwrCCK];
					priv->pmib->dot11RFEntry.pwrlevelCCK_B[i]= hwinfo[TxPwrCCK+3];

					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] = hwinfo[TxPwrHT40_1S];
					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] = hwinfo[TxPwrHT40_1S+3];

					priv->pmib->dot11RFEntry.pwrdiffHT40_2S[i]= hwinfo[TxPwrHT40_2SDiff];
					priv->pmib->dot11RFEntry.pwrdiffHT20[i] = hwinfo[TxPwrHT20Diff];
					priv->pmib->dot11RFEntry.pwrdiffOFDM[i] = hwinfo[TxPwrOFDMDiff];

				} else if (i < 9) {
					priv->pmib->dot11RFEntry.pwrlevelCCK_A[i]= hwinfo[TxPwrCCK+1];
					priv->pmib->dot11RFEntry.pwrlevelCCK_B[i]= hwinfo[TxPwrCCK+4];

					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] = hwinfo[TxPwrHT40_1S+1];
					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] = hwinfo[TxPwrHT40_1S+4];

					priv->pmib->dot11RFEntry.pwrdiffHT40_2S[i]= hwinfo[TxPwrHT40_2SDiff+1];
					priv->pmib->dot11RFEntry.pwrdiffHT20[i] = hwinfo[TxPwrHT20Diff+1];
					priv->pmib->dot11RFEntry.pwrdiffOFDM[i] = hwinfo[TxPwrOFDMDiff+1];
				} else {
					priv->pmib->dot11RFEntry.pwrlevelCCK_A[i]= hwinfo[TxPwrCCK+2];
					priv->pmib->dot11RFEntry.pwrlevelCCK_B[i]= hwinfo[TxPwrCCK+5];

					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] = hwinfo[TxPwrHT40_1S+2];
					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] = hwinfo[TxPwrHT40_1S+5];

					priv->pmib->dot11RFEntry.pwrdiffHT40_2S[i]= hwinfo[TxPwrHT40_2SDiff+2];
					priv->pmib->dot11RFEntry.pwrdiffHT20[i] = hwinfo[TxPwrHT20Diff+2];
					priv->pmib->dot11RFEntry.pwrdiffOFDM[i] = hwinfo[TxPwrOFDMDiff+2];

				}
			}
		}
		DEBUG_INFO("EFUSE Autoload success!\n");
	}
}


static void ReadMacAddressFromEfuse(struct rtl8192cd_priv *priv)
{
	u8 efuse_MAC=0;
#ifdef __KERNEL__
	struct sockaddr addr;
#endif

#ifdef CONFIG_RTL_92C_SUPPORT
	if ((GET_CHIP_VER(priv) == VERSION_8188C) || (GET_CHIP_VER(priv) == VERSION_8192C)) {
		efuse_MAC = EEPROM_MACADDRESS;
	}
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
#ifdef CONFIG_RTL_92D_DMDP
		if (priv->pshare->wlandev_idx == 0) {
			efuse_MAC = EEPROM_MAC0_MACADDRESS;
		} else {
			efuse_MAC = EEPROM_MAC1_MACADDRESS;
		}
#else
		efuse_MAC = EEPROM_MAC0_MACADDRESS;
#endif
	}
#endif

#ifdef CONFIG_RTL_8812_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8812E)
		efuse_MAC = EEPROM_8812_MACADDRESS;
#endif

	if (/*priv->AutoloadFailFlag==FALSE &&*/ priv->pmib->efuseEntry.enable_efuse == 1) {
		unsigned char *hwinfo = &(priv->EfuseMap[EFUSE_INIT_MAP][0]);
		unsigned char *efuse_mac = hwinfo + efuse_MAC;
		unsigned char zero[] = {0, 0, 0, 0, 0, 0}, mac[6];

		memcpy(mac, efuse_mac, MACADDRLEN);
		/* printk("wlan%d EFUSE MAC [%02x:%02x:%02x:%02x:%02x:%02x]\n", priv->pshare->wlandev_idx,
				*mac, *(mac+1), *(mac+2), *(mac+3), *(mac+4), *(mac+5)); */
#if 0
		if(memcmp(mac, zero, MACADDRLEN) && !IS_MCAST(mac)) {
#ifdef __KERNEL__
			memcpy(addr.sa_data, mac, MACADDRLEN);
			rtl8192cd_set_hwaddr(priv->dev, (void *)&addr);
#else
			rtl8192cd_set_hwaddr(priv->dev, (void *)mac);
#endif
		}
#else
		if(!memcmp(mac, zero, MACADDRLEN) || IS_MCAST(mac)) {
			memcpy(mac, zero, MACADDRLEN);
		}

#ifdef __KERNEL__
		memcpy(addr.sa_data, mac, MACADDRLEN);
		rtl8192cd_set_hwaddr(priv->dev, (void *)&addr);
#else
		rtl8192cd_set_hwaddr(priv->dev, (void *)mac);
#endif
#endif
	}
}


void ReadThermalMeterFromEfuse(struct rtl8192cd_priv *priv)
{
	unsigned char *hwinfo = &(priv->EfuseMap[EFUSE_INIT_MAP][0]);
	u8 efuse_Ther = 0, TherMask=0x1f;

	if (!priv->pmib->efuseEntry.enable_efuse)
		return;

#ifdef CONFIG_RTL_92C_SUPPORT
	if ((GET_CHIP_VER(priv) == VERSION_8188C) || (GET_CHIP_VER(priv) == VERSION_8192C)) {
		efuse_Ther = EEPROM_THERMAL_METER;
	}
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		efuse_Ther = EEPROM_92D_THERMAL_METER;
	}
#endif

#ifdef CONFIG_RTL_8812_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8812E) {
		efuse_Ther = EEPROM_8812_THERMAL_METER;
		TherMask = 0xff;
	}
#endif

	priv->pmib->dot11RFEntry.ther = (hwinfo[efuse_Ther] & TherMask);
	DEBUG_INFO("ThermalMeter = 0x%x\n", priv->pmib->dot11RFEntry.ther);

#ifdef CONFIG_RTL_88E_SUPPORT //for 88e tx power tracking
	if(GET_CHIP_VER(priv) != VERSION_8812E)
#endif
	{
#ifdef CONFIG_RTL_88E_SUPPORT //for 88e tx power tracking
		if(GET_CHIP_VER(priv) == VERSION_8188E){
			if ((priv->pmib->dot11RFEntry.ther < 0x07) || (priv->pmib->dot11RFEntry.ther > 0x32)) {
				DEBUG_ERR("TPT: unreasonable target ther %d, disable tpt\n", priv->pmib->dot11RFEntry.ther);
				priv->pmib->dot11RFEntry.ther = 0;
			}
		}
		else
#endif
		if ((priv->pmib->dot11RFEntry.ther < 0x07) || (priv->pmib->dot11RFEntry.ther > 0x1d)) {
			priv->pmib->dot11RFEntry.ther = 0;
		}
	}
}


#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT)
void ReadCrystalCalibrationFromEfuse(struct rtl8192cd_priv *priv)
{
	unsigned char *hwinfo = &(priv->EfuseMap[EFUSE_INIT_MAP][0]);

	if (!priv->pmib->efuseEntry.enable_efuse)
		return;

	if(GET_CHIP_VER(priv) == VERSION_8812E) {
		if(hwinfo[EEPROM_8812_XTAL_K] == 0xff)
			priv->pmib->dot11RFEntry.xcap = 0x0;
		else
			priv->pmib->dot11RFEntry.xcap = hwinfo[EEPROM_8812_XTAL_K]&0x3f;	//[5:0]
	}
#if defined(CONFIG_RTL_92D_SUPPORT)
	else
		priv->pmib->dot11RFEntry.xcap = (hwinfo[EEPROM_92D_XTAL_K]);	//[7:0]
#endif
}
#endif

#if defined(CONFIG_RTL_92D_SUPPORT)
void ReadDeltaValFromEfuse(struct rtl8192cd_priv *priv)
{
	unsigned char *hwinfo = &(priv->EfuseMap[EFUSE_INIT_MAP][0]);

	if (!priv->pmib->efuseEntry.enable_efuse)
		return;

	priv->pmib->dot11RFEntry.deltaIQK = (hwinfo[EEPROM_92D_IQK_DELTA] & 0x03);		//[1:0]
	priv->pmib->dot11RFEntry.deltaLCK = (hwinfo[EEPROM_92D_LCK_DELTA] & 0x0C) >> 2;	//[3:2]
}

void ReadTRSWPAPEFromEfuse(struct rtl8192cd_priv *priv)
{
	unsigned char *hwinfo = &(priv->EfuseMap[EFUSE_INIT_MAP][0]);

	if (!priv->pmib->efuseEntry.enable_efuse)
		return;

	priv->pmib->dot11RFEntry.trsw_pape_C9 = (hwinfo[EEPROM_92D_TRSW_CTRL] & 0xff);
	priv->pmib->dot11RFEntry.trsw_pape_CC = (hwinfo[EEPROM_92D_PAPE_CTRL] & 0xff);

	if (priv->pmib->dot11RFEntry.trsw_pape_C9 == 0xff)
		priv->pmib->dot11RFEntry.trsw_pape_C9 = 0;
}

#endif


//
//	Description:
//		Read HW adapter information by E-Fuse or EEPROM according CR9346 reported.
//
//	Assumption:
//		1. CR9346 regiser has verified.
//		2. PASSIVE_LEVEL (USB interface)
//
//	Created by Roger, 2008.10.21.
//
int ReadAdapterInfo8192CE(struct rtl8192cd_priv *priv)
{
	unsigned char			tmpU1b;
	tmpU1b = RTL_R8(CR9346);

	// To check system boot selection.
	if (tmpU1b & CmdEERPOMSEL)	{
		DEBUG_INFO("Boot from EEPROM\n");
	} else	{
		DEBUG_INFO("Boot from EFUSE\n");
	}

	// To check autoload success or not.
	if (tmpU1b & CmdEEPROM_En)	{
		DEBUG_INFO("Autoload OK!!\n");
		priv->AutoloadFailFlag=FALSE;
#if 0
		EFUSE_ShadowMapUpdate(priv);
		ReadTxPowerInfoFromHWPG(priv);
		ReadMacAddressFromEfuse(priv);
#endif
	} else { // Auto load fail.
		DEBUG_INFO("AutoLoad Fail reported from CR9346!!\n");
		priv->AutoloadFailFlag=TRUE;
	}
	EFUSE_ShadowMapUpdate(priv);
	ReadTxPowerInfoFromHWPG(priv);
	ReadThermalMeterFromEfuse(priv);
	ReadMacAddressFromEfuse(priv);
#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8192D){
		ReadCrystalCalibrationFromEfuse(priv);
		ReadDeltaValFromEfuse(priv);
		ReadTRSWPAPEFromEfuse(priv);
	}
#endif

#if defined(CONFIG_RTL_8812_SUPPORT)
	if (GET_CHIP_VER(priv)==VERSION_8812E)
		ReadCrystalCalibrationFromEfuse(priv);
#endif

	return 0;
}


static char FLASH_NAME_PARAM[][32] = {
	"HW_TX_POWER_CCK_A",
	"HW_TX_POWER_CCK_B",
	"HW_TX_POWER_HT40_1S_A",
	"HW_TX_POWER_HT40_1S_B",
	"HW_TX_POWER_DIFF_HT40_2S",
	"HW_TX_POWER_DIFF_HT20",
	"HW_TX_POWER_DIFF_OFDM",
	"HW_WLAN0_WLAN_ADDR",
	"EFUSE_MAP0",
	"EFUSE_MAP1",
	"HW_11N_THER",
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT)
	"HW_TX_POWER_5G_HT40_1S_A",
	"HW_TX_POWER_5G_HT40_1S_B",
	"HW_TX_POWER_DIFF_5G_HT40_2S",
	"HW_TX_POWER_DIFF_5G_HT20",
	"HW_TX_POWER_DIFF_5G_OFDM",
	"HW_11N_TRSWPAPE_C9",
	"HW_11N_TRSWPAPE_CC",
	"HW_11N_XCAP"	
#endif
};

#define EFUSECMD_NUM_92C 11
#define EFUSECMD_NUM_92D 19


static int getEEPROMOffset(struct rtl8192cd_priv *priv, int type)
{
	int offset=0;

#ifdef CONFIG_RTL_92C_SUPPORT
	if ((GET_CHIP_VER(priv) == VERSION_8188C) || (GET_CHIP_VER(priv) == VERSION_8192C)) {
		switch(type) {
		case 0:	offset = EEPROM_TxPowerCCK;
				break;
		case 1: offset = EEPROM_TxPowerCCK+3;
				break;
		case 2: offset = EEPROM_TxPowerHT40_1S;
				break;
		case 3: offset = EEPROM_TxPowerHT40_1S+3;
				break;
		case 4: offset = EEPROM_TxPowerHT40_2SDiff;
				break;
		case 5: offset = EEPROM_TxPowerHT20Diff;
				break;
		case 6: offset = EEPROM_TxPowerOFDMDiff;
				break;
		case 7:	offset = EEPROM_MACADDRESS;
				break;
		case 10: offset = EEPROM_THERMAL_METER;
				break;
		default:
				offset = -1;
				panic_printk("NOT SUPPORT!!\n");
				break;
		}
	}
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		switch(type) {
		case 0:	offset = EEPROM_2G_TxPowerCCK;
				break;
		case 1: offset = EEPROM_2G_TxPowerCCK+3;
				break;
		case 2: offset = EEPROM_2G_TxPowerHT40_1S;
				break;
		case 3: offset = EEPROM_2G_TxPowerHT40_1S+3;
				break;
		case 4: offset = EEPROM_2G_TxPowerHT40_2SDiff;
				break;
		case 5: offset = EEPROM_2G_TxPowerHT20Diff;
				break;
		case 6: offset = EEPROM_2G_TxPowerOFDMDiff;
				break;
		case 7:
#ifdef CONFIG_RTL_92D_DMDP
				if (priv->pshare->wlandev_idx == 1)
					offset = EEPROM_MAC1_MACADDRESS;
				else
#endif
				offset = EEPROM_MAC0_MACADDRESS;
				break;
		case 8: offset = 0x00;
				break;
		case 9: offset = 0x32;
				break;
		case 10: offset = EEPROM_92D_THERMAL_METER;
				break;
		case 11: offset = EEPROM_5GL_TxPowerHT40_1S;
				break;
		case 12: offset = EEPROM_5GL_TxPowerHT40_1S+3;
				break;
		case 13: offset = EEPROM_5GL_TxPowerHT40_2SDiff;
				break;
		case 14: offset = EEPROM_5GL_TxPowerHT20Diff;
				break;
		case 15: offset = EEPROM_5GL_TxPowerOFDMDiff;
				break;
		case 16: offset = EEPROM_92D_TRSW_CTRL;
				break;
		case 17: offset = EEPROM_92D_PAPE_CTRL;
				break;
		case 18: offset = EEPROM_92D_XTAL_K;
				break;
		default:
				offset = -1;
				panic_printk("NOT SUPPORT!!\n");
				break;
		}
	}
#endif

#ifdef CONFIG_RTL_8812_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8812E) {
			switch(type) {
			case 0: offset = EEPROM_2G_CCK1T_TxPower+PATHA_OFFSET;
					break;
			case 1: offset = EEPROM_2G_CCK1T_TxPower+PATHB_OFFSET;
					break;
			case 2: offset = EEPROM_2G_HT401S_TxPower+PATHA_OFFSET;
					break;
			case 3: offset = EEPROM_2G_HT401S_TxPower+PATHA_OFFSET;
					break;
			case 4: offset = EEPROM_2G_HT402S_TxPowerDiff+PATHA_OFFSET;
					break;
			case 5: offset = EEPROM_2G_HT201S_TxPowerDiff+PATHA_OFFSET;
					break;
			case 6: offset = EEPROM_2G_OFDM1T_TxPowerDiff+PATHA_OFFSET;
					break;
			case 7: offset = EEPROM_8812_MACADDRESS;
					break;
			case 8: offset = 0x00;
					break;
			case 9: offset = 0x32;
					break;
			case 10: offset = EEPROM_8812_THERMAL_METER;
					break;
			case 11: offset = EEPROM_5G_HT401S_TxPower+PATHA_OFFSET;
					break;
			case 12: offset = EEPROM_5G_HT401S_TxPower+PATHB_OFFSET;
					break;
			case 13: offset = EEPROM_5G_HT402S_TxPowerDiff+PATHA_OFFSET;
					break;
			case 14: offset = EEPROM_5G_HT201S_TxPowerDiff+PATHA_OFFSET;
					break;
			case 15: offset = EEPROM_5G_OFDM1T_TxPowerDiff+PATHA_OFFSET;
					break;
			case 18: offset = EEPROM_8812_XTAL_K;
					break;
			default:
					offset = -1;
					panic_printk("NOT SUPPORT!!\n");
					break;
			}
		}
#endif

	return offset;
}


/*  11/16/2008 MH Add description. Get current efuse area enabled word!!. */
static UINT8 efuse_CalculateWordCnts( UINT8	word_en)
{
	UINT8 word_cnts = 0;
	if(!(word_en & BIT(0)))	word_cnts++; // 0 : write enable
	if(!(word_en & BIT(1)))	word_cnts++;
	if(!(word_en & BIT(2)))	word_cnts++;
	if(!(word_en & BIT(3)))	word_cnts++;
	return word_cnts;
}	// efuse_CalculateWordCnts


/*-----------------------------------------------------------------------------
 * Function:	efuse_GetCurrentSize
 *
 * Overview:	Get current efuse size!!!
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/16/2008 	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
static UINT16 efuse_GetCurrentSize(struct rtl8192cd_priv *priv)
{
	INT32 bContinual = TRUE;
	UINT16 efuse_addr = 0;
	UINT8 hoffset=0,hworden=0;
	UINT8 efuse_data,word_cnts=0;

	do	{
		ReadEFuseByte(priv, efuse_addr, &efuse_data) ;
		if(efuse_data!=0xFF) 
		{
			if((efuse_data&0x1F) == 0x0F)		//extended header
			{
				hoffset = efuse_data;
				efuse_addr++;
				ReadEFuseByte(priv, efuse_addr ,&efuse_data);
				if((efuse_data & 0x0F) == 0x0F)
				{
					efuse_addr++;
					continue;
				}
				else
				{
					hoffset = ((hoffset & 0xE0) >> 5) | ((efuse_data & 0xF0) >> 1);
					hworden = efuse_data & 0x0F;
				}
			}		
			else
			{
				hoffset = (efuse_data>>4) & 0x0F;
				hworden =  efuse_data & 0x0F;
			}
			word_cnts = efuse_CalculateWordCnts(hworden);
			//read next header
			efuse_addr = efuse_addr + (word_cnts*2)+1;
		} else {
			bContinual = FALSE ;
		}
	} while (bContinual  && (efuse_addr  < EFUSE_REAL_CONTENT_LEN) );

	return efuse_addr;

}	// efuse_GetCurrentSize}


/*-----------------------------------------------------------------------------
 * Function:	efuse_WordEnableDataRead
 *
 * Overview:	Read allowed word in current efuse section data.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/16/2008 	MHC		Create Version 0.
 * 11/21/2008 	MHC		Fix Write bug when we only enable late word.
 *
 *---------------------------------------------------------------------------*/
static void efuse_WordEnableDataRead(UINT8 word_en, UINT8 *sourdata, UINT8 *targetdata)
{
	if (!(word_en&BIT(0)))	{
		targetdata[0] = sourdata[0];
		targetdata[1] = sourdata[1];
	}
	if (!(word_en&BIT(1)))	{
		targetdata[2] = sourdata[2];
		targetdata[3] = sourdata[3];
	}
	if (!(word_en&BIT(2)))	{
		targetdata[4] = sourdata[4];
		targetdata[5] = sourdata[5];
	}
	if (!(word_en&BIT(3)))	{
		targetdata[6] = sourdata[6];
		targetdata[7] = sourdata[7];
	}
}	// efuse_WordEnableDataRead


/*-----------------------------------------------------------------------------
 * Function:	efuse_PgPacketRead
 *
 * Overview:	Receive dedicated Efuse are content. For92s, we support 16
 *				area now. It will return 8 bytes content for every area.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/16/2008 	MHC		Reorganize code Arch and assign as local API.
 *
 *---------------------------------------------------------------------------*/
static INT32 efuse_PgPacketRead(struct rtl8192cd_priv *priv, UINT8 offset, UINT8 *data)
{
	UINT8 ReadState = PG_STATE_HEADER;
	INT32 bContinual = TRUE, bDataEmpty = TRUE ;
	UINT16 efuse_addr = 0;
	UINT8 hoffset=0,hworden=0, efuse_data,word_cnts=0, tmpidx=0;
	UINT8 tmpdata[8];
	UINT8 tmp_header;

	if(data==NULL)
		return FALSE;
	if(offset>15)
		return FALSE;

	memset(data, 0xff, sizeof(UINT8)*PGPKT_DATA_SIZE);
	memset(tmpdata, 0xff, sizeof(UINT8)*PGPKT_DATA_SIZE);

	//
	// <Roger_TODO> Efuse has been pre-programmed dummy 5Bytes at the end of Efuse by CP.
	// Skip dummy parts to prevent unexpected data read from Efuse.
	// By pass right now. 2009.02.19.
	//
	while(bContinual && (efuse_addr  < EFUSE_REAL_CONTENT_LEN) )	{
		//-------  Header Read -------------
		if(ReadState & PG_STATE_HEADER)		{
			ReadEFuseByte(priv, efuse_addr ,&efuse_data);
			if(efuse_data!=0xFF)
			{
				if((efuse_data & 0x1F) == 0x0F)
				{
					tmp_header = efuse_data;
					efuse_addr++;
					ReadEFuseByte(priv, efuse_addr ,&efuse_data);
					if((efuse_data & 0x0F) != 0x0F)
					{
						hoffset = ((tmp_header & 0xE0) >> 5) | ((efuse_data & 0xF0) >> 1);
						hworden = efuse_data & 0x0F;						
					}
					else
					{
						efuse_addr++;
						continue;
					}
					
				}
				else	
				{
					hoffset = (efuse_data>>4) & 0x0F;
					hworden =  efuse_data & 0x0F;
				}
				word_cnts = efuse_CalculateWordCnts(hworden);
				bDataEmpty = TRUE ;

				if(hoffset==offset){
					for(tmpidx = 0;tmpidx< word_cnts*2 ;tmpidx++){
						ReadEFuseByte(priv, efuse_addr+1+tmpidx ,&efuse_data);
						tmpdata[tmpidx] = efuse_data;
						if(efuse_data!=0xff){
							bDataEmpty = FALSE;
						}
					}
					if(bDataEmpty==FALSE){
						ReadState = PG_STATE_DATA;
					}else{//read next header
						efuse_addr = efuse_addr + (word_cnts*2)+1;
						ReadState = PG_STATE_HEADER;
					}
				}
				else{//read next header
					efuse_addr = efuse_addr + (word_cnts*2)+1;
					ReadState = PG_STATE_HEADER;
				}
			}
			else{
				bContinual = FALSE ;
			}
		}
		//-------  Data section Read -------------
		else if(ReadState & PG_STATE_DATA)	{
			efuse_WordEnableDataRead(hworden,tmpdata,data);
			efuse_addr = efuse_addr + (word_cnts*2)+1;
			ReadState = PG_STATE_HEADER;
		}
	}
	if(	(data[0]==0xff) &&(data[1]==0xff) && (data[2]==0xff)  && (data[3]==0xff) &&
		(data[4]==0xff) &&(data[5]==0xff) && (data[6]==0xff)  && (data[7]==0xff))
		return FALSE;
	else
		return TRUE;

}	// efuse_PgPacketRead


/*  11/16/2008 MH Write one byte to reald Efuse. */
static INT32 WriteEFuseByte(struct rtl8192cd_priv *priv, UINT16 addr, UINT8 data)
{
	UINT8 tmpidx = 0;
	INT32 bResult;

//	DEBUG_INFO("Addr = %x Data=%x\n", addr, data);

	// -----------------e-fuse reg ctrl ---------------------------------
	//address
	RTL_W8( EFUSE_CTRL+1, (UINT8)(addr&0xff));
	RTL_W8( EFUSE_CTRL+2, 	(RTL_R8(EFUSE_CTRL+2)&0xFC )|(UINT8)((addr>>8)&0x03));
	RTL_W8( EFUSE_CTRL, data);//data

	RTL_W8( EFUSE_CTRL+3, 0xF2	);//write cmd

	while((0x80 &  RTL_R8(EFUSE_CTRL+3)) && (tmpidx<100) ){
		tmpidx++;
	}

	if(tmpidx<100)	{
		bResult = TRUE;
	} else	{
		bResult = FALSE;
	}

	return bResult;
}	//


/*-----------------------------------------------------------------------------
 * Function:	efuse_WordEnableDataWrite
 *
 * Overview:	Write necessary word unit into current efuse section!
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/16/2008 	MHC		Reorganize Efuse operate flow!!.
 *
 *---------------------------------------------------------------------------*/
static UINT8 efuse_WordEnableDataWrite(struct rtl8192cd_priv *priv,
				UINT16 	efuse_addr, UINT8 word_en, UINT8 *data)
{
	UINT16 tmpaddr = 0;
	UINT16 start_addr = efuse_addr;
	UINT8 badworden = 0x0F;
	UINT8 tmpdata[8];

	//memset(tmpdata,0xff,PGPKT_DATA_SIZE);
	memset(tmpdata,  0xff, PGPKT_DATA_SIZE);
	DEBUG_INFO("word_en = %x efuse_addr=%x\n", word_en, efuse_addr);

	if(!(word_en&BIT(0)))	{
		tmpaddr = start_addr;
		WriteEFuseByte(priv,start_addr++, data[0]);
		WriteEFuseByte(priv,start_addr++, data[1]);
		ReadEFuseByte(priv,tmpaddr, &tmpdata[0]);
		ReadEFuseByte(priv,tmpaddr+1, &tmpdata[1]);
		if((data[0]!=tmpdata[0])||(data[1]!=tmpdata[1])){
			badworden &= (~ BIT(0));
		}
	}
	if(!(word_en&BIT(1)))	{
		tmpaddr = start_addr;
		WriteEFuseByte(priv,start_addr++, data[2]);
		WriteEFuseByte(priv,start_addr++, data[3]);
		ReadEFuseByte(priv,tmpaddr    , &tmpdata[2]);
		ReadEFuseByte(priv,tmpaddr+1, &tmpdata[3]);
		if((data[2]!=tmpdata[2])||(data[3]!=tmpdata[3])){
			badworden &=( ~ BIT(1));
		}
	}
	if(!(word_en&BIT(2)))	{
		tmpaddr = start_addr;
		WriteEFuseByte(priv,start_addr++, data[4]);
		WriteEFuseByte(priv,start_addr++, data[5]);
		ReadEFuseByte(priv,tmpaddr, &tmpdata[4]);
		ReadEFuseByte(priv,tmpaddr+1, &tmpdata[5]);
		if((data[4]!=tmpdata[4])||(data[5]!=tmpdata[5])){
			badworden &=( ~ BIT(2));
		}
	}
	if(!(word_en&BIT(3)))
	{
		tmpaddr = start_addr;
		WriteEFuseByte(priv,start_addr++, data[6]);
		WriteEFuseByte(priv,start_addr++, data[7]);
		ReadEFuseByte(priv,tmpaddr, &tmpdata[6]);
		ReadEFuseByte(priv,tmpaddr+1, &tmpdata[7]);
		if((data[6]!=tmpdata[6])||(data[7]!=tmpdata[7])){
			badworden &=( ~ BIT(3));
		}
	}
	return badworden;
}	// efuse_WordEnableDataWrite


//
//	Description:
//		This routine will calculate current shadow map that
//		how much bytes needs to be updated.
//
//	Assumption:
//		We shall call this routine before programming physical Efuse content.
//
//	Return Value:
//		TRUE: Efuse has enough capacity to program.
//		FALSE: Efuse do NOT has enough capacity to program.
//
//	Created by Roger, 2008.04.21.
//
static int EFUSE_ShadowUpdateChk(struct rtl8192cd_priv *priv)
{
	UINT8	SectionIdx, i, Base;
	UINT16	WordsNeed = 0, HdrNum = 0, TotalBytes = 0, EfuseUsed = 0;
	UINT8	bWordChanged, bResult = TRUE;

	// Efuse contain total 16 sections.
	for (SectionIdx = 0; SectionIdx < EFUSE_MAX_SECTION; SectionIdx++)	{
		Base = SectionIdx * 8;
		bWordChanged = FALSE;

		// One section contain 4 words = 8 bytes.
		for (i = 0; i < 8; i=i+2)		{
			if ((priv->EfuseMap[EFUSE_INIT_MAP][Base+i] != priv->EfuseMap[EFUSE_MODIFY_MAP][Base+i]) ||
				(priv->EfuseMap[EFUSE_INIT_MAP][Base+i+1] != priv->EfuseMap[EFUSE_MODIFY_MAP][Base+i+1])) {
				WordsNeed++;
				bWordChanged = TRUE;
			}
		}

		// We shall append Efuse header If any WORDs changed in this section.
		if (bWordChanged == TRUE)
			HdrNum++;
	}

	TotalBytes = HdrNum + WordsNeed*2;
	EfuseUsed = priv->EfuseUsedBytes;

	// Calculate whether updated map has enough capacity.
	if ((TotalBytes + EfuseUsed) >= (EFUSE_REAL_CONTENT_LEN - EFUSE_OOB_PROTECT_BYTES))
		bResult = FALSE;

	DEBUG_INFO("EFUSE_ShadowUpdateChk(): TotalBytes(%#x), HdrNum(%#x), WordsNeed(%#x), EfuseUsed(%d)\n",
		TotalBytes, HdrNum, WordsNeed, EfuseUsed);

	return bResult;
}


/*-----------------------------------------------------------------------------
 * Function:	efuse_PgPacketWrite
 *
 * Overview:	Send A G package for different section in real efuse area.
 *				For 92S, One PG package contain 8 bytes content and 4 word
 *				unit. PG header = 0x[bit7-4=offset][bit3-0word enable]
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/16/2008 	MHC		Reorganize code Arch and assign as local API.
 *
 *---------------------------------------------------------------------------*/
static int efuse_PgPacketWrite(struct rtl8192cd_priv *priv,
				UINT8 offset, UINT8 word_en, UINT8 *data)
{
	UINT8 WriteState = PG_STATE_HEADER;
	INT32 bContinual = TRUE, bDataEmpty= TRUE, bResult = TRUE, bExtendedHeader = FALSE;
	UINT16 efuse_addr = 0;
	UINT8 efuse_data, pg_header = 0, pg_header_temp = 0;

	UINT8 tmp_word_cnts=0,target_word_cnts=0;
	UINT8 tmp_header,match_word_en,tmp_word_en;

	PGPKT_STRUCT target_pkt;
	PGPKT_STRUCT tmp_pkt;

	UINT8 originaldata[sizeof(UINT8)*8];
	UINT8 tmpindex = 0, badworden = 0x0F;
	static INT32 repeat_times = 0;

	//
	// <Roger_Notes> Efuse has been pre-programmed dummy 5Bytes at the end of Efuse by CP.
	// So we have to prevent unexpected data string connection, which will cause
	// incorrect data auto-load from HW. The total size is equal or smaller than 498bytes
	// (i.e., offset 0~497, and dummy 1bytes) expected after CP test.
	// 2009.02.19.
	//
	if( efuse_GetCurrentSize(priv) >= (EFUSE_REAL_CONTENT_LEN-EFUSE_OOB_PROTECT_BYTES))	{
		DEBUG_INFO ("efuse_PgPacketWrite error \n");
		return FALSE;
	}

	tmp_pkt.offset = 0;
	tmp_pkt.word_en= 0;
	// Init the 8 bytes content as 0xff
	target_pkt.offset = offset;
	target_pkt.word_en= word_en;
	memset(target_pkt.data,  0xFF, sizeof(UINT8)*8);

	efuse_WordEnableDataRead(word_en,data,target_pkt.data);
	target_word_cnts = efuse_CalculateWordCnts(target_pkt.word_en);

	//
	// <Roger_Notes> Efuse has been pre-programmed dummy 5Bytes at the end of Efuse by CP.
	// So we have to prevent unexpected data string connection, which will cause
	// incorrect data auto-load from HW. Dummy 1bytes is additional.
	// 2009.02.19.
	//
	while( bContinual && (efuse_addr  < (EFUSE_REAL_CONTENT_LEN-EFUSE_OOB_PROTECT_BYTES)) )	{
		if(WriteState==PG_STATE_HEADER)		{
			bDataEmpty=TRUE;
			badworden = 0x0F;
			//************  so *******************
			DEBUG_INFO("EFUSE PG_STATE_HEADER\n");
			ReadEFuseByte(priv, efuse_addr ,&efuse_data);
			if (efuse_data!=0xFF)
			{
				if((efuse_data&0x1F) == 0x0F)		//extended header
				{
					tmp_header = efuse_data;
					efuse_addr++;
					ReadEFuseByte(priv, efuse_addr ,&efuse_data);
					if((efuse_data & 0x0F) == 0x0F)	//wren fail
					{
						efuse_addr++;
						continue;
					}
					else
					{
						tmp_pkt.offset = ((tmp_header & 0xE0) >> 5) | ((efuse_data & 0xF0) >> 1);
						tmp_pkt.word_en = efuse_data & 0x0F;
					}
				}
				else
				{
					tmp_header  =  efuse_data;
					tmp_pkt.offset 	= (tmp_header>>4) & 0x0F;
					tmp_pkt.word_en 	= tmp_header & 0x0F;
				}
				
				tmp_word_cnts =  efuse_CalculateWordCnts(tmp_pkt.word_en);

				//************  so-1 *******************
				if(tmp_pkt.offset  != target_pkt.offset)				{
					efuse_addr = efuse_addr + (tmp_word_cnts*2) +1; //Next pg_packet
#ifdef EFUSE_ERROE_HANDLE
					WriteState = PG_STATE_HEADER;
#endif
				} else	{
					//************  so-2 *******************
					for(tmpindex=0 ; tmpindex<(tmp_word_cnts*2) ; tmpindex++)	{
						ReadEFuseByte(priv, (efuse_addr+1+tmpindex) ,&efuse_data);
						if(efuse_data != 0xFF)
							bDataEmpty = FALSE;
					}
					//************  so-2-1 *******************
					if(bDataEmpty ==FALSE)	{
						efuse_addr = efuse_addr + (tmp_word_cnts*2) +1; //Next pg_packet
#ifdef EFUSE_ERROE_HANDLE
						WriteState=PG_STATE_HEADER;
#endif
					} else {
						//************  so-2-2 *******************
						match_word_en = 0x0F;
						if(   !( (target_pkt.word_en&BIT(0))|(tmp_pkt.word_en&BIT(0))  ))	{
							 match_word_en &= (~ BIT(0));
						}
						if(   !( (target_pkt.word_en&BIT(1))|(tmp_pkt.word_en&BIT(1))  ))	{
							 match_word_en &= (~ BIT(1));
						}
						if(   !( (target_pkt.word_en&BIT(2))|(tmp_pkt.word_en&BIT(2))  ))	{
							 match_word_en &= (~ BIT(2));
						}
						if(   !( (target_pkt.word_en&BIT(3))|(tmp_pkt.word_en&BIT(3))  ))	{
							 match_word_en &= (~ BIT(3));
						}

						//************  so-2-2-A *******************
						if((match_word_en&0x0F)!=0x0F)		{
							badworden = efuse_WordEnableDataWrite(priv,efuse_addr+1, tmp_pkt.word_en ,target_pkt.data);

							//************  so-2-2-A-1 *******************
							if(0x0F != (badworden&0x0F))	{
								UINT8 reorg_offset = offset;
								UINT8 reorg_worden=badworden;
								efuse_PgPacketWrite(priv,reorg_offset,reorg_worden,originaldata);
							}

							tmp_word_en = 0x0F;
							if(  (target_pkt.word_en&BIT(0))^(match_word_en&BIT(0))  )	{
								tmp_word_en &= (~ BIT(0));
							}
							if(   (target_pkt.word_en&BIT(1))^(match_word_en&BIT(1)) )	{
								tmp_word_en &=  (~ BIT(1));
							}
							if(   (target_pkt.word_en&BIT(2))^(match_word_en&BIT(2)) )	{
								tmp_word_en &= (~ BIT(2));
							}
							if(   (target_pkt.word_en&BIT(3))^(match_word_en&BIT(3)) )	{
								tmp_word_en &=(~ BIT(3));
							}

							//************  so-2-2-A-2 *******************
							if((tmp_word_en&0x0F)!=0x0F){
								//reorganize other pg packet
								efuse_addr = efuse_GetCurrentSize(priv);
								target_pkt.offset = offset;
								target_pkt.word_en= tmp_word_en;
							} else {
								bContinual = FALSE;
							}
#ifdef EFUSE_ERROE_HANDLE
							WriteState=PG_STATE_HEADER;
							repeat_times++;
							if(repeat_times>EFUSE_REPEAT_THRESHOLD_){
								bContinual = FALSE;
								bResult = FALSE;
							}
#endif
						}
						else{//************  so-2-2-B *******************
							//reorganize other pg packet
							efuse_addr = efuse_addr + (2*tmp_word_cnts) +1;//next pg packet addr
							target_pkt.offset = offset;
							target_pkt.word_en= target_pkt.word_en;
#ifdef EFUSE_ERROE_HANDLE
							WriteState=PG_STATE_HEADER;
#endif
						}
					}
				}
				DEBUG_INFO("EFUSE PG_STATE_HEADER-1\n");
			}else {
				//************  s1: header == oxff  *******************
				bExtendedHeader = FALSE;
				
				if(target_pkt.offset >= EFUSE_MAX_SECTION_BASE)
				{
					pg_header = ((target_pkt.offset &0x07) << 5) | 0x0F;

					DEBUG_INFO("efuse_PgPacketWrite extended pg_header[2:0] |0x0F 0x%x \n", pg_header);
					
					WriteEFuseByte(priv,efuse_addr, pg_header);
					ReadEFuseByte(priv,efuse_addr, &tmp_header);							

					while(tmp_header == 0xFF)
					{		
						DEBUG_INFO("efuse_PgPacketWrite extended pg_header[2:0] wirte fail \n");
					
						repeat_times++; 	
					
						if(repeat_times>EFUSE_REPEAT_THRESHOLD_){
							bContinual = FALSE;
							bResult = FALSE;
							efuse_addr++;
							break;
						}		
						WriteEFuseByte(priv,efuse_addr, pg_header);
						ReadEFuseByte(priv,efuse_addr, &tmp_header);																									
					}
					
					if(!bContinual)
						break;

					if(tmp_header == pg_header)
					{
						efuse_addr++;
						pg_header_temp = pg_header;
						pg_header = ((target_pkt.offset & 0x78) << 1 ) | target_pkt.word_en;

						DEBUG_INFO("efuse_PgPacketWrite extended pg_header[6:3] | worden 0x%x word_en 0x%x \n", pg_header);
						
						WriteEFuseByte(priv,efuse_addr, pg_header);
						ReadEFuseByte(priv,efuse_addr, &tmp_header);													

						while(tmp_header == 0xFF)
						{											
							repeat_times++;		

							if(repeat_times>EFUSE_REPEAT_THRESHOLD_){
								bContinual = FALSE;
								bResult = FALSE;
								break;
							}							
							WriteEFuseByte(priv,efuse_addr, pg_header);
							ReadEFuseByte(priv,efuse_addr, &tmp_header);																										
						}

						if(!bContinual)
							break;

						if((tmp_header & 0x0F) == 0x0F)	//wren PG fail
						{
							repeat_times++;		

							if(repeat_times>EFUSE_REPEAT_THRESHOLD_){
								bContinual = FALSE;
								bResult = FALSE;
								break;
							}							
							else
							{
								efuse_addr++;
								continue;
							}
						}
						else if(pg_header != tmp_header)	//offset PG fail						
						{
							bExtendedHeader = TRUE;
							tmp_pkt.offset = ((pg_header_temp & 0xE0) >> 5) | ((tmp_header & 0xF0) >> 1);
							tmp_pkt.word_en=  tmp_header & 0x0F;					
							tmp_word_cnts =  efuse_CalculateWordCnts(tmp_pkt.word_en);							
						}
					}
					else if ((tmp_header & 0x1F) == 0x0F)		//wrong extended header
					{
						efuse_addr+=2;
						continue;						
					}
				}
				else
				{
					pg_header = ((target_pkt.offset << 4)&0xf0) |target_pkt.word_en;
					WriteEFuseByte(priv,efuse_addr, pg_header);
					ReadEFuseByte(priv, efuse_addr, &tmp_header);
				}
				if(tmp_header == pg_header)	{ //************  s1-1*******************
					WriteState = PG_STATE_DATA;
				}	else
#ifdef EFUSE_ERROE_HANDLE
				 if(tmp_header == 0xFF){//************  s1-3: if Write or read func doesn't work *******************
					//efuse_addr doesn't change
					WriteState = PG_STATE_HEADER;
					repeat_times++;
					if(repeat_times>EFUSE_REPEAT_THRESHOLD_){
						bContinual = FALSE;
						bResult = FALSE;
					}
				} else
#endif
				{//************  s1-2 : fixed the header procedure *******************				
					if(!bExtendedHeader)
					{
						tmp_pkt.offset = (tmp_header>>4) & 0x0F;
						tmp_pkt.word_en=  tmp_header & 0x0F;
						tmp_word_cnts =  efuse_CalculateWordCnts(tmp_pkt.word_en);
					}

					//************  s1-2-A :cover the exist data *******************
					//memset(originaldata,0xff,sizeof(UINT8)*8);
					memset(originaldata, 0xff, sizeof(UINT8)*8);

					if(efuse_PgPacketRead( priv, tmp_pkt.offset,originaldata))	{	//check if data exist
						badworden = efuse_WordEnableDataWrite(priv,efuse_addr+1,tmp_pkt.word_en,originaldata);
						if(0x0F != (badworden&0x0F))						{
							UINT8 reorg_offset = tmp_pkt.offset;
							UINT8 reorg_worden=badworden;
							efuse_PgPacketWrite(priv,reorg_offset,reorg_worden,originaldata);
							efuse_addr = efuse_GetCurrentSize(priv);
						} else {
							efuse_addr = efuse_addr + (tmp_word_cnts*2) +1; //Next pg_packet
						}
					} else	{
						//************  s1-2-B: wrong address*******************
						efuse_addr = efuse_addr + (tmp_word_cnts*2) +1; //Next pg_packet
					}

#ifdef EFUSE_ERROE_HANDLE
					WriteState=PG_STATE_HEADER;
					repeat_times++;
					if(repeat_times>EFUSE_REPEAT_THRESHOLD_){
						bContinual = FALSE;
						bResult = FALSE;
					}
#endif
					DEBUG_INFO("EFUSE PG_STATE_HEADER-2\n");
				}

			}
		}
		//write data state
		else if(WriteState==PG_STATE_DATA)
		{	//************  s1-1  *******************
			DEBUG_INFO("EFUSE PG_STATE_DATA\n");
			badworden = 0x0f;
			badworden = efuse_WordEnableDataWrite(priv,efuse_addr+1,target_pkt.word_en,target_pkt.data);
			if((badworden&0x0F)==0x0F) {
				//************  s1-1-A *******************
				bContinual =FALSE;
			} else {
				//reorganize other pg packet //************  s1-1-B *******************
				efuse_addr = efuse_addr + (2*target_word_cnts) +1;//next pg packet addr
				//===========================
				target_pkt.offset = offset;
				target_pkt.word_en= badworden;
				target_word_cnts =  efuse_CalculateWordCnts(target_pkt.word_en);
				//===========================
#ifdef EFUSE_ERROE_HANDLE
				WriteState=PG_STATE_HEADER;
				repeat_times++;
				if(repeat_times>EFUSE_REPEAT_THRESHOLD_){
					bContinual = FALSE;
					bResult = FALSE;
				}
#endif
				DEBUG_INFO("EFUSE PG_STATE_HEADER-3\n");
			}
		}
	}

	if(efuse_addr  >= (EFUSE_REAL_CONTENT_LEN-EFUSE_OOB_PROTECT_BYTES)) 	{
		DEBUG_INFO("efuse_PgPacketWrite(): efuse_addr(%#x) Out of size!!\n", efuse_addr);
	}

	return TRUE;
}	// efuse_PgPacketWrite


/*-----------------------------------------------------------------------------
 * Function:	EFUSE_ShadowUpdate
 *
 * Overview:	Compare init and modify map to update Efuse!!!!!
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/12/2008 	MHC		Create Version 0.
 * 12/11/2008	MHC		92SE PH workaround to prevent HW autoload fail.
 * 12/30/2008	Roger	Fix the bug that EFUSE will writed out-of boundary.
 * 02/16/2009	Roger	Revise PCIe autoload fail case and compatible with USB interface to
 *						overcome MP issue.
 *
 *---------------------------------------------------------------------------*/
static int EFUSE_ShadowUpdate(struct rtl8192cd_priv *priv)
{
	UINT16			i, offset, base = 0;
	UINT8			word_en = 0x0F;

	//
	// <Roger_Notes> We have to check whether current Efuse capacity is big enough to program!!
	// 2009.04.21.
	//
	if(!EFUSE_ShadowUpdateChk(priv))	{
		//
		// <Roger_Notes> We shall reload current Efuse all map and synchronize current modified
		// map to prevent inconsistent Efuse content.
		// 2009.04.21.
		//
		EFUSE_ShadowMapUpdate(priv);
		DEBUG_INFO("<---EFUSE_ShadowUpdate(): Efuse out of capacity!!\n");
		return FALSE;
	}
	// For Efuse write action, we must enable LDO2.5V and 40MHZ clk.
	efuse_PowerSwitch(priv,TRUE,TRUE);

	//
	// Efuse support 16 write are with PG header packet!!!!
	//
	for (offset = 0; offset < EFUSE_MAX_SECTION; offset++) 	{
		// From section(0) to section(15) sequential write.
		word_en = 0x0F;
		base = offset * 8;
		//
		// Decide Word Enable Bit for the Efuse section
		// One section contain 4 words = 8 bytes!!!!!
		//
		for (i = 0; i < 8; i++)		{
			if (priv->EfuseMap[EFUSE_INIT_MAP][base+i] != priv->EfuseMap[EFUSE_MODIFY_MAP][base+i]) 	{
				word_en &= ~(BIT(i/2));
				DEBUG_INFO("Section(%#x) Addr[%x] %x update to %x, Word_En=%02x\n",
				offset, base+i, priv->EfuseMap[EFUSE_INIT_MAP][base+i],	priv->EfuseMap[EFUSE_MODIFY_MAP][base+i],word_en);
			}
		}

		//
		// This section will need to update, call Efuse real write section !!!!
		//
		if (word_en != 0x0F)	{
			UINT8	tmpdata[8];
			memcpy(tmpdata, (&priv->EfuseMap[EFUSE_MODIFY_MAP][base]), 8);

			//
			// <Roger_Notes> Break programming process if efuse capacity is NOT available.
			// 2009.04.20.
			//
			if(!efuse_PgPacketWrite(priv,(UINT8)offset,word_en,tmpdata))	{
				DEBUG_INFO("EFUSE_ShadowUpdate(): PG section(%#x) fail!!\n", offset);
				break;
			}
		}
	}

	// For warm reboot, we must resume Efuse clock to 500K.
	efuse_PowerSwitch(priv,TRUE, FALSE);

	//
	// <Roger_Notes> We update both init shadow map again and modified map
	// while WPG do loading operation after previous programming.
	// 2008.12.30.
	//
	EFUSE_ShadowMapUpdate(priv);
	DEBUG_INFO ("<---EFUSE_ShadowUpdate()\n");
	return TRUE;
}	// EFUSE_ShadowUpdate


static void shadowMapWrite(struct rtl8192cd_priv *priv, int type, char *value, unsigned char *hwinfo)
{
	int  i, offset ;
	offset = getEEPROMOffset(priv, type);

	if (offset<0)
		return;

#ifdef CONFIG_RTL_92C_SUPPORT
	if ((GET_CHIP_VER(priv) == VERSION_8188C) || (GET_CHIP_VER(priv) == VERSION_8192C)) {
		if (offset == EEPROM_MACADDRESS) {
			for (i=0; i<MACADDRLEN; i++) {
				get_array_val(hwinfo+offset+i, value+i*2, 2);
			}
		} else if (offset == EEPROM_THERMAL_METER) {
			get_array_val(hwinfo+offset,   value,     2);
		}
		else {
			get_array_val(hwinfo+offset,   value,     2);
			get_array_val(hwinfo+offset+1, value+3*2, 2);
			get_array_val(hwinfo+offset+2, value+9*2, 2);
		}
	}
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		if (offset == 0 || offset == 0x32) {
			for (i=0; i<0x32; i++) {
				get_array_val(hwinfo+offset+i, value+i*2, 2);
			}
		} else if (offset == EEPROM_MAC0_MACADDRESS || offset == EEPROM_MAC1_MACADDRESS) {
			for (i=0; i<MACADDRLEN; i++) {
				get_array_val(hwinfo+offset+i, value+i*2, 2);
			}
		} else if (offset == EEPROM_92D_THERMAL_METER ||
					offset == EEPROM_92D_TRSW_CTRL ||
					offset == EEPROM_92D_PAPE_CTRL ||
					offset == EEPROM_92D_XTAL_K ) {
			get_array_val(hwinfo+offset, value, 2);
		} else if ((offset == EEPROM_2G_TxPowerCCK) ||
			(offset == EEPROM_2G_TxPowerCCK+3) ||
			(offset == EEPROM_2G_TxPowerHT40_1S) ||
			(offset == EEPROM_2G_TxPowerHT40_1S+3) ||
			(offset == EEPROM_2G_TxPowerHT40_2SDiff) ||
			(offset == EEPROM_2G_TxPowerHT20Diff) ||
			(offset == EEPROM_2G_TxPowerOFDMDiff))
		{
			get_array_val(hwinfo+offset,   value,     2);
			get_array_val(hwinfo+offset+1, value+3*2, 2);
			get_array_val(hwinfo+offset+2, value+9*2, 2);
		} else if (offset == EEPROM_5GL_TxPowerHT40_1S) {
			get_array_val(hwinfo+EEPROM_5GL_TxPowerHT40_1S,   value+35*2,  2);
			get_array_val(hwinfo+EEPROM_5GL_TxPowerHT40_1S+1, value+45*2,  2);
			get_array_val(hwinfo+EEPROM_5GL_TxPowerHT40_1S+2, value+55*2,  2);
			get_array_val(hwinfo+EEPROM_5GM_TxPowerHT40_1S,   value+99*2,  2);
			get_array_val(hwinfo+EEPROM_5GM_TxPowerHT40_1S+1, value+113*2, 2);
			get_array_val(hwinfo+EEPROM_5GM_TxPowerHT40_1S+2, value+127*2, 2);
			get_array_val(hwinfo+EEPROM_5GH_TxPowerHT40_1S,   value+148*2, 2);
			get_array_val(hwinfo+EEPROM_5GH_TxPowerHT40_1S+1, value+154*2, 2);
			get_array_val(hwinfo+EEPROM_5GH_TxPowerHT40_1S+2, value+160*2, 2);
		} else if (offset == EEPROM_5GL_TxPowerHT40_1S+3) {
			get_array_val(hwinfo+EEPROM_5GL_TxPowerHT40_1S+3, value+35*2,  2);
			get_array_val(hwinfo+EEPROM_5GL_TxPowerHT40_1S+4, value+45*2,  2);
			get_array_val(hwinfo+EEPROM_5GL_TxPowerHT40_1S+5, value+55*2,  2);
			get_array_val(hwinfo+EEPROM_5GM_TxPowerHT40_1S+3, value+99*2,  2);
			get_array_val(hwinfo+EEPROM_5GM_TxPowerHT40_1S+4, value+113*2, 2);
			get_array_val(hwinfo+EEPROM_5GM_TxPowerHT40_1S+5, value+127*2, 2);
			get_array_val(hwinfo+EEPROM_5GH_TxPowerHT40_1S+3, value+148*2, 2);
			get_array_val(hwinfo+EEPROM_5GH_TxPowerHT40_1S+4, value+154*2, 2);
			get_array_val(hwinfo+EEPROM_5GH_TxPowerHT40_1S+5, value+160*2, 2);
		} else if (offset == EEPROM_5GL_TxPowerHT40_2SDiff) {
			get_array_val(hwinfo+EEPROM_5GL_TxPowerHT40_2SDiff,   value+35*2,  2);
			get_array_val(hwinfo+EEPROM_5GL_TxPowerHT40_2SDiff+1, value+45*2,  2);
			get_array_val(hwinfo+EEPROM_5GL_TxPowerHT40_2SDiff+2, value+55*2,  2);
			get_array_val(hwinfo+EEPROM_5GM_TxPowerHT40_2SDiff,   value+99*2,  2);
			get_array_val(hwinfo+EEPROM_5GM_TxPowerHT40_2SDiff+1, value+113*2, 2);
			get_array_val(hwinfo+EEPROM_5GM_TxPowerHT40_2SDiff+2, value+127*2, 2);
			get_array_val(hwinfo+EEPROM_5GH_TxPowerHT40_2SDiff,   value+148*2, 2);
			get_array_val(hwinfo+EEPROM_5GH_TxPowerHT40_2SDiff+1, value+154*2, 2);
			get_array_val(hwinfo+EEPROM_5GH_TxPowerHT40_2SDiff+2, value+160*2, 2);
		} else if (offset == EEPROM_5GL_TxPowerHT20Diff) {
			get_array_val(hwinfo+EEPROM_5GL_TxPowerHT20Diff,   value+35*2,  2);
			get_array_val(hwinfo+EEPROM_5GL_TxPowerHT20Diff+1, value+45*2,  2);
			get_array_val(hwinfo+EEPROM_5GL_TxPowerHT20Diff+2, value+55*2,  2);
			get_array_val(hwinfo+EEPROM_5GM_TxPowerHT20Diff,   value+99*2,  2);
			get_array_val(hwinfo+EEPROM_5GM_TxPowerHT20Diff+1, value+113*2, 2);
			get_array_val(hwinfo+EEPROM_5GM_TxPowerHT20Diff+2, value+127*2, 2);
			get_array_val(hwinfo+EEPROM_5GH_TxPowerHT20Diff,   value+148*2, 2);
			get_array_val(hwinfo+EEPROM_5GH_TxPowerHT20Diff+1, value+154*2, 2);
			get_array_val(hwinfo+EEPROM_5GH_TxPowerHT20Diff+2, value+160*2, 2);
		} else if (offset == EEPROM_5GL_TxPowerOFDMDiff) {
			get_array_val(hwinfo+EEPROM_5GL_TxPowerOFDMDiff,   value+35*2,  2);
			get_array_val(hwinfo+EEPROM_5GL_TxPowerOFDMDiff+1, value+45*2,  2);
			get_array_val(hwinfo+EEPROM_5GL_TxPowerOFDMDiff+2, value+55*2,  2);
			get_array_val(hwinfo+EEPROM_5GM_TxPowerOFDMDiff,   value+99*2,  2);
			get_array_val(hwinfo+EEPROM_5GM_TxPowerOFDMDiff+1, value+113*2, 2);
			get_array_val(hwinfo+EEPROM_5GM_TxPowerOFDMDiff+2, value+127*2, 2);
			get_array_val(hwinfo+EEPROM_5GH_TxPowerOFDMDiff,   value+148*2, 2);
			get_array_val(hwinfo+EEPROM_5GH_TxPowerOFDMDiff+1, value+154*2, 2);
			get_array_val(hwinfo+EEPROM_5GH_TxPowerOFDMDiff+2, value+160*2, 2);
		}
	}
#endif

#ifdef CONFIG_RTL_8812_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8812E) {
		if (offset == 0 || offset == 0x32) {
			for (i=0; i<0x32; i++) {
				get_array_val(hwinfo+offset+i, value+i*2, 2);
			}
		} else if (offset == EEPROM_8812_MACADDRESS) {
			for (i=0; i<MACADDRLEN; i++) {
				get_array_val(hwinfo+offset+i, value+i*2, 2);
			}
		} else if (offset == EEPROM_8812_THERMAL_METER ||
					offset == EEPROM_8812_XTAL_K ) {
			get_array_val(hwinfo+offset, value, 2);
		} else if ((offset == EEPROM_2G_CCK1T_TxPower+PATHA_OFFSET) ||
			(offset == EEPROM_2G_CCK1T_TxPower+PATHB_OFFSET) ||
			(offset == EEPROM_2G_HT401S_TxPower+PATHA_OFFSET) ||
			(offset == EEPROM_2G_HT401S_TxPower+PATHB_OFFSET) ||
			(offset == EEPROM_2G_HT201S_TxPowerDiff+PATHA_OFFSET) ||
			(offset == EEPROM_2G_HT201S_TxPowerDiff+PATHA_OFFSET) ||
			(offset == EEPROM_2G_OFDM1T_TxPowerDiff+PATHA_OFFSET))
		{
			get_array_val(hwinfo+offset,   value,     2);
			get_array_val(hwinfo+offset+1, value+3*2, 2);
			get_array_val(hwinfo+offset+2, value+9*2, 2);
		} else if (offset == EEPROM_5G_HT401S_TxPower+PATHA_OFFSET) {
			get_array_val(hwinfo+EEPROM_5G_HT401S_TxPower+PATHA_OFFSET,   value+35*2,  2);
			get_array_val(hwinfo+EEPROM_5G_HT401S_TxPower+1, value+45*2,  2);
			get_array_val(hwinfo+EEPROM_5G_HT401S_TxPower+2, value+55*2,  2);
			get_array_val(hwinfo+EEPROM_5G_HT401S_TxPower,   value+99*2,  2);
			get_array_val(hwinfo+EEPROM_5G_HT401S_TxPower+1, value+113*2, 2);
			get_array_val(hwinfo+EEPROM_5G_HT401S_TxPower+2, value+127*2, 2);
			get_array_val(hwinfo+EEPROM_5G_HT401S_TxPower,   value+148*2, 2);
			get_array_val(hwinfo+EEPROM_5G_HT401S_TxPower+1, value+154*2, 2);
			get_array_val(hwinfo+EEPROM_5G_HT401S_TxPower+2, value+160*2, 2);
		} else if (offset == EEPROM_5G_HT401S_TxPower+PATHB_OFFSET) {
			get_array_val(hwinfo+EEPROM_5G_HT401S_TxPower+PATHB_OFFSET, value+35*2,  2);
			get_array_val(hwinfo+EEPROM_5G_HT401S_TxPower+PATHB_OFFSET+1, value+45*2,  2);
			get_array_val(hwinfo+EEPROM_5G_HT401S_TxPower+PATHB_OFFSET+2, value+55*2,  2);
			get_array_val(hwinfo+EEPROM_5G_HT401S_TxPower+PATHB_OFFSET, value+99*2,  2);
			get_array_val(hwinfo+EEPROM_5G_HT401S_TxPower+PATHB_OFFSET+1, value+113*2, 2);
			get_array_val(hwinfo+EEPROM_5G_HT401S_TxPower+PATHB_OFFSET+2, value+127*2, 2);
			get_array_val(hwinfo+EEPROM_5G_HT401S_TxPower+PATHB_OFFSET, value+148*2, 2);
			get_array_val(hwinfo+EEPROM_5G_HT401S_TxPower+PATHB_OFFSET+1, value+154*2, 2);
			get_array_val(hwinfo+EEPROM_5G_HT401S_TxPower+PATHB_OFFSET+2, value+160*2, 2);
		} else if (offset == EEPROM_5G_HT402S_TxPowerDiff+PATHA_OFFSET) {
			get_array_val(hwinfo+EEPROM_5G_HT402S_TxPowerDiff+PATHA_OFFSET,   value+35*2,  2);
			get_array_val(hwinfo+EEPROM_5G_HT402S_TxPowerDiff+PATHA_OFFSET+1, value+45*2,  2);
			get_array_val(hwinfo+EEPROM_5G_HT402S_TxPowerDiff+PATHA_OFFSET+2, value+55*2,  2);
			get_array_val(hwinfo+EEPROM_5G_HT402S_TxPowerDiff+PATHA_OFFSET,   value+99*2,  2);
			get_array_val(hwinfo+EEPROM_5G_HT402S_TxPowerDiff+PATHA_OFFSET+1, value+113*2, 2);
			get_array_val(hwinfo+EEPROM_5G_HT402S_TxPowerDiff+PATHA_OFFSET+2, value+127*2, 2);
			get_array_val(hwinfo+EEPROM_5G_HT402S_TxPowerDiff+PATHA_OFFSET,   value+148*2, 2);
			get_array_val(hwinfo+EEPROM_5G_HT402S_TxPowerDiff+PATHA_OFFSET+1, value+154*2, 2);
			get_array_val(hwinfo+EEPROM_5G_HT402S_TxPowerDiff+PATHA_OFFSET+2, value+160*2, 2);
		} else if (offset == EEPROM_5G_HT201S_TxPowerDiff+PATHA_OFFSET) {
			get_array_val(hwinfo+EEPROM_5G_HT201S_TxPowerDiff+PATHA_OFFSET,   value+35*2,  2);
			get_array_val(hwinfo+EEPROM_5G_HT201S_TxPowerDiff+PATHA_OFFSET+1, value+45*2,  2);
			get_array_val(hwinfo+EEPROM_5G_HT201S_TxPowerDiff+PATHA_OFFSET+2, value+55*2,  2);
			get_array_val(hwinfo+EEPROM_5G_HT201S_TxPowerDiff+PATHA_OFFSET,   value+99*2,  2);
			get_array_val(hwinfo+EEPROM_5G_HT201S_TxPowerDiff+PATHA_OFFSET+1, value+113*2, 2);
			get_array_val(hwinfo+EEPROM_5G_HT201S_TxPowerDiff+PATHA_OFFSET+2, value+127*2, 2);
			get_array_val(hwinfo+EEPROM_5G_HT201S_TxPowerDiff+PATHA_OFFSET,   value+148*2, 2);
			get_array_val(hwinfo+EEPROM_5G_HT201S_TxPowerDiff+PATHA_OFFSET+1, value+154*2, 2);
			get_array_val(hwinfo+EEPROM_5G_HT201S_TxPowerDiff+PATHA_OFFSET+2, value+160*2, 2);
		} else if (offset == EEPROM_5G_OFDM1T_TxPowerDiff+PATHA_OFFSET) {
			get_array_val(hwinfo+EEPROM_5G_OFDM1T_TxPowerDiff+PATHA_OFFSET,   value+35*2,  2);
			get_array_val(hwinfo+EEPROM_5G_OFDM1T_TxPowerDiff+PATHA_OFFSET+1, value+45*2,  2);
			get_array_val(hwinfo+EEPROM_5G_OFDM1T_TxPowerDiff+PATHA_OFFSET+2, value+55*2,  2);
			get_array_val(hwinfo+EEPROM_5G_OFDM1T_TxPowerDiff+PATHA_OFFSET,   value+99*2,  2);
			get_array_val(hwinfo+EEPROM_5G_OFDM1T_TxPowerDiff+PATHA_OFFSET+1, value+113*2, 2);
			get_array_val(hwinfo+EEPROM_5G_OFDM1T_TxPowerDiff+PATHA_OFFSET+2, value+127*2, 2);
			get_array_val(hwinfo+EEPROM_5G_OFDM1T_TxPowerDiff+PATHA_OFFSET,   value+148*2, 2);
			get_array_val(hwinfo+EEPROM_5G_OFDM1T_TxPowerDiff+PATHA_OFFSET+1, value+154*2, 2);
			get_array_val(hwinfo+EEPROM_5G_OFDM1T_TxPowerDiff+PATHA_OFFSET+2, value+160*2, 2);
		}
	}
#endif
}


static int converToFlashFormat(struct rtl8192cd_priv *priv, unsigned char *out, unsigned char *hwinfo, int type)
{
	int  i, offset, len=0;
	offset = getEEPROMOffset(priv, type);

	if (offset < 0)
		return 0;

	sprintf(out, "%s=", FLASH_NAME_PARAM[type]);
	len += 1 + strlen(FLASH_NAME_PARAM[type]);

#ifdef CONFIG_RTL_92C_SUPPORT
	if ((GET_CHIP_VER(priv) == VERSION_8188C) || (GET_CHIP_VER(priv) == VERSION_8192C)) {
		if (offset == EEPROM_MACADDRESS) {
			for (i=0; i<MACADDRLEN; i++) {
				sprintf(out+len, "%02x", hwinfo[offset+i]);
				len += 2;
			}
		} else if (offset == EEPROM_THERMAL_METER) {
			sprintf(out+len, "%02x", hwinfo[offset]);
			len += 2;
		} else {
			for (i=0; i<MAX_2G_CHANNEL_NUM; i++) {
				if (i < 3) {
					sprintf(out+len, "%02x", hwinfo[offset]);
					len += 2;
				} else if (i < 9) {
					sprintf(out+len, "%02x", hwinfo[offset+1]);
					len += 2;
				} else {
					sprintf(out+len, "%02x", hwinfo[offset+2]);
					len += 2;
				}
			}
		}
	}
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		if (offset == EEPROM_MAC0_MACADDRESS || offset == EEPROM_MAC1_MACADDRESS) {
			for (i=0; i<MACADDRLEN; i++) {
				sprintf(out+len, "%02x", hwinfo[offset+i]);
				len += 2;
			}
		} else if (offset == EEPROM_92D_THERMAL_METER  ||
					offset == EEPROM_92D_TRSW_CTRL ||
					offset == EEPROM_92D_PAPE_CTRL ||
					offset == EEPROM_92D_XTAL_K ) {
			sprintf(out+len, "%02x", hwinfo[offset]);
			len += 2;
		} else if ((offset == EEPROM_2G_TxPowerCCK) ||
			(offset == EEPROM_2G_TxPowerCCK+3) ||
			(offset == EEPROM_2G_TxPowerHT40_1S) ||
			(offset == EEPROM_2G_TxPowerHT40_1S+3) ||
			(offset == EEPROM_2G_TxPowerHT40_2SDiff) ||
			(offset == EEPROM_2G_TxPowerHT20Diff) ||
			(offset == EEPROM_2G_TxPowerOFDMDiff))
		{
			for (i=0; i<MAX_2G_CHANNEL_NUM; i++) {
				if (i < 3) {
					sprintf(out+len, "%02x", hwinfo[offset]);
					len += 2;
				} else if (i < 9) {
					sprintf(out+len, "%02x", hwinfo[offset+1]);
					len += 2;
				} else {
					sprintf(out+len, "%02x", hwinfo[offset+2]);
					len += 2;
				}
			}
		} else if (offset == EEPROM_5GL_TxPowerHT40_1S) {
			for (i=0; i<MAX_5G_CHANNEL_NUM; i++) {
				if (i >= 35 && i <= 63) { // ch 36 ~ 64
					if (i >= 35 && i <= 43) { // ch 36 ~ 44
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GL_TxPowerHT40_1S]);
						len += 2;
					} else if (i >= 45 && i <= 53) { // ch 46 ~ 54
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GL_TxPowerHT40_1S+1]);
						len += 2;
					} else if (i >= 55 && i <= 63) { // ch 56 ~ 64
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GL_TxPowerHT40_1S+2]);
						len += 2;
					} else {
						sprintf(out+len, "00");
						len += 2;
					}
				} else if (i >= 99 && i <= 139) { // ch 100 ~ 140
					if (i >= 99 && i <= 111) { // ch 100 ~ 112
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GM_TxPowerHT40_1S]);
						len += 2;
					} else if (i >= 113 && i <= 125) { // ch 114 ~ 126
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GM_TxPowerHT40_1S+1]);
						len += 2;
					} else if (i >= 127 && i <= 139) { // ch 128 ~ 140
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GM_TxPowerHT40_1S+2]);
						len += 2;
					} else {
						sprintf(out+len, "00");
						len += 2;
					}
				} else if (i >= 148 && i <= 164 ) { // ch 149 ~ 165
					if (i >= 148 && i <= 152) { // ch 149 ~ 153
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GH_TxPowerHT40_1S]);
						len += 2;
					} else if (i >= 154 && i <= 158) { // ch 155 ~ 159
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GH_TxPowerHT40_1S+1]);
						len += 2;
					} else if (i >= 160 && i <= 164) { // ch 161 ~ 165
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GH_TxPowerHT40_1S+2]);
						len += 2;
					} else {
						sprintf(out+len, "00");
						len += 2;
					}
				}
				else {
					sprintf(out+len, "00");
					len += 2;
				}
			}
		} else if (offset == EEPROM_5GL_TxPowerHT40_1S+3) {
			for (i=0; i<MAX_5G_CHANNEL_NUM; i++) {
				if (i >= 35 && i <= 63) { // ch 36 ~ 64
					if (i >= 35 && i <= 43) { // ch 36 ~ 44
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GL_TxPowerHT40_1S+3]);
						len += 2;
					} else if (i >= 45 && i <= 53) { // ch 46 ~ 54
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GL_TxPowerHT40_1S+4]);
						len += 2;
					} else if (i >= 55 && i <= 63) { // ch 56 ~ 64
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GL_TxPowerHT40_1S+5]);
						len += 2;
					} else {
						sprintf(out+len, "00");
						len += 2;
					}
				} else if (i >= 99 && i <= 139) { // ch 100 ~ 140
					if (i >= 99 && i <= 111) { // ch 100 ~ 112
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GM_TxPowerHT40_1S+3]);
						len += 2;
					} else if (i >= 113 && i <= 125) { // ch 114 ~ 126
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GM_TxPowerHT40_1S+4]);
						len += 2;
					} else if (i >= 127 && i <= 139) { // ch 128 ~ 140
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GM_TxPowerHT40_1S+5]);
						len += 2;
					} else {
						sprintf(out+len, "00");
						len += 2;
					}
				} else if (i >= 148 && i <= 164 ) { // ch 149 ~ 165
					if (i >= 148 && i <= 152) { // ch 149 ~ 153
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GH_TxPowerHT40_1S+3]);
						len += 2;
					} else if (i >= 154 && i <= 158) { // ch 155 ~ 159
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GH_TxPowerHT40_1S+4]);
						len += 2;
					} else if (i >= 160 && i <= 164) { // ch 161 ~ 165
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GH_TxPowerHT40_1S+5]);
						len += 2;
					} else {
						sprintf(out+len, "00");
						len += 2;
					}
				}
				else {
					sprintf(out+len, "00");
					len += 2;
				}
			}
		} else if (offset == EEPROM_5GL_TxPowerHT40_2SDiff) {
			for (i=0; i<MAX_5G_CHANNEL_NUM; i++) {
				if (i >= 35 && i <= 63) { // ch 36 ~ 64
					if (i >= 35 && i <= 43) { // ch 36 ~ 44
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GL_TxPowerHT40_2SDiff]);
						len += 2;
					} else if (i >= 45 && i <= 53) { // ch 46 ~ 54
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GL_TxPowerHT40_2SDiff+1]);
						len += 2;
					} else if (i >= 55 && i <= 63) { // ch 56 ~ 64
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GL_TxPowerHT40_2SDiff+2]);
						len += 2;
					} else {
						sprintf(out+len, "00");
						len += 2;
					}
				} else if (i >= 99 && i <= 139) { // ch 100 ~ 140
					if (i >= 99 && i <= 111) { // ch 100 ~ 112
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GM_TxPowerHT40_2SDiff]);
						len += 2;
					} else if (i >= 113 && i <= 125) { // ch 114 ~ 126
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GM_TxPowerHT40_2SDiff+1]);
						len += 2;
					} else if (i >= 127 && i <= 139) { // ch 128 ~ 140
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GM_TxPowerHT40_2SDiff+2]);
						len += 2;
					} else {
						sprintf(out+len, "00");
						len += 2;
					}
				} else if (i >= 148 && i <= 164 ) { // ch 149 ~ 165
					if (i >= 148 && i <= 152) { // ch 149 ~ 153
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GH_TxPowerHT40_2SDiff]);
						len += 2;
					} else if (i >= 154 && i <= 158) { // ch 155 ~ 159
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GH_TxPowerHT40_2SDiff+1]);
						len += 2;
					} else if (i >= 160 && i <= 164) { // ch 161 ~ 165
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GH_TxPowerHT40_2SDiff+2]);
						len += 2;
					} else {
						sprintf(out+len, "00");
						len += 2;
					}
				}
				else {
					sprintf(out+len, "00");
					len += 2;
				}
			}
		} else if (offset == EEPROM_5GL_TxPowerHT20Diff) {
			for (i=0; i<MAX_5G_CHANNEL_NUM; i++) {
				if (i >= 35 && i <= 63) { // ch 36 ~ 64
					if (i >= 35 && i <= 43) { // ch 36 ~ 44
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GL_TxPowerHT20Diff]);
						len += 2;
					} else if (i >= 45 && i <= 53) { // ch 46 ~ 54
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GL_TxPowerHT20Diff+1]);
						len += 2;
					} else if (i >= 55 && i <= 63) { // ch 56 ~ 64
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GL_TxPowerHT20Diff+2]);
						len += 2;
					} else {
						sprintf(out+len, "00");
						len += 2;
					}
				} else if (i >= 99 && i <= 139) { // ch 100 ~ 140
					if (i >= 99 && i <= 111) { // ch 100 ~ 112
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GM_TxPowerHT20Diff]);
						len += 2;
					} else if (i >= 113 && i <= 125) { // ch 114 ~ 126
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GM_TxPowerHT20Diff+1]);
						len += 2;
					} else if (i >= 127 && i <= 139) { // ch 128 ~ 140
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GM_TxPowerHT20Diff+2]);
						len += 2;
					} else {
						sprintf(out+len, "00");
						len += 2;
					}
				} else if (i >= 148 && i <= 164 ) { // ch 149 ~ 165
					if (i >= 148 && i <= 152) { // ch 149 ~ 153
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GH_TxPowerHT20Diff]);
						len += 2;
					} else if (i >= 154 && i <= 158) { // ch 155 ~ 159
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GH_TxPowerHT20Diff+1]);
						len += 2;
					} else if (i >= 160 && i <= 164) { // ch 161 ~ 165
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GH_TxPowerHT20Diff+2]);
						len += 2;
					} else {
						sprintf(out+len, "00");
						len += 2;
					}
				}
				else {
					sprintf(out+len, "00");
					len += 2;
				}
			}
		} else if (offset == EEPROM_5GL_TxPowerOFDMDiff) {
			for (i=0; i<MAX_5G_CHANNEL_NUM; i++) {
				if (i >= 35 && i <= 63) { // ch 36 ~ 64
					if (i >= 35 && i <= 43) { // ch 36 ~ 44
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GL_TxPowerOFDMDiff]);
						len += 2;
					} else if (i >= 45 && i <= 53) { // ch 46 ~ 54
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GL_TxPowerOFDMDiff+1]);
						len += 2;
					} else if (i >= 55 && i <= 63) { // ch 56 ~ 64
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GL_TxPowerOFDMDiff+2]);
						len += 2;
					} else {
						sprintf(out+len, "00");
						len += 2;
					}
				} else if (i >= 99 && i <= 139) { // ch 100 ~ 140
					if (i >= 99 && i <= 111) { // ch 100 ~ 112
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GM_TxPowerOFDMDiff]);
						len += 2;
					} else if (i >= 113 && i <= 125) { // ch 114 ~ 126
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GM_TxPowerOFDMDiff+1]);
						len += 2;
					} else if (i >= 127 && i <= 139) { // ch 128 ~ 140
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GM_TxPowerOFDMDiff+2]);
						len += 2;
					} else {
						sprintf(out+len, "00");
						len += 2;
					}
				} else if (i >= 148 && i <= 164 ) { // ch 149 ~ 165
					if (i >= 148 && i <= 152) { // ch 149 ~ 153
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GH_TxPowerOFDMDiff]);
						len += 2;
					} else if (i >= 154 && i <= 158) { // ch 155 ~ 159
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GH_TxPowerOFDMDiff+1]);
						len += 2;
					} else if (i >= 160 && i <= 164) { // ch 161 ~ 165
						sprintf(out+len, "%02x", hwinfo[EEPROM_5GH_TxPowerOFDMDiff+2]);
						len += 2;
					} else {
						sprintf(out+len, "00");
						len += 2;
					}
				}
				else {
					sprintf(out+len, "00");
					len += 2;
				}
			}
		}
 	}
#endif

#ifdef CONFIG_RTL_8812_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8812E) {
		if (offset == EEPROM_8812_MACADDRESS) {
			for (i=0; i<MACADDRLEN; i++) {
				sprintf(out+len, "%02x", hwinfo[offset+i]);
				len += 2;
			}
		} else if (offset == EEPROM_8812_THERMAL_METER  ||
					offset == EEPROM_8812_XTAL_K ) {
			sprintf(out+len, "%02x", hwinfo[offset]);
			len += 2;
		} else if ((offset == EEPROM_2G_CCK1T_TxPower+PATHA_OFFSET) ||
			(offset == EEPROM_2G_CCK1T_TxPower+PATHB_OFFSET) ||
			(offset == EEPROM_2G_HT401S_TxPower+PATHA_OFFSET) ||
			(offset == EEPROM_2G_HT401S_TxPower+PATHB_OFFSET) )
		{
			for (i=0; i<MAX_2G_CHANNEL_NUM; i++) {
				int chnl_gp = find_2gchnlgroup(i);

				if (chnl_gp > -1) {
					sprintf(out+len, "%02x", hwinfo[offset+chnl_gp]);
					len += 2;
				}
			}
		} else if (	(offset == EEPROM_2G_HT402S_TxPowerDiff+PATHA_OFFSET) ||
			(offset == EEPROM_2G_HT201S_TxPowerDiff+PATHA_OFFSET) ||
			(offset == EEPROM_2G_OFDM1T_TxPowerDiff+PATHA_OFFSET))
		{
			for (i=0; i<MAX_2G_CHANNEL_NUM; i++) {
				int chnl_gp = find_2gchnlgroup(i);

				if (chnl_gp > -1) {
					sprintf(out+len, "%02x", hwinfo[offset]);
					len += 2;
				}
			}
		} else if (offset == EEPROM_5G_HT401S_TxPower+PATHA_OFFSET) {
			for (i=0; i<MAX_5G_CHANNEL_NUM; i++) {
				int chnl_gp = find_5gchnlgroup(i);

				if (chnl_gp > -1) {
					sprintf(out+len, "%02x", hwinfo[offset+chnl_gp]);
					len += 2;
				}
			}
		} else if (offset == EEPROM_5G_HT401S_TxPower+PATHB_OFFSET) {
			for (i=0; i<MAX_5G_CHANNEL_NUM; i++) {
				int chnl_gp = find_5gchnlgroup(i);

				if (chnl_gp > -1) {
					sprintf(out+len, "%02x", hwinfo[offset+chnl_gp]);
					len += 2;
				}
			}
		} else if (offset == EEPROM_5G_HT402S_TxPowerDiff+PATHA_OFFSET) {
			for (i=0; i<MAX_5G_CHANNEL_NUM; i++) {
				int chnl_gp = find_5gchnlgroup(i);

				if (chnl_gp > -1) {
					sprintf(out+len, "%02x", hwinfo[offset]);
					len += 2;
				}
			}
		} else if (offset == EEPROM_5G_HT201S_TxPowerDiff+PATHA_OFFSET) {
			for (i=0; i<MAX_5G_CHANNEL_NUM; i++) {
				int chnl_gp = find_5gchnlgroup(i);

				if (chnl_gp > -1) {
					sprintf(out+len, "%02x", hwinfo[offset]);
					len += 2;
				}
			}
		} else if (offset == EEPROM_5G_OFDM1T_TxPowerDiff+PATHA_OFFSET) {
			for (i=0; i<MAX_5G_CHANNEL_NUM; i++) {
				int chnl_gp = find_5gchnlgroup(i);

				if (chnl_gp > -1) {
					sprintf(out+len, "%02x", hwinfo[offset]);
					len += 2;
				}
			}
		}
	}
#endif

	out[len]='\0';
	return len+1;
}


int efuse_get(struct rtl8192cd_priv *priv, unsigned char *data)
{
	int j, len, para_num=0;

#ifdef CONFIG_RTL_92C_SUPPORT
	if ((GET_CHIP_VER(priv) == VERSION_8188C) || (GET_CHIP_VER(priv) == VERSION_8192C))
		para_num = EFUSECMD_NUM_92C;
#endif
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT)
	if ((GET_CHIP_VER(priv) == VERSION_8192D) || (GET_CHIP_VER(priv) == VERSION_8812E))
		para_num = EFUSECMD_NUM_92D;
#endif

	for (j=0; j<para_num; j++) {
		if (strcmp(data, FLASH_NAME_PARAM[j])==0) {
			len =  converToFlashFormat(priv, data, &(priv->EfuseMap[EFUSE_INIT_MAP][0]), j);
			//printk("%s\n", data);
			return len;
		}
	}
	return 0;
}


int efuse_set(struct rtl8192cd_priv *priv, unsigned char *data)
{
	char *val;
	int j, para_num=0;

#ifdef CONFIG_RTL_92C_SUPPORT
	if ((GET_CHIP_VER(priv) == VERSION_8188C) || (GET_CHIP_VER(priv) == VERSION_8192C))
		para_num = EFUSECMD_NUM_92C;
#endif
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT)
	if ((GET_CHIP_VER(priv) == VERSION_8192D) || (GET_CHIP_VER(priv) == VERSION_8812E))
		para_num = EFUSECMD_NUM_92D;
#endif

	for (j=0; j<para_num; j++) {
		val = get_value_by_token((char *)data, FLASH_NAME_PARAM[j]);
		if (val) {
			printk("%s=[%s]\n", FLASH_NAME_PARAM[j], val+1);
			shadowMapWrite(priv, j, val+1, &(priv->EfuseMap[EFUSE_MODIFY_MAP][0]));
		}
	}
	return 0;
}


int efuse_sync(struct rtl8192cd_priv *priv, unsigned char *data)
{
	DEBUG_INFO("efuse sync\n");
	EFUSE_ShadowUpdate(priv);
	return 0;
}
#endif // EN_EFUSE

#ifdef TX_EARLY_MODE
void enable_em(struct rtl8192cd_priv *priv)
{
#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8188E) {
		RTL_W32(EARLY_MODE_CTRL, RTL_R32(EARLY_MODE_CTRL) |0x8000001f);
		priv->pshare->aggrmax_bak = RTL_R16(PROT_MODE_CTRL+2);
		//RTL_W16(PROT_MODE_CTRL+2, 0x0808);
		RTL_W16(PROT_MODE_CTRL+2, 0x0a0a);
	} else
#endif	
	{
		RTL_W32(EARLY_MODE_CTRL, RTL_R32(EARLY_MODE_CTRL) |0x8000000f); // enable signel AMPDU, early mode for vi/vo/be/bk queue
	}

	RTL_W16(TCR, RTL_R16(TCR) | WMAC_TCR_ERRSTEN2);		
}

void disable_em(struct rtl8192cd_priv *priv)
{
#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8188E) {
		RTL_W32(EARLY_MODE_CTRL, RTL_R32(EARLY_MODE_CTRL) & ~0x8000001f);
		if (priv->pshare->aggrmax_bak != 0)
			RTL_W16(PROT_MODE_CTRL+2, (priv->pshare->aggrmax_bak & 0xffff));
	} else
#endif	
	{
		RTL_W32(EARLY_MODE_CTRL, RTL_R32(EARLY_MODE_CTRL) & ~0x8000000f); // disable signel AMPDU, early mode for vi/vo/be/bk queue
	}
	
	RTL_W16(TCR, RTL_R16(TCR) & ~WMAC_TCR_ERRSTEN2 );
}
#endif



#ifdef RTLWIFINIC_GPIO_CONTROL
struct rtl8192cd_priv *root_priv;

void RTLWIFINIC_GPIO_init_priv(struct rtl8192cd_priv *priv)
{
	root_priv = priv;
}

void RTLWIFINIC_GPIO_config(unsigned int gpio_num, unsigned int direction)
{
	struct rtl8192cd_priv *priv = root_priv;
	if(!root_priv)
		return;

#ifdef PCIE_POWER_SAVING
	PCIeWakeUp(priv, POWER_DOWN_T0);
#endif

	if ((gpio_num >= 0) && (gpio_num <= 7)) {
		priv->pshare->phw->GPIO_dir[gpio_num] = direction;

		if (direction == 0x01) {
			RTL_W32(GPIO_PIN_CTRL, RTL_R32(GPIO_PIN_CTRL) & ~(BIT(gpio_num+24) | BIT(gpio_num+16)));
			return;
		}
		else if (direction == 0x10) {
			RTL_W32(GPIO_PIN_CTRL, (RTL_R32(GPIO_PIN_CTRL) & ~BIT(gpio_num+24)) | (BIT(gpio_num+16) | BIT(gpio_num+8)));
			return;
		}
	}
	if ((gpio_num >= 8) && (gpio_num <= 11)) {
		priv->pshare->phw->GPIO_dir[gpio_num] = direction;

		if (direction == 0x01) {
			RTL_W32(GPIO_MUXCFG, RTL_R32(GPIO_MUXCFG) & ~(BIT(gpio_num+20) | BIT(gpio_num+16)));
			return;
		}
		else if (direction == 0x10) {
			RTL_W32(GPIO_MUXCFG, (RTL_R32(GPIO_MUXCFG) & ~BIT(gpio_num+20)) | (BIT(gpio_num+16) | BIT(gpio_num+12)));
			return;
		}
	}


	panic_printk("GPIO %d action %d not support!\n", gpio_num, direction);
	return;
}

void RTLWIFINIC_GPIO_write(unsigned int gpio_num, unsigned int value)
{
	struct rtl8192cd_priv *priv = root_priv;
	if(!root_priv)
		return;
#ifdef PCIE_POWER_SAVING
		PCIeWakeUp(priv, POWER_DOWN_T0);
#endif	

	if (((gpio_num >= 0) && (gpio_num <= 7)) && (priv->pshare->phw->GPIO_dir[gpio_num] == 0x10)) {
		if (value)
			RTL_W32(GPIO_PIN_CTRL, RTL_R32(GPIO_PIN_CTRL) & ~BIT(gpio_num+8));
		else
			RTL_W32(GPIO_PIN_CTRL, RTL_R32(GPIO_PIN_CTRL) | BIT(gpio_num+8));
		return;
	}
	if (((gpio_num >= 8) && (gpio_num <= 11)) && (priv->pshare->phw->GPIO_dir[gpio_num] == 0x10)) {
		if (value)
			RTL_W32(GPIO_MUXCFG, RTL_R32(GPIO_MUXCFG) & ~BIT(gpio_num+12));
		else
			RTL_W32(GPIO_MUXCFG, RTL_R32(GPIO_MUXCFG) | BIT(gpio_num+12));
		return;
	}

	panic_printk("GPIO %d set value %d not support!\n", gpio_num, value);
	return;
}


int RTLWIFINIC_GPIO_read(unsigned int gpio_num)
{
	struct rtl8192cd_priv *priv = root_priv;
	unsigned int val32;
	if(!root_priv)
		return -1;

	if (((gpio_num >= 0) && (gpio_num <= 7)) && (priv->pshare->phw->GPIO_dir[gpio_num] == 0x01)) {
#ifdef PCIE_POWER_SAVING
		if ((priv->pwr_state == L2) || (priv->pwr_state == L1)) 
			val32 = priv->pshare->phw->GPIO_cache[0];
		else
#endif			
			val32 = RTL_R32(GPIO_PIN_CTRL);
		if (val32 & BIT(gpio_num))
			return 0;
		else
			return 1;
	}
	if (((gpio_num >= 8) && (gpio_num <= 11)) && (priv->pshare->phw->GPIO_dir[gpio_num] == 0x01)) {
#ifdef PCIE_POWER_SAVING
		if ((priv->pwr_state == L2) || (priv->pwr_state == L1)) 
			val32 = priv->pshare->phw->GPIO_cache[1];
		else
#endif		
			val32 = RTL_R32(GPIO_MUXCFG);
		if (val32 & BIT(gpio_num+8))
			return 0;
		else
			return 1;
	}

	panic_printk("GPIO %d get value not support!\n", gpio_num);
	return -1;
}
#endif

#ifdef CONFIG_WLAN_HAL
// TODO: move into HAL
BOOLEAN 
compareAvailableTXBD(
        struct rtl8192cd_priv   *priv, 
        unsigned int            num, 
        unsigned int            qNum, 
        int                     compareFlag
)
{
    PHCI_TX_DMA_MANAGER_88XX    ptx_dma = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PTxDMA88XX);
    unsigned int                halQnum = GET_HAL_INTERFACE(priv)->MappingTxQueueHandler(priv, qNum);
    unsigned long               avail_txbd_flag;
    
    SAVE_INT_AND_CLI(avail_txbd_flag);

    if (compareFlag == 1) {
        if (ptx_dma->tx_queue[halQnum].avail_txbd_num > num)  {
            RESTORE_INT(avail_txbd_flag);
            return TRUE;
        } else {        
            RESTORE_INT(avail_txbd_flag);        
            return FALSE;
        }        
    } else if (compareFlag == 2){
        if (ptx_dma->tx_queue[halQnum].avail_txbd_num < num)  {
            RESTORE_INT(avail_txbd_flag);            
            return TRUE;
        } else {        
            RESTORE_INT(avail_txbd_flag);        
            return FALSE;
        }    
    } else {
        printk("%s(%d): Error setting !!! \n", __FUNCTION__, __LINE__);
    }    
    
    RESTORE_INT(avail_txbd_flag);

    return FALSE;
}

#endif // CONFIG_WLAN_HAL

