/* 
 * files.c -- DHCP server file manipulation *
 * Rewrite by Russ Dill <Russ.Dill@asu.edu> July 2001
 */
 
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <netdb.h>

#ifdef STATIC_LEASE
//#include <netinet/ether.h>
#include "static_leases.h"
#endif

#include "debug.h"
#include "dhcpd.h"
#include "files.h"
#include "options.h"
#include "leases.h"

/* on these functions, make sure you datatype matches */
static int read_ip(char *line, void *arg)
{
	struct in_addr *addr = arg;
	struct hostent *host;
	int retval = 1;

	if (!inet_aton(line, addr)) {
		if ((host = gethostbyname(line))) 
			addr->s_addr = *((unsigned long *) host->h_addr_list[0]);
		else retval = 0;
	}
	return retval;
}


static int read_str(char *line, void *arg)
{
	char **dest = arg;
	
	if (*dest) free(*dest);
	*dest = strdup(line);
	
	return 1;
}


static int read_u32(char *line, void *arg)
{
	u_int32_t *dest = arg;
	char *endptr;
	*dest = strtoul(line, &endptr, 0);
	return endptr[0] == '\0';
}


static int read_yn(char *line, void *arg)
{
	char *dest = arg;
	int retval = 1;

	if (!strcasecmp("yes", line))
		*dest = 1;
	else if (!strcasecmp("no", line))
		*dest = 0;
	else retval = 0;
	
	return retval;
}


/* read a dhcp option and add it to opt_list */
static int read_opt(char *line, void *arg)
{
	struct option_set **opt_list = arg;
	char *opt, *val, *endptr;
	struct dhcp_option *option = NULL;
	int retval = 0, length = 0;
	char buffer[255];
	u_int16_t result_u16;
	u_int32_t result_u32;
	int i;

	if (!(opt = strtok(line, " \t="))) return 0;
	
	for (i = 0; options[i].code; i++)
		if (!strcmp(options[i].name, opt))
			option = &(options[i]);
		
	if (!option) return 0;
	
	do {
//#ifdef CONFIG_RTL865X_AC
#if defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD)
		if(!strcmp(option, "domain")){	
			val = strtok(NULL, "\t");
		}else{
			val = strtok(NULL, ", \t");
		}
#else
		val = strtok(NULL, ", \t");
#endif		
		if (val) {
			length = option_lengths[option->flags & TYPE_MASK];
			retval = 0;
			switch (option->flags & TYPE_MASK) {
			case OPTION_IP:
				retval = read_ip(val, buffer);
				break;
			case OPTION_IP_PAIR:
				retval = read_ip(val, buffer);
				if (!(val = strtok(NULL, ", \t/-"))) retval = 0;
				if (retval) retval = read_ip(val, buffer + 4);
				break;
			case OPTION_STRING:
				length = strlen(val);
				if (length > 0) {
					if (length > 254) length = 254;
					memcpy(buffer, val, length);
					retval = 1;
				}
				break;
			case OPTION_BOOLEAN:
				retval = read_yn(val, buffer);
				break;
			case OPTION_U8:
				buffer[0] = strtoul(val, &endptr, 0);
				retval = (endptr[0] == '\0');
				break;
			case OPTION_U16:
				result_u16 = htons(strtoul(val, &endptr, 0));
				memcpy(buffer, &result_u16, 2);
				retval = (endptr[0] == '\0');
				break;
			case OPTION_S16:
				result_u16 = htons(strtol(val, &endptr, 0));
				memcpy(buffer, &result_u16, 2);
				retval = (endptr[0] == '\0');
				break;
			case OPTION_U32:
				result_u32 = htonl(strtoul(val, &endptr, 0));
				memcpy(buffer, &result_u32, 4);
				retval = (endptr[0] == '\0');
				break;
			case OPTION_S32:
				result_u32 = htonl(strtol(val, &endptr, 0));	
				memcpy(buffer, &result_u32, 4);
				retval = (endptr[0] == '\0');
				break;
			default:
				break;
			}
			if (retval) 
				attach_option(opt_list, option, buffer, length);
		};
	} while (val && retval && option->flags & OPTION_LIST);
	return retval;
}
#if defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8186_TR)		
int read_opt_from_isp(char *data){ 
	read_opt(data, &(server_config.options));
}
#endif

