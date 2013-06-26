/*
 *      Web server handler routines for management (password, save config, f/w update)
 *
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *
 *      $Id: fmmgmt.c,v 1.45 2009/09/03 05:04:42 keith_huang Exp $
 *
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/reboot.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "boa.h"
#include "globals.h"
#include "apmib.h"
#include "apform.h"
#include "utility.h"
#include "mibtbl.h"
#include "asp_page.h"

// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT
#include "web_voip.h"
#include "voip_flash_mib.h"
#include "voip_flash_tool.h"
#endif

#if defined(POWER_CONSUMPTION_SUPPORT)
#include "powerCon.h"
#endif

#define DEFAULT_GROUP		"administrators"
#define ACCESS_URL		"/"

#ifdef CONFIG_RTL_WAPI_SUPPORT
#define MTD1_SIZE 0x2d0000	//Address space: 0x2d0000
#define WAPI_SIZE 0x10000	//Address space: 64K
#define WAPI_AREA_BASE (MTD1_SIZE-WAPI_SIZE)
#endif

extern int Decode(unsigned char *ucInput, unsigned int inLen, unsigned char *ucOutput);

#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT)
extern void Stop_Domain_Query_Process(void);
extern unsigned char WaitCountTime;
#endif
//static char superName[MAX_NAME_LEN]={0}, superPass[MAX_NAME_LEN]={0};
//static char userName[MAX_NAME_LEN]={0}, userPass[MAX_NAME_LEN]={0};
int isUpgrade_OK=0;
int isFWUPGRADE=0;
int isCFGUPGRADE=0;
int isREBOOTASP=0;
int Reboot_Wait=0;
int isCFG_ONLY=0;
#ifdef LOGIN_URL
static void delete_user(request *wp);
#endif
int configlen = 0;

int opModeHandler(request *wp, char *tmpBuf);
int find_head_offset(char *upload_data);

////////////////////////////////////////////////////////////////////////////////
#ifdef _LITTLE_ENDIAN_
#if 0
void swap_mib_word_value(APMIB_Tp pMib)
{
	pMib->wlan[wlan_idx][vwlan_idx].fragThreshold = WORD_SWAP(pMib->wlan[wlan_idx][vwlan_idx].fragThreshold);
	pMib->wlan[wlan_idx][vwlan_idx].rtsThreshold = WORD_SWAP(pMib->wlan[wlan_idx][vwlan_idx].rtsThreshold);
	pMib->wlan[wlan_idx][vwlan_idx].supportedRates = WORD_SWAP(pMib->wlan[wlan_idx][vwlan_idx].supportedRates);
	pMib->wlan[wlan_idx][vwlan_idx].basicRates = WORD_SWAP(pMib->wlan[wlan_idx][vwlan_idx].basicRates);
	pMib->wlan[wlan_idx][vwlan_idx].beaconInterval = WORD_SWAP(pMib->wlan[wlan_idx][vwlan_idx].beaconInterval);
	pMib->wlan[wlan_idx][vwlan_idx].inactivityTime = DWORD_SWAP(pMib->wlan[wlan_idx][vwlan_idx].inactivityTime);
	pMib->wlan[wlan_idx][vwlan_idx].wpaGroupRekeyTime = DWORD_SWAP(pMib->wlan[wlan_idx][vwlan_idx].wpaGroupRekeyTime);
	pMib->wlan[wlan_idx][vwlan_idx].rsPort = WORD_SWAP(pMib->wlan[wlan_idx][vwlan_idx].rsPort);

#ifdef HOME_GATEWAY
{
	int i;
	pMib->pppIdleTime = WORD_SWAP(pMib->pppIdleTime);
	for (i=0; i<pMib->portFwNum; i++) {
		pMib->portFwArray[i].fromPort = WORD_SWAP(pMib->portFwArray[i].fromPort);
		pMib->portFwArray[i].toPort = WORD_SWAP(pMib->portFwArray[i].toPort);
	}

	for (i=0; i<pMib->portFilterNum; i++) {
		pMib->portFilterArray[i].fromPort = WORD_SWAP(pMib->portFilterArray[i].fromPort);
		pMib->portFilterArray[i].toPort = WORD_SWAP(pMib->portFilterArray[i].toPort);
	}
	for (i=0; i<pMib->triggerPortNum; i++) {
		pMib->triggerPortArray[i].tri_fromPort = WORD_SWAP(pMib->triggerPortArray[i].tri_fromPort);
		pMib->triggerPortArray[i].tri_toPort = WORD_SWAP(pMib->triggerPortArray[i].tri_toPort);
		pMib->triggerPortArray[i].inc_fromPort = WORD_SWAP(pMib->triggerPortArray[i].inc_fromPort);
		pMib->triggerPortArray[i].inc_toPort = WORD_SWAP(pMib->triggerPortArray[i].inc_toPort);
	}
#ifdef GW_QOS_ENGINE
	pMib->qosManualUplinkSpeed = DWORD_SWAP(pMib->qosManualUplinkSpeed);	
	pMib->qosManualDownLinkSpeed = DWORD_SWAP(pMib->qosManualDownLinkSpeed);	

	for (i=0; i<pMib->qosRuleNum; i++) {
		pMib->qosRuleArray[i].protocol = WORD_SWAP(pMib->qosRuleArray[i].protocol);
		pMib->qosRuleArray[i].local_port_start = WORD_SWAP(pMib->qosRuleArray[i].local_port_start);
		pMib->qosRuleArray[i].local_port_end = WORD_SWAP(pMib->qosRuleArray[i].local_port_end);
		pMib->qosRuleArray[i].remote_port_start = WORD_SWAP(pMib->qosRuleArray[i].remote_port_start);
		pMib->qosRuleArray[i].remote_port_end = WORD_SWAP(pMib->qosRuleArray[i].remote_port_end);
	}
#endif

#ifdef QOS_BY_BANDWIDTH
	pMib->qosManualUplinkSpeed = DWORD_SWAP(pMib->qosManualUplinkSpeed);	
	pMib->qosManualDownLinkSpeed = DWORD_SWAP(pMib->qosManualDownLinkSpeed);	

	for (i=0; i<pMib->qosRuleNum; i++) {
		pMib->qosRuleArray[i].bandwidth = DWORD_SWAP(pMib->qosRuleArray[i].bandwidth);
	}
#endif
}
#endif

// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT
	voip_mibtbl_swap_value(&pMib->voipCfgParam);
#endif
}
#else
static int _mib_swap_value(const mib_table_entry_T *mib, void *data)
{
	short *pShort;
	int *pInt;

	switch (mib->type)
	{
	case WORD_T:
		pShort = (short *) data;
		*pShort = htons(*pShort);
		break;
	case DWORD_T:
		pInt = (int *) data;
		*pInt = htonl(*pInt);
		break;
	default:
		break;
	}

	return 0;
}

static int _mibtbl_swap_value(const mib_table_entry_T *mib_tbl, void *data, int offset)
{
	int i, j;
	const mib_table_entry_T *mib;
	int new_offset;

	for (i=0; mib_tbl[i].id; i++)
	{
		mib = &mib_tbl[i];
		new_offset = offset + mib->offset;
		for (j=0; j<(mib->total_size / mib->unit_size); j++)
		{
			if (mib->type >= TABLE_LIST_T)
			{
				if (_mibtbl_swap_value(mib->next_mib_table, data, new_offset) != 0)
				{
					fprintf(stderr, "MIB (%s, %d, %d) Error: swap failed\n",
						mib_tbl[i].name, mib_tbl[i].total_size, mib_tbl[i].unit_size);
					return -1;
				}
			}
			else
			{
				_mib_swap_value(mib, (void *)((int) data + new_offset));
			}
			new_offset += mib->unit_size;
		}
	}

	return 0;
}

void swap_mib_word_value(APMIB_Tp pMib)
{
	mib_table_entry_T *pmib_tl;

	pmib_tl = mib_get_table(CURRENT_SETTING);
	_mibtbl_swap_value(pmib_tl, pMib, 0);
#ifdef VOIP_SUPPORT
	voip_mibtbl_swap_value(&pMib->voipCfgParam);
#endif
}
#endif // if 0
#endif // _LITTLE_ENDIAN_

///////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
static int updateConfigIntoFlash(unsigned char *data, int total_len, int *pType, int *pStatus)
{
	int len=0, status=1, type=0, ver, force;
	PARAM_HEADER_Tp pHeader;
#ifdef COMPRESS_MIB_SETTING
	COMPRESS_MIB_HEADER_Tp pCompHeader;
	unsigned char *expFile=NULL;
	unsigned int expandLen=0;
	int complen=0;
#endif
	char *ptr;
	unsigned char isValidfw = 0;

	do {
		if (
#ifdef COMPRESS_MIB_SETTING
			memcmp(&data[complen], COMP_HS_SIGNATURE, COMP_SIGNATURE_LEN) &&
			memcmp(&data[complen], COMP_DS_SIGNATURE, COMP_SIGNATURE_LEN) &&
			memcmp(&data[complen], COMP_CS_SIGNATURE, COMP_SIGNATURE_LEN)
#else
			memcmp(&data[len], CURRENT_SETTING_HEADER_TAG, TAG_LEN) &&
			memcmp(&data[len], CURRENT_SETTING_HEADER_FORCE_TAG, TAG_LEN) &&
			memcmp(&data[len], CURRENT_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) &&
			memcmp(&data[len], DEFAULT_SETTING_HEADER_TAG, TAG_LEN) &&
			memcmp(&data[len], DEFAULT_SETTING_HEADER_FORCE_TAG, TAG_LEN) &&
			memcmp(&data[len], DEFAULT_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) &&
			memcmp(&data[len], HW_SETTING_HEADER_TAG, TAG_LEN) &&
			memcmp(&data[len], HW_SETTING_HEADER_FORCE_TAG, TAG_LEN) &&
			memcmp(&data[len], HW_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) 
#endif
		) {
			if (isValidfw == 1)
				break;
		}
		
#ifdef COMPRESS_MIB_SETTING
		pCompHeader =(COMPRESS_MIB_HEADER_Tp)&data[complen];
#ifdef _LITTLE_ENDIAN_
		pCompHeader->compRate = WORD_SWAP(pCompHeader->compRate);
		pCompHeader->compLen = DWORD_SWAP(pCompHeader->compLen);
#endif
		/*decompress and get the tag*/
		expFile=malloc(pCompHeader->compLen*pCompHeader->compRate);
		if (NULL==expFile) {
			printf("malloc for expFile error!!\n");
			return 0;
		}
		expandLen = Decode(data+complen+sizeof(COMPRESS_MIB_HEADER_T), pCompHeader->compLen, expFile);
		pHeader = (PARAM_HEADER_Tp)expFile;
#else
		pHeader = (PARAM_HEADER_Tp)&data[len];
#endif
		
#ifdef _LITTLE_ENDIAN_
		pHeader->len = WORD_SWAP(pHeader->len);
#endif
		len += sizeof(PARAM_HEADER_T);

		if ( sscanf((char *)&pHeader->signature[TAG_LEN], "%02d", &ver) != 1)
			ver = -1;
			
		force = -1;
		if ( !memcmp(pHeader->signature, CURRENT_SETTING_HEADER_TAG, TAG_LEN) ) {
			isValidfw = 1;
			force = 1; // update
		}
		else if ( !memcmp(pHeader->signature, CURRENT_SETTING_HEADER_FORCE_TAG, TAG_LEN)) {
			isValidfw = 1;
			force = 2; // force
		}
		else if ( !memcmp(pHeader->signature, CURRENT_SETTING_HEADER_UPGRADE_TAG, TAG_LEN)) {
			isValidfw = 1;
			force = 0; // upgrade
		}

		if ( force >= 0 ) {
#if 0
			if ( !force && (ver < CURRENT_SETTING_VER || // version is less than current
				(pHeader->len < (sizeof(APMIB_T)+1)) ) { // length is less than current
				status = 0;
				break;
			}
#endif

#ifdef COMPRESS_MIB_SETTING
			ptr = (char *)(expFile+sizeof(PARAM_HEADER_T));
#else
			ptr = &data[len];
#endif

#ifdef COMPRESS_MIB_SETTING
#else
			DECODE_DATA(ptr, pHeader->len);
#endif
			if ( !CHECKSUM_OK((unsigned char *)ptr, pHeader->len)) {
				status = 0;
				break;
			}
#ifdef _LITTLE_ENDIAN_
			swap_mib_word_value((APMIB_Tp)ptr);
#endif

// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT
			flash_voip_import_fix(&((APMIB_Tp)ptr)->voipCfgParam, &pMib->voipCfgParam);
#endif

#ifdef COMPRESS_MIB_SETTING
			apmib_updateFlash(CURRENT_SETTING, (char *)&data[complen], pCompHeader->compLen+sizeof(COMPRESS_MIB_HEADER_T), force, ver);
#else
			apmib_updateFlash(CURRENT_SETTING, ptr, pHeader->len-1, force, ver);
#endif

#ifdef COMPRESS_MIB_SETTING
			complen += pCompHeader->compLen+sizeof(COMPRESS_MIB_HEADER_T);
			if (expFile) {
				free(expFile);
				expFile=NULL;
			}
#else
			len += pHeader->len;
#endif
			type |= CURRENT_SETTING;
			continue;
		}


		if ( !memcmp(pHeader->signature, DEFAULT_SETTING_HEADER_TAG, TAG_LEN) ) {
			isValidfw = 1;
			force = 1;	// update
		}
		else if ( !memcmp(pHeader->signature, DEFAULT_SETTING_HEADER_FORCE_TAG, TAG_LEN) ) {
			isValidfw = 1;
			force = 2;	// force
		}
		else if ( !memcmp(pHeader->signature, DEFAULT_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) ) {
			isValidfw = 1;
			force = 0;	// upgrade
		}

		if ( force >= 0 ) {
#if 0
			if ( (ver < DEFAULT_SETTING_VER) || // version is less than current
				(pHeader->len < (sizeof(APMIB_T)+1)) ) { // length is less than current
				status = 0;
				break;
			}
#endif

#ifdef COMPRESS_MIB_SETTING
			ptr = (char *)(expFile+sizeof(PARAM_HEADER_T));
#else
			ptr = &data[len];
#endif

#ifdef COMPRESS_MIB_SETTING
#else
			DECODE_DATA(ptr, pHeader->len);
#endif
			if ( !CHECKSUM_OK((unsigned char *)ptr, pHeader->len)) {
				status = 0;
				break;
			}

#ifdef _LITTLE_ENDIAN_
			swap_mib_word_value((APMIB_Tp)ptr);
#endif

// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT
			flash_voip_import_fix(&((APMIB_Tp)ptr)->voipCfgParam, &pMibDef->voipCfgParam);
#endif

#ifdef COMPRESS_MIB_SETTING
			apmib_updateFlash(DEFAULT_SETTING, (char *)&data[complen], pCompHeader->compLen+sizeof(COMPRESS_MIB_HEADER_T), force, ver);
#else
			apmib_updateFlash(DEFAULT_SETTING, ptr, pHeader->len-1, force, ver);
#endif

#ifdef COMPRESS_MIB_SETTING
			complen += pCompHeader->compLen+sizeof(COMPRESS_MIB_HEADER_T);
			if (expFile) {
				free(expFile);
				expFile=NULL;
			}	
#else
			len += pHeader->len;
#endif
			type |= DEFAULT_SETTING;
			continue;
		}

		if ( !memcmp(pHeader->signature, HW_SETTING_HEADER_TAG, TAG_LEN) ) {
			isValidfw = 1;
			force = 1;	// update
		}
		else if ( !memcmp(pHeader->signature, HW_SETTING_HEADER_FORCE_TAG, TAG_LEN) ) {
			isValidfw = 1;
			force = 2;	// force
		}
		else if ( !memcmp(pHeader->signature, HW_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) ) {
			isValidfw = 1;
			force = 0;	// upgrade
		}

		if ( force >= 0 ) {
#if 0
			if ( (ver < HW_SETTING_VER) || // version is less than current
				(pHeader->len < (sizeof(HW_SETTING_T)+1)) ) { // length is less than current
				status = 0;
				break;
			}
#endif
#ifdef COMPRESS_MIB_SETTING
			ptr = (char *)(expFile+sizeof(PARAM_HEADER_T));
#else
			ptr = &data[len];
#endif
			

#ifdef COMPRESS_MIB_SETTING
#else
			DECODE_DATA(ptr, pHeader->len);
#endif
			if ( !CHECKSUM_OK((unsigned char *)ptr, pHeader->len)) {
				status = 0;
				break;
			}
#ifdef COMPRESS_MIB_SETTING
			apmib_updateFlash(HW_SETTING, (char *)&data[complen], pCompHeader->compLen+sizeof(COMPRESS_MIB_HEADER_T), force, ver);
#else
			apmib_updateFlash(HW_SETTING, ptr, pHeader->len-1, force, ver);
#endif

#ifdef COMPRESS_MIB_SETTING
			complen += pCompHeader->compLen+sizeof(COMPRESS_MIB_HEADER_T);
			if (expFile) {
				free(expFile);
				expFile=NULL;
			}
#else
			len += pHeader->len;
#endif

			type |= HW_SETTING;
			continue;
		}
	}
#ifdef COMPRESS_MIB_SETTING	
	while (complen < total_len);

	if (expFile) {
		free(expFile);
		expFile=NULL;
	}
#else
	while (len < total_len);
#endif

	*pType = type;
	*pStatus = status;
#ifdef COMPRESS_MIB_SETTING	
	return complen;
#else
	return len;
#endif
}

