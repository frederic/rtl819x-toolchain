/*
#*******************************************************************************
# Copyright (C) 2006 PMC-Sierra Inc.  All Rights Reserved.
#-------------------------------------------------------------------------------
# This software embodies materials and concepts which are proprietary and
# confidential to PMC-Sierra, Inc.  PMC-Sierra distributes this software to
# its customers pursuant to the terms and conditions of the Software License
# Agreement contained in the text file software.lic that is distributed along
# with the software.  This software can only be utilized if all terms and
# conditions of the Software License Agreement are accepted.  If there are
# any questions, concerns, or if the Software License Agreement text file
# software.lic is missing, please contact PMC-Sierra for assistance.
#-------------------------------------------------------------------------------
# $RCSfile: dallas_app_util.c,v $
#
# $Date: 2009-01-14 06:02:04 $
#
# $Revision: 1.4 $
#-------------------------------------------------------------------------------
# Utility for Dallas Controller application. 
#-------------------------------------------------------------------------------
*/

#include <unistd.h>
#include <string.h>
#include <errno.h> 
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <ctype.h>

#include "pmc_typs.h"
#include "dallas_app.h"
#include "dallas_app_util.h"
#include "dallas_app_cmd.h"

/*Constants*/
#define NVDB_NUM 34

enum
{
	TYPE_NUM=0,
	TYPE_IP,
	TYPE_MAC,
	TYPE_MGC,
};

typedef struct
{
	short len_dword;
	short type;
	char  name[40];	
} t_nvdb_struct;

t_nvdb_struct nvdb_param[NVDB_NUM]=
{
 { 1, TYPE_MGC, "sram_magic_number=",        },
 { 1, TYPE_NUM, "ram_size=",                        },
 { 2, TYPE_MAC, "mac_pon_addr=",                    },
 { 1, TYPE_IP,  "ip_pon_addr=",                     },
 { 1, TYPE_IP,  "net_pon_mask=",                    },
 { 2, TYPE_MAC, "mac_uni0_addr=",                   },
 { 1, TYPE_IP,  "ip_uni0_addr=",                    },
 { 1, TYPE_IP,  "net_uni0_mask=",                   },
 { 2, TYPE_MAC, "mac_uni1_addr=",                   },
 { 1, TYPE_IP,  "ip_uni1_addr=",                    },
 { 1, TYPE_IP,  "net_uni1_mask=",                   },
 { 1, TYPE_NUM, "uni0_bridge_enable=",              },
 { 1, TYPE_NUM, "uni1_bridge_enable=",              },
 { 1, TYPE_NUM, "uni0_autoneg_enable=",             },
 { 1, TYPE_NUM, "uni1_autoneg_enable=",             },
 { 1, TYPE_NUM, "uni0_master_mode=",                },
 { 1, TYPE_NUM, "uni1_master_mode=",                },
 { 1, TYPE_NUM, "uni0_advertise_1000t_multi_port=", },
 { 1, TYPE_NUM, "uni1_advertise_1000t_multi_port=", },
 { 1, TYPE_NUM, "uni0_advertise_1000t_full_duplex=",},
 { 1, TYPE_NUM, "uni1_advertise_1000t_full_duplex=",},
 { 1, TYPE_NUM, "uni0_advertise_pause_asymetric=",  },
 { 1, TYPE_NUM, "uni1_advertise_pause_asymetric=",  },
 { 1, TYPE_NUM, "uni0_advertise_pause_enabled=",    },
 { 1, TYPE_NUM, "uni1_advertise_pause_enabled=",    },
 { 1, TYPE_NUM, "uni0_advertise_100tx_fd=",         },
 { 1, TYPE_NUM, "uni1_advertise_100tx_fd=",         },
 { 1, TYPE_NUM, "uni0_advertise_100tx_hd=",         },
 { 1, TYPE_NUM, "uni1_advertise_100tx_hd=",         },
 { 1, TYPE_NUM, "uni0_advertise_10tx_fd=",          },
 { 1, TYPE_NUM, "uni1_advertise_10tx_fd=",          },
 { 1, TYPE_NUM, "uni0_advertise_10tx_hd=",          },
 { 1, TYPE_NUM, "uni1_advertise_10tx_hd=",          },
 { 1, TYPE_NUM, "remote_host="                      },
};

/*Functions*/
unsigned long dallas_convert_endian(unsigned long value)
{
	return (((value & 0xFF000000) >> 24) + ((value & 0x00FF0000) >> 8) + ((value & 0x0000FF00) << 8) + ((value & 0x000000FF) << 24));
}

