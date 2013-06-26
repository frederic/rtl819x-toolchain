#include <linux/types.h>
#include <linux/netfilter.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/moduleparam.h>
#include <linux/notifier.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/inetdevice.h>

#include <net/ip_fib.h>
#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_nat_protocol.h>
#include <net/ip_vs.h>
#include <linux/ip_vs.h>

#ifdef CONFIG_BRIDGE
#include <bridge/br_private.h>
#endif

#include <net/rtl/rtl_types.h>
#include <net/rtl/fastpath/fastpath_core.h>
#include <net/rtl/features/rtl_features.h>
#include <net/rtl/features/rtl_ps_hooks.h>

#ifdef CONFIG_RTL_LAYERED_DRIVER_L3
#include <net/rtl/rtl865x_ppp.h>
#include <net/rtl/rtl865x_route_api.h>
#include <net/rtl/rtl865x_arp_api.h>
#endif

#ifdef CONFIG_RTL_LAYERED_DRIVER_L2
#include <net/rtl/rtl865x_fdb_api.h>
#endif

#if defined(CONFIG_RTL_HW_QOS_SUPPORT) && defined(CONFIG_RTL_HARDWARE_NAT)
#include <net/rtl/rtl865x_netif.h>
#include <net/rtl/rtl865x_outputQueue.h>
#endif

#if defined (CONFIG_RTL865X_LANPORT_RESTRICTION)
#include <net/rtl/features/lan_restrict.h>
#endif

#ifdef CONFIG_RTL_HARDWARE_NAT
//#define CONFIG_HARDWARE_NAT_DEBUG
#ifdef CONFIG_RTL_LAYERED_DRIVER_L3
#include <net/rtl/rtl865x_ppp.h>
#include <net/rtl/rtl865x_route_api.h>
#endif
//unsigned int ldst, lmask, wdst, wmask;
extern int gHwNatEnabled;
#endif

#ifdef CONFIG_RTL_HARDWARE_NAT
/*2007-12-19*/
#ifdef CONFIG_RTL_LAYERED_DRIVER_L3
#include <net/rtl/rtl865x_ip_api.h>
#endif
#ifdef CONFIG_RTL_LAYERED_DRIVER_L4
#include <net/rtl/rtl865x_nat.h>
#endif
extern char masq_if[];
extern unsigned int hw_napt_ip;
#endif

#if defined(CONFIG_RTL_IPTABLES_FAST_PATH) && defined(CONFIG_FAST_PATH_MODULE)
enum LR_RESULT (*FastPath_hook5)( ipaddr_t ip, ether_addr_t* mac, enum ARP_FLAGS flags )=NULL;
enum LR_RESULT (*FastPath_hook7)( ipaddr_t ip )=NULL;
enum LR_RESULT (*FastPath_hook8)( ipaddr_t ip, ether_addr_t* mac, enum ARP_FLAGS flags )=NULL;
EXPORT_SYMBOL(FastPath_hook8);
EXPORT_SYMBOL(FastPath_hook7);
EXPORT_SYMBOL(FastPath_hook5);
#endif

int32 rtl_nf_conntrack_in_hooks(rtl_nf_conntrack_inso_s *info)
{
#if defined(CONFIG_RTL_IPTABLES_FAST_PATH)
	#if !defined(IMPROVE_QOS)
			rtl_fpAddConnCache(info->ct, info->skb);
	#elif defined(CONFIG_RTL_ROUTER_FAST_PATH)
			if(routerTypeFlag == 1)
				rtl_fpAddConnCache(info->ct, info->skb);
	#endif
#endif
	return RTL_PS_HOOKS_CONTINUE;
}

int32 rtl_nf_conntrack_death_by_timeout_hooks(rtl_nf_conntrack_inso_s *info)
{
	int	ret;

	ret = RTL_PS_HOOKS_CONTINUE;
	#if defined(CONFIG_RTL_IPTABLES_FAST_PATH) || defined(CONFIG_RTL_HARDWARE_NAT)
	if (rtl_connCache_timer_update(info->ct)==SUCCESS) {
		ret = RTL_PS_HOOKS_RETURN;
	}
	#endif

	return ret;
}

int32 rtl_nf_conntrack_destroy_hooks(rtl_nf_conntrack_inso_s *info)
{
	#if defined(CONFIG_RTL_IPTABLES_FAST_PATH) || defined(CONFIG_RTL_HARDWARE_NAT)
	rtl_delConnCache(info->ct);
	#endif

	return RTL_PS_HOOKS_CONTINUE;
}

int32 rtl_nf_conntrack_confirm_hooks(rtl_nf_conntrack_inso_s *info)
{
	#if defined(CONFIG_RTL_NF_CONNTRACK_GARBAGE_NEW)
	rtl_connGC_addList((void*)info->skb, (void*)info->ct);
	#endif

	return RTL_PS_HOOKS_CONTINUE;
}

int32 rtl_nf_init_conntrack_hooks(rtl_nf_conntrack_inso_s *info)
{

	#if defined(CONFIG_RTL_NF_CONNTRACK_GARBAGE_NEW)
	INIT_LIST_HEAD(&info->ct->state_tuple);
	#endif
	return RTL_PS_HOOKS_CONTINUE;
}


int32 rtl_nf_conntrack_init_hooks(void)
{
	#if defined(CONFIG_RTL_NF_CONNTRACK_GARBAGE_NEW)
	rtl_nf_conn_GC_init();
	#endif
	return RTL_PS_HOOKS_CONTINUE;
}

