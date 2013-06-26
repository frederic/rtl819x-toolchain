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
 * From: @(#)startup.c	5.19 (Berkeley) 2/28/91
 * From: @(#)startup.c	8.1 (Berkeley) 6/5/93
 */
char startup_rcsid[] = 
  "$Id: startup.c,v 1.5 2009/08/28 10:57:24 bradhuang Exp $";


/*
 * Routing Table Management Daemon
 */

#include "defs.h"
#include <sys/ioctl.h>
/* #include <net/if.h> (redundant with defs.h) */
#include <syslog.h>
#include <errno.h>
#include "pathnames.h"

// Added by mason Yu
#include <stdarg.h>
#include <sys/types.h>
#if 0//WiSOC unused
//ql
#include <config/autoconf.h>
#endif
struct	interface *ifnet;
struct	interface **ifnext = &ifnet;
int	lookforinterfaces = 1;
int	foundloopback;			/* valid flag for loopaddr */
struct	sockaddr loopaddr;		/* our address on loopback */

static int externalinterfaces = 0;	/* # of remote and local interfaces */

void add_ptopt_localrt(struct interface *);
int getnetorhostname(char *, char *, struct sockaddr_in *);
int gethostnameornumber(char *, struct sockaddr_in *);

void quit(char *s)
{
	int sverrno = errno;

	(void) fprintf(stderr, "route: ");
	if (s)
		(void) fprintf(stderr, "%s: ", s);
	(void) fprintf(stderr, "%s\n", strerror(sverrno));
	exit(1);
}

#define ROUNDUP(a) \
	((a) > 0 ? (1 + (((a) - 1) | (sizeof(long) - 1))) : sizeof(long))
#define ADVANCE(x, n) (x += ROUNDUP((n)->sa_len))

/*
 * Find the network interfaces which have configured themselves.
 * If the interface is present but not yet up (for example an
 * ARPANET IMP), set the lookforinterfaces flag so we'll
 * come back later and look again.
 */

void ifinit(void)
{
	struct interface ifs, *ifp;
	int s;
	char buf[BUFSIZ], *cp, *cplim;
        struct ifconf ifc;
        struct ifreq ifreq, *ifr;
        struct sockaddr_in *sin;
	u_long i;

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		syslog(LOG_ERR, "socket: %m");
		close(s);
                return;
	}
        ifc.ifc_len = sizeof (buf);
        ifc.ifc_buf = buf;
        if (ioctl(s, SIOCGIFCONF, (char *)&ifc) < 0) {
                syslog(LOG_ERR, "ioctl (get interface configuration)");
		close(s);
                return;
        }
        ifr = ifc.ifc_req;
	lookforinterfaces = 0;
	cplim = buf + ifc.ifc_len; /*skip over if's with big ifr_addr's */
	for (cp = buf; cp < cplim;
			cp += sizeof (ifr->ifr_name) + sizeof(ifr->ifr_ifru)) {
		ifr = (struct ifreq *)cp;
		bzero((char *)&ifs, sizeof(ifs));
		ifs.int_addr = ifr->ifr_addr;
		ifreq = *ifr;
                if (ioctl(s, SIOCGIFFLAGS, (char *)&ifreq) < 0) {
                        syslog(LOG_ERR, "%s: ioctl (get interface flags)",
			    ifr->ifr_name);
                        continue;
                }
		ifs.int_flags =
		    ifreq.ifr_flags | IFF_INTERFACE;
		if ((ifs.int_flags & IFF_UP) == 0 ||
		    ifr->ifr_addr.sa_family == AF_UNSPEC) {
			lookforinterfaces = 1;
			continue;
		}
		/* argh, this'll have to change sometime */
		if (ifs.int_addr.sa_family != AF_INET)
			continue;
                if (ifs.int_flags & IFF_POINTOPOINT) {
                        if (ioctl(s, SIOCGIFDSTADDR, (char *)&ifreq) < 0) {
                                syslog(LOG_ERR, "%s: ioctl (get dstaddr)",
				    ifr->ifr_name);
                                continue;
			}
			if (ifr->ifr_addr.sa_family == AF_UNSPEC) {
				lookforinterfaces = 1;
				continue;
			}
			ifs.int_dstaddr = ifreq.ifr_dstaddr;
		}
		/*
		 * already known to us?
		 * This allows multiple point-to-point links
		 * to share a source address (possibly with one
		 * other link), but assumes that there will not be
		 * multiple links with the same destination address.
		 */
		if (ifs.int_flags & IFF_POINTOPOINT) {
			if (if_ifwithdstaddr(&ifs.int_dstaddr))
				continue;
		} else if (if_ifwithaddr(&ifs.int_addr))
			continue;
		if (ifs.int_flags & IFF_LOOPBACK) {
			ifs.int_flags |= IFF_PASSIVE;
			foundloopback = 1;
			loopaddr = ifs.int_addr;
			for (ifp = ifnet; ifp; ifp = ifp->int_next)
			    if (ifp->int_flags & IFF_POINTOPOINT)
				add_ptopt_localrt(ifp);
		}
                if (ifs.int_flags & IFF_BROADCAST) {
                        if (ioctl(s, SIOCGIFBRDADDR, (char *)&ifreq) < 0) {
                                syslog(LOG_ERR, "%s: ioctl (get broadaddr)",
				    ifr->ifr_name);
                                continue;
                        }
			ifs.int_broadaddr = ifreq.ifr_broadaddr;
		}
#ifdef SIOCGIFMETRIC
		if (ioctl(s, SIOCGIFMETRIC, (char *)&ifreq) < 0) {
			syslog(LOG_ERR, "%s: ioctl (get metric)",
			    ifr->ifr_name);
			ifs.int_metric = 0;
		} else
			ifs.int_metric = ifreq.ifr_metric;
#else
		ifs.int_metric = 0;
#endif
		/*
		 * Use a minimum metric of one;
		 * treat the interface metric (default 0)
		 * as an increment to the hop count of one.
		 */
		ifs.int_metric++;
		if (ioctl(s, SIOCGIFNETMASK, (char *)&ifreq) < 0) {
			syslog(LOG_ERR, "%s: ioctl (get netmask)",
			    ifr->ifr_name);
			continue;
		}
		sin = (struct sockaddr_in *)&ifreq.ifr_addr;
		ifs.int_subnetmask = ntohl(sin->sin_addr.s_addr);
		sin = (struct sockaddr_in *)&ifs.int_addr;
		i = ntohl(sin->sin_addr.s_addr);
		if (IN_CLASSA(i))
			ifs.int_netmask = IN_CLASSA_NET;
		else if (IN_CLASSB(i))
			ifs.int_netmask = IN_CLASSB_NET;
		else
			ifs.int_netmask = IN_CLASSC_NET;
		ifs.int_net = i & ifs.int_netmask;
		ifs.int_subnet = i & ifs.int_subnetmask;
		if (ifs.int_subnetmask != ifs.int_netmask)
			ifs.int_flags |= IFF_SUBNET;
		ifp = (struct interface *)malloc(sizeof (struct interface));
		if (ifp == 0) {
			printf("routed: out of memory\n");
			break;
		}
		*ifp = ifs;
		/*
		 * Count the # of directly connected networks
		 * and point to point links which aren't looped
		 * back to ourself.  This is used below to
		 * decide if we should be a routing ``supplier''.
		 */
		if ((ifs.int_flags & IFF_LOOPBACK) == 0 &&
		    ((ifs.int_flags & IFF_POINTOPOINT) == 0 ||
		    if_ifwithaddr(&ifs.int_dstaddr) == 0))
			externalinterfaces++;
		/*
		 * If we have a point-to-point link, we want to act
		 * as a supplier even if it's our only interface,
		 * as that's the only way our peer on the other end
		 * can tell that the link is up.
		 */
		if ((ifs.int_flags & IFF_POINTOPOINT) && supplier < 0)
			supplier = 1;
		ifp->int_name = malloc(strlen(ifr->ifr_name) + 1);
		if (ifp->int_name == 0) {
			fprintf(stderr, "routed: ifinit: out of memory\n");
			syslog(LOG_ERR, "routed: ifinit: out of memory\n");
			close(s);
			return;
		}
		strcpy(ifp->int_name, ifr->ifr_name);
		*ifnext = ifp;
		ifnext = &ifp->int_next;
		traceinit(ifp);
		addrouteforif(ifp);
	}
	if (externalinterfaces > 1 && supplier < 0)
		supplier = 1;
	close(s);
}

/*
 * Add route for interface if not currently installed.
 * Create route to other end if a point-to-point link,
 * otherwise a route to this (sub)network.
 * INTERNET SPECIFIC.
 */

void addrouteforif(struct interface *ifp)
{
	struct sockaddr_in net;
	struct sockaddr_in netmask;
	struct sockaddr *dst;
	int state;
	struct rt_entry *rt;

	if (ifp->int_flags & IFF_POINTOPOINT) {
		dst = &ifp->int_dstaddr;
		netmask.sin_addr.s_addr = 0xffffffff;
	}
	else {
		memset(&net, 0, sizeof (net));
		net.sin_family = AF_INET;
		net.sin_addr = inet_makeaddr(ifp->int_subnet, INADDR_ANY);
		dst = (struct sockaddr *)&net;
		netmask.sin_addr.s_addr = ifp->int_subnetmask;
	}
	rt = rtfind(dst);
	if (rt &&
	    (rt->rt_state & (RTS_INTERFACE | RTS_INTERNAL)) == RTS_INTERFACE){
		return;
		}
	if (rt){
		rtdelete(rt);
		}
	/*
	 * If interface on subnetted network,
	 * install route to network as well.
	 * This is meant for external viewers.
	 */
	if ((ifp->int_flags & (IFF_SUBNET|IFF_POINTOPOINT)) == IFF_SUBNET) {
		struct in_addr subnet;

		subnet = net.sin_addr;
		net.sin_addr = inet_makeaddr(ifp->int_net, INADDR_ANY);
		rt = rtfind(dst);
		if (rt == 0) {
			rtadd(dst, &ifp->int_addr, ifp->int_metric,
			    ((ifp->int_flags & (IFF_INTERFACE|IFF_REMOTE)) |
			    RTS_PASSIVE | RTS_INTERNAL | RTS_SUBNET),
			    &netmask);
		}
		else if ((rt->rt_state & (RTS_INTERNAL|RTS_SUBNET)) == 
		    (RTS_INTERNAL|RTS_SUBNET) &&
		    ifp->int_metric < rt->rt_metric)
			rtchange(rt, &rt->rt_router, ifp->int_metric);
		net.sin_addr = subnet;
	}
	if (ifp->int_transitions++ > 0)
		syslog(LOG_ERR, "re-installing interface %s", ifp->int_name);
	state = ifp->int_flags &
	    (IFF_INTERFACE | IFF_PASSIVE | IFF_REMOTE | IFF_SUBNET);
	if (ifp->int_flags & IFF_POINTOPOINT &&
	    (ntohl(((struct sockaddr_in *)&ifp->int_dstaddr)->sin_addr.s_addr) &
	    ifp->int_netmask) != ifp->int_net){
		state &= ~RTS_SUBNET;
		}
	if (ifp->int_flags & IFF_LOOPBACK){
		state |= RTS_EXTERNAL;
		}
	rtadd(dst, &ifp->int_addr, ifp->int_metric, state, &netmask);
	if (ifp->int_flags & IFF_POINTOPOINT && foundloopback){
		add_ptopt_localrt(ifp);
		}
}

/*
 * Add route to local end of point-to-point using loopback.
 * If a route to this network is being sent to neighbors on other nets,
 * mark this route as subnet so we don't have to propagate it too.
 */

void add_ptopt_localrt(struct interface *ifp)
{
	struct rt_entry *rt;
	struct sockaddr *dst;
	struct sockaddr_in net;
	struct sockaddr_in netmask;
	int state;

	state = RTS_INTERFACE | RTS_PASSIVE;

	/* look for route to logical network */
	memset(&net, 0, sizeof (net));
	net.sin_family = AF_INET;
	net.sin_addr = inet_makeaddr(ifp->int_net, INADDR_ANY);
	dst = (struct sockaddr *)&net;
	rt = rtfind(dst);
	if (rt && rt->rt_state & RTS_INTERNAL)
		state |= RTS_SUBNET;

	dst = &ifp->int_addr;
	if ((rt = rtfind(dst))!=NULL) {
		if (rt && rt->rt_state & RTS_INTERFACE)
			return;
		rtdelete(rt);
	}
	netmask.sin_addr.s_addr = ifp->int_subnetmask;
	rtadd(dst, &loopaddr, 1, state, &netmask);
}

/*
 * As a concession to the ARPANET we read a list of gateways
 * from /etc/gateways and add them to our tables.  This file
 * exists at each ARPANET gateway and indicates a set of ``remote''
 * gateways (i.e. a gateway which we can't immediately determine
 * if it's present or not as we can do for those directly connected
 * at the hardware level).  If a gateway is marked ``passive''
 * in the file, then we assume it doesn't have a routing process
 * of our design and simply assume it's always present.  Those
 * not marked passive are treated as if they were directly
 * connected -- they're added into the interface list so we'll
 * send them routing updates.
 *
 * PASSIVE ENTRIES AREN'T NEEDED OR USED ON GATEWAYS RUNNING EGP.
 */

void gwkludge(void)
{
	struct sockaddr_in dst, gate, netmask;
	FILE *fp;
	char *type, *gname, *qual, buf[BUFSIZ];
	struct interface *ifp;
	int metric, n;
	struct rt_entry route;
	// Mason Yu
	char dname[20];
	

	fp = fopen(_PATH_GATEWAYS, "r");
	if (fp == NULL){		
		return;
	}
	qual = buf;
	//dname = buf + 64;
	gname = buf + ((BUFSIZ - 64) / 3);
	type = buf + (((BUFSIZ - 64) * 2) / 3);
	memset(&dst, 0, sizeof (dst));
	memset(&gate, 0, sizeof (gate));
	memset(&route, 0, sizeof(route));
/* format: {net | host} XX gateway XX metric DD [passive | external]\n */
#define	readentry(fp) \
	fscanf((fp), "%s %s gateway %s metric %d %s\n", \
		type, dname, gname, &metric, qual)
	for (;;) {
		if ((n = readentry(fp)) == EOF)
			break;
		/*
		 *	Lusertrap. Vendors should ship the line
		 *
		 *	CONFIGME CONFIGME gateway CONFIGME metric 1
		 *
		 */
		
		// Mason Yu
		//printf("type = %s dname = %s gname = %s metric = %d qual = %s\n", type, dname, gname, metric, qual);
		
		if (strcmp(type,"CONFIGME")==0)
		{
			fprintf(stderr,"Not starting gated. Please configure first.\n");
			exit(1);
		}
		
		// Modified by Mason Yu
		#if 0
		if (!getnetorhostname(type, dname, &dst))
			continue;
		if (!gethostnameornumber(gname, &gate))
			continue;
		#endif
		{
			struct in_addr dname_ip, gname_ip;
			
			inet_aton(dname, &dname_ip);
			inet_aton(gname, &gname_ip);
			dst.sin_addr.s_addr = dname_ip.s_addr;
			dst.sin_family = AF_INET;
			gate.sin_addr.s_addr = gname_ip.s_addr;
			gate.sin_family = AF_INET;		
		}
	
		if (metric == 0)			/* XXX */
			metric = 1;
		if (strcmp(qual, "passive") == 0) {
			/*
			 * Passive entries aren't placed in our tables,
			 * only the kernel's, so we don't copy all of the
			 * external routing information within a net.
			 * Internal machines should use the default
			 * route to a suitable gateway (like us).
			 */
			route.rt_dst = *(struct sockaddr *) &dst;
			route.rt_router = *(struct sockaddr *) &gate;
			route.rt_flags = RTF_UP;
			if (strcmp(type, "host") == 0)
				route.rt_flags |= RTF_HOST;
			if (metric)
				route.rt_flags |= RTF_GATEWAY;
			(void) rtioctl(ADD, &route.rt_rt);
			continue;
		}
		if (strcmp(qual, "external") == 0) {
			/*
			 * Entries marked external are handled
			 * by other means, e.g. EGP,
			 * and are placed in our tables only
			 * to prevent overriding them
			 * with something else.
			 */
			netmask.sin_addr.s_addr = inet_maskof(gate.sin_addr.s_addr);
			rtadd((struct sockaddr *)&dst,
			    (struct sockaddr *)&gate, metric,
			    RTS_EXTERNAL|RTS_PASSIVE, &netmask);
			continue;
		}
		/* assume no duplicate entries */
		externalinterfaces++;
		ifp = (struct interface *)malloc(sizeof (*ifp));
		memset(ifp, 0, sizeof (*ifp));		
		ifp->int_flags = IFF_REMOTE;		
		/* can't identify broadcast capability */
		ifp->int_net = inet_netof_subnet(dst.sin_addr);
		if (strcmp(type, "host") == 0) {
			ifp->int_flags |= IFF_POINTOPOINT;
			ifp->int_dstaddr = *((struct sockaddr *)&dst);
		}
		ifp->int_addr = *((struct sockaddr *)&gate);
		ifp->int_metric = metric;
		ifp->int_next = ifnet;
		ifnet = ifp;		
		addrouteforif(ifp);
	}
	fclose(fp);
}

int getnetorhostname(char *type, char *name, struct sockaddr_in *sin)
{
#ifndef EMBED

	if (strcmp(type, "net") == 0) {
		struct netent *np = getnetbyname(name);
		int n;

		if (np == 0)
			n = inet_network(name);
		else {
			if (np->n_addrtype != AF_INET)
				return (0);
			n = np->n_net;
			/*
			 * getnetbyname returns right-adjusted value.
			 */
			if (n < 128)
				n <<= IN_CLASSA_NSHIFT;
			else if (n < 65536)
				n <<= IN_CLASSB_NSHIFT;
			else
				n <<= IN_CLASSC_NSHIFT;
		}
		sin->sin_family = AF_INET;
		sin->sin_addr = inet_makeaddr(n, INADDR_ANY);
		return (1);
	}
	if (strcmp(type, "host") == 0)
		return (gethostnameornumber(name, sin));
#endif
	return (0);
}

int gethostnameornumber(char *name, struct sockaddr_in *sin)
{
	struct hostent *hp;

	if (inet_aton(name, &sin->sin_addr) == 0) {
		hp = gethostbyname(name);
		if (hp == 0)
			return (0);
		memcpy(&sin->sin_addr, hp->h_addr, sizeof(sin->sin_addr));
		sin->sin_family = hp->h_addrtype;
	} else
		sin->sin_family = AF_INET;
	return (1);
}


///////////////////////////////////////////////

struct cfg_interface *ifcpq = NULL;

/*
 */
struct cfg_interface *if_cfg_lookup(char *if_name)
{
	struct cfg_interface *ifcp;
	
	for(ifcp = ifcpq; ifcp; ifcp = ifcp->if_next) {
		if(!strcmp(ifcp->if_name, if_name)) {
			return(ifcp);	/* found */
		}
	}
	return NULL;	/* not found */
}

int if_cfg_status(void)
{
struct cfg_interface *ifcp;
struct ifreq ifreq;
int s;
int has_send_req=0;
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return 0;

	for(ifcp = ifcpq; ifcp; ifcp = ifcp->if_next) {
		strcpy(ifreq.ifr_name, ifcp->if_name);
		if (ioctl(s, SIOCGIFFLAGS, (char *)&ifreq) == 0) {
			/* from down to up */
			if( (ifcp->if_flags & IFF_UP)==0 && (ifreq.ifr_flags&IFF_UP)) {
				rtinit();
				ifinit();
				rip_input_init();
				if(has_send_req==0){//WiSOC send 1 request in start only
				rip_request_send();
					has_send_req=1;
				}

			}
			/* from up to down */
			if( (ifcp->if_flags & IFF_UP) && (ifreq.ifr_flags&IFF_UP)==0) {
				clean_if_rt();
			}
			ifcp->if_flags = ifreq.ifr_flags;
		}
	}
	
	close(s);
	return 0;
}

// Added by Kaohj
int do_cmd(const char *filename, char *argv [], int dowait)
{
	pid_t pid, wpid;
	int stat, st;
	
	if((pid = vfork()) == 0) {
		/* the child */
		char *env[3];
		
		signal(SIGINT, SIG_IGN);
		argv[0] = (char *)filename;
		env[0] = "PATH=/bin:/usr/bin:/etc:/sbin:/usr/sbin";
		env[1] = NULL;

		execve(filename, argv, env);

		printf("exec %s failed\n", filename);
		_exit(2);
	} else if(pid > 0) {
		if (!dowait)
			stat = 0;
		else {
			/* parent, wait till rc process dies before spawning */
			while ((wpid = wait(&stat)) != pid)
				if (wpid == -1 && errno == ECHILD) { /* see wait(2) manpage */
					stat = 0;
					break;
				}
		}
	} else if(pid < 0) {
		printf("fork of %s failed\n", filename);
		stat = -1;
	}
	//st = WEXITSTATUS(stat);
	return st;
}

void va_cmd(const char *cmd, int num, int dowait, ...)
{
	va_list ap;
	int k;
	char *s;
	char *argv[19];
	
	//TRACE(STA_SCRIPT, "%s ", cmd);
	va_start(ap, dowait);
	
	for (k=0; k<num; k++)
	{
		s = va_arg(ap, char *);
		argv[k+1] = s;
		//TRACE(STA_SCRIPT|STA_NOTAG, "%s ", s);
	}
	
	//TRACE(STA_SCRIPT|STA_NOTAG, "\n");
	argv[k+1] = NULL;
	do_cmd(cmd, argv, dowait);
	va_end(ap);
}


// Added by Mason Yu for send route error
unsigned long *g_wanip[8];
int WANIF_NUMBER = 1;
unsigned long RIPIP[9];
int RIPIF_NUMBER = 0;

/*
 */
// Modified by Mason Yu for bind interface
//int if_cfg_add(char *if_name)
#ifndef CONFIG_BOA_WEB_E8B_CH
int if_cfg_add(char *if_name, int receive_mode, int send_mode)
#else
int if_cfg_add(char *if_name, int receive_mode, int send_mode, int operation)
#endif
{
struct cfg_interface *ifcp;
int skfd;
struct ifreq ifr, ifreq;
struct sockaddr_in *addr, *sin;
unsigned long i, int_netmask, int_net;
char wanip[16];
struct in_addr vcip;
struct sockaddr_in dst, gate;
struct interface *ifp;
int j;

	// Mason Yu 123	
	if(if_cfg_lookup(if_name)) {
		printf("RIP: The if_name is on the List.\n");
		return -1;		
	}
	
	ifcp = malloc(sizeof(struct cfg_interface));
	if(!ifcp)
		return -1;
	strcpy(ifcp->if_name, if_name);
	
	// Added by Mason Yu for bind interface
	ifcp->receive_mode = receive_mode;
	ifcp->send_mode = send_mode;
#ifdef CONFIG_BOA_WEB_E8B_CH
	ifcp->operation = operation;
#endif

	ifcp->if_next = ifcpq;
	ifcpq = ifcp;
		
	
	// Added by Mason Yu for send route error, Start
	// Add interfaces which configured be supported RIP into ifnet link list
	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	strcpy(ifr.ifr_name, if_name);
	
	if (ioctl(skfd, SIOCGIFADDR, &ifr) == 0) {
		addr = ((struct sockaddr_in *)&ifr.ifr_addr);		
		
		// Save RIP interface IP Address
		RIPIP[RIPIF_NUMBER] = addr->sin_addr.s_addr;
		RIPIF_NUMBER++;			
		
		
	}
	else
	{
		printf("The Interface ip address not found !\n");
		
	}
	close( skfd );

// Commented by Mason Yu
// The following programs will cause the system crash.
#if 0	
	if (strncmp("vc", if_name, 2) == 0 ){	
		
		// Get Net(such as : 192.168.1.0)		
		i = ntohl(addr->sin_addr.s_addr);
		if (IN_CLASSA(i))
			int_netmask = IN_CLASSA_NET;
		else if (IN_CLASSB(i))
			int_netmask = IN_CLASSB_NET;
		else
			int_netmask = IN_CLASSC_NET;
		int_net = i & int_netmask;		
		
		vcip.s_addr = int_net;
		
		strncpy(wanip, inet_ntoa(vcip), 16);
		wanip[15] = '\0';		

		g_wanip[WANIF_NUMBER] = &int_net;
		WANIF_NUMBER++;
	}	
	
	
	
	if (strncmp("br", if_name, 2) == 0 ){
		for (j=1; j<WANIF_NUMBER; j++) {		
			dst.sin_addr.s_addr = *g_wanip[j];
			dst.sin_family = AF_INET;
			gate.sin_addr.s_addr = addr->sin_addr.s_addr;
			gate.sin_family = AF_INET;	
			//printf("dst.sin_addr.s_addr = %x\n", dst.sin_addr.s_addr);
			//printf("gate.sin_addr.s_addr = %x\n", gate.sin_addr.s_addr);
			
			/* assume no duplicate entries */
			externalinterfaces++;
			ifp = (struct interface *)malloc(sizeof (*ifp));
			memset(ifp, 0, sizeof (*ifp));			
			ifp->int_flags = IFF_REMOTE;
			
			/* can't identify broadcast capability */
			ifp->int_net = inet_netof_subnet(dst.sin_addr);		
			ifp->int_addr = *((struct sockaddr *)&gate);
			ifp->int_metric = 1;
			ifp->int_next = ifnet;
			ifnet = ifp;			
			addrouteforif(ifp);		
		
		
			// Delete error default route
			va_cmd("/bin/route", 3, 1, "del", "default", "br0");
		}
		
	}
#endif	
	// Added by Mason Yu for send route error, End
	
	return 0;
}

/*
 */
int if_cfg_del(char *if_name)
{
struct cfg_interface **q, *p;
  
	/* Remove the entry matching timeout and remove it from the list. */
	for (q = &ifcpq; (p = *q); q = &p->if_next) {
		if (!strcmp(p->if_name, if_name)) {
			*q = p->if_next;
			free(p);
			return 0;
		}
	}
	return -1;
}

/*
 */
int if_cfg_flush(void)
{
struct cfg_interface *p;
  
	/* Remove all entries */
	p = ifcpq;
	while(p) {
		ifcpq = p->if_next;
		free(p);
		p = ifcpq;
	}
	return 0;
}

/*
 */
int if_cfg_write(FILE *fp)
{
struct cfg_interface *ifcp;
  
  	if(!fp)
  		return -1;
  	ifcp = ifcpq;
	while(ifcp) {
		fprintf(fp, "network %s\n", ifcp->if_name);
		ifcp = ifcp->if_next;
	}
	return 0;
}


struct cfg_interface *ifchkpq = NULL;

/*
 */
struct cfg_interface *if_chk_lookup(char *if_name)
{
	struct cfg_interface *ifcp;
	int s;
	
	for(ifcp = ifchkpq; ifcp; ifcp = ifcp->if_next) {
		if(!strcmp(ifcp->if_name, if_name)) {
			return(ifcp);	/* found */
		}
	}
	return NULL;	/* not found */
}

/*
 */
// Modified by Mason Yu for bind interface
//int if_chk_add(char *if_name)
#ifndef CONFIG_BOA_WEB_E8B_CH
int if_chk_add(char *if_name, int receive_mode, int send_mode)
#else
int if_chk_add(char *if_name, int receive_mode, int send_mode, int operation)
#endif
{
struct cfg_interface *ifcp;

	ifcp = malloc(sizeof(struct cfg_interface));
	if(!ifcp)
		return -1;
	strcpy(ifcp->if_name, if_name);
	
	// Added by Mason Yu for bind interface
	ifcp->receive_mode = receive_mode;
	ifcp->send_mode = send_mode;	
	//printf("if_chk_add: if_name=%s receive_mode=%d ifcp->receive_mode=%d ifcp->send_mode=%d\n", if_name, receive_mode, ifcp->receive_mode, ifcp->send_mode);
#ifdef CONFIG_BOA_WEB_E8B_CH
	ifcp->operation = operation;
#endif

	ifcp->if_next = ifchkpq;
	ifchkpq = ifcp;
	return 0;
}

/*
 */
int if_chk_flush(void)
{
struct cfg_interface *p;
  
	/* Remove all entries */
	p = ifchkpq;
	while(p) {
		ifchkpq = p->if_next;
		free(p);
		p = ifchkpq;
	}
	return 0;
}




int if_cfg_refresh(void)
{
struct cfg_interface *ifcp;
// Mason Yu
struct ifreq ifreq;
int s, ifIPFlag;


	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return 0;	
	
	/* check need to add */
	ifcp = ifchkpq;
	while(ifcp) {		
		strcpy(ifreq.ifr_name, ifcp->if_name);				
		
		if (ioctl(s, SIOCGIFADDR, &ifreq) == 0) {
			//printf("The interface(%s) IP exist\n", ifcp->if_name);
			ifIPFlag = 1;
		} else {
			//printf("The interface(%s) IP not exist\n", ifcp->if_name);
			ifIPFlag = 0;		
		}			
		
		if( (!if_cfg_lookup(ifcp->if_name)) && (ifIPFlag == 1) ){
			//printf("Need to add\n");
			// Modified by Mason Yu for bind interafce
			//if_cfg_add(ifcp->if_name);
#ifndef CONFIG_BOA_WEB_E8B_CH
			if_cfg_add(ifcp->if_name, ifcp->receive_mode, ifcp->send_mode);
#else
			if_cfg_add(ifcp->if_name, ifcp->receive_mode, ifcp->send_mode, ifcp->operation);
#endif
		}		
		ifcp = ifcp->if_next;		 
		
	}
	/* check need to delete */
	ifcp = ifcpq;
	while(ifcp) {		
		if(!if_chk_lookup(ifcp->if_name)){			
			if_cfg_del(ifcp->if_name);
		}	
		ifcp = ifcp->if_next;
	}
	
	close(s);
	if_chk_flush();
	return 0;
}
//WiSOC add for redundant route entry check

int getIfaceAddr( char *interface, int type, void *pAddr )
{    
	struct ifreq ifr;    
	int skfd=0, found=0;    
	struct sockaddr_in *addr;    
	skfd = socket(AF_INET, SOCK_DGRAM, 0);	
	if(skfd==-1)		
		return 0;		    
	strcpy(ifr.ifr_name, interface);    
	if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0){    	
		close( skfd );		
		return (0);	
	}    
	if (type == 0) {    	
		if (ioctl(skfd, SIOCGIFHWADDR, &ifr) >= 0) {		
			memcpy(pAddr, &ifr.ifr_hwaddr, sizeof(struct sockaddr));		
			found = 1;	
		}    
	}    else if (type == 1) {	
	if (ioctl(skfd, SIOCGIFADDR, &ifr) == 0) {		
		addr = ((struct sockaddr_in *)&ifr.ifr_addr);		
		*((struct in_addr *)pAddr) = *((struct in_addr *)&addr->sin_addr);		
		found = 1;	
	}    
	}    else if (type == 2) {	
	if (ioctl(skfd, SIOCGIFNETMASK, &ifr) >= 0) {		
		addr = ((struct sockaddr_in *)&ifr.ifr_addr);		
		*((struct in_addr *)pAddr) = *((struct in_addr *)&addr->sin_addr);		
		found = 1;	
		}    
	}    
	close( skfd );    
	return found;
}
static char *get_name(char *name, char *p)
{
    while (isspace(*p))
 p++;
    while (*p) {
 if (isspace(*p))
     break;
 if (*p == ':') { /* could be an alias */
     char *dot = p, *dotname = name;
     *name++ = *p++;
     while (isdigit(*p))
  *name++ = *p++;
     if (*p != ':') { /* it wasn't, backup */
  p = dot;
  name = dotname;
     }
     if (*p == '\0')
  return NULL;
     p++;
     break;
 }
 *name++ = *p++;
    }
    *name++ = '\0';
    return p;
}

