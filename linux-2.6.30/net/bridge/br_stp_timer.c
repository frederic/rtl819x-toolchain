/*
 *	Spanning tree protocol; timer-related code
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
#include <linux/times.h>

#include "br_private.h"
#include "br_private_stp.h"

#if defined (CONFIG_RTL_STP)
#include <net/rtl/rtl_nic.h>
#include <net/rtl/rtk_stp.h>
#endif

#if defined (CONFIG_RTK_MESH)
//static void br_eth_disable_timer_expired(unsigned long arg);
static void br_eth0_monitor_timer_expired(unsigned long arg);
//static void br_eth0_autostp_timer_expired(unsigned long arg);
#endif	//CONFIG_RTK_MESH

/* called under bridge lock */
static int br_is_designated_for_some_port(const struct net_bridge *br)
{
	struct net_bridge_port *p;

	list_for_each_entry(p, &br->port_list, list) {
		if (p->state != BR_STATE_DISABLED &&
		    !memcmp(&p->designated_bridge, &br->bridge_id, 8))
			return 1;
	}

	return 0;
}

static void br_hello_timer_expired(unsigned long arg)
{
	struct net_bridge *br = (struct net_bridge *)arg;

	pr_debug("%s: hello timer expired\n", br->dev->name);
	spin_lock(&br->lock);
	if (br->dev->flags & IFF_UP) {
		br_config_bpdu_generation(br);

		mod_timer(&br->hello_timer, round_jiffies(jiffies + br->hello_time));
	}
	spin_unlock(&br->lock);
}

static void br_message_age_timer_expired(unsigned long arg)
{
	struct net_bridge_port *p = (struct net_bridge_port *) arg;
	struct net_bridge *br = p->br;
	const bridge_id *id = &p->designated_bridge;
	int was_root;

	if (p->state == BR_STATE_DISABLED)
		return;


	pr_info("%s: neighbor %.2x%.2x.%.2x:%.2x:%.2x:%.2x:%.2x:%.2x lost on port %d(%s)\n",
		br->dev->name,
		id->prio[0], id->prio[1],
		id->addr[0], id->addr[1], id->addr[2],
		id->addr[3], id->addr[4], id->addr[5],
		p->port_no, p->dev->name);

#if defined(CONFIG_RTK_MESH) && defined(STP_ADDCOST_ETH)
	br_reset_cost(p);
#endif
	
	/*
	 * According to the spec, the message age timer cannot be
	 * running when we are the root bridge. So..  this was_root
	 * check is redundant. I'm leaving it in for now, though.
	 */
	spin_lock(&br->lock);
	if (p->state == BR_STATE_DISABLED)
		goto unlock;
	was_root = br_is_root_bridge(br);

	br_become_designated_port(p);
	br_configuration_update(br);
	br_port_state_selection(br);
	if (br_is_root_bridge(br) && !was_root)
		br_become_root_bridge(br);
 unlock:
	spin_unlock(&br->lock);
}

