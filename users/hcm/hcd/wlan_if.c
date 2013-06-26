/*
  *   Module to access wlan driver
  *
  *	Copyright (C)2008, Realtek Semiconductor Corp. All rights reserved.
  *
  *	$Id: wlan_if.c,v 1.27 2011/02/14 07:50:28 marklee Exp $
  */


/*================================================================*/
/* System Include Files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <netinet/in.h>
#include <linux/if_ether.h> 
#include <linux/wireless.h>
/*================================================================*/
/* Local Include Files */

#include "hcd.h"
#include "mib.h"
#include "wlan_if.h"

#ifdef _KERNEL_LINUX_26
#include "../../../linux-2.6.30/drivers/net/wireless/rtl8192cd/ieee802_mib.h"
#else
#include "../../../linux-2.4.18/drivers/net/rtl8192cd/ieee802_mib.h" 
#endif

#define LOCAL_ADMIN_BIT 0x02

#define NULL_FILE 0
#define NULL_STR ""
#define IWCONTROL_PID_FILE "/var/run/iwcontrol.pid"
#define IAPP_PID_FILE "/var/run/iapp.pid"
#define WPS_CONFIG_IN "/etc/wscd.conf"
#define WPS_CONFIG_OUT "/var/wsc.conf"

#define WRITE_WSC_PARAM(dst, tmp, str, val) {	\
	sprintf(tmp, str, val); \
	memcpy(dst, tmp, strlen(tmp)); \
	dst += strlen(tmp); \
}

static void __inline__ WRITE_WPA_FILE(int fh, unsigned char *buf)
{
	if ( write(fh, buf, strlen(buf)) != strlen(buf) ) {
		printf("Write WPA config file error!\n");
		close(fh);
		exit(1);
	}
}


enum { 
	MODE_AP_UNCONFIG=1, 			// AP unconfigured (enrollee)
	MODE_CLIENT_UNCONFIG=2, 		// client unconfigured (enrollee) 
	MODE_CLIENT_CONFIG=3,			// client configured (registrar) 
	MODE_AP_PROXY=4, 			// AP configured (proxy)
	MODE_AP_PROXY_REGISTRAR=5,		// AP configured (proxy and registrar)
	MODE_CLIENT_UNCONFIG_REGISTRAR=6		// client unconfigured (registrar) 
};

/*================================================================*/
/* Constant Definitions */
//extern struct wlan_config_mib wlan_mib;
extern struct config_mib_all mib_all;

typedef struct bss_info {
    unsigned char bssid[8];	// first 6 bytes indicate as BSS ID/AP mac address, last 2 bytes are padding data
    unsigned char ssid[34];	// SSID in string
    unsigned short channel;
    unsigned char network;	// network type, bit-mask value: 
                            // 1 ¡V 11b, 2 ¡V 11g, 4 ¡V 11a-legacy, 8 ¡V 11n, 16 ¡V 11a-n
    unsigned char type;	// AP or Ad-hoc. 0 - AP, 1 - Ad-hoc
    unsigned char encrypt;// encryption type. 0 ¡V open, 1 ¡V WEP, 2 ¡V WPA, 3 ¡VWPA2, 
                            // 4 ¡VMixed Mode (WPA+WPA2)
    unsigned char rssi;		// received signal strength. 1-100
} bss_info;

/*================================================================*/

static inline int iw_get_ext(int skfd, /* Socket to the kernel */
                             char *ifname,         /* Device name */
                             int  request,        /* WE ID */
                             struct iwreq *       pwrq)           /* Fixed part of the request */
{
         /* Set device name */
         strncpy(pwrq->ifr_name, ifname, IFNAMSIZ);
            /* Do the request */
            return(ioctl(skfd, request, pwrq));
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

void bring_down_wlan(void)
{
	int index=0, pid=-1;
	unsigned char tmpbuf[100]={0}, strPID[10]={0};

	for( index=0; index< MAX_WLAN_INTF - 1; index++)
	{
		if( !mib_all.wlan[index+1].legacy_flash_settings.wlanDisabled ) {
			memset(tmpbuf,0,100);
			DEL_BR_INTERFACE(IF_BR, mib_all.wlan[index+1].name, 0,1);
		}
	}

	//disable root interface
	DEL_BR_INTERFACE(IF_BR, mib_all.wlan[0].name, 0,1);
}

void bridge_all_wlan()
{
   unsigned char tmpbuf[100]={0}, mac[6]={0};
   int index;
    for( index=0; index< MAX_WLAN_INTF; index++)				
		ADD_BR_INTERFACE(IF_BR, mib_all.wlan[index].name, ALL_ZERO_MAC_ADDR,1,0);
}


int get_root_mac(char *mac)
{
	int fd;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	ifr.ifr_addr.sa_family = AF_INET;
	strcpy(ifr.ifr_name, "wlan0");
	if( ioctl(fd, SIOCGIFHWADDR, &ifr) < 0 )
		return -1;

	close(fd);
	memcpy(mac,ifr.ifr_hwaddr.sa_data,6);
}

void calc_incr(unsigned char *mac, int idx)
{
	if( (*mac+idx) == 0x0 )
		calc_incr(mac-1,1);
	else
		*mac += idx;
}

int generate_auth_conf(CONFIG_WLAN_SETTING_Tp wlan_mib, unsigned char *outputFile, int isWds)
{
	int fh, intVal, encrypt, enable1x, wep;
	unsigned char buf1[1024], buf2[1024];
	
#if 0
//#ifdef UNIVERSAL_REPEATER 
	int isVxd = 0;
	
	if (strstr(outputFile, "-vxd")) 
		isVxd = 1;	
#endif		
	
	fh = open(outputFile, O_RDWR|O_CREAT|O_TRUNC);
	if (fh == -1) {
		printf("Create WPA config file error!\n");
		return -1;
	}
	if (!isWds) {
		//apmib_get( MIB_WLAN_ENCRYPT, (void *)&encrypt);
		encrypt = wlan_mib->encrypt;
#if 0
//#ifdef UNIVERSAL_REPEATER
	if (isVxd && (encrypt == ENCRYPT_WPA2_MIXED)) {
		apmib_get( MIB_WLAN_MODE, (void *)&intVal);
		if (intVal == AP_MODE || intVal == AP_WDS_MODE) 
			encrypt = ENCRYPT_WPA;		
	}
#endif			
	
	sprintf(buf2, "encryption = %d\n", encrypt);
	WRITE_WPA_FILE(fh, buf2);

#if 0
//#ifdef UNIVERSAL_REPEATER
	if (isVxd) {
		if (strstr(outputFile, "wlan0-vxd"))
			apmib_get( MIB_REPEATER_SSID1, (void *)buf1);		
		else			
			apmib_get( MIB_REPEATER_SSID2, (void *)buf1);	
	}
	else
#endif
	//apmib_get( MIB_WLAN_SSID,  (void *)buf1);
	sprintf(buf2, "ssid = \"%s\"\n", wlan_mib->ssid);
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_ENABLE_1X, (void *)&enable1x);
	enable1x = wlan_mib->enable1X;
	sprintf(buf2, "enable1x = %d\n", enable1x);
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_ENABLE_MAC_AUTH, (void *)&intVal);
	sprintf(buf2, "enableMacAuth = %d\n", wlan_mib->macAuthEnabled);
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_ENABLE_SUPP_NONWPA, (void *)&intVal);
	intVal = wlan_mib->enableSuppNonWpa;
	if (intVal)
		intVal = wlan_mib->suppNonWpa;
		//apmib_get( MIB_WLAN_SUPP_NONWPA, (void *)&intVal);

	sprintf(buf2, "supportNonWpaClient = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_WEP, (void *)&wep);
	wep = wlan_mib->wep;
	sprintf(buf2, "wepKey = %d\n", wep);
	WRITE_WPA_FILE(fh, buf2);

	if ( encrypt==1 && enable1x ) {
		if (wep == 1) {
			//apmib_get( MIB_WLAN_WEP64_KEY1, (void *)buf1);
			memcpy(buf1,wlan_mib->wep64Key1,5);
			sprintf(buf2, "wepGroupKey = \"%02x%02x%02x%02x%02x\"\n", buf1[0],buf1[1],buf1[2],buf1[3],buf1[4]);
		}
		else {
			//apmib_get( MIB_WLAN_WEP128_KEY1, (void *)buf1);
			memcpy(buf1,wlan_mib->wep128Key1,12);
			sprintf(buf2, "wepGroupKey = \"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\"\n",
				buf1[0],buf1[1],buf1[2],buf1[3],buf1[4],
				buf1[5],buf1[6],buf1[7],buf1[8],buf1[9],
				buf1[10],buf1[11],buf1[12]);
		}
	}
	else
		strcpy(buf2, "wepGroupKey = \"\"\n");
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_WPA_AUTH, (void *)&intVal);
	sprintf(buf2, "authentication = %d\n", wlan_mib->wpaAuth);
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_WPA_CIPHER_SUITE, (void *)&intVal);
	sprintf(buf2, "unicastCipher = %d\n", wlan_mib->wpaCipher);
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&intVal);
	sprintf(buf2, "wpa2UnicastCipher = %d\n", wlan_mib->wpa2Cipher);
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_WPA2_PRE_AUTH, (void *)&intVal);
	sprintf(buf2, "enablePreAuth = %d\n", wlan_mib->wpa2PreAuth);
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_WPA_PSK_FORMAT, (void *)&intVal);
	if (wlan_mib->wpaPSKFormat == 0)
		sprintf(buf2, "usePassphrase = 1\n");
	else
		sprintf(buf2, "usePassphrase = 0\n");
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_WPA_PSK, (void *)buf1);
	sprintf(buf2, "psk = \"%s\"\n", wlan_mib->wpaPSK);
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_WPA_GROUP_REKEY_TIME, (void *)&intVal);
	sprintf(buf2, "groupRekeyTime = %d\n", wlan_mib->wpaGroupRekeyTime);
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_RS_PORT, (void *)&intVal);
	sprintf(buf2, "rsPort = %d\n", wlan_mib->rsPort);
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_RS_IP, (void *)buf1);
	sprintf(buf2, "rsIP = %s\n", inet_ntoa(*((struct in_addr *)&(wlan_mib->rsIpAddr))));
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_RS_PASSWORD, (void *)buf1);
	sprintf(buf2, "rsPassword = \"%s\"\n", wlan_mib->rsPassword);
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_RS_RETRY, (void *)&intVal);
	//sprintf(buf2, "rsMaxReq = %d\n", wlan_mib->rsMaxRetry);
	sprintf(buf2, "rsMaxReq = %d\n", 3);
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_RS_INTERVAL_TIME, (void *)&intVal);
	//sprintf(buf2, "rsAWhile = %d\n", wlan_mib->rsIntervalTime);
	sprintf(buf2, "rsAWhile = %d\n", 5);
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_ACCOUNT_RS_ENABLED, (void *)&intVal);
	sprintf(buf2, "accountRsEnabled = %d\n", wlan_mib->accountRsEnabled);
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_ACCOUNT_RS_PORT, (void *)&intVal);
	sprintf(buf2, "accountRsPort = %d\n", wlan_mib->accountRsPort);
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_ACCOUNT_RS_IP, (void *)buf1);
	sprintf(buf2, "accountRsIP = %s\n", inet_ntoa(*((struct in_addr *)&(wlan_mib->accountRsIpAddr))));
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_ACCOUNT_RS_PASSWORD, (void *)buf1);
	sprintf(buf2, "accountRsPassword = \"%s\"\n", wlan_mib->accountRsPassword);
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_ACCOUNT_UPDATE_ENABLED, (void *)&intVal);
	sprintf(buf2, "accountRsUpdateEnabled = %d\n", wlan_mib->accountRsUpdateEnabled);
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_ACCOUNT_UPDATE_DELAY, (void *)&intVal);
	sprintf(buf2, "accountRsUpdateTime = %d\n", wlan_mib->accountRsUpdateDelay);
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_ACCOUNT_RS_RETRY, (void *)&intVal);
	sprintf(buf2, "accountRsMaxReq = %d\n", wlan_mib->accountRsMaxRetry);
	WRITE_WPA_FILE(fh, buf2);

	//apmib_get( MIB_WLAN_ACCOUNT_RS_INTERVAL_TIME, (void *)&intVal);
	sprintf(buf2, "accountRsAWhile = %d\n", wlan_mib->accountRsIntervalTime);
	WRITE_WPA_FILE(fh, buf2);
	} else {
		//apmib_get( MIB_WLAN_WDS_ENCRYPT, (void *)&encrypt);
		encrypt = wlan_mib->wdsEncrypt;
		if (encrypt == WDS_ENCRYPT_TKIP)		
			encrypt = ENCRYPT_WPA;
		else if (encrypt == WDS_ENCRYPT_AES)		
			encrypt = ENCRYPT_WPA2; 	
		else
			encrypt = 0;
	
		sprintf(buf2, "encryption = %d\n", encrypt);
		WRITE_WPA_FILE(fh, buf2);
		WRITE_WPA_FILE(fh, "ssid = \"REALTEK\"\n");
		WRITE_WPA_FILE(fh, "enable1x = 1\n");
		WRITE_WPA_FILE(fh, "enableMacAuth = 0\n");
		WRITE_WPA_FILE(fh, "supportNonWpaClient = 0\n");
		WRITE_WPA_FILE(fh, "wepKey = 0\n");
		WRITE_WPA_FILE(fh,	"wepGroupKey = \"\"\n");
		WRITE_WPA_FILE(fh,	"authentication = 2\n");

		if (encrypt == ENCRYPT_WPA)
			intVal = WPA_CIPHER_TKIP;
		else
			intVal = WPA_CIPHER_AES;
			
		sprintf(buf2, "unicastCipher = %d\n", intVal);
		WRITE_WPA_FILE(fh, buf2);

		sprintf(buf2, "wpa2UnicastCipher = %d\n", intVal);
		WRITE_WPA_FILE(fh, buf2);

		WRITE_WPA_FILE(fh, "enablePreAuth = 0\n");

		//apmib_get( MIB_WLAN_WDS_PSK_FORMAT, (void *)&intVal);
		if (wlan_mib->wdsPskFormat==0)
			sprintf(buf2, "usePassphrase = 1\n");
		else
			sprintf(buf2, "usePassphrase = 0\n");
		WRITE_WPA_FILE(fh, buf2);

		//apmib_get( MIB_WLAN_WDS_PSK, (void *)buf1);
		sprintf(buf2, "psk = \"%s\"\n", wlan_mib->wdsPsk);
		WRITE_WPA_FILE(fh, buf2);

		WRITE_WPA_FILE(fh, "groupRekeyTime = 0\n");
		WRITE_WPA_FILE(fh, "rsPort = 1812\n");
		WRITE_WPA_FILE(fh, "rsIP = 192.168.1.1\n");
		WRITE_WPA_FILE(fh, "rsPassword = \"\"\n");
		WRITE_WPA_FILE(fh, "rsMaxReq = 3\n");
		WRITE_WPA_FILE(fh, "rsAWhile = 10\n");
		WRITE_WPA_FILE(fh, "accountRsEnabled = 0\n");
		WRITE_WPA_FILE(fh, "accountRsPort = 1813\n");
		WRITE_WPA_FILE(fh, "accountRsIP = 192.168.1.1\n");
		WRITE_WPA_FILE(fh, "accountRsPassword = \"\"\n");
		WRITE_WPA_FILE(fh, "accountRsUpdateEnabled = 0\n");
		WRITE_WPA_FILE(fh, "accountRsUpdateTime = 1000\n");
		WRITE_WPA_FILE(fh, "accountRsMaxReq = 3\n");
		WRITE_WPA_FILE(fh, "accountRsAWhile = 1\n");
	}

	sprintf(buf2, "hostmac = %s\n", mib_all.sys_config.host_mac);
	WRITE_WPA_FILE(fh, buf2);

	close(fh);
	return 0;
}

