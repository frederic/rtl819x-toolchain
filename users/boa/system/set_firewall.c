/*
 *      Utiltiy function for setting firewall filter
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include <arpa/inet.h>

#include <sys/stat.h>


#include "apmib.h"
#include "sysconf.h"
#include "sys_utility.h"
#ifdef CONFIG_RTK_VOIP
#include "voip_manager.h"
#endif
int setFirewallIptablesRules(int argc, char** argv);
char Iptables[]="iptables";
char INPUT[]= "INPUT";
char OUTPUT[]= "OUTPUT";
char FORWARD[]= "FORWARD";
char PREROUTING[]="PREROUTING";
char POSTROUTING[]= "POSTROUTING";
char ACCEPT[]= "ACCEPT";
char DROP[]= "DROP";
char RET[]= "RETURN";
char LOG[]= "LOG";
char MASQUERADE[]="MASQUERADE";
char REDIRECT[]="REDIRECT";
char MARK[]="MARK";
// iptables operations, manupilations, matches, options etc.
char ADD[]= "-A";
char DEL[]= "-D";
char FLUSH[]= "-F";
char INSERT[]="-I";
char NEW[]= "-N";
char POLICY[]= "-P";
char X[]= "-X";
char Z[]= "-Z";
char _dest[]= "-d";
char in[]= "-i";
char jump[]= "-j";
char match[]= "-m";
char out[]= "-o";
char _protocol[]= "-p";
char _src[]= "-s";
char _table[]= "-t";
char nat_table[]= "nat";
char mangle_table[]= "mangle";
char NOT[]= "!";
char _mac[]= "mac";
char mac_src[]= "--mac-source";
char mac_dst[]= "--mac-destination";
char dport[]= "--dport";
char sport[]= "--sport";
char syn[]= "--syn";
char ALL[]= "ALL";
char DNAT[]= "DNAT";
char icmp_type[]="--icmp-type";
char echo_request[]="echo-request";
char echo_reply[]="echo-reply";
char mstate[]="state";
char state[]="--state";
char _udp[]="udp";
char _tcp[]="tcp";
char _icmp[]="icmp";
char RELATED_ESTABLISHED[]= "RELATED,ESTABLISHED";
char tcp_flags[]="--tcp-flags";
char MSS_FLAG1[]="SYN,RST";
char MSS_FLAG2[]="SYN";
char clamp[]="--clamp-mss-to-pmtu";
char TCPMSS[]="TCPMSS";
char ip_range[]="iprange";
char src_rnage[]="--src-range";
char dst_rnage[]="--dst-range";
char set_mark[]="--set-mark";

static const char _tc[] = "tc";
static const char _qdisc[] = "qdisc";
static const char _add[] = "add";
static const char _dev[] = "dev";
static const char _root[] = "root";
static const char _handle[] = "handle";
static const char _htb[] = "htb";
static const char _default[] = "default";
static const char _classid[] = "classid";
static const char _rate[] = "rate";
static const char _ceil[] = "ceil";
static const char _sfq[] = "sfq";
static const char _perturb[] = "perturb";
static const char _class[] = "class";
static const char _filter[] = "filter";
static const char _protocol2[] = "protocol";
static const char _ip[] = "ip";
static const char _prio[] = "prio";
static const char _fw[] = "fw";
static const char _parent[] = "parent";
static const char _quantum[] = "quantum";
static const char _r2q[] = "r2q";

#ifdef MULTI_PPPOE
//#define MULTI_PPP_DEBUG

struct PPP_info
{ 
	char client_ip[20];
	char server_ip[20];
	char ppp_name[5];
	int order;
};
/*
struct subNet
{	
	int SubnetCount;
	unsigned char startip[3][20];
	unsigned char endip[3][20];
};*/


char SubNet[4][30];
char flushCmds[12][80];
int CmdCount = 0 ;


//struct subNet SubNets[4];
//br0_info
int pppNumbers = 0;
int info_setting = 0;
struct PPP_info infos[5];
char Br0NetSectAddr[30];
//lan partition info
char  lan_ip[4][40] ;

int get_info()
{
	int subCount;
	unsigned char buffer[30];
	int connectNumber,index = -1;
	FILE *local,*remote,*order,*number,*br0,*pF,*pdev;
	if((local=fopen("/etc/ppp/ppp_local","r+"))==NULL)
	{
		printf("Cannot open this file\n");
		return 0;
	}
	if((remote=fopen("/etc/ppp/ppp_remote","r+"))==NULL)
	{
		printf("Cannot open this file\n");		
		return 0;
	}
	
	if((order=fopen("/etc/ppp/ppp_order_info","r+"))==NULL)
	{
		printf("Cannot open this file\n");
		return 0;
	}
	
	if((number=fopen("/etc/ppp/lineNumber","r+"))==NULL)
	{
		printf("Cannot open this file\n");
		return 0;
	}
	if((br0=fopen("/etc/ppp/br0_info","r+"))==NULL)
	{
		printf("Cannot open this file\n");
		return 0;
	}	
	if((pdev=fopen("/etc/ppp/ppp_device","r+"))==NULL)
	{
		printf("Cannot open this file\n");
		return 0;
	}		

	close(order);
	fscanf(br0,"%s",Br0NetSectAddr);
	fscanf(number,"%d",&pppNumbers);
	
	for( index = 0 ; index < pppNumbers ; ++index)
	{		
		int num,i,j;
		char name[5];
		char devname[5];
		
		fscanf(local,"%s",infos[index].client_ip);
		fscanf(remote,"%s",infos[index].server_ip);						
		fscanf(pdev,"%s",devname);
		if((order=fopen("/etc/ppp/ppp_order_info","r+"))==NULL)
			return ;
		while(fscanf(order,"%d--%s",&num,name) > 0 )
		{			
#ifdef MULTI_PPP_DEBUG		
			printf("devname value is:%s\n",devname);
			printf("name value is:%s\n",name);	
			printf("num value is:%d\n",num);				
#endif			
			if(!strcmp(devname,name))
			{
				infos[index].order = num;
				strcpy(infos[index].ppp_name,devname);
#ifdef MULTI_PPP_DEBUG					
				printf("infos[index].order value is:%d\n",infos[index].order);
				printf("infos[index].ppp_name value is:%s\n",infos[index].ppp_name);
#endif				
				break;
			}				
		}
		fclose(order);
	}
	fclose(local);
	fclose(remote);	
	fclose(number);	
	fclose(br0);
	fclose(pdev);
	//get the subnet info
	if((pF = fopen("/etc/ppp/ppp_connect_number","r"))==NULL)
	{
		printf("can't open the file\n");
		return 0;
	}	
	fscanf(pF,"%d",&connectNumber);		//max value is 4
	fclose(pF);	

	//apmib_get( MIB_SUBNET1_F1_START,  (void *)buffer);
	//printf("test-------------%s\n",inet_ntoa(*((struct in_addr *)buffer)));
	if(connectNumber >= 1)
	{
		apmib_get(MIB_PPP_SUBNET1,(void *)buffer);
		strcpy(SubNet[0],buffer);
		
/*	
		apmib_get(MIB_SUBNET1_COUNT, (void *)&subCount);
		SubNets[0].SubnetCount = subCount;
		if(subCount >= 1)
		{
			apmib_get(MIB_SUBNET1_F1_START,(void *)buffer);
			strcpy(SubNets[0].startip[0],inet_ntoa(*((struct in_addr *)buffer)));
			apmib_get(MIB_SUBNET1_F1_END, (void *)buffer);
			strcpy(SubNets[0].endip[0],inet_ntoa(*((struct in_addr *)buffer)));
		}
		if(subCount >= 2)
		{
			apmib_get(MIB_SUBNET1_F2_START,(void *)buffer);
			strcpy(SubNets[0].startip[1],inet_ntoa(*((struct in_addr *)buffer)));
			apmib_get(MIB_SUBNET1_F2_END, (void *)buffer);
			strcpy(SubNets[0].endip[1],inet_ntoa(*((struct in_addr *)buffer)));
		}
		if(subCount >= 3)
		{
			apmib_get(MIB_SUBNET1_F3_START,(void *)buffer);
			strcpy(SubNets[0].startip[2],inet_ntoa(*((struct in_addr *)buffer)));
			apmib_get(MIB_SUBNET1_F3_END, (void *)buffer);
			strcpy(SubNets[0].endip[2],inet_ntoa(*((struct in_addr *)buffer)));
		}		
*/
	}
	if(connectNumber >= 2)
	{
		apmib_get(MIB_PPP_SUBNET2,(void *)buffer);
		strcpy(SubNet[1],buffer);
	
/*		
		apmib_get(MIB_SUBNET2_COUNT, (void *)&subCount);
		SubNets[1].SubnetCount = subCount;
		if(subCount >= 1)
		{
			apmib_get(MIB_SUBNET2_F1_START,(void *)buffer);
			strcpy(SubNets[1].startip[0],inet_ntoa(*((struct in_addr *)buffer)));
			apmib_get(MIB_SUBNET2_F1_END, (void *)buffer);
			strcpy(SubNets[1].endip[0],inet_ntoa(*((struct in_addr *)buffer)));
		}
		if(subCount >= 2)
		{
			apmib_get(MIB_SUBNET2_F2_START,(void *)buffer);
			strcpy(SubNets[1].startip[1],inet_ntoa(*((struct in_addr *)buffer)));
			apmib_get(MIB_SUBNET2_F2_END, (void *)buffer);
			strcpy(SubNets[1].endip[1],inet_ntoa(*((struct in_addr *)buffer)));	
		}
		if(subCount >= 3)
		{
			apmib_get(MIB_SUBNET2_F3_START,(void *)buffer);
			strcpy(SubNets[1].startip[2],inet_ntoa(*((struct in_addr *)buffer)));
			apmib_get(MIB_SUBNET2_F3_END, (void *)buffer);
			strcpy(SubNets[1].endip[2],inet_ntoa(*((struct in_addr *)buffer)));	
		}
*/		
	}
	if(connectNumber >= 3)
	{
		apmib_get(MIB_PPP_SUBNET3,(void *)buffer);	
		strcpy(SubNet[2],buffer);		
/*		
		apmib_get(MIB_SUBNET3_COUNT, (void *)&subCount);
		SubNets[2].SubnetCount = subCount;
		if(subCount >= 1)
		{
			apmib_get(MIB_SUBNET3_F1_START,(void *)buffer);
			strcpy(SubNets[2].startip[0],inet_ntoa(*((struct in_addr *)buffer)));
			apmib_get(MIB_SUBNET3_F1_END, (void *)buffer);
			strcpy(SubNets[2].endip[0],inet_ntoa(*((struct in_addr *)buffer)));	
		}
		if(subCount >= 2)
		{
			apmib_get(MIB_SUBNET3_F2_START,(void *)buffer);
			strcpy(SubNets[2].startip[1],inet_ntoa(*((struct in_addr *)buffer)));
			apmib_get(MIB_SUBNET3_F2_END, (void *)buffer);
			strcpy(SubNets[2].endip[1],inet_ntoa(*((struct in_addr *)buffer)));	
		}
		if(subCount >= 3)
		{
			apmib_get(MIB_SUBNET3_F3_START,(void *)buffer);
			strcpy(SubNets[2].startip[2],inet_ntoa(*((struct in_addr *)buffer)));
			apmib_get(MIB_SUBNET3_F3_END, (void *)buffer);
			strcpy(SubNets[2].endip[2],inet_ntoa(*((struct in_addr *)buffer)));	
		}
*/		

	}
	if(connectNumber >= 4)
	{
		apmib_get(MIB_PPP_SUBNET4,(void *)buffer);	
		strcpy(SubNet[3],buffer);		
		
/*		
		apmib_get(MIB_SUBNET4_COUNT, (void *)&subCount);
		SubNets[3].SubnetCount = subCount;
		if(subCount >= 1)
		{
			apmib_get(MIB_SUBNET4_F1_START,(void *)buffer);
			strcpy(SubNets[3].startip[0],inet_ntoa(*((struct in_addr *)buffer)));
			apmib_get(MIB_SUBNET4_F1_END, (void *)buffer);
			strcpy(SubNets[3].endip[0],inet_ntoa(*((struct in_addr *)buffer)));	
		}
		if(subCount >= 2)
		{
			apmib_get(MIB_SUBNET4_F2_START,(void *)buffer);
			strcpy(SubNets[3].startip[1],inet_ntoa(*((struct in_addr *)buffer)));
			apmib_get(MIB_SUBNET4_F2_END, (void *)buffer);
			strcpy(SubNets[3].endip[1],inet_ntoa(*((struct in_addr *)buffer)));	
		}
		if(subCount >= 3)
		{
			apmib_get(MIB_SUBNET4_F3_START,(void *)buffer);
			strcpy(SubNets[3].startip[2],inet_ntoa(*((struct in_addr *)buffer)));
			apmib_get(MIB_SUBNET4_F3_END, (void *)buffer);
			strcpy(SubNets[3].endip[2],inet_ntoa(*((struct in_addr *)buffer)));
		}	
*/		
	}
	return 1;
}
void 	print_info()
{
	int index;
	int sub_index;
	int sub_number;
	for(index = 0 ; index < 4 ; ++index)
	{
		/*
		sub_number = SubNets[index].SubnetCount;

		for(sub_index = 0 ;sub_index< sub_number;++sub_index)
		{	
			printf("the %d subnet  is:%s\n",sub_index+1);
			printf("the value of startip is:%s\n",SubNets[index].startip[sub_index]);
			printf("the value of endip is:%s\n",SubNets[index].endip[sub_index]);				
		}
		printf("--------------------------------------------------------\n");
		*/
	}
}
#endif
extern int apmib_initialized;
extern int getInAddr( char *interface, int type, void *pAddr );
extern int isFileExist(char *file_name);

