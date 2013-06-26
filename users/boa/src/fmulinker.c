//#ifdef ROUTE_SUPPORT
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/reboot.h>
#include <unistd.h>
#include <net/route.h>
#include "boa.h"
#include "asp_page.h"
#include "apmib.h"
#include "apform.h"
#include "utility.h"

#ifdef HOME_GATEWAY


#if defined(CONFIG_RTL_ULINKER)

struct link_info {
	int  link; //0: no link, 1: eth, 2:wlan0, 3:wlan0-vxd
	bss_info bss;
};

struct link_info pri_link = {0}, cur_link = {0};
static int ulinker_domain_name_query_ready = 0;
static char domain_name_query_dns_ip[32];
static char dnrd_param[128];

static int prev_connect_state = -1;
static int triggered_connect_state = -1;


#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]

extern void set_lan_dhcpd(char *interface, int mode);
int kill_ppp(void)
{
#if defined(PPPOE_DISC_FLOW_PATCH)
			int sessid = 0;
			char cmdBuf[50],tmpBuff[30];
			int ppp_flag = 0;
			memset(tmpBuff,0, sizeof(tmpBuff));
			apmib_get(MIB_PPP_SESSION_NUM, (void *)&sessid);
			apmib_get(MIB_PPP_SERVER_MAC,  (void *)tmpBuff);
			apmib_get(MIB_PPP_NORMAL_FINISH,  (void *)&ppp_flag);	
			if(!ppp_flag) //1:nomal 0:abnormal
			{
				if(memcmp(tmpBuff, "\x00\x00\x00\x00\x00\x00", MAC_ADDR_LEN))
				{
					sprintf(cmdBuf,"flash clearppp %d %02x%02x%02x%02x%02x%02x",
						sessid,(unsigned char)tmpBuff[0],(unsigned char)tmpBuff[1],(unsigned char)tmpBuff[2],(unsigned char)tmpBuff[3],(unsigned char)tmpBuff[4],(unsigned char)tmpBuff[5]);
					system(cmdBuf);
					sessid = 0;
					memset(tmpBuff,0,sizeof(tmpBuff));
					apmib_set(MIB_PPP_SESSION_NUM, (void *)&sessid);
					apmib_set(MIB_PPP_SERVER_MAC, (void *)tmpBuff); 								
				}
			}		
			//system("flash set PPP_NORMAL_FINISH 0");		
			ppp_flag = 0 ;
			apmib_set(MIB_PPP_NORMAL_FINISH, (void *)&ppp_flag);		
			apmib_update(CURRENT_SETTING);
#else
			int sessid = 0;
			char cmdBuf[50],tmpBuff[30];
			memset(tmpBuff,0, sizeof(tmpBuff));
			apmib_get(MIB_PPP_SESSION_NUM, (void *)&sessid);
			apmib_get(MIB_PPP_SERVER_MAC,  (void *)tmpBuff);
	
			sprintf(cmdBuf,"flash clearppp %d %02x%02x%02x%02x%02x%02x",sessid,(unsigned char)tmpBuff[0],(unsigned char)tmpBuff[1],(unsigned char)tmpBuff[2],(unsigned char)tmpBuff[3],(unsigned char)tmpBuff[4],(unsigned char)tmpBuff[5]);
			system(cmdBuf);
			sleep(2);	// Wait util pppoe server reply PADT, then start pppoe dialing, otherwise pppoe server will reply PADS with PPPoE tags: Generic-Error.
#endif

	return 1;
}

static inline int wlan_up(void)
{
	system("ifconfig wlan0 up 2> /dev/null");

	if (pMib->wlan[0][NUM_VWLAN_INTERFACE].wlanDisabled == 0)
		system("ifconfig wlan0-vxd up 2> /dev/null");

	if (pMib->wlan[0][1].wlanDisabled == 0)
		system("ifconfig wlan0-va0 up 2> /dev/null");
	if (pMib->wlan[0][2].wlanDisabled == 0)
		system("ifconfig wlan0-va1 up 2> /dev/null");
	if (pMib->wlan[0][3].wlanDisabled == 0)
		system("ifconfig wlan0-va2 up 2> /dev/null");
	if (pMib->wlan[0][4].wlanDisabled == 0)
		system("ifconfig wlan0-va3 up 2> /dev/null");

	return 1;
}

