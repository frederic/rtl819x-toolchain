/*
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

#ifndef _BR_PRIVATE_H
#define _BR_PRIVATE_H

#include <linux/netdevice.h>
#include <linux/if_bridge.h>
#include <net/route.h>

//#define DEBUG_PRINT(fmt, args...) printk(fmt, ## args)
#define DEBUG_PRINT(fmt, args...)

#if defined (CONFIG_RTL_IGMP_SNOOPING)

#define MULTICAST_MAC(mac) 	   ((mac[0]==0x01)&&(mac[1]==0x00)&&(mac[2]==0x5e))
#define IPV6_MULTICAST_MAC(mac) ((mac[0]==0x33)&&(mac[1]==0x33) && mac[2]!=0xff)
//#define CONFIG_BRIDGE_IGMPV3_SNOOPING

#define MCAST_TO_UNICAST

#define IGMP_EXPIRE_TIME (260*HZ)

#if defined (MCAST_TO_UNICAST)
#define IPV6_MCAST_TO_UNICAST
#endif

#define MLCST_FLTR_ENTRY	16
#define MLCST_MAC_ENTRY		64

extern int rtk_vlan_support_enable;
// interface to set multicast bandwidth control
//#define MULTICAST_BWCTRL

// interface to enable MAC clone function
//#define RTL_BRIDGE_MAC_CLONE
//#define RTL_BRIDGE_DEBUG

#define MCAST_QUERY_INTERVAL 30

#endif

#ifdef CONFIG_RTL_LAYERED_DRIVER_L2
#define CONFIG_RTL865X_ETH
#endif

#if defined(CONFIG_RTL_HW_STP)
#include <net/rtl/rtl865x_netif.h>
#include <net/rtl/rtk_vlan.h>
#include <net/rtl/rtk_stp.h>
#endif

#define BR_HASH_BITS 8
#define BR_HASH_SIZE (1 << BR_HASH_BITS)

#define BR_HOLD_TIME (1*HZ)

#define BR_PORT_BITS	10
#define BR_MAX_PORTS	(1<<BR_PORT_BITS)

#define BR_VERSION	"2.3"

/* Path to usermode spanning tree program */
#define BR_STP_PROG	"/sbin/bridge-stp"

#if defined (CONFIG_RTK_MESH)
#define STP_ADDCOST_ETH

#define MESH_PORTAL_EXPIRE 300 //seconds
#if 0
#ifdef STP_DISABLE_ETH
//Chris: stp+mesh
#define ETH_CHK_INTVL		(30*HZ)
#endif
#endif
#endif

typedef struct bridge_id bridge_id;
typedef struct mac_addr mac_addr;
typedef __u16 port_id;

#if defined (CONFIG_RTL_IGMP_SNOOPING)
#define FDB_IGMP_EXT_NUM 8
struct fdb_igmp_ext_entry
{
	int valid;
	unsigned long ageing_time;
	unsigned char SrcMac[6];	
	unsigned char port;

};

struct fdb_igmp_ext_array
{
	struct fdb_igmp_ext_entry igmp_fdb_arr[FDB_IGMP_EXT_NUM];
};

#endif

struct bridge_id
{
	unsigned char	prio[2];
	unsigned char	addr[6];
};

struct mac_addr
{
	unsigned char	addr[6];
};

struct net_bridge_fdb_entry
{
	struct hlist_node		hlist;
	struct net_bridge_port		*dst;

	struct rcu_head			rcu;
	atomic_t			use_count;
	unsigned long			ageing_timer;
#if defined (CONFIG_RTL_IGMP_SNOOPING)
	unsigned short group_src;
	unsigned char			igmpFlag;
	unsigned char			portlist;
	int 					portUsedNum[8];	// be used with portlist, for record each port has how many client
	//struct fdb_igmp_ext_array	igmp_ext_array;
	struct fdb_igmp_ext_entry igmp_fdb_arr[FDB_IGMP_EXT_NUM];
#endif

	mac_addr			addr;
	unsigned char			is_local;
	unsigned char			is_static;
};

struct net_bridge_port
{
	struct net_bridge		*br;
	struct net_device		*dev;
	struct list_head		list;

	/* STP */
	u8				priority;
	u8				state;
	u16				port_no;
	unsigned char			topology_change_ack;
	unsigned char			config_pending;
	port_id				port_id;
	port_id				designated_port;
	bridge_id			designated_root;
	bridge_id			designated_bridge;
	u32				path_cost;
	u32				designated_cost;

