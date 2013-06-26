#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <asm/types.h>
#include <linux/socket.h>
#include <linux/config.h>
#include <netinet/in.h>

#include "rtk_cmd.h"
#if defined(CONFIG_RTL_FAST_FILTER)
#include "rtk_filter.h"
#endif
#include "rtk_igmp_delete.h"
#if defined(CONFIG_RTL_FASTBRIDGE)
#include "rtk_fastbridge.h"
#endif
#include "rtk_hw_qos.h"

SIGN_T sign_tbl[]={
#if defined(CONFIG_RTL_FAST_FILTER)
		{"filter","fast filter",rtk_filter_parse},
#else
		{"test","netlink test",rtk_debug_parse},
#endif
		{"igmp_delete", "clear igmp record after mac filter", rtk_igmp_delete_parse},
#if defined(CONFIG_RTL_FASTBRIDGE)
		{"fb", "fast bridge config", rtk_fb_parse},
#endif
		{"qos", "hw qos config", rtk_hw_qos_parse},
		{"end","table end",NULL}
};

/************************************************************/

int main(int argc, char* argv[])
{
	int i;

	if(argc < 2) return -1;

	i = 0;
	while(sign_tbl[i].handler != NULL)
	{
		if(!(memcmp(argv[1],sign_tbl[i]._key,strlen(argv[1]))))
		{
			return sign_tbl[i].handler(argc,argv);
		}
		else
		{
			i++;
			continue;
		}
	}

	return -1;
}

/************************************************************/
int rtk_netlink(int id,struct nl_data_info *_send_info, struct nl_data_info *_recv_info)
{
	struct sockaddr_nl src_addr, dest_addr;
	struct nlmsghdr *nlh = NULL;
	struct iovec iov;
	int sock_fd;
	struct msghdr msg;
//init
        sock_fd = socket(PF_NETLINK, SOCK_RAW, id);
        memset(&msg, 0, sizeof(msg));
        memset(&src_addr, 0, sizeof(src_addr));
        src_addr.nl_family = AF_NETLINK;
        src_addr.nl_pid = getpid(); /* self pid */
        src_addr.nl_groups = 0; /* not in mcast groups */
        bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));
        memset(&dest_addr, 0, sizeof(dest_addr));
        dest_addr.nl_family = AF_NETLINK;
        dest_addr.nl_pid = 0; /* For Linux Kernel */
        dest_addr.nl_groups = 0; /* unicast */

//prepare data
        nlh=(struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
/* Fill the netlink message header */
        nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
        nlh->nlmsg_pid = getpid(); /* self pid */
        nlh->nlmsg_flags = 0;
/* Fill in the netlink message payload */
	memcpy(NLMSG_DATA(nlh), _send_info->data,_send_info->len);
	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
#if defined(DEBUG_FLAG)
	printf(" Sending message. ...\n");
#endif
	sendmsg(sock_fd, &msg, 0);
/* Read message from kernel */
	memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
#ifdef DEBUG_FLAG
	printf(" Waiting message. ...\n");
#endif
	usleep(100);
	recvmsg(sock_fd, &msg, 0);
	memcpy(_recv_info->data,NLMSG_DATA(nlh),_recv_info->len);
	/* Close Netlink Socket */
	close(sock_fd);
	return 0;
}
#if !defined(CONFIG_RTL_FAST_FILTER)
int rtk_debug_parse(int _num, char* _param[])
{
	struct test_struct send_data,recv_data;
	struct nl_data_info send_info,recv_info;
	send_data.flag=14;
	sprintf(send_data.data,"Hello,I'm %s!",__FILE__);
	send_info.len=sizeof(struct test_struct);
	send_info.data=&send_data;
	recv_info.len=sizeof(struct test_struct);
	recv_info.data=&recv_data;
	rtk_netlink(NETLINK_RTK_DEBUG,&send_info, &recv_info);
	printf(" Received message payload: %s[%d]\n",recv_data.data,recv_data.flag);
	return 0;
}
#endif
