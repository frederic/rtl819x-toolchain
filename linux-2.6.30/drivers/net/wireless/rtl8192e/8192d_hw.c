
#define _8192D_HW_C_

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <asm/unistd.h>
#endif

#include "./8192cd_cfg.h"
#include "./8192cd.h"
#include "./8192cd_hw.h"
#include "./8192cd_headers.h"
#include "./8192cd_debug.h"


#ifdef CONFIG_RTL_92D_SUPPORT

#if defined(CONFIG_RTL_819X) && defined(USE_RLX_BSP)
#include <bsp/bspchip.h>
#endif

#if defined(CONFIG_RTL865X_WTDOG) || defined(CONFIG_RTL_WTDOG)
#ifdef CONFIG_NET_PCI
#ifndef CONFIG_RTL_8198B
        #define BSP_WDTCNR 0xB800311C
#endif
#endif
#endif

#ifndef USE_OUT_SRC
#define IQK_ADDA_REG_NUM 16
#endif


#ifdef CONFIG_RTL_92D_DMDP

extern u32 if_priv[];

__inline__ unsigned char DMDP_RTL_R8(unsigned int phy, unsigned int reg)
{
	struct rtl8192cd_priv *priv;
	//printk("++++++++++++++++++++++++++%s(%x)++++++++++++++++++++++++++\n", __FUNCTION__, reg);
	if (phy >= NUM_WLAN_IFACE || phy < 0) {
		printk("%s: phy index[%d] out of bound !!\n", __FUNCTION__, phy);
		return -1;
	}
	priv = (struct rtl8192cd_priv *)if_priv[phy];
	return RTL_R8(reg);
}

__inline__ void DMDP_RTL_W8(unsigned int phy, unsigned int reg, unsigned char val8)
{
	struct rtl8192cd_priv *priv;
	//printk("++++++++++++++++++++++++++%s(%x,%x)++++++++++++++++++++++++++\n", __FUNCTION__, reg, val8);
	if (phy >= NUM_WLAN_IFACE || phy < 0) {
		printk("%s: phy index[%d] out of bound !!\n", __FUNCTION__, phy);
		return;
	}
	priv = (struct rtl8192cd_priv *)if_priv[phy];
	RTL_W8(reg, val8);
}

__inline__ unsigned short DMDP_RTL_R16(unsigned int phy, unsigned int reg)
{
	struct rtl8192cd_priv *priv;
	//printk("++++++++++++++++++++++++++%s++++++++++++++++++++++++++\n", __FUNCTION__);
	if (phy >= NUM_WLAN_IFACE || phy < 0) {
		printk("%s: phy index[%d] out of bound !!\n", __FUNCTION__, phy);
		return -1;
	}
	priv = (struct rtl8192cd_priv *)if_priv[phy];
	return RTL_R16(reg);
}

__inline__ void DMDP_RTL_W16(unsigned int phy, unsigned int reg, unsigned short val16)
{
	struct rtl8192cd_priv *priv;
	//printk("++++++++++++++++++++++++++%s++++++++++++++++++++++++++\n", __FUNCTION__);
	if (phy >= NUM_WLAN_IFACE || phy < 0) {
		printk("%s: phy index[%d] out of bound !!\n", __FUNCTION__, phy);
		return;
	}
	priv = (struct rtl8192cd_priv *)if_priv[phy];
	RTL_W16(reg, val16);
}

__inline__ unsigned int DMDP_RTL_R32(unsigned int phy, unsigned int reg)
{
	struct rtl8192cd_priv *priv;
	//printk("++++++++++++++++++++++++++%s(%x)++++++++++++++++++++++++++\n", __FUNCTION__, reg);
	if (phy >= NUM_WLAN_IFACE || phy < 0) {
		printk("%s: phy index[%d] out of bound !!\n", __FUNCTION__, phy);
		return -1;
	}
	priv = (struct rtl8192cd_priv *)if_priv[phy];
	return RTL_R32(reg);
}

__inline__ void DMDP_RTL_W32(unsigned int phy, unsigned int reg, unsigned int val32)
{
	struct rtl8192cd_priv *priv;
	//printk("++++++++++++++++++++++++++%s(%x, %x)++++++++++++++++++++++++++\n", __FUNCTION__, reg, val32);
	if (phy >= NUM_WLAN_IFACE || phy < 0) {
		printk("%s: phy index[%d] out of bound !!\n", __FUNCTION__, phy);
		return;
	}
	priv = (struct rtl8192cd_priv *)if_priv[phy];
	RTL_W32(reg, val32);
}

unsigned int DMDP_PHY_QueryBBReg(unsigned int phy, unsigned int RegAddr, unsigned int BitMask)
{
	//printk("++++++++++++++++++++++++++%s++++++++++++++++++++++++++\n", __FUNCTION__);
	if (phy >= NUM_WLAN_IFACE || phy < 0) {
		printk("%s: phy index[%d] out of bound !!\n", __FUNCTION__, phy);
		return -1;
	}
	return PHY_QueryBBReg((struct rtl8192cd_priv *)if_priv[phy], RegAddr, BitMask);
}

void DMDP_PHY_SetBBReg(unsigned int phy, unsigned int RegAddr, unsigned int BitMask, unsigned int Data)
{
	//printk("++++++++++++++++++++++++++%s++++++++++++++++++++++++++\n", __FUNCTION__);
	if (phy >= NUM_WLAN_IFACE || phy < 0) {
		printk("%s: phy index[%d] out of bound !!\n", __FUNCTION__, phy);
		return;
	}
	PHY_SetBBReg((struct rtl8192cd_priv *)if_priv[phy], RegAddr, BitMask, Data);
}

unsigned int DMDP_PHY_QueryRFReg(unsigned int phy, RF92CD_RADIO_PATH_E eRFPath,
				unsigned int RegAddr, unsigned int BitMask, unsigned int dbg_avoid)
{
	//printk("++++++++++++++++++++++++++%s++++++++++++++++++++++++++\n", __FUNCTION__);
	if (phy >= NUM_WLAN_IFACE || phy < 0) {
		printk("%s: phy index[%d] out of bound !!\n", __FUNCTION__, phy);
		return -1;
	}
	return PHY_QueryRFReg((struct rtl8192cd_priv *)if_priv[phy], eRFPath, RegAddr, BitMask, dbg_avoid);
}

void DMDP_PHY_SetRFReg(unsigned int phy, RF92CD_RADIO_PATH_E eRFPath, unsigned int RegAddr,
				unsigned int BitMask, unsigned int Data)
{
	//printk("++++++++++++++++++++++++++%s++++++++++++++++++++++++++\n", __FUNCTION__);
	if (phy >= NUM_WLAN_IFACE || phy < 0) {
		printk("%s: phy index[%d] out of bound !!\n", __FUNCTION__, phy);
		return;
	}
	PHY_SetRFReg((struct rtl8192cd_priv *)if_priv[phy], eRFPath, RegAddr, BitMask, Data);
}

#endif //CONFIG_RTL_92D_DMDP

void SetSYN_para(struct rtl8192cd_priv *priv, unsigned char channel)
{
	unsigned int eRFPath, tmp=0;
	unsigned int idx=-1, i;
	unsigned int SYN_PARA[8][8] = {
		{0xe43be, 0xfc638, 0x77c0a, 0xde471, 0xd7110, 0x8cb04, 0x00000, 0x00000},	// CH36-140 20MHz
		{0xe43be, 0xfc078, 0xf7c1a, 0xe0c71, 0xd7550, 0xacb04, 0x00000, 0x00000},	// CH36-140 40MHz
		{0xe43bf, 0xff038, 0xf7c0a, 0xde471, 0xe5550, 0xacb04, 0x00000, 0x00000},	// CH149, 155, 161
		{0xe43bf, 0xff079, 0xf7c1a, 0xde471, 0xe5550, 0xacb04, 0x00000, 0x00000},	// CH151, 153, 163, 165
		{0xe43bf, 0xff038, 0xf7c1a, 0xde471, 0xd7550, 0xacb04, 0x00000, 0x00000},	// CH157, 159
#ifdef SW_LCK_92D
		{0x643bc, 0xfc038, 0x77c1a, 0x00000, 0x00000, 0x00000, 0x61289, 0x01840},	// CH1,2,4,9,10,11,12
		{0x643bc, 0xfc038, 0x07c1a, 0x00000, 0x00000, 0x00000, 0x61289, 0x01840},	// CH3,13,14
		{0x243bc, 0xfc438, 0x07c1a, 0x00000, 0x00000, 0x00000, 0x6128b, 0x0fc41}	// CH5-8
#else
		{0x643bc, 0xfc038, 0x77c1a, 0x00000, 0x00000, 0x00000, 0x41289, 0x01840},	// CH1,2,4,9,10,11,12
		{0x643bc, 0xfc038, 0x07c1a, 0x00000, 0x00000, 0x00000, 0x41289, 0x01840},	// CH3,13,14
		{0x243bc, 0xfc438, 0x07c1a, 0x00000, 0x00000, 0x00000, 0x4128b, 0x0fc41}	// CH5-8
#endif
	};


	if (priv->pmib->dot11RFEntry.phyBandSelect==PHY_BAND_2G)
		eRFPath = RF92CD_PATH_B;
	else
		eRFPath = RF92CD_PATH_A;

	if (priv->pmib->dot11RFEntry.phyBandSelect==PHY_BAND_5G){
		if (channel >=36 && channel <=140){
			if (!priv->pshare->CurrentChannelBW)
				idx = 0;
			else
				idx = 1;
		}
		else if (channel==149 || channel==155 || channel==161)
			idx = 2;
		else if (channel==151 || channel==153 || channel==163 || channel==165)
			idx = 3;
		else if (channel==157 || channel==159)
			idx = 4;
	} else {
		if (channel==1 || channel==2 || channel==4 || channel==9 || channel==10 || channel==11 || channel==12)
			idx = 5;
		else if (channel==3 || channel==13 || channel==14)
			idx = 6;
		else if (channel>=5 && channel<=8)
			idx = 7;
	}

	if (idx==-1){
		DEBUG_ERR("No suitable channel (%d) for setting synthersizer parameter!\n", channel);
		return;
	}

	for (i=0;i<8;i++){
#ifdef CONFIG_RTL_92D_DMDP
		if (i==0 && (idx>=0 && idx <=4) &&
			(priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY))
			tmp = 0xe439d;
		else
#endif
			tmp = SYN_PARA[idx][i];

		if (tmp!=0) {

#ifdef CONFIG_RTL_92D_DMDP
			if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY && eRFPath == RF92CD_PATH_B) {
				DMDP_PHY_SetRFReg(1, RF92CD_PATH_A, (0x25+i), bMask20Bits, tmp);
				//DEBUG_TRACE("DMDP_PHY_SetRFReg(1, %d, 0x%x, bMask20Bits, 0x%x)\n", eRFPath, (0x25+i), tmp);
			} else
#endif
			{
				PHY_SetRFReg(priv, eRFPath, (0x25+i), bMask20Bits, tmp);
				//DEBUG_TRACE("PHY_SetRFReg(priv, %d, 0x%x, bMask20Bits, 0x%x)\n", eRFPath, (0x25+i), tmp);
			}
			if (i==3)
				priv->pshare->RegRF28[eRFPath] = tmp;
		}
	}
}