	struct timer_list		forward_delay_timer;
	struct timer_list		hold_timer;
	struct timer_list		message_age_timer;
	struct kobject			kobj;
	struct rcu_head			rcu;

//Chris:  stp+mesh
#if 0
#ifdef STP_DISABLE_ETH
//Chris:  stp+mesh
	int	 				disable_by_mesh; 
		//0: no, 1:eth port disabled because mesh entering blocking state
	struct timer_list		eth_disable_timer;
#endif
#endif //CONFIG_RTK_MESH
};

struct net_bridge
{
	spinlock_t			lock;
	struct list_head		port_list;
	struct net_device		*dev;
	spinlock_t			hash_lock;
	struct hlist_head		hash[BR_HASH_SIZE];
	struct list_head		age_list;
	unsigned long			feature_mask;
#ifdef CONFIG_BRIDGE_NETFILTER
	struct rtable 			fake_rtable;
#endif
	unsigned long			flags;
#define BR_SET_MAC_ADDR		0x00000001

	/* STP */
	bridge_id			designated_root;
	bridge_id			bridge_id;
	u32				root_path_cost;
	unsigned long			max_age;
	unsigned long			hello_time;
	unsigned long			forward_delay;
	unsigned long			bridge_max_age;
	unsigned long			ageing_time;
	unsigned long			bridge_hello_time;
	unsigned long			bridge_forward_delay;

	u8				group_addr[ETH_ALEN];
	u16				root_port;

	enum {
		BR_NO_STP, 		/* no spanning tree */
		BR_KERNEL_STP,		/* old STP in kernel */
		BR_USER_STP,		/* new RSTP in userspace */
	} stp_enabled;

	unsigned char			topology_change;
	unsigned char			topology_change_detected;

	struct timer_list		hello_timer;
	struct timer_list		tcn_timer;
	struct timer_list		topology_change_timer;
	struct timer_list		gc_timer;
	struct kobject			*ifobj;
#if defined (CONFIG_RTK_MESH)
	//by brian, record pid for dynamic enable portal
	int mesh_pathsel_pid;
	int eth0_received;

	int	eth0_monitor_interval;
	struct timer_list	eth0_monitor_timer;

#if 0
#if defined (STP_DISABLE_ETH)
	struct timer_list	eth0_autostp_timer;
#endif
#endif

#if defined (STP_ADDCOST_ETH)
	int is_cost_changed;
#endif

#endif //CONFIG_RTK_MESH

#if defined (CONFIG_RTL_IGMP_SNOOPING)
	int igmpProxy_pid;
	struct timer_list	mCastQuerytimer;
#endif
};

extern struct notifier_block br_device_notifier;
extern const u8 br_group_address[ETH_ALEN];

/* called under bridge lock */
static inline int br_is_root_bridge(const struct net_bridge *br)
{
	return !memcmp(&br->bridge_id, &br->designated_root, 8);
}

/* br_device.c */
extern void br_dev_setup(struct net_device *dev);
extern int br_dev_xmit(struct sk_buff *skb, struct net_device *dev);

/* br_fdb.c */
extern int br_fdb_init(void);
extern void br_fdb_fini(void);
extern void br_fdb_flush(struct net_bridge *br);
extern void br_fdb_changeaddr(struct net_bridge_port *p,
			      const unsigned char *newaddr);
extern void br_fdb_cleanup(unsigned long arg);
extern void br_fdb_delete_by_port(struct net_bridge *br,
				  const struct net_bridge_port *p, int do_all);
extern struct net_bridge_fdb_entry *__br_fdb_get(struct net_bridge *br,
						 const unsigned char *addr);
extern struct net_bridge_fdb_entry *br_fdb_get(struct net_bridge *br,
					       unsigned char *addr);
extern void br_fdb_put(struct net_bridge_fdb_entry *ent);
extern int br_fdb_fillbuf(struct net_bridge *br, void *buf,
			  unsigned long count, unsigned long off);
extern int br_fdb_insert(struct net_bridge *br,
			 struct net_bridge_port *source,
			 const unsigned char *addr);
extern void br_fdb_update(struct net_bridge *br,
			  struct net_bridge_port *source,
			  const unsigned char *addr);

/* br_forward.c */
extern void br_deliver(const struct net_bridge_port *to,
		struct sk_buff *skb);
extern int br_dev_queue_push_xmit(struct sk_buff *skb);
extern void br_forward(const struct net_bridge_port *to,
		struct sk_buff *skb);
extern int br_forward_finish(struct sk_buff *skb);
extern void br_flood_deliver(struct net_bridge *br, struct sk_buff *skb);
extern void br_flood_forward(struct net_bridge *br, struct sk_buff *skb);

