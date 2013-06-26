//============================================================
//
// File Name: Hal8192CDMOutSrc.c 
//
// Description:
//
// This file is for 92CE/92CU outsource dynamic mechanism for partner.
//
//
//============================================================

#ifndef _HAL8192CDM_C_
#define _HAL8192CDM_C_

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <asm/unistd.h>
#include <linux/synclink.h>
#endif

#include "../../8192cd_cfg.h"
#include "../../8192cd.h"
#include "../../8192cd_hw.h"
#include "../../8192cd_headers.h"
#include "../../8192cd_debug.h"


#ifdef __KERNEL__
#ifdef __LINUX_2_6__
#include <linux/syscalls.h>
#else
#include <linux/fs.h>
#endif
#endif

#ifdef USE_OUT_SRC
#include "../odm_precomp.h"
#include "HalDMOutSrc8192C_AP.h"
#endif

//============================================================
// Global var
//============================================================

#ifdef CONFIG_RTL_92D_SUPPORT


#ifdef SW_LCK_92D

#define TARGET_CHNL_NUM_5G 221
#define TARGET_CHNL_NUM_2G 14
#define CV_CURVE_CNT 64

unsigned int CurveIndex_5G[TARGET_CHNL_NUM_5G]={0};
unsigned int CurveIndex_2G[TARGET_CHNL_NUM_2G]={0};

static unsigned int TargetChnl_5G[TARGET_CHNL_NUM_5G] = {
	25141,	25116,	25091,	25066,	25041,
	25016,	24991,	24966,	24941,	24917,
	24892,	24867,	24843,	24818,	24794,
	24770,	24765,	24721,	24697,	24672,
	24648,	24624,	24600,	24576,	24552,
	24528,	24504,	24480,	24457,	24433,
	24409,	24385,	24362,	24338,	24315,
	24291,	24268,	24245,	24221,	24198,
	24175,	24151,	24128,	24105,	24082,
	24059,	24036,	24013,	23990,	23967,
	23945,	23922,	23899,	23876,	23854,
	23831,	23809,	23786,	23764,	23741,
	23719,	23697,	23674,	23652,	23630,
	23608,	23586,	23564,	23541,	23519,
	23498,	23476,	23454,	23432,	23410,
	23388,	23367,	23345,	23323,	23302,
	23280,	23259,	23237,	23216,	23194,
	23173,	23152,	23130,	23109,	23088,
	23067,	23046,	23025,	23003,	22982,
	22962,	22941,	22920,	22899,	22878,
	22857,	22837,	22816,	22795,	22775,
	22754,	22733,	22713,	22692,	22672,
	22652,	22631,	22611,	22591,	22570,
	22550,	22530,	22510,	22490,	22469,
	22449,	22429,	22409,	22390,	22370,
	22350,	22336,	22310,	22290,	22271,
	22251,	22231,	22212,	22192,	22173,
	22153,	22134,	22114,	22095,	22075,
	22056,	22037,	22017,	21998,	21979,
	21960,	21941,	21921,	21902,	21883,
	21864,	21845,	21826,	21807,	21789,
	21770,	21751,	21732,	21713,	21695,
	21676,	21657,	21639,	21620,	21602,
	21583,	21565,	21546,	21528,	21509,
	21491,	21473,	21454,	21436,	21418,
	21400,	21381,	21363,	21345,	21327,
	21309,	21291,	21273,	21255,	21237,
	21219,	21201,	21183,	21166,	21148,
	21130,	21112,	21095,	21077,	21059,
	21042,	21024,	21007,	20989,	20972,
	25679,	25653,	25627,	25601,	25575,
	25549,	25523,	25497,	25471,	25446,
	25420,	25394,	25369,	25343,	25318,
	25292,	25267,	25242,	25216,	25191,
	25166	};

static unsigned int TargetChnl_2G[TARGET_CHNL_NUM_2G] = {	// channel 1~14
	26084, 26030, 25976, 25923, 25869, 25816, 25764,
	25711, 25658, 25606, 25554, 25502, 25451, 25328
};

#endif


#ifdef DPK_92D

#define DPK_DEBUG(fmt,args...) 

#define		DP_BB_REG_NUM		7
//#define		DP_BB_REG_NUM_A	11
//#define		DP_BB_REG_NUM_B	10
#define		DP_BB_REG_NUM_A	10
#define		DP_BB_REG_NUM_B	9

#define		DP_BB_REG_NUM_settings	6
#define		DP_BB_REG_NUM_loop	30
#define		DP_BB_REG_NUM_loop_tx	12
#define		DP_BB_REG_NUM_loop_rx	8
#define		DP_BB_REG_NUM_loop_pa	4
#define		DP_RF_REG_NUM		4
#define		DP_SRAM_NUM		16
//#define		DP_SRAM_NUM_db		22
#define		DP_SRAM_NUM_db		86

#define		DP_PATH_NUM		2
#define		DP_PA_MODEL_NUM	32
#define		DP_PA_MODEL_RUN_NUM	8
#define		DP_PA_MODEL_PER_RUN_NUM	4
#define		DP_RETRY_LIMIT		10
#define		DP_DPK_NUM			3
#define		DP_DPK_VALUE_NUM	2
#if 1
#define		DP_GAIN_LOSS_BOUND_NUM	14
#else
#define		DP_GAIN_LOSS_BOUND_NUM	8
#endif
#define		DP_OFFSET_NUM		9
#define		DP_AP_CUREVE_SELECT_NUM		2	// 3
#define		DP_gain_loss	1

#endif
#endif


#ifdef HW_ANT_SWITCH
#define RXDVY_A_EN		((HW_DIV_ENABLE && !priv->pshare->rf_ft_var.antSw_select) ? 0x80 : 0)
#define RXDVY_B_EN		((HW_DIV_ENABLE &&  priv->pshare->rf_ft_var.antSw_select) ? 0x80 : 0)
#endif




void DIG_process(struct rtl8192cd_priv *priv)
{
	#define DEAD_POINT_TH		10000
	#define DOWN_IG_HIT_TH		5
	#define DEAD_POINT_HIT_TH	3

	unsigned char value_IGI;
	signed char value8;

#ifdef INTERFERENCE_CONTROL
	unsigned short thd0, thd1, thd2;
#endif

	if (priv->pshare->DIG_on == 1)
	{
		if (priv->pshare->rf_ft_var.use_ext_lna == 1) {
//			priv->pshare->FA_upper = 0x42;
			priv->pshare->FA_upper = MIN_NUM(0x42, priv->pshare->rssi_min+36);
			priv->pshare->FA_lower = 0x30;
		} else {
			// Reset initial gain upper & lower bounds
#ifdef DFS
			if (!priv->pmib->dot11DFSEntry.disable_DFS &&
				(OPMODE & WIFI_AP_STATE) &&
				(((priv->pmib->dot11RFEntry.dot11channel >= 52) &&
				(priv->pmib->dot11RFEntry.dot11channel <= 64)) ||
				((priv->pmib->dot11RFEntry.dot11channel >= 100) &&
				(priv->pmib->dot11RFEntry.dot11channel <= 140))))
				priv->pshare->FA_upper = 0x24;
			else
#endif
			{
#ifdef INTERFERENCE_CONTROL
				priv->pshare->FA_lower = 0x20;

				if (priv->pshare->rssi_min != 0xFF)
				{
//					priv->pshare->FA_upper = 0x3E;
					
					if (priv->pshare->rssi_min > 30)
						priv->pshare->FA_lower = 0x24;
					else if (priv->pshare->rssi_min > 25)
						priv->pshare->FA_lower = 0x22;

					// limit upper bound to prevent the minimal signal sta from disconnect
//					if ((priv->pshare->rssi_min + 10) < priv->pshare->FA_upper)
//						priv->pshare->FA_upper = priv->pshare->rssi_min + 10;
					priv->pshare->FA_upper = MIN_NUM(0x3E, priv->pshare->rssi_min+20);

					thd0 = priv->pshare->threshold0;
					thd1 = priv->pshare->threshold1;
					thd2 = priv->pshare->threshold2;
				}
				else		// before link
				{
					priv->pshare->FA_upper = 0x32;
					
					thd0 = 500;
					thd1 = 8000;
					thd2 = 10000;
				}
#else
				if (priv->pmib->dot11RFEntry.tx2path) {
					if (priv->pmib->dot11BssType.net_work_type == WIRELESS_11B)
						priv->pshare->FA_upper = MIN_NUM(0x2A, priv->pshare->rssi_min+20);
					else
						priv->pshare->FA_upper = MIN_NUM(0x32, priv->pshare->rssi_min+20);
				}
				else
					priv->pshare->FA_upper = MIN_NUM(0x32, priv->pshare->rssi_min+20);
				priv->pshare->FA_lower = 0x20;

				if (priv->pshare->rssi_min > 30)
					priv->pshare->FA_lower = 0x24;
				else if (priv->pshare->rssi_min > 25)
					priv->pshare->FA_lower = 0x22;
#endif
			}
		}

		// determine a new initial gain index according to the sumation of all FA counters as well as upper & lower bounds
		value8 = RTL_R8(0xc50);
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
			if(priv->pshare->rf_ft_var.one_path_cca==2)
				value8 = RTL_R8(0xc58);
		}
#endif

		value_IGI = (value8 & 0x7F);

#if  defined(CONFIG_RTL_NOISE_CONTROL_92C)
	if(priv->pshare->rf_ft_var.dnc_enable)
	if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)){
		unsigned long tp_now = (priv->ext_stats.tx_avarage+priv->ext_stats.rx_avarage)>>17;
		if(priv->pshare->rf_ft_var.use_ext_lna) {
			if( (priv->pshare->rssi_min > 50)  )  {
				if((!priv->pshare->DNC_on) && (value_IGI >= priv->pshare->FA_upper) && (priv->pshare->FA_total_cnt > priv->pshare->threshold2)) {
					priv->pshare->DNC_on = 1;
					priv->ext_stats.tp_average_pre = tp_now;
					priv->pshare->FA_lower = 0x20;
					PHY_SetBBReg(priv, 0x870, bMaskDWord, RTL_R32(0x870)|BIT(5)|BIT(6)|BIT(21)|BIT(22));

#ifdef HW_ANT_SWITCH
					PHY_SetBBReg(priv, 0xc50, bMaskByte0, priv->pshare->FA_lower | RXDVY_A_EN);
					PHY_SetBBReg(priv, 0xc58, bMaskByte0, priv->pshare->FA_lower | RXDVY_B_EN);
#else
					PHY_SetBBReg(priv, 0xc50, bMaskByte0, priv->pshare->FA_lower);
					PHY_SetBBReg(priv, 0xc58, bMaskByte0, priv->pshare->FA_lower);
#endif

				} else if(priv->pshare->DNC_on ==1)  {
					if(tp_now < priv->ext_stats.tp_average_pre + 2) {
						priv->pshare->DNC_on = 0;
					}
					else {
						priv->pshare->DNC_on =2;
						priv->ext_stats.tp_average_pre = tp_now;
					}
				} else if(priv->pshare->DNC_on >= 2 ) {
					if(( tp_now+10 < priv->ext_stats.tp_average_pre ) || (tp_now < 1) ) {
							priv->pshare->DNC_on = 0;
					} else if(priv->pshare->DNC_on<5) {
						priv->ext_stats.tp_average_pre = tp_now;
						++priv->pshare->DNC_on;
					}
				}
			 }else {
				priv->pshare->DNC_on = 0;
			}

			if(	priv->pshare->DNC_on )
				return;
			else
				PHY_SetBBReg(priv, 0x870, bMaskDWord, RTL_R32(0x870)&  ~(BIT(5)|BIT(6)|BIT(21)|BIT(22)));

		} else {
			if( (priv->pshare->rssi_min > 40) && (value_IGI >= priv->pshare->FA_upper) )  {
//				unsigned long tp_now = (priv->ext_stats.tx_avarage+priv->ext_stats.rx_avarage)>>17;
				if((!priv->pshare->DNC_on) && (priv->pshare->FA_total_cnt > priv->pshare->threshold2)) {
					priv->pshare->DNC_on = 1;
					priv->ext_stats.tp_average_pre = tp_now;
				} else if(priv->pshare->DNC_on ==1)  {
					if(tp_now < priv->ext_stats.tp_average_pre + 2) {
						priv->pshare->DNC_on = 0;
					}
					else {
						priv->pshare->DNC_on = 2;
						priv->ext_stats.tp_average_pre = tp_now;
					}
				} else if(priv->pshare->DNC_on >= 2 ) {
					if((tp_now +10 < priv->ext_stats.tp_average_pre )
						|| ((priv->ext_stats.tp_average_pre < 10) && (priv->pshare->FA_total_cnt < priv->pshare->threshold1))) {
						priv->pshare->DNC_on = 0;
					} else 	if(priv->pshare->DNC_on<6) {
						priv->ext_stats.tp_average_pre = tp_now;
						++priv->pshare->DNC_on;
					}
				}
				if(priv->pshare->DNC_on) {
					priv->pshare->FA_upper = 0x3e;
				}
			}else {
				priv->pshare->DNC_on = 0;
			}
		}
	}
#endif

		if ((priv->pshare->digDeadPoint == 0) && (priv->pshare->FA_total_cnt > DEAD_POINT_TH)) {
			if ((priv->pshare->digDeadPointHitCount > 0) && (priv->pshare->digDeadPointCandidate == value_IGI)) {
				priv->pshare->digDeadPointHitCount++;
				if (priv->pshare->digDeadPointHitCount == DEAD_POINT_HIT_TH) {
					priv->pshare->digDeadPoint = priv->pshare->digDeadPointCandidate;
				}
			} else {
				priv->pshare->digDeadPointCandidate = value_IGI;
				priv->pshare->digDeadPointHitCount = 1;
			}
		}

#ifdef INTERFERENCE_CONTROL
		if (priv->pshare->FA_total_cnt < thd0) {
#else
		if (priv->pshare->FA_total_cnt < priv->pshare->threshold0) {
#endif
			priv->pshare->digDownCount++;
			if (priv->pshare->digDownCount > DOWN_IG_HIT_TH) {
				// Reset deadpoint hit count
				if ((priv->pshare->digDeadPoint == 0) && (priv->pshare->digDeadPointHitCount > 0) && (value_IGI == priv->pshare->digDeadPointCandidate))
					priv->pshare->digDeadPointHitCount = 0;

				value_IGI--;

				// Check if the new value is dead point
				if ((priv->pshare->digDeadPoint > 0) && (value_IGI == priv->pshare->digDeadPoint))
					value_IGI++;
			}
#ifdef INTERFERENCE_CONTROL
		} else if (priv->pshare->FA_total_cnt < thd1) {
#else
		} else if (priv->pshare->FA_total_cnt < priv->pshare->threshold1) {
#endif
			value_IGI += 0;
			priv->pshare->digDownCount = 0;
#ifdef INTERFERENCE_CONTROL
		} else if (priv->pshare->FA_total_cnt < thd2) {
#else
		} else if (priv->pshare->FA_total_cnt < priv->pshare->threshold2) {
#endif
			value_IGI++;
			priv->pshare->digDownCount = 0;
#ifdef INTERFERENCE_CONTROL
		} else if (priv->pshare->FA_total_cnt >= thd2) {
#else
		} else if (priv->pshare->FA_total_cnt >= priv->pshare->threshold2) {
#endif
			value_IGI += 2;
			priv->pshare->digDownCount = 0;
		} else {
			priv->pshare->digDownCount = 0;
		}

		if (value_IGI > priv->pshare->FA_upper)
			value_IGI = priv->pshare->FA_upper;
		else if (value_IGI < priv->pshare->FA_lower)
			value_IGI = priv->pshare->FA_lower;

#if defined(HW_ANT_SWITCH)
		// wirte new initial gain index into regC50/C58
		if (priv->pshare->rf_ft_var.one_path_cca == 0)	{
			RTL_W8(0xc50, value_IGI | RXDVY_A_EN);
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
				RTL_W8(0xc58, value_IGI | RXDVY_B_EN);
#endif
		} else if (priv->pshare->rf_ft_var.one_path_cca == 1)	{
			RTL_W8(0xc50, value_IGI | RXDVY_A_EN);
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
				RTL_W8(0xc58, getIGIFor1RCCA(value_IGI) | RXDVY_B_EN);
#endif
		} else if (priv->pshare->rf_ft_var.one_path_cca == 2)		{
			RTL_W8(0xc50, getIGIFor1RCCA(value_IGI) | RXDVY_A_EN);
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
				RTL_W8(0xc58, value_IGI| RXDVY_B_EN);
#endif
		}
#else
		// Write IGI into HW
		if (priv->pshare->rf_ft_var.one_path_cca == 0) {
			RTL_W8(0xc50, value_IGI);
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
				RTL_W8(0xc58, value_IGI);
#endif
		} else if (priv->pshare->rf_ft_var.one_path_cca == 1) {
			RTL_W8(0xc50, value_IGI);
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
				RTL_W8(0xc58, getIGIFor1RCCA(value_IGI));
#endif
		} else if (priv->pshare->rf_ft_var.one_path_cca == 2) {
			RTL_W8(0xc50, getIGIFor1RCCA(value_IGI));
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
				RTL_W8(0xc58, value_IGI);
#endif
		}
#endif

	}
}



#ifdef INTERFERENCE_CONTROL
void check_NBI_by_rssi(struct rtl8192cd_priv *priv, unsigned char rssi_strength)
{
	if (OPMODE & WIFI_SITE_MONITOR)
		return;

	if (priv->pshare->phw->nbi_filter_on) {
		if (rssi_strength < 20) {
			priv->pshare->phw->nbi_filter_on = 0;
			RTL_W16(rOFDM0_RxDSP, RTL_R16(rOFDM0_RxDSP) & ~ BIT(9));	// NBI off
		}
	} else {	// NBI OFF previous
		if (rssi_strength > 25) {
			priv->pshare->phw->nbi_filter_on = 1;
			RTL_W16(rOFDM0_RxDSP, RTL_R16(rOFDM0_RxDSP) | BIT(9));		// NBI on
		}
	}
}
#endif

/*
 * dynamic CCK CCA enhance by rssi
 */
void CCK_CCA_dynamic_enhance(struct rtl8192cd_priv *priv, unsigned char rssi_strength)
{
#if 1
	unsigned int cck_fa = priv->pshare->FA_total_cnt;
	int rssi_thd = 30;

	if (rssi_strength == 0xff) {
		if (cck_fa < 1000) {
			if (priv->pshare->phw->CCK_CCA_enhanced != 2) {
				RTL_W8(0xa0a, 0x40);
				priv->pshare->phw->CCK_CCA_enhanced = 2;
			}
		} else {
			if (priv->pshare->phw->CCK_CCA_enhanced != 1) {
				RTL_W8(0xa0a, 0x83);
				priv->pshare->phw->CCK_CCA_enhanced = 1;
			}
		}
		return;
	}

	if (rssi_strength > rssi_thd+5) {
		if (priv->pshare->phw->CCK_CCA_enhanced != 0) {
			RTL_W8(0xa0a, 0xcd);
			priv->pshare->phw->CCK_CCA_enhanced = 0;
		}
	} else if (rssi_strength< rssi_thd) {
		if ((rssi_strength > 9) || (priv->assoc_num >1)) {
			if (priv->pshare->phw->CCK_CCA_enhanced != 1) {
				RTL_W8(0xa0a, 0x83);
				priv->pshare->phw->CCK_CCA_enhanced = 1;
			}
		} else {
			if(cck_fa<1000) {
				if (priv->pshare->phw->CCK_CCA_enhanced != 2) {
					RTL_W8(0xa0a, 0x40);
					priv->pshare->phw->CCK_CCA_enhanced = 2;
				}
			} else {
				if (priv->pshare->phw->CCK_CCA_enhanced != 1) {
					RTL_W8(0xa0a, 0x83);
					priv->pshare->phw->CCK_CCA_enhanced = 1;
				}
			}
		}
	}

#else

	if (rssi_strength == 0xff)
		return;

	if (!priv->pshare->phw->CCK_CCA_enhanced && (rssi_strength < 30)) {
		priv->pshare->phw->CCK_CCA_enhanced = TRUE;
		RTL_W8(0xa0a, 0x83);
	}
	else if (priv->pshare->phw->CCK_CCA_enhanced && (rssi_strength > 35)) {
		priv->pshare->phw->CCK_CCA_enhanced = FALSE;
		RTL_W8(0xa0a, 0xcd);
	}
#endif
}


//3 ============================================================
//3 Dynamic Tx Power / Power Tracking
//3 ============================================================

#ifdef HIGH_POWER_EXT_PA
void tx_power_control(struct rtl8192cd_priv *priv)
{
	unsigned long x;

	int pwr_value = 0x10101010;
	if( priv->pshare->phw->signal_strength == 3 && priv->pshare->phw->lower_tx_power== 0) {
		SAVE_INT_AND_CLI(x);
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
		priv->pshare->phw->power_backup[0x0c] = RTL_R32(rTxAGC_A_CCK11_2_B_CCK11);
		priv->pshare->phw->power_backup[0x0d] = RTL_R32(rTxAGC_A_CCK1_Mcs32);
		priv->pshare->phw->power_backup[0x0e] = RTL_R32(rTxAGC_B_CCK5_1_Mcs32);
		RTL_W32(rTxAGC_A_Rate18_06, pwr_value);
		RTL_W32(rTxAGC_A_Rate54_24, pwr_value);
		RTL_W32(rTxAGC_B_Rate18_06, pwr_value);
		RTL_W32(rTxAGC_B_Rate54_24, pwr_value);
		RTL_W32(rTxAGC_A_Mcs03_Mcs00, pwr_value);
		RTL_W32(rTxAGC_A_Mcs07_Mcs04, pwr_value);
		RTL_W32(rTxAGC_A_Mcs11_Mcs08, pwr_value);
		RTL_W32(rTxAGC_A_Mcs15_Mcs12, pwr_value);
		RTL_W32(rTxAGC_B_Mcs03_Mcs00, pwr_value);
		RTL_W32(rTxAGC_B_Mcs07_Mcs04, pwr_value);
		RTL_W32(rTxAGC_B_Mcs11_Mcs08, pwr_value);
		RTL_W32(rTxAGC_B_Mcs15_Mcs12, pwr_value);
		RTL_W32(rTxAGC_A_CCK11_2_B_CCK11, pwr_value);
		RTL_W32(rTxAGC_A_CCK1_Mcs32, (pwr_value & 0x0000ff00) | (priv->pshare->phw->power_backup[0x0d] &0xffff00ff));
		RTL_W32(rTxAGC_B_CCK5_1_Mcs32, (pwr_value & 0xffffff00) | (priv->pshare->phw->power_backup[0x0e] &0x000000ff));
		priv->pshare->phw->lower_tx_power = 1;
		RESTORE_INT(x);
	}
	else if( priv->pshare->phw->signal_strength != 3 && priv->pshare->phw->lower_tx_power) {
		SAVE_INT_AND_CLI(x);
		RTL_W32(rTxAGC_A_Rate18_06, priv->pshare->phw->power_backup[0x00]);
		RTL_W32(rTxAGC_A_Rate54_24, priv->pshare->phw->power_backup[0x01]);
		RTL_W32(rTxAGC_B_Rate18_06, priv->pshare->phw->power_backup[0x02]);
		RTL_W32(rTxAGC_B_Rate54_24, priv->pshare->phw->power_backup[0x03]);
		RTL_W32(rTxAGC_A_Mcs03_Mcs00, priv->pshare->phw->power_backup[0x04]);
		RTL_W32(rTxAGC_A_Mcs07_Mcs04, priv->pshare->phw->power_backup[0x05]);
		RTL_W32(rTxAGC_A_Mcs11_Mcs08, priv->pshare->phw->power_backup[0x06]);
		RTL_W32(rTxAGC_A_Mcs15_Mcs12, priv->pshare->phw->power_backup[0x07]);
		RTL_W32(rTxAGC_B_Mcs03_Mcs00, priv->pshare->phw->power_backup[0x08]);
		RTL_W32(rTxAGC_B_Mcs07_Mcs04, priv->pshare->phw->power_backup[0x09]);
		RTL_W32(rTxAGC_B_Mcs11_Mcs08, priv->pshare->phw->power_backup[0x0a]);
		RTL_W32(rTxAGC_B_Mcs15_Mcs12, priv->pshare->phw->power_backup[0x0b]);
		RTL_W32(rTxAGC_A_CCK11_2_B_CCK11, priv->pshare->phw->power_backup[0x0c]);
		RTL_W32(rTxAGC_A_CCK1_Mcs32,  priv->pshare->phw->power_backup[0x0d]);
		RTL_W32(rTxAGC_B_CCK5_1_Mcs32,  priv->pshare->phw->power_backup[0x0e]);
		priv->pshare->phw->lower_tx_power = 0;
		RESTORE_INT(x);
	}
}
#endif


