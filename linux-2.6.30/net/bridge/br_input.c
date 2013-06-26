/*
 *	Handle incoming frames
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
#include <linux/netfilter_bridge.h>
#include "br_private.h"

#if defined (CONFIG_RTL_IGMP_SNOOPING)
#include <linux/ip.h>
#include <linux/in.h>
#if defined (CONFIG_RTL_MLD_SNOOPING)
#include <linux/ipv6.h>
#include <linux/in6.h>
#endif
#include <linux/igmp.h>
#include <net/checksum.h>
#include <net/rtl/rtl865x_igmpsnooping_glue.h>
#include <net/rtl/rtl865x_igmpsnooping.h>
#include <net/rtl/rtl865x_netif.h>
#include <net/rtl/rtl_nic.h>

#if defined (CONFIG_RTL_IGMP_SNOOPING) && defined (CONFIG_NETFILTER)
#include <linux/netfilter_ipv4/ip_tables.h>
#endif

extern int igmpsnoopenabled;
#if defined (CONFIG_RTL_MLD_SNOOPING)
extern int mldSnoopEnabled;
#endif
extern unsigned int brIgmpModuleIndex;
extern unsigned int br0SwFwdPortMask;

#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
extern unsigned int brIgmpModuleIndex_2;
extern unsigned int br1SwFwdPortMask;
#endif

#if defined (MCAST_TO_UNICAST)
extern int IGMPProxyOpened;

#if defined (IPV6_MCAST_TO_UNICAST)
#include <linux/ipv6.h>
#include <linux/in6.h>
#include <linux/icmpv6.h>
//#define	DBG_ICMPv6	//enable it to debug icmpv6 check
static char ICMPv6_check(struct sk_buff *skb , unsigned char *gmac);
#endif	//end of IPV6_MCAST_TO_UNICAST

#endif	//end of MCAST_TO_UNICAST

#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT) || defined(CONFIG_RTL_ULINKER)
#include <net/udp.h>
#endif

static char igmp_type_check(struct sk_buff *skb, unsigned char *gmac,unsigned int *gIndex,unsigned int *moreFlag);
static void br_update_igmp_snoop_fdb(unsigned char op, struct net_bridge *br, struct net_bridge_port *p, unsigned char *gmac
									,struct sk_buff *skb);
#endif	//end of CONFIG_RTL_IGMP_SNOOPING

#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT) || defined(CONFIG_RTL_ULINKER)
extern int br_filter_enter(struct sk_buff *skb);
extern unsigned char dut_br0_mac[];
extern unsigned char Filter_State;
extern int enable_filter;
#endif

/* Bridge group multicast address 802.1d (pg 51). */
const u8 br_group_address[ETH_ALEN] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x00 };

#ifdef CONFIG_RTK_INBAND_HOST_HACK
#include <net/arp.h>
#include <net/tcp.h>
extern unsigned char inband_Hostmac[]; //mark_test
extern unsigned int inband_HostIP;
extern int br_hackMac_enable;

#define RTK_WPS_LISTEN_PORT 52881
#define MAX_LISTEN_ENDPOINT 8

struct listen_info {
	unsigned int src_ip;
	unsigned short src_port;
	unsigned char state;
	unsigned char sync_retry;
};

enum {
  IDLE_STATE = 0,
  SYN_SENT_STATE,
  CONNECTED_STATE,
  FIN_WAIT_STATE,  
  RTK_MAX_STATES /* Leave at the end! */
};


static struct listen_info listen_endpoint[MAX_LISTEN_ENDPOINT];

void init_listen_endpoint()
{
	memset(&listen_endpoint[0],0,sizeof(struct listen_info)*MAX_LISTEN_ENDPOINT);
}

static int is_listening_endpoint(unsigned int ip,unsigned short port,struct tcphdr *tcph_ptr)
{
	int i,ret=0;

	for(i=0;i<MAX_LISTEN_ENDPOINT;i++)
	{
		if( (listen_endpoint[i].src_port== port ) && (listen_endpoint[i].src_ip == ip) && 
			(listen_endpoint[i].state != IDLE_STATE))			
			break;
	}

	if( i == MAX_LISTEN_ENDPOINT)
		return 0; //not listen..

	if(listen_endpoint[i].state == SYN_SENT_STATE)	
	{
		if( tcph_ptr->syn  &&  tcph_ptr->ack )  //rcv - syn ack
		{
			listen_endpoint[i].state = CONNECTED_STATE;
			ret=1;
		}
	} 	
	else if(listen_endpoint[i].state == CONNECTED_STATE)// can rcv all pkt	
	{
		ret=1;
		if(tcph_ptr->rst)
			listen_endpoint[i].state = IDLE_STATE;
	}
	else if(listen_endpoint[i].state == FIN_WAIT_STATE)	//rcv ack to finish
	{
		ret=1;		  
		if( (tcph_ptr->ack && (!(tcph_ptr->fin)))  || tcph_ptr->rst )  //rcv - pure  ack or rst , fin+ack will not enter idle state
		{
			listen_endpoint[i].state = IDLE_STATE;		
		}	
	}
	
	return ret;	
}

static void add_listen_info(unsigned int pkt_ip, unsigned short pkt_port)
{
	int i;
	int entry1=-1,entry2=-1,entry2_retry=0;
	//rule 1 , find idle state entry to replace
	//rule 2 , find syn_sent state entry to replace
	for(i=0;i<MAX_LISTEN_ENDPOINT;i++)
	{
		if( listen_endpoint[i].state == IDLE_STATE )			
		{
			entry1 = i;
			break;
		}
		else if( (listen_endpoint[i].state == SYN_SENT_STATE) && (listen_endpoint[i].sync_retry >= 2))
		{
			if(listen_endpoint[i].sync_retry > entry2_retry)
			{
				entry2 = i;
				entry2_retry = listen_endpoint[i].sync_retry;
			}			
		}
	}		

	if(entry1 < 0) //no idle entry
	{
		if(entry2 < 0)
		{
			printk("no ap_hcm listen_entry can be used!!!\n");
			return;
		}				
		else
			entry1 = entry2;
	}	

	listen_endpoint[entry1].src_ip = pkt_ip;
	listen_endpoint[entry1].src_port= pkt_port;
	listen_endpoint[entry1].state = SYN_SENT_STATE;
	listen_endpoint[entry1].sync_retry= 0;

}

