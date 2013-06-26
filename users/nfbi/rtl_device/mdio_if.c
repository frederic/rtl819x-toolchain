#include <stdio.h> 
#include <stdlib.h> 
#include <fcntl.h> 
#include <string.h>
#include <sys/types.h> 

#include <sys/ioctl.h>
#include "hcd.h"
#include "mib.h"
#include "../../../linux-2.6.30/drivers/char/rtl_mdio/rtl_mdio.h"

static int fd;
#define DEV_NAME						("/dev/rtl_mdio")

void (*cmd_process_fn)(unsigned char *, int) = NULL; 


void mdio_open()
{
     fd = open(DEV_NAME, O_RDWR); 
	if (!fd) {
		perror("mdio_open fail:");
		exit(1);
	}
}

void mdio_write_data(unsigned char *data, int len) 
{
	if (len > MDIO_BUFSIZE) {
		printf("Write length > MDIO_BUFSIZE!\n");
		return;
	}
	write(fd, data, len);
}

int do_mdio_ioctl(int id, void *data)
{
		int ret = RET_OK;
		
		switch (id) {
			
			case id_cmd_timeout	:
				if (ioctl(fd, MDIO_IOCTL_SET_CMD_TIMEOUT, data)) {
					DEBUG_ERR("Set ioctl MDIO_IOCTL_SET_CMD_TIMEOUT error!\n");
					ret = -RET_IOCTL_ERR;
				}
				break;				
			case id_mii_pause_enable:
				if (ioctl(fd, MDIO_IOCTL_SET_MII_PAUSE, data)) {
					DEBUG_ERR("Set ioctl MDIO_IOCTL_SET_MII_PAUSE error!\n");
					ret = -RET_IOCTL_ERR;
				}
				break;
			case id_eth_pause_enable:
				if (ioctl(fd, MDIO_IOCTL_SET_ETH_PAUSE, data)) {
					DEBUG_ERR("Set ioctl MDIO_IOCTL_SET_ETH_PAUSE error!\n");
					ret = -RET_IOCTL_ERR;
				}
				break;
			case id_cpu_suspend_enable:
				if (ioctl(fd, MDIO_IOCTL_SET_SUSPEND, data)) {
					DEBUG_ERR("Set ioctl MDIO_IOCTL_SET_SUSPEND error!\n");
					ret = -RET_IOCTL_ERR;
				}
				break;				
			case id_phy_reg_poll_time:
				if (ioctl(fd, MDIO_IOCTL_SET_PHY_POLL_TIME, data)) {
					DEBUG_ERR("Set ioctl MDIO_IOCTL_SET_PHY_POLL_TIME error!\n");
					ret = -RET_IOCTL_ERR;
				}
				break;							
		
			case id_set_host_pid:
				if (ioctl(fd, MDIO_IOCTL_SET_HOST_PID, data)) {
					DEBUG_ERR("Set ioctl MDIO_IOCTL_SET_HOST_PID error!\n");
					ret = -RET_IOCTL_ERR;
				}
				break;								
			default:
				DEBUG_ERR("Invalid cmd id [0x%x] !\n", id);
				ret = -RET_NOT_SUPPORT_NOW;				
		}
		return ret;
}

void mdio_wait_event(void *process_fn)
{  
     struct evt_msg evt;

     cmd_process_fn = (void *)process_fn;	 

     while (read(fd, &evt, sizeof(evt)) > 0) {
         if (evt.id == IND_CMD_EV)
             (*cmd_process_fn)(evt.buf, evt.len);        
         else {
             printf("Invalid evt id [%d]!\n", evt.id);
         }
     }
}
    

