/*
 *	Spanning tree protocol; generic parts
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
#include <linux/rculist.h>

#include "br_private.h"
#include "br_private_stp.h"

#if defined (CONFIG_RTL_STP)
#include <net/rtl/rtl_nic.h>
#include <net/rtl/rtk_stp.h>
#endif

/* since time values in bpdu are in jiffies and then scaled (1/256)
 * before sending, make sure that is at least one.
 */
#define MESSAGE_AGE_INCR	((HZ < 256) ? 1 : (HZ/256))

static const char *br_port_state_names[] = {
	[BR_STATE_DISABLED] = "disabled",
	[BR_STATE_LISTENING] = "listening",
	[BR_STATE_LEARNING] = "learning",
	[BR_STATE_FORWARDING] = "forwarding",
	[BR_STATE_BLOCKING] = "blocking",
};

void br_log_state(const struct net_bridge_port *p)
{
	pr_info("%s: port %d(%s) entering %s state\n",
		p->br->dev->name, p->port_no, p->dev->name,
		br_port_state_names[p->state]);

}

/* called under bridge lock */
struct net_bridge_port *br_get_port(struct net_bridge *br, u16 port_no)
{
	struct net_bridge_port *p;

	list_for_each_entry_rcu(p, &br->port_list, list) {
		if (p->port_no == port_no)
			return p;
	}

	return NULL;
}

/* called under bridge lock */
static int br_should_become_root_port(const struct net_bridge_port *p,
				      u16 root_port)
{
	struct net_bridge *br;
	struct net_bridge_port *rp;
	int t;

	br = p->br;
	if (p->state == BR_STATE_DISABLED ||
	    br_is_designated_port(p))
		return 0;

	if (memcmp(&br->bridge_id, &p->designated_root, 8) <= 0)
		return 0;

	if (!root_port)
		return 1;

	rp = br_get_port(br, root_port);

	t = memcmp(&p->designated_root, &rp->designated_root, 8);
	if (t < 0)
		return 1;
	else if (t > 0)
		return 0;

	if (p->designated_cost + p->path_cost <
	    rp->designated_cost + rp->path_cost)
		return 1;
	else if (p->designated_cost + p->path_cost >
		 rp->designated_cost + rp->path_cost)
		return 0;

	t = memcmp(&p->designated_bridge, &rp->designated_bridge, 8);
	if (t < 0)
		return 1;
	else if (t > 0)
		return 0;

	if (p->designated_port < rp->designated_port)
		return 1;
	else if (p->designated_port > rp->designated_port)
		return 0;

	if (p->port_id < rp->port_id)
		return 1;

	return 0;
}

/* called under bridge lock */
static void br_root_selection(struct net_bridge *br)
{
	struct net_bridge_port *p;
	u16 root_port = 0;

	list_for_each_entry(p, &br->port_list, list) {
		if (br_should_become_root_port(p, root_port))
			root_port = p->port_no;

	}

	br->root_port = root_port;

	if (!root_port) {
		br->designated_root = br->bridge_id;
		br->root_path_cost = 0;
	} else {
		p = br_get_port(br, root_port);
		br->designated_root = p->designated_root;
		br->root_path_cost = p->designated_cost + p->path_cost;
	}
}

/* called under bridge lock */
void br_become_root_bridge(struct net_bridge *br)
{
	br->max_age = br->bridge_max_age;
	br->hello_time = br->bridge_hello_time;
	br->forward_delay = br->bridge_forward_delay;
	br_topology_change_detection(br);
	del_timer(&br->tcn_timer);

	if (br->dev->flags & IFF_UP) {
		br_config_bpdu_generation(br);
		mod_timer(&br->hello_timer, jiffies + br->hello_time);
	}
}

