/*
 *	Userspace interface
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
#include <linux/netdevice.h>
#include <linux/ethtool.h>
#include <linux/if_arp.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/rtnetlink.h>
#include <linux/if_ether.h>
#include <net/sock.h>

#include "br_private.h"

#if defined (CONFIG_RTL_IGMP_SNOOPING)
#include <net/rtl/rtl865x_igmpsnooping_glue.h>
#include <net/rtl/rtl865x_igmpsnooping.h>
#include <net/rtl/rtl865x_netif.h>
#include <net/rtl/rtl_nic.h>
struct net_bridge *bridge0=NULL;
unsigned int brIgmpModuleIndex=0xFFFFFFFF;
unsigned int br0SwFwdPortMask=0xFFFFFFFF;
extern void br_mCastQueryTimerExpired(unsigned long arg);
#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
struct net_bridge *bridge1=NULL;
unsigned int brIgmpModuleIndex_2=0xFFFFFFFF;
unsigned int br1SwFwdPortMask=0xFFFFFFFF;
#endif
#endif

#if defined (CONFIG_RTL_STP)
#include <net/rtl/rtk_stp.h>
#endif

#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
int rtl865x_generateBridgeDeviceInfo( struct net_bridge *br, rtl_multicastDeviceInfo_t *devInfo);
#endif
/*
 * Determine initial path cost based on speed.
 * using recommendations from 802.1d standard
 *
 * Since driver might sleep need to not be holding any locks.
 */
static int port_cost(struct net_device *dev)
{
	if (dev->ethtool_ops && dev->ethtool_ops->get_settings) {
		struct ethtool_cmd ecmd = { .cmd = ETHTOOL_GSET, };

		if (!dev->ethtool_ops->get_settings(dev, &ecmd)) {
			switch(ecmd.speed) {
			case SPEED_10000:
				return 2;
			case SPEED_1000:
				return 4;
			case SPEED_100:
				return 19;
			case SPEED_10:
				return 100;
			}
		}
	}

	/* Old silly heuristics based on name */
	if (!strncmp(dev->name, "lec", 3))
		return 7;

	if (!strncmp(dev->name, "plip", 4))
		return 2500;

	return 100;	/* assume old 10Mbps */
}

#if defined(CONFIG_RTK_MESH) && defined(STP_ADDCOST_ETH)
int br_initial_port_cost(struct net_device *dev)
{
	return port_cost(dev);
}
#endif

/*
 * Check for port carrier transistions.
 * Called from work queue to allow for calling functions that
 * might sleep (such as speed check), and to debounce.
 */
void br_port_carrier_check(struct net_bridge_port *p)
{
	struct net_device *dev = p->dev;
	struct net_bridge *br = p->br;

	if (netif_carrier_ok(dev))
		p->path_cost = port_cost(dev);

	if (netif_running(br->dev)) {
		spin_lock_bh(&br->lock);
		if (netif_carrier_ok(dev)) {
			if (p->state == BR_STATE_DISABLED)
				br_stp_enable_port(p);
		} else {
			if (p->state != BR_STATE_DISABLED)
				br_stp_disable_port(p);
		}
		spin_unlock_bh(&br->lock);
	}
}

static void release_nbp(struct kobject *kobj)
{
	struct net_bridge_port *p
		= container_of(kobj, struct net_bridge_port, kobj);
	kfree(p);
}

static struct kobj_type brport_ktype = {
#ifdef CONFIG_SYSFS
	.sysfs_ops = &brport_sysfs_ops,
#endif
	.release = release_nbp,
};

static void destroy_nbp(struct net_bridge_port *p)
{
	struct net_device *dev = p->dev;

	p->br = NULL;
	p->dev = NULL;
	dev_put(dev);

	kobject_put(&p->kobj);
}

static void destroy_nbp_rcu(struct rcu_head *head)
{
	struct net_bridge_port *p =
			container_of(head, struct net_bridge_port, rcu);
	destroy_nbp(p);
}

/* Delete port(interface) from bridge is done in two steps.
 * via RCU. First step, marks device as down. That deletes
 * all the timers and stops new packets from flowing through.
 *
 * Final cleanup doesn't occur until after all CPU's finished
 * processing packets.
 *
 * Protected from multiple admin operations by RTNL mutex
 */
