/*
 *      Utiltiy function for setting wlan application 
 *
 */

/*-- System inlcude files --*/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <arpa/inet.h>
#include <unistd.h>
    
#include "apmib.h"
#include "sysconf.h"
#include "sys_utility.h"
extern int SetWlan_idx(char * wlan_iface_name);
//extern int wlan_idx;	// interface index 
//extern int vwlan_idx;	// initially set interface index to root   
extern int apmib_initialized;

#define IWCONTROL_PID_FILE "/var/run/iwcontrol.pid"
#define PATHSEL_PID_FILE "/var/run/pathsel.pid"
#define IAPP_PID_FILE "/var/run/iapp.pid"
#define MESH_PATHSEL "/bin/pathsel" 

#ifdef CONFIG_RTL_WAPI_SUPPORT
#define MAX_WAPI_CONF_NUM 10
int asIpExist(WAPI_ASSERVER_CONF_T *pwapiconf, const unsigned char wapiCertSel, char *asipaddr, int *index)
{
	int j;
	if(NULL == pwapiconf || NULL == asipaddr || NULL == index )
		return 0;
	for(j=0;j<MAX_WAPI_CONF_NUM;j++)
	{
		if(pwapiconf[j].valid){
			if((!memcmp(pwapiconf[j].wapi_asip,asipaddr,4)) && (pwapiconf[j].wapi_cert_sel == wapiCertSel)){
				*index=j;
				return 1;
			}
		} else {
			*index = j;
			break;
		}		
	}
	return 0;
}
#endif

int setWlan_Applications(char *action, char *argv)
{
	int pid=-1;
	char strPID[10];
	char iface_name[16];
	char tmpBuff[100], tmpBuff1[100], arg_buff[200],wlan_wapi_asipaddr[100];
	int wlan_wapi_cert_sel;
	int _enable_1x=0, _use_rs=0;
	int wlan_mode_root=0,wlan_disabled_root=0, wlan_wpa_auth_root=0; 
	int wlan0_mode=1, wlan1_mode=1, both_band_ap=0;
	int wlan_iapp_disabled_root=0,wlan_wsc_disabled_root=0, wlan_network_type_root=0, wlan0_wsc_disabled_vxd=0, wlan1_wsc_disabled_vxd=0;
	int wlan_1x_enabled_root=0, wlan_encrypt_root=0, wlan_mac_auth_enabled_root=0,wlan_wapi_auth=0;
	int wlan_disabled=0, wlan_mode=0, wlan_wds_enabled=0, wlan_wds_num=0;
	int wlan_encrypt=0, wlan_wds_encrypt=0;
	int wlan_wpa_auth=0, wlan_mesh_encrypt=0;
	int wlan_1x_enabled=0,wlan_mac_auth_enabled=0;
	int wlan_root_auth_enable=0, wlan_vap_auth_enable=0;
	int wlan_network_type=0, wlan_wsc_disabled=0, wlan_iapp_disabled=0;
	char tmp_iface[30]={0}, wlan_role[30]={0}, wlan_vap[30]={0}, wlan_vxd[30]={0};
	char valid_wlan_interface[200]={0}, all_wlan_interface[200]={0};
	int vap_not_in_pure_ap_mode=0, deamon_created=0;
	int isWLANEnabled=0, isAP=0, isIAPPEnabled=0, intValue=0;
	char bridge_iface[30]={0};
	char *token=NULL, *savestr1=NULL;
	int wps_debug=0, use_iwcontrol=1;
	int WSC=1, WSC_UPNP_Enabled=0;
	FILE *fp;
	char wsc_pin_local[16]={0},wsc_pin_peer[16]={0};
	int wait_fifo=0;
	char *cmd_opt[16]={0};
	int cmd_cnt = 0;
	//Added for virtual wlan interface
	int i=0, wlan_encrypt_virtual=0;
	char wlan_vname[16];
#ifdef CONFIG_RTL_WAPI_SUPPORT
	/*assume MAX 10 configuration*/
	char wlan_name[10];
	int wlan_index;
	int index;
	int apAsAS;
	int wlanBand2G5GSelect;
	WAPI_ASSERVER_CONF_T wapiconf[MAX_WAPI_CONF_NUM];
	int wlan_wapi_auth_root;
#endif

	int wlan_wsc1_disabled = 1 ;
	int wlan1_disabled_root = 1;


#if defined(FOR_DUAL_BAND)
	SetWlan_idx("wlan0");
	apmib_get( MIB_WLAN_MODE, (void *)&wlan0_mode);
	apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&wlan_disabled_root);
	apmib_get(MIB_WLAN_WSC_DISABLE, (void *)&wlan_wsc_disabled);
	SetWlan_idx("wlan1");
	apmib_get( MIB_WLAN_MODE, (void *)&wlan1_mode);
	apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&wlan1_disabled_root);
	apmib_get(MIB_WLAN_WSC_DISABLE, (void *)&wlan_wsc1_disabled);
	if( ((wlan0_mode == AP_MODE) || (wlan0_mode == AP_WDS_MODE)) && ((wlan1_mode == 0) || (wlan1_mode == AP_WDS_MODE))
		&& (wlan_disabled_root == 0) && (wlan1_disabled_root == 0) && (wlan_wsc_disabled == 0) && (wlan_wsc1_disabled == 0))
		both_band_ap = 1;	
	SetWlan_idx("wlan0");