int32 rtl_tcp_packet_hooks(rtl_nf_conntrack_inso_s *info)
{
	#if defined(CONFIG_RTL_HARDWARE_NAT)
	if (info->new_state==TCP_CONNTRACK_LAST_ACK) {
		rtl865x_handle_nat(info->ct, 0, NULL);
	}
	#endif

	return RTL_PS_HOOKS_CONTINUE;
}

int32 rtl_nf_nat_packet_hooks(rtl_nf_conntrack_inso_s *info)
{
	#if (defined(IMPROVE_QOS) && defined(CONFIG_RTL_IPTABLES_FAST_PATH)) || defined(CONFIG_RTL_HARDWARE_NAT)
	rtl_addConnCache(info->ct, info->skb);
	#endif

	return RTL_PS_HOOKS_CONTINUE;
}

#ifdef CONFIG_RTL_WLAN_DOS_FILTER
#define RTL_WLAN_NAME "wlan"
#define TCP_SYN 2
#define _MAX_SYN_THRESHOLD 		400
#define _WLAN_BLOCK_TIME			20	// unit: seconds

int wlan_syn_cnt=0;
int wlan_block=0, wlan_block_count=0;
unsigned int dbg_wlan_dos_block_pkt_num=0;
unsigned char block_source_mac[6];
int wlan_dos_filter_enabled = 1;

extern unsigned int _br0_ip;

#if defined(CONFIG_RTL8192CD) || defined(CONFIG_RTL8192E)
extern int issue_disassoc_from_kernel(void *priv, unsigned char *mac);
#endif

static struct timer_list wlan_dos_timer;
static void wlan_dos_timer_fn(unsigned long arg)
{	
	wlan_syn_cnt = 0;

	if(wlan_block_count >=_WLAN_BLOCK_TIME) {		
		wlan_block=0;
		wlan_block_count=0;
	}
	if(wlan_block == 1)
		wlan_block_count++;
	
      	mod_timer(&wlan_dos_timer, jiffies + HZ);
}

#endif

int32 rtl_nat_init_hooks(void)
{
#ifdef CONFIG_RTL_819X_SWCORE
	rtl_nat_init();
#endif
	#if defined(CONFIG_NET_SCHED)
	rtl_qos_init();
	#endif

#ifdef CONFIG_RTL_WLAN_DOS_FILTER
      init_timer(&wlan_dos_timer);
      wlan_dos_timer.expires  = jiffies + HZ;
      wlan_dos_timer.data     = 0L;
      wlan_dos_timer.function = wlan_dos_timer_fn;
      mod_timer(&wlan_dos_timer, jiffies + HZ);
	#endif

	return RTL_PS_HOOKS_CONTINUE;
}

int32 rtl_nat_cleanup_hooks(void)
{
	#if defined(CONFIG_NET_SCHED)
	rtl_qos_cleanup();
	#endif

	return RTL_PS_HOOKS_CONTINUE;
}

int32 rtl_fn_hash_insert_hooks(struct fib_table *tb, struct fib_config *cfg, struct fib_info *fi)
{
#if defined( CONFIG_RTL_IPTABLES_FAST_PATH)
	#if defined(CONFIG_FAST_PATH_MODULE)
	if(FastPath_hook3!=NULL)
	{
		FastPath_hook3(cfg->fc_dst? cfg->fc_dst: 0, inet_make_mask(cfg->fc_dst_len),cfg->fc_gw ? cfg->fc_gw : 0, (uint8 *)fi->fib_dev->name, RT_NONE);
	}
	#else
	rtk_addRoute(cfg->fc_dst? cfg->fc_dst: 0, inet_make_mask(cfg->fc_dst_len),cfg->fc_gw ? cfg->fc_gw : 0, (uint8 *)fi->fib_dev->name, RT_NONE);
	#endif
#endif


	#if defined(CONFIG_RTL_HARDWARE_NAT)
	rtl_fn_insert(tb, cfg, fi);
	#endif

	return RTL_PS_HOOKS_CONTINUE;
}

int32 rtl_fn_hash_delete_hooks(struct fib_table *tb, struct fib_config *cfg)
{
	#if defined(CONFIG_RTK_IPTABLES_FAST_PATH)
	#ifdef CONFIG_FAST_PATH_MODULE
	if(FastPath_hook1!=NULL)
	{
		FastPath_hook1(cfg->fc_dst? cfg->fc_dst : 0, inet_make_mask(cfg->fc_dst_len));
	}
	#else
	rtk_delRoute(cfg->fc_dst? cfg->fc_dst : 0, inet_make_mask(cfg->fc_dst_len));
	#endif
	#endif

	#if defined(CONFIG_RTL_HARDWARE_NAT)
	rtl_fn_delete(tb, cfg);
	#endif

	return RTL_PS_HOOKS_CONTINUE;
}


int32 rtl_fn_flush_list_hooks(int	 fz_order, int idx, u32 tb_id, u32 fn_key)
{
	#if defined(CONFIG_RTL_HARDWARE_NAT)
	rtl_fn_flush(fz_order, idx, tb_id, fn_key);
	#endif

	return RTL_PS_HOOKS_CONTINUE;
}