static void del_nbp(struct net_bridge_port *p)
{
	struct net_bridge *br = p->br;
	struct net_device *dev = p->dev;

#if 0
#ifdef STP_DISABLE_ETH
	//Chris: stp+mesh
	p->disable_by_mesh = 0;
#endif
#endif 

	sysfs_remove_link(br->ifobj, dev->name);

	dev_set_promiscuity(dev, -1);

	spin_lock_bh(&br->lock);
	br_stp_disable_port(p);
	spin_unlock_bh(&br->lock);

	br_ifinfo_notify(RTM_DELLINK, p);

	br_fdb_delete_by_port(br, p, 1);

	list_del_rcu(&p->list);

	rcu_assign_pointer(dev->br_port, NULL);

	kobject_uevent(&p->kobj, KOBJ_REMOVE);
	kobject_del(&p->kobj);

	call_rcu(&p->rcu, destroy_nbp_rcu);
}

/* called with RTNL */
static void del_br(struct net_bridge *br)
{
	struct net_bridge_port *p, *n;

	list_for_each_entry_safe(p, n, &br->port_list, list) {
		del_nbp(p);
	}

	del_timer_sync(&br->gc_timer);

	br_sysfs_delbr(br->dev);
	unregister_netdevice(br->dev);
}

#if defined (CONFIG_RTK_MESH) || defined (CONFIG_RTL_IGMP_SNOOPING)
struct net_bridge *find_br_by_name(char *name)
{
	struct net_bridge *br;
	struct net_device *dev =NULL;;

	dev = __dev_get_by_name(&init_net, name);
	if (dev == NULL) 
	{
		return NULL;

	}	
	else if (!(dev->priv_flags & IFF_EBRIDGE)) {
		
		return NULL;
	}
	else 
	{
		br = netdev_priv(dev);
	}
	return br;
}


#endif

static struct net_device *new_bridge_dev(struct net *net, const char *name)
{
	struct net_bridge *br;
	struct net_device *dev;

	dev = alloc_netdev(sizeof(struct net_bridge), name,
			   br_dev_setup);

	if (!dev)
		return NULL;
	dev_net_set(dev, net);

	br = netdev_priv(dev);
	br->dev = dev;

	spin_lock_init(&br->lock);
	INIT_LIST_HEAD(&br->port_list);
	spin_lock_init(&br->hash_lock);

	br->bridge_id.prio[0] = 0x80;
	br->bridge_id.prio[1] = 0x00;

	memcpy(br->group_addr, br_group_address, ETH_ALEN);

	br->feature_mask = dev->features;
	br->stp_enabled = BR_NO_STP;
	br->designated_root = br->bridge_id;
	br->root_path_cost = 0;
	br->root_port = 0;
	br->bridge_max_age = br->max_age = 20 * HZ;
	br->bridge_hello_time = br->hello_time = 2 * HZ;
#if defined(CONFIG_RTL_819X)
	br->bridge_forward_delay = br->forward_delay = 10 * HZ;
#else
	br->bridge_forward_delay = br->forward_delay = 15 * HZ;
#endif
	br->topology_change = 0;
	br->topology_change_detected = 0;
	br->ageing_time = 300 * HZ;

#if defined (CONFIG_RTK_MESH)
	br->eth0_monitor_interval = MESH_PORTAL_EXPIRE * HZ;
	br->mesh_pathsel_pid=0;
#if defined (STP_ADDCOST_ETH)
	br->is_cost_changed = 0;
#endif
#endif

	br_netfilter_rtable_init(br);

#if defined (CONFIG_RTL_IGMP_SNOOPING)
	br->igmpProxy_pid=0;
#endif
	INIT_LIST_HEAD(&br->age_list);

	br_stp_timer_init(br);

	return dev;
}

/* find an available port number */
static int find_portno(struct net_bridge *br)
{
	int index;
	struct net_bridge_port *p;
	unsigned long *inuse;

	inuse = kcalloc(BITS_TO_LONGS(BR_MAX_PORTS), sizeof(unsigned long),
			GFP_KERNEL);
	if (!inuse)
		return -ENOMEM;

	set_bit(0, inuse);	/* zero is reserved */
	list_for_each_entry(p, &br->port_list, list) {
		set_bit(p->port_no, inuse);
	}
	index = find_first_zero_bit(inuse, BR_MAX_PORTS);
	kfree(inuse);

	return (index >= BR_MAX_PORTS) ? -EXFULL : index;
}

