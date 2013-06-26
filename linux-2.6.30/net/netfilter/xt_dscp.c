/* IP tables module for matching the value of the IPv4/IPv6 DSCP field
 *
 * (C) 2002 by Harald Welte <laforge@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <net/dsfield.h>

#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/xt_dscp.h>
#include <linux/netfilter_ipv4/ipt_tos.h>

#if defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl865x_netif.h>
#endif


MODULE_AUTHOR("Harald Welte <laforge@netfilter.org>");
MODULE_DESCRIPTION("Xtables: DSCP/TOS field match");
MODULE_LICENSE("GPL");
MODULE_ALIAS("ipt_dscp");
MODULE_ALIAS("ip6t_dscp");
MODULE_ALIAS("ipt_tos");
MODULE_ALIAS("ip6t_tos");

static bool
dscp_mt(const struct sk_buff *skb, const struct xt_match_param *par)
{
	const struct xt_dscp_info *info = par->matchinfo;
	u_int8_t dscp = ipv4_get_dsfield(ip_hdr(skb)) >> XT_DSCP_SHIFT;

	return (dscp == info->dscp) ^ !!info->invert;
}

#if defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
static int dscp_match2acl(const char *tablename,
			  const void *ip,
			  const struct xt_match *match,
			  void *matchinfo,
			  void *acl_rule,
			  unsigned int *invflags)
{

	const struct xt_dscp_info *info = matchinfo;
	rtl865x_AclRule_t *rule = (rtl865x_AclRule_t *)acl_rule;

	if(ip == NULL || matchinfo == NULL || rule == NULL)
		return 1;
	
	rule->ruleType_ = RTL865X_ACL_IP;
	rule->tos_ = ipv4_get_dsfield((struct iphdr *)ip);
	rule->tosMask_ = XT_DSCP_MASK;

	return 0;
}
#endif

static bool
dscp_mt6(const struct sk_buff *skb, const struct xt_match_param *par)
{
	const struct xt_dscp_info *info = par->matchinfo;
	u_int8_t dscp = ipv6_get_dsfield(ipv6_hdr(skb)) >> XT_DSCP_SHIFT;

	return (dscp == info->dscp) ^ !!info->invert;
}

static bool dscp_mt_check(const struct xt_mtchk_param *par)
{
	const struct xt_dscp_info *info = par->matchinfo;

	if (info->dscp > XT_DSCP_MAX) {
		printk(KERN_ERR "xt_dscp: dscp %x out of range\n", info->dscp);
		return false;
	}

	return true;
}

static bool
tos_mt_v0(const struct sk_buff *skb, const struct xt_match_param *par)
{
	const struct ipt_tos_info *info = par->matchinfo;

	return (ip_hdr(skb)->tos == info->tos) ^ info->invert;
}

static bool tos_mt(const struct sk_buff *skb, const struct xt_match_param *par)
{
	const struct xt_tos_match_info *info = par->matchinfo;

	if (par->match->family == NFPROTO_IPV4)
		return ((ip_hdr(skb)->tos & info->tos_mask) ==
		       info->tos_value) ^ !!info->invert;
	else
		return ((ipv6_get_dsfield(ipv6_hdr(skb)) & info->tos_mask) ==
		       info->tos_value) ^ !!info->invert;
}

static struct xt_match dscp_mt_reg[] __read_mostly = {
	{
		.name		= "dscp",
		.family		= NFPROTO_IPV4,
		.checkentry	= dscp_mt_check,
		.match		= dscp_mt,
		.matchsize	= sizeof(struct xt_dscp_info),
		.me		= THIS_MODULE,
#if defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
		.match2acl	= dscp_match2acl,
#endif

	},
	{
		.name		= "dscp",
		.family		= NFPROTO_IPV6,
		.checkentry	= dscp_mt_check,
		.match		= dscp_mt6,
		.matchsize	= sizeof(struct xt_dscp_info),
		.me		= THIS_MODULE,
	},
	{
		.name		= "tos",
		.revision	= 0,
		.family		= NFPROTO_IPV4,
		.match		= tos_mt_v0,
		.matchsize	= sizeof(struct ipt_tos_info),
		.me		= THIS_MODULE,
	},
	{
		.name		= "tos",
		.revision	= 1,
		.family		= NFPROTO_IPV4,
		.match		= tos_mt,
		.matchsize	= sizeof(struct xt_tos_match_info),
		.me		= THIS_MODULE,
	},
	{
		.name		= "tos",
		.revision	= 1,
		.family		= NFPROTO_IPV6,
		.match		= tos_mt,
		.matchsize	= sizeof(struct xt_tos_match_info),
		.me		= THIS_MODULE,
	},
};

static int __init dscp_mt_init(void)
{
	return xt_register_matches(dscp_mt_reg, ARRAY_SIZE(dscp_mt_reg));
}

static void __exit dscp_mt_exit(void)
{
	xt_unregister_matches(dscp_mt_reg, ARRAY_SIZE(dscp_mt_reg));
}

module_init(dscp_mt_init);
module_exit(dscp_mt_exit);
