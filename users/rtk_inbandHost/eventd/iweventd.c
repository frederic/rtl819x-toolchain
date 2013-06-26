#include "eventd.h"

#define FALSE 0
#define TRUE 1

#define WIFI_SIMPLE_CONFIG 1

typedef enum{
        DOT11_EVENT_NO_EVENT = 1,
        DOT11_EVENT_REQUEST = 2,
        DOT11_EVENT_ASSOCIATION_IND = 3,
        DOT11_EVENT_ASSOCIATION_RSP = 4,
        DOT11_EVENT_AUTHENTICATION_IND = 5,
        DOT11_EVENT_REAUTHENTICATION_IND = 6,
        DOT11_EVENT_DEAUTHENTICATION_IND = 7,
        DOT11_EVENT_DISASSOCIATION_IND = 8,
        DOT11_EVENT_DISCONNECT_REQ = 9,
        DOT11_EVENT_SET_802DOT11 = 10,
        DOT11_EVENT_SET_KEY = 11,
        DOT11_EVENT_SET_PORT = 12,
        DOT11_EVENT_DELETE_KEY = 13,
        DOT11_EVENT_SET_RSNIE = 14,
        DOT11_EVENT_GKEY_TSC = 15,
        DOT11_EVENT_MIC_FAILURE = 16,
        DOT11_EVENT_ASSOCIATION_INFO = 17,
        DOT11_EVENT_INIT_QUEUE = 18,
        DOT11_EVENT_EAPOLSTART = 19,
//2003-07-30 ------------
        DOT11_EVENT_ACC_SET_EXPIREDTIME = 31,
        DOT11_EVENT_ACC_QUERY_STATS = 32,
        DOT11_EVENT_ACC_QUERY_STATS_ALL = 33,
//-----------------------

// --- 2003-08-04 ---
        DOT11_EVENT_REASSOCIATION_IND = 34,
        DOT11_EVENT_REASSOCIATION_RSP = 35,
//-----------------------
        DOT11_EVENT_STA_QUERY_BSSID = 36,
        DOT11_EVENT_STA_QUERY_SSID = 37,

// jimmylin: pass EAP packet by event queue
        DOT11_EVENT_EAP_PACKET = 41,

#ifdef RTL_WPA2
        DOT11_EVENT_EAPOLSTART_PREAUTH = 45,
        DOT11_EVENT_EAP_PACKET_PREAUTH = 46,
#endif        

#ifdef RTL_WPA2_CLIENT
	DOT11_EVENT_WPA2_MULTICAST_CIPHER = 47,       
#endif

	DOT11_EVENT_WPA_MULTICAST_CIPHER = 48,       

#ifdef AUTO_CONFIG
	DOT11_EVENT_AUTOCONF_ASSOCIATION_IND = 50,
	DOT11_EVENT_AUTOCONF_ASSOCIATION_CONFIRM = 51,
	DOT11_EVENT_AUTOCONF_PACKET = 52,
	DOT11_EVENT_AUTOCONF_LINK_IND = 53,
#endif

#ifdef WIFI_SIMPLE_CONFIG
	DOT11_EVENT_WSC_SET_IE = 55,		
	DOT11_EVENT_WSC_PROBE_REQ_IND = 56,
	DOT11_EVENT_WSC_PIN_IND = 57,
	DOT11_EVENT_WSC_ASSOC_REQ_IE_IND = 58,
#ifdef CONFIG_IWPRIV_INTF
	DOT11_EVENT_WSC_START_IND = 70,
	//EV_MODE, EV_STATUS, EV_MEHOD, EV_STEP, EV_OOB
	DOT11_EVENT_WSC_MODE_IND = 71,
	DOT11_EVENT_WSC_STATUS_IND = 72,
	DOT11_EVENT_WSC_METHOD_IND = 73,
	DOT11_EVENT_WSC_STEP_IND = 74,
	DOT11_EVENT_WSC_OOB_IND = 75,
#endif  //ifdef CONFIG_IWPRIV_INTF
#endif

        DOT11_EVENT_MAX = 59,
} DOT11_EVENT;

