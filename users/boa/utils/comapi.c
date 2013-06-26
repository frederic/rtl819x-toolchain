/*
 *      Operation routines for config-file-based API
 *
 */


/* System include files */
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <ctype.h>
#include <regex.h>


/* Local include files */
#include "apmib.h"
#include "mibtbl.h"

//#if defined(CONFIG_NET_RADIO)
#if defined(CONFIG_RTL8192E)
        #include "../../../linux-2.6.30/drivers/net/wireless/rtl8192e/ieee802_mib.h"
#elif defined(CONFIG_RTL8192CD)
	#include "../../../linux-2.6.30/drivers/net/wireless/rtl8192cd/ieee802_mib.h"
#else
	#include "../../../linux-2.6.30/drivers/net/wireless/rtl8190/ieee802_mib.h"
#endif
//#endif //#if defined(CONFIG_NET_RADIO)

#ifdef WIFI_SIMPLE_CONFIG
enum { 
	MODE_AP_UNCONFIG=1, 			// AP unconfigured (enrollee)
	MODE_CLIENT_UNCONFIG=2, 		// client unconfigured (enrollee) 
	MODE_CLIENT_CONFIG=3,			// client configured (registrar) 
	MODE_AP_PROXY=4, 			// AP configured (proxy)
	MODE_AP_PROXY_REGISTRAR=5,		// AP configured (proxy and registrar)
	MODE_CLIENT_UNCONFIG_REGISTRAR=6		// client unconfigured (registrar) 
};
#endif

void convert_bin_to_str(unsigned char *bin, int len, char *out)
{
        int i;
        char tmpbuf[10];

        out[0] = '\0';

        for (i=0; i<len; i++) {
                sprintf(tmpbuf, "%02x", bin[i]);
                strcat(out, tmpbuf);
        }
}

void fprintf_BArray(FILE *fp, unsigned char *name, unsigned char *ifname, unsigned char *val, unsigned int argc)
{
	int i;
	fprintf(fp, "%s_%s=", ifname, name);
	for (i=0; i<argc; i++)
		fprintf(fp, "%02x", val[i]);
	fprintf(fp, "\n");
}

