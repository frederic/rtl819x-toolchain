#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "../inband_lib/ioh.h"
#include "../inband_lib/inband_if.h"
#include <linux/if.h>                   /* for IFNAMSIZ and co... */
#include "../inband_lib/wireless_copy.h"

#define CONFIG_HCD_FLASH_SUPPORT 1
#define ETH_P_RTK               0x8899
#define ETH_P_RTK_NETLINK 0x9000  //0x0101 ~ 0x01FF  , Experimental
#define LOCAL_NETIF  ("br0")

#define CMD_INBAND_IOCTL 0x01
#define CMD_INBAND_SYSTEMCALL 0x02
#define CMD_INBAND_FIRMWARE_DOWNLOAD 0x03

//netlink
#define NETLINK_ROUTE 0
#define RTMGRP_LINK 1

#define HDR_INFO_OFFSET 4
#define HDR_IOCTL_DATA_OFFSET 4+2

#ifndef WPAS_INB
#define WPAS_INB
#endif

int wext_sock=-1;
int ioctl_sock=-1;

struct sockaddr_nl
{
	sa_family_t nl_family;
	unsigned short nl_pad;
	unsigned int nl_pid;
	unsigned int nl_groups;
};

static struct ioh_class __ioh_nl_obj = {
        sockfd: 0,
        dev: {0},
        src_mac: {0},
        dest_mac: {0},
        socket_address: {0},
        tx_buffer: {0},
        rx_buffer: {0},
        tx_header: NULL,
        rx_header: NULL,
        tx_data: NULL,
        rx_data: NULL,
        debug: 0,
};

int hcd_inband_ioctl_chan=-1;

#ifdef CONFIG_HCD_FLASH_SUPPORT
//#define FLASH_DEVICE_KERNEL               ("/dev/mtd")
#define FLASH_DEVICE_KERNEL               ("/dev/mtdblock0")
#define FLASH_DEVICE_ROOTFS              ("/dev/mtdblock1")
#include <fcntl.h> 
#include "../goahead-2.1.1/LINUX/apmib.h"
static int fwChecksumOk(char *data, int len)
{
        unsigned short sum=0;
        int i;

        for (i=0; i<len; i+=2) {
#ifdef _LITTLE_ENDIAN_
                sum += WORD_SWAP( *((unsigned short *)&data[i]) );
#else
                sum += *((unsigned short *)&data[i]);
#endif

        }
        return( (sum==0) ? 1 : 0);
}

