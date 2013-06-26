#include "voip_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>


#define BUFFER_FULL 1
#define BUFFER_NOTFULL 0

int voiceplay_main(int argc, char *argv[])
{
	FILE *file_out_tx;
	int chid;
	int loop_times;
	int dir;
	int i;
	int pre_stat, stat;
	time_t timer = time(0);
	int kernel_buffer_prestat;

	TstVoipdataput_o stVoipdataput_o;
	TstVoipCfg stVoipCfg;
	struct timespec pause_time;

	char txfilename[40];


	if (argc == 5) {
		chid = atoi(argv[1]);
		stVoipdataput_o.ch_id=chid;
		strcpy(txfilename ,argv[4]);
		loop_times = atoi(argv[3]);
		dir = atoi(argv[2]);

		pause_time.tv_sec=0;
		pause_time.tv_nsec=15000000;

		timer = time(0);


		if ((file_out_tx = fopen(txfilename,"rb")) == NULL) {
			printf("voiceplay: Unable to open %s \n",txfilename);
			return(2);
		}

		if (loop_times == 0) {

			while (1) {
				pre_stat=stat;
				rtk_Set_GetPhoneStat(&stVoipCfg);  //1:offhook, 0:onhook
				stat=stVoipCfg.cfg;
	
				if( (pre_stat==1) && (stat==0) ) {
					stVoipdataput_o.write_enable = 0;
					stVoipdataput_o.mode = 0;
					rewind(file_out_tx);
	
				} else {
					stVoipdataput_o.write_enable = dir;
					stVoipdataput_o.mode = 1;
					if (kernel_buffer_prestat!=BUFFER_FULL) {
						i=fread(stVoipdataput_o.buf,sizeof(char), EACH_DATAPUTBUFSIZE,file_out_tx);
						if (i != EACH_DATAPUTBUFSIZE)
							rewind(file_out_tx);
					}
				}
				rtk_Set_Voice_Play(&stVoipdataput_o);
				kernel_buffer_prestat = stVoipdataput_o.ret_val;

				nanosleep(&pause_time, 0);
			}
		} else {
			while (loop_times) {
				stVoipdataput_o.write_enable = dir;
				stVoipdataput_o.mode = 1;
				if (kernel_buffer_prestat!=BUFFER_FULL) {
					i=fread(stVoipdataput_o.buf,sizeof(char), EACH_DATAPUTBUFSIZE,file_out_tx);
					if (i != EACH_DATAPUTBUFSIZE) {
						//printf("rewind");
						rewind(file_out_tx);
						loop_times--;
					}
				}
				rtk_Set_Voice_Play(&stVoipdataput_o);
				kernel_buffer_prestat = stVoipdataput_o.ret_val;

				nanosleep(&pause_time, 0);
			}

			stVoipdataput_o.write_enable=0;
			stVoipdataput_o.mode = 0;
			rtk_Set_Voice_Play(&stVoipdataput_o);

			fclose(file_out_tx);

		}
	} else {
		printf("use error! Method: voiceplay chid dir loop_times voice_file & \ndir-> 2:tx 4:rx\n");
		printf("tx: network->dsp->phone, rx: phone->dsp->network\n");
	}

	return 0;
}
