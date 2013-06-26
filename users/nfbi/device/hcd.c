/*
  *   Host Control Daemon for RTL8197B
  *
  *	Copyright (C)2008, Realtek Semiconductor Corp. All rights reserved.
  *
  *	$Id: hcd.c,v 1.22 2009/04/24 11:44:52 michael Exp $
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

#include "hcd.h"
#include "mib.h"
#include "../../../linux-2.6.30/drivers/char/rtl_mdio/rtl_mdio.h"


/*================================================================*/
/* Local Variables */

static int fd;
static struct mib cmd_param;
static unsigned int sys_ctrl_reg = 0;
static int is_sys_init = 0;
int is_wlan_found = 0;

/*================================================================*/
/* External Import */

#ifdef ACCESS_WLAN_IF
extern int set_wlan_mib(char *ifname, struct mib *phmib);
extern int get_interface_index(char *ifname);
extern int getStaConnectStatus(void);
#endif


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

static int pack_rsp_frame(int access, unsigned char cmd_id, int len, unsigned char *in, unsigned char *out)
{
	int i;
	out[0] = 0x80;
	if (access & ACCESS_MIB_BAD_CMD)
		out[0] |= 0x40;		
	out[1] = cmd_id;
	out[2] = 0x00;
	out[3] = (unsigned char)len;
	for (i=0; i<len; i++) {
		out[4+i*2] = 0x00;
		out[4+i*2+1] = in[i];
	}
	return (4+i*2);
}

static void write_data_to_driver(unsigned char *data, int len)
{
	if (len > MDIO_BUFSIZE) {
		printf("Write length > MDIO_BUFSIZE!\n");
		return;
	}
	write(fd, data, len);
}

static void process_host_cmd(unsigned char *data, int len)
{
	int id, access_flag = ACCESS_MIB_BY_ID | ACCESS_MIB_ACTION, i, ret;
	unsigned char cmd[1024], rsp_packet[1024];
	
	if (len < 4) {
		DEBUG_ERR("Invalid cmd len [%d] !\n", len);
		return;
	}	

	if (len != (data[3]*2+4)) {
		DEBUG_ERR("cmd length not matched [%d, %d] !\n", len, data[3]);
		return;
	}
	
	if (data[0] & 0x20) { // loopback command
		DEBUG_OUT("execute loopback command!\n");	
		write_data_to_driver(data, len);
		return;
	}

	if (len > sizeof(cmd)) {
		DEBUG_ERR("CMD length too big [%d]!\n", len);
		return;
	}
	memset(cmd, '\0', sizeof(cmd));

	if (data[0] & 0x40)  // get command
		access_flag |= ACCESS_MIB_GET;	
	else
		access_flag |= ACCESS_MIB_SET | ACCESS_MIB_ACTION;
		
	id = data[1];

	for (i=0; i<(len-4)/2; i++)
		cmd[i] = data[i*2+5];

	DEBUG_OUT("%s command, id=0x%x\n", (access_flag&ACCESS_MIB_SET) ? "SET" : "GET",  id);

#ifdef DB_MSG
	if (i > 0) {
		debug_out("cmd data", cmd, i);
	}
#endif	

	ret = access_mib(&cmd_param, access_flag, (char *)&id, cmd, NULL);
	if (ret >= 0) { // success
		if (ret == 0) { 
			cmd[0] = '\0';
			ret = 1;
		}
		len = pack_rsp_frame(access_flag, (unsigned char)id, ret, cmd, rsp_packet);		
		write_data_to_driver(rsp_packet, len);

#ifdef DB_MSG
		debug_out("rsp status", rsp_packet, len);
#endif		
	}
	else {
		DEBUG_ERR("access_mib return failed!\n");
		access_flag |= ACCESS_MIB_BAD_CMD;
		cmd[0] = (unsigned char)( ~ret + 1);			
		len = pack_rsp_frame(access_flag, (unsigned char)id, 1, cmd, rsp_packet);
		write_data_to_driver(rsp_packet, len);
	}	
}

static int sel_mii_clk(int param)
{
    if (ioctl(fd, MDIO_IOCTL_SET_MII_CLK, &param)) {
        printf("%s: MDIO_IOCTL_SET_MII_CLK error!\n", __FUNCTION__);
        return -1;
    }
    return 0;
}

