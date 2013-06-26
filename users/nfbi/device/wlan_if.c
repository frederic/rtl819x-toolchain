/*
  *   Module to access wlan driver
  *
  *	Copyright (C)2008, Realtek Semiconductor Corp. All rights reserved.
  *
  *	$Id: wlan_if.c,v 1.12 2009/04/24 11:44:52 michael Exp $
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

/*================================================================*/
/* Local Include Files */

#include "hcd.h"
#include "mib.h"
//#include "../../../linux-2.4.18/drivers/net/rtl8190/ieee802_mib.h"
//#include "../../wireless_tools.29/iwlib.h"		/* Header */

/*================================================================*/
/* Constant Definitions */

#define ENC_WEP64				1
#define ENC_WEP128			5
#define WEP64_KEY_LEN	5
#define WEP128_KEY_LEN	13

#ifdef RT_WLAN
#define CFG_FILENAME        "/tmp/RT2870STA.dat"
#define CFG_TMP_FILENAME    "/tmp/RT2870STA.tmp"
#define SCAN_LOCK_FILE      "/tmp/scan.lock"
#endif

typedef struct rtk_wireless_info {
    unsigned char ap[8];// first 6 bytes indicated as associated AP MAC/BSSID address, last two bytes
                        // are padding data. An address equal to 00:00:00:00:00:00 means that the module 
                        // failed in association
    unsigned char ssid[36];// SSID of associated AP/IBSS
	int freq;			// channel number of associated AP/IBSS
	int link_quality;
	int signal_level;
	int noise_level;
	int rx_invalid_cypt;	// error frame for decryption
	int rx_invalid_frag;	// error frame for decode fragmentation
	int tx_excessive_retry;	// error frame to transmission
	int invalid_misc; 	// other packets lost in relation with specific wireless operations. 
    int missed_beacon;	// number of periodic beacons from the Access Point/ we have missed.
} rtk_wireless_info;

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

typedef struct scan_result {
    unsigned char index;      // indicate the position of bssdb[0] in the scanning result
    unsigned char number;	// number bss_info existed in bssdb[] array
    unsigned char more;      // 0:no more scanning result, 1:there are still scanning results after bssdb[4]
    unsigned char pad;	    // padding field, not use
    bss_info bssdb[5];
} scan_result;

/*================================================================*/
extern int is_wlan_found;

/* Routine Implementations */
int get_interface_index(char *ifname)
{
    struct ifreq ifr;
    int sockfd = 0;
    
    /*open socket*/
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		perror("socket():");
        return -1;
	}
	
    memset((char *)&ifr, 0, sizeof(struct ifreq));
	/*retrieve interface index*/
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1) {
		//perror("SIOCGIFINDEX");
		close(sockfd);
		return -1;
	}
	close(sockfd);
	return ifr.ifr_ifindex;
}

int get_wlan_mac_addr(char *ifname, char *macaddr)
{
    struct ifreq ifr;
    int sockfd = 0;
    int i;
    
    memset(macaddr, 0, 6);
    if (!is_wlan_found)
        return 0;
    
    /*open socket*/
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		perror("socket():");
        return -1;
	}
	
    memset((char *)&ifr, 0, sizeof(struct ifreq));
	/*retrieve interface index*/
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1) {
		//perror("SIOCGIFINDEX");
		close(sockfd);
		return -1;
	}
    //printf("Successfully got interface index: %i\n", ifr.ifr_ifindex);

	/*retrieve corresponding MAC*/
	if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) == -1) {
		perror("SIOCGIFINDEX");
		close(sockfd);
		return -1;
	}
    for (i = 0; i < 6; i++) {
		macaddr[i] = ifr.ifr_hwaddr.sa_data[i];
	}
    //printf("Successfully got our MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n", 
	//		macaddr[0],macaddr[1],macaddr[2],macaddr[3],macaddr[4],macaddr[5]);
	close(sockfd);
	return 0;
}

