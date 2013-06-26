/*
 *
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
/* Local include files */
#include "apmib.h"
#include "mibtbl.h"

#include "sysconf.h"
#include "sys_utility.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <net/if.h>
#include <stddef.h>		/* offsetof */
#include <net/if_arp.h>
#include <linux/if_ether.h>


#if defined(CONFIG_RTL_ULINKER)
#include <sys/ioctl.h>
#include <net/if.h>

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

static inline int get_mib(char *val, char *mib)
{
	FILE *fp;
 	char buf[64];

	sprintf(buf, "flash get %s", mib);
    fp = popen(buf, "r");
	if (fp==NULL)
		return -1;

	if (NULL == fgets(buf, sizeof(buf),fp)) {
		pclose(fp);
		return -1;
	}
	pclose(fp);

	strcpy(val, strstr(buf, "=")+1);
	return 0;
}

static inline int set_eth1_mac(void)
{
	char *mac = calloc(sizeof(char), 20);
	char *buf = calloc(sizeof(char), 64);

	/* set eth1 mac */
#if 0 //fixme
	apmib_get(MIB_HW_NIC1_ADDR, (void *)mac);
#else
	get_mib(mac, "HW_NIC1_ADDR");
#endif
	sprintf(buf, "ifconfig eth1 hw ether %s", mac);
	system(buf);

	if (mac) free(mac);
	if (buf) free(buf);
	return 1;
}

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