void bring_up_wlan(void)
{
	int index=0;
	unsigned char tmpbuf[100]={0}, mac[6]={0}, intf[100]={"\0"}, conf_name[30]={"\0"};
	unsigned char root_mac[6]={0};

	if( !mib_all.wlan[0].legacy_flash_settings.wlanDisabled ) {
		if( mib_all.sys_config.enable_efuse_config ) {
			ADD_BR_INTERFACE(IF_BR, mib_all.wlan[0].name, ALL_ZERO_MAC_ADDR,0,1);
			if( get_root_mac(root_mac) < 0 ) {
				printf("[%s]ioctl get interface MAC failed.\n");
				return;
			}
		} else {
			string_to_mac(root_mac,mib_all.wlan[0].legacy_flash_settings.wlanMacAddr,6);
			ADD_BR_INTERFACE(IF_BR, mib_all.wlan[0].name, root_mac,0,1);
		}

		//mark_vxd ,if up with root mac
		if(mib_all.sys_config.mode == REPEATER_AP_MODE ) // mark_issue , to do (client ?)
		{
			ADD_BR_INTERFACE(IF_BR, mib_all.wlan[WLAN_VXD_INDEX].name, root_mac,0,1);
		}
		

		for( index=1; index< (get_vap_num()+1); index++)
		{
			if( !mib_all.wlan[index].legacy_flash_settings.wlanDisabled ) {
				if( mib_all.sys_config.enable_efuse_config ) {
					mac[0] = LOCAL_ADMIN_BIT;
					calc_incr((char *)mac+6-1,index);

				} else {
					memset(tmpbuf,0,100);
					memset(mac,0,6);
					string_to_mac(mac,mib_all.wlan[index].legacy_flash_settings.wlanMacAddr,6); 
				}
				ADD_BR_INTERFACE(IF_BR, mib_all.wlan[index].name, mac,0,1);
			}
		}
	}
}

int get_wlan_bssinfo(char *interface, WLAN_BSS_INFO_Tp pInfo)
{

    int skfd=0;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;
    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0)
      /* If no wireless name : no wireless extensions */
      {
      	 close( skfd );
        return -1;
      }

    wrq.u.data.pointer = (caddr_t)pInfo;
    wrq.u.data.length = sizeof(bss_info);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLGETBSSINFO, &wrq) < 0){
    	 close( skfd );
	return -1;
	}
    close( skfd );

    return 0;
}

int get_wlan_stanum( char *interface, int *num )
{
    int skfd=0;
    unsigned short staNum;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;
    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
      /* If no wireless name : no wireless extensions */
      close( skfd );
      return -1;
	}
    wrq.u.data.pointer = (caddr_t)&staNum;
    wrq.u.data.length = sizeof(staNum);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLSTANUM, &wrq) < 0){
    	 close( skfd );
	return -1;
	}
    *num  = (int)staNum;

    close( skfd );

    return 0;
}

int get_wlan_stainfo( char *interface,  struct wlan_sta_info  *pInfo )
{

    int skfd=0;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;
    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
      /* If no wireless name : no wireless extensions */
      close( skfd );
        return -1;
	}
    wrq.u.data.pointer = (caddr_t)pInfo;
    wrq.u.data.length = sizeof(struct wlan_sta_info) * (MAX_STA_NUM+1);
    memset(pInfo, 0, sizeof(struct wlan_sta_info) * (MAX_STA_NUM+1));

    if (iw_get_ext(skfd, interface, SIOCGIWRTLSTAINFO, &wrq) < 0){
    	close( skfd );
		return -1;
	}
    close( skfd );

    return 0;
}

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

static void convert_bin_to_str(unsigned char *bin, int len, char *out)
{
	int i;
	char tmpbuf[10];

	out[0] = '\0';

	for (i=0; i<len; i++) {
		sprintf(tmpbuf, "%02x", bin[i]);
		strcat(out, tmpbuf);
	}
}