int check_isredundant(unsigned long dst, unsigned long netmask, unsigned long gate)
{
	int result=0;
	int ret=0;
	struct in_addr IfaceAddr, IfaceMask;
	unsigned long currIPaddr=0;
	unsigned long currNetMask=0;
	// Mason Yu
	 FILE *fh;
	 char buf[512];


	if(gate==0x7F000001) //do not need to add route to loopback interface 
		return 1;
	//get interface ip from net/dev list , check from interface that system supported
	fh = fopen("/proc/net/dev", "r");
	 if (!fh) {  
	  return 0;
	 }
	 	
	fgets(buf, sizeof buf, fh); /* eat line */
	fgets(buf, sizeof buf, fh);
	
	while (fgets(buf, sizeof buf, fh)) {
	  char *s, name[16];  
	  s = get_name(name, buf);
	  
	  result= getIfaceAddr( name, 1, &IfaceAddr);
	  if(result==1){
	   currIPaddr = (unsigned long)IfaceAddr.s_addr;
	   result= getIfaceAddr( name, 2, &IfaceMask);
	   currNetMask = (unsigned long)IfaceMask.s_addr;
	   //if((dst & netmask)== ( currIPaddr & currNetMask)){ //may do not check netmask??
		 if(currIPaddr==gate){
		  	ret=1;
		  	break;
		  }
	   // }
	   
	  }
	 } 
#if 0 //get interface ip from ifcpq list, check from interface that rip support only	 

	
	for(ifcp = ifcpq; ifcp; ifcp = ifcp->if_next) {
		result=0;
		result= getIfaceAddr( ifcp->if_name, 1, &IfaceAddr);
		if(result==1){
			currIPaddr = (unsigned long)IfaceAddr.s_addr;
			result= getIfaceAddr( ifcp->if_name, 2, &IfaceMask);
			currNetMask = (unsigned long)IfaceMask.s_addr;
			//if((dst & netmask)== ( currIPaddr & currNetMask)){ //may do not check netmask??
					if(currIPaddr==gate){
						ret=1;
						break;
						}
			//	}
			
		}	
		
	}
#endif	
	return ret;
}