static void init_system(int action, unsigned int scr)
{
	char tmpbuf[100];
	//int id;
	unsigned char addr[6];
#ifdef ACCESS_WLAN_IF	
//	unsigned char macclone_enabled;
#endif

	DEBUG_OUT("action=0x%x, scr=0x%x\n", action, scr);
    
#ifdef DB_MSG
	if (action & INIT_MII_MAC)
		DEBUG_OUT("Re-init MII [%s]\n",  (!(scr&CR_ISOLATION) ? "0->1" : "1->0"));
	
	if (action & INIT_MII_CLK)
		DEBUG_OUT("Re-init MII CLK to [%x]\n",  (scr&CR_SELMIICLK)>>9);
		    
	if (action & INIT_ETH_MAC)
		DEBUG_OUT("Re-init ETH mac [%s]\n",  ((scr&CR_ETHMAC) ? "0->1" : "1->0"));

	if (action & INIT_ETH_PHY)
		DEBUG_OUT("Re-init ETH phy [%s]\n",  ((scr&CR_ETHPHY) ? "0->1" : "1->0"));
	
	if (action & INIT_WLAN_MAC)
		DEBUG_OUT("Re-init WLAN mac [%s]\n",  ((scr&CR_WLANMAC) ? "0->1" : "1->0"));

	if (action & INIT_WLAN_PHY)
		DEBUG_OUT("Re-init WLAN phy [%s]\n",  ((scr&CR_WLANPHY) ? "0->1" : "1->0"));

	if (action & INIT_BR)
		DEBUG_OUT("Re-init bridge\n");	
#endif

	// shutdown interface	
	if (is_sys_init) {
		DEL_BR_INTERFACE(IF_BR, IF_ETH, 
											(action&INIT_ETH_MAC) && !(scr&CR_ETHMAC),
											(action&INIT_ETH_PHY) && !(scr&CR_ETHPHY));
	
		DEL_BR_INTERFACE(IF_BR, IF_MII,
											(action&INIT_MII_MAC) && (scr&CR_ISOLATION),
											0);	
#ifdef ACCESS_WLAN_IF
		DEL_BR_INTERFACE(IF_BR, IF_WLAN,
											(action&INIT_WLAN_MAC) && !(scr&CR_WLANMAC),
											(action&INIT_WLAN_PHY) && !(scr&CR_WLANPHY));
#endif
	
		if (action & INIT_BR)
			DEL_BR(IF_BR);
	}

	// add interface and start
#ifdef ACCESS_WLAN_IF
//	if ((action & INIT_WLAN_PHY) && (scr & CR_WLANPHY))			
//		set_wlan_mib(IF_WLAN, &cmd_param);
#endif

	if (action & INIT_BR) {
		ADD_BR(IF_BR);
		DISABLE_STP(IF_BR);
	}	
	//id = id_eth_mac_addr;
	//access_mib(&cmd_param, ACCESS_MIB_GET|ACCESS_MIB_BY_ID, (char *)&id, (void *)addr, NULL);
	memcpy(addr, ALL_ZERO_MAC_ADDR, 6);
	ADD_BR_INTERFACE(IF_BR, IF_ETH, addr,
											(action&INIT_ETH_MAC) && (scr&CR_ETHMAC),
											(action&INIT_ETH_PHY) && (scr&CR_ETHPHY));

	//memcpy(addr, "\x0\x0\x0\x0\x0\x97", 6);
	memcpy(addr, ALL_ZERO_MAC_ADDR, 6);
	ADD_BR_INTERFACE(IF_BR, IF_MII, addr,
											(action&INIT_MII_MAC) && !(scr&CR_ISOLATION),
											(action & INIT_BR));

    if (action&INIT_MII_CLK) {
        if ((scr&CR_SELMIICLK) == 0x0200) //01: 2.5MHz at 10M mode
            sel_mii_clk(1);
        else if ((scr&CR_SELMIICLK) == 0x0400) //10: 50MHz at Turbo-MII mode
            sel_mii_clk(2);
        else //00: 25MHz at 100M mode
            sel_mii_clk(0);
    }
        
#ifdef ACCESS_WLAN_IF
    /*
	id = id_macclone_enable;
	access_mib(&cmd_param, ACCESS_MIB_GET|ACCESS_MIB_BY_ID, (char *)&id, (void *)&macclone_enabled, NULL);
	if (!macclone_enabled) 
		access_mib(&cmd_param, ACCESS_MIB_GET|ACCESS_MIB_BY_ID, (char *)&id, (void *)addr, NULL);
	else
		memcpy(addr, ALL_ZERO_MAC_ADDR, 6);	
	ADD_BR_INTERFACE(IF_BR, IF_WLAN, addr,
    									    (action&INIT_WLAN_MAC) && (scr&CR_WLANMAC),
    										(action&INIT_WLAN_PHY) && (scr&CR_WLANPHY));
	*/
	if (is_wlan_found) {
    	ADD_BR_INTERFACE(IF_BR, IF_WLAN, ALL_ZERO_MAC_ADDR,
    											(action&INIT_WLAN_MAC) && (scr&CR_WLANMAC),
    											(action&INIT_WLAN_PHY) && (scr&CR_WLANPHY));
    }
#endif
}

