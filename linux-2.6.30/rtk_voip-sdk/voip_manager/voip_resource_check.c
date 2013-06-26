#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_control.h"

#include "voip_resource_check.h"

#ifdef VOIP_RESOURCE_CHECK

int resource_weight[DSP_RTK_SS_NUM] = {0};

int GetCurrentVoipResourceStatus(int pkt_fmt)
{
	int ss_id, weight_sum=0, weight, pt, result;
	for (ss_id=0; ss_id < DSP_RTK_SS_NUM; ss_id++)
	{
		weight_sum += resource_weight[ss_id];
	}

	pt = pkt_fmt;

	if ((pt == 0) || (pt == 8))							// G.711
		weight = G711_WEIGHT;
	else if ((pt == 3) ||(pt == 4) || (pt == 18) || (pt == rtpPayload_AMR_NB))	// G.729, G.723, GSM-FR, AMR-NB
		weight = G729_WEIGHT;
	else if ((pt == 2) || (pt == 21) || (pt == 22) || (pt == 23))			// G.726
		weight = G726_WEIGHT;
	else if ((pt == rtpPayload_iLBC) || (pt == rtpPayload_iLBC_20ms))		// iLBC
		weight = ILBC_WEIGHT;
	else if ((pt == rtpPayload_SPEEX_NB_RATE2P15) || (pt == rtpPayload_SPEEX_NB_RATE5P95) || (pt == rtpPayload_SPEEX_NB_RATE8) || (pt == rtpPayload_SPEEX_NB_RATE11)
	      || (pt == rtpPayload_SPEEX_NB_RATE15) || (pt == rtpPayload_SPEEX_NB_RATE18P2) || (pt == rtpPayload_SPEEX_NB_RATE24P6) || (pt == rtpPayload_SPEEX_NB_RATE3P95))
		weight = SPEEXNB_WEIGHT;
	else if (pt == rtpPayloadT38_Virtual)								// T.38
		weight = T38_WEIGHT;
	else
		weight = DEFAULT_WEIGHT;

	//printk("weight_sum=%d, weight=%d, ===>", weight_sum, weight);

	if ((weight_sum + weight) > RES_WEIGHT_THS)
	{
		result = VOIP_RESOURCE_UNAVAILABLE;
		//printk("VOIP_RESOURCE_UNAVAILABLE\n");
	}
	else
	{
		result = VOIP_RESOURCE_AVAILABLE;
		//printk("VOIP_RESOURCE_AVAILABLE\n");
	}

	return 	result;
}

int SetVoipResourceWeight(uint32 s_id, int pltype)
{
	if ((pltype == 0) || (pltype == 8))//G.711
		resource_weight[s_id] = G711_WEIGHT;
	else if ((pltype == 3) ||(pltype == 4) || (pltype == 18) || (pltype == rtpPayload_AMR_NB))// G.729, G.723, GSM-FR, AMR-NB
		resource_weight[s_id] = G729_WEIGHT;
	else if ((pltype == 2) || (pltype == 21)|| (pltype == 22) || (pltype == 23))//G.726
		resource_weight[s_id] = G726_WEIGHT;
	else if ((pltype == rtpPayload_iLBC) || (pltype == rtpPayload_iLBC_20ms))// iLBC
		resource_weight[s_id] = ILBC_WEIGHT;
	else if (pltype == rtpPayloadT38_Virtual)// T.38
		resource_weight[s_id] = T38_WEIGHT;
	else if (pltype == rtpPayloadG722)// G.722
		resource_weight[s_id] = G722_WEIGHT;
	else if ((pltype == rtpPayload_SPEEX_NB_RATE2P15) || 
		(pltype == rtpPayload_SPEEX_NB_RATE5P95) || 
		(pltype == rtpPayload_SPEEX_NB_RATE8) || 
		(pltype == rtpPayload_SPEEX_NB_RATE11) || 
		(pltype == rtpPayload_SPEEX_NB_RATE15) ||
		(pltype == rtpPayload_SPEEX_NB_RATE18P2) || 
		(pltype == rtpPayload_SPEEX_NB_RATE24P6) || 
		(pltype == rtpPayload_SPEEX_NB_RATE3P95))
		resource_weight[s_id] = SPEEXNB_WEIGHT;
	else if ((pltype == rtpPayloadPCMU_WB) || (pltype == rtpPayloadPCMA_WB)) // G.711.1
		resource_weight[s_id] = G711_WB_WEIGHT;
	else
	{
		DBG_ERROR("%s: set pltype%d weight = %d\n", __FUNCTION__, pltype, 1);
		resource_weight[s_id] = 1;
	}

	return resource_weight[s_id];
}

#endif // VOIP_RESOURCE_CHECK

