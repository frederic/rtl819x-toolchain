/*
 *  MP routines
 *
 *  $Id: 8192cd_mp.c,v 1.25.2.8 2010/11/24 12:17:18 button Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_MP_C_

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/circ_buf.h>
#endif

#include "./8192cd_cfg.h"

#ifndef __KERNEL__
#ifdef __ECOS
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#else
#include "./sys-support.h"
#endif
#endif

#include "./8192cd_headers.h"
#include "./8192cd_tx.h"
#include "./8192cd_debug.h"

#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
#include <asm/mips16_lib.h>
#endif

#ifdef MP_TEST

#define POWER_MIN_CHECK(a,b)			(((a) > (b)) ? (b) : (a))
#define POWER_RANGE_CHECK(val)		(((val) > 0x3f)? 0x3f : ((val < 0) ? 0 : val))

#ifdef _MP_TELNET_SUPPORT_
extern void mp_pty_write_monitor(int en);
extern int mp_pty_is_hit(void);
extern int mp_pty_write(const unsigned char *buf, int count);
void mp_printk(const char *fmt, ...)
{
	va_list args;
	int r;
	int mp_printed_len;
	static char mp_printk_buf[1024];

	va_start(args, fmt);
	r = vprintk(fmt, args);
	va_end(args);

	va_start(args, fmt);
	mp_printed_len = vscnprintf(mp_printk_buf, sizeof(mp_printk_buf), fmt, args);
	va_end(args);
	mp_pty_write( mp_printk_buf, mp_printed_len );

	return r;
}
#define printk mp_printk
#else
#if defined(CONFIG_PANIC_PRINTK)
#define printk panic_printk
#endif
#endif //_MP_TELNET_SUPPORT_

#ifdef _LITTLE_ENDIAN_
typedef struct _R_ANTENNA_SELECT_OFDM {
	unsigned int		r_tx_antenna:4;
	unsigned int		r_ant_l:4;
	unsigned int		r_ant_non_ht:4;
	unsigned int		r_ant_ht1:4;
	unsigned int		r_ant_ht2:4;
	unsigned int		r_ant_ht_s1:4;
	unsigned int		r_ant_non_ht_s1:4;
	unsigned int		OFDM_TXSC:2;
	unsigned int		Reserved:2;
} R_ANTENNA_SELECT_OFDM;

typedef struct _R_ANTENNA_SELECT_CCK {
	unsigned char		r_cckrx_enable_2:2;
	unsigned char		r_cckrx_enable:2;
	unsigned char		r_ccktx_enable:4;
} R_ANTENNA_SELECT_CCK;

#else // _BIG_ENDIAN_

typedef struct _R_ANTENNA_SELECT_OFDM {
	unsigned int		Reserved:2;
	unsigned int		OFDM_TXSC:2;
	unsigned int		r_ant_non_ht_s1:4;
	unsigned int		r_ant_ht_s1:4;
	unsigned int		r_ant_ht2:4;
	unsigned int		r_ant_ht1:4;
	unsigned int		r_ant_non_ht:4;
	unsigned int		r_ant_l:4;
	unsigned int		r_tx_antenna:4;
} R_ANTENNA_SELECT_OFDM;

typedef struct _R_ANTENNA_SELECT_CCK {
	unsigned char		r_ccktx_enable:4;
	unsigned char		r_cckrx_enable:2;
	unsigned char		r_cckrx_enable_2:2;
} R_ANTENNA_SELECT_CCK;
#endif


extern int PHYCheckIsLegalRfPath8192cPci(struct rtl8192cd_priv *priv, unsigned int eRFPath);
static void mp_chk_sw_ant(struct rtl8192cd_priv *priv);


extern unsigned int TxPwrTrk_OFDM_SwingTbl[TxPwrTrk_OFDM_SwingTbl_Len];
extern unsigned char TxPwrTrk_CCK_SwingTbl[TxPwrTrk_CCK_SwingTbl_Len][8];
extern unsigned char TxPwrTrk_CCK_SwingTbl_CH14[TxPwrTrk_CCK_SwingTbl_Len][8];


#ifdef CONFIG_RTL8671
#define _GIMR_			0xb9c03010
#define _UART_RBR_		0xb9c00000
#define _UART_LSR_		0xb9c00014
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
#ifdef NOT_RTK_BSP
#include "bspchip.h"
#else
#include <bsp/bspchip.h>
#endif
#else
	#include <asm/rtl865x/platform.h>
#endif
#endif

#ifdef CONFIG_RTL8672
#ifdef USE_RLX_BSP
#ifdef CONFIG_RTL8686
	#define _GIMR_				BSP_GIMR1_0
	#else
	#define _GIMR_				BSP_GIMR
	#endif
#else
	#define _GIMR_				GIMR
#endif
#define _ICU_UART_MSK_			_UART_IE
#define _UART_RBR_				_UART_RBR 
#define _UART_LSR_				_UART_LSR
#else
#if defined(__LINUX_2_6__)
#ifdef CONFIG_RTL_8198B
#define _GIMR_				BSP_GIMR0_0
#else
#define _GIMR_				BSP_GIMR
#endif
#define _ICU_UART0_MSK_		BSP_UART0_IE
#define _UART0_RBR_			BSP_UART0_RBR
#define _UART0_LSR_			BSP_UART0_LSR
#else
#define _GIMR_				GIMR
#define _ICU_UART0_MSK_		UART0_IE
#define _UART0_RBR_			UART0_RBR
#define _UART0_LSR_			UART0_LSR
#endif
#endif
#endif

#ifdef B2B_TEST
#define MP_PACKET_HEADER		("wlan-tx-test")
#define MP_PACKET_HEADER_LEN	12
#endif


#ifdef CONFIG_RTL8671
#define RTL_UART_W16(reg, val16) writew ((val16), (unsigned int)((reg)))
#define RTL_UART_R16(reg) readw ((unsigned int)((reg)))
#define RTL_UART_R32(reg) readl ((unsigned int)((reg)))

#define DISABLE_UART0_INT() \
	do { \
		RTL_UART_W16(_GIMR_, RTL_UART_R16(_GIMR_) & ~_ICU_UART0_MSK_); \
		RTL_UART_R32(_UART_RBR_); \
		RTL_UART_R32(_UART_RBR_); \
	} while (0)

#define RESTORE_UART0_INT() \
	do { \
		RTL_UART_W16(_GIMR_, RTL_UART_R16(_GIMR_) | _ICU_UART0_MSK_); \
	} while (0)

static inline int IS_KEYBRD_HIT(void)
{
	if (RTL_UART_R32(_UART_LSR_) & 0x01000000) { // data ready
		RTL_UART_R32(_UART_RBR_);	 // clear rx FIFO
		return 1;
	}
	return 0;
}
#endif // CONFIG_RTL8671

#ifndef CONFIG_RTL_92C_SUPPORT
#define 	VERSION_8188C  0x1000
#define	VERSION_8192C  0x1001
#endif	
#ifndef CONFIG_RTL_92D_SUPPORT
#define VERSION_8192D 0x1002
#endif

#define CHECKICIS92C() ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C))
#define CHECKICIS92D()  (GET_CHIP_VER(priv)==VERSION_8192D)


#ifdef USE_RTL8186_SDK
#ifdef __LINUX_2_6__
#define RTL_UART_R8(reg)		readb((unsigned char *)reg)
#define RTL_UART_R32(reg)		readl((unsigned char *)reg)
#define RTL_UART_W8(reg, val)	writeb(val, (unsigned char *)reg)
#define RTL_UART_W32(reg, val)	writel(val, (unsigned char *)reg)
#else
#define RTL_UART_R8(reg)		readb((unsigned int)reg)
#define RTL_UART_R32(reg)		readl((unsigned int)reg)
#define RTL_UART_W8(reg, val)	writeb(val, (unsigned int)reg)
#define RTL_UART_W32(reg, val)	writel(val, (unsigned int)reg)
#endif
#ifdef CONFIG_RTL8672
#define DISABLE_UART0_INT() \
	do { \
		RTL_UART_W32(_GIMR_, RTL_UART_R32(_GIMR_) & ~_ICU_UART_MSK_); \
		RTL_UART_R8(_UART_RBR_); \
		RTL_UART_R8(_UART_RBR_); \
	} while (0)

#define RESTORE_UART0_INT() \
	do { \
		RTL_UART_W32(_GIMR_, RTL_UART_R32(_GIMR_) | _ICU_UART_MSK_); \
	} while (0)

static inline int IS_KEYBRD_HIT(void)
{
	if (RTL_UART_R8(_UART_LSR_) & 1) { // data ready
		RTL_UART_R8(_UART_RBR_);	 // clear rx FIFO
		return 1;
	}
	return 0;
}
#else
#define DISABLE_UART0_INT() \
	do { \
		RTL_UART_W32(_GIMR_, RTL_UART_R32(_GIMR_) & ~_ICU_UART0_MSK_); \
		RTL_UART_R8(_UART0_RBR_); \
		RTL_UART_R8(_UART0_RBR_); \
	} while (0)

#define RESTORE_UART0_INT() \
	do { \
		RTL_UART_W32(_GIMR_, RTL_UART_R32(_GIMR_) | _ICU_UART0_MSK_); \
	} while (0)

static inline int IS_KEYBRD_HIT(void)
{
	if (RTL_UART_R8(_UART0_LSR_) & 1) { // data ready
		RTL_UART_R8(_UART0_RBR_);	 // clear rx FIFO
		return 1;
	}
	return 0;
}

#endif
#endif // USE_RTL8186_SDK


/*
 *  find a token in a string. If succes, return pointer of token next. If fail, return null
 */
char *get_value_by_token(char *data, char *token)
{
		int idx=0, src_len=strlen(data), token_len=strlen(token);

		while (src_len >= token_len) {
			if (!memcmp(&data[idx], token, token_len))
				return (&data[idx+token_len]);
			src_len--;
			idx++;
		}
		return NULL;
}


unsigned char * get_bssid_mp(unsigned char *pframe)
{
	unsigned char 	*bssid;
	unsigned int	to_fr_ds	= (GetToDs(pframe) << 1) | GetFrDs(pframe);

	switch (to_fr_ds) {
		case 0x00:	// ToDs=0, FromDs=0
			bssid = GetAddr3Ptr(pframe);
			break;
		case 0x01:	// ToDs=0, FromDs=1
			bssid = GetAddr2Ptr(pframe);
			break;
		case 0x02:	// ToDs=1, FromDs=0
			bssid = GetAddr1Ptr(pframe);
			break;
		default:	// ToDs=1, FromDs=1
			bssid = GetAddr2Ptr(pframe);
			break;
	}

	return bssid;
}


static __inline__ int isLegalRate(unsigned int rate)
{
	unsigned int res = 0;

	switch(rate)
	{
	case _1M_RATE_:
	case _2M_RATE_:
	case _5M_RATE_:
	case _6M_RATE_:
	case _9M_RATE_:
	case _11M_RATE_:
	case _12M_RATE_:
	case _18M_RATE_:
	case _24M_RATE_:
	case _36M_RATE_:
	case _48M_RATE_:
	case _54M_RATE_:
		res = 1;
		break;
	case _MCS0_RATE_:
	case _MCS1_RATE_:
	case _MCS2_RATE_:
	case _MCS3_RATE_:
	case _MCS4_RATE_:
	case _MCS5_RATE_:
	case _MCS6_RATE_:
	case _MCS7_RATE_:
	case _MCS8_RATE_:
	case _MCS9_RATE_:
	case _MCS10_RATE_:
	case _MCS11_RATE_:
	case _MCS12_RATE_:
	case _MCS13_RATE_:
	case _MCS14_RATE_:
	case _MCS15_RATE_:
		res = 1;
		break;
	default:
		res = 0;
		break;
	}

	return res;
}

#ifdef MP_PSD_SUPPORT
int GetPSDData(struct rtl8192cd_priv *priv,unsigned int point)
{
	int psd_val;

	psd_val = RTL_R32(0x808);
	psd_val &= 0xFFBFFC00;
	psd_val |= point;

	RTL_W32(0x808, psd_val);
	delay_ms(2);
	psd_val |= 0x00400000;

	RTL_W32(0x808, psd_val);
	delay_ms(2);
	psd_val = RTL_R32(0x8B4);

	psd_val &= 0x0000FFFF;
	return psd_val;
}
#endif
#if 0
static void mp_RL5975e_Txsetting(struct rtl8192cd_priv *priv)
{
	RF92CD_RADIO_PATH_E eRFPath;
	unsigned int rfReg0x14, rfReg0x15, rfReg0x2c;

	// reg0x14
	rfReg0x14 = 0x5ab;
	if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20)
	{
		//channel = 1, 11, in 20MHz mode, set RF-reg[0x14] = 0x59b
		if(priv->pshare->working_channel == 1 || priv->pshare->working_channel == 11)
		{
			if(!is_CCK_rate(priv->pshare->mp_datarate)) //OFDM, MCS rates
				rfReg0x14 = 0x59b;
		}
	}
	else
	{
		//channel = 3, 9, in 40MHz mode, set RF-reg[0x14] = 0x59b
		if(priv->pshare->working_channel == 3 || priv->pshare->working_channel == 9)
			rfReg0x14 = 0x59b;
	}
	for (eRFPath = RF92CD_PATH_A; eRFPath<priv->pshare->phw->NumTotalRFPath; eRFPath++)
	{
		if (!PHYCheckIsLegalRfPath8192cPci(priv, eRFPath))
			continue;
/*
		if (get_rf_mimo_mode(priv) == MIMO_1T2R) {
			if ((eRFPath == RF92CD_PATH_A) || (eRFPath == RF92CD_PATH_B))
				continue;
		}
		else if (get_rf_mimo_mode(priv) == MIMO_2T2R) {
			if ((eRFPath == RF92CD_PATH_B) || (eRFPath == RF90_PATH_D))
				continue;
		}
*/
		PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)eRFPath, 0x14, bMask20Bits, rfReg0x14);
		delay_us(100);
	}

	// reg0x15
	rfReg0x15 = 0xf80;
	if(priv->pshare->mp_datarate == 4)
	{
		if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20)
			rfReg0x15 = 0xfc0;
	}
	for (eRFPath = RF92CD_PATH_A; eRFPath<priv->pshare->phw->NumTotalRFPath; eRFPath++)
	{
		if (!PHYCheckIsLegalRfPath8192cPci(priv, eRFPath))
			continue;
/*
		if (get_rf_mimo_mode(priv) == MIMO_1T2R) {
			if ((eRFPath == RF92CD_PATH_A) || (eRFPath == RF92CD_PATH_B))
				continue;
		}
		else if (get_rf_mimo_mode(priv) == MIMO_2T2R) {
			if ((eRFPath == RF92CD_PATH_B) || (eRFPath == RF90_PATH_D))
				continue;
		}
*/
		PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)eRFPath, 0x15, bMask20Bits, rfReg0x15);
		delay_us(100);
	}

	//reg0x2c
	if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20)
	{
		rfReg0x2c = 0x3d7;
		if(is_CCK_rate(priv->pshare->mp_datarate)) //CCK rate
		{
			if(priv->pshare->working_channel == 1 || priv->pshare->working_channel == 11)
				rfReg0x2c = 0x3f7;
		}
	}
	else
	{
		rfReg0x2c = 0x3ff;
	}
	for (eRFPath = RF92CD_PATH_A; eRFPath<priv->pshare->phw->NumTotalRFPath; eRFPath++)
	{
		if (!PHYCheckIsLegalRfPath8192cPci(priv, eRFPath))
			continue;
/*
		if (get_rf_mimo_mode(priv) == MIMO_1T2R) {
			if ((eRFPath == RF92CD_PATH_A) || (eRFPath == RF92CD_PATH_B))
				continue;
		}
		else if (get_rf_mimo_mode(priv) == MIMO_2T2R) {
			if ((eRFPath == RF92CD_PATH_B) || (eRFPath == RF90_PATH_D))
				continue;
		}
*/
		PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)eRFPath, 0x2c, bMask20Bits, rfReg0x2c);
		delay_us(100);
	}

	if (priv->pshare->rf_ft_var.use_frq_2_3G)
		PHY_SetRFReg(priv, RF90_PATH_C, 0x2c, 0x60, 0);
}


static void mp_RF_RxLPFsetting(struct rtl8192cd_priv *priv)
{
	unsigned int rfBand_A=0, rfBand_B=0, rfBand_C=0, rfBand_D=0;

	//==================================================
	//because the EVM issue, CCK ACPR spec, asked by bryant.
	//when BAND_20MHZ_MODE, should overwrite CCK Rx path RF, let the bandwidth
	//from 10M->8M, we should overwrite the following values to the cck rx rf.
	//RF_Reg[0xb]:bit[11:8] = 0x4, otherwise RF_Reg[0xb]:bit[11:8] = 0x0
	switch(priv->pshare->mp_antenna_rx)
	{
	case ANTENNA_A:
	case ANTENNA_AC:
	case ANTENNA_ABCD:
		rfBand_A = 0x500; //for TxEVM, CCK ACPR
		break;
	case ANTENNA_B:
	case ANTENNA_BD:
		rfBand_B = 0x500; //for TxEVM, CCK ACPR
		break;
	case ANTENNA_C:
	case ANTENNA_CD:
		rfBand_C = 0x500; //for TxEVM, CCK ACPR
		break;
	case ANTENNA_D:
		rfBand_D = 0x500; //for TxEVM, CCK ACPR
		break;
	}

	if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20)
	{
		if(!rfBand_A)
			rfBand_A = 0x100;
		if(!rfBand_B)
			rfBand_B = 0x100;
		if(!rfBand_C)
			rfBand_C = 0x100;
		if(!rfBand_D)
			rfBand_D = 0x100;
	}
	else
	{
		rfBand_A = 0x300;
		rfBand_B = 0x300;
		rfBand_C = 0x300;
		rfBand_D = 0x300;
	}

	PHY_SetRFReg(priv, RF92CD_PATH_A, 0x0b, bMask20Bits, rfBand_A);
	delay_us(100);
	PHY_SetRFReg(priv, RF92CD_PATH_B, 0x0b, bMask20Bits, rfBand_B);
	delay_us(100);
/*
	PHY_SetRFReg(priv, RF90_PATH_C, 0x0b, bMask20Bits, rfBand_C);
	delay_us(100);
	PHY_SetRFReg(priv, RF90_PATH_D, 0x0b, bMask20Bits, rfBand_D);
	delay_us(100);
*/
}
#endif


static void mp_8192CD_tx_setting(struct rtl8192cd_priv *priv)
{
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
	unsigned int odd_pwr = 0;
	extern void set_CCK_swing_index(struct rtl8192cd_priv*, short );

	if (is_CCK_rate(priv->pshare->mp_datarate) 
		&& (
#ifdef CONFIG_RTL_92C_SUPPORT
		(!IS_TEST_CHIP(priv) && (GET_CHIP_VER(priv) == VERSION_8192C)) 
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
#ifdef CONFIG_RTL_92C_SUPPORT
		|| 
#endif
		(GET_CHIP_VER(priv) == VERSION_8192D)
#endif
		)) {
		if (RTL_R8(0xa07) & 0x80) {
			if (priv->pshare->mp_txpwr_patha % 2)
				odd_pwr++;
		} else {
			if (priv->pshare->mp_txpwr_pathb % 2)
				odd_pwr++;
		}
		
		if(CHECKICIS92C()||(CHECKICIS92D()&&(RTL_R8(0xa07) & 0x40))) {
			switch((odd_pwr<<1)| priv->pshare->mp_cck_txpwr_odd) {
			case 1:
				set_CCK_swing_index(priv, get_CCK_swing_index(priv)+1);
				break;
			case 2:
				set_CCK_swing_index(priv, get_CCK_swing_index(priv)-1);
				break;
			default:
				break;
			};
		}
		priv->pshare->mp_cck_txpwr_odd = odd_pwr;
	}
#endif
}


