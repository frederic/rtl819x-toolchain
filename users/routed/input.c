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
 * From: @(#)input.c	5.22 (Berkeley) 6/1/90
 * From: @(#)input.c	8.1 (Berkeley) 6/5/93
 */
char input_rcsid[] = 
  "$Id: input.c,v 1.2 2009/08/25 11:22:52 bradhuang Exp $";


/*
 * Routing Table Management Daemon
 */

#include "defs.h"
#include <syslog.h>

#if 0//WiSOC unused
#include <rtk/sysconfig.h>
#endif

extern int debug;

struct timeval now;		/* current idea of time */
struct timeval lastbcast;	/* last time all/changes broadcast */
struct timeval lastfullupdate;	/* last time full table broadcast */
struct timeval nextbcast;    /* time to wait before changes broadcast */
int needupdate;		    /* true if we need update at nextbcast */

// Added by Mason Yu for send route error
extern int RIPIF_NUMBER;
extern unsigned long RIPIP[9];

/*
 * Process a newly received packet.
 */

void rip_input(struct sockaddr *from, struct rip *rip, int size)
{
	struct rt_entry *rt;
	struct netinfo *n;
	struct interface *ifp, *ifp2;
	int count, changes = 0;
	struct afswitch *afp;
	static struct sockaddr badfrom, badfrom2;
	struct sockaddr dest;
	char buf1[256], buf2[256];
	int j;
	struct cfg_interface *ifcp;    // Added by Mason Yu for bind interface	
		
	/* check if disable */
	// Commented by Mason Yu
	/*
	if(!ripversion)
		return;
	*/
		
	ifp = 0;
	TRACE_INPUT(ifp, from, (char *)rip, size);
	if (from->sa_family >= af_max ||
	    (afp = &afswitch[from->sa_family])->af_hash == (void (*)(struct sockaddr *, struct afhash *))0) {
		syslog(LOG_INFO,
	 		"\"from\" address in unsupported address family (%d), cmd %d\n",
		    from->sa_family, rip->rip_cmd);		
		return;
	}
	if (rip->rip_vers == 0) {
		syslog(LOG_ERR,
		    "RIP version 0 packet received from %s! (cmd %d)",
		    (*afswitch[from->sa_family].af_format)(from, buf1,
							   sizeof(buf1)),
		    rip->rip_cmd);		
		return;
	}
	
	//syslog(LOG_DEBUG, "got cmd %d",rip->rip_cmd);


	switch (rip->rip_cmd) {
	case RIPCMD_REQUEST:
		if(debug)
			printf("RECV: RIPv%d Request from %s\n", rip->rip_vers, inet_ntoa(satosin(*from)->sin_addr));
		n = rip->rip_nets;
		count = size - ((char *)n - (char *)rip);
		if (count < (int)sizeof (struct netinfo))
			return;
		for (; count > 0; n++) {
			if (count < (int)sizeof (struct netinfo))
				break;
			count -= sizeof (struct netinfo);
			
			n->n_family = ntohs(n->n_family);
            n->n_tag = ntohs(n->n_tag);
            n->n_dst = ntohl(n->n_dst);
            n->n_mask = ntohl(n->n_mask);
            n->n_nhop = ntohl(n->n_nhop);
            n->n_metric = ntohl(n->n_metric);
			/* 
			 * A single entry with sa_family == AF_UNSPEC and
			 * metric ``infinity'' means ``all routes''.
			 * We respond to routers only if we are acting
			 * as a supplier, or to anyone other than a router
			 * (eg, query).
			 */
			if (n->n_family == AF_UNSPEC &&
			    n->n_metric == HOPCNT_INFINITY && count == 0) {
			    	if (supplier || (*afp->af_portmatch)(from) == 0)
			    		// Kaohj
					//supply(from, 0, 0, 0);
					if ((ifp = if_iflookup(from)) != 0)
						supply(from, 0, ifp, 0);
				return;
			}
			if (n->n_family < af_max &&
			    afswitch[n->n_family].af_hash) {
                            struct sockaddr sock;
                            sock.sa_family = n->n_family;
                            
				rt = rtlookup(n);
			}
			else {
				rt = 0;
#define min(a, b) (a < b ? a : b)
			n->n_metric = rt == 0 ? HOPCNT_INFINITY :
				min(rt->rt_metric + 1, HOPCNT_INFINITY);

				n->n_family = htons(n->n_family);
				n->n_tag = htons(n->n_tag);
				n->n_dst = htonl(n->n_dst);
				n->n_mask = htonl(n->n_mask);
				n->n_nhop = htonl(n->n_nhop);
				n->n_metric = htonl(n->n_metric);
			}
///////
		}

		rip->rip_cmd = RIPCMD_RESPONSE;
		/* Warning! it is HERE that the rip structure is filled out with filename information */
		memcpy(packet, rip, size);
		(*afp->af_output)(sock, 0, from, size);
		return;

	case RIPCMD_TRACEON:
	case RIPCMD_TRACEOFF:
		/* verify message came from a privileged port */
		if ((*afp->af_portcheck)(from) == 0)
			return;
		if ((ifp = if_iflookup(from)) == 0) {
			syslog(LOG_ERR, "trace command from unknown router, %s",
			    (*afswitch[from->sa_family].af_format)(from, buf1,
							       sizeof(buf1)));
			return;
		}

		if ((ifp->int_flags & 
			(IFF_BROADCAST|IFF_POINTOPOINT|IFF_REMOTE)) == 0) {
			syslog(LOG_ERR,
			    "trace command from router %s, with bad flags %x",
			    (*afswitch[from->sa_family].af_format)(from, buf1,
							       sizeof(buf1)),
			    ifp->int_flags);
			return;
		}

		if ((ifp->int_flags & IFF_PASSIVE) != 0) {
			syslog(LOG_ERR,
				"trace command from  %s on a passive interface",
			    (*afswitch[from->sa_family].af_format)(from, buf1,
							       sizeof(buf1)));
			return;
		}

		((char *)rip)[size] = '\0';
		if (rip->rip_cmd == RIPCMD_TRACEON)
			traceon(rip->rip_tracefile);
		else
			traceoff();
		return;

	case RIPCMD_RESPONSE:
		/* verify message came from a router */		
		if ((*afp->af_portmatch)(from) == 0) {			
			return;
		}
		(*afp->af_canon)(from);
		/* are we talking to ourselves? */
		
		// Mason Yu		
		//printf("Receive RIP from %x\n",((struct sockaddr_in *)from)->sin_addr.s_addr);
		ifp = if_ifwithaddr(from);			
		if (ifp) {
			if (ifp->int_flags & IFF_PASSIVE) {
				syslog(LOG_ERR,
				  "bogus input (from passive interface, %s)",
				  (*afswitch[from->sa_family].af_format)(from,
							 buf1, sizeof(buf1)));				
				return;
			}
			rt = rtfind(from);
			if (rt == 0 || (((rt->rt_state & RTS_INTERFACE)==0) &&
					rt->rt_metric >= ifp->int_metric)) {	
				addrouteforif(ifp);				
			}
			else
				rt->rt_timer = 0;
			
			return;
		}
		/* check if this interface be configured to supply RIP */
		
		// Added by Mason Yu for Receive RIP via RIP interface
		// If the route does not come from ourself RIP interface, it need not do the following procedures.				
		ifp2 = if_iflookup(from);
		if ( ifp2 == NULL ) {
			printf("RIP: The interafce not supply RIP\n");
			return;
		}	
		
		
		// Modified by Mason Yu for Receive RIP via RIP interface
		//if(!if_cfg_lookup(ifp->int_name)){ 
		// if(!if_cfg_lookup(ifp2->int_name)){
		ifcp = (struct cfg_interface *)if_cfg_lookup(ifp2->int_name);
		if(!ifcp){							
			printf("rip_input: This interface not supply to receive RIP,ifac_name=%s\n",ifcp->if_name);
			return;
		}		

		// Added by Mason Yu for bind interface
		if ( ifcp->receive_mode == RIP_NONE ) {
			return;			
		} else if ( (ifcp->receive_mode != RIP_V1_V2) && (ifcp->receive_mode != rip->rip_vers) ) {
			//printf("ifcp->if_name=%s ifcp->receive_mode=%d  rip->rip_vers=%d\n", ifcp->if_name, ifcp->receive_mode, rip->rip_vers);
			//printf("RIPCMD_RESPONSE: The interface can not receive RIP!!\n");
			return;
		} else if ( (ifcp->receive_mode == RIP_V1_V2) && ( rip->rip_vers != RIP_V1 && rip->rip_vers != RIP_V2) ) {
			return;
		}		
			
		if(debug)
			printf("RECV: RIPv%d Response from %s\n", rip->rip_vers, inet_ntoa(satosin(*from)->sin_addr));

		/*
		 * Update timer for interface on which the packet arrived.
		 * If from other end of a point-to-point link that isn't
		 * in the routing tables, (re-)add the route.
		 */
		if ((rt = rtfind(from)) &&
		    (rt->rt_state & (RTS_INTERFACE | RTS_REMOTE)))
			rt->rt_timer = 0;
		else if ((ifp = if_ifwithdstaddr(from)) &&
		    (rt == 0 || rt->rt_metric >= ifp->int_metric)){		    	
			addrouteforif(ifp);
		}	
		/*
		 * "Authenticate" router from which message originated.
		 * We accept routing packets from routers directly connected
		 * via broadcast or point-to-point networks,
		 * and from those listed in /etc/gateways.
		 */
		if ((ifp = if_iflookup(from)) == 0 || (ifp->int_flags &
		    (IFF_BROADCAST | IFF_POINTOPOINT | IFF_REMOTE)) == 0 ||
		    ifp->int_flags & IFF_PASSIVE) {
			if (memcmp(from, &badfrom, sizeof(badfrom)) != 0) {
				syslog(LOG_ERR,
				  "packet from unknown router, %s",
				  (*afswitch[from->sa_family].af_format)(from,
							 buf1, sizeof(buf1)));
				badfrom = *from;
			}			
			return;
		}
		size -= 4 * sizeof (char);
		n = rip->rip_nets;

/////////
		for (; size > 0; size -= sizeof (struct netinfo), n++) {			
			if(debug) {
				printf(" family=%d, tag=%d, ", n->n_family, n->n_tag);
				printf("dest=%s, ", inet_ntoa(*(struct in_addr*)&n->n_dst));
				printf("mask=%s, ", inet_ntoa(*(struct in_addr*)&n->n_mask));
				printf("nhop=%s, ", inet_ntoa(*(struct in_addr*)&n->n_nhop));
				printf("cost=%d\n", n->n_metric);
			}
			//WiSOC check the dst address is 0.0.0.0 or not
			if(n->n_dst == INADDR_ANY){
				continue;
			}	
			if (size < (int)sizeof (struct netinfo))
				break;
			if (sizeof(n->n_family) > 1)	/* XXX */
				n->n_family = ntohs(n->n_family);
			n->n_metric = ntohl(n->n_metric);
			if (n->n_family >= af_max ||
			    (afp = &afswitch[n->n_family])->af_hash ==
			    (void (*)(struct sockaddr *,struct afhash *))0) {
					syslog(LOG_INFO,
						"route in unsupported address family (%d), from %s (af %d)\n",
				   		n->n_family,
				   		(*afswitch[from->sa_family].af_format)(from,
						buf1, sizeof(buf1)),
				   		from->sa_family);
				continue;
			}
			dest.sa_family = n->n_family;
			satosin(dest)->sin_addr.s_addr = n->n_dst;
			satosin(dest)->sin_port = 0;
			if (((*afp->af_checkhost)(&dest)) == 0) {
				syslog(LOG_DEBUG,
				   "bad host %s in route from %s (af %d)\n",
				   (*afswitch[n->n_family].af_format)(
					&dest, buf1, sizeof(buf1)),
				   (*afswitch[from->sa_family].af_format)(from,
					buf2, sizeof(buf2)),
				   from->sa_family);
				continue;
			}
			if (n->n_metric == 0 ||
			    (unsigned) n->n_metric > HOPCNT_INFINITY) {
				if (memcmp(from, &badfrom2,
				    sizeof(badfrom2)) != 0) {
					syslog(LOG_ERR,
					    "bad metric (%d) from %s\n",
					    n->n_metric,
				  (*afswitch[from->sa_family].af_format)(from,
						buf1, sizeof(buf1)));
					badfrom2 = *from;
				}
				continue;
			}
			/*
			 * Adjust metric according to incoming interface.
			 */
			if ((unsigned) n->n_metric < HOPCNT_INFINITY)
				n->n_metric += ifp->int_metric;
			if ((unsigned) n->n_metric > HOPCNT_INFINITY)
				n->n_metric = HOPCNT_INFINITY;
			rt = rtlookup(n);
			if (rt == 0 ||
			    (rt->rt_state & (RTS_INTERNAL|RTS_INTERFACE)) ==
			    (RTS_INTERNAL|RTS_INTERFACE)) {
				/*
				 * If we're hearing a logical network route
				 * back from a peer to which we sent it,
				 * ignore it.
				 */
				if (rt && rt->rt_state & RTS_SUBNET &&
				    (*afp->af_sendroute)(rt, from))
					continue;
				if ((unsigned)n->n_metric < HOPCNT_INFINITY) {
					struct sockaddr_in netmask;
					struct sockaddr_in *pnmask=&netmask;
					/*
				     * Look for an equivalent route that
				     * includes this one before adding
				     * this route.
				     */
					// Kaohj
					netmask.sin_addr.s_addr = n->n_mask;
				    //rt = rtfind(&dest);
				    rt = rtfind2(&dest, (struct sockaddr *)&netmask, ifp);
				    if (rt && equal(from, &rt->rt_router)) {
				    	rt->rt_timer = 0;
					    continue;
					}
				    // Kaohj
					//if(ripversion == 2 && rip->rip_vers == 2)
					if(rip->rip_vers == 2)
						netmask.sin_addr.s_addr = n->n_mask;
					else
						//netmask.sin_addr.s_addr = inet_maskof(n->n_dst);
						pnmask = 0;
					
					if(debug)
						printf("rtadd: dest=%x gate=%x netmask=%x metric=%d\n", ((struct sockaddr_in *)&dest)->sin_addr.s_addr, ((struct sockaddr_in *)from)->sin_addr.s_addr, netmask.sin_addr.s_addr, n->n_metric);

					// Kaohj
				    //rtadd(&dest, from, n->n_metric, 0, &netmask);
				    rtadd(&dest, from, n->n_metric, 0, pnmask);
				    changes++;
				}
				continue;
			}

			/*
			 * Update if from gateway and different,
			 * shorter, or equivalent but old route
			 * is getting stale.
			 */
			if (equal(from, &rt->rt_router)) {
				if (n->n_metric != rt->rt_metric) {
					rtchange(rt, from, n->n_metric);
					changes++;
					rt->rt_timer = 0;
					if (rt->rt_metric >= HOPCNT_INFINITY)
						rt->rt_timer =
						    GARBAGE_TIME - EXPIRE_TIME;
				} else if (rt->rt_metric < HOPCNT_INFINITY)
					rt->rt_timer = 0;
			} else if ((unsigned) n->n_metric < (unsigned)rt->rt_metric ||
			    (rt->rt_metric == n->n_metric &&
			    rt->rt_timer > (EXPIRE_TIME/2) &&
			    (unsigned) n->n_metric < HOPCNT_INFINITY)) {
				rtchange(rt, from, n->n_metric);
				changes++;
				rt->rt_timer = 0;
			}
		}		
		
		break;
	}

	/*
	 * If changes have occurred, and if we have not sent a broadcast
	 * recently, send a dynamic update.  This update is sent only
	 * on interfaces other than the one on which we received notice
	 * of the change.  If we are within MIN_WAITTIME of a full update,
	 * don't bother sending; if we just sent a dynamic update
	 * and set a timer (nextbcast), delay until that time.
	 * If we just sent a full update, delay the dynamic update.
	 * Set a timer for a randomized value to suppress additional
	 * dynamic updates until it expires; if we delayed sending
	 * the current changes, set needupdate.
	 */
	if (changes && supplier &&
	   now.tv_sec - lastfullupdate.tv_sec < SUPPLY_INTERVAL-MAX_WAITTIME) {
		u_long delay;

		if (now.tv_sec - lastbcast.tv_sec >= MIN_WAITTIME &&
		    timercmp(&nextbcast, &now, <)) {
			if (traceactions)
				fprintf(ftrace, "send dynamic update\n");
			toall(supply, RTS_CHANGED, ifp);
			lastbcast = now;
			needupdate = 0;
			nextbcast.tv_sec = 0;
		} else {
			needupdate++;
			if (traceactions)
				fprintf(ftrace, "delay dynamic update\n");
		}
#define RANDOMDELAY()	(MIN_WAITTIME * 1000000 + \
		(u_long)random() % ((MAX_WAITTIME - MIN_WAITTIME) * 1000000))

		if (nextbcast.tv_sec == 0) {
			delay = RANDOMDELAY();
			if (traceactions)
				fprintf(ftrace,
				    "inhibit dynamic update for %ld usec\n",
				    delay);
			nextbcast.tv_sec = delay / 1000000;
			nextbcast.tv_usec = delay % 1000000;
			timevaladd(&nextbcast, &now);
			/*
			 * If the next possibly dynamic update
			 * is within MIN_WAITTIME of the next full update,
			 * force the delay past the full update,
			 * or we might send a dynamic update just before
			 * the full update.
			 */
			if (nextbcast.tv_sec > lastfullupdate.tv_sec +
			    SUPPLY_INTERVAL - MIN_WAITTIME)
				nextbcast.tv_sec = lastfullupdate.tv_sec +
				    SUPPLY_INTERVAL + 1;
		}
	}
}


int rip_input_init(void)
{
	struct interface *ifp;

	if(ripversion == 2) {
		for (ifp = ifnet; ifp; ifp = ifp->int_next) {
			if (ifp->int_flags & IFF_PASSIVE)
				continue;
			mcast_join(&ifp->int_addr);
		}
	}
	return 0;
}
