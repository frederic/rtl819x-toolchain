#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

#include <linux/if_ether.h>
#include <linux/if_packet.h>
#ifndef __IOH_H
#include "ioh.h"
#endif
#include <linux/if_arp.h>
#include "wireless_copy.h"
#ifdef INBAND_HOST
#else
#include <net80211/ieee80211.h>
#include <net80211/ieee80211_crypto.h>
#include <net80211/ieee80211_ioctl.h>
#endif

#define INBAND_INTF		"br0"
#define INBAND_SLAVE	("001234567899")
#define INBAND_IOCTL_TYPE	0x8899
#define INBAND_NETLINK_TYPE 0x9000
#define INBAND_DEBUG 0
#define RX_EXPIRE_PERIOD 3	//in secs

#define IWREQ_LEN 32
#define INBAND_IOCTLTYPE_LEN	4
#define INBAND_IOCTLHDR_LEN	6
#define INBAND_PENDING_START(data) data+INBAND_IOCTLHDR_LEN+IWREQ_LEN
#define INBAND_IOCTLRET_PTR(data) data+INBAND_IOCTLTYPE_LEN
#define IOH_HDR_LEN sizeof(struct ioh_header)
#ifdef INBAND_HOST
#define MAXDATALEN      1560    // jimmylin: org:256, enlarge for pass EAP packet by event queue
#define DOT11_EVENT_REQUEST 2
#define SIOCGIWIND 0x89ff
#endif
#define ioctl_cmd_type 0x05
#define sycall_cmd_type 0x04

#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...) printf("[Inband]%s %d:"fmt,__FUNCTION__ , __LINE__ , ## args)
#else
#define DEBUG_PRINT(fmt, args...)
#endif

#ifdef IEEE80211_IOCTL_SETWMMPARAMS
/* Assume this is built against realtek-ng */
#define REALTEK_NG
#endif /* IEEE80211_IOCTL_SETWMMPARAMS */
//#define RTL8192CD_IOCTL_DEL_STA	0x89f7
//#define SIOCGIWIND      		0x89ff
//#defein SIOCIWLASTPRIV		0x8BFF

typedef struct _DOT11_REQUEST{
        unsigned char   EventId;
}DOT11_REQUEST;

unsigned int get_random(int max)
{
	struct timeval tod;

	gettimeofday(&tod , NULL);
	srand(tod.tv_usec);
	return rand()%max;
}

static int rx_expired(struct timeval *start)
{
	struct timeval now;

	gettimeofday(&now , NULL);

	return (now.tv_sec-start->tv_sec<RX_EXPIRE_PERIOD)?0:1;
}

/****************************************************

	tx_data will be formatted as following:
	+--------------+------------+--------------+--------------+
	| ioctl number | req length | struct iwreq | pending data |
	+--------------+------------+--------------+--------------+

	
 ****************************************************/