static void br_forward_delay_timer_expired(unsigned long arg)
{
	struct net_bridge_port *p = (struct net_bridge_port *) arg;
	struct net_bridge *br = p->br;

#if defined (CONFIG_RTL_STP)
	int retval=0, Port;
	char name[IFNAMSIZ];
	struct net_device *pseudo_dev;		
#endif
#if defined(CONFIG_RTL_HW_STP)
	int retval, i;
	uint32 vid, portMask;
#endif

	pr_debug("%s: %d(%s) forward delay timer\n",
		 br->dev->name, p->port_no, p->dev->name);
	spin_lock(&br->lock);
	if (p->state == BR_STATE_LISTENING) {
		p->state = BR_STATE_LEARNING;

#if defined (CONFIG_RTL_STP)
	{
		strcpy(name, p->dev->name);
		Port=STP_PortDev_Mapping[name[strlen(name)-1]-'0'];
		#if defined (CONFIG_RTK_MESH)
		if ((Port != WLAN_PSEUDO_IF_INDEX) && (Port != WLAN_MESH_PSEUDO_IF_INDEX) && (Port!=NO_MAPPING))
		#else
		if ((Port != WLAN_PSEUDO_IF_INDEX) && (Port!=NO_MAPPING))
		#endif	
		{
			retval = rtl865x_setMulticastSpanningTreePortState(Port , RTL8651_PORTSTA_LEARNING);

			retval = rtl865x_setSpanningTreePortState(Port, RTL8651_PORTSTA_LEARNING);
		}
		else  if (Port == WLAN_PSEUDO_IF_INDEX)
		{
			if ((pseudo_dev = __dev_get_by_name(&init_net,WLAN_IF_NAME)) == NULL)
			{	
				return;
			}
			pseudo_dev->br_port->state = BR_STATE_LEARNING;
			retval = SUCCESS;
		}
		#if defined (CONFIG_RTK_MESH)
		else  if (Port == WLAN_MESH_PSEUDO_IF_INDEX)
		{
			if ((pseudo_dev = __dev_get_by_name(&init_net,WLAN_MESH_IF_NAME)) == NULL)
			{	
				return;
			}
			pseudo_dev->br_port->state = BR_STATE_LEARNING;
			retval = SUCCESS;
		}
		#endif
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
				retval = rtl865x_setMulticastSpanningTreePortState(i , RTL8651_PORTSTA_LEARNING);
				if(retval==FAILED)
					printk("%s(%d): rtl865x_setMulticastSpanningTreePortState port(%d) failed.\n",__FUNCTION__,__LINE__,i);
				
				retval = rtl865x_setSpanningTreePortState(i, RTL8651_PORTSTA_LEARNING);
				if(retval==FAILED)
					printk("%s(%d): rtl865x_setSpanningTreePortState port(%d) failed.\n",__FUNCTION__,__LINE__,i);
			}
		}
	}
#endif

		mod_timer(&p->forward_delay_timer,
			  jiffies + br->forward_delay);
	} else if (p->state == BR_STATE_LEARNING) {
		p->state = BR_STATE_FORWARDING;
#if defined (CONFIG_RTL_STP)
	{
		strcpy(name, p->dev->name);
		Port=STP_PortDev_Mapping[name[strlen(name)-1]-'0'];

		#if defined (CONFIG_RTK_MESH)
		if ((Port != WLAN_PSEUDO_IF_INDEX) && (Port != WLAN_MESH_PSEUDO_IF_INDEX) &&(Port!=NO_MAPPING))
		#else
		if ((Port != WLAN_PSEUDO_IF_INDEX) && (Port!=NO_MAPPING))
		#endif
		{
			retval = rtl865x_setMulticastSpanningTreePortState(Port , RTL8651_PORTSTA_FORWARDING);

			retval = rtl865x_setSpanningTreePortState(Port, RTL8651_PORTSTA_FORWARDING);
		}
		else  if (Port == WLAN_PSEUDO_IF_INDEX)
		{
			if ((pseudo_dev = __dev_get_by_name(&init_net,WLAN_IF_NAME)) == NULL)
			{	
				return;
			}
			pseudo_dev->br_port->state = BR_STATE_FORWARDING;
			retval = SUCCESS;
		}
		#if defined (CONFIG_RTK_MESH)
		else  if (Port == WLAN_MESH_PSEUDO_IF_INDEX)
		{
			if ((pseudo_dev = __dev_get_by_name(&init_net,WLAN_MESH_IF_NAME)) == NULL)
			{	
				return;
			}
			pseudo_dev->br_port->state = BR_STATE_FORWARDING;
			retval = SUCCESS;
		}
		#endif
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
				retval = rtl865x_setMulticastSpanningTreePortState(i , RTL8651_PORTSTA_FORWARDING);
				if(retval==FAILED)
					printk("%s(%d): rtl865x_setMulticastSpanningTreePortState port(%d) failed.\n",__FUNCTION__,__LINE__,i);
				
				retval = rtl865x_setSpanningTreePortState(i, RTL8651_PORTSTA_FORWARDING);
				if(retval==FAILED)
					printk("%s(%d): rtl865x_setSpanningTreePortState port(%d) failed.\n",__FUNCTION__,__LINE__,i);
			}
		}
	}
#endif

		if (br_is_designated_for_some_port(br))
			br_topology_change_detection(br);
	}
	br_log_state(p);
	spin_unlock(&br->lock);
}