/* called with RTNL but without bridge lock */
static struct net_bridge_port *new_nbp(struct net_bridge *br,
				       struct net_device *dev)
{
	int index;
	struct net_bridge_port *p;
#if defined (CONFIG_RTL_STP)
	int retval=0, Port;
	char name[IFNAMSIZ];
	struct net_device *pseudo_dev;
#endif
#if defined(CONFIG_RTL_HW_STP)
	int retval, i;
	uint32 vid, portMask;
#endif
	index = find_portno(br);
	if (index < 0)
		return ERR_PTR(index);

	p = kzalloc(sizeof(*p), GFP_KERNEL);
	if (p == NULL)
		return ERR_PTR(-ENOMEM);

	p->br = br;
	dev_hold(dev);
	p->dev = dev;
	p->path_cost = port_cost(dev);
	p->priority = 0x8000 >> BR_PORT_BITS;
	p->port_no = index;
	br_init_port(p);
	p->state = BR_STATE_DISABLED;

//Chris: stp+mesh
#if 0
 #if defined STP_DISABLE_ETH
	p->disable_by_mesh = 0;
 #endif
#endif

#if defined (CONFIG_RTL_STP)
	{
		strcpy(name, p->dev->name);
		Port=STP_PortDev_Mapping[name[strlen(name)-1]-'0'];
		#ifdef CONFIG_RTK_MESH
		if ((Port != WLAN_PSEUDO_IF_INDEX) && (Port !=WLAN_MESH_PSEUDO_IF_INDEX) && (Port!=NO_MAPPING))
		#else
		if ((Port != WLAN_PSEUDO_IF_INDEX) && (Port!=NO_MAPPING))
		#endif
		{
			retval = rtl865x_setMulticastSpanningTreePortState(Port , RTL8651_PORTSTA_DISABLED);

			retval = rtl865x_setSpanningTreePortState(Port, RTL8651_PORTSTA_DISABLED);
		}
		else  if (Port == WLAN_PSEUDO_IF_INDEX)
		{
			if ((pseudo_dev = __dev_get_by_name(&init_net, WLAN_IF_NAME)) == NULL)
			{	
				return NULL;
			}
			pseudo_dev->br_port->state = BR_STATE_DISABLED;
			retval = SUCCESS;
		}
		#ifdef CONFIG_RTK_MESH
		else  if (Port == WLAN_MESH_PSEUDO_IF_INDEX)
		{
			if ((pseudo_dev = __dev_get_by_name(&init_net, WLAN_MESH_IF_NAME)) == NULL)
			{	
				return NULL;
			}
			pseudo_dev->br_port->state = BR_STATE_DISABLED;
			retval = SUCCESS;
		}
		#endif
		else if(Port == NO_MAPPING)
		{
			p->state = BR_STATE_DISABLED;
		}
	}
	
#endif

#if defined(CONFIG_RTL_HW_STP)
	vid=0;
	portMask=0;

	#if define(CONFIG_RTL_NETIF_MAPPING)
	{
		ps_drv_netif_mapping_t *entry;
		entry = rtl_get_ps_drv_netif_mapping_by_psdev(p->dev);

		retval = rtl865x_getNetifVid(entry?entry->drvName:p->dev->name,&vid);
	}
	#else
	if(strcmp(p->dev->name,"eth0")==0)
		retval=rtl865x_getNetifVid("br0", &vid);
	else
		retval=rtl865x_getNetifVid(p->dev->name, &vid);
	#endif
	
	if(retval==FAILED){
//		printk("%s(%d): rtl865x_getNetifVid failed.\n",__FUNCTION__,__LINE__);
	}
	else{
		portMask=rtl865x_getVlanPortMask(vid);
		for ( i = 0 ; i < MAX_RTL_STP_PORT_WH; i ++ ){
			if((1<<i)&portMask){
				retval = rtl865x_setMulticastSpanningTreePortState(i , BR_STATE_DISABLED);
				if(retval==FAILED)
					printk("%s(%d): rtl865x_setMulticastSpanningTreePortState port(%d) failed.\n",__FUNCTION__,__LINE__,i);
				
				retval = rtl865x_setSpanningTreePortState(i, BR_STATE_DISABLED);
				if(retval==FAILED)
					printk("%s(%d): rtl865x_setSpanningTreePortState port(%d) failed.\n",__FUNCTION__,__LINE__,i);
			}
		}
	}
#endif

	br_stp_port_timer_init(p);

	return p;
}

