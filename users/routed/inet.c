/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * From: @(#)inet.c	5.8 (Berkeley) 6/1/90
 * From: @(#)inet.c	8.2 (Berkeley) 8/14/93
 */
char inet_rcsid[] = 
  "$Id: inet.c,v 1.4 2009/09/15 02:14:10 bradhuang Exp $";


/*
 * Temporarily, copy these routines from the kernel,
 * as we need to know about subnets.
 */
#include "defs.h"

extern struct interface *ifnet;

#if !defined(__UCLIBC__) && (!defined(__GLIBC__) || (__GLIBC__ < 2))
/*
 * Formulate an Internet address from network + host.
 */
struct in_addr inet_makeaddr(u_long net, u_long host)
{
	struct interface *ifp;
	u_long mask;
	u_long addr;

	if (IN_CLASSA(net))
		mask = IN_CLASSA_HOST;
	else if (IN_CLASSB(net))
		mask = IN_CLASSB_HOST;
	else
		mask = IN_CLASSC_HOST;
	for (ifp = ifnet; ifp; ifp = ifp->int_next)
		if ((ifp->int_netmask & net) == ifp->int_net) {
			mask = ~ifp->int_subnetmask;
			break;
		}
	addr = net | (host & mask);
	addr = htonl(addr);
	return (*(struct in_addr *)&addr);
}
#endif /* not glibc */

/*
 * Return the network number from an internet address.
 */
u_long inet_netof_subnet(struct in_addr in)
{
	u_long i = ntohl(in.s_addr);
	u_long net;
	struct interface *ifp;

	if (IN_CLASSA(i))
		net = i & IN_CLASSA_NET;
	else if (IN_CLASSB(i))
		net = i & IN_CLASSB_NET;
	else
		net = i & IN_CLASSC_NET;

	/*
	 * Check whether network is a subnet;
	 * if so, return subnet number.
	 */
	for (ifp = ifnet; ifp; ifp = ifp->int_next)
		if ((ifp->int_netmask & net) == ifp->int_net)
			return (i & ifp->int_subnetmask);
	return (net);
}

/*
 * Return the netmask pertaining to an internet address.
 */

int inet_maskof(u_long inaddr)
{
	u_long i = ntohl(inaddr);
	u_long mask;
	struct interface *ifp;

	if (i == 0) {
		mask = 0;
	} else if (IN_CLASSA(i)) {
		mask = IN_CLASSA_NET;
	} else if (IN_CLASSB(i)) {
		mask = IN_CLASSB_NET;
	} else
		mask = IN_CLASSC_NET;

	/*
	 * Check whether network is a subnet;
	 * if so, use the modified interpretation of `host'.
	 */
	for (ifp = ifnet; ifp; ifp = ifp->int_next)
		if ((ifp->int_netmask & i) == ifp->int_net)
			mask = ifp->int_subnetmask;
	return (htonl(mask));
}

/*
 * Return RTF_HOST if the address is
 * for an Internet host, RTF_SUBNET for a subnet,
 * 0 for a network.
 */

int inet_rtflags(struct sockaddr *sa)
{
	struct sockaddr_in *sin=(struct sockaddr_in *)sa;
	u_long i = ntohl(sin->sin_addr.s_addr);
	u_long net, host;
	struct interface *ifp;

	if (IN_CLASSA(i)) {
		net = i & IN_CLASSA_NET;
		host = i & IN_CLASSA_HOST;
	} else if (IN_CLASSB(i)) {
		net = i & IN_CLASSB_NET;
		host = i & IN_CLASSB_HOST;
	} else {
		net = i & IN_CLASSC_NET;
		host = i & IN_CLASSC_HOST;
	}

	/*
	 * Check whether this network is subnetted;
	 * if so, check whether this is a subnet or a host.
	 */
	for (ifp = ifnet; ifp; ifp = ifp->int_next)
		if (net == ifp->int_net) {
			if (host &~ ifp->int_subnetmask)
				return (RTF_HOST);
			else if (ifp->int_subnetmask != ifp->int_netmask)
				return (RTF_SUBNET);
			else
				return (0);		/* network */
		}
	if (host == 0)
		return (0);	/* network */
	else
		return (RTF_HOST);
}

/*
 * Return true if a route to subnet/host of route rt should be sent to dst.
 * Send it only if dst is on the same logical network if not "internal",
 * otherwise only if the route is the "internal" route for the logical net.
 */
 //WiSOC modify the route check in below, we should make sure the static route entry will be send out if the entry is valid
#if 0
int inet_sendroute(struct rt_entry *rt, struct sockaddr *sa)
{
	struct sockaddr_in *dst=(struct sockaddr_in *)sa;
	u_long r =
	    ntohl(((struct sockaddr_in *)&rt->rt_dst)->sin_addr.s_addr);
	u_long d = ntohl(dst->sin_addr.s_addr);

	if (IN_CLASSA(r)) {
		if ((r & IN_CLASSA_NET) == (d & IN_CLASSA_NET)) {
			if ((r & IN_CLASSA_HOST) == 0)
				return ((rt->rt_state & RTS_INTERNAL) == 0);
			return (1);
		}
		if (r & IN_CLASSA_HOST)
			return (0);
		return ((rt->rt_state & RTS_INTERNAL) != 0);
	} else if (IN_CLASSB(r)) {
		if ((r & IN_CLASSB_NET) == (d & IN_CLASSB_NET)) {
			if ((r & IN_CLASSB_HOST) == 0)
				return ((rt->rt_state & RTS_INTERNAL) == 0);
			return (1);
		}
		if (r & IN_CLASSB_HOST)
			return (0);
		return ((rt->rt_state & RTS_INTERNAL) != 0);
	} else {
		if ((r & IN_CLASSC_NET) == (d & IN_CLASSC_NET)) {
			if ((r & IN_CLASSC_HOST) == 0)
				return ((rt->rt_state & RTS_INTERNAL) == 0);
			return (1);
		}
		if (r & IN_CLASSC_HOST)
			return (0);
		return ((rt->rt_state & RTS_INTERNAL) != 0);
	}
}
#endif
int inet_sendroute(struct rt_entry *rt, struct sockaddr *sa)
{
	u_long addr1=0, addr2=0, addr3=0, addr4=0;
	u_long m1=0, m2=0, m3=0, m4=0;
	struct sockaddr_in *dst=(struct sockaddr_in *)sa;
	u_long r =ntohl(((struct sockaddr_in *)&rt->rt_dst)->sin_addr.s_addr);
	u_long m =ntohl(((struct sockaddr_in *)&rt->rt_netmask)->sin_addr.s_addr);
	u_long d = ntohl(dst->sin_addr.s_addr);
	if (IN_CLASSA(r)) {
		if ((r & IN_CLASSA_NET) == (d & IN_CLASSA_NET)) {
			if ((r & IN_CLASSA_HOST) == 0){
				//WiSOC it is redundant entry for the same subnet 
				if((rt->rt_state & RTS_INTERFACE) != 0){
					return (0);
				}else{
					return ((rt->rt_state & RTS_INTERNAL) == 0);
				}
			}
			return (1);
		}
		if (r & IN_CLASSA_HOST){//WiSOC should make sure the static route entry can be send out, event it's host route 
				if(((rt->rt_state & RTS_INTERNAL) != 0) && ((rt->rt_state & RTS_PASSIVE) != 0) && (IN_MULTICAST(r)==0))
					return (1);
				else{
					if(IN_MULTICAST(r)==0){
						addr1 = (r & m) & 0xFF000000;
						addr2 = (r & m) & 0x00FF0000;
						addr3 = (r & m) & 0x0000FF00;
						addr4 = (r & m) & 0x000000FF;
						if(((addr1 != 0) && (addr2 != 0) && (addr3 != 0) && (addr4 != 0))){
							return (0);
						}else{
								if((rt->rt_state & RTS_INTERFACE) == 0)
									return (1);
								else
									return (0);
							}
					}else{
						return (0);
					}
				}	
		}
		if(IN_MULTICAST(r)==0 && (((rt->rt_state & RTS_INTERNAL) != 0) || ((rt->rt_state & RTS_INTERFACE) != 0))) //WiSOC we donot send multicast entry, since we treat multicast entry as bad host in receive
			return(1);
		else
			return (0);
		
	} else if (IN_CLASSB(r)) {
		if ((r & IN_CLASSB_NET) == (d & IN_CLASSB_NET)) {
			if ((r & IN_CLASSB_HOST) == 0){
				//WiSOC it is redundant entry for the same subnet 
				if((rt->rt_state & RTS_INTERFACE) != 0)
					return (0);
				else
					return ((rt->rt_state & RTS_INTERNAL) == 0);
			}	
			return (1);
		}
		if (r & IN_CLASSB_HOST){ //WiSOC should make sure the static route entry can be send out, event it's host route 
			if(((rt->rt_state & RTS_INTERNAL) != 0) && ((rt->rt_state & RTS_PASSIVE) != 0) && (IN_MULTICAST(r)==0))
				return (1);
			else{
					if(IN_MULTICAST(r)==0){
						addr1 = (r & m) & 0xFF000000;
						addr2 = (r & m) & 0x00FF0000;
						addr3 = (r & m) & 0x0000FF00;
						addr4 = (r & m) & 0x000000FF;
						if(((addr1 != 0) && (addr2 != 0) && (addr3 != 0) && (addr4 != 0))){
							return (0);
						}else{
							if((rt->rt_state & RTS_INTERFACE) == 0)
								return (1);
							else
								return (0);
							}
					}else{
						return (0);
					}
				}	
		}
		if(IN_MULTICAST(r)==0 && (((rt->rt_state & RTS_INTERNAL) != 0) || ((rt->rt_state & RTS_INTERFACE) != 0))) //WiSOC we donot send multicast entry, since we treat multicast entry as bad host in receive
			return(1);
		else
			return (0);
	} else {
		if ((r & IN_CLASSC_NET) == (d & IN_CLASSC_NET)) {
			if ((r & IN_CLASSC_HOST) == 0){
				//WiSOC it is redundant entry for the same subnet 
					if((rt->rt_state & RTS_INTERFACE) != 0)
						return (0);
					else
						return ((rt->rt_state & RTS_INTERNAL) == 0);
				}	
				return (1);
		}
		if (r & IN_CLASSC_HOST){//WiSOC should make sure the static route entry can be send out, event it's host route 
			if(((rt->rt_state & RTS_INTERNAL) != 0) && ((rt->rt_state & RTS_PASSIVE) != 0) && (IN_MULTICAST(r)==0))
				return (1);
			else
				return (0);
		}
		if(IN_MULTICAST(r)==0 && (((rt->rt_state & RTS_INTERNAL) != 0) || ((rt->rt_state & RTS_INTERFACE) != 0)))
			return(1);
		else
			return (0);
	}
}
