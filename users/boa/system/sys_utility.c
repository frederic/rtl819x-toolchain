/*
 *
 */

/* System include files */
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <netdb.h>
#include <stdarg.h>
#include <errno.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/wait.h>
//#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#include <net/if.h>
#include <stddef.h>		/* offsetof */
#include <net/if_arp.h>
#include <linux/if_ether.h>
#include "sysconf.h"
#include "sys_utility.h"

#include <dirent.h>


#define READ_BUF_SIZE	50

#define ifreq_offsetof(x)  offsetof(struct ifreq, x)
#define _PATH_PROCNET_DEV "/proc/net/dev"
///////////////////////////////////////////////////////////////////////////////////////

int DoCmd(char *const argv[], char *file)
{    
	pid_t pid;
	int status;
	int fd;
	char _msg[30];
	switch (pid = fork()) {
			case -1:	/* error */
				perror("fork");
				return errno;
			case 0:	/* child */
				
				signal(SIGINT, SIG_IGN);
				if(file){
					if((fd = open(file, O_RDWR | O_CREAT))==-1){ /*open the file */
						sprintf(_msg, "open %s", file); 
  						perror(_msg);
  						exit(errno);
					}
					dup2(fd,STDOUT_FILENO); /*copy the file descriptor fd into standard output*/
					dup2(fd,STDERR_FILENO); /* same, for the standard error */
					close(fd); /* close the file descriptor as we don't need it more  */
				}else{
			#ifndef SYS_DEBUG		
					close(2); //do not output error messages
			#endif	
				}
				setenv("PATH", "/sbin:/bin:/usr/sbin:/usr/bin", 1);
				execvp(argv[0], argv);
				perror(argv[0]);
				exit(errno);
			default:	/* parent */
			{
				
				waitpid(pid, &status, 0);
			#ifdef SYS_DEBUG	
				if(status != 0)
					printf("parent got child's status:%d, cmd=%s %s %s\n", status, argv[0], argv[1], argv[2]);
			#endif		
				if (WIFEXITED(status)){
			#ifdef SYS_DEBUG	
					printf("parent will return :%d\n", WEXITSTATUS(status));
			#endif		
					return WEXITSTATUS(status);
				}else{
					
					return status;
				}
			}
	}
}
int RunSystemCmd(char *filepath, ...)
{
	va_list argp;
	char *argv[24]={0};
	int status;
	char *para;
	int argno = 0;
	va_start(argp, filepath);

	while (1){ 
		para = va_arg( argp, char*);
		if ( strcmp(para, "") == 0 )
			break;
		argv[argno] = para;
		//printf("Parameter %d is: %s\n", argno, para); 
		argno++;
	} 
	argv[argno+1] = NULL;
	status = DoCmd(argv, filepath);
	va_end(argp);
	return status;
}

void string_casecade(char *dest, char *src)
{
	char tmp_str[30]={0};
	if(dest[0]==0x0){
		sprintf(dest, "%s",src); 
	}else{
		sprintf(tmp_str, " %s", src);
		strcat(dest, tmp_str);
	}
}


int isFileExist(char *file_name)
{
	struct stat status;

	if ( stat(file_name, &status) < 0)
		return 0;

	return 1;
}
int getPid_fromFile(char *file_name)
{
	FILE *fp;
	char *pidfile = file_name;
	int result = -1;
	
	fp= fopen(pidfile, "r");
	if (!fp) {
        	printf("can not open:%s\n", file_name);
		return -1;
   	}
	fscanf(fp,"%d",&result);
	fclose(fp);
	
	return result;
}

