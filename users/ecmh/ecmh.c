/**************************************
 ecmh - Easy Cast du Multi Hub
 by Jeroen Massar <jeroen@unfix.org>
***************************************
 $Author: fuzzel $
 $Id: ecmh.c,v 1.1.1.1 2004/01/10 23:59:32 fuzzel Exp $
 $Date: 2004/01/10 23:59:32 $
**************************************/
//
// Docs to check: netdevice(7), packet(7)
//

#include "ecmh.h"
#define	ECMH_VERSION	"20040110"
// Configuration Variables
struct conf	*g_conf;

/**************************************
  Functions
**************************************/

// Actual code for doing the trick(tm)

// Send a packet
void sendpacket6(struct intnode *intn, const struct ip6_hdr *iph, const uint16_t len)
{
	struct sockaddr_ll	sa;
	int			sent,i;

	memset(&sa, 0, sizeof(sa));

	sa.sll_family	= AF_PACKET;
	sa.sll_protocol	= htons(ETH_P_IPV6);
	sa.sll_ifindex	= intn->ifindex;
	sa.sll_hatype	= intn->hwaddr.sa_family;
	sa.sll_pkttype	= 0;
	sa.sll_halen	= 6;

	// Construct a Ethernet MAC address from the IPv6 destination multicast address.
	// Per RFC2464
	sa.sll_addr[0] = 0x33;
	sa.sll_addr[1] = 0x33;
	sa.sll_addr[2] = iph->ip6_dst.s6_addr[12];
	sa.sll_addr[3] = iph->ip6_dst.s6_addr[13];
	sa.sll_addr[4] = iph->ip6_dst.s6_addr[14];
	sa.sll_addr[5] = iph->ip6_dst.s6_addr[15];

	// Resend the packet
	errno = 0;
	sent = sendto(g_conf->rawsocket_out, iph, len, 0, (struct sockaddr *)&sa, sizeof(sa));
	if (sent < 0)
	{
		// Remove the device if it doesn't exist anymore, can happen with dynamic tunnels etc
		if (errno == ENXIO) intn->removeme = true;
		else dolog(LOG_WARNING, "[%-5s] sending %d bytes failed, mtu = %d, %d: %s\n", intn->name, len, intn->mtu, errno, strerror(errno));
		return;
	}

	// Update the counters	
	g_conf->stat_packets_sent++;
	g_conf->stat_bytes_sent+=len;
	return;
}

uint16_t inchksum(const void *data, uint32_t length)
{
	register long		sum = 0;
	register const uint16_t *wrd = (const uint16_t *)data;
	register long		slen = (long)length;
	
	while (slen > 1)
	{
		sum += *wrd++;
		slen-=2;
	}
	
	if (slen > 0) sum+=*(const uint8_t *)wrd;

	while (sum >> 16) sum = (sum & 0xffff) + (sum >> 16);

	return (uint16_t)sum;
}

uint16_t ipv6_checksum(const struct ip6_hdr *ip6, uint8_t protocol, const void *data, const uint16_t length)
{
	struct
	{
		uint16_t	length;
		uint16_t	zero1;
		uint8_t		zero2;
		uint8_t		next;
	} pseudo;
	register uint32_t	chksum = 0;
	
	pseudo.length	= htons(length);
	pseudo.zero1	= 0;
	pseudo.zero2	= 0;
	pseudo.next	= protocol;

	// IPv6 Source + Dest
	chksum  = inchksum(&ip6->ip6_src, sizeof(ip6->ip6_src) + sizeof(ip6->ip6_dst));
	chksum += inchksum(&pseudo, sizeof(pseudo));
	chksum += inchksum(data, length);

	// Wrap in the carries to reduce chksum to 16 bits.
	chksum  = (chksum >> 16) + (chksum & 0xffff);
	chksum += (chksum >> 16);

	// Take ones-complement and replace 0 with 0xFFFF.
	chksum = (uint16_t) ~chksum;
	if (chksum == 0UL) chksum = 0xffffUL;
	return (uint16_t)chksum;
}

void mld_send_query(struct intnode *intn)
{
	struct mld_query_packet
	{
		struct ip6_hdr		ip6;
		struct ip6_hbh		hbh;
		struct
		{
			uint8_t		type;
			uint8_t		length;
			uint16_t	value;
			uint8_t		optpad[2];
		}			routeralert;
		
		struct mld2_query	mldq;
		
	} packet;
	
	memset(&packet, 0, sizeof(packet));

	// Create the IPv6 packet
	packet.ip6.ip6_vfc		= 0x60;
	packet.ip6.ip6_plen		= ntohs(sizeof(packet) - sizeof(packet.ip6));
	packet.ip6.ip6_nxt		= IPPROTO_HOPOPTS;
	packet.ip6.ip6_hlim		= 1;
	
	// The source address must be the link-local address
	// of the interface we are sending on
	memcpy(&packet.ip6.ip6_src, &intn->linklocal, sizeof(packet.ip6.ip6_src));

	// Generaly Query -> link-scope all-nodes (ff02::1)
	packet.ip6.ip6_dst.s6_addr[0]	= 0xff;
	packet.ip6.ip6_dst.s6_addr[1]	= 0x02;
	packet.ip6.ip6_dst.s6_addr[15]	= 0x01;

	// HopByHop Header Extension
	packet.hbh.ip6h_nxt		= IPPROTO_ICMPV6;
	packet.hbh.ip6h_len		= 0;
	
	// Router Alert Option
	packet.routeralert.type		= 5;
	packet.routeralert.length	= sizeof(packet.routeralert.value);
	packet.routeralert.value	= 0;			// MLD ;)

	// Option Padding
	packet.routeralert.optpad[0]	= IP6OPT_PADN;
	packet.routeralert.optpad[1]	= 0;

	// ICMPv6 MLD Query
	packet.mldq.type		= ICMP6_MEMBERSHIP_QUERY;
	packet.mldq.mrc			= htons(2000);
	packet.mldq.nsrcs		= 0;

	// Calculate and fill in the checksum
	packet.mldq.csum		= ipv6_checksum(&packet.ip6, IPPROTO_ICMPV6, (uint8_t *)&packet.mldq, sizeof(packet.mldq));

	sendpacket6(intn, (const struct ip6_hdr *)&packet, sizeof(packet));
}