#ifdef CONFIG_APP_TR069
extern char acsURLStr[];
#endif //#ifdef CONFIG_APP_TR069

#ifdef CONFIG_RTK_VOIP
int set_voip_parameter(char* pInterface){

	#ifdef CONFIG_RTL_HW_NAPT
	unsigned long	dos_enabled = 0;
	int intVal=0;
	int intVal_num=0;
	#endif
#ifdef SLIC_CH_NUM  // old design 
	const int total_voip_ports = SLIC_CH_NUM + DECT_CH_NUM + DAA_CH_NUM;
#else
	const int total_voip_ports = g_VoIP_Ports;
#endif
	char rtp_port[20]={0};
	char sip_port[10]={0};
	int index;
	#ifdef CONFIG_RTL_HARDWARE_NAT
	int ivalue = 0;	
	#endif
	voipCfgParam_t  voipCfgParam;


	//printf("int set_voip_parameter....\n");
	apmib_get(MIB_VOIP_CFG, (void*)&voipCfgParam);


	for(index = 0; index < total_voip_ports; index++){
		//iptables -A INPUT -i eth1 -p udp --dport 5060 -j ACCEPT
		sprintf(sip_port,"%d", voipCfgParam.ports[index].sip_port);
		RunSystemCmd(NULL_FILE, Iptables, ADD, INPUT, in, pInterface, _protocol, _udp, dport,sip_port ,jump,ACCEPT, NULL_STR);

		// iptables -I PREROUTING -t nat -i eth1 -p udp --dport 5060 -j ACCEPT
		RunSystemCmd(NULL_FILE, Iptables, INSERT, PREROUTING, _table, nat_table , in, pInterface, _protocol, _udp, dport,sip_port ,jump,ACCEPT, NULL_STR);

		sprintf(rtp_port,"%d:%d",voipCfgParam.ports[index].media_port,voipCfgParam.ports[index].media_port+3);
		//iptables -I PREROUTING -t nat -i eth1 -p udp --dport 9000:9003 -j ACCEPT
		RunSystemCmd(NULL_FILE, Iptables, INSERT, PREROUTING, _table, nat_table , in, pInterface, _protocol, _udp, dport, rtp_port ,jump,ACCEPT, NULL_STR);
	}


	#if 0
	def CONFIG_RTL_HW_NAPT
	apmib_get(MIB_URLFILTER_ENABLED,  (void *)&intVal);
	apmib_get(MIB_URLFILTER_TBL_NUM,  (void *)&intVal_num);
	apmib_get(MIB_DOS_ENABLED, (void *)&dos_enabled);
		apmib_get(MIB_SUBNET_MASK,(void*)&ivalue);
		
	//when dos or urlfilter is enable, hwnat must be turn off!
	if((intVal !=0 && intVal_num>0)||(dos_enabled > 0)||(!voipCfgParam.hwnat_enable))
			RunSystemCmd("/proc/hw_nat", "echo", "0", NULL_STR);
	else if(voipCfgParam.hwnat_enable)
		{
			if((ivalue&HW_NAT_LIMIT_NETMASK)!=HW_NAT_LIMIT_NETMASK)
				RunSystemCmd("/proc/hw_nat", "echo", "0", NULL_STR);
			else
				RunSystemCmd("/proc/hw_nat", "echo", "1", NULL_STR);				
		}
	#endif
}
#endif

