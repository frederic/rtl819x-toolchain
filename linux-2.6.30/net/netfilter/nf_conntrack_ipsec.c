#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/ip.h>
#include <net/checksum.h>
#include <net/tcp.h>
#include <net/udp.h>

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_tuple.h>
#include <net/netfilter/nf_conntrack_expect.h>
#include <net/netfilter/nf_conntrack_ecache.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <linux/netfilter/nf_conntrack_ipsec.h>

MODULE_AUTHOR("Magnus Boden <mb@ozaba.mine.nu>");
MODULE_DESCRIPTION("ipsec connection tracking helper");
MODULE_LICENSE("GPL");
MODULE_ALIAS("ip_conntrack_ipsec");
MODULE_ALIAS_NFCT_HELPER("ipsec");

#define MAX_PORTS 8
static unsigned short ports[MAX_PORTS];
static unsigned int ports_c;
module_param_array(ports, ushort, &ports_c, 0400);
MODULE_PARM_DESC(ports, "Port numbers of ipsec servers");

//static DEFINE_SPINLOCK(nf_ipsec_lock);
extern char ipsec_flag;
const unsigned long ip_ct_esp_timeout = 30*HZ;
const unsigned long ip_ct_esp_timeout_stream = 180*HZ;


#if 0
#define DEBUGP(fmt, args...) printk("%s:%s: " fmt, __FILE__, __FUNCTION__, ##args)
#else
#define DEBUGP(format, args...)
#endif


/*
 * Prototypes of IPSec NAT hook
 */
unsigned int (*nf_nat_ipsec_inbound_hook)
				(struct sk_buff *skb,
				 struct nf_conn *ct,
				 enum ip_conntrack_info ctinfo,
				 struct nf_conntrack_expect *exp);
EXPORT_SYMBOL_GPL(nf_nat_ipsec_inbound_hook);


unsigned int (*nf_nat_ipsec_outbound_hook)
				(struct sk_buff *skb,
				 struct nf_conn *ct,
				 enum ip_conntrack_info ctinfo,
				 struct nf_conntrack_expect *exp);
EXPORT_SYMBOL_GPL(nf_nat_ipsec_outbound_hook);

unsigned int (*nf_nat_esp_hook)
				(struct sk_buff *skb,
				 struct nf_conn *ct,
				 enum ip_conntrack_info ctinfo,
				 struct nf_conntrack_expect *exp);
EXPORT_SYMBOL_GPL(nf_nat_esp_hook);


static bool esp_pkt_to_tuple(const struct sk_buff *skb, unsigned int dataoff,
			     struct nf_conntrack_tuple *tuple)
{
    struct udphdr _hdr, *hdr;

	hdr = skb_header_pointer(skb, dataoff, sizeof(_hdr), &_hdr);
	if(hdr == NULL) {
		return false;
	}

	tuple->src.u.all = hdr->source;
	tuple->dst.u.all = hdr->dest;

	DEBUGP("tuple->src = src %x, tuple->dst = dst %x\n", tuple->src.u.all, tuple->dst.u.all);

	return true;
}


static bool esp_invert_tuple(struct nf_conntrack_tuple *tuple,
			    const struct nf_conntrack_tuple *orig)
{
	tuple->src.u.all = orig->dst.u.all;
	tuple->dst.u.all = orig->src.u.all;

	DEBUGP("tuple->src = dst %x, dst = src %x\n", orig->dst.u.all, orig->src.u.all);
	return true;
}

/* Print out the per-protocol part of the tuple. */
static int esp_print_tuple(struct seq_file *s,
			   const struct nf_conntrack_tuple *tuple)
{
	return seq_printf(s, "sport=%hu dport=%hu ",
		       ntohs(tuple->src.u.all),
		       ntohs(tuple->dst.u.all));
}

/* Print out the private part of the conntrack. */
static int esp_print_conntrack(struct seq_file *s,
			       const struct nf_conn *ct)
{
	return seq_printf(s, "timeout=%lu, stream_timeout=%lu ",
			  (ip_ct_esp_timeout / HZ),
			  (ip_ct_esp_timeout_stream / HZ));
}


/* Returns verdict for packet, and may modify conntrack */
static int esp_packet(struct nf_conn *ct,
		      const struct sk_buff *skb,
		      unsigned int dataoff,
		      enum ip_conntrack_info ctinfo,
		      u_int8_t pf,
		      unsigned int hooknum)
{
    if (nf_nat_esp_hook) {
		if (!nf_nat_esp_hook((struct sk_buff *)skb, ct, ctinfo, NULL))
			return  NF_DROP;
	}

	pr_debug("the status of the conntrack is %d\n", ctinfo);

	/* If we've seen traffic both ways, this is some kind of UDP
	   stream.  Extend timeout. */
	if (ct->status & IPS_SEEN_REPLY) {
        nf_ct_refresh_acct(ct, ctinfo, skb, ip_ct_esp_timeout_stream);
		pr_debug("refresh ip_ct on kinda UDP stream.\n");

		/* Also, more likely to be important, and not a probe */
		set_bit(IPS_ASSURED_BIT, &ct->status);
		nf_conntrack_event_cache(IPCT_STATUS, ct);

	} else {
	    nf_ct_refresh_acct(ct, ctinfo, skb, ip_ct_esp_timeout);
		pr_debug("refresh ip_ct with shorter expected timeout.\n");
	}

	return NF_ACCEPT;
}