#ifdef STATIC_LEASE
static int read_mac(const char *line, void *arg)
{
	unsigned char *mac_bytes = arg;
	unsigned char temp_ether_addr[20], tmpbuf[20];
	int retval = 1, i;

	for (i=0; i<6; i++) {
		memcpy(tmpbuf, &line[i*2], 2);
		tmpbuf[2] = '\0';
		temp_ether_addr[i] = strtol(tmpbuf, NULL, 16);
	}	
	memcpy(mac_bytes, temp_ether_addr, 6);
	return retval;
}

static int read_staticlease(char *const_line, void *arg)
{

	char *line;
//	char *mac_string;
	char *ip_string;
	unsigned char *mac_bytes;
	u_int32_t *ip;
	char *host_str, *host=NULL;


	/* Allocate memory for addresses */
	mac_bytes = xmalloc(sizeof(unsigned char) * 8);
	ip = xmalloc(sizeof(u_int32_t));

	/* Read mac */
	line = (char *) const_line;
	read_mac(line, mac_bytes);

	/* Read ip */
	ip_string = strstr(line, " ");
	ip_string = ip_string + strspn(ip_string, " ");	
	read_ip(ip_string, ip);

	/* Read Host */	
	host_str = strstr(ip_string, " ");
	if (host_str) {
		host_str = host_str + strspn(host_str, " ");
		host = xmalloc(strlen(host_str)+1);
		strcpy(host, host_str);
	}
	addStaticLease(arg, mac_bytes, ip, host);

#ifdef UDHCP_DEBUG
	printStaticLeases(arg);
#endif

	return 1;

}
#endif // STATIC_LEASE


static struct config_keyword keywords[] = {
	/* keyword[14]	handler   variable address		default[20] */
	{"start",	read_ip,  &(server_config.start),	"192.168.0.20"},
	{"end",		read_ip,  &(server_config.end),		"192.168.0.254"},
	{"interface",	read_str, &(server_config.interface),	"eth0"},
	{"option",	read_opt, &(server_config.options),	""},
	{"opt",		read_opt, &(server_config.options),	""},
	{"max_leases",	read_u32, &(server_config.max_leases),	"254"},
	{"remaining",	read_yn,  &(server_config.remaining),	"yes"},
	{"auto_time",	read_u32, &(server_config.auto_time),	"7200"},
	{"decline_time",read_u32, &(server_config.decline_time),"3600"},
	{"conflict_time",read_u32,&(server_config.conflict_time),"3600"},
	{"offer_time",	read_u32, &(server_config.offer_time),	"60"},
	{"min_lease",	read_u32, &(server_config.min_lease),	"60"},
	{"lease_file",	read_str, &(server_config.lease_file),	"/var/lib/misc/udhcpd.leases"},
	{"pidfile",	read_str, &(server_config.pidfile),	"/var/run/udhcpd.pid"},
	{"notify_file", read_str, &(server_config.notify_file),	""},
	{"siaddr",	read_ip,  &(server_config.siaddr),	"0.0.0.0"},
	{"sname",	read_str, &(server_config.sname),	""},
	{"boot_file",	read_str, &(server_config.boot_file),	""},
#ifdef GUEST_ZONE	
	{"guestmac_check",	read_u32, &(server_config.guestmac_check),	""},
#endif	
#ifdef STATIC_LEASE
	{"static_lease",read_staticlease, &(server_config.static_leases),	""},
#endif	
#if defined(CONFIG_RTL865X_KLD)	
{"updatecfg_isp",	read_u32, &(server_config.upateConfig_isp),	""},
{"res_br",	read_u32, &(server_config.response_broadcast),	""},
{"updatecfg_dns",	read_u32, &(server_config.upateConfig_isp_dns),	""},
#endif
	/*ADDME: static lease */
	{"",		NULL, 	  NULL,				""}
};