#define MAX_WLAN_INTF 1
#define RWFIFOSIZE	1600	// jimmylin: org: 160, for passing EAP packet by event queue
#define FIFO_TYPE_DLISTEN	0x02
static char event_SendBuf[RWFIFOSIZE];
#define MAX_TCP_LEN 65535

static Dot1x_RTLDListener RTLDListenerWscd[MAX_WLAN_INTF];

typedef struct _DOT11_WSC_PIN_IND{
	unsigned char	EventId;
	unsigned char	IsMoreEvent;
	unsigned char	code[256];
} DOT11_WSC_PIN_IND;


static void show_help(void)
{
	printf("  Usage: %s [argument]...\n", PROGRAM_NAME);
	printf("    Where arguments is optional as:\n");
	printf("\t-w WPS event listen mode\n");
	printf("\t-r Radius event listen modd\n");
	printf("\twps trigger WPS by pin/pbc if wscd exist\n");
	printf("\n");	
}


static int pidfile_acquire(char *pidfile)
{
	int pid_fd;
	struct stat status;
	FILE *pid_file=NULL;
	unsigned char line[100];

	if(pidfile == NULL)
                return -1;

	if( !stat(pidfile,&status) ) {
		pid_file = fopen(pidfile,"r");
		if( pid_file )
			fgets(line,sizeof line, pid_file);
		else
			goto out;
		sscanf(line,"%d",&pid_fd);
		kill(pid_fd,SIGTERM);
	}

	pid_fd = open(pidfile, O_CREAT | O_WRONLY, 0644);
	if (pid_fd < 0) 
		printf("Unable to open pidfile %s\n", pidfile);
	else 
		lockf(pid_fd, F_LOCK, 0);
out:
	if(pid_file)
		fclose(pid_file);
	return pid_fd;
}

static void pidfile_write_release(int pid_fd)
{
	FILE *out;

	if(pid_fd < 0)
		return;

	if((out = fdopen(pid_fd, "w")) != NULL) {
		fprintf(out, "%d\n", getpid());
		fclose(out);
	}
	lockf(pid_fd, F_UNLCK, 0);
	close(pid_fd);
}


static int init_socket(struct network_info_t *network_info, int type)
{
	int flags,i;
	struct ioh_class *ioh_ptr;

	switch(type){
		case 1:
		case 3:
			network_info->channel = inband_open(INTERFACE,SLAVE_MAC,ETH_P_RTK_NOTIFY,0);
			break;
		case 2:
			network_info->channel = inband_open(INTERFACE,DEST_MAC,ETH_P_RTK_NOTIFY,0);
			//network_info->channel = inband_open(INTERFACE,DEST_MAC,ETH_P_RTK,0);
			
			//ioh_ptr = get_chan_obj(network_info->channel);
			network_info->inband_sock = get_inband_socket(network_info->channel);
			memset( &network_info->localaddr, 0, sizeof(network_info->localaddr));
			network_info->localaddr.sin_family = AF_INET;
			network_info->localaddr.sin_addr.s_addr = htonl(INADDR_ANY);
			network_info->localaddr.sin_port = htons(0);

			for( i=0;i<INTERFACE_NUM*MBSSID;i++) {
				if( network_info->udp_info[i].if_index ){
					network_info->udp_info[i].radsrvaddr.sin_family = PF_INET;
					network_info->udp_info[i].radsrvaddr.sin_port = htons( RADIUS_PORT );
					network_info->udp_info[i].udpsock = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP);
					if ( network_info->udp_info[i].udpsock == -1 ){
						//debug_message("lib1x_nal_initialize: Could not open Radius Authentication UDP socket !");
						printf("lib1x_nal_initialize: Could not open Radius Authentication UDP socket !");
						goto err_out;
					}
					
					if ( bind( network_info->udp_info[i].udpsock, (struct sockaddr *) &network_info->localaddr, sizeof(network_info->localaddr)) != 0){
						//debug_message("Could not BIND Authentication server UDP socket.");
						printf("Could not BIND Authentication server UDP socket.");
						goto err_out;
					}

					// wait till succeed
					while (1) {
						if (connect( network_info->udp_info[i].udpsock, (struct sockaddr *)&network_info->udp_info[i].radsrvaddr, sizeof(network_info->udp_info[i].radsrvaddr) ) != 0 )
						{
							debug_message("lib1x_nal_connect: Could not connect to Authentication Server . ");
							sleep(1);
						} else {
							debug_message("Index:%d Radius connected !!!\n",network_info->udp_info[i].if_index);
							break;
						}
					}

					flags = fcntl(network_info->udp_info[i].udpsock, F_GETFL );
					if ( fcntl( network_info->udp_info[i].udpsock, F_SETFL, flags | O_NONBLOCK) != 0 ) {
						debug_message("lib1x_nal_connect : FCNTL failed on UDP socket" );
						goto err_out;
					}
				}
			}
			break;
		default:
			debug_message("Unknown usage type!!\n");
			goto err_out;
	}

	return 0;