static inline int ulinker_kill_wlan_applications(void)
{
	system("killall -9 wscd 2> /dev/null");
	system("killall -9 iwcontrol 2> /dev/null");
	system("killall -9 auth 2> /dev/null");
	system("killall -9 disc_server 2> /dev/null");
	system("killall -9 iapp 2> /dev/null");
	system("killall -9 mini_upnpd 2> /dev/null");
	return 1;
}

static inline int set_bridge_dhcp_filter(int val)
{
	/* set dhcp discover filter in bridge */
	char cmd_buf[100];
	struct sockaddr hwaddr;
	unsigned char *pMacAddr;
	unsigned long ipAddr ;
	memset(cmd_buf, 0x00, sizeof(cmd_buf));
	getInAddr("br0", IP_ADDR, (void *)&ipAddr);
	getInAddr("br0", HW_ADDR, (void *)&hwaddr);
	pMacAddr = hwaddr.sa_data;
	sprintf(cmd_buf, "echo \"%08X %02X%02X%02X%02X%02X%02X %d\" > /proc/pocket/filter_conf",
			ipAddr, pMacAddr[0], pMacAddr[1], pMacAddr[2], pMacAddr[3], pMacAddr[4], pMacAddr[5], val);
	system(cmd_buf);
	return 1;
}

int set_domain_name_query_ready(int val)
{
	int same_value = (ulinker_domain_name_query_ready == val?1:0);
	ulinker_domain_name_query_ready = val;

	if (val == 2) {
		system("rm -f /var/ulinker_reset_domain >/dev/null 2>&1");
		prev_connect_state = -1;
	}

	if (same_value == 0 && val == 1) {
		system_initial_led(0);
		prev_connect_state = -1;
	}

	return 0;
}

static void get_dns_server_ip(void)
{
	int dns_mode, get_dns_ip = 0;

	memset(domain_name_query_dns_ip, 0x00, sizeof(domain_name_query_dns_ip));
	memset(dnrd_param, 0x00, sizeof(dnrd_param));

	apmib_get(MIB_DNS_MODE, (void *)&dns_mode);
	if (dns_mode == DNS_MANUAL) /* Set DNS Manually */
	{
	#define NUM_OF_DNS_SERVER 3
		int i, dns = 0, dns_mib[NUM_OF_DNS_SERVER] = {MIB_DNS1, MIB_DNS2, MIB_DNS3};

		for (i=0;i<NUM_OF_DNS_SERVER;i++)
		{
			apmib_get(dns_mib[i], (void *)&dns);
			if (dns) {
				sprintf(domain_name_query_dns_ip, " -s %u.%u.%u.%u", NIPQUAD(dns));
				strcat(dnrd_param, domain_name_query_dns_ip);
				dns = 0;
				get_dns_ip = 1;
			}
		}
	#undef NUM_OF_DNS_SERVER
	}
	else {
		FILE *fp = NULL;
		int op_mode, wan_dhcp;
		
		apmib_get(MIB_OP_MODE, (void *)&op_mode);
		apmib_get(MIB_WAN_DHCP, (void *)&wan_dhcp);
		if (op_mode == GATEWAY_MODE && 
			(wan_dhcp == PPPOE || wan_dhcp == PPTP || wan_dhcp == L2TP)) /* for ppp interface*/
		{
			fp = fopen("/etc/ppp/resolv.conf", "r");
			if (fp != NULL) {
				char *ptr, *findstr = "nameserver ";		
				fgets(domain_name_query_dns_ip, sizeof(domain_name_query_dns_ip), fp);
				fclose(fp);	

				if (strlen(domain_name_query_dns_ip) != 0) {
					ptr = strtok(domain_name_query_dns_ip, findstr);
					strcpy(domain_name_query_dns_ip, ptr);					
					get_dns_ip = 1;
					sprintf(dnrd_param, " -s %s", domain_name_query_dns_ip);	
				}
			}			
		}
		else {
			fp = fopen("/var/ulinker_dns", "r");
			if (fp != NULL) {
				fgets(domain_name_query_dns_ip, sizeof(domain_name_query_dns_ip), fp);
				fclose(fp);
	
				if (strlen(domain_name_query_dns_ip) != 0) {
					get_dns_ip = 1;
					sprintf(dnrd_param, " -s %s", domain_name_query_dns_ip);	
				}
			}
		}
	}

	if (get_dns_ip == 0)
		sprintf(dnrd_param, " -s %s", "168.95.1.1");
}

