/* Kernel module to match the port-ranges, trigger related port-ranges,
 * and alters the destination to a local IP address.
 *
 * Copyright (C) 2003, CyberTAN Corporation
 * All Rights Reserved.
 *
 * Description:
 *   This is kernel module for port-triggering.
 *
 *   The module follows the Netfilter framework, called extended packet
 *   matching modules.
 */

//#define DEBUG 1

#include <linux/autoconf.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/timer.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netdevice.h>
#include <linux/if.h>
#include <linux/inetdevice.h>
#include <net/protocol.h>
#include <net/checksum.h>

#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_TRIGGER.h>

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_tuple.h>
#include <net/netfilter/nf_conntrack_expect.h>
#include <net/netfilter/nf_conntrack_ecache.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_nat.h>


#ifdef DEBUG
#undef  pr_debug
#define pr_debug(fmt, args...) printk("[%s:%d] "fmt, __FUNCTION__, __LINE__, ##args)
#endif


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Netfilter Core Team <coreteam@netfilter.org>");
MODULE_DESCRIPTION("iptables trigger target module");

/* This rwlock protects the main hash table, protocol/helper/expected
 *    registrations, conntrack timers*/
#define ASSERT_READ_LOCK(x)
#define ASSERT_WRITE_LOCK(x)
#include <linux/netfilter_ipv4/listhelp.h>

struct ipt_trigger {
    struct list_head list;		/* Trigger list */
    struct timer_list timeout;	/* Timer for list destroying */
    u_int32_t srcip;		/* Outgoing source address */
    u_int32_t dstip;		/* Outgoing destination address */
    u_int16_t mproto;		/* Trigger protocol */
    u_int16_t rproto;		/* Related protocol */
    struct ipt_trigger_ports ports;	/* Trigger and related ports */
    u_int8_t reply;			/* Confirm a reply connection */
};

static LIST_HEAD(trigger_list);
static DEFINE_RWLOCK(ip_trigger_lock);
static unsigned int trigger_ip_timeout = 600;	/* 600s */

static void trigger_refresh(struct ipt_trigger *trig, unsigned long extra_jiffies)
{
    NF_CT_ASSERT(trig);
    write_lock_bh(&ip_trigger_lock);

    pr_debug("Refresh trigger, srcip= %pI4, dstip= %pI4, mproto= %d, mport[0..1]=%d-%d, rproto= %d, rport[0..1]=%d-%d.\n",
            &(trig->srcip), &(trig->dstip),
            trig->mproto, trig->ports.mport[0], trig->ports.mport[1],
            trig->rproto, trig->ports.rport[0], trig->ports.rport[1]);

    /* Need del_timer for race avoidance (may already be dying). */
    if (del_timer(&trig->timeout)) {
        trig->timeout.expires = jiffies + extra_jiffies;
        add_timer(&trig->timeout);
    }

    write_unlock_bh(&ip_trigger_lock);
}

static void __del_trigger(struct ipt_trigger *trig)
{
    NF_CT_ASSERT(trig);
    ASSERT_WRITE_LOCK(&ip_trigger_lock);

    pr_debug("Del trigger, srcip= %pI4, dstip= %pI4, mproto= %d, mport[0..1]=%d-%d, rproto= %d, rport[0..1]=%d-%d.\n",
            &(trig->srcip), &(trig->dstip),
            trig->mproto, trig->ports.mport[0], trig->ports.mport[1],
            trig->rproto, trig->ports.rport[0], trig->ports.rport[1]);

    /* delete from 'trigger_list' */
    list_del(&trig->list);
    kfree(trig);
}

static void trigger_timeout(unsigned long ul_trig)
{
    struct ipt_trigger *trig= (void *) ul_trig;

    pr_debug("trigger list %p timed out.\n", trig);
    write_lock_bh(&ip_trigger_lock);
    __del_trigger(trig);
    write_unlock_bh(&ip_trigger_lock);
}

