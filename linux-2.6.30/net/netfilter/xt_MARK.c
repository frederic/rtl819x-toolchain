/*
 *	xt_MARK - Netfilter module to modify the NFMARK field of an skb
 *
 *	(C) 1999-2001 Marc Boucher <marc@mbsi.ca>
 *	Copyright © CC Computer Consultants GmbH, 2007 - 2008
 *	Jan Engelhardt <jengelh@computergmbh.de>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <net/checksum.h>

#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/xt_MARK.h>
#if	defined(CONFIG_RTL_HW_QOS_SUPPORT) && defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
#include <linux/netfilter_ipv4/ip_tables.h>
#include <net/pkt_cls.h>
#include <net/rtl/rtl865x_netif.h>
#include <net/rtl/rtl865x_outputQueue.h>
#endif

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Marc Boucher <marc@mbsi.ca>");
MODULE_DESCRIPTION("Xtables: packet mark modification");
MODULE_ALIAS("ipt_MARK");
MODULE_ALIAS("ip6t_MARK");

static unsigned int
mark_tg_v0(struct sk_buff *skb, const struct xt_target_param *par)
{
	const struct xt_mark_target_info *markinfo = par->targinfo;

	skb->mark = markinfo->mark;
	return XT_CONTINUE;
}

static unsigned int
mark_tg_v1(struct sk_buff *skb, const struct xt_target_param *par)
{
	const struct xt_mark_target_info_v1 *markinfo = par->targinfo;
	int mark = 0;

	switch (markinfo->mode) {
	case XT_MARK_SET:
		mark = markinfo->mark;
		break;

	case XT_MARK_AND:
		mark = skb->mark & markinfo->mark;
		break;

	case XT_MARK_OR:
		mark = skb->mark | markinfo->mark;
		break;
	}

	skb->mark = mark;
	return XT_CONTINUE;
}

static unsigned int
mark_tg(struct sk_buff *skb, const struct xt_target_param *par)
{
	const struct xt_mark_tginfo2 *info = par->targinfo;

	skb->mark = (skb->mark & ~info->mask) ^ info->mark;
	return XT_CONTINUE;
}

static bool mark_tg_check_v0(const struct xt_tgchk_param *par)
{
	const struct xt_mark_target_info *markinfo = par->targinfo;

	if (markinfo->mark > 0xffffffff) {
		printk(KERN_WARNING "MARK: Only supports 32bit wide mark\n");
		return false;
	}
	return true;
}

static bool mark_tg_check_v1(const struct xt_tgchk_param *par)
{
	const struct xt_mark_target_info_v1 *markinfo = par->targinfo;

	if (markinfo->mode != XT_MARK_SET
	    && markinfo->mode != XT_MARK_AND
	    && markinfo->mode != XT_MARK_OR) {
		printk(KERN_WARNING "MARK: unknown mode %u\n",
		       markinfo->mode);
		return false;
	}
	if (markinfo->mark > 0xffffffff) {
		printk(KERN_WARNING "MARK: Only supports 32bit wide mark\n");
		return false;
	}
	return true;
}

#ifdef CONFIG_COMPAT
struct compat_xt_mark_target_info {
	compat_ulong_t	mark;
};

static void mark_tg_compat_from_user_v0(void *dst, void *src)
{
	const struct compat_xt_mark_target_info *cm = src;
	struct xt_mark_target_info m = {
		.mark	= cm->mark,
	};
	memcpy(dst, &m, sizeof(m));
}

static int mark_tg_compat_to_user_v0(void __user *dst, void *src)
{
	const struct xt_mark_target_info *m = src;
	struct compat_xt_mark_target_info cm = {
		.mark	= m->mark,
	};
	return copy_to_user(dst, &cm, sizeof(cm)) ? -EFAULT : 0;
}

struct compat_xt_mark_target_info_v1 {
	compat_ulong_t	mark;
	u_int8_t	mode;
	u_int8_t	__pad1;
	u_int16_t	__pad2;
};

static void mark_tg_compat_from_user_v1(void *dst, void *src)
{
	const struct compat_xt_mark_target_info_v1 *cm = src;
	struct xt_mark_target_info_v1 m = {
		.mark	= cm->mark,
		.mode	= cm->mode,
	};
	memcpy(dst, &m, sizeof(m));
}

static int mark_tg_compat_to_user_v1(void __user *dst, void *src)
{
	const struct xt_mark_target_info_v1 *m = src;
	struct compat_xt_mark_target_info_v1 cm = {
		.mark	= m->mark,
		.mode	= m->mode,
	};
	return copy_to_user(dst, &cm, sizeof(cm)) ? -EFAULT : 0;
}
#endif /* CONFIG_COMPAT */