///////////////////////////////////////////////////////////////////////////////
/*
void sig_alm(int signo)
{
	if(isUpgrade_OK ==1){	
		reboot( RB_AUTOBOOT);
		return;
	}

}
*/
///////////////////////////////////////////////////////////////////////////////
#ifdef CONFIG_SNMP
void formSetSNMP(request *wp, char *path, char *query)
{
	
		char *submitUrl;
        char *strValue;
        int     snmpEnabled;
        struct in_addr ip;
        char tmpBuf[100];
		submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page

        strValue = (char *)req_get_cstream_var(wp, ("snmp_enabled"), "");
		if(!strcmp(strValue, "ON")){
                snmpEnabled = 1;
        } else {
                snmpEnabled = 0;
        }
        if (!apmib_set(MIB_SNMP_ENABLED, (void *)&snmpEnabled)) {
                strcpy(tmpBuf, ("Set SNMP enabled error!"));
                goto setErr;
        }

        strValue = (char *)req_get_cstream_var(wp, ("snmp_name"), "");
        if (strValue[0]) {
                if (!apmib_set(MIB_SNMP_NAME, (void *)strValue)) {
                        strcpy(tmpBuf, ("Set SNMP location error!"));
                        goto setErr;
                }
        }

        strValue = (char *)req_get_cstream_var(wp, ("snmp_location"), "");
        if (strValue[0]) {
                if (!apmib_set(MIB_SNMP_LOCATION, (void *)strValue)) {
                        strcpy(tmpBuf, ("Set SNMP location error!"));
                        goto setErr;
                }
        }

        strValue = (char *)req_get_cstream_var(wp, ("snmp_contact"), "");
        if (strValue[0]) {
                if (!apmib_set(MIB_SNMP_CONTACT, (void *)strValue)) {
                        strcpy(tmpBuf, ("Set SNMP contact error!"));
                        goto setErr;
                }
        }
		
        strValue = (char *)req_get_cstream_var(wp, ("snmp_rwcommunity"), "");
        if (strValue[0]) {
                if (!apmib_set(MIB_SNMP_RWCOMMUNITY, (void *)strValue)) {
                        strcpy(tmpBuf, ("Set SNMP community error!"));
                        goto setErr;
                }
        }


        strValue = (char *)req_get_cstream_var(wp, ("snmp_rocommunity"), "");
        if (strValue[0]) {
                if (!apmib_set(MIB_SNMP_ROCOMMUNITY, (void *)strValue)) {
                        strcpy(tmpBuf, ("Set SNMP community error!"));
                        goto setErr;
                }
        }

        strValue = (char *)req_get_cstream_var(wp, ("snmp_trap1"), "");
        if (strValue[0]) {
                if (!inet_aton(strValue, &ip) ) {
                        strcpy(tmpBuf, ("Invalid Trap Receiver 1 IP-address value!"));
                        goto setErr;
                }
                if (!apmib_set(MIB_SNMP_TRAP_RECEIVER1, (void *)&ip)) {
                        strcpy(tmpBuf, ("Set Trap Receiver 1 IP-address error!"));
                        goto setErr;
                }
        }

        strValue = (char *)req_get_cstream_var(wp, ("snmp_trap2"), "");
        if (strValue[0]) {
                if (!inet_aton(strValue, &ip) ) {
                        strcpy(tmpBuf, ("Invalid Trap Receiver 2 IP-address value!"));
                        goto setErr;
                }
                if (!apmib_set(MIB_SNMP_TRAP_RECEIVER2, (void *)&ip)) {
                        strcpy(tmpBuf, ("Set Trap Receiver 2 IP-address error!"));
                        goto setErr;
                }
        }

        strValue = (char *)req_get_cstream_var(wp, ("snmp_trap3"), "");
        if (strValue[0]) {
                if (!inet_aton(strValue, &ip) ) {
                        strcpy(tmpBuf, ("Invalid Trap Receiver 3 IP-address value!"));
                        goto setErr;
                }
                if (!apmib_set(MIB_SNMP_TRAP_RECEIVER3, (void *)&ip)) {
                        strcpy(tmpBuf, ("Set Trap Receiver 3 IP-address error!"));
                        goto setErr;
                }
        }

        apmib_update_web(CURRENT_SETTING);

#ifndef NO_ACTION
        run_init_script("all");
#endif
		OK_MSG(submitUrl);
        return;

setErr:
		ERR_MSG(tmpBuf);
}
#endif /* CONFIG_SNMP */

///////////////////////////////////////////////////////////////////////////////
void formSaveConfig(request *wp, char *path, char *query)
{
	char *strRequest;
	char *buf, *ptr=NULL;
	unsigned char checksum;
	int len, len1;
	char tmpBuf[200];
	CONFIG_DATA_T type=0;
	//char *submitUrl;
	char lan_ip_buf[30], lan_ip[30];
	
	len1 = sizeof(PARAM_HEADER_T) + sizeof(APMIB_T) + sizeof(checksum) + 100;  // 100 for expansion
	len = csHeader.len;
#ifdef _LITTLE_ENDIAN_
#ifdef VOIP_SUPPORT
	// rock: don't need swap here
	// 1. write to private space (ex: flash)
	// 2. read from private space (ex: flash)
#else
	len  = WORD_SWAP(len);
#endif
#endif
	len += sizeof(PARAM_HEADER_T) + 100;
	if (len1 > len)
		len = len1;

	buf = malloc(len);
	if ( buf == NULL ) {
		strcpy(tmpBuf, "Allocate buffer failed!");		
		goto back;
	}

	strRequest = req_get_cstream_var(wp, ("save-cs"), "");
	if (strRequest[0])
		type |= CURRENT_SETTING;

	strRequest = req_get_cstream_var(wp, ("save"), "");
	if (strRequest[0])
		type |= CURRENT_SETTING;

	strRequest = req_get_cstream_var(wp, ("save-hs"), "");
	if (strRequest[0])
		type |= HW_SETTING;

	strRequest = req_get_cstream_var(wp, ("save-ds"), "");
	if (strRequest[0])
		type |= DEFAULT_SETTING;

	strRequest = req_get_cstream_var(wp, ("save-all"), "");
	if (strRequest[0])
		type |= HW_SETTING | DEFAULT_SETTING | CURRENT_SETTING;
	if (type) {
		send_redirect_perm(wp, "/config.dat");
		return;
	}

	strRequest = req_get_cstream_var(wp, ("reset"), "");
	if (strRequest[0] && strcmp(strRequest,"Reset") == 0) {
		if ( !apmib_updateDef() ) {
			free(ptr);
			strcpy(tmpBuf, "Write default to current setting failed!\n");
			free(buf);
			goto back;
		}
#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
		//To clear 802.1x certs
		//RunSystemCmd(NULL_FILE, "rsCert","-rst", NULL_STR);
		system("rsCert -rst");
#endif
#ifdef CONFIG_RTL_WAPI_SUPPORT
		//To clear CA files
		system("storeWapiFiles -reset");
#endif

#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_POCKET_AP_SUPPORT)
		Reboot_Wait = 60;
#else
		Reboot_Wait = 40;
#endif
#ifdef HOME_GATEWAY
		sprintf(tmpBuf, "%s","Reload setting successfully!<br><br>The Router is booting.<br>Do not turn off or reboot the Device during this time.<br>");
#else
		sprintf(tmpBuf, "%s", "Reload setting successfully!<br><br>The AP is booting.<br>");
#endif
		//ERR_MSG(tmpBuf);
		apmib_reinit();
		apmib_update_web(CURRENT_SETTING);	// update configuration to flash
		apmib_get( MIB_IP_ADDR,  (void *)lan_ip_buf);
		sprintf(lan_ip,"%s",inet_ntoa(*((struct in_addr *)lan_ip_buf)) );
		OK_MSG_FW(tmpBuf, submitUrl,Reboot_Wait,lan_ip);
		if(ptr != NULL) {
			free(ptr);
		}
		/* Reboot DUT. Keith */
		isUpgrade_OK=1;
		system("reboot");
		return;
	}

back:
	ERR_MSG(tmpBuf);
	return;
}

void formUploadConfig(request *wp, char *path, char *query)
{
	int status=0;
	char tmpBuf[200];
	CONFIG_DATA_T type=0;
	char *submitUrl;
	char lan_ip_buf[30], lan_ip[30];
	int head_offset=0;
	
	head_offset = find_head_offset((char *)wp->upload_data);
	//fprintf(stderr,"####%s:%d head_offset=%d###\n",  __FILE__, __LINE__ , head_offset);
	if (head_offset == -1) {
		strcpy(tmpBuf, "Invalid file format!");
		goto back;
	}
	if(
#ifdef COMPRESS_MIB_SETTING
		!memcmp(&wp->upload_data[head_offset], COMP_HS_SIGNATURE, COMP_SIGNATURE_LEN) ||
		!memcmp(&wp->upload_data[head_offset], COMP_DS_SIGNATURE, COMP_SIGNATURE_LEN) ||
		!memcmp(&wp->upload_data[head_offset], COMP_CS_SIGNATURE, COMP_SIGNATURE_LEN)
#else
		!memcmp(&wp->upload_data[head_offset], CURRENT_SETTING_HEADER_TAG, TAG_LEN) ||
		!memcmp(&wp->upload_data[head_offset], CURRENT_SETTING_HEADER_FORCE_TAG, TAG_LEN) ||
		!memcmp(&wp->upload_data[head_offset], CURRENT_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) ||
		!memcmp(&wp->upload_data[head_offset], DEFAULT_SETTING_HEADER_TAG, TAG_LEN) ||
		!memcmp(&wp->upload_data[head_offset], DEFAULT_SETTING_HEADER_FORCE_TAG, TAG_LEN) ||
		!memcmp(&wp->upload_data[head_offset], DEFAULT_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) ||
		!memcmp(&wp->upload_data[head_offset], HW_SETTING_HEADER_TAG, TAG_LEN) ||
		!memcmp(&wp->upload_data[head_offset], HW_SETTING_HEADER_FORCE_TAG, TAG_LEN) ||
		!memcmp(&wp->upload_data[head_offset], HW_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) 
#endif
	) {
		updateConfigIntoFlash((unsigned char *)&wp->upload_data[head_offset], (wp->upload_len-head_offset), (int *)&type, &status);
	}
	if (status == 0 || type == 0) { // checksum error
		strcpy(tmpBuf, "Invalid configuration file!");
		goto back;
	}
	else {
		if (type) { // upload success
#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
			//To clear 802.1x certs
			//RunSystemCmd(NULL_FILE, "rsCert","-rst", NULL_STR);
			system("rsCert -rst");
#endif
#ifdef CONFIG_RTL_WAPI_SUPPORT 
			//To clear CA files
			system("storeWapiFiles -reset");
#endif
		}

#ifdef HOME_GATEWAY
		sprintf(tmpBuf, ("%s"), "Update successfully!<br><br>Update in progressing.<br>Do not turn off or reboot the Device during this time.<br>");
#else
		sprintf(tmpBuf, ("%s"), "Update successfully!<br><br>Update in progress.<br> Do not turn off or reboot the AP during this time.");
#endif			
		Reboot_Wait = 45;
		submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page
			
		apmib_reinit();
		apmib_update_web(CURRENT_SETTING);	// update configuration to flash
		apmib_get( MIB_IP_ADDR,  (void *)lan_ip_buf) ;
		sprintf(lan_ip,"%s",inet_ntoa(*((struct in_addr *)lan_ip_buf)) );
#ifdef REBOOT_CHECK
		sprintf(lastUrl,"%s",submitUrl);
		sprintf(okMsg,"%s",tmpBuf);
		countDownTime = Reboot_Wait;
		send_redirect_perm(wp, COUNTDOWN_PAGE);
		/*Reboot DUT in main loop*/
		isCFGUPGRADE=1;
#else
		OK_MSG_FW(tmpBuf, submitUrl,Reboot_Wait,lan_ip);
		
		/* Reboot DUT. Keith */
		isUpgrade_OK=1;
		system("reboot");
#endif		
		return;
	}
back:
	ERR_MSG(tmpBuf);
	return;
}
///////////////////////////////////////////////////////////////////////////////

#if 0 //Keith. move to utility.c
void kill_processes(void)
{


	printf("upgrade: killing tasks...\n");
	
	kill(1, SIGTSTP);		/* Stop init from reforking tasks */
	kill(1, SIGSTOP);		
	kill(2, SIGSTOP);		
	kill(3, SIGSTOP);		
	kill(4, SIGSTOP);		
	kill(5, SIGSTOP);		
	kill(6, SIGSTOP);		
	kill(7, SIGSTOP);		
	//atexit(restartinit);		/* If exit prematurely, restart init */
	sync();

	signal(SIGTERM,SIG_IGN);	/* Don't kill ourselves... */
	setpgrp(); 			/* Don't let our parent kill us */
	sleep(1);
	signal(SIGHUP, SIG_IGN);	/* Don't die if our parent dies due to
					 * a closed controlling terminal */
	
}
#endif //#if 0 //Keith. move to utility.c

//////////////////////////////////////////////////////////////////////////////
#ifdef CONFIG_RTL_FLASH_DUAL_IMAGE_ENABLE

#define SQSH_SIGNATURE		((char *)"sqsh")
#define SQSH_SIGNATURE_LE       ((char *)"hsqs")

#define IMAGE_ROOTFS 2
#define IMAGE_KERNEL 1
#define GET_BACKUP_BANK 2
#define GET_ACTIVE_BANK 1

#define GOOD_BANK_MARK_MASK 0x80000000  //goo abnk mark must set bit31 to 1

#define NO_IMAGE_BANK_MARK 0x80000000  
#define OLD_BURNADDR_BANK_MARK 0x80000001 
#define BASIC_BANK_MARK 0x80000002           
#define FORCEBOOT_BANK_MARK 0xFFFFFFF0  //means always boot/upgrade in this bank

char *Kernel_dev_name[2]=
 {
   "/dev/mtdblock0", "/dev/mtdblock2"
 };
char *Rootfs_dev_name[2]=
 {
   "/dev/mtdblock1", "/dev/mtdblock3"
 };

static int get_actvie_bank()
{
	FILE *fp;
	char buffer[2];
	int bootbank;
	fp = fopen("/proc/bootbank", "r");
	
	if (!fp) {
		fprintf(stderr,"%s\n","Read /proc/bootbank failed!\n");
	}else
	{
			//fgets(bootbank, sizeof(bootbank), fp);
			fgets(buffer, sizeof(buffer), fp);
			fclose(fp);
	}
	bootbank = buffer[0] - 0x30;	
	if ( bootbank ==1 || bootbank ==2)
		return bootbank;
	else
		return 1;	
}

void get_bank_info(int dual_enable,int *active,int *backup)
{
	int bootbank=0,backup_bank;
	
	bootbank = get_actvie_bank();	

	if(bootbank == 1 )
	{
		if( dual_enable ==0 )
			backup_bank =1;
		else
			backup_bank =2;
	}
	else if(bootbank == 2 )
	{
		if( dual_enable ==0 )
			backup_bank =2;
		else
			backup_bank =1;
	}
	else
	{
		bootbank =1 ;
		backup_bank =1 ;
	}	

	*active = bootbank;
	*backup = backup_bank;	

	//fprintf(stderr,"get_bank_info active_bank =%d , backup_bank=%d  \n",*active,*backup); //mark_debug	   
}
static unsigned long header_to_mark(int  flag, IMG_HEADER_Tp pHeader)
{
	unsigned long ret_mark=NO_IMAGE_BANK_MARK;
	//mark_dual ,  how to diff "no image" "image with no bank_mark(old)" , "boot with lowest priority"
	if(flag) //flag ==0 means ,header is illegal
	{
		if( (pHeader->burnAddr & GOOD_BANK_MARK_MASK) )
			ret_mark=pHeader->burnAddr;	
		else
			ret_mark = OLD_BURNADDR_BANK_MARK;
	}
	return ret_mark;
}

// return,  0: not found, 1: linux found, 2:linux with root found
static int check_system_image(int fh,IMG_HEADER_Tp pHeader)
{
	// Read header, heck signature and checksum
	int i, ret=0;		
	char image_sig[4]={0};
	char image_sig_root[4]={0};
	
        /*check firmware image.*/
	if ( read(fh, pHeader, sizeof(IMG_HEADER_T)) != sizeof(IMG_HEADER_T)) 
     		return 0;	
	
	memcpy(image_sig, FW_HEADER, SIGNATURE_LEN);
	memcpy(image_sig_root, FW_HEADER_WITH_ROOT, SIGNATURE_LEN);

	if (!memcmp(pHeader->signature, image_sig, SIGNATURE_LEN))
		ret=1;
	else if  (!memcmp(pHeader->signature, image_sig_root, SIGNATURE_LEN))
		ret=2;
	else{
		printf("no sys signature at !\n");
	}				
       //mark_dual , ignore checksum() now.(to do) 
	return (ret);
}

static int check_rootfs_image(int fh)
{
	// Read header, heck signature and checksum
	int i;
	unsigned short sum=0, *word_ptr;
	unsigned long length=0;
	unsigned char rootfs_head[SIGNATURE_LEN];		
	
	if ( read(fh, &rootfs_head, SIGNATURE_LEN ) != SIGNATURE_LEN ) 
     		return 0;	
	
	if ( memcmp(rootfs_head, SQSH_SIGNATURE, SIGNATURE_LEN) && memcmp(rootfs_head, SQSH_SIGNATURE_LE, SIGNATURE_LEN)) {
		printf("no rootfs signature at !\n");
		return 0;
	}
	
	return 1;
}

static int get_image_header(int fh,IMG_HEADER_Tp header_p)
{
	int ret=0;
	//check 	CODE_IMAGE_OFFSET2 , CODE_IMAGE_OFFSET3 ?
	//ignore check_image_header () for fast get header , assume image are same offset......	
	// support CONFIG_RTL_FLASH_MAPPING_ENABLE ? , scan header ...

	lseek(fh, CODE_IMAGE_OFFSET, SEEK_SET);		
	ret = check_system_image(fh,header_p);

	//assume , we find the image header in CODE_IMAGE_OFFSET
	lseek(fh, CODE_IMAGE_OFFSET, SEEK_SET);	
	
	return ret;	
}

 int check_bank_image(int bank)
{
	int i,ret=0;	
    	int fh,fh_rootfs;
	char *rootfs_dev = Rootfs_dev_name[bank-1];	
	char *kernel_dev = Kernel_dev_name[bank-1];	
	IMG_HEADER_T header;
           	
	fh = open(kernel_dev, O_RDONLY);
	if ( fh == -1 ) {
      		printf("Open file failed!\n");
		return 0;
	}
	ret = get_image_header(fh,&header);			
	
	close(fh);	
	if(ret==2)
        {	
	      	fh_rootfs = open(rootfs_dev, O_RDONLY);
		if ( fh_rootfs == -1 ) {
      		printf("Open file failed!\n");
		return 0;
		}
              ret=check_rootfs_image(fh_rootfs);
		close(fh_rootfs);	  
	  }
	return ret;
}

int write_header_bankmark(char *kernel_dev, unsigned long bankmark)
{
	int ret=0,fh,numWrite;
	IMG_HEADER_T header,*header_p;
	char buffer[200]; //mark_debug
	
	header_p = &header;
	fh = open(kernel_dev, O_RDWR);

	if ( fh == -1 ) {
      		printf("Open file failed!\n");
		return -1;
	}
	ret = get_image_header(fh,&header);

	if(!ret)
		return -2; //can't find active(current) imager header ...something wrong

	//fh , has been seek to correct offset	

	header_p->burnAddr = bankmark;

	//sprintf(buffer, ("write_header_bankmark kernel_dev =%s , bankmark=%x \n"), kernel_dev , header_p->burnAddr);
       //fprintf(stderr, "%s\n", buffer); //mark_debug	
       
	 //move to write image header will be done in get_image_header
	numWrite = write(fh, (char *)header_p, sizeof(IMG_HEADER_T));
	
	close(fh);
	
	return 0;	//success
}

// return,  0: not found, 1: linux found, 2:linux with root found

unsigned long get_next_bankmark(char *kernel_dev,int dual_enable)
{
    unsigned long bankmark=NO_IMAGE_BANK_MARK;
    int ret=0,fh;
    IMG_HEADER_T header; 	
	
	fh = open(kernel_dev, O_RDONLY);
	if ( fh == -1 ) {
      		fprintf(stderr,"%s\n","Open file failed!\n");
		return NO_IMAGE_BANK_MARK;
	}
	ret = get_image_header(fh,&header);	

	//fprintf(stderr,"get_next_bankmark = %s , ret = %d \n",kernel_dev,ret); //mark_debug

	bankmark= header_to_mark(ret, &header);	
	close(fh);
	//get next boot mark

	if( bankmark < BASIC_BANK_MARK)
		return BASIC_BANK_MARK;
	else if( (bankmark ==  FORCEBOOT_BANK_MARK) || (dual_enable == 0)) //dual_enable = 0 ....	 	
		return FORCEBOOT_BANK_MARK;
	else
		return bankmark+1;  
	
}

