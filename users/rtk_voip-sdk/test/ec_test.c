/*
 *	Realtek echo canceller test program
 *
 *	Author : jwsyu@realtek.com
 *
 *	2010.05.11
 *
 *	Copyright 2010 Realtek Semiconductor Corp.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "voip_manager.h"

#define FRAME_SIZE 80


void usage(void)
{

	printf("ec_test mode filename\n");
	printf("mode: 0->lec, 2->ec128\n");
	printf("inputfile-> [filename input.dat]\n");
	printf("echofile-> [filename echo.dat]\n");
}

int ec_test_main(int argc, char *argv[])
{

	FILE *file_input, *file_echo, *file_echo_estimate, *file_echo_clean, *file_outecho;

	
	char inputfile[30];
	char echofile[30];
	char outputfile[30];
	char name_length;

	char inputfilename[20]="input.dat";
	char echofilename[20]="echo.dat";
	char outputfilename[20]="output.dat";
	char outechofilename[20]="outecho.dat";
	short inbuf[FRAME_SIZE],echobuf[FRAME_SIZE];
	short outbuf[FRAME_SIZE];
	int test_mode;


	TstVoipEcDebug stVoipEcDebug;

	int32_t taps;

	if (argc !=3) {
		usage();
		goto error;
	}

	name_length=strlen(argv[2]);
	if (name_length > 12) {
		printf("name_length > 12");
		goto error;
	}
	strcpy(inputfile, argv[2]);
	strcpy(outputfile, argv[2]);
	strcpy(echofile, argv[2]);

	strcat(inputfile, inputfilename);
	strcat(echofile, echofilename);
	strcat(outputfile, outputfilename);

	if ((file_input = fopen(inputfile,"rb")) == NULL) {
		printf("ec_test: Unable to open %s \n",inputfilename);
		return(2);
	}
	if ((file_echo = fopen(echofile,"rb")) == NULL) {
		printf("ec_test: Unable to open %s \n",echofilename);
		return(2);
	}
	if ((file_echo_clean = fopen(outputfile,"wb")) == NULL) {
		printf("ec_test: Unable to open %s \n",outputfilename);
		return(2);
	}

	/*if ((file_outecho = fopen(outechofilename,"wb")) == NULL) {
			printf("ec_test: Unable to open %s \n",outechofilename);
			return(2);
	}
	*/

	test_mode = atoi(argv[1]);

	stVoipEcDebug.mode = test_mode;
	rtk_Set_GET_EC_DEBUG(&stVoipEcDebug);

	printf("init OK\n");

	stVoipEcDebug.mode = 1;

	while (taps = fread(stVoipEcDebug.buf1, sizeof(char), 2*FRAME_SIZE, file_input)) {
		//printf("init OK2\n");
		fread(stVoipEcDebug.buf2, sizeof(char), 2*FRAME_SIZE, file_echo);

		//printf("init OK3\n");
		if (taps==(FRAME_SIZE*2)) {

			rtk_Set_GET_EC_DEBUG(&stVoipEcDebug);
			fwrite(stVoipEcDebug.buf1,sizeof(char),2*FRAME_SIZE,file_echo_clean);
			//fwrite(stVoipEcDebug.buf2,sizeof(char),2*FRAME_SIZE,file_outecho);
		}


	}

	fclose(file_input);
	fclose(file_echo);
	fclose(file_echo_clean);
	//fclose(file_outecho);

error:

	return 0;
}