#if defined (CONFIG_RTL_IGMP_SNOOPING)
void br_multicast_deliver(struct net_bridge *br,
			unsigned int fwdPortMask, 
			struct sk_buff *skb,
			int clone);

void br_multicast_forward(struct net_bridge *br,
                        unsigned int fwdPortMask,
                        struct sk_buff *skb,
                        int clone);
#endif

/* br_if.c */
extern void br_port_carrier_check(struct net_bridge_port *p);
extern int br_add_bridge(struct net *net, const char *name);
extern int br_del_bridge(struct net *net, const char *name);
extern void br_net_exit(struct net *net);
extern int br_add_if(struct net_bridge *br,
	      struct net_device *dev);
extern int br_del_if(struct net_bridge *br,
	      struct net_device *dev);
extern int br_min_mtu(const struct net_bridge *br);
extern void br_features_recompute(struct net_bridge *br);

#if defined(CONFIG_RTK_MESH) && defined(STP_ADDCOST_ETH)
extern  int br_initial_port_cost(struct net_device *dev);
#endif

/* br_input.c */
extern int br_handle_frame_finish(struct sk_buff *skb);
extern struct sk_buff *br_handle_frame(struct net_bridge_port *p,
				       struct sk_buff *skb);

/* br_ioctl.c */
extern int br_dev_ioctl(struct net_device *dev, struct ifreq *rq, int cmd);
extern int br_ioctl_deviceless_stub(struct net *net, unsigned int cmd, void __user *arg);

/* br_netfilter.c */
#ifdef CONFIG_BRIDGE_NETFILTER
extern int br_netfilter_init(void);
extern void br_netfilter_fini(void);
extern void br_netfilter_rtable_init(struct net_bridge *);
#else
#define br_netfilter_init()	(0)
#define br_netfilter_fini()	do { } while(0)
#define br_netfilter_rtable_init(x)
#endif

/* br_stp.c */
extern void br_log_state(const struct net_bridge_port *p);
extern struct net_bridge_port *br_get_port(struct net_bridge *br,
					   u16 port_no);
extern void br_init_port(struct net_bridge_port *p);
extern void br_become_designated_port(struct net_bridge_port *p);

/* br_stp_if.c */
extern void br_stp_enable_bridge(struct net_bridge *br);
extern void br_stp_disable_bridge(struct net_bridge *br);
extern void br_stp_set_enabled(struct net_bridge *br, unsigned long val);
extern void br_stp_enable_port(struct net_bridge_port *p);
extern void br_stp_disable_port(struct net_bridge_port *p);
extern void br_stp_recalculate_bridge_id(struct net_bridge *br);
extern void br_stp_change_bridge_id(struct net_bridge *br, const unsigned char *a);
extern void br_stp_set_bridge_priority(struct net_bridge *br,
				       u16 newprio);
extern void br_stp_set_port_priority(struct net_bridge_port *p,
				     u8 newprio);
extern void br_stp_set_path_cost(struct net_bridge_port *p,
				 u32 path_cost);
extern ssize_t br_show_bridge_id(char *buf, const struct bridge_id *id);

/* br_stp_bpdu.c */
struct stp_proto;
extern void br_stp_rcv(const struct stp_proto *proto, struct sk_buff *skb,
		       struct net_device *dev);

/* br_stp_timer.c */
extern void br_stp_timer_init(struct net_bridge *br);
extern void br_stp_port_timer_init(struct net_bridge_port *p);
extern unsigned long br_timer_value(const struct timer_list *timer);

/* br.c */
extern struct net_bridge_fdb_entry *(*br_fdb_get_hook)(struct net_bridge *br,
						       unsigned char *addr);
extern void (*br_fdb_put_hook)(struct net_bridge_fdb_entry *ent);


/* br_netlink.c */
extern int br_netlink_init(void);
extern void br_netlink_fini(void);
extern void br_ifinfo_notify(int event, struct net_bridge_port *port);

#ifdef CONFIG_SYSFS
/* br_sysfs_if.c */
extern struct sysfs_ops brport_sysfs_ops;
extern int br_sysfs_addif(struct net_bridge_port *p);

/* br_sysfs_br.c */
extern int br_sysfs_addbr(struct net_device *dev);
extern void br_sysfs_delbr(struct net_device *dev);

#else

#define br_sysfs_addif(p)	(0)
#define br_sysfs_addbr(dev)	(0)
#define br_sysfs_delbr(dev)	do { } while(0)
#endif /* CONFIG_SYSFS */

#endif
