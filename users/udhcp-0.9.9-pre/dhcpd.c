/* dhcpd.c
 *
 * udhcp Server
 * Copyright (C) 1999 Matthew Ramsay <matthewr@moreton.com.au>
 *			Chris Trew <ctrew@moreton.com.au>
 *
 * Rewrite by Russ Dill <Russ.Dill@asu.edu> July 2001
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

#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>

#include "debug.h"
#include "dhcpd.h"
#include "arpping.h"
#include "socket.h"
#include "options.h"
#include "files.h"
#include "leases.h"
#include "packet.h"
#include "serverpacket.h"
#include "pidfile.h"
#ifdef STATIC_LEASE
#include "static_leases.h"
#endif
#if defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196C_EC)
#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]
unsigned char update_lease_time=0;
unsigned char update_lease_time1=0;
unsigned char update_option_dns=0;
#endif
/* globals */
struct dhcpOfferedAddr *leases;
struct server_config_t server_config;
static int signal_pipe[2];

/* Exit and cleanup */
static void exit_server(int retval)
{
	pidfile_delete(server_config.pidfile);
	CLOSE_LOG();
	exit(retval);
}


/* Signal handler */
static void signal_handler(int sig)
{
	if (send(signal_pipe[1], &sig, sizeof(sig), MSG_DONTWAIT) < 0) {
		LOG(LOG_ERR, "Could not send signal: %s", 
			strerror(errno));
	}
}
#if defined(CONFIG_RTL865X_KLD)	
static char *get_token(char *data, char *token)
{
	char *ptr=data;
	int len=0, idx=0;

	while (*ptr && *ptr != '\n' ) {
		if (*ptr == '=') {
			if (len <= 1)
				return NULL;
			memcpy(token, data, len);

			/* delete ending space */
			for (idx=len-1; idx>=0; idx--) {
				if (token[idx] !=  ' ')
					break;
			}
			token[idx+1] = '\0';

			return ptr+1;
		}
		len++;
		ptr++;
	}
	return NULL;
}

