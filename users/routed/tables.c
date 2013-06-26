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
 * From: @(#)tables.c	5.17 (Berkeley) 6/1/90
 * From: @(#)tables.c	8.1 (Berkeley) 6/5/93
 */
char tables_rcsid[] = 
  "$Id: tables.c,v 1.3 2009/08/26 07:26:20 bradhuang Exp $";


/*
 * Routing Table Management Daemon
 */

#include "defs.h"
#include <sys/ioctl.h>
#include <syslog.h>
#include <errno.h>
#include <search.h>

#ifndef DEBUG
#define	DEBUG	0
#endif

#define FIXLEN(s) { }

static int install = !DEBUG;		/* if 1 call kernel */
struct rthash nethash[ROUTEHASHSIZ];
struct rthash hosthash[ROUTEHASHSIZ];
extern struct cfg_interface *ifcpq;

/*
 * Lookup dst in the tables for an exact match.
 */
struct rt_entry *rtlookup(struct netinfo *n)
{
	register struct rt_entry *rt;
	register struct rthash *rh;
	register u_int hash;
	struct afhash h;
	int doinghost = 1;
	struct sockaddr dst;

	satosin(dst)->sin_addr.s_addr = n->n_dst;
	if (n->n_family >= af_max)
		return (0);
		
	// Modified by Mason Yu
	// RIP crash	
	//(*afswitch[n->n_family].af_hash)(n->n_dst, &h);
	(*afswitch[n->n_family].af_hash)(&(n->n_dst), &h);
	
	hash = h.afh_hosthash;
	rh = &hosthash[hash & ROUTEHASHMASK];
	
again:
	for (rt = rh->rt_forw; rt != (struct rt_entry *)rh; rt = rt->rt_forw) {
		if (rt->rt_hash != hash)
			continue;
		if (equal(&rt->rt_dst, &dst))
			return (rt);
	}
	if (doinghost) {
		doinghost = 0;
		hash = h.afh_nethash;
		rh = &nethash[hash & ROUTEHASHMASK];
		goto again;
	}
	return (0);
}

struct sockaddr wildcard;	/* zero valued cookie for wildcard searches */

/*
 * Find a route to dst as the kernel would.
 */

struct rt_entry *rtfind(struct sockaddr *dst)
{
	register struct rt_entry *rt;
	register struct rthash *rh;
	register u_int hash;
	struct afhash h;
	int af = dst->sa_family;
	int doinghost = 1, (*match)(struct sockaddr *,struct sockaddr *)=NULL;

	if (af >= af_max)
		return (0);
	(*afswitch[af].af_hash)(dst, &h);
	hash = h.afh_hosthash;
	rh = &hosthash[hash & ROUTEHASHMASK];

again:
	for (rt = rh->rt_forw; rt != (struct rt_entry *)rh; rt = rt->rt_forw) {
		if (rt->rt_hash != hash)
			continue;
		if (doinghost) {
			if (equal(&rt->rt_dst, dst))
				return (rt);
		} else {
			if (rt->rt_dst.sa_family == af &&
			    (*match)(&rt->rt_dst, dst))
				return (rt);
		}
	}
	if (doinghost) {
		doinghost = 0;
		hash = h.afh_nethash;
		rh = &nethash[hash & ROUTEHASHMASK];
		match = afswitch[af].af_netmatch;
		goto again;
	}
	return (0);
}

// Kaohj -- rip2 supported
struct rt_entry *rtfind2(struct sockaddr *dst, struct sockaddr *netmask, struct interface *fromIf)
{
	register struct rt_entry *rt;
	register struct rthash *rh;
	register u_int hash;
	struct afhash h;
	int af = dst->sa_family;
	int doinghost = 1, (*match)(struct sockaddr *,struct sockaddr *)=NULL;
	struct cfg_interface *ifcp;

	if (fromIf == 0)
		return (0);
	if (af >= af_max)
		return (0);
	ifcp = (struct cfg_interface *)if_cfg_lookup(fromIf->int_name);
	if (!ifcp)
		return(0);
	(*afswitch[af].af_hash)(dst, &h);
	hash = h.afh_hosthash;
	rh = &hosthash[hash & ROUTEHASHMASK];

again:
	for (rt = rh->rt_forw; rt != (struct rt_entry *)rh; rt = rt->rt_forw) {
		if (rt->rt_hash != hash)
			continue;
		if (doinghost) {
			if (equal(&rt->rt_dst, dst)) {
				//printf("found host route\n");
				return (rt);
			}
		} else {
			//if (ripversion >= 2) {
			if (ifcp->receive_mode >= 2) {
				u_long net1, net2, ip1, ip2, mask1, mask2;
				struct sockaddr_in *sip1=(struct sockaddr_in *)&rt->rt_dst;
				struct sockaddr_in *sip2=(struct sockaddr_in *)dst;
				struct sockaddr_in *snet1=(struct sockaddr_in *)&rt->rt_netmask;
				struct sockaddr_in *snet2=(struct sockaddr_in *)netmask;
				ip1 = sip1->sin_addr.s_addr;
				ip2 = sip2->sin_addr.s_addr;
				mask1 = snet1->sin_addr.s_addr;
				mask2 = snet2->sin_addr.s_addr;
				net1 = ip1 & mask1;
				net2 = ip2 & mask2;
				//printf("net1=%x, net2=%x\n", net1, net2);
				if (net1 == net2) {
					//printf("found subnet route\n");
					return (rt);
				}
			}
			else {
			if (rt->rt_dst.sa_family == af &&
			    (*match)(&rt->rt_dst, dst)) {
			    	//printf("found net rout\n");
				return (rt);
			}
			}
		}
	}
	if (doinghost) {
		doinghost = 0;
		hash = h.afh_nethash;
		rh = &nethash[hash & ROUTEHASHMASK];
		match = afswitch[af].af_netmatch;
		goto again;
	}
	//printf("not found\n");
	return (0);
}

// Added by Mason Yu
void checkRIPHashTable()
{
	struct rt_entry *rt;	
	struct rthash *rh;
	struct rthash *base = hosthash;
	int doinghost = 1, size;
	
again:
	for (rh = base; rh < &base[ROUTEHASHSIZ]; rh++)		
	for (rt = rh->rt_forw; rt != (struct rt_entry *)rh; rt = rt->rt_forw) {
		/*
		
		 * Don't resend the information on the network
		 * from which it was received (unless sending
		 * in response to a query).
		 */
		// Mason Yu
		//printf("rtadd: rt->rt_dst = %x\n", satosin(rt->rt_dst)->sin_addr.s_addr);
	
	}
	
	if (doinghost) {
		doinghost = 0;
		base = nethash;
		goto again;
	}
	return;
}



void rtadd(struct sockaddr *dst, struct sockaddr *gate, int metric, int state, struct sockaddr *netmask)
{
	struct afhash h;
	register struct rt_entry *rt;
	struct rthash *rh;
	int af = dst->sa_family, flags;
	u_int hash;
	char buf1[256], buf2[256];
	struct sockaddr v1mask;
	struct sockaddr *pmask;
	
	if (af >= af_max)
		return;
	(*afswitch[af].af_hash)(dst, &h);
	// Kaohj -- check for ripv2
	if (netmask) {
		pmask = netmask;
		if (*(long *)netmask == 0xffffffff)
			flags = RTF_HOST;
		else
			flags = RTF_SUBNET;
		//printf("flags=%x\n", flags);
	}
	else
	{
		pmask = &v1mask;
		((struct sockaddr_in *)pmask)->sin_addr.s_addr = inet_maskof(((struct sockaddr_in *)dst)->sin_addr.s_addr);
		flags = (*afswitch[af].af_rtflags)(dst);
	}
	
	/*
	 * Subnet flag isn't visible to kernel, move to state.	XXX
	 */
	FIXLEN(dst);
	FIXLEN(gate);
	
	// Mason Yu
	//printf("rtadd: ((struct sockaddr_in *)dst)->sin_addr.s_addr = %x\n", ((struct sockaddr_in *)dst)->sin_addr.s_addr);
	//printf("rtadd(2): dest=%x gate=%x netmask=%x \n", ((struct sockaddr_in *)dst)->sin_addr.s_addr, ((struct sockaddr_in *)gate)->sin_addr.s_addr, ((struct sockaddr_in *)netmask)->sin_addr.s_addr);
	
	if (flags & RTF_SUBNET) {
		state |= RTS_SUBNET;
		flags &= ~RTF_SUBNET;
	}
	if (flags & RTF_HOST) {
		hash = h.afh_hosthash;
		rh = &hosthash[hash & ROUTEHASHMASK];		
	} else {
		hash = h.afh_nethash;
		rh = &nethash[hash & ROUTEHASHMASK];		
	}
	rt = (struct rt_entry *)malloc(sizeof (*rt));
	if (rt == 0)
		return;
	rt->rt_hash = hash;
	rt->rt_dst = *dst;
	rt->rt_router = *gate;
	//rt->rt_netmask = *netmask;
	rt->rt_netmask = *pmask;
	rt->rt_timer = 0;
	rt->rt_flags = RTF_UP | flags;
	rt->rt_state = state | RTS_CHANGED;
	rt->rt_ifp = if_ifwithdstaddr(&rt->rt_dst);
	if (rt->rt_ifp == 0)
		rt->rt_ifp = if_ifwithnet(&rt->rt_router);
	if ((state & RTS_INTERFACE) == 0)
		rt->rt_flags |= RTF_GATEWAY;
	rt->rt_metric = metric;
	insque((struct qelem *)rt, (struct qelem *)rh);
	TRACE_ACTION("ADD", rt);
	/*
	 * If the ioctl fails because the gateway is unreachable
	 * from this host, discard the entry.  This should only
	 * occur because of an incorrect entry in /etc/gateways.
	 */	

	if ((rt->rt_state & (RTS_INTERNAL | RTS_EXTERNAL)) == 0 &&
	    rtioctl(ADD, &rt->rt_rt) < 0) {
		if (errno != EEXIST && gate->sa_family < af_max)
			syslog(LOG_ERR,
			"adding route to net/host %s through gateway %s: %m\n",
			   (*afswitch[dst->sa_family].af_format)(dst, buf1,
							     sizeof(buf1)),
			   (*afswitch[gate->sa_family].af_format)(gate, buf2,
							      sizeof(buf2)));
		//WiSOC do not display the error message to stdout
		;//perror("ADD ROUTE");
		if (errno == ENETUNREACH) {
			TRACE_ACTION("DELETE", rt);
			remque((struct qelem *)rt);
			free((char *)rt);
		}
	}
	
	// Added by Mason Yu
	// Check if we input route to RIP hash table
	//checkRIPHashTable();

}

void rtchange(struct rt_entry *rt, struct sockaddr *gate, short metric)
{
	int add = 0, delete = 0, newgateway = 0;
	struct rtuentry oldroute;

	FIXLEN(gate);
	FIXLEN(&(rt->rt_router));
	FIXLEN(&(rt->rt_dst));
	if (!equal(&rt->rt_router, gate)) {
		newgateway++;
		TRACE_ACTION("CHANGE FROM ", rt);
	} else if (metric != rt->rt_metric)
		TRACE_NEWMETRIC(rt, metric);
	if ((rt->rt_state & RTS_INTERNAL) == 0) {
		/*
		 * If changing to different router, we need to add
		 * new route and delete old one if in the kernel.
		 * If the router is the same, we need to delete
		 * the route if has become unreachable, or re-add
		 * it if it had been unreachable.
		 */
		if (newgateway) {
			add++;
			if (rt->rt_metric != HOPCNT_INFINITY)
				delete++;
		} else if (metric == HOPCNT_INFINITY)
			delete++;
		else if (rt->rt_metric == HOPCNT_INFINITY)
			add++;
	}
	/* Linux 1.3.12 and up */
	if (kernel_version >= 0x01030b) {
		if (add && delete && rt->rt_metric == metric)
			delete = 0;
	} else {
	/* Linux 1.2.x and 1.3.7 - 1.3.11 */
		if (add && delete)
			delete = 0;
	}

	if (delete)
		oldroute = rt->rt_rt;
	if ((rt->rt_state & RTS_INTERFACE) && delete) {
		rt->rt_state &= ~RTS_INTERFACE;
		rt->rt_flags |= RTF_GATEWAY;
		if (metric > rt->rt_metric && delete)
			syslog(LOG_ERR, "%s route to interface %s (timed out)",
			    add? "changing" : "deleting",
			    rt->rt_ifp ? rt->rt_ifp->int_name : "?");
	}
	if (add) {
		rt->rt_router = *gate;
		rt->rt_ifp = if_ifwithdstaddr(&rt->rt_router);
		if (rt->rt_ifp == 0)
			rt->rt_ifp = if_ifwithnet(&rt->rt_router);
	}
	rt->rt_metric = metric;
	rt->rt_state |= RTS_CHANGED;
	if (newgateway)
		TRACE_ACTION("CHANGE TO   ", rt);
	if (add && rtioctl(ADD, &rt->rt_rt) < 0)
		perror("ADD ROUTE");
	if (delete && rtioctl(DELETE, &oldroute) < 0)
		perror("DELETE ROUTE");
}

void rtdelete(struct rt_entry *rt)
{

	TRACE_ACTION("DELETE", rt);
	FIXLEN(&(rt->rt_router));
	FIXLEN(&(rt->rt_dst));
	if (rt->rt_metric < HOPCNT_INFINITY) {
	    if ((rt->rt_state & (RTS_INTERFACE|RTS_INTERNAL)) == RTS_INTERFACE)
		syslog(LOG_ERR,
		    "deleting route to interface %s? (timed out?)",
		    rt->rt_ifp->int_name);
	    if ((rt->rt_state & (RTS_INTERNAL | RTS_EXTERNAL)) == 0 &&
					    rtioctl(DELETE, &rt->rt_rt) < 0)
		    ;//perror("rtdelete");
	}	
	remque((struct qelem *)rt);
	free((char *)rt);
}
#if 0
void rtdeleteall(int sig)
{
	register struct rthash *rh;
	register struct rt_entry *rt;
	struct rthash *base = hosthash;
	int doinghost = 1, i;

again:
	for (i = 0; i < ROUTEHASHSIZ; i++) {
		rh = &base[i];
		rt = rh->rt_forw;
		for (; rt != (struct rt_entry *)rh; rt = rt->rt_forw) {
			if (rt->rt_state & RTS_INTERFACE ||
			    rt->rt_metric >= HOPCNT_INFINITY)
				continue;
			TRACE_ACTION("DELETE", rt);
			if ((rt->rt_state & (RTS_INTERNAL|RTS_EXTERNAL)) == 0 &&
			    rtioctl(DELETE, &rt->rt_rt) < 0)
				perror("rtdeleteall");
		}
	}
	if (doinghost) {
		doinghost = 0;
		base = nethash;
		goto again;
	}
	exit(sig);
}
#endif
void rtdeleteall(int sig)
{	register struct rthash *rh;	
	register struct rt_entry *rt;	
	struct rthash *base = hosthash;	
	int doinghost = 1, i, c;	
	struct in_addr dst;
	struct in_addr gw;
	struct in_addr dest;	
	struct in_addr mask;	
	char tmpBuf[200];
	char gwip[30],subnet[30],dstIp[30];	
	char *tmpStr;
	again:	
		for (i = 0; i < ROUTEHASHSIZ; i++) {		
			rh = &base[i];		
			rt = rh->rt_forw;		
			for (; rt != (struct rt_entry *)rh; rt = rt->rt_forw) {			
				//if (rt->rt_state & RTS_INTERFACE || rt->rt_metric >= HOPCNT_INFINITY) //WiSOC modify  for customize 			
				if (rt->rt_metric >= HOPCNT_INFINITY)				
					continue;			
				TRACE_ACTION("DELETE", rt);			
				if ((rt->rt_state & (RTS_INTERNAL|RTS_EXTERNAL|RTS_INTERFACE)) == 0 &&
					rtioctl(DELETE, &rt->rt_rt) < 0)				
					perror("rtdeleteall");			
				if (rt->rt_state ==0x400000)					
					if(rtioctl(DELETE, &rt->rt_rt)<0)					
						perror("rtdeleteerror");		
					}	
			}	
		if (doinghost) {		
			doinghost = 0;		
			base = nethash;		
			goto again;	
			}		
		base = nethash;	//RTK WiSOC add for deleting the route entry that routed created	
		for (i = 0; i < ROUTEHASHSIZ; i++) {		
			rh = &base[i];		
			rt = rh->rt_forw;		
			for (; rt != (struct rt_entry *)rh; rt = rt->rt_forw) {							
				gw = satosin(rt->rt_router)->sin_addr;				
				tmpStr=inet_ntoa(gw);				
				strcpy(gwip, tmpStr);				
				//printf("gw=%s\n", gwip);				
				dest = satosin(rt->rt_dst)->sin_addr;				
				tmpStr=inet_ntoa(dest);				
				strcpy(dstIp, tmpStr);				
				//printf("dest ip=%s\n", dstIp);				
				mask = satosin(rt->rt_netmask)->sin_addr;				
				tmpStr=inet_ntoa(mask);				
				strcpy(subnet, tmpStr);				
				//printf("subnet=%s\n", subnet);				
				//printf("entry: dest=%08X, mask=%08X, gw=%08x, metric=%d\n", dest.s_addr, mask.s_addr, gw.s_addr, rt->rt_ifmetric);				
				if(gw.s_addr != 0x0 && rt->rt_metric == 1){					
					sprintf(tmpBuf,"route del -net %s netmask %s gw %s", dstIp, subnet, gwip); 					
					//printf("cmd=%s\n", tmpBuf);				
					if(system(tmpBuf) != 0)
						;		
					}	
				}	
			}		
		exit(sig);
}

/*
 * If we have an interface to the wide, wide world,
 * add an entry for an Internet default route (wildcard) to the internal
 * tables and advertise it.  This route is not added to the kernel routes,
 * but this entry prevents us from listening to other people's defaults
 * and installing them in the kernel here.
 */

void rtdefault(void)
{

	rtadd((struct sockaddr *)&inet_default, 
		(struct sockaddr *)&inet_default, 1,
		RTS_CHANGED | RTS_PASSIVE | RTS_INTERNAL,
		(struct sockaddr *)&inet_default);
}

void rtinit(void)
{
	register struct rthash *rh;
	int i;

	for (i = 0; i < ROUTEHASHSIZ; i++) {
		rh = &nethash[i];
		rh->rt_forw = rh->rt_back = (struct rt_entry *)rh;
	}
	for (i = 0; i < ROUTEHASHSIZ; i++) {
		rh = &hosthash[i];
		rh->rt_forw = rh->rt_back = (struct rt_entry *)rh;
	}
}

int rtioctl(int action, struct rtuentry *ort)
{
	struct rtentry rt;
	unsigned int netmask;
	unsigned int dst;
	/*WiSOC add for check redundant route entry */
	unsigned long dest_addr=0;
	unsigned long mask_addr=0;
	unsigned long gate_addr=0;
	int isredundant=0;
	struct sockaddr_in *dstAddr, *gateAddr, *maskAddr;

	if (install == 0)
		return (errno = 0);

#undef rt_flags
#undef rt_ifp
#undef rt_metric
#undef rt_dst
	/* WiSOC do not add route that is exist for lan and wan*/
	/*WiSOC disabled, since it is redundant action for route setting, it is not necessary*/

	if(action==ADD){
		dstAddr = (struct sockaddr_in *)&ort->rtu_dst;
		gateAddr = (struct sockaddr_in *)&ort->rtu_router;
		maskAddr= (struct sockaddr_in *)&ort->rtu_netmask;
		dest_addr = (unsigned long)((dstAddr)->sin_addr.s_addr);
		mask_addr = (unsigned long)((maskAddr)->sin_addr.s_addr);
		gate_addr = (unsigned long)((gateAddr)->sin_addr.s_addr);

		if(dest_addr != 0 && mask_addr != 0 && gate_addr != 0){
			isredundant=check_isredundant(dest_addr, mask_addr, gate_addr); 
			if(isredundant==1){
				//printf("It is redundant, will not add again\n");
				return 0;
			}
		}
	}
	rt.rt_flags = (ort->rtu_flags & (RTF_UP|RTF_GATEWAY|RTF_HOST));
	rt.rt_metric = ort->rtu_metric;
	rt.rt_dev = NULL;
	rt.rt_dst     = *(struct sockaddr *)&ort->rtu_dst;
	dst = ((struct sockaddr_in *)&rt.rt_dst)->sin_addr.s_addr;
	rt.rt_gateway = *(struct sockaddr *)&ort->rtu_router;
	if (rt.rt_flags & RTF_HOST)
		netmask = 0xffffffff;
	else {
		// Modified by Mason Yu
		//netmask = inet_maskof(dst);
		rt.rt_genmask   = *(struct sockaddr *)&ort->rtu_netmask;
		netmask = ((struct sockaddr_in *)&rt.rt_genmask)->sin_addr.s_addr;
	}
		
	((struct sockaddr_in *)&rt.rt_genmask)->sin_family = AF_INET;
	((struct sockaddr_in *)&rt.rt_genmask)->sin_addr.s_addr = netmask;
	
	if (traceactions) {
		fprintf(ftrace, "rtioctl %s %08lx/%08lx\n",
			action == ADD ? "ADD" : "DEL",
			(unsigned long int)ntohl(dst),
			(unsigned long int)ntohl(netmask));
		fflush(ftrace);
	}
	
	switch (action) {

	case ADD:
		return (ioctl(sock, SIOCADDRT, (char *)&rt));

	case DELETE:
		return (ioctl(sock, SIOCDELRT, (char *)&rt));

	default:
		return (-1);
	}
}
