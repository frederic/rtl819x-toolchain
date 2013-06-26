#include "talk.h"

int g_fdpcm;
int g_peersock;
unsigned char* hear_buffer;

unsigned int hear_R;
unsigned int hear_W;
unsigned char hear_flag;
unsigned char cached_flag;
//struct sockaddr_in g_peeraddr;


void* talk(void* ptr)
{
	int readsize;
	int sndlen;
	int n;
	unsigned char rx_buffer[BLOCK_SIZE];

#ifdef USE_UDP
	struct sockaddr_in* peer_addr = (struct sockaddr_in *)ptr;
	socklen_t len_inet= sizeof(struct sockaddr);
#endif

	printf("Starting talk thread.\n");
	while(1)
	{
		sndlen = 0;
		n = 0;	

		if((readsize = read(g_fdpcm, rx_buffer, BLOCK_SIZE)) > 0)
		{
			while(readsize > 0)
			{
#ifdef USE_UDP
				if((n = sendto(g_peersock, rx_buffer + sndlen, readsize, 0, (struct sockaddr *)peer_addr, len_inet)) < 0)
#else
				if((n = send(g_peersock, rx_buffer + sndlen, readsize, 0)) < 0)
#endif
				{
					perror("Network send failed.\n");
					return;
				}
				PDBUG("readsize = %d, n = %d\n", readsize, n);
				readsize -= n;
				sndlen += n;
			}
		}

	}
	printf("Exit talk thread.\n");
}

int recvn(const int socket, unsigned char* buffer, size_t len)
{
	size_t rcvlen = 0;	//total size read
	size_t n;

#ifdef USE_UDP
	struct sockaddr_in adr_clnt;
	socklen_t len_inet= sizeof(struct sockaddr_in);
#endif

	while(len)
	{
		PDBUG("enter while loop.\n");
#ifdef USE_UDP
		if((n = recvfrom(socket, (void *)(buffer + rcvlen), len, 0, (struct sockaddr *)&adr_clnt, &len_inet)) <= 0)
#else
		if((n = recv(socket, (void *)(buffer + rcvlen), len, 0)) <= 0)
#endif
		{
			perror("An error occurred when receiving data.\n");
			PDBUG("n = %d\n", n);
			return n;
		}

		PDBUG("len = %d, n = %d\n", len, n);
		rcvlen += n;
		len -= n;
	}

	return rcvlen;
}

#ifdef TWO_THREAD
void* hear_net(void* ptr)
{
	while(hear_flag == 0)
	{
		if(((hear_W + 1)%BLOCK_NUM) != hear_R)
		{
			if(recvn(g_peersock, (void *)(hear_buffer + (hear_W * BLOCK_SIZE)), BLOCK_SIZE) <= 0)
			{
				perror("An error occurred when receiving data.\n");
				hear_flag = 1;
			}

			hear_W = (hear_W + 1)%BLOCK_NUM;
			//printf("%s - Move hear_W to %d\n", __FUNCTION__, hear_W);
		}
		else
		{
			printf("%s - Hear Buffer overflow.\n", __FUNCTION__);
		}
	}
}