int get_CCK_swing_index(struct rtl8192cd_priv *priv)
{
	int TempCCk, index=12, i;
	short channel;
#ifdef MP_TEST
	if ((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific)
		channel=priv->pshare->working_channel;
	else
#endif
		channel = (priv->pmib->dot11RFEntry.dot11channel);

	//Query CCK default setting From 0xa24
	TempCCk = PHY_QueryBBReg(priv, rCCK0_TxFilter2, bMaskDWord)&bMaskCCK;
	TempCCk = cpu_to_le32(TempCCk);
	for(i=0 ; i<CCK_TABLE_SIZE ; i++)		{
		if(channel==14) {
			if(memcmp((void*)&TempCCk, (void*)&CCKSwingTable_Ch14[i][2], 4)==0) {
				index = i;
				break;
			}
		} else {
			if(memcmp((void*)&TempCCk, (void*)&CCKSwingTable_Ch1_Ch13[i][2], 4)==0) {
				index = i;
				break;
			}
		}
	}
	DEBUG_INFO("Initial reg0x%x = 0x%lx, CCK_index=0x%x, ch %d\n",
							rCCK0_TxFilter2, TempCCk, index, channel);
	return index;
}

unsigned char getThermalValue(struct rtl8192cd_priv *priv)
{
	unsigned char	ThermalValue;
	int sum=0, i=0;
	PHY_SetRFReg(priv, RF92CD_PATH_A, RF_T_METER, bMask20Bits, 0x60);
	while ((PHY_QueryRFReg(priv, RF92CD_PATH_A, RF_T_METER, bMask20Bits, 1) > 0x1f) && ((i++) < 1000)) {//<20ms, test is in 20 us
		delay_us(20);
	}	
	ThermalValue =(unsigned char)PHY_QueryRFReg(priv, RF92CD_PATH_A, RF_T_METER, bMask20Bits, 1) & 0x01f;
	priv->pshare->Thermal_idx = (priv->pshare->Thermal_idx+1)%8;
	priv->pshare->Thermal_log[ priv->pshare->Thermal_idx ] = ThermalValue;
	for(i=0; i<8; i++) {
		if(!priv->pshare->Thermal_log[i])
			return ThermalValue;
		sum += priv->pshare->Thermal_log[i];
	}
	return (sum+4)>>3;
}



#ifdef _TRACKING_TABLE_FILE
int get_tx_tracking_index(struct rtl8192cd_priv *priv, int channel, int i, int delta, int is_decrease, int is_CCK)
{
	int index = 0;

	if(delta == 0)
		return 0; 

	if(delta > index_mapping_NUM_MAX)
		delta = index_mapping_NUM_MAX;

	printk("\n\n_eric_tracking +++ channel = %d, i = %d, delta = %d, is_decrease = %d, is_CCK = %d\n", 
					channel, i, delta, is_decrease, is_CCK);

	delta = delta - 1;

	if (channel > 14)
	{
		if(channel <= 99)
		{
			index = priv->pshare->txpwr_tracking_5GL[(i*2)+ is_decrease][delta];
		}
		else if(channel <= 140)
		{

			index = priv->pshare->txpwr_tracking_5GM[(i*2)+ is_decrease][delta];
		}
		else
		{
			index = priv->pshare->txpwr_tracking_5GH[(i*2)+ is_decrease][delta];
		}
	}
	else
	{
		if(is_CCK)
		{
			index = priv->pshare->txpwr_tracking_2G_CCK[(i*2)+ is_decrease][delta];
		}
		else
		{
			index = priv->pshare->txpwr_tracking_2G_OFDM[(i*2)+ is_decrease][delta];
		}
	}

	printk("_eric_tracking +++ offset = %d\n\n", index);

	return index; 

}
#endif

#ifdef CONFIG_RTL_92C_SUPPORT	

#ifdef HIGH_POWER_EXT_PA
void swingIndexRemap2(int *a, int b, int i)
{

	u8		index_mapping_HighPower_92C[4][15] = {
			{0,  1,  3,  4,  6,  7,  9, 10, 12, 13, 15, 16, 18, 18, 18}, //2.4G, path A/MAC 0, decrease power
			{0,  2,  4,  5,  7,  8, 10, 11, 13, 14, 16, 17, 19, 20, 22}, //2.4G, path A/MAC 0, increase power
			{0,  1,  3,  4,  6,  7,  9, 10, 12, 13, 15, 16, 18, 18, 18}, //2.4G, path A/MAC 0, decrease power
			{0,  3,  5,  6,  8,  9, 11, 12, 14, 15, 17, 18, 20, 21, 23}, //2.4G, path A/MAC 0, increase power
			};

	int d = RTL_ABS(*a, b);
	int offset = 0;


	if(i == 0)
		offset = 1;
	else 
		offset = 3;

	if(*a < b )
	{
		//printk("\n\n  Increase Power !! \n\n");
		*a = b - index_mapping_HighPower_92C[offset][d];
	}
	else
	{
		//printk("\n\n  Decrease Power !! \n\n");
		offset = offset - 1;
		*a = b + index_mapping_HighPower_92C[offset][d];
	}

	//printk("\n\ a = %d, b = %d, offset = %d, d = %d, diff = %d \n\n", 
		//*a, b, offset, d, index_mapping_HighPower_92C[offset][d]);
	
}
void swingIndexRemap(int *a, int b)
{
	int d = (RTL_ABS(*a, b) *3)>>1;
	if(*a < b )
		*a = b - d;
	else
		*a = b + d;
}
#endif

void tx_power_tracking(struct rtl8192cd_priv *priv)
{
	unsigned char	ThermalValue = 0, delta, delta_LCK, delta_IQK;
	int 			ele_A, ele_D, value32, X, Y, ele_C;
	int			OFDM_index[2]={0,0}, CCK_index;
	int	    		i = 0;
	char			is2T = ((GET_CHIP_VER(priv) == VERSION_8192C) ?1 :0);
	unsigned char		TxPwrLevel[2];
	unsigned char 		channel, OFDM_min_index = 6, rf=1; //OFDM BB Swing should be less than +3.0dB, which is required by Arthur
#ifdef MP_TEST
	if ((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific) {
		channel=priv->pshare->working_channel;
		if(priv->pshare->mp_txpwr_tracking == FALSE)
			return;
	} else
#endif
	{
		channel = (priv->pmib->dot11RFEntry.dot11channel);
	}

	ThermalValue = getThermalValue(priv);

	rf += is2T;
	if(ThermalValue)	{

		if(!priv->pshare->ThermalValue)	{
			priv->pshare->ThermalValue = priv->pmib->dot11RFEntry.ther;
			priv->pshare->ThermalValue_LCK = ThermalValue;
			priv->pshare->ThermalValue_IQK = ThermalValue;

			//Query OFDM path A default setting
			ele_D = PHY_QueryBBReg(priv, rOFDM0_XATxIQImbalance, bMaskDWord)&bMaskOFDM_D;
			for(i=0; i<OFDM_TABLE_SIZE; i++)	{
				if(ele_D == (OFDMSwingTable[i]&bMaskOFDM_D))	{
					priv->pshare->OFDM_index[0] = i;
					priv->pshare->OFDM_index0[0] = i;
					break;
				}
			}

			//Query OFDM path B default setting
			if(is2T)	{
				ele_D = PHY_QueryBBReg(priv, rOFDM0_XBTxIQImbalance, bMaskDWord)&bMaskOFDM_D;
				for(i=0; i<OFDM_TABLE_SIZE; i++)			{
					if(ele_D == (OFDMSwingTable[i]&bMaskOFDM_D))	{
						priv->pshare->OFDM_index[1] = i;
						priv->pshare->OFDM_index0[1] = i;
						break;
					}
				}
			}
			priv->pshare->CCK_index = get_CCK_swing_index(priv);
			priv->pshare->CCK_index0 = priv->pshare->CCK_index;

		}

		delta     = RTL_ABS(ThermalValue, priv->pshare->ThermalValue);
		delta_LCK = RTL_ABS(ThermalValue, priv->pshare->ThermalValue_LCK);
		delta_IQK = RTL_ABS(ThermalValue, priv->pshare->ThermalValue_IQK);

//		printk("Readback Thermal Meter = 0x%lx pre thermal meter 0x%lx EEPROMthermalmeter 0x%lx delta 0x%lx delta_LCK 0x%lx delta_IQK 0x%lx\n",
//			ThermalValue, priv->pshare->ThermalValue, priv->pmib->dot11RFEntry.ther, delta, delta_LCK, delta_IQK);

		if(delta_LCK > 1)	{
			priv->pshare->ThermalValue_LCK = ThermalValue;

#ifdef MP_TEST
			if(priv->pshare->rf_ft_var.mp_specific)
			{
				if((OPMODE & WIFI_MP_CTX_BACKGROUND) && !(OPMODE & WIFI_MP_CTX_PACKET))
					printk("NOT do LCK during ctx !!!! \n"); 
				else
			PHY_LCCalibrate(priv);
		}
			else
#endif
				PHY_LCCalibrate(priv);
			
		}

		if(delta > 0)	{
			if(ThermalValue > priv->pshare->ThermalValue)	{
				for(i = 0; i < rf; i++)
				 	priv->pshare->OFDM_index[i] -= delta;
				priv->pshare->CCK_index -= delta;
			} else {
				for(i = 0; i < rf; i++)
					priv->pshare->OFDM_index[i] += delta;
				priv->pshare->CCK_index += delta;
			}
			if(ThermalValue > priv->pmib->dot11RFEntry.ther)	{
				for(i = 0; i < rf; i++)
					OFDM_index[i] = priv->pshare->OFDM_index[i]+1;
				CCK_index = priv->pshare->CCK_index+1;
			} else {
				for(i = 0; i < rf; i++)
					OFDM_index[i] = priv->pshare->OFDM_index[i];
				CCK_index = priv->pshare->CCK_index;
			}
#ifdef MP_TEST
			if ((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific) {
				TxPwrLevel[0] = priv->pshare->mp_txpwr_patha;
				TxPwrLevel[1] = priv->pshare->mp_txpwr_pathb;
			} else
#endif
			{
				TxPwrLevel[0] = priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[channel-1];
				TxPwrLevel[1] = priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[channel-1];

				if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20) {
					unsigned char offset = (priv->pmib->dot11RFEntry.pwrdiffHT20[channel-1] & 0x0f);
					TxPwrLevel[0] = COUNT_SIGN_OFFSET(TxPwrLevel[0], offset);
					offset = ((priv->pmib->dot11RFEntry.pwrdiffOFDM[channel-1] & 0xf0) >> 4);
					TxPwrLevel[1] = COUNT_SIGN_OFFSET(TxPwrLevel[1], offset);
				}
			}

//			printk("TxPwrLevel[0]=%d, TxPwrLevel[1]=%d\n", TxPwrLevel[0], TxPwrLevel[1]);
			for(i = 0; i < rf; i++)		{
				if(/*TxPwrLevel[i] >=0 &&*/ TxPwrLevel[i] <=26)	{
					if(ThermalValue > priv->pmib->dot11RFEntry.ther) {
						if (delta < 5)
							OFDM_index[i] -= 1;
						else
							OFDM_index[i] -= 2;
					} else if(delta > 5 && ThermalValue < priv->pmib->dot11RFEntry.ther) {
						OFDM_index[i] += 1;
					}
				} else if (TxPwrLevel[i] >= 27 && TxPwrLevel[i] <= 32 && ThermalValue > priv->pmib->dot11RFEntry.ther) {
					if (delta < 5)
						OFDM_index[i] -= 1;
					else
						OFDM_index[i] -= 2;
				} else if (TxPwrLevel[i] >= 32 && TxPwrLevel[i] <= 38 && ThermalValue > priv->pmib->dot11RFEntry.ther && delta > 5) {
					OFDM_index[i] -= 1;
				}

#ifdef _TRACKING_TABLE_FILE

				{
					int d = 0; 
					
					OFDM_index[i] = priv->pshare->OFDM_index[i];
					d = RTL_ABS(OFDM_index[i], priv->pshare->OFDM_index0[i]);

					if(OFDM_index[i] < priv->pshare->OFDM_index0[i])
					{
						OFDM_index[i] = priv->pshare->OFDM_index0[i] - get_tx_tracking_index(priv, channel, i, d, 0, 0);
					}
					else
					{
						OFDM_index[i] = priv->pshare->OFDM_index0[i] + get_tx_tracking_index(priv, channel, i, d, 1, 0);
					}

				}


#else
#ifdef HIGH_POWER_EXT_PA
				if (priv->pshare->rf_ft_var.use_ext_pa) {
					OFDM_index[i] = priv->pshare->OFDM_index[i];
					swingIndexRemap2(&OFDM_index[i], priv->pshare->OFDM_index0[i], i); //Modify HP tracking table, from Arthur 2012.02.13
					//swingIndexRemap(&OFDM_index[i], priv->pshare->OFDM_index0[i]);
				}
#endif
#endif
				if(OFDM_index[i] > OFDM_TABLE_SIZE-1)
					OFDM_index[i] = OFDM_TABLE_SIZE-1;
				else if (OFDM_index[i] < OFDM_min_index)
					OFDM_index[i] = OFDM_min_index;
			}
			i=0;
			{
				if(/*TxPwrLevel[i] >=0 &&*/ TxPwrLevel[i] <=26)		{
					if(ThermalValue > priv->pmib->dot11RFEntry.ther)	{
						if (delta < 5)
							CCK_index -= 1;
						else
							CCK_index -= 2;
					} else if(delta > 5 && ThermalValue < priv->pmib->dot11RFEntry.ther) {
						CCK_index += 1;
					}
				} else if (TxPwrLevel[i] >= 27 && TxPwrLevel[i] <= 32 && ThermalValue > priv->pmib->dot11RFEntry.ther) {
					if (delta < 5)
						CCK_index -= 1;
					else
						CCK_index -= 2;
				} else if (TxPwrLevel[i] >= 32 && TxPwrLevel[i] <= 38 && ThermalValue > priv->pmib->dot11RFEntry.ther && delta > 5) {
					CCK_index -= 1;
				}

#ifdef _TRACKING_TABLE_FILE
				 {
					int d = 0; 
					
					CCK_index = priv->pshare->CCK_index;
					d = RTL_ABS(CCK_index, priv->pshare->CCK_index0);

					if(CCK_index < priv->pshare->CCK_index0)
					{
						CCK_index = priv->pshare->CCK_index0 - get_tx_tracking_index(priv, channel, i, d, 0, 1);
					}
					else
					{
						CCK_index = priv->pshare->CCK_index0 + get_tx_tracking_index(priv, channel, i, d, 1, 1);
					}

				}
#else
#ifdef HIGH_POWER_EXT_PA
				if (priv->pshare->rf_ft_var.use_ext_pa) {
					CCK_index = priv->pshare->CCK_index;
					swingIndexRemap2( &CCK_index, priv->pshare->CCK_index0, i); //Modify HP tracking table, from Arthur 2012.02.13
					//swingIndexRemap( &CCK_index, priv->pshare->CCK_index0);
				}
#endif
#endif
				if(CCK_index > CCK_TABLE_SIZE-1)
					CCK_index = CCK_TABLE_SIZE-1;
				else if (CCK_index < 0)
					CCK_index = 0;
			}

			//Adujst OFDM Ant_A according to IQK result
			ele_D = (OFDMSwingTable[(unsigned int)OFDM_index[0]] & 0xFFC00000)>>22;
			X = priv->pshare->RegE94;
			Y = priv->pshare->RegE9C;

			if(X != 0) {
				if ((X & 0x00000200) != 0)
					X = X | 0xFFFFFC00;
				ele_A = ((X * ele_D)>>8)&0x000003FF;

				//new element C = element D x Y
				if ((Y & 0x00000200) != 0)
					Y = Y | 0xFFFFFC00;
				ele_C = ((Y * ele_D)>>8)&0x000003FF;

				//wirte new elements A, C, D to regC80 and regC94, element B is always 0
				value32 = (ele_D<<22)|((ele_C&0x3F)<<16)|ele_A;
				PHY_SetBBReg(priv, rOFDM0_XATxIQImbalance, bMaskDWord, value32);

				value32 = (ele_C&0x000003C0)>>6;
				PHY_SetBBReg(priv, rOFDM0_XCTxAFE, bMaskH4Bits, value32);

				value32 = ((X * ele_D)>>7)&0x01;
				PHY_SetBBReg(priv, rOFDM0_ECCAThreshold, BIT(24), value32);

			} else {
				PHY_SetBBReg(priv, rOFDM0_XATxIQImbalance, bMaskDWord, OFDMSwingTable[(unsigned int)OFDM_index[0]]);
				PHY_SetBBReg(priv, rOFDM0_XCTxAFE, bMaskH4Bits, 0x00);
				PHY_SetBBReg(priv, rOFDM0_ECCAThreshold, BIT(24), 0x00);
			}


			set_CCK_swing_index(priv, CCK_index);


			if(is2T) {
				ele_D = (OFDMSwingTable[(unsigned int)OFDM_index[1]] & 0xFFC00000)>>22;
				X = priv->pshare->RegEB4;
				Y = priv->pshare->RegEBC;

				if(X != 0) {
					if ((X & 0x00000200) != 0)	//consider minus
						X = X | 0xFFFFFC00;
					ele_A = ((X * ele_D)>>8)&0x000003FF;

					//new element C = element D x Y
					if ((Y & 0x00000200) != 0)
						Y = Y | 0xFFFFFC00;
					ele_C = ((Y * ele_D)>>8)&0x00003FF;

					//wirte new elements A, C, D to regC88 and regC9C, element B is always 0
					value32=(ele_D<<22)|((ele_C&0x3F)<<16) |ele_A;
					PHY_SetBBReg(priv, rOFDM0_XBTxIQImbalance, bMaskDWord, value32);

					value32 = (ele_C&0x000003C0)>>6;
					PHY_SetBBReg(priv, rOFDM0_XDTxAFE, bMaskH4Bits, value32);

					value32 = ((X * ele_D)>>7)&0x01;
					PHY_SetBBReg(priv, rOFDM0_ECCAThreshold, BIT(28), value32);

				} else {
					PHY_SetBBReg(priv, rOFDM0_XBTxIQImbalance, bMaskDWord, OFDMSwingTable[(unsigned int)OFDM_index[1]]);
					PHY_SetBBReg(priv, rOFDM0_XDTxAFE, bMaskH4Bits, 0x00);
					PHY_SetBBReg(priv, rOFDM0_ECCAThreshold, BIT(28), 0x00);
				}
			}
		}

		if(delta_IQK > 3) {
			priv->pshare->ThermalValue_IQK = ThermalValue;

#ifdef MP_TEST
			if(priv->pshare->rf_ft_var.mp_specific)
			{
				if((OPMODE & WIFI_MP_CTX_BACKGROUND) && !(OPMODE & WIFI_MP_CTX_PACKET))
					printk("NOT do IQK during ctx !!!! \n"); 
				else
					PHY_IQCalibrate(priv);
			}
			else
#endif
			PHY_IQCalibrate(priv);
		}

		//update thermal meter value
		priv->pshare->ThermalValue = ThermalValue;

	}
}
#endif


#ifdef RX_GAIN_TRACK_92D
static void rx_gain_tracking_92D(struct rtl8192cd_priv *priv)
{
	u8	index_mapping[Rx_index_mapping_NUM] = {
						0x0f,	0x0f,	0x0f,	0x0f,	0x0b,
						0x0a,	0x09,	0x08,	0x07,	0x06,
						0x05,	0x04,	0x04,	0x03,	0x02
					};

	u8	eRFPath, curMaxRFPath;
	u32	u4tmp;

	u4tmp = (index_mapping[(priv->pmib->dot11RFEntry.ther - priv->pshare->ThermalValue_RxGain)]) << 12;

	DEBUG_INFO("===>%s interface %d  Rx Gain %x\n", __FUNCTION__, priv->pshare->wlandev_idx, u4tmp);

	if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY)
		curMaxRFPath = RF92CD_PATH_B;
	else
		curMaxRFPath = RF92CD_PATH_MAX;

	for(eRFPath = RF92CD_PATH_A; eRFPath < curMaxRFPath; eRFPath++)
		PHY_SetRFReg(priv, eRFPath, 0x3C, bMask20Bits, (priv->pshare->RegRF3C[eRFPath]&(~(0xF000)))|u4tmp);

};

#endif

#ifdef CONFIG_RTL_92D_SUPPORT

void getDeltaValue(struct rtl8192cd_priv *priv)
{
	unsigned int tempval[2];

	tempval[0] = priv->pmib->dot11RFEntry.deltaIQK;
	tempval[1] = priv->pmib->dot11RFEntry.deltaLCK;

	switch(tempval[0])
	{
		case 0:
			tempval[0] = 5;
			break;

		case 1:
			tempval[0] = 4;
			break;

		case 2:
			tempval[0] = 3;
			break;

		case 3:
		default:
			tempval[0] = 0;
			break;
	}

	switch(tempval[1])
	{
		case 0:
			tempval[1] = 4;
			break;

		case 1:
			tempval[1] = 3;
			break;

		case 2:
			tempval[1] = 2;
			break;

		case 3:
		default:
			tempval[1] = 0;
			break;
	}

	priv->pshare->Delta_IQK = tempval[0];
	priv->pshare->Delta_LCK = tempval[1];
}

void tx_power_tracking_92D(struct rtl8192cd_priv *priv)
{
	u8		ThermalValue = 0, delta, delta_LCK, delta_IQK, index[2], offset, ThermalValue_AVG_count = 0;
	u32		ThermalValue_AVG = 0;
	int 	ele_A, ele_D, X, value32, Y, ele_C;
	int		OFDM_index[2], CCK_index=0;
	int	   	i = 0;
	char	is2T = ((priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY) ?1 :0);
	u8 		OFDM_min_index = 6, OFDM_min_index_internalPA = 5, rf=1, channel; //OFDM BB Swing should be less than +3.0dB, which is required by Arthur	u1Byte			OFDM_min_index = 6, rf; //OFDM BB Swing should be less than +3.0dB, which is required by Arthur

	u8		index_mapping[5][index_mapping_NUM] = {
					{0,	1,	3,	6,	8,	9,				//5G, path A/MAC 0, decrease power
					11,	13,	14,	16,	17,	18, 18},
					{0,	2,	4,	5,	7,	10,				//5G, path A/MAC 0, increase power
					12,	14,	16,	18,	18,	18,	18},
					{0,	2,	3,	6,	8,	9,				//5G, path B/MAC 1, decrease power
					11,	13,	14,	16,	17,	18,	18},
					{0,	2,	4,	5,	7,	10,				//5G, path B/MAC 1, increase power
					13,	16,	16,	18,	18,	18,	18},
					{0,	1,	2,	3,	4,	5,				//2.4G, for decreas power
					6,	7,	7,	8,	9,	10,	10},
					};

#if defined(RTL8192D_INT_PA)

	u8		index_mapping_internalPA[8][index_mapping_NUM] = {
					 {0,  1, 3, 4, 6, 7,	//5G, path A/MAC 0, ch36-64, decrease power
					 9,  11,  13,  15,	16,  16,  16},
					 {0,  1, 3, 4, 6, 7,	 //5G, path A/MAC 0, ch36-64, increase power
					 9,  11,  13,  15,	16,  18,  20},
					 {0,  1, 3, 4, 6, 7,	//5G, path A/MAC 0, ch100-165, decrease power
					 9, 11,  13,  15,  16,	16,  16},
					 {0,  1, 3, 4, 6, 7,	 //5G, path A/MAC 0, ch100-165, increase power
					 9,  11,  13,  15,	16,  18,  20},
					 {0,  1, 3, 4, 6, 7,	//5G, path B/MAC 1, ch36-64, decrease power
					 9, 11,  13,  15,  16,	16,  16},
					 {0,  1, 3, 4, 6, 7,	 //5G, path B/MAC 1, ch36-64, increase power
					 9,  11,  13,  15,	16,  18,  20},
					 {0,  1, 3, 4, 6, 7,	//5G, path B/MAC 1, ch100-165, decrease power
					 9, 11, 13,  15,  16,  16,	16},
					 {0,  1, 3, 4, 6, 7,	 //5G, path B/MAC 1, ch100-165, increase power
					 9,  11,  13,  15,	16,  18,  20},
					};	

	u8			bInteralPA[2];	
				
#endif

#ifdef DPK_92D
	short	index_mapping_DPK[4][index_mapping_DPK_NUM]={
				{0, 0,	1,	2,	2,				//path A current thermal > PG thermal
				3,	4,	5,	5,	6,		
				7,	7,	8,	9,	9},
				{0, 0,	-1, -2, -3, 			//path A current thermal < PG thermal
				-3, -4, -5, -6, -6, 	
				-7, -8, -9, -9, -10},
				{0, 0,	1,	2,	2,				//path B current thermal > PG thermal
				3,	4,	5,	5,	6,		
				7,	7,	8,	9,	9},
				{0, 0,	-1, -2, -3, 			//path B current thermal < PG thermal
				-3, -4, -5, -6, -6, 	
				-7, -8, -9, -9, -10}					
				};

	u8		delta_DPK;
	short	index_DPK[2] = { 0xb68,	0xb6c }, value_DPK, value_DPK_shift;
	int j;

	if(priv->pshare->bDPKworking) {
		DEBUG_INFO("DPK in progress abort tx power tracking \n");
		return; 
	}

#endif


#ifdef HIGH_POWER_EXT_PA //Modify HP tracking table, from Arthur 2012.02.13

u8		index_mapping_HighPower_PA[12][index_mapping_NUM] = {
		{0,  2,  3,  4,  7,  8, 10, 12, 13, 15,	16, 17, 18}, //5G, path A/MAC 0, ch36-64, decrease power
		{0,  2,  4,  7,  8, 10,	11, 15, 17, 19,	21, 23, 23}, //5G, path A/MAC 0, ch36-64, increase power
		{0,  4,  5,  8,  9, 11, 14, 15, 16, 17,	18, 19, 20}, //5G, path A/MAC 0, ch100-140, decrease power
		{0,  2,  4,  5,  7,  9,	13, 15, 19, 21,	22, 23, 23}, //5G, path A/MAC 0, ch100-140, increase power
		{0,  4,  5,  8,  9, 11, 14, 15, 17, 18, 19, 20, 21}, //5G, path A/MAC 0, ch149-165, decrease power
		{0,  2,  4,  6,  8, 10, 14, 16, 19, 21, 22, 24, 24}, //5G, path A/MAC 0, ch149-165, increase power
		{0,  4,  5,  6,  8,  9, 11, 12, 13, 14,	15, 16, 17}, //5G, path B/MAC 1, ch36-64, decrease power
		{0,  2,  4,  7,  8, 10,	11, 15, 17, 19,	21, 23, 23}, //5G, path B/MAC 1, ch36-64, increase power
		{0,  3,  4,  6,  7,  9, 12, 13, 14, 15,	17, 18, 19}, //5G, path B/MAC 1, ch100-140, decrease power
		{0,  2,  4,  5,  7,  9,	13, 15, 19, 21,	22, 23, 23}, //5G, path B/MAC 1, ch100-140, increase power
		{0,  3,  4,  6,  7,  9, 12, 13, 15, 16, 17, 18, 19}, //5G, path B/MAC 1, ch149-165, decrease power
		{0,  3,  5,  7,  9, 11, 13, 17, 19, 21, 22, 23, 23}, //5G, path B/MAC 1, ch149-165, increase power
		};

#endif

#ifdef MP_TEST
	if ((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific) {
		channel=priv->pshare->working_channel;
		if(priv->pshare->mp_txpwr_tracking == FALSE)
			return;
	} else
#endif
	{
		channel = (priv->pmib->dot11RFEntry.dot11channel);
	}

	if (priv->pshare->pwr_trk_ongoing==0) {
		PHY_SetRFReg(priv, RF92CD_PATH_A, RF_T_METER_92D, bMask20Bits, 0x30000);
		priv->pshare->pwr_trk_ongoing = 1;
		return;
	}else{
		ThermalValue =(unsigned char)PHY_QueryRFReg(priv, RF92CD_PATH_A, RF_T_METER_92D, 0xf800, 1);
		priv->pshare->pwr_trk_ongoing = 0;
#ifdef DPK_92D
		priv->pshare->ThermalValue_DPKtrack = ThermalValue;
#endif
	}
	DEBUG_INFO("Readback Thermal Meter = 0x%lx pre thermal meter 0x%lx EEPROMthermalmeter 0x%lx\n", ThermalValue,
				priv->pshare->ThermalValue, priv->pmib->dot11RFEntry.ther);

	if(is2T)
		rf = 2;
	else
		rf = 1;

	if (ThermalValue) {

		//Query OFDM path A default setting
		ele_D = PHY_QueryBBReg(priv, rOFDM0_XATxIQImbalance, bMaskDWord)&bMaskOFDM_D;
		for(i=0; i<OFDM_TABLE_SIZE_92D; i++) {	//find the index
			if(ele_D == (OFDMSwingTable_92D[i]&bMaskOFDM_D))	{
				priv->pshare->OFDM_index0[0] = i;
				DEBUG_INFO("Initial pathA ele_D reg0x%x = 0x%lx, OFDM_index=0x%x\n",
						rOFDM0_XATxIQImbalance, ele_D, priv->pshare->OFDM_index0[0]);
				break;
			}
		}

		//Query OFDM path B default setting
		if(is2T)	{
			ele_D = PHY_QueryBBReg(priv, rOFDM0_XBTxIQImbalance, bMaskDWord)&bMaskOFDM_D;
			for(i=0; i<OFDM_TABLE_SIZE_92D; i++)			{
				if(ele_D == (OFDMSwingTable_92D[i]&bMaskOFDM_D))	{
					priv->pshare->OFDM_index0[1] = i;
					DEBUG_INFO("Initial pathB ele_D reg0x%x = 0x%lx, OFDM_index=0x%x\n",
							rOFDM0_XBTxIQImbalance, ele_D, priv->pshare->OFDM_index0[1]);
					break;
				}
			}
		}

		if(priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G) {
			priv->pshare->CCK_index0 = get_CCK_swing_index(priv);
		} else {
			priv->pshare->CCK_index0 = 12;
		}

		if(!priv->pshare->ThermalValue)	{
			priv->pshare->ThermalValue = priv->pmib->dot11RFEntry.ther;
			priv->pshare->ThermalValue_LCK = ThermalValue;
			priv->pshare->ThermalValue_IQK = ThermalValue;
#ifdef RX_GAIN_TRACK_92D
			priv->pshare->ThermalValue_RxGain = priv->pmib->dot11RFEntry.ther;
#endif
#ifdef DPK_92D
			priv->pshare->ThermalValue_DPK = ThermalValue;
#endif
			for(i = 0; i < rf; i++)
				priv->pshare->OFDM_index[i] = priv->pshare->OFDM_index0[i];
			priv->pshare->CCK_index = priv->pshare->CCK_index0;
		}


		//calculate average thermal meter
		{
			priv->pshare->Thermal_log[priv->pshare->Thermal_idx] = ThermalValue;
			priv->pshare->Thermal_idx = (priv->pshare->Thermal_idx+1)%8;

			for(i=0; i<8; i++) {
				if(priv->pshare->Thermal_log[i]) {
					ThermalValue_AVG += priv->pshare->Thermal_log[i];
					ThermalValue_AVG_count++;
				}
			}

			if(ThermalValue_AVG_count)
				ThermalValue = (u8)(ThermalValue_AVG / ThermalValue_AVG_count);
		}


		delta     = RTL_ABS(ThermalValue, priv->pshare->ThermalValue);
		delta_LCK = RTL_ABS(ThermalValue, priv->pshare->ThermalValue_LCK);
		delta_IQK = RTL_ABS(ThermalValue, priv->pshare->ThermalValue_IQK);

//		printk("Readback Thermal Meter = 0x%lx pre thermal meter 0x%lx EEPROMthermalmeter 0x%lx delta 0x%lx delta_LCK 0x%lx delta_IQK 0x%lx\n",
//			ThermalValue, priv->pshare->ThermalValue, priv->pmib->dot11RFEntry.ther, delta, delta_LCK, delta_IQK);

		getDeltaValue(priv);

#ifdef DPK_92D

		if(priv->pshare->bDPKstore)	{

			priv->pshare->ThermalValue_DPK = ThermalValue;
			delta_DPK = 0;

			for(j = 0; j < rf; j++)	{

				if(priv->pshare->ThermalValue_DPKstore > priv->pmib->dot11RFEntry.ther)
					value_DPK_shift = index_mapping_DPK[j*2][priv->pshare->ThermalValue_DPKstore- priv->pmib->dot11RFEntry.ther];
				else
					value_DPK_shift = index_mapping_DPK[j*2+1][priv->pmib->dot11RFEntry.ther- priv->pshare->ThermalValue_DPKstore];

				for(i = 0; i < index_mapping_DPK_NUM; i++) 	{
					priv->pshare->index_mapping_DPK_current[j*2][i] = 
						index_mapping_DPK[j*2][i]-value_DPK_shift;
					priv->pshare->index_mapping_DPK_current[j*2+1][i] = 
						index_mapping_DPK[j*2+1][i]-value_DPK_shift;										
				}				
			}		
		}
		else
		{
			delta_DPK = RTL_ABS(ThermalValue, priv->pshare->ThermalValue_DPK);
		}

		for(j = 0; j < rf; j++)			{
			if(!priv->pshare->bDPKdone[j])
				priv->pshare->OFDM_min_index_internalPA_DPK[j] = 0;
		}

#endif

#if 1
		if ((delta_LCK > priv->pshare->Delta_LCK) && (priv->pshare->Delta_LCK != 0)) {
			priv->pshare->ThermalValue_LCK = ThermalValue;
			PHY_LCCalibrate_92D(priv);
		}
#endif
		if(delta > 0
#ifdef DPK_92D
			||(priv->pshare->bDPKstore)			
#endif
		){
			if(delta == 0 && priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G)
				goto TxPowerDPK;
#ifdef DPK_92D
			if(priv->pshare->bDPKstore)
				priv->pshare->bDPKstore = FALSE;
#endif
			delta	= RTL_ABS(ThermalValue, priv->pmib->dot11RFEntry.ther);

			//calculate new OFDM / CCK offset
			{
				if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G){

#ifdef _TRACKING_TABLE_FILE
				if(ThermalValue > priv->pmib->dot11RFEntry.ther)
				{
					for(i = 0; i < rf; i++)
						 OFDM_index[i] = priv->pshare->OFDM_index[i] - get_tx_tracking_index(priv, channel, i, delta, 0, 0);
					
					CCK_index = priv->pshare->CCK_index - get_tx_tracking_index(priv, channel, 0, delta, 0, 1);
				}
				else
				{
					for(i = 0; i < rf; i++)
						OFDM_index[i] = priv->pshare->OFDM_index[i] + get_tx_tracking_index(priv, channel, i, delta, 1, 0);
					
					CCK_index = priv->pshare->CCK_index + get_tx_tracking_index(priv, channel, i, delta, 1, 1);
				}

#else
					offset = 4;

					if(delta > index_mapping_NUM-1)
						index[0] = index_mapping[offset][index_mapping_NUM-1];
					else
						index[0] = index_mapping[offset][delta];

					if(ThermalValue > priv->pmib->dot11RFEntry.ther)	{
						for(i = 0; i < rf; i++)
						 	OFDM_index[i] = priv->pshare->OFDM_index[i] - delta;
						CCK_index = priv->pshare->CCK_index - delta;
					}
					else	{
						for(i = 0; i < rf; i++)
							OFDM_index[i] = priv->pshare->OFDM_index[i] + index[0];
						CCK_index = priv->pshare->CCK_index + index[0];
					}
#endif
				} else if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G) {
					for(i = 0; i < rf; i++){

#if defined(RTL8192D_INT_PA)

						if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY && priv->pshare->wlandev_idx==1)		//MAC 1 5G
							bInteralPA[i] = priv->pshare->phw->InternalPA5G[1];
						else
							bInteralPA[i] = priv->pshare->phw->InternalPA5G[i];	

						if(bInteralPA[i]) {
							if(priv->pshare->wlandev_idx == 1 || i == 1/*rf*/)
								offset = 4;
							else
								offset = 0;
							if(channel >= 100 && channel <= 165)
								offset += 2;													
						}
						else
#endif
						{
							if(priv->pshare->wlandev_idx == 1 || i == 1)
								offset = 2;
							else
								offset = 0;
						}


#ifdef HIGH_POWER_EXT_PA //Modify HP tracking table, from Arthur 2012.02.13
						if(i == 0)
						{
							if(channel <= 99)
								offset = 0; 
							else if(channel <= 140)
								offset = 2;
							else
								offset = 4;
						}
						else
						{
							if(channel <= 99)
								offset = 6; 
							else if(channel <= 140)
								offset = 8;
							else
								offset = 10;
						}
#endif


						if(ThermalValue > priv->pmib->dot11RFEntry.ther) //set larger Tx power
							offset++;
#if defined(RTL8192D_INT_PA)
						if(bInteralPA[i]) {
							if(delta > index_mapping_NUM-1)
								index[i] = index_mapping_internalPA[offset][index_mapping_NUM-1];
							else
								index[i] = index_mapping_internalPA[offset][delta];
						} else
#endif
						{
							if(delta > index_mapping_NUM-1)
								index[i] = index_mapping[offset][index_mapping_NUM-1];
							else
								index[i] = index_mapping[offset][delta];
						}



#ifdef _TRACKING_TABLE_FILE
						{
							if(ThermalValue > priv->pmib->dot11RFEntry.ther)
								index[i] = get_tx_tracking_index(priv, channel, i, delta, 0, 0);	
							else
								index[i] = get_tx_tracking_index(priv, channel, i, delta, 1, 0);
						}

#else

#ifdef HIGH_POWER_EXT_PA //Modify HP tracking table, from Arthur 2012.02.13
  						{
							if(delta > index_mapping_NUM-1)
								index[i] = index_mapping_HighPower_PA[offset][index_mapping_NUM-1];
							else
								index[i] = index_mapping_HighPower_PA[offset][delta];

							//printk("\n\n offset = %d delta = %d \n", offset, delta);
							//printk("index[%d]= %d\n\n", i, index[i]);
						}
#endif

#endif


						if(ThermalValue > priv->pmib->dot11RFEntry.ther) //set larger Tx power
						{
#if 0						
							if(bInteralPA[i] && ThermalValue > 0x12)
								index[i] = ((delta/2)*3+(delta%2));	
#endif							
							OFDM_index[i] = priv->pshare->OFDM_index[i] -index[i];
						}
						else
						{
							OFDM_index[i] = priv->pshare->OFDM_index[i] + index[i];
						}
					}
				}

				if(is2T)
				{
					DEBUG_INFO("temp OFDM_A_index=0x%x, OFDM_B_index=0x%x, CCK_index=0x%x\n",
						priv->pshare->OFDM_index[0], priv->pshare->OFDM_index[1], priv->pshare->CCK_index);
				}
				else
				{
					DEBUG_INFO("temp OFDM_A_index=0x%x, CCK_index=0x%x\n",
						priv->pshare->OFDM_index[0], priv->pshare->CCK_index);
				}

				for(i = 0; i < rf; i++)
				{
					if(OFDM_index[i] > OFDM_TABLE_SIZE_92D-1) {
						OFDM_index[i] = OFDM_TABLE_SIZE_92D-1;
					}
					else if(priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G) {
						if (OFDM_index[i] < (OFDM_min_index_internalPA))
							OFDM_index[i] = (OFDM_min_index_internalPA);
					} else if(bInteralPA[i]) {
#ifdef DPK_92D
						if (OFDM_index[i] < (OFDM_min_index_internalPA+ priv->pshare->OFDM_min_index_internalPA_DPK[i]))
						{
							priv->pshare->TxPowerLevelDPK[i] = OFDM_min_index_internalPA+ priv->pshare->OFDM_min_index_internalPA_DPK[i]-OFDM_index[i];
							OFDM_index[i] = (OFDM_min_index_internalPA+ priv->pshare->OFDM_min_index_internalPA_DPK[i]);				
						}
						else
						{
							priv->pshare->TxPowerLevelDPK[i] = 0;
						}
#else
                                                if (OFDM_index[i] < (OFDM_min_index_internalPA))
                                                {
                                                        OFDM_index[i] = (OFDM_min_index_internalPA);                   
                                                }
#endif				
					} else if(OFDM_index[i] < OFDM_min_index) {
#ifdef HIGH_POWER_EXT_PA //Modify HP tracking table, from Arthur 2012.02.13
#else
						OFDM_index[i] = OFDM_min_index;
#endif
					}
				}

				if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G){
					if(CCK_index > CCK_TABLE_SIZE_92D-1)
						CCK_index = CCK_TABLE_SIZE_92D-1;
					else if (CCK_index < 0)
						CCK_index = 0;
				}

				if(is2T) {
					DEBUG_INFO("new OFDM_A_index=0x%x, OFDM_B_index=0x%x, CCK_index=0x%x\n",
							OFDM_index[0], OFDM_index[1], CCK_index);
				}
				else
				{
					DEBUG_INFO("new OFDM_A_index=0x%x, CCK_index=0x%x\n",
							OFDM_index[0], CCK_index);
				}
			}

			//Config by SwingTable
			{
				//Adujst OFDM Ant_A according to IQK result
				ele_D = (OFDMSwingTable_92D[OFDM_index[0]] & 0xFFC00000)>>22;
				X = priv->pshare->RegE94;
				Y = priv->pshare->RegE9C;

				if(X != 0 && (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G)){
					if ((X & 0x00000200) != 0)
						X = X | 0xFFFFFC00;
					ele_A = ((X * ele_D)>>8)&0x000003FF;

					//new element C = element D x Y
					if ((Y & 0x00000200) != 0)
						Y = Y | 0xFFFFFC00;
					ele_C = ((Y * ele_D)>>8)&0x000003FF;

					//wirte new elements A, C, D to regC80 and regC94, element B is always 0
					value32 = (ele_D<<22)|((ele_C&0x3F)<<16)|ele_A;
					PHY_SetBBReg(priv, rOFDM0_XATxIQImbalance, bMaskDWord, value32);

					value32 = (ele_C&0x000003C0)>>6;
					PHY_SetBBReg(priv, rOFDM0_XCTxAFE, bMaskH4Bits, value32);

					value32 = ((X * ele_D)>>7)&0x01;
					PHY_SetBBReg(priv, rOFDM0_ECCAThreshold, BIT(24), value32);

				}
				else
				{
					PHY_SetBBReg(priv, rOFDM0_XATxIQImbalance, bMaskDWord, OFDMSwingTable_92D[OFDM_index[0]]);
					PHY_SetBBReg(priv, rOFDM0_XCTxAFE, bMaskH4Bits, 0x00);
					PHY_SetBBReg(priv, rOFDM0_ECCAThreshold, BIT(24), 0x00);
#ifdef MP_TEST
					if ((priv->pshare->rf_ft_var.mp_specific) && (!is2T)) {
						unsigned char str[50];
						sprintf(str, "patha=%d,pathb=%d", priv->pshare->mp_txpwr_patha, priv->pshare->mp_txpwr_pathb);
						mp_set_tx_power(priv, str);
					}
#endif
				}
				DEBUG_INFO("TxPwrTracking for interface %d path A: X = 0x%x, Y = 0x%x ele_A = 0x%x ele_C = 0x%x ele_D = 0x%x\n",
							priv->pshare->wlandev_idx, X, Y, ele_A, ele_C, ele_D);


				if(priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G)
				{
					//Adjust CCK according to IQK result
					set_CCK_swing_index(priv, CCK_index);
				}

				if(is2T)
				{
					ele_D = (OFDMSwingTable_92D[OFDM_index[1]] & 0xFFC00000)>>22;

					//new element A = element D x X
					X = priv->pshare->RegEB4;
					Y = priv->pshare->RegEBC;

					if(X != 0 && (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G)){
						if ((X & 0x00000200) != 0)	//consider minus
							X = X | 0xFFFFFC00;
						ele_A = ((X * ele_D)>>8)&0x000003FF;

						//new element C = element D x Y
						if ((Y & 0x00000200) != 0)
							Y = Y | 0xFFFFFC00;
						ele_C = ((Y * ele_D)>>8)&0x00003FF;

						//wirte new elements A, C, D to regC88 and regC9C, element B is always 0
						value32=(ele_D<<22)|((ele_C&0x3F)<<16) |ele_A;
						PHY_SetBBReg(priv, rOFDM0_XBTxIQImbalance, bMaskDWord, value32);

						value32 = (ele_C&0x000003C0)>>6;
						PHY_SetBBReg(priv, rOFDM0_XDTxAFE, bMaskH4Bits, value32);

						value32 = ((X * ele_D)>>7)&0x01;
						PHY_SetBBReg(priv, rOFDM0_ECCAThreshold, BIT(28), value32);

					}
					else{
						PHY_SetBBReg(priv, rOFDM0_XBTxIQImbalance, bMaskDWord, OFDMSwingTable_92D[OFDM_index[1]]);
						PHY_SetBBReg(priv, rOFDM0_XDTxAFE, bMaskH4Bits, 0x00);
						PHY_SetBBReg(priv, rOFDM0_ECCAThreshold, BIT(28), 0x00);
#ifdef MP_TEST
					if ((priv->pshare->rf_ft_var.mp_specific) ) {
						unsigned char str[50];
						sprintf(str, "patha=%d,pathb=%d", priv->pshare->mp_txpwr_patha, priv->pshare->mp_txpwr_pathb);
						mp_set_tx_power(priv, str);

					}
#endif				
					}

					DEBUG_INFO("TxPwrTracking path B: X = 0x%x, Y = 0x%x ele_A = 0x%x ele_C = 0x%x ele_D = 0x%x\n",
									X, Y, ele_A, ele_C, ele_D);
				}

				DEBUG_INFO("TxPwrTracking 0xc80 = 0x%x, 0xc94 = 0x%x RF 0x24 = 0x%x\n", PHY_QueryBBReg(priv, 0xc80, bMaskDWord),
						PHY_QueryBBReg(priv, 0xc94, bMaskDWord), PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x24, bMask20Bits,1));
			}
		}

TxPowerDPK:
#ifdef DPK_92D
		{
			char bNOPG = FALSE;
			unsigned char pwrlevelHT40_1S_A = priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[channel-1];
			if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G)
				pwrlevelHT40_1S_A = priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[channel-1];
#ifdef CONFIG_RTL_92D_DMDP			
			if ((priv->pmib->dot11RFEntry.macPhyMode==DUALMAC_DUALPHY) &&
				(priv->pshare->wlandev_idx == 1) && (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G))
					pwrlevelHT40_1S_A = priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[channel-1];
			if (pwrlevelHT40_1S_A == 0)
				bNOPG = TRUE;
#endif

			//for DPK
			if(delta_DPK > 0 && !bNOPG /*&& pHalData->bDPKdone*/) {
				for(i = 0; i < rf; i++) {
					if(bInteralPA[i] && priv->pshare->bDPKdone[i]) {				
						if(ThermalValue > priv->pmib->dot11RFEntry.ther) 	
							value_DPK = priv->pshare->index_mapping_DPK_current[i*2][ThermalValue-priv->pmib->dot11RFEntry.ther];
						else
							value_DPK = priv->pshare->index_mapping_DPK_current[i*2+1][priv->pmib->dot11RFEntry.ther-ThermalValue];
						
						PHY_SetBBReg(priv, index_DPK[i], 0x7c00, value_DPK);						
					}
				}				
				priv->pshare->ThermalValue_DPK = ThermalValue;
			}
		}
#endif
		priv->pshare->pwr_trk_ongoing = 0;
#if 1
		if ((delta_IQK > priv->pshare->Delta_IQK) && (priv->pshare->Delta_IQK != 0)) {
			priv->pshare->ThermalValue_IQK = ThermalValue;
			PHY_IQCalibrate(priv);
		}
#endif

#ifdef RX_GAIN_TRACK_92D
		if(priv->pmib->dot11RFEntry.ther && (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G) &&
			(ThermalValue < priv->pmib->dot11RFEntry.ther)) { 
			priv->pshare->ThermalValue_RxGain = ThermalValue;
			rx_gain_tracking_92D(priv);
		}
#endif

		//update thermal meter value
		priv->pshare->ThermalValue = ThermalValue;
	}
}
#endif




void check_txrate_by_reg(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	unsigned char initial_rate = 0x7f;
	unsigned char legacyRA =0 ;

	if( should_restrict_Nrate(priv, pstat) && is_fixedMCSTxRate(priv))
		legacyRA = 1;
	
#ifdef STA_EXT
	if (pstat->remapped_aid && (pstat->remapped_aid < FW_NUM_STAT-1))
#else
	if (pstat->aid && (pstat->aid < 32))
#endif
	{
#ifdef WDS
		if (((pstat->state & WIFI_WDS) && (priv->pmib->dot11WdsInfo.entry[pstat->wds_idx].txRate == 0)) ||
			(!(pstat->state & WIFI_WDS) && (priv->pmib->dot11StationConfigEntry.autoRate)) || legacyRA)
#else
		if (priv->pmib->dot11StationConfigEntry.autoRate || legacyRA)
#endif
		{
			initial_rate = RTL_R8(INIDATA_RATE_SEL + REMAP_AID(pstat)) & 0x7f;
			if (initial_rate == 0x7f)
				return;

			if ((initial_rate&0x3f) < 12) {
				pstat->current_tx_rate = dot11_rate_table[initial_rate&0x3f];
				pstat->ht_current_tx_info &= ~TX_USE_SHORT_GI;				
			} else {
				pstat->current_tx_rate = 0x80|((initial_rate&0x3f) -12);
				if (initial_rate & BIT(6))
					pstat->ht_current_tx_info |= TX_USE_SHORT_GI;
				else
					pstat->ht_current_tx_info &= ~TX_USE_SHORT_GI;
			}

			priv->pshare->current_tx_rate    = pstat->current_tx_rate;
			priv->pshare->ht_current_tx_info = pstat->ht_current_tx_info;
		} else if (pstat->ht_cap_len) {
			unsigned int is_sgi = 0;

			if (priv->pshare->is_40m_bw && (pstat->tx_bw == HT_CHANNEL_WIDTH_20_40)
#ifdef WIFI_11N_2040_COEXIST
				&& !((OPMODE & WIFI_AP_STATE) && priv->pmib->dot11nConfigEntry.dot11nCoexist &&
				(priv->bg_ap_timeout || priv->force_20_sta || priv->switch_20_sta
#ifdef CONFIG_RTL_88E_SUPPORT
				|| (GET_CHIP_VER(priv) == VERSION_8188E)?(priv->force_20_sta_88e_hw_ext 
				|| priv->switch_20_sta_88e_hw_ext):(0)
#endif
#ifdef STA_EXT
				|| priv->force_20_sta_ext || priv->switch_20_sta_ext
#endif
				))
#endif
			) {
				if (priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M
					&& (pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_40M_)))
					is_sgi++;
			} else if (priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M
				&& (pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_20M_))) {
				is_sgi++;
			}

			if (is_sgi)
				pstat->ht_current_tx_info |= TX_USE_SHORT_GI;
			else
				pstat->ht_current_tx_info &= ~TX_USE_SHORT_GI;
		}

		if (pstat->ht_cap_len) {
			if (priv->pshare->is_40m_bw && (pstat->tx_bw == HT_CHANNEL_WIDTH_20_40))
				pstat->ht_current_tx_info |= TX_USE_40M_MODE;
			else
				pstat->ht_current_tx_info &= ~TX_USE_40M_MODE;
		}

		priv->pshare->ht_current_tx_info = pstat->ht_current_tx_info;
	} else {
		DEBUG_INFO("sta has no aid found to check current tx rate\n");
	}
}

