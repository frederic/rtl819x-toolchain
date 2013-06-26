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
#include "apmib.h"
#include "utility.h"

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifdef HOME_GATEWAY

#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT) || defined(CONFIG_RTL_P2P_SUPPORT) || defined(CONFIG_RTL_ULINKER)

typedef enum { LAN_NETWORK=0, WAN_NETWORK } DHCPC_NETWORK_TYPE_T;
typedef enum { WAIT_IP_STATE=0, MODIFY_DNS_STATE=1} WAIT_IP_STATE_T;
#define DHCPD_CONF_FILE "/var/udhcpd.conf"
#define DHCPD_PID_FILE "/var/run/udhcpd.pid"
#define DHCPD_LEASE_FILE "/var/lib/misc/udhcpd.leases"
#define WAIT_TIME 10 
#define DHCPC_WAIT_TIME 20 
static int CurrentTime=0;
int Start_Domain_Query_Process=1;
int WLAN_State=0; 
int DHCPD_State=1; 
int DHCPC_State=1; 
int Restore_IptablesRule=0;
int check_count=0;
//int State=0;
int Confirm_Time=0;
int Confirm_DHCP_Time=0;
int wlan_iface_state=0;
int confirm_wlan_iface_state=0;
char Last_SSID[SSID_LEN+1]={0};
#if defined(CONFIG_RTL_92D_SUPPORT)
char Last_SSID_WLAN1[SSID_LEN+1]={0};
#endif
int Renew_State=0;
int Last_Connect_Reason=0;
char connect_ap_info[100] = {0};
int client_reinit = 0;
int reconnect = 0;
#if defined(LOGDEBUG_ENABLED)
#include <syslog.h> 
#endif
#if defined(CONFIG_RTL_92D_SUPPORT)
int isBandModeBoth()
{
	int val;
	apmib_get(MIB_WLAN_BAND2G5G_SELECT, (void *)&val);
	if(val == BANDMODEBOTH)
		return 1;
	else
		return 0;
}
#endif


