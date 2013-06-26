/* serverpacket.c
 *
 * Constuct and send DHCP server packets
 *
 * Russ Dill <Russ.Dill@asu.edu> July 2001
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

#include "packet.h"
#include "debug.h"
#include "dhcpd.h"
#include "options.h"
#include "leases.h"
#ifdef STATIC_LEASE
#include "static_leases.h"
#endif
#if defined(CONFIG_RTL865X_KLD)	
extern unsigned char update_lease_time;
extern unsigned char update_lease_time1;
#endif
/* send a packet to giaddr using the kernel ip stack */
static int send_packet_to_relay(struct dhcpMessage *payload)
{
	DEBUG(LOG_INFO, "Forwarding packet to relay");

	return kernel_packet(payload, server_config.server, SERVER_PORT,
			payload->giaddr, SERVER_PORT);
}


/* send a packet to a specific arp address and ip address by creating our own ip packet */
static int send_packet_to_client(struct dhcpMessage *payload, int force_broadcast)
{
	unsigned char *chaddr;
	u_int32_t ciaddr;
	
	if (force_broadcast) {
		DEBUG(LOG_INFO, "broadcasting packet to client (NAK)");
		ciaddr = INADDR_BROADCAST;
		chaddr = MAC_BCAST_ADDR;	
	} else if (ntohs(payload->flags) & BROADCAST_FLAG) {
		DEBUG(LOG_INFO, "broadcasting packet to client (requested)");
		ciaddr = INADDR_BROADCAST;
		chaddr = MAC_BCAST_ADDR;
	} else if (payload->ciaddr) {
		DEBUG(LOG_INFO, "unicasting packet to client ciaddr");
		ciaddr = payload->ciaddr;
		chaddr = payload->chaddr;		
	} else {
		DEBUG(LOG_INFO, "unicasting packet to client yiaddr");
		ciaddr = payload->yiaddr;
		chaddr = payload->chaddr;
	}
	return raw_packet(payload, server_config.server, SERVER_PORT, 
			ciaddr, CLIENT_PORT, chaddr, server_config.ifindex);
}


/* send a dhcp packet, if force broadcast is set, the packet will be broadcast to the client */
static int send_packet(struct dhcpMessage *payload, int force_broadcast)
{
	int ret;

	if (payload->giaddr)
		ret = send_packet_to_relay(payload);
	else ret = send_packet_to_client(payload, force_broadcast);
	return ret;
}


static void init_packet(struct dhcpMessage *packet, struct dhcpMessage *oldpacket, char type)
{
	init_header(packet, type);
	packet->xid = oldpacket->xid;
	memcpy(packet->chaddr, oldpacket->chaddr, 16);
	packet->flags = oldpacket->flags;
	packet->giaddr = oldpacket->giaddr;
	packet->ciaddr = oldpacket->ciaddr;
	add_simple_option(packet->options, DHCP_SERVER_ID, server_config.server);
}


/* add in the bootp options */
static void add_bootp_options(struct dhcpMessage *packet)
{
	packet->siaddr = server_config.siaddr;
	if (server_config.sname)
		strncpy(packet->sname, server_config.sname, sizeof(packet->sname) - 1);
	if (server_config.boot_file)
		strncpy(packet->file, server_config.boot_file, sizeof(packet->file) - 1);
}
	

