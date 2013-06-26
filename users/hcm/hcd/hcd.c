/*
  *   Host Control Daemon for RTL8197B
  *
  *	Copyright (C)2008, Realtek Semiconductor Corp. All rights reserved.
  *
  *	$Id: hcd.c,v 1.24 2011/02/14 08:13:53 brian Exp $
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
#include <sys/stat.h> //mark_file
#include <net/if.h>
#include "../../inband_lib/ioh.h"
#include "../../inband_lib/inband_if.h"
#include "../../inband_lib/wireless_copy.h"

/*================================================================*/
/* Local Include Files */
#include "cmd.h"
#include "hcd.h"
#include "mib.h"

/*================================================================*/
/* Local Variables */
static int is_sys_init = 0;
static int mdio_fd;
int ioctl_sock=-1;
#define RTK_NFBI_AP_HCM 1  //mark_test
#define WLAN_ROOT ("wlan0")
#define HCD_WPS_CONFIG ("/tmp/wps_config")
/* INBAND_HOST ---> */
#define SIOCSAPPPID     0x8b3e
static int event_channel = -1;
#define INBAND_INTF		"br0"
#define INBAND_SLAVE	("001234567899")
#define INBAND_HOST		("00e04c8196c1")
#define INBAND_EVENT_TYPE 0x9001
#define INBAND_DEBUG 0
#define MAXDATALEN      1560
#define DOT11_EVENT_REQUEST 2
#define SIOCGIWIND 0x89ff
/* INBAND_HOST <--- */

#ifdef INBAND_WPS_OVER_HOST
#else
#define RTK_WPS_BRIDGE_HACK 1
#endif

#ifdef RTK_NFBI_AP_HCM
#ifdef _KERNEL_LINUX_26
#include "../../../linux-2.6.30/drivers/char/rtl_mdio/rtl_mdio.h"
#else
#include "../../../linux-2.4.18/drivers/char/rtl_mdio.h"   //mark_test , it's not a good way!!!
#endif
#define id_set_host_pid                                         0x100           // set host daemon pid
#endif
/*================================================================*/
/* External Import */
extern int init_wlan(char *ifname);
extern int init_vlan(void);
extern unsigned char *get_macaddr(unsigned char *ifname);
extern unsigned int get_vap_num(void);

extern struct config_mib_all mib_all;

typedef struct _DOT11_REQUEST{
        unsigned char   EventId;
}DOT11_REQUEST;

int hcd_inband_chan=0;
/*================================================================*/
/* Routine Implementations */

void real_sleep(unsigned int sec)
{
    unsigned int s;
    
	s = sec;
	while ((s = sleep(s)))
	    ;
}

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

static void bridge_all_if()
{  
  //lan if
  bridge_all_lan();
   
  //wlan if
  bridge_all_wlan();
}
void bring_up_br()
{
	char tmpbuf[100];
	sprintf(tmpbuf, "%s %s hw ether %s up", IFCONFG, IF_BR,INBAND_MAC); 
	system(tmpbuf);

	/*
	sprintf(tmpbuf,"%s %s %s", IFCONFG, IF_BR, get_host_ip());
	system(tmpbuf);	*/  //mark_debug
}

void init_bridge()
{
	unsigned char addr[6];
	char tmpbuf[100];
	
#ifdef CONFIG_RTK_VLAN_SUPPORT
	sprintf(tmpbuf,"echo \"4\" > %s", "/proc/wan_port"); //mark_issue ,always set inband port to port4 ?
       system(tmpbuf);
	//mark_vlan
	sprintf(tmpbuf,"echo \"1\" > %s", "/proc/disable_l2_table");
       system(tmpbuf);
#endif

    sprintf(tmpbuf, "%s addbr %s", BR_UTL, IF_BR); 
	system(tmpbuf); 
    
	DISABLE_STP(IF_BR);

	bridge_all_if(); // lan if +  wlan if
}

static char *wps_get_token(char *data, char *token)
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

