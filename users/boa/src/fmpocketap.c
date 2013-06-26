/*
 *      Web server handler routines for management (password, save config, f/w update)
 *
 *      Authors: sc_yang <sc_yang@realtek.com.tw>
 *
 *      $Id
 *
 */
//#ifdef ROUTE_SUPPORT
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/reboot.h>
#include <unistd.h>
#include <net/route.h>
#include "boa.h"
#include "asp_page.h"
#include "apmib.h"
#include "apform.h"
#include "utility.h"

#ifdef HOME_GATEWAY

#if defined(CONFIG_POCKET_ROUTER_SUPPORT)
#define PHY_IF_FILE "/proc/phyif"
#define POCKETAP_HW_SET_FLAG "/proc/pocketAP_hw_set_flag"
#define DC_PWR_PLUGGED_FLAG "/proc/dc_pwr_plugged_flag"

#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT)
extern int Domain_query_Process();
#endif



void stopAllWlanDev(void)
{
	  system("ifconfig wlan1 down 2> /dev/null");
	  system("ifconfig wlan0 down 2> /dev/null");
	  return;
}

int pocketAPProcess()
{
	int i;
	char  pocketAP_hw_set_flag = 0;	
	int Operation_Mode=0;
	int WLAN_Mode=0;
	unsigned char LAN_Mac[12];
	char cmdBuffer[100];
	int lan_dhcp=0;
	static int wait_reinit = 0;
		
	if(isFileExist(DC_PWR_PLUGGED_FLAG))
	{
		FILE *fp=NULL;	
		unsigned char dcPwr_plugged_str[10];
		memset(dcPwr_plugged_str,0x00,sizeof(dcPwr_plugged_str));
			
		fp=fopen(DC_PWR_PLUGGED_FLAG, "r");
		if(fp!=NULL)
		{
			fgets(dcPwr_plugged_str,sizeof(dcPwr_plugged_str),fp);
			fclose(fp);

			if(strlen(dcPwr_plugged_str) != 0)
			{
				dcPwr_plugged_str[1]='\0';
				if(strcmp(dcPwr_plugged_str,"1") == 0) //plugged state changed
				{								
					system("init.sh gw all");
					wait_reinit = 5;
				}
			}
		}
	}
	
	if(isFileExist(POCKETAP_HW_SET_FLAG))
	{
		FILE *fp=NULL;	
		unsigned char pocketAP_hw_set_flag_str[10];
		memset(pocketAP_hw_set_flag_str,0x00,sizeof(pocketAP_hw_set_flag_str));
			
		fp=fopen(POCKETAP_HW_SET_FLAG, "r");
		if(fp!=NULL)
		{
			fgets(pocketAP_hw_set_flag_str,sizeof(pocketAP_hw_set_flag_str),fp);
			fclose(fp);

			if(strlen(pocketAP_hw_set_flag_str) != 0)
			{
				pocketAP_hw_set_flag_str[1]='\0';
				if(strcmp(pocketAP_hw_set_flag_str,"1") == 0)
				{
					pocketAP_hw_set_flag = 1;
				}
				else
				{
					pocketAP_hw_set_flag = 0;										
				}
			}
		}
	}

	if(pocketAP_hw_set_flag == 0)
	{	
		stopAllWlanDev();
		system("init.sh gw all");
		wait_reinit = 5;		
	}

	if(isFileExist(PHY_IF_FILE))
	{
		FILE *fp=NULL;	
		unsigned char phy_if_str[10];
		
		memset(phy_if_str,0x00,sizeof(phy_if_str));
			
		fp=fopen(PHY_IF_FILE, "r");
		if(fp!=NULL)
		{
			fgets(phy_if_str,sizeof(phy_if_str),fp);
			fclose(fp);

			if(strlen(phy_if_str) != 0)
			{
				phy_if_str[1]='\0';
				if(strcmp(phy_if_str,"0") == 0)
				{
					//system("reboot");
				}									
			}
		}
	}

	if (wait_reinit > 0) {
		wait_reinit--;
		return 0;
	}
	
#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT)
	Domain_query_Process();
#endif

	return 0;
}

#endif //#if defined(CONFIG_POCKET_ROUTER_SUPPORT)


#endif /* #ifdef HOME_GATEWAY */

//#endif //ROUTE_SUPPORT
