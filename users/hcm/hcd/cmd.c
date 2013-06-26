/*================================================================*/
/* System Include Files */

#include <stdio.h> 
#include <stdlib.h> 
#include <fcntl.h> 
#include <string.h>
#include <sys/types.h> 
#include <sys/uio.h> 
#include <unistd.h> 
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/wait.h> 
#include <net/if.h>
#include <linux/if_packet.h>
#include "../../inband_lib/wireless_copy.h"
/*================================================================*/
/* Local Include Files */

#include <sys/stat.h>

#include "hcd.h"
#include "mib.h"
#include "cmd.h"
#include "wlan_if.h"
//#include "inband_if.h"
#define MAX_INBAND_PAYLOAD_LEN 1480


/*=Local variables===============================================================*/
WLAN_RATE_T rate_11n_table_20M_LONG[]={
	{MCS0, 	"6.5"},
	{MCS1, 	"13"},
	{MCS2, 	"19.5"},
	{MCS3, 	"26"},
	{MCS4, 	"39"},
	{MCS5, 	"52"},
	{MCS6, 	"58.5"},
	{MCS7, 	"65"},
	{MCS8, 	"13"},
	{MCS9, 	"26"},
	{MCS10, 	"39"},
	{MCS11, 	"52"},
	{MCS12, 	"78"},
	{MCS13, 	"104"},
	{MCS14, 	"117"},
	{MCS15, 	"130"},
	{0}
};
WLAN_RATE_T rate_11n_table_20M_SHORT[]={
	{MCS0, 	"7.2"},
	{MCS1, 	"14.4"},
	{MCS2, 	"21.7"},
	{MCS3, 	"28.9"},
	{MCS4, 	"43.3"},
	{MCS5, 	"57.8"},
	{MCS6, 	"65"},
	{MCS7, 	"72.2"},
	{MCS8, 	"14.4"},
	{MCS9, 	"28.9"},
	{MCS10, 	"43.3"},
	{MCS11, 	"57.8"},
	{MCS12, 	"86.7"},
	{MCS13, 	"115.6"},
	{MCS14, 	"130"},
	{MCS15, 	"144.5"},
	{0}
};
WLAN_RATE_T rate_11n_table_40M_LONG[]={
	{MCS0, 	"13.5"},
	{MCS1, 	"27"},
	{MCS2, 	"40.5"},
	{MCS3, 	"54"},
	{MCS4, 	"81"},
	{MCS5, 	"108"},
	{MCS6, 	"121.5"},
	{MCS7, 	"135"},
	{MCS8, 	"27"},
	{MCS9, 	"54"},
	{MCS10, 	"81"},
	{MCS11, 	"108"},
	{MCS12, 	"162"},
	{MCS13, 	"216"},
	{MCS14, 	"243"},
	{MCS15, 	"270"},
	{0}
};
WLAN_RATE_T rate_11n_table_40M_SHORT[]={
	{MCS0, 	"15"},
	{MCS1, 	"30"},
	{MCS2, 	"45"},
	{MCS3, 	"60"},
	{MCS4, 	"90"},
	{MCS5, 	"120"},
	{MCS6, 	"135"},
	{MCS7, 	"150"},
	{MCS8, 	"30"},
	{MCS9, 	"60"},
	{MCS10, 	"90"},
	{MCS11, 	"120"},
	{MCS12, 	"180"},
	{MCS13, 	"240"},
	{MCS14, 	"270"},
	{MCS15, 	"300"},
	{0}
};

#define CFG_FILE_IN "/etc/Wireless/realtekap.conf"

char *lan_link_spec[2] = {"DOWN","UP"};
char *lan_speed_spec[3] = {"10M","100M","1G"};
char *enable_spec[2] = {"DIsable","Enable"};
#define FLASH_DEVICE_KERNEL               ("/dev/mtd")
#define FLASH_DEVICE_ROOTFS              ("/dev/mtdblock1")

#define HDR_INFO_OFFSET 4
#define HDR_IOCTL_DATA_OFFSET 4+2
#define LOCAL_NETIF  ("br0")
static int ioctl_sock=-1;
static int hcd_inband_ioctl_chan=-1;
#define CMD_INBAND_IOCTL 0x03
#define CMD_INBAND_SYSTEMCALL 0x04


