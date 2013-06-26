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
# $RCSfile: pmc_typs.h,v $
#
# $Date: 2009-01-14 06:02:04 $
#
# $Revision: 1.5 $
#-------------------------------------------------------------------------------
# Dallas Controller common headfile.
#-------------------------------------------------------------------------------
*/


#ifndef _PMC_TYPS_H
#define _PMC_TYPS_H

//#define	DALLAS_GPIO_RESET	(26)
//in rtk pmc board, the reset pin is C3
//A0=0, A7=7, B0=8, B7=15, C0=16, C1=17, C2=18, C3=19
#define	DALLAS_GPIO_RESET	(19)
#define	DALLAS_GPIO_IRQ		(16)

#define PBRC_GET 		0x01
#define PBRC_SET 		0x02
#define PBRC_SET_FIELD		0x03
#define PBRC_BRANCH		0x04
#define PBRC_RESET		0x05
#define PBRC_READ_UNI_MAC	0x06
#define PBRC_WRITE_UNI_MAC	0x07
#define PBRC_READ_LAG		0x08
#define PBRC_WRITE_LAG		0x09
#define MODE_NON_SLAVE_FPGA	0x10
#define MODE_NON_SLAVE		0x11
#define MODE_SLAVE		0x12
#define DALLAS_IRQ		0x20
#define DALLAS_RPC		0x21
#define PBRC_EXIT		0x99

#define OPCODE_FLAG_ACK_REQ 0x80   /* Acknowledge should be returned on this command */
#define OPCODE_FLAG_ACK 0x40   /* This is an acknowledgment on the last command */
#define OPCODE_FLAG_ERR 0x20   /* Error occurred while processing command.*/

#define DALLAS_START_ADDRESS 0xC0000000
#define DALLAS_START_ADDRESS_FPGA 0x20000000
#define DALLAS_NVDB_ADDRESS 0x40001000

#define PBRC_MAGIC_NUMBER	0x50425243
#define RPC_MAGIC_NUMBER	0x52504300

#define COMPRESSED_IMAGE_HEADER_SIZE 272

/*Constants and enums*/
#define PBRC_MAX_FRAME_ID 65535
#define PBRC_ETHER_TYPE_TX 0x1200
#define PBRC_ETHER_TYPE_RX 0x4000

#define PBRC_TIMEOUT_NO_WAIT    0
#define PBRC_TIMEOUT_UNIT       50
#define PBRC_SOCKET_TIME        999999
#define PBRC_CMD_TIMEOUT_FLAG   0x10000000  /*this is used together with the PBRC_CMD enum, to indicate
                                             *whether this ACK frame of the command is timeout or not*/
#define PBRC_CMD_ACKLOST_FLAG   0x20000000

#define PBRC_OPCODE_FLAG_MASK   0x000000FF  /*0x   AA      BB      CC      DD
                                             * |cmd type| id_hi | id_lo | flag |*/
#define PBRC_FRAMEID_MASK       0x00FFFF00
#define PBRC_OPCODE_FLAG_ERR    0x20000000
#define PBRC_OPCODE_FLAG_ACK    0x40000000

#define PBRC_QUEUE_MAX_LENGTH   1000

#define PBRC_PACKET_TIMEOUT     200  /* general timeout for packets, msec  */

/* register indirect access */
#define STANDARD_HW_REGION_SIZE             0x800
#define PAS7401_UNI_MAC_LAG_DOMAIN_OFFSET   (0x5C000000)
#define PAS7401_UNI_MAC_LAG_BASE            (PAS7401_UNI_MAC_LAG_DOMAIN_OFFSET)
#define LAG_CONFIG_BASE_ADDR                (PAS7401_UNI_MAC_LAG_BASE + 0x00800)
#define LAG_SIZE                            STANDARD_HW_REGION_SIZE

#define SLOW_MEM_BASE				0x60000000	/* Slow memory (normal clock) space range base */
#define PAS7401_CORE_DOMAIN_OFFSET		SLOW_MEM_BASE
#define PAS7401_UNI_CORE_BASE			(PAS7401_CORE_DOMAIN_OFFSET)
#define GPON_TOP_UNI_BASE_ADDR			(PAS7401_UNI_CORE_BASE    + 0x13000)
#define ETH_reg_mnem_REGS_BASE_ADDR		GPON_TOP_UNI_BASE_ADDR
#define ETH_reg_mnem_M_PM_83_STATUS		(ETH_reg_mnem_REGS_BASE_ADDR + 0x4)