/* Called when a new connection for this protocol found. */
static bool esp_new(struct nf_conn *ct, const struct sk_buff *skb,
		    unsigned int dataoff)
{
    pr_debug(": ");
	nf_ct_dump_tuple(&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);

	return true;
}

struct nf_conntrack_l4proto nf_conntrack_protocol_esp = {
	.l3proto	 = AF_INET,
	.l4proto	 = IPPROTO_ESP,
	.name		 = "esp",
	.pkt_to_tuple	 = esp_pkt_to_tuple,
	.invert_tuple	 = esp_invert_tuple,
	.print_tuple	 = esp_print_tuple,
	.print_conntrack = esp_print_conntrack,
	.packet		 = esp_packet,  //to do
	.new		 = esp_new,     //to do
 	.destroy	 = NULL,
	.me 		 = THIS_MODULE,
#if defined(CONFIG_NF_CT_NETLINK) || defined(CONFIG_NF_CT_NETLINK_MODULE)
	.tuple_to_nlattr = nf_ct_port_tuple_to_nlattr,
	.nlattr_tuple_size = nf_ct_port_nlattr_tuple_size,
	.nlattr_to_tuple = nf_ct_port_nlattr_to_tuple,
	.nla_policy	 = nf_ct_port_nla_policy,
#endif
};
EXPORT_SYMBOL(nf_conntrack_protocol_esp);


static int ipsec_help(struct sk_buff *skb, unsigned int protoff,
		struct nf_conn *ct, enum ip_conntrack_info ctinfo)
{
	int ret = NF_ACCEPT;

	 #ifdef RTL_NF_ALG_CTL
	ALG_CHECK_ONOFF(alg_type_ipsec);
	#endif

	//call outbound hook to record and update the track of ipsec
	if (nf_nat_ipsec_outbound_hook)
	{
		ret = nf_nat_ipsec_outbound_hook(skb, ct, ctinfo, NULL);
	}

	return ret;
}

static struct nf_conntrack_helper ipsec_helpers[MAX_PORTS] __read_mostly;
static char ipsec_names[MAX_PORTS][sizeof("ipsec-65535")] __read_mostly;
static struct nf_conntrack_expect_policy ipsec_exp_policy;


/* This function is intentionally _NOT_ defined as __exit, because
 * it is needed by the init function */
static void nf_conntrack_ipsec_fini(void)
{
	int i;
	for (i = 0; (i < MAX_PORTS) && ports[i]; i++) {
		nf_conntrack_helper_unregister(&ipsec_helpers[i]);
	}

    nf_conntrack_l4proto_unregister(&nf_conntrack_protocol_esp);
}

static int __init nf_conntrack_ipsec_init(void)
{
    int i ;
    char *tmpname;

	/* If no port given, default to standard ipsec port */

	if (ports_c == 0)
		ports[ports_c++] = IPSEC_PORT;

    ipsec_exp_policy.max_expected = 2;
    ipsec_exp_policy.timeout = ip_ct_esp_timeout_stream;

	for (i = 0; (i < MAX_PORTS) && ports[i]; i++) {
		memset(&ipsec_helpers[i], 0, sizeof(struct nf_conntrack_helper));
        ipsec_helpers[i].tuple.src.l3num = AF_INET;
		ipsec_helpers[i].tuple.src.u.udp.port = htons(ports[i]);
		ipsec_helpers[i].tuple.dst.protonum = IPPROTO_UDP;
		ipsec_helpers[i].expect_policy = &ipsec_exp_policy;
		ipsec_helpers[i].me = THIS_MODULE;
		ipsec_helpers[i].help = ipsec_help;

        tmpname = &ipsec_names[i];
		if (ports[i] == IPSEC_PORT) {
            sprintf(tmpname, "ipsec");
		} else {
            sprintf(tmpname, "ipsec-%d", i);
        }
        ipsec_helpers[i].name = tmpname;

        pr_debug("port #%d: %d\n", i, ports[i]);

		if( nf_conntrack_helper_register(&ipsec_helpers[i])){
			printk("nf_conntrack_ipsec: ERROR registering port %d\n",
				ports[i]);
			nf_conntrack_ipsec_fini();
			return (-1);
		}
	}

    /* tcplen not negative guaranteed by ip_conntrack_tcp.c */
	nf_conntrack_l4proto_register(&nf_conntrack_protocol_esp);

	printk("nf_conntrack_ipsec loaded\n");

    return 0;
}

module_init(nf_conntrack_ipsec_init);
module_exit(nf_conntrack_ipsec_fini);