int rtk_FirmwareUpgrade(char *upload_data, int upload_len)
{
int head_offset=0 ;
int isIncludeRoot=0;
 int		 len;
    int          locWrite;
    int          numLeft;
    int          numWrite;
    IMG_HEADER_Tp pHeader;
	int flag=0, startAddr=-1, startAddrWeb=-1;	
    int fh;
   	unsigned char buffer[200];

	int fwSizeLimit = 0x200000;

while(head_offset <   upload_len) {
	
    locWrite = 0;
    pHeader = (IMG_HEADER_Tp) &upload_data[head_offset];
    len = pHeader->len;
    numLeft = len + sizeof(IMG_HEADER_T) ;   
    // check header and checksum
    if (!memcmp(&upload_data[head_offset], FW_HEADER, SIGNATURE_LEN) ||
			!memcmp(&upload_data[head_offset], FW_HEADER_WITH_ROOT, SIGNATURE_LEN))
    	flag = 1;
    else if (!memcmp(&upload_data[head_offset], WEB_HEADER, SIGNATURE_LEN))
    	flag = 2;
    else if (!memcmp(&upload_data[head_offset], ROOT_HEADER, SIGNATURE_LEN)){
    	flag = 3;
    	isIncludeRoot = 1;
    	}	
#if 0		
	else if ( !memcmp(&upload_data[head_offset], CURRENT_SETTING_HEADER_TAG, TAG_LEN) ||
				!memcmp(&upload_data[head_offset], CURRENT_SETTING_HEADER_FORCE_TAG, TAG_LEN) ||
				!memcmp(&upload_data[head_offset], CURRENT_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) ||
				!memcmp(&upload_data[head_offset], DEFAULT_SETTING_HEADER_TAG, TAG_LEN) ||
				!memcmp(&upload_data[head_offset], DEFAULT_SETTING_HEADER_FORCE_TAG, TAG_LEN) ||
				!memcmp(&upload_data[head_offset], DEFAULT_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) ||
				!memcmp(&upload_data[head_offset], HW_SETTING_HEADER_TAG, TAG_LEN) ||
				!memcmp(&upload_data[head_offset], HW_SETTING_HEADER_FORCE_TAG, TAG_LEN) ||
				!memcmp(&upload_data[head_offset], HW_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) )	 {
		int type, status, cfg_len;
		cfg_len = updateConfigIntoFlash(&upload_data[head_offset], 0, &type, &status);
		
		if (status == 0 || type == 0) { // checksum error
			strcpy(buffer, "Invalid configuration file!");
			goto ret_upload;
		}
		else { // upload success
			strcpy(buffer, "Update successfully!");
			head_offset += cfg_len;
			update_cfg = 1;
		}    	
		continue;
	}
#endif	
    else {
       	strcpy(buffer, "Invalid file format!");
		goto ret_upload;
    }

       if(len > fwSizeLimit){ //len check by sc_yang 
      		sprintf(buffer, "Image len exceed max size 0x%x ! len=0x%x",fwSizeLimit, len);
		goto ret_upload;
    }
    if ( (flag == 1) || (flag == 3)) {
    	if ( !fwChecksumOk(&upload_data[sizeof(IMG_HEADER_T)+head_offset], len)) {
      		sprintf(buffer, "Image checksum mismatched! len=0x%x, checksum=0x%x", len,
			*((unsigned short *)&upload_data[len-2]) );
		goto ret_upload;
	}
    }
    else {
    	char *ptr = &upload_data[sizeof(IMG_HEADER_T)+head_offset];
    	if ( !CHECKSUM_OK(ptr, len) ) {
     		sprintf(buffer, "Image checksum mismatched! len=0x%x", len);
		goto ret_upload;
	}
    }


    if(flag == 3)
    	fh = open(FLASH_DEVICE_ROOTFS, O_RDWR);
    else
    fh = open(FLASH_DEVICE_KERNEL, O_RDWR);

    if ( fh == -1 ) {
		printf("flag = %d\n",flag);
       	strcpy(buffer, "File open failed!");
		goto ret_upload;
    } else {

	if (flag == 1) {
		if ( startAddr == -1){
			//startAddr = CODE_IMAGE_OFFSET;
			startAddr = pHeader->burnAddr ;
		}
	}
	else if (flag == 3) {
		if ( startAddr == -1){
			startAddr = 0; // always start from offset 0 for 2nd FLASH partition
		}
	}
	else {
		if ( startAddrWeb == -1){
			//startAddr = WEB_PAGE_OFFSET;
			startAddr = pHeader->burnAddr ;
		}
		else
			startAddr = startAddrWeb;
	}
	lseek(fh, startAddr, SEEK_SET);
	if(flag == 3){
		fprintf(stderr,"\r\n close all interface");		
		locWrite += sizeof(IMG_HEADER_T); // remove header
		numLeft -=  sizeof(IMG_HEADER_T);
		//kill_processes();
		//sleep(2);
	}	
	//fprintf(stderr,"\r\n flash write");		
	numWrite = write(fh, &(upload_data[locWrite+head_offset]), numLeft);	
	//numWrite = numLeft;
	/*sprintf(buffer,"write flash flag=%d,locWrite+head_offset=%d,numLeft=%d,startAddr=%x\n",
				flag,locWrite+head_offset,numLeft,startAddr);
	fprintf(stderr, "%s\n", buffer);
	hex_dump(&(upload_data[locWrite+head_offset]),16);*/

	if (numWrite < numLeft) {

		sprintf(buffer, "File write failed. locWrite=%d numLeft=%d numWrite=%d Size=%d bytes.", locWrite, numLeft, numWrite, upload_len);

	goto ret_upload;
	}

	locWrite += numWrite;
 	numLeft -= numWrite;

	close(fh);

	head_offset += len + sizeof(IMG_HEADER_T) ;
	startAddr = -1 ; //by sc_yang to reset the startAddr for next image	
    }
} //while //sc_yang   	
  return 0;
  ret_upload:	
  	fprintf(stderr, "%s\n", buffer);	
	return -1;
}