int generate_wps_conf(unsigned char *in, unsigned char *out, CONFIG_WLAN_SETTING_Tp wlan_mib)
{
	int fh;
	struct stat status;
	char *buf, *ptr;
	int intVal, intVal2, is_client, is_config, is_registrar, len, is_wep=0, wep_key_type=0,
		wep_transmit_key=0, encrypt1=0, encrypt2=0, auth=0, shared_type=0, iVal=0;
	char tmpbuf[100], tmp1[100];
	HW_SETTING_Tp hw_config = fecth_hwmib();
		
	//apmib_get(MIB_WSC_PIN, (void *)tmpbuf);
	memcpy(tmpbuf,hw_config->wlan[0].wscPin,PIN_LEN);
	//if (genpin || !memcmp(tmpbuf, "\x0\x0\x0\x0\x0\x0\x0\x0", PIN_LEN)) {
	if ( !memcmp(tmpbuf, "\x0\x0\x0\x0\x0\x0\x0\x0", PIN_LEN)) {
	#include <sys/time.h>			
		struct timeval tod;
		unsigned long num;
		
		gettimeofday(&tod , NULL);

		//apmib_get(MIB_HW_NIC0_ADDR, (void *)&tmp1); 		
		memcpy(tmp1,hw_config->nic0Addr,6);
		tod.tv_sec += tmp1[4]+tmp1[5];		
		srand(tod.tv_sec);
		num = rand() % 10000000;
		num = num*10 + compute_pin_checksum(num);
		convert_hex_to_ascii((unsigned long)num, tmpbuf);

		//apmib_set(MIB_WSC_PIN, (void *)tmpbuf);
		update_wps_pincode(tmpbuf);
//		apmib_update(CURRENT_SETTING);		
		//apmib_update(HW_SETTING);	//brian, shoud br rewrite

		printf("Generated PIN = %s\n", tmpbuf);

//		if (genpin)
//			return 0;
	}

	if (stat(in, &status) < 0) {
		printf("stat() error [%s]!\n", in);
		return -1;
	}

	buf = malloc(status.st_size+2048);
	if (buf == NULL) {
		printf("malloc() error [%d]!\n", (int)status.st_size+2048);
		return -1;		
	}

	ptr = buf;
	is_client = wlan_mib->wlanMode;
	is_config = wlan_mib->wscConfigured;
	is_registrar = wlan_mib->wscRegistrarEnabled;

	if (is_client == CLIENT_MODE) {
		{
			if (is_registrar) {
				if (!is_config)
					intVal = MODE_CLIENT_UNCONFIG_REGISTRAR;
				else
					intVal = MODE_CLIENT_CONFIG;			
			}
			else
				intVal = MODE_CLIENT_UNCONFIG;
		}
	}
	else {
		if (!is_config)
			intVal = MODE_AP_UNCONFIG;
		else
			intVal = MODE_AP_PROXY_REGISTRAR;
	}
	WRITE_WSC_PARAM(ptr, tmpbuf, "mode = %d\n", intVal);
	WRITE_WSC_PARAM(ptr, tmpbuf, "upnp = %d\n", wlan_mib->wscUpnpEnabled);

	//apmib_get(MIB_WSC_METHOD, (void *)&intVal);
	intVal = wlan_mib->wscMethod;
	//Ethernet(0x2)+Label(0x4)+PushButton(0x80) Bitwise OR
	if (intVal == 1) //Pin+Ethernet
		intVal = (CONFIG_METHOD_ETH | CONFIG_METHOD_PIN);
	else if (intVal == 2) //PBC+Ethernet
		intVal = (CONFIG_METHOD_ETH | CONFIG_METHOD_PBC);
	if (intVal == 3) //Pin+PBC+Ethernet
		intVal = (CONFIG_METHOD_ETH | CONFIG_METHOD_PIN | CONFIG_METHOD_PBC);
	WRITE_WSC_PARAM(ptr, tmpbuf, "config_method = %d\n", intVal);

	encrypt1 = wlan_mib->encrypt;
	if (encrypt1 == ENCRYPT_DISABLED) {
		auth = WSC_AUTH_OPEN;
		encrypt2 = WSC_ENCRYPT_NONE;
	}
	else if (encrypt1 == ENCRYPT_WEP) {
		shared_type = wlan_mib->authType;
		//apmib_get(MIB_WLAN_AUTH_TYPE, (void *)&shared_type);
		if (shared_type == AUTH_OPEN || shared_type == AUTH_BOTH)
			auth = WSC_AUTH_OPEN;
		else
			auth = WSC_AUTH_SHARED;
		encrypt2 = WSC_ENCRYPT_WEP;		
	}
	else if (encrypt1 == ENCRYPT_WPA) {
		auth = WSC_AUTH_WPAPSK;
		//apmib_get(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&encrypt1);
		encrypt1 = wlan_mib->wpaCipher;
		if (encrypt1 == WPA_CIPHER_TKIP)
			encrypt2 = WSC_ENCRYPT_TKIP;		
		else if (encrypt1 == WPA_CIPHER_AES)
			encrypt2 = WSC_ENCRYPT_AES;		
		else 
			encrypt2 = WSC_ENCRYPT_TKIPAES;				
	}
	else if (encrypt1 == ENCRYPT_WPA2) {
		auth = WSC_AUTH_WPA2PSK;
		//apmib_get(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&encrypt1);
		encrypt1 = wlan_mib->wpa2Cipher;
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
		//apmib_get(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&encrypt1);
		encrypt1 = wlan_mib->wpaCipher;
		//apmib_get(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&iVal);
		iVal = wlan_mib->wpa2Cipher;
		if (!(iVal & WPA_CIPHER_AES)) {
			if (encrypt1 &	WPA_CIPHER_AES) {			
				auth = WSC_AUTH_WPAPSK;
				encrypt2 = WSC_ENCRYPT_AES;	
			}
			else
				encrypt2 = WSC_ENCRYPT_TKIP;	
		}
//-------------------------------------------- david+2008-01-03

	}
	//apmib_set(MIB_WSC_AUTH, (void *)&auth);
	wlan_mib->wscAuth = auth;
	//apmib_set(MIB_WSC_ENC, (void *)&encrypt2);
	wlan_mib->wscEnc = encrypt2;

	//apmib_get(MIB_WSC_AUTH, (void *)&intVal2);
	WRITE_WSC_PARAM(ptr, tmpbuf, "auth_type = %d\n", wlan_mib->wscAuth);

	//apmib_get(MIB_WSC_ENC, (void *)&intVal);
	WRITE_WSC_PARAM(ptr, tmpbuf, "encrypt_type = %d\n", wlan_mib->wscEnc);

	if (wlan_mib->wscEnc == WSC_ENCRYPT_WEP)
		is_wep = 1;

	if (is_client) {
		{
			//apmib_get(MIB_WLAN_NETWORK_TYPE, (void *)&intVal);
			intVal = wlan_mib->networkType;
			if (intVal == 0)
				intVal = 1;
			else
				intVal = 2;
		}
	}
	else
		intVal = 1;
	
	WRITE_WSC_PARAM(ptr, tmpbuf, "connection_type = %d\n", intVal);

	//apmib_get(MIB_WSC_MANUAL_ENABLED, (void *)&intVal);
	WRITE_WSC_PARAM(ptr, tmpbuf, "manual_config = %d\n", intVal);

	if (is_wep) { // only allow WEP in none-MANUAL mode (configured by external registrar)
		//apmib_get(MIB_WLAN_ENCRYPT, (void *)&intVal);
		intVal = wlan_mib->encrypt;
		if (intVal != ENCRYPT_WEP) {
			printf("WEP mismatched between WPS and host system\n");
			free(buf);
			return -1;
		}
		//apmib_get(MIB_WLAN_WEP, (void *)&intVal);
		intVal = wlan_mib->wep;
		if (intVal <= WEP_DISABLED || intVal > WEP128) {
			printf("WEP encrypt length error\n");
			free(buf);
			return -1;
		}
		//apmib_get(MIB_WLAN_WEP_KEY_TYPE, (void *)&wep_key_type);
		//apmib_get(MIB_WLAN_WEP_DEFAULT_KEY, (void *)&wep_transmit_key);
		wep_key_type = wlan_mib->wepKeyType;
		wep_transmit_key = wlan_mib->wepDefaultKey;
		wep_transmit_key++;
		WRITE_WSC_PARAM(ptr, tmpbuf, "wep_transmit_key = %d\n", wep_transmit_key);
		if (intVal == WEP64) {
			//apmib_get(MIB_WLAN_WEP64_KEY1, (void *)&tmpbuf);
			memcpy(tmpbuf,wlan_mib->wep64Key1,5);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 5);
				tmp1[5] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 5, tmp1);
				tmp1[10] = '\0';
			}			
			WRITE_WSC_PARAM(ptr, tmpbuf, "network_key = %s\n", tmp1);

			//apmib_get(MIB_WLAN_WEP64_KEY2, (void *)&tmpbuf);
			memcpy(tmpbuf,wlan_mib->wep64Key2,5);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 5);
				tmp1[5] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 5, tmp1);
				tmp1[10] = '\0';
			}			
			WRITE_WSC_PARAM(ptr, tmpbuf, "wep_key2 = %s\n", tmp1);

			//apmib_get(MIB_WLAN_WEP64_KEY3, (void *)&tmpbuf);
			memcpy(tmpbuf,wlan_mib->wep64Key3,5);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 5);
				tmp1[5] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 5, tmp1);
				tmp1[10] = '\0';
			}			
			WRITE_WSC_PARAM(ptr, tmpbuf, "wep_key3 = %s\n", tmp1);


			//apmib_get(MIB_WLAN_WEP64_KEY4, (void *)&tmpbuf);
			memcpy(tmpbuf,wlan_mib->wep64Key4,5);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 5);
				tmp1[5] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 5, tmp1);
				tmp1[10] = '\0';
			}			
			WRITE_WSC_PARAM(ptr, tmpbuf, "wep_key4 = %s\n", tmp1);
		}
		else {
			//apmib_get(MIB_WLAN_WEP128_KEY1, (void *)&tmpbuf);
			memcpy(tmpbuf,wlan_mib->wep128Key1,5);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 13);
				tmp1[13] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 13, tmp1);
				tmp1[26] = '\0';
			}
			WRITE_WSC_PARAM(ptr, tmpbuf, "network_key = %s\n", tmp1);

			//apmib_get(MIB_WLAN_WEP128_KEY2, (void *)&tmpbuf);
			memcpy(tmpbuf,wlan_mib->wep128Key2,5);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 13);
				tmp1[13] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 13, tmp1);
				tmp1[26] = '\0';
			}
			WRITE_WSC_PARAM(ptr, tmpbuf, "wep_key2 = %s\n", tmp1);

			//apmib_get(MIB_WLAN_WEP128_KEY3, (void *)&tmpbuf);
			memcpy(tmpbuf,wlan_mib->wep128Key3,5);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 13);
				tmp1[13] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 13, tmp1);
				tmp1[26] = '\0';
			}
			WRITE_WSC_PARAM(ptr, tmpbuf, "wep_key3 = %s\n", tmp1);

			//apmib_get(MIB_WLAN_WEP128_KEY4, (void *)&tmpbuf);
			memcpy(tmpbuf,wlan_mib->wep128Key4,5);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 13);
				tmp1[13] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 13, tmp1);
				tmp1[26] = '\0';
			}
			WRITE_WSC_PARAM(ptr, tmpbuf, "wep_key4 = %s\n", tmp1);
		}
	}
	else {
		//apmib_get(MIB_WLAN_WPA_PSK, (void *)&tmp1);
		strcpy(tmp1,wlan_mib->wpaPSK);
		WRITE_WSC_PARAM(ptr, tmpbuf, "network_key = %s\n", tmp1);
	}

	//apmib_get(MIB_WLAN_SSID, (void *)&tmp1);	
	strcpy(tmp1,wlan_mib->ssid);
	WRITE_WSC_PARAM(ptr, tmpbuf, "ssid = %s\n", tmp1);

#if 0	
//	}
//	else {			
		apmib_get(MIB_WSC_PSK, (void *)&tmp1);
		WRITE_WSC_PARAM(ptr, tmpbuf, "network_key = %s\n", tmp1);		
		
		apmib_get(MIB_WSC_SSID, (void *)&tmp1);
		WRITE_WSC_PARAM(ptr, tmpbuf, "ssid = %s\n", tmp1);
//	}
#endif

	//apmib_get(MIB_WSC_PIN, (void *)&tmp1);
	memcpy(tmp1,hw_config->wlan[0].wscPin,PIN_LEN);
	tmp1[8] = '\0';
	WRITE_WSC_PARAM(ptr, tmpbuf, "pin_code = %s\n", tmp1);
	

	//apmib_get(MIB_WLAN_CHAN_NUM, (void *)&intVal);
	intVal = wlan_mib->channel;
	if (intVal > 14)
		intVal = 2;
	else
		intVal = 1;
	WRITE_WSC_PARAM(ptr, tmpbuf, "rf_band = %d\n", intVal);

/*
	apmib_get(MIB_HW_MODEL_NUM, (void *)&tmp1); 
	WRITE_WSC_PARAM(ptr, tmpbuf, "model_num = \"%s\"\n", tmp1); 

	apmib_get(MIB_HW_SERIAL_NUM, (void *)&tmp1);	
	WRITE_WSC_PARAM(ptr, tmpbuf, "serial_num = \"%s\"\n", tmp1);	
*/
	//apmib_get(MIB_DEVICE_NAME, (void *)&tmp1);
	//strcpy(tmp1,pmib->deviceName);
	WRITE_WSC_PARAM(ptr, tmpbuf, "device_name = \"%s\"\n", "Realtek Wireless AP");

	//apmib_get(MIB_WSC_CONFIG_BY_EXT_REG, (void *)&intVal);
	intVal = wlan_mib->wscConfigByExtReg;
	WRITE_WSC_PARAM(ptr, tmpbuf, "config_by_ext_reg = %d\n", intVal);

	WRITE_WSC_PARAM(ptr, tmpbuf, "wlan_fifo0 = %s\n", "/var/wscd-wlan0.fifo");
	WRITE_WSC_PARAM(ptr, tmpbuf, "wlan_fifo1 = %s\n", "/var/wscd-wlan1.fifo");

	WRITE_WSC_PARAM(ptr, tmpbuf, "wlan0_wsc_disabled = %d\n", 0);
	WRITE_WSC_PARAM(ptr, tmpbuf, "wlan1_wsc_disabled = %d\n", 1);

	len = (int)(((long)ptr)-((long)buf));
	
	fh = open(in, O_RDONLY);
	if (fh == -1) {
		printf("open() error [%s]!\n", in);
		return -1;
	}

	lseek(fh, 0L, SEEK_SET);
	if (read(fh, ptr, status.st_size) != status.st_size) {		
		printf("read() error [%s]!\n", in);
		return -1;	
	}
	close(fh);

	// search UUID field, replace last 12 char with hw mac address
	ptr = strstr(ptr, "uuid =");
	if (ptr) {
		char tmp2[100];
		//apmib_get(MIB_HW_NIC0_ADDR, (void *)&tmp1); 
		memcpy(tmp1,hw_config->nic0Addr,6);
		convert_bin_to_str(tmp1, 6, tmp2);
		memcpy(ptr+27, tmp2, 12);		
	}

	fh = open(out, O_RDWR|O_CREAT|O_TRUNC);
	if (fh == -1) {
		printf("open() error [%s]!\n", out);
		return -1;
	}

	if (write(fh, buf, len+status.st_size) != len+status.st_size ) {
		printf("Write() file error [%s]!\n", out);
		return -1;
	}
	close(fh);
	free(buf);

	return 0;
}

