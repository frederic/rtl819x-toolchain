/*
  *   MIB access control for RTL8197B
  *
  *	Copyright (C)2008, Realtek Semiconductor Corp. All rights reserved.
  *
  *	$Id: mib.c,v 1.3 2010/02/05 12:23:18 marklee Exp $
  */
  
/*================================================================*/
/* Include Files */
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


/*================================================================*/
/* Local Include Files */

#include "hcd.h"
#include "mib.h"
#include "../../../linux-2.6.30/drivers/char/rtl_mdio/rtl_mdio.h"
#define FW_VERSION	"1.0" 

/*================================================================*/
/* Local Variables */
//struct wlan_config_mib wlan_mib;
static struct io_config_mib io_mib;
struct config_mib_all mib_all;

/*for internal use now....*/
struct config_cmd_entry io_cmd_table[]={
CMD_IO_DEF(cmd_timeout, 				BYTE_T,	"10"	,			1,			255		    ),
CMD_IO_DEF(mii_pause_enable, 			BYTE_T, 	"1"	,			0,			1			),
CMD_IO_DEF(eth_pause_enable, 			BYTE_T, 	"1"	,				0,			1			),
CMD_IO_DEF(cpu_suspend_enable,		    BYTE_T, 	"0"	,				0,			1			),
CMD_IO_DEF(phy_reg_poll_time, 			BYTE_T, 	"10"	,			1,			100	    ),		
{0}
};	

struct config_cmd_entry wlan_comm_config_cmd_table[] = {
CMD_WLAN_COMM_DEF(MIB_VAP_NUM ,  "vap_number", BYTE_T,	vap_number, "4" ,	0, 4),
CMD_WLAN_COMM_DEF(MIB_HW_RF_TYPE ,  "RFPowerScale", BYTE_T,	RFPowerScale	, "0" ,  0 , 1),
CMD_WLAN_COMM_DEF(MIB_HW_REG_DOMAIN ,  "regdomain", BYTE_T,	regDomain , "1" ,  1 , 10),
CMD_WLAN_COMM_DEF(MIB_WLAN_CHAN_NUM ,  "channel", BYTE_T,	channel , "11" ,  1 , 14),
CMD_WLAN_COMM_DEF(MIB_WLAN_BAND ,  "band",	 BYTE_T,	wlanBand	, "11" ,  1 , 31),
CMD_WLAN_COMM_DEF(MIB_WLAN_CHANNEL_BONDING ,  "use40M", BYTE_T,	channelbonding	, "1" ,  0 , 1),
CMD_WLAN_COMM_DEF(MIB_WLAN_CONTROL_SIDEBAND	,  "2ndchoffset",	 BYTE_T,	controlsideband ,"1" ,	1 , 2),
CMD_WLAN_COMM_DEF(MIB_WLAN_FIX_RATE	,  "fixrate",	 INT_BIT_T, fixedTxRate ,"1" ,	1 , 0xffffffff),
CMD_WLAN_COMM_DEF(MIB_WLAN_BASIC_RATE ,	"basicrates", WORD_T,	basicRates	, "15"	,  1 , 0xffff),
CMD_WLAN_COMM_DEF(MIB_WLAN_PROTECTION_DISABLED ,  "disable_protection", BYTE_T,	protectionDisabled	, "0" ,  0 , 1),
CMD_WLAN_COMM_DEF(MIB_WLAN_BEACON_INTERVAL ,  "bcnint", WORD_T,	beaconInterval , "100" ,  20 , 1024),
CMD_WLAN_COMM_DEF(MIB_WLAN_DTIM_PERIOD ,  "dtimperiod", INT_T, dtimPeriod	, "0" ,  0 , 1),
CMD_WLAN_COMM_DEF(MIB_WLAN_RTS_THRESHOLD ,  "rtsthres", WORD_T,	rtsThreshold	, "2347" ,	0 , 2347),
CMD_WLAN_COMM_DEF(MIB_WLAN_FRAG_THRESHOLD	,  "fragthres",  WORD_T,	fragThreshold	,"2346" ,  256 , 2346),
{0},
};

