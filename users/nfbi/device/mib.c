/*
  *   MIB access control for RTL8197B
  *
  *	Copyright (C)2008, Realtek Semiconductor Corp. All rights reserved.
  *
  *	$Id: mib.c,v 1.17 2009/05/27 06:36:56 michael Exp $
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

#define FW_VERSION	"1.4"   //the length of version string < 16

extern int get_interface_index(char *ifname);
extern int get_wlan_mac_addr(char *ifname, char *macaddr);
#ifdef RT_WLAN
extern int get_wlan_info(char *data);
extern int request_scan(char *param);
extern int get_scan_result(char *param);
extern int cfgwrite(char *param);
extern int cfgread(char *param);
extern int priv_retrylimit(char *param, int shortflag);
#endif
/*================================================================*/
/* Local Variables */

struct cmd_table_entry cmd_table[]={ \
	CMD_DEF(cmd_timeout, 				BYTE_T,				"10",		1,			255,		    ACT_MIB_RW_IOCTL),
	CMD_DEF(mii_pause_enable, 			BYTE_T, 			"1",			0,			1,			ACT_MIB_RW_IOCTL),
	CMD_DEF(eth_pause_enable, 			BYTE_T, 			"1",			0,			1,			ACT_MIB_RW_IOCTL),
	CMD_DEF(cpu_suspend_enable,		    BYTE_T, 			"0",			0,			1,			ACT_MIB_RW_IOCTL),
	CMD_DEF(phy_reg_poll_time, 			BYTE_T, 			"10",		1,			100,		    ACT_MIB_RW_IOCTL),
	CMD_DEF(write_memory, 				INT_T, 				"0",			0,			0,			ACT_IOCTL),
	CMD_DEF(read_memory, 				INT_T, 				"0",			0,			0,			ACT_IOCTL),
	CMD_DEF(fw_version,				    STRING_T,	    FW_VERSION, 		0,			15,			ACT_MIB_RW),
	CMD_DEF(wlan_mac_addr,				BYTE_6_T,	"000000000000",			6,			6,			ACT_IOCTL),
	CMD_DEF(wlan_link_down_time,		BYTE_T, 			"5",			1,			60,		    ACT_MIB_RW),
	CMD_DEF(channel,					BYTE_T, 			"0",			0,			0,		    ACT_MIB_RW),
	CMD_DEF(ssid,						STRING_T, 			""	,			0,			32,		    ACT_MIB_RW),		
	CMD_DEF(bssid2join,					BYTE_6_T,			"",			6,			6,			    ACT_MIB_RW),
	CMD_DEF(regdomain,					BYTE_T, 				"1",			1,			11,		ACT_MIB_RW),
 	CMD_DEF(autorate,					BYTE_T, 				"1",			0,			1,			ACT_MIB_RW),
	CMD_DEF(fixrate,					INT_BIT_T, 			"0",			1,			0x4095, ACT_MIB_RW),	
	CMD_DEF(authtype,					BYTE_T, 				"2",			0,			2,			ACT_MIB_RW),
	CMD_DEF(encmode,					BYTE_T, 				"0",			0,			5,			ACT_MIB_RW),
	CMD_DEF(wepdkeyid,					BYTE_T, 				"0",			0,			3,			ACT_MIB_RW),
	CMD_DEF(psk_enable,					BYTE_T, 				"0",			0,			1,			ACT_MIB_RW),
	CMD_DEF(wpa_cipher,					INT_BIT_T,			"0",			0,			8,			ACT_MIB_RW),
	CMD_DEF(wpa2_cipher,				INT_BIT_T,			"0",			0,			8,			ACT_MIB_RW),
	CMD_DEF(passphrase,					STRING_T, 			"",			8,			64,		ACT_MIB_RW),
	CMD_DEF(wepkey1,					BYTE_13_T, 		"",			13,		13,		ACT_MIB_RW),
	CMD_DEF(wepkey2,					BYTE_13_T, 		"",			13,		13,		ACT_MIB_RW),
	CMD_DEF(wepkey3,					BYTE_13_T, 		"",			13,		13,		ACT_MIB_RW),
	CMD_DEF(wepkey4,					BYTE_13_T, 		"",			13,		13,		ACT_MIB_RW),
	CMD_DEF(opmode,						BYTE_T, 				"8",			8,			8,			ACT_MIB_RW),
	CMD_DEF(rtsthres,					WORD_T, 			"2347",	0,			2347,	ACT_MIB_RW),
	CMD_DEF(fragthres,					WORD_T, 			"2346",	256,		2346,	ACT_MIB_RW),
	CMD_DEF(shortretry,					BYTE_T, 				"6",			0,			255,		ACT_MIB_RW),
	CMD_DEF(longretry,					BYTE_T, 				"6",			0,			255,		ACT_MIB_RW),
	CMD_DEF(band,						BYTE_T, 				"11",		1,			31,		ACT_MIB_RW),
	CMD_DEF(macclone_enable,			BYTE_T, 				"0",			0,			1,			ACT_MIB_RW),
	CMD_DEF(clone_mac_addr,				BYTE_6_T,			"",			6,			6,			ACT_MIB_RW),
	CMD_DEF(qos_enable,					BYTE_T, 				"1",			0,			1,			ACT_MIB_RW),
	CMD_DEF(use40M,						BYTE_T, 				"1",			0,			1,			ACT_MIB_RW),
 	CMD_DEF(shortGI20M,					BYTE_T, 				"1",			0,			1,			ACT_MIB_RW),
	CMD_DEF(shortGI40M,					BYTE_T, 				"1",			0,			1,			ACT_MIB_RW),
	CMD_DEF(aggregation,				BYTE_T, 				"1",			0,			1,			ACT_MIB_RW),
	CMD_DEF(power_save_enable,		    BYTE_T, 				"0",			0,			3,			ACT_MIB_RW),
	CMD_DEF(sens,						BYTE_T, 				"0",			1,			99,		ACT_MIB_RW),
	CMD_DEF(txpower_cck,				BYTE_T, 				"0",			7,			18,		ACT_MIB_RW),
	CMD_DEF(txpower_ofdm,				BYTE_T, 				"0",			7,			15,		ACT_MIB_RW),
	CMD_DEF(txpower_ht,					BYTE_T, 				"0",			7,			13,		ACT_MIB_RW),
#ifdef RT_WLAN
	CMD_DEF(get_wlan_info,				INT_T, 				"0",			0,			1,			ACT_IOCTL),
	CMD_DEF(request_scan,				INT_T, 				"0",			0,			1,			ACT_IOCTL),
	CMD_DEF(get_scan_result,			INT_T, 				"0",			0,			1,			ACT_IOCTL),
	CMD_DEF(cfgwrite,			        INT_T, 				"0",			0,			1,			ACT_IOCTL),
	CMD_DEF(cfgread,			        INT_T, 				"0",			0,			1,			ACT_IOCTL),
	CMD_DEF(priv_shortretry,	        INT_T, 				"0",			0,			1,			ACT_IOCTL),
	CMD_DEF(priv_longretry,		        INT_T, 				"0",			0,			1,			ACT_IOCTL),
#endif
#ifdef JUMP_CMD	
	CMD_DEF(jump,						INT_T, 				"0",			0,			1,			ACT_IOCTL),
#endif
	/* last one type should be LAST_ENTRY */   
	CMD_DEF(get_scan_result,			LAST_ENTRY, 		"",			0,			0,			0),       
};


