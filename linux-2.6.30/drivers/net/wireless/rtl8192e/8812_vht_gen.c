

#include "8192cd.h"
#include "8192cd_cfg.h"
#include "8192cd_util.h"
#include "8192cd_headers.h"

#include "8812_vht_gen.h"
#ifdef RTK_AC_SUPPORT

void input_value_32(unsigned int* p, unsigned char start, unsigned char end, unsigned int value)
{
	unsigned int bit_mask = 0;

	if(value > 0) //only none-zero value needs to be assigned 
	{
		if(start == end) //1-bit value
		{
			*p |= BIT(start);
		}
		else
		{
			unsigned char x = 0;
				
			for(x = 0; x<=(end-start); x ++)
				bit_mask |= BIT(x);

			*p |= ((value&bit_mask) << start);	
		}
	}

}

// 				20/40/80,	ShortGI,	MCS Rate 
const u2Byte VHT_MCS_DATA_RATE[3][2][20] = 
	{	{	{13, 26, 39, 52, 78, 104, 117, 130, 156, 156,
			 26, 52, 78, 104, 156, 208, 234, 260, 312, 312},			// Long GI, 20MHz
			{14, 29, 43, 58, 87, 116, 130, 144, 173, 173,
			29, 58, 87, 116, 173, 231, 260, 289, 347, 347}	},		// Short GI, 20MHz
		{	{27, 54, 81, 108, 162, 216, 243, 270, 324, 360, 
			54, 108, 162, 216, 324, 432, 486, 540, 648, 720}, 		// Long GI, 40MHz
			{30, 60, 90, 120, 180, 240, 270, 300,360, 400, 
			60, 120, 180, 240, 360, 480, 540, 600, 720, 800}},		// Short GI, 40MHz
		{	{59, 117,  176, 234, 351, 468, 527, 585, 702, 780,
			117, 234, 351, 468, 702, 936, 1053, 1170, 1404, 1560}, 	// Long GI, 80MHz
			{65, 130, 195, 260, 390, 520, 585, 650, 780, 867, 
			130, 260, 390, 520, 780, 1040, 1170, 1300, 1560,1733}	}	// Short GI, 80MHz
	};


/*
*	Description:
*		This function will get the highest speed rate in input MCS set.
*
*	/param 	Adapter			Pionter to Adapter entity
*			pMCSRateSet		Pointer to MCS rate bitmap
*			pMCSFilter		Pointer to MCS rate filter
*	
*	/return	Highest MCS rate included in pMCSRateSet and filtered by pMCSFilter.
*
*/
u1Byte
VHTGetHighestMCSRate(
	struct rtl8192cd_priv *priv,
	IN	pu1Byte			pVHTMCSRateSet
	)
{
	u1Byte		i, j;
	u1Byte		bitMap;
	u1Byte		VHTMcsRate = 0;
	
	for(i = 0; i < 2; i++)
	{
		if(pVHTMCSRateSet[i] != 0xff)
		{
			for(j = 0; j < 8; j += 2)
			{
				bitMap = (pVHTMCSRateSet[i] >> j) & 3;
				
				if(bitMap != 3)
					VHTMcsRate = _NSS1_MCS7_RATE_ + 5*j + i*40 + bitMap;  //VHT rate indications begin from 0x90
			}
		}
	}
	
	return VHTMcsRate;
}

u2Byte
VHTMcsToDataRate(
	struct rtl8192cd_priv *priv,
	u2Byte			VHTMcsRate
	)
{
	BOOLEAN						isShortGI = FALSE;


	VHTMcsRate -=_NSS1_MCS0_RATE_;
	
	switch(priv->pshare->CurrentChannelBW){
		case HT_CHANNEL_WIDTH_20:
			isShortGI = priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M?1:0;
			break;
		case HT_CHANNEL_WIDTH_20_40:
			isShortGI = priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M?1:0;
			break;
		case HT_CHANNEL_WIDTH_80:
			isShortGI = priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M?1:0;		// ??
			break;
	}
	if( (VHTMcsRate>20) || (priv->pshare->CurrentChannelBW > 2))
		return 600;
	else
		return VHT_MCS_DATA_RATE[priv->pshare->CurrentChannelBW][isShortGI][(VHTMcsRate&0x3f)];
}


void construct_vht_ie(struct rtl8192cd_priv *priv, unsigned char channel_center)
{
	struct vht_cap_elmt	*vht_cap;
	struct vht_oper_elmt *vht_oper;
	unsigned int value; 

	
//// ===== VHT CAPABILITIES ELEMENT ===== /////
//VHT CAPABILITIES INFO field

	priv->vht_cap_len = sizeof(struct vht_cap_elmt);
	vht_cap = &priv->vht_cap_buf; 
	memset(vht_cap, 0, sizeof(struct vht_cap_elmt));

	input_value_32(&vht_cap->vht_cap_info, MAX_MPDU_LENGTH_S, MAX_MPDU_LENGTH_E, 0);

	//0 - not support 160/80+80; 1 - support 160; 2 - support 80+80 
	if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_AC_160)
		value = 1;
	else
		value = 0;
	input_value_32(&vht_cap->vht_cap_info, CHL_WIDTH_S, CHL_WIDTH_E, value);


	if(priv->pshare->rf_ft_var.rxldpc)
		input_value_32(&vht_cap->vht_cap_info, RX_LDPC_S, RX_LDPC_E, 1);
	else
		input_value_32(&vht_cap->vht_cap_info, RX_LDPC_S, RX_LDPC_E, 0);
	
	input_value_32(&vht_cap->vht_cap_info, SHORT_GI80M_S, SHORT_GI80M_E, (priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M ? 1 : 0));
	input_value_32(&vht_cap->vht_cap_info, SHORT_GI160M_S, SHORT_GI160M_E, 0);

