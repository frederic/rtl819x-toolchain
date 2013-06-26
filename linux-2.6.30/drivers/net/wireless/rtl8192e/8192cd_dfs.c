/*
 *   Handling routines for DFS (Dynamic Frequency Selection) functions
 *
 *  $Id: 8192cd_dfs.c,v 1.1 2012/05/04 12:49:07 jimmylin Exp $
 *
 *  Copyright (c) 2012 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_DFS_C_

#include "./8192cd_cfg.h"
#include "./8192cd.h"
#include "./8192cd_headers.h"
#include "./8192cd_debug.h"


#ifdef DFS

void rtl8192cd_dfs_chk_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	if (timer_pending(&priv->dfs_chk_timer)){
		del_timer_sync(&priv->dfs_chk_timer);
	}
	
	PHY_SetBBReg(priv, 0xcdc, BIT(8)|BIT(9), 1);
	PRINT_INFO("DFS CP END.\n");
}


void rtl8192cd_DFS_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	unsigned int radar_type = 0;	/* 0 for short, 1 for long */
	unsigned int dfs_chk;
	int tp_th = ((priv->pshare->is_40m_bw)?45:20);

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

#ifdef PCIE_POWER_SAVING
	if ((priv->pwr_state == L2) || (priv->pwr_state == L1))
		goto exit_timer;