#ifdef MULTI_PPPOE
int set_QoS(int operation, int wan_type, int wisp_wan_id , char* interface)
#else 
int set_QoS(int operation, int wan_type, int wisp_wan_id)
#endif
{
#ifdef   HOME_GATEWAY
	char *br_interface="br0";
	char tmp_args[32]={0}, tmp_args1[32]={0}, tmp_args2[32]={0};
	char tmp_args3[64]={0}, tmp_args4[32]={0};
	char *tmpStr=NULL;
	int wan_pkt_mark=13, lan_pkt_mark=53;
	char iface[20], *pInterface="eth1", *pInterface2=NULL;
	int i, QoS_Enabled=0;
	int QoS_Auto_Uplink=0, QoS_Manual_Uplink=0;
	int QoS_Auto_Downlink=0, QoS_Manual_Downlink=0;
	int QoS_Rule_EntryNum=0;
	char PROC_QOS[128]={0};
	int uplink_speed=102400, downlink_speed=102400;
	IPQOS_T entry;
	int get_wanip=0;
	struct in_addr wanaddr;
	unsigned char str_l7_filter[128]={0};

	int needSetOnce = 1;
#ifdef CONFIG_RTL_8198
	uplink_speed=1024000;
	downlink_speed=1024000;
#endif

#ifdef MULTI_PPPOE
	if(!strncmp(interface,"ppp0",3) ||!strncmp(interface,"ppp1",3) || !strncmp(interface,"ppp2",3)
				|| !strncmp(interface,"ppp3",3))
	{
		FILE* fp;
		int pppDeviceNumber;
		if((fp=fopen("/etc/ppp/hasPppoedevice","r+"))==NULL)
		{
#ifdef MULTI_PPP_DEBUG	   
			printf("Cannot open this file\n");
#endif
			return 0;
		}
		fscanf(fp,"%d",&pppDeviceNumber);
		if(pppDeviceNumber == 1)
			needSetOnce = 1;
		else if( pppDeviceNumber >=2)
			needSetOnce = 0;						
	}
#endif
#ifdef MULTI_PPPOE
		if(needSetOnce){
#endif
	RunSystemCmd(NULL_FILE, Iptables, FLUSH, _table, mangle_table, NULL_STR);
	RunSystemCmd(NULL_FILE, Iptables, X, _table, mangle_table, NULL_STR);
	RunSystemCmd(NULL_FILE, Iptables, Z, _table, mangle_table, NULL_STR);
#ifdef MULTI_PPPOE
		}
#endif
	if(operation == WISP_MODE){
		sprintf(iface, "wlan%d", wisp_wan_id);
#if defined(CONFIG_SMART_REPEATER)			
		getWispRptIfaceName(iface,wisp_wan_id);
		//strcat(iface, "-vxd");
#endif		
		pInterface = iface;
		if (wan_type == PPPOE || wan_type == PPTP /*|| wan_type == L2TP */)
#ifdef MULTI_PPPOE
			pInterface = interface;
#else
			pInterface="ppp0";
#endif
	}else{
		if(operation == GATEWAY_MODE){
			if (wan_type == PPPOE || wan_type == PPTP || wan_type == USB3G /*|| wan_type == L2TP*/)
#ifdef MULTI_PPPOE
			pInterface = interface;
#else
			pInterface="ppp0";
#endif
		}
	}

	if(wan_type == L2TP)//wantype is l2tp
		pInterface2="ppp0";

	get_wanip = getInAddr(pInterface, IP_ADDR_T, (void *)&wanaddr);
	if( get_wanip ==0){   //get wan ip fail
		printf("No wan ip currently!\n");
		return 0;
	}

	apmib_get( MIB_QOS_ENABLED, (void *)&QoS_Enabled);
	apmib_get( MIB_QOS_AUTO_UPLINK_SPEED, (void *)&QoS_Auto_Uplink);
	apmib_get( MIB_QOS_MANUAL_UPLINK_SPEED, (void *)&QoS_Manual_Uplink);
	apmib_get( MIB_QOS_MANUAL_DOWNLINK_SPEED, (void *)&QoS_Manual_Downlink);
	apmib_get( MIB_QOS_AUTO_DOWNLINK_SPEED, (void *)&QoS_Auto_Downlink);
	apmib_get( MIB_QOS_RULE_TBL_NUM, (void *)&QoS_Rule_EntryNum);

	RunSystemCmd(NULL_FILE, "tc", "qdisc", "del", "dev", br_interface, "root", NULL_STR);

	//To avoid rule left when wan changed
	RunSystemCmd(NULL_FILE, "tc", "qdisc", "del", "dev", pInterface, "root", NULL_STR);
	RunSystemCmd(NULL_FILE, "tc", "qdisc", "del", "dev", "ppp0", "root", NULL_STR);

	if((strcmp(pInterface, "eth1")!=0)&&(strcmp(pInterface, "ppp0")!=0))
		RunSystemCmd(NULL_FILE, "tc", "qdisc", "del", "dev", pInterface, "root", NULL_STR);

#ifdef MULTI_PPPOE
	if(needSetOnce){
#endif
	sprintf(PROC_QOS, "%s", "0,");

	if(QoS_Enabled==1){
		sprintf(PROC_QOS, "%s", "1,");
	}

	// echo /proc/qos should before tc rules because of qos patch (CONFIG_RTL_QOS_PATCH in kernel)
	RunSystemCmd("/proc/qos", "echo", PROC_QOS, NULL_STR);
#ifdef MULTI_PPPOE
		}
#endif

	if(QoS_Enabled==1){
		if(QoS_Auto_Uplink==0){
			uplink_speed=QoS_Manual_Uplink;
			if(uplink_speed < 100)
				uplink_speed=100;
		}

		// patch for uplink QoS accuracy
#if 0
#ifdef CONFIG_RTL_8198
		if(uplink_speed > 160000)
			uplink_speed=160000;
#else
		if(uplink_speed > 75000)
			uplink_speed=75000;
#endif
#endif

		if(QoS_Auto_Downlink==0){
			downlink_speed=QoS_Manual_Downlink;
			if(downlink_speed < 100)
				downlink_speed=100;
		}
		// patch for downlink QoS accuracy
#if 0
#ifdef CONFIG_RTL_8198
		if(downlink_speed > 130000)
			downlink_speed=130000;
#else
		if(downlink_speed > 70000)
			downlink_speed=70000;
#endif
#endif

		/* total bandwidth section--uplink*/
		RunSystemCmd(NULL_FILE, _tc, _qdisc, _add, _dev, pInterface, _root, _handle, "2:0", _htb, _default, "2", _r2q, "64", NULL_STR);
		//tc qdisc add dev $WAN root handle 2:0 htb default 2 r2q 64
		sprintf(tmp_args, "%dkbit", uplink_speed);
		RunSystemCmd(NULL_FILE, _tc, _class, _add, _dev, pInterface, _parent, "2:0", _classid, "2:1", _htb, _rate, tmp_args, _ceil, tmp_args,  _quantum, "30000", NULL_STR);
		//TC_CMD="tc class add dev $WAN parent 2:0 classid 2:1 htb rate ${UPLINK_SPEED}kbit ceil ${UPLINK_SPEED}kbit"
		RunSystemCmd(NULL_FILE, _tc, _class, _add, _dev, pInterface, _parent, "2:1", _classid, "2:2", _htb, _rate, "1kbit", _ceil, tmp_args, _prio, "256",  _quantum, "30000", NULL_STR);
    		//TC_CMD="tc class add dev $WAN parent 2:1 classid 2:2 htb rate 1kbit ceil ${UPLINK_SPEED}kbit prio 256 quantum 30000"
    		RunSystemCmd(NULL_FILE, _tc, _qdisc, _add, _dev, pInterface, _parent, "2:2", _handle, "102:", _sfq, _perturb, "10", NULL_STR);
    		//TC_CMD="tc qdisc add dev $WAN parent 2:2 handle 102: sfq perturb 10"

#if 1
		if((pInterface2!=NULL)&&strcmp(pInterface2, "ppp0")==0)//wantype is l2tp
		{
			RunSystemCmd(NULL_FILE, _tc, _qdisc, _add, _dev, pInterface2, _root, _handle, "3:0", _htb, _default, "2", _r2q, "64", NULL_STR);
			//tc qdisc add dev $WAN2 root handle 3:0 htb default 2 r2q 64
			sprintf(tmp_args, "%dkbit", uplink_speed);
			RunSystemCmd(NULL_FILE, _tc, _class, _add, _dev, pInterface2, _parent, "3:0", _classid, "3:1", _htb, _rate, tmp_args, _ceil, tmp_args,  _quantum, "30000", NULL_STR);
			//TC_CMD="tc class add dev $WAN2 parent 3:0 classid 3:1 htb rate ${UPLINK_SPEED}kbit ceil ${UPLINK_SPEED}kbit"
			RunSystemCmd(NULL_FILE, _tc, _class, _add, _dev, pInterface2, _parent, "3:1", _classid, "3:2", _htb, _rate, "1kbit", _ceil, tmp_args, _prio, "256",  _quantum, "30000", NULL_STR);
	    		//TC_CMD="tc class add dev $WAN2 parent 3:1 classid 3:2 htb rate 1kbit ceil ${UPLINK_SPEED}kbit prio 256 quantum 30000"
	    		RunSystemCmd(NULL_FILE, _tc, _qdisc, _add, _dev, pInterface2, _parent, "3:2", _handle, "302:", _sfq, _perturb, "10", NULL_STR);
	    		//TC_CMD="tc qdisc add dev $WAN2 parent 3:2 handle 302: sfq perturb 10"
		}
#endif
#ifdef MULTI_PPPOE
			if(needSetOnce){
#endif

		/* total bandwidth section--downlink*/
    		RunSystemCmd(NULL_FILE, _tc, _qdisc, _add, _dev, br_interface, _root, _handle, "5:0", _htb, _default, "2", _r2q, "64",NULL_STR);
    		//tc qdisc add dev $BRIDGE root handle 5:0 htb default 5 r2q 64
    		sprintf(tmp_args, "%dkbit", downlink_speed);
    		RunSystemCmd(NULL_FILE, _tc, _class, _add, _dev, br_interface, _parent, "5:0", _classid, "5:1", _htb, _rate, tmp_args, _ceil, tmp_args,  _quantum, "30000", NULL_STR);
    		//TC_CMD="tc class add dev $BRIDGE parent 5:0 classid 5:1 htb rate ${DOWNLINK_SPEED}kbit ceil ${DOWNLINK_SPEED}kbit"
    		RunSystemCmd(NULL_FILE, _tc, _class, _add, _dev, br_interface, _parent, "5:1", _classid, "5:2", _htb, _rate, "1kbit", _ceil, tmp_args, _prio, "256", _quantum, "30000", NULL_STR);
		//TC_CMD="tc class add dev $BRIDGE parent 5:1 classid 5:5 htb rate 1kbit ceil ${DOWNLINK_SPEED}kbit prio 256 quantum 30000"
		RunSystemCmd(NULL_FILE, _tc, _qdisc, _add, _dev, br_interface, _parent, "5:2", _handle, "502:", _sfq, _perturb, "10", NULL_STR);
		//TC_CMD="tc qdisc add dev $BRIDGE parent 5:5 handle 502: sfq perturb 10"
//		sprintf(PROC_QOS, "%s", "1,");
#ifdef MULTI_PPPOE
		}
#endif

		if(QoS_Rule_EntryNum > 0){
			for (i=1; i<=QoS_Rule_EntryNum; i++) {
				unsigned char command[200]={0};
				*((char *)&entry) = (char)i;
				apmib_get(MIB_QOS_RULE_TBL, (void *)&entry);
				if(entry.enabled > 0){
					if(entry.bandwidth > 0){/*UPlink*/
						sprintf(tmp_args, "%d", wan_pkt_mark);

						if((strcmp(entry.l7_protocol,"") == 0) || (strcmp(entry.l7_protocol,"Disable") == 0))
						{
							sprintf(str_l7_filter,"%s","");
						}
						else
						{
							sprintf(str_l7_filter,"%s %s","-m layer7 --l7proto ", entry.l7_protocol);							
						}
						
						if(entry.mode & QOS_RESTRICT_IP)//if(entry.mode == 5 || entry.mode == 6){
						{
							/*this qos rule is set by IP address*/
							tmpStr = inet_ntoa(*((struct in_addr *)entry.local_ip_start));
							sprintf(tmp_args1, "%s", tmpStr);
							tmpStr = inet_ntoa(*((struct in_addr *)entry.local_ip_end));
							sprintf(tmp_args2, "%s", tmpStr);
							sprintf(tmp_args3, "%s-%s",tmp_args1, tmp_args2);
							//iptables -A PREROUTING -t mangle -m iprange --src-range 192.168.1.11-192.168.1.22 -j MARK --set-mark 13
							//RunSystemCmd(NULL_FILE, Iptables, ADD, PREROUTING, _table, mangle_table , match, ip_range, src_rnage, tmp_args3, str_l7_filter, jump, MARK, set_mark, tmp_args, NULL_STR);
							sprintf(command,"%s %s %s %s %s %s %s %s %s %s %s %s %s %s", Iptables, ADD, PREROUTING, _table, mangle_table , match, ip_range, src_rnage, tmp_args3, str_l7_filter, jump, MARK, set_mark, tmp_args, NULL_STR);
//printf("\r\n command=[%s],__[%s-%u]\r\n",command,__FILE__,__LINE__);							
							system(command);
						}
						else if(entry.mode & QOS_RESTRICT_MAC){
							sprintf(tmp_args3, "%02x:%02x:%02x:%02x:%02x:%02x",entry.mac[0], entry.mac[1], entry.mac[2], entry.mac[3], entry.mac[4], entry.mac[5]);
							//iptables -A PREROUTING -t mangle -m mac --mac-source 00:11:22:33:44:55 -j MARK --set-mark 13
							//RunSystemCmd(NULL_FILE, Iptables, ADD, PREROUTING, _table, mangle_table , match, _mac, mac_src, tmp_args3, str_l7_filter, jump, MARK, set_mark, tmp_args, NULL_STR);
							sprintf(command,"%s %s %s %s %s %s %s %s %s %s %s %s %s %s", Iptables, ADD, PREROUTING, _table, mangle_table , match, _mac, mac_src, tmp_args3, str_l7_filter, jump, MARK, set_mark, tmp_args, NULL_STR);
//printf("\r\n command=[%s],__[%s-%u]\r\n",command,__FILE__,__LINE__);							
							system(command);
						}
						else //any
						{
							//iptables -A PREROUTING -t mangle -j MARK --set-mark 13
							//RunSystemCmd(NULL_FILE, Iptables, ADD, PREROUTING, _table, mangle_table , str_l7_filter, jump, MARK, set_mark, tmp_args, NULL_STR);
							sprintf(command,"%s %s %s %s %s %s %s %s %s %s", Iptables, ADD, PREROUTING, _table, mangle_table , str_l7_filter, jump, MARK, set_mark, tmp_args, NULL_STR);
//printf("\r\n command=[%s],__[%s-%u]\r\n",command,__FILE__,__LINE__);							
							system(command);
						}

						sprintf(tmp_args1, "2:%d", wan_pkt_mark);
						sprintf(tmp_args2, "%ldkbit", entry.bandwidth);
						sprintf(tmp_args3, "%dkbit", uplink_speed);
						sprintf(tmp_args4, "1%d:", wan_pkt_mark);
						if(entry.mode & QOS_RESTRICT_MIN){//if(entry.mode == 5 || entry.mode == 9){
							RunSystemCmd(NULL_FILE, _tc, _class, _add, _dev, pInterface, _parent, "2:1", _classid, tmp_args1, _htb, _rate, tmp_args2, _ceil, tmp_args3, _prio, "2",  _quantum, "30000",NULL_STR);
							//TC_CMD="tc class add dev $WAN parent 2:1 classid 2:$wan_pkt_mark htb rate ${bandwidth}kbit ceil ${UPLINK_SPEED}kbit prio 2 quantum 30000"
						}else{
							RunSystemCmd(NULL_FILE, _tc, _class, _add, _dev, pInterface, _parent, "2:1", _classid, tmp_args1, _htb, _rate, "1kbit", _ceil, tmp_args2, _prio, "2" , _quantum, "30000", NULL_STR);
							//TC_CMD="tc class add dev $WAN parent 2:1 classid 2:$wan_pkt_mark htb rate 1kbit ceil ${bandwidth}kbit prio 2 quantum 30000"
						}

						RunSystemCmd(NULL_FILE, _tc, _qdisc, _add, _dev, pInterface, _parent, tmp_args1, _handle, tmp_args4, _sfq, _perturb, "10", NULL_STR);
						//TC_CMD="tc qdisc add dev $WAN parent 2:$wan_pkt_mark handle 1$wan_pkt_mark: sfq perturb 10"

						RunSystemCmd(NULL_FILE, _tc, _filter, _add, _dev, pInterface, _parent, "2:0", _protocol2, _ip, _prio, "100", _handle, tmp_args, _fw, _classid, tmp_args1, NULL_STR);
						//TC_CMD="tc filter add dev $WAN parent 2:0 protocol ip prio 100 handle $wan_pkt_mark fw classid 2:$wan_pkt_mark"

#if 1
						sprintf(tmp_args1, "3:%d", wan_pkt_mark);
						sprintf(tmp_args2, "%ldkbit", entry.bandwidth);
						sprintf(tmp_args3, "%dkbit", uplink_speed);
						sprintf(tmp_args4, "3%d:", wan_pkt_mark);
						if((pInterface2!=NULL)&&strcmp(pInterface2, "ppp0")==0)//wantype is l2tp
						{
							if(entry.mode & QOS_RESTRICT_MIN){//if(entry.mode == 5 || entry.mode == 9){
								RunSystemCmd(NULL_FILE, _tc, _class, _add, _dev, pInterface2, _parent, "3:1", _classid, tmp_args1, _htb, _rate, tmp_args2, _ceil, tmp_args3, _prio, "2",  _quantum, "30000",NULL_STR);
								//TC_CMD="tc class add dev $WAN2 parent 3:1 classid 3:$wan_pkt_mark htb rate ${bandwidth}kbit ceil ${UPLINK_SPEED}kbit prio 2 quantum 30000"
							}else{
								RunSystemCmd(NULL_FILE, _tc, _class, _add, _dev, pInterface2, _parent, "3:1", _classid, tmp_args1, _htb, _rate, "1kbit", _ceil, tmp_args2, _prio, "2" , _quantum, "30000", NULL_STR);
								//TC_CMD="tc class add dev $WAN2 parent 3:1 classid 3:$wan_pkt_mark htb rate 1kbit ceil ${bandwidth}kbit prio 2 quantum 30000"
							}

							RunSystemCmd(NULL_FILE, _tc, _qdisc, _add, _dev, pInterface2, _parent, tmp_args1, _handle, tmp_args4, _sfq, _perturb, "10", NULL_STR);
							//TC_CMD="tc qdisc add dev $WAN2 parent 3:$wan_pkt_mark handle 3$wan_pkt_mark: sfq perturb 10"

							RunSystemCmd(NULL_FILE, _tc, _filter, _add, _dev, pInterface2, _parent, "3:0", _protocol2, _ip, _prio, "100", _handle, tmp_args, _fw, _classid, tmp_args1, NULL_STR);
							//TC_CMD="tc filter add dev $WAN2 parent 3:0 protocol ip prio 100 handle $wan_pkt_mark fw classid 3:$wan_pkt_mark"
						}
#endif

						wan_pkt_mark = wan_pkt_mark+1;
					}
#ifdef MULTI_PPPOE
						if(needSetOnce){
#endif

					if(entry.bandwidth_downlink > 0){/*DOWNlink*/
						sprintf(tmp_args, "%d", lan_pkt_mark);
						if(entry.mode & QOS_RESTRICT_IP){//if(entry.mode == 5 || entry.mode == 6){
							/*this qos rule is set by IP address*/
							tmpStr = inet_ntoa(*((struct in_addr *)entry.local_ip_start));
							sprintf(tmp_args1, "%s", tmpStr);
							tmpStr = inet_ntoa(*((struct in_addr *)entry.local_ip_end));
							sprintf(tmp_args2, "%s", tmpStr);
							sprintf(tmp_args3, "%s-%s",tmp_args1, tmp_args2);

							//RunSystemCmd(NULL_FILE, Iptables, ADD, POSTROUTING, _table, mangle_table , match, ip_range, dst_rnage, tmp_args3, jump, MARK,  set_mark, tmp_args, NULL_STR);							
							sprintf(command,"%s %s %s %s %s %s %s %s %s %s %s %s %s %s", Iptables, ADD, POSTROUTING, _table, mangle_table , match, ip_range, dst_rnage, tmp_args3, str_l7_filter, jump, MARK, set_mark, tmp_args, NULL_STR);
//printf("\r\n command=[%s],__[%s-%u]\r\n",command,__FILE__,__LINE__);							
							system(command);
						}
						else if(entry.mode & QOS_RESTRICT_MAC){
							sprintf(tmp_args3, "%02x:%02x:%02x:%02x:%02x:%02x",entry.mac[0], entry.mac[1], entry.mac[2], entry.mac[3], entry.mac[4], entry.mac[5]);
							//RunSystemCmd(NULL_FILE, Iptables, ADD, POSTROUTING, _table, mangle_table , match, _mac, mac_dst, tmp_args3, jump, MARK, set_mark, tmp_args, NULL_STR);
							sprintf(command,"%s %s %s %s %s %s %s %s %s %s %s %s %s %s", Iptables, ADD, POSTROUTING, _table, mangle_table , match, _mac, mac_dst, tmp_args3, str_l7_filter, jump, MARK, set_mark, tmp_args, NULL_STR);
//printf("\r\n command=[%s],__[%s-%u]\r\n",command,__FILE__,__LINE__);							
							system(command);
						}
						else
						{
							sprintf(command,"%s %s %s %s %s %s %s %s %s %s", Iptables, ADD, POSTROUTING, _table, mangle_table , str_l7_filter, jump, MARK, set_mark, tmp_args, NULL_STR);
//printf("\r\n command=[%s],__[%s-%u]\r\n",command,__FILE__,__LINE__);							
							system(command);							
						}

						sprintf(tmp_args1, "5:%d", lan_pkt_mark);
						sprintf(tmp_args2, "%ldkbit", entry.bandwidth_downlink);
						sprintf(tmp_args3, "%dkbit", downlink_speed);
						sprintf(tmp_args4, "5%d:", lan_pkt_mark);


						if(entry.mode & QOS_RESTRICT_MIN){//if(entry.mode == 5 || entry.mode == 9){
							RunSystemCmd(NULL_FILE, _tc, _class, _add, _dev, br_interface, _parent, "5:1", _classid, tmp_args1, _htb, _rate, tmp_args2, _ceil, tmp_args3, _prio, "2", _quantum, "30000",NULL_STR);
							//TC_CMD="tc class add dev $BRIDGE parent 5:1 classid 5:$lan_pkt_mark htb rate ${bandwidth_dl}kbit ceil ${DOWNLINK_SPEED}kbit prio 2 quantum 30000"
						}else{
							RunSystemCmd(NULL_FILE, _tc, _class, _add, _dev, br_interface, _parent, "5:1", _classid, tmp_args1, _htb, _rate, "1kbit", _ceil, tmp_args2, _prio, "2" ,_quantum, "30000", NULL_STR);
							//TC_CMD="tc class add dev $BRIDGE parent 5:1 classid 5:$lan_pkt_mark htb rate 1kbit ceil ${bandwidth_dl}kbit prio 2 quantum 30000"
						}
						RunSystemCmd(NULL_FILE, _tc, _qdisc, _add, _dev, br_interface, _parent, tmp_args1, _handle, tmp_args4, _sfq, _perturb, "10", NULL_STR);
						//TC_CMD="tc qdisc add dev $BRIDGE parent 5:$lan_pkt_mark handle 5$lan_pkt_mark: sfq perturb 10"
						RunSystemCmd(NULL_FILE, _tc, _filter, _add, _dev, br_interface, _parent, "5:0", _protocol2, _ip, _prio, "100", _handle, tmp_args, _fw, _classid, tmp_args1, NULL_STR);
						//TC_CMD="tc filter add dev $BRIDGE parent 5:0 protocol ip prio 100 handle $lan_pkt_mark fw classid 5:$lan_pkt_mark"
						lan_pkt_mark = lan_pkt_mark+1;
					}
#ifdef MULTI_PPPOE
				  }
#endif					
				}
			}
		}
	}

//	RunSystemCmd("/proc/qos", "echo", PROC_QOS, NULL_STR);
#endif
	return 0;
}