struct config_cmd_entry wlan_config_cmd_table[]={
/*WLAN config mib cmd----------------------------------------*/	
/*id , name, type, mib_name ,def,start, end, ----------------------------------------*/
CMD_WLAN_DEF(MIB_INTF_NAME ,  "name", STRING_T,	name, "wlan" ,  0, 10),
CMD_WLAN_DEF(MIB_WLAN_PREAMBLE_TYPE ,  "preamble", BYTE_T,	preambleType	, "0" ,  0 , 1),
CMD_WLAN_DEF(MIB_WLAN_MAC_ADDR ,  "macaddr", STRING_T,	macaddr	, "00e04c8196c0" ,  0 , 12),
CMD_WLAN_DEF(MIB_WLAN_SSID 	,  "ssid",	 STRING_T,	ssid	,"NFBI_AP" ,  0 , 32),
CMD_WLAN_DEF(MIB_WLAN_AC_ENABLED ,  "aclmode", BYTE_T,	acEnabled	, "0" ,  0 , 2),
CMD_WLAN_DEF(MIB_WLAN_SUPPORTED_RATE ,  "oprates", WORD_T,	supportedRates	, "4095"  ,  1 , 0xffff),
CMD_WLAN_DEF(MIB_WLAN_RATE_ADAPTIVE_ENABLED ,  "autorate", BYTE_T,	rateAdaptiveEnabled, "1" ,  0 , 1),
CMD_WLAN_DEF(MIB_WLAN_AUTH_TYPE ,  "authtype", BYTE_T,	authType	, "2"  ,  0 , 2),
CMD_WLAN_DEF(MIB_WLAN_ENCRYPT ,  "encmode", BYTE_T,	encrypt	, "0" ,  0 , 5),
CMD_WLAN_DEF(MIB_WLAN_WEP_DEFAULT_KEY ,  "wepdkeyid", BYTE_T,	wepDefaultKey	, "0" ,  0 , 3),
CMD_WLAN_DEF(MIB_WLAN_WPA_CIPHER_SUITE ,  "wpa_cipher", BYTE_T,	wpaCipher	, "0"  ,  0 , 8),
CMD_WLAN_DEF(MIB_WLAN_WPA2_CIPHER_SUITE ,  "wpa2_cipher", BYTE_T,	wpa2Cipher	, "0" ,  0 , 8),
CMD_WLAN_DEF(MIB_WLAN_WPA_PSK ,  "passphrase", STRING_T,	wpaPSK	, "" ,  0 , 32),
CMD_WLAN_DEF(MIB_WLAN_WPA_GROUP_REKEY_TIME ,  "gk_rekey",	 INT_T,	wpaGroupRekeyTime	,"0" ,  0 , 1000),
CMD_WLAN_DEF(MIB_WLAN_ENABLE_1X ,  "802_1x", BYTE_T,	enable1X	, "0"  ,  0 , 1),
CMD_WLAN_DEF(MIB_WLAN_WEP128_KEY1 ,  "wepkey1", BYTE13_T,	wep128Key1	, "" ,  13 , 13),
CMD_WLAN_DEF(MIB_WLAN_WEP128_KEY2 ,  "wepkey2", BYTE13_T,	wep128Key2	, "" ,  13, 13),
CMD_WLAN_DEF(MIB_WLAN_WEP128_KEY3 ,  "wepkey3", BYTE13_T,	wep128Key3	,"" ,  13, 13),
CMD_WLAN_DEF(MIB_WLAN_WEP128_KEY4 ,  "wepkey4", BYTE13_T,	wep128Key4	, ""  ,  13 , 13),
CMD_WLAN_DEF(MIB_WLAN_BLOCK_RELAY ,  "block_relay", BYTE_T,	blockRelay	, "0" ,  0 , 2),
CMD_WLAN_DEF(MIB_WLAN_WMM_ENABLED ,  "qos_enable", BYTE_T,	wmmEnabled	, "0"  ,  0 , 1),
CMD_WLAN_DEF(MIB_WLAN_AGGREGATION ,  "aggregation",  BYTE_T,	aggregation	,"1" ,  0 , 3), 
CMD_WLAN_DEF(MIB_WLAN_SHORT_GI ,  "shortGI", BYTE_T,	shortgiEnabled	, "1"  ,  0 , 1),
CMD_WLAN_DEF(MIB_WLAN_PSK_ENABLE ,  "psk_enable",	 BYTE_T,	psk_enable	,"0" ,  0 , 2), 
//--------------------priv mib-------------
CMD_WLAN_DEF(MIB_WLAN_SHORT_RETRY ,  "shortretry", BYTE_T,	shortretry	, "3"  ,  1 , 0xff),
CMD_WLAN_DEF(MIB_WLAN_LONG_RETRY ,  "longretry", BYTE_T,	longretry	, "3" ,  1 , 0xff),
CMD_WLAN_DEF(MIB_WLAN_HIDDEN_SSID ,  "hiddenAP", BYTE_T,	hiddenSSID	, "0" ,  0 , 1),
CMD_WLAN_DEF(MIB_WLAN_DISABLED ,  "rf_disable", BYTE_T,	wlanDisabled	, "0" ,  0 , 1),
#if 0 //TBD
//CMD_WLAN_DEF(MIB_WLAN_AC_ADDR 	,  "acladdr",	 WLAC_ARRAY_T,	acAddrArray	,"1" ,  0 , 32),
CMD_WLAN_DEF(MIB_WLAN_SHORT_RETRY ,  "shortretry", BYTE_T,	shortretry	, "3"  ,  1 , 0xff),
CMD_WLAN_DEF(MIB_WLAN_LONG_RETRY ,  "longretry", BYTE_T,	longretry	, "3" ,  1 , 0xff),
CMD_WLAN_DEF(MIB_WLAN_WDS_ENABLED ,  "wds_enable", BYTE_T,	wdsEnabled	, "0"  ,  0 , 32),
CMD_WLAN_DEF(MIB_WLAN_CHAN_NUM ,  "wds_pure", BYTE_T,	channel	, "1" ,  1 , 11),
CMD_WLAN_DEF(MIB_WLAN_PREAMBLE_TYPE ,  "wds_priority", BYTE_T,	preambleType	, "1" ,  0 , 32),
CMD_WLAN_DEF(MIB_WLAN_SSID 	,  "wds_add",	 STRING_T,	ssid	,"MotoAP" ,  0 , 32),
CMD_WLAN_DEF(MIB_WLAN_BASIC_RATE ,  "wds_encrypt", WORD_T,	basicRates	, "1"  ,  0 , 32),
CMD_WLAN_DEF(MIB_WLAN_CHAN_NUM ,  "wds_wepkey", BYTE_T,	channel	, "0" ,  0 , 14),
CMD_WLAN_DEF(MIB_WLAN_PREAMBLE_TYPE ,  "wds_passphrase", BYTE_T,	preambleType	, "1" ,  0 , 32),
CMD_WLAN_DEF(MIB_WLAN_WSC_ENABLE ,  "wsc_enable", BYTE_T,	wsc_enable	, "0" ,  0 , 32),
CMD_WLAN_DEF(MIB_WLAN_SSID 	,  "pin",	 STRING_T,	ssid	,"MotoAP" ,  0 , 32),
CMD_WLAN_DEF(MIB_WLAN_BASIC_RATE ,  "supportedmcs", WORD_T,	basicRates	, "1"  ,  0 , 32),
CMD_WLAN_DEF(MIB_WLAN_CHAN_NUM ,  "basicmcs", BYTE_T,	channel	, "0" ,  0 , 14),
#endif
CMD_WLAN_DEF(MIB_VLAN_CONFIG ,  "VLAN", STRING_T,	vlan, "0,0,0,1,0,1" ,  0, 30),
{0}
};