#if defined (CONFIG_RTK_MESH)
/*notice:this function is different from kernel 2.4 sdk*/
int br_set_meshpathsel_pid(int pid)
{
	struct net_device *dev=NULL;
	struct net_bridge *br=NULL;
	
	int ret=0;

	dev = __dev_get_by_name(&init_net,RTL_PS_BR0_DEV_NAME);
	if (dev == NULL) 
	{
		ret =  -ENXIO; 	/* Could not find device */

	}	
	else if (!(dev->priv_flags & IFF_EBRIDGE)) {
		
		ret = -EPERM;
	}
	else 
	{
		br = netdev_priv(dev);
	}

	br->mesh_pathsel_pid = pid;

	if( !pid )
		br->eth0_received = 0;

	printk("Receive Pathsel daemon pid:%d\n",br->mesh_pathsel_pid);
	return 1;
}
#endif

#if defined (CONFIG_RTL_IGMP_SNOOPING)
int br_set_igmpProxy_pid(int pid)
{
	struct net_device *dev;
	struct net_bridge *br=NULL;
	int ret=0;

	dev = __dev_get_by_name(&init_net,RTL_PS_BR0_DEV_NAME);
	if (dev == NULL) 
	{
		ret =  -ENXIO; 	/* Could not find device */

	}	
	else if (!(dev->priv_flags & IFF_EBRIDGE)) {
		
		ret = -EPERM;
	}
	else 
	{
		br = netdev_priv(dev);
	}
	
	if(br!=NULL)
	{
		br->igmpProxy_pid= pid;
	}

	return 1;
}
#endif

int br_add_bridge(struct net *net, const char *name)
{
	struct net_device *dev;
	int ret;

	dev = new_bridge_dev(net, name);
	if (!dev)
		return -ENOMEM;

	rtnl_lock();
	if (strchr(dev->name, '%')) {
		ret = dev_alloc_name(dev, dev->name);
		if (ret < 0)
			goto out_free;
	}

	ret = register_netdevice(dev);
	if (ret)
		goto out_free;

	ret = br_sysfs_addbr(dev);
	if (ret)
		unregister_netdevice(dev);

#if defined (CONFIG_RTL_IGMP_SNOOPING)
	if(strcmp(name,RTL_PS_BR0_DEV_NAME)==0)
	{
		rtl_multicastDeviceInfo_t devInfo;
		memset(&devInfo, 0, sizeof(rtl_multicastDeviceInfo_t ));
		strcpy(devInfo.devName, RTL_PS_BR0_DEV_NAME);
		
		ret=rtl_registerIgmpSnoopingModule(&brIgmpModuleIndex);
		#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		if(ret==0)
		{
			 rtl_setIgmpSnoopingModuleDevInfo(brIgmpModuleIndex,&devInfo);
		}
		#endif
		bridge0=netdev_priv(dev);
		if(bridge0!=NULL)
		{
			init_timer(&bridge0->mCastQuerytimer);
			bridge0->mCastQuerytimer.data=bridge0;
			bridge0->mCastQuerytimer.expires=jiffies+MCAST_QUERY_INTERVAL*HZ;
			bridge0->mCastQuerytimer.function=(void*)br_mCastQueryTimerExpired;
			add_timer(&bridge0->mCastQuerytimer);
		}
		
		rtl_setIpv4UnknownMCastFloodMap(brIgmpModuleIndex, 0xFFFFFFFF);
		rtl_setIpv6UnknownMCastFloodMap(brIgmpModuleIndex, 0xFFFFFFFF);
	}
    #ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	if(strcmp(name,RTL_PS_BR1_DEV_NAME)==0)
	{
		rtl_multicastDeviceInfo_t devInfo2;
		memset(&devInfo2, 0, sizeof(rtl_multicastDeviceInfo_t ));
		strcpy(devInfo2.devName, RTL_PS_BR1_DEV_NAME);
		
		ret=rtl_registerIgmpSnoopingModule(&brIgmpModuleIndex_2);
		#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		if(ret==0)
		{
			 rtl_setIgmpSnoopingModuleDevInfo(brIgmpModuleIndex_2,&devInfo2);
		}
		#endif
		bridge1=netdev_priv(dev);
		if(bridge1!=NULL)
		{
			init_timer(&bridge1->mCastQuerytimer);
			bridge1->mCastQuerytimer.data=bridge1;
			bridge1->mCastQuerytimer.expires=jiffies+MCAST_QUERY_INTERVAL*HZ;
			bridge1->mCastQuerytimer.function=(void*)br_mCastQueryTimerExpired;
			add_timer(&bridge1->mCastQuerytimer);
		}
	}
#endif
#endif
 out:
	rtnl_unlock();
	return ret;

out_free:
	free_netdev(dev);
	goto out;
}

