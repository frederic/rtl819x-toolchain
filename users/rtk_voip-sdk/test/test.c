#include "voip_manager.h"

int test_main(int argc, char *argv[])
{
	int nMaxVoIPPorts;
	int i, chid;
	SIGNSTATE key;
	char buffer[256];
	int buf_len;

	//nMaxVoIPPorts = RTK_VOIP_SLIC_NUM(g_VoIP_Feature);
	nMaxVoIPPorts = RTK_VOIP_CH_NUM(g_VoIP_Feature);

	rtk_DisableRingFXS(1);
    for (i=0; i<nMaxVoIPPorts; i++)
    {
		rtk_InitDSP(i);
		
		if( !RTK_VOIP_IS_SLIC_CH( i, g_VoIP_Feature ) )
			continue;

		rtk_SetRingFXS(i, 1);
		sleep(1);
		rtk_SetRingFXS(i, 0);
	}

	chid = 0;
	buf_len = 0;
	while (1)
	{
		chid = (chid == 0) ? 1 : 0;
		
		if( !RTK_VOIP_IS_SLIC_CH( chid, g_VoIP_Feature ) )
			continue;
		
		rtk_GetSlicEvent(chid, &key);
		switch (key)
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
			printf("--- key = %d ---\n", (int) key);
			rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);

			if (key == SIGN_HASH)
			{
				if (strcmp(buffer, "1000") == 0)
				{
					printf("Gen FSK Caller ID Type I\n");
					rtk_Set_FSK_Area(chid == 0 ? 1 : 0, 0); /* area -> 0:Bellcore 1:ETSI 2:BT 3:NTT */
					rtk_Gen_FSK_CID(chid == 0 ? 1 : 0, "9876543210", (void *) 0, (void *) 0, 0);
				}
				else if (strcmp(buffer, "2000") == 0)
				{
					printf("Gen FSK Caller ID Type II\n");
					rtk_Set_FSK_Area(chid == 0 ? 1 : 0, 0); 
					rtk_Gen_FSK_CID(chid == 0 ? 1 : 0, "9876543210", "06011200", (void *) 0, 1); // mmddHHMM
				}
				else if (strcmp(buffer, "3000") == 0)
				{
					printf("Gen DTMF Caller ID\n");
					rtk_Gen_Dtmf_CID(chid == 0 ? 1 : 0, buffer);
				}

				buf_len = 0;
				memset(buffer, 0, sizeof(buffer));
			}
			else
			{
				if (buf_len >= 255)
				{
					printf("warning: out of buffer\n");
					continue;
				}

				if (key >= SIGN_KEY1 && key <= SIGN_KEY9)
					buffer[buf_len++] = (int) key + 48;
				else if (key == SIGN_KEY0)
					buffer[buf_len++] = '0';

				buffer[buf_len] = 0;
			}
			break;
		case SIGN_FLASHHOOK:
			printf("--- FLASH HOOK ---\n");
			break;
		case SIGN_ONHOOK:
			printf("--- ON HOOK ---\n");
			break;
		case SIGN_OFFHOOK:
			printf("--- OFF HOOK ---\n");
			rtk_SetTranSessionID(chid, 0);
			rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_DIAL, 1, DSPCODEC_TONEDIRECTION_LOCAL);
		
			break;
		}
		usleep(100000);

		
	}
	return 0;
}

