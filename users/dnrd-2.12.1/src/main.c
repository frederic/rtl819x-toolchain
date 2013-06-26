/*
 * main.c - contains the main() function
 *
 * Copyright (C) 1998 Brad M. Garcia <garsh@home.com>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <sys/types.h>
#include "relay.h"
#include "cache.h"
#include "common.h"
#include "args.h"
#include "sig.h"
#include "master.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <dirent.h>

#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT) || defined(CONFIG_RTL8196C_EC)
//Brad add for domain blocking
char domain_tbl[30][40];
int domain_filter=0;
int total_entryDomain=0;
//Brad add for url blocking ipaddress
char url_tbl[30][40];

int url_filter=0;
int total_entryURL=0;
int isKeyMatched=0;
int TRlogEnabled=0;
unsigned int ipaddr[50];
char HOSTNAME_CK[100];
char HOSTNAME[100];
//Brad add end
#endif
#ifdef DNS_MAP
int init_DnsTable(void)
{
	FILE *inConfig=NULL;
    char *gap;
	char *pBuf;
	int tableIdx = 0;
	int i;
	
	for(i=0;i<MAX_TABLE_NUM;i++)
	{
		bzero((char*)gDnsTable[i].exp, sizeof(gDnsTable[i].exp));
		gDnsTable[i].idx = 0;
	}

	char buffer[MAX_STRING_LEN+1];
	bzero(buffer, sizeof(buffer));
	pBuf = buffer;
	if (!(inConfig = fopen(DNS_MAP_FILE, "r"))) 
	{
		log_debug("unable to open dns map file: %s", DNS_MAP_FILE);
		return 0;
	}
    while(fgets(pBuf,MAX_STRING_LEN,inConfig)) 
	{	
		pBuf[MAX_STRING_LEN] = '\0';
		gap=strchr(pBuf,'#');
		if(gap)
		{
			*gap = '\0';
			memcpy(gDnsTable[tableIdx].exp, pBuf, strlen(pBuf));
			pBuf += strlen(pBuf)+1;
			gap=strchr(pBuf,'#');
			if(gap)
			{
				*gap = '\0';
				gDnsTable[tableIdx].idx = atoi(pBuf);
				tableIdx++;		//only exp and idx is not null, tableIdx will add 
			}
		}
		if(tableIdx == MAX_TABLE_NUM)
		{
			log_debug("Do not have enough space to save DNS rules!\n");
			break;
		}

		pBuf=buffer;
		bzero(pBuf, sizeof(buffer));
	}

	TableNum = tableIdx;
	close(inConfig);
	return 1;
}
#endif
/*
 * main() - startup the program.
 *
 * In:      argc - number of command-line arguments.
 *          argv - string array containing command-line arguments.
 *
 * Returns: 0 on exit, -1 on error.
 *
 * Abstract: We set up the signal handler, parse arguments,
 *           turn into a daemon, write our pid to /var/run/dnrd.pid,
 *           setup our sockets, and then parse packets until we die.
 */