int start_wps(void)
{
	struct stat status;
	//generate wps config
	if( !mib_all.wlan[0].legacy_flash_settings.wscDisable ){
		system("mkdir /var/wps");
		system("cp /etc/simplecfgservice.xml /var/wps");
		generate_wps_conf(WPS_CONFIG_IN,WPS_CONFIG_OUT,&mib_all.wlan[0].legacy_flash_settings);
		//start wscd
		system("wscd -start -c /var/wsc.conf -w wlan0 -fi /var/wscd-wlan0.fifo -daemon");
		
		while( stat("/var/wscd-wlan0.fifo",&status) < 0 ){
			sleep(1);
		}
		system("iwcontrol wlan0");
	}
}

int initWlan(int index, CONFIG_WLAN_SETTING_Tp wlan_config, HW_SETTING_Tp hw_config)
{
	struct wifi_mib *pmib;
	int i, intVal, intVal2, encrypt, enable1x, wep, mode/*, enable1xVxd*/;
	unsigned char buf1[1024], buf2[1024], mac[6];
	int skfd;
	struct iwreq wrq, wrq_root;
	int wlan_band=0, channel_bound=0, aggregation=0;
	MACFILTER_T *pAcl=NULL;
	struct wdsEntry *wds_Entry=NULL;
	WDS_Tp pwds_EntryUI;
#ifdef MBSSID
	int v_previous=0;
#ifdef CONFIG_RTL8196B
	int vap_enable=0, intVal4=0;
#endif
#endif
	unsigned char *ifname = mib_all.wlan[index].name;
	
	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(wrq.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(skfd, SIOCGIWNAME, &wrq) < 0) {
		printf("Interface open failed!\n");
		return -1;
	}

	if ((pmib = (struct wifi_mib *)malloc(sizeof(struct wifi_mib))) == NULL) {
		printf("MIB buffer allocation failed!\n");
		return -1;
	}

	// Disable WLAN MAC driver and shutdown interface first
	// shutdown all WDS interface
	// kill wlan application daemon
	// virtual interface	
	//sprintf(buf1, "wlan%d", wlan_idx);
	strncpy(wrq_root.ifr_name, "wlan0", IFNAMSIZ);  //mark_issue, now only support wlan0 root
	if (ioctl(skfd, SIOCGIWNAME, &wrq_root) < 0) {
  		printf("Root Interface open failed!\n");
		return -1;
	}			

	/*
	apmib_get(MIB_WLAN_DISABLED, (void *)&intVal);
	if (intVal == 1) {
	*/
	if ( wlan_config->wlanDisabled == 1) {
		free(pmib);
		close(skfd);		
		return 0;
	}

	// get mib from driver
	wrq.u.data.pointer = (caddr_t)pmib;
	wrq.u.data.length = sizeof(struct wifi_mib);

	if (index == 0) {
		if (ioctl(skfd, 0x8B42, &wrq) < 0) {
			printf("Get WLAN MIB failed!\n");
			return -1;
		}
	} else {
		wrq_root.u.data.pointer = (caddr_t)pmib;
		wrq_root.u.data.length = sizeof(struct wifi_mib);				
		if (ioctl(skfd, 0x8B42, &wrq_root) < 0) {
			printf("Get WLAN MIB failed!\n");
			return -1;
		}		
	}

	// check mib version
	//if not root interface, clone root mib to virtual interface

	// Set parameters to driver
	if (index == 0) {	
		/*
		apmib_get(MIB_HW_REG_DOMAIN, (void *)&intVal);
		pmib->dot11StationConfigEntry.dot11RegDomain = intVal;
		*/
		//pmib->dot11StationConfigEntry.dot11RegDomain = hw_config->wlan[0].regDomain;
		pmib->dot11StationConfigEntry.dot11RegDomain = mib_all.sys_config.regDomain;
	}
#if 0
	apmib_get(MIB_WLAN_WLAN_MAC_ADDR, (void *)mac);
	if (!memcmp(mac, "\x00\x00\x00\x00\x00\x00", 6)) {
#ifdef WLAN_MAC_FROM_EFUSE
        if( !get_root_mac(ifname,mac) ){
		if( vwlan_idx > 0 ) {
        				if (*(char *)(ifname+4) == '0' ) //wlan0
		        		  *(char *)mac |= LOCAL_ADMIN_BIT;
		        		else {
		        			(*(char *)mac) += 4; 
		        			(*(char *)mac) |= LOCAL_ADMIN_BIT;
		        		}
		}
                calc_incr((char *)mac+MACADDRLEN-1,vwlan_idx);
        } else {
        		apmib_get(MIB_HW_WLAN_ADDR, (void *)mac);
				return -1;
        }
#else
#ifdef MBSSID
		if (vwlan_idx > 0 && vwlan_idx != NUM_VWLAN_INTERFACE) {
			switch (vwlan_idx)
			{
				case 1:
					apmib_get(MIB_HW_WLAN_ADDR1, (void *)mac);
					break;
				case 2:
					apmib_get(MIB_HW_WLAN_ADDR2, (void *)mac);
					break;
				case 3:
					apmib_get(MIB_HW_WLAN_ADDR3, (void *)mac);
					break;
				case 4:
					apmib_get(MIB_HW_WLAN_ADDR4, (void *)mac);
					break;
				default:
					printf("Fail to get MAC address of VAP%d!\n", vwlan_idx-1);
					return 0;
			}
		}
		else
#endif
		apmib_get(MIB_HW_WLAN_ADDR, (void *)mac);
#endif	//WLAN_MAC_FROM_EFUSE
	}
#endif
	// ifconfig all wlan interface when not in WISP
	// ifconfig wlan1 later interface when in WISP mode, the wlan0  will be setup in WAN interface
	/* section 2
	apmib_get(MIB_OP_MODE, (void *)&intVal);
	apmib_get(MIB_WISP_WAN_ID, (void *)&intVal2);
	sprintf(buf1, "wlan%d", intVal2);
	if ((intVal != 2) ||
#ifdef MBSSID
		vwlan_idx > 0 ||
#endif
		strcmp(ifname, buf1)) {
		sprintf(buf2, "ifconfig %s hw ether %02x%02x%02x%02x%02x%02x", ifname, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		system(buf2);
		memcpy(&(pmib->dot11OperationEntry.hwaddr[0]), mac, 6);
	}
	section2, brian, removed for setup MAC in hcd.c and not consider gateway mode*/

	/* section3
#ifdef BR_SHORTCUT	
	if (intVal == 2
#ifdef MBSSID
		&& vwlan_idx == 0
#endif
	) 
		pmib->dot11OperationEntry.disable_brsc = 1;
#endif
	section3, brian, removed for bridge shortcut always enabled*/

	/*
	apmib_get(MIB_HW_LED_TYPE, (void *)&intVal);
	pmib->dot11OperationEntry.ledtype = intVal;
	*/
	pmib->dot11OperationEntry.ledtype = hw_config->wlan[0].ledType;

	// set AP/client/WDS mode
	//apmib_get(MIB_WLAN_SSID, (void *)buf1);
	strcpy(buf1,wlan_config->ssid);
	intVal2 = strlen(buf1);
	pmib->dot11StationConfigEntry.dot11DesiredSSIDLen = intVal2;
	memset(pmib->dot11StationConfigEntry.dot11DesiredSSID, 0, 32);
	memcpy(pmib->dot11StationConfigEntry.dot11DesiredSSID, buf1, intVal2);
	if ((pmib->dot11StationConfigEntry.dot11DesiredSSIDLen == 3) &&
		((pmib->dot11StationConfigEntry.dot11DesiredSSID[0] == 'A') || (pmib->dot11StationConfigEntry.dot11DesiredSSID[0] == 'a')) &&
		((pmib->dot11StationConfigEntry.dot11DesiredSSID[1] == 'N') || (pmib->dot11StationConfigEntry.dot11DesiredSSID[1] == 'n')) &&
		((pmib->dot11StationConfigEntry.dot11DesiredSSID[2] == 'Y') || (pmib->dot11StationConfigEntry.dot11DesiredSSID[2] == 'y'))) {
		pmib->dot11StationConfigEntry.dot11SSIDtoScanLen = 0;
		memset(pmib->dot11StationConfigEntry.dot11SSIDtoScan, 0, 32);
	}
	else {
		pmib->dot11StationConfigEntry.dot11SSIDtoScanLen = intVal2;
		memset(pmib->dot11StationConfigEntry.dot11SSIDtoScan, 0, 32);
		memcpy(pmib->dot11StationConfigEntry.dot11SSIDtoScan, buf1, intVal2);
	}

	
	//decide the mode from ifname + sys_mode
	mode = 0; //if is default AP mode
	if(index == WLAN_VXD_INDEX)
		if(mib_all.sys_config.mode == REPEATER_AP_MODE )
			mode = 1;  // vxd to client mode

	//mode = wlan_config->wlanMode;
	//-------below to map mode to wifi driver mode
	/*
	apmib_get(MIB_WLAN_MODE, (void *)&mode);	
	*/
	if( mode == 1 ) {
		// client mode
		/*
		apmib_get(MIB_WLAN_NETWORK_TYPE, (void *)&intVal2);
		if (intVal2 == 0)
		*/
		if (wlan_config->networkType == 0)
			pmib->dot11OperationEntry.opmode = 8;
		else {
			pmib->dot11OperationEntry.opmode = 32;
			//apmib_get(MIB_WLAN_DEFAULT_SSID, (void *)buf1);
			strcpy(buf1,wlan_config->defaultSsid);
			intVal2 = strlen(buf1);
			pmib->dot11StationConfigEntry.dot11DefaultSSIDLen = intVal2;
			memset(pmib->dot11StationConfigEntry.dot11DefaultSSID, 0, 32);
			memcpy(pmib->dot11StationConfigEntry.dot11DefaultSSID, buf1, intVal2);
		}
	}
	else
		pmib->dot11OperationEntry.opmode = 16;

	if (mode == 2)	// WDS only
		pmib->dot11WdsInfo.wdsPure = 1;
	else
		pmib->dot11WdsInfo.wdsPure = 0;


	//section-mac-filter ,//mark_mac
			
   	  pmib->dot11StationConfigEntry.dot11AclMode = wlan_config->acEnabled;
	  pmib->dot11StationConfigEntry.dot11AclNum = wlan_config->acNum;
 	  for(i=0;i<wlan_config->acNum;i++)
		memcpy(&(pmib->dot11StationConfigEntry.dot11AclAddr[i][0]), &(wlan_config->acAddrArray[i].macAddr[0]), 6);	
	
		// section sta num , mark_sta
	  pmib->dot11StationConfigEntry.supportedStaNum = mib_all.wlan[index].stanum;

	if ( index == 0) { // root interface	
		// set RF parameters
		/*
		apmib_get(MIB_HW_RF_TYPE, (void *)&intVal);
		pmib->dot11RFEntry.dot11RFType = intVal;
		*/
		//pmib->dot11RFEntry.dot11RFType = hw_config->wlan[index].rfType;
		pmib->dot11RFEntry.dot11RFType = 10; //mark_issue, always to 10?
		

#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198)
	//apmib_get(MIB_HW_BOARD_VER, (void *)&intVal);
	hw_config->boardVer=1; //mark_issue, now we force to 2T2R mode , FIX ME
	
	intVal = hw_config->boardVer;
	if (intVal == 1)
		pmib->dot11RFEntry.MIMO_TR_mode = 3;	// 2T2R
	else if(intVal == 2)
		pmib->dot11RFEntry.MIMO_TR_mode = 4; // 1T1R
	else
		pmib->dot11RFEntry.MIMO_TR_mode = 1;	// 1T2R
/*		
	apmib_get(MIB_HW_TX_POWER_CCK_A, (void *)buf1);
	memcpy(pmib->dot11RFEntry.pwrlevelCCK_A, buf1, MAX_2G_CHANNEL_NUM_MIB);	
	
	apmib_get(MIB_HW_TX_POWER_CCK_B, (void *)buf1);
	memcpy(pmib->dot11RFEntry.pwrlevelCCK_B, buf1, MAX_2G_CHANNEL_NUM_MIB);
	
	apmib_get(MIB_HW_TX_POWER_HT40_1S_A, (void *)buf1);
	memcpy(pmib->dot11RFEntry.pwrlevelHT40_1S_A, buf1, MAX_2G_CHANNEL_NUM_MIB);
	
	apmib_get(MIB_HW_TX_POWER_HT40_1S_B, (void *)buf1);
	memcpy(pmib->dot11RFEntry.pwrlevelHT40_1S_B, buf1, MAX_2G_CHANNEL_NUM_MIB);
	
	apmib_get(MIB_HW_TX_POWER_DIFF_HT40_2S, (void *)buf1);
	memcpy(pmib->dot11RFEntry.pwrdiffHT40_2S, buf1, MAX_2G_CHANNEL_NUM_MIB);
	
	apmib_get(MIB_HW_TX_POWER_DIFF_HT20, (void *)buf1);
	memcpy(pmib->dot11RFEntry.pwrdiffHT20, buf1, MAX_2G_CHANNEL_NUM_MIB);
	
	apmib_get(MIB_HW_TX_POWER_DIFF_OFDM, (void *)buf1);
	memcpy(pmib->dot11RFEntry.pwrdiffOFDM, buf1, MAX_2G_CHANNEL_NUM_MIB);
*/
#if 0  //mark_issue , now these val should come from EFUSE
	memcpy(pmib->dot11RFEntry.pwrlevelCCK_A, hw_config->wlan[0].pwrlevelCCK_A, MAX_2G_CHANNEL_NUM_MIB);	
	memcpy(pmib->dot11RFEntry.pwrlevelCCK_B, hw_config->wlan[0].pwrlevelCCK_B, MAX_2G_CHANNEL_NUM_MIB);
	memcpy(pmib->dot11RFEntry.pwrlevelHT40_1S_A, hw_config->wlan[0].pwrlevelHT40_1S_A, MAX_2G_CHANNEL_NUM_MIB);
	memcpy(pmib->dot11RFEntry.pwrlevelHT40_1S_B, hw_config->wlan[0].pwrlevelHT40_1S_B, MAX_2G_CHANNEL_NUM_MIB);
	memcpy(pmib->dot11RFEntry.pwrdiffHT40_2S, hw_config->wlan[0].pwrdiffHT40_2S, MAX_2G_CHANNEL_NUM_MIB);
	memcpy(pmib->dot11RFEntry.pwrdiffHT20, hw_config->wlan[0].pwrdiffHT20, MAX_2G_CHANNEL_NUM_MIB);
	memcpy(pmib->dot11RFEntry.pwrdiffOFDM, hw_config->wlan[0].pwrdiffOFDM, MAX_2G_CHANNEL_NUM_MIB);
#endif
	
#if defined(CONFIG_RTL_92D_SUPPORT)
#if 0 //mark_issue , now these val should come from EFUSE
	apmib_get(MIB_HW_TX_POWER_5G_HT40_1S_A, (void *)buf1);
	memcpy(pmib->dot11RFEntry.pwrlevel5GHT40_1S_A, buf1, MAX_5G_CHANNEL_NUM_MIB);
	
	apmib_get(MIB_HW_TX_POWER_5G_HT40_1S_B, (void *)buf1);
	memcpy(pmib->dot11RFEntry.pwrlevel5GHT40_1S_B, buf1, MAX_5G_CHANNEL_NUM_MIB);
	
	apmib_get(MIB_HW_TX_POWER_DIFF_5G_HT40_2S, (void *)buf1);
	memcpy(pmib->dot11RFEntry.pwrdiff5GHT40_2S, buf1, MAX_5G_CHANNEL_NUM_MIB);
	
	apmib_get(MIB_HW_TX_POWER_DIFF_5G_HT20, (void *)buf1);
	memcpy(pmib->dot11RFEntry.pwrdiff5GHT20, buf1, MAX_5G_CHANNEL_NUM_MIB);
	
	apmib_get(MIB_HW_TX_POWER_DIFF_5G_OFDM, (void *)buf1);
	memcpy(pmib->dot11RFEntry.pwrdiff5GOFDM, buf1, MAX_5G_CHANNEL_NUM_MIB);
#endif
#endif
/*	
	apmib_get(MIB_HW_11N_TSSI1, (void *)&intVal);
	pmib->dot11RFEntry.tssi1 = intVal;

	apmib_get(MIB_HW_11N_TSSI2, (void *)&intVal);
	pmib->dot11RFEntry.tssi2 = intVal;

	apmib_get(MIB_HW_11N_THER, (void *)&intVal);
	pmib->dot11RFEntry.ther = intVal;
*/

#if 0  //mark_issue , now these val should come from EFUSE
	pmib->dot11RFEntry.tssi1 = hw_config->wlan[0].TSSI1;
	pmib->dot11RFEntry.tssi2 = hw_config->wlan[0].TSSI2;
	pmib->dot11RFEntry.ther = hw_config->wlan[0].Ther;
#endif
	//mark_issue , we assume 8198 and move hardcode to per vap setting below
	//apmib_get(MIB_HW_11N_TRSWITCH, (void *)&intVal);
	//pmib->dot11RFEntry.trswitch = intVal;	 
	
	if (pmib->dot11RFEntry.dot11RFType == 10) { // Zebra ,mark_issue --> set to 10? or com from RFtype?
		//apmib_get(MIB_WLAN_RFPOWER_SCALE, (void *)&intVal);
		intVal = wlan_config->RFPowerScale;
		if(intVal == 1)
			intVal = 3;
		else if(intVal == 2)
				intVal = 6;
			else if(intVal == 3)
					intVal = 9;
				else if(intVal == 4)
						intVal = 17;
		if (intVal) {
			for (i=0; i<MAX_2G_CHANNEL_NUM_MIB; i++) {
				if(pmib->dot11RFEntry.pwrlevelCCK_A[i] != 0){ 
					if ((pmib->dot11RFEntry.pwrlevelCCK_A[i] - intVal) >= 1)
						pmib->dot11RFEntry.pwrlevelCCK_A[i] -= intVal;
					else
						pmib->dot11RFEntry.pwrlevelCCK_A[i] = 1;
				}
				if(pmib->dot11RFEntry.pwrlevelCCK_B[i] != 0){ 
					if ((pmib->dot11RFEntry.pwrlevelCCK_B[i] - intVal) >= 1)
						pmib->dot11RFEntry.pwrlevelCCK_B[i] -= intVal;
					else
						pmib->dot11RFEntry.pwrlevelCCK_B[i] = 1;
				}
				if(pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] != 0){ 
					if ((pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] - intVal) >= 1)
						pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] -= intVal;
					else
						pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] = 1;
				}
				if(pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] != 0){ 
					if ((pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] - intVal) >= 1)
						pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] -= intVal;
					else
						pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] = 1;
				}
			}	
			
