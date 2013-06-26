/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal8192EPhyCfg.c
	
Abstract:
	Defined HAL 92E PHY BB setting functions
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-11-22 Eric              Create.	
--*/

#ifndef __ECOS
#include "HalPrecomp.h"
#else
#include "../../HalPrecomp.h"
#endif


void PHYSetOFDMTxPower8192E(
        IN  HAL_PADAPTER    Adapter, 
        IN  u1Byte          channel
)
{
	unsigned int writeVal, defValue =0x28 ;
	unsigned char  offset;
	char base, byte0, byte1, byte2, byte3;
	unsigned char pwrlevelHT40_1S_A = HAL_VAR_pwrlevelHT40_1S_A(channel-1);
	unsigned char pwrlevelHT40_1S_B = HAL_VAR_pwrlevelHT40_1S_B(channel-1);
	unsigned char pwrdiffHT40_2S = HAL_VAR_pwrdiffHT40_2S(channel-1);
	unsigned char pwrdiffHT20 = HAL_VAR_pwrdiffHT20(channel-1);
	unsigned char pwrdiffOFDM = HAL_VAR_pwrdiffOFDM(channel-1);


#if CFG_HAL_POWER_PERCENT_ADJUSTMENT
#xxx
    printk("Do here 2 \n");

	char pwrdiff_percent = HAL_PwrPercent2PwrLevel(HAL_VAR_power_percent);
#endif




	if ((pwrlevelHT40_1S_A == 0) )

	{	// use default value

#if CFG_HAL_HIGH_POWER_EXT_PA
		if (HAL_VAR_use_ext_pa)
			defValue = HP_OFDM_POWER_DEFAULT ;
#endif
#if  (!CFG_HAL_ADD_TX_POWER_BY_CMD)
#ewqwe
		writeVal = (defValue<<24)|(defValue<<16)|(defValue<<8)|(defValue);
		HAL_RTL_W32(rTxAGC_A_Rate18_06, writeVal);
		HAL_RTL_W32(rTxAGC_A_Rate54_24, writeVal);
		HAL_RTL_W32(rTxAGC_A_Mcs03_Mcs00, writeVal);
		HAL_RTL_W32(rTxAGC_A_Mcs07_Mcs04, writeVal);
		HAL_RTL_W32(rTxAGC_B_Rate18_06, writeVal);
		HAL_RTL_W32(rTxAGC_B_Rate54_24, writeVal);
		HAL_RTL_W32(rTxAGC_B_Mcs03_Mcs00, writeVal);
		HAL_RTL_W32(rTxAGC_B_Mcs07_Mcs04, writeVal);
		HAL_RTL_W32(rTxAGC_A_Mcs11_Mcs08, writeVal);
		HAL_RTL_W32(rTxAGC_A_Mcs15_Mcs12, writeVal);
		HAL_RTL_W32(rTxAGC_B_Mcs11_Mcs08, writeVal);
		HAL_RTL_W32(rTxAGC_B_Mcs15_Mcs12, writeVal);
#else
        printk("Do here 4 \n");

		base = defValue;
		byte0 = byte1 = byte2 = byte3 = 0;
		HAL_ASSIGN_TX_POWER_OFFSET(byte0, HAL_VAR_txPowerPlus_ofdm_18);
		HAL_ASSIGN_TX_POWER_OFFSET(byte1, HAL_VAR_txPowerPlus_ofdm_12);
		HAL_ASSIGN_TX_POWER_OFFSET(byte2, HAL_VAR_txPowerPlus_ofdm_9);
		HAL_ASSIGN_TX_POWER_OFFSET(byte3, HAL_VAR_txPowerPlus_ofdm_6);

		byte0 = HAL_POWER_RANGE_CHECK(base + byte0);
		byte1 = HAL_POWER_RANGE_CHECK(base + byte1);
		byte2 = HAL_POWER_RANGE_CHECK(base + byte2);
		byte3 = HAL_POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
		HAL_RTL_W32(rTxAGC_A_Rate18_06, writeVal);
		HAL_RTL_W32(rTxAGC_B_Rate18_06, writeVal);

		byte0 = byte1 = byte2 = byte3 = 0;
		HAL_ASSIGN_TX_POWER_OFFSET(byte0, HAL_VAR_txPowerPlus_ofdm_54);
		HAL_ASSIGN_TX_POWER_OFFSET(byte1, HAL_VAR_txPowerPlus_ofdm_48);
		HAL_ASSIGN_TX_POWER_OFFSET(byte2, HAL_VAR_txPowerPlus_ofdm_36);
		HAL_ASSIGN_TX_POWER_OFFSET(byte3, HAL_VAR_txPowerPlus_ofdm_24);
		byte0 = HAL_POWER_RANGE_CHECK(base + byte0);
		byte1 = HAL_POWER_RANGE_CHECK(base + byte1);
		byte2 = HAL_POWER_RANGE_CHECK(base + byte2);
		byte3 = HAL_POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
		HAL_RTL_W32(rTxAGC_A_Rate54_24, writeVal);
		HAL_RTL_W32(rTxAGC_B_Rate54_24, writeVal);

		byte0 = byte1 = byte2 = byte3 = 0;
		HAL_ASSIGN_TX_POWER_OFFSET(byte0, HAL_VAR_txPowerPlus_mcs_3);
		HAL_ASSIGN_TX_POWER_OFFSET(byte1, HAL_VAR_txPowerPlus_mcs_2);
		HAL_ASSIGN_TX_POWER_OFFSET(byte2, HAL_VAR_txPowerPlus_mcs_1);
		HAL_ASSIGN_TX_POWER_OFFSET(byte3, HAL_VAR_txPowerPlus_mcs_0);
		byte0 = HAL_POWER_RANGE_CHECK(base + byte0);
		byte1 = HAL_POWER_RANGE_CHECK(base + byte1);
		byte2 = HAL_POWER_RANGE_CHECK(base + byte2);
		byte3 = HAL_POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
		HAL_RTL_W32(rTxAGC_A_Mcs03_Mcs00, writeVal);
		HAL_RTL_W32(rTxAGC_B_Mcs03_Mcs00, writeVal);

		byte0 = byte1 = byte2 = byte3 = 0;
		HAL_ASSIGN_TX_POWER_OFFSET(byte0, HAL_VAR_txPowerPlus_mcs_7);
		HAL_ASSIGN_TX_POWER_OFFSET(byte1, HAL_VAR_txPowerPlus_mcs_6);
		HAL_ASSIGN_TX_POWER_OFFSET(byte2, HAL_VAR_txPowerPlus_mcs_5);
		HAL_ASSIGN_TX_POWER_OFFSET(byte3, HAL_VAR_txPowerPlus_mcs_4);
		byte0 = HAL_POWER_RANGE_CHECK(base + byte0);
		byte1 = HAL_POWER_RANGE_CHECK(base + byte1);
		byte2 = HAL_POWER_RANGE_CHECK(base + byte2);
		byte3 = HAL_POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
		HAL_RTL_W32(rTxAGC_A_Mcs07_Mcs04, writeVal);
		HAL_RTL_W32(rTxAGC_B_Mcs07_Mcs04, writeVal);


		byte0 = HAL_POWER_RANGE_CHECK(base + byte0);
		byte1 = HAL_POWER_RANGE_CHECK(base + byte1);
		byte2 = HAL_POWER_RANGE_CHECK(base + byte2);
		byte3 = HAL_POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
		HAL_RTL_W32(rTxAGC_A_Mcs11_Mcs08, writeVal);
		HAL_RTL_W32(rTxAGC_B_Mcs11_Mcs08, writeVal);



		byte0 = HAL_POWER_RANGE_CHECK(base + byte0);
		byte1 = HAL_POWER_RANGE_CHECK(base + byte1);
		byte2 = HAL_POWER_RANGE_CHECK(base + byte2);
		byte3 = HAL_POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
		HAL_RTL_W32(rTxAGC_A_Mcs15_Mcs12, writeVal);
		HAL_RTL_W32(rTxAGC_B_Mcs15_Mcs12, writeVal);

#endif // ADD_TX_POWER_BY_CMD
		return; // use default
	}

	/******************************  PATH A  ******************************/
	base = pwrlevelHT40_1S_A;
	offset = (pwrdiffOFDM & 0x0f);
	base = HAL_COUNT_SIGN_OFFSET(base, offset);
#if CFG_HAL_POWER_PERCENT_ADJUSTMENT
	base = base + pwrdiff_percent;
#endif

    printk("Do here 5 \n");
    // fill 6Mbps ~ 18Mbps
	byte0 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_A(0));
	byte1 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_A(1));
	byte2 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_A(2));
	byte3 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_A(3));
	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	HAL_RTL_W32(rTxAGC_A_Rate18_06, writeVal);

    // fill 24Mbps ~ 54Mbps
	byte0 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_A(4));
	byte1 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_A(5));
	byte2 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_A(6));
	byte3 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_A(7));
	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	HAL_RTL_W32(rTxAGC_A_Rate54_24, writeVal);

    // cal HT rate diff
	base = pwrlevelHT40_1S_A;
	if (HAL_VAR_CurrentChannelBW == HT_CHANNEL_WIDTH_20) {
		offset = (pwrdiffHT20 & 0x0f);
		base = HAL_COUNT_SIGN_OFFSET(base, offset);
	}