#endif

	
#ifdef CONFIG_RTL_P2P_SUPPORT							
	int p2p_mode=0;
#endif

	token=NULL;
	savestr1=NULL;	     
	sprintf(arg_buff, "%s", argv);	

	token = strtok_r(arg_buff," ", &savestr1);
	do{
		if (token == NULL){/*check if the first arg is NULL*/
			break;
		}else{        
			sprintf(iface_name, "%s", token);                                           		
			if(strncmp(iface_name, "wlan", 4)==0){//wlan iface   
				if(all_wlan_interface[0]==0x0){
					sprintf(all_wlan_interface, "%s",iface_name); 
				}else{
					sprintf(tmp_iface, " %s", iface_name);
					strcat(all_wlan_interface, tmp_iface);
				}
			}else{
				sprintf(bridge_iface, "%s", iface_name);
			}
		}
		token = strtok_r(NULL, " ", &savestr1);
	}while(token !=NULL);
	
	if(isFileExist(IWCONTROL_PID_FILE)){
		pid=getPid_fromFile(IWCONTROL_PID_FILE);
		if(pid != -1){
			sprintf(strPID, "%d", pid);
			RunSystemCmd(NULL_FILE, "kill", "-9", strPID, NULL_STR);
		}
		unlink(IWCONTROL_PID_FILE);
	}
	if(isFileExist(PATHSEL_PID_FILE)){
		pid=getPid_fromFile(PATHSEL_PID_FILE);
		if(pid != -1){
			sprintf(strPID, "%d", pid);
			RunSystemCmd(NULL_FILE, "kill", "-9", strPID,NULL_STR);
		}
		unlink(PATHSEL_PID_FILE);
		RunSystemCmd(NULL_FILE, "brctl", "meshsignaloff",NULL_STR);
		
	}
	token=NULL;
	savestr1=NULL;	     
	sprintf(arg_buff, "%s", all_wlan_interface);
	token = strtok_r(arg_buff," ", &savestr1);
	do{
		if (token == NULL){/*check if the first arg is NULL*/
			break;
		}else{	
			sprintf(iface_name, "%s", token);
			if(strncmp(iface_name, "wlan", 4)==0){//wlan iface
				sprintf(tmpBuff, "/var/run/auth-%s.pid",iface_name);
				if(isFileExist(tmpBuff)){
					pid=getPid_fromFile(tmpBuff);
						if(pid != -1){
							sprintf(strPID, "%d", pid);
							RunSystemCmd(NULL_FILE, "kill", "-9", strPID, NULL_STR);
						}
					unlink(tmpBuff);
					sprintf(tmpBuff1, "/var/run/auth-%s-vxd.pid",iface_name);
					if(isFileExist(tmpBuff1)){
					pid=getPid_fromFile(tmpBuff1);
						if(pid != -1){
							sprintf(strPID, "%d", pid);
							RunSystemCmd(NULL_FILE, "kill", "-9", strPID, NULL_STR);
						}
					unlink(tmpBuff1);
					}
				}
#if 1 //def FOR_DUAL_BAND
			if(both_band_ap == 1)
				sprintf(tmpBuff1, "/var/run/wscd-wlan0-wlan1.pid");
			else
				sprintf(tmpBuff1, "/var/run/wscd-%s.pid",iface_name);
#endif
				if(isFileExist(tmpBuff1)){
					pid=getPid_fromFile(tmpBuff1);
					if(pid != -1){
						sprintf(strPID, "%d", pid);
						RunSystemCmd(NULL_FILE, "kill", "-9", strPID, NULL_STR);
					}
					unlink(tmpBuff1);
				}
#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)

				do{
					sprintf(tmpBuff1, "/var/run/wscd-%s-vxd.pid",iface_name);
					if(isFileExist(tmpBuff1))
					{
						pid=getPid_fromFile(tmpBuff1);
						if(pid != -1){
							sprintf(strPID, "%d", pid);
							RunSystemCmd(NULL_FILE, "kill", "-9", strPID, NULL_STR);
						}
						else
							break;
						unlink(tmpBuff1);
						sleep(1);					
					}										

				}while(find_pid_by_name("wscd") > 0);
				
#endif

				RunSystemCmd("/proc/gpio", "echo", "0", NULL_STR);///is it need to do this for other interface??????except wps
			}
			
			do{
				if(isFileExist("/var/run/wscd-wlan0.pid"))
				{
					pid=getPid_fromFile("/var/run/wscd-wlan0.pid");
					if(pid != -1){
						sprintf(strPID, "%d", pid);
						RunSystemCmd(NULL_FILE, "kill", "-9", strPID, NULL_STR);
					}
					else
						break;
					unlink("/var/run/wscd-wlan0.pid");
					sleep(1);					
				}
				else if(isFileExist("/var/run/wscd-wlan1.pid"))
				{
					pid=getPid_fromFile("/var/run/wscd-wlan1.pid");
					if(pid != -1){
						sprintf(strPID, "%d", pid);
						RunSystemCmd(NULL_FILE, "kill", "-9", strPID, NULL_STR);
					}
					else
						break;
					unlink("/var/run/wscd-wlan1.pid");
					sleep(1);					
				}
				else if(isFileExist("/var/run/wscd-wlan0-wlan1.pid"))
				{
					pid=getPid_fromFile("/var/run/wscd-wlan0-wlan1.pid");
					if(pid != -1){
						sprintf(strPID, "%d", pid);
						RunSystemCmd(NULL_FILE, "kill", "-9", strPID, NULL_STR);
					}
					else
						break;
					unlink("/var/run/wscd-wlan0-wlan1.pid");
					sleep(1);					
				}
				else
					break;
			}while(find_pid_by_name("wscd") > 0);
		}	
		token = strtok_r(NULL, " ", &savestr1);
	}while(token !=NULL);
	
	if(isFileExist(IAPP_PID_FILE)){
		pid=getPid_fromFile(IAPP_PID_FILE);
		if(pid != -1){
			sprintf(strPID, "%d", pid);
			RunSystemCmd(NULL_FILE, "kill", "-9", strPID, NULL_STR);
		}
		unlink(IAPP_PID_FILE);
	}
	//RunSystemCmd(NULL_FILE, "rm", "-f", "/var/*.fifo", NULL_STR);
	system("rm -f /var/*.fifo");
	if(!strcmp(action, "kill"))
		return 0;
	printf("Init Wlan application...\n");
	//get root setting first//no this operate in script
	if(SetWlan_idx("wlan0")){
		apmib_get( MIB_WLAN_WLAN_DISABLED, (void *)&wlan_disabled_root);
		apmib_get( MIB_WLAN_MODE, (void *)&wlan_mode_root); 
		apmib_get( MIB_WLAN_IAPP_DISABLED, (void *)&wlan_iapp_disabled_root);
		apmib_get( MIB_WLAN_WSC_DISABLE, (void *)&wlan_wsc_disabled_root);
		apmib_get( MIB_WLAN_ENABLE_1X, (void *)&wlan_1x_enabled_root);
		apmib_get( MIB_WLAN_ENCRYPT, (void *)&wlan_encrypt_root);

#ifdef CONFIG_RTL_WAPI_SUPPORT
		apmib_get(MIB_WLAN_WAPI_AUTH, (void *)&wlan_wapi_auth_root);
#endif
		
		apmib_get( MIB_WLAN_MAC_AUTH_ENABLED, (void *)&wlan_mac_auth_enabled_root);
		apmib_get( MIB_WLAN_NETWORK_TYPE, (void *)&wlan_network_type_root);
		apmib_get( MIB_WLAN_WPA_AUTH, (void *)&wlan_wpa_auth_root);
		apmib_get( MIB_WLAN_WSC_UPNP_ENABLED, (void *)&WSC_UPNP_Enabled);

#ifdef CONFIG_RTL_P2P_SUPPORT							
		apmib_get( MIB_WLAN_P2P_TYPE, (void *)&p2p_mode); 
#endif							


		// For WAPI.now not support  VAP
//		apmib_get(MIB_WLAN_WAPI_AUTH, (void *)&wlan_wapi_auth);
//		memset(wlan_wapi_asipaddr,0x00,sizeof(wlan_wapi_asipaddr));
//		apmib_get(MIB_WLAN_WAPI_ASIPADDR,  (void*)wlan_wapi_asipaddr);
		
	}

	token=NULL;
	savestr1=NULL;
	sprintf(arg_buff, "%s", all_wlan_interface);
	token = strtok_r(arg_buff," ", &savestr1);
	do{
		_enable_1x=0;
		_use_rs=0;

		if (token == NULL){/*check if the first arg is NULL*/
			break;
		}else{                
			sprintf(iface_name, "%s", token); 
			if(strncmp(iface_name, "wlan", 4)==0){//wlan iface   
					
				if(strlen(iface_name)>=9){
					wlan_vap[0]=iface_name[6];
					wlan_vap[1]=iface_name[7];	
				}else{
					wlan_vap[0]=0;
				}
				
				if(SetWlan_idx(iface_name)){
					
					apmib_get( MIB_WLAN_WLAN_DISABLED, (void *)&wlan_disabled);
					apmib_get( MIB_WLAN_MODE, (void *)&wlan_mode); 
					apmib_get( MIB_WLAN_WDS_ENABLED, (void *)&wlan_wds_enabled);
					apmib_get( MIB_WLAN_WDS_NUM, (void *)&wlan_wds_num);
					apmib_get( MIB_WLAN_ENCRYPT, (void *)&wlan_encrypt);
					apmib_get( MIB_WLAN_WPA_AUTH, (void *)&wlan_wpa_auth);
					
					if(wlan_disabled==0 && wlan_mode >3){
						apmib_get( MIB_MESH_ENCRYPT, (void *)&wlan_mesh_encrypt);
					}
					if(wlan_disabled==0 && (wlan_mode ==2 || wlan_mode ==3) && (wlan_wds_enabled !=0) &&(wlan_wds_num!=0)){
						apmib_get( MIB_WLAN_WDS_ENCRYPT, (void *)&wlan_wds_encrypt);
						if(wlan_wds_encrypt==3 || wlan_wds_encrypt==4){
							sprintf(tmpBuff, "/var/wpa-wds-%s.conf",iface_name);//encrytp conf file
							RunSystemCmd(NULL_FILE, "flash", "wpa", iface_name, tmpBuff, "wds", NULL_STR); 
							RunSystemCmd(NULL_FILE, "auth", iface_name, bridge_iface, "wds", tmpBuff, NULL_STR); 
							sprintf(tmpBuff1, "/var/run/auth-%s.pid",iface_name);//auth pid file
							do{
								if(isFileExist(tmpBuff1)){//check pid file is exist or not
									break;
								}else{
									sleep(1);
								}
							}while(1);
						}
						
					}

					if(wlan_encrypt < 2){
						apmib_get( MIB_WLAN_ENABLE_1X, (void *)&wlan_1x_enabled);
						apmib_get( MIB_WLAN_MAC_AUTH_ENABLED, (void *)&wlan_mac_auth_enabled);
						if(wlan_1x_enabled != 0 || wlan_mac_auth_enabled != 0){
							_enable_1x=1;
							_use_rs=1;
						}
					}else{
						if(wlan_encrypt != 7){	// not wapi
							_enable_1x=1;
							if(wlan_wpa_auth ==1){
								_use_rs=1;
							}
						}
					}
					
					////////for mesh start
					if(wlan_disabled==0 && wlan_mode >3){
						if(wlan_mesh_encrypt != 0){
							
							sprintf(tmpBuff, "/var/wpa-%s-msh0.conf",iface_name);//encrytp conf file
							RunSystemCmd(NULL_FILE, "flash", "wpa", iface_name, tmpBuff, "msh", NULL_STR); 
							sprintf(tmp_iface, "%s-msh0", iface_name);
							RunSystemCmd(NULL_FILE, "auth", tmp_iface, bridge_iface,"wds", tmpBuff, NULL_STR); 
							sprintf(tmpBuff1, "/var/run/auth-%s-msh0.pid",iface_name);//auth pid file
							do{
								if(isFileExist(tmpBuff1)){//check pid file is exist or not
									break;
								}else{
									sleep(1);
								}
							}while(1);
							
							
						}
					}
					
					///////for mesh end

					
					if(_enable_1x !=0 && wlan_disabled==0){
						
						sprintf(tmpBuff, "/var/wpa-%s.conf",iface_name);//encrytp conf file
						
						RunSystemCmd(NULL_FILE, "flash", "wpa", iface_name, tmpBuff, NULL_STR); 
						if(wlan_mode==1){//client mode
							apmib_get( MIB_WLAN_NETWORK_TYPE, (void *)&wlan_network_type);
							if(wlan_network_type==0){
								sprintf(wlan_role, "%s", "client-infra");
							}else{
								sprintf(wlan_role, "%s", "client-adhoc");
							}
						}else{
							sprintf(wlan_role, "%s", "auth");
						}
						
						if(wlan_vap[0]=='v' && wlan_vap[1]=='a'){
							if(wlan_mode_root != 0 && wlan_mode_root != 3){
								vap_not_in_pure_ap_mode=1;
							}
						}
						if(wlan_mode != 2 && vap_not_in_pure_ap_mode==0){
							
							if(wlan_wpa_auth != 2 || _use_rs !=0 ){
								deamon_created=1;
								RunSystemCmd(NULL_FILE, "auth", iface_name, bridge_iface, wlan_role, tmpBuff, NULL_STR); 
								
								if(wlan_vap[0]=='v' && wlan_vap[1]=='a')
									wlan_vap_auth_enable=1;
								else
									wlan_root_auth_enable=1;
							}
						} 
					}
				
					if(wlan_vap[0]=='v' && wlan_vap[1]=='x' && wlan_disabled==0){
						if(strcmp(wlan_role, "auth") || (!strcmp(wlan_role, "auth") && (_use_rs !=0)))
						{
							strcat(wlan_vxd," ");
							strcat(wlan_vxd,iface_name);
						}
					}
					
					if(wlan_vap[0]=='v' && wlan_vap[1]=='a'){
						if(wlan_disabled==0){
							if(wlan_iapp_disabled_root==0 || wlan_vap_auth_enable==1){
								if(valid_wlan_interface[0]==0){
									sprintf(valid_wlan_interface, "%s",iface_name); 
								}else{
									sprintf(tmp_iface, " %s", iface_name);
									strcat(valid_wlan_interface, tmp_iface);
								}
							}
						}
					}else{
						if(wlan_vap[0] !='v' && wlan_vap[1] !='x'){
							apmib_get( MIB_WLAN_IAPP_DISABLED, (void *)&wlan_iapp_disabled);
							apmib_get( MIB_WLAN_WSC_DISABLE, (void *)&wlan_wsc_disabled); 
							if(wlan_disabled==0 && (wlan_root_auth_enable==1 || wlan_iapp_disabled==0 || wlan_wsc_disabled==0)){
								if(valid_wlan_interface[0]==0){
									sprintf(valid_wlan_interface, "%s",iface_name); 
								}else{
									sprintf(tmp_iface, " %s", iface_name);
									strcat(valid_wlan_interface, tmp_iface);
								}
							}
						}
					}
				
						if((wlan_vap[0] !='v' && wlan_vap[1] !='a') && (wlan_vap[0] !='v' && wlan_vap[1] !='x')){
							 if(wlan_disabled==0)
							 	isWLANEnabled=1;
							 if(wlan_mode ==0 || wlan_mode ==3 || wlan_mode ==4 || wlan_mode ==6)
							 	isAP=1;
							 if(wlan_iapp_disabled==0)
							 	isIAPPEnabled=1;
						}
				}	
			}
		}
		token = strtok_r(NULL, " ", &savestr1);
	}while(token !=NULL);
		
	if(isWLANEnabled==1 && isAP==1 && isIAPPEnabled==1){
#if defined(CONFIG_RTL_ULINKER)
		//fixme: disable iapp temporary
#else
		sprintf(tmpBuff, "iapp %s %s",bridge_iface, valid_wlan_interface);
		system(tmpBuff);
		
		deamon_created=1;
        if(isFileExist(RESTART_IAPP))
            unlink(RESTART_IAPP);
        RunSystemCmd(RESTART_IAPP, "echo", tmpBuff, NULL_STR);
#endif
	}
	