// set mib at the same time or get mib to set this function? 
int set_dualbank(int enable)
{	
	int ret =0, active_bank=0, backup_bank=0;
	unsigned long bankmark=0;		

	get_bank_info(enable,&active_bank,&backup_bank);    	
	if(enable)
	{
		//set_to mib to 1.??		
		bankmark = get_next_bankmark(Kernel_dev_name[backup_bank-1],enable);		
		ret = write_header_bankmark(Kernel_dev_name[active_bank-1], bankmark);
	}
	else //disable this
	{
		//set_to mib to 0 .??		
		ret = write_header_bankmark(Kernel_dev_name[active_bank-1], FORCEBOOT_BANK_MARK);		
	}	
	if(!ret)
	{
   	       apmib_set( MIB_DUALBANK_ENABLED, (void *)&enable);
		//fprintf(stderr,"set_dualbank enable =%d ,ret2 =%d  \n",enable,ret2); //mark_debug			
	}
	
	return ret; //-1 fail , 0 : ok
}

// need to reject this function if dual bank is disable
int  boot_from_backup()
{
	int ret =0, active_bank=0, backup_bank=0;
	unsigned long bankmark=0;	

	get_bank_info(1,&active_bank,&backup_bank);    

	ret = check_bank_image(backup_bank);	
	if(!ret)
	    return -2;			
	bankmark = get_next_bankmark(Kernel_dev_name[active_bank-1],1);
	
	ret = write_header_bankmark(Kernel_dev_name[backup_bank-1], bankmark);

	return ret; //-2 , no kernel , -1 fail , 0 : ok}
}
#endif

int find_head_offset(char *upload_data)
{
	int head_offset=0 ;
	char *pStart=NULL;
	int iestr_offset=0;
	char *dquote;
	char *dquote1;
	
	if (upload_data==NULL) {
		//fprintf(stderr, "upload data is NULL\n");
		return -1;
	}

	pStart = strstr(upload_data, WINIE6_STR);
	if (pStart == NULL) {
		pStart = strstr(upload_data, LINUXFX36_FWSTR);
		if (pStart == NULL) {
			pStart = strstr(upload_data, MACIE5_FWSTR);
			if (pStart == NULL) {
				pStart = strstr(upload_data, OPERA_FWSTR);
				if (pStart == NULL) {
					pStart = strstr(upload_data, "filename=");
					if (pStart == NULL) {
						return -1;
					}
					else {
						dquote =  strstr(pStart, "\"");
						if (dquote !=NULL) {
							dquote1 = strstr(dquote, LINE_FWSTR);
							if (dquote1!=NULL) {
								iestr_offset = 4;
								pStart = dquote1;
							}
							else {
								return -1;
							}
						}
						else {
							return -1;
						}
					}
				}
				else {
					iestr_offset = 16;
				}
			} 
			else {
				iestr_offset = 14;
			}
		}
		else {
			iestr_offset = 26;
		}
	}
	else {
		iestr_offset = 17;
	}
	//fprintf(stderr,"####%s:%d %d###\n",  __FILE__, __LINE__ , iestr_offset);
	head_offset = (int)(((unsigned long)pStart)-((unsigned long)upload_data)) + iestr_offset;
	return head_offset;
}

int FirmwareUpgrade(char *upload_data, int upload_len, int is_root, char *buffer)
{
	int head_offset=0 ;
	int isIncludeRoot=0;
	int		 len;
	int          locWrite;
	int          numLeft;
	int          numWrite;
	IMG_HEADER_Tp pHeader;
	int flag=0, startAddr=-1, startAddrWeb=-1;
	int update_fw=0, update_cfg=0;
	int fh;
	//unsigned char cmdBuf[30];
	//Support WAPI/openssl, the flash MUST up to 4m
/*
#if defined(CONFIG_RTL_WAPI_SUPPORT) || defined(HTTP_FILE_SERVER_SUPPORTED) || defined(CONFIG_APP_TR069)
	int fwSizeLimit = 0x400000;
#elif defined( CONFIG_RTK_VOIP )
	int fwSizeLimit = 0x400000;
#else
	int fwSizeLimit = 0x200000;
#endif
*/
	int fwSizeLimit = CONFIG_FLASH_SIZE;
#ifdef CONFIG_RTL_FLASH_DUAL_IMAGE_ENABLE
	int active_bank,backup_bank;
	int dual_enable =0;
#endif
	unsigned char isValidfw = 0;



#if defined(CONFIG_APP_FWD)
#define FWD_CONF "/var/fwd.conf"
	int newfile = 1;
	extern int get_shm_id();
	extern int clear_fwupload_shm();
	int shm_id = get_shm_id();			
#endif

	if (isCFG_ONLY == 0) {
		/*
		#ifdef CONFIG_RTL_8196B
			sprintf(cmdBuf, "echo \"4 %d\" > /proc/gpio", (Reboot_Wait+12));
		#else	
			sprintf(cmdBuf, "echo \"4 %d\" > /proc/gpio", (Reboot_Wait+20));
		#endif
		
			system(cmdBuf);
		*/
		system("ifconfig br0 down 2> /dev/null");
	}
#ifdef CONFIG_RTL_FLASH_DUAL_IMAGE_ENABLE
	apmib_get(MIB_DUALBANK_ENABLED,(void *)&dual_enable);   
	get_bank_info(dual_enable,&active_bank,&backup_bank);        
#endif
	head_offset = find_head_offset(upload_data);
	//fprintf(stderr,"####%s:%d %d upload_data=%p###\n",  __FILE__, __LINE__ , head_offset, upload_data);
	if (head_offset == -1) {
		strcpy(buffer, "Invalid file format!");
		goto ret_upload;
	}
	while ((head_offset+sizeof(IMG_HEADER_T)) < upload_len) {
		locWrite = 0;
		pHeader = (IMG_HEADER_Tp) &upload_data[head_offset];
		len = pHeader->len;
#ifdef _LITTLE_ENDIAN_
		len  = DWORD_SWAP(len);
#endif
		numLeft = len + sizeof(IMG_HEADER_T) ;
		// check header and checksum
		if (!memcmp(&upload_data[head_offset], FW_HEADER, SIGNATURE_LEN) ||
			!memcmp(&upload_data[head_offset], FW_HEADER_WITH_ROOT, SIGNATURE_LEN)) {
			isValidfw = 1;
			flag = 1;
		}
		else if (!memcmp(&upload_data[head_offset], WEB_HEADER, SIGNATURE_LEN)) {
			isValidfw = 1;
			flag = 2;
		}
		else if (!memcmp(&upload_data[head_offset], ROOT_HEADER, SIGNATURE_LEN)) {
			isValidfw = 1;
			flag = 3;
			isIncludeRoot = 1;
		}else if (
#ifdef COMPRESS_MIB_SETTING
				!memcmp(&upload_data[head_offset], COMP_HS_SIGNATURE, COMP_SIGNATURE_LEN) ||
				!memcmp(&upload_data[head_offset], COMP_DS_SIGNATURE, COMP_SIGNATURE_LEN) ||
				!memcmp(&upload_data[head_offset], COMP_CS_SIGNATURE, COMP_SIGNATURE_LEN)
#else
				!memcmp(&upload_data[head_offset], CURRENT_SETTING_HEADER_TAG, TAG_LEN) ||
				!memcmp(&upload_data[head_offset], CURRENT_SETTING_HEADER_FORCE_TAG, TAG_LEN) ||
				!memcmp(&upload_data[head_offset], CURRENT_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) ||
				!memcmp(&upload_data[head_offset], DEFAULT_SETTING_HEADER_TAG, TAG_LEN) ||
				!memcmp(&upload_data[head_offset], DEFAULT_SETTING_HEADER_FORCE_TAG, TAG_LEN) ||
				!memcmp(&upload_data[head_offset], DEFAULT_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) ||
				!memcmp(&upload_data[head_offset], HW_SETTING_HEADER_TAG, TAG_LEN) ||
				!memcmp(&upload_data[head_offset], HW_SETTING_HEADER_FORCE_TAG, TAG_LEN) ||
				!memcmp(&upload_data[head_offset], HW_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) 
#endif			
				) {
			int type=0, status=0, cfg_len;
			cfg_len = updateConfigIntoFlash((unsigned char *)&upload_data[head_offset],configlen , &type, &status);

			if (status == 0 || type == 0) { // checksum error
				strcpy(buffer, "Invalid configuration file!");
				goto ret_upload;
			}
			else { // upload success
				strcpy(buffer, "Update successfully!");
				head_offset += cfg_len;
				isValidfw = 1;
				update_cfg = 1;
			}
			continue;
		}
		else {
			if (isValidfw == 1)
				break;
			strcpy(buffer, ("Invalid file format!"));
			goto ret_upload;
		}

		if (len > fwSizeLimit) { //len check by sc_yang 
			sprintf(buffer, ("Image len exceed max size 0x%x ! len=0x%x</b><br>"),fwSizeLimit, len);
			goto ret_upload;
		}
		if ( (flag == 1) || (flag == 3)) {
			if ( !fwChecksumOk(&upload_data[sizeof(IMG_HEADER_T)+head_offset], len)) {
				sprintf(buffer, ("Image checksum mismatched! len=0x%x, checksum=0x%x</b><br>"), len,
					*((unsigned short *)&upload_data[len-2]) );
				goto ret_upload;
			}
		}
		else {
			char *ptr = &upload_data[sizeof(IMG_HEADER_T)+head_offset];
			if ( !CHECKSUM_OK((unsigned char *)ptr, len) ) {
				sprintf(buffer, ("Image checksum mismatched! len=0x%x</b><br>"), len);
				goto ret_upload;
			}
		}

#ifndef CONFIG_RTL_FLASH_DUAL_IMAGE_ENABLE
		if (flag == 3)
		{
			fh = open(FLASH_DEVICE_NAME1, O_RDWR);

#if defined(CONFIG_APP_FWD)			
			write_line_to_file(FWD_CONF, (newfile==1?1:2), FLASH_DEVICE_NAME1);
			newfile = 2;
#endif			
		}
		else
		{
			fh = open(FLASH_DEVICE_NAME, O_RDWR);
#if defined(CONFIG_APP_FWD)			
			write_line_to_file(FWD_CONF, (newfile==1?1:2), FLASH_DEVICE_NAME);
			newfile = 2;
#endif			
		}
#else
		if (flag == 3) //rootfs
		{
			fh = open(Rootfs_dev_name[backup_bank-1], O_RDWR);
			
#if defined(CONFIG_APP_FWD)			
			write_line_to_file(FWD_CONF, (newfile==1?1:2), Rootfs_dev_name[backup_bank-1]);
			newfile = 2;
#endif			
		}
		else if (flag == 1) //linux
		{
			fh = open(Kernel_dev_name[backup_bank-1], O_RDWR);
#if defined(CONFIG_APP_FWD)			
			write_line_to_file(FWD_CONF, (newfile==1?1:2), Kernel_dev_name[backup_bank-1]);
			newfile = 2;
#endif			
		}
		else //web
		{
			fh = open(FLASH_DEVICE_NAME, O_RDWR);		
#if defined(CONFIG_APP_FWD)			
			write_line_to_file(FWD_CONF, (newfile==1?1:2), FLASH_DEVICE_NAME);
			newfile = 2;
#endif			
		}
#endif

		if ( fh == -1 ) {
			strcpy(buffer, ("File open failed!"));
		} else {
			if (flag == 1) {
				if (startAddr == -1) {
					//startAddr = CODE_IMAGE_OFFSET;
					startAddr = pHeader->burnAddr ;
#ifdef _LITTLE_ENDIAN_
					startAddr = DWORD_SWAP(startAddr);
#endif
				}
			}
			else if (flag == 3) {
				if (startAddr == -1) {
					startAddr = 0; // always start from offset 0 for 2nd FLASH partition
				}
			}
			else {
				if (startAddrWeb == -1) {
					//startAddr = WEB_PAGE_OFFSET;
					startAddr = pHeader->burnAddr ;
#ifdef _LITTLE_ENDIAN_
					startAddr = DWORD_SWAP(startAddr);
#endif
				}
				else
					startAddr = startAddrWeb;
			}
			lseek(fh, startAddr, SEEK_SET);
			
#if defined(CONFIG_APP_FWD)			
			{
				char tmpStr[20]={0};
				sprintf(tmpStr,"\n%d",startAddr);
				write_line_to_file(FWD_CONF, (newfile==1?1:2), tmpStr);
				newfile = 2;
			}
#endif			
			
			
						
			if (flag == 3) {
				locWrite += sizeof(IMG_HEADER_T); // remove header
				numLeft -=  sizeof(IMG_HEADER_T);
				system("ifconfig br0 down 2> /dev/null");
				system("ifconfig eth0 down 2> /dev/null");
				system("ifconfig eth1 down 2> /dev/null");
				system("ifconfig ppp0 down 2> /dev/null");
				system("ifconfig wlan0 down 2> /dev/null");
				system("ifconfig wlan0-vxd down 2> /dev/null");		
				system("ifconfig wlan0-va0 down 2> /dev/null");		
				system("ifconfig wlan0-va1 down 2> /dev/null");		
				system("ifconfig wlan0-va2 down 2> /dev/null");		
				system("ifconfig wlan0-va3 down 2> /dev/null");
				system("ifconfig wlan0-wds0 down 2> /dev/null");
				system("ifconfig wlan0-wds1 down 2> /dev/null");
				system("ifconfig wlan0-wds2 down 2> /dev/null");
				system("ifconfig wlan0-wds3 down 2> /dev/null");
				system("ifconfig wlan0-wds4 down 2> /dev/null");
				system("ifconfig wlan0-wds5 down 2> /dev/null");
				system("ifconfig wlan0-wds6 down 2> /dev/null");
				system("ifconfig wlan0-wds7 down 2> /dev/null");
#if defined(CONFIG_RTL_92D_SUPPORT)	
				system("ifconfig wlan1 down 2> /dev/null");
				system("ifconfig wlan1-vxd down 2> /dev/null");		
				system("ifconfig wlan1-va0 down 2> /dev/null");		
				system("ifconfig wlan1-va1 down 2> /dev/null");		
				system("ifconfig wlan1-va2 down 2> /dev/null");		
				system("ifconfig wlan1-va3 down 2> /dev/null");
				system("ifconfig wlan1-wds0 down 2> /dev/null");
				system("ifconfig wlan1-wds1 down 2> /dev/null");
				system("ifconfig wlan1-wds2 down 2> /dev/null");
				system("ifconfig wlan1-wds3 down 2> /dev/null");
				system("ifconfig wlan1-wds4 down 2> /dev/null");
				system("ifconfig wlan1-wds5 down 2> /dev/null");
				system("ifconfig wlan1-wds6 down 2> /dev/null");
				system("ifconfig wlan1-wds7 down 2> /dev/null");
#endif
				kill_processes();
				sleep(2);
			}
#ifdef CONFIG_RTL_FLASH_DUAL_IMAGE_ENABLE
			if (flag == 1) {  //kernel image
				pHeader->burnAddr = get_next_bankmark(Kernel_dev_name[active_bank-1],dual_enable);	//replace the firmware header with new bankmark //mark_debug		
			}
#endif

#if defined(CONFIG_APP_FWD)
			{
				char tmpStr[20]={0};
				sprintf(tmpStr,"\n%d",numLeft);
				write_line_to_file(FWD_CONF, (newfile==1?1:2), tmpStr);
				sprintf(tmpStr,"\n%d\n",locWrite+head_offset);
				write_line_to_file(FWD_CONF, (newfile==1?1:2), tmpStr);					
				newfile = 2;
			}

#else //#if defined(CONFIG_APP_FWD)
			numWrite = write(fh, &(upload_data[locWrite+head_offset]), numLeft);
			if (numWrite < numLeft) {
				sprintf(buffer, ("File write failed. locWrite=%d numLeft=%d numWrite=%d Size=%d bytes."), locWrite, numLeft, numWrite, upload_len);
				goto ret_upload;
			}

#endif //#if defined(CONFIG_APP_FWD)
			
			locWrite += numWrite;
			numLeft -= numWrite;
			sync();
			close(fh);

			head_offset += len + sizeof(IMG_HEADER_T) ;
			startAddr = -1 ; //by sc_yang to reset the startAddr for next image
			update_fw = 1;
		}
	} //while //sc_yang   

	//fprintf(stderr,"####isUpgrade_OK###\n");
#ifndef NO_ACTION
	isUpgrade_OK=1;
	
#if defined(CONFIG_APP_FWD)
	{			
			char tmpStr[20]={0};
			
			sprintf(tmpStr,"%d",shm_id);
			
			write_line_to_file("/var/fwd.ready", 1, tmpStr);
			
			sync();
			exit(0);
	}
#else	//#if defined(CONFIG_APP_FWD)
	system("reboot");
	for(;;);
#endif //#if defined(CONFIG_APP_FWD)

	
#else
#ifdef VOIP_SUPPORT
	// rock: for x86 simulation
	if (update_cfg && !update_fw) {
		if (apmib_reinit()) {
			//reset_user_profile();  // re-initialize user password
		}
	}
#endif
#endif

	return 1;
ret_upload:
	fprintf(stderr, "%s\n", buffer);
	
#if defined(CONFIG_APP_FWD)		
	clear_fwupload_shm(shm_id);
#endif

	return 0;
}
//////////////////////////////////////////////////////////////////////////////
#if defined(CONFIG_RTL_FLASH_DUAL_IMAGE_ENABLE)
void formDualFirmware(request *wp, char *path, char *query)
{
	char *strRequest, *submitUrl, *strVal;
	unsigned char enableDualFW=0, whichBand=0;
	unsigned char tmpBuf[200];
	
	//displayPostDate(wp->post_data);
	
	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page
	
	strVal = req_get_cstream_var(wp, ("active"), "");
	if(strVal[0])
	{
		if(strcmp(strVal,"save") == 0)
		{
//fprintf(stderr,"\r\n apply setting,__[%s-%u]",__FILE__,__LINE__);								
			strVal = req_get_cstream_var(wp, ("dualFw"), "");
			if (strVal[0])
			{
				enableDualFW = 1;
			}
			set_dualbank(enableDualFW);

			
		}
		else if(strcmp(strVal,"reboot") == 0)
		{

			if( boot_from_backup() == 0)
			{
			 	strcpy(tmpBuf, ("Rebooting !!~~~~Please wait for 40~50secs! "));
				 goto setReboot;
			}	
			else {
				strcpy(tmpBuf, ("Reboot Fail!!The image in Backup Bank maybe corrupted!! "));
       	              goto setErr;			
			}
			
		}
	}
	
	apmib_update_web(CURRENT_SETTING);

#ifndef NO_ACTION
	run_init_script("all");
#endif

	OK_MSG(submitUrl);		

	return;

setErr:
	ERR_MSG(tmpBuf);
	return ;

setReboot:
	ERR_MSG(tmpBuf);
	system("reboot");	
}
#endif