static void mpt_StartCckContTx(struct rtl8192cd_priv *priv)
{
	unsigned int cckrate;

	// 1. if CCK block on?
	if (!PHY_QueryBBReg(priv, rFPGA0_RFMOD, bCCKEn))
		PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, bEnable);//set CCK block on

	//Turn Off All Test Mode
	PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMContinueTx, bDisable);
	PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleCarrier, bDisable);
	PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleTone, bDisable);
	//Set CCK Tx Test Rate
	switch (priv->pshare->mp_datarate)
	{
		case 2:
			cckrate = 0;
			break;
		case 4:
			cckrate = 1;
			break;
		case 11:
			cckrate = 2;
			break;
		case 22:
			cckrate = 3;
			break;
		default:
			cckrate = 0;
			break;
	}
	PHY_SetBBReg(priv, rCCK0_System, bCCKTxRate, cckrate);

	PHY_SetBBReg(priv, rCCK0_System, bCCKBBMode, 0x2);    //transmit mode
	PHY_SetBBReg(priv, rCCK0_System, bCCKScramble, 0x1);  //turn on scramble setting

#if 1//def CONFIG_RTL8672
	// Commented out for background mode, sync with SD7, 2010-07-08 by Annie ---
	// We will set 0x820 and 0x828 under Tx mode in mp_ctx(), 2010-09-17 by Family.
	//	PHY_SetBBReg(priv, 0x820, 0x400, 0x1);
	//	PHY_SetBBReg(priv, 0x828, 0x400, 0x1);
	//---
#else //CONFIG_RTL8672
	PHY_SetBBReg(priv, 0x820, 0x400, 0x1);
	PHY_SetBBReg(priv, 0x828, 0x400, 0x1);
#endif //CONFIG_RTL8672

}


static void mpt_StopCckCoNtTx(struct rtl8192cd_priv *priv)
{
	PHY_SetBBReg(priv, rCCK0_System, bCCKBBMode, 0x0);    //normal mode
	PHY_SetBBReg(priv, rCCK0_System, bCCKScramble, 0x1);  //turn on scramble setting

	PHY_SetBBReg(priv, 0x820, 0x400, 0x0);
	PHY_SetBBReg(priv, 0x828, 0x400, 0x0);
}


static void mpt_StartOfdmContTx(struct rtl8192cd_priv *priv)
{
	unsigned int go=0;

	// 1. if OFDM block on?
	if (!PHY_QueryBBReg(priv, rFPGA0_RFMOD, bOFDMEn))
		PHY_SetBBReg(priv, rFPGA0_RFMOD, bOFDMEn, bEnable);//set OFDM block on

	if (CHECKICIS92D())
	{
		if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G)
			go=1;
		else
			go=0;
	}
	else
		go=1;

	if (go==1)
	{
		// 2. set CCK test mode off, set to CCK normal mode
		PHY_SetBBReg(priv, rCCK0_System, bCCKBBMode, bDisable);

		// 3. turn on scramble setting
		PHY_SetBBReg(priv, rCCK0_System, bCCKScramble, bEnable);
	}

	// 4. Turn On Continue Tx and turn off the other test modes.
	PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMContinueTx, bEnable);
	PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleCarrier, bDisable);
	PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleTone, bDisable);


#if 1//def CONFIG_RTL8672
	// Commented out for background mode, sync with SD7, 2010-07-08 by Annie ---
	// We will set 0x820 and 0x828 under Tx mode in mp_ctx(), 2010-09-17 by Family.
	//	PHY_SetBBReg(priv, 0x820, 0x400, 0x1);
	//	PHY_SetBBReg(priv, 0x828, 0x400, 0x1);
	//---
#else //CONFIG_RTL8672
	PHY_SetBBReg(priv, 0x820, 0x400, 0x1);
	PHY_SetBBReg(priv, 0x828, 0x400, 0x1);
#endif //CONFIG_RTL8672

}


static void mpt_StopOfdmContTx(struct rtl8192cd_priv *priv)
{
	PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMContinueTx, bDisable);
	PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleCarrier, bDisable);
	PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleTone, bDisable);
	//Delay 10 ms
	delay_ms(10);

	PHY_SetBBReg(priv, 0x820, 0x400, 0x0);
	PHY_SetBBReg(priv, 0x828, 0x400, 0x0);
}


static void mpt_ProSetCarrierSupp(struct rtl8192cd_priv *priv, int enable)
{
	if (enable)
	{ // Start Carrier Suppression.
		// 1. if CCK block on?
		if(!PHY_QueryBBReg(priv, rFPGA0_RFMOD, bCCKEn))
			PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, bEnable);//set CCK block on

		//Turn Off All Test Mode
		PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMContinueTx, bDisable);
		PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleCarrier, bDisable);
		PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleTone, bDisable);

		PHY_SetBBReg(priv, rCCK0_System, bCCKBBMode, 0x2);    //transmit mode
		PHY_SetBBReg(priv, rCCK0_System, bCCKScramble, 0x0);  //turn off scramble setting
   		//Set CCK Tx Test Rate
		//PHY_SetBBReg(pAdapter, rCCK0_System, bCCKTxRate, pMgntInfo->ForcedDataRate);
		PHY_SetBBReg(priv, rCCK0_System, bCCKTxRate, 0x0);    //Set FTxRate to 1Mbps
	}
	else
	{ // Stop Carrier Suppression.
		PHY_SetBBReg(priv, rCCK0_System, bCCKBBMode, 0x0);    //normal mode
		PHY_SetBBReg(priv, rCCK0_System, bCCKScramble, 0x1);  //turn on scramble setting

		//BB Reset
/*
		PHY_SetBBReg(priv, rPMAC_Reset, bBBResetB, 0x0);
		PHY_SetBBReg(priv, rPMAC_Reset, bBBResetB, 0x1);
*/
	}
}


/*
 * start mp testing. stop beacon and change to mp mode
 */
void mp_start_test(struct rtl8192cd_priv *priv)
{
	unsigned int go=0;

	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return;
	}

	if (OPMODE & WIFI_MP_STATE)
	{
		printk("\nFail: already in MP mode\n");
		return;
	}

#ifdef MP_SWITCH_LNA
	priv->pshare->rx_packet_ss_a = 0;
	priv->pshare->rx_packet_ss_b = 0;
#endif

#ifdef TEMP_MP_92D
	RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_NOLINK & NETYPE_Mask) << NETYPE_SHIFT));

	// disable beacon
	RTL_W8(TXPAUSE, STOP_BCN);

	priv->pmib->dot11StationConfigEntry.autoRate = 0;
	priv->pmib->dot11StationConfigEntry.protectionDisabled = 1;
	priv->pmib->dot11ErpInfo.ctsToSelf = 0;
	priv->pmib->dot11ErpInfo.protection = 0;
	priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 0;
	OPMODE |= WIFI_MP_STATE;
	OPMODE &= ~WIFI_STATION_STATE;
	OPMODE |= WIFI_AP_STATE;

	// stop site survey
	if (timer_pending(&priv->ss_timer))
		del_timer_sync(&priv->ss_timer);

	// stop receiving packets
	RTL_W32(RCR, RTL_R32(RCR) & ~(RCR_AB | RCR_AM | RCR_APM | RCR_AAP));

	priv->mp_start = 1;
#else

#ifdef UNIVERSAL_REPEATER
	if (IS_VXD_INTERFACE(priv)) {
		printk("\nFail: only root interface supports MP mode\n");
		return;
	}
	else if (IS_ROOT_INTERFACE(priv) && IS_DRV_OPEN(GET_VXD_PRIV(priv)))
		rtl8192cd_close(GET_VXD_PRIV(priv)->dev);
#endif

#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable &&
		IS_VAP_INTERFACE(priv)) {
		printk("\nFail: only root interface supports MP mode\n");
		return;
	} else if (IS_ROOT_INTERFACE(priv)) {
		int i;
		if (priv->pmib->miscEntry.vap_enable) {
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
				if (IS_DRV_OPEN(priv->pvap_priv[i]))
					rtl8192cd_close(priv->pvap_priv[i]->dev);
			}
		}
	}
#endif

	// initialize rate to 54M (or 1M ?)
	priv->pshare->mp_datarate = _54M_RATE_;

	// initialize antenna
	priv->pshare->mp_antenna_tx = ANTENNA_A;
	priv->pshare->mp_antenna_rx = ANTENNA_A;
	mp_chk_sw_ant(priv);
	
	if ((get_rf_mimo_mode(priv) == MIMO_2T2R) && priv->pmib->dot11RFEntry.txbf) {
			// Tx Path Selection by ctrl_reg in MP mode
			PHY_SetBBReg(priv, 0x90C, BIT(30), 0);
		}

	// initialize swing index
	{
		unsigned int regval, i;

		regval = PHY_QueryBBReg(priv, rOFDM0_XATxIQImbalance, bMaskDWord);
		for (i=0; i<TxPwrTrk_OFDM_SwingTbl_Len; i++) {
			if (regval == TxPwrTrk_OFDM_SwingTbl[i]) {
				priv->pshare->mp_ofdm_swing_idx = (unsigned char)i;
				break;
			}
		}

		if (CHECKICIS92D())
		{
			if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G)
				go=1;
			else
				go=0;
		}
		else
			go=1;

		if (go==1)		
				priv->pshare->mp_cck_swing_idx = get_cck_swing_idx(priv->pshare->CurrentChannelBW, priv->pshare->mp_ofdm_swing_idx);
	}

	// change mode to mp mode
	OPMODE |= WIFI_MP_STATE;

	// disable beacon
	RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_NOLINK & NETYPE_Mask) << NETYPE_SHIFT));
	RTL_W8(TXPAUSE, STOP_BCN);

	priv->pmib->dot11StationConfigEntry.autoRate = 0;
	priv->pmib->dot11StationConfigEntry.protectionDisabled = 1;
	priv->pmib->dot11ErpInfo.ctsToSelf = 0;
	priv->pmib->dot11ErpInfo.protection = 0;
	priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 0;
	OPMODE &= ~WIFI_STATION_STATE;
	OPMODE |= WIFI_AP_STATE;

	// stop site survey
	if (timer_pending(&priv->ss_timer))
		del_timer_sync(&priv->ss_timer);

	// stop receiving packets
	RTL_W32(RCR, RTL_R32(RCR) & ~(RCR_AB | RCR_AM | RCR_APM | RCR_AAP));
	// Global setting for MP no ack CCK
	RTL_W8(0x700, 0xe0);

	// stop dynamic mechanism
//	if ((get_rf_mimo_mode(priv) == MIMO_2T4R) && (priv->pmib->dot11BssType.net_work_type != WIRELESS_11B))
//		rx_path_by_rssi(priv, NULL, FALSE);
//	tx_power_control(priv, NULL, FALSE);

	// DIG off and set initial gain
	priv->pshare->rf_ft_var.dig_enable = 0;
	set_DIG_state(priv, 0);
	delay_ms(1);

#if defined( HIGH_POWER_EXT_PA) && defined(CONFIG_RTL_92C_SUPPORT)

//_TXPWR_REDEFINE ?? shall also apply to Normal Driver ??
	if ((priv->pshare->rf_ft_var.use_ext_pa) && 
		((GET_CHIP_VER(priv)==VERSION_8192C)  || (GET_CHIP_VER(priv)==VERSION_8188E))) {
		RTL_W8(0xc50, 0x2e);
		RTL_W8(0xc58, 0x2e);
	} else
#endif
	if (priv->pshare->rf_ft_var.use_ext_lna) {
		RTL_W8(0xc50, 0x2a);
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
			RTL_W8(0xc58, 0x2a);
#endif
	}
#ifdef CONFIG_RTL_92D_SUPPORT
	else if (GET_CHIP_VER(priv)==VERSION_8192D) {
		if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
			RTL_W8(0xc50, 0x1c);
			RTL_W8(0xc58, 0x1c);
		} else {
			RTL_W8(0xc50, 0x20);
			RTL_W8(0xc58, 0x20);
		}
	}
#endif


	RTL_W8(0xa0a, 0x83);

	priv->pshare->rf_ft_var.tpt_period=5;

	mp_8192CD_tx_setting(priv);

#ifdef GREEN_HILL
	printk("Enter testing mode\n");
#else
	printk("\nUsage:\n");
	printk("  iwpriv wlanx mp_stop\n");
	printk("  iwpriv wlanx mp_rate {2-108,128-143}\n");
#ifdef CONFIG_RTL_92D_SUPPORT
	printk("  iwpriv wlanx mp_channel 1\n");
	printk("        - if bg band, use channel 1-14 only\n");
	printk("        - if a band, 20M mode use channel 36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140,149,153,157,161,165 only\n");
	printk("        - if a band, 40M mode use channel 38,42,46,50,54,58,62,102,106,110,114,118,122,126,130,134,138,151,152,155,159,160,163 only\n");
#else
	printk("  iwpriv wlanx mp_channel {1-14}\n");
#endif
	printk("  iwpriv wlanx mp_bandwidth [40M={0|1},shortGI={0|1}]\n");
	printk("        - default: 40M=0, shortGI=0\n");
	printk("  iwpriv wlanx mp_txpower [patha=x,pathb=y]\n");
	printk("        - if no parameters, driver will set tx power according to flash setting.\n");
	printk("  iwpriv wlanx mp_phypara xcap=x\n");
	printk("  iwpriv wlanx mp_bssid 001122334455\n");
	printk("  iwpriv wlanx mp_ant_tx {a,b,ab}\n");
	printk("  iwpriv wlanx mp_ant_rx {a,b,ab}\n");
	printk("  iwpriv wlanx mp_arx {start,stop}\n");
	printk("  iwpriv wlanx mp_ctx [time=t,count=n,background,stop,pkt,cs,stone,scr]\n");
	printk("        - if \"time\" is set, tx in t sec. if \"count\" is set, tx with n packet.\n");
	printk("        - if \"background\", it will tx continuously until \"stop\" is issued.\n");
	printk("        - if \"pkt\", send cck packet in packet mode (not h/w).\n");
	printk("        - if \"cs\", send cck packet with carrier suppression.\n");
	printk("        - if \"stone\", send packet in single-tone.\n");
	printk("        - default: tx infinitely (no background).\n");
	printk("  iwpriv wlanx mp_query\n");
#ifdef MP_PSD_SUPPORT
	printk("  iwpriv wlanx mp_psd\n"); 
#endif
	printk("  iwpriv wlanx mp_ther\n");
	printk("  iwpriv wlanx mp_pwrtrk [ther={7-29}, stop]\n");
#ifdef CONFIG_RTL_92D_SUPPORT
	printk("  iwpriv wlanx mp_phyband {a, bg}\n");
#endif
	printk("  iwpriv wlanx mp_reset_stats\n");
	printk("        - to reset tx and rx count\n");
	printk("  iwpriv wlanx mp_get_pwr\n");

#ifdef B2B_TEST
	printk("  iwpriv wlanx mp_tx [da=xxxxxx,time=n,count=n,len=n,retry=n,err=n]\n");
	printk("        - if \"time\" is set, tx in t sec. if \"count\" is set, tx with n packet.\n");
	printk("        - if \"time=-1\", tx infinitely.\n");
	printk("        - If \"err=1\", display statistics when tx err.\n");
 	printk("        - default: da=ffffffffffff, time=0, count=1000, len=1500,\n");
 	printk("              retry=6(mac retry limit), err=1.\n");
#if 0
	printk("  iwpriv wlanx mp_rx [ra=xxxxxx,quiet=t,interval=n]\n");
	printk("        - ra: rx mac. defulat is burn-in mac\n");
	printk("        - quiet_time: quit rx if no rx packet during quiet_time. default is 5s\n");
	printk("        - interval: report rx statistics periodically in sec.\n");
	printk("              default is 0 (no report).\n");
#endif
	printk("  iwpriv wlanx mp_brx {start[,ra=xxxxxx],stop}\n");
	printk("        - start: start rx immediately.\n");
 	printk("        - ra: rx mac. defulat is burn-in mac.\n");
	printk("        - stop: stop rx immediately.\n");
#endif
#endif // GREEN_HILL
#endif
}


/*
 * stop mp testing. recover system to previous status.
 */
void mp_stop_test(struct rtl8192cd_priv *priv)
{
	if (!netif_running(priv->dev))	{
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE))	{
		printk("Fail: not in MP mode\n");
		return;
	}

#if 1//def CONFIG_RTL8672
	// make sure mp_ctx action stop, otherwise it will cause memory leak(skb_pool_ptr) for linux SDK
	//	 ,or crash dump for OSK because free skb_pool using rtl_kfree_skb()
	if (OPMODE & WIFI_MP_CTX_BACKGROUND) {
		mp_ctx(priv, "stop");
	}
#endif

	// enable beacon
	RTL_W8(TXPAUSE, RTL_R8(TXPAUSE) & ~STOP_BCN);
	OPMODE &= ~WIFI_MP_STATE;

	priv->pshare->ThermalValue = 0;

	printk("Please restart the interface\n");
}


/*
 * set data rate
 */
void mp_set_datarate(struct rtl8192cd_priv *priv, unsigned char *data)
{
	unsigned char rate, rate_org;
	char tmpbuf[32];
#if 0
	RF92CD_RADIO_PATH_E eRFPath;
#endif

	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return;
	}
	if (OPMODE & WIFI_MP_CTX_BACKGROUND)
	{
		printk("Fail: In MP background mode, please stop and retry it again\n");
		return;
	}

	rate = _atoi((char *)data, 10);
#ifdef CONFIG_RTL_92D_SUPPORT
	if(CHECKICIS92D()) {
		if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
			if (is_CCK_rate(rate)) {
				printk("(%d/2) Mbps data rate is not supported in A band\n", rate);
				return;
			}
		}
	}
#endif

	if(!isLegalRate(rate))
	{
		printk("(%d/2) Mbps data rate may not be supported\n", rate);
		return;
	}

	rate_org = priv->pshare->mp_datarate;
	priv->pshare->mp_datarate = rate;
#if 0	
	for (eRFPath = RF92CD_PATH_A; eRFPath<priv->pshare->phw->NumTotalRFPath; eRFPath++) {
		if (!PHYCheckIsLegalRfPath8192cPci(priv, eRFPath))
			continue;
		if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20){
#ifdef CONFIG_RTL_92D_SUPPORT
		if(CHECKICIS92D()){
			priv->pshare->RegRF18[eRFPath] &= (~BIT(11));
			priv->pshare->RegRF18[eRFPath] |= BIT(10);
		}
#endif
#ifdef CONFIG_RTL_92C_SUPPORT
		if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)){
			PHY_SetRFReg(priv, eRFPath, rRfChannel, (BIT(11)|BIT(10)), 0x1);
		}
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8188E){
			PHY_SetRFReg(priv, eRFPath, rRfChannel, (BIT(11)|BIT(10)), 0x3);
		}
#endif
		}else{
#ifdef CONFIG_RTL_92D_SUPPORT
			if(CHECKICIS92D()){
				priv->pshare->RegRF18[eRFPath] &= (~BIT(11));
				priv->pshare->RegRF18[eRFPath] &= (~BIT(10));
			} else
#endif
				PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)eRFPath, rRfChannel, (BIT(11)|BIT(10)), 0x00);
		}