static int host_firmware_receive(char *cmd , int cmd_len)
{
	int fd,ret=0;	
	char tmp[10];
	ret = rtk_FirmwareUpgrade(cmd,cmd_len);

	if(ret< 0)
		printf("rtk_FirmwareUpgrade fail !!!\n");
	else
		inband_write(hcd_inband_ioctl_chan,0,CMD_INBAND_FIRMWARE_DOWNLOAD,tmp,0,1);
	
	return ret;
}
#endif


static int wireless_nl_receive(int sock)
{
	char buf[1024];
	int left;
	struct sockaddr_nl from;
	socklen_t fromlen;
	struct ioh_class *ioh_nl_p = &__ioh_nl_obj;	

	fromlen = sizeof(from);

	left = recvfrom(sock, buf, sizeof(buf), MSG_DONTWAIT,
			(struct sockaddr *) &from, &fromlen);
	if (left < 0) {
		if (errno != EINTR && errno != EAGAIN)
			perror("recvfrom(netlink)");
		return;
	}
	//printf("wireless_nl_receive left=%d\n ",left);	
	//hex_dump(ioh_nl_p->dest_mac, 6);
	//send to Host with inbnad
	get_inband_destMac( hcd_inband_ioctl_chan,ioh_nl_p->dest_mac);// get the inband event destmac from inband ioctl.	

	memcpy(ioh_nl_p->tx_data, buf , left );
	ioh_send(ioh_nl_p,sizeof(struct ioh_header)+left);	
}