//////////////////////////////////////////////////////////////////////
static int wps_get_value(char *data, char *value)
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

static int notif_host(void)
{
	int i, ret,fd;
	int chan;
	unsigned int seq=0;
	char *data;		
	char *buf_p,rx_cmd_type;
	int count;
	struct stat ffstat;
	 int flen=0,len;

	chan = inband_open(INBAND_NETIF,"00e04c8196c1",ETH_P_RTK_NOTIFY,INBAND_DEBUG);

	if (chan < 0) {
		printf("open inband failed!\n");
		return -1;
	}

	fd = open(WPS_CONF, O_RDONLY);

  	if (fd < 0)	{
		printf("Cannot Open file %s!\n", WPS_CONF);
		return -1;
   	}
	fstat(fd, &ffstat);
 	flen = ffstat.st_size;
  	printf("flen = %d \n",flen);

  	if((data = (char *)malloc(flen)) == NULL)
  	{
		printf("data buffer allocation failed!\n");
		return -1;
	}
	
	ret = read(fd, data, flen);
   	
  	if (ret != flen) {
		printf("Reading error\n");
		free(data);	//need free before return!!!!!	
		return -1;
	}

 	close(fd);
	len = flen;		
	ret = inband_write(chan,0,0,data,len,0); //send request		
  	 
	free(data);	//need free before return!!!!!   

	count = inband_rcv_data(chan,&rx_cmd_type,&buf_p,3); //return data length

	if(count < 0)
	{
		ret=-1;	
		DEBUG_ERR("wps  notify fail\n");
	}	

	if( (rx_cmd_type == 0) )
		DEBUG_ERR("wps  notify ok!\n");
	else
	{
		ret=-1;
		DEBUG_ERR("wps  notify fail!\n");
	}		

	inband_close(chan);

	return 0;
}

int signal_daemon_by_name(unsigned char *name,unsigned int signo)
{
	struct stat status;
	unsigned char line[100];
	int pid;
	FILE *in=NULL;

	sprintf(line,"/var/run/%s.pid",name);
	if( stat(line,&status) == 0 ){
		in = fopen(line,"r");
		if( in )
			fgets(line,sizeof line,in);
		else
			goto out;
		sscanf(line,"%d",&pid);
		kill(pid,signo);
	}

out:
	if(in)
		fclose(in);

	return 0;
}

int kill_daemons(void)
{
	int i=0,index;
	unsigned char name[100], intf[30];
	struct stat ffstat;
	char tmpbuf[100];	
	unsigned char wpa_conf_name[8][30] = {{"/var/auth-wlan0.conf"},{"/var/auth-wlan0-va0.conf"},{"/var/auth-wlan0-va1.conf"},
						{"/var/auth-wlan0-va2.conf"},{"/var/auth-wlan0-va3.conf"},{"/var/auth-wlan0-va4.conf"},
						{"/var/auth-wlan0-va5.conf"},{"/var/auth-wlan0-va6.conf"}};

	//rm all auth conf
	for( index=0;index<8;index++ ) {
		if( !stat(wpa_conf_name[index],&ffstat) ) {
			sprintf(tmpbuf,"rm %s",wpa_conf_name[index]);
			system(tmpbuf);
		}
	}

	//rm fifo
	system("rm -f /var/*.fifo");

	signal_daemon_by_name("iwcontrol",SIGTERM);
	/* kill auth by-BSS */
	sprintf(intf,"wlan0");
	sprintf(name,"auth-%s",intf);
	signal_daemon_by_name(name,SIGTERM);

	for(i=0;i<MAX_WLAN_AP_INTF-1;i++){
		sprintf(intf,"wlan0-va%d",i);
		sprintf(name,"auth-%s",intf);
		signal_daemon_by_name(name,SIGTERM);
	}
	/* kill wscd */
	sprintf(intf,"wlan0");
	sprintf(name,"wscd-%s-wlan1",intf);
	signal_daemon_by_name(name,SIGTERM);
	sprintf(name,"wscd-%s",intf);
        signal_daemon_by_name(name,SIGTERM);
	sprintf(name,"wscd-wlan1");
        signal_daemon_by_name(name,SIGTERM);

	return 0;
}

