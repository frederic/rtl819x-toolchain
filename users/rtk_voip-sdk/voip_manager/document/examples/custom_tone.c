#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "voip_manager.h"

//the ToneTable vlaue can be filled in Web page 
TstVoipToneCfg ToneTable[] =
{
	{0, 0, 1, 2000,    0, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, 13, 13, 0, 0},
	{0, 0, 1, 2000,    0, 0, 0, 0, 0, 0, 0, 0, 1, 100,   0, 0, 0, 13,  0, 0, 0},
	{0, 0, 1, 2000,    0, 0, 0, 0, 0, 0, 0, 0, 2, 350, 440, 0, 0, 13, 13, 0, 0},			
	{0, 0, 1, 2000,    0, 0, 0, 0, 0, 0, 0, 0, 1, 350,   0, 0, 0, 13,  0, 0, 0},				
	{0, 0, 1,  500,  500, 0, 0, 0, 0, 0, 0, 0, 2, 480, 620, 0, 0, 24, 24, 0, 0}, 			
	{0, 1, 1, 1500,    0, 0, 0, 0, 0, 0, 0, 0, 2, 440, 480, 0, 0, 13, 13, 0, 0},			
	{0, 0, 1, 2000, 4000, 0, 0, 0, 0, 0, 0, 0, 2, 440, 480, 0, 0,  7,  7, 0, 0},			
	{0, 0, 0,    0,    0, 0, 0, 0, 0, 0, 0, 0, 0,   0,   0, 0, 0,  0,  0, 0, 0}			
};

void SetCustomTone(void);

int main(void)
{
	int i;
	SIGNSTATE val;	
	
	SetCustomTone();
	
	for (i=0; i<CON_CH_NUM; i++)
	{
		if( !RTK_VOIP_IS_SLIC_CH( i, g_VoIP_Feature ) )
			continue;
		
		rtk_Set_Voice_Gain(i, 0, 0);
		rtk_Set_Flash_Hook_Time(i, 0, 300);
		rtk_Set_flush_fifo(i);					// flush kernel fifo before app run
	}

main_loop:

	for (i=0; i<CON_CH_NUM; i++)
	{
		if( !RTK_VOIP_IS_SLIC_CH( i, g_VoIP_Feature ) )
			continue;
			
		rtk_GetFxsEvent(i, &val);
		switch (val)
		{
		case SIGN_KEY1:
			rtk_SetPlayTone(i, 0, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);
			rtk_SetPlayTone(i, 0, DSPCODEC_TONE_DIAL, 1, DSPCODEC_TONEDIRECTION_LOCAL);
			break;
		case SIGN_KEY2:
			rtk_SetPlayTone(i, 0, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);
			rtk_SetPlayTone(i, 0, DSPCODEC_TONE_RINGING, 1, DSPCODEC_TONEDIRECTION_LOCAL);
			break;
		case SIGN_KEY3:
			rtk_SetPlayTone(i, 0, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);
			rtk_SetPlayTone(i, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
			break;
		case SIGN_KEY4:
			rtk_SetPlayTone(i, 0, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);
			rtk_SetPlayTone(i, 0, DSPCODEC_TONE_CALL_WAITING, 1, DSPCODEC_TONEDIRECTION_LOCAL);
			break;
		case SIGN_ONHOOK:
			rtk_SetPlayTone(i, 0, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);
			rtk_Onhook_Action(i);
			break;
		case SIGN_OFFHOOK:
			rtk_Offhook_Action(i);
			break;
		case SIGN_FLASHHOOK:
			rtk_SetPlayTone(i, 0, DSPCODEC_TONE_HOLD, 1, DSPCODEC_TONEDIRECTION_LOCAL);
			break;
		case SIGN_NONE:
			break;
		case SIGN_OFFHOOK_2:
			break;
		default:
			printf("unknown(%d)\n", val);
			break;
		}

		usleep(100000 / CON_CH_NUM); // 100 ms
	}

	goto main_loop;

	return 0;
}
void SetCustomTone()
{
	int cust_idx;
	
	st_ToneCfgParam custom_tone[TONE_CUSTOMER_MAX];
	
	voipCfgParam_t VoIPCfg;
	
	
	memcpy(custom_tone, ToneTable, TONE_CUSTOMER_MAX*sizeof(st_ToneCfgParam));
	
	for (cust_idx=0; cust_idx < TONE_CUSTOMER_MAX; cust_idx++)
	{
		rtk_Set_Custom_Tone(cust_idx, &custom_tone[cust_idx]);
	}	
	
	VoIPCfg.tone_of_country = 13; //Custom
	VoIPCfg.tone_of_custdial = 0;
	VoIPCfg.tone_of_custring = 1;
	VoIPCfg.tone_of_custbusy = 2;
	VoIPCfg.tone_of_custwaiting = 3;
	
	rtk_Set_Country(&VoIPCfg);
}
