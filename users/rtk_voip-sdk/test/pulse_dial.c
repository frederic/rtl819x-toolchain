/* Author : Thlin */

#include "voip_manager.h"

int pulse_dial_main(int argc, char *argv[])
{
	int chid, i, enable;
	unsigned int pause_time, max_break_ths, min_break_ths;

	if (argc < 2)
		goto PULSE_DIAL_USAGE;

	if (argv[1][0] == 'g') // Pulse Dial Generation
	{
		if (argc == 4)
		{
			if (argv[1][1] == 'm')
			{
				rtk_Set_Dail_Mode(atoi(argv[2]), atoi(argv[3]));
	
				if (atoi(argv[2]) == 1)
					printf("Enable pulse dial gen for ch=%d\n", chid);
				else if (atoi(argv[2]) == 0)
					printf("Disable pulse dial gen for ch=%d\n", chid);
			}
			else if (argv[1][1] == 'g')
			{
				int sum = 0;
				int len = 0;
			
				rtk_debug(0); // disable kernel debug message to avoid effect pulse dial
	
				chid = atoi(argv[2]);
				len = strlen(argv[3]);
	
				rtk_FXO_offhook(chid);
				sleep(2);
				for (i=0; i < len; i++)
				{
					//printf("gen %d\n", argv[2][i] - '0');
					rtk_Gen_Pulse_Dial(chid, argv[3][i] - '0');
					sum = sum + (argv[3][i] - '0');
				}
				usleep(100000*sum + 1000*800*(len-1));	// sleep enough time to make sure pulse dial gen finish, assume interdigit_duration = 800 ms
				sleep(2);
				rtk_FXO_onhook(chid);
	
				rtk_debug(3); //enable kernel debug message
			}
		}
		else if (argc == 5)
		{

			rtk_PulseDial_Gen_Cfg(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
			printf("Config pulse dial gen to %d pps, make: %d ms, interdigit duration:%d ms\n", atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
		}
		else
		{
			printf("usage error!\n\n");
			goto PULSE_DIAL_USAGE;
		}
	}
	else if (argv[1][0] == 'd') // Pulse Dial Detection
	{
		if (argc == 7)
		{
			chid = atoi(argv[2]);
			enable = atoi(argv[3]);
			pause_time = atoi(argv[4]);
			min_break_ths = atoi(argv[5]);
			max_break_ths = atoi(argv[6]);

			rtk_Set_Pulse_Digit_Det(chid, enable, pause_time, min_break_ths, max_break_ths);
			if (enable == 1)
			{
				printf("Enable pulse dial det for ch= %d, ", chid); 
				printf("pause_time= %d ms, min_break_ths = %d ms, max_break_ths = %d ms\n", pause_time, min_break_ths, max_break_ths);
			}
			else if (enable == 0)
				printf("Disable pulse dial det for ch= %d\n", chid); 
		}
		else
		{
			printf("usage error!\n\n");
			goto PULSE_DIAL_USAGE;
		}
	}
	else
	{
PULSE_DIAL_USAGE:
		printf("pulse_dial usage:\n");
		printf(" - Enable/Disable pulse dial generation: pulse_dial gm chid flag\n");
		printf("   + Example to enable pulse dial gen: pulse_dial g 2 1\n");
		printf(" - Enable/Disable pulse dial detection: pulse_dial d chid flag pause_time min_break_ths max_break_ths\n");
		printf("   + Example to enable pulse dial det: pulse_dial d 0 1 450 20 70\n");
		printf(" - Config pulse dial generation:  pulse_dial g pps make_time interdigit_duration\n");
		printf("   + Example1: pulse_dial g 10 40 700\n");
		printf("   + Example2: pulse_dial g 20 20 750\n");
		printf(" - Generate pulse dial :  pulse_dial gg chid digit(0~9)\n");
		printf("   + Example1 to gen digit 3939889: pulse_dial gg 2 3939889\n");

		return -1;
	}

	return 0;
}