void check_listen_info(struct sk_buff *skb)
{
	struct iphdr *iph_ptr;
	struct tcphdr *tcph_ptr;
	int i,in_listening=0;
	unsigned int pkt_ip;
	unsigned short pkt_port;

	//iph_ptr = skb->nh.iph;
	iph_ptr = ip_hdr(skb); //mark_26
	tcph_ptr=(void *) iph_ptr + iph_ptr->ihl*4;
	pkt_ip = iph_ptr->daddr;
	pkt_port = tcph_ptr->dest;

	if((iph_ptr->protocol != IPPROTO_TCP))
		return;

	if((tcph_ptr->source == RTK_WPS_LISTEN_PORT)) //ingnore wps
		return;
	
	//find if the pkt is already listening
	for(i=0;i<MAX_LISTEN_ENDPOINT;i++)
	{
		if( (listen_endpoint[i].src_port== pkt_port ) && (listen_endpoint[i].src_ip == pkt_ip) )			
		{
			in_listening=1;
			break;
		}	
	}

	if(in_listening == 0) //the entry not find , so pnly sync pkt need to create a new listen entry
	{
		if((tcph_ptr->syn) && (!(tcph_ptr->ack)))
			add_listen_info(pkt_ip,pkt_port);
	}
	else //already in listen
	{
		if( (tcph_ptr->syn) && (!(tcph_ptr->ack))) //only syn pkt
		{
			listen_endpoint[i].state = SYN_SENT_STATE;
			listen_endpoint[i].sync_retry++;
		}
		else if((tcph_ptr->fin))
		{
			listen_endpoint[i].state = FIN_WAIT_STATE;
		}
		else if((tcph_ptr->rst))
		{
			listen_endpoint[i].state = IDLE_STATE;
		}	
		
	}	

}

#endif

#if defined (CONFIG_RTK_MESH)

void br_signal_pathsel(struct net_bridge *br)
{
	struct task_struct *task;
	if(br==NULL)
	{
		return;
	}
	read_lock(&tasklist_lock);
	task = find_task_by_vpid(br->mesh_pathsel_pid);
	read_unlock(&tasklist_lock);
	if(task)
	{
    	    	//printk("Send signal from kernel\n");
		send_sig(SIGUSR2,task,0);
	}
	else {
	//printk("Path selection daemon pid: %d does not exist\n", br->mesh_pathsel_pid);
	}
}

/*
void br_signal_pathsel()
{
	struct task_struct *task;

	struct net_bridge *br;

	br = find_br_by_name("br0");
	
	read_lock(&tasklist_lock);
    task = find_task_by_pid(br->mesh_pathsel_pid);
    read_unlock(&tasklist_lock);
    if(task)
    {
    	//printk("Send signal from kernel\n");
        send_sig(SIGUSR2,task,0);
		//br->stp_enabled = 1; //now pathsel daemon can turn it on
	}
    else {
        //printk("Path selection daemon pid: %d does not exist\n", br->mesh_pathsel_pid);
    }
}
*/
#endif	//CONFIG_RTK_MESH

static void br_pass_frame_up(struct net_bridge *br, struct sk_buff *skb)
{
	struct net_device *indev, *brdev = br->dev;

#if 0//defined (CONFIG_RTL_IGMP_SNOOPING)
        unsigned char *dest;
        struct net_bridge_port *p;
        unsigned char macAddr[6];
        unsigned char operation;
        struct iphdr *iph;
	unsigned char proto=0;  
                          
	//iph = skb->nh.iph;
	iph=(struct iphdr *)skb_network_header(skb);
	proto =  iph->protocol;    
	dest =  eth_hdr(skb)->h_dest;
	p = skb->dev->br_port;

	if (igmpsnoopenabled && MULTICAST_MAC(dest) && (eth_hdr(skb)->h_proto == ETH_P_IP)){
                if (proto== IPPROTO_IGMP){
			uint32 fwdPortMask;
			//rtl_igmpMldProcess(brIgmpModuleIndex, skb->mac.raw, p->port_no, &fwdPortMask);
			rtl_igmpMldProcess(brIgmpModuleIndex, skb_mac_header(skb), p->port_no, &fwdPortMask);
			if ((operation=igmp_type_check(skb, macAddr)) > 0) {	
               	            br_update_igmp_snoop_fdb(operation, br, p, macAddr,skb);
                          }
		}
        }

	#if defined (MCAST_TO_UNICAST)	
	#if defined (IPV6_MCAST_TO_UNICAST)
	else if(igmpsnoopenabled 
		&& IPV6_MULTICAST_MAC(dest) 
		&& (eth_hdr(skb)->h_proto == ETH_P_IPV6) )
	{		
		operation = ICMPv6_check(skb , macAddr);
		if (operation > 0) {
				#ifdef	DBG_ICMPv6
				if( operation == 1)
					printk("ICMPv6 mac add (from frame_up)\n");
				else if(operation == 2)
					printk("ICMPv6 mac del (from frame_up)\n");	
				#endif
               	br_update_igmp_snoop_fdb(operation, br, p, macAddr,skb);
		}
	}
	#endif	//end of IPV6_MCAST_TO_UNICAST
	#endif
#endif
	brdev->stats.rx_packets++;
	brdev->stats.rx_bytes += skb->len;

	indev = skb->dev;
	skb->dev = brdev;

	NF_HOOK(PF_BRIDGE, NF_BR_LOCAL_IN, skb, indev, NULL,
		netif_receive_skb);
}

#if defined (CONFIG_RTL865X_LANPORT_RESTRICTION)
extern int32 rtl865x_BlkCheck(const unsigned char *addr);
#endif

#if defined(CONFIG_RTL_MLD_SNOOPING)
extern int re865x_getIpv6TransportProtocol(struct ipv6hdr* ipv6h);
#endif
#if defined (CONFIG_RTL_HARDWARE_MULTICAST) 
extern int rtl865x_ipMulticastHardwareAccelerate(struct net_bridge *br, unsigned int brFwdPortMask,
												unsigned int srcPort,unsigned int srcVlanId, 
												unsigned int srcIpAddr, unsigned int destIpAddr);
#endif

#ifdef CONFIG_RTL_EAP_RELAY
extern unsigned char inband_Hostmac[];
//unsigned char Wlan_mac[6]={0x00,0xE0,0x4C,0x01,0x96,0xC0};
#endif

#if defined(CONFIG_RTL_WLAN_BLOCK_RELAY)
#define RTL_WLAN_INT_PREFIX "wlan"
extern int rtl_wlan_block_relay_enable;
#endif

#if defined (CONFIG_RTL_QUERIER_SELECTION)
#define HOP_BY_HOP_OPTIONS_HEADER 0
#define ROUTING_HEADER 43
#define FRAGMENT_HEADER 44
#define DESTINATION_OPTION_HEADER 60
#define NO_NEXT_HEADER 59
#define ICMP_PROTOCOL 58
#define IPV4_ROUTER_ALTER_OPTION 0x94040000
#define IPV6_ROUTER_ALTER_OPTION 0x05020000
#define IPV6_HEADER_LENGTH 40
#define MLD_QUERY 130
#define MLDV1_REPORT 131
#define MLDV1_DONE 132
#define MLDV2_REPORT 143
#define S_FLAG_MASK 0x08