void force_eth_port_power_down(int flag)
{
    int param;

    if (flag)
        param = 2; //on
    else
        param = 3; //off
    ioctl(fd, MDIO_IOCTL_PRIV_CMD, &param);
}

void changeWlanLinkStatus(int flag)
{
    static int status = -1;
    int param;

    if (flag) {
        param = 5; //up
        flag = 1;
    }
    else {
        param = 6; //down
        flag = 0;
    }
    if (status != flag) {
        ioctl(fd, MDIO_IOCTL_PRIV_CMD, &param);
        status = flag;
    }
}

static void process_host_sysctl(unsigned int scr)
{
	int init_flag = 0, param;

	if (!is_sys_init) 
		init_flag = INIT_ETH_MAC | INIT_ETH_PHY | INIT_MII_MAC | INIT_MII_CLK
							| INIT_WLAN_MAC |INIT_WLAN_PHY | INIT_BR;
	else {
		if ((scr & CR_ISOLATION) ^ (sys_ctrl_reg & CR_ISOLATION))
			init_flag |=	INIT_MII_MAC;

		if ((scr & CR_SELMIICLK) != (sys_ctrl_reg & CR_SELMIICLK))
			init_flag |=	INIT_MII_CLK;

		if ((scr & CR_ETHMAC) ^ (sys_ctrl_reg & CR_ETHMAC))
			init_flag |=	INIT_ETH_MAC;
			
		if ((scr & CR_ETHPHY) ^ (sys_ctrl_reg & CR_ETHPHY))
			init_flag |=	INIT_ETH_PHY;
	
		if ((scr & CR_WLANMAC) ^ (sys_ctrl_reg & CR_WLANMAC))
			init_flag |= INIT_WLAN_MAC;
		
		if ((scr & CR_WLANPHY) ^ (sys_ctrl_reg & CR_WLANPHY))
			init_flag |=	INIT_WLAN_PHY;
	}
	
	init_system(init_flag, scr);

	sys_ctrl_reg = scr;	
#ifndef CONFIG_RTK_VOIP_ETHERNET_DSP_IS_DSP
    force_eth_port_power_down(!(sys_ctrl_reg&CR_ETHPHY));
#endif
    
	if (!is_sys_init) {
	    //start poll timer
	    param = 1;
	    ioctl(fd, MDIO_IOCTL_PRIV_CMD, &param);
		is_sys_init = 1;
	}
}

static void host_event_handler(void)
{
	struct evt_msg evt;

	while (read(fd, &evt, sizeof(evt)) > 0) {
#ifdef DB_MSG		
		DEBUG_OUT("evt: %s, len=%d\n", (evt.id == IND_CMD_EV ? "CMD" : "SYSCTL") , evt.len);
		if (evt.len > 0) {
			debug_out("data", evt.buf, evt.len);			
		}
#endif		
		if (evt.id == IND_CMD_EV) 
			process_host_cmd(evt.buf, evt.len);			
		else if (evt.id == IND_SYSCTL_EV) 
			process_host_sysctl(*((unsigned int *)evt.buf));	
		else {
			DEBUG_ERR("Invalid evt id [%d]!\n", evt.id);					
		}
	}
}

