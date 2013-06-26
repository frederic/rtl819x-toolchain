/*
 *
 *
 */

/* System include files */
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <netdb.h>
#include <stdarg.h>
#include <errno.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/wait.h>
/* Local include files */
#include "apmib.h"
#include "mibtbl.h"

#include "sysconf.h"
#include "sys_utility.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <net/if.h>
#include <stddef.h>		/* offsetof */
#include <net/if_arp.h>
#include <linux/if_ether.h>
int apmib_initialized = 0;
extern int setinit(int argc, char** argv);
extern int Init_Internet(int argc, char** argv);
extern int setbridge(char *argv);
extern int setFirewallIptablesRules(int argc, char** argv);
extern int setWlan_Applications(char *action, char *argv);
#ifdef MULTI_PPPOE
extern void wan_disconnect(char *option , char *conncetOrder);
#else
extern void wan_disconnect(char *option);
#endif
extern int wan_dhcpcNeedRenewConn(char *interface, char *option);
extern void wan_connect(char *interface, char *option);

extern int Init_QoS(int argc, char** argv);
extern void start_lan_dhcpd(char *interface);
//extern int save_cs_to_file();


#ifdef CONFIG_DOMAIN_NAME_QUERY_SUPPORT
extern void wan_connect_pocket(char *interface, char *option);
extern int Check_setting_default(int opmode, int wlan_mode);
extern int Check_setting(int type);
extern void start_upnpd(int isgateway, int sys_op);
#endif
//////////////////////////////////////////////////////////////////////

#ifdef CONFIG_POCKET_ROUTER_SUPPORT
#define POCKETAP_HW_SET_FLAG "/proc/pocketAP_hw_set_flag"
#define AP_CLIENT_ROU_FILE "/proc/ap_client_rou"
#define DC_PWR_FILE "/proc/dc_pwr"

static void set_wlan_low_power()
{
//fprintf(stderr,"\r\n __[%s_%u]\r\n",__FILE__,__LINE__);
	system("iwpriv wlan0 set_mib txPowerPlus_cck_1=0");
	system("iwpriv wlan0 set_mib txPowerPlus_cck_2=0");	
	system("iwpriv wlan0 set_mib txPowerPlus_cck_5=0");		
	system("iwpriv wlan0 set_mib txPowerPlus_cck_11=0");
	system("iwpriv wlan0 set_mib txPowerPlus_ofdm_6=0");	
	system("iwpriv wlan0 set_mib txPowerPlus_ofdm_9=0");
	system("iwpriv wlan0 set_mib txPowerPlus_ofdm_12=0");
	system("iwpriv wlan0 set_mib txPowerPlus_ofdm_18=0");	
	system("iwpriv wlan0 set_mib txPowerPlus_ofdm_24=0");		
	system("iwpriv wlan0 set_mib txPowerPlus_ofdm_36=0");
	system("iwpriv wlan0 set_mib txPowerPlus_ofdm_48=0");	
	system("iwpriv wlan0 set_mib txPowerPlus_ofdm_54=0");
	system("iwpriv wlan0 set_mib txPowerPlus_mcs_0=0");
	system("iwpriv wlan0 set_mib txPowerPlus_mcs_1=0");
	system("iwpriv wlan0 set_mib txPowerPlus_mcs_2=0");
	system("iwpriv wlan0 set_mib txPowerPlus_mcs_3=0");
	system("iwpriv wlan0 set_mib txPowerPlus_mcs_4=0");
	system("iwpriv wlan0 set_mib txPowerPlus_mcs_5=0");
	system("iwpriv wlan0 set_mib txPowerPlus_mcs_6=0");
	system("iwpriv wlan0 set_mib txPowerPlus_mcs_7=0");
	system("iwpriv wlan0 set_mib txPowerPlus_mcs_8=0");
	system("iwpriv wlan0 set_mib txPowerPlus_mcs_9=0");
	system("iwpriv wlan0 set_mib txPowerPlus_mcs_10=0");
	system("iwpriv wlan0 set_mib txPowerPlus_mcs_11=0");
	system("iwpriv wlan0 set_mib txPowerPlus_mcs_12=0");
	system("iwpriv wlan0 set_mib txPowerPlus_mcs_13=0");
	system("iwpriv wlan0 set_mib txPowerPlus_mcs_14=0");
	system("iwpriv wlan0 set_mib txPowerPlus_mcs_15=0");	
//fprintf(stderr,"\r\n __[%s-%u]\r\n",__FILE__,__LINE__);
}