int getFilter_Type(void)
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
#if 0 // move to src/utility.c
int write_line_to_file(char *filename, int mode, char *line_data)
{
	unsigned char tmpbuf[512];
	int fh=0;

	if(mode == 1) {/* write line datato file */
		
		fh = open(filename, O_RDWR|O_CREAT|O_TRUNC);
		
	}else if(mode == 2){/*append line data to file*/
		
		fh = open(filename, O_RDWR|O_APPEND);	
	}
	
	
	if (fh < 0) {
		fprintf(stderr, "Create %s error!\n", filename);
		return 0;
	}


	sprintf(tmpbuf, "%s", line_data);
	write(fh, tmpbuf, strlen(tmpbuf));



	close(fh);
	return 1;
}
#endif
int Check_Wlan_isConnected(int mode)
{
	FILE *stream;
	int result=0;
	int LinkType=0;
	bss_info bss;
	static int check_time = 0;
	int wlan0Active=0;
	#if defined(CONFIG_RTL_92D_SUPPORT)
	int wlan1Active=0;
	#endif
	if(mode==1){ //client mode
		check_time ++;
		if(check_time > 1000)
			check_time = 40;
		stream = fopen ( "/proc/wlan0/sta_info", "r" );
		if ( stream != NULL ) {		
			char *strtmp;
			char line[100];
			while (fgets(line, sizeof(line), stream))
			{
				unsigned char *p;
				strtmp = line;
				while(*strtmp == ' ')
					strtmp++;
					
				if(strstr(strtmp,"active") != 0){
					unsigned char str1[10];
							
					//-- STA info table -- (active: 1)
					sscanf(strtmp, "%*[^:]:%[^)]",str1);
							
					p = str1;
					while(*p == ' ')
						p++;										
					if(strcmp(p,"0") == 0){
						result=0;
						wlan0Active=0;
					}else{
						result=1;		
						wlan0Active=1;
					}										
					//break;
				}
				if(strstr(strtmp,"hwaddr") != 0){
					
					client_reinit = 0;
					if( memcmp(line, connect_ap_info, strlen(line) ) != 0)
					{
						memcpy(connect_ap_info, line, strlen(line));
						if(check_time >= 40)
						{
							client_reinit = 1;
						}
						
					}
					break;
				}
						
			}
			fclose(stream );
					
		}

		#if defined(CONFIG_RTL_92D_SUPPORT)
		/*wlan0 is not active, check wlan1*/
		if(wlan0Active==0)
		{
			stream = fopen ( "/proc/wlan1/sta_info", "r" );
			if ( stream != NULL ) {		
				char *strtmp;
				char line[100];
				while (fgets(line, sizeof(line), stream))
				{
					unsigned char *p;
					strtmp = line;
					while(*strtmp == ' ')
						strtmp++;
						
					if(strstr(strtmp,"active") != 0){
						unsigned char str1[10];
								
						//-- STA info table -- (active: 1)
						sscanf(strtmp, "%*[^:]:%[^)]",str1);
								
						p = str1;
						while(*p == ' ')
							p++;										
						if(strcmp(p,"0") == 0){
							result=0;
							wlan1Active=0;
						}else{
							result=1;	
							wlan1Active=1;
						}										
						//break;
					}
					if(strstr(strtmp,"hwaddr") != 0){
						
						client_reinit = 0;
						if( memcmp(line, connect_ap_info, strlen(line) ) != 0)
						{
							memcpy(connect_ap_info, line, strlen(line));
							if(check_time >= 40)
							{
								client_reinit = 1;
							}
							
						}
						break;
					}
							
				}
				fclose(stream );
			}
		}
		#endif		
		
	}else{
		//ap mode , we check ethernet port phy link status
		if(getWanLink("eth1") < 0){
			//printf("ethernet is disconnect\n");
			result=0;
		}else
			result=1;
	}
	
	if(mode==1){//client mode
		if(result==1){	//wlan interface is active
			if(wlan0Active==1)
			{
				 getWlBssInfo("wlan0", &bss);
				 if(bss.state != STATE_CONNECTED){
				 	result=0;
					client_reinit = 0;
				}
				else if( (client_reinit == 1) || ((reconnect == 1)&&( bss.state == STATE_CONNECTED)) )
				{
					WLAN_State = 0;
					reconnect = 0;
					CurrentTime = 0;
					//system("reboot");
				}
			}
			
			#if defined(CONFIG_RTL_92D_SUPPORT)
			/*wlan0 is not active, check wlan1 link state*/
			if((wlan0Active==0) && (wlan1Active==1))
			{
				 getWlBssInfo("wlan1", &bss);
				 if(bss.state != STATE_CONNECTED){
				 	result=0;
					client_reinit = 0;
				}
				else if( (client_reinit == 1) || ((reconnect == 1)&&( bss.state == STATE_CONNECTED)) )
				{
					WLAN_State = 0;
					reconnect = 0;
					CurrentTime = 0;
					//system("reboot");
				}
			}
			#endif
		}
		if(result == 0)
			reconnect = 1;
	}
	 if(mode==1 && CurrentTime >= WAIT_TIME){
	 	if(isFileExist("/etc/udhcpc/udhcpc-br0.pid")){
	 		LinkType = getFilter_Type();
	 		
			if(LinkType==0 && Confirm_DHCP_Time >= 5){
				Last_Connect_Reason=1; ///wlan can not got ip address, we modify phy link status, we treat it as phy link down
				#if defined(LOGDEBUG_ENABLED)
					syslog(LOG_INFO, "klogd: DNQP: DHCP Client started, but CanNot get Ip, treat link down\n");
					printf("DNQP: DHCP Client started, but CanNOT get Ip, treat link down\n");
				#endif
				
			}else if(LinkType==1){
				Last_Connect_Reason=0;///wlan can got ip address, we do not modify wlan phy link status 
				#if defined(LOGDEBUG_ENABLED)
					syslog(LOG_INFO, "klogd: DNQP: DHCP Client started, and got Ip, DONT Modify phy link status\n");
					printf("DNQP: DHCP Client started, and got Ip, DONT Modify phy link status\n");
				#endif
			}
	 	}
	 	if(Last_Connect_Reason==1){
	 		#if defined(LOGDEBUG_ENABLED)
					syslog(LOG_INFO, "klogd: DNQP: LastState CanNot get Ip, treat link down\n");
					printf("DNQP: LastState CanNOT get Ip, treat link down\n");
			#endif
	 		result=0;
	 	}
	}	
	return result;
}
void Create_script(char *script_path, char *iface, int network, char *ipaddr, char *mask, char *gateway)
{
	
	unsigned char tmpbuf[100];
	int fh;
	
	fh = open(script_path, O_RDWR|O_CREAT|O_TRUNC, S_IRWXO|S_IRWXG);	
	if (fh < 0) {
		fprintf(stderr, "Create %s file error!\n", script_path);
		return;
	}
	if(network==LAN_NETWORK){
		sprintf(tmpbuf, "%s", "#!/bin/sh\n");
		write(fh, tmpbuf, strlen(tmpbuf));
		//sprintf(tmpbuf, "%s\n", "echo \"br0 defconfig\"");
		//write(fh, tmpbuf, strlen(tmpbuf));
		//sprintf(tmpbuf, "ifconfig %s %s netmask %s 2> /dev/null\n", iface, ipaddr, mask);
		//write(fh, tmpbuf, strlen(tmpbuf));
		//sprintf(tmpbuf, "while route del default dev %s 2> /dev/null\n", iface);
		//write(fh, tmpbuf, strlen(tmpbuf));
		//sprintf(tmpbuf, "%s\n", "do :");
		//write(fh, tmpbuf, strlen(tmpbuf));
		//sprintf(tmpbuf, "%s\n", "done");
		//write(fh, tmpbuf, strlen(tmpbuf));
		//sprintf(tmpbuf, "route add -net default gw %s dev %s 2> /dev/null\n", gateway, iface);
	//	write(fh, tmpbuf, strlen(tmpbuf));
	//	sprintf(tmpbuf, "%s\n", "sysconf dhcpc deconfig br0");
	//	write(fh, tmpbuf, strlen(tmpbuf));
	}
	close(fh);
}

