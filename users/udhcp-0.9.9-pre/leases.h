/* leases.h */
#ifndef _LEASES_H
#define _LEASES_H


struct dhcpOfferedAddr {
	u_int8_t chaddr[16];
	u_int32_t yiaddr;	/* network order */
	u_int32_t expires;	/* host order */
#if defined(CONFIG_RTL8186_KB) || defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD)
	char hostname[64]; /* Brad add for get hostname of client */
	u_int32_t isUnAvailableCurr;	/* Brad add for WEB GUI check */
#endif
};

extern unsigned char blank_chaddr[];

void clear_lease(u_int8_t *chaddr, u_int32_t yiaddr);
struct dhcpOfferedAddr *add_lease(u_int8_t *chaddr, u_int32_t yiaddr, unsigned long lease);
int lease_expired(struct dhcpOfferedAddr *lease);
struct dhcpOfferedAddr *oldest_expired_lease(void);
struct dhcpOfferedAddr *find_lease_by_chaddr(u_int8_t *chaddr);
struct dhcpOfferedAddr *find_lease_by_yiaddr(u_int32_t yiaddr);
u_int32_t find_address(int check_expired);
int check_ip(u_int32_t addr);
#ifdef GUEST_ZONE
 int is_guest_mac(char *iface, unsigned char *addr);
struct guest_mac_entry *is_guest_exist(unsigned char *addr, struct guest_mac_entry **empty);
#endif

#endif
