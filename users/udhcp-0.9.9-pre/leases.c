/* 
 * leases.c -- tools to manage DHCP leases 
 * Russ Dill <Russ.Dill@asu.edu> July 2001
 */

#include <time.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "debug.h"
#include "dhcpd.h"
#include "files.h"
#include "options.h"
#include "leases.h"
#include "arpping.h"

unsigned char blank_chaddr[] = {[0 ... 15] = 0};

#ifdef GUEST_ZONE
 int is_guest_mac(char *iface, unsigned char *addr)
{
	char tmpbuf[100];
	int ret;

	sprintf(tmpbuf, "brctl chkguestmac  %s 0 %02x%02x%02x%02x%02x%02x",
			iface, addr[0],addr[1],addr[2],addr[3],addr[4],addr[5]);

	ret = system(tmpbuf);
	if (ret != -1) 
		return (ret>>8);
	
	return 0;
}

struct guest_mac_entry *is_guest_exist(unsigned char *addr, struct guest_mac_entry **empty)
{	
	int i;

	if (empty)
		*empty = NULL;
	
	if (server_config.guestmac_check && server_config.guestmac_tbl) {
		for (i=0; i<server_config.max_leases; i++) {
			if (server_config.guestmac_tbl[i].valid &&
						!memcmp(server_config.guestmac_tbl[i].addr, addr, 6)) 
				return (void *)&server_config.guestmac_tbl[i];	
			
			if (empty && *empty == NULL) {
				if (!server_config.guestmac_tbl[i].valid) 
					*empty = &server_config.guestmac_tbl[i];			
			}
		}
	}
	return NULL;
}
#endif // GUEST_ZONE


/* clear every lease out that chaddr OR yiaddr matches and is nonzero */
void clear_lease(u_int8_t *chaddr, u_int32_t yiaddr)
{
	unsigned int i, j;
	
	for (j = 0; j < 16 && !chaddr[j]; j++);
	
	for (i = 0; i < server_config.max_leases; i++)
		if ((j != 16 && !memcmp(leases[i].chaddr, chaddr, 16)) ||
		    (yiaddr && leases[i].yiaddr == yiaddr)) {
			memset(&(leases[i]), 0, sizeof(struct dhcpOfferedAddr));
		}

#ifdef GUEST_ZONE
	{ 
		struct guest_mac_entry *guest = is_guest_exist(chaddr, NULL);
		if (guest) 			
			guest->valid = 0;	
	}
#endif	
}
#if defined(CONFIG_RTL8186_KB) || defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196C_EC)
struct dhcpOfferedAddr *add_lease_fromfile(u_int8_t *chaddr, u_int32_t yiaddr, unsigned long lease, char *hostname)
{
	struct dhcpOfferedAddr *oldest;
	/* clean out any old ones */
	clear_lease(chaddr, yiaddr);

	oldest = oldest_expired_lease();
	
	if (oldest) {
		memcpy(oldest->chaddr, chaddr, 16);
		oldest->yiaddr = yiaddr;
		oldest->expires = time(0) + lease;
		if(hostname[0] != '\0')
			sprintf(oldest->hostname, "%s", hostname);	
	}

#ifdef GUEST_ZONE
	if (server_config.guestmac_check) {
		if (is_guest_mac(server_config.interface, chaddr)) {
			struct guest_mac_entry *newguest;
			if (!is_guest_exist(chaddr, &newguest)) {				
				if (newguest) {					
					memcpy(newguest->addr, chaddr, 6);		
					newguest->valid = 1;					
				}
			}			
		}
		else {
			struct guest_mac_entry *guest = is_guest_exist(chaddr, NULL);
			if (guest)
				guest->valid = 0;
		}		
	}
#endif	

	return oldest;
}
#endif
/* add a lease into the table, clearing out any old ones */
struct dhcpOfferedAddr *add_lease(u_int8_t *chaddr, u_int32_t yiaddr, unsigned long lease)
{
	struct dhcpOfferedAddr *oldest;
	
	/* clean out any old ones */
	clear_lease(chaddr, yiaddr);
		
	oldest = oldest_expired_lease();
	
	if (oldest) {
		memcpy(oldest->chaddr, chaddr, 16);
		oldest->yiaddr = yiaddr;
		oldest->expires = time(0) + lease;
	}
	
#ifdef GUEST_ZONE
	if (server_config.guestmac_check) {	
		if (is_guest_mac(server_config.interface, chaddr)) {			
			struct guest_mac_entry *newguest;
			if (!is_guest_exist(chaddr, &newguest)) {		
				if (newguest) {					
					memcpy(newguest->addr, chaddr, 6);		
					newguest->valid = 1;					
				}
			}			
		}
		else {		
			struct guest_mac_entry *guest = is_guest_exist(chaddr, NULL);
			if (guest)
				guest->valid = 0;
		}		
	}
#endif	