#if defined(CONFIG_RTL_92D_SUPPORT)			
			for (i=0; i<MAX_5G_CHANNEL_NUM_MIB; i++) {
				if(pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] != 0){ 
					if ((pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] - intVal) >= 1)
						pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] -= intVal;
					else
						pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = 1;					
				}
				if(pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] != 0){ 
					if ((pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] - intVal) >= 1)
						pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] -= intVal;
					else
						pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = 1;
				}
			}
#endif //#if defined(CONFIG_RTL_92D_SUPPORT)						
		}	
	}	
#endif
		//apmib_get(MIB_WLAN_BEACON_INTERVAL, (void *)&intVal);
		pmib->dot11StationConfigEntry.dot11BeaconPeriod = wlan_config->beaconInterval;
		//apmib_get(MIB_WLAN_CHAN_NUM, (void *)&intVal);
		pmib->dot11RFEntry.dot11channel = wlan_config->channel;

		//apmib_get(MIB_WLAN_RTS_THRESHOLD, (void *)&intVal);
		pmib->dot11OperationEntry.dot11RTSThreshold = wlan_config->rtsThreshold;

		//apmib_get(MIB_WLAN_FRAG_THRESHOLD, (void *)&intVal);
		pmib->dot11OperationEntry.dot11FragmentationThreshold = wlan_config->fragThreshold;

		//apmib_get(MIB_WLAN_INACTIVITY_TIME, (void *)&intVal);
		pmib->dot11OperationEntry.expiretime = wlan_config->inactivityTime;

		//apmib_get(MIB_WLAN_PREAMBLE_TYPE, (void *)&intVal);
		pmib->dot11RFEntry.shortpreamble = wlan_config->preambleType;

		//apmib_get(MIB_WLAN_DTIM_PERIOD, (void *)&intVal);
		pmib->dot11StationConfigEntry.dot11DTIMPeriod = wlan_config->dtimPeriod;

		/*STBC and Coexist*/
		//apmib_get(MIB_WLAN_STBC_ENABLED, &intVal);
		pmib->dot11nConfigEntry.dot11nSTBC = wlan_config->STBCEnabled;
		//apmib_get(MIB_WLAN_COEXIST_ENABLED, &intVal);
		pmib->dot11nConfigEntry.dot11nCoexist = wlan_config->CoexistEnabled;

		// set 11g protection mode
		//apmib_get(MIB_WLAN_PROTECTION_DISABLED, (void *)&intVal);
		pmib->dot11StationConfigEntry.protectionDisabled = wlan_config->protectionDisabled;

		// set block relay
		//apmib_get(MIB_WLAN_BLOCK_RELAY, (void *)&intVal);
		pmib->dot11OperationEntry.block_relay = wlan_config->blockRelay;

		// set WiFi specific mode
		//apmib_get(MIB_WIFI_SPECIFIC, (void *)&intVal);
		//pmib->dot11OperationEntry.wifi_specific = hw_config->wlan[0].wifiSpecific;
		//brian, assign statically temporaily
		pmib->dot11OperationEntry.wifi_specific = 2;

		// Set WDS
		//apmib_get(MIB_WLAN_WDS_ENABLED, (void *)&intVal);
		intVal = wlan_config->wdsEnabled;
		//apmib_get(MIB_WLAN_WDS_NUM, (void *)&intVal2);
		pmib->dot11WdsInfo.wdsNum = 0;
#ifdef MBSSID 
		if (v_previous > 0) 
			intVal = 0;
#endif
		/* section-wds
		if (((mode == 2) || (mode == 3)) &&
			(intVal != 0) &&
			(intVal2 != 0)) {
			for (i=0; i<intVal2; i++) {
				buf1[0] = i+1;
				apmib_get(MIB_WLAN_WDS, (void *)buf1);
				pwds_EntryUI = (WDS_Tp)buf1;
				wds_Entry = &(pmib->dot11WdsInfo.entry[i]);
				memcpy(wds_Entry->macAddr, &(pwds_EntryUI->macAddr[0]), 6);
				wds_Entry->txRate = pwds_EntryUI->fixedTxRate;
				pmib->dot11WdsInfo.wdsNum++;
				sprintf(buf2, "ifconfig %s-wds%d hw ether %02x%02x%02x%02x%02x%02x", ifname, i, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
				system(buf2);
			}
			pmib->dot11WdsInfo.wdsEnabled = intVal;
		}
		else
			pmib->dot11WdsInfo.wdsEnabled = 0;

		if (((mode == 2) || (mode == 3)) &&
			(intVal != 0)) {
			apmib_get(MIB_WLAN_WDS_ENCRYPT, (void *)&intVal);
			if (intVal == 0)
				pmib->dot11WdsInfo.wdsPrivacy = 0;
			else if (intVal == 1) {
				apmib_get(MIB_WLAN_WDS_WEP_KEY, (void *)buf1);
				pmib->dot11WdsInfo.wdsPrivacy = 1;
				string_to_hex(buf1, &(pmib->dot11WdsInfo.wdsWepKey[0]), 10);
			}
			else if (intVal == 2) {
				apmib_get(MIB_WLAN_WDS_WEP_KEY, (void *)buf1);
				pmib->dot11WdsInfo.wdsPrivacy = 5;
				string_to_hex(buf1, &(pmib->dot11WdsInfo.wdsWepKey[0]), 26);
			}
			else if (intVal == 3)
				pmib->dot11WdsInfo.wdsPrivacy = 2;
			else
				pmib->dot11WdsInfo.wdsPrivacy = 4;
		}
		section-wds brian, not config temporary*/

		// enable/disable the notification for IAPP
		//apmib_get(MIB_WLAN_IAPP_DISABLED, (void *)&intVal);
		intVal = wlan_config->iappDisabled;
		if (intVal == 0)
			pmib->dot11OperationEntry.iapp_enable = 1;
		else
			pmib->dot11OperationEntry.iapp_enable = 0;
				

