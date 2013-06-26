/* dhcpc.c
 *
 * udhcp DHCP client
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
 
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/file.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>
#include <sys/stat.h>
#include "dhcpd.h"
#include "dhcpc.h"
#include "options.h"
#include "clientpacket.h"
#include "packet.h"
#include "script.h"
#include "socket.h"
#include "debug.h"
#include "pidfile.h"
#include <sys/sysinfo.h>  //brad add 20090314

#if defined(UDHCPC_STATIC_FLASH) || defined(UDHCPC_UPDATE_FLASH_ISP)
#include "apmib.h"
#include "mibtbl.h"
#endif
#ifdef UDHCPC_PASS_DOMAINNAME2DHCPSERVER
unsigned char isp_domain_name[80];
#endif
#if defined(CONFIG_RTL865X_KLD)
unsigned char netbios_nodtype=0;
unsigned char netbios_scope[32];
unsigned int wins_1=0;
unsigned int wins_2=0;
#endif
static int state;
static unsigned long requested_ip; /* = 0 */
static unsigned long server_addr;
static unsigned long timeout;
static int packet_num; /* = 0 */
static int fd = -1;
static int signal_pipe[2];
static int isDaemon=0;
#if defined(CONFIG_RTL_ULINKER)
static int dhcpc_retry = 0;
static int detect=0;
#endif
#define LISTEN_NONE 0
#define LISTEN_KERNEL 1
#define LISTEN_RAW 2
static int listen_mode;
#ifdef CHECK_SERVER_ALIVE
#define CHECK_SERVER_ALIVE_BY_CABLE_LINK
#endif
//brad add 20090314
struct sysinfo system_info;
unsigned long dhcpc_seconds;
//brad add end //brad add 20090314
#define DEFAULT_SCRIPT	"/usr/share/udhcpc/default.script"

#ifdef SEND_GRATUITOUS_ARP
#define GRATUITOUS_ARP_NUM 3
#endif

struct client_config_t client_config = {
	/* Default options. */
	abort_if_no_lease: 0,
	foreground: 0,
	quit_after_lease: 0,
	background_if_no_lease: 0,
	interface: "eth0",
	pidfile: NULL,
	script: DEFAULT_SCRIPT,
	clientid: NULL,
	hostname: NULL,
	ifindex: 0,
	arp: "\0\0\0\0\0\0",		/* appease gcc-3.0 */
#ifdef CHECK_SERVER_ALIVE
	alive_time: 0,
	url_name: NULL,
#endif	
#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_KLD)	
	wan_type:0,
	Inform:1,
	broadcast_flag:0,
#endif
};
void create_config_file(void)
{
	unsigned char tmpbuf[100];
	unsigned char tmp[100];
	int fh;
	struct in_addr	intaddr;
	int i;
	fh = open("/var/isp_dhcp.conf", O_RDWR|O_CREAT|O_TRUNC);	
	if (fh < 0) {
		fprintf(stderr, "Create /var/isp_dhcp.conf file error!\n");
		return;
	}
#ifdef UDHCPC_PASS_DOMAINNAME2DHCPSERVER	
	sprintf(tmpbuf, "domain %s %d", isp_domain_name, client_config.wan_type);
	write(fh, tmpbuf, strlen(tmpbuf));
#endif	
#if defined(CONFIG_RTL865X_KLD)
	if(netbios_scope[0]){
		sprintf(tmpbuf, "nbscope=\"%s\"\n", netbios_scope);
		write(fh, tmpbuf, strlen(tmpbuf));
	}
	
	if(wins_1 !=0 ){
		intaddr.s_addr = wins_1;
		sprintf(tmpbuf, "wins=\"%s\"\n", inet_ntoa(intaddr));
		write(fh, tmpbuf, strlen(tmpbuf));
	}
	if(wins_2 !=0 ){
		intaddr.s_addr = wins_2;
		sprintf(tmpbuf, "wins=\"%s\"\n", inet_ntoa(intaddr));
		write(fh, tmpbuf, strlen(tmpbuf));
	}			
	if(netbios_nodtype !=0){
		
		sprintf(tmpbuf, "nbntype=\"%d\"\n", netbios_nodtype);
		write(fh, tmpbuf, strlen(tmpbuf));
	}
#endif

	close(fh);
}

int getUpEvent(char *filename)
{
	struct stat status;
	char buff[100];
	FILE *fp;

	if ( stat(filename, &status) < 0)
		return -1;
	fp = fopen(filename, "r");
	if (!fp) {
        	printf("Can not open file :%s\n", filename);
		return -1;
   	}
	fgets(buff, 100, fp);
	fclose(fp);
	return (atoi(buff));
}
int clearUpEvent(char *filename)
{
	char cmdBuffer[100];
	sprintf(cmdBuffer,"echo 0 > %s",filename);
	system(cmdBuffer);
}

#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_KLD)		
int getPid(char *filename)
{
	struct stat status;
	char buff[100];
	FILE *fp;

	if ( stat(filename, &status) < 0)
		return -1;
	fp = fopen(filename, "r");
	if (!fp) {
        	printf("Can not open file :%s\n", filename);
		return -1;
   	}
	fgets(buff, 100, fp);
	fclose(fp);

	return (atoi(buff));
}

void Inform_DHCPD(void)
{
	char tmpBuf[100];
	int pid;
	memset(tmpBuf, '\0', 100);
	sprintf(tmpBuf, "%s/%s.pid", "/var/run", "udhcpd");
	pid = getPid(tmpBuf);
	if ( pid > 0)
		kill(pid, SIGUSR2);
	usleep(1000);
}
#endif
#if defined(CONFIG_RTL865X_KLD)
/*option 46*/
int OptionNetBios_NodeType(struct dhcpMessage *packet)
{
	uint8_t option_len=0,*temp;
	//unsigned char *ntype;
	if (!(temp = get_option(packet, DHCP_NETBIOS_NODETYPE))) {
		LOG(LOG_DEBUG, "No DHCP ACK with option DHCP_NETBIOS_NODETYPE");
		return 0;
	} else {
		LOG(LOG_DEBUG, "DHCP ACK with option DHCP_NETBIOS_NODETYPE");
		
		option_len = (temp-1)[0];
		netbios_nodtype = (temp-1)[1];
	}
	
}
/*option 47*/
int OptionNetBios_Scope(struct dhcpMessage *packet)
{
	unsigned char *scope;
	uint8_t option_len=0,*temp;
	if (!(temp = get_option(packet, DHCP_NETBIOS_SCOPE))) {
		LOG(LOG_DEBUG, "No DHCP ACK with option DHCP_NETBIOS_SCOPE");
		return 0;
	} else {
		LOG(LOG_DEBUG, "DHCP ACK with option DHCP_NETBIOS_SCOPE");
		
		option_len = (temp-1)[0];
		
		scope = &netbios_scope[0];
		memset(scope, '\0', 32);
		if(option_len >= 1){ //rfc specifies minimun length is 1
			if(option_len > 32){
				strncpy(scope,(&(temp-1)[1]),32);
				scope[32]= '\0'; 
			}else{
				strncpy(scope,&((temp-1)[1]),option_len);
				scope[option_len]= '\0';
			}
		}
		//printf("the netbios scope=%s\n", scope);
	}
}
/*option 44*/
int OptionWINS_Ip(struct dhcpMessage *packet)
{
	unsigned char *scope;
	uint8_t option_len=0,*temp;
	if (!(temp = get_option(packet, DHCP_WINS_SERVER))) {
		LOG(LOG_DEBUG, "No DHCP ACK with option DHCP_WINS_SERVER");
		return 0;
	} else {
		LOG(LOG_DEBUG, "DHCP ACK with option DHCP_WINS_SERVER");

		option_len = (temp-1)[0];
		if((option_len/4)==1){
			((unsigned char *)&wins_1)[0] = ((unsigned char *)&wins_1)[1] = ((unsigned char *)&wins_1)[2] = ((unsigned char *)&wins_1)[3] = 0;
			((unsigned char *)&wins_1)[0] = (temp-1)[1];
			((unsigned char *)&wins_1)[1] = (temp-1)[2];
			((unsigned char *)&wins_1)[2] = (temp-1)[3];
			((unsigned char *)&wins_1)[3] = (temp-1)[4];
		}
		if((option_len/4) > 1){
			((unsigned char *)&wins_2)[0] = ((unsigned char *)&wins_2)[1] = ((unsigned char *)&wins_2)[2] = ((unsigned char *)&wins_2)[3] = 0;
			((unsigned char *)&wins_2)[0] = (temp-1)[5];
			((unsigned char *)&wins_2)[1] = (temp-1)[6];
			((unsigned char *)&wins_2)[2] = (temp-1)[7];
			((unsigned char *)&wins_2)[3] = (temp-1)[8];
		}
	}
}
#endif	
#if defined(CONFIG_RTL8186_TR)
/* option 15 */
int OptionDomainName(struct dhcpMessage *packet)
{
#ifdef UDHCPC_PASS_DOMAINNAME2DHCPSERVER
#define MAX_DHCP_DOMAIN_NAME_LEN 60
	uint8_t option_len=0,*temp;
	unsigned char *domain;
	if (!(temp = get_option(packet, DOMAIN_NAME_ISP))) {
		LOG(LOG_DEBUG, "No DHCP ACK with option DHCPC_DOMAIN_NAME");
		return 0;
	} else {
		LOG(LOG_DEBUG, "DHCP ACK with option DHCPC_DOMAIN_NAME");
		
		option_len = (temp-1)[0];
		domain = &isp_domain_name[0];
		memset(domain, '\0', 80);
				if(option_len >= 1){ //rfc specifies minimun length is 1
				
					if(option_len > MAX_DHCP_DOMAIN_NAME_LEN){
						strncpy(domain,(&(temp-1)[1]),MAX_DHCP_DOMAIN_NAME_LEN);
						domain[MAX_DHCP_DOMAIN_NAME_LEN]= '\0'; 
					}else{
						strncpy(domain,&((temp-1)[1]),option_len);
						domain[option_len]= '\0';
					}
					create_config_file();
#ifdef UDHCPC_UPDATE_FLASH_ISP					
					if ( !apmib_init()) {
						//printf("apmib init fail\n");
					}else{
						if ( apmib_set(MIB_DOMAIN_NAME_ISP, (void *)domain) == 0) {
							LOG(LOG_DEBUG,("\"Add table entry error!\""));
						}
						else
						{
							apmib_update(CURRENT_SETTING);
						}
					}
#endif					
				}else{
					LOG(LOG_ERR,"%s, Invalid option length %u !",__FUNCTION__,option_len);
					return 0;
			}
		}
#endif
	return 1;
}
#endif

