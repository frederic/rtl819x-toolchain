#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>
#include <asm/types.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <stdio.h>
#include "kernel_config.h"
#include "netlink.h"
#include "rtnetlink.h"
#include "voip_manager.h"
#include "voip_params.h"

#define SOLAR_FIFO "/var/run/solar_control.fifo"

static uint32 wanPortstatus = 1;//First time must be different

void wanLinkCheck(void);
void msgToSolar(int);
int main()
{
	int h_nl;
	struct sockaddr_nl nl_local;
	int slow;
	int err;
	struct sockaddr_nl nl_remote;
	struct nlmsghdr *nlh;
	int nl_remote_len;
	char buf[2000];
    uint32 status;

	h_nl = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (h_nl == -1)
	{
		printf("netlink: open socket failed\n");
		return -1;
	}
	bzero(&nl_local, sizeof(nl_local));
	nl_local.nl_family = AF_NETLINK;
	nl_local.nl_pad = 0;
	nl_local.nl_pid = getpid();
	nl_local.nl_groups = RTMGRP_IPV4_IFADDR | RTMGRP_NOTIFY | RTMGRP_LINK;

	err = bind(h_nl, (struct sockaddr *) &nl_local, sizeof(nl_local));
	if (err == -1){
		printf("netlink: bind failed\n");
		return -1;
	}

	rtk_GetPortLinkStatus(&status);
    wanPortstatus = (status & PORT_LINK_WAN);
	
	memset(buf, 0,512);
	while(1)
	{

		nl_remote_len = sizeof( nl_remote );
		err = recvfrom(h_nl, buf, sizeof(buf), 0, (struct sockaddr *) &nl_remote, &nl_remote_len);
		if (err < 0)
		{
			printf("netlink recv failed \n");
			continue;
		}

		if (err == 0)
		{
			printf("netlink: EOF\n");
			continue;
		}

		if (nl_remote_len != sizeof(nl_remote))
		{
			printf("netlink: sender address len = %d\n", nl_remote_len);
			continue;
		}

		nlh = (struct nlmsghdr *) buf;
		for (; NLMSG_OK(nlh, err); nlh = NLMSG_NEXT(nlh, err))
		{
			switch (nlh->nlmsg_type)
			{
			case NLMSG_DONE:
			case NLMSG_ERROR:
				break;
			case RTM_NEWADDR:
				//printf("netlink: ip change\n");
				msgToSolar(0);
				break;
			case RTM_DELADDR:
				break;
			case RTM_DELLINK:
			case RTM_NEWLINK:
				//printf("netlink: link status change\n");
				break;
			case RTM_LINKCHANGE:
				wanLinkCheck();
				msgToSolar(1);
				break;
			default:
				printf("wrong msg\n");
				break;
			}
		}
	}
	return 0;
}

void msgToSolar(int msg)
{
	char tmp[50];
	if (access(SOLAR_FIFO, F_OK) == -1)
	{
        	if (mkfifo(SOLAR_FIFO, 0755) == -1)
	        	printf("access %s failed: %s\n", SOLAR_FIFO, strerror(errno));
	}
	sprintf(tmp,"echo \"n%d\" > %s", msg, SOLAR_FIFO);
	system(tmp);
}
void wanLinkCheck()
{
	uint32 status;
	rtk_GetPortLinkStatus(&status);

	if(wanPortstatus != (status & PORT_LINK_WAN))
	{
		if(status & PORT_LINK_WAN)
			system("wanlink.sh connect");
		else
			system("wanlink.sh disconnect");
	}

	wanPortstatus = (status & PORT_LINK_WAN);
}
