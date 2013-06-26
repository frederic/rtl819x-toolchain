#ifndef _RTK_CMD_H
#define _RTK_CMD_H
//dzh begin
//#include <linux/autoconf.h>
//dzh end
#include <linux/netlink.h>

//#define DEBUG_FLAG 1

/************************************************************/
//DEBUG
#define MAX_PAYLOAD 1024 /* maximum payload size*/
#if !defined(CONFIG_RTL_FAST_FILTER)
struct test_struct
{
	int flag;
	char data[MAX_PAYLOAD];
};
int rtk_debug_parse(int _num, char* _param[]);
#endif
struct nl_data_info
{
	int len;
	void *data;
};
/************************************************************/
int rtk_netlink(int id,struct nl_data_info *_send_info, struct nl_data_info *_recv_info);
typedef struct _rtk_cmd_signature__ {
	unsigned char *_key;
	unsigned char *comment ;
	int  (*handler)(int _num, char* _param[]);
} SIGN_T ;
#endif