int setInAddr( char *interface, char *Ipaddr, char *Netmask, char *HwMac, int type)
{
    struct ifreq ifr;
    int skfd=0;
    struct in_addr in_addr, in_netmask, in_broadaddr;
    struct sockaddr sa;
    int default_type= (IFACE_FLAG_T) | (IP_ADDR_T) | (NET_MASK_T) | (HW_ADDR_T);
    int request_action=0;
    request_action =type & default_type;
	
    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;
		
    strcpy(ifr.ifr_name, interface);
   
	    if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0){
	    		close( skfd );
			return (-1);
		}
	 if(request_action ==IFACE_FLAG_T){
	 	return 0;
		}
	ifr.ifr_flags = IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST;
	if(request_action & IP_ADDR_T ){
		if (Ipaddr) {
			inet_aton(Ipaddr, &in_addr);
			(((struct sockaddr_in *)(&ifr.ifr_addr))->sin_addr).s_addr= in_addr.s_addr;
			ifr.ifr_addr.sa_family = AF_INET;
			if (ioctl(skfd, SIOCSIFADDR, &ifr) < 0)
				goto set_err;
		}
	}
	if(request_action & NET_MASK_T){
		if (Ipaddr && Netmask) {
			inet_aton(Netmask, &in_netmask);
			(((struct sockaddr_in *)(&ifr.ifr_netmask))->sin_addr).s_addr= in_netmask.s_addr;
			ifr.ifr_netmask.sa_family = AF_INET;
			if (ioctl(skfd, SIOCSIFNETMASK, &ifr) < 0)
				goto set_err;
	
			in_broadaddr.s_addr = (in_addr.s_addr & in_netmask.s_addr) | ~in_netmask.s_addr;
			(((struct sockaddr_in *)(&ifr.ifr_broadaddr))->sin_addr).s_addr= in_broadaddr.s_addr;
			ifr.ifr_broadaddr.sa_family = AF_INET;
			if (ioctl(skfd, SIOCSIFBRDADDR, &ifr) < 0)
				goto set_err;
		}
	}
	if(request_action & HW_ADDR_T){
		if(HwMac){
			sa.sa_family = ARPHRD_ETHER;
			memcpy(&sa.sa_data,HwMac,6); 
			memcpy((((char *) (&ifr)) +ifreq_offsetof(ifr_hwaddr)), &sa, sizeof(struct sockaddr));
			if (ioctl(skfd, SIOCSIFHWADDR, &ifr) < 0)
				goto set_err;
			
		}
	}
    close( skfd );
    return 0;
    
set_err:
	close(skfd);
	perror(interface);
	return -1;
}
int getInAddr( char *interface, int type, void *pAddr )
{
    struct ifreq ifr;
    int skfd, found=0;
	struct sockaddr_in *addr;
    skfd = socket(AF_INET, SOCK_DGRAM, 0);

    strcpy(ifr.ifr_name, interface);
    if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0){
    		close( skfd );
		return (0);
	}
    if (type == HW_ADDR_T) {
    	if (ioctl(skfd, SIOCGIFHWADDR, &ifr) >= 0) {
		memcpy(pAddr, &ifr.ifr_hwaddr, sizeof(struct sockaddr));
		found = 1;
	}
    }
    else if (type == IP_ADDR_T) {
	if (ioctl(skfd, SIOCGIFADDR, &ifr) == 0) {
		addr = ((struct sockaddr_in *)&ifr.ifr_addr);
		*((struct in_addr *)pAddr) = *((struct in_addr *)&addr->sin_addr);
		found = 1;
	}
    }
    else if (type == NET_MASK_T) {
	if (ioctl(skfd, SIOCGIFNETMASK, &ifr) >= 0) {
		addr = ((struct sockaddr_in *)&ifr.ifr_addr);
		*((struct in_addr *)pAddr) = *((struct in_addr *)&addr->sin_addr);
		found = 1;
	}
    }else {
    	
    	if (ioctl(skfd, SIOCGIFDSTADDR, &ifr) >= 0) {
		addr = ((struct sockaddr_in *)&ifr.ifr_addr);
		*((struct in_addr *)pAddr) = *((struct in_addr *)&addr->sin_addr);
		found = 1;
	}
    	
    }
    close( skfd );
    return found;

}
char *get_name(char *name, char *p)
{
	while (isspace(*p))
		p++;
	while (*p) {
		if (isspace(*p))
			break;
		if (*p == ':') {	/* could be an alias */
			char *dot = p, *dotname = name;

			*name++ = *p++;
			while (isdigit(*p))
				*name++ = *p++;
			if (*p != ':') {	/* it wasn't, backup */
				p = dot;
				name = dotname;
			}
			if (*p == '\0')
				return NULL;
			p++;
			break;
		}
		*name++ = *p++;
	}
	*name++ = '\0';
	return p;
}

