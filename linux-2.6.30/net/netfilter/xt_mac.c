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
#include <linux/netfilter/xt_mac.h>
#include <linux/netfilter/x_tables.h>
#include <net/dst.h>

#if defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl865x_netif.h>
#endif


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Netfilter Core Team <coreteam@netfilter.org>");
MODULE_DESCRIPTION("Xtables: MAC address match");
MODULE_ALIAS("ipt_mac");
MODULE_ALIAS("ip6t_mac");

static int compare_with_header_cache_dest_mac(const struct sk_buff *skb, char *macaddr)
{
	struct dst_entry *dst = skb->dst;
	struct hh_cache *hh;
	int ret = 0;
	
	if ((dst)&&(dst->hh)&&(dst->hh->hh_data))
	{
	    	hh = dst->hh;
	    	if (hh && (hh->hh_type==ETH_P_IP || hh->hh_type==ETH_P_IPV6))
		{
	    		read_lock_bh(&hh->hh_lock);
	      		memcpy(skb->data - 16, hh->hh_data, 16);
	      		if (memcmp((((u8*)hh->hh_data) + 2), macaddr, ETH_ALEN) == 0)
	      		    ret = 1;
	    		read_unlock_bh(&hh->hh_lock);
	    	}
	}
	else
	{
		if(!compare_ether_addr(eth_hdr(skb)->h_dest, macaddr))
		{
			ret=1;
		}
	}
	return ret;
}

static bool mac_mt(const struct sk_buff *skb, const struct xt_match_param *par)
{
	const struct xt_mac_info *info = par->matchinfo;   

	#if defined(CONFIG_RTL_MAC_FILTER_CARE_INPORT)
	if((info->flags&INPORT_FLAG) && !((1<<(skb->srcPhyPort)) & info->inPortMask)){
		return 0;
	}
	#endif
	
	if (info->flags & MAC_SRC) {
	     /* Is mac pointer valid? */
	    if ((skb_mac_header(skb) >= skb->head
		    && (skb_mac_header(skb) + ETH_HLEN) <= skb->data
		    /* If so, compare... */
		    && ((!compare_ether_addr(eth_hdr(skb)->h_source, info->srcaddr.macaddr))
			^ !!(info->flags & MAC_SRC_INV)))==0)
	    	{
			return 0;
	    	}
	}

	if (info->flags & MAC_DST) {
	     /* Is mac pointer valid? */
	    if( (skb_mac_header(skb) >= skb->head
		    && (skb_mac_header(skb) + ETH_HLEN) <= skb->data
		    /* If so, compare... */
		    && (compare_with_header_cache_dest_mac(skb, (char*)(info->dstaddr.macaddr)) ^ !!(info->flags & MAC_DST_INV)))==0)
	    	{
			return 0;
	    	}
	}

	return 1;

}

#if defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
static int mac_match2acl(const char *tablename,
			  const void *ip,
			  const struct xt_match *match,
			  void *matchinfo,
			  void *acl_rule,
			  unsigned int *invflags)
{

	const struct xt_mac_info *info = matchinfo;
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

static struct xt_match mac_mt_reg[] __read_mostly = {
	{
	.name      = "mac",
	.revision  = 0,
	.family    = NFPROTO_IPV4,
	.match     = mac_mt,
	.matchsize = sizeof(struct xt_mac_info),
/*	.hooks     = (1 << NF_INET_PRE_ROUTING) | (1 << NF_INET_LOCAL_IN) |
	             (1 << NF_INET_FORWARD),
*/
	.me        = THIS_MODULE,
#if defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
	.match2acl	= mac_match2acl,
#endif
	},
	{
	.name      = "mac",
	.revision  = 0,
	.family    = NFPROTO_IPV6,
	.match     = mac_mt,
	.matchsize = sizeof(struct xt_mac_info),
/*	.hooks     = (1 << NF_INET_PRE_ROUTING) | (1 << NF_INET_LOCAL_IN) |
	             (1 << NF_INET_FORWARD),
*/
	.me        = THIS_MODULE,
#if defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
	.match2acl	= mac_match2acl,
#endif
	},

};

static int __init mac_mt_init(void)
{
	return xt_register_matches(mac_mt_reg, ARRAY_SIZE(mac_mt_reg));	
}

static void __exit mac_mt_exit(void)
{
	xt_unregister_matches(mac_mt_reg, ARRAY_SIZE(mac_mt_reg));
}

module_init(mac_mt_init);
module_exit(mac_mt_exit);