#ifdef CONFIG_RTL_92D_SUPPORT
		if(CHECKICIS92D()){
			PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)eRFPath, rRfChannel, bMask20Bits, priv->pshare->RegRF18[eRFPath]);
		}
#endif		
		delay_us(100);
	}
#endif	
/*
	if(CHECKICIS92C()) {
		if (is_CCK_rate(priv->pshare->mp_datarate)) {
			PHY_SetRFReg(priv, 0, 0x26, bMask20Bits, 0x0f400);
		} else {
			if(IS_UMC_A_CUT_88C(priv) || GET_CHIP_VER(priv) == VERSION_8192C ) 
				PHY_SetRFReg(priv, 0, 0x26, bMask20Bits, 0x4f000);
			else
				PHY_SetRFReg(priv, 0, 0x26, bMask20Bits, 0x4f200);
		}
	}
*/
	mp_8192CD_tx_setting(priv);

	if (rate <= 108)
		sprintf(tmpbuf, "Set data rate to %d Mbps\n", rate/2);
	else
		sprintf(tmpbuf, "Set data rate to MCS %d\n", rate&0x7f);
	printk(tmpbuf);
}


/*
 * set channel
 */
void mp_set_channel(struct rtl8192cd_priv *priv, unsigned char *data)
{
	unsigned char channel, channel_org;
	char tmpbuf[48];
	unsigned int eRFPath;

	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return;
	}

	channel = (unsigned char)_atoi((char *)data, 10);

#ifdef CONFIG_RTL_92D_SUPPORT
	unsigned int go=0;
	if(CHECKICIS92D())		
	{	if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G)
	          go=1;
		else go=0; 			
	}
	else go=1;
	if(go==1)	
#endif
	{
		if (priv->pshare->is_40m_bw &&
			((channel < 3) || (channel > 12))) {
			sprintf(tmpbuf, "channel %d is invalid\n", channel);
			printk(tmpbuf);
			return;
		}
	}

	channel_org = priv->pmib->dot11RFEntry.dot11channel;
	priv->pmib->dot11RFEntry.dot11channel = channel;

	if (priv->pshare->rf_ft_var.use_frq_2_3G)
		channel += 14;

	{
		unsigned int val_read;
		unsigned int val= channel;

		val_read = PHY_QueryRFReg(priv, 0, 0x18, bMask20Bits, 1);
	if(CHECKICIS92D())
		val_read &= 0xffffff00;
	else
		val_read &= 0xfffffff0;

		for(eRFPath = RF92CD_PATH_A; eRFPath < RF92CD_PATH_MAX; eRFPath++) {
#ifdef CONFIG_RTL_92D_SUPPORT
			if(CHECKICIS92D()) {
				priv->pshare->RegRF18[eRFPath] = (val_read | val);
				
				if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
					/*
					 *	Set Bit18 when channel >= 100, for 5G only
					 */
					if (val >= 100)
						//PHY_SetRFReg(priv, eRFPath, rRfChannel, BIT(18), 1);
						priv->pshare->RegRF18[eRFPath] |= BIT(18);
					else
						//PHY_SetRFReg(priv, eRFPath, rRfChannel, BIT(18), 0);
						priv->pshare->RegRF18[eRFPath] &= (~BIT(18));

					//PHY_SetRFReg(priv, eRFPath, rRfChannel, BIT(16), 1);
					//PHY_SetRFReg(priv, eRFPath, rRfChannel, BIT(8), 1);
					priv->pshare->RegRF18[eRFPath] |= BIT(16);
					priv->pshare->RegRF18[eRFPath] |= BIT(8);
					// CLOAD for RF paht_A/B (MP-chip)
					if (val < 149)
						PHY_SetRFReg(priv, eRFPath, 0xB, BIT(16)|BIT(15)|BIT(14), 0x7);
					else
						PHY_SetRFReg(priv, eRFPath, 0xB, BIT(16)|BIT(15)|BIT(14), 0x2);
				} else {
					//PHY_SetRFReg(priv, eRFPath, rRfChannel, BIT(16), 0);
					//PHY_SetRFReg(priv, eRFPath, rRfChannel, BIT(8), 0);
					priv->pshare->RegRF18[eRFPath] &= (~BIT(16));
					priv->pshare->RegRF18[eRFPath] &= (~BIT(8));
					
					// CLOAD for RF paht_A/B (MP-chip)
					PHY_SetRFReg(priv, eRFPath, 0xB, BIT(16)|BIT(15)|BIT(14), 0x7);
				}
				PHY_SetRFReg(priv, eRFPath, 0x18, bMask20Bits, priv->pshare->RegRF18[eRFPath]);
			}else
#endif
			{
				PHY_SetRFReg(priv, eRFPath, 0x18, bMask20Bits, val_read | val);
			}
		}

		channel = val;
	}


#ifdef CONFIG_RTL_88E_SUPPORT 
	if(GET_CHIP_VER(priv) == VERSION_8188E){
		unsigned int val_read;
		val_read = PHY_QueryRFReg(priv, 0, 0x18, bMask20Bits, 1);

		if(priv->pshare->CurrentChannelBW == 1)
		{
			val_read |= BIT(10);
			val_read &= (~BIT(11));
		}
		else
		{
			val_read |= BIT(10);
			val_read |= BIT(11);
		}

		PHY_SetRFReg(priv, 0, 0x18, bMask20Bits, val_read);
	}
#endif


#ifdef CONFIG_RTL_92D_SUPPORT
	if(CHECKICIS92D())
	{
		reload_txpwr_pg(priv);

#ifdef USB_POWER_SUPPORT
//_TXPWR_REDEFINE
	{
			int i;
		for (i=8; i<=15; i++){
			priv->pshare->phw->OFDMTxAgcOffset_A[i] = 0;
			priv->pshare->phw->OFDMTxAgcOffset_B[i] = 0;
			}

		}
#endif

		SetSYN_para(priv,channel);
#ifdef SW_LCK_92D
		phy_ReloadLCKSetting(priv);
#endif
		SetIMR_n(priv, channel);

		Update92DRFbyChannel(priv, channel);
		
		PHY_IQCalibrate(priv);
#ifdef DPK_92D		
		if (priv->pmib->dot11RFEntry.phyBandSelect==PHY_BAND_5G && priv->pshare->rf_ft_var.dpk_on)
			PHY_DPCalibrate(priv);
#endif
	}
#endif

//#ifndef CONFIG_RTL_92D_SUPPORT
	if(CHECKICIS92C()) {
		if (priv->pshare->rf_ft_var.use_frq_2_3G)
			channel -= 14;
	}
//#endif

	priv->pshare->working_channel = channel;

	//CCK Shaping Filter
	if(priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G)
		set_CCK_swing_index(priv, 12);
	
//	mp_8192CD_tx_setting(priv);

#ifdef CONFIG_RTL_92C_SUPPORT
	if(IS_UMC_B_CUT_88C(priv)) {
		if(channel==6)
			RTL_W8(0xc50, 0x22);
		else
			RTL_W8(0xc50, 0x20);
	}
#endif

#ifdef TXPWR_LMT
	if (!priv->pshare->rf_ft_var.disable_txpwrlmt){
		int i;
		int max_idx;
		
		find_pwr_limit(priv);

		if (!priv->pshare->txpwr_lmt_OFDM || !priv->pshare->tgpwr_OFDM){
			DEBUG_INFO("No limit for OFDM TxPower\n");
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
			DEBUG_INFO("No limit for HT1S TxPower\n");
			max_idx=255;
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
			DEBUG_INFO("No limit for HT2S TxPower\n");
			max_idx=255;
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

		if (!priv->pshare->txpwr_lmt_CCK || !priv->pshare->tgpwr_CCK){
			DEBUG_INFO("No limit for CCK TxPower\n");
			max_idx=255;
		}else{
			// maximum additional power index 
			max_idx = (priv->pshare->txpwr_lmt_CCK - priv->pshare->tgpwr_CCK);
		}

		for (i=0; i<=3; i++) {
			priv->pshare->phw->CCKTxAgc_A[i] = POWER_MIN_CHECK(priv->pshare->phw->CCKTxAgc_A[i], max_idx);
			priv->pshare->phw->CCKTxAgc_A[i] = POWER_MIN_CHECK(priv->pshare->phw->CCKTxAgc_A[i], max_idx);
			//printk("priv->pshare->phw->CCKTxAgc_A[%d]=%x\n",i, priv->pshare->phw->CCKTxAgc_A[i]);
			//printk("priv->pshare->phw->CCKTxAgc_A[%d]=%x\n",i, priv->pshare->phw->CCKTxAgc_A[i]);
		}
	}
#endif

	sprintf(tmpbuf, "Change channel %d to channel %d\n", channel_org, channel);
	printk(tmpbuf);
}


/*
 * set tx power
 */
void mp_set_tx_power(struct rtl8192cd_priv *priv, unsigned char *data)
{
	unsigned int channel = priv->pmib->dot11RFEntry.dot11channel;
	char *val, tmpbuf[64];
	unsigned int writeVal;
	char baseA,baseB, byte[4];
	int i;

	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return;
	}

	if (strlen(data) == 0) {
		priv->pshare->mp_txpwr_patha = priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[channel-1];
		priv->pshare->mp_txpwr_pathb = priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[channel-1];
	} else {
		val = get_value_by_token((char *)data, "patha=");
		if (val) {
			priv->pshare->mp_txpwr_patha = _atoi(val, 10);
		}

		val = get_value_by_token((char *)data, "pathb=");
		if (val) {
			priv->pshare->mp_txpwr_pathb = _atoi(val, 10);
		}
	}
/*
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		if(priv->pshare->mp_txpwr_patha > HP_OFDM_POWER_MAX)
			priv->pshare->mp_txpwr_patha = HP_OFDM_POWER_MAX;
		if(priv->pshare->mp_txpwr_pathb > HP_OFDM_POWER_MAX)
			priv->pshare->mp_txpwr_pathb = HP_OFDM_POWER_MAX;		

		sprintf(tmpbuf, "Set OFDM power level path_A:%d path_B:%d\n",
			priv->pshare->mp_txpwr_patha, priv->pshare->mp_txpwr_pathb);
		printk(tmpbuf);

	}
#endif
*/

#if defined(CALIBRATE_BY_ODM) && defined(CONFIG_RTL_88E_SUPPORT)
	if (GET_CHIP_VER(priv) == VERSION_8188E) {
	PHY_RF6052SetCCKTxPower(priv, *(ODMPTR->pChannel));
	PHY_RF6052SetOFDMTxPower(priv, *(ODMPTR->pChannel));
	} else
#endif	
	{
	baseA = priv->pshare->mp_txpwr_patha;
	baseB = priv->pshare->mp_txpwr_pathb;

	/**************path-A**************/
	// 18M ~ 6M
	for (i=0; i<4; i++) {
		if (priv->pshare->rf_ft_var.pwr_by_rate)
			byte[i] = POWER_RANGE_CHECK(baseA + priv->pshare->phw->OFDMTxAgcOffset_A[i]);
		else
			byte[i] = baseA;
	}
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {	
		for (i=0; i<4; i++)
			byte[i] = POWER_MIN_CHECK(byte[i], HP_OFDM_POWER_MAX);
	}
#endif
	writeVal = (byte[0]<<24) | (byte[1]<<16) |(byte[2]<<8) | byte[3];
	RTL_W32(rTxAGC_A_Rate18_06, writeVal);

	// 54M ~ 24M
	for (i=0; i<4; i++) {
		if (priv->pshare->rf_ft_var.pwr_by_rate)
			byte[i] = POWER_RANGE_CHECK(baseA + priv->pshare->phw->OFDMTxAgcOffset_A[i+4]);
		else
			byte[i] = baseA;
	}
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {	
		for (i=0; i<4; i++)
			byte[i] = POWER_MIN_CHECK(byte[i], HP_OFDM_POWER_MAX);
	}
#endif
	writeVal = (byte[0]<<24) | (byte[1]<<16) |(byte[2]<<8) | byte[3];
	RTL_W32(rTxAGC_A_Rate54_24, writeVal);

	// MCS3 ~ MCS0
	for (i=0; i<4; i++) {
		if (priv->pshare->rf_ft_var.pwr_by_rate)
			byte[i] = POWER_RANGE_CHECK(baseA + priv->pshare->phw->MCSTxAgcOffset_A[i]);
		else
			byte[i] = baseA;
	}
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {	
		for (i=0; i<4; i++)
			byte[i] = POWER_MIN_CHECK(byte[i], HP_OFDM_POWER_MAX);
	}
#endif
	writeVal = (byte[0]<<24) | (byte[1]<<16) |(byte[2]<<8) | byte[3];
	RTL_W32(rTxAGC_A_Mcs03_Mcs00, writeVal);

	// MCS7 ~ MCS4
	for (i=0; i<4; i++) {
		if (priv->pshare->rf_ft_var.pwr_by_rate)
			byte[i] = POWER_RANGE_CHECK(baseA + priv->pshare->phw->MCSTxAgcOffset_A[i+4]);
		else
			byte[i] = baseA;
	}
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {	
		for (i=0; i<4; i++)
			byte[i] = POWER_MIN_CHECK(byte[i], HP_OFDM_POWER_MAX);
	}
#endif
	writeVal = (byte[0]<<24) | (byte[1]<<16) |(byte[2]<<8) | byte[3];
	RTL_W32(rTxAGC_A_Mcs07_Mcs04, writeVal);

	// MCS11 ~ MCS8
	for (i=0; i<4; i++) {
		if (priv->pshare->rf_ft_var.pwr_by_rate)
		{
			//_TXPWR_REDEFINE ?? #if 0 in FOX 
			byte[i] = POWER_RANGE_CHECK(baseA + priv->pshare->phw->MCSTxAgcOffset_A[i+8]);
		}
		else
			byte[i] = baseA;
	}
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {	
		for (i=0; i<4; i++)
			byte[i] = POWER_MIN_CHECK(byte[i], HP_OFDM_POWER_MAX);
	}
#endif
	writeVal = (byte[0]<<24) | (byte[1]<<16) |(byte[2]<<8) | byte[3];
	RTL_W32(rTxAGC_A_Mcs11_Mcs08, writeVal);

	// MCS15 ~ MCS12
	for (i=0; i<4; i++) {
		if (priv->pshare->rf_ft_var.pwr_by_rate)
		{
			//_TXPWR_REDEFINE ?? #if 0 in FOX 
			byte[i] = POWER_RANGE_CHECK(baseA + priv->pshare->phw->MCSTxAgcOffset_A[i+12]);
		}
		else
			byte[i] = baseA;
	}
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {	
		for (i=0; i<4; i++)
			byte[i] = POWER_MIN_CHECK(byte[i], HP_OFDM_POWER_MAX);
	}
#endif
	writeVal = (byte[0]<<24) | (byte[1]<<16) |(byte[2]<<8) | byte[3];
	RTL_W32(rTxAGC_A_Mcs15_Mcs12, writeVal);


	/**************path-B**************/
	// 18M ~ 6M
	for (i=0; i<4; i++) {
		if (priv->pshare->rf_ft_var.pwr_by_rate)
			byte[i] = POWER_RANGE_CHECK(baseB + priv->pshare->phw->OFDMTxAgcOffset_B[i]);
		else
			byte[i] = baseB;
	}
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {	
		for (i=0; i<4; i++)
			byte[i] = POWER_MIN_CHECK(byte[i], HP_OFDM_POWER_MAX);
	}
#endif
	writeVal = (byte[0]<<24) | (byte[1]<<16) |(byte[2]<<8) | byte[3];
	RTL_W32(rTxAGC_B_Rate18_06, writeVal);

	// 54M ~ 24M
	for (i=0; i<4; i++) {
		if (priv->pshare->rf_ft_var.pwr_by_rate)
			byte[i] = POWER_RANGE_CHECK(baseB + priv->pshare->phw->OFDMTxAgcOffset_B[i+4]);
		else
			byte[i] = baseB;
	}
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {	
		for (i=0; i<4; i++)
			byte[i] = POWER_MIN_CHECK(byte[i], HP_OFDM_POWER_MAX);
	}
#endif
	writeVal = (byte[0]<<24) | (byte[1]<<16) |(byte[2]<<8) | byte[3];
	RTL_W32(rTxAGC_B_Rate54_24, writeVal);

	// MCS3 ~ MCS0
	for (i=0; i<4; i++) {
		if (priv->pshare->rf_ft_var.pwr_by_rate)
			byte[i] = POWER_RANGE_CHECK(baseB + priv->pshare->phw->MCSTxAgcOffset_B[i]);
		else
			byte[i] = baseB;
	}
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {	
		for (i=0; i<4; i++)
			byte[i] = POWER_MIN_CHECK(byte[i], HP_OFDM_POWER_MAX);
	}
#endif
	writeVal = (byte[0]<<24) | (byte[1]<<16) |(byte[2]<<8) | byte[3];
	RTL_W32(rTxAGC_B_Mcs03_Mcs00, writeVal);

	// MCS7 ~ MCS4
	for (i=0; i<4; i++) {
		if (priv->pshare->rf_ft_var.pwr_by_rate)
			byte[i] = POWER_RANGE_CHECK(baseB + priv->pshare->phw->MCSTxAgcOffset_B[i+4]);
		else
			byte[i] = baseB;
	}
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {	
		for (i=0; i<4; i++)
			byte[i] = POWER_MIN_CHECK(byte[i], HP_OFDM_POWER_MAX);
	}
#endif
	writeVal = (byte[0]<<24) | (byte[1]<<16) |(byte[2]<<8) | byte[3];
	RTL_W32(rTxAGC_B_Mcs07_Mcs04, writeVal);

	// MCS11 ~ MCS8
	for (i=0; i<4; i++) {
		if (priv->pshare->rf_ft_var.pwr_by_rate)
		{
			//_TXPWR_REDEFINE ?? #if 0 in FOX 
			byte[i] = POWER_RANGE_CHECK(baseB + priv->pshare->phw->MCSTxAgcOffset_B[i+8]);
		}
		else
			byte[i] = baseB;
	}
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {	
		for (i=0; i<4; i++)
			byte[i] = POWER_MIN_CHECK(byte[i], HP_OFDM_POWER_MAX);
	}
#endif
	writeVal = (byte[0]<<24) | (byte[1]<<16) |(byte[2]<<8) | byte[3];
	RTL_W32(rTxAGC_B_Mcs11_Mcs08, writeVal);

	// MCS15 ~ MCS12
	for (i=0; i<4; i++) {
		if (priv->pshare->rf_ft_var.pwr_by_rate)
		{
			//_TXPWR_REDEFINE ?? #if 0 in FOX 
			byte[i] = POWER_RANGE_CHECK(baseB + priv->pshare->phw->MCSTxAgcOffset_B[i+12]);
		}
		else
			byte[i] = baseB;
	}
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {	
		for (i=0; i<4; i++)
			byte[i] = POWER_MIN_CHECK(byte[i], HP_OFDM_POWER_MAX);
	}
#endif
	writeVal = (byte[0]<<24) | (byte[1]<<16) |(byte[2]<<8) | byte[3];
	RTL_W32(rTxAGC_B_Mcs15_Mcs12, writeVal);

/*
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		if(priv->pshare->mp_txpwr_patha > HP_CCK_POWER_MAX)
			priv->pshare->mp_txpwr_patha = HP_CCK_POWER_MAX;
		if(priv->pshare->mp_txpwr_pathb > HP_CCK_POWER_MAX)
			priv->pshare->mp_txpwr_pathb = HP_CCK_POWER_MAX;	

		sprintf(tmpbuf, "Set CCK power level path_A:%d path_B:%d\n",
			priv->pshare->mp_txpwr_patha, priv->pshare->mp_txpwr_pathb);
		printk(tmpbuf);		
	}
#endif
*/

	// CCK-A 1M
	if (priv->pshare->rf_ft_var.pwr_by_rate)
		writeVal = POWER_RANGE_CHECK(baseA + priv->pshare->phw->CCKTxAgc_A[3]);
	else
		writeVal = baseA;
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {	
		writeVal = POWER_MIN_CHECK(writeVal, HP_CCK_POWER_MAX);
	}
#endif
	PHY_SetBBReg(priv, rTxAGC_A_CCK1_Mcs32, 0x0000ff00, writeVal);

	// CCK-B 11M ~ 2M
	for (i=1; i<4; i++) {
		if (priv->pshare->rf_ft_var.pwr_by_rate)
			byte[i] = POWER_RANGE_CHECK(baseB + priv->pshare->phw->CCKTxAgc_B[i]);
		else
			byte[i] = baseB;
	}
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {	
		for (i=1; i<4; i++)
			byte[i] = POWER_MIN_CHECK(byte[i], HP_CCK_POWER_MAX);
	}
#endif
	writeVal = (byte[1]<<16) |(byte[2]<<8) | byte[3];
	PHY_SetBBReg(priv, rTxAGC_B_CCK5_1_Mcs32, 0xffffff00, writeVal);

	// CCK-A 11M ~ 2M CCK-B 11M
	for (i=1; i<4; i++) {
		if (priv->pshare->rf_ft_var.pwr_by_rate)
			byte[i] = POWER_RANGE_CHECK(baseA + priv->pshare->phw->CCKTxAgc_A[i-1]);
		else
			byte[i] = baseA;
	}
	if (priv->pshare->rf_ft_var.pwr_by_rate)
		byte[0] = POWER_RANGE_CHECK(baseB + priv->pshare->phw->CCKTxAgc_B[0]);
	else
		byte[0] = baseB;
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		for (i=0; i<4; i++)
			byte[i] = POWER_MIN_CHECK(byte[i], HP_CCK_POWER_MAX);
	}