/*================================================================*/
/* Routine Implementations */


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

int assign_initial_value(struct mib *pmib)
{
	int i = 0, val, j, max_len;
	char *ptr, bVal;
	unsigned short wVal;

	while (cmd_table[i].type != LAST_ENTRY) {
		switch (cmd_table[i].type) {
			case BYTE_T:
				if (cmd_table[i].def) {				
					bVal = (unsigned char)atoi(cmd_table[i].def);				
					memcpy(((unsigned char *)pmib)+cmd_table[i].offset, &bVal, 1);
				}
				break;
				
			case WORD_T:			
				if (cmd_table[i].def) {				
					wVal = (unsigned short)atoi(cmd_table[i].def);				
					memcpy(((unsigned char *)pmib)+cmd_table[i].offset, &wVal, 2);
				}
				break;							
			
			case INT_T:
			case INT_BIT_T:
				if (cmd_table[i].def) {				
					val = atoi(cmd_table[i].def);				
					memcpy(((unsigned char *)pmib)+cmd_table[i].offset, &val, 4);
				}
				break;				
				
			case STRING_T:
				if (cmd_table[i].def) {
					strcpy(((unsigned char *)pmib)+cmd_table[i].offset, cmd_table[i].def);
				}
				break;				
			
			case BYTE_6_T:
			case BYTE_13_T:				
				if (cmd_table[i].def) {
					if (cmd_table[i].type ==BYTE_6_T)
						max_len = 6;
					else
						max_len = 13;					
					for (j=0, ptr=cmd_table[i].def; *ptr && j<max_len; j++, ptr+=2) {
						if ( !isxdigit((int)*ptr) || !isxdigit((int)*(ptr+1)) ) {
							printf("Invalid BYTE_T vlaue!\n");
							return -1;
						}				
						*(((unsigned char *)pmib)+cmd_table[i].offset+j) = convert_atob(ptr, 16);
					}					
				}
				break;	
				
			default:
				printf("Invalid mib type!\n");
				return -1;
		}
		i++;
	}
	return 0;
}

