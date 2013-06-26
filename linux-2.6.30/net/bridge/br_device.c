/*
 *	Device handling code
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
#include <linux/etherdevice.h>
#include <linux/ethtool.h>

#include <asm/uaccess.h>
#include "br_private.h"

#if defined (CONFIG_RTL_IGMP_SNOOPING)
/*2008-01-15,for porting igmp snooping to linux kernel 2.6*/
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/igmp.h>
#include <net/checksum.h>
#include <net/rtl/rtl865x_igmpsnooping_glue.h>
#include <net/rtl/rtl865x_igmpsnooping.h>
extern int igmpsnoopenabled;
#if defined (CONFIG_RTL_MLD_SNOOPING)
#include <linux/ipv6.h>
#include <linux/in6.h>
extern int mldSnoopEnabled;
#endif
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
#include <net/rtl/rtl865x_multicast.h>
#include <net/rtl/rtl865x_netif.h>
#include <net/rtl/rtl_nic.h>
#endif
extern unsigned int br0SwFwdPortMask;
extern unsigned int brIgmpModuleIndex;
extern unsigned int nicIgmpModuleIndex;
#endif

#if defined(CONFIG_RTL_HW_VLAN_SUPPORT)
extern uint32 rtl_hw_vlan_get_tagged_portmask(void);
#endif

#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
extern unsigned int br1SwFwdPortMask;
extern unsigned int nicIgmpModuleIndex_2;
#endif
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
int rtl865x_ipMulticastHardwareAccelerate(struct net_bridge *br, unsigned int brFwdPortMask,
												unsigned int srcPort,unsigned int srcVlanId, 
												unsigned int srcIpAddr, unsigned int destIpAddr);
#endif
#if defined (CONFIG_RTL_MLD_SNOOPING)
extern int re865x_getIpv6TransportProtocol(struct ipv6hdr* ipv6h);
#endif

#ifdef CONFIG_RTK_INBAND_HOST_HACK
#define ETHER_HDR_LEN 14
#define ARP_HRD_LEN 8
extern unsigned char inband_Hostmac[];
extern int br_hackMac_enable;
extern void check_listen_info(struct sk_buff *skb);
#endif

/* net device transmit always called with no BH (preempt_disabled) */
int br_dev_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct net_bridge *br = netdev_priv(dev);
	const unsigned char *dest = skb->data;
	struct net_bridge_fdb_entry *dst;

#if defined (CONFIG_RTL_IGMP_SNOOPING)	
	struct iphdr *iph=NULL;
	unsigned char proto=0;
	unsigned char reserved=0;
#if defined (CONFIG_RTL_MLD_SNOOPING) 	
	struct ipv6hdr *ipv6h=NULL;
#endif
	struct rtl_multicastDataInfo multicastDataInfo;
	struct rtl_multicastFwdInfo multicastFwdInfo;
	int ret=FAILED;
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	unsigned int srcPort=skb->srcPort;
	unsigned int srcVlanId=skb->srcVlanId;
#endif
#endif	

#ifdef CONFIG_RTK_INBAND_HOST_HACK
// send all paket that from local with hostmac (after bridge mac learning)	
	//hex_dump(skb->data, 48);
	if(br_hackMac_enable){			
		if(memcmp(skb->data,inband_Hostmac,6)) //if destmac is not to host
			memcpy(skb->data+6, inband_Hostmac,6); //then modify source mac to hostmac
		else
			goto ap_hcm_out;
		// if it is arp then sender mac also need modify to hostmac
		if( (skb->data[12] == 0x08) && (skb->data[13] == 0x06) ) //0806 = ARP
		{
			memcpy(skb->data+ETHER_HDR_LEN+ARP_HRD_LEN, inband_Hostmac,6); //modify arp sender mac			
		}
		else if((skb->data[12] == 0x08) && (skb->data[13] == 0x00)) //IP
		{
			check_listen_info(skb);
		}
	}