int32 rtl_fn_hash_replace_hooks(struct fib_table *tb, struct fib_config *cfg, struct fib_info *fi)
{
#if defined( CONFIG_RTL_IPTABLES_FAST_PATH)
	#if defined(CONFIG_FAST_PATH_MODULE)
	if(FastPath_hook2!=NULL)
	{
		FastPath_hook2(cfg->fc_dst? cfg->fc_dst: 0, inet_make_mask(cfg->fc_dst_len),cfg->fc_gw ? cfg->fc_gw : 0, (uint8 *)fi->fib_dev->name, RT_NONE);
	}
	#else
	rtk_modifyRoute(cfg->fc_dst? cfg->fc_dst: 0, inet_make_mask(cfg->fc_dst_len),cfg->fc_gw ? cfg->fc_gw : 0, (uint8 *)fi->fib_dev->name, RT_NONE);
	#endif
#endif
	return RTL_PS_HOOKS_CONTINUE;
}

int32 rtl_dev_queue_xmit_hooks(struct sk_buff *skb, struct net_device *dev)
{
	#if defined( CONFIG_RTL_IPTABLES_FAST_PATH)
	rtl_fp_dev_queue_xmit_check(skb, dev);
	#endif

	return RTL_PS_HOOKS_CONTINUE;
}

int32 rtl_dev_hard_start_xmit_hooks(struct sk_buff *skb, struct net_device *dev, struct netdev_queue *txq)
{
	#if defined( CONFIG_RTL_IPTABLES_FAST_PATH)
	rtl_fp_dev_hard_start_xmit_check(skb, dev, txq);
	#endif

	return RTL_PS_HOOKS_CONTINUE;
}

#ifdef CONFIG_RTL_WLAN_DOS_FILTER

static int  filter_dos_wlan(struct sk_buff *skb)
{
	struct iphdr *iph;
	struct tcphdr *tcph;	
	unsigned char *tflag;		
	int ret=NF_ACCEPT;
	
	iph=ip_hdr(skb);
	tcph=(void *) iph + iph->ihl*4;
	tflag=(void *) tcph + 13;

     	//wlan_dev=__dev_get_by_name(&init_net,RTL_PS_WLAN0_DEV_NAME);	// wlan0
     	//wlan_dev=__dev_get_by_name(&init_net,RTL_PS_LAN_P0_DEV_NAME);	// eth0

	//if(skb->dev && (skb->dev == wlan_dev))

	if ((skb->dev) &&	(!strncmp(skb->dev->name, RTL_WLAN_NAME, 4)))	// wlan0, wlan1, wlan0-va0, ... and so on
	{
		if ((iph->protocol==IPPROTO_TCP) && ((*tflag & 0x3f)==TCP_SYN) && (iph->daddr == _br0_ip))		// xdos.exe 192.168.1.254 0-65535
		{	
			//if(wlan_block==1 && attack_daddr2==iph->daddr) {
			if ((wlan_block==1) && (memcmp(block_source_mac, &(skb->mac_header[6]), 6) == 0)) {
				dbg_wlan_dos_block_pkt_num++;
				ret = NF_DROP;
			}
			else {
	      			wlan_syn_cnt++;
			
				if(wlan_syn_cnt > _MAX_SYN_THRESHOLD)
				{
					//attack_daddr2=iph->daddr;
					wlan_block=1;

#if defined(CONFIG_RTL8192CD) || defined(CONFIG_RTL8192E)
					issue_disassoc_from_kernel((void *) skb->dev->priv, &(skb->mac_header[6]));
#endif
					memcpy(block_source_mac, &(skb->mac_header[6]), 6);
				}
			}
		}
			
	}

	return (ret);
}

int filter_dos_wlan_enter(struct sk_buff **pskb)
{
	int ret;
	struct sk_buff *skb;

	skb=*pskb;
	skb->transport_header=skb->data;
	skb->network_header = skb->data;

	ret = filter_dos_wlan((void*)skb);
	if (ret == NF_DROP) {
		kfree_skb(skb);
		ret = NET_RX_DROP;
	}
	else {
		ret = NET_RX_SUCCESS;
	}

	return ret;
}
#endif

int32 rtl_netif_receive_skb_hooks(struct sk_buff **pskb)
{
	int	ret;

	#if defined(CONFIG_RTL_FAST_BRIDGE)
	if(skb->dev->br_port && (rtl_fast_br_forwarding(*pskb)==RTL_FAST_BR_SUCCESS)) {
		ret = RTL_PS_HOOKS_RETURN;
	} else
	#endif

	#ifdef CONFIG_RTL_WLAN_DOS_FILTER
	if (wlan_dos_filter_enabled && filter_dos_wlan_enter(pskb)== NET_RX_DROP) {
		ret = RTL_PS_HOOKS_RETURN;
	} else
	#endif
	
	#if defined(CONFIG_RTL_IPTABLES_FAST_PATH)
	if (FastPath_Enter(pskb)== NET_RX_DROP) {
		ret = RTL_PS_HOOKS_RETURN;
	} else
	#endif
	{
		ret = RTL_PS_HOOKS_CONTINUE;
	}

	return ret;
}

int32 rtl_br_dev_queue_push_xmit_before_xmit_hooks(struct sk_buff *skb)
{
	#if defined(CONFIG_RTL_FAST_BRIDGE)
	rtl_fb_add_br_entry(skb);
	#endif

	return RTL_PS_HOOKS_CONTINUE;
}


