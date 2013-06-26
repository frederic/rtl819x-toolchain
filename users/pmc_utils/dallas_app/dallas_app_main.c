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
# $RCSfile: dallas_app_main.c,v $
#
# $Date: 2009-01-14 06:02:04 $
#
# $Revision: 1.5 $
#-------------------------------------------------------------------------------
# Application for Dallas Controller.
#-------------------------------------------------------------------------------
*/


#include <unistd.h>
#include <string.h>
#include <errno.h> 
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "pmc_typs.h"
#include "dallas_app.h"
#include "dallas_app_util.h"
#include "dallas_app_cmd.h"

unsigned char dallas_image_file[128];
unsigned char dallas_nvdb_file[128];

static void usage(char * name)
{
	printf("Usage: %s COMMAND [ ADDRESS(hex) ] [ MASK(hex) ] [ LENGTH(decimal) ] [ VALUE(hex) ]\n", name);
	printf("\twhere COMMAND := {non_slave | slave | get | set | set_field | reset | branch | read_lag | write_lag | read_mac | write_mac}\n\n");
	
	printf("\tExample: %s non_slave IMAGE_FILE\n", name);
	printf("\t\tThis command will make Dallas running in non_slave mode. After performing this command, Apollo won¡¯t be able to communicate with Dallas before next reset.\n\n");

	printf("\tExample: %s non_slave_fpga IMAGE_FILE\n", name);
	printf("\t\tThis command will make Dallas FPGA board running in non_slave mode. After performing this command, Apollo won¡¯t be able to communicate with Dallas before next reset.\n\n");

	printf("\tExample: %s slave IMAGE_FILE\n", name);
	printf("\t\tThis command will initialize Dallas for running in slave mode.\n\n");
	
	printf("\tExample: %s get ADDRESS LENGTH\n", name);
	printf("\t\tGet the value in address ADDRESS on Dallas.\n\n");
	
	printf("\tExample: %s set ADDRESS LENGTH VALUE\n", name);
	printf("\t\tSet the value VALUE in address ADDRESS on Dallas.\n\n");
	
	printf("\tExample: %s set_field ADDRESS MASK VALUE\n", name);
	printf("\t\tSet value of a specific bit field(s) in address ADDRESS with mask MASK and value VALUE.\n\n");
	
	printf("\tExample: %s reset\n", name);
	printf("\t\tReset Dallas device.\n\n");
	
	printf("\tExample: %s branch ADDRESS\n", name);
	printf("\t\tMake Dallas jump and resume execution from the specified address ADDRESS.\n\n");

	printf("\tExample: %s read_lag ADDRESS\n", name);
	printf("\t\tRead data (32 bits) from the chosen LAG ADDRESS.\n\n");
	
	printf("\tExample: %s write_lag ADDRESS DATA\n", name);
	printf("\t\tWrite DATA (32 bits) to the chosen LAG ADDRESS.\n\n");
	
	printf("\tExample: %s read_mac ADDRESS\n", name);
	printf("\t\tRead data (32 bits) from the chosen ADDRESS of UNI MAC.\n\n");
	
	printf("\tExample: %s write_mac ADDRESS DATA\n", name);
	printf("\t\tWrite DATA (32 bits) to the chosen ADDRESS of UNI MAC.\n\n");
}