	return oldest;
}


/* true if a lease has expired */
int lease_expired(struct dhcpOfferedAddr *lease)
{
#ifdef STATIC_LEASE
	if (reservedIp(server_config.static_leases, lease->yiaddr))
		return 0;
#endif		
	return (lease->expires < (unsigned long) time(0));
}	


/* Find the oldest expired lease, NULL if there are no expired leases */
struct dhcpOfferedAddr *oldest_expired_lease(void)
{
	struct dhcpOfferedAddr *oldest = NULL;
	unsigned long oldest_lease = time(0);
	unsigned int i;

	
	for (i = 0; i < server_config.max_leases; i++)
		if (oldest_lease > leases[i].expires) {
			oldest_lease = leases[i].expires;
			oldest = &(leases[i]);
		}
	return oldest;
		
}


/* Find the first lease that matches chaddr, NULL if no match */
struct dhcpOfferedAddr *find_lease_by_chaddr(u_int8_t *chaddr)
{
	unsigned int i;

	for (i = 0; i < server_config.max_leases; i++)
		if (!memcmp(leases[i].chaddr, chaddr, 16)) return &(leases[i]);
	
	return NULL;
}


/* Find the first lease that matches yiaddr, NULL is no match */
struct dhcpOfferedAddr *find_lease_by_yiaddr(u_int32_t yiaddr)
{
	unsigned int i;

	for (i = 0; i < server_config.max_leases; i++)
		if (leases[i].yiaddr == yiaddr) return &(leases[i]);
	
	return NULL;
}


/* find an assignable address, it check_expired is true, we check all the expired leases as well.
 * Maybe this should try expired leases by age... */
u_int32_t find_address(int check_expired) 
{
	u_int32_t addr, ret;
	struct dhcpOfferedAddr *lease = NULL;		

	addr = ntohl(server_config.start); /* addr is in host order here */
	for (;addr <= ntohl(server_config.end); addr++) {
#if 1
		/* Network IP and BroadcastIP 
		ie. netmask 255.255.255.0, server ip 192.168.55.1
		can't lease 192.168.55.0 or 192.168.55.255
		*/
		if(addr==(server_config.netmask & server_config.server))
			continue;
		if(addr==((~(server_config.netmask))|(server_config.server)))
			continue;
#else
		/* ie, 192.168.55.0 */
		if (!(addr & 0xFF)) continue;

		/* ie, 192.168.55.255 */
		if ((addr & 0xFF) == 0xFF) continue;
#endif
		/* not lease server ip */
        if(addr == server_config.server) continue;
        
#ifdef STATIC_LEASE
		/* Only do if it isn't an assigned as a static lease */		
		if(!reservedIp(server_config.static_leases, htonl(addr)))
#endif		
		{
			/* lease is not taken */
			ret = htonl(addr);
			if ((!(lease = find_lease_by_yiaddr(ret)) ||

			     /* or it expired and we are checking for expired leases */
			     ( (check_expired == 1)  && lease_expired(lease))) &&

			     /* and it isn't on the network */
		    	     !check_ip(ret)) {
				return ret;
			}
			else if(check_expired == 2) //some one leases this ip and this ip is not expired.
			{
				int arpping_time = 0;
				int ret_arpping = 0;
				
//printf("\r\n Any one use this ip[%x]?",ret);
				for(arpping_time = 0; arpping_time<3; arpping_time++)
				{
					char ret_hwaddr[6];
					if (arpping(ret, server_config.server, server_config.arp, server_config.interface, ret_hwaddr) == 0)
					{
						ret_arpping = 1;
						break;
					}					
				}

				// But this ip is not alive in network. we release this ip
				if ( ret_arpping == 1)
				{
//printf(" YES.__[%s-%u]\r\n",__FILE__,__LINE__);					
				}
				else
				{
//printf(" NO.__[%s-%u]\r\n",__FILE__,__LINE__);
					return ret;
				}				
			}
		}
	}
	return 0;
}


/* check if an IP is taken, if it is, add it to the lease table */
int check_ip(u_int32_t addr)
{
	struct in_addr temp;
	char ret_hwaddr[6];
	
	if (arpping(addr, server_config.server, server_config.arp, server_config.interface, ret_hwaddr) == 0) {
		temp.s_addr = addr;
	 	LOG(LOG_INFO, "%s belongs to someone, reserving it for %ld seconds", 
	 		inet_ntoa(temp), server_config.conflict_time);
		add_lease(blank_chaddr, addr, server_config.conflict_time);
		return 1;
	} else return 0;
}