#if defined(CONFIG_USBDISK_UPDATE_IMAGE)
void formUploadFromUsb(request *wp, char * path, char * query)
{
	int oneReadMax = 4096;
	int oneRead = 0;
	int fileLen=0;
	char *buff = NULL;
	char tmpBuf[200];
	char *submitUrl;
	char lan_ip[30];	
	char lan_ip_buf[30];
    	FILE *       fd;
		
	 if(!isFileExist(USB_UPLOAD_FORM_PATH))
	 {
      		strcpy(tmpBuf, ("Error!form ware is not exist in usb storage!\n"));
	 	goto ret_err;
	 }
	fd = open(USB_UPLOAD_FORM_PATH, O_RDONLY);
	if (!fd){
      		strcpy(tmpBuf, ("Open image file  failed!\n"));
	 	goto ret_err;
	}
	lseek(fd, 0L, SEEK_SET);
	printf("		<read image from usb storage device>\n");
	/* read image from file to buff */
	 do{
		 buff = brealloc(B_L, buff, fileLen + oneReadMax);
		 if(buff == NULL)
		 {
      			strcpy(tmpBuf, ("my god breallco failed !\n"));
	 		goto ret_err;
		 }
		oneRead = read(fd, (void *)(buff + fileLen), oneReadMax);
		fileLen += oneRead;
		printf(".");
		if(oneRead == -1)
		{
			printf("file read error!\n");
	 		goto ret_err;
		 }
	 }while(oneRead == oneReadMax);
	 printf("\n");

	free(wp->post_data);
	wp->post_data = buff;
	wp->post_data_len = fileLen;
	formUpload(wp, NULL, NULL);/*further check and upload */
	return;
ret_err:
	ERR_MSG(tmpBuf);
	return;
	 
}
#endif

void formUpload(request *wp, char * path, char * query)
{
	//int fh;
	int len;
	int locWrite;
	int numLeft;
	//int numWrite;
	IMG_HEADER_Tp pHeader;
	char tmpBuf[200];
#ifndef REBOOT_CHECK
	char lan_ip_buf[30];
	char lan_ip[30];
#endif
	char *submitUrl;
	int flag=0, startAddr=-1;
	int isIncludeRoot=0;
#ifndef NO_ACTION
	//int pid;
#endif
	int head_offset=0;
	int update_fw=0, update_cfg=0;
	//Support WAPI/openssl, the flash MUST up to 4m
/*
#if defined(CONFIG_RTL_WAPI_SUPPORT) || defined(HTTP_FILE_SERVER_SUPPORTED)
	int fwSizeLimit = 0x400000;
#elif defined( CONFIG_RTK_VOIP )
	int fwSizeLimit = 0x400000;
#else
	int fwSizeLimit = 0x200000;
#endif
*/
	int fwSizeLimit = CONFIG_FLASH_SIZE;
	unsigned char isValidfw = 0;

#if defined(CONFIG_APP_FWD)
#define FWD_CONF "/var/fwd.conf"
	int newfile = 1;
	extern int get_shm_id();
	extern int clear_fwupload_shm();
	int shm_id = get_shm_id();	
#endif

#ifndef REBOOT_CHECK
	apmib_get( MIB_IP_ADDR,  (void *)lan_ip_buf) ;
	sprintf(lan_ip,"%s",inet_ntoa(*((struct in_addr *)lan_ip_buf)) );
#endif
	
	submitUrl = req_get_cstream_var(wp, "submit-url", "");
	//fprintf(stderr,"####%s:%d submitUrl=%s###\n",  __FILE__, __LINE__ , submitUrl);
	//support multiple image
	head_offset = find_head_offset((char *)wp->upload_data);
	//fprintf(stderr,"####%s:%d %d wp->upload_data=%p###\n",  __FILE__, __LINE__ , head_offset, wp->upload_data);
	//fprintf(stderr,"####%s:%d content_length=%s###contenttype=%s###\n",  __FILE__, __LINE__ ,wp->content_length , wp->content_type);
	if (head_offset == -1) {
		strcpy(tmpBuf, "<b>Invalid file format!");
		goto ret_upload;
	}
	while ((head_offset+sizeof(IMG_HEADER_T)) <  wp->upload_len) {
		locWrite = 0;
		pHeader = (IMG_HEADER_Tp) &wp->upload_data[head_offset];
		len = pHeader->len;
#ifdef _LITTLE_ENDIAN_
		len  = DWORD_SWAP(len);
#endif    
		numLeft = len + sizeof(IMG_HEADER_T);
		// check header and checksum
		if (!memcmp(&wp->upload_data[head_offset], FW_HEADER, SIGNATURE_LEN) ||
		    !memcmp(&wp->upload_data[head_offset], FW_HEADER_WITH_ROOT, SIGNATURE_LEN)) {
		    	isValidfw = 1;
			flag = 1;
			//Reboot_Wait = Reboot_Wait+ 50;
		} else if (!memcmp(&wp->upload_data[head_offset], WEB_HEADER, SIGNATURE_LEN)) {
			isValidfw = 1;
			flag = 2;
			//Reboot_Wait = Reboot_Wait+ 40;
		} else if (!memcmp(&wp->upload_data[head_offset], ROOT_HEADER, SIGNATURE_LEN)) {
			isValidfw = 1;
			flag = 3;
			//Reboot_Wait = Reboot_Wait+ 60;
			isIncludeRoot = 1;	
		}else if ( 
#ifdef COMPRESS_MIB_SETTING
				!memcmp(&wp->upload_data[head_offset], COMP_HS_SIGNATURE, COMP_SIGNATURE_LEN) ||
				!memcmp(&wp->upload_data[head_offset], COMP_DS_SIGNATURE, COMP_SIGNATURE_LEN) ||
				!memcmp(&wp->upload_data[head_offset], COMP_CS_SIGNATURE, COMP_SIGNATURE_LEN)
#else
				!memcmp(&wp->upload_data[head_offset], CURRENT_SETTING_HEADER_TAG, TAG_LEN) ||
				!memcmp(&wp->upload_data[head_offset], CURRENT_SETTING_HEADER_FORCE_TAG, TAG_LEN) ||
				!memcmp(&wp->upload_data[head_offset], CURRENT_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) ||
				!memcmp(&wp->upload_data[head_offset], DEFAULT_SETTING_HEADER_TAG, TAG_LEN) ||
				!memcmp(&wp->upload_data[head_offset], DEFAULT_SETTING_HEADER_FORCE_TAG, TAG_LEN) ||
				!memcmp(&wp->upload_data[head_offset], DEFAULT_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) ||
				!memcmp(&wp->upload_data[head_offset], HW_SETTING_HEADER_TAG, TAG_LEN) ||
				!memcmp(&wp->upload_data[head_offset], HW_SETTING_HEADER_FORCE_TAG, TAG_LEN) ||
				!memcmp(&wp->upload_data[head_offset], HW_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) 
#endif
			) {
#ifdef COMPRESS_MIB_SETTING
				COMPRESS_MIB_HEADER_Tp pHeader_cfg;
				pHeader_cfg = (COMPRESS_MIB_HEADER_Tp)&wp->upload_data[head_offset];
				if(!memcmp(&wp->upload_data[head_offset], COMP_CS_SIGNATURE, COMP_SIGNATURE_LEN)) {
					head_offset +=  pHeader_cfg->compLen+sizeof(COMPRESS_MIB_HEADER_T);
					configlen = head_offset;
				}
				else {
					head_offset +=  pHeader_cfg->compLen+sizeof(COMPRESS_MIB_HEADER_T);
				}
#else
				PARAM_HEADER_Tp pHeader_cfg;
				pHeader_cfg = (PARAM_HEADER_Tp)&wp->upload_data[head_offset];
				head_offset +=  pHeader_cfg->len+sizeof(PARAM_HEADER_T);
#endif
				isValidfw = 1;
				update_cfg = 1;
				continue;
		}
		else {
			if (isValidfw == 1)
				break;
			strcpy(tmpBuf, "<b>Invalid file format!");
			goto ret_upload;
		}

		if (len > fwSizeLimit) { //len check by sc_yang
			sprintf(tmpBuf, "<b>Image len exceed max size 0x%x ! len=0x%x</b><br>",fwSizeLimit, len);
			goto ret_upload;
		}
#ifdef CONFIG_RTL_WAPI_SUPPORT
		if((flag == 3) && (len>WAPI_AREA_BASE)) {
			sprintf(tmpBuf, "<b>Root image len 0x%x exceed 0x%x which will overwrite wapi area at flash ! </b><br>", len, WAPI_AREA_BASE);
			goto ret_upload;
		}
#endif
		if ( (flag == 1) || (flag == 3)) {
			if ( !fwChecksumOk((char *)&wp->upload_data[sizeof(IMG_HEADER_T)+head_offset], len)) {
				sprintf(tmpBuf, "<b>Image checksum mismatched! len=0x%x, checksum=0x%x</b><br>", len,
					*((unsigned short *)&wp->upload_data[len-2]) );
				goto ret_upload;
			}
		}
		else {
			char *ptr = (char *)&wp->upload_data[sizeof(IMG_HEADER_T)+head_offset];
			if ( !CHECKSUM_OK((unsigned char *)ptr, len) ) {
				sprintf(tmpBuf, "<b>Image checksum mismatched! len=0x%x</b><br>", len);
				goto ret_upload;
			}
		}
#ifdef HOME_GATEWAY
#ifdef REBOOT_CHECK
		sprintf(tmpBuf, "Upload successfully (size = %d bytes)!<br><br>Firmware update in progress.", wp->upload_len);
#else
		sprintf(tmpBuf, "Upload successfully (size = %d bytes)!<br><br>Firmware update in progress.<br> Do not turn off or reboot the AP during this time.", wp->upload_len);
#endif
#else
		sprintf(tmpBuf, "Upload successfully (size = %d bytes)!<br><br>Firmware update in progress.<br> Do not turn off or reboot the AP during this time.", wp->upload_len);
#endif
		//sc_yang
		head_offset += len + sizeof(IMG_HEADER_T);
		startAddr = -1 ; //by sc_yang to reset the startAddr for next image
		update_fw = 1;
	} //while //sc_yang    

	isFWUPGRADE = 1;

#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT)
	Stop_Domain_Query_Process();
	WaitCountTime=2;
#endif

#if defined(CONFIG_RTL_819X)
	Reboot_Wait = (wp->upload_len/69633)+57+5;
	if (update_cfg==1 && update_fw==0) {
		strcpy(tmpBuf, "<b>Update successfully!");
		Reboot_Wait = (wp->upload_len/69633)+45+5;
		isCFG_ONLY= 1;
	}
#else
	Reboot_Wait = (wp->upload_len/43840)+35;
	if (update_cfg==1 && update_fw==0) {
		strcpy(tmpBuf, "<b>Update successfully!");
		Reboot_Wait = (wp->upload_len/43840)+30;
		isCFG_ONLY= 1;
	}
#endif

#ifdef REBOOT_CHECK
	sprintf(lastUrl,"%s","/status.htm");
	sprintf(okMsg,"%s",tmpBuf);
	countDownTime = Reboot_Wait;
	send_redirect_perm(wp, COUNTDOWN_PAGE);
#else
	OK_MSG_FW(tmpBuf, submitUrl,Reboot_Wait,lan_ip);
#endif
	return;

ret_upload:
	
#if defined(CONFIG_APP_FWD)		
	clear_fwupload_shm(shm_id);
#endif
	Reboot_Wait=0;
	ERR_MSG(tmpBuf);
}

/////////////////////////////////////////////////////////////////////////////
void formPasswordSetup(request *wp, char *path, char *query)
{
	char *submitUrl, *strUser, *strPassword;
	char tmpBuf[100];

	strUser = req_get_cstream_var(wp, "username", "");
	strPassword = req_get_cstream_var(wp, "newpass", "");
	if ( strUser[0] && !strPassword[0] ) {
		strcpy(tmpBuf, ("ERROR: Password cannot be empty."));
		goto setErr_pass;
	}

	if ( strUser[0] ) {
		/* Check if user name is the same as supervisor name */
		if ( !apmib_get(MIB_SUPER_NAME, (void *)tmpBuf)) {
			strcpy(tmpBuf, ("ERROR: Get supervisor name MIB error!"));
			goto setErr_pass;
		}
		if ( !strcmp(strUser, tmpBuf)) {
			strcpy(tmpBuf, ("ERROR: Cannot use the same user name as supervisor."));
			goto setErr_pass;
		}
	}
	else {
		/* Set NULL account */
	}

	/* Set user account to MIB */
	if ( !apmib_set(MIB_USER_NAME, (void *)strUser) ) {
		strcpy(tmpBuf, ("ERROR: Set user name to MIB database failed."));
		goto setErr_pass;
	}

	if ( !apmib_set(MIB_USER_PASSWORD, (void *)strPassword) ) {
		strcpy(tmpBuf, ("ERROR: Set user password to MIB database failed."));
		goto setErr_pass;
	}

	/* Retrieve next page URL */
	apmib_update_web(CURRENT_SETTING);
		
	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page

#ifdef LOGIN_URL
	if (strUser[0])
		submitUrl = "/login.htm";
#endif

#ifdef REBOOT_CHECK
	{
		char tmpMsg[300];
		char lan_ip_buf[30], lan_ip[30];
		
		sprintf(tmpMsg, "%s","Change setting successfully!<br><br>Do not turn off or reboot the Router during this time.");
		apmib_get( MIB_IP_ADDR,  (void *)lan_ip_buf) ;
		sprintf(lan_ip,"%s",inet_ntoa(*((struct in_addr *)lan_ip_buf)) );
		OK_MSG_FW(tmpMsg, submitUrl,APPLY_COUNTDOWN_TIME,lan_ip);
#ifdef REBOOT_CHECK
		run_init_script_flag = 1;
#endif		
#ifndef NO_ACTION
		run_init_script("all");
#endif	
	}
#else
	OK_MSG(submitUrl);
#endif
	return;

setErr_pass:
	ERR_MSG(tmpBuf);
}

////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void formStats(request *wp, char *path, char *query)
{
	char *submitUrl;

	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page

	if (submitUrl[0])
		send_redirect_perm(wp, submitUrl);
}

#ifdef CONFIG_RTK_MESH
void formMeshStatus(request *wp, char *path, char *query)
{
	char *submitUrl;

	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page

	if (submitUrl[0])
		send_redirect_perm(wp, submitUrl);
}
#endif // CONFIG_RTK_MESH
 
/////////////////////////////////////////////////////////////////////////////
int  ntpHandler(request *wp, char *tmpBuf, int fromWizard)
{
	int enabled=0, ntpServerIdx ;
	struct in_addr ipAddr ;
	char *tmpStr ;
//Brad add for daylight save	
	int dlenabled=0;
//Brad add end	
	if (fromWizard) {
		tmpStr = req_get_cstream_var(wp, ("enabled"), "");  
		if(!strcmp(tmpStr, "ON"))
			enabled = 1 ;
		else 
			enabled = 0 ;

		if ( apmib_set( MIB_NTP_ENABLED, (void *)&enabled) == 0) {
			strcpy(tmpBuf, ("Set enabled flag error!"));
			goto setErr_ntp;
		}
//Brad add for daylight save		
		tmpStr = req_get_cstream_var(wp, ("dlenabled"), "");  
		if(!strcmp(tmpStr, "ON"))
			dlenabled = 1 ;
		else 
			dlenabled = 0 ;

		if ( apmib_set( MIB_DAYLIGHT_SAVE, (void *)&dlenabled) == 0) {
			strcpy(tmpBuf, ("Set enabled flag error!"));
			goto setErr_ntp;
		}
//Brad add end		
	}	
	else
		enabled = 1;
	if(enabled){
		tmpStr = req_get_cstream_var(wp, ("ntpServerId"), "");  
		if(tmpStr[0]){
			ntpServerIdx = tmpStr[0] - '0' ;
			if ( apmib_set(MIB_NTP_SERVER_ID, (void *)&ntpServerIdx) == 0) {
				strcpy(tmpBuf, ("Set Time Zone error!"));
				goto setErr_ntp;
			}
		}
		tmpStr = req_get_cstream_var(wp, ("timeZone"), "");  
		if(tmpStr[0]){
			if ( apmib_set(MIB_NTP_TIMEZONE, (void *)tmpStr) == 0) {
					strcpy(tmpBuf, ("Set Time Zone error!"));
				goto setErr_ntp;
		}
		}

		tmpStr = req_get_cstream_var(wp, ("ntpServerIp1"), "");  
		if(tmpStr[0]){
			inet_aton(tmpStr, &ipAddr);
			if ( apmib_set(MIB_NTP_SERVER_IP1, (void *)&ipAddr) == 0) {
				strcpy(tmpBuf, ("Set NTP server error!"));
				goto setErr_ntp;
			} 
			}
		tmpStr = req_get_cstream_var(wp, ("ntpServerIp2"), "");  
		if(tmpStr[0]){
			inet_aton(tmpStr, &ipAddr);
			if ( apmib_set(MIB_NTP_SERVER_IP2,(void *) &ipAddr ) == 0) {
				strcpy(tmpBuf, ("Set NTP server IP error!"));
				goto setErr_ntp;
			}
		}
	}
	return 0 ;	
setErr_ntp:
	return -1 ;
	
}
void formNtp(request *wp, char *path, char *query)
{
	char *submitUrl,*strVal, *tmpStr;
	char tmpBuf[100];
	int enabled=0;
//Brad add for daylight save	
	int dlenabled=0;
//Brad add end	
#ifndef NO_ACTION
//	int pid;
#endif
	int time_value=0;
	int cur_year=0;
	
	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page
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
		
		tmpStr = req_get_cstream_var(wp, ("timeZone"), "");  
		if(tmpStr[0]){
			if ( apmib_set(MIB_NTP_TIMEZONE, (void *)tmpStr) == 0) {
					strcpy(tmpBuf, ("Set Time Zone error!"));
				goto setErr_end;
			}
		}

		tmpStr = req_get_cstream_var(wp, ("enabled"), "");  
		if(!strcmp(tmpStr, "ON"))
			enabled = 1 ;
		else 
			enabled = 0 ;
		if ( apmib_set( MIB_NTP_ENABLED, (void *)&enabled) == 0) {
			strcpy(tmpBuf, ("Set enabled flag error!"));
			goto setErr_end;
		}
//Brad add for daylight save		
		tmpStr = req_get_cstream_var(wp, ("dlenabled"), "");  
		if(!strcmp(tmpStr, "ON"))
			dlenabled = 1 ;
		else 
			dlenabled = 0 ;
		if ( apmib_set( MIB_DAYLIGHT_SAVE, (void *)&dlenabled) == 0) {
			strcpy(tmpBuf, ("Set dl enabled flag error!"));
			goto setErr_end;
		}
//Brad add end		
	}
	if (enabled == 0)		
		goto  set_ntp_end;
	
	if(ntpHandler(wp, tmpBuf, 0) < 0)
		goto setErr_end ;

