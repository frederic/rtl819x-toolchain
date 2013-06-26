#include "sip_interface.h"

void main()
{
	sip_cfg_t cfg;

	if (sip_cfg_get(&cfg) == 0)
	{
		printf("proxy enable: %d\n", cfg.param[0].proxy_enable);
		printf("proxy port: %d\n", cfg.param[0].proxy_port);
		printf("proxy address: %s\n", cfg.param[0].proxy);
	}
	else
	{
		printf("sip_cfg_get failed.\n");
	}
}
