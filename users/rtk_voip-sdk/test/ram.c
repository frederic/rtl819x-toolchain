#include "voip_manager.h"

int ram_main(int argc, char *argv[])
{
	if (argc == 3)
	{
		int chid, reg;
		int reg_start, reg_end;

		chid = atoi(argv[1]);

		if (argv[2][0] == 'r')
		{
			rtk_reset_slic(chid, 1); //1: set to A-law
			printf("reset slic %d done\n", chid);
		}
		else
		{
			reg = atoi(argv[2]);
			
			if( reg < 0 ) {
				reg_start = 0;
				reg_end = reg * ( -1 );
			} else {
				reg_start = reg_end = reg;
			}
			
			while( reg_start <= reg_end ) {
				printf("read: chid = %d, ram =%d, val = %x\n", 
					chid, reg_start, rtk_Get_SLIC_Ram_Val(chid, reg_start));
				
				reg_start ++;
			}
		}
	}
	else if (argc == 4)
	{
		int chid, reg, val;

		chid = atoi(argv[1]);
		reg = atoi(argv[2]);
		val = atoi(argv[3]);

		rtk_Set_SLIC_Ram_Val(chid, reg, val);
		printf("write: chid = %d, ram = %d, val = %x\n", 
			chid, reg, val);
	}
	else
	{
		printf("use: %s chid ram_num [ram_val]\n", argv[0]);
	}

	return 0;
}