//for mesh========================================================
	if(wlan_mode_root ==4 || wlan_mode_root ==5 || wlan_mode_root ==6 || wlan_mode_root ==7 ){
		apmib_get( MIB_MESH_ENABLE, (void *)&intValue); 
		if(intValue==1){
			system("pathsel br0 wlan0 &");
		}
	}


//========================================================
//for WPS
	if (isFileExist("/bin/wscd")) {
		memset(tmpBuff, 0x00, 100);
		memset(tmpBuff1, 0x00, 100);
		token=NULL;
		savestr1=NULL;	     
		sprintf(arg_buff, "%s", valid_wlan_interface);
		
#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)		
		int isRptEnabled1=0;
		int isRptEnabled2=0;
		
		SetWlan_idx("wlan0-vxd");
		apmib_get(MIB_REPEATER_ENABLED1, (void *)&isRptEnabled1);
		apmib_get(MIB_WLAN_WSC_DISABLE, (void *)&wlan0_wsc_disabled_vxd);
		SetWlan_idx("wlan1-vxd");
		apmib_get(MIB_REPEATER_ENABLED1, (void *)&isRptEnabled1);
		apmib_get( MIB_WLAN_WSC_DISABLE, (void *)&wlan1_wsc_disabled_vxd);

		memset(wlan_vxd, 0x00, sizeof(wlan_vxd));
		if(isRptEnabled1 == 1 && wlan0_wsc_disabled_root == 0
#if defined(CONFIG_RTL_ULINKER)
			&& wlan_mode_root != CLIENT_MODE
#endif			
#if defined(CONFIG_ONLY_SUPPORT_CLIENT_REPEATER_WPS)
			&& wlan_mode_root == CLIENT_MODE
#endif			
		)
		{
			sprintf(wlan_vxd, "%s", "wlan0-vxd");
			}

		if(isRptEnabled2 == 1 && wlan1_wsc_disabled_root == 0
#if defined(CONFIG_RTL_ULINKER)
			&& wlan_mode_root != CLIENT_MODE
#endif			
#if defined(CONFIG_ONLY_SUPPORT_CLIENT_REPEATER_WPS)
			&& wlan_mode_root == CLIENT_MODE
#endif			
		)
		{			
			strcat(wlan_vxd, " wlan1-vxd");
		}

		sprintf(tmpBuff," %s",wlan_vxd);
		strcat(arg_buff, tmpBuff);
#endif	//#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)			

		token = strtok_r(arg_buff," ", &savestr1);
			
		//token = strtok_r(valid_wlan_interface," ", &savestr1);
		do{
			if (token == NULL){
				break;
			}else{
				unsigned char wscConfFile[40];
				unsigned char wscFifoFile[40];
				memset(wscConfFile, 0x00, sizeof(wscConfFile));
				memset(wscFifoFile, 0x00, sizeof(wscFifoFile));

				_enable_1x=0;
				wps_debug=0;
				WSC=1;
				use_iwcontrol=1;

				if(!strcmp(token, "wlan0") //root if
					|| !strcmp(token, "wlan1") 
#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)
					|| !strcmp(token, "-vxd")
#endif					
				)
				{
#if 1 //defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)
					//if(strcmp(token, "wlan0-vxd") == 0) // here we ONLY get vxd mib value 
					{
						SetWlan_idx(token);
						//apmib_get( MIB_WLAN_DISABLED, (void *)&wlan_disabled_root);
						apmib_get( MIB_WLAN_MODE, (void *)&wlan_mode_root);
						if(!strcmp(token, "wlan1") && (both_band_ap == 1))
						{
								token = strtok_r(NULL, " ", &savestr1);
								continue;
						}
						//apmib_get( MIB_WSC_DISABLE, (void *)&wlan_wsc_disabled_root);
						apmib_get( MIB_WLAN_ENABLE_1X, (void *)&wlan_1x_enabled_root);
						apmib_get( MIB_WLAN_ENCRYPT, (void *)&wlan_encrypt_root);
#ifdef CONFIG_RTL_WAPI_SUPPORT
						apmib_get(MIB_WLAN_WAPI_AUTH, (void *)&wlan_wapi_auth_root);
#endif
						apmib_get( MIB_WLAN_MAC_AUTH_ENABLED, (void *)&wlan_mac_auth_enabled_root);
						//apmib_get( MIB_WLAN_NETWORK_TYPE, (void *)&wlan_network_type_root);
						apmib_get( MIB_WLAN_WPA_AUTH, (void *)&wlan_wpa_auth_root);
						apmib_get( MIB_WLAN_WSC_UPNP_ENABLED, (void *)&WSC_UPNP_Enabled);
//fprintf(stderr,"\r\n WSC_UPNP_Enabled=[%d],__[%s-%u]",WSC_UPNP_Enabled,__FILE__,__LINE__);						
						wlan_disabled_root = 0;
						wlan_network_type_root = 0;
					}
#endif					
					if(wlan_encrypt_root < 2){ //ENCRYPT_DISABLED=0, ENCRYPT_WEP=1, ENCRYPT_WPA=2, ENCRYPT_WPA2=4, ENCRYPT_WPA2_MIXED=6 ,ENCRYPT_WAPI=7
						
						if(wlan_1x_enabled_root != 0 || wlan_mac_auth_enabled_root !=0)
							_enable_1x=1;
					}else{
						if(wlan_encrypt_root != 7)	//not wapi
							_enable_1x=1;
					}
						
					if(!strcmp(token, "wlan0") && ((wlan_wsc_disabled_root != 0) || (wlan_disabled_root != 0) || (wlan_mode_root == 2))){
							WSC=0;
					}
					if(!strcmp(token, "wlan1") && ((wlan_wsc1_disabled != 0) || (wlan1_disabled_root != 0) || (wlan_mode_root == 2))){
							WSC=0;
					}
#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)					
					else if(!strcmp(token, "wlan0-vxd") && (wlan0_wsc_disabled_vxd != 0))
					{
							WSC=0;
					}
					else if(!strcmp(token, "wlan1-vxd") && (wlan_wsc_disabled_vxd != 0))
					{
							WSC=0;
					}
#endif					
					else{
							if(wlan_mode_root ==1){
								if(wlan_network_type_root != 0)
									WSC=0;
							}
							if(wlan_mode_root ==0){
								if(wlan_encrypt_root < 2 && _enable_1x !=0 )
									WSC=0;		
								if(wlan_encrypt_root >= 2 && wlan_encrypt_root != 7 && wlan_wpa_auth_root ==1 )
									WSC=0;
#ifdef CONFIG_RTL_WAPI_SUPPORT
									if(wlan_encrypt_root == 7 && wlan_wapi_auth_root == 1)
										WSC=0;
#endif
								}
						
					}
						
					if(WSC==1){ //start wscd 
						memset(cmd_opt, 0x00, 16);
						cmd_cnt=0;
						cmd_opt[cmd_cnt++] = "wscd";
						if(isFileExist("/var/wps/simplecfgservice.xml")==0){ //file is not exist
							if(isFileExist("/var/wps"))
								RunSystemCmd(NULL_FILE, "rm", "/var/wps", "-rf", NULL_STR);
							RunSystemCmd(NULL_FILE, "mkdir", "/var/wps", NULL_STR); 
							system("cp /etc/simplecfg*.xml /var/wps");
						}
						if(wlan_mode_root ==1 
#ifdef CONFIG_RTL_P2P_SUPPORT
							|| (wlan_mode_root ==8 && (p2p_mode == P2P_DEVICE || p2p_mode == P2P_CLIENT))
#endif							
							){
							WSC_UPNP_Enabled=0;
							cmd_opt[cmd_cnt++] = "-mode";
							cmd_opt[cmd_cnt++] = "2";
						}else{
							cmd_opt[cmd_cnt++] = "-start";
						}
						if(WSC_UPNP_Enabled==1){
							RunSystemCmd(NULL_FILE, "route", "del", "-net", "239.255.255.250", "netmask", "255.255.255.255", bridge_iface, NULL_STR); 
							RunSystemCmd(NULL_FILE, "route", "add", "-net", "239.255.255.250", "netmask", "255.255.255.255", bridge_iface, NULL_STR); 
						}


						if(both_band_ap == 1)
							cmd_opt[cmd_cnt++] = "-both_band_ap";
						
						sprintf(wscConfFile,"/var/wsc-%s.conf",token);
						RunSystemCmd(NULL_FILE, "flash", "upd-wsc-conf", "/etc/wscd.conf", wscConfFile, token, NULL_STR); 


						cmd_opt[cmd_cnt++] = "-c";
						#ifdef CONFIG_RTL_COMAPI_CFGFILE
						  #if !defined(CONFIG_RTL_819X)
						    #define WSC_CFG "/var/RTL8190N.dat"
						  #else
						    #define WSC_CFG "/var/RTL8192CD.dat"
						  #endif
						#else
						  #define WSC_CFG "/var/wsc.conf"
						#endif

						cmd_opt[cmd_cnt++] = wscConfFile;

						cmd_opt[cmd_cnt++] = "-w";

						cmd_opt[cmd_cnt++] = token;

						if(wps_debug==1){
							/* when you would like to open debug, you should add define in wsc.h for debug mode enable*/
							cmd_opt[cmd_cnt++] = "-debug";
						}
						if(use_iwcontrol==1){
							cmd_opt[cmd_cnt++] = "-fi";

							sprintf(wscFifoFile,"/var/wscd-%s.fifo",token);
							cmd_opt[cmd_cnt++] = wscFifoFile;

							deamon_created=1;
						}
						if(isFileExist("/var/wps_start_pbc")){
							cmd_opt[cmd_cnt++] = "-start_pbc";
							unlink("/var/wps_start_pbc");
						}
						if(isFileExist("/var/wps_start_pin")){
							cmd_opt[cmd_cnt++] = "-start";
							unlink("/var/wps_start_pin");
						}
						if(isFileExist("/var/wps_local_pin")){
							fp=fopen("/var/wps_local_pin", "r");
							if(fp != NULL){
								fscanf(fp, "%s", tmpBuff1);
								fclose(fp);
							}
							sprintf(wsc_pin_local, "%s", tmpBuff1);
							cmd_opt[cmd_cnt++] = "-local_pin";
							cmd_opt[cmd_cnt++] = wsc_pin_local;
							unlink("/var/wps_local_pin");
						}
						if(isFileExist("/var/wps_peer_pin")){
							fp=fopen("/var/wps_peer_pin", "r");
							if(fp != NULL){
								fscanf(fp, "%s", tmpBuff1);
								fclose(fp);
							}
							sprintf(wsc_pin_peer, "%s", tmpBuff1);
							cmd_opt[cmd_cnt++] = "-peer_pin";
							cmd_opt[cmd_cnt++] = wsc_pin_peer;
							unlink("/var/wps_peer_pin");
						}
						
						cmd_opt[cmd_cnt++] = "-daemon";
						
						cmd_opt[cmd_cnt++] = 0;
						//for (pid=0; pid<cmd_cnt;pid++)
							//printf("cmd index=%d, opt=%s \n", pid, cmd_opt[pid]);
						DoCmd(cmd_opt, NULL_FILE);
					}
					
					wait_fifo=5;
					do{

						if(isFileExist(wscFifoFile))
						{
							wait_fifo=0;
						}else{
							wait_fifo--;
							sleep(1);
						}
						
					}while(use_iwcontrol !=0 && wait_fifo !=0);		
				}
		
			}   
			token = strtok_r(NULL, " ", &savestr1);

		}while(token !=NULL);
	}

	if(deamon_created==1){
		if(wlan_vxd[0]){
				sprintf(tmpBuff, "iwcontrol %s %s",valid_wlan_interface, wlan_vxd);
		}else{
				sprintf(tmpBuff, "iwcontrol %s",valid_wlan_interface);
		}
		system(tmpBuff);	

	}