extern int hcd_inband_chan;
/**/
static int _get_mib(char *cmd , int cmd_len);
static int _set_mib(char *cmd , int cmd_len);
static int _set_mibs(char *cmd , int cmd_len);
//static int _set_mibs(void);
static int _getstainfo(char *cmd , int cmd_len);
static int _getassostanum(char *cmd , int cmd_len);
static int _getbssinfo(char *cmd , int cmd_len);
static int _sysinit(char *cmd , int cmd_len);
static int _getstats(char *cmd , int cmd_len);	
static int _getlanstatus(char *cmd , int cmd_len);
//static int _sendfile(char *cmd , int cmd_len);
static int _firm_upgrade(char *cmd , int cmd_len);
static int host_ioctl_receive(char *ioctl_data_p,int ioctl_data_len);
static int host_systemcall_receive(char *data,int data_len);



struct cmd_entry cmd_table[]={ \
/*Action cmd - ( name, func) */
	CMD_DEF(set_mib, _set_mib),
	CMD_DEF(get_mib, _get_mib),	
	CMD_DEF(getstainfo, _getstainfo),
	CMD_DEF(getassostanum, _getassostanum),
	CMD_DEF(getbssinfo, _getbssinfo),
	CMD_DEF(sysinit, _sysinit),
	CMD_DEF(getstats, _getstats),
	CMD_DEF(getlanstatus, _getlanstatus),
	CMD_DEF(set_mibs, _set_mibs),
    //CMD_DEF(sendfile, _sendfile),
#ifdef CONFIG_HCD_FLASH_SUPPORT        
    CMD_DEF(firm_upgrade, _firm_upgrade),
#endif
	CMD_DEF(ioctl, host_ioctl_receive),
	CMD_DEF(syscall, host_systemcall_receive),
	/* last one type should be LAST_ENTRY - */   
	{0}
};

int do_cmd(int id , char *cmd ,int cmd_len ,int relply)
{
	int i=0,ret=-1,len=0;

	while (cmd_table[i].id != LAST_ENTRY_ID) {
		if ((cmd_table[i].id == id))	{
			ret = cmd_table[i].func(cmd,cmd_len);
			break;
		}	
		i++;
	}
	//no reply
	if(!relply || cmd_table[i].id == id_ioctl )
		return ret;

	if(ret > MAX_INBAND_PAYLOAD_LEN) //mark_issue , it will be reply in it's func in command table
		return ret;

	//reply rsp pkt
	if (ret >= 0) { 
		if (ret == 0) { 
			cmd[0] = '\0';
			ret = 1;
		}

		inband_write(hcd_inband_chan,0,id,cmd,ret,1); //good reply
	}
	else{ //error rsp		
		cmd[0] = (unsigned char)( ~ret + 1);			
		inband_write(hcd_inband_chan,0,id,cmd,1,2); //error reply
	}			
	
	return ret;
}

#if 0 //mark_firm
static inline int CHECKSUM_OK(unsigned char *data, int len)
{
        int i;
        unsigned char sum=0;

        for (i=0; i<len; i++)
                sum += data[i];

        if (sum == 0)
                return 1;
        else
                return 0;
}
#endif

#ifdef CONFIG_HCD_FLASH_SUPPORT
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

static int _firm_upgrade(char *cmd , int cmd_len)
{
	int fd,ret=0;		
	ret = rtk_FirmwareUpgrade(cmd,cmd_len);		

	return ret;
}
#endif