int setURLFilter(void)
{
	char keywords[500];
	char cmdBuffer[500];
	char tmp1[40];
	URLFILTER_T entry;
	int entryNum=0, index;
	int mode;
	//printf("set urlfilter\n");
	/*add URL filter Mode 0:Black list 1:White list*/
	apmib_get(MIB_URLFILTER_MODE,  (void *)&mode);
	apmib_get(MIB_URLFILTER_TBL_NUM, (void *)&entryNum);
	sprintf(keywords, "%d ", entryNum);
	for (index=1; index<=entryNum; index++) {
		memset(&entry, '\0', sizeof(entry));
		*((char *)&entry) = (char)index;
		apmib_get(MIB_URLFILTER_TBL, (void *)&entry);
		if(mode!=entry.ruleMode)
			continue;
		if(!strncmp(entry.urlAddr,"http://",7))
			sprintf(tmp1, "%s ", entry.urlAddr+7);
		else
			sprintf(tmp1, "%s ", entry.urlAddr);
#if defined(CONFIG_RTL_FAST_FILTER)
		memset(cmdBuffer, 0, sizeof(cmdBuffer));
		sprintf(cmdBuffer, "rtk_cmd filter add --url-key %s", tmp1);
		system(cmdBuffer);
#else
		strcat(keywords, tmp1);
#endif
	}
		
	//sprintf(cmdBuffer, "%s", keywords);
	//RunSystemCmd("/proc/url_filter", "echo", cmdBuffer, NULL_STR);//disable h/w nat when url filter enabled
#if defined(CONFIG_RTL_FAST_FILTER)
#else
	sprintf(cmdBuffer, "add:0#3 3 %s", keywords);
	RunSystemCmd("/proc/filter_table", "echo", cmdBuffer, NULL_STR);
#endif

	return 0;
}


int setDoS(unsigned long enabled, int op)
{
	char cmdBuffer[500];
	unsigned int *dst, *mask;
	unsigned int synsynflood=0;
	unsigned int sysfinflood=0;
	unsigned int sysudpflood=0;
	unsigned int sysicmpflood=0;
	unsigned int pipsynflood=0;
	unsigned int pipfinflood=0;
	unsigned int pipudpflood=0;
	unsigned int pipicmpflood=0;
	unsigned int blockTime=0;
	struct in_addr curIpAddr={0}, curSubnet={0};

	apmib_get(MIB_DOS_SYSSYN_FLOOD, (void *)&synsynflood);
	apmib_get(MIB_DOS_SYSFIN_FLOOD, (void *)&sysfinflood);
	apmib_get(MIB_DOS_SYSUDP_FLOOD, (void *)&sysudpflood);
	apmib_get(MIB_DOS_SYSICMP_FLOOD, (void *)&sysicmpflood);
	apmib_get(MIB_DOS_PIPSYN_FLOOD, (void *)&pipsynflood);
	apmib_get(MIB_DOS_PIPFIN_FLOOD, (void *)&pipfinflood);
	apmib_get(MIB_DOS_PIPUDP_FLOOD, (void *)&pipudpflood);
	apmib_get(MIB_DOS_PIPICMP_FLOOD, (void *)&pipicmpflood);
	apmib_get(MIB_DOS_BLOCK_TIME, (void *)&blockTime);

	getInAddr("br0", IP_ADDR_T, (void *)&curIpAddr);
    getInAddr("br0", NET_MASK_T, (void *)&curSubnet);
  	//apmib_get(MIB_IP_ADDR,  (void *)ipbuf);
  	dst = (unsigned int *)&curIpAddr;
  	//apmib_get( MIB_SUBNET_MASK,  (void *)maskbuf);
  	mask = (unsigned int *)&curSubnet;
  	if(op==2){
  		sprintf(cmdBuffer, "echo \" 2 %X %X %ld %d %d %d %d %d %d %d %d %d\" >  /proc/enable_dos", *dst, *mask, enabled, synsynflood, sysfinflood, sysudpflood, sysicmpflood, pipsynflood, pipfinflood, pipudpflood, pipicmpflood, blockTime);
  		  system(cmdBuffer);
  	}else{
  		sprintf(cmdBuffer, "echo \" 0 %X %X %ld %d %d %d %d %d %d %d %d %d\" >  /proc/enable_dos", (*dst & 0xFFFFFF00), *mask, enabled, synsynflood, sysfinflood, sysudpflood, sysicmpflood, pipsynflood, pipfinflood, pipudpflood, pipicmpflood, blockTime);
  		  system(cmdBuffer);
	}
return 0;


}

int setIpFilter(void)
{
	int entryNum=0, index;
	IPFILTER_T entry;
	char ipAddr[30];
	char *tmpStr;
#if defined(CONFIG_RTL_FAST_FILTER)
	char protocol[10];
	char cmdBuffer[120];
#endif

	apmib_get(MIB_IPFILTER_TBL_NUM, (void *)&entryNum);

	for(index=1; index <= entryNum ; index++) {
		memset(&entry, '\0', sizeof(entry));
		*((char *)&entry) = (char)index;
		apmib_get(MIB_IPFILTER_TBL, (void *)&entry);

		tmpStr = inet_ntoa(*((struct in_addr *)entry.ipAddr));
		sprintf(ipAddr, "%s", tmpStr);
#if defined(CONFIG_RTL_FAST_FILTER)
		memset(protocol, 0, sizeof(protocol));
		memset(cmdBuffer, 0, sizeof(cmdBuffer));
		if(entry.protoType==PROTO_TCP){
			sprintf(protocol, "tcp");
		}
		else if(entry.protoType==PROTO_UDP){
			sprintf(protocol, "udp");
		}
		else if(entry.protoType==PROTO_BOTH)	{
			sprintf(protocol, "tcp_udp");
		}
		sprintf(cmdBuffer, "rtk_cmd filter add --ip-src %s --protocol %s", ipAddr, protocol);
		system(cmdBuffer);
#else

		if(entry.protoType==PROTO_TCP){
			RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, _protocol, _tcp, _src, ipAddr, jump, DROP, NULL_STR);
		}
		if(entry.protoType==PROTO_UDP){
			RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, _protocol, _udp, _src, ipAddr, jump, DROP, NULL_STR);
			RunSystemCmd(NULL_FILE, Iptables, ADD, INPUT, _protocol, _udp, dport, "53:53", _src, ipAddr, jump, DROP, NULL_STR);
		}
		if(entry.protoType==PROTO_BOTH)	{
			RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, _protocol, _tcp, _src, ipAddr, jump, DROP, NULL_STR);
			RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, _protocol, _udp, _src, ipAddr, jump, DROP, NULL_STR);
			RunSystemCmd(NULL_FILE, Iptables, ADD, INPUT, _protocol, _udp, dport, "53:53", _src, ipAddr, jump, DROP, NULL_STR);
		}
#endif

	}
	return 0;
}

