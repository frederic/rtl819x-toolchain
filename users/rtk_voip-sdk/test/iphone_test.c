#include "voip_manager.h"
#include <stdlib.h>

int iphone_test_main(int argc, char *argv[])
{
	if (argc == 4)
		rtk_Set_IPhone(atoi(argv[1]),atoi(argv[2]),strtol(argv[3],NULL,16));
	else if (argc == 3) {
		if (atoi(argv[1]) == 1)
			rtk_Set_IPhone(atoi(argv[1]),atoi(argv[2]),0);
		else if (atoi(argv[1]) == 4)
			rtk_Set_IPhone(atoi(argv[1]),strtol(argv[2],NULL,16),0);	
	} else if (argc == 2)
		rtk_Set_IPhone(atoi(argv[1]),0,0);
	else {
		printf("use error! Method: iphone_test function reg value\n");
		printf("function:\n");
		printf("		0:write\n");
		printf("		1:read\n");
		printf("		2:loopback enable\n");
		printf("		3:loopback disable\n");
		printf("		4:led show\n");
	}	
	return 0;
}