/* Fix whan device is change wlan mode from client to AP or Router. *
  * The CIPHER_SUITE of wpa or wpa2 can't be tkip                           */
static int check_wpa_cipher_suite()
{
	int wlan_band, wlan_onoff_tkip, wlan_encrypt, wpaCipher, wpa2Cipher, wdsEncrypt;

	apmib_get( MIB_WLAN_BAND, (void *)&wlan_band) ;
	apmib_get( MIB_WLAN_11N_ONOFF_TKIP, (void *)&wlan_onoff_tkip) ;					
	apmib_get( MIB_WLAN_ENCRYPT, (void *)&wlan_encrypt);
	apmib_get( MIB_WLAN_WDS_ENCRYPT, (void *)&wdsEncrypt);
	if(wlan_onoff_tkip == 0) //Wifi request
	{
		if(wlan_band == 8 || wlan_band == 10 || wlan_band == 11)//8:n; 10:gn; 11:bgn
		{
			if(wlan_encrypt ==ENCRYPT_WPA || wlan_encrypt ==ENCRYPT_WPA2){
				wpaCipher =  WPA_CIPHER_AES;
				apmib_set(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&wpaCipher);

				wpa2Cipher =  WPA_CIPHER_AES;				
				apmib_set(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&wpa2Cipher);
			}
			if(wdsEncrypt == WDS_ENCRYPT_TKIP)
			{
				wdsEncrypt = WDS_ENCRYPT_DISABLED;
				apmib_set( MIB_WLAN_WDS_ENCRYPT, (void *)&wdsEncrypt);
			}

		}

	}


}
#if defined(FOR_DUAL_BAND)	
short whichWlanIfIs(PHYBAND_TYPE_T phyBand)
{
	int i;
	int ori_wlan_idx=wlan_idx;
	int ret=-1;
	
	for(i=0 ; i<NUM_WLAN_INTERFACE ; i++)
	{
		unsigned char wlanif[10];
		memset(wlanif,0x00,sizeof(wlanif));
		sprintf(wlanif, "wlan%d",i);
		if(SetWlan_idx(wlanif))
		{
			int phyBandSelect;
			apmib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&phyBandSelect);
			if(phyBandSelect == phyBand)
			{
				ret = i;
				break;			
			}
		}						
	}
	
	wlan_idx=ori_wlan_idx;
	return ret;		
}
#if defined(CONFIG_RTL_92D_SUPPORT)
void swapWlanMibSetting(unsigned char wlanifNumA, unsigned char wlanifNumB)
{
	unsigned char *wlanMibBuf=NULL;
	unsigned int totalSize = sizeof(CONFIG_WLAN_SETTING_T)*(NUM_VWLAN_INTERFACE+1); // 4vap+1rpt+1root
	wlanMibBuf = malloc(totalSize); 
	if(wlanMibBuf != NULL)
	{
		memcpy(wlanMibBuf, pMib->wlan[wlanifNumA], totalSize);
		memcpy(pMib->wlan[wlanifNumA], pMib->wlan[wlanifNumB], totalSize);
		memcpy(pMib->wlan[wlanifNumB], wlanMibBuf, totalSize);
	
		free(wlanMibBuf);
	}
	
#ifdef UNIVERSAL_REPEATER

	int rptEnable1, rptEnable2;
	char rptSsid1[MAX_SSID_LEN], rptSsid2[MAX_SSID_LEN];
	
	memset(rptSsid1, 0x00, MAX_SSID_LEN);
	memset(rptSsid2, 0x00, MAX_SSID_LEN);
	
	apmib_get(MIB_REPEATER_ENABLED1, (void *)&rptEnable1);
	apmib_get(MIB_REPEATER_ENABLED2, (void *)&rptEnable2);
	apmib_get(MIB_REPEATER_SSID1, (void *)rptSsid1);
	apmib_get(MIB_REPEATER_SSID2, (void *)rptSsid2);
	
	apmib_set(MIB_REPEATER_ENABLED1, (void *)&rptEnable2);
	apmib_set(MIB_REPEATER_ENABLED2, (void *)&rptEnable1);
	apmib_set(MIB_REPEATER_SSID1, (void *)rptSsid2);
	apmib_set(MIB_REPEATER_SSID2, (void *)rptSsid1);
#endif
}
#endif
int switchToClientMode(void)
{
	int intVal=0;
	int i;
	
	intVal=BANDMODESINGLE;
	apmib_set(MIB_WLAN_BAND2G5G_SELECT,(void *)&intVal);

	intVal=15; //A+B+G+N mode
	apmib_set(MIB_WLAN_BAND,(void *)&intVal);
	
	for(i=0 ; i<NUM_WLAN_INTERFACE ; i++)
	{
		unsigned char wlanif[10];
		memset(wlanif,0x00,sizeof(wlanif));
		sprintf(wlanif, "wlan%d",i);
		if(SetWlan_idx(wlanif))
		{
			int intVal;
			intVal = SMACSPHY;
			apmib_set(MIB_WLAN_MAC_PHY_MODE, (void *)&intVal);
		}
	}

	SetWlan_idx("wlan0");
	/*enable wlan0*/
	intVal = 0;
	apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&intVal);

	/*set wlan0 mode to client mode*/
	intVal = 1;
	apmib_set(MIB_WLAN_MODE, (void *)&intVal);
	
	/*set wlan0 Channel Width to 40Mhz*/
	intVal=1;
	apmib_set(MIB_WLAN_CHANNEL_BONDING, (void *)&intVal) ;
		
	SetWlan_idx("wlan1");
	/*disable wlan1*/
	intVal = 1;
	apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&intVal);

	/*set wlan1 mode to client mode*/
	intVal = 1;
	apmib_set(MIB_WLAN_MODE, (void *)&intVal);

	/*set wlan1 Channel Width to 40Mhz*/
	intVal=1;
	apmib_set(MIB_WLAN_CHANNEL_BONDING, (void *)&intVal) ;

	SetWlan_idx("wlan0");
	return 0;
}