ap_hcm_out:		
#endif

	dev->stats.tx_packets++;
	dev->stats.tx_bytes += skb->len;

	skb_reset_mac_header(skb);
	skb_pull(skb, ETH_HLEN);


	if (dest[0] & 1)
	{
#if defined (CONFIG_RTL_IGMP_SNOOPING)	
		if(igmpsnoopenabled) 
		{	
			if(MULTICAST_MAC(dest))
			{
			
				iph=(struct iphdr *)skb_network_header(skb);
				proto =  iph->protocol;
				#if 0
				if(( iph->daddr&0xFFFFFF00)==0xE0000000)
				{
				        reserved=1;
				}
				#endif

				#if defined(CONFIG_USB_UWIFI_HOST)
				if(iph->daddr == 0xEFFFFFFA || iph->daddr == 0xE1010101)
				#else
				if(iph->daddr == 0xEFFFFFFA)
				#endif
				{
					/*for microsoft upnp*/
					reserved=1;
				}
				
				if(((proto ==IPPROTO_UDP) ||(proto ==IPPROTO_TCP))  && (reserved ==0))
				{
					multicastDataInfo.ipVersion=4;
					multicastDataInfo.sourceIp[0]=  (uint32)(iph->saddr);
					multicastDataInfo.groupAddr[0]=  (uint32)(iph->daddr);
					ret= rtl_getMulticastDataFwdInfo(brIgmpModuleIndex, &multicastDataInfo, &multicastFwdInfo);
			
					br_multicast_deliver(br, multicastFwdInfo.fwdPortMask, skb, 0);
					if((ret==SUCCESS) && (multicastFwdInfo.cpuFlag==0))
					{
						#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
						if((srcVlanId!=0) && (srcPort!=0xFFFF))
						{
							#if defined(CONFIG_RTK_VLAN_SUPPORT)
							if(rtk_vlan_support_enable == 0)
							{
								rtl865x_ipMulticastHardwareAccelerate(br, multicastFwdInfo.fwdPortMask,srcPort,srcVlanId, multicastDataInfo.sourceIp[0], multicastDataInfo.groupAddr[0]);
							}
							#else
							rtl865x_ipMulticastHardwareAccelerate(br, multicastFwdInfo.fwdPortMask,srcPort,srcVlanId, multicastDataInfo.sourceIp[0], multicastDataInfo.groupAddr[0]);
							#endif
						}
						#endif		
					}
				
				}
				else
				{
					br_flood_deliver(br, skb);
				}

				
			}
#if defined(CONFIG_RTL_MLD_SNOOPING)	
			else if(mldSnoopEnabled && IPV6_MULTICAST_MAC(dest))
			{
				ipv6h=(struct ipv6hdr *)skb_network_header(skb);
				proto=re865x_getIpv6TransportProtocol(ipv6h);
				if ((proto ==IPPROTO_UDP) ||(proto ==IPPROTO_TCP))
				{
					multicastDataInfo.ipVersion=6;
					memcpy(&multicastDataInfo.sourceIp, &ipv6h->saddr, sizeof(struct in6_addr));
					memcpy(&multicastDataInfo.groupAddr, &ipv6h->daddr, sizeof(struct in6_addr));
					ret= rtl_getMulticastDataFwdInfo(brIgmpModuleIndex, &multicastDataInfo, &multicastFwdInfo);
					br_multicast_deliver(br, multicastFwdInfo.fwdPortMask, skb, 0);
				}
				else
				{
					br_flood_deliver(br, skb);
				}
			}
#endif		
			else
			{
				br_flood_deliver(br, skb);
			}
		
		}
		else
		{ 
			br_flood_deliver(br, skb);
		}	
#else
		br_flood_deliver(br, skb);
#endif
	}
	else if ((dst = __br_fdb_get(br, dest)) != NULL)
		br_deliver(dst->dst, skb);
	else
		br_flood_deliver(br, skb);

	return 0;
}

static int br_dev_open(struct net_device *dev)
{
	struct net_bridge *br = netdev_priv(dev);

	br_features_recompute(br);
	netif_start_queue(dev);
	br_stp_enable_bridge(br);

	return 0;
}

static void br_dev_set_multicast_list(struct net_device *dev)
{
}

static int br_dev_stop(struct net_device *dev)
{
	br_stp_disable_bridge(netdev_priv(dev));

	netif_stop_queue(dev);

	return 0;
}

static int br_change_mtu(struct net_device *dev, int new_mtu)
{
	struct net_bridge *br = netdev_priv(dev);
	if (new_mtu < 68 || new_mtu > br_min_mtu(br))
		return -EINVAL;

	dev->mtu = new_mtu;

#ifdef CONFIG_BRIDGE_NETFILTER
	/* remember the MTU in the rtable for PMTU */
	br->fake_rtable.u.dst.metrics[RTAX_MTU - 1] = new_mtu;
#endif

	return 0;
}

/* Allow setting mac address to any valid ethernet address. */
static int br_set_mac_address(struct net_device *dev, void *p)
{
	struct net_bridge *br = netdev_priv(dev);
	struct sockaddr *addr = p;

	if (!is_valid_ether_addr(addr->sa_data))
		return -EINVAL;

	spin_lock_bh(&br->lock);
	memcpy(dev->dev_addr, addr->sa_data, ETH_ALEN);
	br_stp_change_bridge_id(br, addr->sa_data);
	br->flags |= BR_SET_MAC_ADDR;
	spin_unlock_bh(&br->lock);

	return 0;
}