void set_lan_dhcpc(char *iface)
{
	char script_file[100], deconfig_script[100], pid_file[100];
	char *strtmp=NULL;
	char tmp[32], Ip[32], Mask[32], Gateway[32];
	char cmdBuff[200];
	unsigned char host_name[MAX_NAME_LEN]={0};

	sprintf(script_file, "/usr/share/udhcpc/%s.sh", iface); /*script path*/
	sprintf(deconfig_script, "/usr/share/udhcpc/%s.deconfig", iface);/*deconfig script path*/
	sprintf(pid_file, "/etc/udhcpc/udhcpc-%s.pid", iface); /*pid path*/
	apmib_get( MIB_IP_ADDR,  (void *)tmp);
	strtmp= inet_ntoa(*((struct in_addr *)tmp));
	sprintf(Ip, "%s",strtmp);
	
	apmib_get( MIB_SUBNET_MASK,  (void *)tmp);
	strtmp= inet_ntoa(*((struct in_addr *)tmp));
	sprintf(Mask, "%s",strtmp);
	
	apmib_get( MIB_DEFAULT_GATEWAY,  (void *)tmp);
	strtmp= inet_ntoa(*((struct in_addr *)tmp));
	sprintf(Gateway, "%s",strtmp);
	 
		
	Create_script(deconfig_script, iface, LAN_NETWORK, Ip, Mask, Gateway);
	apmib_get( MIB_HOST_NAME,  (void *)host_name);
	if(host_name[0])
		sprintf(cmdBuff, "udhcpc -i %s -p %s -s %s -h %s &", iface, pid_file, script_file, host_name);
	else
		sprintf(cmdBuff, "udhcpc -i %s -p %s -s %s &", iface, pid_file, script_file);
		
	system(cmdBuff);
}
static void start_dnrd()
{
	unsigned char Ip[32], cmdBuffer[100], tmpBuff[200];
	unsigned char domanin_name[MAX_NAME_LEN];
	int wlan_mode=0;
	
	apmib_get( MIB_WLAN_MODE, (void *)&wlan_mode);

	system("killall -9 dnrd 2> /dev/null");
	apmib_get( MIB_IP_ADDR,  (void *)tmpBuff);
	sprintf(Ip, "%s", inet_ntoa(*((struct in_addr *)tmpBuff)));

	apmib_get( MIB_DOMAIN_NAME,  (void *)domanin_name);
	
	system("rm -f /var/hosts 2> /dev/null");
	memset(cmdBuffer, 0x00, sizeof(cmdBuffer));
	if(strlen(domanin_name) == 0)
	{
		/*
		if(getFilter_Type() == 0) //1:client
		{
			sprintf(cmdBuffer,"%s\\%s\n", Ip, "AlwaysHost");
		}
		else
		*/
		{
			if(wlan_mode==0){//ap mode	
				sprintf(cmdBuffer,"%s\\%s\n", Ip, "RealTekAP.com|RealTekAP.net");
			}else if(wlan_mode==1){//client mode
				sprintf(cmdBuffer,"%s\\%s\n", Ip, "RealTekCL.com|RealTekCL.net");
			}
		}
		write_line_to_file("/etc/hosts", 1, cmdBuffer);

	}
	else
	{
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
		write_line_to_file("/etc/hosts", 1, cmdBuffer);
		
	}
	system("dnrd --cache=off -s 168.95.1.1");

}

int check_upStream_connected(void)
{
	FILE *stream;
	int result=0;
	bss_info bss;
	int wlan0Active=0;
	int wlan0Connected=0;
	#if defined(CONFIG_RTL_92D_SUPPORT)
	int wlan1Active=0;
	int wlan1Connected=0;
	#endif
	int wlanMode=0;
	apmib_get( MIB_WLAN_MODE, (void *)&wlanMode);
	
	if(wlanMode==1){ //client mode
		stream = fopen ( "/proc/wlan0/sta_info", "r" );
		if ( stream != NULL ) {		
			char *strtmp;
			char line[100];
			while (fgets(line, sizeof(line), stream))
			{
				unsigned char *p;
				strtmp = line;
				while(*strtmp == ' ')
					strtmp++;
					
				if(strstr(strtmp,"active") != 0){
					unsigned char str1[10];
							
					//-- STA info table -- (active: 1)
					sscanf(strtmp, "%*[^:]:%[^)]",str1);
							
					p = str1;
					while(*p == ' ')
						p++;										
					if(strcmp(p,"0") == 0){
						wlan0Active=0;
					}else{
						wlan0Active=1;
					}										
					//break;
				}
				if(strstr(strtmp,"hwaddr") != 0){
					
					break;
				}
						
			}
			fclose(stream );
					
		}
		
		if(wlan0Active==1)
		{
			 getWlBssInfo("wlan0", &bss);
			 if(bss.state == STATE_CONNECTED){
			 	wlan0Connected=1;
			}
			 else
			 {
				wlan0Connected=0;
			 }
		}
		
		#if defined(CONFIG_RTL_92D_SUPPORT)
		/*wlan0 is not active, check wlan1*/
		stream = fopen ( "/proc/wlan1/sta_info", "r" );
		if ( stream != NULL ) {		
			char *strtmp;
			char line[100];
			while (fgets(line, sizeof(line), stream))
			{
				unsigned char *p;
				strtmp = line;
				while(*strtmp == ' ')
					strtmp++;
					
				if(strstr(strtmp,"active") != 0){
					unsigned char str1[10];
							
					//-- STA info table -- (active: 1)
					sscanf(strtmp, "%*[^:]:%[^)]",str1);
							
					p = str1;
					while(*p == ' ')
						p++;										
					if(strcmp(p,"0") == 0){
						wlan1Active=0;
					}else{
						wlan1Active=1;
					}										
					//break;
				}
				if(strstr(strtmp,"hwaddr") != 0){
					
					break;
				}
						
			}
			fclose(stream );
		}

		/*wlan0 is not active, check wlan1 link state*/
		if( wlan1Active==1)
		{
			 getWlBssInfo("wlan1", &bss);
			 if(bss.state == STATE_CONNECTED){
			 	wlan1Connected=1;
			}
			 else
			 {
				wlan1Connected=0;
			 }
		}
		#endif
	
		if(wlan0Connected)
		{
			result=1;
		}

		#if defined(CONFIG_RTL_92D_SUPPORT)
		if(wlan1Connected)
		{
			result=1;
		}
		#endif
	}
	else
	{
		//ap mode , we check ethernet port phy link status
		if(getWanLink("eth1") < 0){
			//printf("ethernet is disconnect\n");
			result=0;
		}else
			result=1;
	}
	
	
	return result;
}

