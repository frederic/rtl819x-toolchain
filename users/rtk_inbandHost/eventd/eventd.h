#ifndef __RADIUS_L2TOL3_H__
#define __RADIUS_L2TOL3_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <signal.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <linux/wireless.h>
#include <linux/if_packet.h>

#include "../../inband_lib/ioh.h"
#include "../../inband_lib/inband_if.h"


#ifdef DEBUG
#define debug_message printf
#define DEBUG 1
#else
#define debug_message
#define DEBUG 0
#endif

#define VERSION_STR	"v1.3"
#define DISPLAY_BANNER \
	printf("\nInband Event Handle Daemon %s.\n\n", VERSION_STR)

#ifndef WIN32
#define __PACK__                        __attribute__ ((packed))
#else
#define __PACK__
#endif

#define INTERFACE   "br0"
//#define DEST_MAC    "00e04c819233"
#define DEST_MAC    "00e04c819635"
#define SLAVE_MAC ("001234567899")
#define ETH_P_RTK 0x8899
#define ETH_P_RTK_NOTIFY 0x9001 //reserved ethertyoe
#define BUFLEN 1518
#define MAXEAPLEN 1518
#define PROGRAM_NAME "Event Handle Daemon"
#define RADIUS_PORT 1812
#define RADIUS_IP ("192.168.1.188")
#define ETH_DHR_LEN 14
#define INTERFACE_NUM 2
#define MBSSID 4

struct udp_info_t {
	unsigned int if_index;
	int udpsock; // socket descriptors
	struct sockaddr_in radsrvaddr;
};

struct network_info_t {
    unsigned char func_type;
    int inband_sock; // socket descriptors
    int channel;
    struct ioh_class ioh_obj;
    struct sockaddr_in localaddr;
    struct udp_info_t udp_info[INTERFACE_NUM*MBSSID];
};

typedef struct _Dot1x_RTLDListener
{

	int	WriteFIFO;

	int	Iffd;

	char	wlanName[16];

	//char    SendBuf[1600];



}Dot1x_RTLDListener;


#endif