int32 rtl_neigh_forced_gc_hooks(struct neigh_table *tbl, struct neighbour *n)
{
	#if defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL_LAYERED_DRIVER) && defined(CONFIG_RTL_LAYERED_DRIVER_L3)
	syn_asic_arp(n,  0);
	#endif
	return RTL_PS_HOOKS_CONTINUE;
}

int32 rtl_neigh_flush_dev_hooks(struct neigh_table *tbl, struct net_device *dev, struct neighbour *n)
{
	#if (defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL_LAYERED_DRIVER) && defined(CONFIG_RTL_LAYERED_DRIVER_L3)) || defined(CONFIG_RTL_IPTABLES_FAST_PATH)
	if (n->nud_state & NUD_VALID) {
	#if defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL_LAYERED_DRIVER) && defined(CONFIG_RTL_LAYERED_DRIVER_L3)
	syn_asic_arp(n,  0);
	#endif

	#if defined(CONFIG_RTL_IPTABLES_FAST_PATH)
		#ifdef CONFIG_FAST_PATH_MODULE
		if(FastPath_hook7!=NULL) {
			FastPath_hook7(*(u32*)n->primary_key);
		}
		#else
		rtk_delArp(*(u32*)n->primary_key);
		#endif
	#endif
	}
	#endif

	return RTL_PS_HOOKS_CONTINUE;
}

int32 rtl_neigh_destroy_hooks(struct neighbour *n)
{
	#if defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL_LAYERED_DRIVER) && defined(CONFIG_RTL_LAYERED_DRIVER_L3)
	 syn_asic_arp(n,  0);
	#endif
	return RTL_PS_HOOKS_CONTINUE;
}

int32 rtl_neigh_connect_hooks(struct neighbour *neigh)
{
	#if defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL_LAYERED_DRIVER) && defined(CONFIG_RTL_LAYERED_DRIVER_L3)
	if (neigh->nud_state & NUD_REACHABLE) {
		syn_asic_arp(neigh,  1);
	}
	#endif
	return RTL_PS_HOOKS_CONTINUE;
}

int32 rtl_neigh_update_hooks(struct neighbour *neigh, const u8 *lladdr, uint8 old)
{
	#if defined(CONFIG_RTL_IPTABLES_FAST_PATH)
	if (old & NUD_VALID) {
		#ifdef CONFIG_FAST_PATH_MODULE
		if(FastPath_hook8!=NULL) {
			FastPath_hook8(*(u32*)neigh->primary_key, (ether_addr_t*)lladdr, ARP_NONE);
		}
		#else
		rtk_modifyArp(*(u32*)neigh->primary_key, (ether_addr_t*)lladdr, ARP_NONE);
		#endif
	}
	#endif

	#if defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL_LAYERED_DRIVER) && defined(CONFIG_RTL_LAYERED_DRIVER_L3)
	syn_asic_arp(neigh,  1);
	#endif
	return RTL_PS_HOOKS_CONTINUE;
}

int32 rtl_neigh_update_post_hooks(struct neighbour *neigh, const u8 *lladdr, uint8 old)
{
	#if defined(CONFIG_RTL_IPTABLES_FAST_PATH)
	if ((neigh->nud_state & NUD_VALID) && !(old & NUD_VALID)) {
		#if	defined(CONFIG_FAST_PATH_MODULE)
		if(FastPath_hook5!=NULL)
		{
			FastPath_hook5(*(u32*)neigh->primary_key, (ether_addr_t*)neigh->ha, ARP_NONE);
		}
		#else
		rtk_addArp(*(u32*)neigh->primary_key, (ether_addr_t*)neigh->ha, ARP_NONE);
		#endif
	} else if ((old & NUD_VALID) && !(neigh->nud_state & NUD_VALID)) {
		#if	defined(CONFIG_FAST_PATH_MODULE)
		if(FastPath_hook7!=NULL)
		{
			FastPath_hook7(*(u32*)neigh->primary_key);
		}
		#else
		rtk_delArp(*(u32*)neigh->primary_key);
		#endif
	}
	#endif

	return RTL_PS_HOOKS_CONTINUE;
}

static __inline__ int neigh_max_probes(struct neighbour *n)
{
	struct neigh_parms *p = n->parms;
	return (n->nud_state & NUD_PROBE ?
		p->ucast_probes :
		p->ucast_probes + p->app_probes + p->mcast_probes);
}

int32  rtl_neigh_periodic_timer_hooks(struct neighbour *n,  unsigned int  refresh)
{
	int	ret;

	if (!(n->nud_state & NUD_VALID))
		return RTL_PS_HOOKS_CONTINUE;

	ret = RTL_PS_HOOKS_CONTINUE;
	#if defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL_LAYERED_DRIVER)  && defined(CONFIG_RTL_LAYERED_DRIVER_L3)
	if (rtl865x_arpSync(htonl(*((u32 *)n->primary_key)), refresh)>0) {
		n->used = jiffies;
		n->dead=0;
		ret = RTL_PS_HOOKS_BREAK;
	}
	else
	#endif

	#if defined(CONFIG_RTL_IPTABLES_FAST_PATH)
	{
		#ifdef CONFIG_FAST_PATH_MODULE
		if(FastPath_hook7!=NULL)
		{
			FastPath_hook7(*(u32*)n->primary_key);
		}
		#else
		rtk_delArp(*(u32*)n->primary_key);
		#endif
	}
	#endif

	return ret;
}