#ifdef UDHCPC_STATIC_ROUTE
/* option 33 */
int OptionStaticRoute(struct dhcpMessage *packet)
{
#ifdef UDHCPC_STATIC_ROUTE
#ifndef CONFIG_STATIC_ROUTE_NUMBER
#define CONFIG_STATIC_ROUTE_NUMBER 25
#endif	

#ifndef NVRAM_STATIC_ROUTE_NAME
#define NVRAM_STATIC_ROUTE_NAME "dhcp"
#endif

	int s = 0;
	uint32_t des_ip, gw;
	uint8_t option_len=0,flag_tmp=0,*temp;
	if (!(temp = get_option(packet, DHCP_STATIC_ROUTE))) {
		LOG(LOG_DEBUG, "No DHCP ACK with option DHCP_STATIC_ROUTE");
		return 0;
	} else {
			LOG(LOG_DEBUG, "DHCP ACK with option DHCP_STATIC_ROUTE");
			option_len = (temp-1)[0];

			if((option_len > 0) && (option_len%8) == 0){
				while(flag_tmp < option_len){
					((unsigned char *)&des_ip)[0] = temp[flag_tmp];
					((unsigned char *)&des_ip)[1] = temp[flag_tmp+1];
					((unsigned char *)&des_ip)[2] = temp[flag_tmp+2];
					((unsigned char *)&des_ip)[3] = temp[flag_tmp+3];
					((unsigned char *)&gw)[0] = temp[flag_tmp+4];
					((unsigned char *)&gw)[1] = temp[flag_tmp+5];
					((unsigned char *)&gw)[2] = temp[flag_tmp+6];
					((unsigned char *)&gw)[3] = temp[flag_tmp+7];
					if((((unsigned char *)&des_ip)[0] == 0) && (((unsigned char *)&des_ip)[1] == 0) &&
							(((unsigned char *)&des_ip)[2] == 0) && (((unsigned char *)&des_ip)[3] == 0)){
						LOG(LOG_ERR,"%s,  illegal destion 0.0.0.0 for static routes (option 33)",__FUNCTION__);
						flag_tmp = flag_tmp + 8;
						continue;
					}
				#if 1
				char exebuf[60];		
				sprintf(exebuf,"route add -host %u.%u.%u.%u gw %u.%u.%u.%u metric 1",
							((unsigned char *)&des_ip)[0],((unsigned char *)&des_ip)[1],
							((unsigned char *)&des_ip)[2],((unsigned char *)&des_ip)[3],
							((unsigned char *)&gw)[0],((unsigned char *)&gw)[1],
							((unsigned char *)&gw)[2],((unsigned char *)&gw)[3]);
				system(exebuf);
#ifdef UDHCPC_STATIC_FLASH
{
					STATICROUTE_T entry;
					memset(&entry, '\0', sizeof(entry));
					memcpy(&entry.dstAddr,(unsigned char *)&des_ip,sizeof(des_ip));
					memset(&entry.netmask,0xFF,sizeof(entry.netmask));
					memcpy(&entry.gateway,(unsigned char *)&gw,sizeof(gw));
					entry.interface=2;
					entry.metric=1;
					LOG(LOG_DEBUG, "Adding ENTRY -net %u.%u.%u.%u %u.%u.%u.%u router %u.%u.%u.%u metric 1",
						((unsigned char *)&entry.dstAddr)[0],((unsigned char *)&entry.dstAddr)[1],
						((unsigned char *)&entry.dstAddr)[2],((unsigned char *)&entry.dstAddr)[3],
						((unsigned char *)&entry.netmask)[0],((unsigned char *)&entry.netmask)[1],
						((unsigned char *)&entry.netmask)[2],((unsigned char *)&entry.netmask)[3],
						((unsigned char *)&entry.gateway)[0],((unsigned char *)&entry.gateway)[1],
						((unsigned char *)&entry.gateway)[2],((unsigned char *)&entry.gateway)[3]);
					// set to MIB. try to delete it first to avoid duplicate case
					apmib_init();
					apmib_set(MIB_STATICROUTE_DEL, (void *)&entry);
					if ( apmib_set(MIB_STATICROUTE_ADD, (void *)&entry) == 0) {
						LOG(LOG_DEBUG,("\"Add table entry error!\""));
					}
					else
					{
						apmib_update(CURRENT_SETTING);
					}


				}
#endif
				#endif
     

					flag_tmp = flag_tmp + 8;
				}
			}else{
				LOG(LOG_ERR,"%s, Incorrect option length %u for dhcp option 33 (static route)",__FUNCTION__,option_len);
				return 0;
			}
		}
#endif
	return 1;
	}
#endif
#ifdef RFC3442