err_out:
	return -1;
}

static void do_wifi_event(struct network_info_t *info, char *data,int data_len)
{	
	unsigned char szEvent[64];
	int iSend = FALSE;	// iSend = 1 for test
	int isWscdEvt = FALSE;
	char *wps_config="/var/wps.conf";
	int fd,ret=0;
	int iRet = 0;
	int retVal = 0;

	memcpy(event_SendBuf+5,data,data_len);
	debug_message("event type=%d\n",event_SendBuf[5]);

	if(event_SendBuf[5] != 0)
	{
		memset(szEvent, 0, sizeof szEvent);
		switch(event_SendBuf[5])
		{
		case	DOT11_EVENT_ASSOCIATION_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "ASSOCIATION_IND");
			iSend = TRUE;
			break;

		case	DOT11_EVENT_REASSOCIATION_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "REASSOCIATION_IND");
			iSend = TRUE;
			break;

		case 	DOT11_EVENT_AUTHENTICATION_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "AUTHENTICATION_IND");
			iSend = TRUE;
			break;

		case	DOT11_EVENT_REAUTHENTICATION_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "REAUTHENTICATION_IND");
			iSend = TRUE;
			break;

		case	DOT11_EVENT_DEAUTHENTICATION_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "DEAUTHENTICATION_IND");
			iSend = TRUE;
			break;

		case	DOT11_EVENT_DISASSOCIATION_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "DISASSOCIATION_IND");
			iSend = TRUE;
			break;

		case	DOT11_EVENT_MIC_FAILURE:
			sprintf(szEvent, (char*)"Receive Event %s", "MIC_FAILURE");
			iSend = TRUE;
			break;

		case	DOT11_EVENT_EAPOLSTART:
			sprintf(szEvent, (char*)"Receive Event %s", "EAPOL_START");
			iSend = TRUE;
			break;

		case	DOT11_EVENT_EAP_PACKET:
			sprintf(szEvent, (char*)"Receive Event %s", "EAP_PACKET");
#ifdef WIFI_SIMPLE_CONFIG
			isWscdEvt = TRUE;
#endif
			iSend = TRUE;
			break;
#ifdef RTL_WPA2
		case	DOT11_EVENT_EAPOLSTART_PREAUTH:
			sprintf(szEvent, (char*)"Receive Event %s", "EAPOLSTART_PREAUTH");
			iSend = TRUE;
			break;

		case	DOT11_EVENT_EAP_PACKET_PREAUTH:
			sprintf(szEvent, (char*)"Receive Event %s", "EAP_PACKET_PREAUTH");
			iSend = TRUE;
			break;
#endif

#ifdef RTL_WPA2_CLIENT
		case	DOT11_EVENT_WPA2_MULTICAST_CIPHER:
			sprintf(szEvent, (char*)"Receive Event %s", "WPA2_MULTICAST_CIPHER");
			iSend = TRUE;
			break;
#endif

		case	DOT11_EVENT_WPA_MULTICAST_CIPHER:
			sprintf(szEvent, (char*)"Receive Event %s", "WPA_MULTICAST_CIPHER");
			iSend = TRUE;
			break;