static void host_ioctl_receive(char *ioctl_data_p,int ioctl_data_len)
{
	int rx_len=0;
	int ret=-1;
	struct iwreq iwr;
	struct ifreq ifr;
	int ioctl_op=0;
	int error_code=0;
	int data_len=0;
	char *data_ptr;
	int opt_len=0;	
	unsigned char *is_iwreq;
	unsigned char *use_pointer;
	int reply=1; //good reply
        	 
#ifdef WPAS_INB
	struct iwreq *iwr_get;
	struct ifreq *ifr_get;
	char *data_get_ptr;
	int data_get_len;
	char *ioctl_data_p_tmp;
	int ioctl_ret = -1;
#endif        	 

	 //rcv ioctl succefully
	 memcpy(&ioctl_op,ioctl_data_p,sizeof(int));

	 is_iwreq = ioctl_data_p+HDR_INFO_OFFSET;
	 use_pointer = is_iwreq + 1;

	  //printf("host_ioctl_receive print_len=%d\n",64);
	// hex_dump(ioh_obj_p->rx_buffer, 64);	 

	 if( !(*is_iwreq) )//use ifreq
	 {
    	       //printf("ifreq op_code = %x \n",ioctl_op);
	 	data_ptr = (char *)&ifr;
		data_len = sizeof(struct ifreq);
    	       memcpy(data_ptr,ioctl_data_p+HDR_IOCTL_DATA_OFFSET,data_len);
	 }	 
	 else
	 {	
    	       //printf("iweq op_code = %x \n",ioctl_op);
		data_ptr = (char *)&iwr;
		data_len = sizeof(struct iwreq);
		memcpy(data_ptr,ioctl_data_p+HDR_IOCTL_DATA_OFFSET,data_len);
		

#ifdef WPAS_INB
		if( *use_pointer == 1 )
		{
			iwr.u.data.pointer = (caddr_t) (ioctl_data_p +HDR_IOCTL_DATA_OFFSET+data_len);
			//printf("use data pointer ioh_obj_p->rx_data=%x, iwr.u.data.pointer=%x , iwr.u.data.length=%d!!!\n",ioh_obj_p->rx_data,iwr.u.data.pointer,iwr.u.data.length);	  
			opt_len = iwr.u.data.length; //_Eric ?? use of opt_len ??
		}
		else if(*use_pointer == 2)
		{
			iwr.u.data.pointer = (char *)malloc(data_get_len);
			
			if(ioctl_op == SIOCGIWAP)
			{
				;
			}
			else if(ioctl_op == SIOCGIWESSID)
			{
				iwr.u.essid.length = ntohs(iwr.u.essid.length);
				iwr.u.essid.pointer = malloc(iwr.u.essid.length);
			}
			else if(ioctl_op == SIOCGIWSCAN)
			{
				iwr.u.data.length = ntohs(iwr.u.data.length);
				iwr.u.data.flags = ntohs(iwr.u.data.flags);	
				iwr.u.data.pointer = malloc(iwr.u.data.length);
			}
			else if(ioctl_op == SIOCGIWRANGE)
			{
				iwr.u.data.length = ntohs(iwr.u.data.length);
				iwr.u.data.flags = ntohs(iwr.u.data.flags);	
				iwr.u.data.pointer = malloc(iwr.u.data.length);
			}
			else if(ioctl_op == SIOCGIWMODE)
			{
				;
			}
				

		}
		else if(*use_pointer == 3)
		{
			if(ioctl_op == SIOCSIWAP)
			{
				//iwr.u.ap_addr.sa_data = (caddr_t) (ioctl_data_p + HDR_IOCTL_DATA_OFFSET + data_len);
				memcpy(iwr.u.ap_addr.sa_data ,(caddr_t) (ioctl_data_p + HDR_IOCTL_DATA_OFFSET + data_len), ETH_ALEN);
			}
			else if(ioctl_op == SIOCSIWESSID)
			{
				iwr.u.essid.pointer = (caddr_t) (ioctl_data_p + HDR_IOCTL_DATA_OFFSET + data_len);
				iwr.u.essid.length = ntohs(iwr.u.essid.length);
			}
			else if(ioctl_op == SIOCSIWMODE)
			{
				iwr.u.mode = ntohl(iwr.u.mode);
			}
		}
#else
		//check iwreq data pointer
		if( *use_pointer )
		{
   		      iwr.u.data.pointer = (caddr_t) (ioctl_data_p +HDR_IOCTL_DATA_OFFSET+data_len);
			//printf("use data pointer ioh_obj_p->rx_data=%x, iwr.u.data.pointer=%x , iwr.u.data.length=%d!!!\n",ioh_obj_p->rx_data,iwr.u.data.pointer,iwr.u.data.length);	  
 		      opt_len = iwr.u.data.length;
		}		
#endif
		
	 }	 			

	 ioctl_ret = ioctl(ioctl_sock, ioctl_op, data_ptr);
	 
	 if ( ioctl_ret < 0) {
		printf("\nioctl fail  ioctl_op=0x%x ret = 0x%x!!!!!!!!!!!!\n",ioctl_op, ioctl_ret);
		perror("error #");
		error_code = -1;
		reply =2 ; //bad reply
	}

#ifdef WPAS_INB

	if(*use_pointer == 2)
	{
		data_get_len = 0;
	
		if(ioctl_op == SIOCGIWAP)
		{
			iwr_get = (struct iwreq *)data_ptr;
			data_get_ptr = (char *)iwr_get->u.ap_addr.sa_data;		
			data_get_len = ETH_ALEN;
		}
		else if(ioctl_op == SIOCGIWESSID)
		{	
			iwr_get = (struct iwreq *)data_ptr;
			data_get_ptr = (char *)iwr_get->u.essid.pointer;		
			data_get_len = iwr_get->u.essid.length;
		}
		else if(ioctl_op == SIOCGIWSCAN)
		{
			iwr_get = (struct iwreq *)data_ptr;
			data_get_ptr = (char *)iwr_get->u.data.pointer;
			data_get_len = iwr_get->u.data.length;
		}
		else if(ioctl_op == SIOCGIWRANGE)
		{
			iwr_get = (struct iwreq *)data_ptr;
			data_get_ptr = (char *)iwr_get->u.data.pointer;
			data_get_len = iwr_get->u.data.length;
		}
		else if(ioctl_op == SIOCGIWMODE)
		{
			iwr_get = (struct iwreq *)data_ptr;
			iwr_get->u.mode = htonl(iwr_get->u.mode);
		}

		ioctl_data_len += (4 + data_get_len);

		if(ioctl_data_len >= BUF_SIZE)
			{
				ioctl_data_p_tmp = malloc(ioctl_data_len);
				memcpy(ioctl_data_p_tmp, ioctl_data_p, HDR_IOCTL_DATA_OFFSET);
				ioctl_data_p = ioctl_data_p_tmp;
			}

		memcpy(ioctl_data_p,(char *)&error_code,sizeof(int));	
		memcpy(ioctl_data_p+HDR_IOCTL_DATA_OFFSET,data_ptr,data_len);

		data_get_len = htonl(data_get_len); //_Eric ??
		memcpy(ioctl_data_p+HDR_IOCTL_DATA_OFFSET+data_len, &data_get_len, 4);
		data_get_len = ntohl(data_get_len); //_Eric ??
		memcpy(ioctl_data_p+HDR_IOCTL_DATA_OFFSET+data_len+4, data_get_ptr, data_get_len);
	}
	else
	{
	//send reply		
	//copy erro code to reply (now in rx_data)
	 memcpy(ioctl_data_p,(char *)&error_code,sizeof(int));	
	//printf("octl_data_p+HDR_IOCTL_DATA_OFFSET,data_ptr,data_len)\n");
	 memcpy(ioctl_data_p+HDR_IOCTL_DATA_OFFSET,data_ptr,data_len);	//mark_test	
	}

#else

	//send reply		
	//copy erro code to reply (now in rx_data)
	 memcpy(ioctl_data_p,(char *)&error_code,sizeof(int));	
	//printf("octl_data_p+HDR_IOCTL_DATA_OFFSET,data_ptr,data_len)\n");
	 memcpy(ioctl_data_p+HDR_IOCTL_DATA_OFFSET,data_ptr,data_len);	
//mark_test	

#endif
	inband_write(hcd_inband_ioctl_chan,0,CMD_INBAND_IOCTL,ioctl_data_p,ioctl_data_len,reply);
       
#ifdef WPAS_INB

	if(*use_pointer == 2)
	{
		if(ioctl_op == SIOCGIWAP)
		{
			;
		}
		else if(ioctl_op == SIOCGIWESSID)
		{	
			free(iwr.u.essid.pointer);
		}
		else if(ioctl_op == SIOCGIWSCAN)
		{
			free(iwr.u.essid.pointer);
		}
		else if(ioctl_op == SIOCGIWRANGE)
		{
			free(iwr.u.data.pointer);
		}
		else if(ioctl_op == SIOCGIWMODE)
		{
			;
		}

	if(ioctl_data_len >= BUF_SIZE)
		{
			free(ioctl_data_p_tmp);
		}
	}

#endif
       
}