void* hear_pcm(void* ptr)
{
	size_t totallen, writelen;
	size_t n;

	while(hear_flag == 0)
	{
		totallen = BLOCK_SIZE;
		writelen = 0;

		if(cached_flag)
		{
			if(hear_R != hear_W)
			{
				while(totallen > 0)
				{
					if((n = write(g_fdpcm, (void *)(hear_buffer + (hear_R * BLOCK_SIZE) + writelen), totallen)) > 0)
					{
						totallen -= n;
						writelen += n;
					}
					else if(n < 0)
					{
						hear_flag = 1;
						perror("Write PCM device failed.\n");
						break;
					}
					else continue;
				}
	
				hear_R = (hear_R + 1)%BLOCK_NUM;
				//printf("%s - Move hear_R to %d\n", __FUNCTION__, hear_R);
			}
			else
			{
				printf("%s - No buffer for PCM device.\n", __FUNCTION__);
			}
		}
		else
		{
			printf("%s - caching hear_W = %d\n", __FUNCTION__, hear_W);
			if(hear_W >= CACHED_BLOCK)
				cached_flag = 1;
		}
	}
}
#else
void* hear(void* ptr)
{
	int writesize;
	int rcvlen;
	int n;
	size_t net_n;

	printf("Starting hear thread.\n");

	while(1)
	{
		if(((hear_W + 1)%BLOCK_NUM) != hear_R)
		{
			PDBUG("hear_W = %d, hear_R = %d, hear_flag = %d\n", hear_W, hear_R, hear_flag);
			if( (net_n = recvn(g_peersock, (void *)(hear_buffer + (hear_W * BLOCK_SIZE)), BLOCK_SIZE)) < 0)
			{
				perror("An error occurred when receiving data.\n");
				hear_flag = 1;
			}
			else if(net_n == 0)
			{
				printf("remote side has closed the connection\n");
			}

			hear_W = (hear_W + 1)%BLOCK_NUM;
			PDBUG("Move hear_W to %d\n", hear_W);
		}
		else
		{
			printf("Hear Buffer overflow.\n");
		}


		if(cached_flag)
		{
			rcvlen = BLOCK_SIZE;
			writesize = 0;
	
			while(rcvlen > 0)
			{
	
				if( (n = write(g_fdpcm, (hear_buffer + writesize + (hear_R * BLOCK_SIZE)), rcvlen)) < 0)
				{
					perror("Write PCM device failed.\n");
					return;
				}
				rcvlen -= n;
				writesize += n;
			}

			hear_R = (hear_R + 1)%BLOCK_NUM;
			//printf("Move hear_R to %d\n", hear_R);
		}
		else
		{
			if(hear_W >= CACHED_BLOCK)
			{
				cached_flag = 1;
			}
		}
		
#if 0

		writesize = 0;
		n = 0;
		


		if((rcvlen = recvn(g_peersock, tx_buffer, BLOCK_SIZE, 0)) < 0)
		{
			perror("Network recv failed.\n");
			return;
		}
		
		if(rcvlen == 0)
		{
			printf("remote side has closed the connection\n");
			return;
		}

		while(rcvlen > 0)
		{

			if( (n = write(g_fdpcm, tx_buffer + writesize, rcvlen)) > 0)
			{
				rcvlen -= n;
				writesize += n;
			}
		}
#endif
	}
	printf("Exit hear thread.\n");
}

#endif	//TWO_THREAD

#ifdef TALK_SERVER
//--------------------------------------------------------------------------------------------
//	Server
//--------------------------------------------------------------------------------------------
int main(int argc, char** argv)
{

#ifdef USE_UDP
	if(argc != 2)
	{
		printf("USAGE: %s Peer-IP\n", argv[0]);
		return -1;
	}
#endif

	pthread_t talkthread, hearthread;
	pthread_t hear_net_td, hear_pcm_td;

	int svrsock, clnsock;
	struct sockaddr_in svrtalk;
	int sin_size = sizeof(struct sockaddr_in);


	g_fdpcm = open("/dev/pcmctrl0", O_RDWR);



	if(g_fdpcm < 0)
	{
		printf("Can't open PCM device.\n");
		return -1;
	}

        //set u-law
        if(ioctl(g_fdpcm, PCM_IOCTL_SET_ULAW))
        {
                perror("PCM_IOCTL_SET_ULAW failed!!\n");
                return -1;
        }


#ifdef USE_UDP
	if( (svrsock = socket(AF_INET, SOCK_DGRAM,  IPPROTO_UDP)) < 0 )
#else
	if( (svrsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 )
#endif
	{
		perror("Create socket failed.\n");
		exit(1);
	}

	memset(&svrtalk, 0, sizeof(struct sockaddr_in));
	svrtalk.sin_family = AF_INET;
	svrtalk.sin_addr.s_addr = htonl(INADDR_ANY);
	svrtalk.sin_port = htons(SVR_PORT);

	if(bind(svrsock, (struct sockaddr *)&svrtalk, sizeof(struct sockaddr)) < 0)
	{
		perror("Can't bind.\n");
		exit(1);
	}

#ifdef USE_UDP

	//g_peersock = svrsock;
	g_peersock = dup(svrsock);

	if(g_peersock == -1)
	{
		perror("Can't dup socket fd.\n");
		exit(1);
	}

	struct sockaddr_in peer_addr;

	memset(&peer_addr, 0, sizeof(struct sockaddr_in));
	peer_addr.sin_family = AF_INET;
	peer_addr.sin_port = htons(SVR_PORT);
	
	if(inet_aton(argv[1], &(peer_addr.sin_addr)) == 0)
	{
		perror("Address %s is invalid.\n");
		exit(1);
	}

#else

	if(listen(svrsock, PENDINGQ) < 0)
	{
		perror("listen failed.\n");
		exit(1);
	}

	printf("Starting to accept client connections.\n");

	if((g_peersock = accept(svrsock, (struct sockaddr *)&svrtalk, &sin_size)) < 0)
	{
		perror("accept failed.\n");
		exit(1);
	}

#endif

	unsigned char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);

	hear_buffer = buffer;


#ifdef USE_UDP
	if(pthread_create(&talkthread, NULL, &talk, (void *)&peer_addr))
#else
	if(pthread_create(&talkthread, NULL, &talk, NULL))
#endif
	{
		perror("Talk thread creation failed.\n");
	}


#ifdef TWO_THREAD
	if(pthread_create(&hear_net_td, NULL, &hear_net, NULL))
	{
		perror("Hear of network thread creation failed.\n");
	}

	if(pthread_create(&hear_pcm_td, NULL, &hear_pcm, NULL))
	{
		perror("Hear of PCM thread creation failed.\n");
	}
#else
	if(pthread_create(&hearthread, NULL, &hear, NULL))
	{
		printf("Hear thread creation failed.\n");
	}

#endif

	pthread_join(talkthread, NULL);

#ifdef TWO_THREAD
	pthread_join(hear_net_td, NULL);
	pthread_join(hear_pcm_td, NULL);
#else
	pthread_join(hearthread, NULL);
#endif



failed:
	close(svrsock);
	close(g_fdpcm);

	return 0;
}