#ifdef UDHCPC_RFC_CLASSLESS_STATIC_ROUTE
/* option 121 */
int OptionClasslessStaticRoute(struct dhcpMessage *packet)
{
	/*
Note:
	- Vista dhcp client will send 121 and 249, and if dhcp server response 
		with 121, vista will only accept 121, ignore 249
	- We don't support router address is 0.0.0.0, refer to rfc 3442, page 5
	- if there are "static_routing" option in nvram, i will
		- remove old records
		- then find empty entry to record new records
		- maximun numbers of records are defined as CONFIG_STATIC_ROUTE_NUMBER
		- the name in each entry list are definded as NVRAM_STATIC_ROUTE_NAME
			i will take it as the condition to recognize the records added by me 
			to remove/add to nvram
	 */
#ifdef UDHCPC_RFC_CLASSLESS_STATIC_ROUTE

#ifndef CONFIG_STATIC_ROUTE_NUMBER
#define CONFIG_STATIC_ROUTE_NUMBER 25
#endif
#ifndef NVRAM_STATIC_ROUTE_NAME
#define NVRAM_STATIC_ROUTE_NAME "dhcp"
#endif

	int s = 0;
	uint32_t des_ip, netmask, gw;
	uint8_t option_len=0,mask_num=0,flag_tmp=0,*temp;

	if (!(temp = get_option(packet, RFC_CLASSLESS_STATIC_ROUTE))) {
		LOG(LOG_DEBUG, "No DHCP ACK with option DHCPC_CLASSLESS_STATIC_ROUTE");
		return 0;
	} else {
		LOG(LOG_DEBUG, "DHCP ACK with option DHCPC_CLASSLESS_STATIC_ROUTE");
	
		option_len = (temp-1)[0];

		if(option_len >= 5){ //rfc specifies minimun length is 5
			while(flag_tmp < option_len){
				mask_num = temp[flag_tmp];
				if(mask_num == 0){
					/* netmask is 0.0.0.0, default gw */
					((unsigned char *)&des_ip)[0] = ((unsigned char *)&des_ip)[1] = ((unsigned char *)&des_ip)[2] = ((unsigned char *)&des_ip)[3] = 0;
					((unsigned char *)&gw)[0] = temp[flag_tmp+1];
					((unsigned char *)&gw)[1] = temp[flag_tmp+2];
					((unsigned char *)&gw)[2] = temp[flag_tmp+3];
					((unsigned char *)&gw)[3] = temp[flag_tmp+4];
					flag_tmp = flag_tmp+5;
				}else if( 1 <= mask_num && mask_num <= 8 ){
					((unsigned char *)&des_ip)[0] = temp[flag_tmp+1];
					((unsigned char *)&des_ip)[1] = ((unsigned char *)&des_ip)[2] = ((unsigned char *)&des_ip)[3] = 0;
					((unsigned char *)&gw)[0] = temp[flag_tmp+2];
					((unsigned char *)&gw)[1] = temp[flag_tmp+3];
					((unsigned char *)&gw)[2] = temp[flag_tmp+4];
					((unsigned char *)&gw)[3] = temp[flag_tmp+5];
					flag_tmp = flag_tmp+6;
				}else if( 9 <= mask_num && mask_num <= 16 ){
					((unsigned char *)&des_ip)[0] = temp[flag_tmp+1];
					((unsigned char *)&des_ip)[1] = temp[flag_tmp+2];
					((unsigned char *)&des_ip)[2] = ((unsigned char *)&des_ip)[3] = 0;
					((unsigned char *)&gw)[0] = temp[flag_tmp+3];
					((unsigned char *)&gw)[1] = temp[flag_tmp+4];
					((unsigned char *)&gw)[2] = temp[flag_tmp+5];
					((unsigned char *)&gw)[3] = temp[flag_tmp+6];
					flag_tmp = flag_tmp+7;
				}else if( 17 <= mask_num && mask_num <= 24 ){
					((unsigned char *)&des_ip)[0] = temp[flag_tmp+1];
					((unsigned char *)&des_ip)[1] = temp[flag_tmp+2];
					((unsigned char *)&des_ip)[2] = temp[flag_tmp+3];
					((unsigned char *)&des_ip)[3] = 0;
					((unsigned char *)&gw)[0] = temp[flag_tmp+4];
					((unsigned char *)&gw)[1] = temp[flag_tmp+5];
					((unsigned char *)&gw)[2] = temp[flag_tmp+6];
					((unsigned char *)&gw)[3] = temp[flag_tmp+7];
					flag_tmp = flag_tmp+8;
				}else if( 25 <= mask_num && mask_num <= 32 ){
					((unsigned char *)&des_ip)[0] = temp[flag_tmp+1];
					((unsigned char *)&des_ip)[1] = temp[flag_tmp+2];
					((unsigned char *)&des_ip)[2] = temp[flag_tmp+3];
					((unsigned char *)&des_ip)[3] = temp[flag_tmp+4];
					((unsigned char *)&gw)[0] = temp[flag_tmp+5];
					((unsigned char *)&gw)[1] = temp[flag_tmp+6];
					((unsigned char *)&gw)[2] = temp[flag_tmp+7];
					((unsigned char *)&gw)[3] = temp[flag_tmp+8];
					flag_tmp = flag_tmp+9;
				}else{
					//error , should not go through here !!!
					LOG(LOG_ERR,"%s, Unknown mask num %u !",__FUNCTION__,mask_num);
		continue;
	    }

		        memset(&netmask,0,sizeof(uint32_t));
		        for(s=0;s<mask_num;s++){
                	netmask = netmask | (1<<(31-s));
	    }

				LOG(LOG_DEBUG, "Adding DHCP_STATIC_ROUTE -net %u.%u.%u.%u netmask[%u] %u.%u.%u.%u router %u.%u.%u.%u metric 1",
					((unsigned char *)&des_ip)[0],((unsigned char *)&des_ip)[1],
					((unsigned char *)&des_ip)[2],((unsigned char *)&des_ip)[3],
					mask_num,
					((unsigned char *)&netmask)[0],((unsigned char *)&netmask)[1],
					((unsigned char *)&netmask)[2],((unsigned char *)&netmask)[3],
					((unsigned char *)&gw)[0],((unsigned char *)&gw)[1],
					((unsigned char *)&gw)[2],((unsigned char *)&gw)[3]);
				#if 1
				char exebuf[60];		
				sprintf(exebuf,"route add -net %u.%u.%u.%u netmask %u.%u.%u.%u gw %u.%u.%u.%u metric 1",
						((unsigned char *)&des_ip)[0],((unsigned char *)&des_ip)[1],
						((unsigned char *)&des_ip)[2],((unsigned char *)&des_ip)[3],
						((unsigned char *)&netmask)[0],((unsigned char *)&netmask)[1],
						((unsigned char *)&netmask)[2],((unsigned char *)&netmask)[3],
						((unsigned char *)&gw)[0],((unsigned char *)&gw)[1],
						((unsigned char *)&gw)[2],((unsigned char *)&gw)[3]);
				system(exebuf);
#ifdef UDHCPC_STATIC_FLASH
				{
					STATICROUTE_T entry;

					memset(&entry, '\0', sizeof(entry));
					memcpy(&entry.dstAddr,(unsigned char *)&des_ip,sizeof(des_ip));
					memcpy(&entry.netmask,(unsigned char *)&netmask,sizeof(netmask));
					memcpy(&entry.gateway,(unsigned char *)&gw,sizeof(gw));
					entry.interface=2;
					entry.metric=1;
					LOG(LOG_DEBUG, "Adding ENTRY -net %u.%u.%u.%u netmask[%u] %u.%u.%u.%u router %u.%u.%u.%u",
						((unsigned char *)&entry.dstAddr)[0],((unsigned char *)&entry.dstAddr)[1],
						((unsigned char *)&entry.dstAddr)[2],((unsigned char *)&entry.dstAddr)[3],
						mask_num,
						((unsigned char *)&entry.netmask)[0],((unsigned char *)&entry.netmask)[1],
						((unsigned char *)&entry.netmask)[2],((unsigned char *)&entry.netmask)[3],
						((unsigned char *)&entry.gateway)[0],((unsigned char *)&entry.gateway)[1],
						((unsigned char *)&entry.gateway)[2],((unsigned char *)&entry.gateway)[3]);

					apmib_set(MIB_STATICROUTE_DEL, (void *)&entry);
					if ( apmib_set(MIB_STATICROUTE_ADD, (void *)&entry) == 0) {
						LOG(LOG_DEBUG,("\"Add table entry error!\""));
					}
					else
					{
						apmib_update(CURRENT_SETTING);
	}

				
			}
#endif
				#endif
			}
	} else {
			LOG(LOG_ERR,"%s, Invalid option length %u !",__FUNCTION__,option_len);
			return 0;
		}
	    }
#endif
	return 1;
	}
