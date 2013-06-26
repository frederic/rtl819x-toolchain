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
#include <linux/socket.h>
#include <netinet/in.h>
#include "rtk_cmd.h"
#include "rtk_filter.h"
#include "rtk_igmp_delete.h"

int rtk_igmp_delete_parse(int _num, char* _param[])
{	
	char cmd[20];
	char ret[50];
	struct nl_data_info send_info,recv_info;
	
	if(strlen(_param[2]) < 11 || strlen(_param[2]) > 20)
	{
		printf("error mac:%s size:%d\n", _param[2], strlen(_param[2]));
		return -1;
	}
	memcpy(cmd, _param[2], strlen(_param[2]));
	send_info.len = strlen(_param[2]);
	send_info.data = &cmd[0];
	recv_info.len=sizeof(ret);
	recv_info.data=&ret[0];
	rtk_netlink(NETLINK_MULTICAST_DELETE,&send_info, &recv_info);
	//printf("igmp_delete Received message: \n%s\n",ret);
	return 0;
}


