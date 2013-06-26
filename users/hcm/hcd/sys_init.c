/*
  *   Module to access wlan driver
  *
  *	Copyright (C)2008, Realtek Semiconductor Corp. All rights reserved.
  *
  *	$Id: sys_init.c,v 1.8 2011/01/25 06:27:06 marklee Exp $
  */


/*================================================================*/
/* System Include Files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <linux/if_ether.h> 
#include <linux/wireless.h>
#include <fcntl.h> 
#include <sys/types.h> 
#include <sys/uio.h> 
#include <unistd.h> 
#include <signal.h>
#include <sys/wait.h> 

/*================================================================*/
/* Local Include Files */

#include "hcd.h"
#include "mib.h"

#ifdef _KERNEL_LINUX_26
#include "../../../linux-2.6.30/drivers/net/wireless/rtl8192cd/ieee802_mib.h"
#else
#include "../../../linux-2.4.18/drivers/net/rtl8192cd/ieee802_mib.h" 
#endif


/*================================================================*/
/* Constant Definitions */
extern struct config_mib_all mib_all;

void init_host_info()
{
	char tmpbuf[100];	
	unsigned char mac[6];
	char *mac_ptr = &mib_all.sys_config.host_mac;
	int i;

	if( !mib_all.wlan[0].legacy_flash_settings.wscDisable ) //if wps enable 
	{
		sprintf(tmpbuf,"echo \"1 %s\" > %s",mib_all.sys_config.host_ip,"/proc/br_hostip");
      	       system(tmpbuf);

		for(i=0;i<6;i++)
			mac[i]=convert_atob(mac_ptr+(2*i),16);

		sprintf(tmpbuf,"echo \"%d:%d:%d:%d:%d:%d\" > %s",mac[0],mac[1],mac[2],
													mac[3],mac[4],mac[5],"/proc/br_hostmac");
      	       system(tmpbuf);

		sprintf(tmpbuf,"%s %s %s", IFCONFG, IF_BR, get_host_ip());
        system(tmpbuf);			   

	}
	else
	{
		sprintf(tmpbuf,"echo \"0 %s\" > %s",mib_all.sys_config.host_ip,"/proc/br_hostip");
      	       system(tmpbuf);

		for(i=0;i<6;i++)
			mac[i]=convert_atob(mac_ptr+(2*i),16);

		sprintf(tmpbuf,"echo \"%d:%d:%d:%d:%d:%d\" > %s",mac[0],mac[1],mac[2],
													mac[3],mac[4],mac[5],"/proc/br_hostmac");
      	       system(tmpbuf);			   

		sprintf(tmpbuf,"%s %s %s", IFCONFG, IF_BR, "0.0.0.0"); //reset br0 ip
        system(tmpbuf);	   
	}	
	
}
void parse_vlan_config(void *data1, int *val)
{
	char *ptr, value[5]={'\0'}, tmpbuf[100]={'\0'};
	//int val[VLAN_CONFIG_NUM];
	int index;

	if( strlen(data1) > 0 ) {
		for(index=0;index<VLAN_CONFIG_NUM;index++)
		{
			memset(value,'\0',5);
			if(index==0){
				ptr = strstr(data1,",");
				memcpy(value,data1,ptr-(char *)data1);
				val[index] = atoi(value);
			} else if(index==VLAN_CONFIG_NUM-1) {
				val[index] = atoi(data1+1);
			} else {
				ptr = strstr(data1+1,",");
				memcpy(value,data1+1,ptr-(char *)data1-1);
				val[index] = atoi(value);
			}
			data1 = ptr;
		}
	}
}