static void ulinker_start_dnrd(int alwayshost)
{
	unsigned char cmdBuffer[100];
	unsigned char domanin_name[MAX_NAME_LEN];
	unsigned long ipAddr ;

	memset(cmdBuffer, 0x00, sizeof(cmdBuffer));
	apmib_get(MIB_DOMAIN_NAME, (void *)domanin_name);

	/* start dnrd */
	system("killall -9 dnrd 2> /dev/null");
	system("rm -f /var/hosts 2> /dev/null");
	getInAddr("br0", IP_ADDR, (void *)&ipAddr);
	if (alwayshost)
		sprintf(cmdBuffer, "%u.%u.%u.%u\\%s\n", NIPQUAD(ipAddr), "AlwaysHost");
	else
		sprintf(cmdBuffer, "%u.%u.%u.%u\\%s%s|%s%s\n", NIPQUAD(ipAddr), domanin_name, ".com", domanin_name, ".com.tw");
	write_line_to_file("/etc/hosts", 1, cmdBuffer);
#if 0
	system("dnrd --cache=off -s 168.95.1.1");
#else
	memset(cmdBuffer, 0x00, sizeof(cmdBuffer));
	get_dns_server_ip();
	sprintf(cmdBuffer, "dnrd --cache=off %s", dnrd_param);
	system(cmdBuffer);
#endif

	/* start nmbserver */
	memset(cmdBuffer, 0x00, sizeof(cmdBuffer));
	system("killall nmbserver > /dev/null 2>&1");
	sprintf(cmdBuffer,"nmbserver -D -R -n \"%s%s\"", domanin_name, ".com");
	system(cmdBuffer);

	set_domain_name_query_ready(1);
}

static int get_flag_value(char *file_name)
{
	FILE *fp = NULL;
	unsigned char tmp[3];
	int ret  = 0;
	int fail = 1;

	fp = fopen(file_name, "r");
	if (fp != NULL) {
		memset(tmp, 0x00, sizeof(tmp));
		fgets(tmp, sizeof(tmp), fp);
		fclose(fp);

		if (strlen(tmp) != 0) {
			tmp[1]='\0';
			ret = tmp[0] - '0';
			fail = 0;
		}
	}

	if (fail == 1) {
		fprintf(stderr, "get %s value fail!!!\n", file_name);
		ret = -1;
	}

	return ret;
}

static int get_wlan_active_stat(char *wlan_interface, struct link_info *info)
{
	FILE *fp = NULL;
	unsigned char wlan_stat[32];
	unsigned char buf[64];
	int active = 0;
	bss_info bss;
	static int inital = 0;
	static unsigned char priv_ssid[MAX_SSID_LEN] = {0};
		
	sprintf(wlan_stat, "/proc/%s/sta_info", wlan_interface);

	fp = fopen(wlan_stat, "r");
	if (fp != NULL) {
		memset(buf, 0x00, sizeof(buf));
		fgets(buf, sizeof(buf), fp);
		fclose(fp);

		sscanf(buf, "-- STA info table -- (active: %d)", &active);
	}

	if (active == 1) {
		getWlBssInfo(wlan_interface, &info->bss);
		if(info->bss.state == STATE_CONNECTED) {
			if (inital && strcmp(info->bss.ssid, priv_ssid)!=0) {
				fprintf(stderr, "---> cur[%s],pri[%s], diff SSID!!!!!!!!\n", info->bss.ssid, priv_ssid);//bruce
				active =0;
			}
			else {
				inital = 1;
			}

			memset(priv_ssid, 0x00, sizeof(priv_ssid));
			memcpy(priv_ssid, info->bss.ssid, strlen(info->bss.ssid));
		}
		else
			active = 0;
	}

	return active;
}


static get_ulinker_mode(void)
{
	int ulinker_mode = -1, ulinker_auto, rpt_enable, wlan_mode, op_mode;

	apmib_get(MIB_ULINKER_AUTO, (void *)&ulinker_auto);
	if (ulinker_auto == 1) {
		ulinker_mode = get_flag_value("/var/ulinker_mode_flag");
	}
	else {
		apmib_get(MIB_OP_MODE, (void *)&op_mode);
		apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
		apmib_get(MIB_REPEATER_ENABLED1, (void *)&rpt_enable);

		if (op_mode == BRIDGE_MODE)
		{
			if (wlan_mode == ULINKER_WL_CL)
				ulinker_mode = ULINKER_BR_CL_MODE;
			else if (wlan_mode == ULINKER_WL_AP && rpt_enable == 1) 
				ulinker_mode = ULINKER_BR_RPT_MODE;
			else if (wlan_mode == ULINKER_WL_AP)
				ulinker_mode = ULINKER_BR_AP_MODE;
			else 
				ulinker_mode = -1;

		}
	}

	return ulinker_mode;
}