#else
//--------------------------------------------------------------------------------------------
//	Client
//--------------------------------------------------------------------------------------------
int main(int argc, char** argv)
{

	if(argc != 2)
	{
		printf("USAGE: %s IP\n", argv[0]);
		return -1;
	}


	g_fdpcm = open("/dev/pcmctrl0", O_RDWR);

	if(g_fdpcm < 0)
	{
		printf("Can't open PCM device.\n");
		return -1;
	}

        //set u-law
        if(ioctl(g_fdpcm, PCM_IOCTL_SET_ULAW))
        {
                perror("PCM_IOCTL_SET_ULAW failed!!\n");
                return -1;
        }

#ifdef USE_UDP
	if ((g_peersock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
#else
	if ((g_peersock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
#endif
	{
		close(g_fdpcm);
		perror("socket failed.\n");
		exit(1);
	}


	struct sockaddr_in peer_addr;

	memset(&peer_addr, 0, sizeof(struct sockaddr_in));
	peer_addr.sin_family = AF_INET;
	peer_addr.sin_port = htons(SVR_PORT);
	
	if(inet_aton(argv[1], &(peer_addr.sin_addr)) == 0)
	{
		perror("Address %s is invalid.\n");
		exit(1);
	}

//	memset(&their_addr, 0, sizeof(struct sockaddr_in));
//	their_addr.sin_family = AF_INET;
//	their_addr.sin_port = htons(SVR_PORT);
//	inet_aton(argv[1], &(their_addr.sin_addr));

#ifndef USE_UDP
	if (connect(g_peersock, (struct sockaddr *)&peer_addr, sizeof(struct sockaddr)) == -1)
	{
		perror("connect failed.\n");
		close(g_peersock);
		close(g_fdpcm);
		exit(1);
	}
	
	printf("Connect to %s ...\n", argv[1]);
#endif
	

	unsigned char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);

	hear_buffer = buffer;
	
	//create talk thread - read data from PCM, send data to network.
	pthread_t talkthread;

#ifdef USE_UDP
	if(pthread_create(&talkthread, NULL, &talk, (void *)&peer_addr))
#else
	if(pthread_create(&talkthread, NULL, &talk, NULL))
#endif
	{
		perror("Talk thread creation failed.\n");
	}


#ifdef TWO_THREAD
	
	//one thread read data from network, another thread writes data to PCM.
	pthread_t hear_net_td, hear_pcm_td;

	if(pthread_create(&hear_net_td, NULL, &hear_net, NULL))
	{
		perror("Hear of network thread creation failed.\n");
	}

	if(pthread_create(&hear_pcm_td, NULL, &hear_pcm, NULL))
	{
		perror("Hear of PCM thread creation failed.\n");
	}
#else

	//Just one thread read data from network and writes to PCM.
	pthread_t hearthread;

	if(pthread_create(&hearthread, NULL, &hear, NULL))
	{
		perror("Hear thread creation failed.\n");
	}

#endif


	pthread_join(talkthread, NULL);

#ifdef TWO_THREAD
	pthread_join(hear_net_td, NULL);
	pthread_join(hear_pcm_td, NULL);
#else
	pthread_join(hearthread, NULL);
#endif


failed:
	close(g_peersock);
	close(g_fdpcm);

	return 0;


}

#endif