#if 1
int init_vlan(void)
{
	int index =0, vlan_config[VLAN_CONFIG_NUM] = {0};
	unsigned char cmd_buf[100];
	/*if( mib_all.sys_config.vlan_enable )
		system("echo 1 > /proc/rtk_vlan_support");
	else
		system("echo 0 > /proc/rtk_vlan_support");*/
	if( mib_all.sys_config.vlan_enable ==0 )	//do noting	
		return 0; 

	for(index=0;index<MAX_ETH_INTF;index++)
	{	
		parse_vlan_config(mib_all.eth[index].vlan,vlan_config);
		sprintf(cmd_buf,"echo \"%d %d %d %d %d %d %d\" > /proc/%s/mib_vlan",mib_all.sys_config.vlan_enable,\
			vlan_config[0],vlan_config[1],vlan_config[2],vlan_config[3],vlan_config[4],vlan_config[5],\
			mib_all.eth[index].name);
		system(cmd_buf);
	}
	parse_vlan_config(mib_all.wlan[0].vlan,vlan_config);
	sprintf(cmd_buf,"echo \"%d %d %d %d %d %d %d\" > /proc/%s/mib_vlan",mib_all.sys_config.vlan_enable,\
		vlan_config[0],vlan_config[1],vlan_config[2],vlan_config[3],vlan_config[4],vlan_config[5],mib_all.wlan[0].name);
	system(cmd_buf);

	for(index=0;index<get_vap_num();index++)
	{
		parse_vlan_config(mib_all.wlan[index+1].vlan,vlan_config);
		sprintf(cmd_buf,"echo \"%d %d %d %d %d %d %d\" > /proc/%s/mib_vlan",mib_all.sys_config.vlan_enable,\
			vlan_config[0],vlan_config[1],vlan_config[2],vlan_config[3],vlan_config[4],vlan_config[5],\
			mib_all.wlan[index+1].name);
		system(cmd_buf);
	}

	return 0;
}
#endif

void bring_down_lan(void)
{
	int index=0;
	unsigned char tmpbuf[100]={0};

	// mark_mac , should we keep inband port alive??
#ifdef RTK_MII_PORT_NETIF   	
	DEL_BR_INTERFACE(IF_BR, IF_MII,0,1);	
#endif

	DEL_BR_INTERFACE(IF_BR, IF_ETH, 0, 1); 
#ifdef CONFIG_RTK_VLAN_SUPPORT
	//if( mib_all.sys_config.vlan_enable )
		for( index=2;index<MAX_ETH_INTF;index++) //form eth2
			DEL_BR_INTERFACE(IF_BR, mib_all.eth[index].name, 0, 1); 
#endif	
}

void bring_up_lan(void)
{
	int index=0;
	unsigned char tmpbuf[100]={0}, mac[6]={0};
	unsigned char addr[6];

    //mark_mac , mii eth1 always on??
#ifdef RTK_MII_PORT_NETIF       
    memcpy(addr, ALL_ZERO_MAC_ADDR, 6);
    ADD_BR_INTERFACE(IF_BR, IF_MII, addr,0,1);
#endif

	string_to_mac(mac,mib_all.sys_config.eth_mac,6);
	ADD_BR_INTERFACE(IF_BR, mib_all.eth[0].name, mac, 0, 1); 

#ifdef CONFIG_RTK_VLAN_SUPPORT
//	if( mib_all.sys_config.vlan_enable ){
		for( index=2;index<MAX_ETH_INTF;index++){ //from eth2
			mac[5] += 0x1;
			//printf(">>> bring up %s, MAC:%02x%02x%02x%02x%02x%02x\n",mib_all.eth[index].name,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
			ADD_BR_INTERFACE(IF_BR, mib_all.eth[index].name, mac,0,1);
		}
//	}
#endif
}

void bridge_all_lan(void)
{
	int index=0;
	unsigned char tmpbuf[100]={0}, mac[6]={0};
	unsigned char addr[6];

    //mark_mac , mii eth1 always on??
#ifdef RTK_MII_PORT_NETIF    
    memcpy(addr, ALL_ZERO_MAC_ADDR, 6);
    ADD_BR_INTERFACE(IF_BR, IF_MII, addr,1,0);
#endif	
	string_to_mac(mac,mib_all.sys_config.eth_mac,6);  
	ADD_BR_INTERFACE(IF_BR, mib_all.eth[0].name, mac, 1, 0); 

#ifdef CONFIG_RTK_VLAN_SUPPORT
//	if( mib_all.sys_config.vlan_enable ){
		for( index=2;index<MAX_ETH_INTF;index++){ //from eth2
			mac[5] += 0x1;
			//printf(">>> bring up %s, MAC:%02x%02x%02x%02x%02x%02x\n",mib_all.eth[index].name,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
			ADD_BR_INTERFACE(IF_BR, mib_all.eth[index].name, mac,1,0);
		}
//	}
#endif
}

