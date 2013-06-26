/* Kernel module to match MAC address parameters. */

/* (C) 1999-2001 Paul `Rusty' Russell
 * (C) 2002-2004 Netfilter Core Team <coreteam@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/if_ether.h>
#include <linux/etherdevice.h>

#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>
#include <linux/netfilter/xt_phyport.h>
#include <linux/netfilter/x_tables.h>
#include <net/dst.h>

#if 0	//defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl865x_netif.h>
#endif


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Netfilter Core Team <coreteam@netfilter.org>");
MODULE_DESCRIPTION("Xtables: MAC address match");
MODULE_ALIAS("ipt_phyport");
MODULE_ALIAS("ip6t_phyport");

static bool
checkentry(const struct xt_mtchk_param *par)
{
	const struct xt_phyport_info *phyportinfo = par->matchinfo;

	if(phyportinfo->flags & PORT_SRC){
		if (phyportinfo->srcport> 4) {
			printk(KERN_WARNING "phy port source port: only supports port number 0~4\n");
			return 0;
		}
	}
	else if(phyportinfo->flags & PORT_DST){
		if (phyportinfo->dstport> 4) {
			printk(KERN_WARNING "phy port dest port: only supports port number 0~4\n");
			return 0;
		}
	}
	else{
		printk(KERN_WARNING "wrong phy port flags 0x%x\n", phyportinfo->flags);
		return 0;
	}
	
	return 1;
}

static bool phyport_mt(const struct sk_buff *skb, const struct xt_match_param *par)
{
	const struct xt_phyport_info *info = par->matchinfo;   

	if (info->flags & PORT_SRC) {
		if((skb->srcPhyPort != info->srcport) ^ (!!(info->flags & PORT_SRC_INV))){
			return false;
		}
	}
	else if (info->flags & PORT_DST) {
		if((skb->dstPhyPort != info->dstport) ^ (!!(info->flags & PORT_DST_INV))){
			return false;
		}
	}
	else{
		printk(KERN_WARNING "wrong phy port flags 0x%x\n", info->flags);
		return false;
	}

	return true;
}

#if 0	//defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
static int phyport_match2acl(const char *tablename,
			  const void *ip,
			  const struct xt_match *match,
			  void *matchinfo,
			  void *acl_rule,
			  unsigned int *invflags)
{

	const struct xt_phyport_info *info = matchinfo;
	rtl865x_AclRule_t *rule = (rtl865x_AclRule_t *)acl_rule;
	if(matchinfo == NULL || rule == NULL)
		return 1;

	rule->ruleType_ = RTL865X_ACL_MAC;

	//To initial first
	memset(rule->srcMac_.octet, 0, ETH_ALEN);
	memset(rule->srcMacMask_.octet, 0, ETH_ALEN);
	memset(rule->dstMac_.octet, 0, ETH_ALEN);
	memset(rule->dstMacMask_.octet, 0, ETH_ALEN);
	
	if (info->flags & MAC_SRC) {
		memcpy(rule->srcMac_.octet, info->srcaddr.macaddr, ETH_ALEN);
		memset(rule->srcMacMask_.octet, 0xff, ETH_ALEN);
	}

	if (info->flags & MAC_DST) {
		memcpy(rule->dstMac_.octet, info->dstaddr.macaddr, ETH_ALEN);
		memset(rule->dstMacMask_.octet, 0xff, ETH_ALEN);
	}
	
	rule->typeLen_ = rule->typeLenMask_ = 0;
	
	return 0;
}
#endif

static struct xt_match phyport_mt_reg __read_mostly = {
	.name      = "phyport",
	.revision  = 0,
	.family    = NFPROTO_UNSPEC,
	.checkentry	= checkentry,
	.match     = phyport_mt,
	.matchsize = sizeof(struct xt_phyport_info),
/*	.hooks     = (1 << NF_INET_PRE_ROUTING) | (1 << NF_INET_LOCAL_IN) |
	             (1 << NF_INET_FORWARD),
*/
	.me        = THIS_MODULE,
#if 0	//defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
	.match2acl	= phyport_match2acl,
#endif

};

static int __init phyport_mt_init(void)
{
	return xt_register_match(&phyport_mt_reg);
}

static void __exit phyport_mt_exit(void)
{
	xt_unregister_match(&phyport_mt_reg);
}

module_init(phyport_mt_init);
module_exit(phyport_mt_exit);