int switchFromClientMode(void)
{

	int i;
	int macPhyMode;
	int wlanBand2G5GSelect;
	int val;
	/*qinjunjie:should not force config pocket as to dual mac dual phy in AP/Router mode,
	because, if user config pocket as single mac single phy in AP/Router mode,
	but change to dual mac dual phy after reboot, ,it's un-reasonable*/

	apmib_get(MIB_WLAN_BAND2G5G_SELECT,(void *)&wlanBand2G5GSelect);
		/* Set both wireless interface is radio on and DMACDPHY */
	if(wlanBand2G5GSelect == BANDMODEBOTH)
	{
		int intVal;
		for(i=0 ; i<NUM_WLAN_INTERFACE ; i++)
		{
			unsigned char wlanif[10];
			
			
			memset(wlanif,0x00,sizeof(wlanif));
			sprintf(wlanif, "wlan%d",i);
			if(SetWlan_idx(wlanif))
			{
				
				intVal = DMACDPHY;
				apmib_set(MIB_WLAN_MAC_PHY_MODE, (void *)&intVal);
				intVal = 0;
				apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&intVal);
			}
		}
				
		
		/* 92d rule, 5g must up in wlan0 */
		/* phybandcheck */
		if(whichWlanIfIs(PHYBAND_5G) != 0)
		{
			swapWlanMibSetting(0,1);			
		}
		
		SetWlan_idx("wlan0");
		apmib_get(MIB_WLAN_BAND,(void *)&intVal);
		if(intVal == 15) //15:abgn there is no abgn band selection in ap mode
		{
			intVal = BAND_5G_11AN;
			apmib_set(MIB_WLAN_BAND,(void *)&intVal);	
		}
		
		SetWlan_idx("wlan1");
		apmib_get(MIB_WLAN_BAND,(void *)&intVal);
		if(intVal == 15) //15:abgn there is no abgn band selection in ap mode
		{
			intVal = BAND_11BG+BAND_11N;
			apmib_set(MIB_WLAN_BAND,(void *)&intVal);	
		}
	}	
	else if(wlanBand2G5GSelect == BANDMODESINGLE)
	{
		int intVal=0;
		for(i=0 ; i<NUM_WLAN_INTERFACE ; i++)
		{
			unsigned char wlanif[10];
			memset(wlanif,0x00,sizeof(wlanif));
			sprintf(wlanif, "wlan%d",i);
			if(SetWlan_idx(wlanif))
			{
				int intVal;
				intVal = SMACSPHY;
				apmib_set(MIB_WLAN_MAC_PHY_MODE, (void *)&intVal);				
			}
		}
		
		SetWlan_idx("wlan0");
		intVal = 0;
		apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&intVal);
		
		apmib_get(MIB_WLAN_BAND,(void *)&intVal);
		if(intVal == 15) //15:abgn there is no abgn band selection in ap mode
		{
			apmib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&intVal);
			if(intVal == PHYBAND_2G)
				intVal = BAND_11BG+BAND_11N;
			else
				intVal = BAND_5G_11AN;
				
			apmib_set(MIB_WLAN_BAND,(void *)&intVal);	
		}
		
		SetWlan_idx("wlan1");
		intVal = 1;
		apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&intVal);
		
		apmib_get(MIB_WLAN_BAND,(void *)&intVal);
		if(intVal == 15) //15:abgn there is no abgn band selection in ap mode
		{
			apmib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&intVal);
			if(intVal == PHYBAND_2G)
				intVal = BAND_11BG+BAND_11N;
			else
				intVal = BAND_5G_11AN;
				
			apmib_set(MIB_WLAN_BAND,(void *)&intVal);	
		}
	}							
	
	/*set wlan0 mode to ap mode*/
	SetWlan_idx("wlan0");
	val = 0;
	apmib_set(MIB_WLAN_MODE, (void *)&val);
	 
	/*set wlan1 mode to ap mode*/
	SetWlan_idx("wlan1");
	val = 0;
	apmib_set(MIB_WLAN_MODE, (void *)&val);

	SetWlan_idx("wlan0");
	return 0;
}
#endif