int if_readlist_proc(char *target, char *key, char *exclude)
{
	FILE *fh;
	char buf[512];
	char *s, name[16], tmp_str[16];
	int iface_num=0;
	fh = fopen(_PATH_PROCNET_DEV, "r");
	if (!fh) {
		return 0;
	}
	fgets(buf, sizeof buf, fh);	/* eat line */
	fgets(buf, sizeof buf, fh);
	while (fgets(buf, sizeof buf, fh)) {
		s = get_name(name, buf);
		if(strstr(name, key)){
			iface_num++;
			if(target[0]==0x0){
				sprintf(target, "%s", name);
			}else{
				sprintf(tmp_str, " %s", name);
				strcat(target, tmp_str);
			}
		}
		//printf("iface name=%s, key=%s\n", name, key);
	}
	
	fclose(fh);
	return iface_num;
}
int write_line_to_file(char *filename, int mode, char *line_data)
{
	unsigned char tmpbuf[512];
	int fh=0;

	if(mode == 1) {/* write line datato file */
		
		fh = open(filename, O_RDWR|O_CREAT|O_TRUNC);
		
	}else if(mode == 2){/*append line data to file*/
		
		fh = open(filename, O_RDWR|O_APPEND);	
	}
	
	
	if (fh < 0) {
		fprintf(stderr, "Create %s error!\n", filename);
		return 0;
	}


	sprintf((char *)tmpbuf, "%s", line_data);
	write(fh, tmpbuf, strlen((char *)tmpbuf));



	close(fh);
	return 1;
}
/*create deconfig script for dhcp client*/
void Create_script(char *script_path, char *iface, int network, char *ipaddr, char *mask, char *gateway)
{
	
	unsigned char tmpbuf[100];
	int fh;
	
	fh = open(script_path, O_RDWR|O_CREAT|O_TRUNC, S_IRWXO|S_IRWXG);	
	if (fh < 0) {
		fprintf(stderr, "Create %s file error!\n", script_path);
		return;
	}
	if(network==LAN_NETWORK){
		sprintf((char *)tmpbuf, "%s", "#!/bin/sh\n");
		write(fh, tmpbuf, strlen((char *)tmpbuf));
		sprintf((char *)tmpbuf, "ifconfig %s %s netmask %s\n", iface, ipaddr, mask);
		write(fh, tmpbuf, strlen((char *)tmpbuf));
		sprintf((char *)tmpbuf, "while route del default dev %s\n", iface);
		write(fh, tmpbuf, strlen((char *)tmpbuf));
		sprintf((char *)tmpbuf, "%s\n", "do :");
		write(fh, tmpbuf, strlen((char *)tmpbuf));
		sprintf((char *)tmpbuf, "%s\n", "done");
		write(fh, tmpbuf, strlen((char *)tmpbuf));
		sprintf((char *)tmpbuf, "route add -net default gw %s dev %s\n", gateway, iface);
		write(fh, tmpbuf, strlen((char *)tmpbuf));
		sprintf((char *)tmpbuf, "%s\n", "init.sh ap wlan_app");
		write(fh, tmpbuf, strlen((char *)tmpbuf));
	}
	if(network==WAN_NETWORK){
		sprintf((char *)tmpbuf, "%s", "#!/bin/sh\n");
		write(fh, tmpbuf, strlen((char *)tmpbuf));

#if 0 //def CONFIG_POCKET_ROUTER_SUPPORT //it needn't do this
		sprintf((char *)tmpbuf, "sysconf disc dhcpc\n");
		write(fh, tmpbuf, strlen((char *)tmpbuf));			
#endif	

		sprintf((char *)tmpbuf, "ifconfig %s 0.0.0.0\n", iface);
		write(fh, tmpbuf, strlen((char *)tmpbuf));
		
	}
	close(fh);
}

unsigned char *gettoken(const unsigned char *str,unsigned int index,unsigned char symbol)
{
	static char tmp[50];
	unsigned char tk[50]; //save symbol index
	//char *ptmp;
	int i,j,cnt=1,start,end;
	//scan symbol and save index
	
	memset(tmp, 0x00, sizeof(tmp));
	
	for (i=0;i<strlen((char *)str);i++)
	{          
		if (str[i]==symbol)
		{
			tk[cnt]=i;
			cnt++;
		}
	}
	
	if (index>cnt-1)
	{
		return NULL;
	}
			
	tk[0]=0;
	tk[cnt]=strlen((char *)str);
	
	if (index==0)
		start=0;
	else
		start=tk[index]+1;

	end=tk[index+1];
	
	j=0;
	for(i=start;i<end;i++)
	{
		tmp[j]=str[i];
		j++;
	}
		
	return (unsigned char *)tmp;
}