unsigned int IMR_SET_N[3][11] = {
  {0x00ff0, 0x4400f, 0x00ff0, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x64888, 0xe266c, 0x00090}, //G-mode
  {0x22880, 0x4470f, 0x55880, 0x00070, 0x88000, 0x00000, 0x88080, 0x70000, 0x64a82, 0xe466c, 0x00090}, //36-64
  {0x44880, 0x4477f, 0x77880, 0x00070, 0x88000, 0x00000, 0x880b0, 0x00000, 0x64b82, 0xe466c, 0x00090}, // 100-165
};

void SetIMR_n(struct rtl8192cd_priv *priv, unsigned char channel)
{
	unsigned int eRFPath, curMaxRFPath;
	int imr_idx = -1;
	unsigned char temp_800;

#ifdef CONFIG_RTL_92D_DMDP
	if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY)
		curMaxRFPath = RF92CD_PATH_B;
	else
#endif
		curMaxRFPath = RF92CD_PATH_MAX;

	if (priv->pmib->dot11RFEntry.phyBandSelect==PHY_BAND_2G)
		imr_idx = 0;
	else {
		if (channel>=36 && channel <=64)
			imr_idx = 1;
		else
			imr_idx = 2;
	}
	PHY_SetBBReg(priv, rFPGA0_AnalogParameter4, 0x00f00000, 0xf);
	temp_800 = PHY_QueryBBReg(priv, rFPGA0_RFMOD, 0x0f000000);
	PHY_SetBBReg(priv, rFPGA0_RFMOD, 0x0f000000, 0);

	for(eRFPath = RF92CD_PATH_A; eRFPath < curMaxRFPath; eRFPath++) {
		int i;

		PHY_SetRFReg(priv, eRFPath, 0x00, bMask20Bits, 0x70000);
		//DEBUG_TRACE("IMR [0x00] %05x\n", PHY_QueryRFReg(priv, eRFPath, 0x00, bMask20Bits,1));
		//delay_us(5);

		for (i=0;i<11;i++) {
			PHY_SetRFReg(priv, eRFPath, (0x2f+i), bMask20Bits, IMR_SET_N[imr_idx][i]);
			//DEBUG_TRACE("IMR [0x%x] %05x\n", (0x2f+i), PHY_QueryRFReg(priv, eRFPath, (0x2f+i), bMask20Bits,1));
			//delay_us(5);
		}

		if (priv->pmib->dot11RFEntry.phyBandSelect==PHY_BAND_2G)
			PHY_SetRFReg(priv, eRFPath, 0x00, bMask20Bits, 0x32fff);
		else
			PHY_SetRFReg(priv, eRFPath, 0x00, bMask20Bits, 0x32c9a);

		//DEBUG_TRACE("IMR [0x00] %05x\n", PHY_QueryRFReg(priv, eRFPath, 0x00, bMask20Bits,1));
		//delay_us(5);
	}

	PHY_SetBBReg(priv, rFPGA0_RFMOD, 0x0f000000, temp_800);
	PHY_SetBBReg(priv, rFPGA0_AnalogParameter4, 0x00f00000, 0x0);
}


/*
 *  Follow WS-20101228-Willis-xxxx dynamic parameter-R00
 */
void Update92DRFbyChannel(struct rtl8192cd_priv *priv, unsigned char channel)
{
#ifdef RTL8192D_INT_PA
	u8	eRFPath = 0, curMaxRFPath;
	if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY)
		curMaxRFPath = RF92CD_PATH_B;
	else
		curMaxRFPath = RF92CD_PATH_MAX;

	if (priv->pshare->rf_ft_var.use_intpa92d){
		for(eRFPath = RF92CD_PATH_A; eRFPath <curMaxRFPath; eRFPath++) {
			if (priv->pmib->dot11RFEntry.phyBandSelect==PHY_BAND_5G){
				if (channel>=36 && channel<=64){
					PHY_SetRFReg(priv, eRFPath, 0x0b, bMask20Bits, 0x01a00);
					PHY_SetRFReg(priv, eRFPath, 0x48, bMask20Bits, 0x40443);
					PHY_SetRFReg(priv, eRFPath, 0x49, bMask20Bits, 0x00eb5);
					//PHY_SetRFReg(priv, eRFPath, 0x4a, bMask20Bits, 0x50f0f);
					PHY_SetRFReg(priv, eRFPath, 0x4b, bMask20Bits, 0x89bec);
					//PHY_SetRFReg(priv, eRFPath, 0x4c, bMask20Bits, 0x0dded);
					PHY_SetRFReg(priv, eRFPath, 0x03, bMask20Bits, 0x94a12);
					PHY_SetRFReg(priv, eRFPath, 0x04, bMask20Bits, 0x94a12);
					PHY_SetRFReg(priv, eRFPath, 0x0e, bMask20Bits, 0x94a12);
				}else if (channel>=100 && channel<=140){
					PHY_SetRFReg(priv, eRFPath, 0x0b, bMask20Bits, 0x01800);
					PHY_SetRFReg(priv, eRFPath, 0x48, bMask20Bits, 0xc0443);
					PHY_SetRFReg(priv, eRFPath, 0x49, bMask20Bits, 0x00730);
					//PHY_SetRFReg(priv, eRFPath, 0x4a, bMask20Bits, 0x50f0f);
					PHY_SetRFReg(priv, eRFPath, 0x4b, bMask20Bits, 0x896ee);
					//PHY_SetRFReg(priv, eRFPath, 0x4c, bMask20Bits, 0x0dded);
					PHY_SetRFReg(priv, eRFPath, 0x03, bMask20Bits, 0x94a52);
					PHY_SetRFReg(priv, eRFPath, 0x04, bMask20Bits, 0x94a52);
					PHY_SetRFReg(priv, eRFPath, 0x0e, bMask20Bits, 0x94a52);
				}else if (channel>=149 && channel<=165){
					PHY_SetRFReg(priv, eRFPath, 0x0b, bMask20Bits, 0x01800);
					PHY_SetRFReg(priv, eRFPath, 0x48, bMask20Bits, 0xc0443);
					PHY_SetRFReg(priv, eRFPath, 0x49, bMask20Bits, 0x00730);
					//PHY_SetRFReg(priv, eRFPath, 0x4a, bMask20Bits, 0x50f0f);
					PHY_SetRFReg(priv, eRFPath, 0x4b, bMask20Bits, 0x896ee);
					//PHY_SetRFReg(priv, eRFPath, 0x4c, bMask20Bits, 0x0dded);
					PHY_SetRFReg(priv, eRFPath, 0x03, bMask20Bits, 0x94a12);
					PHY_SetRFReg(priv, eRFPath, 0x04, bMask20Bits, 0x94a12);
					PHY_SetRFReg(priv, eRFPath, 0x0e, bMask20Bits, 0x94a12);
				}
			}else{
					PHY_SetRFReg(priv, eRFPath, 0x0b, bMask20Bits, 0x1c000);
					PHY_SetRFReg(priv, eRFPath, 0x03, bMask20Bits, 0x18c63);
					PHY_SetRFReg(priv, eRFPath, 0x04, bMask20Bits, 0x18c63);
					PHY_SetRFReg(priv, eRFPath, 0x0e, bMask20Bits, 0x18c67);
			}
		}
	}
#endif

	if (priv->pmib->dot11RFEntry.phyBandSelect==PHY_BAND_5G){
		//update fc_area
		if (priv->pmib->dot11RFEntry.dot11channel<149)
			PHY_SetBBReg(priv, rOFDM1_CFOTracking, BIT(14) | BIT(13), 1);
		else
			PHY_SetBBReg(priv, rOFDM1_CFOTracking, BIT(14) | BIT(13), 2);
		// VCO_BF_LDO= 1.12V->1.27V for  40M spur issue
		PHY_SetRFReg(priv, RF92CD_PATH_A, 0x2A, BIT(13)|BIT(12), 2);
		// RX Ch36 40M spurs
		if (channel==36){
			priv->pshare->RegRF28[RF92CD_PATH_A] &= (~BIT(6));
			priv->pshare->RegRF28[RF92CD_PATH_A] |= BIT(5);
			PHY_SetRFReg(priv, RF92CD_PATH_A, 0x28, bMask20Bits, priv->pshare->RegRF28[RF92CD_PATH_A]);
			//PHY_SetRFReg(priv, RF92CD_PATH_A, 0x28, BIT(6)|BIT(5), 0);
		}
	} else {
		//update fc_area
		PHY_SetBBReg(priv, rOFDM1_CFOTracking, BIT(14) | BIT(13), 0);
	}

}