#endif
#ifdef UDHCPC_MS_CLASSLESS_STATIC_ROUTE
int OptionMicroSoftClasslessStaticRoute(struct dhcpMessage *packet)
{
	/*
Note:
	- Vista dhcp client will send 121 and 249, and if dhcp server response 
		with 121, vista will only accept 121, ignore 249
	- We don't support router address is 0.0.0.0, refer to rfc 3442, page 5
	 */
#ifdef UDHCPC_MS_CLASSLESS_STATIC_ROUTE
#ifndef CONFIG_STATIC_ROUTE_NUMBER
#define CONFIG_STATIC_ROUTE_NUMBER 25
#endif
#ifndef NVRAM_STATIC_ROUTE_NAME
#define NVRAM_STATIC_ROUTE_NAME "dhcp"
#endif

	int s = 0;
	uint32_t des_ip, netmask, gw;
	uint8_t option_len=0,mask_num=0,flag_tmp=0,*temp;
	
	if (!(temp = get_option(packet, MS_CLASSLESS_STATIC_ROUTE))) {
		LOG(LOG_DEBUG, "No DHCP ACK with MS_DHCP_STATIC_ROUTE (option 249)");
		return 0;
	} else {
		LOG(LOG_DEBUG, "DHCP ACK with MS_DHCP_STATIC_ROUTE (option 249)");
		option_len = (temp-1)[0];
		
		if(option_len >= 5){ //rfc specifies minimun length is 5
			while(flag_tmp < option_len){
				mask_num = temp[flag_tmp];
				if(mask_num == 0){
					/* netmask is 0.0.0.0, default gw */
					((unsigned char *)&des_ip)[0] = ((unsigned char *)&des_ip)[1] = ((unsigned char *)&des_ip)[2] = ((unsigned char *)&des_ip)[3] = 0;
					((unsigned char *)&gw)[0] = temp[flag_tmp+1];
					((unsigned char *)&gw)[1] = temp[flag_tmp+2];
					((unsigned char *)&gw)[2] = temp[flag_tmp+3];
					((unsigned char *)&gw)[3] = temp[flag_tmp+4];
					flag_tmp = flag_tmp+5;
				}else if( 1 <= mask_num && mask_num <= 8 ){
					((unsigned char *)&des_ip)[0] = temp[flag_tmp+1];
					((unsigned char *)&des_ip)[1] = ((unsigned char *)&des_ip)[2] = ((unsigned char *)&des_ip)[3] = 0;
					((unsigned char *)&gw)[0] = temp[flag_tmp+2];
					((unsigned char *)&gw)[1] = temp[flag_tmp+3];
					((unsigned char *)&gw)[2] = temp[flag_tmp+4];
					((unsigned char *)&gw)[3] = temp[flag_tmp+5];
					flag_tmp = flag_tmp+6;
				}else if( 9 <= mask_num && mask_num <= 16 ){
					((unsigned char *)&des_ip)[0] = temp[flag_tmp+1];
					((unsigned char *)&des_ip)[1] = temp[flag_tmp+2];
					((unsigned char *)&des_ip)[2] = (&des_ip)[3] = 0;
					((unsigned char *)&gw)[0] = temp[flag_tmp+3];
					((unsigned char *)&gw)[1] = temp[flag_tmp+4];
					((unsigned char *)&gw)[2] = temp[flag_tmp+5];
					((unsigned char *)&gw)[3] = temp[flag_tmp+6];
					flag_tmp = flag_tmp+7;
				}else if( 17 <= mask_num && mask_num <= 24 ){
					((unsigned char *)&des_ip)[0] = temp[flag_tmp+1];
					((unsigned char *)&des_ip)[1] = temp[flag_tmp+2];
					((unsigned char *)&des_ip)[2] = temp[flag_tmp+3];
					((unsigned char *)&des_ip)[3] = 0;
					((unsigned char *)&gw)[0] = temp[flag_tmp+4];
					((unsigned char *)&gw)[1] = temp[flag_tmp+5];
					((unsigned char *)&gw)[2] = temp[flag_tmp+6];
					((unsigned char *)&gw)[3] = temp[flag_tmp+7];
					flag_tmp = flag_tmp+8;
				}else if( 25 <= mask_num && mask_num <= 32 ){
					((unsigned char *)&des_ip)[0] = temp[flag_tmp+1];
					((unsigned char *)&des_ip)[1] = temp[flag_tmp+2];
					((unsigned char *)&des_ip)[2] = temp[flag_tmp+3];
					((unsigned char *)&des_ip)[3] = temp[flag_tmp+4];
					((unsigned char *)&gw)[0] = temp[flag_tmp+5];
					((unsigned char *)&gw)[1] = temp[flag_tmp+6];
					((unsigned char *)&gw)[2] = temp[flag_tmp+7];
					((unsigned char *)&gw)[3] = temp[flag_tmp+8];
					flag_tmp = flag_tmp+9;
				}else{
					//error , should not go through here !!!
					LOG(LOG_ERR,"%s, Unknown mask num %u !",__FUNCTION__,mask_num);
	    continue;
	}

		        memset(&netmask,0,sizeof(uint32_t));
		        for(s=0;s<mask_num;s++){
                	netmask = netmask | (1<<(31-s));
		        }
				LOG(LOG_DEBUG, "Adding MS_DHCP_STATIC_ROUTE -net %u.%u.%u.%u netmask[%u] %u.%u.%u.%u router %u.%u.%u.%u",
					((unsigned char *)&des_ip)[0],((unsigned char *)&des_ip)[1],
					((unsigned char *)&des_ip)[2],((unsigned char *)&des_ip)[3],
					mask_num,
					((unsigned char *)&netmask)[0],((unsigned char *)&netmask)[1],
					((unsigned char *)&netmask)[2],((unsigned char *)&netmask)[3],
					((unsigned char *)&gw)[0],((unsigned char *)&gw)[1],
					((unsigned char *)&gw)[2],((unsigned char *)&gw)[3]);
			#if 1
			char exebuf[60];		
			sprintf(exebuf,"route add -net %u.%u.%u.%u netmask %u.%u.%u.%u gw %u.%u.%u.%u metric 1",
				((unsigned char *)&des_ip)[0],((unsigned char *)&des_ip)[1],
				((unsigned char *)&des_ip)[2],((unsigned char *)&des_ip)[3],
				((unsigned char *)&netmask)[0],((unsigned char *)&netmask)[1],
				((unsigned char *)&netmask)[2],((unsigned char *)&netmask)[3],
				((unsigned char *)&gw)[0],((unsigned char *)&gw)[1],
				((unsigned char *)&gw)[2],((unsigned char *)&gw)[3]);
			system(exebuf);
#ifdef UDHCPC_STATIC_FLASH
			{
				STATICROUTE_T entry;
				memset(&entry, '\0', sizeof(entry));
				memcpy(&entry.dstAddr,(unsigned char *)&des_ip,sizeof(des_ip));
				memcpy(&entry.netmask,(unsigned char *)&netmask,sizeof(netmask));
				memcpy(&entry.gateway,(unsigned char *)&gw,sizeof(gw));
				entry.interface=1;
				entry.metric=1;
				LOG(LOG_DEBUG, "Adding ENTRY -net %u.%u.%u.%u netmask[%u] %u.%u.%u.%u router %u.%u.%u.%u metric 1",
					((unsigned char *)&entry.dstAddr)[0],((unsigned char *)&entry.dstAddr)[1],
					((unsigned char *)&entry.dstAddr)[2],((unsigned char *)&entry.dstAddr)[3],
					mask_num,
					((unsigned char *)&entry.netmask)[0],((unsigned char *)&entry.netmask)[1],
					((unsigned char *)&entry.netmask)[2],((unsigned char *)&entry.netmask)[3],
					((unsigned char *)&entry.gateway)[0],((unsigned char *)&entry.gateway)[1],
					((unsigned char *)&entry.gateway)[2],((unsigned char *)&entry.gateway)[3]);
				// set to MIB. try to delete it first to avoid duplicate case
				apmib_set(MIB_STATICROUTE_DEL, (void *)&entry);
				if ( apmib_set(MIB_STATICROUTE_ADD, (void *)&entry) == 0) {
					LOG(LOG_DEBUG,("\"Add table entry error!\""));
				}
				else
				{
					apmib_update(CURRENT_SETTING);
				}				

			}
#endif
			#endif
    }

	   
		}else{
			LOG(LOG_ERR,"%s, Invalid option length %u !",__FUNCTION__,option_len);
			return 0;
	}
    }
#endif
    return 1;
}
#endif
#endif
#ifndef IN_BUSYBOX
static void __attribute__ ((noreturn)) show_usage(void)
{
	printf(
"Usage: udhcpc [OPTIONS]\n\n"
"  -c, --clientid=CLIENTID         Client identifier\n"
"  -H, --hostname=HOSTNAME         Client hostname\n"
"  -h                              Alias for -H\n"
"  -f, --foreground                Do not fork after getting lease\n"
"  -b, --background                Fork to background if lease cannot be\n"
"                                  immediately negotiated.\n"
"  -i, --interface=INTERFACE       Interface to use (default: eth0)\n"
"  -n, --now                       Exit with failure if lease cannot be\n"
"                                  immediately negotiated.\n"
"  -p, --pidfile=file              Store process ID of daemon in file\n"
"  -q, --quit                      Quit after obtaining lease\n"
"  -r, --request=IP                IP address to request (default: none)\n"
"  -s, --script=file               Run file at dhcp events (default:\n"
"                                  " DEFAULT_SCRIPT ")\n"
"  -v, --version                   Display version\n"
#ifdef CHECK_SERVER_ALIVE
"  -a, --alive                      Check DHCP server alive periodically\n"
"  -u, --url                      URL address referred when check DHCP server alive\n"
#endif	
	);
	exit(0);
}
#else
extern void show_usage(void) __attribute__ ((noreturn));
#endif