#ifdef AUTO_CONFIG
		case	DOT11_EVENT_AUTOCONF_ASSOCIATION_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "AUTOCONF_ASSOC_IND");
			iSend = TRUE;
			isAutoconfEvt = 1;
			break;

		case	DOT11_EVENT_AUTOCONF_ASSOCIATION_CONFIRM:
			sprintf(szEvent, (char*)"Receive Event %s", "AUTOCONF_ASSOC_CONFIRM");
			iSend = TRUE;
			isAutoconfEvt = 1;
			break;

		case	DOT11_EVENT_AUTOCONF_PACKET:
			sprintf(szEvent, (char*)"Receive Event %s", "AUTOCONF_PACKET");
			iSend = TRUE;
			isAutoconfEvt = 1;
			break;

		case	DOT11_EVENT_AUTOCONF_LINK_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "AUTOCONF_LINK_IND");
			iSend = TRUE;
			isAutoconfEvt = 1;
			break;
#endif

#ifdef WIFI_SIMPLE_CONFIG
		case	DOT11_EVENT_WSC_PIN_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "WSC_PIN_IND");
			isWscdEvt = TRUE;
			break;
#ifdef CONFIG_IWPRIV_INTF
		case	DOT11_EVENT_WSC_START_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "WSC_START_IND");
			isWscdEvt = TRUE;
			break;
		//EV_MODE, EV_STATUS, EV_MEHOD, EV_STEP, EV_OOB
		case	DOT11_EVENT_WSC_MODE_IND:
		        sprintf(szEvent, (char*)"Receive Event %s", "WSC_MODE_IND");
			isWscdEvt = TRUE;
			break;
		case	DOT11_EVENT_WSC_STATUS_IND:
		        sprintf(szEvent, (char*)"Receive Event %s", "WSC_STATUS_IND");
			isWscdEvt = TRUE;
			break;
		case	DOT11_EVENT_WSC_METHOD_IND:
		        sprintf(szEvent, (char*)"Receive Event %s", "WSC_METHOD_IND");
			isWscdEvt = TRUE;
			break;
		case	DOT11_EVENT_WSC_STEP_IND:
		        sprintf(szEvent, (char*)"Receive Event %s", "WSC_STEP_IND");
			isWscdEvt = TRUE;
			break;
		case	DOT11_EVENT_WSC_OOB_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "WSC_OOB_IND");
			isWscdEvt = TRUE;
			break;
#endif  //ifdef CONFIG_IWPRIV_INTF
		case	DOT11_EVENT_WSC_PROBE_REQ_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "WSC_PROBE_REQ_IND");
			isWscdEvt = TRUE;
			break;

		case	DOT11_EVENT_WSC_ASSOC_REQ_IE_IND:
			sprintf(szEvent, (char*)"Receive Event %s", "WSC_ASSOC_REQ_IE_IND");
			isWscdEvt = TRUE;
			break;
#endif

		default:
			sprintf(szEvent, (char*)"Receive Invalid or Unhandled Event %d",
				event_SendBuf[5]);
			iSend = FALSE;
			break;
		}
	}

	if (isWscdEvt)
	{
		//if (link_wscd && isWscdEvt) {
			//for(i=0; i < link_wscd; i++){
			int i=0;
				//if(!strcmp(RTLDListenerWscd[i].wlanName,wlan_name)){
				//if(!strcmp(RTLDListenerWscd[i].wlanName,"wlan0")){
					if((iRet = write(RTLDListenerWscd[i].WriteFIFO, event_SendBuf, RWFIFOSIZE)) < 0)
						debug_message("Write FIFO fail");
						//iw_message(MESS_DBG_CONTROL, "Write FIFO: %s", strerror(errno));
					else
						debug_message("Write %d bytes\n", iRet);
						//iw_message(MESS_DBG_CONTROL, "Write %d bytes\n", iRet);
				//}
			//}
		//}
		retVal = (event_SendBuf[6] == TRUE)? TRUE : FALSE;	//If more event
	}

	/*
	fd = open(wps_config, O_RDWR | O_CREAT);

	if (fd < 0)	{
		printf("Cannot Open file %s!\n", wps_config);
		return -1;
	}

	write( fd, data, data_len); 
	
	close(fd);

	printf("wps event , the config is store to %s \n",wps_config);
	printf("get wps config from this file and set these mib to AP \n");
	inband_write(info->channel,0,0,data,data_len,1); //send good reply
	*/
}

