

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "apmib.h"
#include "mibtbl.h"
#include "sysconf.h"
#include "sys_utility.h"
#include "syswan.h"

extern int SetWlan_idx(char *wlan_iface_name);

#ifdef CONFIG_DOMAIN_NAME_QUERY_SUPPORT
#if defined(LOGDEBUG_ENABLED)
#include <syslog.h> 
#endif

static int getFilter_Type(void)
{
	FILE *fp;
	char *filter_conf_file = "/proc/pocket/filter_conf";
	char ip_addr[10];
	char mac_addr[14];
	int filter_type=0;
	
	fp= fopen(filter_conf_file, "r");
	if (!fp) {
        	printf("can not open /proc/pocket/filter_conf\n");
		return -1;
   	}
	fscanf(fp,"%s %s %d",ip_addr,mac_addr, &filter_type);
	fclose(fp);
	
	return filter_type;
}

void domain_query_start_dnrd(int wlan_mode, int start_dnrd)
{
	unsigned char Ip[32], cmdBuffer[100], tmpBuff[200];
	unsigned char domanin_name[MAX_NAME_LEN]={0};
	
	apmib_get( MIB_IP_ADDR,  (void *)tmpBuff);
	sprintf(Ip, "%s", inet_ntoa(*((struct in_addr *)tmpBuff)));

	memset(domanin_name, 0x00, sizeof(domanin_name));	
	apmib_get( MIB_DOMAIN_NAME,  (void *)domanin_name);

	RunSystemCmd(NULL_FILE, "rm", "-f", "/etc/hosts", NULL_STR);
	memset(cmdBuffer, 0x00, sizeof(cmdBuffer));	
	//if(strlen(domanin_name) == 0)
	//{
		
	//	if(wlan_mode==0){//ap mode	
	//		sprintf(cmdBuffer,"%s\\%s\n", Ip, "RealTekAP");
	//	}else if(wlan_mode==1){//client mode
	//		sprintf(cmdBuffer,"%s\\%s\n", Ip, "RealTekCL");
	//	}
	//	RunSystemCmd("/etc/hosts", "echo", cmdBuffer,NULL_STR);

	//}
	if(domanin_name[0]){
		/*
		if(getFilter_Type() == 0) //1:client
		{
			sprintf(cmdBuffer,"%s\\%s\n", Ip, "AlwaysHost");
		}
		else
		*/
		{
			if(wlan_mode==0){//ap mode	
				sprintf(cmdBuffer,"%s\\%s%s%s%s\n", Ip, domanin_name, "AP.com|",domanin_name, "AP.net");
			}else if(wlan_mode==1){
				sprintf(cmdBuffer,"%s\\%s%s%s%s\n", Ip, domanin_name, "CL.com|",domanin_name, "CL.net");
			}
		}

		sprintf(tmpBuff, "echo \"%s\" > /etc/hosts", cmdBuffer);
		system(tmpBuff);
		//RunSystemCmd("/etc/hosts", "echo",cmdBuffer,NULL_STR);

		if(start_dnrd)
			RunSystemCmd(NULL_FILE, "dnrd", "--cache=off", "-s", "168.95.1.1",NULL_STR);
	}
}

void Init_Domain_Query_settings(int operation_mode, int wlan_mode, int lan_dhcp_mode, char *lan_mac)
{
	char tmpBuff1[200];
	char lan_domain_name[	MAX_NAME_LEN]={0};
	
	
	apmib_get( MIB_DOMAIN_NAME, (void *)lan_domain_name);

	if(operation_mode==1 && ((wlan_mode==1 && lan_dhcp_mode==15) || (wlan_mode==0 && lan_dhcp_mode==15)) && lan_domain_name[0]){
			system("echo 1 > /proc/pocket/en_filter"); 
			sprintf(tmpBuff1,"echo \"00000000 %s 0\" > /proc/pocket/filter_conf",lan_mac); 
			system(tmpBuff1);
		}else{
			system("echo 0 > /proc/pocket/en_filter"); 
			system("echo \"00000000 000000000000 2\" > /proc/pocket/filter_conf");
		}

}
int Check_setting(int type)
{

	//int lan_dhcp=0;
	struct in_addr inIp;
	char strIp[32], tmp_buff[32];
	int check_flag=0;
	
	//apmib_get( MIB_DHCP, (void *)&lan_dhcp);
	apmib_get(MIB_IP_ADDR, (void *)&tmp_buff);
	apmib_set(MIB_AUTO_DISCOVERY_ENABLED,(void *)&check_flag);
	
	if(type==1){//client mode currently
			//lan_dhcp = 15;
			//apmib_set( MIB_DHCP, (void *)&lan_dhcp);
			domain_query_start_dnrd(1, 0);
			#if 0
			if(strcmp(strIp, "192.168.1.252")){
				sprintf(strIp, "%s", "192.168.1.252");
				inet_aton(strIp, &inIp);
				apmib_set(MIB_IP_ADDR, (void *)&inIp);
			}	
			#endif
	}else if(type==2){//ap mode currently
			//lan_dhcp = 15;
			//apmib_set( MIB_DHCP, (void *)&lan_dhcp);
			domain_query_start_dnrd(0, 0);
			#if 0
			if(strcmp(strIp, "192.168.1.254")){
				sprintf(strIp, "%s", "192.168.1.254");
				inet_aton(strIp, &inIp);
				apmib_set(MIB_IP_ADDR, (void *)&inIp);
			}
			#endif
	}else if(type==3){//router mode currently
			//lan_dhcp = 2;
			//apmib_set( MIB_DHCP, (void *)&lan_dhcp);
			domain_query_start_dnrd(0, 0);
			#if 0
			if(strcmp(strIp, "192.168.1.254")){
				sprintf(strIp, "%s", "192.168.1.254");
				inet_aton(strIp, &inIp);
				apmib_set(MIB_IP_ADDR, (void *)&inIp);
			}
			#endif	
	}
	return 0;
}

