#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/kernel.h>
#include <net/tcp.h>

#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_nat_helper.h>
#include <net/netfilter/nf_nat_rule.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_expect.h>
#include <linux/netfilter/nf_conntrack_ipsec.h>

#if 0
#define DEBUGP(fmt, args...) printk("%s:%s: " fmt, __FILE__, __FUNCTION__, ##args)
#else
#define DEBUGP(format, args...)
#endif


#if 0
#define ASSERT_READ_LOCK(x) MUST_BE_READ_LOCKED(&ip_conntrack_lock)
#define ASSERT_WRITE_LOCK(x) MUST_BE_WRITE_LOCKED(&ip_conntrack_lock)
#endif

struct timer_list ipsec_time;
static struct isakmp_data_s *_isakmpDb=NULL;
u_int32_t esp_addr=0;
u_int32_t esp_spi=0;
u_int16_t spi0;
u_int16_t spi1;
char ipsec_flag='1';

#define MAX_PORTS 8
//lyl: in kernel 2.6.19, the tuple hash is different from kernel 2.4.*

static void _prepend_to_hash(struct nf_conntrack_tuple_hash *tuplehash_reply,struct nf_conntrack_tuple_hash *tuplehash_orig, u_int32_t spi)
{
    u_int16_t	*pu16 = (u_int16_t *)&spi;
    struct nf_conntrack_tuple *tuple;
    struct nf_conn *ct;
    ct = nf_ct_tuplehash_to_ctrack(tuplehash_orig);

    // It's the first reply, associate to this connection
    tuple = &tuplehash_reply->tuple;

    write_lock_bh(&nf_conntrack_lock);
    hlist_nulls_del_rcu(&tuplehash_reply->hnnode);
    hlist_nulls_del_rcu(&tuplehash_orig->hnnode);
    write_unlock_bh(&nf_conntrack_lock);

    tuple->src.u.all = pu16[0];
    tuple->dst.u.all = pu16[1];
    nf_conntrack_hash_insert(ct);
    DEBUGP("%s, lyl: after change ,the real conntrack hash is\n",__func__);
    nf_ct_dump_tuple(&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);
    nf_ct_dump_tuple(&ct->tuplehash[IP_CT_DIR_REPLY].tuple);
}

static struct isakmp_data_s*
_findEspIn(u_int32_t peer_ip,u_int32_t alias_ip,u_int32_t ispi){
    int i;
    struct isakmp_data_s *tp,*new_tp;
    tp = _isakmpDb;
    new_tp=NULL;

    for(i = 0 ; i < MaxSession ; i++)
    {
        if(tp->peer_ip == peer_ip &&
                tp->alias_ip == alias_ip &&
                tp->state == IPSEC_USED &&

                //(tp->ispi & ispi) == tp->ispi )
            tp->ispi == ispi )
            {
                tp->idle_timer = 0;
                // kaohj
                //tp->ispi = ispi;
                DEBUGP("Found session #%d in\n", i);
                return tp;
            }
        tp++;
    }
    tp = _isakmpDb;
    for(i = 0 ; i < MaxSession ; i++)
    {
        if(tp->peer_ip == peer_ip &&
                tp->alias_ip == alias_ip &&
                // kaohj
                tp->ispi == 0 &&
                tp->state == IPSEC_USED)
        {
            // kaohj
            DEBUGP("Refresh ESP session #%d on reply (new spi)\n", i);
            new_tp=tp;
            //lyl
            break;
        }
        tp++;

    }
    /*********************    lyl chenge end *******************************/
    if(new_tp!=NULL)
    {
        // Here comes new spi
        new_tp->idle_timer = 0;
        new_tp->ispi = ispi;

        if (new_tp->pctrack != 0)
        {
            _prepend_to_hash(&new_tp->pctrack->tuplehash[IP_CT_DIR_REPLY],&new_tp->pctrack->tuplehash[IP_CT_DIR_ORIGINAL], ispi);
        }
        else
            DEBUGP("should not be here!\n");
        return new_tp;
    }

    return NULL;
}


static struct isakmp_data_s*
_findEspOut(u_int32_t local_ip,u_int32_t peer_ip, u_int32_t ospi){
    int i;
    struct isakmp_data_s *tp;
    tp = _isakmpDb;

    for(i = 0 ; i < MaxSession ; i++){
        if(tp->peer_ip == peer_ip &&
                tp->local_ip == local_ip &&

                //(tp->ospi  & ospi ) == tp->ospi &&
                tp->ospi == ospi &&
                tp->state == IPSEC_USED ){
            tp->idle_timer = 0;
            tp->ospi = ospi;
            DEBUGP("Found session #%d out\n", i);
            return tp;
        }
        tp++;
    }

    return NULL;
}