static unsigned int
add_new_trigger(struct ipt_trigger *trig)
{
    struct ipt_trigger *new;

    pr_debug("Add trigger, srcip= %pI4, dstip= %pI4, mproto= %d, mport[0..1]=%d-%d, rproto= %d, rport[0..1]=%d-%d.\n",
            &(trig->srcip), &(trig->dstip),
            trig->mproto, trig->ports.mport[0], trig->ports.mport[1],
            trig->rproto, trig->ports.rport[0], trig->ports.rport[1]);

    write_lock_bh(&ip_trigger_lock);
    new = (struct ipt_trigger *)kmalloc(sizeof(struct ipt_trigger), GFP_ATOMIC);
    if (!new) {
        write_unlock_bh(&ip_trigger_lock);
        pr_debug("OOM allocating trigger list\n");
        return -ENOMEM;
    }

    memset(new, 0, sizeof(*trig));
    INIT_LIST_HEAD(&new->list);
    memcpy(new, trig, sizeof(*trig));

    /* add to global table of trigger */
    list_prepend(&trigger_list, &new->list);

    /* add and start timer if required */
    init_timer(&new->timeout);
    new->timeout.data = (unsigned long)new;
    new->timeout.function = trigger_timeout;
    new->timeout.expires = jiffies + (trigger_ip_timeout * HZ);
    add_timer(&new->timeout);

    write_unlock_bh(&ip_trigger_lock);

    return 0;
}

static inline int trigger_out_matched(const struct ipt_trigger *i,
        const u_int16_t proto, const u_int16_t dport, const struct ipt_trigger_info *info)
{
    pr_debug("i=%p, proto=%d, dport=%d, rproto=%d, rport[0..1]=%d-%d.\n",
        i, proto, dport, info->rproto, info->ports.rport[0], info->ports.rport[1]);
    pr_debug("Got one, mproto= %d, mport[0..1]=%d-%d.\n",
        i->mproto, i->ports.mport[0], i->ports.mport[1]);

    return ((i->mproto == 0 || i->mproto == proto)
        && ((i->ports.mport[0] <= dport) && (i->ports.mport[1] >= dport))
        && (i->rproto == info->rproto)
        && ((i->ports.rport[0] == info->ports.rport[0]) && (i->ports.rport[1] == info->ports.rport[1]))
    );
}

static inline int trigger_in_matched(const struct ipt_trigger *i,
        const u_int16_t proto, const u_int16_t dport)
{
    pr_debug("i=%p, proto= %d, dport=%d.\n", i, proto, dport);
    pr_debug("Got one, rproto= %d, rport[0..1]=%d-%d.\n",
            i->rproto, i->ports.rport[0], i->ports.rport[1]);

    return ((i->rproto ==0 || i->rproto == proto)
        && ((i->ports.rport[0] <= dport) && (i->ports.rport[1] >= dport))
    );
}


static unsigned int
trigger_out(struct sk_buff *skb, const struct xt_target_param *par)
{
    const struct ipt_trigger_info *info = par->targinfo;
    struct ipt_trigger trig, *found;
    const struct iphdr *iph = ip_hdr(skb);
    struct tcphdr *tcph = (void *)iph + iph->ihl*4;

    /* Check if the trigger range has already existed in 'trigger_list'. */
    found = LIST_FIND(&trigger_list, trigger_out_matched, struct ipt_trigger *,
        iph->protocol, ntohs(tcph->dest), info);

    pr_debug("############# out found ############\n");

    if (found) {
        /* Yeah, it exists. We need to update(delay) the destroying timer. */
        trigger_refresh(found, trigger_ip_timeout * HZ);

        /* In order to allow multiple hosts use the same port range, we update
           the 'saddr' after previous trigger has a reply connection. */
        if (found->reply)
            found->srcip = iph->saddr;
    }
    else {
        /* Create new trigger */
        memset(&trig, 0, sizeof(trig));
        trig.srcip = iph->saddr;
        trig.mproto = info->mproto;
        trig.rproto = info->rproto;
        memcpy(&trig.ports, &info->ports, sizeof(struct ipt_trigger_ports));

        /* Add the new 'trig' to list 'trigger_list'. */
        add_new_trigger(&trig);

        pr_debug("new trigger:srcip=%pI4 dstip=%pI4 mproto=%d rproto=%d mport[%d-%d] rport[%d-%d]\n",
                &(trig.srcip), &(trig.dstip), trig.mproto, trig.rproto,
                trig.ports.mport[0], trig.ports.mport[1],
                trig.ports.rport[0], trig.ports.rport[1]);
    }

    /* We don't block any packet. */
    return XT_CONTINUE;
}

static unsigned int
trigger_in(struct sk_buff *skb, const struct xt_target_param *par)
{
    struct ipt_trigger *found;
    const struct iphdr *iph = ip_hdr(skb);
    struct tcphdr *tcph = (void *)iph + iph->ihl * 4;

    /* Check if the trigger-ed range has already existed in 'trigger_list'. */
    found = LIST_FIND(&trigger_list, trigger_in_matched,
            struct ipt_trigger *, iph->protocol, ntohs(tcph->dest));
    if (found) {
        pr_debug("############# in found ############\n");

        /* Yeah, it exists. We need to update(delay) the destroying timer. */
        trigger_refresh(found, trigger_ip_timeout * HZ);

        /* Accept it, or the incoming packet could be dropped in the FORWARD chain */
        return NF_ACCEPT;
    }

    return XT_CONTINUE;	/* Our job is the interception. */
}