int setMACFilter(void)
{
	char macEntry[30];
	int entryNum=0, index;
	MACFILTER_T entry;
	char cmdBuffer[80];

	apmib_get(MIB_MACFILTER_TBL_NUM, (void *)&entryNum);

	for (index=1; index<=entryNum; index++) {
		memset(&entry, '\0', sizeof(entry));
		*((char *)&entry) = (char)index;
		apmib_get(MIB_MACFILTER_TBL, (void *)&entry);
		sprintf(macEntry,"%02X:%02X:%02X:%02X:%02X:%02X", entry.macAddr[0], entry.macAddr[1], entry.macAddr[2], entry.macAddr[3], entry.macAddr[4], entry.macAddr[5]);
#if defined(CONFIG_RTL_FAST_FILTER)
		memset(cmdBuffer, 0, sizeof(cmdBuffer));
		sprintf(cmdBuffer, "rtk_cmd filter add --mac-src %s", macEntry);
		system(cmdBuffer);
#else
		RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, match, "mac" ,mac_src, macEntry, jump, DROP, NULL_STR);
		RunSystemCmd(NULL_FILE, Iptables, ADD, INPUT, match, "mac" ,mac_src, macEntry, jump, DROP, NULL_STR);
#endif

		memset(cmdBuffer, 0, sizeof(cmdBuffer));
		sprintf(cmdBuffer, "rtk_cmd igmp_delete %02X:%02X:%02X:%02X:%02X:%02X", entry.macAddr[0], entry.macAddr[1], entry.macAddr[2], entry.macAddr[3], entry.macAddr[4], entry.macAddr[5]);
		system(cmdBuffer);
	}

	return 0;

}



int setPortFilter(void)
{
	char PortRange[30];
	//int DNS_Filter=0;
	int entryNum=0,index;
	PORTFILTER_T entry;
#if defined(CONFIG_RTL_FAST_FILTER)
	char protocol[10];
	char cmdBuffer[120];
#endif
	apmib_get(MIB_PORTFILTER_TBL_NUM, (void *)&entryNum);
	for (index=1; index<=entryNum; index++) {
		memset(&entry, '\0', sizeof(entry));
		*((char *)&entry) = (char)index;
		apmib_get(MIB_PORTFILTER_TBL, (void *)&entry);
		sprintf(PortRange, "%d:%d", entry.fromPort, entry.toPort);
#if defined(CONFIG_RTL_FAST_FILTER)
		memset(protocol, 0, sizeof(protocol));
		memset(cmdBuffer, 0, sizeof(cmdBuffer));

		if(entry.protoType==PROTO_TCP){
			sprintf(protocol, "tcp");
		}
		else if(entry.protoType==PROTO_UDP){
			sprintf(protocol, "udp");
		}
		else if(entry.protoType==PROTO_BOTH){
			sprintf(protocol, "tcp_udp");
		}
		sprintf(cmdBuffer, "rtk_cmd filter add --port-range-dst %d:%d --protocol %s", entry.fromPort, entry.toPort, protocol);
		system(cmdBuffer);
#else

		if(entry.protoType==PROTO_TCP){
			RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, _protocol, _tcp, dport, PortRange, jump, DROP, NULL_STR);
		}
		if(entry.protoType==PROTO_UDP){
			if(entry.fromPort<53 && entry.toPort >= 53)
				RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, _protocol, _udp, dport, "53:53", jump, DROP, NULL_STR);
			RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, _protocol, _udp, dport, PortRange, jump, DROP, NULL_STR);
		}
		if(entry.protoType==PROTO_BOTH)	{
			RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, _protocol, _tcp, dport, PortRange, jump, DROP, NULL_STR);
			RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, _protocol, _udp, dport, PortRange, jump, DROP, NULL_STR);
			if(entry.fromPort<53 && entry.toPort >= 53)
				RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, _protocol, _udp, dport, "53:53", jump, DROP, NULL_STR);
		}
#endif
		/*
		if(DNS_Filter==0){
			if(entry.fromPort<= 53 &&  entry.toPort >= 53){
				if(entry.protoType==PROTO_BOTH || (entry.protoType==PROTO_UDP)){
					RunSystemCmd(NULL_FILE, Iptables, ADD, INPUT, _protocol, _udp, dport, "53", jump, DROP, NULL_STR);
				}
			}
			DNS_Filter=1;
		}
		*/

	}
	return 0;
}

int setPortForward(char *pIfaceWan, char *pIpaddrWan)
{
	char PortRange[60];
	char ip[30];
	char *tmpStr;
	int entryNum=0, index;
	PORTFW_T entry;
	int l2tp_vpn=0;
	int pptp_vpn=0;
	int ipsec_vpn=0;
#if defined(CONFIG_RTL_FAST_FILTER)
	char protocol[10];
	char cmdBuffer[120];
#endif

	apmib_get(MIB_PORTFW_TBL_NUM, (void *)&entryNum);
	for (index=1; index<=entryNum; index++) {
		memset(&entry, '\0', sizeof(entry));
		*((char *)&entry) = (char)index;
		apmib_get(MIB_PORTFW_TBL, (void *)&entry);

		tmpStr =	inet_ntoa(*((struct in_addr *)entry.ipAddr));
		sprintf(ip, "%s", tmpStr);

		sprintf(PortRange, "%d:%d", entry.fromPort, entry.toPort);
#if 0 //defined(CONFIG_RTL_FAST_FILTER)
		memset(protocol, 0, sizeof(protocol));
		memset(cmdBuffer, 0, sizeof(cmdBuffer));

		if(entry.protoType ==PROTO_TCP){
			sprintf(protocol, "tcp");
		}
		else if(entry.protoType ==PROTO_UDP){
			sprintf(protocol, "udp");
		}
		else if(entry.protoType ==PROTO_BOTH){
			sprintf(protocol, "tcp_udp");
		}
		sprintf(cmdBuffer, "rtk_cmd filter add --ip-dst %s --port-dst-range %d:%d --protocol %s --policy fastpath", ip, entry.fromPort, entry.toPort, protocol);
		system(cmdBuffer);
#else
		if(entry.fromPort<80 && entry.toPort>80)
			RunSystemCmd(NULL_FILE, Iptables, DEL, INPUT, _protocol, _tcp, dport, "80:80", in, pIfaceWan, _dest, pIpaddrWan, jump, DROP, NULL_STR);

		if(entry.protoType ==PROTO_TCP){
			RunSystemCmd(NULL_FILE, Iptables, ADD, PREROUTING, _table, nat_table, _protocol, _tcp, dport, PortRange,_dest, pIpaddrWan, jump, DNAT, "--to", ip, NULL_STR);
			RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, in, pIfaceWan, _dest, ip, _protocol, _tcp, dport, PortRange, jump , ACCEPT, NULL_STR);
		}
		if(entry.protoType ==PROTO_UDP){
			RunSystemCmd(NULL_FILE, Iptables, ADD, PREROUTING, _table, nat_table, _protocol, _udp, dport, PortRange,_dest, pIpaddrWan, jump, DNAT, "--to", ip, NULL_STR);
			RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, in, pIfaceWan, _dest, ip, _protocol, _udp, dport, PortRange, jump , ACCEPT, NULL_STR);


		}
		if(entry.protoType ==PROTO_BOTH){
			RunSystemCmd(NULL_FILE, Iptables, ADD, PREROUTING, _table, nat_table, _protocol, _tcp, dport, PortRange,_dest, pIpaddrWan, jump, DNAT, "--to", ip, NULL_STR);
			RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, in, pIfaceWan, _dest, ip, _protocol, _tcp, dport, PortRange, jump , ACCEPT, NULL_STR);
			RunSystemCmd(NULL_FILE, Iptables, ADD, PREROUTING, _table, nat_table, _protocol, _udp, dport, PortRange,_dest, pIpaddrWan, jump, DNAT, "--to", ip, NULL_STR);
			RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, in, pIfaceWan, _dest, ip, _protocol, _udp, dport, PortRange, jump , ACCEPT, NULL_STR);

		}
#endif
		if(pptp_vpn==0){
			if(entry.fromPort<= 1723 &&  entry.toPort >= 1723){
				if(entry.protoType==PROTO_BOTH || (entry.protoType==PROTO_TCP)){
					RunSystemCmd(NULL_FILE, Iptables, ADD, PREROUTING, _table, nat_table, in, pIfaceWan, _protocol, "gre", _dest, pIpaddrWan , jump, DNAT, "--to", ip, NULL_STR);
					RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, _protocol, "gre", in, pIfaceWan, jump, ACCEPT, NULL_STR);
				}
			}

			pptp_vpn=1;
		}

		if(l2tp_vpn==0){
			if(entry.fromPort<= 1701 &&  entry.toPort >= 1701){
				if(entry.protoType==PROTO_BOTH || (entry.protoType==PROTO_UDP)){
						RunSystemCmd("/proc/nat_l2tp", "echo", "0", NULL_STR);
				}
			}
			l2tp_vpn=1;
		}
		if(ipsec_vpn==0){
			if(entry.fromPort<= 500 &&  entry.toPort >= 500){
				if(entry.protoType==PROTO_BOTH || (entry.protoType==PROTO_UDP)){
					RunSystemCmd(NULL_FILE, Iptables, ADD, PREROUTING, _table, nat_table, _protocol, "esp", _dest, pIpaddrWan, jump, DNAT, "--to", ip, NULL_STR);
					RunSystemCmd(NULL_FILE, Iptables, ADD, PREROUTING, _table, nat_table, _protocol, _udp, dport,"4500", _dest, pIpaddrWan, jump, DNAT, "--to", ip, NULL_STR);
					RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, _protocol, _udp, dport,"4500", jump, ACCEPT, NULL_STR);
					RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, _protocol, "esp", in, pIfaceWan, jump, ACCEPT, NULL_STR);
				}
			}
			ipsec_vpn=1;
		}
	}
	return 0;
}