int
inband_ioctl(int ioctl_op, void *req)
{
	int ret=-1, ext_len=0, sq=0, rcv_sq=0, inband_channel=-1, iw=0;
	unsigned char cmd_type=0x04, buf[BUF_SIZE], *rx_buf;
	struct iwreq *local_iwr, *rsp_iwr;
	struct timeval start;
	int ioctl_op_endian;
	int local_iwr_pointer;
	struct ifreq *ifr_ptr;

	switch(ioctl_op) {
#ifdef INBAND_HOST
		case SIOCGIFADDR:
		case SIOCGIFINDEX:
			iw = 0x0;
		case SIOCGIFHWADDR:
			iw = 0x0;
			break;
		default:
			iw = 0x11;
			break;
#else
		case IEEE80211_IOCTL_FILTERFRAME:
		case IEEE80211_IOCTL_SETPARAM:
		case IEEE80211_IOCTL_DELKEY:
			iw = 0x10;
			break;
		case IEEE80211_IOCTL_SETMLME:
		case IEEE80211_IOCTL_SETKEY:
			//printf("~~~%s %d\n",__FUNCTION__,__LINE__);
			//hex_dump(((struct iwreq *)req)->u.data.pointer,((struct iwreq *)req)->u.data.length);
#ifdef REALTEK_NG
		case IEEE80211_IOCTL_STA_STATS:
#else
		case IEEE80211_IOCTL_GETSTASTATS:
#endif
		case IEEE80211_IOCTL_SET_APPIEBUF:
		case SIOCGIWRANGE:
		case SIOCSIWESSID:
		case SIOCGIWESSID:
		case SIOCSIWENCODE:
		case IEEE80211_IOCTL_WDSADDMAC:
		case IEEE80211_IOCTL_WDSDELMAC:
		case IEEE80211_IOCTL_GET_APPIEBUF:
		case 0x89f7: //RTL8192CD_IOCTL_DEL_STA
		case 0x89ff: //SIOCGIWIND
		case 0x8bff:	//SIOCIWLASTPRIV
			iw = 0x11;
			break;
		case SIOCGIFINDEX:
		case SIOCGIFFLAGS:
		case SIOCGIFHWADDR:
                        iw = 0x0;
                        break;
                case SIOCSIFHWADDR:			
			ifr_ptr = (struct ifreq *)req;
			ifr_ptr->ifr_hwaddr.sa_family = htons(ifr_ptr->ifr_hwaddr.sa_family);
			iw = 0x0;
			break;
		case SIOCSIFFLAGS:
			ifr_ptr = (struct ifreq *)req;
			ifr_ptr->ifr_flags = htons(ifr_ptr->ifr_flags);
			iw = 0x0;
			break;
		case SIOCSIFMTU:
			ifr_ptr = (struct ifreq *)req;
			ifr_ptr->ifr_mtu = htonl(ifr_ptr->ifr_mtu);
			iw = 0x0;
			break;
		default:
			printf("Unknown ioctl number:%d\n",ioctl_op);
			return -1;
#endif
	}

	inband_channel = inband_open(INBAND_INTF, INBAND_SLAVE, INBAND_IOCTL_TYPE, INBAND_DEBUG);

	if( inband_channel < 0 ) {
		printf("ioctl(inband channel open) failed %x\n",ioctl_op);
		goto out;
	}
	
	ioctl_op_endian = htonl(ioctl_op);//mark_endian
	memset(buf,0,BUF_SIZE);
	memcpy(buf,(unsigned char *)&ioctl_op_endian,INBAND_IOCTLTYPE_LEN);	
	if( iw & 0x1 ) {
		local_iwr = (struct iwreq *)req;
		ext_len = local_iwr->u.data.length;
		local_iwr_pointer = (int)local_iwr->u.data.pointer;		
		local_iwr->u.data.length = htons(ext_len); 
		local_iwr->u.data.flags = htons(local_iwr->u.data.flags);
		//memcpy(INBAND_PENDING_START(buf),local_iwr->u.data.pointer, local_iwr->u.data.length);
		memcpy(INBAND_PENDING_START(buf),local_iwr->u.data.pointer, ext_len);
	}
	memcpy(buf+INBAND_IOCTLHDR_LEN,(unsigned char *)req,IWREQ_LEN);
	
	buf[INBAND_IOCTLTYPE_LEN] = iw&0x10>>4;
	buf[INBAND_IOCTLTYPE_LEN+1] = iw&0x1;
	sq = get_random(65536);

	//printf("inband ioctl %d >>> \n",sq);
	if( inband_write(inband_channel, sq, ioctl_cmd_type, buf, INBAND_IOCTLHDR_LEN+IWREQ_LEN+ext_len, 0) < 0) {
		DEBUG_PRINT("inband ioctl message send failed\n");
		goto out;
	}
	else {
		ret = 0;

		switch(ioctl_op) {
#ifdef INBAND_HOST
		case SIOCGIFADDR:
		case SIOCGIFINDEX:
		case SIOCGIFHWADDR:
			break;
		default:
			ret = 0;
			goto out;
#else
#ifdef REALTEK_NG
		case IEEE80211_IOCTL_STA_STATS:
#else
		case IEEE80211_IOCTL_GETSTASTATS:
#endif
		case SIOCGIWRANGE:
		case SIOCGIWESSID:
		case IEEE80211_IOCTL_GET_APPIEBUF:
		case 0x89ff: //SIOCGIWIND
		case 0x8bff:	//SIOCIWLASTPRIV
		case SIOCGIFINDEX:
		case SIOCGIFFLAGS:
		case SIOCGIFHWADDR:
			break;
		default:
			ret = 0;
			goto out;
#endif
		}
		
		gettimeofday(&start,NULL);

rx_retry:
		if( inband_rcv_data_and_seq(inband_channel, &rcv_sq, &cmd_type, &rx_buf, 500) < 0 ) {
			if(!rx_expired(&start)) {
				goto rx_retry;
			} else {
				DEBUG_PRINT("inband ioctl message not receive response\n");
				ret = -1;
				goto out;
			}
		}

		if( sq != rcv_sq )
			goto rx_retry;

		memcpy(&ret,rx_buf,sizeof(int));
		DEBUG_PRINT("inband ioctl retVal:%d\n",ret);
		
		memcpy((unsigned char *)req,rx_buf+INBAND_IOCTLHDR_LEN,IWREQ_LEN);	

		if( iw & 0x1 )
		{
			local_iwr = (struct iwreq *)req;
			rsp_iwr = (struct iwreq *)INBAND_IOCTLRET_PTR(rx_buf);
			//memcpy((int *)local_iwr_pointer, INBAND_PENDING_START(rx_buf), ext_len);
			memcpy((int *)local_iwr_pointer, INBAND_PENDING_START(rx_buf), rsp_iwr->u.data.length);
			//local_iwr->u.data.length = ext_len;
			local_iwr->u.data.length = rsp_iwr->u.data.length;
			local_iwr->u.data.pointer = (int *)local_iwr_pointer; 
			local_iwr->u.data.flags =  ntohs(local_iwr->u.data.flags);
		}
		else if (iw == 0x0) //ifreq ;
		{
			switch(ioctl_op) {
			case SIOCGIFINDEX:
			ifr_ptr = (struct ifreq *)req;
			ifr_ptr->ifr_ifindex = ntohl(ifr_ptr->ifr_ifindex);	
				break;
			case SIOCGIFFLAGS:
			ifr_ptr = (struct ifreq *)req;
			ifr_ptr->ifr_flags = ntohs(ifr_ptr->ifr_flags);	
				break;
			//case SIOCGIFHWADDR:
			default:
				break;
			}	

		} 
	}

out:
	if( inband_channel >= 0 )
		inband_close(inband_channel);

	return ret;
}


