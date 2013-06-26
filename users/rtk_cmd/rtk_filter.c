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
/************************************************************/
//FILTER
#define NF_DROP 	0
#define NF_FASTPATH	1
#define NF_LINUX	2
#define NF_MARK 	3
#define NF_REPEAT      	4
#define NF_OMIT 	5
static int mactoi (char *z)
{
	int i = 0;
	
	if(*z >= '0' && *z <= '9')
		i = (*z - '0')*16;
	else if(*z >= 'a' && *z <= 'f')
	{
		i = ((*z - 'a') + 10)*16;
	}	
	else if(*z >= 'A' && *z <= 'F')
	{
		i = ((*z - 'A') + 10)*16;
	}
	z++;
	if(*z >= '0' && *z <= '9')
		i += (*z - '0');
	else if(*z >= 'a' && *z <= 'f')
	{
		i += ((*z - 'a') + 10);
	}	
	else if(*z >= 'A' && *z <= 'F')
	{
		i += ((*z - 'A') + 10);
	}

	return i;
}
int rtk_nl_fastfilter_show(int id,int len,void *_send_data)
{
	struct sockaddr_nl src_addr, dest_addr;
	struct nlmsghdr *nlh = NULL;
	struct iovec iov;
	int sock_fd;
	struct msghdr msg;
	struct filter_respond_struct *precv_data;
	int tmp_len;
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
        nlh->nlmsg_flags = NLM_F_REQUEST;
/* Fill in the netlink message payload */
	memcpy(NLMSG_DATA(nlh), _send_data,len);
	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
#ifdef DEBUG_FLAG
	printf(" Sending message. ...\n");
#endif
	sendmsg(sock_fd, &msg, 0);
/* Read message from kernel */
	memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
#ifdef DEBUG_FLAG
	printf(" Waiting message. ...\n");
#endif
	while(recvmsg(sock_fd, &msg, 0)){
		precv_data=NLMSG_DATA(nlh);
		if(precv_data->flag>0)
		{
			fprintf(stderr,"%s\n",precv_data->data);
			memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
		}
		else
			break;
	};
	/* Close Netlink Socket */
	close(sock_fd);
	return 0;
}
int rtk_filter_parse(int _num, char* _param[])
{
	int i,ret;		
	char buf[MAX_PAYLOAD];
	filter_info_s filter_info;
	struct filter_respond_struct recv_data;
	struct nl_data_info send_info,recv_info;
	
	memset(&filter_info, 0, sizeof(filter_info_s));

	if(!(memcmp(_param[2],"add",strlen(_param[2]))))
	{
		filter_info.action=FILTER_ADD;
	}
	else if(!(memcmp(_param[2],"delete",strlen(_param[2]))) || !(memcmp(_param[2],"del",strlen(_param[2]))))
	{
		filter_info.action=FILTER_DEL;
	}
	else if(!(memcmp(_param[2],"flush",strlen(_param[2]))))
	{
		filter_info.action=FILTER_FLUSH;
	}
	else if(!(memcmp(_param[2],"enableLog",strlen(_param[2]))))
	{
		filter_info.action=FILTER_EN_LOG;
	}
	else if(!(memcmp(_param[2],"disableLog",strlen(_param[2]))))
	{
		filter_info.action=FILTER_DIS_LOG;
	}
	else if(!(memcmp(_param[2],"show",strlen(_param[2]))))
	{
		filter_info.action=FILTER_SHOW;
	}
	else
	{
		printf("The rule is not rtk filter rule format!\n");
		return -1;
	}			
		
	if((filter_info.action == FILTER_ADD) || (filter_info.action == FILTER_DEL) )
	{
		sprintf(buf,"");
		for(i=3;i<_num;i++)
		{
			strcat(buf,_param[i]);	
			strcat(buf," ");
		}
		ret = rtk_filter_rule_transform(buf,&(filter_info.filter_data));
		if(ret != 0)
			goto error;
	}
		
	if(filter_info.action == FILTER_ADD)
	{
		filter_info.action=FILTER_DEL;
		send_info.len=sizeof(filter_info_s);
		send_info.data=&filter_info;
		recv_info.len=sizeof(filter_info_s);
		recv_info.data=&recv_data;
		rtk_netlink(NETLINK_RTK_FILTER,&send_info, &recv_info);
		filter_info.action=FILTER_ADD;
	}

	if(filter_info.action == FILTER_SHOW)
	{
		rtk_nl_fastfilter_show(NETLINK_RTK_FILTER,sizeof(filter_info_s),&filter_info);
	}
	else
	{
		send_info.len=sizeof(filter_info_s);
		send_info.data=&filter_info;
		recv_info.len=sizeof(filter_info_s);
		recv_info.data=&recv_data;
		rtk_netlink(NETLINK_RTK_FILTER,&send_info,&recv_info);
		//printf(" Received message payload: %s[%d]\n",recv_data.data,recv_data.flag);
	}
	return 0;
error:
	return -1;
}