void mld_send_report(struct intnode *intn, const struct in6_addr *mca)
{
	struct mld_query_packet
	{
		struct ip6_hdr		ip6;
		struct ip6_hbh		hbh;
		struct
		{
			uint8_t		type;
			uint8_t		length;
			uint16_t	value;
			uint8_t		optpad[2];
		}			routeralert;
		
		struct mld1		mld1;
		
	} packet;
	
	memset(&packet, 0, sizeof(packet));

	// Create the IPv6 packet
	packet.ip6.ip6_vfc		= 0x60;
	packet.ip6.ip6_plen		= ntohs(sizeof(packet) - sizeof(packet.ip6));
	packet.ip6.ip6_nxt		= IPPROTO_HOPOPTS;
	packet.ip6.ip6_hlim		= 1;
	
	// The source address must be the link-local address
	// of the interface we are sending on
	memcpy(&packet.ip6.ip6_src, &intn->linklocal, sizeof(packet.ip6.ip6_src));

	// Report -> Multicast address
	memcpy(&packet.ip6.ip6_dst, mca, sizeof(*mca));

	// HopByHop Header Extension
	packet.hbh.ip6h_nxt		= IPPROTO_ICMPV6;
	packet.hbh.ip6h_len		= 0;
	
	// Router Alert Option
	packet.routeralert.type		= 5;
	packet.routeralert.length	= sizeof(packet.routeralert.value);
	packet.routeralert.value	= 0;			// MLD ;)

	// Option Padding
	packet.routeralert.optpad[0]	= IP6OPT_PADN;
	packet.routeralert.optpad[1]	= 0;

	// ICMPv6 MLD Report
	packet.mld1.type		= ICMP6_MEMBERSHIP_REPORT;
	packet.mld1.mrc			= 0;
	memcpy(&packet.mld1.mca, mca, sizeof(*mca));

	// Calculate and fill in the checksum
	packet.mld1.csum		= ipv6_checksum(&packet.ip6, IPPROTO_ICMPV6, (uint8_t *)&packet.mld1, sizeof(packet.mld1));

	sendpacket6(intn, (const struct ip6_hdr *)&packet, sizeof(packet));
}

int
open_ecmh_icmpv6_socket(void)
{
	int sock;
	struct icmp6_filter filter;
	int err, val;

    	sock = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
	if (sock < 0)
	{
		dolog(LOG_ERR, "can't create socket(AF_INET6)\n");
		return (-1);
	}

	return sock;
}