#endif
	if (((priv->ext_stats.tx_avarage+priv->ext_stats.rx_avarage)>>17)>tp_th){
		dfs_chk = 1;
		PHY_SetBBReg(priv, 0xcdc, BIT(8)|BIT(9), 0);
	}else{
		dfs_chk = 0;
		PHY_SetBBReg(priv, 0xcdc, BIT(8)|BIT(9), 1);
	}

	if (!(priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_ETSI))
	{
		if (PHY_QueryBBReg(priv, 0xcf8, BIT(31))) {
			radar_type++;
			priv->pmib->dot11DFSEntry.DFS_detected = 1;
		}
	}

	if (PHY_QueryBBReg(priv, 0xcf8, BIT(23)))
		priv->pmib->dot11DFSEntry.DFS_detected = 1;

	/*
	 *	DFS debug mode for logo test
	 */
	if (!priv->pmib->dot11DFSEntry.disable_DFS && priv->pshare->rf_ft_var.dfsdbgmode 
		&& priv->pmib->dot11DFSEntry.DFS_detected) {
		if ((jiffies - priv->pshare->rf_ft_var.dfsrctime)>RTL_SECONDS_TO_JIFFIES(10))
			priv->pshare->rf_ft_var.dfsdbgcnt = 1;
		else
			priv->pshare->rf_ft_var.dfsdbgcnt++;
		panic_printk("[%d] DFS dbg mode, Radar is detected as %x%x (%d)!\n", priv->pshare->rf_ft_var.dfsdbgcnt,
			radar_type, PHY_QueryBBReg(priv, 0xcf4, bMaskDWord)&0xffff, (unsigned int)RTL_JIFFIES_TO_MILISECONDS(jiffies));
		priv->pshare->rf_ft_var.dfsrctime = jiffies;
		priv->pmib->dot11DFSEntry.DFS_detected = 0;
		PHY_SetBBReg(priv, 0xc84, BIT(25), 0);
		PHY_SetBBReg(priv, 0xc84, BIT(25), 1);
		goto exit_timer;
	}

	if (!priv->pmib->dot11DFSEntry.disable_DFS && priv->pmib->dot11DFSEntry.DFS_detected) {

		PRINT_INFO("Radar is detected as %x%x (%d)!\n", radar_type, PHY_QueryBBReg(priv, 0xcf4, bMaskDWord)&0xffff, RTL_JIFFIES_TO_MILISECONDS(jiffies));
		
		if (timer_pending(&priv->dfs_chk_timer)){
			del_timer_sync(&priv->dfs_chk_timer);
			PHY_SetBBReg(priv, 0xcdc, BIT(8)|BIT(9), 1);
			PRINT_INFO("DFS CP2. Switch channel!\n");
		}else{
			if (dfs_chk){
				// reset dfs flag and counter
				priv->pmib->dot11DFSEntry.DFS_detected = 0;
				PHY_SetBBReg(priv, 0xc84, BIT(25), 0);
				PHY_SetBBReg(priv, 0xc84, BIT(25), 1);
				
				PRINT_INFO("DFS CP1.\n");
				init_timer(&priv->dfs_chk_timer);
				priv->dfs_chk_timer.data = (unsigned long) priv;
				priv->dfs_chk_timer.function = rtl8192cd_dfs_chk_timer;
				
				mod_timer(&priv->dfs_chk_timer, jiffies + RTL_SECONDS_TO_JIFFIES(300));

				goto exit_timer;
			}
		}
		
		RTL_W8(TXPAUSE, 0xf);	/* disable transmitter */
		priv->pmib->dot11DFSEntry.disable_tx = 1;

		if (timer_pending(&priv->ch_avail_chk_timer)) {
			del_timer_sync(&priv->ch_avail_chk_timer);
			RTL_W8(TXPAUSE, 0xff);
		}

		switch(priv->pmib->dot11RFEntry.dot11channel) {
		case 52:
			mod_timer(&priv->ch52_timer, jiffies + NONE_OCCUPANCY_PERIOD);
			break;
		case 56:
			mod_timer(&priv->ch56_timer, jiffies + NONE_OCCUPANCY_PERIOD);
			break;
		case 60:
			mod_timer(&priv->ch60_timer, jiffies + NONE_OCCUPANCY_PERIOD);
			break;
		case 64:
			mod_timer(&priv->ch64_timer, jiffies + NONE_OCCUPANCY_PERIOD);
			break;
		case 100:
			mod_timer(&priv->ch100_timer, jiffies + NONE_OCCUPANCY_PERIOD);
			break;
		case 104:
			mod_timer(&priv->ch104_timer, jiffies + NONE_OCCUPANCY_PERIOD);
			break;
		case 108:
			mod_timer(&priv->ch108_timer, jiffies + NONE_OCCUPANCY_PERIOD);
			break;
		case 112:
			mod_timer(&priv->ch112_timer, jiffies + NONE_OCCUPANCY_PERIOD);
			break;
		case 116:
			mod_timer(&priv->ch116_timer, jiffies + NONE_OCCUPANCY_PERIOD);
			break;
		case 120:
			mod_timer(&priv->ch120_timer, jiffies + NONE_OCCUPANCY_PERIOD);
			break;
		case 124:
			mod_timer(&priv->ch124_timer, jiffies + NONE_OCCUPANCY_PERIOD);
			break;
		case 128:
			mod_timer(&priv->ch128_timer, jiffies + NONE_OCCUPANCY_PERIOD);
			break;
		case 132:
			mod_timer(&priv->ch132_timer, jiffies + NONE_OCCUPANCY_PERIOD);
			break;
		case 136:
			mod_timer(&priv->ch136_timer, jiffies + NONE_OCCUPANCY_PERIOD);
			break;
		case 140:
			mod_timer(&priv->ch140_timer, jiffies + NONE_OCCUPANCY_PERIOD);
			break;
		default:
			DEBUG_ERR("DFS_timer: Channel match none!\n");
			break;
		}

		/* add the channel in the blocked-channel list */
		InsertChannel(priv->NOP_chnl, &priv->NOP_chnl_num, priv->pmib->dot11RFEntry.dot11channel);

		/* select a non-DFS channel */
		priv->pshare->dfsSwitchChannel = DFS_SelectChannel(priv);
#ifdef MBSSID
		if (priv->pmib->miscEntry.vap_enable)
			priv->pshare->dfsSwitchChCountDown = 6;
		else
#endif
			priv->pshare->dfsSwitchChCountDown = 5;

		if (priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod >= priv->pshare->dfsSwitchChCountDown)
			priv->pshare->dfsSwitchChCountDown = priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod+1;

		if (timer_pending(&priv->DFS_timer))
			del_timer_sync(&priv->DFS_timer);

		return;
	}