int main(int argc, char *argv[])
{
    int                i;
    FILE              *filep;
    struct servent    *servent;   /* Let's be good and find the port numbers
				     the right way */
    struct passwd     *pwent;
    DIR               *dirp;
    struct dirent     *direntry;
    struct stat        st;
    int                rslt;
#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT) || defined(CONFIG_RTL8196C_EC)
    char line[100];
//Brad add for domain blocking
	FILE *fp;
	int index=0;
	char strBuf[150];
//Brad add end
#endif
    /*
     * Setup signal handlers.
     */
    signal(SIGINT,  sig_handler);
    signal(SIGQUIT, sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGUSR1, sig_handler);
    /*
     * Handling TCP requests is done by forking a child.  When they terminate
     * they send SIGCHLDs to the parent.  This will eventually interrupt
     * some system calls.  Because I don't know if this is handled it's better
     * to ignore them -- 14OCT99wzk
     */
    signal(SIGCHLD, SIG_IGN);

    /*
     * Initialization in common.h of recv_addr is broken, causing at
     * least the '-a' switch not to work.  Instead of assuming
     * positions of fields in the struct across platforms I thought it
     * safer to do a standard initialization in main().
     */
    memset(&recv_addr, 0, sizeof(recv_addr));
    recv_addr.sin_family = AF_INET;
    recv_addr.sin_port = htons(53);

    /*
     * Parse the command line.
     */
    parse_args(argc, argv);

    openlog(progname, LOG_PID, LOG_DAEMON);
#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT) || defined(CONFIG_RTL8196C_EC)
//Brad add
//Add domain filter function here
//read config file first to check domain filter 
fp = fopen("/var/domain.conf", "r");
 if(!fp){
 	//printf("open domain conf fail will not do domain filter!\n");
	domain_filter=0;
 }else{		
	 while(1){
	 	if(!fgets(strBuf,sizeof(strBuf),fp))
			break;
		
		if(index==0){
			domain_filter=atoi(&strBuf[0]);
			if(domain_filter==0)
				break;
		}else if(index ==1){
			//TRlogEnabled=(unsigned char) strtol(&strBuf[0], (char**)NULL, 16);
			sscanf(strBuf, "%d", &TRlogEnabled);
			//printf("TRLog=%02x\n", TRlogEnabled);
		}		
		else{
			sscanf(strBuf, "%s", domain_tbl[index-2]);
			//sprintf(domain_tbl[index-1], "%s", strBuf);
		}
		index++;
	 }	
	  fclose(fp);
 	}

// for(i=0;i<index-2;i++)
// 	printf("domain name=%s\n", domain_tbl[i]);
 total_entryDomain = index-2;
 index=0;
 fp = fopen("/var/url.conf", "r");
 if(!fp){
 	//printf("open url conf fail will not do url filter!\n");
	url_filter=0;
 }else{		
	 while(1){
	 	if(!fgets(strBuf,sizeof(strBuf),fp))
			break;
		
		if(index==0){
			url_filter=atoi(&strBuf[0]);
			if(url_filter==0)
				break;
		}else{
			sscanf(strBuf, "%s", url_tbl[index-1]);
			//sprintf(domain_tbl[index-1], "%s", strBuf);
		}
		index++;
	 }	
	  fclose(fp);
 	}

for(i=0;i<50;i++)
ipaddr[i]=0;
// for(i=0;i<index-1;i++)
 //	printf("url name=%s\n", url_tbl[i]);
 total_entryURL = index-1;

 memset(HOSTNAME_CK, '\0', 100);
 memset(HOSTNAME, '\0', 100);
 
 /* Here we JUST grep host name from /etc/hosts */
 if ((fp = fopen("/etc/hosts", "r")) != NULL) 
 {
 	if (fgets(line, sizeof(line), fp) != NULL) 
	{
		// The line may be like '192.168.2.200\Wireless N Router\Wireless N Router.Wireless N Router\' 
		char *p;
		char *str_p, *end_p;
		int	len=0;
		unsigned int c;
		
		p = skip_ws(noctrl(line));

		log_debug("main(). p = [%s] [%u]",p,__LINE__);
	

		// First we address the str_p
		while((c=*p) !=0 && c != '\\')
		{
			p++;
		}
		p++;
		str_p = p;

		// Then we address the end_p
		while((c=*p) !=0 && c != '\\')
		{
			p++;
			len++;
		}
		end_p = p;

		strncpy(HOSTNAME, str_p, len);

		log_debug("main(). HOSTNAME = %s [%u]",HOSTNAME,__LINE__);

	}
	close(fp);
    }	
//Brad add end
#endif

#ifdef DNS_MAP
	if(0 == init_DnsTable())
	{
		log_debug("LZQ: INTI init_DnsTable error!  ### \n");
		exit(-1);
	}
#endif

    /*
     * Kill any currently running copies of the program.
     */
    kill_current();

    /*
     * Setup the thread synchronization semaphore
     */
    /*
    if (sem_init(&dnrd_sem, 0, 1) == -1) {
	log_msg(LOG_ERR, "Couldn't initialize semaphore");
	cleanexit(-1);
    }
    */

    /*
     * Write our pid to the appropriate file.
     * Just open the file here.  We'll write to it after we fork.
     */
    filep = fopen(pid_file, "w");
    if (!filep) {
	log_msg(LOG_ERR, "can't write to %s.  "
		"Check that dnrd was started by root.", pid_file);
	exit(-1);
    }

    /*
     * Pretend we don't know that we want port 53
     */
    servent = getservbyname("domain", "udp");
    if (servent != getservbyname("domain", "tcp")) {
	log_msg(LOG_ERR, "domain ports for udp & tcp differ.  "
	       "Check /etc/services");
	exit(-1);
    }
    recv_addr.sin_port = servent ? servent->s_port : htons(53);

    /*
     * Setup our DNS query reception socket.
     */
    if ((isock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	log_msg(LOG_ERR, "isock: Couldn't open socket");
	cleanexit(-1);
    }
    else {
	int opt = 1;
	setsockopt(isock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    }

    if (bind(isock, (struct sockaddr *)&recv_addr, sizeof(recv_addr)) < 0) {
	log_msg(LOG_ERR, "isock: Couldn't bind local address");
	cleanexit(-1);
    }
#ifdef ENABLE_TCP
    /*
     * Setup our DNS tcp proxy socket.
     */
    if ((tcpsock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	log_msg(LOG_ERR, "tcpsock: Couldn't open socket");
	cleanexit(-1);
    }
    else {
	int opt = 1;
	setsockopt(tcpsock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    }
    if (bind(tcpsock, (struct sockaddr *)&recv_addr, sizeof(recv_addr)) < 0) {
	log_msg(LOG_ERR, "tcpsock: Couldn't bind local address");
	cleanexit(-1);
    }
    if (listen(tcpsock, 5) != 0) {
	log_msg(LOG_ERR, "tcpsock: Can't listen");
	cleanexit(-1);
    }
#endif
    /* Initialise our cache */
    cache_init();

    /* Initialise out master DNS */
    master_init();

    pwent = getpwnam("root");


    /*
     * Change our root and current working directories to /etc/dnrd.
     * Also, so some sanity checking on that directory first.
     */
    dirp = opendir("/etc/dnrd");
    if (!dirp) {
	log_msg(LOG_ERR, "The directory /etc/dnrd must be created before "
		"dnrd will run");
    }

    rslt = stat("/etc/dnrd", &st);
    if (st.st_uid != 0) {
	log_msg(LOG_ERR, "The /etc/dnrd directory must be owned by root");
	cleanexit(-1);
    }
    if ((st.st_mode & (S_IWGRP | S_IWOTH)) != 0) {
	log_msg(LOG_ERR,
		"The /etc/dnrd directory should only be user writable");
	cleanexit(-1);
    }

    while ((direntry = readdir(dirp)) != NULL) {

	if (!strcmp(direntry->d_name, ".") ||
	    !strcmp(direntry->d_name, "..")) {
	    continue;
	}

	rslt = stat(direntry->d_name, &st);

	if (rslt) continue;
	if (S_ISDIR(st.st_mode)) {
	    log_msg(LOG_ERR, "The /etc/dnrd directory must not contain "
		    "subdirectories");
	    cleanexit(-1);
	}
	if ((st.st_mode & (S_IXUSR|S_IXGRP|S_IXOTH|S_IWGRP|S_IWOTH)) != 0) {
	    log_msg(LOG_ERR, "A file in /etc/dnrd has either execute "
		    "permissions or non-user write permission.  Please do a "
		    "\"chmod a-x,go-w\" on all files in this directory");
	    cleanexit(-1);
	}
	if (st.st_uid != 0) {
	    log_msg(LOG_ERR, "All files in /etc/dnrd must be owned by root");
	    cleanexit(-1);
	}
    }
    closedir(dirp);
//Brad comment out 
#if 0

    if (chdir("/etc/dnrd")) {
	log_msg(LOG_ERR, "couldn't chdir to %s, %s",
		"/etc/dnrd", strerror(errno));
	cleanexit(-1);
    }

    if (chroot("/etc/dnrd")) {
	log_msg(LOG_ERR, "couldn't chroot to %s, %s",
		"/etc/dnrd", strerror(errno));
	cleanexit(-1);
    }
#endif    
	if (chdir("/")) {
	log_msg(LOG_ERR, "couldn't chdir to %s, %s",
		"/", strerror(errno));
	printf("couldn't chdir to %s, %s","/", strerror(errno));	
	cleanexit(-1);
    }
    if (chroot("/")) {
	log_msg(LOG_ERR, "couldn't chroot to %s, %s",
		"/", strerror(errno));
	printf("couldn't chroot to %s, %s","/", strerror(errno));	
	cleanexit(-1);
    }
    /*
     * Change uid/gid to something other than root.
     */

    /* drop supplementary groups */
    if (setgroups(0, NULL) < 0) {
	log_msg(LOG_ERR, "can't drop supplementary groups");
	cleanexit(-1);
    }

    /*
     * Switch uid/gid to something safer than root if requested.
     * By default, attempt to switch to user & group id 65534.
     */

    if (daemongid != 0) {
	if (setgid(daemongid) < 0) {
	    log_msg(LOG_ERR, "couldn't switch to gid %i", daemongid);
	    cleanexit(-1);
	}
    }
    else if (!pwent) {
	log_msg(LOG_ERR, "Couldn't become the \"nobody\" user.  Please use "
		"the \"-uid\" option.\n"
		"       dnrd must become a non-root process.");
	cleanexit(-1);
    }
    else if (setgid(pwent->pw_gid) < 0){
	log_msg(LOG_ERR, "couldn't switch to gid %i", pwent->pw_gid);
	cleanexit(-1);
    }



    if (daemonuid != 0) {
	if (setuid(daemonuid) < 0) {
	    log_msg(LOG_ERR, "couldn't switch to uid %i", daemonuid);
	    cleanexit(-1);
	}
    }
    else if (!pwent) {
	log_msg(LOG_ERR, "Couldn't become the \"nobody\" user.  Please use "
		"the \"-uid\" option.\n"
		"       dnrd must become a non-root process.");
	cleanexit(-1);
    }
    else if (setuid(pwent->pw_uid) < 0){
	log_msg(LOG_ERR, "couldn't switch to uid %i", pwent->pw_uid);
	cleanexit(-1);
    }



	
    /*
     * Setup our DNS query forwarding socket.
     */
    for (i = 0; i < serv_cnt; i++) {
	if ((dns_srv[i].sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	    log_msg(LOG_ERR, "osock: Couldn't open socket");
	    cleanexit(-1);
	}

	dns_srv[i].addr.sin_family = AF_INET;
	dns_srv[i].addr.sin_port   = htons(53);
    }

    /*
     * Now it's time to become a daemon.
     */
    if (!opt_debug) {
	pid_t pid = fork();
	if (pid < 0) {
	    log_msg(LOG_ERR, "%s: Couldn't fork\n", progname);
	    exit(-1);
	}
	if (pid != 0) exit(0);
	gotterminal = 0;
	setsid();
	chdir("/");
	umask(077);
	fclose(stdin);
	fclose(stdout);
	fclose(stderr);
    }

    /*
     * Write our pid to the appropriate file.
     * Now we actually write to it and close it.
     */
    fprintf(filep, "%i\n", (int)getpid());
    fclose(filep);

    /*
     * Run forever.
     */
    run();
    exit(0); /* to make compiler happy */
}

