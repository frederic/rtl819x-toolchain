#define _XOPEN_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <syslog.h>

#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/if_ether.h>
#include <netpacket/packet.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <sys/ioctl.h>
#include "mld.h"

#define PIDFILE "/var/run/ecmh.pid"
#define ECMH_DUMPFILE "/var/run/ecmh.dump"

// Defines
#undef DEBUG
#undef ECMH_SUPPORT_IPV4
#undef ECMH_SUPPORT_MLD2

#ifdef DEBUG
#define D(x) x
#else
#define D(x) {}
#endif

// The timeout for joins, verify with draft - every 5 minutes
#define ECMH_SUBSCRIPTION_TIMEOUT (5*60)

// XXX: Can this be optimized with a 128bit compare?
#define COMPARE_IPV6_ADDRESS(a,b) (a.s6_addr32[0] == b.s6_addr32[0] && a.s6_addr32[1] == b.s6_addr32[1] && a.s6_addr32[2] == b.s6_addr32[2] && a.s6_addr32[3] == b.s6_addr32[3])
#define IS_MC_NODELOCAL(a) (a.s6_addr16[0] == htons(0xff01))
#define IS_MC_SITELOCAL(a) (a.s6_addr16[0] == htons(0xff02))

#define IN6_IS_ADDR_MC_ZERO(a) \
	(IN6_IS_ADDR_MULTICAST(a)					      \
	 && ((((__const uint8_t *) (a))[1] & 0xf) == 0x0))

#include "linklist.h"
#include "common.h"

// Booleans
#define false	0
#define true	(!false)
#define bool	int

#include "interfaces.h"
#include "groups.h"
#include "grpint.h"
#include "subscr.h"

#ifndef ICMP6_MEMBERSHIP_QUERY
#define ICMP6_MEMBERSHIP_QUERY     MLD_LISTENER_QUERY
#define ICMP6_MEMBERSHIP_REDUCTION MLD_LISTENER_REDUCTION
#define ICMP6_MEMBERSHIP_REPORT    MLD_LISTENER_REPORT
#define MLD_LISTENER_DONE          MLD_LISTENER_REDUCTION
#define MLD_MTRACE_RESP            201
#define MLD_MTRACE                 202
#endif
#ifndef IPV6_TLV_PAD0
#define IPV6_TLV_PAD0		0
#define IPV6_TLV_PADN		1
#define IPV6_TLV_ROUTERALERT	5
#define IPV6_TLV_JUMBO		194
#define IPV6_TLV_HAO		201	/* home address option */
#endif
// Our configuration structure
struct conf
{
	int			maxgroups;
	struct list		*ints;				// The interfaces we are watching *
	struct list		*groups;			// The groups we are joined to

	bool			daemonize;			// To Daemonize or to not to Daemonize
	
	int			rawsocket;			// RAW socket for receiving.
	int			rawsocket_out;			// RAW socket for sending outward.
	int			sock_icmp;				// RAW socket for sending icmpv6
	time_t			stat_starttime;			// When did we start.
	uint64_t		stat_packets_received;		// Number of packets received
	uint64_t		stat_packets_sent;		// Number of packets forwarded
	uint64_t		stat_bytes_received;		// Number of bytes received
	uint64_t		stat_bytes_sent;		// Number of bytes forwarded
};

// Global Stuff
extern struct conf *g_conf;

void mld_send_report(struct intnode *intn, const struct in6_addr *mca);