/* called under bridge lock */
void br_transmit_config(struct net_bridge_port *p)
{
	struct br_config_bpdu bpdu;
	struct net_bridge *br;


	if (timer_pending(&p->hold_timer)) {
		p->config_pending = 1;
		return;
	}

	br = p->br;

	bpdu.topology_change = br->topology_change;
	bpdu.topology_change_ack = p->topology_change_ack;
	bpdu.root = br->designated_root;
	bpdu.root_path_cost = br->root_path_cost;
	bpdu.bridge_id = br->bridge_id;
	bpdu.port_id = p->port_id;
	if (br_is_root_bridge(br))
		bpdu.message_age = 0;
	else {
		struct net_bridge_port *root
			= br_get_port(br, br->root_port);
		bpdu.message_age = br->max_age
			- (root->message_age_timer.expires - jiffies)
			+ MESSAGE_AGE_INCR;
	}
	bpdu.max_age = br->max_age;
	bpdu.hello_time = br->hello_time;
	bpdu.forward_delay = br->forward_delay;

	if (bpdu.message_age < br->max_age) {
		br_send_config_bpdu(p, &bpdu);
		p->topology_change_ack = 0;
		p->config_pending = 0;
		mod_timer(&p->hold_timer,
			  round_jiffies(jiffies + BR_HOLD_TIME));
	}
}

/* called under bridge lock */
static inline void br_record_config_information(struct net_bridge_port *p,
						const struct br_config_bpdu *bpdu)
{
	p->designated_root = bpdu->root;
	p->designated_cost = bpdu->root_path_cost;
	p->designated_bridge = bpdu->bridge_id;
	p->designated_port = bpdu->port_id;

	mod_timer(&p->message_age_timer, jiffies
		  + (p->br->max_age - bpdu->message_age));
}

/* called under bridge lock */
static inline void br_record_config_timeout_values(struct net_bridge *br,
					    const struct br_config_bpdu *bpdu)
{
	br->max_age = bpdu->max_age;
	br->hello_time = bpdu->hello_time;
	br->forward_delay = bpdu->forward_delay;
	br->topology_change = bpdu->topology_change;
}

/* called under bridge lock */
void br_transmit_tcn(struct net_bridge *br)
{
	br_send_tcn_bpdu(br_get_port(br, br->root_port));
}

/* called under bridge lock */
static int br_should_become_designated_port(const struct net_bridge_port *p)
{
	struct net_bridge *br;
	int t;

	br = p->br;
	if (br_is_designated_port(p))
		return 1;

	if (memcmp(&p->designated_root, &br->designated_root, 8))
		return 1;

	if (br->root_path_cost < p->designated_cost)
		return 1;
	else if (br->root_path_cost > p->designated_cost)
		return 0;

	t = memcmp(&br->bridge_id, &p->designated_bridge, 8);
	if (t < 0)
		return 1;
	else if (t > 0)
		return 0;

	if (p->port_id < p->designated_port)
		return 1;

	return 0;
}

/* called under bridge lock */
static void br_designated_port_selection(struct net_bridge *br)
{
	struct net_bridge_port *p;

	list_for_each_entry(p, &br->port_list, list) {
		if (p->state != BR_STATE_DISABLED &&
		    br_should_become_designated_port(p))
			br_become_designated_port(p);

	}
}

/* called under bridge lock */
static int br_supersedes_port_info(struct net_bridge_port *p, struct br_config_bpdu *bpdu)
{
	int t;

	t = memcmp(&bpdu->root, &p->designated_root, 8);
	if (t < 0)
		return 1;
	else if (t > 0)
		return 0;

	if (bpdu->root_path_cost < p->designated_cost)
		return 1;
	else if (bpdu->root_path_cost > p->designated_cost)
		return 0;

	t = memcmp(&bpdu->bridge_id, &p->designated_bridge, 8);
	if (t < 0)
		return 1;
	else if (t > 0)
		return 0;

	if (memcmp(&bpdu->bridge_id, &p->br->bridge_id, 8))
		return 1;

	if (bpdu->port_id <= p->designated_port)
		return 1;

	return 0;
}

/* called under bridge lock */
static inline void br_topology_change_acknowledged(struct net_bridge *br)
{
	br->topology_change_detected = 0;
	del_timer(&br->tcn_timer);
}

/* called under bridge lock */
void br_topology_change_detection(struct net_bridge *br)
{
	int isroot = br_is_root_bridge(br);

	if (br->stp_enabled != BR_KERNEL_STP)
		return;

	pr_info("%s: topology change detected, %s\n", br->dev->name,
		isroot ? "propagating" : "sending tcn bpdu");

	if (isroot) {
		br->topology_change = 1;
		mod_timer(&br->topology_change_timer, jiffies
			  + br->bridge_forward_delay + br->bridge_max_age);
	} else if (!br->topology_change_detected) {
		br_transmit_tcn(br);
		mod_timer(&br->tcn_timer, jiffies + br->bridge_hello_time);
	}

	br->topology_change_detected = 1;
}

