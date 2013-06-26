/* (C) 1999-2001 Paul `Rusty' Russell
 * (C) 2002-2006 Netfilter Core Team <coreteam@netfilter.org>
 * (C) 2008 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/types.h>
#include <linux/random.h>
#include <linux/ip.h>

#include <linux/netfilter.h>
#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_nat_core.h>
#include <net/netfilter/nf_nat_rule.h>
#include <net/netfilter/nf_nat_protocol.h>

#if defined (CONFIG_RTL_LAYERED_DRIVER) && defined (CONFIG_RTL_LAYERED_DRIVER_L4) && defined (CONFIG_RTL_HARDWARE_NAT)
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl865x_nat.h>
#endif
#if defined (CONFIG_RTL_HARDWARE_NAT) && defined (CONFIG_RTL_INBOUND_COLLISION_AVOIDANCE)
extern __DRAM_GEN int gHwNatEnabled;
#endif
bool nf_nat_proto_in_range(const struct nf_conntrack_tuple *tuple,
			   enum nf_nat_manip_type maniptype,
			   const union nf_conntrack_man_proto *min,
			   const union nf_conntrack_man_proto *max)
{
	__be16 port;

	if (maniptype == IP_NAT_MANIP_SRC)
		port = tuple->src.u.all;
	else
		port = tuple->dst.u.all;

	return ntohs(port) >= ntohs(min->all) &&
	       ntohs(port) <= ntohs(max->all);
}
EXPORT_SYMBOL_GPL(nf_nat_proto_in_range);

bool nf_nat_proto_unique_tuple(struct nf_conntrack_tuple *tuple,
			       const struct nf_nat_range *range,
			       enum nf_nat_manip_type maniptype,
			       const struct nf_conn *ct,
			       u_int16_t *rover)
{
	unsigned int range_size, min, i;
	__be16 *portptr;
	u_int16_t off;
#if defined (CONFIG_RTL_HARDWARE_NAT) && defined (CONFIG_RTL_INBOUND_COLLISION_AVOIDANCE)
	unsigned int tryCnt=0;
	rtl865x_napt_entry rtl865xNaptEntry;
	unsigned int asicNaptHashScore=0;
	unsigned int highestScore=0;
	unsigned int lowestScore=0;
	__be16 bestCandidatePort=0;
	__be16 newOff=0;
#endif

	if (maniptype == IP_NAT_MANIP_SRC)
		portptr = &tuple->src.u.all;
	else
		portptr = &tuple->dst.u.all;

	/* If no range specified... */
	if (!(range->flags & IP_NAT_RANGE_PROTO_SPECIFIED)) {
		/* If it's dst rewrite, can't change port */
		if (maniptype == IP_NAT_MANIP_DST)
			return false;

		if (ntohs(*portptr) < 1024) {
			/* Loose convention: >> 512 is credential passing */
			if (ntohs(*portptr) < 512) {
				min = 1;
				range_size = 511 - min + 1;
			} else {
				min = 600;
				range_size = 1023 - min + 1;
			}
		} else {
			min = 1024;
			range_size = 65535 - 1024 + 1;
		}
	} else {
		min = ntohs(range->min.all);
		range_size = ntohs(range->max.all) - min + 1;
	}

	if (range->flags & IP_NAT_RANGE_PROTO_RANDOM)
		off = secure_ipv4_port_ephemeral(tuple->src.u3.ip, tuple->dst.u3.ip,
						 maniptype == IP_NAT_MANIP_SRC
						 ? tuple->dst.u.all
						 : tuple->src.u.all);
	else
		off = *rover;

