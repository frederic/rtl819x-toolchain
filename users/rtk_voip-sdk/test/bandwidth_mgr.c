#include "voip_manager.h"
#include<stdio.h>
static unsigned int Value(char c);
int bandwidth_mgr_main(int argc, char* argv[])
{
	if(argc != 4)
	{
		printf("use of bandwidth_mgr: Port Direction Bandwidth\n");
		printf("Direction: RX: 0 TX: 1\n");
		printf("Bandwidth (0: unlimit)\n");
		printf("RX: Unit 16kbps range 0~65535\n");
		printf("TX: Unit 64kbps range 0~16383\n");
		return 1;
	}
	if ((atoi(argv[1]) > 5) || (atoi(argv[1])<0))
	{
		printf("Port (0~5)\n");
		return 1;
	}
	else if((atoi(argv[2]) > 1)||(atoi(argv[2])<0))
	{
		printf("Direction(0/1)\n");
		return 1;
	}
	else if((atoi(argv[3]) > 65535)||(atoi(argv[3])<0))
	{
		printf("Bandwidth (0~16383/65535)\n");
		return 1;
	}
	else if((atoi(argv[2])==0)&&(atoi(argv[3])>65535))
	{
		printf("Ingress Bandwidth (0~65535)\n");
		return 	1;
	}
	else if((atoi(argv[2])==1)&&(atoi(argv[3])>16383))
	{
		printf("Egress Bandwidth (0~16383)\n");
		return 	1;
	}

	rtk_Bandwidth_Mgr(atoi(argv[1]),atoi(argv[2]),atoi(argv[3]));
	return 0;
}




