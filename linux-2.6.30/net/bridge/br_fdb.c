/*
 *	Forwarding database
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/rculist.h>
#include <linux/spinlock.h>
#include <linux/times.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/jhash.h>
#include <linux/random.h>
#include <asm/atomic.h>
#include <asm/unaligned.h>
#include "br_private.h"

#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl865x_fdb_api.h>

#if	defined(CONFIG_RTL_819X)
#include <net/rtl/features/rtl_ps_hooks.h>

#if defined (CONFIG_RTL_LAYERED_DRIVER_L2)&& defined (CONFIG_RTL865X_LANPORT_RESTRICTION)
#include <net/rtl/features/lan_restrict.h>

#endif

#endif	/*	defined(CONFIG_RTL_819X)	*/

#if defined(CONFIG_RTL_819X)
static int fdb_entry_max = 2048;
static int fdb_entry_num = 0;
#endif

#if defined (CONFIG_RTL_IGMP_SNOOPING)
#include <net/rtl/rtl865x_netif.h>
#include <net/rtl/rtl_nic.h>
extern int IGMPProxyOpened;
void add_igmp_ext_entry(	struct net_bridge_fdb_entry *fdb ,unsigned char *srcMac , unsigned char portComeIn);
void update_igmp_ext_entry(	struct net_bridge_fdb_entry *fdb ,unsigned char *srcMac , unsigned char portComeIn);
void del_igmp_ext_entry(	struct net_bridge_fdb_entry *fdb ,unsigned char *srcMac , unsigned char portComeIn );
//void br_igmp_fdb_expired(struct net_bridge_fdb_entry *ent);
#endif

static struct kmem_cache *br_fdb_cache __read_mostly;
static int fdb_insert(struct net_bridge *br, struct net_bridge_port *source,
		      const unsigned char *addr);

static u32 fdb_salt __read_mostly;

int __init br_fdb_init(void)
{
	br_fdb_cache = kmem_cache_create("bridge_fdb_cache",
					 sizeof(struct net_bridge_fdb_entry),
					 0,
					 SLAB_HWCACHE_ALIGN, NULL);
	if (!br_fdb_cache)
		return -ENOMEM;

	get_random_bytes(&fdb_salt, sizeof(fdb_salt));
	return 0;
}

void br_fdb_fini(void)
{
	kmem_cache_destroy(br_fdb_cache);
}


/* if topology_changing then use forward_delay (default 15 sec)
 * otherwise keep longer (default 5 minutes)
 */
static inline unsigned long hold_time(const struct net_bridge *br)
{
	return br->topology_change ? br->forward_delay : br->ageing_time;
}

static inline int has_expired(const struct net_bridge *br,
				  const struct net_bridge_fdb_entry *fdb)
{
	return !fdb->is_static
		&& time_before_eq(fdb->ageing_timer + hold_time(br), jiffies);
}

static inline int br_mac_hash(const unsigned char *mac)
{
	/* use 1 byte of OUI cnd 3 bytes of NIC */
	u32 key = get_unaligned((u32 *)(mac + 2));
	return jhash_1word(key, fdb_salt) & (BR_HASH_SIZE - 1);
}

static inline void fdb_delete(struct net_bridge_fdb_entry *f)
{
	hlist_del_rcu(&f->hlist);
	br_fdb_put(f);
	#if 0
	#if defined(CONFIG_RTL_819X)
	rtl_fdb_delete_hooks(f);
	#endif
	#endif
}

void br_fdb_changeaddr(struct net_bridge_port *p, const unsigned char *newaddr)
{
	struct net_bridge *br = p->br;
	int i;

	spin_lock_bh(&br->hash_lock);

	/* Search all chains since old address/hash is unknown */
	for (i = 0; i < BR_HASH_SIZE; i++) {
		struct hlist_node *h;
		hlist_for_each(h, &br->hash[i]) {
			struct net_bridge_fdb_entry *f;

			f = hlist_entry(h, struct net_bridge_fdb_entry, hlist);
			if (f->dst == p && f->is_local) {
				/* maybe another port has same hw addr? */
				struct net_bridge_port *op;
				list_for_each_entry(op, &br->port_list, list) {
					if (op != p &&
					    !compare_ether_addr(op->dev->dev_addr,
								f->addr.addr)) {
						f->dst = op;
						goto insert;
					}
				}

				/* delete old one */
				//printk("[%s][%d].\n", __FUNCTION__, __LINE__);
				fdb_delete(f);
				goto insert;
			}
		}
	}
 insert:
	/* insert new address,  may fail if invalid address or dup. */
	fdb_insert(br, p, newaddr);

	spin_unlock_bh(&br->hash_lock);
}

