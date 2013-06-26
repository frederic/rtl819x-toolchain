#include "eventd.h"

#define MAX_TCP_LEN 65535

static void show_help(void)
{
	printf("  Usage: %s [argument]...\n", PROGRAM_NAME);
	printf("    Where arguments is optional as:\n");
	printf("\t-c config_filename, config filename, default is\n");
	printf("\n");	
}

static int parse_argument(struct network_info_t *network_info,unsigned char *type, int argc, char *argv[])
{
	int argNum=1;
	int pid;
	unsigned char line[100];
	char tmpbuf[100];
	int ip;

	while (argNum < argc) { 
		/*
		if ( !strcmp(argv[argNum], "-a")) {
			network_info->radsrvaddr.sin_addr.s_addr = inet_addr(argv[argNum]);
		} else*/ if (!strcmp(argv[argNum], "-w")) {
			*type = 1;
		} else if (!strcmp(argv[argNum], "-r")) {
			*type = 2;
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

static int init_socket(struct network_info_t *network_info, int type)
{
	int flags;
	struct ioh_class *ioh_ptr;

	switch(type){
		case 1:
			network_info->channel = inband_open(INTERFACE,SLAVE_MAC,ETH_P_RTK_NOTIFY,0);
			break;
		/*
		case 2:
			network_info->channel = inband_open(INTERFACE,DEST_MAC,ETH_P_RTK_NOTIFY,0);
			//network_info->channel = inband_open(INTERFACE,DEST_MAC,ETH_P_RTK,0);
			
			//ioh_ptr = get_chan_obj(network_info->channel);
			network_info->inband_sock = get_inband_socket(network_info->channel);
			network_info->radsrvaddr.sin_family = PF_INET;
			network_info->radsrvaddr.sin_port = htons( RADIUS_PORT );
			network_info->radsrvaddr.sin_addr.s_addr = inet_addr( RADIUS_IP );
			memset( &network_info->localaddr, 0, sizeof(network_info->localaddr));
			network_info->localaddr.sin_family = AF_INET;
			network_info->localaddr.sin_addr.s_addr = htonl(INADDR_ANY);
			network_info->localaddr.sin_port = htons(0);
			
			network_info->udpsock = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if ( network_info->udpsock == -1 ){
				debug_message("lib1x_nal_initialize: Could not open Radius Authentication UDP socket !");
				goto err_out;
			}
			
			if ( bind( network_info->udpsock, (struct sockaddr *) &network_info->localaddr, sizeof(network_info->localaddr)) != 0){
				debug_message("Could not BIND Authentication server UDP socket.");
				goto err_out;
			}

			// wait till succeed
			while (1) {
				if (connect( network_info->udpsock, (struct sockaddr *)&network_info->radsrvaddr, sizeof(network_info->radsrvaddr) ) != 0 )
				{
					debug_message("lib1x_nal_connect: Could not connect to Authentication Server . ");
					sleep(1);
				} else {
					debug_message("Radius connected !!!\n");
					break;
				}
			}

			flags = fcntl(network_info->udpsock, F_GETFL );
			if ( fcntl( network_info->udpsock, F_SETFL, flags | O_NONBLOCK) != 0 ) {
				debug_message("lib1x_nal_connect : FCNTL failed on UDP socket" );
				goto err_out;
			}
			break;
		*/
		default:
			debug_message("Unknown usage type!!\n");
			goto err_out;
	}

	return 0;

err_out:
	return -1;
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
	do_wps_event(info,data_p,data_len);  //do requested cmd , and reply a rsp_frame if need
	
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

/*
void listen_and_process_radius_event(struct network_info_t *info)
{
	int res=0, rx_len=0, tx_len=0;
	fd_set fsRead;
	struct timeval tvTimeOut;
	int 	iFD_SETSIZE = 0;
	unsigned char cmd_type, *rad_pkt, rad_buf[MAX_TCP_LEN];

	printf("listening radius L2 packet...\n");
	while(1) {
		FD_ZERO(&fsRead);
		FD_SET( info->udpsock, &fsRead);
		iFD_SETSIZE = (iFD_SETSIZE > info->udpsock)?iFD_SETSIZE:info->udpsock;

		FD_SET( info->inband_sock, &fsRead);
		iFD_SETSIZE = (iFD_SETSIZE > info->inband_sock)?iFD_SETSIZE:info->inband_sock;
		iFD_SETSIZE += 1;

		res = select( iFD_SETSIZE, &fsRead, NULL, NULL, NULL);
		if( res ){
			if(FD_ISSET(info->udpsock, &fsRead)){
				rx_len = recv( info->udpsock, rad_buf, MAX_TCP_LEN, 0);
				//hex_dump(info->ioh_obj.tx_data, rx_len);
				if( rx_len < 0 ){
					debug_message("udp receive error!!");
				}
				//hex_dump(rad_buf, rx_len);
				printf("%d bytes received from RADIUS server\n",rx_len);
				inband_write(info->channel,0,0x0,rad_buf,rx_len,0);
			}

			if(FD_ISSET(info->inband_sock, &fsRead)){
				rx_len = inband_rcv_data(info->channel,&cmd_type,&rad_pkt,0);
				if (rx_len <= 0) {
					debug_message("inband channel receive error!!");
				} else {
					printf("Receive :%d-byte data\n",rx_len);
					//hex_dump(rad_pkt, rx_len);
					tx_len = send( info->udpsock, rad_pkt, rx_len, 0); // flags = 0
					printf("%d bytes forwardded to RADIUS server as UDP\n",tx_len);
					rx_len = 0;
					printf("\n\n");
				}
			}
		}
	}
}
*/

int main(int argc, char *argv[])
{
	struct network_info_t network_info;
	unsigned char *rad_pkt, *attr_nas_ip, cmd_type, rad_buf[MAX_TCP_LEN];

	unsigned char rx_buffer[1600] = {0}, func_type;
	int rx_len, tx_len;
	//func_type = 1: wps enable, 2:auth enable

	char tmpbuf[100];

	/*
	sprintf(tmpbuf, "%s addbr %s", "brctl", "br0"); 
	system(tmpbuf);
	sprintf(tmpbuf,"%s %s %s", "ifconfig", "br0", "192.168.1.254");
	system(tmpbuf);
	sprintf(tmpbuf,"%s %s %s %s", "brctl", "addif", "br0", "eth0");
	system(tmpbuf);
	sprintf(tmpbuf, "%s %s hw ether %s up", "ifconfig", "eth0","001234567899");
	system(tmpbuf);
	*/

	if(argc < 2) {
		show_help();
		return -1;
	}

	parse_argument(&network_info, &func_type, argc, argv);

    if( init_socket(&network_info, func_type) < 0 ){
		debug_message("Initial socket error\n");
		return -1;
    }

	switch(func_type){
		case 1:
			listen_and_process_wps_event(&network_info);
			break;
		/*
		case 2:
			listen_and_process_radius_event(&network_info);
			break;
		*/
		default:
			break;
	}

	return 0;
}