static int check_up_stream_link(struct link_info *info, int val)
{
	int active = -1, ulinker_mode = -1, wan_link;

	ulinker_mode = get_ulinker_mode();

	if (ulinker_mode == ULINKER_BR_CL_MODE)	{
		active = get_wlan_active_stat("wlan0", info);//client mode
		info->link = ((active == 0)?0:2);
	}
	else if (ulinker_mode == ULINKER_BR_RPT_MODE) {
		active = get_wlan_active_stat("wlan0-vxd", info);//repeater mode
		info->link = ((active == 0)?0:3);
	}
	else if (ulinker_mode == ULINKER_BR_AP_MODE) {
		wan_link = getWanLink("eth1");//ap mode
		if (wan_link < 0)
			info->link = active = 0;
		else
			info->link = active = 1;
	}

	if (prev_connect_state == -1)
		prev_connect_state = active;

	return active;
}

static int ulinker_domain_name_query_br(void)
{
	FILE *fp = NULL;
	unsigned char tmp[10];
	int do_probe = 0, dhcp = 0, ulinker_auto, rpt_enable, wlan_mode, op_mode;
	int ulinker_mode, mac_clone, cloned_client = 0;	
 
	memset(tmp, 0x00, sizeof(tmp));

	int active = 0;
	int retry = 100;
	struct link_info info;

	while (retry>0 && active==0) {
		active = check_up_stream_link(&info, 0);
		retry--;
		usleep(10000*10); //10000 --> 1/1000sec, 10000*10 --> 1/100sec
	}

	if (active == 0) {
		system("echo 2 > /var/ulinker_auto_dhcp");
	}
	else {
		extern int dhcp_probe(void);
	
		apmib_get(MIB_ULINKER_AUTO, (void *)&ulinker_auto);
		apmib_get(MIB_OP_MODE, (void *)&op_mode);
		apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
		apmib_get(MIB_REPEATER_ENABLED1, (void *)&rpt_enable);
		apmib_get(MIB_WLAN_MACCLONE_ENABLED, (void *)&mac_clone);

		dhcp = dhcps_discover(6);
		if (dhcp <= 0) {
			bdbg_ulinker_domain_name_query(stderr, "[%s:%d]\n", __FUNC__, __LINE__);
			system("echo 2 > /var/ulinker_auto_dhcp");
		}
		else {
			bdbg_ulinker_domain_name_query(stderr, "[%s:%d]\n", __FUNC__, __LINE__);
			system("echo 1 > /var/ulinker_auto_dhcp");
		}

		if (op_mode == BRIDGE_MODE && wlan_mode == ULINKER_WL_CL && rpt_enable == 0 && mac_clone == 1) {
			bdbg_ulinker_domain_name_query(stderr, "[%s:%d]\n", __FUNC__, __LINE__);
			system("echo 1 > /var/ulinker_auto_dhcp");
			cloned_client = 1;
		}
	}

	fp = fopen(ULINKER_AUTO_DHCP, "r");
	if (fp != NULL)
	{
		fgets(tmp, sizeof(tmp), fp);
		fclose(fp);

		if (strlen(tmp) != 0)
		{
			unsigned long ipAddr, ipAddr2;
			getInAddr("br0", IP_ADDR, (void *)&ipAddr);
			apmib_get(MIB_IP_ADDR,	(void *)&ipAddr2);
			
			tmp[1]='\0';
			if (strcmp(tmp, "1") == 0 || cloned_client == 1) //if get ip, set domain name query
			{
				bdbg_ulinker_domain_name_query(stderr, 
					"[%s:%d] ULINKER_AUTO_DHCP=1, %s, set domain name query\n",
					__FUNC__, __LINE__, (cloned_client == 1?"cloned client":"get ip"));

				bdbg_ulinker_domain_name_query(stderr,
					"[%s:%d] ipAddr[0x%x][0x%x]\n", __FUNCTION__, __LINE__, ipAddr, ipAddr2);

				sleep(1);//delay for dhcpc bind ip to br0
				clean_auto_dhcp_flag();
				if (ipAddr == ipAddr2)
					disable_bridge_dhcp_filter();
				else {
					set_bridge_dhcp_filter(1);
					enable_bridge_dhcp_filter();
				}

				ulinker_start_dnrd(0);
				sleep(2);
				usb0_up();
				set_domain_name_query_ready(1);
			}
			else if(strcmp(tmp, "2") == 0) //else, enable dhcpd
			{
				bdbg_ulinker_domain_name_query(stderr, 
					"[%s:%d] ULINKER_AUTO_DHCP=2, can't get ip, enable dhcpd\n", __FUNC__, __LINE__);

				clean_auto_dhcp_flag();
				set_bridge_dhcp_filter(0);
				enable_bridge_dhcp_filter();
				stop_dhcpd();
				stop_dhcpc();
				set_lan_dhcpd("br0", 2);
			#if 0 // disable always host
				ulinker_start_dnrd(1);
			#else
				ulinker_start_dnrd(0);
			#endif
				sleep(1);
				usb0_up();
				set_domain_name_query_ready(1);
			}
		}
	}
	else {
		bdbg_ulinker_domain_name_query(stderr, 
					"[%s:%d] open /var/ulinker_auto_dhcp fail\n", __FUNC__, __LINE__);
	}

	return 0;
}