extern pid_t find_pid_by_name( char* pidName)
{
	DIR *dir;
	struct dirent *next;

	dir = opendir("/proc");
	if (!dir) {
		printf("Cannot open /proc");
		return 0;
	}

	while ((next = readdir(dir)) != NULL) {
		FILE *status;
		char filename[READ_BUF_SIZE];
		char buffer[READ_BUF_SIZE];
		char name[READ_BUF_SIZE];

		/* Must skip ".." since that is outside /proc */
		if (strcmp(next->d_name, "..") == 0)
			continue;

		/* If it isn't a number, we don't want it */
		if (!isdigit(*next->d_name))
			continue;

		sprintf(filename, "/proc/%s/status", next->d_name);
		if (! (status = fopen(filename, "r")) ) {
			continue;
		}
		if (fgets(buffer, READ_BUF_SIZE-1, status) == NULL) {
			fclose(status);
			continue;
		}
		fclose(status);

		/* Buffer should contain a string like "Name:   binary_name" */
		sscanf(buffer, "%*s %s", name);
		if (strcmp(name, pidName) == 0) {
		//	pidList=xrealloc( pidList, sizeof(pid_t) * (i+2));
			return((pid_t)strtol(next->d_name, NULL, 0));

		}
	}
	if ( strcmp(pidName, "init")==0)
		return 1;

	return 0;
}

void reinit_webs()
{
	FILE *fp=NULL;	
	char *webPid = "/var/run/webs.pid";
	pid_t pid=0;
	char line[20];
	//unsigned char cmdBuffer[100];

	if ((fp = fopen(webPid, "r")) != NULL) 
	{
		fgets(line, sizeof(line), fp);
		fclose(fp);
		if ( sscanf(line, "%d", &pid) ) 
		{
			if (pid > 1)
			{
				printf("reinit boa \n");
				system("killall -9 boa > /dev/null");
				system("rm -rf /var/run/webs.pid");
				system("boa");
			}
		}		
	}

}
int getDefaultRoute(char *interface, struct in_addr *route)
{
	char buff[1024], iface[16];
	char gate_addr[128], net_addr[128], mask_addr[128];
	int num, iflags, metric, refcnt, use, mss, window, irtt;
	FILE *fp = fopen(_PATH_PROCNET_ROUTE, "r");
	char *fmt;
	int found=0;
	unsigned long addr;

	if (!fp) {
       		printf("Open %s file error.\n", _PATH_PROCNET_ROUTE);
		return 0;
    	}

	fmt = "%16s %128s %128s %X %d %d %d %128s %d %d %d";

	while (fgets(buff, 1023, fp)) {
		num = sscanf(buff, fmt, iface, net_addr, gate_addr,
		     		&iflags, &refcnt, &use, &metric, mask_addr, &mss, &window, &irtt);
		if (num < 10 || !(iflags & RTF_UP) || !(iflags & RTF_GATEWAY) || strcmp(iface, interface))
	    		continue;
		sscanf(gate_addr, "%lx", &addr );
		*route = *((struct in_addr *)&addr);

		found = 1;
		break;
	}

    	fclose(fp);
    	return found;
}
/***************************************************
/		get data from file, file must be
/   dataName data
/   	mode,
/   when dataName appear more than once,use nubmber 
/   count and get the data,begin from 1 
***************************************************/
int getDataFormFile(char* fileName,char* dataName,char* data,char number)
{
	char buff[128]={0};
	char strName[64]={0},strData[64]={0};
	char count=0;
	FILE *fp=fopen(fileName,"r");
	if (!fp) 
	{
   		printf("Open %s file error.\n", fileName);
		return 0;
	}

	while(fgets(buff,127,fp))
	{
		sscanf(buff,"%s %s",strName,strData);
		if(!strcmp(dataName,strName))
		{
			count++;
			if(number==0||count==number)
				goto FIND;			
		}		
	}
	fclose(fp);
	return 0;	
FIND:
	strcpy(data,strData);
	fclose(fp);
	return 1;
}

int killDaemonByPidFile(char *pidFile)
{
	char strPID[10];
	int pid=-1;
	
	if(!pidFile)
	{
		printf("%s : input file name is null!\n",__FUNCTION__);
		return -1;
	}
	if(isFileExist(pidFile))
	{
		pid=getPid_fromFile(pidFile);
		if(pid != 0){
			sprintf(strPID, "%d", pid);
			RunSystemCmd(NULL_FILE, "kill", "-9", strPID, NULL_STR);
		}
		unlink(pidFile);
		return 0;
	}else
	{
		//printf("%s : %s file is not exist!\n",__FUNCTION__,pidFile);
		return -1;
	}
}