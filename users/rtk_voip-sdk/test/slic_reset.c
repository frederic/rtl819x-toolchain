#include "voip_manager.h"

int slic_reset_main(int argc, char *argv[])
{
	unsigned int codec_law;
	if (argc == 2)
	{
		codec_law = atoi(argv[1]);			
		rtk_reset_slic(0, codec_law);
	}
	else
	{
		printf("Usage: %s codec_law\n", argv[0]);
		printf("codec_law: 0: linear, 1: A-law, 2:u-law\n");
	}
	
	return 0;
}