int br_del_bridge(struct net *net, const char *name)
{
	struct net_device *dev;
	int ret = 0;

	rtnl_lock();
	dev = __dev_get_by_name(net, name);
	if (dev == NULL)
		ret =  -ENXIO; 	/* Could not find device */

	else if (!(dev->priv_flags & IFF_EBRIDGE)) {
		/* Attempt to delete non bridge device! */
		ret = -EPERM;
	}

	else if (dev->flags & IFF_UP) {
		/* Not shutdown yet. */
		ret = -EBUSY;
	}

	else
		del_br(netdev_priv(dev));

	#if defined (CONFIG_RTL_IGMP_SNOOPING)
	if(strcmp(name,RTL_PS_BR0_DEV_NAME)==0)
	{
		rtl_unregisterIgmpSnoopingModule(brIgmpModuleIndex);
		#if defined (CONFIG_RTL_MLD_SNOOPING)
		if(bridge0 && timer_pending(&bridge0->mCastQuerytimer))
		{
			del_timer(&bridge0->mCastQuerytimer);
		}
		#endif
		bridge0=NULL;
	}
    #ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	if(strcmp(name,RTL_PS_BR1_DEV_NAME)==0)
	{
		rtl_unregisterIgmpSnoopingModule(brIgmpModuleIndex_2);
		#if defined (CONFIG_RTL_MLD_SNOOPING)
		if(bridge1 && timer_pending(&bridge1->mCastQuerytimer))
		{
			del_timer(&bridge1->mCastQuerytimer);
		}
		#endif
		bridge1=NULL;
	}
	#endif
	#endif

	rtnl_unlock();
	return ret;
}

/* MTU of the bridge pseudo-device: ETH_DATA_LEN or the minimum of the ports */
int br_min_mtu(const struct net_bridge *br)
{
	const struct net_bridge_port *p;
	int mtu = 0;

	ASSERT_RTNL();

	if (list_empty(&br->port_list))
		mtu = ETH_DATA_LEN;
	else {
		list_for_each_entry(p, &br->port_list, list) {
			if (!mtu  || p->dev->mtu < mtu)
				mtu = p->dev->mtu;
		}
	}
	return mtu;
}

/*
 * Recomputes features using slave's features
 */
void br_features_recompute(struct net_bridge *br)
{
	struct net_bridge_port *p;
	unsigned long features, mask;

	features = mask = br->feature_mask;
	if (list_empty(&br->port_list))
		goto done;

	features &= ~NETIF_F_ONE_FOR_ALL;

	list_for_each_entry(p, &br->port_list, list) {
		features = netdev_increment_features(features,
						     p->dev->features, mask);
	}

done:
	br->dev->features = netdev_fix_features(features, NULL);
}

/* called with RTNL */
int br_add_if(struct net_bridge *br, struct net_device *dev)
{
	struct net_bridge_port *p;
	int err = 0;

	if (dev->flags & IFF_LOOPBACK || dev->type != ARPHRD_ETHER)
		return -EINVAL;

	if (dev->netdev_ops->ndo_start_xmit == br_dev_xmit)
		return -ELOOP;

	if (dev->br_port != NULL)
		return -EBUSY;

	p = new_nbp(br, dev);
	if (IS_ERR(p))
		return PTR_ERR(p);

	err = dev_set_promiscuity(dev, 1);
	if (err)
		goto put_back;

	err = kobject_init_and_add(&p->kobj, &brport_ktype, &(dev->dev.kobj),
				   SYSFS_BRIDGE_PORT_ATTR);
	if (err)
		goto err0;

	err = br_fdb_insert(br, p, dev->dev_addr);
	if (err)
		goto err1;

	err = br_sysfs_addif(p);
	if (err)
		goto err2;

	rcu_assign_pointer(dev->br_port, p);
	dev_disable_lro(dev);

	list_add_rcu(&p->list, &br->port_list);

	spin_lock_bh(&br->lock);
	br_stp_recalculate_bridge_id(br);
	br_features_recompute(br);

	if ((dev->flags & IFF_UP) && netif_carrier_ok(dev) &&
	    (br->dev->flags & IFF_UP))
		br_stp_enable_port(p);
	spin_unlock_bh(&br->lock);

	br_ifinfo_notify(RTM_NEWLINK, p);

	dev_set_mtu(br->dev, br_min_mtu(br));

	kobject_uevent(&p->kobj, KOBJ_ADD);

	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(strcmp(br->dev->name,RTL_PS_BR0_DEV_NAME)==0)
	{
		rtl_multicastDeviceInfo_t brDevInfo;
		rtl865x_generateBridgeDeviceInfo(br, &brDevInfo);
		if(brIgmpModuleIndex!=0xFFFFFFFF)
		{
			rtl_setIgmpSnoopingModuleDevInfo(brIgmpModuleIndex,&brDevInfo);
		}
		br0SwFwdPortMask=brDevInfo.swPortMask;
	}
	#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	if(strcmp(br->dev->name,RTL_PS_BR1_DEV_NAME)==0)
	{
		rtl_multicastDeviceInfo_t brDevInfo;
		rtl865x_generateBridgeDeviceInfo(br, &brDevInfo);
		if(brIgmpModuleIndex!=0xFFFFFFFF)
		{
			rtl_setIgmpSnoopingModuleDevInfo(brIgmpModuleIndex_2,&brDevInfo);
		}
		br1SwFwdPortMask=brDevInfo.swPortMask;
	}
	#endif
	#endif

	return 0;
err2:
	br_fdb_delete_by_port(br, p, 1);
err1:
	kobject_del(&p->kobj);
err0:
	dev_set_promiscuity(dev, -1);
put_back:
	dev_put(dev);
	kfree(p);
	return err;
}