exit_timer:
	mod_timer(&priv->DFS_timer, jiffies + DFS_TO);
}

#ifdef CLIENT_MODE
#ifdef __KERNEL__
void rtl8192cd_dfs_cntdwn_timer(unsigned long task_priv)
#elif defined(__ECOS)
void rtl8192cd_dfs_cntdwn_timer(void *task_priv)
#endif
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;
	
	DEBUG_INFO("rtl8192cd_dfs_cntdwn_timer timeout!\n");

	priv->pshare->dfsSwCh_ongoing = 0;	
}
#endif

void rtl8192cd_ch_avail_chk_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	priv->pmib->dot11DFSEntry.disable_tx = 0;

	if (GET_CHIP_VER(priv) == VERSION_8192D){
		if (priv->pshare->rf_ft_var.dfsdelayiqk)
		PHY_IQCalibrate(priv);
	}

        RTL_W16(PCIE_CTRL_REG, RTL_R16(PCIE_CTRL_REG)& (~BCNQSTOP));

	panic_printk("Transmitter is enabled!\n");
}


void rtl8192cd_ch52_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	//still block channel 52 if in adhoc mode in Japan
	if (((priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK) ||
		 (priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK3)) &&
		(OPMODE & WIFI_ADHOC_STATE))
		return;

	//remove the channel from NOP_chnl[4] and place it in available_chnl[32]
	if (RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 52)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->available_chnl, &priv->available_chnl_num, 52);
		DEBUG_INFO("Channel 52 is released!\n");
	}
}


void rtl8192cd_ch56_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (((priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK) ||
		 (priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK3)) &&
		(OPMODE & WIFI_ADHOC_STATE))
		return;
	if (RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 56)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->available_chnl, &priv->available_chnl_num, 56);
		DEBUG_INFO("Channel 56 is released!\n");
	}
}


void rtl8192cd_ch60_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (((priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK) ||
		 (priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK3)) &&
		(OPMODE & WIFI_ADHOC_STATE))
		return;
	if (RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 60)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->available_chnl, &priv->available_chnl_num, 60);
		DEBUG_INFO("Channel 60 is released!\n");
	}
}


void rtl8192cd_ch64_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (((priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK) ||
		 (priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK3)) &&
		(OPMODE & WIFI_ADHOC_STATE))
		return;
	if (RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 64)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->available_chnl, &priv->available_chnl_num, 64);
		DEBUG_INFO("Channel 64 is released!\n");
	}
}


void rtl8192cd_ch100_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 100)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->available_chnl, &priv->available_chnl_num, 100);
		DEBUG_INFO("Channel 100 is released!\n");
	}
}


void rtl8192cd_ch104_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 104)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->available_chnl, &priv->available_chnl_num, 104);
		DEBUG_INFO("Channel 104 is released!\n");
	}
}


void rtl8192cd_ch108_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 108)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->available_chnl, &priv->available_chnl_num, 108);
		DEBUG_INFO("Channel 108 is released!\n");
	}
}


void rtl8192cd_ch112_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 112)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->available_chnl, &priv->available_chnl_num, 112);
		DEBUG_INFO("Channel 112 is released!\n");
	}
}


void rtl8192cd_ch116_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 116)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->available_chnl, &priv->available_chnl_num, 116);
		DEBUG_INFO("Channel 116 is released!\n");
	}
}


void rtl8192cd_ch120_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 120)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->available_chnl, &priv->available_chnl_num, 120);
		DEBUG_INFO("Channel 120 is released!\n");
	}
}


void rtl8192cd_ch124_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 124)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->available_chnl, &priv->available_chnl_num, 124);
		DEBUG_INFO("Channel 124 is released!\n");
	}
}