#if 0
#ifdef ACCESS_WLAN_IF
int set_wlan_mib(char *ifname, struct mib *phmib)
{
	struct wifi_mib *pmib;
	struct iwreq wrq;
	int i, skfd;
			
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

	// Set parameters to driver
	pmib->dot11StationConfigEntry.dot11RegDomain = phmib->regdomain;
	if (!phmib->macclone_enable && memcmp(phmib->clone_mac_addr.addr, ALL_ZERO_MAC_ADDR, 6)) 
		memcpy(pmib->dot11OperationEntry.hwaddr, phmib->clone_mac_addr.addr, 6);

	pmib->dot11OperationEntry.disable_brsc = 1; // alwalys disable br shortcut
	pmib->dot11OperationEntry.ledtype = 3; // LED0, Link/Tx/Rx
	
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

	pmib->dot11OperationEntry.opmode = phmib->opmode;
	pmib->dot11RFEntry.dot11RFType = 10;

/* TODO
		// set RF parameters
		apmib_get(MIB_HW_RF_TYPE, (void *)&intVal);

		apmib_get(MIB_HW_ANT_DIVERSITY, (void *)&intVal);
		pmib->dot11RFEntry.dot11DiversitySupport = intVal;

		apmib_get(MIB_HW_TX_ANT, (void *)&intVal);
		pmib->dot11RFEntry.defaultAntennaB = intVal;

#if 0
		apmib_get(MIB_HW_INIT_GAIN, (void *)&intVal);
		pmib->dot11RFEntry.initialGain = intVal;
#endif

		apmib_get(MIB_HW_TX_POWER_CCK, (void *)buf1);
		memcpy(pmib->dot11RFEntry.pwrlevelCCK, buf1, 14);
		
		apmib_get(MIB_HW_TX_POWER_OFDM, (void *)buf1);
		memcpy(pmib->dot11RFEntry.pwrlevelOFDM, buf1, 162);

		apmib_get(MIB_HW_11N_LOFDMPWD, (void *)&intVal);
		pmib->dot11RFEntry.legacyOFDM_pwrdiff = intVal;
		
		apmib_get(MIB_HW_11N_ANTPWD_C, (void *)&intVal);
		pmib->dot11RFEntry.antC_pwrdiff = intVal;
		
		apmib_get(MIB_HW_11N_THER_RFIC, (void *)&intVal);
		pmib->dot11RFEntry.ther_rfic = intVal;
		
		apmib_get(MIB_HW_11N_XCAP, (void *)&intVal);
		pmib->dot11RFEntry.crystalCap = intVal;

		// set output power scale
		//if (pmib->dot11RFEntry.dot11RFType == 7) { // Zebra
			if (pmib->dot11RFEntry.dot11RFType == 10) { // Zebra
			apmib_get(MIB_WLAN_RF_POWER, (void *)&intVal);
			if(intVal == 1)
				intVal = 3;
			else if(intVal == 2)
					intVal = 6;
				else if(intVal == 3)
						intVal = 9;
					else if(intVal == 4)
							intVal = 17;
			if (intVal) {
				for (i=0; i<14; i++) {
					if(pmib->dot11RFEntry.pwrlevelCCK[i] != 0){ 
						if ((pmib->dot11RFEntry.pwrlevelCCK[i] - intVal) >= 1)
							pmib->dot11RFEntry.pwrlevelCCK[i] -= intVal;
						else
							pmib->dot11RFEntry.pwrlevelCCK[i] = 1;
					}
				}
				for (i=0; i<162; i++) {
					if (pmib->dot11RFEntry.pwrlevelOFDM[i] != 0){
						if((pmib->dot11RFEntry.pwrlevelOFDM[i] - intVal) >= 1)
							pmib->dot11RFEntry.pwrlevelOFDM[i] -= intVal;
						else
							pmib->dot11RFEntry.pwrlevelOFDM[i] = 1;
					}
				}		
			}
		}
*/

	pmib->dot11RFEntry.dot11channel = phmib->channel;
	pmib->dot11OperationEntry.dot11RTSThreshold = phmib->rtsthres;
	pmib->dot11OperationEntry.dot11FragmentationThreshold = phmib->fragthres;

	pmib->ethBrExtInfo.nat25_disable = 1;
	pmib->ethBrExtInfo.macclone_enable = phmib->macclone_enable;

#ifdef WIFI_SIMPLE_CONFIG
	pmib->wscEntry.wsc_enable = 0;
#endif

	pmib->dot11nConfigEntry.dot11nUse40M = phmib->use40M;		
	pmib->dot11nConfigEntry.dot11nShortGIfor20M = phmib->shortGI20M;
	pmib->dot11nConfigEntry.dot11nShortGIfor40M = phmib->shortGI40M;
	pmib->dot11nConfigEntry.dot11nAMPDU = phmib->aggregation;
	pmib->dot11nConfigEntry.dot11nAMSDU = phmib->aggregation;
	pmib->dot11StationConfigEntry.autoRate = phmib->autorate;
	pmib->dot11StationConfigEntry.fixedTxRate = phmib->fixrate;
	pmib->dot11BssType.net_work_type = phmib->band;
	pmib->dot11QosEntry.dot11QosEnable = phmib->qos_enable;
	
	if (phmib->encmode != 1 && phmib->encmode != 5) // not WEP64 and WEP128
		pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 0;
	else
		pmib->dot1180211AuthEntry.dot11AuthAlgrthm = phmib->authtype;

	pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = phmib->encmode;
	if (phmib->encmode == ENC_WEP64)
		i = WEP64_KEY_LEN;
	else
		i = WEP128_KEY_LEN;
	memcpy(&pmib->dot11DefaultKeysTable.keytype[0], phmib->wepkey1, i);
	memcpy(&pmib->dot11DefaultKeysTable.keytype[1], phmib->wepkey2, i);
	memcpy(&pmib->dot11DefaultKeysTable.keytype[2], phmib->wepkey3, i);
	memcpy(&pmib->dot11DefaultKeysTable.keytype[3], phmib->wepkey4, i);
	pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex = phmib->wepdkeyid;

	if (phmib->encmode && phmib->encmode != ENC_WEP64 &&
													 phmib->encmode != ENC_WEP128) { // WPA/WPA2
		pmib->dot1180211AuthEntry.dot11EnablePSK = phmib->psk_enable;
		pmib->dot1180211AuthEntry.dot11WPACipher = phmib->wpa_cipher;
		pmib->dot1180211AuthEntry.dot11WPA2Cipher = phmib->wpa2_cipher;		
		strcpy(pmib->dot1180211AuthEntry.dot11PassPhrase, phmib->passphrase);
	}

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

#endif // ACCESS_WLAN_IF
#endif

#ifdef RT_WLAN
/*------------------------------------------------------------------*/
static int get_info(int skfd, char *ifname, struct wireless_info *info)
{
    struct iwreq wrq;

    memset((char *) info, 0, sizeof(struct wireless_info));

    /* Get basic information */
    if(iw_get_basic_config(skfd, ifname, &(info->b)) < 0) {
        /* If no wireless name : no wireless extensions */
        /* But let's check if the interface exists at all */
        struct ifreq ifr;
        
        strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
        if(ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0)
            return(-ENODEV);
        else
            return(-ENOTSUP);
    }

    /* Get ranges */
    if(iw_get_range_info(skfd, ifname, &(info->range)) >= 0)
      info->has_range = 1;
    
    /* Get AP address */
    if(iw_get_ext(skfd, ifname, SIOCGIWAP, &wrq) >= 0)
      {
        info->has_ap_addr = 1;
        memcpy(&(info->ap_addr), &(wrq.u.ap_addr), sizeof (sockaddr));
      }

    /* Get stats */
    if(iw_get_stats(skfd, ifname, &(info->stats),
            &info->range, info->has_range) >= 0)
      {
        info->has_stats = 1;
      }

  return(0);
}

/*------------------------------------------------------------------*/
static void translate2rtkinfo(struct wireless_info *info, rtk_wireless_info *rtk_info)
{
    memset((char *)rtk_info, 0, sizeof(rtk_wireless_info));
    //ESSID (extended network)
    if (info->b.has_essid) {
        if (info->b.essid_on) {
            strncpy(rtk_info->ssid, info->b.essid, IW_ESSID_MAX_SIZE);
            //rtk_info->ssid[IW_ESSID_MAX_SIZE] = '\0';
        }
        //else
	    //    printf("ESSID:off/any  ");
    }

    //frequency / channel
    if (info->b.has_freq) {
        double	freq = info->b.freq;	/* Frequency/channel */

        if (info->has_range && (freq >= KILO))
            rtk_info->freq = iw_freq_to_channel(freq, &info->range); //channel number
    }

    /* associated AP BSID */
    if(info->has_ap_addr) {
        const struct ether_addr ether_zero = {{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }};
        const struct ether_addr *ether = (const struct ether_addr *) (&info->ap_addr)->sa_data;

        if (iw_ether_cmp(ether, &ether_zero)) {
            rtk_info->ap[0] = ether->ether_addr_octet[0];
            rtk_info->ap[1] = ether->ether_addr_octet[1];
            rtk_info->ap[2] = ether->ether_addr_octet[2];
            rtk_info->ap[3] = ether->ether_addr_octet[3];
            rtk_info->ap[4] = ether->ether_addr_octet[4];
            rtk_info->ap[5] = ether->ether_addr_octet[5];
        }
    }

    /* statistics */
    if (info->has_stats) {
        if (info->has_range && ((info->stats.qual.level != 0)
		   || (info->stats.qual.updated & (IW_QUAL_DBM | IW_QUAL_RCPI)))) {
            /* Deal with quality : always a relative value */
            if (!(info->stats.qual.updated & IW_QUAL_QUAL_INVALID)) {
                rtk_info->link_quality = info->stats.qual.qual;
            }

            /* Check if the statistics are in RCPI (IEEE 802.11k) */
            if (info->stats.qual.updated & IW_QUAL_RCPI) {
                /* Deal with signal level in RCPI */
                /* RCPI = int{(Power in dBm +110)*2} for 0dbm > Power > -110dBm */
                if (!(info->stats.qual.updated & IW_QUAL_LEVEL_INVALID)) {
                    rtk_info->signal_level = (info->stats.qual.level / 2.0) - 110.0;
                }
            
	            /* Deal with noise level in dBm (absolute power measurement) */
                if (!(info->stats.qual.updated & IW_QUAL_NOISE_INVALID)) {
                    rtk_info->noise_level = (info->stats.qual.noise / 2.0) - 110.0;
		    	}
            }
            else {
	            /* Check if the statistics are in dBm */
                if ((info->stats.qual.updated & IW_QUAL_DBM)
                    || (info->stats.qual.level > info->range.max_qual.level)) {
	                /* Deal with signal level in dBm  (absolute power measurement) */
                    if (!(info->stats.qual.updated & IW_QUAL_LEVEL_INVALID)) {
                        int	dblevel = info->stats.qual.level;
                        /* Implement a range for dBm [-192; 63] */
                        if (info->stats.qual.level >= 64)
                            dblevel -= 0x100;
                        rtk_info->signal_level = dblevel;
                    }

                    /* Deal with noise level in dBm (absolute power measurement) */
                    if (!(info->stats.qual.updated & IW_QUAL_NOISE_INVALID)) {
                        int	dbnoise = info->stats.qual.noise;
                        /* Implement a range for dBm [-192; 63] */
                        if (info->stats.qual.noise >= 64)
                            dbnoise -= 0x100;
                        rtk_info->noise_level = dbnoise;
                    }
                }
                else {
	                /* Deal with signal level as relative value (0 -> max) */
                    if (!(info->stats.qual.updated & IW_QUAL_LEVEL_INVALID)) {
                        rtk_info->signal_level = info->stats.qual.level;
                    }

                    /* Deal with noise level as relative value (0 -> max) */
                    if (!(info->stats.qual.updated & IW_QUAL_NOISE_INVALID)) {
                        rtk_info->noise_level = info->stats.qual.noise;
                    }
                }
            }
        }
        else {
            /* We can't read the range, so we don't know... */
            rtk_info->link_quality = info->stats.qual.qual;
            rtk_info->signal_level = info->stats.qual.level;
            rtk_info->noise_level = info->stats.qual.noise;
        }

        rtk_info->rx_invalid_cypt = info->stats.discard.code;
        rtk_info->rx_invalid_frag = info->stats.discard.fragment;
        rtk_info->tx_excessive_retry = info->stats.discard.retries;
        rtk_info->invalid_misc = info->stats.discard.misc;
        rtk_info->missed_beacon = info->stats.miss.beacon;
    }
}

int get_wlan_info(char *data)
{
    int skfd, rc;
    struct wireless_info info;
    rtk_wireless_info *rtkinfo = (rtk_wireless_info *)data;

    if (-1 == get_interface_index(IF_WLAN))
        return -101;
        
    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (skfd < 0) {
        perror("socket");
        return -102;
    }

    rc = get_info(skfd, "ra0", &info);
    if (0 == rc) { /* Success */
        translate2rtkinfo(&info, rtkinfo);
        rc = sizeof(rtk_wireless_info);
    }
    
    close(skfd);
    return rc;
}

int create_scanning_file(void)
{
    int fd;
    
    /* Create lock file */
    if ((fd = open(SCAN_LOCK_FILE, O_RDWR|O_CREAT|O_EXCL, 0444)) < 0) {
        perror("fail to create file");
        return -1;
    }

    close(fd);
    return 0;
}

int check_scanning_status(void)
{
    int fd;

    /* Check lock file */
    if ((fd = open(SCAN_LOCK_FILE, O_RDONLY)) < 0)
        return 0;   /* No site survey in progress */
    
    close(fd);
    return 1;       /* site survey in progress */

}

int request_scan(char *param)
{
    pid_t childpid;

    if (-1 == get_interface_index(IF_WLAN))
        return -101;
        
    /* create lock file */
    if (create_scanning_file() < 0) {
        //perror("fail to create file");
        return -102;
    }
    
    unlink("/tmp/site_survey_result");
    if ((childpid=fork()) == -1) {
        perror("can't fork");
        return -103;
    }
    else if (childpid == 0) {
        //child process
        system("iwlist ra0 scanning > /dev/null");
        system("iwpriv ra0 get_site_survey > /tmp/site_survey_result");
        unlink(SCAN_LOCK_FILE);
        exit(0);
    }
    else {
        //parent process
        //printf("childpid=%d\n", childpid);
        param[0] = 0;
        return 1;
    }
}

int get_scan_result(char *param)
{
    FILE *fp;
    char buf[1024];
    int start_index, i, j, num;
    bss_info info;
    scan_result *result = (scan_result *)param;
    
    if (-1 == get_interface_index(IF_WLAN))
        return -101;
        
    //check if site survey is in process
    if (check_scanning_status()) {
        result->index = 0;
        result->number = 0xff; //scanning
        result->more = 0;
        result->pad = 0;
        return 4;
    }
    
    start_index = param[0]&0xff;
    //open result file
    if ((fp = fopen("/tmp/site_survey_result", "r")) == NULL) {
        perror("fail to open result file");
        result->index = start_index;
        result->number = 0;
        result->more = 0;
        result->pad = 0;
        return 4;
    }
    
    //skip 2 lines of header
    fgets(buf, sizeof(buf), fp);
    fgets(buf, sizeof(buf), fp);
    
    //read each line
    i = 0;
    num = 0;
    result->more = 0;
    while (fgets(buf, sizeof(buf), fp)) {
        char *token, *saveptr1;
        unsigned int mac_addr[6];
        char *enc;
        
        if (strlen(buf) < (4+33))
            break;
            
        //channel
        buf[3] = '\0';
        info.channel = (unsigned short)atoi(buf);
        //ssid
        strncpy(info.ssid, (buf+4), 32);
        info.ssid[32] = '\0';
        for (j=31; j>=0; j--) {
            if (info.ssid[j] == ' ')
                info.ssid[j] = '\0';
            else
                break;
        }
        
        //bssid
        token = strtok_r((buf+4+33), " ", &saveptr1);
        if (token == NULL)
            break;
        sscanf(token, "%02x:%02x:%02x:%02x:%02x:%02x",
                    (unsigned int *)&mac_addr[0], 
                    (unsigned int *)&mac_addr[1],
                    (unsigned int *)&mac_addr[2],
                    (unsigned int *)&mac_addr[3],
                    (unsigned int *)&mac_addr[4],
                    (unsigned int *)&mac_addr[5]);
        for (j=0; j<6; j++)
            info.bssid[j] = (unsigned char)mac_addr[j];        

        //enryption
        token = strtok_r(NULL, " ", &saveptr1);
        if (token == NULL)
            break;
        enc = token;
        //authentication
        token = strtok_r(NULL, " ", &saveptr1);
        if (token == NULL)
            break;
        if (strcmp(token, "OPEN")==0) {
            if (strcmp(enc, "WEP")==0)
                info.encrypt = 1;
            else if ((strcmp(enc, "TKIP")==0) ||
                     (strcmp(enc, "AES")==0) ||
                     (strcmp(enc, "TKIPAES")==0))
                info.encrypt = 2;
            else
                info.encrypt = 0;
        }
        else if ((strcmp(token, "SHARED")==0) ||
                 (strcmp(token, "AUTOWEP")==0))
            info.encrypt = 1;
        else if (strcmp(token, "UNKNOW")==0) {
            if (strcmp(enc, "WEP")==0)
                info.encrypt = 1;
            else
                info.encrypt = 5; //UNKNOW
        }
        else if ((strcmp(token, "WPA")==0) ||
                 (strcmp(token, "WPAPSK")==0) ||
                 (strcmp(token, "WPANONE")==0))
            info.encrypt = 2;
        else if ((strcmp(token, "WPA2")==0) ||
                 (strcmp(token, "WPA2PSK")==0))
            info.encrypt = 3;
        else if ((strcmp(token, "WPA1WPA2")==0) ||
                 (strcmp(token, "WPA1PSKWPA2PSK")==0))
            info.encrypt = 4;
        else
            info.encrypt = 5; //UNKNOW
            
        //signal
        token = strtok_r(NULL, " ", &saveptr1);
        if (token == NULL)
            break;
        info.rssi = atoi(token);
        
        //mode
        token = strtok_r(NULL, " ", &saveptr1);
        if (token == NULL)
            break;
        if (strcmp(token, "11b/g/n")==0)
            info.network = 0x01|0x02|0x08;
        else if (strcmp(token, "11b/g")==0)
            info.network = 0x01|0x02;
        else if (strcmp(token, "11b")==0)
            info.network = 0x01;
        else if (strcmp(token, "11a/n")==0)
            info.network = 0x10;
        else if (strcmp(token, "11a")==0)
            info.network = 0x04;
        else if (strcmp(token, "unknow")==0)
            info.network = 0x00;
        
        //network type
        token = strtok_r(NULL, " \n", &saveptr1);
        if (token == NULL)
            break;
        if (strcmp(token, "Ad")==0)
            info.type = 1; //Ad hoc
        else
            info.type = 0; //Infra
        
        if (i >= start_index) {
            if (num>=5) {
                result->more = 1;
                num = 5;
                break;
            }
            else {
                memcpy(&result->bssdb[num], &info, sizeof(bss_info));
                num++;
            }
        }
        i++;
    }
    result->index = start_index;
    result->number = num;
    result->pad = 0;
    
    fclose(fp);
    return (4+num*sizeof(bss_info));
}

int cfgwrite(char *param)
{
    FILE *fp = NULL;
    FILE *tmp_fp = NULL;
    char buf[256], *p, *q;
    int len, found;

    //parse the parameter line from host
    q = strchr(param, '=');
    if (q == NULL) {
        return -101;
    }
    
    if (!(fp = fopen(CFG_FILENAME, "r"))) {
        perror(CFG_FILENAME);
        return -102;
    }
    if (!(tmp_fp = fopen(CFG_TMP_FILENAME, "w"))) {
        fclose(fp);
        perror(CFG_TMP_FILENAME);
        return -103;
    }
    found = 0;
    while (fgets(buf, 256, fp)) {
        if (!found) {
            len = strspn(buf, " ");
            if (buf[len] != '#') {
                p = strchr(buf, '=');
                if (p != NULL) {
                    if (((int)(p-buf) == (int)(q-param)) &&
                        (strncmp(buf, param, (int)(p-buf)) == 0)) {
                        fputs(param, tmp_fp);
                        fputs("\n", tmp_fp);
                        found = 1;
                        continue;
                    }
                }
            }
        }
        fputs(buf, tmp_fp);
    }
    if (!found) {
        fputs(param, tmp_fp);
        fputs("\n", tmp_fp);
    }
    fclose(fp);
    fclose(tmp_fp);
    
    sprintf(buf, "cp -f %s %s", CFG_TMP_FILENAME, CFG_FILENAME);
    system(buf);
    sprintf(buf, "rm -f %s", CFG_TMP_FILENAME);
    system(buf);
    
    param[0] = 0;
    return 1;
}

int cfgread(char *param)
{
    FILE *fp = NULL;
    char buf[256], *p;
    int len;

    if (!(fp = fopen(CFG_FILENAME, "r"))) {
        perror(CFG_FILENAME);
        return -101;
    }

    while (fgets(buf, 256, fp)) {
        len = strspn(buf, " ");
        if (buf[len] == '#')
            continue;
        p = strchr(buf, '=');
        if (p == NULL)
            continue;
        else if (((int)(p-buf) == strlen(param)) &&
                 (strncmp(buf, param, (int)(p-buf)) == 0)) {
            strcpy(param, buf);
            fclose(fp);
            len = strlen(param);
            if ((param[len-1] == '\x0D') || (param[len-1] == '\x0A'))
                param[len-1] = '\0';
            if ((param[len-2] == '\x0D') || (param[len-2] == '\x0A'))
                param[len-2] = '\0';
            return strlen(param);
        }
    }
    fclose(fp);
    return -102;
}

int priv_retrylimit(char *param, int shortflag)
{
    char buf[256];
    int retrylimit;

    if (-1 == get_interface_index(IF_WLAN))
        return -101;
        
    retrylimit = param[0]&0xff;
    if ((retrylimit<0) || (retrylimit>255))
        return -102;
    if (shortflag)
        sprintf(buf, "iwpriv ra0 set ShortRetry=%d", retrylimit);
    else
        sprintf(buf, "iwpriv ra0 set LongRetry=%d", retrylimit);
    //printf(buf);
    //printf("\n");
    system(buf);
    param[0] = 0;
    return 1;
}

#define RT_PRIV_IOCTL								(SIOCIWFIRSTPRIV + 0x0E)
#define	OID_GEN_MEDIA_CONNECT_STATUS				0x060B
#define	RT_OID_802_11_RADIO							0x050B
int getStaConnectStatus(void)
{
    int skfd;
    struct iwreq wrq;
	unsigned int connectStatus = 0;
    unsigned char bRadio;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (skfd < 0) {
        perror("socket");
        return -1;
    }

    strcpy(wrq.ifr_name, IF_WLAN);
    wrq.u.data.length = sizeof(connectStatus);
    wrq.u.data.pointer = &connectStatus;
    wrq.u.data.flags = OID_GEN_MEDIA_CONNECT_STATUS;
    ioctl(skfd, RT_PRIV_IOCTL, &wrq);

    strcpy(wrq.ifr_name, IF_WLAN);
    wrq.u.data.length = sizeof(bRadio);
    wrq.u.data.pointer = &bRadio;
    wrq.u.data.flags = RT_OID_802_11_RADIO;
    ioctl(skfd, RT_PRIV_IOCTL, &wrq);

    close(skfd);
    //printf("connectStatus=%d bRadio=%d\n", connectStatus, bRadio);
	if ((connectStatus == 1) && bRadio)
		return 1;
	else
		return 0;
}
#endif //RT_WLAN
