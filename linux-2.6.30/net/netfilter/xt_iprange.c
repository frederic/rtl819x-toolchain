/*
 *	xt_iprange - Netfilter module to match IP address ranges
 *
 *	(C) 2003 Jozsef Kadlecsik <kadlec@blackhole.kfki.hu>
 *	(C) CC Computer Consultants GmbH, 2008
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/xt_iprange.h>
#include <linux/netfilter_ipv4/ipt_iprange.h>

#if defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl865x_netif.h>
#endif


static bool
iprange_mt_v0(const struct sk_buff *skb, const struct xt_match_param *par)
{
	const struct ipt_iprange_info *info = par->matchinfo;
	const struct iphdr *iph = ip_hdr(skb);

	if (info->flags & IPRANGE_SRC) {
		if ((ntohl(iph->saddr) < ntohl(info->src.min_ip)
			  || ntohl(iph->saddr) > ntohl(info->src.max_ip))
			 ^ !!(info->flags & IPRANGE_SRC_INV)) {
			pr_debug("src IP %pI4 NOT in range %s%pI4-%pI4\n",
				 &iph->saddr,
				 info->flags & IPRANGE_SRC_INV ? "(INV) " : "",
				 &info->src.min_ip,
				 &info->src.max_ip);
			return false;
		}
	}
	if (info->flags & IPRANGE_DST) {
		if ((ntohl(iph->daddr) < ntohl(info->dst.min_ip)
			  || ntohl(iph->daddr) > ntohl(info->dst.max_ip))
			 ^ !!(info->flags & IPRANGE_DST_INV)) {
			pr_debug("dst IP %pI4 NOT in range %s%pI4-%pI4\n",
				 &iph->daddr,
				 info->flags & IPRANGE_DST_INV ? "(INV) " : "",
				 &info->dst.min_ip,
				 &info->dst.max_ip);
			return false;
		}
	}
	return true;
}

static bool
iprange_mt4(const struct sk_buff *skb, const struct xt_match_param *par)
{
	const struct xt_iprange_mtinfo *info = par->matchinfo;
	const struct iphdr *iph = ip_hdr(skb);
	bool m;

	if (info->flags & IPRANGE_SRC) {
		m  = ntohl(iph->saddr) < ntohl(info->src_min.ip);
		m |= ntohl(iph->saddr) > ntohl(info->src_max.ip);
		m ^= !!(info->flags & IPRANGE_SRC_INV);
		if (m) {
			pr_debug("src IP %pI4 NOT in range %s%pI4-%pI4\n",
			         &iph->saddr,
			         (info->flags & IPRANGE_SRC_INV) ? "(INV) " : "",
			         &info->src_max.ip,
			         &info->src_max.ip);
			return false;
		}
	}
	if (info->flags & IPRANGE_DST) {
		m  = ntohl(iph->daddr) < ntohl(info->dst_min.ip);
		m |= ntohl(iph->daddr) > ntohl(info->dst_max.ip);
		m ^= !!(info->flags & IPRANGE_DST_INV);
		if (m) {
			pr_debug("dst IP %pI4 NOT in range %s%pI4-%pI4\n",
			         &iph->daddr,
			         (info->flags & IPRANGE_DST_INV) ? "(INV) " : "",
			         &info->dst_min.ip,
			         &info->dst_max.ip);
			return false;
		}
	}
	return true;
}

#if defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
static int iprange_match2acl(const char *tablename,
                          const void *ip,
                          const struct xt_match *match,
                          void *matchinfo,
                          void *acl_rule,
                          unsigned int *invflags)
{
 
        const struct ipt_iprange_info *info = matchinfo;
	rtl865x_AclRule_t *rule = (rtl865x_AclRule_t *)acl_rule;
 
        if(ip == NULL || matchinfo == NULL || rule == NULL)
                return 1;
 
        switch(rule->ruleType_)
        {
                case            RTL865X_ACL_TCP:
                        rule->ruleType_ = RTL865X_ACL_TCP_IPRANGE;
                        break;
                case            RTL865X_ACL_UDP:
                        rule->ruleType_ = RTL865X_ACL_UDP_IPRANGE;
                        break;
                case            RTL865X_ACL_IGMP:
                        rule->ruleType_ = RTL865X_ACL_ICMP_IPRANGE;
                        break;
                case            RTL865X_ACL_ICMP:
                        rule->ruleType_ = RTL865X_ACL_IGMP_IPRANGE;
                        break;
                case            RTL865X_ACL_SRCFILTER:
                        rule->ruleType_ = RTL865X_ACL_SRCFILTER_IPRANGE;
                        break;
                case            RTL865X_ACL_MAC:
                case            RTL865X_ACL_IP:
                        rule->ruleType_ = RTL865X_ACL_IP_RANGE;
                        break;
                default:
                        return 1;
        }

        rule->srcIpAddrLB_ = rule->dstIpAddrLB_ = 0;
        rule->srcIpAddrUB_ = rule->dstIpAddrUB_ = 0xffffffff;
        if (info->flags & IPRANGE_SRC) {
                rule->srcIpAddrLB_ = ntohl(info->src.min_ip);
                rule->srcIpAddrUB_ = ntohl(info->src.max_ip);
        }
        if (info->flags & IPRANGE_DST) {
                rule->dstIpAddrLB_ = ntohl(info->dst.min_ip);
                rule->dstIpAddrUB_ = ntohl(info->dst.max_ip);
        }
 
        return 0;
}
static int iprange_match2acl_mt4(const char *tablename,
                          const void *ip,
                          const struct xt_match *match,
                          void *matchinfo,
                          void *acl_rule,
                          unsigned int *invflags)
{
 
        const struct xt_iprange_mtinfo *info = matchinfo;
	rtl865x_AclRule_t *rule = (rtl865x_AclRule_t *)acl_rule;
 
        if(ip == NULL || matchinfo == NULL || rule == NULL)
                return 1;
 
        switch(rule->ruleType_)
        {
                case            RTL865X_ACL_TCP:
                        rule->ruleType_ = RTL865X_ACL_TCP_IPRANGE;
                        break;
                case            RTL865X_ACL_UDP:
                        rule->ruleType_ = RTL865X_ACL_UDP_IPRANGE;
                        break;
                case            RTL865X_ACL_IGMP:
                        rule->ruleType_ = RTL865X_ACL_ICMP_IPRANGE;
                        break;
                case            RTL865X_ACL_ICMP:
                        rule->ruleType_ = RTL865X_ACL_IGMP_IPRANGE;
                        break;
                case            RTL865X_ACL_SRCFILTER:
                        rule->ruleType_ = RTL865X_ACL_SRCFILTER_IPRANGE;
                        break;
                case            RTL865X_ACL_MAC:
                case            RTL865X_ACL_IP:
                        rule->ruleType_ = RTL865X_ACL_IP_RANGE;
                        break;
                default:
                        return 1;
        }

        rule->srcIpAddrLB_ = rule->dstIpAddrLB_ = 0;
        rule->srcIpAddrUB_ = rule->dstIpAddrUB_ = 0xffffffff;
        if (info->flags & IPRANGE_SRC) {
                rule->srcIpAddrLB_ = info->src_min.in.s_addr;
                rule->srcIpAddrUB_ = info->src_max.in.s_addr;
        }
        if (info->flags & IPRANGE_DST) {
                rule->dstIpAddrLB_ = info->dst_min.in.s_addr;
                rule->dstIpAddrUB_ = info->dst_max.in.s_addr;
        }
 
        return 0;
}
#endif

static inline int
iprange_ipv6_sub(const struct in6_addr *a, const struct in6_addr *b)
{
	unsigned int i;
	int r;

	for (i = 0; i < 4; ++i) {
		r = ntohl(a->s6_addr32[i]) - ntohl(b->s6_addr32[i]);
		if (r != 0)
			return r;
	}

	return 0;
}

static bool
iprange_mt6(const struct sk_buff *skb, const struct xt_match_param *par)
{
	const struct xt_iprange_mtinfo *info = par->matchinfo;
	const struct ipv6hdr *iph = ipv6_hdr(skb);
	bool m;

	if (info->flags & IPRANGE_SRC) {
		m  = iprange_ipv6_sub(&iph->saddr, &info->src_min.in6) < 0;
		m |= iprange_ipv6_sub(&iph->saddr, &info->src_max.in6) > 0;
		m ^= !!(info->flags & IPRANGE_SRC_INV);
		if (m)
			return false;
	}
	if (info->flags & IPRANGE_DST) {
		m  = iprange_ipv6_sub(&iph->daddr, &info->dst_min.in6) < 0;
		m |= iprange_ipv6_sub(&iph->daddr, &info->dst_max.in6) > 0;
		m ^= !!(info->flags & IPRANGE_DST_INV);
		if (m)
			return false;
	}
	return true;
}

static struct xt_match iprange_mt_reg[] __read_mostly = {
	{
		.name      = "iprange",
		.revision  = 0,
		.family    = NFPROTO_IPV4,
		.match     = iprange_mt_v0,
		.matchsize = sizeof(struct ipt_iprange_info),
		.me        = THIS_MODULE,
#if defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
        	.match2acl      = iprange_match2acl,
#endif

	},
	{
		.name      = "iprange",
		.revision  = 1,
		.family    = NFPROTO_IPV4,
		.match     = iprange_mt4,
		.matchsize = sizeof(struct xt_iprange_mtinfo),
		.me        = THIS_MODULE,
#if defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
        	.match2acl      = iprange_match2acl_mt4,
#endif
	},
	{
		.name      = "iprange",
		.revision  = 1,
		.family    = NFPROTO_IPV6,
		.match     = iprange_mt6,
		.matchsize = sizeof(struct xt_iprange_mtinfo),
		.me        = THIS_MODULE,
	},
};

static int __init iprange_mt_init(void)
{
	return xt_register_matches(iprange_mt_reg, ARRAY_SIZE(iprange_mt_reg));
}

static void __exit iprange_mt_exit(void)
{
	xt_unregister_matches(iprange_mt_reg, ARRAY_SIZE(iprange_mt_reg));
}

module_init(iprange_mt_init);
module_exit(iprange_mt_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jozsef Kadlecsik <kadlec@blackhole.kfki.hu>, Jan Engelhardt <jengelh@computergmbh.de>");
MODULE_DESCRIPTION("Xtables: arbitrary IPv4 range matching");
MODULE_ALIAS("ipt_iprange");
MODULE_ALIAS("ip6t_iprange");
