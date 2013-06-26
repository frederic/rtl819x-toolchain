#include "voip_manager.h"

void ShowUsage(char *cmd)
{
	printf("Usage:\n");
        printf( "* Enable/Disable DTMF det:\n"	\
        	" - %s <chid> <dir> <enable>\n" \
        	"   - dir => 0 : TDM-side, 1: IP-side\n" \
        	"   - enable => 0: disable, 1: enable\n" \
        	"\n* Set DTMF det parameters:\n" \
        	" - %s <chid> <dir> <threshold> <on_time> <fore_twist> <rev_twist>\n" \
        	"   - dir => 0 : TDM-side, 1: IP-side\n" \
                "   - threshold => 0 ~ 40, it means 0 ~ -40 dBm\n" \
                "   - on_time_10ms => 3~12, it means minimum dtmf on time\n" \
                "   - fore_twist => 1 ~ 16, it means 0 ~ 16 dB\n" \
                "   - rev_twist => 1 ~ 16, it means 0 ~ 16 dB\n" , cmd, cmd);
        exit(0);
}

int dtmf_det_cfg_main(int argc, char *argv[])
{

	if (argc == 4)
	{
		rtk_Set_DTMF_CFG(atoi(argv[1]), atoi(argv[3]), atoi(argv[2]));
		printf("set DTMF det = %s, ch%d, dir%d\n", atoi(argv[3]) ? "On" : "Off", atoi(argv[1]), atoi(argv[2]));
	}
	else if (argc == 7)
	{
		rtk_set_dtmf_det_param(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]));
	}
	else
	{
		ShowUsage(argv[0]);
	}

	return 0;
}