int dumpCfgFile(char *ifname, struct wifi_mib *pmib, int idx)
{
	FILE *fp;
	int i, intVal, is_client, is_config, is_registrar, wep_key_type;
	unsigned char buf1[1024];
	int keylength;
	
#ifdef CONFIG_RTL8186_KLD_REPEATER
	int is_repeater_enabled;
	int wps_vxdAP_enabled=0;
	apmib_get(MIB_REPEATER_ENABLED1, (void *)&is_repeater_enabled);
#endif
	
	
#define FPRINTF_INT(str,val) \
		fprintf(fp, "%s_"str"=%d\n", ifname, val)
#define FPRINTF_LONG(str,val) \
		fprintf(fp, "%s_"str"=%ld\n", ifname, val)
#define FPRINTF_STR(str,val) \
		fprintf(fp, "%s_"str"=\"%s\"\n", ifname, val)
		
#if !defined(CONFIG_RTL_8196C)
#define CFG_FILE_PATH "/var/RTL8190N.dat"
#else
#define CFG_FILE_PATH "/var/RTL8192CD.dat"
#endif

	// open config file
	printf("dumpCfgFile!!!!!!!!!!!!!!!!!!! %s %d\n", ifname, idx);
	if (memcmp("wlan0", ifname, strlen(ifname)) == 0)
		fp = fopen( CFG_FILE_PATH,"w" );
	else
		fp = fopen( CFG_FILE_PATH,"a" );
	if (!fp){
		printf("Open configure file failed!\n");
		return -1;
	}
	
	FPRINTF_INT("regdomain", 			pmib->dot11StationConfigEntry.dot11RegDomain);
	fprintf_BArray(fp,"hwaddr",ifname,	pmib->dot11OperationEntry.hwaddr, 6);
	FPRINTF_INT("disable_brsc", 		pmib->dot11OperationEntry.disable_brsc);
	FPRINTF_INT("led_type", 			pmib->dot11OperationEntry.ledtype);
	//FPRINTF_INT("dot11DesiredSSIDLen", 	pmib->dot11StationConfigEntry.dot11DesiredSSIDLen);
	FPRINTF_STR("ssid", 				pmib->dot11StationConfigEntry.dot11DesiredSSID);
	//FPRINTF_INT("dot11SSIDtoScanLen", 	pmib->dot11StationConfigEntry.dot11SSIDtoScanLen);
	FPRINTF_STR("ssid2scan", 			pmib->dot11StationConfigEntry.dot11SSIDtoScan);
	FPRINTF_INT("opmode", 				pmib->dot11OperationEntry.opmode);
	//FPRINTF_INT("dot11DefaultSSIDLen", 	pmib->dot11StationConfigEntry.dot11DefaultSSIDLen);
	FPRINTF_STR("dot11DefaultSSID", 	pmib->dot11StationConfigEntry.dot11DefaultSSID);
	FPRINTF_INT("wds_pure", 				pmib->dot11WdsInfo.wdsPure);
	//FPRINTF_INT("RFChipID", 			pmib->dot11RFEntry.dot11RFType);
#if defined(CONFIG_RTL_92D_SUPPORT)
	FPRINTF_INT("phyBandSelect", 			pmib->dot11RFEntry.phyBandSelect);
	FPRINTF_INT("macPhyMode", 			pmib->dot11RFEntry.macPhyMode);
#endif

#if defined(CONFIG_RTL_819X) 
  #if 0 //!defined(CONFIG_RTL_8196C)
	FPRINTF_INT("MIMO_TR_mode",			pmib->dot11RFEntry.MIMO_TR_mode);
	fprintf_BArray(fp,"TxPowerCCK",ifname,pmib->dot11RFEntry.pwrlevelCCK,14);
	fprintf_BArray(fp,"TxPowerOFDM_1SS",ifname,pmib->dot11RFEntry.pwrlevelOFDM_1SS,162);
	fprintf_BArray(fp,"TxPowerOFDM_2SS",ifname,pmib->dot11RFEntry.pwrlevelOFDM_2SS,162);
	FPRINTF_INT("LOFDM_pwd_A",			pmib->dot11RFEntry.LOFDM_pwd_A);
	FPRINTF_INT("LOFDM_pwd_B",			pmib->dot11RFEntry.LOFDM_pwd_B);
	FPRINTF_INT("tssi1",				pmib->dot11RFEntry.tssi1);
	FPRINTF_INT("tssi2",				pmib->dot11RFEntry.tssi2);
	FPRINTF_INT("ther",					pmib->dot11RFEntry.ther);
  #else 
	FPRINTF_INT("MIMO_TR_mode",			pmib->dot11RFEntry.MIMO_TR_mode);
  	fprintf_BArray(fp,"pwrlevelCCK_A",ifname,pmib->dot11RFEntry.pwrlevelCCK_A,MAX_2G_CHANNEL_NUM_MIB);
	fprintf_BArray(fp,"pwrlevelCCK_B",ifname,pmib->dot11RFEntry.pwrlevelCCK_B,MAX_2G_CHANNEL_NUM_MIB);
	fprintf_BArray(fp,"pwrlevelHT40_1S_A",ifname,pmib->dot11RFEntry.pwrlevelHT40_1S_A,MAX_2G_CHANNEL_NUM_MIB);
	fprintf_BArray(fp,"pwrlevelHT40_1S_B",ifname,pmib->dot11RFEntry.pwrlevelHT40_1S_B,MAX_2G_CHANNEL_NUM_MIB);
	fprintf_BArray(fp,"pwrdiffHT40_2S",ifname,pmib->dot11RFEntry.pwrdiffHT40_2S,MAX_2G_CHANNEL_NUM_MIB);
	fprintf_BArray(fp,"pwrdiffHT20",ifname,pmib->dot11RFEntry.pwrdiffHT20,MAX_2G_CHANNEL_NUM_MIB);
	fprintf_BArray(fp,"pwrdiffOFDM",ifname,pmib->dot11RFEntry.pwrdiffOFDM,MAX_2G_CHANNEL_NUM_MIB);
	FPRINTF_INT("tssi1",				pmib->dot11RFEntry.tssi1);
	FPRINTF_INT("tssi2",				pmib->dot11RFEntry.tssi2);
	FPRINTF_INT("ther",					pmib->dot11RFEntry.ther);
  #endif //!defined(CONFIG_RTL8196C)
#else
	//!CONFIG_RTL8196B => rtl8651c+rtl8190
	FPRINTF_INT("dot11DiversitySupport",pmib->dot11RFEntry.dot11DiversitySupport);
	FPRINTF_INT("defaultAntennaB",		pmib->dot11RFEntry.defaultAntennaB);
	fprintf_BArray(fp,"TxPowerCCK",ifname,pmib->dot11RFEntry.pwrlevelCCK,14);
	fprintf_BArray(fp,"TxPowerOFDM",ifname,pmib->dot11RFEntry.pwrlevelOFDM,162);
	FPRINTF_INT("LOFDM_pwrdiff",	pmib->dot11RFEntry.legacyOFDM_pwrdiff);
	FPRINTF_INT("antC_pwrdiff",			pmib->dot11RFEntry.antC_pwrdiff);
	FPRINTF_INT("ther_rfic",			pmib->dot11RFEntry.ther_rfic);
	FPRINTF_INT("crystalCap",			pmib->dot11RFEntry.crystalCap);
#endif //defined(CONFIG_RTL8196B)

	FPRINTF_INT("bcnint",				pmib->dot11StationConfigEntry.dot11BeaconPeriod);
	FPRINTF_INT("channel",				pmib->dot11RFEntry.dot11channel);
	FPRINTF_INT("rtsthres",				pmib->dot11OperationEntry.dot11RTSThreshold);
	FPRINTF_INT("fragthres",			pmib->dot11OperationEntry.dot11FragmentationThreshold);
	FPRINTF_INT("expired_time",			pmib->dot11OperationEntry.expiretime);
	FPRINTF_INT("preamble",				pmib->dot11RFEntry.shortpreamble);
	FPRINTF_INT("dtimperiod",			pmib->dot11StationConfigEntry.dot11DTIMPeriod);
	FPRINTF_INT("iapp_enable",			pmib->dot11OperationEntry.iapp_enable);
	FPRINTF_INT("disable_protection",	pmib->dot11StationConfigEntry.protectionDisabled);
	FPRINTF_INT("block_relay",			pmib->dot11OperationEntry.block_relay);
	FPRINTF_INT("wifi_specific",		pmib->dot11OperationEntry.wifi_specific);
	FPRINTF_INT("wds_num",				0);
	FPRINTF_INT("wds_enable",			pmib->dot11WdsInfo.wdsEnabled);
	FPRINTF_INT("wds_encrypt",			pmib->dot11WdsInfo.wdsPrivacy);
	for (i=0; i<pmib->dot11WdsInfo.wdsNum; i++){
		struct wdsEntry *wds_Entry;
		wds_Entry = &(pmib->dot11WdsInfo.entry[i]);
		fprintf(fp, "%s_wds_add=%02x%02x%02x%02x%02x%02x,%d\n",ifname,wds_Entry->macAddr[0],wds_Entry->macAddr[1],wds_Entry->macAddr[2],
										wds_Entry->macAddr[3], wds_Entry->macAddr[4], wds_Entry->macAddr[5], wds_Entry->txRate);
		
	}

	memset(buf1, 0, 1024);
	apmib_get(MIB_WLAN_WDS_WEP_KEY, (void *)buf1);
	FPRINTF_STR("wds_wepkey",			(char *)buf1);
	//FPRINTF_INT("aclnum",				pmib->dot11StationConfigEntry.dot11AclNum);
	FPRINTF_INT("aclnum",				0);
	FPRINTF_INT("aclmode",				pmib->dot11StationConfigEntry.dot11AclMode);
	for (i=0; i<pmib->dot11StationConfigEntry.dot11AclNum; i++)
		fprintf_BArray(fp,"acladdr",ifname,pmib->dot11StationConfigEntry.dot11AclAddr[i],6);
	
#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_) // below code copy above ACL code
	//FPRINTF_INT("meshaclnum",			pmib->dot1180211sInfo.mesh_acl_num);
	//fprintf_BArray(fp,"meshacladdr",ifname,pmib->dot1180211sInfo.mesh_acl_addr[0],
	//				pmib->dot1180211sInfo.mesh_acl_num*6);
	for (i=0; i<pmib->dot1180211sInfo.mesh_acl_num; i++)
		fprintf_BArray(fp,"meshacladdr",ifname,pmib->dot1180211sInfo.mesh_acl_addr[i],6);
#endif

	FPRINTF_INT("nat25_disable",		pmib->ethBrExtInfo.nat25_disable);
	FPRINTF_INT("macclone_enable",		pmib->ethBrExtInfo.macclone_enable);
#ifdef WIFI_SIMPLE_CONFIG
	FPRINTF_INT("wsc_enable",			pmib->wscEntry.wsc_enable);
#endif
	FPRINTF_INT("use40M",				pmib->dot11nConfigEntry.dot11nUse40M);
	FPRINTF_INT("2ndchoffset",			pmib->dot11nConfigEntry.dot11n2ndChOffset);
	FPRINTF_INT("shortGI20M",			pmib->dot11nConfigEntry.dot11nShortGIfor20M);
	FPRINTF_INT("shortGI40M",			pmib->dot11nConfigEntry.dot11nShortGIfor40M);
	FPRINTF_INT("stbc",					pmib->dot11nConfigEntry.dot11nSTBC);
	FPRINTF_INT("coexist",				pmib->dot11nConfigEntry.dot11nCoexist);
	FPRINTF_INT("ampdu",				pmib->dot11nConfigEntry.dot11nAMPDU);
	FPRINTF_INT("amsdu",				pmib->dot11nConfigEntry.dot11nAMSDU);
#if defined(CONFIG_RTL_819X) && defined(MBSSID)
	FPRINTF_INT("vap_enable",			pmib->miscEntry.vap_enable);
#endif
	FPRINTF_INT("basicrates",			pmib->dot11StationConfigEntry.dot11BasicRates);
	FPRINTF_INT("oprates",				pmib->dot11StationConfigEntry.dot11SupportedRates);
	FPRINTF_INT("fixrate",				pmib->dot11StationConfigEntry.fixedTxRate);
	FPRINTF_INT("autorate",				pmib->dot11StationConfigEntry.autoRate);
	FPRINTF_INT("hiddenAP",				pmib->dot11OperationEntry.hiddenAP);
	FPRINTF_INT("deny_legacy",			pmib->dot11StationConfigEntry.legacySTADeny);
	FPRINTF_INT("band",					pmib->dot11BssType.net_work_type);
	FPRINTF_INT("guest_access",			pmib->dot11OperationEntry.guest_access);
	FPRINTF_INT("qos_enable",			pmib->dot11QosEntry.dot11QosEnable);	
#ifdef CONFIG_RTL_WAPI_SUPPORT
	FPRINTF_INT("wapiType",				pmib->wapiInfo.wapiType);
#endif
	FPRINTF_INT("authtype",				pmib->dot1180211AuthEntry.dot11AuthAlgrthm);
	FPRINTF_INT("encmode",				pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm);
	switch (pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm) {
		case 1:
			keylength = 5;
			break;
		case 5:
			keylength = 13;
			break;
		default:
			keylength = 0;
	};
	fprintf_BArray(fp,"wepkey1",ifname,(unsigned char *)&pmib->dot11DefaultKeysTable.keytype[0], keylength);
	fprintf_BArray(fp,"wepkey2",ifname,(unsigned char *)&pmib->dot11DefaultKeysTable.keytype[1], keylength);
	fprintf_BArray(fp,"wepkey3",ifname,(unsigned char *)&pmib->dot11DefaultKeysTable.keytype[2], keylength);
	fprintf_BArray(fp,"wepkey4",ifname,(unsigned char *)&pmib->dot11DefaultKeysTable.keytype[3], keylength);
	FPRINTF_INT("wepdkeyid",			pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex);
#ifndef CONFIG_RTL8196B_TLD
#ifdef MBSSID
	FPRINTF_INT("block_relay",			pmib->dot11OperationEntry.block_relay);
#endif
#endif
	FPRINTF_INT("802_1x",				pmib->dot118021xAuthEntry.dot118021xAlgrthm);	
#ifdef CONFIG_RTL_WAPI_SUPPORT
	FPRINTF_INT("wapiType",				pmib->wapiInfo.wapiType);
	FPRINTF_INT("wapiMCastKeyPktNum",	pmib->wapiInfo.wapiUpdateMCastKeyPktNum);
	FPRINTF_INT("wapiMCastKeyType",		pmib->wapiInfo.wapiUpdateMCastKeyType);
	FPRINTF_INT("wapiMCastKeyTimeout",	pmib->wapiInfo.wapiUpdateMCastKeyTimeout);
	FPRINTF_INT("wapiUCastKeyPktNum",	pmib->wapiInfo.wapiUpdateUCastKeyPktNum);
	FPRINTF_INT("wapiUCastKeyType",		pmib->wapiInfo.wapiUpdateUCastKeyType);
	FPRINTF_INT("wapiUCastKeyTimeout",	pmib->wapiInfo.wapiUpdateUCastKeyTimeout);
	FPRINTF_INT("wapiPsklen",			pmib->wapiInfo.wapiPsk.len);
	memset(buf1, 0, 1024);
	apmib_get(MIB_WLAN_WAPI_PSK, (void *)buf1);
	FPRINTF_STR("wapiPsk",				(char *)buf1);
#endif

#ifdef CONFIG_RTK_MESH
#ifdef CONFIG_NEW_MESH_UI
	FPRINTF_INT("meshSilence",			pmib->dot1180211sInfo.meshSilence);
	FPRINTF_INT("mesh_enable",			pmib->dot1180211sInfo.mesh_enable);
	FPRINTF_INT("mesh_ap_enable",		pmib->dot1180211sInfo.mesh_ap_enable);
	FPRINTF_INT("mesh_portal_enable",	pmib->dot1180211sInfo.mesh_portal_enable);
	FPRINTF_INT("mesh_root_enable",		pmib->dot1180211sInfo.mesh_root_enable);
	FPRINTF_INT("mesh_max_neightbor",	pmib->dot1180211sInfo.mesh_max_neightbor);
	FPRINTF_INT("log_enabled",			pmib->dot1180211sInfo.log_enabled);
	FPRINTF_STR("mesh_id",				pmib->dot1180211sInfo.mesh_id);
#endif 
#endif

	FPRINTF_INT("psk_enable",			pmib->dot1180211AuthEntry.dot11EnablePSK);
	FPRINTF_INT("wpa_cipher",			pmib->dot1180211AuthEntry.dot11WPACipher);
	FPRINTF_INT("wpa2_cipher",			pmib->dot1180211AuthEntry.dot11WPA2Cipher);
	FPRINTF_STR("passphrase",			pmib->dot1180211AuthEntry.dot11PassPhrase);
	FPRINTF_LONG("gk_rekey",			pmib->dot1180211AuthEntry.dot11GKRekeyTime);
	/* wps relative config */
	if( strcmp(ifname,"wlan0") )
	   return 0;

	apmib_get(MIB_WLAN_MODE, (void *)&is_client);
	apmib_get(MIB_WLAN_WSC_CONFIGURED, (void *)&is_config);
	apmib_get(MIB_WLAN_WSC_REGISTRAR_ENABLED, (void *)&is_registrar);
	if (is_client == CLIENT_MODE) {
#ifdef CONFIG_RTL8186_KLD_REPEATER
		if (is_repeater_enabled && is_config) {
			intVal = MODE_AP_PROXY_REGISTRAR;
			wps_vxdAP_enabled = 1;
			WRITE_WSC_PARAM(ptr, tmpbuf, "disable_configured_by_exReg = %d\n", 1);
		}
		else
#endif
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
	fprintf(fp, "wps_mode=%d\n", intVal);
	apmib_get(MIB_WLAN_WSC_UPNP_ENABLED, (void *)&intVal);
	FPRINTF_INT("wps_upnp_enable",		intVal);
	apmib_get(MIB_WLAN_WSC_METHOD, (void *)&intVal);
	//Ethernet(0x2)+Label(0x4)+PushButton(0x80) Bitwise OR
	if (intVal == 1) //Pin+Ethernet
		intVal = (CONFIG_METHOD_ETH | CONFIG_METHOD_PIN);
	else if (intVal == 2) //PBC+Ethernet
		intVal = (CONFIG_METHOD_ETH | CONFIG_METHOD_PBC);
	if (intVal == 3) //Pin+PBC+Ethernet
		intVal = (CONFIG_METHOD_ETH | CONFIG_METHOD_PIN | CONFIG_METHOD_PBC);
	fprintf(fp, "wps_config_method=%d\n",intVal);
	apmib_get(MIB_WLAN_WSC_AUTH, (void *)&intVal);
	FPRINTF_INT("wps_auth",				intVal);
	apmib_get(MIB_WLAN_WSC_ENC, (void *)&intVal);
	FPRINTF_INT("wps_enc",				intVal);
	apmib_get(MIB_WLAN_WPA_PSK, (void *)buf1);
	apmib_get(MIB_WLAN_WEP_KEY_TYPE, (void *)&wep_key_type);
	if( intVal == 2 ){
        	int x=0, wep_keylen;
	        char key[32];
	
		apmib_get(MIB_WLAN_WEP,(void *)&wep_keylen);

    	if( pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex == 0 ) {
            fprintf(fp,"wps_wep_transmit_key=%d\n",		1);
            if( wep_key_type == KEY_ASCII ){
		if( wep_keylen == WEP64 )
                apmib_get(MIB_WLAN_WEP64_KEY1, (void *)key);
		else
		apmib_get(MIB_WLAN_WEP128_KEY1, (void *)key);
		} else {
                convert_bin_to_str(pmib->dot11DefaultKeysTable.keytype,keylength,key);
		}
            buf1[keylength] = '\0';
    		FPRINTF_STR("network_key",(char *)key);
    	} else {
            fprintf(fp,"wps_wep_transmit_key=%d\n",		pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex);
	        for( x=0; x<4; x++ ){
	            if( x == (pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex-1) ){
                    if( wep_key_type == KEY_ASCII ) {
			if( wep_keylen == WEP64 )
                		apmib_get(MIB_WLAN_WEP64_KEY1, (void *)key);
	                else
		                apmib_get(MIB_WLAN_WEP128_KEY1, (void *)key);
                	} else {
		                convert_bin_to_str(&pmib->dot11DefaultKeysTable.keytype[x],keylength,key);
                	}
                    buf1[keylength] = '\0';
	                FPRINTF_STR("network_key",key);
	            } else {
	                unsigned char line[100];
                    if( wep_key_type == KEY_ASCII ){
			if( wep_keylen == WEP64 )
                                apmib_get(MIB_WLAN_WEP64_KEY1, (void *)key);
                        else
                                apmib_get(MIB_WLAN_WEP128_KEY1, (void *)key);
                        } else {
                                convert_bin_to_str(&pmib->dot11DefaultKeysTable.keytype[x],keylength,key);
                        }
                    buf1[keylength] = '\0';
                    sprintf(line,"wep_key%d=%s",x+1,(char *)buf1);
	                fprintf(fp,"%s_%s",ifname,line);
        	    }
            }
        }
    } else if( intVal > 2 && intVal < 13 ) {
        FPRINTF_STR("network_key",			buf1);
    } else {
        fprintf(fp,"#");
        FPRINTF_STR("network_key",			buf1);
    }
    if (is_client) {
#ifdef CONFIG_RTL8186_KLD_REPEATER
		if (wps_vxdAP_enabled)
			intVal = 1;
		else
#endif
		{
			apmib_get(MIB_WLAN_NETWORK_TYPE, (void *)&intVal);
			if (intVal == 0)
				intVal = 1;
			else
				intVal = 2;
		}
	}
	else
		intVal = 1;    
    fprintf(fp, "wps_connection_type=%d\n", intVal);
    apmib_get(MIB_WLAN_WSC_MANUAL_ENABLED, (void *)&intVal);
    fprintf(fp, "wps_manual_enabled=%d\n",   !intVal);
    apmib_get(MIB_HW_WSC_PIN, (void *)&buf1);
    fprintf(fp, "wps_pin_code=%s\n", buf1);
    apmib_get(MIB_WLAN_CHANNEL, (void *)&intVal);
	if (intVal > 14)
		intVal = 2;
	else
		intVal = 1;
	fprintf(fp, "wps_rf_band=%d\n", intVal);
    fprintf(fp, "wps_use_ie=%d\n", 1);
    fprintf(fp, "wps_auth_type_flags=%d\n", 39);
    fprintf(fp, "wps_encrypt_type_flags=%d\n", 15);
    fprintf(fp, "wps_uuid=63041253101920061228%02x%02x%02x%02x%02x%02x\n",pmib->dot11OperationEntry.hwaddr[0],pmib->dot11OperationEntry.hwaddr[1],pmib->dot11OperationEntry.hwaddr[2],pmib->dot11OperationEntry.hwaddr[3],pmib->dot11OperationEntry.hwaddr[4],pmib->dot11OperationEntry.hwaddr[5]);
    fprintf(fp, "wps_device_name=%s\n", "\"Realtek Wireless AP\"");
    fprintf(fp, "wps_manufacturer=%s\n", "\"Realtek Semiconductor Corp.\"");
    fprintf(fp, "wps_manufacturerURL=%s\n", "\"http:\/\/www.realtek.com\/\"");
    fprintf(fp, "wps_modelURL=%s\n", "\"http:\/\/www.realtek.com\/\"");
    fprintf(fp, "wps_model_name=%s\n", "\"RTL8xxx\"");
    fprintf(fp, "wps_model_num=%s\n", "\"EV-2009-02-06\"");
    fprintf(fp, "wps_serial_num=%s\n", "\"123456789012347\"");
    fprintf(fp, "wps_modelDescription=%s\n", "\"WLAN Access Point\"");
    fprintf(fp, "wps_device_attrib_id=%d\n", 1);
    fprintf(fp, "wps_device_oui=%s\n", "0050f204");
    fprintf(fp, "wps_device_category_id=%d\n", 6);
    fprintf(fp, "wps_device_sub_category_id=%d\n", 1);
    fprintf(fp, "wps_device_password_id=%d\n", 0);
    fprintf(fp, "wps_resent_limit=%d\n", 2);
    fprintf(fp, "wps_tx_timeout=%d\n", 5);
    fprintf(fp, "wps_reg_timeout=%d\n", 120);
    fprintf(fp, "wps_block_timeout=%d\n", 60);
    fprintf(fp, "WPS_START_LED_GPIO_number=%d\n", 2);
    fprintf(fp, "WPS_END_LED_unconfig_GPIO_number=%d\n", 0);
    fprintf(fp, "WPS_END_LED_config_GPIO_number=%d\n", 0);
    fprintf(fp, "WPS_PBC_overlapping_GPIO_number=%d\n", 1);
    fprintf(fp, "PBC_overlapping_LED_time_out=%d\n", 30);
    fprintf(fp, "No_ifname_for_flash_set=%d\n", 0);
    /*
    fprintf(fp, "wps_disable_disconnect=%s\n", "1");
    fprintf(fp, "wps_disable_auto_gen_ssid=%s\n", "1");
    fprintf(fp, "wps_manual_key_type=%s\n", "3");
    fprintf(fp, "wps_manual_key=%s\n", "1234567890");
    */
    fprintf(fp, "wps_disable_hidden_ap=%d\n", 1);
    fprintf(fp, "wps_button_hold_time=%d\n", 3);
	fclose(fp);
	return 0;

}

/* Macro definition */
static int _is_hex(char c)
{
    return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||
            ((c >= 'a') && (c <= 'f')));
}