set_ntp_end:
	apmib_update_web(CURRENT_SETTING);
//Brad modify for system re-init method
#if 0
	pid = find_pid_by_name("ntp.sh");
	if(pid)
		kill(pid, SIGTERM);

	pid = fork();
        if (pid)
		waitpid(pid, NULL, 0);
        else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _NTP_SCRIPT_PROG);
		execl( tmpBuf, _NTP_SCRIPT_PROG, NULL);
               	exit(1);
       	}
#endif
#ifndef NO_ACTION
	run_init_script("all");
#endif
	OK_MSG(submitUrl);
	return;

setErr_end:
	ERR_MSG(tmpBuf);
}


void formPocketWizard(request *wp, char *path, char *query)
{
	char *tmpStr, *strVal;
	char tmpBuf[100];
	char varName[20];
	int i=0;
	int mode=-1;
	int val;
	int wlBandMode;
	int band2G5GSelect;
	int dns_changed=0;
#if defined(CONFIG_RTL_ULINKER)
	int ulinker_auto_changed;	
#endif

//displayPostDate(wp->post_data);

/*
	strVal = req_get_cstream_var(wp, "band0", "");
	val = strtol( strVal, (char **)NULL, 10);
	val = (val + 1);
	apmib_set( MIB_WLAN_BAND, (void *)&val);
*/
		
#if defined(CONFIG_RTL_ULINKER)
	tmpStr = req_get_cstream_var(wp, "otg_auto_val", "");
	if(tmpStr[0] != 0)
	{
		apmib_get(MIB_ULINKER_AUTO, (void *)&ulinker_auto_changed);
		val = atoi(tmpStr);
		apmib_set(MIB_ULINKER_AUTO, (void *)&val);

		if (ulinker_auto_changed != val)
			ulinker_auto_changed = 1;
		else
			ulinker_auto_changed = 0;	
	}
#endif
	
#ifdef HOME_GATEWAY
	if(tcpipWanHandler(wp, tmpBuf, &dns_changed) < 0){
		goto setErr_end;	
	}
#endif
		
#if defined(CONFIG_RTL_92D_SUPPORT)		
	tmpStr = req_get_cstream_var(wp, "wlBandMode", "");
	if(tmpStr[0] != 0)
	{
		wlBandMode = atoi(tmpStr);
		apmib_set(MIB_WLAN_BAND2G5G_SELECT, (void *)&wlBandMode);
	}
	
	apmib_get(MIB_WLAN_BAND2G5G_SELECT, (void *)&wlBandMode);
	if(wlBandMode == BANDMODEBOTH)
	{					
		unsigned char wlanIfStr[10];				
		
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
	
		/* 92d rule, 5g must up in wlan0 */
		/* phybandcheck */
		if(whichWlanIfIs(PHYBAND_5G) != 0)
		{
			swapWlanMibSetting(0,1);			
		}
		
		
	}
	else if(wlBandMode == BANDMODESINGLE)
	{
		unsigned int wlanif;
		
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
				intVal = 1;
				apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&intVal);
			}						
		}
		
		tmpStr = req_get_cstream_var(wp, "Band2G5GSupport", ""); //wlan0 PHYBAND_TYPE
		if(tmpStr[0] != 0)
		{
			band2G5GSelect = atoi(tmpStr);			
		}
		
		wlanif = whichWlanIfIs(band2G5GSelect);
		
		/* 92d rule, 5g must up in wlan0 */
		/* phybandcheck */
		if(wlanif != 0)
		{
			swapWlanMibSetting(0,1);			
		}
		
		wlan_idx = 0 ;		
		
		val = 0;
		apmib_set( MIB_WLAN_WLAN_DISABLED, (void *)&val); // enable wlan0 and disable wlan1
		
		
	}
#endif //#if defined(CONFIG_RTL_92D_SUPPORT)
				
	wlan_idx = 0 ;
	tmpStr = req_get_cstream_var(wp, "pocket_ssid", "");
	if(tmpStr[0] != 0)
		apmib_set(MIB_WLAN_SSID, (void *)tmpStr);
		
	for(i = 0 ; i<NUM_WLAN_INTERFACE ; i++)
	{
		wlan_idx = i;
		vwlan_idx = 0;
		
		if(i == 1)
		{
			if(wlBandMode != BANDMODEBOTH) // single band, no need process wlan1
				continue;
				
			tmpStr = req_get_cstream_var(wp, "pocket_ssid1", "");
			if(tmpStr[0] != 0)
				apmib_set(MIB_WLAN_SSID, (void *)tmpStr);			
		}
		sprintf(varName, "mode%d", i);
		tmpStr = req_get_cstream_var(wp, varName, "");
		if(tmpStr[0])
		{
			val = atoi(tmpStr);
			apmib_set( MIB_WLAN_MODE, (void *)&val);
		}
		
		
		
		sprintf(varName, "method%d", i);
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
				if(wepHandler(wp, tmpBuf, i) < 0)
				{
					goto setErr_end;
				}
			}
			else if(val > ENCRYPT_WEP && val <= WSC_AUTH_WPA2PSKMIXED)
			{
				if(wpaHandler(wp, tmpBuf, i) < 0)
				{
					goto setErr_end;
				}
			}
		}

#if defined(WLAN_PROFILE)
		if(addWlProfileHandler(wp, tmpBuf, i) < 0){
			//submitUrl = req_get_cstream_var(wp, ("submit-url-wlan2"), "");   // hidden page
			//goto setErr_end;
		}
		
#endif //#if defined(WLAN_PROFILE)


	}
	
	
#if defined(CONFIG_RTL_ULINKER) //repeater mode: clone wlan setting to wlan-vxd and modify wlan ssid
		int wlan_mode;
		int rptEnabled;
		int wlanvxd_mode;
		

		if(wlan_idx == 0)
			apmib_get(MIB_REPEATER_ENABLED1, (void *)&rptEnabled);
		else
			apmib_get(MIB_REPEATER_ENABLED2, (void *)&rptEnabled);



		apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
		
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

#if defined(CONFIG_RTL_ULINKER)
	if (ulinker_auto_changed == 1) {
		char *submitUrl;
		needReboot = 1;
		submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page
		sprintf(lastUrl,"%s",submitUrl);
		send_redirect_perm(wp, "/reload.htm");
		return ;
	}
#endif


#ifdef REBOOT_CHECK
	run_init_script_flag = 1;
#endif
#ifndef NO_ACTION
	run_init_script("all");
#endif
	tmpStr = req_get_cstream_var(wp, ("method0"), "");
	REBOOT_WAIT("/wizard.htm");
	
	return ;
setErr_end:
	
	OK_MSG1(tmpBuf,"/wizard.htm");
	return ;
}


#if defined(MIB_TLV)
extern int mib_search_by_id(const mib_table_entry_T *mib_tbl, unsigned short mib_id, unsigned char *pmib_num, const mib_table_entry_T **ppmib, unsigned int *offset);
extern mib_table_entry_T mib_root_table[];
#else
extern int update_linkchain(int fmt, void *Entry_old, void *Entry_new, int type_size);
#endif
void formWizard(request *wp, char *path, char *query)
{
	char *tmpStr;
	char tmpBuf[100];
	char varName[20];
	int i;
	int showed_wlan_num;
	int wlBandMode;
#ifdef HOME_GATEWAY	
	int dns_changed=0;
#endif	
	int mode=-1;
	char *submitUrl;
	char buffer[200];
	struct in_addr inLanaddr_orig, inLanaddr_new;
	struct in_addr inLanmask_orig, inLanmask_new;
	int	entryNum_resvdip;
	DHCPRSVDIP_T entry_resvdip, checkentry_resvdip;
	int link_type;
	struct in_addr private_host, tmp_private_host, update;	
	struct in_addr dhcpRangeStart, dhcpRangeEnd;
#ifdef MIB_TLV
	char pmib_num[10]={0};
	mib_table_entry_T *pmib_tl = NULL;
	unsigned int offset;
#endif

//displayPostDate(wp->post_data);
		

	apmib_get( MIB_IP_ADDR,  (void *)buffer); //save the orig lan subnet
	memcpy((void *)&inLanaddr_orig, buffer, 4);
	
	apmib_get( MIB_SUBNET_MASK,  (void *)buffer); //save the orig lan mask
	memcpy((void *)&inLanmask_orig, buffer, 4);
#ifdef HOME_GATEWAY
	if(opModeHandler(wp, tmpBuf) < 0)
		goto setErr_end;	
#endif

	if(ntpHandler(wp, tmpBuf, 1) < 0)
		goto setErr_end;
		
	if(tcpipLanHandler(wp, tmpBuf) < 0){
		submitUrl = req_get_cstream_var(wp, ("submit-url-lan"), "");   // hidden page
		goto setErr_end;
	}

#ifdef HOME_GATEWAY
	if(tcpipWanHandler(wp, tmpBuf, &dns_changed) < 0){
		submitUrl = req_get_cstream_var(wp, ("submit-url-wan"), "");   // hidden page
		goto setErr_end;	
	}
#endif

#if defined(CONFIG_RTL_92D_SUPPORT)

	tmpStr = req_get_cstream_var(wp, "wlBandMode", "");
	if(tmpStr[0] != 0)
	{
		wlBandMode = atoi(tmpStr);
		apmib_set(MIB_WLAN_BAND2G5G_SELECT, (void *)&wlBandMode);
	}

	for(i=0 ; i<NUM_WLAN_INTERFACE ; i++)
	{
		unsigned char wlanif[10];
		memset(wlanif,0x00,sizeof(wlanif));
		sprintf(wlanif, "wlan%d",i);
		if(SetWlan_idx(wlanif))
		{
			int intVal;
			
			intVal = 1;
			apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&intVal);
		}						
	}
#endif

	for(i=0 ; i < wlan_num ;i++){	
		wlan_idx = i ;
		sprintf(WLAN_IF, "wlan%d", wlan_idx);
		if(wlanHandler(wp, tmpBuf,&mode, i) < 0){
		submitUrl = req_get_cstream_var(wp, ("submit-url-wlan1"), "");   // hidden page
		goto setErr_end;
	}	
		
		sprintf(varName, "method%d", i);
		tmpStr = req_get_cstream_var(wp, varName, "");
	if(tmpStr[0] && tmpStr[0] == '1'){
			if(wepHandler(wp, tmpBuf, i) < 0){
			submitUrl = req_get_cstream_var(wp, ("submit-url-wlan2"), "");   // hidden page
			goto setErr_end;
		}
	}	
		if(wpaHandler(wp, tmpBuf, i) < 0){
		submitUrl = req_get_cstream_var(wp, ("submit-url-wlan2"), "");   // hidden page
		goto setErr_end;
	}


		
#if defined(WLAN_PROFILE)
		if(addWlProfileHandler(wp, tmpBuf, i) < 0){
			//submitUrl = req_get_cstream_var(wp, ("submit-url-wlan2"), "");   // hidden page
			//goto setErr_end;
		}
		
#endif //#if defined(WLAN_PROFILE)


	}
		
#if defined(CONFIG_RTL_92D_SUPPORT)		
	apmib_get(MIB_WLAN_BAND2G5G_SELECT, (void *)&wlBandMode);
	if(BANDMODEBOTH == wlBandMode)
	{
		unsigned char wlanIfStr[10];				
				
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
			}						
		}
		
		/* 92d rule, 5g must up in wlan0 */
		/* phybandcheck */
		if(whichWlanIfIs(PHYBAND_5G) != 0)
		{
			swapWlanMibSetting(0,1);			
		}
	}
	else
	{
		int band2G5GSelect;
		int intVal;		
		
		for(i=0 ; i<NUM_WLAN_INTERFACE ; i++)
		{
			unsigned char wlanif[10];
			memset(wlanif,0x00,sizeof(wlanif));
			sprintf(wlanif, "wlan%d",i);
			if(SetWlan_idx(wlanif))
			{				
				intVal = SMACSPHY;
				apmib_set(MIB_WLAN_MAC_PHY_MODE, (void *)&intVal);				
			}						
		}
		
		tmpStr = req_get_cstream_var(wp, "Band2G5GSupport", "");
		
		if(tmpStr[0] != 0)
		{
			band2G5GSelect = atoi(tmpStr);			
		}
		//1:2g 2:5g
		/* 92d rule, 5g must up in wlan0 if only one wlanif enable*/
		/* phybandcheck */
		if(whichWlanIfIs(band2G5GSelect) != 0)
		{
			swapWlanMibSetting(0,1);			
		}
		apmib_save_wlanIdx();
		wlan_idx = 1;
		intVal = 1;
		apmib_set( MIB_WLAN_WLAN_DISABLED, (void *)&intVal); // disable wlan1
		apmib_recov_wlanIdx();
	}
#endif //#if defined(CONFIG_RTL_92D_SUPPORT)
	
	apmib_update_web(CURRENT_SETTING);
	apmib_get( MIB_IP_ADDR,  (void *)buffer); //check the new lan subnet
	memcpy((void *)&inLanaddr_new, buffer, 4);
		
	apmib_get( MIB_SUBNET_MASK,  (void *)buffer); //check the new lan mask
	memcpy((void *)&inLanmask_new, buffer, 4);
	
	if((inLanaddr_orig.s_addr & inLanmask_orig.s_addr) != (inLanaddr_new.s_addr & inLanmask_new.s_addr)){
		
		//check static dhcp ip 
		apmib_get(MIB_DHCPRSVDIP_TBL_NUM, (void *)&entryNum_resvdip);
		link_type = 8; //DHCPRSVDIP_ARRY_T
		for (i=1; i<=entryNum_resvdip; i++) {
			memset(&checkentry_resvdip, '\0', sizeof(checkentry_resvdip));
			*((char *)&entry_resvdip) = (char)i;
			apmib_get(MIB_DHCPRSVDIP_TBL, (void *)&entry_resvdip);
			memcpy(&checkentry_resvdip, &entry_resvdip, sizeof(checkentry_resvdip));
			memcpy((void *)&private_host, &(entry_resvdip.ipAddr), 4);
			if((inLanaddr_new.s_addr & inLanmask_new.s_addr) != (private_host.s_addr & inLanmask_new.s_addr)){
				update.s_addr = inLanaddr_new.s_addr & inLanmask_new.s_addr;
				tmp_private_host.s_addr  = ~(inLanmask_new.s_addr) & private_host.s_addr;
				update.s_addr = update.s_addr | tmp_private_host.s_addr;
				memcpy((void *)&(checkentry_resvdip.ipAddr), &(update), 4);
#if defined(MIB_TLV)
				offset=0;//must initial first for mib_search_by_id
				mib_search_by_id(mib_root_table, MIB_DHCPRSVDIP_TBL, (unsigned char *)pmib_num, &pmib_tl, &offset);
				update_tblentry(pMib,offset,entryNum_resvdip,pmib_tl,&entry_resvdip, &checkentry_resvdip);
#else
				update_linkchain(link_type, &entry_resvdip, &checkentry_resvdip , sizeof(checkentry_resvdip));
#endif

			}
		}
		apmib_get( MIB_DHCP_CLIENT_START,  (void *)buffer); //save the orig dhcp start 
		memcpy((void *)&dhcpRangeStart, buffer, 4);
		apmib_get( MIB_DHCP_CLIENT_END,  (void *)buffer); //save the orig dhcp end 
		memcpy((void *)&dhcpRangeEnd, buffer, 4);
		
		if((dhcpRangeStart.s_addr & inLanmask_new.s_addr) != (inLanaddr_new.s_addr & inLanmask_new.s_addr)){
			update.s_addr = inLanaddr_new.s_addr & inLanmask_new.s_addr;
			tmp_private_host.s_addr  = ~(inLanmask_new.s_addr) & dhcpRangeStart.s_addr;
			update.s_addr = update.s_addr | tmp_private_host.s_addr;
			memcpy((void *)&(dhcpRangeStart), &(update), 4);
			apmib_set(MIB_DHCP_CLIENT_START, (void *)&dhcpRangeStart);
		}
		if((dhcpRangeEnd.s_addr & inLanmask_new.s_addr) != (inLanaddr_new.s_addr & inLanmask_new.s_addr)){
			update.s_addr = inLanaddr_new.s_addr & inLanmask_new.s_addr;
			tmp_private_host.s_addr  = ~(inLanmask_new.s_addr) & dhcpRangeEnd.s_addr;
			update.s_addr = update.s_addr | tmp_private_host.s_addr;
			memcpy((void *)&(dhcpRangeEnd), &(update), 4);
			apmib_set(MIB_DHCP_CLIENT_END, (void *)&dhcpRangeEnd);
		}
		apmib_update_web(CURRENT_SETTING);
	}
#ifdef REBOOT_CHECK
	run_init_script_flag = 1;
#endif
#ifndef NO_ACTION
	run_init_script("all");
#endif
	submitUrl = req_get_cstream_var(wp, ("next_url"), "");
	REBOOT_WAIT("/wizard.htm");

	return ;
setErr_end:
	
	OK_MSG1(tmpBuf,"/wizard.htm");
	return ;

}

///////////////////////////////////////////////////////////////////////////////////////////////
int logout=0 ;
void formLogout(request *wp, char *path, char *query)
{
	char *logout_str, *return_url;
	logout_str = req_get_cstream_var(wp, ("logout"), "");
	if (logout_str[0]) {
		logout = 1 ;
#ifdef LOGIN_URL
		delete_user(wp);
	    OK_MSG("/login.htm");
	    return;
#endif		
	}

	return_url = req_get_cstream_var(wp, ("return-url"), "");

#ifdef REBOOT_CHECK
	send_redirect_perm(wp, return_url);	
#else
        OK_MSG(return_url);
#endif

	return;
}
#define _PATH_SYSCMD_LOG "/tmp/syscmd.log"

void formSysCmd(request *wp, char *path, char *query)
{
	char  *submitUrl, *sysCmd;
#ifndef NO_ACTION
	char tmpBuf[100];
#endif
	
	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page
	sysCmd = req_get_cstream_var(wp, "sysCmd", "");   // hidden page

#ifndef NO_ACTION
	if(sysCmd[0]){
		snprintf(tmpBuf, 100, "%s 2>&1 > %s",sysCmd,  _PATH_SYSCMD_LOG);
		system(tmpBuf);
	}
#endif
		send_redirect_perm(wp, submitUrl);
	return;
}

int sysCmdLog(request *wp, int argc, char **argv)
{
        FILE *fp;
	char  buf[150];
	int nBytesSent=0;

        fp = fopen(_PATH_SYSCMD_LOG, "r");
        if ( fp == NULL )
                goto err1;
        while(fgets(buf,150,fp)){
		nBytesSent += req_format_write(wp, ("%s"), buf);
        }
	fclose(fp);
	unlink(_PATH_SYSCMD_LOG);
err1:
	return nBytesSent;
}

#if defined(CONFIG_RTL_ULINKER)