//#ifdef CONFIG_RTL865X_AC /* In order to accept empty character in HostName. Keith */		
#if defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD)
#define MAXWORDLEN 256
int getword(FILE *f, char *word)
{
    int c, len, escape;
    int quoted, comment;
    int value, got;

    len = 0;
    escape = 0;
    comment = 0;

    /*
     * First skip white-space and comments.
     */
     
    for (;;) {
	c = getc(f);
	if (c == EOF)
	    break;

	/*
	 * A newline means the end of a comment; backslash-newline
	 * is ignored.  Note that we cannot have escape && comment.
	 */
	if (c == '\n') {
	    if (!escape) {
		comment = 0;
	    } else
		escape = 0;
	    continue;
	}

	/*
	 * Ignore characters other than newline in a comment.
	 */
	if (comment)
	    continue;

	/*
	 * If this character is escaped, we have a word start.
	 */
	if (escape)
	    break;

	/*
	 * If this is the escape character, look at the next character.
	 */
	if (c == '\\') {
	    escape = 1;
	    continue;
	}

	/*
	 * If this is the start of a comment, ignore the rest of the line.
	 */
	//if (c == '#') {
	//    comment = 1;
	//    continue;
	//}
	/*
	 * A non-whitespace character is the start of a word.
	 */
	if (!isspace(c))
	    break;
	
    }

    /*
     * Save the delimiter for quoted strings.
     */
    if (!escape && (c == '"' || c == '\'')) {
        quoted = c;
	c = getc(f);
    } else
        quoted = 0;

    /*
     * Process characters until the end of the word.
     */
    while (c != EOF) {
	if (escape) {
	    /*
	     * This character is escaped: backslash-newline is ignored,
	     * various other characters indicate particular values
	     * as for C backslash-escapes.
	     */
	    escape = 0;
	    if (c == '\n') {
	        c = getc(f);
		continue;
	    }

	    got = 0;
	    switch (c) {
	    case 'a':
		value = '\a';
		break;
	    case 'b':
		value = '\b';
		break;
	    case 'f':
		value = '\f';
		break;
	    case 'n':
		value = '\n';
		break;
	    case 'r':
		value = '\r';
		break;
	    case 's':
		value = ' ';
		break;
	    case 't':
		value = '\t';
		break;

	    default:
		/*
		 * Otherwise the character stands for itself.
		 */
		value = c;
		break;
	    }

	    /*
	     * Store the resulting character for the escape sequence.
	     */
	    if (len < MAXWORDLEN-1)
		word[len] = value;
	    ++len;

	    if (!got)
		c = getc(f);
	    continue;

	}

	/*
	 * Not escaped: see if we've reached the end of the word.
	 */
	if (quoted) {
	    if (c == quoted)
		break;
	} else {
	    //if (isspace(c) || c == '#') {
	    if (isspace(c)) {	
		ungetc (c, f);
		break;
	    }
	}

	/*
	 * Backslash starts an escape sequence.
	 */
	if (c == '\\') {
	    escape = 1;
	    c = getc(f);
	    continue;
	}

	/*
	 * An ordinary character: store it in the word and get another.
	 */
	if (len < MAXWORDLEN-1)
	    word[len] = c;
	++len;

	c = getc(f);
    }

    /*
     * End of the word: check for errors.
     */
    if (c == EOF) {
	if (ferror(f)) {
	    if (errno == 0)
		errno = EIO;
	   
	}
	/*
	 * If len is zero, then we didn't find a word before the
	 * end of the file.
	 */
	if (len == 0)
	    return 0;
    }

    
    word[len] = 0;


    return 1;



}

#endif

#if defined(CONFIG_RTL_ULINKER)
/* option 6 */
int OptionDNSServer(struct dhcpMessage *packet)
{
	uint8_t option_len=0,*temp;
	uint8_t dns_ip[4];
	char buf[48];
	if (!(temp = get_option(packet, DHCP_DNS_SERVER))) {
		LOG(LOG_DEBUG, "No DHCP ACK with option Domain Name Server");
		system("echo 168.95.1.1 > /var/ulinker_dns");
		return 0;
	} else {
		LOG(LOG_DEBUG, "DHCP ACK with option Domain Name Server");
		memcpy(dns_ip, temp, 4);
		sprintf(buf, "echo %u.%u.%u.%u > /var/ulinker_dns", dns_ip[0], dns_ip[1], dns_ip[2], dns_ip[3]);
		system(buf);
	}

	return 1;
}
#endif /* #if defined(CONFIG_RTL_ULINKER) */

/* just a little helper */
static void change_mode(int new_mode)
{

	static int restore_fast_nat=0;
		
	DEBUG(LOG_INFO, "entering %s listen mode",
		new_mode ? (new_mode == 1 ? "kernel" : "raw") : "none");
	if (fd >= 0) close(fd);
	fd = -1;
	listen_mode = new_mode;

// Following code is a workaround to fix fast-nat bug
	if (listen_mode == LISTEN_RAW) {
		FILE *fp=NULL;	
		int i=0;
		fp=fopen("/proc/fast_nat","r");	
		if(fp != NULL){
			fscanf(fp,"%d",&i);	
			fclose(fp); 
		}
			if (i && i < 10) {
				system("echo 0 > /proc/fast_nat");			
				restore_fast_nat=1;
			}
		}
	else {
		if (restore_fast_nat) {
			system("echo 1 > /proc/fast_nat");
			restore_fast_nat=0;
		}
	}
//-------------------------- david+2007-05-03
	
}


/* perform a renew */
static void perform_renew(void)
{
#if defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD)
	char logbuf[300];
#endif
	LOG(LOG_INFO, "Performing a DHCP renew");
#if defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD)
		sprintf(logbuf, "exlog /tmp/log_web.lck /tmp/log_web \"tag:SYSACT;log_num:13;msg:DHCP renew;\"");
		system(logbuf);
#endif	
	switch (state) {
	case BOUND:
		change_mode(LISTEN_KERNEL);
	case RENEWING:
	case REBINDING:
		state = RENEW_REQUESTED;
		break;
	case RENEW_REQUESTED: /* impatient are we? fine, square 1 */
		run_script(NULL, "deconfig");
	case REQUESTING:
	case RELEASED:
		change_mode(LISTEN_RAW);
		state = INIT_SELECTING;
		break;
	case INIT_SELECTING:
		break;
	}

	/* start things over */
	packet_num = 0;

	/* Kill any timeouts because the user wants this to hurry along */
	timeout = 0;
}


/* perform a release */
static void perform_release(void)
{
	char buffer[16];
	struct in_addr temp_addr;
#if defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD)
	char logbuf[300];
#endif
	/* send release packet */
	if (state == BOUND || state == RENEWING || state == REBINDING) {
		temp_addr.s_addr = server_addr;
		sprintf(buffer, "%s", inet_ntoa(temp_addr));
		temp_addr.s_addr = requested_ip;
		LOG(LOG_INFO, "Unicasting a release of %s to %s", 
				inet_ntoa(temp_addr), buffer);
		send_release(server_addr, requested_ip); /* unicast */
		run_script(NULL, "deconfig");
	}
	LOG(LOG_INFO, "Entering released state");
#if defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD)
				sprintf(logbuf, "exlog /tmp/log_web.lck /tmp/log_web \"tag:SYSACT;log_num:13;msg:DHCP release;\"");
				system(logbuf);
#endif
#if defined(CONFIG_RTL865X_KLD)
	system("rm -f /var/isp_dhcp.conf 2> /dev/null");
	system("rm -f /var/eth1_ip 2> /dev/null");
#endif

	change_mode(LISTEN_NONE);
	state = RELEASED;
	timeout = 0x7fffffff;
}


#ifdef CHECK_SERVER_ALIVE
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PING_TIMEOUT		3	//in sec
#define PING_MAX_RETRY		4	

static int create_icmp_socket(void)
{
	struct protoent *proto;
	int sock;

	proto = getprotobyname("icmp");
	/* if getprotobyname failed, just silently force
	 * proto->p_proto to have the correct value for "icmp" */
	if ((sock = socket(AF_INET, SOCK_RAW,
			(proto ? proto->p_proto : 1))) < 0) {        /* 1 == ICMP */
		printf("Create ICMP socket failed!\n");
		return -1;
	}
	return sock;
}

static int in_cksum(unsigned short *buf, int sz)
{
	int nleft = sz;
	int sum = 0;
	unsigned short *w = buf;
	unsigned short ans = 0;

	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}

	if (nleft == 1) {
		*(unsigned char *) (&ans) = *(unsigned char *) w;
		sum += ans;
	}

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	ans = ~sum;
	return (ans);
}