#if CFG_HAL_POWER_PERCENT_ADJUSTMENT
	base = base + pwrdiff_percent;
#endif

    printk("Do here 6 \n");

    // fill MCS0~MCS3
	byte0 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(0));
	byte1 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(1));
	byte2 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(2));
	byte3 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(3));
	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	HAL_RTL_W32(rTxAGC_A_Mcs03_Mcs00, writeVal);

    // fill MCS4~MCS7
	byte0 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(4));
	byte1 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(5));
	byte2 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(6));
	byte3 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(7));
	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	HAL_RTL_W32(rTxAGC_A_Mcs07_Mcs04, writeVal);

	offset = (pwrdiffHT40_2S & 0x0f);	
	base = HAL_COUNT_SIGN_OFFSET(base, offset);

    printk("Do here 7 fill MCS8~11  \n");

    //_TXPWR_REDEFINE ?? MCS 8 - 11, shall NOT add power by rate even NOT USB power ??
    byte0 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(8));
    byte1 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(9));
    byte2 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(10));
    byte3 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(11));

	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;

	//DEBUG_INFO("debug e18:%x,%x,[%x,%x,%x,%x],%x\n", offset, base, byte0, byte1, byte2, byte3, writeVal);
	HAL_RTL_W32(rTxAGC_A_Mcs11_Mcs08, writeVal);


        printk("Do here 8 \n");

		byte0 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(12));
		byte1 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(13));
		byte2 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(14));
		byte3 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_A(15));

    printk("Do here 9 \n");

	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	HAL_RTL_W32(rTxAGC_A_Mcs15_Mcs12, writeVal);

	/******************************  PATH B  ******************************/
	base = pwrlevelHT40_1S_B;
	offset = ((pwrdiffOFDM & 0xf0) >> 4);
	base = HAL_COUNT_SIGN_OFFSET(base, offset);
