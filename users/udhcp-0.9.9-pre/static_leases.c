/*
 * static_leases.c -- Couple of functions to assist with storing and
 * retrieving data for static leases
 *
 * Wade Berrier <wberrier@myrealbox.com> September 2004
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "static_leases.h"
#include "dhcpd.h"

/* Takes the address of the pointer to the static_leases linked list,
 *   Address to a 6 byte mac address
 *   Address to a 4 byte ip address */
int addStaticLease(struct static_lease **lease_struct, unsigned char *mac, u_int32_t *ip, char *host)
{
	struct static_lease *cur;
	struct static_lease *new_static_lease;

	/* Build new node */
	new_static_lease = xmalloc(sizeof(struct static_lease));
	new_static_lease->mac = mac;
	new_static_lease->ip = ip;
	new_static_lease->host = host;	
	new_static_lease->next = NULL;

	/* If it's the first node to be added... */
	if(*lease_struct == NULL)
	{
		*lease_struct = new_static_lease;
	}
	else
	{
		cur = *lease_struct;
		while(cur->next != NULL)
		{
			cur = cur->next;
		}

		cur->next = new_static_lease;
	}

	return 1;

}

/* Check to see if a mac has an associated static lease */
u_int32_t getIpByMac(struct static_lease *lease_struct, void *arg, char **host)
{
	u_int32_t return_ip;
	struct static_lease *cur = lease_struct;
	unsigned char *mac = arg;

	return_ip = 0;

	while(cur != NULL)
	{
		/* If the client has the correct mac  */
		if(memcmp(cur->mac, mac, 6) == 0)
		{
			return_ip = *(cur->ip);
			*host = cur->host;
		}

		cur = cur->next;
	}

	return return_ip;

}


/* Check to see if a host has an associated static lease */
u_int32_t getIpByHost(struct static_lease *lease_struct, void *arg, int len, char **host)
{
	u_int32_t return_ip;
	struct static_lease *cur = lease_struct;
	unsigned char *name = arg;

	return_ip = 0;

	while(cur != NULL)
	{
		/* If the client has the correct host  */
		if(cur->host && (strlen(cur->host) == (size_t)len) &&
				memcmp(cur->host, name, len) == 0)
		{
			return_ip = *(cur->ip);
			*host = cur->host;			
		}

		cur = cur->next;
	}
	
	return return_ip;
}


/* Check to see if an ip is reserved as a static ip */
u_int32_t reservedIp(struct static_lease *lease_struct, u_int32_t ip)
{
	struct static_lease *cur = lease_struct;

	u_int32_t return_val = 0;

	while(cur != NULL)
	{
		/* If the client has the correct ip  */
		if(*cur->ip == ip)
			return_val = 1;

		cur = cur->next;
	}

	return return_val;

}
#ifdef UDHCP_DEBUG
/* Print out static leases just to check what's going on */
/* Takes the address of the pointer to the static_leases linked list */
void printStaticLeases(struct static_lease **arg)
{
	/* Get a pointer to the linked list */
	struct static_lease *cur = *arg;

	while(cur != NULL)
	{
		/* printf("PrintStaticLeases: Lease mac Address: %x\n", cur->mac); */
		printf("PrintStaticLeases: Lease mac Value: %02x%02x%02x%02x%02x%02x\n", *(cur->mac), 
		*(cur->mac+1), *(cur->mac+2), *(cur->mac+3), *(cur->mac+4), *(cur->mac+5));
		/* printf("PrintStaticLeases: Lease ip Address: %x\n", cur->ip); */
		printf("PrintStaticLeases: Lease ip Value: %x\n", *(cur->ip));
		printf("PrintStaticLeases: hostname: %s\n", cur->host);
		cur = cur->next;
	}


}
#endif