int32 rtl_neigh_timer_handler_pre_update_hooks(struct neighbour *neigh, unsigned state)
{
	#if defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL_LAYERED_DRIVER)  && defined(CONFIG_RTL_LAYERED_DRIVER_L3)
	int32	tval;
	#endif

	if (state & NUD_REACHABLE) {
		#if defined(CONFIG_RTL_IPTABLES_FAST_PATH)
		if (neigh->nud_state & NUD_VALID) {
			#ifdef CONFIG_FAST_PATH_MODULE
			if(FastPath_hook7!=NULL)
			{
				FastPath_hook7(*(u32*)neigh->primary_key);
			}
			#else
			rtk_delArp(*(u32*)neigh->primary_key);
			#endif
		}
		#endif

		#if defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL_LAYERED_DRIVER)  && defined(CONFIG_RTL_LAYERED_DRIVER_L3)
		tval = rtl865x_arpSync(htonl(*((u32 *)neigh->primary_key)), 0);
		if (tval > 0)
		{
			neigh->confirmed = jiffies;
		}
		#if 0
		printk("%s:%d: ip:%u.%u.%u.%u, mac:%x:%x:%x:%x:%x:%x, tval is %d\n",
		__FUNCTION__,__LINE__,NIPQUAD(htonl(*((u32 *)neigh->primary_key))), neigh->ha[0], neigh->ha[1],
		neigh->ha[2], neigh->ha[3], neigh->ha[4], neigh->ha[5],tval);
		#endif
		#endif

	} else if (state & NUD_DELAY) {
		#if defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL_LAYERED_DRIVER)  && defined(CONFIG_RTL_LAYERED_DRIVER_L3)
		tval = rtl865x_arpSync(htonl(*((u32 *)neigh->primary_key)), 0);
		if (tval > 0)
		{
			neigh->confirmed = jiffies;
		}
		#if 0
		printk("%s:%d: ip:%u.%u.%u.%u, mac:%x:%x:%x:%x:%x:%x, tval is %d",
		__FUNCTION__,__LINE__,NIPQUAD(htonl(*((u32 *)neigh->primary_key))), neigh->ha[0], neigh->ha[1],
		neigh->ha[2], neigh->ha[3], neigh->ha[4], neigh->ha[5],tval);
		#endif
		#endif
	}

	return RTL_PS_HOOKS_CONTINUE;
}

int32 rtl_neigh_timer_handler_during_update_hooks(struct neighbour *neigh, unsigned state)
{
	#if defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL_LAYERED_DRIVER)  && defined(CONFIG_RTL_LAYERED_DRIVER_L3)
	if ((neigh->nud_state & (NUD_INCOMPLETE | NUD_PROBE)) &&
	    atomic_read(&neigh->probes) >= neigh_max_probes(neigh)) {
		if (neigh->nud_state & NUD_VALID) {
			/*delete asic arp entry*/
			syn_asic_arp(neigh, 0);
			#if 0
			printk("%s:%d: ip:%u.%u.%u.%u, mac:%x:%x:%x:%x:%x:%x\n",
			__FUNCTION__,__LINE__,NIPQUAD(htonl(*((u32 *)neigh->primary_key))), neigh->ha[0], neigh->ha[1],
			neigh->ha[2], neigh->ha[3], neigh->ha[4],neigh->ha[5]);
			#endif
		}
	}
	#endif
	return RTL_PS_HOOKS_CONTINUE;
}

int32 rtl_neigh_timer_handler_post_update_hooks(struct neighbour *neigh, unsigned state)
{
	#if defined(CONFIG_RTL_IPTABLES_FAST_PATH)
	if ((neigh->nud_state & NUD_VALID) && !(state & NUD_VALID)) {
		#ifdef CONFIG_FAST_PATH_MODULE
		if(FastPath_hook5!=NULL)
		{
			FastPath_hook5(*(u32*)neigh->primary_key, (ether_addr_t*)neigh->ha, ARP_NONE);
		}
		#else
		rtk_addArp(*(u32*)neigh->primary_key, (ether_addr_t*)neigh->ha, ARP_NONE);
		#endif
	} else if ((state & NUD_VALID) && !(neigh->nud_state & NUD_VALID)) {
		#ifdef CONFIG_FAST_PATH_MODULE
		if(FastPath_hook7!=NULL)
		{
			FastPath_hook7(*(u32*)neigh->primary_key);
		}
		#else
		rtk_delArp(*(u32*)neigh->primary_key);
		#endif
	}
	#endif
	return RTL_PS_HOOKS_CONTINUE;
}

int32 rtl___neigh_event_send_pre_hooks(struct neighbour *neigh, struct sk_buff *skb)
{
	#if defined(CONFIG_RTL_IPTABLES_FAST_PATH)
	if (!(neigh->nud_state & (NUD_STALE | NUD_INCOMPLETE))) {
		if (neigh->nud_state & NUD_VALID) {
			#if	defined(CONFIG_FAST_PATH_MODULE)
			if(FastPath_hook7!=NULL) {
				FastPath_hook7(*(u32*)neigh->primary_key);
			}
			#else
			rtk_delArp(*(u32*)neigh->primary_key);
			#endif
		}
	}
	#endif
	return RTL_PS_HOOKS_CONTINUE;
}

int32 rtl___neigh_event_send_post_hooks(struct neighbour *neigh, struct sk_buff *skb)
{
	return RTL_PS_HOOKS_CONTINUE;
}