void formUlkOpMode(request *wp, char *path, char *query)
{
	char *submitUrl;
	char *tmpStr;
	int ulinker_auto, opmode, wlanMode, rpt_enabled;
	char tmpBuf[100];

//displayPostDate(wp->post_data);
	
	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page


	tmpStr = req_get_cstream_var(wp, ("ulinker_auto"), "");
	if(tmpStr[0])
	{
		ulinker_auto = tmpStr[0] - '0' ;
		apmib_set( MIB_ULINKER_AUTO, (void *)&ulinker_auto);
		
		if(ulinker_auto == 0)
		{
			int selVal;
			tmpStr = req_get_cstream_var(wp, ("ulinker_manual_Sel"), "");
			if(tmpStr[0])
			{
				selVal = tmpStr[0] - '0';
				
				switch(selVal)
				{
					case 0:
ulinker_wlan_mib_copy(&pMib->wlan[0][0], &pMib->wlan[0][ULINKER_AP_MIB]);
						opmode = BRIDGE_MODE;
						wlanMode = AP_MODE;
						rpt_enabled = 0; 
						break;
					case 1:
ulinker_wlan_mib_copy(&pMib->wlan[0][0], &pMib->wlan[0][ULINKER_CL_MIB]);						
						opmode = BRIDGE_MODE;
						wlanMode = CLIENT_MODE;
						rpt_enabled = 0; 
						break;
					case 2:
ulinker_wlan_mib_copy(&pMib->wlan[0][0], &pMib->wlan[0][ULINKER_AP_MIB]);
						opmode = GATEWAY_MODE;
						wlanMode = AP_MODE;
						rpt_enabled = 0; 
						break;
					case 3:
ulinker_wlan_mib_copy(&pMib->wlan[0][0], &pMib->wlan[0][ULINKER_RPT_MIB]);						
						opmode = BRIDGE_MODE;
						wlanMode = AP_MODE;
						rpt_enabled = 1; 
						break;
					case 4:
ulinker_wlan_mib_copy(&pMib->wlan[0][0], &pMib->wlan[0][ULINKER_RPT_MIB]);
						opmode = WISP_MODE;
						wlanMode = AP_MODE;
						rpt_enabled = 1;
						break;
				}
				apmib_set( MIB_OP_MODE, (void *)&opmode);
				apmib_set( MIB_WLAN_MODE, (void *)&wlanMode);
				pMib->wlan[wlan_idx][NUM_VWLAN_INTERFACE].wlanMode = CLIENT_MODE;
				
				if(wlanMode == CLIENT_MODE) //set cipher suit to AES and encryption to wpa2 only if wpa2 mixed mode is set
				{
					
					ENCRYPT_T encrypt;
					int intVal;
					apmib_get( MIB_WLAN_ENCRYPT, (void *)&encrypt);
					if(encrypt == ENCRYPT_WPA2_MIXED)
					{
						intVal =   WPA_CIPHER_AES ;
						encrypt = ENCRYPT_WPA2;
						
						apmib_set(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&intVal);
						apmib_set(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&intVal);
						apmib_set(MIB_WLAN_ENCRYPT, (void *)&encrypt);
					}
				}
	
				if(wlan_idx == 0)
				{
					apmib_set( MIB_REPEATER_ENABLED1, (void *)&rpt_enabled);						
				}
				else
				{
					apmib_set( MIB_REPEATER_ENABLED2, (void *)&rpt_enabled);
				}
				pMib->wlan[wlan_idx][NUM_VWLAN_INTERFACE].wlanDisabled = (rpt_enabled?0:1);
				
			}
		}
	}
	
	apmib_update_web(CURRENT_SETTING);

#if defined(CONFIG_RTL_ULINKER)
	if (ulinker_auto == 0) {
		char *submitUrl;
		needReboot = 1;
		submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page
		sprintf(lastUrl,"%s",submitUrl);
		send_redirect_perm(wp, "/reload.htm");
		return ;
	}
#endif

#ifdef REBOOT_CHECK
	run_init_script_flag = 1;
#endif		

#ifdef REBOOT_CHECK
	REBOOT_WAIT(submitUrl);
#else //#ifdef REBOOT_CHECK	.
	OK_MSG(submitUrl);
#endif //#ifdef REBOOT_CHECK	


#ifndef NO_ACTION
	run_init_script("all");
#endif
return;

setErr:
	ERR_MSG(tmpBuf);
}
#endif//#if defined(CONFIG_RTL_ULINKER)

#ifdef HOME_GATEWAY
int  opModeHandler(request *wp, char *tmpBuf)
{
	char *tmpStr;
	int opmode, wanId;
	
	tmpStr = req_get_cstream_var(wp, ("opMode"), "");  
	if(tmpStr[0]){
		opmode = tmpStr[0] - '0' ;
		if ( apmib_set(MIB_OP_MODE, (void *)&opmode) == 0) {
			strcpy(tmpBuf, ("Set Opmode error!"));
			goto setErr_opmode;
		}
	}
#if defined(CONFIG_SMART_REPEATER)
	if(opmode==2)
	{//wisp mode
#endif
		tmpStr = req_get_cstream_var(wp, ("wispWanId"), "");  
		if(tmpStr[0]){
			wanId = tmpStr[0] - '0' ;
			if ( apmib_set(MIB_WISP_WAN_ID, (void *)&wanId) == 0) {
				strcpy(tmpBuf, ("Set WISP WAN Id error!"));
				goto setErr_opmode;
			}
#if defined(CONFIG_SMART_REPEATER)
			int rpt_enabled = 1;
			char wlanifStr[20];
			int wlanMode;

			apmib_save_wlanIdx();
			if(wanId == 0)
			{
				apmib_set( MIB_REPEATER_ENABLED1, (void *)&rpt_enabled);

				rpt_enabled=0;
				apmib_set(MIB_REPEATER_ENABLED2,(void *)&rpt_enabled);
			}
			else
			{
				apmib_set( MIB_REPEATER_ENABLED2, (void *)&rpt_enabled);

				rpt_enabled=0;
				apmib_set(MIB_REPEATER_ENABLED1,(void *)&rpt_enabled);
			}

			sprintf(wlanifStr, "wlan%d", wanId); 
			SetWlan_idx(wlanifStr);
			wlanMode = AP_MODE;
			apmib_set( MIB_WLAN_MODE, (void *)&wlanMode);

			sprintf(wlanifStr, "wlan%d-vxd", wanId); 
			SetWlan_idx(wlanifStr);
			wlanMode = CLIENT_MODE;
			apmib_set( MIB_WLAN_MODE, (void *)&wlanMode);
			rpt_enabled = 0;
			apmib_set( MIB_WLAN_WLAN_DISABLED, (void *)&rpt_enabled);
			apmib_recov_wlanIdx();

#endif
		}
#if defined(CONFIG_SMART_REPEATER)
		else{//only one wlan:92c
			int rpt_enabled = 1;
			char wlanifStr[20]={0};
			int wlanMode;

			wanId=0;
			apmib_save_wlanIdx();
			apmib_set( MIB_REPEATER_ENABLED1, (void *)&rpt_enabled);
			
			sprintf(wlanifStr, "wlan%d", wanId); 
			SetWlan_idx(wlanifStr);
			wlanMode = AP_MODE;
			apmib_set( MIB_WLAN_MODE, (void *)&wlanMode);

			sprintf(wlanifStr, "wlan%d-vxd", wanId); 
			SetWlan_idx(wlanifStr);
			wlanMode = CLIENT_MODE;
			apmib_set( MIB_WLAN_MODE, (void *)&wlanMode);
			rpt_enabled = 0;
			apmib_set( MIB_WLAN_WLAN_DISABLED, (void *)&rpt_enabled);
			apmib_recov_wlanIdx();
		}
	}else //opmode is gw or bridge
	{	

		int rpt_enabled=0;
		apmib_set(MIB_REPEATER_ENABLED1, (void *)&rpt_enabled);
		apmib_set(MIB_REPEATER_ENABLED2,(void *)&rpt_enabled);
	}
#endif
	return 0;

setErr_opmode:
	return -1;

}
void formOpMode(request *wp, char *path, char *query)
{
	char *submitUrl;
	char tmpBuf[100];
	
	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page

	if(opModeHandler(wp, tmpBuf) < 0)
			goto setErr;
	
	apmib_update_web(CURRENT_SETTING);
	
#ifdef REBOOT_CHECK
	run_init_script_flag = 1;
#endif		

#ifdef REBOOT_CHECK
	REBOOT_WAIT(submitUrl);
#else //#ifdef REBOOT_CHECK	.
	OK_MSG(submitUrl);
#endif //#ifdef REBOOT_CHECK	


#ifndef NO_ACTION
	run_init_script("all");
#endif
return;

setErr:
	ERR_MSG(tmpBuf);
}
#endif

#ifdef REBOOT_CHECK
void formRebootCheck(request *wp, char *path, char *query)
{
	char *submitUrl;
	
	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page
	apmib_update_web(CURRENT_SETTING);
#ifdef REBOOT_CHECK
	run_init_script_flag = 1;
#endif
#ifndef NO_ACTION
	run_init_script("all");
#endif
	REBOOT_WAIT(submitUrl);
	needReboot = 0;
}

#if defined(WLAN_PROFILE)
void formSiteSurveyProfile(request *wp, char *path, char *query)
{
	char *submitUrl, *strTmp, *addProfileTmp;
	char tmpBuf[100];
	char varName[20];
		
//displayPostDate(wp->post_data);	


	sprintf(varName, "wizardAddProfile%d", wlan_idx);
	addProfileTmp = req_get_cstream_var(wp, varName, "");
	
	if(addProfileTmp[0])	
	{
		int rptEnabled, wlan_mode;
		int ori_vwlan_idx=vwlan_idx;
		int profile_enabled_id, profile_num_id, profile_tbl_id;
		int profileEnabledVal=1;
		char iwprivCmd[600]={0};
		int entryNum;
		WLAN_PROFILE_T entry;
		int profileIdx;
		char ifname[10]={0}; //max is wlan0-vxd
		
		memset(iwprivCmd, 0x00, sizeof(iwprivCmd));
		apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);

		if(wlan_idx == 0)
			apmib_get(MIB_REPEATER_ENABLED1, (void *)&rptEnabled);
		else
			apmib_get(MIB_REPEATER_ENABLED2, (void *)&rptEnabled);

		
		if( (wlan_mode == AP_MODE || wlan_mode == AP_WDS_MODE) && (rptEnabled == 1))
		{
			sprintf(ifname,"wlan%d-vxd",wlan_idx);
			vwlan_idx = NUM_VWLAN_INTERFACE;
		}
		else
		{
			sprintf(ifname,"wlan%d",wlan_idx);
			vwlan_idx = 0;
		}
		
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

		apmib_set(profile_enabled_id, (void *)&profileEnabledVal);
		


		if(addWlProfileHandler(wp, tmpBuf, wlan_idx) < 0){
	printf("\r\n Add wireless profile fail__[%s-%u]\r\n",__FILE__,__LINE__);
			//strcpy(tmpBuf, ("Add wireless profile fail!"));
			//goto ss_err;
		}

		sprintf(iwprivCmd,"iwpriv %s set_mib ap_profile_enable=%d",ifname, profileEnabledVal);
		system(iwprivCmd);
		
		sprintf(iwprivCmd,"iwpriv %s set_mib ap_profile_num=0",ifname);
		system(iwprivCmd);

		apmib_get(profile_num_id, (void *)&entryNum);

		for(profileIdx=1; profileIdx<=entryNum;profileIdx++)
		{
			memset(iwprivCmd, 0x00, sizeof(iwprivCmd));
			memset(&entry, 0x00, sizeof(WLAN_PROFILE_T));
			*((char *)&entry) = (char)profileIdx;
			apmib_get(profile_tbl_id, (void *)&entry);
		



		
		
		
		
			//iwpriv wlan0 set_mib ap_profile_add="open-ssid",0,0
			if(entry.encryption == ENCRYPT_DISABLED)
			{
				sprintf(iwprivCmd,"iwpriv %s set_mib ap_profile_add=\"%s\",%d,%d",ifname,entry.ssid,0,0);
			}
			else if(entry.encryption == WEP64 || entry.encryption == WEP128)
			{
				char tmp1[400];
				if (entry.encryption == WEP64)			
					sprintf(tmp1,"%d,%d,%02x%02x%02x%02x%02x,%02x%02x%02x%02x%02x,%02x%02x%02x%02x%02x,%02x%02x%02x%02x%02x", 
						entry.auth,
						entry.wep_default_key,
						entry.wepKey1[0],entry.wepKey1[1],entry.wepKey1[2],entry.wepKey1[3],entry.wepKey1[4],
						entry.wepKey2[0],entry.wepKey2[1],entry.wepKey2[2],entry.wepKey2[3],entry.wepKey2[4],
						entry.wepKey3[0],entry.wepKey3[1],entry.wepKey3[2],entry.wepKey3[3],entry.wepKey3[4],
						entry.wepKey4[0],entry.wepKey4[1],entry.wepKey4[2],entry.wepKey4[3],entry.wepKey4[4]);
				else
					sprintf(tmp1,"%d,%d,%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x,%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x,%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x,%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
						entry.auth,
						entry.wep_default_key,
						entry.wepKey1[0],entry.wepKey1[1],entry.wepKey1[2],entry.wepKey1[3],entry.wepKey1[4],entry.wepKey1[5],entry.wepKey1[6],entry.wepKey1[7],entry.wepKey1[8],
						entry.wepKey1[9],entry.wepKey1[10],entry.wepKey1[11],entry.wepKey1[12],
						entry.wepKey2[0],entry.wepKey2[1],entry.wepKey2[2],entry.wepKey2[3],entry.wepKey2[4],entry.wepKey2[5],entry.wepKey2[6],entry.wepKey2[7],entry.wepKey2[8],
						entry.wepKey2[9],entry.wepKey2[10],entry.wepKey2[11],entry.wepKey2[12],
						entry.wepKey3[0],entry.wepKey3[1],entry.wepKey3[2],entry.wepKey3[3],entry.wepKey3[4],entry.wepKey3[5],entry.wepKey3[6],entry.wepKey3[7],entry.wepKey3[8],
						entry.wepKey3[9],entry.wepKey3[10],entry.wepKey3[11],entry.wepKey3[12],
						entry.wepKey4[0],entry.wepKey4[1],entry.wepKey4[2],entry.wepKey4[3],entry.wepKey4[4],entry.wepKey4[5],entry.wepKey4[6],entry.wepKey4[7],entry.wepKey4[8],
						entry.wepKey4[9],entry.wepKey4[10],entry.wepKey4[11],entry.wepKey4[12]);	
				
				sprintf(iwprivCmd,"iwpriv %s set_mib ap_profile_add=\"%s\",%d,%s,",ifname,entry.ssid,entry.encryption, tmp1);
			}
			else if(entry.encryption == 3 || entry.encryption == 4) //wpa or wpa2
			{
				char tmp1[400];
				sprintf(tmp1, "%d,%s", entry.wpa_cipher, entry.wpaPSK);
				sprintf(iwprivCmd,"iwpriv %s set_mib ap_profile_add=\"%s\",%d,0,%s",ifname,entry.ssid,entry.encryption,tmp1 );
			}

	
			system(iwprivCmd);
		}

		vwlan_idx = ori_vwlan_idx;
		apmib_update_web(CURRENT_SETTING);
		
		
	}

	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page
	
	strTmp = req_get_cstream_var(wp, "restartNow", ""); 
	if(strTmp[0])
	{
		
		//apmib_update_web(CURRENT_SETTING);
#ifdef REBOOT_CHECK
		run_init_script_flag = 1;
#endif
#ifndef NO_ACTION
		run_init_script("all");
#endif
		REBOOT_WAIT(submitUrl);
		needReboot = 0;
	}
	else
	{
		send_redirect_perm(wp,submitUrl);
	}
	
}
#endif //#if defined(WLAN_PROFILE)



#endif //#ifdef REBOOT_CHECK

void formSysLog(request *wp, char *path, char *query)
{
	char *submitUrl, *tmpStr;
	char tmpBuf[100];
	int enabled, rt_enabled;
	struct in_addr ipAddr ;
	
	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page

	tmpStr = req_get_cstream_var(wp, ("clear"), "");  
	if(tmpStr[0]){
		snprintf(tmpBuf, 100, "echo \" \" > %s", "/var/log/messages");
		system(tmpBuf);
		//### add by sen_liu 2011.4.21 sync the system log update (enlarge from 1 pcs to 8 pcs) to  SDKv2.5 from kernel 2.4
#ifdef RINGLOG
		system("rm /var/log/messages.* >/dev/null 2>&1");
#endif
		//### end
		send_redirect_perm(wp, submitUrl);
		return;
	}

/*
 *	NOTE: If variable enabled (MIB_SCRLOG_ENABLED) bitmask modify(bitmap),
 *	 	Please modify driver rtl8190 reference variable (dot1180211sInfo.log_enabled in linux-2.4.18/drivers/net/rtl8190/8190n_cfg.h) 
 */
	apmib_get(MIB_SCRLOG_ENABLED, (void *)&enabled);
	
	tmpStr = req_get_cstream_var(wp, ("logEnabled"), "");  
	if(!strcmp(tmpStr, "ON")) {
		enabled |= 1;

		tmpStr = req_get_cstream_var(wp, ("syslogEnabled"), "");
		if(!strcmp(tmpStr, "ON"))
			enabled |= 2;		
		else
			enabled &= ~2;
		
		tmpStr = req_get_cstream_var(wp, ("wlanlogEnabled"), "");
		if(!strcmp(tmpStr, "ON")) 
			enabled |= 4;	
		else
			enabled &= ~4;
		
#ifdef HOME_GATEWAY
#ifdef DOS_SUPPORT
		tmpStr = req_get_cstream_var(wp, ("doslogEnabled"), "");
		if(!strcmp(tmpStr, "ON")) 
			enabled |= 8;		
		else
			enabled &= ~8;		
#endif
#endif

#ifdef CONFIG_RTK_MESH
		tmpStr = req_get_cstream_var(wp, ("meshlogEnabled"), "");
		if(!strcmp(tmpStr, "ON")) 
			enabled |= 16;	
		else
			enabled &= ~16;
#endif

	}
	else
		enabled &= ~1;						

	if ( apmib_set(MIB_SCRLOG_ENABLED, (void *)&enabled) == 0) {
		strcpy(tmpBuf, ("Set log enable error!"));
		goto setErr;
	}
	
	if(enabled & 1){
		tmpStr = req_get_cstream_var(wp, ("rtLogEnabled"), "");  

		if(!strcmp(tmpStr, "ON"))
			rt_enabled= 1;
		else
			rt_enabled= 0;
		if ( apmib_set(MIB_REMOTELOG_ENABLED, (void *)&rt_enabled) == 0) {
			strcpy(tmpBuf, ("Set remote log enable error!"));
			goto setErr;
		}

		tmpStr = req_get_cstream_var(wp, ("logServer"), "");  
		if(tmpStr[0]){
			inet_aton(tmpStr, &ipAddr);
			if ( apmib_set(MIB_REMOTELOG_SERVER, (void *)&ipAddr) == 0) {
				strcpy(tmpBuf, ("Set remote log server error!"));
				goto setErr;
			}
		}
	}
	apmib_update_web(CURRENT_SETTING);
#ifndef NO_ACTION
	run_init_script("all");
#endif
	OK_MSG(submitUrl);
	return;

setErr:
	ERR_MSG(tmpBuf);
}