static int get_value(char *data, char *value)
{
	char *ptr=data;	
	int len=0, idx, i;

	while (*ptr && *ptr != '\n' && *ptr != '\r') {
		len++;
		ptr++;
	}

	/* delete leading space */
	idx = 0;
	while (len-idx > 0) {
		if (data[idx] != ' ') 
			break;	
		idx++;
	}
	len -= idx;

	/* delete bracing '"' */
	if (data[idx] == '"') {
		for (i=idx+len-1; i>idx; i--) {
			if (data[i] == '"') {
				idx++;
				len = i - idx;
			}
			break;
		}
	}

	if (len > 0) {
		memcpy(value, &data[idx], len);
		value[len] = '\0';
	}
	return len;
}
#endif
#ifdef COMBINED_BINARY	
int udhcpd_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{	
	fd_set rfds;
	struct timeval tv;
	int server_socket = -1;
	int bytes, retval;
	struct dhcpMessage packet;
	unsigned char *state;
	unsigned char *server_id, *requested;
	u_int32_t server_id_align, requested_align;
	unsigned long timeout_end;
	struct option_set *option;
	struct dhcpOfferedAddr *lease;
	int pid_fd;
	int max_sock;
	int sig;
	unsigned long num_ips;
#ifdef STATIC_LEASE	
	u_int32_t static_lease_ip;
	char *host, *sname;
	int len;
	int isStatic_Lease_Entry=0;
#endif
	FILE *fp, *fp_action;
	char Action[30];
	char revoke_ip[30];
	unsigned long sysTime_orig=0;
	char tmpBuf2[100];
		char token[60], value[60], *ptr, optdata[60];
#if defined(CONFIG_RTL8186_KB) || defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196C_EC)
	char *hostname;
#endif
#if defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196C_EC)
	char logbuf[500];
#endif
	OPEN_LOG("udhcpd");
	LOG(LOG_INFO, "udhcp server (v%s) started", VERSION);

	memset(&server_config, 0, sizeof(struct server_config_t));
	
	if (argc < 2)
		read_config(DHCPD_CONF_FILE);
	else read_config(argv[1]);

	pid_fd = pidfile_acquire(server_config.pidfile);
	pidfile_write_release(pid_fd);

	if ((option = find_option(server_config.options, DHCP_LEASE_TIME))) {
		memcpy(&server_config.lease, option->data + 2, 4);
		server_config.lease = ntohl(server_config.lease);
	}
	else server_config.lease = LEASE_TIME;
#if defined(CONFIG_RTL865X_KLD)	
	if(server_config.upateConfig_isp==1){
			update_lease_time = 1;
			fp=fopen("/var/isp_dhcp.conf","r");
			if(fp){
						while ( fgets(tmpBuf2, 100, fp) ) {
								ptr = get_token(tmpBuf2, token);
								if (ptr == NULL)
										continue;
								if (get_value(ptr, value)==0)
										continue;
								sprintf(optdata, "%s %s", token, value);
									read_opt_from_isp(optdata);
							}
								fclose(fp);
								update_lease_time =0;
				}
		}else{
			update_lease_time = 0;
		}
		
		if(server_config.upateConfig_isp_dns==1){
			update_lease_time1 =1;
			fp=fopen("/etc/resolv.conf","r");
			if(fp){
					update_option_dns = 0;
					while (fgets(tmpBuf2, 200, fp) ) {		
						if (sscanf(tmpBuf2, "nameserver %s", value)) {
								sprintf(optdata, "dns %s", value);
								read_opt_from_isp(optdata);
						}
					}
						fclose(fp);
						update_lease_time1 =0;
				}
			
		}else{
			update_lease_time1 =0;
		}
		
		
#endif			
	/* Sanity check */
	// 2007.12.24, Forrest Lin.
	// Number of IPs should be $end - $start + 1 or one ip will be lost.
	// num_ips = ntohl(server_config.end) - ntohl(server_config.start);
	num_ips = ntohl(server_config.end) - ntohl(server_config.start) + 1;
	if(server_config.server>=server_config.start && server_config.server<=server_config.end)
		num_ips--;
	if (server_config.max_leases > num_ips) {
// david, disable message. 2003-5-21 
//		LOG(LOG_ERR, "max_leases value (%lu) not sane, setting to %lu instead",
//			server_config.max_leases, num_ips);
		server_config.max_leases = num_ips;
	}
	if (read_interface(server_config.interface, &server_config.ifindex,
			   &server_config.server, server_config.arp) < 0)
		exit_server(1);
	if(read_interface_netmask(server_config.interface,&server_config.netmask)<0)
		exit_server(1);
	leases = xmalloc(sizeof(struct dhcpOfferedAddr) * server_config.max_leases);
	memset(leases, 0, sizeof(struct dhcpOfferedAddr) * server_config.max_leases);
	read_leases(server_config.lease_file);

#ifdef GUEST_ZONE
	if (server_config.guestmac_check) {
		server_config.guestmac_tbl = xmalloc(sizeof(struct guest_mac_entry) * server_config.max_leases);
		memset(server_config.guestmac_tbl , 0, sizeof(struct guest_mac_entry) * server_config.max_leases);	
	}
#endif



#ifndef DEBUGGING
	pid_fd = pidfile_acquire(server_config.pidfile); /* hold lock during fork. */
	if (daemon(0, 1) == -1) {
		perror("fork");
		exit_server(1);
	}
	pidfile_write_release(pid_fd);
#endif


	socketpair(AF_UNIX, SOCK_STREAM, 0, signal_pipe);
	signal(SIGUSR1, signal_handler);
//Brad add sync system time for dhcpd 2008/01/28 		
	signal(SIGUSR2, signal_handler);
//----------------------------------------------------------
	signal(SIGTERM, signal_handler);

	timeout_end = time(0) + server_config.auto_time;
	while(1) { /* loop until universe collapses */

#ifdef STATIC_LEASE
		isStatic_Lease_Entry = 0;
#endif
		if (server_socket < 0)
			if ((server_socket = listen_socket(INADDR_ANY, SERVER_PORT, server_config.interface)) < 0) {
				LOG(LOG_ERR, "FATAL: couldn't create server socket, %s", strerror(errno));
				exit_server(0);
			}			

		FD_ZERO(&rfds);
		FD_SET(server_socket, &rfds);
		FD_SET(signal_pipe[0], &rfds);
		if (server_config.auto_time) {
			tv.tv_sec = timeout_end - time(0);
			tv.tv_usec = 0;
		}
		if (!server_config.auto_time || tv.tv_sec > 0) {
			max_sock = server_socket > signal_pipe[0] ? server_socket : signal_pipe[0];
			retval = select(max_sock + 1, &rfds, NULL, NULL, 
					server_config.auto_time ? &tv : NULL);
		} else retval = 0; /* If we already timed out, fall through */

		if (retval == 0) {
			write_leases();
			timeout_end = time(0) + server_config.auto_time;
			continue;
		} else if (retval < 0 && errno != EINTR) {
			DEBUG(LOG_INFO, "error on select");
			continue;
		}
		
		if (FD_ISSET(signal_pipe[0], &rfds)) {
			if (read(signal_pipe[0], &sig, sizeof(sig)) < 0)
				continue; /* probably just EINTR */
			switch (sig) {
			case SIGUSR1:
				LOG(LOG_INFO, "Received a SIGUSR1");
				write_leases();
				/* why not just reset the timeout, eh */
				timeout_end = time(0) + server_config.auto_time;
				continue;
			case SIGTERM:
				LOG(LOG_INFO, "Received a SIGTERM");
				exit_server(0);
				
			case SIGUSR2:	
				LOG(LOG_INFO, "Received a SIGUSR2");
				fp_action = fopen("/tmp/dhcpd_action", "r");
				if(fp_action){
					memset(Action, '\0', 30);
					fscanf(fp_action, "%s", Action);
					fclose(fp_action);
				}else{
					continue;
				}
				if(!strcmp(Action, "time_update")){
				fp=fopen("/tmp/dhcpd_unix","r");
				if(fp){
  					fscanf(fp,"%ld",&sysTime_orig);
  					fclose(fp);
  					system("rm -f /tmp/dhcpd_unix");
  					read_leases_update_expire(server_config.lease_file, sysTime_orig);
				}
				}		
				#if defined(CONFIG_RTL865X_KLD)	
				if(!strcmp(Action, "revoke")){
						fp=fopen("/tmp/revoke_ip","r");
						if(fp){
							memset(revoke_ip, '\0', 30);
		  					fscanf(fp, "%s", revoke_ip);
		  					fclose(fp);
		  					revoke_leases(revoke_ip);
		  					system("rm -f /tmp/revoke_ip");
					}		
				}
				if(!strcmp(Action, "update_conf_isp")){		
					
							if(server_config.upateConfig_isp==1){
							fp=fopen("/var/isp_dhcp.conf","r");
							if(fp){
									while ( fgets(tmpBuf2, 100, fp) ) {
												ptr = get_token(tmpBuf2, token);
											if (ptr == NULL)
												continue;
											if (get_value(ptr, value)==0)
													continue;
											sprintf(optdata, "%s %s", token, value);
											read_opt_from_isp(optdata);
									}
								fclose(fp);
								update_lease_time =0;
								//system("rm -f /var/isp_dhcp.conf");
							}else{
								update_lease_time =0;
							}	
					}
			}
				if(!strcmp(Action, "update_conf_dns")){		
					
							if(server_config.upateConfig_isp_dns==1){
									update_lease_time1 =1;
									fp=fopen("/etc/resolv.conf","r");
									if(fp){
											update_option_dns = 0;
											while (fgets(tmpBuf2, 200, fp) ) {		
												if (sscanf(tmpBuf2, "nameserver %s", value)) {
														sprintf(optdata, "dns %s", value);
														read_opt_from_isp(optdata);
												}
											}
											fclose(fp);
											update_lease_time1 = 0;
									}else{
										update_lease_time1 = 0;
									}
							}
			}
			#endif //for kld
			#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL8196C_EC)
					if(!strcmp(Action, "update_conf_isp")){	
						if(server_config.upateConfig_isp==1){
								char tmpBuf[20];
								char tmpBuf1[60];
								char tmpBuf2[100];
								int wanType=0;
								fp=fopen("/var/isp_dhcp.conf","r");
								if(fp){
									//fgets(tmpBuf,sizeof(tmpBuf),fp);
									fscanf(fp,"%s %s %d",tmpBuf, tmpBuf1, &wanType);
									sprintf(tmpBuf2, "%s %s", tmpBuf, tmpBuf1);
									read_opt_from_isp(tmpBuf2);
									fclose(fp);

										if(server_config.upateConfig_isp==1){
											update_lease_time =0;
										}
								}else{
											//we donot receive the conf from isp, then we do not update lease time in next
										update_lease_time =0;
									}
							}
						}
			#endif //for tr domain name
				continue;
			}
		}
		if ((bytes = get_packet(&packet, server_socket)) < 0) { /* this waits for a packet - idle */
			if (bytes == -1 && errno != EINTR) {
				DEBUG(LOG_INFO, "error on read, %s, reopening socket", strerror(errno));
				close(server_socket);
				server_socket = -1;
			}
			continue;
		}

		if ((state = get_option(&packet, DHCP_MESSAGE_TYPE)) == NULL) {
			DEBUG(LOG_ERR, "couldn't get option from packet, ignoring");
			continue;
		}
		
#ifdef STATIC_LEASE
		/* Look for a static lease */
		static_lease_ip = getIpByMac(server_config.static_leases, packet.chaddr, &host);
		sname = get_option(&packet, DHCP_HOST_NAME);
		if (sname)
			len = (int)sname[-1];
		else
			len = 0;		

		if (!static_lease_ip && len) {
			static_lease_ip = getIpByHost(server_config.static_leases, sname, len, &host);
		}
		if(static_lease_ip && 
			((host == NULL) || (host && (len==(int)strlen(host)) && !memcmp(sname, host, len))))
		{
//			printf("Found static lease: %x\n", static_lease_ip);

/*
			memcpy(&static_lease.chaddr, &packet.chaddr, 16);
			static_lease.yiaddr = static_lease_ip;
			static_lease.expires = 0;
			lease = &static_lease;
*/
			lease = find_lease_by_chaddr(packet.chaddr);
			if (lease == NULL) {
				if (!add_lease(packet.chaddr, static_lease_ip, 0xffffffff)) {
					LOG(LOG_WARNING, "lease pool is full -- OFFER abandoned");
					continue;
				}						
			}			
			isStatic_Lease_Entry = 1;			
		}
		else
#endif
		{
		lease = find_lease_by_chaddr(packet.chaddr);
		}

		switch (state[0]) {
		case DHCPDISCOVER:
			DEBUG(LOG_INFO,"received DISCOVER");
			
			if (sendOffer(&packet) < 0) {
				LOG(LOG_ERR, "send OFFER failed");
			}
			break;			
 		case DHCPREQUEST:
			DEBUG(LOG_INFO, "received REQUEST");

			requested = get_option(&packet, DHCP_REQUESTED_IP);
			server_id = get_option(&packet, DHCP_SERVER_ID);
#if defined(CONFIG_RTL8186_KB) || defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196C_EC)
			hostname = get_option(&packet, DHCP_HOST_NAME);
#endif
			if (requested) memcpy(&requested_align, requested, 4);
			if (server_id) memcpy(&server_id_align, server_id, 4);
		
			if (lease) { /*ADDME: or static lease */
				if (server_id) {
					/* SELECTING State */
					DEBUG(LOG_INFO, "server_id = %08x", ntohl(server_id_align));
					if (server_id_align == server_config.server && requested && 
					    requested_align == lease->yiaddr) {
						sendACK(&packet, lease->yiaddr);
					}
				} 
				else
				{
					if ( (requested && (lease->yiaddr == requested_align))
					|| (lease->yiaddr == packet.ciaddr) 
					)
					{
						sendACK(&packet, lease->yiaddr);
					}					
					else
					{
						if (requested)
						{
							memcpy(&requested_align, requested, 4);
						}
						else if(packet.ciaddr != 0)
						{
							memcpy(&requested_align, &packet.ciaddr, 4);					
						}
						else
						{
							sendNAK(&packet);
							break;
						}
						/* INIT-REBOOT State */
						if (requested_align < server_config.start || 
							   requested_align > server_config.end ||
							   requested_align == server_config.server) 
						{
							sendNAK(&packet);
						}
#if 0	// do we need to check lease table or just ask????
						else if ((lease = find_lease_by_yiaddr(requested_align))) 
						{
							if (lease_expired(lease)) 
							{
								/* probably best if we drop this lease */
								memset(lease->chaddr, 0, 16);
							/* make some contention for this address */
							} 
							else 
							{
								sendNAK(&packet);
							}
						}
#endif						
						else
						{
							 
							int arpping_time = 0;
							int ret_arpping = 0;
							int retNAK = 1;
							
							char ret_hwaddr[6];
							memset(ret_hwaddr, 0x00, sizeof(ret_hwaddr));
								
							for(arpping_time = 0; arpping_time<1; arpping_time++)
							{						
								if (arpping(requested_align, server_config.server, server_config.arp, server_config.interface, ret_hwaddr) == 0)
								{
									ret_arpping = 1;
									break;
								}					
							}
		
							if(ret_arpping == 1)
							{						
								if(packet.ciaddr != 0 && memcmp(ret_hwaddr,packet.chaddr, 6) == 0)
								{
									retNAK = 0;
								}
								else
								{
									retNAK = 1;
								}
							}
							else
							{
								retNAK = 0;
							}
							
							// But this ip is not alive in network. we release this ip							
							if ( retNAK == 1)
							{
								sendNAK(&packet);
							}
							else
							{
								lease->expires = time(0); // free original lease item.
								
								if(packet.yiaddr == 0)
								{
									packet.yiaddr = requested_align;
								}
									
								if (!add_lease(packet.chaddr, packet.yiaddr, server_config.offer_time))
								{
										LOG(LOG_WARNING, "lease pool is full -- OFFER abandoned");
										sendNAK(&packet);
								}
								else
								{
									sendACK(&packet, packet.yiaddr);
								}
							}				
		
							
							
						}
	
					}
				
#if 0				
					if (requested) {
						/* INIT-REBOOT State */
						if (lease->yiaddr == requested_align)
							sendACK(&packet, lease->yiaddr);
						else sendNAK(&packet);
					} else {
						/* RENEWING or REBINDING State */
						if (lease->yiaddr == packet.ciaddr)
							sendACK(&packet, lease->yiaddr);
						else {
							/* don't know what to do!!!! */
							sendNAK(&packet);
						}
					}						
#endif // #if 0					
				}
#if defined(CONFIG_RTL8186_KB) || defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196C_EC)
				/* Brad add for get hostname of dhcp client */
			if (hostname) {
					bytes = hostname[-1];
					if (bytes >= (int) sizeof(lease->hostname))
						bytes = sizeof(lease->hostname) - 1;
					strncpy(lease->hostname, hostname, bytes);
					lease->hostname[bytes] = '\0';
				} else
				{
					lease->hostname[0] = '\0';
				}
#endif
#if defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196C_EC)
	if(hostname){	
		sprintf(logbuf, "exlog /tmp/log_web.lck /tmp/log_web \"tag:SYSACT;log_num:13;msg:DHCP lease IP %u.%u.%u.%u to %s;note:%02x-%02x-%02x-%02x-%02x-%02x;\""\
		, NIPQUAD(lease->yiaddr), lease->hostname, lease->chaddr[0], lease->chaddr[1], lease->chaddr[2], lease->chaddr[3], lease->chaddr[4], lease->chaddr[5] );
	}else{
		sprintf(logbuf, "exlog /tmp/log_web.lck /tmp/log_web \"tag:SYSACT;log_num:13;msg:DHCP lease IP %u.%u.%u.%u to unknow host;note:%02x-%02x-%02x-%02x-%02x-%02x;\""\
		,NIPQUAD(lease->yiaddr), lease->chaddr[0], lease->chaddr[1], lease->chaddr[2], lease->chaddr[3], lease->chaddr[4], lease->chaddr[5] );
	}
		system(logbuf);
#endif
			/* what to do if we have no record of the client */
			} else if (server_id) {
				/* SELECTING State */
			} 
			else
			{
				if (requested)
				{
					memcpy(&requested_align, requested, 4);
				}
				else if(packet.ciaddr != 0)
				{
					memcpy(&requested_align, &packet.ciaddr, 4);					
				}
				else
				{
					sendNAK(&packet);
					break;
				}
				/* INIT-REBOOT State */
				if (requested_align < server_config.start || 
					   requested_align > server_config.end ||
					   requested_align == server_config.server) 
				{
					sendNAK(&packet);
				}
#if 0	// do we need to check lease table or just ask????
				else if ((lease = find_lease_by_yiaddr(requested_align))) 
				{
					if (lease_expired(lease)) 
					{
						/* probably best if we drop this lease */
						memset(lease->chaddr, 0, 16);
					/* make some contention for this address */
					} 
					else 
					{
					sendNAK(&packet);
					}
				} 
#endif				
				else
				{ /* else remain silent */

					int arpping_time = 0;
					int ret_arpping = 0;
					int retNAK = 1;
					
					char ret_hwaddr[6];
					memset(ret_hwaddr, 0x00, sizeof(ret_hwaddr));
						
					for(arpping_time = 0; arpping_time<1; arpping_time++)
					{						
						if (arpping(requested_align, server_config.server, server_config.arp, server_config.interface, ret_hwaddr) == 0)
						{
							ret_arpping = 1;
							break;
						}					
					}

					if(ret_arpping == 1)
					{						
						if(packet.ciaddr != 0 && memcmp(ret_hwaddr,packet.chaddr, 6) == 0)
						{
							retNAK = 0;
						}
						else
						{
							retNAK = 1;
						}
					}
					else
					{
						retNAK = 0;
					}
					
					// But this ip is not alive in network. we release this ip							
					if ( retNAK == 1)
					{
						sendNAK(&packet);
					}
					else
					{
						if(packet.yiaddr == 0)
						{
							packet.yiaddr = requested_align;
						}
							
						if (!add_lease(packet.chaddr, packet.yiaddr, server_config.offer_time))
						{
								LOG(LOG_WARNING, "lease pool is full -- OFFER abandoned");
								sendNAK(&packet);
						}
						else
						{
							sendACK(&packet, packet.yiaddr);
						}
					}				

					
					
				}

			}
#if 0			
			else 
			{
				 /* RENEWING or REBINDING State */
				 sendNAK(&packet);	// jimmylin 050920 - Fix no response while renewing
			}
#endif			
			break;
		case DHCPDECLINE:
			DEBUG(LOG_INFO,"received DECLINE");
			if (lease) {
				memset(lease->chaddr, 0, 16);
				lease->expires = time(0) + server_config.decline_time;
			}			
			break;
		case DHCPRELEASE:
			DEBUG(LOG_INFO,"received RELEASE");
			if (lease){
				 lease->expires = time(0);
		#ifdef STATIC_LEASE		 
				 if(isStatic_Lease_Entry){
				 //this entry is from static lease host, clear it from lease table
				 	clear_lease(lease->chaddr, lease->yiaddr);
				 	isStatic_Lease_Entry = 0;
				 	
				}
		#endif		
			}
			break;
		case DHCPINFORM:
			DEBUG(LOG_INFO,"received INFORM");
			send_inform(&packet);
			break;	
		default:
			LOG(LOG_WARNING, "unsupported DHCP message (%02x) -- ignoring", state[0]);
		}
	}

	return 0;
}

