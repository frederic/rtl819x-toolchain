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
#include <net80211/ieee80211.h>
#include <net80211/ieee80211_crypto.h>
#include <net80211/ieee80211_ioctl.h>

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

#ifdef IEEE80211_IOCTL_SETWMMPARAMS
/* Assume this is built against realtek-ng */
#define REALTEK_NG
#endif /* IEEE80211_IOCTL_SETWMMPARAMS */
//#define RTL8192CD_IOCTL_DEL_STA	0x89f7
//#define SIOCGIWIND      		0x89ff
//#defein SIOCIWLASTPRIV		0x8BFF

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
	unsigned char cmd_type=0x01, buf[BUF_SIZE], *rx_buf;
	struct iwreq *local_iwr;
	struct timeval start;
	int ioctl_op_endian;
	int local_iwr_pointer;
	struct ifreq *ifr_ptr;
#ifdef WPAS_INB
	char* data_set_ptr;
	int	  data_set_len = 0;
	char* data_get_ptr;
	int   data_get_len = 0;

	int	  rx_retry_cnt = 0;
	int   tx_retry_cnt = 0;
#endif

/*_Eric WPAS_INB ========

[iw]
Byte(0): 0, u.name
	      1, u.data.pointer
	      2, u."Others" - get
	      3, u."Others" - set
Byte(1): 0, ifreq
	      1, iwreq
	   
=====================*/

	switch(ioctl_op) {

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
		//WPAS_INB case SIOCGIWRANGE:
		//WPAS_INB case SIOCSIWESSID:
		//WPAS_INB case SIOCGIWESSID:
		case SIOCSIWENCODE:
		case IEEE80211_IOCTL_WDSADDMAC:
		case IEEE80211_IOCTL_WDSDELMAC:
		case IEEE80211_IOCTL_GET_APPIEBUF:
		case 0x89f7: //RTL8192CD_IOCTL_DEL_STA
		case 0x89ff: //SIOCGIWIND
		case 0x8bff:	//SIOCIWLASTPRIV
#ifdef WPAS_INB
		case IEEE80211_IOCTL_SETOPTIE://0x8bee
#endif
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
#ifdef WPAS_INB 
		case SIOCSIWSCAN:
			iw = 0x10;
			break;

		case SIOCGIWAP:
		case SIOCGIWESSID:
		case SIOCGIWSCAN:
		case SIOCGIWRANGE:
		case SIOCGIWMODE:
			iw = 0x12;
			break;
			
		case SIOCSIWAP:
		case SIOCSIWESSID:
		case SIOCSIWMODE:
			iw = 0x13;
			break;
#endif
		default:
			printf("Unknown ioctl number:%d\n",ioctl_op);
			return -1;
	}

	inband_channel = inband_open(INBAND_INTF, INBAND_SLAVE, INBAND_IOCTL_TYPE, INBAND_DEBUG);
	if( inband_channel < 0 ) {
		printf("ioctl(inband channel open) failed \n");
		goto out;
	}
	
	ioctl_op_endian = htonl(ioctl_op);//mark_endian
	memset(buf,0,BUF_SIZE);
	memcpy(buf,(unsigned char *)&ioctl_op_endian,INBAND_IOCTLTYPE_LEN);	
#ifdef WPAS_INB
	if( (iw & 0x1) && ((iw != 0x13) && (iw != 0x12)) )
#else
	if( iw & 0x1 )
#endif
	{
		local_iwr = (struct iwreq *)req;
		ext_len = local_iwr->u.data.length;
		local_iwr_pointer = (int)local_iwr->u.data.pointer;		
		local_iwr->u.data.length = htons(ext_len); 
		local_iwr->u.data.flags = htons(local_iwr->u.data.flags);
		//memcpy(INBAND_PENDING_START(buf),local_iwr->u.data.pointer, local_iwr->u.data.length);
		memcpy(INBAND_PENDING_START(buf),local_iwr->u.data.pointer, ext_len);
	}

#ifdef WPAS_INB
	if( iw == 0x12 ) {
		
		local_iwr = (struct iwreq *)req;

		if(ioctl_op == SIOCGIWAP)
		{
			;
		}
		else if(ioctl_op == SIOCGIWESSID)
		{
			local_iwr = (struct iwreq *)req;
			local_iwr->u.essid.flags = htons(local_iwr->u.essid.flags);
			local_iwr->u.essid.length = htons(local_iwr->u.essid.length);
		}
		else if(ioctl_op == SIOCGIWSCAN)
		{
			local_iwr = (struct iwreq *)req;
			local_iwr->u.data.length = htons(local_iwr->u.data.length); 
			local_iwr->u.data.flags = htons(local_iwr->u.data.flags);
			ext_len = 0;
		}
		else if(ioctl_op == SIOCGIWRANGE)
		{
			local_iwr = (struct iwreq *)req;
			local_iwr->u.data.length = htons(local_iwr->u.data.length); 
			local_iwr->u.data.flags = htons(local_iwr->u.data.flags);
			ext_len = 0;
		}
		else if(ioctl_op == SIOCGIWMODE)
		{
			;
		}
		
	}

	if( iw == 0x13 ) {
		
		local_iwr = (struct iwreq *)req;
		data_set_len = 0;

		if(ioctl_op == SIOCSIWAP)
		{
			data_set_len = ETH_ALEN;
			data_set_ptr = (char *)local_iwr->u.ap_addr.sa_data;
		}
		else if(ioctl_op == SIOCSIWESSID)
		{
			data_set_len = local_iwr->u.essid.length;
			data_set_ptr = (char*)local_iwr->u.essid.pointer;			
			local_iwr->u.essid.length = htons(local_iwr->u.essid.length); 
		}
		else if(ioctl_op == SIOCSIWMODE)
		{
			local_iwr->u.mode = htonl(local_iwr->u.mode);
		}


		memcpy(INBAND_PENDING_START(buf), data_set_ptr, data_set_len);
		ext_len = data_set_len;
			
	}
