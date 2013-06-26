/* Masquerade.  Simple mapping which alters range to a local IP address
   (depending on route). */

/* (C) 1999-2001 Paul `Rusty' Russell
 * (C) 2002-2004 Netfilter Core Team <coreteam@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

//#define DEBUG 1

#include <linux/autoconf.h>
#include <linux/types.h>
#include <linux/inetdevice.h>
#include <linux/ip.h>
#include <linux/timer.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <net/protocol.h>
#include <net/ip.h>
#include <net/checksum.h>
#include <net/route.h>
#include <linux/netdevice.h>
#include <linux/netfilter.h>

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_tuple.h>
#include <net/netfilter/nf_conntrack_expect.h>
#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_nat_rule.h>
#include <net/netfilter/nf_nat_helper.h>

#define ASSERT_READ_LOCK(x)
#define ASSERT_WRITE_LOCK(x)
#include <linux/netfilter_ipv4/listhelp.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Netfilter Core Team <coreteam@netfilter.org>");
MODULE_DESCRIPTION("iptables CONENAT target module");

#ifdef DEBUG
#undef  pr_debug
#define pr_debug(fmt, args...) printk("[%s:%d] "fmt, __FUNCTION__, __LINE__, ##args)
#endif


/* Lock protects conenat region inside conntrack */
static DEFINE_RWLOCK(conenat_lock);

/*
  0, Symmetric nat
  1, full conenat
  2, restricted conenat
  3, port restricted conenat
*/
unsigned int conenat_type = 0;


/****************************************************************************/
void cone_nat_expect(struct nf_conn *ct, struct nf_conntrack_expect *exp)
{
	struct nf_nat_range range;

   	 /* This must be a fresh one. */
    	BUG_ON(ct->status & IPS_NAT_DONE_MASK);

	/* For DST manip, map port here to where it's expected. */
	range.flags = (IP_NAT_RANGE_MAP_IPS | IP_NAT_RANGE_PROTO_SPECIFIED);
	range.min = range.max = exp->saved_proto;
	range.min_ip = range.max_ip
	    = exp->master->tuplehash[!exp->dir].tuple.src.u3.ip;

	/* hook doesn't matter, but it has to do destination manip */
	nf_nat_setup_info(ct, &range, IP_NAT_MANIP_DST);

    	pr_debug("dst nat setup: %pI4:%hu\n",
	       &(range.min_ip),
	       ntohs(range.min.udp.port));

	return;
}

/****************************************************************************/
static int
cone_nat_help(struct sk_buff *skb, unsigned int protoff,
    struct nf_conn *ct, enum ip_conntrack_info ctinfo)
{
	int dir = CTINFO2DIR(ctinfo);
	struct nf_conntrack_expect *exp;
    struct nf_conntrack_tuple *tuple;
    union nf_inet_addr *src_addr = NULL;
    __be16 *src_port = NULL;
	int ret = NF_ACCEPT;

	if (ctinfo == IP_CT_ESTABLISHED || dir != IP_CT_DIR_ORIGINAL)
		return NF_ACCEPT;

	pr_debug("skb[%p] ctinfo[%d] dir[%d]\n", skb, ctinfo, dir);
	pr_debug("packet[%d bytes] "
	       "%pI4:%hu->%pI4:%hu, "
	       "reply: %pI4:%hu->%pI4:%hu\n",
	       skb->len,
	       &(ct->tuplehash[dir].tuple.src.u3.ip),
	       ntohs(ct->tuplehash[dir].tuple.src.u.udp.port),
	       &(ct->tuplehash[dir].tuple.dst.u3.ip),
	       ntohs(ct->tuplehash[dir].tuple.dst.u.udp.port),
	       &(ct->tuplehash[!dir].tuple.src.u3.ip),
	       ntohs(ct->tuplehash[!dir].tuple.src.u.udp.port),
	       &(ct->tuplehash[!dir].tuple.dst.u3.ip),
	       ntohs(ct->tuplehash[!dir].tuple.dst.u.udp.port));

	/* Create expect */
	if ((exp = nf_ct_expect_alloc(ct)) == NULL)
		return NF_ACCEPT;