#define ETH_reg_mnem_REGS_BASE_ADDR                         GPON_TOP_UNI_BASE_ADDR
#define ETH_reg_mnem_M_LAG_CONFIGURATION                      (ETH_reg_mnem_REGS_BASE_ADDR + 0xac)
#define ETH_reg_mnem_M_WR_DATA_LAG_CONFIGURATION              (ETH_reg_mnem_REGS_BASE_ADDR + 0xa8)

#define ETH_reg_mnem_M_41_DATA                                (ETH_reg_mnem_REGS_BASE_ADDR + 0x10)

#define ETH_reg_mnem_WRITE_LAG_SC_OFFSET                       16
#define ETH_reg_mnem_READ_LAG_SC_OFFSET                        17

#define LAG_ADDR_OFFSET_MASK                   0x3FF

#define ETH_reg_mnem_READ_MAC_SC_OFFSET                        17
#define ETH_reg_mnem_WRITE_MAC_SC_OFFSET                       16
#define UNI_MAC_ADDR_OFFSET_MASK                           0x7FF
#define UNI_MAC_SIZE                        0x400
#define UNI_0_CONFIG_BASE_ADDR              (PAS7401_UNI_MAC_LAG_BASE + 0x00000)
#define UNI_1_CONFIG_BASE_ADDR              (PAS7401_UNI_MAC_LAG_BASE + UNI_MAC_SIZE)

#define ETH_reg_mnem_M_WR_DATA_MAC_CONFIGURATION              (ETH_reg_mnem_REGS_BASE_ADDR + 0xa0)
#define ETH_reg_mnem_M_MAC_CONFIGURATION                      (ETH_reg_mnem_REGS_BASE_ADDR + 0xa4)
#define ETH_reg_mnem_M_79_DATA                                (ETH_reg_mnem_REGS_BASE_ADDR + 0xc)

#define MA_MASK( bits )  ((unsigned long) ((1UL << (bits)) - 1))
#define MA_GENERATE_FIELD( val, field_shift, field_bits )         \
	( (((unsigned long) (val)) & MA_MASK( field_bits )) << (field_shift) )
#define MA_GENERATE_BIT( val, bit )  MA_GENERATE_FIELD( val, bit, 1 )
/* register indirect access */

enum {
	PBRC_OK = 0,
	PBRC_E_INIT,
	PBRC_E_SOCKET,
	PBRC_E_TIMEOUT,
	PBRC_E_UNKNOWN,
	PBRC_E_INVALID_CMD,
	PBRC_E_ADDRESS,
	PBRC_E_LENGTH
};

enum {
	PBRC_CMD_RPC=0x0,
	PBRC_CMD_GET=0x00000001,
	PBRC_CMD_SET=0x00000002,
	PBRC_CMD_SET_FIELD=0x00000003,
	PBRC_CMD_BRANCH=0x00000004
};

#define PBRC_DATA_MAX_LENGTH	369 /* 1476/4 = 369 */

struct pbrc_cmd {
	unsigned char command;   /* opcode command, get/set/set field/branch */
	unsigned char flag;   /* opcode flags, do we need an ACK? */
	unsigned short error_code;   /* error code, for acknowledgment frame */
	unsigned long address;   /* target address */
	unsigned long mask;   /* mask for set field command */
	unsigned long length;   /* address length */
	unsigned long buff[PBRC_DATA_MAX_LENGTH];   /* data */
};

/* burst read/write */
#define DISR_MAX_WRITE_LIST		180
#define DISR_MAX_READ_LIST		248

#define DALLAS_ISR_CODE_START_ADDRESS			0x1000
#define DALLAS_WRITE_READ_LIST_START_ADDRESS		0x1420
#define DALLAS_START_READ_OUTPUT_DATA_ADDRESS		0x1800
#define DALLAS_START_INTERRUPT_OUTPUT_DATA_ADDRESS	0x1be0
#define DALLAS_ENABLE_INTERRUPT_ADDRESS			0x1304
#define DALLAS_DISABLE_INTERRUPT_ADDRESS		0x1334