#endif
	writeVal = (byte[1]<<24) | (byte[2]<<16) |(byte[3]<<8) | byte[0];
	PHY_SetBBReg(priv, rTxAGC_A_CCK11_2_B_CCK11, 0xffffffff, writeVal);

	mp_8192CD_tx_setting(priv);

#ifdef HIGH_POWER_EXT_PA
	if (!priv->pshare->rf_ft_var.use_ext_pa) 
#endif
	{
		sprintf(tmpbuf, "Set power level path_A:%d path_B:%d\n",
			priv->pshare->mp_txpwr_patha, priv->pshare->mp_txpwr_pathb);
		printk(tmpbuf);
	}

#ifdef CONFIG_RTL_92D_SUPPORT
#ifdef DPK_92D
	if ((GET_CHIP_VER(priv) == VERSION_8192D) && (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) ) {

		unsigned int tmp_txpwr_dpk_0 = 0, tmp_txpwr_dpk_1 = 0;

		tmp_txpwr_dpk_0 += (priv->pshare->TxPowerLevelDPK[0]);
		tmp_txpwr_dpk_0 += (priv->pshare->TxPowerLevelDPK[0]<<8);
		tmp_txpwr_dpk_0 += (priv->pshare->TxPowerLevelDPK[0]<<16);
		tmp_txpwr_dpk_0 += (priv->pshare->TxPowerLevelDPK[0]<<24);

		tmp_txpwr_dpk_1 += (priv->pshare->TxPowerLevelDPK[1]);
		tmp_txpwr_dpk_1 += (priv->pshare->TxPowerLevelDPK[1]<<8);
		tmp_txpwr_dpk_1 += (priv->pshare->TxPowerLevelDPK[1]<<16);
		tmp_txpwr_dpk_1 += (priv->pshare->TxPowerLevelDPK[1]<<24);
		
		priv->pshare->phw->power_backup[0x00] = RTL_R32(rTxAGC_A_Rate18_06);
		priv->pshare->phw->power_backup[0x01] = RTL_R32(rTxAGC_A_Rate54_24);
		priv->pshare->phw->power_backup[0x02] = RTL_R32(rTxAGC_B_Rate18_06);
		priv->pshare->phw->power_backup[0x03] = RTL_R32(rTxAGC_B_Rate54_24);
		priv->pshare->phw->power_backup[0x04] = RTL_R32(rTxAGC_A_Mcs03_Mcs00);
		priv->pshare->phw->power_backup[0x05] = RTL_R32(rTxAGC_A_Mcs07_Mcs04);
		priv->pshare->phw->power_backup[0x06] = RTL_R32(rTxAGC_A_Mcs11_Mcs08);
		priv->pshare->phw->power_backup[0x07] = RTL_R32(rTxAGC_A_Mcs15_Mcs12);
		priv->pshare->phw->power_backup[0x08] = RTL_R32(rTxAGC_B_Mcs03_Mcs00);
		priv->pshare->phw->power_backup[0x09] = RTL_R32(rTxAGC_B_Mcs07_Mcs04);
		priv->pshare->phw->power_backup[0x0a] = RTL_R32(rTxAGC_B_Mcs11_Mcs08);
		priv->pshare->phw->power_backup[0x0b] = RTL_R32(rTxAGC_B_Mcs15_Mcs12);

		RTL_W32(rTxAGC_A_Rate18_06, priv->pshare->phw->power_backup[0x00]	 +tmp_txpwr_dpk_0);
		RTL_W32(rTxAGC_A_Rate54_24, priv->pshare->phw->power_backup[0x01]	 +tmp_txpwr_dpk_0);
		RTL_W32(rTxAGC_B_Rate18_06, priv->pshare->phw->power_backup[0x02]	 +tmp_txpwr_dpk_1);
		RTL_W32(rTxAGC_B_Rate54_24, priv->pshare->phw->power_backup[0x03]	 +tmp_txpwr_dpk_1);
		RTL_W32(rTxAGC_A_Mcs03_Mcs00, priv->pshare->phw->power_backup[0x04]  +tmp_txpwr_dpk_0);
		RTL_W32(rTxAGC_A_Mcs07_Mcs04, priv->pshare->phw->power_backup[0x05]  +tmp_txpwr_dpk_0);
		RTL_W32(rTxAGC_A_Mcs11_Mcs08, priv->pshare->phw->power_backup[0x06]  +tmp_txpwr_dpk_0);
		RTL_W32(rTxAGC_A_Mcs15_Mcs12, priv->pshare->phw->power_backup[0x07]  +tmp_txpwr_dpk_0);
		RTL_W32(rTxAGC_B_Mcs03_Mcs00, priv->pshare->phw->power_backup[0x08]  +tmp_txpwr_dpk_1);
		RTL_W32(rTxAGC_B_Mcs07_Mcs04, priv->pshare->phw->power_backup[0x09]  +tmp_txpwr_dpk_1);
		RTL_W32(rTxAGC_B_Mcs11_Mcs08, priv->pshare->phw->power_backup[0x0a]  +tmp_txpwr_dpk_1);
		RTL_W32(rTxAGC_B_Mcs15_Mcs12, priv->pshare->phw->power_backup[0x0b]  +tmp_txpwr_dpk_1);
	}
#endif
#endif
}
}


/*
 * continuous tx
 *  command: "iwpriv wlanx mp_ctx [time=t,count=n,background,stop,pkt,cs,stone,scr]"
 *			  if "time" is set, tx in t sec. if "count" is set, tx with n packet
 *			  if "background", it will tx continuously until "stop" is issue
 *			  if "pkt", send cck packets with packet mode (not hardware)
 *			  if "cs", send cck packet with carrier suppression
 *			  if "stone", send packet in single-tone
 *			  default: tx infinitely (no background)
 */
void mp_ctx(struct rtl8192cd_priv *priv, unsigned char *data)
{
	unsigned int orgTCR = RTL_R32(TCR);
	unsigned char pbuf[6]={0xff,0xff,0xff,0xff,0xff,0xff};
	int payloadlen=1500, time=-1;
	struct sk_buff *skb;
	struct wlan_ethhdr_t *pethhdr;
	int len, i=0, q_num;
	unsigned char pattern;
	char *val;
#if 1//def CONFIG_RTL8672
	unsigned long flags2=0;
#endif
	unsigned long end_time=0;
	unsigned long flags=0;
	int tx_from_isr=0, background=0;
	struct rtl8192cd_hw *phw = GET_HW(priv);
	volatile unsigned int head, tail;
	RF92CD_RADIO_PATH_E eRFPath;

#ifdef CONFIG_RTL_92D_SUPPORT
	unsigned int temp_860,  temp_864, temp_870;
#endif
/*
// We need to turn off ADC before entering CTX mode
	RTL_W32(0xe70, (RTL_R32(0xe70) & 0xFE1FFFFF ) );
	delay_us(100);
*/
	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return;
	}

	// get count
	val = get_value_by_token((char *)data, "count=");
	if (val) {
		priv->pshare->mp_ctx_count = _atoi(val, 10);
		if (priv->pshare->mp_ctx_count)
			time = 0;
	}

	// get time
	val = get_value_by_token((char *)data, "time=");
	if (val) {
		if (!memcmp(val, "-1", 2))
			time = -1;
		else
			time = RTL_SECONDS_TO_JIFFIES(_atoi(val, 10));
		if (time > 0)
			end_time = jiffies + time;
	}

	// get background
	val = get_value_by_token((char *)data, "background");
	if (val)
		background = 1;

	// get carrier suppression mode
	val = get_value_by_token((char *)data, "cs");
	if (val) {
		if (!is_CCK_rate(priv->pshare->mp_datarate)) {
			printk("Specify carrier suppression but not CCK rate!\n");
			return;
		}
		else
			OPMODE |= WIFI_MP_CTX_CCK_CS;
	}

	// get single-tone
	val = get_value_by_token((char *)data, "stone");
	if (val)
		OPMODE |= WIFI_MP_CTX_ST;

	// get single-carrier
	val = get_value_by_token((char *)data, "scr");
	if (val) {
		if (is_CCK_rate(priv->pshare->mp_datarate)) {
			printk("Specify single carrier but CCK rate!\n");
			return;
		}
		else
			OPMODE |= WIFI_MP_CTX_SCR;
	}

	// get stop
	val = get_value_by_token((char *)data, "stop");
	if (val) {

		if (!(OPMODE & WIFI_MP_CTX_BACKGROUND)) {
			printk("Error! Continuous-Tx is not on-going.\n");
			return;
		}
		goto stop_tx;
	}


	// get tx-isr flag, which is set in ISR when Tx ok
	val = get_value_by_token((char *)data, "tx-isr");
	if (val) {
		if (OPMODE & WIFI_MP_CTX_BACKGROUND) {

#if 1//def CONFIG_RTL8672
			if ((OPMODE & WIFI_MP_CTX_ST) || (!(OPMODE & (WIFI_MP_CTX_PACKET|WIFI_MP_CTX_CCK_CS)) &&
					(priv->net_stats.tx_packets > 0)) )
				return;
#else //CONFIG_RTL8672
			if (OPMODE & WIFI_MP_CTX_OFDM_HW)
				return;
#endif //CONFIG_RTL8672

			tx_from_isr = 1;
			time = -1;
		}
	}

	if (!tx_from_isr && (OPMODE & WIFI_MP_CTX_BACKGROUND)) {
		printk("Continuous-Tx is on going. You can't issue any tx command except 'stop'.\n");
		return;
	}
	// get packet mode
		val = get_value_by_token((char *)data, "pkt");
		if (val)
			OPMODE |= WIFI_MP_CTX_PACKET;

	if (background) {
		priv->pshare->skb_pool_ptr = kmalloc(sizeof(struct sk_buff)*NUM_MP_SKB, GFP_KERNEL);
		if (priv->pshare->skb_pool_ptr == NULL) {
			printk("Allocate skb fail!\n");
			return;
		}
		memset(priv->pshare->skb_pool_ptr, 0, sizeof(struct sk_buff)*NUM_MP_SKB);
		for (i=0; i<NUM_MP_SKB; i++) {
			priv->pshare->skb_pool[i] = (struct sk_buff *)(priv->pshare->skb_pool_ptr + i * sizeof(struct sk_buff));
			priv->pshare->skb_pool[i]->head = kmalloc(RX_BUF_LEN, GFP_KERNEL);
			if (priv->pshare->skb_pool[i]->head == NULL) {
				for (i=0; i<NUM_MP_SKB; i++) {
					if (priv->pshare->skb_pool[i]->head)
						kfree(priv->pshare->skb_pool[i]->head);
					else
						break;
				}
				kfree(priv->pshare->skb_pool_ptr);
				printk("Allocate skb fail!\n");
				return;
			}
			else {
				priv->pshare->skb_pool[i]->data = priv->pshare->skb_pool[i]->head;
				priv->pshare->skb_pool[i]->tail = priv->pshare->skb_pool[i]->data;
				priv->pshare->skb_pool[i]->end = priv->pshare->skb_pool[i]->head + RX_BUF_LEN;
				priv->pshare->skb_pool[i]->len = 0;
			}
		}
		priv->pshare->skb_head = 0;
		priv->pshare->skb_tail = 0;

		/*disable interrupt and change OPMODE here to avoid re-enter*/
		SAVE_INT_AND_CLI(flags);

		head = get_txhead(phw, BE_QUEUE);
		tail = get_txtail(phw, BE_QUEUE);
		
		while (head != tail) {
			DEBUG_INFO("BEQ head/tail=%d/%d\n", head, tail);
			rtl8192cd_tx_dsr((unsigned long)priv);
			delay_us(50);
			tail = get_txtail(phw, BE_QUEUE);
		}
		OPMODE |= WIFI_MP_CTX_BACKGROUND;
		time = -1; // set as infinite
	}

	len = payloadlen + WLAN_ETHHDR_LEN;
	pattern = 0xAB;
	q_num = BE_QUEUE;

	if (!tx_from_isr) {
#ifdef GREEN_HILL
		printk("Start continuous TX");
#else
		if (time < 0) // infinite
			printk("Start continuous DA=%02x%02x%02x%02x%02x%02x len=%d infinite=yes",
				pbuf[0], pbuf[1], pbuf[2], pbuf[3], pbuf[4], pbuf[5], payloadlen);
			else if (time > 0) // by time
				printk("Start continuous DA=%02x%02x%02x%02x%02x%02x len=%d time=%ds",
					pbuf[0], pbuf[1], pbuf[2], pbuf[3], pbuf[4], pbuf[5],
					payloadlen, time/HZ);
			else // by count
				printk("Start TX DA=%02x%02x%02x%02x%02x%02x len=%d count=%d",
					pbuf[0], pbuf[1], pbuf[2], pbuf[3], pbuf[4], pbuf[5],
				payloadlen, priv->pshare->mp_ctx_count);

#if defined(USE_RTL8186_SDK)
		if (!background) {
	  		printk(", press any key to escape.\n");
		} else
#endif
	  	printk(".\n");
#endif // GREEN_HILL

		if (OPMODE & WIFI_MP_CTX_PACKET) {
			RTL_W16(TX_PTCL_CTRL, RTL_R16(TX_PTCL_CTRL) & ~DIS_CW);
			RTL_W32(EDCA_BE_PARA, (RTL_R32(EDCA_BE_PARA) & 0xffffff00) | (10 + 2 * 20));
		} else {
			RTL_W16(TX_PTCL_CTRL, RTL_R16(TX_PTCL_CTRL) | DIS_CW);
			RTL_W32(EDCA_BE_PARA, (RTL_R32(EDCA_BE_PARA) & 0xffffff00) | 0x01);

			if (is_CCK_rate(priv->pshare->mp_datarate)) {
				if (OPMODE & WIFI_MP_CTX_CCK_CS)
					mpt_ProSetCarrierSupp(priv, TRUE);
				else
					mpt_StartCckContTx(priv);
			} else {
				if (!((OPMODE & WIFI_MP_CTX_ST) && (OPMODE & WIFI_MP_CTX_SCR)))
					mpt_StartOfdmContTx(priv);
				OPMODE |= WIFI_MP_CTX_OFDM_HW;
			}
		}
		RTL_W32(RCR, RTL_R32(RCR) & ~(RCR_AB | RCR_AM | RCR_APM | RCR_AAP));

#if defined(USE_RTL8186_SDK)
		if (!background) {
	  		DISABLE_UART0_INT();
#ifdef _MP_TELNET_SUPPORT_
			mp_pty_write_monitor(1);
#endif //_MP_TELNET_SUPPORT_
		}
#endif

		memset(&priv->net_stats, 0,  sizeof(struct net_device_stats));
	}


        if (is_CCK_rate(priv->pshare->mp_datarate)) {
                PHY_SetRFReg(priv, 0, 0x26, bMask20Bits, 0x0f400);
        } else {
#ifdef CONFIG_RTL_92C_SUPPORT
		if( IS_UMC_A_CUT_88C(priv) || GET_CHIP_VER(priv) == VERSION_8192C) {
			PHY_SetRFReg(priv, 0, 0x26, bMask20Bits, 0x4f000);
		} else
#endif
		{
#ifdef CONFIG_RTL_92C_SUPPORT
			if( (IS_UMC_B_CUT_88C(priv) || GET_CHIP_VER(priv) == VERSION_8192C) 
				&& ((priv->pmib->dot11RFEntry.dot11channel == 4)||(priv->pmib->dot11RFEntry.dot11channel == 12)))
				PHY_SetRFReg(priv, 0, 0x26, bMask20Bits, 0x4f000);
			else
#endif
				PHY_SetRFReg(priv, 0, 0x26, bMask20Bits, 0x4f200);
		}
	}


	i = 0;
	while (1)
	{

#if defined(USE_RTL8186_SDK)
#ifdef _MP_TELNET_SUPPORT_
		if (!in_atomic() && !tx_from_isr && !background)
			schedule();
		if (!tx_from_isr && !background && (IS_KEYBRD_HIT()||mp_pty_is_hit()))
			break;
#else
		if (!tx_from_isr && !background && IS_KEYBRD_HIT())
			break;
#endif //_MP_TELNET_SUPPORT_
#endif

		if (time) {
			if (time != -1) {
				if (jiffies > end_time)
					break;
			}
			else {
				if ((priv->pshare->mp_ctx_count > 0) && (priv->pshare->mp_ctx_pkt >= priv->pshare->mp_ctx_count)) {
					if (background)
						RESTORE_INT(flags);
					delay_ms(10);
					return;
				}
			}
		}
		else {
			if (i >= priv->pshare->mp_ctx_count)
				break;
		}
		i++;
		priv->pshare->mp_ctx_pkt++;

		if ((OPMODE & WIFI_MP_CTX_ST) &&
			(i == 1)) {
			i++;

			{
				switch (priv->pshare->mp_antenna_tx) {
				case ANTENNA_B:
					eRFPath = RF92CD_PATH_B;
					break;
				case ANTENNA_A:
				default:
					eRFPath = RF92CD_PATH_A;
					break;
				}
			}

			// Start Single Tone.
			PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, 0x0);
			PHY_SetBBReg(priv, rFPGA0_RFMOD, bOFDMEn, 0x0);
#ifdef CONFIG_RTL_92D_SUPPORT // single tone 
			if (priv->pmib->dot11RFEntry.phyBandSelect==PHY_BAND_5G){
				temp_860 = PHY_QueryBBReg(priv, 0x860, bMaskDWord);
				temp_864 = PHY_QueryBBReg(priv, 0x864, bMaskDWord);
				temp_870 = PHY_QueryBBReg(priv, 0x870, bMaskDWord);
				
				if (eRFPath == RF92CD_PATH_A){
					PHY_SetBBReg(priv, 0x860, BIT(11), 0x1);
					PHY_SetBBReg(priv, 0x870, BIT(11), 0x1);
					PHY_SetBBReg(priv, 0x870, BIT(6)|BIT(5), 0x3);
				} else {
					PHY_SetBBReg(priv, 0x864, BIT(11), 0x1);
					PHY_SetBBReg(priv, 0x870, BIT(27), 0x1);
					PHY_SetBBReg(priv, 0x870, BIT(22)|BIT(21), 0x3);
				}
				PHY_SetRFReg(priv, eRFPath, 0x41, BIT(19), 1);
			} else
#endif
			{
				PHY_SetRFReg(priv, RF92CD_PATH_A, 0x21, bMask20Bits, 0xd4000);
			}
			delay_us(100);
#ifdef HIGH_POWER_EXT_PA
				PHY_SetRFReg(priv, eRFPath, 0x00, bMask20Bits, 0x20000); //From suggestion of BS (RF Team)
#else
				PHY_SetRFReg(priv, eRFPath, 0x00, bMask20Bits, 0x20010);
#endif
			delay_us(100);

#ifdef HIGH_POWER_EXT_PA
			if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)){
			if(priv->pshare->rf_ft_var.use_ext_pa) {
				PHY_SetBBReg(priv, 0x860, BIT(10), 0x1);
				PHY_SetBBReg(priv, 0x864, BIT(10), 0x1);
				PHY_SetBBReg(priv, 0x870, BIT(10), 0x1);
				PHY_SetBBReg(priv, 0x870, BIT(26), 0x1);
			}
			}