extern int br_updateQuerierInfo(unsigned int version, unsigned char *devName, unsigned int* querierIp);

int check_igmpQueryExist(struct iphdr * iph)
{

	if(iph==NULL)
	{
		return 0;
	}

	if(*(unsigned char *)((unsigned char*)iph+((iph->ihl)<<2))==0x11)
	{
		return 1;
	}
	
	return 0;
}



int check_mldQueryExist(struct ipv6hdr* ipv6h)
{

	unsigned char *ptr=NULL;
	unsigned char *startPtr=NULL;
	unsigned char *lastPtr=NULL;
	unsigned char nextHeader=0;
	unsigned short extensionHdrLen=0;

	unsigned char  optionDataLen=0;
	unsigned char  optionType=0;
	unsigned int ipv6RAO=0;

	if(ipv6h==NULL)
	{
		return 0;
	}

	if(ipv6h->version!=6)
	{
		return 0;
	}

	startPtr= (unsigned char *)ipv6h;
	lastPtr=startPtr+sizeof(struct ipv6hdr)+(ipv6h->payload_len);
	nextHeader= ipv6h ->nexthdr;
	ptr=startPtr+sizeof(struct ipv6hdr);

	while(ptr<lastPtr)
	{
		switch(nextHeader)
		{
			case HOP_BY_HOP_OPTIONS_HEADER:
				/*parse hop-by-hop option*/
				nextHeader=ptr[0];
				extensionHdrLen=((uint16)(ptr[1])+1)*8;
				ptr=ptr+2;

				while(ptr<(startPtr+extensionHdrLen+sizeof(struct ipv6hdr)))
				{
					optionType=ptr[0];
					/*pad1 option*/
					if(optionType==0)
					{
						ptr=ptr+1;
						continue;
					}

					/*padN option*/
					if(optionType==1)
					{
						optionDataLen=ptr[1];
						ptr=ptr+optionDataLen+2;
						continue;
					}

					/*router altert option*/
					if(ntohl(*(uint32 *)(ptr))==IPV6_ROUTER_ALTER_OPTION)
					{
						ipv6RAO=IPV6_ROUTER_ALTER_OPTION;
						ptr=ptr+4;
						continue;
					}

					/*other TLV option*/
					if((optionType!=0) && (optionType!=1))
					{
						optionDataLen=ptr[1];
						ptr=ptr+optionDataLen+2;
						continue;
					}


				}

				break;

			case ROUTING_HEADER:
				nextHeader=ptr[0];
				extensionHdrLen=((uint16)(ptr[1])+1)*8;
                            ptr=ptr+extensionHdrLen;
				break;

			case FRAGMENT_HEADER:
				nextHeader=ptr[0];
				ptr=ptr+8;
				break;

			case DESTINATION_OPTION_HEADER:
				nextHeader=ptr[0];
				extensionHdrLen=((uint16)(ptr[1])+1)*8;
				ptr=ptr+extensionHdrLen;
				break;

			case ICMP_PROTOCOL:
				nextHeader=NO_NEXT_HEADER;
				if(ptr[0]==MLD_QUERY)
				{
					return 1;

				}
				break;

			default:
				/*not ipv6 multicast protocol*/
				return 0;
		}

	}
	return 0;
}

