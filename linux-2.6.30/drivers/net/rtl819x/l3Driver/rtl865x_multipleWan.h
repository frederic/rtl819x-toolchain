/*
* Copyright c                  Realtek Semiconductor Corporation, 2010  
* All rights reserved.
* 
* Program : multiple wan device driver header file
* Abstract : 
* Author : hyking (hyking_liu@realsil.com.cn)  
*/

#ifndef RTL865X_MULTIPLEWAN_H
#define RTL865X_MULTIPLEWAN_H

#define RTL_ADVRT_MAX_NUM 4

rtl_advRoute_entry_t* rtl_get_advRt_entry_by_nexthop(ipaddr_t nexthop);
int rtl_init_advRt(void);
int rtl_exit_advRt(void);
int rtl_add_advRt_entry(rtl_advRoute_entry_t *entry);
int rtl_del_advRt_entry(rtl_advRoute_entry_t *entry);
int rtl_disable_advRt_by_netifName(const char *name);

#endif

