#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h> 
#include <sys/ioctl.h>
#include <errno.h>

#include <fcntl.h> 
#include <sys/types.h> 
#include <sys/uio.h> 

#include <sys/stat.h> //mark_file

#include "inband_if.h"

#define HOST_NETIF  ("br0")
#define SLAVE_MAC ("001234567899")
#define ETH_P_RTK		0x8899	// Realtek Remote Control Protocol (RRCP)
int hcd_inband_chan=0;

// command ID
#define id_ioctl						      0x01
#define id_systemcall						0x02
#define id_firm_upgrade					0x03
#define id_hostcmd_only                               0xff //only for new hostcmd implement ,if the cmd is not real "inband cmd"

// Command Table
struct inband_cmd_table_t {
	char cmd_id;
	const char *cmd;			// Input command string	
	int (*func)(int index,int argc, char *argv[]);
	void (*print_func)(char *para,char *data,int len);
	const char *msg;			// Help message
};

/*firmware define*/
#define __PACK__                        __attribute__ ((packed))

#define SIGNATURE_LEN                   4
#define FW_HEADER_WITH_ROOT     ((char *)"cr6c")
#define FW_HEADER                       ((char *)"cs6c")
#define ROOT_HEADER                     ((char *)"r6cr")
#define WEB_HEADER                      ((char *)"w6cg") //no ap sign now //mark_issue

typedef struct img_header {
        unsigned char signature[SIGNATURE_LEN] __PACK__;
        unsigned int startAddr __PACK__;
        unsigned int burnAddr __PACK__;
        unsigned int len __PACK__;
} IMG_HEADER_T, *IMG_HEADER_Tp;


unsigned int host_inband_seq=0; //use to do requset <-> response check

int cmd_systemcall(int index,int argc, char *argv[]);
int cmd_firmware_upgrade(int index,int argc, char *argv[]);
void print_systemcall(char *para,char *data,int len);


struct inband_cmd_table_t inband_cmd_table[]=
{
    //    
    {id_systemcall,"systemcall",     cmd_systemcall, print_systemcall,  "inband systemcall \'cmd_string\'"},
    {id_firm_upgrade,"firm_upgrade",     cmd_firmware_upgrade,   NULL,"inband firm_upgrade /var/linux.bin"}, 
    //{id_hostcmd_only,"send_conf",     cmd_sendconf,   NULL,"inband send_conf /etc/xxx.conf"},     
    {NULL,  NULL, NULL},
};


unsigned int get_random(int max)
{
	struct timeval tod;

	gettimeofday(&tod , NULL);
	srand(tod.tv_usec);
	return rand()%max;
}

void print_command_list(void)
{
    int i;

    printf("\n==========commands for debugging============\n");
    i = 0;
    while (inband_cmd_table[i].cmd != NULL) {         
            printf("%s\n", inband_cmd_table[i].msg);
        i++;
	}   
}

void print_systemcall(char *para,char *data,int len)
{
	int i=0;
	unsigned char *value;

	data[len] = '\0';
	printf("cmd_cfgread \n");	
	printf("%s",data);
}

static int host_inband_write(char cmd_id,char *data,int data_len)
{
	int ret;

	host_inband_seq = get_random(65536);

	ret = inband_write(hcd_inband_chan,host_inband_seq,cmd_id,data,data_len,0); //send request	
	return ret;
}

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
static int host_firmware_write(char *data,int len)
{
  unsigned int tx_seq,rx_seq;	 	
  int ret=0,count;
  char rx_cmd_type;
  char *buf_p;

   ret = host_inband_write(id_firm_upgrade,data,len); //send request

   tx_seq = host_inband_seq;	

   if(ret < 0)
   	return -1;	

   ret = inband_rcv_data_and_seq(hcd_inband_chan,&rx_seq,&rx_cmd_type,&buf_p,20000); //return data length 	

   if(ret < 0)
   {
	 	ret=-1;
		goto out;
   }	
   
   if( (rx_cmd_type == id_firm_upgrade) && (tx_seq == rx_seq))
   	printf("host_firmware_write ok!\n");
   else
   {
	ret=-1;
   	printf("host_firmware_write fail!\n");
   }		

out:
	inband_free_buf(buf_p, count);
	return ret;

}
	
static int rtk_firmware_update(char *upload_data, int upload_len)
{
int head_offset=0 ;
int isIncludeRoot=0;
int		 len; 
 int          numLeft;
int          numWrite;
IMG_HEADER_Tp pHeader;	
int fh;
int fwSizeLimit = 0x200000;
char buffer[100];
int flag=0;

while(head_offset <   upload_len) {
    
    pHeader = (IMG_HEADER_Tp) &upload_data[head_offset];
    len = pHeader->len;
#ifdef _LITTLE_ENDIAN_
    len  = DWORD_SWAP(len);
#endif

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
	
       //numWrite = write(fh, &(upload_data[locWrite+head_offset]), numLeft);
       //inband write , and check ack	
       if(host_firmware_write(&(upload_data[head_offset]),numLeft) < 0 )
       {
       	sprintf(buffer, "host_firmware_write! numLeft=0x%x", numLeft);
	   	goto ret_upload;
       }	
 	
	head_offset += len + sizeof(IMG_HEADER_T) ;
    }

  return 0;

  ret_upload:	
  	fprintf(stderr, "%s\n", buffer);	
	return -1;
} //while //sc_yang   

