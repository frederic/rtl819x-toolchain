#include "voip_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

int phonerecord_main(int argc, char *argv[])
{
	FILE *file_out_rx, *file2_out;
	int chid;
	int loop_times;
	int i;
	int pre_stat,stat;
	time_t timer = time(0);
	int date_cnt=10;

	TstVoipdataget_o stVoipdataget_o;
	TstVoipCfg stVoipCfg;
	struct timespec pause_time;

	char date[20],rxfilename[20],lenfilename[20];


	if (argc == 2)
	{
		chid = atoi(argv[1]);

		stVoipdataget_o.ch_id=chid;

		pause_time.tv_sec=0;
		//pause_time.tv_nsec=62500000;
		pause_time.tv_nsec=15000000;



		stVoipCfg.ch_id=chid;

		while (1)
		{
			pre_stat=stat;
			rtk_Set_GetPhoneStat(&stVoipCfg);  //1:offhook, 0:onhook
			stat=stVoipCfg.cfg;

			if ( (pre_stat==0) && (stat==1) )
			{
				timer = time(0);
				strftime(date, 11, "%m%d%H%M%S", localtime(&timer));
				strcpy(rxfilename,date);
				strcpy(lenfilename,date);
				strcpy(&lenfilename[date_cnt],"rec_len");
				rxfilename[date_cnt]='r';
				rxfilename[date_cnt+1]='x';
				rxfilename[date_cnt+2]=0;

				if ((file_out_rx = fopen(rxfilename,"wb")) == NULL)
				{
					printf("phonerecord: Unable to open %s \n",rxfilename);
					return(2);
				}
				file2_out = fopen(lenfilename,"wb");
			}
			else if ( (pre_stat==1) && (stat==1) )
			{
				stVoipdataget_o.write_enable=1;
				stVoipdataget_o.mode=0;		//rx
				rtk_Set_GETDATA_Mode(&stVoipdataget_o);
				fwrite(&stVoipdataget_o.length,sizeof(long),1,file2_out);
				while (stVoipdataget_o.length>1024)
				{
					fwrite(stVoipdataget_o.buf,sizeof(char),1120,file_out_rx);
					rtk_Set_GETDATA_Mode(&stVoipdataget_o);
				}
				fwrite(stVoipdataget_o.buf,sizeof(char),stVoipdataget_o.length,file_out_rx);
			}
			else if ( (pre_stat==1) && (stat==0) )
			{
				stVoipdataget_o.write_enable=0;
				stVoipdataget_o.mode=1;		//tx
				rtk_Set_GETDATA_Mode(&stVoipdataget_o);

				fclose(file_out_rx);
				fclose(file2_out);
			}

			nanosleep(&pause_time, 0);
		}
	}
	else if (argc == 3)
	{
		chid = atoi(argv[1]);
		loop_times = atoi(argv[2]);
		stVoipdataget_o.ch_id=chid;

		pause_time.tv_sec=0;
		pause_time.tv_nsec=15000000;

		stVoipCfg.ch_id=chid;

		timer = time(0);
		strftime(date, 11, "%m%d%H%M%S", localtime(&timer));
		strcpy(rxfilename,date);
		strcpy(lenfilename,date);
		strcpy(&lenfilename[date_cnt],"rec_len");
		rxfilename[date_cnt]='r';
		rxfilename[date_cnt+1]='x';
		rxfilename[date_cnt+2]=0;

		if ((file_out_rx = fopen(rxfilename,"wb")) == NULL)
		{
			printf("phonerecord: Unable to open %s \n",rxfilename);
			return(2);
		}
		file2_out = fopen(lenfilename,"wb");


		for (i=0 ; i<loop_times ; i++)
		{
			stVoipdataget_o.write_enable=1;
			stVoipdataget_o.mode=0;		//rx
			rtk_Set_GETDATA_Mode(&stVoipdataget_o);
			fwrite(&stVoipdataget_o.length,sizeof(long),1,file2_out);
			while (stVoipdataget_o.length>1024)
			{
				fwrite(stVoipdataget_o.buf,sizeof(char),1120,file_out_rx);
				rtk_Set_GETDATA_Mode(&stVoipdataget_o);
			}
			fwrite(stVoipdataget_o.buf,sizeof(char),stVoipdataget_o.length,file_out_rx);

			nanosleep(&pause_time, 0);
		}

		stVoipdataget_o.write_enable=0;
		stVoipdataget_o.mode=1;		//tx
		rtk_Set_GETDATA_Mode(&stVoipdataget_o);
		fclose(file_out_rx);
		fclose(file2_out);
	}
	else
	{
		printf("use error! Method1: phonerecord daa_chid & , Method2:phonerecord daa_chid 200 &\n");
		printf("in Method2,200 is loop times\n");
	}

	return 0;
}