#if defined(CONFIG_APP_TR069)
void start_tr069(char *interface, char *wan_addr)
{
	int cwmp_flag = 0;
	int conReqPort = 0;
	char acsUrl[CWMP_ACS_URL_LEN+1] = {0};
	char acsUrlRange[2*(CWMP_ACS_URL_LEN+1)] = {0};
	char conReqPortRange[2*(5+1)] = {0};
	char strPID[10];
	int pid=-1;
		
//printf("\r\n wan_addr=[%s],__[%s-%u]\r\n",wan_addr,__FILE__,__LINE__);
	apmib_get( MIB_CWMP_FLAG, (void *)&cwmp_flag );
	
	if(isFileExist(TR069_PID_FILE))
	{
		
		
	}
	else
	{
		if(cwmp_flag & CWMP_FLAG_AUTORUN)
		{
			unsigned char acsUrltmp[CWMP_ACS_URL_LEN+1];
			unsigned char *notifyList;
	
			notifyList=malloc(CWMP_NOTIFY_LIST_LEN);
			
			if(notifyList==NULL)
			{
				fprintf(stderr,"\r\n ERR:notifyList malloc fail! __[%s-%u]",__FILE__,__LINE__);
			}
			else
			{
				char *lineptr = NULL;
				char *str;
				int firstline = 1;
				
				
				memset(notifyList,0x00,CWMP_NOTIFY_LIST_LEN);
				apmib_get(MIB_CWMP_NOTIFY_LIST,(void *)notifyList);
				
				if(strlen(notifyList) == 0)
				{
					system("echo \"\" > /var/CWMPNotify.txt");
					
				}
				else
				{
				
					lineptr = notifyList;
					
					// A1]A2]A3[B1]B2]B3
					str = strsep(&lineptr,"[");
					
					//A1]A2]A3
					while(str != NULL)
					{
						char *strptr = str;
						char *str1,*str2,*str3;
						char tmpStr[5];
						char *insertStr=NULL;
						
						insertStr=malloc(strlen(str));
						
						if(insertStr != NULL)
						{
							memset(insertStr,0x00,strlen(str));
												
							//A1]A2]A3
							str1 = strsep(&strptr,"]");
							sprintf(insertStr,"%s",str1);
							//A1
							
							//A2]A3
							str2 = strsep(&strptr,"]");
							//A2
							memset(tmpStr,0x00,sizeof(tmpStr));
							sprintf(tmpStr," %s",str2);
							strcat(insertStr,tmpStr);
							
							//A3
							str3 = strsep(&strptr,"]");
							//A3
							memset(tmpStr,0x00,sizeof(tmpStr));
							sprintf(tmpStr," %s\n",str3);
							strcat(insertStr,tmpStr);
		
	//fprintf(stderr,"\r\n insertStr=[%s] __[%s-%u]",insertStr,__FILE__,__LINE__);
																	
							
							if(firstline == 1)
								write_line_to_file("/var/CWMPNotify.txt", 1, insertStr);
							else
								write_line_to_file("/var/CWMPNotify.txt", 2, insertStr);
							
							firstline = 0;
							
							if(insertStr)
								free(insertStr);
														
						}
						
						str = strsep(&lineptr,"["); //get next line
					}
				}
				
				if(notifyList)
					free(notifyList);
			}

			apmib_get( MIB_CWMP_ACS_URL, (void *)acsUrltmp);
			
			system("/bin/cwmpClient &");
			//memset(acsURLStr,0x00,sizeof(acsURLStr));
			sprintf(acsURLStr,"%s",acsUrltmp);
		}
		
	}
		
	if(cwmp_flag & CWMP_FLAG_AUTORUN)
	{
		apmib_get( MIB_CWMP_CONREQ_PORT, (void *)&conReqPort);
		if(conReqPort >0 && conReqPort<65535)
		{
			char tmpStr[CWMP_ACS_URL_LEN] = {0};
			
			apmib_get( MIB_CWMP_ACS_URL, (void *)acsUrl);
//printf("\r\n acsUrl=[%s],__[%s-%u]\r\n",acsUrl,__FILE__,__LINE__);
			if((strstr(acsUrl,"https://") != 0 || strstr(acsUrl,"http://") != 0) && strlen(acsUrl) != 0)
			{
				char *lineptr = acsUrl;
				char *str=NULL;

//printf("\r\n lineptr=[%s],__[%s-%u]\r\n",lineptr,__FILE__,__LINE__);
				
				str = strsep(&lineptr,"/");
//printf("\r\n str=[%s],__[%s-%u]\r\n",str,__FILE__,__LINE__);
				str = strsep(&lineptr,"/");
//printf("\r\n str=[%s],__[%s-%u]\r\n",str,__FILE__,__LINE__);
				str = strsep(&lineptr,"/");
//printf("\r\n str=[%s],__[%s-%u]\r\n",str,__FILE__,__LINE__);

				if(str != NULL && strlen(str) != 0)
				{
					sprintf(acsUrlRange,"%s-%s",str,str);
					
					sprintf(conReqPortRange,"%d:%d",conReqPort,conReqPort);
					//iptables -A INPUT -p tcp -m iprange --src-range $ACS_URL-$ACS_URL --dport $CWMP_CONREQ_PORT:$CWMP_CONREQ_PORT -i $WAN -d $EXT_IP -j ACCEPT	  
					//printf("\r\n acsUrlRange=[%s],__[%s-%u]\r\n",acsUrlRange,__FILE__,__LINE__);			
					//printf("\r\n conReqPortRange=[%s],__[%s-%u]\r\n",conReqPortRange,__FILE__,__LINE__);			
					//printf("\r\n interface=[%s],__[%s-%u]\r\n",interface,__FILE__,__LINE__);			
					//printf("\r\n wan_addr=[%s],__[%s-%u]\r\n",wan_addr,__FILE__,__LINE__);			
					//iptables -A INPUT -p tcp --dport 4567:4567 -i eth1 -d 172.21.69.21 -j ACCEPT	 		
					//RunSystemCmd(NULL_FILE, "iptables", "-A", "INPUT", "-p", "tcp", "-m", "iprange", "--src-range", acsUrlRange, "--dport", conReqPortRange, "-i", interface, "-d", wan_addr, "-j", "ACCEPT", NULL_STR);
					RunSystemCmd(NULL_FILE, "iptables", "-A", "INPUT", "-p", "tcp", "--dport", conReqPortRange, "-i", interface, "-d", wan_addr, "-j", "ACCEPT", NULL_STR);
				}
			}			
		}				
	}	
}
#endif
#ifdef MULTI_PPPOE
void setMulPppoeRules(int argc, char** argv)
{
	//dzh add for multi-pppoe route set and lan-partition set	
	if(argc >=3 && argv[2] && (strcmp(argv[2], "pppoe")==0))
	{							
		system("ifconfig |grep 'P-t-P' | cut  -d ':' -f 2 | cut -d ' ' -f 1 > /etc/ppp/ppp_local");			
		system("ifconfig |grep 'P-t-P' | cut  -d ':' -f 3 | cut -d ' ' -f 1 > /etc/ppp/ppp_remote");
		system("ifconfig |grep 'ppp'| cut -d ' ' -f 1 > /etc/ppp/ppp_device");			
		system("cat /etc/ppp/ppp_local | wc -l > /etc/ppp/lineNumber");					
		if(0 == get_info())
		{		
 #ifdef MULTI_PPP_DEBUG		
			printf("get info error\n");
 #endif
			return ;
		}
		//print_info();
		if(argc >=4 && argv[3]) //if exist pppoe interface,set interface information
		{
			//set route 
			int index ;
			char command[100];
			char flushCmd[100];
			for( index = 0 ; index < pppNumbers ; ++index)
			{
			 	#ifdef MULTI_PPP_DEBUG	
				printf("the ppp_name is:%s\n",infos[index].ppp_name);
				printf("the argv[3] is:%s\n",argv[3]);			
				#endif
				if(!strcmp(infos[index].ppp_name,argv[3]))//match the interface
				{
					int sub_index;								
					//set subnet rules char SubNet[4][30];
					//SubNet[infos[index].order-1];
					/*
					for(sub_index = 0 ; sub_index < SubNets[infos[index].order-1].SubnetCount; ++sub_index)
					{						
						//-i eth0
						sprintf(command,
							"iptables -t mangle -A PREROUTING -i eth0 -m iprange --src-range %s-%s -j MARK --set-mark %d",
							SubNets[infos[index].order-1].startip[sub_index],SubNets[infos[index].order-1].endip[sub_index],
							infos[index].order+sub_index+100);		
						printf("%s\n",command); 					
						system(command);
						
						sprintf(command,"ip rule add fwmark %d table %d pref %d",
							infos[index].order+sub_index+100,
							infos[index].order+30,
							infos[index].order+sub_index+100);
						printf("%s\n",command); 					
						system(command);

						sprintf(command,"iptables -t nat -A POSTROUTING  -m iprange --src-range %s-%s -o %s -j MASQUERADE",
								SubNets[infos[index].order-1].startip[sub_index],SubNets[infos[index].order-1].endip[sub_index],
									infos[index].ppp_name);
						printf("%s\n",command); 					
						system(command);

					}						
					*/
					
					FILE* pF;// = fopen("/etc/ppp/flushCmds","w+");
					char path[50];
					sprintf(path,"/etc/ppp/%s.cmd",argv[3]);
					pF = fopen(path,"wt");
						
					system("ip rule del table 100 >/dev/null 2>&1");
					system("ip route del table 100 >/dev/null 2>&1");
					system(" ip rule add from  192.168.1.0/24 table 100 prio 32765");					
					system("ip route add default dev br0 table 100");		
					
					#ifdef MULTI_PPP_DEBUG					
					printf("%s\n",command); 					
					#endif	
					
					//system(command);					
					
					sprintf(command,"ip rule add from %s table %d",
						SubNet[infos[index].order-1],
						infos[index].order+30);
					#ifdef MULTI_PPP_DEBUG
					printf("%s\n",command); 					
					#endif
					system(command);

					//flush command
					fprintf(pF,"ip rule del table %d >/dev/null 2>&1 \n",infos[index].order+30);																							
					//iptables -A POSTROUTING -t nat  -s 192.168.1.0/25 -o ppp0 -j MASQUERADE

					sprintf(command,"iptables -A POSTROUTING -t nat -s %s -o %s -j MASQUERADE",
						SubNet[infos[index].order-1],
						infos[index].ppp_name);
					#ifdef MULTI_PPP_DEBUG					
					printf("%s\n",command); 	
					#endif
					system(command);								
					
					//set route
					
					sprintf(command,"ip route add %s dev %s table %d",infos[index].server_ip,
							infos[index].ppp_name,infos[index].order+10);
					#ifdef MULTI_PPP_DEBUG					
					printf("%s\n",command); 
					#endif
					system(command);	

					fprintf(pF,"ip route del table %d >/dev/null 2>&1 \n",infos[index].order+10);
					
					sprintf(command,"ip rule add from %s table %d",infos[index].client_ip,
						infos[index].order+10); 
					#ifdef MULTI_PPP_DEBUG					
					printf("%s\n",command); 	
					#endif
					system(command);	

					fprintf(pF,"ip rule del table %d >/dev/null 2>&1 \n",infos[index].order+10);								
					//set lan-partion					
					sprintf(command,"ip route add default via %s dev %s table %d",
						infos[index].server_ip,infos[index].ppp_name,
							infos[index].order+30);

					fprintf(pF,"ip route del table %d >/dev/null 2>&1 \n",infos[index].order+30);	
					#ifdef MULTI_PPP_DEBUG				
					printf("%s\n",command); 					
					#endif
					system(command);

					sprintf(command,"ip route add %s dev br0 table %d",Br0NetSectAddr,
							infos[index].order+30);
					#ifdef MULTI_PPP_DEBUG					
					printf("%s\n",command); 					
					#endif
					system(command);
					//iptables -A POSTROUTING -t nat -m iprange --src-range 192.168.1.1-192.168.1.50 -o ppp0 -j MASQUERADE	
					break;
				}//end if				
			}//end for
		}//end if		
		
	}//end if	
}
#endif
void setRulesWithOutDevice(int opmode, int wan_dhcp , char* pInterface_wanPhy,char* Interface_wanPhy)
{
	int intVal=0, natEnabled=0;
	int intVal1=0;
	int intVal2 = 0;
	int dyn_rt_support=0;
	int intVal_num=0;
	int hw_nat_support=0;
	int my_wan_type = 0;
	unsigned long	dos_enabled = 0;	
	char wan_type[8];
	int mode;
#ifdef CONFIG_RTL_HW_NAPT
	int ivalue = 0; 
#endif		
	RunSystemCmd("/proc/sys/net/ipv4/ip_forward", "echo", "0", NULL_STR);//don't enable ip_forward before set MASQUERADE
	RunSystemCmd("/proc/fast_nat", "echo", "2", NULL_STR);//clean conntrack table before set new rules
	RunSystemCmd(NULL_FILE, Iptables, FLUSH, NULL_STR);
	RunSystemCmd(NULL_FILE, Iptables,_table, nat_table, FLUSH, POSTROUTING, NULL_STR);
	RunSystemCmd(NULL_FILE, Iptables,_table, nat_table, FLUSH, PREROUTING, NULL_STR);
	RunSystemCmd(NULL_FILE, Iptables, FLUSH, _table, mangle_table, NULL_STR);		
	RunSystemCmd(NULL_FILE, Iptables, FLUSH, INPUT, NULL_STR);		
	RunSystemCmd(NULL_FILE, Iptables, FLUSH, OUTPUT, NULL_STR);	
	RunSystemCmd(NULL_FILE, Iptables, FLUSH, FORWARD, NULL_STR);
	RunSystemCmd(NULL_FILE, Iptables, POLICY, OUTPUT, ACCEPT, NULL_STR);

	if(opmode != BRIDGE_MODE){
		RunSystemCmd(NULL_FILE, Iptables, POLICY, INPUT, DROP, NULL_STR);
	}else{
		RunSystemCmd(NULL_FILE, Iptables, POLICY, INPUT, ACCEPT, NULL_STR);
	}
	if(opmode != 3){
		RunSystemCmd(NULL_FILE, Iptables, POLICY, FORWARD, DROP, NULL_STR);
	}else{
		RunSystemCmd(NULL_FILE, Iptables, POLICY, FORWARD, ACCEPT, NULL_STR);
	}

	//#redefine url and schedule filter

	if(isFileExist("/bin/routed")){
		dyn_rt_support=1;
	}
	if(isFileExist("/proc/hw_nat")){
		hw_nat_support=1;
	}
	if(dyn_rt_support ==1 && opmode != BRIDGE_MODE){
		apmib_get(MIB_NAT_ENABLED, (void *)&natEnabled);
		if(natEnabled==0){
			RunSystemCmd(NULL_FILE, Iptables, POLICY, INPUT, ACCEPT, NULL_STR);
			RunSystemCmd(NULL_FILE, Iptables, POLICY, FORWARD, ACCEPT, NULL_STR);
			RunSystemCmd("/proc/fast_nat", "echo", "0", NULL_STR);//disable fastpath when nat is disabled
			return 0;
		}
	}
	if(opmode == BRIDGE_MODE)
		return 0;

	//url filter setting
	apmib_get(MIB_URLFILTER_ENABLED,  (void *)&intVal);
	apmib_get(MIB_URLFILTER_TBL_NUM,  (void *)&intVal_num);
	apmib_get(MIB_URLFILTER_MODE,  (void *)&mode);
#if defined(CONFIG_RTL_FAST_FILTER)
	system("rtk_cmd filter flush");
#else
	RunSystemCmd("/proc/filter_table", "echo", "flush", NULL_STR);
	RunSystemCmd("/proc/filter_table", "echo", "init", "3",  NULL_STR);
	if(mode)
		RunSystemCmd("/proc/filter_table", "echo", "white", NULL_STR);
	else
		RunSystemCmd("/proc/filter_table", "echo", "black", NULL_STR);
#endif

	if(intVal !=0 && intVal_num>0){
//		RunSystemCmd("/proc/url_filter", "echo", " ", NULL_STR);
		setURLFilter();
#if 0
defined(CONFIG_RTL_HW_NAPT)
		if(opmode==0){
			RunSystemCmd("/proc/hw_nat", "echo", "0", NULL_STR);//disable h/w nat when url filter enabled
		}
#endif
	}else{
//		RunSystemCmd("/proc/url_filter", "echo", "0", NULL_STR);//disable url filter
#if defined(CONFIG_RTL_FAST_FILTER)
#else
		RunSystemCmd("/proc/filter_table", "echo", "flush", NULL_STR);
#endif
#if 0
defined(CONFIG_RTL_HW_NAPT)
		if(opmode==0){
			apmib_get(MIB_SUBNET_MASK,(void*)&ivalue);
			if((ivalue&HW_NAT_LIMIT_NETMASK)!=HW_NAT_LIMIT_NETMASK)
			{
					RunSystemCmd("/proc/hw_nat", "echo", "0", NULL_STR);
			}
			else
			{
				RunSystemCmd("/proc/hw_nat", "echo", "1", NULL_STR);//enable h/w nat when url filter disable
			}
		}
#endif

	}
#if defined(CONFIG_RTL_HW_NAPT)

	RunSystemCmd("/proc/hw_nat", "echo", "9", NULL_STR);
	my_wan_type = 0;
	my_wan_type = wan_dhcp + 80;
	sprintf(wan_type, "%d", my_wan_type);
	RunSystemCmd("/proc/hw_nat", "echo", wan_type, NULL_STR);
#else
	RunSystemCmd("/proc/sw_nat", "echo", "9", NULL_STR);
#endif

	////////////////////////////////////////////////////
	//ip filter setting
	intVal = 0;
	apmib_get(MIB_IPFILTER_ENABLED,  (void *)&intVal);
	apmib_get(MIB_IPFILTER_TBL_NUM,  (void *)&intVal_num);
	if(intVal ==1 && intVal_num>0){
			//set ip filter
			setIpFilter();
	}

	intVal = 0;
	apmib_get(MIB_MACFILTER_ENABLED,  (void *)&intVal);
	apmib_get(MIB_MACFILTER_TBL_NUM, (void *)&intVal_num);
	if(intVal==1 && intVal_num>0){
		//set mac filter
		setMACFilter();
	}

	intVal=0;
	apmib_get(MIB_PORTFILTER_ENABLED,  (void *)&intVal);
	apmib_get(MIB_PORTFILTER_TBL_NUM, (void *)&intVal_num);
	if(intVal==1 && intVal_num>0){
		setPortFilter();
	}	
	///////////////////////////////////////////////////////////
	apmib_get(MIB_VPN_PASSTHRU_L2TP_ENABLED, (void *)&intVal);
	if(intVal ==0){
		RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, _protocol, _udp, dport, "1701", jump, DROP, NULL_STR);
	}
	else if(intVal == 1){
		RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, _protocol, _udp, sport, "1701", jump, ACCEPT, NULL_STR);
		RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, _protocol, _udp, dport, "1701", jump, ACCEPT, NULL_STR);
	}
	apmib_get(MIB_VPN_PASSTHRU_PPTP_ENABLED, (void *)&intVal2);
	if(intVal2 ==0){
		RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, _protocol, _tcp, dport, "1723", jump, DROP, NULL_STR);
	}
	else if(intVal2 == 1){
		RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, _protocol, _tcp, dport, "1723", jump, ACCEPT, NULL_STR);
		RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, _protocol, _tcp, sport, "1723", jump, ACCEPT, NULL_STR);
		RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, _protocol, "47", jump, ACCEPT, NULL_STR); //GRE
	}

	if((intVal == 1) || (intVal2 == 1)){
	//	RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, _protocol, _icmp, jump, ACCEPT, NULL_STR);
	//	RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, in, pInterface_wanPhy, jump, ACCEPT, NULL_STR);
	}
	///////////////////////////////////////////////////////////
	RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, _protocol, _udp, match, _udp, in, Interface_wanPhy, "--destination" , "224.0.0.0/4", jump, ACCEPT, NULL_STR);	
	///////////////////////////////////////////////////////////
	RunSystemCmd(NULL_FILE, Iptables, ADD, INPUT, match, mstate, state, RELATED_ESTABLISHED, jump, ACCEPT, NULL_STR);
	//iptables -I FORWARD -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu
	RunSystemCmd(NULL_FILE, Iptables, INSERT, FORWARD, _protocol, _tcp, tcp_flags, MSS_FLAG1, MSS_FLAG2, jump, TCPMSS, clamp, NULL_STR);	
	///////////////////////////////////////////////////////////
	RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, in, "br0", jump, ACCEPT, NULL_STR);	
	///////////////////////////////////////////////////////////
	if(wan_dhcp==4){			
		RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, in, pInterface_wanPhy, match, mstate, state, RELATED_ESTABLISHED, jump, ACCEPT, NULL_STR);
	}
	///////////////////////////////////////////////////////////
	
	RunSystemCmd("/tmp/firewall_igd", "echo", "1", NULL_STR);//disable fastpath when port filter is enabled

	apmib_get(MIB_DOS_ENABLED, (void *)&dos_enabled);
	if(dos_enabled > 0){
#if 0
defined(CONFIG_RTL_HW_NAPT)
	if(opmode == GATEWAY_MODE)
		RunSystemCmd("/proc/hw_nat", "echo", "0", NULL_STR);
#endif
	}
	setDoS(dos_enabled, opmode);
	if(wan_dhcp==PPTP){
	
		RunSystemCmd(NULL_FILE, Iptables, _table, nat_table, ADD, POSTROUTING, out, pInterface_wanPhy, jump, MASQUERADE, NULL_STR);
	}

	if(wan_dhcp == L2TP){
		
		RunSystemCmd(NULL_FILE, Iptables, _table, nat_table, ADD, POSTROUTING, out, pInterface_wanPhy, jump, MASQUERADE, NULL_STR);
	}
}
int setFirewallIptablesRules(int argc, char** argv)
{
	int opmode=-1;
	int wan_dhcp=-1;
	char iface[20], *pInterface="eth1";
	char *pInterface_wanPhy="eth1";
	char Interface_wanPhy[20]="eth1";
	int wlaniface=0, get_wanip=0;
	struct in_addr wanaddr;
	char IpAddr[30], *strIp;
	char WanIpAddr[30], *strWanIp;
	char WanPhyIpAddr[30];
	char IpAddrBuf[30];
	int intVal=0, natEnabled=0;
	int intVal1=0;
	int intVal2 = 0;
	unsigned long	dos_enabled = 0;
	int dyn_rt_support=0;
	int intVal_num=0;
	int hw_nat_support=0;
	int my_wan_type = 0;	
#ifdef MULTI_PPPOE
	int isNeedSetOnce = 1;
#endif
//	int wan_type=0;
	//add for DMZ 
	//echo 192.168.1.0/24 >/etc/ppp/br0_info
	char *br0info[50];
	char *bInterface = "br0";
	int get_br0ip =0;
	int get_br0mask =0;
	char Br0NetSectAddr[30];
	char * strbr0Ip ,* strbr0Mask ;
	struct in_addr br0addr,br0mask;
	unsigned int numofone ;	
	char NumStr[10]; 
	struct stat status;
#ifdef CONFIG_RTL_HW_NAPT
	int ivalue = 0; 
#endif

	printf("Init Firewall Rules....\n");
#ifdef MULTI_PPPOE	

	wait_lable:
	if (isFileExist("/etc/ppp/firewall_lock") == 1) {	
		system("rm -f /etc/ppp/firewall_lock");
	}else{
		sleep(1);
		goto wait_lable;
	}
	if(argc >=4 && argv[3] && (!strncmp(argv[3],"ppp",3)))
	{
		if(!isFileExist("/etc/ppp/hasPppoedevice"))
		{
			system("echo 1 > /etc/ppp/hasPppoedevice");
			isNeedSetOnce = 1;		
		}else{		
			isNeedSetOnce = 0;		
		}
	}
#endif
	memset(WanPhyIpAddr,'\0',30);
	apmib_get(MIB_OP_MODE, (void *)&opmode);
	apmib_get(MIB_WAN_DHCP, (void *)&wan_dhcp);
	if(opmode == WISP_MODE){
		apmib_get(MIB_WISP_WAN_ID, (void *)&wlaniface);
		sprintf(iface, "wlan%d", wlaniface);
#if defined(CONFIG_SMART_REPEATER)			
		getWispRptIfaceName(iface,wlaniface);
		//strcat(iface, "-vxd");
#endif
		pInterface = iface;
		pInterface_wanPhy=iface;
		if (wan_dhcp == PPPOE || wan_dhcp == PPTP || wan_dhcp == L2TP )
#ifdef MULTI_PPPOE
			if(argc >=4 && argv[3])
				pInterface = argv[3];
			else 
				pInterface="ppp0";
#else
			pInterface="ppp0";
#endif
	}else{
		if(opmode == GATEWAY_MODE){
			if (wan_dhcp == PPPOE || wan_dhcp == PPTP || wan_dhcp == L2TP || wan_dhcp == USB3G)
#ifdef MULTI_PPPOE
			if(argc >=4 && argv[3])
				pInterface = argv[3];
			else 
				pInterface="ppp0";
#else
			pInterface="ppp0";
#endif
		}
	}
	get_wanip = getInAddr(pInterface, IP_ADDR_T, (void *)&wanaddr);
	if( get_wanip ==0){   //get wan ip fail
		printf("No wan ip currently!\n");
		goto EXIT_setFirewallIptablesRules;
	}else{
		strWanIp = inet_ntoa(wanaddr);
		strcpy(WanIpAddr, strWanIp);
	}

	//flush fast natp table
	//RunSystemCmd("/proc/net/flush_conntrack", "echo", "1", NULL_STR);
#ifdef MULTI_PPPOE
	if(isNeedSetOnce)
		setRulesWithOutDevice(opmode,wan_dhcp,pInterface_wanPhy,Interface_wanPhy);
#else
		setRulesWithOutDevice(opmode,wan_dhcp,pInterface_wanPhy,Interface_wanPhy);
#endif

	intVal = 0;
	apmib_get( MIB_WEB_WAN_ACCESS_ENABLED, (void *)&intVal);
	if(intVal==1){
		RunSystemCmd(NULL_FILE, Iptables, ADD, INPUT, _protocol, _tcp,  dport, "80:80", in, pInterface, _dest, WanIpAddr, jump, ACCEPT, NULL_STR);
	}else{
		RunSystemCmd(NULL_FILE, Iptables, ADD, INPUT, _protocol, _tcp,  dport, "80:80", in, pInterface, _dest, WanIpAddr, jump, DROP, NULL_STR);
	}


	// SNMP setting
#ifdef CONFIG_SNMP
	intVal = 0;
	apmib_get(MIB_SNMP_ENABLED, (void *)&intVal);
	if (intVal == 1) {
		RunSystemCmd(NULL_FILE, Iptables, ADD, INPUT, _protocol, _udp, dport, "161:161", in, pInterface, _dest, WanIpAddr, jump, ACCEPT, NULL_STR);
		/*???where script*/
		RunSystemCmd(NULL_FILE, Iptables, ADD, PREROUTING, _table, nat_table, in, pInterface, _protocol, _udp, dport, "161", _dest, WanIpAddr, jump, REDIRECT, "--to-port", "161", NULL_STR);
	}
#endif



	apmib_get(MIB_PORTFW_ENABLED,  (void *)&intVal);
	apmib_get(MIB_PORTFW_TBL_NUM, (void *)&intVal_num);
	if(intVal==1 && intVal_num>0){
		setPortForward(pInterface, WanIpAddr);
	}

#if 0
	// Move to set_init
	apmib_get(MIB_CUSTOM_PASSTHRU_ENABLED, (void *)&intVal);
	RunSystemCmd("/proc/custom_Passthru", "echo", (intVal & 0x1)?"1":"0", NULL_STR);
#endif


	//dzh modify
//	apmib_get(MIB_WAN_DHCP,(void *)&wan_type);
	if(wan_dhcp==PPTP)
	{
		RunSystemCmd(NULL_FILE, Iptables, ADD, INPUT, _protocol, "47", in, pInterface_wanPhy, jump, ACCEPT, NULL_STR);		
	}
	else if(wan_dhcp==L2TP)
	{
		RunSystemCmd(NULL_FILE, Iptables, ADD, INPUT, _protocol, _udp, sport ,"1701" ,in, pInterface_wanPhy, jump, ACCEPT, NULL_STR);
	}

	apmib_get(MIB_VPN_PASSTHRU_IPSEC_ENABLED, (void *)&intVal);
	if(intVal ==0){
#ifdef MULTI_PPPOE	
	if(isNeedSetOnce){
#endif			
		RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, _protocol, _udp, dport, "500", jump, DROP, NULL_STR);
#ifdef MULTI_PPPOE	
	}
#endif	
	}else{
		RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, _protocol, _udp, dport, "500", in ,pInterface, out, "br0", jump, ACCEPT, NULL_STR);
	}

	if(wan_dhcp == DHCP_CLIENT || wan_dhcp == PPTP || wan_dhcp == L2TP)
	{
		RunSystemCmd(NULL_FILE, Iptables, ADD, INPUT, _protocol, _udp, dport, "68", in ,pInterface, jump, ACCEPT, NULL_STR);	
	}
	//add for DMZ
	get_br0ip = getInAddr(bInterface, IP_ADDR_T,(void *)&br0addr);
	if(get_br0ip ==0 ){
		printf("No ip currently!\n");
		goto EXIT_setFirewallIptablesRules;
	}
	get_br0mask = getInAddr(bInterface, NET_MASK_T,(void *)&br0mask);
	if( get_br0mask ==0 ){
		printf("No MASK currently!\n");
		goto EXIT_setFirewallIptablesRules;
	}
	br0addr.s_addr &= br0mask.s_addr ;
	for(numofone =0;br0mask.s_addr;++numofone)
		br0mask.s_addr &= br0mask.s_addr-1;
	sprintf (NumStr, "%d", numofone);
	strcpy(Br0NetSectAddr,inet_ntoa(br0addr));
	strcat(Br0NetSectAddr,"/");
	strcat(Br0NetSectAddr,NumStr);
	//echo 192.168.1.0/24 >/etc/ppp/br0_info