static int host_ioctl_receive(char *ioctl_data_p,int ioctl_data_len)
{
	int rx_len=0;
	int ret=-1;
	struct iwreq iwr;
	struct ifreq ifr;
	struct iw_point *data;
	int ioctl_op=0;
	int error_code=0;
	int data_len=0;
	char *data_ptr;
	int opt_len=0;	
	unsigned char *is_iwreq;
	unsigned char *use_pointer;
	//int reply=1; //good reply
			 

	 //rcv ioctl succefully
	 memcpy(&ioctl_op,ioctl_data_p,sizeof(int));

	 is_iwreq = ioctl_data_p+HDR_INFO_OFFSET;
	 use_pointer = is_iwreq + 1;

	 //printf("%s print_len=%d\n",__FUNCTION__,ioctl_data_len);
	 //hex_dump(ioctl_data_p, ioctl_data_len);

	 if( !(*is_iwreq) )//use ifreq
	 {
		//printf("ifreq op_code = %x \n",ioctl_op);
		data_ptr = (char *)&ifr;
		data_len = sizeof(struct ifreq);
			   memcpy(data_ptr,ioctl_data_p+HDR_IOCTL_DATA_OFFSET,data_len);
	 }	 
	 else
	 {	
		//printf("iwreq op_code = %x \n",ioctl_op);
		data_ptr = (char *)&iwr;
		data_len = sizeof(struct iwreq);
		memcpy(data_ptr,ioctl_data_p+HDR_IOCTL_DATA_OFFSET,data_len);
		
		//check iwreq data pointer
		if( *use_pointer )
		{
			  iwr.u.data.pointer = (caddr_t) (ioctl_data_p +HDR_IOCTL_DATA_OFFSET+data_len);
			  //printf("use data pointer ioh_obj_p->rx_data=%x, iwr.u.data.pointer=%x , iwr.u.data.length=%d!!!\n",ioctl_data_p,iwr.u.data.pointer,iwr.u.data.length);
			  opt_len = iwr.u.data.length;
		}		

		if( ioctl_op == 0xffffffff ){
			int n=0;
			struct ifreq ifr;
			struct sockaddr_ll addr;
			int ifindex;
			int sock=-1;

			sock = socket(PF_PACKET, SOCK_RAW, 0);
			if (socket < 0) {
				perror("socket[PF_PACKET,SOCK_RAW]");
				return -1;
			}	
			memset(&ifr, 0, sizeof(ifr));
			strncpy(ifr.ifr_name, "wlan0", sizeof(ifr.ifr_name));

			while (ioctl(ioctl_sock , SIOCGIFINDEX, &ifr) != 0) {
				printf(" ioctl(SIOCGIFINDEX) failed!\n");
				sleep(1);
			}
			ifindex = ifr.ifr_ifindex;	
			memset(&addr, 0, sizeof(addr));
			addr.sll_family = AF_PACKET;
			addr.sll_ifindex = ifindex;
			if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
				perror("bind[PACKET]");
				return -1;
			}
			
		 	//send wlan
		 	if ((n = send(sock, iwr.u.data.pointer, iwr.u.data.length, 0)) < 0) {
				printf("send_wlan failed!");
				return -1;
			}

			close(sock);
			return n;
		 }
	 }

	 if( ioctl_sock < 0 ) {
	 	ioctl_sock = socket(PF_INET, SOCK_DGRAM, 0);
		if (ioctl_sock < 0) {
			printf("\nioctl socket open failed !!!!!!!!!!!!\n");
			perror("socket[PF_INET,SOCK_DGRAM]");
			return -1;
		}
	}

	 if (ioctl(ioctl_sock, ioctl_op, data_ptr) < 0) {
		printf("\nioctl fail  ioctl_op=%x!!!!!!!!!!!!\n",ioctl_op);
		error_code = -1;
		//reply =2 ; //bad reply
		ret = 2;
	}

	//send reply		
	//copy erro code to reply (now in rx_data)
	if( *use_pointer ) {
		//printf("%s %d iwr.u.data.length:%d\n",__FUNCTION__,__LINE__,iwr.u.data.length);
		memcpy(ioctl_data_p,(char *)&error_code,sizeof(int));	
		memcpy(ioctl_data_p+HDR_IOCTL_DATA_OFFSET+sizeof(iwr),iwr.u.data.pointer,iwr.u.data.length);	//mark_test 
		ret = iwr.u.data.length;
		ioctl_data_len += iwr.u.data.length;
	} else {
		memcpy(ioctl_data_p,(char *)&error_code,sizeof(int));
		memcpy(ioctl_data_p+HDR_IOCTL_DATA_OFFSET,data_ptr,data_len);	//mark_test
	}

	//return ret;
	/*
	 if( hcd_inband_ioctl_chan < 0 ) {
	 	hcd_inband_ioctl_chan = inband_open(LOCAL_NETIF,NULL,ETH_P_RTK_NOTIFY,0);

		if(hcd_inband_ioctl_chan < 0)
		{
		       printf(" inband_open failed!\n");
		        return -1;
		}
	}*/
	//inband_write(hcd_inband_ioctl_chan,0,CMD_INBAND_IOCTL,ioctl_data_p,ioctl_data_len,reply);   
	//inband_write(hcd_inband_ioctl_chan,0,CMD_INBAND_IOCTL,ioctl_data_p,ioctl_data_len,ret);
	inband_write(hcd_inband_chan,0,CMD_INBAND_IOCTL,ioctl_data_p,ioctl_data_len,ret); //good reply
	//inband_write(hcd_inband_ioctl_chan,0,CMD_INBAND_IOCTL,ioctl_data_p,ioctl_data_len,ret);
	return ret;
}