int Load_92D_Firmware(struct rtl8192cd_priv *priv)
{
	int fw_len, wait_cnt=0;
	unsigned int CurPtr=0;
	unsigned int WriteAddr;
	unsigned int Temp;
	unsigned char *ptmp;
	unsigned long flags = 0;

#ifdef CONFIG_RTL8672
	printk("val=%x\n",RTL_R8(0x80));
#endif

#ifdef MP_TEST
	if (priv->pshare->rf_ft_var.mp_specific)
		return TRUE;
#endif

	printk("===> %s\n", __FUNCTION__);

	SAVE_INT_AND_CLI(flags);

	printk("Firmware check %x(%x)\n", RTL_R32(MCUFWDL), (RTL_R8(MCUFWDL) & MCUFWDL_RDY));
	if (RTL_R8(MCUFWDL) & MCUFWDL_RDY){
		printk("<=== Firmware Downloaded\n");
		goto check_fwdl_rdy;
	}

	wait_cnt=0;
	while(RTL_R8(RF_CTRL) & FW_DL_INPROC){
		wait_cnt++;
		delay_ms(50);
	}

#ifdef CONFIG_RTL_92D_DMDP
	if (wait_cnt==0) {
		if (priv->pshare->wlandev_idx == 0)
			RTL_W8(RF_CTRL, RTL_R8(RF_CTRL)|FW_DL_INPROC);
		else {
			if (RTL_R8(RSV_MAC0_CTRL) & MAC0_EN)
				goto check_fwdl_rdy;
			else
				RTL_W8(RF_CTRL, RTL_R8(RF_CTRL)|FW_DL_INPROC);
		}
	} else {
		if (RTL_R8(MCUFWDL) & MCUFWDL_RDY){
			printk("<=== Firmware Downloaded\n");
			RESTORE_INT(flags);
			return TRUE;
		}else{
			RTL_W8(RF_CTRL, RTL_R8(RF_CTRL)|FW_DL_INPROC);
		}
	}
#else
	if (wait_cnt==0) {
		RTL_W8(RF_CTRL, RTL_R8(RF_CTRL)|FW_DL_INPROC);
	} else {
		if (RTL_R8(MCUFWDL) & MCUFWDL_RDY){
			printk("<=== Firmware Downloaded\n");
			RESTORE_INT(flags);
			return TRUE;
		}else{
			RTL_W8(RF_CTRL, RTL_R8(RF_CTRL)|FW_DL_INPROC);
		}
	}

#endif

	if ((priv->pshare->fw_signature & 0xfff0 ) == 0x92D0)
		ptmp = data_rtl8192dfw_n_start + RT_8192CD_FIRMWARE_HDR_SIZE;
	else
		ptmp = data_rtl8192dfw_n_start;

	fw_len = (int)(data_rtl8192dfw_n_end - ptmp);
	printk("[%s][rtl8192dfw_n]\n",__FUNCTION__);

	// Disable SIC
	RTL_W16(GPIO_MUXCFG, (RTL_R16(GPIO_MUXCFG) & 0xff) | HTP_EN);
	delay_ms(1);

	// Enable MCU
	RTL_W16(SYS_FUNC_EN, (RTL_R16(SYS_FUNC_EN) & 0x0ff) | FEN_MREGEN
		| FEN_HWPDN | FEN_DIO_RF | FEN_ELDR | FEN_DCORE |FEN_CPUEN | FEN_PCIED);
	delay_ms(1);

#ifdef CONFIG_RTL8672
	RTL_W8(APS_FSMCO, RTL_R8(APS_FSMCO) | PFM_ALDN);
	delay_ms(1);  //czyao
#endif

	// Load SRAM
	WriteAddr = 0x1000;
	RTL_W8(MCUFWDL, RTL_R8(MCUFWDL) | MCUFWDL_EN);
	delay_ms(1);

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

			if(WriteAddr == 0x2000) {
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

#if defined(CONFIG_RTL865X_WTDOG) || defined(CONFIG_RTL_WTDOG)
#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
	REG32(BSP_WDTCNR) |=  1 << 23;
#elif defined(CONFIG_RTL_8198B)
	REG32(BSP_WDTCNTRR) |= BSP_WDT_KICK;
#endif
#endif

	RTL_W8(TCR+3, 0x7f);
	RTL_W8(MCUFWDL, (RTL_R8(MCUFWDL) & 0xfe) | MCUFWDL_RDY);
	delay_ms(1);
	//RTL_W8(RF_CTRL, RTL_R8(RF_CTRL) | BIT(6));
	RTL_W8(RF_CTRL, RTL_R8(RF_CTRL) & (~FW_DL_INPROC));
	delay_ms(1);

check_fwdl_rdy:

	printk("<=== %s\n", __FUNCTION__);
	// check if firmware is ready
	wait_cnt = 0;
#ifdef CONFIG_RTL_92D_DMDP
	if (priv->pshare->wlandev_idx == 0)
#endif
	{
		while (!(RTL_R8(RSV_MAC0_FWCTRL) & MAC0_WINTINI_RDY)) {
			if (++wait_cnt > 10) {
				RTL_W8(MCUFWDL, RTL_R8(MCUFWDL) & (~MCUFWDL_RDY));
				RESTORE_INT(flags);
				DEBUG_ERR("8192d mac0 firmware not ready\n");
				return FALSE;
			}
			delay_ms(2*wait_cnt);
		}
	}
#ifdef CONFIG_RTL_92D_DMDP
	else {
		while (!(RTL_R8(RSV_MAC1_FWCTRL) & MAC1_WINTINI_RDY)) {
			if (++wait_cnt > 10) {
				RTL_W8(MCUFWDL, RTL_R8(MCUFWDL) & (~MCUFWDL_RDY));
				RESTORE_INT(flags);
				DEBUG_ERR("8192d mac1 firmware not ready\n");
				return FALSE;
			}
			delay_ms(2*wait_cnt);

		}
	}
#endif
	RESTORE_INT(flags);
#ifdef CONFIG_RTL8672
	printk("val=%x\n",RTL_R8(MCUFWDL));
#endif
	return TRUE;
}


/*
 *	92DE Operation Mode
 */
void UpdateBBRFVal8192DE(struct rtl8192cd_priv *priv)
{
	u8	eRFPath = 0, curMaxRFPath;
	//u32	u4RegValue=0;

	//Update BB
	if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
		/*
		 *	5G
		 */
		//r_select_5G for path_A/B
		if (priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY)
			PHY_SetBBReg(priv, rFPGA0_XAB_RFParameter, BIT(16), 0x1);
		PHY_SetBBReg(priv, rFPGA0_XAB_RFParameter, BIT(0), 0x1);
		//rssi_table_select:index 0 for 2.4G.1~3 for 5G
		PHY_SetBBReg(priv, rOFDM0_AGCRSSITable, BIT(7) | BIT(6), 0x01);
		//5G PA power on
		if (priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY)
			PHY_SetBBReg(priv, rFPGA0_XAB_RFParameter, BIT(31), (priv->pmib->dot11RFEntry.trsw_pape_CC & BIT(6))>>6);
		PHY_SetBBReg(priv, rFPGA0_XAB_RFParameter, BIT(15), (priv->pmib->dot11RFEntry.trsw_pape_CC & BIT(4))>>4);

		//TRSW.TRSWB and PAPE2G mode table
		if (priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY){
			// TRSW_2, TRSWB_2
			PHY_SetBBReg(priv, 0x870, BIT(22)|BIT(21), 0);
			// PAPE2G_2
			PHY_SetBBReg(priv, 0x870, BIT(26), (priv->pmib->dot11RFEntry.trsw_pape_CC & BIT(7))>>7);
			PHY_SetBBReg(priv, 0x864, BIT(10), (priv->pmib->dot11RFEntry.trsw_pape_CC & BIT(6))>>6);
		}
		// TRSW_1, TRSWB_1
		PHY_SetBBReg(priv, 0x870, BIT(6)|BIT(5), 0);
		// PAPE2G_1
		PHY_SetBBReg(priv, 0x870, BIT(10), (priv->pmib->dot11RFEntry.trsw_pape_CC & BIT(5))>>5);
		PHY_SetBBReg(priv, 0x860, BIT(10), (priv->pmib->dot11RFEntry.trsw_pape_CC & BIT(4))>>4);
#ifdef RTL8192D_INT_PA
		if (!priv->pshare->rf_ft_var.use_intpa92d)
#endif
		{
			//5G PA power on
			if (priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY)
				PHY_SetBBReg(priv, rFPGA0_XAB_RFParameter, BIT(31), 0x1);
			PHY_SetBBReg(priv, rFPGA0_XAB_RFParameter, BIT(15), 0x1);
		}
		// 5G LNA on
		PHY_SetBBReg(priv, 0xb30, 0x00f00000, 0x0);
		//fc_area
		if (priv->pmib->dot11RFEntry.dot11channel<149)
			PHY_SetBBReg(priv, rOFDM1_CFOTracking, BIT(14) | BIT(13), 1);
		else
			PHY_SetBBReg(priv, rOFDM1_CFOTracking, BIT(14) | BIT(13), 2);
		//cck_disable
		PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, 0x0);
		//TX BB gain shift
#ifdef RTL8192D_INT_PA
		if (priv->pshare->rf_ft_var.use_intpa92d){
			PHY_SetBBReg(priv, rOFDM0_XATxIQImbalance, bMaskDWord, 0x2d4000b5);
			if (priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY)
				PHY_SetBBReg(priv, rOFDM0_XBTxIQImbalance, bMaskDWord, 0x2d4000b5);
		} else
#endif
		{
			PHY_SetBBReg(priv, rOFDM0_XATxIQImbalance, bMaskDWord, 0x20000080);
			if (priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY)
				PHY_SetBBReg(priv, rOFDM0_XBTxIQImbalance, bMaskDWord, 0x20000080);
		}
		// Reset IQC
		PHY_SetBBReg(priv, 0xc94, 0xF0000000, 0);
		if (priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY)
			PHY_SetBBReg(priv, 0xc9c, 0xF0000000, 0);
		//BB/DP IQC
		PHY_SetBBReg(priv, 0xb00, bMaskDWord, 0x010170b8);
		if (priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY)
			PHY_SetBBReg(priv, 0xb70, bMaskDWord, 0x010170b8);

 	} else {
		/*
		 *	2.4G
		 */
		// r_select_5G for path_A/B
		if (priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY)
			PHY_SetBBReg(priv, rFPGA0_XAB_RFParameter, BIT(16), 0x0);
		PHY_SetBBReg(priv, rFPGA0_XAB_RFParameter, BIT(0), 0);
		//rssi_table_select:index 0 for 2.4G.1~3 for 5G
		PHY_SetBBReg(priv, rOFDM0_AGCRSSITable, BIT(7) | BIT(6), 0x00);
		//5G PA power on
		if (priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY)
			PHY_SetBBReg(priv, rFPGA0_XAB_RFParameter, BIT(31), 0x0);
		PHY_SetBBReg(priv, rFPGA0_XAB_RFParameter, BIT(15), 0x0);

		//TRSW.TRSWB and PAPE2G mode table
		if (priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY){
			// TRSW_2
			PHY_SetBBReg(priv, 0x870, BIT(21), (priv->pmib->dot11RFEntry.trsw_pape_C9 & BIT(7))>>7);
			PHY_SetBBReg(priv, 0x864, BIT(5), (priv->pmib->dot11RFEntry.trsw_pape_C9 & BIT(6))>>6);
			// TRSWB_2
			PHY_SetBBReg(priv, 0x870, BIT(22), (priv->pmib->dot11RFEntry.trsw_pape_C9 & BIT(5))>>5);
			PHY_SetBBReg(priv, 0x864, BIT(6), (priv->pmib->dot11RFEntry.trsw_pape_C9 & BIT(4))>>4);
			// PAPE2G_2
			PHY_SetBBReg(priv, 0x870, BIT(26), (priv->pmib->dot11RFEntry.trsw_pape_CC & BIT(3))>>3);
			PHY_SetBBReg(priv, 0x864, BIT(10), (priv->pmib->dot11RFEntry.trsw_pape_CC & BIT(2))>>2);
		}
		// TRSW_1
		PHY_SetBBReg(priv, 0x870, BIT(5), (priv->pmib->dot11RFEntry.trsw_pape_C9 & BIT(3))>>3);
		PHY_SetBBReg(priv, 0x860, BIT(5), (priv->pmib->dot11RFEntry.trsw_pape_C9 & BIT(2))>>2);
		// TRSWB_1
		PHY_SetBBReg(priv, 0x870, BIT(6), (priv->pmib->dot11RFEntry.trsw_pape_C9 & BIT(1))>>1);
		PHY_SetBBReg(priv, 0x860, BIT(6), (priv->pmib->dot11RFEntry.trsw_pape_C9 & BIT(0))>>0);
		// PAPE2G_1
		PHY_SetBBReg(priv, 0x870, BIT(10), (priv->pmib->dot11RFEntry.trsw_pape_CC & BIT(1))>>1);
		PHY_SetBBReg(priv, 0x860, BIT(10), (priv->pmib->dot11RFEntry.trsw_pape_CC & BIT(0))>>0);

		// 5G LNA on
		PHY_SetBBReg(priv, 0xb30, 0x00f00000, 0xa);
		//fc_area
		PHY_SetBBReg(priv, rOFDM1_CFOTracking, BIT(14) | BIT(13), 0x00);
		//cck_enable
		PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, 0x1);
		//TX BB gain shift
		PHY_SetBBReg(priv, rOFDM0_XATxIQImbalance, bMaskDWord, 0x40000100);
		if (priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY)
			PHY_SetBBReg(priv, rOFDM0_XBTxIQImbalance, bMaskDWord, 0x40000100);
		// Reset IQC
		PHY_SetBBReg(priv, 0xc94, 0xF0000000, 0);
		if (priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY)
			PHY_SetBBReg(priv, 0xc9c, 0xF0000000, 0);
		//BB/DP IQC
		PHY_SetBBReg(priv, 0xb00, bMaskDWord, 0x01017038);
		if (priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY)
			PHY_SetBBReg(priv, 0xb70, bMaskDWord, 0x01017038);
	}

	//Update RF
	if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY)
		curMaxRFPath = RF92CD_PATH_B;
	else
		curMaxRFPath = RF92CD_PATH_MAX;

	for(eRFPath = RF92CD_PATH_A; eRFPath <curMaxRFPath; eRFPath++) {
		if(priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
			/*
			 *	5G
			 */
			priv->pshare->RegRF18[eRFPath] &= 0xffffff00;
			priv->pshare->RegRF18[eRFPath] |= (BIT(16)|BIT(8)|0x24); //set channel 36
			PHY_SetRFReg(priv,eRFPath, rRfChannel,bMask20Bits, priv->pshare->RegRF18[eRFPath]);
			delay_ms(1);
			// LDO_DIV
			priv->pshare->RegRF28[eRFPath] = RTL_SET_MASK(priv->pshare->RegRF28[eRFPath],BIT(7)|BIT(6),1,6);
			//PHY_SetRFReg(priv,eRFPath, 0x28, BIT(7)|BIT(6), 0x01);
			PHY_SetRFReg(priv,eRFPath, 0x28, bMask20Bits, priv->pshare->RegRF28[eRFPath]);

			delay_ms(30);
		} else {
			/*
			 *	2.4G
			 */
			priv->pshare->RegRF18[eRFPath] &=  ~(BIT(16)|BIT(8)|0xFF);
			priv->pshare->RegRF18[eRFPath] |= 1; //set channel 1.
			PHY_SetRFReg(priv,eRFPath, rRfChannel,bMask20Bits, priv->pshare->RegRF18[eRFPath]);
			delay_ms(1);
			// LDO_DIV
			priv->pshare->RegRF28[eRFPath] &= (~(BIT(7)|BIT(6)));
			//PHY_SetRFReg(priv,eRFPath, 0x28, BIT(7)|BIT(6), 0x00);
			PHY_SetRFReg(priv,eRFPath, 0x28, bMask20Bits, priv->pshare->RegRF28[eRFPath]);

			delay_ms(30);
		}
	}