/*send pkt too big icmp error*/
void icmpv6_pkt_send(const struct ip6_hdr *iph, const uint32_t mtu)
{
		uint8_t all_hosts_addr[] = {0xff,0x02,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
		struct sockaddr_in6 addr;
		int sockopt;
		struct icmp6_hdr	icmpv6;	
		unsigned char buff[2048];
		int len = 0;
		int err;
		struct in6_addr *dest=&iph->ip6_src;
		
	
		if (dest == NULL)
		{
			dest = (struct in6_addr *)all_hosts_addr;
		}
		/*create sockaddr_in6 */
		memset((void *)&addr, 0, sizeof(addr));
		addr.sin6_family = AF_INET6;
		memcpy(&addr.sin6_addr, dest, sizeof(struct in6_addr));
		
		/*create icmpv6 pkt too big error messag*/
		memset((void *)&icmpv6,0,sizeof(icmpv6));
		icmpv6.icmp6_type = ICMP6_PACKET_TOO_BIG;
		icmpv6.icmp6_mtu = mtu;
		memcpy(buff,&icmpv6,sizeof(icmpv6));
		
		/*copy data as much as possible*/
		len = mtu-sizeof(icmpv6)-sizeof(struct ip6_hdr);
		memcpy(buff+sizeof(icmpv6),(void*)((void*)iph+(mtu-len)),len);
		
		sockopt = 2;
		setsockopt(g_conf->sock_icmp, SOL_RAW, IPV6_CHECKSUM, &sockopt, sizeof(sockopt));
		err = sendto(g_conf->sock_icmp, buff, len+sizeof(icmpv6), 0,(struct sockaddr *) &addr, sizeof(addr));
		if (err < 0) {
			dolog(LOG_ERR, "Couldn't sendmsg\n");
			perror("Error");
		}
}



#ifdef ECMH_SUPPORT_IPV4

// IPv4 ICMP
void l4_ipv4_icmp(struct intnode *intn, struct ip *iph, const uint8_t *packet, const uint16_t len)
{
	D(dolog(LOG_DEBUG, "%5s L4:IPv4 ICMP\n", intn->name);)
	return;
}

// Protocol 41 - IPv6 in IPv4 (RFC3065)
void l4_ipv4_proto41(struct intnode *intn, struct ip *iph, const uint8_t *packet, const uint16_t len)
{
	D(dolog(LOG_DEBUG, "%5s L4:IPv4 Proto 41\n", intn->name);)
	return;
}

// IPv4
void l3_ipv4(struct intnode *intn, const uint8_t *packet, const uint16_t len)
{
	struct ip *iph;
	char src[16], dst[16];

	// Ignore IPv4 ;)
	return;

	iph = (struct ip *)packet;

	if (iph->ip_v != 4)
	{
		D(dolog(LOG_DEBUG, "%5s L3:IPv4: IP version %d not supported\n", intn->name, iph->ip_v);)
		return;
	}

	if (iph->ip_hl < 5)
	{
		D(dolog(LOG_DEBUG, "%5s L3IPv4: IP hlen < 5 bytes (%d)\n", intn->name, iph->ip_hl);)
		return;
	}
	if (ntohs (iph->ip_len) != len)
	{
		// This happens mostly with unknown ARPHRD_* types
		D(dolog(LOG_DEBUG, "%5s L3:IPv4: *** L3 length != L2 length (%d != %d)\n", intn->name, ntohs(iph->ip_len), len);)
//		return;
	}
	inet_ntop(AF_INET, &iph->ip_src, src, sizeof(src));
	inet_ntop(AF_INET, &iph->ip_dst, dst, sizeof(dst));

	D(dolog(LOG_DEBUG, "%5s L3:IPv4: IPv%01u %-16s %-16s %4u\n", intn->name, iph->ip_v, src, dst, ntohs(iph->ip_len));)

	// Go to Layer 4
	if (iph->ip_p == 1) l4_ipv4_icmp(intn, iph, packet+4*iph->ip_hl, len-4*iph->ip_hl);
	// IGMP ??? if (iph->ip_p ==...)
	else if (iph->ip_p == 41) l4_ipv4_proto41(intn, iph, packet+4*iph->ip_hl, len-4*iph->ip_hl);
}

#endif // ECMH_SUPPORT_IPV4

void mld_warning(char *fmt, struct in6_addr *i_mca, const struct intnode *intn)
{
	char mca[60];
	inet_ntop(AF_INET6, i_mca, mca, sizeof(*mca));
	dolog(LOG_WARNING, fmt, mca, intn->name);
}

void l4_ipv6_icmpv6_mld1_report(struct intnode *intn, const struct ip6_hdr *iph, const uint16_t len, struct mld1 *mld1, const uint16_t plen)
{
	
	struct grpintnode	*grpintn;
	struct in6_addr		any;

	// Ignore node and link local multicast addresses
	if (	IN6_IS_ADDR_MC_NODELOCAL(&mld1->mca) ||
		IN6_IS_ADDR_MC_LINKLOCAL(&mld1->mca)) return;

	// Find the grpintnode or create it
	grpintn = groupint_get(&mld1->mca, intn);
	if (!grpintn)
	{
		mld_warning("Couldn't find or create new group %s for %s\n", &mld1->mca, intn);
		return;
	}

	// No source address, so use any
	memset(&any,0,sizeof(any));
	
	if (!grpint_refresh(grpintn, &any))
	{
		mld_warning("Couldn't create subscription to %s for %s\n", &mld1->mca, intn);
		return;
	}

	return;
}

void l4_ipv6_icmpv6_mld1_reduction(struct intnode *intn, const struct ip6_hdr *iph, const uint16_t len, struct mld1 *mld1, const uint16_t plen)
{
	struct groupnode	*groupn;
	struct grpintnode	*grpintn;
	struct subscrnode	*subscrn;
	struct in6_addr		any;

	// Ignore node and link local multicast addresses
	if (	IN6_IS_ADDR_MC_NODELOCAL(&mld1->mca) ||
		IN6_IS_ADDR_MC_LINKLOCAL(&mld1->mca)) return;

	// Find the groupnode
	groupn = group_find(&mld1->mca);
	if (!groupn)
	{
		mld_warning("Couldn't find group %s for reduction of %s\n", &mld1->mca, intn);
		return;
	}

	// Find the grpintnode
	grpintn = grpint_find(groupn->interfaces, intn);
	if (!grpintn)
	{
		mld_warning("Couldn't find the grpint %s for reduction of %s\n", &mld1->mca, intn);
		return;
	}

	// No source address, so use any
	memset(&any,0,sizeof(any));

	if (!subscr_unsub(grpintn->subscriptions, &any))
	{
		mld_warning("Couldn't unsubscribe from %s interface %s\n", &mld1->mca, intn);
		return;
	}

	return;
}

#ifdef ECMH_SUPPORT_MLD2
void l4_ipv6_icmpv6_mld2_report(struct intnode *intn, const struct ip6_hdr *iph, const uint16_t len, struct mld2_report *mld2r, const uint16_t plen)
{
	char			mca[60], srct[60];
	struct grpintnode	*grpintn = NULL;
	struct in6_addr		*src;
	struct mld2_grec	*grec = ((void *)mld2r) + sizeof(*mld2r);
	int ngrec		= ntohs(mld2r->ngrec);
	int nsrcs		= 0;

	// Zero out just in case
	mca[0] = srct[0] = 0;

	while (ngrec > 0)
	{
		if (grec->grec_auxwords != 0)
		{
			dolog(LOG_WARNING, "%5s L4:IPv6:ICMPv6:MLD2_Report Auxwords was %d instead of required 0\n", intn->name, grec->grec_auxwords);
			return;
		}
		
	//	D(dolog(LOG_DEBUG, "MLDv2 Report for %s : ", mca);)

		// Ignore node and link local multicast addresses
		if (	!IN6_IS_ADDR_MC_NODELOCAL(&grec->grec_mca) &&
			!IN6_IS_ADDR_MC_LINKLOCAL(&grec->grec_mca))
		{
			inet_ntop(AF_INET6, &grec->grec_mca, mca, sizeof(mca));

			// Find the grpintnode or create it
			grpintn = groupint_get(&grec->grec_mca, intn);
			if (!grpintn) dolog(LOG_WARNING, "%5s L4:IPv6:ICMPv6:MLD2_Report Couldn't find or create new group for %s\n", intn->name, mca);
		}

		nsrcs = ntohs(grec->grec_nsrcs);
		src = ((void *)grec) + sizeof(*grec);
		// Do all source addresses
		while (nsrcs > 0)
		{
			// Skip if we didn't get a grpint
			if (grpintn)
			{
				inet_ntop(AF_INET6, src, srct, sizeof(srct));
				if (!grpint_refresh(grpintn, src)) dolog(LOG_WARNING, "Couldn't subscribe %s to %s<->\n", intn->name, mca, srct);
			}

			// next src
			src = ((void *)src) + sizeof(*src);
		}
		
		// next grec
		grec = ((void *)grec) + sizeof(*grec);
	}

	return;
}
#endif // ECMH_SUPPORT_MLD2

// Check the ICMPv6 message for MLD's
//
// intn		= The interface we received this packet on
// packet	= The packet, starting with IPv6 header
// len		= Length of the complete packet
// data		= the payload
// plen		= Payload length (should match up to at least icmpv6
void l4_ipv6_icmpv6(struct intnode *intn, const struct ip6_hdr *iph, const uint16_t len, struct icmp6_hdr *icmpv6, const uint16_t plen)
{
	uint16_t		csum;
	struct grpintnode	*grpint;

	// MLD's have to be from Link Local addresses
	if (IN6_IS_ADDR_MC_LINKLOCAL(&iph->ip6_src))
	{
		return;
	}

	// We are only interrested in these types
	// Saves on calculating a checksum and then ignoring it anyways
	/*move outof this function*/
	/*if (	icmpv6->icmp6_type != ICMP6_MEMBERSHIP_REPORT &&
		icmpv6->icmp6_type != ICMP6_MEMBERSHIP_REDUCTION &&
		icmpv6->icmp6_type != ICMP6_V2_MEMBERSHIP_REPORT)
	{
		return;
	}*/

	// Save the checksum
	csum = icmpv6->icmp6_cksum;
	// Clear it temporarily
	icmpv6->icmp6_cksum = 0;

	// Verify checksum
	icmpv6->icmp6_cksum = ipv6_checksum(iph, IPPROTO_ICMPV6, (uint8_t *)icmpv6, plen);
	if (icmpv6->icmp6_cksum != csum)
	{
		dolog(LOG_WARNING, "CORRUPT->DROP (%s): Received a ICMPv6 %s (%d) with wrong checksum (%x vs %x)\n",
			intn->name,
			 icmpv6->icmp6_type == ICMP6_MEMBERSHIP_REPORT    ? "MLDv1 Membership Report" :
			(icmpv6->icmp6_type == ICMP6_MEMBERSHIP_REDUCTION ? "MLDv1 Membership Reduction" :
			(icmpv6->icmp6_type == ICMP6_V2_MEMBERSHIP_REPORT ? "MLDv2 Membership Report" : "Unknown")),
			icmpv6->icmp6_type,
			icmpv6->icmp6_cksum, csum);
	}

	// We are only interrested in reports & done's
	if (icmpv6->icmp6_type == ICMP6_MEMBERSHIP_REPORT)
	{
		l4_ipv6_icmpv6_mld1_report(intn, iph, len, (struct mld1 *)icmpv6, plen);
	}
	else if (icmpv6->icmp6_type == ICMP6_MEMBERSHIP_REDUCTION)
	{
		l4_ipv6_icmpv6_mld1_reduction(intn, iph, len, (struct mld1 *)icmpv6, plen);
	}
#ifdef ECMH_SUPPORT_MLD2
	else if (icmpv6->icmp6_type == ICMP6_V2_MEMBERSHIP_REPORT)
	{
		l4_ipv6_icmpv6_mld2_report(intn, iph, len, (struct mld2_report *)icmpv6, plen);
	}
#endif // ECMH_SUPPORT_MLD2
	return;
}

// Forward a multicast packet to interfaces that have subscriptions for it
//
// intn		= The interface we received this packet on
// packet	= The packet, starting with IPv6 header
// len		= Length of the complete packet
void l4_ipv6_multicast(struct intnode *intn, struct ip6_hdr *iph, const uint16_t len)
{
	struct groupnode	*groupn;
	struct grpintnode	*grpintn;
	struct subscrnode	*subscrn;
	struct listnode		*in, *in2;
	struct ifreq	ifreq;
	// DEBUG
	//char			src[60], dst[60];
	// Don't route multicast packets that:
	// - src = multicast
	// - src = unspecified
	// - dst = unspecified
	// - src = linklocal
	// - dst = node local multicast
	// - dst = link local multicast
	// - dst = reserved zero
	if (IN6_IS_ADDR_MULTICAST(&iph->ip6_src) ||
		IN6_IS_ADDR_UNSPECIFIED(&iph->ip6_src) ||
		IN6_IS_ADDR_UNSPECIFIED(&iph->ip6_dst) ||
		IN6_IS_ADDR_LINKLOCAL(&iph->ip6_src) ||
		IN6_IS_ADDR_MC_NODELOCAL(&iph->ip6_dst) ||
		IN6_IS_ADDR_MC_LINKLOCAL(&iph->ip6_dst) ||
		IN6_IS_ADDR_MC_ZERO(&iph->ip6_dst)
		) 
		return;

	// DEBUG: The addresses for printing
	//inet_ntop(AF_INET6, &iph->ip6_src, src, sizeof(src));
	//inet_ntop(AF_INET6, &iph->ip6_dst, dst, sizeof(dst));

	//D(dolog(LOG_DEBUG, "%5s L3:IPv6: IPv%0x %40s %40s %4u %d\n", intn->name, (int)((iph->ip6_vfc>>4)&0x0f), src, dst, ntohs(iph->ip6_plen), iph->ip6_nxt);)
	
	// Find the group belonging to this multicast destination
	groupn = group_find(&iph->ip6_dst);
	
	if (!groupn)
	{
		// DEBUG
		D(dolog(LOG_DEBUG, "No subscriptions for %s (sent by %s)\n",  dst, src);)
		return;
	}

	LIST_LOOP(groupn->interfaces, grpintn, in)
	{
		// Don't send to the interface this packet originated from
		if (intn->ifindex == grpintn->interface->ifindex) continue;

		// Check the subscriptions for this group
		LIST_LOOP(grpintn->subscriptions, subscrn, in2)
		{
			// Unspecified or specific subscription to this address?
			if (COMPARE_IPV6_ADDRESS(subscrn->ipv6, in6addr_any) ||
				COMPARE_IPV6_ADDRESS(subscrn->ipv6, iph->ip6_src))
			{
				/*check mtu before fowarding*/
				memset(&ifreq, 0, sizeof(ifreq));
				ifreq.ifr_ifindex = grpintn->interface->ifindex;
				memcpy(&ifreq.ifr_name,grpintn->interface->name,IFNAMSIZ);
				if (ioctl(g_conf->rawsocket, SIOCGIFMTU, &ifreq) != 0)
				{
					dolog(LOG_ERR, "Couldn't determine MTU size for %s, link %d : %s\n", intn->name, intn->ifindex, strerror(errno));
					perror("Error");
					
					return ;
				}
				grpintn->interface->mtu=ifreq.ifr_mtu;
				if(len > grpintn->interface->mtu){
					icmpv6_pkt_send(iph,grpintn->interface->mtu);
					return;
				}
				else
					sendpacket6(grpintn->interface, iph, len);
				
				// Even if there are multiple source subscriptions
				// we only send it once, let the router/hosts
				// on the interface determine that they wanted it
				break;
			}
		}
	}
}

void l3_ipv6(struct intnode *intn, struct ip6_hdr *iph, const uint16_t len)
{
	struct ip6_ext		*ipe;
	uint8_t			ipe_type;
	uint16_t		plen;
	uint32_t		l;
	struct icmp6_hdr	*icmpv6 = NULL;
	struct groupnode	*groupn;
	struct grpintnode	*grpintn;
	struct subscrnode	*subscrn;
	struct listnode		*in, *in2;
	uint8_t			*phopopt;
	uint8_t			hopopt_type;
	
	// Prefilter - ignore anything but:
	// - source must be link-local
	// OR
	// - destination must be multicast
	if (	!IN6_IS_ADDR_LINKLOCAL(&iph->ip6_src) &&
		!IN6_IS_ADDR_MULTICAST(&iph->ip6_dst))
	{
		return;
	}
	
	// Save the type of the next header
	ipe_type = iph->ip6_nxt;
	// Step to the next header
	ipe = (struct ip6_ext *)(((void *)iph) + sizeof(*iph));
	plen = ntohs(iph->ip6_plen);

	// Skip the headers that we know
	while (	ipe_type == IPPROTO_HOPOPTS ||
		ipe_type == IPPROTO_ROUTING ||
		ipe_type == IPPROTO_DSTOPTS ||
		ipe_type == IPPROTO_AH)
	{
		/*skip unknown hopopts type*/
		if(ipe_type == IPPROTO_HOPOPTS){
			phopopt=(uint8_t*)ipe;
			/*step to option type*/
			hopopt_type = *(phopopt+2);
			if(hopopt_type != IPV6_TLV_PAD0 &&
				hopopt_type != IPV6_TLV_PADN &&
				hopopt_type != IPV6_TLV_ROUTERALERT &&
				hopopt_type != IPV6_TLV_JUMBO &&
				hopopt_type !=IPV6_TLV_HAO)	{
				/*unknown type */
				return	;
			}				
		}
		// Save the type of the next header				
		ipe_type = ipe->ip6e_nxt;
		
		// Step to the next header
		l = ((ipe->ip6e_len*8)+8);
		plen -= l;
		ipe  = (struct ip6_ext *)(((void *)ipe) + l);

		// Check for corrupt packets
		if ((void *)ipe > (((void *)iph)+len))
		{
			dolog(LOG_WARNING, "CORRUPT->DROP (%s): Header chain beyond packet data\n", intn->name);
			return;
		}
	}

	// Check for ICMP
	if (ipe_type == IPPROTO_ICMPV6)
	{
		// Take care of ICMPv6
		icmpv6=(struct icmp6_hdr *)ipe;
		if (	icmpv6->icmp6_type == ICMP6_MEMBERSHIP_REPORT ||
			icmpv6->icmp6_type == ICMP6_MEMBERSHIP_REDUCTION ||
			icmpv6->icmp6_type == ICMP6_V2_MEMBERSHIP_REPORT){
			l4_ipv6_icmpv6(intn, iph, len, icmpv6, plen);
			return;
		}
	}

	// Handle multicast packets
	if (IN6_IS_ADDR_MULTICAST(&iph->ip6_dst))
	{
		/* Decrease the hoplimit, but only if not 0 yet */
		if (iph->ip6_hlim > 0) 
				iph->ip6_hlim--;
		
		if (iph->ip6_hlim == 0){
			/*g_conf->stat_hlim_exceeded++;*/
			return;
		}
		
		l4_ipv6_multicast(intn, iph, len);
		return;
	}

	// Ignore the rest
	return;
}

void l2(struct intnode *intn, const uint8_t *packet, const uint16_t len, u_int16_t ether_type)
{
#ifdef ECMH_SUPPORT_IPV4
	if (ether_type == ETHERTYPE_IP)
	{
		l3_ipv4(intn, packet, len);
		return;
	}
	else
#endif // ECMH_SUPPORT_IPV4
	if (ether_type == ETH_P_IPV6)
	{
		l3_ipv6(intn, (struct ip6_hdr *)packet, len);
		return;
	}
	
	// We don't care about anything else...
	return;
}

// Initiliaze interfaces 
void update_interfaces(struct intnode *intn)
{
	FILE		*file;
	char		devname[IFNAMSIZ];
	struct in6_addr	linklocal;
	int		ifindex = 0, prefixlen, scope, flags;

	D(dolog(LOG_DEBUG, "Updating Interfaces\n");)

	// Get link local addresses from /proc/net/if_inet6
	file = fopen("/proc/net/if_inet6", "r");
	
	// We can live without it though
	if (!file)
	{
		dolog(LOG_WARNING, "Couldn't open /proc/net/if_inet6 for figuring out local interfaces\n");
		return;
	}
	
	// Format "fe80000000000000029027fffe24bbab 02 0a 20 80     eth0"
	while (	fscanf(	file,
			"%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx %x %x %x %x %8s",
			&linklocal.s6_addr[ 0], &linklocal.s6_addr[ 1], &linklocal.s6_addr[ 2], &linklocal.s6_addr[ 3],
			&linklocal.s6_addr[ 4], &linklocal.s6_addr[ 5], &linklocal.s6_addr[ 6], &linklocal.s6_addr[ 7],
			&linklocal.s6_addr[ 8], &linklocal.s6_addr[ 9], &linklocal.s6_addr[10], &linklocal.s6_addr[11],
			&linklocal.s6_addr[12], &linklocal.s6_addr[13], &linklocal.s6_addr[14], &linklocal.s6_addr[15],
			&ifindex, &prefixlen, &scope, &flags, devname)!=EOF){		
		// Skip all non-link local addresses
		if (!IN6_IS_ADDR_LINKLOCAL(&linklocal)) 
				continue;
		
		if (intn)
		{
			// Was this the one to update?
			if (intn->ifindex == ifindex)
			{
				// Update the linklocal address
				memcpy(&intn->linklocal, &linklocal, sizeof(intn->linklocal));
				// We are done updating
				break;
			}
		}
		// Update everything
		else
		{
			intn = int_find(ifindex);
			if (!intn)
			{
				intn = int_create(ifindex);			
			
				if (intn){
					// Fill in the linklocal
					memcpy(&intn->linklocal, &linklocal, sizeof(intn->linklocal));
					// Add it to the list
					int_add(intn);
				}
			}
			else
			{
				// Just update the linklocal
				memcpy(&intn->linklocal, &linklocal, sizeof(intn->linklocal));
			}

			// We where not searching for a specific interface
			// thus clear this out
			intn = NULL;
		}
	}
	D(dolog(LOG_DEBUG, "*** Updating Interfaces - done\n");)
	fclose(file);
}

void init()
{
	g_conf = malloc(sizeof(struct conf));
	if (!g_conf)
	{
		dolog(LOG_ERR, "Couldn't init()\n");
		exit(-1);
	}

	// Initialize our configuration
	g_conf->maxgroups	= 42;
	g_conf->daemonize	= true;

	// Initialize our list of interfaces
	g_conf->ints = list_new();
	g_conf->ints->del = (void(*)(void *))int_destroy;

	// Initialize our list of groups
	g_conf->groups = list_new();
	g_conf->groups->del = (void(*)(void *))group_destroy;

	// Initialize our counters
	g_conf->stat_starttime		= time(NULL);
	g_conf->stat_packets_received	= 0;
	g_conf->stat_packets_sent	= 0;
	g_conf->stat_bytes_received	= 0;
	g_conf->stat_bytes_sent		= 0;
}

void sighup(int i)
{
	// Reset the signal
	signal(SIGHUP, &sighup);
}

// Dump the group subscription list
void sigusr1(int i)
{
	struct intnode		*intn;
	struct groupnode	*groupn;
	struct listnode		*ln;
	struct grpintnode	*grpintn;
	struct listnode		*gn;
	struct subscrnode	*subscrn;
	struct listnode		*ssn;
	time_t			time_tee;
	char			addr[60];
	int			subscriptions = 0;
	int			uptime, uptime_s, uptime_m, uptime_h, uptime_d;
	FILE			*out;

	// Get the current time
	time_tee  = time(NULL);
	uptime_s  = time_tee - g_conf->stat_starttime;
	uptime_d  = uptime_s / (24*60*60);
	uptime_s -= uptime_d *  24*60*60;
	uptime_h  = uptime_s / (60*60);
	uptime_s -= uptime_h *  60*60;
	uptime_m  = uptime_s /  60;
	uptime_s -= uptime_m *  60;
	
	out = fopen(ECMH_DUMPFILE, "w");
	if (!out)
	{
		dolog(LOG_WARNING, "Couldn't open statistics file \"%s\" for dumping\n", ECMH_DUMPFILE);
		return;
	}
	
	fprintf(out, "*** Subscription Information Dump\n");

	// Timeout all the groups that didn't refresh yet
	LIST_LOOP(g_conf->groups, groupn, ln)
	{
		inet_ntop(AF_INET6, &groupn->mca, addr, sizeof(addr));
		fprintf(out, "Group: %s\n", addr);

		LIST_LOOP(groupn->interfaces, grpintn, gn)
		{
			fprintf(out, "\tInterface: %s\n", grpintn->interface->name);

			LIST_LOOP(grpintn->subscriptions, subscrn, ssn)
			{
				inet_ntop(AF_INET6, &subscrn->ipv6, addr, sizeof(addr));
				fprintf(out, "\t\t%s\n", addr);
				subscriptions++;
			}
		}
	}
	
	fprintf(out, "*** Subscription Information Dump (end - %d groups, %d subscriptions)\n", g_conf->groups->count, subscriptions);
	fprintf(out, "\n");

	fprintf(out, "*** Interface Dump\n");

	// Timeout all the groups that didn't refresh yet
	LIST_LOOP(g_conf->ints, intn, ln)
	{
		// If it is marked for being removed, do so
		if (intn->removeme) listnode_delete(g_conf->ints, ln);
		else
		{
			inet_ntop(AF_INET6, &intn->linklocal, addr, sizeof(addr));
	
			fprintf(out, "\n");
			fprintf(out, "Interface: %s\n", intn->name);
			fprintf(out, "  Index  : %d\n", intn->ifindex);
			fprintf(out, "  MTU    : %d\n", intn->mtu);
			fprintf(out, "  LL     : %s\n", addr);
		}
	}

	fprintf(out, "\n");
	fprintf(out, "*** Interface Dump (end - %d interfaces)\n", g_conf->ints->count);
	fprintf(out, "\n");

	strftime(addr, sizeof(addr), "%F %T", gmtime(&time_tee));

	fprintf(out, "*** Statistics Dump\n");
	fprintf(out, "Version              : ecmh %s\n", ECMH_VERSION);
	fprintf(out, "Started              : %s\n", addr);
	fprintf(out, "Uptime               : %d days %02d:%02d:%02d\n", uptime_d, uptime_h, uptime_m, uptime_s);
	fprintf(out, "Interfaces Monitored : %d\n", g_conf->ints->count);
	fprintf(out, "Groups Managed       : %d\n", g_conf->groups->count);
	fprintf(out, "Total Subscriptions  : %d\n", subscriptions);
	fprintf(out, "Packets Received     : %lld\n", g_conf->stat_packets_received);
	fprintf(out, "Packets Sent         : %lld\n", g_conf->stat_packets_sent);
	fprintf(out, "Bytes Received       : %lld\n", g_conf->stat_bytes_received);
	fprintf(out, "Bytes Sent           : %lld\n", g_conf->stat_bytes_sent);
	fprintf(out, "*** Statistics Dump (end)\n");

	// Close our stats file
	fclose(out);

	dolog(LOG_INFO, "Dumped statistics into %s\n", ECMH_DUMPFILE);

	// Reset the signal
	signal(SIGUSR1, &sigusr1);
}

void send_mld_querys()
{
	struct intnode		*intn;
	struct listnode		*ln;
	
	D(dolog(LOG_DEBUG, "*** Sending MLD Queries\n");)

	// Send a MLD query's
	LIST_LOOP(g_conf->ints, intn, ln)
	{
		// Only send if it isn't marked for removal yet
		if (!intn->removeme) mld_send_query(intn);
		
		// If it is marked for being removed, do so
		if (intn->removeme) listnode_delete(g_conf->ints, ln);
	}

	D(dolog(LOG_DEBUG, "*** Sending MLD Queries - done\n");)
}

void timeout()
{
	struct groupnode	*groupn;
	struct listnode		*ln;
	struct grpintnode	*grpintn;
	struct listnode		*gn;
	struct subscrnode	*subscrn;
	struct listnode		*ssn;
	time_t			time_tee;
	
	D(dolog(LOG_DEBUG, "*** Timeout\n");)

	// Update the complete interfaces list
	update_interfaces(NULL);

	// Get the current time
	time_tee = time(NULL);

	// Timeout all the groups that didn't refresh yet
	LIST_LOOP(g_conf->groups, groupn, ln)
	{
		LIST_LOOP(groupn->interfaces, grpintn, gn)
		{
			LIST_LOOP(grpintn->subscriptions, subscrn, ssn)
			{
				// Calculate the difference
				int i = time_tee - subscrn->refreshtime;
				if (i < 0) i = -i;
			
				// Still alive?
				if (i < (ECMH_SUBSCRIPTION_TIMEOUT)) continue;
	
				// Dead too long -> delete it
				listnode_delete(grpintn->subscriptions, ssn);
			}
		
			if (grpintn->subscriptions->count == 0)
			{
				// Destroy this groupint
				listnode_delete(groupn->interfaces, gn);
			}
		}
	
		if (groupn->interfaces->count == 0)
		{
			// Destroy this group
			listnode_delete(g_conf->groups, ln);
		}
	}

	// Send out MLD queries
	send_mld_querys();

	D(dolog(LOG_DEBUG, "*** Timeout - done\n");)

	signal(SIGALRM, &timeout);
	alarm(ECMH_SUBSCRIPTION_TIMEOUT);
}

int main(int argc, char *argv[], char *envp[])
{
	int			i=0, len;
	
	char			buffer[8192];
	struct sockaddr_ll	sa;
	socklen_t		salen;
	struct ether_header	*hdr_ether = (struct ether_header *)&buffer;
	struct iphdr		*hdr_ip = (struct iphdr *)&buffer;

	struct intnode		*intn = NULL;

	init();
	
	if (argc >= 2 &&
		strcmp(argv[1], "-f")!=0)
	{
		fprintf(stderr,
			"%s -f\n"
			"\n"
			"-f == don't daemonize\n",
			argv[0]);
		return -1;
	}

	if (argc == 2)
	{
		if (strcmp(argv[1], "-f")==0) g_conf->daemonize = true;
	}

	// Daemonize
	if (g_conf->daemonize)
	{
		int i = fork();
		if (i < 0)
		{
			fprintf(stderr, "Couldn't fork\n");
			return -1;
		}
		// Exit the mother fork
		if (i != 0) return 0;

		// Child fork
		setsid();
		// Cleanup stdin/out/err
		//freopen("/dev/null","r",stdin);
		//freopen("/dev/null","w",stdout);
		//freopen("/dev/null","w",stderr);
	}

	// Handle a SIGHUP to reload the config
	signal(SIGHUP, &sighup);

	// Handle SIGTERM/INT/KILL to cleanup the pid file
	signal(SIGTERM,	&cleanpid);
	signal(SIGINT,	&cleanpid);
	signal(SIGKILL,	&cleanpid);

	// Timeout handling
	signal(SIGALRM, &timeout);
	alarm(ECMH_SUBSCRIPTION_TIMEOUT);

	// Dump operations
	signal(SIGUSR1,	&sigusr1);
	
	signal(SIGUSR2, SIG_IGN);

	// Show our version in the startup logs ;)
	dolog(LOG_INFO, "ecmh %s by Jeroen Massar <jeroen@unfix.org>\n", ECMH_VERSION);

	// Save our PID
	savepid();

	// Allocate a PACKET socket which can send and receive
	//  anything we want (anything ???.... anythinggg... ;)
	// We only want IPv6 packets, we don't want to see anything else
	g_conf->rawsocket = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IPV6));
	if (g_conf->rawsocket < 0)
	{
		dolog(LOG_ERR, "Couldn't allocate a RAW socket\n");
		return -1;
	}

	// Allocate a PACKET socket which can send and receive
	//  anything we want (anything ???.... anythinggg... ;)
	g_conf->rawsocket_out = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_ALL));
	if (g_conf->rawsocket_out < 0)
	{
		dolog(LOG_ERR, "Couldn't allocate a RAW socket\n");
		return -1;
	}
	/*add to send icmp error message*/
	g_conf->sock_icmp = open_ecmh_icmpv6_socket();
	if (g_conf->sock_icmp < 0)
	{
		dolog(LOG_ERR, "Couldn't allocate a RAW socket\n");
		return -1;
	}
	// Fix our priority, we need to be near realtime
	//if (setpriority(PRIO_PROCESS, getpid(), -15) == -1)
	//{
	//	dolog(LOG_WARNING, "Couldn't raise priority to -15, if streams are shaky, upgrade your cpu or fix this\n");
	//}

	// Update the complete interfaces list
	update_interfaces(NULL);
	
	send_mld_querys();

	len = 0;
	while (len != -1)
	{
		salen = sizeof(sa);
		memset(&sa, 0, sizeof(sa));
		len = recvfrom(g_conf->rawsocket, &buffer, sizeof(buffer), 0, (struct sockaddr *)&sa, &salen);
		
		if (len == -1) break;
		
		// Ignore:
		// - loopback traffic
		// - any packets that originate from this host
		if(sa.sll_hatype == ARPHRD_LOOPBACK ||
			sa.sll_pkttype == PACKET_OUTGOING) continue;
		
		// Update statistics
		g_conf->stat_packets_received++;
		g_conf->stat_bytes_received+=len;

		intn = int_find(sa.sll_ifindex);
		if (!intn)
		{
			// Create a new interface
			intn = int_create(sa.sll_ifindex);
			if (intn)
			{
				// Determine linklocal address etc.
				update_interfaces(intn);

				// Add it to the list
				int_add(intn);
			}
		}

		if (!intn)
		{
			dolog(LOG_ERR, "Couldn't find nor add interface link %d\n", sa.sll_ifindex);
			break;
		}
		// Directly has a IP header so pretend it is IPv4
		// and use that header to find out the real version
#ifdef ECMH_SUPPORT_IPV4	
		if (hdr_ip->version== 4) l3_ipv4(intn, buffer, len);
		else
#endif //ECMH_SUPPORT_IPV4
		if (hdr_ip->version== 6) l3_ipv6(intn, (struct ip6_hdr *)buffer, len);

		// Ignore the rest
//		D(else dolog(LOG_DEBUG, "%5s Unknown IP version %d\n", intn->name, hdr_ip->ip_v);)
		fflush(stdout);

	}

	cleanpid(SIGINT);

	return 0;
}