#endif

		}

		if ((OPMODE & WIFI_MP_CTX_SCR) &&
			(i == 1)) {

			// 1. if OFDM block on?
			if (!PHY_QueryBBReg(priv, rFPGA0_RFMOD, bOFDMEn))
				PHY_SetBBReg(priv, rFPGA0_RFMOD, bOFDMEn, bEnable);//set OFDM block on

			// 2. set CCK test mode off, set to CCK normal mode
			PHY_SetBBReg(priv, rCCK0_System, bCCKBBMode, bDisable);

			// 3. turn on scramble setting
			PHY_SetBBReg(priv, rCCK0_System, bCCKScramble, bEnable);

			// 4. Turn On Continue Tx and turn off the other test modes.
			PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMContinueTx, bDisable);
			PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleCarrier, bEnable);
			PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleTone, bEnable);
			PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMDisPwsavTx, bEnable);

			PHY_SetBBReg(priv, rOFDM1_TRxMesaure1, 0xfff, 0x404);
		}

		if ((OPMODE & WIFI_MP_CTX_OFDM_HW) && (i > 1)) {
			if (background) {
				RESTORE_INT(flags);
				return;
			} else {
				continue;
			}
		}

		if (background || tx_from_isr) {
			if (CIRC_SPACE(priv->pshare->skb_head, priv->pshare->skb_tail, NUM_MP_SKB) > 1) {
				skb = priv->pshare->skb_pool[priv->pshare->skb_head];
				priv->pshare->skb_head = (priv->pshare->skb_head + 1) & (NUM_MP_SKB - 1);
			} else {
				OPMODE |= WIFI_MP_CTX_BACKGROUND_PENDING;
				priv->pshare->mp_ctx_pkt--;
				if (background)
					RESTORE_INT(flags);
				return;
			}
		} else {
			skb = dev_alloc_skb(len);
		}

		if (skb != NULL) {
			DECLARE_TXINSN(txinsn);

			skb->dev = priv->dev;
			skb_put(skb, len);

			pethhdr = (struct wlan_ethhdr_t *)(skb->data);
			memcpy((void *)pethhdr->daddr, pbuf, MACADDRLEN);
			memcpy((void *)pethhdr->saddr, BSSID, MACADDRLEN);
			pethhdr->type = payloadlen;

			memset(skb->data+WLAN_ETHHDR_LEN, pattern, payloadlen);

			txinsn.q_num	= q_num; //using low queue for data queue
			txinsn.fr_type	= _SKB_FRAME_TYPE_;
			txinsn.pframe	= skb;
			skb->cb[1] = 0;
			
#ifdef MCAST2UI_REFINE
	        memcpy(&skb->cb[10], skb->data, 6);
#endif

			txinsn.tx_rate	= txinsn.lowest_tx_rate = priv->pshare->mp_datarate;
			txinsn.fixed_rate = 1;
			txinsn.retry	= 0;
			txinsn.phdr		= get_wlanllchdr_from_poll(priv);

			memset((void *)txinsn.phdr, 0, sizeof(struct wlanllc_hdr));
			SetFrDs(txinsn.phdr);
			SetFrameType(txinsn.phdr, WIFI_DATA);

			if(rtl8192cd_firetx(priv, &txinsn) == CONGESTED) 
			{
				//printk("Congested\n");
#if 1//def CONFIG_RTL8672 

#else //CONFIG_RTL8672
				if (tx_from_isr) {
					head = get_txhead(phw, BE_QUEUE);
					tail = get_txtail(phw, BE_QUEUE);
					if (head == tail) // if Q empty,invoke 1s-timer to send
						OPMODE |= (WIFI_MP_CTX_BACKGROUND | WIFI_MP_CTX_BACKGROUND_PENDING);
					return;
				}
#endif //CONFIG_RTL8672
				i--;
				priv->pshare->mp_ctx_pkt--;
				if (txinsn.phdr)
					release_wlanllchdr_to_poll(priv, txinsn.phdr);

				if (background || tx_from_isr) {
					skb->tail = skb->data = skb->head;
					skb->len = 0;
					priv->pshare->skb_head = (priv->pshare->skb_head + NUM_MP_SKB - 1) & (NUM_MP_SKB - 1);
				} else if (skb) {
					rtl_kfree_skb(priv, skb, _SKB_TX_);
				}


#if 1//def CONFIG_RTL8672
				if (tx_from_isr) {
					head = get_txhead(phw, BE_QUEUE);
					tail = get_txtail(phw, BE_QUEUE);
					if (head == tail) // if Q empty,invoke 1s-timer to send
						OPMODE |= (WIFI_MP_CTX_BACKGROUND | WIFI_MP_CTX_BACKGROUND_PENDING);
					return;
				} else {
					SAVE_INT_AND_CLI(flags2);
					rtl8192cd_tx_dsr((unsigned long)priv);
					RESTORE_INT(flags2);
				}

#else //CONFIG_RTL8672
				if (!tx_from_isr) {
					SAVE_INT_AND_CLI(flags);
#ifdef SMP_SYNC
					if (!priv->pshare->has_triggered_tx_tasklet) {
						tasklet_schedule(&priv->pshare->tx_tasklet);
						priv->pshare->has_triggered_tx_tasklet = 1;
					}
#else
					rtl8192cd_tx_dsr((unsigned long)priv);
#endif
					RESTORE_INT(flags);
				}
#endif //CONFIG_RTL8672
			}
#if 1//def CONFIG_RTL8672
			else if ((1 == priv->net_stats.tx_packets) &&
				((OPMODE & (WIFI_MP_CTX_PACKET| WIFI_MP_CTX_BACKGROUND| WIFI_MP_CTX_ST|
				WIFI_MP_CTX_CCK_CS))==WIFI_MP_CTX_BACKGROUND) )
			{
				#define CHECK_TX_MODE_CNT	40
				int check_cnt;
				
				// must sure RF is in TX mode before enabling TXAGC function.
				if ( priv->pshare->mp_antenna_tx & ANTENNA_A ) {
					check_cnt = 0;
					while ( (PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x00, 0x70000, 1) != 0x02) && (check_cnt < CHECK_TX_MODE_CNT) ) {
						delay_ms(1); ++check_cnt;
					}
					printk("RF(A) # of check tx mode = %d (%d)\n", check_cnt, tx_from_isr);
					PHY_SetBBReg(priv, 0x820, 0x400, 0x1);
				}
				
				if ( priv->pshare->mp_antenna_tx & ANTENNA_B ) {
					check_cnt = 0;
					while ( (PHY_QueryRFReg(priv, RF92CD_PATH_B, 0x00, 0x70000, 1) != 0x02) && (check_cnt < CHECK_TX_MODE_CNT) ) {
						delay_ms(1); ++check_cnt;
			}
					printk("RF(B) # of check tx mode = %d\n", check_cnt);
					PHY_SetBBReg(priv, 0x828, 0x400, 0x1);
				}

				if ( background ) {
					RESTORE_INT(flags);
				} else if (tx_from_isr) {
					OPMODE &= ~WIFI_MP_CTX_BACKGROUND_PENDING;
				}
				return;

				#undef CHECK_TX_MODE_CNT
			}
#endif
		}
		else 
		{
#ifdef CONFIG_RTL8672
			i--;
			priv->pshare->mp_ctx_pkt--;
#endif
			//printk("Can't allocate sk_buff\n");
			if (tx_from_isr) {
				head = get_txhead(phw, BE_QUEUE);
				tail = get_txtail(phw, BE_QUEUE);
				if (head == tail) // if Q empty,invoke 1s-timer to send
					OPMODE |= (WIFI_MP_CTX_BACKGROUND | WIFI_MP_CTX_BACKGROUND_PENDING);
				return;
			}
#if 1//def CONFIG_RTL8672	
			delay_ms(1);
			SAVE_INT_AND_CLI(flags2);
			rtl8192cd_tx_dsr((unsigned long)priv);
			RESTORE_INT(flags2);
#else //CONFIG_RTL8672
			i--;
			priv->pshare->mp_ctx_pkt--;
			delay_ms(1);
			SAVE_INT_AND_CLI(flags);
#endif //CONFIG_RTL8672
#ifdef SMP_SYNC
			if (!priv->pshare->has_triggered_tx_tasklet) {
				tasklet_schedule(&priv->pshare->tx_tasklet);
				priv->pshare->has_triggered_tx_tasklet = 1;
			}
#else
			rtl8192cd_tx_dsr((unsigned long)priv);
#endif
			RESTORE_INT(flags);
		}

		if ((background || tx_from_isr) && (i == CURRENT_NUM_TX_DESC/4)) {
			OPMODE &= ~WIFI_MP_CTX_BACKGROUND_PENDING;
			if (background)
				RESTORE_INT(flags);
			return;
		}
	}

#if defined(USE_RTL8186_SDK)
	if (!tx_from_isr && !background) {
		RESTORE_UART0_INT();
#ifdef _MP_TELNET_SUPPORT_
		mp_pty_write_monitor(0);
#endif //_MP_TELNET_SUPPORT_
	}
#endif

stop_tx:

	RTL_W32(TCR, orgTCR);
/*
// turn on ADC
	RTL_W32(0xe70, (RTL_R32(0xe70) | 0x01e00000) );
	delay_us(100);
*/

	priv->pshare->mp_ctx_count = 0;
	priv->pshare->mp_ctx_pkt = 0;

	if (OPMODE & WIFI_MP_CTX_PACKET) {
		OPMODE &= ~WIFI_MP_CTX_PACKET;
	} else {
		if (is_CCK_rate(priv->pshare->mp_datarate)) {
			if (OPMODE & WIFI_MP_CTX_CCK_CS) {
				OPMODE &= ~WIFI_MP_CTX_CCK_CS;
				mpt_ProSetCarrierSupp(priv, FALSE);
			} else {
				mpt_StopCckCoNtTx(priv);
			}
		} else {
			mpt_StopOfdmContTx(priv);
			if (OPMODE & WIFI_MP_CTX_OFDM_HW)
				OPMODE &= ~WIFI_MP_CTX_OFDM_HW;
			if (OPMODE & WIFI_MP_CTX_ST) {
				OPMODE &= ~WIFI_MP_CTX_ST;

				if ((get_rf_mimo_mode(priv) == MIMO_1T2R) || (get_rf_mimo_mode(priv) == MIMO_1T1R)) {
					// eRFPath = RF90_PATH_C;
					eRFPath = RF92CD_PATH_A;
				} else {
					switch (priv->pshare->mp_antenna_tx) {
					case ANTENNA_B:
						eRFPath = RF92CD_PATH_B;
						break;
					case ANTENNA_A:
					default:
						eRFPath = RF92CD_PATH_A;
						break;
					}
				}
				// Stop Single Tone.

#ifdef CONFIG_RTL_92D_SUPPORT // single tone 
				if (priv->pmib->dot11RFEntry.phyBandSelect==PHY_BAND_5G){
					PHY_SetBBReg(priv, rFPGA0_RFMOD, bOFDMEn, 0x1);
					PHY_SetBBReg(priv, 0x860, bMaskDWord, temp_860);
					PHY_SetBBReg(priv, 0x864, bMaskDWord, temp_864);
					PHY_SetBBReg(priv, 0x870, bMaskDWord, temp_870);
					PHY_SetRFReg(priv, eRFPath, 0x41, BIT(19), 0);
				} else
#endif
				{
					PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, 0x1);
					PHY_SetBBReg(priv, rFPGA0_RFMOD, bOFDMEn, 0x1);
					PHY_SetRFReg(priv, RF92CD_PATH_A, 0x21, bMask20Bits, 0x54000);
				}
				delay_us(100);
				PHY_SetRFReg(priv, eRFPath, 0x00, bMask20Bits, 0x30000); // PAD all on.
				delay_us(100);

#ifdef HIGH_POWER_EXT_PA
				if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)){
				if(priv->pshare->rf_ft_var.use_ext_pa) {
					PHY_SetBBReg(priv, 0x870, BIT(10), 0x0);
					PHY_SetBBReg(priv, 0x870, BIT(26), 0x0);
				}
				}
#endif
			}

			if (OPMODE & WIFI_MP_CTX_SCR) {
				OPMODE &= ~WIFI_MP_CTX_SCR;

				//Turn off all test modes.
				PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMContinueTx, bDisable);
				PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleCarrier, bDisable);
				PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleTone, bDisable);
				PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMDisPwsavTx, bDisable);

				PHY_SetBBReg(priv, rOFDM1_TRxMesaure1, 0xfff, 0);
				//Delay 10 ms
				delay_ms(10);
				//BB Reset
/*
				PHY_SetBBReg(priv, rPMAC_Reset, bBBResetB, 0x0);
				PHY_SetBBReg(priv, rPMAC_Reset, bBBResetB, 0x1);
*/
			}
		}

		PHY_SetBBReg(priv, rFPGA0_XA_HSSIParameter1, bContTxHSSI, 0);
	}
		if (OPMODE & WIFI_MP_CTX_BACKGROUND) {
		
		printk("Stop continuous TX\n");

		SAVE_INT_AND_CLI(flags);
		OPMODE |= WIFI_MP_CTX_BACKGROUND_STOPPING;
		OPMODE &= ~WIFI_MP_CTX_BACKGROUND_PENDING;
		RESTORE_INT(flags);
		
		while (priv->pshare->skb_head != priv->pshare->skb_tail) {
			DEBUG_INFO("[%s %d] skb_head/skb_tail=%d/%d, head/tail=%d/%d\n",
				__FUNCTION__, __LINE__, priv->pshare->skb_head, priv->pshare->skb_tail,
				get_txhead(phw, BE_QUEUE), get_txtail(phw, BE_QUEUE));
			
			rtl8192cd_tx_dsr((unsigned long)priv);
			delay_us(50);
		}
		
		SAVE_INT_AND_CLI(flags);
		OPMODE &= ~(WIFI_MP_CTX_BACKGROUND | WIFI_MP_CTX_BACKGROUND_PENDING | WIFI_MP_CTX_PACKET |
				WIFI_MP_CTX_ST | WIFI_MP_CTX_SCR | WIFI_MP_CTX_CCK_CS |WIFI_MP_CTX_OFDM_HW |
				WIFI_MP_CTX_BACKGROUND_STOPPING);
		RESTORE_INT(flags);
		
		for (i=0; i<NUM_MP_SKB; i++)
			kfree(priv->pshare->skb_pool[i]->head);
		kfree(priv->pshare->skb_pool_ptr);
	}
}


int mp_query_stats(struct rtl8192cd_priv *priv, unsigned char *data)
{
	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return 0;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return 0;
	}

	sprintf(data, "Tx OK:%d, Tx Fail:%d, Rx OK:%lu, CRC error:%lu",
		(int)(priv->net_stats.tx_packets-priv->net_stats.tx_errors),
		(int)priv->net_stats.tx_errors,
		priv->net_stats.rx_packets, priv->net_stats.rx_crc_errors);
	return strlen(data)+1;
}


void mp_txpower_tracking(struct rtl8192cd_priv *priv, unsigned char *data)
{
	char *val;
	unsigned int target_ther = 0;
	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
		return;
	}

	val = get_value_by_token((char *)data, "stop");
	if (val) {
		if (priv->pshare->mp_txpwr_tracking==FALSE)
			return;
		priv->pshare->mp_txpwr_tracking = FALSE;
		printk("mp tx power tracking stop\n");
		return;
	}

	val = get_value_by_token((char *)data, "ther=");
	if (val)
		target_ther = _atoi(val, 10);
	else if (priv->pmib->dot11RFEntry.ther)
		target_ther = priv->pmib->dot11RFEntry.ther;
	target_ther &= 0xff;

	if (!target_ther) {
		printk("Fail: tx power tracking has no target thermal value\n");
		return;
	}


#ifdef CONFIG_RTL_88E_SUPPORT //for 88e tx power tracking
	if(GET_CHIP_VER(priv) == VERSION_8188E){
		if ((target_ther < 0x07) || (target_ther > 0x32)) {
			printk("Warning: tx power tracking should have target thermal value 7-40\n");
			if (target_ther < 0x07)
				target_ther = 0x07;
			else
				target_ther = 0x32;
			printk("Warning: reset target thermal value as %d\n", target_ther);
		}
	}
	else
#endif
	if ((target_ther < 0x07) || (target_ther > 0x1d)) {
		printk("Warning: tx power tracking should have target thermal value 7-29\n");
		if (target_ther < 0x07)
			target_ther = 0x07;
		else
			target_ther = 0x1d;
		printk("Warning: reset target thermal value as %d\n", target_ther);
	}

	if(priv->pmib->dot11RFEntry.ther && priv->pshare->ThermalValue)
		priv->pshare->ThermalValue += (target_ther - priv->pmib->dot11RFEntry.ther );

	priv->pmib->dot11RFEntry.ther = target_ther;

	priv->pshare->mp_txpwr_tracking = TRUE;
	printk("mp tx power tracking start, target value=%d\n", target_ther);
}