#if defined (CONFIG_RTL_IGMP_SNOOPING)
 void br_igmp_fdb_expired(struct net_bridge_fdb_entry *ent)
{
	int i2;
	unsigned long igmp_walktimeout;	
	unsigned char *DA;
	unsigned char *SA;
	#if defined	(MCAST_TO_UNICAST)
	struct net_device *dev=NULL;
	#endif
	
	igmp_walktimeout = 	jiffies - IGMP_EXPIRE_TIME;	
	    
	//IGMP_EXPIRE_TIME
	for(i2=0 ; i2 < FDB_IGMP_EXT_NUM ; i2++)
	{
		if(ent->igmp_fdb_arr[i2].valid == 1){

			// when timeout expire
			if(time_before_eq(ent->igmp_fdb_arr[i2].ageing_time, igmp_walktimeout))
			{
				DEBUG_PRINT("%s:%d\n",__FUNCTION__,__LINE__);	
				SA = ent->igmp_fdb_arr[i2].SrcMac;					
				DEBUG_PRINT("expired src mac:%02x,%02x,%02x,%02x,%02x,%02x\n",
					SA[0],SA[1],SA[2],SA[3],SA[4],SA[5]);								

				DA = ent->addr.addr;					
				DEBUG_PRINT("fdb:%02x:%02x:%02x-%02x:%02x:%02x\n",
					DA[0],DA[1],DA[2],DA[3],DA[4],DA[5]);				



				/*---for process wlan client expired start---*/								
				#if defined	(MCAST_TO_UNICAST)
				dev = __dev_get_by_name(&init_net, RTL_PS_WLAN0_DEV_NAME);	
	
				
				if (dev) {		
					unsigned char StaMacAndGroup[20];
					memcpy(StaMacAndGroup, DA , 6);
					memcpy(StaMacAndGroup+6, SA, 6);
				#if defined(CONFIG_COMPAT_NET_DEV_OPS)
					if (dev->do_ioctl != NULL) {
						dev->do_ioctl(dev, (struct ifreq*)StaMacAndGroup, 0x8B81);
				#else
					if (dev->netdev_ops->ndo_do_ioctl != NULL) {
						dev->netdev_ops->ndo_do_ioctl(dev, (struct ifreq*)StaMacAndGroup, 0x8B81);
				#endif
						DEBUG_PRINT("(fdb expire) wlan0 ioctl to DEL! M2U entry da:%02x:%02x:%02x-%02x:%02x:%02x; sa:%02x:%02x:%02x-%02x:%02x:%02x\n",
							StaMacAndGroup[0],StaMacAndGroup[1],StaMacAndGroup[2],StaMacAndGroup[3],StaMacAndGroup[4],StaMacAndGroup[5],
							StaMacAndGroup[6],StaMacAndGroup[7],StaMacAndGroup[8],StaMacAndGroup[9],StaMacAndGroup[10],StaMacAndGroup[11]);
					}
				}
					
				#if defined (CONFIG_RTL_92D_SUPPORT)
				dev = __dev_get_by_name(&init_net, RTL_PS_WLAN1_DEV_NAME);	
	
				if (dev) {		
					unsigned char StaMacAndGroup[20];
					memcpy(StaMacAndGroup, DA , 6);
					memcpy(StaMacAndGroup+6, SA, 6);
				#if defined(CONFIG_COMPAT_NET_DEV_OPS)
					if (dev->do_ioctl != NULL) {
						dev->do_ioctl(dev, (struct ifreq*)StaMacAndGroup, 0x8B81);
				#else
					if (dev->netdev_ops->ndo_do_ioctl != NULL) {
						dev->netdev_ops->ndo_do_ioctl(dev, (struct ifreq*)StaMacAndGroup, 0x8B81);
				#endif
						DEBUG_PRINT("(fdb expire) wlan0 ioctl to DEL! M2U entry da:%02x:%02x:%02x-%02x:%02x:%02x; sa:%02x:%02x:%02x-%02x:%02x:%02x\n",
							StaMacAndGroup[0],StaMacAndGroup[1],StaMacAndGroup[2],StaMacAndGroup[3],StaMacAndGroup[4],StaMacAndGroup[5],
							StaMacAndGroup[6],StaMacAndGroup[7],StaMacAndGroup[8],StaMacAndGroup[9],StaMacAndGroup[10],StaMacAndGroup[11]);
					}
				}
				#endif
				#endif			
				/*---for process wlan client expired end---*/

			
				del_igmp_ext_entry(ent , SA , ent->igmp_fdb_arr[i2].port);

	
				if ( (ent->portlist & 0x7f)==0){
					ent->group_src &=  ~(1 << 1); // eth0 all leave
				}
			
				if ( (ent->portlist & 0x80)==0){
					ent->group_src &=  ~(1 << 2); // wlan0 all leave
				}
			
			
			}			
			
		}		
		
	}		
	
}
#endif

void br_fdb_cleanup(unsigned long _data)
{
	struct net_bridge *br = (struct net_bridge *)_data;
#if 0
#if defined(CONFIG_RTL_LAYERED_DRIVER) && defined(CONFIG_RTL_LAYERED_DRIVER_L2)
	int32 port_num;
#if defined (CONFIG_RTL865X_LANPORT_RESTRICTION)
	unsigned char swap_addr[ETHER_ADDR_LEN];
#endif
	int ret;
	unsigned long hw_aging;
#endif
#endif
	unsigned long delay = hold_time(br);
	unsigned long next_timer = jiffies + br->forward_delay;
	int i;
	
	spin_lock_bh(&br->hash_lock);
	
	for (i = 0; i < BR_HASH_SIZE; i++) {
		struct net_bridge_fdb_entry *f;
		struct hlist_node *h, *n;

		hlist_for_each_entry_safe(f, h, n, &br->hash[i], hlist) {
			unsigned long this_timer;
			
			#if defined	(CONFIG_RTL_IGMP_SNOOPING)
			if(	f->is_static &&
				f->igmpFlag &&
				MULTICAST_MAC(f->addr.addr))
			{
										
				br_igmp_fdb_expired(f);
				
				if(time_before_eq(f->ageing_timer +300*HZ,  jiffies))
				{
					DEBUG_PRINT("fdb_delete:f->addr.addr is 0x%02x:%02x:%02x-%02x:%02x:%02x\n",
					f->addr.addr[0],f->addr.addr[1],f->addr.addr[2],f->addr.addr[3],f->addr.addr[4],f->addr.addr[5]);	
					fdb_delete(f);
				}
			
			}
			#endif

			if (f->is_static)
				continue;
			
			#if defined(CONFIG_RTL_819X)
				rtl_br_fdb_cleanup_hooks(br,f, delay);
			#endif
			this_timer = f->ageing_timer+delay;
			
			if (time_before_eq(this_timer, jiffies)) 
			{
				fdb_delete(f);
				/*delete  an  FDB Entry   */
			} 
			else if (time_before(this_timer, next_timer))
			{
				next_timer = this_timer;
			}
			else
			{
				//printk("[%s][%d].\n", __FUNCTION__, __LINE__);
			}

		} //end hlist_for_each_entry_safe
	}
	spin_unlock_bh(&br->hash_lock);

	/* Add HZ/4 to ensure we round the jiffies upwards to be after the next
	 * timer, otherwise we might round down and will have no-op run. */
	mod_timer(&br->gc_timer, round_jiffies(next_timer + HZ/4));
}



/* Completely flush all dynamic entries in forwarding database.*/
void br_fdb_flush(struct net_bridge *br)
{
	int i;

	spin_lock_bh(&br->hash_lock);
	for (i = 0; i < BR_HASH_SIZE; i++) {
		struct net_bridge_fdb_entry *f;
		struct hlist_node *h, *n;
		hlist_for_each_entry_safe(f, h, n, &br->hash[i], hlist) {
		#if defined (CONFIG_RTL_IGMP_SNOOPING)
			if(	f->is_static && 
				f->igmpFlag &&
				MULTICAST_MAC(f->addr.addr) )
			{
				br_igmp_fdb_expired(f);
				if(time_before_eq(f->ageing_timer + 300*HZ,  jiffies))
				{
					//printk("[%s][%d].\n", __FUNCTION__, __LINE__);
					fdb_delete(f);
				}
			}
		#endif
			if (!f->is_static)
				{
					//printk("[%s][%d].\n", __FUNCTION__, __LINE__);
					fdb_delete(f);
				}
		}
	}
	spin_unlock_bh(&br->hash_lock);
}

/* Flush all entries refering to a specific port.
 * if do_all is set also flush static entries
 */
void br_fdb_delete_by_port(struct net_bridge *br,
			   const struct net_bridge_port *p,
			   int do_all)
{
	int i;

	spin_lock_bh(&br->hash_lock);
	for (i = 0; i < BR_HASH_SIZE; i++) {
		struct hlist_node *h, *g;

		hlist_for_each_safe(h, g, &br->hash[i]) {
			struct net_bridge_fdb_entry *f
				= hlist_entry(h, struct net_bridge_fdb_entry, hlist);
			if (f->dst != p)
				continue;

			if (f->is_static && !do_all)
				continue;
			/*
			 * if multiple ports all have the same device address
			 * then when one port is deleted, assign
			 * the local entry to other port
			 */
			if (f->is_local) {
				struct net_bridge_port *op;
				list_for_each_entry(op, &br->port_list, list) {
					if (op != p &&
					    !compare_ether_addr(op->dev->dev_addr,
								f->addr.addr)) {
						f->dst = op;
						goto skip_delete;
					}
				}
			}
			//printk("[%s][%d].\n", __FUNCTION__, __LINE__);
			fdb_delete(f);
		skip_delete: ;
		}
	}
	spin_unlock_bh(&br->hash_lock);
}

/* No locking or refcounting, assumes caller has no preempt (rcu_read_lock) */
struct net_bridge_fdb_entry *__br_fdb_get(struct net_bridge *br,
					  const unsigned char *addr)
{
	struct hlist_node *h;
	struct net_bridge_fdb_entry *fdb;

	hlist_for_each_entry_rcu(fdb, h, &br->hash[br_mac_hash(addr)], hlist) {
		if (!compare_ether_addr(fdb->addr.addr, addr)) {
			if (unlikely(has_expired(br, fdb))) {
			#if defined(CONFIG_RTL_819X)
				if (rtl___br_fdb_get_timeout_hooks(br, fdb, addr)==RTL_PS_HOOKS_BREAK) {
					break;
				}
			#else
				break;
			#endif
			}
			return fdb;
		}
	}

	return NULL;
}

/* Interface used by ATM hook that keeps a ref count */
struct net_bridge_fdb_entry *br_fdb_get(struct net_bridge *br,
					unsigned char *addr)
{
	struct net_bridge_fdb_entry *fdb;

	rcu_read_lock();
	fdb = __br_fdb_get(br, addr);
	if (fdb && !atomic_inc_not_zero(&fdb->use_count))
		fdb = NULL;
	rcu_read_unlock();
	return fdb;
}

/*static void fdb_rcu_free(struct rcu_head *head)
{
	struct net_bridge_fdb_entry *ent
		= container_of(head, struct net_bridge_fdb_entry, rcu);	
	kmem_cache_free(br_fdb_cache, ent);
#if defined(CONFIG_RTL_819X)
	rtl_fdb_delete_hooks(f);
	fdb_entry_num--;
	if(fdb_entry_num < 0)
	{
		printk("fdb entry num error!!!!\n");
		fdb_entry_num = 0;
	}
#endif
}*/

static void fdb_rcu_free(struct rcu_head *head)
{
struct net_bridge_fdb_entry *ent
		= container_of(head, struct net_bridge_fdb_entry, rcu);	
	kmem_cache_free(br_fdb_cache, ent);
#if defined(CONFIG_RTL_819X)
	#if defined(CONFIG_RTL_819X)
	rtl_fdb_delete_hooks(ent);
	#endif
	fdb_entry_num--;
	if(fdb_entry_num < 0)
	{
		printk("fdb entry num error!!!!\n");
		fdb_entry_num = 0;
	}
	/*printk("fdb_entry_num:%d\n",fdb_entry_num);*/
#endif
}

/* Set entry up for deletion with RCU  */
void br_fdb_put(struct net_bridge_fdb_entry *ent)
{
	
	if (atomic_dec_and_test(&ent->use_count)) {

		call_rcu(&ent->rcu, fdb_rcu_free);
		//rtl_fdb_delete_hooks(ent);
	
	}
}

/*
 * Fill buffer with forwarding table records in
 * the API format.
 */
int br_fdb_fillbuf(struct net_bridge *br, void *buf,
		   unsigned long maxnum, unsigned long skip)
{
	struct __fdb_entry *fe = buf;
	int i, num = 0;
	struct hlist_node *h;
	struct net_bridge_fdb_entry *f;

	memset(buf, 0, maxnum*sizeof(struct __fdb_entry));

	rcu_read_lock();
	for (i = 0; i < BR_HASH_SIZE; i++) {
		hlist_for_each_entry_rcu(f, h, &br->hash[i], hlist) {
			if (num >= maxnum)
				goto out;

			if (has_expired(br, f))
				continue;

			if (skip) {
				--skip;
				continue;
			}

			/* convert from internal format to API */
			memcpy(fe->mac_addr, f->addr.addr, ETH_ALEN);

			/* due to ABI compat need to split into hi/lo */
			fe->port_no = f->dst->port_no;
			fe->port_hi = f->dst->port_no >> 8;

			fe->is_local = f->is_local;
			if (!f->is_static)
				fe->ageing_timer_value = jiffies_to_clock_t(jiffies - f->ageing_timer);
			++fe;
			++num;
		}
	}

 out:
	rcu_read_unlock();

	return num;
}

static inline struct net_bridge_fdb_entry *fdb_find(struct hlist_head *head,
						    const unsigned char *addr)
{
	struct hlist_node *h;
	struct net_bridge_fdb_entry *fdb;

	hlist_for_each_entry_rcu(fdb, h, head, hlist) {
		if (!compare_ether_addr(fdb->addr.addr, addr))
			return fdb;
	}
	return NULL;
}

static struct net_bridge_fdb_entry *fdb_create(struct hlist_head *head,
					       struct net_bridge_port *source,
					       const unsigned char *addr,
					       int is_local)
{
	struct net_bridge_fdb_entry *fdb;

#if defined(CONFIG_RTL_819X)
	if(fdb_entry_num >= fdb_entry_max)
		return NULL;
#endif
	int i3;

	fdb = kmem_cache_alloc(br_fdb_cache, GFP_ATOMIC);
	if (fdb) 
	{
		memcpy(fdb->addr.addr, addr, ETH_ALEN);
		atomic_set(&fdb->use_count, 1);
		#if defined(CONFIG_RTL_819X)
			fdb_entry_num++;
		#endif
		hlist_add_head_rcu(&fdb->hlist, head);

		fdb->dst = source;
		fdb->is_local = is_local;
		fdb->is_static = is_local;
		fdb->ageing_timer = jiffies;
		
		#if defined (CONFIG_RTL_IGMP_SNOOPING)
		
		fdb->group_src = 0;
		fdb->igmpFlag=0;
		for(i3=0 ; i3<FDB_IGMP_EXT_NUM ;i3++)
		{
			fdb->igmp_fdb_arr[i3].valid = 0;
			fdb->portUsedNum[i3] = 0;		
		}
		#endif
		
		#if defined(CONFIG_RTL_819X)
		rtl_fdb_create_hooks(fdb, addr);		
		#endif
		
	}
	
	return fdb;
}

static int fdb_insert(struct net_bridge *br, struct net_bridge_port *source,
		  const unsigned char *addr)
{
	struct hlist_head *head = &br->hash[br_mac_hash(addr)];
	struct net_bridge_fdb_entry *fdb;
#if defined (CONFIG_RTL_IGMP_SNOOPING)	
	if(((addr[0]==0xff) && (addr[1]==0xff) && (addr[2]==0xff) && (addr[3]==0xff) && (addr[4]==0xff) && (addr[5]==0xff))||
		((addr[0]==0) && (addr[1]==0) && (addr[2]==0) && (addr[3]==0) && (addr[4]==0) && (addr[5]==0)))
	{
		return -EINVAL;
	}
#else	
	if (!is_valid_ether_addr(addr))
		return -EINVAL;
#endif
	fdb = fdb_find(head, addr);
	if (fdb) {
		/* it is okay to have multiple ports with same
		 * address, just use the first one.
		 */
		if (fdb->is_local)
			return 0;

		printk(KERN_WARNING "%s adding interface with same address "
		       "as a received packet\n",
		       source->dev->name);
		//printk("[%s][%d].\n", __FUNCTION__, __LINE__);
		fdb_delete(fdb);
	}

	if (!fdb_create(head, source, addr, 1))
		return -ENOMEM;

	return 0;
}

int br_fdb_insert(struct net_bridge *br, struct net_bridge_port *source,
		  const unsigned char *addr)
{
	int ret;

	spin_lock_bh(&br->hash_lock);
	ret = fdb_insert(br, source, addr);
	spin_unlock_bh(&br->hash_lock);
	return ret;
}

void br_fdb_update(struct net_bridge *br, struct net_bridge_port *source,
		   const unsigned char *addr)
{
	struct hlist_head *head = &br->hash[br_mac_hash(addr)];
	struct net_bridge_fdb_entry *fdb;

	/* some users want to always flood. */
	if (hold_time(br) == 0)
		return;

	/* ignore packets unless we are using this port */
	if (!(source->state == BR_STATE_LEARNING ||
	      source->state == BR_STATE_FORWARDING))
		return;

	fdb = fdb_find(head, addr);
	if (likely(fdb)) 
	{
		
		/* attempt to update an entry for a local interface */
		if (unlikely(fdb->is_local)) 
		{
			if (net_ratelimit())
				printk(KERN_WARNING "%s: received packet with "
				       " own address as source address\n",
				       source->dev->name);
		} 
		else 
		{
			/* fastpath: update of existing entry */
			/* 02-17-2012: move to WLAN driver update_fwtbl_asoclst() to avoid hacking the Linux kernel or the other kernel */
#if 0 //defined (CONFIG_RTL865X_ETH)
//                       if (((unsigned long)fdb->dst) != ((unsigned long)source->br->dev->a)) { 
			if (fdb->dst != source)
			{	
				update_hw_l2table(source->dev->name, addr);
			}
#endif			
			fdb->dst = source;
			fdb->ageing_timer = jiffies;
		}
	} else {
		spin_lock(&br->hash_lock);
		if (!fdb_find(head, addr))
			fdb_create(head, source, addr, 0);
		/* else  we lose race and someone else inserts
		 * it first, don't bother updating
		 */
		spin_unlock(&br->hash_lock);
	}
}

#if defined (CONFIG_RTL_IGMP_SNOOPING)
int chk_igmp_ext_entry(
	struct net_bridge_fdb_entry *fdb ,
	unsigned char *srcMac)
{

	int i2;
	unsigned char *add;
	add = fdb->addr.addr;

	for(i2=0 ; i2 < FDB_IGMP_EXT_NUM ; i2++){
		if(fdb->igmp_fdb_arr[i2].valid == 1){
			if(!memcmp(fdb->igmp_fdb_arr[i2].SrcMac , srcMac ,6)){
				return 1;
			}
		}
	}
	return 0;
}

extern int bitmask_to_id(unsigned char val);

void add_igmp_ext_entry(	struct net_bridge_fdb_entry *fdb ,
	unsigned char *srcMac , unsigned char portComeIn)
{

//	if(fdb->igmp_ext_array == NULL)
//		return 0;

	int i2;
	unsigned char *add;
	add = fdb->addr.addr;	

	DEBUG_PRINT("add_igmp,DA=%02x:%02x:%02x:%02x:%02x:%02x ; SA=%02x:%02x:%02x:%02x:%02x:%02x\n",
		add[0],add[1],add[2],add[3],add[4],add[5],
		srcMac[0],srcMac[1],srcMac[2],srcMac[3],srcMac[4],srcMac[5]);
	
	for(i2=0 ; i2 < FDB_IGMP_EXT_NUM ; i2++){
		if(fdb->igmp_fdb_arr[i2].valid == 0){
			fdb->igmp_fdb_arr[i2].valid = 1	;
			fdb->igmp_fdb_arr[i2].ageing_time = jiffies ;			
			memcpy(fdb->igmp_fdb_arr[i2].SrcMac , srcMac ,6);
			fdb->igmp_fdb_arr[i2].port = portComeIn ;
			fdb->portlist |= portComeIn;
			fdb->portUsedNum[bitmask_to_id(portComeIn)]++;
			DEBUG_PRINT("portUsedNum[%d]=%d\n\n",bitmask_to_id(portComeIn) , fdb->portUsedNum[bitmask_to_id(portComeIn)]);
			return ;
		}
	}
	DEBUG_PRINT("%s:entry Rdy existed!!!\n", __FUNCTION__);	
}

void update_igmp_ext_entry(	struct net_bridge_fdb_entry *fdb ,
	unsigned char *srcMac , unsigned char portComeIn)
{
	int i2;
	unsigned char *add;
	add = fdb->addr.addr;	

		DEBUG_PRINT("update_igmp,DA=%02x:%02x:%02x:%02x:%02x:%02x ; SA=%02x:%02x:%02x:%02x:%02x:%02x\n",
		add[0],add[1],add[2],add[3],add[4],add[5],
		srcMac[0],srcMac[1],srcMac[2],srcMac[3],srcMac[4],srcMac[5]);

	for(i2=0 ; i2 < FDB_IGMP_EXT_NUM ; i2++){
		if(fdb->igmp_fdb_arr[i2].valid == 1){
			if(!memcmp(fdb->igmp_fdb_arr[i2].SrcMac , srcMac ,6)){

				fdb->igmp_fdb_arr[i2].ageing_time = jiffies ;
				//DEBUG_PRINT("update jiffies ok!\n");
				if(fdb->igmp_fdb_arr[i2].port != portComeIn){

					unsigned char port_orig = fdb->igmp_fdb_arr[i2].port ;					
					int index = bitmask_to_id(port_orig);

					fdb->portUsedNum[index]-- ;
					DEBUG_PRINT("(--) portUsedNum[%d]=%d\n",index , fdb->portUsedNum[index] );					
					if(fdb->portUsedNum[index] <= 0){
						fdb->portlist &= ~(port_orig);
						if(fdb->portUsedNum[index]< 0){
							DEBUG_PRINT("!! portNum[%d] < 0 at (update_igmp_ext_entry)\n",index);
							fdb->portUsedNum[index] = 0 ;
						}
					}					

					
					fdb->portUsedNum[bitmask_to_id(portComeIn)]++;
					DEBUG_PRINT("(++) portUsedNum[%d]=%d\n",bitmask_to_id(portComeIn) , fdb->portUsedNum[bitmask_to_id(portComeIn)] );										
					fdb->portlist |= portComeIn;						

					
					fdb->igmp_fdb_arr[i2].port = portComeIn ;					
					DEBUG_PRINT("	!!! portlist be updated:%x !!!!\n",fdb->portlist);
					
				}
				return ;
			}			
		}		
	}

	DEBUG_PRINT("%s: ...fail!!\n", __FUNCTION__);
}


void del_igmp_ext_entry(	struct net_bridge_fdb_entry *fdb ,unsigned char *srcMac , unsigned char portComeIn )
{
	int i2;
	unsigned char *add;
	add = fdb->addr.addr;	


				

	
	for(i2=0 ; i2 < FDB_IGMP_EXT_NUM ; i2++){
		if(fdb->igmp_fdb_arr[i2].valid == 1){
			if(!memcmp(fdb->igmp_fdb_arr[i2].SrcMac , srcMac ,6))
			{
				fdb->igmp_fdb_arr[i2].ageing_time -=  300*HZ; // make it expired	
				fdb->igmp_fdb_arr[i2].valid = 0;
				DEBUG_PRINT("\ndel_igmp_ext_entry,DA=%02x:%02x:%02x:%02x:%02x:%02x ; SA=%02x:%02x:%02x:%02x:%02x:%02x success!!!\n",
				add[0],add[1],add[2],add[3],add[4],add[5],
				srcMac[0],srcMac[1],srcMac[2],srcMac[3],srcMac[4],srcMac[5]);
				
				//DEBUG_PRINT("%s:success!!\n", __FUNCTION__);
				
				if(portComeIn != 0){
					int index;
					index = bitmask_to_id(portComeIn);
					fdb->portUsedNum[index]--;
					if(fdb->portUsedNum[index] <= 0){
						DEBUG_PRINT("portUsedNum[%d] == 0 ,update portlist from (%x)  " ,index ,fdb->portlist);
						fdb->portlist &= ~ portComeIn;
						DEBUG_PRINT("to (%x) \n" ,fdb->portlist);
						
						if(fdb->portUsedNum[index] < 0){
						DEBUG_PRINT("!! portUsedNum[%d]=%d < 0 at (del_igmp_ext_entry)  \n" ,index ,fdb->portUsedNum[index]);
						fdb->portUsedNum[index] = 0;
						}
					}else{
						DEBUG_PRINT("(del) portUsedNum[%d] = %d \n" ,index, fdb->portUsedNum[index]);
					}
				
				}	
				DEBUG_PRINT("\n");
				return ;
			}			
		}
	}

	DEBUG_PRINT("%s:entry not existed!!\n\n", __FUNCTION__);	
}




#endif

#if defined (CONFIG_RTL_LOCAL_PUBLIC)
int rtl865x_checkMacAddrLocation(unsigned char *addr, unsigned int *isElanMac, unsigned int *isWlanMac)
{
	struct net_device *brDev;
	struct net_bridge *br;
	struct net_bridge_fdb_entry *fdb;

	if((addr==NULL) || (isElanMac==NULL) || (isWlanMac==NULL))
	{
		return -1;
	}

	*isElanMac=0;
	*isWlanMac=0;

	brDev = __dev_get_by_name(&init_net,RTL_PS_BR0_DEV_NAME);
      	if ( brDev== NULL)
	{
		return -1;
	}

	if (!(brDev->priv_flags & IFF_EBRIDGE))
	{
		
		return -1;
	}
	else 
	{
		br = (struct net_bridge *)netdev_priv(brDev);
	}
	
	
	fdb=__br_fdb_get(br, addr);

	if(fdb==NULL)
	{
		return -1;
	}
	
	if(strncmp(fdb->dst->dev->name, RTL_PS_ETH_NAME,3)==0)
	{
		*isElanMac=1;
		
	}
	else if(strncmp(fdb->dst->dev->name, RTL_PS_WLAN_NAME,4)==0)
	{
		*isWlanMac=1;
	}	
	else
	{
		return -1;
	}

	return 0;
	
}
#endif

#ifdef CONFIG_RTL865X_LANPORT_RESTRICTION
int rtl_delFdbByMac(const unsigned char *macAddr, const char* devName)
{
	int i;
	struct net_bridge_port *p;
	struct net_device *dev;
	struct net_bridge *br;

	if((macAddr==NULL) ||(devName ==NULL) )
		return FAILED;

	dev = __dev_get_by_name(&init_net, devName);

	if(dev->br_port == NULL)
		return FAILED;
	
	p = dev->br_port;

	if(p->br == NULL)
		return FAILED;

	br = p->br;

	spin_lock_bh(&br->hash_lock);

	for (i = 0; i < BR_HASH_SIZE; i++) {
		struct net_bridge_fdb_entry *f;
		struct hlist_node *h, *n;

		hlist_for_each_entry_safe(f, h, n, &br->hash[i], hlist) {
			if(f->is_static)
				continue;

			if (!compare_ether_addr(f->addr.addr, macAddr)){
				fdb_delete(f);
				spin_unlock_bh(&br->hash_lock);
				return SUCCESS;
			}
		}
	}
	
	spin_unlock_bh(&br->hash_lock);
	return FAILED;
}		
#endif