/* called under bridge lock */
void br_config_bpdu_generation(struct net_bridge *br)
{
	struct net_bridge_port *p;

	list_for_each_entry(p, &br->port_list, list) {
		if (p->state != BR_STATE_DISABLED &&
		    br_is_designated_port(p))
			{
#if defined (CONFIG_RTL_STP)
				// bpdu not tx at eth0, because bpdu tx at virtual device port0~3 mapping to physical port0~3
				if(memcmp((void *)(p->dev->name), "eth0", 4)==0)
					continue;
#endif
				br_transmit_config(p);
			}
	}
}

/* called under bridge lock */
static inline void br_reply(struct net_bridge_port *p)
{
	br_transmit_config(p);
}

/* called under bridge lock */
void br_configuration_update(struct net_bridge *br)
{
	br_root_selection(br);
	br_designated_port_selection(br);
}

/* called under bridge lock */
void br_become_designated_port(struct net_bridge_port *p)
{
	struct net_bridge *br;

	br = p->br;
	p->designated_root = br->designated_root;
	p->designated_cost = br->root_path_cost;
	p->designated_bridge = br->bridge_id;
	p->designated_port = p->port_id;
}


/* called under bridge lock */
static void br_make_blocking(struct net_bridge_port *p)
{
#if defined (CONFIG_RTK_MESH)
#if defined (STP_ADDCOST_ETH) || defined (STP_DISABLE_ETH) 
//Chris:  stp+mesh
	struct net_bridge_port *tp,*n;
#endif
#endif //CONFIG_RTK_MESH

#if defined (CONFIG_RTL_STP)
	int retval=0, Port;
	char name[IFNAMSIZ];
	struct net_device *pseudo_dev;
#endif
#if defined(CONFIG_RTL_HW_STP)
	int retval, i;
	uint32 vid, portMask;
#endif

	if (p->state != BR_STATE_DISABLED &&
	    p->state != BR_STATE_BLOCKING) {
		if (p->state == BR_STATE_FORWARDING ||
		    p->state == BR_STATE_LEARNING)
			br_topology_change_detection(p->br);
		
#if defined (CONFIG_RTK_MESH)
#if 0
#if defined (STP_DISABLE_ETH)
// Chris: stp+mesh. if the blocking port is mesh, we disable the eth0 to avoid isolated MP
// 			NOTE: MUST resume the interface after loop disapeared
		if (!strncmp(p->dev->name, "wlan0-msh0", 10)){
			
			list_for_each_entry_safe(tp, n, &p->br->port_list, list) 
			{
				if (!strncmp(tp->dev->name, "eth", 3) && (tp->state != BR_STATE_DISABLED))
				{
					tp->disable_by_mesh = 1;
					br_stp_disable_port(tp);
					printk(KERN_INFO "%s: port %i(%s) entering %s state\n",
	  				    tp->br->dev->name, tp->port_no, tp->dev->name, "disabled if mesh blocked");
				}
	         
			}
	
		}
		else
		{
			p->state = BR_STATE_BLOCKING;
			br_log_state(p);
			del_timer(&p->forward_delay_timer);
		}
#endif
#endif

#if defined (STP_ADDCOST_ETH)
// Chris:  stp+mesh. Add cost on eth to block it. 
//		 NOTE: should set to default cost when topology changed
		if (!strncmp(p->dev->name, "wlan0-msh0", 10)){
			list_for_each_entry_safe(tp, n, &p->br->port_list, list) 
			{
				if (tp!=p && (tp->state != BR_STATE_DISABLED)){
					br_stp_set_path_cost(tp, tp->path_cost+50);
					printk(KERN_INFO "%s: port %i(%s) set path_cost %d \n",
	  				    tp->br->dev->name, tp->port_no, tp->dev->name, tp->path_cost);
					p->br->is_cost_changed = 1;
				}
			}
		}else {
			p->state = BR_STATE_BLOCKING;
			br_log_state(p);
			del_timer(&p->forward_delay_timer);
		}
		
#endif
#endif

#if defined (CONFIG_RTL_STP)
	{
		strcpy(name, p->dev->name);
		Port=STP_PortDev_Mapping[name[strlen(name)-1]-'0'];

		#ifdef CONFIG_RTK_MESH	
		if ((Port != WLAN_PSEUDO_IF_INDEX) && (Port !=WLAN_MESH_PSEUDO_IF_INDEX) &&  (Port!=NO_MAPPING))
		#else
		if ((Port != WLAN_PSEUDO_IF_INDEX) && (Port!=NO_MAPPING))
		#endif	
		{
			retval = rtl865x_setMulticastSpanningTreePortState(Port , RTL8651_PORTSTA_BLOCKING);

			retval = rtl865x_setSpanningTreePortState(Port, RTL8651_PORTSTA_BLOCKING);
		}
		else if (Port == WLAN_PSEUDO_IF_INDEX)
		{
			if ((pseudo_dev = __dev_get_by_name(&init_net,WLAN_IF_NAME)) == NULL)
			{	
				return;
			}
			pseudo_dev->br_port->state = BR_STATE_BLOCKING;
			retval = SUCCESS;
		}
		#ifdef CONFIG_RTK_MESH	
		else if (Port == WLAN_MESH_PSEUDO_IF_INDEX)
		{
			if ((pseudo_dev = __dev_get_by_name(&init_net,WLAN_MESH_IF_NAME)) == NULL)
			{	
				return;
			}
			pseudo_dev->br_port->state = BR_STATE_BLOCKING;
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
				retval = rtl865x_setMulticastSpanningTreePortState(i , RTL8651_PORTSTA_BLOCKING);
				if(retval==FAILED)
					printk("%s(%d): rtl865x_setMulticastSpanningTreePortState port(%d) failed.\n",__FUNCTION__,__LINE__,i);
				
				retval = rtl865x_setSpanningTreePortState(i, RTL8651_PORTSTA_BLOCKING);
				if(retval==FAILED)
					printk("%s(%d): rtl865x_setSpanningTreePortState port(%d) failed.\n",__FUNCTION__,__LINE__,i);
			}
		}
	}
#endif

#if defined (CONFIG_RTK_MESH)		
#else
		p->state = BR_STATE_BLOCKING;
		br_log_state(p);
		del_timer(&p->forward_delay_timer);
#endif		
	}
}

