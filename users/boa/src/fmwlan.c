/*
 *      Web server handler routines for wlan stuffs
 *
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *
 *      $Id: fmwlan.c,v 1.69 2009/09/04 07:06:23 keith_huang Exp $
 *
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#ifdef WIFI_SIMPLE_CONFIG
#include <sys/time.h>
#endif

#include "boa.h"
#include "asp_page.h"
#include "apmib.h"
#include "apform.h"
#include "utility.h"

#ifdef WLAN_EASY_CONFIG
#include "../md5.h"
#endif

// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT 
#include "web_voip.h"
#endif

#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT)
extern void Stop_Domain_Query_Process(void);
extern void Reset_Domain_Query_Setting(void);
extern int Start_Domain_Query_Process;
#endif

#ifdef WLAN_EASY_CONFIG
#define DO_CONFIG_WAIT_TIME	60
#define CONFIG_SUCCESS		0
#define AUTOCONF_PID_FILENAME	("/var/run/autoconf.pid")

static int wait_config = CONFIG_SUCCESS;
#endif

static SS_STATUS_Tp pStatus=NULL;

#ifdef CONFIG_RTK_MESH
#ifndef __mips__
        #define _FILE_MESH_ASSOC "mesh_assoc_mpinfo"
        #define _FILE_MESH_ROUTE "mesh_pathsel_routetable"
		#define _FILE_MESH_ROOT  "mesh_root_info"
		#define _FILE_MESH_PROXY "mesh_proxy_table"
		#define _FILE_MESH_PORTAL "mesh_portal_table"		
		#define _FILE_MESHSTATS  "mesh_stats"
#else
        #define _FILE_MESH_ASSOC "/proc/wlan0/mesh_assoc_mpinfo"
        #define _FILE_MESH_ROUTE "/proc/wlan0/mesh_pathsel_routetable"
		#define _FILE_MESH_ROOT  "/proc/wlan0/mesh_root_info"
		#define _FILE_MESH_PROXY "/proc/wlan0/mesh_proxy_table"	
		#define _FILE_MESH_PORTAL "/proc/wlan0/mesh_portal_table"
		#define _FILE_MESHSTATS  "/proc/wlan0/mesh_stats"
#endif
#endif // CONFIG_RTK_MESH

#ifdef WIFI_SIMPLE_CONFIG
enum {	CALLED_FROM_WLANHANDLER=1, CALLED_FROM_WEPHANDLER=2, CALLED_FROM_WPAHANDLER=3, CALLED_FROM_ADVANCEHANDLER=4};
struct wps_config_info_struct {
	int caller_id;
	int wlan_mode;
	int auth;
	int shared_type;
	int wep_enc;
	int wpa_enc;
	int wpa2_enc;
	unsigned char ssid[MAX_SSID_LEN];
	int KeyId;
	unsigned char wep64Key1[WEP64_KEY_LEN];
	unsigned char wep64Key2[WEP64_KEY_LEN];
	unsigned char wep64Key3[WEP64_KEY_LEN];
	unsigned char wep64Key4[WEP64_KEY_LEN];
	unsigned char wep128Key1[WEP128_KEY_LEN];
	unsigned char wep128Key2[WEP128_KEY_LEN];
	unsigned char wep128Key3[WEP128_KEY_LEN];
	unsigned char wep128Key4[WEP128_KEY_LEN];
	unsigned char wpaPSK[MAX_PSK_LEN+1];
};
static struct wps_config_info_struct wps_config_info;
static void update_wps_configured(int reset_flag);
#endif

#if defined(CONFIG_RTL_92D_SUPPORT)
static int _isBandModeBoth()
{
	int val;
	apmib_get(MIB_WLAN_BAND2G5G_SELECT, (void *)&val);
	if(val == BANDMODEBOTH)
		return 1;
	else
		return 0;
}
#endif

static void _Start_Wlan_Applications(void)
{

	#if defined (CONFIG_RTL_92D_SUPPORT)
	if(_isBandModeBoth())
		system("sysconf wlanapp start wlan0 wlan1 br0");
	else
		system("sysconf wlanapp start wlan0 br0");
	#else
	system("sysconf wlanapp start wlan0 br0");
	#endif
	sleep(1);
	/*sysconf upnpd 1(isgateway) 1(opmode is bridge)*/
	system("sysconf upnpd 1 1");
	sleep(1);
}


//changes in following table should be synced to MCS_DATA_RATEStr[] in 8190n_proc.c
WLAN_RATE_T rate_11n_table_20M_LONG[]={
	{MCS0, 	"6.5"},
	{MCS1, 	"13"},
	{MCS2, 	"19.5"},
	{MCS3, 	"26"},
	{MCS4, 	"39"},
	{MCS5, 	"52"},
	{MCS6, 	"58.5"},
	{MCS7, 	"65"},
	{MCS8, 	"13"},
	{MCS9, 	"26"},
	{MCS10, 	"39"},
	{MCS11, 	"52"},
	{MCS12, 	"78"},
	{MCS13, 	"104"},
	{MCS14, 	"117"},
	{MCS15, 	"130"},
	{0}
};
WLAN_RATE_T rate_11n_table_20M_SHORT[]={
	{MCS0, 	"7.2"},
	{MCS1, 	"14.4"},
	{MCS2, 	"21.7"},
	{MCS3, 	"28.9"},
	{MCS4, 	"43.3"},
	{MCS5, 	"57.8"},
	{MCS6, 	"65"},
	{MCS7, 	"72.2"},
	{MCS8, 	"14.4"},
	{MCS9, 	"28.9"},
	{MCS10, 	"43.3"},
	{MCS11, 	"57.8"},
	{MCS12, 	"86.7"},
	{MCS13, 	"115.6"},
	{MCS14, 	"130"},
	{MCS15, 	"144.5"},
	{0}
};
WLAN_RATE_T rate_11n_table_40M_LONG[]={
	{MCS0, 	"13.5"},
	{MCS1, 	"27"},
	{MCS2, 	"40.5"},
	{MCS3, 	"54"},
	{MCS4, 	"81"},
	{MCS5, 	"108"},
	{MCS6, 	"121.5"},
	{MCS7, 	"135"},
	{MCS8, 	"27"},
	{MCS9, 	"54"},
	{MCS10, 	"81"},
	{MCS11, 	"108"},
	{MCS12, 	"162"},
	{MCS13, 	"216"},
	{MCS14, 	"243"},
	{MCS15, 	"270"},
	{0}
};
WLAN_RATE_T rate_11n_table_40M_SHORT[]={
	{MCS0, 	"15"},
	{MCS1, 	"30"},
	{MCS2, 	"45"},
	{MCS3, 	"60"},
	{MCS4, 	"90"},
	{MCS5, 	"120"},
	{MCS6, 	"135"},
	{MCS7, 	"150"},
	{MCS8, 	"30"},
	{MCS9, 	"60"},
	{MCS10, 	"90"},
	{MCS11, 	"120"},
	{MCS12, 	"180"},
	{MCS13, 	"240"},
	{MCS14, 	"270"},
	{MCS15, 	"300"},
	{0}
};

WLAN_RATE_T tx_fixed_rate[]={
	{1, "1"},
	{(1<<1), 	"2"},
	{(1<<2), 	"5.5"},
	{(1<<3), 	"11"},
	{(1<<4), 	"6"},
	{(1<<5), 	"9"},
	{(1<<6), 	"12"},
	{(1<<7), 	"18"},
	{(1<<8), 	"24"},
	{(1<<9), 	"36"},
	{(1<<10), 	"48"},
	{(1<<11), 	"54"},
	{(1<<12), 	"MCS0"},
	{(1<<13), 	"MCS1"},
	{(1<<14), 	"MCS2"},
	{(1<<15), 	"MCS3"},
	{(1<<16), 	"MCS4"},
	{(1<<17), 	"MCS5"},
	{(1<<18), 	"MCS6"},
	{(1<<19), 	"MCS7"},
	{(1<<20), 	"MCS8"},
	{(1<<21), 	"MCS9"},
	{(1<<22), 	"MCS10"},
	{(1<<23), 	"MCS11"},
	{(1<<24), 	"MCS12"},
	{(1<<25), 	"MCS13"},
	{(1<<26), 	"MCS14"},
	{(1<<27), 	"MCS15"},
	{0}
};

/////////////////////////////////////////////////////////////////////////////
#ifndef NO_ACTION
//Patch: kill some daemons to free some RAM in order to call "init.sh gw al"l more quickly
//which need more tests
void killSomeDaemon(void)
{
	system("killall -9 sleep 2> /dev/null");
       system("killall -9 routed 2> /dev/null");
//	system("killall -9 pppoe 2> /dev/null");
//	system("killall -9 pppd 2> /dev/null");
//	system("killall -9 pptp 2> /dev/null");
	system("killall -9 dnrd 2> /dev/null");
	system("killall -9 ntpclient 2> /dev/null");
//	system("killall -9 miniigd 2> /dev/null");	//comment for miniigd iptables rule recovery
	system("killall -9 lld2d 2> /dev/null");
//	system("killall -9 l2tpd 2> /dev/null");	
//	system("killall -9 udhcpc 2> /dev/null");	
//	system("killall -9 udhcpd 2> /dev/null");	
	system("killall -9 reload 2> /dev/null");		
	system("killall -9 iapp 2> /dev/null");	
	system("killall -9 wscd 2> /dev/null");
	system("killall -9 mini_upnpd 2> /dev/null");
	system("killall -9 iwcontrol 2> /dev/null");
	system("killall -9 auth 2> /dev/null");
	system("killall -9 disc_server 2> /dev/null");
	system("killall -9 igmpproxy 2> /dev/null");
	system("echo 1,0 > /proc/br_mCastFastFwd");
	system("killall -9 syslogd 2> /dev/null");
	system("killall -9 klogd 2> /dev/null");
	
	system("killall -9 ppp_inet 2> /dev/null");
	
#ifdef CONFIG_SNMP
	system("killall -9 snmpd 2> /dev/null");
	system("rm -f /var/run/snmpd.pid");
#endif
}

void run_init_script(char *arg)
{
#ifdef NO_ACTION
	// do nothing
#else
	int pid=0;
	int i;
	char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};
	
#ifdef REBOOT_CHECK
	if(run_init_script_flag == 1){
#endif

#ifdef RTK_USB3G
	system("killall -9 mnet 2> /dev/null");
	system("killall -9 hub-ctrl 2> /dev/null");
	system("killall -9 usb_modeswitch 2> /dev/null");
    system("killall -9 ppp_inet 2> /dev/null");
    system("killall -9 pppd 2> /dev/null");
    system("rm /etc/ppp/connectfile >/dev/null 2>&1");
#endif /* #ifdef RTK_USB3G */

#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT)
	Stop_Domain_Query_Process();
	Reset_Domain_Query_Setting();
#endif

#if defined(CONFIG_RTL_ULINKER)
{
	extern int kill_ppp(void);
	int wan_mode, op_mode;

	apmib_get(MIB_OP_MODE,(void *)&op_mode);
	apmib_get(MIB_WAN_DHCP,(void *)&wan_mode);
	if(wan_mode == PPPOE && op_mode == GATEWAY_MODE)
		kill_ppp();
	
	stop_dhcpc();
	stop_dhcpd();
	clean_auto_dhcp_flag();
	disable_bridge_dhcp_filter();
}
#endif

	snprintf(tmpBuf, MAX_MSG_BUFFER_SIZE, "%s/%s.pid", _DHCPD_PID_PATH, _DHCPD_PROG_NAME);
	pid = getPid(tmpBuf);
	if ( pid > 0)
		kill(pid, SIGUSR1);
		
	usleep(1000);
	
	if ( pid > 0){
		system("killall -9 udhcpd 2> /dev/null");
		system("rm -f /var/run/udhcpd.pid 2> /dev/null");
	}

	//Patch: kill some daemons to free some RAM in order to call "init.sh gw all" more quickly
	//which need more tests especially for 8196c 2m/16m
	killSomeDaemon();
	
	system("killsh.sh");	// kill all running script	

#ifdef REBOOT_CHECK
	run_init_script_flag = 0;
	needReboot = 0;
#endif
// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT
	web_restart_solar();
#endif

	pid = fork();
/*	
       	if (pid)
               	waitpid(pid, NULL, 0);
   	else 
*/ 
	if (pid == 0) {
#ifdef HOME_GATEWAY
		sprintf(tmpBuf, "%s gw %s", _CONFIG_SCRIPT_PROG, arg);
#elif defined(VOIP_SUPPORT) && defined(ATA867x)
		sprintf(tmpBuf, "%s ATA867x %s", _CONFIG_SCRIPT_PROG, arg);
#else
		sprintf(tmpBuf, "%s ap %s", _CONFIG_SCRIPT_PROG, arg);
#endif
		for(i=3; i<sysconf(_SC_OPEN_MAX); i++)
                	close(i);
		sleep(1);
		system(tmpBuf);
		exit(1);
	}
#ifdef REBOOT_CHECK
}
	else
	{
	}
#endif
#endif
}

#endif //#ifndef NO_ACTION

/////////////////////////////////////////////////////////////////////////////
static inline int isAllStar(char *data)
{
	int i;
	for (i=0; i<strlen(data); i++) {
		if (data[i] != '*')
			return 0;
	}
	return 1;
}
//////////////////////
#ifndef HOME_GATEWAY
void formSetTime(request *wp, char *path, char *query)
{
	char *submitUrl,*strVal;
	char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};
	int time_value=0;
	int cur_year=0;

	submitUrl = req_get_cstream_var(wp, ("submit-url"), "");   
	strVal = req_get_cstream_var(wp, ("save"), "");   

	if(strVal[0]){		
		struct tm tm_time;
		time_t tm;
		memcpy(&tm_time, localtime(&tm), sizeof(tm_time));
		tm_time.tm_sec = 0;
		tm_time.tm_min = 0;
		tm_time.tm_hour = 0;
		tm_time.tm_isdst = -1;  /* Be sure to recheck dst. */
		strVal = req_get_cstream_var(wp, ("year"), "");	
		cur_year= atoi(strVal);
		tm_time.tm_year = atoi(strVal) - 1900;
		strVal = req_get_cstream_var(wp, ("month"), "");	
		tm_time.tm_mon = atoi(strVal)-1;
		strVal = req_get_cstream_var(wp, ("day"), "");	
		tm_time.tm_mday = atoi(strVal);
		strVal = req_get_cstream_var(wp, ("hour"), "");	
		tm_time.tm_hour = atoi(strVal);
		strVal = req_get_cstream_var(wp, ("minute"), "");	
		tm_time.tm_min = atoi(strVal);
		strVal = req_get_cstream_var(wp, ("second"), "");	
		tm_time.tm_sec = atoi(strVal);
		tm = mktime(&tm_time);
		if(tm < 0){
			sprintf(tmpBuf, "set Time Error\n");
			goto setErr_end;
		}
		if(stime(&tm) < 0){
			sprintf(tmpBuf, "set Time Error\n");
			goto setErr_end;
		}
		
		apmib_set( MIB_SYSTIME_YEAR, (void *)&cur_year);
		time_value = tm_time.tm_mon;
		apmib_set( MIB_SYSTIME_MON, (void *)&time_value);
		time_value = tm_time.tm_mday;
		apmib_set( MIB_SYSTIME_DAY, (void *)&time_value);
		time_value = tm_time.tm_hour;
		apmib_set( MIB_SYSTIME_HOUR, (void *)&time_value);
		time_value = tm_time.tm_min;
		apmib_set( MIB_SYSTIME_MIN, (void *)&time_value);
		time_value = tm_time.tm_sec;
		apmib_set( MIB_SYSTIME_SEC, (void *)&time_value);
	}
	
	apmib_update_web(CURRENT_SETTING);
	OK_MSG(submitUrl);
	return;
setErr_end:
	ERR_MSG(tmpBuf);	
}

#endif

#if defined(NEW_SCHEDULE_SUPPORT)
int wlSchList(request *wp, int argc, char **argv)
{
	SCHEDULE_T entry;
	char *strToken;
	int cmpResult=0;
	int  index=0;
	
	cmpResult= strncmp(argv[0], "wlSchList_", strlen("wlSchList_"));
	strToken=strstr(argv[0], "_");
	index= atoi(strToken+1);

	index++;
	if(index <= MAX_SCHEDULE_NUM)
	{
		*((char *)&entry) = (char)index;
		if ( !apmib_get(MIB_WLAN_SCHEDULE_TBL, (void *)&entry))
		{
			fprintf(stderr,"Get schedule entry fail\n");
			return -1;
		}												
		
		
		/* eco/day/fTime/tTime/week */
		req_format_write(wp, ("%d|%d|%d|%d"), entry.eco, entry.day, entry.fTime, entry.tTime);
	}
	else
	{
		req_format_write(wp, ("0|0|0|0") );
	}
	return 0;
}
#endif //#if defined(NEW_SCHEDULE_SUPPORT)

void formSchedule(request *wp, char *path, char *query)
{
	char	tmpBuf[MAX_MSG_BUFFER_SIZE];
	char *strHours, *strEnabled, *strWeekdays, *strStime, *strEtime;
	SCHEDULE_T entry;
	int entryNum=0;
	char *submitUrl;
	int isEnabled=0;
	submitUrl = req_get_cstream_var(wp, ("webpage"), "");   // hidden page
	
	
	if ( !apmib_set(MIB_WLAN_SCHEDULE_DELALL, (void *)&entry)) {
			strcpy(tmpBuf, ("Delete table entry error!"));
			goto setErr_schedule;
	}
	memset(&entry, '\0', sizeof(entry));
	
	strEnabled = req_get_cstream_var(wp, ("enabled_sch"), "");
	if(strcmp(strEnabled,"true") == 0) // the entry is enabled
	{
			entry.eco |= ECO_LEDDIM_MASK;
			isEnabled = 1;
	}else{
			entry.eco &= ~ECO_LEDDIM_MASK;
			isEnabled = 0;
	}
	apmib_set(MIB_WLAN_SCHEDULE_ENABLED,(void *)&isEnabled);
	sprintf((char *)entry.text, "%s", "wlanSchedule");	
	
	strWeekdays = req_get_cstream_var(wp, ("weekdays"), "");
	entry.day = atoi(strWeekdays);

	

	if(strcmp(strWeekdays, "127") ==0)
	{
		entry.eco |= ECO_EVERYDAY_MASK;
	}else
		entry.eco &= ~ECO_EVERYDAY_MASK;
		  
	strHours = req_get_cstream_var(wp, ("all_day"), "");	

	if(strcmp(strHours,"on") == 0) // the entry is enabled 24 hours
	{
		entry.eco |= ECO_24HOURS_MASK;
		
	}else
		entry.eco &= ~ECO_24HOURS_MASK;

	strStime = req_get_cstream_var(wp, ("start_time"), "");
	if(strStime[0])
		entry.fTime = atoi(strStime);

	strEtime = req_get_cstream_var(wp, ("end_time"), "");
	if(strEtime[0])
		entry.tTime = atoi(strEtime);
	
	if(entry.eco & ECO_24HOURS_MASK){
			entry.fTime = 0;
			entry.tTime = 1440;
	}
	
	if ( !apmib_get(MIB_WLAN_SCHEDULE_TBL_NUM, (void *)&entryNum)) 
	{
			strcpy(tmpBuf, ("\"Get entry number error!\""));
			goto setErr_schedule;
	}
	if ( !apmib_set(MIB_WLAN_SCHEDULE_ADD,(void *)&entry)) {
			strcpy(tmpBuf, ("Add table entry error!"));
			goto setErr_schedule;
	}
	
	
	
	
	apmib_update_web(CURRENT_SETTING);
#ifndef NO_ACTION
	run_init_script("bridge");
#endif

OK_MSG(submitUrl);
	return;

setErr_schedule:
	ERR_MSG(tmpBuf);
	
}

#if defined(NEW_SCHEDULE_SUPPORT)
void formNewSchedule(request *wp, char *path, char *query)
{
	SCHEDULE_T entry;
	char *submitUrl,*strTmp;
	int	i, wlsch_onoff;
	char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};
	
//displayPostDate(wp->post_data);	
	
	strTmp= req_get_cstream_var(wp, ("wlsch_onoff"), "");
	if(strTmp[0])
	{
		wlsch_onoff = atoi(strTmp);
	
		if (!apmib_set(MIB_WLAN_SCHEDULE_ENABLED, (void *)&wlsch_onoff)) 
		{
			strcpy(tmpBuf, ("set  MIB_WLAN_SCHEDULE_ENABLED error!"));
			goto setErr_schedule;
		}
	}
	
	if ( !apmib_set(MIB_WLAN_SCHEDULE_DELALL, (void *)&entry)) {
			strcpy(tmpBuf, ("MIB_WLAN_SCHEDULE_DELALL error!"));
			goto setErr_schedule;
	}
	
	for(i=1; i<=MAX_SCHEDULE_NUM ; i++)
	{
		int index;
		memset(&entry, '\0', sizeof(entry));
		
		*((char *)&entry) = (char)i;
		apmib_get(MIB_WLAN_SCHEDULE_TBL, (void *)&entry);			

		index = i-1;
			
		memset(tmpBuf,0x00, sizeof(tmpBuf));			
		sprintf(tmpBuf,"wlsch_enable_%d",index);
		strTmp = req_get_cstream_var(wp, tmpBuf, "");
		if(strTmp[0])
		{
			entry.eco = atoi(strTmp);
		}
		
		memset(tmpBuf,0x00, sizeof(tmpBuf));			
		sprintf(tmpBuf,"wlsch_day_%d",index);
		strTmp = req_get_cstream_var(wp, tmpBuf, "");
		if(strTmp[0])
		{
			entry.day = atoi(strTmp);
		}
		
		memset(tmpBuf,0x00, sizeof(tmpBuf));			
		sprintf(tmpBuf,"wlsch_from_%d",index);
		strTmp = req_get_cstream_var(wp, tmpBuf, "");
		if (strTmp[0]) {
			entry.fTime = atoi(strTmp);
		}
		
		memset(tmpBuf,0x00, sizeof(tmpBuf));			
		sprintf(tmpBuf,"wlsch_to_%d",index);
		strTmp = req_get_cstream_var(wp, tmpBuf, "");
		if(strTmp[0])
		{
			entry.tTime = atoi(strTmp);
		}
		
		if ( apmib_set(MIB_WLAN_SCHEDULE_ADD, (void *)&entry) == 0) 
		{				
			strcpy(tmpBuf, ("MIB_WLAN_SCHEDULE_ADD error!"));				
			goto setErr_schedule;
		}
	}

	submitUrl = req_get_cstream_var(wp, ("submit-url"), "");   // hidden page
	
	apmib_update_web(CURRENT_SETTING);
#ifndef NO_ACTION
	run_init_script("all");
#endif
	OK_MSG(submitUrl);
	return;

setErr_schedule:
	ERR_MSG(tmpBuf);
	
}
#endif // #if defined(NEW_SCHEDULE_SUPPORT)

int getScheduleInfo(request *wp, int argc, char **argv)
{
	int	entryNum=0, i;
	SCHEDULE_T entry;
	int everyday=0, hours24=0;
	int dayWeek=0;
	char tmpBuf[200];
	unsigned char buffer[200];
	int isEnabled=0;
	char *strToken;
	int cmpResult=0;
	int index=0;
	char	*name_arg;

	//printf("get parameter=%s\n", argv[0]);
	name_arg = argv[0];
	if (name_arg == NULL) {
   		fprintf(stderr, "Insufficient args\n");
   		return -1;
   	}
   	
	if ( !strcmp(name_arg, ("wlan_state")) ) {
		bss_info bss;
		getWlBssInfo(WLAN_IF, &bss);
		if (bss.state == STATE_DISABLED) 
			strcpy((char *)buffer, "Disabled");
		else
			strcpy((char *)buffer, "Enabled");	
		req_format_write(wp, "%s", buffer);
		return 0;
	}else if(!strcmp(name_arg, ("system_time"))){
		#ifdef HOME_GATEWAY
					return 0;
		#else
		
		return req_format_write(wp,"%s","menu.addItem(\"System Time\", \"time.htm\", \"\", \"Setup System Time\");");
		#endif
	} 		
	cmpResult= strncmp(name_arg, "getentry_", 9);
	strToken=strstr(name_arg, "_");
	
	index= atoi(strToken+1);
	
	if ( !apmib_get(MIB_WLAN_SCHEDULE_TBL_NUM, (void *)&entryNum)) {
  		strcpy(tmpBuf, "Get table entry error!");
		return -1;
	}
	apmib_get(MIB_WLAN_SCHEDULE_ENABLED,(void *)&isEnabled);
	if(isEnabled==0){
		req_format_write(wp,"%s", "wlanSchedule-0-0-0-0-0-0");
		return 0;
	}
		
		for (i=1; i<=entryNum; i++) {
				*((char *)&entry) = (char)i;
				if ( !apmib_get(MIB_WLAN_SCHEDULE_TBL, (void *)&entry)){
					fprintf(stderr,"Get SCHEDULE entry fail\n");
					return -1;
				}
				if(entry.eco & ECO_EVERYDAY_MASK)
					everyday = 1;
				else
					everyday = 0;
				
				if(entry.eco & ECO_24HOURS_MASK)
					hours24 = 1;
				else
					hours24 = 0;
					
				if(everyday == 1)
				{
					dayWeek = 127; /* 01111111 */
				}
				else
				{
					dayWeek=entry.day;					
				}
				
				if(hours24 == 1)
				{
					entry.fTime=0;
					entry.tTime=1435;
				}
				
				if(index==i){
					req_format_write(wp,"%s-%d-%d-%d-%d-%d-%d",entry.text, isEnabled, everyday, dayWeek, hours24, entry.fTime, entry.tTime);   
				}
		}

	
	return 0;
	
	
	
}

#ifdef UNIVERSAL_REPEATER
void setRepeaterSsid(int wlanid, int rptid, char *str_ssid)
{
	char wlanifStr[10];
	char tmpStr[MAX_SSID_LEN];		
	
	apmib_save_wlanIdx();
	sprintf(wlanifStr,"wlan%d-vxd",wlanid);
	SetWlan_idx(wlanifStr);	
	
	apmib_get(MIB_WLAN_SSID, (void *)tmpStr);
	
	if(strcmp(tmpStr, str_ssid) != 0 && strcmp(str_ssid, tmpStr) != 0)
	{
		int is_configured = 1;
		
		apmib_set(MIB_WLAN_WSC_CONFIGURED, (void *)&is_configured);
		
		sprintf(tmpStr,"%s",str_ssid);
		apmib_set(MIB_WLAN_SSID, (void *)tmpStr);
		apmib_set(MIB_WLAN_WSC_SSID, (void *)tmpStr);	
		apmib_set(rptid, (void *)tmpStr);
	}
	
	sprintf(wlanifStr,"wlan%d",wlanid);
	SetWlan_idx(wlanifStr);
	apmib_recov_wlanIdx();
}
#endif

////////////////////

#if defined(WLAN_PROFILE)
int addWlProfileHandler(request *wp, char *tmpBuf, int wlan_id)
{
	char *tmpStr;
	char varName[20];

//printf("\r\n wlan_idx=[%d],vwlan_idx=[%d],__[%s-%u]\r\n",wlan_idx,vwlan_idx,__FILE__,__LINE__);	

	sprintf(varName, "wizardAddProfile%d", wlan_id);
	tmpStr = req_get_cstream_var(wp, varName, "");
	if(tmpStr[0])
	{
		int profile_num_id, profile_tbl_id, profile_add_id, profile_delall_id;
		WLAN_PROFILE_T entry;
		char strSSID[64]={0};
		int encryptVal;
		int entryNum;
//printf("\r\n __[%s-%u]\r\n",__FILE__,__LINE__);
		if(wlan_id == 0)
		{
			profile_num_id = MIB_PROFILE_NUM1;
			profile_tbl_id = MIB_PROFILE_TBL1;
			profile_add_id = MIB_PROFILE_ADD1;
			profile_delall_id = MIB_PROFILE_DELALL1;
		}
		else
		{
			profile_num_id = MIB_PROFILE_NUM2;
			profile_tbl_id = MIB_PROFILE_TBL2;
			profile_add_id = MIB_PROFILE_ADD2;
			profile_delall_id = MIB_PROFILE_DELALL2;			
		}
		
		apmib_get(profile_num_id, (void *)&entryNum);

		if ( (entryNum + 1) > MAX_WLAN_PROFILE_NUM) {
			strcpy(tmpBuf, ("Cannot add new entry because table is full!"));
			goto setErr_end;
		}
		
		memset(&entry, 0x00, sizeof(WLAN_PROFILE_T));
		entry.wpa_cipher = 8;//WPA_CIPHER_AES
		
		

		apmib_get(MIB_WLAN_SSID, (void *)strSSID);
		strcpy(entry.ssid, strSSID);

		apmib_get(MIB_WLAN_ENCRYPT, (void *)&encryptVal);

//printf("\r\n encryptVal=[%d],__[%s-%u]\r\n",encryptVal,__FILE__,__LINE__);	

		if(encryptVal == ENCRYPT_WEP)
		{
			int wepType;
			int authType;
			int defKey;
			char key[30];
			int keyType;
			
			apmib_get( MIB_WLAN_WEP, (void *)&wepType);
			if(wepType == WEP64)
			{
				entry.encryption = WEP64;

				memset(key, 0x00, sizeof(key));					
				apmib_get(MIB_WLAN_WEP64_KEY1, (void *)key);
				strcpy(entry.wepKey1, key);

				memset(key, 0x00, sizeof(key));
				apmib_get(MIB_WLAN_WEP64_KEY2, (void *)key);
				strcpy(entry.wepKey2, key);

				memset(key, 0x00, sizeof(key));
				apmib_get(MIB_WLAN_WEP64_KEY3, (void *)key);
				strcpy(entry.wepKey3, key);

				memset(key, 0x00, sizeof(key));
				apmib_get(MIB_WLAN_WEP64_KEY4, (void *)key);
				strcpy(entry.wepKey4, key);
			}
			else
			{
				entry.encryption = WEP128;
				
				memset(key, 0x00, sizeof(key));
				apmib_get(MIB_WLAN_WEP128_KEY1, (void *)key);
				strcpy(entry.wepKey1, key);

				memset(key, 0x00, sizeof(key));
				apmib_get(MIB_WLAN_WEP128_KEY2, (void *)key);
				strcpy(entry.wepKey2, key);

				memset(key, 0x00, sizeof(key));
				apmib_get(MIB_WLAN_WEP128_KEY3, (void *)key);
				strcpy(entry.wepKey3, key);

				memset(key, 0x00, sizeof(key));
				apmib_get(MIB_WLAN_WEP128_KEY4, (void *)key);
				strcpy(entry.wepKey4, key);
			}

			apmib_get( MIB_WLAN_AUTH_TYPE, (void *)&authType);
			entry.auth = authType;

			apmib_get( MIB_WLAN_WEP_DEFAULT_KEY, (void *)&defKey);
			entry.wep_default_key = defKey;

			apmib_get( MIB_WLAN_WEP_KEY_TYPE, (void *)&keyType);
			entry.wepKeyType = keyType;

			
		}
		else if(encryptVal > ENCRYPT_WEP)
		{
			int cipherSuite;
			int pskFormat;
			char wpaPsk[65]={0};
			
			if(encryptVal== ENCRYPT_WPA)
			{
				entry.encryption = 3;
				apmib_get( MIB_WLAN_WPA_CIPHER_SUITE, (void *)&cipherSuite);
			}
			else
			{
				entry.encryption = 4;
				apmib_get( MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&cipherSuite);
			}

			
			if(cipherSuite == WPA_CIPHER_TKIP)
				entry.wpa_cipher = 2;
			else
				entry.wpa_cipher = 8;

			apmib_get( MIB_WLAN_PSK_FORMAT, (void *)&pskFormat);
			entry.wpaPSKFormat = pskFormat;

			apmib_get(MIB_WLAN_WPA_PSK,  (void *)wpaPsk);

			strcpy(entry.wpaPSK, wpaPsk);
			
		}
		else
		{
			entry.encryption = ENCRYPT_DISABLED;
		}

#if defined(PROFILE_BOTTOM_UP)
		WLAN_PROFILE_T oriEntry[MAX_WLAN_PROFILE_NUM];
		int roop=0;

		for(roop=0 ; roop<entryNum; roop++)
		{
			memset(oriEntry+roop, 0x00, sizeof(WLAN_PROFILE_T));
			*((char *)(oriEntry+roop)) = (char)(roop+1);
			apmib_get(profile_tbl_id, (void *)(oriEntry+roop));

//printf("\r\n oriEntry[roop].ssid=[%s],__[%s-%u]\r\n",oriEntry[roop].ssid,__FILE__,__LINE__);

		}

		apmib_set(profile_delall_id, (void *)&entry);

#endif

		if ( apmib_set(profile_add_id, (void *)&entry) == 0) {
			strcpy(tmpBuf, ("Add table entry error!"));
			goto setErr_end;
		}

#if defined(PROFILE_BOTTOM_UP)
		for(roop=0 ; roop<entryNum; roop++)
		{
			apmib_set(profile_add_id, (void *)(oriEntry+roop));
		}
#endif



		

	}

	return 0 ;
	
setErr_end:
	return -1 ;	
}
#endif //#if defined(WLAN_PROFILE)

#if defined(WLAN_PROFILE)
static int wlProfileHandler(request *wp, char *tmpBuf, int wlan_id)
{
	char *strVal, *strSSID;
	char varName[20], strtmp[80];
	char *strAddWlProfile, *strAddRptProfile, *strDelSelProfile, *strDelAllProfile, *strWlProfileEnabled;
	int profile_num_id, profile_tbl_id, profile_add_id, profile_del_id, profile_delall_id, profile_enabled_id;
	int entryNum;
	int wlProfileEnabled;
	WLAN_PROFILE_T entry;
	int mode;
	int val;
	
//displayPostDate(wp->post_data);

	sprintf(varName, "mode%d", wlan_id);
	strVal = req_get_cstream_var(wp, varName, "");

	if(strVal[0] == 0)
	{
		int val;

		apmib_get( MIB_WLAN_MODE, (void *)&val);
		sprintf(strtmp,"%d",val);
		strVal = strtmp;		
	}

	
	if ( strVal[0] ) {
		mode = strVal[0] - '0';

		if (mode == CLIENT_MODE) {
			ENCRYPT_T encrypt;
      		apmib_get( MIB_WLAN_ENCRYPT,  (void *)&encrypt);
		if (encrypt &  ENCRYPT_WPA2_MIXED) {
			int format;
			apmib_get( MIB_WLAN_WPA_AUTH, (void *)&format);
			if (format & 1) { // radius
#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
				//Support wlan client mode with Enterprise (RADIUS)
#else
				strcpy(tmpBuf, ("You cannot set client mode with Enterprise (RADIUS) !<br><br>Please change the encryption method in security page first."));
				goto setErr_wlan;
#endif
			}
		}
		else if (encrypt == ENCRYPT_WEP || encrypt == 0) {
			int use1x;
			apmib_get( MIB_WLAN_ENABLE_1X, (void *)&use1x);
			if (use1x & 1) {
#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
				//Support wlan client mode with Enterprise (RADIUS)
#else
				strcpy(tmpBuf, ("You cannot set client mode with 802.1x enabled!<br><br>Please change the encryption method in security page first."));
				goto setErr_wlan;
#endif
			}
		}
		sprintf(varName, "wlanMacClone%d", wlan_id);
		strVal = req_get_cstream_var(wp, varName, "");
		if ( !strcmp(strVal, "ON"))
			val = 1 ;
		else
			val = 0 ;
		if ( apmib_set( MIB_WLAN_MACCLONE_ENABLED, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set wlan Mac clone error!"));
			goto setErr_wlan;
		}
	}

	if ( apmib_set( MIB_WLAN_MODE, (void *)&mode) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_MODE error!"));
		goto setErr_wlan;
	}
	
#ifdef WLAN_EASY_CONFIG
	apmib_set( MIB_WLAN_EASYCFG_WLAN_MODE, (void *)&mode);
#endif
	

}
		
	if(wlan_id == 0)
	{
		profile_num_id = MIB_PROFILE_NUM1;
		profile_tbl_id = MIB_PROFILE_TBL1;
		profile_add_id = MIB_PROFILE_ADD1;
		profile_del_id = MIB_PROFILE_DEL1;
		profile_delall_id = MIB_PROFILE_DELALL1;
		profile_enabled_id = MIB_PROFILE_ENABLED1;
	}
	else
	{
		profile_num_id = MIB_PROFILE_NUM2;
		profile_tbl_id = MIB_PROFILE_TBL2;
		profile_add_id = MIB_PROFILE_ADD2;		
		profile_del_id = MIB_PROFILE_DEL2;
		profile_delall_id = MIB_PROFILE_DELALL2;
		profile_enabled_id = MIB_PROFILE_ENABLED2;
	}
	

	strWlProfileEnabled = req_get_cstream_var(wp, ("wlProfileEnabled"), "");
	strAddWlProfile = req_get_cstream_var(wp, ("addWlProfile"), "");
	strAddRptProfile = req_get_cstream_var(wp, ("addRptProfile"), "");
	strDelSelProfile = req_get_cstream_var(wp, ("delSelWlProfile"), "");
	strDelAllProfile = req_get_cstream_var(wp, ("delAllWlProfile"), "");
	
	if (strWlProfileEnabled[0]) {
			wlProfileEnabled = atoi(strWlProfileEnabled);
			apmib_set( profile_enabled_id, (void *)&wlProfileEnabled);			
	}
	else
	{
		wlProfileEnabled = 0;
		apmib_set( profile_enabled_id, (void *)&wlProfileEnabled);
	}
	
	/* Add entry */
	if (strAddWlProfile[0]) 
	{
		int del_ret; 

		memset(&entry,0x00, sizeof(WLAN_PROFILE_T));
		entry.wpa_cipher = 8; //WPA_CIPHER_AES
		
		sprintf(varName, "ssid%d", wlan_id);
   		strSSID = req_get_cstream_var(wp, varName, "");
   		strcpy(entry.ssid, strSSID);
   	

		apmib_get(profile_num_id, (void *)&entryNum);
		
		if ( (entryNum + 1) > MAX_WLAN_PROFILE_NUM) {
			strcpy(tmpBuf, ("Cannot add new entry because table is full!"));
			goto setErr_wlan;
		}

#if defined(PROFILE_BOTTOM_UP)
		WLAN_PROFILE_T oriEntry[MAX_WLAN_PROFILE_NUM];
		int roop=0;

		for(roop=0 ; roop<entryNum; roop++)
		{
			memset(oriEntry+roop, 0x00, sizeof(WLAN_PROFILE_T));
			*((char *)(oriEntry+roop)) = (char)(roop+1);
			apmib_get(profile_tbl_id, (void *)(oriEntry+roop));

//printf("\r\n oriEntry[roop].ssid=[%s],__[%s-%u]\r\n",oriEntry[roop].ssid,__FILE__,__LINE__);

		}

		apmib_set(profile_delall_id, (void *)&entry);

#endif
		// set to MIB. try to delete it first to avoid duplicate case
		del_ret = apmib_set(profile_del_id, (void *)&entry);
		if ( apmib_set(profile_add_id, (void *)&entry) == 0) {
			strcpy(tmpBuf, ("Add table entry error!"));
			goto setErr_wlan;
		}

#if defined(PROFILE_BOTTOM_UP)
		for(roop=0 ; roop<entryNum; roop++)
		{
			apmib_set(profile_add_id, (void *)(oriEntry+roop));
		}
#endif
		if(del_ret == 1)
		{
			//printf("\r\n Duplicate add profile__[%s-%u]\r\n",__FILE__,__LINE__);
			strcpy(tmpBuf, ("Add Profile duplicately!"));
			goto setErr_wlan;
		}

		sprintf(varName, "ssid%d", wlan_id);
		strSSID = req_get_cstream_var(wp, varName, "");
		if ( strSSID[0] ) {
			if ( apmib_set(MIB_WLAN_SSID, (void *)strSSID) == 0) {
	   	 			strcpy(tmpBuf, ("Set SSID error!"));
					goto setErr_wlan;
			}
		}
		else if ( mode == 1 && !strSSID[0] ) { // client and NULL SSID
			if ( apmib_set(MIB_WLAN_SSID, (void *)strSSID) == 0) {
	   	 			strcpy(tmpBuf, ("Set SSID error!"));
					goto setErr_wlan;
			}
		}

	}

	if (strAddRptProfile[0]) 
	{
		int id, rpt_enabled;
		int del_ret; 

		memset(&entry,0x00, sizeof(WLAN_PROFILE_T));
		entry.wpa_cipher = 8; //WPA_CIPHER_AES
		
		sprintf(varName, "repeaterSSID%d", wlan_id);
   	strSSID = req_get_cstream_var(wp, varName, "");
   	strcpy(entry.ssid, strSSID);
   	
		apmib_get(profile_num_id, (void *)&entryNum);
		
		if ( (entryNum + 1) > MAX_WLAN_PROFILE_NUM) {
			strcpy(tmpBuf, ("Cannot add new entry because table is full!"));
			goto setErr_wlan;
		}

#if defined(PROFILE_BOTTOM_UP)
		WLAN_PROFILE_T oriEntry[MAX_WLAN_PROFILE_NUM];
		int roop=0;

		for(roop=0 ; roop<entryNum; roop++)
		{
			memset(oriEntry+roop, 0x00, sizeof(WLAN_PROFILE_T));
			*((char *)(oriEntry+roop)) = (char)(roop+1);
			apmib_get(profile_tbl_id, (void *)(oriEntry+roop));

//printf("\r\n oriEntry[roop].ssid=[%s],__[%s-%u]\r\n",oriEntry[roop].ssid,__FILE__,__LINE__);

		}

		apmib_set(profile_delall_id, (void *)&entry);

#endif
		// set to MIB. try to delete it first to avoid duplicate case
		del_ret = apmib_set(profile_del_id, (void *)&entry);
		if ( apmib_set(profile_add_id, (void *)&entry) == 0) {
			strcpy(tmpBuf, ("Add table entry error!"));
			goto setErr_wlan;
		}

#if defined(PROFILE_BOTTOM_UP)
		for(roop=0 ; roop<entryNum; roop++)
		{
			apmib_set(profile_add_id, (void *)(oriEntry+roop));
		}
#endif

		if(del_ret == 1)
		{
			//printf("\r\n Duplicate add profile__[%s-%u]\r\n",__FILE__,__LINE__);
			strcpy(tmpBuf, ("Add Profile duplicately!"));
			goto setErr_wlan;
		}
		
		sprintf(varName, "repeaterEnabled%d", wlan_id);

		
		strVal = req_get_cstream_var(wp, varName, "");
		if ( !strcmp(strVal, "ON"))
			val = 1 ;
		else
			val = 0 ;
			
#if defined(CONFIG_RTL_ULINKER)
		if (wlan_id == 0)
			id = MIB_REPEATER_ENABLED1;
		else
			id = MIB_REPEATER_ENABLED2;
		apmib_get(id, (void *)&rpt_enabled);
		if(mode == AP_MODE && rpt_enabled == 1) //ulinker repeater mode
		{
			val = 1;
		}
#endif
			
		if (wlan_id == 0)
			id = MIB_REPEATER_ENABLED1;
		else
			id = MIB_REPEATER_ENABLED2;
		apmib_set(id, (void *)&val);

		if (val == 1) {
			sprintf(varName, "repeaterSSID%d", wlan_id);
			strVal = req_get_cstream_var(wp, varName, NULL);
			if (strVal){
				if (wlan_id == 0)
					id = MIB_REPEATER_SSID1;
				else
					id = MIB_REPEATER_SSID2;
					
				setRepeaterSsid(wlan_id, id, strVal);
			}
		}

	}


	
	/* Delete entry */
	if (strDelSelProfile[0]) {
		int i;
		apmib_get(profile_num_id, (void *)&entryNum);

			
		for (i=entryNum; i>0; i--) {
			strVal = NULL;
			snprintf(strtmp, 20, "select%d", i);

			strVal = req_get_cstream_var(wp, strtmp, "");
			if(strVal == NULL)
				continue;
				
			if ( !strcmp(strVal, "ON") ) {

				*((char *)&entry) = (char)i;
				
				if ( !apmib_get(profile_tbl_id, (void *)&entry)) {
					strcpy(tmpBuf, ("Get table entry error!"));
					goto setErr_wlan;
				}
				if ( !apmib_set(profile_del_id, (void *)&entry)) {
					strcpy(tmpBuf, ("Delete table entry error!"));
					goto setErr_wlan;
				}
			}
		}

	}

	/* Delete all entry */
	if ( strDelAllProfile[0]) {

		if ( !apmib_set(profile_delall_id, (void *)&entry)) {
			strcpy(tmpBuf, ("Delete all table error!"));
			goto setErr_wlan;
		}
	}


	return  0;

setErr_wlan:
	return -1 ;	
	
}

#endif //#if defined(WLAN_PROFILE)

int wlanHandler(request *wp, char *tmpBuf, int *mode, int wlan_id)
{
  char *strSSID, *strChan, *strDisabled, *strVal, strtmp[80];
	int chan, disabled ;
	NETWORK_TYPE_T net;
	char *strRate;
	int val;
	char varName[20];
	int band_no=0;
	int cur_band=0;
	
//displayPostDate(wp->post_data);
	
	sprintf(varName, "wlanDisabled%d", wlan_id);
	strDisabled = req_get_cstream_var(wp, varName, "");
	if ( !strcmp(strDisabled, "ON"))
		disabled = 1;
	else
		disabled = 0;
	if ( apmib_set( MIB_WLAN_WLAN_DISABLED, (void *)&disabled) == 0) {
  		strcpy(tmpBuf, ("Set disabled flag error!"));
		goto setErr_wlan;
	}

	if ( disabled )
		return 0;

#ifdef WIFI_SIMPLE_CONFIG
	memset(&wps_config_info, 0, sizeof(struct wps_config_info_struct));
	wps_config_info.caller_id = CALLED_FROM_WLANHANDLER;
	apmib_get(MIB_WLAN_SSID, (void *)wps_config_info.ssid);	
	apmib_get(MIB_WLAN_MODE, (void *)&wps_config_info.wlan_mode);
#endif

	sprintf(varName, "regdomain%d", wlan_id);
	strVal = req_get_cstream_var(wp, varName, "");
	if(strVal[0]){
		apmib_get(MIB_HW_REG_DOMAIN, (void *)&val);
		if(val != atoi(strVal)){
			val=atoi(strVal);
			if ( apmib_set(MIB_HW_REG_DOMAIN, (void *)&val) == 0) {
					strcpy(tmpBuf, ("Set wlan regdomain error!"));
					goto setErr_wlan;
			}
			apmib_update(HW_SETTING);
		}
	}

	sprintf(varName, "countrystr%d", wlan_id);
	strVal = req_get_cstream_var(wp, varName, "");
	if(strVal[0]){
			if (apmib_set(MIB_WLAN_COUNTRY_STRING, (void *)strVal) == 0) {
					strcpy(tmpBuf, ("Set wlan countrystr error!"));
					goto setErr_wlan;
			}
	}

	sprintf(varName, "tx_restrict%d", wlan_id);
	strVal = req_get_cstream_var(wp, varName, "");
	if(strVal[0])
	{
		val=atoi(strVal);
		if (apmib_set(MIB_WLAN_TX_RESTRICT, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_TX_RESTRICT error!"));
			goto setErr_wlan;
		}
	}

	sprintf(varName, "rx_restrict%d", wlan_id);
	strVal = req_get_cstream_var(wp, varName, "");
	if(strVal[0])
	{
		val=atoi(strVal);
		if (apmib_set(MIB_WLAN_RX_RESTRICT, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_RX_RESTRICT error!"));
			goto setErr_wlan;
		}
	}

	sprintf(varName, "mode%d", wlan_id);
	strVal = req_get_cstream_var(wp, varName, "");

	if(strVal[0] == 0)
	{
		int val;

		apmib_get( MIB_WLAN_MODE, (void *)&val);
		sprintf(strtmp,"%d",val);
		strVal = strtmp;		
	}


	if ( strVal[0] ) {
#ifndef CONFIG_RTK_MESH
		if (strVal[0]!= '0' && strVal[0]!= '1' && strVal[0]!= '2' &&  strVal[0]!= '3'
#ifdef CONFIG_RTL_P2P_SUPPORT
		&&  strVal[0]!= '8'
#endif
		) {
#else
#ifdef CONFIG_NEW_MESH_UI
		if (strVal[0]!= '0' && strVal[0]!= '1' && strVal[0]!= '2' &&  strVal[0]!= '3' &&  strVal[0]!= '4' &&  strVal[0]!= '5' ) {
#else
		if (strVal[0]!= '0' && strVal[0]!= '1' && strVal[0]!= '2' &&  strVal[0]!= '3' &&  strVal[0]!= '4' &&  strVal[0]!= '5' &&  strVal[0]!= '6'&&  strVal[0]!= '7') {
#endif
#endif // CONFIG_RTK_MESH
			printf("%s\n",strVal[0]);
  			strcpy(tmpBuf, ("Invalid mode value!"));
			goto setErr_wlan;
		}
		*mode = strVal[0] - '0';

		if (*mode == CLIENT_MODE) {
			ENCRYPT_T encrypt;
      		apmib_get( MIB_WLAN_ENCRYPT,  (void *)&encrypt);
			if (encrypt &  ENCRYPT_WPA2_MIXED) {
				int format;
				apmib_get( MIB_WLAN_WPA_AUTH, (void *)&format);
				if (format & 1) { // radius
#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
					//Support wlan client mode with Enterprise (RADIUS)
#else
					strcpy(tmpBuf, ("You cannot set client mode with Enterprise (RADIUS) !<br><br>Please change the encryption method in security page first."));
					goto setErr_wlan;
#endif
				}
			}
			else if (encrypt == ENCRYPT_WEP || encrypt == 0) {
				int use1x;
				apmib_get( MIB_WLAN_ENABLE_1X, (void *)&use1x);
				if (use1x & 1) {
#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
					//Support wlan client mode with Enterprise (RADIUS)
#else
					strcpy(tmpBuf, ("You cannot set client mode with 802.1x enabled!<br><br>Please change the encryption method in security page first."));
					goto setErr_wlan;
#endif
				}
			}
			sprintf(varName, "wlanMacClone%d", wlan_id);
			strVal = req_get_cstream_var(wp, varName, "");
			if ( !strcmp(strVal, "ON"))
				val = 1 ;
			else
				val = 0 ;
			if ( apmib_set( MIB_WLAN_MACCLONE_ENABLED, (void *)&val) == 0) {
				strcpy(tmpBuf, ("Set wlan Mac clone error!"));
				goto setErr_wlan;
			}
		}

		if ( apmib_set( MIB_WLAN_MODE, (void *)mode) == 0) {
   			strcpy(tmpBuf, ("Set MIB_WLAN_MODE error!"));
			goto setErr_wlan;
		}

#ifdef WLAN_EASY_CONFIG
		apmib_set( MIB_WLAN_EASYCFG_WLAN_MODE, (void *)mode);
#endif

	}

	sprintf(varName, "ssid%d", wlan_id);
   	strSSID = req_get_cstream_var(wp, varName, "");
	if ( strSSID[0] ) {
		if ( apmib_set(MIB_WLAN_SSID, (void *)strSSID) == 0) {
   	 			strcpy(tmpBuf, ("Set SSID error!"));
				goto setErr_wlan;
		}
	}
	else if ( *mode == 1 && !strSSID[0] ) { // client and NULL SSID
		if ( apmib_set(MIB_WLAN_SSID, (void *)strSSID) == 0) {
   	 			strcpy(tmpBuf, ("Set SSID error!"));
				goto setErr_wlan;
		}
	}

	sprintf(varName, "chan%d", wlan_id);
	strChan = req_get_cstream_var(wp, varName, "");
	if ( strChan[0] ) {
		errno=0;
		chan = strtol( strChan, (char **)NULL, 10);
		if (errno) {
   			strcpy(tmpBuf, ("Invalid channel number!"));
			goto setErr_wlan;
		}
		if ( apmib_set( MIB_WLAN_CHANNEL, (void *)&chan) == 0) {
   			strcpy(tmpBuf, ("Set channel number error!"));
			goto setErr_wlan;
		}
	}

	sprintf(varName, "type%d", wlan_id);
	strVal = req_get_cstream_var(wp, varName, "");
	if (strVal[0]) {
		if (strVal[0]!= '0' && strVal[0]!= '1') {
  			strcpy(tmpBuf, ("Invalid network type value!"));
			goto setErr_wlan;
		}
		if (strVal[0] == '0')
			net = INFRASTRUCTURE;
		else
			net = ADHOC;
		if ( apmib_set(MIB_WLAN_NETWORK_TYPE, (void *)&net) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_NETWORK_TYPE failed!"));
			goto setErr_wlan;
		}
	}

	sprintf(varName, "band%d", wlan_id);
	strVal = req_get_cstream_var(wp, varName, "");
	if ( strVal[0] ) 
	{
		int wlan_onoff_tkip;
		
		apmib_get( MIB_WLAN_11N_ONOFF_TKIP, (void *)&wlan_onoff_tkip);
				
		band_no = strtol( strVal, (char **)NULL, 10);
		if (band_no < 0 || band_no > 19) {
  			strcpy(tmpBuf, ("Invalid band value!"));
			goto setErr_wlan;
		}
		//val = (strVal[0] - '0' + 1);
		if(wlan_onoff_tkip == 0) //Wifi request
		{
			int wpaCipher;
			int wpa2Cipher;
			int wdsEncrypt;
			int wlan_encrypt=0;
			
			apmib_get( MIB_WLAN_ENCRYPT, (void *)&wlan_encrypt);
			apmib_get(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&wpaCipher);
			apmib_get(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&wpa2Cipher);
			apmib_get( MIB_WLAN_WDS_ENCRYPT, (void *)&wdsEncrypt);
			
			if(*mode != CLIENT_MODE && (band_no == 7 || band_no == 9 || band_no == 10 || band_no == 11)) //7:n; 9:gn; 10:bgn 11:5g_an
			{
				
				if(wlan_encrypt ==ENCRYPT_WPA || wlan_encrypt ==ENCRYPT_WPA2){
				wpaCipher &= ~WPA_CIPHER_TKIP;
					if(wpaCipher== 0)
						wpaCipher =  WPA_CIPHER_AES;
				apmib_set(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&wpaCipher);
				
				wpa2Cipher &= ~WPA_CIPHER_TKIP;
					if(wpa2Cipher== 0)
						wpa2Cipher =  WPA_CIPHER_AES;
				apmib_set(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&wpa2Cipher);
				}
				if(wdsEncrypt == WDS_ENCRYPT_TKIP)
				{
					wdsEncrypt = WDS_ENCRYPT_DISABLED;
					apmib_set( MIB_WLAN_WDS_ENCRYPT, (void *)&wdsEncrypt);
				}
			}		
		}
		val = (band_no + 1);
		if ( apmib_set( MIB_WLAN_BAND, (void *)&val) == 0) {
   			strcpy(tmpBuf, ("Set band error!"));
			goto setErr_wlan;
		}		
	}

	// set tx rate
	sprintf(varName, "txRate%d", wlan_id);
	strRate = req_get_cstream_var(wp, varName, "");
	if ( strRate[0] ) {
		if ( strRate[0] == '0' ) { // auto
			val = 1;
			if ( apmib_set(MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&val) == 0) {
				strcpy(tmpBuf, ("Set rate adaptive failed!"));
				goto setErr_wlan;
			}
		}
		else  {
			val = 0;
			if ( apmib_set(MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&val) == 0) {
				strcpy(tmpBuf, ("Set rate adaptive failed!"));
				goto setErr_wlan;
			}  
			val = atoi(strRate);
			val = 1 << (val-1);
			if ( apmib_set(MIB_WLAN_FIX_RATE, (void *)&val) == 0) {
				strcpy(tmpBuf, ("Set fix rate failed!"));
				goto setErr_wlan;
			}
		}			
	}

	sprintf(varName, "basicrates%d", wlan_id);
	strRate = req_get_cstream_var(wp, varName, "");	
	if ( strRate[0] ) {
		val = atoi(strRate);		
		if ( apmib_set(MIB_WLAN_BASIC_RATES, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set Tx basic rate failed!"));
			goto setErr_wlan;
		}
	}

	sprintf(varName, "operrates%d", wlan_id);
	strRate = req_get_cstream_var(wp, varName, "");	
	if ( strRate[0] ) {
		val = atoi(strRate);
		if ( apmib_set(MIB_WLAN_SUPPORTED_RATES, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set Tx operation rate failed!"));
			goto setErr_wlan;
		}
	}

	// set hidden SSID
	sprintf(varName, "hiddenSSID%d", wlan_id);
	strVal = req_get_cstream_var(wp, varName, "");
	if (strVal[0]) {
		if ( strVal[0] == '0')
			val = 0;
		else if (strVal[0] == '1')
			val = 1;
		else {
			strcpy(tmpBuf, ("Error! Invalid Channel Bonding."));
			goto setErr_wlan;
		}
		if ( apmib_set(MIB_WLAN_HIDDEN_SSID, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set hidden ssid failed!"));
			goto setErr_wlan;
		}
	}
	sprintf(varName, "wlanwmm%d", wlan_id);
	strVal= req_get_cstream_var(wp, varName, "");
	if (strVal[0]) {
		if ( strVal[0] == '0')
			val = 0;
		else if (strVal[0] == '1')
			val = 1;
		else {
			strcpy(tmpBuf, ("Error! Invalid WMM value."));
			goto setErr_wlan;
		}
		if ( apmib_set(MIB_WLAN_WMM_ENABLED, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_WMM_ENABLED failed!"));
			goto setErr_wlan;
		}
	}else{
		//enable wmm in 11N mode always
			apmib_get( MIB_WLAN_BAND, (void *)&cur_band);
			if(cur_band == 10 || cur_band ==11){
				val = 1;
				if ( apmib_set(MIB_WLAN_WMM_ENABLED, (void *)&val) == 0) {
					strcpy(tmpBuf, ("Set MIB_WLAN_WMM_ENABLED failed!"));
					goto setErr_wlan;
				}
			}
	}
// for 11N
	sprintf(varName, "channelbound%d", wlan_id);
	strVal = req_get_cstream_var(wp, varName, "");
	if (strVal[0]) {
		if ( strVal[0] == '0')
			val = 0;
		else if (strVal[0] == '1')
			val = 1;
		else {
			strcpy(tmpBuf, ("Error! Invalid Channel Bonding."));
			goto setErr_wlan;
		}
		if ( apmib_set(MIB_WLAN_CHANNEL_BONDING, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_CHANNEL_BONDING failed!"));
			goto setErr_wlan;
		}
	}

	sprintf(varName, "controlsideband%d", wlan_id);
	strVal= req_get_cstream_var(wp, varName, "");
	if (strVal[0]) {
		if ( strVal[0] == '0')
			val = 0;
		else if ( strVal[0] == '1')
			val = 1;
		else {
			strcpy(tmpBuf, ("Error! Invalid Control SideBand."));
			goto setErr_wlan;
		}
		if ( apmib_set(MIB_WLAN_CONTROL_SIDEBAND, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_CONTROL_SIDEBAND failed!"));
			goto setErr_wlan;
		}
	}

//

	sprintf(varName, "basicrates%d", wlan_id);
	strRate = req_get_cstream_var(wp, varName, "");
	if ( strRate[0] ) {
		val = atoi(strRate);

		if ( val && apmib_set(MIB_WLAN_BASIC_RATES, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set Tx basic rate failed!"));
			goto setErr_wlan;
		}
	}

	sprintf(varName, "operrates%d", wlan_id);
	strRate = req_get_cstream_var(wp, varName, "");
	if ( strRate[0] ) {
		val = atoi(strRate);
		if ( val && apmib_set(MIB_WLAN_SUPPORTED_RATES, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set Tx operation rate failed!"));
			goto setErr_wlan;
		}
	}	//do twice ??

#ifdef UNIVERSAL_REPEATER
#ifdef CONFIG_RTK_MESH
	if( *mode >= 4 && *mode <=7)
	{
		val=0;
		apmib_set(MIB_REPEATER_ENABLED1, (void *)&val);
		apmib_set(MIB_REPEATER_ENABLED2, (void *)&val);
	}
	else
#endif
{	int id;
	sprintf(varName, "repeaterEnabled%d", wlan_id);
	strVal = req_get_cstream_var(wp, ("lan_ip"), "");
	
	if ((strVal==NULL || strVal[0]==0)   // not called from wizard	
			//&& (*mode != WDS_MODE)	&& (*mode != CLIENT_MODE)
		) 
		{
		int rpt_enabled;
		
		strVal = req_get_cstream_var(wp, varName, "");
		if ( !strcmp(strVal, "ON"))
			val = 1 ;
		else
			val = 0 ;
			
#if defined(CONFIG_RTL_ULINKER)
		if (wlan_id == 0)
			id = MIB_REPEATER_ENABLED1;
		else
			id = MIB_REPEATER_ENABLED2;
		apmib_get(id, (void *)&rpt_enabled);
		if(*mode == AP_MODE && rpt_enabled == 1) //ulinker repeater mode
		{
			val = 1;
		}
#endif
			
		if (wlan_id == 0)
			id = MIB_REPEATER_ENABLED1;
		else
			id = MIB_REPEATER_ENABLED2;
		apmib_set(id, (void *)&val);

		if (val == 1) {
			sprintf(varName, "repeaterSSID%d", wlan_id);
			strVal = req_get_cstream_var(wp, varName, NULL);
			if (strVal){
				if (wlan_id == 0)
					id = MIB_REPEATER_SSID1;
				else
					id = MIB_REPEATER_SSID2;
					
				setRepeaterSsid(wlan_id, id, strVal);
			}
		}

#ifdef MBSSID
		int old_idx = vwlan_idx;
		vwlan_idx = NUM_VWLAN_INTERFACE; // repeater interface
		int disable;
		if (val)
			disable = 0;
		else
			disable = 1;		
		apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&disable);

		if (!disable) {
			if (*mode == CLIENT_MODE)
				val = AP_MODE;
			else
				val = CLIENT_MODE;
			apmib_set(MIB_WLAN_MODE, (void *)&val);			
			apmib_set(MIB_WLAN_SSID, (void *)strVal);			
		}

		if (val == CLIENT_MODE) {
			// if client mode, check if Radius or mixed mode encryption is used
			apmib_get(MIB_WLAN_ENCRYPT, (void *)&val);

			if (val <= ENCRYPT_WEP) {				
				apmib_get( MIB_WLAN_ENABLE_1X, (void *)&val);
				if (val != 0) {
					val = 0;
					apmib_set( MIB_WLAN_ENABLE_1X, (void *)&val);				
				}
			}	
			else if (val == ENCRYPT_WPA2_MIXED) {				
				val = ENCRYPT_DISABLED;
				apmib_set(MIB_WLAN_ENCRYPT, (void *)&val);
			}
			else if (val == ENCRYPT_WPA) {	
				apmib_get(MIB_WLAN_WPA_AUTH, (void *)&val);
				if ((val == 0) || (val & 1)) { // if no or radius, force to psk
					val = 2;
					apmib_set(MIB_WLAN_WPA_AUTH, (void *)&val);
				}				
				apmib_get(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&val);
				if ((val == 0) || (val == WPA_CIPHER_MIXED)) {
					val = WPA_CIPHER_AES;
					apmib_set(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&val);					
				}
			}
			else if (val == ENCRYPT_WPA2) {	
				apmib_get(MIB_WLAN_WPA_AUTH, (void *)&val);
				if ((val == 0) || (val & 1)) { // if no or radius, force to psk
					val = 2;
					apmib_set(MIB_WLAN_WPA_AUTH, (void *)&val);
				}				
				apmib_get(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&val);
				if ((val == 0) || (val == WPA_CIPHER_MIXED)) {
					val = WPA_CIPHER_AES;
					apmib_set(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&val);					
				}
			}	
		}

		vwlan_idx = old_idx;
#endif	
	}
}
#endif

#ifdef WIFI_SIMPLE_CONFIG
	sprintf(varName, "wps_clear_configure_by_reg%d", wlan_id);
	strVal = req_get_cstream_var(wp, varName, NULL);
	val = 0;
	if (strVal && strVal[0])
		val = atoi(strVal);
	update_wps_configured(val);
#endif

	return  0;
setErr_wlan:
	return -1 ;
}

#ifdef CONFIG_RTK_MESH
/////////////////////////////////////////////////////////////////////////////

int meshWpaHandler(request *wp, char *tmpBuf, int wlan_id)
{
  	char *strEncrypt, *strVal;
	ENCRYPT_T encrypt;
	int	intVal, getPSK=1, len;	

	char varName[20];

	sprintf(varName, "method%d", wlan_id);
   	strEncrypt = req_get_cstream_var(wp, varName, "");
	if (!strEncrypt[0]) {
 		strcpy(tmpBuf, ("Error! no encryption method."));
		goto setErr_mEncrypt;
	}
	encrypt = (ENCRYPT_T) strEncrypt[0] - '0';
	if (encrypt!=ENCRYPT_DISABLED &&  encrypt != ENCRYPT_WPA2 ) {
		strcpy(tmpBuf, ("Invalid encryption method!"));
		goto setErr_mEncrypt;
	}

	if (apmib_set( MIB_MESH_ENCRYPT, (void *)&encrypt) == 0) {
  		strcpy(tmpBuf, ("Set MIB_MESH_ENCRYPT mib error!"));
		goto setErr_mEncrypt;
	}

	if(encrypt == ENCRYPT_WPA2)
	{
		// WPA authentication  ( RADIU / Pre-Shared Key )
		intVal = WPA_AUTH_PSK;		
		if ( apmib_set(MIB_MESH_WPA_AUTH, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set MIB_MESH_AUTH_TYPE failed!"));
				goto setErr_mEncrypt;
		}

		// cipher suite	 (TKIP / AES)
		intVal =   WPA_CIPHER_AES ;		
		if ( apmib_set(MIB_MESH_WPA2_CIPHER_SUITE, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set MIB_MESH_WPA2_UNICIPHER failed!"));
				goto setErr_mEncrypt;
		}

		// pre-shared key
		if ( getPSK ) {
			int oldFormat, oldPskLen, i;

			sprintf(varName, "pskFormat%d", wlan_id);
   			strVal = req_get_cstream_var(wp, varName, "");
			if (!strVal[0]) {
	 			strcpy(tmpBuf, ("Error! no psk format."));
				goto setErr_mEncrypt;
			}
			intVal = strVal[0] - '0';
			if (intVal != 0 && intVal != 1) {
	 			strcpy(tmpBuf, ("Error! invalid psk format."));
				goto setErr_mEncrypt;
			}

			// remember current psk format and length to compare to default case "****"
			apmib_get(MIB_MESH_PSK_FORMAT, (void *)&oldFormat);
			apmib_get(MIB_MESH_WPA_PSK, (void *)tmpBuf);
			oldPskLen = strlen(tmpBuf);

			sprintf(varName, "pskValue%d", wlan_id);
			strVal = req_get_cstream_var(wp, varName, "");
			len = strlen(strVal);

			if (oldFormat == intVal && len == oldPskLen ) {
				for (i=0; i<len; i++) {
					if ( strVal[i] != '*' )
						break;
				}
				if (i == len)
					goto mRekey_time;
			}

			if ( apmib_set(MIB_MESH_PSK_FORMAT, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set MIB_MESH_PSK_FORMAT failed!"));
				goto setErr_mEncrypt;
			}

			if (intVal==1) { // hex
				if (len!=MAX_PSK_LEN || !string_to_hex(strVal, tmpBuf, MAX_PSK_LEN)) {
	 				strcpy(tmpBuf, ("Error! invalid psk value."));
					goto setErr_mEncrypt;
				}
			}
			else { // passphras
				if (len==0 || len > (MAX_PSK_LEN-1) ) {
	 				strcpy(tmpBuf, ("Error! invalid psk value."));
					goto setErr_mEncrypt;
				}
			}
			if ( !apmib_set(MIB_MESH_WPA_PSK, (void *)strVal)) {
				strcpy(tmpBuf, ("Set MIB_MESH_WPA_PSK error!"));
				goto setErr_mEncrypt;
			}
		}	
	}
mRekey_time:
		// group key rekey time			
	return 0 ;
setErr_mEncrypt:
	return -1 ;		
}	

#ifdef 	_11s_TEST_MODE_
void formEngineeringMode(request *wp, char *path, char *query)
{
	char *submitUrl;
	char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};
	char *param;
	int val;
	//
	param = req_get_cstream_var(wp, "param1", "");
	string_to_dec(param , &val);
	if ( apmib_set(MIB_MESH_TEST_PARAM1, (void *)&val)==0 )
		goto setErr_meshTest;
	sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reserved1=%d", val);
	system(tmpBuf);
	
	param = req_get_cstream_var(wp, "param2", "");
	string_to_dec(param , &val);
	if ( apmib_set(MIB_MESH_TEST_PARAM2, (void *)&val)==0 )
		goto setErr_meshTest;
	sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reserved2=%d", val);
	system(tmpBuf);

	param = req_get_cstream_var(wp, "param3", "");
	string_to_dec(param , &val);
	if ( apmib_set(MIB_MESH_TEST_PARAM3, (void *)&val)==0 )	
		goto setErr_meshTest;
	sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reserved3=%d", val);
	system(tmpBuf);

	param = req_get_cstream_var(wp, "param4", "");
	string_to_dec(param , &val);
	if ( apmib_set(MIB_MESH_TEST_PARAM4, (void *)&val)==0 )
		goto setErr_meshTest;
	sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reserved4=%d", val);
	system(tmpBuf);

	param = req_get_cstream_var(wp, "param5", "");
	string_to_dec(param , &val);
	if ( apmib_set(MIB_MESH_TEST_PARAM5, (void *)&val)==0 )
		goto setErr_meshTest;
	sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reserved5=%d", val);
	system(tmpBuf);
	
	param = req_get_cstream_var(wp, "param6", "");
	string_to_dec(param , &val);
	if ( apmib_set(MIB_MESH_TEST_PARAM6, (void *)&val)==0 )
		goto setErr_meshTest;
	sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reserved6=%d", val);
	system(tmpBuf);

	param = req_get_cstream_var(wp, "param7", "");
	string_to_dec(param , &val);
	if ( apmib_set(MIB_MESH_TEST_PARAM7, (void *)&val)==0 )	
		goto setErr_meshTest;
	sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reserved7=%d", val);
	system(tmpBuf);

	param = req_get_cstream_var(wp, "param8", "");
	string_to_dec(param , &val);
	if ( apmib_set(MIB_MESH_TEST_PARAM8, (void *)&val)==0 )
		goto setErr_meshTest;
	sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reserved8=%d", val);
	system(tmpBuf);

	param = req_get_cstream_var(wp, "param9", "");
	string_to_dec(param , &val);
	if ( apmib_set(MIB_MESH_TEST_PARAM9, (void *)&val)==0 )
		goto setErr_meshTest;
	sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reserved9=%d", val);
	system(tmpBuf);
	
	param = req_get_cstream_var(wp, "parama", "");
	string_to_dec(param , &val);
	if ( apmib_set(MIB_MESH_TEST_PARAMA, (void *)&val)==0 )
		goto setErr_meshTest;
	sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reserveda=%d", val);
	system(tmpBuf);

	param = req_get_cstream_var(wp, "paramb", "");
	string_to_dec(param , &val);
	if ( apmib_set(MIB_MESH_TEST_PARAMB, (void *)&val)==0 )	
		goto setErr_meshTest;
	sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reservedb=%d", val);
	system(tmpBuf);

	param = req_get_cstream_var(wp, "paramc", "");
	string_to_dec(param , &val);
	if ( apmib_set(MIB_MESH_TEST_PARAMC, (void *)&val)==0 )
		goto setErr_meshTest;
	sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reservedc=%d", val);
	system(tmpBuf);

	param = req_get_cstream_var(wp, "paramd", "");
	string_to_dec(param , &val);
	if ( apmib_set(MIB_MESH_TEST_PARAMD, (void *)&val)==0 )
		goto setErr_meshTest;
	sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reservedd=%d", val);
	system(tmpBuf);
	
	param = req_get_cstream_var(wp, "parame", "");
	string_to_dec(param , &val);
	if ( apmib_set(MIB_MESH_TEST_PARAME, (void *)&val)==0 )
		goto setErr_meshTest;
	sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reservede=%d", val);
	system(tmpBuf);

	param = req_get_cstream_var(wp, "paramf", "");
	string_to_dec(param , &val);
	if ( apmib_set(MIB_MESH_TEST_PARAMF, (void *)&val)==0 )	
		goto setErr_meshTest;
	sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reservedf=%d", val);
	system(tmpBuf);
	
	param = req_get_cstream_var(wp, "paramstr1", "");
    if (param[0])
    {
            if (strlen(param)>16) 
                  goto setErr_meshTest;

            if ( apmib_set(MIB_MESH_TEST_PARAMSTR1, (void *)param) == 0)
                    goto setErr_meshTest;
			sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reservedstr1='%s'", param);
			system(tmpBuf);			
    }
    apmib_update(CURRENT_SETTING);
/*
#ifndef NO_ACTION
        run_init_script("bridge");
#endif
*/
	submitUrl = req_get_cstream_var(wp, ("meshtest-url"), "");   // hidden page
	OK_MSG(submitUrl);
	return;

setErr_meshTest:
		strcpy(tmpBuf, ("Error! set Mesh Test Param Error!!! "));
        ERR_MSG(tmpBuf);	 
}

void formEngineeringMode2(request *wp, char *path, char *query)
{
	char *submitUrl;
	char	*strCMD;
	char tmpBuf[200];
	strCMD = req_get_cstream_var(wp, ("cmd"), "");
	system(strCMD);
	submitUrl = req_get_cstream_var(wp, ("meshtest-url"), "");   // hidden page
	OK_MSG1(tmpBuf, submitUrl);
}

#endif



#ifdef _MESH_ACL_ENABLE_
int wlMeshAcList(request *wp, int argc, char **argv)
{
	int nBytesSent=0, entryNum, i;
	MACFILTER_T entry;
	char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};

	if ( !apmib_get(MIB_MESH_ACL_NUM, (void *)&entryNum)) {
  		fprintf(stderr, "Get MIB_MESH_ACL_NUM table entry error!\n");
		return -1;
	}

	nBytesSent += req_format_write(wp, ("<tr>"
      	"<td align=center width=\"45%%\" bgcolor=\"#808080\"><font size=\"2\"><b>MAC Address</b></font></td>\n"
      	"<td align=center width=\"35%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Comment</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td></tr>\n"));

	for (i=1; i<=entryNum; i++) {
		*((char *)&entry) = (char)i;
		if ( !apmib_get(MIB_MESH_ACL_ADDR, (void *)&entry))
			return -1;

		snprintf(tmpBuf, MAX_MSG_BUFFER_SIZE, ("%02x:%02x:%02x:%02x:%02x:%02x"),
			entry.macAddr[0], entry.macAddr[1], entry.macAddr[2],
			entry.macAddr[3], entry.macAddr[4], entry.macAddr[5]);

		nBytesSent += req_format_write(wp, ("<tr>"
			"<td align=center width=\"45%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"35%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
       			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n"),
				tmpBuf, entry.comment, i);
	}
	return nBytesSent;
}

void formMeshACLSetup(request *wp, char *path, char *query)
{
	char *submitUrl;
	char *strAddMac, *strDelMac, *strDelAllMac, *strVal, *strEnabled;
	int entryNum, i, enabled;
	MACFILTER_T macEntry;
	char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};

	strAddMac = req_get_cstream_var(wp, ("addMeshAclMac"), "");
	strDelMac = req_get_cstream_var(wp, ("deleteSelMeshAclMac"), "");
	strDelAllMac = req_get_cstream_var(wp, ("deleteAllMeshAclMac"), "");
	strEnabled = req_get_cstream_var(wp, ("meshAclEnabled"), "");
	submitUrl = req_get_cstream_var(wp, ("mesh-url"), "");   // hidden page

	if (strAddMac[0]) {
		/*if ( !strcmp(strEnabled, "ON"))
			enabled = 1;
		else
			enabled = 0; */ //by sc_yang
		 enabled = strEnabled[0] - '0';
		if ( apmib_set( MIB_MESH_ACL_ENABLED, (void *)&enabled) == 0) {
  			strcpy(tmpBuf, ("Set enabled flag error!"));
			goto setErr_meshACL;
		}

		strVal = req_get_cstream_var(wp, ("aclmac"), "");
		if ( !strVal[0] ) {		// For Disable/Allow/Deny mode setting.
//			strcpy(tmpBuf, ("Error! No mac address to set."));
			goto meshAclExit;
		}
		if (strlen(strVal)!=12 || !string_to_hex(strVal, macEntry.macAddr, 12)) {
			strcpy(tmpBuf, ("Error! Invalid MAC address."));
			goto setErr_meshACL;
		}

		strVal = req_get_cstream_var(wp, ("aclcomment"), "");
		if ( strVal[0] ) {
			if (strlen(strVal) > COMMENT_LEN-1) {
				strcpy(tmpBuf, ("Error! Comment length too long."));
				goto setErr_meshACL;
			}
			strcpy(macEntry.comment, strVal);
		}
		else
			macEntry.comment[0] = '\0';

		if ( !apmib_get(MIB_MESH_ACL_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto setErr_meshACL;
		}
		if ( (entryNum + 1) > MAX_MESH_ACL_NUM) {
			strcpy(tmpBuf, ("Cannot add new entry, Because table is full!"));
			goto setErr_meshACL;
		}

		// set to MIB. try to delete it first to avoid duplicate case
		apmib_set(MIB_MESH_ACL_ADDR_DEL, (void *)&macEntry);
		if ( apmib_set(MIB_MESH_ACL_ADDR_ADD, (void *)&macEntry) == 0) {
			strcpy(tmpBuf, ("Add table entry error!"));
			goto setErr_meshACL;
		}
		goto meshAclExit;
	}

	/* Delete entry */
	if (strDelMac[0]) {
		if ( !apmib_get(MIB_MESH_ACL_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto setErr_meshACL;
		}
		for (i=entryNum; i>0; i--) {
			snprintf(tmpBuf, 20, "select%d", i);

			strVal = req_get_cstream_var(wp, tmpBuf, "");
			if ( !strcmp(strVal, "ON") ) {

				*((char *)&macEntry) = (char)i;
				if ( !apmib_get(MIB_MESH_ACL_ADDR, (void *)&macEntry)) {
					strcpy(tmpBuf, ("Get table entry error!"));
					goto setErr_meshACL;
				}
				if ( !apmib_set(MIB_MESH_ACL_ADDR_DEL, (void *)&macEntry)) {
					strcpy(tmpBuf, ("Delete table entry error!"));
					goto setErr_meshACL;
				}
			}
		}
		goto meshAclExit;
	}

	/* Delete all entry */
	if ( strDelAllMac[0]) {
		if ( !apmib_set(MIB_MESH_ACL_ADDR_DELALL, (void *)&macEntry)) {
			strcpy(tmpBuf, ("Delete all table error!"));
			goto setErr_meshACL;
		}
		goto meshAclExit;
	}

meshAclExit:
#ifndef NO_ACTION
        run_init_script("bridge");
#endif
        apmib_update(CURRENT_SETTING);

        submitUrl = req_get_cstream_var(wp, ("mesh-url"), "");   // hidden page
#ifdef REBOOT_CHECK
        OK_MSG(submitUrl);
#else
	RECONNECT_MSG(submitUrl);       // display reconnect msg to remote
#endif

        return;

setErr_meshACL:
        ERR_MSG(tmpBuf);
}
#endif	// _MESH_ACL_ENABLE_

int formMeshProxyTbl(request *wp, char *path, char *query)
{
        char *submitUrl,*refresh;

        submitUrl = req_get_cstream_var(wp, ("mesh-url"), "");   // hidden page
        refresh = req_get_cstream_var(wp, ("refresh"), "");

        if ( refresh[0] )
        {
                send_redirect_perm(wp, submitUrl);
                return;
        }
}
char * _get_token( FILE * fPtr,char * token,char * data )
{
        char buf[512];
        char * pch;

        strcpy( data,"");

        if( fgets(buf, sizeof buf, fPtr) == NULL ) // get a new line
                return NULL;

        pch = strstr( buf, token ); //parse the tag

        if( pch == NULL )
                return NULL;

        pch += strlen( token );

        sprintf( data,"%s",pch );                  // set data

        return pch;
}


void strtolower(char *str, int len)
{
	int i;
	for (i = 0; i<len; i++) {
		str[i] = tolower(str[i]);
	}
}


void formMeshProxy(request *wp, char *path, char *query)
{
	char *strPrxyOwnr;
	int nRecordCount=0;
	FILE *fh;
	char buf[512];
	char sta[20],owner[20], macstr[20];
	
	strPrxyOwnr = req_get_cstream_var(wp, ("owner"), "");
	strtolower(strPrxyOwnr, 12);
	
	// show proxy
	if ( strPrxyOwnr[0] )
	{
		sprintf(macstr, "%c%c:%c%c:%c%c:%c%c:%c%c:%c%c", strPrxyOwnr[0],strPrxyOwnr[1],strPrxyOwnr[2]
			,strPrxyOwnr[3],strPrxyOwnr[4],strPrxyOwnr[5],strPrxyOwnr[6],strPrxyOwnr[7],strPrxyOwnr[8]
			,strPrxyOwnr[9],strPrxyOwnr[10],strPrxyOwnr[11]);
		req_format_write(wp, ("<html><! Copyright (c) Realtek Semiconductor Corp., 2003~2005. All Rights Reserved. ->\n"));
		req_format_write(wp, ("<head><meta http-equiv=\"Content-Type\" content=\"text/html\">\n"));
		//req_format_write(wp, ("<script type=\"text/javascript\" src=\"util_gw.js\"></script>\n"));
		req_format_write(wp, ("<title>Proxy Table</title></head>\n"));
		req_format_write(wp, ("<blockquote><h2><font color=\"#0000FF\">Active Client Table - %s</font></h2>\n"), macstr);
		req_format_write(wp, ("<body><form action=/boafrm/formMeshProxy method=POST name=\"formMeshProxy\">\n"));
		req_format_write(wp, ("<table border=0 width=550 cellspacing=4 cellpadding=0>\n"));
		req_format_write(wp, ("<tr><font size=2>\n"));
		req_format_write(wp, ("This table shows the MAC address for each proxied wired or wireless client\n"));
		req_format_write(wp, ("</font></tr>\n"));
		req_format_write(wp, ("<tr><hr size=1 noshade align=top></tr></table>\n"));
		
		
		req_format_write(wp, ("<table border=1 width=200>\n"));
		//req_format_write(wp, ("<tr><font size=4><b>Proxy Table </b></font></tr>\n"));
		
				
		req_format_write(wp, ("<tr bgcolor=\"#7F7F7F\">"
		//"<td align=center width=\"50%%\"><font size=\"2\"><b>MP MAC Address</b></font></td>\n"
		"<td align=center><font size=\"2\"><b>Client MAC Address</b></font></td></tr>\n"));
		
		fh = fopen(_FILE_MESH_PROXY , "r");
		if (!fh)
		{
				printf("Warning: cannot open %s\n",_FILE_MESH_PROXY );
				return -1;
		}
		
		while( fgets(buf, sizeof buf, fh) != NULL )
		{
			if( strstr(buf,"table info...") != NULL )
			{
				_get_token( fh,"STA_MAC: ",sta );
				_get_token( fh,"OWNER_MAC: ",owner );
				strtolower(owner, 12);
				if (!strncmp(strPrxyOwnr,owner,12)){
					req_format_write(wp,("<tr bgcolor=\"#b7b7b7\">"
							"<td align=center width=\"50%%\"><font size=\"2\">%s</td>\n"),sta);
					nRecordCount++;
				}
			}
		}
		
		fclose(fh);
		
		if(nRecordCount == 0)
		{
			req_format_write(wp,("<tr bgcolor=\"#b7b7b7\">"
					"<td align=center width=\"17%%\"><font size=\"2\">None</td>\n"));
		}
				
		req_format_write(wp,("</tr></table>\n"));
		req_format_write(wp,("<input type=\"hidden\" value=\"%s\" name=\"owner\">\n"), strPrxyOwnr);
		req_format_write(wp,("<p><input type=\"submit\" value=\"Refresh\" name=\"refresh\">&nbsp;&nbsp;\n"));
		req_format_write(wp,("<input type=\"button\" value=\" Close \" name=\"close\" onClick=\"javascript: window.close();\"><p>\n"));
		req_format_write(wp,("</form>\n"));
		req_format_write(wp, ("</blockquote></body></html>"));
	}
}

void formMeshSetup(request *wp, char *path, char *query)
{
        char *submitUrl,*meshRootEnabled,*refresh, *strMeshID, *strEnabled;
        int enabled,meshenable=0;
        char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};
        int warn=0;
		
#ifdef CONFIG_NEW_MESH_UI
		#if 1
		meshRootEnabled = req_get_cstream_var(wp, ("meshRootEnabled"), "");
		#else
		meshRootEnabled = "ON";
		#endif
#else
        meshRootEnabled = req_get_cstream_var(wp, ("meshRootEnabled"), "");
#endif
        strMeshID = req_get_cstream_var(wp, ("meshID"), "");
        submitUrl = req_get_cstream_var(wp, ("mesh-url"), "");   // hidden page
        refresh = req_get_cstream_var(wp, ("refresh"), "");
		//new feature:Mesh enable/disable
		strEnabled = req_get_cstream_var(wp, ("wlanMeshEnable"), "");

		// refresh button response
        if ( refresh[0] )
        {
        		send_redirect_perm(wp, submitUrl);
                return;
        }
		
		if ( !strcmp(strEnabled, "ON"))
			meshenable = 1;
		else
			meshenable = 0;

		if ( apmib_set(MIB_MESH_ENABLE, (void *)&meshenable) == 0)
        {
                strcpy( tmpBuf, ("Set mesh enable error!"));
                goto setErr_mesh;
        }

		if( !meshenable )
			goto setupEnd;

		// backbone privacy settings
		
		if(meshWpaHandler(wp, tmpBuf, wlan_idx) < 0)
			goto setErr_mesh;
		
#ifdef CONFIG_NEW_MESH_UI
	if(!strcmp(meshRootEnabled, "ON"))
            enabled = 1 ;
    else
            enabled = 0 ;
#else
        if(!strcmp(meshRootEnabled, "ON"))
                enabled = 1 ;
        else
                enabled = 0 ;
#endif
        if ( apmib_set(MIB_MESH_ROOT_ENABLE, (void *)&enabled) == 0)
        {
                strcpy( tmpBuf, ("Set mesh Root enable error!"));
                goto setErr_mesh;
        }

        if (strMeshID[0])
        {
//              if (strlen(strMeshID)!=12 || !string_to_hex(strMeshID, tmpBuf, 12)) {
                if (strlen(strMeshID)>32) {
                        strcpy(tmpBuf, ("Error! Invalid Mesh ID."));
                        goto setErr_mesh;
                }
                if ( apmib_set(MIB_MESH_ID, (void *)strMeshID) == 0)
                {
                        strcpy(tmpBuf, ("Set MIB_MESH_ID error!"));
                        goto setErr_mesh;
                }
        }
setupEnd:
        apmib_update(CURRENT_SETTING);

#ifndef NO_ACTION
        run_init_script("bridge");
#endif

        submitUrl = req_get_cstream_var(wp, ("mesh-url"), "");   // hidden page
        if (warn) {
                OK_MSG1(tmpBuf, submitUrl);
        }
        else {
#ifdef REBOOT_CHECK
		OK_MSG(submitUrl);
#else
		RECONNECT_MSG(submitUrl);       // display reconnect msg to remote
#endif
        }
        return;

setErr_mesh:
        ERR_MSG(tmpBuf);
}

#endif // CONFIG_RTK_MESH

/////////////////////////////////////////////////////////////////////////////
#if defined(CONFIG_RTL_92D_SUPPORT)

#if 0
void swapWlanMibSetting(unsigned char wlanifNumA, unsigned char wlanifNumB)
{
	unsigned char *wlanMibBuf=NULL;
	unsigned int totalSize = sizeof(CONFIG_WLAN_SETTING_T)*(NUM_VWLAN_INTERFACE+1); // 4vap+1rpt+1root
	wlanMibBuf = malloc(totalSize); 
#if 0	
	printf("\r\n wlanifNumA=[%u],__[%s-%u]\r\n",wlanifNumA,__FILE__,__LINE__);
	printf("\r\n wlanifNumB=[%u],__[%s-%u]\r\n",wlanifNumB,__FILE__,__LINE__);
	
	printf("\r\n pMib->wlan[wlanifNumA]=[0x%x],__[%s-%u]\r\n",pMib->wlan[wlanifNumA],__FILE__,__LINE__);
	printf("\r\n pMib->wlan[wlanifNumB]=[0x%x],__[%s-%u]\r\n",pMib->wlan[wlanifNumB],__FILE__,__LINE__);
	
	printf("\r\n pMib->wlan[0][0].wlanDisabled=[%u],__[%s-%u]\r\n",pMib->wlan[0][0].wlanDisabled,__FILE__,__LINE__);
	printf("\r\n pMib->wlan[0][0].phyBandSelect=[%u],__[%s-%u]\r\n",pMib->wlan[0][0].phyBandSelect,__FILE__,__LINE__);
	printf("\r\n pMib->wlan[0][0].channel=[%u],__[%s-%u]\r\n",pMib->wlan[0][0].channel,__FILE__,__LINE__);
	
	printf("\r\n pMib->wlan[1][0].wlanDisabled=[%u],__[%s-%u]\r\n",pMib->wlan[1][0].wlanDisabled,__FILE__,__LINE__);
	printf("\r\n pMib->wlan[1][0].phyBandSelect=[%u],__[%s-%u]\r\n",pMib->wlan[1][0].phyBandSelect,__FILE__,__LINE__);
	printf("\r\n pMib->wlan[1][0].channel=[%u],__[%s-%u]\r\n",pMib->wlan[1][0].channel,__FILE__,__LINE__);
#endif			
	if(wlanMibBuf != NULL)
	{
		memcpy(wlanMibBuf, pMib->wlan[wlanifNumA], totalSize);
		memcpy(pMib->wlan[wlanifNumA], pMib->wlan[wlanifNumB], totalSize);
		memcpy(pMib->wlan[wlanifNumB], wlanMibBuf, totalSize);
	
		free(wlanMibBuf);
	}
	
#if 0	
	printf("\r\n pMib->wlan[0][0].wlanDisabled=[%u],__[%s-%u]\r\n",pMib->wlan[0][0].wlanDisabled,__FILE__,__LINE__);
	printf("\r\n pMib->wlan[0][0].phyBandSelect=[%u],__[%s-%u]\r\n",pMib->wlan[0][0].phyBandSelect,__FILE__,__LINE__);
	printf("\r\n pMib->wlan[0][0].channel=[%u],__[%s-%u]\r\n",pMib->wlan[0][0].channel,__FILE__,__LINE__);
	
	printf("\r\n pMib->wlan[1][0].wlanDisabled=[%u],__[%s-%u]\r\n",pMib->wlan[1][0].wlanDisabled,__FILE__,__LINE__);
	printf("\r\n pMib->wlan[1][0].phyBandSelect=[%u],__[%s-%u]\r\n",pMib->wlan[1][0].phyBandSelect,__FILE__,__LINE__);
	printf("\r\n pMib->wlan[1][0].channel=[%u],__[%s-%u]\r\n",pMib->wlan[1][0].channel,__FILE__,__LINE__);
#endif	
#ifdef UNIVERSAL_REPEATER
	int rptEnable1, rptEnable2;
	char rptSsid1[MAX_SSID_LEN], rptSsid2[MAX_SSID_LEN];
	
	memset(rptSsid1, 0x00, MAX_SSID_LEN);
	memset(rptSsid2, 0x00, MAX_SSID_LEN);
	
	apmib_get(MIB_REPEATER_ENABLED1, (void *)&rptEnable1);
	apmib_get(MIB_REPEATER_ENABLED2, (void *)&rptEnable2);
	apmib_get(MIB_REPEATER_SSID1, (void *)rptSsid1);
	apmib_get(MIB_REPEATER_SSID2, (void *)rptSsid2);
	
	apmib_set(MIB_REPEATER_ENABLED1, (void *)&rptEnable2);
	apmib_set(MIB_REPEATER_ENABLED2, (void *)&rptEnable1);
	apmib_set(MIB_REPEATER_SSID1, (void *)rptSsid2);
	apmib_set(MIB_REPEATER_SSID2, (void *)rptSsid1);
#endif
#if VLAN_CONFIG_SUPPORTED 
	unsigned char *vlanMibBuf=NULL;
	totalSize = sizeof(VLAN_CONFIG_T)*5; // 4vap+1root
	vlanMibBuf = malloc(totalSize);
	if(vlanMibBuf != NULL)
	{
		memcpy(vlanMibBuf, pMib->VlanConfigArray+4, totalSize);
		memcpy(pMib->VlanConfigArray+4, pMib->VlanConfigArray+9, totalSize);
		memcpy(pMib->VlanConfigArray+9, vlanMibBuf, totalSize);
	
		free(vlanMibBuf);
	}
	
#endif
}
#endif

void formWlanBand2G5G(request *wp, char *path, char *query)
{
	char *submitUrl;
	char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};
	char *tmpStr;
	int wlanBand2G5GSelect;
	char lan_ip_buf[30], lan_ip[30];
	int i;
	
//displayPostDate(wp->post_data);
	
	submitUrl = req_get_cstream_var(wp, ("submit-url"), "");   // hidden page

	tmpStr = req_get_cstream_var(wp, ("wlBandMode"), "");  
	if(tmpStr[0]){
		wlanBand2G5GSelect = atoi(tmpStr);
	}
	if(wlanBand2G5GSelect<BANDMODE2G || wlanBand2G5GSelect>BANDMODESINGLE)
	{
		goto setErr;
	}
	else
	{	
		apmib_set(MIB_WLAN_BAND2G5G_SELECT,(void *)&wlanBand2G5GSelect);
	}
	
	/* init all wireless interface is set radio off and DMACDPHY */
	for(i=0 ; i<NUM_WLAN_INTERFACE ; i++)
	{
		unsigned char wlanif[10];
		memset(wlanif,0x00,sizeof(wlanif));
		sprintf(wlanif, "wlan%d",i);
		if(SetWlan_idx(wlanif))
		{
			int intVal;
			intVal = DMACDPHY;
			apmib_set(MIB_WLAN_MAC_PHY_MODE, (void *)&intVal);
			intVal = 1;
			apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&intVal);
		}						
	}
	
	/* Set expect wireless interface is radio on and SMACSPHY */
	if(wlanBand2G5GSelect == BANDMODE2G)
	{
		short wlanif;
		unsigned char wlanIfStr[10];
		int intVal=0;			
		wlanif = whichWlanIfIs(PHYBAND_2G);
		
		memset(wlanIfStr,0x00,sizeof(wlanIfStr));		
		sprintf(wlanIfStr, "wlan%d",wlanif);
		
		if(SetWlan_idx(wlanIfStr))
		{
			int val;
			val = 0;
			apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&val);
			val = SMACSPHY;
			apmib_set(MIB_WLAN_MAC_PHY_MODE, (void *)&val);
		}
		
		/* we can't up wlan1 alone, so we swap wlan0 and wlan1 settings */
		if(wlanif != 0)
		{
			swapWlanMibSetting(0,wlanif);			
		}	
		
		intVal=0;
		apmib_set(MIB_WISP_WAN_ID, (void *)&intVal);
	}
	else if(wlanBand2G5GSelect == BANDMODE5G)
	{
		short wlanif;
		unsigned char wlanIfStr[10];
		int intVal=0;		
		wlanif = whichWlanIfIs(PHYBAND_5G);
		
		memset(wlanIfStr,0x00,sizeof(wlanIfStr));		
		sprintf(wlanIfStr, "wlan%d",wlanif);
		
		if(SetWlan_idx(wlanIfStr))
		{
			int val;
			val = 0;
			apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&val);
			val = SMACSPHY;
			apmib_set(MIB_WLAN_MAC_PHY_MODE, (void *)&val);
		}
		
		/* we can't up wlan1 alone, so we swap wlan0 and wlan1 settings */
		if(wlanif != 0)
		{
			swapWlanMibSetting(0,wlanif);			
		}	

		intVal=0;
		apmib_set(MIB_WISP_WAN_ID, (void *)&intVal);
	}
	/* Set both wireless interface is radio on and DMACDPHY */
	else if(wlanBand2G5GSelect == BANDMODEBOTH)
	{
		short wlanif;
		
		for(i=0 ; i<NUM_WLAN_INTERFACE ; i++)
		{
			unsigned char wlanif[10];
			memset(wlanif,0x00,sizeof(wlanif));
			sprintf(wlanif, "wlan%d",i);
			if(SetWlan_idx(wlanif))
			{
				int intVal;
				intVal = DMACDPHY;
				apmib_set(MIB_WLAN_MAC_PHY_MODE, (void *)&intVal);
				intVal = 0;
				apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&intVal);
			}
		}
				
		unsigned char wlanIfStr[10];				
		wlanif = whichWlanIfIs(PHYBAND_5G);
		
		/* 92d rule, 5g must up in wlan0 */
		/* phybandcheck */
		if(wlanif != 0)
		{
			swapWlanMibSetting(0,1);			
		}
	}							
	if(wlanBand2G5GSelect == BANDMODESINGLE)
	{
		int intVal=0;
		for(i=0 ; i<NUM_WLAN_INTERFACE ; i++)
		{
			unsigned char wlanif[10];
			memset(wlanif,0x00,sizeof(wlanif));
			sprintf(wlanif, "wlan%d",i);
			if(SetWlan_idx(wlanif))
			{
				int intVal;
				intVal = SMACSPHY;
				apmib_set(MIB_WLAN_MAC_PHY_MODE, (void *)&intVal);				
			}
		}
		
		SetWlan_idx("wlan0");
		intVal = 0;
		apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&intVal);

		SetWlan_idx("wlan1");
		intVal = 1;
		apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&intVal);
		
		intVal=0;
		apmib_set(MIB_WISP_WAN_ID, (void *)&intVal);
	}							
	/* set wlan index to 0 to avoid get wrong index when singleband*/
	wlan_idx = 0;
	apmib_update_web(CURRENT_SETTING);
	
#ifdef REBOOT_CHECK
	run_init_script_flag = 1;
#endif		


#ifndef NO_ACTION
	run_init_script("all");
#endif
	apmib_get( MIB_IP_ADDR,  (void *)lan_ip_buf) ;
  sprintf(lan_ip,"%s",inet_ntoa(*((struct in_addr *)lan_ip_buf)) );
  	
  sprintf(tmpBuf,"%s","<h4>Change setting successfully!<BR><BR>Do not turn off or reboot the Device during this time.</h4>");
	OK_MSG_FW(tmpBuf, submitUrl,APPLY_COUNTDOWN_TIME,lan_ip);
return;

setErr:
	ERR_MSG(tmpBuf);
}
#endif //#if defined(CONFIG_RTL_92D_SUPPORT)

void formWlanSetup(request *wp, char *path, char *query)
{
	char *submitUrl;
	char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};
	int mode=-1;
	int warn=0;
	int val;	
	char *strVal=NULL;	
#if defined(CONFIG_RTL_92D_SUPPORT)
	int wlanif=0;
	
	PHYBAND_TYPE_T phyBandSelect = PHYBAND_OFF; 
	int wlanBand2G5GSelect=PHYBAND_OFF;
#endif
//displayPostDate(wp->post_data);	

#if defined(CONFIG_RTL_92D_SUPPORT)		
	apmib_get(MIB_WLAN_BAND2G5G_SELECT,(void *)&wlanBand2G5GSelect);

	if(wlanBand2G5GSelect == BANDMODESINGLE)
	{

		
		strVal=req_get_cstream_var(wp,("Band2G5GSupport"),"");
		if(strVal[0])
		{
			
			phyBandSelect= atoi(strVal);		
			wlanif = whichWlanIfIs(phyBandSelect);			
				

			if(wlanif != 0)
			{

				val = 1;
				apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&val); //close original interface

//printf("\r\n __[%s-%u]\r\n",__FILE__,__LINE__);				
				swapWlanMibSetting(0,wlanif);
//printf("\r\n __[%s-%u]\r\n",__FILE__,__LINE__);

				val = 0;
				apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&val); //enable after interface
				//apmib_update_web(CURRENT_SETTING);
			}
		}
		
#if defined(CONFIG_RTL_P2P_SUPPORT)		
		char varName[20];
		sprintf(varName, "mode%d", wlan_idx);
		strVal = req_get_cstream_var(wp, varName, "");
	
		if(strVal[0] == NULL)
		{
			char strtmp[20];
			
			apmib_get( MIB_WLAN_MODE, (void *)&val);
			sprintf(strtmp,"%d",val);
			strVal = strtmp;		
		}
		
		val=atoi(strVal);
		
		if(val == P2P_SUPPORT_MODE)
		{
			int ori_wlan_idx = wlan_idx;
			int val2 = 0;
			apmib_set( MIB_DHCP, (void *)&val2);
			
			wlan_idx = 0;
			apmib_set( MIB_WLAN_MODE, (void *)&val);
			
			wlan_idx = 1;
			apmib_set( MIB_WLAN_MODE, (void *)&val);
			
			wlan_idx = ori_wlan_idx;
			
		}
#endif //#if defined(CONFIG_RTL_P2P_SUPPORT)
		
	}
#endif //#if defined(CONFIG_RTL_92D_SUPPORT)

//printf("\r\n __[%s-%u]\r\n",__FILE__,__LINE__);
#if defined(WLAN_PROFILE)
	if(wlProfileHandler(wp, tmpBuf, wlan_idx) < 0)
	{
//printf("\r\n __[%s-%u]\r\n",__FILE__,__LINE__);	
		goto setErr_wlan;
	}

#endif //#if defined(WLAN_PROFILE)	

	if(wlanHandler(wp, tmpBuf, &mode, wlan_idx) < 0)
		goto setErr_wlan ;
	if (mode == 1) { // not AP mode
		//set cipher suit to AES and encryption to wpa2 only if wpa2 mixed mode is set
		ENCRYPT_T encrypt;
		int intVal;
		apmib_get( MIB_WLAN_ENCRYPT, (void *)&encrypt);
		if(encrypt == ENCRYPT_WPA2_MIXED){
			intVal =   WPA_CIPHER_AES ;
			if ( apmib_set(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set MIB_WLAN_WPA2_UNICIPHER failed!"));
				goto setErr_wlan;
			}
			encrypt = ENCRYPT_WPA2;
			if ( apmib_set(MIB_WLAN_ENCRYPT, (void *)&encrypt) == 0) {
				strcpy(tmpBuf, ("Set MIB_WLAN_WPA2_UNICIPHER failed!"));
				goto setErr_wlan;
			}

			intVal =   0;
			if ( apmib_set(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set MIB_WLAN_WPA_CIPHER_SUITE failed!"));
				goto setErr_wlan;
			}
			strcpy(tmpBuf, ("Warning! WPA2 Mixed encryption is not supported in client Mode. <BR> Change to WPA2 Encryption."));
			warn = 1;
		}
	}
	apmib_update_web(CURRENT_SETTING);

#ifndef NO_ACTION
	run_init_script("bridge");
#endif

	submitUrl = req_get_cstream_var(wp, ("wlan-url"), "");   // hidden page
	if (warn) {
		OK_MSG1(tmpBuf, submitUrl);
	}
	else {
		OK_MSG(submitUrl);
	}
	return;

setErr_wlan:
	ERR_MSG(tmpBuf);
}

int wepHandler(request *wp, char *tmpBuf, int wlan_id)
{
   	char  *wepKey;
   	char *strKeyLen, *strFormat, *strAuth, /* *strKeyId, */ *strEnabled;
	char key[30];
	int enabled, keyLen, ret, i;
	WEP_T wep;
	ENCRYPT_T encrypt=ENCRYPT_WEP;
	char varName[20];
	int wlanMode, rptEnabled;

//printf("\r\n wlan_idx=[%d],vwlan_idx=[%d],__[%s-%u]\r\n",wlan_idx,vwlan_idx,__FILE__,__LINE__);	
//displayPostDate(wp->post_data);

#if 0 //NO need to change authType in client mode??? Keith
	apmib_get( MIB_WLAN_MODE, (void *)&wlanMode);
	if(wlan_id == 0)
		apmib_get( MIB_REPEATER_ENABLED1, (void *)&rptEnabled);
	else
		apmib_get( MIB_REPEATER_ENABLED2, (void *)&rptEnabled);

//printf("\r\n wlanMode=[%d],rptEnabled=[%d],__[%s-%u]\r\n",wlanMode,rptEnabled,__FILE__,__LINE__);	

	if(wlanMode == CLIENT_MODE || ((wlanMode == AP_MODE || wlanMode == AP_WDS_MODE) && rptEnabled == 1))
	{
		int auahType;
		
		apmib_get( MIB_WLAN_AUTH_TYPE, (void *)&auahType);
//printf("\r\n auahType=[%d],__[%s-%u]\r\n",auahType,__FILE__,__LINE__);	

		if(auahType == AUTH_BOTH)
		{
			auahType= AUTH_OPEN;
//printf("\r\n auahType=[%d],__[%s-%u]\r\n",auahType,__FILE__,__LINE__);				
			apmib_set( MIB_WLAN_AUTH_TYPE, (void *)&auahType);
		}
		
	}
#endif //#if 0 //NO need to change authType in client mode??? Keith

#ifdef WIFI_SIMPLE_CONFIG
#ifdef MBSSID
	if (vwlan_idx == 0)
#endif
	{
		memset(&wps_config_info, 0, sizeof(struct wps_config_info_struct));
		wps_config_info.caller_id = CALLED_FROM_WEPHANDLER;
		apmib_get(MIB_WLAN_ENCRYPT, (void *)&wps_config_info.auth);
		apmib_get(MIB_WLAN_WEP, (void *)&wps_config_info.wep_enc);
		apmib_get(MIB_WLAN_WEP_DEFAULT_KEY, (void *)&wps_config_info.KeyId);
		apmib_get(MIB_WLAN_WEP64_KEY1, (void *)wps_config_info.wep64Key1);
		apmib_get(MIB_WLAN_WEP64_KEY2, (void *)wps_config_info.wep64Key2);
		apmib_get(MIB_WLAN_WEP64_KEY3, (void *)wps_config_info.wep64Key3);
		apmib_get(MIB_WLAN_WEP64_KEY4, (void *)wps_config_info.wep64Key4);
		apmib_get(MIB_WLAN_WEP128_KEY1, (void *)wps_config_info.wep128Key1);
		apmib_get(MIB_WLAN_WEP128_KEY2, (void *)wps_config_info.wep128Key2);
		apmib_get(MIB_WLAN_WEP128_KEY3, (void *)wps_config_info.wep128Key3);
		apmib_get(MIB_WLAN_WEP128_KEY4, (void *)wps_config_info.wep128Key4);
	}
#endif

	sprintf(varName, "wepEnabled%d", wlan_id);
	strEnabled = req_get_cstream_var(wp, varName, "");
	if ( !strcmp(strEnabled, "ON"))
		enabled = 1;
	else
		enabled = 0;

	if ( enabled ) {
		sprintf(varName, "length%d", wlan_id);
		strKeyLen = req_get_cstream_var(wp, varName, "");
		if (!strKeyLen[0]) {
 			strcpy(tmpBuf, ("Key length must exist!"));
			goto setErr_wep;
		}
		if (strKeyLen[0]!='1' && strKeyLen[0]!='2') {
 			strcpy(tmpBuf, ("Invalid key length value!"));
			goto setErr_wep;
		}
		if (strKeyLen[0] == '1')
			wep = WEP64;
		else
			wep = WEP128;
	}
	else
		wep = WEP_DISABLED;

	if ( apmib_set( MIB_WLAN_WEP, (void *)&wep) == 0) {
  		strcpy(tmpBuf, ("Set WEP MIB error!"));
		goto setErr_wep;
	}

	if (wep == WEP_DISABLED)
		encrypt = ENCRYPT_DISABLED;

	if (apmib_set( MIB_WLAN_ENCRYPT, (void *)&encrypt) == 0) {
		strcpy(tmpBuf, ("Set MIB_WLAN_ENCRYPT mib error!"));
		goto setErr_wep;
	}

	if (wep == WEP_DISABLED)
		return 0 ;


	sprintf(varName, "authType%d", wlan_id);
	strAuth = req_get_cstream_var(wp, varName, "");
	if (strAuth[0]) { // new UI
		int authType;
		if (!strcmp(strAuth, ("open")))
			authType = AUTH_OPEN;
		else if ( !strcmp(strAuth, ("shared")))
			authType = AUTH_SHARED;
		else 
			authType = AUTH_BOTH;
		apmib_set(MIB_WLAN_AUTH_TYPE, (void *)&authType);
	}
				
	sprintf(varName, "format%d", wlan_id);
	strFormat = req_get_cstream_var(wp, varName, "");
	if (!strFormat[0]) {
 		strcpy(tmpBuf, ("Key type must exist!"));
		goto setErr_wep;
	}

	if (strFormat[0]!='1' && strFormat[0]!='2') {
		strcpy(tmpBuf, ("Invalid key type value!"));
		goto setErr_wep;
	}

	i = strFormat[0] - '0' - 1;
	if ( apmib_set( MIB_WLAN_WEP_KEY_TYPE, (void *)&i) == 0) {
  		strcpy(tmpBuf, ("Set WEP key type error!"));
		goto setErr_wep;
	}

	if (wep == WEP64) {
		if (strFormat[0]=='1')
			keyLen = WEP64_KEY_LEN;
		else
			keyLen = WEP64_KEY_LEN*2;
	}
	else {
		if (strFormat[0]=='1')
			keyLen = WEP128_KEY_LEN;
		else
			keyLen = WEP128_KEY_LEN*2;
	}
	
		sprintf(varName, "key%d", wlan_id);
	wepKey = req_get_cstream_var(wp, varName, "");
	if  (wepKey[0]) {
		if (strlen(wepKey) != keyLen) {
			sprintf(tmpBuf, ("Invalid key length! Expect length is %d"), keyLen);
			goto setErr_wep;
		}
		if ( !isAllStar(wepKey) ) {
			if (strFormat[0] == '1') // ascii
				strcpy(key, wepKey);
			else { // hex
				if ( !string_to_hex(wepKey, key, keyLen)) {
	   				strcpy(tmpBuf, ("Invalid wep-key value!"));
					goto setErr_wep;
				}
			}
			if (wep == WEP64){
				ret=apmib_set(MIB_WLAN_WEP64_KEY1, (void *)key);
				ret=apmib_set(MIB_WLAN_WEP64_KEY2, (void *)key);
				ret=apmib_set(MIB_WLAN_WEP64_KEY3, (void *)key);
				ret=apmib_set(MIB_WLAN_WEP64_KEY4, (void *)key);
			}else{
				ret=apmib_set(MIB_WLAN_WEP128_KEY1, (void *)key);
				ret=apmib_set(MIB_WLAN_WEP128_KEY2, (void *)key);
				ret=apmib_set(MIB_WLAN_WEP128_KEY3, (void *)key);
				ret=apmib_set(MIB_WLAN_WEP128_KEY4, (void *)key);
			}
			if (!ret) {
	 			strcpy(tmpBuf, ("Set wep-key error!"));
				goto setErr_wep;
			}
		}
	}

	
	
#if 0
	sprintf(varName, "defaultTxKeyId%d", wlan_id);
	strKeyId = req_get_cstream_var(wp, varName, "");
	if ( strKeyId[0] ) {
		if ( strKeyId[0]!='1' && strKeyId[0]!='2' && strKeyId[0]!='3' && strKeyId[0]!='4' ) {
	 		strcpy(tmpBuf, ("Invalid default tx key id!"));
   			goto setErr_wep;
		}
		i = strKeyId[0] - '0' - 1;
		if ( !apmib_set( MIB_WLAN_WEP_DEFAULT_KEY, (void *)&i ) ) {
	 		strcpy(tmpBuf, ("Set default tx key id error!"));
   			goto setErr_wep;
		}
	}

	sprintf(varName, "key1%d", wlan_id);
	wepKey = req_get_cstream_var(wp, varName, "");
	if  (wepKey[0]) {
		if (strlen(wepKey) != keyLen) {
			strcpy(tmpBuf, ("Invalid key 1 length!"));
			goto setErr_wep;
		}
		if ( !isAllStar(wepKey) ) {
			if (strFormat[0] == '1') // ascii
				strcpy(key, wepKey);
			else { // hex
				if ( !string_to_hex(wepKey, key, keyLen)) {
	   				strcpy(tmpBuf, ("Invalid wep-key1 value!"));
					goto setErr_wep;
				}
			}
			if (wep == WEP64)
				ret=apmib_set(MIB_WLAN_WEP64_KEY1, (void *)key);
			else
				ret=apmib_set(MIB_WLAN_WEP128_KEY1, (void *)key);
			if (!ret) {
	 			strcpy(tmpBuf, ("Set wep-key1 error!"));
				goto setErr_wep;
			}
		}
	}
	sprintf(varName, "key2%d", wlan_id);
	wepKey = req_get_cstream_var(wp, varName, "");
	if  (wepKey[0]) {
		if (strlen(wepKey) != keyLen) {
			strcpy(tmpBuf, ("Invalid key 2 length!"));
			goto setErr_wep;
		}
		if ( !isAllStar(wepKey) ) {
			if (strFormat[0] == '1') // ascii
				strcpy(key, wepKey);
			else { // hex
				if ( !string_to_hex(wepKey, key, keyLen)) {
	   				strcpy(tmpBuf, ("Invalid wep-key2 value!"));
   					goto setErr_wep;
				}
			}
			if (wep == WEP64)
				ret=apmib_set(MIB_WLAN_WEP64_KEY2, (void *)key);
			else
				ret=apmib_set(MIB_WLAN_WEP128_KEY2, (void *)key);
			if (!ret) {
	 			strcpy(tmpBuf, ("Set wep-key2 error!"));
				goto setErr_wep;
			}
		}
	}

	sprintf(varName, "key3%d", wlan_id);
	wepKey = req_get_cstream_var(wp, varName, "");
	if  (wepKey[0]) {
		if (strlen(wepKey) != keyLen) {
			strcpy(tmpBuf, ("Invalid key 3 length!"));
			goto setErr_wep;
		}
		if ( !isAllStar(wepKey) ) {
			if (strFormat[0] == '1') // ascii
				strcpy(key, wepKey);
			else { // hex
				if ( !string_to_hex(wepKey, key, keyLen)) {
	   				strcpy(tmpBuf, ("Invalid wep-key3 value!"));
   					goto setErr_wep;
				}
			}
			if (wep == WEP64)
				ret=apmib_set(MIB_WLAN_WEP64_KEY3, (void *)key);
			else
				ret=apmib_set(MIB_WLAN_WEP128_KEY3, (void *)key);
			if (!ret) {
	 			strcpy(tmpBuf, ("Set wep-key3 error!"));
				goto setErr_wep;
			}
		}
	}

	sprintf(varName, "key4%d", wlan_id);
	wepKey = req_get_cstream_var(wp, varName, "");
	if  (wepKey[0]) {
		if (strlen(wepKey) != keyLen) {
			strcpy(tmpBuf, ("Invalid key 1 length!"));
			goto setErr_wep;
		}
		if ( !isAllStar(wepKey) ) {
			if (strFormat[0] == '1') // ascii
				strcpy(key, wepKey);
			else { // hex
				if ( !string_to_hex(wepKey, key, keyLen)) {
	   				strcpy(tmpBuf, ("Invalid wep-key4 value!"));
   					goto setErr_wep;
				}
			}
			if (wep == WEP64)
				ret=apmib_set(MIB_WLAN_WEP64_KEY4, (void *)key);
			else
				ret=apmib_set(MIB_WLAN_WEP128_KEY4, (void *)key);
			if (!ret) {
	 			strcpy(tmpBuf, ("Set wep-key4 error!"));
				goto setErr_wep;
			}
		}
	}	
#endif
#ifdef WIFI_SIMPLE_CONFIG
	#ifdef MBSSID
	if (vwlan_idx == 0)
#endif
	{
		sprintf(varName, "wps_clear_configure_by_reg%d", wlan_id);
		wepKey = req_get_cstream_var(wp, varName, NULL);
		ret = 0;
		if (wepKey[0])
			ret = atoi(wepKey);
		update_wps_configured(ret);
	}
#endif

#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)
	if (vwlan_idx == NUM_VWLAN_INTERFACE)
	{
		sprintf(varName, "wps_clear_configure_by_reg%d", wlan_id);
		wepKey = req_get_cstream_var(wp, varName, NULL);
		ret = 0;
		if (wepKey[0])
			ret = atoi(wepKey);
		update_wps_configured(ret);
	}
#endif

	return 0 ;
setErr_wep:
	return -1 ;	
}	
/////////////////////////////////////////////////////////////////////////////
void formWep(request *wp, char *path, char *query)
{
	char *submitUrl;
	char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};


	if(wepHandler(wp, tmpBuf, wlan_idx) < 0 )
		goto setErr_end ;

	apmib_update_web(CURRENT_SETTING);

#ifndef NO_ACTION
	run_init_script("bridge");
#endif

	submitUrl = req_get_cstream_var(wp, ("submit-url"), "");   // hidden page
	OK_MSG(submitUrl);

	return;

setErr_end:
	ERR_MSG(tmpBuf);
}


int wpaHandler(request *wp, char *tmpBuf, int wlan_id)
{
   	char *strEncrypt, *strVal;
	ENCRYPT_T encrypt;
	int enableRS=0, intVal, getPSK=0, len, val;
	unsigned long reKeyTime;
	SUPP_NONWAP_T suppNonWPA;
	struct in_addr inIp;
	char varName[20];
#ifdef CONFIG_RTL_WAPI_SUPPORT
	int enableAS=0;
#endif

#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
	int wlan_mode;
	int intVal2;
	int wlanIdx_5G, wlanIdx_2G, rsBandSel;
#endif

	sprintf(varName, "method%d", wlan_id);
   	strEncrypt = req_get_cstream_var(wp, varName, "");
	if (!strEncrypt[0]) {
 		strcpy(tmpBuf, ("Error! no encryption method."));
		goto setErr_encrypt;
	}
	encrypt = (ENCRYPT_T) strEncrypt[0] - '0';
	if (encrypt!=ENCRYPT_DISABLED && encrypt!=ENCRYPT_WEP && encrypt!=ENCRYPT_WPA
		&& encrypt != ENCRYPT_WPA2 && encrypt != ENCRYPT_WPA2_MIXED
#ifdef CONFIG_RTL_WAPI_SUPPORT		
		&& encrypt != ENCRYPT_WAPI
#endif
) {
		strcpy(tmpBuf, ("Invalid encryption method!"));
		goto setErr_encrypt;
	}

#ifdef WIFI_SIMPLE_CONFIG
#ifdef MBSSID
	if (vwlan_idx == 0)
#endif
	{
		memset(&wps_config_info, 0, sizeof(struct wps_config_info_struct));
		wps_config_info.caller_id = CALLED_FROM_WPAHANDLER;
		apmib_get(MIB_WLAN_ENCRYPT, (void *)&wps_config_info.auth);
		apmib_get(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&wps_config_info.wpa_enc);
		apmib_get(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&wps_config_info.wpa2_enc);
		apmib_get(MIB_WLAN_WPA_PSK, (void *)wps_config_info.wpaPSK);
	}
#endif

	if (apmib_set( MIB_WLAN_ENCRYPT, (void *)&encrypt) == 0) {
  		strcpy(tmpBuf, ("Set MIB_WLAN_ENCRYPT mib error!"));
		goto setErr_encrypt;
	}

	if (encrypt == ENCRYPT_DISABLED || encrypt == ENCRYPT_WEP) {
		sprintf(varName, "use1x%d", wlan_id);
		strVal = req_get_cstream_var(wp, varName, "");
		if ( !strcmp(strVal, "ON")) {
			apmib_get( MIB_WLAN_MODE, (void *)&intVal);
			if (intVal !=AP_MODE && intVal != AP_WDS_MODE) { // not AP mode
#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
				if(intVal == CLIENT_MODE){//client mode
//					printf("%s(%d): WPA-RADIUS can be used when device is set to client mode\n",__FUNCTION__,__LINE__);//Added for test 
					intVal = 1;
					enableRS = 1;
				}
				else{
					strcpy(tmpBuf, ("Error! 802.1x authentication cannot be used when device is set to wds or mesh mode."));
					goto setErr_encrypt;
					intVal = 0;
				}
#else
				strcpy(tmpBuf, ("Error! 802.1x authentication cannot be used when device is set to client mode."));
				goto setErr_encrypt;
				intVal = 0;				
#endif
			}
			else {
				intVal = 1;
				enableRS = 1;
			}
		}
		else
			intVal = 0;

		if ( apmib_set( MIB_WLAN_ENABLE_1X, (void *)&intVal) == 0) {
  			strcpy(tmpBuf, ("Set 1x enable flag error!"));
			goto setErr_encrypt;
		}

		if (encrypt == ENCRYPT_WEP) {
	 		WEP_T wep;
			if ( !apmib_get( MIB_WLAN_WEP,  (void *)&wep) ) {
				strcpy(tmpBuf, ("Get MIB_WLAN_WEP MIB error!"));
				goto setErr_encrypt;
			}
			if (wep == WEP_DISABLED) {
				wep = WEP64;
				if ( apmib_set( MIB_WLAN_WEP, (void *)&wep) == 0) {
		  			strcpy(tmpBuf, ("Set WEP MIB error!"));
					goto setErr_encrypt;
				}
			}
		}
		else {
			sprintf(varName, "useMacAuth%d", wlan_id);
			strVal = req_get_cstream_var(wp, varName, "");
			if ( !strcmp(strVal, "ON")) {
				intVal = 1;
				enableRS = 1;
			}
			else
				intVal = 0;
			if ( apmib_set( MIB_WLAN_MAC_AUTH_ENABLED, (void *)&intVal) == 0) {
  				strcpy(tmpBuf, ("Set MIB_WLAN_MAC_AUTH_ENABLED MIB error!"));
				goto setErr_encrypt;
			}
		}
	}
#ifdef CONFIG_RTL_WAPI_SUPPORT	
	else if(ENCRYPT_WAPI==encrypt)
	{
		/*WAPI handle*/
		sprintf(varName, "wapiAuth%d", wlan_id);
		strVal = req_get_cstream_var(wp, varName, "");
		if (strVal[0]) 
		{
			if ( !strcmp(strVal, ("eap")))
			{
				apmib_get( MIB_WLAN_MODE, (void *)&intVal);
				if (intVal!=AP_MODE && intVal!=AP_WDS_MODE) { // not AP mode
					strcpy(tmpBuf, ("Error! WAPI AS cannot be used when device is set to client mode."));
					goto setErr_encrypt;
				}
				intVal = WAPI_AUTH_AUTO;
				enableAS = 1;
			}
			else if ( !strcmp(strVal, ("psk"))) 
			{
				intVal = WAPI_AUTH_PSK;
				getPSK = 1;
			}
			else 
			{
				strcpy(tmpBuf, ("Error! Invalid wapi authentication value."));
				goto setErr_encrypt;
			}

			if ( apmib_set(MIB_WLAN_WAPI_AUTH, (void *)&intVal) == 0) 
			{
				strcpy(tmpBuf, ("Set MIB_WLAN_WAPI_AUTH failed!"));
				goto setErr_encrypt;
			}
		}
		// pre-shared key
		if ( getPSK ) {
			int oldFormat, oldPskLen, i;

			sprintf(varName, "wapiPskFormat%d", wlan_id);
   			strVal = req_get_cstream_var(wp, varName, "");
			if (!strVal[0]) {
	 			strcpy(tmpBuf, ("Error! no psk format."));
				goto setErr_encrypt;
			}
			intVal = strVal[0] - '0';
			if (intVal != 0 && intVal != 1) {
	 			strcpy(tmpBuf, ("Error! invalid psk format."));
				goto setErr_encrypt;
			}

			// remember current psk format and length to compare to default case "****"
			apmib_get(MIB_WLAN_WAPI_PSK_FORMAT, (void *)&oldFormat);
			apmib_get(MIB_WLAN_WPA_PSK, (void *)tmpBuf);
			oldPskLen = strlen(tmpBuf);

			sprintf(varName, "wapiPskValue%d", wlan_id);
			strVal = req_get_cstream_var(wp, varName, "");
			len = strlen(strVal);

			if (oldFormat == intVal && len == oldPskLen ) {
				for (i=0; i<len; i++) {
					if ( strVal[i] != '*' )
						break;
				}
				if (i == len)
					goto wapi_end;
			}

			if ( apmib_set(MIB_WLAN_WAPI_PSK_FORMAT, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set MIB_WLAN_PSK_FORMAT failed!"));
				goto setErr_encrypt;
			}

			if (intVal==1) { // hex
				if (/*len!=MAX_PSK_LEN ||*/!string_to_hex(strVal, tmpBuf, MAX_PSK_LEN)) {
	 				strcpy(tmpBuf, ("Error! invalid psk value."));
					goto setErr_encrypt;
				}
				if(0 ==(len % 2))
				{
					len = len/2;
				}
				else
				{
					/*wapi hex key len should be even*/
					strcpy(tmpBuf, ("Error! invalid psk len."));
					goto setErr_encrypt;
				}					
				if(!apmib_set(MIB_WLAN_WAPI_PSKLEN,(void*)&len))
				{
					strcpy(tmpBuf,("Error! Set wapi key len fault"));
				}
			}
			else { // passphras
				if (len==0 || len > (MAX_PSK_LEN-1) ) {
	 				strcpy(tmpBuf, ("Error! invalid psk value."));
					goto setErr_encrypt;
				}
				if(!apmib_set(MIB_WLAN_WAPI_PSKLEN,(void*)&len))
				{
					strcpy(tmpBuf,("Error! Set wapi key len fault"));
				}
			}
			if ( !apmib_set(MIB_WLAN_WAPI_PSK, (void *)strVal)) {
				strcpy(tmpBuf, ("Set MIB_WLAN_WPA_PSK error!"));
				goto setErr_encrypt;
			}
		}
	wapi_end:
		/*save AS IP*/
		if(1==enableAS)
		{ 
			int old_vwlan_idx,i;
			sprintf(varName, "wapiASIP%d", wlan_id);
			strVal = req_get_cstream_var(wp, varName, "");
			if (!strVal[0]) {
				strcpy(tmpBuf, ("No WAPI AS address!"));
				goto setErr_encrypt;
			}
			if ( !inet_aton(strVal, &inIp) ) {
				strcpy(tmpBuf, ("Invalid AS IP-address value!"));
				goto setErr_encrypt;
			}

			sprintf(varName, "wapiCertSel%d", wlan_id);
			strVal = req_get_cstream_var(wp, varName, "");
			if (!strVal[0]) {
				strcpy(tmpBuf, ("No WAPI cert selected!"));
				goto setErr_encrypt;
			}
			intVal=atoi(strVal);

			// To record old vwlan_idx
			old_vwlan_idx=vwlan_idx;
			// Set current MIB_WLAN_WAPI_ASIPADDR and MIB_WLAN_WAPI_CERT_SEL to all wlan interfaces
			// root wlan interface and virtual wlan interface
			for(i=0;i<NUM_VWLAN_INTERFACE+1;i++)
			{
				vwlan_idx=i;
				if ( !apmib_set(MIB_WLAN_WAPI_ASIPADDR, (void *)&inIp)) {
					strcpy(tmpBuf, ("Set RS IP-address error!"));
					goto setErr_encrypt;
				}	
				if ( !apmib_set(MIB_WLAN_WAPI_CERT_SEL, (void *)&intVal)) {
					strcpy(tmpBuf, ("Set WAPI cert sel error!"));
					goto setErr_encrypt;
				}	
			}
			// Back to old vwlan_idx
			vwlan_idx=old_vwlan_idx;
		}
	}
#endif
	else {
		// support nonWPA client

		sprintf(varName, "nonWpaSupp%d", wlan_id);
 		strVal = req_get_cstream_var(wp, varName, "");
		apmib_get( MIB_WLAN_ENABLE_SUPP_NONWPA, (void *)&intVal);
		if(strVal[0])
		{
		if ( !strcmp(strVal, "ON"))
			intVal = 1;
		else
			intVal = 0;
		}
		if ( apmib_set( MIB_WLAN_ENABLE_SUPP_NONWPA, (void *)&intVal) == 0) {
  			strcpy(tmpBuf, ("Set MIB_WLAN_ENABLE_SUPP_NONWPA mib error!"));
			goto setErr_encrypt;
		}
		if ( intVal ) {
			suppNonWPA = SUPP_NONWPA_NONE;
			sprintf(varName, "nonWpaWep%d", wlan_id);
			strVal = req_get_cstream_var(wp, varName, "");
			if ( !strcmp(strVal, "ON"))
				suppNonWPA |= SUPP_NONWPA_WEP;

			sprintf(varName, "nonWpa1x%d", wlan_id);
			strVal = req_get_cstream_var(wp, varName, "");
			if ( !strcmp(strVal, "ON")) {
				suppNonWPA |= SUPP_NONWPA_1X;
				enableRS = 1;
			}

			if ( apmib_set( MIB_WLAN_SUPP_NONWPA, (void *)&suppNonWPA) == 0) {
  				strcpy(tmpBuf, ("Set MIB_WLAN_SUPP_NONWPA mib error!"));
				goto setErr_encrypt;
			}
		}

		// WPA authentication
		sprintf(varName, "wpaAuth%d", wlan_id);
		strVal = req_get_cstream_var(wp, varName, "");
		if (strVal[0]) {
			if ( !strcmp(strVal, ("eap"))) {
				apmib_get( MIB_WLAN_MODE, (void *)&intVal);
#ifndef TLS_CLIENT
				if (intVal!=AP_MODE && intVal!=AP_WDS_MODE) { // not AP mode
#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
					if(intVal == CLIENT_MODE){//client mode
//						printf("%s(%d): WPA-RADIUS can be used when device is set to client mode\n",__FUNCTION__,__LINE__);//Added for test 
					}
					else{
						strcpy(tmpBuf, ("Error! WPA-RADIUS cannot be used when device is set to wds or mesh mode."));
						goto setErr_encrypt;
					}
						
#else
					strcpy(tmpBuf, ("Error! WPA-RADIUS cannot be used when device is set to client mode."));
					goto setErr_encrypt;
#endif
				}
#endif
				intVal = WPA_AUTH_AUTO;
				enableRS = 1;
			}
			else if ( !strcmp(strVal, ("psk"))) {
				intVal = WPA_AUTH_PSK;
				getPSK = 1;

			}
			else {
				strcpy(tmpBuf, ("Error! Invalid wpa authentication value."));
				goto setErr_encrypt;
			}
			if ( apmib_set(MIB_WLAN_WPA_AUTH, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set MIB_WLAN_WPA_AUTH failed!"));
				goto setErr_encrypt;
			}
		}

		// cipher suite		
		// sc_yang write the ciphersuite according to  encrypt for wpa
		// wpa mixed mode is not implemented yet.
		
// get cipher suite from user setting, for wpa-aes -------------------		
#if 0				
		intVal = 0 ;
		if( (encrypt ==  ENCRYPT_WPA) || (encrypt == ENCRYPT_WPA2_MIXED) )
			intVal =   WPA_CIPHER_TKIP ;
		if ( apmib_set(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set MIB_WLAN_WPA_CIPHER_SUITE failed!"));
				goto setErr_encrypt;
		}
		//set wpa2UniCipher  for wpa2
		// wpa2 mixed mode is not implemented yet.
		intVal = 0 ;
		if( (encrypt ==  ENCRYPT_WPA2) || (encrypt == ENCRYPT_WPA2_MIXED) )
			intVal =   WPA_CIPHER_AES ;
		if ( apmib_set(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set MIB_WLAN_WPA2_UNICIPHER failed!"));
				goto setErr_encrypt;
		}
#endif	
		//if ((encrypt == ENCRYPT_WPA) || (encrypt == ENCRYPT_WPA2_MIXED)) 
		{
			sprintf(varName, "ciphersuite%d", wlan_id);
			strVal = req_get_cstream_var(wp, varName, "");	 	
			if (strVal[0]) {
				intVal = 0;				
				if ( strstr(strVal, ("tkip"))) 
					intVal |= WPA_CIPHER_TKIP;
				if ( strstr(strVal, ("aes"))) 
					intVal |= WPA_CIPHER_AES;
				if (intVal == 0) {
					strcpy(tmpBuf, ("Invalid value of cipher suite!"));
					goto setErr_encrypt;
				}
			}
			else{
				int band_value=0;
				 apmib_get( MIB_WLAN_BAND, (void *)&band_value);
				 if(band_value == 10 || band_value ==11)
				 	intVal = WPA_CIPHER_AES;	
				 else
					intVal = WPA_CIPHER_TKIP;	
			}

			// check if both TKIP and AES cipher are selected in client mode
			apmib_get(MIB_WLAN_MODE, (void *)&val);
			if (val == CLIENT_MODE) {
				apmib_get(MIB_WLAN_NETWORK_TYPE, &val);
				if (val == INFRASTRUCTURE && intVal == WPA_CIPHER_MIXED) {
					strcpy(tmpBuf, ("Error! Can't set cipher to TKIP + AES when device is set to client mode."));
					goto setErr_encrypt;							
				}
			}	// david+2006-1-11
					
			if ( apmib_set(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set MIB_WLAN_WPA_CIPHER_SUITE failed!"));
				goto setErr_encrypt;							
			}				
		}		
		//if ((encrypt == ENCRYPT_WPA2) || (encrypt == ENCRYPT_WPA2_MIXED)) 
		{
			sprintf(varName, "wpa2ciphersuite%d", wlan_id);
			strVal = req_get_cstream_var(wp, varName, "");	 	
			if (strVal[0]) {
				intVal = 0;				
				if ( strstr(strVal, ("tkip"))) 
					intVal |= WPA_CIPHER_TKIP;
				if ( strstr(strVal, ("aes"))) 
					intVal |= WPA_CIPHER_AES;
				if (intVal == 0) {
					strcpy(tmpBuf, ("Invalid value of wpa2 cipher suite!"));
					goto setErr_encrypt;
				}
			}
			else
				intVal = WPA_CIPHER_AES;			

			// check if both TKIP and AES cipher are selected in client mode
			apmib_get(MIB_WLAN_MODE, (void *)&val);
			if (val == CLIENT_MODE) {
				apmib_get(MIB_WLAN_NETWORK_TYPE, &val);
				if (val == INFRASTRUCTURE && intVal == WPA_CIPHER_MIXED) {
					strcpy(tmpBuf, ("Error! Can't set cipher to TKIP + AES when device is set to client mode."));
					goto setErr_encrypt;							
				}
			}	// david+2006-1-11
				
			if ( apmib_set(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set MIB_WLAN_WPA2_CIPHER_SUITE failed!"));
				goto setErr_encrypt;							
			}
		}
//-------------------------------------------------- david, 2005-8-03	
	
		if( ((encrypt ==  ENCRYPT_WPA2) || (encrypt == ENCRYPT_WPA2_MIXED)) &&
		    enableRS == 1){
			sprintf(varName, "preAuth%d", wlan_id);
			strVal = req_get_cstream_var(wp, varName, "");
			if ( !strcmp(strVal, "ON"))
				intVal = 1 ;
			else
				intVal = 0 ;
			if ( apmib_set(MIB_WLAN_WPA2_PRE_AUTH, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set MIB_WLAN_WPA_CIPHER_SUITE failed!"));
				goto setErr_encrypt;
			}					
		}

		// pre-shared key
		if ( getPSK ) {
			int oldFormat, oldPskLen, i;

			sprintf(varName, "pskFormat%d", wlan_id);
   			strVal = req_get_cstream_var(wp, varName, "");
			if (!strVal[0]) {
	 			strcpy(tmpBuf, ("Error! no psk format."));
				goto setErr_encrypt;
			}
			intVal = strVal[0] - '0';
			if (intVal != 0 && intVal != 1) {
	 			strcpy(tmpBuf, ("Error! invalid psk format."));
				goto setErr_encrypt;
			}

			// remember current psk format and length to compare to default case "****"
			apmib_get(MIB_WLAN_PSK_FORMAT, (void *)&oldFormat);
			apmib_get(MIB_WLAN_WPA_PSK, (void *)tmpBuf);
			oldPskLen = strlen(tmpBuf);

			sprintf(varName, "pskValue%d", wlan_id);
			strVal = req_get_cstream_var(wp, varName, "");
			len = strlen(strVal);

			if (oldFormat == intVal && len == oldPskLen ) {
				for (i=0; i<len; i++) {
					if ( strVal[i] != '*' )
						break;
				}
				if (i == len)
					goto rekey_time;
			}

			if ( apmib_set(MIB_WLAN_PSK_FORMAT, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set MIB_WLAN_PSK_FORMAT failed!"));
				goto setErr_encrypt;
			}

			if (intVal==1) { // hex
				if (len!=MAX_PSK_LEN || !string_to_hex(strVal, tmpBuf, MAX_PSK_LEN)) {
	 				strcpy(tmpBuf, ("Error! invalid psk value."));
					goto setErr_encrypt;
				}
			}
			else { // passphras
				if (len==0 || len > (MAX_PSK_LEN-1) ) {
	 				strcpy(tmpBuf, ("Error! invalid psk value."));
					goto setErr_encrypt;
				}
			}
			if ( !apmib_set(MIB_WLAN_WPA_PSK, (void *)strVal)) {
				strcpy(tmpBuf, ("Set MIB_WLAN_WPA_PSK error!"));
				goto setErr_encrypt;
			}
		}
rekey_time:
		// group key rekey time
		reKeyTime = 0;
		sprintf(varName, "groupKeyTimeDay%d", wlan_id);
		strVal = req_get_cstream_var(wp, varName, "");
		if (strVal[0]) {
			if ( !string_to_dec(strVal, &intVal) ) {
				strcpy(tmpBuf, ("Error! Invalid value of rekey day."));
				goto setErr_encrypt;
			}
			reKeyTime += intVal*86400;
		}
		sprintf(varName, "groupKeyTimeHr%d", wlan_id);
		strVal = req_get_cstream_var(wp, varName, "");
		if (strVal[0]) {
			if ( !string_to_dec(strVal, &intVal) ) {
				strcpy(tmpBuf, ("Error! Invalid value of rekey hr."));
				goto setErr_encrypt;
			}
			reKeyTime += intVal*3600;
		}
		sprintf(varName, "groupKeyTimeMin%d", wlan_id);
		strVal = req_get_cstream_var(wp, varName, "");
		if (strVal[0]) {
			if ( !string_to_dec(strVal, &intVal) ) {
				strcpy(tmpBuf, ("Error! Invalid value of rekey min."));
				goto setErr_encrypt;
			}
			reKeyTime += intVal*60;
		}

		sprintf(varName, "groupKeyTimeSec%d", wlan_id);
		strVal = req_get_cstream_var(wp, varName, "");
		if (strVal[0]) {
			if ( !string_to_dec(strVal, &intVal) ) {
				strcpy(tmpBuf, ("Error! Invalid value of rekey sec."));
				goto setErr_encrypt;
			}
			reKeyTime += intVal;
		}
		if (reKeyTime) {
			if ( !apmib_set(MIB_WLAN_WPA_GROUP_REKEY_TIME, (void *)&reKeyTime)) {
				strcpy(tmpBuf, ("Set MIB_WLAN_WPA_GROUP_REKEY_TIME error!"));
				goto setErr_encrypt;
			}
		}
	}

	apmib_set( MIB_WLAN_ENABLE_1X, (void *)&enableRS);			
	if (enableRS == 1) { // if 1x enabled, get RADIUS server info
#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
		apmib_get( MIB_WLAN_MODE, (void *)&wlan_mode);
		if (wlan_mode == CLIENT_MODE) { // wlan client mode
			wlanIdx_5G=whichWlanIfIs(PHYBAND_5G);
			wlanIdx_2G=whichWlanIfIs(PHYBAND_2G);
			if(wlan_idx==wlanIdx_5G){
				rsBandSel=PHYBAND_5G;
				if ( !apmib_set(MIB_WLAN_RS_BAND_SEL, (void *)&rsBandSel)) {
					strcpy(tmpBuf, ("Set MIB_WLAN_RS_BAND_SEL error!"));
					goto setErr_encrypt;
				}
			}
			else if(wlan_idx==wlanIdx_2G){
				rsBandSel=PHYBAND_2G;
				if ( !apmib_set(MIB_WLAN_RS_BAND_SEL, (void *)&rsBandSel)) {
					strcpy(tmpBuf, ("Set MIB_WLAN_RS_BAND_SEL error!"));
					goto setErr_encrypt;
				}
			}
			else{
				rsBandSel=PHYBAND_OFF;
				if ( !apmib_set(MIB_WLAN_RS_BAND_SEL, (void *)&rsBandSel)) {
					strcpy(tmpBuf, ("Set MIB_WLAN_RS_BAND_SEL error!"));
					goto setErr_encrypt;
				}
			}
			
			sprintf(varName, "eapType%d", wlan_id);
			strVal = req_get_cstream_var(wp, varName, "");
			if (strVal[0]) {
				if ( !string_to_dec(strVal, &intVal) ) {
					strcpy(tmpBuf, ("Invalid 802.1x EAP type value!"));
					goto setErr_encrypt;
				}
				if ( !apmib_set(MIB_WLAN_EAP_TYPE, (void *)&intVal)) {
					strcpy(tmpBuf, ("Set MIB_WLAN_EAP_TYPE error!"));
					goto setErr_encrypt;
				}
			}
			else{
				strcpy(tmpBuf, ("No 802.1x EAP type!"));
				goto setErr_encrypt;
			}

			if(intVal == EAP_MD5){
				sprintf(varName, "eapUserId%d", wlan_id);
				strVal = req_get_cstream_var(wp, varName, "");
				if (strVal[0]) {
					if(strlen(strVal)>MAX_EAP_USER_ID_LEN){
						strcpy(tmpBuf, ("EAP user ID too long!"));
						goto setErr_encrypt;
					}
					if ( !apmib_set(MIB_WLAN_EAP_USER_ID, (void *)strVal)) {
						strcpy(tmpBuf, ("Set MIB_WLAN_EAP_USER_ID error!"));
						goto setErr_encrypt;
					}
				}
				else{
					strcpy(tmpBuf, ("No 802.1x EAP User ID!"));
					goto setErr_encrypt;
				}
				
				sprintf(varName, "radiusUserName%d", wlan_id);
				strVal = req_get_cstream_var(wp, varName, "");
				if (strVal[0]) {
					if(strlen(strVal)>MAX_RS_USER_NAME_LEN){
						strcpy(tmpBuf, ("RADIUS user name too long!"));
						goto setErr_encrypt;
					}
					if ( !apmib_set(MIB_WLAN_RS_USER_NAME, (void *)strVal)) {
						strcpy(tmpBuf, ("Set MIB_WLAN_RS_USER_NAME error!"));
						goto setErr_encrypt;
					}
				}
				else{
					strcpy(tmpBuf, ("No 802.1x RADIUS User Name!"));
					goto setErr_encrypt;
				}

				sprintf(varName, "radiusUserPass%d", wlan_id);
				strVal = req_get_cstream_var(wp, varName, "");
				if (strVal[0]) {
					if(strlen(strVal)>MAX_RS_USER_PASS_LEN){
						strcpy(tmpBuf, ("RADIUS user password too long!"));
						goto setErr_encrypt;
					}
					if ( !apmib_set(MIB_WLAN_RS_USER_PASSWD, (void *)strVal)) {
						strcpy(tmpBuf, ("Set MIB_WLAN_RS_USER_PASSWD error!"));
						goto setErr_encrypt;
					}
				}
				else{
					strcpy(tmpBuf, ("No 802.1x RADIUS User Password!"));
					goto setErr_encrypt;
				}
			}
			else if(intVal == EAP_TLS){
				sprintf(varName, "eapUserId%d", wlan_id);
				strVal = req_get_cstream_var(wp, varName, "");
				if (strVal[0]) {
					if(strlen(strVal)>MAX_EAP_USER_ID_LEN){
						strcpy(tmpBuf, ("EAP user ID too long!"));
						goto setErr_encrypt;
					}
					if ( !apmib_set(MIB_WLAN_EAP_USER_ID, (void *)strVal)) {
						strcpy(tmpBuf, ("Set MIB_WLAN_EAP_USER_ID error!"));
						goto setErr_encrypt;
					}
				}
				else{
					strcpy(tmpBuf, ("No 802.1x EAP User ID!"));
					goto setErr_encrypt;
				}
				
				sprintf(varName, "radiusUserCertPass%d", wlan_id);
				strVal = req_get_cstream_var(wp, varName, "");
				if (strVal[0]) {
					if(strlen(strVal)>MAX_RS_USER_CERT_PASS_LEN){
						strcpy(tmpBuf, ("RADIUS user cert password too long!"));
						goto setErr_encrypt;
					}
					if ( !apmib_set(MIB_WLAN_RS_USER_CERT_PASSWD, (void *)strVal)) {
						strcpy(tmpBuf, ("Set MIB_WLAN_RS_USER_CERT_PASSWD error!"));
						goto setErr_encrypt;
					}
				}
				else{
					if ( !apmib_set(MIB_WLAN_RS_USER_CERT_PASSWD, (void *)strVal)) {
						strcpy(tmpBuf, ("Clear MIB_WLAN_RS_USER_CERT_PASSWD error!"));
						goto setErr_encrypt;
					}
					//strcpy(tmpBuf, ("No 802.1x RADIUS user cert password!"));
					//goto setErr_encrypt;
				}

				if(rsBandSel == PHYBAND_5G){
					if(isFileExist(RS_USER_CERT_5G) != 1){
						strcpy(tmpBuf, ("No 802.1x RADIUS 5g user cert!\nPlease upload it."));
						goto setErr_encrypt;
					}
					
					if(isFileExist(RS_ROOT_CERT_5G) != 1){
						strcpy(tmpBuf, ("No 802.1x RADIUS 5g root cert!\nPlease upload it."));
						goto setErr_encrypt;
					}
				}
				else if(rsBandSel == PHYBAND_2G){
					if(isFileExist(RS_USER_CERT_2G) != 1){
						strcpy(tmpBuf, ("No 802.1x RADIUS 2g user cert!\nPlease upload it."));
						goto setErr_encrypt;
					}
					
					if(isFileExist(RS_ROOT_CERT_2G) != 1){
						strcpy(tmpBuf, ("No 802.1x RADIUS 2g root cert!\nPlease upload it."));
						goto setErr_encrypt;
					}
				}
			}
			else if(intVal == EAP_PEAP){
				sprintf(varName, "eapInsideType%d", wlan_id);
				strVal = req_get_cstream_var(wp, varName, "");
				if (strVal[0]) {
					if ( !string_to_dec(strVal, &intVal2) ) {
						strcpy(tmpBuf, ("Invalid 802.1x inside tunnel type value!"));
						goto setErr_encrypt;
					}
					if ( !apmib_set(MIB_WLAN_EAP_INSIDE_TYPE, (void *)&intVal2)) {
						strcpy(tmpBuf, ("Set MIB_WLAN_EAP_INSIDE_TYPE error!"));
						goto setErr_encrypt;
					}
				}
				else{
					strcpy(tmpBuf, ("No 802.1x inside tunnel type!"));
					goto setErr_encrypt;
				}

				if(intVal2 == INSIDE_MSCHAPV2){
					sprintf(varName, "eapUserId%d", wlan_id);
					strVal = req_get_cstream_var(wp, varName, "");
					if (strVal[0]) {
						if(strlen(strVal)>MAX_EAP_USER_ID_LEN){
							strcpy(tmpBuf, ("EAP user ID too long!"));
							goto setErr_encrypt;
						}
						if ( !apmib_set(MIB_WLAN_EAP_USER_ID, (void *)strVal)) {
							strcpy(tmpBuf, ("Set MIB_WLAN_EAP_USER_ID error!"));
							goto setErr_encrypt;
						}
					}
					else{
						strcpy(tmpBuf, ("No 802.1x EAP User ID!"));
						goto setErr_encrypt;
					}
					
					sprintf(varName, "radiusUserName%d", wlan_id);
					strVal = req_get_cstream_var(wp, varName, "");
					if (strVal[0]) {
						if(strlen(strVal)>MAX_RS_USER_NAME_LEN){
							strcpy(tmpBuf, ("RADIUS user name too long!"));
							goto setErr_encrypt;
						}
						if ( !apmib_set(MIB_WLAN_RS_USER_NAME, (void *)strVal)) {
							strcpy(tmpBuf, ("Set MIB_WLAN_RS_USER_NAME error!"));
							goto setErr_encrypt;
						}
					}
					else{
						strcpy(tmpBuf, ("No 802.1x RADIUS User Name!"));
						goto setErr_encrypt;
					}

					sprintf(varName, "radiusUserPass%d", wlan_id);
					strVal = req_get_cstream_var(wp, varName, "");
					if (strVal[0]) {
						if(strlen(strVal)>MAX_RS_USER_PASS_LEN){
							strcpy(tmpBuf, ("RADIUS user password too long!"));
							goto setErr_encrypt;
						}
						if ( !apmib_set(MIB_WLAN_RS_USER_PASSWD, (void *)strVal)) {
							strcpy(tmpBuf, ("Set MIB_WLAN_RS_USER_PASSWD error!"));
							goto setErr_encrypt;
						}
					}
					else{
						strcpy(tmpBuf, ("No 802.1x RADIUS User Password!"));
						goto setErr_encrypt;
					}

//					if(isFileExist(RS_USER_CERT) == 1){
						sprintf(varName, "radiusUserCertPass%d", wlan_id);
						strVal = req_get_cstream_var(wp, varName, "");
						if (strVal[0]) {
							if(strlen(strVal)>MAX_RS_USER_CERT_PASS_LEN){
								strcpy(tmpBuf, ("RADIUS user cert password too long!"));
								goto setErr_encrypt;
							}
							if ( !apmib_set(MIB_WLAN_RS_USER_CERT_PASSWD, (void *)strVal)) {
								strcpy(tmpBuf, ("Set MIB_WLAN_RS_USER_CERT_PASSWD error!"));
								goto setErr_encrypt;
							}
						}
						else{
							if ( !apmib_set(MIB_WLAN_RS_USER_CERT_PASSWD, (void *)strVal)) {
								strcpy(tmpBuf, ("[1] Clear MIB_WLAN_RS_USER_CERT_PASSWD error!"));
								goto setErr_encrypt;
							}
							//strcpy(tmpBuf, ("No 802.1x RADIUS user cert password!"));
							//goto setErr_encrypt;
						}
//					}
				}
				else{
					strcpy(tmpBuf, ("802.1x inside tunnel type not support!"));
					goto setErr_encrypt;
				}
			}
			else{
				strcpy(tmpBuf, ("802.1x EAP type not support!"));
				goto setErr_encrypt;
			}
		}
		else
#endif
		{
		sprintf(varName, "radiusPort%d", wlan_id);
		strVal = req_get_cstream_var(wp, varName, "");
		if (!strVal[0]) {
			strcpy(tmpBuf, ("No RS port number!"));
			goto setErr_encrypt;
		}
		if (!string_to_dec(strVal, &intVal) || intVal<=0 || intVal>65535) {
			strcpy(tmpBuf, ("Error! Invalid value of RS port number."));
			goto setErr_encrypt;
		}
		if ( !apmib_set(MIB_WLAN_RS_PORT, (void *)&intVal)) {
			strcpy(tmpBuf, ("Set RS port error!"));
			goto setErr_encrypt;
		}
		sprintf(varName, "radiusIP%d", wlan_id);
		strVal = req_get_cstream_var(wp, varName, "");
		if (!strVal[0]) {
			strcpy(tmpBuf, ("No RS IP address!"));
			goto setErr_encrypt;
		}
		if ( !inet_aton(strVal, &inIp) ) {
			strcpy(tmpBuf, ("Invalid RS IP-address value!"));
			goto setErr_encrypt;
		}
		if ( !apmib_set(MIB_WLAN_RS_IP, (void *)&inIp)) {
			strcpy(tmpBuf, ("Set RS IP-address error!"));
			goto setErr_encrypt;
		}
		sprintf(varName, "radiusPass%d", wlan_id);
		strVal = req_get_cstream_var(wp, varName, "");
		if (strlen(strVal) > (MAX_RS_PASS_LEN -1) ) {
			strcpy(tmpBuf, ("RS password length too long!"));
			goto setErr_encrypt;
		}
		if ( !apmib_set(MIB_WLAN_RS_PASSWORD, (void *)strVal)) {
			strcpy(tmpBuf, ("Set RS password error!"));
			goto setErr_encrypt;
		}

		sprintf(varName, "radiusRetry%d", wlan_id);
		strVal = req_get_cstream_var(wp, varName, "");
		if (strVal[0]) {
			if ( !string_to_dec(strVal, &intVal) ) {
				strcpy(tmpBuf, ("Invalid RS retry value!"));
				goto setErr_encrypt;
			}
			if ( !apmib_set(MIB_WLAN_RS_MAXRETRY, (void *)&intVal)) {
				strcpy(tmpBuf, ("Set MIB_WLAN_RS_MAXRETRY error!"));
				goto setErr_encrypt;
			}
		}
		sprintf(varName, "radiusTime%d", wlan_id);
		strVal = req_get_cstream_var(wp, varName, "");
		if (strVal[0]) {
			if ( !string_to_dec(strVal, &intVal) ) {
				strcpy(tmpBuf, ("Invalid RS time value!"));
				goto setErr_encrypt;
			}
			if ( !apmib_set(MIB_WLAN_RS_INTERVAL_TIME, (void *)&intVal)) {
				strcpy(tmpBuf, ("Set MIB_WLAN_RS_INTERVAL_TIME error!"));
				goto setErr_encrypt;
			}
		}
		sprintf(varName, "useAccount%d", wlan_id);
		strVal = req_get_cstream_var(wp, varName, "");
		if ( !strcmp(strVal, "ON"))
			intVal = 1;
		else
			intVal = 0;
		if ( apmib_set( MIB_WLAN_ACCOUNT_RS_ENABLED, (void *)&intVal) == 0) {
  			strcpy(tmpBuf, ("Set MIB_WLAN_ACCOUNT_RS_ENABLED mib error!"));
			goto setErr_encrypt;
		}
		if (intVal == 0)
			goto get_wepkey;

		sprintf(varName, "accountPort%d", wlan_id);
		strVal = req_get_cstream_var(wp, varName, "");
		if (!strVal[0]) {
			strcpy(tmpBuf, ("No account RS port number!"));
			goto setErr_encrypt;
		}
		if (!string_to_dec(strVal, &intVal) || intVal<=0 || intVal>65535) {
			strcpy(tmpBuf, ("Error! Invalid value of account RS port number."));
			goto setErr_encrypt;
		}
		if ( !apmib_set(MIB_WLAN_ACCOUNT_RS_PORT, (void *)&intVal)) {
			strcpy(tmpBuf, ("Set account RS port error!"));
			goto setErr_encrypt;
		}
		sprintf(varName, "accountIP%d", wlan_id);
		strVal = req_get_cstream_var(wp, varName, "");
		if (!strVal[0]) {
			strcpy(tmpBuf, ("No account RS IP address!"));
			goto setErr_encrypt;
		}
		if ( !inet_aton(strVal, &inIp) ) {
			strcpy(tmpBuf, ("Invalid account RS IP-address value!"));
			goto setErr_encrypt;
		}
		if ( !apmib_set(MIB_WLAN_ACCOUNT_RS_IP, (void *)&inIp)) {
			strcpy(tmpBuf, ("Set account RS IP-address error!"));
			goto setErr_encrypt;
		}
		sprintf(varName, "accountPass%d", wlan_id);
		strVal = req_get_cstream_var(wp, varName, "");
		if (strlen(strVal) > (MAX_RS_PASS_LEN -1) ) {
			strcpy(tmpBuf, ("Account RS password length too long!"));
			goto setErr_encrypt;
		}
		if ( !apmib_set(MIB_WLAN_ACCOUNT_RS_PASSWORD, (void *)strVal)) {
			strcpy(tmpBuf, ("Set account RS password error!"));
			goto setErr_encrypt;
		}
		sprintf(varName, "accountRetry%d", wlan_id);
		strVal = req_get_cstream_var(wp, varName, "");
		if (strVal[0]) {
			if ( !string_to_dec(strVal, &intVal) ) {
				strcpy(tmpBuf, ("Invalid account RS retry value!"));
				goto setErr_encrypt;
			}
			if ( !apmib_set(MIB_WLAN_ACCOUNT_RS_MAXRETRY, (void *)&intVal)) {
				strcpy(tmpBuf, ("Set MIB_WLAN_ACCOUNT_RS_MAXRETRY error!"));
				goto setErr_encrypt;
			}
		}
		sprintf(varName, "accountTime%d", wlan_id);
		strVal = req_get_cstream_var(wp, varName, "");
		if (strVal[0]) {
			if ( !string_to_dec(strVal, &intVal) ) {
				strcpy(tmpBuf, ("Invalid account RS time value!"));
				goto setErr_encrypt;
			}
			if ( !apmib_set(MIB_WLAN_ACCOUNT_RS_INTERVAL_TIME, (void *)&intVal)) {
				strcpy(tmpBuf, ("Set MIB_WLAN_ACCOUNT_RS_INTERVAL_TIME error!"));
				goto setErr_encrypt;
			}
		}
		sprintf(varName, "accountUpdateEnabled%d", wlan_id);
		strVal = req_get_cstream_var(wp, varName, "");
		if ( !strcmp(strVal, "ON"))
			intVal = 1;
		else
			intVal = 0;
		if ( apmib_set( MIB_WLAN_ACCOUNT_RS_UPDATE_ENABLED, (void *)&intVal) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_ACCOUNT_RS_UPDATE_ENABLED mib error!"));
			goto setErr_encrypt;
		}
		sprintf(varName, "accountUpdateTime%d", wlan_id);
		strVal = req_get_cstream_var(wp, varName, "");
		if (strVal[0]) {
			if ( !string_to_dec(strVal, &intVal) ) {
				strcpy(tmpBuf, ("Error! Invalid value of update time"));
				goto setErr_encrypt;
			}
			if ( !apmib_set(MIB_WLAN_ACCOUNT_RS_UPDATE_DELAY, (void *)&intVal)) {
				strcpy(tmpBuf, ("Set MIB_WLAN_ACCOUNT_RS_UPDATE_DELAY mib error!"));
				goto setErr_encrypt;
			}
		}

get_wepkey:
		// get 802.1x WEP key length
		sprintf(varName, "wepKeyLen%d", wlan_id);
		strVal = req_get_cstream_var(wp, varName, "");
		if (strVal[0]) {
			if ( !strcmp(strVal, ("wep64")))
				intVal = WEP64;
			else if ( !strcmp(strVal, ("wep128")))
				intVal = WEP128;
			else {
				strcpy(tmpBuf, ("Error! Invalid wepkeylen value."));
				goto setErr_encrypt;
			}
			if ( apmib_set(MIB_WLAN_WEP, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set MIB_WLAN_WEP failed!"));
				goto setErr_encrypt;
			}
		}
	}
	}

#ifdef WIFI_SIMPLE_CONFIG
#ifdef MBSSID
	if (vwlan_idx == 0)
#endif
	{
		sprintf(varName, "wps_clear_configure_by_reg%d", wlan_id);
		strVal = req_get_cstream_var(wp, varName, NULL);
		val = 0;
		if (strVal[0])
			val = atoi(strVal);
		update_wps_configured(val);
	}
#endif

#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)
	if (vwlan_idx == NUM_VWLAN_INTERFACE)
	{
		sprintf(varName, "wps_clear_configure_by_reg%d", wlan_id);
		strVal = req_get_cstream_var(wp, varName, NULL);
		val = 0;
		if (strVal[0])
			val = atoi(strVal);
		update_wps_configured(val);
	}
#endif

	return 0 ;
setErr_encrypt:
	return -1 ;		
}	


#if defined(WLAN_PROFILE)
int wlanProfileEncryptHandler(request *wp, char *tmpBuf)
{
	char varName[20];
	char *strEncrypt, *strVal;
	int ssid_idx;
	int profile_num_id,	profile_tbl_id, profile_mod_id;
	WLAN_PROFILE_T entry;
	WLAN_PROFILE_T target[2];
	ENCRYPT_T encrypt;

//displayPostDate(wp->post_data);
	memset(target, 0x00, sizeof(WLAN_PROFILE_T)*2);
	strVal = req_get_cstream_var(wp, "SSID_Setting", "");
		
	if (strVal[0])
		ssid_idx = atoi(strVal);

#if defined(UNIVERSAL_REPEATER) 
	if(ssid_idx < (1+NUM_VWLAN+1) || ssid_idx>=(1+NUM_VWLAN+1+MAX_WLAN_PROFILE_NUM))
#else
	if(ssid_idx < (1+NUM_VWLAN) || ssid_idx>=(1+NUM_VWLAN+MAX_WLAN_PROFILE_NUM))
#endif		
	{
		strcpy(tmpBuf, ("ssid_idx is invalid!!"));
		goto setErr_wlan;
	}

	
	if(wlan_idx == 0)
	{
		profile_num_id = MIB_PROFILE_NUM1;
		profile_tbl_id = MIB_PROFILE_TBL1;
		profile_mod_id = MIB_PROFILE_MOD1;
	}
	else
	{
		profile_num_id = MIB_PROFILE_NUM2;
		profile_tbl_id = MIB_PROFILE_TBL2;
		profile_mod_id = MIB_PROFILE_MOD2;
	}

#if defined(UNIVERSAL_REPEATER) 
	ssid_idx -= (1+NUM_VWLAN+1);
#else
	ssid_idx -= (1+NUM_VWLAN);
#endif
	ssid_idx++;

//printf("\r\n ssid_idx=[%d],__[%s-%u]\r\n",ssid_idx,__FILE__,__LINE__);

	*((char *)&entry) = (char)ssid_idx;
	if ( !apmib_get(profile_tbl_id, (void *)&entry)) {
		strcpy(tmpBuf, ("Get table entry error!"));
		goto setErr_wlan;
	}

	memcpy(&target[0], &entry, sizeof(WLAN_PROFILE_T));

	sprintf(varName, "method%d", wlan_idx);
	strEncrypt = req_get_cstream_var(wp, varName, "");
	encrypt = (ENCRYPT_T) strEncrypt[0] - '0';

//printf("\r\n encrypt[%d],__[%s-%u]\r\n",encrypt,__FILE__,__LINE__);

	if (encrypt==ENCRYPT_WEP) 
	{
		char *strWep, *strAuth, *strFormat, *wepKey;
		int keyLen;
		char key[30];
		
		sprintf(varName, "length%d", wlan_idx);
		strWep = req_get_cstream_var(wp, varName, "");
		if(strWep[0])
		{
			entry.encryption = atoi(strWep);
		}

		sprintf(varName, "format%d", wlan_idx);
		strFormat = req_get_cstream_var(wp, varName, "");
		if(strFormat[0])
		{
			entry.wepKeyType = atoi(strFormat)-1;
		}
		
		strAuth = req_get_cstream_var(wp, ("authType"), "");
		if (strAuth[0]) { // new UI
			if (!strcmp(strAuth, ("open")))
				entry.auth = AUTH_OPEN;
			else if ( !strcmp(strAuth, ("shared")))
				entry.auth = AUTH_SHARED;
			else 
				entry.auth = AUTH_BOTH;			
		}

		sprintf(varName, "key%d", wlan_idx);
		wepKey = req_get_cstream_var(wp, varName, "");
		if  (wepKey[0]) {

			if ( !isAllStar(wepKey) ) {

				if (entry.encryption == WEP64) {
					if (entry.wepKeyType==0)
						keyLen = WEP64_KEY_LEN;
					else
						keyLen = WEP64_KEY_LEN*2;
				}
				else 
				{
					if (entry.wepKeyType==0)
						keyLen = WEP128_KEY_LEN;
					else
						keyLen = WEP128_KEY_LEN*2;
				}
		
				if (entry.wepKeyType == 0) // ascii
					strcpy(key, wepKey);
				else // hex
				{ 
					if ( !string_to_hex(wepKey, key, keyLen)) {
		   				strcpy(tmpBuf, ("Invalid wep-key value!"));
						goto setErr_wlan;
					}
				}
				if (entry.encryption == WEP64){
					strncpy(entry.wepKey1, key, 10);
					strncpy(entry.wepKey2, key, 10);
					strncpy(entry.wepKey3, key, 10);
					strncpy(entry.wepKey4, key, 10);
				}else{
					strncpy(entry.wepKey1, key, 26);
					strncpy(entry.wepKey2, key, 26);
					strncpy(entry.wepKey3, key, 26);
					strncpy(entry.wepKey4, key, 26);
				}			
			}
		}
		
	}
	else if (encrypt > ENCRYPT_WEP) 
	{
		char *strWpaAuth, *strCipherSuite, *strPskFormat, *strPskValue;
		int cipherSuite, pskFormat;
		int getPSK;
		// WPA authentication

		if(encrypt == ENCRYPT_WPA)
			entry.encryption = 3; //wpa
		else
			entry.encryption = 4; //wpa2
		
		sprintf(varName, "wpaAuth%d", wlan_idx);
		strWpaAuth = req_get_cstream_var(wp, varName, "");
		if (strWpaAuth[0]) 
		{
			if ( !strcmp(strWpaAuth, ("eap"))) {
				strcpy(tmpBuf, ("Invalid wpaAuth value!"));
				goto setErr_wlan;
			}
			else if ( !strcmp(strWpaAuth, ("psk"))) {
				getPSK = 1;
			}
			else {
				strcpy(tmpBuf, ("Error! Invalid wpa authentication value."));
				goto setErr_wlan;
			}
		}

		sprintf(varName, "ciphersuite%d", wlan_idx);
		strCipherSuite = req_get_cstream_var(wp, varName, "");	 	
		if (strCipherSuite[0]) {
			cipherSuite = 0;				
			if ( strstr(strCipherSuite, ("tkip"))) 
				cipherSuite |= WPA_CIPHER_TKIP;
			if ( strstr(strCipherSuite, ("aes"))) 
				cipherSuite |= WPA_CIPHER_AES;
			if (cipherSuite == 0 || cipherSuite == WPA_CIPHER_MIXED) //check if both TKIP and AES cipher are selected in client mode
			{
				strcpy(tmpBuf, ("Invalid value of cipher suite!"));
				goto setErr_wlan;
			}

			if(cipherSuite == WPA_CIPHER_TKIP)
				entry.wpa_cipher = 2;
			else
				entry.wpa_cipher = 8;
			
		}		
		{
			sprintf(varName, "wpa2ciphersuite%d", wlan_idx);
			strCipherSuite = req_get_cstream_var(wp, varName, "");	 	
			if (strCipherSuite[0]) {
				cipherSuite = 0;				
				if ( strstr(strCipherSuite, ("tkip"))) 
					cipherSuite |= WPA_CIPHER_TKIP;
				if ( strstr(strCipherSuite, ("aes"))) 
					cipherSuite |= WPA_CIPHER_AES;
				if (cipherSuite == 0 || cipherSuite == WPA_CIPHER_MIXED) //check if both TKIP and AES cipher are selected in client mode
				{
					strcpy(tmpBuf, ("Invalid value of cipher suite!"));
					goto setErr_wlan;
				}

				if(cipherSuite == WPA_CIPHER_TKIP)
					entry.wpa_cipher = 2;
				else
					entry.wpa_cipher = 8;
				
			}
		}

		// pre-shared key
		if ( getPSK ) {
			int oldFormat, oldPskLen, i;

			sprintf(varName, "pskFormat%d", wlan_idx);
   			strPskFormat = req_get_cstream_var(wp, varName, "");
			if (!strPskFormat[0]) {
	 			strcpy(tmpBuf, ("Error! no psk format."));
				goto setErr_wlan;
			}
			pskFormat = strPskFormat[0] - '0';
			if (pskFormat != 0 && pskFormat != 1) {
	 			strcpy(tmpBuf, ("Error! invalid psk format."));
				goto setErr_wlan;
			}

			// remember current psk format and length to compare to default case "****"
			sprintf(varName, "pskValue%d", wlan_idx);
			strPskValue = req_get_cstream_var(wp, varName, "");

			entry.wpaPSKFormat= pskFormat;

			if (pskFormat==1) { // hex
				if (strlen(strPskValue)!=MAX_PSK_LEN || !string_to_hex(strPskValue, tmpBuf, MAX_PSK_LEN)) {
	 				strcpy(tmpBuf, ("Error! invalid psk value."));
					goto setErr_wlan;
				}
			}
			else { // passphras
				if (strlen(strPskValue)==0 || strlen(strPskValue) > (MAX_PSK_LEN-1) ) {
	 				strcpy(tmpBuf, ("Error! invalid psk value."));
					goto setErr_wlan;
				}
			}
			strcpy(entry.wpaPSK, strPskValue);
		
		}

	}
	else
	{
		char oldSsid[32];

		strcpy(oldSsid, entry.ssid);
		memset(&entry, 0x00, sizeof(WLAN_PROFILE_T));
		strcpy(entry.ssid, oldSsid);
		entry.wpa_cipher = 8; //WPA_CIPHER_AES
		
	}

	memcpy(&target[1], &entry, sizeof(WLAN_PROFILE_T));

	if ( !apmib_set(profile_mod_id, (void *)&target)) {
		strcpy(tmpBuf, ("Modify table entry error!"));
		goto setErr_wlan;
	}

	return 0 ;
setErr_wlan:
	
	return -1 ;	
}
#endif //#if defined(WLAN_PROFILE)
/////////////////////////////////////////////////////////////////////////////
void formWlEncrypt(request *wp, char *path, char *query)
{
	char *submitUrl;
	char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};
	
//displayPostDate(wp->post_data);	
#ifdef MBSSID	
	char *strEncrypt, *strVal, *strVal1;
	char varName[40];
	int ssid_idx, ssid_idx2, old_idx=-1;

   	strVal1 = req_get_cstream_var(wp, "wlan_ssid_id", "");
   	strVal = req_get_cstream_var(wp, "SSID_Setting", "");
		
	if (strVal[0]) {
		ssid_idx2 = ssid_idx = atoi(strVal);

#if defined(CONFIG_RTL_ULINKER)
	if(ssid_idx == 5) //vxd
		ssid_idx = NUM_VWLAN_INTERFACE;
#endif
		
	mssid_idx = atoi(strVal1); // selected index from UI
	old_idx = vwlan_idx;
	vwlan_idx = ssid_idx;
		
#if defined(WLAN_PROFILE)
//printf("\r\n ssid_idx=[%d],__[%s-%u]\r\n",ssid_idx,__FILE__,__LINE__);

#if defined(UNIVERSAL_REPEATER) 
	if (ssid_idx2 >= (1+NUM_VWLAN+1))
#else
	if (ssid_idx2 >= (1+NUM_VWLAN))
#endif
	{
		if(wlanProfileEncryptHandler(wp, tmpBuf) == 0)
		{
			goto setOK;

		}
		else
		{
			goto setErr_end ;
		}
	}
	else
#endif //#if defined(WLAN_PROFILE)		
	{
		if (ssid_idx > NUM_VWLAN_INTERFACE) {			
			printf("Invald ssid_id!\n");
			return;
		}			
	}


		
	
		sprintf(varName, "method%d", wlan_idx);
	   	strEncrypt = req_get_cstream_var(wp, varName, "");
		ENCRYPT_T encrypt = (ENCRYPT_T) strEncrypt[0] - '0';

		if (encrypt==ENCRYPT_WEP) {
			char *strAuth = req_get_cstream_var(wp, ("authType"), "");
			AUTH_TYPE_T authType;
			if (strAuth[0]) { // new UI
				if (!strcmp(strAuth, ("open")))
					authType = AUTH_OPEN;
				else if ( !strcmp(strAuth, ("shared")))
					authType = AUTH_SHARED;
				else 
					authType = AUTH_BOTH;
				apmib_set(MIB_WLAN_AUTH_TYPE, (void *)&authType);

				sprintf(varName, "use1x%d", wlan_idx);
				strVal = req_get_cstream_var(wp, varName, "");		
			
				if (strVal[0] && strcmp(strVal, "ON")) {
					int intVal = 0;
					apmib_set( MIB_WLAN_ENABLE_1X, (void *)&intVal);			
					formWep(wp, path, query);
					vwlan_idx = old_idx;					
					return;	
				}
			}
		}
	}
	else
			mssid_idx = 0;
#endif // MBSSID
	
	if(wpaHandler(wp, tmpBuf, wlan_idx) < 0) {
#ifdef MBSSID
		if (old_idx >= 0)
			vwlan_idx = old_idx;	
#endif		
		goto setErr_end ;
	}

setOK:
	
	apmib_update_web(CURRENT_SETTING);

#ifndef NO_ACTION
	run_init_script("bridge");
#endif

	submitUrl = req_get_cstream_var(wp, ("submit-url"), "");   // hidden page
	OK_MSG(submitUrl);

#ifdef MBSSID
	if (old_idx >= 0)
		vwlan_idx = old_idx;	
#endif

	return;

setErr_end:
	ERR_MSG(tmpBuf);
}

#ifdef CONFIG_RTK_MESH

/////////////////////////////////////////////////////////////////////////////
int wlMeshNeighborTable(request *wp, int argc, char **argv)
{
        int nBytesSent=0;
        int nRecordCount=0;
        FILE *fh;
        char buf[512], network[100];
        char hwaddr[100],state[100],channel[100],link_rate[100],tx_pkts[10],rx_pkts[10];
        char rssi[100],establish_exp_time[100],bootseq_exp_time[100],dummy[100];

        nBytesSent += req_format_write(wp, ("<tr bgcolor=\"#7F7F7F\">"
        "<td align=center width=\"17%%\"><font size=\"2\"><b>MAC Address</b></font></td>\n"
        //"<td align=center width=\"17%%\"><font size=\"2\"><b>State</b></font></td>\n"
        "<td align=center width=\"17%%\"><font size=\"2\"><b>Mode</b></font></td>\n"
        //"<td align=center width=\"17%%\"><font size=\"2\"><b>Channel</b></font></td>\n"
        "<td align=center width=\"17%%\"><font size=\"2\"><b>Tx Packets</b></font></td>\n"
	"<td align=center width=\"17%%\"><font size=\"2\"><b>Rx Packets</b></font></td>\n"
        "<td align=center width=\"17%%\"><font size=\"2\"><b>Tx Rate (Mbps)</b></font></td>\n"
        "<td align=center width=\"17%%\"><font size=\"2\"><b>RSSI</b></font></td>\n"
	"<td align=center width=\"17%%\"><font size=\"2\"><b>Expired Time (s)</b></font></td>\n"
#if defined(_11s_TEST_MODE_)
        "<td align=center width=\"17%%\"><font size=\"2\"><b>BootSeq_ept</b></font></td></tr>\n"
#endif
	));

        fh = fopen(_FILE_MESH_ASSOC, "r");
        if (!fh)
        {
		printf("Warning: cannot open %s\n",_FILE_MESH_ASSOC);
                return -1;
        }

        while( fgets(buf, sizeof buf, fh) != NULL )
        {
                if( strstr(buf,"Mesh MP_info") != NULL )
                {
                        _get_token( fh,"state: ",state );
                        _get_token( fh,"hwaddr: ",hwaddr );
			_get_token( fh,"mode: ",network );
			_get_token( fh,"Tx Packets: ",tx_pkts );
			_get_token( fh,"Rx Packets: ",rx_pkts );
                        _get_token( fh,"Authentication: ",dummy );
                        _get_token( fh,"Assocation: ",dummy );
                        _get_token( fh,"LocalLinkID: ",dummy );
                        _get_token( fh,"PeerLinkID: ",dummy );
                        _get_token( fh,"operating_CH: ", channel );
                        _get_token( fh,"CH_precedence: ", dummy );
                        _get_token( fh,"R: ", link_rate );
                        _get_token( fh,"Ept: ", dummy );
                        _get_token( fh,"rssi: ", rssi );
                        _get_token( fh,"expire_Establish(jiffies): ", dummy );
                        //_get_token( fh,"(mSec): ", establish_exp_time );
			_get_token( fh,"(Sec): ", establish_exp_time );
                        _get_token( fh,"expire_BootSeq & LLSA(jiffies): ", dummy );
                        _get_token( fh,"(mSec): ", bootseq_exp_time );
                        _get_token( fh,"(mSec): ", bootseq_exp_time );
                        _get_token( fh,"retry: ", dummy );

                        switch( atoi(state) )
                        {
                                case 5:
                                case 6:
                                        strcpy(state,"SUBORDINATE");
                                        break;

                                case 7:
                                case 8:
                                        strcpy(state,"SUPERORDINATE");
                                        break;

                                default:
                                        break;
                        }

                        nBytesSent += req_format_write(wp,("<tr bgcolor=\"#b7b7b7\">"
                                "<td align=center width=\"17%%\"><font size=\"2\">%s</td>\n"
                                "<td align=center width=\"17%%\"><font size=\"2\">%s</td>\n"
                                "<td align=center width=\"17%%\"><font size=\"2\">%s</td>\n"
                                "<td align=center width=\"17%%\"><font size=\"2\">%s</td>\n"
				"<td align=center width=\"17%%\"><font size=\"2\">%s</td>\n"
				"<td align=center width=\"17%%\"><font size=\"2\">%s</td>\n"
#if defined(_11s_TEST_MODE_)
				"<td align=center width=\"17%%\"><font size=\"2\">%s</td>\n"
                                "<td align=center width=\"17%%\"><font size=\"2\">%s</td>\n"),
                                        //hwaddr,state,channel,link_rate,rssi,establish_exp_time,bootseq_exp_time);
                                        hwaddr,network,tx_pkts,rx_pkts,link_rate,rssi,establish_exp_time,bootseq_exp_time);
#else
				"<td align=center width=\"17%%\"><font size=\"2\">%s</td>\n"),
					//hwaddr,state,channel,link_rate,rssi);
					hwaddr,network,tx_pkts,rx_pkts,link_rate,rssi,establish_exp_time);
#endif

                        nRecordCount++;
                }
        }

        fclose(fh);

//      printf("\nWarning: recordcount %d\n",nRecordCount);

        if(nRecordCount == 0)
        {
                nBytesSent += req_format_write(wp,("<tr bgcolor=\"#b7b7b7\">"
                        "<td align=center><font size=\"2\">None</td>"
                        "<td align=center width=\"17%%\"><font size=\"2\">---</td>\n"
                        "<td align=center width=\"17%%\"><font size=\"2\">---</td>\n"                        
                        "<td align=center width=\"17%%\"><font size=\"2\">---</td>\n"
                        "<td align=center width=\"17%%\"><font size=\"2\">---</td>\n"
			"<td align=center width=\"17%%\"><font size=\"2\">---</td>\n"
			"<td align=center width=\"17%%\"><font size=\"2\">---</td>\n"
#if defined(_11s_TEST_MODE_)
			"<td align=center width=\"17%%\"><font size=\"2\">---</td>\n"
#endif
			));
        }


        return nBytesSent;
}

/////////////////////////////////////////////////////////////////////////////
int wlMeshRoutingTable(request *wp, int argc, char **argv)
{
        int nBytesSent=0;
        int nRecordCount=0;
        FILE *fh;
        char buf[512];
		unsigned char mac[7];
		char putstr[20];
		int tmp;


		struct mesh_entry{
			char destMac[50],nexthopMac[50],dsn[50], isPortal[10];
			char metric[50],hopcount[10], start[50], end[50], diff[50], flag[10];
			struct mesh_entry *prev;
			struct mesh_entry *next;
		};

		struct mesh_entry *head = NULL;
		struct mesh_entry *p, *np;
        
		

        nBytesSent += req_format_write(wp, ("<tr bgcolor=\"#7F7F7F\">"
        "<td align=center width=\"15%%\"><font size=\"2\"><b>Destination Mesh Point</b></font></td>\n"
        "<td align=center width=\"15%%\"><font size=\"2\"><b>Next-hop Mesh Point</b></font></td>\n"
	"<td align=center width=\"10%%\"><font size=\"2\"><b>Portal Enable</b></font></td>\n"
        //"<td align=center width=\"10%%\"><font size=\"2\"><b>DSN</b></font></td>\n"
        "<td align=center width=\"10%%\"><font size=\"2\"><b>Metric</b></font></td>\n"
        "<td align=center width=\"10%%\"><font size=\"2\"><b>Hop Count</b></font></td>\n"
	"<td align=center width=\"10%%\"><font size=\"2\"><b>Active Clients List</b></font></td>\n"
#if defined(_11s_TEST_MODE_)
        "<td align=center width=\"10%%\"><font size=\"2\"><b>Gen PREQ</b></font></td>\n"
        "<td align=center width=\"10%%\"><font size=\"2\"><b>Rev PREP</b></font></td>\n"
        "<td align=center width=\"10%%\"><font size=\"2\"><b>Delay</b></font></td>\n"
        "<td align=center width=\"10%%\"><font size=\"2\"><b>Flag</b></font></td></tr>\n"
#endif
	));

        fh = fopen(_FILE_MESH_ROUTE, "r");
        if (!fh)
        {
                printf("Warning: cannot open %s\n",_FILE_MESH_ROUTE );
                return -1;
        }


        while( fgets(buf, sizeof buf, fh) != NULL )
        {
                if( strstr(buf,"Mesh route") != NULL )
                {			                	
					np= malloc(sizeof(struct mesh_entry));
					np->next = NULL;
					np->prev = NULL;
					
//                        _get_token( fh,"isvalid: ",isvalid );
                        _get_token( fh,"destMAC: ", np->destMac );
						tmp = strlen(np->destMac)-1;
						np->destMac[tmp] = '\0';
                        _get_token( fh,"nexthopMAC: ", np->nexthopMac );
						_get_token( fh,"portal enable: ", np->isPortal );
                        _get_token( fh,"dsn: ", np->dsn);
                        _get_token( fh,"metric: ", np->metric );
                        _get_token( fh,"hopcount: ", np->hopcount );
						_get_token( fh,"start: ", np->start );
						_get_token( fh,"end: ", np->end );
						_get_token( fh,"diff: ", np->diff );
						_get_token( fh,"flag: ", np->flag );
						
					if (head == NULL){
						head = np;
					} else {
						p = head;
						while (p!=NULL) {
							if (atoi(np->hopcount)< atoi(p->hopcount)){
								if (p->prev!=NULL) {
									p->prev->next = np;
								}
								np->prev = p->prev;
								np->next = p;
								p->prev = np;
								break;
							} else {
								if (p->next == NULL) {
									p->next = np;
									np->prev = p;
									break;
								}
								else
									p = p->next;
							}
						}
					}
                        nRecordCount++;
                }
        }

        fclose(fh);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

								

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		
		if( apmib_get(MIB_WLAN_WLAN_MAC_ADDR, (void *)mac)<0 )
			fprintf(stderr,"get mib error \n");
		
		if ( (mac[0]|mac[1]|mac[2]|mac[3]|mac[4]|mac[5]) == 0){
			memset(mac,0x0,sizeof(mac));
			apmib_get(MIB_HW_WLAN_ADDR, (void *)mac);
		}
		
        if(nRecordCount == 0)
        {
                nBytesSent += req_format_write(wp,("<tr bgcolor=\"#b7b7b7\">"
                        "<td><font size=\"2\">None</td>"
                        "<td align=center width=\"15%%\"><font size=\"2\">---</td>\n"
                        "<td align=center width=\"15%%\"><font size=\"2\">---</td>\n"
                        "<td align=center width=\"10%%\"><font size=\"2\">---</td>\n"                        
                        "<td align=center width=\"10%%\"><font size=\"2\">---</td>\n"
#if defined(_11s_TEST_MODE_)
                        "<td align=center width=\"10%%\"><font size=\"2\">---</td>\n"
                        "<td align=center width=\"10%%\"><font size=\"2\">---</td>\n"
                        "<td align=center width=\"10%%\"><font size=\"2\">---</td>\n"
                        "<td align=center width=\"10%%\"><font size=\"2\">---</td>\n"
#endif
			"<td align=center width=\"10%%\"><font size=\"2\">---</td>\n"
			));
        } else {
			
			p = head;

			while (p!=NULL){

				if (p->destMac[0] == 'M') { 	 
					sprintf(putstr, "%02X%02X%02X%02X%02X%02X"
						, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
				} else {
					strcpy(putstr, p->destMac);
				}
					
        		nBytesSent += req_format_write(wp,("<tr bgcolor=\"#b7b7b7\">"
								"<td align=center width=\"15%%\"><font size=\"2\">%s</td>\n"
					"<td align=center width=\"15%%\"><font size=\"2\">%s</td>\n"
								"<td align=center width=\"15%%\"><font size=\"2\">%s</td>\n"
								"<td align=center width=\"10%%\"><font size=\"2\">%s</td>\n"
								"<td align=center width=\"10%%\"><font size=\"2\">%s</td>\n"
#if defined(_11s_TEST_MODE_)
								"<td align=center width=\"10%%\"><font size=\"2\">%s</td>\n"
								"<td align=center width=\"10%%\"><font size=\"2\">%s</td>\n"
								"<td align=center width=\"10%%\"><font size=\"2\">%s</td>\n"
								"<td align=center width=\"10%%\"><font size=\"2\">%s</td>\n"
#endif

					"<td align=center width=\"10%%\"><input type=\"button\" value=\"Show\" size=\"2\" onClick=\"showProxiedMAC(\'%s\')\"></td>\n"
							   ), p->destMac,p->nexthopMac,p->isPortal,p->metric,p->hopcount,putstr
#if defined(_11s_TEST_MODE_)
					, p->start, p->end, p->diff, p->flag
#endif
					);
				p = p->next;
			}
        }


        return nBytesSent;
}

int wlMeshPortalTable(request *wp, int argc, char **argv)
{
        int nBytesSent=0;
        int nRecordCount=0;
        FILE *fh;
        char buf[512];
        char mac[100],timeout[100],seq[100];

        nBytesSent += req_format_write(wp, ("<tr bgcolor=\"#7F7F7F\">"
        "<td align=center width=\"16%%\"><font size=\"2\"><b>PortalMAC</b></font></td>\n"
#if defined(_11s_TEST_MODE_)
        "<td align=center width=\"16%%\"><font size=\"2\"><b>timeout</b></font></td>\n"
        "<td align=center width=\"16%%\"><font size=\"2\"><b>seqNum</b></font></td></tr>\n"
#endif
	));

        fh = fopen(_FILE_MESH_PORTAL, "r");
        if (!fh)
        {
                printf("Warning: cannot open %s\n",_FILE_MESH_PORTAL );
                return -1;
        }

        while( fgets(buf, sizeof buf, fh) != NULL )
        {
                if( strstr(buf," portal table info..") != NULL )
                {
                        _get_token( fh,"PortalMAC: ",mac );
                        _get_token( fh,"timeout: ",timeout );
                        _get_token( fh,"seqNum: ",seq );

                        nBytesSent += req_format_write(wp,("<tr bgcolor=\"#b7b7b7\">"
                        "<td align=center width=\"16%%\"><font size=\"2\">%s</td>\n"
#if defined(_11s_TEST_MODE_)
                        "<td align=center width=\"16%%\"><font size=\"2\">%s</td>\n"
                        "<td align=center width=\"16%%\"><font size=\"2\">%s</td>\n"
#endif
                                       ), mac
#if defined(_11s_TEST_MODE_)
			,timeout, seq
#endif
			);
                        nRecordCount++;
                }
        }

        fclose(fh);

        if(nRecordCount == 0)
        {
                nBytesSent += req_format_write(wp,("<tr bgcolor=\"#b7b7b7\">"
                        "<td><font size=\"2\">None</td>"
#if defined(_11s_TEST_MODE_)
                        "<td align=center width=\"17%%\"><font size=\"2\">---</td>\n"
                        "<td align=center width=\"17%%\"><font size=\"2\">---</td>\n"
#endif
			));
        }


        return nBytesSent;
}


/////////////////////////////////////////////////////////////////////////////
int wlMeshProxyTable(request *wp, int argc, char **argv)
{
        int nBytesSent=0;
        int nRecordCount=0;
        FILE *fh;
        char buf[512];
        char sta[100],owner[100];

        nBytesSent += req_format_write(wp, ("<tr bgcolor=\"#7F7F7F\">"
        "<td align=center width=\"50%%\"><font size=\"2\"><b>Owner</b></font></td>\n"
        "<td align=center width=\"50%%\"><font size=\"2\"><b>Client</b></font></td></tr>\n"));

        fh = fopen(_FILE_MESH_PROXY , "r");
        if (!fh)
        {
                printf("Warning: cannot open %s\n",_FILE_MESH_PROXY );
                return -1;
        }

        while( fgets(buf, sizeof buf, fh) != NULL )
        {
                if( strstr(buf,"table info...") != NULL )
                {
                        _get_token( fh,"STA_MAC: ",sta );
                        _get_token( fh,"OWNER_MAC: ",owner );
       

                        nBytesSent += req_format_write(wp,("<tr bgcolor=\"#b7b7b7\">"
                                "<td align=center width=\"50%%\"><font size=\"2\">%s</td>\n"
                                "<td align=center width=\"50%%\"><font size=\"2\">%s</td>\n"),
                                owner,sta);
                        nRecordCount++;
                }
        }

        fclose(fh);

        if(nRecordCount == 0)
        {
                nBytesSent += req_format_write(wp,("<tr bgcolor=\"#b7b7b7\">"
                        "<td><font size=\"2\">None</td>"
                        "<td align=center width=\"17%%\"><font size=\"2\">---</td>\n"));
        }

        return nBytesSent;
}

#ifdef _11s_TEST_MODE_
int wlRxStatics(request *wp, int argc, char **argv)
{
        int nBytesSent=0;
        FILE *fh;
        char buf[512];
        char buf2[15][50];

        nBytesSent += req_format_write(wp, ("<tr bgcolor=\"#7F7F7F\">"
        "<td align=center width=\"10%%\"><font size=\"2\"><b>jiffies</b></font></td>\n"
        "<td align=center width=\"10%%\"><font size=\"2\"><b>tx_packets</b></font></td>\n"
        "<td align=center width=\"10%%\"><font size=\"2\"><b>tx_retrys</b></font></td>\n"
        "<td align=center width=\"10%%\"><font size=\"2\"><b>tx_errors</b></font></td>\n"
        "<td align=center width=\"10%%\"><font size=\"2\"><b>rx_packets</b></font></td>\n"
        "<td align=center width=\"10%%\"><font size=\"2\"><b>tx_pkts</b></font></td>\n"
		"<td align=center width=\"10%%\"><font size=\"2\"><b>rx_pkts</b></font></td>\n"         
        "<td align=center width=\"10%%\"><font size=\"2\"><b>rx_crc_errors</b></font></td>\n"));


        fh = fopen(_FILE_MESHSTATS , "r");
        if (!fh)
        {
                printf("Warning: cannot open %s\n",_FILE_MESHSTATS );
                return -1;
        }

		if( fgets(buf, sizeof buf, fh) && strstr(buf,"Statistics..."))
        {
				_get_token( fh,"OPMODE: ", buf2[0] );
				_get_token( fh,"jiffies: ",buf2[1] );
		}
        if( fgets(buf, sizeof buf, fh) && strstr(buf,"Statistics..."))
        {
				_get_token( fh,"tx_packets: ",buf2[2] );
				_get_token( fh,"tx_bytes: ",buf2[3] );
                _get_token( fh,"tx_errors: ",buf2[4] );

				_get_token( fh,"rx_packets: ",buf2[5] );
				_get_token( fh,"rx_bytes: ",buf2[6] );
				_get_token( fh,"rx_errors: ",buf2[7] );				
                _get_token( fh,"rx_crc_errors: ",buf2[8] );

                nBytesSent += req_format_write(wp,("<tr bgcolor=\"#b7b7b7\">"
                        "<td align=center width=\"50%%\"><font size=\"2\">%s</td>\n"
                        "<td align=center width=\"50%%\"><font size=\"2\">%s</td>\n"
                        "<td align=center width=\"50%%\"><font size=\"2\">%s</td>\n"
                        "<td align=center width=\"50%%\"><font size=\"2\">%s</td>\n"
                        "<td align=center width=\"50%%\"><font size=\"2\">%s</td>\n"
                        "<td align=center width=\"50%%\"><font size=\"2\">%s</td>\n"                                     
                        "<td align=center width=\"50%%\"><font size=\"2\">%s</td>\n"
                        "<td align=center width=\"50%%\"><font size=\"2\">%s</td>\n"),
                        buf2[1], buf2[2], buf2[3], buf2[4], buf2[5], buf2[6], buf2[7], buf2[8] ); 
        }

        fclose(fh);

        return nBytesSent;
}
#endif

int wlMeshRootInfo(request *wp, int argc, char **argv)
{
        int nBytesSent=0;
        FILE *fh;
        char rootmac[100];
		char z12[]= "000000000000";
		
        fh = fopen(_FILE_MESH_ROOT , "r");
        if (!fh)
        {
                printf("Warning: cannot open %s\n",_FILE_MESH_ROOT );
                return -1;
        }
	
        _get_token( fh, "ROOT_MAC: ", rootmac );
		if( memcmp(rootmac,z12,12 ) )
             nBytesSent += req_format_write(wp,"%s",  rootmac);
		else
		     nBytesSent += req_format_write(wp,("None"));

        fclose(fh);
        return nBytesSent;
}


#endif // CONFIG_RTK_MESH

/////////////////////////////////////////////////////////////////////////////
int wlAcList(request *wp, int argc, char **argv)
{
	int nBytesSent=0, entryNum, i;
	MACFILTER_T entry;
	char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};

	if ( !apmib_get(MIB_WLAN_MACAC_NUM, (void *)&entryNum)) {
  		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}

	nBytesSent += req_format_write(wp, ("<tr>"
      	"<td align=center width=\"45%%\" bgcolor=\"#808080\"><font size=\"2\"><b>MAC Address</b></font></td>\n"
      	"<td align=center width=\"35%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Comment</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td></tr>\n"));

	for (i=1; i<=entryNum; i++) {
		*((char *)&entry) = (char)i;
		if ( !apmib_get(MIB_WLAN_MACAC_ADDR, (void *)&entry))
			return -1;

		snprintf(tmpBuf, MAX_MSG_BUFFER_SIZE, ("%02x:%02x:%02x:%02x:%02x:%02x"),
			entry.macAddr[0], entry.macAddr[1], entry.macAddr[2],
			entry.macAddr[3], entry.macAddr[4], entry.macAddr[5]);

		nBytesSent += req_format_write(wp, ("<tr>"
			"<td align=center width=\"45%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"35%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
       			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n"),
				tmpBuf, entry.comment, i);
	}
	return nBytesSent;
}

/////////////////////////////////////////////////////////////////////////////
void formWlAc(request *wp, char *path, char *query)
{
	char *strAddMac, *strDelMac, *strDelAllMac, *strVal, *submitUrl, *strEnabled;
	char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};
	int entryNum, i, enabled;
	MACFILTER_T macEntry;

	strAddMac = req_get_cstream_var(wp, ("addFilterMac"), "");
	strDelMac = req_get_cstream_var(wp, ("deleteSelFilterMac"), "");
	strDelAllMac = req_get_cstream_var(wp, ("deleteAllFilterMac"), "");
	strEnabled = req_get_cstream_var(wp, ("wlanAcEnabled"), "");

	if (strAddMac[0]) {
		/*if ( !strcmp(strEnabled, "ON"))
			enabled = 1;
		else
			enabled = 0; */ //by sc_yang
		 enabled = strEnabled[0] - '0';
		if ( apmib_set( MIB_WLAN_MACAC_ENABLED, (void *)&enabled) == 0) {
  			strcpy(tmpBuf, ("Set enabled flag error!"));
			goto setErr_ac;
		}

		strVal = req_get_cstream_var(wp, ("mac"), "");
		if ( !strVal[0] ) {
//			strcpy(tmpBuf, ("Error! No mac address to set."));
			goto setac_ret;
		}
		if (strlen(strVal)!=12 || !string_to_hex(strVal, macEntry.macAddr, 12)) {
			strcpy(tmpBuf, ("Error! Invalid MAC address."));
			goto setErr_ac;
		}

		strVal = req_get_cstream_var(wp, ("comment"), "");
		if ( strVal[0] ) {
			if (strlen(strVal) > COMMENT_LEN-1) {
				strcpy(tmpBuf, ("Error! Comment length too long."));
				goto setErr_ac;
			}
			strcpy((char *)macEntry.comment, strVal);
		}
		else
			macEntry.comment[0] = '\0';

		if ( !apmib_get(MIB_WLAN_MACAC_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto setErr_ac;
		}
		if ( (entryNum + 1) > MAX_WLAN_AC_NUM) {
			strcpy(tmpBuf, ("Cannot add new entry because table is full!"));
			goto setErr_ac;
		}

		// set to MIB. try to delete it first to avoid duplicate case
		apmib_set(MIB_WLAN_AC_ADDR_DEL, (void *)&macEntry);
		if ( apmib_set(MIB_WLAN_AC_ADDR_ADD, (void *)&macEntry) == 0) {
			strcpy(tmpBuf, ("Add table entry error!"));
			goto setErr_ac;
		}
	}

	/* Delete entry */
	if (strDelMac[0]) {
		if ( !apmib_get(MIB_WLAN_MACAC_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto setErr_ac;
		}
		for (i=entryNum; i>0; i--) {
			snprintf(tmpBuf, 20, "select%d", i);

			strVal = req_get_cstream_var(wp, tmpBuf, "");
			if ( !strcmp(strVal, "ON") ) {

				*((char *)&macEntry) = (char)i;
				if ( !apmib_get(MIB_WLAN_MACAC_ADDR, (void *)&macEntry)) {
					strcpy(tmpBuf, ("Get table entry error!"));
					goto setErr_ac;
				}
				if ( !apmib_set(MIB_WLAN_AC_ADDR_DEL, (void *)&macEntry)) {
					strcpy(tmpBuf, ("Delete table entry error!"));
					goto setErr_ac;
				}
			}
		}
	}

	/* Delete all entry */
	if ( strDelAllMac[0]) {
		if ( !apmib_set(MIB_WLAN_AC_ADDR_DELALL, (void *)&macEntry)) {
			strcpy(tmpBuf, ("Delete all table error!"));
			goto setErr_ac;
		}
	}

setac_ret:
	apmib_update_web(CURRENT_SETTING);

#ifndef NO_ACTION
	run_init_script("bridge");
#endif

	submitUrl = req_get_cstream_var(wp, ("submit-url"), "");   // hidden page
	OK_MSG( submitUrl );
  	return;

setErr_ac:
	ERR_MSG(tmpBuf);
}

int advanceHander(request *wp ,char *tmpBuf)
{
	char *strAuth, *strFragTh, *strRtsTh, *strBeacon, *strPreamble, *strAckTimeout;
	char *strRate, /* *strHiddenSSID, */ *strDtim, *strIapp, *strProtection;
	char *strTurbo, *strPower;
	char *strValue;
	AUTH_TYPE_T authType;
	PREAMBLE_T preamble;
	int val;

#ifdef WIFI_SIMPLE_CONFIG
	memset(&wps_config_info, 0, sizeof(struct wps_config_info_struct));
	wps_config_info.caller_id = CALLED_FROM_ADVANCEHANDLER;
	apmib_get(MIB_WLAN_AUTH_TYPE, (void *)&wps_config_info.shared_type);
#endif

	strAuth = req_get_cstream_var(wp, ("authType"), "");
	if (strAuth[0]) {
		if ( !strcmp(strAuth, ("open")))
			authType = AUTH_OPEN;
		else if ( !strcmp(strAuth, ("shared")))
			authType = AUTH_SHARED;
		else if ( !strcmp(strAuth, ("both")))
			authType = AUTH_BOTH;
		else {
			strcpy(tmpBuf, ("Error! Invalid authentication value."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_AUTH_TYPE, (void *)&authType) == 0) {
			strcpy(tmpBuf, ("Set authentication failed!"));
			goto setErr_advance;
		}
	}
	strFragTh = req_get_cstream_var(wp, ("fragThreshold"), "");
	if (strFragTh[0]) {
		if ( !string_to_dec(strFragTh, &val) || val<256 || val>2346) {
			strcpy(tmpBuf, ("Error! Invalid value of fragment threshold."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_FRAG_THRESHOLD, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set fragment threshold failed!"));
			goto setErr_advance;
		}
	}
	strRtsTh = req_get_cstream_var(wp, ("rtsThreshold"), "");
	if (strRtsTh[0]) {
		if ( !string_to_dec(strRtsTh, &val) || val<0 || val>2347) {
			strcpy(tmpBuf, ("Error! Invalid value of RTS threshold."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_RTS_THRESHOLD, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set RTS threshold failed!"));
			goto setErr_advance;
		}
	}

	strBeacon = req_get_cstream_var(wp, ("beaconInterval"), "");
	if (strBeacon[0]) {
		if ( !string_to_dec(strBeacon, &val) || val<20 || val>1024) {
			strcpy(tmpBuf, ("Error! Invalid value of Beacon Interval."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_BEACON_INTERVAL, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set Beacon interval failed!"));
			goto setErr_advance;
		}
	}

	strAckTimeout = req_get_cstream_var(wp, ("ackTimeout"), "");
	if (strAckTimeout[0]) {
		if ( !string_to_dec(strAckTimeout, &val) || val<0 || val>255) {
			strcpy(tmpBuf, ("Error! Invalid value of Ack timeout."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_ACK_TIMEOUT, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set Ack timeout failed!"));
			goto setErr_advance;
		}
	}
#if 0
	// set tx rate
	strRate = req_get_cstream_var(wp, ("txRate"), "");
	if ( strRate[0] ) {
		if ( strRate[0] == '0' ) { // auto
			val = 1;
			if ( apmib_set(MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&val) == 0) {
				strcpy(tmpBuf, ("Set rate adaptive failed!"));
				goto setErr_advance;
			}
		}
		else  {
			val = 0;
			if ( apmib_set(MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&val) == 0) {
				strcpy(tmpBuf, ("Set rate adaptive failed!"));
				goto setErr_advance;
			}  
			val = atoi(strRate);
			val = 1 << (val-1);
			if ( apmib_set(MIB_WLAN_FIX_RATE, (void *)&val) == 0) {
				strcpy(tmpBuf, ("Set fix rate failed!"));
				goto setErr_advance;
			}
			strRate = req_get_cstream_var(wp, ("basicrates"), "");
			if ( strRate[0] ) {
				val = atoi(strRate);
				if ( apmib_set(MIB_WLAN_BASIC_RATES, (void *)&val) == 0) {
					strcpy(tmpBuf, ("Set Tx basic rate failed!"));
					goto setErr_advance;
				}
			}

			strRate = req_get_cstream_var(wp, ("operrates"), "");
			if ( strRate[0] ) {
				val = atoi(strRate);
				if ( apmib_set(MIB_WLAN_SUPPORTED_RATES, (void *)&val) == 0) {
					strcpy(tmpBuf, ("Set Tx operation rate failed!"));
					goto setErr_advance;
				}
			}	
		}
	}
#endif
	val = 0;
	strRate = req_get_cstream_var(wp, ("operRate1M"), "");
	if (strRate==NULL || strRate[0]==0)
		goto skip_rate_setting;
	if ( !strcmp(strRate, ("1M")))
		val |= TX_RATE_1M;
	strRate = req_get_cstream_var(wp, ("operRate2M"), "");
	if ( !strcmp(strRate, ("2M")))
		val |= TX_RATE_2M;
	strRate = req_get_cstream_var(wp, ("operRate5M"), "");
	if ( !strcmp(strRate, ("5M")))
		val |= TX_RATE_5M;
	strRate = req_get_cstream_var(wp, ("operRate11M"), "");
	if ( !strcmp(strRate, ("11M")))
		val |= TX_RATE_11M;
	strRate = req_get_cstream_var(wp, ("operRate6M"), "");
	if ( !strcmp(strRate, ("6M")))
		val |= TX_RATE_6M;
	strRate = req_get_cstream_var(wp, ("operRate9M"), "");
	if ( !strcmp(strRate, ("9M")))
		val |= TX_RATE_9M;
	strRate = req_get_cstream_var(wp, ("operRate12M"), "");
	if ( !strcmp(strRate, ("12M")))
		val |= TX_RATE_12M;
	strRate = req_get_cstream_var(wp, ("operRate18M"), "");
	if ( !strcmp(strRate, ("18M")))
		val |= TX_RATE_18M;			
	strRate = req_get_cstream_var(wp, ("operRate24M"), "");
	if ( !strcmp(strRate, ("24M")))
		val |= TX_RATE_24M;			
	strRate = req_get_cstream_var(wp, ("operRate36M"), "");
	if ( !strcmp(strRate, ("36M")))
		val |= TX_RATE_36M;			
	strRate = req_get_cstream_var(wp, ("operRate48M"), "");
	if ( !strcmp(strRate, ("48M")))
		val |= TX_RATE_48M;			
	strRate = req_get_cstream_var(wp, ("operRate54M"), "");
	if ( !strcmp(strRate, ("54M")))
		val |= TX_RATE_54M;
	if ( apmib_set(MIB_WLAN_SUPPORTED_RATES, (void *)&val) == 0) {
		strcpy(tmpBuf, ("Set Tx operation rate failed!"));
		goto setErr_advance;
	}

	// set basic tx rate
	val = 0;
	strRate = req_get_cstream_var(wp, ("basicRate1M"), "");
	if (strRate==NULL || strRate[0]==0)
		goto skip_rate_setting;	
	if ( !strcmp(strRate, ("1M")))
		val |= TX_RATE_1M;
	strRate = req_get_cstream_var(wp, ("basicRate2M"), "");
	if ( !strcmp(strRate, ("2M")))
		val |= TX_RATE_2M;
	strRate = req_get_cstream_var(wp, ("basicRate5M"), "");
	if ( !strcmp(strRate, ("5M")))
		val |= TX_RATE_5M;
	strRate = req_get_cstream_var(wp, ("basicRate11M"), "");
	if ( !strcmp(strRate, ("11M")))
		val |= TX_RATE_11M;
	strRate = req_get_cstream_var(wp, ("basicRate6M"), "");
	if ( !strcmp(strRate, ("6M")))
		val |= TX_RATE_6M;
	strRate = req_get_cstream_var(wp, ("basicRate9M"), "");
	if ( !strcmp(strRate, ("9M")))
		val |= TX_RATE_9M;
	strRate = req_get_cstream_var(wp, ("basicRate12M"), "");
	if ( !strcmp(strRate, ("12M")))
		val |= TX_RATE_12M;
	strRate = req_get_cstream_var(wp, ("basicRate18M"), "");
	if ( !strcmp(strRate, ("18M")))
		val |= TX_RATE_18M;			
	strRate = req_get_cstream_var(wp, ("basicRate24M"), "");
	if ( !strcmp(strRate, ("24M")))
		val |= TX_RATE_24M;			
	strRate = req_get_cstream_var(wp, ("basicRate36M"), "");
	if ( !strcmp(strRate, ("36M")))
		val |= TX_RATE_36M;			
	strRate = req_get_cstream_var(wp, ("basicRate48M"), "");
	if ( !strcmp(strRate, ("48M")))
		val |= TX_RATE_48M;			
	strRate = req_get_cstream_var(wp, ("basicRate54M"), "");
	if ( !strcmp(strRate, ("54M")))
		val |= TX_RATE_54M;			
	if ( apmib_set(MIB_WLAN_BASIC_RATES, (void *)&val) == 0) {
		strcpy(tmpBuf, ("Set Tx basic rate failed!"));
		goto setErr_advance;
	}		
skip_rate_setting:
	// set preamble
	strPreamble = req_get_cstream_var(wp, ("preamble"), "");
	if (strPreamble[0]) {
		if (!strcmp(strPreamble, ("long")))
			preamble = LONG_PREAMBLE;
		else if (!strcmp(strPreamble, ("short")))
			preamble = SHORT_PREAMBLE;
		else {
			strcpy(tmpBuf, ("Error! Invalid Preamble value."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_PREAMBLE_TYPE, (void *)&preamble) == 0) {
			strcpy(tmpBuf, ("Set Preamble failed!"));
			goto setErr_advance;
		}
	}
//move to basic setting page
#if 0
	// set hidden SSID
	strHiddenSSID = req_get_cstream_var(wp, ("hiddenSSID"), "");
	if (strHiddenSSID[0]) {
		if (!strcmp(strHiddenSSID, ("no")))
			val = 0;
		else if (!strcmp(strHiddenSSID, ("yes")))
			val = 1;
		else {
			strcpy(tmpBuf, ("Error! Invalid hiddenSSID value."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_HIDDEN_SSID, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set hidden ssid failed!"));
			goto setErr_advance;
		}
	}
#endif
	strDtim = req_get_cstream_var(wp, ("dtimPeriod"), "");
	if (strDtim[0]) {
		if ( !string_to_dec(strDtim, &val) || val<1 || val>255) {
			strcpy(tmpBuf, ("Error! Invalid value of DTIM period."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_DTIM_PERIOD, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set DTIM period failed!"));
			goto setErr_advance;
		}
	}

	strIapp = req_get_cstream_var(wp, ("iapp"), "");
	if (strIapp[0]) {
		if (!strcmp(strIapp, ("no")))
			val = 1;
		else if (!strcmp(strIapp, ("yes")))
			val = 0;
		else {
			strcpy(tmpBuf, ("Error! Invalid IAPP value."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_IAPP_DISABLED, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_IAPP_DISABLED failed!"));
			goto setErr_advance;
		}
	}
	strProtection= req_get_cstream_var(wp, ("11g_protection"), "");
	if (strProtection[0]) {
		if (!strcmp(strProtection, ("no")))
			val = 1;
		else if (!strcmp(strProtection, ("yes")))
			val = 0;
		else {
			strcpy(tmpBuf, ("Error! Invalid 11g Protection value."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_PROTECTION_DISABLED, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_PROTECTION_DISABLED failed!"));
			goto setErr_advance;
		}
	}
#if 0	
// for WMM move to basic setting

	strProtection= req_get_cstream_var(wp, ("wmm"), "");
	if (strProtection[0]) {
		if (!strcmp(strProtection, ("on")))
			val = 1;
		else if (!strcmp(strProtection, ("off")))
			val = 0;
		else {
			strcpy(tmpBuf, ("Error! Invalid WMM value."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_WMM_ENABLED, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_WMM_ENABLED failed!"));
			goto setErr_advance;
		}
	}
#endif	
	strTurbo = req_get_cstream_var(wp, ("turbo"), "");
	if (strTurbo[0]) {
		if (!strcmp(strTurbo, ("off")))
			val = 2;
		else if (!strcmp(strTurbo, ("always")))
			val = 1;
		else if (!strcmp(strTurbo, ("auto")))
			val = 0;
		else {
			strcpy(tmpBuf, ("Error! Invalid turbo mode value."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_TURBO_MODE, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_TURBO_MODE failed!"));
			goto setErr_advance;
		}
	}

	strPower= req_get_cstream_var(wp, ("RFPower"), "");
	if (strPower[0]) {		
		if (!strcmp(strPower, ("0")))
			val = 0;
		else if (!strcmp(strPower, ("1")))
			val = 1;
		else if (!strcmp(strPower, ("2")))
			val = 2;
		else if (!strcmp(strPower, ("3")))
			val = 3;
		else if (!strcmp(strPower, ("4")))
			val = 4;
		else {
			strcpy(tmpBuf, ("Error! Invalid RF output power value."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_RFPOWER_SCALE, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_RFPOWER_SCALE failed!"));
			goto setErr_advance;
		}
	}
#if 0
// for 11N
	strProtection= req_get_cstream_var(wp, ("channelBond0"), "");
	if (strProtection[0]) {
		if ( strProtection[0] == '0')
			val = 0;
		else if (strProtection[0] == '1')
			val = 1;
		else {
			strcpy(tmpBuf, ("Error! Invalid Channel Bonding."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_CHANNEL_BONDING, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_CHANNEL_BONDING failed!"));
			goto setErr_advance;
		}
	}

	strProtection= req_get_cstream_var(wp, ("sideBand0"), "");
	if (strProtection[0]) {
		if ( strProtection[0] == '0')
			val = 0;
		else if ( strProtection[0] == '1')
			val = 1;
		else {
			strcpy(tmpBuf, ("Error! Invalid Control SideBand."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_CONTROL_SIDEBAND, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_CONTROL_SIDEBAND failed!"));
			goto setErr_advance;
		}
	}
#endif	
	strProtection= req_get_cstream_var(wp, ("aggregation"), "");
	if (strProtection[0]) {
		if (!strcmp(strProtection, ("disable")))
			val = DISABLED;	// GANTOE & epopen: DISABLED=0 original is DISABLE=0, Because conflict with ../../auth/include/1x_common.h in AP/net-snmp-5.x.x
		else
			val = A_MIXED;	
		if ( apmib_set(MIB_WLAN_AGGREGATION, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_AGGREGATION failed!"));
			goto setErr_advance;
		}
	}
	strValue = req_get_cstream_var(wp, ("block_relay"), "");
		if (strValue[0]) {
			if (!strcmp(strValue, ("enable")))
				val = 1;
			else
				val = 0;			
			apmib_set(MIB_WLAN_BLOCK_RELAY, (void *)&val);
		}
	strProtection= req_get_cstream_var(wp, ("shortGI0"), "");
	if (strProtection[0]) {
		if (!strcmp(strProtection, ("on")))
			val = 1;
		else if (!strcmp(strProtection, ("off")))
			val = 0;
		else {
			strcpy(tmpBuf, ("Error! Invalid short GI."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_SHORT_GI, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_SHORT_GI failed!"));
			goto setErr_advance;
		}
	}

	strValue = req_get_cstream_var(wp, ("tx_stbc"), "");
		if (strValue[0]) {
			if (!strcmp(strValue, ("enable")))
				val = 1;
			else
				val = 0;	
			apmib_set(MIB_WLAN_STBC_ENABLED, (void *)&val);	
		}
		else
		{		
			int chipVersion = getWLAN_ChipVersion();
			if(chipVersion == 1)
			{
				val = 0;	
				apmib_set(MIB_WLAN_STBC_ENABLED, (void *)&val);	
			}


		}
	strValue = req_get_cstream_var(wp, ("coexist_"), "");
		if (strValue[0]) {
			if (!strcmp(strValue, ("enable")))
				val = 1;
			else
				val = 0;	
			apmib_set(MIB_WLAN_COEXIST_ENABLED, (void *)&val);	
		}

		//### add by sen_liu 2011.3.29 TX Beamforming update to mib in 92D 
		strValue = req_get_cstream_var(wp, ("beamforming_"), "");
		if (strValue[0]) {
			if (!strcmp(strValue, ("enable")))
				val = 1;
			else
				val = 0;	
			apmib_set(MIB_WLAN_TX_BEAMFORMING, (void *)&val);	
		}
		//### end
#ifdef WIFI_SIMPLE_CONFIG
	update_wps_configured(1);
#endif

	return 0;
setErr_advance:
	return -1 ;		
}	
/////////////////////////////////////////////////////////////////////////////
void formAdvanceSetup(request *wp, char *path, char *query)
{

	char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};
	char *submitUrl;

	if(advanceHander(wp ,tmpBuf) < 0)
		goto setErr_end;
	apmib_update_web(CURRENT_SETTING);

#ifndef NO_ACTION
	run_init_script("bridge");
#endif

	submitUrl = req_get_cstream_var(wp, ("submit-url"), "");   // hidden page

//	send_redirect_perm(wp, submitUrl);
	OK_MSG(submitUrl);
  	return;

setErr_end:
	ERR_MSG(tmpBuf);
}

/////////////////////////////////////////////////////////////////////////////
int wirelessClientList(request *wp, int argc, char **argv)
{
	int nBytesSent=0, i, found=0;
	WLAN_STA_INFO_Tp pInfo;
	char *buff;
	char mode_buf[20];
	char txrate[20];
	int rateid=0;

	buff = calloc(1, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1));
	if ( buff == 0 ) {
		printf("Allocate buffer failed!\n");
		return 0;
	}

#ifdef MBSSID
	char Root_WLAN_IF[20];

	if (argc == 2) {
		int virtual_index;
		char virtual_name[20];
		strcpy(Root_WLAN_IF, WLAN_IF);
		virtual_index = atoi(argv[argc-1]) - 1;

#ifdef CONFIG_RTL8196B_GW_8M
		if (virtual_index > 0)
			return 0;
#endif
				
		sprintf(virtual_name, "-va%d", virtual_index);
		strcat(WLAN_IF, virtual_name);
	}
#endif

	if ( getWlStaInfo(WLAN_IF,  (WLAN_STA_INFO_Tp)buff ) < 0 ) {
		printf("Read wlan sta info failed!\n");

#ifdef MBSSID
		if (argc == 2)
			strcpy(WLAN_IF, Root_WLAN_IF);
#endif
		return 0;
	}

#ifdef MBSSID
	if (argc == 2)
		strcpy(WLAN_IF, Root_WLAN_IF);
#endif

	for (i=1; i<=MAX_STA_NUM; i++) {
		pInfo = (WLAN_STA_INFO_Tp)&buff[i*sizeof(WLAN_STA_INFO_T)];
		if (pInfo->aid && (pInfo->flag & STA_INFO_FLAG_ASOC)) {
			
		if(pInfo->network & BAND_11N)
			sprintf(mode_buf, "%s", (" 11n"));
		else if (pInfo->network & BAND_11G)
			sprintf(mode_buf,"%s",  (" 11g"));	
		else if (pInfo->network & BAND_11B)
			sprintf(mode_buf, "%s", (" 11b"));
		else if (pInfo->network& BAND_11A)
			sprintf(mode_buf, "%s", (" 11a"));	
		else
			sprintf(mode_buf, "%s", (" ---"));	
		
		//printf("\n\nthe sta txrate=%d\n\n\n", pInfo->txOperaRates);
		
			
		if((pInfo->txOperaRates & 0x80) != 0x80){	
			if(pInfo->txOperaRates%2){
				sprintf(txrate, "%d%s",pInfo->txOperaRates/2, ".5"); 
			}else{
				sprintf(txrate, "%d",pInfo->txOperaRates/2); 
			}
		}else{
			if((pInfo->ht_info & 0x1)==0){ //20M
				if((pInfo->ht_info & 0x2)==0){//long
					for(rateid=0; rateid<16;rateid++){
						if(rate_11n_table_20M_LONG[rateid].id == pInfo->txOperaRates){
							sprintf(txrate, "%s", rate_11n_table_20M_LONG[rateid].rate);
							break;
						}
					}
				}else if((pInfo->ht_info & 0x2)==0x2){//short
					for(rateid=0; rateid<16;rateid++){
						if(rate_11n_table_20M_SHORT[rateid].id == pInfo->txOperaRates){
							sprintf(txrate, "%s", rate_11n_table_20M_SHORT[rateid].rate);
							break;
						}
					}
				}
			}else if((pInfo->ht_info & 0x1)==0x1){//40M
				if((pInfo->ht_info & 0x2)==0){//long
					
					for(rateid=0; rateid<16;rateid++){
						if(rate_11n_table_40M_LONG[rateid].id == pInfo->txOperaRates){
							sprintf(txrate, "%s", rate_11n_table_40M_LONG[rateid].rate);
							break;
						}
					}
				}else if((pInfo->ht_info & 0x2)==0x2){//short
					for(rateid=0; rateid<16;rateid++){
						if(rate_11n_table_40M_SHORT[rateid].id == pInfo->txOperaRates){
							sprintf(txrate, "%s", rate_11n_table_40M_SHORT[rateid].rate);
							break;
						}
					}
				}
			}
			
		}	
			nBytesSent += req_format_write(wp,	
	   		("<tr bgcolor=#b7b7b7><td><font size=2>%02x:%02x:%02x:%02x:%02x:%02x</td>"
			"<td><font size=2>%s</td>"
			"<td><font size=2>%d</td>"
	     		"<td><font size=2>%d</td>"
			"<td><font size=2>%s</td>"
			"<td><font size=2>%s</td>"
			"<td><font size=2>%d</td>"		
			"</tr>"),
			pInfo->addr[0],pInfo->addr[1],pInfo->addr[2],pInfo->addr[3],pInfo->addr[4],pInfo->addr[5],
			mode_buf,
			pInfo->tx_packets, pInfo->rx_packets,
			txrate,
			( (pInfo->flag & STA_INFO_FLAG_ASLEEP) ? "yes" : "no"),
			pInfo->expired_time/100
			);
			found++;
		}
	}
	if (found == 0) {
		nBytesSent += req_format_write(wp,
	   		("<tr bgcolor=#b7b7b7><td><font size=2>None</td>"
			"<td><font size=2>---</td>"
	     		"<td><font size=2>---</td>"
			"<td><font size=2>---</td>"
			"<td><font size=2>---</td>"
			"<td><font size=2>---</td>"
			"<td><font size=2>---</td>"
			"</tr>"));
	}

	free(buff);

	return nBytesSent;
}

/////////////////////////////////////////////////////////////////////////////
void formWirelessTbl(request *wp, char *path, char *query)
{
	char *submitUrl;

	submitUrl = req_get_cstream_var(wp, ("submit-url"), "");
	if (submitUrl[0])
		send_redirect_perm(wp, submitUrl);
}

/////////////////////////////////////////////////////////////////////////////
#ifdef MBSSID
void formWlanMultipleAP(request *wp, char *path, char *query)
{
	char *strVal, *submitUrl;
	int idx, disabled, old_vwlan_idx, band_no, val;
	char varName[20];
	char redirectUrl[200];

//displayPostDate(wp->post_data);	

	old_vwlan_idx = vwlan_idx;

	for (idx=1; idx<=4; idx++) {
		vwlan_idx = idx;		

		sprintf(varName, "wl_disable%d", idx);		
		strVal = req_get_cstream_var(wp, varName, "");
		if ( !strcmp(strVal, "ON"))
			disabled = 0;
		else
			disabled = 1;	
		apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&disabled);

		if (disabled)
			continue;

		sprintf(varName, "wl_band%d", idx);
		strVal = req_get_cstream_var(wp, varName, "");
		if (strVal[0]) {
			band_no = strtol( strVal, (char **)NULL, 10);
			val = (band_no + 1);
			apmib_set(MIB_WLAN_BAND, (void *)&val);
		}

		sprintf(varName, "wl_ssid%d", idx);
		strVal = req_get_cstream_var(wp, varName, "");
		if (strVal[0]) 
			apmib_set( MIB_WLAN_SSID, (void *)strVal);			
	
		sprintf(varName, "TxRate%d", idx);
		strVal = req_get_cstream_var(wp, varName, "");
		if (strVal[0]) {
			if ( strVal[0] == '0' ) { // auto
				val = 1;
				apmib_set(MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&val);
			}
			else  {
				val = 0;
				apmib_set(MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&val);
				val = atoi(strVal);
				val = 1 << (val-1);
				apmib_set(MIB_WLAN_FIX_RATE, (void *)&val);
			}
		}

		sprintf(varName, "wl_hide_ssid%d", idx);
		strVal = req_get_cstream_var(wp, varName, "");
		if (strVal[0]) {
			if ( strVal[0] == '0')
				val = 0;
			else 
				val = 1;
			apmib_set(MIB_WLAN_HIDDEN_SSID, (void *)&val);
		}
	
		sprintf(varName, "wl_wmm_capable%d", idx);
		strVal= req_get_cstream_var(wp, varName, "");
		if (strVal[0]) {
			if ( strVal[0] == '0')
				val = 0;
			else 
				val = 1;
			apmib_set(MIB_WLAN_WMM_ENABLED, (void *)&val);
		}
		else {	//enable wmm in 11N mode always
			int cur_band;
			apmib_get(MIB_WLAN_BAND, (void *)&cur_band);
			if(cur_band == 10 || cur_band ==11) {
				val = 1;
				apmib_set(MIB_WLAN_WMM_ENABLED, (void *)&val);
			}
		}

		sprintf(varName, "wl_access%d", idx);
		strVal= req_get_cstream_var(wp, varName, "");
		if (strVal[0]) {
			if ( strVal[0] == '0')
				val = 0;
			else 
				val = 1;
			apmib_set(MIB_WLAN_ACCESS, (void *)&val);
		}

		sprintf(varName, "tx_restrict%d", idx);
		strVal= req_get_cstream_var(wp, varName, "");
		if (strVal[0]) {
			val = atoi(strVal);
			apmib_set(MIB_WLAN_TX_RESTRICT, (void *)&val);
		}

		sprintf(varName, "rx_restrict%d", idx);
		strVal= req_get_cstream_var(wp, varName, "");
		if (strVal[0]) {
			val = atoi(strVal);
			apmib_set(MIB_WLAN_RX_RESTRICT, (void *)&val);
		}
		
		// force basic and support rate to zero to let driver set default
		val = 0;
		apmib_set(MIB_WLAN_BASIC_RATES, (void *)&val);		
		apmib_set(MIB_WLAN_SUPPORTED_RATES, (void *)&val);
		
		vwlan_idx = old_vwlan_idx;		
	}

	vwlan_idx = old_vwlan_idx;

	apmib_update_web(CURRENT_SETTING);

#ifndef NO_ACTION
	run_init_script("bridge");
#endif

	submitUrl = req_get_cstream_var(wp, ("submit-url"), ""); 
	
	memset(redirectUrl,0x00,sizeof(redirectUrl));
	sprintf(redirectUrl,"/boafrm/formWlanRedirect?redirect-url=%s&wlan_id=%d",submitUrl,wlan_idx);
	
	OK_MSG(redirectUrl);
}
#endif

/////////////////////////////////////////////////////////////////////////////
void formWlSiteSurvey(request *wp, char *path, char *query)
{
 	char *submitUrl, *refresh, *connect, *strSel, *strVal;
	int status, idx, encrypt;
#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
	int wpaPSK;	// For wpa/wpa2
#endif
	unsigned char res, *pMsg=NULL;
	int wait_time, max_wait_time=5;
	char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};
#if defined(WLAN_PROFILE)
	int profile_enabled_id, profileEnabledVal, oriProfileEnabledVal;
#endif
	int isWizard=0;
#ifdef CONFIG_RTK_MESH 
// ==== inserted by GANTOE for site survey 2008/12/26 ==== 
	int mesh_enable=0; 
// fixed by Joule 2009.01.10
	if(apmib_get(MIB_WLAN_MODE, (void *)&mesh_enable) == 0 || mesh_enable < 4) 
		mesh_enable = 0; 	
#endif 


//displayPostDate(wp->post_data);



	submitUrl = req_get_cstream_var(wp, ("submit-url"), "");

	refresh = req_get_cstream_var(wp, ("refresh"), "");
	if ( refresh[0] ) {
		// issue scan request
		wait_time = 0;
		while (1) {
			strVal = req_get_cstream_var(wp, ("ifname"), "");
			if(strVal[0])
			{
				sprintf(WLAN_IF,"%s",strVal);				
			}
			 
			// ==== modified by GANTOE for site survey 2008/12/26 ==== 
			switch(getWlSiteSurveyRequest(WLAN_IF, &status)) 
			{ 
				case -2: 
					printf("-2\n"); 
					strcpy(tmpBuf, ("Auto scan running!!please wait...")); 
					goto ss_err; 
					break; 
				case -1: 
					printf("-2\n"); 
					strcpy(tmpBuf, ("Site-survey request failed!")); 
					goto ss_err; 
					break; 
				default: 
					break; 
			} 
			// ==== GANTOE ====
/*
			if ( getWlSiteSurveyRequest(WLAN_IF,  &status) < 0 ) {
				strcpy(tmpBuf, ("Site-survey request failed!"));
				goto ss_err;
			}
*/
			if (status != 0) {	// not ready
				if (wait_time++ > 5) {
					strcpy(tmpBuf, ("scan request timeout!"));
					goto ss_err;
				}
#ifdef	CONFIG_RTK_MESH
		// ==== modified by GANTOE for site survey 2008/12/26 ==== 
				usleep(1000000 + (rand() % 2000000));
#else
				sleep(1);
#endif
			}
			else
				break;
		}

		// wait until scan completely
		wait_time = 0;
		while (1) {
			res = 1;	// only request request status
			if ( getWlSiteSurveyResult(WLAN_IF, (SS_STATUS_Tp)&res) < 0 ) {
				strcpy(tmpBuf, ("Read site-survey status failed!"));
				free(pStatus);
				pStatus = NULL;
				goto ss_err;
			}
			if (res == 0xff) {   // in progress
			#if defined (CONFIG_RTL_92D_SUPPORT)  && defined (CONFIG_POCKET_ROUTER_SUPPORT)
				/*prolong wait time due to scan both 2.4G and 5G */
				if (wait_time++ > 20) 
			#else
				if (wait_time++ > 10) 
			#endif		
			{
					strcpy(tmpBuf, ("scan timeout!"));
					free(pStatus);
					pStatus = NULL;
					goto ss_err;
				}
				sleep(1);
			}
			else
				break;
		}

		if (submitUrl[0])
			send_redirect_perm(wp, submitUrl);

		return;
	}

	connect = req_get_cstream_var(wp, ("connect"), "");
	if ( connect[0] ) 
	{
		char *wlanifp, *strSSID;
		int rptEnabled=0;
		int wlan_mode=0;
		int wlanvxd_mode=0;
		char *strChannel;
		int channelIdx;

#if defined(WLAN_PROFILE)
	/* disable wireless profile first */
	if(wlan_idx == 0)
	{
		profile_enabled_id = MIB_PROFILE_ENABLED1;
	}
	else
	{
		profile_enabled_id = MIB_PROFILE_ENABLED2;
	}
	profileEnabledVal = 0;
	
	apmib_get(profile_enabled_id, (void *)&oriProfileEnabledVal);

//printf("\r\n oriProfileEnabledVal=[%d],__[%s-%u]\r\n",oriProfileEnabledVal,__FILE__,__LINE__);

	apmib_set(profile_enabled_id, (void *)&profileEnabledVal);

#endif //#if defined(WLAN_PROFILE)

#if 1//defined(CONFIG_RTL_ULINKER)
		if(wlan_idx == 0)
			apmib_get(MIB_REPEATER_ENABLED1, (void *)&rptEnabled);
		else
			apmib_get(MIB_REPEATER_ENABLED2, (void *)&rptEnabled);
#endif
		
#if 1//defined(CONFIG_SMART_REPEATER)
		int opmode=0;
		int vxd_wisp_wan=0;
		char wlanifp_bak[32];
		apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
		apmib_get(MIB_OP_MODE, (void *)&opmode);

		if( (wlan_mode == AP_MODE) && (rptEnabled == 1)
			//&&(opmode == WISP_MODE) 
		)
			vxd_wisp_wan = 1;
#endif

#if defined(CONFIG_RTL_92D_SUPPORT)		
		int phyBand;
		int i;
		unsigned char wlanIfStr[10];
		int band2g5gselect=0;
		apmib_get(MIB_WLAN_BAND2G5G_SELECT, (void *)&band2g5gselect);
		if(band2g5gselect != BANDMODEBOTH)
		{
			for(i=0 ; i<NUM_WLAN_INTERFACE ; i++)
			{
				unsigned char wlanif[10];
				memset(wlanif,0x00,sizeof(wlanif));
				sprintf(wlanif, "wlan%d",i);
				if(SetWlan_idx(wlanif))
				{
					int intVal = 1;
					apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&intVal);
					
				}						
			}
		
			strChannel = req_get_cstream_var(wp, ("pocket_channel"), "");
			
			if(strChannel[0])
			{
				short wlanif;
				
				
				channelIdx = atoi(strChannel);
				
				if(channelIdx > 14) // connect to 5g AP
					phyBand = PHYBAND_5G;
				else
					phyBand = PHYBAND_2G;
					
				wlanif = whichWlanIfIs(phyBand);
				
				memset(wlanIfStr,0x00,sizeof(wlanIfStr));		
				sprintf(wlanIfStr, "wlan%d",wlanif);
			
				if(SetWlan_idx(wlanIfStr))
				{
					int val;
					val = 0;
					apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&val);												
					
					apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
																
#if defined(CONFIG_RTL_ULINKER)						
					if(wlan_mode != CLIENT_MODE && wlan_mode != WDS_MODE && rptEnabled == 1)
						;
					else
#endif
						wlan_mode = CLIENT_MODE;
						
					apmib_set(MIB_WLAN_MODE, (void *)&wlan_mode);												
				}
			
				/* we can't up wlan1 alone, so we swap wlan0 and wlan1 settings */
				if(wlanif != 0)
				{
					swapWlanMibSetting(0,wlanif);			
				}					
			}
		
		}
		
		
#endif //#if defined(CONFIG_RTL_92D_SUPPORT)
		
		wlanifp = req_get_cstream_var(wp, ("wlanif"), "");
		
//printf("\r\n ++++++++++++wlanifp=[%s],__[%s-%u]\r\n",wlanifp,__FILE__,__LINE__);
#if 0//defined(CONFIG_RTL_ULINKER)
		//in ulinker project, we save settings to root interface, than clone to rpt interface
#else
		if(vxd_wisp_wan== 1)
		{
			sprintf(wlanifp_bak, "%s", wlanifp);
			sprintf(wlanifp, "%s-vxd", wlanifp_bak);
		}		
#endif
		
		SetWlan_idx(wlanifp);
		
		strSSID = req_get_cstream_var(wp, ("pocketAP_ssid"), "");
		apmib_set(MIB_WLAN_SSID, (void *)strSSID);

#if 1//def CONFIG_SMART_REPEATER
		if(vxd_wisp_wan== 1)
			if(wlan_idx == 0)
				apmib_set(MIB_REPEATER_SSID1, (void *)strSSID);
			else
				apmib_set(MIB_REPEATER_SSID2, (void *)strSSID);

#endif
		apmib_update(CURRENT_SETTING);

#if defined(CONFIG_RTL_ULINKER)								
		strChannel = req_get_cstream_var(wp, ("pocket_channel"), "");
			
		if(strChannel[0])
		{
			channelIdx = atoi(strChannel);
			if(wlan_mode != CLIENT_MODE && wlan_mode != WDS_MODE && rptEnabled == 1)
			{
				apmib_set( MIB_WLAN_CHANNEL,  (void *)&channelIdx);
			}
		}						
#endif		
						
		strSel = req_get_cstream_var(wp, ("select"), "");
		if (strSel[0]) {
			unsigned char res;
			NETWORK_TYPE_T net;
			int chan;

			if (pStatus == NULL) {
				strcpy(tmpBuf, ("Please refresh again!"));
				goto ss_err;

			}
			sscanf(strSel, "sel%d", &idx);
			if ( idx >= pStatus->number ) { // invalid index
				strcpy(tmpBuf, ("Connect failed 1!"));
				goto ss_err;
			}
#ifdef CONFIG_RTK_MESH 
// ==== inserted by GANTOE for site survey 2008/12/26 ==== 
			if(mesh_enable) 
			{ 
				int i, mesh_index, tmp_index; 
				char original_mesh_id[MESHID_LEN];
				int original_channel = 0;
				
				// backup related info. 
				strcpy(original_mesh_id, "channel");
				if(getWlMib(WLAN_IF, original_mesh_id, strlen(original_mesh_id)) < 0)
				{
					strcpy(tmpBuf, ("get MIB_CHANNEL error!"));
					goto ss_err;
				}
				else
				{
					original_channel = *(int*)original_mesh_id;
				}
				if(apmib_get(MIB_MESH_ID, (void*)original_mesh_id) == 0)
				{
					strcpy(tmpBuf, ("get MIB_MESH_ID error!"));
					goto ss_err;
				}
				
				// send connect request to the driver
				for(tmp_index = 0, mesh_index = 0; tmp_index < pStatus->number && pStatus->number != 0xff; tmp_index++) 
				if(pStatus->bssdb[idx].bdMeshId.Length > 0 && mesh_index++ == idx) 
					break; 
				idx = tmp_index;
				pMsg = "Connect failed 2!!";
				if(!setWlJoinMesh(WLAN_IF, pStatus->bssdb[idx].bdMeshIdBuf - 2, pStatus->bssdb[idx].bdMeshId.Length, pStatus->bssdb[idx].ChannelNumber, 0)) // the problem of padding still exists 
				{ 
					// check whether the link has established
					for(i = 0; i < 10; i++)	// This block might be removed when the mesh peerlink precedure has been completed
					{
						if(!getWlMeshLink(WLAN_IF, pStatus->bssdb[idx].bdBssId, 6))
						{
							char tmp[MESHID_LEN]; 
							int channel; 
							memcpy(tmp, pStatus->bssdb[idx].bdMeshIdBuf - 2, pStatus->bssdb[idx].bdMeshId.Length); // the problem of padding still exists 
							tmp[pStatus->bssdb[idx].bdMeshId.Length] = '\0'; 
							if ( apmib_set(MIB_MESH_ID, (void *)tmp) == 0)
							{ 
								strcpy(tmpBuf, ("Set MeshID error!")); 
								goto ss_err; 
							} 
							// channel = pStatus->bssdb[idx].ChannelNumber; 
							channel = 0; // requirement of Jason, not me 
							if ( apmib_set(MIB_WLAN_CHANNEL, (void*)&channel) == 0)
							{ 
								strcpy(tmpBuf, ("Set Channel error!")); 
								goto ss_err; 
							} 
							apmib_update_web(CURRENT_SETTING); 
							pMsg = "Connect successfully!!"; 
							break;
						}
						usleep(3000000);
					}
				}
				// if failed, reset to the original channel
				if(strcmp(pMsg, "Connect successfully!!"))
				{
					setWlJoinMesh(WLAN_IF, original_mesh_id, strlen(original_mesh_id), original_channel, 1);
				}
			} 
			else 
// ==== GANTOE ==== 
#endif 
			{ 
#if 1
                                unsigned char wlan_idx;
                                char *tmpStr, *wlanif;
                                char wlan_vxd_if[20];
                                char varName[20];
                                unsigned int i,val;
                                wlanif = req_get_cstream_var(wp, ("wlanif"), "");

                                //SetWlan_idx(tmpStr);
 
                                tmpStr = req_get_cstream_var(wp, ("wlan_idx"), "");
                                if(tmpStr[0])
                                {
                                        wlan_idx = atoi(tmpStr);
                                }
 
                                sprintf(varName, "method%d", wlan_idx);
 
                                tmpStr = req_get_cstream_var(wp, varName, "");
                                if(tmpStr[0])
                                {
                                        val = atoi(tmpStr);
                                        if(val == ENCRYPT_DISABLED)
                                        {
                                                ENCRYPT_T encrypt = ENCRYPT_DISABLED;
                                                apmib_set( MIB_WLAN_ENCRYPT, (void *)&encrypt);
                                        }
                                        else if(val == ENCRYPT_WEP)
                                        {
                                                if(wepHandler(wp, tmpBuf, wlan_idx) < 0)
                                                {
                                                        goto ss_err;
                                                }
                                        }
                                        else if(val > ENCRYPT_WEP && val <= ENCRYPT_WPA2_MIXED)
                                        {
                                                if(wpaHandler(wp, tmpBuf, wlan_idx) < 0)
                                                {
                                                        goto ss_err;
                                                }
                                        }
#ifdef CONFIG_RTL_WAPI_SUPPORT
						 else if(val == ENCRYPT_WAPI){
						 	if(wpaHandler(wp, tmpBuf, wlan_idx) < 0)
                                             {
                                                        goto ss_err;
                                             }
						 }
#endif
					}
#else                                
			// check encryption type match or not
			if ( !apmib_get( MIB_WLAN_ENCRYPT, (void *)&encrypt) ) {
				strcpy(tmpBuf, ("Check encryption error!"));
				goto ss_err;
			}
			else {
				// no encryption
				if (encrypt == ENCRYPT_DISABLED)
				{
					if (pStatus->bssdb[idx].bdCap & 0x00000010) {
						strcpy(tmpBuf, ("Encryption type mismatch!"));
						goto ss_err;
					}
					else
						; // success
				}
				// legacy encryption
				else if (encrypt == ENCRYPT_WEP)
				{
					if ((pStatus->bssdb[idx].bdCap & 0x00000010) == 0) {
						strcpy(tmpBuf, ("Encryption type mismatch!"));
						goto ss_err;
					}
					else if (pStatus->bssdb[idx].bdTstamp[0] != 0) {
						strcpy(tmpBuf, ("Encryption type mismatch!"));
						goto ss_err;
					}
					else
						; // success
				}
#if defined(CONFIG_RTL_WAPI_SUPPORT)
				else if (encrypt == ENCRYPT_WAPI)
				{
					if ((pStatus->bssdb[idx].bdCap & 0x00000010) == 0) {
						strcpy(tmpBuf, ("Encryption type mismatch!"));
						goto ss_err;
					}
					else if (pStatus->bssdb[idx].bdTstamp[0] != SECURITY_INFO_WAPI) {
						strcpy(tmpBuf, ("Encryption type mismatch!"));
						goto ss_err;
					}
					else
						; // success
				}
#endif
				// WPA/WPA2
				else
				{
					int isPSK, auth;
					apmib_get(MIB_WLAN_WPA_AUTH, (void *)&auth);
					if (auth == WPA_AUTH_PSK)
						isPSK = 1;
					else
						isPSK = 0;					
								
#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
					wpaPSK=isPSK;
#endif
								
					if ((pStatus->bssdb[idx].bdCap & 0x00000010) == 0) {
						strcpy(tmpBuf, ("Encryption type mismatch!"));
						goto ss_err;
					}
					else if (pStatus->bssdb[idx].bdTstamp[0] == 0) {
						strcpy(tmpBuf, ("Encryption type mismatch!"));
						goto ss_err;
					}
					else if (encrypt == ENCRYPT_WPA) {
						if (((pStatus->bssdb[idx].bdTstamp[0] & 0x0000ffff) == 0) || 
								(isPSK && !(pStatus->bssdb[idx].bdTstamp[0] & 0x4000)) ||
								(!isPSK && (pStatus->bssdb[idx].bdTstamp[0] & 0x4000)) ) {						
						strcpy(tmpBuf, ("Encryption type mismatch!"));
						goto ss_err;
					}
					}
					else if (encrypt == ENCRYPT_WPA2) {
						if (((pStatus->bssdb[idx].bdTstamp[0] & 0xffff0000) == 0) ||
								(isPSK && !(pStatus->bssdb[idx].bdTstamp[0] & 0x40000000)) ||
								(!isPSK && (pStatus->bssdb[idx].bdTstamp[0] & 0x40000000)) ) {
							strcpy(tmpBuf, ("Encryption type mismatch!"));
							goto ss_err;
						}
					}	
					else
						; // success
				}
			}
#endif

#if 0
			// Set SSID, network type to MIB
			memcpy(tmpBuf, pStatus->bssdb[idx].bdSsIdBuf, pStatus->bssdb[idx].bdSsId.Length);
			tmpBuf[pStatus->bssdb[idx].bdSsId.Length] = '\0';
			
			memset(tmpBuf,0x00,sizeof(tmpBuf));
			
			tmpStr = req_get_cstream_var(wp, ("pocketAP_ssid"), "");
			if(tmpStr[0])
				sprintf(tmpBuf,"%s",tmpStr);
			
//printf("\r\n tmpBuf=[%s],__[%s-%u]\r\n",tmpBuf,__FILE__,__LINE__);
			
			if ( apmib_set(MIB_WLAN_SSID, (void *)tmpBuf) == 0) {
				strcpy(tmpBuf, ("Set SSID error!"));
				goto ss_err;
			}
#endif

			if ( pStatus->bssdb[idx].bdCap & cESS )
				net = INFRASTRUCTURE;
			else
				net = ADHOC;
			
			if ( apmib_set(MIB_WLAN_NETWORK_TYPE, (void *)&net) == 0) {
				strcpy(tmpBuf, ("Set MIB_WLAN_NETWORK_TYPE failed!"));
				goto ss_err;
			}

			if (net == ADHOC) {
				chan = pStatus->bssdb[idx].ChannelNumber;
				if ( apmib_set( MIB_WLAN_CHANNEL, (void *)&chan) == 0) {
   					strcpy(tmpBuf, ("Set channel number error!"));
					goto ss_err;
				}
				int is_40m_bw = (pStatus->bssdb[idx].bdTstamp[1] & 2) ? 1 : 0;				
				apmib_set(MIB_WLAN_CHANNEL_BONDING, (void *)&is_40m_bw);				
			}

#if 0//defined(CONFIG_RTL_ULINKER) //repeater mode: clone wlan setting to wlan-vxd and modify wlan ssid

		if(wlan_mode != CLIENT_MODE && wlan_mode != WDS_MODE && rptEnabled == 1)
		{
			int isUpnpEnabled=0;
			int ori_vwlan_idx = vwlan_idx;
			char ssidBuf[64];
			
			
			vwlan_idx = NUM_VWLAN_INTERFACE;
			
			
			/* get original setting in vxd interface */
			apmib_get(MIB_WLAN_WSC_UPNP_ENABLED, (void *)&isUpnpEnabled);
			apmib_get(MIB_WLAN_MODE, (void *)&wlanvxd_mode);
			
									
			ulinker_wlan_mib_copy(&pMib->wlan[wlan_idx][NUM_VWLAN_INTERFACE], &pMib->wlan[wlan_idx][0]);
			
			/* restore original setting in vxd interface and repeater ssid*/			
			apmib_set(MIB_WLAN_WSC_UPNP_ENABLED, (void *)&isUpnpEnabled);
			apmib_set(MIB_WLAN_MODE, (void *)&wlanvxd_mode);
			
			vwlan_idx = ori_vwlan_idx;
			
			/* add "-ext" at last of wlan ssid */
			apmib_get( MIB_WLAN_SSID,  (void *)ssidBuf);

			if(wlan_idx == 0)
				apmib_set(MIB_REPEATER_SSID1, (void *)&ssidBuf);
			else
				apmib_set(MIB_REPEATER_SSID2, (void *)&ssidBuf);

			
			if(strlen(ssidBuf)<sizeof(ssidBuf)+4)
			{
				strcat(ssidBuf,"-ext");
				apmib_set( MIB_WLAN_SSID,  (void *)ssidBuf);
				apmib_set( MIB_WLAN_WSC_SSID, (void *)ssidBuf);
			}
		}
#endif

			apmib_update_web(CURRENT_SETTING);

#if 1 //reinit wlan interface and mib
                        char command[50];
#if 1//def CONFIG_SMART_REPEATER
						if(vxd_wisp_wan){
							sprintf(command,"ifconfig %s-vxd down",wlanif);
                        	system(command);
                        }
#endif

                        sprintf(command,"ifconfig %s down",wlanif);
                        system(command);
                        sprintf(command,"flash set_mib %s",wlanif);
                        system(command);
                        sprintf(command,"ifconfig %s up",wlanif);
                        system(command);
#if 1//def CONFIG_SMART_REPEATER
						if(vxd_wisp_wan){
							sprintf(command,"flash set_mib %s-vxd",wlanif);                      
	                        system(command);
							sprintf(command,"ifconfig %s-vxd up",wlanif);
	                        system(command);
                        }
#endif                        
					
					
		#if defined(CONFIG_RTL_ULINKER)
			run_init_script_flag = 1;
		#else
			 // wlan0 entering forwarding state need some time
			sleep(3);

			_Start_Wlan_Applications();
		#endif
#endif

			res = idx;
			wait_time = 0;

#if 0	//Because reinit wlan app above, so don't need wlJoinRequest here.
			while (1) {
				if ( getWlJoinRequest(WLAN_IF, &pStatus->bssdb[idx], &res) < 0 ) {
					strcpy(tmpBuf, ("Join request failed!"));
					goto ss_err;
				}
				if ( res == 1 ) { // wait
				#if defined (CONFIG_RTL_92D_SUPPORT)  && defined (CONFIG_POCKET_ROUTER_SUPPORT)
					/*prolong join wait time for pocket ap*/
					if (wait_time++ > 10) 
				#else
					if (wait_time++ > 5) 
				#endif
					{
						strcpy(tmpBuf, ("connect-request timeout!"));
						goto ss_err;
					}
					sleep(1);
					continue;
				}
				break;
			}

			if ( res == 2 ) // invalid index
				pMsg = "Connect failed 3!";
			else 
			{
				wait_time = 0;
				while (1) {
					if ( getWlJoinResult(WLAN_IF, &res) < 0 ) {
						strcpy(tmpBuf, ("Get Join result failed!"));
						goto ss_err;
					}
					if ( res != 0xff ) { // completed
					

						break;
					}
					else
					{
						if (wait_time++ > 10) {
							strcpy(tmpBuf, ("connect timeout!"));
							goto ss_err;
						}
					}
					sleep(1);
				}

				if ( res!=STATE_Bss && res!=STATE_Ibss_Idle && res!=STATE_Ibss_Active )
					pMsg = "Connect failed 4!";
				else {					
					status = 0;
					
					apmib_get( MIB_WLAN_ENCRYPT, (void *)&encrypt);
					
					//if (encrypt == ENCRYPT_WPA || encrypt == ENCRYPT_WPA2) {
					if (encrypt == ENCRYPT_WPA || encrypt == ENCRYPT_WPA2 || encrypt == ENCRYPT_WAPI) {
						bss_info bss;
						wait_time = 0;
						
						max_wait_time=10;	//Need more test, especially for 802.1x client mode
						
						while (wait_time++ < max_wait_time) {
							getWlBssInfo(WLAN_IF, &bss);
							if (bss.state == STATE_CONNECTED){
								break;
							}
							sleep(1);
						}
						if (wait_time > max_wait_time)						
							status = 1;
					}

					if (status)
						pMsg = "Connect failed 5!";
					else
						pMsg = "Connect successfully!";
				}
			}
#else
			{
				wait_time = 0;
				while (1) 
					{

#if 1//def CONFIG_SMART_REPEATER

			
						if(vxd_wisp_wan && !strstr(WLAN_IF,"-vxd"))
							strcat(WLAN_IF,"-vxd");
						
#endif

						if ( getWlJoinResult(WLAN_IF, &res) < 0 ) {
							strcpy(tmpBuf, ("Get Join result failed!"));
							goto ss_err;
						}

					if(res==STATE_Bss  || res==STATE_Ibss_Idle || res==STATE_Ibss_Active) { // completed 
						break;
					}
					else
					{
						if (wait_time++ > 10) {
							strcpy(tmpBuf, ("connect timeout!"));
							goto ss_err;
						}
					}
					sleep(1);
				}

				if ( res!=STATE_Bss && res!=STATE_Ibss_Idle && res!=STATE_Ibss_Active )
					pMsg = (unsigned char *)"Connect failed 4!";
				else {					
					status = 0;
					
					apmib_get( MIB_WLAN_ENCRYPT, (void *)&encrypt);
					
					//if (encrypt == ENCRYPT_WPA || encrypt == ENCRYPT_WPA2) {
					if (encrypt == ENCRYPT_WPA || encrypt == ENCRYPT_WPA2 || encrypt == ENCRYPT_WAPI) {
						bss_info bss;
						wait_time = 0;
						
						max_wait_time=15;	//Need more test, especially for 802.1x client mode
						
						while (wait_time++ < max_wait_time) {
							getWlBssInfo(WLAN_IF, &bss);
							if (bss.state == STATE_CONNECTED){
								break;
							}
							sleep(1);
						}
						if (wait_time > max_wait_time)						
							status = 1; //fail
					}

					if (status)
						pMsg =  (unsigned char *)"Connect failed 5!";
					else
						pMsg =  (unsigned char *)"Connect successfully!";

				}
			}
#endif
		}


#if defined(WLAN_PROFILE)
		/* disable wireless profile first */
		if(wlan_idx == 0)
		{
			profile_enabled_id = MIB_PROFILE_ENABLED1;
		}
		else
		{
			profile_enabled_id = MIB_PROFILE_ENABLED2;
		}

//printf("\r\n wlan_idx=[%d],__[%s-%u]\r\n",wlan_idx,__FILE__,__LINE__);
//printf("\r\n profile_enabled_id=[%d],__[%s-%u]\r\n",profile_enabled_id,__FILE__,__LINE__);
//printf("\r\n oriProfileEnabledVal=[%d],__[%s-%u]\r\n",oriProfileEnabledVal,__FILE__,__LINE__);
		apmib_set(profile_enabled_id, (void *)&oriProfileEnabledVal);

		apmib_update_web(CURRENT_SETTING);

#endif //#if defined(WLAN_PROFILE)


#if defined(CONFIG_POCKET_AP_SUPPORT)  
			if(!status)
			{
				pMsg = "Connect successfully! Please wait while rebooting.";
				OK_MSG1(pMsg, submitUrl);
				sleep(2);
				system("reboot");
			} 
#elif defined(CONFIG_POCKET_ROUTER_SUPPORT)// || defined(CONFIG_RTL_ULINKER)
			{
#ifndef NO_ACTION
				run_init_script("all");
#endif
				REBOOT_WAIT("/wizard.htm");
				//REBOOT_WAIT("/wlsurvey.htm");
				
			} 
#else
			{
				RET_SURVEY_PAGE(pMsg, submitUrl, (status?0:1), wlan_idx, isWizard);
			}
#endif
		}
#if 1//defined(CONFIG_SMART_REPEATER)
		if(vxd_wisp_wan)
		{
			char*ptmp;
			SetWlan_idx(wlanifp_bak);
			ptmp=strstr(WLAN_IF,"-vxd");
			if(ptmp)
				memset(ptmp,0,sizeof(char)*strlen("-vxd"));
		}
#endif



	}
	return;

ss_err:
#if defined(WLAN_PROFILE)
	if(wlan_idx == 0)
	{
		profile_enabled_id = MIB_PROFILE_ENABLED1;
	}
	else
	{
		profile_enabled_id = MIB_PROFILE_ENABLED2;
	}

	apmib_set(profile_enabled_id, (void *)&oriProfileEnabledVal);

	apmib_update_web(CURRENT_SETTING);

#endif //#if defined(WLAN_PROFILE)	
	ERR_MSG(tmpBuf);
}

/////////////////////////////////////////////////////////////////////////////
int wlSiteSurveyTbl(request *wp, int argc, char **argv)
{
	int nBytesSent=0, i;
#ifdef CONFIG_RTK_MESH 
// ==== inserted by GANTOE for site survey 2008/12/26 ==== 
	int mesh_enable; 
#endif 
	BssDscr *pBss;
	char tmpBuf[MAX_MSG_BUFFER_SIZE], ssidbuf[40], tmp1Buf[10], tmp2Buf[20], wpa_tkip_aes[20],wpa2_tkip_aes[20];
#ifdef CONFIG_RTK_MESH 
// ==== inserted by GANTOE for site survey 2008/12/26 ==== 
	char meshidbuf[40] ;
#endif 

	WLAN_MODE_T mode;
	bss_info bss;

	if (pStatus==NULL) {
		pStatus = calloc(1, sizeof(SS_STATUS_T));
		if ( pStatus == NULL ) {
			printf("Allocate buffer failed!\n");
			return 0;
		}
	}

	pStatus->number = 0; // request BSS DB

	if ( getWlSiteSurveyResult(WLAN_IF, pStatus) < 0 ) {
		//ERR_MSG("Read site-survey status failed!");
		req_format_write(wp, "Read site-survey status failed!");
		free(pStatus); //sc_yang
		pStatus = NULL;
		return 0;
	}

	if ( !apmib_get( MIB_WLAN_MODE, (void *)&mode) ) {
		printf("Get MIB_WLAN_MODE MIB failed!");
		return 0;
	}
#ifdef CONFIG_RTK_MESH
// ==== inserted by GANTOE for site survey 2008/12/26 ====
	mesh_enable = mode > 3 ? 1 : 0;	// Might to be corrected after the code refinement
#endif
	if ( getWlBssInfo(WLAN_IF, &bss) < 0) {
		printf("Get bssinfo failed!");
		return 0;
	}

// ==== inserted by GANTOE for site survey 2008/12/26 ==== 
//#ifdef CONFIG_RTK_MESH
#if 0
	if(mesh_enable) 
	{ 
		nBytesSent += req_format_write(wp, ("<tr>" 
		"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b>MESHID</b></font></td>\n" 
		"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>MAC ADDR</b></font></td>\n" 
		"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Channel</b></font></td>\n")); 
		nBytesSent += req_format_write(wp, ("<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td></tr>\n")); 

		for (i=0; i<pStatus->number && pStatus->number!=0xff; i++) 
		{ 
			pBss = &pStatus->bssdb[i]; 
			if(pBss->bdMeshId.Length == 0)
				continue; 

			memcpy(meshidbuf, pBss->bdMeshIdBuf - 2, pBss->bdMeshId.Length); // the problem of padding still exists 
			meshidbuf[pBss->bdMeshId.Length] = '\0'; 

			snprintf(tmpBuf, 200, ("%02x:%02x:%02x:%02x:%02x:%02x"), 
				pBss->bdBssId[0], pBss->bdBssId[1], pBss->bdBssId[2], 
				pBss->bdBssId[3], pBss->bdBssId[4], pBss->bdBssId[5]); 
			memcpy(ssidbuf, pBss->bdSsIdBuf, pBss->bdSsId.Length); 
			ssidbuf[pBss->bdSsId.Length] = '\0'; 
			
			nBytesSent += req_format_write(wp, ("<tr>" 
				"<td align=left width=\"30%%\" bgcolor=\"#C0C0C0\"><pre><font size=\"2\">%s</td>\n" 
				"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n" 
				"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%d</td>\n"), 
				meshidbuf, tmpBuf, pBss->ChannelNumber); 
            
			nBytesSent += req_format_write(wp, 
			("<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=" 
				"\"select\" value=\"sel%d\" onClick=\"enableConnect()\"></td></tr>\n"), i); 
		} 
	} 
	else 
#endif 
	{ 
		int rptEnabled=0;
#if defined(CONFIG_SMART_REPEATER)
		int opmode=0;
		apmib_get(MIB_OP_MODE, (void *)&opmode);
#endif
		if(wlan_idx == 0)
			apmib_get(MIB_REPEATER_ENABLED1, (void *)&rptEnabled);
		else
			apmib_get(MIB_REPEATER_ENABLED2, (void *)&rptEnabled);
		
	nBytesSent += req_format_write(wp, ("<tr>"
	"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b>SSID</b></font></td>\n"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>BSSID</b></font></td>\n"
	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Channel</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Type</b></font></td>\n"
      	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Encrypt</b></font></td>\n"
	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Signal</b></font></td>\n"));
	if( (mode == CLIENT_MODE )
#if defined(CONFIG_RTL_ULINKER)
		|| (mode == AP_MODE && rptEnabled == 1)
#endif
#if defined(CONFIG_SMART_REPEATER)
		||( (mode == AP_MODE) && (rptEnabled == 1) 
		//&& (opmode == WISP_MODE)
	)
#endif
	)
	{
		nBytesSent += req_format_write(wp, ("<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td></tr>\n"));
	}
	else
		nBytesSent += req_format_write(wp, ("</tr>\n"));

	for (i=0; i<pStatus->number && pStatus->number!=0xff; i++) {
		pBss = &pStatus->bssdb[i];
		snprintf(tmpBuf, 200, ("%02x:%02x:%02x:%02x:%02x:%02x"),
			pBss->bdBssId[0], pBss->bdBssId[1], pBss->bdBssId[2],
			pBss->bdBssId[3], pBss->bdBssId[4], pBss->bdBssId[5]);

		memcpy(ssidbuf, pBss->bdSsIdBuf, pBss->bdSsId.Length);
		ssidbuf[pBss->bdSsId.Length] = '\0';

#if defined(CONFIG_RTK_MESH)
		if( pBss->bdMeshId.Length )
		{
			memcpy(meshidbuf, pBss->bdMeshIdBuf - 2, pBss->bdMeshId.Length);	// the problem of padding still exists

			if( !memcmp(ssidbuf, meshidbuf,pBss->bdMeshId.Length-1) )
				continue;
		}
#endif


		if (pBss->network==BAND_11B)
			strcpy(tmp1Buf, (" (B)"));
		else if (pBss->network==BAND_11G)
			strcpy(tmp1Buf, (" (G)"));	
		else if (pBss->network==(BAND_11G|BAND_11B))
			strcpy(tmp1Buf, (" (B+G)"));
		else if (pBss->network==(BAND_11N))
			strcpy(tmp1Buf, (" (N)"));		
		else if (pBss->network==(BAND_11G|BAND_11N))
			strcpy(tmp1Buf, (" (G+N)"));	
		else if (pBss->network==(BAND_11G|BAND_11B | BAND_11N))
			strcpy(tmp1Buf, (" (B+G+N)"));	
		else if(pBss->network== BAND_11A)
			strcpy(tmp1Buf, (" (A)"));
		else if(pBss->network== (BAND_11A | BAND_11N))
			strcpy(tmp1Buf, (" (A+N)"));	
		else
			sprintf(tmp1Buf, (" -%d-"),pBss->network);

		memset(wpa_tkip_aes,0x00,sizeof(wpa_tkip_aes));
		memset(wpa2_tkip_aes,0x00,sizeof(wpa2_tkip_aes));
		
		if ((pBss->bdCap & cPrivacy) == 0)
			sprintf(tmp2Buf, "no");
		else {
			if (pBss->bdTstamp[0] == 0)
				sprintf(tmp2Buf, "WEP");
#if defined(CONFIG_RTL_WAPI_SUPPORT)
			else if (pBss->bdTstamp[0] == SECURITY_INFO_WAPI)
				sprintf(tmp2Buf, "WAPI");
#endif
			else {
				int wpa_exist = 0, idx = 0;
				if (pBss->bdTstamp[0] & 0x0000ffff) {
					idx = sprintf(tmp2Buf, "WPA");
					if (((pBss->bdTstamp[0] & 0x0000f000) >> 12) == 0x4)
						idx += sprintf(tmp2Buf+idx, "-PSK");
					else if(((pBss->bdTstamp[0] & 0x0000f000) >> 12) == 0x2)
						idx += sprintf(tmp2Buf+idx, "-1X");
					wpa_exist = 1;

					if (((pBss->bdTstamp[0] & 0x00000f00) >> 8) == 0x5)
						sprintf(wpa_tkip_aes,"%s","aes/tkip");
					else if (((pBss->bdTstamp[0] & 0x00000f00) >> 8) == 0x4)
						sprintf(wpa_tkip_aes,"%s","aes");
					else if (((pBss->bdTstamp[0] & 0x00000f00) >> 8) == 0x1)
						sprintf(wpa_tkip_aes,"%s","tkip");
				}
				if (pBss->bdTstamp[0] & 0xffff0000) {
					if (wpa_exist)
						idx += sprintf(tmp2Buf+idx, "/");
					idx += sprintf(tmp2Buf+idx, "WPA2");
					if (((pBss->bdTstamp[0] & 0xf0000000) >> 28) == 0x4)
						idx += sprintf(tmp2Buf+idx, "-PSK");
					else if (((pBss->bdTstamp[0] & 0xf0000000) >> 28) == 0x2)
						idx += sprintf(tmp2Buf+idx, "-1X");

					if (((pBss->bdTstamp[0] & 0x0f000000) >> 24) == 0x5)
						sprintf(wpa2_tkip_aes,"%s","aes/tkip");
					else if (((pBss->bdTstamp[0] & 0x0f000000) >> 24) == 0x4)
						sprintf(wpa2_tkip_aes,"%s","aes");
					else if (((pBss->bdTstamp[0] & 0x0f000000) >> 24) == 0x1)
						sprintf(wpa2_tkip_aes,"%s","tkip");
				}
			}
		}

#if 0
		if( mesh_enable && (pBss->bdMeshId.Length > 0) )
		{
			nBytesSent += req_format_write(wp, ("<tr>"
			"<td align=left width=\"20%%\" bgcolor=\"#C0C0C0\"><pre><font size=\"2\">%s</td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%d%s</td>\n"     
      			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%d</td>\n"),
			ssidbuf, tmpBuf, pBss->ChannelNumber, tmp1Buf, "Mesh Node", tmp2Buf, pBss->rssi);
		}
		else
#endif
		{
			nBytesSent += req_format_write(wp, ("<tr>"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><pre><font size=\"2\">%s</td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%d%s</td>\n"     
      			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%d</td>\n"),
			ssidbuf, tmpBuf, pBss->ChannelNumber, tmp1Buf,
			((pBss->bdCap & cIBSS) ? "Ad hoc" : "AP"), tmp2Buf, pBss->rssi);
		}

		if( ( mode == CLIENT_MODE )
#if defined(CONFIG_RTL_ULINKER)
		|| (mode == AP_MODE && rptEnabled == 1)
#endif
#if defined(CONFIG_SMART_REPEATER)
		||( (mode == AP_MODE) && (rptEnabled == 1) 
		//&& (opmode == WISP_MODE)
		)
#endif

		)
		{
			nBytesSent += req_format_write(wp,
			("<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"> <input type=\"hidden\" id=\"selSSID_%d\" value=\"%s\" > <input type=\"hidden\" id=\"selChannel_%d\" value=\"%d\" > <input type=\"hidden\" id=\"selEncrypt_%d\" value=\"%s\" > <input type=\"hidden\" id=\"wpa_tkip_aes_%d\" value=\"%s\" > <input type=\"hidden\" id=\"wpa2_tkip_aes_%d\" value=\"%s\" > <input type=\"radio\" name="
			"\"select\" value=\"sel%d\" onClick=\"enableConnect(%d)\"></td></tr>\n"), i,ssidbuf,i,pBss->ChannelNumber,i,tmp2Buf,i,wpa_tkip_aes,i,wpa2_tkip_aes ,i,i);
		}
		else
			nBytesSent += req_format_write(wp, ("</tr>\n"));
	}

	if( pStatus->number == 0 )
	{
		if (( mode == CLIENT_MODE )
#if defined(CONFIG_RTL_ULINKER)
		|| (mode == AP_MODE && rptEnabled == 1)
#endif
#if defined(CONFIG_SMART_REPEATER)
		||( (mode == AP_MODE) && (rptEnabled == 1) 
		//&& (opmode == WISP_MODE)
		)
#endif
		)
		{
			nBytesSent += req_format_write(wp, ("<tr>"
	                "<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><pre><font size=\"2\">None</td>\n"
	                "<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"
	                "<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"
	                "<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"
	                "<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"
	                "<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"
	                "<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"
	                "</tr>\n"));
		}
		else
		{
			nBytesSent += req_format_write(wp, ("<tr>"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><pre><font size=\"2\">None</td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"
			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"
			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"
			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"
			"</tr>\n"));
		}
	}
	nBytesSent += req_format_write(wp, ("</table>\n"));

#ifdef CONFIG_RTK_MESH
	if(mesh_enable) 
	{ 
		int mesh_count = 0;

		nBytesSent += req_format_write(wp, ("<table border=\"1\" width=\"500\">"
		"<tr><h4><font><br><br>List of Mesh Points</font></tr><tr>"
		"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Mesh ID</b></font></td>\n" 
		"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>MAC Adddress</b></font></td>\n" 
		"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Channel</b></font></td>\n")); 
		nBytesSent += req_format_write(wp, ("<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td></tr>\n"));

		for (i=0; i<pStatus->number && pStatus->number!=0xff; i++) 
		{
			pBss = &pStatus->bssdb[i]; 
			if(pBss->bdMeshId.Length == 0)
				continue; 
			mesh_count++;
			memcpy(meshidbuf, pBss->bdMeshIdBuf - 2, pBss->bdMeshId.Length); // the problem of padding still exists
			meshidbuf[pBss->bdMeshId.Length] = '\0'; 

			snprintf(tmpBuf, 200, ("%02x:%02x:%02x:%02x:%02x:%02x"), 
				pBss->bdBssId[0], pBss->bdBssId[1], pBss->bdBssId[2], 
				pBss->bdBssId[3], pBss->bdBssId[4], pBss->bdBssId[5]); 
			memcpy(ssidbuf, pBss->bdSsIdBuf, pBss->bdSsId.Length); 
			ssidbuf[pBss->bdSsId.Length] = '\0'; 
			
			nBytesSent += req_format_write(wp, ("<tr>" 
				"<td align=left width=\"30%%\" bgcolor=\"#C0C0C0\"><pre><font size=\"2\">%s</td>\n" 
				"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n" 
				"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%d</td>\n"), 
				meshidbuf, tmpBuf, pBss->ChannelNumber); 
            
			nBytesSent += req_format_write(wp, 
			("<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=" 
				"\"select\" value=\"sel%d\" onClick=\"enableConnect()\"></td></tr>\n"), i); 
		}
		if( mesh_count == 0 )
		{
			nBytesSent += req_format_write(wp, ("<tr>"
			"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><pre><font size=\"2\">None</td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"
			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"
			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"));
		}
		nBytesSent += req_format_write(wp, ("</table>")); 
	}
#endif
	} 
	return nBytesSent;
}


#ifdef CONFIG_RTK_MESH
/////////////////////////////////////////////////////////////////////////////
void formWlMesh(request *wp, char *path, char *query)
{
	char *strAddMac, *strDelMac, *strDelAllMac, *strVal, *submitUrl, *strEnabled;
	char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};
	int entryNum, i, enabled;
	WDS_T macEntry;

	strAddMac = req_get_cstream_var(wp, ("addWdsMac"), "");
	strDelMac = req_get_cstream_var(wp, ("deleteSelWdsMac"), "");
	strDelAllMac = req_get_cstream_var(wp, ("deleteAllWdsMac"), "");
	strEnabled = req_get_cstream_var(wp, ("wlanWdsEnabled"), "");

	if (strAddMac[0]) {
		if ( !strcmp(strEnabled, "ON"))
			enabled = 1;
		else
			enabled = 0;
		if ( apmib_set( MIB_WLAN_WDS_ENABLED, (void *)&enabled) == 0) {
  			strcpy(tmpBuf, ("Set enabled flag error!"));
			goto setErr_wds;
		}

		strVal = req_get_cstream_var(wp, ("mac"), "");
		if ( !strVal[0] )
			goto setWds_ret;

		if (strlen(strVal)!=12 || !string_to_hex(strVal, macEntry.macAddr, 12)) {
			strcpy(tmpBuf, ("Error! Invalid MAC address."));
			goto setErr_wds;
		}

		strVal = req_get_cstream_var(wp, ("comment"), "");
		if ( strVal[0] ) {
			if (strlen(strVal) > COMMENT_LEN-1) {
				strcpy(tmpBuf, ("Error! Comment length too long."));
				goto setErr_wds;
			}
			strcpy(macEntry.comment, strVal);
		}
		else
			macEntry.comment[0] = '\0';

		if ( !apmib_get(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto setErr_wds;
		}
		if ( (entryNum + 1) > MAX_WDS_NUM) {
			strcpy(tmpBuf, ("Cannot add new entry because table is full!"));
			goto setErr_wds;
		}

		// set to MIB. try to delete it first to avoid duplicate case
		apmib_set(MIB_WLAN_WDS_DEL, (void *)&macEntry);
		if ( apmib_set(MIB_WLAN_WDS_ADD, (void *)&macEntry) == 0) {
			strcpy(tmpBuf, ("Add table entry error!"));
			goto setErr_wds;
		}
	}

	/* Delete entry */
	if (strDelMac[0]) {
		if ( !apmib_get(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto setErr_wds;
		}
		for (i=entryNum; i>0; i--) {
			snprintf(tmpBuf, 20, "select%d", i);

			strVal = req_get_cstream_var(wp, tmpBuf, "");
			if ( !strcmp(strVal, "ON") ) {

				*((char *)&macEntry) = (char)i;
				if ( !apmib_get(MIB_WLAN_WDS, (void *)&macEntry)) {
					strcpy(tmpBuf, ("Get table entry error!"));
					goto setErr_wds;
				}
				if ( !apmib_set(MIB_WLAN_WDS_DEL, (void *)&macEntry)) {
					strcpy(tmpBuf, ("Delete table entry error!"));
					goto setErr_wds;
				}
			}
		}
	}

	/* Delete all entry */
	if ( strDelAllMac[0]) {
		if ( !apmib_set(MIB_WLAN_WDS_DELALL, (void *)&macEntry)) {
			strcpy(tmpBuf, ("Delete all table error!"));
			goto setErr_wds;
		}
	}

setWds_ret:
	apmib_update(CURRENT_SETTING);

#ifndef NO_ACTION
	run_init_script("bridge");
#endif

	submitUrl = req_get_cstream_var(wp, ("submit-url"), "");   // hidden page
	OK_MSG( submitUrl );
  	return;

setErr_wds:
	ERR_MSG(tmpBuf);
}

#endif // CONFIG_RTK_MESH


/////////////////////////////////////////////////////////////////////////////
void formWlWds(request *wp, char *path, char *query)
{
	char *strRate, *strAddMac, *strDelMac, *strDelAllMac, *strVal, *submitUrl, *strEnabled;
	char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};
	int entryNum, i, enabled, val;
	WDS_T macEntry;

	int maxWDSNum;
	
#ifdef CONFIG_RTL8196B_GW_8M
	maxWDSNum = 4;
#else
	maxWDSNum = MAX_WDS_NUM;
#endif

	strAddMac = req_get_cstream_var(wp, ("addWdsMac"), "");
	strDelMac = req_get_cstream_var(wp, ("deleteSelWdsMac"), "");
	strDelAllMac = req_get_cstream_var(wp, ("deleteAllWdsMac"), "");
	strEnabled = req_get_cstream_var(wp, ("wlanWdsEnabled"), "");

	if (strAddMac[0]) {
		if ( !strcmp(strEnabled, "ON"))
			enabled = 1;
		else
			enabled = 0;
		if ( apmib_set( MIB_WLAN_WDS_ENABLED, (void *)&enabled) == 0) {
  			strcpy(tmpBuf, ("Set enabled flag error!"));
			goto setErr_wds;
		}

		strVal = req_get_cstream_var(wp, ("mac"), "");
		if ( !strVal[0] )
			goto setWds_ret;

		if (strlen(strVal)!=12 || !string_to_hex(strVal, macEntry.macAddr, 12)) {
			strcpy(tmpBuf, ("Error! Invalid MAC address."));
			goto setErr_wds;
		}

		strVal = req_get_cstream_var(wp, ("comment"), "");
		if ( strVal[0] ) {
			if (strlen(strVal) > COMMENT_LEN-1) {
				strcpy(tmpBuf, ("Error! Comment length too long."));
				goto setErr_wds;
			}
			strcpy( (char *)macEntry.comment, strVal);
		}
		else
			macEntry.comment[0] = '\0';


		
		strRate = req_get_cstream_var(wp, "txRate", "");
		if ( strRate[0] ) {
			if ( strRate[0] == '0' ) { // auto
				macEntry.fixedTxRate =0;
			}else  {
				val = atoi(strRate);
				val = 1 << (val-1);
				macEntry.fixedTxRate = val;
			}
		}
	

		
		if ( !apmib_get(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto setErr_wds;
		}
		if ( (entryNum + 1) > maxWDSNum) {
			strcpy(tmpBuf, ("Cannot add new entry because table is full!"));
			goto setErr_wds;
		}

		// set to MIB. try to delete it first to avoid duplicate case
		apmib_set(MIB_WLAN_WDS_DEL, (void *)&macEntry);
		if ( apmib_set(MIB_WLAN_WDS_ADD, (void *)&macEntry) == 0) {
			strcpy(tmpBuf, ("Add table entry error!"));
			goto setErr_wds;
		}
	}

	/* Delete entry */
	if (strDelMac[0]) {
		if ( !apmib_get(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto setErr_wds;
		}
		for (i=entryNum; i>0; i--) {
			snprintf(tmpBuf, 20, "select%d", i);

			strVal = req_get_cstream_var(wp, tmpBuf, "");
			if ( !strcmp(strVal, "ON") ) {

				*((char *)&macEntry) = (char)i;
				if ( !apmib_get(MIB_WLAN_WDS, (void *)&macEntry)) {
					strcpy(tmpBuf, ("Get table entry error!"));
					goto setErr_wds;
				}
				if ( !apmib_set(MIB_WLAN_WDS_DEL, (void *)&macEntry)) {
					strcpy(tmpBuf, ("Delete table entry error!"));
					goto setErr_wds;
				}
			}
		}
	}

	/* Delete all entry */
	if ( strDelAllMac[0]) {
		if ( !apmib_set(MIB_WLAN_WDS_DELALL, (void *)&macEntry)) {
			strcpy(tmpBuf, ("Delete all table error!"));
			goto setErr_wds;
		}
	}

setWds_ret:
	apmib_update_web(CURRENT_SETTING);

#ifndef NO_ACTION
	run_init_script("bridge");
#endif

	submitUrl = req_get_cstream_var(wp, ("submit-url"), "");   // hidden page
	OK_MSG( submitUrl );
  	return;

setErr_wds:
	ERR_MSG(tmpBuf);
}

#if defined(WLAN_PROFILE)
int wlProfileTblList(request *wp, int argc, char **argv)
{
	int nBytesSent=0, entryNum, i;
	WLAN_PROFILE_T entry;
	char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};
	int profile_num_id, profile_tbl_id;

	if(wlan_idx == 0)
	{
		profile_num_id = MIB_PROFILE_NUM1;
		profile_tbl_id = MIB_PROFILE_TBL1;
	}
	else
	{
		profile_num_id = MIB_PROFILE_NUM2;
		profile_tbl_id = MIB_PROFILE_TBL2;
	}
	
	apmib_get(profile_num_id, (void *)&entryNum);

	nBytesSent += req_format_write(wp, ("<tr>"
      	"<td align=center width=\"\" bgcolor=\"#808080\"><font size=\"2\"><b>SSID</b></font></td>\n"
      	"<td align=center width=\"\" bgcolor=\"#808080\"><font size=\"2\"><b>Encrypt</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td></tr>\n"));

	for (i=1; i<=entryNum; i++) {
		unsigned char wlSecurityBuf[43]={0};
		
		*((char *)&entry) = (char)i;
		if ( !apmib_get(profile_tbl_id, (void *)&entry))
			return -1;

		if(entry.encryption == WEP64 || entry.encryption == WEP128)
		{
			sprintf(wlSecurityBuf, "%s", "WEP");
		}
		else if(entry.encryption == 3 ) //WPA
		{
			sprintf(wlSecurityBuf, "%s", "WPA/");
			if(entry.wpa_cipher == 2)
				strcat(wlSecurityBuf, "TKIP");
			else
				strcat(wlSecurityBuf, "AES");
		}
		else if(entry.encryption == 4 ) //WPA2
		{
			sprintf(wlSecurityBuf, "%s", "WPA2/");
			if(entry.wpa_cipher == 2)
				strcat(wlSecurityBuf, "TKIP");
			else
				strcat(wlSecurityBuf, "AES");
		}
		else
			sprintf(wlSecurityBuf, "%s", "OPEN");

		
		nBytesSent += req_format_write(wp, ("<tr>"
      			"<td align=center width=\"35%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
       			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n"),
				entry.ssid,wlSecurityBuf, i);
	}
	return nBytesSent;
}

int wlProfileList(request *wp, int argc, char **argv)
{
	int nBytesSent=0, entryNum, i;
	WLAN_PROFILE_T entry;
	int profile_num_id, profile_tbl_id, profile_enabled_id, wlProfileEnabled;
	
	if(wlan_idx == 0)
	{
		profile_num_id = MIB_PROFILE_NUM1;
		profile_tbl_id = MIB_PROFILE_TBL1;
		profile_enabled_id = MIB_PROFILE_ENABLED1;
	}
	else
	{
		profile_num_id = MIB_PROFILE_NUM2;
		profile_tbl_id = MIB_PROFILE_TBL2;
		profile_enabled_id = MIB_PROFILE_ENABLED2;
	}

	apmib_get( profile_enabled_id, (void *)&wlProfileEnabled);
	if(wlProfileEnabled == 0)
	{
		req_format_write(wp,"%s","//wireless profile disabled");
		return 0;
	}
	
	apmib_get(profile_num_id, (void *)&entryNum);


	for (i=1; i<=entryNum; i++) {		
		unsigned char encryptBuf[43]={0};
		unsigned char wepBuf[43]={0};		
		int wpaCipherVal = 0;
		
		sprintf(wepBuf, "%d", 0); 
		//Ssid|Encrypt|Authtype|wep|Wepkeytype|wpaCipher|wpa2Cipher|Pskformat

		memset(&entry, 0x00, sizeof(WLAN_PROFILE_T));
		*((char *)&entry) = (char)i;
		if ( !apmib_get(profile_tbl_id, (void *)&entry))
			return -1;

		if(entry.encryption == WEP64 || entry.encryption == WEP128)
		{
			sprintf(encryptBuf, "%d", 1);
			if(entry.encryption == WEP64)
				sprintf(wepBuf, "%d", 1);
			else
				sprintf(wepBuf, "%d", 2); //128
		}
		else if(entry.encryption == 3 ) //WPA
		{
			sprintf(encryptBuf, "%d", 2);
		}
		else if(entry.encryption == 4 ) //WPA2
		{
			sprintf(encryptBuf, "%d", 4);
		}
		else
		{
			sprintf(encryptBuf, "%d", 0);
		}

		if(entry.wpa_cipher == 2)
			wpaCipherVal = 1;
		else
			wpaCipherVal = 2;
		
		//Ssid|Encrypt|Authtype|wep|Wepkeytype|wpaCipher|wpa2Cipher|Pskformat|Wpapsk
		nBytesSent += req_format_write(wp, ("token[%d] =\'%s|%s|%d|%s|%d|%d|%d|%d|%s\';\n"),(i-1), entry.ssid,encryptBuf, entry.auth, wepBuf, entry.wepKeyType, wpaCipherVal, wpaCipherVal, entry.wpaPSKFormat, entry.wpaPSK );
	}
	return nBytesSent;

}
#endif //#if defined(WLAN_PROFILE)
/////////////////////////////////////////////////////////////////////////////
int wlWdsList(request *wp, int argc, char **argv)
{
	int nBytesSent=0, entryNum, i;
	WDS_T entry;
	char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};
	char txrate[20];
	int rateid=0;

	if ( !apmib_get(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
  		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}

	nBytesSent += req_format_write(wp, ("<tr>"
      	"<td align=center width=\"35%%\" bgcolor=\"#808080\"><font size=\"2\"><b>MAC Address</b></font></td>\n"
      	"<td align=center width=\"25%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Tx Rate (Mbps)</b></font></td>\n"
      	"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Comment</b></font></td>\n"
      	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td></tr>\n"));

	for (i=1; i<=entryNum; i++) {
		*((char *)&entry) = (char)i;
		if ( !apmib_get(MIB_WLAN_WDS, (void *)&entry))
			return -1;

		snprintf(tmpBuf, MAX_MSG_BUFFER_SIZE, ("%02x:%02x:%02x:%02x:%02x:%02x"),
			entry.macAddr[0], entry.macAddr[1], entry.macAddr[2],
			entry.macAddr[3], entry.macAddr[4], entry.macAddr[5]);

		if(entry.fixedTxRate == 0){	
				sprintf(txrate, "%s","Auto"); 
		}else{
			for(rateid=0; rateid<28;rateid++){
				if(tx_fixed_rate[rateid].id == entry.fixedTxRate){
					sprintf(txrate, "%s", tx_fixed_rate[rateid].rate);
					break;
				}
			}
		}	
		nBytesSent += req_format_write(wp, ("<tr>"
      			"<td align=center width=\"35%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
       			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n"),
				tmpBuf, txrate, entry.comment,i);
	}
	return nBytesSent;
}


/////////////////////////////////////////////////////////////////////////////
void formWdsEncrypt(request *wp, char *path, char *query)
{
   	char *strVal, *submitUrl;
	char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};
	WDS_ENCRYPT_T encrypt;
	int intVal, keyLen=0, oldFormat, oldPskLen, len, i;
	char charArray[16]={'0' ,'1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
	char key[100];
	char varName[20];

	sprintf(varName, "encrypt%d", wlan_idx);
	strVal = req_get_cstream_var(wp, varName, "");
	if (strVal[0]) {
		encrypt = strVal[0] - '0';
		if (encrypt != WDS_ENCRYPT_DISABLED && encrypt != WDS_ENCRYPT_WEP64 &&
			encrypt != WDS_ENCRYPT_WEP128 && encrypt != WDS_ENCRYPT_TKIP &&
				encrypt != WDS_ENCRYPT_AES) {
 			strcpy(tmpBuf, ("encrypt value not validt!"));
			goto setErr_wdsEncrypt;
		}
		apmib_set( MIB_WLAN_WDS_ENCRYPT, (void *)&encrypt);
	}
	else
		apmib_get( MIB_WLAN_WDS_ENCRYPT, (void *)&encrypt);

	if (encrypt == WDS_ENCRYPT_WEP64 || encrypt == WDS_ENCRYPT_WEP128) {
		sprintf(varName, "format%d", wlan_idx);
		strVal = req_get_cstream_var(wp, varName, "");
		if (strVal[0]) {
			if (strVal[0]!='0' && strVal[0]!='1') {
				strcpy(tmpBuf, ("Invalid wep key format value!"));
				goto setErr_wdsEncrypt;
		}
			intVal = strVal[0] - '0';
			apmib_set( MIB_WLAN_WDS_WEP_FORMAT, (void *)&intVal);
		}
		else
			apmib_get( MIB_WLAN_WDS_WEP_FORMAT, (void *)&intVal);

		if (encrypt == WDS_ENCRYPT_WEP64)
			keyLen = WEP64_KEY_LEN;
		else if (encrypt == WDS_ENCRYPT_WEP128)
			keyLen = WEP128_KEY_LEN;

		if (intVal == 1) // hex
			keyLen <<= 1;

		sprintf(varName, "wepKey%d", wlan_idx);
		strVal = req_get_cstream_var(wp, varName, "");
		if  (strVal[0]) {
			if (strlen(strVal) != keyLen) {
				strcpy(tmpBuf, ("Invalid wep key length!"));
				goto setErr_wdsEncrypt;
		}
			if ( !isAllStar(strVal) ) {
				if (intVal == 0) { // ascii
					for (i=0; i<keyLen; i++) {
						key[i*2] = charArray[(strVal[i]>>4)&0xf];
						key[i*2+1] = charArray[strVal[i]&0xf];
				}
					key[i*2] = '\0';
			}
				else  // hex
					strcpy(key, strVal);
				apmib_set( MIB_WLAN_WDS_WEP_KEY, (void *)key);
			}
		}
	}
	if (encrypt == WDS_ENCRYPT_TKIP || encrypt == WDS_ENCRYPT_AES) {
		sprintf(varName, "pskFormat%d", wlan_idx);
		strVal = req_get_cstream_var(wp, varName, "");
		if (strVal[0]) {
			if (strVal[0]!='0' && strVal[0]!='1') {
				strcpy(tmpBuf, ("Invalid wep key format value!"));
				goto setErr_wdsEncrypt;
				}
			intVal = strVal[0] - '0';
			}
			else
			apmib_get( MIB_WLAN_WDS_PSK_FORMAT, (void *)&intVal);


		// remember current psk format and length to compare to default case "****"
		apmib_get(MIB_WLAN_WDS_PSK_FORMAT, (void *)&oldFormat);
		apmib_get(MIB_WLAN_WDS_PSK, (void *)tmpBuf);
		oldPskLen = strlen(tmpBuf);

		sprintf(varName, "pskValue%d", wlan_idx);
		strVal = req_get_cstream_var(wp, varName, "");
		len = strlen(strVal);
		if (len > 0 && oldFormat == intVal && len == oldPskLen ) {
			for (i=0; i<len; i++) {
				if ( strVal[i] != '*' )
				break;
			}
			if (i == len)
				goto save_wdsEcrypt;
		}
		if (intVal==1) { // hex
			if (len!=MAX_PSK_LEN || !string_to_hex(strVal, tmpBuf, MAX_PSK_LEN)) {
				strcpy(tmpBuf, ("Error! invalid psk value."));
				goto setErr_wdsEncrypt;
	}
				}
		else { // passphras
			if (len==0 || len > (MAX_PSK_LEN-1) ) {
				strcpy(tmpBuf, ("Error! invalid psk value."));
				goto setErr_wdsEncrypt;
			}
		}
		apmib_set( MIB_WLAN_WDS_PSK_FORMAT, (void *)&intVal);
		apmib_set( MIB_WLAN_WDS_PSK, (void *)strVal);
	}

save_wdsEcrypt:
	intVal = 1;
	apmib_set( MIB_WLAN_WDS_ENABLED, (void *)&intVal);

	apmib_update_web(CURRENT_SETTING);

#ifndef NO_ACTION
	run_init_script("bridge");
#endif

	submitUrl = req_get_cstream_var(wp, ("submit-url"), "");   // hidden page
	OK_MSG(submitUrl);

	return;

setErr_wdsEncrypt:
	ERR_MSG(tmpBuf);
}


/////////////////////////////////////////////////////////////////////////////
int wdsList(request *wp, int argc, char **argv)
{
	int nBytesSent=0, i;
	WDS_INFO_Tp pInfo;
	char *buff;
	char txrate[20];
	int rateid=0;
	int short_gi=0;
	int channel_bandwidth=0;

	buff = calloc(1, sizeof(WDS_INFO_T)*MAX_STA_NUM);
	if ( buff == 0 ) {
		printf("Allocate buffer failed!\n");
		return 0;
	}

	if ( getWdsInfo(WLAN_IF, buff) < 0 ) {
		printf("Read wlan sta info failed!\n");
		return 0;
	}

	for (i=0; i<MAX_WDS_NUM; i++) {
		pInfo = (WDS_INFO_Tp)&buff[i*sizeof(WDS_INFO_T)];

		if (pInfo->state == STATE_WDS_EMPTY)
			break;

		if((pInfo->txOperaRate & 0x80) != 0x80){	
			if(pInfo->txOperaRate%2){
				sprintf(txrate, "%d%s",pInfo->txOperaRate/2, ".5"); 
			}else{
				sprintf(txrate, "%d",pInfo->txOperaRate/2); 
			}
		}else{
			apmib_get(MIB_WLAN_CHANNEL_BONDING, (void *)&channel_bandwidth);
			apmib_get(MIB_WLAN_SHORT_GI, (void *)&short_gi);
			if(channel_bandwidth ==0){ //20M
				if(short_gi==0){//long
					for(rateid=0; rateid<16;rateid++){
						if(rate_11n_table_20M_LONG[rateid].id == pInfo->txOperaRate){
							sprintf(txrate, "%s", rate_11n_table_20M_LONG[rateid].rate);
							break;
						}
					}
				}else if(short_gi==1){//short
					for(rateid=0; rateid<16;rateid++){
						if(rate_11n_table_20M_SHORT[rateid].id == pInfo->txOperaRate){
							sprintf(txrate, "%s", rate_11n_table_20M_SHORT[rateid].rate);
							break;
						}
					}
				}
			}else if(channel_bandwidth ==1){ //40
					if(short_gi==0){//long
						for(rateid=0; rateid<16;rateid++){
							if(rate_11n_table_40M_LONG[rateid].id == pInfo->txOperaRate){
								sprintf(txrate, "%s", rate_11n_table_40M_LONG[rateid].rate);
								break;
							}
						}
					}else if(short_gi==1){//short
						for(rateid=0; rateid<16;rateid++){
							if(rate_11n_table_40M_SHORT[rateid].id == pInfo->txOperaRate){
								sprintf(txrate, "%s", rate_11n_table_40M_SHORT[rateid].rate);
								break;
							}
						}
					}	
			}
		}	
		nBytesSent += req_format_write(wp,
	   		"<tr bgcolor=#b7b7b7><td><font size=2>%02x:%02x:%02x:%02x:%02x:%02x</td>"
			"<td><font size=2>%d</td>"
	     		"<td><font size=2>%d</td>"
			"<td><font size=2>%d</td>"
			"<td><font size=2>%s</td>",
			pInfo->addr[0],pInfo->addr[1],pInfo->addr[2],pInfo->addr[3],pInfo->addr[4],pInfo->addr[5],
			pInfo->tx_packets, pInfo->tx_errors, pInfo->rx_packets,
			txrate);
	}

	free(buff);

	return nBytesSent;
}

#ifdef WLAN_EASY_CONFIG
/////////////////////////////////////////////////////////////////////////////
void sigHandler_autoconf(int signo)
{
	int val, reinit=1;
	char tmpbuf[MAX_MSG_BUFFER_SIZE]={0};	
	
	apmib_get( MIB_WLAN_MODE, (void *)&val);	
	if (val == AP_MODE || val == AP_WDS_MODE) {	
		apmib_get(MIB_WLAN_EASYCFG_KEY, (void *)tmpbuf);	
		if (strlen(tmpbuf) > 0) // key already installed
			reinit = 0;		
	}

#ifdef WIFI_SIMPLE_CONFIG	
{
	#define REINIT_WEB_FILE		("/tmp/reinit_web")
	struct stat status;

	if (stat(REINIT_WEB_FILE, &status) == 0) { // file existed
        unlink(REINIT_WEB_FILE);
		reinit = 0;		
	}
}
#endif
	
	if (reinit) { // re-init system
		wait_config = 1;
#ifdef REBOOT_CHECK
	run_init_script_flag = 1;
#endif		
#ifndef NO_ACTION
		run_init_script("all");
#endif		
		wait_config = CONFIG_SUCCESS;
	}
	if (apmib_reinit() == 0) 
		printf(("Re-initialize AP MIB failed!\n"));	
}

/////////////////////////////////////////////////////////////////////////////
void formAutoCfg(request *wp, char *path, char *query)
{
   	char *strVal, *submitUrl;
 	int isButtonPress=0, isSave=0, isDelete=0, isDoConfigButton=0, isDoConfigQuestion=0;
	int mode, val, isAP, mode_old, enable, enable_old, wlan_disabled, i, isAdhoc, first=0;
	char tmpBuf[200], wlan_interface_set[100]={0}, hashBuf[33];
	
	strVal = req_get_cstream_var(wp, ("cfgEnabled"), "");
	if ( !strcmp(strVal, "ON"))
		enable = 1;
	else
		enable = 0;

	apmib_get( MIB_WLAN_EASYCFG_ENABLED, (void *)&enable_old);
	apmib_get( MIB_WLAN_WLAN_DISABLED, (void *)&wlan_disabled);

	strVal = req_get_cstream_var(wp, ("buttonClicked"), "");
	if (strVal[0])
		isButtonPress = 1;
	else {
		strVal = req_get_cstream_var(wp, ("save"), "");
		if (strVal[0])
			isSave = 1;
		else {
			strVal = req_get_cstream_var(wp, ("deletekey"), "");
			if (strVal[0])
				isDelete = 1;
			else {
				strVal = req_get_cstream_var(wp, ("doConfigButton"), "");
				if (strVal[0])
					isDoConfigButton = 1;
				else {
					strVal = req_get_cstream_var(wp, ("doConfigQuestion"), "");
					if (strVal[0])
						isDoConfigQuestion = 1;		
					else {
						strcpy(tmpBuf, ("Error, no action is defined!"));
						goto setErr_autocfg;
					}
				}
			}
		}
	}

	apmib_get( MIB_WLAN_MODE, (void *)&val);
	if (val == AP_MODE || val == AP_WDS_MODE) 
		isAP = 1;
	else 
		isAP = 0;

	apmib_set( MIB_WLAN_EASYCFG_WLAN_MODE, (void *)&val);

	apmib_get( MIB_WLAN_EASYCFG_MODE, (void *)&mode_old);
	
	if (isAP && isDoConfigQuestion) {
		strcpy(tmpBuf, ("Error, invalid action request!"));
		goto setErr_autocfg;
	}

	if (!isAP && isButtonPress ) {
		strcpy(tmpBuf, ("Error, invalid action request!"));
		goto setErr_autocfg;
	}

	for (i=0; i<wlan_num; i++) {
		sprintf(tmpBuf, "wlan%d ", i);
		strcat(wlan_interface_set, tmpBuf);
	}

	strVal = req_get_cstream_var(wp, ("mode"), "");
	if ( strVal[0] ) {
		if (strVal[0]!= '1' && strVal[0]!= '2' && strVal[0]!= '3'
#ifdef CONFIG_RTL_P2P_SUPPORT
		&&  strVal[0]!= '8'
#endif
		) {
  			strcpy(tmpBuf, ("Invalid mode value!"));
			goto setErr_autocfg;
		}
		mode = strVal[0] - '0';
	}
	else
		mode = mode_old;

	if (enable != enable_old) {
		int modify=0, aval, cipher;
		unsigned char tmp1[100], tmp2[100];
		
		apmib_set( MIB_WLAN_EASYCFG_ENABLED, (void *)&enable);
		
		apmib_get( MIB_WLAN_EASYCFG_KEY, (void *)&tmpBuf);	
		if (enable && strlen(tmpBuf) > 0) { /* key installed */
			/* see if current setting diff with AUTOCFG value. */
			/* if modify, flush AUTOCFG value */
			apmib_get( MIB_WLAN_WPA_AUTH, (void *)&val);			
			if (val != WPA_AUTH_PSK) 
				modify = 1;
		
			apmib_get( MIB_WLAN_EASYCFG_SSID, (void *)&tmp1);
			apmib_get( MIB_WLAN_SSID, (void *)&tmp2);		
			if ( strcmp(tmp1, tmp2))
				modify = 1;			
		
			if (!modify ) {		
				apmib_get( MIB_WLAN_ENCRYPT, (void *)&val);
				apmib_get( MIB_WLAN_EASYCFG_ALG_REQ, (void *)&aval);	
				apmib_get( MIB_WLAN_WPA_CIPHER_SUITE, (void *)&cipher);
				
				if ( !(val & ENCRYPT_WPA) && !(val & ENCRYPT_WPA2)) 
					modify = 1;		
				if (val & ENCRYPT_WPA) {
					if ((aval & ACF_ALGORITHM_WPA_TKIP) && !(cipher & WPA_CIPHER_TKIP))
						modify = 1;
					if ((aval & ACF_ALGORITHM_WPA_AES) && !(cipher & WPA_CIPHER_AES))
						modify = 1;					
				}
			 	if (val & ENCRYPT_WPA2) {
					if ((aval & ACF_ALGORITHM_WPA2_TKIP) && !(cipher & WPA_CIPHER_TKIP))
						modify = 1;
					if ((aval & ACF_ALGORITHM_WPA2_AES) && !(cipher & WPA_CIPHER_AES))
						modify = 1;					
				}
			}		
			if (!modify) {
				apmib_get( MIB_WLAN_EASYCFG_ROLE, (void *)&val);	
				if (isAP) {
					if (val != ROLE_SERVER)
						modify = 1;
				}
				else {					
					apmib_get( MIB_WLAN_NETWORK_TYPE, (void *)&isAdhoc);
					if (val == ROLE_SERVER)
						modify = 1;
					else {
						if ((val == ROLE_CLIENT) && isAdhoc)
							modify = 1;
						else if ((val == ROLE_ADHOC) && !isAdhoc)
							modify = 1;
					}
				}				
			}			
			
			if (modify) {
				tmpBuf[0] = '\0';
				apmib_set(MIB_WLAN_EASYCFG_KEY, (void *)tmpBuf);
				apmib_set(MIB_WLAN_EASYCFG_DIGEST, (void *)tmpBuf);	
			}
		}
	}

	if ((isDoConfigButton || isDoConfigQuestion) && !wlan_disabled) {
		if ((mode & MODE_QUESTION) && isDoConfigQuestion ) {
			MD5_CONTEXT md5ctx;
			unsigned char hash[16];
			int i;
			const char *hex = "0123456789abcdef";
			char *r;

			strVal = req_get_cstream_var(wp, ("firstCfg"), "");
			if ( !strcmp(strVal, "ON"))
				first = 1;
			else
				first = 0;

			tmpBuf[0]='\0';
			strVal = req_get_cstream_var(wp, ("q1"), "");
			strcat(tmpBuf, strVal);
			strVal = req_get_cstream_var(wp, ("q1ans"), "");
			strcat(tmpBuf, strVal);

			strVal = req_get_cstream_var(wp, ("q2"), "");
			strcat(tmpBuf, strVal);
			strVal = req_get_cstream_var(wp, ("q2ans"), "");
			strcat(tmpBuf, strVal);

			MD5Init(&md5ctx);
			MD5Update(&md5ctx, tmpBuf, (unsigned int)strlen(tmpBuf));
			MD5Final(hash, &md5ctx);

			/*
 			 *  Prepare the resulting hash string
 			 */
   			for (i = 0, r = hashBuf; i < 16; i++) {
             		*r++ = toupper(hex[hash[i] >> 4]);
               		*r++ = toupper(hex[hash[i] & 0xF]);
   			}
			*r = '\0';
			apmib_get( MIB_WLAN_NETWORK_TYPE, (void *)&isAdhoc);
#if 0			
			if (!isAP && isAdhoc) {
				char tmpBuf1[100];
				apmib_get( MIB_WLAN_EASYCFG_KEY, (void *)tmpBuf1);
				apmib_get( MIB_WLAN_EASYCFG_DIGEST, (void *)tmpBuf);
				if (tmpBuf1[0] && tmpBuf[0] && strcmp(hashBuf, tmpBuf)) {
		  			strcpy(tmpBuf, ("The question selection or answer of Q&A mode is not matched with installed value!"));
					goto setErr_autocfg;					
				}				
			}
#endif			
		}
	}
	
	if (mode != mode_old)
		apmib_set( MIB_WLAN_EASYCFG_MODE, (void *)&mode);

	if (isDelete) {
		tmpBuf[0] = '\0';
		apmib_set(MIB_WLAN_EASYCFG_KEY, (void *)tmpBuf);
		apmib_set(MIB_WLAN_EASYCFG_DIGEST, (void *)tmpBuf);
	}

	if (enable != enable_old || mode != mode_old || isDelete) {
		apmib_update_web(CURRENT_SETTING);
#ifndef NO_ACTION
		if (!wlan_disabled) {	
			sprintf(tmpBuf, "%s/%s start %s %s", _CONFIG_SCRIPT_PATH, 
				_WLAN_APP_SCRIPT_PROG, wlan_interface_set, BRIDGE_IF);
			system( tmpBuf );			
			sleep(2);
		}
#endif		
	}
	
#ifndef NO_ACTION	
	if (isButtonPress && !wlan_disabled) {
		sprintf(tmpBuf, "%s/%s -w wlan%d -press_button", _CONFIG_SCRIPT_PATH, 
			_AUTO_CONFIG_DAEMON_PROG, wlan_idx);
		system( tmpBuf );
	}

	if ((isDoConfigButton || isDoConfigQuestion) && !wlan_disabled) {
		if ((mode & MODE_QUESTION) && isDoConfigQuestion ) {
			sprintf(tmpBuf, "%s/%s start %s %s %s", _CONFIG_SCRIPT_PATH,
				_WLAN_APP_SCRIPT_PROG, wlan_interface_set, BRIDGE_IF, hashBuf);
			if (first)
				strcat(tmpBuf, " qfirst");
			system( tmpBuf );				
			sleep(2);
		}
		else {
			
			sprintf(tmpBuf, "%s/%s -w wlan%d -press_button", _CONFIG_SCRIPT_PATH, 
				_AUTO_CONFIG_DAEMON_PROG, wlan_idx);
			system( tmpBuf );
		}

		wait_config = 1;		
		while (wait_config <= DO_CONFIG_WAIT_TIME &&
					wait_config != CONFIG_SUCCESS) {
			wait_config++;						
			sleep(1);			
		}
		
		submitUrl = req_get_cstream_var(wp, ("submit-url"), "");   // hidden page
		
		if (wait_config == CONFIG_SUCCESS) {
			OK_MSG1(("Do Auto-Config successfully!"), submitUrl);
		}
		else {
			sprintf(tmpBuf, "%s/%s -w wlan%d -release_button", _CONFIG_SCRIPT_PATH, 
				_AUTO_CONFIG_DAEMON_PROG, wlan_idx);
			system( tmpBuf );
			
			OK_MSG1(("Do Auto-Config failed!"), submitUrl);			
			
			if (!isAP) {
				sprintf(tmpBuf, "%s/%s start %s %s", _CONFIG_SCRIPT_PATH,
					_WLAN_APP_SCRIPT_PROG, wlan_interface_set, BRIDGE_IF);		
				system( tmpBuf );	
			}						
		}
		
		wait_config = CONFIG_SUCCESS;		

		return;
	}
#endif

	submitUrl = req_get_cstream_var(wp, ("submit-url"), "");   // hidden page
	
	if (wlan_disabled && (isButtonPress || isDoConfigButton || isDoConfigQuestion)) {
		OK_MSG1(("The wireless interface is disabled, can't proceed the request!"), submitUrl);	
	}
	else {	
		if (isButtonPress) {
			OK_MSG1(("Waiting for client Auto-Config request..."), submitUrl);
		}
		else {		
			OK_MSG(submitUrl);
		}
	}
	return;

setErr_autocfg:
	ERR_MSG(tmpBuf);
}
#endif // WLAN_EASY_CONFIG


#ifdef WIFI_SIMPLE_CONFIG
#ifndef WLAN_EASY_CONFIG
void sigHandler_autoconf(int signo)
{
	#define REINIT_WEB_FILE		("/tmp/reinit_web")
	struct stat status;
	int reinit=1;

	if (stat(REINIT_WEB_FILE, &status) == 0) { // file existed
        unlink(REINIT_WEB_FILE);
		reinit = 0;		
	}	
	if (reinit) { // re-init system
#ifdef REBOOT_CHECK
	run_init_script_flag = 1;
#endif		
#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT)
	Start_Domain_Query_Process=0;
#endif
#ifndef NO_ACTION
		run_init_script("all");
#endif		
	}
	apmib_reinit();
}
#endif //!WLAN_EASY_CONFIG

static void update_wps_configured(int reset_flag)
{
	int is_configured, encrypt1, encrypt2, auth, disabled, iVal, format, shared_type;
	char ssid1[100];
	unsigned char tmpbuf[MAX_MSG_BUFFER_SIZE]={0};	
	
	if (wps_config_info.caller_id == CALLED_FROM_WLANHANDLER) {
		apmib_get(MIB_WLAN_SSID, (void *)ssid1);
		apmib_get(MIB_WLAN_MODE, (void *)&iVal);
		if (strcmp(ssid1, (char *)wps_config_info.ssid) || (iVal != wps_config_info.wlan_mode)) {
			apmib_set(MIB_WLAN_WSC_SSID, (void *)ssid1);
			goto configuration_changed;
		}

		return;
	}
	else if (wps_config_info.caller_id == CALLED_FROM_ADVANCEHANDLER) {
		apmib_get(MIB_WLAN_AUTH_TYPE, (void *)&shared_type);
		apmib_get(MIB_WLAN_ENCRYPT, (void *)&encrypt1);
		if (encrypt1 == ENCRYPT_WEP && 
			shared_type != wps_config_info.shared_type) {
			if (shared_type == AUTH_OPEN || shared_type == AUTH_BOTH) {
				if (wps_config_info.shared_type == AUTH_SHARED) {
					auth = WSC_AUTH_OPEN;
					apmib_set(MIB_WLAN_WSC_AUTH, (void *)&auth);
					goto configuration_changed;
				}
			}
			else {
				if (wps_config_info.shared_type == AUTH_OPEN ||
					wps_config_info.shared_type == AUTH_BOTH) {
					auth = WSC_AUTH_SHARED;
					apmib_set(MIB_WLAN_WSC_AUTH, (void *)&auth);
					goto configuration_changed;
				}
			}
		}

		return;
	}

	apmib_get(MIB_WLAN_ENCRYPT, (void *)&encrypt1);
	if (encrypt1 == ENCRYPT_DISABLED) {
		auth = WSC_AUTH_OPEN;
		encrypt2 = WSC_ENCRYPT_NONE;
	}
	else if (encrypt1 == ENCRYPT_WEP) {
		apmib_get(MIB_WLAN_AUTH_TYPE, (void *)&shared_type);
		if (shared_type == AUTH_OPEN || shared_type == AUTH_BOTH)
			auth = WSC_AUTH_OPEN;
		else
			auth = WSC_AUTH_SHARED;
		encrypt2 = WSC_ENCRYPT_WEP;		
	}
	else if (encrypt1 == ENCRYPT_WPA) {
		auth = WSC_AUTH_WPAPSK;
		apmib_get(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&encrypt1);
		if (encrypt1 == WPA_CIPHER_TKIP)
			encrypt2 = WSC_ENCRYPT_TKIP;		
		else if (encrypt1 == WPA_CIPHER_AES)
			encrypt2 = WSC_ENCRYPT_AES;		
		else 
			encrypt2 = WSC_ENCRYPT_TKIPAES;				
	}
	else if (encrypt1 == ENCRYPT_WPA2) {
		auth = WSC_AUTH_WPA2PSK;
		apmib_get(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&encrypt1);
		if (encrypt1 == WPA_CIPHER_TKIP)
			encrypt2 = WSC_ENCRYPT_TKIP;		
		else if (encrypt1 == WPA_CIPHER_AES)
			encrypt2 = WSC_ENCRYPT_AES;		
		else 
			encrypt2 = WSC_ENCRYPT_TKIPAES;				
	}
	else {
		auth = WSC_AUTH_WPA2PSKMIXED;
		encrypt2 = WSC_ENCRYPT_TKIPAES;			

// When mixed mode, if no WPA2-AES, try to use WPA-AES or WPA2-TKIP
		apmib_get(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&encrypt1);
		apmib_get(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&iVal);
		if (!(iVal &	WPA_CIPHER_AES)) {
			if (encrypt1 &	WPA_CIPHER_AES) {			
				//auth = WSC_AUTH_WPAPSK;
				encrypt2 = WSC_ENCRYPT_AES;	
				//printf("%s %d : %d\n",__FUNCTION__,__LINE__ ,encrypt2);
			}
			else{
				encrypt2 = WSC_ENCRYPT_TKIP;	
				//printf("%s %d : %d\n",__FUNCTION__,__LINE__ ,encrypt2);
			}
		}
//-------------------------------------------- david+2008-01-03
		if(encrypt1==WPA_CIPHER_AES && iVal ==WPA_CIPHER_AES){
			encrypt2 = WSC_ENCRYPT_AES;	
			printf("%s %d\n",__FUNCTION__,__LINE__);			
		}
		//printf("%s %d :auth=0x%02X\n",__FUNCTION__,__LINE__ ,auth);		
		// for correct wsc_auth wsc_encry value when security is mixed mode
	}
	apmib_set(MIB_WLAN_WSC_AUTH, (void *)&auth);
	apmib_set(MIB_WLAN_WSC_ENC, (void *)&encrypt2);

	apmib_get(MIB_WLAN_ENCRYPT, (void *)&encrypt1);
	if (encrypt1 == ENCRYPT_WPA || encrypt1 == ENCRYPT_WPA2 || encrypt1 == ENCRYPT_WPA2_MIXED) {
		apmib_get(MIB_WLAN_WPA_AUTH, (void *)&format);
		if (format & 2) { // PSK
			apmib_get(MIB_WLAN_WPA_PSK, (void *)tmpbuf);
			apmib_set(MIB_WLAN_WSC_PSK, (void *)tmpbuf);					
		}		
	}
	if (reset_flag) {
		reset_flag = 0;
		apmib_set(MIB_WLAN_WSC_CONFIGBYEXTREG, (void *)&reset_flag);		
	}	

	if (wps_config_info.caller_id == CALLED_FROM_WEPHANDLER) {
		apmib_get(MIB_WLAN_ENCRYPT, (void *)&auth);
		if (wps_config_info.auth != auth)
			goto configuration_changed;

		apmib_get(MIB_WLAN_WEP, (void *)&encrypt2);
		if (wps_config_info.wep_enc != encrypt2)
			goto configuration_changed;
		
		apmib_get(MIB_WLAN_WEP_DEFAULT_KEY, (void *)&iVal);
		if (wps_config_info.KeyId != iVal)
			goto configuration_changed;
		
		apmib_get(MIB_WLAN_WEP64_KEY1, (void *)tmpbuf);
		if (strcmp((char *)wps_config_info.wep64Key1, (char *)tmpbuf))
			goto configuration_changed;

		apmib_get(MIB_WLAN_WEP64_KEY2, (void *)tmpbuf);
		if (strcmp((char *)wps_config_info.wep64Key2, (char *)tmpbuf))
			goto configuration_changed;

		apmib_get(MIB_WLAN_WEP64_KEY3, (void *)tmpbuf);
		if (strcmp((char *)wps_config_info.wep64Key3, (char *)tmpbuf))
			goto configuration_changed;

		apmib_get(MIB_WLAN_WEP64_KEY4, (void *)tmpbuf);
		if (strcmp((char *)wps_config_info.wep64Key4, (char *)tmpbuf))
			goto configuration_changed;

		apmib_get(MIB_WLAN_WEP128_KEY1, (void *)tmpbuf);
		if (strcmp((char *)wps_config_info.wep128Key1, (char *)tmpbuf))
			goto configuration_changed;

		apmib_get(MIB_WLAN_WEP128_KEY2, (void *)tmpbuf);
		if (strcmp((char *)wps_config_info.wep128Key2, (char *)tmpbuf))
			goto configuration_changed;

		apmib_get(MIB_WLAN_WEP128_KEY3, (void *)tmpbuf);
		if (strcmp((char *)wps_config_info.wep128Key3,(char *)tmpbuf))
			goto configuration_changed;

		apmib_get(MIB_WLAN_WEP128_KEY4, (void *)tmpbuf);
		if (strcmp((char *)wps_config_info.wep128Key4, (char *)tmpbuf))
			goto configuration_changed;

		return;
	}
	else if (wps_config_info.caller_id == CALLED_FROM_WPAHANDLER) {
		apmib_get(MIB_WLAN_ENCRYPT, (void *)&auth);
		if (wps_config_info.auth != auth)
			goto configuration_changed;
		
		apmib_get(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&encrypt1);
		if (wps_config_info.wpa_enc != encrypt1)
			goto configuration_changed;
		
		apmib_get(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&encrypt2);
		if (wps_config_info.wpa2_enc != encrypt2)
			goto configuration_changed;
		
		apmib_get(MIB_WLAN_WPA_PSK, (void *)tmpbuf);
		if (strcmp((char *)wps_config_info.wpaPSK, (char *)tmpbuf))
			goto configuration_changed;

		return;
	}
	else
		return;
	
configuration_changed:	
	reset_flag = 0;
	apmib_set(MIB_WLAN_WSC_CONFIGBYEXTREG, (void *)&reset_flag);
	apmib_get(MIB_WLAN_WSC_DISABLE, (void *)&disabled);	
	apmib_get(MIB_WLAN_WSC_CONFIGURED, (void *)&is_configured);
	//if (!is_configured && !disabled) { //We do not care wsc is enable for disable--20081223
	if (!is_configured) {
		is_configured = 1;
		apmib_set(MIB_WLAN_WSC_CONFIGURED, (void *)&is_configured);
#if defined(CONFIG_RTL_92D_SUPPORT)
		if(wlan_idx==0){
			wlan_idx = 1;
			apmib_set(MIB_WLAN_WSC_CONFIGURED, (void *)&is_configured);
			wlan_idx = 0;			
		}else if(wlan_idx == 1){
			wlan_idx = 0;
			apmib_set(MIB_WLAN_WSC_CONFIGURED, (void *)&is_configured);
			wlan_idx = 1;			
		}
#endif
	}
}

#if 0
static void convert_hex_to_ascii(unsigned long code, char *out)
{
	*out++ = '0' + ((code / 10000000) % 10);  
	*out++ = '0' + ((code / 1000000) % 10);
	*out++ = '0' + ((code / 100000) % 10);
	*out++ = '0' + ((code / 10000) % 10);
	*out++ = '0' + ((code / 1000) % 10);
	*out++ = '0' + ((code / 100) % 10);
	*out++ = '0' + ((code / 10) % 10);
	*out++ = '0' + ((code / 1) % 10);
	*out = '\0';
}

static int compute_pin_checksum(unsigned long int PIN)
{
	unsigned long int accum = 0;
	int digit;
	
	PIN *= 10;
	accum += 3 * ((PIN / 10000000) % 10); 	
	accum += 1 * ((PIN / 1000000) % 10);
	accum += 3 * ((PIN / 100000) % 10);
	accum += 1 * ((PIN / 10000) % 10); 
	accum += 3 * ((PIN / 1000) % 10); 
	accum += 1 * ((PIN / 100) % 10); 
	accum += 3 * ((PIN / 10) % 10);

	digit = (accum % 10);
	return (10 - digit) % 10;
}
#endif

////////////////////////////////////////////////////////////////////////////////
void apmib_reset_wlan_to_default(unsigned char *wlanif_name)
{
	SetWlan_idx(wlanif_name);
	memcpy(&pMib->wlan[wlan_idx][vwlan_idx], &pMibDef->wlan[wlan_idx][vwlan_idx], sizeof(CONFIG_WLAN_SETTING_T));	
	if(strstr((char *)wlanif_name,"vxd") != 0)
	{
		if(wlan_idx == 0)
		{
			sprintf((char *)pMib->repeaterSSID1, (char *)pMib->wlan[wlan_idx][vwlan_idx].ssid);
			pMib->wlan[wlan_idx][vwlan_idx].wlanDisabled = !pMib->repeaterEnabled1;			
		}
		else
		{
			sprintf((char *)pMib->repeaterSSID2, (char *)pMib->wlan[wlan_idx][vwlan_idx].ssid);
			pMib->wlan[wlan_idx][vwlan_idx].wlanDisabled = !pMib->repeaterEnabled2;			
		}
	}
}

void updateVapWscDisable(int wlan_root,int value)
{
	int i=0;
	int wlanif_idx = 0;
	char ifname[20];
	
	for(i=0;i<(NUM_VWLAN_INTERFACE-1);i++) // vap0~vap3
	{
		memset(ifname,0x00,sizeof(ifname));
		sprintf(ifname,"wlan%d-va%d",wlan_root,i);
		SetWlan_idx(ifname);
		apmib_set(MIB_WLAN_WSC_DISABLE, (void *)&value);
	}
	memset(ifname,0x00,sizeof(ifname));
	sprintf(ifname,"wlan%d-vxd",wlan_root);
	SetWlan_idx(ifname);
	apmib_set(MIB_WLAN_WSC_DISABLE, (void *)&value);
	
	memset(ifname,0x00,sizeof(ifname));
	sprintf(ifname,"wlan%d",wlan_root);
	SetWlan_idx(ifname);
}

void formWsc(request *wp, char *path, char *query)
{
	char *strVal, *submitUrl, *strResetUnCfg, *wlanIf;
	char *targetAPSsid, *targetAPMac; /* WPS2DOTX */
	/*WPS2DOTX ;check if unlock button be clicked ; brute force attack */
	char  *unlockclicked;
	int intVal;
	char tmpbuf[200];
	int mode;
	int reset_to_unconfig_state_flag = 0;

	// 1104
	int tmpint;	
	char ifname[30];

	// WPS2DOTX ; 2011-0428
    int idx,idx2;
	char pincodestr_b[20];	
	// WPS2DOTX ; 2011-0428

#if defined(CONFIG_RTL_92D_SUPPORT)
	int wlanDisabled[2];
	int wlanMode[2];		
	int wlanPhyBand[2];
	int wlanMacPhy[2];
	int wlanif;
	int isSwapWlwanIf = 0;
	int wlan0_mode=1, wlan1_mode=1;
	int wlan0_disable=0, wlan1_disable=0;
	int wlan_orig;

	wlan_orig = wlan_idx;
	SetWlan_idx("wlan0");
	apmib_get(MIB_WLAN_MODE, &wlan0_mode);
	apmib_get(MIB_WLAN_WLAN_DISABLED, &wlan0_disable);
	SetWlan_idx("wlan1");
	apmib_get(MIB_WLAN_MODE, &wlan1_mode);
	apmib_get(MIB_WLAN_WLAN_DISABLED, &wlan1_disable);

	sprintf(tmpbuf, "wlan%d", wlan_idx);
	SetWlan_idx(tmpbuf);
	wlan_idx = wlan_orig;
#endif
	memset(ifname,'\0',30);
	
//displayPostDate(wp->post_data);	
	
	/* support  special MAC , 2011-0505 WPS2DOTX */
	
	targetAPMac = req_get_cstream_var(wp, "targetAPMac", (""));
	targetAPSsid = req_get_cstream_var(wp, "targetAPSsid", (""));
	/* support  special SSID , 2011-0505 WPS2DOTX */	
	submitUrl = req_get_cstream_var(wp, ("submit-url"), "");

	strResetUnCfg = req_get_cstream_var(wp, ("resetUnCfg"), "");
	if(strResetUnCfg[0] && strResetUnCfg[0]=='1')// reset to unconfig state. Keith
	{				
//#if defined(FOR_DUAL_BAND) //  ; both reset two unterface (wlan0 AND wlan1)
#if defined(CONFIG_RTL_92D_SUPPORT)
		wlanif = whichWlanIfIs(PHYBAND_5G);
		
		if(wlanif != 0)
		{
			swapWlanMibSetting(0,1);
			isSwapWlwanIf = 1;
		}
		wlanDisabled[0] = pMib->wlan[0][0].wlanDisabled;
		wlanDisabled[1] = pMib->wlan[1][0].wlanDisabled;
		wlanMode[0] = pMib->wlan[0][0].wlanMode;
		wlanMode[1] = pMib->wlan[1][0].wlanMode;
		wlanMacPhy[0] = pMib->wlan[0][0].macPhyMode;
		wlanMacPhy[1] = pMib->wlan[1][0].macPhyMode;
			
		printf("(%s,%d)Reset to OOB ...\n",__FUNCTION__ , __LINE__);
		if(wlanMode[0] != CLIENT_MODE)
		{
			apmib_reset_wlan_to_default("wlan0");
			pMib->wlan[0][0].wlanDisabled = wlanDisabled[0];
			pMib->wlan[0][0].macPhyMode = wlanMacPhy[0];
		}
		if(wlanMode[1] != CLIENT_MODE)
		{
			apmib_reset_wlan_to_default("wlan1");
			pMib->wlan[1][0].wlanDisabled = wlanDisabled[1];
			pMib->wlan[1][0].macPhyMode = wlanMacPhy[1];
		}
		
		if(isSwapWlwanIf == 1)
		{
			swapWlanMibSetting(0,1);
			isSwapWlwanIf = 0;
		}
		
#else
//		wlanIf = req_get_cstream_var(wp, ("wlanIf"), "");
//		if(wlanIf[0])
		apmib_reset_wlan_to_default((unsigned char *)"wlan0");
//		else
//			printf("Reset wlan to default fail!! No wlan name. %s,%d\n",__FUNCTION__ , __LINE__);
#endif
		
#ifdef REBOOT_CHECK
		strVal = req_get_cstream_var(wp, ("disableWPS"), "");
		if ( !strcmp(strVal, "ON"))
			intVal = 1;
		else
			intVal = 0;
		apmib_set(MIB_WLAN_WSC_DISABLE, (void *)&intVal);
		updateVapWscDisable(wlan_idx,intVal);
		REBOOT_WAIT(submitUrl);
		run_init_script_flag = 1;
#endif

		apmib_update_web(CURRENT_SETTING);
		
#ifndef NO_ACTION
		run_init_script("bridge");
#endif
		return;
	}
	
#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)
	strResetUnCfg = req_get_cstream_var(wp, ("resetRptUnCfg"), "");
	if(strResetUnCfg[0] && strResetUnCfg[0]=='1')// reset to unconfig state. Keith
	{
		wlanIf = req_get_cstream_var(wp, ("wlanIf"), "");
		if(wlanIf[0])
			apmib_reset_wlan_to_default((unsigned char *)wlanIf);
		else
			printf("Reset wlan to default fail!! No wlan name. %s,%d\n",__FUNCTION__ , __LINE__);		

#ifdef REBOOT_CHECK
		strVal = req_get_cstream_var(wp, ("disableWPS"), "");
		if ( !strcmp(strVal, "ON"))
			intVal = 1;
		else
			intVal = 0;
		apmib_set(MIB_WLAN_WSC_DISABLE, (void *)&intVal);
		updateVapWscDisable(wlan_idx, intVal);
		REBOOT_WAIT(submitUrl);
		run_init_script_flag = 1;
#endif		

		apmib_update_web(CURRENT_SETTING);
		
#ifndef NO_ACTION
		run_init_script("bridge");
#endif
		return;
	}
#endif //#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)


	/*WPS2DOTX ;check if unlock button be clicked ; brute force attack */
	unlockclicked = req_get_cstream_var(wp, ("unlockautolockdown"), "");
	if(unlockclicked[0])
	{
		sprintf(tmpbuf, "%s -sig_unlock wlan%d", _WSC_DAEMON_PROG,wlan_idx);
		printf("%s\n",tmpbuf);
		system(tmpbuf);	
		OK_MSG2(UNLOCK_MSG, ((mode==AP_MODE) ? "client" : "AP"), submitUrl);	
		return;
	}
	/*WPS2DOTX ;check if unlock button be clicked ; brute force attack */
	apmib_get(MIB_WLAN_MODE, (void *)&mode);	
	strVal = req_get_cstream_var(wp, ("triggerPBC"), "");
	if (strVal[0]) {
		apmib_get(MIB_WLAN_WSC_DISABLE, (void *)&intVal);
		if (intVal) {
			intVal = 0;
			apmib_set(MIB_WLAN_WSC_DISABLE, (void *)&intVal);
                        updateVapWscDisable(wlan_idx, intVal);
			apmib_update_web(CURRENT_SETTING);	// update to flash	
			system("echo 1 > /var/wps_start_pbc");
#ifndef NO_ACTION
			run_init_script("bridge");
#endif			
		}
		else {		
#ifndef NO_ACTION	
				sprintf(tmpbuf, "%s -sig_pbc wlan%d", _WSC_DAEMON_PROG,wlan_idx);
				system(tmpbuf);

#endif
		}
		OK_MSG2(START_PBC_MSG, ((mode==AP_MODE) ? "client" : "AP"), submitUrl);
		return;
	}
#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)	
	strVal = req_get_cstream_var(wp, ("triggerRptPBC"), "");
	if (strVal[0]) {
		apmib_get(MIB_WLAN_WSC_DISABLE, (void *)&intVal);
		if (intVal) {
			intVal = 0;
			apmib_set(MIB_WLAN_WSC_DISABLE, (void *)&intVal);
			updateVapWscDisable(wlan_idx, intVal);
			apmib_update_web(CURRENT_SETTING);	// update to flash	
			system("echo 1 > /var/wps_start_pbc");
#ifndef NO_ACTION
			run_init_script("bridge");
#endif			
		}
		else {		
#ifndef NO_ACTION		
			sprintf(tmpbuf, "%s -sig_pbc wlan%d-vxd", _WSC_DAEMON_PROG,wlan_idx);
			system(tmpbuf);
#endif
		}
		OK_MSG2(START_PBC_MSG, ((mode==AP_MODE) ? "client" : "AP"), submitUrl);
		return;
	}
#endif //#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)

/* support  special SSID , 2011-0505 WPS2DOTX */
	strVal = req_get_cstream_var(wp, ("stopwsc"), (""));
	if (strVal[0]) {
		system("echo 1 > /tmp/wscd_cancel");	
		OK_MSG2(STOP_MSG, ((mode==AP_MODE) ? "client" : "AP"), submitUrl);
		return;
	}
/* support  special SSID , 2011-0505 WPS2DOTX */

	strVal = req_get_cstream_var(wp, ("triggerPIN"), "");
	if (strVal[0]) {
		int local_pin_changed = 0;		
		strVal = req_get_cstream_var(wp, ("localPin"), "");
		if (strVal[0]) {
			apmib_get(MIB_HW_WSC_PIN, (void *)tmpbuf);
			if (strcmp(tmpbuf, strVal)) {
				apmib_set(MIB_HW_WSC_PIN, (void *)strVal);
				local_pin_changed = 1;				
			}			
		}		
		apmib_get(MIB_WLAN_WSC_DISABLE, (void *)&intVal);
		if (intVal) {
			char localpin[100];
			intVal = 0;			
			apmib_set(MIB_WLAN_WSC_DISABLE, (void *)&intVal);
			updateVapWscDisable(wlan_idx, intVal);
			apmib_update_web(CURRENT_SETTING);	// update to flash	
			system("echo 1 > /var/wps_start_pin");

#ifndef NO_ACTION
			if (local_pin_changed) {
				apmib_get(MIB_HW_WSC_PIN, (void *)localpin);
				sprintf(tmpbuf, "echo %s > /var/wps_local_pin", localpin);
				system(tmpbuf);
			}
			run_init_script("bridge");			
#endif			
		}
		else {		
#ifndef NO_ACTION		
			if (local_pin_changed) {
				system("echo 1 > /var/wps_start_pin");
				
				apmib_update_web(CURRENT_SETTING);					
				run_init_script("bridge");
			}
			else {

				/* support  special MAC , 2011-0505 ;WPS2DOTX*/
				if(targetAPMac[0]){
					unsigned char targetAPMacFilter[20];
					int idx = 0;
					int idx2 = 0;					
					//printf("before ,mac =%s len=%d \n",targetAPMac , strlen(targetAPMac));
					for(idx;idx<strlen(targetAPMac);idx++){
						if( _is_hex(targetAPMac[idx])){
							targetAPMacFilter[idx2]=targetAPMac[idx];
							idx2++;
						}
					}
					
					targetAPMacFilter[idx2]='\0';
					
					if(strlen(targetAPMacFilter)!=12){
						printf("invaild MAC Addr Len\n\n");
					}else{					
						sprintf(tmpbuf, "iwpriv wlan%d set_mib wsc_specmac=%s ",wlan_idx, targetAPMacFilter);
						//printf("tmpbuf=%s\n",tmpbuf);
						system(tmpbuf);						
					}											
					
				}
				if(targetAPSsid[0]){					
					if(strlen(targetAPSsid)<= 32){
						sprintf(tmpbuf, "iwpriv wlan%d set_mib wsc_specssid=\"%s\" ",wlan_idx, targetAPSsid);
						system(tmpbuf);						
					}else{					
						printf("invaild SSID Len\n");
					}											
					
				}				
				/* support  special SSID , 2011-0505 WPS2DOTX */
#if defined(FOR_DUAL_BAND)
				if( (wlan0_mode == 0) && (wlan1_mode == 0) && (wlan0_disable == 0) && (wlan1_disable == 0))
						sprintf(tmpbuf, "%s -sig_start %s", _WSC_DAEMON_PROG, "wlan0-wlan1");
				else 
						sprintf(tmpbuf, "%s -sig_start wlan%d", _WSC_DAEMON_PROG,wlan_idx);
				system(tmpbuf);
#else
				sprintf(tmpbuf, "%s -sig_start wlan%d", _WSC_DAEMON_PROG,wlan_idx);
				system(tmpbuf);
#endif
			}			
#endif
		}
		OK_MSG2(START_PIN_MSG, ((mode==AP_MODE) ? "client" : "AP"), submitUrl);
		return;
	}
	
#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)
	strVal = req_get_cstream_var(wp, ("triggerRptPIN"), "");

	if (strVal[0]) {
		int local_pin_changed = 0;		
		strVal = req_get_cstream_var(wp, ("localPin"), "");
		if (strVal[0]) {
			apmib_get(MIB_HW_WSC_PIN, (void *)tmpbuf);

			if (strcmp(tmpbuf, strVal)) {
				apmib_set(MIB_HW_WSC_PIN, (void *)strVal);
				local_pin_changed = 1;				
			}			
		}		
		apmib_get(MIB_WLAN_WSC_DISABLE, (void *)&intVal);
		if (intVal) {
			char localpin[100];
			intVal = 0;			
			apmib_set(MIB_WLAN_WSC_DISABLE, (void *)&intVal);
			apmib_update_web(CURRENT_SETTING);	// update to flash	
			system("echo 1 > /var/wps_start_pin");

#ifndef NO_ACTION
			if (local_pin_changed) {
				apmib_get(MIB_HW_WSC_PIN, (void *)localpin);
				sprintf(tmpbuf, "echo %s > /var/wps_local_pin", localpin);
				system(tmpbuf);
			}
			run_init_script("bridge");			
#endif			
		}
		else {		
#ifndef NO_ACTION		
			if (local_pin_changed) {
				system("echo 1 > /var/wps_start_pin");
				
				apmib_update_web(CURRENT_SETTING);					
				run_init_script("bridge");
			}
			else {
				sprintf(tmpbuf, "%s -sig_start wlan%d-vxd", _WSC_DAEMON_PROG,wlan_idx);
				system(tmpbuf);
			}			
#endif
		}
		OK_MSG2(START_PIN_MSG, ((mode==AP_MODE) ? "client" : "AP"), submitUrl);
		return;
	}
#endif //#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)		
	
	strVal = req_get_cstream_var(wp, ("setPIN"), "");
	if (strVal[0]) {		
		strVal = req_get_cstream_var(wp, ("peerPin"), "");
		if (strVal[0]) {
			apmib_get(MIB_WLAN_WSC_DISABLE, (void *)&intVal);
			if (intVal) {
				intVal = 0;
				apmib_set(MIB_WLAN_WSC_DISABLE, (void *)&intVal);
				apmib_update_web(CURRENT_SETTING);	

				sprintf(tmpbuf, "echo %s > /var/wps_peer_pin", strVal);
				system(tmpbuf);

#ifndef NO_ACTION
				run_init_script("bridge");
#endif					
			}
			else {			
#ifndef NO_ACTION
				// WPS2DOTX ; 2011-0428 ; support the format pin code 1234-5670 (include "-")
				memset(pincodestr_b,'\0',20);				
				//printf("before filter pin code =%s , len =%d\n", strVal ,strlen(strVal));
				idx2=0;
				for(idx=0 ; idx <strlen(strVal) ; idx++){
					//printf("strVal[%d]=%x\n",idx,strVal[idx]);	
					if(strVal[idx] >= '0' && strVal[idx]<= '9'){
						pincodestr_b[idx2]=strVal[idx];	
						idx2++;
					}
				}
				
				//printf("after filter pin code =%s , len =%d\n", pincodestr_b ,strlen(pincodestr_b));				
				sprintf(tmpbuf, "iwpriv %s set_mib pin=%s", WLAN_IF, pincodestr_b);				
				//printf("tmpbuf=%s\n",tmpbuf);
				system(tmpbuf);
				// WPS2DOTX ; 2011-0428 ; support the format pin code 1234-5670 (include "-")				
#endif
			}
			OK_MSG1(SET_PIN_MSG, submitUrl);			
			return;
		}
	}

#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)
	strVal = req_get_cstream_var(wp, ("setRptPIN"), "");
	if (strVal[0]) {		
		strVal = req_get_cstream_var(wp, ("peerRptPin"), "");
		if (strVal[0]) {
			apmib_get(MIB_WLAN_WSC_DISABLE, (void *)&intVal);
			if (intVal) {
				intVal = 0;
				apmib_set(MIB_WLAN_WSC_DISABLE, (void *)&intVal);
				apmib_update_web(CURRENT_SETTING);	

				sprintf(tmpbuf, "echo %s > /var/wps_peer_pin", strVal);
				system(tmpbuf);

#ifndef NO_ACTION
				run_init_script("bridge");
#endif					
			}
			else {			
#ifndef NO_ACTION
				sprintf(tmpbuf, "iwpriv wlan%d-vxd set_mib pin=%s", wlan_idx, strVal);
				system(tmpbuf);
#endif
			}
			OK_MSG1(SET_PIN_MSG, submitUrl);			
			return;
		}
	}
#endif //#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)

	strVal = req_get_cstream_var(wp, ("disableWPS"), "");
	if ( !strcmp(strVal, "ON"))
		intVal = 1;
	else
		intVal = 0;

	// 1104
	sprintf(ifname,"wlan%d",wlan_idx);
	SetWlan_idx(ifname);
	
	apmib_set(MIB_WLAN_WSC_DISABLE, (void *)&intVal);
	updateVapWscDisable(wlan_idx, intVal);

	strVal = req_get_cstream_var(wp, ("localPin"), "");
	if (strVal[0])
		apmib_set(MIB_HW_WSC_PIN, (void *)strVal);

//	update_wps_configured(0);
		
	apmib_update_web(CURRENT_SETTING);	// update to flash
	
#ifndef NO_ACTION
	run_init_script("bridge");
#endif

	OK_MSG(submitUrl);
}
////////////////////////////////////////////////////////////////////////
#endif // WIFI_SIMPLE_CONFIG


void formWlanRedirect(request *wp, char *path, char *query)
{
	char *redirectUrl;
	char *strWlanId;
	
        redirectUrl= req_get_cstream_var(wp, ("redirect-url"), "");   // hidden page
        strWlanId= req_get_cstream_var(wp, ("wlan_id"), "");   // hidden page
	if(strWlanId[0]){
		wlan_idx = atoi(strWlanId);
		sprintf(WLAN_IF, "wlan%d", wlan_idx);
	}
#ifdef MBSSID	
	mssid_idx = 0;
#endif
        if (redirectUrl[0])
                send_redirect_perm(wp,redirectUrl);
}

void formWlanRedirect2(request *wp, char *redirectUrl, char *strWlanId)
{
	if(strWlanId){
		wlan_idx = atoi(strWlanId);
		sprintf(WLAN_IF, "wlan%d", wlan_idx);
	}
#ifdef MBSSID	
	mssid_idx = 0;
#endif
        if (redirectUrl)
                send_redirect_perm(wp,redirectUrl);
}

#ifdef CONFIG_RTL_WAPI_SUPPORT
void formWapiReKey(request *wp, char * path, char * query)
{
	char *webpage, *strVal;
	char tmpBuf[200];
	int val;
	int mPolicy,policy;
	/*get Mcast Ucast*/
	webpage=req_get_cstream_var(wp,("next_webpage"),"");

	strVal=req_get_cstream_var(wp,("KEY_TYPE"),"");
//	printf("KEY_TYPE %s \n",strVal);
	strVal=req_get_cstream_var(wp,("MAC"),"");
//	printf("MAC %s \n",strVal);

	/*1: off  2: time 3: packet 4:time+packet*/
	strVal=req_get_cstream_var(wp,("REKEY_M_POLICY"),"");
	if(strVal)
	{
		mPolicy=strVal[0]-'0';
		if(!apmib_set(MIB_WLAN_WAPI_MCASTREKEY,(void *)&mPolicy))
		{
			strcpy(tmpBuf,"Can not set MCAST key policy!");
			goto setErr_rekey;

		}
//		printf("REKEY_M_POLICY %s \n",strVal);
	}
	
	strVal=req_get_cstream_var(wp,("REKEY_M_TIME"),"");
	if(strVal)
	{
		val=atoi(strVal);
		if(!apmib_set(MIB_WLAN_WAPI_MCAST_TIME,(void *)&val))
		{
			strcpy(tmpBuf,"Can not set MCAST TIME!");
			goto setErr_rekey;
		}
//		printf("REKEY_M_TIME %s \n",strVal);
	}
	
	strVal=req_get_cstream_var(wp,("REKEY_M_PACKET"),"");
	if(strVal)
	{
		val=atoi(strVal);
		if(!apmib_set(MIB_WLAN_WAPI_MCAST_PACKETS,(void *)&val))
		{
			strcpy(tmpBuf,"Can not set MCAST Packet!");
			goto setErr_rekey;
		}
//		printf("REKEY_M_PACKET %s \n",strVal);
	}	
	
	strVal=req_get_cstream_var(wp,("REKEY_POLICY"),"");
	if(strVal)
	{
		policy=strVal[0]-'0';
		if(!apmib_set(MIB_WLAN_WAPI_UCASTREKEY,(void *)&policy))
		{
			strcpy(tmpBuf,"Can not set ucast key policy!");
			goto setErr_rekey;
		}
//		printf("REKEY_POLICY %s \n",strVal);
	}

	strVal=req_get_cstream_var(wp,("REKEY_TIME"),"");
	if(strVal)
	{
		val=atoi(strVal);
		if(!apmib_set(MIB_WLAN_WAPI_UCAST_TIME,(void *)&val))
		{
			strcpy(tmpBuf,"Can not set ucast time!");
			goto setErr_rekey;
		}
//		printf("REKEY_TIME %s \n",strVal);
	}
	
	strVal=req_get_cstream_var(wp,("REKEY_PACKET"),"");
	if(strVal)
	{
		val=atoi(strVal);
		if(!apmib_set(MIB_WLAN_WAPI_UCAST_PACKETS,(void *)&val))
		{
			strcpy(tmpBuf,"Can not set ucast Packet!");
			goto setErr_rekey;
		}
//		printf("REKEY_PACKET %s \n",strVal);
	}
	
	apmib_update_web(CURRENT_SETTING);	// update configuration to flash
#ifndef NO_ACTION
	run_init_script("all");                
#endif
	OK_MSG(webpage);
//	send_redirect_perm(wp, webpage);
	return;
setErr_rekey:
	ERR_MSG(tmpBuf);
}
#define TMP_CERT "/var/tmp/tmp.cert"
#define AP_CERT "/var/myca/ap.cert"	//From local AS
#define CA_CERT "/var/myca/CA.cert"
#define CA4AP_CERT "/var/myca/ca4ap.cert"		//From local AS
#define CERT_START "-----BEGIN CERTIFICATE-----"
#define CERT_END "-----END CERTIFICATE-----"
#define CA_PRIV_KEY	"/var/myca/CA.key"


#define AP_CERT_AS0 "/var/myca/ap_as0.cert"	//From remote AS0
#define CA4AP_CERT_AS0 "/var/myca/ca4ap_as0.cert"		//From remote AS0
#define AP_CERT_AS1 "/var/myca/ap_as1.cert"	//From remote AS1
#define CA4AP_CERT_AS1 "/var/myca/ca4ap_as1.cert"		//From remote AS1

//Add for ASU certificate
#define ASU_CERT "/var/myca/asu.cert"
#define ASU_CERT_AS0 "/var/myca/asu_as0.cert"
#define ASU_CERT_AS1"/var/myca/asu_as1.cert"

#define CERT_VERIFY_RESULT "/var/myca/certVerifyResult"

//Add for verify user certificate
int verifyUserCertificate(char *ca_filename, char *user_filename)
{
	int retval,fd_res,size;	
	char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};
	unsigned char buffer[2];
	if(isFileExist(ca_filename)!=1)
	{		
		//sprintf(tmpBuf,"the %s is not exist,please upload it firstly!",ca_filename);
		printf("the %s is not exist!\n",ca_filename);
		retval=-1;
		goto check_ERR;
	}
	if(isFileExist(user_filename)!=1)
	{		
		//sprintf(tmpBuf,"the %s is not exist,please upload it firstly!",ca_filename);
		printf("the %s is not exist!\n",user_filename);
		retval=-1;
		goto check_ERR;
	}
	sprintf(tmpBuf, "openssl verify -CAfile %s %s > /dev/null 2>&1", ca_filename,user_filename); 
	system(tmpBuf);

	fd_res=open(CERT_VERIFY_RESULT, O_RDONLY);
	if (fd_res== -1 ) 
	{		
		//sprintf(tmpBuf,"can not open file %s!", CERT_VERIFY_RESULT);//Added for test
		printf("can not open file %s!\n", CERT_VERIFY_RESULT);
		retval=-2;
		goto check_ERR;
	}
	memset(buffer, 0, sizeof(buffer));
	size=read(fd_res, (void *)buffer, sizeof(buffer));
	if(size==-1)
	{		
		//sprintf(tmpBuf,"read file %s failure!", CERT_VERIFY_RESULT);//Added for test
		printf("read file %s failure!\n", CERT_VERIFY_RESULT);
		retval=-2;
		goto check_ERR;
	}
	if(buffer[0]!='0')
	{		
		//sprintf(tmpBuf,"user catificate %s is invalid!", user_filename);//Added for test
		retval=0;
		goto check_ERR;
	}
	retval=1;
	return retval;
	
	check_ERR:		
		return retval;	
		 
}
void formUploadWapiCert(request *wp, char * path, char * query)	// Only for Local AS
{
	/*save asu and user cert*/
	char *submitUrl,*strVal,*CertStart;
	unsigned int CertLength=0;
	char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};
	char cmd[128];
	FILE *fp;
	int check_res;
	
	int val =0;
	char * auth_mode;	
	
	auth_mode = req_get_cstream_var_in_mime(wp, ("auth_mode"), "",NULL);
	submitUrl = req_get_cstream_var_in_mime(wp, ("submit-url"), "",NULL);   // hidden page
	strVal = req_get_cstream_var_in_mime(wp, ("uploadcerttype"), "",NULL);

	if(!strcmp(auth_mode,"three_certification"))
	{
	//printf("val =3\n");
		val =3;		
	}else
	{
	//printf("val =2\n");
		val =2;
	}
	if(!apmib_set(MIB_WLAN_WAPI_AUTH_MODE_2or3_CERT,(void*)&val))
	{
			strcpy(tmpBuf, "Set Search MIB Index Error!");
			goto upload_ERR;
	}
	if(NULL == strstr(wp->upload_data,CERT_START) || NULL ==strstr(wp->upload_data,CERT_END))
	{
		strcpy(tmpBuf,"Not a Cert File!");
		goto upload_ERR;
	}
	fp=fopen(TMP_CERT,"w");
	if(NULL == fp)
	{
		strcpy(tmpBuf,"Can not open tmp cert!");
		goto upload_ERR;
	}
	else
	{
		if(!strcmp(strVal,"ca"))
		{
			CertStart = req_get_cstream_var_in_mime(wp,("ca_binary"),"",&CertLength);
		}else
		if(!strcmp(strVal,"user"))
		{
			CertStart = req_get_cstream_var_in_mime(wp,("user_binary"),"",&CertLength);
		}else
		if(!strcmp(strVal,"asu"))
		{
			CertStart = req_get_cstream_var_in_mime(wp,("asu_binary"),"",&CertLength);
		}else
		{
			strcpy(tmpBuf,"Unknown Cert File!");
			goto upload_ERR;
		}
		fwrite(CertStart,CertLength,0x1,fp);
		fclose(fp);
		strcpy(cmd,"cp ");
		strcat(cmd,TMP_CERT);
		strcat(cmd," ");
		if(!strcmp(strVal,"user"))
		{
			strcat(cmd,AP_CERT);
			system(cmd);
			if(val==3)
			{
				check_res=verifyUserCertificate(CA4AP_CERT,AP_CERT);
				if(check_res!=1)
				{
					if(check_res==0)
						sprintf(tmpBuf,"user catificate %s is invalid!", AP_CERT);//Added for test
					else if(check_res==-1)					
						sprintf(tmpBuf,"the %s is not exist,please upload it firstly!",CA4AP_CERT);

					system("rm -f /var/myca/ap.cert"); //rm AP_CERT
					goto upload_ERR;
				}
			}
			system("storeWapiFiles -apCert");
//			system("storeWapiFiles -oneUser");
		}else
		if(!strcmp(strVal,"asu"))
		{
		//	strcat(cmd,CA_CERT);
		//	system(cmd);
		//	system("storeWapiFiles -caCert");
		
		//	strcat(cmd,CA4AP_CERT);
			strcat(cmd,ASU_CERT);
			system(cmd);

			//Add for check asu certificate
			if(val==3)
			{
				check_res=verifyUserCertificate(CA4AP_CERT,ASU_CERT);
				if(check_res!=1)
				{
					if(check_res==0 || check_res==-2)
						sprintf(tmpBuf,"user catificate %s is invalid!", ASU_CERT);//Added for test
					else if(check_res==-1)					
						sprintf(tmpBuf,"the %s is not exist,please upload it firstly!",CA4AP_CERT);

					system("rm -f /var/myca/asu.cert"); //rm AP_CERT
					goto upload_ERR;
				}
			}
			
			system("storeWapiFiles -asuCert");
		}
		if(!strcmp(strVal,"ca"))
		{
			strcat(cmd,CA4AP_CERT);
			system(cmd);
			system("storeWapiFiles -ca4apCert");
			if(val==2)
			{
				/*if local AS and 2 cert mode, append private key to aus cert*/
				sprintf(tmpBuf,"cp %s %s ; cat %s >> %s",CA4AP_CERT,ASU_CERT,CA_PRIV_KEY,ASU_CERT);
 //				sprintf(tmpBuf,"cp %s %s",CA4AP_CERT,ASU_CERT);

				system(tmpBuf);			
				
				system("storeWapiFiles -asuCert");
			}
			//system("storeWapiFiles -caCertAS0");
		}		
	}
	/*check if user or asu cerification*/
	strcpy(tmpBuf,"Cerification Install Success!");
	OK_MSG1(tmpBuf, submitUrl);
	return;
upload_ERR:
	ERR_MSG(tmpBuf);
}

void formUploadWapiCertAS0(request *wp, char * path, char * query)	// Only for Remote AS0
{
		/*save asu and user cert*/
	char *submitUrl,*strVal,*CertStart;
	unsigned int CertLength=0;
	char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};
	char cmd[128];
	FILE *fp;
	int check_res;
	
	int val =0;
	char * auth_mode;	
	
	auth_mode = req_get_cstream_var_in_mime(wp, ("auth_mode"), "",NULL);
	submitUrl = req_get_cstream_var_in_mime(wp, ("submit-url"), "",NULL);   // hidden page
	strVal = req_get_cstream_var_in_mime(wp, ("uploadcerttype"), "",NULL);

	if(!strcmp(auth_mode,"three_certification"))
	{
	//printf("val =3\n");
		val =3;		
	}else
	{
	//printf("val =2\n");
		val =2;
	}
	if(!apmib_set(MIB_WLAN_WAPI_AUTH_MODE_2or3_CERT,(void*)&val))
	{
			strcpy(tmpBuf, "Set Search MIB Index Error!");
			goto upload_ERR;
	}
	if(NULL == strstr(wp->upload_data,CERT_START) || NULL ==strstr(wp->upload_data,CERT_END))
	{
		strcpy(tmpBuf,"Not a Cert File!");
		goto upload_ERR;
	}
	fp=fopen(TMP_CERT,"w");
	if(NULL == fp)
	{
		strcpy(tmpBuf,"Can not open tmp cert!");
		goto upload_ERR;
	}
	else
	{
		if(!strcmp(strVal,"ca"))
		{
			CertStart = req_get_cstream_var_in_mime(wp,("ca_binary"),"",&CertLength);
		}else
		if(!strcmp(strVal,"user"))
		{
			CertStart = req_get_cstream_var_in_mime(wp,("user_binary"),"",&CertLength);
		}else
		if(!strcmp(strVal,"asu"))
		{
			CertStart = req_get_cstream_var_in_mime(wp,("asu_binary"),"",&CertLength);
		}else
		{
			strcpy(tmpBuf,"Unknown Cert File!");
			goto upload_ERR;
		}
		fwrite(CertStart,CertLength,0x1,fp);
		fclose(fp);
		strcpy(cmd,"cp ");
		strcat(cmd,TMP_CERT);
		strcat(cmd," ");
		if(!strcmp(strVal,"user"))
		{
			strcat(cmd,AP_CERT_AS0);
			system(cmd);
			if(val==3)
			{
				check_res=verifyUserCertificate(CA4AP_CERT_AS0,AP_CERT_AS0);
				if(check_res!=1)
				{	
					if(check_res==0)
						sprintf(tmpBuf,"user catificate %s is invalid!", AP_CERT_AS0); 
					else if(check_res==-1)					
						sprintf(tmpBuf,"the %s is not exist,please upload it firstly!",CA4AP_CERT_AS0);
				
					system("rm -f /var/myca/ap_as0.cert"); //rm AP_CERT
					goto upload_ERR;
				}
			}
			system("storeWapiFiles -apCert");
//			system("storeWapiFiles -oneUser");
		}else
		if(!strcmp(strVal,"asu"))
		{
			//strcat(cmd,CA4AP_CERT_AS0);
			strcat(cmd, ASU_CERT_AS0);
			system(cmd);
			//system("storeWapiFiles -ca4apCertAS0");
			system("storeWapiFiles -asuCertAS0");
		}
		if(!strcmp(strVal,"ca"))
		{
			strcat(cmd,CA4AP_CERT_AS0);
			//strcat(cmd, CA_CERT_AS0);
			system(cmd);
			system("storeWapiFiles -ca4apCertAS0");
			if(val==2)
			{
//				sprintf(tmpBuf,"cp %s %s ; cat %s >> %s",CA4AP_CERT_AS0,ASU_CERT_AS0,CA_PRIV_KEY,ASU_CERT_AS0);
				sprintf(tmpBuf,"cp %s %s",CA4AP_CERT_AS0,ASU_CERT_AS0);

				system(tmpBuf);
				
				system("storeWapiFiles -asuCertAS0");
			}
			//system("storeWapiFiles -caCertAS0");
		}		
	}
	/*check if user or asu cerification*/
	strcpy(tmpBuf,"Cerification Install Success!");
	OK_MSG1(tmpBuf, submitUrl);
	return;
upload_ERR:
	ERR_MSG(tmpBuf);

}

void formUploadWapiCertAS1(request *wp, char * path, char * query)	// Only for Remote AS1
{
		/*save asu and user cert*/
	char *submitUrl,*strVal,*CertStart;
	unsigned int CertLength=0;
	char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};
	char cmd[128];
	FILE *fp;
	int check_res;
	
	int val =0;
	char * auth_mode;	
	
	auth_mode = req_get_cstream_var_in_mime(wp, ("auth_mode"), "",NULL);
	submitUrl = req_get_cstream_var_in_mime(wp, ("submit-url"), "",NULL);   // hidden page
	strVal = req_get_cstream_var_in_mime(wp, ("uploadcerttype"), "",NULL);

	if(!strcmp(auth_mode,"three_certification"))
	{
	//printf("val =3\n");
		val =3;		
	}else
	{
	//printf("val =2\n");
		val =2;
	}
	if(!apmib_set(MIB_WLAN_WAPI_AUTH_MODE_2or3_CERT,(void*)&val))
	{
			strcpy(tmpBuf, "Set Search MIB Index Error!");
			goto upload_ERR;
	}
	if(NULL == strstr(wp->upload_data,CERT_START) || NULL ==strstr(wp->upload_data,CERT_END))
	{
		strcpy(tmpBuf,"Not a Cert File!");
		goto upload_ERR;
	}
	fp=fopen(TMP_CERT,"w");
	if(NULL == fp)
	{
		strcpy(tmpBuf,"Can not open tmp cert!");
		goto upload_ERR;
	}
	else
	{
		if(!strcmp(strVal,"ca"))
		{
			CertStart = req_get_cstream_var_in_mime(wp,("ca_binary"),"",&CertLength);
		}else
		if(!strcmp(strVal,"user"))
		{
			CertStart = req_get_cstream_var_in_mime(wp,("user_binary"),"",&CertLength);
		}else
		if(!strcmp(strVal,"asu"))
		{
			CertStart = req_get_cstream_var_in_mime(wp,("asu_binary"),"",&CertLength);
		}else
		{
			strcpy(tmpBuf,"Unknown Cert File!");
			goto upload_ERR;
		}
		fwrite(CertStart,CertLength,0x1,fp);
		fclose(fp);
		strcpy(cmd,"cp ");
		strcat(cmd,TMP_CERT);
		strcat(cmd," ");
		if(!strcmp(strVal,"user"))
		{
			strcat(cmd,AP_CERT_AS1);
			system(cmd);
			
			//Add for check user certificate
			if(val==3)
			{
				check_res=verifyUserCertificate(CA4AP_CERT_AS1,AP_CERT_AS1);
				
				if(check_res!=1)
				{
					if(check_res==0)
						sprintf(tmpBuf,"user catificate %s is invalid!", AP_CERT_AS1);//Added for test
					else if(check_res==-1)					
						sprintf(tmpBuf,"the %s is not exist,please upload it firstly!",CA4AP_CERT_AS1);
				
					system("rm -f /var/myca/ap_as1.cert"); //rm AP_CERT
					goto upload_ERR;
				}
			}
			
			system("storeWapiFiles -apCertAS1");
		}
		if(!strcmp(strVal,"asu"))
		{
			//strcat(cmd,CA4AP_CERT_AS1);
			strcat(cmd,ASU_CERT_AS1);
			system(cmd);
			system("storeWapiFiles -asuCertAS1");
		}
		if(!strcmp(strVal,"ca"))
		{
			strcat(cmd,CA4AP_CERT_AS1);
			//strcat(cmd, CA_CERT_AS0);
			system(cmd);
			system("storeWapiFiles -ca4apCertAS1");

			if(val==2)
			{
//				sprintf(tmpBuf,"cp %s %s ; cat %s >> %s",CA4AP_CERT_AS1,ASU_CERT_AS1,CA_PRIV_KEY,ASU_CERT_AS1);
				sprintf(tmpBuf,"cp %s %s",CA4AP_CERT_AS1,ASU_CERT_AS1);

				system(tmpBuf);
				system("storeWapiFiles -asuCertAS1");
			}				
			//system("storeWapiFiles -caCertAS0");
		}
	}
	/*check if user or asu cerification*/
	strcpy(tmpBuf,"Cerification Install Success!");
	OK_MSG1(tmpBuf, submitUrl);
	return;
upload_ERR:
	ERR_MSG(tmpBuf);

}

char *getCertSerial(char *src, char *val)
{
	int len=0;

	while (*src && *src!=':') {
		*val++ = *src++;
		len++;
	}
	if (len == 0)
		return NULL;

	*val = '\0';

	if (*src==':')
		src++;

	return src;
}
void formWapiCertManagement(request *wp, char * path, char * query)
{
	char *strVal, sn[32], *webpage;
	char tmpBuf[200];
	// 1---revoke  2----unrevoke 3----del  4---active 5---search 6--clearall
	int operation;
	int val=0;
	webpage=req_get_cstream_var(wp,("next_webpage"),"");
	strVal=req_get_cstream_var(wp,("CERT_MNG"),"");
//	printf("CERT_MNG %s\n",strVal);
	/*Search  Revoke*/
	if(strVal)
		operation=strVal[0] -'0';
	else
		return;
	if(1 == operation)
	{
		/*get the serial no  122345:34244:343424:*/
		strVal=req_get_cstream_var(wp,("CERT_SN"),"");
//		printf("strVal=%s\n",strVal);//Added for test

		strVal=getCertSerial(strVal,sn);
		while(strVal)
		{
			/*call revoke API*/
			strcpy(tmpBuf," revokeUserCert.sh ");
			strcat(tmpBuf,sn);

			strVal=getCertSerial(strVal,sn);
			if(strVal!=NULL)
			{
				//There is more serial to revoke
				strcat(tmpBuf," option");
			}
			
//			printf("tmpBuf=%s\n",tmpBuf);//Added for test
			system(tmpBuf);
		}			
	}
	/*search*/
	if(5 == operation)
	{
		/*set search index*/
		strVal=req_get_cstream_var(wp,("SELECT1"),"");
		if(strVal)
		{
			val=strVal[0]-'0';
			if(!apmib_set(MIB_WLAN_WAPI_SEARCHINDEX,(void*)&val))
			{
				strcpy(tmpBuf, ("Set Search Index Error!"));
				goto setErr_cert;
			}
		}
//		printf("SELECT1 %s\n",strVal);
		strVal=req_get_cstream_var(wp,("CERT_INFO"),"");
		if(!apmib_set(MIB_WLAN_WAPI_SEARCHINFO,(void*)strVal))
		{
			strcpy(tmpBuf, ("Set Search Info Error!"));
			goto setErr_cert;
		}
//		printf("CERT_INFO %s\n",strVal);
		/*set search key*/
	}

	if(6 == operation)
	{
		system("initCAFiles.sh");
		val=1;
		apmib_set(MIB_WLAN_WAPI_CA_INIT,(void *)&val);
		
		//Keith add for update current time to MIB
	  
	  //if(time_mode == 0) //Manual Mode
	  { 
	   time_t current_secs;
	   int cur_time;
	   struct tm * tm_time;
	   
	   time(&current_secs);
	   tm_time = localtime(&current_secs);
	   cur_time = tm_time->tm_year+ 1900;
	   apmib_set( MIB_SYSTIME_YEAR, (void *)&cur_time);
	   cur_time = tm_time->tm_mon;
	   apmib_set( MIB_SYSTIME_MON, (void *)&cur_time);
	   cur_time = tm_time->tm_mday;
	   apmib_set( MIB_SYSTIME_DAY, (void *)&cur_time);
	   cur_time = tm_time->tm_hour;
	   apmib_set( MIB_SYSTIME_HOUR, (void *)&cur_time);
	   cur_time = tm_time->tm_min;
	   apmib_set( MIB_SYSTIME_MIN, (void *)&cur_time);
	   cur_time = tm_time->tm_sec;
	   apmib_set( MIB_SYSTIME_SEC, (void *)&cur_time);
	   
	   apmib_update_web(CURRENT_SETTING);
	  }

	}
	/*sync to flash*/
	apmib_update_web(CURRENT_SETTING);	// update configuration to flash

	send_redirect_perm(wp, webpage);
	return;
setErr_cert:
	ERR_MSG(tmpBuf);
}

extern void log_boaform(char *form);
#define WAPI_USER_CERT  "/var/myca/user.cert"
void formWapiCertDistribute(request *wp, char * path, char * query)
{
	char *strVal, *strName,*strTime, *webpage;
	int count=0;
	char tmpbuf[200];
	struct stat status;
	
	/*only 40 actived cert allowed*/
	CERTS_DB_ENTRY_Tp cert=(CERTS_DB_ENTRY_Tp)malloc(128*sizeof(CERTS_DB_ENTRY_T));
	/*update wapiCertInfo*/
	count=searchWapiCert(cert,5,"0");
	free(cert);
	if(count >= 40)
	{
		ERR_MSG("Too many active certifications. Please revoke unused certifications!");
		return;
	}
	/*generate a cert. Call generate API*/
	strVal=req_get_cstream_var(wp,("cert_type"),"");
//	printf("cert_type %s\n",strVal);
	strName=req_get_cstream_var(wp,("cert_name"),"");
//	printf("cert_name %s\n",strName);
	strTime=req_get_cstream_var(wp,("certPeriod"),"");
//	printf("certPeriod %s\n",strTime);
	strVal=req_get_cstream_var(wp,("time_unit"),"");
//	printf("time_unit %s\n",strVal);
	webpage=req_get_cstream_var(wp,("nextwebpage"),"");
//	printf("webpage %s\n",webpage);
	system("rm -f /var/myca/user.cert");
	system("rm -f /web/user.cer");

	/*To generate user.cert*/
	strcpy(tmpbuf,"genUserCert.sh ");
	strcat(tmpbuf,strName);
	strcat(tmpbuf," ");
	strcat(tmpbuf,strTime);
//	printf("tmpbuf :%s\n",tmpbuf);
	system(tmpbuf);

	sleep(1);
	
	if ( stat(WAPI_USER_CERT, &status) < 0 ) {
		printf("WAPI cert not generated!\n");
	}
	system("cp /var/myca/user.cert /web/user.cer");
	sleep(1);
	
	//Keith add for update current time to MIB
	//if(time_mode == 0) //Manual Mode
	{	
		time_t current_secs;
		int cur_time;
		struct tm * tm_time;
		
		time(&current_secs);
		tm_time = localtime(&current_secs);
		cur_time = tm_time->tm_year+ 1900;
		apmib_set( MIB_SYSTIME_YEAR, (void *)&cur_time);
		cur_time = tm_time->tm_mon;
		apmib_set( MIB_SYSTIME_MON, (void *)&cur_time);
		cur_time = tm_time->tm_mday;
		apmib_set( MIB_SYSTIME_DAY, (void *)&cur_time);
		cur_time = tm_time->tm_hour;
		apmib_set( MIB_SYSTIME_HOUR, (void *)&cur_time);
		cur_time = tm_time->tm_min;
		apmib_set( MIB_SYSTIME_MIN, (void *)&cur_time);
		cur_time = tm_time->tm_sec;
		apmib_set( MIB_SYSTIME_SEC, (void *)&cur_time);
		
		apmib_update_web(CURRENT_SETTING);
	}
	
		
	send_redirect_perm(wp, webpage);
#ifdef ASP_SECURITY_PATCH
	log_boaform("formWapiCertDistribute");	//To set formWapiCertDistribute valid at security_tbl
#endif
	return;
}
#endif

#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
#define RS_CERT_START "-----BEGIN CERTIFICATE-----"
#define RS_CERT_END "-----END CERTIFICATE-----"

#define RS_RSA_PRIV_KEY_START "-----BEGIN RSA PRIVATE KEY-----"
#define RS_RSA_PRIV_KEY_END "-----END RSA PRIVATE KEY-----"
#define RS_PRIV_KEY_TIP "PRIVATE KEY-----"



void formUpload8021xUserCert(request *wp, char * path, char * query)
{
	char *submitUrl,*strVal, *deleteAllCerts, *user_certstart,*ca_certstart;
	char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};
	int user_cert_len,ca_cert_len;
	char cmd[256];
	FILE *fp;
	char tryFormChange;
	char line[256];
	unsigned char userKeyPass[MAX_RS_USER_CERT_PASS_LEN+1];
	char certOk, userKeyOk;
	int wlanIdx_5G,wlanIdx_2G,rsBandSel;

	wlanIdx_5G=whichWlanIfIs(PHYBAND_5G);
	wlanIdx_2G=whichWlanIfIs(PHYBAND_2G);
	if(wlan_idx==wlanIdx_5G){
		rsBandSel=PHYBAND_5G;
		if ( !apmib_set(MIB_WLAN_RS_BAND_SEL, (void *)&rsBandSel)) {
			strcpy(tmpBuf, ("Set MIB_WLAN_RS_BAND_SEL error!"));
			goto upload_ERR;
		}
	}
	else if(wlan_idx==wlanIdx_2G){
		rsBandSel=PHYBAND_2G;
		if ( !apmib_set(MIB_WLAN_RS_BAND_SEL, (void *)&rsBandSel)) {
			strcpy(tmpBuf, ("Set MIB_WLAN_RS_BAND_SEL error!"));
			goto upload_ERR;
		}
	}
	else{
		rsBandSel=PHYBAND_OFF;
		if ( !apmib_set(MIB_WLAN_RS_BAND_SEL, (void *)&rsBandSel)) {
			strcpy(tmpBuf, ("Set MIB_WLAN_RS_BAND_SEL error!"));
			goto upload_ERR;
		}
	}

	if((rsBandSel != PHYBAND_5G) && (rsBandSel != PHYBAND_2G)){
		strcpy(tmpBuf, ("Wrong rsBandSel !"));
		goto upload_ERR;
	}
	
	//printf("---%s:%d---sizeof(upload_data)=%d	upload_len=%d\n",__FUNCTION__,__LINE__,wp->upload_data,wp->upload_len);
	
	strVal = req_get_cstream_var_in_mime(wp, ("uploadCertType"), "",NULL);
	submitUrl = req_get_cstream_var_in_mime(wp, ("submit-url"), "",NULL);   // hidden page
	deleteAllCerts = req_get_cstream_var_in_mime(wp, ("delAllCerts"), "",NULL);   // hidden page

	if(deleteAllCerts[0]=='1')
	{
		//To delete all 802.1x certs
		if(rsBandSel == PHYBAND_5G){
			system("rsCert -rst_5g");
			strcpy(tmpBuf,"Delete all 802.1x cerificates of 5GHz success!");
		}
		else{
			system("rsCert -rst_2g");
			strcpy(tmpBuf,"Delete all 802.1x cerificates of 2.4GHz success!");
		}
	}
	else
	{
		//Initial
		tryFormChange=0;
		certOk=0;
		userKeyOk=0;	
	
		if(NULL == strstr(wp->upload_data,RS_CERT_START)|| NULL ==strstr(wp->upload_data,RS_CERT_END))
		{
			//printf("---%s:%d---No 802.1x cert inclued in upload file!\n",__FUNCTION__,__LINE__);
			strcpy(tmpBuf,"No 802.1x cert inclued in upload file!");
			tryFormChange=1;
		}

		if((tryFormChange==0)&&(!strcmp(strVal,"user")))
		{
			//if(NULL == strstr(wp->upload_data,RS_PRIV_KEY_TIP))				
			if((NULL ==strstr(wp->upload_data,RS_RSA_PRIV_KEY_START)) || (NULL ==strstr(wp->upload_data,RS_RSA_PRIV_KEY_END)))
			{			
				//printf("---%s:%d---No 802.1x private key inclued in upload file!\n",__FUNCTION__,__LINE__);
				strcpy(tmpBuf,"No 802.1x private key inclued in upload file!");
				tryFormChange=1;
			}
		}	
		if(!strcmp(strVal,"user"))
		{		
			user_certstart= req_get_cstream_var_in_mime(wp, ("radiusUserCert"), "",&user_cert_len);			
			
			if(tryFormChange==0)
			{
				if(rsBandSel == PHYBAND_5G){
					fp=fopen(RS_USER_CERT_5G,"w");
					if(NULL == fp)
					{
						sprintf(tmpBuf,"Can not open tmp RS cert(%s)!", RS_USER_CERT_5G);
						goto upload_ERR;
					}
				}
				else{
					fp=fopen(RS_USER_CERT_2G,"w");
					if(NULL == fp)
					{
						sprintf(tmpBuf,"Can not open tmp RS cert(%s)!", RS_USER_CERT_2G);
						goto upload_ERR;
					}
				}				 
				fwrite(user_certstart,user_cert_len,0x1,fp);
				
				fclose(fp);
			}
			else
			{
				//To store user cert in tmp file: RS_USER_CERT_TMP
				fp=fopen(RS_USER_CERT_TMP,"w");
				if(NULL == fp)
				{
					sprintf(tmpBuf,"[2] Can not open tmp user cert(%s)!", RS_USER_CERT_TMP);
					goto upload_ERR;
				}
				fwrite(user_certstart,user_cert_len,0x1,fp);	
				fclose(fp);

				// try change user cert form from pfx to pem
				memset(userKeyPass, 0, sizeof(userKeyPass));
				apmib_get( MIB_WLAN_RS_USER_CERT_PASSWD, (void *)userKeyPass);
				if(rsBandSel == PHYBAND_5G){
					sprintf(cmd, "openssl pkcs12 -in %s -nodes -out %s -passin pass:%s", RS_USER_CERT_TMP, RS_USER_CERT_5G, userKeyPass);
				}
				else{
					sprintf(cmd, "openssl pkcs12 -in %s -nodes -out %s -passin pass:%s", RS_USER_CERT_TMP, RS_USER_CERT_2G, userKeyPass);
				}
				system(cmd);
				
				sleep(3); // wait for system(cmd) and avoid to open file failure;

				if(rsBandSel == PHYBAND_5G){
					fp=fopen(RS_USER_CERT_5G,"r");
					if(NULL == fp)
					{
						sprintf(tmpBuf,"[2] Can not open tmp user cert(%s)!Maybe you should upload your user certificate once again", RS_USER_CERT_5G);
						goto upload_ERR;
					}
				}
				else{
					fp=fopen(RS_USER_CERT_2G,"r");
					if(NULL == fp)
					{
						sprintf(tmpBuf,"[2] Can not open tmp user cert(%s)!Maybe you should upload your user certificate once again", RS_USER_CERT_2G);
						goto upload_ERR;
					}
				}
				while (fgets(line, sizeof(line), fp))
				{
					if((NULL != strstr(line,RS_CERT_START) ) || (NULL != strstr(line,RS_CERT_END) ))
						certOk=1;					
					//if(NULL != strstr(line,RS_PRIV_KEY_TIP))					
					if((NULL !=strstr(line,RS_RSA_PRIV_KEY_START)) || (NULL !=strstr(line,RS_RSA_PRIV_KEY_END)))
						userKeyOk=1;

					if((certOk == 1) && (userKeyOk == 1))
						break;
				}

				if((certOk != 1) || (userKeyOk != 1))
				{
					if(rsBandSel == PHYBAND_5G){
						sprintf(cmd, "rm -rf %s", RS_USER_CERT_5G);
					}
					else{
						sprintf(cmd, "rm -rf %s", RS_USER_CERT_2G);
					}
					system(cmd);
					
					sprintf(tmpBuf,"Upload user cert failed. Please make sure: 1) uploaded file in pem or pfx form, 2) uploaded file contain user cert and user key, 3) if any [User Key Password], please set it firstly at [Security] webpage for wlan client mode wpa/wpa2 enterprise.");
					goto upload_ERR;
				}

				fclose(fp);
			}

			//To store 802.1x user cert
			if(rsBandSel == PHYBAND_5G){
				system("rsCert -wrUser_5g");
			}
			else{
				system("rsCert -wrUser_2g");
			}
			strcpy(tmpBuf,"802.1x user cerificate and user key upload success!");
		}
		else if(!strcmp(strVal,"root"))
		{		
			ca_certstart= req_get_cstream_var_in_mime(wp, ("radiusRootCert"), "",&ca_cert_len);

			if(tryFormChange == 0)
			{
				if(rsBandSel == PHYBAND_5G){
					fp=fopen(RS_ROOT_CERT_5G,"w");
					if(NULL == fp)
					{
						sprintf(tmpBuf,"Can not open tmp RS cert(%s)!", RS_ROOT_CERT_5G);
						goto upload_ERR;
					}
				}
				else{
					fp=fopen(RS_ROOT_CERT_2G,"w");
					if(NULL == fp)
					{
						sprintf(tmpBuf,"Can not open tmp RS cert(%s)!", RS_ROOT_CERT_2G);
						goto upload_ERR;
					}
				}
				fwrite(ca_certstart,ca_cert_len,0x1,fp);				
				fclose(fp);
			}
			else
			{
				// To store ca cert in tmp file: RS_ROOT_CERT_TMP
				fp=fopen(RS_ROOT_CERT_TMP,"w");
				if(NULL == fp)
				{
					sprintf(tmpBuf,"Can not open tmp RS cert(%s)!", RS_ROOT_CERT_TMP);
					goto upload_ERR;
				}
				fwrite(ca_certstart,ca_cert_len,0x1,fp);				
				fclose(fp);
				
				// try change ca cert form from der to pem
				if(rsBandSel == PHYBAND_5G){
					sprintf(cmd, "openssl x509 -inform DER -in %s -outform PEM -out %s",RS_ROOT_CERT_TMP,RS_ROOT_CERT_5G);
				}
				else{
					sprintf(cmd, "openssl x509 -inform DER -in %s -outform PEM -out %s",RS_ROOT_CERT_TMP,RS_ROOT_CERT_2G);
				}
				system(cmd);
				
				sleep(3);	// wait for system(cmd) and avoid to open file failure;

				if(rsBandSel == PHYBAND_5G){
					fp=fopen(RS_ROOT_CERT_5G,"r");
					if(NULL == fp)
					{
						sprintf(tmpBuf,"[2] Can not open tmp RS cert(%s)!\nMaybe you should upload your root certificate once again!", RS_ROOT_CERT_5G);
						goto upload_ERR;
					}
				}
				else{
					fp=fopen(RS_ROOT_CERT_2G,"r");
					if(NULL == fp)
					{
						sprintf(tmpBuf,"[2] Can not open tmp RS cert(%s)!\nMaybe you should upload your root certificate once again!", RS_ROOT_CERT_2G);
						goto upload_ERR;
					}
				}

				while (fgets(line, sizeof(line), fp))
				{
					if((NULL != strstr(line,RS_CERT_START) ) || (NULL != strstr(line,RS_CERT_END) ))
					{
						certOk=1;
						break;
					}
				}

				if(certOk != 1)
				{
					if(rsBandSel == PHYBAND_5G){
						sprintf(cmd, "rm -rf %s", RS_ROOT_CERT_5G);
					}
					else{
						sprintf(cmd, "rm -rf %s", RS_ROOT_CERT_2G);
					}
					system(cmd);
					
					strcpy(tmpBuf,"[2] No 802.1x cert inclued in upload file!");
					goto upload_ERR;
				}
				
				fclose(fp);
			}

			//To store 802.1x root cert
			if(rsBandSel == PHYBAND_5G){
				system("rsCert -wrRoot_5g");
			}
			else{
				system("rsCert -wrRoot_2g");
			}
			strcpy(tmpBuf,"802.1x root cerificate upload success!");
		}
		else
		{
			sprintf(tmpBuf,"Upload cert type(%s) is not supported!", strVal);
			goto upload_ERR;
		}
	}
	
	OK_MSG1(tmpBuf, submitUrl);
	return;
	
upload_ERR:
	if(fp != NULL)
		fclose(fp);
	
	ERR_MSG(tmpBuf);
}
#endif


#ifdef TLS_CLIENT
#define MAXFNAME	60
#undef WEB_PAGE_OFFSET
#define WEB_PAGE_OFFSET 0x10000

//#define DWORD_SWAP(v) (v)
//#define WORD_SWAP(v) (v)
#define __PACK__	__attribute__ ((packed))
char *tag="CERT";
/////////////////////////////////////////////////////////////////////////////
static int compress(char *inFile, char *outFile)
{
	char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};

	//sprintf(tmpBuf, "bzip2 -9 -c %s > %s", inFile, outFile);
	sprintf(tmpBuf, "cat %s > %s", inFile, outFile);
	system(tmpBuf);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
#if 0
static unsigned char CHECKSUM(unsigned char *data, int len)
{
	int i;
	unsigned char sum=0;

	for (i=0; i<len; i++)
		sum += data[i];

	sum = ~sum + 1;
	return sum;
}
#endif

/////////////////////////////////////////////////////////////////////////////
#if 0
static int lookfor_cert_dir(FILE *lp, char *dirpath, int is_for_web)
{
	char file[MAXFNAME];
	char *p;
	struct stat sbuf;

	fseek(lp, 0L, SEEK_SET);
	dirpath[0] = '\0';

	while (fgets(file, sizeof(file), lp) != NULL) {
		if ((p = strchr(file, '\n')) || (p = strchr(file, '\r'))) {
			*p = '\0';
		}
		if (*file == '\0') {
			continue;
		}
		if (stat(file, &sbuf) == 0 && sbuf.st_mode & S_IFDIR) {
			continue;
		}
		if (is_for_web)
			p=strstr(file, "home.htm");

		else
			p=strrchr(file, '/');
		if (p) {

			*p = '\0';
			strcpy(dirpath, file);
// for debug
//printf("Found dir=%s\n", dirpath);
			return 0;
		}
	}
	//printf("error\n");
	return -1;
}
#endif
/////////////////////////////////////////////////////////////////////////////
static void strip_dirpath(char *file, char *dirpath)
{
	char *p, tmpBuf[MAXFNAME];

	if ((p=strstr(file, dirpath))) {
		strcpy(tmpBuf, &p[strlen(dirpath)]);
		strcpy(file, tmpBuf);
	}
// for debug
//printf("adding file %s\n", file);
}
int makeCertImage(char *outFile, char *fileList)
{
	int fh;
	struct stat sbuf;
	FILE *lp;
	char file[MAXFNAME];
	char tmpFile[100], dirpath[100];
	char buf[512];
	FILE_ENTRY_T entry;
	unsigned char	*p;
	int i, len, fd, nFile, pad=0;
	IMG_HEADER_T head;
	char *tmpFile1 = "/var/tmp/cert" ;
	
	fh = open(tmpFile1, O_RDWR|O_CREAT|O_TRUNC);
	if (fh == -1) {
		printf("Create output file error %s!\n", tmpFile1);
		return 0;
	}
	lseek(fh, 0L, SEEK_SET);

	if ((lp = fopen(fileList, "r")) == NULL) {
		printf("Can't open file list %s\n!", fileList);
		return 0;
	}
#if 0	
	if (lookfor_cert_dir(lp, dirpath, 0)<0) {
		printf("Can't find cert dir\n");
		fclose(lp);
		return 0;
	}
#else
	strcpy(dirpath, "/etc/1x");
#endif	
	fseek(lp, 0L, SEEK_SET);
	nFile = 0;
	while (fgets(file, sizeof(file), lp) != NULL) {
		if ((p = strchr(file, '\n')) || (p = strchr(file, '\r'))) {
			*p = '\0';
		}
		if (*file == '\0') {
			continue;
		}
		if (stat(file, &sbuf) == 0 && sbuf.st_mode & S_IFDIR) {
			continue;
		}

		if ((fd = open(file, O_RDONLY)) < 0) {
			printf("Can't open file %s\n", file);
			exit(1);
		}
		lseek(fd, 0L, SEEK_SET);

		strip_dirpath(file, dirpath);

		strcpy(entry.name, file);
#ifndef __mips__	
		entry.size = DWORD_SWAP(sbuf.st_size);
#else		
		entry.size = (sbuf.st_size);
#endif		

		if ( write(fh, (const void *)&entry, sizeof(entry))!=sizeof(entry) ) {
			printf("Write file failed!\n");
			return 0;
		}

		i = 0;
		while ((len = read(fd, buf, sizeof(buf))) > 0) {
			if ( write(fh, (const void *)buf, len)!=len ) {
				printf("Write file failed!\n");
				exit(1);
			}
			i += len;
		}
		close(fd);
		if ( i != sbuf.st_size ) {
			printf("Size mismatch in file %s!\n", file );
		}

		nFile++;
	}

	fclose(lp);
	close(fh);
	sync();

// for debug -------------
#if 0
sprintf(tmpFile, "cp %s web.lst -f", outFile);
system(tmpFile);
#endif
//-------------------------

	sprintf(tmpFile, "%sXXXXXX",  tmpFile1);
	mkstemp(tmpFile);

	if ( compress(tmpFile1, tmpFile) < 0) {
		printf("compress file error!\n");
		return 0;
	}

	// append header
	if (stat(tmpFile, &sbuf) != 0) {
		printf("Create file error!\n");
		return 0;
	}
	if((sbuf.st_size+1)%2)
		pad = 1;
	p = malloc(sbuf.st_size + 1 + pad);
	memset(p, 0 , sbuf.st_size + 1);
	if ( p == NULL ) {
		printf("allocate buffer failed!\n");
		return 0;
	}

	memcpy(head.signature, tag, 4);
	head.len = sbuf.st_size + 1 + pad;
#ifndef __mips__	
	head.len = DWORD_SWAP(head.len);
	head.startAddr = DWORD_SWAP(WEB_PAGE_OFFSET);
	head.burnAddr = DWORD_SWAP(WEB_PAGE_OFFSET);
#else
	head.len = (head.len);
	head.startAddr = (WEB_PAGE_OFFSET);
	head.burnAddr = (WEB_PAGE_OFFSET);
#endif		

	if ((fd = open(tmpFile, O_RDONLY)) < 0) {
		printf("Can't open file %s\n", tmpFile);
		return 0;
	}
	lseek(fd, 0L, SEEK_SET);
	if ( read(fd, p, sbuf.st_size) != sbuf.st_size ) {
		printf("read file error!\n");
		return 0;;
	}
	close(fd);

	p[sbuf.st_size + pad] = CHECKSUM(p, (sbuf.st_size+pad));

	fh = open(outFile, O_RDWR|O_CREAT|O_TRUNC);
	if (fh == -1) {
		printf("Create output file error %s!\n", outFile );
		return 0;
	}
#ifdef __mips__
	lseek(fh, CERT_PAGE_OFFSET , SEEK_SET);
#endif
	if ( write(fh, &head, sizeof(head)) != sizeof(head)) {
		printf("write header failed!\n");
		return 0;
	}

	if ( write(fh, p, (sbuf.st_size+1+pad) ) != (sbuf.st_size+1+pad)) {
		printf("write data failed!\n");
		return 0;
	}

	close(fh);
	chmod(outFile,  DEFFILEMODE);

	sync();

	free(p);
	unlink(tmpFile);

	return 0;
}


#define CERT_PATH "/etc/1x/"
void formCertUpload(request *wp, char * path, char * query)
{
    FILE *       fp;
    int          numWrite;
    char tmpBuf[200];
    int intVal, entryNum, i=0, add_entry=0, update_image=1;
    char *submitUrl, *strVal, *loadroot, *name, *loaduser, *strDelRoot, 
    		*strDelAllRoot,*strDelUser, *strDelAllUser, *strSelectCa;
    char fileName[50];
    int num_id, get_id, add_id, del_id, delall_id, max_num,index_id;
    CERTROOT_T rootEntry;
    CERTUSER_T userEntry;
    void *pEntry;

     	submitUrl = req_get_cstream_var(wp, ("url"), "");   // hidden page
     	loadroot =  req_get_cstream_var(wp, ("loadroot"), ""); 
     	loaduser =  req_get_cstream_var(wp, ("loaduser"), ""); 
     	name =  req_get_cstream_var(wp, ("name"), "");
     	strDelRoot =   req_get_cstream_var(wp, ("deleteSelRoot"), "");
     	strDelAllRoot =   req_get_cstream_var(wp, ("deleteAllRoot"), "");
	strDelUser =   req_get_cstream_var(wp, ("deleteSelUser"), "");
     	strDelAllUser =   req_get_cstream_var(wp, ("deleteAllUser"), "");     	
     	strSelectCa =   req_get_cstream_var(wp, ("selectca"), ""); 
     	memset(&rootEntry, '\0', sizeof(rootEntry));
     	
     	if(loadroot[0] || strDelRoot[0] || strDelAllRoot[0] || strSelectCa[0]){
		num_id = MIB_CERTROOT_TBL_NUM;
		max_num = MAX_CERTROOT_NUM;
		add_id = MIB_CERTROOT_ADD;
		del_id = MIB_CERTROOT_DEL ;
		delall_id = MIB_CERTROOT_DELALL ;
		get_id = MIB_CERTROOT_TBL ;
		index_id = MIB_ROOT_IDX;
		memset(&rootEntry, '\0', sizeof(rootEntry));
		pEntry = (void *) & rootEntry ;
			
	}
	else if(loaduser[0] || strDelUser[0] || strDelAllUser[0]){
		num_id = MIB_CERTUSER_TBL_NUM;
		max_num = MAX_CERTUSER_NUM;
		add_id = MIB_CERTUSER_ADD;
		del_id = MIB_CERTUSER_DEL ;
		delall_id = MIB_CERTUSER_DELALL ;
		get_id = MIB_CERTUSER_TBL ;
		index_id = MIB_USER_IDX;
		memset(&userEntry, '\0', sizeof(userEntry));
		pEntry = (void *) & userEntry ;
	}
	else{
		strcpy(tmpBuf, "error handle\n");
		goto  ret_upload;
	}

	if(strSelectCa[0]){ //set ca index
		
		strVal = req_get_cstream_var(wp, "rootSelect", "");
		if ( !apmib_get(MIB_CERTROOT_TBL_NUM, (void *)&entryNum)) {
				strcpy(tmpBuf, ("Get entry number error!"));
				goto ret_upload;
		}
		if ( strVal[0] ) {
			intVal =  atoi(strVal) ;
			if ( !apmib_set(MIB_ROOT_IDX, (void *)&intVal)) {
				strcpy(tmpBuf, ("Set CA select error!"));
				goto ret_upload;
			}
			if( intVal <= entryNum){
				pEntry = (void *) &rootEntry ;
				*((char *)pEntry) = (char)intVal;
				if ( !apmib_get(MIB_CERTROOT_TBL, (void *)pEntry)){
					sprintf(tmpBuf, "Get Mib Root CA  entry %d error\n", intVal);
					goto ret_upload;      
				}
			}
			else{
					sprintf(tmpBuf, "invalid Root CA entry %d select\n",intVal );
					goto ret_upload;      
			}
		}
		strVal = req_get_cstream_var(wp, "userSelect", "");
		if ( !apmib_get(MIB_CERTUSER_TBL_NUM, (void *)&entryNum)) {
				strcpy(tmpBuf, ("Get entry number error!"));
				goto ret_upload;
		}
		if ( strVal[0] ) {
			intVal =  atoi(strVal) ;
			if ( !apmib_set(MIB_USER_IDX, (void *)&intVal)) {
				strcpy(tmpBuf, ("Set User select error!"));
				goto ret_upload;
			}
			if( intVal <= entryNum){
				pEntry = (void *) &userEntry ;
				*((char *)pEntry) = (char)intVal;
				if ( !apmib_get(MIB_CERTUSER_TBL, (void *)pEntry)){
					sprintf(tmpBuf, "Get Mib User entry entry %d error\n",i );
					goto ret_upload;      
				}
			}
			else{
					sprintf(tmpBuf, "invalid User entry select %d\n", i);
					goto ret_upload;      
			}
		}	
#if 0
		//printf(" ca files %s %s\n", rootEntry.comment, userEntry.comment); //for debug
		sprintf(tmpBuf, "openssl pkcs12 -des3 -in /etc/1x/%s.pfx -out /etc/1x/user.pem   -passout pass:realtek -passin pass:realtek", userEntry.comment);
		system(tmpBuf);
		sprintf(tmpBuf, "openssl x509 -inform PEM -outform DER -in /etc/1x/user.pem -out /etc/1x/user.der");
		system(tmpBuf);
		sprintf(tmpBuf, "openssl x509 -inform DER -in /etc/1x/%s.cer -outform PEM -out /etc/1x/root.pem", rootEntry.comment);
		system(tmpBuf);
#endif
		update_image=0;
	}
	
     	if(loadroot[0] || loaduser[0]){		//Add entry
		// get entry number to see if it exceeds max
		
		if ( !apmib_get(num_id, (void *)&intVal)) {
				strcpy(tmpBuf, ("Get entry number error!"));
				goto ret_upload;
		}
		if ( (intVal + 1) > max_num) {
			strcpy(tmpBuf, ("Cannot add new entry because table is full!"));
			goto ret_upload;
		}     		
		if(wp->post_data_len == 0){
			strcpy(tmpBuf, ("Error ! Upload file length is 0 !"));
			goto  ret_upload;
		 }
		 
		 for(i=1 ; i <= intVal ; i++) //check the duplicate entry
		 {
		 	*((char *)pEntry) = (char)i;
			if ( !apmib_get(get_id, (void *)pEntry)){
				sprintf(tmpBuf, "Get Mib CA entry %d error\n", i);
				goto ret_upload;      
			}
			if(loadroot[0] && !strcmp(rootEntry.comment,name)){
				sprintf(tmpBuf, "Error! Duplicate Root CA name %s with entry %d\n", name, i);
				goto ret_upload;
			}
			if(loaduser[0] && !strcmp(userEntry.comment,name)){
				sprintf(tmpBuf, "Error! Duplicate User CA name %s with entry %d\n", name, i);
				goto ret_upload;
			}
		 }
		 if(loaduser[0]){
			strVal = req_get_cstream_var(wp, "pass", "");
			if(strVal[0])
				strcpy(userEntry.pass, strVal);
		 }
		 if(loadroot[0]){
		     	strcpy(fileName, CERT_PATH);
		     	strcat(fileName, name);
			strcat(fileName,".cer");
			strcpy(rootEntry.comment, name);
		 }
		 else{
		     	strcpy(fileName, CERT_PATH);
		     	strcat(fileName, name);
		     	strcat(fileName, ".pfx");
		     	strcpy(userEntry.comment, name);
		 }  
		 if ((fp = fopen(fileName, "w+b")) != NULL) {
			numWrite = fwrite(wp->post_data,1, wp->post_data_len, fp);
			if(numWrite < 0) perror("write error");
			if (numWrite == wp->post_data_len)
				sprintf(tmpBuf, ("Update successfully (size = %d bytes)!<br>"), wp->post_data_len);
			else
				sprintf(tmpBuf, ("Writesize=%d %dbytes."), wp->post_data_len, numWrite);
		 }
		 else {
			sprintf(tmpBuf, ("open file error"));
			goto ret_upload;
		 }
		    	
		fclose(fp);
		if ( apmib_set(add_id, (void *)pEntry) == 0) {
			strcpy(tmpBuf, ("Add table entry error!"));
			goto ret_upload;
		}
		add_entry =1 ;
		
    	}
    	/* Delete entry */
	if (strDelRoot[0] || strDelUser[0]) {
		if ( !apmib_get(num_id, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto ret_upload;
		}

		strVal = req_get_cstream_var(wp, "selectcert", "");
		if ( strVal[0] ) {
			*((char *)pEntry) = atoi(strVal);
			if ( !apmib_get(get_id, (void *)pEntry)) {
				strcpy(tmpBuf, ("Get table entry error!"));
				goto ret_upload;
			}
			if ( !apmib_set(del_id, (void *)pEntry)) {
				strcpy(tmpBuf, ("Delete table entry error!"));
				goto ret_upload;
			}
		}
		if(strDelRoot[0])
			sprintf(tmpBuf, "rm -f %s%s.cer", CERT_PATH, rootEntry.comment);
		else			
			sprintf(tmpBuf, "rm -f %s%s.pfx", CERT_PATH, userEntry.comment);
		
		system(tmpBuf);
	}
	/* Delete all entry */
	if ( strDelAllRoot[0] || strDelAllUser[0]) {
		if ( !apmib_set(delall_id, pEntry)) {
			strcpy(tmpBuf, ("Delete all table error!"));
			goto ret_upload;
		}
		if(strDelAllRoot[0])
			system("rm -f /etc/1x/*.cer");
		else
			system("rm -f /etc/1x/*.pfx");
	}
	apmib_update_web(CURRENT_SETTING);	// update configuration to flash
	if(update_image){
		system("find   /etc/1x/*.pfx  -type f > /var/tmp/cert.list"); 
		system("find   /etc/1x/*.cer  -type f >> /var/tmp/cert.list"); 
#ifdef __mips__
		makeCertImage(FLASH_DEVICE_NAME, "/tmp/cert.list");
#else
		makeCertImage("cert.img", "/var/tmp/cert.list");
#endif
		system("rm -f /var/tmp/cert.list");

	}
#ifndef NO_ACTION
	else
		run_init_script("bridge");
#endif
		
	if(add_entry){
		OK_MSG1(tmpBuf, submitUrl);
	}
	else
	{
		if (submitUrl[0])
			send_redirect_perm(wp, submitUrl);
	}			
    return;

ret_upload:
    ERR_MSG(tmpBuf);
}

int certRootList(request *wp, int argc, char **argv)
{
	int	nBytesSent=0, entryNum, i;
	CERTROOT_T entry;

	if ( !apmib_get(MIB_CERTROOT_TBL_NUM, (void *)&entryNum)) {
  		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}

	nBytesSent += req_format_write(wp, ("<tr>"
      	"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Name</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td></tr>\n"));

	for (i=1; i<=entryNum; i++) {
		*((char *)&entry) = (char)i;
		if ( !apmib_get(MIB_CERTROOT_TBL, (void *)&entry))
			return -1;

		nBytesSent += req_format_write(wp, ("<tr>"
			"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"selectcert\" value=\"%d\" onClick=\"selectcaClick(this)\">"
      			"</td></tr>\n"), entry.comment, i);
	}
	return nBytesSent;
}

int certUserList(request *wp, int argc, char **argv)
{
	int	nBytesSent=0, entryNum, i;
	CERTUSER_T entry;

	if ( !apmib_get(MIB_CERTUSER_TBL_NUM, (void *)&entryNum)) {
  		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}

	nBytesSent += req_format_write(wp, ("<tr>"
      	"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Name</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td></tr>\n"));

	for (i=1; i<=entryNum; i++) {
		*((char *)&entry) = (char)i;
		if ( !apmib_get(MIB_CERTUSER_TBL, (void *)&entry))
			return -1;

		nBytesSent += req_format_write(wp, ("<tr>"
			"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"selectcert\" value=\"%d\" onClick=\"selectprClick(this)\"></td></tr>\n"), entry.comment, i);
	}
	return nBytesSent;
}
#endif

#if defined(CONFIG_RTL_P2P_SUPPORT)

int getWifiP2PState(request *wp, int argc, char **argv)
{
	int nBytesSent=0, i;
	int intVal;
	P2P_SS_STATUS_Tp pP2PStatus=NULL;
	
	if (pP2PStatus==NULL) {
		pP2PStatus = calloc(1, sizeof(P2P_SS_STATUS_Tp));
		if ( pP2PStatus == NULL ) {
			printf("Allocate buffer failed!\n");
			return 0;
		}
	}
	
	sprintf(WLAN_IF,"wlan0");
	if ( getWlP2PStateEvent(WLAN_IF, pP2PStatus) < 0)
	{
		printf("\r\n getWlP2PStateEvent fail,__[%s-%u]\r\n",__FILE__,__LINE__);
		nBytesSent += req_format_write(wp,("None"));
	}
	else
	{
		unsigned char line_buffer[100]={0};
		
			
		//printf("\r\n p2p_status_event=[%d|%d|%d],__[%s-%u]\r\n",
		//pP2PStatus->p2p_status,pP2PStatus->p2p_event,pP2PStatus->p2p_wsc_method,__FILE__,__LINE__);	
		sprintf(line_buffer,"%d|%d|%d|%d|%s",pP2PStatus->p2p_status,pP2PStatus->p2p_event,pP2PStatus->p2p_wsc_method, pP2PStatus->p2p_role);
//printf("\r\n line_buffer=[%s],__[%s-%u]\r\n",line_buffer,__FILE__,__LINE__);		
		nBytesSent += req_format_write(wp,"%s",line_buffer);
	}
	
	if(pP2PStatus)
		free(pP2PStatus);
	
	return nBytesSent;
}



void formWiFiDirect(request *wp, char *path, char *query)
{
	char *submitUrl,*strTmp;
	char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};
	int valTmp;
	int wlanBand2G5GSelect;
	char syscmd[50];
	int needapply = 0;
	
	//displayPostDate(wp->post_data);	
	
	strTmp= req_get_cstream_var(wp, ("p2p_op_channel"), "");
	if(strTmp[0])
	{
		valTmp = atoi(strTmp);		
		
#if defined(CONFIG_RTL_92D_SUPPORT)		
		apmib_get(MIB_WLAN_BAND2G5G_SELECT,(void *)&wlanBand2G5GSelect);
	
		if(wlanBand2G5GSelect == BANDMODESINGLE)
		{
			int phyBandSelect = PHYBAND_OFF;
			int wlanif;
			
			if(valTmp > 11) // 5g band
			{
				phyBandSelect = PHYBAND_5G;				
			}
			else
			{
				phyBandSelect = PHYBAND_2G;				
			}
						
			wlanif = whichWlanIfIs(phyBandSelect);			
				
			if(wlanif != 0)
			{
				int val;
				val = 1;
				apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&val); //close original interface
				
				swapWlanMibSetting(0,wlanif);
				
				val = 0;
				apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&val); //enable after interface							
			}
		}
#endif //#if defined(CONFIG_RTL_92D_SUPPORT)
			
		apmib_set(MIB_WLAN_P2P_OPERATION_CHANNEL, (void *)&valTmp); 				
		sprintf(syscmd,"iwpriv wlan0 p2pcmd opchannel,%d",valTmp);
		system(syscmd);
		needapply = 1;
	}
	
	strTmp= req_get_cstream_var(wp, ("p2p_type"), "");
	if(strTmp[0])
	{
		valTmp = atoi(strTmp);
	
		if(valTmp == 0)
			valTmp = P2P_DEVICE;
		else
			valTmp = P2P_TMP_GO;
			
		//apmib_set(MIB_WLAN_P2P_TYPE, (void *)&valTmp);
		if(valTmp == P2P_TMP_GO)
		{
			sprintf(syscmd,"iwpriv wlan0 p2pcmd asgo");
			P2P_DEBUG("\n\n%s\n",syscmd);
			system(syscmd);
		}
		else
		{
			//apmib_set(MIB_WLAN_SSID, "");
			sprintf(syscmd,"iwpriv wlan0 p2pcmd bakdev");
			P2P_DEBUG("\n\n%s\n",syscmd);			
			system(syscmd);
		}

		//needapply = 1;		
	}
	
	strTmp= req_get_cstream_var(wp, ("p2p_intent"), "");
	if(strTmp[0])
	{
		valTmp = atoi(strTmp);			
		apmib_set(MIB_WLAN_P2P_INTENT, (void *)&valTmp); 		
		sprintf(syscmd,"iwpriv wlan0 p2pcmd intent,%d",valTmp);
		system(syscmd);
		needapply = 1;		
	}
	
	strTmp= req_get_cstream_var(wp, ("p2p_listen_channel"), "");
	if(strTmp[0])
	{
		valTmp = atoi(strTmp);
			
		apmib_set(MIB_WLAN_P2P_LISTEN_CHANNEL, (void *)&valTmp); 
		sprintf(syscmd,"iwpriv wlan0 p2pcmd channel,%d",valTmp);
		system(syscmd);
		needapply = 1;		
	}
	
	strTmp= req_get_cstream_var(wp, ("dev_name"), "");
	if(strTmp[0])
	{					
		apmib_set(MIB_DEVICE_NAME, (void *)strTmp); 				
		sprintf(syscmd,"iwpriv wlan0 p2pcmd devname,%s",strTmp);
		system(syscmd);
		needapply = 1;		
	}


	if(	needapply == 1){
		sprintf(syscmd,"iwpriv wlan0 p2pcmd apply");
		system(syscmd);
	}


	
	strTmp= req_get_cstream_var(wp, ("action"), "");
	if(strTmp[0])
	{
		
		//printf("\r\n action=[%s],__[%s-%u]\r\n",strTmp,__FILE__,__LINE__);
		if(strcmp(strTmp,"p2pBackToDevice") == 0)
		{
			int valTmp = P2P_DEVICE;
			int valTmp2 = 0;
			int tmpint,tmpint2;
			
			unsigned char ssidstr[33];


			tmpint = wlan_idx ;
			tmpint2 = vwlan_idx ;		
			strcpy(ssidstr,"");

			vwlan_idx = 0;
			wlan_idx=0;
			
			apmib_set( MIB_WLAN_P2P_TYPE, (void *)&valTmp);		
			apmib_set( MIB_WLAN_SSID, (void *)ssidstr);
			apmib_set( MIB_WLAN_WPA_PSK, (void *)ssidstr);		
			apmib_set( MIB_WLAN_AUTH_TYPE, (void *)&valTmp2);
			apmib_set( MIB_WLAN_ENCRYPT, (void *)&valTmp2);
			apmib_set( MIB_WLAN_WPA_CIPHER_SUITE, (void *)&valTmp2);
			apmib_set( MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&valTmp2);
			apmib_set( MIB_WLAN_WSC_CONFIGURED, (void *)&valTmp2);		
			apmib_update_web(CURRENT_SETTING);	// update to flash					
			wlan_idx = tmpint ;		
			vwlan_idx = tmpint2 ;



			sprintf(syscmd,"iwpriv wlan0 p2pcmd bakdev");
			//printf("%s %d : %s\n",__FUNCTION__,__LINE__,syscmd);
			system(syscmd);


		}
		else if(strcmp(strTmp,"p2pWSCSelect") == 0)
		{
			P2P_PROVISION_COMM_T p2pProvisionComm;			
			
			
			strTmp= req_get_cstream_var(wp, ("wscProvsionMethod"), "");
			if(strTmp[0])
			{
				int wscProvsionMethod;
				
				wscProvsionMethod = atoi(strTmp);


				if(wscProvsionMethod == 1) //display
					p2pProvisionComm.wsc_config_method = CONFIG_METHOD_DISPLAY;
				else if(wscProvsionMethod == 2) //keypad
					p2pProvisionComm.wsc_config_method = CONFIG_METHOD_KEYPAD;
				else if(wscProvsionMethod == 4) //pbc
					p2pProvisionComm.wsc_config_method = CONFIG_METHOD_PBC;
			}
			
			strTmp= req_get_cstream_var(wp, ("wscProvsionDevMac"), "");
			if(strTmp[0])
			{	
				if (strlen(strTmp)!=12 || !string_to_hex(strTmp, p2pProvisionComm.dev_address, 12)) 
				{
					strcpy(tmpBuf, ("Error! Invalid MAC address."));
				
				}
				

			}
			
			strTmp= req_get_cstream_var(wp, ("wscProvsionDevChannel"), "");
			if(strTmp[0])
			{			
				p2pProvisionComm.channel = atoi(strTmp);				
			}
			
			sprintf(WLAN_IF,"wlan0");
			
			if ( sendP2PProvisionCommInfo(WLAN_IF, &p2pProvisionComm) < 0)
			{
				printf("\r\n sendP2PProvisionCommInfo fail,__[%s-%u]\r\n",__FILE__,__LINE__);
			}
			
			
			return;
		}
		else if(strcmp(strTmp,"p2pWSCConnect") == 0)
		{
			P2P_WSC_CONFIRM_T p2pWscConfirm;
			
			memset(&p2pWscConfirm, 0x00, sizeof(P2P_WSC_CONFIRM_T));
			
			strTmp= req_get_cstream_var(wp, ("wscProvsionMethod"), "");
			if(strTmp[0])
			{
				int wscProvsionMethod;
				
				wscProvsionMethod = atoi(strTmp);
				
				if(wscProvsionMethod == 1) //display
					p2pWscConfirm.wsc_config_method = 0x8;
				else if(wscProvsionMethod == 2) //keypad
					p2pWscConfirm.wsc_config_method = 0x100;
				else if(wscProvsionMethod == 4) //pbc
					p2pWscConfirm.wsc_config_method = 0x80;
			}
			
			strTmp= req_get_cstream_var(wp, ("wscPinCode"), "");
			if(strTmp[0])
			{			
				sprintf(p2pWscConfirm.pincode, "%s", strTmp);
			}

			// 0316 add
			strTmp= req_get_cstream_var(wp, ("wscProvsionDevMac"), "");
			if(strTmp[0])
			{	
				if (strlen(strTmp)!=12 || !string_to_hex(strTmp, p2pWscConfirm.dev_address, 12)) 
				{
					strcpy(tmpBuf, ("Error! Invalid MAC address."));
				
				}
				

			}
			
			sprintf(WLAN_IF,"wlan0");
			if ( sendP2PWscConfirm(WLAN_IF, &p2pWscConfirm) < 0)
			{
				printf("\r\n sendP2PWscConfirm fail,__[%s-%u]\r\n",__FILE__,__LINE__);
			}
			
			return;
		}
		
	}

	
	submitUrl = req_get_cstream_var(wp, ("submit-url"), "");   // hidden page
	
	apmib_update_web(CURRENT_SETTING);
	
	if(run_init_script_flag == 1)
	{
		char tmpMsg[300];
		char lan_ip_buf[30], lan_ip[30];
		
		sprintf(tmpMsg, "%s","Change setting successfully!<br><br>Do not turn off or reboot the Router during this time.");
		apmib_get( MIB_IP_ADDR,  (void *)lan_ip_buf) ;
	  sprintf(lan_ip,"%s",inet_ntoa(*((struct in_addr *)lan_ip_buf)) );
		OK_MSG_FW(tmpMsg, submitUrl,APPLY_COUNTDOWN_TIME,lan_ip);		
		run_init_script("all");
	}
	else
	{
		if(needReboot == 1)
		{
		run_init_script("all");
		OK_MSG(submitUrl);
	}
		else
		{
			send_redirect_perm(wp, submitUrl);
		}
	}
	
	return;

setErr_WifiDirect:
	ERR_MSG(tmpBuf);
	
}

void formWlP2PScan(request *wp, char *path, char *query)
{
 	char *submitUrl, *refresh, *strVal;
	int status;

	unsigned char res, *pMsg=NULL;
	int wait_time, max_wait_time=5;
	char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};

	//displayPostDate(wp->post_data);	

	submitUrl = req_get_cstream_var(wp, ("submit-url"), "");

	refresh = req_get_cstream_var(wp, ("refresh"), "");
	if ( refresh[0] ) {
		// issue scan request
		wait_time = 0;
		while (1) 
		{
			strVal = req_get_cstream_var(wp, ("ifname"), "");
			if(strVal[0])
			{
				sprintf(WLAN_IF,"%s",strVal);				
			}
			 
			switch(getWlP2PScanRequest(WLAN_IF, &status)) 
			{ 
				case -2: 
					printf("-2\n"); 
					strcpy(tmpBuf, ("Auto scan running!!please wait...")); 
					goto ss_err; 
					break; 
				case -1: 
					printf("-2\n"); 
					strcpy(tmpBuf, ("Site-survey request failed!")); 
					goto ss_err; 
					break; 
				default: 
					break; 
			} 

//printf("\r\n status=[%d],__[%s-%u]\r\n",status,__FILE__,__LINE__);

			if (status != 0) // not ready
			{	
				if (wait_time++ > 5) 
				{
					strcpy(tmpBuf, ("scan request timeout!"));
					printf("\r\n scan request timeout,__[%s-%u]\r\n",__FILE__,__LINE__);
					goto ss_err;
				}
				sleep(1);
			}
			else
				break;
		}
		
		
		wait_time = 0;
		while (1) {
			res = 1;	// only request request status
			
			if ( getWlP2PScanResult(WLAN_IF, (SS_STATUS_Tp)&res) < 0 ) 
			{
			
			}

			if (res == 0xff)    // in progress
			{

					
				if (wait_time++ > 20) 			
				{
					strcpy(tmpBuf, ("scan timeout!"));
					if(pStatus)
					{
						free(pStatus);
						pStatus = NULL;
					}
					goto ss_err;
				}
//printf("\r\n wait_time=[%d],__[%s-%u]\r\n",wait_time,__FILE__,__LINE__);				
				sleep(1);
			}
			else
				break;
		}
//printf("\r\n submitUrl=[%s],__[%s-%u]\r\n",submitUrl,__FILE__,__LINE__);

		if (submitUrl[0])
			send_redirect_perm(wp, submitUrl);

		return;
	}
	
	return;
	
ss_err:
	ERR_MSG(tmpBuf);
}

int wlP2PScanTbl(request *wp, int argc, char **argv)
{
	int nBytesSent=0, i;
	BssDscr *pBss;	
	char tmpBuf[MAX_MSG_BUFFER_SIZE], tmpBuf2[MAX_MSG_BUFFER_SIZE], p2pTypeStr[5];

	//printf("\r\n __[%s-%u]\r\n",__FILE__,__LINE__);

	if (pStatus==NULL) {
		pStatus = calloc(1, sizeof(SS_STATUS_T));
		if ( pStatus == NULL ) {
			printf("Allocate buffer failed!\n");
			return 0;
		}
	}

	pStatus->number = 0; // request BSS DB

	if ( getWlP2PScanResult(WLAN_IF, pStatus) < 0 ) {
		//ERR_MSG("Read site-survey status failed!");
		//printf("\r\n getWlP2PScanResult() fail.__[%s-%u]\r\n",__FILE__,__LINE__);		
		//req_format_write(wp, "Read site-survey status failed!");
		free(pStatus); //sc_yang
		pStatus = NULL;
		return 0;
	}

	nBytesSent += req_format_write(wp, ("<tr>"
	"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Device Name</b></font></td>\n"
	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Channel</b></font></td>\n"
	"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Device address</b></font></td>\n"
  "<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Role</b></font></td>\n"));
	
	nBytesSent += req_format_write(wp, ("<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td></tr>\n"));


	

//printf("\r\n pStatus->number=[%d],__[%s-%u]\r\n",pStatus->number,__FILE__,__LINE__);

	for (i=0; i<pStatus->number && pStatus->number!=0xff; i++) 
	{
		pBss = &pStatus->bssdb[i];
		snprintf(tmpBuf, MAX_MSG_BUFFER_SIZE, ("%02x:%02x:%02x:%02x:%02x:%02x"),
			pBss->p2paddress[0], pBss->p2paddress[1], pBss->p2paddress[2],
			pBss->p2paddress[3], pBss->p2paddress[4], pBss->p2paddress[5]);
			
		snprintf(tmpBuf2, MAX_MSG_BUFFER_SIZE, ("%02x%02x%02x%02x%02x%02x"),
			pBss->p2paddress[0], pBss->p2paddress[1], pBss->p2paddress[2],
			pBss->p2paddress[3], pBss->p2paddress[4], pBss->p2paddress[5]);
			
//printf("\r\n pBss->p2prole=[%d],__[%s-%u]\r\n",pBss->p2prole,__FILE__,__LINE__);
	
		memset(p2pTypeStr, 0x00, sizeof(p2pTypeStr));

	   if(pBss->p2prole == R_P2P_DEVICE)
	   {
	       strcpy(p2pTypeStr,"DEV");
	   }
	   else if(pBss->p2prole == R_P2P_GO)// 1:GO
	   {
	       strcpy(p2pTypeStr,"GO");
	   }else{
	       strcpy(p2pTypeStr,"?D");
	   }




		nBytesSent += req_format_write(wp, ("<tr>"
		"<td align=center bgcolor=\"#C0C0C0\"><pre><font size=\"2\">%s</td>\n"
		"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%d</td>\n"
		"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"     
    "<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"),
		pBss->p2pdevname, pBss->ChannelNumber, tmpBuf, p2pTypeStr);

		nBytesSent += req_format_write(wp,("\
		<td align=center bgcolor=\"#C0C0C0\"> \
		<input type=\"radio\" name=""\"select\" value=\"sel%d\" onClick=\"p2pSelect(%d)\"> \
		</td></tr>\n\
		<input type=\"hidden\" id=\"selDeviceName_%d\" value=\"%s\" > \
		<input type=\"hidden\" id=\"selChannel_%d\" value=\"%d\" > \
		<input type=\"hidden\" id=\"selMacAddr_%d\" value=\"%s\" > \
		<input type=\"hidden\" id=\"selRole_%d\" value=\"%s\" > \
		<input type=\"hidden\" id=\"selWSCMethod_%d\" value=\"%d\" > \
		"),
		 i,i,i,pBss->p2pdevname,i,pBss->ChannelNumber,i,tmpBuf2,i,p2pTypeStr,i,pBss->p2pwscconfig);
		
	}

	if( pStatus->number == 0 )
	{
		nBytesSent += req_format_write(wp, "<tr>"
    "<td align=center bgcolor=\"#C0C0C0\"><pre><font size=\"2\">None</td>\n"
    "<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"
    "<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"
    "<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"
    "<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"
    "</tr>\n");
	}
	return nBytesSent;
}


void p2p_dhcp_process(void)
{
	int Ret = 0;
	Ret = getClientConnectState();		
	if(Ret == P2P_S_CLIENT_CONNECTED_DHCPC)
	{

		P2P_DEBUG("Web server start udhcpc !!!\n");
		set_lan_dhcpc("br0");
	
	}else if(Ret == P2P_S_preGO2GO_DHCPD){

		P2P_DEBUG("Web server start udhcpd !!!\n");	
		set_lan_dhcpd("br0", 2);
		
	}else if(Ret == P2P_S_back2dev){
	
		#if 1
			int valTmp = P2P_DEVICE;
			int valTmp2 = 0;
			int tmpint,tmpint2;
			
			unsigned char ssidstr[33];
			unsigned char syscmd[50];

			tmpint = wlan_idx ;
			tmpint2 = vwlan_idx ;		
			strcpy(ssidstr,"");

			vwlan_idx = 0;
			wlan_idx=0;
			
			apmib_set( MIB_WLAN_P2P_TYPE, (void *)&valTmp);		
			apmib_set( MIB_WLAN_SSID, (void *)ssidstr);
			apmib_set( MIB_WLAN_WPA_PSK, (void *)ssidstr);		
			apmib_set( MIB_WLAN_AUTH_TYPE, (void *)&valTmp2);
			apmib_set( MIB_WLAN_ENCRYPT, (void *)&valTmp2);
			apmib_set( MIB_WLAN_WPA_CIPHER_SUITE, (void *)&valTmp2);
			apmib_set( MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&valTmp2);
			apmib_set( MIB_WLAN_WSC_CONFIGURED, (void *)&valTmp2);		
			apmib_update_web(CURRENT_SETTING);	// update to flash					
			wlan_idx = tmpint ;		
			vwlan_idx = tmpint2 ;



			sprintf(syscmd,"iwpriv wlan0 p2pcmd bakdev");
			//printf("%s %d : %s\n",__FUNCTION__,__LINE__,syscmd);
			system(syscmd);	

		#else		
		system("flash set WLAN0_P2P_TYPE 1");
		system("flash set WLAN0_SSID \"\"");
		system("flash set WLAN0_AUTH_TYPE 0");
		system("flash set WLAN0_ENCRYPT 0");
		system("flash set WLAN0_WSC_CONFIGURED 0");		
		system("init.sh gw all");		
		#endif	
	}
	
	return ;
}


#endif // #if defined(CONFIG_RTL_P2P_SUPPORT)

void getWlProfileInfo(request *wp, int argc, char **argv)
{
#if defined(WLAN_PROFILE)
	char	*name;
	int idx;
	int profile_enabled_id, profile_num_id, profile_tbl_id;
	int entryNum;
	WLAN_PROFILE_T entry;
	int wlProfileEnabled;

	name = argv[0];
	idx = atoi(argv[1]);
	
	if (name == NULL) {
   		fprintf(stderr, "Insufficient args\n");
   		return -1;
   	}

	if(wlan_idx == 0)
	{
		profile_enabled_id = MIB_PROFILE_ENABLED1;
		profile_num_id = MIB_PROFILE_NUM1;
		profile_tbl_id = MIB_PROFILE_TBL1;
	}
	else
	{
		profile_enabled_id = MIB_PROFILE_ENABLED2;
		profile_num_id = MIB_PROFILE_NUM2;
		profile_tbl_id = MIB_PROFILE_TBL2;
	}

	apmib_get( profile_enabled_id, (void *)&wlProfileEnabled);
	if(wlProfileEnabled == 0)
	{
		req_format_write(wp,"%s","0");
		return 0;
	}
	
	apmib_get(profile_num_id, (void *)&entryNum);
	if(idx > entryNum)
	{
		req_format_write(wp,"%s","0");
		return 0;
	}
	
	*((char *)&entry) = (char)idx;
	if ( !apmib_get(profile_tbl_id, (void *)&entry))
	{
		req_format_write(wp,"%s","0");
		return 0;
	}
		
//////MENU//////////////////////////////////////////////////
	if(!strcmp(name,"wlProfileTblEnable"))
	{
		req_format_write(wp,"%d",1);
		return 0;
	}
	else if(!strcmp(name,"wlProfileSSID"))
	{
		req_format_write(wp,"%s",entry.ssid);
		return 0;
	}
	
#endif //#if defined(WLAN_PROFILE)
	
	return 0;
}