static int pocketAP_bootup()
{
	char	pocketAP_hw_set_flag = 0;
	int op_mode=1;
	int lan_dhcp;
	int cur_op_mode;
	int wlan0_mode;
	int ret = 0;

	apmib_get( MIB_OP_MODE, (void *)&cur_op_mode);
	apmib_get( MIB_WLAN_MODE, (void *)&wlan0_mode);
	if(isFileExist(DC_PWR_FILE))
	{
		FILE *fp=NULL;	
		unsigned char dcPwr_str[100];
		memset(dcPwr_str,0x00,sizeof(dcPwr_str));
			
		fp=fopen(DC_PWR_FILE, "r");
		if(fp!=NULL)
		{
			fgets(dcPwr_str,sizeof(dcPwr_str),fp);
			fclose(fp);

			if(strlen(dcPwr_str) != 0)
			{
				dcPwr_str[1]='\0';
				if(strcmp(dcPwr_str,"2") == 0)
				{
					set_wlan_low_power();
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
					system("echo 1 > /proc/pocketAP_hw_set_flag");					
				}
			}
		}
		
	}

	if(pocketAP_hw_set_flag == 0 && isFileExist(AP_CLIENT_ROU_FILE))
	{
		FILE *fp=NULL;	
		unsigned char ap_cli_rou_str[10];
		unsigned char kill_webs_flag = 0;
		memset(ap_cli_rou_str,0x00,sizeof(ap_cli_rou_str));		
		
		fp=fopen(AP_CLIENT_ROU_FILE, "r");
		if(fp!=NULL)
		{
			fgets(ap_cli_rou_str,sizeof(ap_cli_rou_str),fp);
			fclose(fp);

			if(strlen(ap_cli_rou_str) != 0)
			{
				ap_cli_rou_str[1]='\0';
												
				if((cur_op_mode != 1 || wlan0_mode == CLIENT_MODE) && strcmp(ap_cli_rou_str,"2") == 0) //AP
				{
					cur_op_mode = 1;
					wlan0_mode = 0;
					lan_dhcp = 15;
					apmib_set( MIB_OP_MODE, (void *)&cur_op_mode);
					apmib_set( MIB_WLAN_MODE, (void *)&wlan0_mode);
				#if defined(FOR_DUAL_BAND)
					switchFromClientMode();	
				#endif
					check_wpa_cipher_suite();
				#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT)
					apmib_set( MIB_DHCP, (void *)&lan_dhcp);
					Check_setting(2);//ap
				#endif				
					if(apmib_update(CURRENT_SETTING) == 1)
						save_cs_to_file();

					reinit_webs();
					//RunSystemCmd(NULL_FILE, "boa", NULL_STR);
				}
				else if((cur_op_mode != 1 || wlan0_mode != CLIENT_MODE) && strcmp(ap_cli_rou_str,"1") == 0) //CLIENT
				{
					cur_op_mode = 1;
					wlan0_mode = 1;
					lan_dhcp = 15;
					
					apmib_set( MIB_OP_MODE, (void *)&cur_op_mode);
					apmib_set( MIB_WLAN_MODE, (void *)&wlan0_mode);		
				#if defined(FOR_DUAL_BAND)
					switchToClientMode();
				#endif
				#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT)
					apmib_set( MIB_DHCP, (void *)&lan_dhcp);
					Check_setting(1);//client
				#endif
					if(apmib_update(CURRENT_SETTING) == 1)
						save_cs_to_file();
					reinit_webs();
					
				}
				else if(cur_op_mode != 0 && strcmp(ap_cli_rou_str,"3") == 0) //router
				{
					cur_op_mode = 0;
					wlan0_mode = 0;
					lan_dhcp = 2;

					apmib_set( MIB_OP_MODE, (void *)&cur_op_mode);
					apmib_set( MIB_WLAN_MODE, (void *)&wlan0_mode);
				#if defined(FOR_DUAL_BAND)
					switchFromClientMode();					
				#endif
					check_wpa_cipher_suite();
				#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT)
					apmib_set( MIB_DHCP, (void *)&lan_dhcp);
					Check_setting(3);//router
				#endif
					if(apmib_update(CURRENT_SETTING) == 1)
						save_cs_to_file();
					reinit_webs();
					//RunSystemCmd(NULL_FILE, "boa", NULL_STR);				
				}
				else
				{
					#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT)
					apmib_get(MIB_OP_MODE, (void *)&op_mode);
					if(op_mode == 0)
					{
						lan_dhcp = 2;
						apmib_set(MIB_DHCP, (void *)&lan_dhcp);
					}
					else
					{
						lan_dhcp = 15;
						apmib_set(MIB_DHCP, (void *)&lan_dhcp);
					}
					ret=Check_setting_default(cur_op_mode, wlan0_mode);
					#if defined(FOR_DUAL_BAND)	
					apmib_get( MIB_WLAN_MODE, (void *)&wlan0_mode);
					if(wlan0_mode == 1)
					{
						switchToClientMode();
					}					
					#endif
				
					if(ret==1){
						if(apmib_update(CURRENT_SETTING) == 1)
							save_cs_to_file();

						reinit_webs();	
					}	
					#endif
				}
			}
		}
	}
	else
	{

		#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT)
		apmib_get(MIB_OP_MODE, (void *)&op_mode);
		if(op_mode == 0)
		{
			lan_dhcp = 2;
			apmib_set(MIB_DHCP, (void *)&lan_dhcp);
		}
		else
		{
			lan_dhcp = 15;
			apmib_set(MIB_DHCP, (void *)&lan_dhcp);
		}

		ret=Check_setting_default(cur_op_mode, wlan0_mode);
		if(ret==1){
			if(apmib_update(CURRENT_SETTING) == 1)
				save_cs_to_file();

			reinit_webs();	
		}	
		#endif
	}

}
#endif


