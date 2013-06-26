/* files.h */
#ifndef _FILES_H
#define _FILES_H

struct config_keyword {
	char keyword[14];
	int (*handler)(char *line, void *var);
	void *var;
	char def[30];
};


int read_config(char *file);
void write_leases(void);
void read_leases(char *file);
//Brad add sync system time for dhcpd 2008/01/28 	
void read_leases_update_expire(char *file, unsigned long sysTime_orig);
#if defined(CONFIG_RTL865X_KLD)	
int read_opt_from_isp(char *data);
void revoke_leases(char *entry_ip);
#endif
#endif