/* send a DHCP OFFER to a DHCP DISCOVER */
int sendOffer(struct dhcpMessage *oldpacket)
{
	struct dhcpMessage packet;
	struct dhcpOfferedAddr *lease = NULL;
	u_int32_t req_align, lease_time_align = server_config.lease;
	unsigned char *req, *lease_time;
	struct option_set *curr;
	struct in_addr addr;
#ifdef STATIC_LEASE
	u_int32_t static_lease_ip;
	char *host, *sname;
	int len;
#endif

	{ 
                   u_int8_t empty_haddr[16]; 
    
                   memset(empty_haddr, 0, 16); 
                   if (!memcmp(oldpacket->chaddr, empty_haddr, 16)) { 
                           LOG(LOG_WARNING, "Empty Client Hardware Addresses"); 
                           return -1; 
                   } 
        } 	


	init_packet(&packet, oldpacket, DHCPOFFER);
	
#ifdef STATIC_LEASE
	static_lease_ip = getIpByMac(server_config.static_leases, oldpacket->chaddr, &host);
	sname = get_option(oldpacket, DHCP_HOST_NAME);
	if (sname)
		len = (int)sname[-1];
	else
		len = 0;

	if (!static_lease_ip && len)
		static_lease_ip = getIpByHost(server_config.static_leases, sname, len, &host);
	
	if(!static_lease_ip || 
		(static_lease_ip && host && 
			((strlen(host)!=(size_t)len) || memcmp(host, sname, len))))
#endif		
	{
		/* the client is in our lease/offered table */
		if ((lease = find_lease_by_chaddr(oldpacket->chaddr))) {
			if (!lease_expired(lease)) 
				lease_time_align = lease->expires - time(0);
			packet.yiaddr = lease->yiaddr;
			
		/* Or the client has a requested ip */
		} else if ((req = get_option(oldpacket, DHCP_REQUESTED_IP)) &&

			   /* Don't look here (ugly hackish thing to do) */
			   memcpy(&req_align, req, 4) &&

			   /* and the ip is in the lease range */
			   ntohl(req_align) >= ntohl(server_config.start) &&
			   ntohl(req_align) <= ntohl(server_config.end) &&
			   ntohl(req_align)!=ntohl(server_config.server) &&
#ifdef STATIC_LEASE
				!reservedIp(server_config.static_leases, req_align) &&
#endif		
			   
			   /* and its not already taken/offered */ /* ADDME: check that its not a static lease */
			   ((!(lease = find_lease_by_yiaddr(req_align)) ||
			   
			   /* or its taken, but expired */ /* ADDME: or maybe in here */
			   lease_expired(lease)))) {
					packet.yiaddr = req_align; /* FIXME: oh my, is there a host using this IP? */

		/* otherwise, find a free IP */ /*ADDME: is it a static lease? */
		} else {
			packet.yiaddr = find_address(0);
			
			/* try for an expired lease */
			if (!packet.yiaddr) packet.yiaddr = find_address(1);
			if (!packet.yiaddr) packet.yiaddr = find_address(2);
		}
		
		if(!packet.yiaddr) {
			LOG(LOG_WARNING, "no IP addresses to give -- OFFER abandoned");
			return -1;
		}
		
		if (!add_lease(packet.chaddr, packet.yiaddr, server_config.offer_time)) {
			LOG(LOG_WARNING, "lease pool is full -- OFFER abandoned");
			return -1;
		}		

		if ((lease_time = get_option(oldpacket, DHCP_LEASE_TIME))) {
			memcpy(&lease_time_align, lease_time, 4);
			lease_time_align = ntohl(lease_time_align);
			if (lease_time_align > server_config.lease) 
				lease_time_align = server_config.lease;
		}

		/* Make sure we aren't just using the lease time from the previous offer */
		if (lease_time_align < server_config.min_lease) 
			lease_time_align = server_config.lease;
	}
#ifdef STATIC_LEASE
	/* ADDME: end of short circuit */		
	else
	{
		/* It is a static lease... use it */
		packet.yiaddr = static_lease_ip;
		if (!add_lease(packet.chaddr, packet.yiaddr, 0xffffffff)) {
			LOG(LOG_WARNING, "lease pool is full -- OFFER abandoned");
			return -1;
		}				
		lease_time_align = 0xffffffff;
	}
#endif	
#if defined(CONFIG_RTL865X_KLD)	
	if(server_config.upateConfig_isp == 1){
		if(update_lease_time){
			lease_time_align = server_config.min_lease;
		}
	}
	if(server_config.upateConfig_isp_dns == 1){
		if(update_lease_time1){	
			lease_time_align = server_config.min_lease;
		}
	}
#endif
	add_simple_option(packet.options, DHCP_LEASE_TIME, htonl(lease_time_align));

	curr = server_config.options;
	while (curr) {
		if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
			add_option_string(packet.options, curr->data);
		curr = curr->next;
	}

	add_bootp_options(&packet);
	
	addr.s_addr = packet.yiaddr;
	LOG(LOG_INFO, "sending OFFER of %s", inet_ntoa(addr));
	return send_packet(&packet, 0);
}


int sendNAK(struct dhcpMessage *oldpacket)
{
	struct dhcpMessage packet;

	init_packet(&packet, oldpacket, DHCPNAK);
	
	DEBUG(LOG_INFO, "sending NAK");
	#if defined(CONFIG_RTL865X_KLD)
		return send_packet(&packet, server_config.response_broadcast);
	#elif defined(CONFIG_RTL_ULINKER)
		return send_packet(&packet, 1);
	#else
		return send_packet(&packet, 0);
	#endif
}


int sendACK(struct dhcpMessage *oldpacket, u_int32_t yiaddr)
{
	struct dhcpMessage packet;
	struct option_set *curr;
	unsigned char *lease_time;
	u_int32_t lease_time_align = server_config.lease;
	struct in_addr addr;

	init_packet(&packet, oldpacket, DHCPACK);
	packet.yiaddr = yiaddr;
	
	if ((lease_time = get_option(oldpacket, DHCP_LEASE_TIME))) {
		memcpy(&lease_time_align, lease_time, 4);
		lease_time_align = ntohl(lease_time_align);
		if (lease_time_align > server_config.lease) 
			lease_time_align = server_config.lease;
		else if (lease_time_align < server_config.min_lease) 
			lease_time_align = server_config.lease;
	}

#ifdef STATIC_LEASE
	if (reservedIp(server_config.static_leases, yiaddr))
		lease_time_align = 0xffffffff;
#endif
#if defined(CONFIG_RTL865X_KLD)	
	if(server_config.upateConfig_isp == 1){
		if(update_lease_time)
			lease_time_align = server_config.min_lease;
	}
	if(server_config.upateConfig_isp_dns == 1){
		if(update_lease_time1){
			lease_time_align = server_config.min_lease;
		}
	}
#endif	
	add_simple_option(packet.options, DHCP_LEASE_TIME, htonl(lease_time_align));
	
	curr = server_config.options;
	while (curr) {
		if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
			add_option_string(packet.options, curr->data);
		curr = curr->next;
	}

	add_bootp_options(&packet);

	addr.s_addr = packet.yiaddr;
	LOG(LOG_INFO, "sending ACK to %s", inet_ntoa(addr));
	#if defined(CONFIG_RTL865X_KLD)
			if (send_packet(&packet, server_config.response_broadcast) < 0) 	
				return -1;
	#else
	if (send_packet(&packet, 0) < 0)
		return -1;
	#endif
	add_lease(packet.chaddr, packet.yiaddr, lease_time_align);

	return 0;
}


int send_inform(struct dhcpMessage *oldpacket)
{
	struct dhcpMessage packet;
	struct option_set *curr;

	init_packet(&packet, oldpacket, DHCPACK);
	
	curr = server_config.options;
	while (curr) {
		if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
			add_option_string(packet.options, curr->data);
		curr = curr->next;
	}

	add_bootp_options(&packet);
	#if defined(CONFIG_RTL865X_KLD)
		return send_packet(&packet, server_config.response_broadcast);				
	#else
	return send_packet(&packet, 0);
	#endif
}