int32 rtl_neigh_init_hooks(void)
{
	#if defined(CONFIG_RTL_MULTIPLE_WAN)
	rtl_set_callback_for_ps_arp(rtl_get_ps_arp_mapping);
	#endif
	return RTL_PS_HOOKS_CONTINUE;
}

#if defined(CONFIG_BRIDGE)
int32 rtl___br_fdb_get_timeout_hooks(struct net_bridge *br, struct net_bridge_fdb_entry *fdb, const unsigned char *addr)
{
	#if defined(CONFIG_RTL_IPTABLES_FAST_PATH) || defined(CONFIG_RTL_FASTBRIDGE)
	if (rtl_br_fdb_time_update((void*)br, (void*)fdb, addr)==FAILED) {
		return RTL_PS_HOOKS_BREAK;
	}
	#endif

	return RTL_PS_HOOKS_CONTINUE;
}
#endif

int32 rtl_masq_device_event_hooks(struct notifier_block *this, struct net_device *dev,  unsigned long event)
{
	#if defined(CONFIG_RTL_HARDWARE_NAT)
	if (dev!=NULL && dev->ip_ptr!=NULL)
		rtl_update_ip_tables(dev->name, event, (struct in_ifaddr *)(((struct in_device *)(dev->ip_ptr))->ifa_list));
	#endif

	return RTL_PS_HOOKS_CONTINUE;
}


 int32 rtl_masq_inet_event_hooks(struct notifier_block *this, unsigned long event, void *ptr)
 {
	#if defined(CONFIG_RTL_HARDWARE_NAT)
	struct in_ifaddr *ina;

	ina = (struct in_ifaddr*)ptr;

	#ifdef CONFIG_HARDWARE_NAT_DEBUG
	/*2007-12-19*/
	printk("%s:%d\n",__FUNCTION__,__LINE__);
	printk("ptr->ifa_dev->dev->name is %s\n", ina->ifa_dev->dev->name);
	#endif

	rtl_update_ip_tables(ina->ifa_dev->dev->name, event, ina);
	#endif

	return RTL_PS_HOOKS_CONTINUE;
 }


int32 rtl_translate_table_hooks(const char *name,
		unsigned int valid_hooks,
		struct xt_table_info *newinfo,
		void *entry0,
		unsigned int size,
		unsigned int number,
		const unsigned int *hook_entries,
		const unsigned int *underflows)
{
#if defined(CONFIG_RTL_HARDWARE_NAT)
	//hyking:check masquerade and add ip
	if(strcmp(name,"nat") == 0)
	{
		rtl_flush_extern_ip();
		rtl_init_masq_info();
		rtl_check_for_extern_ip(name,valid_hooks,newinfo,entry0,size,number,hook_entries,underflows);
	}
#endif

	return RTL_PS_HOOKS_CONTINUE;
}

int32 rtl_ip_tables_init_hooks(void)
{
	#if defined(CONFIG_RTL_HARDWARE_NAT)
	rtl_init_masq_info();
	#endif

	return RTL_PS_HOOKS_CONTINUE;
}


int32 rtl_ip_vs_conn_expire_hooks1(struct ip_vs_conn *cp)
{
	int32	ret;

	ret = RTL_PS_HOOKS_CONTINUE;
	#if defined(CONFIG_RTL_HARDWARE_NAT)
	if (FAILED==rtl_ip_vs_conn_expire_check(cp)) {
		ret = RTL_PS_HOOKS_RETURN;
	}
	#endif

	return ret;
}

int32 rtl_ip_vs_conn_expire_hooks2(struct ip_vs_conn *cp)
{
	#if defined(CONFIG_RTL_HARDWARE_NAT)
	rtl_ip_vs_conn_expire_check_delete(cp);
	#endif

	return RTL_PS_HOOKS_CONTINUE;
}

#if defined(CONFIG_IP_VS_PROTO_TCP)
int32 rtl_tcp_state_transition_hooks(struct ip_vs_conn *cp, int direction, const struct sk_buff *skb, struct ip_vs_protocol *pp)
{
	#if defined(CONFIG_RTL_HARDWARE_NAT)
	rtl_tcp_state_transition_check(cp, direction, skb, pp);
	#endif

	return RTL_PS_HOOKS_CONTINUE;
}
#endif

#if defined(CONFIG_IP_VS_PROTO_UDP)
int32 rtl_udp_state_transition_hooks(struct ip_vs_conn *cp, int direction, const struct sk_buff *skb, struct ip_vs_protocol *pp)
{
	#if defined(CONFIG_RTL_HARDWARE_NAT)
	rtl_udp_state_transition_check(cp, direction, skb, pp);
	#endif

	return RTL_PS_HOOKS_CONTINUE;
}
#endif

#ifdef CONFIG_PROC_FS
int rtl_ct_seq_show_hooks(struct seq_file *s, struct nf_conn *ct)
{
	//hyking add for hw use
	#if defined(CONFIG_RTL_HARDWARE_NAT)
	struct nf_conn_nat *nat;
	char *state[2]={"Hardware","software"};

	nat = nfct_nat(ct);
	if(seq_printf(s,"[%s] ",nat->hw_acc?state[0]:state[1]) != 0)
		return RTL_PS_HOOKS_BREAK;
	#endif
	return RTL_PS_HOOKS_CONTINUE;
}
#endif