#define DALLAS_M_IRQ_MASK			0x50000014
#define DALLAS_M_GPIO_RGF_IRQ_MASK		0x50000054 
#define DALLAS_M_IRQ_MASK_VALUE			0xFFDFFFFF
#define DALLAS_M_GPIO_RGF_IRQ_MASK_VALUE	0x00FFFFFD

typedef struct {
	unsigned int reg_irq_mask_value;
	unsigned int reg_irq_pending_value; 
	unsigned int  reg_gmac_pending_value;
	unsigned int reg_gmac_clear_value;
	unsigned int reg_gmac_mask_value;
} DISR_ICU_regs; 

/* register pair: address + value */
typedef struct {
	unsigned int reg_address;   /* register physical address */
	unsigned int reg_value;       /* register value  */
}  DISR_register_address_value_list;

/* the write list data structure */
typedef struct {
	DISR_ICU_regs                              icu_regs;
	unsigned int                                    us_ploam[3]; 
	/* if there are no hw register to update, write a NULL pointer in list[0]  */
	DISR_register_address_value_list list[DISR_MAX_WRITE_LIST];  /* null terminated list of registers to update */
} DISR_burst_write_list;        


typedef struct {
	unsigned int list[DISR_MAX_READ_LIST];  /* null terminated list of register addresses to read */
} DISR_burst_read_list;


/*
request_operation field:
     bit 0每2: indicates the command operation (nop - b000, write 每 b001, read 每 b010)
     bit 3: indicates if ISR should write a US_Ploam (0 - nop, 1 每 write PLOAM)
     bit 4: indicates if ISR should write ICU registers (0 - nop, 1 每 write ICU registers)
     bit 5: indicates if Apollo has ended processing the interrupt. (0 每 no, 1 每 yes) 

*/
typedef struct {
	unsigned int request_operation;     /* Bit fields which identify the operation for Dallas. */
	union 
	{
		DISR_burst_write_list	burst_write_list;
		DISR_burst_read_list	burst_read_list;
	};
} DISR_burst_read_write_input_data;


typedef struct {
	unsigned int data[DISR_MAX_READ_LIST];  /* register values (that were read from HW) */
} DISR_burst_read_output_data;

/* RPC */
#define RPC_S_TIMEOUT	19

typedef struct {
	unsigned long ioctrl_number;
	unsigned long legal_size;
	long status;
	unsigned long data[370];	/* 1480/4 = 370 */
} dallas_rpc;

/* end of RPC */

/*nvdb*/

typedef struct 
{
 int sram_magic_number;	/*0-3*/
 int ram_size;					/*4-7*/
 int mac_pon_addr[2];		/*8-15, index is 2*/ 
 int ip_pon_addr;				/*16-19, index is 3*/
 int net_pon_mask;			/*20-23*/
 int mac_uni0_addr[2];	/*24-31, index is 5*/
 int ip_uni0_addr;			/*32-35, index is 6*/
 int net_uni0_mask;			/*36-39*/
 int mac_uni1_addr[2];	/*40-47, index is 8*/
 int ip_uni1_addr;			/*48-51, index is 9*/
 int net_uni1_mask;			/*52-55*/
 int uni0_bridge_enable;
 int uni1_bridge_enable;
 int uni0_autoneg_enable;
 int uni1_autoneg_enable;
 int uni0_master_mode;
 int uni1_master_mode;
 int uni0_advertise_1000t_multi_port;
 int uni1_advertise_1000t_multi_port;
 int uni0_advertise_1000t_full_duplex;
 int uni1_advertise_1000t_full_duplex;
 int uni0_advertise_pause_asymetric ;
 int uni1_advertise_pause_asymetric;
 int uni0_advertise_pause_enabled;
 int uni1_advertise_pause_enabled;
 int uni0_advertise_100tx_fd;
 int uni1_advertise_100tx_fd;
 int uni0_advertise_100tx_hd;
 int uni1_advertise_100tx_hd;
 int uni0_advertise_10tx_fd;
 int uni1_advertise_10tx_fd;
 int uni0_advertise_10tx_hd;
 int uni1_advertise_10tx_hd;
 int remote_host; 
}dallas_nvdb; /*end of nvdb*/

#endif /* end _PMC_TYPS_h */