int
inband_remote_cmd(unsigned char *cmd)
{
	unsigned char *buf;
	unsigned int channel = -1;

	channel = inband_open(INBAND_INTF,INBAND_SLAVE,INBAND_IOCTL_TYPE,INBAND_DEBUG);
	if( channel < 0 || inband_write(channel, get_random(65535), sycall_cmd_type, cmd, strlen(cmd), 0) < 0) {
		printf("inband sent remote command failed\n");
	}/* else {
		if( inband_rcv_data(channel, &syscall_cmd_type, &buf, -1) < 0 )
			printf("inband try to receive respone but failed\n");
		inband_close(channel);
	}*/
	inband_close(channel);
}

#ifdef INBAND_HOST
int InbandRequestIndication(
	char *                 ifname,
	char *		       out,
	int  *		       outlen)
{

	struct iwreq          wrq;
	DOT11_REQUEST	      * req;



  	/* Get wireless name */
	memset(wrq.ifr_name, 0, sizeof wrq.ifr_name);
  	strncpy(wrq.ifr_name, ifname, IFNAMSIZ);

	req = (DOT11_REQUEST *)malloc(MAXDATALEN);
	wrq.u.data.pointer = (caddr_t)req;
	req->EventId = DOT11_EVENT_REQUEST;
	wrq.u.data.length = sizeof(DOT11_REQUEST);

	//iw_message(MESS_DBG_IWCONTROL, "[RequestIndication] : Start\n");
	printf("\n[RequestIndication] : Start\n");
  	if(inband_ioctl(SIOCGIWIND, &wrq) < 0)
	{
    	// If no wireless name : no wireless extensions
		free(req);
		strerror(errno);
		printf("\n[RequestIndication] : Failed\n");
    		return(-1);
	}
  	else{
		//iw_message(MESS_DBG_IWCONTROL, "[RequestIndication]"," : Return\n");
		//iw_ctldump("RequestIndication", wrq.u.data.pointer, wrq.u.data.length, "receive message from driver");
		printf("\n[RequestIndication] : End with data len:%d\n",*outlen);
		memcpy(out, wrq.u.data.pointer, wrq.u.data.length);
		*outlen = wrq.u.data.length;
		//write(1, "RequestIndication<1>\n", sizeof("RequestIndication<1>\n"));

	}
	free(req);
	return 1;
}
#endif

