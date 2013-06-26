#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <asm/types.h>
#include <linux/config.h>
#include <linux/netlink.h>
#include <net/rtl/rtl_types.h>
#include <linux/netdevice.h>
#include <net/rtl/features/fast_bridge.h>
#include "rtk_cmd.h"
#include "rtk_fastbridge.h"

#if defined(CONFIG_RTL_FASTBRIDGE)
void rtk_fb_usage(void)
{
	printf("usage:	rtk_cmd fb enable/disalbe fwd/filter\n");
	printf("     	rtk_cmd fb status\n");
	printf("     	rtk_cmd fb dump\n");
}

static inline int rtk_fb_fwd_enable(int _num, char* _param[])
{
	return (_num==4 && 
		((!(memcmp(_param[2],"enable",strlen(_param[2]))) 
			&& !(memcmp(_param[3],"fwd",strlen(_param[3]))))||
		(!(memcmp(_param[2],"ENABLE",strlen(_param[2]))) 
			&& !(memcmp(_param[3],"fwd",strlen(_param[3]))))||
		(!(memcmp(_param[2],"enable",strlen(_param[2]))) 
			&& !(memcmp(_param[3],"FWD",strlen(_param[3]))))||
		(!(memcmp(_param[2],"ENABLE",strlen(_param[2]))) 
			&& !(memcmp(_param[3],"FWD",strlen(_param[3]))))));
}

static inline int rtk_fb_fwd_disable(int _num, char* _param[])
{
	return (_num==4 && 
		((!(memcmp(_param[2],"disable",strlen(_param[2]))) 
			&& !(memcmp(_param[3],"fwd",strlen(_param[3]))))||
		(!(memcmp(_param[2],"DISABLE",strlen(_param[2]))) 
			&& !(memcmp(_param[3],"fwd",strlen(_param[3]))))||
		(!(memcmp(_param[2],"disable",strlen(_param[2]))) 
			&& !(memcmp(_param[3],"FWD",strlen(_param[3]))))||
		(!(memcmp(_param[2],"DISABLE",strlen(_param[2]))) 
			&& !(memcmp(_param[3],"FWD",strlen(_param[3]))))));
}

static inline int rtk_fb_filter_enable(int _num, char* _param[])
{
	return (_num==4 && 
		((!(memcmp(_param[2],"enable",strlen(_param[2]))) 
			&& !(memcmp(_param[3],"filter",strlen(_param[3]))))||
		(!(memcmp(_param[2],"ENABLE",strlen(_param[2]))) 
			&& !(memcmp(_param[3],"filter",strlen(_param[3]))))||
		(!(memcmp(_param[2],"enable",strlen(_param[2]))) 
			&& !(memcmp(_param[3],"FILTER",strlen(_param[3]))))||
		(!(memcmp(_param[2],"ENABLE",strlen(_param[2]))) 
			&& !(memcmp(_param[3],"FILTER",strlen(_param[3]))))));
}

static inline int rtk_fb_filter_disable(int _num, char* _param[])
{
	return (_num==4 && 
		((!(memcmp(_param[2],"disable",strlen(_param[2]))) 
			&& !(memcmp(_param[3],"filter",strlen(_param[3]))))||
		(!(memcmp(_param[2],"DISABLE",strlen(_param[2]))) 
			&& !(memcmp(_param[3],"filter",strlen(_param[3]))))||
		(!(memcmp(_param[2],"disable",strlen(_param[2]))) 
			&& !(memcmp(_param[3],"FILTER",strlen(_param[3]))))||
		(!(memcmp(_param[2],"DISABLE",strlen(_param[2]))) 
			&& !(memcmp(_param[3],"FILTER",strlen(_param[3]))))));
}

static inline int rtk_fb_set_cnt(int _num, char* _param[])
{
	return (_num==4 && 
		(!(memcmp(_param[2],"set_cnt",strlen(_param[2])))||
			!(memcmp(_param[2],"SET_CNT",strlen(_param[2])))));
}

static inline int rtk_fb_status(int _num, char* _param[])
{
	return (_num==3 && 
		(!(memcmp(_param[2],"status",strlen(_param[2])))||
			!(memcmp(_param[2],"STATUS",strlen(_param[2])))));
}

static inline int rtk_fb_dump(int _num, char* _param[])
{
	return (_num==3 && 
		(!(memcmp(_param[2],"dump",strlen(_param[2])))||
			!(memcmp(_param[2],"DUMP",strlen(_param[2])))));
}

static inline void rtk_fb_dump_entry(rtl_fb_nl_entry *entry)
{
	printf("Mac[%x:%x:%x:%x:%x:%x] RxDev[%s] Idle[%lds].\n", 
		entry->mac_addr[0], entry->mac_addr[1], entry->mac_addr[2],
		entry->mac_addr[3], entry->mac_addr[4], entry->mac_addr[5], 
		entry->name,
		entry->last_used);
}

