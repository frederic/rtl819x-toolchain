/*
 * IOH helper functions
 * Copyright (C)2010, Realtek Semiconductor Corp. All rights reserved
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include "ioh.h"

int bin2hex(const unsigned char *bin, char *hex, const int len)
{
	int i, idx;
	char hex_char[] = "0123456789ABCDEF";

	for (i=0, idx=0; i<len; i++)
	{
		hex[idx++] = hex_char[(bin[i] & 0xf0) >> 4];
		hex[idx++] = hex_char[bin[i] & 0x0f];
	}

	hex[idx] = 0;
	return 0;
}

int hex2bin(const char *hex, unsigned char *bin, const int len)
{
	int i, idx;
	unsigned char bytes[2];

	for (i=0, idx=0; hex[i]; i++)
	{
		if (hex[i & 0x01] == 0)
			return -1; // hex length != even

		if (hex[i] >= '0' && hex[i] <= '9')
			bytes[i & 0x01] = hex[i] - '0';
		else if (hex[i] >= 'A' && hex[i] <= 'F')
			bytes[i & 0x01] = hex[i] - 'A' + 10;
		else if (hex[i] >= 'a' && hex[i] <= 'f')
			bytes[i & 0x01] = hex[i] - 'a' + 10;
		else
			return -1; // not hex

		if (i & 0x01)
		{
			if (idx >= len)
				return -1; // out of size

			bin[idx++] = (bytes[0] << 4) | bytes[1];
		}
	}

	return 0;
}

void hex_dump(void *data, int size)
{
    /* dumps size bytes of *data to stdout. Looks like:
     * [0000] 75 6E 6B 6E 6F 77 6E 20
     *                  30 FF 00 00 00 00 39 00 unknown 0.....9.
     * (in a single line of course)
     */

    unsigned char *p = data;
    unsigned char c;
    int n;
    char bytestr[4] = {0};
    char addrstr[10] = {0};
    char hexstr[ 16*3 + 5] = {0};
    char charstr[16*1 + 5] = {0};
    for(n=1;n<=size;n++) {
        if (n%16 == 1) {
            /* store address for this line */
            snprintf(addrstr, sizeof(addrstr), "%.4x",
               ((unsigned int)p-(unsigned int)data) );
        }
            
        c = *p;
        if (isalnum(c) == 0) {
            c = '.';
        }

        /* store hex str (for left side) */
        snprintf(bytestr, sizeof(bytestr), "%02X ", *p);
        strncat(hexstr, bytestr, sizeof(hexstr)-strlen(hexstr)-1);

        /* store char str (for right side) */
        snprintf(bytestr, sizeof(bytestr), "%c", c);
        strncat(charstr, bytestr, sizeof(charstr)-strlen(charstr)-1);

        if(n%16 == 0) { 
            /* line completed */
            printf("[%4.4s]   %-50.50s  %s\n", addrstr, hexstr, charstr);
            hexstr[0] = 0;
            charstr[0] = 0;
        } else if(n%8 == 0) {
            /* half line: add whitespaces */
            strncat(hexstr, "  ", sizeof(hexstr)-strlen(hexstr)-1);
            strncat(charstr, " ", sizeof(charstr)-strlen(charstr)-1);
        }
        p++; /* next byte */
    }

    if (strlen(hexstr) > 0) {
        /* print rest of buffer if not empty */
        printf("[%4.4s]   %-50.50s  %s\n", addrstr, hexstr, charstr);
    }
}