/* called with RTNL */
int br_del_if(struct net_bridge *br, struct net_device *dev)
{
	struct net_bridge_port *p = dev->br_port;

	if (!p || p->br != br)
		return -EINVAL;

	del_nbp(p);

	spin_lock_bh(&br->lock);
	br_stp_recalculate_bridge_id(br);
	br_features_recompute(br);
	spin_unlock_bh(&br->lock);

	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(strcmp(br->dev->name,RTL_PS_BR0_DEV_NAME)==0)
	{
		rtl_multicastDeviceInfo_t brDevInfo;
		rtl865x_generateBridgeDeviceInfo(br, &brDevInfo);
		if(brIgmpModuleIndex!=0xFFFFFFFF)
		{
			rtl_setIgmpSnoopingModuleDevInfo(brIgmpModuleIndex,&brDevInfo);
		}
		br0SwFwdPortMask=brDevInfo.swPortMask;
	}
#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	if(strcmp(br->dev->name,RTL_PS_BR1_DEV_NAME)==0)
	{
		rtl_multicastDeviceInfo_t brDevInfo;
		rtl865x_generateBridgeDeviceInfo(br, &brDevInfo);
		if(brIgmpModuleIndex!=0xFFFFFFFF)
		{
			rtl_setIgmpSnoopingModuleDevInfo(brIgmpModuleIndex_2,&brDevInfo);
		}
		br1SwFwdPortMask=brDevInfo.swPortMask;
	}
#endif
	#endif

	return 0;
}

void br_net_exit(struct net *net)
{
	struct net_device *dev;

	rtnl_lock();
restart:
	for_each_netdev(net, dev) {
		if (dev->priv_flags & IFF_EBRIDGE) {
			del_br(netdev_priv(dev));
			goto restart;
		}
	}
	rtnl_unlock();

}

#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
int rtl865x_generateBridgeDeviceInfo( struct net_bridge *br, rtl_multicastDeviceInfo_t *devInfo)
{
	struct net_bridge_port *p, *n;
	
	if((br==NULL) || (devInfo==NULL))
	{
		return -1;
	}
	
	memset(devInfo, 0, sizeof(rtl_multicastDeviceInfo_t));

#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	if(strcmp(br->dev->name,RTL_PS_BR0_DEV_NAME)!=0 && strcmp(br->dev->name,RTL_PS_BR1_DEV_NAME)!=0 )
#else
	if(strcmp(br->dev->name,RTL_PS_BR0_DEV_NAME)!=0)
#endif
	{
		return -1;
	}

	strcpy(devInfo->devName,br->dev->name);
	
		
	list_for_each_entry_safe(p, n, &br->port_list, list) 
	{
		if(memcmp(p->dev->name, RTL_PS_ETH_NAME,3)!=0)
		{
			devInfo->swPortMask|=(1<<p->port_no);
		}
		devInfo->portMask|=(1<<p->port_no);
		
	}
	
	return 0;
}
#endif