/*for WAPI*/
	//first, to kill daemon related wapi-cert
	//in order to avoid multiple daemon existing
#ifdef CONFIG_RTL_WAPI_SUPPORT
#ifdef CONFIG_RTL_WAPI_LOCAL_AS_SUPPORT
	RunSystemCmd(NULL_FILE, "killall", "aseUdpServer", NULL_STR); 
#endif
	RunSystemCmd(NULL_FILE, "killall", "aeUdpClient", NULL_STR);

	///////////////////////////////
	//no these operations in script
	//should sync with WLAN_INTERFACE_LIST: "wlan0,wlan0-va0,wlan0-va1,wlan0-va2,wlan0-va3"
	//At first, check virtual wlan interface
	apAsAS=0;//Initial, note: as IP only need to be set once because all wlan interfaces use the same as IP setting
	memset(wapiconf,0x0,sizeof(WAPI_ASSERVER_CONF_T)*MAX_WAPI_CONF_NUM);
	apmib_get(MIB_WLAN_BAND2G5G_SELECT,(void *)&wlanBand2G5GSelect);	
	for(wlan_index=0; wlan_index<NUM_WLAN_INTERFACE; wlan_index++)
	{
		if((wlanBand2G5GSelect!=BANDMODEBOTH)&&(wlan_index>0))
			break;
			
		sprintf(wlan_name,"wlan%d",wlan_index);
		for(i=0;i<4;i++)
		{
			memset(wlan_vname,0,sizeof(wlan_vname));
			sprintf(wlan_vname, "%s-va%d",wlan_name,i);
			if(SetWlan_idx(wlan_vname)){
				apmib_get( MIB_WLAN_ENCRYPT, (void *)&wlan_encrypt_virtual);
				apmib_get(MIB_WLAN_WAPI_AUTH, (void *)&wlan_wapi_auth);
				memset(wlan_wapi_asipaddr,0x00,sizeof(wlan_wapi_asipaddr));
				apmib_get(MIB_WLAN_WAPI_ASIPADDR,  (void*)wlan_wapi_asipaddr);
				apmib_get(MIB_WLAN_WAPI_CERT_SEL,  (void*)&wlan_wapi_cert_sel);
			}
	//		printf("%s(%d): wlan_vname(%s), wlan_encrypt_virtual(%d), wlan_wapi_auth(%d), wlan_wapi_asipaddr(%s)\n",
	//			__FUNCTION__,__LINE__,wlan_vname, wlan_encrypt_virtual,wlan_wapi_auth,inet_ntoa(*((struct in_addr *)wlan_wapi_asipaddr)));//Added for test
			if(wlan_encrypt_virtual == 7){
				if(wlan_wapi_auth == 1){
#ifdef CONFIG_RTL_WAPI_LOCAL_AS_SUPPORT
					if(!apAsAS){
						apmib_get(MIB_IP_ADDR,  (void*)tmpBuff1);
						if(!memcmp(wlan_wapi_asipaddr, tmpBuff1, 4)){
							apAsAS=1;
						}
					}
#endif
					if(asIpExist(wapiconf, (unsigned char)wlan_wapi_cert_sel,wlan_wapi_asipaddr,&index)){
						if(wapiconf[index].valid){
							strcat(wapiconf[index].network_inf,",");
							strcat(wapiconf[index].network_inf,wlan_vname);
						}
					}else {
						memcpy(wapiconf[index].wapi_asip,wlan_wapi_asipaddr,4);
						strcpy(wapiconf[index].network_inf,wlan_vname);
						wapiconf[index].wapi_cert_sel=wlan_wapi_cert_sel;
						wapiconf[index].valid=1;
					}
				}
			}
		}
		////////////////////////////////////

		//At last, check root wlan interface
		if(SetWlan_idx(wlan_name)){
			apmib_get(MIB_WLAN_WAPI_AUTH, (void *)&wlan_wapi_auth);
			memset(wlan_wapi_asipaddr,0x00,sizeof(wlan_wapi_asipaddr));
			apmib_get(MIB_WLAN_WAPI_ASIPADDR,  (void*)wlan_wapi_asipaddr);
			apmib_get(MIB_WLAN_WAPI_CERT_SEL,  (void*)&wlan_wapi_cert_sel);
		}
	//	printf("%s(%d): wlan0, wlan_encrypt_root(%d), wlan_wapi_auth(%d), wlan_wapi_asipaddr(%s)\n",
	//		__FUNCTION__,__LINE__,wlan_encrypt_root,wlan_wapi_auth,inet_ntoa(*((struct in_addr *)wlan_wapi_asipaddr)));//Added for test
		if(wlan_encrypt_root == 7){
			if(wlan_wapi_auth == 1){
#ifdef CONFIG_RTL_WAPI_LOCAL_AS_SUPPORT
				if(!apAsAS){
					apmib_get(MIB_IP_ADDR,  (void*)tmpBuff1);
					if(!memcmp(wlan_wapi_asipaddr, tmpBuff1, 4)){		
						apAsAS=1;
					}
				}
#endif
				if(asIpExist(wapiconf, (unsigned char)wlan_wapi_cert_sel,wlan_wapi_asipaddr,&index)){
					if(wapiconf[index].valid){
						strcat(wapiconf[index].network_inf,",");
						strcat(wapiconf[index].network_inf,wlan_name);
					}
				}else {
					memcpy(wapiconf[index].wapi_asip,wlan_wapi_asipaddr,4);
					strcpy(wapiconf[index].network_inf,wlan_name);
					wapiconf[index].wapi_cert_sel=wlan_wapi_cert_sel;
					wapiconf[index].valid=1;
				}
			}
		}
	}
	
#ifdef CONFIG_RTL_WAPI_LOCAL_AS_SUPPORT
	if(apAsAS){
		system("aseUdpServer &");
	}
#endif

	for(index=0;index<MAX_WAPI_CONF_NUM;index++)
	{
		if(wapiconf[index].valid){
			sprintf(arg_buff,"aeUdpClient -d %s -i %s -s %d &", inet_ntoa(*((struct in_addr *)wapiconf[index].wapi_asip)), wapiconf[index].network_inf, wapiconf[index].wapi_cert_sel);
			system(arg_buff);
		}
	}	
#endif

return 0;	
	
		
}

 
 
 
 
 
 
 
 
 
 
 