#if defined(CONFIG_RTL_NF_CONNTRACK_GARBAGE_NEW)
//When do garbage collection and dst cache overflow, should not do garbage collection for rtl_gc_overflow_timout(default 3s)
//Note: rtl_gc_overflow_timout can be modified by user via /proc/gc_overflow_timout
unsigned long rtl_gc_overflow_timout=3*HZ;
static unsigned long rtl_gc_overflow_timeout=0;
#endif

//Before do garbage collection, do rtl_dst_alloc_gc_pre_check_hooks
int32 rtl_dst_alloc_gc_pre_check_hooks(struct dst_ops * ops)
{
#if defined(CONFIG_RTL_NF_CONNTRACK_GARBAGE_NEW)
	if((rtl_gc_overflow_timeout > 0)&& time_after_eq(rtl_gc_overflow_timeout, jiffies)){
		return RTL_PS_HOOKS_RETURN;
	}
#endif

	return RTL_PS_HOOKS_CONTINUE;
}

//After do garbage collection and dst cache overflow, do rtl_dst_alloc_gc_post_check1_hooks
int32 rtl_dst_alloc_gc_post_check1_hooks(struct dst_ops * ops)
{
#if defined(CONFIG_RTL_NF_CONNTRACK_GARBAGE_NEW)
	rtl_gc_overflow_timeout=jiffies+rtl_gc_overflow_timout;
#endif

	return RTL_PS_HOOKS_CONTINUE;
}

//After [do garbage collection and dst cache not overflow] and dst alloc success, do rtl_dst_alloc_gc_post_check2_hooks
int32 rtl_dst_alloc_gc_post_check2_hooks(struct dst_ops * ops, struct dst_entry * dst)
{
#if defined(CONFIG_RTL_NF_CONNTRACK_GARBAGE_NEW)
	rtl_gc_overflow_timeout=0;
#endif

	return RTL_PS_HOOKS_CONTINUE;
}

#if defined(CONFIG_RTL_NF_CONNTRACK_GARBAGE_NEW)
// hooks in clean_from_lists at rtl_nf_connGC.c
int32 clean_from_lists_hooks(struct nf_conn *ct, struct net *net)
{
	#if defined(CONFIG_RTL_IPTABLES_FAST_PATH) || defined(CONFIG_RTL_HARDWARE_NAT)
		rtl_delConnCache(ct);
	#endif

	return RTL_PS_HOOKS_CONTINUE;
}

// hooks in __nf_ct_refresh_acct_proto at rtl_nf_connGC.c
int32 __nf_ct_refresh_acct_proto_hooks(struct nf_conn *ct,
					enum ip_conntrack_info ctinfo,
					const struct sk_buff *skb,
					int do_acct,
					int *event)
{
#ifdef CONFIG_IP_NF_CT_ACCT
	if (do_acct) {
		ct->counters[CTINFO2DIR(ctinfo)].packets++;
		ct->counters[CTINFO2DIR(ctinfo)].bytes +=
						ntohs(skb->nh.iph->tot_len);
		if ((ct->counters[CTINFO2DIR(ctinfo)].packets & 0x80000000)
		    || (ct->counters[CTINFO2DIR(ctinfo)].bytes & 0x80000000))
			*event |= IPCT_COUNTER_FILLING;
	}
#endif

	return RTL_PS_HOOKS_CONTINUE;
}

// hooks1 in __drop_one_conntrack_process at rtl_nf_connGC.c
int32 __drop_one_conntrack_process_hooks1(struct nf_conn* ct, int dropPrioIdx, int factor, int checkFlags, int tcpUdpState)
{
#if defined(CONFIG_RTL_IPTABLES_FAST_PATH) || defined(CONFIG_RTL_HARDWARE_NAT)
	if (checkFlags==TRUE && drop_priority[dropPrioIdx].state==tcpUdpState) {
		#if defined(CONFIG_RTL_IPTABLES_FAST_PATH)
		if (FAILED==rtl_fpTimer_update((void*)ct)) {
			read_unlock_bh(&nf_conntrack_lock);
			rtl_death_action((void*)ct);
			return RTL_PS_HOOKS_RETURN;
		}
		#endif

		#if defined(CONFIG_RTL_HARDWARE_NAT)
		if (FAILED==rtl_hwnat_timer_update(ct)) {
			read_unlock_bh(&nf_conntrack_lock);
			rtl_death_action((void*)ct);
			return RTL_PS_HOOKS_RETURN;
		}
		#endif
	}

#if (HZ==100)
	if (((ct->timeout.expires - jiffies) >> (factor+7))<=drop_priority[dropPrioIdx].threshold)
#elif (HZ==1000)
	if (((ct->timeout.expires - jiffies) >> (factor+10))<=drop_priority[dropPrioIdx].threshold)
#else
	#error "Please Check the HZ defination."
#endif
	{
		read_unlock_bh(&nf_conntrack_lock);
		rtl_death_action((void*)ct);
		return RTL_PS_HOOKS_RETURN;
	}

	return RTL_PS_HOOKS_CONTINUE;
#else
	return RTL_PS_HOOKS_BREAK;
#endif
}

// hooks2 in __drop_one_conntrack_process at rtl_nf_connGC.c
int32 __drop_one_conntrack_process_hooks2(struct nf_conn* ct, int dropPrioIdx, int factor, int checkFlags, int tcpUdpState)
{
#if defined(CONFIG_RTL_IPTABLES_FAST_PATH) || defined(CONFIG_RTL_HARDWARE_NAT)
	add_timer(&ct->timeout);
#endif

	return RTL_PS_HOOKS_CONTINUE;
}