/* called under bridge lock */
static void br_make_forwarding(struct net_bridge_port *p)
{
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

	if (p->state != BR_STATE_BLOCKING)
		return;

	if (br->forward_delay == 0) {
		p->state = BR_STATE_FORWARDING;
		br_topology_change_detection(br);
		del_timer(&p->forward_delay_timer);
	}
	else if (p->br->stp_enabled == BR_KERNEL_STP)
	{
		p->state = BR_STATE_LISTENING;
#if defined (CONFIG_RTL_STP)
	{
		strcpy(name, p->dev->name);
		Port=STP_PortDev_Mapping[name[strlen(name)-1]-'0'];
		#ifdef CONFIG_RTK_MESH
		if ((Port != WLAN_PSEUDO_IF_INDEX) && (Port != WLAN_MESH_PSEUDO_IF_INDEX) && (Port!=NO_MAPPING))
		#else
		if ((Port != WLAN_PSEUDO_IF_INDEX) && (Port!=NO_MAPPING))
		#endif	
		{
			retval = rtl865x_setMulticastSpanningTreePortState(Port , RTL8651_PORTSTA_LISTENING);

			retval = rtl865x_setSpanningTreePortState(Port, RTL8651_PORTSTA_LISTENING);
		}
		else  if (Port == WLAN_PSEUDO_IF_INDEX)
		{
			if ((pseudo_dev = __dev_get_by_name(&init_net,WLAN_IF_NAME)) == NULL)
			{	
				return;
			}
			pseudo_dev->br_port->state = BR_STATE_LISTENING;
			retval = SUCCESS;
		}
		#ifdef CONFIG_RTK_MESH
		else  if (Port == WLAN_MESH_PSEUDO_IF_INDEX)
		{
			if ((pseudo_dev = __dev_get_by_name(&init_net,WLAN_MESH_IF_NAME)) == NULL)
			{	
				return;
			}
			pseudo_dev->br_port->state = BR_STATE_LISTENING;
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
		ps_drv_netif_mapping *entry;
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
				retval = rtl865x_setMulticastSpanningTreePortState(i , RTL8651_PORTSTA_LISTENING);
				if(retval==FAILED)
					printk("%s(%d): rtl865x_setMulticastSpanningTreePortState port(%d) failed.\n",__FUNCTION__,__LINE__,i);
				
				retval = rtl865x_setSpanningTreePortState(i, RTL8651_PORTSTA_LISTENING);
				if(retval==FAILED)
					printk("%s(%d): rtl865x_setSpanningTreePortState port(%d) failed.\n",__FUNCTION__,__LINE__,i);
			}
		}
	}
#endif

	}
	else
	{
		p->state = BR_STATE_LEARNING;
#if defined (CONFIG_RTL_STP)
	{
		strcpy(name, p->dev->name);
		Port=STP_PortDev_Mapping[name[strlen(name)-1]-'0'];	
		#ifdef CONFIG_RTK_MESH
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
		#ifdef CONFIG_RTK_MESH
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
		ps_drv_netif_mapping *entry;
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

	}

	br_log_state(p);

	if (br->forward_delay != 0)
		mod_timer(&p->forward_delay_timer, jiffies + br->forward_delay);
}