#ifdef CMD_LINE
void dump_all_mib(struct mib *pmib, int flag, int show_value)
{
	int i = 0, val;
	unsigned char *ptr, tmpbuf[256], bVal;
	unsigned short wVal;

	while (cmd_table[i].type != LAST_ENTRY) {
		if (cmd_table[i].action != flag) {
			i++;
			continue;
		}
		
		if (!show_value) {
			printf("%s\n", cmd_table[i].name);
			i++;
			continue;	
		}		
		
		switch (cmd_table[i].type) {
			case BYTE_T:
				memcpy(&bVal, ((unsigned char *)pmib)+cmd_table[i].offset, 1);
				printf("%s=%d\n", cmd_table[i].name, (int)bVal);
				break;				

			case WORD_T:
				memcpy(&wVal, ((unsigned char *)pmib)+cmd_table[i].offset, 2);
				printf("%s=%d\n", cmd_table[i].name, (int)wVal);
				break;				
			
			case INT_T:
			case INT_BIT_T:
				memcpy(&val, ((unsigned char *)pmib)+cmd_table[i].offset, 4);
				printf("%s=%d\n", cmd_table[i].name, val);
				break;				
				
			case STRING_T:
				ptr = ((unsigned char *)pmib)+cmd_table[i].offset;
				if (strlen(ptr) > 0)
					strcpy(tmpbuf, ptr);
				else
					tmpbuf[0] = '\0';
				printf("%s=%s\n", cmd_table[i].name, ptr);				
				break;				
			
			case BYTE_6_T:
				ptr = ((unsigned char *)pmib)+cmd_table[i].offset;
				printf("%s=%02x%02x%02x%02x%02x%02x\n", cmd_table[i].name, 
							*ptr, *(ptr+1),  *(ptr+2),  *(ptr+3),  *(ptr+4),  *(ptr+5));
				break;
				
			case BYTE_13_T:				
				ptr = ((unsigned char *)pmib)+cmd_table[i].offset;
				printf("%s=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
							cmd_table[i].name, 
							*ptr, *(ptr+1),  *(ptr+2),  *(ptr+3),  *(ptr+4),  *(ptr+5), *(ptr+6), 
							*(ptr+7),  *(ptr+8),  *(ptr+9),  *(ptr+10),  *(ptr+11), *(ptr+12));
				break;				
			
			default:
				printf("Invalid mib type!\n");
				return;
		}
		i++;
	}
}
#endif

/**
  *	access_mib	-access mib parameters
  *	@pmib: mib structure pointer
  *	@flag: ACCESS_MIB_SET, ACCESS_MIB_GET, 
  *				ACCESS_MIB_BY_NAME, ACCESS_MIB_BY_ID, ACCESS_MIB_ACTION
  *	@nameid: if ACCESS_MIB_BY_NAME, pointer to mib name
  *				if ACCESS_MIB_BY_ID, pointer to mib id
  *	@data1: if ACCESS_MIB_SET, set data of 1st argument
  *				if ACCESS_MIB_GET, read data for return
  *	@data2: if ACCESS_MIB_SET, set data of 2nd argument
  *			
  *
  *	Get/Set mib value from/to pmib structure.  
  *		Success: return 0 for ACCESS_MIB_SET command
  *						return read mib length for ACCESS_MIB_GET command
  *		Failure: return -1
  *		Action command: return 1 
  */