    /*
      IP_CT_DIR_REPLY
      0 - symmetric nat
      1 - full,  *:* -> natip:natport -> lanip:lanport
      2 - restricted, wanip:* -> natip:natport -> lanip:lanport
      3 - port restricted, wanip:wanport -> natip:natport -> lanip:lanport
    */
    tuple = &ct->tuplehash[!dir].tuple;
    switch (conenat_type)
    {
	case 1:
	    src_addr = NULL;
	    src_port = NULL;
	    break;
        case 2:
            src_addr = &tuple->src.u3;
            src_port = NULL;
            break;
        case 3:
            src_addr = &tuple->src.u3;
            src_port = &tuple->src.u.udp.port;
            break;
        default:
            src_addr = NULL;
            src_port = NULL;
            break;
    }
    nf_ct_expect_init(exp, NF_CT_EXPECT_CLASS_DEFAULT,
    			  nf_ct_l3num(ct),
    			  src_addr, &tuple->dst.u3,
    			  tuple->dst.protonum,
    			  src_port, &tuple->dst.u.udp.port);
    exp->dir = !dir;
    exp->flags = NF_CT_EXPECT_PERMANENT;
    exp->saved_ip = ct->tuplehash[dir].tuple.src.u3.ip;
    exp->saved_proto = ct->tuplehash[dir].tuple.src.u;
    exp->expectfn = cone_nat_expect;

    pr_debug("save %pI4:%hu, ",
        &exp->saved_ip, ntohs(exp->saved_proto.udp.port));
	nf_ct_dump_tuple(&exp->tuple);

    /* Setup expect */
    ret = nf_ct_expect_related(exp);
    nf_ct_expect_put(exp);
    if (ret == 0)
    {
        pr_debug("expect setup, skb=%p, ret=%d.\n", skb, ret);
    }
    else
    {
        pr_debug("expect setup failed.\n");
    }


	return NF_ACCEPT;
}

/****************************************************************************/
struct nf_conntrack_helper nf_conntrack_helper_cone_nat = {
	.name = "CONE-NAT",
	.me = THIS_MODULE,
	.expect_policy = (&(struct nf_conntrack_expect_policy) {
	    .max_expected = 0,
	    .timeout = 60*60*24*365*10,
	}),
	.help = &cone_nat_help
};


/****************************************************************************/
static inline int
exp_cmp(const struct nf_conntrack_expect * exp, u_int32_t ip, u_int16_t port,
	u_int16_t proto)
{
	pr_debug("ip[%d:%d]\n	port[%d:%d]\n	proto[%d:%d]\n",
			exp->tuple.dst.u3.ip, ip,
			exp->tuple.dst.u.udp.port, port,
			exp->tuple.dst.protonum, proto);
	return	exp->tuple.dst.u3.ip == ip &&
			exp->tuple.dst.u.udp.port == port &&
			exp->tuple.dst.protonum == proto;
}

/****************************************************************************/
static inline int
exp_src_cmp(const struct nf_conntrack_expect * exp,
	    const struct nf_conntrack_tuple * tp)
{
	return	exp->saved_ip == tp->src.u3.ip &&
			exp->saved_proto.udp.port == tp->src.u.udp.port &&
			exp->tuple.dst.protonum == tp->dst.protonum;
}



/* FIXME: Multiple targets. --RR */
static bool conenat_tg_check(const struct xt_tgchk_param *par)
{
	const struct nf_nat_multi_range_compat *mr = par->targinfo;

	if (mr->range[0].flags & IP_NAT_RANGE_MAP_IPS) {
		pr_debug("bad MAP_IPS.\n");
		return false;
	}
	if (mr->rangesize != 1) {
		pr_debug("bad rangesize %u.\n", mr->rangesize);
		return false;
	}

	return true;
}