static inline int bdbg_show_ulinker_mode(int mode)
{
#if BDBG_ULINKER_DECIDE_DUT_MODE
	switch (mode) {
	case ULINKER_BR_AP_MODE:
		fprintf(stderr, "[%s] ULINKER_BR_AP_MODE\n", __FUNC__); break;
	case ULINKER_BR_CL_MODE:
		fprintf(stderr, "[%s] ULINKER_BR_CL_MODE\n", __FUNC__); break;
	case ULINKER_BR_USB_MODE:
		//system("ifconfig wlan0 down");
		fprintf(stderr, "[%s] ULINKER_BR_USB_MODE\n", __FUNC__); break;
	case ULINKER_RT_DHCP_MODE:
		fprintf(stderr, "[%s] ULINKER_RT_DHCP_MODE\n", __FUNC__); break;
	case ULINKER_RT_PPPOE_MODE:
		fprintf(stderr, "[%s] ULINKER_RT_PPPOE_MODE\n", __FUNC__); break;
	case ULINKER_BR_RPT_MODE:
		fprintf(stderr, "[%s] ULINKER_BR_RPT_MODE\n", __FUNC__); break;
	default:
		fprintf(stderr, "[%s] undefined!\n", __FUNC__); break;
	}
#else
	switch (mode) {
	case ULINKER_BR_AP_MODE:
		fprintf(stderr, "==> AP\n"); break;
	case ULINKER_BR_CL_MODE:
		fprintf(stderr, "==> client\n"); break;
	case ULINKER_RT_DHCP_MODE:
		fprintf(stderr, "==> dhcp router\n"); break;
	case ULINKER_RT_PPPOE_MODE:
		fprintf(stderr, "==> pppoe router\n"); break;
	case ULINKER_BR_RPT_MODE:
		fprintf(stderr, "==> repeater\n"); break;		
	default:
		fprintf(stderr, "undefined!\n"); break;
	}
#endif
	return 1;
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

static int ulinker_check_cdc_rndis_status(void)
{
	int ret = 1;

	bdbg_rndis_check(stderr, "\n[%s] checking...\n", __FUNC__, ret);

	ret = get_flag_value(ULINKER_CDC_RNDIS_STATUS_FLAG);

	bdbg_rndis_check(stderr, "[%s] rndis status[%d]\n", __FUNC__, ret);

	return ret;
}

#define PING_COUNT 3
#define PING_TARGET "8.8.8.8"
//#define PING_TARGET "168.95.1.1"
int ping_test(int count)
{
	FILE *fp;
 	char buf[128];
 	int ping_try, ping_success;

	bdbg_ping_test(stderr, "\n[%s] ping...\n", __FUNC__);

	sprintf(buf, "ping %s -w %d | grep \"packets received\"", PING_TARGET, count);
    fp = popen(buf, "r");
	if (fp==NULL)
		return -1;

	if (NULL == fgets(buf, sizeof(buf),fp)) {
		pclose(fp);
		return -1;
	}
	pclose(fp);

	sscanf(buf, "%d packets transmitted, %d packets received",
		&ping_try, &ping_success);

	bdbg_ping_test(stderr,
		"[%s] buf[%s], ping_try[%d], ping_success[%d]\n",
		__FUNC__, buf, ping_try, ping_success);

	return ping_success;
}

/* check any existed dhcp server  */
#define DISCOVER_DURATION 3
static int dhcps_discover(int val)
{
	int dhcps_exist = 0;
	int round = val;
	int mode;
	int ping_ok;

	int op_ori, wan_ori, dhcp_ori, wlan_ori;
	int op_mode, wan_mode, dhcp_mode, wlan_mode;

	bdbg_dhcps_discover(stderr, "\n[%s] dhcp server discovering...\n", __FUNC__);

	system("brctl delif br0 eth1 >/dev/null 2>&1");

	/* backup current mode */
	apmib_get(MIB_OP_MODE, (void *)&op_ori);
	apmib_get(MIB_WAN_DHCP, (void *)&wan_ori);
	apmib_get(MIB_DHCP, (void *)&dhcp_ori);
	apmib_get(MIB_WLAN_MODE, (void *)&wlan_ori);

	/* set dut to gateway mode for execute dhcp server discover*/
	op_mode   = GATEWAY_MODE;
	wan_mode  = DHCP_CLIENT;
	dhcp_mode = DHCP_SERVER;
	wlan_mode = AP_MODE;

	apmib_set(MIB_OP_MODE, (void *)&op_mode);
	apmib_set(MIB_WAN_DHCP, (void *)&wan_mode);
	apmib_set(MIB_DHCP, (void *)&dhcp_mode);
	apmib_set(MIB_WLAN_MODE, (void *)&wlan_mode);

	clean_dhcps_discover();
	set_eth1_mac();
	eth1_up();

	while (round > 0) {
		stop_dhcpc();
		system("udhcpc -d 2 -i eth1 -p /etc/udhcpc/udhcpc-eth1.pid -s /usr/share/udhcpc/br0.bound >/dev/null 2>&1");
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

	/* restore mode */
	apmib_set(MIB_OP_MODE, (void *)&op_ori);
	apmib_set(MIB_WAN_DHCP, (void *)&wan_ori);
	apmib_set(MIB_DHCP, (void *)&dhcp_ori);
	apmib_set(MIB_WLAN_MODE, (void *)&wlan_ori);

	stop_dhcpc();
	//eth1_down();
	clean_dhcps_discover();

	bdbg_dhcps_discover(stderr, "[%s] dhcp server result[%d]\n", __FUNC__, dhcps_exist);

	return dhcps_exist;
}

static int ulinker_get_decided_mode()
{
	int mode;
	int auto_wan;
	apmib_get(MIB_ULINKER_AUTO, (void *)&auto_wan);

	if (auto_wan == 1)
		mode = get_flag_value(ULINKER_MODE_FLAG);
	else {
	#if 0
		int op_mode, wan_mode, dhcp_mode, wlan_mode;
		apmib_get(MIB_OP_MODE, (void *)&op_mode);
		apmib_get(MIB_WAN_DHCP, (void *)&wan_mode);
		apmib_get(MIB_DHCP, (void *)&dhcp_mode);
		apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);

		if (op_mode == 0) {
			mode = ULINKER_RT_DHCP_MODE:
			mode = ULINKER_RT_PPPOE_MODE:
		}
		else if (op_mode == 1) {
			mode = ULINKER_BR_AP_MODE:
			mode = ULINKER_BR_CL_MODE;
		}
	#endif
	}

	return mode;
}

static int ulinker_get_decided_wlan_mode()
{
	int mode, wlan_mode;

	mode = get_flag_value(ULINKER_MODE_FLAG);

	switch (mode)
	{
		case ULINKER_BR_CL_MODE:
		case ULINKER_BR_USB_MODE:
			wlan_mode = ULINKER_WL_CL;
			break;
		case ULINKER_BR_AP_MODE:
		case ULINKER_RT_DHCP_MODE:
		case ULINKER_RT_PPPOE_MODE:
			wlan_mode = ULINKER_WL_AP;
			break;
		default:
			fprintf(stderr, "[%s] undefined mode[%d]!!\n", __FUNC__, mode);
			break;
	}

	return wlan_mode;
}

static int ulinker_decide_dut_mode()
{
	int eth1_link, cdc_rndis_status=-1, ping_ok=-1, dhcps_exist=-1;
	int mode = 0;

	bdbg_decide_dut_mode(stderr, "\n[%s] dut mode deciding...\n", __FUNC__);

	/* check eth link */
	eth1_link = getWanLink("eth1");
	if (eth1_link < 0)
		eth1_link = 0;
	else
		eth1_link = 1;
	if (eth1_link) mode |= 2;

	/* check cdc/rndis status */
	cdc_rndis_status = ulinker_check_cdc_rndis_status();
	if (cdc_rndis_status) mode |= 1;

	/* change usb2eth to autowan mode by spec */
	if (mode == ULINKER_BR_USB_MODE)
		mode = ULINKER_AUTO_WAN_MODE;

  #if AUTO_DETECT_RPEATER//defined(UNIVERSAL_REPEATER)
	/* change ULINKER_BR_AP_MODE(wall&no_eth) to repeater mode by spec */
	if (mode == ULINKER_BR_AP_MODE)
		mode = ULINKER_BR_RPT_MODE;
  #endif

	bdbg_decide_dut_mode(stderr,
		"[%s] mode[%d], eth1_link[%d], cdc_rndis_status[%d]\n",
		__FUNC__, mode, eth1_link, cdc_rndis_status);

	if (mode == ULINKER_AUTO_WAN_MODE) {	
		dhcps_exist = dhcps_discover(DISCOVER_DURATION);

		if (dhcps_exist == 0) {
			extern int  discovery_ppp(char *);
			int wan_mode, op_mode;
			apmib_get(MIB_OP_MODE,(void *)&op_mode);
			apmib_get(MIB_WAN_DHCP,(void *)&wan_mode);
			if(wan_mode == PPPOE && op_mode == GATEWAY_MODE)
				kill_ppp();

			if(discovery_ppp("eth1") == 1)
				mode = ULINKER_RT_PPPOE_MODE;
			else
				mode = ULINKER_BR_AP_MODE;
		}
		else if (dhcps_exist == 1) {
			mode = ULINKER_RT_DHCP_MODE;
			ping_ok = 1;
		}
		//else if (dhcps_exist == 2)
			//mode = ULINKER_BR_AP_MODE;
		else
			fprintf(stderr, "[%s]\n", __FUNC__);
	}

	export_decided_mode(mode);

	bdbg_show_ulinker_mode(mode);
	bdbg_decide_dut_mode(stderr,
		"==> mode[%d], eth1_link[%d], cdc_rndis_status[%d], dhcps[%d], ping[%d]\n",
		mode, eth1_link, cdc_rndis_status, dhcps_exist, ping_ok);
	bdbg_decide_dut_mode(stderr, "[%s] dut mode decided, done.\n", __FUNC__);

	return mode;
}

static int ulinker_wlan_mib_copy(CONFIG_WLAN_SETTING_T* dst, CONFIG_WLAN_SETTING_T* src)
{

#if CONFIG_APMIB_SHARED_MEMORY == 1
	if (apmib_sem_lock() != 0) return -1;
#endif

	memcpy(dst, src, sizeof(CONFIG_WLAN_SETTING_T));

#if CONFIG_APMIB_SHARED_MEMORY == 1
	if (apmib_sem_unlock() != 0) return -1;
#endif

	return 0;
}

static inline int debug_ulinker_swap_ap_cl_mib(int wlan_mode)
{
#if BDBG_ULINKER_SWAP_AP_CL_MIB
	fprintf(stderr, "  ==> wlan_mode[%d]\n"
	"\troot_wl_mode[%d], root_ssid[%s], root_wl_dis[%d]\n"
	"\tap_wl_mode[%d],	 ap_ssid[%s], ap_wl_dis[%d]\n"
	"\tcl_wl_mode[%d],	 cl_ssid[%s], cl_wl_dis[%d]\n",
		wlan_mode,
		pMib->wlan[0][0].wlanMode, pMib->wlan[0][0].ssid, pMib->wlan[0][0].wlanDisabled,
		pMib->wlan[0][ULINKER_AP_MIB].wlanMode, pMib->wlan[0][ULINKER_AP_MIB].ssid, pMib->wlan[0][ULINKER_AP_MIB].wlanDisabled,
		pMib->wlan[0][ULINKER_CL_MIB].wlanMode, pMib->wlan[0][ULINKER_CL_MIB].ssid, pMib->wlan[0][ULINKER_CL_MIB].wlanDisabled);
		
  #if defined(UNIVERSAL_REPEATER)
	fprintf(stderr, "\trpt_wl_mode[%d],	 rpt_ssid[%s], rpt_wl_dis[%d]\n",
		pMib->wlan[0][ULINKER_RPT_MIB].wlanMode, pMib->wlan[0][ULINKER_RPT_MIB].ssid, pMib->wlan[0][ULINKER_RPT_MIB].wlanDisabled);
  #endif		
#endif

	return 1;
}

static int ulinker_swap_ap_cl_mib(int wlan_mode)
{
	int has_set = 0;
	int pre_mode;
	int cur_mode;
	int wl_disable = 0;

	int auto_wan;
	apmib_get(MIB_ULINKER_AUTO, (void *)&auto_wan);

	bdbg_swap_ap_cl_mib(stderr, "\n[%s] should swap ap cl mib? checking...\n", __FUNC__);

	
	if (auto_wan == 1) {
		/* auto_mode, wlan should always enable! */
		if (wlan_mode == ULINKER_WL_CL) {
			if (pMib->wlan[0][ULINKER_CL_MIB].wlanDisabled == 1) {
				has_set = 1;
				pMib->wlan[0][ULINKER_CL_MIB].wlanDisabled = 0;
			}
		}
		else if (wlan_mode == ULINKER_WL_AP) {
			if (pMib->wlan[0][ULINKER_AP_MIB].wlanDisabled == 1) {
				has_set = 1;
				pMib->wlan[0][ULINKER_AP_MIB].wlanDisabled = 0;
			}
	
			if (pMib->repeaterEnabled1 == 1)
			{
				if (pMib->wlan[0][ULINKER_RPT_MIB].wlanDisabled == 1) {
					has_set = 1;
					pMib->wlan[0][ULINKER_RPT_MIB].wlanDisabled = 0;
				}
			}
		}
	}

	/* get mib from corresponding place */
	if (wlan_mode == ULINKER_WL_CL) {
		pMib->wlan[0][ULINKER_CL_MIB].wlanMode = 1;
		ulinker_wlan_mib_copy(&pMib->wlan[0][0], &pMib->wlan[0][ULINKER_CL_MIB]);
	}
	else if (wlan_mode == ULINKER_WL_AP) {
	  #if defined(UNIVERSAL_REPEATER)
		if (pMib->repeaterEnabled1 == 1) {
			ulinker_wlan_mib_copy(&pMib->wlan[0][0], &pMib->wlan[0][ULINKER_RPT_MIB]);
		}
		else 
	  #endif
		{
			ulinker_wlan_mib_copy(&pMib->wlan[0][0], &pMib->wlan[0][ULINKER_AP_MIB]);
		}
	}

	debug_ulinker_swap_ap_cl_mib(wlan_mode);
	bdbg_swap_ap_cl_mib(stderr, "[%s] swap ap cl mib, done.\n", __FUNC__);

	return has_set;
}

static int ulinker_does_dut_mode_changed(int mode)
{
	int has_set  = 0;
	int pre_mode, cur_mode;
	int op_mode, wan_mode, dhcp_mode, rtp_enable = 0;

	bdbg_ulinker_does_dut_mode_changed(stderr, "\n[%s] starting...\n", __FUNC__);

	/* is initalized? */
	apmib_get(MIB_ULINKER_LATEST_MODE, (void *)&pre_mode);
	if (pre_mode == 255) {
		has_set = 1;

		apmib_set(MIB_ULINKER_LATEST_MODE, (void *)&mode);
		apmib_set(MIB_ULINKER_CURRENT_MODE, (void *)&mode);

		bdbg_ulinker_does_dut_mode_changed(stderr, "[%s] init to mode[%d]\n", __FUNC__, mode);
	}

	/* does match latest dut mode? */
	apmib_get(MIB_ULINKER_LATEST_MODE, (void *)&pre_mode);
	apmib_get(MIB_ULINKER_CURRENT_MODE, (void *)&cur_mode);

	//if (mode != cur_mode)
	{
		//has_set = 1;

		apmib_set(MIB_ULINKER_LATEST_MODE, (void *)&cur_mode);
		apmib_set(MIB_ULINKER_CURRENT_MODE, (void *)&pre_mode);

		apmib_get(MIB_OP_MODE, (void *)&op_mode);
		apmib_get(MIB_WAN_DHCP, (void *)&wan_mode);
		apmib_get(MIB_DHCP, (void *)&dhcp_mode);

		switch (mode) {
		case ULINKER_BR_AP_MODE:
			op_mode   = BRIDGE_MODE;
			wan_mode  = DHCP_CLIENT;
			dhcp_mode = DHCP_AUTO_WAN;
			break;
		case ULINKER_BR_CL_MODE:
			op_mode   = BRIDGE_MODE;
			wan_mode  = DHCP_CLIENT;
			dhcp_mode = DHCP_AUTO_WAN;
			break;
		case ULINKER_BR_USB_MODE:
		  #if 1 //need check
			fprintf(stderr, "[%s] wlan0 down\n", __FUNC__);
			system("ifconfig wlan0 down");
		  #endif
			op_mode   = BRIDGE_MODE;
			wan_mode  = DHCP_CLIENT;
			dhcp_mode = DHCP_AUTO_WAN;
			break;
		case ULINKER_RT_DHCP_MODE:
			op_mode   = GATEWAY_MODE;
			wan_mode  = DHCP_CLIENT;
			dhcp_mode = DHCP_SERVER;
			break;
		case ULINKER_RT_PPPOE_MODE:
			op_mode   = GATEWAY_MODE;
			wan_mode  = PPPOE;
			dhcp_mode = DHCP_SERVER;
			break;
	#if defined(UNIVERSAL_REPEATER)
		case ULINKER_BR_RPT_MODE:
			op_mode   = BRIDGE_MODE;
			wan_mode  = DHCP_CLIENT;
			dhcp_mode = DHCP_AUTO_WAN;
			rtp_enable= 1;
			break;
	#endif
		}

		apmib_set(MIB_OP_MODE, (void *)&op_mode);
		apmib_set(MIB_WAN_DHCP, (void *)&wan_mode);
		apmib_set(MIB_DHCP, (void *)&dhcp_mode);

	#if defined(UNIVERSAL_REPEATER)
		apmib_set(MIB_REPEATER_ENABLED1, (void *)&rtp_enable);
	#endif

		bdbg_ulinker_does_dut_mode_changed(stderr,
			"change dut mode to [%d], [%d]->[%d]: op_mode[%d], wan_mode[%d], dhcp_mode[%d]\n",
			mode, cur_mode, pre_mode, op_mode, wan_mode, dhcp_mode);
	}

	bdbg_ulinker_does_dut_mode_changed(stderr, "[%s] done.\n", __FUNC__);

	return has_set;
}

static int ulinker_set_dut_to_decided_mode(int mode)
{
	int has_set = 0;
	int wlan_mode;
	bdbg_set_decided_mode(stderr, "\n[%s] decided mode setting...\n", __FUNC__);

	switch (mode)
	{
		case ULINKER_BR_CL_MODE:
		case ULINKER_BR_USB_MODE:
			wlan_mode = ULINKER_WL_CL;
			break;
		case ULINKER_BR_AP_MODE:
		case ULINKER_RT_DHCP_MODE:
		case ULINKER_RT_PPPOE_MODE:
		case ULINKER_BR_RPT_MODE:
			wlan_mode = ULINKER_WL_AP;
			break;
		default:
			fprintf(stderr, "[%s] undefined mode[%d]!!\n", __FUNC__, mode);
			break;
	}


	has_set |= ulinker_does_dut_mode_changed(mode);
	has_set |= ulinker_swap_ap_cl_mib(wlan_mode);

	if (has_set) apmib_update(CURRENT_SETTING);

	bdbg_set_decided_mode(stderr, "[%s] decided mode set, done.\n", __FUNC__);

	return has_set;
}

int ulinker_bootup(void)
{
	int mode;
	int auto_wan;

	//usb0_up();
	usb0_down();
	wlan_down();

	sleep(3);

	/* clean up flags */
	clean_decided_mode();
	clean_auto_dhcp_flag();
	clean_up_layer_dns_flag();
	disable_bridge_dhcp_filter();

	apmib_get(MIB_ULINKER_AUTO, (void *)&auto_wan);
	if (auto_wan == 0) {
		int wlan_mode;
		int val, set = 1;

		/* restore repeater mib */
		pMib->repeaterEnabled1 = pMib->ulinker_repeaterEnabled1;
		pMib->repeaterEnabled2 = pMib->ulinker_repeaterEnabled2;

		apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
		ulinker_swap_ap_cl_mib(wlan_mode);

		apmib_get(MIB_OP_MODE, (void *)&val);

		if (val == GATEWAY_MODE || val == WISP_MODE)
			val = DHCP_SERVER;
		else if (val == BRIDGE_MODE)
			val = DHCP_AUTO_WAN;
		else 
			set = 0;

		if (set == 1)
			apmib_set(MIB_DHCP, (void *)&val);

		return 0;
	}

#if defined(CONFIG_RTL_ULINKER_WLAN_DELAY_INIT)
	if (isFileExist("/var/ulinker_init2")) {
		return 0;
	}
#endif

	bdbg_ulinker_bootup(stderr, "\n[%s] start ulinker_bootup...\n", __FUNC__);

	/* decide dut mode */
	fprintf(stderr, "auto mode deciding...\n");
	mode = ulinker_decide_dut_mode();

	/* set dut mode */
	ulinker_set_dut_to_decided_mode(mode);

	bdbg_ulinker_bootup(stderr, "[%s] end ulinker_bootup.\n", __FUNC__);

	return 0;
}

int ulinker_wlan_init(void)
{
	static int init_wlan = 0;
	int mode, wlan_mode, auto_wan;

	apmib_get(MIB_ULINKER_AUTO, (void *)&auto_wan);
	if (auto_wan == 0) {
		return 0;
	}

	if (init_wlan == 0)
	{
		if (isFileExist("/var/ulinker_init2")) {
			return 0;
		}
		else {
			system("echo \" \" > /var/ulinker_init2");
			init_wlan = 1;
			mode = ulinker_get_decided_mode();

			if (isFileExist("/var/wlan_initated") == 0)
			{
				/* router mode or client mode(no eth1 link) need to initate wlan */
				if (mode != ULINKER_BR_USB_MODE)
				{
					#define WLAN_INIT_DELAY 1
					//fprintf(stderr, "delay %dsec for initate wlan driver\n", WLAN_INIT_DELAY);
					if (mode == ULINKER_RT_DHCP_MODE || mode == ULINKER_RT_PPPOE_MODE) {
						sleep(WLAN_INIT_DELAY); /*delay for initate wlan driver */
					}
					system("echo \"wlan 1\" > /var/wlan_initated");
					system("echo \"wlan 1\" > /proc/wlan_init");
					sleep(1);
				}
			}

			system("ifconfig wlan0 down >/dev/null 2>&1");
			if (mode != ULINKER_BR_USB_MODE)
			{
			#if 1 //fixme
				system("ifconfig eth0 down >/dev/null 2>&1");
				system("brctl delif br0 eth0 >/dev/null 2>&1");
			#endif
				system("flash set_mib wlan0 >/dev/null 2>&1");
				system("ifconfig wlan0 up >/dev/null 2>&1");

				system("brctl addif br0 wlan0 >/dev/null 2>&1");
				if (mode != ULINKER_BR_CL_MODE)//fixme, not good
					system("init.sh gw wlan_app"); //start_wlanapp(0);
			}
		}
	}

	if (init_wlan == 1)
	{
		init_wlan = 0;
		system("rm -f /var/ulinker_init2");

		#if 1//fixme
		{
			int mode = ulinker_get_decided_mode();
			if (mode == ULINKER_BR_AP_MODE)
			{
				system("echo 1 > /proc/sw_nat");
				system("ifconfig eth1 0.0.0.0");
				system("brctl addif br0 eth1 >/dev/null 2>&1");
				system("udhcpc -i br0 -p /etc/udhcpc/udhcpc-br0.pid -s /usr/share/udhcpc/br0.bound");
			}
		}
		#endif
	}

	system("ifconfig usb0 down >/dev/null 2>&1");//tmp
}
#endif /* #if defined(CONFIG_RTL_ULINKER)  */