int active_daemons(void)
{
	unsigned char line[100],intf[100]={0},conf_name[30]={"\0"};
	
	int i=0;
	if( !mib_all.wlan[0].legacy_flash_settings.wlanDisabled ) {
#ifdef RTK_WPS_BRIDGE_HACK //mark_wps
		if( !mib_all.wlan[0].legacy_flash_settings.wscDisable )
			start_wps();
#endif	
		if(is_8021x_mode(&mib_all.wlan[0].legacy_flash_settings)){
			//generate config
			sprintf(conf_name,"/var/auth-%s.conf",mib_all.wlan[0].name);
			if( generate_auth_conf(&mib_all.wlan[0].legacy_flash_settings,conf_name,0) < 0 )
				return -1;							
			strcat(intf," ");
			strcat(intf,mib_all.wlan[0].name);
			sprintf(line,"auth %s br0 auth /var/auth-%s.conf",mib_all.wlan[0].name,mib_all.wlan[0].name);
			system(line);
		}
			
		//for(i=1;i<MAX_WLAN_AP_INTF-1;i++){
		for( i=1; i< (get_vap_num()+1); i++){
			if( !mib_all.wlan[i].legacy_flash_settings.wlanDisabled ) {
				if(is_8021x_mode(&mib_all.wlan[i].legacy_flash_settings)){
				//generate config
				sprintf(conf_name,"/var/auth-%s.conf",mib_all.wlan[i].name);
				if( generate_auth_conf(&mib_all.wlan[i].legacy_flash_settings,conf_name,0) < 0 )
					return -1;
				strcat(intf," ");
				strcat(intf,mib_all.wlan[i].name);
				sprintf(line,"auth %s br0 auth /var/auth-%s.conf",mib_all.wlan[i].name,mib_all.wlan[i].name);
				system(line);
				}
			}
		}
		if( strlen(intf) > 0 ) {				
			sprintf(line,"iwcontrol%s",intf);
			system(line);
		}

	}

	return 0;
}