#endif
/* note: already called with rcu_read_lock (preempt_disabled) */
int br_handle_frame_finish(struct sk_buff *skb)
{
	const unsigned char *dest = eth_hdr(skb)->h_dest;
	struct net_bridge_port *p = rcu_dereference(skb->dev->br_port);
	struct net_bridge *br;
	struct net_bridge_fdb_entry *dst;
	struct sk_buff *skb2;

	if (!p || p->state == BR_STATE_DISABLED)
		goto drop;

	/* insert into forwarding database after filtering to avoid spoofing */
	br = p->br;
	br_fdb_update(br, p, eth_hdr(skb)->h_source);

#if defined (CONFIG_RTL865X_LANPORT_RESTRICTION)
	if (rtl865x_BlkCheck(eth_hdr(skb)->h_source) == TRUE)
	{

		kfree_skb(skb);
		goto out;
	}
#endif
	
#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT)	
	if(enable_filter){
		struct iphdr *iph_check;
		iph_check = (struct iphdr *)skb_network_header(skb);	
		if(Filter_State==0){
			if(memcmp(dut_br0_mac, dest, 6)){
				if(iph_check->protocol==IPPROTO_ICMP){
					goto drop;
				}
			}
		}
#if 0		
		else if(Filter_State==1 && (dest[0] & 1 == 0)){
			struct udphdr *udph;
			udph=(void *)iph_check + iph_check->ihl*4;
			if(iph_check->protocol==IPPROTO_UDP && udph->dest ==68){ //if dhcp server packet if unicast
				if(br_filter_enter(skb))
					goto err;
			}
		}
#endif		
	}
#endif

#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT) || defined(CONFIG_RTL_ULINKER)
	if(enable_filter){			
		if(br_filter_enter(skb))
			goto drop;
	}	
#endif	


	if (p->state == BR_STATE_LEARNING)
		goto drop;

//mark_issue, all filter above will not work in host ip hack ,FIX ME
#ifdef CONFIG_RTK_INBAND_HOST_HACK
	//if it is arp response then we need to pass to both local and remote host	
	//and redirect some protocol wps,802.1x to local not to remote host	
	if(br_hackMac_enable == 0) //donot nothing 
		goto ap_hcm_out;
	else{		
		if(eth_hdr(skb)->h_proto == ETH_P_ARP)
		{
			struct arphdr *arp = arp_hdr(skb);
			if(arp->ar_op == __constant_htons(ARPOP_REPLY))
			{
				struct sk_buff *skb3;

				if(memcmp(dest,inband_Hostmac,6)) //goout if mac is not for host
					goto ap_hcm_out;

				skb3 = skb_clone(skb, GFP_ATOMIC);			
				if (skb3 != NULL) {
					struct arphdr *arp3 = arp_hdr(skb3);
					unsigned char *arp_ptr= (unsigned char *)(arp3+1);
					//memcpy(arp_ptr+10,br->dev.dev_addr,6);   // cheat ARP layer that it's a ARP reply for local					
					memcpy(arp_ptr+10,skb->dev->dev_addr,6);   // mark_issue, 	perm_addr in 2.6	
					//memcpy(eth_hdr(skb3)->h_dest,br->dev.dev_addr,6);  							
					memcpy(eth_hdr(skb3)->h_dest,skb->dev->dev_addr,6);  // mark_issue, 	perm_addr in 2.6							
					//passedup = 1;				
					//hex_dump(skb3->data,48);
					skb3->pkt_type=PACKET_HOST; 
					br_pass_frame_up(br, skb3);
					memcpy(dest,inband_Hostmac,6); //recovery packet's mac to host
				}
			}
		}
		else if(eth_hdr(skb)->h_proto == ETH_P_IP)	//check packet's destIP == br0IP == Hostip
		{
			struct iphdr *iph_ptr;				
			//struct udphdr *udph_ptr;
			iph_ptr = ip_hdr(skb); //mark_26		
		
			if(iph_ptr->daddr == inband_HostIP )
			{
				if(memcmp(dest,inband_Hostmac,6)) //goout if mac is not for host
					goto ap_hcm_out;

				if((iph_ptr->protocol==IPPROTO_TCP))
				{
					struct tcphdr *tcph_ptr;
					int trap_to_local =0;

					tcph_ptr=(void *) iph_ptr + iph_ptr->ihl*4;

					if( tcph_ptr->dest <= 1024) //dont listen  public tcp port, just bypass to Host ...
						goto ap_hcm_out;

					//listen for wps dst port 	
					if( (tcph_ptr->dest == RTK_WPS_LISTEN_PORT)  || 
					is_listening_endpoint(iph_ptr->saddr,tcph_ptr->source,tcph_ptr))
						trap_to_local = 1;

					if(trap_to_local)
					{
						//memcpy(dest,br->dev.dev_addr,6);  						
						memcpy(eth_hdr(skb)->h_dest,skb->dev->dev_addr,6);										
						//hex_dump(skb->data,48);
						skb->pkt_type=PACKET_HOST; 
						br_pass_frame_up(br, skb);
						goto out;										
					}	

				}
				//use icmp packet for test		
				#if 0
				else if(iph_ptr->protocol==IPPROTO_ICMP)
				{
					memcpy(dest,br->dev.dev_addr,6);  				
					br_pass_frame_up(br, skb);
						goto out;
				}			
				#endif
			}			
		}
	}		
ap_hcm_out:	
#endif
	/* The packet skb2 goes to the local host (NULL to skip). */
	skb2 = NULL;

	if (br->dev->flags & IFF_PROMISC)
		skb2 = skb;

	dst = NULL;

	if (is_multicast_ether_addr(dest)) {
		br->dev->stats.multicast++;
		skb2 = skb;
	} else if ((dst = __br_fdb_get(br, dest)) && dst->is_local) {
#ifdef CONFIG_RTL_EAP_RELAY
		if(eth_hdr(skb)->h_proto == 0x888E)
		{			
			memcpy(eth_hdr(skb)->h_dest,inband_Hostmac,6);
			//else if(!memcmp(eth_hdr(skb)->h_source,inband_Hostmac,6)) // if src_mac == host .
			//	memcpy(eth_hdr(skb)->h_source,Wlan_mac,6);
			dst = __br_fdb_get(br, dest);					
		}
		else
#endif	
		{		
		skb2 = skb;
		/* Do not forward the packet since it's local. */
		skb = NULL;
	}
	}

	if (skb2 == skb)
		skb2 = skb_clone(skb, GFP_ATOMIC);

	if (skb2)
		br_pass_frame_up(br, skb2);


	if (skb) {
#if defined(CONFIG_RTL_WLAN_BLOCK_RELAY)
		if(rtl_wlan_block_relay_enable && dst){
			if(!memcmp(skb->dev->name,RTL_WLAN_INT_PREFIX,sizeof(RTL_WLAN_INT_PREFIX)-1)){
				if(!memcmp(dst->dst->dev->name,RTL_WLAN_INT_PREFIX,sizeof(RTL_WLAN_INT_PREFIX)-1))
					goto drop;
			}
		}
#endif
#if defined (CONFIG_RTL_IGMP_SNOOPING)
	if (is_multicast_ether_addr(dest) && igmpsnoopenabled) {
		struct iphdr *iph=NULL;
#if defined (CONFIG_RTL_MLD_SNOOPING) 	
		struct ipv6hdr *ipv6h=NULL;
#endif
		uint32 fwdPortMask=0;
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		unsigned int srcPort=skb->srcPort;
		unsigned int srcVlanId=skb->srcVlanId;
#endif

		unsigned char proto=0;
		unsigned char reserved=0;
		int ret=FAILED;
		
		unsigned char macAddr[6];
		unsigned char operation;
		char tmpOp;
		unsigned int gIndex=0;
		unsigned int moreFlag=1;
		
		struct rtl_multicastDataInfo multicastDataInfo;
		struct rtl_multicastFwdInfo multicastFwdInfo;

		if ( !(br->dev->flags & IFF_PROMISC) 
		 &&MULTICAST_MAC(dest) 
		&& (eth_hdr(skb)->h_proto == ETH_P_IP))
		{

			iph=(struct iphdr *)skb_network_header(skb);
			#if defined(CONFIG_USB_UWIFI_HOST)
				if(iph->daddr == 0xEFFFFFFA || iph->daddr == 0xE1010101)
			#else
				if(iph->daddr == 0xEFFFFFFA)
			#endif
			
			{
				/*for microsoft upnp*/
				reserved=1;
			}
#if 0
			if((iph->daddr&0xFFFFFF00)==0xE0000000)
			reserved=1;
#endif
			proto =  iph->protocol;  
			if (proto == IPPROTO_IGMP) 
			{	
#if defined (CONFIG_RTL_IGMP_SNOOPING) && defined (CONFIG_NETFILTER)
				//filter igmp pkts by upper hook like iptables 
				if(IgmpRxFilter_Hook != NULL)
				{
					struct net_device	*origDev=skb->dev;
					if((skb->dev->br_port) && (skb->dev->br_port->br))
					{
						skb->dev=skb->dev->br_port->br->dev;
					}
					
					if(IgmpRxFilter_Hook(skb, NF_INET_PRE_ROUTING,  skb->dev, NULL,dev_net(skb->dev)->ipv4.iptable_filter) !=NF_ACCEPT)
					{
						skb->dev=origDev;
						DEBUG_PRINT(" filter a pkt:%d %s:% \n", k, skb->dev->name, &(dev_net(skb->dev)->ipv4.iptable_filter->name[0]));
						goto drop;
					}
					else
					{
						skb->dev=origDev;
					}
						
					
					
				}else
					DEBUG_PRINT("IgmpRxFilter_Hook is NULL\n");
#endif
				while(moreFlag)
				{
					tmpOp=igmp_type_check(skb, macAddr, &gIndex, &moreFlag);
					if(tmpOp>0)
					{
						//printk("%s:%d,macAddr is 0x%x:%x:%x:%x:%x:%x\n",__FUNCTION__,__LINE__,macAddr[0],macAddr[1],macAddr[2],macAddr[3],macAddr[4],macAddr[5]);
						operation=(unsigned char)tmpOp;
						br_update_igmp_snoop_fdb(operation, br, p, macAddr,skb);
					}
				}
				
				#if defined (CONFIG_RTL_QUERIER_SELECTION)
				if(check_igmpQueryExist(iph)==1)
				{
					/*igmp query packet*/
					if(skb->dev->br_port)
					{
						br_updateQuerierInfo(4,skb->dev->br_port->br->dev->name,&(iph->saddr));
					}
					else
					{
						br_updateQuerierInfo(4,skb->dev->name,&(iph->saddr));
					}
				}
				#endif
                #ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
				if(!strcmp(br->dev->name,RTL_PS_BR1_DEV_NAME))
				{
					rtl_igmpMldProcess(brIgmpModuleIndex_2, skb_mac_header(skb), p->port_no, &fwdPortMask);
					//flooding igmp packet
					fwdPortMask=(~(1<<(p->port_no))) & 0xFFFFFFFF;
				}
				else
				#endif
				rtl_igmpMldProcess(brIgmpModuleIndex, skb_mac_header(skb), p->port_no, &fwdPortMask);
				br_multicast_forward(br, fwdPortMask, skb, 0);
			}
			else if(((proto ==IPPROTO_UDP) ||(proto ==IPPROTO_TCP)) && (reserved ==0))
			{

				iph=(struct iphdr *)skb_network_header(skb);
				multicastDataInfo.ipVersion=4;
				multicastDataInfo.sourceIp[0]=  (uint32)(iph->saddr);
				multicastDataInfo.groupAddr[0]=  (uint32)(iph->daddr);
				
                #ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
				if(!strcmp(br->dev->name,RTL_PS_BR1_DEV_NAME))
				{
					ret= rtl_getMulticastDataFwdInfo(brIgmpModuleIndex_2, &multicastDataInfo, &multicastFwdInfo);
				}
				else
				#endif
				ret= rtl_getMulticastDataFwdInfo(brIgmpModuleIndex, &multicastDataInfo, &multicastFwdInfo);
				br_multicast_forward(br, multicastFwdInfo.fwdPortMask, skb, 0);
				if((ret==SUCCESS) && (multicastFwdInfo.cpuFlag==0))
				{

					#if defined  (CONFIG_RTL_HARDWARE_MULTICAST)
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
				br_flood_forward(br, skb);
			}
		}
		else if(!(br->dev->flags & IFF_PROMISC) 
			&& IPV6_MULTICAST_MAC(dest)
			&& (eth_hdr(skb)->h_proto == ETH_P_IPV6))
		{

#if defined (IPV6_MCAST_TO_UNICAST)
			tmpOp=ICMPv6_check(skb , macAddr);
			if(tmpOp > 0){
				operation=(unsigned char)tmpOp;
#ifdef	DBG_ICMPv6
			if( operation == 1)
				printk("icmpv6 add from frame finish\n");
			else if(operation == 2)
				printk("icmpv6 del from frame finish\n");	
#endif
				br_update_igmp_snoop_fdb(operation, br, p, macAddr,skb);
			}
#endif

#if defined (CONFIG_RTL_MLD_SNOOPING)
			if(mldSnoopEnabled)
			{
				ipv6h=(struct ipv6hdr *)skb_network_header(skb);
				proto =  re865x_getIpv6TransportProtocol(ipv6h);
				/*icmp protocol*/
				if (proto == IPPROTO_ICMPV6) 
				{	
					#if defined (CONFIG_RTL_QUERIER_SELECTION)
					if(check_mldQueryExist(ipv6h)==1)
					{
						if(skb->dev->br_port)
						{
							br_updateQuerierInfo(6,skb->dev->br_port->br->dev->name,&(ipv6h->saddr));
						}
						else
						{
							br_updateQuerierInfo(6,skb->dev->name,&(ipv6h->saddr));
						}

					}
					#endif
					rtl_igmpMldProcess(brIgmpModuleIndex, skb_mac_header(skb), p->port_no, &fwdPortMask);	
					br_multicast_forward(br, fwdPortMask, skb, 0);
				}
				else if ((proto ==IPPROTO_UDP) ||(proto ==IPPROTO_TCP))
				{
					multicastDataInfo.ipVersion=6;
					memcpy(&multicastDataInfo.sourceIp, &ipv6h->saddr, sizeof(struct in6_addr));
					memcpy(&multicastDataInfo.groupAddr, &ipv6h->daddr, sizeof(struct in6_addr));	
					ret= rtl_getMulticastDataFwdInfo(brIgmpModuleIndex, &multicastDataInfo, &multicastFwdInfo);
					br_multicast_forward(br, multicastFwdInfo.fwdPortMask, skb, 0);
				
				}
				else
				{
					br_flood_forward(br, skb);
				}	
			}
			else
#endif				
			{
				br_flood_forward(br, skb);
			}

		}
		else
		{
			br_flood_forward(br, skb);
		}

	
	}
	else
	{
		/*known/unknown unicast packet*/
		if (dst)
			br_forward(dst->dst, skb);
		else
			br_flood_forward(br, skb);
	}	
#else 

		if (dst)
			br_forward(dst->dst, skb);
		else
			br_flood_forward(br, skb);
#endif	// CONFIG_RTL_IGMP_SNOOPING
	}

out:
	return 0;
drop:
	kfree_skb(skb);
	goto out;
}

/* note: already called with rcu_read_lock (preempt_disabled) */
static int br_handle_local_finish(struct sk_buff *skb)
{
	struct net_bridge_port *p = rcu_dereference(skb->dev->br_port);

	if (p)
		br_fdb_update(p->br, p, eth_hdr(skb)->h_source);
	return 0;	 /* process further */
}

/* Does address match the link local multicast address.
 * 01:80:c2:00:00:0X
 */
static inline int is_link_local(const unsigned char *dest)
{
	__be16 *a = (__be16 *)dest;
	static const __be16 *b = (const __be16 *)br_group_address;
	static const __be16 m = cpu_to_be16(0xfff0);

	return ((a[0] ^ b[0]) | (a[1] ^ b[1]) | ((a[2] ^ b[2]) & m)) == 0;
}

/*
 * Called via br_handle_frame_hook.
 * Return NULL if skb is handled
 * note: already called with rcu_read_lock (preempt_disabled)
 */
struct sk_buff *br_handle_frame(struct net_bridge_port *p, struct sk_buff *skb)
{
	const unsigned char *dest = eth_hdr(skb)->h_dest;
	int (*rhook)(struct sk_buff *skb);

	if (!is_valid_ether_addr(eth_hdr(skb)->h_source))
		goto drop;

	skb = skb_share_check(skb, GFP_ATOMIC);
	if (!skb)
		return NULL;

	if (unlikely(is_link_local(dest))) {
		/* Pause frames shouldn't be passed up by driver anyway */
		if (skb->protocol == htons(ETH_P_PAUSE))
			goto drop;

		/* If STP is turned off, then forward */
		if (p->br->stp_enabled == BR_NO_STP && dest[5] == 0)
			goto forward;

#if !defined(CONFIG_RTL_ULINKER)
		if (NF_HOOK(PF_BRIDGE, NF_BR_LOCAL_IN, skb, skb->dev,
			    NULL, br_handle_local_finish))
			return NULL;	/* frame consumed by filter */
		else
			return skb;	/* continue processing */
#endif
	}
#if 0
#if defined (STP_DISABLE_ETH)
//chris: auto stp on
	if (!(p->br->dev->flags & IFF_UP))
		goto err;

	if (p->state==BR_STATE_DISABLED && p->disable_by_mesh ==0){
		goto err;
	}else if (!strncmp(p->dev->name,"eth", 3)){
			p->br->stp_enabled = 1;
			mod_timer(&p->br->eth0_autostp_timer, jiffies+p->br->eth0_monitor_interval);
	}
#endif	
#endif 

forward:
	switch (p->state) {
	case BR_STATE_FORWARDING:
		rhook = rcu_dereference(br_should_route_hook);
		if (rhook != NULL) {
			if (rhook(skb))
				return skb;
			dest = eth_hdr(skb)->h_dest;
		}
		/* fall through */
	case BR_STATE_LEARNING:
		if (!compare_ether_addr(p->br->dev->dev_addr, dest))
			skb->pkt_type = PACKET_HOST;

#if defined (CONFIG_RTK_MESH)
if (p->state == BR_STATE_FORWARDING) {
		//brian modify for trigger portal-enable event
		if(!strncmp(p->dev->name,RTL_PS_ETH_NAME, 3)){
			if(p->br->mesh_pathsel_pid!= 0){
				if( !(p->br->eth0_received) )
				{
					p->br->eth0_received = 1;
					p->br->stp_enabled = 1;
					br_signal_pathsel(p->br);
					printk(KERN_INFO,"eth0 learning, event pathsel daemon \n");
				}
				
				mod_timer(&p->br->eth0_monitor_timer, jiffies+p->br->eth0_monitor_interval);
				
			}
		}
}
#endif	//CONFIG_RTK_MESH

		NF_HOOK(PF_BRIDGE, NF_BR_PRE_ROUTING, skb, skb->dev, NULL,
			br_handle_frame_finish);
		break;
	default:
drop:
		kfree_skb(skb);
	}
	return NULL;
}

#if defined (CONFIG_RTL_IGMP_SNOOPING)

#if defined (IPV6_MCAST_TO_UNICAST)
/*Convert  MultiCatst IPV6_Addr to MAC_Addr*/
static void CIPV6toMac
	(unsigned char* icmpv6_McastAddr, unsigned char *gmac )
{
	/*ICMPv6 valid addr 2^32 -1*/
	gmac[0] = 0x33;
	gmac[1] = 0x33;
	gmac[2] = icmpv6_McastAddr[12];
	gmac[3] = icmpv6_McastAddr[13];
	gmac[4] = icmpv6_McastAddr[14];
	gmac[5] = icmpv6_McastAddr[15];			
}



static char ICMPv6_check(struct sk_buff *skb , unsigned char *gmac)
{
	
	struct ipv6hdr *ipv6h;
	char* protoType;	
	
	/* check IPv6 header information */
	//ipv6h = skb->nh.ipv6h;
	ipv6h = (struct ipv6hdr *)skb_network_header(skb);
	if(ipv6h->version != 6){	
		//printk("ipv6h->version != 6\n");
		return -1;
	}


	/*Next header: IPv6 hop-by-hop option (0x00)*/
	if(ipv6h->nexthdr == 0)	{
		protoType = (unsigned char*)( (unsigned char*)ipv6h + sizeof(struct ipv6hdr) );	
	}else{
		//printk("ipv6h->nexthdr != 0\n");
		return -1;
	}

	if(protoType[0] == 0x3a){
		
		//printk("recv icmpv6 packet\n");
		struct icmp6hdr* icmpv6h = (struct icmp6hdr*)(protoType + 8);
		unsigned char* icmpv6_McastAddr ;
	
		if(icmpv6h->icmp6_type == 0x83){
			
			icmpv6_McastAddr = (unsigned char*)((unsigned char*)icmpv6h + 8);
			#ifdef	DBG_ICMPv6					
			printk("Type: 0x%x (Multicast listener report) \n",icmpv6h->icmp6_type);
			#endif

		}else if(icmpv6h->icmp6_type == 0x8f){		
		
			icmpv6_McastAddr = (unsigned char*)((unsigned char*)icmpv6h + 8 + 4);
			#ifdef	DBG_ICMPv6					
			printk("Type: 0x%x (Multicast listener report v2) \n",icmpv6h->icmp6_type);
			#endif			
		}else if(icmpv6h->icmp6_type == 0x84){
		
			icmpv6_McastAddr = (unsigned char*)((unsigned char*)icmpv6h + 8 );			
			#ifdef	DBG_ICMPv6					
			printk("Type: 0x%x (Multicast listener done ) \n",icmpv6h->icmp6_type);
			#endif			
		}
		else{
			#ifdef	DBG_ICMPv6
			printk("Type: 0x%x (unknow type)\n",icmpv6h->icmp6_type);
			#endif			
			return -1;
		}				

		#ifdef	DBG_ICMPv6			
		printk("MCAST_IPV6Addr:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n",
			icmpv6_McastAddr[0],icmpv6_McastAddr[1],icmpv6_McastAddr[2],icmpv6_McastAddr[3],
			icmpv6_McastAddr[4],icmpv6_McastAddr[5],icmpv6_McastAddr[6],icmpv6_McastAddr[7],
			icmpv6_McastAddr[8],icmpv6_McastAddr[9],icmpv6_McastAddr[10],icmpv6_McastAddr[11],
			icmpv6_McastAddr[12],icmpv6_McastAddr[13],icmpv6_McastAddr[14],icmpv6_McastAddr[15]);
		#endif

		CIPV6toMac(icmpv6_McastAddr, gmac);
		
		#ifdef	DBG_ICMPv6					
		printk("group_mac [%02x:%02x:%02x:%02x:%02x:%02x] \n",
			gmac[0],gmac[1],gmac[2],
			gmac[3],gmac[4],gmac[5]);
		#endif
			


		if(icmpv6h->icmp6_type == 0x83){

			return 1;//icmpv6 listener report (add)
		}
		else if(icmpv6h->icmp6_type == 0x8f){
			return 1;//icmpv6 listener report v2 (add) 
		}
		else if(icmpv6h->icmp6_type == 0x84){
			return 2;//icmpv6 Multicast listener done (del)
		}
	}		
	else{
		//printk("protoType[0] != 0x3a\n");		
		return -1;//not icmpv6 type
	}
		
	return -1;
}

#endif	//end of IPV6_MCAST_TO_UNICAST

/*2008-01-15,for porting igmp snooping to linux kernel 2.6*/
static void ConvertMulticatIPtoMacAddr(__u32 group, unsigned char *gmac)
{
	__u32 u32tmp, tmp;
	int i;

	u32tmp = group & 0x007FFFFF;
	gmac[0]=0x01; gmac[1]=0x00; gmac[2]=0x5e;
	for (i=5; i>=3; i--) {
		tmp=u32tmp&0xFF;
		gmac[i]=tmp;
		u32tmp >>= 8;
	}
}
static char igmp_type_check(struct sk_buff *skb, unsigned char *gmac,unsigned int *gIndex,unsigned int *moreFlag)
{
        struct iphdr *iph;
	__u8 hdrlen;
	struct igmphdr *igmph;
	int i;
	unsigned int groupAddr=0;// add  for fit igmp v3
	*moreFlag=0;
	/* check IP header information */
	iph=(struct iphdr *)skb_network_header(skb);
	hdrlen = iph->ihl << 2;
	if ((iph->version != 4) &&  (hdrlen < 20))
		return -1;
	if (ip_fast_csum((u8 *)iph, iph->ihl) != 0)
		return -1;
	{ /* check the length */
		__u32 len = ntohs(iph->tot_len);
		if (skb->len < len || len < hdrlen)
			return -1; 
	}
	/* parsing the igmp packet */
	igmph = (struct igmphdr *)((u8*)iph+hdrlen);

	
	
	if ((igmph->type==IGMP_HOST_MEMBERSHIP_REPORT) ||
	    (igmph->type==IGMPV2_HOST_MEMBERSHIP_REPORT)) 
	{
		groupAddr = igmph->group;
		if(!IN_MULTICAST(groupAddr))
		{			
				return -1;
		}
		
		ConvertMulticatIPtoMacAddr(groupAddr, gmac);
		
		return 1; /* report and add it */
	}
	else if (igmph->type==IGMPV3_HOST_MEMBERSHIP_REPORT)	{ 
		
	
		/*for support igmp v3 ; plusWang add 2009-0311*/   	
		struct igmpv3_report *igmpv3report=(struct igmpv3_report * )igmph;
		struct igmpv3_grec	*igmpv3grec=NULL; 
		//printk("%s:%d,*gIndex is %d,igmpv3report->ngrec is %d\n",__FUNCTION__,__LINE__,*gIndex,igmpv3report->ngrec);
		if(*gIndex>=igmpv3report->ngrec)
		{
			*moreFlag=0;
			return -1;
		}
	
		for(i=0;i<igmpv3report->ngrec;i++)
		{

			if(i==0)
			{
				igmpv3grec = (struct igmpv3_grec *)(&(igmpv3report->grec)); /*first igmp group record*/
			}
			else
			{
				igmpv3grec=(struct igmpv3_grec *)((unsigned char*)igmpv3grec+8+igmpv3grec->grec_nsrcs*4+(igmpv3grec->grec_auxwords)*4);
				
				
			}
			
			if(i!=*gIndex)
			{	
				
				continue;
			}
			
			if(i==(igmpv3report->ngrec-1))
			{
				/*last group record*/
				*moreFlag=0;
			}
			else
			{
				*moreFlag=1;
			}
			
			/*gIndex move to next group*/
			*gIndex=*gIndex+1;	
			
			groupAddr=igmpv3grec->grec_mca;
			//printk("%s:%d,groupAddr is %d.%d.%d.%d\n",__FUNCTION__,__LINE__,NIPQUAD(groupAddr));
			if(!IN_MULTICAST(groupAddr))
			{			
				return -1;
			}
			
			ConvertMulticatIPtoMacAddr(groupAddr, gmac);
			if(((igmpv3grec->grec_type == IGMPV3_CHANGE_TO_INCLUDE) || (igmpv3grec->grec_type == IGMPV3_MODE_IS_INCLUDE))&& (igmpv3grec->grec_nsrcs==0))
			{	
				return 2; /* leave and delete it */	
			}
			else if((igmpv3grec->grec_type == IGMPV3_CHANGE_TO_EXCLUDE) ||
				(igmpv3grec->grec_type == IGMPV3_MODE_IS_EXCLUDE) ||
				(igmpv3grec->grec_type == IGMPV3_ALLOW_NEW_SOURCES))
			{
				return 1;
			}
			else
			{
				/*ignore it*/
			}
			
			return -1;
		}
		
		/*avoid dead loop in case of initial gIndex is too big*/
		if(i>=(igmpv3report->ngrec-1))
		{
			/*last group record*/
			*moreFlag=0;
			return -1;
		}
		
	
	}
	else if (igmph->type==IGMP_HOST_LEAVE_MESSAGE){

		groupAddr = igmph->group;
		if(!IN_MULTICAST(groupAddr))
		{			
				return -1;
		}
		
		ConvertMulticatIPtoMacAddr(groupAddr, gmac);
		return 2; /* leave and delete it */
	}	
	
	
	return -1;
}

extern int chk_igmp_ext_entry(struct net_bridge_fdb_entry *fdb ,unsigned char *srcMac);
extern void add_igmp_ext_entry(	struct net_bridge_fdb_entry *fdb , unsigned char *srcMac , unsigned char portComeIn);
extern void update_igmp_ext_entry(	struct net_bridge_fdb_entry *fdb ,unsigned char *srcMac , unsigned char portComeIn);
extern void del_igmp_ext_entry(	struct net_bridge_fdb_entry *fdb ,unsigned char *srcMac , unsigned char portComeIn );

static void br_update_igmp_snoop_fdb(unsigned char op, struct net_bridge *br, struct net_bridge_port *p, unsigned char *dest 
										,struct sk_buff *skb)
{
	struct net_bridge_fdb_entry *dst;
	unsigned char *src;
	unsigned short del_group_src=0;
	unsigned char port_comein;
	int tt1;

#if defined (MCAST_TO_UNICAST)
	struct net_device *dev; 
	if(!dest)	return;
	if( !MULTICAST_MAC(dest)
#if defined (IPV6_MCAST_TO_UNICAST)
		&& !IPV6_MULTICAST_MAC(dest)
#endif	
	   )
	   { 
	   	return; 
	   }
#endif

#if defined( CONFIG_RTL_HARDWARE_MULTICAST) || defined(CONFIG_RTL865X_LANPORT_RESTRICTION)

	if(skb->srcPort!=0xFFFF)
	{
		port_comein = 1<<skb->srcPort;
	}
	else
	{
		port_comein=0x80;
	}
	
#else
	if(p && p->dev && p->dev->name && !memcmp(p->dev->name, RTL_PS_LAN_P0_DEV_NAME, 4))
	{
		port_comein = 0x01;
	}
	
	if(p && p->dev && p->dev->name && !memcmp(p->dev->name, RTL_PS_WLAN_NAME, 4))
	{
		port_comein=0x80;
	}
	
#endif
//	src=(unsigned char*)(skb->mac.raw+ETH_ALEN);
	src=(unsigned char*)(skb_mac_header(skb)+ETH_ALEN);
	/* check whether entry exist */
	dst = __br_fdb_get(br, dest);

	if (op == 1) /* add */
	{	
	
#if defined (MCAST_TO_UNICAST)
		/*process wlan client join --- start*/
		if (dst && p && p->dev && p->dev->name && !memcmp(p->dev->name, RTL_PS_WLAN_NAME, 4)) 
		{ 
			dst->portlist |= 0x80;
			port_comein = 0x80;
			//dev = __dev_get_by_name(&init_net,RTL_PS_WLAN0_DEV_NAME);	
			dev=p->dev;
			if (dev) 
			{		
				unsigned char StaMacAndGroup[20];
				memcpy(StaMacAndGroup, dest, 6);
				memcpy(StaMacAndGroup+6, src, 6);	
			#if defined(CONFIG_COMPAT_NET_DEV_OPS)
				if (dev->do_ioctl != NULL) 
				{
					dev->do_ioctl(dev, (struct ifreq*)StaMacAndGroup, 0x8B80);
					DEBUG_PRINT("... add to wlan mcast table:  DA:%02x:%02x:%02x:%02x:%02x:%02x ; SA:%02x:%02x:%02x:%02x:%02x:%02x\n", 
						StaMacAndGroup[0],StaMacAndGroup[1],StaMacAndGroup[2],StaMacAndGroup[3],StaMacAndGroup[4],StaMacAndGroup[5],
						StaMacAndGroup[6],StaMacAndGroup[7],StaMacAndGroup[8],StaMacAndGroup[9],StaMacAndGroup[10],StaMacAndGroup[11]);					
				}
			#else
				if (dev->netdev_ops->ndo_do_ioctl != NULL) 
				{
					dev->netdev_ops->ndo_do_ioctl(dev, (struct ifreq*)StaMacAndGroup, 0x8B80);
					DEBUG_PRINT("... add to wlan mcast table:  DA:%02x:%02x:%02x:%02x:%02x:%02x ; SA:%02x:%02x:%02x:%02x:%02x:%02x\n", 
						StaMacAndGroup[0],StaMacAndGroup[1],StaMacAndGroup[2],StaMacAndGroup[3],StaMacAndGroup[4],StaMacAndGroup[5],
						StaMacAndGroup[6],StaMacAndGroup[7],StaMacAndGroup[8],StaMacAndGroup[9],StaMacAndGroup[10],StaMacAndGroup[11]);	
				}
			#endif
				
														
			}
		}
	/*process wlan client join --- end*/
#endif

			if (dst) 
			{
		        	dst->group_src = dst->group_src | (1 << p->port_no);

				dst->ageing_timer = jiffies;

				tt1 = chk_igmp_ext_entry(dst , src); 
				if(tt1 == 0)
				{
					add_igmp_ext_entry(dst , src , port_comein);									
				}
				else
				{
					update_igmp_ext_entry(dst , src , port_comein);
				}	
			}
			else
			{
				/* insert one fdb entry */
				DEBUG_PRINT("insert one fdb entry\n");
				br_fdb_insert(br, p, dest);
				dst = __br_fdb_get(br, dest);
				if(dst !=NULL)
				{
					dst->igmpFlag=1;
					dst->is_local=0;
					dst->portlist = port_comein; 
					dst->group_src = dst->group_src | (1 << p->port_no);
				}
			}
		
	}
	else if (op == 2 && dst) /* delete */
	{
		DEBUG_PRINT("dst->group_src = %x change to ",dst->group_src);		
			del_group_src = ~(1 << p->port_no);
			dst->group_src = dst->group_src & del_group_src;
		DEBUG_PRINT(" %x ; p->port_no=%x \n",dst->group_src ,p->port_no);

		/*process wlan client leave --- start*/
		if (p && p->dev && p->dev->name && !memcmp(p->dev->name, RTL_PS_WLAN_NAME, 4)) 
		{ 			
			#ifdef	MCAST_TO_UNICAST
			//struct net_device *dev = __dev_get_by_name(&init_net,RTL_PS_WLAN0_DEV_NAME);
			struct net_device *dev=p->dev;
			if (dev) 
			{			
				unsigned char StaMacAndGroup[12];
				memcpy(StaMacAndGroup, dest , 6);
				memcpy(StaMacAndGroup+6, src, 6);
			#if defined(CONFIG_COMPAT_NET_DEV_OPS)
				if (dev->do_ioctl != NULL) 
				{
					dev->do_ioctl(dev, (struct ifreq*)StaMacAndGroup, 0x8B81);									
					DEBUG_PRINT("(del) wlan0 ioctl (del) M2U entry da:%02x:%02x:%02x-%02x:%02x:%02x; sa:%02x:%02x:%02x-%02x:%02x:%02x\n",
						StaMacAndGroup[0],StaMacAndGroup[1],StaMacAndGroup[2],StaMacAndGroup[3],StaMacAndGroup[4],StaMacAndGroup[5],
						StaMacAndGroup[6],StaMacAndGroup[7],StaMacAndGroup[8],StaMacAndGroup[9],StaMacAndGroup[10],StaMacAndGroup[11]);
				}
			#else
				if (dev->netdev_ops->ndo_do_ioctl != NULL) 
				{
					dev->netdev_ops->ndo_do_ioctl(dev, (struct ifreq*)StaMacAndGroup, 0x8B81);				
					DEBUG_PRINT("(del) wlan0 ioctl (del) M2U entry da:%02x:%02x:%02x-%02x:%02x:%02x; sa:%02x:%02x:%02x-%02x:%02x:%02x\n",
						StaMacAndGroup[0],StaMacAndGroup[1],StaMacAndGroup[2],StaMacAndGroup[3],StaMacAndGroup[4],StaMacAndGroup[5],
						StaMacAndGroup[6],StaMacAndGroup[7],StaMacAndGroup[8],StaMacAndGroup[9],StaMacAndGroup[10],StaMacAndGroup[11]);
				}
			#endif	
			
			}
			#endif	
			//dst->portlist &= ~0x80;	// move to del_igmp_ext_entry
			port_comein	= 0x80;
		}
		/*process wlan client leave --- end*/

		/*process entry del , portlist update*/
		del_igmp_ext_entry(dst , src ,port_comein);
		
		if (dst->portlist == 0)  // all joined sta are gone
		{
			DEBUG_PRINT("----all joined sta are gone,make it expired----\n");
			dst->ageing_timer -=  300*HZ; // make it expired		
		}
		

	}
}

#endif // CONFIG_RTL_IGMP_SNOOPING