// extern for P2P_SUPPORT
void set_lan_dhcpd(char *interface, int mode)
{
	char tmpBuff1[32]={0}, tmpBuff2[32]={0};
	int intValue=0, dns_mode=0;
	char line_buffer[100]={0};
	char tmp1[64]={0};
	char tmp2[64]={0};
	int opMode=-1;
	int dhcpMode=-1;
	struct in_addr lanIpAddr;
	char *strtmp=NULL, *strtmp1=NULL;
	//DHCPRSVDIP_T entry;
	//int i, entry_Num=0;
#ifdef   HOME_GATEWAY
	char tmpBuff3[32]={0};
#endif

	sprintf(line_buffer,"interface %s\n",interface);
	write_line_to_file(DHCPD_CONF_FILE, 1, line_buffer);
	
	apmib_get(MIB_DHCP_CLIENT_START,  (void *)tmp1);
	strtmp= inet_ntoa(*((struct in_addr *)tmp1));
	sprintf(line_buffer,"start %s\n",strtmp);
	write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);
	
	apmib_get(MIB_DHCP_CLIENT_END,  (void *)tmp1);
	strtmp= inet_ntoa(*((struct in_addr *)tmp1));
	sprintf(line_buffer,"end %s\n",strtmp);
	write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);

	apmib_get(MIB_SUBNET_MASK,  (void *)tmp1);
	strtmp= inet_ntoa(*((struct in_addr *)tmp1));
	sprintf(line_buffer,"opt subnet %s\n",strtmp);
	write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);
	
	apmib_get( MIB_OP_MODE,&opMode);
	apmib_get( MIB_DHCP,&dhcpMode);
	if((opMode==1) && (dhcpMode==15))
	{
		if(check_upStream_connected())
		{
			getInAddr("br0", IP_ADDR, (void *)&tmp2);
		}
		else
		{
			apmib_get( MIB_IP_ADDR,  (void *)tmp2);
			if (memcmp(tmp2, "\x0\x0\x0\x0", 4)){
				strtmp= inet_ntoa(*((struct in_addr *)tmp2));
				sprintf(line_buffer,"opt router %s\n",strtmp);
				write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);
			}
		}
		
		if (memcmp(tmp2, "\x0\x0\x0\x0", 4)){
			strtmp= inet_ntoa(*((struct in_addr *)tmp2));
			sprintf(line_buffer,"opt dns %s\n",strtmp);
			write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);
		}	
	}
	else
	{
		if(mode==1){//ap
			apmib_get( MIB_DEFAULT_GATEWAY,  (void *)tmp2);
			if (memcmp(tmp2, "\x0\x0\x0\x0", 4)){
				strtmp= inet_ntoa(*((struct in_addr *)tmp2));
				sprintf(line_buffer,"opt router %s\n",strtmp);
				write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);
			}
			
			
		}else{
			apmib_get(MIB_IP_ADDR,  (void *)tmp1);
			strtmp= inet_ntoa(*((struct in_addr *)tmp1));
			sprintf(line_buffer,"opt router %s\n",strtmp);
			write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);
#ifdef   HOME_GATEWAY		
			apmib_get( MIB_DNS_MODE, (void *)&dns_mode);
			if(dns_mode==0){
				sprintf(line_buffer,"opt dns %s\n",strtmp); /*now strtmp is ip address value */
				write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);
			}
#endif
		}	

		if((mode==1) 
#if 1
		||(mode==2 && dns_mode==1)
#endif
		){
			if(intValue==0){ /*no dns option for dhcp server, use default gatewayfor dns opt*/
				
				if(mode==1){
					apmib_get( MIB_DEFAULT_GATEWAY,  (void *)tmp2);
					if (memcmp(tmp2, "\x0\x0\x0\x0", 4)){
						strtmp= inet_ntoa(*((struct in_addr *)tmp2));
						sprintf(line_buffer,"opt dns %s\n",strtmp);
						write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);
					}
				}else {
					apmib_get( MIB_IP_ADDR,  (void *)tmp2);
					if (memcmp(tmp2, "\x0\x0\x0\x0", 4)){
						strtmp= inet_ntoa(*((struct in_addr *)tmp2));
						sprintf(line_buffer,"opt dns %s\n",strtmp);
						write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);
					}
				}
			}
		}
	}
	memset(tmp1, 0x00, 64);
	apmib_get( MIB_DOMAIN_NAME, (void *)&tmp1);
	if(tmp1[0]){
		sprintf(line_buffer,"opt domain %s\n",tmp1);
		write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);
	}

	/* may not need to set ip again*/
	apmib_get(MIB_IP_ADDR,  (void *)tmp1);
	strtmp= inet_ntoa(*((struct in_addr *)tmp1));
	sprintf(tmpBuff1, "%s", strtmp);
	apmib_get(MIB_SUBNET_MASK,  (void *)tmp2);
	strtmp1= inet_ntoa(*((struct in_addr *)tmp2));
	sprintf(tmpBuff2, "%s", strtmp1);
	
	sprintf(line_buffer, "ifconfig %s %s netmask %s", interface, tmpBuff1, tmpBuff2);
	system(line_buffer);
	
	sprintf(line_buffer, "udhcpd %s", DHCPD_CONF_FILE);
	system(line_buffer);
	//start_dnrd();
}