struct config_cmd_entry eth_config_cmd_table[]={
CMD_ETH_DEF(MIB_INTF_NAME ,  "name", STRING_T,	name, "eth" ,  0, 10),
CMD_ETH_DEF(MIB_VLAN_CONFIG ,  "VLAN", STRING_T,	vlan, "0,0,0,1,0,1" ,  0, 30),
{0}
};

struct config_cmd_entry sys_config_cmd_table[]={
CMD_SYS_DEF(MIB_ENABLE_EFUSE_CONFIG , "enabel_efuse_config", BYTE_T, enable_efuse_config, "0", 0, 1),
CMD_SYS_DEF(MIB_ETH_MAC ,  "eth_mac", STRING_T,	eth_mac, "001234567899" ,	0, 13),
CMD_SYS_DEF(MIB_VLAN_ENABLE ,  "vlan_enable", BYTE_T,	vlan_enable, "1" ,  0, 1),
{0}
};



/*================================================================*/
/* Routine Implementations */

static unsigned char convert_atob(char *data, int base)
{
	char tmpbuf[10];
	int bin;

	memcpy(tmpbuf, data, 2);
	tmpbuf[2]='\0';
	if (base == 16)
		sscanf(tmpbuf, "%02x", &bin);
	else
		sscanf(tmpbuf, "%02d", &bin);
	return((unsigned char)bin);
}

/*
  *	check if more than one bits is asserted.    
  */
static int is_more_bit_asserted(int val)
{
	int i, num=0;
	
	for (i=0; i<32; i++) {
		if (BIT(i) & val)
			num++;
	}
	if (num > 1)
		return 1;
	else
		return 0;
}

#ifdef CMD_LINE
void dump_mib( int all , char *nameid)
{
	int i = 0, val, index=0;
	unsigned char *ptr, tmpbuf[256], bVal;
	unsigned short wVal;
	unsigned char *pmib ;
	struct config_cmd_entry *config_cmd_table;
	char *intf, *tmp;

	intf = strchr(nameid,' ');
	tmp = (char *)malloc(intf-nameid)+1;
	memset(tmp,0,intf-nameid+1);
	memcpy(tmp, nameid, intf-nameid);
	tmp[intf-nameid] = '\0';
	intf = tmp;
	nameid = strchr(nameid,' ')+1;
	
	if( select_mib(intf, &pmib, &config_cmd_table) < 0) {
		DEBUG_ERR("MIB not found!\n");
		return -1;
	}

	while (config_cmd_table[i].id != LAST_ENTRY_ID) {
		if( (all ==1) || ( (all !=1) && (!strcmp(config_cmd_table[i].name, nameid))) ){
		
		switch (config_cmd_table[i].type) {
			case BYTE_T:
				memcpy(&bVal, ((unsigned char *)pmib)+config_cmd_table[i].offset, 1);
				printf("%s=%d\n", config_cmd_table[i].name, (int)bVal);
				break;				

			case WORD_T:
				memcpy(&wVal, ((unsigned char *)pmib)+config_cmd_table[i].offset, 2);
				printf("%s=%d\n", config_cmd_table[i].name, (int)wVal);
				break;				
			
			case INT_T:
			case INT_BIT_T:
				memcpy(&val, ((unsigned char *)pmib)+config_cmd_table[i].offset, 4);
				printf("%s=%d\n", config_cmd_table[i].name, val);
				break;				
				
			case STRING_T:
				ptr = ((unsigned char *)pmib)+config_cmd_table[i].offset;
				if (strlen(ptr) > 0)
					strcpy(tmpbuf, ptr);
				else
					tmpbuf[0] = '\0';
				printf("%s=%s\n", config_cmd_table[i].name, ptr);				
				break;				

			case BYTE5_T:
				ptr = ((unsigned char *)pmib)+config_cmd_table[i].offset;
				printf("%s=%02x%02x%02x%02x%02x\n", config_cmd_table[i].name, 
							*ptr, *(ptr+1),  *(ptr+2),  *(ptr+3),  *(ptr+4) );
				break;

			case BYTE6_T:
				ptr = ((unsigned char *)pmib)+config_cmd_table[i].offset;
				printf("%s=%02x%02x%02x%02x%02x%02x\n", config_cmd_table[i].name, 
							*ptr, *(ptr+1),  *(ptr+2),  *(ptr+3),  *(ptr+4),  *(ptr+5));
				break;
				
			case BYTE13_T:				
				ptr = ((unsigned char *)pmib)+config_cmd_table[i].offset;
				printf("%s=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
							config_cmd_table[i].name, 
							*ptr, *(ptr+1),  *(ptr+2),  *(ptr+3),  *(ptr+4),  *(ptr+5), *(ptr+6), 
							*(ptr+7),  *(ptr+8),  *(ptr+9),  *(ptr+10),  *(ptr+11), *(ptr+12));
				break;				
			
			default:
				printf("Invalid mib type!\n");
				return;
			}
			if(!all) break; 
		}
		//printf(">>> [%s] read (%02x)%s as %d\n",__FUNCTION__,((unsigned char *)pmib)+config_cmd_table[i].offset,config_cmd_table[i].name,*(((unsigned char *)pmib)+config_cmd_table[i].offset));
		i++;
	}
}
#endif