static int ping(const char *host)
{
	struct hostent *h;
	struct sockaddr_in pingaddr;
	struct icmp *pkt;
	int pingsock, c;
	char packet[100];
	fd_set netFD;	
	int selret, max_sock, is_ok=0, retry=0;

	pingsock = create_icmp_socket();
	if (pingsock <= 0) {
		printf("ping failed (1)!\n");
		goto ret;
	}

	memset(&pingaddr, 0, sizeof(struct sockaddr_in));
	pingaddr.sin_family = AF_INET;
	h = gethostbyname(host);
	if (h == NULL) {
		printf("ping failed (2)!\n");
		goto ret;
	}	
	memcpy(&pingaddr.sin_addr, h->h_addr, sizeof(pingaddr.sin_addr));
//	hostname = h->h_name;

	pkt = (struct icmp *)packet;
	memset(pkt, 0, sizeof(packet));
	pkt->icmp_type = ICMP_ECHO;
	pkt->icmp_cksum = in_cksum((unsigned short *) pkt, sizeof(packet));

	while (!is_ok && retry++ < PING_MAX_RETRY) {
		c = sendto(pingsock, packet, sizeof(packet), 0,
				   (struct sockaddr *) &pingaddr, sizeof(struct sockaddr_in));

		if (c < 0 || c != sizeof(packet)) {
			printf("ping failed (3)");
			break;
		}

		/* listen for replies */
		while (1) {	
			struct timeval timeout;	
			timeout.tv_sec = PING_TIMEOUT;
			timeout.tv_usec = 0;
			max_sock = 0;
			FD_ZERO(&netFD);
			FD_SET(pingsock, &netFD);
			max_sock = (max_sock > pingsock) ? max_sock : pingsock;
			selret = select(max_sock+1, &netFD, NULL, NULL, &timeout);
			if (selret && FD_ISSET(pingsock, &netFD)) {
				struct sockaddr_in from;
				size_t fromlen = sizeof(from);
				if ((c = recvfrom(pingsock, packet, sizeof(packet), 0,
						  (struct sockaddr *) &from, &fromlen)) < 0) {
					if (errno == EINTR)
						continue;
					printf("recvfrom() return error!");
					continue;
				}			
				if (c >= 76) {	/* ip + icmp */
					struct iphdr *iphdr = (struct iphdr *) packet;
					pkt = (struct icmp *) (packet + (iphdr->ihl << 2));	/* skip ip hdr */
					if (pkt->icmp_type == ICMP_ECHOREPLY) {
						is_ok = 1;
						break;
					}
				}
			}

			break;
		}		
	}
ret:	
	if (pingsock > 0)
		close(pingsock);

	return is_ok;	
}
#if defined(CHECK_SERVER_ALIVE_BY_CABLE_LINK)
#define RTL8651_IOCTL_GETWANLINKSTATUS 2000
static int re865xIoctl(char *name, unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3)
{
  unsigned int args[4];
  struct ifreq ifr;
  int sockfd;
	unsigned int *p=arg3;
  args[0] = arg0;
  args[1] = arg1;
  args[2] = arg2;
  args[3] = arg3;

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      perror("fatal error socket\n");
      return -3;
    }
  
  strcpy((char*)&ifr.ifr_name, name);
  ((unsigned int *)(&ifr.ifr_data))[0] = (unsigned int)args;

  if (ioctl(sockfd, SIOCDEVPRIVATE, &ifr)<0)
    {
      perror("device ioctl:");
      close(sockfd);
      return -1;
    }
  close(sockfd);
  return 0;
} 

int getWanLink(char *interface)
{
	int    ret=-1;
	int    args[0];

        re865xIoctl(interface, RTL8651_IOCTL_GETWANLINKSTATUS, (unsigned int)(args), 0, (unsigned int)&ret) ;
       return ret;
}
#endif
static int is_server_alive(void)
{
	char buffer[32];
	struct in_addr temp_addr;
	int ret=0;
#if !defined(CHECK_SERVER_ALIVE_BY_CABLE_LINK)
	temp_addr.s_addr = server_addr;
	sprintf(buffer, "%s", inet_ntoa(temp_addr));
	if (!ping(buffer)) {		
	#if CONFIG_RTL8186_KB
		if (client_config.url_name==NULL ||(client_config.url_name && !ping(client_config.url_name))) 
	#endif	
			return 0;		
	}
#else
	if(!strcmp(client_config.interface, "eth1")){
		if (getWanLink("eth1") < 0){ 
			return 0;
		}else{
			return 1;
		}
	}
#endif
	return 1;
}
#endif // CHECK_SERVER_ALIVE