int getLan_MacAddress(unsigned char *dst)
{
	struct sockaddr hwaddr;
	unsigned char *pMacAddr;
	 if(getInAddr("br0", HW_ADDR, (void *)&hwaddr)){ 
		pMacAddr = hwaddr.sa_data;
		sprintf(dst, "%02X%02X%02X%02X%02X%02X",pMacAddr[0], pMacAddr[1], pMacAddr[2], pMacAddr[3],pMacAddr[4],pMacAddr[5]); 
		return 1;
	}
	return 0;
}
extern void translate_control_code(char *buffer);
int check_ssid()
{
	char buffer[128];
	bss_info bss;
	int ret=0;
	memset(&bss,0,sizeof(bss_info));
	memset(&buffer,0,sizeof(buffer));
	if(getWlBssInfo("wlan0", &bss) == 0)
	{
		memcpy(buffer, bss.ssid, SSID_LEN+1);
		translate_control_code(buffer);
		if(Last_SSID[0]){
		if(memcmp(Last_SSID, buffer,SSID_LEN+1)){
			memcpy(Last_SSID, buffer, SSID_LEN+1);
			ret =1;
		}
		}else{
		if(buffer[0])
			memcpy(Last_SSID, buffer, SSID_LEN+1);
		else
			memset(Last_SSID, 0x00, SSID_LEN+1);
		}
	}
	#if defined(CONFIG_RTL_92D_SUPPORT)
	memset(&bss,0,sizeof(bss_info));
	memset(&buffer,0,sizeof(buffer));
	if(getWlBssInfo("wlan1", &bss) == 0)
	{
		memcpy(buffer, bss.ssid, SSID_LEN+1);
		translate_control_code(buffer);
		if(Last_SSID_WLAN1[0]){
			if(memcmp(Last_SSID_WLAN1, buffer,SSID_LEN+1)){
				memcpy(Last_SSID_WLAN1, buffer, SSID_LEN+1);
				ret =1;
			}
		}else{
			if(buffer[0])
				memcpy(Last_SSID_WLAN1, buffer, SSID_LEN+1);
			else
				memset(Last_SSID_WLAN1, 0x00, SSID_LEN+1);
		}
	}
	#endif
	return ret;
}

void Kill_Wlan_Applications(void)
{
	system("killall -9 wscd 2> /dev/null");
	system("killall -9 iwcontrol 2> /dev/null");
	system("killall -9 auth 2> /dev/null");
	system("killall -9 disc_server 2> /dev/null");
	system("killall -9 iapp 2> /dev/null");	
	system("killall -9 mini_upnpd 2> /dev/null");
}

void Start_Wlan_Applications(void)
{

	#if defined (CONFIG_RTL_92D_SUPPORT)
	if(isBandModeBoth())
		system("sysconf wlanapp start wlan0 wlan1 br0");
	else
		system("sysconf wlanapp start wlan0 br0");
	#else
	system("sysconf wlanapp start wlan0 br0");
	#endif
	sleep(1);
	/*sysconf upnpd 1(isgateway) 1(opmode is bridge)*/
	system("sysconf upnpd 1 1");
	sleep(1);
}