static void event_handler(int sig_no)
{
#if 0
	struct evt_msg evt;

	while (read(fd, &evt, sizeof(evt)) > 0) {
#ifdef DB_MSG		
		DEBUG_OUT("evt: %s, len=%d\n", (evt.id == IND_CMD_EV ? "CMD" : "SYSCTL") , evt.len);
		if (evt.len > 0) {
			debug_out("data", evt.buf, evt.len);			
		}
#endif		
		if (evt.id == IND_CMD_EV) 
			process_host_cmd(evt.buf, evt.len);			
		else if (evt.id == IND_SYSCTL_EV) 
			process_host_sysctl(*((unsigned int *)evt.buf));	
		else {
			DEBUG_ERR("Invalid evt id [%d]!\n", evt.id);					
		}
	}
#endif
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

int do_mdio_ioctl(int id, void *data)
{
		int ret = RET_OK;
		
		switch (id) {
			case id_cmd_timeout	:
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
			case id_write_memory:
				if (ioctl(fd, MDIO_IOCTL_WRITE_MEM, data)) {
					DEBUG_ERR("Set ioctl MDIO_IOCTL_WRITE_MEM error!\n");
					ret = -RET_IOCTL_ERR;
				}
				break;				
			case id_read_memory:
				if (ioctl(fd, MDIO_IOCTL_READ_MEM, data)) {
					DEBUG_ERR("Set ioctl MDIO_IOCTL_READ_MEM error!\n");
					ret = -RET_IOCTL_ERR;
				}
				ret = sizeof(int);
				break;				
			case id_get_wlan_info:				
			case id_request_scan:
			case id_get_scan_result:
				DEBUG_ERR("Command [0x%x] not supported yet!\n", id);
				ret = -RET_NOT_SUPPORT_NOW;
				break;
			case id_set_host_pid:
				if (ioctl(fd, MDIO_IOCTL_SET_HOST_PID, data)) {
					DEBUG_ERR("Set ioctl MDIO_IOCTL_SET_HOST_PID error!\n");
					ret = -RET_IOCTL_ERR;
				}
				break;								
#ifdef JUMP_CMD
			case id_jump:
				if (ioctl(fd, MDIO_IOCTL_JUMP_ADDR, data)) {
					DEBUG_ERR("Set ioctl MDIO_IOCTL_JUMP_ADDR error!\n");
					ret = -RET_IOCTL_ERR;
				}
				break;								
#endif
			default:
				DEBUG_ERR("Invalid cmd id [0x%x] !\n", id);
				ret = -RET_NOT_SUPPORT_NOW;				
		}
		return ret;
}

static void manual_cmd_handler(int sig_no)
{
	FILE *fp;
	char line[100], *t1=NULL, *t2=NULL, *t3=NULL, *t4=NULL;
	int num;

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
		dump_all_mib(&cmd_param, ACT_MIB_RW, 1);

		printf("\n------ Read/Write MIB + IOCTL ------\n");		
		dump_all_mib(&cmd_param, ACT_MIB_RW_IOCTL, 1);

		printf("\n------ IOCTL ------\n");				
		dump_all_mib(&cmd_param, ACT_IOCTL, 0);		
	}
	else if (num == 2 && !strcmp(t1, SET_SCR)) {
		unsigned int scr;
		if (sscanf(t2, "%x", &scr) != 1) 
			DEBUG_ERR("Invalid value [%s] of %s cmd!\n", t2, SET_SCR);			
		else
			process_host_sysctl(scr);
	}		
	else if (num == 2 && !strcmp(t1, GET_MIB)) {
		if (access_mib(&cmd_param, ACCESS_MIB_GET|ACCESS_MIB_BY_NAME, t2, NULL, NULL) < 0)
			DEBUG_ERR("get mib failed [%s]!\n", t2);	
	}
	else if (num == 3 && !strcmp(t1, SET_MIB)) {
		if (access_mib(&cmd_param, ACCESS_MIB_SET|ACCESS_MIB_BY_NAME|ACCESS_MIB_ACTION, t2, t3, NULL) < 0)
			DEBUG_ERR("set mib failed [%s, %s]!\n", t2, t3);
	}
	else if (num == 3 && !strcmp(t1, ACT_REQ)) {
		if (access_mib(&cmd_param, ACCESS_MIB_SET|ACCESS_MIB_BY_NAME|ACCESS_MIB_ACTION, t2, t3, NULL) < 0) 
			DEBUG_ERR("request action failed [%s, %s]!\n", t2, t3);
	}
	else if (num == 4 && !strcmp(t1, ACT_REQ)) {
		if (access_mib(&cmd_param, ACCESS_MIB_SET|ACCESS_MIB_BY_NAME|ACCESS_MIB_ACTION, t2, t3, t4) < 0) 
			DEBUG_ERR("request action failed [%s, %s, %s]!\n", t2, t3, t4);		
	}
#ifdef JUMP_CMD
	else if (num == 2 && !strcmp(t1, JUMP_ADDR)) {
		unsigned int addr;
		if (sscanf(t2, "%x", &addr) != 1) 
			DEBUG_ERR("Invalid value [%s] of %s cmd!\n", t2, JUMP_ADDR);			
		else
			do_mdio_ioctl(id_jump, (void *)&addr);
	}		
#endif		
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
		printf("\t-%s name value\n", SET_MIB);
		printf("\t-%s name\n", GET_MIB);
		printf("\t-%s name value1 [value2]\n", ACT_REQ);		
		printf("\t-%s\n", DUMP_ALL_MIB);
		printf("\t-%s value\n", SET_SCR);
#ifdef JUMP_CMD
		printf("\t-%s value\n", JUMP_ADDR);
#endif
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
	
	if (argv[argNum][0]=='-' && !strcmp(&argv[argNum][1], SET_MIB)) {
		if (fp == NULL) 
			goto daemon_not_start;		
		if (argNum+3 > argc) {
			printf("Invalid format! [%s mib_name value]\n", SET_MIB);
			goto show_set_help;
		}
		sprintf(tmpbuf, "echo %s %s %s > %s", SET_MIB, argv[argNum+1], argv[argNum+2], CMD_FILE);
	}	
	else if (argv[argNum][0]=='-' && !strcmp(&argv[argNum][1], GET_MIB)) {
		if (fp == NULL) 
			goto daemon_not_start;		
		if (argNum+2 > argc) {
			printf("Invalid format! [%s mib_name]\n", GET_MIB);
			goto show_get_help;
		}
		sprintf(tmpbuf, "echo %s %s > %s", GET_MIB, argv[argNum+1], CMD_FILE);			
	}
	else if (argv[argNum][0]=='-' && !strcmp(&argv[argNum][1], ACT_REQ)) {
		if (fp == NULL) 
			goto daemon_not_start;		
		if (argNum+3 > argc) {
			goto show_act_help;
		}
		if (argc == argNum+3)
			sprintf(tmpbuf, "echo %s %s %s > %s", ACT_REQ, argv[argNum+1], argv[argNum+2], CMD_FILE);
		else		
			sprintf(tmpbuf, "echo %s %s %s %s > %s", ACT_REQ, argv[argNum+1], argv[argNum+2], argv[argNum+3], CMD_FILE);			
	}		
	else if (argv[argNum][0]=='-' && !strcmp(&argv[argNum][1], DUMP_ALL_MIB)) {
		if (fp == NULL) 
			goto daemon_not_start;		
		sprintf(tmpbuf, "echo %s > %s",  DUMP_ALL_MIB, CMD_FILE);			
	}
	else if (argv[argNum][0]=='-' && !strcmp(&argv[argNum][1], SET_SCR)) {
		if (fp == NULL) 
			goto daemon_not_start;		
		if (argNum+2 > argc) 
			goto show_set_scr_help;		
		sprintf(tmpbuf, "echo %s %s > %s", SET_SCR, argv[argNum+1], CMD_FILE);			
	}	
#ifdef JUMP_CMD
	else if (argv[argNum][0]=='-' && !strcmp(&argv[argNum][1], JUMP_ADDR)) {
		if (fp == NULL) 
			goto daemon_not_start;		
		if (argNum+2 > argc) 
			goto show_jump_help;		
		sprintf(tmpbuf, "echo %s %s > %s", JUMP_ADDR, argv[argNum+1], CMD_FILE);			
	}	
#endif	
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


show_set_help:
	printf("\n<mib_name>:\n");
	dump_all_mib(&cmd_param, ACT_MIB_RW, 0);	
	dump_all_mib(&cmd_param, ACT_MIB_RW_IOCTL, 0);
	goto ret;
	
show_get_help:
	printf(" \n<mib_name>:\n");
	dump_all_mib(&cmd_param, ACT_MIB_RW, 0);	
	goto ret;
	
show_set_scr_help:
	printf("%s scr_value. src_value in hex\n", SET_SCR);
	goto ret;

#ifdef JUMP_CMD
show_jump_help:
	printf("%s addr. addr in hex\n", JUMP_ADDR);
	goto ret;
#endif
	
show_act_help:
	printf("\n\"mib_name\" are defined below:\n");
	dump_all_mib(&cmd_param, ACT_IOCTL, 0);
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

#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP_IS_DSP
static void ethernet_dsp_set_mac(int sig_no)
{	
	// get DSP ID to set DSP MAC address
	unsigned int dsp_id;
	if (ioctl(fd, MDIO_IOCTL_READ_DSP_ID, &dsp_id)) {
		DEBUG_ERR("Get MDIO_IOCTL_READ_DSP_ID error!\n");
	}
	printf("ethernet_dsp_set_mac: 02E04C8972B%d (dsp_id = %d)\n", dsp_id, dsp_id);
	
	switch(dsp_id)
	{
		case 0:
			system("ifconfig eth0 hw ether 02E04C8972B0");
			break;
		case 1:
			system("ifconfig eth0 hw ether 02E04C8972B1");
			break;
		case 2:
			system("ifconfig eth0 hw ether 02E04C8972B2");
			break;
		case 3:
			system("ifconfig eth0 hw ether 02E04C8972B3");
			break;
		default:
			printf("not support dsp_id for ethernet_dsp_set_mac\n");
			break;
	}
}
#endif

int main(int argc, char *argv[])
{ 	
	int pid_fd, pid, scr;
	char tmpbuf[100];
#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP_IS_DSP
	int ignore_pause = 1;
#endif
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
	memset(&cmd_param, '\0', sizeof(cmd_param));
	assign_initial_value(&cmd_param);

	// register signal handler
	signal(SIGUSR1, event_handler);	
#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP_IS_DSP
	signal(SIGUSR2, ethernet_dsp_set_mac);
#else
#ifdef CMD_LINE
	signal(SIGUSR2, manual_cmd_handler);	
#endif
#endif
    signal(SIGCHLD, sigchld_handler);

	// open mdio driver
	fd = open(DEV_NAME, O_RDWR); 
	if (!fd) {
		printf("open driver failed!\n");
		return 1;
	}	
#ifndef CONFIG_RTK_VOIP_ETHERNET_DSP_IS_DSP
	// set initial value of mib to mdio driver
	set_init_mib_to_driver(&cmd_param, ACT_MIB_RW_IOCTL);
#endif

	// get SCR to init system
	if (ioctl(fd, MDIO_IOCTL_READ_SCR, &scr)) {
		DEBUG_ERR("Set MDIO_IOCTL_READ_SCR error!\n");
		return 1;
	}
#ifndef CONFIG_RTK_VOIP_ETHERNET_DSP_IS_DSP
	process_host_sysctl(scr);
#endif

	// Set daemon pid to driver. 
	// When this ioctl is set, driver will set AllSoftwareReady bit to 'CPU System Status' Register
	pid =  getpid();
	if (do_mdio_ioctl(id_set_host_pid, (void *)&pid)) 
		return 1;
	
	// pause daemon to wait signal 
	while (1) {
#ifdef ACCESS_WLAN_IF
		sleep(1);
		if (-1 != get_interface_index(IF_WLAN)) {
		    if (!is_wlan_found) {
		        real_sleep(1);
                ADD_BR_INTERFACE(IF_BR, IF_WLAN, ALL_ZERO_MAC_ADDR,
		                        (sys_ctrl_reg&CR_WLANMAC), (sys_ctrl_reg&CR_WLANPHY));
                if (!(sys_ctrl_reg&CR_WLANPHY)) {
                    //let ra0's mac address be available for host access
                    system("ifconfig ra0 up");
                    real_sleep(1);
                    system("ifconfig ra0 down");
                }
                is_wlan_found = 1;
		    }
		    if (getStaConnectStatus())
		        changeWlanLinkStatus(1);
		    else
		        changeWlanLinkStatus(0);
		}
		else {
		    if (is_wlan_found) {
		        is_wlan_found = 0;
        		DEL_BR_INTERFACE(IF_BR, IF_WLAN,
								(sys_ctrl_reg&CR_WLANMAC), (sys_ctrl_reg&CR_WLANPHY));
		        changeWlanLinkStatus(0);
		    }
		}
		host_event_handler();
#else
#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP_IS_DSP
		if( ignore_pause )
			ignore_pause = 0;
		else
#endif
        pause();
        host_event_handler();
#endif
	}
}
