#include <stdio.h> 
#include <stdlib.h> 
#include <fcntl.h> 
#include <string.h>
#include <sys/types.h> 
#include <sys/uio.h> 
#include <unistd.h> 
#include <signal.h>
#include "sysinit.h"

unsigned char INBAND_SLAVE_MAC[6] ={0x00,0x12,0x34,0x56,0x78,0x99};


void bring_up_lan(void)
{
	int index=0;
	unsigned char addr[6];
	unsigned char tmpbuf[100]={0}, mac[6]={0};

	if(strcmp(IF_MII,IF_ETH) == 0) //if all port belong to one IF
	{
		ADD_BR_INTERFACE(IF_BR, IF_ETH, INBAND_SLAVE_MAC,1,1); 
	}
	else
	{		
		ADD_BR_INTERFACE(IF_BR, IF_MII, INBAND_SLAVE_MAC,1,1);    

		memcpy(addr, ALL_ZERO_MAC_ADDR, 6);
		ADD_BR_INTERFACE(IF_BR, IF_ETH, addr,1,1); 
	}
	
	   		
}
void bridge_wlan(void)
{
	int index=0;
	unsigned char addr[6];
	unsigned char tmpbuf[100]={0}, mac[6]={0};
	unsigned char tmpbuf1[100]={0};
	int i;

	//memcpy(addr, ALL_ZERO_MAC_ADDR, 6);
	ADD_BR_INTERFACE(IF_BR, IF_WLAN, addr,1,0);    
#ifdef RTK_MBSSID_SUPPORT
	for(i=0;i<RTK_MAX_MBSSID;i++)
	{
		sprintf(tmpbuf1,"wlan0-va%d",i);
		ADD_BR_INTERFACE(IF_BR, tmpbuf1, addr,1,0);    
	}
#endif
	
}

void init_bridge()
{	
	char tmpbuf[100];
	
#ifdef CONFIG_RTK_VLAN_SUPPORT
	sprintf(tmpbuf,"echo \"4\" > %s", "/proc/wan_port"); //mark_issue ,always set inband port to port4 ?
       system(tmpbuf);
	//mark_vlan
	sprintf(tmpbuf,"echo \"1\" > %s", "/proc/disable_l2_table");
       system(tmpbuf);
#endif
       
	sprintf(tmpbuf, "%s addbr %s", BR_UTL, IF_BR); 
	system(tmpbuf);        

	DISABLE_STP(IF_BR);

	//init_host_info(); //vlan....etc	
	bring_up_lan();	

	//add wlan to bridge
	bridge_wlan();

	sprintf(tmpbuf, "%s %s hw ether %s up", IFCONFG, IF_BR,INBAND_MAC); 
	system(tmpbuf);

}

int main(int argc, char *argv[])
{ 	
	int pid_fd, pid, scr;
	char tmpbuf[100];
	int chan;

	//if (parse_argument(argc, argv) != 0) 
	//	return 0;

	// become daemon
	/*if (daemon(0,1) == -1) {
		printf("fork daemon error!\n");
		return 0;
	}*/
      init_bridge();
      sleep(3);	//wait bridge forwarding		  
}

