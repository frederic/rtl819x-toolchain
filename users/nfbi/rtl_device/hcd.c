/*
  *   Host Control Daemon for RTL8197B
  *
  *	Copyright (C)2008, Realtek Semiconductor Corp. All rights reserved.
  *
  *	$Id: hcd.c,v 1.3 2010/02/05 10:39:37 marklee Exp $
  */

/*================================================================*/
/* System Include Files */

#include <stdio.h> 
#include <stdlib.h> 
#include <fcntl.h> 
#include <string.h>
#include <sys/types.h> 
#include <sys/uio.h> 
#include <unistd.h> 
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/wait.h> 

/*================================================================*/
/* Local Include Files */
#include "cmd.h"
#include "hcd.h"
#include "mib.h"
#include "../../../linux-2.6.30/drivers/char/rtl_mdio/rtl_mdio.h"

/*================================================================*/
/* Local Variables */
static int is_sys_init = 0;

/*================================================================*/
/* External Import */
extern int init_wlan(char *ifname);
extern int init_vlan(void);
extern unsigned char *get_macaddr(unsigned char *ifname);
extern unsigned int get_vap_num(void);


/*================================================================*/
/* Routine Implementations */

void real_sleep(unsigned int sec)
{
    unsigned int s;
    
	s = sec;
	while ((s = sleep(s)))
	    ;
}

#ifdef DB_MSG
void inline debug_out(unsigned char *label, unsigned char *data, int data_length)
{
    int i,j;
    int num_blocks;
    int block_remainder;

    num_blocks = data_length >> 4;
    block_remainder = data_length & 15;

	if (label) {
	    DEBUG_OUT("%s\n", label);
	}

	if (data==NULL || data_length==0)
		return;

    for (i=0; i<num_blocks; i++)
    {
        printf("\t");
        for (j=0; j<16; j++)
        {
            printf("%02x ", data[j + (i<<4)]);
        }
        printf("\n");
    }

    if (block_remainder > 0)
    {
        printf("\t");
        for (j=0; j<block_remainder; j++)
        {
            printf("%02x ", data[j+(num_blocks<<4)]);
        }
        printf("\n");
    }
}
#endif // DB_MSG

static int pidfile_acquire(char *pidfile)
{
	int pid_fd;

	if(pidfile == NULL)
		return -1;

	pid_fd = open(pidfile, O_CREAT | O_WRONLY, 0644);
	if (pid_fd < 0) 
		printf("Unable to open pidfile %s\n", pidfile);
	else 
		lockf(pid_fd, F_LOCK, 0);

	return pid_fd;
}

static void pidfile_write_release(int pid_fd)
{
	FILE *out;

	if(pid_fd < 0)
		return;

	if((out = fdopen(pid_fd, "w")) != NULL) {
		fprintf(out, "%d\n", getpid());
		fclose(out);
	}
	lockf(pid_fd, F_UNLCK, 0);
	close(pid_fd);
}

void process_host_cmd(unsigned char *data, int len)
{
	int id, ret,cmd_len=0,reply_flag=1,i;
	unsigned char cmd[MAX_HOST_CMD_LEN];
#if 0
	if (len > sizeof(cmd)) {
		DEBUG_ERR("CMD length too big [%d]!\n", len);
		return;
	}	
#endif	
	memset(cmd, '\0', sizeof(cmd));
	
	ret = parse_cmd_header(data,len,&cmd,&cmd_len);
	if(ret != 0)  //header error
	 return ;

	id = data[1];
	ret = do_cmd(id,&cmd,cmd_len,reply_flag);  //do requested cmd , and reply a rsp_frame if need
	
	if (ret < 0) 
		DEBUG_ERR("do command return failed!\n");		
}
void init_bridge()
{
	unsigned char addr[6];
	char tmpbuf[100];
	//ADD_BR(IF_BR);	
	sprintf(tmpbuf, "%s addbr %s", BR_UTL, IF_BR); 
	system(tmpbuf); 
	sprintf(tmpbuf, "%s %s hw ether %s up", IFCONFG, IF_BR,INBAND_MAC); 
	system(tmpbuf);
	DISABLE_STP(IF_BR);		

	memcpy(addr, ALL_ZERO_MAC_ADDR, 6);
	ADD_BR_INTERFACE(IF_BR, IF_MII, addr,1,1);    
	
	bring_up_lan();	
	is_sys_init =1 ;	//mark_issue
}