#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)

void set_rssi_cmd(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	unsigned long flags;
	unsigned int content = 0;

	int rssi = pstat->rssi;

#ifdef HIGH_POWER_EXT_PA
	if( priv->pshare->rf_ft_var.use_ext_pa ) 
		rssi += RSSI_DIFF_PA;
	if( rssi > 100)
		rssi = 100;
#endif

	
	SAVE_INT_AND_CLI(flags);

	/*
	 * set rssi
	 */
	 content = rssi<< 24;

#ifdef CONFIG_RTL_92D_SUPPORT
	/*
	 * set max macid
	 */
	if (GET_CHIP_VER(priv) == VERSION_8192D){
		 content |= priv->pshare->max_fw_macid << 16;
	}
#endif

	/*
	 * set macid
	 */
	 content |= pstat->aid << 8;

	/*
	 * set cmd id
	 */
	 content |= H2C_CMD_RSSI;

	signin_h2c_cmd(priv, content, 0);

	RESTORE_INT(flags);
}



void add_rssi_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	struct stat_info *pstat = NULL;
	unsigned int set_timer = 0;
	unsigned long flags;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (timer_pending(&priv->add_rssi_timer))
		del_timer_sync(&priv->add_rssi_timer);

#ifdef PCIE_POWER_SAVING
	if ((priv->pwr_state == L2) || (priv->pwr_state == L1))
			return;
#endif

	if (!list_empty(&priv->addrssi_list)) {
		pstat = list_entry(priv->addrssi_list.next, struct stat_info, addrssi_list);
		if (!pstat)
			return;

		if (!is_h2c_buf_occupy(priv)) {
			set_rssi_cmd(priv, pstat);
			if (!list_empty(&pstat->addrssi_list)) {
				SAVE_INT_AND_CLI(flags);
				SMP_LOCK(flags);
				list_del_init(&pstat->addrssi_list);
				RESTORE_INT(flags);
				SMP_UNLOCK(flags);
			}

			if (!list_empty(&priv->addrssi_list))
				set_timer++;
		} else {
			set_timer++;
		}
	}

	if (set_timer)
		mod_timer(&priv->add_rssi_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(50));	// 50 ms
}


void add_update_rssi(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	unsigned long flags;

	if (is_h2c_buf_occupy(priv)) {
		if (list_empty(&pstat->addrssi_list)) {
			SAVE_INT_AND_CLI(flags);
			list_add_tail(&(pstat->addrssi_list), &(priv->addrssi_list));
			RESTORE_INT(flags);

			if (!timer_pending(&priv->add_rssi_timer))
				mod_timer(&priv->add_rssi_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(50));	// 50 ms
		}
	} else {
		set_rssi_cmd(priv, pstat);
	}
}
#endif


void set_RATid_cmd(struct rtl8192cd_priv *priv, unsigned int macid, unsigned int rateid, unsigned int ratemask)
{
	unsigned int content = 0;
	unsigned short ext_content = 0;

	/*
	 * set ratemask
	 */
	ext_content = ratemask & 0xffff;
	content = ((ratemask & 0xfff0000) >> 16) << 8;

	/*
	 * set short GI
	 */
	if (ratemask & BIT(28))
		content |= BIT(29);

	/*
	 * set macid (station aid)
	 */
	content |= (macid & 0x1f) << 24;

	/*
	 * set rateid (ARFR table)
	 */
	content |= (rateid & 0xf) << 20;

	/*
	 * set ext_content used
	 */
	content |= BIT(7);

	/*
	 * set cmd id
	 */
	content |= H2C_CMD_MACID;

	signin_h2c_cmd(priv, content, ext_content);
}



//3 ============================================================
//3 Antenna Diversity
//3 ============================================================
#ifdef SW_ANT_SWITCH

//
// 20100514 Luke/Joseph:
// Add new function to reset antenna diversity state after link.
//

void resetSwAntDivVariable(struct rtl8192cd_priv *priv)
{
	priv->pshare->RSSI_sum_R = 0;
	priv->pshare->RSSI_cnt_R = 0;
	priv->pshare->RSSI_sum_L = 0;
	priv->pshare->RSSI_cnt_L = 0;
	priv->pshare->TXByteCnt_R = 0;
	priv->pshare->TXByteCnt_L = 0;
	priv->pshare->RXByteCnt_R = 0;
	priv->pshare->RXByteCnt_L = 0;

}
void SwAntDivRestAfterLink(struct rtl8192cd_priv *priv)
{
	priv->pshare->RSSI_test = FALSE;
	priv->pshare->DM_SWAT_Table.try_flag = SWAW_STEP_RESET;
	memset(priv->pshare->DM_SWAT_Table.SelectAntennaMap, 0, sizeof(priv->pshare->DM_SWAT_Table.SelectAntennaMap));
	priv->pshare->DM_SWAT_Table.mapIndex = 0;
	priv->pshare->lastTxOkCnt = priv->net_stats.tx_bytes;
	priv->pshare->lastRxOkCnt = priv->net_stats.rx_bytes;
	resetSwAntDivVariable(priv);
}


void dm_SW_AntennaSwitchInit(struct rtl8192cd_priv *priv)
{
	if(!priv->pshare->rf_ft_var.antSw_enable)
		return;

//	if (GET_CHIP_VER(priv) == VERSION_8188C)
	if(get_rf_mimo_mode(priv)== MIMO_1T1R)
		priv->pshare->rf_ft_var.antSw_select = 0;

	//ODM_RT_TRACE(pDM_Odm,COMP_SWAS, DBG_LOUD, ("SWAS:Init SW Antenna Switch\n"));
	resetSwAntDivVariable(priv);
	priv->pshare->DM_SWAT_Table.CurAntenna = Antenna_B;
	priv->pshare->DM_SWAT_Table.try_flag = SWAW_STEP_RESET;
	memset(priv->pshare->DM_SWAT_Table.SelectAntennaMap, 0, sizeof(priv->pshare->DM_SWAT_Table.SelectAntennaMap));
	priv->pshare->DM_SWAT_Table.mapIndex = 0;

#ifdef GPIO_ANT_SWITCH
#ifdef CONFIG_RTL_92D_DMDP
		 if(priv->pshare->wlandev_idx==0)
			priv->pshare->rf_ft_var.antHw_enable=0;
		 else 
		 	return;
#endif
// GPIO 45 : 
// GPIO_MOD     => data port
// GPIO_IO_SEL  => output 
	RTL_W32(GPIO_PIN_CTRL, 0x00300000| RTL_R32(GPIO_PIN_CTRL));
	PHY_SetBBReg(priv, GPIO_PIN_CTRL, 0x3000, priv->pshare->DM_SWAT_Table.CurAntenna);
	RTL_W32(rFPGA0_XCD_RFParameter, RTL_R32(rFPGA0_XCD_RFParameter)| BIT(15)|BIT(16));		// enable ANTSEL

#else
	RTL_W32(LEDCFG, RTL_R32(LEDCFG) | BIT(23) );	//enable LED[1:0] pin as ANTSEL

	if ( !priv->pshare->rf_ft_var.antSw_select)	{
		RTL_W32(rFPGA0_XAB_RFParameter, RTL_R32(rFPGA0_XAB_RFParameter) | BIT(13) );	 //select ANTESEL from path A
		RTL_W32(rFPGA0_XAB_RFInterfaceSW, RTL_R32(rFPGA0_XAB_RFInterfaceSW) | BIT(8)| BIT(9) );  // enable ANTSEL  A as SW control
		RTL_W32(rFPGA0_XA_RFInterfaceOE, (RTL_R32(rFPGA0_XA_RFInterfaceOE) & ~(BIT(8)|BIT(9)))| 0x01<<8 );	// 0x01: left antenna, 0x02: right antenna
	} else 	{
		RTL_W32(rFPGA0_XAB_RFParameter, RTL_R32(rFPGA0_XAB_RFParameter) & ~ BIT(13) );	 //select ANTESEL from path B
		RTL_W32(rFPGA0_XAB_RFInterfaceSW, RTL_R32(rFPGA0_XAB_RFInterfaceSW) | BIT(24)| BIT(25) );  // enable ANTSEL B as SW control
		RTL_W32(rFPGA0_XB_RFInterfaceOE, (RTL_R32(rFPGA0_XB_RFInterfaceOE) & ~(BIT(8)|BIT(9)))| 0x01<<8 );	// 0x01: left antenna, 0x02: right antenna
	}
	RTL_W16(rFPGA0_TxInfo, (RTL_R16(rFPGA0_TxInfo)&0xf0ff) | BIT(8) );	// b11-b8=0001
#endif

	// Move the timer initialization to InitializeVariables function.
	//PlatformInitializeTimer(Adapter, &pMgntInfo->SwAntennaSwitchTimer, (RT_TIMER_CALL_BACK)dm_SW_AntennaSwitchCallback, NULL, "SwAntennaSwitchTimer");
}
//
// 20100514 Luke/Joseph:
// Add new function for antenna diversity after link.
// This is the main function of antenna diversity after link.
// This function is called in HalDmWatchDog() and dm_SW_AntennaSwitchCallback().
// HalDmWatchDog() calls this function with SWAW_STEP_PEAK to initialize the antenna test.
// In SWAW_STEP_PEAK, another antenna and a 500ms timer will be set for testing.
// After 500ms, dm_SW_AntennaSwitchCallback() calls this function to compare the signal just
// listened on the air with the RSSI of original antenna.
// It chooses the antenna with better RSSI.
// There is also a aged policy for error trying. Each error trying will cost more 5 seconds waiting
// penalty to get next try.
//
void dm_SW_AntennaSwitch(struct rtl8192cd_priv *priv, char Step)
{
	unsigned int	curTxOkCnt, curRxOkCnt;
	unsigned int	CurByteCnt, PreByteCnt;
	int 			Score_R=0, Score_L=0;
	int				RSSI_R, RSSI_L;
	char 			nextAntenna=priv->pshare->DM_SWAT_Table.CurAntenna;
	int				i;

//1 1. Determine which condition should turn off Antenna Diversity

#ifdef MP_TEST
	if ((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific)
		return;
#endif

//	if(!(GET_CHIP_VER(priv) == VERSION_8188C) || !priv->pshare->rf_ft_var.antSw_enable)
//		return;

	if((!priv->assoc_num)
#ifdef PCIE_POWER_SAVING
	|| (priv->pwr_state == L2) || (priv->pwr_state == L1)
#endif
	){
		SwAntDivRestAfterLink(priv);
		return;
	}

	// Handling step mismatch condition.
	// Peak step is not finished at last time. Recover the variable and check again.
	if( Step != priv->pshare->DM_SWAT_Table.try_flag)
	{
		SwAntDivRestAfterLink(priv);
	}

//1 2. Initialization: Select a assocaiated AP or STA as RSSI target
	if(priv->pshare->DM_SWAT_Table.try_flag == SWAW_STEP_RESET) {
#ifdef CLIENT_MODE
		if((OPMODE & (WIFI_STATION_STATE | WIFI_ASOC_STATE)) == (WIFI_STATION_STATE | WIFI_ASOC_STATE)) {
			// Target: Infrastructure mode AP.
			priv->pshare->RSSI_target = NULL;
		}
#endif
		resetSwAntDivVariable(priv);
		priv->pshare->DM_SWAT_Table.try_flag = SWAW_STEP_PEAK;
		return;
	}
	else  {

//1 3. Antenna Diversity

		//2 Calculate TX and RX OK bytes

		curTxOkCnt = priv->net_stats.tx_bytes - priv->pshare->lastTxOkCnt;
		curRxOkCnt = priv->net_stats.rx_bytes - priv->pshare->lastRxOkCnt;
		priv->pshare->lastTxOkCnt = priv->net_stats.tx_bytes;
		priv->pshare->lastRxOkCnt = priv->net_stats.rx_bytes;

		//2 Try State
		if(priv->pshare->DM_SWAT_Table.try_flag == SWAW_STEP_DETERMINE)	{
			//3 1. Seperately caculate TX and RX OK byte counter for ant A and B
			if(priv->pshare->DM_SWAT_Table.CurAntenna == Antenna_A) {
				priv->pshare->TXByteCnt_R += curTxOkCnt;
				priv->pshare->RXByteCnt_R += curRxOkCnt;
			} else 	{
				priv->pshare->TXByteCnt_L += curTxOkCnt;
				priv->pshare->RXByteCnt_L += curRxOkCnt;
			}

			//3 2. Change anntena for testing
			if(priv->pshare->DM_SWAT_Table.RSSI_Trying != 0) {
				nextAntenna = (priv->pshare->DM_SWAT_Table.CurAntenna ) ^ Antenna_MAX;
				priv->pshare->DM_SWAT_Table.RSSI_Trying--;
			}

			//2 Try State End: Determine the best antenna

			if(priv->pshare->DM_SWAT_Table.RSSI_Trying==0) {
				nextAntenna = priv->pshare->DM_SWAT_Table.CurAntenna;
				priv->pshare->DM_SWAT_Table.mapIndex = (priv->pshare->DM_SWAT_Table.mapIndex+1)%SELANT_MAP_SIZE;

				//3 TP Mode: Determine the best antenna by throuhgput
				if(priv->pshare->DM_SWAT_Table.TestMode == TP_MODE)  {



					//3 (1) Saperately caculate total byte count for two antennas
					if(priv->pshare->DM_SWAT_Table.CurAntenna == Antenna_A) {
						CurByteCnt = (priv->pshare->TXByteCnt_R + (priv->pshare->RXByteCnt_R<<1));
						PreByteCnt = (priv->pshare->TXByteCnt_L + (priv->pshare->RXByteCnt_L<<1));
					} else {
						CurByteCnt = (priv->pshare->TXByteCnt_L + (priv->pshare->RXByteCnt_L<<1));
						PreByteCnt = (priv->pshare->TXByteCnt_R + (priv->pshare->RXByteCnt_R<<1));
					}

					//3 (2) Throughput Normalization
					if(priv->pshare->TrafficLoad == TRAFFIC_HIGH)
						CurByteCnt >>=3;
					else if(priv->pshare->TrafficLoad == TRAFFIC_LOW)
						CurByteCnt >>=1;

					if(priv->pshare->DM_SWAT_Table.CurAntenna == Antenna_A) {
						priv->pshare->DM_SWAT_Table.SelectAntennaMap[0][priv->pshare->DM_SWAT_Table.mapIndex] = PreByteCnt;
						priv->pshare->DM_SWAT_Table.SelectAntennaMap[1][priv->pshare->DM_SWAT_Table.mapIndex] = CurByteCnt;
					} else {
						priv->pshare->DM_SWAT_Table.SelectAntennaMap[0][priv->pshare->DM_SWAT_Table.mapIndex] = CurByteCnt;
						priv->pshare->DM_SWAT_Table.SelectAntennaMap[1][priv->pshare->DM_SWAT_Table.mapIndex] = PreByteCnt;
					}

					Score_R = Score_L=0;
					for (i= 0; i<SELANT_MAP_SIZE; i++) {
						Score_L += priv->pshare->DM_SWAT_Table.SelectAntennaMap[0][i];
						Score_R += priv->pshare->DM_SWAT_Table.SelectAntennaMap[1][i];
					}

					nextAntenna = (Score_L > Score_R) ? Antenna_B : Antenna_A;

					if(priv->pshare->rf_ft_var.ant_dump&8)
						panic_printk("Mode TP, select Ant%d, [Score1=%d,Score2=%d]\n", nextAntenna, Score_L, Score_R);

				}

				//3 RSSI Mode: Determine the best anntena by RSSI
				else if(priv->pshare->DM_SWAT_Table.TestMode == RSSI_MODE) {

					//2 Saperately caculate average RSSI for two antennas
					RSSI_L = RSSI_R = 0;

					if(priv->pshare->RSSI_cnt_R > 0)
						RSSI_R = priv->pshare->RSSI_sum_R/priv->pshare->RSSI_cnt_R;
					if(priv->pshare->RSSI_cnt_L > 0)
						RSSI_L = priv->pshare->RSSI_sum_L/priv->pshare->RSSI_cnt_L;

					if(RSSI_L && RSSI_R )
						nextAntenna =  (RSSI_L > RSSI_R) ? Antenna_B : Antenna_A;

					if(priv->pshare->rf_ft_var.ant_dump&8)
						panic_printk("Mode RSSI, RSSI_R=%d(%d), RSSI_L=%d(%d), Ant=%d\n",
						RSSI_R, priv->pshare->RSSI_cnt_R, RSSI_L,  priv->pshare->RSSI_cnt_L, nextAntenna);

				}

				//3 Reset state
				resetSwAntDivVariable(priv);
				priv->pshare->DM_SWAT_Table.try_flag = SWAW_STEP_PEAK;
				priv->pshare->RSSI_test = FALSE;
			}
		}

		//1 Normal State
		else if(priv->pshare->DM_SWAT_Table.try_flag == SWAW_STEP_PEAK)	{

			//3 Determine TP/RSSI mode by TRX OK count
			if((curRxOkCnt+curTxOkCnt) > TP_MODE_THD) {
				//2 Determine current traffic is high or low
				if((curTxOkCnt+curRxOkCnt) > TRAFFIC_THRESHOLD)
					priv->pshare->TrafficLoad = TRAFFIC_HIGH;
				else
					priv->pshare->TrafficLoad = TRAFFIC_LOW;

				priv->pshare->DM_SWAT_Table.RSSI_Trying = 10;
				priv->pshare->DM_SWAT_Table.TestMode = TP_MODE;
			} else	{

				int idx = 0;
				struct stat_info* pEntry = findNextSTA(priv, &idx);
				priv->pshare->RSSI_target = NULL;
				while(pEntry) {
					if(pEntry && pEntry->expire_to) {
						if(!priv->pshare->RSSI_target)
							priv->pshare->RSSI_target = pEntry;
						else if( pEntry->rssi < priv->pshare->RSSI_target->rssi )
							priv->pshare->RSSI_target = pEntry;
					}
					pEntry = findNextSTA(priv, &idx);
				};

				priv->pshare->DM_SWAT_Table.RSSI_Trying = 6;
				priv->pshare->DM_SWAT_Table.TestMode = RSSI_MODE;

				if(priv->pshare->RSSI_target == NULL)	{
					SwAntDivRestAfterLink(priv);
					return;
				}

				//3 reset state
				memset(priv->pshare->DM_SWAT_Table.SelectAntennaMap, 0, sizeof(priv->pshare->DM_SWAT_Table.SelectAntennaMap));
			}

			//3 Begin  to enter Try State
			nextAntenna = (priv->pshare->DM_SWAT_Table.CurAntenna ) ^ Antenna_MAX;
			priv->pshare->DM_SWAT_Table.try_flag = SWAW_STEP_DETERMINE;
			priv->pshare->RSSI_test = TRUE;

			//3 Reset variables
			resetSwAntDivVariable(priv);
		}
	}

//1 4.Change TRX antenna
	if(nextAntenna != priv->pshare->DM_SWAT_Table.CurAntenna) {

#ifdef GPIO_ANT_SWITCH
		PHY_SetBBReg(priv, GPIO_PIN_CTRL, 0x3000, nextAntenna);	
#else		
		if (!priv->pshare->rf_ft_var.antSw_select)
			PHY_SetBBReg(priv, rFPGA0_XA_RFInterfaceOE, 0x300, nextAntenna);
		else
			PHY_SetBBReg(priv, rFPGA0_XB_RFInterfaceOE, 0x300, nextAntenna);
#endif
	}

//1 5.Reset Statistics
	priv->pshare->DM_SWAT_Table.CurAntenna = nextAntenna;

//1 6.Set next timer

	if(priv->pshare->DM_SWAT_Table.RSSI_Trying == 0) {
		return;
	}

	if(priv->pshare->DM_SWAT_Table.TestMode == RSSI_MODE)	{
		mod_timer(&priv->pshare->swAntennaSwitchTimer, jiffies +40);		// 400 ms
	} else if(priv->pshare->DM_SWAT_Table.TestMode == TP_MODE)	{

		if(priv->pshare->TrafficLoad == TRAFFIC_HIGH)	{
			if(priv->pshare->DM_SWAT_Table.RSSI_Trying%2 == 0)
				mod_timer(&priv->pshare->swAntennaSwitchTimer, jiffies + 1);	// 10 ms
			else
				mod_timer(&priv->pshare->swAntennaSwitchTimer, jiffies + 8);	// 80 ms

		} else if(priv->pshare->TrafficLoad == TRAFFIC_LOW) {
			if(priv->pshare->DM_SWAT_Table.RSSI_Trying%2 == 0)
				mod_timer(&priv->pshare->swAntennaSwitchTimer, jiffies + 4);	// 40 ms
			else
				mod_timer(&priv->pshare->swAntennaSwitchTimer, jiffies + 8);	// 80 ms
		}
	}

}


void dm_SW_AntennaSwitchCallback(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv*)task_priv;
	unsigned long flags;
	SAVE_INT_AND_CLI(flags);
	dm_SW_AntennaSwitch(priv, SWAW_STEP_DETERMINE);
	RESTORE_INT(flags);
}

//
// 20100514 Luke/Joseph:
// This function is used to gather the RSSI information for antenna testing.
// It selects the RSSI of the peer STA that we want to know.
//
void dm_SWAW_RSSI_Check(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo)
{
	struct stat_info* pEntry = NULL;
	pEntry = get_stainfo(priv, GetAddr2Ptr(get_pframe(pfrinfo)));

	if((priv->pshare->RSSI_target==NULL)||(priv->pshare->RSSI_target==pEntry)) {
		//1 RSSI for SW Antenna Switch
		if(priv->pshare->DM_SWAT_Table.CurAntenna == Antenna_A)
		{
			priv->pshare->RSSI_sum_R += pfrinfo->rssi;
			priv->pshare->RSSI_cnt_R++;
		} else {
			priv->pshare->RSSI_sum_L += pfrinfo->rssi;
			priv->pshare->RSSI_cnt_L++;
		}
	}
}

#ifndef HW_ANT_SWITCH

int diversity_antenna_select(struct rtl8192cd_priv *priv, unsigned char *data)
{
	int ant = _atoi(data, 16);
//	if(GET_CHIP_VER(priv) != VERSION_8188C)
//		return 0;

#ifdef PCIE_POWER_SAVING
		PCIeWakeUp(priv, POWER_DOWN_T0);
#endif
	if(ant==Antenna_B || ant==Antenna_A) {
#ifdef GPIO_ANT_SWITCH
		PHY_SetBBReg(priv, GPIO_PIN_CTRL, 0x3000, ant); 
#else		
		if (!priv->pshare->rf_ft_var.antSw_select)
			PHY_SetBBReg(priv, rFPGA0_XA_RFInterfaceOE, 0x300, ant);
		else
			PHY_SetBBReg(priv, rFPGA0_XB_RFInterfaceOE, 0x300, ant);
#endif		
		priv->pshare->DM_SWAT_Table.CurAntenna = ant;
		priv->pshare->rf_ft_var.antSw_enable = 0;
		SwAntDivRestAfterLink(priv);
		memset(priv->pshare->DM_SWAT_Table.SelectAntennaMap, 0, sizeof(priv->pshare->DM_SWAT_Table.SelectAntennaMap));
		return 1;
	} else {
		priv->pshare->rf_ft_var.antSw_enable = 1;
		priv->pshare->lastTxOkCnt = priv->net_stats.tx_bytes;
		priv->pshare->lastRxOkCnt = priv->net_stats.rx_bytes;

		return 0;
	}
}
#endif
#endif
#if defined(HW_ANT_SWITCH)

void dm_HW_AntennaSwitchInit(struct rtl8192cd_priv *priv)
{
	if(!priv->pshare->rf_ft_var.antHw_enable)
		return;
#ifdef SW_ANT_SWITCH
	priv->pshare->rf_ft_var.antSw_enable =0;
#endif

//	if (GET_CHIP_VER(priv) == VERSION_8188C)
	if(get_rf_mimo_mode(priv)== MIMO_1T1R)
		priv->pshare->rf_ft_var.antSw_select = 0;
	if ( !priv->pshare->rf_ft_var.antSw_select)	{
		RTL_W32(rFPGA0_XAB_RFParameter, RTL_R32(rFPGA0_XAB_RFParameter) | BIT(13) );	 //select ANTESEL from path A
		RTL_W32(rFPGA0_XAB_RFInterfaceSW, RTL_R32(rFPGA0_XAB_RFInterfaceSW) & ~(BIT(8)| BIT(9)) ); // ANTSEL as HW control
		RTL_W32(rFPGA0_XA_RFInterfaceOE, (RTL_R32(rFPGA0_XA_RFInterfaceOE) & ~(BIT(8)|BIT(9)))| 0x01<<8 );	// 0x01: left antenna, 0x02: right antenna
		RTL_W8(0xc50, RTL_R8(0xc50) | BIT(7));	// Enable Hardware antenna switch
		RTL_W32(0xc54, RTL_R32(0xc54) | BIT(23) );	// Decide final antenna by comparing 2 antennas' pwdb
	} else 	{
		RTL_W32(rFPGA0_XAB_RFParameter, RTL_R32(rFPGA0_XAB_RFParameter) & ~ BIT(13) );	 //select ANTESEL from path B
		RTL_W32(rFPGA0_XAB_RFInterfaceSW, RTL_R32(rFPGA0_XAB_RFInterfaceSW) & ~(BIT(24)| BIT(25)) ); // ANTSEL as HW control
		RTL_W32(rFPGA0_XB_RFInterfaceOE, (RTL_R32(rFPGA0_XB_RFInterfaceOE) & ~(BIT(8)|BIT(9)))| 0x01<<8 );	// 0x01: left antenna, 0x02: right antenna
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
			RTL_W8(0xc58, RTL_R8(0xc58) | BIT(7));	// Enable Hardware antenna switch
#endif
		RTL_W32(0xc5C, RTL_R32(0xc5c) | BIT(23) );	// Decide final antenna by comparing 2 antennas' pwdb
	}

	priv->pshare->rf_ft_var.CurAntenna = 0;

	RTL_W32(LEDCFG, RTL_R32(LEDCFG) | BIT(23) );	//enable LED[1:0] pin as ANTSEL
	RTL_W16(0xca4, (RTL_R16(0xca4) & ~(0xfff))|0x0c0); 	// Pwdb threshold=12dB
	RTL_W32(0x874, RTL_R32(0x874) & ~ BIT(23) );	// No update ANTSEL during GNT_BT=1
	RTL_W16(rFPGA0_TxInfo, (RTL_R16(rFPGA0_TxInfo)&0xf0ff) | BIT(8) );	// b11-b8=0001
	RTL_W32(0x80c, RTL_R32(0x80c) | BIT(21) );		// assign antenna by tx desc

	// CCK setting
	RTL_W8(0xa01, RTL_R8(0xa01) | BIT(7));			// enable hw ant diversity
	RTL_W8(0xa0c, (RTL_R8(0xa0c) & 0xe0) | 0x0f );	// b4=0, b3:0 = 1111	32 sample
	RTL_W8(0xa11, RTL_R8(0xa11) | BIT(5));			// do not change default optional antenna
	RTL_W8(0xa14, (RTL_R8(0xa14) & 0xe0) | 0x08 );	// default : optional = 1:1
#ifdef GPIO_ANT_SWITCH
	PHY_SetBBReg(priv, rFPGA0_XCD_RFParameter, 0x40000000, 0x01);		// enable ANTSEL
#endif	

}

void setRxIdleAnt(struct rtl8192cd_priv *priv, char Ant)
{
	if(priv->pshare->rf_ft_var.CurAntenna != Ant) {
		if(Ant) {
			RTL_W32(0x858, 0x65a965a9);
//			RTL_W8(0x6d8, RTL_R8(0x6d8) | BIT(6) );
		}
		else {
			RTL_W32(0x858, 0x569a569a);
//			RTL_W8(0x6d8, RTL_R8(0x6d8) & (~ BIT(6)));
		}
		priv->pshare->rf_ft_var.CurAntenna = Ant;
	}
}

void dm_STA_Ant_Select(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int ScoreA=0, ScoreB=0, i, nextAnt= pstat->CurAntenna, idleAnt=priv->pshare->rf_ft_var.CurAntenna;

	if((priv->pshare->rf_ft_var.CurAntenna & 0x80)
		|| ((pstat->hwRxAntSel[0] + pstat->hwRxAntSel[1])==0 && (pstat->cckPktCount[0] + pstat->cckPktCount[1])<10)	)
		return;

	for(i=0; i<2; i++) {
		if(pstat->cckPktCount[i]==0 && pstat->hwRxAntSel[i]==0)
			pstat->AntRSSI[i] = 0;
	}

	if(pstat->hwRxAntSel[0] || pstat->hwRxAntSel[1]) {
		ScoreA = pstat->hwRxAntSel[0];
		ScoreB = pstat->hwRxAntSel[1];

		if(ScoreA != ScoreB) {
			if(ScoreA > ScoreB)
				nextAnt = 0;
			else
				nextAnt = 1;
		}
	} else {
		ScoreA = pstat->cckPktCount[0];
		ScoreB = pstat->cckPktCount[1];

		if(ScoreA > 5*ScoreB)
			nextAnt = 0;
		else if(ScoreB > 5*ScoreA)
			nextAnt = 1;
		else if(ScoreA > ScoreB)
			nextAnt = 1;
		else if(ScoreB > ScoreA)
			nextAnt = 0;
	}

	pstat->CurAntenna = nextAnt;

	if(priv->pshare->rf_ft_var.ant_dump&2) {
		panic_printk("id=%d, OFDM/CCK: (%d, %d/%d, %d), RSSI:(%d, %d), ant=%d, RxIdle=%d\n",
			pstat->aid,
			pstat->hwRxAntSel[1],
			pstat->hwRxAntSel[0],
			pstat->cckPktCount[1],
			pstat->cckPktCount[0],
			pstat->AntRSSI[1],
			pstat->AntRSSI[0],
			(pstat->CurAntenna==0 ? 2: 1)
			 ,((priv->pshare->rf_ft_var.CurAntenna&1)==0 ? 2 : 1)
			 );
	}

	if(pstat->AntRSSI[idleAnt]==0)
		pstat->AntRSSI[idleAnt] = pstat->AntRSSI[idleAnt^1];

// reset variables
	pstat->hwRxAntSel[1] = pstat->hwRxAntSel[0] =0;
	pstat->cckPktCount[1]= pstat->cckPktCount[0] =0;

}

void dm_HW_IdleAntennaSelect(struct rtl8192cd_priv *priv)
{
	struct stat_info	*pstat, *pstat_min=NULL;
	struct list_head	*phead, *plist;
	int rssi_min= 0xff, i;

	if(priv->pshare->rf_ft_var.CurAntenna & 0x80)
		return;

	phead = &priv->asoc_list;
	plist = phead->next;

	while(plist != phead)	{
		pstat = list_entry(plist, struct stat_info, asoc_list);
		if((pstat->expire_to) && (pstat->AntRSSI[0] || pstat->AntRSSI[1])) {
			int rssi = (pstat->AntRSSI[0] < pstat->AntRSSI[1]) ? pstat->AntRSSI[0] : pstat->AntRSSI[1];
			if((!pstat_min) || ( rssi < rssi_min) ) {
				pstat_min = pstat;
				rssi_min = rssi;
			}
		}
		if (plist == plist->next)
			break;
		plist = plist->next;
	};

	if(pstat_min)
		setRxIdleAnt(priv, pstat_min->CurAntenna);


#ifdef TX_SHORTCUT
	if (!priv->pmib->dot11OperationEntry.disable_txsc) {
		plist = phead->next;
		while(plist != phead)	{
			pstat = list_entry(plist, struct stat_info, asoc_list);
			if(pstat->expire_to) {
				for (i=0; i<TX_SC_ENTRY_NUM; i++) {
					struct tx_desc *pdesc= &(pstat->tx_sc_ent[i].hwdesc1);	
					pdesc->Dword2 &= set_desc(~ (BIT(24)|BIT(25)));
					if((pstat->CurAntenna^priv->pshare->rf_ft_var.CurAntenna)&1)
						pdesc->Dword2 |= set_desc(BIT(24)|BIT(25));
					pdesc= &(pstat->tx_sc_ent[i].hwdesc2);	
					pdesc->Dword2 &= set_desc(~ (BIT(24)|BIT(25)));
					if((pstat->CurAntenna^priv->pshare->rf_ft_var.CurAntenna)&1)
						pdesc->Dword2 |= set_desc(BIT(24)|BIT(25));					
				}
			}		

			if (plist == plist->next)
				break;
			plist = plist->next;
		};
	}
#endif	

}

int diversity_antenna_select(struct rtl8192cd_priv *priv, unsigned char *data)
{
	int ant = _atoi(data, 16);

#ifdef PCIE_POWER_SAVING
	PCIeWakeUp(priv, POWER_DOWN_T0);
#endif

		if (ant==Antenna_B || ant==Antenna_A) {
#ifdef GPIO_ANT_SWITCH		
		if(priv->pshare->rf_ft_var.antSw_enable)  {
			PHY_SetBBReg(priv, GPIO_PIN_CTRL, 0x3000, ant); 
		} else
#endif
		{
			if ( !priv->pshare->rf_ft_var.antSw_select) {
				RTL_W32(rFPGA0_XAB_RFInterfaceSW, RTL_R32(rFPGA0_XAB_RFInterfaceSW) | BIT(8)| BIT(9) );  //  ANTSEL A as SW control
				RTL_W8(0xc50, RTL_R8(0xc50) & (~ BIT(7)));	// rx OFDM SW control
				PHY_SetBBReg(priv, rFPGA0_XA_RFInterfaceOE, 0x300, ant);
			} else {
				RTL_W32(rFPGA0_XAB_RFInterfaceSW, RTL_R32(rFPGA0_XAB_RFInterfaceSW) | BIT(24)| BIT(25) ); // ANTSEL B as HW control
				PHY_SetBBReg(priv, rFPGA0_XB_RFInterfaceOE, 0x300, ant);
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
					RTL_W8(0xc58, RTL_R8(0xc58) & (~ BIT(7)));		// rx OFDM SW control
#endif
			}
			RTL_W8(0xa01, RTL_R8(0xa01) & (~ BIT(7)));	// rx CCK SW control
			RTL_W32(0x80c, RTL_R32(0x80c) & (~ BIT(21))); // select ant by tx desc
			RTL_W32(0x858, 0x569a569a);
		}
		if(HW_DIV_ENABLE)
			priv->pshare->rf_ft_var.antHw_enable = BIT(5);			
		priv->pshare->rf_ft_var.CurAntenna  = (ant%2);
#ifdef SW_ANT_SWITCH
		if(priv->pshare->rf_ft_var.antSw_enable) 
			priv->pshare->rf_ft_var.antSw_enable = BIT(5);
		priv->pshare->DM_SWAT_Table.CurAntenna = ant;
		priv->pshare->RSSI_test =0;
#endif
	}
	else if(ant==0){
#ifdef GPIO_ANT_SWITCH		
			if(priv->pshare->rf_ft_var.antHw_enable) 
#endif
			{
			if (!priv->pshare->rf_ft_var.antSw_select)  {
				RTL_W32(rFPGA0_XAB_RFInterfaceSW, RTL_R32(rFPGA0_XAB_RFInterfaceSW) & ~(BIT(8)| BIT(9)) );
				RTL_W8(0xc50, RTL_R8(0xc50) | BIT(7));	// OFDM HW control
			} else {
				RTL_W32(rFPGA0_XAB_RFInterfaceSW, RTL_R32(rFPGA0_XAB_RFInterfaceSW) & ~(BIT(24)| BIT(25)) );
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
				RTL_W8(0xc58, RTL_R8(0xc58) | BIT(7));	// OFDM HW control
#endif
			}

			RTL_W8(0xa01, RTL_R8(0xa01) | BIT(7));	// CCK HW control
			RTL_W32(0x80c, RTL_R32(0x80c) | BIT(21) ); // by tx desc
			priv->pshare->rf_ft_var.CurAntenna = 0;
			RTL_W32(0x858, 0x569a569a);
			priv->pshare->rf_ft_var.antHw_enable = 1;
#ifdef SW_ANT_SWITCH
			priv->pshare->rf_ft_var.antSw_enable = 0;
			priv->pshare->RSSI_test =0;
#endif
		}
#ifdef SW_ANT_SWITCH
		if(priv->pshare->rf_ft_var.antSw_enable) {
			dm_SW_AntennaSwitchInit(priv);
			RTL_W32(0x858, 0x569a569a);
			priv->pshare->lastTxOkCnt = priv->net_stats.tx_bytes;
			priv->pshare->lastRxOkCnt = priv->net_stats.rx_bytes;
			priv->pshare->rf_ft_var.antHw_enable = 0;
			priv->pshare->rf_ft_var.antSw_enable = 1;
		}
#endif

	}
#if defined(SW_ANT_SWITCH) && !defined(GPIO_ANT_SWITCH)
	else if(ant==3) {
		if(!priv->pshare->rf_ft_var.antSw_enable) {
			dm_SW_AntennaSwitchInit(priv);
			RTL_W32(0x858, 0x569a569a);
			priv->pshare->lastTxOkCnt = priv->net_stats.tx_bytes;
			priv->pshare->lastRxOkCnt = priv->net_stats.rx_bytes;
		}
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8188E) {
				RTL_W8(0xc50, RTL_R8(0xc50) & (~ BIT(7)));	// rx OFDM SW control
		} else
#endif
		{
			if ( !priv->pshare->rf_ft_var.antSw_select)
				RTL_W8(0xc50, RTL_R8(0xc50) & (~ BIT(7)));	// rx OFDM SW control
			else
				RTL_W8(0xc58, RTL_R8(0xc58) & (~ BIT(7)));	// rx OFDM SW control
		}

		RTL_W8(0xa01, RTL_R8(0xa01) & (~ BIT(7)));		// rx CCK SW control
		RTL_W32(0x80c, RTL_R32(0x80c) & (~ BIT(21))); 	// select ant by tx desc
		priv->pshare->rf_ft_var.antHw_enable = 0;
		priv->pshare->rf_ft_var.antSw_enable = 1;

	}