static struct isakmp_data_s*
_addEsp(u_int32_t local_ip,u_int32_t peer_ip,u_int32_t ospi){
    int i;
    struct isakmp_data_s *tp;
    tp = _isakmpDb;

    for(i = 0 ; i < MaxSession ; i++){
        if(tp->state == IPSEC_USED && tp->peer_ip == peer_ip
                && tp->local_ip == local_ip){
            // Here comes new spi
            DEBUGP("New ESP session #%d out, spi=%x -> %x\n", i, tp->ospi, ospi);
            tp->idle_timer = 0;
            tp->ospi = ospi;
            tp->ispi = 0;
            return tp;
        }
        tp++;
    }
    return NULL;
}




/*********************************************************************************
 * Routine Name :  _findIsakmpIn
 * Description :
 * Input :
 * Output :
 * Return :
 * Note :
 *        ThreadSafe: n
 **********************************************************************************/
static struct isakmp_data_s*
_findIsakmpIn(u_int32_t peer_ip,u_int32_t alias_ip,u_int64_t icookie,u_int64_t rcookie){
    int i;
    struct isakmp_data_s *tp;

    u_int32_t *pu32;
    tp = _isakmpDb;

    for(i = 0 ; i < MaxSession ; i++){
        if(tp->peer_ip == peer_ip &&
                //  tp->alias_ip == alias_ip &&
                tp->icookie == icookie &&
                tp->state == IPSEC_USED){
            tp->idle_timer = 0;
            tp->icookie = icookie;

            pu32 = (u_int32_t *)&icookie;
            DEBUGP("find IKE in i=%d, local_ip=%d.%d.%d.%d, icookie=%04x%04x\n",
                    i, NIPQUAD(tp->local_ip), pu32[0], pu32[1]);
            return tp;
        }
        tp++;
    }

    return NULL;
}


/*********************************************************************************
 * Routine Name :  _findIsakmpOut
 * Description :
 * Input :
 * Output :
 * Return :
 * Note :
 *        ThreadSafe: n
 **********************************************************************************/
static struct isakmp_data_s*
_findIsakmpOut(u_int32_t local_ip,u_int32_t peer_ip,u_int64_t icookie,u_int64_t rcookie){
    int i;
    struct isakmp_data_s *tp = _isakmpDb;
    u_int32_t *pu32;

    for(i = 0 ; i < MaxSession  ; i++){
        if(tp->peer_ip == peer_ip &&
                tp->local_ip == local_ip &&
                tp->icookie == icookie &&
                tp->state == IPSEC_USED){

            pu32 = (u_int32_t *)&icookie;
            DEBUGP("find IKE out i=%d, local %d.%d.%d.%d peer %d.%d.%d.%d icookie=%04x%04x\n",
                    i,NIPQUAD(local_ip), NIPQUAD(peer_ip), pu32[0], pu32[1]);
            return tp;
        }
        tp++;
    }

    return NULL;
}



/*********************************************************************************
 * Routine Name :  _addIsakmp
 * Description :
 * Input :
 * Output :
 * Return :
 * Note :
 *        ThreadSafe: n
 **********************************************************************************/
static struct isakmp_data_s*
_addIsakmp(u_int32_t local_ip,u_int32_t peer_ip,u_int32_t alias_ip,
        u_int64_t icookie,u_int64_t rcookie){
    int i;
    struct isakmp_data_s *tp;
    tp = _isakmpDb;
    // find the existed one
    for(i = 0 ; i < MaxSession  ; i++){
        if(tp->peer_ip == peer_ip &&
                tp->local_ip == local_ip &&
                tp->state == IPSEC_USED) {
            // session refresh
            tp->idle_timer = 0;
            tp->icookie = icookie;
            tp->rcookie = rcookie;
            DEBUGP("Existed session #%d been found\n", i);
            return tp;
        }
        tp++;
    }

    tp = _isakmpDb;
    // if not already exists, find a new one
    for(i = 0 ; i < MaxSession  ; i++){
        if(tp->state == IPSEC_FREE){
            memset(tp,0,sizeof(struct isakmp_data_s));
            tp->idle_timer = 0;
            tp->peer_ip = peer_ip;
            tp->local_ip = local_ip;
            tp->alias_ip = alias_ip;
            tp->icookie = icookie;
            tp->rcookie = rcookie;
            tp->state = IPSEC_USED;

            DEBUGP("Free session #%d been found\n", i);
            return tp;
        }
        tp++;
    }
    return NULL;
}