int access_mib(struct mib *pmib, int flag, char *nameid, void *data1, void *data2)
{
	int val, i = 0, ret = RET_OK;
#ifdef CMD_LINE	
	int j;
	char *ptr;
#endif	
	unsigned char bVal;
	unsigned short wVal;
	struct mdio_mem32_param	mem_param;
	void *data=NULL;

	if (nameid == NULL) {
		DEBUG_ERR("nameid == NULL!\n");
		return -1;
	}

	while (cmd_table[i].type != LAST_ENTRY) {
		if (
#ifdef CMD_LINE
			((flag & ACCESS_MIB_BY_NAME) && !strcmp(cmd_table[i].name, nameid)) ||
#endif
				 ((flag & ACCESS_MIB_BY_ID) && (cmd_table[i].id == *((int *)nameid)))) {	

			// Do ioctl
			if (cmd_table[i].action == ACT_IOCTL && (flag & ACCESS_MIB_ACTION)) {
				if (cmd_table[i].id == id_write_memory) {
#ifdef CMD_LINE					
					if 	(flag & ACCESS_MIB_BY_NAME) {					
						if (!data1 || !data2) {
							DEBUG_ERR("Invalid argument for id_write_memory!\n");
							return -RET_INVALID_ARG;
						}
						if (sscanf(data1, "%x", (unsigned int *)&mem_param.addr) != 1) {
							DEBUG_ERR("Invalid argument 1 for id_write_memory!\n");
							return -RET_INVALID_ARG;
						}							
						if (sscanf(data2, "%x", (unsigned int *)&mem_param.val) != 1) {
							DEBUG_ERR("Invalid argument 2 for id_write_memory!\n");
							return -RET_INVALID_ARG;
						}						
						DEBUG_OUT("addr=0x%x, value=0x%x\n", (unsigned int)mem_param.addr, (unsigned int)mem_param.val);
						data = &mem_param;	
					}
					else 
#endif				
					{
						memcpy((void *)&mem_param, data1, sizeof(struct mdio_mem32_param));											
						data = (void *)&mem_param;				
					}
				}				
				else if (cmd_table[i].id == id_read_memory) {
#ifdef CMD_LINE					
					if 	(flag & ACCESS_MIB_BY_NAME) {					
						if (!data1) {
							DEBUG_ERR("Invalid argument for id_read_memory!\n");
							return -RET_INVALID_ARG;
						}
						if (sscanf(data1, "%x", (unsigned int *)&mem_param.addr) != 1) {
							DEBUG_ERR("Invalid argument 1 for id_read_memory!\n");
							return -RET_INVALID_ARG;
						}							
						DEBUG_OUT("addr=0x%x\n", (unsigned int)mem_param.addr);						
						data = &mem_param.addr;			
					}
					else 
#endif
					{			
						memcpy((void *)&mem_param.addr, data1, sizeof(int));;
						data = (void *)&mem_param.addr;
					}											
				}
				else if (cmd_table[i].id == id_wlan_mac_addr) {
#ifdef ACCESS_WLAN_IF
					get_wlan_mac_addr(IF_WLAN, data1);
#else
					memset(data1, 0, 6);
#endif
					return 6; //length of mac address
				}
#ifdef RT_WLAN
				else if (cmd_table[i].id == id_get_wlan_info) {
			        return get_wlan_info(data1);
			    }
				else if (cmd_table[i].id == id_request_scan) {
			        return request_scan(data1);
			    }
				else if (cmd_table[i].id == id_get_scan_result) {
			        return get_scan_result(data1);
			    }
				else if (cmd_table[i].id == id_cfgwrite) {
			        return cfgwrite(data1);
			    }
			    else if (cmd_table[i].id == id_cfgread) {
			        return cfgread(data1);
			    }
				else if (cmd_table[i].id == id_priv_shortretry) {
			        return priv_retrylimit(data1, 1);
			    }
				else if (cmd_table[i].id == id_priv_longretry) {
			        return priv_retrylimit(data1, 0);
			    }
#endif
				else {
					DEBUG_ERR("Not supported now!\n");
					return -RET_NOT_SUPPORT_NOW;					
				}
				
				ret = do_mdio_ioctl(cmd_table[i].id, data);				
				
				if (ret > 0 && ((unsigned long)data) != ((unsigned long)data1) && 	 
												(flag & ACCESS_MIB_BY_ID))
					memcpy(data1, data, ret);
				
#ifdef CMD_LINE
				 if (ret >= 0 && cmd_table[i].id == id_read_memory && (flag & ACCESS_MIB_BY_NAME))
				 	printf("0x%s=0x%04x\n", (char *)data1, *((int *)data));
#endif				 
				 return ret;
			}

			// Do MIB R/W
			if  ((cmd_table[i].action == ACT_MIB_RW || cmd_table[i].action == ACT_MIB_RW_IOCTL) &&
						(flag & ACCESS_MIB_GET || flag & ACCESS_MIB_SET)) {
				switch (cmd_table[i].type) {
					case BYTE_T:
						if (flag & ACCESS_MIB_SET) {
#ifdef CMD_LINE							
							if 	(flag & ACCESS_MIB_BY_NAME)
								bVal = (unsigned char)atoi(data1);
							else
#endif								
								bVal = ((unsigned char *)data1)[0];
							if ((cmd_table[i].def=="" || (cmd_table[i].def && val != atoi(cmd_table[i].def))) &&
									(((int)bVal) < cmd_table[i].start || ((int)bVal) > cmd_table[i].end)) {
								DEBUG_ERR("Invalid BYTE_T cmd range [%d, %d, %d])!\n", bVal, cmd_table[i].start, cmd_table[i].end);
								return -RET_INVALID_RANGE;
							}						
							memcpy(((unsigned char *)pmib)+cmd_table[i].offset, &bVal, 1);							
						}
						else {
							memcpy(&bVal, ((unsigned char *)pmib)+cmd_table[i].offset, 1);			
#ifdef CMD_LINE														
							if (flag & ACCESS_MIB_BY_NAME)
								printf("%s=%d\n", cmd_table[i].name, (int)bVal);						
							else
#endif								
								memcpy(data1, &bVal, 1);	
							ret = 1;
						}					
						break;				
						
					case WORD_T:
						if (flag & ACCESS_MIB_SET) {			
#ifdef CMD_LINE
							if 	(flag & ACCESS_MIB_BY_NAME)						
								wVal = (unsigned short)atoi(data1);
							else
#endif		
								memcpy(&wVal, data1, 2);
							if ((cmd_table[i].def=="" || (cmd_table[i].def && val != atoi(cmd_table[i].def))) &&
									(((int)wVal) < cmd_table[i].start || ((int)wVal) > cmd_table[i].end)) {
								DEBUG_ERR("Invalid WORD_T cmd range [%d, %d, %d])!\n", val, cmd_table[i].start, cmd_table[i].end);
								return -RET_INVALID_RANGE;
							}						
							memcpy(((unsigned char *)pmib)+cmd_table[i].offset, &wVal, 2);
						}
						else {
							memcpy(&wVal, ((unsigned char *)pmib)+cmd_table[i].offset, 2);	
#ifdef CMD_LINE														
							if (flag & ACCESS_MIB_BY_NAME)
								printf("%s=%d\n", cmd_table[i].name, (int)wVal);	
							else
#endif								
								memcpy(data1, &wVal, 2);	
							ret = 2;
						}					
						break;				
								
					case INT_T:
					case INT_BIT_T:
						if (flag & ACCESS_MIB_SET) {					
#ifdef CMD_LINE
							if 	(flag & ACCESS_MIB_BY_NAME)							
								val = atoi(data1);
							else
#endif		
								memcpy(&val, data1, 4);							
							if ((cmd_table[i].def=="" || (cmd_table[i].def && val != atoi(cmd_table[i].def))) &&
									(val < cmd_table[i].start || val > cmd_table[i].end)) {
								DEBUG_ERR("Invalid INT_T cmd range [%d, %d, %d])!\n", val, cmd_table[i].start, cmd_table[i].end);
								return -RET_INVALID_RANGE;
							}						
							if ((cmd_table[i].type == INT_BIT_T) && is_more_bit_asserted(val)) {
								DEBUG_ERR("Invalid cmd range [%d, %d, %d])!\n", val, cmd_table[i].start, cmd_table[i].end);
								return -RET_INVALID_RANGE;
							}						
							memcpy(((unsigned char *)pmib)+cmd_table[i].offset, &val, 4);									
						}
						else {
							memcpy(&val, ((unsigned char *)pmib)+cmd_table[i].offset, 4);			
#ifdef CMD_LINE							
							if (flag & ACCESS_MIB_BY_NAME)
								printf("%s=%d\n", cmd_table[i].name, val);						
							else
#endif								
								memcpy(data1, &val, 4);	
							ret = 4;
						}					
						break;				
					
					case STRING_T:
						if (flag & ACCESS_MIB_SET) {
							if ((strlen(data1) > 0) && (strlen(data1) < cmd_table[i].start || strlen(data1) > cmd_table[i].end)) {
								DEBUG_ERR("Invalid STRINT_T cmd range [%d, %d, %d])!\n", strlen(data1), cmd_table[i].start, cmd_table[i].end);
								return -RET_INVALID_RANGE;
							}						
							strcpy(((unsigned char *)pmib)+cmd_table[i].offset, data1);
						}
						else {
#ifdef CMD_LINE
							if (flag & ACCESS_MIB_BY_NAME)
								printf("%s=\"%s\"\n", cmd_table[i].name, ((unsigned char *)pmib)+cmd_table[i].offset);	
							else
#endif
							{
								strcpy(data1, ((unsigned char *)pmib)+cmd_table[i].offset);	
								ret = strlen(data1);
							}
						}
						break;
				
					case BYTE_6_T:
					case BYTE_13_T:				
						if (flag & ACCESS_MIB_SET) {	
#ifdef CMD_LINE
							if (flag & ACCESS_MIB_BY_NAME) {						
								if (strlen(data1) != cmd_table[i].start*2) {
									DEBUG_ERR("Invalid BYTE cmd length [%d, %d])!\n", strlen(data1), cmd_table[i].start);
									return -RET_INVALID_RANGE;
								}						
								for (j=0, ptr=data1; *ptr && j<cmd_table[i].start; j++, ptr+=2) {
									if (!isxdigit((int)*ptr) || !isxdigit((int)*(ptr+1)) ) {
										DEBUG_ERR("%s: Invalid BYTE_T vlaue!\n", __FUNCTION__);
										return -RET_INVALID_RANGE;
									}				
									*(((unsigned char *)pmib)+cmd_table[i].offset+j) = convert_atob(ptr, 16);
								}		
							}
							else 
#endif
								memcpy(((unsigned char *)pmib)+cmd_table[i].offset, data1, cmd_table[i].start);
						}
						else {
#ifdef CMD_LINE							
							if (flag & ACCESS_MIB_BY_NAME) {
								ptr = ((unsigned char *)pmib)+cmd_table[i].offset;
								if (cmd_table[i].type ==BYTE_6_T)
									ret = printf("%s=%02x%02x%02x%02x%02x%02x\n", cmd_table[i].name, 
										*ptr, *(ptr+1),  *(ptr+2),  *(ptr+3),  *(ptr+4),  *(ptr+5));
								else							
									ret = printf("%s=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
											cmd_table[i].name,
											*ptr, *(ptr+1),  *(ptr+2),  *(ptr+3),  *(ptr+4),  *(ptr+5), *(ptr+6),
											*(ptr+7),  *(ptr+8),  *(ptr+9),  *(ptr+10),  *(ptr+11), *(ptr+12));
							}
							else 
#endif		
							{
								memcpy(data1, ((unsigned char *)pmib)+cmd_table[i].offset,  cmd_table[i].start);	
								ret =  cmd_table[i].start;	
							}
						}					
						break;
					
					default:
						DEBUG_ERR("Invalid mib type!\n");
						return -RET_NOT_NOW;
				}
			}

			// Do ioctl
			if (cmd_table[i].action == ACT_MIB_RW_IOCTL &&  (flag & ACCESS_MIB_ACTION)) {
				int ret1 = do_mdio_ioctl(cmd_table[i].id, ((unsigned char *)pmib)+cmd_table[i].offset);	
				if (ret1 != RET_OK)
					ret = ret1;
			}
			
			return ret;
		}
		i++;
	}

	DEBUG_ERR("Can't find mib!\n");	
	return -RET_INVALID_CMD_ID;
}

/*
 * 	Set intial value to mdio driver for specific action mib
 */
void set_init_mib_to_driver(struct mib *pmib, int action)
{
	int i=0;
	
	while (cmd_table[i].type != LAST_ENTRY) {
		if (cmd_table[i].action == action) 
			do_mdio_ioctl(cmd_table[i].id, ((unsigned char *)pmib)+cmd_table[i].offset);		
		i++;
	}
}