int read_config(char *file)
{
	FILE *in;
	char buffer[80], orig[80], *token, *line;
	int i;

	for (i = 0; strlen(keywords[i].keyword); i++)
		if (strlen(keywords[i].def))
			keywords[i].handler(keywords[i].def, keywords[i].var);

	if (!(in = fopen(file, "r"))) {
		LOG(LOG_ERR, "unable to open config file: %s", file);
		return 0;
	}
	
	while (fgets(buffer, 80, in)) {
		if (strchr(buffer, '\n')) *(strchr(buffer, '\n')) = '\0';
		strncpy(orig, buffer, 80);
		if (strchr(buffer, '#')) *(strchr(buffer, '#')) = '\0';
		token = buffer + strspn(buffer, " \t");
		if (*token == '\0') continue;
		line = token + strcspn(token, " \t=");
		if (*line == '\0') continue;
		*line = '\0';
		line++;
		
		/* eat leading whitespace */
		line = line + strspn(line, " \t=");
		/* eat trailing whitespace */
		for (i = strlen(line); i > 0 && isspace(line[i - 1]); i--);
		line[i] = '\0';
		
		for (i = 0; strlen(keywords[i].keyword); i++)
			if (!strcasecmp(token, keywords[i].keyword))
				if (!keywords[i].handler(line, keywords[i].var)) {
					LOG(LOG_ERR, "unable to parse '%s'", orig);
					/* reset back to the default value */
					keywords[i].handler(keywords[i].def, keywords[i].var);
				}
	}
	fclose(in);
	return 1;
}


void write_leases(void)
{
	FILE *fp;
	unsigned int i;
	char buf[255];
	time_t curr = time(0);
	unsigned long lease_time;
#if defined(CONFIG_RTL8186_KB) || defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD)
	 u_int8_t empty_haddr[16]; 
        memset(empty_haddr, 0, 16); 
#endif	
	if (!(fp = fopen(server_config.lease_file, "w"))) {
		LOG(LOG_ERR, "Unable to open %s for writing", server_config.lease_file);
		return;
	}
	
	for (i = 0; i < server_config.max_leases; i++) {
		if (leases[i].yiaddr != 0) {
			if (server_config.remaining) {
				if (lease_expired(&(leases[i])))
					lease_time = 0;
				else lease_time = leases[i].expires - curr;
			} else lease_time = leases[i].expires;

#ifdef STATIC_LEASE
			if (reservedIp(server_config.static_leases, leases[i].yiaddr))
				lease_time = 0xffffffff;
#endif	
			lease_time = htonl(lease_time);
			fwrite(leases[i].chaddr, 16, 1, fp);
			fwrite(&(leases[i].yiaddr), 4, 1, fp);
			fwrite(&lease_time, 4, 1, fp);
#if defined(CONFIG_RTL8186_KB) || defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD)
			fwrite(leases[i].hostname, 64, 1, fp); /* Brad add for get hostname of client */
			  if (!memcmp(leases[i].chaddr, empty_haddr, 16)) {
			  	 leases[i].isUnAvailableCurr = 1;
			}else
				leases[i].isUnAvailableCurr = 0;
			fwrite(&(leases[i].isUnAvailableCurr), 4, 1, fp);
#endif

#ifdef GUEST_ZONE
			{
				int is_guest = 0;
				struct guest_mac_entry *newguest;
				if (server_config.guestmac_check && server_config.guestmac_tbl) {				
					if (!is_guest_exist(leases[i].chaddr, &newguest)) {	
						if (is_guest_mac(server_config.interface, leases[i].chaddr)) {
							if (newguest) {
								memcpy(newguest->addr, leases[i].chaddr, 6);		
								newguest->valid = 1;											
							}
							is_guest = 1;
						}
					}
					else
						is_guest = 1;
				}
				fwrite(&is_guest, 4, 1, fp);
			}
#endif
		}
	}
	fclose(fp);
	
	if (server_config.notify_file) {
		sprintf(buf, "%s %s", server_config.notify_file, server_config.lease_file);
		system(buf);
	}
}