static void br_getinfo(struct net_device *dev, struct ethtool_drvinfo *info)
{
	strcpy(info->driver, "bridge");
	strcpy(info->version, BR_VERSION);
	strcpy(info->fw_version, "N/A");
	strcpy(info->bus_info, "N/A");
}

static int br_set_sg(struct net_device *dev, u32 data)
{
	struct net_bridge *br = netdev_priv(dev);

	if (data)
		br->feature_mask |= NETIF_F_SG;
	else
		br->feature_mask &= ~NETIF_F_SG;

	br_features_recompute(br);
	return 0;
}

static int br_set_tso(struct net_device *dev, u32 data)
{
	struct net_bridge *br = netdev_priv(dev);

	if (data)
		br->feature_mask |= NETIF_F_TSO;
	else
		br->feature_mask &= ~NETIF_F_TSO;

	br_features_recompute(br);
	return 0;
}

static int br_set_tx_csum(struct net_device *dev, u32 data)
{
	struct net_bridge *br = netdev_priv(dev);

	if (data)
		br->feature_mask |= NETIF_F_NO_CSUM;
	else
		br->feature_mask &= ~NETIF_F_ALL_CSUM;

	br_features_recompute(br);
	return 0;
}

static const struct ethtool_ops br_ethtool_ops = {
	.get_drvinfo    = br_getinfo,
	.get_link	= ethtool_op_get_link,
	.get_tx_csum	= ethtool_op_get_tx_csum,
	.set_tx_csum 	= br_set_tx_csum,
	.get_sg		= ethtool_op_get_sg,
	.set_sg		= br_set_sg,
	.get_tso	= ethtool_op_get_tso,
	.set_tso	= br_set_tso,
	.get_ufo	= ethtool_op_get_ufo,
	.get_flags	= ethtool_op_get_flags,
};

static const struct net_device_ops br_netdev_ops = {
	.ndo_open		 = br_dev_open,
	.ndo_stop		 = br_dev_stop,
	.ndo_start_xmit		 = br_dev_xmit,
	.ndo_set_mac_address	 = br_set_mac_address,
	.ndo_set_multicast_list	 = br_dev_set_multicast_list,
	.ndo_change_mtu		 = br_change_mtu,
	.ndo_do_ioctl		 = br_dev_ioctl,
};

void br_dev_setup(struct net_device *dev)
{
	random_ether_addr(dev->dev_addr);
	ether_setup(dev);

	dev->netdev_ops = &br_netdev_ops;
	dev->destructor = free_netdev;
	SET_ETHTOOL_OPS(dev, &br_ethtool_ops);
	dev->tx_queue_len = 0;
	dev->priv_flags = IFF_EBRIDGE;
#if defined(CONFIG_HTTP_FILE_SERVER_SUPPORT) || defined(CONFIG_RTL_USB_UWIFI_HOST_SPEEDUP)
	dev->features = NETIF_F_SG | NETIF_F_FRAGLIST | NETIF_F_HIGHDMA |
			NETIF_F_GSO_MASK | NETIF_F_NO_CSUM | NETIF_F_LLTX |
			NETIF_F_NETNS_LOCAL | NETIF_F_GSO|NETIF_F_GRO|NETIF_F_LRO;

#elif defined(CONFIG_RTL_USB_IP_HOST_SPEEDUP)
        dev->features = NETIF_F_SG | NETIF_F_FRAGLIST | NETIF_F_HIGHDMA |
                        NETIF_F_GSO_MASK | NETIF_F_NO_CSUM | NETIF_F_LLTX |
                        NETIF_F_NETNS_LOCAL | NETIF_F_GSO|NETIF_F_GRO;
#else
	dev->features = NETIF_F_SG | NETIF_F_FRAGLIST | NETIF_F_HIGHDMA |
			NETIF_F_GSO_MASK | NETIF_F_NO_CSUM | NETIF_F_LLTX |
			NETIF_F_NETNS_LOCAL | NETIF_F_GSO;
#endif
}

#if defined (CONFIG_RTL_HARDWARE_MULTICAST)

