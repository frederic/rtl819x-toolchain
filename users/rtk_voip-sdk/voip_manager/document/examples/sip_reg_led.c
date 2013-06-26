#include <stdio.h>
#include <stdlib.h>
#include "voip_manager.h"

void ShowUsage(char *cmd)
{
	printf("usage: %s <chid> <register status> \n" \
	       " - register status => 0: SIP not register \n" \
	       "                      1: SIP registered \n", cmd);
	exit(0);
}

int main(int argc, char *argv[])
{
	unsigned int ch, reg;

	if (argc < 3)
		ShowUsage(argv[0]);
	else
	{
		ch = atoi(argv[1]);
		reg = atoi(argv[2]);
		rtk_sip_register(ch, reg);
	}

	return 0;
}