void read_leases(char *file)
{
	FILE *fp;
	unsigned int i = 0;
	struct dhcpOfferedAddr lease;
	
	if (!(fp = fopen(file, "r"))) {
		LOG(LOG_ERR, "Unable to open %s for reading", file);
		return;
	}
	
	while (i < server_config.max_leases && (fread(&lease, sizeof lease, 1, fp) == 1)) {
		/* ADDME: is it a static lease */
	//	printf("%s:%d read lease.yiaddr=%08x\n",__FUNCTION__,__LINE__,lease.yiaddr);
		if (lease.yiaddr >= server_config.start && lease.yiaddr <= server_config.end
			&& lease.yiaddr!=server_config.server) {
			lease.expires = ntohl(lease.expires);
#ifdef STATIC_LEASE				
			if(server_config.static_leases == NULL){
				//check the entry is static lease or not when static lease is disable
				if(lease.expires  == 0xFFFFFFFF){
					lease.expires = ntohl(server_config.lease);
				}
			}
#endif			
			if (!server_config.remaining) lease.expires -= time(0);
#if defined(CONFIG_RTL8186_KB) || defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD)
			if (!(add_lease_fromfile(lease.chaddr, lease.yiaddr, lease.expires, lease.hostname))) {
				LOG(LOG_WARNING, "Too many leases while loading %s\n", file);
				break;
			}				
#else
			if (!(add_lease(lease.chaddr, lease.yiaddr, lease.expires))) {
				LOG(LOG_WARNING, "Too many leases while loading %s\n", file);
				break;
			}				
#endif			
		}
		i++;
	}
	DEBUG(LOG_INFO, "Read %d leases", i);
	fclose(fp);
}
#if defined(CONFIG_RTL865X_KLD)	
//Brad add for revoke lease entry 2008/08/16 		
void revoke_leases(char *entry_ip){
	struct in_addr inIp;
	
	inet_aton(entry_ip, &inIp);
	unsigned int i = 0;
	while (i < server_config.max_leases) {
		if (leases[i].yiaddr == inIp.s_addr){
			clear_lease(leases[i].chaddr, leases[i].yiaddr);
			break;
		}
	}
	
}		
#endif		
		
//Brad add sync system time for dhcpd 2008/01/28 		
void read_leases_update_expire(char *file, unsigned long sysTime_orig)
{
	unsigned int i = 0;
	unsigned long sysTime_curr=0;
	unsigned long lease_remaining=0;
	
	time(&sysTime_curr);
	
	while (i < server_config.max_leases) {
		if (leases[i].yiaddr >= server_config.start && leases[i].yiaddr <= server_config.end
				&& leases[i].yiaddr!=server_config.server) {
			leases[i].expires = ntohl(leases[i].expires);
#ifdef STATIC_LEASE			
				if(server_config.static_leases == NULL){
					//static lease is disabled, check the entry is leased for static lease already or not
					if(leases[i].expires != 0xFFFFFFFF){
						if(leases[i].expires < sysTime_orig){
							leases[i].expires = 0;
						}else{
							lease_remaining = leases[i].expires - sysTime_orig;
							leases[i].expires = lease_remaining+sysTime_curr;
							}
					}else{
						//the entry is for static lease and it is used already, set the lease time as default
						leases[i].expires = server_config.lease+sysTime_curr;
					}
			}else{
				//static lease is enabled
					if(leases[i].expires != 0xFFFFFFFF && (reservedIp(server_config.static_leases, leases[i].yiaddr)==0)){
					//check the entry is not leased for static lease entry
					if(leases[i].expires < sysTime_orig){
						leases[i].expires = 0;
					}else{
						lease_remaining = leases[i].expires - sysTime_orig;
						leases[i].expires = lease_remaining+sysTime_curr;
						}
					}else if(leases[i].expires != 0xFFFFFFFF && (reservedIp(server_config.static_leases, leases[i].yiaddr)==1)){
						//check the entry is leased for static lease entry
							leases[i].expires = 0xFFFFFFFF;
				}//else the entry is leased for static lease and static lease is enabled
			}
#else
			if(leases[i].expires < sysTime_orig){
					leases[i].expires = 0;
			}else{
					lease_remaining = leases[i].expires - sysTime_orig;
					leases[i].expires = lease_remaining+sysTime_curr;
			}
#endif	//STATIC_LEASE			
		}
		i++;
	}
}
//----------------------------------------------------------


		