#ifdef CONFIG_RTL_92D_DMDP
	if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY) {
		//Use antenna 0 & 1
		PHY_SetBBReg(priv, rOFDM0_TRxPathEnable, bMaskByte0, 0x11);
		PHY_SetBBReg(priv, rOFDM1_TRxPathEnable, bDWord, 0x1);

		//disable ad/da clock1
		if (!(DMDP_RTL_R8(0,SYS_FUNC_EN)&(FEN_BB_GLB_RST|FEN_BBRSTB))){
			DMDP_RTL_W8(0, SYS_FUNC_EN, (DMDP_RTL_R8(0,SYS_FUNC_EN)|FEN_BB_GLB_RST|FEN_BBRSTB));
		}
		DMDP_PHY_SetBBReg(0, rFPGA0_AdDaClockEn, BIT(13)|BIT(12), 3);
	} else
#endif
	{
		//Use antenna 0 & 1
		PHY_SetBBReg(priv, rOFDM0_TRxPathEnable, bMaskByte0, 0x33);
		PHY_SetBBReg(priv, rOFDM1_TRxPathEnable, bDWord, 0x3);

		//disable ad/da clock1
		PHY_SetBBReg(priv, rFPGA0_AdDaClockEn, BIT(13) | BIT(12), 0);
	}
}


#if 0 //def CLIENT_MODE
void clnt_save_IQK_res(struct rtl8192cd_priv *priv)
{
	priv->site_survey.bk_iqc[0] = PHY_QueryBBReg(priv,0xc80, bMaskDWord);
	priv->site_survey.bk_iqc[1] = PHY_QueryBBReg(priv,0xc94, bMaskByte3);
	priv->site_survey.bk_iqc[2] = PHY_QueryBBReg(priv,0xc4c, bMaskByte3);
	priv->site_survey.bk_iqc[3] = PHY_QueryBBReg(priv,0xc88, bMaskDWord);
	priv->site_survey.bk_iqc[4] = PHY_QueryBBReg(priv,0xc9c, bMaskByte3);
	priv->site_survey.bk_iqc[5] = PHY_QueryBBReg(priv,0xc14, bMaskDWord);
	priv->site_survey.bk_iqc[6] = PHY_QueryBBReg(priv,0xca0, bMaskByte3);
	priv->site_survey.bk_iqc[7] = PHY_QueryBBReg(priv,0xc1c, bMaskDWord);
	priv->site_survey.bk_iqc[8] = PHY_QueryBBReg(priv,0xc78, bMaskByte1);
	priv->site_survey.bk_iqc[9] = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x08, bMask20Bits, 1);
	priv->site_survey.bk_iqc[10] = PHY_QueryRFReg(priv, RF92CD_PATH_B, 0x08, bMask20Bits, 1);
}


void clnt_load_IQK_res(struct rtl8192cd_priv *priv)
{
	PHY_SetBBReg(priv,0xc80, bMaskDWord, priv->site_survey.bk_iqc[0]);
	PHY_SetBBReg(priv,0xc94, bMaskByte3, priv->site_survey.bk_iqc[1]);
	PHY_SetBBReg(priv,0xc4c, bMaskByte3, priv->site_survey.bk_iqc[2]);
	PHY_SetBBReg(priv,0xc88, bMaskDWord, priv->site_survey.bk_iqc[3]);
	PHY_SetBBReg(priv,0xc9c, bMaskByte3, priv->site_survey.bk_iqc[4]);
	PHY_SetBBReg(priv,0xc14, bMaskDWord, priv->site_survey.bk_iqc[5]);
	PHY_SetBBReg(priv,0xca0, bMaskByte3, priv->site_survey.bk_iqc[6]);
	PHY_SetBBReg(priv,0xc1c, bMaskDWord, priv->site_survey.bk_iqc[7]);
	PHY_SetBBReg(priv,0xc78, bMaskByte1, priv->site_survey.bk_iqc[8]);
	PHY_SetRFReg(priv,RF92CD_PATH_A, 0x08, bMask20Bits, priv->site_survey.bk_iqc[9]);
	PHY_SetRFReg(priv,RF92CD_PATH_B, 0x08, bMask20Bits, priv->site_survey.bk_iqc[10]);
}

#endif

#ifdef CONFIG_RTL_92D_DMDP
#if 0 //def CLIENT_MODE

void clnt_92D_2T_AGSwitch(struct rtl8192cd_priv * priv, int target)
{
	unsigned int flags, i;
	int rtStatus = 0;
	unsigned char temp_0522, temp_0550, temp_0551, temp_0800;
	unsigned char reg;

	SAVE_INT_AND_CLI(flags);

	/*
	 * Save MAC default value
	 */
	temp_0522 = RTL_R8(0x522);
	temp_0550 = RTL_R8(0x550);
	temp_0551 = RTL_R8(0x551);

	/*
	 *	MAC register setting
	 */
	RTL_W8(0x522, 0x3f);
	RTL_W8(0x550, temp_0550& (~BIT(3)));
	RTL_W8(0x551, temp_0551& (~BIT(3)));

	// stop BB
	PHY_SetBBReg(priv, rFPGA0_AnalogParameter4, 0x00f00000, 0xf);
	temp_0800 = PHY_QueryBBReg(priv, rFPGA0_RFMOD, 0x0f000000);
	PHY_SetBBReg(priv, rFPGA0_RFMOD, 0x0f000000, 0);

	// 5G_PAPE Select & external PA power on
	PHY_SetBBReg(priv, 0x878, BIT(0), 0);
	PHY_SetBBReg(priv, 0x878, BIT(16), 0);
	PHY_SetBBReg(priv, 0x878, BIT(15), 0);
	PHY_SetBBReg(priv, 0x878, BIT(31), 0);
	// RSSI Table Select
	PHY_SetBBReg(priv, 0xc78, BIT(7)|BIT(6), 0);
	// fc_area
	PHY_SetBBReg(priv, 0xd2c, BIT(14)|BIT(13), 0);
	// cck_enable
	PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, 0x1);
	// LDO_DIV
	PHY_SetRFReg(priv, RF92CD_PATH_A, 0x28, BIT(7)|BIT(6), 0);
	PHY_SetRFReg(priv, RF92CD_PATH_B, 0x28, BIT(7)|BIT(6), 0);
	// MOD_AG // Set channel number
	PHY_SetRFReg(priv, RF92CD_PATH_A, 0x18, BIT(16), 0);
	PHY_SetRFReg(priv, RF92CD_PATH_A, 0x18, BIT(8), 0);
	PHY_SetRFReg(priv, RF92CD_PATH_B, 0x18, BIT(16), 0);
	PHY_SetRFReg(priv, RF92CD_PATH_B, 0x18, BIT(8), 0);
 	// CLOAD for path_A
	PHY_SetRFReg(priv, RF92CD_PATH_A, 0xB, BIT(16)|BIT(15)|BIT(14), 0x7);
	PHY_SetRFReg(priv, RF92CD_PATH_B, 0xB, BIT(16)|BIT(15)|BIT(14), 0x7);

	// IMR
	PHY_SetRFReg(priv, RF92CD_PATH_A, 0x00, bMask20Bits, 0x70000);
	for (i=0;i<11;i++) {
		PHY_SetRFReg(priv, RF92CD_PATH_A, (0x2f+i), bMask20Bits, IMR_SET_N[0][i]);
	}
	PHY_SetRFReg(priv, RF92CD_PATH_A, 0x00, bMask20Bits, 0x32fff);

	// Enable BB
	PHY_SetBBReg(priv, rFPGA0_RFMOD, 0x0f000000, temp_0800);
	// IQK
	PHY_SetBBReg(priv, 0xc80, bMaskDWord, 0x40000100);
	PHY_SetBBReg(priv, 0xc94, bMaskByte3, 0);
	PHY_SetBBReg(priv, 0xc4c, bMaskByte3, 0);
	PHY_SetBBReg(priv, 0xc88, bMaskDWord, 0x40000100);
	PHY_SetBBReg(priv, 0xc9c, bMaskByte3, 0);
	PHY_SetBBReg(priv, 0xc14, bMaskDWord, 0x40000100);
	PHY_SetBBReg(priv, 0xca0, bMaskByte3, 0);
	PHY_SetBBReg(priv, 0xc1c, bMaskDWord, 0x40000100);
	PHY_SetBBReg(priv, 0xc78, bMaskByte1, 0);
	PHY_SetRFReg(priv, RF92CD_PATH_A, 0x08, bMask20Bits, 0x84000);
	PHY_SetRFReg(priv, RF92CD_PATH_B, 0x08, bMask20Bits, 0x84000);

	//Set related registers for BW config

	PHY_SetBBReg(priv, rFPGA0_AnalogParameter4, 0x00f00000, 0x0);

	/*
	 *	Reload MAC default value
	 */
	RTL_W8(0x550, temp_0550);
	RTL_W8(0x551, temp_0551);
	RTL_W8(0x522, temp_0522);

	RESTORE_INT(flags);
}