static int init_config_mib_default()
{
	int i = 0, val, j, max_len, index=0;
	char *ptr, bVal;
	unsigned short wVal;
	//struct wlan_config_mib *pmib = &wlan_mib;

	i = 0;
	while (sys_config_cmd_table[i].id != LAST_ENTRY_ID) {
		switch (sys_config_cmd_table[i].type) {
			case BYTE_T:
				if (sys_config_cmd_table[i].def) {				
					bVal = (unsigned char)atoi(sys_config_cmd_table[i].def);				
					memcpy(((unsigned char *)&mib_all.sys_config)+sys_config_cmd_table[i].offset, &bVal, 1);
				}
				break;
				
			case WORD_T:			
				if (sys_config_cmd_table[i].def) {				
					wVal = (unsigned short)atoi(sys_config_cmd_table[i].def);				
					memcpy(((unsigned char *)&mib_all.sys_config)+sys_config_cmd_table[i].offset, &wVal, 2);
				}
				break;							
			
			case INT_T:
			case INT_BIT_T:
				if (sys_config_cmd_table[i].def) {				
					val = atoi(sys_config_cmd_table[i].def);				
					memcpy(((unsigned char *)&mib_all.sys_config)+sys_config_cmd_table[i].offset, &val, 4);
				}
				break;				
				
			case STRING_T:
				if (sys_config_cmd_table[i].def) {
					strcpy(((unsigned char *)&mib_all.sys_config)+sys_config_cmd_table[i].offset, sys_config_cmd_table[i].def);
				}
				break;				
			case BYTE5_T:
			case BYTE6_T:
			case BYTE13_T:				
				if (sys_config_cmd_table[i].def) {
					if (sys_config_cmd_table[i].type ==BYTE5_T)
						max_len = 5;
					else if (sys_config_cmd_table[i].type ==BYTE6_T)
						max_len = 6;
					else
						max_len = 13;					
					for (j=0, ptr=sys_config_cmd_table[i].def; *ptr && j<max_len; j++, ptr+=2) {
						if ( !isxdigit((int)*ptr) || !isxdigit((int)*(ptr+1)) ) {
							printf("Invalid BYTE_T vlaue!\n");
							return -1;
						}				
						*(((unsigned char *)&mib_all.sys_config)+sys_config_cmd_table[i].offset+j) = convert_atob(ptr, 16);
					}					
				}
				break;	
				
			default:
				printf("Invalid mib type!\n");
				return -1;
		}
		i++;
	}

	for( index=0; index<MAX_ETH_INTF; index++) {
		i = 0;
		while (eth_config_cmd_table[i].id != LAST_ENTRY_ID) {
			switch (eth_config_cmd_table[i].type) {
				case BYTE_T:
					if (eth_config_cmd_table[i].def) {				
						bVal = (unsigned char)atoi(eth_config_cmd_table[i].def);				
						memcpy(((unsigned char *)&mib_all.eth[index])+eth_config_cmd_table[i].offset, &bVal, 1);
					}
					break;
					
				case WORD_T:			
					if (eth_config_cmd_table[i].def) {				
						wVal = (unsigned short)atoi(eth_config_cmd_table[i].def);				
						memcpy(((unsigned char *)&mib_all.eth[index])+eth_config_cmd_table[i].offset, &wVal, 2);
					}
					break;							
				
				case INT_T:
				case INT_BIT_T:
					if (eth_config_cmd_table[i].def) {				
						val = atoi(eth_config_cmd_table[i].def);				
						memcpy(((unsigned char *)&mib_all.eth[index])+eth_config_cmd_table[i].offset, &val, 4);
					}
					break;				
					
				case STRING_T:
					if (eth_config_cmd_table[i].def) {
						strcpy(((unsigned char *)&mib_all.eth[index])+eth_config_cmd_table[i].offset, eth_config_cmd_table[i].def);
					}
					break;				
				case BYTE5_T:
				case BYTE6_T:
				case BYTE13_T:				
					if (eth_config_cmd_table[i].def) {
						if (eth_config_cmd_table[i].type ==BYTE5_T)
							max_len = 5;
						else if (eth_config_cmd_table[i].type ==BYTE6_T)
							max_len = 6;
						else
							max_len = 13;					
						for (j=0, ptr=eth_config_cmd_table[i].def; *ptr && j<max_len; j++, ptr+=2) {
							if ( !isxdigit((int)*ptr) || !isxdigit((int)*(ptr+1)) ) {
								printf("Invalid BYTE_T vlaue!\n");
								return -1;
							}				
							*(((unsigned char *)&mib_all.eth[index])+eth_config_cmd_table[i].offset+j) = convert_atob(ptr, 16);
						}					
					}
					break;	
					
				default:
					printf("Invalid mib type!\n");
					return -1;
			}
			i++;
		}
	}

	i = 0;
	while (wlan_comm_config_cmd_table[i].id != LAST_ENTRY_ID) {
		switch (wlan_comm_config_cmd_table[i].type) {
			case BYTE_T:
				if (wlan_comm_config_cmd_table[i].def) {				
					bVal = (unsigned char)atoi(wlan_comm_config_cmd_table[i].def);				
					memcpy(((unsigned char *)&mib_all.wlan_comm)+wlan_comm_config_cmd_table[i].offset, &bVal, 1);
				}
				break;
				
			case WORD_T:			
				if (wlan_comm_config_cmd_table[i].def) {				
					wVal = (unsigned short)atoi(wlan_comm_config_cmd_table[i].def);				
					memcpy(((unsigned char *)&mib_all.wlan_comm)+wlan_comm_config_cmd_table[i].offset, &wVal, 2);
				}
				break;							
			
			case INT_T:
			case INT_BIT_T:
				if (wlan_comm_config_cmd_table[i].def) {				
					val = atoi(wlan_comm_config_cmd_table[i].def);				
					memcpy(((unsigned char *)&mib_all.wlan_comm)+wlan_comm_config_cmd_table[i].offset, &val, 4);
				}
				break;				
				
			case STRING_T:
				if (wlan_comm_config_cmd_table[i].def) {
					strcpy(((unsigned char *)&mib_all.wlan_comm)+wlan_comm_config_cmd_table[i].offset, wlan_comm_config_cmd_table[i].def);
				}
				break;				
			case BYTE5_T:
			case BYTE6_T:
			case BYTE13_T:				
				if (wlan_comm_config_cmd_table[i].def) {
					if (wlan_comm_config_cmd_table[i].type ==BYTE5_T)
						max_len = 5;
					else if (wlan_comm_config_cmd_table[i].type ==BYTE6_T)
						max_len = 6;
					else
						max_len = 13;					
					for (j=0, ptr=wlan_comm_config_cmd_table[i].def; *ptr && j<max_len; j++, ptr+=2) {
						if ( !isxdigit((int)*ptr) || !isxdigit((int)*(ptr+1)) ) {
							printf("Invalid BYTE_T vlaue!\n");
							return -1;
						}				
						*(((unsigned char *)&mib_all.wlan_comm)+wlan_comm_config_cmd_table[i].offset+j) = convert_atob(ptr, 16);
					}					
				}
				break;	
				
			default:
				printf("Invalid mib type!\n");
				return -1;
		}
		//printf(">>> [%s] set (%02x)%s to deafult:%d\n",__FUNCTION__,((unsigned char *)&mib_all.wlan_comm)+wlan_comm_config_cmd_table[i].offset,wlan_comm_config_cmd_table[i].name,*(((unsigned char *)&mib_all.wlan_comm)+wlan_comm_config_cmd_table[i].offset));
		i++;
	}

	for( index=0; index<MAX_WLAN_INTF; index++) {
		i = 0;
		while (wlan_config_cmd_table[i].id != LAST_ENTRY_ID) {
			/*
			if( strcmp(wlan_config_cmd_table[i].name,"VLAN") == 0 ){
				unsigned char tmpbuf[100] = {'\0'};
				while( (ptr = strstr(wlan_config_cmd_table[i].def,',')) ){
					*ptr = '\0';
					ptr = wlan_config_cmd_table[i].def;
					val = atoi(ptr);
					sprintf(tmpbuf,"%s %d ",tmpbuf,val);
					wlan_config_cmd_table[i].def = strstr(wlan_config_cmd_table[i].def,'\0')+1;
				}
				printf(">>> Default VLAN config:%s\n",tmpbuf);
				i++;
				continue;
			}
			*/
			
			switch (wlan_config_cmd_table[i].type) {
				case BYTE_T:
					if (wlan_config_cmd_table[i].def) {				
						bVal = (unsigned char)atoi(wlan_config_cmd_table[i].def);				
						memcpy(((unsigned char *)&mib_all.wlan[index])+wlan_config_cmd_table[i].offset, &bVal, 1);
					}
					break;
					
				case WORD_T:			
					if (wlan_config_cmd_table[i].def) {				
						wVal = (unsigned short)atoi(wlan_config_cmd_table[i].def);				
						memcpy(((unsigned char *)&mib_all.wlan[index])+wlan_config_cmd_table[i].offset, &wVal, 2);
					}
					break;							
				
				case INT_T:
				case INT_BIT_T:
					if (wlan_config_cmd_table[i].def) {				
						val = atoi(wlan_config_cmd_table[i].def);				
						memcpy(((unsigned char *)&mib_all.wlan[index])+wlan_config_cmd_table[i].offset, &val, 4);
					}
					break;				
					
				case STRING_T:
					if (wlan_config_cmd_table[i].def) {
						if( index != 0 ) {
							char tmp[32] = {'\0'};
							if( strcmp(wlan_config_cmd_table[i].name,"ssid") == 0 ) {
								sprintf(tmp,"NFBI-VAP-%02x",index);
								strcpy(((unsigned char *)&mib_all.wlan[index])+wlan_config_cmd_table[i].offset,tmp);
							} else if( strcmp(wlan_config_cmd_table[i].name,"macaddr") == 0 ) {
								sprintf(tmp,"00e04c00fb0%d",index);
								strcpy(((unsigned char *)&mib_all.wlan[index])+wlan_config_cmd_table[i].offset,tmp);
							} else {
								strcpy(((unsigned char *)&mib_all.wlan[index])+wlan_config_cmd_table[i].offset, wlan_config_cmd_table[i].def);
							}
						} else {
							strcpy(((unsigned char *)&mib_all.wlan[index])+wlan_config_cmd_table[i].offset, wlan_config_cmd_table[i].def);
						}
					}
					break;				
				case BYTE5_T:
				case BYTE6_T:
				case BYTE13_T:				
					if (wlan_config_cmd_table[i].def) {
						if (wlan_config_cmd_table[i].type ==BYTE5_T)
							max_len = 5;
						else if (wlan_config_cmd_table[i].type ==BYTE6_T)
							max_len = 6;
						else
							max_len = 13;					
						for (j=0, ptr=wlan_config_cmd_table[i].def; *ptr && j<max_len; j++, ptr+=2) {
							if ( !isxdigit((int)*ptr) || !isxdigit((int)*(ptr+1)) ) {
								printf("Invalid BYTE_T vlaue!\n");
								return -1;
							}				
							*(((unsigned char *)&mib_all.wlan[index])+wlan_config_cmd_table[i].offset+j) = convert_atob(ptr, 16);	
						}
					}
					break;	
					
				default:
					printf("Invalid mib type!\n");
					return -1;
			}
			//printf(">>> [%s] set %s to deafult:%d\n",__FUNCTION__,wlan_config_cmd_table[i].name,*(((unsigned char *)&mib_all.wlan[index])+wlan_config_cmd_table[i].offset));
			i++;
		}
	}

	for( index=0;index<MAX_ETH_INTF;index++){
		sprintf(mib_all.eth[index].port_name, "p%d",index);
		sprintf(mib_all.eth[index].name, "eth%d",index);
	}

	for( index=0;index<MAX_WLAN_INTF;index++){
		if( index == 0 )
			sprintf(mib_all.wlan[index].name, "wlan0");
		else
			sprintf(mib_all.wlan[index].name, "wlan0-va%d",index-1);
	}

	return 0;
}

