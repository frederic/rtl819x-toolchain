#include "voip_manager.h"
#include <stdlib.h>

int switchmii_main(int argc, char *argv[])
{
	printf("use switchmii_main\n");
	if (argc == 4)
	{
		rtk_8305_switch(atoi(argv[1]),atoi(argv[2]),strtol(argv[3],NULL,16),1);
	} else if (argc == 3) {
		rtk_8305_switch(atoi(argv[1]),atoi(argv[2]),0,0);
	} else if (argc == 2) {
		rtk_8305_switch(0,0,0,atoi(argv[1]));
	} else if (argc == 1) {
		rtk_8305_switch(0,0,0,4);
	}
	else
		printf("use error! Method: switchmii phy reg value r_w \n");
	
	return 0;
}