/* called under bridge lock */
void br_port_state_selection(struct net_bridge *br)
{
	struct net_bridge_port *p;

	/* Don't change port states if userspace is handling STP */
	if (br->stp_enabled == BR_USER_STP)
		return;

	list_for_each_entry(p, &br->port_list, list) {
		if (p->state != BR_STATE_DISABLED) {
			if (p->port_no == br->root_port) {
				p->config_pending = 0;
				p->topology_change_ack = 0;
				br_make_forwarding(p);
			} else if (br_is_designated_port(p)) {
				del_timer(&p->message_age_timer);
				br_make_forwarding(p);
			} else {
				p->config_pending = 0;
				p->topology_change_ack = 0;
				br_make_blocking(p);
			}
		}

	}
}

/* called under bridge lock */
static inline void br_topology_change_acknowledge(struct net_bridge_port *p)
{
	p->topology_change_ack = 1;
	br_transmit_config(p);
}

/* called under bridge lock */
void br_received_config_bpdu(struct net_bridge_port *p, struct br_config_bpdu *bpdu)
{
	struct net_bridge *br;
	int was_root;

	br = p->br;
	was_root = br_is_root_bridge(br);

	if (br_supersedes_port_info(p, bpdu)) {
		br_record_config_information(p, bpdu);
		br_configuration_update(br);
		br_port_state_selection(br);

		if (!br_is_root_bridge(br) && was_root) {
			del_timer(&br->hello_timer);
			if (br->topology_change_detected) {
				del_timer(&br->topology_change_timer);
				br_transmit_tcn(br);

				mod_timer(&br->tcn_timer,
					  jiffies + br->bridge_hello_time);
			}
		}

		if (p->port_no == br->root_port) {
			br_record_config_timeout_values(br, bpdu);
			br_config_bpdu_generation(br);
			if (bpdu->topology_change_ack)
				br_topology_change_acknowledged(br);
		}
	} else if (br_is_designated_port(p)) {
		br_reply(p);
	}
}

/* called under bridge lock */
void br_received_tcn_bpdu(struct net_bridge_port *p)
{
	if (br_is_designated_port(p)) {
		pr_info("%s: received tcn bpdu on port %i(%s)\n",
		       p->br->dev->name, p->port_no, p->dev->name);
#if defined(CONFIG_RTK_MESH) && defined(STP_ADDCOST_ETH)
		br_reset_cost(p);
#endif
		br_topology_change_detection(p->br);
		br_topology_change_acknowledge(p);
	}
}


#if defined(CONFIG_RTK_MESH) && defined(STP_ADDCOST_ETH)

/* called under bridge lock */
void br_reset_cost(struct net_bridge_port *p){
	struct net_bridge_port *tp,*n;
	
	if (p->br->is_cost_changed) {
		list_for_each_entry_safe(tp, n, &p->br->port_list, list) 
		{
			if (strncmp(tp->dev->name, "wlan0-msh0", 10)){
				tp->path_cost = br_initial_port_cost(tp->dev);
				printk(KERN_INFO "%s: port %i(%s) set default path_cost %d \n",
					    tp->br->dev->name, tp->port_no, tp->dev->name, tp->path_cost);
			}
		}
		p->br->is_cost_changed = 0;
	}
}
#endif