int Domain_query_Process()
{

	int i;
	int Operation_Mode=0;
	int WLAN_Mode=0;
	unsigned char LAN_Mac[12];
	char cmdBuffer[100];
	int lan_dhcp=0;
	char lan_domain_name[	MAX_NAME_LEN]={0};
	unsigned char Confirm_Threshold=0;
	int Type=0;
	int Check_status=0;
	
	apmib_get( MIB_DOMAIN_NAME, (void *)lan_domain_name);
	
	if(Start_Domain_Query_Process==0 ||isFileExist("/var/system/start_init") || !lan_domain_name[0]){//during init procedure 
		//printf("web init return directly\n");
		return 0;
	}
	apmib_get( MIB_OP_MODE, (void *)&Operation_Mode);
	apmib_get( MIB_WLAN_MODE, (void *)&WLAN_Mode);
	apmib_get( MIB_DHCP, (void *)&lan_dhcp);
	if(Operation_Mode==1 && ((WLAN_Mode == 1 && lan_dhcp==15)|| (WLAN_Mode == 0 && lan_dhcp==15))){ //in bridge mode and wlan ap/client mode 
		if(CurrentTime < WAIT_TIME){ //it time to check wlan connected or not
			/*if wlan is connected will not start dhcp server*/
			Check_status = Check_Wlan_isConnected(WLAN_Mode);
			if( Check_status==1 && DHCPD_State==1){ //wlan connetced
				//printf("wlan connected within WAIT_TIME and start dhcpc\n");
				system("killall -9 udhcpd 2> /dev/null");
				system("killall -9 udhcpc 2> /dev/null");
				system("echo 1 > /proc/pocket/en_filter"); //start to filter dhcp discover in bridge 
				if(getLan_MacAddress(LAN_Mac)){
					sprintf(cmdBuffer, "echo \"00000000 %s 0\" > /proc/pocket/filter_conf", LAN_Mac);
					system(cmdBuffer);
				}else{
					system("echo \"00000000 000000000000 0\" > /proc/pocket/filter_conf");
				}
				system("rm -f /var/run/udhcpd.pid 2> /dev/null");
				system("rm -f /etc/udhcpc/udhcpc-br0.pid 2> /dev/null");
				
				#if defined(LOGDEBUG_ENABLED)
				if(WLAN_Mode == 1){
					syslog(LOG_INFO, "klogd: DNQP: Connected within WAIT_TIME, Start DHCP Client\n");
					printf("DNQP: Connected within WAIT_TIME, Start DHCP Client\n");
				}
				#endif		
				set_lan_dhcpc("br0");
				DHCPC_State=2; // start DHCP client
				DHCPD_State=0;

				WLAN_State=1; 
				if(WLAN_Mode == 0){
				#if defined(LOGDEBUG_ENABLED)
						syslog(LOG_INFO, "klogd: DNQP: AP mode, within WAIT_TIME, Start DHCP Client, shutdown wlan first\n");
						printf("DNQP: AP mode, within WAIT_TIME, Start DHCP Client, shutdown wlan first\n");
				#endif	
					system("ifconfig wlan0 down");
					#if defined (CONFIG_RTL_92D_SUPPORT)
					system("ifconfig wlan1 down");
					#endif
					Kill_Wlan_Applications();
				}
				wlan_iface_state=1;
				
			}else if(Check_status ==0 && DHCPD_State==1){
	
				if(isFileExist(DHCPD_PID_FILE)==0){
		
					system("killall -9 udhcpd 2> /dev/null");
					system("rm -f /var/run/udhcpd.pid 2> /dev/null");
					system("echo 1 > /proc/pocket/en_filter"); //start to filter dhcp discover in bridge 
					if(getLan_MacAddress(LAN_Mac)){
						sprintf(cmdBuffer, "echo \"00000000 %s 2\" > /proc/pocket/filter_conf", LAN_Mac);
						system(cmdBuffer);
					}else{
						system("echo \"00000000 000000000000 2\" > /proc/pocket/filter_conf");
					}
					#if defined(LOGDEBUG_ENABLED)
					if(WLAN_Mode == 1){
						syslog(LOG_INFO, "klogd: DNQP: Disconnected within WAIT_TIME, Start DHCP Server\n");
						printf("DNQP: Disconnected within WAIT_TIME, Start DHCP Server\n");
					}
					#endif	
					set_lan_dhcpd("br0", 2);
					
					for(i=0;i<3;i++){
						 if(isFileExist(DHCPD_PID_FILE)){
						 	break;
						 }else{
						 	sleep(1);
						}
					}
					if(WLAN_Mode == 1){
					system("ifconfig eth0 down");
					//system("ifconfig eth1 down");
					sleep(5);
					system("ifconfig eth0 up");
					//system("ifconfig eth1 up");
					}
					else if(WLAN_Mode == 0)
					{
						#if defined(LOGDEBUG_ENABLED)
							syslog(LOG_INFO, "klogd: DNQP: AP mode, within WAIT_TIME, Start DHCP Server, shutdown wlan first\n");
							printf("DNQP: AP mode, within WAIT_TIME, Start DHCP Server, shutdown wlan first\n");
						#endif	
						system("ifconfig wlan0 down");
						#if defined (CONFIG_RTL_92D_SUPPORT)
						system("ifconfig wlan1 down");
						#endif
						Kill_Wlan_Applications();
					}
					start_dnrd();
					wlan_iface_state=1;
					system("iptables -F INPUT");
					WLAN_State=0;					
				}
			}
	
		}else if(CurrentTime >= WAIT_TIME){ //it's time to check wlan connect or not
			   Check_status = Check_Wlan_isConnected(WLAN_Mode);
			if(Check_status ==0){ //wlan/eth1  is not connetced

				//disconnect state
				if(isFileExist(DHCPD_PID_FILE)==0){ //dhcp server is not running

					if(WLAN_Mode==0)
						Confirm_Threshold=0;
					if(WLAN_Mode==1)
						Confirm_Threshold=20;
					if(WLAN_State==1 && Confirm_Time < Confirm_Threshold){
						Confirm_Time++;
						goto ToNext;
					}
					Confirm_Time=0;
					if(isFileExist("/etc/udhcpc/udhcpc-br0.pid")){
						system("killall -9 udhcpc 2> /dev/null");
						system("rm -f /etc/udhcpc/udhcpc-br0.pid 2> /dev/null");	
					}
					sleep(1);
					system("killall -9 udhcpd 2> /dev/null");
					system("rm -f /var/run/udhcpd.pid 2> /dev/null");
					#if defined(LOGDEBUG_ENABLED)
					if(WLAN_Mode == 1){
						syslog(LOG_INFO, "klogd: DNQP: Disconnected after WAIT_TIME, Start DHCP Server\n");
						printf("DNQP: Disconnected after WAIT_TIME, Start DHCP Server\n");
					}
					#endif
					set_lan_dhcpd("br0", 2);
					
					for(i=0;i<3;i++){
						 if(isFileExist(DHCPD_PID_FILE)){
						 	break;
						 }else{
						 	sleep(1);
						}
					}

					system("echo 1 > /proc/pocket/en_filter"); //start to filter dhcp discover in bridge 
					if(getLan_MacAddress(LAN_Mac)){
						sprintf(cmdBuffer, "echo \"00000000 %s 2\" > /proc/pocket/filter_conf", LAN_Mac);
						system(cmdBuffer);
					}else{
						system("echo \"00000000 000000000000 2\" > /proc/pocket/filter_conf");
					}
					if(WLAN_Mode == 1){
					system("ifconfig eth0 down");
					//system("ifconfig eth1 down");
					sleep(5);
					system("ifconfig eth0 up");
					//system("ifconfig eth1 up");
					}
					else if(WLAN_Mode == 0)
					{
						#if defined(LOGDEBUG_ENABLED)
							syslog(LOG_INFO, "klogd: DNQP: AP mode, after WAIT_TIME, Start DHCP Server, shutdown wlan first\n");
							printf("DNQP: AP mode, after WAIT_TIME, Start DHCP Server, shutdown wlan first\n");
						#endif
						system("ifconfig wlan0 down");
						#if defined (CONFIG_RTL_92D_SUPPORT)
						system("ifconfig wlan1 down");
						#endif
						Kill_Wlan_Applications();
					}
					start_dnrd();
					wlan_iface_state=1;	
					system("iptables -F INPUT");
					if(WLAN_Mode == 0){
					WLAN_State=0;					
					}					
				}else{
					//dhcp server is running, and not connected
						if(isFileExist("/etc/udhcpc/udhcpc-br0.pid")){
							system("killall -9 udhcpc 2> /dev/null");
							system("rm -f /etc/udhcpc/udhcpc-br0.pid 2> /dev/null");							
							if(getLan_MacAddress(LAN_Mac)){
								sprintf(cmdBuffer, "echo \"00000000 %s 2\" > /proc/pocket/filter_conf", LAN_Mac);
								system(cmdBuffer);
							}else{
								system("echo \"00000000 000000000000 2\" > /proc/pocket/filter_conf");
							}
						}else{
							Type = getFilter_Type();
							if(Type==0){
								#if defined(LOGDEBUG_ENABLED)
									syslog(LOG_INFO, "klogd: DNQP: Disconnected after WAIT_TIME, Filter state is 0, reset to 2 in DHCP server state\n");
									printf("DNQP: AP mode, Disconnected after WAIT_TIME, Filter state is 0, reset to 2 in DHCP server state\n");
								#endif
								system("echo \"00000000 000000000000 2\" > /proc/pocket/filter_conf");
							}else if(Type==2 && WLAN_State==1){
								Renew_State++;
							}
							if(WLAN_Mode == 0 && Renew_State >= 1 && WLAN_State==1){
									WLAN_State=0;
									Renew_State=0;
								#if defined(LOGDEBUG_ENABLED)
									syslog(LOG_INFO, "klogd: DNQP: AP mode, Disconnected after WAIT_TIME, LastState is Connected, clean WLAN_State to cause start DHCP client when Connect again\n");
									printf("DNQP: AP mode, Disconnected after WAIT_TIME, LastState is Connected, clean WLAN_State to cause start DHCP client when Connect again\n");
								#endif
							}							
						}
				}
			}else{
                               //connected state
				if(WLAN_State ==0){
					system("killall -9 udhcpd 2> /dev/null");
					system("killall -9 udhcpc 2> /dev/null");
					system("rm -f /var/run/udhcpd.pid 2> /dev/null");
					system("rm -f /etc/udhcpc/udhcpc-br0.pid 2> /dev/null");
					system("echo 1 > /proc/pocket/en_filter"); //start to filter dhcp discover in bridge 
					if(getLan_MacAddress(LAN_Mac)){
						sprintf(cmdBuffer, "echo \"00000000 %s 0\" > /proc/pocket/filter_conf", LAN_Mac);
						system(cmdBuffer);
					}else{
						system("echo \"00000000 000000000000 0\" > /proc/pocket/filter_conf");
					}
					
					#if defined(LOGDEBUG_ENABLED)
					if(WLAN_Mode == 1){
						syslog(LOG_INFO, "klogd: DNQP: Connected after WAIT_TIME, Start DHCP Client\n");
						printf("DNQP: Connected after WAIT_TIME, Start DHCP Client\n");
					}
					#endif	
					set_lan_dhcpc("br0");
					DHCPC_State=2; // start DHCP client					
					
					
					system("iptables -A INPUT -p icmp --icmp-type echo-request -i br0 -j DROP");
//					sleep(1);
					WLAN_State=1;
					Restore_IptablesRule=1;
					check_count=0;
					if(WLAN_Mode == 0){
						#if defined(LOGDEBUG_ENABLED)
							syslog(LOG_INFO, "klogd: DNQP: AP mode, after WAIT_TIME, Start DHCP Client, shutdown wlan first\n");
							printf("DNQP: AP mode, after WAIT_TIME, Start DHCP Client, shutdown wlan first\n");
						#endif
						system("ifconfig wlan0 down");
						#if defined (CONFIG_RTL_92D_SUPPORT)
						system("ifconfig wlan1 down");
						#endif

						Kill_Wlan_Applications();
					}
						wlan_iface_state=1;
				}else{
					if(Restore_IptablesRule==1){
						if(isFileExist(DHCPD_PID_FILE)){
							unlink(DHCPD_PID_FILE);
						}
						if(check_count > 5){
							system("iptables -F INPUT");
							Restore_IptablesRule=0;
						}else{
							check_count++;
						}
					}
					
					if(getFilter_Type()==0){
						if(Confirm_DHCP_Time >= DHCPC_WAIT_TIME && (DHCPC_State==1 || DHCPC_State==2)){
							system("killall -9 udhcpd 2> /dev/null");
							system("killall -9 udhcpc 2> /dev/null");
							sleep(1);
							system("rm -f /var/run/udhcpd.pid 2> /dev/null");
							system("rm -f /etc/udhcpc/udhcpc-br0.pid 2> /dev/null");
							
							
						#if defined(LOGDEBUG_ENABLED)
						if(WLAN_Mode == 1){
							syslog(LOG_INFO, "klogd: DNQP: Connected after WAIT_TIME,  DHCP Client start, and CanNOT get ip after 20 seconds, Start DHCP Server\n");
							printf("DNQP: Connected after WAIT_TIME,  DHCP Client start, and CanNOT get ip after 20 seconds, Start DHCP Server\n");
						}
						#endif	
							
							set_lan_dhcpd("br0", 2);
					
							for(i=0;i<3;i++){
							 if(isFileExist(DHCPD_PID_FILE)){
						 		break;
							 }else{
						 		sleep(1);
								}
							}
							system("echo 1 > /proc/pocket/en_filter"); //start to filter dhcp discover in bridge 
							if(getLan_MacAddress(LAN_Mac)){
							sprintf(cmdBuffer, "echo \"00000000 %s 2\" > /proc/pocket/filter_conf", LAN_Mac);
							system(cmdBuffer);
							}else{
								system("echo \"00000000 000000000000 2\" > /proc/pocket/filter_conf");
							}
							if(WLAN_Mode == 1){
							system("ifconfig eth0 down");
							//system("ifconfig eth1 down");
							sleep(5);
							system("ifconfig eth0 up");
							//system("ifconfig eth1 up");
							}
							else if(WLAN_Mode == 0)
							{
						#if defined(LOGDEBUG_ENABLED)
							syslog(LOG_INFO, "klogd: DNQP: AP mode, Connected after WAIT_TIME,  DHCP Client start, and CanNOT get ip after 20 seconds, Start DHCP Server, shutdown wlan first\n");
							printf("DNQP: AP mode, Connected after WAIT_TIME,  DHCP Client start, and CanNOT get ip after 20 seconds, Start DHCP Server, shutdown wlan first\n");
						#endif	
								system("ifconfig wlan0 down");
								#if defined (CONFIG_RTL_92D_SUPPORT)
								system("ifconfig wlan1 down");
								#endif

								Kill_Wlan_Applications();
							}
							start_dnrd();
							wlan_iface_state=1;	
							system("iptables -F INPUT");
							if(WLAN_State==0)
								WLAN_State=1;
							DHCPC_State=0;
						}else
							Confirm_DHCP_Time++;
					}else{
						Confirm_DHCP_Time=0;
						if (DHCPC_State == 2) {
							if(WLAN_Mode == 1){
							system("ifconfig eth0 down");
							//system("ifconfig eth1 down");
							sleep(5);
							system("ifconfig eth0 up");
							//system("ifconfig eth1 up");
							}
							
							if(WLAN_Mode == 0){
								system("ifconfig wlan0 down");
								#if defined (CONFIG_RTL_92D_SUPPORT)
								system("ifconfig wlan1 down");
								#endif
								Kill_Wlan_Applications();
							}
							wlan_iface_state=1;
							system("iptables -F INPUT");		
							
						}						
						DHCPC_State=1;
					}
						
				}
				
				if(WLAN_Mode ==1){
					if(WLAN_State==1 && (Confirm_Time >= 3 && Confirm_Time < 20) &&getFilter_Type()==1){
						//printf("client ever disconnect for 3~20 seconds, we should update our ip address\n");
						#if defined(LOGDEBUG_ENABLED)
							syslog(LOG_INFO, "klogd: DNQP: Client mode, client ever disconnect for 3~20 seconds, we should update our ip address\n");
							printf("DNQP: Client mode, client ever disconnect for 3~20 seconds, we should update our ip address\n");
						#endif
						WLAN_State=0;
						Confirm_Time=0;
					}
				}
			}
		}
ToNext:		
		if(CurrentTime > WAIT_TIME+5)
			CurrentTime=WAIT_TIME;
		else
			CurrentTime++;
			
			if(wlan_iface_state==1){
				Type = getFilter_Type();
				if(Type==2 ||Type==1 ){ //dhcp server state, we wait 8 seconds, type=2:server state, type=1:dhcp client and got ip address
				confirm_wlan_iface_state++;
				if(confirm_wlan_iface_state >=8){
					if(WLAN_Mode == 0){
						#if defined(LOGDEBUG_ENABLED)
							if(Type==2){
								syslog(LOG_INFO, "klogd: DNQP: AP mode, Up wlan interface when dhcp server started\n");
								printf("DNQP: AP mode, Up wlan interface when dhcp server started\n");
							}else if (Type==1){
								syslog(LOG_INFO, "klogd: DNQP: AP mode, Up wlan interface when dhcp client got ip\n");
								printf("DNQP: AP mode, Up wlan interface when dhcp client got ip\n");
							}
						#endif
						system("ifconfig wlan0 up");
						#if defined (CONFIG_RTL_92D_SUPPORT)
						if(isBandModeBoth())
						{
							system("ifconfig wlan1 up");
						}
						#endif

							sleep(1);
						Start_Wlan_Applications();
					}
						wlan_iface_state=0;
						confirm_wlan_iface_state=0;
				}
			}
			}
			
		#if 0
			if(WLAN_State==1 && check_ssid() && CurrentTime >= WAIT_TIME){
				WLAN_State=0;//restart all procedure
				system("iptables -F INPUT");//to avoid add iptables rule duplicate
			}
		#endif
	}else{
		system("echo 0 > /proc/pocket/en_filter");
	}

	return 0;
}