int mp_query_tssi(struct rtl8192cd_priv *priv, unsigned char *data)
{
	unsigned int i=0, j=0, val32, tssi, tssi_total=0, tssi_reg, reg_backup;

	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return 0;
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
		return 0;
	}

	if (priv->pshare->mp_txpwr_tracking) {
		priv->pshare->mp_txpwr_tracking = FALSE;
		sprintf(data, "8651");
		return strlen(data)+1;
	}

	if (is_CCK_rate(priv->pshare->mp_datarate)) {
		reg_backup = RTL_R32(rFPGA0_AnalogParameter4);
		RTL_W32(rFPGA0_AnalogParameter4, (reg_backup & 0xfffff0ff));

		while (i < 5) {
			j++;
			delay_ms(10);
			val32 = PHY_QueryBBReg(priv, rCCK0_TRSSIReport, bMaskByte0);
			tssi = val32 & 0x7f;
			if (tssi > 10) {
				tssi_total += tssi;
				i++;
			}
			if (j > 20)
				break;
		}

		RTL_W32(rFPGA0_AnalogParameter4, reg_backup);

		if (i > 0)
			tssi = tssi_total / i;
		else
			tssi = 0;
	} else {
//		if (priv->pshare->mp_antenna_tx == ANTENNA_A)
			tssi_reg = rFPGA0_XAB_RFInterfaceRB;
//		else
//			tssi_reg = rFPGA0_XCD_RFInterfaceRB;

		reg_backup = PHY_QueryBBReg(priv, rOFDM0_TRxPathEnable, 0x0000000f);
		PHY_SetBBReg(priv, rOFDM0_TRxPathEnable, 0x0000000f, 0x0000000f);

		PHY_SetBBReg(priv, rFPGA0_XAB_RFParameter, BIT(25), 1);

		while (i < 5) {
			delay_ms(5);
			PHY_SetBBReg(priv, rFPGA0_XAB_RFParameter, BIT(25), 0);
			val32 = PHY_QueryBBReg(priv, tssi_reg, bMaskDWord);
			PHY_SetBBReg(priv, rFPGA0_XAB_RFParameter, BIT(25), 1);
			tssi = ((val32 & 0x04000000) >> 20) |
				   ((val32 & 0x00600000) >> 17) |
				   ((val32 & 0x00000c00) >> 8)  |
				   ((val32 & 0x00000060) >> 5);
			if (tssi) {
				tssi_total += tssi;
				i++;
			}
		}

		PHY_SetBBReg(priv, rOFDM0_TRxPathEnable, 0x0000000f, reg_backup);

		tssi = tssi_total / 5;
	}

	sprintf(data, "%d", tssi);
	return strlen(data)+1;
}


int mp_query_ther(struct rtl8192cd_priv *priv, unsigned char *data)
{
	unsigned int ther=0;

	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return 0;
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
		return 0;
	}

	// enable power and trigger
#ifdef CONFIG_RTL_88E_SUPPORT //for 88e tx power tracking
	if (GET_CHIP_VER(priv)==VERSION_8188E)
		PHY_SetRFReg(priv, RF92CD_PATH_A, RF_T_METER_88E, (BIT(17) | BIT(16)), 0x03);
	else
#endif
	if (CHECKICIS92D())
		PHY_SetRFReg(priv, RF92CD_PATH_A, RF_T_METER_92D, bMask20Bits, 0x30000);
	else	
		PHY_SetRFReg(priv, RF92CD_PATH_A, 0x24, bMask20Bits, 0x60);

	// delay for 1 second
	delay_ms(1000);

	// query rf reg 0x24[4:0], for thermal meter value
#ifdef CONFIG_RTL_88E_SUPPORT //for 88e tx power tracking
	if (GET_CHIP_VER(priv)==VERSION_8188E)
		ther = PHY_QueryRFReg(priv, RF92CD_PATH_A, RF_T_METER_88E, 0xfc00, 1);
	else
#endif
	if (CHECKICIS92D())	
		ther = PHY_QueryRFReg(priv, RF92CD_PATH_A, RF_T_METER_92D, 0xf800, 1);
	else	
		ther = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x24, bMask20Bits, 1) & 0x01f;


	sprintf(data, "%d", ther);
	return strlen(data)+1;
}

#ifdef MP_PSD_SUPPORT
int mp_query_psd(struct rtl8192cd_priv *priv, unsigned char * data)
{
	char *val;
	unsigned int i, psd_pts=0, psd_start=0, psd_stop=0;
	int psd_data=0;

	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return 0;
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
		return 0;
	}

	if (strlen(data) == 0) { //default value
		psd_pts = 128;
		psd_start = 64;
		psd_stop = 128;   
		
	}
	else
	{
		val = get_value_by_token((char *)data, "pts=");
		if (val) {
			psd_pts = _atoi(val,10);
		}else {
			psd_pts = 128;
		}

		val = get_value_by_token((char *)data, "start=");
		if (val) {
			psd_start=_atoi(val,10);
		}else {
			psd_start = 64;
		}

		val = get_value_by_token((char *)data, "stop=");
		if (val) {
			psd_stop=_atoi(val,10);
		}else {
			psd_stop = psd_pts;   
		}
	}

	memset(data,'\0',sizeof(data));

	i = psd_start;
	while( i < psd_stop) {
		
		if( i>= psd_pts) {
			psd_data = GetPSDData(priv,(i-psd_pts));
		}
		else {
			psd_data = GetPSDData(priv,i);
		}
		sprintf(data, "%s%x ",data, psd_data);
		i++;
	}

	panic_printk("\"read psd = %s\"\n", data);
	panic_printk("Length is %d\n",strlen(data));
	printk("read psd = %s\n", data);
	printk("Length is %d\n",strlen(data));
	
	delay_ms(500);

	return strlen(data)+1;
}
#endif

int mp_get_txpwr(struct rtl8192cd_priv *priv, unsigned char *data)
{
	unsigned int pwrA=0,pwrB=0;

	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return 0;
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
		return 0;
	}

	switch (priv->pshare->mp_datarate){
	case 2:
		pwrA = PHY_QueryBBReg(priv, 0xe08, bMaskByte1);
		pwrB = PHY_QueryBBReg(priv, 0x838, bMaskByte1);
		break;
	case 4:
		pwrA = PHY_QueryBBReg(priv, 0x86c, bMaskByte1);
		pwrB = PHY_QueryBBReg(priv, 0x838, bMaskByte2);
		break;
	case 11:
		pwrA = PHY_QueryBBReg(priv, 0x86c, bMaskByte2);
		pwrB = PHY_QueryBBReg(priv, 0x838, bMaskByte3);
		break;
	case 22:
		pwrA = PHY_QueryBBReg(priv, 0x86c, bMaskByte3);
		pwrB = PHY_QueryBBReg(priv, 0x86c, bMaskByte0);
		break;
	case 12:
		pwrA = PHY_QueryBBReg(priv, 0xe00, bMaskByte0);
		pwrB = PHY_QueryBBReg(priv, 0x830, bMaskByte0);
		break;
	case 18:
		pwrA = PHY_QueryBBReg(priv, 0xe00, bMaskByte1);
		pwrB = PHY_QueryBBReg(priv, 0x830, bMaskByte1);
		break;
	case 24:
		pwrA = PHY_QueryBBReg(priv, 0xe00, bMaskByte2);
		pwrB = PHY_QueryBBReg(priv, 0x830, bMaskByte2);
		break;
	case 36:
		pwrA = PHY_QueryBBReg(priv, 0xe00, bMaskByte3);
		pwrB = PHY_QueryBBReg(priv, 0x830, bMaskByte3);
		break;
	case 48:
		pwrA = PHY_QueryBBReg(priv, 0xe04, bMaskByte0);
		pwrB = PHY_QueryBBReg(priv, 0x834, bMaskByte0);
		break;
	case 72:
		pwrA = PHY_QueryBBReg(priv, 0xe04, bMaskByte1);
		pwrB = PHY_QueryBBReg(priv, 0x834, bMaskByte1);
		break;
	case 96:
		pwrA = PHY_QueryBBReg(priv, 0xe04, bMaskByte2);
		pwrB = PHY_QueryBBReg(priv, 0x834, bMaskByte2);
		break;
	case 108:
		pwrA = PHY_QueryBBReg(priv, 0xe04, bMaskByte3);
		pwrB = PHY_QueryBBReg(priv, 0x834, bMaskByte3);
		break;
	case 128:
		pwrA = PHY_QueryBBReg(priv, 0xe10, bMaskByte0);
		pwrB = PHY_QueryBBReg(priv, 0x83c, bMaskByte0);
		break;
	case 129:
		pwrA = PHY_QueryBBReg(priv, 0xe10, bMaskByte1);
		pwrB = PHY_QueryBBReg(priv, 0x83c, bMaskByte1);
		break;
	case 130:
		pwrA = PHY_QueryBBReg(priv, 0xe10, bMaskByte2);
		pwrB = PHY_QueryBBReg(priv, 0x83c, bMaskByte2);
		break;
	case 131:
		pwrA = PHY_QueryBBReg(priv, 0xe10, bMaskByte3);
		pwrB = PHY_QueryBBReg(priv, 0x83c, bMaskByte3);
		break;
	case 132:
		pwrA = PHY_QueryBBReg(priv, 0xe14, bMaskByte0);
		pwrB = PHY_QueryBBReg(priv, 0x848, bMaskByte0);
		break;
	case 133:
		pwrA = PHY_QueryBBReg(priv, 0xe14, bMaskByte1);
		pwrB = PHY_QueryBBReg(priv, 0x848, bMaskByte1);
		break;
	case 134:
		pwrA = PHY_QueryBBReg(priv, 0xe14, bMaskByte2);
		pwrB = PHY_QueryBBReg(priv, 0x848, bMaskByte2);
		break;
	case 135:
		pwrA = PHY_QueryBBReg(priv, 0xe14, bMaskByte3);
		pwrB = PHY_QueryBBReg(priv, 0x848, bMaskByte3);
		break;
	case 136:
		pwrA = PHY_QueryBBReg(priv, 0xe18, bMaskByte0);
		pwrB = PHY_QueryBBReg(priv, 0x84c, bMaskByte0);
		break;
	case 137:
		pwrA = PHY_QueryBBReg(priv, 0xe18, bMaskByte1);
		pwrB = PHY_QueryBBReg(priv, 0x84c, bMaskByte1);
		break;
	case 138:
		pwrA = PHY_QueryBBReg(priv, 0xe18, bMaskByte2);
		pwrB = PHY_QueryBBReg(priv, 0x84c, bMaskByte2);
		break;
	case 139:
		pwrA = PHY_QueryBBReg(priv, 0xe18, bMaskByte3);
		pwrB = PHY_QueryBBReg(priv, 0x84c, bMaskByte3);
		break;
	case 140:
		pwrA = PHY_QueryBBReg(priv, 0xe1c, bMaskByte0);
		pwrB = PHY_QueryBBReg(priv, 0x868, bMaskByte0);
		break;
	case 141:
		pwrA = PHY_QueryBBReg(priv, 0xe1c, bMaskByte1);
		pwrB = PHY_QueryBBReg(priv, 0x868, bMaskByte1);
		break;
	case 142:
		pwrA = PHY_QueryBBReg(priv, 0xe1c, bMaskByte2);
		pwrB = PHY_QueryBBReg(priv, 0x868, bMaskByte2);
		break;
	case 143:
		pwrA = PHY_QueryBBReg(priv, 0xe1c, bMaskByte3);
		pwrB = PHY_QueryBBReg(priv, 0x868, bMaskByte3);
		break;
	}

	sprintf(data, "%d %d", pwrA, pwrB);
	return strlen(data)+1;
}


#ifdef B2B_TEST
/* Do checksum and verification for configuration data */
static unsigned char byte_checksum(unsigned char *data, int len)
{
	int i;
	unsigned char sum=0;

	for (i=0; i<len; i++)
		sum += data[i];

	sum = ~sum + 1;
	return sum;
}

static int is_byte_checksum_ok(unsigned char *data, int len)
{
	int i;
	unsigned char sum=0;

	for (i=0; i<len; i++)
		sum += data[i];

	if (sum == 0)
		return 1;
	else
		return 0;
}


static void mp_init_sta(struct rtl8192cd_priv *priv,unsigned char *da_mac)
{
	struct stat_info *pstat;
	unsigned char *da;

	da = da_mac;
	// prepare station info
	if (memcmp(da, "\x0\x0\x0\x0\x0\x0", 6) && !IS_MCAST(da))
	{
		pstat = get_stainfo(priv, da);
		if (pstat == NULL)
		{
			pstat = alloc_stainfo(priv, da, -1);
			pstat->state = WIFI_AUTH_SUCCESS | WIFI_ASOC_STATE;
			memcpy(pstat->bssrateset, AP_BSSRATE, AP_BSSRATE_LEN);
			pstat->bssratelen = AP_BSSRATE_LEN;
			pstat->expire_to = 30000;
			list_add_tail(&pstat->asoc_list, &priv->asoc_list);
			cnt_assoc_num(priv, pstat, INCREASE, (char *)__FUNCTION__);
			if (QOS_ENABLE)
				pstat->QosEnabled = 1;
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) {
				pstat->ht_cap_len = priv->ht_cap_len;
				memcpy(&pstat->ht_cap_buf, &priv->ht_cap_buf, priv->ht_cap_len);
			}
			pstat->current_tx_rate = priv->pshare->mp_datarate;
			update_fwtbl_asoclst(priv, pstat);
//			add_update_RATid(priv, pstat);
		}
	}
}


/*
 * tx pakcet.
 *  command: "iwpriv wlanx mp_tx,da=xxx,time=n,count=n,len=n,retry=n,tofr=n,wait=n,delay=n,err=n"
 *		default: da=ffffffffffff, time=0,count=1000, len=1500, retry=6, tofr=0, wait=0, delay=0(ms), err=1
 *		note: if time is set, it will take time (in sec) rather count.
 *		     if "time=-1", tx will continue tx until ESC. If "err=1", display statistics when tx err.
 */
int mp_tx(struct rtl8192cd_priv *priv, unsigned char *data)
{
	unsigned int orgTCR = RTL_R32(TCR);
	unsigned char increaseIFS=0; // set to 1 to increase the inter frame spacing while in PER test
	unsigned char pbuf[6]={0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	int count=1000, payloadlen=1500, retry=6, tofr=0, wait=0, delay=0, time=0;
	int err=1;
	struct sk_buff *skb;
	struct wlan_ethhdr_t *pethhdr;
	int len, i, q_num, ret, resent;
	unsigned char pattern=0xab;
	char *val;
	struct rtl8192cd_hw *phw = GET_HW(priv);
	static int last_tx_err;
	unsigned long end_time=0, flags;

	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return 0;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return 0;
	}

	// get da
	val = get_value_by_token((char *)data, "da=");
	if (val) {
		ret = get_array_val(pbuf, val, 12);
		if (ret != 6) {
			printk("Error da format\n");
			return 0;
		}
	}

	// get time
	val = get_value_by_token((char *)data, "time=");
	if (val) {
		if (!memcmp(val, "-1", 2))
			time = -1;
		else {
			time = _atoi(val, 10);
			time = time*HZ;	// in 10ms
		}
	}

	// get count
	val = get_value_by_token((char *)data, "count=");
	if (val) {
		count = _atoi(val, 10);
	}

	// get payload len
	val = get_value_by_token((char *)data, "len=");
	if (val) {
		payloadlen = _atoi(val, 10);
		if (payloadlen < 20) {
			printk("len should be greater than 20!\n");
			return 0;
		}
	}

	// get retry number
	val = get_value_by_token((char *)data, "retry=");
	if (val) {
		retry = _atoi(val, 10);
	}

	// get tofr
	val = get_value_by_token((char *)data, "tofr=");
	if (val) {
		tofr = _atoi(val, 10);
	}

	// get wait
	val = get_value_by_token((char *)data, "wait=");
	if (val) {
		wait = _atoi(val, 10);
	}

	// get err
	val = get_value_by_token((char *)data, "err=");
	if (val) {
		err = _atoi(val, 10);
	}

	len = payloadlen + WLAN_ETHHDR_LEN;
	q_num = BE_QUEUE;

	if (time)
		printk("Start TX DA=%02x%02x%02x%02x%02x%02x len=%d tofr=%d retry=%d wait=%s time=%ds",
			pbuf[0], pbuf[1], pbuf[2], pbuf[3], pbuf[4], pbuf[5],
			payloadlen, tofr, retry, (wait ? "yes" : "no"), ((time > 0) ? time/100 : -1));
	else
		printk("Start TX DA=%02x%02x%02x%02x%02x%02x count=%d len=%d tofr=%d retry=%d wait=%s",
			pbuf[0], pbuf[1], pbuf[2], pbuf[3], pbuf[4], pbuf[5],
			count, payloadlen, tofr, retry, (wait ? "yes" : "no"));

#if defined(USE_RTL8186_SDK)
	printk(", press any key to escape.\n");
#else
	printk("\n");
#endif

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
		priv->pshare->InterruptMask |= HIMR_88E_BEDOK;
		RTL_W32(REG_88E_HIMR, priv->pshare->InterruptMask);
	} else
#endif
	{
		RTL_W32(HIMR, RTL_R32(HIMR) | HIMR_BEDOK);
	}

//	RTL_W32(_RCR_, _NO_ERLYRX_);
	RTL_W32(RCR, RTL_R32(RCR) & ~(RCR_AB | RCR_AM | RCR_APM | RCR_AAP));

	if (increaseIFS) {
//		RTL_W32(_TCR_, RTL_R32(_TCR_) | _DISCW_);
		RTL_W16(TX_PTCL_CTRL, RTL_R16(TX_PTCL_CTRL) | DIS_CW);
	}

	memset(&priv->net_stats, 0, sizeof(struct net_device_stats));
	priv->ext_stats.tx_retrys=0;
	last_tx_err = 0;

	if (time > 0) {
		end_time = jiffies + time;
	}

	i = 0;
	resent = 0;

#if defined(USE_RTL8186_SDK)
	DISABLE_UART0_INT();
#endif

	mp_init_sta(priv, &pbuf[0]);

	while (1)
	{
#if defined(USE_RTL8186_SDK)
		if ( IS_KEYBRD_HIT())
			break;
#endif

		if (time) {
			if (time != -1) {
				if (jiffies > end_time)
					break;
			}
		}
		else {
			if (!resent && i >= count)
				break;
		}
		if (!resent)
			i++;

		skb = dev_alloc_skb(len);

		if (skb != NULL)
		{
			DECLARE_TXINSN(txinsn);

			skb->dev = priv->dev;
			skb_put(skb, len);

			pethhdr = (struct wlan_ethhdr_t *)(skb->data);
			memcpy((void *)pethhdr->daddr, pbuf, MACADDRLEN);
			memcpy((void *)pethhdr->saddr, BSSID, MACADDRLEN);
			pethhdr->type = payloadlen;

			// construct tx patten
			memset(skb->data+WLAN_ETHHDR_LEN, pattern, payloadlen);

			memcpy(skb->data+WLAN_ETHHDR_LEN, MP_PACKET_HEADER, MP_PACKET_HEADER_LEN); // header
			memcpy(skb->data+WLAN_ETHHDR_LEN+12, &i, 4); // packet sequence
			skb->data[len-1] = byte_checksum(skb->data+WLAN_ETHHDR_LEN,	payloadlen-1); // checksum

			txinsn.q_num	= q_num; //using low queue for data queue
			txinsn.fr_type	= _SKB_FRAME_TYPE_;
			txinsn.pframe	= skb;
			txinsn.tx_rate	= txinsn.lowest_tx_rate = priv->pshare->mp_datarate;
			txinsn.fixed_rate = 1;
			txinsn.retry	= retry;
			txinsn.phdr		= get_wlanllchdr_from_poll(priv);
			skb->cb[1] = 0;
			
#ifdef MCAST2UI_REFINE
			memcpy(&skb->cb[10], skb->data, 6);
#endif

			memset((void *)txinsn.phdr, 0, sizeof(struct wlanllc_hdr));

			if (tofr & 2)
				SetToDs(txinsn.phdr);
			if (tofr & 1)
				SetFrDs(txinsn.phdr);

			SetFrameType(txinsn.phdr, WIFI_DATA);

			if (wait) {
				while (1) {
					volatile unsigned int head, tail;
					head = get_txhead(phw, BE_QUEUE);
					tail = get_txtail(phw, BE_QUEUE);
					if (head == tail)
						break;
					delay_ms(1);
				}
			}

			if(rtl8192cd_firetx(priv, &txinsn) == CONGESTED)
			{
				if (txinsn.phdr)
					release_wlanllchdr_to_poll(priv, txinsn.phdr);
				if (skb)
					rtl_kfree_skb(priv, skb, _SKB_TX_);

				//printk("CONGESTED : busy waiting...\n");
				delay_ms(1);
				resent = 1;

				SAVE_INT_AND_CLI(flags);
#ifdef SMP_SYNC
				if (!priv->pshare->has_triggered_tx_tasklet) {
					tasklet_schedule(&priv->pshare->tx_tasklet);
					priv->pshare->has_triggered_tx_tasklet = 1;
				}
#else
				rtl8192cd_tx_dsr((unsigned long)priv);
#endif
				RESTORE_INT(flags);
			}
			else {
				SAVE_INT_AND_CLI(flags);
#ifdef SMP_SYNC
				if (!priv->pshare->has_triggered_tx_tasklet) {
					tasklet_schedule(&priv->pshare->tx_tasklet);
					priv->pshare->has_triggered_tx_tasklet = 1;
				}
#else
				rtl8192cd_tx_dsr((unsigned long)priv);
#endif
				RESTORE_INT(flags);

				if (err && ((int)priv->net_stats.tx_errors) != last_tx_err) { // err happen
					printk("\tout=%d\tfail=%d\n", (int)priv->net_stats.tx_packets,
							(int)priv->net_stats.tx_errors);
					last_tx_err = (int)priv->net_stats.tx_errors;
				}
				else {
					if ( (i%10000) == 0 )
						printk("Tx status: ok=%d\tfail=%d\tretry=%ld\n", (int)(priv->net_stats.tx_packets-priv->net_stats.tx_errors),
							(int)priv->net_stats.tx_errors, priv->ext_stats.tx_retrys);
				}
				resent = 0;
			}
			if (delay)
				delay_ms(delay);
		}
		else
		{
			printk("Can't allocate sk_buff\n");
			delay_ms(1);
			resent = 1;

			SAVE_INT_AND_CLI(flags);
#ifdef SMP_SYNC
			if (!priv->pshare->has_triggered_tx_tasklet) {
				tasklet_schedule(&priv->pshare->tx_tasklet);
				priv->pshare->has_triggered_tx_tasklet = 1;
			}
#else
			rtl8192cd_tx_dsr((unsigned long)priv);
#endif
			RESTORE_INT(flags);
		}
	}

