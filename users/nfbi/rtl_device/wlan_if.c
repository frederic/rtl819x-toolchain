/*
  *   Module to access wlan driver
  *
  *	Copyright (C)2008, Realtek Semiconductor Corp. All rights reserved.
  *
  *	$Id: wlan_if.c,v 1.3 2010/02/05 10:40:30 marklee Exp $
  */


/*================================================================*/
/* System Include Files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <linux/if_ether.h> 
#include <linux/wireless.h>
/*================================================================*/
/* Local Include Files */

#include "hcd.h"
#include "mib.h"
#include "wlan_if.h"
#include "../../../linux-2.6.30/drivers/net/wireless/rtl8192cd/ieee802_mib.h" 


#define LOCAL_ADMIN_BIT 0x02

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
#if 0 //MARKLEE_DEBUG
static init_general_wifi(struct wifi_mib *pmib)  
{

	pmib->dot11OperationEntry.disable_brsc = 1; // alwalys disable br shortcut
	pmib->dot11OperationEntry.ledtype = 3; // LED0, Link/Tx/Rx

	pmib->dot11RFEntry.dot11RFType = 10;

	pmib->ethBrExtInfo.nat25_disable = 1;
}
#endif

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

void bring_down_wlan(void)
{
	int index=0;
	unsigned char tmpbuf[100]={0};

	if( !mib_all.wlan[0].wlanDisabled ) {
		DEL_BR_INTERFACE(IF_BR, mib_all.wlan[0].name, 1,1);

		for( index=0; index<mib_all.wlan_comm.vap_number; index++)
		{
			if( !mib_all.wlan[index+1].wlanDisabled ) {
				memset(tmpbuf,0,100);
				DEL_BR_INTERFACE(IF_BR, mib_all.wlan[index+1].name, 1,1);
			}
		}
	}
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

void bring_up_wlan(void)
{
	int index=0;
	unsigned char tmpbuf[100]={0}, mac[6]={0};

	if( !mib_all.wlan[0].wlanDisabled ) {
		if( mib_all.sys_config.enable_efuse_config ) {
			ADD_BR_INTERFACE(IF_BR, mib_all.wlan[0].name, ALL_ZERO_MAC_ADDR,1,1);
			if( get_root_mac(mac) < 0 ) {
				printf("[%s]ioctl get interface MAC failed.\n");
				return;
			}
		} else {
			string_to_hex(mac,mib_all.wlan[0].macaddr,6);
			ADD_BR_INTERFACE(IF_BR, mib_all.wlan[0].name, mac,1,1);
		}

		for( index=0; index<mib_all.wlan_comm.vap_number; index++)
		{
			if( !mib_all.wlan[index+1].wlanDisabled ) {
				if( mib_all.sys_config.enable_efuse_config ) {
					memset(tmpbuf,0,100);
					memset(mac,0,6);
					string_to_hex(mac,mib_all.wlan[index+1].macaddr,6);
				} else {
					mac[0] = LOCAL_ADMIN_BIT;
					calc_incr((char *)mac+6-1,index+1);
				}
				ADD_BR_INTERFACE(IF_BR, mib_all.wlan[index+1].name, mac,1,1);
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

int init_wlan_comm(struct wifi_mib *pmib)
{
	struct wlan_comm_config_mib *wlan_comm = &mib_all.wlan_comm;

//HW
	pmib->dot11RFEntry.dot11channel = wlan_comm->channel;
	//set output power
	/*
	if (pmib->dot11RFEntry.dot11RFType == 10) { // Zebra
		power = phmib->RFPowerScale;
		if(power == 1)
			power = 3;
		else if(power == 2)
				power = 6;
			else if(power == 3)
					power = 9;
				else if(power == 4)
						power = 17;
		if (power) {
			for (i=0; i<14; i++) {
				if(pmib->dot11RFEntry.pwrlevelCCK[i] != 0){ 
					if ((pmib->dot11RFEntry.pwrlevelCCK[i] - power) >= 1)
						pmib->dot11RFEntry.pwrlevelCCK[i] -= power;
					else
						pmib->dot11RFEntry.pwrlevelCCK[i] = 1;
				}
			}
			for (i=0; i<162; i++) {
				if (pmib->dot11RFEntry.pwrlevelOFDM[i] != 0){
					if((pmib->dot11RFEntry.pwrlevelOFDM[i] - power) >= 1)
						pmib->dot11RFEntry.pwrlevelOFDM[i] -= power;
					else
						pmib->dot11RFEntry.pwrlevelOFDM[i] = 1;
				}
			}		
		}
	}
	*/
//BSS parameters
	pmib->dot11StationConfigEntry.dot11BeaconPeriod = wlan_comm->beaconInterval;
	pmib->dot11StationConfigEntry.dot11DTIMPeriod = wlan_comm->dtimPeriod;
	pmib->dot11StationConfigEntry.dot11BasicRates = wlan_comm->basicRates;
	pmib->dot11StationConfigEntry.dot11RegDomain = wlan_comm->regDomain;
	pmib->dot11StationConfigEntry.fixedTxRate = wlan_comm->fixedTxRate;
	pmib->dot11OperationEntry.dot11RTSThreshold = wlan_comm->rtsThreshold;
	pmib->dot11OperationEntry.dot11FragmentationThreshold = wlan_comm->fragThreshold;
	pmib->dot11BssType.net_work_type = wlan_comm->wlanBand;
	pmib->dot11nConfigEntry.dot11nUse40M = wlan_comm->channelbonding;
	pmib->dot11StationConfigEntry.protectionDisabled = wlan_comm->protectionDisabled;
	if( wlan_comm->vap_number )
		pmib->miscEntry.vap_enable = 1;
}


//int init_wlan(char *ifname)
int init_wlan(int index)
{
	unsigned char tmpbuf[100];
	struct wifi_mib *pmib;
	struct iwreq wrq;
	int i,skfd, power;
	//struct wlan_config_mib *phmib=&wlan_mib;
	struct wlan_config_mib *phmib=&mib_all.wlan[index];

	if( phmib->wlanDisabled )
		return 0;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	//strncpy(wrq.ifr_name, ifname, IFNAMSIZ);
	strncpy(wrq.ifr_name, phmib->name, IFNAMSIZ);
	if (ioctl(skfd, SIOCGIWNAME, &wrq) < 0) {
		printf("Interface open failed!\n");
		return -1;
	}

	if ((pmib = (struct wifi_mib *)malloc(sizeof(struct wifi_mib))) == NULL) {
		printf("MIB buffer allocation failed!\n");
		return -1;
	}

	// get mib from driver
	wrq.u.data.pointer = (caddr_t)pmib;
	wrq.u.data.length = sizeof(struct wifi_mib);

	if (ioctl(skfd, 0x8B42, &wrq) < 0) {
		printf("Get WLAN MIB failed!\n");
		return -1;
	}

	// check mib version
	if (pmib->mib_version != MIB_VERSION) {
		printf("WLAN MIB version mismatch!\n");
		return -1;
	}

	/*
	if( index == 0 && mib_all.sys_config.enable_efuse_config )
		pmib->efuseEntry.enable_efuse = 1;
	*/
		
	//strcpy(phmib->name, "wlan0");
	//init_general_wifi(pmib);  //MARKLEE_DEBUG
	init_wlan_comm(pmib);
	
	// Set parameters to driver
	pmib->dot11RFEntry.shortpreamble = phmib->preambleType;	
	
	//ssid
	pmib->dot11StationConfigEntry.dot11DesiredSSIDLen = strlen(phmib->ssid);
	memset(pmib->dot11StationConfigEntry.dot11DesiredSSID, 0, 32);	
	memcpy(pmib->dot11StationConfigEntry.dot11DesiredSSID, phmib->ssid, strlen(phmib->ssid));	
	if ((pmib->dot11StationConfigEntry.dot11DesiredSSIDLen == 3) &&
		((pmib->dot11StationConfigEntry.dot11DesiredSSID[0] == 'A') || (pmib->dot11StationConfigEntry.dot11DesiredSSID[0] == 'a')) &&
		((pmib->dot11StationConfigEntry.dot11DesiredSSID[1] == 'N') || (pmib->dot11StationConfigEntry.dot11DesiredSSID[1] == 'n')) &&
		((pmib->dot11StationConfigEntry.dot11DesiredSSID[2] == 'Y') || (pmib->dot11StationConfigEntry.dot11DesiredSSID[2] == 'y'))) {
		pmib->dot11StationConfigEntry.dot11SSIDtoScanLen = 0;
		memset(pmib->dot11StationConfigEntry.dot11SSIDtoScan, 0, 32);
	}
	else {
		pmib->dot11StationConfigEntry.dot11SSIDtoScanLen =  strlen(phmib->ssid);
		memset(pmib->dot11StationConfigEntry.dot11SSIDtoScan, 0, 32);
		memcpy(pmib->dot11StationConfigEntry.dot11SSIDtoScan, phmib->ssid, strlen(phmib->ssid));
	}
	pmib->dot11OperationEntry.hiddenAP = phmib->hiddenSSID;

	//BSS
	pmib->dot11StationConfigEntry.dot11AclMode = phmib->acEnabled;
	pmib->dot11StationConfigEntry.dot11SupportedRates=phmib->supportedRates;
	pmib->dot11StationConfigEntry.autoRate = phmib->rateAdaptiveEnabled;

	//security
	if (phmib->encrypt != ENC_WEP64 && phmib->encrypt != ENC_WEP128) // not WEP64 and WEP128
		pmib->dot1180211AuthEntry.dot11AuthAlgrthm = AUTH_OPEN;
	else
		pmib->dot1180211AuthEntry.dot11AuthAlgrthm = phmib->authType;

	pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = phmib->encrypt;

	if (phmib->encrypt == ENC_WEP64)
		i = WEP64_KEY_LEN;
	else
		i = WEP128_KEY_LEN;
	memcpy(&pmib->dot11DefaultKeysTable.keytype[0], phmib->wep128Key1, i);
	memcpy(&pmib->dot11DefaultKeysTable.keytype[1], phmib->wep128Key2, i);
	memcpy(&pmib->dot11DefaultKeysTable.keytype[2], phmib->wep128Key3, i);
	memcpy(&pmib->dot11DefaultKeysTable.keytype[3], phmib->wep128Key4, i);

	pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex = phmib->wepDefaultKey;

	if (phmib->encrypt && phmib->encrypt != ENC_WEP64 &&
													 phmib->encrypt != ENC_WEP128) { // WPA/WPA2		
		pmib->dot1180211AuthEntry.dot11EnablePSK = phmib->psk_enable;
		pmib->dot1180211AuthEntry.dot11WPACipher = phmib->wpaCipher;
		pmib->dot1180211AuthEntry.dot11WPA2Cipher = phmib->wpa2Cipher;		
		strcpy(pmib->dot1180211AuthEntry.dot11PassPhrase, phmib->wpaPSK);		
	}
      else
    	{
        	pmib->dot1180211AuthEntry.dot11EnablePSK=0;
		pmib->dot1180211AuthEntry.dot11WPACipher = 0;
		pmib->dot1180211AuthEntry.dot11WPA2Cipher =0;
		strcpy(pmib->dot1180211AuthEntry.dot11PassPhrase, "");
	}	
	pmib->dot118021xAuthEntry.dot118021xAlgrthm = 0; //MARKLEE_DEBUG ,future support

	pmib->dot1180211AuthEntry.dot11GKRekeyTime = phmib->wpaGroupRekeyTime;

	pmib->dot11OperationEntry.block_relay = phmib->blockRelay;
	pmib->dot11QosEntry.dot11QosEnable = phmib->wmmEnabled;

	if(phmib->aggregation == AGGREGATE_OFF ){
			pmib->dot11nConfigEntry.dot11nAMPDU = 0;
			pmib->dot11nConfigEntry.dot11nAMSDU = 0;
		}else if(phmib->aggregation == AGGREGATE_AMPDU){
			pmib->dot11nConfigEntry.dot11nAMPDU = 1;
			pmib->dot11nConfigEntry.dot11nAMSDU = 0;
		}else if(phmib->aggregation == AGGREGATE_AMSDU){
			pmib->dot11nConfigEntry.dot11nAMPDU = 0;
			pmib->dot11nConfigEntry.dot11nAMSDU = 1;
		}
		else if(phmib->aggregation == AGGREGATE_BOTH){
			pmib->dot11nConfigEntry.dot11nAMPDU = 1;
			pmib->dot11nConfigEntry.dot11nAMSDU = 1;
		}
	pmib->dot11nConfigEntry.dot11nShortGIfor20M = phmib->shortgiEnabled;
	pmib->dot11nConfigEntry.dot11nShortGIfor40M = phmib->shortgiEnabled;
	
	//set to wlan driver
	wrq.u.data.pointer = (caddr_t)pmib;
	wrq.u.data.length = sizeof(struct wifi_mib);
	if (ioctl(skfd, 0x8B43, &wrq) < 0) {
		printf("Set WLAN MIB failed!\n");
		return -1;
	}

	close(skfd);

	free(pmib);
	return 0;
}