static void host_systemcall_receive(char *data,int data_len)
{
	FILE *fp;
	char buffer[MAX_INBAND_PAYLOAD_LEN];
	int resp_len=0;
	int reply=1; //good reply
	//it's a string cmd
	data[data_len] = '\0';

	//printf("system call = %s\n",data);

	fp = popen(data, "r");

	if (fp)
	{
		while (!feof(fp)) 
		{
			// 4 bytes for FCS
			//if (resp_len + sizeof(*obj->rx_header) + 4 == sizeof(obj->rx_buffer))
			//break;	// out of buffer

			resp_len += fread(	&buffer[resp_len],
							sizeof(char), 
							MAX_INBAND_PAYLOAD_LEN, 
							fp);
		}
	}
	else
	{
		printf("popen error!!!\n");
		reply=2; //bad reply
	}	
	//send reply
	inband_write(hcd_inband_ioctl_chan,0,CMD_INBAND_SYSTEMCALL,buffer,resp_len,reply);

}
static void inband_packet_process(unsigned char cmd,char *data,int data_len)
{
	//printf("inband_packet_process cmd=%x \n",cmd);
	switch(cmd){

		case CMD_INBAND_IOCTL :
		 	host_ioctl_receive(data,data_len);		    
		break;	

		case CMD_INBAND_SYSTEMCALL:
			host_systemcall_receive(data,data_len);			
		break;	

		case CMD_INBAND_FIRMWARE_DOWNLOAD:
			host_firmware_receive(data,data_len);			
		break;	

		default:
			printf("unndefined inband cmd !!\n ");

	}
}