static void do_wps_event(struct network_info_t *info, char *data,int data_len)
{	
	char *wps_config="/tmp/wps.conf";
	int fd,ret=0;

	fd = open(wps_config, O_RDWR | O_CREAT);

	if (fd < 0)	{
		printf("Cannot Open file %s!\n", wps_config);
		return -1;
	}

	write( fd, data, data_len); 
	
	close(fd);

	debug_message("wps event , the config is store to %s \n",wps_config);
	debug_message("get wps config from this file and set these mib to AP \n");
	inband_write(info->channel,0,0,data,data_len,1); //send good reply
 }

static void inband_wait_event(struct network_info_t *info)
{
	int ret,data_len;
	char cmd_type;		
	char *data_p;

	data_len = inband_rcv_data(info->channel,&cmd_type,&data_p,-1); //try to rcv inband data 	

	if(data_len < 0)
		goto rcv_end;
	//do cmd process
	if( info->func_type == 1 )
		do_wifi_event(info,data_p,data_len);  //do requested cmd , and reply a rsp_frame if need
	else
		do_wps_event(info,data_p,data_len);
	
rcv_end:	
	inband_free_buf(data_p, data_len);	
	return;
}


void listen_and_process_wps_event(struct network_info_t *info)
{
	printf("listening wps notification...\n");
	while(1){		
		inband_wait_event(info);   
	}
}

void listen_and_process_radius_event(struct network_info_t *info)
{
	int res=0, rx_len=0, tx_len=0,i;
	fd_set fsRead;
	struct timeval tvTimeOut;
	int 	iFD_SETSIZE = 0;
	unsigned char cmd_type, *rad_pkt, rad_buf[MAX_TCP_LEN];

	printf("listening radius L2 packet...\n");
	while(1) {
		FD_ZERO(&fsRead);
		for( i=0;i<INTERFACE_NUM*MBSSID;i++ ) {
			if( info->udp_info[i].udpsock ) {
				FD_SET( info->udp_info[i].udpsock, &fsRead);
				iFD_SETSIZE = (iFD_SETSIZE > info->udp_info[i].udpsock)?iFD_SETSIZE:info->udp_info[i].udpsock;
			}
		}

		FD_SET( info->inband_sock, &fsRead);
		iFD_SETSIZE = (iFD_SETSIZE > info->inband_sock)?iFD_SETSIZE:info->inband_sock;
		iFD_SETSIZE += 1;

		res = select( iFD_SETSIZE, &fsRead, NULL, NULL, NULL);
		if( res ){
			if(FD_ISSET(info->inband_sock, &fsRead)){
				rx_len = inband_rcv_data(info->channel,&cmd_type,&rad_pkt,0);
				if (rx_len <= 0) {
					debug_message("inband channel receive error!!");
				} else {
					debug_message("Receive :%d-byte data\n",rx_len);
					//hex_dump(rad_pkt, rx_len);
					debug_message("Received from client with index:%d\n",*(unsigned int *)rad_pkt);
					for( i=0;i<INTERFACE_NUM*MBSSID;i++ ) {
						if( info->udp_info[i].if_index == *(unsigned int *)rad_pkt ){
							rad_pkt += sizeof(int);
							rx_len -= sizeof(int);
							tx_len = send( info->udp_info[i].udpsock, rad_pkt, rx_len, 0); // flags = 0
							debug_message("%d bytes forwardded to RADIUS server No.%d as UDP\n",tx_len,info->udp_info[i].if_index);
							debug_message("\n\n");
						}
					}
					rx_len = 0;
				}
			}
			
			for( i=0;i<INTERFACE_NUM*MBSSID;i++ ) {
				if( info->udp_info[i].udpsock && FD_ISSET(info->udp_info[i].udpsock, &fsRead)){
					memcpy(rad_buf,&info->udp_info[i].if_index,sizeof(int));

					rx_len = recv( info->udp_info[i].udpsock, rad_buf+sizeof(int), MAX_TCP_LEN, 0);
					//hex_dump(info->ioh_obj.tx_data, rx_len);
					if( rx_len < 0 ){
						debug_message("udp receive error!!");
					}
					//hex_dump(rad_buf, rx_len);
					debug_message("%d bytes received from RADIUS server\n",rx_len);
					rx_len += sizeof(int);
					//hex_dump(rad_buf, rx_len);
					inband_indexed_write(info->channel,0,0x0,rad_buf,rx_len,0,info->udp_info[i].if_index);
				}
			}
		}
	}
}

