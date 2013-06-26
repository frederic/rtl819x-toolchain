/* Shared library add-on to iptables to add MAC address support. */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#if defined(__GLIBC__) && __GLIBC__ == 2
#include <net/ethernet.h>
#else
#include <linux/if_ether.h>
#endif
#include <xtables.h>
#include <linux/netfilter/xt_mac.h>

static void mac_help(void)
{
	printf(
"mac match options:\n"
"[!] --mac-source XX:XX:XX:XX:XX:XX\n"
"				Match source MAC address\n"
" --mac-destination [!] XX:XX:XX:XX:XX:XX\n"
"				Match destination MAC address\n");
}

static const struct option mac_opts[] = {
	{ "mac-source", 1, NULL, '1' },
	{ "mac-destination", 1, NULL, '2' },
	{ .name = NULL }
};

static void
parse_mac(const char *mac, struct xt_mac *info)
{
	unsigned int i = 0;

	if (strlen(mac) != ETH_ALEN*3-1)
		xtables_error(PARAMETER_PROBLEM, "Bad mac address \"%s\"", mac);

	for (i = 0; i < ETH_ALEN; i++) {
		long number;
		char *end;

		number = strtol(mac + i*3, &end, 16);

		if (end == mac + i*3 + 2
		    && number >= 0
		    && number <= 255)
		{
			info->macaddr[i] = number;
		}
		else
			xtables_error(PARAMETER_PROBLEM,
				   "Bad mac address `%s'", mac);
	}

}

static int
mac_parse(int c, char **argv, int invert, unsigned int *flags,
          const void *entry, struct xt_entry_match **match)
{
	struct xt_mac_info *macinfo = (struct xt_mac_info *)(*match)->data;
        switch (c) {
        case '1':
                if (*flags & MAC_SRC)
                        xtables_error(PARAMETER_PROBLEM,
                                   "mac match: Only use --mac-source ONCE!");
                *flags |= MAC_SRC;

                macinfo->flags |= MAC_SRC;
                xtables_check_inverse(optarg, &invert, &optind, 0);
                if (invert) {
                        macinfo->flags |= MAC_SRC_INV;
                }
                parse_mac(optarg, &macinfo->srcaddr);
                break;

        case '2':
                if (*flags & MAC_DST)
                        xtables_error(PARAMETER_PROBLEM,
                                   "mac match: Only use --mac-destination ONCE!");
                *flags |= MAC_DST;

                macinfo->flags |= MAC_DST;
                xtables_check_inverse(optarg, &invert, &optind, 0);
                if (invert){
                        macinfo->flags |= MAC_DST_INV;
                }
                parse_mac(optarg, &macinfo->dstaddr);
                break;

        default:
                return 0;
        }

        return 1;
}

static void print_mac(struct xt_mac *info)
{
	unsigned int i;

	printf("%02X", info->macaddr[0]);
	for (i = 1; i < ETH_ALEN; i++)
		printf(":%02X", info->macaddr[i]);
	printf(" ");
}

static void mac_check(unsigned int flags)
{
	if (!flags)
		xtables_error(PARAMETER_PROBLEM,
			   "You must specify `--mac-source' or `--mac-destination'");
}

static void
mac_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	struct xt_mac_info *macinfo = (struct xt_mac_info *)(match->data);

        if (macinfo->flags & MAC_SRC) {
                printf("source MAC  ");
                if (macinfo->flags & MAC_SRC_INV)
                        printf("! ");
                print_mac(&macinfo->srcaddr);
        }
        if (macinfo->flags & MAC_DST) {
                printf("destination MAC ");
                if (macinfo->flags & MAC_DST_INV)
                        printf("! ");
                print_mac(&macinfo->dstaddr);
        }
}

static void mac_save(const void *ip, const struct xt_entry_match *match)
{

	        struct xt_mac_info *macinfo = (struct xt_mac_info *)(match->data);

        if (macinfo->flags & MAC_SRC) {
                if (macinfo->flags & MAC_SRC_INV)
                        printf("! ");
                printf("--mac-source ");
                print_mac(&macinfo->srcaddr);
                if (macinfo->flags & MAC_DST)
                        fputc(' ', stdout);
        }
        if (macinfo->flags & MAC_DST) {
                if (macinfo->flags & MAC_DST_INV)
                        printf("! ");
                printf("--mac-destination ");
                print_mac(&macinfo->dstaddr);
        }
}

static struct xtables_match mac_match = {
	.family		= NFPROTO_IPV4,
 	.name		= "mac",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(sizeof(struct xt_mac_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_mac_info)),
	.help		= mac_help,
	.parse		= mac_parse,
	.final_check	= mac_check,
	.print		= mac_print,
	.save		= mac_save,
	.extra_opts	= mac_opts,
};

static struct xtables_match mac_match6 = {
	.family		= NFPROTO_IPV6,
 	.name		= "mac",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(sizeof(struct xt_mac_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_mac_info)),
	.help		= mac_help,
	.parse		= mac_parse,
	.final_check	= mac_check,
	.print		= mac_print,
	.save		= mac_save,
	.extra_opts	= mac_opts,
};

void _init(void)
{
	xtables_register_match(&mac_match);
	xtables_register_match(&mac_match6);
}