#endif



#ifdef SMART_CONCURRENT_92D
/*
 *	mode - 	0: 2x2A0->1x1A0G1 (w. IQK)
 *			1: 2x2G1->1x1A0G1 (w/o IQK)
 */
int smcc_92D_enable1x1_5G(struct rtl8192cd_priv * priv, int mode)
{
	unsigned int flags, i;
	//int rtStatus = 0;
	unsigned char temp_0522, temp_0550, temp_0551, temp_0800;
	unsigned char temp_1522, temp_1550, temp_1551;
	struct rtl8192cd_priv * priv0=(struct rtl8192cd_priv *)if_priv[0];
	unsigned char reg;
	reg = MAC_PHY_CTRL_MP;

	SAVE_INT_AND_CLI(flags);

	printk("%s\n",__FUNCTION__);
	priv0->pmib->dot11RFEntry.macPhyMode = DUALMAC_DUALPHY;
	if (mode==1){
		priv->pmib->dot11RFEntry.macPhyMode = DUALMAC_DUALPHY;
	}
	priv0->pmib->dot11RFEntry.phyBandSelect = PHY_BAND_5G;
	priv0->pshare->phw->MIMO_TR_hw_support = MIMO_1T1R;

	/*
	 * Save MAC default value
	 */
	temp_0522 = DMDP_RTL_R8(0, 0x522);
	temp_0550 = DMDP_RTL_R8(0, 0x550);
	temp_0551 = DMDP_RTL_R8(0, 0x551);
	if (mode==1){
		temp_1522 = DMDP_RTL_R8(1, 0x522);
		temp_1550 = DMDP_RTL_R8(1, 0x550);
		temp_1551 = DMDP_RTL_R8(1, 0x551);
	}

	/*
	 *	MAC register setting
	 */
	DMDP_RTL_W8(0, 0x522, 0x3f);
	DMDP_RTL_W8(0, 0x550, temp_0550& (~BIT(3)));
	DMDP_RTL_W8(0, 0x551, temp_0551& (~BIT(3)));
	if (mode==1){
		DMDP_RTL_W8(1, 0x522, 0x3f);
		DMDP_RTL_W8(1, 0x550, temp_1550& (~BIT(3)));
		DMDP_RTL_W8(1, 0x551, temp_1551& (~BIT(3)));
	}

	// Set Dual-PHY mode
	DMDP_RTL_W8(0, reg, RTL_R8(reg) | BIT(1));

	// stop BB
	DMDP_PHY_SetBBReg(0, rFPGA0_AnalogParameter4, 0x00f00000, 0xf);
	if (mode==1){
		DMDP_PHY_SetBBReg(1, rFPGA0_AnalogParameter4, 0x00f00000, 0xf);
		//temp_0800 = DMDP_PHY_QueryBBReg(0, rFPGA0_RFMOD, 0x0f000000);
		//DMDP_PHY_SetBBReg(0, rFPGA0_RFMOD, 0x0f000000, 0);
	}

	// Set as 1R
	DMDP_PHY_SetBBReg(0, 0xc04, bMaskByte0, 0x11);
	DMDP_PHY_SetBBReg(0, 0xd04, 0xf, 0x1);
	// Set ad/da clock 1
	DMDP_PHY_SetBBReg(0, 0x888, BIT(13)|BIT(12), 3);
	// Set RF as 1T1R mode
	if (mode==0){
		DMDP_PHY_SetBBReg(0, 0xc80, bMaskDWord, 0x20000080);
		DMDP_PHY_SetBBReg(0, 0xc94, 0xf0000000, 0);
		DMDP_PHY_SetBBReg(0, 0xc4c, bMaskByte3, 0);
		DMDP_PHY_SetBBReg(0, 0xc14, bMaskDWord, 0x40000100);
		DMDP_PHY_SetBBReg(0, 0xca0, 0xf0000000, 0);
		DMDP_PHY_SetRFReg(0, RF92CD_PATH_A, 0x08, bMask20Bits, 0x84000);
		// IQK
#ifdef DFS
        if (!((priv->pshare->rf_ft_var.dfsdelayiqk) &&
                (OPMODE & WIFI_AP_STATE) &&
                !priv->pmib->dot11DFSEntry.disable_DFS &&
                (timer_pending(&priv->ch_avail_chk_timer) ||
                 priv->pmib->dot11DFSEntry.disable_tx)))
#endif

		IQK_92D_5G_phy0_n(priv);

	}else{
		// 5G_PAPE Select & extenal PA power on
		DMDP_PHY_SetBBReg(0, 0x878, BIT(0), 1);
		DMDP_PHY_SetBBReg(0, 0x878, BIT(15), 1);
		DMDP_PHY_SetBBReg(0, 0x878, BIT(16), 1);

		// RSSI Table Select
		DMDP_PHY_SetBBReg(0, 0xc78, BIT(7)|BIT(6), 1);
		// fc_area
		DMDP_PHY_SetBBReg(0, 0xd2c, BIT(14)|BIT(13), (priv0->MAC_info->bb_reg[17]>>13)&0x03);
		// cck_enable
		DMDP_PHY_SetBBReg(1, rFPGA0_RFMOD, bCCKEn, 1);
		DMDP_PHY_SetBBReg(0, rFPGA0_RFMOD, bCCKEn, 0);
		// 5G LNA_On
		DMDP_PHY_SetBBReg(0, 0xb30, 0x00f00000, 0);
		// LDO_DIV
		DMDP_PHY_SetRFReg(0, RF92CD_PATH_A, 0x28, BIT(7)|BIT(6), 1);
		// MOD_AG // Set channel number
		DMDP_PHY_SetRFReg(0, RF92CD_PATH_A, 0x18, bMask20Bits, priv0->MAC_info->rfA_reg[1]);
	 	// CLOAD for path_A
		DMDP_PHY_SetRFReg(0, RF92CD_PATH_A, 0xB, bMask20Bits, priv0->MAC_info->rfA_reg[0]);

		// IMR
		DMDP_PHY_SetRFReg(0, RF92CD_PATH_A, 0x00, bMask20Bits, 0x70000);
		for (i=0;i<11;i++) {
			DMDP_PHY_SetRFReg(0, RF92CD_PATH_A, (0x2f+i), bMask20Bits, priv0->MAC_info->imr[i]);
		}
		DMDP_PHY_SetRFReg(0, RF92CD_PATH_A, 0x00, bMask20Bits, priv0->MAC_info->imr[i]);
		// Enable BB
		//DMDP_PHY_SetBBReg(0, rFPGA0_RFMOD, 0x0f000000, temp_0800);

		// IQC Setting
		DMDP_PHY_SetBBReg(0, 0xc80, bMaskDWord, priv0->MAC_info->diqc_c80_b31b0);
		DMDP_PHY_SetBBReg(0, 0xc94, bMaskByte3, priv0->MAC_info->diqc_c94_b31b24);
		DMDP_PHY_SetBBReg(0, 0xc4c, 0xf0000000, priv0->MAC_info->diqc_c4c_b31b28);
		DMDP_PHY_SetBBReg(0, 0xc14, bMaskDWord, priv0->MAC_info->diqc_c14_b31b0);
		DMDP_PHY_SetBBReg(0, 0xca0, bMaskByte3, priv0->MAC_info->diqc_ca0_b31b24);
		DMDP_PHY_SetRFReg(0, RF92CD_PATH_A, 0x08, bMask20Bits, priv0->MAC_info->loft_0A);
		//Set related registers for BW config
		DMDP_PHY_SetBBReg(0, 0x800, BIT(0), priv0->MAC_info->bb_reg[0]& BIT(0));
		DMDP_PHY_SetBBReg(0, 0x900, BIT(0), priv0->MAC_info->bb_reg[6]& BIT(0));
		DMDP_PHY_SetBBReg(0, 0xa00, BIT(4), (priv0->MAC_info->bb_reg[7]& BIT(4))>>4);
		DMDP_PHY_SetBBReg(0, 0xd00, BIT(11)|BIT(10), (priv0->MAC_info->bb_reg[15]&(BIT(11)|BIT(10)))>>10);
		DMDP_PHY_SetBBReg(0, 0x818, BIT(27)|BIT(26), (priv0->MAC_info->bb_reg[1]&(BIT(27)|BIT(26)))>>26);
		DMDP_PHY_SetBBReg(0, 0x884, BIT(11) | BIT(10), (priv0->MAC_info->bb_reg[3]&(BIT(11) | BIT(10)))>>10);
	}


	if (mode==1){
		priv0->pmib->dot11RFEntry.macPhyMode = DUALMAC_SINGLEPHY;
		priv0->pshare->phw->MIMO_TR_hw_support = MIMO_2T2R;
		priv->pmib->dot11RFEntry.macPhyMode = DUALMAC_SINGLEPHY;
		priv->pshare->phw->MIMO_TR_hw_support = MIMO_2T2R;
	}

	DMDP_PHY_SetBBReg(0, rFPGA0_AnalogParameter4, 0x00f00000, 0x0);
	if (mode==1){
		DMDP_PHY_SetBBReg(1, rFPGA0_AnalogParameter4, 0x00f00000, 0x0);
	}

	/*
	 *	Reload MAC default value
	 */
	DMDP_RTL_W8(0, 0x550, temp_0550);
	DMDP_RTL_W8(0, 0x551, temp_0551);
	DMDP_RTL_W8(0, 0x522, temp_0522);
	if (mode==1){
		DMDP_RTL_W8(1, 0x550, temp_1550);
		DMDP_RTL_W8(1, 0x551, temp_1551);
		DMDP_RTL_W8(1, 0x522, temp_1522);
	}

	RESTORE_INT(flags);
	return 0;
}


