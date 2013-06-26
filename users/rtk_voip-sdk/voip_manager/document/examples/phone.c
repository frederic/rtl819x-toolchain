#include <stdio.h>
#include <unistd.h>
#include "voip_manager.h"

int main(void)
{
	int i;
	SIGNSTATE val;
	
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
		case SIGN_KEY2:
		case SIGN_KEY3:
		case SIGN_KEY4:
		case SIGN_KEY5:
		case SIGN_KEY6:
		case SIGN_KEY7:
		case SIGN_KEY8:
		case SIGN_KEY9:
		case SIGN_KEY0:
		case SIGN_STAR:
		case SIGN_HASH:
			rtk_SetPlayTone(i, 0, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);
			break;
		case SIGN_ONHOOK:
			rtk_SetPlayTone(i, 0, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);
			rtk_Onhook_Action(i);
			break;
		case SIGN_OFFHOOK:
			rtk_Offhook_Action(i);
			rtk_SetPlayTone(i, 0, DSPCODEC_TONE_DIAL, 1, DSPCODEC_TONEDIRECTION_LOCAL);
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