static unsigned int
trigger_dnat(struct sk_buff *skb, const struct xt_target_param *par)
{
    struct ipt_trigger *found;
    const struct iphdr *iph = ip_hdr(skb);
    struct tcphdr *tcph = (void *)iph + iph->ihl * 4;
    struct nf_conn *ct;
    enum ip_conntrack_info ctinfo;
    struct nf_nat_range newrange;

    NF_CT_ASSERT(par->hooknum == NF_IP_PRE_ROUTING);

    /* Check if the trigger-ed range has already existed in 'trigger_list'. */
    found = LIST_FIND(&trigger_list, trigger_in_matched,
            struct ipt_trigger *, iph->protocol, ntohs(tcph->dest));
    if (!found || !found->srcip)
    {
        /* We don't block any packet. */
        return XT_CONTINUE;
    }

    pr_debug("############# dnat found ############\n");

    /* Confirm there has been a reply connection. */
    found->reply = 1;
    ct = nf_ct_get(skb, &ctinfo);
    NF_CT_ASSERT(ct && (ctinfo == IP_CT_NEW));

    pr_debug("hooknum=%d got ", par->hooknum);
    nf_ct_dump_tuple(&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);

    /* This must be a fresh one. */
	BUG_ON(ct->status & IPS_NAT_DONE_MASK);

    /* Alter the destination of imcoming packet. */
    newrange = ((struct nf_nat_range)
    {
        IP_NAT_RANGE_MAP_IPS,
        found->srcip, found->srcip,
        { 0 }, { 0 }
    });

    //add by ramen to let the packet through our modem 2008/03/06
    skb->nfctinfo = IP_CT_RELATED;

    /* Hand modified range to generic setup. */
    return nf_nat_setup_info(ct, &newrange, IP_NAT_MANIP_DST);
}

static unsigned int
trigger_tg(struct sk_buff *skb, const struct xt_target_param *par)
{
    const struct ipt_trigger_info *info = par->targinfo;
    const struct iphdr *iph = ip_hdr(skb);
    struct tcphdr *tcph = (void *)iph + iph->ihl*4;	/* Might be TCP, UDP */

    /* The Port-trigger only supports TCP and UDP. */
    if ((iph->protocol != IPPROTO_TCP) && (iph->protocol != IPPROTO_UDP))
        return XT_CONTINUE;

    pr_debug("type=%s, protocol=%s, dport=%d\n",
        (info->type == IPT_TRIGGER_DNAT) ? "dnat"
            :(info->type == IPT_TRIGGER_IN) ? "in" : "out",
        (iph->protocol == IPPROTO_TCP) ? "tcp" : "udp",
        ntohs(tcph->dest));

    if (info->type == IPT_TRIGGER_OUT)
    {
        /* Check if the protocol equal to trigger protocol
           and if the destport is in trigger port range
        */
        if ((info->mproto != 0 && info->mproto != iph->protocol)
            || (ntohs(tcph->dest) < info->ports.mport[0] || ntohs(tcph->dest) > info->ports.mport[1])
        )
        {
            return XT_CONTINUE;
        }

        return trigger_out(skb, par);
    }
    else
    {
        /* Check if the protocol equal to related protocol
           and if the destport is in related port range
        */
        if (info->ports.rport[0])
        {
            if ((info->rproto != 0 && info->rproto != iph->protocol)
                || (ntohs(tcph->dest) < info->ports.rport[0] || ntohs(tcph->dest) > info->ports.rport[1])
            )
            {
                return XT_CONTINUE;
            }
        }

        if (info->type == IPT_TRIGGER_IN)
        {
            return trigger_in(skb, par);
        }
        else if (info->type == IPT_TRIGGER_DNAT)
        {
            return trigger_dnat(skb, par);
        }
    }

    return XT_CONTINUE;
}