void rtl8192cd_ch128_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 128)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->available_chnl, &priv->available_chnl_num, 128);
		DEBUG_INFO("Channel 128 is released!\n");
	}
}


void rtl8192cd_ch132_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 132)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->available_chnl, &priv->available_chnl_num, 132);
		DEBUG_INFO("Channel 132 is released!\n");
	}
}


void rtl8192cd_ch136_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 136)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->available_chnl, &priv->available_chnl_num, 136);
		DEBUG_INFO("Channel 136 is released!\n");
	}
}


void rtl8192cd_ch140_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 140)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
			InsertChannel(priv->available_chnl, &priv->available_chnl_num, 140);
		DEBUG_INFO("Channel 140 is released!\n");
	}
}


unsigned int DFS_SelectChannel(struct rtl8192cd_priv *priv)
{
	unsigned char random;
	unsigned int num, random_base, which_channel;
	int reg = priv->pmib->dot11StationConfigEntry.dot11RegDomain;

	if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20) {
#ifdef __ECOS
		{
			unsigned char random_buf[4];
			get_random_bytes(random_buf, 4);
			random = random_buf[3];
		}
#else
		get_random_bytes(&random, 1);
#endif
		num = random % priv->Not_DFS_chnl_num;
		which_channel = priv->Not_DFS_chnl[num];
	} else {
		//40M
		if (reg == DOMAIN_FCC)
			random_base = 4;
		else if (reg == DOMAIN_IC)
			random_base = 4;
		else if (reg == DOMAIN_NCC)
			random_base = 3;
		else
			random_base = 2;

#ifdef __ECOS
		{
			unsigned char random_buf[4];
			get_random_bytes(random_buf, 4);
			random = random_buf[3];
		}
#else
		get_random_bytes(&random, 1);
#endif
		num = random % random_base;

		if (reg == DOMAIN_NCC) {
			if (priv->pshare->offset_2nd_chan == HT_2NDCH_OFFSET_BELOW)
				which_channel = priv->Not_DFS_chnl[num*2+2];
			else
				which_channel = priv->Not_DFS_chnl[num*2+1];
		}
		else {
			if (priv->pshare->offset_2nd_chan == HT_2NDCH_OFFSET_BELOW)
				which_channel = priv->Not_DFS_chnl[num*2+1];
			else
				which_channel = priv->Not_DFS_chnl[num*2];
		}
	}

	return which_channel;
}


//insert the channel into the channel list
//if successful, return 1, else return 0
int InsertChannel(unsigned int chnl_list[], unsigned int *chnl_num, unsigned int channel)
{
	unsigned int i, j;

	if (*chnl_num==0) {
		chnl_list[0] = channel;
		(*chnl_num)++;
		return SUCCESS;
	}

	for (i=0; i < *chnl_num; i++) {
		if (chnl_list[i] == channel) {
			_DEBUG_INFO("Inserting channel failed: channel %d already exists!\n", channel);
			return FAIL;
		} else if (chnl_list[i] > channel) {
			break;
		}
	}

	if (i == *chnl_num) {
		chnl_list[(*chnl_num)++] = channel;
	} else {
		for (j=*chnl_num; j > i; j--)
			chnl_list[j] = chnl_list[j-1];
		chnl_list[j] = channel;
		(*chnl_num)++;
	}

	return SUCCESS;
}


/*
 *	remove the channel from the channel list
 *	if successful, return 1, else return 0
 */
int RemoveChannel(unsigned int chnl_list[], unsigned int *chnl_num, unsigned int channel)
{
	unsigned int i, j;

	if (*chnl_num) {
		for (i=0; i < *chnl_num; i++)
			if (channel == chnl_list[i])
				break;
		if (i == *chnl_num)  {
			_DEBUG_INFO("Can not remove channel %d!\n", channel);
			return FAIL;
		} else {
			for (j=i; j < (*chnl_num-1); j++)
				chnl_list[j] = chnl_list[j+1];
			(*chnl_num)--;
			return SUCCESS;
		}
	} else {
		_DEBUG_INFO("Can not remove channel %d!\n", channel);
		return FAIL;
	}
}

