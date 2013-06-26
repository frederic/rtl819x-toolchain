/*
 * IOH daemon
 * Copyright (C)2010, Realtek Semiconductor Corp. All rights reserved
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "ioh.h"

static void (*cmd_process_fn)(unsigned char *, int) = NULL; 

static struct ioh_class __ioh_obj = {
	sockfd: 0,
	dev: {0},
	src_mac: {0},
	dest_mac: {0},
	socket_address: {0},
	tx_buffer: {0},
	rx_buffer: {0},
	tx_header: NULL,
	rx_header: NULL,
	tx_data: NULL,
	rx_data: NULL,
	debug: 0,
};


void inband_open(char *inband_if)
{
	struct ioh_class *obj = &__ioh_obj;

	ioh_open(obj, inband_if, NULL, 1);	
}

#if 0
void init_inband_if(char *inband_if,char *inband_mac)
{
	char buf[50];
	sprintf(buf,"ifconfig %s hw ether %s up",inband_if,inband_mac);
	system(buf);		
}
#endif
void inband_wait_event(void *process_fn)
{  
	struct ioh_class *obj = &__ioh_obj;
	int rx_len;

	cmd_process_fn = (void *)process_fn;	 

	rx_len = ioh_recv(obj, -1);
	if (rx_len < 0)
	{
		perror("ioh_recv:");
		exit(1);
       }	
	if (obj->rx_header->eth_type != ntohs(ETH_P_RTK) ||
		obj->rx_header->rrcp_type != RRCP_P_IOH)
		return;

	if (rx_len != ntohs(obj->rx_header->ioh_data_len) + sizeof(*obj->rx_header))
	{
		if (ntohs(obj->rx_header->ioh_data_len) + sizeof(*obj->rx_header) < 
			ETH_MIN_FRAME_LEN && rx_len == ETH_MIN_FRAME_LEN)
			{
				// its ok for min ethernet packet padding
			}
			else
			{
				printf("%s: rx len (%d) != %d\n", __FUNCTION__,
					rx_len, ntohs(obj->rx_header->ioh_data_len) + sizeof(*obj->rx_header));
				return;
			}
	}

	switch (obj->rx_header->ioh_cmd)
	{
		case IOH_CMD_STR:			
			(*cmd_process_fn)(obj->rx_data, obj->rx_header->ioh_data_len);      		       		
			break;			
		default:
			printf("unknown ioh cmd = %d\n", obj->rx_header->ioh_cmd);
			break;
	}   
	
}
void inband_write_data(char *data,int data_len)
{
	struct ioh_class *obj = &__ioh_obj;
	unsigned char mac[ETH_MAC_LEN];	
	
	memcpy(&obj->rx_data[0], data,data_len );							
	obj->rx_header->ioh_data_len = data_len;
	obj->rx_header->ioh_data_len = htons(obj->rx_header->ioh_data_len); //host to network
	obj->rx_header->ioh_cmd = IOH_CMD_STR_RESP;

	// swap SA and DA
	memcpy(mac, obj->rx_buffer + ETH_MAC_LEN, ETH_MAC_LEN);
	memcpy(obj->rx_buffer + ETH_MAC_LEN, obj->rx_buffer, ETH_MAC_LEN);
	memcpy(obj->rx_buffer, mac, ETH_MAC_LEN);

	//hex_dump(obj->rx_buffer, ntohs(obj->rx_header->ioh_data_len) + sizeof(*obj->rx_header));//mark_issue
	write(obj->sockfd, obj->rx_buffer,
						ntohs(obj->rx_header->ioh_data_len) + sizeof(*obj->rx_header));

}

