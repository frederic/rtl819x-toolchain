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

//--------------------------------------------------------
//	PCM Controller IOCTL
//--------------------------------------------------------
#define PCM_IOCTL_SET_LINEAR	0xBE01
#define PCM_IOCTL_SET_ALAW	0xBE02
#define PCM_IOCTL_SET_ULAW	0xBE03
#define PCM_IOCTL_GET_SIZE	0xBE04	//Get page size.
#define PCM_IOCTL_SET_EX_CLK	0xBE05	//External clock source from codec
#define PCM_IOCTL_SET_IN_CLK	0xBE06	//Clock source from internal PLL (Output to codec)


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