unsigned char *string_to_hex(unsigned char *hex, unsigned char *str, int len)
{
	int i=0;
	
	for( i=0;i<len*2;i++ ){
		if( !(str[i] < '0') && str[i] < 'a' )
			hex[i/2] |= (str[i]-'0') << (((i+1)%2)*4);
		else if( str[i] > '9' && str[i] < 'g' )
			hex[i/2] |= ((str[i]-'a')+10) << (((i+1)%2)*4);
	}
	return hex;
}

unsigned int get_vap_num(void)
{
	return mib_all.wlan_comm.vap_number;
}

unsigned char *get_macaddr(unsigned char *ifname)
{
	int i=0;
	unsigned char mac[7]={'\0'}, name[20];

	for( i=0; i<MAX_WLAN_INTF; i++ ){
		if( strcmp(mib_all.wlan[i].name,ifname) == 0 ){
			return string_to_hex(mac,mib_all.wlan[i].macaddr,6);
		}
	}
	return NULL;
}

int select_mib(char *intf, unsigned char **mib, struct config_cmd_entry **config_cmd_table)
//int select_mib(char *intf, char *mib, int *table_switch)
{
	int index = 0;

	/*
	if( strncmp(intf,"eth",3) == 0 ) {
		for( index=0;index<MAX_ETH_INTF;index++) {
			if( strcmp(mib_all.eth[index].name,intf) == 0 ){
				*mib = &mib_all.eth[index];
				*config_cmd_table = eth_config_cmd_table;
				return 0;
			}
		}
	}
	*/
	if( strncmp(intf,"p",1) == 0 ) {
		for( index=0;index<MAX_ETH_INTF;index++) {
			if( strcmp(mib_all.eth[index].port_name,intf) == 0 ){
				*mib = &mib_all.eth[index];
				*config_cmd_table = eth_config_cmd_table;
				return 0;
			}
		}
	}

	if( strncmp(intf,"wlan",4) == 0 ) {
		for(index=0 ;index<MAX_WLAN_INTF;index++) {
			if( strcmp(mib_all.wlan[index].name,intf) == 0 ){
				*mib = &mib_all.wlan[index];
				*config_cmd_table = wlan_config_cmd_table;
				return 0;
			}
		}
	}

	if( strncmp(intf,"wlan_comm",9) == 0 ) {
		*mib = &mib_all.wlan_comm;
		*config_cmd_table = wlan_comm_config_cmd_table;
		return 0;
	}

	if( strncmp(intf,"sys",9) == 0 ) {
		*mib = &mib_all.sys_config;
		*config_cmd_table = sys_config_cmd_table;
		return 0;
	}

	return -1;
}