void init_system(int action)
{
	int index=0;
	char tmpbuf[100];	

	system("echo 0 > /proc/gpio"); //mark_issue , disable gpio anytime ?
    //kill daemon
    kill_daemons();	
	
	//mark_mac , now , we dont support lan reopen
	//bring_down_lan();	
	
	bring_down_wlan();	
	
	// init wlan driver
	if ((action & INIT_WLAN_PHY))
	{
		for(index=0; index<get_vap_num()+1; index++)
			initWlan(index,fecth_wlanmib(index),fecth_hwmib());

		if( get_sys_mode() == REPEATER_AP_MODE ) //mark_issue , to do Client mode??
			initWlan(WLAN_VXD_INDEX,fecth_wlanmib(WLAN_VXD_INDEX),fecth_hwmib());		
	}		
	//-----------------------------------------------
	init_vlan(); 	

	bring_up_wlan();

	sprintf(tmpbuf, "%s %s hw ether %s ", IFCONFG, IF_BR,INBAND_MAC); 
	system(tmpbuf);

#ifdef RTK_WPS_BRIDGE_HACK
	init_host_info();
#endif

	active_daemons();
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
	else if (num == 2 && !strcmp(t1, "set_mib")) {
		if(do_cmd(id_set_mib,t2,strlen(t2)+1, 0) < 0)
			DEBUG_ERR("set_mib failed : [%s]!\n", t2);	
		else//ok
			printf("set_mib ok: [%s]\n", t2);	
	}
	else if (num == 2 && !strcmp(t1, "get_mib")) { 
		if(do_cmd(id_get_mib,t2,strlen(t2)+1, 0) < 0)
			DEBUG_ERR("get_mib failed : [%s]!\n", t2);	
		else//ok
			printf("get_mib ok: [%s]\n", t2);
		//printf("get_mib [%s] :",t2);	
		//dump_mib( 0, t2);
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
		printf("\t-%s sys/p0/wlan0/wlan_comm xxx=xxx \n", "set_mib");
		printf("\t-%s sys/p0/wlan0/wlan_comm xxx \n", "get_mib");
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
	if (argv[argNum][0]=='-' && !strcmp(&argv[argNum][1], "set_mib")) {
		if (fp == NULL) 
			goto daemon_not_start;		
		if (argNum+2 > argc) {
			printf("Invalid format! [%s mib_name]\n", "set_mib");
			goto show_cfgwrite_help;
		}
		sprintf(tmpbuf, "echo %s %s > %s", "set_mib", argv[argNum+1], CMD_FILE);
	}
	else if (argv[argNum][0]=='-' && !strcmp(&argv[argNum][1], "get_mib")) {
		if (fp == NULL) 
			goto daemon_not_start;		
		if (argNum+2 > argc) {
			printf("Invalid format! [%s mib_name]\n", "get_mib");
			goto show_cfgread_help;
		}
		sprintf(tmpbuf, "echo %s %s > %s", "get_mib", argv[argNum+1], CMD_FILE);			
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
	else if (argv[argNum][0]=='-' && !strcmp(&argv[argNum][1], "notif_wps_config")) {
		if (fp == NULL) 
			goto daemon_not_start;
		if( notif_host() < 0 )
			printf("Event host failed!!\n");
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

static void inband_wait_event()
{
	int ret,data_len ;
	char cmd_type;		
	char *data_p;

	data_len = inband_rcv_data(hcd_inband_chan,&cmd_type,&data_p,-1); //try to rcv inband data 	

	if(data_len < 0)
		goto rcv_end;
	//do cmd process
	ret = do_cmd(cmd_type,data_p,data_len,1);  //do requested cmd , and reply a rsp_frame if need
	
rcv_end:	
	inband_free_buf(data_p, data_len);	
	return;
}

#ifdef RTK_NFBI_AP_HCM
int do_mdio_ioctl(int id, void *data)
{
                int ret = RET_OK;

                switch (id) {
#if 0
                        case id_cmd_timeout     :
                                if (ioctl(fd, MDIO_IOCTL_SET_CMD_TIMEOUT, data)) {
                                        DEBUG_ERR("Set ioctl MDIO_IOCTL_SET_CMD_TIMEOUT error!\n");
                                        ret = -RET_IOCTL_ERR;
                                }
                                break;
                        case id_mii_pause_enable:
                                if (ioctl(fd, MDIO_IOCTL_SET_MII_PAUSE, data)) {
                                        DEBUG_ERR("Set ioctl MDIO_IOCTL_SET_MII_PAUSE error!\n");
                                        ret = -RET_IOCTL_ERR;
                                }
                                break;
                        case id_eth_pause_enable:
                                if (ioctl(fd, MDIO_IOCTL_SET_ETH_PAUSE, data)) {
                                        DEBUG_ERR("Set ioctl MDIO_IOCTL_SET_ETH_PAUSE error!\n");
                                        ret = -RET_IOCTL_ERR;
                                }
                                break;
                        case id_cpu_suspend_enable:
                                if (ioctl(fd, MDIO_IOCTL_SET_SUSPEND, data)) {
                                        DEBUG_ERR("Set ioctl MDIO_IOCTL_SET_SUSPEND error!\n");
                                        ret = -RET_IOCTL_ERR;
                                }
                                break;
                        case id_phy_reg_poll_time:
                                if (ioctl(fd, MDIO_IOCTL_SET_PHY_POLL_TIME, data)) {
                                        DEBUG_ERR("Set ioctl MDIO_IOCTL_SET_PHY_POLL_TIME error!\n");
                                        ret = -RET_IOCTL_ERR;
                                }
                                break;
#endif
                        case id_set_host_pid:
                                if (ioctl(mdio_fd, MDIO_IOCTL_SET_HOST_PID, data)) {
                                        DEBUG_ERR("Set ioctl MDIO_IOCTL_SET_HOST_PID error!\n");
                                        ret = -RET_IOCTL_ERR;
                                }
                                break;
                        default:
                                DEBUG_ERR("Invalid mdio cmd id [0x%x] !\n", id);
                                ret = -RET_NOT_SUPPORT_NOW;
                }
                return ret;
}
#endif

int notify_host(int signo)
{
	struct iwreq wrq;
	DOT11_REQUEST *req;

  	/* Get wireless name */
	memset(wrq.ifr_ifrn.ifrn_name, 0, sizeof wrq.ifr_ifrn.ifrn_name);
	strncpy(wrq.ifr_ifrn.ifrn_name, "wlan0", IFNAMSIZ);

	req = (DOT11_REQUEST *)malloc(MAXDATALEN);
	wrq.u.data.pointer = (caddr_t)req;
	req->EventId = DOT11_EVENT_REQUEST;
	wrq.u.data.length = sizeof(DOT11_REQUEST);

	//iw_message(MESS_DBG_IWCONTROL, "[RequestIndication] : Start\n");
	//printf("\n[RequestIndication] : Start\n");
  	if(ioctl(ioctl_sock, SIOCGIWIND, &wrq) < 0)
	{
    	// If no wireless name : no wireless extensions
		return(-1);
	}

	inband_write(event_channel,0,0xff,wrq.u.data.pointer,wrq.u.data.length,0);
}

int main(int argc, char *argv[])
{ 	
	int pid_fd, pid, scr;
	char tmpbuf[100];
	int chan;
	/* INBAND_HOST ---> */
	struct iwreq wrq;
	/* INBAND_HOST <--- */

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

        // open mdio driver
#ifdef RTK_NFBI_AP_HCM
        mdio_fd = open(DEV_NAME, O_RDWR);
        if (!mdio_fd) {
                printf("open driver failed!\n");
                return 1;
        }
#endif
	  init_bridge();
	  bring_up_lan();
	  bring_up_br();
      sleep(1);
	   chan = inband_open(INBAND_NETIF,NULL,ETH_P_RTK,INBAND_DEBUG);
	   if(chan < 0)
	   {
	   	printf(" inband_open failed!\n");
	   	return -1;
	    }
	   else
	   	hcd_inband_chan = chan;	   

	// Set daemon pid to driver. 
	// When this ioctl is set, driver will set AllSoftwareReady bit to 'CPU System Status' Register
	pid =  getpid();
#ifdef RTK_NFBI_AP_HCM
	if (do_mdio_ioctl(id_set_host_pid, (void *)&pid)) //mark_issue , if it is need then mdio_open must be done 
		return 1;
#endif
	ioctl_sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (ioctl_sock < 0) {
		perror("socket[PF_INET,SOCK_DGRAM]");
		return -1;
	}

	/* INBAND_HOST ---> */
  	/* Get wireless name */
	memset(wrq.ifr_ifrn.ifrn_name, 0, sizeof wrq.ifr_ifrn.ifrn_name);
  	strncpy(wrq.ifr_ifrn.ifrn_name, "wlan0", IFNAMSIZ);

	pid = getpid();
	wrq.u.data.pointer = (caddr_t)&pid;
	wrq.u.data.length = sizeof(pid_t);

  	if(ioctl(ioctl_sock, SIOCSAPPPID, &wrq) < 0)
	{
    	// If no wireless name : no wireless extensions
		return(-1);
	}
	event_channel = inband_open(INBAND_INTF,INBAND_HOST,INBAND_EVENT_TYPE,INBAND_DEBUG);
	if( event_channel < 0 )
		return -1;
	signal(SIGIO, notify_host);
	/* INBAND_HOST <--- */

	//init_system(INIT_ALL); //mark_debug
	while(1){		
	inband_wait_event();   
	}	
}