//	char *br0info[50];
#ifdef MULTI_PPPOE
	sprintf(br0info,"echo %s > /etc/ppp/br0_info",Br0NetSectAddr);
	system(br0info);
#endif	
	apmib_get( MIB_DMZ_ENABLED, (void *)&intVal);
	if(intVal ==1){
		apmib_get( MIB_DMZ_HOST,  (void *)IpAddrBuf);
		strIp = inet_ntoa(*((struct in_addr *)IpAddrBuf));
		if(strcmp(strIp, "0.0.0.0")){
			strcpy(IpAddr, strIp);
			RunSystemCmd(NULL_FILE, Iptables, ADD, PREROUTING, _table , nat_table, _protocol, ALL, _dest, WanIpAddr, jump, DNAT, "--to", IpAddr, NULL_STR);
			RunSystemCmd(NULL_FILE, Iptables, DEL, INPUT, _protocol, _tcp, dport, "80:80", in, pInterface, _dest, WanIpAddr, jump, DROP, NULL_STR);
			RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, in, pInterface, _dest, IpAddr, _protocol, ALL, jump, ACCEPT, NULL_STR);
			//add for DMZ
			RunSystemCmd(NULL_FILE, Iptables, ADD, POSTROUTING, _table, nat_table,_src,Br0NetSectAddr,_dest, IpAddr, jump, "SNAT","--to", WanIpAddr, NULL_STR);
		}
	}

	intVal = 0;
	apmib_get( MIB_PING_WAN_ACCESS_ENABLED, (void *)&intVal);
	if(intVal==1){
		RunSystemCmd(NULL_FILE, Iptables, ADD, INPUT, _protocol, _icmp, icmp_type, echo_request,  in, pInterface, _dest, WanIpAddr, jump, ACCEPT, NULL_STR);
	}else{
		RunSystemCmd(NULL_FILE, Iptables, ADD, INPUT, _protocol, _icmp, icmp_type, echo_request,  in, pInterface, _dest, WanIpAddr, jump, DROP, NULL_STR);
	}

	intVal = 0;
	apmib_get( MIB_IGMP_PROXY_DISABLED, (void *)&intVal);
	if(intVal==0){
		RunSystemCmd(NULL_FILE, Iptables, ADD, INPUT, _protocol, "2", in, pInterface, jump, ACCEPT, NULL_STR);
		RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, _protocol, _udp, match, _udp, in, pInterface, "--destination" , "224.0.0.0/4", jump, ACCEPT, NULL_STR);
		apmib_get( MIB_WAN_DHCP, (void *)&intVal);
		//if wan is pptp(4) or l2tp(6), add this rule to permit multicast transter from wan to lan
		if(intVal==4 || intVal==6){
#ifdef MULTI_PPPOE	
	if(isNeedSetOnce){
#endif					
			RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, _protocol, _udp, match, _udp, in, Interface_wanPhy, "--destination" , "224.0.0.0/4", jump, ACCEPT, NULL_STR);
#ifdef MULTI_PPPOE	
		}