static int event_handler()
{
	fd_set fsRead;
	int res;
	int     iFD_SETSIZE = 0;	
	int  ib_ioctl_sock = get_inband_socket(hcd_inband_ioctl_chan);	
	char *ioctl_data_p;
	int ioctl_data_len=0;	
	
	while(1)
	{ 	   
		
		FD_ZERO ( &fsRead);

		FD_SET( wext_sock, &fsRead);
		iFD_SETSIZE = wext_sock;
		
		FD_SET( ib_ioctl_sock, &fsRead);
		iFD_SETSIZE = (iFD_SETSIZE > ib_ioctl_sock)?iFD_SETSIZE:ib_ioctl_sock ;
		iFD_SETSIZE += 1;

		res = select( iFD_SETSIZE, &fsRead, NULL, NULL, NULL);
		
		if(res <= 0)
			continue;

		if(FD_ISSET(wext_sock, &fsRead)) //rcv netlink
		{
		     wireless_nl_receive(wext_sock);
		}

		if(FD_ISSET(ib_ioctl_sock , &fsRead)) //rcv ioctl
		{			
			unsigned char cmd_type;
			ioctl_data_p = NULL;			 
		        ioctl_data_len = inband_rcv_data(hcd_inband_ioctl_chan,&cmd_type,&ioctl_data_p,NULL); //try to rcv inband data

		        if(ioctl_data_len >= 0)
		        	inband_packet_process(cmd_type,ioctl_data_p,ioctl_data_len);

		        inband_free_buf(ioctl_data_p, ioctl_data_len);
		}

	}

}

static int
wireless_nl_init()
{	
	int s;
	struct sockaddr_nl local;

	s = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (s < 0) {
		perror("socket(PF_NETLINK,SOCK_RAW,NETLINK_ROUTE)");
		return -1;
	}
	memset(&local, 0, sizeof(local));
	local.nl_family = AF_NETLINK;
	local.nl_groups = RTMGRP_LINK;
	if (bind(s, (struct sockaddr *) &local, sizeof(local)) < 0) {
		perror("bind(netlink)");
		close(s);
		return -1;
	}

	wext_sock = s;

	return 0;
}

int main(int argc, char *argv[])
{	
	int ret1=-1,chan=-1;
	//open raw sokcet for nelink notify to HOST

	ret1 = ioh_open(&__ioh_nl_obj, LOCAL_NETIF, NULL, ETH_P_RTK_NETLINK, 0);

	if( ret1 < 0 )
	{
		printf("open inband event fail!!!\n");
		return -1;	
	}	

	//open raw sokcet for ioctl from HOST request
	//ret2 = ioh_open(&__ioh_ioctl_obj, LOCAL_NETIF, NULL, ETH_P_RTK_IOCTL, 0);
	chan = inband_open(LOCAL_NETIF,NULL,ETH_P_RTK,0);

        if(chan < 0)
        {
               printf(" inband_open failed!\n");
                return -1;
        }
        else
	        hcd_inband_ioctl_chan = chan;
	
	printf(" inband_open all done!\n");
	
	ioctl_sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (ioctl_sock < 0) {
		perror("socket[PF_INET,SOCK_DGRAM]");
		return -1;
	}
	//realtek_wireless_event_init
	ret1 = wireless_nl_init();

	event_handler();

	return ret1;
}