#if defined (CONFIG_RTL_HARDWARE_NAT) && defined (CONFIG_RTL_INBOUND_COLLISION_AVOIDANCE)
if(ct->master || (ct->status & IPS_EXPECTED)){
	for (i = 0; i < range_size; i++, off++) {
		*portptr = htons(min + off % range_size);
		if (nf_nat_used_tuple(tuple, ct))
			continue;
		if (!(range->flags & IP_NAT_RANGE_PROTO_RANDOM))
			*rover = off;
		return true;
	}
}else
{
	for (i = 0; i < range_size; i++) {
		if((gHwNatEnabled) && (maniptype==IP_NAT_MANIP_SRC))
		{
			rtl865x_optimizeExtPort(off++, range_size, &newOff);
			*portptr = htons(min + newOff % range_size);
		}
		else
		{
			*portptr = htons(min + off % range_size);
			off++;
		}

		if (nf_nat_used_tuple(tuple, ct))
			continue;

		if((gHwNatEnabled) && (maniptype==IP_NAT_MANIP_SRC))
		{
			/* outbound flow */
			rtl865xNaptEntry.protocol = (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum==IPPROTO_TCP)? 1: 0;
			rtl865xNaptEntry.intIp = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip;
			rtl865xNaptEntry.remIp = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip;
			rtl865xNaptEntry.extIp = tuple->src.u3.ip;
			rtl865xNaptEntry.intPort = rtl865xNaptEntry.protocol?ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.tcp.port:ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.udp.port;
			rtl865xNaptEntry.remPort = rtl865xNaptEntry.protocol?ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.tcp.port:ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.udp.port;
			rtl865xNaptEntry.extPort= rtl865xNaptEntry.protocol?tuple->src.u.tcp.port:tuple->src.u.udp.port;

			rtl865x_getAsicNaptHashScore(&rtl865xNaptEntry,&asicNaptHashScore);

			if(asicNaptHashScore==100)
			{
				highestScore=asicNaptHashScore;
				lowestScore=asicNaptHashScore;
				bestCandidatePort=*portptr;
			}
			else
			{
				/*otherwise use the best cadidate port*/
				if(bestCandidatePort==0)
				{
					bestCandidatePort=*portptr;
					highestScore=asicNaptHashScore;
					lowestScore=asicNaptHashScore;
				}

				if(asicNaptHashScore>highestScore)
				{
					highestScore=asicNaptHashScore;
					bestCandidatePort=*portptr;
				}

				if(asicNaptHashScore<lowestScore)
				{
					lowestScore=asicNaptHashScore;
				}

				if(tryCnt++<=MAX_EXTPORT_TRY_CNT)
				{
					continue;
				}
				else
				{
					*portptr=bestCandidatePort;
				}
			}

			rtl865xNaptEntry.extPort=*portptr;
			rtl865x_preReserveConn(&rtl865xNaptEntry);
		}
/*
			printk("%s:%d:maniptype is %d, %s (%u.%u.%u.%u:%u -> %u.%u.%u.%u:%u -> %u.%u.%u.%u:%u) ,lowestScore is %d,highestScore is %d\n\n\n",
			__FUNCTION__,__LINE__,maniptype, rtl865xNaptEntry.protocol?"tcp":"udp",
			NIPQUAD(rtl865xNaptEntry.intIp), rtl865xNaptEntry.intPort, NIPQUAD(rtl865xNaptEntry.extIp), rtl865xNaptEntry.extPort, NIPQUAD(rtl865xNaptEntry.remIp), rtl865xNaptEntry.remPort,lowestScore, highestScore);
*/

		if (!(range->flags & IP_NAT_RANGE_PROTO_RANDOM))
			*rover = off;
		return true;
	}
}
#else
	for (i = 0; i < range_size; i++, off++) {
		*portptr = htons(min + off % range_size);
		if (nf_nat_used_tuple(tuple, ct))
			continue;
		if (!(range->flags & IP_NAT_RANGE_PROTO_RANDOM))
			*rover = off;
		return true;
	}
#endif
	return false;
}
EXPORT_SYMBOL_GPL(nf_nat_proto_unique_tuple);

#if defined(CONFIG_NF_CT_NETLINK) || defined(CONFIG_NF_CT_NETLINK_MODULE)
int nf_nat_proto_range_to_nlattr(struct sk_buff *skb,
				 const struct nf_nat_range *range)
{
	NLA_PUT_BE16(skb, CTA_PROTONAT_PORT_MIN, range->min.all);
	NLA_PUT_BE16(skb, CTA_PROTONAT_PORT_MAX, range->max.all);
	return 0;

nla_put_failure:
	return -1;
}
EXPORT_SYMBOL_GPL(nf_nat_proto_nlattr_to_range);

int nf_nat_proto_nlattr_to_range(struct nlattr *tb[],
				 struct nf_nat_range *range)
{
	if (tb[CTA_PROTONAT_PORT_MIN]) {
		range->min.all = nla_get_be16(tb[CTA_PROTONAT_PORT_MIN]);
		range->max.all = range->min.tcp.port;
		range->flags |= IP_NAT_RANGE_PROTO_SPECIFIED;
	}
	if (tb[CTA_PROTONAT_PORT_MAX]) {
		range->max.all = nla_get_be16(tb[CTA_PROTONAT_PORT_MAX]);
		range->flags |= IP_NAT_RANGE_PROTO_SPECIFIED;
	}
	return 0;
}
EXPORT_SYMBOL_GPL(nf_nat_proto_range_to_nlattr);
#endif
