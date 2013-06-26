

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
#if defined(CONFIG_GET_SERVER_IP_BY_DOMAIN)
#include <netdb.h>
#include <sys/socket.h>
#endif
#define RTL_L2TP_POWEROFF_PATCH 1

extern int setFirewallIptablesRules(int argc, char** argv);
extern int Last_WAN_Mode;
void start_dns_relay(void);
void start_igmpproxy(char *wan_iface, char *lan_iface);
void del_routing(void);

#define DHCPD_CONF_FILE "/var/udhcpd.conf"

#if defined(CONFIG_GET_SERVER_IP_BY_DOMAIN)
unsigned char tmp_default_gw[32], tmp_wan_if[8];
#endif

#ifdef SEND_GRATUITOUS_ARP
#include <net/if_arp.h>
#include <linux/if_ether.h>


#define _CONFIG_SCRIPT_PATH	"/bin"
#define _FIREWALL_SCRIPT_PROG	"firewall.sh"


#define ARP_TABLE_FILE "/proc/net/arp"
#define WAN_STATUS_FILE "/proc/eth1/up_event"
#define GRATUITOUS_ARP_NUM 3

struct arpMsg {
	struct ethhdr ethhdr;	 		/* Ethernet header */
	u_short htype;				/* hardware type (must be ARPHRD_ETHER) */
	u_short ptype;				/* protocol type (must be ETH_P_IP) */
	u_char  hlen;				/* hardware address length (must be 6) */
	u_char  plen;				/* protocol address length (must be 4) */
	u_short operation;			/* ARP opcode */
	u_char  sHaddr[6];			/* sender's hardware address */
	u_char  sInaddr[4];			/* sender's IP address */
	u_char  tHaddr[6];			/* target's hardware address */
	u_char  tInaddr[4];			/* target's IP address */
	u_char  pad[18];			/* pad for min. Ethernet payload (60 bytes) */
};
#define MAC_BCAST_ADDR		(unsigned char *) "\xff\xff\xff\xff\xff\xff"
int sendArpPack(unsigned char *mac, u_int32_t srcIp, u_int32_t targetIp)
{

	int 	optval = 1;
	int	s;			/* socket */
	int	rv = 1;			/* return value */
	struct sockaddr addr;		/* for interface name */
	struct arpMsg	arp;

	if ((s = socket (PF_PACKET, SOCK_PACKET, htons(ETH_P_ARP))) == -1) {
		return -1;
	}
	
	if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) == -1) {
		close(s);
		return -1;
	}

	/* send arp request */
	memset(&arp, 0, sizeof(arp));
	memcpy(arp.ethhdr.h_dest, MAC_BCAST_ADDR, 6);	/* MAC DA */
	memcpy(arp.ethhdr.h_source, mac, 6);		/* MAC SA */
	arp.ethhdr.h_proto = htons(ETH_P_ARP);		/* protocol type (Ethernet) */
	arp.htype = htons(ARPHRD_ETHER);		/* hardware type */
	arp.ptype = htons(ETH_P_IP);			/* protocol type (ARP message) */
	arp.hlen = 6;					/* hardware address length */
	arp.plen = 4;					/* protocol address length */
	arp.operation = htons(ARPOP_REQUEST);		/* ARP op code */
	memcpy(arp.sInaddr, &srcIp, sizeof(srcIp));		/* source IP address */
	memcpy(arp.sHaddr, mac, 6);			/* source hardware address */
	memcpy(arp.tInaddr, &targetIp, sizeof(targetIp));	/* target IP address */
	
	memset(&addr, 0, sizeof(addr));
	strcpy(addr.sa_data, "eth1");//interface);

	if (sendto(s, &arp, sizeof(arp), 0, &addr, sizeof(addr)) < 0)
		rv = 0;

	close(s);
	//DEBUG(LOG_INFO, "%salid arp replies for this address", rv ? "No v" : "V");	 
	return rv;
}

int sendArp()
{
	int i;
	char ip[24];
	char wanMacAddr[24];
	struct in_addr wanaddr;

	getInAddr("eth1", IP_ADDR_T, (void *)&wanaddr);
	sprintf(ip, "%s", inet_ntoa(wanaddr));

	bzero(wanMacAddr,sizeof(wanMacAddr));
	apmib_get(MIB_ELAN_MAC_ADDR,  (void *)wanMacAddr);
	if(!memcmp(wanMacAddr, "\x00\x00\x00\x00\x00\x00", 6)){
		apmib_get(MIB_HW_NIC0_ADDR,  (void *)wanMacAddr);
	}
	for(i=0;i<GRATUITOUS_ARP_NUM;i++)
	{
		sendArpPack(wanMacAddr,wanaddr.s_addr, wanaddr.s_addr);
		sleep(1);
	}
}

int checkWanStatus()
{ 
	FILE *pfile = NULL;
	int status = -1;
	int wan_type = -1;
	struct in_addr wanaddr;
	char ip[24];
	char tmpBuf[128];
	char wanMacAddr[24];
	int i;
	
	if(!isFileExist(WAN_STATUS_FILE))
	{
		printf("%s: %s is not exist!!\n",__FUNCTION__, WAN_STATUS_FILE);
		return -1;
	}
	apmib_get(MIB_WAN_DHCP,(void *)&wan_type);
	if(DHCP_DISABLED != wan_type)
	{	
		return -1;
	}
	if((pfile = fopen(WAN_STATUS_FILE,"r+"))!= NULL)
	{
		fscanf(pfile,"%d",&status);
		if(status == 1)
		{		
			RunSystemCmd(WAN_STATUS_FILE, "echo", "0", NULL_STR);	/*bridge mode with multiple vlan*/
			sendArp();
		}
	}
	else
	{
		return -1;
	}
	return 0;
}

#endif


int avoid_confliction_ip(char *wanIp, char *wanMask)
{
	char line_buffer[100]={0};
	char *strtmp=NULL;
	char tmp1[64]={0};
	unsigned int tmp1Val;
	struct in_addr inIp, inMask, inGateway;
	struct in_addr myIp, myMask, mask;
	unsigned int inIpVal, inMaskVal, myIpVal, myMaskVal, maskVal;
	char tmpBufIP[64]={0}, tmpBufMask[64]={0};
	DHCP_T dhcp;
	
	apmib_get( MIB_DHCP, (void *)&dhcp);
	
	if(isFileExist(DHCPD_PID_FILE) == 0 || dhcp == DHCP_SERVER){

	}else{
		return 0; //no dhcpd or dhcp server is disable
	}
	
	if ( !inet_aton(wanIp, &inIp) ) {
		printf("\r\n Invalid IP-address value!__[%s-%u]\r\n",__FILE__,__LINE__);
		return 0;
	}
	
	if ( !inet_aton(wanMask, &inMask) ) {
		printf("\r\n Invalid IP-address value!__[%s-%u]\r\n",__FILE__,__LINE__);
		return 0;
	}
	
	memcpy(&inIpVal, &inIp, 4);
	memcpy(&inMaskVal, &inMask, 4);


	getInAddr("br0", IP_ADDR_T, (void *)&myIp );	
	getInAddr("br0", NET_MASK_T, (void *)&myMask );
		
	
	memcpy(&myIpVal, &myIp, 4);
	memcpy(&myMaskVal, &myMask, 4);

//printf("\r\n inIpVal=[0x%x],__[%s-%u]\r\n",inIpVal,__FILE__,__LINE__);
//printf("\r\n inMaskVal=[0x%x],__[%s-%u]\r\n",inMaskVal,__FILE__,__LINE__);
//printf("\r\n myIpVal=[0x%x],__[%s-%u]\r\n",myIpVal,__FILE__,__LINE__);
//printf("\r\n myMaskVal=[0x%x],__[%s-%u]\r\n",myMaskVal,__FILE__,__LINE__);

	memcpy(&maskVal,myMaskVal>inMaskVal?&inMaskVal:&myMaskVal,4);
	
//printf("\r\n maskVal=[0x%x],__[%s-%u]\r\n",maskVal,__FILE__,__LINE__);
	
	if((inIpVal & maskVal) == (myIpVal & maskVal)) //wan ip conflict lan ip 
	{
		int i=0, j=0;
//printf("\r\n wan ip conflict lan ip!,__[%s-%u]\r\n",__FILE__,__LINE__);

		for(i=0; i<32; i++)
		{
			if((maskVal & (1<<i)) != 0)
				break;
		}
		
		if((myIpVal & (1<<i)) == 0)
		{
			myIpVal = myIpVal+(1<<i);
		}
		else
		{
			myIpVal = myIpVal-(1<<i);
		}
		
		memcpy(&myIp, &myIpVal, 4);
				
						
		for(j=0; j<32; j++)
		{
			if((myMaskVal & (1<<j)) != 0)
				break;
		}
		
	//	j=(32-j)/8;

		system("killall -9 udhcpd 2> /dev/null");
		system("rm -f /var/run/udhcpd.pid 2> /dev/null");
		system("rm -f /var/udhcpd.conf");
		
		sprintf(line_buffer,"interface %s\n","br0");
		write_line_to_file(DHCPD_CONF_FILE, 1, line_buffer);
		
		apmib_get(MIB_DHCP_CLIENT_START,  (void *)tmp1);		
	//	memcpy(tmp1, &myIpVal,  j);
		*(unsigned int*)tmp1 ^= (1<<(j));
		strtmp= inet_ntoa(*((struct in_addr *)tmp1));
//printf("\r\n start ip=[%s],__[%s-%u]\r\n",strtmp,__FILE__,__LINE__);		
		sprintf(line_buffer,"start %s\n",strtmp);
		write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);
		
		apmib_get(MIB_DHCP_CLIENT_END,  (void *)tmp1);		
		//memcpy(tmp1, &myIpVal,  j);
		*(unsigned int*)tmp1 ^= (1<<(j));
		strtmp= inet_ntoa(*((struct in_addr *)tmp1));
//printf("\r\n end ip=[%s],__[%s-%u]\r\n",strtmp,__FILE__,__LINE__);		
		sprintf(line_buffer,"end %s\n",strtmp);
		write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);
	
//printf("\r\n subnet mask=[%s],__[%s-%u]\r\n",inet_ntoa(myMask),__FILE__,__LINE__);			
		sprintf(line_buffer,"opt subnet %s\n",inet_ntoa(myMask));
		write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);

//printf("\r\n gateway ip=[%s],__[%s-%u]\r\n",inet_ntoa(myIp),__FILE__,__LINE__);					
		sprintf(line_buffer,"opt router %s\n",inet_ntoa(myIp));
		write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);

//printf("\r\n dns ip=[%s],__[%s-%u]\r\n",inet_ntoa(myIp),__FILE__,__LINE__);							
		sprintf(line_buffer,"opt dns %s\n",inet_ntoa(myIp)); /*now strtmp is ip address value */
		write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);
		
		memset(tmp1,0x00,sizeof(tmp1));
		apmib_get( MIB_DOMAIN_NAME, (void *)&tmp1);
		if(tmp1[0]){
			sprintf(line_buffer,"opt domain %s\n",tmp1);
			write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);
		}
		
		memset(tmp1,0x00,sizeof(tmp1));
		memcpy(tmp1, &myIpVal,  4);
		strtmp= inet_ntoa(*((struct in_addr *)tmp1));
		sprintf(tmpBufIP,"%s",strtmp);
//printf("\r\n tmpBufIP=[%s],__[%s-%u]\r\n",tmpBufIP,__FILE__,__LINE__);

		memset(tmp1,0x00,sizeof(tmp1));
		memcpy(tmp1, &myMaskVal,  4);
		strtmp= inet_ntoa(*((struct in_addr *)tmp1));
		sprintf(tmpBufMask,"%s",strtmp);
//printf("\r\n tmpBufMask=[%s],__[%s-%u]\r\n",tmpBufMask,__FILE__,__LINE__);

		memset(line_buffer,0x00,sizeof(line_buffer));
		sprintf(line_buffer, "ifconfig br0 %s netmask %s", tmpBufIP, tmpBufMask);
//printf("\r\n line_buffer=[%s],__[%s-%u]\r\n",line_buffer,__FILE__,__LINE__);									
		system(line_buffer);

		sprintf(line_buffer, "udhcpd %s", DHCPD_CONF_FILE);
		system(line_buffer);
		//start_dnrd();
		return 1;
	}

	return 0;
}
#if defined(CONFIG_GET_SERVER_IP_BY_DOMAIN)
int translate_domain_to_ip(unsigned char *server_domain, struct in_addr *server_ip)
{
	unsigned char tmp_server_ip[32];	
	unsigned char str[32], tmp_cmd[128];
	char   **pptr;
	struct hostent *hptr;
	int count=0;
		
	while(count<=3)
	{
		if((hptr = gethostbyname(server_domain)) != NULL)
		{
			sprintf(tmp_server_ip, "%s", inet_ntop(hptr->h_addrtype, hptr->h_addr, str, sizeof(str)));
			inet_aton(tmp_server_ip, (void *)server_ip);
			return 0;		
		}else
		{
			printf(" gethostbyname error for host:%s try again!\n", server_domain);
			count++;
		}
	}
	return -1;
}
#endif