#if CFG_HAL_POWER_PERCENT_ADJUSTMENT
	base = base + pwrdiff_percent;
#endif

    printk("Do here 10 \n");

	byte0 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_B(0));
	byte1 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_B(1));
	byte2 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_B(2));
	byte3 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_B(3));
	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	HAL_RTL_W32(rTxAGC_B_Rate18_06, writeVal);

	byte0 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_B(4));
	byte1 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_B(5));
	byte2 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_B(6));
	byte3 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_OFDMTxAgcOffset_B(7));
	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	HAL_RTL_W32(rTxAGC_B_Rate54_24, writeVal);

	base = pwrlevelHT40_1S_B;
	if (HAL_VAR_CurrentChannelBW == HT_CHANNEL_WIDTH_20) {
		offset = ((pwrdiffHT20 & 0xf0) >> 4);
		base = HAL_COUNT_SIGN_OFFSET(base, offset);
	}
#if CFG_HAL_POWER_PERCENT_ADJUSTMENT
	base = base + pwrdiff_percent;
#endif

    printk("Do here 11 \n");

	byte0 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(0));
	byte1 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(1));
	byte2 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(2));
	byte3 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(3));
	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	HAL_RTL_W32(rTxAGC_B_Mcs03_Mcs00, writeVal);

	byte0 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(4));
	byte1 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(5));
	byte2 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(6));
	byte3 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(7));
	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	HAL_RTL_W32(rTxAGC_B_Mcs07_Mcs04, writeVal);

	offset = ((pwrdiffHT40_2S & 0xf0) >> 4);
	base = HAL_COUNT_SIGN_OFFSET(base, offset);


        printk("Do here 12 \n");

		byte0 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(8));
		byte1 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(9));
		byte2 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(10));
		byte3 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(11));


    printk("Do here 13 \n");

	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	HAL_RTL_W32(rTxAGC_B_Mcs11_Mcs08, writeVal);



        printk("Do here 13.5 \n");

		byte0 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(12));
		byte1 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(13));
		byte2 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(14));
		byte3 = HAL_POWER_RANGE_CHECK(base + HAL_VAR_MCSTxAgcOffset_B(15));


    printk("Do here 14 \n");

	writeVal = (byte0<<24) | (byte1<<16) |(byte2<<8) | byte3;
	HAL_RTL_W32(rTxAGC_B_Mcs15_Mcs12, writeVal);
}