/* Exit and cleanup */
static void exit_client(int retval)
{
	pidfile_delete(client_config.pidfile);
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


static void background(void)
{
	int pid_fd;

	pid_fd = pidfile_acquire(client_config.pidfile); /* hold lock during fork. */
	while (pid_fd >= 0 && pid_fd < 3) pid_fd = dup(pid_fd); /* don't let daemon close it */
	if (daemon(0, 0) == -1) {
		perror("fork");
		exit_client(1);
	}
	client_config.foreground = 1; /* Do not fork again. */
	client_config.background_if_no_lease = 0;
	pidfile_write_release(pid_fd);
}


#ifdef COMBINED_BINARY
int udhcpc_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	unsigned char *temp, *message;
	unsigned long t1 = 0, t2 = 0, xid = 0;
	unsigned long start = 0, lease;
	fd_set rfds;
	int retval;
	struct timeval tv;
	int c, len;
	struct dhcpMessage packet;
	struct in_addr temp_addr;
	int pid_fd;
	time_t now;
	int max_fd;
	int sig;
	char filename[64];
#if defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD)
	char logbuf[300];
	char cmdBuf[60];
#endif
#ifdef CHECK_SERVER_ALIVE
	int lease_expired_left=0;
#endif
//#ifdef CONFIG_RTL865X_AC /* In order to accept empty character in HostName. Keith */	
#if defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD)
	FILE *fp_hostname;
	char buf_hostname[300];
#endif	
#ifdef SEND_GRATUITOUS_ARP
	int k;
#endif
	static struct option arg_options[] = {
		{"clientid",	required_argument,	0, 'c'},
		{"foreground",	no_argument,		0, 'f'},
		{"background",	no_argument,		0, 'b'},
		{"hostname",	required_argument,	0, 'H'},
		{"hostname",    required_argument,      0, 'h'},
		{"interface",	required_argument,	0, 'i'},
		{"now", 	no_argument,		0, 'n'},
		{"pidfile",	required_argument,	0, 'p'},
		{"quit",	no_argument,		0, 'q'},
		{"request",	required_argument,	0, 'r'},
		{"script",	required_argument,	0, 's'},
#ifdef CHECK_SERVER_ALIVE		
		{"alive",	required_argument,	0, 'a'},
		{"url",		required_argument,	0, 'u'},
#endif
#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL865X_AC)
		{"wantype",	required_argument,	0, 'w'},
		{"Inform",	required_argument,	0, 'I'},
		{"broadcast_flag",	no_argument,0, 'B'},
#endif
		{"daemon",	no_argument,		0, 'x'},
#if defined(CONFIG_RTL_ULINKER)
		{"detect",	required_argument,		0, 'd'},
#endif
		{"version",	no_argument,		0, 'v'},
		{"help",	no_argument,		0, '?'},
		{0, 0, 0, 0}
	};

#if defined(CONFIG_RTL_ULINKER)
	if (detect)
		system("echo \"0\" > /var/ulinker_dhcps_discover_flag");
	else
		system("echo \"0\" > /var/ulinker_auto_dhcp");
#endif

	/* get options */
	while (1) {
		int option_index = 0;
#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL865X_AC)	
		c = getopt_long(argc, argv, "c:fbH:h:i:np:qr:s:a:u:w:I:Bv", arg_options, &option_index);
#else

#if defined(CHECK_SERVER_ALIVE) && defined(CONFIG_RTL_ULINKER)
	c = getopt_long(argc, argv, "c:fbH:h:i:np:qr:s:a:u:xd:v", arg_options, &option_index);
#elif defined(CHECK_SERVER_ALIVE)
	c = getopt_long(argc, argv, "c:fbH:h:i:np:qr:s:a:u:xv", arg_options, &option_index);
#elif defined(CONFIG_RTL_ULINKER)
	c = getopt_long(argc, argv, "c:fbH:h:i:np:qr:s:xgv", arg_options, &option_index);
#else
		c = getopt_long(argc, argv, "c:fbH:h:i:np:qr:s:xv", arg_options, &option_index);
#endif
#endif
		if (c == -1) break;
		
		switch (c) {
		case 'c':
			len = strlen(optarg) > 255 ? 255 : strlen(optarg);
			if (client_config.clientid) free(client_config.clientid);
			client_config.clientid = xmalloc(len + 2);
			client_config.clientid[OPT_CODE] = DHCP_CLIENT_ID;
			client_config.clientid[OPT_LEN] = len;
			client_config.clientid[OPT_DATA] = '\0';
			strncpy(client_config.clientid + OPT_DATA, optarg, len);
			break;
		case 'f':
			client_config.foreground = 1;
			break;
		case 'b':
			client_config.background_if_no_lease = 1;
			break;
		case 'h':
		case 'H':
//#ifdef CONFIG_RTL865X_AC /* In order to accept empty character in HostName. Keith */	
#if defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD)
			fp_hostname=fopen("/var/hostname", "r");
			if(fp_hostname){
				memset(buf_hostname, '\0', 300);
				fseek(fp_hostname,10L, SEEK_SET);
				getword(fp_hostname, buf_hostname);
				//fscanf(fp_hostname, "%s", buf_hostname);
				//printf("the hostname =%s\n", buf_hostname);
				fclose(fp_hostname);
			}
			
			len = strlen(buf_hostname) > 255 ? 255 : strlen(buf_hostname);
			if (client_config.hostname) free(client_config.hostname);
			client_config.hostname = xmalloc(len + 2);
			client_config.hostname[OPT_CODE] = DHCP_HOST_NAME;
			client_config.hostname[OPT_LEN] = len;
			strncpy(client_config.hostname + 2, buf_hostname, len);
#else
			len = strlen(optarg) > 255 ? 255 : strlen(optarg);
			if (client_config.hostname) free(client_config.hostname);
			client_config.hostname = xmalloc(len + 2);
			client_config.hostname[OPT_CODE] = DHCP_HOST_NAME;
			client_config.hostname[OPT_LEN] = len;
			strncpy(client_config.hostname + 2, optarg, len);
#endif			
			
			break;
		case 'i':
			client_config.interface =  optarg;
			break;
		case 'n':
			client_config.abort_if_no_lease = 1;
			break;
		case 'p':
			client_config.pidfile = optarg;
			break;
		case 'q':
			client_config.quit_after_lease = 1;
			break;
		case 'r':
			requested_ip = inet_addr(optarg);
			break;
		case 's':
			client_config.script = optarg;
			break;
		case 'v':
			printf("udhcpcd, version %s\n\n", VERSION);
			exit_client(0);
			break;
#ifdef CHECK_SERVER_ALIVE
		case 'a':
			client_config.alive_time = atoi(optarg);
			break;	
		case 'u':
			client_config.url_name= optarg;
			break;
#endif
#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL865X_AC)
		case 'w':
			client_config.wan_type = atoi(optarg);
			break;	
		case 'I':
			client_config.Inform = atoi(optarg);
			break;	
		case 'B':
			client_config.broadcast_flag = 1;
			break;
#endif			
		case 'x':
			isDaemon = 1;
			break;
#if defined(CONFIG_RTL_ULINKER)
		case 'd':
			detect = atoi(optarg);
			break;
#endif
		default:
			show_usage();
		}
	}

	OPEN_LOG("udhcpc");
	LOG(LOG_INFO, "udhcp client (v%s) started", VERSION);
	if(isDaemon==1){
		if (daemon(0, 1) == -1) {
			perror("udhcpc fork error");
			return 0;
		}
	}
	pid_fd = pidfile_acquire(client_config.pidfile);
	pidfile_write_release(pid_fd);

	if (read_interface(client_config.interface, &client_config.ifindex, 
			   NULL, client_config.arp) < 0)
		exit_client(1);
		
	if (!client_config.clientid) {
		client_config.clientid = xmalloc(6 + 3);
		client_config.clientid[OPT_CODE] = DHCP_CLIENT_ID;
		client_config.clientid[OPT_LEN] = 7;
		client_config.clientid[OPT_DATA] = 1;
		memcpy(client_config.clientid + 3, client_config.arp, 6);
	}

	/* setup signal handlers */
	socketpair(AF_UNIX, SOCK_STREAM, 0, signal_pipe);
	signal(SIGUSR1, signal_handler);
	signal(SIGUSR2, signal_handler);
	signal(SIGTERM, signal_handler);
	
	state = INIT_SELECTING;
	run_script(NULL, "deconfig");
	change_mode(LISTEN_RAW);
	//brad add 20090314
	sysinfo(&system_info);
	dhcpc_seconds = (unsigned long) system_info.uptime;
	//end of brad add 20090314
	for (;;) {

		//tv.tv_sec = timeout - time(0);//brad modify, we do not care system time 20090314
		
		tv.tv_sec = timeout - dhcpc_seconds; //brad add 20090314
		
		tv.tv_usec = 0;
		FD_ZERO(&rfds);

		if (listen_mode != LISTEN_NONE && fd < 0) {
			if (listen_mode == LISTEN_KERNEL)
				fd = listen_socket(INADDR_ANY, CLIENT_PORT, client_config.interface);
			else
				fd = raw_socket(client_config.ifindex);
			if (fd < 0) {
				LOG(LOG_ERR, "FATAL: couldn't listen on socket, %s", strerror(errno));
				exit_client(0);
			}
		}
		if (fd >= 0) FD_SET(fd, &rfds);
		FD_SET(signal_pipe[0], &rfds);		

		if (tv.tv_sec > 0) {
			DEBUG(LOG_INFO, "Waiting on select...\n");
			max_fd = signal_pipe[0] > fd ? signal_pipe[0] : fd;
			retval = select(max_fd + 1, &rfds, NULL, NULL, &tv);
		} else retval = 0; /* If we already timed out, fall through */

		//now = time(0); //brad modify, we do not care system time 20090314
		//brad add 20090314
		sysinfo(&system_info);
		dhcpc_seconds = (unsigned long) system_info.uptime;
		now = dhcpc_seconds;
		//end of brad add 20090314
		if (retval == 0) {
			/* timeout dropped to zero */
			switch (state) {
			case INIT_SELECTING:
				if (packet_num < 3) {
					if (packet_num == 0)
						xid = random_xid();

					/* send discover packet */
					send_discover(xid, requested_ip); /* broadcast */
#if defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD)
				system("exlog /tmp/log_web.lck /tmp/log_web \"tag:SYSACT;log_num:13;msg:DHCP Discover;\"");
#endif
					timeout = now + ((packet_num == 2) ? 4 : 2);

				#if defined(CONFIG_RTL_ULINKER)
					if (detect)      timeout = now + 1;
					if (detect == 2) packet_num = 2;
				#endif

					packet_num++;
				} else {
					if (client_config.background_if_no_lease) {
						LOG(LOG_INFO, "No lease, forking to background.");
						background();
					} else if (client_config.abort_if_no_lease) {
						LOG(LOG_INFO, "No lease, failing.");
						exit_client(1);
				  	}
					/* wait to try again */
					packet_num = 0;

					/* make more constructive to send dhcp discover */
					timeout = now + 10;

#if defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD)
				system("exlog /tmp/log_web.lck /tmp/log_web \"tag:SYSACT;log_num:13;msg:DHCP Discover no response;\"");
				system("echo 1 > /var/wanip_fail");
#endif

#if defined(CONFIG_RTL_ULINKER)
	if (detect) {
		system("echo \"2\" > /var/ulinker_dhcps_discover_flag"); //fail to get ip
		exit(0);
	}
	else {
		dhcpc_retry++;
		if (dhcpc_retry >= 2) {
			system("echo \"2\" > /var/ulinker_auto_dhcp"); //fail to get ip
		}
	}
#endif

				}
				break;
			case RENEW_REQUESTED:
			case REQUESTING:
				if (packet_num < 3) {
					/* send request packet */
					if (state == RENEW_REQUESTED)
						send_renew(xid, server_addr, requested_ip); /* unicast */
					else send_selecting(xid, server_addr, requested_ip); /* broadcast */
					
					timeout = now + ((packet_num == 2) ? 10 : 2);
					packet_num++;
#if defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD)
				sprintf(logbuf, "exlog /tmp/log_web.lck /tmp/log_web \"tag:SYSACT;log_num:13;msg:DHCP request;note:%u.%u.%u.%u;\"", ((unsigned char*)&requested_ip)[0], ((unsigned char*)&requested_ip)[1], ((unsigned char*)&requested_ip)[2], ((unsigned char*)&requested_ip)[3] );
				system(logbuf);
#endif	
				} else {
					/* timed out, go back to init state */
					if (state == RENEW_REQUESTED) run_script(NULL, "deconfig");
					state = INIT_SELECTING;
					timeout = now;
					packet_num = 0;
					change_mode(LISTEN_RAW);
				}
				break;
			case BOUND:
				
#ifdef CHECK_SERVER_ALIVE
				if (lease_expired_left > 0) {
					if (!is_server_alive()) {				
						perform_release();
						perform_renew();
						break;
					}			
					if (lease_expired_left > client_config.alive_time) {
						//timeout =  client_config.alive_time + time(0);// brad modify , we do not care system time 20090314
						timeout =  client_config.alive_time + dhcpc_seconds; //brad add 20090314
						lease_expired_left -= client_config.alive_time;						
					}					
					else {
						//timeout = lease_expired_left + time(0);// brad modify , we do not care system time 20090314
						timeout = lease_expired_left +dhcpc_seconds;//brad add 20090314
						lease_expired_left = 0;
					}
					break;
				}
#endif
				
				/* Lease is starting to run out, time to enter renewing state */
				state = RENEWING;
				change_mode(LISTEN_KERNEL);
				DEBUG(LOG_INFO, "Entering renew state");
				/* fall right through */
			case RENEWING:
				/* Either set a new T1, or enter REBINDING state */
				if ((t2 - t1) <= (lease / 14400 + 1)) {
					/* timed out, enter rebinding state */
					state = REBINDING;
					timeout = now + (t2 - t1);
					DEBUG(LOG_INFO, "Entering rebinding state");
				} else {
					/* send a request packet */
					send_renew(xid, server_addr, requested_ip); /* unicast */
					
					t1 = (t2 - t1) / 2 + t1;
					timeout = t1 + start;
				}
				break;
			case REBINDING:
				/* Either set a new T2, or enter INIT state */
				if ((lease - t2) <= (lease / 14400 + 1)) {
					/* timed out, enter init state */
					state = INIT_SELECTING;
					LOG(LOG_INFO, "Lease lost, entering init state");
					run_script(NULL, "deconfig");
					timeout = now;
					packet_num = 0;
					change_mode(LISTEN_RAW);
				} else {
					/* send a request packet */	
					change_mode(LISTEN_RAW);
				
					send_renew(xid, 0, requested_ip); /* broadcast */

					t2 = (lease - t2) / 2 + t2;
					timeout = t2 + start;
				}
				break;
			case RELEASED:
				/* yah, I know, *you* say it would never happen */
				timeout = 0x7fffffff;
				break;
			}
		} else if (retval > 0 && listen_mode != LISTEN_NONE && FD_ISSET(fd, &rfds)) {
			/* a packet is ready, read it */
			
			if (listen_mode == LISTEN_KERNEL)
				len = get_packet(&packet, fd);
			else len = get_raw_packet(&packet, fd);
			
			if (len == -1 && errno != EINTR) {
				DEBUG(LOG_INFO, "error on read, %s, reopening socket", strerror(errno));
				change_mode(listen_mode); /* just close and reopen */
			}
			if (len < 0) continue;
			
			if (packet.xid != xid) {
				DEBUG(LOG_INFO, "Ignoring XID %lx (our xid is %lx)",
					(unsigned long) packet.xid, xid);
				continue;
			}
			
			if ((message = get_option(&packet, DHCP_MESSAGE_TYPE)) == NULL) {
				DEBUG(LOG_ERR, "couldnt get option from packet -- ignoring");
				continue;
			}
			
			switch (state) {
			case INIT_SELECTING:
				/* Must be a DHCPOFFER to one of our xid's */
				if (*message == DHCPOFFER) {
					if ((temp = get_option(&packet, DHCP_SERVER_ID))) {
						memcpy(&server_addr, temp, 4);
						xid = packet.xid;
						requested_ip = packet.yiaddr;
						
						/* enter requesting state */
						state = REQUESTING;
						timeout = now;
						packet_num = 0;

				#if defined(CONFIG_RTL_ULINKER)
						if (detect == 2) {
							system("echo 3 > /var/ulinker_dhcps_discover_flag");
							exit(0);
						}
				#endif
					
					} else {
						DEBUG(LOG_ERR, "No server ID in message");
					}
				}
				break;
			case RENEW_REQUESTED:
			case REQUESTING:
			case RENEWING:
			case REBINDING:
				if (*message == DHCPACK) {
					if (!(temp = get_option(&packet, DHCP_LEASE_TIME))) {
						LOG(LOG_ERR, "No lease time with ACK, using 1 hour lease");
						lease = 60 * 60;
					} else {
						memcpy(&lease, temp, 4);
						lease = ntohl(lease);
					}
						
					/* enter bound state */
					t1 = lease / 2;
					
					/* little fixed point for n * .875 */
					t2 = (lease * 0x7) >> 3;
					temp_addr.s_addr = packet.yiaddr;
					LOG(LOG_INFO, "Lease of %s obtained, lease time %ld", 
						inet_ntoa(temp_addr), lease);
					start = now;
					timeout = t1 + start;
					requested_ip = packet.yiaddr;
#ifdef UDHCPC_PASS_DOMAINNAME2DHCPSERVER
					/* 20080805 handle option 15 if needed */
					OptionDomainName(&packet);
#endif
#if defined(CONFIG_RTL_ULINKER)
					OptionDNSServer(&packet);
#endif
					run_script(&packet,((state == RENEWING || state == REBINDING) ? "renew" : "bound"));
#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_AC)					   	
				if((client_config.wan_type == 7) || (client_config.wan_type == 8) || (client_config.wan_type == 9)){
#ifdef RFC3442			
					sleep(10);	
					// set to MIB. try to delete it first to avoid duplicate case
#if defined(UDHCPC_STATIC_FLASH) 
					apmib_init();
#endif
#ifdef UDHCPC_MS_CLASSLESS_STATIC_ROUTE
					/* 20080508 handle option 249 if needed */
					OptionMicroSoftClasslessStaticRoute(&packet);
#endif
#ifdef UDHCPC_RFC_CLASSLESS_STATIC_ROUTE
					/* handle option 121 if needed */
					OptionClasslessStaticRoute(&packet);
#endif
#endif
#ifdef UDHCPC_STATIC_ROUTE
					/* 20080508 handle option 33 if needed */				
					OptionStaticRoute(&packet);
#endif
				}//for tr ru wan type
				
#endif	
#if defined(CONFIG_RTL865X_KLD)
					netbios_nodtype=0;
					memset(netbios_scope, '\0', 32);
					wins_1=0;
					wins_2=0;
					OptionNetBios_NodeType(&packet);
					OptionNetBios_Scope(&packet);
					OptionWINS_Ip(&packet);
					create_config_file();
#endif

#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_KLD)	
				if(client_config.Inform==1){
					sprintf(cmdBuf, "echo \"%s\" > /tmp/dhcpd_action", "update_conf_isp");
					system(cmdBuf);
					 Inform_DHCPD();
					}
#endif			
					/*hf add 20101130. if success we clear up event*/
					sprintf(filename,"/proc/%s/up_event",client_config.interface);
					clearUpEvent(filename);
					/*hf add end 20101130*/
					state = BOUND;
					change_mode(LISTEN_NONE);
#ifdef SEND_GRATUITOUS_ARP
					for(k=0;k<GRATUITOUS_ARP_NUM;k++)
					{
						sendArpPack(client_config.arp, temp_addr.s_addr, temp_addr.s_addr);
						sleep(1);
					}
#endif
					if (client_config.quit_after_lease) 
						exit_client(0);

#if defined(CONFIG_RTL_ULINKER)
{
	/* dhcpc get ip */
	if (detect) {
		system("echo \"1\" > /var/ulinker_dhcps_discover_flag");
	}
	else {
		system("echo \"1\" > /var/ulinker_auto_dhcp");
	}
}
#endif

					if (!client_config.foreground)
						background();
#ifdef CHECK_SERVER_ALIVE
					if (client_config.alive_time && t1 > client_config.alive_time) {
						lease_expired_left = t1 - client_config.alive_time;						
						//timeout = client_config.alive_time + time(0); //brad modify, we donot care system time 20090314
						timeout = client_config.alive_time + dhcpc_seconds; //brad add 20090314
					}
					else
						lease_expired_left = 0;									
#endif
#if defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD)
				sprintf(logbuf, "exlog /tmp/log_web.lck /tmp/log_web \"tag:SYSACT;log_num:13;msg:DHCP request success;note:%u.%u.%u.%u;\"",((unsigned char*)&requested_ip)[0], ((unsigned char*)&requested_ip)[1], ((unsigned char*)&requested_ip)[2], ((unsigned char*)&requested_ip)[3] );
				system(logbuf);
#endif
				} else if (*message == DHCPNAK) {
					/* return to init state */
					LOG(LOG_INFO, "Received DHCP NAK");
					run_script(&packet, "nak");
					if (state != REQUESTING)
						run_script(NULL, "deconfig");
					state = INIT_SELECTING;
					timeout = now;
					requested_ip = 0;
					packet_num = 0;
					change_mode(LISTEN_RAW);
					sleep(3); /* avoid excessive network traffic */
				}
				break;
			/* case BOUND, RELEASED: - ignore all packets */
			}	
		} else if (retval > 0 && FD_ISSET(signal_pipe[0], &rfds)) {
			if (read(signal_pipe[0], &sig, sizeof(sig)) < 0) {
				DEBUG(LOG_ERR, "Could not read signal: %s", 
					strerror(errno));
				continue; /* probably just EINTR */
			}
			switch (sig) {
			case SIGUSR1: 
				perform_renew();
				break;
			case SIGUSR2:
				perform_release();
				break;
			case SIGTERM:
				LOG(LOG_INFO, "Received SIGTERM");
				exit_client(0);
			}
		} else if (retval == -1 && errno == EINTR) {
			/* a signal was caught */		
		} else {
			/* An error occured */
			DEBUG(LOG_ERR, "Error on select");
		}
		/*hf add 20101130*/
		if(BOUND == state) {
			sprintf(filename,"/proc/%s/up_event",client_config.interface);
			if(getUpEvent(filename) > 0){
				clearUpEvent(filename);
				perform_release();
				sleep(1);
				perform_renew();
			}
		}
		/*hf add end 20101130*/
	}
	return 0;
}