//static unsigned int esp_help(struct sk_buff **pskb,
static unsigned int esp_help(struct sk_buff *skb,
        struct nf_conn *ct,
        enum ip_conntrack_info ctinfo,
        struct nf_conntrack_expect *exp)
{
    struct isakmp_data_s *tb;
    struct nf_conntrack_tuple *tuple_temp = NULL;
    struct iphdr *iph =  ip_hdr(skb);
    u_int32_t  *spi = (void *)((char *) iph + iph->ihl * 4);
    int dir = CTINFO2DIR(ctinfo);
    u_int32_t s_addr, d_addr,o_spi;
    o_spi= *spi;

    if(ipsec_flag=='0')
        return NF_ACCEPT;

    s_addr=iph->saddr;
    d_addr=iph->daddr;
    DEBUGP("%d.%d.%d.%d -------> %d.%d.%d.%d, spi=%x\n", NIPQUAD(s_addr), NIPQUAD(d_addr), o_spi);

    if (dir == IP_CT_DIR_ORIGINAL)
    {	// original
        DEBUGP("original\n");
        //printk("ori, %x, %x\n",
        // (int)(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all),
        // (int)(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all));
        //printk("rep, %x, %x\n",
        // (int)(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.all),
        // (int)(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.all));
        DEBUGP("findEspOut src %d.%d.%d.%d, dst %d.%d.%d.%d\n", NIPQUAD(ct->tuplehash[dir].tuple.src.ip), NIPQUAD(d_addr));
        tb = _findEspOut(ct->tuplehash[dir].tuple.src.u3.ip, d_addr, o_spi );
        if(tb==NULL)
        {
            tb=_addEsp(ct->tuplehash[dir].tuple.src.u3.ip,d_addr,o_spi);
            if(tb != NULL)
            {
                // original SA been changed
                if (tb->pctrack != ct){
                    DEBUGP("original SA been changed, update it !\n");
                    tb->pctrack = ct;
                }
                return NF_ACCEPT;
            }
            else
            {
                goto reply;
                // maybe it's reply
                //printk("can not bind to session on original\n");
                //				return NF_DROP;
            }
        }
        else
            return NF_ACCEPT;
    }

    else // reply
    {
reply:
        DEBUGP("reply\n");
        DEBUGP("findEspIn src %d.%d.%d.%d, dst %d.%d.%d.%d\n", NIPQUAD(ct->tuplehash[dir].tuple.src.u3.ip), NIPQUAD(ct->tuplehash[dir].tuple.dst.u3.ip));
        tb = _findEspIn(ct->tuplehash[dir].tuple.src.u3.ip, ct->tuplehash[dir].tuple.dst.u3.ip, o_spi);
        if(tb!=NULL)
        {
            if (tb->pctrack != ct)
            {
                DEBUGP("a new ct, reply should have been refreshed\n");
                DEBUGP("modify ip from %d.%d.%d.%d to %d.%d.%d.%d\n", NIPQUAD(iph->daddr), NIPQUAD(tb->local_ip));
                skb->nfctinfo = IP_CT_RELATED;
                iph->daddr=tb->local_ip;
                iph->check=0;
                iph->check=ip_fast_csum((unsigned char *)iph, iph->ihl);

                //lyl add the solve the first packet lose
                tuple_temp = &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
                tuple_temp->src.u.all += 1234;
                tuple_temp->dst.u.all += 1234;
                tuple_temp = &ct->tuplehash[IP_CT_DIR_REPLY].tuple;
                tuple_temp->src.u.all += 1234;
                tuple_temp->dst.u.all += 1234;
            }

            DEBUGP("reply to localip=%x\n", tb->local_ip);
        }
        else
        {
            DEBUGP("can not bind to session on reply, drop it!\n");
            return NF_DROP;
        }
    }

    return NF_ACCEPT;
}


static unsigned int in_help(struct sk_buff *skb,
        struct nf_conn *ct,
        enum ip_conntrack_info ctinfo,
        struct nf_conntrack_expect *exp)
{
    struct iphdr *iph =  ip_hdr(skb);
    struct udphdr *udph = (void *)((char *) iph + iph->ihl * 4);
    int dir = CTINFO2DIR(ctinfo);
    u_int32_t s_addr, d_addr;
    u_int32_t *pu32;
    u_int64_t *dptr,icookie, rcookie;
    struct isakmp_data_s *tb;

