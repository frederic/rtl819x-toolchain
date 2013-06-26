#include "voip_manager.h"

int led_main(int argc, char *argv[])
{
	if (argc == 3)
		; //rtk_led_control(atoi(argv[1]),atoi(argv[2]));
	else
		printf("use error! Method: led device state \n");
	
	return 0;
}

