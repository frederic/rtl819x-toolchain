/*
 *      Headler file of Realtek VLAN
 *
 *      $Id: rtk_vlan.h,v 1.3 2009/06/01 07:00:27 davidhsu Exp $
 */

#ifndef _RTK_VLAN_H
#define _RTK_VLAN_H
#include "rtl_types.h"

struct vlan_info {
	int global_vlan;	// 0/1 - global vlan disable/enable
	int is_lan;				// 1: eth-lan/wlan port, 0: wan port
	int vlan;					// 0/1: disable/enable vlan
	int tag;					// 0/1: disable/enable tagging
	int id;						// 1~4090: vlan id
	int pri;						// 0~7: priority;
	int cfi;						// 0/1: cfi
#if defined(CONFIG_RTK_VLAN_NEW_FEATURE)
	int forwarding_rule;				// 0:disabled/1: bridge/2:NAT
	int index;
#endif
};

struct _vlan_tag {
	unsigned short tpid;	// protocol id
	unsigned short pci;	// priority:3, cfi:1, id:12
};

struct vlan_tag {
	union
	{
		unsigned long v;
		struct _vlan_tag f;
	};
};

#if defined(CONFIG_RTK_VLAN_NEW_FEATURE)
extern struct vlan_info management_vlan;
extern unsigned char lan_macaddr[6];
struct vlan_info_item {
	struct vlan_info info;
	struct net_device *dev;
};
struct net_device *rtl_get_wan_from_vlan_info(void);
int rtl_add_vlan_info(struct vlan_info *info, struct net_device *dev);
struct vlan_info_item *rtl_get_vlan_info_item_by_dev(struct net_device *dev);

#endif


#if defined(CONFIG_RTL_HW_STP)
uint32 rtl865x_getVlanPortMask(uint32 vid);
#endif

#endif // _RTK_VLAN_H
