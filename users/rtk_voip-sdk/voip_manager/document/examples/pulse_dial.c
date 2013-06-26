#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "voip_manager.h"

int main(int argc, char *argv[])
{
	if (argc != 7)
	{
		printf("\n");
		printf("Usage: pulse_dial chid enable pps make_duration pause_duration pulse_digit\n");
		printf("- chid: FXO channel ID\n");
		printf("- enable: 0-disable, 1-enable\n");
		printf("- pps: 10 or 20 pulse per second\n");
		printf("- make_duration: should smaller than 100msec, ex: 40 msec\n");
		printf("- pause_duration: the pause duration between two adjacent pulse digit, uint: msec\n");
		printf("- pulse_digit: the pulse digit you want to gen.\n");
		printf("\n");
		return -1;
	}
	else
	{
		int chid = atoi(argv[1]);	
		
		// 1. off-hook daa first
		rtk_FXO_offhook(chid);
		// 2. wait for a while
		sleep(1);
		// 3. enable pulse dial gen
		rtk_Set_Dail_Mode(chid, atoi(argv[2]));
		// 4. config the pulse dial gen
		rtk_PulseDial_Gen_Cfg(atoi(argv[3]), atoi(argv[4]), atoi(argv[5]));
		// 5. gen the pulse digit you want
		rtk_Gen_Pulse_Dial(chid, atoi(argv[6]));
		
		// Note: No need to repeat step 1 ~ 4, it's just demo.
		// After step 1 ~ 4 are finished, you can just call step 5 to generate different pulse digit.
	
		return 0;
	}
}


