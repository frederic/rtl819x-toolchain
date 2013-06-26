#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "cwmpevt.h"
#include "mib_tr104.h"

#define CWMP_CLINET_PATH "/var/cwmpclient.chanl"
#define SOLAR_CHANNEL_PATH "/var/solar.chanl"

#define SOLAR_SYNC_INTERVAL		60	// (1 min.) polling solar's all log 

static int ipcSocket = 0;
cwmpEvtMsg pEvtMsg;

static void cwmpSendRequestToSolar(void);

/*init the solar listener*/
static int cwmp_solarInit(void){
	struct sockaddr_un ipcAddr;

	printf("+++++go to cwmp_solarInit+++++\n");
	ipcSocket = socket(PF_LOCAL, SOCK_DGRAM, 0);
	if(0 > ipcSocket){
		perror("Error:open IPC socket fail");
		return 0;
	}
	unlink(CWMP_CLINET_PATH);
	bzero(&ipcAddr, sizeof(ipcAddr));
	ipcAddr.sun_family = PF_LOCAL;
	strncpy(ipcAddr.sun_path, CWMP_CLINET_PATH, sizeof(CWMP_CLINET_PATH));
	if(bind(ipcSocket, (struct sockaddr*)&ipcAddr, sizeof(ipcAddr)) == -1){
		close(ipcSocket);
		perror("Error:bind IPC socket fail");
		return 0;
	}

	return 1;
}

/*create the solar listener thread*/
static void *cwmp_solarListener(void *data)
{
	cwmpEvtMsg evtMsg;
	
	fd_set fdset;
	int h_max;
	struct timeval tv;
	int err;
	
	if( sizeof( evtMsg ) != cwmpEvtMsgSizeof() )
		printf("sizeof( evtMsg ) != cwmpEvtMsgSizeof()?? %d != %d\n", sizeof( evtMsg ), cwmpEvtMsgSizeof() );

	if(!cwmp_solarInit())
		return NULL;
		
	printf("+++++go to cwmp_solarListener+++++\n");
	
	cwmpSendRequestToSolar();	// query current status 
	
	while(1){
		
		FD_ZERO( &fdset );
		
		h_max = ipcSocket;
		FD_SET( ipcSocket, &fdset );
		
		tv.tv_sec = SOLAR_SYNC_INTERVAL;	// 1 min. 
		tv.tv_usec = 0;
		
		err = select(h_max + 1, &fdset, NULL, NULL, &tv);
		
		if( err == 0 ) {	// timeout 
			cwmpSendRequestToSolar();
			continue;
		}
		
		if (recvfrom(ipcSocket, (void*)&evtMsg, cwmpEvtMsgSizeof(), MSG_DONTWAIT, NULL, NULL) > 1){
			if(EVT_VOICEPROFILE_LINE_SET_STATUS == evtMsg.event){
				memcpy((void*)&pEvtMsg, (void*)&evtMsg,  cwmpEvtMsgSizeof());
			}
			printf("+++++recv string+++++\n");
		}
	}
	//printf("+++++go to cwmp_solarListener+++++\n");
	
	return NULL;
}


/*open the connection from solar to cwmpclient*/
static void cwmp_solarOpen( void )
{
	pthread_t cwmp_solar_pid;
	
	if( pthread_create(&cwmp_solar_pid, NULL, cwmp_solarListener, NULL) != 0)
		fprintf(stderr,"Error:initial solar listener fail\n");
}

/*close the connection from solar to cwmpclient*/
static void cwmp_solarClose(void){
	close(ipcSocket);
}

/* send the request to solar */
static void cwmpSendRequestToSolar(void){
	cwmpEvtMsg *sendEvtMsg=NULL;
	int sendSock=0;
	struct sockaddr_un addr;

	printf("+++++cwmpSendRequestToSolar+++++\n");
	if((sendEvtMsg = cwmpEvtMsgNew()) != NULL){
		sendEvtMsg->event = EVT_VOICEPROFILE_LINE_GET_STATUS;
		sendSock=socket(PF_LOCAL, SOCK_DGRAM, 0);
		bzero(&addr, sizeof(addr));
		addr.sun_family = PF_LOCAL;
		strncpy(addr.sun_path, SOLAR_CHANNEL_PATH, sizeof(SOLAR_CHANNEL_PATH));
		sendto(sendSock, (void*)sendEvtMsg, cwmpEvtMsgSizeof(), 0, (struct sockaddr*)&addr, sizeof(addr));
		close(sendSock);
		free(sendEvtMsg);
	}
}

/* main() init/exit functions */
void tr104_main_init( void )
{
	/* voip flash client initial */
	if(voip_flash_client_init(&g_pVoIPShare, VOIP_FLASH_WRITE_CLIENT_TR104) == -1){
		fprintf( stderr, "<%s>initial flash fail!\n",__FILE__ );
		return;
	}
	
	/* open IPC to communicate solar */
	cwmp_solarOpen();
}

void tr104_main_exit( void )
{
	/* close IPC to communicate solar */
	cwmp_solarClose();
	
	/* close the flash voip client */
	voip_flash_client_close();
}