#endif

	return 1;
}

#endif

//3 ============================================================
//3 Dynamic Noise Control
//3 ============================================================

#if defined(CONFIG_RTL_92D_SUPPORT) && defined(CONFIG_RTL_NOISE_CONTROL)
void dnc_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	struct stat_info *pstat = NULL;
	unsigned int set_timer = 0;
	unsigned long flags;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (timer_pending(&priv->dnc_timer))
		del_timer_sync(&priv->dnc_timer);

#ifdef PCIE_POWER_SAVING
	if ((priv->pwr_state == L2) || (priv->pwr_state == L1))
			return;
#endif

	if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G) {
		//PHY_SetBBReg(priv, 0xb30, bMaskDWord, 0x00a00000);
		PHY_SetBBReg(priv, 0x870, bMaskDWord, 0x07600760);
		PHY_SetBBReg(priv, 0xc50, bMaskByte0, 0x20);
		PHY_SetBBReg(priv, 0xc58, bMaskByte0, 0x20);
	}
}
#endif


//3 ============================================================
//3 Leaving STA check
//3 ============================================================

#if defined(DETECT_STA_EXISTANCE) && (defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT))
// Check for STA existance. If STA disappears, disconnect it. Added by Annie, 2010-08-10.
void DetectSTAExistance(struct rtl8192cd_priv *priv, struct tx_rpt *report, struct stat_info *pstat)
{
	unsigned char tmpbuf[16];

	// Parameters
	const unsigned int		maxTxFailCnt = 300;		// MAX Tx fail packet count
	const unsigned int		minTxFailCnt = 30;		// MIN Tx fail packet count; this value should be less than maxTxFailCnt.
	const unsigned int		txFailSecThr= 3;			// threshold of Tx Fail Time (in second)

	// Temporarily change Retry Limit when TxFail. (tfrl: TxFailRetryLimit)
	const unsigned char	TFRL = 7;				// New Retry Limit value
	const unsigned char	TFRL_FailCnt = 2;		// Tx Fail Count threshold to set Retry Limit
	const unsigned char	TFRL_SetTime = 2;		// Time to set Retry Limit (in second)
	const unsigned char	TFRL_RcvTime = 10;		// Time to recover Retry Limit (in second)
	
	if(OPMODE & WIFI_STATION_STATE)
		return;

	if( report->txok != 0 )
	{ // Reset Counter
		pstat->tx_conti_fail_cnt = 0;
		pstat->tx_last_good_time = priv->up_time;
		pstat->leave = 0;
	}
	else if( report->txfail != 0 )
	{
		pstat->tx_conti_fail_cnt += report->txfail;
		DEBUG_WARN( "detect: txfail=%d, tx_conti_fail_cnt=%d\n", report->txfail, pstat->tx_conti_fail_cnt );

		if(	priv->up_time >= (pstat->tx_last_good_time+TFRL_SetTime) &&
			pstat->tx_conti_fail_cnt >= TFRL_FailCnt && 
			//!pstat->ht_cap_len && // legacy rate only
			!priv->pshare->bRLShortened )
		{ // Shorten retry limit, because AP spending too much time to send out g mode STA pending packets in HW queue.
			RTL_W16(RL, (TFRL&SRL_Mask)<<SRL_SHIFT|(TFRL&LRL_Mask)<<LRL_SHIFT);
			priv->pshare->bRLShortened = TRUE;
			DEBUG_WARN( "== Shorten RetryLimit to 0x%04X ==\n", RTL_R16(RL) );
		}

		if( 	(pstat->tx_conti_fail_cnt >= maxTxFailCnt) ||
			(pstat->tx_conti_fail_cnt >= minTxFailCnt && priv->up_time >= (pstat->tx_last_good_time+txFailSecThr) )
			)
		{ // This STA is considered as disappeared, so delete it.
			DEBUG_WARN( "** tx_conti_fail_cnt=%d (min=%d,max=%d)\n", pstat->tx_conti_fail_cnt, minTxFailCnt, maxTxFailCnt);
			DEBUG_WARN( "** tx_last_good_time=%d, up_time=%d (Thr:%d)\n", (int)pstat->tx_last_good_time, (int)priv->up_time, txFailSecThr );
			DEBUG_WARN( "AP is going to del_sta %02X:%02X:%02X:%02X:%02X:%02X\n", pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5] );

			sprintf((char *)tmpbuf, "%02x%02x%02x%02x%02x%02x", pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5]);
		
//			del_sta(priv, tmpbuf);
			++(pstat->leave);

			if (timer_pending(&priv->pshare->rl_recover_timer))
				del_timer_sync (&priv->pshare->rl_recover_timer);
			mod_timer(&priv->pshare->rl_recover_timer, jiffies + EXPIRE_TO*TFRL_RcvTime);

			// Reset Counter
			pstat->tx_conti_fail_cnt = 0;
			pstat->tx_last_good_time = priv->up_time;
		}
	}
}

// Timer callback function to recover hardware retry limit register. Added by Annie, 2010-08-10.
void RetryLimitRecovery(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	if( priv->pshare->bRLShortened )
	{
		RTL_W16(RL, (priv->pshare->RLShort&SRL_Mask)<<SRL_SHIFT|(priv->pshare->RLLong&LRL_Mask)<<LRL_SHIFT);
		priv->pshare->bRLShortened = FALSE;
		DEBUG_WARN( "== Recover RetryLimit to 0x%04X ==\n", RTL_R16(RL) );
	}
}

// Chack STA leaving status; per interface. Added by Annie, 2010-08-10.
unsigned char NoLeavingSTA(struct rtl8192cd_priv *priv)
{
	unsigned char bStaAllOK = TRUE;
	struct list_head *phead, *plist;
	struct stat_info *pstat;

	phead = &priv->asoc_list;
	if (!netif_running(priv->dev) || list_empty(phead))
		return bStaAllOK;

	plist = phead->next;
	while (plist != phead)  {
		pstat = list_entry(plist, struct stat_info, asoc_list);
		if( pstat->tx_conti_fail_cnt != 0 ) {
			bStaAllOK = FALSE;
			break;
		}
		plist = plist->next;
	}

	return bStaAllOK;
}

// Chack STA leaving status for all active interface and recover retry limit register value. Added by Annie, 2010-08-10.
void LeavingSTA_RLCheck(struct rtl8192cd_priv *priv)
{
	unsigned char bIfAllOK = TRUE;
	static int AllOKTimes = 0;
#ifdef MBSSID
	int i;
#endif
	// Parameter
	const unsigned char	TFRL_RcvTime = 10;		// Time to recover Retry Limit (in second)

	if( !NoLeavingSTA(priv) )
		bIfAllOK = FALSE;

#ifdef UNIVERSAL_REPEATER
	if (IS_ROOT_INTERFACE(priv) && GET_VXD_PRIV(priv) ) {
		if( !NoLeavingSTA(GET_VXD_PRIV(priv)) )
			bIfAllOK = FALSE;
	}
#endif

#ifdef MBSSID
	if (IS_ROOT_INTERFACE(priv)) {
		if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
				if (IS_DRV_OPEN(priv->pvap_priv[i])) {
					if( !NoLeavingSTA(priv->pvap_priv[i]) )
						bIfAllOK = FALSE;
				}
			}
		}
	}
#endif

	if( bIfAllOK ) {
		AllOKTimes ++;

		if( AllOKTimes >= TFRL_RcvTime )
			RetryLimitRecovery((unsigned long)priv);
	}
	else {
		AllOKTimes = 0;
	}
}
#endif


#ifdef CONFIG_RTL_92C_SUPPORT

/*
 *	PA Analog Pre-distortion Calibration R06
 */
void APK_MAIN(struct rtl8192cd_priv *priv, unsigned int is2T)
{
	unsigned int regD[PATH_NUM];
	unsigned int tmpReg, index, offset, path, i=0, pathbound = PATH_NUM, apkbound=6;
	unsigned int BB_backup[APK_BB_REG_NUM];
	unsigned int BB_REG[APK_BB_REG_NUM] = {0x904, 0xc04, 0x800, 0xc08, 0x874};
	unsigned int BB_AP_MODE[APK_BB_REG_NUM] = {0x00000020, 0x00a05430, 0x02040000, 0x000800e4, 0x00204000};
	unsigned int BB_normal_AP_MODE[APK_BB_REG_NUM] = {0x00000020, 0x00a05430, 0x02040000, 0x000800e4, 0x22204000};
	unsigned int AFE_backup[APK_AFE_REG_NUM];
	unsigned int AFE_REG[APK_AFE_REG_NUM] = {	0x85c, 0xe6c, 0xe70, 0xe74, 0xe78, 0xe7c, 0xe80, 0xe84,
											0xe88, 0xe8c, 0xed0, 0xed4, 0xed8, 0xedc, 0xee0, 0xeec};
	unsigned int MAC_backup[IQK_MAC_REG_NUM];
	unsigned int MAC_REG[IQK_MAC_REG_NUM] = {0x522, 0x550, 0x551, 0x040};
	unsigned int APK_RF_init_value[PATH_NUM][APK_BB_REG_NUM] = {{0x0852c, 0x1852c, 0x5852c, 0x1852c, 0x5852c},
																{0x2852e, 0x0852e, 0x3852e, 0x0852e, 0x0852e}};
	unsigned int APK_normal_RF_init_value[PATH_NUM][APK_BB_REG_NUM] =
							{	{0x0852c, 0x0a52c, 0x3a52c, 0x5a52c, 0x5a52c},	//path settings equal to path b settings
								{0x0852c, 0x0a52c, 0x5a52c, 0x5a52c, 0x5a52c}	};

	unsigned int		APK_RF_value_0[PATH_NUM][APK_BB_REG_NUM] =
							{	{0x52019, 0x52014, 0x52013, 0x5200f, 0x5208d},
															{0x5201a, 0x52019, 0x52016, 0x52033, 0x52050}};

	unsigned int APK_normal_RF_value_0[PATH_NUM][APK_BB_REG_NUM] =
							{	{0x52019, 0x52017, 0x52010, 0x5200d, 0x5206a},	//path settings equal to path b settings
								{0x52019, 0x52017, 0x52010, 0x5200d, 0x5206a}	};

	unsigned int AFE_on_off[PATH_NUM] = {0x04db25a4, 0x0b1b25a4};	//path A on path B off / path A off path B on
	unsigned int APK_offset[PATH_NUM] = {0xb68, 0xb6c};
	unsigned int APK_normal_offset[PATH_NUM] = {0xb28, 0xb98};
	unsigned int APK_value[PATH_NUM] = {0x92fc0000, 0x12fc0000};
	unsigned int APK_normal_value[PATH_NUM] = {0x92680000, 0x12680000};
	char	APK_delta_mapping[APK_BB_REG_NUM][13] = {{-4, -3, -2, -2, -1, -1, 0, 1, 2, 3, 4, 5, 6},
													{-4, -3, -2, -2, -1, -1, 0, 1, 2, 3, 4, 5, 6},
													{-6, -4, -2, -2, -1, -1, 0, 1, 2, 3, 4, 5, 6},
													{-1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6},
													{-11, -9, -7, -5, -3, -1, 0, 0, 0, 0, 0, 0, 0}};
	unsigned int APK_normal_setting_value_1[13] =
		{	0x01017018, 0xf7ed8f84, 0x1b1a1816, 0x2522201e, 0x322e2b28,
			0x433f3a36, 0x5b544e49, 0x7b726a62, 0xa69a8f84, 0xdfcfc0b3,
			0x12680000, 0x00880000, 0x00880000		};
	unsigned int APK_normal_setting_value_2[16] =
		{	0x01c7021d, 0x01670183, 0x01000123, 0x00bf00e2, 0x008d00a3,
			0x0068007b, 0x004d0059, 0x003a0042, 0x002b0031, 0x001f0025,
			0x0017001b, 0x00110014, 0x000c000f, 0x0009000b, 0x00070008,
			0x00050006	};


	unsigned int APK_normal_RF_init_value_old[PATH_NUM][APK_BB_REG_NUM] =
			{{0x0852c, 0x5a52c, 0x0a52c, 0x5a52c, 0x4a52c}, //path settings equal to path b settings
			 {0x0852c, 0x5a52c, 0x0a52c, 0x5a52c, 0x4a52c}};
	unsigned int APK_normal_RF_value_0_old[PATH_NUM][APK_BB_REG_NUM] =
			{{0x52019, 0x52017, 0x52010, 0x5200d, 0x5200a}, //path settings equal to path b settings
			 {0x52019, 0x52017, 0x52010, 0x5200d, 0x5200a}};
	unsigned int APK_normal_setting_value_1_old[13] =
			{0x01017018, 0xf7ed8f84, 0x40372d20, 0x5b554e48, 0x6f6a6560,
												0x807c7873, 0x8f8b8884, 0x9d999693, 0xa9a6a3a0, 0xb5b2afac,
												0x12680000, 0x00880000, 0x00880000};
	unsigned int APK_normal_setting_value_2_old[16] =
			{0x00810100, 0x00400056, 0x002b0032, 0x001f0024, 0x0019001c,
												0x00150017, 0x00120013, 0x00100011, 0x000e000f, 0x000c000d,
												0x000b000c, 0x000a000b, 0x0009000a, 0x00090009, 0x00080008,
												0x00080008};
	unsigned int AP_curve[PATH_NUM][APK_CURVE_REG_NUM];

	unsigned int APK_result[PATH_NUM][APK_BB_REG_NUM];	//val_1_1a, val_1_2a, val_2a, val_3a, val_4a
	unsigned int ThermalValue = 0;
	int BB_offset, delta_V, delta_offset;
	int newVerAPK = (IS_UMC_A_CUT_88C(priv)) ? 1 : 0;
	unsigned int *pAPK_normal_setting_value_1 = APK_normal_setting_value_1, *pAPK_normal_setting_value_2 = APK_normal_setting_value_2 ;
#ifdef HIGH_POWER_EXT_PA
	unsigned int tmp0x870=0, tmp0x860=0, tmp0x864=0;

	if(priv->pshare->rf_ft_var.use_ext_pa)
		newVerAPK = 1;
#endif

	if(!newVerAPK) {
		apkbound = 12;
		pAPK_normal_setting_value_1 = APK_normal_setting_value_1_old;
		pAPK_normal_setting_value_2 = APK_normal_setting_value_2_old;
	}

	if(!is2T)
		pathbound = 1;

	for(index = 0; index < PATH_NUM; index ++) {
		APK_offset[index] = APK_normal_offset[index];
		APK_value[index] = APK_normal_value[index];
		AFE_on_off[index] = 0x6fdb25a4;
	}

	for(index = 0; index < APK_BB_REG_NUM; index ++) {
		for(path = 0; path < pathbound; path++) {
			if(newVerAPK) {
			APK_RF_init_value[path][index] = APK_normal_RF_init_value[path][index];
			APK_RF_value_0[path][index] = APK_normal_RF_value_0[path][index];
			} else {
				APK_RF_init_value[path][index] = APK_normal_RF_init_value_old[path][index];
				APK_RF_value_0[path][index] = APK_normal_RF_value_0_old[path][index];
			}

		}
		BB_AP_MODE[index] = BB_normal_AP_MODE[index];
	}

	/*
	 *	save BB default value
	 */
	for(index = 1; index < APK_BB_REG_NUM ; index++)
		BB_backup[index] = PHY_QueryBBReg(priv, BB_REG[index], bMaskDWord);

#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		tmp0x870 = PHY_QueryBBReg(priv, 0x870, bMaskDWord);
		tmp0x860 = PHY_QueryBBReg(priv, 0x860, bMaskDWord);
		tmp0x864 = PHY_QueryBBReg(priv, 0x864, bMaskDWord);
	}
#endif

		//save MAC default value
	_PHY_SaveMACRegisters(priv, MAC_REG, MAC_backup);

	//save AFE default value
	_PHY_SaveADDARegisters(priv, AFE_REG, AFE_backup, APK_AFE_REG_NUM);

	for(path = 0; path < pathbound; path++) {
		/*
		 *	save old AP curve
		 */
		if(path == RF92CD_PATH_A) {
			/*
			 *	path A APK
			 *	load APK setting
			 *	path-A
			 */
			offset = 0xb00;
			for(index = 0; index < 11; index ++) {
				PHY_SetBBReg(priv, offset, bMaskDWord, pAPK_normal_setting_value_1[index]);
				offset += 0x04;
			}
			PHY_SetBBReg(priv, 0xb98, bMaskDWord, 0x12680000);

			offset = 0xb68;
			for(; index < 13; index ++) {
				PHY_SetBBReg(priv, offset, bMaskDWord, pAPK_normal_setting_value_1[index]);
				offset += 0x04;
			}

			/*
			 *	page-B1
			 */
			PHY_SetBBReg(priv, 0xe28, bMaskDWord, 0x40000000);

			/*
			 *path A
			 */
			offset = 0xb00;
			for(index = 0; index < 16; index++) {
				PHY_SetBBReg(priv, offset, bMaskDWord, pAPK_normal_setting_value_2[index]);
				offset += 0x04;
			}
			PHY_SetBBReg(priv, 0xe28, bMaskDWord, 0x00000000);
		} else if(path == RF92CD_PATH_B) {
			/*
			 *	path B APK
			 *	load APK setting
			 *	path-B
			 */
			offset = 0xb70;
			for(index = 0; index < 10; index ++) {
				PHY_SetBBReg(priv, offset, bMaskDWord, pAPK_normal_setting_value_1[index]);
				offset += 0x04;
			}
			PHY_SetBBReg(priv, 0xb28, bMaskDWord, 0x12680000);
			PHY_SetBBReg(priv, 0xb98, bMaskDWord, 0x12680000);

			offset = 0xb68;
			index = 11;
			for(; index < 13; index ++) {
				//offset 0xb68, 0xb6c
				PHY_SetBBReg(priv, offset, bMaskDWord, pAPK_normal_setting_value_1[index]);
				offset += 0x04;
			}

			/*
			 *	page-B1
			 */
			PHY_SetBBReg(priv, 0xe28, bMaskDWord, 0x40000000);

			/*
			 *	path B
			 */
			offset = 0xb60;
			for(index = 0; index < 16; index++) {
				PHY_SetBBReg(priv, offset, bMaskDWord, pAPK_normal_setting_value_2[index]);
				offset += 0x04;
			}
			PHY_SetBBReg(priv, 0xe28, bMaskDWord, 0x00000000);
		}

		if(!newVerAPK) {
		tmpReg = PHY_QueryRFReg(priv, (RF92CD_RADIO_PATH_E)path, 0x3, bMaskDWord, 1);

		AP_curve[path][0] = tmpReg & 0x1F;				//[4:0]

		tmpReg = PHY_QueryRFReg(priv, (RF92CD_RADIO_PATH_E)path, 0x4, bMaskDWord, 1);
		AP_curve[path][1] = (tmpReg & 0xF8000) >> 15; 	//[19:15]
		AP_curve[path][2] = (tmpReg & 0x7C00) >> 10;	//[14:10]
		AP_curve[path][3] = (tmpReg & 0x3E0) >> 5;		//[9:5]
		}

		/*
		 *	save RF default value
		 */
		regD[path] = PHY_QueryRFReg(priv, (RF92CD_RADIO_PATH_E)path, 0xd, bMaskDWord, 1);

		/*
		 *	Path A AFE all on, path B AFE All off or vise versa
		 */
		for(index = 0; index < APK_AFE_REG_NUM ; index++)
			PHY_SetBBReg(priv, AFE_REG[index], bMaskDWord, AFE_on_off[path]);

		/*
		 *	BB to AP mode
		 */
		if(path == RF92CD_PATH_A) {
			for(index = 1; index < APK_BB_REG_NUM ; index++)
				PHY_SetBBReg(priv, BB_REG[index], bMaskDWord, BB_AP_MODE[index]);
		}

#ifdef HIGH_POWER_EXT_PA
		if (priv->pshare->rf_ft_var.use_ext_pa) {
			PHY_SetBBReg(priv, 0x870, BIT(10), 1);
			PHY_SetBBReg(priv, 0x870, BIT(26), 1);
			PHY_SetBBReg(priv, 0x860, BIT(10), 0);
			PHY_SetBBReg(priv, 0x864, BIT(10), 0);
		}
#endif

		if(newVerAPK) {
			if(path == RF92CD_PATH_A) {
				PHY_SetBBReg(priv, 0xe30 , bMaskDWord, 0x01008c00);
				PHY_SetBBReg(priv, 0xe34 , bMaskDWord, 0x01008c00);
			} else if(path == RF92CD_PATH_B) {
				PHY_SetBBReg(priv, 0xe50 , bMaskDWord, 0x01008c00);
				PHY_SetBBReg(priv, 0xe54 , bMaskDWord, 0x01008c00);
			}
		}

		//MAC settings
		_PHY_MACSettingCalibration(priv, MAC_REG, MAC_backup);


		if(path == RF92CD_PATH_A) {
			//Path B to standby mode
			PHY_SetRFReg(priv, RF92CD_PATH_B, 0x0, bMaskDWord, 0x10000);
		} else {
			//Path A to standby mode
			PHY_SetRFReg(priv, RF92CD_PATH_A, 0x00, bMaskDWord, 0x10000);
			PHY_SetRFReg(priv, RF92CD_PATH_A, 0x10, bMaskDWord, 0x1000f);
			PHY_SetRFReg(priv, RF92CD_PATH_A, 0x11, bMaskDWord, 0x20103);
		}

		/*
		 *	Check Thermal value delta
		 */
		 if (priv->pmib->dot11RFEntry.ther) {
			ThermalValue = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x24, 0x1f, 1) & 0xff;
			 ThermalValue -= priv->pmib->dot11RFEntry.ther;
		 }

		 delta_offset = ((ThermalValue+14)/2);
		 if(delta_offset < 0)
			 delta_offset = 0;
		 else if (delta_offset > 12)
			 delta_offset = 12;

		//AP calibration
		for(index = 1; index < APK_BB_REG_NUM; index++) {
			tmpReg = APK_RF_init_value[path][index];
			if (priv->pmib->dot11RFEntry.ther) {
				BB_offset = (tmpReg & 0xF0000) >> 16;

				if(!(tmpReg & BIT(15))) //sign bit 0
					BB_offset = -BB_offset;
				delta_V = APK_delta_mapping[index][delta_offset];
				BB_offset += delta_V;

				if(BB_offset < 0) {
					tmpReg = tmpReg & (~BIT(15));
					BB_offset = -BB_offset;
				} else {
					tmpReg = tmpReg | BIT(15);
				}
				tmpReg = (tmpReg & 0xFFF0FFFF) | (BB_offset << 16);
			}

			if(newVerAPK)
				PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)path, 0xc, bMaskDWord, 0x8992e);
			else
			PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)path, 0xc, bMaskDWord, 0x8992f);

			PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)path, 0x0, bMaskDWord, APK_RF_value_0[path][index]);
			PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)path, 0xd, bMaskDWord, tmpReg);

			/*
			 *	PA11+PAD01111, one shot
			 */
			i = 0;
			do {
				PHY_SetBBReg(priv, 0xe28, bMaskDWord, 0x80000000);
				PHY_SetBBReg(priv, APK_offset[path], bMaskDWord, APK_value[0]);
				delay_ms(3);
				PHY_SetBBReg(priv, APK_offset[path], bMaskDWord, APK_value[1]);
				delay_ms(20);
				PHY_SetBBReg(priv, 0xe28, bMaskDWord, 0x00000000);

				if(!newVerAPK) {
				tmpReg = PHY_QueryRFReg(priv, (RF92CD_RADIO_PATH_E)path, 0xb, bMaskDWord, 1);
				tmpReg = (tmpReg & 0x3E00) >> 9;
				} else {
					if(path == RF92CD_PATH_A)
						tmpReg = PHY_QueryBBReg(priv, 0xbd8, 0x03E00000);
					else
						tmpReg = PHY_QueryBBReg(priv, 0xbd8, 0xF8000000);
				}
				i++;
			} while((tmpReg > apkbound) && i < 4);

			APK_result[path][index] = tmpReg;
		}
	}

	/*
	 *	reload MAC default value
	 */
	_PHY_ReloadMACRegisters(priv, MAC_REG, MAC_backup);

	/*
	 *	reload BB default value
	 */
	for(index = 1; index < APK_BB_REG_NUM ; index++)
		PHY_SetBBReg(priv, BB_REG[index], bMaskDWord, BB_backup[index]);

#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		PHY_SetBBReg(priv, 0x870, bMaskDWord, tmp0x870);
		PHY_SetBBReg(priv, 0x860, bMaskDWord, tmp0x860);
		PHY_SetBBReg(priv, 0x864, bMaskDWord, tmp0x864);
	}
#endif

	/*
	 *	reload AFE default value
	 */
	_PHY_ReloadADDARegisters(priv, AFE_REG, AFE_backup, 16);


	/*
	 *	reload RF path default value
	 */
	for(path = 0; path < pathbound; path++) {
		PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)path, 0xd, bMaskDWord, regD[path]);
		if(path == RF92CD_PATH_B) {
			PHY_SetRFReg(priv, RF92CD_PATH_A, 0x10, bMaskDWord, 0x1000f);
			PHY_SetRFReg(priv, RF92CD_PATH_A, 0x11, bMaskDWord, 0x20101);
		}

		if(newVerAPK) {
			if (APK_result[path][1] > 6)
				APK_result[path][1] = 6;
		} else {
			if(APK_result[path][1] < 1)
				APK_result[path][1] = 1;
			else if (APK_result[path][1] > 5)
				APK_result[path][1] = 5;

			if(APK_result[path][2] < 2)
				APK_result[path][2] = 2;
			else if (APK_result[path][2] > 6)
				APK_result[path][2] = 6;

			if(APK_result[path][3] < 2)
				APK_result[path][3] = 2;
			else if (APK_result[path][3] > 6)
				APK_result[path][3] = 6;

			if(APK_result[path][4] < 5)
				APK_result[path][4] = 5;
			else if (APK_result[path][4] > 9)
				APK_result[path][4] = 9;
		}
	}

	for(path = 0; path < pathbound; path++) {
		PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)path, 0x3, bMaskDWord,
		((APK_result[path][1] << 15) | (APK_result[path][1] << 10) | (APK_result[path][1] << 5) | APK_result[path][1]));
		if(newVerAPK) {
			if(path == RF92CD_PATH_A)
				PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)path, 0x4, bMaskDWord,
				((APK_result[path][1] << 15) | (APK_result[path][1] << 10) | (0x00 << 5) | 0x05));
			else
				PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)path, 0x4, bMaskDWord,
				((APK_result[path][1] << 15) | (APK_result[path][1] << 10) | (0x02 << 5) | 0x05));
			PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)path, 0xe, bMaskDWord,
			((0x08 << 15) | (0x08 << 10) | (0x08 << 5) | 0x08));
		} else {
			PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)path, 0x4, bMaskDWord,
			((APK_result[path][1] << 15) | (APK_result[path][1] << 10) | (APK_result[path][2] << 5) | APK_result[path][3]));
			PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)path, 0xe, bMaskDWord,
			((APK_result[path][4] << 15) | (APK_result[path][4] << 10) | (APK_result[path][4] << 5) | APK_result[path][4]));
		}
	}
}



/*
return FALSE => do IQK again
*/
char _PHY_SimularityCompare(struct rtl8192cd_priv *priv, int result[][8], unsigned char c1, unsigned char c2)
{
	unsigned int		i, j, diff, SimularityBitMap, bound = 0;
	unsigned char		final_candidate[2] = {0xFF, 0xFF};	//for path A and path B
	char				bResult = TRUE, is2T = (GET_CHIP_VER(priv) == VERSION_8192C ? 1 : 0);

	bound = (is2T) ? 8 : 4;
	SimularityBitMap = 0;

	for( i = 0; i < bound; i++ )	{
		diff = (result[c1][i] > result[c2][i]) ? (result[c1][i] - result[c2][i]) : (result[c2][i] - result[c1][i]);
		if (diff > MAX_TOLERANCE)		{
			if((i == 2 || i == 6) && !SimularityBitMap)		{
				if( result[c1][i]+ result[c1][i+1] == 0)
					final_candidate[(i>>2)] = c2;
				else if (result[c2][i]+result[c2][i+1] == 0)
					final_candidate[(i>>2)] = c1;
				else
					 SimularityBitMap |= (1<<i);
			}
			else
				SimularityBitMap |= (1<<i);
		}
	}

	if ( SimularityBitMap == 0)	{
		for( i = 0; i < (bound>>2); i++ )		{
			if(final_candidate[i] != 0xFF) 			{
				for( j = (i<<2); j < ((i+1)<<2)-2; j++)
					result[3][j] = result[final_candidate[i]][j];
				bResult = FALSE;
			}
		}
		return bResult;
	}
	else if (!(SimularityBitMap & 0x0F)) {			//path A OK
		for(i = 0; i < 4; i++)
			result[3][i] = result[c1][i];
		return FALSE;
	}
	else if (!(SimularityBitMap & 0xF0) && is2T) {	//path B OK
		for(i = 4; i < 8; i++)
			result[3][i] = result[c1][i];
		return FALSE;
	}
	else
		return FALSE;

}


//bit0 = 1 => Tx OK, bit1 = 1 => Rx OK
unsigned char _PHY_PathA_IQK(struct rtl8192cd_priv *priv, char	configPathB)
{
	unsigned int regEAC, regE94, regE9C, regEA4;
	unsigned char result = 0x00;

	//path-A IQK setting
//	RTPRINT(FINIT, INIT_IQK, ("Path-A IQK setting!\n"));
	PHY_SetBBReg(priv, 0xe30, bMaskDWord, 0x10008c1f);
	PHY_SetBBReg(priv, 0xe34, bMaskDWord, 0x10008c1f);
	PHY_SetBBReg(priv, 0xe38, bMaskDWord, 0x82140102);
	PHY_SetBBReg(priv, 0xe3c, bMaskDWord, ((configPathB |IS_UMC_B_CUT_88C(priv)) ? 0x28160202 : 0x28160502));

#if 1
	//path-B IQK setting
	if(configPathB)	{
		PHY_SetBBReg(priv, 0xe50, bMaskDWord, 0x10008c22);
		PHY_SetBBReg(priv, 0xe54, bMaskDWord, 0x10008c22);
		PHY_SetBBReg(priv, 0xe58, bMaskDWord, 0x82140102);
		PHY_SetBBReg(priv, 0xe5c, bMaskDWord, 0x28160202);
	}
#endif
	//LO calibration setting
	PHY_SetBBReg(priv, 0xe4c, bMaskDWord, 0x001028d1);

	//One shot, path A LOK & IQK
	PHY_SetBBReg(priv, 0xe48, bMaskDWord, 0xf9000000);
	PHY_SetBBReg(priv, 0xe48, bMaskDWord, 0xf8000000);

	// delay x ms
	delay_ms(IQK_DELAY_TIME);

	// Check failed
	regEAC = PHY_QueryBBReg(priv, 0xeac, bMaskDWord);
	regE94 = PHY_QueryBBReg(priv, 0xe94, bMaskDWord);
	regE9C= PHY_QueryBBReg(priv, 0xe9c, bMaskDWord);
	regEA4= PHY_QueryBBReg(priv, 0xea4, bMaskDWord);

	if(!(regEAC & BIT(28)) &&
		(((regE94 & 0x03FF0000)>>16) != 0x142) &&
		(((regE9C & 0x03FF0000)>>16) != 0x42) )
		result |= 0x01;
	else							//if Tx not OK, ignore Rx
		return result;

	if(!(regEAC & BIT(27)) &&		//if Tx is OK, check whether Rx is OK
		(((regEA4 & 0x03FF0000)>>16) != 0x132) &&
		(((regEAC & 0x03FF0000)>>16) != 0x36))
		result |= 0x02;
	else {
//		RTPRINT(FINIT, INIT_IQK, ("Path A Rx IQK fail!!\n"));
	}

	return result;
}

//bit0 = 1 => Tx OK, bit1 = 1 => Rx OK
unsigned char _PHY_PathB_IQK(struct rtl8192cd_priv *priv)
{
	unsigned int regEAC, regEB4, regEBC, regEC4, regECC;
	unsigned char	result = 0x00;
#if 0
	//path-B IQK setting
	RTPRINT(FINIT, INIT_IQK, ("Path-B IQK setting!\n"));
	PHY_SetBBReg(pAdapter, 0xe50, bMaskDWord, 0x10008c22);
	PHY_SetBBReg(pAdapter, 0xe54, bMaskDWord, 0x10008c22);
	PHY_SetBBReg(pAdapter, 0xe58, bMaskDWord, 0x82140102);
	PHY_SetBBReg(pAdapter, 0xe5c, bMaskDWord, 0x28160202);

	//LO calibration setting
	RTPRINT(FINIT, INIT_IQK, ("LO calibration setting!\n"));
	PHY_SetBBReg(pAdapter, 0xe4c, bMaskDWord, 0x001028d1);
#endif
	//One shot, path B LOK & IQK
//	RTPRINT(FINIT, INIT_IQK, ("One shot, path A LOK & IQK!\n"));
	PHY_SetBBReg(priv, 0xe60, bMaskDWord, 0x00000002);
	PHY_SetBBReg(priv, 0xe60, bMaskDWord, 0x00000000);

	// delay x ms
	delay_ms(IQK_DELAY_TIME);

	// Check failed
	regEAC = PHY_QueryBBReg(priv, 0xeac, bMaskDWord);
	regEB4 = PHY_QueryBBReg(priv, 0xeb4, bMaskDWord);
	regEBC= PHY_QueryBBReg(priv, 0xebc, bMaskDWord);
	regEC4= PHY_QueryBBReg(priv, 0xec4, bMaskDWord);
	regECC= PHY_QueryBBReg(priv, 0xecc, bMaskDWord);

	if(!(regEAC & BIT(31)) &&
		(((regEB4 & 0x03FF0000)>>16) != 0x142) &&
		(((regEBC & 0x03FF0000)>>16) != 0x42))
		result |= 0x01;
	else
		return result;

	if(!(regEAC & BIT(30)) &&
		(((regEC4 & 0x03FF0000)>>16) != 0x132) &&
		(((regECC & 0x03FF0000)>>16) != 0x36))
		result |= 0x02;
	else {
//		RTPRINT(FINIT, INIT_IQK, ("Path B Rx IQK fail!!\n"));
	}

	return result;

}

void _PHY_PathAFillIQKMatrix(struct rtl8192cd_priv *priv, char bIQKOK, int	result[][8], unsigned char final_candidate, char bTxOnly)
{
	int	Oldval_0, X, TX0_A, reg;
	int	Y, TX0_C;

	if(final_candidate == 0xFF)
		return;

	else if(bIQKOK) 	{
		Oldval_0 = (PHY_QueryBBReg(priv, rOFDM0_XATxIQImbalance, bMaskDWord) >> 22) & 0x3FF;

		X = result[final_candidate][0];
		if ((X & 0x00000200) != 0)
			X = X | 0xFFFFFC00;
		TX0_A = (X * Oldval_0) >> 8;
		PHY_SetBBReg(priv, rOFDM0_XATxIQImbalance, 0x3FF, TX0_A);
		PHY_SetBBReg(priv, rOFDM0_ECCAThreshold, BIT(31), ((X* Oldval_0>>7) & 0x1));

		Y = result[final_candidate][1];
		if ((Y & 0x00000200) != 0)
			Y = Y | 0xFFFFFC00;
		TX0_C = (Y * Oldval_0) >> 8;
		PHY_SetBBReg(priv, rOFDM0_XCTxAFE, 0xF0000000, ((TX0_C&0x3C0)>>6));
		PHY_SetBBReg(priv, rOFDM0_XATxIQImbalance, 0x003F0000, (TX0_C&0x3F));
		PHY_SetBBReg(priv, rOFDM0_ECCAThreshold, BIT(29), ((Y* Oldval_0>>7) & 0x1));

		if(bTxOnly)		{
//			RTPRINT(FINIT, INIT_IQK, ("_PHY_PathAFillIQKMatrix only Tx OK\n"));
			return;
		}

		reg = result[final_candidate][2];
		PHY_SetBBReg(priv, rOFDM0_XARxIQImbalance, 0x3FF, reg);

		reg = result[final_candidate][3] & 0x3F;
		PHY_SetBBReg(priv, rOFDM0_XARxIQImbalance, 0xFC00, reg);

		reg = (result[final_candidate][3] >> 6) & 0xF;
		PHY_SetBBReg(priv, 0xca0, 0xF0000000, reg);
	}
}