    if(ipsec_flag=='0')
        return NF_ACCEPT;

    DEBUGP("-------------  In-bound helper  -------------\n");
    s_addr=iph->saddr;
    d_addr=iph->daddr;
    dptr = (u_int64_t *) ((void *) udph + sizeof(struct udphdr));
    icookie= dptr[0];
    rcookie= dptr[1];
    DEBUGP("s_addr=%x, d_addr=%x ", s_addr, d_addr);
    pu32 = (u_int32_t *)&icookie;
    DEBUGP("icookie=%x, %x, \n", pu32[0], pu32[1]);
    pu32 = (u_int32_t *)&rcookie;
    DEBUGP("rcookie=%x, %x\n", pu32[0], pu32[1]);


    if( rcookie!=0 || icookie==0)
    {
        if (ctinfo >= IP_CT_IS_REPLY)
        {
            DEBUGP("This is REPLY dir packet, finding in IN way\n");
            // It's reply
            tb = _findIsakmpIn(ct->tuplehash[dir].tuple.src.u3.ip, ct->tuplehash[dir].tuple.dst.u3.ip, icookie, rcookie);
            if(tb!=NULL)
            {
                if(iph->daddr != tb->local_ip) {
                    DEBUGP("change ip-daddr %d.%d.%d.%d to %d.%d.%d.%dn", NIPQUAD(iph->daddr),NIPQUAD( tb->local_ip));
                    iph->daddr=tb->local_ip;
                }

                iph->check=0;
                iph->check=ip_fast_csum((unsigned char *)iph, iph->ihl);
                udph->check=0;
                udph->check=csum_partial((char *)udph,ntohs(udph->len),0);
                udph->check=csum_tcpudp_magic(iph->saddr,iph->daddr ,ntohs(udph->len),IPPROTO_UDP,udph->check);

                DEBUGP("New local:%d.%d.%d.%d  IPcheck=%x UDPcheck=%x\n",NIPQUAD(d_addr),iph->check,udph->check);
            }
        }
        else	// It's original
        {
            DEBUGP("This is ORIGINAL dir packet, finding in OUT way\n");
            tb = _findIsakmpOut(ct->tuplehash[dir].tuple.src.u3.ip, d_addr, icookie, rcookie);

            // Mason Yu IKE Bug
            //if (tb == NULL)
            if (tb == NULL && icookie!=0)
            {
                DEBUGP("Not found IKE session, drop it\n");
                return NF_DROP;
            }

        }
    }

    return NF_ACCEPT;
}

static unsigned int out_help(struct sk_buff *skb,
        struct nf_conn *ct,
        enum ip_conntrack_info ctinfo,
        struct nf_conntrack_expect *exp)
{
    struct iphdr *iph =  ip_hdr(skb);
    struct udphdr *udph = (void *)((char *) iph + iph->ihl * 4);
    int dir = CTINFO2DIR(ctinfo);
    u_int32_t s_addr, d_addr;
    u_int32_t *pu32;
    u_int64_t *dptr,icookie, rcookie;
    struct isakmp_data_s *tb;

    if(ipsec_flag=='0')
        return NF_ACCEPT;


    DEBUGP("-------------- out-bound helper -----------------\n");
    s_addr=iph->saddr;
    d_addr=iph->daddr;
    dptr = (u_int64_t *) ((void *) udph + sizeof(struct udphdr));
    icookie= dptr[0];
    rcookie= dptr[1];
    DEBUGP("s_addr=%d.%d.%d.%d, d_addr=%d.%d.%d.%d ", NIPQUAD(s_addr), NIPQUAD(d_addr));
    pu32 = (u_int32_t *)&icookie;
    DEBUGP("icookie=%x, %x, \n", pu32[0], pu32[1]);
    pu32 = (u_int32_t *)&rcookie;
    DEBUGP("rcookie=%x, %x\n", pu32[0], pu32[1]);