void init_system(int action)
{
	int index=0;
	char tmpbuf[100];

	// add interface and start
	if ((action & INIT_WLAN_PHY))
		for(index=0; index<MAX_WLAN_INTF; index++)
			init_wlan(index);
	
	// shutdown interface	
	if (is_sys_init) {
		bring_down_lan();
	
		DEL_BR_INTERFACE(IF_BR, IF_MII,
											(action&INIT_MII_MAC), //MARKLEE_DEBUG											
											0);	
		bring_down_wlan();
	
		if (action & INIT_BR)
			DEL_BR(IF_BR);
	}	
	
	init_bridge();

	bring_up_wlan();

	init_vlan(); //mark_issue

	is_sys_init =1 ;	
}

#ifdef CMD_LINE
static int get_token(char *line, char **token1, char **token2, char **token3, char **token4) 
{		
	int len;
	int token_idx = 0, total = strlen(line);

search_next:
	len = 0;
	while (*line== 0x20 ||*line== '\t') {
		line++;
		total--;		
	}

	if (token_idx == 0)
		*token1 = line;		
	else if (token_idx == 1)
		*token2 = line;		
	else	 if (token_idx == 2)
		*token3 = line;		
	else	 if (token_idx == 3)
		*token4 = line;			
	else
		return token_idx;

	while (total > 0 &&  *line && *line != 0x20  && *line != '\t' && *line != '\r' && *line != '\n' ) {
		line++;
		len++;
		total--;
	}

	if (len > 0) {
		*line = '\0';
		line++;
		token_idx++;
		total--;
		goto search_next;
	}

	if (strlen(line)==0 || token_idx ==4 || total <= 0)	
		return token_idx;
	else
		goto search_next;		
}

static void manual_cmd_handler(int sig_no)
{
	FILE *fp;
	char line[300], *t1=NULL, *t2=NULL, *t3=NULL, *t4=NULL;
	int num;
	char tmp_buf[100];
	char cmd_rsp[MAX_HOST_CMD_LEN];
	
	fp = fopen(CMD_FILE, "r");	
	
	if (fp == NULL) {
		DEBUG_ERR("manual cmd file empty!\n");
		return;
	}
		
	fgets(line, sizeof(line), fp);
	fclose(fp);
	unlink(CMD_FILE);
	
	num = get_token(line,  &t1, &t2, &t3, &t4);
	

	if (!strcmp(t1, DUMP_ALL_MIB)) {
		printf("\n------ Read/Write MIB ------\n");
		sprintf(tmp_buf,"%s ",t2);
		dump_mib( 1, tmp_buf);
	}
	else if (num == 2 && !strcmp(t1, "getstainfo")) {
		printf("\n--------getstainfo---------- \n");
		strcpy(cmd_rsp,t2);
		if(do_cmd(id_getstainfo,cmd_rsp,strlen(t2)+1, 0) < 0)
			DEBUG_ERR("getstainfo failed !\n");	
		else//ok
			print_stainfo(cmd_rsp);
	}
	else if (num == 2 && !strcmp(t1, "getassostanum")) {
		printf("\n--------getassostanum---------- \n");
		strcpy(cmd_rsp,t2);
		if(do_cmd(id_getassostanum,cmd_rsp,strlen(t2)+1, 0) < 0)
			DEBUG_ERR("getassostanum failed !\n");	
		else
			printf("Associated statsion number = %d \n",(unsigned char)cmd_rsp[0]);
	}
	else if (num == 2 &&  !strcmp(t1, "getbssinfo")) {
		printf("\n--------getbssinfo---------- \n");
		strcpy(cmd_rsp,t2);
		if(do_cmd(id_getbssinfo,cmd_rsp,strlen(t2)+1, 0) < 0)
			DEBUG_ERR("getbssinfo failed !\n");	
		else//ok
			print_bssinfo(cmd_rsp);
	}
	else if (num == 3 && !strcmp(t1, "cfgwrite")) {
		sprintf(tmp_buf,"%s %s",t2,t3);
		//if(do_cmd(id_cfgwrite,t2,strlen(t2)+1, 0) < 0)
		if(do_cmd(id_cfgwrite,tmp_buf,strlen(tmp_buf)+1, 0) < 0)
			DEBUG_ERR("cfgwrite failed : [%s]!\n", tmp_buf);	
		else//ok
			printf("cfgwrite ok: [%s]\n", tmp_buf);	
	}
	else if (num == 3 && !strcmp(t1, "cfgread")) { 
		sprintf(tmp_buf,"%s %s",t2,t3);
		//printf("cfgread [%s] :",tmp_buf);	
		dump_mib( 0, tmp_buf);
	}
	else if (num == 2 && !strcmp(t1, "sysinit")) {
		strcpy(tmp_buf,t2);
		if(do_cmd(id_sysinit,t2,strlen(t2)+1, 0) < 0)
			DEBUG_ERR("sysinit failed : [%s]!\n", tmp_buf);	
		else//ok
			printf("sysinit ok: [%s]\n", tmp_buf);	
	}
	else if (num == 2 && !strcmp(t1, "getlanstatus")) {
		strcpy(cmd_rsp,t2);
		if(do_cmd(id_getlanstatus,cmd_rsp,strlen(t2)+1, 0) < 0)
			DEBUG_ERR("getlanstatus failed : [%s]!\n", t2);	
		else//ok
		{			
			printf("getlanstatus ok: [%s]\n", t2);	
			print_port_status(cmd_rsp);
		}	
	}
	else if (num == 2 && !strcmp(t1, "getstats")) {
		strcpy(cmd_rsp,t2);
		if(do_cmd(id_getstats,cmd_rsp,strlen(t2)+1, 0) < 0)
			DEBUG_ERR("getstats failed : [%s]!\n", t2);	
		else//ok
		{
			printf("getstats ok: [%s] \n", t2);	
			print_port_stats(cmd_rsp);
		}	
	}
	else {
		DEBUG_ERR("%s: invalid cmd! [num=%d, t1=%s, t2=%s, t3=%s]\n", 
							__FUNCTION__, num, t1, t2, t3);
	}
}
#endif // CMD_LINE
	
