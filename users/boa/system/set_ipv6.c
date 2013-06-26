/*
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
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#include <net/if.h>
#include <stddef.h>		/* offsetof */
#include <net/if_arp.h>
#include <linux/if_ether.h>
#include "apmib.h"
#include "mibtbl.h"
#include "sysconf.h"
#include "sys_utility.h"

#define DHCP6S_PID_FILE "/var/run/dhcp6s.pid"
#define DHCP6S_CONF_FILE "/var/dhcp6s.conf"
#define DNSV6_PID_FILE "/var/run/dnsmasq.pid"
#define DNRD_PID_FILE "/var/run/dnrd.pid"
#define DNSV6_CONF_FILE "/var/dnsmasq.conf"
#define RESOLV_CONF_FILE "/etc/resolv.conf"
#define RADVD_CONF_FILE "/var/radvd.conf"
#define RADVD_PID_FILE "/var/run/radvd.pid"
#define ECMH_PID_FILE	"/var/run/ecmh.pid"

void set_dhcp6s()
{
	dhcp6sCfgParam_t dhcp6sCfgParam;
	char tmpStr[256];
	int fh;
	int pid=-1;
	
	if ( !apmib_get(MIB_IPV6_DHCPV6S_PARAM,(void *)&dhcp6sCfgParam)){
		printf("get MIB_IPV6_DHCPV6S_PARAM failed\n");
		return;  
	}
	
	if(isFileExist(DHCP6S_CONF_FILE) == 0) {
		/*create config file*/
		fh = open(DHCP6S_CONF_FILE, O_RDWR|O_CREAT|O_TRUNC, S_IRWXO|S_IRWXG);	
		if (fh < 0) {
			fprintf(stderr, "Create %s file error!\n", DHCP6S_CONF_FILE);
			return;
		}
		printf("create dhcp6s.conf\n");
		
		sprintf(tmpStr, "option domain-name-servers %s;\n", dhcp6sCfgParam.DNSaddr6);
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "interface %s {\n", dhcp6sCfgParam.interfaceNameds);
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "  address-pool pool1 3600;\n");
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "};\n");
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "pool pool1 {\n");
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "  range %s to %s ;\n", dhcp6sCfgParam.addr6PoolS, dhcp6sCfgParam.addr6PoolE);
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "};\n");
		write(fh, tmpStr, strlen(tmpStr));

		close(fh);
	}

	/*start daemon*/
	if(isFileExist(DHCP6S_PID_FILE)) {
		pid=getPid_fromFile(DHCP6S_PID_FILE);
		if(dhcp6sCfgParam.enabled == 1){
			sprintf(tmpStr, "%d", pid);
			RunSystemCmd(NULL_FILE, "kill", "-9", tmpStr, NULL_STR);
			unlink(DHCP6S_PID_FILE);
			RunSystemCmd(NULL_FILE, "/bin/dhcp6s", dhcp6sCfgParam.interfaceNameds, NULL_STR);
		}
		else 
			RunSystemCmd(NULL_FILE, "kill", "-9", tmpStr, NULL_STR);
			
	}else {
		if(dhcp6sCfgParam.enabled == 1)
			RunSystemCmd(NULL_FILE, "/bin/dhcp6s", dhcp6sCfgParam.interfaceNameds, NULL_STR);
	}
		
	return;
}
void set_dnsv6()
{
	dnsv6CfgParam_t dnsCfgParam;
	int pid = -1;
	int fh;
	char tmpStr[128];
	char tmpBuff[32];
	char tmpChar;
	char addr[256];
	char cmdBuffer[256];
	
	if ( !apmib_get(MIB_IPV6_DNSV6_PARAM,(void *)&dnsCfgParam)){
		printf("get MIB_IPV6_DNSV6_PARAM failed\n");
		return;  
	}

	if(!isFileExist(DNSV6_CONF_FILE)){
		/*create config file*/
		fh = open(DNSV6_CONF_FILE, O_RDWR|O_CREAT|O_TRUNC, S_IRWXO|S_IRWXG);	
		if (fh < 0) {
			fprintf(stderr, "Create %s file error!\n", DNSV6_CONF_FILE);
			return;
		}

		printf("create dnsmasq.conf\n");

		apmib_get(MIB_ELAN_MAC_ADDR,  (void *)tmpBuff);
		if(!memcmp(tmpBuff, "\x00\x00\x00\x00\x00\x00", 6))
			apmib_get(MIB_HW_NIC0_ADDR,  (void *)tmpBuff);
		sprintf(cmdBuffer, "%02x%02x%02x%02x%02x%02x", (unsigned char)tmpBuff[0], (unsigned char)tmpBuff[1], 
			(unsigned char)tmpBuff[2], (unsigned char)tmpBuff[3], (unsigned char)tmpBuff[4], (unsigned char)tmpBuff[5]);

		tmpChar=cmdBuffer[1];
		
		switch(tmpChar) {
			case '0':
			case '1':
			case '4':
			case '5':
			case '8':
			case '9':
			case 'c':
			case 'd':
				tmpChar = (char)((int)tmpChar+2);
				break;
			default:
				break;
		}
		sprintf(addr, "Fe80::%c%c%c%c:%c%cFF:FE%c%c:%c%c%c%c", 
			cmdBuffer[0], tmpChar, cmdBuffer[2], cmdBuffer[3],
			cmdBuffer[4], cmdBuffer[5],
			cmdBuffer[6], cmdBuffer[7],
			cmdBuffer[8],cmdBuffer[9],cmdBuffer[10],cmdBuffer[11]
			);

		
		sprintf(tmpStr, "domain-needed\n");
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "bogus-priv\n");
		write(fh, tmpStr, strlen(tmpStr));
		if(isFileExist(RESOLV_CONF_FILE)){			
			sprintf(tmpStr, "resolv-file=%s\n", RESOLV_CONF_FILE);
			write(fh, tmpStr, strlen(tmpStr));
		}
		sprintf(tmpStr, "#strict-order\n");
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "#no-resolv\n");
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "#no-poll\n");
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "address=/%s/%s\n",dnsCfgParam.routerName,addr);
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "#listen-address=\n");
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "#bind-interfaces\n");
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "#no-hosts\n");
		write(fh, tmpStr, strlen(tmpStr));
		
		close(fh);	
	}

	if(isFileExist(DNSV6_PID_FILE)) {
		pid=getPid_fromFile(DNSV6_PID_FILE);
		sprintf(tmpStr, "%d", pid);
		if(dnsCfgParam.enabled == 1) {
			RunSystemCmd(NULL_FILE, "kill", "-9", tmpStr, NULL_STR);
			unlink(DNSV6_PID_FILE);
			if(isFileExist(DNRD_PID_FILE)){
				pid=getPid_fromFile(DNRD_PID_FILE);
				sprintf(tmpStr, "%d", pid);
				RunSystemCmd(NULL_FILE, "kill", "-9", tmpStr, NULL_STR);
				unlink(DNRD_PID_FILE);				
			}
			sprintf(tmpStr, "dnsmasq -C /var/dnsmasq.conf");
			RunSystemCmd(NULL_FILE, tmpStr, NULL_STR);
		}else {
			RunSystemCmd(NULL_FILE, "kill", "-9", tmpStr, NULL_STR);
		}
	} else{
		if(dnsCfgParam.enabled == 1) {
			if(isFileExist(DNRD_PID_FILE)) {
				pid=getPid_fromFile(DNRD_PID_FILE);
				sprintf(tmpStr, "%d", pid);
				RunSystemCmd(NULL_FILE, "kill", "-9", tmpStr, NULL_STR);
				unlink(DNRD_PID_FILE);			
			}
			sprintf(tmpStr, "dnsmasq -C /var/dnsmasq.conf");
			RunSystemCmd(NULL_FILE, tmpStr, NULL_STR);
		}
	}

	
	return;
}
void set_radvd()
{
	radvdCfgParam_t radvdCfgParam;
	int pid=-1;
	int fh;
	char tmpStr[256];
	char tmpBuf[256];
	unsigned short tmpNum[8];

	if ( !apmib_get(MIB_IPV6_RADVD_PARAM,(void *)&radvdCfgParam)){
		printf("get MIB_IPV6_RADVD_PARAM failed\n");
		return;  
	}

	if(!isFileExist(RADVD_CONF_FILE)){
		/*create config file*/
		fh = open(RADVD_CONF_FILE, O_RDWR|O_CREAT|O_TRUNC, S_IRWXO|S_IRWXG);	
		if (fh < 0) {
			fprintf(stderr, "Create %s file error!\n", RADVD_CONF_FILE);
			return;
		}
		printf("create radvd.conf\n");
		sprintf(tmpStr, "interface %s\n", radvdCfgParam.interface.Name);
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "{\n");
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "AdvSendAdvert on;\n");
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "MaxRtrAdvInterval %d;\n", radvdCfgParam.interface.MaxRtrAdvInterval);
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "MinRtrAdvInterval %d;\n", radvdCfgParam.interface.MinRtrAdvInterval);
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "MinDelayBetweenRAs %d;\n", radvdCfgParam.interface.MinDelayBetweenRAs);
		write(fh, tmpStr, strlen(tmpStr));
		if(radvdCfgParam.interface.AdvManagedFlag > 0) {
			sprintf(tmpStr, "AdvManagedFlag on;\n");
			write(fh, tmpStr, strlen(tmpStr));			
		}
		if(radvdCfgParam.interface.AdvOtherConfigFlag > 0){
			sprintf(tmpStr, "AdvOtherConfigFlag on;\n");
			write(fh, tmpStr, strlen(tmpStr));	
		}
		sprintf(tmpStr, "AdvLinkMTU %d;\n", radvdCfgParam.interface.AdvLinkMTU);
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "AdvReachableTime %u;\n", radvdCfgParam.interface.AdvReachableTime);
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "AdvRetransTimer %u;\n", radvdCfgParam.interface.AdvRetransTimer);
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "AdvCurHopLimit %d;\n", radvdCfgParam.interface.AdvCurHopLimit);
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "AdvDefaultLifetime %d;\n", radvdCfgParam.interface.AdvDefaultLifetime);
		write(fh, tmpStr, strlen(tmpStr));
		sprintf(tmpStr, "AdvDefaultPreference %s;\n", radvdCfgParam.interface.AdvDefaultPreference);
		write(fh, tmpStr, strlen(tmpStr));
		if(radvdCfgParam.interface.AdvSourceLLAddress > 0) {
			sprintf(tmpStr, "AdvSourceLLAddress on;\n");
			write(fh, tmpStr, strlen(tmpStr));			
		}		
		if(radvdCfgParam.interface.UnicastOnly > 0){
			sprintf(tmpStr, "UnicastOnly on;\n");
			write(fh, tmpStr, strlen(tmpStr));	
		}
		

		/*prefix 1*/
		if(radvdCfgParam.interface.prefix[0].enabled > 0){
			memcpy(tmpNum,radvdCfgParam.interface.prefix[0].Prefix, sizeof(radvdCfgParam.interface.prefix[0].Prefix));
			sprintf(tmpBuf, "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x", tmpNum[0], tmpNum[1], 
				tmpNum[2], tmpNum[3], tmpNum[4], tmpNum[5],tmpNum[6],tmpNum[7]);
			strcat(tmpBuf, "\0");
			sprintf(tmpStr, "prefix %s/%d\n", tmpBuf, radvdCfgParam.interface.prefix[0].PrefixLen);			
			write(fh, tmpStr, strlen(tmpStr));
			sprintf(tmpStr, "{\n");
			write(fh, tmpStr, strlen(tmpStr));
			if(radvdCfgParam.interface.prefix[0].AdvOnLinkFlag > 0){
				sprintf(tmpStr, "AdvOnLink on;\n");
				write(fh, tmpStr, strlen(tmpStr));					
			}
			if(radvdCfgParam.interface.prefix[0].AdvAutonomousFlag > 0){
				sprintf(tmpStr, "AdvAutonomous on;\n");
				write(fh, tmpStr, strlen(tmpStr));					
			}
			sprintf(tmpStr, "AdvValidLifetime %u;\n", radvdCfgParam.interface.prefix[0].AdvValidLifetime);
			write(fh, tmpStr, strlen(tmpStr));					
			sprintf(tmpStr, "AdvPreferredLifetime %u;\n", radvdCfgParam.interface.prefix[0].AdvPreferredLifetime);
			write(fh, tmpStr, strlen(tmpStr));	
			if(radvdCfgParam.interface.prefix[0].AdvRouterAddr > 0){
				sprintf(tmpStr, "AdvRouterAddr on;\n");
				write(fh, tmpStr, strlen(tmpStr));						
			}
			if(!memcmp(radvdCfgParam.interface.prefix[0].if6to4, "\x00\x00\x00\x00\x00\x00", 6)){
				sprintf(tmpStr, "Base6to4Interface %s\n;", radvdCfgParam.interface.prefix[0].if6to4);
				write(fh, tmpStr, strlen(tmpStr));						
			}
			sprintf(tmpStr, "};\n");
			write(fh, tmpStr, strlen(tmpStr));						
		}

		/*prefix 2*/
		if(radvdCfgParam.interface.prefix[1].enabled > 0){
			memcpy(tmpNum,radvdCfgParam.interface.prefix[1].Prefix, sizeof(radvdCfgParam.interface.prefix[1].Prefix));
			sprintf(tmpBuf, "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x", tmpNum[0], tmpNum[1], 
				tmpNum[2], tmpNum[3], tmpNum[4], tmpNum[5],tmpNum[6],tmpNum[7]);
			strcat(tmpBuf, "\0");
			sprintf(tmpStr, "prefix %s/%d\n", tmpBuf, radvdCfgParam.interface.prefix[1].PrefixLen);
			write(fh, tmpStr, strlen(tmpStr));
			sprintf(tmpStr, "{\n");
			write(fh, tmpStr, strlen(tmpStr));
			if(radvdCfgParam.interface.prefix[1].AdvOnLinkFlag > 0){
				sprintf(tmpStr, "AdvOnLink on;\n");
				write(fh, tmpStr, strlen(tmpStr));					
			}
			if(radvdCfgParam.interface.prefix[1].AdvAutonomousFlag > 0){
				sprintf(tmpStr, "AdvAutonomous on;\n");
				write(fh, tmpStr, strlen(tmpStr));					
			}
			sprintf(tmpStr, "AdvValidLifetime %u;\n", radvdCfgParam.interface.prefix[1].AdvValidLifetime);
			write(fh, tmpStr, strlen(tmpStr));					
			sprintf(tmpStr, "AdvPreferredLifetime %u;\n", radvdCfgParam.interface.prefix[1].AdvPreferredLifetime);
			write(fh, tmpStr, strlen(tmpStr));	
			if(radvdCfgParam.interface.prefix[1].AdvRouterAddr > 0){
				sprintf(tmpStr, "AdvRouterAddr on;\n");
				write(fh, tmpStr, strlen(tmpStr));						
			}
			if(!memcmp(radvdCfgParam.interface.prefix[0].if6to4, "\x00\x00\x00\x00\x00\x00", 6)){
				sprintf(tmpStr, "Base6to4Interface %s;\n", radvdCfgParam.interface.prefix[1].if6to4);
				write(fh, tmpStr, strlen(tmpStr));						
			}
			sprintf(tmpStr, "};\n");
			write(fh, tmpStr, strlen(tmpStr));						
		}

		sprintf(tmpStr, "};\n");
		write(fh, tmpStr, strlen(tmpStr));

		close(fh);		
	}
	
	if(isFileExist(RADVD_PID_FILE)){
		if(radvdCfgParam.enabled == 1) {
			system("killall radvd 2> /dev/null");			
			system("rm -f /var/run/radvd.pid 2> /dev/null");		
			unlink(DNRD_PID_FILE);						
			system("echo 1 > /proc/sys/net/ipv6/conf/all/forwarding");
			system("radvd -C /var/radvd.conf");
				
		} else {	
			system("killall radvd 2> /dev/null");		
			system("rm -f /var/run/radvd.pid 2> /dev/null");			
		}
	} else{
		if(radvdCfgParam.enabled == 1) {
			system("echo 1 > /proc/sys/net/ipv6/conf/all/forwarding");
			system("radvd -C /var/radvd.conf");		
		}		
	}
	
	return;
}