#ifdef FOR_VHT5G_PF2 //8812 stbc
	if(priv->pmib->dot11nConfigEntry.dot11nSTBC)
	{
		input_value_32(&vht_cap->vht_cap_info, TX_STBC_S, TX_STBC_E, 1);
		input_value_32(&vht_cap->vht_cap_info, RX_STBC_S, RX_STBC_E, 1);
	}
	else
#endif
	{
		input_value_32(&vht_cap->vht_cap_info, TX_STBC_S, TX_STBC_E, 0);
		input_value_32(&vht_cap->vht_cap_info, RX_STBC_S, RX_STBC_E, 0);
	}

	input_value_32(&vht_cap->vht_cap_info, SU_BFER_S, SU_BFER_E, 0);
	input_value_32(&vht_cap->vht_cap_info, SU_BFEE_S, SU_BFEE_E, 0);

	input_value_32(&vht_cap->vht_cap_info, MAX_ANT_SUPP_S, MAX_ANT_SUPP_E, 2);
	input_value_32(&vht_cap->vht_cap_info, SOUNDING_DIMENSIONS_S, SOUNDING_DIMENSIONS_E, 1);

	input_value_32(&vht_cap->vht_cap_info, MU_BFER_S, MU_BFER_E, 0);
	input_value_32(&vht_cap->vht_cap_info, MU_BFEE_S, MU_BFEE_E, 0);
	
	input_value_32(&vht_cap->vht_cap_info, TXOP_PS_S, TXOP_PS_E, 0);

	input_value_32(&vht_cap->vht_cap_info, HTC_VHT_S, HTC_VHT_E, 1);

	input_value_32(&vht_cap->vht_cap_info, MAX_RXAMPDU_FACTOR_S, MAX_RXAMPDU_FACTOR_E, 7);
	
	input_value_32(&vht_cap->vht_cap_info, LINK_ADAPTION_S, LINK_ADAPTION_E, 0);
	
	input_value_32(&vht_cap->vht_cap_info, RX_ANT_PC_S, RX_ANT_PC_E, 0);
	input_value_32(&vht_cap->vht_cap_info, TX_ANT_PC_S, TX_ANT_PC_E, 0);

	//printk("vht_cap->vht_cap_info 0x%08X ", vht_cap->vht_cap_info);
	vht_cap->vht_cap_info = cpu_to_le32(vht_cap->vht_cap_info);
	//printk("0x%08X\n", vht_cap->vht_cap_info);

	{
		input_value_32(&vht_cap->vht_support_mcs[0], MCS_RX_MAP_S, MCS_RX_MAP_E, priv->pmib->dot11acConfigEntry.dot11SupportedVHT);
		value = (VHTMcsToDataRate(priv, vht_cap->vht_support_mcs)+1)>>1;
		input_value_32(&vht_cap->vht_support_mcs[0], MCS_RX_HIGHEST_RATE_S, MCS_RX_HIGHEST_RATE_E, value);
		vht_cap->vht_support_mcs[0] = cpu_to_le32(vht_cap->vht_support_mcs[0]);

		input_value_32(&vht_cap->vht_support_mcs[1], MCS_TX_MAP_S, MCS_TX_MAP_E, priv->pmib->dot11acConfigEntry.dot11SupportedVHT);
		value = (VHTMcsToDataRate(priv, vht_cap->vht_support_mcs)+1)>>1;
		input_value_32(&vht_cap->vht_support_mcs[1], MCS_TX_HIGHEST_RATE_S, MCS_TX_HIGHEST_RATE_E, value);
		vht_cap->vht_support_mcs[1] = cpu_to_le32(vht_cap->vht_support_mcs[1]);
	}

//// ===== VHT CAPABILITIES ELEMENT ===== /////
	priv->vht_oper_len = sizeof(struct vht_oper_elmt);
	vht_oper = &priv->vht_oper_buf; 
	memset(vht_oper, 0, sizeof(struct vht_oper_elmt));

	if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_AC_80)
	{
		vht_oper->vht_oper_info[0] = (priv->pmib->dot11nConfigEntry.dot11nUse40M==2) ? 1 : 0;	

		if(OPMODE & (WIFI_STATION_STATE))
		vht_oper->vht_oper_info[0] = 1; //8812_client
			
		vht_oper->vht_oper_info[1] = channel_center;
		vht_oper->vht_oper_info[2] = 0;
	}

	if(get_rf_mimo_mode(priv) == MIMO_1T1R) 
		value = 0xfffc; 
	else
		value = 0xfff0; 
	
	vht_oper->vht_basic_msc = value; 
	vht_oper->vht_basic_msc = cpu_to_le16(vht_oper->vht_basic_msc);


}







#endif //RTK_AC_SUPPORT