static int string_to_hex(char *string, unsigned char *key, int len)
{
	char tmpBuf[4];
	int idx, ii=0;
	for (idx=0; idx<len; idx+=2) {
		tmpBuf[0] = string[idx];
		tmpBuf[1] = string[idx+1];
		tmpBuf[2] = 0;
		if ( !_is_hex(tmpBuf[0]) || !_is_hex(tmpBuf[1]))
			return 0;

		key[ii++] = (unsigned char) strtol(tmpBuf, (char**)NULL, 16);
	}
	return 1;
}

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/wireless.h>
int comapi_initWlan(char *ifname)
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
#ifdef CONFIG_RTL_819X
	int vap_enable=0, intVal4=0;
#endif
#endif
	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(wrq.ifr_name, ifname, IFNAMSIZ);

	if ((pmib = (struct wifi_mib *)malloc(sizeof(struct wifi_mib))) == NULL) {
		printf("MIB buffer allocation failed!\n");
		return -1;
	}

	if (!apmib_init()) {
		printf("Initialize AP MIB failed!\n");
		return -1;
	}

	// Disable WLAN MAC driver and shutdown interface first
	sprintf(buf1, "ifconfig %s down", ifname);
	system(buf1);

	if (vwlan_idx == 0) {
		// shutdown all WDS interface
		for (i=0; i<8; i++) {
			sprintf(buf1, "ifconfig %s-wds%d down", ifname, i);
			system(buf1);
		}
	
		// kill wlan application daemon
		sprintf(buf1, "wlanapp.sh kill %s", ifname);
		system(buf1);
	}
	else { // virtual interface
		sprintf(buf1, "wlan%d", wlan_idx);		
		strncpy(wrq_root.ifr_name, buf1, IFNAMSIZ);
		if (ioctl(skfd, SIOCGIWNAME, &wrq_root) < 0) {
			printf("Root Interface [%s] open failed!\n",buf1);
			return -1;
		}		
	}

	apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&intVal);

	if (intVal == 1) {
		free(pmib);
		close(skfd);		
		return 0;
	}

	// get mib from driver
	wrq.u.data.pointer = (caddr_t)pmib;
	wrq.u.data.length = sizeof(struct wifi_mib);

	if (vwlan_idx == 0) {
		if (ioctl(skfd, 0x8B42, &wrq) < 0) {
			printf("Get WLAN MIB failed!\n");
			return -1;
		}
	}
	else {
		system("iwpriv wlan0 cfgfile");
		
		wrq_root.u.data.pointer = (caddr_t)pmib;
		wrq_root.u.data.length = sizeof(struct wifi_mib);				
		if (ioctl(skfd, 0x8B42, &wrq_root) < 0) {
			printf("Get WLAN MIB failed!\n");
			return -1;
		}		
	}
	// Set parameters to driver
	if (vwlan_idx == 0) {	
		apmib_get(MIB_HW_REG_DOMAIN, (void *)&intVal);
		pmib->dot11StationConfigEntry.dot11RegDomain = intVal;
	}

	apmib_get(MIB_WLAN_WLAN_MAC_ADDR, (void *)mac);
	if (!memcmp(mac, "\x00\x00\x00\x00\x00\x00", 6)) {
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
	}

	// ifconfig all wlan interface when not in WISP
	// ifconfig wlan1 later interface when in WISP mode, the wlan0	will be setup in WAN interface
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