#if defined(CONFIG_RTL_ULINKER)
extern int ulinker_bootup(void);
extern int ulinker_wlan_init(void);
#endif /* #if defined(CONFIG_RTL_ULINKER)  */


int main(int argc, char** argv)
{
	char	line[300];
	char action[16];
	int i;
	//printf("start.......:%s\n",argv[1]);
    #if 0
    if(strcmp(argv[1],"firewall"))
{
    printf("******************\n");
    for(i=0;i<argc;i++)
    {
        printf("%s ",argv[i]);
    }
    printf("\n***************\n");
 }
   #endif	
	
	
	if ( !apmib_init()) {
		printf("Initialize AP MIB failed !\n");
		return -1;
	}
	apmib_initialized = 1;
	memset(line,0x00,300);
	
	if(argv[1] && (strcmp(argv[1], "init")==0)){
#if defined(CONFIG_RTL_ULINKER)
  #if defined(CONFIG_RTL_ULINKER_WLAN_DELAY_INIT)
	int ulinker_auto = 0;
	apmib_get(MIB_ULINKER_AUTO,  (void *)&ulinker_auto);
	if (ulinker_auto == 0)
		system("echo \"wlan 1\" > /proc/wlan_init");
  #endif

	if (strcmp(argv[3], "all")==0) {
		ulinker_bootup();
	}

#elif defined(CONFIG_POCKET_ROUTER_SUPPORT)
	pocketAP_bootup();
#endif

#ifdef CONFIG_POCKET_AP_SUPPORT
		i=BRIDGE_MODE;
		apmib_set(MIB_OP_MODE,(void *)&i);
		apmib_get(MIB_DHCP, (void *)&i);	//for FC, dhcp server not allowed when client
		if( i > DHCP_CLIENT )	i=0;
		apmib_set(MIB_DHCP, (void *)&i);	//0:DHCP_DISABLED
		i = 0xc0a801fa;
		apmib_set(MIB_IP_ADDR, (void *)&i);	//for FC, default IP to 192.168.1.250
#endif
		setinit(argc,argv);

#if defined(CONFIG_RTL_ULINKER_WLAN_DELAY_INIT)
	if (ulinker_auto == 1)
		ulinker_wlan_init();
#endif
		return 0;
	} else if(argv[1] && (strcmp(argv[1], "br")==0)){
		for(i=0;i<argc;i++){
			if( i>2 )
				string_casecade(line, argv[i]);
		}
		setbridge(line);
	}
#ifdef   HOME_GATEWAY	
	else if(argv[1] && (strcmp(argv[1], "firewall")==0)){		
		if(argv[2] && (strcmp(argv[2], "Send_GARP")==0))	//it will be call by set_staticIP function
		{
			#ifdef SEND_GRATUITOUS_ARP
			sendArp();
			#endif
		}
		else
		{
			setFirewallIptablesRules(argc,argv);
		}
	}
	else if(argv[1] && (strcmp(argv[1], "wlanapp")==0)){
		for(i=0;i<argc;i++){
			if( i>2 )
				string_casecade(line, argv[i]);
			if(i==2)
				sprintf(action, "%s",argv[i]); 
		}
		setWlan_Applications(action, line);
	}else if(argv[1] && (strcmp(argv[1], "disc")==0)){
		sprintf(line, "%s", argv[2]);
#ifdef MULTI_PPPOE
		if(argv[3])
			wan_disconnect(line,argv[3]);
		else 
			wan_disconnect(line,"NOMULPPPOE");
#else
		wan_disconnect(line);
#endif
	}else if(argv[1] && 
		((strcmp(argv[1], "conn")==0)||(strcmp(argv[1], "renew")==0))){
		
		if(argc < 4){
			printf("sysconf conn Invalid agrments!\n");
			return 0;
		}
		sprintf(action, "%s",argv[3]);
		for(i=0;i<argc;i++){
				if( i>2 )
					string_casecade(line, argv[i]);
			}
		if((strcmp(argv[1], "renew")==0)&&!strcmp(argv[2],"dhcp") &&!wan_dhcpcNeedRenewConn(action,line))
		{
			return 0;
		}
			
#if defined(CONFIG_DYNAMIC_WAN_IP)
		if((!strcmp(argv[2], "dhcp"))&&(isFileExist(TEMP_WAN_CHECK))){
			RunSystemCmd(TEMP_WAN_DHCP_INFO, "echo", line, NULL_STR);
		}
#endif

#if defined(CONFIG_RTL_ULINKER)
	/* notice ulinker_process to reset domain name query */
	system("echo 1 > /var/ulinker_reset_domain");
#endif
		
#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT)
		if(!strcmp(action, "br0"))
			wan_connect_pocket(action, line);
		else
		wan_connect(action, line);
#else
		wan_connect(action, line);
#endif
	}else if(argv[1] && (strcmp(argv[1], "pppoe")==0)){
		Init_Internet(argc,argv);
	}else if(argv[1] && (strcmp(argv[1], "pptp")==0)){
		Init_Internet(argc,argv);
	}else if(argv[1] && (strcmp(argv[1], "l2tp")==0)){
		Init_Internet(argc,argv);
	}else if(argv[1] && (strcmp(argv[1], "setQos")==0)){
		Init_QoS(argc,argv);
	}else if(argv[1] && (strcmp(argv[1], "dhcpd")==0)){
		sprintf(action, "%s",argv[2]);
		start_lan_dhcpd(action);
	}
#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT)	
	else if(argv[1] && (strcmp(argv[1], "upnpd")==0)){
		if(argc < 4){
			printf("sysconf upnpd Invalid agrments!\n");
			return 0;
	}
		start_upnpd(atoi(argv[2]),atoi(argv[3]));
	} 
#endif	

//### add by sen_liu 2011.4.21 sync the system log update (enlarge from 1 pcs to 8 pcs) to	SDKv2.5 from kernel 2.4
#if defined(RINGLOG)
	else if(argv[1] && (strcmp(argv[1], "log")==0)){
		if (argc == 6 && !strcmp(argv[2], "-s") && !strcmp(argv[4], "-b"))
		{
			system("killall syslogd >/dev/null 2>&1");
			system("rm /var/log/log_split >/dev/null 2>&1");
			sprintf(line, "echo %s > /var/log/log_split", argv[5]);
			system(line);
			RunSystemCmd(NULL_FILE, "syslogd", "-L", "-s", argv[3], "-b", argv[5], NULL_STR);
			fprintf(stderr, "syslog will use %dKB for log(%s rotate, 1 original, %sKB for each).\n",
				atoi(argv[3]) * ((atoi(argv[5]))+1), argv[5], argv[3]);
		}
	/*
		else if (argc == 4 && !strcmp(argv[2], "-R"))
		{
			RunSystemCmd(NULL_FILE, "killall syslogd", NULL_STR);
			RunSystemCmd(NULL_FILE, "syslogd", "-L", "-R", argv[3], NULL_STR);
		}
	*/
		else
		{
			fprintf(stderr, "usage:\n");
			fprintf(stderr, "sysconf log -s size -b number-of-rotate-backup\n");
			//fprintf(stderr, "sysconf log -R IP\n");
		}
	}
#endif
//### end
	
#endif	
	
//#ifdef CONFIG_POCKET_ROUTER_SUPPORT
//	system("boa");
//#endif		
	return 0;
}
////////////////////////////////////////////////////////////////////////