static int ulinker_domain_name_query_gw(void)
{
	FILE *fp = NULL;
	unsigned char tmp[10];
	int set = 0;

	int wan_mode;
	apmib_get(MIB_WAN_DHCP, (void *)&wan_mode);

	if (wan_mode == DHCP_DISABLED || wan_mode ==PPPOE || wan_mode==PPTP || wan_mode==L2TP) {
		set = 1;
	}
	else 
	{
		fp = fopen(ULINKER_AUTO_DHCP, "r");
		if (fp != NULL) {
			memset(tmp, 0x00, sizeof(tmp));
			fgets(tmp, sizeof(tmp), fp);
			fclose(fp);
		
			if (strlen(tmp) != 0) {
				tmp[1]='\0';
				if (strcmp(tmp, "0") == 0) {
					//do nothing
				}
				else { /* get ip, set domain name query */
					set = 1;
				}
			}
		}
	}

	set = 1;

	if (set == 1)
	{
		ulinker_start_dnrd(0);
		sleep(1);
		usb0_up();
		set_domain_name_query_ready(1);
	}

	return 0;
}

static int ulinker_domain_name_query(void)
{
	int op_mode;

	if (isFileExist(ULINKER_RESET_DOMAIN_NAME_QUERY)) {
		set_domain_name_query_ready(0);
		system("rm -f /var/ulinker_reset_domain >/dev/null 2>&1");
	}

	if (ulinker_domain_name_query_ready >= 1)
		return 0;

	apmib_get(MIB_OP_MODE, (void *)&op_mode);

	if (op_mode == BRIDGE_MODE)
		return ulinker_domain_name_query_br();
	else if (op_mode == GATEWAY_MODE || op_mode == WISP_MODE)
		return ulinker_domain_name_query_gw();
	else
		fprintf(stderr, "[%s:%d] unknow mode!!\n", __FUNC__, __LINE__);

	return 0;
}

int wlan_down_sleep(int sec)
{
	#if 1
		sleep(sec);
	#else
		int i = sec;
		while (i>0)
		{
			printf("sleep %d\n", i);
			sleep(1);
			i--;
		}
	#endif

	return 0;
}

static int dhcps_discover(int val)
{
	int dhcps_exist = 0;
	int round = val;

	while (round > 0) {
		system("udhcpc -d 2 -i br0 -p /etc/udhcpc/udhcpc-br0-test.pid");
		dhcps_exist = get_flag_value(ULINKER_DHCPS_DISCOVER_FLAG);
		round--;
		if (dhcps_exist == 3) {
			dhcps_exist = 1;
			break;
		}
		else {
			dhcps_exist = 0;
		}
	}

	system("rm -f /etc/udhcpc/udhcpc-br0-test.pid >/dev/null 2>&1");
	clean_dhcps_discover();

	return dhcps_exist;
}