int cmd_firmware_upgrade(int index,int argc, char *argv[])
{
    int len,ret,count;
    //char data[1480];    	    
    char filename[50];
    char *data;
    int fd,rc;
   struct stat ffstat;
   int flen=0;   
    
  //sprintf(data,"%s",argv[2]); 
  sprintf(filename,"%s",argv[2]); 

  fd = open(filename, O_RDONLY);

  if (fd < 0)	{
		printf("Cannot Open file %s!\n", filename);
		return -1;
   }
  fstat(fd, &ffstat);
  flen = ffstat.st_size;
  printf("flen = %d \n",flen);

  if((data = (char *)malloc(flen)) == NULL)
  {
	printf("data buffer allocation failed!\n");
	return -1;
  }

 // rc = read(fd, data, 1480);
  rc = read(fd, data, flen);
   	
  //if (rc != 1480) {
  if (rc != flen) {
  	        printf("Reading error\n");
		 free(data);	//need free before return!!!!!	
  	        return -1;
   }

  close(fd);
  
  ret = rtk_firmware_update(data, flen);

  if(ret < 0)
  	printf("rtk_firmware_update fail \n");
  else
 	printf("rtk_firmware_update ok \n");  	
  
  free(data);	//need free before return!!!!!

  return ret;  
}

#if 0
int cmd_sendconf(int index,int argc, char *argv[])
{
    int len,ret,count;
    //char data[1480];    	
    char cmd_id,rx_cmd_type;
    char *buf_p;
    unsigned int tx_seq,rx_seq;	 	
    char filename[50];
    char *data;
    int fd,rc;
   struct stat ffstat;
   int flen=0;

   cmd_id = id_set_mib;
    
  //sprintf(data,"%s",argv[2]); 
  sprintf(filename,"%s",argv[2]); 

  fd = open(filename, O_RDONLY);

  if (fd < 0)	{
		printf("Cannot Open file %s!\n", filename);
		return -1;
   }
  fstat(fd, &ffstat);
  flen = ffstat.st_size;
  printf("flen = %d \n",flen);

  if((data = (char *)malloc(flen)) == NULL)
  {
	printf("data buffer allocation failed!\n");
	return -1;
  }

 // rc = read(fd, data, 1480);
  rc = read(fd, data, flen);
   	
  //if (rc != 1480) {
  if (rc != flen) {
  	        printf("Reading error\n");
		 free(data);	//need free before return!!!!!	
  	        return -1;
   }

  close(fd);
  //len = 1480;		
  len = flen;		
   tx_seq = host_inband_seq;	
   ret = host_inband_write(cmd_id,data,len); //send request

   free(data);	//need free before return!!!!!

   if(ret < 0)
   	return -1;
	
   //count = inband_rcv_data(&rx_cmd_type,&buf_p,1000); //return data length
   count = inband_rcv_data_and_seq(hcd_inband_chan,&rx_seq,&rx_cmd_type,&buf_p,3000); //return data length 	
   if(count < 0)
   {
	 	ret=-1;
		goto out;
   }	
   
   if( (rx_cmd_type == cmd_id) && (tx_seq == rx_seq))
   	printf("remote write ok!\n");
   else
   {
	ret=-1;
   	printf("remote write fail!\n");
   }		

   out:
   inband_free_buf(buf_p, count);
   	
   return ret;
}
#endif
int cmd_systemcall(int index,int argc, char *argv[])
{
    int len,ret,count;
    char data[1480];	
    char cmd_id,rx_cmd_type;
    char *buf_p;	
   unsigned int tx_seq,rx_seq;	   
   
   cmd_id = inband_cmd_table[index].cmd_id;
   
  if(argc > 3) 
  	sprintf(data,"%s %s",argv[2],argv[3]); 
  else
  	sprintf(data,"%s",argv[2]); 

   len = strlen(data);		

   
   ret = host_inband_write(cmd_id,data,len); //send request

  tx_seq = host_inband_seq;	
  
   if(ret < 0)
   	return -1;
	
   //count = inband_rcv_data(&rx_cmd_type,&buf_p,1000); //return data length
   count = inband_rcv_data_and_seq(hcd_inband_chan,&rx_seq,&rx_cmd_type,&buf_p,1000); //return data length 

   if(count < 0)
	 	return -1;
   //printf("count=%d rx_cmd_type=%x,cmd_id=%x,tx_seq=%d,rx_seq=%d",count,rx_cmd_type,cmd_id,tx_seq,rx_seq); 	

   if((rx_cmd_type == cmd_id) && (tx_seq == rx_seq))
   {
	if(inband_cmd_table[index].print_func)
		inband_cmd_table[index].print_func(data,buf_p,count);
   }
   else
	return -1;

   return 0;
    
}

int main(int argc, char *argv[])
{
	//int fdflags;
	//unsigned int arg;
	int i, ret;
	int chan;

    if (argc < 2) {
        print_command_list();
        return 0;
    }
    
    //open inband  	
	chan = inband_open(HOST_NETIF,SLAVE_MAC,ETH_P_RTK,0);
	if (chan < 0) {
		printf("open inband failed!\n");
		return -1;
	}
	hcd_inband_chan = chan;	   

    i = 0;
    while (inband_cmd_table[i].cmd != NULL) {
        if (0 == strcmp(argv[1], inband_cmd_table[i].cmd)) {
			if (inband_cmd_table[i].func) {
			    ret = inband_cmd_table[i].func(i,argc, argv);
		        if (ret > 0)
		            printf("OK\n");
		        else if (ret < 0)
		            printf("FAIL\n");
		    }
			break;
        }
        i++;
	}
	
	inband_close(hcd_inband_chan);
	
    return 0;
}