int smcc_92D_enable2x2_2G(struct rtl8192cd_priv * priv)
{
	unsigned int flags, i;
	int rtStatus = 0;
	unsigned char temp_0522, temp_0550, temp_0551, temp_0800;
	unsigned char temp_1522, temp_1550, temp_1551, temp_1800;
	unsigned char reg;
	struct rtl8192cd_priv * priv0=(struct rtl8192cd_priv *)if_priv[0];
	reg = MAC_PHY_CTRL_MP;

	SAVE_INT_AND_CLI(flags);

	printk("%s\n",__FUNCTION__);
	priv0->pmib->dot11RFEntry.macPhyMode = DUALMAC_SINGLEPHY;
	priv->pmib->dot11RFEntry.macPhyMode = DUALMAC_SINGLEPHY;
	priv0->pmib->dot11RFEntry.phyBandSelect = PHY_BAND_2G;

	/*
	 * Save MAC default value
	 */
	temp_0522 = DMDP_RTL_R8(0, 0x522);
	temp_0550 = DMDP_RTL_R8(0, 0x550);
	temp_0551 = DMDP_RTL_R8(0, 0x551);
	temp_1522 = DMDP_RTL_R8(1, 0x522);
	temp_1550 = DMDP_RTL_R8(1, 0x550);
	temp_1551 = DMDP_RTL_R8(1, 0x551);

	/*
	 *	MAC register setting
	 */
	DMDP_RTL_W8(0, 0x522, 0x3f);
	DMDP_RTL_W8(0, 0x550, temp_0550& (~BIT(3)));
	DMDP_RTL_W8(0, 0x551, temp_0551& (~BIT(3)));
	DMDP_RTL_W8(1, 0x522, 0x3f);
	DMDP_RTL_W8(1, 0x550, temp_1550& (~BIT(3)));
	DMDP_RTL_W8(1, 0x551, temp_1551& (~BIT(3)));

	// stop BB
	DMDP_PHY_SetBBReg(0, rFPGA0_AnalogParameter4, 0x00f00000, 0xf);
	DMDP_PHY_SetBBReg(1, rFPGA0_AnalogParameter4, 0x00f00000, 0xf);
	//temp_0800 = DMDP_PHY_QueryBBReg(0, rFPGA0_RFMOD, 0x0f000000);
	//DMDP_PHY_SetBBReg(0, rFPGA0_RFMOD, 0x0f000000, 0);
	//temp_1800 = DMDP_PHY_QueryBBReg(1, rFPGA0_RFMOD, 0x0f000000);
	//DMDP_PHY_SetBBReg(1, rFPGA0_RFMOD, 0x0f000000, 0);

	// Set Single-PHY mode
	DMDP_RTL_W8(0, reg, RTL_R8(reg) & (~BIT(1)));
	// Set as 2R
	DMDP_PHY_SetBBReg(0, 0xc04, bMaskByte0, 0x33);
	DMDP_PHY_SetBBReg(0, 0xd04, 0xf, 0x3);
	// Set ad/da clock 1
	DMDP_PHY_SetBBReg(0, 0x888, BIT(13)|BIT(12), 0);
	// 5G_PAPE Select & external PA power on
	DMDP_PHY_SetBBReg(0, 0x878, BIT(0), 0);
	DMDP_PHY_SetBBReg(0, 0x878, BIT(15), 0);
	DMDP_PHY_SetBBReg(0, 0x878, BIT(16), 0);
	DMDP_PHY_SetBBReg(0, 0x878, BIT(31), 0);
	// RSSI Table Select
	DMDP_PHY_SetBBReg(0, 0xc78, BIT(7)|BIT(6), 0);
	// fc_area
	DMDP_PHY_SetBBReg(0, 0xd2c, BIT(14)|BIT(13), 0);
	// cck_enable
	DMDP_PHY_SetBBReg(1, rFPGA0_RFMOD, bCCKEn, 0);
	DMDP_PHY_SetBBReg(0, rFPGA0_RFMOD, bCCKEn, 1);
	// 5G LNA_On
	DMDP_PHY_SetBBReg(0, 0xb30, 0x00f00000, 0xa);
	// LDO_DIV
	DMDP_PHY_SetRFReg(0, RF92CD_PATH_A, 0x28, BIT(7)|BIT(6), 0);
	// MOD_AG // Set channel number
	DMDP_PHY_SetRFReg(0, RF92CD_PATH_A, 0x18, bMask20Bits, priv->MAC_info->rfA_reg[1]);
 	// CLOAD for path_A
	DMDP_PHY_SetRFReg(0, RF92CD_PATH_A, 0xB, BIT(16)|BIT(15)|BIT(14), 0x7);

	// IMR
	DMDP_PHY_SetRFReg(0, RF92CD_PATH_A, 0x00, bMask20Bits, 0x70000);
	for (i=0;i<11;i++) {
		DMDP_PHY_SetRFReg(0, RF92CD_PATH_A, (0x2f+i), bMask20Bits, priv->MAC_info->imr[i]);
	}
	DMDP_PHY_SetRFReg(0, RF92CD_PATH_A, 0x00, bMask20Bits, priv->MAC_info->imr[i]);

	// Enable BB
	//DMDP_PHY_SetBBReg(0, rFPGA0_RFMOD, 0x0f000000, temp_0800);
	//DMDP_PHY_SetBBReg(1, rFPGA0_RFMOD, 0x0f000000, temp_1800);
	// IQK
	DMDP_PHY_SetBBReg(0, 0xc80, bMaskDWord, 0x40000100);
	DMDP_PHY_SetBBReg(0, 0xc94, bMaskByte3, 0);
	DMDP_PHY_SetBBReg(0, 0xc4c, bMaskByte3, 0);
	DMDP_PHY_SetBBReg(0, 0xc88, bMaskDWord, 0x40000100);
	DMDP_PHY_SetBBReg(0, 0xc9c, bMaskByte3, 0);
	DMDP_PHY_SetBBReg(0, 0xc14, bMaskDWord, 0x40000100);
	DMDP_PHY_SetBBReg(0, 0xca0, bMaskByte3, 0);
	DMDP_PHY_SetBBReg(0, 0xc1c, bMaskDWord, 0x40000100);
	DMDP_PHY_SetBBReg(0, 0xc78, bMaskByte1, 0);
	DMDP_PHY_SetRFReg(0, RF92CD_PATH_A, 0x08, bMask20Bits, 0x84000);
	IQK_92D_2G(priv0);
	//Set related registers for BW config
	DMDP_PHY_SetBBReg(0, 0x800, BIT(0), priv->MAC_info->bb_reg[0]& BIT(0));
	DMDP_PHY_SetBBReg(0, 0x900, BIT(0), priv->MAC_info->bb_reg[6]& BIT(0));
	DMDP_PHY_SetBBReg(0, 0xa00, BIT(4), (priv->MAC_info->bb_reg[7]& BIT(4))>>4);
	DMDP_PHY_SetBBReg(0, 0xd00, BIT(11)|BIT(10), (priv->MAC_info->bb_reg[15]&(BIT(11)|BIT(10)))>>10);
	DMDP_PHY_SetBBReg(0, 0x818, BIT(27)|BIT(26), (priv->MAC_info->bb_reg[1]&(BIT(27)|BIT(26)))>>26);
	DMDP_PHY_SetBBReg(0, 0x884, BIT(11) | BIT(10), (priv->MAC_info->bb_reg[3]&(BIT(11) | BIT(10)))>>10);

	DMDP_PHY_SetBBReg(0, rFPGA0_AnalogParameter4, 0x00f00000, 0x0);
	DMDP_PHY_SetBBReg(1, rFPGA0_AnalogParameter4, 0x00f00000, 0x0);

	/*
	 *	Reload MAC default value
	 */
	DMDP_RTL_W8(0, 0x550, temp_0550);
	DMDP_RTL_W8(0, 0x551, temp_0551);
	DMDP_RTL_W8(0, 0x522, temp_0522);
	DMDP_RTL_W8(1, 0x550, temp_1550);
	DMDP_RTL_W8(1, 0x551, temp_1551);
	DMDP_RTL_W8(1, 0x522, temp_1522);

	RESTORE_INT(flags);
	return 0;
}

#if 0
int smcc_92D_enable2x2_5G(struct rtl8192cd_priv * priv)
{
	unsigned int flags, val=0;
	//int rtStatus = 0;
	unsigned char temp_522, temp_550, temp_551;
	unsigned char reg;
	reg = MAC_PHY_CTRL_MP;

	SAVE_INT_AND_CLI(flags);
	priv->pmib->dot11RFEntry.macPhyMode = DUALMAC_SINGLEPHY;
	priv->pshare->phw->MIMO_TR_hw_support = MIMO_2T2R;

	/*
	 * Save MAC default value
	 */
	temp_522 = RTL_R8(0x522);
	temp_550 = RTL_R8(0x550);
	temp_551 = RTL_R8(0x551);

	/*
	 *	MAC register setting
	 */
	RTL_W8(0x522, 0x3f);
	RTL_W8(0x550, RTL_R8(0x550)& (~BIT(3)));
	RTL_W8(0x551, RTL_R8(0x551)& (~BIT(3)));

	// stop BB
	PHY_SetBBReg(priv, rFPGA0_AnalogParameter4, 0x00f00000, 0xf);

	// Set Single-PHY mode
	RTL_W8(reg, RTL_R8(reg) & (~BIT(1)));
	// Set as 2R
	PHY_SetBBReg(priv, 0xc04, bMaskByte0, 0x33);
	PHY_SetBBReg(priv, 0xd04, 0xf, 0x3);
	// Set ad/da clock 1
	PHY_SetBBReg(priv, 0x888, BIT(13)|BIT(12), 0);
	// Set RF as 2T2R mode
	PHY_SetRFReg(priv, RF92CD_PATH_A, 0x38, BIT(12), 1);
	// Reload AGC table
	/*
	rtStatus = PHY_ConfigBBWithParaFile(priv, AGCTAB);
	if (rtStatus) {
		printk("phy_BB8192CD_Config_ParaFile(): Write BB AGC Table Fail!!\n");
		RESTORE_INT(flags);
		return -1;
	}
	*/
	// 5G_PAPE Select & external PA power on
	PHY_SetBBReg(priv, 0x878, BIT(15), 1);
	PHY_SetBBReg(priv, 0x878, BIT(31), 1);
	// 1.5V_LDO
	RTL_W32(0x14, ((RTL_R32(0x14)&0xff0fffff)|0x00d00000));
	// LDO_DIV
	PHY_SetRFReg(priv, RF92CD_PATH_B, 0x28, BIT(7)|BIT(6), 1);
	// A/G mode LO buffer
	PHY_SetRFReg(priv, RF92CD_PATH_B, 0x38, BIT(16)|BIT(15)|BIT(14), 3);
	// MOD_AG
	// Set channel number
	val = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x18, bMask20Bits, 1);
	PHY_SetRFReg(priv, RF92CD_PATH_B, 0x18, bMask20Bits, val);
	// IMR parameter for path_A/B
	SetIMR(priv, priv->pmib->dot11RFEntry.dot11channel);
	PHY_SetBBReg(priv, 0xc80, bMaskDWord, 0x20000080);
	PHY_SetBBReg(priv, 0xc94, 0xf0000000, 0);
	PHY_SetBBReg(priv, 0xc4c, bMaskByte3, 0);
	PHY_SetBBReg(priv, 0xc14, bMaskDWord, 0x40000100);
	PHY_SetBBReg(priv, 0xca0, 0xf0000000, 0);
	PHY_SetBBReg(priv, 0xc88, bMaskDWord, 0x20000080);
	PHY_SetBBReg(priv, 0xc9c, 0xf0000000, 0);
	PHY_SetBBReg(priv, 0xc1c, bMaskDWord, 0x40000100);
	PHY_SetBBReg(priv, 0xc78, 0x0000f000, 0);
	PHY_SetBBReg(priv, 0xc78, BIT(7)|BIT(6), 1);
	PHY_SetRFReg(priv, RF92CD_PATH_A, 0x08, bMask20Bits, 0x84000);
	PHY_SetRFReg(priv, RF92CD_PATH_B, 0x08, bMask20Bits, 0x84000);

	// IQK
	IQK_92D_5G_n(priv);
	//Set related registers for BW config
	SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);

	// Enable BB
	PHY_SetBBReg(priv, rFPGA0_AnalogParameter4, 0x00f00000, 0x0);

	/*
	 *	Reload MAC default value
	 */
	RTL_W8(0x550, temp_550);
	RTL_W8(0x551, temp_551);
	RTL_W8(0x522, temp_522);

	RESTORE_INT(flags);
	return 0;
}