void _PHY_PathBFillIQKMatrix(struct rtl8192cd_priv *priv, char bIQKOK, int	result[][8], unsigned char final_candidate, char bTxOnly)
{
	int	Oldval_1, X, TX1_A, reg;
	int	Y, TX1_C;

	//RTPRINT(FINIT, INIT_IQK, ("Path B IQ Calibration %s !\n",(bIQKOK)?"Success":"Failed"));

	if(final_candidate == 0xFF)
		return;

	else if(bIQKOK)
	{
		Oldval_1 = (PHY_QueryBBReg(priv, rOFDM0_XBTxIQImbalance, bMaskDWord) >> 22) & 0x3FF;

		X = result[final_candidate][4];
		if ((X & 0x00000200) != 0)
			X = X | 0xFFFFFC00;
		TX1_A = (X * Oldval_1) >> 8;
		PHY_SetBBReg(priv, rOFDM0_XBTxIQImbalance, 0x3FF, TX1_A);
		PHY_SetBBReg(priv, rOFDM0_ECCAThreshold, BIT(27), ((X* Oldval_1>>7) & 0x1));

		Y = result[final_candidate][5];
		if ((Y & 0x00000200) != 0)
			Y = Y | 0xFFFFFC00;
		TX1_C = (Y * Oldval_1) >> 8;
		PHY_SetBBReg(priv, rOFDM0_XDTxAFE, 0xF0000000, ((TX1_C&0x3C0)>>6));
		PHY_SetBBReg(priv, rOFDM0_XBTxIQImbalance, 0x003F0000, (TX1_C&0x3F));
		PHY_SetBBReg(priv, rOFDM0_ECCAThreshold, BIT(25), ((Y* Oldval_1>>7) & 0x1));

		if(bTxOnly)
			return;

		reg = result[final_candidate][6];
		PHY_SetBBReg(priv, rOFDM0_XBRxIQImbalance, 0x3FF, reg);

		reg = result[final_candidate][7] & 0x3F;
		PHY_SetBBReg(priv, rOFDM0_XBRxIQImbalance, 0xFC00, reg);

		reg = (result[final_candidate][7] >> 6) & 0xF;
		PHY_SetBBReg(priv, rOFDM0_AGCRSSITable, 0x0000F000, reg);
	}
}



void _PHY_PathAStandBy(struct rtl8192cd_priv *priv)
{
	PHY_SetBBReg(priv, 0xe28, bMaskDWord, 0x0);
	PHY_SetBBReg(priv, 0x840, bMaskDWord, 0x00010000);
	PHY_SetBBReg(priv, 0xe28, bMaskDWord, 0x80800000);
}


void _PHY_IQCalibrate(struct rtl8192cd_priv *priv, int result[][8], unsigned char t, char is2T)
{
	unsigned int	i;
	unsigned char	PathAOK, PathBOK;
	unsigned int	ADDA_REG[IQK_ADDA_REG_NUM] = {	0x85c, 0xe6c, 0xe70, 0xe74,
													0xe78, 0xe7c, 0xe80, 0xe84,
													0xe88, 0xe8c, 0xed0, 0xed4,
													0xed8, 0xedc, 0xee0, 0xeec };
	unsigned int	IQK_MAC_REG[IQK_MAC_REG_NUM] = {0x522, 0x550,	0x551,	0x040};

	char isNormal = IS_TEST_CHIP(priv) ? 0 : 1;
	unsigned int	retryCount = 2;

#ifdef MP_TEST
	if(priv->pshare->rf_ft_var.mp_specific)
		retryCount = 9;
#endif

	if(t==0)	{
	 	// Save ADDA parameters, turn Path A ADDA on
	 	_PHY_SaveADDARegisters(priv, ADDA_REG, priv->pshare->ADDA_backup, APK_AFE_REG_NUM);
		_PHY_SaveMACRegisters(priv, IQK_MAC_REG, priv->pshare->IQK_MAC_backup);
	}

 	_PHY_PathADDAOn(priv, ADDA_REG, TRUE, is2T);

	if(t==0)	{
	 	// Store 0xC04, 0xC08, 0x874 vale
	 	priv->pshare->RegC04 = PHY_QueryBBReg(priv, 0xc04, bMaskDWord);
	 	priv->pshare->RegC08 = PHY_QueryBBReg(priv, 0xc08, bMaskDWord);
	 	priv->pshare->Reg874 = PHY_QueryBBReg(priv, 0x874, bMaskDWord);
	}

	PHY_SetBBReg(priv, 0x800, bMaskDWord, (PHY_QueryBBReg(priv, 0x800, bMaskDWord)& ~ BIT(24)));
	PHY_SetBBReg(priv, 0xc04, bMaskDWord, 0x03a05600);
	PHY_SetBBReg(priv, 0xc08, bMaskDWord, 0x000800e4);
	PHY_SetBBReg(priv, 0x874, bMaskDWord, 0x22204000);

	PHY_SetBBReg(priv, 0x870, BIT(10), 1);
	PHY_SetBBReg(priv, 0x870, BIT(26), 1);
	PHY_SetBBReg(priv, 0x860, BIT(10), 0);
	PHY_SetBBReg(priv, 0x864, BIT(10), 0);

	if(is2T) {
		PHY_SetBBReg(priv, 0x840, bMaskDWord, 0x00010000);
		PHY_SetBBReg(priv, 0x844, bMaskDWord, 0x00010000);
	}

	//MAC settings
	_PHY_MACSettingCalibration(priv, IQK_MAC_REG, priv->pshare->IQK_MAC_backup);

	//Page B init
	if(isNormal)
		PHY_SetBBReg(priv, 0xb68, bMaskDWord, 0x00080000);
	else
		PHY_SetBBReg(priv, 0xb68, bMaskDWord, 0x0f600000);

	if(is2T)	{
		if(isNormal)
			PHY_SetBBReg(priv, 0xb6c, bMaskDWord, 0x00080000);
		else
			PHY_SetBBReg(priv, 0xb6c, bMaskDWord, 0x0f600000);
	}

	// IQ calibration setting
	PHY_SetBBReg(priv, 0xe28, bMaskDWord, 0x80800000);
	PHY_SetBBReg(priv, 0xe40, bMaskDWord, 0x01007c00);
	PHY_SetBBReg(priv, 0xe44, bMaskDWord, 0x01004800);

	for(i = 0 ; i < retryCount ; i++){
		PathAOK = _PHY_PathA_IQK(priv, is2T);
		if(PathAOK == 0x03){
				result[t][0] = (PHY_QueryBBReg(priv, 0xe94, bMaskDWord)&0x3FF0000)>>16;
				result[t][1] = (PHY_QueryBBReg(priv, 0xe9c, bMaskDWord)&0x3FF0000)>>16;
				result[t][2] = (PHY_QueryBBReg(priv, 0xea4, bMaskDWord)&0x3FF0000)>>16;
				result[t][3] = (PHY_QueryBBReg(priv, 0xeac, bMaskDWord)&0x3FF0000)>>16;
			break;
		}
		else if (i == (retryCount-1) && PathAOK == 0x01)	//Tx IQK OK
		{
			result[t][0] = (PHY_QueryBBReg(priv, 0xe94, bMaskDWord)&0x3FF0000)>>16;
			result[t][1] = (PHY_QueryBBReg(priv, 0xe9c, bMaskDWord)&0x3FF0000)>>16;
		}
	}

	if(0x00 == PathAOK){
//		RTPRINT(FINIT, INIT_IQK, ("Path A IQK failed!!\n"));
	}

	if(is2T){
		_PHY_PathAStandBy(priv);

		// Turn Path B ADDA on
		_PHY_PathADDAOn(priv, ADDA_REG, FALSE, is2T);

		for(i = 0 ; i < retryCount ; i++){
			PathBOK = _PHY_PathB_IQK(priv);
			if(PathBOK == 0x03){
//				RTPRINT(FINIT, INIT_IQK, ("Path B IQK Success!!\n"));
				result[t][4] = (PHY_QueryBBReg(priv, 0xeb4, bMaskDWord)&0x3FF0000)>>16;
				result[t][5] = (PHY_QueryBBReg(priv, 0xebc, bMaskDWord)&0x3FF0000)>>16;
				result[t][6] = (PHY_QueryBBReg(priv, 0xec4, bMaskDWord)&0x3FF0000)>>16;
				result[t][7] = (PHY_QueryBBReg(priv, 0xecc, bMaskDWord)&0x3FF0000)>>16;
				break;
			}
			else if (i == (retryCount - 1) && PathBOK == 0x01)	//Tx IQK OK
			{
//				RTPRINT(FINIT, INIT_IQK, ("Path B Only Tx IQK Success!!\n"));
				result[t][4] = (PHY_QueryBBReg(priv, 0xeb4, bMaskDWord)&0x3FF0000)>>16;
				result[t][5] = (PHY_QueryBBReg(priv, 0xebc, bMaskDWord)&0x3FF0000)>>16;
			}
		}

		if(0x00 == PathBOK){
//			RTPRINT(FINIT, INIT_IQK, ("Path B IQK failed!!\n"));
		}
	}

	//Back to BB mode, load original value
//	RTPRINT(FINIT, INIT_IQK, ("IQK:Back to BB mode, load original value!\n"));
	PHY_SetBBReg(priv, 0xc04, bMaskDWord, priv->pshare->RegC04);
	PHY_SetBBReg(priv, 0x874, bMaskDWord, priv->pshare->Reg874);
	PHY_SetBBReg(priv, 0xc08, bMaskDWord, priv->pshare->RegC08);

	PHY_SetBBReg(priv, 0xe28, bMaskDWord, 0);

	// Restore RX initial gain
	PHY_SetBBReg(priv, 0x840, bMaskDWord, 0x00032ed3);

	if(is2T)
		PHY_SetBBReg(priv, 0x844, bMaskDWord, 0x00032ed3);

	if(t!=0) {
	 	// Reload ADDA power saving parameters
	 	_PHY_ReloadADDARegisters(priv, ADDA_REG, priv->pshare->ADDA_backup, 16);

		// Reload MAC parameters
		_PHY_ReloadMACRegisters(priv, IQK_MAC_REG, priv->pshare->IQK_MAC_backup);
	}
}


void PHY_IQCalibrate_92C(struct rtl8192cd_priv *priv)
{
	int				result[4][8];	//last is final result
	unsigned char	i, final_candidate;
	char			bPathAOK, bPathBOK;
	int				RegE94, RegE9C, RegEA4, RegEAC, RegEB4, RegEBC, RegEC4, RegECC, RegTmp = 0;
	char			is12simular, is13simular, is23simular;
	unsigned int 	temp_870, temp_860, temp_864, temp_800;

#ifdef MP_TEST
	if (!priv->pshare->rf_ft_var.mp_specific)
#endif
	{
		if (priv->pshare->iqk_2g_done)
			return;
		priv->pshare->iqk_2g_done = 1;
	}

	temp_870 = PHY_QueryBBReg(priv, 0x870, bMaskDWord);
	temp_860 = PHY_QueryBBReg(priv, 0x860, bMaskDWord);
	temp_864 = PHY_QueryBBReg(priv, 0x864, bMaskDWord);
	temp_800 = PHY_QueryBBReg(priv, 0x800, bMaskDWord);

	memset(result, 0, sizeof(result));

	final_candidate = 0xff;
	bPathAOK = FALSE;
	bPathBOK = FALSE;
	is12simular = FALSE;
	is23simular = FALSE;
	is13simular = FALSE;

	for (i=0; i<3; i++)	{
		 _PHY_IQCalibrate(priv, result, i, (GET_CHIP_VER(priv) == VERSION_8192C ? 1 : 0));

		if(i == 1)	{
			is12simular = _PHY_SimularityCompare(priv, result, 0, 1);
			if(is12simular) 			{
				final_candidate = 0;
				break;
			}
		}

		if(i == 2) 		{
			is13simular = _PHY_SimularityCompare(priv, result, 0, 2);
			if(is13simular)		{
				final_candidate = 0;
				break;
			}

			is23simular = _PHY_SimularityCompare(priv, result, 1, 2);
			if(is23simular)
				final_candidate = 1;
			else
			{
				for(i = 0; i < 8; i++)
					RegTmp += result[3][i];

				if(RegTmp != 0)
					final_candidate = 3;
				else
					final_candidate = 0xFF;
			}
		}
	}


	RTL_W32(0x870, temp_870);
	RTL_W32(0x860, temp_860);
	RTL_W32(0x864, temp_864);
	RTL_W32(0x800, temp_800);

	//load 0xe30 IQC default value
	if(GET_CHIP_VER(priv) == VERSION_8188C) {
		RTL_W32(0xe30, 0x01008c00);
		RTL_W32(0xe34, 0x01008c00);
	}

	for (i=0; i<4; i++)	{
		RegE94 = result[i][0];
		RegE9C = result[i][1];
		RegEA4 = result[i][2];
		RegEAC = result[i][3];
		RegEB4 = result[i][4];
		RegEBC = result[i][5];
		RegEC4 = result[i][6];
		RegECC = result[i][7];
		DEBUG_INFO("IQK: RegE94=%lx RegE9C=%lx RegEA4=%lx RegEAC=%lx RegEB4=%lx RegEBC=%lx RegEC4=%lx RegECC=%lx\n ", RegE94, RegE9C, RegEA4, RegEAC, RegEB4, RegEBC, RegEC4, RegECC);
	}

	if(final_candidate != 0xff)	{
		priv->pshare->RegE94 = RegE94 = result[final_candidate][0];
		priv->pshare->RegE9C = RegE9C = result[final_candidate][1];
		RegEA4 = result[final_candidate][2];
		RegEAC = result[final_candidate][3];
		priv->pshare->RegEB4 = RegEB4 = result[final_candidate][4];
		priv->pshare->RegEBC = RegEBC = result[final_candidate][5];
		RegEC4 = result[final_candidate][6];
		RegECC = result[final_candidate][7];
		DEBUG_INFO ("IQK: final_candidate is %x\n",final_candidate);
		DEBUG_INFO ("IQK: RegE94=%lx RegE9C=%lx RegEA4=%lx RegEAC=%lx RegEB4=%lx RegEBC=%lx RegEC4=%lx RegECC=%lx\n ", RegE94, RegE9C, RegEA4, RegEAC, RegEB4, RegEBC, RegEC4, RegECC);
		bPathAOK = bPathBOK = TRUE;
	}
	else 	{
		priv->pshare->RegE94 = priv->pshare->RegEB4 = 0x100;	//X default value
		priv->pshare->RegE9C = priv->pshare->RegEBC = 0x0;		//Y default value
	}

	if((RegE94 != 0)/*&&(RegEA4 != 0)*/)
		_PHY_PathAFillIQKMatrix(priv, bPathAOK, result, final_candidate, (RegEA4 == 0)? 1 :0);
	if(GET_CHIP_VER(priv) == VERSION_8192C){
		if((RegEB4 != 0)/*&&(RegEC4 != 0)*/)
		_PHY_PathBFillIQKMatrix(priv, bPathBOK, result, final_candidate, (RegEC4 == 0)? 1 :0);
	}
}

#endif





// 92d IQK
#ifdef CONFIG_RTL_92D_SUPPORT

// IQK

void IQK_92D_5G_n(struct rtl8192cd_priv *priv)
{
	unsigned int temp_800, temp_c04, temp_874, temp_c08, temp_870,
				temp_860, temp_864, temp_88c, temp_c50, temp_c58, temp_b30,
				switch2PI=0, X, reg; //Oldval_0, Oldval_1, TX0_A, TX1_A;
	u8 temp_522, temp_550, temp_551;
	unsigned int cal_num=0, cal_retry=0, ADDA_backup[IQK_ADDA_REG_NUM];
	int Y, result[8][3], result_final[8]; //TX0_C, TX1_C;

	unsigned int i, RX0REG0xe40[3], RX0REG0xe40_final=0, REG0xe40, REG0xe94, REG0xe9c, delay_count;
	unsigned int REG0xeac, RX1REG0xe40[3], RX1REG0xe40_final=0, REG0xeb4, REG0xea4,REG0xec4;
	unsigned char TX0IQKOK = FALSE, TX1IQKOK = FALSE;
	unsigned int TX_X0, TX_Y0, TX_X1, TX_Y1, RX_X0, RX_Y0, RX_X1, RX_Y1;
	unsigned int ADDA_REG[IQK_ADDA_REG_NUM] = {0x85c, 0xe6c, 0xe70, 0xe74,	0xe78, 0xe7c, 0xe80, 0xe84,
												0xe88, 0xe8c, 0xed0, 0xed4, 0xed8, 0xedc, 0xee0, 0xeec};
#ifdef CONFIG_RTL_92D_DMDP
	if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY)
		return IQK_92D_5G_phy0_n(priv);
#endif

	 //always do IQK for MP mode
#ifdef MP_TEST
	if (!priv->pshare->rf_ft_var.mp_specific)
#endif
	{
		if (priv->pmib->dot11RFEntry.macPhyMode == SINGLEMAC_SINGLEPHY) {
			if (priv->pshare->iqk_5g_done)
				return;
			priv->pshare->iqk_5g_done = 1;
		}
	}

	printk(">> %s \n",__FUNCTION__);
#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
	REG32(BSP_WDTCNR) |=  1 << 23;
#elif defined(CONFIG_RTL_8198B)
	REG32(BSP_WDTCNTRR) |= BSP_WDT_KICK;
#endif
	/*
	 * Save MAC default value
	 */
	temp_522 = RTL_R8(0x522);
	temp_550 = RTL_R8(0x550);
	temp_551 = RTL_R8(0x551);

	/*
	 *	Save BB Parameter
	 */
	temp_800 = RTL_R32(0x800);
	temp_c04 = RTL_R32(0xc04);
	temp_874 = RTL_R32(0x874);
	temp_c08 = RTL_R32(0xc08);
	temp_870 = RTL_R32(0x870);
	temp_860 = RTL_R32(0x860);
	temp_864 = RTL_R32(0x864);
	temp_88c = RTL_R32(0x88c);
	temp_c50 = RTL_R32(0xc50);	// 01/11/2011 update
	temp_c58 = RTL_R32(0xc58);	// 01/11/2011 update
	temp_b30 = RTL_R32(0xb30);	// 03/03/2011 update

	/*
	*	Save AFE Parameters
	*/
	for( i = 0 ; i < IQK_ADDA_REG_NUM ; i++)
		ADDA_backup[i] = RTL_R32(ADDA_REG[i]);

	/*
	 *	==============
	 *	Path-A TX/RX IQK
	 *	==============
	 */
	while (cal_num < 3) {
		/*
		 *	Path-A AFE all on
		 */
		for( i = 0 ; i < IQK_ADDA_REG_NUM ; i++)
			RTL_W32(ADDA_REG[i], 0x04db25a4);

		/*
		 *	MAC register setting
		 */
		RTL_W8(0x522, 0x3f);
		RTL_W8(0x550, RTL_R8(0x550)& (~BIT(3)));
		RTL_W8(0x551, RTL_R8(0x551)& (~BIT(3)));

		/*
		 *	IQK must be done in PI mode
		 */
		if (!PHY_QueryBBReg(priv, 0x820, BIT(8)) || !PHY_QueryBBReg(priv, 0x828, BIT(8))) {
			PHY_SetBBReg(priv, 0x820, bMaskDWord, 0x01000100);
			PHY_SetBBReg(priv, 0x828, bMaskDWord, 0x01000100);
			switch2PI++;
		}

		/*
		 *	BB setting
		 */
		PHY_SetBBReg(priv, 0x800, BIT(24), 0);
		PHY_SetBBReg(priv, 0xc04, bMaskDWord, 0x03a05600);
		PHY_SetBBReg(priv, 0xc08, bMaskDWord, 0x000800e4);
		PHY_SetBBReg(priv, 0x874, bMaskDWord, 0x22208000);
		PHY_SetBBReg(priv, 0x88c, BIT(23)|BIT(22)|BIT(21)|BIT(20), 0xf);
		PHY_SetBBReg(priv, 0xb30, bMaskDWord, 0x00a00000);	// 03/03/2011 update

		/*
		 *	AP or IQK
		 */
		//PHY_SetBBReg(priv, 0xb68, bMaskDWord, 0x0f600000);
		//PHY_SetBBReg(priv, 0xb6c, bMaskDWord, 0x0f600000);

		// IQK-R03 2011/02/16 update

		//path A AP setting for IQK
		PHY_SetBBReg(priv, 0xb00, bMaskDWord, 0);
		PHY_SetBBReg(priv, 0xb68, bMaskDWord, 0x20000000);
		//path B AP setting for IQK
		PHY_SetBBReg(priv, 0xb70, bMaskDWord, 0);
		PHY_SetBBReg(priv, 0xb6c, bMaskDWord, 0x20000000);

		/*
		 *	IQK global setting
		 */
		PHY_SetBBReg(priv, 0xe28, bMaskDWord, 0x80800000);
		PHY_SetBBReg(priv, 0xe40, bMaskDWord, 0x10007c00);
		PHY_SetBBReg(priv, 0xe44, bMaskDWord, 0x01004800);

		/*
		 *	path-A IQK setting
		 */
		PHY_SetBBReg(priv, 0xe30, bMaskDWord, 0x18008c1f);
		PHY_SetBBReg(priv, 0xe34, bMaskDWord, 0x18008c1f);
		PHY_SetBBReg(priv, 0xe38, bMaskDWord, 0x82140307);	// 01/11/2011 update
#ifdef USB_POWER_SUPPORT
		PHY_SetBBReg(priv, 0xe3c, bMaskDWord, 0x68160c66);
#else
		PHY_SetBBReg(priv, 0xe3c, bMaskDWord, 0x68160960);	// 01/11/2011 update
#endif

		/*
		 *	path-B IQK setting
		 */
		PHY_SetBBReg(priv, 0xe50, bMaskDWord, 0x18008c2f);
		PHY_SetBBReg(priv, 0xe54, bMaskDWord, 0x18008c2f);
		PHY_SetBBReg(priv, 0xe58, bMaskDWord, 0x82110000);
		PHY_SetBBReg(priv, 0xe5c, bMaskDWord, 0x68110000);

		/*
		 *	LO calibration setting
		 */
		PHY_SetBBReg(priv, 0xe4c, bMaskDWord, 0x00462911);

#ifdef USB_POWER_SUPPORT
		//	path-A TRSW setting
		PHY_SetBBReg(priv, 0x870, BIT(6)|BIT(5), 3);
		PHY_SetBBReg(priv, 0x860, BIT(6)|BIT(5), 3);
#else
		/*
		 *	path-A PA on
		 */
		/*
		PHY_SetBBReg(priv, 0x870, BIT(11)|BIT(10), 3);
		PHY_SetBBReg(priv, 0x870, BIT(6)|BIT(5), 3);
		PHY_SetBBReg(priv, 0x860, BIT(11)|BIT(10), 3);
		 */
		PHY_SetBBReg(priv, 0x870, bMaskDWord, 0x07000f60);	// 01/11/2011 update
		PHY_SetBBReg(priv, 0x860, bMaskDWord, 0x66e60e30);	// 01/11/2011 update
#endif
		/*
		 *	One shot, path A LOK & IQK
		 */
		PHY_SetBBReg(priv, 0xe48, bMaskDWord, 0xf9000000);
		PHY_SetBBReg(priv, 0xe48, bMaskDWord, 0xf8000000);

		/*
		 *	Delay 10 ms
		 */
		delay_ms(10);

		delay_count = 0;
		while (1){
			REG0xeac = PHY_QueryBBReg(priv, 0xeac, bMaskDWord);
			if ((REG0xeac&BIT(26))||(delay_count>20)){
				break;
			}else {
				delay_ms(1);
				delay_count++;
			}
		}
		/*
		 *	Check_TX_IQK_A_result
		 */
		REG0xe40 = PHY_QueryBBReg(priv, 0xe40, bMaskDWord);
		REG0xe94 = PHY_QueryBBReg(priv, 0xe94, bMaskDWord);
		if(((REG0xeac&BIT(28)) == 0) && (((REG0xe94&0x3FF0000)>>16)!=0x142)) {
			TX0IQKOK = TRUE;
			REG0xe9c = PHY_QueryBBReg(priv, 0xe9c, bMaskDWord);
			TX_X0 = (PHY_QueryBBReg(priv, 0xe94, bMaskDWord)&0x3FF0000)>>16;
			TX_Y0 = (PHY_QueryBBReg(priv, 0xe9c, bMaskDWord)&0x3FF0000)>>16;
			RX0REG0xe40[cal_num] =	(REG0xe40 & 0xfc00fc00) | (TX_X0<<16) | TX_Y0;
			DEBUG_INFO("TX_X0 %08x TX_Y0 %08x RX0REG0xe40 %08x\n", TX_X0, TX_Y0, RX0REG0xe40[cal_num]);
			result[0][cal_num] = TX_X0;
			result[1][cal_num] = TX_Y0;
		} else {
			TX0IQKOK = FALSE;
			if (++cal_retry >= 10) {
				printk("%s Path-A Tx/Rx Check\n",__FUNCTION__);
				break;
			}
		}

		/*
		 *	Check_RX_IQK_A_result
		 */
		if(TX0IQKOK == TRUE) {
			REG0xeac = PHY_QueryBBReg(priv, 0xeac, bMaskDWord);
			REG0xea4 = PHY_QueryBBReg(priv, 0xea4, bMaskDWord);
			if(((REG0xeac&BIT(27)) == 0) && (((REG0xea4&0x3FF0000)>>16)!=0x132)) {
				RX_X0 =  (PHY_QueryBBReg(priv, 0xea4, bMaskDWord)&0x3FF0000)>>16;
				RX_Y0 =  (PHY_QueryBBReg(priv, 0xeac, bMaskDWord)&0x3FF0000)>>16;
				DEBUG_INFO("RX_X0 %08x RX_Y0 %08x\n", RX_X0, RX_Y0);
				result[2][cal_num] = RX_X0;
				result[3][cal_num] = RX_Y0;
				cal_num++;
			} else {
				PHY_SetBBReg(priv, 0xc14, bMaskDWord, 0x40000100);
				PHY_SetBBReg(priv, 0xe34, bMaskDWord, 0x19008c00);
				if (++cal_retry >= 10) {
					printk("%s Path-A Tx/Rx Check\n",__FUNCTION__);
					break;
				}
			}
		}
	}

	if (cal_num == 3) {
		result_final[0] = get_mean_of_2_close_value(result[0]);
		result_final[1] = get_mean_of_2_close_value(result[1]);
		result_final[2] = get_mean_of_2_close_value(result[2]);
		result_final[3] = get_mean_of_2_close_value(result[3]);
		RX0REG0xe40_final = 0x80000000 | get_mean_of_2_close_value(RX0REG0xe40);

		priv->pshare->RegE94=result_final[0];
		priv->pshare->RegE9C=result_final[1];
	} else {
		priv->pshare->RegE94=0x100;
		priv->pshare->RegE9C=0x00;
	}

	/*
	 *	Path-A PA off
	 */
	PHY_SetBBReg(priv, 0x870, bMaskDWord, temp_870);
	PHY_SetBBReg(priv, 0x860, bMaskDWord, temp_860);


	/*
	 *	==============
	 *	Path-B TX/RX IQK
	 *	==============
	 */
	cal_num = cal_retry = 0;
	while (cal_num < 3) {
		/*
		 *	Path-B AFE all on
		 */
		for( i = 0 ; i < IQK_ADDA_REG_NUM ; i++)
			PHY_SetBBReg(priv, ADDA_REG[i], bMaskDWord, 0x0b1b25a4);

		/*
		 *	path-A IQK setting
		 */
		PHY_SetBBReg(priv, 0xe30, bMaskDWord, 0x18008c1f);
		PHY_SetBBReg(priv, 0xe34, bMaskDWord, 0x18008c1f);
		PHY_SetBBReg(priv, 0xe38, bMaskDWord, 0x82110000);
		PHY_SetBBReg(priv, 0xe3c, bMaskDWord, 0x68110000);

		/*
		 *	path-B IQK setting
		 */
		PHY_SetBBReg(priv, 0xe50, bMaskDWord, 0x18008c22);
		PHY_SetBBReg(priv, 0xe54, bMaskDWord, 0x18008c22);
		PHY_SetBBReg(priv, 0xe58, bMaskDWord, 0x82140307);	// 01/11/2011 update

		// 01/11/2011 update
#ifdef USB_POWER_SUPPORT
		PHY_SetBBReg(priv, 0xe5c, bMaskDWord, 0x68160c66);
#else
		PHY_SetBBReg(priv, 0xe5c, bMaskDWord, 0x68160960);	// 01/11/2011 update
#endif

		/*
		 *	LO calibration setting
		 */
		PHY_SetBBReg(priv, 0xe4c, bMaskDWord, 0x00462911);

#ifdef USB_POWER_SUPPORT
		PHY_SetBBReg(priv, 0x870, BIT(22)|BIT(21), 3);
		PHY_SetBBReg(priv, 0x864, BIT(6)|BIT(5), 3);
#else
		/*
		 *	path-B PA on
		 */
		/*
		PHY_SetBBReg(priv, 0x870, BIT(27)|BIT(26), 3);
		PHY_SetBBReg(priv, 0x870, BIT(22)|BIT(21), 3);
		PHY_SetBBReg(priv, 0x864, BIT(11)|BIT(10), 3);
		*/
		PHY_SetBBReg(priv, 0x870, bMaskDWord, 0x0f600700);
		PHY_SetBBReg(priv, 0x864, bMaskDWord, 0x061f0d30);
#endif

		/*
		 *	One shot, path A LOK & IQK
		 */
		PHY_SetBBReg(priv, 0xe60, bMaskDWord, 0x00000002);
		PHY_SetBBReg(priv, 0xe60, bMaskDWord, 0x00000000);

		/*
		 *	Delay 10 ms
		 */
		delay_ms(10);

		delay_count = 0;
		while (1){
			REG0xeac = PHY_QueryBBReg(priv, 0xeac, bMaskDWord);
			if ((REG0xeac&BIT(29))||(delay_count>20)){
				break;
			}else {
				delay_ms(1);
				delay_count++;
			}
		}
		/*
		 *	Check_TX_IQK_B_result
		 */
		REG0xe40 = PHY_QueryBBReg(priv, 0xe40, bMaskDWord);
		REG0xeac = PHY_QueryBBReg(priv, 0xeac, bMaskDWord);
		REG0xeb4 = PHY_QueryBBReg(priv, 0xeb4, bMaskDWord);
		if(((REG0xeac&BIT(31)) == 0) && ((REG0xeb4&0x3FF0000)!=0x142)) {
			TX1IQKOK = TRUE;
			TX_X1 = (PHY_QueryBBReg(priv, 0xeb4, bMaskDWord)&0x3FF0000)>>16;
			TX_Y1 = (PHY_QueryBBReg(priv, 0xebc, bMaskDWord)&0x3FF0000)>>16;
			RX1REG0xe40[cal_num] = (REG0xe40 & 0xfc00fc00) | (TX_X1<<16) | TX_Y1;
			DEBUG_INFO("TX_X1 %08x TX_Y1 %08x RX1REG0xe40 %08x\n", TX_X1, TX_Y1, RX1REG0xe40[cal_num]);
			result[4][cal_num] = TX_X1;
			result[5][cal_num] = TX_Y1;
		} else {
			TX1IQKOK = FALSE;
			if (++cal_retry >= 10) {
				printk("%s Path-B Tx/Rx Check\n",__FUNCTION__);
				break;
			}
		}

		/*
		 *	Check_RX_IQK_B_result
		 */
		if(TX1IQKOK == TRUE) {
			REG0xeac = PHY_QueryBBReg(priv, 0xeac, bMaskDWord);
			REG0xec4 = PHY_QueryBBReg(priv, 0xec4, bMaskDWord);
			if(((REG0xeac&BIT(30)) == 0) && (((REG0xec4&0x3FF0000)>>16)!=0x132)) {
				RX_X1 =  (PHY_QueryBBReg(priv, 0xec4, bMaskDWord)&0x3FF0000)>>16;
				RX_Y1 =  (PHY_QueryBBReg(priv, 0xecc, bMaskDWord)&0x3FF0000)>>16;
				DEBUG_INFO("RX_X1 %08x RX_Y1 %08x\n", RX_X1, RX_Y1);
				result[6][cal_num] = RX_X1;
				result[7][cal_num] = RX_Y1;
				cal_num++;
			} else {
				PHY_SetBBReg(priv, 0xc1c, bMaskDWord, 0x40000100);
				PHY_SetBBReg(priv, 0xe54, bMaskDWord, 0x19008c00);
				if (++cal_retry >= 10) {
					printk("%s Path-B Tx/Rx Check\n",__FUNCTION__);
					break;
				}
			}
		}
	}

	if (cal_num == 3) {
		result_final[4] = get_mean_of_2_close_value(result[4]);
		result_final[5] = get_mean_of_2_close_value(result[5]);
		result_final[6] = get_mean_of_2_close_value(result[6]);
		result_final[7] = get_mean_of_2_close_value(result[7]);
		RX1REG0xe40_final = 0x80000000 | get_mean_of_2_close_value(RX1REG0xe40);

		priv->pshare->RegEB4=result_final[4];
		priv->pshare->RegEBC=result_final[5];
	} else {
		priv->pshare->RegEB4=0x100;
		priv->pshare->RegEBC=0x00;
	}

	/*
	 *	Fill IQK result for Path A
	 */
	if (result_final[0]) {
		/*
		Oldval_0 = (PHY_QueryBBReg(priv, 0xc80, bMaskDWord) >> 22) & 0x3FF;
		X = result_final[0];
		if ((X & 0x00000200) != 0)
			X = X | 0xFFFFFC00;
		TX0_A = (X * Oldval_0) >> 8;
		PHY_SetBBReg(priv, 0xc80, 0x3FF, TX0_A);
		PHY_SetBBReg(priv, 0xc4c, BIT(24), ((X* Oldval_0>>7) & 0x1));

		Y = result_final[1];
		if ((Y & 0x00000200) != 0)
			Y = Y | 0xFFFFFC00;
		TX0_C = (Y * Oldval_0) >> 8;
		PHY_SetBBReg(priv, 0xc94, 0xF0000000, ((TX0_C&0x3C0)>>6));
		PHY_SetBBReg(priv, 0xc80, 0x003F0000, (TX0_C&0x3F));
		PHY_SetBBReg(priv, 0xc4c, BIT(26), ((Y* Oldval_0>>7) & 0x1));
		*/

		// IQK-R03 2011/02/16 update
		X = result_final[0];
		Y = result_final[1];
		//printk("X=%x Y=%x\n",X,Y);
		//Path-A OFDM_A
		PHY_SetBBReg(priv, 0xe30, 0x03FF0000, X);
		PHY_SetBBReg(priv, 0xc4c, BIT(24), 0);
		//Path-A OFDM_C
		PHY_SetBBReg(priv, 0xe30, 0x000003FF, Y);
		PHY_SetBBReg(priv, 0xc4c, BIT(26), 0);

		if(result_final[2]) {
			reg = result_final[2];
			PHY_SetBBReg(priv, 0xc14, 0x3FF, reg);
			reg = result_final[3] & 0x3F;
			PHY_SetBBReg(priv, 0xc14, 0xFC00, reg);

			reg = (result_final[3] >> 6) & 0xF;
			PHY_SetBBReg(priv, 0xca0, 0xF0000000, reg);
		
			PHY_SetBBReg(priv, 0xe34, 0x03FF0000, result_final[2]); // X
			PHY_SetBBReg(priv, 0xe34, 0x3FF, result_final[3]);  //Y			
		}
	}

	/*
	 *	Fill IQK result for Path B
	 */
	if (result_final[4]) {
		/*
		Oldval_1 = (PHY_QueryBBReg(priv, 0xc88, bMaskDWord) >> 22) & 0x3FF;

		X = result_final[4];
		if ((X & 0x00000200) != 0)
			X = X | 0xFFFFFC00;
		TX1_A = (X * Oldval_1) >> 8;
		PHY_SetBBReg(priv, 0xc88, 0x3FF, TX1_A);
		PHY_SetBBReg(priv, 0xc4c, BIT(28), ((X* Oldval_1>>7) & 0x1));

		Y = result_final[5];
		if ((Y & 0x00000200) != 0)
			Y = Y | 0xFFFFFC00;
		TX1_C = (Y * Oldval_1) >> 8;
		PHY_SetBBReg(priv, 0xc9c, 0xF0000000, ((TX1_C&0x3C0)>>6));
		PHY_SetBBReg(priv, 0xc88, 0x003F0000, (TX1_C&0x3F));
		PHY_SetBBReg(priv, 0xc4c, BIT(30), ((Y* Oldval_1>>7) & 0x1));
		*/

		// IQK-R03 2011/02/16 update
		X = result_final[4];
		Y = result_final[5];
		//printk("X=%x Y=%x\n",X,Y);
		//Path-A OFDM_A
		PHY_SetBBReg(priv, 0xe50, 0x03FF0000, X);
		PHY_SetBBReg(priv, 0xc4c, BIT(28), 0);
		//Path-A OFDM_C
		PHY_SetBBReg(priv, 0xe50, 0x000003FF, Y);
		PHY_SetBBReg(priv, 0xc4c, BIT(30), 0);

		if(result_final[6]) {
			reg = result_final[6];
			PHY_SetBBReg(priv, 0xc1c, 0x3FF, reg);

			reg = result_final[7] & 0x3F;
			PHY_SetBBReg(priv, 0xc1c, 0xFC00, reg);

			reg = (result_final[7] >> 6) & 0xF;
			PHY_SetBBReg(priv, 0xc78, 0x0000F000, reg);
			
			PHY_SetBBReg(priv, 0xe54, 0x03FF0000, result_final[6]); // X
			PHY_SetBBReg(priv, 0xe54, 0x3FF, result_final[7]);  //Y		
		}
	}

	/*
	 *	Path B PA off
	 */
	PHY_SetBBReg(priv, 0x870, bMaskDWord, temp_870);
	PHY_SetBBReg(priv, 0x864, bMaskDWord, temp_864);

	/*
	 *	Exit IQK mode
	 */
	PHY_SetBBReg(priv, 0xe28, bMaskDWord, 0);
	PHY_SetBBReg(priv, 0xc04, bMaskDWord, temp_c04);
	PHY_SetBBReg(priv, 0xc08, bMaskDWord, temp_c08);
	PHY_SetBBReg(priv, 0x874, bMaskDWord, temp_874);
	PHY_SetBBReg(priv, 0x800, bMaskDWord, temp_800);
	PHY_SetBBReg(priv, 0x88c, bMaskDWord, temp_88c);
	PHY_SetBBReg(priv, 0xb30, bMaskDWord, temp_b30);	// 03/03/2011 update
	//PHY_SetBBReg(priv, 0x840, bMaskDWord, 0x00032fff);		// 01/11/2011 update
	//PHY_SetBBReg(priv, 0x844, bMaskDWord, 0x00032fff);		// 01/11/2011 update

	// IQK-R03 2011/02/16 update
	//path A IQ path to DP block
	PHY_SetBBReg(priv, 0xb00, bMaskDWord, 0x010170b8);
	//path B IQ path to DP block
	PHY_SetBBReg(priv, 0xb70, bMaskDWord, 0x010170b8);

	//path AB to initial gain
	PHY_SetBBReg(priv, 0xc50, bMaskDWord, 0x50);		// 01/11/2011 update
	PHY_SetBBReg(priv, 0xc50, bMaskDWord, temp_c50);	// 01/11/2011 update
	PHY_SetBBReg(priv, 0xc58, bMaskDWord, 0x50);		// 01/11/2011 update
	PHY_SetBBReg(priv, 0xc58, bMaskDWord, temp_c58);	// 01/11/2011 update


	/*
	 *	Reload MAC default value
	 */
	RTL_W8(0x550, temp_550);
	RTL_W8(0x551, temp_551);
	RTL_W8(0x522, temp_522);

	/*
	 *	Switch back to SI if needed, after IQK
	 */
	if (switch2PI) {
		PHY_SetBBReg(priv, 0x820, bMaskDWord, 0x01000000);
		PHY_SetBBReg(priv, 0x828, bMaskDWord, 0x01000000);
	}

	/*
	 *	Reload ADDA power saving parameters
	 */
	for(i = 0 ; i < IQK_ADDA_REG_NUM ; i++)
		PHY_SetBBReg(priv, ADDA_REG[i], bMaskDWord, ADDA_backup[i]);