static int host_systemcall_receive(char *cmd , int cmd_len) //mark_cmd
{
	char *param;		
	char *tmp;
	FILE *fp;
	int resp_len=0;
	char res[64*200]; //mark_issue
	
	cmd[cmd_len]='\0';//mark_patch

    //printf("system call = %s\n",data);
	fp = popen(cmd, "r");
	if (fp)
	{
		while (!feof(fp)) 
		{
			// 4 bytes for FCS
			//if (resp_len + sizeof(*obj->rx_header) + 4 == sizeof(obj->rx_buffer))
			//break;	// out of buffer

			resp_len += fread(	&res[resp_len],
							sizeof(char), 
							MAX_INBAND_PAYLOAD_LEN, 
							fp);
		}
	}
	else
	{
		printf("popen error!!!\n");
		return -1; //error reply in do_cmd
	}		
	//syscmd reply here , not in do_cmd bcz some reply is very long (site_survey!)
	inband_write(hcd_inband_chan,0,id_syscall,res,resp_len,1); 

	//mark_issue , always syscmd reply here so , ret set to MAX_INBAND_PAYLOAD_LEN +1
	return MAX_INBAND_PAYLOAD_LEN+1;
}



#if 0
static int _sendfile(char *cmd , int cmd_len)
{
	int fd,ret=0;
	//char *filename="/var/hostapd.conf";
	char *filename="/var/linux.bin";
	char test_buf[64];

	fd = open(filename, O_RDWR | O_CREAT);
	printf("_sendfile cmd_len = %d \n",cmd_len);
	
	if (fd < 0)	{
		printf("Cannot Open file %s!\n", filename);
		return -1;
	}

	write( fd, cmd, cmd_len); 
	
	close(fd);

	//-------read file for test
	fd = open(filename, O_RDONLY);
	if (fd < 0)	{
		printf("Cannot Open file %s!\n", filename);
		return -1;
       }

	 read(fd, test_buf, 64);
	 hex_dump( test_buf, 64);
	 close(fd);
	return ret;
}
#endif

static int _set_mib(char *cmd , int cmd_len)
{
	char *param,*val,*next;	
	int ret = RET_OK,access_flag=ACCESS_MIB_SET | ACCESS_MIB_BY_NAME;
	char *intf, *tmp;

	cmd[cmd_len]='\0';//mark_patch
	do {
		intf = cmd;
		param = strchr(intf,'_')+1;
		if( !strchr(param,'=') ){
			printf("invalid command format:%s\n",cmd);
			return -1;
		}
		val = strchr(param,'=')+1;

		intf[param-intf-1] = '\0';
		param[val-param-1] = '\0';

		if( !strcmp(intf,"wps") )
			break;

		cmd = strchr(val,'\n');
		if(cmd)
		    *cmd = '\0';
		//printf(">>> set %s=%s to %s\n",param,val,intf);
		ret = access_config_mib(access_flag,param,val,intf);  //ret the

		if(cmd && (*(cmd+1) != '\0')) 
		    cmd++;
		else
			break;	
	} while(cmd);

	//free(intf);
	return ret;
}

int config_from_local(unsigned char *ptr)
{
	int fh = 0;
	struct stat status;

	if (stat(CFG_FILE_IN, &status) < 0) {
		printf("stat() error [%s]!\n", CFG_FILE_IN);
		return -1;
	}

	fh = open(CFG_FILE_IN,O_RDONLY);
	if( fh < 0 ){
		printf("File open failed\n");
		return -1;
	}

	/*
	ptr = (unsigned char *)calloc(0,status.st_size);
	if( !ptr ){
		printf("%d:memory alloc failed\n",status.st_size);
		return -1;
	}
	*/
	lseek(fh, 0L, SEEK_SET);

	if (read(fh, ptr, status.st_size) != status.st_size) {		
		printf("read() error [%s]!\n", CFG_FILE_IN);
		return -1;	
	}
	close(fh);
	return 0;
}

int parse_then_set(unsigned char *str_start, unsigned char *str_end)
{
	int ret;
	unsigned char *param, *value, *intf, line[100] = {0};

	memcpy(line,str_start,str_end-str_start);
	param = line;
	intf = param;
	param = strchr(intf,'_')+1;
	value = strchr(param,'=')+1;
	intf[param-intf-1] = '\0';
	param[value-param-1] = '\0';
	printf(">>> %s %s %s = %s \n",__FUNCTION__,intf,param,value);
	ret = access_config_mib(ACCESS_MIB_BY_NAME|ACCESS_MIB_SET,param,value,intf);
	return ret;
}

