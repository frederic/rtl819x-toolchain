#include <sys/time.h>
#include "voip_manager.h"

int vmwigen_main(int argc, char *argv[])
{
	if (argc == 4)
	{
		char vmwi_on = 1, vmwi_off = 0;
		int i;
		
		/* fsk area -> 0:Bellcore 1:ETSI 2:BT 3:NTT */
		rtk_Set_FSK_Area(atoi(argv[1])/*chid*/, atoi(argv[2])/*area*/);
		
		if (atoi(argv[3]) == 1) //on
		{
			/* Set VMWI message on in on_hook mode */
			rtk_Gen_FSK_VMWI( atoi(argv[1])/*chid*/, &vmwi_on, 0);
		}
		else if (atoi(argv[3]) == 0) //off
		{
			/* Set VMWI message off in on_hook mode */
			rtk_Gen_FSK_VMWI( atoi(argv[1])/*chid*/, &vmwi_off, 0);
		}
	}
	else
	{
		printf("use error! Method: vmwigen chid fsk_area on(1)/off(0)\n");
		printf("Note:\n");
		printf(" - fsk_area: 0 -> Bellcore, 1 -> ETSI, 2 -> BT, 3 -> NTT\n");
	}

	return 0;
}