static int process_msg(char *msg, int is_wlan_only)
{
	char *p1, *p2;
	p1 = strstr(msg, "rlx-linux"); // host name
	if (p1 == NULL)
		return 0;

#ifdef CONFIG_RTK_MESH	
	if (is_wlan_only == 4) {
		p2 = strstr(p1, "msh");
		if (p2 && p2[4]==':')
			memcpy(p1, p2, strlen(p2)+1);
		else
			return 0;

	}else	
#endif

	if (is_wlan_only == 3){
		p2 = strstr(p1, "DoS");
		if (p2 && p2[3]==':'){
			memcpy(p1, p2, strlen(p2)+1);
		}else{
			p2 = strstr(p1, "wlan");	
			if ((p2 && p2[5]==':') || (p2 && p2[9]==':'))	{// vxd interface
				memcpy(p1, p2, strlen(p2)+1);
			}else	
				return 0;
			}	
	}else if (is_wlan_only == 2){
		p2 = strstr(p1, "DoS");
		if (p2 && p2[3]==':')
			memcpy(p1, p2, strlen(p2)+1);
		else
			return 0;

	}else{
		p2 = strstr(p1, "wlan");	
		if ((p2 && p2[5]==':') ||
			 (p2 && p2[9]==':'))	// vxd interface
			memcpy(p1, p2, strlen(p2)+1);
		else {
			if (is_wlan_only)
				return 0;
			
			p2 = strstr(p1, "kernel: ");
			if (p2 == NULL)
				return 0;
			memcpy(p1, p2+7, strlen(p2)-7+1);
		}
	}
	return 1;
}


int sysLogList(request *wp, int argc, char **argv)
{
	FILE *fp;
	char  buf[200];
	int nBytesSent=0;
	int enabled;

//### add by sen_liu 2011.4.21 sync the system log update (enlarge from 1 pcs to 8 pcs) to  SDKv2.5 from kernel 2.4
#ifdef RINGLOG
	char logname[32];
	int lognum = LOG_SPLIT;
#endif

//### end
	apmib_get(MIB_SCRLOG_ENABLED, (void *)&enabled);
	if ( !(enabled & 1))
		goto err1;
//### add by sen_liu 2011.4.21 sync the system log update (enlarge from 1 pcs to 8 pcs) to  SDKv2.5 from kernel 2.4
#ifdef RINGLOG
		fp = fopen("/var/log/log_split", "r");
		if (fp == NULL)
			goto err1;
		fgets(buf,200,fp);
		lognum = atoi(buf);
		fclose(fp);
	
	while (lognum >= 0)
	{
		if (lognum > 0)
			snprintf(logname, 32, "/var/log/messages.%d", lognum-1);
		else if (lognum == 0)
			snprintf(logname, 32, "/var/log/messages");
		else
			goto err1;
	
		fp = fopen(logname, "r");
		if (fp == NULL)
			goto next_log;
#else
//### end
	fp = fopen("/var/log/messages", "r");
	if (fp == NULL)
		goto err1;
#endif
        
	while(fgets(buf,200,fp)){
		int ret=0;
		if (enabled&2) // system all
			ret = process_msg(buf, 0);
		else {
			if((enabled&0xC) == 0xC){ //both wlan and DoS
				ret = process_msg(buf, 3);
			}else if (enabled&4)	// wlan only
				ret = process_msg(buf, 1);
			else if (enabled&8)	//DoS only
				ret = process_msg(buf, 2);

#ifdef CONFIG_RTK_MESH			
			 if(enabled&16 && ret==0)	// mesh only
				ret = process_msg(buf, 4);
#endif

		}
		if (ret==0)
			continue;
		nBytesSent += req_format_write(wp, ("%s"), buf);
	}
	fclose(fp);

//### add by sen_liu 2011.4.21 sync the system log update (enlarge from 1 pcs to 8 pcs) to	SDKv2.5 from kernel 2.4
#ifdef RINGLOG
next_log:
	lognum--;
}
#endif
//### end
err1:
	return nBytesSent;
}

#ifdef HOME_GATEWAY
#ifdef DOS_SUPPORT
void formDosCfg(request *wp, char *path, char *query)
{
	char	*submitUrl, *tmpStr;
	char	tmpBuf[100];
	int	floodCount=0,blockTimer=0;
	long	enabled = 0;

	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page

	apmib_get(MIB_DOS_ENABLED, (void *)&enabled);

	tmpStr = req_get_cstream_var(wp, ("dosEnabled"), "");
	if(!strcmp(tmpStr, "ON")) {
		enabled |= 1;

		tmpStr = req_get_cstream_var(wp, ("sysfloodSYN"), "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 2;
			tmpStr = req_get_cstream_var(wp, ("sysfloodSYNcount"), "");
			string_to_dec(tmpStr,&floodCount);
			if ( apmib_set(MIB_DOS_SYSSYN_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, ("Set DoS SYSSYN_FLOOD error!"));
				goto setErr;
			}
		}
		else{
			enabled &= ~2;
		}
		tmpStr = req_get_cstream_var(wp, ("sysfloodFIN"), "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 4;
			tmpStr = req_get_cstream_var(wp, ("sysfloodFINcount"), "");
			string_to_dec(tmpStr,&floodCount);
			if ( apmib_set(MIB_DOS_SYSFIN_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, ("Set DoS SYSFIN_FLOOD error!"));
				goto setErr;
			}
		}
		else{
			enabled &= ~4;
		}
		tmpStr = req_get_cstream_var(wp, ("sysfloodUDP"), "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 8;
			tmpStr = req_get_cstream_var(wp, ("sysfloodUDPcount"), "");
			string_to_dec(tmpStr,&floodCount);
			if ( apmib_set(MIB_DOS_SYSUDP_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, ("Set DoS SYSUDP_FLOOD error!"));
				goto setErr;
			}
		}
		else{
			enabled &= ~8;
		}
		tmpStr = req_get_cstream_var(wp, ("sysfloodICMP"), "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x10;
			tmpStr = req_get_cstream_var(wp, ("sysfloodICMPcount"), "");
			string_to_dec(tmpStr,&floodCount);
			if ( apmib_set(MIB_DOS_SYSICMP_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, ("Set DoS SYSICMP_FLOOD error!"));
				goto setErr;
			}
		}
		else{
			enabled &= ~0x10;
		}
		tmpStr = req_get_cstream_var(wp, ("ipfloodSYN"), "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x20;
			tmpStr = req_get_cstream_var(wp, ("ipfloodSYNcount"), "");
			string_to_dec(tmpStr,&floodCount);
			if ( apmib_set(MIB_DOS_PIPSYN_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, ("Set DoS PIPSYN_FLOOD error!"));
				goto setErr;
			}
		}
		else{
			enabled &= ~0x20;
		}
		tmpStr = req_get_cstream_var(wp, ("ipfloodFIN"), "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x40;
			tmpStr = req_get_cstream_var(wp, ("ipfloodFINcount"), "");
			string_to_dec(tmpStr,&floodCount);
			if ( apmib_set(MIB_DOS_PIPFIN_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, ("Set DoS PIPFIN_FLOOD error!"));
				goto setErr;
			}
		}
		else{
			enabled &= ~0x40;
		}
		tmpStr = req_get_cstream_var(wp, ("ipfloodUDP"), "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x80;
			tmpStr = req_get_cstream_var(wp, ("ipfloodUDPcount"), "");
			string_to_dec(tmpStr,&floodCount);
			if ( apmib_set(MIB_DOS_PIPUDP_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, ("Set DoS PIPUDP_FLOOD error!"));
				goto setErr;
			}
		}
		else{
			enabled &= ~0x80;
		}
		tmpStr = req_get_cstream_var(wp, ("ipfloodICMP"), "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x100;
			tmpStr = req_get_cstream_var(wp, ("ipfloodICMPcount"), "");
			string_to_dec(tmpStr,&floodCount);
			if ( apmib_set(MIB_DOS_PIPICMP_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, ("Set DoS PIPICMP_FLOOD error!"));
				goto setErr;
			}
		}
		else{
			enabled &= ~0x100;
		}
		tmpStr = req_get_cstream_var(wp, ("TCPUDPPortScan"), "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x200;

			tmpStr = req_get_cstream_var(wp, ("portscanSensi"), "");
			if( tmpStr[0]=='1' ) {
				enabled |= 0x800000;
			}
			else{
				enabled &= ~0x800000;
			}
		}
		else{
			enabled &= ~0x200;
		}
		tmpStr = req_get_cstream_var(wp, ("ICMPSmurfEnabled"), "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x400;
		}
		else{
			enabled &= ~0x400;
		}
		tmpStr = req_get_cstream_var(wp, ("IPLandEnabled"), "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x800;
		}
		else{
			enabled &= ~0x800;
		}
		tmpStr = req_get_cstream_var(wp, ("IPSpoofEnabled"), "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x1000;
		}
		else{
			enabled &= ~0x1000;
		}
		tmpStr = req_get_cstream_var(wp, ("IPTearDropEnabled"), "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x2000;
		}
		else{
			enabled &= ~0x2000;
		}
		tmpStr = req_get_cstream_var(wp, ("PingOfDeathEnabled"), "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x4000;
		}
		else{
			enabled &= ~0x4000;
		}
		tmpStr = req_get_cstream_var(wp, ("TCPScanEnabled"), "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x8000;
		}
		else{
			enabled &= ~0x8000;
		}
		tmpStr = req_get_cstream_var(wp, ("TCPSynWithDataEnabled"), "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x10000;
		}
		else{
			enabled &= ~0x10000;
		}
		tmpStr = req_get_cstream_var(wp, ("UDPBombEnabled"), "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x20000;
		}
		else{
			enabled &= ~0x20000;
		}
		tmpStr = req_get_cstream_var(wp, ("UDPEchoChargenEnabled"), "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x40000;
		}
		else{
			enabled &= ~0x40000;
		}
		tmpStr = req_get_cstream_var(wp, ("sourceIPblock"), "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x400000;
			tmpStr = req_get_cstream_var(wp, ("IPblockTime"), "");
			string_to_dec(tmpStr,&blockTimer);
			if ( apmib_set(MIB_DOS_BLOCK_TIME, (void *)&blockTimer) == 0) {
				strcpy(tmpBuf, ("Set DoS IP Block Timer error!"));
				goto setErr;
			}
		}
		else{
			enabled &= ~0x400000;
		}
	}
	else
		enabled = 0;

	if ( apmib_set(MIB_DOS_ENABLED, (void *)&enabled) == 0) {
		strcpy(tmpBuf, ("Set DoS enable error!"));
		goto setErr;
	}

	apmib_update_web(CURRENT_SETTING);

#ifndef NO_ACTION
	run_init_script("all");
#endif

	OK_MSG(submitUrl);
	return;

setErr:
	ERR_MSG(tmpBuf);
}
#endif
#endif


#ifdef LOGIN_URL

#define MAX_USER	5
#define ACCESS_TIMEOUT	 300	// 5m

#define MAGIC_NUMER	7168186

struct user_profile {
	int flag;
	time_t last_time;
	char ipaddr[32];		
};

static struct user_profile users[MAX_USER];

///////////////////////////////////////////////////////////////////
static void delete_user(request *wp)
{
	int i;
	for (i=0; i<MAX_USER; i++) {
		if (users[i].flag == MAGIC_NUMER && !strcmp(wp->ipaddr, users[i].ipaddr)) {
			users[i].flag = 0;
			return;
		}			
	}
}

///////////////////////////////////////////////////////////////////
static int add_user(request *wp)
{
	int i;
	for (i=0; i<MAX_USER; i++) {
		if (users[i].flag == MAGIC_NUMER && strcmp(wp->ipaddr, users[i].ipaddr) &&
			((unsigned long)wp->timestamp)-((unsigned long)users[i].last_time) < ACCESS_TIMEOUT )
			continue;
				
		users[i].flag = MAGIC_NUMER;		
		users[i].last_time = wp->timestamp;	
		strcpy(users[i].ipaddr, wp->ipaddr);				
		return 0;
	}

	printf("boa: add_user error (exceed max connection)!\n");

	return -1;
}

///////////////////////////////////////////////////////////////////
int is_valid_user(request *wp)
{
	int i;
	for (i=0; i<MAX_USER; i++) {
		if (users[i].flag == MAGIC_NUMER && !strcmp(wp->ipaddr, users[i].ipaddr)) {
			if (((unsigned long)wp->timestamp)-((unsigned long)users[i].last_time) > ACCESS_TIMEOUT)
				return -1; // timeout
			return 1;
		}
	}

	return 0; // not a valid user
}

///////////////////////////////////////////////////////////////////
void formLogin(request *wp, char *path, char *query)
{
	char *strUser, *strPassword, *userpass;
	char tmpbuf[200];

	strUser = req_get_cstream_var(wp, ("username"), "");
	strPassword = req_get_cstream_var(wp, ("password"), "");
	if ( strUser[0] && !strPassword[0] ) {
		strcpy(tmpbuf, ("ERROR: Password cannot be empty."));
		goto login_err;
	}

	if (!umUserExists(strUser)) {
		strcpy(tmpbuf, ("ERROR: Access denied, unknown user!"));
		goto login_err;
	}
	userpass = umGetUserPassword(strUser);
	if (userpass) {
		if (strcmp(strPassword, userpass) != 0) {
			strcpy(tmpbuf, ("ERROR: Access denied, unknown user!"));
			goto login_err;
		}
	}

	if (add_user(wp) < 0) {
		strcpy(tmpbuf, ("ERROR: Exceed max user number!"));
		goto login_err;
	}

	send_redirect_perm(wp, ("home.htm"));
	return;

login_err:
	ERR_MSG(tmpbuf);
}
#endif // LOGIN_URL

#if defined(POWER_CONSUMPTION_SUPPORT)
unsigned int pre_cpu_d4, pre_time_secs, max_cpu_delta=0;
unsigned int ethBytesCount_previous[5] = {0};