void DFS_SwChnl_clnt(struct rtl8192cd_priv *priv)
{
	/* signin non-DFS channel */
	priv->pmib->dot11RFEntry.dot11channel = priv->pshare->dfsSwitchChannel;
	priv->pshare->dfsSwitchChannel = 0;
	RTL_W8(TXPAUSE, 0xff);
	panic_printk("Swiching channel to %d!\n", priv->pmib->dot11RFEntry.dot11channel);
	reload_txpwr_pg(priv);
	SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);
	RTL_W8(TXPAUSE, 0x00);
	if (((priv->pmib->dot11RFEntry.dot11channel >= 52) &&
		(priv->pmib->dot11RFEntry.dot11channel <= 64)) || 
		((priv->pmib->dot11RFEntry.dot11channel >= 100) &&
		(priv->pmib->dot11RFEntry.dot11channel <= 140))) {

			panic_printk("Switched to DFS band (ch %d) again!!\n", priv->pmib->dot11RFEntry.dot11channel);

	 }

#ifdef CONFIG_RTL_92D_SUPPORT
	if ((GET_CHIP_VER(priv) == VERSION_8192D) && (priv->pmib->dot11Bss.channel > 14)) {
		priv->pshare->iqk_5g_done = 0;
		PHY_IQCalibrate(priv);
	}
#endif
}


void DFS_SwitchChannel(struct rtl8192cd_priv *priv)
{
#ifdef MBSSID
	unsigned int i;
#endif

	/* signin non-DFS channel */
	priv->pmib->dot11RFEntry.dot11channel = priv->pshare->dfsSwitchChannel;
	//priv->pshare->dfsSwitchChannel = 0;
	RTL_W8(TXPAUSE, 0xff);

	panic_printk("Swiching channel to %d!\n", priv->pmib->dot11RFEntry.dot11channel);
	priv->pmib->dot11OperationEntry.keep_rsnie = 1; // recovery in WPA case, david+2006-01-27
#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
		for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
			if (IS_DRV_OPEN(priv->pvap_priv[i]))
				priv->pvap_priv[i]->pmib->dot11OperationEntry.keep_rsnie = 1;
		}
	}
#endif
#if 0
	reload_txpwr_pg(priv);
	SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);
 	if (GET_CHIP_VER(priv) == VERSION_8192D)
		PHY_IQCalibrate(priv);
#else
	rtl8192cd_close(priv->dev);
	rtl8192cd_open(priv->dev);
#endif
	RTL_W8(TXPAUSE, 0x00);
}


void DFS_SetReg(struct rtl8192cd_priv *priv)
{
	PHY_SetBBReg(priv, 0xc38, BIT(23) | BIT(22), 2);
	PHY_SetBBReg(priv, 0x814, bMaskDWord, 0x04cc4d10);

	if (priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_ETSI) {
		PHY_SetBBReg(priv, 0xc8c, BIT(23) | BIT(22), 3);
		PHY_SetBBReg(priv, 0xc30, 0xf, 0xa);
		PHY_SetBBReg(priv, 0xcdc, 0xf0000, 4);
		PHY_SetBBReg(priv, 0xcd8, 0xf0000, 0x3);
	} else {
		PHY_SetBBReg(priv, 0xc8c, BIT(23) | BIT(22), 0);
		PHY_SetBBReg(priv, 0xcd8, 0xffff, 0x1a1f);
		PHY_SetBBReg(priv, 0xcd8, 0xf0000, 0x4);
	}

	/*
	 *	Enable h/w DFS detect
	 */
	PHY_SetBBReg(priv, 0xc84, BIT(25), 1);

	if (!priv->pshare->rf_ft_var.dfsdbgmode){
		PHY_SetBBReg(priv, 0xc7c, BIT(28), 1); // ynlin dbg
	}

}
#endif
 