#if defined(USE_RTL8186_SDK)
	RESTORE_UART0_INT();
#endif

	// wait till all tx is done
	printk("\nwaiting tx is finished...");
	i = 0;
	while (1) {
		volatile unsigned int head, tail;
		head = get_txhead(phw, BE_QUEUE);
		tail = get_txtail(phw, BE_QUEUE);
		if (head == tail)
			break;

		SAVE_INT_AND_CLI(flags);
#ifdef SMP_SYNC
		if (!priv->pshare->has_triggered_tx_tasklet) {
			tasklet_schedule(&priv->pshare->tx_tasklet);
			priv->pshare->has_triggered_tx_tasklet = 1;
		}
#else
		rtl8192cd_tx_dsr((unsigned long)priv);
#endif
		RESTORE_INT(flags);

		delay_ms(1);

		if (i++ >10000)
			break;
	}
	printk("done.\n");

	RTL_W32(TCR, orgTCR);

	sprintf(data, "Tx result: ok=%d,fail=%d", (int)(priv->net_stats.tx_packets-priv->net_stats.tx_errors),
			(int)priv->net_stats.tx_errors);
	return strlen(data)+1;
}


/*
 * validate rx packet. rx packet format:
 *	|wlan-header(24 byte)|MP_PACKET_HEADER (12 byte)|sequence(4 bytes)|....|checksum(1 byte)|
 *
 */
void mp_validate_rx_packet(struct rtl8192cd_priv *priv, unsigned char *data, int len)
{
	int tofr = get_tofr_ds(data);
	unsigned int type=GetFrameType(data);
	int header_size = 24;
	unsigned long sequence;
	unsigned short fr_seq;

	if (!priv->pshare->mp_rx_waiting)
		return;

	if (type != WIFI_DATA)
		return;

	fr_seq = GetTupleCache(data);
	if (GetRetry(data) && fr_seq == priv->pshare->mp_cached_seq) {
		priv->pshare->mp_rx_dup++;
		return;
	}

	if (tofr == 3)
		header_size = 30;

	if (len < (header_size+20) )
		return;

	// see if test header matched
	if (memcmp(&data[header_size], MP_PACKET_HEADER, MP_PACKET_HEADER_LEN))
		return;

	priv->pshare->mp_cached_seq = fr_seq;

	memcpy(&sequence, &data[header_size+MP_PACKET_HEADER_LEN], 4);

	if (!is_byte_checksum_ok(&data[header_size], len-header_size)) {
#if 0
		printk("mp_rx: checksum error!\n");
#endif
		printk("mp_brx: checksum error!\n");
	}
	else {
		if (sequence <= priv->pshare->mp_rx_sequence) {
#if 0
			printk("mp_rx: invalid sequece (%ld) <= current (%ld)!\n",
									sequence, priv->pshare->mp_rx_sequence);
#endif
			printk("mp_brx: invalid sequece (%ld) <= current (%ld)!\n",
									sequence, priv->pshare->mp_rx_sequence);
		}
		else {
			if (sequence > (priv->pshare->mp_rx_sequence+1))
				priv->pshare->mp_rx_lost_packet += (sequence-priv->pshare->mp_rx_sequence-1);
			priv->pshare->mp_rx_sequence = sequence;
			priv->pshare->mp_rx_ok++;
		}
	}
}


#if 0
/*
 * Rx test packet.
 *   command: "iwpriv wlanx mp_rx [ra=xxxxxx,quiet=t,interval=n]"
 *	           - ra: rx mac. defulat is burn-in mac
 *             - quiet_time: quit rx if no rx packet during quiet_time. default is 5s
 *             - interval: report rx statistics periodically in sec. default is 0 (no report)
 */
static void mp_rx(struct rtl8192cd_priv *priv, unsigned char *data)
{
	char *val;
	unsigned char pbuf[6];
	int quiet_time=5, interval_time=0, quiet_period=0, interval_period=0, ret;
	unsigned int o_rx_ok, o_rx_lost_packet, mac_changed=0;
	unsigned long reg, counter=0;

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return;
	}

	// get ra
	val = get_value_by_token((char *)data, "ra=");
	if (val) {
		ret =0;
		if (strlen(val) >=12)
			ret = get_array_val(pbuf, val, 12);
		if (ret != 6) {
			printk("Error mac format\n");
			return;
		}
		printk("set ra to %02X:%02X:%02X:%02X:%02X:%02X\n",
				pbuf[0], pbuf[1], pbuf[2], pbuf[3], pbuf[4], pbuf[5]);

		memcpy(&reg, pbuf, 4);
		RTL_W32(_IDR0_, (cpu_to_le32(reg)));

		memcpy(&reg, pbuf+4, 4);
		RTL_W32(_IDR0_ + 4, (cpu_to_le32(reg)));
		mac_changed = 1;
	}

	// get quiet time
	val = get_value_by_token((char *)data, "quiet=");
	if (val)
		quiet_time = _atoi(val, 10);

	// get interval time
	val = get_value_by_token((char *)data, "interval=");
	if (val)
		interval_time = _atoi(val, 10);

	RTL_W32(_RCR_, _ENMARP_ | _APWRMGT_ | _AMF_ | _ADF_ | _NO_ERLYRX_ |
			_RX_DMA64_ | _ACRC32_ | _AB_ | _AM_ | _APM_ | _AAP_);

	priv->pshare->mp_cached_seq = 0;
	priv->pshare->mp_rx_ok = 0;
	priv->pshare->mp_rx_sequence = 0;
	priv->pshare->mp_rx_lost_packet = 0;
	priv->pshare->mp_rx_dup = 0;

	printk("Waiting for rx packet, quit if no packet in %d sec", quiet_time);

#if (defined(CONFIG_RTL_EB8186) && defined(__KERNEL__)) || defined(CONFIG_RTL865X)
	printk(", or press any key to escape.\n");
	DISABLE_UART0_INT();
#else
	printk(".\n");
#endif

	priv->pshare->mp_rx_waiting = 1;

	while (1) {
		// save old counter
		o_rx_ok = priv->pshare->mp_rx_ok;
		o_rx_lost_packet = priv->pshare->mp_rx_lost_packet;

#if (defined(CONFIG_RTL_EB8186) && defined(__KERNEL__)) || defined(CONFIG_RTL865X)
		if ( IS_KEYBRD_HIT())
			break;
#endif

		delay_ms(1000);

		if (interval_time && ++interval_period == interval_time) {
			printk("\tok=%ld\tlost=%ld\n", priv->pshare->mp_rx_ok, priv->pshare->mp_rx_lost_packet);
			interval_period=0;
		}
		else {
			if ((priv->pshare->mp_rx_ok-counter) > 10000) {
				printk("Rx status: ok=%ld\tlost=%ld, duplicate=%ld\n", priv->pshare->mp_rx_ok, priv->pshare->mp_rx_lost_packet, priv->pshare->mp_rx_dup);
				counter += 10000;
			}
		}

		if (o_rx_ok == priv->pshare->mp_rx_ok && o_rx_lost_packet == priv->pshare->mp_rx_lost_packet)
			quiet_period++;
		else
			quiet_period = 0;

		if (quiet_period >= quiet_time)
			break;
	}

//	printk("\nRx result: ok=%ld\tlost=%ld, duplicate=%ld\n\n", priv->pshare->mp_rx_ok, priv->pshare->mp_rx_lost_packet, priv->mp_rx_dup);
	printk("\nRx reseult: ok=%ld\tlost=%ld\n\n", priv->pshare->mp_rx_ok, priv->pshare->mp_rx_lost_packet);

	priv->pshare->mp_rx_waiting = 0;

	if (mac_changed) {
		memcpy(pbuf, priv->pmib->dot11OperationEntry.hwaddr, MACADDRLEN);

		memcpy(&reg, pbuf, 4);
		RTL_W32(_IDR0_, (cpu_to_le32(reg)));

		memcpy(&reg, pbuf+4, 4);
		RTL_W32(_IDR0_ + 4, (cpu_to_le32(reg)));
	}

#if (defined(CONFIG_RTL_EB8186) && defined(__KERNEL__)) || defined(CONFIG_RTL865X)
	RESTORE_UART0_INT();
#endif

}
#endif


/*
 * Rx test packet.
 *   command: "iwpriv wlanx mp_brx start[,ra=xxxxxx]"
 *	           - ra: rx mac. defulat is burn-in mac
 *   command: "iwpriv wlanx mp_brx stop"
 *               - stop rx immediately
 */
int mp_brx(struct rtl8192cd_priv *priv, unsigned char *data)
{
	char *val;
	unsigned char pbuf[6];
	int ret;
	unsigned long reg;
	unsigned long	flags;

	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return 0;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return 0;
	}

	SAVE_INT_AND_CLI(flags);

	if (!strcmp(data, "stop"))
		goto stop_brx;

	// get start
	val = get_value_by_token((char *)data, "start");
	if (val) {
		// get ra if it exists
		val = get_value_by_token((char *)data, "ra=");
		if (val) {
			ret =0;
			if (strlen(val) >=12)
				ret = get_array_val(pbuf, val, 12);
			if (ret != 6) {
				printk("Error mac format\n");
				RESTORE_INT(flags);
				return 0;
			}
			printk("set ra to %02X:%02X:%02X:%02X:%02X:%02X\n",
				pbuf[0], pbuf[1], pbuf[2], pbuf[3], pbuf[4], pbuf[5]);

			memcpy(&reg, pbuf, 4);
			RTL_W32(MACID, (cpu_to_le32(reg)));

			memcpy(&reg, pbuf+4, 4);
			RTL_W32(MACID + 4, (cpu_to_le32(reg)));
			priv->pshare->mp_mac_changed = 1;
		}
	}
	else {
		RESTORE_INT(flags);
		return 0;
	}

	priv->pshare->mp_cached_seq = 0;
	priv->pshare->mp_rx_ok = 0;
	priv->pshare->mp_rx_sequence = 0;
	priv->pshare->mp_rx_lost_packet = 0;
	priv->pshare->mp_rx_dup = 0;
	priv->pshare->mp_rx_waiting = 1;

	// record the start time of MP throughput test
	priv->pshare->txrx_start_time = jiffies;

	OPMODE |= WIFI_MP_RX;
//	RTL_W32(RCR, _ENMARP_ | _APWRMGT_ | _AMF_ | _ADF_ | _NO_ERLYRX_ |
//			_RX_DMA64_ | _ACRC32_ | _AB_ | _AM_ | _APM_);
	RTL_W32(RCR, RCR_HTC_LOC_CTRL | RCR_AMF | RCR_ADF | RCR_ACRC32 | RCR_APWRMGT | RCR_AB | RCR_AM | RCR_APM);

	memset(&priv->net_stats, 0,  sizeof(struct net_device_stats));
	memset(&priv->ext_stats, 0,  sizeof(struct extra_stats));

	RESTORE_INT(flags);
	return 0;

stop_brx:
	OPMODE &= ~WIFI_MP_RX;

//	RTL_W32(RCR, _ENMARP_ | _NO_ERLYRX_ | _RX_DMA64_);
	RTL_W32(RCR, RCR_HTC_LOC_CTRL);
	priv->pshare->mp_rx_waiting = 0;

	//record the elapsed time of MP throughput test
	priv->pshare->txrx_elapsed_time = jiffies - priv->pshare->txrx_start_time;

	if (priv->pshare->mp_mac_changed) {
		memcpy(pbuf, priv->pmib->dot11OperationEntry.hwaddr, MACADDRLEN);

		memcpy(&reg, pbuf, 4);
		RTL_W32(MACID, (cpu_to_le32(reg)));

		memcpy(&reg, pbuf+4, 4);
		RTL_W32(MACID + 4, (cpu_to_le32(reg)));
	}
	priv->pshare->mp_mac_changed = 0;

	RESTORE_INT(flags);

	sprintf(data, "Rx reseult: ok=%ld,lost=%ld,elapsed time=%ld",
		priv->pshare->mp_rx_ok, priv->pshare->mp_rx_lost_packet, priv->pshare->txrx_elapsed_time);
	return strlen(data)+1;
}
#endif // B2B_TEST


void mp_set_bandwidth(struct rtl8192cd_priv *priv, unsigned char *data)
{
	char *val;
	int use40M=0, shortGI=0;
//	unsigned int regval, i, val32;
//	unsigned char *CCK_SwingEntry;

	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
		return;
	}

	// get 20M~40M , 40M=1 or 40M=0(20M)
	val = get_value_by_token((char *)data, "40M=");
	if (val) {
		use40M = _atoi(val, 10);
	}

    // get shortGI=1 or 0.
	val = get_value_by_token((char *)data, "shortGI=");
	if (val) {
		shortGI = _atoi(val, 10);
	}

	// modify short GI
	if(shortGI) {
		priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M = 1;
		priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M = 1;
	} else {
		priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M = 0;
		priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M = 0;
	}

	// modify 40M
	if (use40M) {
		priv->pshare->is_40m_bw = 1;
		priv->pshare->CurrentChannelBW = 1;
		priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_BELOW;
		priv->pmib->dot11nConfigEntry.dot11nUse40M=1;
	} else {
		priv->pshare->is_40m_bw = 0;
		priv->pshare->CurrentChannelBW = 0;
		priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_DONTCARE;
		priv->pmib->dot11nConfigEntry.dot11nUse40M=0;
	}

	SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);

//	mp_8192CD_tx_setting(priv);
	return; // end here
}


/*
 * auto-rx
 * accept CRC32 error pkt
 * accept destination address pkt
 * drop tx pkt (implemented in other functions)
 */