unsigned int rtl_find_appropriate_newrange(struct nf_conn *ct, __be32 newsrc, const struct nf_nat_multi_range_compat *mr)
{
	struct net *net;
	unsigned int ret,expectcount = net->ct.expect_count;
	u_int16_t minport, maxport;
	u_int16_t newport, tmpport;
	struct nf_conntrack_expect *exp=NULL;
	struct nf_conntrack_tuple tuple;
	struct nf_nat_range newrange;
	struct nf_conn_help *help = nfct_help(ct);

	net = nf_ct_net(ct);
	expectcount = net->ct.expect_count;
	/* Choose port */
	spin_lock_bh(&nf_conntrack_lock);

	#if 0
	exp = LIST_FIND(&nf_conntrack_expect_list,
			exp_src_cmp,
			struct nf_conntrack_expect *,
			&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);
	#endif

	memset(&tuple,0,sizeof(tuple));

	//src
	tuple.src.l3num = ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.l3num;
	tuple.src.u3.ip = ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.ip;
	tuple.src.u.udp.port = ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.udp.port;

	//dst
	tuple.dst.u3.ip = newsrc;
	//tuple.dst.u.udp.port = htons(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.udp.port);
	newport = htons(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.udp.port);
	tuple.dst.protonum = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum;

	pr_debug("tupple1 = %pI4:%hu\n", &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip,ntohs(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.udp.port));

	if(expectcount > 0){
		for(tmpport=0; (tmpport<=expectcount)&&(newport<=65535); tmpport++,newport++){
			tuple.dst.u.udp.port=newport;
			exp = __nf_ct_expect_find_bysave(net, &tuple, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);
			if(exp)
				break;
		}
	}

	if (exp) {
		minport = maxport = exp->tuple.dst.u.udp.port;
		pr_debug("existing mapped port = %hu\n", ntohs(minport));
	} else {


		minport = mr->range[0].min.udp.port == 0 ?
			ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.udp.port :
			mr->range[0].min.udp.port;

		maxport = mr->range[0].max.udp.port == 0 ?
			htons(65535) :
			mr->range[0].max.udp.port;

		for (newport = ntohs(minport),tmpport = ntohs(maxport);
			 newport <= tmpport; newport++) {
        #if 0
			exp = LIST_FIND(&ip_conntrack_expect_list,
					   exp_cmp,
					   struct nf_conntrack_expect *,
					   newsrc, htons(newport), ct->tuplehash[IP_CT_DIR_ORIGINAL].
					   tuple.dst.protonum);
        #endif

			//dst
		tuple.dst.u.udp.port = htons(newport);

			 exp = __nf_ct_expect_find(net, &tuple);
			if (!exp)
			{
				pr_debug("new mapping: %pI4:%hu -> %pI4:%hu\n",
					&(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.ip),
					ntohs(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.udp.port),
					&newsrc,  newport);
				minport = maxport = htons(newport);
				break;
			}
		}
	}
	spin_unlock_bh(&nf_conntrack_lock);

	newrange.flags = mr->range[0].flags | IP_NAT_RANGE_MAP_IPS |IP_NAT_RANGE_PROTO_SPECIFIED;
	newrange.min_ip = newrange.max_ip = newsrc;
	newrange.min.udp.port = minport;
	newrange.max.udp.port = maxport;

	/* Set ct helper */
	ret = nf_nat_setup_info(ct, &newrange, IP_NAT_MANIP_SRC);
	if (ret == NF_ACCEPT)
        {
            rcu_read_lock();
            if (help == NULL) {
                help = nf_ct_helper_ext_add(ct, GFP_ATOMIC);
                if (help == NULL) {
                    return NF_ACCEPT;
                }
            } else {
                memset(&help->help, 0, sizeof(help->help));
            }
            rcu_assign_pointer(help->helper, &nf_conntrack_helper_cone_nat);
            rcu_read_unlock();
	}

	return ret;
}