#endif	
		}
	}

//modify	
	RunSystemCmd(NULL_FILE, Iptables, ADD, INPUT, in, "br0", jump, ACCEPT, NULL_STR);
	RunSystemCmd(NULL_FILE, Iptables, ADD, INPUT, in, "lo", jump, ACCEPT, NULL_STR);
//	RunSystemCmd(NULL_FILE, Iptables, ADD, INPUT, in, NOT, pInterface, jump, ACCEPT, NULL_STR);

#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
 	apmib_get( MIB_VLAN_WAN_BRIDGE_TAG, (void *)&intVal);
	if(intVal!=0)
	{
    	RunSystemCmd(NULL_FILE, Iptables, ADD, INPUT, in, "br1", jump, ACCEPT, NULL_STR);
	}
#endif

	RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, _protocol, "50", in, pInterface, out, "br0", jump, ACCEPT, NULL_STR);
	RunSystemCmd(NULL_FILE, Iptables, ADD, FORWARD, in, pInterface, match, mstate, state, RELATED_ESTABLISHED, jump, ACCEPT, NULL_STR);
	/*when layered driver enable, permit all icmp packet but icmp request...*/
	//RunSystemCmd(NULL_FILE, Iptables, ADD, INPUT, _protocol, "2", in, pInterface, jump, ACCEPT, NULL_STR);



	if(dyn_rt_support ==1){
		apmib_get(MIB_NAT_ENABLED, (void *)&natEnabled);
#if defined(CONFIG_ROUTE)
		apmib_get(MIB_RIP_ENABLED, (void *)&intVal);
		apmib_get(MIB_RIP_WAN_RX, (void *)&intVal1);
#endif
		if(natEnabled==1 && intVal==1){
			if(intVal1==1){
				RunSystemCmd(NULL_FILE, Iptables, ADD, INPUT, in, pInterface, _protocol, _udp, dport, "520", jump, ACCEPT, NULL_STR);
			}
		}
	}
	

	RunSystemCmd(NULL_FILE, Iptables, _table, nat_table, ADD, POSTROUTING, out, pInterface, jump, MASQUERADE, NULL_STR);
/*
	RunSystemCmd("/proc/sys/net/ipv4/ip_conntrack_max", "echo", "1280", NULL_STR);
	RunSystemCmd("/proc/sys/net/ipv4/netfilter/ip_conntrack_tcp_timeout_established", "echo", "600", NULL_STR);
	RunSystemCmd("/proc/sys/net/ipv4/netfilter/ip_conntrack_udp_timeout", "echo", "60", NULL_STR);
	RunSystemCmd("/proc/sys/net/ipv4/netfilter/ip_conntrack_tcp_timeout_time_wait", "echo", "5", NULL_STR);
	RunSystemCmd("/proc/sys/net/ipv4/netfilter/ip_conntrack_tcp_timeout_close", "echo", "5", NULL_STR);
*/

//hyking:packet from wan is NOT allowed
#if 0 //defined(CONFIG_RTL_LAYERED_DRIVER_ACL)
	RunSystemCmd(NULL_FILE, "iptables", ADD, INPUT, jump, ACCEPT, NULL_STR);
#else
//	RunSystemCmd(NULL_FILE, "iptables", ADD, INPUT, "!", in, pInterface, jump, ACCEPT, NULL_STR);
#endif

	//RunSystemCmd("/proc/sys/net/ipv4/conf/eth1/arp_ignore", "echo", "1", NULL_STR);
#ifdef MULTI_PPPOE	
	if(argc >=4 && argv[3])
		set_QoS(opmode, wan_dhcp, wlaniface,argv[3]);
	else
		set_QoS(opmode, wan_dhcp, wlaniface,"ppp0");
#else
	set_QoS(opmode, wan_dhcp, wlaniface);
			
#endif		

#ifdef CONFIG_RTK_VOIP
	set_voip_parameter(pInterface);
#endif

#ifdef MULTI_PPPOE
	setMulPppoeRules(argc,argv);	
#endif
#ifdef CONFIG_RTL_HW_NAPT
		update_hwnat_setting();
#endif

	#if defined(CONFIG_APP_TR069)
	// tr069 connection request	
		start_tr069(pInterface, WanIpAddr);
	#endif //#if defined(CONFIG_APP_TR069)
	system("echo 1 > /etc/ppp/firewall_lock");
	
EXIT_setFirewallIptablesRules:
	RunSystemCmd("/proc/sys/net/ipv4/ip_forward", "echo", "1", NULL_STR);
	return 0;

}


#ifdef CONFIG_RTL_HW_NAPT
int update_hwnat_setting()
{
	int opmode=0;
	//fprintf(stderr,"--------------%s:%d -------------------------\n",__FUNCTION__,__LINE__);
	if(!apmib_get(MIB_OP_MODE, (void *)&opmode))
		goto error;
//for subMask
	{
		int ivalue=0;
		if(!apmib_get(MIB_SUBNET_MASK,(void*)&ivalue))
			goto error;
		if((ivalue&HW_NAT_LIMIT_NETMASK)!=HW_NAT_LIMIT_NETMASK)
		{
				RunSystemCmd("/proc/hw_nat", "echo", "-1", NULL_STR);
				return 0;
		}		
	}
#ifdef CONFIG_RTK_VOIP
	//for voip
	{
		voipCfgParam_t  voipCfgParam ={0};
		if(!apmib_get(MIB_VOIP_CFG, (void*)&voipCfgParam))
			goto error;
		
		if(!voipCfgParam.hwnat_enable)
		{
			RunSystemCmd(HW_NAT_FILE, "echo", "0", NULL_STR);
			return 0;
		}
	}
#endif

//for url filter
	{
		int urlfilter_enable=0,urlfilter_num=0;
		if(!apmib_get(MIB_URLFILTER_ENABLED,  (void *)&urlfilter_enable))
			goto error;
		if(!apmib_get(MIB_URLFILTER_TBL_NUM,  (void *)&urlfilter_num))
			goto error;
		if(opmode == GATEWAY_MODE && urlfilter_enable!=0 && urlfilter_num>0)
		{
			RunSystemCmd("/proc/hw_nat", "echo", "0", NULL_STR);
			return 0;
		}
	}
//for dos
	{
		int dos_enabled=0;
		if(!apmib_get(MIB_DOS_ENABLED, (void *)&dos_enabled))
			goto error;
		if(dos_enabled>0)
		{
			RunSystemCmd("/proc/hw_nat", "echo", "0", NULL_STR);
			return 0;
		}
	}

	//printf("%s:%d\n",__FUNCTION__,__LINE__);
	RunSystemCmd(HW_NAT_FILE, "echo", "1", NULL_STR);
	return 0;
error:
	printf("update hardware nat error!\n");
	RunSystemCmd(HW_NAT_FILE, "echo", "-1", NULL_STR);
	return -1;
}
#endif