int Check_setting_default(int opmode, int wlan_mode)
{
	int isSettingChanged=0;
	//int lan_dhcp=0;
	struct in_addr inIp;
	char strIp[32], tmp_buff[32];
	int check_flag=0;
	unsigned char domanin_name[MAX_NAME_LEN];


	//apmib_get( MIB_DHCP, (void *)&lan_dhcp);
	apmib_get(MIB_IP_ADDR, (void *)&tmp_buff);
	apmib_get(MIB_AUTO_DISCOVERY_ENABLED,(void *)&check_flag);
	
	sprintf(strIp, "%s", inet_ntoa(*((struct in_addr *)tmp_buff)));	
	if(opmode == 1 &&  wlan_mode == CLIENT_MODE && check_flag ==0){
		//lan_dhcp=15;
		//apmib_set( MIB_DHCP, (void *)&lan_dhcp);
		domain_query_start_dnrd(wlan_mode, 0);
		isSettingChanged=1;
		#if 0
		if(strcmp(strIp, "192.168.1.252")){
			sprintf(strIp, "%s", "192.168.1.252");
			inet_aton(strIp, &inIp);
			apmib_set(MIB_IP_ADDR, (void *)&inIp);
		}	
		#endif
	}else if(opmode == 1 &&  wlan_mode == AP_MODE && check_flag ==0){
		//lan_dhcp = 15;
		//apmib_set( MIB_DHCP, (void *)&lan_dhcp);
		domain_query_start_dnrd(wlan_mode, 0);
		isSettingChanged=1;
		#if 0
		if(strcmp(strIp, "192.168.1.254")){
			sprintf(strIp, "%s", "192.168.1.254");
			inet_aton(strIp, &inIp);
			apmib_set(MIB_IP_ADDR, (void *)&inIp);
		}	
		#endif
	}else if(opmode == 0 &&  check_flag ==0){
		//lan_dhcp = 15;
		//apmib_set( MIB_DHCP, (void *)&lan_dhcp);
		isSettingChanged=1;
		domain_query_start_dnrd(0, 0);
		#if 0
		if(strcmp(strIp, "192.168.1.254")){
			sprintf(strIp, "%s", "192.168.1.254");
			inet_aton(strIp, &inIp);
			apmib_set(MIB_IP_ADDR, (void *)&inIp);
		}
		#endif	
	}

	/*if domain name is empty, we set it default to "Realtek"*/
	apmib_get( MIB_DOMAIN_NAME,  (void *)domanin_name);
	if(domanin_name[0]==0){
		sprintf(domanin_name,"Realtek");
		apmib_set( MIB_DOMAIN_NAME,  (void *)domanin_name);
	}
	
	return isSettingChanged;
}