static bool trigger_tg_check(const struct xt_tgchk_param *par)
{
    const struct ipt_trigger_info *info = par->targinfo;

    if ((strcmp(par->table, "mangle") == 0)) {
        pr_debug("bad table `%s'.\n", par->table);
        return false;
    }
    if (par->hook_mask & ~((1 << NF_IP_PRE_ROUTING) | (1 << NF_IP_FORWARD))) {
        pr_debug("bad hooks %x, %x.\n", par->hook_mask, ~((1 << NF_IP_PRE_ROUTING) | (1 << NF_IP_FORWARD)));
        return false;
    }
    if (info->mproto) {
        if (info->mproto != IPPROTO_TCP && info->mproto != IPPROTO_UDP) {
            pr_debug("bad mproto %d.\n", info->mproto);
            return false;
        }
    }
    if (info->rproto) {
        if (info->rproto != IPPROTO_TCP && info->rproto != IPPROTO_UDP) {
            pr_debug("bad rproto %d.\n", info->rproto);
            return false;
        }
    }
    if (info->type == IPT_TRIGGER_OUT) {
        if (!info->ports.mport[0] || !info->ports.rport[0]) {
            pr_debug("Try 'iptables -j TRIGGER -h' for help.\n");
            return false;
        }
    }
    else
    {
        if (info->mproto || info->ports.mport[0]) {
            pr_debug("When trigger-type is 'in' or 'dnat', mproto and mport is not needed\n");
            return false;
        }
    }

	return true;
}

/* drop reference count of cluster config when rule is deleted */
static void trigger_tg_destory(const struct xt_tgdtor_param *par)
{
}



static struct proc_dir_entry *trigger_proc = NULL;
static int proc_trigger_read(char *page, char **start, off_t off,
			  int count, int *eof, void *data)
{
    int i = 0;
    char *out = page;
	int len = 0;
    struct list_head *cur_item, *tmp_item;

    out += sprintf(out, "timeout: %d\n", trigger_ip_timeout/60);
    out += sprintf(out, "trigger list:\n");

    /* Empty the 'trigger_list' */
    list_for_each_safe(cur_item, tmp_item, &trigger_list) {
        struct ipt_trigger *trig = (void *)cur_item;
        out += sprintf(out, "#%d srcip= %pI4, dstip= %pI4, mproto= %d, mport[0..1]=%d-%d, rproto= %d, rport[0..1]=%d-%d.\n",
                    ++i, &(trig->srcip), &(trig->dstip),
                    trig->mproto, trig->ports.mport[0], trig->ports.mport[1],
                    trig->rproto, trig->ports.rport[0], trig->ports.rport[1]);
    }
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

static int proc_trigger_write( struct file *filp, const char __user *buf,unsigned long len, void *data )
{
	int ret;
	char str_buf[256];
    char action[20] = {0};
	int val = 0;
    struct list_head *cur_item, *tmp_item;

	if(len > 255)
	{
		printk("Error: the value must be between 10-3600\n");
		return len;
	}

	copy_from_user(str_buf, buf, len);
	str_buf[len] = '\0';

	ret = sscanf(str_buf, "%s %d", action, (int*)&val);
    if(ret != 2 || val < 0 )
	{
		printk("Error.Sample: echo debug 1 > /proc/debug \n");
		return len;
	}
	if (strcmp(action, "timeout") == 0)
	{
        if(val < 1 || val > 9999)
    	{
    		printk("Error: the value must be between 1-9999 minutes\n");
    		return len;
    	}
    	trigger_ip_timeout = val * 60;
	}
    else if (strcmp(action, "flush") == 0)
    {
        pr_debug("Flush trigger list.\n");

        /* Empty the 'trigger_list' */
        list_for_each_safe(cur_item, tmp_item, &trigger_list) {
            struct ipt_trigger *trig = (void *)cur_item;
            del_timer(&trig->timeout);
            __del_trigger(trig);
        }
    }
    else
    {
        printk("Error: Unkown command.\n");
    }

	return len;
}


static struct xt_target trigger_tg_reg __read_mostly = {
	.name		= "TRIGGER",
	.family		= NFPROTO_IPV4,
	.target		= trigger_tg,
	.targetsize	= sizeof(struct ipt_trigger_info),
	//.table		= "nat",
	//.hooks		= (1 << NF_INET_PRE_ROUTING) | (1 << NF_INET_LOCAL_OUT),
	.checkentry	= trigger_tg_check,
	.destroy	= trigger_tg_destory,
	.me		= THIS_MODULE,
};

static int __init trigger_tg_init(void)
{
    trigger_proc = create_proc_entry("porttrigger", 0, NULL);
	if (trigger_proc) {
		trigger_proc->read_proc = proc_trigger_read;
		trigger_proc->write_proc = proc_trigger_write;
	}

    return xt_register_target(&trigger_tg_reg);
}

static void __exit trigger_tg_exit(void)
{
    if (trigger_proc) {
        remove_proc_entry("porttrigger", NULL);
		trigger_proc = NULL;
	}

    xt_unregister_target(&trigger_tg_reg);
}

module_init(trigger_tg_init);
module_exit(trigger_tg_exit);