#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_) // below code copy above ACL code
		// Copy Webpage setting to userspace MIB struct table
		pmib->dot1180211sInfo.mesh_acl_num = 0;
		apmib_get(MIB_MESH_ACL_ENABLED, (void *)&intVal);
		pmib->dot1180211sInfo.mesh_acl_mode = intVal;

		if (intVal != 0) {
			apmib_get(MIB_MESH_ACL_NUM, (void *)&intVal);
			if (intVal != 0) {
				for (i=0; i<intVal; i++) {
					buf1[0] = i+1;
					apmib_get(MIB_MESH_ACL_ADDR, (void *)buf1);
					pAcl = (MACFILTER_T *)buf1;
					memcpy(&(pmib->dot1180211sInfo.mesh_acl_addr[i][0]), &(pAcl->macAddr[0]), 6);
					pmib->dot1180211sInfo.mesh_acl_num++;
				}
			}
		}
#endif
		

		// set nat2.5 disable when client and mac clone is set
		//apmib_get(MIB_WLAN_NAT25_MAC_CLONE, (void *)&intVal);
		intVal = wlan_config->maccloneEnabled;
		if ((intVal == 1) && (mode == 1)) {
			pmib->ethBrExtInfo.nat25_disable = 1;
			pmib->ethBrExtInfo.macclone_enable = 1;
		}
		else {
			pmib->ethBrExtInfo.nat25_disable = 0;
			pmib->ethBrExtInfo.macclone_enable = 0;
		}		

		// set nat2.5 disable and macclone disable when wireless isp mode
		/*
		apmib_get(MIB_OP_MODE, (void *)&intVal);
		if (intVal == 2) {
			pmib->ethBrExtInfo.nat25_disable = 1;
			pmib->ethBrExtInfo.macclone_enable = 0;
		}
		*/

#ifdef WIFI_SIMPLE_CONFIG
		pmib->wscEntry.wsc_enable = 0;
#endif

	// for 11n
		//apmib_get(MIB_WLAN_CHANNEL_BONDING, &channel_bound);
		channel_bound = wlan_config->channelbonding;
		pmib->dot11nConfigEntry.dot11nUse40M = channel_bound;
		//apmib_get(MIB_WLAN_CONTROL_SIDEBAND, &intVal);
		intVal = wlan_config->controlsideband;
		if(channel_bound ==0){
			pmib->dot11nConfigEntry.dot11n2ndChOffset = 0;
		}else {
			if(intVal == 0 )
				pmib->dot11nConfigEntry.dot11n2ndChOffset = 1;
			if(intVal == 1 )
				pmib->dot11nConfigEntry.dot11n2ndChOffset = 2;	
		}
		//apmib_get(MIB_WLAN_SHORT_GI, &intVal);
		intVal = wlan_config->shortgiEnabled;
		pmib->dot11nConfigEntry.dot11nShortGIfor20M = intVal;
		pmib->dot11nConfigEntry.dot11nShortGIfor40M = intVal;		
		
		
		//apmib_get(MIB_WLAN_AGGREGATION, &aggregation);
		aggregation = wlan_config->aggregation;
		if(aggregation ==0){
			pmib->dot11nConfigEntry.dot11nAMPDU = 0;
			pmib->dot11nConfigEntry.dot11nAMSDU = 0;
		}else if(aggregation ==1){
			pmib->dot11nConfigEntry.dot11nAMPDU = 1;
			pmib->dot11nConfigEntry.dot11nAMSDU = 0;
		}else if(aggregation ==2){
			pmib->dot11nConfigEntry.dot11nAMPDU = 0;
			pmib->dot11nConfigEntry.dot11nAMSDU = 1;
		}
		else if(aggregation ==3){
			pmib->dot11nConfigEntry.dot11nAMPDU = 1;
			pmib->dot11nConfigEntry.dot11nAMSDU = 1;
		}

#if defined(CONFIG_RTL8196B) && defined(MBSSID)
		if(pmib->dot11OperationEntry.opmode & 0x00000010){// AP mode
			for (index = 1; index < 5; index++) {
				//apmib_get(MIB_WLAN_DISABLED, (void *)&intVal4);
				intVal4 = wlan_config->wlanDisabled;
				if (intVal4 == 0)
					vap_enable++;
				intVal4=0;
			}
			index = 0;
		}
		if (vap_enable && (mode ==  AP_MODE || mode ==  AP_WDS_MODE))	
			pmib->miscEntry.vap_enable=1;
		else
			pmib->miscEntry.vap_enable=0;
#endif
	}	
	
	//if (index != NUM_VWLAN_INTERFACE) { // not repeater interface
	if (index != WLAN_VXD_INDEX) { // not repeater interface , mark_fix in ap_hcm mode
		//apmib_get(MIB_WLAN_BASIC_RATE, (void *)&intVal);
		pmib->dot11StationConfigEntry.dot11BasicRates = wlan_config->basicRates;

		//apmib_get(MIB_WLAN_SUPPORTED_RATE, (void *)&intVal);
		pmib->dot11StationConfigEntry.dot11SupportedRates = wlan_config->supportedRates;

		//apmib_get(MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&intVal);
		intVal = wlan_config->rateAdaptiveEnabled;
		if (intVal == 0) {
			pmib->dot11StationConfigEntry.autoRate = 0;
			//apmib_get(MIB_WLAN_FIX_RATE, (void *)&intVal);
			pmib->dot11StationConfigEntry.fixedTxRate = wlan_config->fixedTxRate;
		}
		else
			pmib->dot11StationConfigEntry.autoRate = 1;

		//apmib_get(MIB_WLAN_HIDDEN_SSID, (void *)&intVal);
		pmib->dot11OperationEntry.hiddenAP = wlan_config->hiddenSSID;

#if defined(CONFIG_RTL_92D_SUPPORT)
		//apmib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&intVal);
		pmib->dot11RFEntry.phyBandSelect = wlan_config->phyBandSelect;
		//apmib_get(MIB_WLAN_MAC_PHY_MODE, (void *)&intVal);
		pmib->dot11RFEntry.macPhyMode = wlan_config->macPhyMode;
#endif

	// set band
		//apmib_get(MIB_WLAN_BAND, (void *)&intVal);
		//wlan_band = intVal;
		wlan_band = wlan_config->wlanBand;
		if ((mode != 1) && (pmib->dot11OperationEntry.wifi_specific == 1) && (wlan_band == 2))
			wlan_band = 3;

		if (wlan_band == 8) { // pure-11n
#if defined(CONFIG_RTL_92D_SUPPORT)
			if(pmib->dot11RFEntry.phyBandSelect == PHYBAND_5G)
				wlan_band += 4; // a+n
			else if (pmib->dot11RFEntry.phyBandSelect == PHYBAND_2G)
#endif		
			wlan_band += 3; // b+g+n
			pmib->dot11StationConfigEntry.legacySTADeny = 3;
		}
		else if (wlan_band == 2) { // pure-11g
			wlan_band += 1; // b+g
			pmib->dot11StationConfigEntry.legacySTADeny = 1;
		}
		else if (wlan_band == 10) { // g+n
			wlan_band += 1; // b+g+n
			pmib->dot11StationConfigEntry.legacySTADeny = 1;
		}
		else
			pmib->dot11StationConfigEntry.legacySTADeny = 0;	

		pmib->dot11BssType.net_work_type = wlan_band;

		// set guest access
		//apmib_get(MIB_WLAN_ACCESS, (void *)&intVal);
		pmib->dot11OperationEntry.guest_access = wlan_config->access;

		// set WMM
		//apmib_get(MIB_WLAN_WMM_ENABLED, (void *)&intVal);
		pmib->dot11QosEntry.dot11QosEnable = wlan_config->wmmEnabled;
		
	}

	//apmib_get(MIB_WLAN_AUTH_TYPE, (void *)&intVal);
	//apmib_get(MIB_WLAN_ENCRYPT, (void *)&encrypt);
	intVal = wlan_config->authType;
	encrypt = wlan_config->encrypt;
#ifdef CONFIG_RTL_WAPI_SUPPORT
	/*wapi is independed. disable WAPI first if not WAPI*/
	if(7 !=encrypt)
	{
		pmib->wapiInfo.wapiType=0;	
	}
#endif
	if ((intVal == 1) && (encrypt != 1)) {
		// shared-key and not WEP enabled, force to open-system
		intVal = 0;
	}
	pmib->dot1180211AuthEntry.dot11AuthAlgrthm = intVal;

	if (encrypt == 0)
		pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 0;
	else if (encrypt == 1) {
		// WEP mode
		//apmib_get(MIB_WLAN_WEP, (void *)&wep);
		wep = wlan_config->wep;
		if (wep == 1) {
			pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 1;
			/*
			apmib_get(MIB_WLAN_WEP64_KEY1, (void *)buf1);
			memcpy(&(pmib->dot11DefaultKeysTable.keytype[0]), buf1, 5);
			apmib_get(MIB_WLAN_WEP64_KEY2, (void *)buf1);
			memcpy(&(pmib->dot11DefaultKeysTable.keytype[1]), buf1, 5);
			apmib_get(MIB_WLAN_WEP64_KEY3, (void *)buf1);
			memcpy(&(pmib->dot11DefaultKeysTable.keytype[2]), buf1, 5);
			apmib_get(MIB_WLAN_WEP64_KEY4, (void *)buf1);
			memcpy(&(pmib->dot11DefaultKeysTable.keytype[3]), buf1, 5);
			apmib_get(MIB_WLAN_WEP_DEFAULT_KEY, (void *)&intVal);
			pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex = intVal;
			*/
			memcpy(&(pmib->dot11DefaultKeysTable.keytype[0]), wlan_config->wep64Key1, 5);
			memcpy(&(pmib->dot11DefaultKeysTable.keytype[1]), wlan_config->wep64Key2, 5);
			memcpy(&(pmib->dot11DefaultKeysTable.keytype[2]), wlan_config->wep64Key3, 5);
			memcpy(&(pmib->dot11DefaultKeysTable.keytype[3]), wlan_config->wep64Key4, 5);
			pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex = wlan_config->wepDefaultKey;
		}
		else {
			pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 5;
			/*
			apmib_get(MIB_WLAN_WEP128_KEY1, (void *)buf1);
			memcpy(&(pmib->dot11DefaultKeysTable.keytype[0]), buf1, 13);
			apmib_get(MIB_WLAN_WEP128_KEY2, (void *)buf1);
			memcpy(&(pmib->dot11DefaultKeysTable.keytype[1]), buf1, 13);
			apmib_get(MIB_WLAN_WEP128_KEY3, (void *)buf1);
			memcpy(&(pmib->dot11DefaultKeysTable.keytype[2]), buf1, 13);
			apmib_get(MIB_WLAN_WEP128_KEY4, (void *)buf1);
			memcpy(&(pmib->dot11DefaultKeysTable.keytype[3]), buf1, 13);
			apmib_get(MIB_WLAN_WEP_DEFAULT_KEY, (void *)&intVal);
			pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex = intVal;
			*/
			memcpy(&(pmib->dot11DefaultKeysTable.keytype[0]), wlan_config->wep128Key1, 13);
			memcpy(&(pmib->dot11DefaultKeysTable.keytype[1]), wlan_config->wep128Key2, 13);
			memcpy(&(pmib->dot11DefaultKeysTable.keytype[2]), wlan_config->wep128Key3, 13);
			memcpy(&(pmib->dot11DefaultKeysTable.keytype[3]), wlan_config->wep128Key4, 13);
			pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex = wlan_config->wepDefaultKey;
		}
	}