static unsigned int
conenat_tg(struct sk_buff *skb, const struct xt_target_param *par)
{
       struct net *net;
	struct nf_conn *ct;
       struct nf_conn_nat *nat;
	enum ip_conntrack_info ctinfo;
	struct nf_nat_range newrange;
       const struct nf_nat_multi_range_compat *mr;
	struct rtable *rt;
	__be32 newsrc;

	NF_CT_ASSERT(par->hooknum == NF_INET_POST_ROUTING);

	ct = nf_ct_get(skb, &ctinfo);
	nat = nfct_nat(ct);
   	net = nf_ct_net(ct);

	NF_CT_ASSERT(ct && (ctinfo == IP_CT_NEW || ctinfo == IP_CT_RELATED
			    || ctinfo == IP_CT_RELATED + IP_CT_IS_REPLY));


    /* Source address is 0.0.0.0 - locally generated packet that is
     * probably not supposed to be masqueraded.
     */
	 if (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip == 0)
	        return NF_ACCEPT;

    	mr = par->targinfo;
    	rt = skb->rtable;
    	newsrc = inet_select_addr(par->out, rt->rt_gateway, RT_SCOPE_UNIVERSE);
    	if (!newsrc) {
        	printk("CONENAT: %s ate my IP address\n", par->out->name);
        	return NF_DROP;
    	}

       write_lock_bh(&conenat_lock);
	nat->masq_index = par->out->ifindex;
	write_unlock_bh(&conenat_lock);

	if (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum == IPPROTO_UDP)
	{
		unsigned int ret,expectcount = net->ct.expect_count;
		u_int16_t minport, maxport;
		u_int16_t newport, tmpport;
		struct nf_conntrack_expect *exp=NULL;
              struct nf_conntrack_tuple tuple;
              struct nf_conn_help *help = nfct_help(ct);

		/* Choose port */
		spin_lock_bh(&nf_conntrack_lock);

       		#if 0
		exp = LIST_FIND(&nf_conntrack_expect_list,
                exp_src_cmp,
				struct nf_conntrack_expect *,
				&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);
        	#endif

		memset(&tuple,0,sizeof(tuple));

		//src
		tuple.src.l3num = ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.l3num;
		tuple.src.u3.ip = ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.ip;
		tuple.src.u.udp.port = ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.udp.port;

        	//dst
              tuple.dst.u3.ip = newsrc;
		//tuple.dst.u.udp.port = htons(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.udp.port);
		newport = htons(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.udp.port);
		tuple.dst.protonum = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum;

		pr_debug("tupple1 = %pI4:%hu\n", &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip,ntohs(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.udp.port));


		if(expectcount > 0){
			for(tmpport=0; (tmpport<=expectcount)&&(newport<=65535); tmpport++,newport++){
				tuple.dst.u.udp.port=newport;
				exp = __nf_ct_expect_find_bysave(net, &tuple, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);
				if(exp)
					break;
			}
		}

		if (exp) {
			minport = maxport = exp->tuple.dst.u.udp.port;
			pr_debug("existing mapped port = %hu\n", ntohs(minport));
		} else {


			minport = mr->range[0].min.udp.port == 0 ?
				ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.udp.port :
				mr->range[0].min.udp.port;

			maxport = mr->range[0].max.udp.port == 0 ?
				htons(65535) :
				mr->range[0].max.udp.port;

			for (newport = ntohs(minport),tmpport = ntohs(maxport);
			     newport <= tmpport; newport++) {
                #if 0
                exp = LIST_FIND(&ip_conntrack_expect_list,
					       exp_cmp,
					       struct nf_conntrack_expect *,
					       newsrc, htons(newport), ct->tuplehash[IP_CT_DIR_ORIGINAL].
					       tuple.dst.protonum);
                #endif

				//dst
			tuple.dst.u.udp.port = htons(newport);

               	 exp = __nf_ct_expect_find(net, &tuple);
		        if (!exp)
			{
				pr_debug("new mapping: %pI4:%hu -> %pI4:%hu\n",
                    &(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.ip),
                    ntohs(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.udp.port),
                    &newsrc,  newport);
				minport = maxport = htons(newport);
				break;
			}
			}
		}
		spin_unlock_bh(&nf_conntrack_lock);

		newrange.flags = mr->range[0].flags | IP_NAT_RANGE_MAP_IPS |IP_NAT_RANGE_PROTO_SPECIFIED;
		newrange.min_ip = newrange.max_ip = newsrc;
		newrange.min.udp.port = minport;
		newrange.max.udp.port = maxport;

		/* Set ct helper */
		ret = nf_nat_setup_info(ct, &newrange, IP_NAT_MANIP_SRC);
		if (ret == NF_ACCEPT)
	        {
	            rcu_read_lock();
	            if (help == NULL) {
	                help = nf_ct_helper_ext_add(ct, GFP_ATOMIC);
	                if (help == NULL) {
	                    return NF_ACCEPT;
	                }
	            } else {
	                memset(&help->help, 0, sizeof(help->help));
	            }
	            rcu_assign_pointer(help->helper, &nf_conntrack_helper_cone_nat);
	            rcu_read_unlock();

		    pr_debug("helper setup, skb=%p\n", skb);
		}

		return ret;
	}

	/* Transfer from original range. */
	newrange = ((struct nf_nat_range)
		{ mr->range[0].flags | IP_NAT_RANGE_MAP_IPS,
		  newsrc, newsrc,
		  mr->range[0].min, mr->range[0].max });

	/* Hand modified range to generic setup. */
	return nf_nat_setup_info(ct, &newrange, IP_NAT_MANIP_SRC);
}