// hooks in rtl_nf_conn_GC_init at rtl_nf_connGC.c
int32 rtl_nf_conn_GC_init_hooks(void)
{
#if defined(CONFIG_PROC_FS)
	gc_overflow_timout_proc_init();
#endif

	return RTL_PS_HOOKS_CONTINUE;
}
#endif


#if defined(CONFIG_BRIDGE)
int32 rtl_fdb_create_hooks(struct net_bridge_fdb_entry *fdb,const unsigned char *addr)
{
#if defined(CONFIG_RTL_LAYERED_DRIVER) && defined(CONFIG_RTL_LAYERED_DRIVER_L2)
	#if defined (CONFIG_RTL865X_LANPORT_RESTRICTION)
		if (fdb->is_static == 0)
		{
			rtl865x_addAuthFDBEntry_hooks(addr);
		}

	#else
	//fdb->ageing_timer = 300*HZ;
		rtl865x_addFDBEntry(addr);
	#endif
#endif

return RTL_PS_HOOKS_CONTINUE;

}
int32 rtl_fdb_delete_hooks(struct net_bridge_fdb_entry *f)
{

#if defined(CONFIG_RTL_LAYERED_DRIVER) && defined(CONFIG_RTL_LAYERED_DRIVER_L2) //&& defined(CONFIG_RTL865X_SYNC_L2)
	#if defined (CONFIG_RTL865X_LANPORT_RESTRICTION)
	rtl865x_delAuthLanFDBEntry(RTL865x_L2_TYPEII, f->addr.addr);
	#else
	rtl865x_delLanFDBEntry(RTL865x_L2_TYPEII, f->addr.addr);
	#endif	/*	defined (CONFIG_RTL865X_LANPORT_RESTRICTION)		*/
#endif	/*	defined(CONFIG_RTL_LAYERED_DRIVER) && defined(CONFIG_RTL_LAYERED_DRIVER_L2) && defined(CONFIG_RTL865X_SYNC_L2)		*/

#if defined(CONFIG_RTL_FASTBRIDGE)
	rtl_fb_del_entry(f->addr.addr);
#endif	/*	defined(CONFIG_RTL_FASTBRIDGE)	*/
	return RTL_PS_HOOKS_CONTINUE;
}


int32 rtl_br_fdb_cleanup_hooks(struct net_bridge *br, struct net_bridge_fdb_entry *f, unsigned long delay)
{
	#if defined(CONFIG_RTL_FASTBRIDGE)
	unsigned long	fb_aging;
	#endif
	#if defined(CONFIG_RTL_LAYERED_DRIVER) && defined(CONFIG_RTL_LAYERED_DRIVER_L2) && defined(CONFIG_RTL865X_SYNC_L2)
	int32 port_num;
	unsigned long hw_aging;
	#endif
	int ret;

	/*printk("timelist as follow:(s)jiffies:%ld,f->ageing_timer:%ld,delay:%ld",jiffies/HZ,f->ageing_timer/HZ,delay/HZ);*/
	if (time_after(f->ageing_timer, jiffies))
	{
		DEBUG_PRINT("\nf->ageing_timer AFTER jiffies:addr is :%x,%x,%x,%x,%x,%x\n",f->addr.addr[0],f->addr.addr[1],f->addr.addr[2],f->addr.addr[3],f->addr.addr[4],f->addr.addr[5]);
		DEBUG_PRINT("time list:jiffies:%ld,hw_aging:%ld,f->ageing_timer:%ld\n",jiffies/HZ,hw_aging/HZ,f->ageing_timer/HZ );
		return RTL_PS_HOOKS_BREAK;
	}

	#if defined(CONFIG_RTL_LAYERED_DRIVER) && defined(CONFIG_RTL_LAYERED_DRIVER_L2) && defined(CONFIG_RTL865X_SYNC_L2)
		port_num= -100;
		ret = rtl865x_arrangeFdbEntry(f->addr.addr, &port_num);

		switch (ret) {
			case RTL865X_FDBENTRY_450SEC:
				hw_aging = jiffies;
				break;
			case RTL865X_FDBENTRY_300SEC:
				hw_aging = jiffies -150*HZ;
				break;
			case RTL865X_FDBENTRY_150SEC:
				hw_aging = jiffies -300*HZ;
				break;
			case RTL865X_FDBENTRY_TIMEOUT:
			case FAILED:
			default:
				hw_aging =jiffies -450*HZ;
				break;
		}

		ret = 0;
		if(time_before_eq(f->ageing_timer,  hw_aging))
		{
			/*fresh f->ageing_timer*/
			f->ageing_timer = hw_aging;
		}
	#endif

	#if defined(CONFIG_RTL_FASTBRIDGE)
		fb_aging = rtl_fb_get_entry_lastused(f->addr.addr);
		if(time_before_eq(f->ageing_timer,  fb_aging))
		{
			f->ageing_timer = fb_aging;
		}
	#endif

	if (ret==0) {
		return RTL_PS_HOOKS_CONTINUE;
	} else {
		return RTL_PS_HOOKS_BREAK;
	}
}


#endif	/*	defined(CONFIG_BRIDGE)	*/