int rtl865x_ipMulticastHardwareAccelerate(struct net_bridge *br, unsigned int brFwdPortMask,
												unsigned int srcPort,unsigned int srcVlanId, 
												unsigned int srcIpAddr, unsigned int destIpAddr)
{
	int ret;
	//int fwdDescCnt;
	//unsigned short port_bitmask=0;

	unsigned int tagged_portmask=0;


	struct rtl_multicastDataInfo multicastDataInfo;
	struct rtl_multicastFwdInfo  multicastFwdInfo;
	
	rtl865x_tblDrv_mCast_t * existMulticastEntry;
	rtl865x_mcast_fwd_descriptor_t  fwdDescriptor;

	#if 0
	printk("%s:%d,srcPort is %d,srcVlanId is %d,srcIpAddr is 0x%x,destIpAddr is 0x%x\n",__FUNCTION__,__LINE__,srcPort,srcVlanId,srcIpAddr,destIpAddr);
	#endif


#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	
	if(strcmp(br->dev->name,RTL_PS_BR0_DEV_NAME)!=0 &&strcmp(br->dev->name,RTL_PS_BR1_DEV_NAME)!=0 )
	{
		return -1;
	}
	if(strcmp(br->dev->name,RTL_PS_BR0_DEV_NAME)==0 && (brFwdPortMask & br0SwFwdPortMask))
	{
		return -1;
	}	
	
	if(strcmp(br->dev->name,RTL_PS_BR1_DEV_NAME)==0 && (brFwdPortMask & br1SwFwdPortMask))
	{
		return -1;
	}
#else
	if(strcmp(br->dev->name,RTL_PS_BR0_DEV_NAME)!=0)
	{
		return -1;
	}

	if(brFwdPortMask & br0SwFwdPortMask)
	{
		return -1;
	}
#endif
	//printk("%s:%d,destIpAddr is 0x%x, srcIpAddr is 0x%x, srcVlanId is %d, srcPort is %d\n",__FUNCTION__,__LINE__,destIpAddr, srcIpAddr, srcVlanId, srcPort);
	existMulticastEntry=rtl865x_findMCastEntry(destIpAddr, srcIpAddr, (unsigned short)srcVlanId, (unsigned short)srcPort);
	if(existMulticastEntry!=NULL)
	{
		/*it's already in cache */
		return 0;

	}

	if(brFwdPortMask==0)
	{
		rtl865x_blockMulticastFlow(srcVlanId, srcPort, srcIpAddr, destIpAddr);
		return 0;
	}
	
	multicastDataInfo.ipVersion=4;
	multicastDataInfo.sourceIp[0]=  srcIpAddr;
	multicastDataInfo.groupAddr[0]=  destIpAddr;

	/*add hardware multicast entry*/

	#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	if(strcmp(br->dev->name,RTL_PS_BR0_DEV_NAME)==0)
	{
		memset(&fwdDescriptor, 0, sizeof(rtl865x_mcast_fwd_descriptor_t ));
		strcpy(fwdDescriptor.netifName,"eth*");
		fwdDescriptor.fwdPortMask=0xFFFFFFFF;
		ret= rtl_getMulticastDataFwdInfo(nicIgmpModuleIndex, &multicastDataInfo, &multicastFwdInfo);
	}
	else if(strcmp(br->dev->name,RTL_PS_BR1_DEV_NAME)==0)
	{
		memset(&fwdDescriptor, 0, sizeof(rtl865x_mcast_fwd_descriptor_t ));
		strcpy(fwdDescriptor.netifName,"eth2");
		fwdDescriptor.fwdPortMask=0xFFFFFFFF;
		ret= rtl_getMulticastDataFwdInfo(nicIgmpModuleIndex_2, &multicastDataInfo, &multicastFwdInfo);
	}
	#else
	memset(&fwdDescriptor, 0, sizeof(rtl865x_mcast_fwd_descriptor_t ));
	strcpy(fwdDescriptor.netifName,"eth*");
	fwdDescriptor.fwdPortMask=0xFFFFFFFF;
	
	ret= rtl_getMulticastDataFwdInfo(nicIgmpModuleIndex, &multicastDataInfo, &multicastFwdInfo);
	#endif
	if(ret!=0)
	{
		return -1;
	}
	else
	{
		if(multicastFwdInfo.cpuFlag)
		{
			fwdDescriptor.toCpu=1;
		}
		fwdDescriptor.fwdPortMask=multicastFwdInfo.fwdPortMask & (~(1<<srcPort));
	}

#if defined(CONFIG_RTL_HW_VLAN_SUPPORT)
	if(rtl_hw_vlan_ignore_tagged_mc == 1)
		tagged_portmask = rtl_hw_vlan_get_tagged_portmask();
#endif
	if((fwdDescriptor.fwdPortMask & tagged_portmask) == 0)
	{

		ret=rtl865x_addMulticastEntry(destIpAddr, srcIpAddr, (unsigned short)srcVlanId, (unsigned short)srcPort,
							&fwdDescriptor, 1, 0, 0, 0);
	}

	return 0;
}

#endif
#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
int rtl865x_same_root(struct net_device *dev1,struct net_device *dev2){

	struct net_bridge_port *p = rcu_dereference(dev1->br_port);
	struct net_bridge_port *p2 = rcu_dereference(dev2->br_port);
	return !strncmp(p->br->dev->name,p2->br->dev->name,3);
}
#endif
