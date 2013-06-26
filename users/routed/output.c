/*
 * Copyright (c) 1983, 1988, 1993
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
 * From: @(#)output.c	5.15 (Berkeley) 2/28/91
 * From: @(#)output.c	8.1 (Berkeley) 6/5/93
 */
char output_rcsid[] = 
  "$Id: output.c,v 1.2 2009/08/25 11:23:16 bradhuang Exp $";


/*
 * Routing Table Management Daemon
 */

#include "defs.h"
#if 0//WiSOC unused
#include <rtk/sysconfig.h>
//ql
#include <config/autoconf.h>
#endif
extern int ripversion;
extern int debug;

/*
 * Apply the function "f" to all non-passive
 * interfaces.  If the interface supports the
 * use of broadcasting use it, otherwise address
 * the output to the known router.
 */

void toall(void (*f)(struct sockaddr *, int, struct interface *, int), 
	int rtstate, struct interface *skipif)
{
	struct interface *ifp;
	struct sockaddr *dst;
	int flags;
	struct cfg_interface *ifcp;    // Added by Mason Yu for bind interface

	//if(!ripversion)
	//	return;

	for (ifp = ifnet; ifp; ifp = ifp->int_next) {
		/* check if this interface be configured to supply RIP */
		// Modified by Mason Yu for bind interface
		//if(!if_cfg_lookup(ifp->int_name))
		//	continue;
		ifcp = (struct cfg_interface *)if_cfg_lookup(ifp->int_name);
		// Kaohj -- check send mode
		//if (!ifcp)
		if (!ifcp || ifcp->send_mode == RIP_NONE)
			continue;		

		// Added by Mason Yu for bind interface
		// Kaohj
		#if 0
		//ripversion = ifcp->send_mode;
		if ( ifcp->send_mode == RIP_V1_COMPAT ) {
			ripversion = RIP_V2;
			multicast = 0;
		} else if ( ifcp->send_mode == RIP_V2 ) {
			multicast = 1;
		}
		#else
		if (ifcp->send_mode == RIP_V2 || ifcp->send_mode == RIP_V1_V2)
			multicast = 1;
		else
			multicast = 0;
		#endif
		
		if (ifp->int_flags & IFF_PASSIVE || ifp == skipif)
			continue;
		dst = ifp->int_flags & IFF_BROADCAST ? &ifp->int_broadaddr :
		      ifp->int_flags & IFF_POINTOPOINT ? &ifp->int_dstaddr :
		      &ifp->int_addr;

		flags = ifp->int_flags & IFF_INTERFACE ? MSG_DONTROUTE : 0;
		// Casey, set multicast interface
		// Kaohj modified
		//if(ripversion == 2 && multicast) {
		if(multicast) {
			set_mcast_if(&ifp->int_addr);
		}		
		//		
		(*f)(dst, flags, ifp, rtstate);
	}
}

/*
 * Output a preformed packet.
 */

void sndmsg(struct sockaddr *dst, int flags, struct interface *ifp, int rtstate)
{
//WiSOC add send mode check for different network interface 
	struct cfg_interface *ifcp;
	struct rip *query = msg;
	ifcp = (struct cfg_interface *)if_cfg_lookup(ifp->int_name);
	
	(void)rtstate;
	//printf("ifname=%s, rip_ver=%d, ifcp->send_mode=%d\n", ifcp->if_name, query->rip_vers, ifcp->send_mode);
	if (ifcp->send_mode == RIP_V2 || ifcp->send_mode == RIP_V1_V2)
		query->rip_vers = 2; //version 2
	else
		query->rip_vers = 1;//version 1
		
	(*afswitch[dst->sa_family].af_output)(sock, flags,dst, sizeof (struct rip));
	TRACE_OUTPUT(ifp, dst, sizeof (struct rip));

}

/*
 * Supply dst with the contents of the routing tables.
 * If this won't fit in one packet, chop it up into several.
 */
 