#ifdef BR_SHORTCUT	
	if (intVal == 2
#ifdef MBSSID
		&& vwlan_idx == 0
#endif
	) 
		pmib->dot11OperationEntry.disable_brsc = 1;
#endif
	
	apmib_get(MIB_HW_LED_TYPE, (void *)&intVal);
	pmib->dot11OperationEntry.ledtype = intVal;

	// set AP/client/WDS mode
	apmib_get(MIB_WLAN_SSID, (void *)buf1);
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

#if defined(CONFIG_RTL_92D_SUPPORT)
		apmib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&intVal);
		pmib->dot11RFEntry.phyBandSelect = intVal;
		apmib_get(MIB_WLAN_MAC_PHY_MODE, (void *)&intVal);
		pmib->dot11RFEntry.macPhyMode = intVal;
#endif
	apmib_get(MIB_WLAN_MODE, (void *)&mode);
	if (mode == 1) {
		// client mode
		apmib_get(MIB_WLAN_NETWORK_TYPE, (void *)&intVal2);
		if (intVal2 == 0)
			pmib->dot11OperationEntry.opmode = 8;
		else {
			pmib->dot11OperationEntry.opmode = 32;
			apmib_get(MIB_WLAN_DEFAULT_SSID, (void *)buf1);
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

	if (vwlan_idx == 0) { // root interface 
		// set RF parameters
		apmib_get(MIB_HW_RF_TYPE, (void *)&intVal);
		pmib->dot11RFEntry.dot11RFType = intVal;
		
#if defined(CONFIG_RTL_819X)
#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
		apmib_get(MIB_HW_BOARD_VER, (void *)&intVal);
	if (intVal == 1)
		pmib->dot11RFEntry.MIMO_TR_mode = 3;	// 2T2R
	else if(intVal == 2)
		pmib->dot11RFEntry.MIMO_TR_mode = 4; // 1T1R
	else
		pmib->dot11RFEntry.MIMO_TR_mode = 1;	// 1T2R
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
	
	apmib_get(MIB_HW_11N_TSSI1, (void *)&intVal);
	pmib->dot11RFEntry.tssi1 = intVal;

	apmib_get(MIB_HW_11N_TSSI2, (void *)&intVal);
	pmib->dot11RFEntry.tssi2 = intVal;

	apmib_get(MIB_HW_11N_THER, (void *)&intVal);
	pmib->dot11RFEntry.ther = intVal;
	
/**************** ToDo**************/
/*		
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
			}
		}
	}	
*/	
#elif defined(CONFIG_RTL_8196C)
		apmib_get(MIB_HW_BOARD_VER, (void *)&intVal);
	if (intVal == 1)
		pmib->dot11RFEntry.MIMO_TR_mode = 3;	// 2T2R
	else if(intVal == 2)
		pmib->dot11RFEntry.MIMO_TR_mode = 4; // 1T1R
	else
		pmib->dot11RFEntry.MIMO_TR_mode = 1;	// 1T2R
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
	
	apmib_get(MIB_HW_11N_TSSI1, (void *)&intVal);
	pmib->dot11RFEntry.tssi1 = intVal;

	apmib_get(MIB_HW_11N_TSSI2, (void *)&intVal);
	pmib->dot11RFEntry.tssi2 = intVal;

	apmib_get(MIB_HW_11N_THER, (void *)&intVal);
	pmib->dot11RFEntry.ther = intVal;
	
/**************** ToDo**************/
/*		
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
			}
		}
	}	
*/	
#else
		apmib_get(MIB_HW_BOARD_VER, (void *)&intVal);
		if (intVal == 1)
			pmib->dot11RFEntry.MIMO_TR_mode = 3;	// 2T2R
		else if(intVal == 2)
						pmib->dot11RFEntry.MIMO_TR_mode = 4; // 1T1R
		else
			pmib->dot11RFEntry.MIMO_TR_mode = 1;	// 1T2R

		apmib_get(MIB_HW_TX_POWER_CCK, (void *)buf1);
		memcpy(pmib->dot11RFEntry.pwrlevelCCK, buf1, 14);
		
		apmib_get(MIB_HW_TX_POWER_OFDM_1S, (void *)buf1);
		memcpy(pmib->dot11RFEntry.pwrlevelOFDM_1SS, buf1, 162);
		
		apmib_get(MIB_HW_TX_POWER_OFDM_2S, (void *)buf1);
		memcpy(pmib->dot11RFEntry.pwrlevelOFDM_2SS, buf1, 162);

		// not used for RTL8192SE
		//apmib_get(MIB_HW_11N_XCAP, (void *)&intVal);
		//pmib->dot11RFEntry.crystalCap = intVal;
		
		apmib_get(MIB_HW_11N_LOFDMPWDA, (void *)&intVal);
		pmib->dot11RFEntry.LOFDM_pwd_A = intVal;

		apmib_get(MIB_HW_11N_LOFDMPWDB, (void *)&intVal);
		pmib->dot11RFEntry.LOFDM_pwd_B = intVal;

		apmib_get(MIB_HW_11N_TSSI1, (void *)&intVal);
		pmib->dot11RFEntry.tssi1 = intVal;

		apmib_get(MIB_HW_11N_TSSI2, (void *)&intVal);
		pmib->dot11RFEntry.tssi2 = intVal;

		apmib_get(MIB_HW_11N_THER, (void *)&intVal);
		pmib->dot11RFEntry.ther = intVal;

		if (pmib->dot11RFEntry.dot11RFType == 10) { // Zebra
			apmib_get(MIB_WLAN_RFPOWER_SCALE, (void *)&intVal);
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
					if (pmib->dot11RFEntry.pwrlevelOFDM_1SS[i] != 0){
						if((pmib->dot11RFEntry.pwrlevelOFDM_1SS[i] - intVal) >= 1)
							pmib->dot11RFEntry.pwrlevelOFDM_1SS[i] -= intVal;
						else
							pmib->dot11RFEntry.pwrlevelOFDM_1SS[i] = 1;
					}
					if (pmib->dot11RFEntry.pwrlevelOFDM_2SS[i] != 0){
						if((pmib->dot11RFEntry.pwrlevelOFDM_2SS[i] - intVal) >= 1)
							pmib->dot11RFEntry.pwrlevelOFDM_2SS[i] -= intVal;
						else
							pmib->dot11RFEntry.pwrlevelOFDM_2SS[i] = 1;
					}
				}		
			}
		}
	
#endif		
#else
//!CONFIG_RTL8196B => rtl8651c+rtl8190
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
			apmib_get(MIB_WLAN_RFPOWER_SCALE, (void *)&intVal);
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
#endif  //For Check CONFIG_RTL8196B
		
		apmib_get(MIB_WLAN_BEACON_INTERVAL, (void *)&intVal);
		pmib->dot11StationConfigEntry.dot11BeaconPeriod = intVal;

		apmib_get(MIB_WLAN_CHANNEL, (void *)&intVal);
		pmib->dot11RFEntry.dot11channel = intVal;

		apmib_get(MIB_WLAN_RTS_THRESHOLD, (void *)&intVal);
		pmib->dot11OperationEntry.dot11RTSThreshold = intVal;

		apmib_get(MIB_WLAN_FRAG_THRESHOLD, (void *)&intVal);
		pmib->dot11OperationEntry.dot11FragmentationThreshold = intVal;

		apmib_get(MIB_WLAN_INACTIVITY_TIME, (void *)&intVal);
		pmib->dot11OperationEntry.expiretime = intVal;

		apmib_get(MIB_WLAN_PREAMBLE_TYPE, (void *)&intVal);
		pmib->dot11RFEntry.shortpreamble = intVal;

		apmib_get(MIB_WLAN_DTIM_PERIOD, (void *)&intVal);
		pmib->dot11StationConfigEntry.dot11DTIMPeriod = intVal;

		// enable/disable the notification for IAPP
		apmib_get(MIB_WLAN_IAPP_DISABLED, (void *)&intVal);
		if (intVal == 0)
			pmib->dot11OperationEntry.iapp_enable = 1;
		else
			pmib->dot11OperationEntry.iapp_enable = 0;

		// set 11g protection mode
		apmib_get(MIB_WLAN_PROTECTION_DISABLED, (void *)&intVal);
		pmib->dot11StationConfigEntry.protectionDisabled = intVal;

		// set block relay
		apmib_get(MIB_WLAN_BLOCK_RELAY, (void *)&intVal);
		pmib->dot11OperationEntry.block_relay = intVal;

		// set WiFi specific mode
		apmib_get(MIB_WIFI_SPECIFIC, (void *)&intVal);
		pmib->dot11OperationEntry.wifi_specific = intVal;

		// Set WDS
		apmib_get(MIB_WLAN_WDS_ENABLED, (void *)&intVal);
		apmib_get(MIB_WLAN_WDS_NUM, (void *)&intVal2);
		pmib->dot11WdsInfo.wdsNum = 0;
#ifdef MBSSID 
		if (v_previous > 0) 
			intVal = 0;
#endif
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

		// enable/disable the notification for IAPP
		apmib_get(MIB_WLAN_IAPP_DISABLED, (void *)&intVal);
		if (intVal == 0)
			pmib->dot11OperationEntry.iapp_enable = 1;
		else
			pmib->dot11OperationEntry.iapp_enable = 0;

		pmib->dot11StationConfigEntry.dot11AclNum = 0;
		apmib_get(MIB_WLAN_MACAC_ENABLED, (void *)&intVal);
		pmib->dot11StationConfigEntry.dot11AclMode = intVal;
		if (intVal != 0) {
			apmib_get(MIB_WLAN_MACAC_NUM, (void *)&intVal);
			if (intVal != 0) {
				for (i=0; i<intVal; i++) {
					buf1[0] = i+1;
					apmib_get(MIB_WLAN_MACAC_ADDR, (void *)buf1);
					pAcl = (MACFILTER_T *)buf1;
					memcpy(&(pmib->dot11StationConfigEntry.dot11AclAddr[i][0]), &(pAcl->macAddr[0]), 6);
					pmib->dot11StationConfigEntry.dot11AclNum++;
				}
			}
		}

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
		apmib_get(MIB_WLAN_MACCLONE_ENABLED, (void *)&intVal);
		if ((intVal == 1) && (mode == 1)) {
			pmib->ethBrExtInfo.nat25_disable = 1;
			pmib->ethBrExtInfo.macclone_enable = 1;
		}
		else {
			pmib->ethBrExtInfo.nat25_disable = 0;
			pmib->ethBrExtInfo.macclone_enable = 0;
		}		

		// set nat2.5 disable and macclone disable when wireless isp mode
		apmib_get(MIB_OP_MODE, (void *)&intVal);
		if (intVal == 2) {
			pmib->ethBrExtInfo.nat25_disable = 0;	// enable nat25 for ipv6-passthru ping6 fail issue at wisp mode && wlan client mode .
			pmib->ethBrExtInfo.macclone_enable = 0;
		}

#ifdef WIFI_SIMPLE_CONFIG
		pmib->wscEntry.wsc_enable = 0;
#endif

	// for 11n
		apmib_get(MIB_WLAN_CHANNEL_BONDING, &channel_bound);
		pmib->dot11nConfigEntry.dot11nUse40M = channel_bound;
		apmib_get(MIB_WLAN_CONTROL_SIDEBAND, &intVal);
		if(channel_bound ==0){
			pmib->dot11nConfigEntry.dot11n2ndChOffset = 0;
		}else {
			if(intVal == 0 )
				pmib->dot11nConfigEntry.dot11n2ndChOffset = 1;
			if(intVal == 1 )
				pmib->dot11nConfigEntry.dot11n2ndChOffset = 2;	
		}
		apmib_get(MIB_WLAN_SHORT_GI, &intVal);
		pmib->dot11nConfigEntry.dot11nShortGIfor20M = intVal;
		pmib->dot11nConfigEntry.dot11nShortGIfor40M = intVal;
		
		apmib_get(MIB_WLAN_STBC_ENABLED, &intVal);
		pmib->dot11nConfigEntry.dot11nSTBC = intVal;
		apmib_get(MIB_WLAN_COEXIST_ENABLED, &intVal);
		pmib->dot11nConfigEntry.dot11nCoexist = intVal;
		
		apmib_get(MIB_WLAN_AGGREGATION, &aggregation);
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

#if defined(CONFIG_RTL_819X) && defined(MBSSID)
		if(pmib->dot11OperationEntry.opmode & 0x00000010){// AP mode
			for (vwlan_idx = 1; vwlan_idx < 5; vwlan_idx++) {
				apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&intVal4);
				if (intVal4 == 0)
					vap_enable++;
				intVal4=0;
			}
			vwlan_idx = 0;
		}
		if (vap_enable && (mode ==	AP_MODE || mode ==	AP_WDS_MODE))	
			pmib->miscEntry.vap_enable=1;
		else
			pmib->miscEntry.vap_enable=0;
