/* Shared library add-on to iptables to add VALN priority support. */
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
#include <linux/netfilter/xt_vlanpriority.h>

/* Function which prints out usage message. */
static void
help(void)
{
	printf(
"VLAN priority options:\n"
" --prio-value [!] XX\n"
"Match VLAN priority\n"
"\n");
}

static struct option opts[] = {
	{ "prio-value", 1, 0, '1' },
	{0}
};

static void
parse_vlanpriority(const char *vlanpriority, struct xt_vlanpriority_info *info)
{
	long number;
	char *end;

	if(vlanpriority==NULL)
		xtables_error(PARAMETER_PROBLEM, "vlanpriority pointer is null");
	
	number=strtol(vlanpriority, &end, 10);
//	printf("%s(%d): number=%d\n",__FUNCTION__,__LINE__, number);//Added for test

	if((number<0)||(number>255))
		xtables_error(PARAMETER_PROBLEM, "Bad vlan priority `%s'", vlanpriority);

	info->priority=(unsigned char)number;
}

/* Function which parses command options; returns true if it
   ate an option */
static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const void *entry,
      struct xt_entry_match **match)
{
	struct xt_vlanpriority_info *vlanPrioInfo = (struct xt_vlanpriority_info *)(*match)->data;

	switch (c) {
	case '1':
		xtables_check_inverse(optarg, &invert, &optind, 0);
		parse_vlanpriority(argv[optind-1], vlanPrioInfo);
		if (invert)
			vlanPrioInfo->invert = 1;
		*flags = 1;
		break;

	default:
		return 0;
	}

	return 1;
}

static void print_vlanpriority(unsigned char vlanpriority)
{
	printf("%u ", vlanpriority);
}

/* Final check; must have specified --prio. */
static void final_check(unsigned int flags)
{
	if (!flags)
		xtables_error(PARAMETER_PROBLEM,
			   "You must specify `--prio-value'");
}

/* Prints out the vlanPrioInfo. */
static void
print(const void *ip,
      const struct xt_entry_match *match,
      int numeric)
{
	printf("VLAN priority ");

	if (((struct xt_vlanpriority_info *)match->data)->invert)
		printf("! ");
	
	print_vlanpriority(((struct xt_vlanpriority_info *)match->data)->priority);
}

/* Saves the union ipt_matchinfo in parsable form to stdout. */
static void save(const void *ip, const struct xt_entry_match *match)
{
	if (((struct xt_vlanpriority_info *)match->data)->invert)
		printf("! ");

	printf("--prio-value ");
	print_vlanpriority(((struct xt_vlanpriority_info *)match->data)->priority);
}

static struct xtables_match vlanpriority = { 
 	.name		= "vlanpriority",
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_IPV4,
	.size		= XT_ALIGN(sizeof(struct xt_vlanpriority_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_vlanpriority_info)),
	.help		= &help,
	.parse		= &parse,
	.final_check	= &final_check,
	.print		= &print,
	.save		= &save,
	.extra_opts	= opts
};

void _init(void)
{
	xtables_register_match(&vlanpriority);
}