static void show_help(void)
{
		printf("\nhcd [argument...]\n");
		printf("\t-%s\n", LOAD_DAEMON);
#ifdef CMD_LINE		
		printf("\t-%s\n", DUMP_ALL_MIB);	
		printf("\t-%s sys/p0/wlan0/wlan_comm xxx=xxx \n", "cfgwrite");
		printf("\t-%s sys/p0/wlan0/wlan_comm xxx \n", "cfgread");
		printf("\t-%s all      (it will init the system using new config!!!) \n", "sysinit");	
		printf("\t-%s wlan0 \n", "getstainfo");	
		printf("\t-%s wlan0 \n", "getassostanum");	
		printf("\t-%s wlan0 \n", "getbssinfo");	
		printf("\t-%s \n", "getlanstatus p1/p2/p3/p4/p5");	
		printf("\t-%s \n", "getstats p1/p2/p3/p4/p5/wlan0");	
		printf("\t-%s wlan0 (not support yet) \n", "getwdsinfo");		

#endif		
		printf("\n");
}

static int parse_argument(int argc, char *argv[])
{
	int argNum=1;
#ifdef CMD_LINE	
	FILE *fp;
	int pid;
	unsigned char line[100];
	char tmpbuf[100];
#endif

	if (argc <= 1) {
		show_help();
		return -1;
	}		

	// parsing argument		
	if (argv[argNum][0]=='-' && !strcmp(&argv[argNum][1], LOAD_DAEMON)) 
 		return 0;	

#ifdef CMD_LINE	
	// look for daemon pid file
	fp = fopen(PID_FILE, "r");
	if (fp) {		
		fgets(line, sizeof(line), fp);
		if (sscanf(line, "%d", &pid) <= 0 ||pid <= 1) {
			printf("Invalid pid file!\n");
			return -1;
		}
	}	
	if (argv[argNum][0]=='-' && !strcmp(&argv[argNum][1], "cfgwrite")) {
		if (fp == NULL) 
			goto daemon_not_start;		
		if (argNum+2 > argc) {
			printf("Invalid format! [%s mib_name]\n", "cfgwrite");
			goto show_cfgwrite_help;
		}
		sprintf(tmpbuf, "echo %s %s %s > %s", "cfgwrite", argv[argNum+1], argv[argNum+2], CMD_FILE);
	}
	else if (argv[argNum][0]=='-' && !strcmp(&argv[argNum][1], "cfgread")) {
		if (fp == NULL) 
			goto daemon_not_start;		
		if (argNum+2 > argc) {
			printf("Invalid format! [%s mib_name]\n", "cfgread");
			goto show_cfgread_help;
		}
		sprintf(tmpbuf, "echo %s %s %s> %s", "cfgread", argv[argNum+1], argv[argNum+2], CMD_FILE);			
	}
	else if (argv[argNum][0]=='-' && !strcmp(&argv[argNum][1], "sysinit")) {
		if (fp == NULL) 
			goto daemon_not_start;		
		if (argNum+2 > argc) {
			printf("Invalid format! [%s mib_name]\n", "sysinit");
			goto show_sysinit_help;
		}
		sprintf(tmpbuf, "echo %s %s > %s", "sysinit", argv[argNum+1], CMD_FILE);			
	}
	else if (argv[argNum][0]=='-' && !strcmp(&argv[argNum][1], "getlanstatus")) {
		if (fp == NULL) 
			goto daemon_not_start;		
		if (argNum+2 > argc) {
			printf("Invalid format! [%s mib_name]\n", "getlanstatus");
			goto show_sysinit_help;
		}
		sprintf(tmpbuf, "echo %s %s > %s", "getlanstatus", argv[argNum+1], CMD_FILE);			
	}
	else if (argv[argNum][0]=='-' && !strcmp(&argv[argNum][1], "getstainfo")) {
		if (fp == NULL) 
			goto daemon_not_start;				
		sprintf(tmpbuf, "echo %s %s > %s",  "getstainfo", argv[argNum+1], CMD_FILE);
	}
	else if (argv[argNum][0]=='-' && !strcmp(&argv[argNum][1], "getassostanum")) {
		if (fp == NULL) 
			goto daemon_not_start;				
		sprintf(tmpbuf, "echo %s %s > %s",  "getassostanum", argv[argNum+1], CMD_FILE);
	}	
	else if (argv[argNum][0]=='-' && !strcmp(&argv[argNum][1], "getbssinfo")) {
		if (fp == NULL) 
			goto daemon_not_start;							
		sprintf(tmpbuf, "echo %s %s > %s",  "getbssinfo", argv[argNum+1], CMD_FILE);
	}	
	else if (argv[argNum][0]=='-' && !strcmp(&argv[argNum][1], DUMP_ALL_MIB)) {
		if (fp == NULL) 
			goto daemon_not_start;		
		sprintf(tmpbuf, "echo %s %s > %s",  DUMP_ALL_MIB, argv[argNum+1], CMD_FILE);
	}	
	else if (argv[argNum][0]=='-' && !strcmp(&argv[argNum][1], "getstats")) {
		if (fp == NULL) 
			goto daemon_not_start;		
		sprintf(tmpbuf, "echo %s %s > %s",  "getstats", argv[argNum+1], CMD_FILE);
	}
#endif // CMD_LINE
	else {
		show_help();
#ifdef CMD_LINE
		if (fp)
			fclose(fp);
#endif		
		return -1;		
	}

#ifdef CMD_LINE
	system(tmpbuf);
	kill(pid, SIGUSR2);		
	fclose(fp);		
	return -1;

show_cfgwrite_help:
show_cfgread_help:
show_sysinit_help:

ret:
	fclose(fp);		
	return -1;
#endif
	return -1;

#ifdef CMD_LINE
daemon_not_start:
	printf("daemon has not been started!\n");
	return -1;
#endif	
}