/* Show the reueset command information. */
void dallas_show_request(struct pbrc_cmd * pbrc)
{
	int i;
	unsigned char * ptr;
	int length = 0;
	
	printf("\nRequest information:\n");
	switch (pbrc->command) {
	case PBRC_GET:
		printf("Command: get\n");
		printf("Length: %ld\n", pbrc->length);
		break;
	case PBRC_SET:
		printf("Command: set\n");
		printf("Length: %ld\n", pbrc->length);
		if (pbrc->length > (PBRC_DATA_MAX_LENGTH * 4)) {
			length = PBRC_DATA_MAX_LENGTH * 4;
		} else {
			length = pbrc->length;
		}
		printf("Value: \n");
		for(i = 0; i < length/4; i++) {
			printf("0x%08lx:  0x%08lx\n", pbrc->address + i * 4, pbrc->buff[i]);
		}
		printf("\n");
		break;
	case PBRC_SET_FIELD:
		printf("Command: set_field\n");
		printf("Mask:\n  0x%08lx\n", pbrc->mask);
		printf("Value:\n  0x%08lx\n", pbrc->buff[0]);
		break;
	case PBRC_READ_LAG:
		printf("Command: read_lag\n");
		break;
	case PBRC_WRITE_LAG:
		printf("Command: write_lag\n");
		printf("Value: \n");
		printf("  0x%08lx\n", pbrc->buff[0]);
		break;
	case PBRC_READ_UNI_MAC:
		printf("Command: read_mac\n");
		break;
	case PBRC_WRITE_UNI_MAC:
		printf("Command: write_mac\n");
		printf("Value: \n");
		printf("  0x%08lx\n", pbrc->buff[0]);
		break;
	case PBRC_BRANCH:
		printf("Command: branch\n");
		break;
	default:
		printf("In dallas_show_request, unknown cmd.\n");
		break;
	}
	printf("Address: 0x%08lx\n\n", pbrc->address);
}

/* Show the reply information. */
void dallas_show_reply(struct pbrc_cmd * pbrc)
{
	int i;
	int length = 0;

	printf("\nReply information:\n");
	switch (pbrc->command) {
	case PBRC_GET:
		printf("Command: get\n");
		break;
	case PBRC_SET:
		printf("Command: set\n");
		break;
	case PBRC_SET_FIELD:
		printf("Command: set_field\n");
		break;
	case PBRC_BRANCH:
		printf("Command: branch\n");
		break;
	case PBRC_READ_LAG:
		printf("Command: read_lag\n");
		break;
	case PBRC_WRITE_LAG:
		printf("Command: write_lag\n");
		break;
	case PBRC_READ_UNI_MAC:
		printf("Command: read_mac\n");
		break;
	case PBRC_WRITE_UNI_MAC:
		printf("Command: write_mac\n");
		break;
	default:
		printf("In dallas_show_reply, unknown cmd.\n");
		break;
	}
	
	switch (pbrc->error_code) {
	case PBRC_E_INVALID_CMD:
		printf("Error: Unknown opcode value\n");
		break;
	case PBRC_E_ADDRESS:
		printf("Error: Address unaligned or out of range\n");
		break;
	case PBRC_E_LENGTH:
		printf("Error: Length unaligned or larger than max size\n");
		break;
	case PBRC_E_TIMEOUT:
		printf("Error: Receive timeout\n");
		break;
	case PBRC_E_SOCKET:
		printf("Error: snd&rcv error\n");
		break;
	case PBRC_OK:
		printf("Status: success\n");
		if((pbrc->command == PBRC_GET) || (pbrc->command == PBRC_READ_LAG)
			 || (pbrc->command == PBRC_READ_UNI_MAC)) {
			if (pbrc->length > (PBRC_DATA_MAX_LENGTH * 4)) {
				length = PBRC_DATA_MAX_LENGTH * 4;
			} else {
				length = pbrc->length;
			}
			if ((pbrc->command == PBRC_READ_LAG) || (pbrc->command == PBRC_READ_UNI_MAC)) {
				length = 4;
			}
			printf("Value: \n");
			for(i = 0; i < length/4; i++) {
				printf("0x%08lx:  0x%08lx\n", pbrc->address + i * 4, pbrc->buff[i]);
			}
			printf("\n");
		}
		break;
	default:
		printf("Error: Unknown error!\n");
		break;
	}
	printf("Address: 0x%08lx\n\n", pbrc->address);
}


