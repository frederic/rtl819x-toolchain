/* Kernel module to match Realsil extension functions. */

/* (C) 2009-2011 ZhaoBo <bo_zhao@realsil.com.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/skbuff.h>

#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/xt_vlanpriority.h>

#if defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl865x_netif.h>
#endif


MODULE_LICENSE("GPL");
MODULE_AUTHOR("ZhaoBo <bo_zhao@realsil.com.cn>");
MODULE_DESCRIPTION("iptables realsil extensions matching module");
MODULE_ALIAS("ipt_vlanpriority");
MODULE_ALIAS("ip6t_vlanpriority");

static bool
match(const struct sk_buff *skb, const struct xt_match_param *par)
{
	const struct xt_vlanpriority_info *info = par->matchinfo;

	return (skb->srcVlanPriority== info->priority) ^ info->invert;
}

static bool
checkentry(const struct xt_mtchk_param *par)
{
	const struct xt_vlanpriority_info *minfo = par->matchinfo;

	if (minfo->priority > 7) {
		printk(KERN_WARNING "vlan priority: only supports priority 0~7\n");
		return 0;
	}
	return 1;
}

#ifdef CONFIG_COMPAT
struct compat_xt_vlanpriority_info {
	u_int8_t	priority;
	u_int8_t	invert;
	u_int16_t	__pad2;
};

static void compat_from_user(void *dst, void *src)
{
	struct compat_xt_vlanpriority_info *cm = src;
	struct xt_vlanpriority_info m = {
		.priority	= cm->priority,
		.invert	= cm->invert,
	};
	memcpy(dst, &m, sizeof(m));
}

static int compat_to_user(void __user *dst, void *src)
{
	struct xt_vlanpriority_info *m = src;
	struct compat_xt_vlanpriority_info cm = {
		.priority	= cm->priority,
		.invert	= cm->invert,
	};
	return copy_to_user(dst, &cm, sizeof(cm)) ? -EFAULT : 0;
}
#endif /* CONFIG_COMPAT */

#if defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
static int vlanpriority_match2acl(const char *tablename,
			  const void *ip,
			  const struct xt_match *match,
			  void *matchinfo,
			  void *acl_rule,
			  unsigned int *invflags)
{

	const struct xt_vlanpriority_info *info = matchinfo;
	rtl865x_AclRule_t *rule = (rtl865x_AclRule_t *)acl_rule;

	if(matchinfo == NULL || rule == NULL)
		return 1;

#if	defined(CONFIG_RTL_QOS_8021P_SUPPORT)
	rule->ruleType_ = RTL865X_ACL_802D1P;
	rule->vlanTagPri_ = info->priority;
#endif

	return 0;
}
#endif

static struct xt_match xt_vlanpriority_match[] = {
	{
		.name		= "vlanpriority",
		.family		= AF_INET,
		.checkentry	= checkentry,
		.match		= match,
		.matchsize	= sizeof(struct xt_vlanpriority_info),
#ifdef CONFIG_COMPAT
		.compatsize	= sizeof(struct compat_xt_vlanpriority_info),
		.compat_from_user = compat_from_user,
		.compat_to_user	= compat_to_user,
#endif
#if defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
		.match2acl	= vlanpriority_match2acl,
#endif
		.me		= THIS_MODULE,
	},
	{
		.name		= "vlanpriority",
		.family		= AF_INET6,
		.checkentry	= checkentry,
		.match		= match,
		.matchsize	= sizeof(struct xt_vlanpriority_info),
		.me		= THIS_MODULE,
	},
};

static int __init xt_vlanpriority_init(void)
{
	return xt_register_matches(xt_vlanpriority_match, ARRAY_SIZE(xt_vlanpriority_match));
}

static void __exit xt_vlanpriority_fini(void)
{
	xt_unregister_matches(xt_vlanpriority_match, ARRAY_SIZE(xt_vlanpriority_match));
}

module_init(xt_vlanpriority_init);
module_exit(xt_vlanpriority_fini);
