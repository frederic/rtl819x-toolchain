#include "voip_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#define RECORD_PCM_INTERFACE	4
	// voice record the pcm interface , before LEC in RX , and after IVR in TX.
#define RECORD_PCM_HANDLER	1
	// voice record the pcm handler ,

#define RECORD_PCM_LEC		8
#define DUMP_MODE_3		16

#define VOICERECORD_TYPE	RECORD_PCM_LEC

//#define LENGTH_RECORD

int voicerecord_main(int argc, char *argv[])
{
	FILE *file_out_tx, *file_out_rx, *file2_out, *file_out_rx2;
	int chid;
	int rec_time;
	int loop_times;
	int i;
	int pre_stat,stat;
	time_t timer = time(0);
	int date_cnt=10;
	int mode;

	TstVoipdataget_o stVoipdataget_o;
	TstVoipCfg stVoipCfg;
	struct timespec pause_time;

	char date[20],txfilename[20],rxfilename[20],lenfilename[20], rx2filename[20];


	if (argc == 3)
	{
		chid = atoi(argv[1]);
		mode = atoi(argv[2]);

		stVoipdataget_o.ch_id=chid;


		pause_time.tv_sec=0;
		//pause_time.tv_nsec=62500000;
		pause_time.tv_nsec=15000000;



		stVoipCfg.ch_id=chid;

		while(1)
		{
			pre_stat=stat;
			rtk_Set_GetPhoneStat(&stVoipCfg);  //1:offhook, 0:onhook
			stat=stVoipCfg.cfg;

			if( (pre_stat==0) && (stat==1) )
			{
				timer = time(0);
				strftime(date, 11, "%m%d%H%M%S", localtime(&timer));
				strcpy(txfilename,date);
				strcpy(rxfilename,date);
				strcpy(rx2filename,date);
				strcpy(lenfilename,date);
				strcpy(&lenfilename[date_cnt],"rec_len");
				txfilename[date_cnt]='t';
				txfilename[date_cnt+1]='x';
				txfilename[date_cnt+2]=0;
				rxfilename[date_cnt]='r';
				rxfilename[date_cnt+1]='x';
				rxfilename[date_cnt+2]=0;
				rx2filename[date_cnt]='r';
				rx2filename[date_cnt+1]='x';
				rx2filename[date_cnt+2]='2';
				rx2filename[date_cnt+3]=0;

				if ((file_out_tx = fopen(txfilename,"wb")) == NULL)
				{
					printf("voicerecord: Unable to open %s \n",txfilename);
					return(2);
				}
				if ((file_out_rx = fopen(rxfilename,"wb")) == NULL)
				{
					printf("voicerecord: Unable to open %s \n",rxfilename);
					return(2);
				}
				if (mode & DUMP_MODE_3)
				{
	  				if ((file_out_rx2 = fopen(rx2filename,"wb")) == NULL)
	  				{
	  					printf("voicerecord: Unable to open %s \n",rx2filename);
	  					return(2);
	  				}
	  			}
#ifdef LENGTH_RECORD
				file2_out = fopen(lenfilename,"wb");
#endif
				stVoipdataget_o.write_enable=0;
				stVoipdataget_o.mode=1;		//tx
				rtk_Set_GETDATA_Mode(&stVoipdataget_o);
			}
			else if( (pre_stat==1) && (stat==1) )
			{
				stVoipdataget_o.write_enable=mode;
				stVoipdataget_o.mode=1;		//tx
				rtk_Set_GETDATA_Mode(&stVoipdataget_o);
				while(stVoipdataget_o.length>1024)
				{
					fwrite(stVoipdataget_o.buf,sizeof(char),1120,file_out_tx);
					rtk_Set_GETDATA_Mode(&stVoipdataget_o);
				}
				fwrite(stVoipdataget_o.buf,sizeof(char),stVoipdataget_o.length,file_out_tx);
				stVoipdataget_o.mode=0;		//rx
				rtk_Set_GETDATA_Mode(&stVoipdataget_o);
#ifdef LENGTH_RECORD
				fwrite(&stVoipdataget_o.length,sizeof(long),1,file2_out);
#endif
				while(stVoipdataget_o.length>1024)
				{
					fwrite(stVoipdataget_o.buf,sizeof(char),1120,file_out_rx);
					rtk_Set_GETDATA_Mode(&stVoipdataget_o);
				}
				fwrite(stVoipdataget_o.buf,sizeof(char),stVoipdataget_o.length,file_out_rx);

				if (mode & DUMP_MODE_3)
				{
					stVoipdataget_o.mode=2;		//rx2
					rtk_Set_GETDATA_Mode(&stVoipdataget_o);
					while(stVoipdataget_o.length>1024)
					{
						fwrite(stVoipdataget_o.buf,sizeof(char),1120,file_out_rx2);
						rtk_Set_GETDATA_Mode(&stVoipdataget_o);
					}
					fwrite(stVoipdataget_o.buf,sizeof(char),stVoipdataget_o.length,file_out_rx2);
				}
			}
			else if( (pre_stat==1) && (stat==0) )
			{
				stVoipdataget_o.write_enable=0;
				stVoipdataget_o.mode=1;		//tx
				rtk_Set_GETDATA_Mode(&stVoipdataget_o);

				fclose(file_out_tx);
				fclose(file_out_rx);
#ifdef LENGTH_RECORD
				fclose(file2_out);
#endif
				if (mode & DUMP_MODE_3)
					fclose(file_out_rx2);
			}

			nanosleep(&pause_time, 0);
		}
	}
	else if (argc == 4)
	{
		chid = atoi(argv[1]);
		stVoipdataget_o.ch_id=chid;
		mode = atoi(argv[2]);
		loop_times = atoi(argv[3]);


		pause_time.tv_sec=0;
		pause_time.tv_nsec=15000000;


		timer = time(0);
		strftime(date, 11, "%m%d%H%M%S", localtime(&timer));
		strcpy(txfilename,date);
		strcpy(rxfilename,date);
		strcpy(rx2filename,date);
		strcpy(lenfilename,date);
		strcpy(&lenfilename[date_cnt],"rec_len");
		txfilename[date_cnt]='t';
		txfilename[date_cnt+1]='x';
		txfilename[date_cnt+2]=0;
		rxfilename[date_cnt]='r';
		rxfilename[date_cnt+1]='x';
		rxfilename[date_cnt+2]=0;
		rx2filename[date_cnt]='r';
		rx2filename[date_cnt+1]='x';
		rx2filename[date_cnt+2]='2';
		rx2filename[date_cnt+3]=0;


		if ((file_out_tx = fopen(txfilename,"wb")) == NULL)
		{
			printf("voicerecord: Unable to open %s \n",txfilename);
			return(2);
		}
		if ((file_out_rx = fopen(rxfilename,"wb")) == NULL)
		{
			printf("voicerecord: Unable to open %s \n",rxfilename);
			return(2);
		}
#ifdef LENGTH_RECORD
		file2_out = fopen(lenfilename,"wb");
#endif
		if (mode & DUMP_MODE_3)
		{
			if ((file_out_rx2 = fopen(rx2filename,"wb")) == NULL)
			{
				printf("voicerecord: Unable to open %s \n",rx2filename);
				return(2);
			}
		}

		for(i=0;i<loop_times;i++)
		{
			stVoipdataget_o.write_enable=mode;
			stVoipdataget_o.mode=1;		//tx
			rtk_Set_GETDATA_Mode(&stVoipdataget_o);
			while(stVoipdataget_o.length>1024)
			{
				fwrite(stVoipdataget_o.buf,sizeof(char),1120,file_out_tx);
				rtk_Set_GETDATA_Mode(&stVoipdataget_o);
			}
			fwrite(stVoipdataget_o.buf,sizeof(char),stVoipdataget_o.length,file_out_tx);
			stVoipdataget_o.mode=0;		//rx
			rtk_Set_GETDATA_Mode(&stVoipdataget_o);
#ifdef LENGTH_RECORD
			fwrite(&stVoipdataget_o.length,sizeof(long),1,file2_out);
#endif
			while(stVoipdataget_o.length>1024)
			{
				fwrite(stVoipdataget_o.buf,sizeof(char),1120,file_out_rx);
				rtk_Set_GETDATA_Mode(&stVoipdataget_o);
			}
			fwrite(stVoipdataget_o.buf,sizeof(char),stVoipdataget_o.length,file_out_rx);

			if (mode & DUMP_MODE_3)
			{
				stVoipdataget_o.mode=2;		//rx2
				rtk_Set_GETDATA_Mode(&stVoipdataget_o);
				while(stVoipdataget_o.length>1024)
				{
					fwrite(stVoipdataget_o.buf,sizeof(char),1120,file_out_rx2);
					rtk_Set_GETDATA_Mode(&stVoipdataget_o);
				}
				fwrite(stVoipdataget_o.buf,sizeof(char),stVoipdataget_o.length,file_out_rx2);
			}

			nanosleep(&pause_time, 0);
		}

		stVoipdataget_o.write_enable=0;
		stVoipdataget_o.mode=1;		//tx
		rtk_Set_GETDATA_Mode(&stVoipdataget_o);

		fclose(file_out_tx);
		fclose(file_out_rx);
#ifdef LENGTH_RECORD
		fclose(file2_out);
#endif
		if (mode & DUMP_MODE_3)
			fclose(file_out_rx2);
	}
	else
	{
		printf("use error! Method1: voicerecord chid mode & \n");
		printf("Method2:voicerecord chid mode 200 &\n");
		printf("in Method2,200 is loop times\n");
		printf("mode: DUMP_MODE_3 | [RECORD_PCM_HANDLER | RECORD_TX_SINE_RX_ECHO | RECORD_PCM_INTERFACE | RECORD_PCM_LEC] | RECORD_PLAY_TONE\n");
		printf("RECORD_PCM_HANDLER: 1, RECORD_TX_SINE_RX_ECHO: 2, RECORD_PCM_INTERFACE: 4, RECORD_PCM_LEC: 8,\n");
		printf("DUMP_MODE_3: 16, RECORD_PLAY_TONE: 32 \n");
	}

	return 0;
}