int smcc_92D_enable2x2_2G(struct rtl8192cd_priv * priv)
{
	unsigned int flags;
	int rtStatus = 0;
	unsigned char temp_522, temp_550, temp_551;
	unsigned char reg;
	reg = MAC_PHY_CTRL_MP;

	SAVE_INT_AND_CLI(flags);
	priv->pmib->dot11RFEntry.macPhyMode = DUALMAC_SINGLEPHY;

	/*
	 * Save MAC default value
	 */
	temp_522 = RTL_R8(0x522);
	temp_550 = RTL_R8(0x550);
	temp_551 = RTL_R8(0x551);

	/*
	 *	MAC register setting
	 */
	RTL_W8(0x522, 0x3f);
	RTL_W8(0x550, RTL_R8(0x550)& (~BIT(3)));
	RTL_W8(0x551, RTL_R8(0x551)& (~BIT(3)));

	// stop BB
	PHY_SetBBReg(priv, rFPGA0_AnalogParameter4, 0x00f00000, 0xf);

	// Set Single-PHY mode
	RTL_W8(reg, RTL_R8(reg) & (~BIT(1)));
	// Set as 2R
	PHY_SetBBReg(priv, 0xc04, bMaskByte0, 0x33);
	PHY_SetBBReg(priv, 0xd04, 0xf, 0x3);
	// Set ad/da clock 1
	PHY_SetBBReg(priv, 0x888, BIT(13)|BIT(12), 0);
	// Set RF as 2T2R mode
	PHY_SetRFReg(priv, RF92CD_PATH_A, 0x38, BIT(12), 1);
	// Reload AGC table
	rtStatus = PHY_ConfigBBWithParaFile(priv, AGCTAB);
	if (rtStatus) {
		printk("phy_BB8192CD_Config_ParaFile(): Write BB AGC Table Fail!!\n");
		RESTORE_INT(flags);
		return -1;
	}
	// 5G_PAPE Select & external PA power on
	PHY_SetBBReg(priv, 0x878, BIT(0), 0);
	PHY_SetBBReg(priv, 0x878, BIT(15), 0);
	PHY_SetBBReg(priv, 0x878, BIT(16), 0);
	PHY_SetBBReg(priv, 0x878, BIT(31), 0);
	// RSSI Table Select
	PHY_SetBBReg(priv, 0xc78, BIT(7)|BIT(6), 0);
	// fc_area
	PHY_SetBBReg(priv, 0xd2c, BIT(14)|BIT(13), 0);
	// cck_enable
	PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, 0x1);;
	//AGC trsw threshold
	PHY_SetBBReg(priv, 0xc70, 0x007F0000, 0x7f);
	// 1.5V_LDO
	RTL_W32(0x14, ((RTL_R32(0x14)&0xff0fffff)|0x00700000));
	// LDO_DIV
	PHY_SetRFReg(priv, RF92CD_PATH_B, 0x28, BIT(7)|BIT(6), 1);
	// A/G mode LO buffer
	PHY_SetRFReg(priv, RF92CD_PATH_B, 0x38, BIT(16)|BIT(15)|BIT(14), 4);
	// MOD_AG
	// Set channel number
	SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
	SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);
	// IQK
	IQK_92D_2G(priv);
	//Set related registers for BW config


	// Enable BB
	PHY_SetBBReg(priv, rFPGA0_AnalogParameter4, 0x00f00000, 0x0);

	/*
	 *	Reload MAC default value
	 */
	RTL_W8(0x550, temp_550);
	RTL_W8(0x551, temp_551);
	RTL_W8(0x522, temp_522);

	RESTORE_INT(flags);
	return 0;
}

int smcc_92D_enable1x1_2G(struct rtl8192cd_priv * priv)
{
	struct rtl8192cd_priv *priv_phy0 = (struct rtl8192cd_priv *)if_priv[0];
	unsigned int flags;
	int rtStatus = 0;
	unsigned char temp_522, temp_550, temp_551;
	int i;
	unsigned char reg;
	reg = MAC_PHY_CTRL_MP;

	SAVE_INT_AND_CLI(flags);
	printk("%s %d %x\n",__FUNCTION__, __LINE__, PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x18, bMask20Bits,1));
	priv->pmib->dot11RFEntry.macPhyMode = DUALMAC_DUALPHY;
	priv->pshare->phw->MIMO_TR_hw_support = MIMO_1T1R;

	/*
	 * Save MAC default value
	 */
	temp_522 = RTL_R8(0x522);
	temp_550 = RTL_R8(0x550);
	temp_551 = RTL_R8(0x551);

	/*
	 *	MAC register setting
	 */
	RTL_W8(0x522, 0x3f);
	RTL_W8(0x550, RTL_R8(0x550)& (~BIT(3)));
	RTL_W8(0x551, RTL_R8(0x551)& (~BIT(3)));

	// stop BB
	PHY_SetBBReg(priv, rFPGA0_AnalogParameter4, 0x00f00000, 0xf);

	// Set Dual-PHY mode
	RTL_W8(reg, RTL_R8(reg) | BIT(1));
	// Set as 1R
	DMDP_PHY_SetBBReg(0, 0xc04, bMaskByte0, 0x11);
	DMDP_PHY_SetBBReg(0, 0xd04, 0xf, 0x1);
	// Set ad/da clock 1
	DMDP_PHY_SetBBReg(0, 0x888, BIT(13)|BIT(12), 3);
	// Set RF as 2T2R mode
	DMDP_PHY_SetRFReg(0, RF92CD_PATH_A, 0x38, BIT(12), 0);
	// Reload AGC table
	rtStatus = PHY_ConfigBBWithParaFile(priv_phy0, AGCTAB);
	if (rtStatus) {
		printk("phy_BB8192CD_Config_ParaFile(): Write BB AGC Table Fail!!\n");
		RESTORE_INT(flags);
		return -1;
	}
	// 5G_PAPE Select & extenal PA power on
	DMDP_PHY_SetBBReg(0, 0x878, BIT(0), 1);
	DMDP_PHY_SetBBReg(0, 0x878, BIT(15), 1);
	// RSSI Table Select
	DMDP_PHY_SetBBReg(0, 0xc78, BIT(7)|BIT(6), 1);
	// fc_area
	DMDP_PHY_SetBBReg(0, 0xd2c, BIT(14)|BIT(13), 1);
	// cck_enable
	DMDP_PHY_SetBBReg(0, rFPGA0_RFMOD, bCCKEn, 0);
	// AGC trsw threshold
	DMDP_PHY_SetBBReg(0, 0xc70, 0x007F0000, 0x4e);
	// 1.5V_LDO
	DMDP_RTL_W32(0, 0x14, (DMDP_RTL_R32(0, 0x14)&0xff0fffff)|0x00d00000);
	// LDO_DIV
	DMDP_PHY_SetRFReg(0, RF92CD_PATH_A, 0x28, BIT(7)|BIT(6), 1);
	PHY_SetRFReg(priv, RF92CD_PATH_A, 0x28, BIT(7)|BIT(6), 0);
	// A/G mode LO buffer
	DMDP_PHY_SetRFReg(0, RF92CD_PATH_A, 0x38, BIT(16)|BIT(15)|BIT(14), 3);
	PHY_SetRFReg(priv, RF92CD_PATH_A, 0x38, BIT(16)|BIT(15)|BIT(14), 4);
	// PHY0 MOD_AG //Set channel number
	DMDP_PHY_SetRFReg(0, RF92CD_PATH_A, rRfChannel, bMask20Bits, priv_phy0->MAC_info->rfA_reg[0]);
	// PHY1 MOD_AG //Set channel number
	// PHY_SetRFReg(priv, RF92CD_PATH_A, rRfChannel, bMask20Bits, priv->MAC_info->rfA_reg[0]);
	// PHY0 IMR Path A
	DMDP_PHY_SetRFReg(0, RF92CD_PATH_A, 0x00, bMask20Bits, 0x72c15);
	for (i=0; i<10; i++) {
		if (i==8){
			DMDP_PHY_SetRFReg(0, RF92CD_PATH_A, (0x30+i), bMask12Bits, priv_phy0->MAC_info->imr_rfA[i]);
		} else if (i==9) {
			DMDP_PHY_SetRFReg(0, RF92CD_PATH_A, (0x30+i), bMask20Bits, priv_phy0->MAC_info->imr_rfA[i]);
		} else {
			DMDP_PHY_SetRFReg(0, RF92CD_PATH_A, (0x2f+i), bMask20Bits, priv_phy0->MAC_info->imr_rfA[i]);
		}
	}
	DMDP_PHY_SetRFReg(0, RF92CD_PATH_A, 0x00, bMask20Bits, 0x32c15);
	// PHY1 IMR Path A
	//PHY_SetRFReg(priv, RF92CD_PATH_A, 0x00, bMask20Bits, 0x72c15);
	//for (i=0; i<10; i++) {
	//	PHY_SetRFReg(priv, RF92CD_PATH_A, (0x2f+i), bMask20Bits, priv->MAC_info->imr_rfA[i]);
	//}
	//PHY_SetRFReg(priv, RF92CD_PATH_A, 0x00, bMask20Bits, 0x32c15);
	// PHY0 IQC
	DMDP_PHY_SetBBReg(0, 0xc80, bMaskDWord, priv_phy0->MAC_info->diqc_c80_b31b0);
	DMDP_PHY_SetBBReg(0, 0xc94, 0xf0000000, priv_phy0->MAC_info->diqc_c94_b31b24);
	DMDP_PHY_SetBBReg(0, 0xc4c, bMaskByte3, priv_phy0->MAC_info->diqc_c4c_b31b28);
	DMDP_PHY_SetBBReg(0, 0xc14, bMaskDWord, priv_phy0->MAC_info->diqc_c14_b31b0);
	DMDP_PHY_SetBBReg(0, 0xca0, 0xf0000000, priv_phy0->MAC_info->diqc_ca0_b31b24);
	DMDP_PHY_SetRFReg(0, RF92CD_PATH_A, 0x08, bMask20Bits, priv_phy0->MAC_info->loft_0A);
	//  PHY0 BB Enable
	DMDP_PHY_SetBBReg(0, rFPGA0_AnalogParameter4, 0x00f00000, 0x0);
	/*
	 *	Reload PHY0 MAC default value
	 */
	DMDP_RTL_W8(0, 0x550, temp_550);
	DMDP_RTL_W8(0, 0x551, temp_551);
	DMDP_RTL_W8(0, 0x522, temp_522);

	// PHY1 IQK
	PHY_SetBBReg(priv, 0xc80, bMaskDWord, 0x40000100);
	PHY_SetBBReg(priv, 0xc94, 0xf0000000, 0);
	PHY_SetBBReg(priv, 0xc4c, bMaskByte3, 0);
	PHY_SetBBReg(priv, 0xc14, bMaskDWord, 0x40000100);
	PHY_SetBBReg(priv, 0xca0, 0xf0000000, 0);
	PHY_SetRFReg(priv, RF92CD_PATH_A, 0x08, bMask20Bits, 0x84000);

	IQK_92D_2G_phy1(priv);

	PHY_SetBBReg(priv, rFPGA0_AnalogParameter4, 0x00f00000, 0x0);

	/*
	 *	Reload MAC default value
	 */
	RTL_W8(0x550, temp_550);
	RTL_W8(0x551, temp_551);
	RTL_W8(0x522, temp_522);

	printk("%s %d %x\n",__FUNCTION__, __LINE__, PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x18, bMask20Bits,1));

	RESTORE_INT(flags);
	return 0;
}