int rtk_filter_rule_transform(char *user_rule, filter_rule_ap_p fastpath_filter_rule)
{	
	unsigned char *strptr, *tokptr, *tokptr1, *tokptr2, *tokptr3;
	filter_rule_ap_p fastpath_rule=fastpath_filter_rule;
	unsigned char *mac_addr; 
	struct in_addr *ip_addr;
	int i;
	

	strptr = user_rule;

	if(strptr == NULL)
		printf("!!!!!! the user rule try to parse is NULL!\n");

	fastpath_rule->nf_flag.priority = 6;
	
	do{
		tokptr = strsep(&strptr, " ");
		tokptr1 = tokptr;
		if(!strlen(tokptr)) break;
		
		if( !memcmp(tokptr, "--mac-src", strlen("--mac-src")) )
		{
			//printf("this rule include source mac filter\n");
			fastpath_rule->nf_flag.mac_filter |= 0x1;
						
			tokptr = strsep(&strptr, " ");
			tokptr1 = tokptr;
			tokptr2 = strsep(&tokptr1, "/");
			if(tokptr2)
			{
				for(i=0; i<ETH_ALEN; i++)
				{
					tokptr3 = strsep(&tokptr2, ":");
					if(tokptr3)
						fastpath_rule->nf_src_filter.nf_mac.mac_mask.mac[i] = mactoi(tokptr3);
				}
				
				tokptr2 = strsep(&tokptr1, "/");
				if(tokptr2)
				{
					for(i=0; i<ETH_ALEN; i++)
					{
						tokptr3 = strsep(&tokptr2, ":");
						if(tokptr3)
							fastpath_rule->nf_src_filter.nf_mac.mac_mask.mask[i] = mactoi(tokptr3);
					}
				}
				else
					memcpy(fastpath_rule->nf_src_filter.nf_mac.mac_mask.mask, fastpath_rule->nf_src_filter.nf_mac.mac_mask.mac, ETH_ALEN);
			}
			
		}

		
		else if( !memcmp(tokptr, "--mac-dst", strlen("--mac-dst")) )
		{
			//printf("this rule include destination mac filter\n");
			fastpath_rule->nf_flag.mac_filter |= 0x4;

			tokptr = strsep(&strptr, " ");
			tokptr1 = tokptr;
			tokptr2 = strsep(&tokptr1, "/");
			if(tokptr2)
			{
				for(i=0; i<ETH_ALEN; i++)
				{
					tokptr3 = strsep(&tokptr2, ":");
					if(tokptr3)
						fastpath_rule->nf_dst_filter.nf_mac.mac_mask.mac[i] = mactoi(tokptr3);
				}
				tokptr2 = strsep(&tokptr1, "/");
				if(tokptr2)
				{
					for(i=0; i<ETH_ALEN; i++)
					{
						tokptr3 = strsep(&tokptr2, ":");
						if(tokptr3)
							fastpath_rule->nf_dst_filter.nf_mac.mac_mask.mac[i] = mactoi(tokptr3);
					}
				}
				else
					memcpy(fastpath_rule->nf_dst_filter.nf_mac.mac_mask.mask, fastpath_rule->nf_dst_filter.nf_mac.mac_mask.mac, ETH_ALEN);
			}
			
		}

		else if( !memcmp(tokptr, "--mac-range-src", strlen("--mac-range-src")) )
		{
			//printf("this rule include source mac range filter\n");
			fastpath_rule->nf_flag.mac_filter |= 0x2;
						
			tokptr = strsep(&strptr, " ");
			tokptr1 = tokptr;
			tokptr2 = strsep(&tokptr1, "/");
			if(tokptr2)
			{
				for(i=0; i<ETH_ALEN; i++)
				{
					tokptr3 = strsep(&tokptr2, ":");
					if(tokptr3)
					{
						fastpath_rule->nf_src_filter.nf_mac.mac_range.smac[i] = mactoi(tokptr3);
					}
				}
				mac_addr = &fastpath_rule->nf_src_filter.nf_mac.mac_range.smac;
				
				tokptr2 = strsep(&tokptr1, "/");
				if(tokptr2)
				{
					for(i=0; i<ETH_ALEN; i++)
					{
						tokptr3 = strsep(&tokptr2, ":");
						if(tokptr3)
						{
							fastpath_rule->nf_src_filter.nf_mac.mac_range.emac[i] = mactoi(tokptr3);
						}
					}
				}
				else
					memcpy(fastpath_rule->nf_src_filter.nf_mac.mac_range.emac, fastpath_rule->nf_src_filter.nf_mac.mac_range.smac, ETH_ALEN);
			}
			
		}

		
		else if( !memcmp(tokptr, "--mac-range-dst", strlen("--mac-range-dst")) )
		{
			//printf("this rule include destination mac filter\n");
			fastpath_rule->nf_flag.mac_filter |= 0x8;

			tokptr = strsep(&strptr, " ");
			tokptr1 = tokptr;
			tokptr2 = strsep(&tokptr1, "/");
			if(tokptr2)
			{
				for(i=0; i<ETH_ALEN; i++)
				{
					tokptr3 = strsep(&tokptr2, ":");
					if(tokptr3)
						fastpath_rule->nf_dst_filter.nf_mac.mac_range.smac[i] = mactoi(tokptr3);
				}
				tokptr2 = strsep(&tokptr1, "/");
				if(tokptr2)
				{
					for(i=0; i<ETH_ALEN; i++)
					{
						tokptr3 = strsep(&tokptr2, ":");
						if(tokptr3)
							fastpath_rule->nf_dst_filter.nf_mac.mac_range.emac[i] = mactoi(tokptr3);
					}
				}
				else
					memcpy(fastpath_rule->nf_dst_filter.nf_mac.mac_range.emac, fastpath_rule->nf_dst_filter.nf_mac.mac_range.smac, ETH_ALEN);
			}
			
		}

		else if( !memcmp(tokptr, "--ip-src", strlen("--ip-src")) )
		{
			//printf("this rule include source ip/mask filter\n");
			fastpath_rule->nf_flag.ip_filter |= 0x1;
			
			tokptr = strsep(&strptr, " ");
			tokptr1 = tokptr;
			tokptr2 = strsep(&tokptr1, "/");
			
			if(tokptr2)
			{
				ip_addr = (struct in_addr *)&(fastpath_rule->nf_src_filter.nf_ip.ip_mask.addr);
				if(ip_addr != NULL)
				{
					inet_aton(tokptr2, ip_addr);
				}
				if(tokptr1)
				{
					tokptr2 = strsep(&tokptr1, "/");
				
					if(tokptr2)
					{
						ip_addr = (struct in_addr *)&fastpath_rule->nf_src_filter.nf_ip.ip_mask.mask;
						inet_aton(tokptr2, ip_addr);
					}					
				}
				else
				{
					ip_addr = (struct in_addr *)&(fastpath_rule->nf_src_filter.nf_ip.ip_mask.mask);
					inet_aton("255.255.255.255", ip_addr);
				}
					
			}
		}

		else if( !memcmp(tokptr, "--ip-dst", strlen("--ip-dst")) )
		{
			//printf("this rule include destination ip filter\n");
			fastpath_rule->nf_flag.ip_filter |= 0x4;
			
			tokptr = strsep(&strptr, " ");
			tokptr1 = tokptr;
			tokptr2 = strsep(&tokptr1, "/");
			
			if(tokptr2)
			{
			
				ip_addr = (struct in_addr *)&fastpath_rule->nf_dst_filter.nf_ip.ip_mask.addr;
				inet_aton(tokptr2, ip_addr);

				
				tokptr2 = strsep(&tokptr1, "/");
				ip_addr = (struct in_addr *)&fastpath_rule->nf_dst_filter.nf_ip.ip_range.eaddr;
				if(tokptr2)
					inet_aton(tokptr2, ip_addr);
				else
					inet_aton("255.255.255.255", ip_addr);
					
			}
		}

		else if( !memcmp(tokptr, "--ip-range-src", strlen("--ip-range-src")) )
		{
			//printf("this rule include source ip range filter\n");
			fastpath_rule->nf_flag.ip_filter |= 0x2;
			
			tokptr = strsep(&strptr, " ");
			tokptr1 = tokptr;
			tokptr2 = strsep(&tokptr1, ":");
			
			if(tokptr2)
			{
			
				ip_addr = (struct in_addr *)&fastpath_rule->nf_src_filter.nf_ip.ip_range.saddr;
				inet_aton(tokptr2, ip_addr);
				//printf("===tokptr2 is %s, ip_addr_s is %x\n", tokptr2, ip_addr->s_addr);
				
				tokptr2 = strsep(&tokptr1, ":");
				ip_addr = (struct in_addr *)&fastpath_rule->nf_src_filter.nf_ip.ip_range.eaddr;
				if(tokptr2)
				{
					inet_aton(tokptr2, ip_addr);
				}
				else
				{
					printf("the end addr of ip_range filter can't be empty!!!\n");
					goto error;
				}
					
			}
		}

		else if( !memcmp(tokptr, "--ip-range-dst", strlen("--ip-range-dst")) )
		{
			//printf("this rule include destination ip range filter\n");
			fastpath_rule->nf_flag.ip_filter |= 0x8;
			
			tokptr = strsep(&strptr, " ");
			tokptr1 = tokptr;
			tokptr2 = strsep(&tokptr1, ":");
			
			if(tokptr2)
			{
			
				ip_addr = (struct in_addr *)&fastpath_rule->nf_dst_filter.nf_ip.ip_range.saddr;
				inet_aton(tokptr2, ip_addr);
				
				tokptr2 = strsep(&tokptr1, ":");
				ip_addr = (struct in_addr *)&fastpath_rule->nf_dst_filter.nf_ip.ip_range.eaddr;
				if(tokptr2)
				{
					inet_aton(tokptr2, ip_addr);
				}
				else
				{
					printf("the end addr of ip_range filter can't be empty!!!\n");
					goto error;
				}
					
			}
		}

		else if( !memcmp(tokptr, "--port-src", strlen("--port-src")) )
		{
			//printf("this rule include source port filter\n");
			fastpath_rule->nf_flag.port_filter |= 0x1;

			tokptr = strsep(&strptr, " ");
			tokptr1 = tokptr;
			tokptr2 = strsep(&tokptr1, ":");

			if(tokptr2)
			{
				fastpath_rule->nf_src_filter.nf_port.sport = fastpath_rule->nf_src_filter.nf_port.eport = atoi(tokptr2);
			}
		}
		
		else if( !memcmp(tokptr, "--port-dst", strlen("--port-dst")) )
		{
		
			//printf("this rule include source port filter\n");
			fastpath_rule->nf_flag.port_filter |= 0x4;
			
			tokptr = strsep(&strptr, " ");
			tokptr1 = tokptr;
			tokptr2 = strsep(&tokptr1, ":");
			
			if(tokptr2)
			{
				fastpath_rule->nf_dst_filter.nf_port.sport = fastpath_rule->nf_dst_filter.nf_port.eport = atoi(tokptr2);
			}
		}
		else if( !memcmp(tokptr, "--port-range-src", strlen("--port-range-src")) )
		{
			//printf("this rule include source port range filter\n");
			fastpath_rule->nf_flag.port_filter |= 0x2;
			tokptr = strsep(&strptr, " ");
			tokptr1 = tokptr;
			tokptr2 = strsep(&tokptr1, ":");
			if(tokptr2)
			{
				fastpath_rule->nf_src_filter.nf_port.sport = atoi(tokptr2);
				if(tokptr1)
				{
					tokptr2 = strsep(&tokptr1, ":");
					if(tokptr2)
						fastpath_rule->nf_src_filter.nf_port.eport = atoi(tokptr2);
				}
			}
		}
		else if( !memcmp(tokptr, "--port-range-dst", strlen("--port-range-dst")) )
		{
			//printf("this rule include destination port range filter\n");
			fastpath_rule->nf_flag.port_filter |= 0x8;
			
			tokptr = strsep(&strptr, " ");
			tokptr1 = tokptr;
			tokptr2 = strsep(&tokptr1, ":");
			
			if(tokptr2)
			{
				fastpath_rule->nf_dst_filter.nf_port.sport = atoi(tokptr2);
				if(tokptr1)
				{
					tokptr2 = strsep(&tokptr1, ":");
					if(tokptr2)
						fastpath_rule->nf_dst_filter.nf_port.eport = atoi(tokptr2);
				}
			}
		}
		
		else if( !memcmp(tokptr, "--protocol", strlen("--protocol")) )
		{
			//printf("this rule include protocol filter\n");
			tokptr = strsep(&strptr, " ");
			tokptr1 = tokptr;
			tokptr2 = strsep(&tokptr1, ":");
			if(tokptr2)
			{
				if(!memcmp(tokptr2, "tcp", strlen(tokptr2)))
					fastpath_rule->nf_flag.protocol_filter |= 0x1;					
				else if( !memcmp(tokptr2, "udp", strlen(tokptr2)))
					fastpath_rule->nf_flag.protocol_filter |= 0x2;
				else if( !memcmp(tokptr2, "tcp_udp", strlen(tokptr2)))
					fastpath_rule->nf_flag.protocol_filter |= 0x3;
				else
					goto error;
			}
		}

		else if( !memcmp(tokptr, "--schedule", strlen("--schedule")) )
		{
			//printf("this rule include schedule filter\n");
			tokptr = strsep(&strptr, " ");
			tokptr1 = tokptr;
			tokptr2 = strsep(&tokptr1, ":");
			fastpath_rule->nf_flag.schedule_filter = 2;
			if(tokptr2)
			{
				fastpath_rule->nf_schedule.day_mask = atoi(tokptr2);
				tokptr2 = strsep(&tokptr1, ":");
				fastpath_rule->nf_schedule.all_hours = atoi(tokptr2);
				tokptr2 = strsep(&tokptr1, ":");
				if(tokptr2)
				{
					fastpath_rule->nf_schedule.stime = atoi(tokptr2);
					tokptr2 = strsep(&tokptr1, ":");
					if(tokptr2)
						fastpath_rule->nf_schedule.etime = atoi(tokptr2);
				}
				
			}
			//printf("==fastpath_rule->nf_schedule.day_mask is %d, fastpath_rule->nf_schedule.all_hours is %d, fastpath_rule->nf_schedule.stime is %d ,fastpath_rule->nf_schedule.etime is %d, fastpath_rule->nf_flag.schedule_filter is %d\n",
			//	fastpath_rule->nf_schedule.day_mask, fastpath_rule->nf_schedule.all_hours, fastpath_rule->nf_schedule.stime, fastpath_rule->nf_schedule.etime, fastpath_rule->nf_flag.schedule_filter);
		}
		
		else if( !memcmp(tokptr, "--priority", strlen("--priority")) )
		{
			//printf("this rule include priority\n");
			tokptr = strsep(&strptr, " ");
			tokptr1 = tokptr;
			tokptr2 = strsep(&tokptr1, ":");
			if(tokptr2)
				fastpath_rule->nf_flag.priority = atoi(tokptr2);
			if(fastpath_rule->nf_flag.priority<0 || fastpath_rule->nf_flag.priority>7)
			{
				printf("the rtk_filter rule priority should be 0~7, set it to default priority(6)\n");
			}	
		}
		
		else if( !memcmp(tokptr, "--policy", strlen("--policy")) )
		{
			tokptr = strsep(&strptr, " ");
			tokptr1 = tokptr;
			tokptr2 = strsep(&tokptr1, ":");
			if(tokptr2)
			{
				if( !memcmp(tokptr2, "drop", strlen(tokptr2)) )
				{
					//printf("all the packets that match this rule will be droped\n");
					fastpath_rule->nf_flag.action = NF_DROP;
				}
				else if( !memcmp(tokptr2, "fastpath", strlen(tokptr2)) )
				{
					//printf("all the packets that match this rule will be trip to fastpath\n");
					fastpath_rule->nf_flag.action = NF_FASTPATH;					
				}
				else if( !memcmp(tokptr2, "linux_protocol_stack", strlen(tokptr2)) )
				{
					//printf("all the patckets that match this rule will be trip to linux protocol stack\n");
					fastpath_rule->nf_flag.action = NF_LINUX;
				}
				else if( !memcmp(tokptr2, "mark", strlen(tokptr2)) )
				{
					//printf("all the packets that match this rule will be add a mark for it\n");
					fastpath_rule->nf_flag.action = NF_MARK;
				}
				else if( !memcmp(tokptr2, "omit", strlen(tokptr2)) )
				{
					//printf("all the packets that match this rule will be omitted, system will do nothing for these packet\n");
					fastpath_rule->nf_flag.action = NF_OMIT;
				}
				else
				{
					printf("rtk_filter don't support this action!!!!");
					goto error;
				}
			}
		}

		else if( !memcmp(tokptr, "--url-key", strlen("--url-key")) )
		{
			tokptr = strsep(&strptr, " ");
			tokptr1 = tokptr;
			tokptr2 = strsep(&tokptr1, ":");
			if(tokptr2)
			{
				fastpath_rule->nf_flag.url_filter = 0x3;
				memcpy(fastpath_rule->nf_url.url_key, tokptr2, strlen(tokptr2));
			}
		}
		else if( !memcmp(tokptr, "--url-strict", strlen("--url-strict")) )
		{
			tokptr = strsep(&strptr, " ");
			tokptr1 = tokptr;
			tokptr2 = strsep(&tokptr1, ":");
			if(tokptr2)
			{
				fastpath_rule->nf_flag.url_filter = 0x1;
				memcpy(fastpath_rule->nf_url.url_key, tokptr2, strlen(tokptr2));
			}
		}		
		else if(!memcmp(tokptr, "--url-extend", strlen("--url-extend")) )
		{
			tokptr = strsep(&strptr, " ");
			tokptr1 = tokptr;
			tokptr2 = strsep(&tokptr1, ":");
			if(tokptr2)
			{
				fastpath_rule->nf_flag.url_filter = 0x2;
				memcpy(fastpath_rule->nf_url.url_key, tokptr2, strlen(tokptr2));
			}
		}
		else if(!memcmp(tokptr, "--phy-port-source", strlen("--phy-port-source")))
		{
			tokptr = strsep(&strptr, " ");
			tokptr1 = tokptr;
			tokptr2 = strsep(&tokptr1, ":");
			if(tokptr2)
			{
				fastpath_rule->nf_flag.src_phy_port_filter = atoi(tokptr2);
				fastpath_rule->nf_flag.src_phy_port_filter |= 1<<7;
			}
		}
		else if(!memcmp(tokptr, "--mark", strlen("--mark")))
		{
			tokptr = strsep(&strptr, " ");
			tokptr1 = tokptr;
			tokptr2 = strsep(&tokptr1, ":");
			if(tokptr2)
			{
				fastpath_rule->nf_flag.mark= atoi(tokptr2);
			}
		}
		else if( !memcmp(tokptr, " ", strlen(" "))) 
		{
			continue;
		}	
		else
		{
			printf("don't support this option %s in rtk filter rule\n", tokptr);
			goto error;
		}
	}while(tokptr != NULL && strptr != NULL);

	return 0;

error:
	printf("the rule is not rtk_filter rule format!!!!!!!!!\n");
	return -1;
}