int ioh_open(struct ioh_class *obj, char *dev, char *da, int debug)
{
	struct ifreq ifr;
	int ifindex;

	strcpy(obj->dev, dev);

	if (da == NULL) 
		; // da == NULL if iohd
	else
		hex2bin(da, obj->dest_mac, ETH_MAC_LEN);

	obj->debug = debug;

	obj->tx_header = (void *) obj->tx_buffer;
	obj->rx_header = (void *) obj->rx_buffer;
	obj->tx_data = (unsigned char *) obj->tx_buffer + sizeof(*obj->tx_header);
	obj->rx_data = (unsigned char *) obj->rx_buffer + sizeof(*obj->rx_header);

	// create raw socket
	obj->sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (obj->sockfd == -1) 
	{
		perror("socket():");
		exit(1);
	}

	if (obj->debug)
		printf("Successfully opened socket: %i\n", obj->sockfd);

	// retrieve ethernet interface index
	bzero(&ifr, sizeof(ifr));
	strncpy(ifr.ifr_name, obj->dev, IFNAMSIZ);
	if (ioctl(obj->sockfd, SIOCGIFINDEX, &ifr) == -1) 
	{
		perror("SIOCGIFINDEX");
		exit(1);
	}
	ifindex = ifr.ifr_ifindex;
	if (obj->debug)
		printf("Successfully got interface index: %i\n", ifindex);

	// retrieve corresponding MAC
	if (ioctl(obj->sockfd, SIOCGIFHWADDR, &ifr) == -1) 
	{
		perror("SIOCGIFHWADDR");
		exit(1);
	}

	memcpy(obj->src_mac, ifr.ifr_hwaddr.sa_data, sizeof(obj->src_mac));
	if (obj->debug)
	{
		printf("Successfully got our MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
			obj->src_mac[0], obj->src_mac[1], obj->src_mac[2],
			obj->src_mac[3], obj->src_mac[4], obj->src_mac[5]);
	}

	// Bind our raw socket to this interface
	bzero(&obj->socket_address, sizeof(obj->socket_address));
	obj->socket_address.sll_family   = PF_PACKET;
	obj->socket_address.sll_protocol = htons(ETH_P_RTK);
	obj->socket_address.sll_ifindex  = ifindex;
	if((bind(obj->sockfd, (struct sockaddr *) &obj->socket_address, 
		sizeof(obj->socket_address)))== -1)
	{
		perror("Error binding socket to interface\n");
		exit(1);
	}

	return 0;
}

int ioh_close(struct ioh_class *obj)
{
	close(obj->sockfd);
	return 0;
}

int ioh_send(struct ioh_class *obj, unsigned char *da)
{
	int sent;	// length of sent packet
	static int ioh_seq = 0;

	obj->socket_address.sll_hatype   = ARPHRD_ETHER;
	obj->socket_address.sll_pkttype  = PACKET_OTHERHOST;
	obj->socket_address.sll_halen    = ETH_ALEN;
	obj->socket_address.sll_addr[0]  = da[0];
	obj->socket_address.sll_addr[1]  = da[1];
	obj->socket_address.sll_addr[2]  = da[2];
	obj->socket_address.sll_addr[3]  = da[3];
	obj->socket_address.sll_addr[4]  = da[4];
	obj->socket_address.sll_addr[5]  = da[5];
	obj->socket_address.sll_addr[6]  = 0x00; 
	obj->socket_address.sll_addr[7]  = 0x00;

	obj->tx_header->ioh_seq = htons(ioh_seq++);

	if (obj->debug)
	{
		printf("%s: tx len = %d\n", __FUNCTION__, 
			ntohs(obj->tx_header->ioh_data_len) + sizeof(*obj->tx_header));
		hex_dump(obj->tx_buffer, ntohs(obj->tx_header->ioh_data_len) + sizeof(*obj->tx_header));
	}

	sent = sendto(obj->sockfd, obj->tx_buffer, 
		ntohs(obj->tx_header->ioh_data_len) + sizeof(*obj->tx_header), 0, 
		(struct sockaddr*) &obj->socket_address, sizeof(obj->socket_address));

	if (sent == -1) 
	{
		perror("sendto():");
		exit(1);
	}

	return sent;
}

int ioh_recv(struct ioh_class *obj, int timeout_ms)
{
    fd_set rfds;
    struct timeval timeout;
    int retval;
	int rx_len;

	FD_ZERO(&rfds);
	FD_SET(obj->sockfd, &rfds);

	if (timeout_ms < 0)
	{
		retval = select(obj->sockfd + 1, &rfds, NULL, NULL, NULL);
	}
	else
	{
		timeout.tv_sec = 0;
		timeout.tv_usec = timeout_ms * 1000;
		retval = select(obj->sockfd + 1, &rfds, NULL, NULL, &timeout);
	}
	
	if (retval && FD_ISSET(obj->sockfd, &rfds)) 
	{
		rx_len = recvfrom(obj->sockfd, obj->rx_buffer, 
			sizeof(obj->rx_buffer), 0, NULL, NULL);

		if (rx_len == -1) 
		{
			perror("recvfrom():");
			exit(1);
		}

		return rx_len;
	}
	else if (retval == 0)
	{ 
		if (obj->debug)
			printf("Timeout!!!\n");

		return -1;
	}
	else
	{
		perror("select():");
		exit(1);
	}

	return -2;
}