static int _set_mibs(char *cmd , int cmd_len)
//static int _cfgswrite(void)
{
	unsigned char cmd_buf[20480] = {0};
	unsigned char line[100] = {0}, *str_start, *str_end;
	cmd[cmd_len]='\0';//mark_patch
	if( config_from_local(cmd_buf) < 0 ) {
		printf("Read config from %s failed\n","/etc/Wireless/realtekap.conf");
		return -1;
	}

	str_start = cmd_buf;
	str_end = strchr(cmd_buf,'\n');
	while( str_end ){
		if( *str_start != '#' && *str_start != ' ' && *str_start != '\n' ){
			parse_then_set(str_start,str_end);
		}
		str_start = str_end+1;
		str_end = strchr(str_start,'\n');
	}

	free(cmd_buf);
	return 0;
}


static int _get_mib(char *cmd , int cmd_len)
{
	char *param;	
	int ret = RET_OK,access_flag=ACCESS_MIB_GET | ACCESS_MIB_BY_NAME;
	char *intf, *tmp;
	cmd[cmd_len]='\0';//mark_patch
	intf = cmd;
	param = strchr(intf,'_')+1;
	intf[param-intf-1] = '\0';
	//printf(">>> read %s from %s\n",param,intf);
	ret = access_config_mib(access_flag,param,cmd,intf);  //return value in cmd

	//free(intf);
	return ret;
}

static int _sysinit(char *cmd , int cmd_len)
{
	//now , only support init all
	cmd[cmd_len]='\0';//mark_patch	
	if(!strcmp(cmd, "all"))
		init_system(INIT_ALL);		
	else if(!strcmp(cmd, "lan"))		
		init_system(INIT_ALL);
	else if(!strcmp(cmd, "wlan"))
		init_system(INIT_ALL);
	else
		return -1;

	return 0;
}
//sta_info frame , {sta_num}{sta_info1}{sta_info2}........... 
//first byre will be total sta_info numer in this reply
static int _getstainfo(char *cmd , int cmd_len)
{
	char *buff,*tmpInfo;	
	WLAN_STA_INFO_Tp pInfo;
	int sta_num =0,i,ret;
	cmd[cmd_len]='\0';//mark_patch
	buff = calloc(1, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1));
	tmpInfo = cmd +1 ; // first byte reserve for sta_info num

	if ( buff == 0 ) {
		printf("Allocate buffer failed!\n");
		return -1;
	}

	if ( get_wlan_stainfo(cmd,  (WLAN_STA_INFO_Tp)buff ) < 0 ) {
		printf("Read wlan sta info failed!\n");

		return -1;
	}

	for (i=1; i<=MAX_STA_NUM; i++) {
		pInfo = (WLAN_STA_INFO_Tp)&buff[i*sizeof(WLAN_STA_INFO_T)];
		if (pInfo->aid && (pInfo->flag & STA_INFO_FLAG_ASOC)) {//sta exist
			memcpy(tmpInfo+ (sta_num*sizeof(WLAN_STA_INFO_T)),(char *)pInfo, sizeof(WLAN_STA_INFO_T));
			sta_num++;
		}		
	}	

	cmd[0]= (unsigned char)(sta_num&0xff);
	ret = sta_num*sizeof(WLAN_STA_INFO_T) + 1;		
	
	return ret;
}
static int _getassostanum(char *cmd , int cmd_len)
{
	int num=0;
	cmd[cmd_len]='\0';//mark_patch
	if (get_wlan_stanum(cmd, &num) < 0)
		return -1;

	cmd[0]=(unsigned char)(num&0xff);	

	return 1; // return len=1  to show sta num
	
}
static int _getbssinfo(char *cmd , int cmd_len)
{	
	WLAN_BSS_INFO_T bss;
	int bss_len=sizeof(WLAN_BSS_INFO_T);
	cmd[cmd_len]='\0';//mark_patch
	if ( get_wlan_bssinfo(cmd, &bss) < 0)
			return -1;

	memcpy(cmd,(char *)&bss,bss_len);
	
	return bss_len;
}

int get_lanport_stats(int portnum,struct port_statistics *port_stats)
{
	struct ifreq ifr;
	 int sockfd;
	 char *name="eth0";	 
	 struct port_statistics stats;
	 unsigned int *args;	

	 if(portnum > 5)
	 	return -1;	 	
	 
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
 	{
      		printf("fatal error socket\n");
      		return -3;
       }
	args = (unsigned int *)&stats;
	((unsigned int *)(&ifr.ifr_data))[0] =(struct port_statistics *)&stats;
	*args = portnum;
	
	strcpy((char*)&ifr.ifr_name, name);       

    if (ioctl(sockfd, RTL819X_IOCTL_READ_PORT_STATS, &ifr)<0)
    {
      		printf("device ioctl:");
      		close(sockfd);
     		 return -1;
     }
     close(sockfd);   	     
     memcpy(port_stats,(char *)&stats,sizeof(struct port_statistics));

    return 0;	 

}