static void br_tcn_timer_expired(unsigned long arg)
{
	struct net_bridge *br = (struct net_bridge *) arg;

	pr_debug("%s: tcn timer expired\n", br->dev->name);
	spin_lock(&br->lock);
	if (br->dev->flags & IFF_UP) {
		br_transmit_tcn(br);

		mod_timer(&br->tcn_timer,jiffies + br->bridge_hello_time);
	}
	spin_unlock(&br->lock);
}

static void br_topology_change_timer_expired(unsigned long arg)
{
	struct net_bridge *br = (struct net_bridge *) arg;

	pr_debug("%s: topo change timer expired\n", br->dev->name);
	spin_lock(&br->lock);
	br->topology_change_detected = 0;
	br->topology_change = 0;
	spin_unlock(&br->lock);
}

static void br_hold_timer_expired(unsigned long arg)
{
	struct net_bridge_port *p = (struct net_bridge_port *) arg;

	pr_debug("%s: %d(%s) hold timer expired\n",
		 p->br->dev->name,  p->port_no, p->dev->name);

	spin_lock(&p->br->lock);
	if (p->config_pending)
		br_transmit_config(p);
	spin_unlock(&p->br->lock);
}

void br_stp_timer_init(struct net_bridge *br)
{
	setup_timer(&br->hello_timer, br_hello_timer_expired,
		      (unsigned long) br);

	setup_timer(&br->tcn_timer, br_tcn_timer_expired,
		      (unsigned long) br);

	setup_timer(&br->topology_change_timer,
		      br_topology_change_timer_expired,
		      (unsigned long) br);

	setup_timer(&br->gc_timer, br_fdb_cleanup, (unsigned long) br);

	#if defined (CONFIG_RTK_MESH)
	setup_timer(&br->eth0_monitor_timer,
		      br_eth0_monitor_timer_expired,
		      (unsigned long) br);
	/*
	#if defined (STP_DISABLE_ETH)
	setup_timer(&br->eth0_autostp_timer,
		      br_eth0_autostp_timer_expired,
		      (unsigned long) br);
	#endif
	*/
	#endif
}

#if defined (CONFIG_RTK_MESH)
static void br_eth0_monitor_timer_expired(unsigned long arg)
{
	struct net_bridge *br = (struct net_bridge *) arg;
	br->eth0_received = 0;
	br->stp_enabled = 0;
	br_signal_pathsel(br);
	printk("%d seconds never receive packet from eth0 \n",MESH_PORTAL_EXPIRE);
	return;
	
}

#if 0

#if defined (STP_DISABLE_ETH)
static void br_eth_disable_timer_expired(unsigned long arg)
{

//Chris: stp+mesh
	struct net_bridge_port *p = (struct net_bridge_port *) arg;
	printk(KERN_INFO "%s: enabled for MESH-STP configuration\n", p->br->dev->name);
	if(p->disable_by_mesh == 1)
	{
		br_stp_enable_port(p);
	}
	p->disable_by_mesh=0;
	return;
	
}
#endif

static void br_eth0_autostp_timer_expired(unsigned long arg)
{

//Chris: stp+mesh
	struct net_bridge *br = (struct net_bridge *) arg;
	mod_timer(&br->eth0_autostp_timer, jiffies+ br->eth0_monitor_interval);
	br->stp_enabled = 0;

	return;
}
#endif

#endif	//CONFIG_RTK_MESH

void br_stp_port_timer_init(struct net_bridge_port *p)
{
	setup_timer(&p->message_age_timer, br_message_age_timer_expired,
		      (unsigned long) p);

	setup_timer(&p->forward_delay_timer, br_forward_delay_timer_expired,
		      (unsigned long) p);

	setup_timer(&p->hold_timer, br_hold_timer_expired,
		      (unsigned long) p);

	#if defined (CONFIG_RTK_MESH)
	#if defined (STP_DISABLE_ETH)
	setup_timer(&p->eth_disable_timer, br_eth_disable_timer_expired,
		      (unsigned long) p);
	#endif
	#endif
}

/* Report ticks left (in USER_HZ) used for API */
unsigned long br_timer_value(const struct timer_list *timer)
{
	return timer_pending(timer)
		? jiffies_to_clock_t(timer->expires - jiffies) : 0;
}