static int time_to_check_upstream = 0;
int ulinker_wlan_process()
{
	FILE *fp = NULL;
	char buf[64];

#if 0
#define bdbg_ulinker_wlan_process(format, arg...)		\
		fprintf(format , ## arg)
#else
#define bdbg_ulinker_wlan_process(format, arg...)
#endif

	if (ulinker_domain_name_query_ready ==1 && time_to_check_upstream > 2)
	{
		int active = 0, retry = 1, alwayshost, op_mode;
		int sleep_sec = 6, dut_get_ip = 1;

		time_to_check_upstream = 0;	

		if (triggered_connect_state != -1)
		{
		bdbg_ulinker_wlan_process(stderr, "connect_state[prev:%d, triggered:%d]!!\n", prev_connect_state, triggered_connect_state);
			prev_connect_state = triggered_connect_state;
			triggered_connect_state = -1;
		}
		
		apmib_get(MIB_OP_MODE, (void *)&op_mode);
		if (op_mode == BRIDGE_MODE) 
		{
			int dhcpd_running = -1;
			unsigned long ipAddr, ipAddr2;
			char cmd[64];

			getInAddr("br0", IP_ADDR, (void *)&ipAddr);
			apmib_get(MIB_IP_ADDR,	(void *)&ipAddr2);

			while (retry>0 && active==0) {
				active = check_up_stream_link(&cur_link, 1);
				retry--;
				usleep(10000*10); //10000 --> 1/1000sec, 10000*10 --> 1/100sec
			}

			if(prev_connect_state - active > 0) {
				dhcpd_running = 0;
			}
			else if(prev_connect_state - active < 0) {
				dhcpd_running = 1;
			}
			else if (active && ipAddr == ipAddr2) {
				bdbg_ulinker_wlan_process(stderr, "br0_IP: %u.%u.%u.%u, mib_lan: %u.%u.%u.%u, and dhcpd running\n", NIPQUAD(ipAddr), NIPQUAD(ipAddr2));//debug
				//dhcpd_running = 1;
				//dut_get_ip = 0;
			}
			else {
				dhcpd_running = -1;
			}

			prev_connect_state = active;

			if (dhcpd_running == 1)
			{
				int cloned_client = 0, dhxps_exist = 0, wlan_mode, rpt_enable, mac_clone;

				apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
				apmib_get(MIB_REPEATER_ENABLED1, (void *)&rpt_enable);
				apmib_get(MIB_WLAN_MACCLONE_ENABLED, (void *)&mac_clone);
				
				if (wlan_mode == ULINKER_WL_CL) {
					if (rpt_enable == 0 && mac_clone == 1)
						cloned_client = 1;
				}

				if (cloned_client == 1)
					dhxps_exist = 1;
				else {
					dhxps_exist = dhcps_discover(6);
				}

				if (dhxps_exist == 1) {
					triggered_connect_state = active;
					bdbg_ulinker_wlan_process(stderr, 
						"===> 1. active[%d], alwayshost[%d], dhxps_exist[%d], stop dhcpd, enable dhcpc\n", active, alwayshost, dhxps_exist);
					system_initial_led(1);
					disable_bridge_dhcp_filter();
					sprintf(cmd, "ifconfig br0 %u.%u.%u.%u >/dev/null 2>&1", NIPQUAD(ipAddr2));
					system(cmd);
					usb0_down();
					wlan_down();
					stop_dhcpc();
					stop_dhcpd();
					clean_auto_dhcp_flag();
					clean_up_layer_dns_flag();
					wlan_down_sleep(sleep_sec);
					reset_domain_query();
					set_lan_dhcpc("br0");
					wlan_up();
				}
				else {
					bdbg_ulinker_wlan_process(stderr, 
						"===> 3. active[%d], alwayshost[%d], dhxps_exist[%d], no dhcp server, do nothing\n", active, alwayshost, dhxps_exist);
				}
			}
			else if (dhcpd_running == 0)
			{
				triggered_connect_state = active;
				bdbg_ulinker_wlan_process(stderr, 
					"===> 2. active[%d], alwayshost[%d], stop dhcpc, enable dhcpd\n", active, alwayshost);
				system_initial_led(1);
				disable_bridge_dhcp_filter();
				sprintf(cmd, "ifconfig br0 %u.%u.%u.%u >/dev/null 2>&1", NIPQUAD(ipAddr2));
				system(cmd);
				usb0_down();
				wlan_down();
				stop_dhcpc();
				stop_dhcpd();
				clean_auto_dhcp_flag();
				clean_up_layer_dns_flag();
				wlan_down_sleep(sleep_sec);
				reset_domain_query();
				wlan_up();
			}
			else {
				bdbg_ulinker_wlan_process(stderr, 
					"===> 4. active[%d], alwayshost[%d], do nothing\n", active, alwayshost);
			}
		
		}
	}

	time_to_check_upstream++;
	
#undef bdbg_ulinker_wlan_process

	return 0;
}

int reload_in_progress(void)
{
	FILE *fp;
	int i = 0;

	fp=fopen("/proc/load_default","r");
	if(fp) {
		fscanf(fp,"%d",&i);
		fclose(fp); 
	}
	return i;
}


int firmware_upgrading = 0;
int ulinker_process()
{
	static int wait_reinit = 0;
	//static int switching = 0;

	static int pre_status = -1;
	static int nxt_status = -1;
	static int cur_status = -1;
	static int switched   = 0;
	static int running	  = 0;

	int ulinker_auto;

	int rndis_reset = get_flag_value("/proc/rndis_reset");
	static int set_rndis_reset = 0;

	if (rndis_reset == 2) {
		sleep(3);
		set_rndis_reset = 0;
		system("echo 0 > /proc/rndis_reset");
		system("ifconfig usb0 up");
	} else if (rndis_reset == 1 && set_rndis_reset%2 == 0) {
		set_rndis_reset++;
		system("ifconfig usb0 down");
		sleep(1);
		system("ew 0xb8030804 0x2000000");
		sleep(2);
		system("ew 0xb8030804 0x0");
	}

	if (reload_in_progress() > 0)
		return 0;

	if(wait_reinit == 0 && firmware_upgrading == 0)
	{
		ulinker_domain_name_query();
		ulinker_wlan_process();

		if (ulinker_domain_name_query_ready == 1) {	
			apmib_get(MIB_ULINKER_AUTO, (void *)&ulinker_auto);
			if (ulinker_auto == 0) {
				pre_status = -1;
				nxt_status = -1;
				cur_status = -1;
				switched   = 0;
				running	  = 0;
			}
			else 
			{
				int wan_link = getWanLink("eth1");

				if (wan_link <0)
					cur_status = 0;
				else
					cur_status = 1;

				if (running == 0)
				{
					if (pre_status == -1)
					{
						pre_status = cur_status;
					}
					else if (pre_status != cur_status || switched) {
						if (switched == 0) {
							switched =1;
							nxt_status = cur_status;
						}
						else {
							if (cur_status == nxt_status)
								switched++;
							else {
								switched = 0;
							}

							if (switched > 2) {
								bdbg_ulinker_domain_name_query(stderr, 
									"ulinker_process: change link mode [%d->%d]\n", pre_status, cur_status);
								running  = 1;
								switched = 0;
								pre_status = cur_status;
								nxt_status = -1;
							}
						}
					}
					else {
						pre_status = cur_status;
					}
				}

				if (running == 1)
				{
					int mode;
					running = 0;

					mode = get_flag_value("/var/ulinker_mode_flag");
					if (mode == ULINKER_RT_PPPOE_MODE) {
						kill_ppp();
					}
					#if 0
					if (cur_status == 0 && mode == ULINKER_BR_CL_MODE)
						return 0;
					else if (cur_status == 1 && mode != ULINKER_BR_CL_MODE)
						return 0;
					#endif

					set_domain_name_query_ready(2);
					stop_dhcpc();
					stop_dhcpd();
					kill_daemons();
				#if 1
					system("ifconfig wlan0 down >/dev/null 2>&1");
					system("ifconfig br0 0.0.0.0 >/dev/null 2>&1");
				#endif
					usb0_down();
					wlan_down();
					clean_auto_dhcp_flag();
					disable_bridge_dhcp_filter();
					system_initial_led(1);			
					system("init.sh gw all");
					wait_reinit = 5;
				}
			}
		}
	}

	if (wait_reinit > 0) {
		wait_reinit--;
		return 0;
	}

	return 0;
}

#endif /* #if defined(CONFIG_RTL_ULINKER) */


#endif /* #ifdef HOME_GATEWAY */

//#endif //ROUTE_SUPPORT
