/*
 * IOH header
 * Copyright (C)2010, Realtek Semiconductor Corp. All rights reserved
 */


#ifndef __IOH_H
#define __IOH_H

#include <netinet/in.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>

#define ETH_MAC_LEN        ETH_ALEN      /* Octets in one ethernet addr   */
#define ETH_HEADER_LEN     ETH_HLEN      /* Total octets in header.       */
#define ETH_MIN_FRAME_LEN  ETH_ZLEN      /* Min. octets in frame sans FCS */
#define ETH_USER_DATA_LEN  ETH_DATA_LEN  /* Max. octets in payload        */
#define ETH_MAX_FRAME_LEN  ETH_FRAME_LEN /* Max. octets in frame sans FCS */

#define ETH_FRAME_TOTALLEN 1518          /*Header: 14 + User Data: 1500 FCS: 4*/

#define ETH_P_RTK		0x8899	// Realtek Remote Control Protocol (RRCP)
#define RRCP_P_IOH		0x38	// RRCP IOH

// ioh_ctrl_type
enum {
	IOH_CMD_STR = 0,
	IOH_CMD_STR_ACK,
	IOH_CMD_STR_RESP,
	IOH_CMD_TEST
};

// ioh_ctrl_frag
#define IOH_FRAG_EOF		0x80
#define IOH_FRAG_SEQ_MASK	0x7f

struct ioh_header {
	unsigned char da[ETH_ALEN];
	unsigned char sa[ETH_ALEN];
	unsigned short eth_type;	// should be ETH_P_RTK 
	unsigned char rrcp_type;	// should be RRCP_P_IOH
	unsigned char ioh_cmd;
	unsigned short ioh_seq;
	unsigned char ioh_frag;
	unsigned char ioh_reserved[3];
	unsigned short ioh_data_len;
} __attribute__((packed));

#define BUF_SIZE ETH_FRAME_TOTALLEN

struct ioh_class {
	int sockfd;
	char dev[64];
	unsigned char src_mac[ETH_MAC_LEN];
	unsigned char dest_mac[ETH_MAC_LEN];
	struct sockaddr_ll socket_address;
	char tx_buffer[BUF_SIZE];
	char rx_buffer[BUF_SIZE];
	struct ioh_header *tx_header;
	struct ioh_header *rx_header;
	unsigned char *tx_data;
	unsigned char *rx_data;
	int debug;
};

int ioh_open(struct ioh_class *obj, char *dev, char *da, int debug);
int ioh_close(struct ioh_class *obj);
int ioh_send(struct ioh_class *obj, unsigned char *da);
int ioh_recv(struct ioh_class *obj, int timeout_ms);

#define mac2str(mac, str) bin2hex((mac), (str), 6)
#define str2mac(str, mac) hex2bin((str), (mac), 6)

int bin2hex(const unsigned char *bin, char *hex, const int len);
int hex2bin(const char *hex, unsigned char *bin, const int len);
void hex_dump(void *data, int size);

#endif