#ifdef CONFIG_RTL_WAPI_SUPPORT	
	else if(7 == encrypt)
	{
		pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 0;
		pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 0;
	}
#endif	
	else {
		// WPA mode
		pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 2;
	}

#ifndef CONFIG_RTL8196B_TLD
#ifdef MBSSID
	if (index > 0 && pmib->dot11OperationEntry.guest_access)
		pmib->dot11OperationEntry.block_relay = 1;	
#endif
#endif

	// Set 802.1x flag
	enable1x = 0;
	if (encrypt < 2) {
		/*
		apmib_get(MIB_WLAN_ENABLE_1X, (void *)&intVal);
		apmib_get(MIB_WLAN_ENABLE_MAC_AUTH, (void *)&intVal2);
		*/
		intVal = wlan_config->enable1X;
		intVal2 = wlan_config->macAuthEnabled;
		if ((intVal != 0) || (intVal2 != 0))
			enable1x = 1;
	}
#ifdef CONFIG_RTL_WAPI_SUPPORT
	else if(encrypt == 7)
	{
		/*wapi*/
		enable1x = 0;
	}
#endif	
	else
		enable1x = 1;
	pmib->dot118021xAuthEntry.dot118021xAlgrthm = enable1x;

#ifdef CONFIG_RTL_WAPI_SUPPORT
	if(7 == encrypt)
	{
		//apmib_get(MIB_WLAN_WAPI_ASIPADDR,);
		apmib_get(MIB_WLAN_WAPI_AUTH,(void *)&intVal);
		pmib->wapiInfo.wapiType=intVal;

		apmib_get(MIB_WLAN_WAPI_MCAST_PACKETS,(void *)&intVal);
		pmib->wapiInfo.wapiUpdateMCastKeyPktNum=intVal;
		
		apmib_get(MIB_WLAN_WAPI_MCAST_REKEYTYPE,(void *)&intVal);
		pmib->wapiInfo.wapiUpdateMCastKeyType=intVal;

		apmib_get(MIB_WLAN_WAPI_MCAST_TIME,(void *)&intVal);
		pmib->wapiInfo.wapiUpdateMCastKeyTimeout=intVal;

		apmib_get(MIB_WLAN_WAPI_UCAST_PACKETS,(void *)&intVal);
		pmib->wapiInfo.wapiUpdateUCastKeyPktNum=intVal;
		
		apmib_get(MIB_WLAN_WAPI_UCAST_REKETTYPE,(void *)&intVal);
		pmib->wapiInfo.wapiUpdateUCastKeyType=intVal;

		apmib_get(MIB_WLAN_WAPI_UCAST_TIME,(void *)&intVal);
		pmib->wapiInfo.wapiUpdateUCastKeyTimeout=intVal;

		/*1: hex  -else passthru*/
		apmib_get(MIB_WLAN_WAPI_PSK_FORMAT,(void *)&intVal2);
		apmib_get(MIB_WLAN_WAPI_PSKLEN,(void *)&intVal);
		apmib_get(MIB_WLAN_WAPI_PSK,(void *)buf1);
		pmib->wapiInfo.wapiPsk.len=intVal;
		if(1 == intVal2 )
		{
			/*hex*/	
			string_to_hex(buf1, buf2, pmib->wapiInfo.wapiPsk.len*2);
		}else
		{
			/*passthru*/
			strcpy(buf2,buf1);
		}
		memcpy(pmib->wapiInfo.wapiPsk.octet,buf2,pmib->wapiInfo.wapiPsk.len);
	}
#endif


#ifdef CONFIG_RTK_MESH

