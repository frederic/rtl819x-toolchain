/* static_leases.h */
#ifndef _STATIC_LEASES_H
#define _STATIC_LEASES_H

#include "dhcpd.h"

/* Config file will pass static lease info to this function which will add it
 * to a data structure that can be searched later */
int addStaticLease(struct static_lease **lease_struct, unsigned char *mac, u_int32_t *ip, char *host);

/* Check to see if a mac has an associated static lease */
u_int32_t getIpByMac(struct static_lease *lease_struct, void *arg, char **host);

/* Check to see if a host has an associated static lease */
u_int32_t getIpByHost(struct static_lease *lease_struct, void *arg, int len, char **host);

/* Check to see if an ip is reserved as a static ip */
u_int32_t reservedIp(struct static_lease *lease_struct, u_int32_t ip);

#ifdef UDHCP_DEBUG
/* Print out static leases just to check what's going on */
void printStaticLeases(struct static_lease **lease_struct);
#endif

#endif



