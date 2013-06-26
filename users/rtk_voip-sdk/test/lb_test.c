#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "voip_manager.h"

/*
 *  Loopback Test code.
 *  FXS0 and FXS1 are connected when off-hook handsets.
 *  Please killall solar_monitor before running this sample code.
 *  note: FXS0 have to be control channel 0
 *  note: FXS1 have to be control channel 1
 *
 */

int lb_test_main(int argc, char *argv[])
{
	int chid;
	SIGNSTATE val;
	int mode;
	int ring_delay = 0;
	int state[2] = {0,0}; // 0: idle, 1: ring, 2: ringing, 3: connected
	struct timeval ring_start, ring_current;

	if (argc < 2 || (atoi(argv[1]) < 0 || atoi(argv[1]) > 1))
	{
		printf("usage: %s mode ring_delay\n", argv[0]);
		printf("mode: pcm(0), slic(1)\n");
		printf("ring_delay: 0~98, 99: disable\n");
		return 0;
	}

	mode = atoi(argv[1]);

	if (argc == 3)
	{
		ring_delay = atoi(argv[2]);
	}

	for(chid = 0 ; chid <CON_CH_NUM ;chid ++)
	{
		if( !RTK_VOIP_IS_SLIC_CH( chid, g_VoIP_Feature ) )
			continue;
			
		rtk_InitDSP(chid);
		rtk_Set_flush_fifo(chid);	
		rtk_SetDTMFMODE(chid, DTMF_INBAND);	// set dtmf mode
	}
	
	while (1)
	{
		for (chid = 0 ; chid <CON_CH_NUM ; chid ++)
		{
			if ( !RTK_VOIP_IS_SLIC_CH( chid, g_VoIP_Feature ) )
				continue;

			rtk_GetFxsEvent(chid, &val);
			if (val == SIGN_OFFHOOK)
			{
				switch (state[chid])
				{
					case 0: // idle
						state[chid] = 1; // ring
#if 1
						gettimeofday(&ring_start, NULL);
						if (ring_delay == 99)
						{
							// enter ringing state directly
							state[chid ? 0 : 1] = 2; // ringing
						}
#else
						rtk_SetRingFXS(chid ? 0 : 1, 1);
						state[chid ? 0 : 1] = 2; // ringing
#endif
						break;
					case 1: // ring (caller)
						break;
					case 2: // ringing (callee)
						rtk_SetRingFXS(chid, 0);
						if (mode == 0) // pcm loopback
						{
							rtk_Set_PCM_Loop_Mode(0, 1, 0, 1);
						}
						else
						{
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK
							system("reg 0 64 0");
							system("reg 0 64 2");
							system("reg 1 64 2");
							system("reg 1 64 0");
#else
							system("reg 0 12 1");
							system("reg 0 14 17");
							system("reg 1 12 17");
							system("reg 1 14 1");
#endif
						}
						state[chid] = 3; // connected
						state[chid ? 0 : 1] = 3; // connected
						break;
					case 3: // connected
						break;
					default:
						printf("unknown state\n");
						exit(-1);
				}
			}
			else if (val == SIGN_ONHOOK)
			{
				switch (state[chid])
				{
					case 0: // idle
						break;
					case 1: // ring (caller)
						rtk_SetRingFXS(chid ? 0 : 1, 0);
						state[chid] = 0; // idle
						state[chid ? 0 : 1] = 0; // idle
						break;
					case 2: // ringing (callee)
						break;
					case 3: // connected
						if (mode == 0) // pcm loopback
						{
							rtk_Set_PCM_Loop_Mode(0, 0, 0, 1);
						}
						else
						{
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK
							system("reg 0 64 0");
							system("reg 0 64 0");
							system("reg 1 64 2");
							system("reg 1 64 2");
#else
							system("reg 0 12 1");
							system("reg 0 14 1");
							system("reg 1 12 17");
							system("reg 1 14 17");
#endif
						}
						state[chid] = 0; // idle
						state[chid ? 0 : 1] = 0; // idle
						break;
					default:
						printf("unknown state\n");
						exit(-1);
				}
			}
			else
			{
				switch (state[chid])
				{
					case 0: // idle
						break;
					case 1: // ring (caller)
						if (state[chid ? 0 : 1] != 0)
							break;

						gettimeofday(&ring_current, NULL);
						if (ring_current.tv_sec < 
							ring_start.tv_sec + ring_delay)
							break; 

						rtk_SetRingFXS(chid ? 0 : 1, 1);
						state[chid ? 0 : 1] = 2; // ringing
						break;
					case 2: // ringing (callee)
						break;
					case 3: // connected
						break;
					default:
						printf("unknown state\n");
						exit(-1);
				}
			}
		}

		usleep(100000); // 100ms
	}
	return 0;
}