int main(int argc, char *argv[])
{
	int dallas_fd = -1;
	char *endptr;
	int i;
	int retval = 0;
	
	unsigned char flag;   /* opcode flags, if we need a ACK */
	unsigned long address;   /* target address */
	unsigned long mask;   /* mask for set field command */
	unsigned long length;   /* address length */
	unsigned long buff[PBRC_DATA_MAX_LENGTH];   /* data */
	unsigned char temp[16];
	
	dallas_fd = open(DALLAS_DEVICE, O_RDWR);
	if (dallas_fd < 0) {
		printf("Cannot open device %s.\n", DALLAS_DEVICE);
		exit(-1);
	}
	
	memset(buff, 0x00, sizeof(buff));
	if (argc < 2) {
		goto USAGE;
	} else if (!strcmp(argv[1], "non_slave")) {
		if(argc == 3) {
			strcpy(dallas_image_file, argv[2]);
		} else if(argc == 4) {
			strcpy(dallas_image_file, argv[2]);
			strcpy(dallas_nvdb_file, argv[3]);
		} else {
			goto USAGE;
		}
		retval = pmc_pbrc_non_slave(dallas_fd, MODE_NON_SLAVE);
		if ( 0 != retval) {
			printf("Failed to bring up non_slave mode!\n");
			close(dallas_fd);
			return -1;
		}
	} else if (!strcmp(argv[1], "non_slave_fpga")) {
		if(argc == 3) {
			strcpy(dallas_image_file, argv[2]);
		} else {
			goto USAGE;
		}
		retval = pmc_pbrc_non_slave(dallas_fd, MODE_NON_SLAVE_FPGA);
		if ( 0 != retval) {
			printf("Failed to bring up non_slave mode!\n");
		}
	} else if(!strcmp(argv[1], "slave")) {
		if (argc != 3) {
			goto USAGE;
		}
		strcpy(dallas_image_file, argv[2]);
		retval = pmc_pbrc_slave(dallas_fd);
		if ( 0 != retval) {
			printf("Failed to bring up slave mode!\n");
		}
	} else if(!strcmp(argv[1], "get")) {
		if (argc != 4) {
			goto USAGE;
		}
		flag = OPCODE_FLAG_ACK_REQ;
		address = strtoul(argv[2], &endptr, 16);
		length = strtoul(argv[3], &endptr, 10);
		pmc_pbrc_get_debug(dallas_fd, address, length);
	} else if(!strcmp(argv[1], "set")) {
		if (argc != 5) {
			goto USAGE;
		}
		length = strtoul(argv[3], &endptr, 10);
		if ((length != 4) && ((length * 2) != strlen(argv[4]))) {
			printf("Invalid value, value length should be length*2!\n\n");
			goto USAGE;
		}
		flag = OPCODE_FLAG_ACK_REQ;
		address = strtoul(argv[2], &endptr, 16);
		if (length == 4) {
			buff[0] = strtoul(argv[4], &endptr, 16);
		} else {
			for (i = 0; i < length/4 ; i++) {
				memset(temp, 0, sizeof(temp));
				memcpy(temp, argv[4] + 8 * i, 8);
				buff[i] = strtoul(temp, &endptr, 16);
			}
		}
		pmc_pbrc_set_debug(dallas_fd, address, length, buff);
	} else if(!strcmp(argv[1], "set_field")) {
		if (argc != 5) {
			goto USAGE;
		}
		flag = OPCODE_FLAG_ACK_REQ;
		address = strtoul(argv[2], &endptr, 16);
		mask = strtoul(argv[3], &endptr, 16);
		buff[0] = strtoul(argv[4], &endptr, 16);
		pmc_pbrc_set_field_debug(dallas_fd, address, mask, buff);
	} else if(!strcmp(argv[1], "reset")) {
		if (argc != 2) {
			goto USAGE;
		}
		pmc_pbrc_reset_debug(dallas_fd);
	} else if(!strcmp(argv[1], "branch")) {
		if (argc != 3) {
			goto USAGE;
		}
		flag = OPCODE_FLAG_ACK_REQ;
		address = strtoul(argv[2], &endptr, 16);
		pmc_pbrc_branch_debug(dallas_fd, address);
	}else if(!strcmp(argv[1], "read_lag")) {
		if (argc != 3) {
			goto USAGE;
		}
		flag = OPCODE_FLAG_ACK_REQ;
		address = strtoul(argv[2], &endptr, 16);
		pmc_pbrc_read_lag_debug(dallas_fd, address);
	} else if(!strcmp(argv[1], "write_lag")) {
		if (argc != 4) {
			goto USAGE;
		}
		flag = OPCODE_FLAG_ACK_REQ;
		address = strtoul(argv[2], &endptr, 16);
		buff[0] = strtoul(argv[3], &endptr, 16);
		pmc_pbrc_write_lag_debug(dallas_fd, address, buff);
	} else if(!strcmp(argv[1], "read_mac")) {
		if (argc != 3) {
			goto USAGE;
		}
		flag = OPCODE_FLAG_ACK_REQ;
		address = strtoul(argv[2], &endptr, 16);
		pmc_pbrc_read_uni_mac_debug(dallas_fd, address);
	} else if(!strcmp(argv[1], "write_mac")) {
		if (argc != 4) {
			goto USAGE;
		}
		flag = OPCODE_FLAG_ACK_REQ;
		address = strtoul(argv[2], &endptr, 16);
		buff[0] = strtoul(argv[3], &endptr, 16);
		pmc_pbrc_write_uni_mac_debug(dallas_fd, address, buff);
	} else if(!strcmp(argv[1], "init")) {
		if (argc != 3) {
			goto USAGE;
		}
		if (!strcmp(argv[2], "slave")) {
			pmc_pbrc_init_debug(dallas_fd, MODE_SLAVE);
		} else {
			pmc_pbrc_init_debug(dallas_fd, MODE_NON_SLAVE);
		}
	} else if(!strcmp(argv[1], "irq")) {
		if (argc != 2) {
			goto USAGE;
		}
		pmc_pbrc_dallas_irq(dallas_fd);
	} else {
		goto USAGE;
	}
	close(dallas_fd);
	return 0;
	
USAGE:
	usage(argv[0]);
	close(dallas_fd);
	return 0;
}