void wan_connect_pocket(char *interface, char *option)
{
	char line[128], arg_buff[200];
	char *cmd_opt[16];
	int cmd_cnt = 0 ,x, index=0;
	int dns_found=0; 
	struct in_addr addr;
	char dynip[32]={0}, mask[32]={0},remoteip[32]={0};
	char dns_server[5][32];
	char *token=NULL, *savestr1=NULL;
	struct sockaddr hwaddr;
	unsigned char *pMacAddr;
	unsigned char cmdBuffer[100], tmpBuff[64];
	unsigned char domanin_name[MAX_NAME_LEN];
	int wlan_mode_root=0;
	RunSystemCmd(NULL_FILE, "killall", "-15", "dnrd", NULL_STR);
	if(isFileExist(DNRD_PID_FILE)){
		unlink(DNRD_PID_FILE);
	}
	if(SetWlan_idx("wlan0")){
		apmib_get( MIB_WLAN_MODE, (void *)&wlan_mode_root); 
	}
	 if(strcmp(interface, "ppp0")){//do not care wan type, it is dhcp client only
		for (x=0;x<5;x++){
			memset(dns_server[x], '\0', 32);
		}
		token=NULL;
		savestr1=NULL;	     
		sprintf(arg_buff, "%s", option);
	
		token = strtok_r(arg_buff," ", &savestr1);
		index=1;
		do{
			dns_found=0;
			if (token == NULL){/*check if the first arg is NULL*/
				break;
			}else{   
				if(index==2)
					sprintf(dynip, "%s", token); /*wan ip address */
				if(index==3)
					sprintf(mask, "%s", token); /*subnet mask*/
				if(index==4)
					sprintf(remoteip, "%s", token); /*gateway ip*/			
				if(index > 4){
					for(x=0;x<5;x++){
						if(dns_server[x][0] != '\0'){
							if(!strcmp(dns_server[x], token)){
								dns_found = 1; 
								break;
							}
						}
					}
					if(dns_found ==0){
						for(x=0;x<5;x++){
							if(dns_server[x][0] == '\0'){
								sprintf(dns_server[x], "%s", token);
								break;
							}
						}
					}
				}
			}
			index++;
			token = strtok_r(NULL, " ", &savestr1);
		}while(token !=NULL);  
		
		inet_aton(dynip, &addr);//save ipaddr we got
		
		RunSystemCmd(NULL_FILE, "ifconfig", interface, dynip, "netmask", mask, NULL_STR);	
		RunSystemCmd(NULL_FILE, "route", "del", "default", "dev", interface, NULL_STR);
		
		RunSystemCmd(NULL_FILE, "route", "add", "-net", "default", "gw", remoteip, "dev", interface, NULL_STR);
		apmib_get( MIB_DOMAIN_NAME,  (void *)domanin_name);

		RunSystemCmd(NULL_FILE, "rm", "-f", "/etc/hosts", NULL_STR);
		memset(cmdBuffer, 0x00, sizeof(cmdBuffer));
		//if(strlen(domanin_name) == 0)
		//{
				
		//	if(wlan_mode_root==0){
		//	sprintf(domanin_name, "%s", "RealTekAP");
		//	}else if (wlan_mode_root==1){
		//		sprintf(domanin_name, "%s", "RealTekCL");
		//	}
		//	sprintf(cmdBuffer,"%s\\%s", dynip, domanin_name);
		//	RunSystemCmd("/etc/hosts", "echo", cmdBuffer,NULL_STR);
		//}
		//else
		if(domanin_name[0]){

			if(wlan_mode_root==0){
				sprintf(cmdBuffer,"%s\\%s%s%s%s", dynip, domanin_name, "AP.com|",domanin_name, "AP.net");
			}else if (wlan_mode_root==1){
				sprintf(cmdBuffer,"%s\\%s%s%s%s", dynip, domanin_name, "CL.com|",domanin_name, "CL.net");
			}
			RunSystemCmd(NULL_FILE, "rm", "-f", "/etc/hosts", NULL_STR);
			sprintf(tmpBuff, "echo \"%s\" > /etc/hosts", cmdBuffer);
			system(tmpBuff);
			//RunSystemCmd("/etc/hosts", "echo",cmdBuffer,NULL_STR);
		
			cmd_opt[cmd_cnt++]="dnrd";
			cmd_opt[cmd_cnt++]="--cache=off";
			for(x=0;x<5;x++){
				if(dns_server[x][0] != '\0'){
					cmd_opt[cmd_cnt++]="-s";
					cmd_opt[cmd_cnt++]=&dns_server[x][0];
					sprintf(line,"nameserver %s\n", dns_server[x]);
					if(x==0)
						write_line_to_file(RESOLV_CONF, 1, line);
					else
						write_line_to_file(RESOLV_CONF, 2, line);
				}
			}
			cmd_opt[cmd_cnt++] = 0;
			//for (x=0; x<cmd_cnt;x++)
			//	printf("cmd index=%d, opt=%s \n", x, cmd_opt[x]);
			DoCmd(cmd_opt, NULL_FILE);
		}
	}	
	/* we got ip, we should let lan pc to down/up, then it will renew ip again*/
//	system("ifconfig eth0 down");
//	system("ifconfig eth1 down");
//	sleep(2);
//	system("ifconfig eth0 up");
//	system("ifconfig eth1 up");
//	sleep(2);

	if(domanin_name[0]){
	 if(getInAddr("br0", HW_ADDR_T, (void *)&hwaddr)){ 
		pMacAddr = hwaddr.sa_data;
		sprintf(arg_buff, "echo \"%08X %02X%02X%02X%02X%02X%02X 1\" > /proc/pocket/filter_conf",addr.s_addr, pMacAddr[0], pMacAddr[1],pMacAddr[2], pMacAddr[3], pMacAddr[4], pMacAddr[5]); 
	}else{
		sprintf(arg_buff, "echo \"%08X 000000000000 1\" > /proc/pocket/filter_conf",addr.s_addr); 
	}
	
	system(arg_buff);
	
	#if defined(LOGDEBUG_ENABLED)
	if(wlan_mode_root==0){
		sprintf(arg_buff, "klogd: DNQP: AP mode, DHCP Client Connected, IP=%s\n",dynip);
		printf("DNQP: AP mode, DHCP Client Connected, IP=%s\n",dynip);
	}
	if(wlan_mode_root==1){
		sprintf(arg_buff, "klogd: DNQP: Client mode, DHCP Client Connected, IP=%s\n",dynip);
		printf("DNQP: Client mode, DHCP Client Connected, IP=%s\n",dynip);
	}
		syslog(LOG_INFO, arg_buff);
	#endif		
}
}


#endif