//wait child process to terminate
static void sigchld_handler(int signo)
{
	pid_t pid;
	int stat;
	
	while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
		printf("child %d termniated\n", pid);
}

int main(int argc, char *argv[])
{ 	
	int pid_fd, pid, scr;
	char tmpbuf[100];

	if (parse_argument(argc, argv) != 0) 
		return 0;

	// become daemon
	if (daemon(0,1) == -1) {
		printf("fork daemon error!\n");
		return 0;
	}

	// create pid file
	pid_fd = pidfile_acquire(PID_FILE);
	if (pid_fd < 0) {
		printf("Create PID file failed!\n");
		return 0;
	}
	pidfile_write_release(pid_fd);

	DISPLAY_BANNER;
	
	// initialize mib
	init_config_mib();

	// register signal handler
#ifdef CMD_LINE
	signal(SIGUSR2, manual_cmd_handler);	
#endif
     signal(SIGCHLD, sigchld_handler);

	// open mdio or  inband channel
	
#ifdef CONFIG_SYSTEM_MII_INBAND_CTL	
	   //init_inband_if(INBAND_NETIF,INBAND_MAC);
	   init_bridge();
          sleep(1);	
	   inband_open(INBAND_NETIF);	
#else
	    mdio_open();
#endif	
    
       //init_system(INIT_ALL);  //mark_issue

	// Set daemon pid to driver. 
	// When this ioctl is set, driver will set AllSoftwareReady bit to 'CPU System Status' Register
	pid =  getpid();
	//if (do_mdio_ioctl(id_set_host_pid, (void *)&pid)) //mark_issue , if it is need then mdio_open must be done 
	//	return 1;

	while(1){		
#ifdef CONFIG_SYSTEM_MII_INBAND_CTL	
	   //ioh_open();
	inband_wait_event((void *)&process_host_cmd);   
#else	
	mdio_wait_event((void *)&process_host_cmd);
	sleep(1);
#endif
	}	
}