static int rtk_fb_get_used_entry(void)
{
	fb_cmd_info_s		fb_info;
	fb_cmd_info_s		rx_info;

	struct nl_data_info send_info,recv_info;
	
	fb_info.action = FB_CMD_GET_USED_NUM;
	send_info.len=sizeof(fb_cmd_info_s);
	recv_info.len=send_info.len;
	send_info.data=(void*)&fb_info;
	recv_info.data=(void*)&rx_info;
	rx_info.action = FB_CMD_NO_CMD;
	rtk_netlink(NETLINK_RTK_FB, &send_info, &recv_info);

	if (rx_info.action!=FB_CMD_GET_USED_NUM) {
		printf("RTK CMD FP get  fdb info failed...\n");
		return -1;
	}

	return rx_info.info.in_used;
}

int rtk_fb_parse(int _num, char* _param[])
{
	fb_cmd_info_s		fb_info;
	fb_cmd_info_s		rx_info;
	uint32			i, entry_number;
	struct nl_data_info send_info, recv_info;
		
	memset(&fb_info, 0, sizeof(fb_cmd_info_s));
	
	if(rtk_fb_fwd_enable(_num, _param)) {
		fb_info.action = FB_CMD_SET_FWD;
		fb_info.info.data.enable_fb_fwd = TRUE;
	} else if(rtk_fb_fwd_disable(_num, _param)) {
		fb_info.action = FB_CMD_SET_FWD;
		fb_info.info.data.enable_fb_fwd = FALSE;
	} else if(rtk_fb_filter_enable(_num, _param)) {
		fb_info.action = FB_CMD_SET_FILTER;
		fb_info.info.data.enable_fb_filter = TRUE;
	} else if(rtk_fb_filter_disable(_num, _param)) {
		fb_info.action = FB_CMD_SET_FILTER;
		fb_info.info.data.enable_fb_filter = FALSE;
	} else if (rtk_fb_set_cnt(_num, _param)) {
		fb_info.action = FB_CMD_SET_ENTRY_NUM;
		fb_info.info.data.entry_num = atoi(_param[3]);
	} else if(rtk_fb_status(_num, _param)) {
		fb_info.action = FB_CMD_GET_STATUS;
	} else if (rtk_fb_dump(_num, _param)) {
		entry_number = rtk_fb_get_used_entry();
		printf("FastBridge FDB Entrys [%d used]:\n", entry_number);
		if (entry_number==0)
			return SUCCESS;
		fb_info.info.in_used = entry_number;
		fb_info.info.entry = (rtl_fb_nl_entry*)malloc(entry_number*sizeof(rtl_fb_nl_entry));
		if (fb_info.info.entry==NULL) {
			printf("FastBridge FDB dump entry failed due to memory insufficient.\n");
			return FAILED;
		}
		fb_info.action = FB_CMD_DUMP_ENTRYS;
	} else {
		printf("The rule is not rtk fb rule format!\n");
		rtk_fb_usage();
		return FAILED;
	}

	send_info.len=sizeof(fb_cmd_info_s);
	send_info.data=(void*)&fb_info;

	recv_info.len=sizeof(fb_cmd_info_s);
	recv_info.data=(void*)&rx_info;
	rx_info.action = FB_CMD_NO_CMD;

	rtk_netlink(NETLINK_RTK_FB, &send_info, &recv_info);

	if (rx_info.action==FB_CMD_NO_CMD)
		printf("RTK CMD FP PROCESS Error...\n");
	else if (rx_info.action==FB_CMD_DUMP_ENTRYS) {
		for(i=0;i<entry_number;i++) {
			rtk_fb_dump_entry(&((rtl_fb_nl_entry*)rx_info.info.entry)[i]);
		}
	} else if (rx_info.action==FB_CMD_GET_STATUS) {
		printf("FastBridge FDB Status:\n");
		printf("		fast bridge %s.\n", rx_info.info.data.enable_fb_fwd==TRUE?"Enabled":"Disabled");
		printf("		fast bridge filter %s.\n", rx_info.info.data.enable_fb_filter==TRUE?"Enabled":"Disabled");
		printf("		fast bridge total entry number:	%d.\n", rx_info.info.data.entry_num);
		printf("		fast bridge used entry number:	%d.\n", rx_info.info.in_used);
	} else if (rx_info.action==FB_CMD_SET_ENTRY_NUM) {
		if (rx_info.info.data.entry_num!=fb_info.info.data.entry_num)
			printf("Due to memory issue, FastBridge FDB set total entry number %d.\n", rx_info.info.data.entry_num);
		else
			printf("FastBridge FDB set total entry number %d Successfully.\n", rx_info.info.data.entry_num);
	} else if ((rx_info.action==FB_CMD_SET_FWD)||(rx_info.action==FB_CMD_SET_FILTER)) {
		printf("FastBridge FDB Set Successfully.\n");
	}
	return SUCCESS;
}
#endif