#endif
	}

	if (vwlan_idx != NUM_VWLAN_INTERFACE) { // not repeater interface
		apmib_get(MIB_WLAN_BASIC_RATES, (void *)&intVal);
		pmib->dot11StationConfigEntry.dot11BasicRates = intVal;

		apmib_get(MIB_WLAN_SUPPORTED_RATES, (void *)&intVal);
		pmib->dot11StationConfigEntry.dot11SupportedRates = intVal;

		apmib_get(MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&intVal);
		if (intVal == 0) {
			pmib->dot11StationConfigEntry.autoRate = 0;
			apmib_get(MIB_WLAN_FIX_RATE, (void *)&intVal);
			pmib->dot11StationConfigEntry.fixedTxRate = intVal;
		}
		else
			pmib->dot11StationConfigEntry.autoRate = 1;

		apmib_get(MIB_WLAN_HIDDEN_SSID, (void *)&intVal);
		pmib->dot11OperationEntry.hiddenAP = intVal;

	// set band
		apmib_get(MIB_WLAN_BAND, (void *)&intVal);
		wlan_band = intVal;
		if ((mode != 1) && (pmib->dot11OperationEntry.wifi_specific == 1) && (wlan_band == 2))
			wlan_band = 3;

		if (wlan_band == 8) { // pure-11n
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
		apmib_get(MIB_WLAN_ACCESS, (void *)&intVal);
		pmib->dot11OperationEntry.guest_access = intVal;

		// set WMM
		apmib_get(MIB_WLAN_WMM_ENABLED, (void *)&intVal);
		pmib->dot11QosEntry.dot11QosEnable = intVal;		
	}

	apmib_get(MIB_WLAN_AUTH_TYPE, (void *)&intVal);
	apmib_get(MIB_WLAN_ENCRYPT, (void *)&encrypt);
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
		apmib_get(MIB_WLAN_WEP, (void *)&wep);
		if (wep == 1) {
			pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 1;
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
		}
		else {
			pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 5;
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
		}
	}