/* http://www.360doc.com/content/070213/11/17255_365683.html */
int getPowerConsumption(request *wp, int argc, char **argv)
{
	//char dev[80];
	//char *devPtr;
	FILE *stream;
	int i=1;
	//int j;
	//char logbuf[500];
	//unsigned int rxbytes=0,rxpackets=0,rxerrs=0,rxdrops=0,txbytes=0,txpackets=0,txerrs=0,txdrops=0,txcolles=0;
	//unsigned int txeth0packets=0;
	//unsigned int tmp1,tmp2,tmp3,tmp4;
	char askfor[20];

//	unsigned int totalPwrCon = 0;
	unsigned int totalPwrCon = (rand()%2 ? 10 :0);
	
	typedef enum { NO_LINK=0, NORMAL_LINK=1, EEE_LINK=2} ETHERNET_LINK_T;
	unsigned short isLink_eth0[5]={0};
	unsigned short ethLinkNum= 0, ethEeeLinkNum = 0;
	unsigned short perEthPwrCon = PWRCON_PER_ETHERNET;
	unsigned int perEthEeeMinus = PWRCON_PER_EEE_ETHERNET_LINK_MINUS; // mw*100
	unsigned int perEthEeePwrCon = PWRCON_PER_EEE_ETHERNET; // mw*100/Mbps
	unsigned int ethThroughPut[5] = {0};
	unsigned int ethEeeThroughPut_Total = 0;
	int ethPwrCon_Total = 0;
	
	typedef enum { CHIP_UNKNOWN=0, CHIP_RTL8188C=1, CHIP_RTL8192C=2} CHIP_VERSION_T;
	CHIP_VERSION_T chipVersion = CHIP_UNKNOWN;	
	
	typedef enum { CPU_NORMAL=0, CPU_SUSPEND=1} CPU_MODE_T;
	CPU_MODE_T cpuMode = CPU_NORMAL;
	unsigned short cpuPwrCon[3][2] = { {0,0},{PWRCON_CPU_NORMAL_88C,PWRCON_CPU_SUSPEND_88C},{PWRCON_CPU_NORMAL_92C,PWRCON_CPU_SUSPEND_92C} }; // 3:chipVersion; 2:cpu mode
	
	typedef enum { WLAN_OFF=0, WLAN_NO_LINK=1, WLAN_LINK=2} WLAN_STATE_T;
	WLAN_STATE_T wlanState = WLAN_OFF; 
	unsigned short wlanStatePwrCon[3][3] = { {0,0,0},{PWRCON_WLAN_OFF_88C,PWRCON_WLAN_NOLINK_88C,PWRCON_WLAN_LINK_88C},{PWRCON_WLAN_OFF_92C,PWRCON_WLAN_NOLINK_92C,PWRCON_WLAN_LINK_92C}}; //3:chipVersion; 3:wlanState
	int wlanOff = 0;
	
	typedef enum { WLAN_MCS8_15=0, WLAN_MCS0_7=1, WLAN_OFDM=2, WLAN_CCK=3} WLAN_TRAFFIC_STATE_T;
	WLAN_TRAFFIC_STATE_T wlanTrafficState = WLAN_MCS8_15;
	unsigned int wlanTrafficStatePwrCon[3][4] = { {0,0,0,0},{PWRCON_WLAN_TRAFFIC_MCS8_15_88C,PWRCON_WLAN_TRAFFIC_MCS0_7_88C,PWRCON_WLAN_TRAFFIC_OFDM_88C,PWRCON_WLAN_TRAFFIC_CCK_88C},{PWRCON_WLAN_TRAFFIC_MCS8_15_92C,PWRCON_WLAN_TRAFFIC_MCS0_7_92C,PWRCON_WLAN_TRAFFIC_OFDM_92C,PWRCON_WLAN_TRAFFIC_CCK_92C}}; //3:chipVersion; 4:wlanTrafficState
	unsigned int wlanTrafficStatePwrConZ[3][28] = { 
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{1000,1099,1188,1454,2014,3254,4271,8039,1082,1176,1289,1681,2768,4482,6275,10458,719,919,1225,1697,2377,3735,5038,7557,938,1681,2894,5865},
		{1000,1099,1188,1454,2014,3254,4271,8039,1082,1176,1289,1681,2768,4482,6275,10458,1000,1278,1705,2360,3306,5195,7008,10511,938,1681,2894,5865}
	};//3:chipVersion; 28:DataRate MCS15~1
	unsigned int wlanTrafficZ = 0;
	
	unsigned int tx_average = 0;
	unsigned short tx_average_multiply2 = 0;
	unsigned int rx_average = 0;
	unsigned int wlanTrafficStatePwrCon_Total;
		
	unsigned int cpuUtilizationPwrCon[3] = { 0,PWRCON_CPU_UTILIZATION_88C,PWRCON_CPU_UTILIZATION_92C}; //3:chipVersion;
	unsigned short cpu_utilization=0;
	
	unsigned short debug_check = 0;
	
	time_t current_secs;
	unsigned int time_delta = 1;						
#if 0	
	for(i=0 ;i<3;i++)
		for(j=0; j<1; j++)
			fprintf(stderr,"\r\n cpuUtilizationPwrCon[%d][%d]=[%f]",i,j,cpuUtilizationPwrCon[i][j]);
#endif			

	//get current system time in second.
	time(&current_secs);
	if(pre_time_secs == 0) //first time
	{
		pre_time_secs = (int)(current_secs);
		time_delta = 1;
	}
	else
	{
		time_delta = (int)(current_secs) - (int)(pre_time_secs);
		pre_time_secs = (int)(current_secs);
	}

		
	//get chipVersion
	stream = fopen ( "/var/pwrConDebug", "r" );
	if ( stream != NULL )
	{		
		char *strtmp;
		char line[100];				
		char strTmp[10];
		
		while (fgets(line, sizeof(line), stream))
		{
			strtmp = line;
			
			while(*strtmp == ' ')
			{
				strtmp++;
			}

			sscanf(strtmp,"%[01]",strTmp);

			debug_check=atoi(strTmp);
			
		}
		
		fclose ( stream );
	}

	
	if(debug_check)
		fprintf(stderr,"\r\n  === Pwr Con Debug ===");
	//get chipVersion
	chipVersion = getWLAN_ChipVersion();
#if 0	
	stream = fopen ( "/proc/wlan0/mib_rf", "r" );
	if ( stream != NULL )
	{		
		char *strtmp;
		char line[100];
								 
		while (fgets(line, sizeof(line), stream))
		{
			
			strtmp = line;
			while(*strtmp == ' ')
			{
				strtmp++;
			}
			

			if(strstr(strtmp,"RTL8192SE") != 0)
			{
				chipVersion = CHIP_UNKNOWN;
			}
			else if(strstr(strtmp,"RTL8188C") != 0)
			{
				if(debug_check)
					fprintf(stderr,"\r\n [%s]",strtmp);				
				chipVersion = CHIP_RTL8188C;
			}
			else if(strstr(strtmp,"RTL8192C") != 0)
			{
				if(debug_check)				
					fprintf(stderr,"\r\n [%s]",strtmp);				
				chipVersion = CHIP_RTL8192C;
			}
		}			
		fclose ( stream );
	}
#endif

	if(debug_check)
	{
		fprintf(stderr,"\r\n chipVersion=[%u]",chipVersion);
		fprintf(stderr,"\r\n");
	}
	
	//get cpu mode
	stream = fopen ( "/proc/suspend_check", "r" );
	if ( stream != NULL )
	{		
		char *strtmp;
		char line[100];
		
		while (fgets(line, sizeof(line), stream))
		{			
			//enable=1, winsize=5(10), high=3200, low=2200, suspend=1
			strtmp = strstr(line,"suspend");
			if(strtmp != NULL)
			{
				
				//suspend=1
				if(debug_check)
					fprintf(stderr,"\r\n [%s]",strtmp);
				sscanf(strtmp,"%*[^=]=%u",&cpuMode);								
			}
			
		}			
		fclose ( stream );
	}
	if(debug_check)
	{
		fprintf(stderr,"\r\n cpuMode=[%u]",cpuMode);
		fprintf(stderr,"\r\n cpuPwrCon=[%u]",cpuPwrCon[chipVersion][cpuMode]);
		fprintf(stderr,"\r\n");
	}
	totalPwrCon+=cpuPwrCon[chipVersion][cpuMode];
	
	//get Eth0 port link and bytesCount
	for(i=0; i<5; i++)
	{
		unsigned int ethBytesCount[5] = {0};
		
		isLink_eth0[i]=getEth0PortLink(i);
		if(isLink_eth0[i])
		{
			isLink_eth0[i] = NORMAL_LINK;
			if(getEthernetEeeState(i))
				isLink_eth0[i] = EEE_LINK;			
	}
		else
		{
			isLink_eth0[i] = NO_LINK;
		}
		
		ethBytesCount[i] = getEthernetBytesCount(i);
		
		if(time_delta <= 0)
			time_delta = 1;
		ethThroughPut[i] = (ethBytesCount[i] - ethBytesCount_previous[i])/time_delta;		
		ethBytesCount_previous[i] = ethBytesCount[i];
	}
	
	for(i=0; i<5; i++)
	{
		if(isLink_eth0[i] == NORMAL_LINK)
		{
			ethLinkNum++;
		}
		else if(isLink_eth0[i] == EEE_LINK)
		{
			ethEeeLinkNum++;
			ethEeeThroughPut_Total += ethThroughPut[i];
		}						
	}
	ethEeeThroughPut_Total *= 8; // transfer to bits.
	
	ethPwrCon_Total += ethLinkNum*perEthPwrCon;
	ethPwrCon_Total -= (ethEeeLinkNum*perEthEeeMinus)/100;
	ethPwrCon_Total += (((float)ethEeeThroughPut_Total*perEthEeePwrCon)/100)/1000000;
	
	
	if(debug_check)
	{
		fprintf(stderr,"\r\n Eth Link State:%u-%u-%u-%u-%u", isLink_eth0[0],isLink_eth0[1],isLink_eth0[2],isLink_eth0[3],isLink_eth0[4]);
		fprintf(stderr,"\r\n Eth ThroughPut:%u-%u-%u-%u-%u (bits/sec)", ethThroughPut[0]*8,ethThroughPut[1]*8,ethThroughPut[2]*8,ethThroughPut[3]*8,ethThroughPut[4]*8);
		fprintf(stderr,"\r\n ethEeeThroughPut_Total: %u (bits/sec)",ethEeeThroughPut_Total);
		fprintf(stderr,"\r\n perEthPwrCon Total: (%u*%u)-(%u*%u)/100+(%u*%u)/100/10^6 = %u",ethLinkNum,perEthPwrCon,ethEeeLinkNum,perEthEeeMinus,ethEeeThroughPut_Total,perEthEeePwrCon,ethPwrCon_Total);
		fprintf(stderr,"\r\n");
	}
	totalPwrCon+=ethPwrCon_Total;

	//get wlan state
	apmib_get( MIB_WLAN_WLAN_DISABLED, (void *)&wlanOff);
	if(wlanOff)
		wlanState = WLAN_OFF;
	else
	{
		wlanState = updateWlanifState("wlan0");								
	}										
					
	
	if(debug_check)
					{
		fprintf(stderr,"\r\n wlanState=[%u]",wlanState);	
		fprintf(stderr,"\r\n wlanStatePwrCon = [%u]",wlanStatePwrCon[chipVersion][wlanState]);
		fprintf(stderr,"\r\n");
	}
						
	totalPwrCon+=wlanStatePwrCon[chipVersion][wlanState];
		
	// get wlan traffic power consumption
	if(wlanState == WLAN_LINK)
	{
			//get chipVersion
		stream = fopen ( "/proc/wlan0/stats", "r" );
		if ( stream != NULL )
		{		
			char *strtmp;
			char line[100];
			while (fgets(line, sizeof(line), stream))
			{
				char *p;
				strtmp = line;
				
				
				while(*strtmp == ' ')
					strtmp++;
				
				
				if(strstr(strtmp,"tx_avarage") != 0)
				{
					char str1[10];
					
					if(debug_check)																
						fprintf(stderr,"\r\n [%s]",strtmp);
						
					//tx_avarage:    1449
					sscanf(strtmp, "%*[^:]:%s",str1);
					
					p = str1;
					while(*p == ' ')
						p++;
					
					tx_average = atoi(p);	
					tx_average*=8; // bytes->bits
					
					if(debug_check)
						fprintf(stderr,"\r\n tx_average=[%u]",tx_average);
				}
				else if(strstr(strtmp,"rx_avarage") != 0)
				{
					char str1[10];
					
					if(debug_check)																
						fprintf(stderr,"\r\n [%s]",strtmp);
						
					//rx_avarage:    1449
					sscanf(strtmp, "%*[^:]:%s",str1);
					
					p = str1;
					while(*p == ' ')
						p++;
					
					rx_average = atoi(p);	
					rx_average*=8; // bytes->bits
					
					if(debug_check)
						fprintf(stderr,"\r\n rx_average=[%u]",rx_average);
					}
				else if(strstr(strtmp,"cur_tx_rate") != 0)
				{
					char str1[10];
					unsigned short OFDM_CCK = 0;
					
					if(debug_check)
						fprintf(stderr,"\r\n [%s]",strtmp);
					
					//cur_tx_rate:   MCS[8-15]
					//cur_tx_rate:   MCS[0-7]
					//cur_tx_rate:   [1,2,5,11]
					//cur_tx_rate:   [6,9,12,18,24,36,48,54]
					sscanf(strtmp, "%*[^:]:%s",str1);
					p = str1;
					while(*p == ' ')
						p++;
					
					if(debug_check)
						fprintf(stderr,"\r\n p=[%s]",p);
											
					if(strstr(p, "MCS8") != 0 || strstr(p, "MCS9") != 0 ||
						 strstr(p, "MCS10") != 0 || strstr(p, "MCS11") != 0 ||
						 strstr(p, "MCS12") != 0 || strstr(p, "MCS13") != 0 ||
						 strstr(p, "MCS14") != 0 || strstr(p, "MCS15") != 0 )
					{
						wlanTrafficState = WLAN_MCS8_15;																																	
					}
					else if(strstr(p, "MCS0") != 0 || strstr(p, "MCS1") != 0 ||
									 strstr(p, "MCS2") != 0 || strstr(p, "MCS3") != 0 ||
									 strstr(p, "MCS4") != 0 || strstr(p, "MCS5") != 0 ||
									 strstr(p, "MCS6") != 0 || strstr(p, "MCS7") != 0 )
					{						
						wlanTrafficState = WLAN_MCS0_7;						
					}
					else
					{
						OFDM_CCK = atoi(p);
						
						if(OFDM_CCK == 1 || OFDM_CCK == 2 || OFDM_CCK == 5 || OFDM_CCK ==11)										 
						{
							wlanTrafficState = WLAN_CCK;													
						}
						else if(OFDM_CCK == 6 || OFDM_CCK == 9 || OFDM_CCK == 12 || OFDM_CCK == 18 ||
							      OFDM_CCK == 24 || OFDM_CCK == 36 || OFDM_CCK == 48 || OFDM_CCK == 54 )
						{
							wlanTrafficState = WLAN_OFDM;
						}
					}
					
					if(strstr(p, "MCS15") != 0)
						wlanTrafficZ = wlanTrafficStatePwrConZ[chipVersion][0];
					else if(strstr(p, "MCS14") != 0)
						wlanTrafficZ = wlanTrafficStatePwrConZ[chipVersion][1];
					else if(strstr(p, "MCS13") != 0)
						wlanTrafficZ = wlanTrafficStatePwrConZ[chipVersion][2];
					else if(strstr(p, "MCS12") != 0)
						wlanTrafficZ = wlanTrafficStatePwrConZ[chipVersion][3];						
					else if(strstr(p, "MCS11") != 0)
						wlanTrafficZ = wlanTrafficStatePwrConZ[chipVersion][4];
					else if(strstr(p, "MCS10") != 0)
						wlanTrafficZ = wlanTrafficStatePwrConZ[chipVersion][5];
					else if(strstr(p, "MCS9") != 0)
						wlanTrafficZ = wlanTrafficStatePwrConZ[chipVersion][6];
					else if(strstr(p, "MCS8") != 0)
						wlanTrafficZ = wlanTrafficStatePwrConZ[chipVersion][7];
					else if(strstr(p, "MCS7") != 0)
						wlanTrafficZ = wlanTrafficStatePwrConZ[chipVersion][8];
					else if(strstr(p, "MCS6") != 0)
						wlanTrafficZ = wlanTrafficStatePwrConZ[chipVersion][9];
					else if(strstr(p, "MCS5") != 0)
						wlanTrafficZ = wlanTrafficStatePwrConZ[chipVersion][10];
					else if(strstr(p, "MCS4") != 0)
						wlanTrafficZ = wlanTrafficStatePwrConZ[chipVersion][11];
					else if(strstr(p, "MCS3") != 0)
						wlanTrafficZ = wlanTrafficStatePwrConZ[chipVersion][12];
					else if(strstr(p, "MCS2") != 0)	
						wlanTrafficZ = wlanTrafficStatePwrConZ[chipVersion][13];
					else if(strstr(p, "MCS1") != 0)	
						wlanTrafficZ = wlanTrafficStatePwrConZ[chipVersion][14];
					else if(strstr(p, "MCS0") != 0)	
						wlanTrafficZ = wlanTrafficStatePwrConZ[chipVersion][15];																
					else if(OFDM_CCK == 54)
						wlanTrafficZ = wlanTrafficStatePwrConZ[chipVersion][16];							
					else if(OFDM_CCK == 48)
						wlanTrafficZ = wlanTrafficStatePwrConZ[chipVersion][17];
					else if(OFDM_CCK == 36)
						wlanTrafficZ = wlanTrafficStatePwrConZ[chipVersion][18];
					else if(OFDM_CCK == 24)
						wlanTrafficZ = wlanTrafficStatePwrConZ[chipVersion][19];
					else if(OFDM_CCK == 18)
						wlanTrafficZ = wlanTrafficStatePwrConZ[chipVersion][20];
					else if(OFDM_CCK == 12)
						wlanTrafficZ = wlanTrafficStatePwrConZ[chipVersion][21];
					else if(OFDM_CCK == 9)
						wlanTrafficZ = wlanTrafficStatePwrConZ[chipVersion][22];
					else if(OFDM_CCK == 6)
						wlanTrafficZ = wlanTrafficStatePwrConZ[chipVersion][23];
					else if(OFDM_CCK == 11)
						wlanTrafficZ = wlanTrafficStatePwrConZ[chipVersion][24];
					else if(OFDM_CCK == 5)
						wlanTrafficZ = wlanTrafficStatePwrConZ[chipVersion][25];
					else if(OFDM_CCK == 2)
						wlanTrafficZ = wlanTrafficStatePwrConZ[chipVersion][26];
					else if(OFDM_CCK == 1)
						wlanTrafficZ = wlanTrafficStatePwrConZ[chipVersion][27];
															
				}
				
			}
			fclose(stream );
			
		}
	}
	
	if(debug_check)
		fprintf(stderr,"\r\n wlanTrafficState=[%u], wlanTrafficZ=[%u]",wlanTrafficState, wlanTrafficZ);

	switch(wlanTrafficState)
	{
		case WLAN_MCS8_15:
			//tx_average /= 1000000;
			if(tx_average > 95000000)
				tx_average = 95000000;
									
			wlanTrafficStatePwrCon_Total = ((((float)tx_average*wlanTrafficStatePwrCon[chipVersion][wlanTrafficState]*wlanTrafficZ)/1000)/100)/1000000;
			if(debug_check)
				fprintf(stderr,"\r\n wlanTrafficStatePwrCon_Total:(((%u*%u*%u)/1000)/100)/10^6 = [%u]",tx_average,wlanTrafficStatePwrCon[chipVersion][wlanTrafficState],wlanTrafficZ,wlanTrafficStatePwrCon_Total);
	
			totalPwrCon+=wlanTrafficStatePwrCon_Total;
			break;
		case WLAN_MCS0_7:
			//tx_average /= 1000000;
			if(tx_average > 90000000)
				tx_average = 90000000;
		
			wlanTrafficStatePwrCon_Total = ((((float)tx_average*wlanTrafficStatePwrCon[chipVersion][wlanTrafficState]*wlanTrafficZ)/1000)/100)/1000000;
			if(debug_check)
				fprintf(stderr,"\r\n wlanTrafficStatePwrCon_Total:(((%u*%u*%u)/1000)/100)/10^6 = [%u]",tx_average,wlanTrafficStatePwrCon[chipVersion][wlanTrafficState],wlanTrafficZ,wlanTrafficStatePwrCon_Total);
			
			totalPwrCon+=wlanTrafficStatePwrCon_Total;
			break;
		case WLAN_OFDM:
			//tx_average /= 1000000;
			if(tx_average > 25000000)
				tx_average = 25000000;
			
			wlanTrafficStatePwrCon_Total = ((((float)tx_average*wlanTrafficStatePwrCon[chipVersion][wlanTrafficState]*wlanTrafficZ)/1000)/100)/1000000;
			if(debug_check)
				fprintf(stderr,"\r\n wlanTrafficStatePwrCon_Total:(((%u*%u*%u)/1000)/100)/10^6 = [%u]",tx_average,wlanTrafficStatePwrCon[chipVersion][wlanTrafficState],wlanTrafficZ,wlanTrafficStatePwrCon_Total);
			
			totalPwrCon+=wlanTrafficStatePwrCon_Total;
			break;
		case WLAN_CCK:

		wlanTrafficStatePwrCon_Total = ((((float)tx_average*wlanTrafficStatePwrCon[chipVersion][wlanTrafficState]*wlanTrafficZ)/1000)/100)/1000000;
			if(debug_check)
				fprintf(stderr,"\r\n wlanTrafficStatePwrCon_Total:(((%u*%u*%u)/1000)/100)/10^6 = [%u]",tx_average, wlanTrafficStatePwrCon[chipVersion][wlanTrafficState],wlanTrafficZ,wlanTrafficStatePwrCon_Total);
			totalPwrCon+=wlanTrafficStatePwrCon_Total;
			break;						
	}
	
	//get CPU utilization
	stream = fopen ( "/proc/stat", "r" );
	if ( stream != NULL )
	{
		char buf[512];
		unsigned int d1, d2, d3, d4;
		
		fgets(buf, sizeof(buf), stream);	/* eat line */
				
		
		sscanf(buf, "cpu %d %d %d %d", &d1, &d2, &d3, &d4);
		fclose(stream);
				
		if(pre_cpu_d4 == 0)
		{
			pre_cpu_d4 = d4;
		}
		else
		{			
			
			unsigned int delta = 0;						
				
			delta = (d4 - pre_cpu_d4)/time_delta;
			
			pre_cpu_d4 = d4;
			if(delta > max_cpu_delta)
				max_cpu_delta = delta;
			
			cpu_utilization = 100 - (int)(delta*100/max_cpu_delta);

			if(debug_check)
				fprintf(stderr,"\r\n cpu_busy: (%u*%u)/100=[%u] ",cpu_utilization,cpuUtilizationPwrCon[chipVersion],((cpu_utilization*cpuUtilizationPwrCon[chipVersion])/100));

	}

	}

	if(cpuMode == CPU_NORMAL)
		totalPwrCon+=((cpu_utilization*cpuUtilizationPwrCon[chipVersion])/100);


	if(1 || strcmp(askfor,"all")==0){

		
		if(debug_check)
		fprintf(stderr,"\r\n totalPwrCon=%u",totalPwrCon);
			
		if(tx_average_multiply2)
			tx_average/=2;
			
		req_format_write(wp, "<interface><name>LAN</name><type>LAN</type><totalPwrCon>%d</totalPwrCon><wlanTx>%d</wlanTx><wlanRx>%d</wlanRx></interface>",totalPwrCon,tx_average,rx_average);
		
	}

	return 0;
	
}
#endif // #if defined(POWER_CONSUMPTION_SUPPORT)
