#include <stdio.h> 
#include <fcntl.h> 
#include <sys/types.h> 
#include <sys/uio.h> 
#include <unistd.h> 
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include "../../../linux-2.6.30/drivers/char/rtl_nfbi/rtl_nfbi.h"
#include "nfbi_api.h"

#define DEV_NAME  ("/dev/"DRIVER_NAME)

extern int nfbi_fd;

static int got_sigusr1 = 0;

static void event_handler(int sig_no)
{
    int ret;
    struct evt_msg evt;
    
    got_sigusr1 = 1;

    ret = nfbi_get_event_msg(&evt);
    while ((ret==0) && (evt.id==1)) {
        printf("evt.id=%d evt.value=%x evt.status=%x ret=%d\n", evt.id, evt.value, evt.status, ret);
        if (evt.status & IP_CHECKSUM_DONE) {
            if (evt.value & BM_CHECKSUM_DONE)
                printf("Checksum Done: 1\n");
            else
                printf("Checksum Done: 0\n");
        }
        if (evt.status & IP_CHECKSUM_OK) {
            if (evt.value & BM_CHECKSUM_OK)
                printf("Checksum OK: 1\n");
            else
                printf("Checksum OK: 0\n");
        }
        if (evt.status & IP_WLANLINK) {
            if (evt.value & BM_WLANLINK)
                printf("WLAN link: up\n");
            else
                printf("WLAN link: down\n");
        }
        if (evt.status & IP_ETHLINK) {
            if (evt.value & BM_ETHLINK)
                printf("Eth link: up\n");
            else
                printf("Eth link: down\n");
        }
        if (evt.status & IP_ETHPHY_STATUS_CHANGE) {
            printf("EthPhy status: change\n");
        }
        if (evt.status & IP_ALLSOFTWARE_READY) {
            if (evt.value & BM_ALLSOFTWARE_READY)
                printf("All Software: ready\n");
            else
                printf("All Software: not ready\n");
        }
        if (evt.status & IP_USBInsertStatus) {
            printf("A USB device was inserted.\n");
        }
        if (evt.status & IP_USBRemoveStatus) {
            printf("A USB device was removed.\n");
        }
        if (evt.status & IP_BOOTCODE_READY) {
            printf("Bootcode: ready\n");
        }
        if (evt.status & IP_NEED_BOOTCODE) {
            printf("Need Bootcode\n");
        }
        ret = nfbi_get_event_msg(&evt);
    }
}

int main(int argc, char *argv[])
{ 	
	//int fdflags;
	//unsigned int arg;
	//int i, ret;
    //struct evt_msg evt;
   
	signal(SIGUSR1, event_handler);

    //open device file
    nfbi_fd = open(DEV_NAME, O_RDWR);
	if (-1 == nfbi_fd) {
		printf("open driver failed!\n");
		return -1;
	}

    nfbi_set_hcd_pid(getpid());
    
	while (1) {
		//pause();
		sleep(1);
        if (got_sigusr1) {
            
        }
        got_sigusr1 = 0;
	}

    nfbi_set_hcd_pid(-1); //unregister
    eqreset();
    close(nfbi_fd);
    
    return 0;
} 