int get_lanport_status(int portnum,struct lan_port_status *port_status)
{
	struct ifreq ifr;
	 int sockfd;
	 char *name="eth0";	 
	 struct lan_port_status status;
	 unsigned int *args;	

	 if(portnum > 5)
	 	return -1;	 	
	 
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
 	{
      		printf("fatal error socket\n");
      		return -3;
       }
	args = (unsigned int *)&status;
	((unsigned int *)(&ifr.ifr_data))[0] =(struct lan_port_status *)&status;
	*args = portnum;
	
	strcpy((char*)&ifr.ifr_name, name);       

    if (ioctl(sockfd, RTL819X_IOCTL_READ_PORT_STATUS, &ifr)<0)
    {
      		printf("device ioctl:");
      		close(sockfd);
     		 return -1;
     }
     close(sockfd);   	
     memcpy((char *)port_status,(char *)&status,sizeof(struct lan_port_status));

    return 0;	 

}

static int get_dev_fields(int type, char *bp, struct user_net_device_stats *pStats)
{
    switch (type) {
    case 3:
	sscanf(bp,
	"%Lu %Lu %lu %lu %lu %lu %lu %lu %Lu %Lu %lu %lu %lu %lu %lu %lu",
	       &pStats->rx_bytes,
	       &pStats->rx_packets,
	       &pStats->rx_errors,
	       &pStats->rx_dropped,
	       &pStats->rx_fifo_errors,
	       &pStats->rx_frame_errors,
	       &pStats->rx_compressed,
	       &pStats->rx_multicast,

	       &pStats->tx_bytes,
	       &pStats->tx_packets,
	       &pStats->tx_errors,
	       &pStats->tx_dropped,
	       &pStats->tx_fifo_errors,
	       &pStats->collisions,
	       &pStats->tx_carrier_errors,
	       &pStats->tx_compressed);
	break;

    case 2:
	sscanf(bp, "%Lu %Lu %lu %lu %lu %lu %Lu %Lu %lu %lu %lu %lu %lu",
	       &pStats->rx_bytes,
	       &pStats->rx_packets,
	       &pStats->rx_errors,
	       &pStats->rx_dropped,
	       &pStats->rx_fifo_errors,
	       &pStats->rx_frame_errors,

	       &pStats->tx_bytes,
	       &pStats->tx_packets,
	       &pStats->tx_errors,
	       &pStats->tx_dropped,
	       &pStats->tx_fifo_errors,
	       &pStats->collisions,
	       &pStats->tx_carrier_errors);
	pStats->rx_multicast = 0;
	break;

    case 1:
	sscanf(bp, "%Lu %lu %lu %lu %lu %Lu %lu %lu %lu %lu %lu",
	       &pStats->rx_packets,
	       &pStats->rx_errors,
	       &pStats->rx_dropped,
	       &pStats->rx_fifo_errors,
	       &pStats->rx_frame_errors,

	       &pStats->tx_packets,
	       &pStats->tx_errors,
	       &pStats->tx_dropped,
	       &pStats->tx_fifo_errors,
	       &pStats->collisions,
	       &pStats->tx_carrier_errors);
	pStats->rx_bytes = 0;
	pStats->tx_bytes = 0;
	pStats->rx_multicast = 0;
	break;
    }
    return 0;
}

static char *get_name(char *name, char *p)
{
    while (isspace(*p))
	p++;
    while (*p) {
	if (isspace(*p))
	    break;
	if (*p == ':') {	/* could be an alias */
	    char *dot = p, *dotname = name;
	    *name++ = *p++;
	    while (isdigit(*p))
		*name++ = *p++;
	    if (*p != ':') {	/* it wasn't, backup */
		p = dot;
		name = dotname;
	    }
	    if (*p == '\0')
		return NULL;
	    p++;
	    break;
	}
	*name++ = *p++;
    }
    *name++ = '\0';
    return p;
}


void fill_statistics(struct user_net_device_stats *pStats, struct port_statistics *statistics)
{
	statistics->rx_bropkts = 0;
	statistics->rx_bytes = pStats->rx_bytes;
	statistics->rx_discard = pStats->rx_dropped;
	statistics->rx_error = pStats->rx_errors;
	statistics->rx_mulpkts = pStats->rx_multicast;
	statistics->rx_unipkts = pStats->rx_packets-pStats->rx_multicast;
	statistics->tx_bropkts = 0;
	statistics->tx_bytes = pStats->tx_bytes;
	statistics->tx_discard = pStats->tx_dropped;
	statistics->tx_error = pStats->tx_errors;
	statistics->tx_mulpkts = 0;
	statistics->tx_unipkts = pStats->tx_packets;
}


