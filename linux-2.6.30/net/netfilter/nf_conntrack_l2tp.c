/*
 * nf_conntrack_l2tp.c	- Version 3.0
 *
 * Connection tracking support for PPTP (Point to Point Tunneling Protocol).
 * PPTP is a a protocol for creating virtual private networks.
 * It is a specification defined by Microsoft and some vendors
 * working with Microsoft.  PPTP is built on top of a modified
 * version of the Internet Generic Routing Encapsulation Protocol.
 * GRE is defined in RFC 1701 and RFC 1702.  Documentation of
 * PPTP can be found in RFC 2637
 *
 * (C) 2000-2005 by Harald Welte <laforge@gnumonks.org>
 *
 * Development of this code funded by Astaro AG (http://www.astaro.com/)
 *
 * Limitations:
 * 	 - We blindly assume that control connections are always
 * 	   established in PNS->PAC direction.  This is a violation
 * 	   of RFFC2673
 * 	 - We can only support one single call within each session
 *
 * TODO:
 *	 - testing of incoming L2TP calls
 *
 * Changes:
 * 	2002-02-05 - Version 1.3
 * 	  - Call nf_conntrack_unexpect_related() from
 * 	    pptp_destroy_siblings() to destroy expectations in case
 * 	    CALL_DISCONNECT_NOTIFY or tcp fin packet was seen
 * 	    (Philip Craig <philipc@snapgear.com>)
 * 	  - Add Version information at module loadtime
 * 	2002-02-10 - Version 1.6
 * 	  - move to C99 style initializers
 * 	  - remove second expectation if first arrives
 * 	2004-10-22 - Version 2.0
 * 	  - merge Mandrake's 2.6.x port with recent 2.6.x API changes
 * 	  - fix lots of linear skb assumptions from Mandrake's port
 * 	2005-06-10 - Version 2.1
 * 	  - use nf_conntrack_expect_free() instead of kfree() on the
 * 	    expect's (which are from the slab for quite some time)
 * 	2005-06-10 - Version 3.0
 * 	  - port helper to post-2.6.11 API changes,
 * 	    funded by Oxcoda NetBox Blue (http://www.netboxblue.com/)
 * 	2005-07-30 - Version 3.1
 * 	  - port helper to 2.6.13 API changes
 *
 */

#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/ip.h>
#include <net/checksum.h>
#include <net/tcp.h>

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_helper.h>

#define IP_CT_L2TP_VERSION "3.1"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Harald Welte <laforge@gnumonks.org>");
MODULE_DESCRIPTION("Netfilter connection tracking helper module for l2tp");

#if 0
#define DEBUGP(format, args...)	printk(KERN_DEBUG "%s:%s: " format, __FILE__, __FUNCTION__, ## args)
#else
#define DEBUGP(format, args...)
#endif
#define L2TP_CONTROL_PORT 1701


/* track caller id inside control connection, call expect_related */
static int conntrack_l2tp_help(struct sk_buff *skb, unsigned int protoff,
		struct nf_conn *ct, enum ip_conntrack_info ctinfo)
{
    #ifdef RTL_NF_ALG_CTL
    ALG_CHECK_ONOFF(alg_type_l2tp);
    #endif

    return NF_ACCEPT;
}

static struct nf_conntrack_expect_policy l2tp_exp_policy =  {
    .max_expected = 2,
    .timeout = 5 * 60
};

/* control protocol helper */
static struct nf_conntrack_helper l2tp = {
	.name = "l2tp",
	.me = THIS_MODULE,
	.tuple.src.l3num	= AF_INET,
	.tuple.src.u.udp.port	= __constant_htons(L2TP_CONTROL_PORT),
	.tuple.dst.protonum	= IPPROTO_UDP,

	.help = conntrack_l2tp_help,
	.expect_policy = &l2tp_exp_policy
};


static int __init nf_conntrack_l2tp_init(void)
{
	int retcode;
	DEBUGP("registering helper\n");
	if ((retcode = nf_conntrack_helper_register(&l2tp))) {
		printk(KERN_ERR "Unable to register conntrack application "
				"helper for l2tp: %d\n", retcode);
		return retcode;
	}

	printk("nf_conntrack_l2tp version %s loaded\n", IP_CT_L2TP_VERSION);
	return 0;
}

static void  nf_conntrack_l2tp_fini(void)
{
	nf_conntrack_helper_unregister(&l2tp);
	printk("nf_conntrack_pptp version %s unloaded\n", IP_CT_L2TP_VERSION);
}


module_init(nf_conntrack_l2tp_init);
module_exit(nf_conntrack_l2tp_fini);