#endif

	
	memcpy(buf+INBAND_IOCTLHDR_LEN,(unsigned char *)req,IWREQ_LEN);
	

#ifdef WPAS_INB
	if(iw >= 0x10)
		buf[INBAND_IOCTLTYPE_LEN] = 0x1;
	else
		buf[INBAND_IOCTLTYPE_LEN] = 0x0;

	buf[INBAND_IOCTLTYPE_LEN+1] = iw&0x3;
#else
	buf[INBAND_IOCTLTYPE_LEN] = iw&0x10>>4;
	buf[INBAND_IOCTLTYPE_LEN+1] = iw&0x1;
#endif


	sq = get_random(65536);

#ifdef WPAS_INB
	tx_retry_cnt = 0;
			
tx_retry:

	rx_retry_cnt = 0;
	tx_retry_cnt ++;
		
	if(tx_retry_cnt > 3)
		{	
			printf("CAN NOT RECEIVE INBAND RESPONSE !!!! EXIT ~~~ \n");
			ret = -1;
			goto out;
		}
#endif

	//printf("inband ioctl %d %d >>> \n", sq, ext_len);


	if( inband_write(inband_channel, sq, cmd_type, buf, INBAND_IOCTLHDR_LEN+IWREQ_LEN+ext_len, 0) < 0) {
		printf("inband ioctl message send failed\n");
		goto out;
	} 
	else {
		gettimeofday(&start,NULL);
rx_retry:
		
		//_Eric timeout shall set ??
		if( inband_rcv_data_and_seq(inband_channel, &rcv_sq, &cmd_type, &rx_buf, 500000) < 0 && !rx_expired(&start) ) {
			printf("inband ioctl message not receive response\n");
			
#ifdef WPAS_INB
			rx_retry_cnt ++;

			if(rx_retry_cnt >= 3)
				goto tx_retry;

			sleep(1);
			goto rx_retry;
#else
			ret = -1;
			goto out;
#endif
		}
		//printf(" >>> inband ioctl %d \n",rcv_sq);


#ifdef WPAS_INB
		if( sq != rcv_sq )
			goto tx_retry;
#else
		if( sq != rcv_sq )
			goto rx_retry;
#endif


		ret = *(int *)rx_buf;
		
#ifdef WPAS_INB
		ret = ntohl(ret);
#endif


#ifdef WPAS_INB
		if(iw != 0x12)
#endif
		memcpy((unsigned char *)req,rx_buf+INBAND_IOCTLHDR_LEN,IWREQ_LEN);	

#ifdef WPAS_INB
		if( (iw & 0x1) && (iw != 0x12) && (iw != 0x13))
#else
		if( iw & 0x1 )
#endif
		{
			local_iwr = (struct iwreq *)req;			
			memcpy((int *)local_iwr_pointer, INBAND_PENDING_START(rx_buf), ext_len);
			local_iwr->u.data.length = ext_len; 
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
			

#ifdef WPAS_INB
		if(iw == 0x12)
		{
			memcpy(&data_get_len, (rx_buf + INBAND_IOCTLHDR_LEN + IWREQ_LEN + ext_len), 4);
			data_get_len = ntohl(data_get_len);
			data_get_ptr = (char *)(rx_buf + INBAND_IOCTLHDR_LEN + IWREQ_LEN + ext_len + 4);
			
			switch(ioctl_op){
			case SIOCGIWAP:
			local_iwr = (struct iwreq *)req;				
			memcpy(local_iwr->u.ap_addr.sa_data, data_get_ptr, data_get_len);
			break;
			case SIOCGIWESSID:
			local_iwr = (struct iwreq *)req;
			local_iwr->u.essid.length = data_get_len;
			memcpy(local_iwr->u.essid.pointer, data_get_ptr, data_get_len);
			break;
			case SIOCGIWSCAN:
			local_iwr = (struct iwreq *)req;
			local_iwr->u.data.length = data_get_len;
			memcpy(local_iwr->u.data.pointer, data_get_ptr, data_get_len);
			break;
			case SIOCGIWRANGE:
			local_iwr = (struct iwreq *)req;
			local_iwr->u.data.length = data_get_len;
			memcpy(local_iwr->u.data.pointer, data_get_ptr, data_get_len);
			break;
			case SIOCGIWMODE:
			local_iwr = (struct iwreq *)req;				
			local_iwr->u.mode = ntohl(local_iwr->u.mode);
			break;
			default:
			break;
			}
		}

#endif
			
	}
out:
	if( inband_channel >= 0 )
		inband_close(inband_channel);

	return ret;
}


int
inband_remote_cmd(unsigned char *cmd)
{
	unsigned char cmd_type=0x02, *buf;
	unsigned int channel = -1;

	channel = inband_open(INBAND_INTF,INBAND_SLAVE,INBAND_IOCTL_TYPE,0);
	if( channel < 0 || inband_write(channel, get_random(65535), cmd_type, cmd, strlen(cmd), 0) < 0) {
		printf("inband sent remote command failed\n");
	} else {
		if( inband_rcv_data(channel, &cmd_type, &buf, -1) < 0 )
			printf("inband try to receive respone but failed\n");
		inband_close(channel);
	}
}