    // lyl add this, i think it is not harm
    if (dir == IP_CT_DIR_ORIGINAL)
    {
        if( rcookie==0 || icookie==0)
        {
            DEBUGP("This is ORIGINAL dir packet, finding in OUT way\n");
            tb = _findIsakmpOut(ct->tuplehash[dir].tuple.src.u3.ip, d_addr, icookie, rcookie);

            if(tb == NULL)
            {
                _addIsakmp(ct->tuplehash[dir].tuple.src.u3.ip,d_addr,s_addr,icookie,rcookie);
                return NF_ACCEPT;
            }
            else
            {
                //printk("drop the very first IKE\n");
                // Mason Yu IKE Bug
                //return NF_DROP;	// first packet, should not be found
            }
        }
    }  //end dir == IP_CT_DIR_ORIGINA

    /*lyl: we must add this ,because the second PC ike packet arrive at server likes (sport:1,dport:500),but the server will
      return (sport:500, dport:500), the packet will be recognize the first PC conntrack, and will SNAT to the first
      conntrack, so ,we have to direct to the second conntrack mannuely. and as the help is after NAt, so we can't
      compair dst ip, for it has change to the first PC ip.
      */
#if 1
    if (dir == IP_CT_DIR_REPLY) {
        DEBUGP("This is REPLY dir packet, finding in IN way\n");
        // It's reply
        tb = _findIsakmpIn(ct->tuplehash[dir].tuple.src.u3.ip, ct->tuplehash[dir].tuple.dst.u3.ip, icookie, rcookie);
        if(tb!=NULL)
        {
            if(iph->daddr != tb->local_ip) {

                DEBUGP("change ip-daddr %d.%d.%d.%d to %d.%d.%d.%dn", NIPQUAD(iph->daddr),NIPQUAD( tb->local_ip));
                iph->daddr=tb->local_ip;
            }

            iph->check=0;
            iph->check=ip_fast_csum((unsigned char *)iph, iph->ihl);
            udph->check=0;
            udph->check=csum_partial((char *)udph,ntohs(udph->len),0);
            udph->check=csum_tcpudp_magic(iph->saddr,iph->daddr ,ntohs(udph->len),IPPROTO_UDP,udph->check);

            DEBUGP("New local:%d.%d.%d.%d  IPcheck=%x UDPcheck=%x\n",NIPQUAD(d_addr),iph->check,udph->check);
        }
    }
#endif

    return NF_ACCEPT;
}


static void check_timeout(unsigned long data)
{
    struct isakmp_data_s *tp = _isakmpDb;
    int i;

    for(i=0; i < MaxSession ;i++)
    {
        if(tp == NULL || _isakmpDb == NULL)
            break;
        if(tp->state == IPSEC_FREE)
        {
            tp++;
            continue;
        }
        tp->idle_timer++;
        if(tp->idle_timer > IPSEC_IDLE_TIME)
        {
            tp->state = IPSEC_FREE;
        }
        tp++;
    }

    ipsec_time.expires=jiffies + 100;
    add_timer(&ipsec_time);
}


/* This function is intentionally _NOT_ defined as  __exit, because
 * it is needed by init() */
static void __exit nf_nat_ipsec_fini(void)
{
    rcu_assign_pointer(nf_nat_ipsec_inbound_hook, NULL);
    rcu_assign_pointer(nf_nat_ipsec_outbound_hook, NULL);
    rcu_assign_pointer(nf_nat_esp_hook, NULL);

    /* Make sure noone calls it, meanwhile. */
    synchronize_net();

    /* del timer */
    del_timer(&ipsec_time);
}

static int __init nf_nat_ipsec_init(void)
{
    int ret = 0;

    BUG_ON(nf_nat_ipsec_inbound_hook);
    rcu_assign_pointer(nf_nat_ipsec_inbound_hook, in_help);

    BUG_ON(nf_nat_ipsec_outbound_hook);
    rcu_assign_pointer(nf_nat_ipsec_outbound_hook, out_help);

    BUG_ON(nf_nat_esp_hook);
    rcu_assign_pointer(nf_nat_esp_hook, esp_help);

    _isakmpDb = kmalloc(MaxSession * (sizeof(struct isakmp_data_s)), GFP_KERNEL);
    memset(_isakmpDb, 0, MaxSession * (sizeof(struct isakmp_data_s)));

    init_timer(&ipsec_time);
    ipsec_time.expires = jiffies + 100;
    ipsec_time.data = (unsigned long)_isakmpDb;
    ipsec_time.function = &check_timeout;
    add_timer(&ipsec_time);

    printk("nf_nat_ipsec loaded\n");

    return ret;
}

module_init(nf_nat_ipsec_init);
module_exit(nf_nat_ipsec_fini);

MODULE_LICENSE("GPL");