#ifdef CONFIG_NEW_MESH_UI
	//new feature:Mesh enable/disable
	//brian add new key:MIB_WLAN_MESH_ENABLE
	pmib->dot1180211sInfo.meshSilence = 0;

	apmib_get(MIB_WLAN_MESH_ENABLE,(void *)&intVal);
	if (mode == AP_MESH_MODE || mode == MESH_MODE)
	{
		if( intVal )
			pmib->dot1180211sInfo.mesh_enable = 1;
		else
			pmib->dot1180211sInfo.mesh_enable = 0;
	}
	else
		pmib->dot1180211sInfo.mesh_enable = 0;

	// set mesh argument
	// brian change to shutdown portal/root as default
	if (mode == AP_MESH_MODE)
	{
		pmib->dot1180211sInfo.mesh_ap_enable = 1;
		pmib->dot1180211sInfo.mesh_portal_enable = 0;
	}
	else if (mode == MESH_MODE)
	{
		if( !intVal )
			//pmib->dot11OperationEntry.opmode += 64; // WIFI_MESH_STATE = 0x00000040
			pmib->dot1180211sInfo.meshSilence = 1;

		pmib->dot1180211sInfo.mesh_ap_enable = 0;
		pmib->dot1180211sInfo.mesh_portal_enable = 0;		
	}
	else
	{
		pmib->dot1180211sInfo.mesh_ap_enable = 0;
		pmib->dot1180211sInfo.mesh_portal_enable = 0;	
	}
	#if 0	//by brian, dont enable root by default
	apmib_get(MIB_MESH_ROOT_ENABLE, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_root_enable = intVal;
	#else
	pmib->dot1180211sInfo.mesh_root_enable = 0;
	#endif
#else
	if (mode == AP_MPP_MODE)
	{
		pmib->dot1180211sInfo.mesh_enable = 1;
		pmib->dot1180211sInfo.mesh_ap_enable = 1;
		pmib->dot1180211sInfo.mesh_portal_enable = 1;	
	}
	else if (mode == MPP_MODE)
	{
		pmib->dot1180211sInfo.mesh_enable = 1;
		pmib->dot1180211sInfo.mesh_ap_enable = 0;
		pmib->dot1180211sInfo.mesh_portal_enable = 1;
	}
	else if (mode == MAP_MODE)
	{
		pmib->dot1180211sInfo.mesh_enable = 1;
		pmib->dot1180211sInfo.mesh_ap_enable = 1;
		pmib->dot1180211sInfo.mesh_portal_enable = 0;
	}		
	else if (mode == MP_MODE)
	{
		pmib->dot1180211sInfo.mesh_enable = 1;
		pmib->dot1180211sInfo.mesh_ap_enable = 0;
		pmib->dot1180211sInfo.mesh_portal_enable = 0;		
	}
	else
	{
		pmib->dot1180211sInfo.mesh_enable = 0;
		pmib->dot1180211sInfo.mesh_ap_enable = 0;
		pmib->dot1180211sInfo.mesh_portal_enable = 0;	
	}

	apmib_get(MIB_MESH_ROOT_ENABLE, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_root_enable = intVal;
#endif
	apmib_get(MIB_MESH_MAX_NEIGHTBOR, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_max_neightbor = intVal;

	apmib_get(MIB_LOG_ENABLED, (void *)&intVal);
	pmib->dot1180211sInfo.log_enabled = intVal;

	apmib_get(MIB_MESH_ID, (void *)buf1);
	intVal2 = strlen(buf1);
	memset(pmib->dot1180211sInfo.mesh_id, 0, 32);
	memcpy(pmib->dot1180211sInfo.mesh_id, buf1, intVal2);

	apmib_get(MIB_MESH_ENCRYPT, (void *)&intVal);
	apmib_get(MIB_MESH_WPA_AUTH, (void *)&intVal2);

	if( intVal2 == 2 && intVal)
		pmib->dot11sKeysTable.dot11Privacy  = 2;
	else
		pmib->dot11sKeysTable.dot11Privacy  = 0;
	
#ifdef 	_11s_TEST_MODE_	

	apmib_get(MIB_MESH_TEST_PARAM1, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_reserved1 = intVal;

	apmib_get(MIB_MESH_TEST_PARAM2, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_reserved2 = intVal;

	apmib_get(MIB_MESH_TEST_PARAM3, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_reserved3 = intVal;

	apmib_get(MIB_MESH_TEST_PARAM4, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_reserved4 = intVal;

	apmib_get(MIB_MESH_TEST_PARAM5, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_reserved5 = intVal;

	apmib_get(MIB_MESH_TEST_PARAM6, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_reserved6 = intVal;

	apmib_get(MIB_MESH_TEST_PARAM7, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_reserved7 = intVal;

	apmib_get(MIB_MESH_TEST_PARAM8, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_reserved8 = intVal;
	
	apmib_get(MIB_MESH_TEST_PARAM9, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_reserved9 = intVal;

	apmib_get(MIB_MESH_TEST_PARAMA, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_reserveda = intVal;

	apmib_get(MIB_MESH_TEST_PARAMB, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_reservedb = intVal;
	
	apmib_get(MIB_MESH_TEST_PARAMC, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_reservedc = intVal;

	apmib_get(MIB_MESH_TEST_PARAMD, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_reservedd = intVal;

	apmib_get(MIB_MESH_TEST_PARAME, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_reservede = intVal;

	apmib_get(MIB_MESH_TEST_PARAMF, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_reservedf = intVal;
	
	apmib_get(MIB_MESH_TEST_PARAMSTR1, (void *)buf1);
	intVal2 = strlen(buf1)<15 ? strlen(buf1) : 15;
	memset(pmib->dot1180211sInfo.mesh_reservedstr1, 0, 16);
	memcpy(pmib->dot1180211sInfo.mesh_reservedstr1, buf1, intVal2);
	
#endif
	
#endif // CONFIG_RTK_MESH
	

	// When using driver base WPA, set wpa setting to driver
#if 1
	int intVal3;
	//apmib_get(MIB_WLAN_WPA_AUTH, (void *)&intVal3);
	intVal3 = wlan_config->wpaAuth;
//#ifdef CONFIG_RTL8196B
// button 2009.05.21
#if 1
	if ((intVal3 & WPA_AUTH_PSK) && encrypt >= 2 
#ifdef CONFIG_RTL_WAPI_SUPPORT
		&& encrypt < 7
#endif
	)
#else
	if (mode != 1 && (intVal3 & WPA_AUTH_PSK) && encrypt >= 2 
#ifdef CONFIG_RTL_WAPI_SUPPORT
&& encrypt < 7
#endif
)
#endif
	{
		if (encrypt == 2)
			intVal = 1;
		else if (encrypt == 4)
			intVal = 2;
		else if (encrypt == 6)
			intVal = 3;
		else {
			printf("invalid ENCRYPT value!\n");
			return -1;
		}
		pmib->dot1180211AuthEntry.dot11EnablePSK = intVal;

		if (encrypt == 2 || encrypt == 6) {
			//apmib_get(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&intVal2);
			intVal2 = wlan_config->wpaCipher;
			if (intVal2 == 1)
				intVal = 2;
			else if (intVal2 == 2)
				intVal = 8;
			else if (intVal2 == 3)
				intVal = 10;
			else {
				printf("invalid WPA_CIPHER_SUITE value!\n");
				return -1;
			}
			pmib->dot1180211AuthEntry.dot11WPACipher = intVal;			
		}
		
		if (encrypt == 4 || encrypt == 6) {
			//apmib_get(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&intVal2);
			intVal2 = wlan_config->wpa2Cipher;
			if (intVal2 == 1)
				intVal = 2;
			else if (intVal2 == 2)
				intVal = 8;
			else if (intVal2 == 3)
				intVal = 10;
			else {
				printf("invalid WPA2_CIPHER_SUITE value!\n");
				return -1;
			}
			pmib->dot1180211AuthEntry.dot11WPA2Cipher = intVal;			
		}

		//apmib_get(MIB_WLAN_WPA_PSK, (void *)buf1);
		strcpy(pmib->dot1180211AuthEntry.dot11PassPhrase, wlan_config->wpaPSK);

		//apmib_get(MIB_WLAN_WPA_GROUP_REKEY_TIME, (void *)&intVal);
		pmib->dot1180211AuthEntry.dot11GKRekeyTime = wlan_config->wpaGroupRekeyTime;
	}
	else		
		pmib->dot1180211AuthEntry.dot11EnablePSK = 0;
#endif

#ifdef CONFIG_RTL8198
	pmib->dot11RFEntry.trswitch = 1;
#endif

/* Manual EDCA ---------->*/
	pmib->dot11QosEntry.ManualEDCA = mib_all.wlan[index].enable_manual_edca;
	pmib->dot11QosEntry.STA_manualEDCA[BK].ACM		=	mib_all.wlan[index].sta_bkq_acm;
	pmib->dot11QosEntry.STA_manualEDCA[BK].AIFSN	=	mib_all.wlan[index].sta_bkq_aifsn;
	pmib->dot11QosEntry.STA_manualEDCA[BK].ECWmin	=	mib_all.wlan[index].sta_bkq_cwmin;
	pmib->dot11QosEntry.STA_manualEDCA[BK].ECWmax	=	mib_all.wlan[index].sta_bkq_cwmax;
	pmib->dot11QosEntry.STA_manualEDCA[BK].TXOPlimit=	mib_all.wlan[index].sta_bkq_txoplimit;
	pmib->dot11QosEntry.STA_manualEDCA[BE].ACM		=	mib_all.wlan[index].sta_beq_acm;
	pmib->dot11QosEntry.STA_manualEDCA[BE].AIFSN	=	mib_all.wlan[index].sta_beq_aifsn;
	pmib->dot11QosEntry.STA_manualEDCA[BE].ECWmin	=	mib_all.wlan[index].sta_beq_cwmin;
	pmib->dot11QosEntry.STA_manualEDCA[BE].ECWmax	=	mib_all.wlan[index].sta_beq_cwmax;
	pmib->dot11QosEntry.STA_manualEDCA[BE].TXOPlimit=	mib_all.wlan[index].sta_beq_txoplimit;
	pmib->dot11QosEntry.STA_manualEDCA[VI].ACM		=	mib_all.wlan[index].sta_viq_acm;
	pmib->dot11QosEntry.STA_manualEDCA[VI].AIFSN	=	mib_all.wlan[index].sta_viq_aifsn;
	pmib->dot11QosEntry.STA_manualEDCA[VI].ECWmin	=	mib_all.wlan[index].sta_viq_cwmin;
	pmib->dot11QosEntry.STA_manualEDCA[VI].ECWmax	=	mib_all.wlan[index].sta_viq_cwmax;
	pmib->dot11QosEntry.STA_manualEDCA[VI].TXOPlimit=	mib_all.wlan[index].sta_viq_txoplimit;
	pmib->dot11QosEntry.STA_manualEDCA[VO].ACM		=	mib_all.wlan[index].sta_voq_acm;
	pmib->dot11QosEntry.STA_manualEDCA[VO].AIFSN	=	mib_all.wlan[index].sta_voq_aifsn;
	pmib->dot11QosEntry.STA_manualEDCA[VO].ECWmin	=	mib_all.wlan[index].sta_voq_cwmin;
	pmib->dot11QosEntry.STA_manualEDCA[VO].ECWmax	=	mib_all.wlan[index].sta_voq_cwmax;
	pmib->dot11QosEntry.STA_manualEDCA[VO].TXOPlimit=	mib_all.wlan[index].sta_voq_txoplimit;
	//pmib->dot11QosEntry.AP_manualEDCA[BK].ACM		=	mib_all.wlan[index].ap_bkq_acm;
	pmib->dot11QosEntry.AP_manualEDCA[BK].AIFSN	=	mib_all.wlan[index].ap_bkq_aifsn;
	pmib->dot11QosEntry.AP_manualEDCA[BK].ECWmin	=	mib_all.wlan[index].ap_bkq_cwmin;
	pmib->dot11QosEntry.AP_manualEDCA[BK].ECWmax	=	mib_all.wlan[index].ap_bkq_cwmax;
	pmib->dot11QosEntry.AP_manualEDCA[BK].TXOPlimit=	mib_all.wlan[index].ap_bkq_txoplimit;
	//pmib->dot11QosEntry.AP_manualEDCA[BE].ACM		=	mib_all.wlan[index].ap_beq_acm;
	pmib->dot11QosEntry.AP_manualEDCA[BE].AIFSN	=	mib_all.wlan[index].ap_beq_aifsn;
	pmib->dot11QosEntry.AP_manualEDCA[BE].ECWmin	=	mib_all.wlan[index].ap_beq_cwmin;
	pmib->dot11QosEntry.AP_manualEDCA[BE].ECWmax	=	mib_all.wlan[index].ap_beq_cwmax;
	pmib->dot11QosEntry.AP_manualEDCA[BE].TXOPlimit=	mib_all.wlan[index].ap_beq_txoplimit;
	//pmib->dot11QosEntry.AP_manualEDCA[VI].ACM		=	mib_all.wlan[index].ap_viq_acm;
	pmib->dot11QosEntry.AP_manualEDCA[VI].AIFSN	=	mib_all.wlan[index].ap_viq_aifsn;
	pmib->dot11QosEntry.AP_manualEDCA[VI].ECWmin	=	mib_all.wlan[index].ap_viq_cwmin;
	pmib->dot11QosEntry.AP_manualEDCA[VI].ECWmax	=	mib_all.wlan[index].ap_viq_cwmax;
	pmib->dot11QosEntry.AP_manualEDCA[VI].TXOPlimit=	mib_all.wlan[index].ap_viq_txoplimit;
	//pmib->dot11QosEntry.AP_manualEDCA[VO].ACM		=	mib_all.wlan[index].ap_voq_acm;
	pmib->dot11QosEntry.AP_manualEDCA[VO].AIFSN	=	mib_all.wlan[index].ap_voq_aifsn;
	pmib->dot11QosEntry.AP_manualEDCA[VO].ECWmin	=	mib_all.wlan[index].ap_voq_cwmin;
	pmib->dot11QosEntry.AP_manualEDCA[VO].ECWmax	=	mib_all.wlan[index].ap_voq_cwmax;
	pmib->dot11QosEntry.AP_manualEDCA[VO].TXOPlimit=	mib_all.wlan[index].ap_voq_txoplimit;
/* <---------- Manual EDCA*/

	wrq.u.data.pointer = (caddr_t)pmib;
	wrq.u.data.length = sizeof(struct wifi_mib);
	if (ioctl(skfd, 0x8B43, &wrq) < 0) {
		printf("Set WLAN MIB failed!\n");
		return -1;
	}
	close(skfd);

#if 0
//#ifdef UNIVERSAL_REPEATER
	// set repeater interface
	if (!strcmp(ifname, "wlan0")) {
		apmib_get(MIB_REPEATER_ENABLED1, (void *)&intVal);
		apmib_get(MIB_WLAN_NETWORK_TYPE, (void *)&intVal2);		
		system("ifconfig wlan0-vxd down");
		if (intVal != 0 && mode != WDS_MODE && 
				!(mode==CLIENT_MODE && intVal2==ADHOC)) {
			skfd = socket(AF_INET, SOCK_DGRAM, 0);
			strncpy(wrq.ifr_name, "wlan0-vxd", IFNAMSIZ);
			if (ioctl(skfd, SIOCGIWNAME, &wrq) < 0) {
				printf("Interface open failed!\n");
				return -1;
			}

			wrq.u.data.pointer = (caddr_t)pmib;
			wrq.u.data.length = sizeof(struct wifi_mib);
			if (ioctl(skfd, 0x8B42, &wrq) < 0) {
				printf("Get WLAN MIB failed!\n");
				return -1;
			}

			apmib_get(MIB_REPEATER_SSID1, (void *)buf1);
			intVal2 = strlen(buf1);
			pmib->dot11StationConfigEntry.dot11DesiredSSIDLen = intVal2;
			memset(pmib->dot11StationConfigEntry.dot11DesiredSSID, 0, 32);
			memcpy(pmib->dot11StationConfigEntry.dot11DesiredSSID, buf1, intVal2);

			sprintf(buf1, "ifconfig %s-vxd hw ether %02x%02x%02x%02x%02x%02x", ifname, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			system(buf1);

			enable1xVxd = 0;
			if (encrypt == 0)
				pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 0;
			else if (encrypt == 1) {
				if (enable1x == 0) {
					if (wep == 1)
						pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 1;
					else
						pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 5;
				}
				else
					pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 0;
			}
			else {
				apmib_get(MIB_WLAN_WPA_AUTH, (void *)&intVal2);
				if (intVal2 == 2) {
					pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 2;
					enable1xVxd = 1;
				}
				else
					pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 0;
			}
			pmib->dot118021xAuthEntry.dot118021xAlgrthm = enable1xVxd;
			
			wrq.u.data.pointer = (caddr_t)pmib;
			wrq.u.data.length = sizeof(struct wifi_mib);
			if (ioctl(skfd, 0x8B43, &wrq) < 0) {
				printf("Set WLAN MIB failed!\n");
				return -1;
			}
			close(skfd);
		}
	}

	if (!strcmp(ifname, "wlan1")) {
		apmib_get(MIB_REPEATER_ENABLED1, (void *)&intVal);
		system("ifconfig wlan1-vxd down");
		if (intVal != 0) {
			skfd = socket(AF_INET, SOCK_DGRAM, 0);
			strncpy(wrq.ifr_name, "wlan1-vxd", IFNAMSIZ);
			if (ioctl(skfd, SIOCGIWNAME, &wrq) < 0) {
				printf("Interface open failed!\n");
				return -1;
			}

			wrq.u.data.pointer = (caddr_t)pmib;
			wrq.u.data.length = sizeof(struct wifi_mib);
			if (ioctl(skfd, 0x8B42, &wrq) < 0) {
				printf("Get WLAN MIB failed!\n");
				return -1;
			}

			apmib_get(MIB_REPEATER_SSID1, (void *)buf1);
			intVal2 = strlen(buf1);
			pmib->dot11StationConfigEntry.dot11DesiredSSIDLen = intVal2;
			memset(pmib->dot11StationConfigEntry.dot11DesiredSSID, 0, 32);
			memcpy(pmib->dot11StationConfigEntry.dot11DesiredSSID, buf1, intVal2);

			sprintf(buf1, "ifconfig %s-vxd hw ether %02x%02x%02x%02x%02x%02x", ifname, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			system(buf1);

			enable1xVxd = 0;
			if (encrypt == 0)
				pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 0;
			else if (encrypt == 1) {
				if (enable1x == 0) {
					if (wep == 1)
						pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 1;
					else
						pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 5;
				}
				else
					pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 0;
			}
			else {
				apmib_get(MIB_WLAN_WPA_AUTH, (void *)&intVal2);
				if (intVal2 == 2) {
					pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 2;
					enable1xVxd = 1;
				}
				else
					pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 0;
			}
			pmib->dot118021xAuthEntry.dot118021xAlgrthm = enable1xVxd;

			wrq.u.data.pointer = (caddr_t)pmib;
			wrq.u.data.length = sizeof(struct wifi_mib);
			if (ioctl(skfd, 0x8B43, &wrq) < 0) {
				printf("Set WLAN MIB failed!\n");
				return -1;
			}
			close(skfd);
		}
	}
#endif

	free(pmib);
	return 0;
}