int access_config_mib(int flag, char *nameid, void *data1, char *intf)
{
	int val, i = 0, ret = RET_OK;
	
	int j;
	char *ptr;
	unsigned long dwVal;
	unsigned short wVal;
	unsigned char tmpbuf[256] = {'\0'}, bVal;
	void *data=NULL;
	unsigned char *pmib ;
	struct config_cmd_entry *config_cmd_table;

	if (nameid == NULL) {
		DEBUG_ERR("nameid == NULL!\n");
		return -1;
	}

	ret = select_mib(intf, &pmib, &config_cmd_table);
	if( ret < 0) {
		DEBUG_ERR("MIB not found!\n");
		return -1;
	}
	//pmib = (struct wlan_config_mib *)&wlan_mib;

	while (config_cmd_table[i].id != LAST_ENTRY_ID){
		if (((flag & ACCESS_MIB_BY_NAME) && !strcmp(config_cmd_table[i].name, nameid)) ||
				 ((flag & ACCESS_MIB_BY_ID) && (config_cmd_table[i].id == *((int *)nameid)))) {
			// Do MIB R/W			
				switch (config_cmd_table[i].type) {
					case BYTE_T:
						if (flag & ACCESS_MIB_SET) {
						
							if 	(flag & ACCESS_MIB_BY_NAME)
								bVal = (unsigned char)atoi(data1);
							else
								bVal = ((unsigned char *)data1)[0];
							if ((((int)bVal) < config_cmd_table[i].start || ((int)bVal) > config_cmd_table[i].end)) {
								DEBUG_ERR("Invalid BYTE_T cmd range [%d, %d, %d])!\n", bVal, config_cmd_table[i].start, config_cmd_table[i].end);
								return -RET_INVALID_RANGE;
							}
							//printf("%s from %d to %d\n", config_cmd_table[i].name, *(((unsigned char *)pmib)+config_cmd_table[i].offset), bVal);
							memcpy(((unsigned char *)pmib)+config_cmd_table[i].offset, &bVal, 1);
						}
						else { //GET
							memcpy(&bVal, ((unsigned char *)pmib)+config_cmd_table[i].offset, 1);			
							memcpy(data1, &bVal, 1);
							ret = 1;
							//printf("get %s vlaue:%d\n", config_cmd_table[i].name, *(char *)data1);
						}					
						break;				
						
					case WORD_T:
						if (flag & ACCESS_MIB_SET) {			

							if 	(flag & ACCESS_MIB_BY_NAME)						
								wVal = (unsigned short)atoi(data1);
							else
								memcpy(&wVal, data1, 2);

							if ((((int)wVal) < config_cmd_table[i].start || ((int)wVal) > config_cmd_table[i].end)) {
								DEBUG_ERR("Invalid WORD_T cmd range [%d, %d, %d])!\n", val, config_cmd_table[i].start, config_cmd_table[i].end);
								return -RET_INVALID_RANGE;
							}
							//printf("%s from %d to %d\n", config_cmd_table[i].name, *(((unsigned char *)pmib)+config_cmd_table[i].offset), wVal);
							memcpy(((unsigned char *)pmib)+config_cmd_table[i].offset, &wVal, 2);
						}
						else {
							memcpy(&wVal, ((unsigned char *)pmib)+config_cmd_table[i].offset, 2);	
							memcpy(data1, &wVal, 2);	
							ret = 2;
							//printf("get %s value:%d\n", config_cmd_table[i].name, wVal);
						}					
						break;												
					case INT_T:
					case INT_BIT_T:
						if (flag & ACCESS_MIB_SET) {					

							if 	(flag & ACCESS_MIB_BY_NAME)							
								val = atoi(data1);
							else
								memcpy(&val, data1, 4);							
							
							if ((val < config_cmd_table[i].start || val > config_cmd_table[i].end)) {
								DEBUG_ERR("Invalid INT_T cmd range [%d, %d, %d])!\n", val, config_cmd_table[i].start, config_cmd_table[i].end);
								return -RET_INVALID_RANGE;
							}						
							if ((config_cmd_table[i].type == INT_BIT_T) && is_more_bit_asserted(val)) {
								DEBUG_ERR("Invalid cmd range [%d, %d, %d])!\n", val, config_cmd_table[i].start, config_cmd_table[i].end);
								return -RET_INVALID_RANGE;
							}
							//printf("%s from %d to %d\n", config_cmd_table[i].name, *(((unsigned char *)pmib)+config_cmd_table[i].offset), val);
							memcpy(((unsigned char *)pmib)+config_cmd_table[i].offset, &val, 4);
						}
						else { //GET
							memcpy(&val, ((unsigned char *)pmib)+config_cmd_table[i].offset, 4);			
							memcpy(data1, &val, 4);	
							ret = 4;
							//printf("get %s value:%d\n", config_cmd_table[i].name, val);
						}					
						break;				
					
					case STRING_T:
						if (flag & ACCESS_MIB_SET) {
							if ((strlen(data1) > 0) && (strlen(data1) < config_cmd_table[i].start || strlen(data1) > config_cmd_table[i].end)) {
								DEBUG_ERR("Invalid STRINT_T cmd range [%d, %d, %d])!\n", strlen(data1), config_cmd_table[i].start, config_cmd_table[i].end);
								return -RET_INVALID_RANGE;
							}
							//printf("%s from %s to %s\n", config_cmd_table[i].name, ((unsigned char *)pmib)+config_cmd_table[i].offset, data1);
							strcpy(((unsigned char *)pmib)+config_cmd_table[i].offset, data1);
						}
						else { //GET
							strcpy((unsigned char *)data1, ((unsigned char *)pmib)+config_cmd_table[i].offset);	
							ret = strlen(data1);
							//printf("get %s value:%s\n", config_cmd_table[i].name, data1);
						}
						break;
				       case BYTE5_T:
					case BYTE6_T:
					case BYTE13_T:				
						if (flag & ACCESS_MIB_SET) {	
							if (flag & ACCESS_MIB_BY_NAME) {						
								if (strlen(data1) != config_cmd_table[i].start*2) { //HEX ,ex -> 112233445566
									DEBUG_ERR("Invalid BYTE cmd length [%d, %d])!\n", strlen(data1), config_cmd_table[i].start);
									return -RET_INVALID_RANGE;
								}						
								for (j=0, ptr=data1; *ptr && j<config_cmd_table[i].start; j++, ptr+=2) {
									if (!isxdigit((int)*ptr) || !isxdigit((int)*(ptr+1)) ) {
										DEBUG_ERR("%s: Invalid BYTE_T vlaue!\n", __FUNCTION__);
										return -RET_INVALID_RANGE;
									}				
									*(((unsigned char *)pmib)+config_cmd_table[i].offset+j) = convert_atob(ptr, 16);
								}		
							}
							else 
								memcpy(((unsigned char *)pmib)+config_cmd_table[i].offset, data1, config_cmd_table[i].start);
						}
						else { //GET
#if 0
							if (flag & ACCESS_MIB_BY_NAME) {
								ptr = ((unsigned char *)pmib)+config_cmd_table[i].offset;
								if (config_cmd_table[i].type ==BYTE6_T)  //5T 
									ret = printf("%s=%02x%02x%02x%02x%02x%02x\n", config_cmd_table[i].name, 
										*ptr, *(ptr+1),  *(ptr+2),  *(ptr+3),  *(ptr+4),  *(ptr+5));
								else							
									ret = printf("%s=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
											config_cmd_table[i].name,
											*ptr, *(ptr+1),  *(ptr+2),  *(ptr+3),  *(ptr+4),  *(ptr+5), *(ptr+6),
											*(ptr+7),  *(ptr+8),  *(ptr+9),  *(ptr+10),  *(ptr+11), *(ptr+12));
							}
							//else 
#endif		
							{
								memcpy(data1, ((unsigned char *)pmib)+config_cmd_table[i].offset,  config_cmd_table[i].start);	
								ret =  config_cmd_table[i].start;	
							}
						}					
						break;
					
					default:
						DEBUG_ERR("Invalid mib type!\n");
						return -RET_NOT_NOW;
				}
			return ret;
		}
		i++;
	}
	DEBUG_ERR("Can't find mib!\n");
	return -RET_INVALID_CMD_ID;
}

void init_config_mib() 
{
	memset(&mib_all, '\0', sizeof(mib_all)); 
	//memset(&mib_all.wlan, '\0', sizeof(struct wlan_config_mib)*MAX_WLAN_INTF); 
	//internal use
	io_mib.cmd_timeout = 10;
	io_mib.mii_pause_enable= 1;
	io_mib.eth_pause_enable= 1;
	io_mib.cpu_suspend_enable= 0;
	io_mib.phy_reg_poll_time= 10;

	//init wlan config mib to default
	init_config_mib_default(); 
}


 int init_io_config()
{
	int i=0;
	struct io_config_mib *pmib=&io_mib;	
	
	while (io_cmd_table[i].id != LAST_ENTRY_ID) {		
			do_mdio_ioctl(io_cmd_table[i].id, ((unsigned char *)pmib)+io_cmd_table[i].offset);		
		i++;
	}
}

