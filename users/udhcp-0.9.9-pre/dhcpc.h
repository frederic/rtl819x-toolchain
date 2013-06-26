/* dhcpc.h */
#ifndef _DHCPC_H
#define _DHCPC_H

#include "libbb_udhcp.h"

#define SEND_GRATUITOUS_ARP

#define INIT_SELECTING	0
#define REQUESTING	1
#define BOUND		2
#define RENEWING	3
#define REBINDING	4
#define INIT_REBOOT	5
#define RENEW_REQUESTED 6
#define RELEASED	7


struct client_config_t {
	char foreground;		/* Do not fork */
	char quit_after_lease;		/* Quit after obtaining lease */
	char abort_if_no_lease;		/* Abort if no lease */
	char background_if_no_lease;	/* Fork to background if no lease */
	char *interface;		/* The name of the interface to use */
	char *pidfile;			/* Optionally store the process ID */
	char *script;			/* User script to run at dhcp events */
	unsigned char *clientid;	/* Optional client id to use */
	unsigned char *hostname;	/* Optional hostname to use */
	int ifindex;			/* Index number of the interface to use */
	unsigned char arp[6];		/* Our arp address */
#ifdef CHECK_SERVER_ALIVE
	time_t alive_time;
	char *url_name;
#endif	
#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL865X_AC)
	int wan_type;
	int Inform;
	int broadcast_flag;
#endif
};

extern struct client_config_t client_config;


#endif