void wan_connect(char *interface, char *option)
{
	char line[128], arg_buff[200];
	char *cmd_opt[16];
	int cmd_cnt = 0, intValue=0, x, dns_mode=0, index=0;
	int dns_found=0, wan_type=0, conn_type=0, ppp_mtu=0;
	struct in_addr wanaddr, lanaddr;
	char *strtmp=NULL;
	char wanip[32]={0}, mask[32]={0},remoteip[32]={0};
	char nameserver[32], nameserver_ip[32];
	char dns_server[5][32];
	char tmp_args[16]={0};
	char *token=NULL, *savestr1=NULL;
	FILE *fp1;
	unsigned char domanin_name[MAX_NAME_LEN]={0};
	unsigned char cmdBuffer[100]={0};
	unsigned char tmpBuff[200]={0};
	unsigned char dynip[32]={0};
	int lan_type=0;
	int op_mode=0;
	int ret = 0;
//	printf("%s(%d): wan_connect option=%s\n",__FUNCTION__,__LINE__, option);//Added for test
//printf("%s(%d): wan_connect interface=%s\n",__FUNCTION__,__LINE__, interface);//Added for test
	#if defined(CONFIG_DYNAMIC_WAN_IP)
	int opmode=0, wisp_wan_id=0;
	char tmp_buf[64]={0};
	char ServerIp[32],netIp[32];
	unsigned int serverAddr,netAddr;
	struct in_addr tmpInAddr;
	unsigned int wanIpAddr, maskAddr, remoteIpAddr;
	#endif
	apmib_get(MIB_WAN_DHCP,(void *)&wan_type);
	apmib_get( MIB_DNS_MODE, (void *)&dns_mode);
	apmib_get(MIB_DHCP,(void *)&lan_type);
	apmib_get(MIB_OP_MODE, (void *)&op_mode);


	//when lan set dhcp client,only br0 con allowed.wan conn make no sense
	if(lan_type==DHCP_CLIENT && strcmp(interface, "br0")!=0)
		return;
	
#ifdef MULTI_PPPOE
	if(wan_type > 2 && (!strncmp(interface, "ppp",3))){
#else
#if defined(CONFIG_DYNAMIC_WAN_IP)
	if(!strcmp(interface, "ppp0")){
#else
	if(wan_type > 2 && !strcmp(interface, "ppp0")){
#endif
#endif
		
#if 1//AVOID_CONFLICTION_IP
#ifdef MULTI_PPPOE
		getInAddr(interface, IP_ADDR_T, (void *)&wanaddr);
#else
		getInAddr("ppp0", IP_ADDR_T, (void *)&wanaddr);
#endif		
		strtmp = inet_ntoa(wanaddr);
		sprintf(wanip, "%s",strtmp); 
#ifdef MULTI_PPPOE
		getInAddr(interface, NET_MASK_T, (void *)&wanaddr);
#else
		getInAddr("ppp0", NET_MASK_T, (void *)&wanaddr);
#endif			
		strtmp = inet_ntoa(wanaddr);
		sprintf(mask, "%s",strtmp); 
		ret = avoid_confliction_ip(wanip,mask);
#endif

#if defined(CONFIG_DYNAMIC_WAN_IP)
		if(wan_type==PPTP || wan_type==L2TP){
			if(opmode==GATEWAY_MODE)
				RunSystemCmd(NULL_FILE, "route", "del", "default", "dev", "eth1", NULL_STR);
			if(opmode==WISP_MODE)
				RunSystemCmd(NULL_FILE, "route", "del", "default", "dev", "wlan0", NULL_STR);
		}
#endif

		if(wan_type==PPTP){
			apmib_get(MIB_PPTP_CONNECTION_TYPE, (void *)&conn_type);
			if(intValue==1){
				RunSystemCmd(PROC_PPTP_CONN_FILE, "echo", "5", NULL_STR);
			}else{
				RunSystemCmd(PROC_PPTP_CONN_FILE, "echo", "0", NULL_STR);
			}
		}
		if((wan_type==PPPOE)||(wan_type==PPTP)||(wan_type==L2TP))
		{
#ifdef MULTI_PPPOE
			intValue = getInAddr(interface, 0, (void *)&wanaddr);
#else
			intValue = getInAddr("ppp0", 0, (void *)&wanaddr);
#endif			
			if(intValue==1){

				strtmp = inet_ntoa(wanaddr);
				sprintf(remoteip, "%s",strtmp); 
#ifdef MULTI_PPPOE
#else
				RunSystemCmd(NULL_FILE, "route", "del", "default", NULL_STR);
#endif

#ifdef MULTI_PPPOE
			RunSystemCmd(NULL_FILE, "route", "add", "-net", "default", "gw", remoteip, "dev", interface, NULL_STR);
#else
			RunSystemCmd(NULL_FILE, "route", "add", "-net", "default", "gw", remoteip, "dev", "ppp0", NULL_STR);
#endif				
			}			
		}
		if(wan_type==PPTP || wan_type==L2TP){
			token=NULL;
			savestr1=NULL;	     
			sprintf(arg_buff, "%s", option);
		
			token = strtok_r(arg_buff," ", &savestr1);
			x=0;
			do{
				if (token == NULL){/*check if the first arg is NULL*/
					break;
				}else{   
					if(x==1){
						ppp_mtu = atoi(token);
						break;
					}
					if(!strcmp(token, "mtu"))
						x=1;
				}
			
				token = strtok_r(NULL, " ", &savestr1);
			}while(token !=NULL);  
		
		}
		if(wan_type==PPTP){
			apmib_get(MIB_PPTP_MTU_SIZE, (void *)&intValue);
			if(ppp_mtu > 0 && intValue > ppp_mtu)
				intValue = ppp_mtu;
			sprintf(tmp_args, "%d", intValue);
		}else if(wan_type==L2TP){
			apmib_get(MIB_L2TP_MTU_SIZE, (void *)&intValue);
			if(ppp_mtu > 0 && intValue > ppp_mtu)
				intValue = ppp_mtu;
			sprintf(tmp_args, "%d", intValue);
		}else if(wan_type==PPPOE){
			apmib_get(MIB_PPP_MTU_SIZE, (void *)&intValue);
			sprintf(tmp_args, "%d", intValue);			
		}
#ifdef MULTI_PPPOE
		/* Do not set mtu by ifconfig, pppd negotiates about mtu by itself */
		//RunSystemCmd(NULL_FILE, "ifconfig", interface, "mtu", tmp_args, "txqueuelen", "25",NULL_STR);
		RunSystemCmd(NULL_FILE, "ifconfig", interface, "txqueuelen", "25",NULL_STR);
#else
		/* Do not set mtu by ifconfig, pppd negotiates about mtu by itself */
		//RunSystemCmd(NULL_FILE, "ifconfig", "ppp0", "mtu", tmp_args, "txqueuelen", "25",NULL_STR);
		RunSystemCmd(NULL_FILE, "ifconfig", "ppp0", "txqueuelen", "25",NULL_STR);
#endif
//		printf("%s(%d): wan_type=%d,dns_mode=%d\n",__FUNCTION__,__LINE__, wan_type,dns_mode);//Added for test
		if(dns_mode==1){
			start_dns_relay();
		}else{
			fp1= fopen(PPP_RESOLV_FILE, "r");
			if (fp1 != NULL){
				for (x=0;x<5;x++){
					memset(dns_server[x], '\0', 32);
				}
				while (fgets(line, sizeof(line), fp1) != NULL) {
						memset(nameserver_ip, '\0', 32);
						dns_found = 0;
						sscanf(line, "%s %s", nameserver, nameserver_ip);
						for(x=0;x<5;x++){
							if(dns_server[x][0] != '\0'){
								if(!strcmp(dns_server[x],nameserver_ip)){
									dns_found = 1; 
									break;
								}
							}
						}
						if(dns_found ==0){
							for(x=0;x<5;x++){
								if(dns_server[x][0] == '\0'){
									sprintf(dns_server[x], "%s", nameserver_ip);
									//printf("---%s---\n",nameserver_ip);
									break;
								}
							}
						}
				}
				fclose(fp1);
			}else
			{//PPP_RESOLV_FILE not exist, use default dns
				sprintf(dns_server[0], "%s", "168.95.1.1");	
				
				//printf("---%s---\n","168.95.1.1");
			}
			//for (x=0;x<5;x++){
			//	if(dns_server[x]){
			//		fprintf(stderr, "name server=%s\n", dns_server[x]);
			//	}
			//}
			RunSystemCmd(NULL_FILE, "killall", "dnrd", NULL_STR);
			if(isFileExist(DNRD_PID_FILE)){
				unlink(DNRD_PID_FILE);
			}		
			apmib_get( MIB_DOMAIN_NAME,  (void *)domanin_name);					
			getInAddr("br0", IP_ADDR_T, (void *)&lanaddr);
			strtmp = inet_ntoa(lanaddr);
			sprintf((char *)dynip, "%s",strtmp); 						
		#if !defined(CONFIG_RTL_ULINKER)
			RunSystemCmd(NULL_FILE, "rm", "-f", "/etc/hosts", NULL_STR);
			memset(cmdBuffer, 0x00, sizeof(cmdBuffer));
			if(domanin_name[0])
			{
				sprintf((char *)cmdBuffer,"%s\\%s%s%s%s", dynip, domanin_name, "AP.com|",domanin_name, "AP.net");				
			}
			else
			{
				sprintf((char *)cmdBuffer,"%s\\%s%s%s%s", dynip, "realtek", "AP.com|","realtek", "AP.net");
			}
			RunSystemCmd("/etc/hosts", "echo",cmdBuffer,NULL_STR);
		#else
			//usb0_up();
		#endif /* #if !defined(CONFIG_RTL_ULINKER) */
			cmd_opt[cmd_cnt++]="dnrd";
			cmd_opt[cmd_cnt++]="--cache=off";

			for(x=0;x<5;x++){
				if(dns_server[x][0] != '\0'){
					cmd_opt[cmd_cnt++]="-s";
					cmd_opt[cmd_cnt++]=&dns_server[x][0];
				}
			}

			cmd_opt[cmd_cnt++] = 0;
			//for (x=0; x<cmd_cnt;x++)
			//	fprintf(stderr, "cmd index=%d, opt=%s \n", x, cmd_opt[x]);
			
			RunSystemCmd(NULL_FILE, "cp", PPP_RESOLV_FILE, "/var/resolv.conf", NULL_STR);
			DoCmd(cmd_opt, NULL_FILE);
		}
	}else 
#if defined(CONFIG_DYNAMIC_WAN_IP)
	if(strcmp(interface, "ppp0")){	
#else
	if(wan_type == 1 && (strncmp(interface, "ppp",3))){//dhcp conn
#endif
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
					sprintf(wanip, "%s", token); /*wan ip address */
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
		
#if 1//AVOID_CONFLICTION_IP
		/*if br0 get ip need to check*/
 		if(strcmp(interface, "br0")){
			ret = avoid_confliction_ip(wanip,mask);
		}
#endif

		RunSystemCmd(NULL_FILE, "ifconfig", interface, wanip, "netmask", mask, NULL_STR);	
		//RunSystemCmd(NULL_FILE, "route", "add", "-net", "default", "gw", remoteip, "dev", interface, NULL_STR);
#if defined(CONFIG_DYNAMIC_WAN_IP)
		if(wan_type != PPTP && wan_type != L2TP) {
#endif
		RunSystemCmd(NULL_FILE, "route", "del", "default", NULL_STR);
		RunSystemCmd(NULL_FILE, "route", "add", "-net", "default", "gw", remoteip, "dev", interface, NULL_STR);
//		printf("%s(%d): wan_type=%d,dns_mode=%d\n",__FUNCTION__,__LINE__, wan_type,dns_mode);//Added for test
			
		if(dns_mode==1){
			start_dns_relay();
		}else{
RunSystemCmd(NULL_FILE, "killall", "dnrd", NULL_STR);
			if(isFileExist(DNRD_PID_FILE)){
				unlink(DNRD_PID_FILE);
			}
			apmib_get( MIB_DOMAIN_NAME,  (void *)domanin_name);
						
			getInAddr("br0", IP_ADDR_T, (void *)&lanaddr);
			strtmp = inet_ntoa(lanaddr);
			sprintf((char *)dynip, "%s",strtmp); 						
			
			RunSystemCmd(NULL_FILE, "rm", "-f", "/etc/hosts", NULL_STR);
			memset(cmdBuffer, 0x00, sizeof(cmdBuffer));
			
			if(domanin_name[0])
			{
				sprintf((char *)cmdBuffer,"%s\\%s%s%s%s", dynip, domanin_name, "AP.com|",domanin_name, "AP.net");				
			}
			else
			{
				sprintf((char *)cmdBuffer,"%s\\%s%s%s%s", dynip, "realtek", "AP.com|","realtek", "AP.net");
			}
			RunSystemCmd("/etc/hosts", "echo",cmdBuffer,NULL_STR);
			
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
 		if(strcmp(interface, "br0")){
			DoCmd(cmd_opt, NULL_FILE);
		}
			
		}
#ifdef CONFIG_POCKET_AP_SUPPORT
#else
	 if(strcmp(interface, "br0")){	
		setFirewallIptablesRules(0, NULL);
	 }
     else
     {
        unsigned char restart_iapp[100] = {0};
        FILE *fp;
        if(isFileExist(RESTART_IAPP)){
            fp= fopen(RESTART_IAPP, "r");
            if (!fp) {
                printf("can not open /var/restart_iapp\n");
                return;
            }
            fgets(restart_iapp, sizeof(restart_iapp), fp);
            fclose(fp);
            system(restart_iapp);
        }
     }
#endif	//CONFIG_POCKET_AP_SUPPORT
#if defined(CONFIG_DYNAMIC_WAN_IP)
	}
#endif
#if defined(CONFIG_DYNAMIC_WAN_IP)
	if(wan_type == PPTP || wan_type == L2TP){

		for(x=0;x<5;x++){
			if(dns_server[x][0] != '\0'){
				sprintf(line,"nameserver %s\n", dns_server[x]);
				if(x==0){
					write_line_to_file(RESOLV_CONF, 1, line);
	//						write_line_to_file(DHCP_RESOLV_FILE, 1, line);
					
				}else{
					write_line_to_file(RESOLV_CONF, 2, line);
	//						write_line_to_file(DHCP_RESOLV_FILE, 2, line);
				}
			}
		}

		
		RunSystemCmd(NULL_FILE, "route", "del", "default", "dev", interface, NULL_STR);
		//printf("%s:%d route del  default dev %s\n",__FUNCTION__,__LINE__,interface);
#if defined(CONFIG_GET_SERVER_IP_BY_DOMAIN)	
		//set tmp default gw for get ip from domain
		sprintf(tmp_default_gw, "%s", remoteip);
		sprintf(tmp_wan_if, "%s", interface);
#endif
		RunSystemCmd(NULL_FILE, "route", "add", "-net", "default", "gw", remoteip, "dev", interface, NULL_STR); //redundant, but safe
		//printf("%s:%d route add -net default gw %s dev %s\n",__FUNCTION__,__LINE__,remoteip,interface);
		if(isFileExist(TEMP_WAN_CHECK) && isFileExist(TEMP_WAN_DHCP_INFO)){
			if(wan_type == PPTP){				
#if defined(CONFIG_GET_SERVER_IP_BY_DOMAIN)	
					unsigned char pptp_server_domain[32];
					struct in_addr server_ip;
					int enable_pptp_server_domain=0;
					//printf("%s:%d pptpServerDomain=%s\n",__FUNCTION__,__LINE__,pptp_server_domain);
				
					apmib_get(MIB_PPTP_GET_SERV_BY_DOMAIN,(void*)&enable_pptp_server_domain);
					if(enable_pptp_server_domain)
					{	
						apmib_get(MIB_PPTP_SERVER_DOMAIN, pptp_server_domain);
						//printf("%s:%d pptpServerDomain=%s\n",__FUNCTION__,__LINE__,pptp_server_domain);
						if(translate_domain_to_ip(pptp_server_domain, &server_ip) == 0)
						{			
							//printf("%s:%d server_ip=%s\n",__FUNCTION__,__LINE__,inet_ntoa(server_ip));
							apmib_set(MIB_PPTP_SERVER_IP_ADDR, (void *)&server_ip);
							apmib_update(CURRENT_SETTING);
						}else
						{
							printf("can't get pptpServerDomain:%s 's IP",pptp_server_domain);
							return 0;
						}
					}
#endif
				apmib_get(MIB_PPTP_SERVER_IP_ADDR,	(void *)tmp_buf);
				strtmp= inet_ntoa(*((struct in_addr *)tmp_buf));
				sprintf(ServerIp, "%s", strtmp);
				serverAddr=((struct in_addr *)tmp_buf)->s_addr;
				
				inet_aton(wanip, &tmpInAddr);
				wanIpAddr=tmpInAddr.s_addr;

				inet_aton(mask, &tmpInAddr);
				maskAddr=tmpInAddr.s_addr;

				inet_aton(remoteip, &tmpInAddr);
				remoteIpAddr=tmpInAddr.s_addr;

				if((serverAddr & maskAddr) != (wanIpAddr & maskAddr)){
					//Patch for our router under another router to dial up pptp
					//let pptp pkts via pptp default gateway
					netAddr = (serverAddr & maskAddr);
					((struct in_addr *)tmp_buf)->s_addr=netAddr;
					strtmp= inet_ntoa(*((struct in_addr *)tmp_buf));
					sprintf(netIp, "%s", strtmp);
					RunSystemCmd(NULL_FILE, "route", "add", "-net", netIp, "netmask", mask,"gw", remoteip,NULL_STR);
					printf("%s:%d route add -net %s netmask %s gw %s\n",__FUNCTION__,__LINE__,netIp,mask,remoteip);

				}
			}
			else if(wan_type == L2TP){			
#if defined(CONFIG_GET_SERVER_IP_BY_DOMAIN)	
				unsigned char l2tp_server_domain[32];
				struct in_addr server_ip;
				int enable_l2tp_server_domain=0;
			
				apmib_get(MIB_L2TP_GET_SERV_BY_DOMAIN,(void*)&enable_l2tp_server_domain);
				if(enable_l2tp_server_domain)
				{	
					apmib_get(MIB_L2TP_SERVER_DOMAIN, l2tp_server_domain);
					//printf("%s:%d l2tpServerDomain=%s\n",__FUNCTION__,__LINE__,l2tp_server_domain);
					if(translate_domain_to_ip(l2tp_server_domain, &server_ip) == 0)
					{
						//printf("%s:%d server_ip=%s\n",__FUNCTION__,__LINE__,inet_ntoa(server_ip));
						apmib_set(MIB_L2TP_SERVER_IP_ADDR, (void *)&server_ip);
						apmib_update(CURRENT_SETTING);
					}else
					{
						printf("can't get l2tpServerDomain:%s 's IP",l2tp_server_domain);
						return 0;
					}
				}
#endif
				apmib_get(MIB_L2TP_SERVER_IP_ADDR,	(void *)tmp_buf);
				strtmp= inet_ntoa(*((struct in_addr *)tmp_buf));
				sprintf(ServerIp, "%s", strtmp);
				serverAddr=((struct in_addr *)tmp_buf)->s_addr;
				
				inet_aton(wanip, &tmpInAddr);
				wanIpAddr=tmpInAddr.s_addr;

				inet_aton(mask, &tmpInAddr);
				maskAddr=tmpInAddr.s_addr;

				inet_aton(remoteip, &tmpInAddr);
				remoteIpAddr=tmpInAddr.s_addr;

				if((serverAddr & maskAddr) != (wanIpAddr & maskAddr)){
					//Patch for our router under another router to dial up pptp
					//let l2tp pkts via pptp default gateway
					netAddr = (serverAddr & maskAddr);
					((struct in_addr *)tmp_buf)->s_addr=netAddr;
					strtmp= inet_ntoa(*((struct in_addr *)tmp_buf));
					sprintf(netIp, "%s", strtmp);
					RunSystemCmd(NULL_FILE, "route", "add", "-net", netIp, "netmask", mask,"gw", remoteip,NULL_STR);
				}
			}
			
			unlink(TEMP_WAN_CHECK);
			unlink(TEMP_WAN_DHCP_INFO);
		}

		if(isFileExist(PPP_CONNECT_FILE)){
			unlink(PPP_CONNECT_FILE);
		}
		
		//start pptp/l2tp dial up

		if(wan_type == PPTP){
			set_pptp(opmode, interface, "br0", wisp_wan_id, 1);
		}
		if(wan_type == L2TP){
			set_l2tp(opmode, interface, "br0", wisp_wan_id, 1);
		}
		return;
	}
#endif

	}
	else if(lan_type == 1 && strcmp(interface, "br0")==0){
		
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
					sprintf(wanip, "%s", token); /*wan ip address */
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
		
		RunSystemCmd(NULL_FILE, "ifconfig", interface, wanip, "netmask", mask, NULL_STR);	
		RunSystemCmd(NULL_FILE, "route", "add", "-net", "default", "gw", remoteip, "dev", interface, NULL_STR);
//		printf("%s(%d): wan_type=%d,dns_mode=%d\n",__FUNCTION__,__LINE__, wan_type,dns_mode);//Added for test
		if(dns_mode==1){
			start_dns_relay();
		}else{
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
			DoCmd(cmd_opt, NULL_FILE);

		}
		if(op_mode!=1)
		{
			start_igmpproxy(interface, "br0");
		}
	}
#ifdef CONFIG_POCKET_AP_SUPPORT
#else

	if(strcmp(interface, "br0")){
		
#if defined(CONFIG_POCKET_ROUTER_SUPPORT)
		if((ret == 1) && (op_mode == GATEWAY_MODE)) //AP/client mode won't call this function
#else
		if(ret == 1)
#endif			
		{
			apmib_get(MIB_WLAN_BAND2G5G_SELECT,(void *)&intValue);
			if(intValue == BANDMODEBOTH)
			{
				system("ifconfig wlan1 down");
			}
			
#if !defined(CONFIG_POCKET_ROUTER_SUPPORT)
			//when op_mode== GATEWAY_MODE for pocket AP, there isn't interface eth0
			system("ifconfig eth0 down");
#endif
			system("ifconfig wlan0 down");
		
			sleep(10);
			
#if !defined(CONFIG_POCKET_ROUTER_SUPPORT)
			system("ifconfig eth0 up");
#endif
			system("ifconfig wlan0 up");
			if(intValue == BANDMODEBOTH)
			{
				system("ifconfig wlan1 up");
			}
		}


		printf("WAN Connected\n");
		start_ntp();
		start_ddns();
#ifdef MULTI_PPPOE
		if(!strcmp(interface,"ppp0"))
			start_igmpproxy(interface, "br0");
#else 
		start_igmpproxy(interface, "br0");
#endif
	}
#endif	//CONFIG_POCKET_AP_SUPPORT
#if defined(ROUTE_SUPPORT)
	if(strcmp(interface, "br0")){	
	del_routing();
	start_routing(interface);
	}
#endif

}


#ifdef MULTI_PPPOE
void wan_disconnect(char *option , char *conncetOrder)
#else
void wan_disconnect(char *option)
#endif
{
	int intValue=0;
	int wan_type=0;
	int Last_WAN_Mode=0;
	FILE *fp;
//	printf("WAN Disconnect option=%s\n", option);//Added for test

	apmib_get( MIB_WAN_DHCP,(void *)&wan_type);

#ifdef MULTI_PPPOE
	int connnect_num,IsRuningNum = 0;
	char cmd[50];
	//when one pppoe timeout,execute disconnect.sh ,just return to let it go on connecting
	//only if all pppoe disconnect			
	//if(getRuningNum(wan_type,conncetOrder) >=1)

	if(wan_type == PPPOE && strcmp(conncetOrder,"NOMULPPPOE") && strcmp(conncetOrder,""))
	{
		
		FILE *pF;
		apmib_get(MIB_PPP_CONNECT_COUNT, (void *)&connnect_num);	
		if(connnect_num >= 1){
			if(isFileExist("/etc/ppp/link")){
				if(strcmp(conncetOrder,"1"))
					++IsRuningNum;
				else
					system("rm /etc/ppp/link >/dev/null 2>&1");
			}
		}	
				
		if(connnect_num >= 2){
			if(isFileExist("/etc/ppp/link2")){
				if(strcmp(conncetOrder,"2"))
					++IsRuningNum;
				else
					system("rm /etc/ppp/link2 >/dev/null 2>&1");			
			}		
		}		
		if(connnect_num >= 3){
			if(isFileExist("/etc/ppp/link3")){
				if(strcmp(conncetOrder,"3"))
					++IsRuningNum;
				else
					system("rm /etc/ppp/link3 >/dev/null 2>&1");			
			}	
		}
	
		if(connnect_num >= 4){
			if(isFileExist("/etc/ppp/link4")){
				if(strcmp(conncetOrder,"4"))
					++IsRuningNum;
				else
					system("rm /etc/ppp/link4 >/dev/null 2>&1");			
			}	
		}
	
		if((pF = fopen("/etc/ppp/ppp_order_info","r+"))!= NULL){				
			FILE* ftmp=fopen("/etc/ppp/tmp","wt");			
			int match,order;
			char name[10];
            if(ftmp == NULL)
            {
                   printf("can't open the file \n");
                   return ;
            }
			sscanf(conncetOrder,"%d",&match);			
			while( fscanf(pF,"%d--%s",&order,name) > 0 )
			{			    
			   if(match != order){
			   	
				  fprintf(ftmp,"%d--%s\n",order,name);				  
			   }else{			   	 
			   	
				   //clear the iptables rule 
				   char flushcmd[100];
				   char buf[100];
				   FILE *pRule;			   
				   //clear filter chain
				   sprintf(buf,"iptables -t filter -S | grep %s | cut -d ' ' -f 2- > /etc/ppp/filterrule",
					   name);
				   system(buf);
				   if((pRule = fopen("/etc/ppp/filterrule","r+"))!= NULL){
					   while(fgets(buf, 100, pRule)){
						   sprintf(flushcmd,"iptables -t filter -D %s >/dev/null 2>&1",buf);
						   system(flushcmd);
					   }
					   fclose(pRule);	
				   }
				   system("rm /etc/ppp/filterrule >/dev/null 2>&1");
				   //clear nat chain		   
				   sprintf(buf,"iptables -t nat -S | grep %s | cut -d ' ' -f 2- > /etc/ppp/natrule",
					   name);
				   system(buf);
				   if((pRule = fopen("/etc/ppp/natrule","r+"))!= NULL){
					   while(fgets(buf, 100, pRule)){
						   sprintf(flushcmd,"iptables -t nat -D %s >/dev/null 2>&1",buf);
						   system(flushcmd);
					   }
					   fclose(pRule); 
				   }
				   system("rm /etc/ppp/natrule >/dev/null 2>&1");				   
				   //clear mangle chain			   
				   sprintf(buf,"iptables -t mangle -S | grep %s | cut -d ' ' -f 2- > /etc/ppp/manglerule",
					   name);
				   system(buf); 				   
				   if((pRule = fopen("/etc/ppp/manglerule","r+"))!= NULL){
					   while(fgets(buf, 100, pRule)){
						   sprintf(flushcmd,"iptables -t nat -D %s >/dev/null 2>&1",buf);
						   system(flushcmd);
					   }
					   fclose(pRule); 
				   }
				   system("rm /etc/ppp/manglerule >/dev/null 2>&1");				
					
				   //clear ip policy rule				   
				   sprintf(buf,"/etc/ppp/%s.cmd",name);
				   if((pRule = fopen(buf,"r+")) != NULL){
				   		while(fgets(buf, 100, pRule)){
						   system(buf);
					   }						
						fclose(pRule);
				   }				   
				   sprintf(flushcmd,"rm %s >/dev/null 2>&1",buf);
				   system(flushcmd);
				   
			   }			   
			}			
			fclose(ftmp);
			fclose(pF);					
			system("cp /etc/ppp/tmp /etc/ppp/ppp_order_info >/dev/null 2>&1");			
			system("rm /etc/ppp/tmp >/dev/null 2>&1");	
		}		
		
		if(IsRuningNum >=1 )
		{
			return;
		}			
	    system("ip rule del table 100 >/dev/null 2>&1"); 
	    system("ip route del table 100 >/dev/null 2>&1");		
		system("rm /etc/ppp/ppp_order_info >/dev/null 2>&1");
		system("rm /etc/ppp/hasPppoedevice >/dev/null 2>&1");
		system("rm /etc/ppp/AC_Names >/dev/null 2>&1");
		system("rm /etc/ppp/SubInfos >/dev/null 2>&1");	
		return ;
	}
	
#endif
	if(isFileExist(LAST_WAN_TYPE_FILE)){
		fp= fopen(LAST_WAN_TYPE_FILE, "r");
		if (!fp) {
	        	printf("can not /var/system/last_wan\n");
			return; 
	   	}
		fscanf(fp,"%d",&Last_WAN_Mode);
		fclose(fp);
	}
	RunSystemCmd("/var/disc", "echo", "enter", NULL_STR); 
	
//	apmib_get(MIB_WAN_DHCP,(void *)&wan_type);
	
	RunSystemCmd(NULL_FILE, "killall", "-15", "routed", NULL_STR); 
	
	RunSystemCmd(NULL_FILE, "killall", "-9", "ntp_inet", NULL_STR);
	if(isFileExist("/var/ntp_run")){
		unlink("/var/ntp_run");
	} 
	
	RunSystemCmd(NULL_FILE, "killall", "-15", "ddns_inet", NULL_STR); 
	RunSystemCmd(NULL_FILE, "killall", "-9", "updatedd", NULL_STR);
	RunSystemCmd(NULL_FILE, "killall", "-9", "ntpclient", NULL_STR);
	//RunSystemCmd("/proc/pptp_src_ip", "echo", "0 0", NULL_STR);
	
	#if	defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT) || defined(CONFIG_RTL_ULINKER)
		if(!strcmp(option, "all")){
	RunSystemCmd(NULL_FILE, "killall", "-9", "dnrd", NULL_STR);
	if(isFileExist(DNRD_PID_FILE)){
		unlink(DNRD_PID_FILE);
	}
	}
#if 0	//it is for pocket AP wan connect? but it has done the related operations in wan_connect() when IP conflict 
	else if(!strcmp(option, "dhcpc"))
	{
		unsigned char dynip[32]={0};
		struct in_addr	intaddr;
		unsigned char cmdBuffer[100]={0};
		unsigned char tmpBuff[200]={0};
		unsigned char domain_name[32]={0};
		
		if ( getInAddr("eth1", IP_ADDR_T, (void *)&intaddr ) )
			sprintf(dynip,"%s",inet_ntoa(intaddr));
		else
			sprintf(dynip,"%s","0.0.0.0");
			
		if(strcmp(dynip, "0.0.0.0") != 0) //do nothing at first time
		{
			system("echo \"WAN Disconnected\n\" > var/wanlink");
			system("killall -9 dnrd 2> /dev/null");
			system("rm -f /var/hosts 2> /dev/null");
			
			if ( getInAddr("br0", IP_ADDR_T, (void *)&intaddr ) )
				sprintf(dynip,"%s",inet_ntoa(intaddr));
			else
				sprintf(dynip,"%s","0.0.0.0");
			
			apmib_get( MIB_DOMAIN_NAME,  (void *)domain_name);
			sprintf(cmdBuffer,"%s\\%s%s%s%s", dynip, domain_name, "AP.com|",domain_name, "AP.net");
			//RunSystemCmd("/etc/hosts", "echo",cmdBuffer,NULL_STR);
			sprintf(tmpBuff, "echo \"%s\" > /etc/hosts", cmdBuffer);
			system(tmpBuff);
			
			system("ifconfig eth0 down");
			system("ifconfig wlan0 down");
	
			sleep(10);
			
			system("ifconfig eth0 up");
			system("ifconfig wlan0 up");
			
			system("dnrd --cache=off -s 168.95.1.1");
		}
	}
#endif
	else
	{
			if(isFileExist(PPPLINKFILE)){ //Last state, ppp0 is not connected, we do not kill dnrd
				RunSystemCmd(NULL_FILE, "killall", "-9", "dnrd", NULL_STR);
				if(isFileExist(DNRD_PID_FILE)){
					unlink(DNRD_PID_FILE);
				}
			}
		}
	#else

	RunSystemCmd(NULL_FILE, "killall", "-9", "dnrd", NULL_STR);
	if(isFileExist(DNRD_PID_FILE)){
		unlink(DNRD_PID_FILE);
	}
	#endif
	
	RunSystemCmd(NULL_FILE, "killall", "-9", "igmpproxy", NULL_STR);
	if(isFileExist(IGMPPROXY_PID_FILE)){
		unlink(IGMPPROXY_PID_FILE);
	}
	RunSystemCmd(PROC_BR_MCASTFASTFWD, "echo", "1,0", NULL_STR);
//	printf("Last_WAN_Mode==%d\n", Last_WAN_Mode);//Added for test
	if(!strcmp(option, "all"))
		RunSystemCmd(NULL_FILE, "killall", "-9", "ppp_inet", NULL_STR); 

	if(Last_WAN_Mode==PPPOE){
		RunSystemCmd(NULL_FILE, "killall", "-15", "pppd", NULL_STR);
	}else{
		RunSystemCmd(NULL_FILE, "killall", "-9", "pppd", NULL_STR);
	}
	sleep(3);

	if((wan_type!=L2TP)&&(Last_WAN_Mode==L2TP)){
		RunSystemCmd(NULL_FILE, "killall", "-9", "l2tpd", NULL_STR);
	}
	RunSystemCmd(NULL_FILE, "killall", "-9", "pptp", NULL_STR);
	RunSystemCmd(NULL_FILE, "killall", "-9", "pppoe", NULL_STR);
	if(isFileExist(PPPD_PID_FILE)){
		unlink(PPPD_PID_FILE);
	} 

	if(wan_type==L2TP && !strcmp(option, "option") && isFileExist(PPPLINKFILE)){
		apmib_get( MIB_L2TP_CONNECTION_TYPE, (void *)&intValue);
		if(intValue==1){
			if(isFileExist("/var/disc_l2tp")){
				system("echo\"d client\" > /var/run/l2tp-control &");
				system("echo \"l2tpdisc\" > /var/disc_l2tp");
			}
		}
	}
/*clean pptp_info in fastpptp*/
	if(wan_type==PPTP)
		system("echo 1 > /proc/fast_pptp");

	if(isFileExist(FIRSTDDNS)){
	 	unlink(FIRSTDDNS);
	}

	if(!strcmp(option, "option") && isFileExist(PPPLINKFILE)){
		RunSystemCmd(NULL_FILE, "rm", "-f", "/etc/ppp/first", NULL_STR);
		RunSystemCmd(NULL_FILE, "rm", "-f", "/etc/ppp/firstpptp", NULL_STR);
		RunSystemCmd(NULL_FILE, "rm", "-f", "/etc/ppp/firstl2tp", NULL_STR);
		RunSystemCmd(NULL_FILE, "rm", "-f", "/etc/ppp/firstdemand", NULL_STR);
	}
	if(isFileExist(PPPLINKFILE)){
	 	unlink(PPPLINKFILE);
	}
	/*in PPPOE and PPTP mode do this in pppd , not here !!*/
	if(wan_type !=PPPOE || strcmp(option, "option")){
		if(isFileExist(PPP_CONNECT_FILE)){
	 		unlink(PPP_CONNECT_FILE);
		}
	}
	if(wan_type==PPTP){
		apmib_get(MIB_PPTP_CONNECTION_TYPE, (void *)&intValue);
		if(intValue==1){
			RunSystemCmd(PROC_PPTP_CONN_FILE, "echo", "3", NULL_STR);
		}else{
			RunSystemCmd(PROC_PPTP_CONN_FILE, "echo", "0", NULL_STR);
		}
	}
	RunSystemCmd(NULL_FILE, "rm", "-f", "/var/disc", NULL_STR);
	RunSystemCmd(NULL_FILE, "rm", "-f", "/var/disc_l2tp", NULL_STR);
	
}

/*write dns server ip address to resolv.conf file and start dnrd
* 
*/
void start_dns_relay(void)
{
	char tmpBuff1[32]={0}, tmpBuff2[32]={0}, tmpBuff3[32]={0};
	int intValue=0;
	char line_buffer[100]={0};
	char tmp1[32]={0}, tmp2[32]={0}, tmp3[32]={0};
	char *strtmp=NULL;
	
	RunSystemCmd(NULL_FILE, "killall", "-9", "dnrd", NULL_STR);
	apmib_get( MIB_DNS1,  (void *)tmpBuff1);
	apmib_get( MIB_DNS2,  (void *)tmpBuff2);
	apmib_get( MIB_DNS3,  (void *)tmpBuff3);
	
	if (memcmp(tmpBuff1, "\x0\x0\x0\x0", 4))
		intValue++;
	if (memcmp(tmpBuff2, "\x0\x0\x0\x0", 4))
		intValue++;
	if (memcmp(tmpBuff3, "\x0\x0\x0\x0", 4))
		intValue++;	
			
	if(intValue==1){
		strtmp= inet_ntoa(*((struct in_addr *)tmpBuff1));
		sprintf(tmp1,"%s",strtmp);
		sprintf(line_buffer,"nameserver %s\n",strtmp);
		write_line_to_file(RESOLV_CONF,1, line_buffer);
		RunSystemCmd(NULL_FILE, "dnrd", "--cache=off", "-s", tmp1, NULL_STR);
		
	}else if(intValue==2){
		strtmp= inet_ntoa(*((struct in_addr *)tmpBuff1));
		sprintf(tmp1,"%s",strtmp);
		sprintf(line_buffer,"nameserver %s\n",strtmp);
		write_line_to_file(RESOLV_CONF,1, line_buffer);
		
		strtmp= inet_ntoa(*((struct in_addr *)tmpBuff2));
		sprintf(tmp2,"%s",strtmp);
		sprintf(line_buffer,"nameserver %s\n", strtmp);
		write_line_to_file(RESOLV_CONF,2, line_buffer);
		RunSystemCmd(NULL_FILE, "dnrd", "--cache=off", "-s", tmp1, "-s", tmp2, NULL_STR);
	}else if(intValue==3){
		strtmp= inet_ntoa(*((struct in_addr *)tmpBuff1));
		sprintf(tmp1,"%s",strtmp);
		sprintf(line_buffer,"nameserver %s\n",strtmp);
		write_line_to_file(RESOLV_CONF,1, line_buffer);
		
		strtmp= inet_ntoa(*((struct in_addr *)tmpBuff2));
		sprintf(tmp2,"%s",strtmp);
		sprintf(line_buffer,"nameserver %s\n", strtmp);
		write_line_to_file(RESOLV_CONF, 2, line_buffer);
		
		strtmp= inet_ntoa(*((struct in_addr *)tmpBuff3));
		sprintf(tmp3,"%s",strtmp);
		sprintf(line_buffer,"nameserver %s\n", strtmp);
		write_line_to_file(RESOLV_CONF, 2, line_buffer);
		
		RunSystemCmd(NULL_FILE, "dnrd", "--cache=off", "-s", tmp1, "-s", tmp2, "-s", tmp3, NULL_STR);
	}else{
		printf("Invalid DNS server setting\n");
	}	
}
void start_upnp_igd(int wantype, int sys_opmode, int wisp_id, char *lan_interface)
{
	int intValue=0;
	char tmp1[16]={0};
	char tmp2[16]={0};
	apmib_get(MIB_UPNP_ENABLED, (void *)&intValue);
	RunSystemCmd(NULL_FILE, "killall", "-15", "miniigd", NULL_STR); 
	if(intValue==1){
		RunSystemCmd(NULL_FILE, "route", "del", "-net", "239.255.255.250", "netmask", "255.255.255.255", lan_interface, NULL_STR); 
		RunSystemCmd(NULL_FILE, "route", "add", "-net", "239.255.255.250", "netmask", "255.255.255.255", lan_interface, NULL_STR); 
		sprintf(tmp1, "%d", wantype);
		sprintf(tmp2, "wlan%d", wisp_id);
		if(sys_opmode==WISP_MODE)
		{
#if defined(CONFIG_SMART_REPEATER)				
			getWispRptIfaceName(tmp2,wisp_id);
			//strcat(tmp2, "-vxd");
#endif			
			RunSystemCmd(NULL_FILE, "miniigd", "-e", tmp1, "-i", lan_interface, "-w", tmp2, NULL_STR); 
		}
		else	
		{
#ifdef MULTI_PPPOE
			int connnect_num;
			char str_connect[10];;
			apmib_get(MIB_PPP_CONNECT_COUNT, (void *)&connnect_num);
			sprintf(str_connect," %d",connnect_num);	
			if(PPPOE == wantype)
			{
				RunSystemCmd(NULL_FILE, "miniigd", "-e", tmp1, "-i", lan_interface,"-s",str_connect,NULL_STR);
			}
			else
			{
				RunSystemCmd(NULL_FILE, "miniigd", "-e", tmp1, "-i", lan_interface,NULL_STR);
			}
			
#else
			RunSystemCmd(NULL_FILE, "miniigd", "-e", tmp1, "-i", lan_interface,NULL_STR);
#endif
						
		}
		
	}
	
}
void start_ddns(void)
{
	unsigned int ddns_onoff;
	unsigned int ddns_type;
	unsigned char ddns_domanin_name[MAX_DOMAIN_LEN];
	unsigned char ddns_user_name[MAX_DOMAIN_LEN];
	unsigned char ddns_password[MAX_DOMAIN_LEN];
	
	RunSystemCmd(NULL_FILE, "killall", "-9", "ddns_inet", NULL_STR);
	
	apmib_get( MIB_DDNS_ENABLED,  (void *)&ddns_onoff);

	if(ddns_onoff == 1)
	{
		apmib_get( MIB_DDNS_TYPE,  (void *)&ddns_type);

		apmib_get( MIB_DDNS_DOMAIN_NAME,  (void *)ddns_domanin_name);

		apmib_get( MIB_DDNS_USER,  (void *)ddns_user_name);

		apmib_get( MIB_DDNS_PASSWORD,  (void *)ddns_password);		

		if(ddns_type == 0) // 0:ddns; 1:tzo
			RunSystemCmd(NULL_FILE, "ddns_inet", "-x", "dyndns", ddns_user_name, ddns_password, ddns_domanin_name, NULL_STR);
		else if(ddns_type == 1)
			RunSystemCmd(NULL_FILE, "ddns_inet", "-x", "tzo", ddns_user_name, ddns_password, ddns_domanin_name, NULL_STR);


	}

}

#define NTPTMP_FILE "/tmp/ntp_tmp"
#define TZ_FILE "/var/TZ"
void start_ntp(void)
{
	unsigned int ntp_onoff=0;
	unsigned char buffer[500];

	unsigned int ntp_server_id;
	char	ntp_server[40];
	unsigned int daylight_save = 1;
	char daylight_save_str[5];
	char time_zone[8];

	char command[100], str_datnight[100];
	unsigned char *str_tz1;
	
	apmib_get(MIB_NTP_ENABLED, (void *)&ntp_onoff);
	RunSystemCmd(NULL_FILE, "rm", NTPTMP_FILE, NULL_STR);
	RunSystemCmd(NULL_FILE, "rm", TZ_FILE, NULL_STR);
	if(ntp_onoff == 1)
	{
		RunSystemCmd(NULL_FILE, "echo", "Start NTP daemon", NULL_STR);
		/* prepare requested info for ntp daemon */
		apmib_get( MIB_NTP_SERVER_ID,  (void *)&ntp_server_id);

		if(ntp_server_id == 0)
			apmib_get( MIB_NTP_SERVER_IP1,  (void *)buffer);
		else
			apmib_get( MIB_NTP_SERVER_IP2,  (void *)buffer);

		sprintf(ntp_server, "%s", inet_ntoa(*((struct in_addr *)buffer)));

		apmib_get( MIB_DAYLIGHT_SAVE,  (void *)&daylight_save);
		memset(daylight_save_str, 0x00, sizeof(daylight_save_str));
		sprintf(daylight_save_str,"%u",daylight_save);
		
		apmib_get( MIB_NTP_TIMEZONE,  (void *)&time_zone);

		if(daylight_save == 0)
			sprintf( str_datnight, "%s", "");
		else if(strcmp(time_zone,"9 1") == 0)
			sprintf( str_datnight, "%s", "PDT,M4.1.0/02:00:00,M10.5.0/02:00:00");
		else if(strcmp(time_zone,"8 1") == 0)
			sprintf( str_datnight, "%s", "PDT,M4.1.0/02:00:00,M10.5.0/02:00:00");
		else if(strcmp(time_zone,"7 2") == 0)
                        sprintf( str_datnight, "%s", "PDT,M4.1.0/02:00:00,M10.5.0/02:00:00");
               else if(strcmp(time_zone,"6 1") == 0)
                        sprintf( str_datnight, "%s", "PDT,M4.1.0/02:00:00,M10.5.0/02:00:00");
               else if(strcmp(time_zone,"6 2") == 0)
                        sprintf( str_datnight, "%s", "PDT,M4.1.0/02:00:00,M10.5.0/02:00:00");
               else if(strcmp(time_zone,"5 2") == 0)
                        sprintf( str_datnight, "%s", "PDT,M4.1.0/02:00:00,M10.5.0/02:00:00");
               else if(strcmp(time_zone,"5 3") == 0)
                        sprintf( str_datnight, "%s", "PDT,M4.1.0/02:00:00,M10.5.0/02:00:00");
               else if(strcmp(time_zone,"4 3") == 0)
                        sprintf( str_datnight, "%s", "PDT,M10.2.0/00:00:00,M3.2.0/00:00:00");
               else if(strcmp(time_zone,"3 1") == 0)
                        sprintf( str_datnight, "%s", "PDT,M4.1.0/00:00:00,M10.5.0/00:00:00");
               else if(strcmp(time_zone,"3 2") == 0)
                        sprintf( str_datnight, "%s", "PDT,M2.2.0/00:00:00,M10.2.0/00:00:00");
               else if(strcmp(time_zone,"1 1") == 0)
                        sprintf( str_datnight, "%s", "PDT,M3.5.0/00:00:00,M10.5.0/01:00:00");
               else if(strcmp(time_zone,"0 2") == 0)
                        sprintf( str_datnight, "%s", "PDT,M3.5.0/01:00:00,M10.5.0/02:00:00");
               else if(strcmp(time_zone,"-1") == 0)
                        sprintf( str_datnight, "%s", "PDT,M3.5.0/02:00:00,M10.5.0/03:00:00");
               else if(strcmp(time_zone,"-2 1") == 0)
                        sprintf( str_datnight, "%s", "PDT,M3.5.0/02:00:00,M10.5.0/03:00:00");
               else if(strcmp(time_zone,"-2 2") == 0)
                        sprintf( str_datnight, "%s", "PDT,M3.5.0/03:00:00,M10.5.0/04:00:00");
               else if(strcmp(time_zone,"-2 3") == 0)
                        sprintf( str_datnight, "%s", "PDT,M4.5.5/00:00:00,M9.5.5/00:00:00");
               else if(strcmp(time_zone,"-2 5") == 0)
                        sprintf( str_datnight, "%s", "PDT,M3.5.0/03:00:00,M10.5.5/04:00:00");
               else if(strcmp(time_zone,"-2 6") == 0)
                        sprintf( str_datnight, "%s", "PDT,M3.5.5/02:00:00,M10.1.0/02:00:00");
               else if(strcmp(time_zone,"-3 2") == 0)
                        sprintf( str_datnight, "%s", "PDT,M3.5.0/02:00:00,M10.5.0/03:00:00");
               else if(strcmp(time_zone,"-4 2") == 0)
                        sprintf( str_datnight, "%s", "PDT,M3.5.0/04:00:00,M10.5.0/05:00:00");
               else if(strcmp(time_zone,"-9 4") == 0)
                        sprintf( str_datnight, "%s", "PDT,M10.5.0/02:00:00,M4.1.0/03:00:00");
               else if(strcmp(time_zone,"-10 2") == 0)
                        sprintf( str_datnight, "%s", "PDT,M10.5.0/02:00:00,M4.1.0/03:00:00");
               else if(strcmp(time_zone,"-10 4") == 0)
                        sprintf( str_datnight, "%s", "PDT,M10.1.0/02:00:00,M4.1.0/03:00:00");
               else if(strcmp(time_zone,"-10 5") == 0)
                        sprintf( str_datnight, "%s", "PDT,M3.5.0/02:00:00,M10.5.0/03:00:00");
               else if(strcmp(time_zone,"-12 1") == 0)
                        sprintf( str_datnight, "%s", "PDT,M3.2.0/03:00:00,M10.1.0/02:00:00");
               else
                        sprintf( str_datnight, "%s", "");

		str_tz1 = gettoken((unsigned char*)time_zone, 0, ' ');
		
		if(strcmp(time_zone,"3 1") == 0 ||
			strcmp(time_zone,"-3 4") == 0 ||
		 	strcmp(time_zone,"-4 3") == 0 ||
		 	strcmp(time_zone,"-5 3") == 0 ||
		 	strcmp(time_zone,"-9 4") == 0 ||
		 	strcmp(time_zone,"-9 5") == 0
		)
		{
                       sprintf( command, "GMT%s:30%s", str_tz1, str_datnight);
		}
		else
			sprintf( command, "GMT%s%s", str_tz1, str_datnight); 
		
		RunSystemCmd(NULL_FILE, "ntp_inet", "-x", ntp_server, command, daylight_save_str, NULL_STR);
}



}

#if defined(ROUTE_SUPPORT)
void del_routing(void)
{
	int intValue=0, i;
	char	ip[32], netmask[32], gateway[32], *tmpStr=NULL;	
	int entry_Num=0;
	STATICROUTE_T entry;
	int rip_enabled=0;
	
	apmib_get(MIB_STATICROUTE_TBL_NUM, (void *)&entry_Num);
	if(entry_Num > 0){
		for (i=1; i<=entry_Num; i++) {
			*((char *)&entry) = (char)i;
			apmib_get(MIB_STATICROUTE_TBL, (void *)&entry);
	
			tmpStr = inet_ntoa(*((struct in_addr *)entry.dstAddr));
			sprintf(ip, "%s", tmpStr);
			tmpStr = inet_ntoa(*((struct in_addr *)entry.netmask));
			sprintf(netmask, "%s", tmpStr);
			tmpStr = inet_ntoa(*((struct in_addr *)entry.gateway));
			sprintf(gateway, "%s", tmpStr);
			
			RunSystemCmd(NULL_FILE, "route", "del", "-net", ip, "netmask", netmask, "gw",  gateway, NULL_STR);
		}
	}

	apmib_get(MIB_RIP_ENABLED, (void *)&rip_enabled);
	if(rip_enabled)
	{
		RunSystemCmd(PROC_BR_IGMPDB, "echo", "add all ipv4 224.0.0.9 0xffffffff", NULL_STR);
	}
	else
	{
		RunSystemCmd(PROC_BR_IGMPDB, "echo", "del all ipv4 224.0.0.9 0xffffffff", NULL_STR);
	}
	
}
void start_routing(char *interface)
{
	int intValue=0, i;
	char line_buffer[64]={0};
	char tmp_args[16]={0};
	char	ip[32], netmask[32], gateway[32], *tmpStr=NULL;	
	int entry_Num=0;
	STATICROUTE_T entry;
	int nat_enabled=0, rip_enabled=0, rip_wan_tx=0;
	int rip_wan_rx=0, rip_lan_tx=0, rip_lan_rx=0;
	int start_routed=1;
	
	RunSystemCmd(NULL_FILE, "killall", "-15", "routed", NULL_STR); 
	apmib_get(MIB_NAT_ENABLED, (void *)&nat_enabled);
	apmib_get(MIB_RIP_ENABLED, (void *)&rip_enabled);
	apmib_get(MIB_RIP_LAN_TX, (void *)&rip_lan_tx);
	apmib_get(MIB_RIP_LAN_RX, (void *)&rip_lan_rx);
	apmib_get(MIB_RIP_WAN_TX, (void *)&rip_wan_tx);
	apmib_get(MIB_RIP_WAN_RX, (void *)&rip_wan_rx);
	line_buffer[0]=0x0d;
	line_buffer[1]=0x0a;
	write_line_to_file(ROUTED_CONF_FILE,1, line_buffer);
	memset(line_buffer, 0x00, 64);
	if(nat_enabled==0){
		if(rip_lan_tx !=0 && rip_lan_rx==0){
			sprintf(line_buffer,"network br0 0 %d\n",rip_lan_tx);
			write_line_to_file(ROUTED_CONF_FILE, 2 , line_buffer);
			sprintf(line_buffer,"network %s 0 %d\n",interface, rip_lan_tx);
			write_line_to_file(ROUTED_CONF_FILE, 2 , line_buffer);
			
		}else if(rip_lan_tx !=0 && rip_lan_rx !=0){
				sprintf(line_buffer,"network br0 %d %d\n",rip_lan_rx, rip_lan_tx);
				write_line_to_file(ROUTED_CONF_FILE, 2 , line_buffer);
				sprintf(line_buffer,"network %s %d %d\n",interface, rip_lan_rx, rip_lan_tx);
				write_line_to_file(ROUTED_CONF_FILE, 2 , line_buffer);
			
		}else{
			if( rip_lan_rx !=0){
				sprintf(line_buffer,"network br0 %d 0\n",rip_lan_rx);
				write_line_to_file(ROUTED_CONF_FILE, 2 , line_buffer);
				sprintf(line_buffer,"network %s %d 0\n",interface, rip_lan_rx);
				write_line_to_file(ROUTED_CONF_FILE, 2 , line_buffer);
			}else
				start_routed=0;
		}
	}else{
		if( rip_lan_rx !=0){
			sprintf(line_buffer,"network br0 %d 0\n",rip_lan_rx);
			write_line_to_file(ROUTED_CONF_FILE, 2 , line_buffer);
			sprintf(line_buffer,"network %s %d 0\n",interface, rip_lan_rx);
			write_line_to_file(ROUTED_CONF_FILE, 2 , line_buffer);
		}else
			start_routed=0;
	}
	apmib_get(MIB_STATICROUTE_ENABLED, (void *)&intValue);
	apmib_get(MIB_STATICROUTE_TBL_NUM, (void *)&entry_Num);
	if(intValue > 0 && entry_Num > 0){
		for (i=1; i<=entry_Num; i++) {
			*((char *)&entry) = (char)i;
			apmib_get(MIB_STATICROUTE_TBL, (void *)&entry);
	
			tmpStr = inet_ntoa(*((struct in_addr *)entry.dstAddr));
			sprintf(ip, "%s", tmpStr);
			tmpStr = inet_ntoa(*((struct in_addr *)entry.netmask));
			sprintf(netmask, "%s", tmpStr);
			tmpStr = inet_ntoa(*((struct in_addr *)entry.gateway));
			sprintf(gateway, "%s", tmpStr);
			sprintf(tmp_args, "%d", entry.metric);
			if(!strcmp(interface, "ppp0")){
				if(entry.interface==1){//wan interface
					RunSystemCmd(NULL_FILE, "route", "add", "-net", ip, "netmask", netmask, "metric", tmp_args, "dev", interface,  NULL_STR);
				}else{
					RunSystemCmd(NULL_FILE, "route", "add", "-net", ip, "netmask", netmask, "gw",  gateway, "metric", tmp_args, "dev", "br0",  NULL_STR);
				}
			}else{
				if(entry.interface==1){//wan interface
					RunSystemCmd(NULL_FILE, "route", "add", "-net", ip, "netmask", netmask, "gw",  gateway, "metric", tmp_args, "dev", interface,  NULL_STR);
				}else if(entry.interface==0){
					RunSystemCmd(NULL_FILE, "route", "add", "-net", ip, "netmask", netmask, "gw",  gateway, "metric", tmp_args, "dev", "br0",  NULL_STR);
				}
			}
		}
	}
	
	if(rip_enabled !=0 && start_routed==1)
		RunSystemCmd(NULL_FILE, "routed", "-s",  NULL_STR);
	
	if(nat_enabled==0){
		if(isFileExist(IGMPPROXY_PID_FILE)){
			unlink(IGMPPROXY_PID_FILE);
		}
		RunSystemCmd(NULL_FILE, "killall", "-9", "igmpproxy", NULL_STR);
		RunSystemCmd(PROC_BR_MCASTFASTFWD, "echo", "1,0", NULL_STR);
	}

	if(rip_enabled)
	{
		RunSystemCmd(PROC_BR_IGMPDB, "echo", "add all ipv4 224.0.0.9 0xffffffff", NULL_STR);
	}
	else
	{
		RunSystemCmd(PROC_BR_IGMPDB, "echo", "del all ipv4 224.0.0.9 0xffffffff", NULL_STR);
	}
}
#endif

void start_igmpproxy(char *wan_iface, char *lan_iface)
{
	int intValue=0;
	apmib_get(MIB_IGMP_PROXY_DISABLED, (void *)&intValue);
	RunSystemCmd(NULL_FILE, "killall", "-9", "igmpproxy", NULL_STR);
	RunSystemCmd(PROC_BR_MCASTFASTFWD, "echo", "1,0", NULL_STR);
	if(intValue==0) {
		RunSystemCmd(NULL_FILE, "igmpproxy", wan_iface, lan_iface, NULL_STR);
		RunSystemCmd(PROC_IGMP_MAX_MEMBERS, "echo", "128", NULL_STR);
		RunSystemCmd(PROC_BR_MCASTFASTFWD, "echo", "1,1", NULL_STR);
	}
	
}
void start_wan_dhcp_client(char *iface)
{
	char hostname[100];
	char cmdBuff[200];
	char script_file[100], deconfig_script[100], pid_file[100];
	
	sprintf(script_file, "/usr/share/udhcpc/%s.sh", iface); /*script path*/
	sprintf(deconfig_script, "/usr/share/udhcpc/%s.deconfig", iface);/*deconfig script path*/
	sprintf(pid_file, "/etc/udhcpc/udhcpc-%s.pid", iface); /*pid path*/
	killDaemonByPidFile(pid_file);
	Create_script(deconfig_script, iface, WAN_NETWORK, 0, 0, 0);
	memset(hostname, 0x00, 100);
	apmib_get( MIB_HOST_NAME, (void *)&hostname);

	if(hostname[0]){
		sprintf(cmdBuff, "udhcpc -i %s -p %s -s %s -h %s -a 30 &", iface, pid_file, script_file, hostname);
		//RunSystemCmd(NULL_FILE, "udhcpc", "-i", iface, "-p", pid_file, "-s", script_file,  "-a", "30", "-h", hostname,  NULL_STR);
	}else{
		sprintf(cmdBuff, "udhcpc -i %s -p %s -s %s -a 30 &", iface, pid_file, script_file);
		//RunSystemCmd(NULL_FILE, "udhcpc", "-i", iface, "-p", pid_file, "-s", script_file,  "-a", "30", NULL_STR);
	}
	system(cmdBuff);
}
void set_staticIP(int sys_op, char *wan_iface, char *lan_iface, int wisp_id, int act_source)
{
	int intValue=0;
	char tmpBuff[200];
	char tmp_args[16];
	char Ip[32], Mask[32], Gateway[32];
	
	apmib_get( MIB_WAN_IP_ADDR,  (void *)tmpBuff);
	sprintf(Ip, "%s", inet_ntoa(*((struct in_addr *)tmpBuff)));
	apmib_get( MIB_WAN_SUBNET_MASK,  (void *)tmpBuff);
	sprintf(Mask, "%s", inet_ntoa(*((struct in_addr *)tmpBuff)));
	apmib_get(MIB_WAN_DEFAULT_GATEWAY,  (void *)tmpBuff);
				
	if (!memcmp(tmpBuff, "\x0\x0\x0\x0", 4))
		memset(Gateway, 0x00, 32);
	else
		sprintf(Gateway, "%s", inet_ntoa(*((struct in_addr *)tmpBuff)));
			
	RunSystemCmd(NULL_FILE, "ifconfig", wan_iface, Ip, "netmask", Mask, NULL_STR);
		
	if(Gateway[0]){
		RunSystemCmd(NULL_FILE, "route", "del", "default", wan_iface, NULL_STR);
		RunSystemCmd(NULL_FILE, "route", "add", "-net", "default", "gw", Gateway, "dev", wan_iface, NULL_STR);
	}	
		apmib_get(MIB_FIXED_IP_MTU_SIZE, (void *)&intValue);
		sprintf(tmp_args, "%d", intValue);
		RunSystemCmd(NULL_FILE, "ifconfig", wan_iface, "mtu", tmp_args, NULL_STR);
		//RunSystemCmd(NULL_FILE, "killall", "-9", "dnrd", NULL_STR);
		start_dns_relay();
		start_upnp_igd(DHCP_DISABLED, sys_op, wisp_id, lan_iface);
		setFirewallIptablesRules(0, NULL);
		
		start_ntp();
		start_ddns();
		start_igmpproxy(wan_iface, lan_iface);
#if defined(ROUTE_SUPPORT)
		del_routing();
		start_routing(wan_iface);
#endif

#ifdef SEND_GRATUITOUS_ARP
		//char tmpBuf[128];
		snprintf(tmpBuff, 128, "%s/%s %s", _CONFIG_SCRIPT_PATH, _FIREWALL_SCRIPT_PROG, "Send_GARP"); 	
		//printf("CMD is : %s \n", tmpBuff);
		system(tmpBuff);
#endif

}
void set_dhcp_client(int sys_op, char *wan_iface, char *lan_iface, int wisp_id, int act_source)
{
	int intValue=0;
	char tmp_args[16];
	
	apmib_get(MIB_DHCP_MTU_SIZE, (void *)&intValue);
	sprintf(tmp_args, "%d", intValue);
	RunSystemCmd(NULL_FILE, "ifconfig", wan_iface, "mtu", tmp_args, NULL_STR);
	start_wan_dhcp_client(wan_iface);
	start_upnp_igd(DHCP_CLIENT, sys_op, wisp_id, lan_iface);
}
void set_pppoe(int sys_op, char *wan_iface, char *lan_iface, int wisp_id, int act_source)
{
	int intValue=0, cmdRet=-1;
//	int intValue1=0;
	char line_buffer[100]={0};
	char tmp_args[64]={0};
	char tmp_args1[32]={0};
	int connect_type=0, idle_time=0;
#ifdef MULTI_PPPOE
	FILE *pAC , *PSubNet;
	int connnect_num,index;
	char AC_Name[40];
	char SubNet[40];
	char command[100];
		char* wan_interface[] = {"eth1","eth5"};
	char* order2Name[] = {"FIRST","SECOND","THIRD","FORTH"};
	//dzh 2011-12-21
	system("echo eth1 br0 172.29.17.10 172.29.17.11 >> /etc/dnrd/dns_config");
	system("echo eth5 br0 172.29.17.10 172.29.17.11 >> /etc/dnrd/dns_config");	
	//dzh end
	
	char* pppoe_file_list[4][3]=
	{
		{"/etc/ppp/pap-secrets","/etc/ppp/chap-secrets","/etc/ppp/options"},
		{"/etc/ppp/pap-secrets2","/etc/ppp/chap-secrets2","/etc/ppp/options2"},
		{"/etc/ppp/pap-secrets3","/etc/ppp/chap-secrets3","/etc/ppp/options3"},
		{"/etc/ppp/pap-secrets4","/etc/ppp/chap-secrets4","/etc/ppp/options4"}	
	};
	apmib_get(MIB_PPP_CONNECT_COUNT, (void *)&connnect_num);
	sprintf(command,"echo %d > /etc/ppp/ppp_connect_number",connnect_num);
	system(command);
	
	if(isFileExist("/etc/ppp/AC_Names"))
		unlink("/etc/ppp/AC_Names");
	
	if(isFileExist("/etc/ppp/SubInfos"))
		unlink("/etc/ppp/SubInfos");
	
	pAC = fopen("/etc/ppp/AC_Names","w+");
	PSubNet = fopen("/etc/ppp/SubInfos","w+");
	
	fprintf(pAC,"%d\n",connnect_num);
	fprintf(PSubNet,"%d\n",connnect_num);			
	for(index = 0 ; index < connnect_num ; ++index)
	{
		if(0 == index)
		{
			apmib_get(MIB_PPP_SERVICE_NAME, (void *)&AC_Name);	
			apmib_get(MIB_PPP_SUBNET1, (void *)&SubNet);	
		}
		else if(1 == index)
		{
			apmib_get(MIB_PPP_SERVICE_NAME2, (void *)&AC_Name);
			apmib_get(MIB_PPP_SUBNET2, (void *)&SubNet);				
		}
		else if(2 == index)
		{	
			apmib_get(MIB_PPP_SERVICE_NAME3, (void *)&AC_Name);	
			apmib_get(MIB_PPP_SUBNET3, (void *)&SubNet);				
		}
		else if(3 == index)
		{
			apmib_get(MIB_PPP_SERVICE_NAME4, (void *)&AC_Name);
			apmib_get(MIB_PPP_SUBNET4, (void *)&SubNet);				
		}
		fprintf(pAC,"%s\n",AC_Name);
		fprintf(PSubNet,"%s\n",SubNet);	
	}

	close(pAC);
	close(PSubNet);
#endif
	RunSystemCmd(NULL_FILE, "ifconfig", wan_iface, "0.0.0.0", NULL_STR);
//	RunSystemCmd(NULL_FILE, "route", "del", "default", "gw", "0.0.0.0", NULL_STR);
//	cmdRet = RunSystemCmd(NULL_FILE, "flash", "gen-pppoe", PPP_OPTIONS_FILE, PPP_PAP_FILE, PPP_CHAP_FILE,NULL_STR);
#ifdef MULTI_PPPOE
	for(index = 0 ;index < connnect_num ;++index)
	{
		cmdRet = RunSystemCmd(NULL_FILE, "flash", "gen-pppoe",
			pppoe_file_list[index][2], 
			pppoe_file_list[index][0],
			pppoe_file_list[index][1], 
			order2Name[index] , NULL_STR);
		if(cmdRet==0){
			sprintf(line_buffer,"%s\n", "noauth");
			write_line_to_file(pppoe_file_list[index][2],2, line_buffer);
			sprintf(line_buffer,"%s\n", "nomppc");
			write_line_to_file(pppoe_file_list[index][2],2, line_buffer);
			sprintf(line_buffer,"%s\n", "noipdefault");
			write_line_to_file(pppoe_file_list[index][2],2, line_buffer);
			sprintf(line_buffer,"%s\n", "hide-password");
			write_line_to_file(pppoe_file_list[index][2],2, line_buffer);
			sprintf(line_buffer,"%s\n", "defaultroute");
			write_line_to_file(pppoe_file_list[index][2],2, line_buffer);
			sprintf(line_buffer,"%s\n", "persist");
			write_line_to_file(pppoe_file_list[index][2],2, line_buffer);
			sprintf(line_buffer,"%s\n", "ipcp-accept-remote");
			write_line_to_file(pppoe_file_list[index][2],2, line_buffer);
			sprintf(line_buffer,"%s\n", "ipcp-accept-local");
			write_line_to_file(pppoe_file_list[index][2],2, line_buffer);
			sprintf(line_buffer,"%s\n", "nodetach");
			write_line_to_file(pppoe_file_list[index][2],2, line_buffer);
			sprintf(line_buffer,"%s\n", "usepeerdns");
			write_line_to_file(pppoe_file_list[index][2],2, line_buffer);
			
			if(0 == index)
			{
				apmib_get(MIB_PPP_MTU_SIZE, (void *)&intValue);
				apmib_get(MIB_PPP_CONNECT_TYPE, (void *)&connect_type);
				apmib_get(MIB_PPP_IDLE_TIME, (void *)&idle_time);
			//	apmib_get( MIB_PPP_SERVICE_NAME,  (void *)tmp_args);
			}
			else if(1 == index)
			{
				apmib_get(MIB_PPP_MTU_SIZE2, (void *)&intValue);
				apmib_get(MIB_PPP_CONNECT_TYPE2, (void *)&connect_type);
				apmib_get(MIB_PPP_IDLE_TIME, (void *)&idle_time);				
			//	apmib_get( MIB_PPP_SERVICE_NAME2,  (void *)tmp_args);
			}
			else if(2 == index)
			{
				apmib_get(MIB_PPP_MTU_SIZE3, (void *)&intValue);
				apmib_get(MIB_PPP_CONNECT_TYPE3, (void *)&connect_type);
				apmib_get(MIB_PPP_IDLE_TIME3, (void *)&idle_time);				
			//	apmib_get( MIB_PPP_SERVICE_NAME3,  (void *)tmp_args);
			}
			else if(3 == index)
			{
				apmib_get(MIB_PPP_MTU_SIZE4, (void *)&intValue);
				apmib_get(MIB_PPP_CONNECT_TYPE4, (void *)&connect_type);
				apmib_get(MIB_PPP_IDLE_TIME4, (void *)&idle_time);				
			//	apmib_get( MIB_PPP_SERVICE_NAME4,  (void *)tmp_args);
			}
			
			sprintf(line_buffer,"mtu %d\n", intValue);
			write_line_to_file(pppoe_file_list[index][2],2, line_buffer);
			sprintf(line_buffer,"mru %d\n", intValue);
			write_line_to_file(pppoe_file_list[index][2],2, line_buffer);
			sprintf(line_buffer,"%s\n", "lcp-echo-interval 20");
			write_line_to_file(pppoe_file_list[index][2],2, line_buffer);
			sprintf(line_buffer,"%s\n", "lcp-echo-failure 3");
			write_line_to_file(pppoe_file_list[index][2],2, line_buffer);
			sprintf(line_buffer,"%s\n", "wantype 3");
			write_line_to_file(pppoe_file_list[index][2],2, line_buffer);
			sprintf(line_buffer,"%s\n", "holdoff 10");
			write_line_to_file(pppoe_file_list[index][2],2, line_buffer);
			//apmib_get( MIB_PPP_SERVICE_NAME,  (void *)tmp_args);
			wan_iface = wan_interface[index];
			if(tmp_args[0]){
				//sprintf(line_buffer,"plugin /etc/ppp/plubins/libplugin.a rp_pppoe_ac 62031090091393-Seednet_240_58 rp_pppoe_service %s %s\n",tmp_args, wan_iface);
				sprintf(line_buffer,"plugin /etc/ppp/plubins/libplugin.a rp_pppoe_service %s %s\n",tmp_args, wan_iface);
			}else{
				sprintf(line_buffer,"plugin /etc/ppp/plubins/libplugin.a %s\n", wan_iface);
			}
			write_line_to_file(pppoe_file_list[index][2],2, line_buffer);
			
			//apmib_get(MIB_PPP_CONNECT_TYPE, (void *)&connect_type);
			if(connect_type==1){
				//apmib_get(MIB_PPP_IDLE_TIME, (void *)&idle_time);
				sprintf(line_buffer,"%s\n", "demand");
				write_line_to_file(pppoe_file_list[index][2],2, line_buffer);
				sprintf(line_buffer,"idle %d\n", idle_time);
				write_line_to_file(pppoe_file_list[index][2],2, line_buffer);
			}else if(connect_type==2 && act_source==1) //manual mode we do not dial up from init.sh
					return;				
		}
		
	
	}
#else
	cmdRet = RunSystemCmd(NULL_FILE, "flash", "gen-pppoe", PPP_OPTIONS_FILE1, PPP_PAP_FILE1, PPP_CHAP_FILE1,NULL_STR);
	if(cmdRet==0){
		sprintf(line_buffer,"%s\n", "noauth");
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
		sprintf(line_buffer,"%s\n", "nomppc");
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
		sprintf(line_buffer,"%s\n", "noipdefault");
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
		sprintf(line_buffer,"%s\n", "hide-password");
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
		sprintf(line_buffer,"%s\n", "defaultroute");
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
		sprintf(line_buffer,"%s\n", "persist");
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
		sprintf(line_buffer,"%s\n", "ipcp-accept-remote");
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
		sprintf(line_buffer,"%s\n", "ipcp-accept-local");
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
		sprintf(line_buffer,"%s\n", "nodetach");
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
		sprintf(line_buffer,"%s\n", "usepeerdns");
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
		
		apmib_get(MIB_PPP_MTU_SIZE, (void *)&intValue);
		sprintf(line_buffer,"mtu %d\n", intValue);
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
		sprintf(line_buffer,"mru %d\n", intValue);
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
		sprintf(line_buffer,"%s\n", "lcp-echo-interval 20");
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
		sprintf(line_buffer,"%s\n", "lcp-echo-failure 3");
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
		sprintf(line_buffer,"%s\n", "wantype 3");
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
		sprintf(line_buffer,"%s\n", "holdoff 10");
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
		apmib_get( MIB_PPP_SERVICE_NAME,  (void *)tmp_args);
		if(tmp_args[0]){
			//sprintf(line_buffer,"plugin /etc/ppp/plubins/libplugin.a rp_pppoe_ac 62031090091393-Seednet_240_58 rp_pppoe_service %s %s\n",tmp_args, wan_iface);
			sprintf(line_buffer,"plugin /etc/ppp/plubins/libplugin.a rp_pppoe_service %s %s\n",tmp_args, wan_iface);
		}else{
			sprintf(line_buffer,"plugin /etc/ppp/plubins/libplugin.a %s\n", wan_iface);
		}
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
		
		apmib_get(MIB_PPP_CONNECT_TYPE, (void *)&connect_type);
		if(connect_type==1){
			apmib_get(MIB_PPP_IDLE_TIME, (void *)&idle_time);
			sprintf(line_buffer,"%s\n", "demand");
			write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
			sprintf(line_buffer,"idle %d\n", idle_time);
			write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
		}else if(connect_type==2 && act_source==1) //manual mode we do not dial up from init.sh
				return;		
	}
		

#endif
/*
		apmib_get(MIB_PPP_CONNECT_TYPE, (void *)&connect_type);
		if(connect_type==1){
			apmib_get(MIB_PPP_IDLE_TIME, (void *)&idle_time);
			sprintf(line_buffer,"%s\n", "demand");
			write_line_to_file(PPP_OPTIONS_FILE,2, line_buffer);
			sprintf(line_buffer,"idle %d\n", idle_time);
			write_line_to_file(PPP_OPTIONS_FILE,2, line_buffer);
		}else if(connect_type==2 && act_source==1) //manual mode we do not dial up from init.sh
				return;
*/			
	#if 0
		apmib_get( MIB_DNS_MODE, (void *)&intValue1);
		if(intValue1==1){
			start_dns_relay();
		}else{
			RunSystemCmd(NULL_FILE, "dnrd", "--cache=off", "-s", "168.95.1.1",NULL_STR);
		}
	#endif
		if(isFileExist(PPP_FILE)){
			unlink(PPP_FILE);
		} 
		sprintf(tmp_args, "%s", "3");/*wan type*/
		sprintf(tmp_args1, "%d", connect_type);/*connect type*/
		RunSystemCmd(NULL_FILE, "ppp_inet", "-t", tmp_args,  "-c", tmp_args1, "-x", NULL_STR);
		start_upnp_igd(PPPOE, sys_op, wisp_id, lan_iface);
}
void set_pptp(int sys_op, char *wan_iface, char *lan_iface, int wisp_id, int act_source)
{
	int intValue=0, intValue1=0, cmdRet=-1;
	char line_buffer[100]={0};
	char tmp_args[64]={0};
	char tmp_args1[32]={0};
	char Ip[32], Mask[32], ServerIp[32];
	int connect_type=0, idle_time=0;
	char *strtmp=NULL;
#if defined(CONFIG_DYNAMIC_WAN_IP)
	char pptpDefGw[32], netIp[32];
	unsigned int ipAddr, netAddr, netMask, serverAddr;
	int pptp_wanip_dynamic=0;
	

	apmib_get(MIB_PPTP_WAN_IP_DYNAMIC, (void *)&pptp_wanip_dynamic);

	apmib_get(MIB_PPTP_SERVER_IP_ADDR,  (void *)tmp_args);
	strtmp= inet_ntoa(*((struct in_addr *)tmp_args));
	sprintf(ServerIp, "%s", strtmp);
	serverAddr=((struct in_addr *)tmp_args)->s_addr;
	
	if(pptp_wanip_dynamic==STATIC_IP){	//pptp use static wan ip
	apmib_get(MIB_PPTP_DEFAULT_GW,  (void *)tmp_args);
	strtmp= inet_ntoa(*((struct in_addr *)tmp_args));
	sprintf(pptpDefGw, "%s", strtmp);
#else
	apmib_get(MIB_PPTP_SERVER_IP_ADDR,  (void *)tmp_args);
	strtmp= inet_ntoa(*((struct in_addr *)tmp_args));
	sprintf(ServerIp, "%s", strtmp);	
#endif
	apmib_get(MIB_PPTP_IP_ADDR,  (void *)tmp_args);
	strtmp= inet_ntoa(*((struct in_addr *)tmp_args));
	sprintf(Ip, "%s", strtmp);	
#if defined(CONFIG_DYNAMIC_WAN_IP)
	ipAddr=((struct in_addr *)tmp_args)->s_addr;
#endif	

	apmib_get(MIB_PPTP_SUBNET_MASK,  (void *)tmp_args);
	strtmp= inet_ntoa(*((struct in_addr *)tmp_args));
	sprintf(Mask, "%s", strtmp);
#if defined(CONFIG_DYNAMIC_WAN_IP)
	netMask=((struct in_addr *)tmp_args)->s_addr;
#endif
	
	RunSystemCmd(NULL_FILE, "ifconfig", wan_iface, Ip, "netmask", Mask, NULL_STR);
	RunSystemCmd(NULL_FILE, "route", "del", "default", "gw", "0.0.0.0", NULL_STR);
#if defined(CONFIG_DYNAMIC_WAN_IP)
		if((serverAddr & netMask) != (ipAddr & netMask)){
			//Patch for our router under another router to dial up pptp
			//let pptp dialing pkt via pptp default gateway
			netAddr = (serverAddr & netMask);
			((struct in_addr *)tmp_args)->s_addr=netAddr;
			strtmp= inet_ntoa(*((struct in_addr *)tmp_args));
			sprintf(netIp, "%s", strtmp);
			RunSystemCmd(NULL_FILE, "route", "add", "-net", netIp, "netmask", Mask,"gw", pptpDefGw,NULL_STR);
		}
	} //end for pptp use static wan ip
#endif
	cmdRet = RunSystemCmd(NULL_FILE, "flash", "gen-pptp", PPP_OPTIONS_FILE1, PPP_PAP_FILE1, PPP_CHAP_FILE1,NULL_STR);
	
	if(cmdRet==0){
		sprintf(line_buffer,"%s\n", "lock");
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
		
		sprintf(line_buffer,"%s\n", "noauth");
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
		
		sprintf(line_buffer,"%s\n", "nobsdcomp");
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
		
		sprintf(line_buffer,"%s\n", "nodeflate");
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
		
		sprintf(line_buffer,"%s\n", "usepeerdns");
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
		
		sprintf(line_buffer,"%s\n", "lcp-echo-interval 20");
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
		
		sprintf(line_buffer,"%s\n", "lcp-echo-failure 3");
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
		
		sprintf(line_buffer,"%s\n", "wantype 4");
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
		
		apmib_get(MIB_PPTP_MTU_SIZE, (void *)&intValue);
		sprintf(line_buffer,"mtu %d\n", intValue);
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
		
		sprintf(line_buffer,"%s\n", "holdoff 2");
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
		
		sprintf(line_buffer,"%s\n", "refuse-eap");
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
		
		sprintf(line_buffer,"%s\n", "remotename PPTP");
		write_line_to_file(PPTP_PEERS_FILE,1, line_buffer);
		
		sprintf(line_buffer,"%s\n", "linkname PPTP");
		write_line_to_file(PPTP_PEERS_FILE,2, line_buffer);
		
		sprintf(line_buffer,"%s\n", "ipparam PPTP");
		write_line_to_file(PPTP_PEERS_FILE,2, line_buffer);
		
		sprintf(tmp_args, "pty \"pptp %s --nolaunchpppd\"", ServerIp);
		sprintf(line_buffer,"%s\n", tmp_args);
		write_line_to_file(PPTP_PEERS_FILE,2, line_buffer);
		
		apmib_get( MIB_PPTP_USER_NAME,  (void *)tmp_args);
		sprintf(line_buffer,"name %s\n", tmp_args);
		write_line_to_file(PPTP_PEERS_FILE,2, line_buffer);
		
		apmib_get( MIB_PPTP_SECURITY_ENABLED, (void *)&intValue);
		if(intValue==1){
			sprintf(line_buffer,"%s\n", "+mppe required,stateless");
			write_line_to_file(PPTP_PEERS_FILE,2, line_buffer);
			
			//sprintf(line_buffer,"%s\n", "+mppe no128,stateless");/*disable 128bit encrypt*/
			//write_line_to_file(PPTP_PEERS_FILE,2, line_buffer);
			//sprintf(line_buffer,"%s\n", "+mppe no56,stateless");/*disable 56bit encrypt*/
			//write_line_to_file(PPTP_PEERS_FILE,2, line_buffer);
			
		}
		apmib_get( MIB_PPTP_MPPC_ENABLED, (void *)&intValue1);
		if(intValue1==1){
			sprintf(line_buffer,"%s\n", "mppc");
			write_line_to_file(PPTP_PEERS_FILE,2, line_buffer);
			sprintf(line_buffer,"%s\n", "stateless");
			write_line_to_file(PPTP_PEERS_FILE,2, line_buffer);
		}else{
			sprintf(line_buffer,"%s\n", "nomppc");
			write_line_to_file(PPTP_PEERS_FILE,2, line_buffer);
		}
		if(intValue ==0 && intValue1==0){
			sprintf(line_buffer,"%s\n", "noccp");
			write_line_to_file(PPTP_PEERS_FILE,2, line_buffer);
		}
		
		sprintf(line_buffer,"%s\n", "persist");
		write_line_to_file(PPTP_PEERS_FILE,2, line_buffer);
		
		sprintf(line_buffer,"%s\n", "noauth");
		write_line_to_file(PPTP_PEERS_FILE,2, line_buffer);
		
		sprintf(line_buffer,"%s\n", "file /etc/ppp/options");
		write_line_to_file(PPTP_PEERS_FILE,2, line_buffer);
		
		sprintf(line_buffer,"%s\n", "nobsdcomp");
		write_line_to_file(PPTP_PEERS_FILE,2, line_buffer);
		
		sprintf(line_buffer,"%s\n", "nodetach");
		write_line_to_file(PPTP_PEERS_FILE,2, line_buffer);
		
		sprintf(line_buffer,"%s\n", "novj");
		write_line_to_file(PPTP_PEERS_FILE,2, line_buffer);
		
		
		apmib_get(MIB_PPTP_CONNECTION_TYPE, (void *)&connect_type);
		if(connect_type==1){
			
			sprintf(line_buffer,"%s\n", "persist");
			write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
			
			sprintf(line_buffer,"%s\n", "nodetach");
			write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
			
			sprintf(line_buffer,"%s\n", "connect /etc/ppp/true");
			write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
			
			sprintf(line_buffer,"%s\n", "demand");
			write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
			
			apmib_get(MIB_PPTP_IDLE_TIME, (void *)&idle_time);
			sprintf(line_buffer,"idle %d\n", idle_time);
			write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
			
			sprintf(line_buffer,"%s\n", "ktune");
			write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
			
			sprintf(line_buffer,"%s\n", "ipcp-accept-remote");
			write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
			
			sprintf(line_buffer,"%s\n", "ipcp-accept-local");
			write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
			
			sprintf(line_buffer,"%s\n", "noipdefault");
			write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
			
			sprintf(line_buffer,"%s\n", "hide-password");
			write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
			
			sprintf(line_buffer,"%s\n", "defaultroute");
			write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
		}else if(connect_type==2 && act_source==1 && !isFileExist(MANUAL_CONNECT_NOW)) //manual mode we do not dial up from init.sh
				return;
			
	#if 0
		apmib_get( MIB_DNS_MODE, (void *)&intValue1);
		if(intValue1==1){
			start_dns_relay();
		}else{
			RunSystemCmd(NULL_FILE, "dnrd", "--cache=off", "-s", "168.95.1.1",NULL_STR);
		}
	#endif	
		if(isFileExist(PPP_FILE)){
			unlink(PPP_FILE);
		} 
		sprintf(tmp_args, "%s", "4");/*wan type*/
		sprintf(tmp_args1, "%d", connect_type);/*connect type*/
		RunSystemCmd(NULL_FILE, "ppp_inet", "-t", tmp_args,  "-c", tmp_args1, "-x", NULL_STR);
	}
	start_upnp_igd(PPTP, sys_op, wisp_id, lan_iface);
}

void set_l2tp(int sys_op, char *wan_iface, char *lan_iface, int wisp_id, int act_source)
{
	int intValue=0;
//	int intValue1=0;
	char line_buffer[100]={0};
	char tmp_args[64]={0};
	char tmp_args1[32]={0};
	char Ip[32], Mask[32], ServerIp[32];
	int connect_type=0, idle_time=0;
	char *strtmp=NULL;
	int pwd_len=0;
#if defined(CONFIG_DYNAMIC_WAN_IP)
	char l2tpDefGw[32], netIp[32];
	unsigned int ipAddr, netAddr, netMask, serverAddr;
	int l2tp_wanip_dynamic=0;
	

	apmib_get(MIB_L2TP_WAN_IP_DYNAMIC, (void *)&l2tp_wanip_dynamic);

	apmib_get(MIB_L2TP_SERVER_IP_ADDR,  (void *)tmp_args);
	strtmp= inet_ntoa(*((struct in_addr *)tmp_args));
	sprintf(ServerIp, "%s", strtmp);
	serverAddr=((struct in_addr *)tmp_args)->s_addr;

	if(l2tp_wanip_dynamic==STATIC_IP)
	{//l2tp use static wan ip
	apmib_get(MIB_L2TP_DEFAULT_GW,  (void *)tmp_args);
	strtmp= inet_ntoa(*((struct in_addr *)tmp_args));
	sprintf(l2tpDefGw, "%s", strtmp);
#else
	apmib_get(MIB_L2TP_SERVER_IP_ADDR,  (void *)tmp_args);
	strtmp= inet_ntoa(*((struct in_addr *)tmp_args));
	sprintf(ServerIp, "%s", strtmp);
#endif
	apmib_get(MIB_L2TP_IP_ADDR,  (void *)tmp_args);
	strtmp= inet_ntoa(*((struct in_addr *)tmp_args));
	sprintf(Ip, "%s", strtmp);
#if defined(CONFIG_DYNAMIC_WAN_IP)
	ipAddr=((struct in_addr *)tmp_args)->s_addr;
#endif
	apmib_get(MIB_L2TP_SUBNET_MASK,  (void *)tmp_args);
	strtmp= inet_ntoa(*((struct in_addr *)tmp_args));
	sprintf(Mask, "%s", strtmp);
#if defined(CONFIG_DYNAMIC_WAN_IP)
	netMask=((struct in_addr *)tmp_args)->s_addr;
#endif
	
	RunSystemCmd(NULL_FILE, "ifconfig", wan_iface, Ip, "netmask", Mask, NULL_STR);
	RunSystemCmd(NULL_FILE, "route", "del", "default", "gw", "0.0.0.0", NULL_STR);
#if defined(CONFIG_DYNAMIC_WAN_IP)
		if((serverAddr & netMask) != (ipAddr & netMask)){
			//Patch for our router under another router to dial up l2tp
			//let l2tp dialing pkt via l2tp default gateway
			netAddr = (serverAddr & netMask);
			((struct in_addr *)tmp_args)->s_addr=netAddr;
			strtmp= inet_ntoa(*((struct in_addr *)tmp_args));
			sprintf(netIp, "%s", strtmp);
			RunSystemCmd(NULL_FILE, "route", "add", "-net", netIp, "netmask", Mask,"gw", l2tpDefGw,NULL_STR);
		}
	} // end for l2tp static ip
#endif

#if defined(RTL_L2TP_POWEROFF_PATCH)    //patch for l2tp by jiawenjan
	char l2tp_cmdBuf[100];
	int buff_length = 0;
	unsigned int l2tp_ns = 0;
	unsigned char  l2tp_tmpBuff[100], lanIp_tmp[16], serverIp_tmp[16];
	memset(lanIp_tmp,0, sizeof(lanIp_tmp));
	memset(serverIp_tmp,0, sizeof(serverIp_tmp));
	memset(l2tp_tmpBuff,0, sizeof(l2tp_tmpBuff));
	
	apmib_get(MIB_L2TP_PAYLOAD_LENGTH, (void *)&buff_length);
	if(buff_length>0)
	{	
		apmib_get(MIB_L2TP_NS, (void *)&l2tp_ns);
		apmib_get(MIB_L2TP_IP_ADDR,  (void *)lanIp_tmp);	
		apmib_get(MIB_L2TP_SERVER_IP_ADDR,	(void *)serverIp_tmp);
		apmib_get(MIB_L2TP_PAYLOAD,  (void *)l2tp_tmpBuff);
	
		sprintf(l2tp_cmdBuf,"flash clearl2tp %d %d %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
		l2tp_ns, buff_length, lanIp_tmp[0], lanIp_tmp[1], lanIp_tmp[2], lanIp_tmp[3], serverIp_tmp[0], serverIp_tmp[1], serverIp_tmp[2], serverIp_tmp[3], 
		l2tp_tmpBuff[0], l2tp_tmpBuff[1], l2tp_tmpBuff[2], l2tp_tmpBuff[3], l2tp_tmpBuff[4], l2tp_tmpBuff[5], l2tp_tmpBuff[6], l2tp_tmpBuff[7], 
		l2tp_tmpBuff[8], l2tp_tmpBuff[9], l2tp_tmpBuff[10], l2tp_tmpBuff[11], l2tp_tmpBuff[12], l2tp_tmpBuff[13], l2tp_tmpBuff[14], l2tp_tmpBuff[15], 
		l2tp_tmpBuff[16], l2tp_tmpBuff[17], l2tp_tmpBuff[18], l2tp_tmpBuff[19], l2tp_tmpBuff[20], l2tp_tmpBuff[21], l2tp_tmpBuff[22], l2tp_tmpBuff[23], 
		l2tp_tmpBuff[24], l2tp_tmpBuff[25], l2tp_tmpBuff[26], l2tp_tmpBuff[27], l2tp_tmpBuff[28], l2tp_tmpBuff[29], l2tp_tmpBuff[30], l2tp_tmpBuff[31], 
		l2tp_tmpBuff[32], l2tp_tmpBuff[33], l2tp_tmpBuff[34], l2tp_tmpBuff[35], l2tp_tmpBuff[36], l2tp_tmpBuff[37]);

		system(l2tp_cmdBuf); 
	}
#endif 	
	
	apmib_get( MIB_L2TP_USER_NAME,  (void *)tmp_args);
	apmib_get( MIB_L2TP_PASSWORD,  (void *)tmp_args1);
	pwd_len = strlen(tmp_args1);
	/*options file*/
	sprintf(line_buffer,"user \"%s\"\n",tmp_args);
	write_line_to_file(PPP_OPTIONS_FILE1, 1, line_buffer);
	
	/*secrets files*/
	sprintf(line_buffer,"%s\n","#################################################");
	write_line_to_file(PPP_PAP_FILE1, 1, line_buffer);
	
	sprintf(line_buffer, "\"%s\"	*	\"%s\"\n",tmp_args, tmp_args1);
	write_line_to_file(PPP_PAP_FILE1, 2, line_buffer);
	
	sprintf(line_buffer,"%s\n","#################################################");
	write_line_to_file(PPP_CHAP_FILE1, 1, line_buffer);
	
	sprintf(line_buffer, "\"%s\"	*	\"%s\"\n",tmp_args, tmp_args1);
	write_line_to_file(PPP_CHAP_FILE1, 2, line_buffer);
	
	sprintf(line_buffer,"%s\n", "lock");
	write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
	sprintf(line_buffer,"%s\n", "noauth");
	write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
	sprintf(line_buffer,"%s\n", "defaultroute");
	write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
	sprintf(line_buffer,"%s\n", "usepeerdns");
	write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
	sprintf(line_buffer,"%s\n", "lcp-echo-interval 0");
	write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
	sprintf(line_buffer,"%s\n", "wantype 6");
	write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
	
	apmib_get(MIB_L2TP_MTU_SIZE, (void *)&intValue);
	sprintf(line_buffer,"mtu %d\n", intValue);
	write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
	
	apmib_get( MIB_L2TP_USER_NAME,  (void *)tmp_args);
	sprintf(line_buffer,"name %s\n", tmp_args);
	write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
	
//	sprintf(line_buffer,"%s\n", "noauth");
//	write_line_to_file(PPP_OPTIONS_FILE,2, line_buffer);
	
	sprintf(line_buffer,"%s\n", "nodeflate");
	write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
	
	sprintf(line_buffer,"%s\n", "nobsdcomp");
	write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
	
	sprintf(line_buffer,"%s\n", "nodetach");
	write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
	
	sprintf(line_buffer,"%s\n", "novj");
	write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
	
	sprintf(line_buffer,"%s\n", "default-asyncmap");
	write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
	
	sprintf(line_buffer,"%s\n", "nopcomp");
	write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
	
	sprintf(line_buffer,"%s\n", "noaccomp");
	write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
	
	sprintf(line_buffer,"%s\n", "noccp");
	write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
	
	sprintf(line_buffer,"%s\n", "novj");
	write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
	
	sprintf(line_buffer,"%s\n", "refuse-eap");
	write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
	
	if(pwd_len > 35){
		sprintf(line_buffer,"%s\n", "-mschap");
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
		
		sprintf(line_buffer,"%s\n", "-mschap-v2");
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
	}
	
	sprintf(line_buffer,"%s\n", "[global]");
	write_line_to_file(L2TPCONF,1, line_buffer);
	
	sprintf(line_buffer,"%s\n", "port = 1701");
	write_line_to_file(L2TPCONF,2, line_buffer);
	
	sprintf(line_buffer,"auth file = %s\n", PPP_CHAP_FILE1);
	write_line_to_file(L2TPCONF,2, line_buffer);
	
	sprintf(line_buffer,"%s\n", "[lac client]");
	write_line_to_file(L2TPCONF,2, line_buffer);
	
	sprintf(line_buffer,"lns=%s\n", ServerIp);
	write_line_to_file(L2TPCONF,2, line_buffer);

	sprintf(line_buffer,"%s\n", "require chap = yes");
	write_line_to_file(L2TPCONF,2, line_buffer);
	
	apmib_get( MIB_L2TP_USER_NAME,  (void *)tmp_args);
	sprintf(line_buffer,"name = %s\n", tmp_args);
	write_line_to_file(L2TPCONF,2, line_buffer);
	
	sprintf(line_buffer,"%s\n", "pppoptfile = /etc/ppp/options");
	write_line_to_file(L2TPCONF, 2, line_buffer);

	RunSystemCmd(NULL_FILE, "killall", "l2tpd", NULL_STR);
	sleep(1);
	//RunSystemCmd(NULL_FILE, "l2tpd", NULL_STR);	
	system("l2tpd&");
	sleep(3);
	
	apmib_get(MIB_L2TP_CONNECTION_TYPE, (void *)&connect_type);
	if(connect_type==1){
			
		sprintf(line_buffer,"%s\n", "connect /etc/ppp/true");
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
			
		sprintf(line_buffer,"%s\n", "demand");
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
			
		apmib_get(MIB_L2TP_IDLE_TIME, (void *)&idle_time);
		sprintf(line_buffer,"idle %d\n", idle_time);
		write_line_to_file(PPP_OPTIONS_FILE1,2, line_buffer);
			
		}else if(connect_type==2 && act_source==1 && !isFileExist(MANUAL_CONNECT_NOW)) //manual mode we do not dial up from init.sh
				return;
			
	#if 0
		apmib_get( MIB_DNS_MODE, (void *)&intValue1);
		if(intValue1==1){
			start_dns_relay();
		}else{
			RunSystemCmd(NULL_FILE, "dnrd", "--cache=off", "-s", "168.95.1.1",NULL_STR);
		}
	#endif	
		if(isFileExist(PPP_FILE)){
			unlink(PPP_FILE);
		} 
		sprintf(tmp_args, "%s", "6");/*wan type*/
		sprintf(tmp_args1, "%d", connect_type);/*connect type*/
		RunSystemCmd(NULL_FILE, "ppp_inet", "-t", tmp_args,  "-c", tmp_args1, "-x", NULL_STR);
		start_upnp_igd(L2TP, sys_op, wisp_id, lan_iface);
}
int start_wan(int wan_mode, int sys_op, char *wan_iface, char *lan_iface, int wisp_id, int act_source)
{
	int lan_type=0;
#if defined(CONFIG_DYNAMIC_WAN_IP)
	int pptp_wanip_dynamic=0, l2tp_wanip_dynamic=0;
#endif
	printf("Init WAN Interface...\n");
	//RunSystemCmd(NULL_FILE, "ifconfig", NULL_STR);
	//RunSystemCmd(NULL_FILE, "brctl","show",NULL_STR);
	
	if(wan_mode == DHCP_DISABLED)
		set_staticIP(sys_op, wan_iface, lan_iface, wisp_id, act_source);
	else if(wan_mode == DHCP_CLIENT)
		set_dhcp_client(sys_op, wan_iface, lan_iface, wisp_id, act_source);
	else if(wan_mode == PPPOE){

		
#if defined(PPPOE_DISC_FLOW_PATCH)
		int sessid = 0;
		char cmdBuf[50],tmpBuff[30];
		int ppp_flag = 0;
		memset(tmpBuff,0, sizeof(tmpBuff));
		apmib_get(MIB_PPP_SESSION_NUM, (void *)&sessid);
		apmib_get(MIB_PPP_SERVER_MAC,  (void *)tmpBuff);
		apmib_get(MIB_PPP_NORMAL_FINISH,  (void *)&ppp_flag);	
		if(!ppp_flag) //1:nomal	0:abnormal
		{
			if(memcmp(tmpBuff, "\x00\x00\x00\x00\x00\x00", MAC_ADDR_LEN))
			{
				sprintf(cmdBuf,"flash clearppp %d %02x%02x%02x%02x%02x%02x",
					sessid,(unsigned char)tmpBuff[0],(unsigned char)tmpBuff[1],(unsigned char)tmpBuff[2],(unsigned char)tmpBuff[3],(unsigned char)tmpBuff[4],(unsigned char)tmpBuff[5]);
				system(cmdBuf);
				sessid = 0;
				memset(tmpBuff,0,sizeof(tmpBuff));
				apmib_set(MIB_PPP_SESSION_NUM, (void *)&sessid);
				apmib_set(MIB_PPP_SERVER_MAC, (void *)tmpBuff); 								
			}
		}		
		//system("flash set PPP_NORMAL_FINISH 0");		
		ppp_flag = 0 ;
		apmib_set(MIB_PPP_NORMAL_FINISH, (void *)&ppp_flag);		
		apmib_update(CURRENT_SETTING);
#else
		int sessid = 0;
		char cmdBuf[50],tmpBuff[30];
		memset(tmpBuff,0, sizeof(tmpBuff));
		apmib_get(MIB_PPP_SESSION_NUM, (void *)&sessid);
		apmib_get(MIB_PPP_SERVER_MAC,  (void *)tmpBuff);

		sprintf(cmdBuf,"flash clearppp %d %02x%02x%02x%02x%02x%02x",sessid,(unsigned char)tmpBuff[0],(unsigned char)tmpBuff[1],(unsigned char)tmpBuff[2],(unsigned char)tmpBuff[3],(unsigned char)tmpBuff[4],(unsigned char)tmpBuff[5]);
		system(cmdBuf);
		sleep(2);	// Wait util pppoe server reply PADT, then start pppoe dialing, otherwise pppoe server will reply PADS with PPPoE tags: Generic-Error.
#endif
		
		set_pppoe(sys_op, wan_iface, lan_iface, wisp_id, act_source);
	}else if(wan_mode == PPTP){
#if defined(CONFIG_DYNAMIC_WAN_IP)
		apmib_get(MIB_PPTP_WAN_IP_DYNAMIC, (void *)&pptp_wanip_dynamic);
		if(pptp_wanip_dynamic==STATIC_IP){
			set_pptp(sys_op, wan_iface, lan_iface, wisp_id, act_source);
		}else{
			RunSystemCmd(TEMP_WAN_CHECK, "echo", "dhcpc", NULL_STR);
			RunSystemCmd(NULL, "rm -rf", MANUAL_CONNECT_NOW, " 2>/dev/null",  NULL_STR);
			if(act_source == 0)
				RunSystemCmd(MANUAL_CONNECT_NOW, "echo",  "1", NULL_STR);
			set_dhcp_client(sys_op, wan_iface, lan_iface, wisp_id, act_source);
		}
#else
		set_pptp(sys_op, wan_iface, lan_iface, wisp_id, act_source);
#endif
		//RunSystemCmd(NULL_FILE, "pptp.sh", wan_iface, NULL_STR);
	}else if(wan_mode == L2TP){
		//RunSystemCmd(NULL_FILE, "l2tp.sh", wan_iface, NULL_STR);
#if defined(CONFIG_DYNAMIC_WAN_IP)
		apmib_get(MIB_L2TP_WAN_IP_DYNAMIC, (void *)&l2tp_wanip_dynamic);
		if(l2tp_wanip_dynamic==STATIC_IP){
			set_l2tp(sys_op, wan_iface, lan_iface, wisp_id, act_source);
		}else{
			RunSystemCmd(TEMP_WAN_CHECK, "echo", "dhcpc", NULL_STR);	
			RunSystemCmd(NULL, "rm -rf", MANUAL_CONNECT_NOW, " 2>/dev/null", NULL_STR);
			if(act_source == 0)
				RunSystemCmd(MANUAL_CONNECT_NOW, "echo", "1", NULL_STR);
			set_dhcp_client(sys_op, wan_iface, lan_iface, wisp_id, act_source);
		}
#else
		set_l2tp(sys_op, wan_iface, lan_iface, wisp_id, act_source);
#endif
	}	
	apmib_get(MIB_DHCP,(void*)&lan_type);
	if(lan_type == DHCP_CLIENT)
	{//when set lan dhcp client,default route should get from lan dhcp server.
	//otherwise,DHCP offer pocket from dhcp server would be routed to wan(default gw),and client can't complete dhcp
		RunSystemCmd(NULL_FILE, "route", "del", "default", wan_iface, NULL_STR);
	}
	return 0;
}
///***************************************
// to decide whether should reconn dhcp
//***************************************/
int wan_dhcpcNeedRenewConn(char *interface, char *option)
{
	struct in_addr ipAddr={0},netmask={0},router={0};
	char new_ipAddr[32]={0},new_netmask[32]={0},new_router[32]={0};
	char buf[256]={0};
	char strDns1[64]={0};
	char strDns2[64]={0};
	char strDns3[64]={0};
	int dns_mode=0;
	if(!getInAddr(interface, IP_ADDR_T, (void *)&ipAddr))
		return 1;
	if(!getInAddr(interface, NET_MASK_T, (void *)&netmask))
		return 1;
	if(!getDefaultRoute(interface,&router))
		return 1;
	if(!getDataFormFile(RESOLV_CONF,"nameserver",&strDns1,1))
		return 1;

	sprintf(buf,"%s %s",interface,inet_ntoa(ipAddr));
	
	strcat(buf," ");
	strcat(buf,inet_ntoa(netmask));

	strcat(buf," ");
	strcat(buf,inet_ntoa(router));

	apmib_get( MIB_DNS_MODE, (void *)&dns_mode);
	if(!dns_mode)
	{//dns auto
		strcat(buf," ");
		strcat(buf,strDns1);
		
		if(getDataFormFile(RESOLV_CONF,"nameserver",&strDns2,2))
		{
			strcat(buf," ");
			strcat(buf,strDns2);
		}
		if(getDataFormFile(RESOLV_CONF,"nameserver",&strDns3,3))
		{
			strcat(buf," ");
			strcat(buf,strDns3);
		}
	}
	//for dns manual, not compare dns
//	printf("%s:%dbuf=%s option=%s\n",__FUNCTION__,__LINE__,buf,option);
	if(!strncmp(option,buf,strlen(buf)))
		return 0;
	else
		return 1;
}
 
 
 
 
 
 
 
 
 
 