#if 0 //def CLIENT_MODE
		clnt_save_IQK_res(priv);
#endif

}


void IQK_92D_2G(struct rtl8192cd_priv *priv)
{
	unsigned int cal_num=0, cal_retry=0, Oldval=0, temp_c04=0, temp_c08=0, temp_874=0, temp_eac;
	unsigned int cal_e94, cal_e9c, cal_ea4, cal_eac, cal_eb4, cal_ebc, cal_ec4, cal_ecc;
	unsigned int X, Y, val_e94[3], val_e9c[3], val_ea4[3], val_eac[3], val_eb4[3], val_ebc[3], val_ec4[3], val_ecc[3];
	unsigned int ADDA_REG[IQK_ADDA_REG_NUM] = {0x85c, 0xe6c, 0xe70, 0xe74,	0xe78, 0xe7c, 0xe80, 0xe84,
												0xe88, 0xe8c, 0xed0, 0xed4, 0xed8, 0xedc, 0xee0, 0xeec};
	unsigned int ADDA_backup[IQK_ADDA_REG_NUM], i;
	u8 temp_522, temp_550, temp_551;
	u32 temp_040, temp_800, temp_870, temp_860, temp_864, temp_88c;
	u8 switch2PI = 0;

#ifdef CONFIG_RTL_92D_DMDP
	if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY)
		return IQK_92D_2G_phy1(priv);
#endif

	 //always do IQK for MP mode
#ifdef MP_TEST
	if (!priv->pshare->rf_ft_var.mp_specific)
#endif
	{
		if (priv->pmib->dot11RFEntry.macPhyMode == SINGLEMAC_SINGLEPHY) {
			if (priv->pshare->iqk_2g_done)
				return;
			priv->pshare->iqk_2g_done = 1;
		}
	}

	printk(">> %s \n",__FUNCTION__);

	// Save ADDA power saving parameters
	for( i = 0 ; i < IQK_ADDA_REG_NUM ; i++)
		ADDA_backup[i] = RTL_R32(ADDA_REG[i]);

	/*
	 * Save MAC default value
	 */
	temp_522 = RTL_R8(0x522);
	temp_550 = RTL_R8(0x550);
	temp_551 = RTL_R8(0x551);
	temp_040 = RTL_R32(0x40);

	// Save BB default
	temp_800 = RTL_R32(0x800);
	temp_870 = RTL_R32(0x870);
	temp_860 = RTL_R32(0x860);
	temp_864 = RTL_R32(0x864);
	temp_88c = RTL_R32(0x88c);

	// Path-A ADDA all on
	for( i = 0 ; i < IQK_ADDA_REG_NUM ; i++)
		RTL_W32(ADDA_REG[i], 0x04db25a4);

	// IQ&LO calibration Setting
	//IQK must be done in PI mode
	if (!PHY_QueryBBReg(priv, 0x820, BIT(8)) || !PHY_QueryBBReg(priv, 0x828, BIT(8))) {
		PHY_SetBBReg(priv, 0x820, bMaskDWord, 0x01000100);
		PHY_SetBBReg(priv, 0x828, bMaskDWord, 0x01000100);
		switch2PI++;
	}

	//BB setting
	temp_c04 = RTL_R32(0xc04);
	temp_c08 = RTL_R32(0xc08);
	temp_874 = RTL_R32(0x874);
	PHY_SetBBReg(priv,0x800,BIT(24),0);
	RTL_W32(0xc04, 0x03a05600);
	RTL_W32(0xc08, 0x000800e4);
	RTL_W32(0x874, 0x22204000);

	PHY_SetBBReg(priv, 0x870, BIT(10), 1);
	PHY_SetBBReg(priv, 0x870, BIT(26), 1);
	PHY_SetBBReg(priv, 0x860, BIT(10), 0);
	PHY_SetBBReg(priv, 0x864, BIT(10), 0);

	PHY_SetBBReg(priv,0x88c,0x00f00000,0xf);
	RTL_W32(0x840, 0x00010000);
	RTL_W32(0x844, 0x00010000);

	//MAC register setting
	RTL_W8(0x522, 0x3f);
	RTL_W8(0x550, RTL_R8(0x550)& (~BIT(3)));
	RTL_W8(0x551, RTL_R8(0x551)& (~BIT(3)));
	RTL_W32(0x40, 0);

	//AP or IQK
	RTL_W32(0xb68 , 0x0f600000);
	RTL_W32(0xb6c , 0x0f600000);

	// IQK setting
	RTL_W32(0xe28, 0x80800000);
	RTL_W32(0xe40, 0x01007c00);
	RTL_W32(0xe44, 0x01004800);
	// path-A IQK setting
	RTL_W32(0xe30, 0x10008c1f);
	RTL_W32(0xe34, 0x10008c1f);
	RTL_W32(0xe38, 0x82140102);
	RTL_W32(0xe3c, 0x28160206);
	// path-B IQK setting
	RTL_W32(0xe50, 0x10008c22);
	RTL_W32(0xe54, 0x10008c22);
	RTL_W32(0xe58, 0x82140102);
	RTL_W32(0xe5c, 0x28160206);
	// LO calibration setting
	RTL_W32(0xe4c, 0x00462911);

	// delay to ensure Path-A IQK success
	delay_ms(10);

	// step 4: One shot, path A LOK & IQK
	while (cal_num < 3) {
		// One shot, path A LOK & IQK
		RTL_W32(0xe48, 0xf9000000);
		RTL_W32(0xe48, 0xf8000000);
		// delay 1ms
		delay_ms(1);

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
		PHY_SetBBReg(priv, 0xc80, 0x3ff, (X * Oldval)>>8);
		PHY_SetBBReg(priv, 0xc4c, BIT(24), ((X * Oldval) >> 7) & 0x1);

		Y = cal_e9c;
		if ((Y & 0x00000200) != 0)
			Y = Y | 0xFFFFFC00;
		PHY_SetBBReg(priv, 0xc94, 0xf0000000, (((Y * Oldval) >> 8) >> 6) & 0xf);
		PHY_SetBBReg(priv, 0xc80, 0x003f0000, ((Y * Oldval) >> 8) & 0x3f);
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
	for( i = 0 ; i < IQK_ADDA_REG_NUM ; i++)
		RTL_W32(ADDA_REG[i], 0x0b1b25a4);

	// step 7: One shot, path B LOK & IQK
	cal_num = 0;
	cal_retry = 0;
	while (cal_num < 3) {
			// One shot, path B LOK & IQK
		RTL_W32(0xe60, 2);
		RTL_W32(0xe60, 0);
		// delay 1ms
		delay_ms(1);

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
		PHY_SetBBReg(priv, 0xc88, 0x3ff, (X * Oldval) >> 8 );
		PHY_SetBBReg(priv, 0xc4c, BIT(28), ((X * Oldval) >> 7) & 0x1);

		Y = cal_ebc;
		if ((Y & 0x00000200) != 0)
			Y = Y | 0xFFFFFC00;
		PHY_SetBBReg(priv, 0xc9c, 0xf0000000, (((Y * Oldval) >> 8 ) >> 6) & 0xf);
		PHY_SetBBReg(priv, 0xc88, 0x003f0000, ((Y * Oldval) >> 8 ) & 0x3f);
		PHY_SetBBReg(priv, 0xc4c, BIT(30), ((Y * Oldval) >> 7) & 0x1);

		PHY_SetBBReg(priv, 0xc1c, 0x3ff, cal_ec4);

		PHY_SetBBReg(priv, 0xc1c, 0xfc00, cal_ecc & 0x3f);

		PHY_SetBBReg(priv, 0xc78, 0xf000, (cal_ecc >> 6) & 0xf);
	}else {
		priv->pshare->RegEB4=0x100;
		priv->pshare->RegEBC=0x00;
	}

	// step 8: back to BB mode, load original values
	RTL_W32(0xe28, 0);
	RTL_W32(0xc04, temp_c04);
	RTL_W32(0xc08, temp_c08);
	RTL_W32(0x874, temp_874);
	RTL_W32(0x800, temp_800);
	RTL_W32(0x88c, temp_88c);
	RTL_W32(0x840, 0x32fff);
	RTL_W32(0x844, 0x32fff);
	RTL_W32(0x870, temp_870);
	RTL_W32(0x860, temp_860);
	RTL_W32(0x864, temp_864);

	/*
	 *	Switch back to SI if needed, after IQK
	 */
	if (switch2PI) {
		PHY_SetBBReg(priv, 0x820, bMaskDWord, 0x01000000);
		PHY_SetBBReg(priv, 0x828, bMaskDWord, 0x01000000);
	}

#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
	REG32(BSP_WDTCNR) |=  1 << 23;
#elif defined(CONFIG_RTL_8198B)
	REG32(BSP_WDTCNTRR) |= BSP_WDT_KICK;
#endif

	/*
	 *	Reload ADDA power saving parameters
	 */
	for(i = 0 ; i < IQK_ADDA_REG_NUM ; i++)
		PHY_SetBBReg(priv, ADDA_REG[i], bMaskDWord, ADDA_backup[i]);

	/*
	 *	Reload MAC default value
	 */
	RTL_W8(0x550, temp_550);
	RTL_W8(0x551, temp_551);
	RTL_W32(0x40, temp_040);
	RTL_W8(0x522, temp_522);


#if 0 //def CLIENT_MODE
	clnt_save_IQK_res(priv);
#endif

}

#ifdef CONFIG_RTL_92D_DMDP

void IQK_92D_2G_phy1(struct rtl8192cd_priv *priv)
{
	unsigned int cal_num=0, cal_retry=0, Oldval=0, temp_c04=0, temp_c08=0, temp_874=0, temp_eac;
	unsigned int cal_e94, cal_e9c, cal_ea4, cal_eac;
	unsigned int X, Y, val_e94[3], val_e9c[3], val_ea4[3], val_eac[3];
	unsigned int ADDA_REG[IQK_ADDA_REG_NUM] = {0x85c, 0xe6c, 0xe70, 0xe74, 	0xe78, 0xe7c, 0xe80, 0xe84,
												0xe88, 0xe8c, 0xed0, 0xed4, 0xed8, 0xedc, 0xee0, 0xeec};
	unsigned int ADDA_backup[IQK_ADDA_REG_NUM], i;
	u8 temp_522, temp_550, temp_551;
	u32 temp_040, temp_870, temp_860, temp_864, temp_800, temp_88c;
	u8 switch2PI = 0;


#ifdef MP_TEST
	if (!priv->pshare->rf_ft_var.mp_specific)
#endif
	{
		if (priv->pshare->iqk_2g_done)
			return;
		priv->pshare->iqk_2g_done = 1;
	}

	printk(">> %s \n",__FUNCTION__);

	// Save ADDA power saving parameters
	for( i = 0 ; i < IQK_ADDA_REG_NUM ; i++)
		ADDA_backup[i] = RTL_R32(ADDA_REG[i]);

	/*
	 * Save MAC default value
	 */
	temp_522 = RTL_R8(0x522);
	temp_550 = RTL_R8(0x550);
	temp_551 = RTL_R8(0x551);
	temp_040 = RTL_R32(0x40);

	// Save BB default
	temp_800 = RTL_R32(0x800);
	temp_870 = RTL_R32(0x870);
	temp_860 = RTL_R32(0x860);
	temp_864 = RTL_R32(0x864);
	temp_88c = RTL_R32(0x88c);

	// Path-A ADDA all on
	for( i = 0 ; i < IQK_ADDA_REG_NUM ; i++) {
#ifdef NON_INTR_ANTDIV
		if (DMDP_PHY_QueryBBReg(0, 0xb30,BIT(27)))
			RTL_W32(ADDA_REG[i], 0x04db25a4);
		else
#endif
		RTL_W32(ADDA_REG[i], 0x0b1b25a4);
	}


	// IQ&LO calibration Setting

	/*
	 *	IQK must be done in PI mode
	 */
	if (!PHY_QueryBBReg(priv, 0x820, BIT(8)) || !PHY_QueryBBReg(priv, 0x828, BIT(8))) {
		PHY_SetBBReg(priv, 0x820, bMaskDWord, 0x01000100);
		PHY_SetBBReg(priv, 0x828, bMaskDWord, 0x01000100);
		switch2PI++;
	}

	//BB setting
	temp_c04 = RTL_R32(0xc04);
	temp_c08 = RTL_R32(0xc08);
	temp_874 = RTL_R32(0x874);
	PHY_SetBBReg(priv, 0x800, BIT(24), 0);
	RTL_W32(0xc04, 0x03a05600);
	RTL_W32(0xc08, 0x000800e4);
	RTL_W32(0x874, 0x22204000);

	PHY_SetBBReg(priv, 0x870, BIT(10), 1);
	PHY_SetBBReg(priv, 0x870, BIT(26), 1);
	PHY_SetBBReg(priv, 0x860, BIT(10), 0);
	PHY_SetBBReg(priv, 0x864, BIT(10), 0);

	PHY_SetBBReg(priv,0x88c,0x00f00000,0xf);

	RTL_W32(0x840, 0x00010000);

	//MAC register setting
	RTL_W8(0x522, 0x3f);
	RTL_W8(0x550, RTL_R8(0x550)& (~BIT(3)));
	RTL_W8(0x551, RTL_R8(0x551)& (~BIT(3)));
	RTL_W32(0x40, 0);

	//AP or IQK
	RTL_W32(0xb68 , 0x0f600000);
	RTL_W32(0xb6c , 0x0f600000);

	// IQK setting
	RTL_W32(0xe28, 0x80800000);
	RTL_W32(0xe40, 0x01007c00);
	RTL_W32(0xe44, 0x01004800);

	// path-A IQK setting
	RTL_W32(0xe30, 0x10008c22);
	RTL_W32(0xe34, 0x10008c22);
	RTL_W32(0xe38, 0x82140102);
	RTL_W32(0xe3c, 0x28160206);

	// LO calibration setting
	RTL_W32(0xe4c, 0x00462911);

	// delay to ensure Path-A IQK success
	delay_ms(10);

	// step 4: One shot, path A LOK & IQK
	while (cal_num < 3) {
			// One shot, path A LOK & IQK
		RTL_W32(0xe48, 0xf9000000);
		RTL_W32(0xe48, 0xf8000000);
			// delay 1ms
		delay_ms(1);

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
		PHY_SetBBReg(priv, 0xc80, 0x3ff, (X * Oldval) >> 8);
		PHY_SetBBReg(priv, 0xc4c, BIT(24), ((X * Oldval) >> 7) & 0x1);

		Y = cal_e9c;
		if ((Y & 0x00000200) != 0)
			Y = Y | 0xFFFFFC00;
		PHY_SetBBReg(priv, 0xc94, 0xf0000000, (((Y * Oldval) >> 8) >> 6) & 0xf);
		PHY_SetBBReg(priv, 0xc80, 0x003f0000, ((Y * Oldval) >> 8) & 0x3f);
		PHY_SetBBReg(priv, 0xc4c, BIT(26), ((Y * Oldval) >> 7) & 0x1);

		PHY_SetBBReg(priv, 0xc14, 0x3ff, cal_ea4);

		PHY_SetBBReg(priv, 0xc14, 0xfc00, cal_eac & 0x3f);

		PHY_SetBBReg(priv, 0xca0, 0xf0000000, (cal_eac >> 6) & 0xf);
	}else {
		priv->pshare->RegE94=0x100;
		priv->pshare->RegE9C=0x00;
	}

	// back to BB mode, load original values
	RTL_W32(0xe28, 0);
	RTL_W32(0xc04, temp_c04);
	RTL_W32(0xc08, temp_c08);
	RTL_W32(0x874, temp_874);
	RTL_W32(0x800, temp_800);

	RTL_W32(0x88c, temp_88c);
	RTL_W32(0x840, 0x32fff);
	RTL_W32(0x870, temp_870);
	RTL_W32(0x860, temp_860);
	RTL_W32(0x864, temp_864);

	// return to SI mode
	if (switch2PI) {
		RTL_W32(0x820, 0x01000000);
		RTL_W32(0x828, 0x01000000);
	}

#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
	REG32(BSP_WDTCNR) |=  1 << 23;
#elif defined(CONFIG_RTL_8198B)
	REG32(BSP_WDTCNTRR) |= BSP_WDT_KICK;
#endif

	/*
	 *	Reload ADDA power saving parameters
	 */
	for(i = 0 ; i < IQK_ADDA_REG_NUM ; i++)
		PHY_SetBBReg(priv, ADDA_REG[i], bMaskDWord, ADDA_backup[i]);

	/*
	 *	Reload MAC default value
	 */
	RTL_W8(0x550, temp_550);
	RTL_W8(0x551, temp_551);
	RTL_W32(0x40, temp_040);
	RTL_W8(0x522, temp_522);

}


void IQK_92D_5G_phy0_n(struct rtl8192cd_priv *priv)
{
	unsigned int temp_800, temp_c04, temp_874, temp_c08, temp_870, temp_860, temp_88c, temp_c50, temp_b30,
				switch2PI=0, X, reg; //, Oldval_0, TX0_A;
	u8 temp_522, temp_550, temp_551;
	unsigned int cal_num=0, cal_retry=0, ADDA_backup[IQK_ADDA_REG_NUM];
	int Y, result[8][3], result_final[8]; //, TX0_C;

	unsigned int i, RX0REG0xe40[3], RX0REG0xe40_final=0, REG0xe40, REG0xe94, REG0xe9c, delay_count;
	unsigned int REG0xeac, REG0xea4;
	unsigned char TX0IQKOK = FALSE;
	unsigned int TX_X0, TX_Y0, RX_X0, RX_Y0;
	unsigned int ADDA_REG[IQK_ADDA_REG_NUM] = {0x85c, 0xe6c, 0xe70, 0xe74,	0xe78, 0xe7c, 0xe80, 0xe84,
												0xe88, 0xe8c, 0xed0, 0xed4, 0xed8, 0xedc, 0xee0, 0xeec};

#ifdef MP_TEST
	if (!priv->pshare->rf_ft_var.mp_specific)
#endif
	{
		if (priv->pshare->iqk_5g_done)
			return;
		priv->pshare->iqk_5g_done = 1;
	}

	printk(">> %s \n",__FUNCTION__);
#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
	REG32(BSP_WDTCNR) |=  1 << 23;
#elif defined(CONFIG_RTL_8198B)
	REG32(BSP_WDTCNTRR) |= BSP_WDT_KICK;
#endif
	/*
	 * Save MAC default value
	 */
	temp_522 = RTL_R8(0x522);
	temp_550 = RTL_R8(0x550);
	temp_551 = RTL_R8(0x551);

	/*
	 *	Save BB Parameter
	 */
	temp_800 = RTL_R32(0x800);
	temp_c04 = RTL_R32(0xc04);
	temp_874 = RTL_R32(0x874);
	temp_c08 = RTL_R32(0xc08);
	temp_870 = RTL_R32(0x870);
	temp_860 = RTL_R32(0x860);
	temp_88c = RTL_R32(0x88c);
	temp_c50 = RTL_R32(0xc50);
	temp_b30 = RTL_R32(0xb30);	// 03/03/2011 update

	/*
	*	Save AFE Parameters
	*/
	for( i = 0 ; i < IQK_ADDA_REG_NUM ; i++)
		ADDA_backup[i] = RTL_R32(ADDA_REG[i]);

	/*
	 *	==============
	 *	Path-A TX/RX IQK
	 *	==============
	 */
	while (cal_num < 3) {
		/*
		 *	Path-A AFE all on
		 */
		for( i = 0 ; i < IQK_ADDA_REG_NUM ; i++) {
#ifdef NON_INTR_ANTDIV
			if (DMDP_PHY_QueryBBReg(0, 0xb30,BIT(27)))
				RTL_W32(ADDA_REG[i], 0x0b1b25a4);
			else
#endif
			RTL_W32(ADDA_REG[i], 0x04db25a4);
		}

		/*
		 *	MAC register setting
		 */
		RTL_W8(0x522, 0x3f);
		RTL_W8(0x550, RTL_R8(0x550)& (~BIT(3)));
		RTL_W8(0x551, RTL_R8(0x551)& (~BIT(3)));

		/*
		 *	IQK must be done in PI mode
		 */
		if (!PHY_QueryBBReg(priv, 0x820, BIT(8)) || !PHY_QueryBBReg(priv, 0x828, BIT(8))) {
			PHY_SetBBReg(priv, 0x820, bMaskDWord, 0x01000100);
			PHY_SetBBReg(priv, 0x828, bMaskDWord, 0x01000100);
			switch2PI++;
		}

		/*
		 *	BB setting
		 */
		PHY_SetBBReg(priv, 0x800, BIT(24), 0);
		PHY_SetBBReg(priv, 0xc04, bMaskDWord, 0x03a05600);
		PHY_SetBBReg(priv, 0xc08, bMaskDWord, 0x000800e4);
		PHY_SetBBReg(priv, 0x874, bMaskDWord, 0x22208000);
		PHY_SetBBReg(priv, 0x88c, BIT(23)|BIT(22)|BIT(21)|BIT(20), 0xf);
		PHY_SetBBReg(priv, 0xb30, bMaskDWord, 0x00a00000);	// 03/03/2011 update

		/*
		 *	AP or IQK
		 */
		//PHY_SetBBReg(priv, 0xb68, bMaskDWord, 0x0f600000);
		//PHY_SetBBReg(priv, 0xb6c, bMaskDWord, 0x0f600000);

		// IQK-R03 2011/02/16 update
		PHY_SetBBReg(priv, 0xb00, bMaskDWord, 0);
		PHY_SetBBReg(priv, 0xb68, bMaskDWord, 0x20000000);

		/*
		 *	IQK global setting
		 */
		PHY_SetBBReg(priv, 0xe28, bMaskDWord, 0x80800000);
		PHY_SetBBReg(priv, 0xe40, bMaskDWord, 0x10007c00);
		PHY_SetBBReg(priv, 0xe44, bMaskDWord, 0x01004800);

		/*
		 *	path-A IQK setting
		 */
		PHY_SetBBReg(priv, 0xe30, bMaskDWord, 0x18008c1f);
		PHY_SetBBReg(priv, 0xe34, bMaskDWord, 0x18008c1f);
		PHY_SetBBReg(priv, 0xe38, bMaskDWord, 0x82140307);	// 01/11/2011 update

#ifdef USB_POWER_SUPPORT
		PHY_SetBBReg(priv, 0xe3c, bMaskDWord, 0x68160c66);
#else
		PHY_SetBBReg(priv, 0xe3c, bMaskDWord, 0x68160960);	// 01/11/2011 update
#endif
		/*
		 *	LO calibration setting
		 */
		PHY_SetBBReg(priv, 0xe4c, bMaskDWord, 0x00462911);

#ifdef USB_POWER_SUPPORT
		//	PHY0 TRSW seting
		PHY_SetBBReg(priv, 0x870, BIT(6)|BIT(5), 3);
		PHY_SetBBReg(priv, 0x860, BIT(6)|BIT(5), 3);
#else
		/*
		 *	path-A PA on
		 */
		/*
		PHY_SetBBReg(priv, 0x870, BIT(11)|BIT(10), 3);
		PHY_SetBBReg(priv, 0x870, BIT(6)|BIT(5), 3);
		PHY_SetBBReg(priv, 0x860, BIT(11)|BIT(10), 3);
		 */
		PHY_SetBBReg(priv, 0x870, bMaskDWord, 0x07000f60);	// 01/11/2011 update
		PHY_SetBBReg(priv, 0x860, bMaskDWord, 0x66e60e30);	// 01/11/2011 update
#endif
		/*
		 *	One shot, path A LOK & IQK
		 */
		PHY_SetBBReg(priv, 0xe48, bMaskDWord, 0xf9000000);
		PHY_SetBBReg(priv, 0xe48, bMaskDWord, 0xf8000000);

		/*
		 *	Delay 10 ms
		 */
		delay_ms(10);

		delay_count = 0;
		while (1){
			REG0xeac = PHY_QueryBBReg(priv, 0xeac, bMaskDWord);
			if ((REG0xeac&BIT(26))||(delay_count>20)){
				break;
			}else {
				delay_ms(1);
				delay_count++;
			}
		}

		/*
		 *	Check_TX_IQK_A_result
		 */
		REG0xe40 = PHY_QueryBBReg(priv, 0xe40, bMaskDWord);
		REG0xeac = PHY_QueryBBReg(priv, 0xeac, bMaskDWord);
		REG0xe94 = PHY_QueryBBReg(priv, 0xe94, bMaskDWord);
		if(((REG0xeac&BIT(28)) == 0) && (((REG0xe94&0x3FF0000)>>16)!=0x142)) {
			TX0IQKOK = TRUE;
			REG0xe9c = PHY_QueryBBReg(priv, 0xe9c, bMaskDWord);
			TX_X0 = (PHY_QueryBBReg(priv, 0xe94, bMaskDWord)&0x3FF0000)>>16;
			TX_Y0 = (PHY_QueryBBReg(priv, 0xe9c, bMaskDWord)&0x3FF0000)>>16;
			RX0REG0xe40[cal_num] =	(REG0xe40 & 0xfc00fc00) | (TX_X0<<16) | TX_Y0;
			DEBUG_INFO("TX_X0 %08x TX_Y0 %08x RX0REG0xe40 %08x\n", TX_X0, TX_Y0, RX0REG0xe40[cal_num]);
			result[0][cal_num] = TX_X0;
			result[1][cal_num] = TX_Y0;
		} else {
			TX0IQKOK = FALSE;
			if (++cal_retry >= 10) {
				printk("%s Path-A Tx/Rx Check\n",__FUNCTION__);
				break;
			}
		}

		/*
		 *	Check_RX_IQK_A_result
		 */
		if(TX0IQKOK == TRUE) {
			REG0xeac = PHY_QueryBBReg(priv, 0xeac, bMaskDWord);
			REG0xea4 = PHY_QueryBBReg(priv, 0xea4, bMaskDWord);
			if(((REG0xeac&BIT(27)) == 0) && (((REG0xea4&0x3FF0000)>>16)!=0x132)) {
				RX_X0 =  (PHY_QueryBBReg(priv, 0xea4, bMaskDWord)&0x3FF0000)>>16;
				RX_Y0 =  (PHY_QueryBBReg(priv, 0xeac, bMaskDWord)&0x3FF0000)>>16;
				DEBUG_INFO("RX_X0 %08x RX_Y0 %08x\n", RX_X0, RX_Y0);
				result[2][cal_num] = RX_X0;
				result[3][cal_num] = RX_Y0;
				cal_num++;
			} else {
				PHY_SetBBReg(priv, 0xc14, bMaskDWord, 0x40000100);
				PHY_SetBBReg(priv, 0xe34, bMaskDWord, 0x19008c00);
				if (++cal_retry >= 10) {
					printk("%s Path-A Tx/Rx Check\n",__FUNCTION__);
					break;
				}
			}
		}
	}

	if (cal_num == 3) {
		result_final[0] = get_mean_of_2_close_value(result[0]);
		result_final[1] = get_mean_of_2_close_value(result[1]);
		result_final[2] = get_mean_of_2_close_value(result[2]);
		result_final[3] = get_mean_of_2_close_value(result[3]);
		RX0REG0xe40_final = 0x80000000 | get_mean_of_2_close_value(RX0REG0xe40);

		priv->pshare->RegE94=result_final[0];
		priv->pshare->RegE9C=result_final[1];
	} else {
		priv->pshare->RegE94=0x100;
		priv->pshare->RegE9C=0x00;
	}

	/*
	 *	Fill IQK result for Path A
	 */
	if (result_final[0]) {
		/*
		Oldval_0 = (PHY_QueryBBReg(priv, 0xc80, bMaskDWord) >> 22) & 0x3FF;
		X = result_final[0];
		if ((X & 0x00000200) != 0)
			X = X | 0xFFFFFC00;
		TX0_A = (X * Oldval_0) >> 8;
		PHY_SetBBReg(priv, 0xc80, 0x3FF, TX0_A);
		PHY_SetBBReg(priv, 0xc4c, BIT(24), ((X* Oldval_0>>7) & 0x1));

		Y = result_final[1];
		if ((Y & 0x00000200) != 0)
			Y = Y | 0xFFFFFC00;
		TX0_C = (Y * Oldval_0) >> 8;
		PHY_SetBBReg(priv, 0xc94, 0xF0000000, ((TX0_C&0x3C0)>>6));
		PHY_SetBBReg(priv, 0xc80, 0x003F0000, (TX0_C&0x3F));
		PHY_SetBBReg(priv, 0xc4c, BIT(26), ((Y* Oldval_0>>7) & 0x1));
		*/

		// IQK-R03 2011/02/16 update
		X = result_final[0];
		Y = result_final[1];
		//printk("X=%x Y=%x\n",X,Y);
		//Path-A OFDM_A
		PHY_SetBBReg(priv, 0xe30, 0x03FF0000, X);
		PHY_SetBBReg(priv, 0xc4c, BIT(24), 0);
		//Path-A OFDM_C
		PHY_SetBBReg(priv, 0xe30, 0x000003FF, Y);
		PHY_SetBBReg(priv, 0xc4c, BIT(26), 0);


		if(result_final[2]) {
			reg = result_final[2];
			PHY_SetBBReg(priv, 0xc14, 0x3FF, reg);

			reg = result_final[3] & 0x3F;
			PHY_SetBBReg(priv, 0xc14, 0xFC00, reg);

			reg = (result_final[3] >> 6) & 0xF;
			PHY_SetBBReg(priv, 0xca0, 0xF0000000, reg);
			
			PHY_SetBBReg(priv, 0xe34, 0x03FF0000, result_final[2]); // X
			PHY_SetBBReg(priv, 0xe34, 0x3FF, result_final[3]);  //Y		
		}
	}

	/*
	 *	Path-A PA off
	 */
	PHY_SetBBReg(priv, 0x870, bMaskDWord, temp_870);
	PHY_SetBBReg(priv, 0x860, bMaskDWord, temp_860);

	/*
	 *	Exit IQK mode
	 */
	PHY_SetBBReg(priv, 0xe28, bMaskDWord, 0);
	PHY_SetBBReg(priv, 0xc04, bMaskDWord, temp_c04);
	PHY_SetBBReg(priv, 0xc08, bMaskDWord, temp_c08);
	PHY_SetBBReg(priv, 0x874, bMaskDWord, temp_874);
	PHY_SetBBReg(priv, 0x800, bMaskDWord, temp_800);
	PHY_SetBBReg(priv, 0x88c, bMaskDWord, temp_88c);
	PHY_SetBBReg(priv, 0xb30, bMaskDWord, temp_b30);	// 03/03/2011 update
	//PHY_SetBBReg(priv, 0x840, bMaskDWord, 0x00032fff);	// 01/11/2011 update

	//PHY0 IQ path to DP block
	PHY_SetBBReg(priv, 0xb00, bMaskDWord, 0x010170b8);

	PHY_SetBBReg(priv, 0xc50, bMaskDWord, 0x50);
	PHY_SetBBReg(priv, 0xc50, bMaskDWord, temp_c50);

	/*
	 *	Reload MAC default value
	 */
	RTL_W8(0x550, temp_550);
	RTL_W8(0x551, temp_551);
	RTL_W8(0x522, temp_522);

	/*
	 *	Switch back to SI if needed, after IQK
	 */
	if (switch2PI) {
		PHY_SetBBReg(priv, 0x820, bMaskDWord, 0x01000000);
		PHY_SetBBReg(priv, 0x828, bMaskDWord, 0x01000000);
	}

	/*
	 *	Reload ADDA power saving parameters
	 */
	for(i = 0 ; i < IQK_ADDA_REG_NUM ; i++)
		PHY_SetBBReg(priv, ADDA_REG[i], bMaskDWord, ADDA_backup[i]);
}
#endif

#ifdef SW_LCK_92D

void _PHY_CalcCurvIndex(struct rtl8192cd_priv *priv, unsigned int *TargetChnl,
								unsigned int *CurveCountVal, char is5G,  unsigned int *CurveIndex)
{
	unsigned int	smallestABSVal = 0xffffffff, u4tmp;
	unsigned char	i, j;
	unsigned char	chnl_num = is5G?(TARGET_CHNL_NUM_5G) : (TARGET_CHNL_NUM_2G);


	for(i=0; i<chnl_num; i++)
	{
		//if(is5G && !IsLegal5GChannel(pAdapter, i+1))
			//continue;

		CurveIndex[i] = 0;
		for(j=0; j<(CV_CURVE_CNT*2); j++)
		{
			u4tmp = RTL_ABS(TargetChnl[i], CurveCountVal[j]);
			//if (i==115)
				//printk("cv[%d]=%x\n", j, u4tmp);
			if(u4tmp < smallestABSVal)
			{
				CurveIndex[i] = j;
				smallestABSVal = u4tmp;
			}
		}

		smallestABSVal = 0xffffffff;
	}
}

void phy_ReloadLCKSetting(struct rtl8192cd_priv *priv)
{
	unsigned int	eRFPath = priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G? RF92CD_PATH_A:(priv->pmib->dot11RFEntry.macPhyMode==SINGLEMAC_SINGLEPHY?RF92CD_PATH_B:RF92CD_PATH_A);
	unsigned int 	u4tmp = 0;
//	unsigned char	bNeedPowerDownRadio = FALSE;
	unsigned int 	channel = priv->pshare->RegRF18[eRFPath]&0xff;
	//unsigned int 	channel =  PHY_QueryRFReg(priv, eRFPath, rRfChannel, 0xff, 1);

	DEBUG_INFO("====>phy_ReloadLCKSetting interface %d path %d ch %d [0x%05x]\n", priv->pshare->wlandev_idx, eRFPath, channel, priv->pshare->RegRF28[eRFPath]);

	//only for 92D C-cut SMSP
#if 1
	if(GET_CHIP_VER(priv)!=VERSION_8192D || IS_TEST_CHIP(priv))
		return;
#endif

	if(priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G)
	{
		//Path-A for 5G
		u4tmp = CurveIndex_5G[channel-1];
		//printk("%s ver 1 set RF-A, 5G,	0x28 = 0x%x !!\n",__FUNCTION__, u4tmp);
#ifdef CONFIG_RTL_92D_DMDP
		if(priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY && priv->pshare->wlandev_idx == 1)
		{
			priv->pshare->RegRF28[RF92CD_PATH_A] = RTL_SET_MASK(priv->pshare->RegRF28[RF92CD_PATH_A],0x3f800,u4tmp,11);	 //DMDP_PHY_SetRFReg(0,  RF92CD_PATH_A, 0x28, 0x3f800, u4tmp);
			DMDP_PHY_SetRFReg(0,  RF92CD_PATH_A, 0x28, bMask20Bits, priv->pshare->RegRF28[RF92CD_PATH_A]);
		}else
#endif
		{
			priv->pshare->RegRF28[eRFPath] = RTL_SET_MASK(priv->pshare->RegRF28[eRFPath],0x3f800,u4tmp,11);	//PHY_SetRFReg(priv, eRFPath, 0x28, 0x3f800, u4tmp);
			PHY_SetRFReg(priv, eRFPath, 0x28, bMask20Bits, priv->pshare->RegRF28[eRFPath]);
		}
		DEBUG_INFO("%s ver 3 set RF-B, 2G, 0x28 = 0x%05x [0x%05x]!!\n", __FUNCTION__, PHY_QueryRFReg(priv, eRFPath, 0x28, bMask20Bits, 1), priv->pshare->RegRF28[eRFPath]);
	}
	else if(priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G)
	{
		u4tmp = CurveIndex_2G[channel-1];
		//printk("%s ver 3 set RF-B, 2G, 0x28 = 0x%x !!\n", __FUNCTION__, u4tmp);
#ifdef CONFIG_RTL_92D_DMDP
		if(priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY && priv->pshare->wlandev_idx == 0)
		{
			priv->pshare->RegRF28[RF92CD_PATH_A] = RTL_SET_MASK(priv->pshare->RegRF28[RF92CD_PATH_A],0x3f800,u4tmp,11);	 //DMDP_PHY_SetRFReg(1,  RF92CD_PATH_A, 0x28, 0x3f800, u4tmp);
			DMDP_PHY_SetRFReg(1,  RF92CD_PATH_A, 0x28, bMask20Bits, priv->pshare->RegRF28[RF92CD_PATH_A]);
		}else
#endif
		{
			priv->pshare->RegRF28[eRFPath] = RTL_SET_MASK(priv->pshare->RegRF28[eRFPath],0x3f800,u4tmp,11); // PHY_SetRFReg(priv, eRFPath, 0x28, 0x3f800, u4tmp);
			PHY_SetRFReg(priv, eRFPath, 0x28, bMask20Bits, priv->pshare->RegRF28[eRFPath]);
		}
		DEBUG_INFO("%s ver 3 set RF-B, 2G, 0x28 = 0x%05x [0x%05x]!!\n", __FUNCTION__, PHY_QueryRFReg(priv, eRFPath, 0x28, bMask20Bits, 1), priv->pshare->RegRF28[eRFPath]);
	}

}