#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
int mp_arx(struct rtl8192cd_priv *priv, unsigned char *data)
{
	char *val;
	
	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return 0;
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
		return 0;
	}
	if (OPMODE & WIFI_MP_CTX_BACKGROUND) {
		printk("Fail: In MP background mode, please stop and retry it again\n");
		return 0;
	}

	if (!strcmp(data, "start")) {
		OPMODE |= WIFI_MP_RX;
#ifdef MP_SWITCH_LNA
		priv->pshare->rx_packet_ss_a = 0;
		priv->pshare->rx_packet_ss_b = 0;
		PHY_SetBBReg(priv, 0xd00, BIT(27)|BIT(26), 0x3);
		RTL_W32(RCR, RCR_APWRMGT | RCR_AMF | RCR_ADF |RCR_ACRC32 |RCR_AB | RCR_AM | RCR_APM | RCR_AAP | RCR_APP_PHYSTS);
#else
		RTL_W32(RCR, RCR_APWRMGT | RCR_AMF | RCR_ADF |RCR_ACRC32 |RCR_AB | RCR_AM | RCR_APM | RCR_AAP);
#endif
		if (priv->pshare->rf_ft_var.use_frq_2_3G)
			PHY_SetBBReg(priv, rCCK0_System, bCCKEqualizer, 0x0);

		memset(&priv->net_stats, 0,  sizeof(struct net_device_stats));
		memset(&priv->ext_stats, 0,  sizeof(struct extra_stats));

#ifdef CONFIG_RTL_92C_SUPPORT
               if( IS_UMC_B_CUT_88C(priv) )
                        PHY_SetRFReg(priv, 0, 0x26, bMask20Bits, 0x4f200);
#endif
	} else if (!strcmp(data, "stop")) {
		OPMODE &= ~WIFI_MP_RX;
		RTL_W32(RCR, RTL_R32(RCR) & ~(RCR_AB | RCR_AM | RCR_APM | RCR_AAP));

		if (priv->pshare->rf_ft_var.use_frq_2_3G)
			PHY_SetBBReg(priv, rCCK0_System, bCCKEqualizer, 0x1);

//clear filter:
		if((priv->pshare->mp_filter_flag) || (OPMODE & WIFI_MP_ARX_FILTER)) {
			OPMODE &= ~WIFI_MP_ARX_FILTER;
			priv->pshare->mp_filter_flag=0;
			memset(priv->pshare->mp_filter_DA,0,6);
			memset(priv->pshare->mp_filter_SA,0,6);
			memset(priv->pshare->mp_filter_BSSID,0,6);
		}	
		
		sprintf(data, "Received packet OK:%lu  CRC error:%lu\n", priv->net_stats.rx_packets, priv->net_stats.rx_crc_errors);
		return strlen(data)+1;
	} else if (!memcmp(data, "filter_SA",9)) {
		OPMODE |= WIFI_MP_ARX_FILTER;	
		memset(priv->pshare->mp_filter_SA,'\0',6);
		
		priv->pshare->mp_filter_flag |=0x1;
		val = get_value_by_token((char *)data, "filter_SA=");
		val[2]='\0';
		priv->pshare->mp_filter_SA[0]=_atoi(val,16);
		val[5]='\0';
		priv->pshare->mp_filter_SA[1]=_atoi(val+3,16);
		val[8]='\0';
		priv->pshare->mp_filter_SA[2]=_atoi(val+6,16);
		val[11]='\0';
		priv->pshare->mp_filter_SA[3]=_atoi(val+9,16);
		val[14]='\0';
		priv->pshare->mp_filter_SA[4]=_atoi(val+12,16);
		val[17]='\0';
		priv->pshare->mp_filter_SA[5]=_atoi(val+15,16);
		sprintf(data,"flag: %x\nSA: %02x:%02x:%02x:%02x:%02x:%02x\n",priv->pshare->mp_filter_flag,
															priv->pshare->mp_filter_SA[0],
															priv->pshare->mp_filter_SA[1],
															priv->pshare->mp_filter_SA[2],
															priv->pshare->mp_filter_SA[3],
															priv->pshare->mp_filter_SA[4],
															priv->pshare->mp_filter_SA[5]);
		return strlen(data)+1;
		
	} else if (!memcmp(data, "filter_DA",9)) {
		OPMODE |= WIFI_MP_ARX_FILTER;	
		priv->pshare->mp_filter_flag |=0x2;
		memset(priv->pshare->mp_filter_DA,'\0',6);
		
		val = get_value_by_token((char *)data, "filter_DA=");
		val[2]='\0';
		priv->pshare->mp_filter_DA[0]=_atoi(val,16);
		val[5]='\0';
		priv->pshare->mp_filter_DA[1]=_atoi(val+3,16);
		val[8]='\0';
		priv->pshare->mp_filter_DA[2]=_atoi(val+6,16);
		val[11]='\0';
		priv->pshare->mp_filter_DA[3]=_atoi(val+9,16);
		val[14]='\0';
		priv->pshare->mp_filter_DA[4]=_atoi(val+12,16);
		val[17]='\0';
		priv->pshare->mp_filter_DA[5]=_atoi(val+15,16);
		sprintf(data,"flag: %x\nDA: %02x:%02x:%02x:%02x:%02x:%02x\n",priv->pshare->mp_filter_flag,
															priv->pshare->mp_filter_DA[0],
															priv->pshare->mp_filter_DA[1],
															priv->pshare->mp_filter_DA[2],
															priv->pshare->mp_filter_DA[3],
															priv->pshare->mp_filter_DA[4],
															priv->pshare->mp_filter_DA[5]);
		return strlen(data)+1;
	} else if (!memcmp(data, "filter_BSSID",12)) {
		OPMODE |= WIFI_MP_ARX_FILTER;	
		priv->pshare->mp_filter_flag |=0x4;
		memset(priv->pshare->mp_filter_BSSID,'\0',6);
		
		val = get_value_by_token((char *)data, "filter_BSSID=");
		val[2]='\0';
		priv->pshare->mp_filter_BSSID[0]=_atoi(val,16);
		val[5]='\0';
		priv->pshare->mp_filter_BSSID[1]=_atoi(val+3,16);
		val[8]='\0';
		priv->pshare->mp_filter_BSSID[2]=_atoi(val+6,16);
		val[11]='\0';
		priv->pshare->mp_filter_BSSID[3]=_atoi(val+9,16);
		val[14]='\0';
		priv->pshare->mp_filter_BSSID[4]=_atoi(val+12,16);
		val[17]='\0';
		priv->pshare->mp_filter_BSSID[5]=_atoi(val+15,16);
		sprintf(data,"flag: %x\nBSSID: %02x:%02x:%02x:%02x:%02x:%02x\n",priv->pshare->mp_filter_flag,
															priv->pshare->mp_filter_BSSID[0],
															priv->pshare->mp_filter_BSSID[1],
															priv->pshare->mp_filter_BSSID[2],
															priv->pshare->mp_filter_BSSID[3],
															priv->pshare->mp_filter_BSSID[4],
															priv->pshare->mp_filter_BSSID[5]);
		return strlen(data)+1;
	} else if( (!memcmp(data, "filter_clean",12))  || (!memcmp(data, "filter_init",11))) {
		OPMODE &=  (~WIFI_MP_ARX_FILTER);	
		priv->pshare->mp_filter_flag=0;
		memset(priv->pshare->mp_filter_SA,'\0',6);
		memset(priv->pshare->mp_filter_DA,'\0',6);
		memset(priv->pshare->mp_filter_BSSID,'\0',6);
		sprintf(data,"reset arx filter table\n");
		return strlen(data)+1;
	} else if (!memcmp(data, "filter_print",12)) {		
		sprintf(data,"flag: %x\nSA: %02x:%02x:%02x:%02x:%02x:%02x\nDA: %02x:%02x:%02x:%02x:%02x:%02x\nBSSID: %02x:%02x:%02x:%02x:%02x:%02x\n",priv->pshare->mp_filter_flag,
															priv->pshare->mp_filter_SA[0],
															priv->pshare->mp_filter_SA[1],
															priv->pshare->mp_filter_SA[2],
															priv->pshare->mp_filter_SA[3],
															priv->pshare->mp_filter_SA[4],
															priv->pshare->mp_filter_SA[5],
															priv->pshare->mp_filter_DA[0],
															priv->pshare->mp_filter_DA[1],
															priv->pshare->mp_filter_DA[2],
															priv->pshare->mp_filter_DA[3],
															priv->pshare->mp_filter_DA[4],
															priv->pshare->mp_filter_DA[5],
															priv->pshare->mp_filter_BSSID[0],
															priv->pshare->mp_filter_BSSID[1],
															priv->pshare->mp_filter_BSSID[2],
															priv->pshare->mp_filter_BSSID[3],
															priv->pshare->mp_filter_BSSID[4],
															priv->pshare->mp_filter_BSSID[5]);
		
		return strlen(data)+1;
	}

	return 0;
}


/*
 * set bssid
 */
void mp_set_bssid(struct rtl8192cd_priv *priv, unsigned char *data)
{
	unsigned char pbuf[6];
	int ret;

	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
		return;
	}

	ret = get_array_val(pbuf, (char *)data, strlen(data));

	if (ret != 6) {
		printk("Error bssid format\n");
		return;
	} else {
		printk("set bssid to %02X:%02X:%02X:%02X:%02X:%02X\n",
			pbuf[0], pbuf[1], pbuf[2], pbuf[3], pbuf[4], pbuf[5]);
	}

	memcpy(BSSID, pbuf, MACADDRLEN);
}


static void mp_chk_sw_ant(struct rtl8192cd_priv *priv)
{
	R_ANTENNA_SELECT_OFDM	*p_ofdm_tx;	/* OFDM Tx register */
	R_ANTENNA_SELECT_CCK	*p_cck_txrx;
	unsigned char			r_rx_antenna_ofdm=0, r_ant_select_cck_val=0;
	unsigned char			chgTx=0, chgRx=0;
	unsigned int			r_ant_sel_cck_val=0, r_ant_select_ofdm_val=0, r_ofdm_tx_en_val=0;

	p_ofdm_tx = (R_ANTENNA_SELECT_OFDM *)&r_ant_select_ofdm_val;
	p_cck_txrx = (R_ANTENNA_SELECT_CCK *)&r_ant_select_cck_val;

	p_ofdm_tx->r_ant_ht1			= 0x1;
	p_ofdm_tx->r_ant_non_ht 		= 0x3;
	p_ofdm_tx->r_ant_ht2			= 0x2;

	// 原因是Tx 3-wire enable需隨Tx Ant path打開才會開啟，
	// 所以需在設BB 0x824與0x82C時，同時將BB 0x804[3:0]設為3(同時打開Ant. A and B)。
	// 要省電的情況下，A Tx時 0x90C=0x11111111，B Tx時 0x90C=0x22222222，AB同時開就維持原來設定0x3321333


	switch(priv->pshare->mp_antenna_tx)
	{
	case ANTENNA_A:
		p_ofdm_tx->r_tx_antenna		= 0x1;
		r_ofdm_tx_en_val			= 0x1;
		p_ofdm_tx->r_ant_l 			= 0x1;
		p_ofdm_tx->r_ant_ht_s1 		= 0x1;
		p_ofdm_tx->r_ant_non_ht_s1 	= 0x1;
		p_cck_txrx->r_ccktx_enable	= 0x8;
		chgTx = 1;
			PHY_SetBBReg(priv, rFPGA0_XA_HSSIParameter2, 0xe, 2);
			PHY_SetBBReg(priv, rFPGA0_XB_HSSIParameter2, 0xe, 1);
			r_ofdm_tx_en_val			= 0x3;
			// Power save
			r_ant_select_ofdm_val = 0x11111111;

#ifdef HIGH_POWER_EXT_PA
			if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)){
			if(priv->pshare->rf_ft_var.use_ext_pa) {
			PHY_SetBBReg(priv, 0x870, BIT(26), 1);
			PHY_SetBBReg(priv, 0x870, BIT(10), 0);
			}
			}
#endif
		break;
	case ANTENNA_B:
		p_ofdm_tx->r_tx_antenna		= 0x2;
		r_ofdm_tx_en_val			= 0x2;
		p_ofdm_tx->r_ant_l 			= 0x2;
		p_ofdm_tx->r_ant_ht_s1 		= 0x2;
		p_ofdm_tx->r_ant_non_ht_s1 	= 0x2;
		p_cck_txrx->r_ccktx_enable	= 0x4;
		chgTx = 1;
			PHY_SetBBReg(priv, rFPGA0_XA_HSSIParameter2, 0xe, 1);
			PHY_SetBBReg(priv, rFPGA0_XB_HSSIParameter2, 0xe, 2);
			r_ofdm_tx_en_val			= 0x3;
			// Power save
			r_ant_select_ofdm_val = 0x22222222;

#ifdef HIGH_POWER_EXT_PA
		if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)){
		if (priv->pshare->rf_ft_var.use_ext_pa) {
			PHY_SetBBReg(priv, 0x870, BIT(26), 0);
			PHY_SetBBReg(priv, 0x870, BIT(10), 1);
			}
		}
#endif
		break;
	case ANTENNA_AB:
		p_ofdm_tx->r_tx_antenna		= 0x3;
		r_ofdm_tx_en_val			= 0x3;
		p_ofdm_tx->r_ant_l 			= 0x3;
		p_ofdm_tx->r_ant_ht_s1 		= 0x3;
		p_ofdm_tx->r_ant_non_ht_s1 	= 0x3;
		p_cck_txrx->r_ccktx_enable	= 0xC;
		chgTx = 1;
		PHY_SetBBReg(priv, rFPGA0_XA_HSSIParameter2, 0xe, 2);
		PHY_SetBBReg(priv, rFPGA0_XB_HSSIParameter2, 0xe, 2);
		// Disable Power save
		r_ant_select_ofdm_val		= 0x3321333;

#ifdef HIGH_POWER_EXT_PA
		if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)){
		if (priv->pshare->rf_ft_var.use_ext_pa) {
			PHY_SetBBReg(priv, 0x870, BIT(26), 0);
			PHY_SetBBReg(priv, 0x870, BIT(10), 0);
		}
		}
#endif
		break;
	default:
		break;
	}

	//
	// r_rx_antenna_ofdm, bit0=A, bit1=B, bit2=C, bit3=D
	// r_cckrx_enable : CCK default, 0=A, 1=B, 2=C, 3=D
	// r_cckrx_enable_2 : CCK option, 0=A, 1=B, 2=C, 3=D
	//
	switch(priv->pshare->mp_antenna_rx)
	{
	case ANTENNA_A:
		r_rx_antenna_ofdm 			= 0x1;	// A
		p_cck_txrx->r_cckrx_enable 	= 0x0;	// default: A
		p_cck_txrx->r_cckrx_enable_2= 0x0;	// option: A
		chgRx = 1;
		PHY_SetRFReg(priv, 0, 7, 0x3, 0x0);
		break;
	case ANTENNA_B:
		r_rx_antenna_ofdm 			= 0x2;	// B
		p_cck_txrx->r_cckrx_enable 	= 0x1;	// default: B
		p_cck_txrx->r_cckrx_enable_2= 0x1;	// option: B
		chgRx = 1;
		PHY_SetRFReg(priv, 0, 7, 0x3, 0x1);
		break;
	case ANTENNA_AB:	// For 8192S and 8192E/U...
		r_rx_antenna_ofdm 			= 0x3;	// AB
		p_cck_txrx->r_cckrx_enable 	= 0x0;	// default:A
		p_cck_txrx->r_cckrx_enable_2= 0x1;		// option:B
		chgRx = 1;
		break;
	}

	if(chgTx && chgRx) {
		r_ant_sel_cck_val = r_ant_select_cck_val;//(r_ant_select_cck_val<<24);
		PHY_SetBBReg(priv, rFPGA1_TxInfo, 0x0fffffff, r_ant_select_ofdm_val);		//OFDM Tx
		PHY_SetBBReg(priv, rFPGA0_TxInfo, 0x0000000f, r_ofdm_tx_en_val);		//OFDM Tx
		PHY_SetBBReg(priv, rOFDM0_TRxPathEnable, 0x0000000f, r_rx_antenna_ofdm);	//OFDM Rx
		PHY_SetBBReg(priv, rOFDM1_TRxPathEnable, 0x0000000f, r_rx_antenna_ofdm);	//OFDM Rx
		PHY_SetBBReg(priv, rCCK0_AFESetting, bMaskByte3, r_ant_sel_cck_val);		//CCK TxRx
	}
}


#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
void mp_set_ant_tx(struct rtl8192cd_priv *priv, unsigned char *data)
{
	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
		return;
	}

	if (!strcmp(data, "a")) {
		priv->pshare->mp_antenna_tx = ANTENNA_A;
	} else if (!strcmp(data, "b")) {
		priv->pshare->mp_antenna_tx = ANTENNA_B;
	} else if  (!strcmp(data, "ab")) {
		priv->pshare->mp_antenna_tx = ANTENNA_AB;
	} else {
		printk("Usage: mp_ant_tx {a,b,ab}\n");
		return;
	}

	mp_chk_sw_ant(priv);
	mp_8192CD_tx_setting(priv);

	printk("switch Tx antenna to %s\n", data);
}


#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
void mp_set_ant_rx(struct rtl8192cd_priv *priv, unsigned char *data)
{
	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
		return;
	}

	if (!strcmp(data, "a")) {
		priv->pshare->mp_antenna_rx = ANTENNA_A;
	} else if (!strcmp(data, "b")) {
		priv->pshare->mp_antenna_rx = ANTENNA_B;
	} else if (!strcmp(data, "ab")) {
		priv->pshare->mp_antenna_rx = ANTENNA_AB;
	} else {
		printk("Usage: mp_ant_rx {a,b,ab}\n");
		return;
	}

	mp_chk_sw_ant(priv);
	printk("switch Rx antenna to %s\n", data);
}


void mp_set_phypara(struct rtl8192cd_priv *priv, unsigned char *data)
{
	char *val;
	int xcap=-32, sign;

	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return;
	}

	
	if (GET_CHIP_VER(priv) != VERSION_8192D && GET_CHIP_VER(priv) != VERSION_8188E) {
		printk("Fail: xcap is for 92D or 88E only\n");
		return;
	}
	
	// get CrystalCap value
	val = get_value_by_token((char *)data, "xcap=");
	if (val) {
		if (*val == '-') {
			sign = 1;
			val++;
		}
		else
			sign = 0;

		xcap = _atoi(val, 10);
		if (sign)
			xcap = 0 - xcap;
	}

	// set CrystalCap value
	if(GET_CHIP_VER(priv) == VERSION_8192D){
		if (xcap != -32) {
			/*
			PHY_SetBBReg(priv, rFPGA0_AnalogParameter1, bXtalCap01, (xcap & 0x00000003));
			PHY_SetBBReg(priv, rFPGA0_AnalogParameter2, bXtalCap23, ((xcap & 0x0000000c) >> 2));
			*/
			PHY_SetBBReg(priv, 0x24, 0xF0, xcap & 0x0F);
			PHY_SetBBReg(priv, 0x28, 0xF0000000, ((xcap & 0xF0) >> 4));
		}
	} 
#ifdef CONFIG_RTL_88E_SUPPORT 
	if(GET_CHIP_VER(priv) == VERSION_8188E){
		if(xcap > 0 && xcap < 0x3F) {
			PHY_SetBBReg(priv, 0x24, BIT(11)|BIT(12)|BIT(13)|BIT(14)|BIT(15)|BIT(16), xcap & 0x3F);
			PHY_SetBBReg(priv, 0x24, BIT(17)|BIT(18)|BIT(19)|BIT(20)|BIT(21)|BIT(22), xcap & 0x3F);
		}
	}
#endif
	printk("Set xcap=%d\n", xcap);
}


#ifdef CONFIG_RTL_92D_SUPPORT
void mp_set_phyBand(struct rtl8192cd_priv *priv, unsigned char *data)
{
	unsigned int eRFPath, curMaxRFPath=((priv->pmib->dot11RFEntry.macPhyMode==SINGLEMAC_SINGLEPHY)?RF92CD_PATH_MAX : RF92CD_PATH_B);

	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
		return;
	}

	if (!strcmp(data, "a")) {
		if (!(priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G)) {
			priv->pmib->dot11RFEntry.phyBandSelect = PHY_BAND_5G;
		} else {
			printk("Fail: It is already set as A band\n");
			return;
		}
	} else if (!strcmp(data, "bg")) {
		if (!(priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G)) {
			priv->pmib->dot11RFEntry.phyBandSelect = PHY_BAND_2G;
		} else {
			printk("Fail: It is already set as BG band\n");
			return;
		}
	} else {
		printk("Usage: mp_phyband {a, bg}\n");
		return;
	}

	/*
	 *	Stop RX/Tx 
	 */
	PHY_SetBBReg(priv, rOFDM0_TRxPathEnable, bMaskByte0, 0x00);
	PHY_SetBBReg(priv, rOFDM1_TRxPathEnable, bDWord, 0x0);

	/*
	 *	Reconfig BB/RF according to wireless mode
	 */
	/* mac config */
	if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G)
		RTL_W8(BWOPMODE, (RTL_R8(BWOPMODE) & ~BW_OPMODE_5G));
	else /* if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) */
		RTL_W8(BWOPMODE, RTL_R8(BWOPMODE) | BW_OPMODE_5G);

	/* BB & RF Config */
	PHY_SetBBReg(priv, rFPGA0_RFMOD, BIT(25) | BIT(24), 0);
	PHY_ConfigBBWithParaFile(priv, AGCTAB);
	PHY_SetBBReg(priv, rFPGA0_RFMOD, BIT(25) | BIT(24), 3);
	
	for(eRFPath = RF92CD_PATH_A; eRFPath <curMaxRFPath; eRFPath++) {
		priv->pshare->RegRF18[eRFPath] = PHY_QueryRFReg(priv, eRFPath, 0x18, bMask20Bits, 1);
		priv->pshare->RegRF28[eRFPath] = PHY_QueryRFReg(priv, eRFPath, 0x28, bMask20Bits, 1);
	}

	UpdateBBRFVal8192DE(priv);

	// 2010/09/30 protect cck
	if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
		RTL_W16(RRSR, 0x150);
	} else{
		RTL_W16(RRSR, 0x15D);
	}

	/* 
	 *	Recovery RX/Tx
	 */
	PHY_SetBBReg(priv, rOFDM0_TRxPathEnable, bMaskByte0, 0x33);
	PHY_SetBBReg(priv, rOFDM1_TRxPathEnable, bDWord, 0x3);

	/*
	 *	IQK
	 */
	PHY_IQCalibrate(priv);
	/*
	 *	Set Rx AGC
	 */
	if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
		RTL_W8(0xc50, 0x1c);
		RTL_W8(0xc58, 0x1c);
	} else {
		RTL_W8(0xc50, 0x20);
		RTL_W8(0xc58, 0x20);
	}

	if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G)
		printk("Set band as A band\n");
	else
		printk("Set band as BG band\n");
}
#endif


void mp_reset_stats(struct rtl8192cd_priv *priv)
{
	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
		return;
	}

	memset(&priv->net_stats, 0,  sizeof(struct net_device_stats));
	memset(&priv->ext_stats, 0,  sizeof(struct extra_stats));
}
#endif	// MP_TEST

