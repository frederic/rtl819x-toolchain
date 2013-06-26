#ifndef __TALK_H__
#define __TALK_H__

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PENDINGQ	1

#define SVR_PORT	3595

#define BLOCK_NUM	16
//#define BLOCK_SIZE	512
//#define BLOCK_SIZE	160
#define BLOCK_SIZE	480
#define BUFFER_SIZE	(BLOCK_SIZE * BLOCK_NUM)

#define CACHED_BLOCK	1

#if 0
struct priv_data{
	int fd_pcm;
	int fd_socket;
	unsigned int hear_W;
	unsigned int hear_R;
	unsigned char* hear_buffer;
	unsigned char hear_flag;
	unsigned char cached_flag;
};
#endif

//#define TALK_DEBUG

//--------------------------------------------------------
//      PCM Controller IOCTL
//--------------------------------------------------------
#define PCM_IOCTL_SET_LINEAR    0xBE01
#define PCM_IOCTL_SET_ALAW      0xBE02
#define PCM_IOCTL_SET_ULAW      0xBE03
#define PCM_IOCTL_GET_SIZE      0xBE04  //Get page size.
#define PCM_IOCTL_SET_EX_CLK    0xBE05  //External clock source from codec
#define PCM_IOCTL_SET_IN_CLK    0xBE06  //Clock source from internal PLL (Output to codec)



#ifdef TALK_DEBUG
	#ifdef TALK_SERVER
        	#define PDBUG(fmt, args...) printf("svr - %s:" fmt, __FUNCTION__, ## args)
	#else
		#define PDBUG(fmt, args...) printf("cln - %s:" fmt, __FUNCTION__, ## args)
	#endif
#else
        #define PDBUG(fmt, args...)
#endif


#endif