/* Software LCK */
void PHY_LCCalibrate_92D(struct rtl8192cd_priv *priv)
{
	unsigned char	tmpReg;
	unsigned int 	RF_mode[2];
	unsigned int	eRFPath, curMaxRFPath;
	unsigned int	i;
	unsigned int	curveCountVal[CV_CURVE_CNT*2]={0};
	unsigned short	timeout = 800, timecount = 0;

	if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY)
		curMaxRFPath = RF92CD_PATH_B;
	else
		curMaxRFPath = RF92CD_PATH_MAX;

	//Check continuous TX and Packet TX
	tmpReg = RTL_R8(0xd03);

	if ((tmpReg & 0x70) != 0)				// Deal with contisuous TX case
		RTL_W8(0xd03, tmpReg&0x8F);	// disable all continuous TX
	else									// Deal with Packet TX case
		RTL_W8(TXPAUSE, 0xFF);			// block all queues

	PHY_SetBBReg(priv, rFPGA0_AnalogParameter4, 0xF00000, 0x0F);

	for(eRFPath = RF92CD_PATH_A; eRFPath < curMaxRFPath; eRFPath++) {
		// 1. Read original RF mode
		RF_mode[eRFPath] = PHY_QueryRFReg(priv, eRFPath, 0x00, bMask20Bits, 1);

		// 2. Set RF mode = standby mode
		PHY_SetRFReg(priv, eRFPath, 0x00, 0x70000, 0x01);

		// switch CV-curve control by LC-calibration
		PHY_SetRFReg(priv, eRFPath, 0x2B, BIT(17), 0x0);

		// jenyu suggest
		PHY_SetRFReg(priv, eRFPath, 0x28, BIT(8), 0x1);

		//4. Set LC calibration begin
		PHY_SetRFReg(priv, eRFPath, 0x18, BIT(15), 0x01);

		while(!(PHY_QueryRFReg(priv, eRFPath, 0x2A, BIT(11), 1) &&
				timecount <= timeout)){
			//DEBUG_INFO("PHY_LCK delay for %d ms=2\n", timecount);
			delay_ms(50);
			timecount+=50;
		}

		//u4tmp = PHY_QueryRFReg(priv, eRFPath, 0x28, bMask20Bits, 1);

		memset((void *)curveCountVal, 0, CV_CURVE_CNT*2);

		//Set LC calibration off
		PHY_SetRFReg(priv, eRFPath, 0x18, BIT(15), 0x00);

		// jenyu suggest
		PHY_SetRFReg(priv, eRFPath, 0x28, BIT(8), 0x0);

		//save Curve-counting number
		for(i=0; i<CV_CURVE_CNT; i++)
		{
			unsigned int readVal=0, readVal2=0;

			PHY_SetRFReg(priv, eRFPath, 0x3F, 0x7f, i);

			PHY_SetRFReg(priv, eRFPath, 0x4D, bMask20Bits, 0);

			readVal = PHY_QueryRFReg(priv, eRFPath, 0x4F, bMask20Bits, 1);

			curveCountVal[2*i+1] = (readVal & 0xfffe0) >> 5;

			readVal2 = PHY_QueryRFReg(priv, eRFPath, 0x50, 0xffc00, 1);

			curveCountVal[2*i] = (((readVal & 0x1F) << 10) | readVal2);
		}

		if(eRFPath == RF92CD_PATH_A
#ifdef CONFIG_RTL_92D_DMDP
			&& priv->pshare->wlandev_idx == 0
#endif
			)
			_PHY_CalcCurvIndex(priv, TargetChnl_5G, curveCountVal, TRUE, CurveIndex_5G);
		else
			_PHY_CalcCurvIndex(priv, TargetChnl_2G, curveCountVal, FALSE, CurveIndex_2G);

		// switch CV-curve control mode
		PHY_SetRFReg(priv, eRFPath, 0x2B, BIT(17), 0x1);

		// store 0x28 for Reload_LCK
		priv->pshare->RegRF28[eRFPath] = PHY_QueryRFReg(priv, eRFPath, 0x28, bMask20Bits, 1);
	}

	//Restore original situation
	for(eRFPath = RF92CD_PATH_A; eRFPath < curMaxRFPath; eRFPath++)
	{
		PHY_SetRFReg(priv, eRFPath, 0x00, bMask20Bits, RF_mode[eRFPath]);
	}

	if((tmpReg&0x70) != 0)
	{
		//Path-A
		RTL_W8(0xd03, tmpReg);
	}
	else // Deal with Packet TX case
	{
		RTL_W8(TXPAUSE, 0x00);
	}

	PHY_SetBBReg(priv, rFPGA0_AnalogParameter4, 0xF00000, 0x00);

	phy_ReloadLCKSetting(priv);

}

#else
/* Hardware LCK */
static void PHY_LCCalibrate_92D(struct rtl8192cd_priv *priv)
{
	unsigned char tmpReg;
	unsigned int RF_mode[2], tmpu4Byte[2];

	unsigned int eRFPath, curMaxRFPath;
	unsigned char	timeout = 800, timecount = 0;

	if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY)
		curMaxRFPath = RF92CD_PATH_B;
	else
		curMaxRFPath = RF92CD_PATH_MAX;


	// Check continuous TX and Packet TX
	tmpReg = RTL_R8(0xd03);

	if ((tmpReg & 0x70) != 0)				// Deal with contisuous TX case
		RTL_W8(0xd03, tmpReg&0x8F);	// disable all continuous TX
	else									// Deal with Packet TX case
		RTL_W8(TXPAUSE, 0xFF);			// block all queues

	PHY_SetBBReg(priv, rFPGA0_AnalogParameter4, 0xF00000, 0x0F);

	for(eRFPath = RF92CD_PATH_A; eRFPath < curMaxRFPath; eRFPath++) {
		// 1. Read original RF mode
		RF_mode[eRFPath] = PHY_QueryRFReg(priv, eRFPath, 0x00, bMask20Bits, 1);

		// 2. Set RF mode = standby mode
		PHY_SetRFReg(priv, eRFPath, 0x00, 0x70000, 0x01);

		tmpu4Byte[eRFPath] = PHY_QueryRFReg(priv, eRFPath, 0x28, bMask20Bits, 1);
		PHY_SetRFReg(priv, eRFPath, 0x28, 0x700, 0x07);

		//4. Set LC calibration begin
		PHY_SetRFReg(priv, eRFPath, 0x18, 0x08000, 0x01);
	}

#if (defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)) && defined(CONFIG_RTL_92D_SUPPORT)
	REG32(BSP_WDTCNR) |=  1 << 23;
#elif defined(CONFIG_RTL_8198B) && defined(CONFIG_RTL_92D_SUPPORT)
	REG32(BSP_WDTCNTRR) |= BSP_WDT_KICK;
#endif

	for(eRFPath = RF92CD_PATH_A; eRFPath < curMaxRFPath; eRFPath++) {
		while(!(PHY_QueryRFReg(priv, eRFPath, 0x2A, BIT(11), 1) &&
				timecount <= timeout)){
			DEBUG_INFO("PHY_LCK delay for %d ms=2\n", timecount);
			delay_ms(50);
			timecount+=50;
		}
	}

	for(eRFPath = RF92CD_PATH_A; eRFPath < curMaxRFPath; eRFPath++) {
		PHY_SetRFReg(priv, eRFPath, 0x28, bMask20Bits, tmpu4Byte[eRFPath]);
		priv->pshare->RegRF28[eRFPath] = tmpu4Byte[eRFPath];
		PHY_SetRFReg(priv, eRFPath, 0x00, bMask20Bits, RF_mode[eRFPath]);
	}

	// Restore original situation
	if ((tmpReg & 0x70) != 0)	// Deal with contisuous TX case
		RTL_W8(0xd03, tmpReg);
	else 						// Deal with Packet TX case
		RTL_W8(TXPAUSE, 0x00);

	PHY_SetBBReg(priv, rFPGA0_AnalogParameter4, 0xF00000, 0x0);

}


#endif //LCK_SW


#ifdef DPK_92D


void rtl8192cd_DPK_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (priv->pshare->pwr_trk_ongoing){
		DPK_DEBUG("==>_PHY_DigitalPredistortion() TxPowerTrackingInProgress() delay 100ms\n"); 	
		mod_timer(&priv->pshare->DPKTimer, jiffies + RTL_MILISECONDS_TO_JIFFIES(100));
	}else{
		PHY_DPCalibrate(priv);
	}
}

void _PHY_DPK_polling(struct rtl8192cd_priv *priv)
{
	unsigned int	delaycount = 0, delaybound = 30, delay = 800;
	unsigned int	u4tmp;

	delaycount = 0;

	do{
		delay_us(delay);
		
		u4tmp = PHY_QueryBBReg(priv, 0xdf4, bMaskDWord);		
		//RTPRINT(FINIT, INIT_IQK, ("0xdf4 = 0x%x, delay %d us\n", u4tmp, delaycount*delay+800));			
		delaycount++;		
		delay = 100;
		u4tmp = (u4tmp & BIT(26)) >> 26;
	}while(u4tmp == 0x01 && delaycount < delaybound);

}

// if AP curve check fail return FALSE
int _PHY_DPK_AP_curve_check(struct rtl8192cd_priv *priv, unsigned int *PA_power, unsigned int RegiesterNum)
{
	unsigned int 	PA_power_temp[DP_PA_MODEL_NUM], i = 0, index = 5, 
			base = 532, ref1, ref2;
	int	power_I, power_Q;

	//store I, Q 

	for(i = 0; i < DP_PA_MODEL_NUM; i++){
		power_I = (PA_power[i] >> 8);
		if(power_I & BIT(7))
			power_I |= bMaskH3Bytes;

		power_Q = PA_power[i] & bMaskByte0;
		if(power_Q & BIT(7))
			power_Q |= bMaskH3Bytes;

		PA_power_temp[i] = power_I*power_I+power_Q*power_Q;
	}

	ref1 = PA_power_temp[0];
	for(i = 0; i < index; i++)
		ref1 = (ref1 > PA_power_temp[i])?ref1:PA_power_temp[i];

	ref2 = PA_power_temp[index];
	for(i = index; i < index*2; i++)
		ref2 = (ref2 > PA_power_temp[i])?ref2:PA_power_temp[i];
		
	DPK_DEBUG("==>_PHY_DPK_AP_cureve_check ref1 =  0x%x ref2 =  0x%x\n", ref1, ref2);

	if(ref1 == 0)
		return FALSE;

	return	((ref2 << 9)/ref1) < base;
}

// if DPK fail return FALSE
int _PHY_DPK_check(struct rtl8192cd_priv *priv, unsigned int *PA_power, unsigned int RegiesterNum)
{
	unsigned int base = 407, PA_power_temp[2], i = 0;
	int	power_I, power_Q;

	while(i ==0 || i == (RegiesterNum-1))
	{
		power_I = (PA_power[i] >> 8);
		if(power_I & BIT(7))
			power_I |= bMaskH3Bytes;

		power_Q = PA_power[i] & bMaskByte0;
		if(power_Q & BIT(7))
			power_Q |= bMaskH3Bytes;
		
		PA_power_temp[i==0?0:1] = power_I*power_I+ power_Q*power_Q;

		DPK_DEBUG("==>_PHY_DPK_check pa_power_temp[%d] 0x%x\n", i, PA_power_temp[i==0?0:1]);								
		
		if(i == 0)
			i = RegiesterNum -1;
		else if (i == RegiesterNum -1)
			break;
	}	

	//normalization
	if(PA_power_temp[0] == 0)
		return TRUE;
	else
		return (((PA_power_temp[1] << 9) /PA_power_temp[0]) <= base);

}

int _PHY_Find_Tx_Power_Index(struct rtl8192cd_priv *priv, unsigned int *PA_power, unsigned char path, unsigned char bPlus3db, char bDecreaseTxIndex, unsigned char *tx_index_out)
{
	unsigned char	i,  tx_index = bDecreaseTxIndex?0x0f:0x15;
	unsigned int	tmpReg[11], tmpBase, RegNum = 11, base = /*323*/256;
	unsigned int	PA_power_normal[11];
//	unsigned int	check_base =bPlus3db?(400-base):(323-base);
	unsigned int	check_base =bPlus3db?400:323;

	int	power_I, power_Q;
	int	index = -1;	

	DPK_DEBUG("==>tx_index minus %d bplus3db %d\n", base, bPlus3db);

	_PHY_DPK_polling(priv);

	if(path == RF92CD_PATH_A)	
	{
		PHY_SetBBReg(priv, 0xb00, bMaskDWord, 0x01017018);
		tmpReg[0] = PHY_QueryBBReg(priv, 0xbdc, bMaskDWord);		
		tmpReg[1] = PHY_QueryBBReg(priv, 0xbe8, bMaskDWord);

		PHY_SetBBReg(priv, 0xb00, bMaskDWord, 0x01017019);
		tmpReg[2] = PHY_QueryBBReg(priv, 0xbdc, bMaskDWord);		
		tmpReg[3] = PHY_QueryBBReg(priv, 0xbe0, bMaskDWord);		
		tmpReg[4] = PHY_QueryBBReg(priv, 0xbe8, bMaskDWord);

		PHY_SetBBReg(priv, 0xb00, bMaskDWord, 0x0101701a);
		tmpReg[5] = PHY_QueryBBReg(priv, 0xbe0, bMaskDWord);		

		PHY_SetBBReg(priv, 0xb00, bMaskDWord, 0x0101701b);
		tmpReg[6] = PHY_QueryBBReg(priv, 0xbdc, bMaskDWord);		
		tmpReg[7] = PHY_QueryBBReg(priv, 0xbe8, bMaskDWord);

		PHY_SetBBReg(priv, 0xb00, bMaskDWord, 0x0101701c);
		tmpReg[8] = PHY_QueryBBReg(priv, 0xbe8, bMaskDWord);

		PHY_SetBBReg(priv, 0xb00, bMaskDWord, 0x0101701e);
		tmpReg[9] = PHY_QueryBBReg(priv, 0xbdc, bMaskDWord);		

		PHY_SetBBReg(priv, 0xb00, bMaskDWord, 0x0101701f);
		tmpReg[10] = PHY_QueryBBReg(priv, 0xbe8, bMaskDWord);

		//RTPRINT(FINIT, INIT_IQK, ("==>_PHY_Find_Tx_Power_Index path A\n")); 						
		
	}
	else if(path == RF92CD_PATH_B)
	{
		PHY_SetBBReg(priv, 0xb70, bMaskDWord, 0x01017018);
		tmpReg[0] = PHY_QueryBBReg(priv, 0xbec, bMaskDWord);		
		tmpReg[1] = PHY_QueryBBReg(priv, 0xbf8, bMaskDWord);

		PHY_SetBBReg(priv, 0xb70, bMaskDWord, 0x01017019);
		tmpReg[2] = PHY_QueryBBReg(priv, 0xbec, bMaskDWord);		
		tmpReg[3] = PHY_QueryBBReg(priv, 0xbf0, bMaskDWord);		
		tmpReg[4] = PHY_QueryBBReg(priv, 0xbf8, bMaskDWord);

		PHY_SetBBReg(priv, 0xb70, bMaskDWord, 0x0101701a);
		tmpReg[5] = PHY_QueryBBReg(priv, 0xbf4, bMaskDWord);		

		PHY_SetBBReg(priv, 0xb70, bMaskDWord, 0x0101701b);
		tmpReg[6] = PHY_QueryBBReg(priv, 0xbec, bMaskDWord);		
		tmpReg[7] = PHY_QueryBBReg(priv, 0xbf8, bMaskDWord);

		PHY_SetBBReg(priv, 0xb70, bMaskDWord, 0x0101701c);
		tmpReg[8] = PHY_QueryBBReg(priv, 0xbf8, bMaskDWord);

		PHY_SetBBReg(priv, 0xb70, bMaskDWord, 0x0101701e);
		tmpReg[9] = PHY_QueryBBReg(priv, 0xbec, bMaskDWord);		

		PHY_SetBBReg(priv, 0xb70, bMaskDWord, 0x0101701f);
		tmpReg[10] = PHY_QueryBBReg(priv, 0xbf8, bMaskDWord);

		//RTPRINT(FINIT, INIT_IQK, ("==>_PHY_Find_Tx_Power_Index path B\n")); 						
		
	}	
	
	for(i = 0; i < RegNum; i++)
	{
		power_I = (tmpReg[i] >> 8);
		if(power_I & BIT(7))
			power_I |= bMaskH3Bytes;

		power_Q = tmpReg[i] & bMaskByte0;
		if(power_Q & BIT(7))
			power_Q |= bMaskH3Bytes;

		PA_power[i] = power_I*power_I+ power_Q*power_Q;
	}	

	//normalization
	tmpBase = PA_power[0];
	//RTPRINT(FINIT, INIT_IQK, ("==>PA_power START normalized\n")); 						

	if(tmpBase == 0)
		DPK_DEBUG("==>PA_power[0] is ZERO !!!!!\n");

	for(i = 0; i < RegNum; i++)
	{
		if(tmpBase != 0)			
			PA_power[i] = (PA_power[i] << 9) /tmpBase;
		else
			PA_power[i] = (PA_power[i] << 9) ;		
		PA_power_normal[i] = PA_power[i];
		PA_power[i] = (PA_power[i] > base)?(PA_power[i] - base):(base - PA_power[i]);
		DPK_DEBUG("==>PA_power normalized index %d value 0x%x\n", i, PA_power[i]);							
	}

	//choose min for TX index to do DPK
	base = bMaskDWord;
	for(i = 0; i < RegNum; i++)
	{
		if(PA_power[i] < base)
		{
			base = PA_power[i];
			index = i;		
		}
	}

	if(index == -1)
	{
		tx_index = 0x1c;
		index = 0x1f - tx_index;
	}
	else
	{
		tx_index += index;
	}	

	DPK_DEBUG("==>tx_index result 0x%x  PA_power[%d] = 0x%x\n", tx_index, index, PA_power[index]);

	*tx_index_out = tx_index;

	//Check pattern reliability
	if(((PA_power_normal[index] > check_base) && (tx_index == 0x1f)) ||
		((PA_power_normal[10] > base) && (!bPlus3db)) ||
		((tx_index < 0x1a) && (!bPlus3db)) ||
		((tx_index < 0x13) && (bDecreaseTxIndex))
		)
		return FALSE;
	else
		return TRUE;
//	return tx_index;

}

unsigned char _PHY_Find_Rx_Power_Index(struct rtl8192cd_priv *priv, unsigned char tx_index, unsigned char rx_index, unsigned char	path, char *bDecreaseTxIndex)
{
//	u1Byte	rx_index = 0x04;
	unsigned int	tmpReg;
	int	power_I, power_Q, tmp;
	unsigned char	bPlus = FALSE, bMinus = FALSE;
	unsigned short	offset[2][2] = {{	//path, offset
			0xb28,	0xbe8},{
			0xb98,	0xbf8}};
	
	while (TRUE){
		tmpReg = 0x52000 | tx_index | (rx_index << 5);
		PHY_SetRFReg(priv, path, RF_AC, bMask20Bits, tmpReg);
		//RTPRINT(FINIT, INIT_IQK, ("==>RF 0ffset 0 = 0x%x readback = 0x%x\n", tmpReg, 
			//PHY_QueryRFReg(pAdapter, path, RF_AC, bRFRegOffsetMask)));	
	
		//----send one shot signal----//
		PHY_SetBBReg(priv, offset[path][0], bMaskDWord, 0x80080000);	//0xb28, 0xb98
		PHY_SetBBReg(priv, offset[path][0], bMaskDWord, 0x00080000);
				
		_PHY_DPK_polling(priv);
		
		tmpReg = PHY_QueryBBReg(priv, offset[path][1], bMaskDWord);
		power_I = ((tmpReg & bMaskByte1) >> 8);
		power_Q = tmpReg & bMaskByte0;

		if(power_I & BIT(7))
		{
			power_I |= bMaskH3Bytes; 
			power_I = 0-power_I;	//absolute value
		}

		if(power_Q & BIT(7))
		{
			power_Q |= bMaskH3Bytes;
			power_Q = 0-power_Q;
		}
		//RTPRINT(FINIT, INIT_IQK, ("==>rx_index 0x%x I = 0x%x Q = 0x%x offset 0xbe8 = 0x%x\n", rx_index, power_I, power_Q, tmpReg));	

		tmp = (power_I > power_Q)? power_I:power_Q;

#if 0
		if((rx_index == 0 && tmp > 0x6f)||(rx_index == 31 && tmp < 0x50))
			break;
#endif

		if((tmp<= 0x6f && tmp >= 0x50) )
		{
			break;
		}
		else if(tmp < 0x50)
		{
			bPlus = TRUE;
			if(bMinus)
			{
				rx_index++;
				break;
			}
//			rx_index++;
			rx_index += 2;
		}
		else if (tmp > 0x6f)
		{
			bMinus = TRUE;
			if(bPlus)
			{
				rx_index--;
				break;
			}
//			rx_index--;
			rx_index -= 2;
		}

		if(rx_index == 0 || rx_index == 31)
			break;
		
	}
	if(rx_index == 0 && tmp > 0x6f)
		*bDecreaseTxIndex = TRUE;
	
	DPK_DEBUG("==>rx_index FINAL 0x%x I = 0x%x Q = 0x%x\n", rx_index, power_I, power_Q);	

	return	rx_index;

}