int get_wlan_stats(char *intf, struct port_statistics *statistic)
{
 	FILE *fh;
  	char buf[512];
	int type;
	struct user_net_device_stats pStats;

	fh = fopen(_PATH_PROCNET_DEV, "r");
	if (!fh) {
		printf("Warning: cannot open %s\n",_PATH_PROCNET_DEV);
		return -1;
	}
	fgets(buf, sizeof buf, fh);	/* eat line */
	fgets(buf, sizeof buf, fh);

  	if (strstr(buf, "compressed"))
		type = 3;
	else if (strstr(buf, "bytes"))
		type = 2;
	else
		type = 1;

	while (fgets(buf, sizeof buf, fh)) {
		char *s, name[40];
		s = get_name(name, buf);
		if ( strcmp(intf, name))
			continue;
		get_dev_fields(type, s, &pStats);
		fill_statistics(&pStats,statistic);
		fclose(fh);
		return 0;
    }
	fclose(fh);
	return -1;
}


int portname_to_num(char *name)
{
	int portnum=0;
	if(!strncmp(name,"p0",2)) 
		portnum=0;
	else if(!strncmp(name,"p1",2)) 
		portnum=1;
	else if(!strncmp(name,"p2",2)) 
		portnum=2;
	else if(!strncmp(name,"p3",2)) 
		portnum=3;
	else if(!strncmp(name,"p4",2)) 
		portnum=4;
	else if(!strncmp(name,"p5",2)) 
		portnum=5;

	return portnum;
}

static int _getlanstatus(char *cmd , int cmd_len)
{
	struct lan_port_status port_status;
	int len=sizeof(struct lan_port_status);
	int portnum;
	cmd[cmd_len]='\0';//mark_patch
	portnum = portname_to_num(cmd);
	
	if ( get_lanport_status(portnum, &port_status) < 0)
			return -1;
	
	memcpy(cmd,(char *)&port_status,len);
	
	return len;
}

static int _getstats(char *cmd , int cmd_len)
{
	struct port_statistics statistics;
	int len=sizeof(struct port_statistics);
	int portnum;
	char *intf, *tmp;
	cmd[cmd_len]='\0';//mark_patch
	//printf(">>> %s\n",cmd);
	
	if(!strncmp(cmd,"p",1)) {
		portnum = portname_to_num(cmd);
		
		if ( get_lanport_stats(portnum, &statistics) < 0)
				return -1;		
		memcpy(cmd,(char *)&statistics,len);
	} else {
		//get statistics of wlan
		if ( get_wlan_stats(cmd, &statistics) < 0)
				return -1;

		memcpy(cmd,(char *)&statistics,len);
	}
	
	return len;
}


//-------------------------------------------------------------------

