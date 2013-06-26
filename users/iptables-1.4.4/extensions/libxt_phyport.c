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
#include <linux/netfilter/xt_phyport.h>

static void phyport_help(void)
{
	printf(
"phyport match options:\n"
"[!] --port-source XX:XX:XX:XX:XX:XX\n"
"				Match source port number\n"
" --port-destination [!] XX:XX:XX:XX:XX:XX\n"
"				Match destination port number\n");
}

static const struct option phyport_opts[] = {
	{ "port-source", 1, NULL, '1' },
	{ "port-destination", 1, NULL, '2' },
	{ .name = NULL }
};

static void
parse_phyportnum(const char *phyportnum, struct xt_phyport_info *info)
{
	long number;
	char *end;

	if(phyportnum==NULL)
		xtables_error(PARAMETER_PROBLEM, "phy port number pointer is null");
	
	number=strtol(phyportnum, &end, 10);

	if((number<0)||(number>255))
		xtables_error(PARAMETER_PROBLEM, "Bad phy port number `%s'", phyportnum);

	if(info->flags & PORT_SRC){
		info->srcport=(unsigned char)number;
	}
	else if (info->flags & PORT_DST){
		info->dstport=(unsigned char)number;
	}
	else{
		xtables_error(PARAMETER_PROBLEM, "wrong info->flags: 0x%x", info->flags);
	}	
}

static int
phyport_parse(int c, char **argv, int invert, unsigned int *flags,
          const void *entry, struct xt_entry_match **match)
{
	struct xt_phyport_info *phyportinfo = (struct xt_phyport_info *)((*match)->data);
        switch (c) {
        case '1':
                if (*flags & PORT_SRC)
                        xtables_error(PARAMETER_PROBLEM,
                                   "phyport match: Only use --port-source ONCE!");
                *flags |= PORT_SRC;

                phyportinfo->flags |= PORT_SRC;
                xtables_check_inverse(optarg, &invert, &optind, 0);
                if (invert) {
                        phyportinfo->flags |= PORT_SRC_INV;
                }
                parse_phyportnum(optarg, phyportinfo);
                break;

        case '2':
                if (*flags & PORT_DST)
                        xtables_error(PARAMETER_PROBLEM,
                                   "phyport match: Only use --port-destination ONCE!");
                *flags |= PORT_DST;

                phyportinfo->flags |= PORT_DST;
                xtables_check_inverse(optarg, &invert, &optind, 0);
                if (invert){
                        phyportinfo->flags |= PORT_DST_INV;
                }
                parse_phyportnum(optarg, phyportinfo);
                break;

        default:
                return 0;
        }

        return 1;
}

static void print_phyportnum(struct xt_phyport_info *info)
{
	if(info->flags & PORT_SRC){
		printf("%d", info->srcport);
	}
	else if (info->flags & PORT_DST){
		printf("%d", info->dstport);
	}
	else{
		xtables_error(PARAMETER_PROBLEM, "wrong info->flags: 0x%x", info->flags);
	}
		
	printf(" ");
}

static void phyport_check(unsigned int flags)
{
	if (!flags)
		xtables_error(PARAMETER_PROBLEM,
			   "You must specify `--port-source' or `--port-destination'");
}

static void
phyport_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	struct xt_phyport_info *phyportinfo = (struct xt_phyport_info *)(match->data);

        if (phyportinfo->flags & PORT_SRC) {
                printf("source port  ");
                if (phyportinfo->flags & PORT_SRC_INV)
                        printf("! ");
                print_phyportnum(phyportinfo);
        }
        if (phyportinfo->flags & PORT_DST) {
                printf("destination port ");
                if (phyportinfo->flags & PORT_DST_INV)
                        printf("! ");
                print_phyportnum(phyportinfo);
        }
}

static void phyport_save(const void *ip, const struct xt_entry_match *match)
{
	struct xt_phyport_info *phyportinfo = (struct xt_phyport_info *)(match->data);

        if (phyportinfo->flags & PORT_SRC) {
                if (phyportinfo->flags & PORT_SRC_INV)
                        printf("! ");
                printf("--port-source ");
                print_phyportnum(phyportinfo);
                if (phyportinfo->flags & PORT_DST)
                        fputc(' ', stdout);
        }
        if (phyportinfo->flags & PORT_DST) {
                if (phyportinfo->flags & PORT_DST_INV)
                        printf("! ");
                printf("--port-destination ");
                print_phyportnum(phyportinfo);
        }
}

static struct xtables_match phyport_match = {
	.family		= NFPROTO_IPV4,
 	.name		= "phyport",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(sizeof(struct xt_phyport_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_phyport_info)),
	.help		= phyport_help,
	.parse		= phyport_parse,
	.final_check	= phyport_check,
	.print		= phyport_print,
	.save		= phyport_save,
	.extra_opts	= phyport_opts,
};

static struct xtables_match phyport_match6 = {
	.family		= NFPROTO_IPV6,
 	.name		= "phyport",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(sizeof(struct xt_phyport_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_phyport_info)),
	.help		= phyport_help,
	.parse		= phyport_parse,
	.final_check	= phyport_check,
	.print		= phyport_print,
	.save		= phyport_save,
	.extra_opts	= phyport_opts,
};

void _init(void)
{
	xtables_register_match(&phyport_match);
	xtables_register_match(&phyport_match6);
}