#ifdef CONFIG_RTL_WAPI_SUPPORT	
	else if(7 == encrypt)
	{
		pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 7;
		pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 0;
	}
#endif	
	else {
		// WPA mode
		pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 2;
	}

#ifndef CONFIG_RTL8196B_TLD
#ifdef MBSSID
	if (vwlan_idx > 0 && pmib->dot11OperationEntry.guest_access)
		pmib->dot11OperationEntry.block_relay = 1;	
#endif
#endif

	// Set 802.1x flag
	enable1x = 0;
	if (encrypt < 2) {
		apmib_get(MIB_WLAN_ENABLE_1X, (void *)&intVal);
		apmib_get(MIB_WLAN_MAC_AUTH_ENABLED, (void *)&intVal2);
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
		
		apmib_get(MIB_WLAN_WAPI_MCASTREKEY,(void *)&intVal);
		pmib->wapiInfo.wapiUpdateMCastKeyType=intVal;

		apmib_get(MIB_WLAN_WAPI_MCAST_TIME,(void *)&intVal);
		pmib->wapiInfo.wapiUpdateMCastKeyTimeout=intVal;

		apmib_get(MIB_WLAN_WAPI_UCAST_PACKETS,(void *)&intVal);
		pmib->wapiInfo.wapiUpdateUCastKeyPktNum=intVal;
		
		apmib_get(MIB_WLAN_WAPI_UCASTREKEY,(void *)&intVal);
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

	apmib_get(MIB_MESH_ENABLE,(void *)&intVal);
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

	apmib_get(MIB_SCRLOG_ENABLED, (void *)&intVal);
	pmib->dot1180211sInfo.log_enabled = intVal;

	apmib_get(MIB_MESH_ID, (void *)buf1);
	intVal2 = strlen(buf1);
	memset(pmib->dot1180211sInfo.mesh_id, 0, 32);
	memcpy(pmib->dot1180211sInfo.mesh_id, buf1, intVal2);

	apmib_get(MIB_MESH_ENCRYPT, (void *)&intVal);
	apmib_get(MIB_MESH_WPA_AUTH, (void *)&intVal2);

	if( intVal2 == 2 && intVal)
		pmib->dot11sKeysTable.dot11Privacy	= 2;
	else
		pmib->dot11sKeysTable.dot11Privacy	= 0;
	
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
	apmib_get(MIB_WLAN_WPA_AUTH, (void *)&intVal3);
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
			apmib_get(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&intVal2);
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
			apmib_get(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&intVal2);
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

		apmib_get(MIB_WLAN_WPA_PSK, (void *)buf1);
		strcpy(pmib->dot1180211AuthEntry.dot11PassPhrase, buf1);

		apmib_get(MIB_WLAN_WPA_GROUP_REKEY_TIME, (void *)&intVal);
		pmib->dot1180211AuthEntry.dot11GKRekeyTime = intVal;			
	}
	else		
		pmib->dot1180211AuthEntry.dot11EnablePSK = 0;
#endif
	dumpCfgFile(ifname, pmib, vwlan_idx);
	
	free(pmib);
	return 0;
}


