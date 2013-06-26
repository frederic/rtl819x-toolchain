#include "talk.h"

int g_fdpcm;
int g_peersock;

unsigned char hear_flag;
unsigned char cached_flag;
unsigned int g_page_size;

void* talk(void* ptr)
{
	int readsize;
	int sndlen;
	int n;
	unsigned char rx_buffer[g_page_size];
	

	printf("Starting talk thread.\n");
	while(1)
	{
		sndlen = 0;
		n = 0;	

		if((readsize = read(g_fdpcm, rx_buffer, g_page_size)) > 0)
		{
			
			while(readsize > 0)
			{
				if((n = send(g_peersock, rx_buffer + sndlen, readsize, 0)) < 0)
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

	while(len)
	{
		PDBUG("enter while loop.\n");

		if((n = recv(socket, (void *)(buffer + rcvlen), len, 0)) <= 0)
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

void* hear(void* ptr)
{
	int writesize;
	int rcvlen;
	int n;
	size_t net_n;
	

	unsigned char buffer[g_page_size];
	printf("Starting hear thread.\n");

	while(1)
	{
		if(recvn(g_peersock, buffer, g_page_size) < 0)
		{
			perror("Network error!\n");
			exit(1);
		}

		if(write(g_fdpcm, buffer, g_page_size) < 0)
		{
			perror("write PCM failed\n");
			exit(1);
		}
	}
	printf("Exit hear thread.\n");
}

int pcm_init(void)
{
	g_fdpcm = open("/dev/pcmctrl0", O_RDWR);

	if(g_fdpcm < 0)
	{
		printf("Can't open PCM device.\n");
		return -1;
	}

	//Get PCM Controller Page Size
        unsigned int pagesize = 0;
        if(ioctl(g_fdpcm, PCM_IOCTL_GET_SIZE, &g_page_size) == -1)
        {
                perror("PCM_IOCTL_GET_SIZE faile!!\n");
		return -1;
	}
#if 1 // for SLIC, thlin  
        //set u-law
        if(ioctl(g_fdpcm, PCM_IOCTL_SET_ULAW))
        {
                perror("PCM_IOCTL_SET_ULAW failed!!\n");
                return -1;
        }
#endif
	return 0;
}


#ifdef TALK_SERVER
//--------------------------------------------------------------------------------------------
//	Server
//--------------------------------------------------------------------------------------------
int main(int argc, char** argv)
{
	pthread_t talkthread, hearthread;
	pthread_t hear_net_td, hear_pcm_td;

	int svrsock, clnsock;
	struct sockaddr_in svrtalk;
	int sin_size = sizeof(struct sockaddr_in);


	if(pcm_init() != 0)
	{
		perror("Initial PCM failed.\n");
		exit(1);
	}


	if( (svrsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 )
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


	if(pthread_create(&talkthread, NULL, &talk, NULL))
	{
		perror("Talk thread creation failed.\n");
	}


	if(pthread_create(&hearthread, NULL, &hear, NULL))
	{
		printf("Hear thread creation failed.\n");
	}


	pthread_join(talkthread, NULL);
	pthread_join(hearthread, NULL);

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

	if(pcm_init() != 0)
	{
		perror("Initial PCM failed.\n");
		exit(1);
	}

	if ((g_peersock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
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

	if (connect(g_peersock, (struct sockaddr *)&peer_addr, sizeof(struct sockaddr)) == -1)
	{
		perror("connect failed.\n");
		close(g_peersock);
		close(g_fdpcm);
		exit(1);
	}
	
	printf("Connect to %s ...\n", argv[1]);
	
	//create talk thread - read data from PCM, send data to network.
	pthread_t talkthread;
#if 1 // disable talk 
	if(pthread_create(&talkthread, NULL, &talk, NULL))
	{
		perror("Talk thread creation failed.\n");
	}
#endif

	//Just one thread read data from network and writes to PCM.
	pthread_t hearthread;
	if(pthread_create(&hearthread, NULL, &hear, NULL))
	{
		perror("Hear thread creation failed.\n");
	}


	pthread_join(talkthread, NULL);
	pthread_join(hearthread, NULL);


failed:
	close(g_peersock);
	close(g_fdpcm);

	return 0;


}

#endif