#endif

void smcc_92D_fill_MAC_info(struct rtl8192cd_priv * priv, struct SMCC_MAC_Info_Tbl *info_tbl)
{
	int i, val, imr_idx = 0;

	unsigned int BB_IDX[18] = {0x800, 0x818, 0x878, 0x884, 0x888, 0x88C, 0x900, 0xA00, 0xC04,
								0xC4C, 0xC70, 0xC78, 0xC94, 0xC9C, 0xCA0, 0xD00, 0xD04, 0xD2C};
	unsigned int RF_IDX[3] = {0x0B, 0x18, 0x28};

	info_tbl->channel = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x18, bMaskByte0, 1);;
	info_tbl->bandwidth = priv->pshare->CurrentChannelBW==HT_CHANNEL_WIDTH_20_40  ? 1 : 0;
	if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20_40) {
		if (priv->pshare->offset_2nd_chan == 1)
			info_tbl->bandwidth |= BIT(2); // control upper, 2nd below
		else
			info_tbl->bandwidth |= BIT(1); // control lower, 2nd upper
	}
	if (priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY) {
		// Single PHY IQC 		Byte 2~22
		info_tbl->siqc_c80_b31b0 = DMDP_PHY_QueryBBReg(0, 0xc80, bMaskDWord);
		info_tbl->siqc_c94_b31b24 = DMDP_PHY_QueryBBReg(0, 0xc94, bMaskByte3);
		info_tbl->siqc_c4c_b31b24 = DMDP_PHY_QueryBBReg(0, 0xc4c, bMaskByte3);
		info_tbl->siqc_c88_b31b0 = DMDP_PHY_QueryBBReg(0, 0xc88, bMaskDWord);
		info_tbl->siqc_c9c_b31b24 = DMDP_PHY_QueryBBReg(0, 0xc9c, bMaskByte3);
		info_tbl->siqc_c14_b31b0 = DMDP_PHY_QueryBBReg(0, 0xc14, bMaskDWord);
		info_tbl->siqc_ca0_b31b24 = DMDP_PHY_QueryBBReg(0, 0xca0, bMaskByte3);
		info_tbl->siqc_c1c_b31b0 = DMDP_PHY_QueryBBReg(0, 0xc1c, bMaskDWord);
		info_tbl->siqc_c78_b15b8 = DMDP_PHY_QueryBBReg(0, 0xc78, bMaskByte1);
	}else{
		// Dual PHY IQC		Byte 23~33
		info_tbl->diqc_c80_b31b0 = PHY_QueryBBReg(priv, 0xc80, bMaskDWord);
		info_tbl->diqc_c94_b31b24 = PHY_QueryBBReg(priv, 0xc94, bMaskByte3);
		info_tbl->diqc_c4c_b31b28 = PHY_QueryBBReg(priv, 0xc4c, 0xf0000000);
		info_tbl->diqc_c14_b31b0 = PHY_QueryBBReg(priv, 0xc14, bMaskDWord);
		info_tbl->diqc_ca0_b31b24 = PHY_QueryBBReg(priv, 0xca0, bMaskByte3);
	}

	if (priv->pmib->dot11RFEntry.macPhyMode != DUALMAC_DUALPHY) {
		// 0_B LOFT			Byte 115
		info_tbl->loft_0B = DMDP_PHY_QueryRFReg(0, RF92CD_PATH_B, 0x08, bMask20Bits, 1);
		// RF_reg			Byte 206~217
		for (i = 0; i<3; i++)
			info_tbl->rfB_reg[i] = DMDP_PHY_QueryRFReg(0, RF92CD_PATH_B, RF_IDX[i], bMask20Bits, 1);
		//
	}	else {
		val = info_tbl->channel;
		if (priv->pmib->dot11RFEntry.phyBandSelect==PHY_BAND_2G)
			imr_idx = 0;
		else {
			if (val>=36 && val <=64)
				imr_idx = 1;
			else
				imr_idx = 2;
		}
		// IMR				Byte 34~73
		for (i = 0; i<11; i++)
			info_tbl->imr[i] = IMR_SET_N[imr_idx][i];
		if (imr_idx==0)
			info_tbl->imr[11] = 0x32fff;
		else
			info_tbl->imr[11] = 0x32c9a;

		// 0_A LOFT			Byte 114
		info_tbl->loft_0A = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x08, bMask20Bits, 1);
		// BB_reg			Byte 122~193
		for (i = 0; i<18; i++)
			info_tbl->bb_reg[i] = PHY_QueryBBReg(priv, BB_IDX[i], bMaskDWord);
		// RF_reg			Byte 194~205
		for (i = 0; i<3; i++)
			info_tbl->rfA_reg[i] = PHY_QueryRFReg(priv, RF92CD_PATH_A, RF_IDX[i], bMask20Bits, 1);
	}
}

void smcc_dump_MAC_info(struct rtl8192cd_priv * priv, struct SMCC_MAC_Info_Tbl *info_tbl)
{
	int i, flags;

	unsigned int BB_IDX[18] = {0x800, 0x818, 0x878, 0x884, 0x888, 0x88C, 0x900, 0xA00, 0xC04,
								0xC4C, 0xC70, 0xC78, 0xC94, 0xC9C, 0xCA0, 0xD00, 0xD04, 0xD2C};
	unsigned int RF_IDX[3] = {0x0B, 0x18, 0x28};

	SAVE_INT_AND_CLI(flags);

	printk("info_tbl->channel = %d \n", info_tbl->channel);
	printk("info_tbl->bandwidth = %x \n", info_tbl->bandwidth);
	printk("info_tbl->siqc_c80_b31b0 = %x \n", info_tbl->siqc_c80_b31b0);
	printk("info_tbl->siqc_c94_b31b24 = %x \n", info_tbl->siqc_c94_b31b24);
	printk("info_tbl->siqc_c4c_b31b24 = %x \n", info_tbl->siqc_c4c_b31b24);
	printk("info_tbl->siqc_c88_b31b0 = %x \n", info_tbl->siqc_c88_b31b0);
	printk("info_tbl->siqc_c9c_b31b24 = %x \n", info_tbl->siqc_c9c_b31b24);
	printk("info_tbl->siqc_c14_b31b0 = %x \n", info_tbl->siqc_c14_b31b0);
	printk("info_tbl->siqc_ca0_b31b24 = %x \n", info_tbl->siqc_ca0_b31b24);
	printk("info_tbl->siqc_c1c_b31b0 = %x \n", info_tbl->siqc_c1c_b31b0);
	printk("info_tbl->siqc_c78_b15b8 = %x \n", info_tbl->siqc_c78_b15b8);
	printk("info_tbl->diqc_c80_b31b0 = %x \n", info_tbl->diqc_c80_b31b0);
	printk("info_tbl->diqc_c94_b31b24 = %x \n", info_tbl->diqc_c94_b31b24);
	printk("info_tbl->diqc_c4c_b31b28 = %x \n", info_tbl->diqc_c4c_b31b28);
	printk("info_tbl->diqc_c14_b31b0 = %x \n", info_tbl->diqc_c14_b31b0);
	printk("info_tbl->diqc_ca0_b31b24 = %x \n", info_tbl->diqc_ca0_b31b24);

	for (i = 0; i<12; i++)
		printk("info_tbl->imr[%d] = %x \n", i, info_tbl->imr[i]);

	printk("info_tbl->loft_0A = %x \n", info_tbl->loft_0A);
	printk("info_tbl->loft_0B = %x \n", info_tbl->loft_0B);

	for (i = 0; i<18; i++)
		printk("info_tbl->bb_reg[%d](0x%2x) = %x \n", i, BB_IDX[i], info_tbl->bb_reg[i]);
	for (i = 0; i<3; i++)
		printk("info_tbl->rfA_reg[%d](0x%2x) = %x \n", i, RF_IDX[i], info_tbl->rfA_reg[i]);
	for (i = 0; i<3; i++)
		printk("info_tbl->rfB_reg[%d](0x%2x) = %x \n", i, RF_IDX[i], info_tbl->rfB_reg[i]);


	RESTORE_INT(flags);
}

void smcc_signin_MAC_info(struct rtl8192cd_priv * priv, struct SMCC_MAC_Info_Tbl *info_tbl)
{
	unsigned long flags;
	unsigned int content = 0, info_idx;
	int count = 10;

	SAVE_INT_AND_CLI(flags);

	RTL_W8(0x422, RTL_R8(0x422)&(~BIT(6)));
	RTL_W8(0x662, RTL_R8(0x662)&(~BIT(0)));

	signin_beacon_desc(priv, (unsigned int *)info_tbl, sizeof(struct SMCC_MAC_Info_Tbl));

	/*
	 * BCN_HEAD
	 */
	content |= (RTL_R16(TDECTRL)>>8)<<16;

	/*
	 * Info_idx: Test chip = 0; MP chip = 1
	 */
 	info_idx = 0;
	content |= info_idx << 8;

	/*
	 * set cmd id
	 */
	content |= H2C_CMD_INFO_PKT;

	signin_h2c_cmd(priv, content, 0);

	while (count > 0){
		if (RTL_R8(0x662) & BIT(0)) {
			RTL_W8(0x422, RTL_R8(0x422)|BIT(6));
			printk("SMCC signin MAC info success!\n");
			break;
		}
		count--;
		delay_ms(5);
	}

	if (count <= 0)
		printk("SMCC signin MAC info FAIL!\n");

	#if 0
		/*
		 *	MAC register setting
		 */
		RTL_W8(0x522, 0x3f);
		RTL_W8(0x550, RTL_R8(0x550)& (~BIT(3)));
		RTL_W8(0x551, RTL_R8(0x551)& (~BIT(3)));

		// stop BB
		PHY_SetBBReg(priv, rFPGA0_AnalogParameter4, 0x00f00000, 0xf);
	#endif

	RESTORE_INT(flags);
}

/*
 En : 1 for enable, 0 for disable
 DMDP_Duration : valid while Mode = 1. The unit is 4ms.
 MODE : 00b for static mode , 01b for preserve mode, 10b and 11b are reserved
 PSV : valid while mode = 1. set 1 for RF resource preservation request
 AP / STA : 0 for AP, 1 for STA
 LINK_STATE : valid while Mode = 0.  0 for no any link, 1 for link exist
 */
void smcc_signin_linkstate(struct rtl8192cd_priv * priv, unsigned char enable, unsigned char duration, unsigned char link_state)
{
	unsigned long flags;
	unsigned int content = 0;

	printk(">>>> [wlan-%d] %s en=%d, du=%d, st=%d.\n", priv->pshare->wlandev_idx, __FUNCTION__, enable, duration, link_state);

	SAVE_INT_AND_CLI(flags);

	// Link State
	content |= (link_state & BIT(0))<< 21;

	// Enable
	content |= (enable & BIT(0)) << 16;

	// DMDP_Duration
	content |= duration << 8;
	// set cmd id
	content |= H2C_CMD_SMCC;

	signin_h2c_cmd(priv, content, 0);

	RESTORE_INT(flags);
}

#endif // SMART_CONCURRENT_92D
#endif //CONFIG_RTL_92D_DMDP

#endif // CONFIG_RTL_92D_SUPPORT



