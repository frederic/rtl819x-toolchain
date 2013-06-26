#include <unistd.h>
#include "voip_manager.h"

#define IDLE_TIME		100			// 100ms
#define RING_STOP_TIME	3000		// 3000ms
#define MAX_RING_CNT	(RING_STOP_TIME / IDLE_TIME)

int main(void)
{
	SIGNSTATE val;
	int bStartRinging, nRingCnt;

	rtk_Set_DAA_Tx_Gain(7);
	rtk_Set_DAA_Rx_Gain(7);

	nRingCnt = 0;
	bStartRinging = 0;
	while (1)
	{
		if (!bStartRinging)
		{
			if (rtk_DAA_ring(0))
			{
				// DAA Ring Start
				rtk_SetRingFXS(0, 1);
				nRingCnt = 0;
				bStartRinging = 1;
			}
		}
		else
		{
			if (rtk_DAA_ring(0))
			{
				// still ringing, reset cnt
				nRingCnt = 0;
			}
			else
			{
				if (++nRingCnt >= MAX_RING_CNT)
				{
					// DAA Ring Stop
					rtk_SetRingFXS(0, 0);
					bStartRinging = 0;
				}
			}
		}
	
		rtk_GetFxsEvent(0, &val);
		switch (val)
		{
		case SIGN_ONHOOK:
			rtk_DAA_on_hook(0);
			rtk_Onhook_Action(0);
			break;
		case SIGN_OFFHOOK:
			rtk_Offhook_Action(0);
			rtk_DAA_off_hook(0);
			break;
		default:
			break;
		}

		usleep(IDLE_TIME * 1000);
	}

	return 0;
}