void PHYSetCCKTxPower8192E(
        IN  HAL_PADAPTER    Adapter, 
        IN  u1Byte          channel
)
{
	unsigned int writeVal = 0;
	char byte, byte1, byte2;
	char pwrlevelCCK_A = HAL_VAR_pwrlevelCCK_A(channel-1);
	char pwrlevelCCK_B = HAL_VAR_pwrlevelCCK_B(channel-1);
#if CFG_HAL_POWER_PERCENT_ADJUSTMENT
	char pwrdiff_percent = HAL_PwrPercent2PwrLevel(HAL_VAR_power_percent);
#endif




	if (HAL_cck_pwr_max) {
		byte = HAL_POWER_RANGE_CHECK((char)HAL_cck_pwr_max);
		writeVal = byte;
		HAL_PHY_SetBBReg(Adapter, rTxAGC_A_CCK1_Mcs32, 0x0000ff00, writeVal);
		writeVal = (byte << 16) | (byte << 8) | byte;
		HAL_PHY_SetBBReg(Adapter, rTxAGC_B_CCK5_1_Mcs32, 0xffffff00, writeVal);
		writeVal = (byte << 24) | (byte << 16) | (byte << 8) | byte;
		HAL_PHY_SetBBReg(Adapter, rTxAGC_A_CCK11_2_B_CCK11, 0xffffffff, writeVal);
		return;
	}

	if ((pwrlevelCCK_A == 0))
	{	
#if CFG_HAL_HIGH_POWER_EXT_PA
		if (HAL_VAR_use_ext_pa)
			byte = HP_CCK_POWER_DEFAULT;
		else
#endif
			byte = 0x24;


#if (!CFG_HAL_ADD_TX_POWER_BY_CMD)
        // use default value
		writeVal = byte;
		HAL_PHY_SetBBReg(Adapter, rTxAGC_A_CCK1_Mcs32, 0x0000ff00, writeVal);
		writeVal = (byte << 16) | (byte << 8) | byte;
		HAL_PHY_SetBBReg(Adapter, rTxAGC_B_CCK5_1_Mcs32, 0xffffff00, writeVal);
		writeVal = (byte << 24) | (byte << 16) | (byte << 8) | byte;
		HAL_PHY_SetBBReg(Adapter, rTxAGC_A_CCK11_2_B_CCK11, 0xffffffff, writeVal);
#else
        // use cmd value
		pwrlevelCCK_A = pwrlevelCCK_B = byte;
		byte = 0;
		HAL_ASSIGN_TX_POWER_OFFSET(byte, HAL_txPowerPlus_cck_1);
		writeVal = HAL_POWER_RANGE_CHECK(pwrlevelCCK_A + byte);
		HAL_PHY_SetBBReg(Adapter, rTxAGC_A_CCK1_Mcs32, 0x0000ff00, writeVal);

		byte = byte1 = byte2 = 0;
		HAL_ASSIGN_TX_POWER_OFFSET(byte, HAL_txPowerPlus_cck_1);
		HAL_ASSIGN_TX_POWER_OFFSET(byte1, HAL_txPowerPlus_cck_2);
		HAL_ASSIGN_TX_POWER_OFFSET(byte2, HAL_txPowerPlus_cck_5);
		byte  = HAL_POWER_RANGE_CHECK(pwrlevelCCK_B + byte);
		byte1 = HAL_POWER_RANGE_CHECK(pwrlevelCCK_B + byte1);
		byte2 = HAL_POWER_RANGE_CHECK(pwrlevelCCK_B + byte2);
		writeVal = ((byte2 << 16) | (byte1 << 8) | byte);
		HAL_PHY_SetBBReg(Adapter, rTxAGC_B_CCK5_1_Mcs32, 0xffffff00, writeVal);

		byte = byte1 = byte2 = 0;
		HAL_ASSIGN_TX_POWER_OFFSET(byte, HAL_txPowerPlus_cck_2);
		HAL_ASSIGN_TX_POWER_OFFSET(byte1, HAL_txPowerPlus_cck_5);
		HAL_ASSIGN_TX_POWER_OFFSET(byte2, HAL_txPowerPlus_cck_11);
		byte  = HAL_POWER_RANGE_CHECK(pwrlevelCCK_A + byte);
		byte1 = HAL_POWER_RANGE_CHECK(pwrlevelCCK_A + byte1);
		byte2 = HAL_POWER_RANGE_CHECK(pwrlevelCCK_A + byte2);
		writeVal = ((byte2 << 24) | (byte1 << 16) | (byte << 8) | byte2);
		HAL_PHY_SetBBReg(Adapter, rTxAGC_A_CCK11_2_B_CCK11, 0xffffffff, writeVal);
#endif
		return; // use default
	}
#if CFG_HAL_POWER_PERCENT_ADJUSTMENT
	pwrlevelCCK_A += pwrdiff_percent;
	pwrlevelCCK_B += pwrdiff_percent;
#endif

    // use flash value to fill 
	writeVal = HAL_POWER_RANGE_CHECK(pwrlevelCCK_A + HAL_CCKTxAgc_A(3));
	HAL_PHY_SetBBReg(Adapter, rTxAGC_A_CCK1_Mcs32, 0x0000ff00, writeVal);
	writeVal = (HAL_POWER_RANGE_CHECK(pwrlevelCCK_B + HAL_CCKTxAgc_B(1)) << 16) |
	           (HAL_POWER_RANGE_CHECK(pwrlevelCCK_B + HAL_CCKTxAgc_B(2)) << 8)  |
	            HAL_POWER_RANGE_CHECK(pwrlevelCCK_B + HAL_CCKTxAgc_B(3));
	HAL_PHY_SetBBReg(Adapter, rTxAGC_B_CCK5_1_Mcs32, 0xffffff00, writeVal);
	writeVal = (HAL_POWER_RANGE_CHECK(pwrlevelCCK_A + HAL_CCKTxAgc_A(0)) << 24) |
	           (HAL_POWER_RANGE_CHECK(pwrlevelCCK_A + HAL_CCKTxAgc_A(1)) << 16) |
	           (HAL_POWER_RANGE_CHECK(pwrlevelCCK_A + HAL_CCKTxAgc_A(2)) << 8)  |
	            HAL_POWER_RANGE_CHECK(pwrlevelCCK_B + HAL_CCKTxAgc_B(0));
	HAL_PHY_SetBBReg(Adapter, rTxAGC_A_CCK11_2_B_CCK11, 0xffffffff, writeVal);
}