void supply(struct sockaddr *dst, int flags, struct interface *ifp, int rtstate)
{
	struct rt_entry *rt;
	struct netinfo *n = msg->rip_nets;
	struct rthash *rh;
	struct rthash *base = hosthash;
	struct cfg_interface *ifcp;
	int doinghost = 1, size;
	void (*output)(int,int,struct sockaddr *,int) = 
		afswitch[dst->sa_family].af_output;
	int (*sendroute)(struct rt_entry *, struct sockaddr *) = 
		afswitch[dst->sa_family].af_sendroute;
	int npackets = 0;

	// Kaohj
	if (!ifp)
		return;
	ifcp = (struct cfg_interface *)if_cfg_lookup(ifp->int_name);
	if (!ifcp || ifcp->send_mode == RIP_NONE)
		return;
	//ql_xu
#ifdef CONFIG_BOA_WEB_E8B_CH
	if (ifcp->operation == RIP_IFF_PASSIVE)
		return;
#endif
	msg->rip_cmd = RIPCMD_RESPONSE;
	// Kaohj
	//msg->rip_vers = ripversion;
	if (ifcp->send_mode == RIP_V1)
		msg->rip_vers = RIPv1;
	else
		msg->rip_vers = RIPv2;
	
	// Mason Yu
	//printf("SEND: RIPv%d Response to %s\n", msg->rip_vers, inet_ntoa(satosin(*dst)->sin_addr));		
	if(debug)
		printf("SEND: RIPv%d Response to %s\n", msg->rip_vers, inet_ntoa(satosin(*dst)->sin_addr));

	memset(&msg->rip_res1, 0, sizeof(msg->rip_res1));
again:
	for (rh = base; rh < &base[ROUTEHASHSIZ]; rh++)		
	for (rt = rh->rt_forw; rt != (struct rt_entry *)rh; rt = rt->rt_forw) {
		/*
		
		 * Don't resend the information on the network
		 * from which it was received (unless sending
		 * in response to a query).
		 */				
		 
		if (ifp && rt->rt_ifp == ifp &&
		    (rt->rt_state & RTS_INTERFACE) == 0)
			continue;
		if (rt->rt_state & RTS_EXTERNAL)
			continue;
		//Casey, ignore default route
		if (!satosin(rt->rt_dst)->sin_addr.s_addr)
			continue;
		//
		/*
		 * For dynamic updates, limit update to routes
		 * with the specified state.
		 */
		if (rtstate && (rt->rt_state & rtstate) == 0)
			continue;
		/*
		 * Limit the spread of subnet information
		 * to those who are interested.
		 */
		if (doinghost == 0 && rt->rt_state & RTS_SUBNET) {
			if (rt->rt_dst.sa_family != dst->sa_family)
				continue;
			if ((*sendroute)(rt, dst) == 0)
				continue;
		}
		size = (char *)n - packet;
		if (size > MAXPACKETSIZE - (int)sizeof (struct netinfo)) {
			TRACE_OUTPUT(ifp, dst, size);
			(*output)(sock, flags, dst, size);
			/*
			 * If only sending to ourselves,
			 * one packet is enough to monitor interface.
			 */
			if (ifp && (ifp->int_flags &
			   (IFF_BROADCAST | IFF_POINTOPOINT | IFF_REMOTE)) == 0)
				return;
			n = msg->rip_nets;
			npackets++;
		}

		// Casey, RIP2
		n->n_family = AF_INET;
		n->n_tag = 0;
		n->n_dst = satosin(rt->rt_dst)->sin_addr.s_addr;
		// Kaohj
		//if(ripversion == 2) { 
		if (ifcp->send_mode != RIP_V1) {
			n->n_mask = satosin(rt->rt_netmask)->sin_addr.s_addr;
			// Added by Mason Yu 
			// Modified IP Address in RIP packet. The correct IP address is subnet address not route's dst address.
			n->n_dst &= n->n_mask;
		}	
		else/* RIP1 */
			n->n_mask = 0;
		n->n_nhop = 0;

		n->n_metric = htonl(rt->rt_metric);
		
		
		if(debug) {
			printf(" route: flags=%x, state=%x, timer=%d\n", rt->rt_flags, rt->rt_state, rt->rt_timer);
			printf(" dest=%s,", inet_ntoa(*(struct in_addr*)&n->n_dst));
			printf(" mask=%s,", inet_ntoa(*(struct in_addr*)&n->n_mask));
			printf(" nhop=%s,", inet_ntoa(*(struct in_addr*)&n->n_nhop));
			printf(" cost=%d\n", n->n_metric);
		}
		
		/* to next entry */
		n++;

	}
	
	if (doinghost) {
		doinghost = 0;
		base = nethash;
		goto again;
	}
	if (n != msg->rip_nets || (npackets == 0 && rtstate == 0)) {
		size = (char *)n - packet;
		TRACE_OUTPUT(ifp, dst, size);
		(*output)(sock, flags, dst, size);
	}

}