int chex2dec(char *in_hex, int *out_dec, int len)
{
	int tmp=0, i;
	
	if(len>10) {
		printf("Error: input hex larger than 32 bit, can not convert. len=%d\n",len);
		return -1;
	}
	
	for(i=0; i<len; i++) {
		if((in_hex[i]>='0')&&(in_hex[i]<='9')) {
			tmp=(tmp*16)+(int)(in_hex[i]-'0');
		}	else if((in_hex[i]>='a')&&(in_hex[i]<='f')) {
			tmp=(tmp*16)+(int)(in_hex[i]-'a'+10);
		} else if((in_hex[i]>='A')&&(in_hex[i]<='F')) {
			tmp=(tmp*16)+(int)(in_hex[i]-'A'+10);
		} else {
			printf("Error: invalid character found in the input hex, can not convert.\n");
			return -1;
		}
	}
	
	*out_dec=tmp;
	return 0;
}

int cdec2ip(char *in_dec, int out_ip[], int len)
{
	int tmp_index, i, tmp;
	if(len>17) {
		printf("Error: input string larger than 15 characters, can not convert. len=%d\n",len);
		return -1;
	}
	tmp_index=0;
	for(i=0; i<4; i++) {
		tmp=atoi(&in_dec[tmp_index]);
		tmp_index=(tmp>9)?(tmp_index+3):(tmp_index+2);
		tmp_index=(tmp>99)?(tmp_index+1):tmp_index;
		out_ip[i]=tmp;
	}
	return 0;
}

/*return:
 *	0--successful
 */
int dallas_get_nvdb_file(dallas_nvdb *p_nvdb, FILE *p_fd)
{
	int i;
	char string[256];
	char param[256];
	int name_len, total_len;
	int tmp, tmp_conv[6];
	unsigned char *p_strmem;
	int return_no=0, error_occur=0;
	int srch_id=0, start_dword;
	
	p_strmem = (unsigned char *)p_nvdb;
	while( fgets(string, sizeof(string),p_fd) ) {		
		/*Process a line of the file*/
		for(i=0; i<NVDB_NUM; i++) {
			if(strncmp(string,nvdb_param[srch_id].name, strlen(nvdb_param[srch_id].name))==0) {
				break;
			}
			srch_id++;
			srch_id%=NVDB_NUM;			
		}
		
		if(strncmp(string,nvdb_param[srch_id].name, strlen(nvdb_param[srch_id].name))!=0)
			continue;
			
		name_len = strlen(nvdb_param[srch_id].name);
		total_len = strlen(string);
		memcpy(param, &string[name_len], (total_len-name_len));
		
		start_dword=0;
		for(i=0; i<srch_id; i++)
			start_dword+=nvdb_param[i].len_dword;
		
		switch(nvdb_param[srch_id].type) {
			case TYPE_NUM:
				tmp = atoi(param);
				memcpy(&p_strmem[start_dword*4], (unsigned char *)&tmp, sizeof(tmp));		
				break;
			case TYPE_IP:
				cdec2ip(param, (int *)tmp_conv, (total_len-name_len));
				for(i=0; i<4; i++) {
					p_strmem[start_dword*4+i]=(unsigned char)tmp_conv[i];
				}
				break;
			case TYPE_MAC:
				for(i=0; i<6; i++) {
					if(chex2dec(&param[i*3],&tmp_conv[i], 2)<0) {
						printf("Parameter %x chex2dec return error.\n", nvdb_param[srch_id].name);
						error_occur=1;
					}
				}
				if(error_occur==0) {
					p_strmem[start_dword*4+3]=(unsigned char)tmp_conv[0];
					p_strmem[start_dword*4+2]=(unsigned char)tmp_conv[1];
					p_strmem[start_dword*4+1]=(unsigned char)tmp_conv[2];
					p_strmem[start_dword*4]=(unsigned char)tmp_conv[3];
					p_strmem[start_dword*4+7]=(unsigned char)tmp_conv[4];
					p_strmem[start_dword*4+6]=(unsigned char)tmp_conv[5];
				} else {
					error_occur=0;
				}
				break;
			case TYPE_MGC: 
				if(chex2dec(&param[2],&tmp, 8)<0) {
					printf("chex2dec return error.\n");
				} else {
					memcpy(&p_strmem[start_dword*4], (unsigned char *)&tmp, sizeof(tmp));		
				}
				break;
			default:
				printf("Not valid parameter. Can not convert.\n");
				break;
		}

		memset(string, 0, 256);
		memset(param, 0, 256);
	}
		
	return return_no;
}