void PHY_DPCalibrate(struct rtl8192cd_priv *priv)
{
	char	is2T = ((priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY) ?1 :0);

	unsigned int	tmpReg, value32, checkbit;					
	unsigned int	AFE_backup[IQK_ADDA_REG_NUM];
	unsigned int	AFE_REG[IQK_ADDA_REG_NUM] = {	
						rFPGA0_XCD_SwitchControl, 0xe6c, 0xe70, 0xe74, 0xe78, 
						0xe7c, 0xe80, 0xe84, 0xe88, 0xe8c, 
						0xed0, 0xed4, 0xed8, 0xedc, 0xee0,
						0xeec};

	unsigned int	BB_backup[DP_BB_REG_NUM];	
	unsigned int	BB_REG[DP_BB_REG_NUM] = {
						rOFDM0_TRxPathEnable, rFPGA0_RFMOD, 
						rOFDM0_TRMuxPar, rFPGA0_XCD_RFInterfaceSW,
						rFPGA0_AnalogParameter4, rFPGA0_XAB_RFInterfaceSW, 
						rTxAGC_B_CCK11_A_CCK2_11
						};

	unsigned int	BB_backup_A[DP_BB_REG_NUM_A];	
	unsigned int	BB_REG_A[DP_BB_REG_NUM_A] = {
						rFPGA0_XA_RFInterfaceOE,	rTxAGC_A_Rate18_06, 
						rTxAGC_A_Rate54_24,		rTxAGC_A_CCK1_Mcs32, 
						0xe0c, 					rTxAGC_A_Mcs03_Mcs00, 
						rTxAGC_A_Mcs07_Mcs04, 	rTxAGC_A_Mcs11_Mcs08, 
						rTxAGC_A_Mcs15_Mcs12,	rOFDM0_XAAGCCore1/*,
						rBndA*/
						};

	
	unsigned int	BB_backup_B[DP_BB_REG_NUM_B];	
	unsigned int	BB_REG_B[DP_BB_REG_NUM_B] = {
						rFPGA0_XB_RFInterfaceOE,	rTxAGC_B_Rate18_06, 
						rTxAGC_B_Rate54_24,		rTxAGC_B_CCK1_55_Mcs32, 
						rTxAGC_B_Mcs03_Mcs00, 	rTxAGC_B_Mcs07_Mcs04, 
						rTxAGC_B_Mcs11_Mcs08,	rTxAGC_B_Mcs15_Mcs12,
						rOFDM0_XBAGCCore1 /*, rBndB*/
						};
	
	unsigned int	BB_settings[DP_BB_REG_NUM_settings] = {
						0x00a05430, 0x02040000, 0x000800e4, 0x22208000, 
						0xccf000c0/*,	0x07600760*/};
						
	unsigned int	BB_REG_loop[DP_PATH_NUM][DP_BB_REG_NUM_loop] = {
						{0xb00, 0xb04, 0xb28, 0xb68, 
						0xb08, 0xb0c, 0xb10, 0xb14, 
						0xb18, 0xb1c, 0xb20, 0xb24,   
						0xe28, 0xb00, 0xb04, 0xb08, 
						0xb0c, 0xb10, 0xb14, 0xb18,
						0xb1c, 0xb20, 0xb24, 0xb28,
						0xb2c, rBndA, 0xb34, 0xb38, 
						0xb3c, 0xe28},
						{0xb70, 0xb74, 0xb98, 0xb6C, 
						0xb78, 0xb7c, 0xb80, 0xb84, 
						0xb88, 0xb8c, 0xb90, 0xb94,   
						0xe28, 0xb60, 0xb64, 0xb68, 
						0xb6c, 0xb70, 0xb74, 0xb78,
						0xb7c, 0xb80, 0xb84, 0xb88,
						0xb8c, 0xb90, 0xb94, 0xb98, 
						0xb9c, 0xe28} 						
						};			

	unsigned int	BB_settings_loop[DP_BB_REG_NUM_loop] = {
						0x01017e18, 0xf76d9f84, 0x00080000, 0x11880000, 
						0x41382e21,	0x5b554f48, 0x6f6b6661, 0x817d7874,
						0x908c8884, 0x9d9a9793, 0xaaa7a4a1, 0xb6b3b0ad,
						0x40000000, 0x7d327c18, 0x7e057db3, 0x7e5f7e37,
						0x7e967e7c, 0x7ebe7eac, 0x7ed77ecc, 0x7eee7ee4,
						0x7f017ef9, 0x7f0e7f07, 0x7f1c7f15, 0x7f267f20,
						0x7f2f7f2a, 0x7f377f34, 0x7f3e7f3b, 0x7f457f42,
						0x7f4b7f48, 0x00000000 
						};

	unsigned int	BB_settings_loop_3db[DP_BB_REG_NUM_loop] = {
						0x01017e18, 0xf76d9f84, 0x00080000, 0x11880000, 
						0x5b4e402e,	0x7f776f65, 0x9c968f88, 0xb5afa8a3,
						0xcac4bfb9, 0xdcd8d4ce, 0xeeeae6e2, 0xfffbf7f2,
						0x40000000, 0x7dfe7d32, 0x7e967e59, 0x7ed77eba,
						0x7efd7eeb, 0x7f1a7f0e, 0x7f2d7f25, 0x7f3c7f36,
						0x7f4a7f44, 0x7f547f4e, 0x7f5d7f58, 0x7f657f60,
						0x7f6a7f68, 0x7f717f6e, 0x7f767f73, 0x7f7b7f78,
						0x7f7f7f7d, 0x00000000 
						};	
	
	unsigned int	BB_settings_loop_tx[DP_BB_REG_NUM_loop] = {	
						0x01017e18, 0xf76d9f84, 0x00080000, 0x11880000, 
						0x21212121, 0x21212121, 0x21212121, 0x21212121,
						0x21212121, 0x21212121, 0x21212121, 0x21212121,
						0x40000000,	0x7c187c18, 0x7c187c18, 0x7c187c18, 
						0x7c187c18, 	0x7c187c18,	0x7c187c18, 0x7c187c18, 
						0x7c187c18,	0x7c187c18, 0x7c187c18, 0x7c187c18, 
						0x7c187c18,	0x7c187c18, 0x7c187c18, 0x7c187c18, 
						0x7c187c18, 0x00000000 
						};	

	unsigned int	BB_settings_loop_tx_3db[DP_BB_REG_NUM_loop] = { 
						0x01017e18, 0xf76d9f84, 0x00080000, 0x11880000, 
						0x2e2e2e2e, 0x2e2e2e2e, 0x2e2e2e2e, 0x2e2e2e2e,
						0x2e2e2e2e, 0x2e2e2e2e, 0x2e2e2e2e, 0x2e2e2e2e,
						0x40000000, 0x7d327d32, 0x7d327d32, 0x7d327d32, 
						0x7d327d32, 	0x7d327d32, 0x7d327d32, 0x7d327d32, 
						0x7d327d32, 0x7d327d32, 0x7d327d32, 0x7d327d32, 
						0x7d327d32, 0x7d327d32, 0x7d327d32, 0x7d327d32, 
						0x7d327d32, 0x00000000 
						};


					//for find 2dB loss point
	unsigned int	BB_settings_loop_tx_2[DP_BB_REG_NUM_loop] = { 
						0x01017e18, 0xf76d9f84, 0x00080000, 0x11880000, 
						0x41382e21, 0x5b554f48, 0x6f6b6661, 0x817d7874,
						0x908c8884, 0x9d9a9793, 0xaaa7a4a1, 0xb6b3b0ad,
						0x40000000, 0x7d327c18, 0x7e057db3, 0x7e5f7e37, 
						0x7e967e7c, 0x7ebe7eac, 0x7ed77ecc, 0x7eee7ee4, 
						0x7f017ef9, 0x7f0e7f07, 0x7f1c7f15, 0x7f267f20, 
						0x7f2f7f2a, 0x7f377f34, 0x7f3e7f3b, 0x7f457f42, 
						0x7f4b7f48, 0x00000000 
						};

					//for find 2dB loss point
	unsigned int	BB_settings_loop_tx_2_3db[DP_BB_REG_NUM_loop] = { 
						0x01017e18, 0xf76d9f84, 0x00080000, 0x11880000, 
						0x5b4e402e, 0x7f776f65, 0x9c968f88, 0xb5afa8a3,
						0xcac4bfb9, 0xdcd8d4ce, 0xeeeae6e2, 0xfffbf7f2,
						0x40000000, 0x7dfe7d32, 0x7e967e59, 0x7ed77eba, 
						0x7efd7eeb, 0x7f1a7f0e, 0x7f2d7f25, 0x7f3c7f36, 
						0x7f4a7f44, 0x7f547f4e, 0x7f5d7f58, 0x7f657f60, 
						0x7f6a7f68, 0x7f717f6e, 0x7f767f73, 0x7f7b7f78, 
						0x7f7f7f7d, 0x00000000 
						};					


	unsigned int	BB_settings_loop_rx[DP_BB_REG_NUM_loop_rx] = {
						0x01017e18, 0xf76d9f84, 0x00080000, 0x11880000, 
						0x21212121,	0x40000000, 0x7c187c18, 0x00000000
						};	

	unsigned int	BB_settings_loop_rx_3db[DP_BB_REG_NUM_loop_rx] = {
						0x01017e18, 0xf76d9f84, 0x00080000, 0x11880000, 
						0x2e2e2e2e,	0x40000000, 0x7d327d32, 0x00000000
						};	

	unsigned int	BB_settings_loop_pa[DP_BB_REG_NUM_loop_pa] = {
						0x02096eb8, 0xf76d9f84, 0x00044499, 0x02880140  
						};	

	unsigned int	BB_settings_loop_dp[DP_BB_REG_NUM_loop_pa] = {
						0x01017098, 0x776d9f84, 0x00000000, 0x08080000  
						};	

	unsigned int	*BB_settings_temp;

	unsigned char	Sram_db_settings[DP_SRAM_NUM_db] = {
						0xfe,	0xf0,	0xe3,	0xd6,	0xca,	
						0xbf,	0xb4,	0xaa,	0xa0,	0x97,
						0x8f,	0x87,	0x7f,	0x78,	0x71,
						0x6b,	0x65,	0x5f,	0x5a,	0x55,
						0x50,	0x4c,	0x47,	0x43,	0x40,	
						0x3c,	0x39,	0x35,	0x32,	0x2f,
						0x2d,	0x2a,	0x28,	0x26,	0x23,
						0x21,	0x20,	0x1e,	0x1c,	0x1a,
						0x19,	0x18,	0x16,	0x16,	0x14,	
						0x13,	0x12,	0x11,	0x10,	0x0f,
						0x0e,	0x0d,	0x0c,	0x0c,	0x0b,	
						0x0a,	0x0a,	0x09,	0x09,	0x08,	
						0x08,	0x07,	0x07,	0x06,	0x06,	
						0x06,	0x05,	0x05,	0x05,	0x04,	
						0x04,	0x04,	0x04,	0x03,	0x03,	
						0x03,	0x03,	0x03,	0x02,	0x02,	
						0x02,	0x02,	0x02,	0x02,	0x02,	
						0x01
					};

	//unsigned int	pwsf[DP_SRAM_NUM];

	unsigned int	offset[2][DP_OFFSET_NUM] = {{		//path, offset
						0xe34,	0xb28, 	0xb00,	0xbdc,	0xbc0,
						0xbe8,	rOFDM0_XATxIQImbalance,	rBndA,	
						0xb68},{
						0xe54,	0xb98,	0xb70,	0xbec,	0xbc4,
						0xbf8,	rOFDM0_XBTxIQImbalance,	rBndB,
						0xb6c}};
						
	unsigned char	OFDM_min_index = 6, OFDM_min_index_internalPA = 3;
	unsigned char	OFDM_index[2];
	unsigned char	retrycount = 0, retrybound = 1;

	unsigned int	RF_backup[DP_PATH_NUM][DP_RF_REG_NUM];
	unsigned int	RF_REG[DP_RF_REG_NUM] = {
						RF_TX_G3,	RF_TXPA_A4,	RF_RXRF_A3,	
						RF_BS_PA_APSET_G1_G4/*,	RF_BS_PA_APSET_G5_G8,
						RF_BS_PA_APSET_G9_G11*/};

	unsigned int	RF_AP_curve_select[DP_AP_CUREVE_SELECT_NUM] = {
						0x7bdef,	0x94a52,	0xa5294/*,	0xb5ad6*/	};	

	unsigned int	RF_PA_BIAS[3][DP_PA_BIAS_NUM] = {{	//40MHz / 20MHz, original
						0xe189f,	0xa189f,	0x6189f,	0x2189f	},{
						0xe087f,	0xa087f,	0x6087f,	0x2087f	},{
						0xe1874,	0xa1874,	0x61874,	0x21874}};

	unsigned int	PA_model_backup[DP_PATH_NUM][DP_PA_MODEL_NUM];

	unsigned int	PA_power[DP_PATH_NUM][DP_PA_MODEL_RUN_NUM*2];

#if DP_gain_loss == 1

	int				power_I, power_Q, coef;

	int				gain_loss_backup[DP_PATH_NUM][DP_PA_MODEL_NUM];	//I,Q

	unsigned int	gain_loss_bound[DP_GAIN_LOSS_BOUND_NUM] = {						
						63676,	60114 ,	56751 ,	53577 ,	49145,	
						47750, 	45079 ,	42557 ,	40177 ,	37929 ,
						35807 ,	33804 ,	31913,	30128  		
					};	

	int				gain_loss_coef[DP_GAIN_LOSS_BOUND_NUM+1] = { 					
						512,	527,	543, 	558, 	573, 	
						589,	609,	625, 	645, 	666, 	
						681,	701, 	722, 	742,	768
					};

//	BOOLEAN			bNegative = FALSE;
//	unsigned char	index_for_zero_db = 24, AP_curve_index = 0;
	unsigned char	GainLossIndex = 0; //0db, 0x40
	char			SramIndex = 24;
	unsigned char	index_for_zero_db = 6, AP_curve_index = 0;

#else

	unsigned char	index_for_zero_db = 6, AP_curve_index = 0;
	int				power_I, power_Q;
	unsigned int	gain_loss_bound[DP_GAIN_LOSS_BOUND_NUM] = {						
						61870,	55142,	49145,	43801,	39037,
						34792,	31008,	27636		
					};		
#endif
	
	unsigned int	MAC_backup[IQK_MAC_REG_NUM];
	unsigned int	MAC_REG[IQK_MAC_REG_NUM] = {
						0x522,	0x550, 	0x551, 	0x040};		

	unsigned int	AFE_on_off[PATH_NUM] = {
					0x04db25a4, 0x0b1b25a4};	//path A on path B path A off path B on

	unsigned char	path_num, path_bound, path = RF92CD_PATH_A, i, j, tx_index, rx_index;
	int				index, index_1, index_repeat;

	char			bInternalPA = FALSE, SkipStep5 = FALSE;
	char			bPlus3db = FALSE, bDecreaseTxIndex = FALSE, bDecreaseTxIndexWithRx = FALSE;

	
	DPK_DEBUG("==>_PHY_DigitalPredistortion() interface index %d is2T = %d\n", priv->pshare->wlandev_idx, is2T); //anchin
	
	DPK_DEBUG("_PHY_DigitalPredistortion for %s\n", (is2T ? "2T2R" : "1T1R"));

	DPK_DEBUG("==>_PHY_DigitalPredistortion() current thermal meter = 0x%x PG thermal meter = 0x%x bPlus3db %d\n", 
		priv->pshare->ThermalValue_DPKtrack, priv->pmib->dot11RFEntry.ther, bPlus3db);
	
	if ((priv->pmib->dot11RFEntry.phyBandSelect!= PHY_BAND_5G)||(GET_CHIP_VER(priv)!=VERSION_8192D))
		return;
	
	bInternalPA = priv->pshare->rf_ft_var.use_intpa92d;

	if(!is2T)
		path_num = 1;
	else 
		path_num = 2;
	
	if(!bInternalPA) {
		DPK_DEBUG("==>_PHY_DigitalPredistortion() NOT internal5G\n");	
		return;	
	}

	if(priv->pshare->pwr_trk_ongoing){
		DPK_DEBUG("==>_PHY_DigitalPredistortion() TxPowerTrackingInProgress() delay 100ms\n"); 	
		mod_timer(&priv->pshare->DPKTimer, jiffies + RTL_MILISECONDS_TO_JIFFIES(100));
		return;
	}	
	
	OFDM_index[RF92CD_PATH_A] = priv->pshare->OFDM_index[RF92CD_PATH_A];
	OFDM_index[RF92CD_PATH_B] = priv->pshare->OFDM_index[RF92CD_PATH_B];

	DPK_DEBUG("original index 0x%x \n", priv->pshare->OFDM_index[0]);		
	
	priv->pshare->bDPKworking = TRUE;
	
	//save global setting
	//save BB default value
	_PHY_SaveADDARegisters(priv, BB_REG, BB_backup, DP_BB_REG_NUM);

	//save MAC default value
	_PHY_SaveMACRegisters(priv, MAC_REG, MAC_backup);

	//save AFE default value
	_PHY_SaveADDARegisters(priv, AFE_REG, AFE_backup, IQK_ADDA_REG_NUM);

	//save path A default value
	//save path A BB default value
	_PHY_SaveADDARegisters(priv, BB_REG_A, BB_backup_A, DP_BB_REG_NUM_A);

	//save path B BB default value	
	if(is2T)
		_PHY_SaveADDARegisters(priv, BB_REG_B, BB_backup_B, DP_BB_REG_NUM_B);
	
	//save pathA/B RF default value
	for(path=0; path<path_num; path++){
		for(index=0; index<DP_RF_REG_NUM; index++)
			RF_backup[path][index] = PHY_QueryRFReg(priv, path, RF_REG[index], bMaskDWord, 1);	
	}	
	
	//BB register setting
	for(index = 0; index < DP_BB_REG_NUM_settings; index++)
		PHY_SetBBReg(priv, BB_REG[index], bMaskDWord, BB_settings[index]);

	//BB path A debug setting
	PHY_SetBBReg(priv, rFPGA1_DebugSelect, bMaskDWord, 0x00000302);

	//BB pah A register setting: fix TRSW to TX, external PA on, external LAN off
	if(!bInternalPA)
	{
		PHY_SetBBReg(priv, rFPGA0_XAB_RFInterfaceSW, bMaskDWord, 0x07600f60);			
		PHY_SetBBReg(priv, rFPGA0_XA_RFInterfaceOE, ~(BIT8|BIT9), 0x66e60a30);		
	}
	else
	{
		PHY_SetBBReg(priv, rFPGA0_XAB_RFInterfaceSW, bMaskDWord, 0x07600760);			
		PHY_SetBBReg(priv, rFPGA0_XA_RFInterfaceOE, ~(BIT8|BIT9), 0x66e60230);		
	}
	PHY_SetBBReg(priv, rBndA, 0xF00000, 0x0a);

	//BB pah B register setting: fix TRSW to TX, external PA off, external LNA off
	if(is2T)
	{
		if(!bInternalPA)
		{
			PHY_SetBBReg(priv, rFPGA0_XAB_RFInterfaceSW, bMaskDWord, 0x0f600f60);					
		PHY_SetBBReg(priv, rFPGA0_XB_RFInterfaceOE, bMaskDWord, 0x061f0130);
		}
		else
		{
			PHY_SetBBReg(priv, rFPGA0_XAB_RFInterfaceSW, bMaskDWord, 0x07600760);						
			PHY_SetBBReg(priv, rFPGA0_XB_RFInterfaceOE, bMaskDWord, 0x061f0130);
		}
		PHY_SetBBReg(priv, rBndB, 0xF00000, 0x0a);		
	}

	//MAC register setting
	_PHY_MACSettingCalibration(priv, MAC_REG, MAC_backup);

	//path A/B DPK
	//Path-A/B AFE all on
	for(path=0; path<path_num; path++)	
	{

		//if(is2T && !pHalData->InternalPA5G[path])		
			//continue;

		if(path == RF92CD_PATH_B)
		{
			//BB pah A register setting:fix TRSW to TX;external LNA off
			if(!bInternalPA)
			{
				PHY_SetBBReg(priv, rFPGA0_XAB_RFInterfaceSW, bMaskDWord, 0x0f600f60);			
				PHY_SetBBReg(priv, rFPGA0_XA_RFInterfaceOE, ~(BIT8|BIT9), 0x66e60230);		
			}
			else
			{
				PHY_SetBBReg(priv, rFPGA0_XAB_RFInterfaceSW, bMaskDWord, 0x07600760);			
				PHY_SetBBReg(priv, rFPGA0_XA_RFInterfaceOE, ~(BIT8|BIT9), 0x66e60230);		
			}
			PHY_SetBBReg(priv, rBndA, 0xF00000, 0x0a);

			//BB pah B register setting:fix TRSW to TX;external LNA off
			if(is2T)
			{
				if(!bInternalPA)
				{
					PHY_SetBBReg(priv, rFPGA0_XAB_RFInterfaceSW, bMaskDWord, 0x0f600f60);					
					PHY_SetBBReg(priv, rFPGA0_XB_RFInterfaceOE, bMaskDWord, 0x061f0930);			
				}
				else
				{
					PHY_SetBBReg(priv, rFPGA0_XAB_RFInterfaceSW, bMaskDWord, 0x07600760);						
					PHY_SetBBReg(priv, rFPGA0_XB_RFInterfaceOE, bMaskDWord, 0x061f0130);
				}
				PHY_SetBBReg(priv, rBndB, 0xF00000, 0x0a);		
			}			
		}

		AP_curve_index = 1;
		rx_index = 0x06;		
		bPlus3db = FALSE;
		bDecreaseTxIndex = FALSE;
	
		if(path == RF92CD_PATH_A)
		{
			_PHY_PathADDAOn(priv, AFE_REG, TRUE, is2T);
		}
		else
		{
			_PHY_PathADDAOn(priv, AFE_REG, FALSE, is2T);
		}

		if(path == RF92CD_PATH_B)
			PHY_SetBBReg(priv, rFPGA1_DebugSelect, bMaskDWord, 0x00000303); 		

		//path A/B RF setting
		//internal lopback off	
		if(path == RF92CD_PATH_A && !bInternalPA)
		{
			PHY_SetRFReg(priv, RF92CD_PATH_A, RF_MODE1, bMask20Bits, 0x5007f);					
			PHY_SetRFReg(priv, RF92CD_PATH_A, RF_MODE2, bMask20Bits, 0x6f1f9);								
		}		
		else if(path == RF92CD_PATH_B)
		{
			PHY_SetRFReg(priv, RF92CD_PATH_A, RF_MODE1, bMask20Bits, 0x1000f);					
			PHY_SetRFReg(priv, RF92CD_PATH_A, RF_MODE2, bMask20Bits, 0x60103);								
		}

		PHY_SetRFReg(priv, path, RF_RXRF_A3, bMask20Bits, 0xef456);

		//Path A/B to standby mode
		PHY_SetRFReg(priv, path==RF92CD_PATH_A?RF92CD_PATH_B:RF92CD_PATH_A, 
			RF_AC, bMask20Bits, 0x10000);

		//set DPK PA bias table
		index = priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20_40?0:1;		
		if(path == RF92CD_PATH_A)
		{
			for(i = 0; i < path_num; i++)
		{
				for(j = 0; j < DP_PA_BIAS_NUM; j++)
					PHY_SetRFReg(priv, i, RF_TXBIAS, bMask20Bits, RF_PA_BIAS[index][j]);	
			}
		}
		
Step1:

		DPK_DEBUG("==>AP curve select 0x%x bplus3db %d path%s!!\n", RF_AP_curve_select[AP_curve_index], bPlus3db, path==RF92CD_PATH_A?"A":"B");				
	
		//RF setting for AP curve selection
		//default AP curve = 15	
		PHY_SetRFReg(priv, path, RF_BS_PA_APSET_G1_G4, bMask20Bits, RF_AP_curve_select[AP_curve_index]);	

		//////////////////////////////////////////////////
		// step 1: find RF TX/RX index
		/////////////////////////////////////////////////
		//find RF TX index
		//=============================
		// PAGE_B for Path-A PM setting
		//=============================
		// open inner loopback @ b00[19]:od 0xb00 0x01097018
		if(bPlus3db)
			BB_settings_temp = &(BB_settings_loop_tx_3db[0]);
		else
			BB_settings_temp = &(BB_settings_loop_tx[0]);
		_PHY_SetADDARegisters(priv, BB_REG_loop[path], BB_settings_temp, DP_BB_REG_NUM_loop);

		if(bDecreaseTxIndex)
			tx_index = 0x19;
		else
			tx_index = 0x1f;
		bDecreaseTxIndexWithRx = FALSE;

		//Set Tx GAC = 0x1f, than find Rx AGC
		rx_index = _PHY_Find_Rx_Power_Index(priv, tx_index, rx_index, path, &bDecreaseTxIndexWithRx);
		if(bDecreaseTxIndexWithRx)
		{
			if(bDecreaseTxIndex)
			{
				PHY_SetBBReg(priv, offset[path][2], bMaskDWord, 0x01017098);	//0xb00, 0xb70				
				PHY_SetBBReg(priv, offset[path][8], bMaskDWord, 0x28080000);	//0xb68, 0xb6c
				
				for(i = 3; i < DP_RF_REG_NUM; i++)
					PHY_SetRFReg(priv, path, RF_REG[i], bMask20Bits, RF_backup[path][i]);

				//set original DPK bias table
				for(j = 0; j < DP_PA_BIAS_NUM; j++)
					PHY_SetRFReg(priv, path, RF_TXBIAS, bMask20Bits, RF_PA_BIAS[2][j]);
				continue;														
			}
			else
			{
				bDecreaseTxIndex = TRUE;
				goto Step1;
			}
		}

		//find 2dB loss point
		//=============================
		// PAGE_B for Path-A PM setting
		//=============================
		// open inner loopback @ b00[19]:od 0xb00 0x01097018
		if(bPlus3db)
			BB_settings_temp = &(BB_settings_loop_tx_2_3db[0]);
		else
			BB_settings_temp = &(BB_settings_loop_tx_2[0]);
		_PHY_SetADDARegisters(priv, BB_REG_loop[path], BB_settings_temp, DP_BB_REG_NUM_loop);

		//RF setting
		PHY_SetRFReg(priv, path, RF_AC, bMask20Bits, 0x52000 | tx_index | (rx_index << 5));

		//----send one shot signal----//
		PHY_SetBBReg(priv, offset[path][1], bMaskDWord, 0x80080000);	//0xb28, 0xb98
		PHY_SetBBReg(priv, offset[path][1], bMaskDWord, 0x00080000);
		
		//get power
		if(!_PHY_Find_Tx_Power_Index(priv, PA_power[path], path, bPlus3db, bDecreaseTxIndex, &tx_index))
		{
			if(/*tx_index == 0x1f &&*/ !bPlus3db)
			{
				if(bDecreaseTxIndex)
				{
					if(tx_index < 0x11)
					{
						PHY_SetBBReg(priv, offset[path][2], bMaskDWord, 0x01017098);	//0xb00, 0xb70				
						PHY_SetBBReg(priv, offset[path][8], bMaskDWord, 0x28080000);	//0xb68, 0xb6c
						
						for(i = 3; i < DP_RF_REG_NUM; i++)
							PHY_SetRFReg(priv, path, RF_REG[i], bMask20Bits, RF_backup[path][i]);

						//set original DPK bias table
						for(j = 0; j < DP_PA_BIAS_NUM; j++)
							PHY_SetRFReg(priv, path, RF_TXBIAS, bMask20Bits, RF_PA_BIAS[2][j]);

						continue;											
					}	
					else
					{
						//RTPRINT(FINIT, INIT_IQK, ("==>Check pattern reliability path%s SUCCESS tx_index = 0x1b!!!!\n", path==RF90_PATH_A?"A":"B")); 										
					}
				}
				else if(tx_index < 0x1a)
				{
					bDecreaseTxIndex = TRUE;
					goto Step1; 				
				}
				else
				{
					bPlus3db = TRUE;
					goto Step1; 				
				}
			}
			else if(tx_index == 0x1f) 
			{
				DPK_DEBUG("==>Check pattern reliability path%s FAIL!!!!\n", path==RF92CD_PATH_A?"A":"B");
				PHY_SetBBReg(priv, offset[path][2], bMaskDWord, 0x01017098);	//0xb00, 0xb70
				PHY_SetBBReg(priv, offset[path][8], bMaskDWord, 0x28080000);	//0xb68, 0xb6c
				
				for(i = 3; i < DP_RF_REG_NUM; i++)
					PHY_SetRFReg(priv, path, RF_REG[i], bMask20Bits, RF_backup[path][i]);

				//set original DPK bias table
				for(j = 0; j < DP_PA_BIAS_NUM; j++)
					PHY_SetRFReg(priv, path, RF_TXBIAS, bMask20Bits, RF_PA_BIAS[2][j]);

				continue;							
			}		

		
		}
		else
		{
			DPK_DEBUG("==>Check pattern reliability path%s SUCCESS!!!!\n", path==RF92CD_PATH_A?"A":"B"); 					
		}

		//find RF RX index
		//=============================
		// PAGE_B for Path-A PM setting
		//=============================
		// open inner loopback @ b00[19]:od 0xb00 0x01097018
		if(bPlus3db)
			BB_settings_temp = &(BB_settings_loop_rx_3db[0]);
		else
			BB_settings_temp = &(BB_settings_loop_rx[0]);

		for(i = 0; i < 4; i++)
			PHY_SetBBReg(priv, BB_REG_loop[path][i], bMaskDWord, BB_settings_temp[i]);
		for(; i < 12; i++)
			PHY_SetBBReg(priv, BB_REG_loop[path][i], bMaskDWord, BB_settings_temp[4]);
		PHY_SetBBReg(priv, BB_REG_loop[path][i], bMaskDWord, BB_settings_temp[5]);
		for(; i < 29; i++)
			PHY_SetBBReg(priv, BB_REG_loop[path][i], bMaskDWord, BB_settings_temp[6]);
		PHY_SetBBReg(priv, BB_REG_loop[path][i], bMaskDWord, BB_settings_temp[7]);
		
		rx_index = _PHY_Find_Rx_Power_Index(priv, tx_index, rx_index, path, &bDecreaseTxIndex);

		//////////////////////////////////////
		//2.measure PA model
		//////////////////////////////////////
		//=========================================
		//PAGE_B for Path-A PAS setting //=========================================
		// open inner loopback @ b00[19]:10 od 0xb00 0x01097018
		if(bPlus3db)
			BB_settings_temp = &(BB_settings_loop_3db[0]);
		else
			BB_settings_temp = &(BB_settings_loop[0]);
		_PHY_SetADDARegisters(priv, BB_REG_loop[path], BB_settings_temp, DP_BB_REG_NUM_loop);

		//LNA VDD to gnd
		PHY_SetRFReg(priv,path, RF_AC, bMask20Bits, 0x52000 | tx_index | (rx_index << 5));
			
		//----send one shot signal----//
		// Path A
		PHY_SetBBReg(priv, offset[path][1], bMaskDWord, 0x80080000);	//0xb28, 0xb98
		PHY_SetBBReg(priv, offset[path][1], bMaskDWord, 0x00080000);
		
		PHY_SetRFReg(priv, RF92CD_PATH_A, RF_T_METER, BIT17|BIT16, 0x03);
	
		_PHY_DPK_polling(priv);

		priv->pshare->ThermalValue_DPKstore = (unsigned char)PHY_QueryRFReg(priv, RF92CD_PATH_A, RF_T_METER, 0xf800, 1);	//0x42: RF Reg[15:11] 92D
		
		// read PA model and save to PA_model_A[32]
		for(i = 0; i < DP_PA_MODEL_RUN_NUM; i++)
		{
			PHY_SetBBReg(priv, offset[path][2], bMaskDWord, 0x01017018+i);	//0xb00, 0xb70
			for(index = 0; index < DP_PA_MODEL_PER_RUN_NUM; index++)
			{
				PA_model_backup[path][i*4+index] = PHY_QueryBBReg(priv, offset[path][3]+index*4, bMaskDWord);	//0xbdc, 0xbec
				DPK_DEBUG("==>PA_model_backup index %d value 0x%x()\n", i*4+index, PA_model_backup[path][i*4+index]);			
			}
		}

#if 0
		//find appropriate AP curve
		if(AP_curve_index != (DP_AP_CUREVE_SELECT_NUM-1))
		{
			if(!_PHY_DPK_AP_curve_check(priv, PA_model_backup[path], DP_PA_MODEL_NUM))
			{
				DPK_DEBUG("==>find appropriate AP curve 0x%x path%s FAIL!!!!\n", RF_AP_curve_select[AP_curve_index], path==RF92CD_PATH_A?"A":"B");			
				AP_curve_index++;
				if(AP_curve_index < DP_AP_CUREVE_SELECT_NUM)
					goto Step1; 		
			}
			else
			{
				DPK_DEBUG("==>find appropriate AP curve path%s SUCCESS!!!!\n", path==RF92CD_PATH_A?"A":"B"); 					
			}
		}
#endif

		//check PA model		
		if(!_PHY_DPK_check(priv, PA_model_backup[path], DP_PA_MODEL_NUM))
		{
			PHY_SetBBReg(priv, offset[path][2], bMaskDWord, 0x01017098);	// add in 2011-06-02
			PHY_SetBBReg(priv, offset[path][8], bMaskDWord, 0x28080000);	//0xb68, 0xb6c

			for(i = 3; i < DP_RF_REG_NUM; i++)
				PHY_SetRFReg(priv, path, RF_REG[i], bMaskDWord, RF_backup[path][i]);
			DPK_DEBUG("==>PA model path%s FAIL!!!!\n", path==RF92CD_PATH_A?"A":"B"); 	
			priv->pshare->bDPKdone[path] = FALSE;							
			//set original DPK bias table
			for(j = 0; j < DP_PA_BIAS_NUM; j++)
				PHY_SetRFReg(priv, path, RF_TXBIAS, bMask20Bits, RF_PA_BIAS[2][j]);
												
			continue;
		}
		else
		{
			DPK_DEBUG("==>PA model path%s SUCCESS!!!!\n", path==RF92CD_PATH_A?"A":"B");			
//			priv->pshare->bDPKdone[path] = TRUE;	
//			priv->pshare->bDPKstore = TRUE; 		
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////
		// step 3: fill PA model to DP Calibration
		/////////////////////////////////////////////////////////////////////////////////////////////////////
		//fill BB TX index for the DPK reference 
		DPK_DEBUG("==>fill PA model to DP Calibration\n"); 		

		if(path == RF92CD_PATH_A){
			for(index = 0; index < DP_PA_MODEL_RUN_NUM; index++){
				if(index != 3){
					PHY_SetBBReg(priv, 0xe00+index*4, bMaskDWord, 0x3c3c3c3c);	
				} else {
					PHY_SetBBReg(priv, 0xe00+index*4, bMaskDWord, 0x03903c3c);						
				}
			}
			PHY_SetBBReg(priv, 0x86c, bMaskDWord, 0x3c3c3c3c);	
		}else if (path == RF92CD_PATH_B){
			for(index = 0; index < 4; index++) {
				PHY_SetBBReg(priv, 0x830+index*4, bMaskDWord, 0x3c3c3c3c);			
			}
			for(index = 0; index < 2; index++) {
				PHY_SetBBReg(priv, 0x848+index*4, bMaskDWord, 0x3c3c3c3c);	
			}
			for(index = 0; index < 2; index++) {
				PHY_SetBBReg(priv, 0x868+index*4, bMaskDWord, 0x3c3c3c3c);										
			}
		}		

		// SRAM boundary setting
		PHY_SetBBReg(priv, 0xe28, bMaskDWord, 0x40000000);	

		if(priv->pshare->phw->bNewTxGainTable)
			PHY_SetBBReg(priv, offset[path][4], bMaskDWord, 0x0008421f);	//0xbc0, 0xbc4	
		else	
			PHY_SetBBReg(priv, offset[path][4], bMaskDWord, 0x0009ce7f);	//0xbc0, 0xbc4	

		PHY_SetBBReg(priv, 0xe28, bMaskDWord, 0x00000000);	

		_PHY_SetADDARegisters(priv, BB_REG_loop[path], BB_settings_loop_pa, DP_BB_REG_NUM_loop_pa);
		
		// fill PA model to page B1 registers
		PHY_SetBBReg(priv, 0xe28, bMaskDWord, 0x40000000);	
		for(index = 0; index < (DP_PA_MODEL_NUM/2); index++){	//path A = 0xb00, path B = 0xb60
			PHY_SetBBReg(priv, 0xb00+index*4+path*0x60, bMaskDWord, 
			(PA_model_backup[path][index*2+1] << 16) | PA_model_backup[path][index*2]); 
		}
		PHY_SetBBReg(priv, 0xe28, bMaskDWord, 0x00000000);	

		//one shot	
		PHY_SetBBReg(priv, offset[path][1], bMaskDWord, 0x80044499);	//0xb28, 0xb98
		PHY_SetBBReg(priv, offset[path][1], bMaskDWord, 0x00044499);

		_PHY_DPK_polling(priv);

#if 1
		//////////////////////////////////////////////////////
		// step 4: calculate gain loss caused by DP
		//////////////////////////////////////////////////////
		PHY_SetBBReg(priv, offset[path][2], bMaskDWord, 0x0029701f);	//0xb00, 0xb70
		tmpReg = PHY_QueryBBReg(priv, offset[path][5], bMaskDWord); 	//0xbe8, 0xbf8

		power_I = (tmpReg >> 16);
		if(power_I & BIT(15))
			power_I |= bMaskLWord; ////////ZZZZZZZZZZZZZZZZZZ
		
		power_Q = tmpReg & bMaskLWord;
		if(power_Q & BIT(15))
			power_Q |= bMaskHWord;

		DPK_DEBUG("0x%x =  0x%x power_I = 0x%x power_Q = 0x%x\n", offset[path][5], tmpReg, power_I, power_Q);

		tmpReg = power_I*power_I + power_Q*power_Q;

		DPK_DEBUG("gain loss =	0x%x \n", tmpReg); 					

		if(tmpReg < 26090)
		{
			PHY_SetBBReg(priv, offset[path][2], bMaskDWord, 0x01017098);	//0xb00, 0xb70						
			PHY_SetBBReg(priv, offset[path][8], bMaskDWord, 0x28080000);	//0xb68, 0xb6c

			for(i = 3; i < DP_RF_REG_NUM; i++)
				PHY_SetRFReg(priv, path, RF_REG[i], bMask20Bits, RF_backup[path][i]);
			priv->pshare->bDPKdone[path] = FALSE;							
			//set original DPK bias table
			for(j = 0; j < DP_PA_BIAS_NUM; j++)
				PHY_SetRFReg(priv, path, RF_TXBIAS, bMask20Bits, RF_PA_BIAS[2][j]);					
			
			continue;		
		}
		else
		{
			priv->pshare->bDPKdone[path] = TRUE;	
			priv->pshare->bDPKstore = TRUE; 		
		}

		for(i = 0; i < DP_GAIN_LOSS_BOUND_NUM; i++)
		{
#if DP_gain_loss == 1						
			if(tmpReg > gain_loss_bound[i]/* || i == (DP_GAIN_LOSS_BOUND_NUM -1)*/) 	
#else				
			if(tmpReg > gain_loss_bound[i] || i == (DP_GAIN_LOSS_BOUND_NUM -1))
#endif				
			{
#if DP_gain_loss == 0
				if(i == 0)
					break;
	
				index = OFDM_index[path] > i?OFDM_index[path]-i:0;
				if(index < OFDM_min_index_internalPA)
					index = OFDM_min_index_internalPA;
				PHY_SetBBReg(priv, offset[path][6], bMaskDWord, OFDMSwingTable[index]); //0xc80, 0xc88							
				DPK_DEBUG("original index 0x%x gain_loss minus index 0x%x\n", priv->pshare->OFDM_index[0], i); 					
#endif
				break;
			}
		}
	
#if DP_gain_loss == 1
	
		DPK_DEBUG("gain_loss Compensated coefficient %d\n", gain_loss_coef[i]);					
		coef = gain_loss_coef[i];
		GainLossIndex = i;
		priv->pshare->OFDM_min_index_internalPA_DPK[path] = GainLossIndex == 0?0:(GainLossIndex/2+GainLossIndex%2);		
		
		//read DP LUT value from register
		for(i = 0; i < DP_PA_MODEL_RUN_NUM; i++)
		{
			PHY_SetBBReg(priv, offset[path][2], bMaskDWord, 0x00297018+i);	//0xb00, 0xb70
			for(index = 0; index < DP_PA_MODEL_PER_RUN_NUM; index++)
			{
				tmpReg = (i == 0 && index==0)?0x01000000:PHY_QueryBBReg(priv, offset[path][3]+index*4, bMaskDWord); //0xbdc, 0xbec		

				gain_loss_backup[1][i*4+index] = (tmpReg >> 16);			//I
				if(gain_loss_backup[1][i*4+index] & BIT(15))
					gain_loss_backup[1][i*4+index] |= bMaskHWord;			
				
				gain_loss_backup[0][i*4+index] = (tmpReg & bMaskLWord); //Q
				if(gain_loss_backup[0][i*4+index] & BIT(15))
					gain_loss_backup[0][i*4+index] |= bMaskHWord;				
				DPK_DEBUG("==>DP LUT index %d value 0x%x() I = 0x%x, Q = 0x%x\n", i*4+index, tmpReg, gain_loss_backup[1][i*4+index], gain_loss_backup[0][i*4+index]);												
	
				//gain * LUT			
				for(j = 0; j < 2; j++)
				{
	//				RTPRINT(FINIT, INIT_IQK, ("==>0DP LUT sram %s index %d value %d()\n", j == 0?"Q":"I", i*4+index, gain_loss_backup[j][i*4+index]));												
				
					gain_loss_backup[j][i*4+index] = (gain_loss_backup[j][i*4+index] * coef) / (int)(512);
	//				RTPRINT(FINIT, INIT_IQK, ("==>1DP LUT sram %s index %d value 0x%x()\n", j == 0?"Q":"I", i*4+index, gain_loss_backup[j][i*4+index]));												
					
					gain_loss_backup[j][i*4+index] = gain_loss_backup[j][i*4+index] >= (int)(512)?(int)(511):gain_loss_backup[j][i*4+index] < (int)(-512)?(int)(-512):gain_loss_backup[j][i*4+index];
	//				RTPRINT(FINIT, INIT_IQK, ("==>2DP LUT sram %s index %d value 0x%x()\n", j == 0?"Q":"I", i*4+index, gain_loss_backup[j][i*4+index]));												
					
					gain_loss_backup[j][i*4+index] = gain_loss_backup[j][i*4+index] >> 2;
	//				RTPRINT(FINIT, INIT_IQK, ("==>3DP LUT sram %s index %d value 0x%x()\n", j == 0?"Q":"I", i*4+index, gain_loss_backup[j][i*4+index]));												
					
				}
				tmpReg = ((gain_loss_backup[1][i*4+index] & bMaskByte0) << 8 ) | ((gain_loss_backup[0][i*4+index] & bMaskByte0));
				gain_loss_backup[0][i*4+index] = tmpReg & bMaskLWord;
				DPK_DEBUG("==>DP LUT sram index %d value 0x%x()\n", i*4+index, tmpReg);												
			}
		}
	
		//write DP LUT into sram
		for(i = 0; i < DP_PA_MODEL_NUM; i++)
		{
			value32 =  (path==RF92CD_PATH_A?((i%2 == 0)?0x01000000:0x02000000):
				((i%2 == 0)?0x04000000:0x08000000)) | 
				gain_loss_backup[0][(DP_PA_MODEL_NUM-1)-i] |( (i/2) << 16); 	
			DPK_DEBUG("0xb2c value = 0x%x\n",  value32);									
			PHY_SetBBReg(priv, 0xb2c , bMaskDWord, value32);		
		}
//		PHY_SetBBReg(priv, 0xb2c , bMaskDWord, 0x00000000); 		
			
#endif
			
#endif
	
		///////////////////////////////////////////////////////////////
		// step 5: Enable Digital Predistortion
		///////////////////////////////////////////////////////////////
		// LUT from sram
#if DP_gain_loss == 1
		{
			_PHY_SetADDARegisters(priv, BB_REG_loop[path], BB_settings_loop_dp, DP_BB_REG_NUM_loop_pa);

			// pwsf boundary
			PHY_SetBBReg(priv, offset[path][7], bMaskDWord, 0x000fffff);	//0xb30, 0xba0

			// write pwsf to sram				
			//find tx_index index value
			SramIndex = 24; //restore default value
			SramIndex -= GainLossIndex;
			if(bPlus3db)
				SramIndex += 3*4;			

			SramIndex = SramIndex >= DP_SRAM_NUM_db?DP_SRAM_NUM_db-1:(SramIndex<0?0:SramIndex);
			
			DPK_DEBUG("tx_index = 0x%x, sram value 0x%x gainloss index %d bPlus3db %d\n",  tx_index, Sram_db_settings[SramIndex], GainLossIndex, bPlus3db);						

			index = 0x1f - tx_index;			
			if(SramIndex >= index*4)
			{
				index = SramIndex - index*4;
				index_repeat = -2;
				SramIndex = -2;
			}
			else
			{
				index_repeat = index - SramIndex/4;
				SramIndex %= 4;
				index = 0;				
			}

			index = index >= DP_SRAM_NUM_db?DP_SRAM_NUM_db-1:index;
			if(index_repeat == 1)
				index_1 = SramIndex;
			else
				index_1 = index < (DP_SRAM_NUM_db-1)?(index_repeat==-2?index+1*4:index):index;

			DPK_DEBUG("0x1f value = 0x%x, index 0x%x repeat %d SramIndex %d\n",  Sram_db_settings[index], index, index_repeat, SramIndex); 					
			
			for(i = 0; i < DP_SRAM_NUM; i++)
			{								
				value32 = (path==RF92CD_PATH_A?0x10000000:0x20000000) | (i << 16) | 
					(Sram_db_settings[index_1] << 8 )| Sram_db_settings[index]; 	
				DPK_DEBUG("0xb2c value = 0x%x\n",  value32);					
				
				PHY_SetBBReg(priv, 0xb2c , bMaskDWord, value32);
				if(index_repeat >= 0)
					index_repeat -= 2;
				else if(index_repeat == -1)
					index_repeat = -2;
					
				if((index < (DP_SRAM_NUM_db-1)-1))
				{
					if(index_repeat == -2)
					{						
						index+=2*4;
						index_1 = index < (DP_SRAM_NUM_db-1)?index+1*4:index;		
					}
					if(index_repeat == 0)
					{
						index = SramIndex;
						index_1 = index < (DP_SRAM_NUM_db-1)?index+1*4:index;		
					}	
					else if(index_repeat == 1)
					{						
						index_1 = SramIndex;
					}
					else if(index_repeat == -1)
					{
						index = index_1+1*4;
						index_1 = index < (DP_SRAM_NUM_db-1)?index+1*4:index;								
					}
				}
				else
				{
					index = index_1 = (DP_SRAM_NUM_db-1);
				}

				index = index < DP_SRAM_NUM_db?index:DP_SRAM_NUM_db-1;
				index_1 = index_1 < DP_SRAM_NUM_db?index_1:DP_SRAM_NUM_db-1;
			}
		}		
#else		
		if(!SkipStep5)
		{
			_PHY_SetADDARegisters(priv, BB_REG_loop[path], BB_settings_loop_dp, DP_BB_REG_NUM_loop_pa);

			// pwsf boundary
			PHY_SetBBReg(priv, offset[path][7], bMaskDWord, 0x000fffff);	//0xb30, 0xba0

			// write pwsf to sram				
			//find RF0x1f index value
			if(bPlus3db)
				tx_index += 3;

			index = 0x1f - tx_index;
			if(index_for_zero_db >= index)
			{
				index = index_for_zero_db - index;
				index_repeat = -2;
			}
			else
			{
				index_repeat = index - index_for_zero_db;
				index = 0;				
			}

			index = index >= DP_SRAM_NUM_db?DP_SRAM_NUM_db-1:index;
			index_1 = index < (DP_SRAM_NUM_db-1)?(index_repeat==-2?index+1:index):index;

			DPK_DEBUG("0x1f value = 0x%x, index 0x%x repeat %d\n",	Sram_db_settings[index], index, index_repeat); 					
			
			for(i = 0; i < DP_SRAM_NUM; i++)
			{								
				value32 = (path==RF92CD_PATH_A?0x10000000:0x20000000) | (i << 16) | 
					(Sram_db_settings[index_1] << 8 )| Sram_db_settings[index]; 	
				DPK_DEBUG("0xb2c value = 0x%x\n",  value32);					
				
				PHY_SetBBReg(priv, 0xb2c , bMaskDWord, value32);
				if(index_repeat >= 0)
					index_repeat -= 2;
				else if(index_repeat == -1)
					index_repeat = -2;
					
				if((index < (DP_SRAM_NUM_db-1)-1))
				{
					if(index_repeat == -2)
					{
						index += 2;
						index_1 = index < (DP_SRAM_NUM_db-1)?index+1:index; 	
					}
					if(index_repeat == 0)
					{
						index_1 = index < (DP_SRAM_NUM_db-1)?index+1:index; 	
					}					
					else if(index_repeat == -1)
					{
						index++;
						index_1 = index < (DP_SRAM_NUM_db-1)?index+1:index; 							
					}
				}
				else
				{
					index = index_1 = (DP_SRAM_NUM_db-1);
				}
			}
		}
#endif		
	}

	//reload RF default value
	for(path = 0; path<path_num; path++){
		for( i = 2 ; i < 3 ; i++){
			PHY_SetRFReg(priv, path, RF_REG[i], bMask20Bits, RF_backup[path][i]);
		}
	}

	//Reload standby mode default value (if path B excute DPK)
	if(is2T && priv->pshare->phw->InternalPA5G[RF92CD_PATH_B]) 	
	{
		PHY_SetRFReg(priv, RF92CD_PATH_A, RF_MODE1, bMask20Bits, 0x1000f);
		PHY_SetRFReg(priv, RF92CD_PATH_A, RF_MODE2, bMask20Bits, 0x60101);		
	}
	
	//reload BB default value
	for(index=0; index<DP_BB_REG_NUM; index++)
		PHY_SetBBReg(priv, BB_REG[index], bMaskDWord, BB_backup[index]);

	//external LNA on	
	PHY_SetBBReg(priv, rBndA, 0xF00000, 0x00);
	
	if(is2T)
		PHY_SetBBReg(priv, rBndB, 0xF00000, 0x00);

	//Reload path A BB default value
	_PHY_ReloadADDARegisters(priv, BB_REG_A, BB_backup_A, DP_BB_REG_NUM_A);


#if 1 //Return to Rx mode after dpk
	//printk("BB_REG_A[9] 0x%x BB_backup_A[9] 0x%x\n\n", BB_REG_A[9], BB_backup_A[9]);
	PHY_SetBBReg(priv, BB_REG_A[9], bMaskByte0, 0x50);
	PHY_SetBBReg(priv, BB_REG_A[9], bMaskDWord, BB_backup_A[9]);
#endif

	//Reload path B default value
	if(is2T)
		_PHY_ReloadADDARegisters(priv, BB_REG_B, BB_backup_B, DP_BB_REG_NUM_B);

#if 1 //Return to Rx mode after dpk
	//printk("BB_REG_B[8] 0x%x BB_backup_B[8] 0x%x\n\n", BB_REG_B[8], BB_backup_B[8]);
	PHY_SetBBReg(priv, BB_REG_B[8], bMaskByte0, 0x50);
	PHY_SetBBReg(priv, BB_REG_B[8], bMaskDWord, BB_backup_B[8]);
#endif
	
	//reload AFE default value
	_PHY_ReloadADDARegisters(priv, AFE_REG, AFE_backup, IQK_ADDA_REG_NUM);	

	//reload MAC default value	
	_PHY_ReloadMACRegisters(priv, MAC_REG, MAC_backup);

	priv->pshare->bDPKworking = FALSE;

	DPK_DEBUG("<==_PHY_DigitalPredistortion()\n");
}

#endif


#endif

#endif




