#include "voip_manager.h"

#define NULL ((void *)0)

int gpio_main(int argc, char *argv[])
{
	int i;
	unsigned long pid;
	unsigned long value;

	if (strcmp(argv[0], "gpio_init") == 0)
	{
		if (argc != 5)
		{
			printf("use: gpio_init <pid> <type> <dir> <interrupt>\n");
			printf("  - pid (hex format): (gpio_group << 16) | (pin & 0xffff)\n");
			printf("  - type: 0=normal, 1=type1, 2=type2\n");
			printf("  - dir: 0=in, 1=out\n");
			printf("  - interrupt: 0=disable, 1=falling edge, 2=rising edge, 3=both edge\n");
			return 0;
		}

		pid = strtol(argv[1], NULL, 16);
		value = (atoi(argv[2]) & 0x03) | (atoi(argv[3]) << 2 & 0x04) | (atoi(argv[4]) << 3 & 0x08);
		i = rtk_gpio(0, pid, value, NULL);
		if (i == 0)
			printf("gpio_init ok: 0x%08X = 0x%08X\n", pid, value);
		else
			printf("gpio_init failed.\n");
	}
	else if (strcmp(argv[0], "gpio_read") == 0)
	{
		if (argc != 2)
		{
			printf("use: gpio_read <pid>\n");
			printf("  - pid (hex format): (gpio_group << 16) | (pin & 0xffff)\n");
			return 0;
		}

		pid = strtol(argv[1], NULL, 16);
		i = rtk_gpio(1, pid, 0, &value);
		if (i == 0)
			printf("gpio_read ok: 0x%08X = 0x%08X\n", pid, value);
		else
			printf("gpio_read failed.\n");
	}
	else if (strcmp(argv[0], "gpio_write") == 0)
	{
		if (argc != 3)
		{
			printf("use: gpio_write <pid> <value>\n");
			printf("  - pid (hex format): (gpio_group << 16) | (pin & 0xffff)\n");
			printf("  - value: 0 or 1\n");
			return 0;
		}

		pid = strtol(argv[1], NULL, 16);
		value = atoi(argv[2]);
		i = rtk_gpio(2, pid, value, NULL);
		if (i == 0)
			printf("gpio_write ok: 0x%08X = 0x%08X\n", pid, value);
		else
			printf("gpio_write failed.\n");
	}

	return 0;
}