void set_ecmh()
{	
		
	if(isFileExist(ECMH_PID_FILE)){
		system("killall ecmh 2> /dev/null");		
	}
	
	system("ecmh");		
	
	return;
}

void set_basicv6() 
{
	addrIPv6CfgParam_t addrIPv6CfgParam;
	char tmpStr[256];
	
	if ( !apmib_get(MIB_IPV6_ADDR_PARAM,(void *)&addrIPv6CfgParam)){
		printf("get MIB_IPV6_ADDR_PARAM failed\n");
		return;        
	}
	if(addrIPv6CfgParam.enabled == 1) {
		/*
		/bin/ifconfig br0 $ADDR1/$PREFIX1
        /bin/ifconfig eth1 $ADDR2/$PREFIX2
        */
		sprintf(tmpStr,"/bin/ifconfig br0 %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x/%d",
			addrIPv6CfgParam.addrIPv6[0][0],addrIPv6CfgParam.addrIPv6[0][1],addrIPv6CfgParam.addrIPv6[0][2],addrIPv6CfgParam.addrIPv6[0][3],
			addrIPv6CfgParam.addrIPv6[0][4],addrIPv6CfgParam.addrIPv6[0][5],addrIPv6CfgParam.addrIPv6[0][6],addrIPv6CfgParam.addrIPv6[0][7],
			addrIPv6CfgParam.prefix_len[0]);
		//RunSystemCmd(NULL_FILE, tmpStr, NULL_STR);
		system(tmpStr);
		//printf("the cmd for ipv6 is %s\n", tmpStr);
		
		sprintf(tmpStr,"/bin/ifconfig eth1 %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x/%d",
			addrIPv6CfgParam.addrIPv6[1][0],addrIPv6CfgParam.addrIPv6[1][1],addrIPv6CfgParam.addrIPv6[1][2],addrIPv6CfgParam.addrIPv6[1][3],
			addrIPv6CfgParam.addrIPv6[1][4],addrIPv6CfgParam.addrIPv6[1][5],addrIPv6CfgParam.addrIPv6[1][6],addrIPv6CfgParam.addrIPv6[1][7],
			addrIPv6CfgParam.prefix_len[1]);
		//RunSystemCmd(NULL_FILE, tmpStr, NULL_STR);
		system(tmpStr);
		//printf("the cmd for ipv6 is %s\n", tmpStr);
	}



	
}

void set_ipv6()
{

#if defined(CONFIG_IPV6)
	printf("Start setting IPv6[IPv6]\n");
	RunSystemCmd("/proc/sys/net/ipv6/conf/all/forwarding", "echo", "1", NULL_STR);
	set_basicv6();

	printf("Start dhcpv6[IPv6]\n");
	set_dhcp6s();

	printf("Start dnsv6[IPv6]\n");
	set_dnsv6();

	printf("Start radvd[IPv6]\n");
	set_radvd();

	printf("Start ECMH[IPv6]\n");
	set_ecmh();
#endif

	return;
}