void print_stainfo(char *stainfo_rsp)
{
	WLAN_STA_INFO_Tp pInfo;
	int i=0,rateid=0,sta_num=stainfo_rsp[0];
	char mode_buf[20],txrate[20];


	if(sta_num <= 0)
		printf("No Associated  station now!!!!\n ",i+1);
	for(i=0;i<sta_num;i++)
	{
		pInfo = (WLAN_STA_INFO_T *)&(stainfo_rsp[i*sizeof(WLAN_STA_INFO_T)+1]);
		printf("-----------------------------------------------\n");
		printf("station No.%d info\n",i+1);
		printf("MAC address : %02x:%02x:%02x:%02x:%02x:%02x \n",pInfo->addr[0],pInfo->addr[1],pInfo->addr[2],pInfo->addr[3],pInfo->addr[4],pInfo->addr[5]);

		if(pInfo->network & BAND_11N)
			sprintf(mode_buf, "%s", " 11n");
		else if (pInfo->network & BAND_11G)
			sprintf(mode_buf,"%s",  " 11g");	
		else if (pInfo->network & BAND_11B)
			sprintf(mode_buf, "%s", " 11b");
		else if (pInfo->network& BAND_11A)
			sprintf(mode_buf, "%s", " 11a");	
		else
			sprintf(mode_buf, "%s", " ---");	

		printf("Mode:%s \n",mode_buf);

		printf("TX packets:%d , RX packets:%d \n",pInfo->tx_packets, pInfo->rx_packets);

		if((pInfo->txOperaRates & 0x80) != 0x80){	
			if(pInfo->txOperaRates%2){
				sprintf(txrate, "%d%s",pInfo->txOperaRates/2, ".5"); 
			}else{
				sprintf(txrate, "%d",pInfo->txOperaRates/2); 
			}
		}else{
			if((pInfo->ht_info & 0x1)==0){ //20M
				if((pInfo->ht_info & 0x2)==0){//long
					for(rateid=0; rateid<16;rateid++){
						if(rate_11n_table_20M_LONG[rateid].id == pInfo->txOperaRates){
							sprintf(txrate, "%s", rate_11n_table_20M_LONG[rateid].rate);
							break;
						}
					}
				}else if((pInfo->ht_info & 0x2)==0x2){//short
					for(rateid=0; rateid<16;rateid++){
						if(rate_11n_table_20M_SHORT[rateid].id == pInfo->txOperaRates){
							sprintf(txrate, "%s", rate_11n_table_20M_SHORT[rateid].rate);
							break;
						}
					}
				}
			}else if((pInfo->ht_info & 0x1)==0x1){//40M
				if((pInfo->ht_info & 0x2)==0){//long
					
					for(rateid=0; rateid<16;rateid++){
						if(rate_11n_table_40M_LONG[rateid].id == pInfo->txOperaRates){
							sprintf(txrate, "%s", rate_11n_table_40M_LONG[rateid].rate);
							break;
						}
					}
				}else if((pInfo->ht_info & 0x2)==0x2){//short
					for(rateid=0; rateid<16;rateid++){
						if(rate_11n_table_40M_SHORT[rateid].id == pInfo->txOperaRates){
							sprintf(txrate, "%s", rate_11n_table_40M_SHORT[rateid].rate);
							break;
						}
					}
				}
			}
		   }
		printf("TX Rate : %s \n",txrate);
		printf("Sleep : %s \n",( (pInfo->flag & STA_INFO_FLAG_ASLEEP) ? "yes" : "no"));
		printf("Expired time : %d seconds \n",pInfo->expired_time/100);

	}

}

void print_bssinfo(char *bssinfo_rsp)
{
	WLAN_BSS_INFO_Tp bssInfo;
	char *pMsg;
	
	bssInfo = (WLAN_BSS_INFO_T *)bssinfo_rsp;
	printf("BSSID : %02x:%02x:%02x:%02x:%02x:%02x \n",bssInfo->bssid[0],bssInfo->bssid[1],bssInfo->bssid[2],
													bssInfo->bssid[3],bssInfo->bssid[4],bssInfo->bssid[5]);
	printf("SSID : %s \n",bssInfo->ssid);

	switch (bssInfo->state) {
		case STATE_DISABLED:
			pMsg = "Disabled";
			break;
		case STATE_IDLE:
			pMsg = "Idle";
			break;
		case STATE_STARTED:
			pMsg = "Started";
			break;
		case STATE_CONNECTED:
			pMsg = "Connected";
			break;
		case STATE_WAITFORKEY:
			pMsg = "Waiting for keys";
			break;
		case STATE_SCANNING:
			pMsg = "Scanning";
			break;
		default:
			pMsg=NULL;
		}

	printf("State : %s \n",pMsg);

	printf("Channel : %d \n",bssInfo->channel);		
	
}

void print_port_status(char *status)
{
	struct lan_port_status *port_status;
	
	port_status = (struct lan_port_status *)status;
	
	printf("Link = %s\n",lan_link_spec[port_status->link]);	
	printf("Speed = %s\n",lan_speed_spec[port_status->speed]);
	printf("Nway mode = %s\n",enable_spec[port_status->nway]);	
	printf("Duplex = %s\n",enable_spec[port_status->duplex]);
		
}
void print_port_stats(char *stats)
{
	struct port_statistics *port_stats;
	port_stats = (struct port_statistics *)stats;

	printf("rx bytes=%d\n",port_stats->rx_bytes);
	printf("rx unicast packets =%d\n",port_stats->rx_unipkts);
	printf("rx multicast packets =%d\n",port_stats->rx_mulpkts);
	printf("rx brocast packets =%d\n",port_stats->rx_bropkts);
	printf("rx discard packets  =%d\n",port_stats->rx_discard);
	printf("rx error packets  =%d\n",port_stats->rx_error);
	printf("tx bytes=%d\n",port_stats->tx_bytes);
	printf("tx unicast packets =%d\n",port_stats->tx_unipkts);
	printf("tx multicast packets =%d\n",port_stats->tx_mulpkts);
	printf("tx brocast packets =%d\n",port_stats->tx_bropkts);
	printf("tx discard packets  =%d\n",port_stats->tx_discard);
	printf("tx error packets  =%d\n",port_stats->tx_error);	
}