void Confirm_Chld_termniated(void)
{
	if(Start_Domain_Query_Process==0){
		Start_Domain_Query_Process=1;
		WLAN_State=0;//restart all procedure
		Last_Connect_Reason=0;
		system("iptables -F INPUT");//to avoid add iptables rule duplicate
	}
}
void Stop_Domain_Query_Process(void)
{
	Start_Domain_Query_Process=0;
}

void Reset_Domain_Query_Setting(void)
{
	int opmode=-1;
	int lan_dhcp_mode=0;
	int wlan_mode_root=0;
	struct sockaddr hwaddr;
	unsigned char *pMacAddr;
	unsigned char cmdBuffer[100];

	char lan_domain_name[	MAX_NAME_LEN]={0};
	
	
	apmib_get( MIB_DOMAIN_NAME, (void *)lan_domain_name);
	apmib_get(MIB_OP_MODE,(void *)&opmode);
	apmib_get(MIB_DHCP,(void *)&lan_dhcp_mode);
	apmib_get( MIB_WLAN_MODE, (void *)&wlan_mode_root); 
	if(opmode==1 &&((wlan_mode_root==1 && lan_dhcp_mode==15) || (wlan_mode_root==0 && lan_dhcp_mode==15)) && lan_domain_name[0]){
		system("echo 1 > /proc/pocket/en_filter"); 
		 if(getInAddr("br0", HW_ADDR, (void *)&hwaddr)){ 
			pMacAddr = hwaddr.sa_data;
			sprintf(cmdBuffer, "echo \"%s %02X%02X%02X%02X%02X%02X 0\" > /proc/pocket/filter_conf","00000000", pMacAddr[0], pMacAddr[1],pMacAddr[2], pMacAddr[3], pMacAddr[4], pMacAddr[5]); 
		}else{
			sprintf(cmdBuffer, "echo \"%s 000000000000 0\" > /proc/pocket/filter_conf","00000000"); 
		}
			system(cmdBuffer);
	}else{
			system("echo 0 > /proc/pocket/en_filter"); 
	}
}	
#endif 

#endif

//#endif