#if	defined(CONFIG_RTL_HW_QOS_SUPPORT) && defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
int xt_target2acl_v0(const char *tablename,
			  const void *entry,
			  const struct xt_target *target,
			  void *targinfo,
			  rtl865x_AclRule_t *rule,
			  unsigned int hook_mask, 
			  void **data)
{
	struct net_device *dev, *matchDev;
	struct ipt_entry *e;
	const struct xt_mark_target_info *markinfo = targinfo;
	rtl865x_qos_rule_t qosRule;


	e = (struct ipt_entry *)entry;

	{
		dev = __dev_get_by_name(&init_net, e->ip.outiface);
		memcpy(qosRule.inIfname, e->ip.iniface, IFNAMSIZ);
		memcpy(qosRule.outIfname, e->ip.outiface, IFNAMSIZ);
		
		qosRule.mark = markinfo->mark;

		////////////////////////
		//Patch for hardware QoS
		//To seperate uplink and downlink according to mark from iptables rule
		if(qosRule.mark<53)
		{
			rule->upDown_=0;//Uplink
		}
		else
		{
			rule->upDown_=1;//Downlink
		}
		//////////////////////////
		
		qosRule.rule = rule;
		
		tc_getHandleByKey(markinfo->mark, &qosRule.handle, dev, &matchDev);
		rtl865x_qosAddMarkRule(&qosRule);
	}

	return RTL865X_SKIP_THIS_RULE;
}
int xt_target2acl_v1(const char *tablename,
			  const void *entry,
			  const struct xt_target *target,
			  void *targinfo,
			  rtl865x_AclRule_t *rule,
			  unsigned int hook_mask, 
			  void **data)
{
	struct net_device *dev, *matchDev;
	struct ipt_entry *e;
	const struct xt_mark_target_info_v1 *markinfo = targinfo;
	rtl865x_qos_rule_t qosRule;


	e = (struct ipt_entry *)entry;

	{
		dev = __dev_get_by_name(&init_net, e->ip.outiface);
		memcpy(qosRule.inIfname, e->ip.iniface, IFNAMSIZ);
		memcpy(qosRule.outIfname, e->ip.outiface, IFNAMSIZ);
		
		qosRule.mark = markinfo->mark;

		////////////////////////
		//Patch for hardware QoS
		//To seperate uplink and downlink according to mark from iptables rule
		if(qosRule.mark<53)
		{
			rule->upDown_=0;//Uplink
		}
		else
		{
			rule->upDown_=1;//Downlink
		}
		//////////////////////////
		
		qosRule.rule = rule;
		
		tc_getHandleByKey(markinfo->mark, &qosRule.handle, dev, &matchDev);
		rtl865x_qosAddMarkRule(&qosRule);
	}

	return RTL865X_SKIP_THIS_RULE;
}
int xt_target2acl_v2(const char *tablename,
			  const void *entry,
			  const struct xt_target *target,
			  void *targinfo,
			  rtl865x_AclRule_t *rule,
			  unsigned int hook_mask, 
			  void **data)
{
	struct net_device *dev, *matchDev;
	struct ipt_entry *e;
	const struct xt_mark_tginfo2 *markinfo = targinfo;
	rtl865x_qos_rule_t qosRule;


	e = (struct ipt_entry *)entry;

	{
		dev = __dev_get_by_name(&init_net, e->ip.outiface);
		memcpy(qosRule.inIfname, e->ip.iniface, IFNAMSIZ);
		memcpy(qosRule.outIfname, e->ip.outiface, IFNAMSIZ);
		
		qosRule.mark = markinfo->mark;

		////////////////////////
		//Patch for hardware QoS
		//To seperate uplink and downlink according to mark from iptables rule
		if(qosRule.mark<53)
		{
			rule->upDown_=0;//Uplink
		}
		else
		{
			rule->upDown_=1;//Downlink
		}
		//////////////////////////
		
		qosRule.rule = rule;
		
		tc_getHandleByKey(markinfo->mark, &qosRule.handle, dev, &matchDev);
		rtl865x_qosAddMarkRule(&qosRule);
	}

	return RTL865X_SKIP_THIS_RULE;
}
#endif

static struct xt_target mark_tg_reg[] __read_mostly = {
	{
		.name		= "MARK",
		.family		= NFPROTO_UNSPEC,
		.revision	= 0,
		.checkentry	= mark_tg_check_v0,
		.target		= mark_tg_v0,
		.targetsize	= sizeof(struct xt_mark_target_info),
#ifdef CONFIG_COMPAT
		.compatsize	= sizeof(struct compat_xt_mark_target_info),
		.compat_from_user = mark_tg_compat_from_user_v0,
		.compat_to_user	= mark_tg_compat_to_user_v0,
#endif
		.table		= "mangle",
#if	defined(CONFIG_RTL_HW_QOS_SUPPORT) && defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
		.target2acl	= xt_target2acl_v0,
#endif
		.me		= THIS_MODULE,
	},
	{
		.name		= "MARK",
		.family		= NFPROTO_UNSPEC,
		.revision	= 1,
		.checkentry	= mark_tg_check_v1,
		.target		= mark_tg_v1,
		.targetsize	= sizeof(struct xt_mark_target_info_v1),
#ifdef CONFIG_COMPAT
		.compatsize	= sizeof(struct compat_xt_mark_target_info_v1),
		.compat_from_user = mark_tg_compat_from_user_v1,
		.compat_to_user	= mark_tg_compat_to_user_v1,
#endif
		.table		= "mangle",
#if	defined(CONFIG_RTL_HW_QOS_SUPPORT) && defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
		.target2acl	= xt_target2acl_v1,
#endif
		.me		= THIS_MODULE,
	},
	{
		.name           = "MARK",
		.revision       = 2,
		.family         = NFPROTO_UNSPEC,
		.target         = mark_tg,
		.targetsize     = sizeof(struct xt_mark_tginfo2),
#if	defined(CONFIG_RTL_HW_QOS_SUPPORT) && defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
		.target2acl	= xt_target2acl_v2,
#endif
		.me             = THIS_MODULE,
	},
};

static int __init mark_tg_init(void)
{
	return xt_register_targets(mark_tg_reg, ARRAY_SIZE(mark_tg_reg));
}

static void __exit mark_tg_exit(void)
{
	xt_unregister_targets(mark_tg_reg, ARRAY_SIZE(mark_tg_reg));
}

module_init(mark_tg_init);
module_exit(mark_tg_exit);