#define iw_L2N(l,c)      (*((c) )=(unsigned char)(((l)>>24)&0xff),\
 			*((c)+1)=(unsigned char)(((l)>>16)&0xff),\
  			*((c)+2)=(unsigned char)(((l)>> 8)&0xff),\
   			*((c)+3)=(unsigned char)(((l)    )&0xff))


void init_sendBuf(char *ptr)
{
	pid_t pid;
	pid = getpid();
	iw_L2N((long)pid, ptr);
	*(ptr + sizeof(long)) = FIFO_TYPE_DLISTEN;
}

static char *get_token(char *data, char *token)
{
	char *ptr=data;
	int len=0, idx=0;

	while (*ptr && *ptr != '\n' ) {
		if (*ptr == '=') {
			if (len <= 1)
				return NULL;
			memcpy(token, data, len);

			/* delete ending space */
			for (idx=len-1; idx>=0; idx--) {
				if (token[idx] !=  ' ')
					break;
			}
			token[idx+1] = '\0';
			
			return ptr+1;
		}
		len++;
		ptr++;
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////
static int get_value(char *data, char *value)
{
	char *ptr=data;	
	int len=0, idx, i;

	while (*ptr && *ptr != '\n' && *ptr != '\r') {
		len++;
		ptr++;
	}

	/* delete leading space */
	idx = 0;
	while (len-idx > 0) {
		if (data[idx] != ' ') 
			break;	
		idx++;
	}
	len -= idx;

	/* delete bracing '"' */
	if (data[idx] == '"') {
		for (i=idx+len-1; i>idx; i--) {
			if (data[i] == '"') {
				idx++;
				len = i - idx;
			}
			break;
		}
	}

	if (len > 0) {
		memcpy(value, &data[idx], len);
		value[len] = '\0';
	}
	return len;
}


static int gain_wlan_index(unsigned char *name)
{
	unsigned char *ptr=NULL;
	unsigned int if_idx=-1,bss_idx=-1,i,j,ret=0;

	ptr = strstr(name,"wlan");
	if( ptr ){
		if_idx = *(ptr+4)-'0'+1;
	}

	ptr = strstr(name,"-va");
	if( ptr ){
		bss_idx = *(ptr+3)-'0'+1;
	} else {
		bss_idx = 1;
	}

	for( i=0;i<if_idx;i++ )
		for( j=0;j<bss_idx;j++ )
			ret++;

	return ret;
}

static int gain_radius_ip(unsigned char *config_name,struct network_info_t *info)
{
	FILE *in;
	unsigned char line[100], token[40], value[60], *ptr=NULL,intf[30];
	int i=0;

	in = fopen(config_name,"r");
	if( !in )
		return -1;

	while( fgets(line,sizeof(line),in) ){
		ptr = get_token(line,token);
		if( !ptr )
			continue;
		ptr = get_value(ptr,value);
		if( !ptr )
			continue;

		if( strstr(token,"rsIP") ){
			//while(info->udp_info[i].if_index && i<INTERFACE_NUM*MBSSID)
				//i++;

			ptr = strchr(token,'_');
			if( ptr ) {
				*ptr = '\0';
				strcpy(intf,token);

				for(i=0;i<INTERFACE_NUM*MBSSID;i++) {
					if( info->udp_info[i].if_index == 0 ){
						info->udp_info[i].if_index = gain_wlan_index(intf);
						info->udp_info[i].radsrvaddr.sin_addr.s_addr = inet_addr(value);
						debug_message("Add %s to idx:%d with index %d\n",intf,i,info->udp_info[i].if_index);
						break;
					}
				}
			}
		}
	}

	if(in)
		fclose(in);
}


static int parse_argument(struct network_info_t *network_info,int argc, char *argv[])
{
	int argNum=1;
	int pid;
	unsigned char line[100];
	char tmpbuf[100];
	int ip;

	memset(network_info,0,sizeof network_info);

	while (argNum < argc) { 
		/*
		if ( !strcmp(argv[argNum], "-a")) {
			network_info->radsrvaddr.sin_addr.s_addr = inet_addr(argv[argNum]);
		} else */if (!strcmp(argv[argNum], "-w")) {
			network_info->func_type = 1;
		} else if (!strcmp(argv[argNum], "-lw")) {
                        network_info->func_type = 3;
                } else if (!strcmp(argv[argNum], "-r")) {
			network_info->func_type = 2;
			gain_radius_ip(argv[argNum+1],network_info);
			//network_info->radsrvaddr.sin_addr.s_addr = inet_addr(argv[argNum+1]);
		} else if (!strcmp(argv[argNum], "wps")) {
			if( !strcmp(argv[argNum+1], "pin") ) {
				DOT11_WSC_PIN_IND wsc_ind;

                wsc_ind.EventId = DOT11_EVENT_WSC_PIN_IND;
                wsc_ind.IsMoreEvent = 0;
				debug_message("PIN=%s\n",argv[argNum+2]);
                strcpy((char *)wsc_ind.code,argv[argNum+2]);
				if((RTLDListenerWscd[0].WriteFIFO = open("/var/wscd-wlan0.fifo", O_WRONLY, 0)) < 0)
				{
					printf("fifo:%s not ready\n", "/var/wscd-wlan0.fifo");
					goto parse_err;
				}
                do_wifi_event(network_info,&wsc_ind,sizeof(wsc_ind));
				goto parse_err;
			} else if ( !strcmp(argv[argNum+1], "pbc") ) {
				//should be configurable interface name
				system("wscd -sig_pbc wlan0");
				goto parse_err;
			} else {
				show_help();
				goto parse_err;
			}
		} else {
			printf("invalid argument - %s\n", argv[argNum]);
			show_help();
			goto parse_err;
		}
		if (++argNum >= argc)
				break;
		argNum++;
	}
	return 0;
parse_err:
	return -1;
}


int main(int argc, char *argv[])
{
	int pid_fd;
	struct network_info_t network_info;
	unsigned char *rad_pkt, *attr_nas_ip, cmd_type, rad_buf[MAX_TCP_LEN];

	unsigned char rx_buffer[1600] = {0};
	int rx_len, tx_len;
	//func_type = 1: wps enable, 2:auth enable

	char tmpbuf[100];

	if(argc < 2) {
		show_help();
		return -1;
	}

	if( parse_argument(&network_info, argc, argv) < 0)
		return 0;

	if (daemon(0,1) == -1) {
		printf("fork iwevent daemon error!\n");
		return 0;
	}

	pid_fd = pidfile_acquire("/var/run/iweventd.pid");
	if (pid_fd < 0) {
		printf("Write pid to file failed\n");
		return -1;
	}

	pidfile_write_release(pid_fd);

	init_sendBuf(event_SendBuf);

    if( init_socket(&network_info, network_info.func_type) < 0 ){
		debug_message("Initial socket error\n");
		return -1;
    }

	DISPLAY_BANNER;

	switch(network_info.func_type){
		case 1:
			if((RTLDListenerWscd[0].WriteFIFO = open("/var/wscd-wlan0.fifo", O_WRONLY, 0)) < 0)
			{
				//iw_message(MESS_DBG_CONTROL, "open fifo %s error: %s", fifo_name, strerror(errno));
				//iw_message(MESS_DBG_CONTROL, "wait %s to create", fifo_name);
				printf("wait %s to create\n", "/var/wscd-wlan0.fifo");
				sleep(1);
				//exit(0);
			}
			listen_and_process_wps_event(&network_info);
			break;
		case 2:
			listen_and_process_radius_event(&network_info);
			break;

		case 3:
			listen_and_process_wps_event(&network_info);
			break;

		default:
			break;
	}

	return 0;
}
