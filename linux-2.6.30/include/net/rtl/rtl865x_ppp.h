/*
* Copyright c                  Realtek Semiconductor Corporation, 2008  
* All rights reserved.
* 
* Program : ppoe table driver
* Abstract : 
* Author : hyking (hyking_liu@realsil.com.cn)  
*/
#ifndef	RTL865X_PPP_H
#define	RTL865X_PPP_H

#if !defined(REDUCE_MEMORY_SIZE_FOR_16M)
#define REDUCE_MEMORY_SIZE_FOR_16M
#endif

#if defined(REDUCE_MEMORY_SIZE_FOR_16M)
#define PPP_NUMBER	2
#else
#define PPP_NUMBER	8
#endif
#define SESSION_TYPE_PPPOE			0x00
#define SESSION_TYPE_PPTP			0x01
#define SESSION_TYPE_L2TP			0x02

int32 rtl865x_delPpp(uint32 sessionId);
int32 rtl865x_delPppbyIfName(char *name);
int32 rtl865x_addPpp(uint8 *ifname, ether_addr_t *mac, uint32 sessionId, int32 type);

#endif