static int
device_cmp(struct nf_conn *i, void *ifindex)
{
	const struct nf_conn_nat *nat = nfct_nat(i);
	int ret;

	if (!nat)
		return 0;

	read_lock_bh(&conenat_lock);
	ret = (nat->masq_index == (int)(long)ifindex);
	read_unlock_bh(&conenat_lock);

	return ret;
}

static int conenat_device_event(struct notifier_block *this,
			     unsigned long event,
			     void *ptr)
{
	const struct net_device *dev = ptr;
	struct net *net = dev_net(dev);

	if (event == NETDEV_DOWN) {
		/* Device was downed.  Search entire table for
		   conntracks which were associated with that device,
		   and forget them. */
		NF_CT_ASSERT(dev->ifindex != 0);

		nf_ct_iterate_cleanup(net, device_cmp,
				      (void *)(long)dev->ifindex);
	}

	return NOTIFY_DONE;
}

static int conenat_inet_event(struct notifier_block *this,
			   unsigned long event,
			   void *ptr)
{
	struct net_device *dev = ((struct in_ifaddr *)ptr)->ifa_dev->dev;
    return conenat_device_event(this, event, dev);
}

static struct notifier_block conenat_dev_notifier = {
	.notifier_call	= conenat_device_event,
};

static struct notifier_block conenat_inet_notifier = {
	.notifier_call	= conenat_inet_event,
};


static struct proc_dir_entry *conenat_proc = NULL;
static int proc_conenat_read(char *page, char **start, off_t off,
			  int count, int *eof, void *data)
{
    char *out = page;
	int len = 0;

    out += sprintf(out, "ConeNATType=%d\n", conenat_type);
	len = out - page;
	len -= off;
	if (len < count) {
		*eof = 1;
		if (len <= 0)
            return 0;
	} else
		len = count;

	*start = page + off;
	return len;
}

static int proc_conenat_write( struct file *filp, const char __user *buf,unsigned long len, void *data )
{
	int ret;
	char str_buf[256];
	int val = 0;

	if(len > 255)
	{
		printk("Error: the value must be between 0-3\n");
		return len;
	}

	copy_from_user(str_buf, buf, len);
	str_buf[len] = '\0';

	ret = sscanf(str_buf, "%d", (int*)&val);
	if(ret != 1 || val < 0 || val > 3)
	{
		printk("Error: the value must be between 0-3\n");
		return len;
	}

    conenat_type = val;

	return len;
}


static struct xt_target conenat_tg_reg __read_mostly = {
	.name		= "CONENAT",
	.family		= NFPROTO_IPV4,
	.target		= conenat_tg,
	.targetsize	= sizeof(struct nf_nat_multi_range_compat),
	.table		= "nat",
	.hooks		= 1 << NF_INET_POST_ROUTING,
	.checkentry	= conenat_tg_check,
	.me		    = THIS_MODULE,
};

static int __init ipt_conenat_tg_init(void)
{
	int ret;

    conenat_proc = create_proc_entry("conenat", 0, NULL);
	if (conenat_proc) {
		conenat_proc->read_proc = proc_conenat_read;
		conenat_proc->write_proc = proc_conenat_write;
	}

	ret = xt_register_target(&conenat_tg_reg);

	if (ret == 0) {
		/* Register for device down reports */
		register_netdevice_notifier(&conenat_dev_notifier);
		/* Register IP address change reports */
		register_inetaddr_notifier(&conenat_inet_notifier);
	}

	printk("\nipt_conenat_init for cone nat nf_conntrack \n");

	return ret;
}

static void __exit ipt_conenat_tg_exit(void)
{
	xt_unregister_target(&conenat_tg_reg);
	unregister_netdevice_notifier(&conenat_dev_notifier);
	unregister_inetaddr_notifier(&conenat_inet_notifier);
}

module_init(ipt_conenat_tg_init);
module_exit(ipt_conenat_tg_exit);

