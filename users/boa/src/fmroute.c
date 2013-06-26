/*
 *      Web server handler routines for management (password, save config, f/w update)
 *
 *      Authors: sc_yang <sc_yang@realtek.com.tw>
 *
 *      $Id
 *
 */
#ifdef ROUTE_SUPPORT
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
int CheckkernelRouteList(STATICROUTE_Tp checkentry)
{
	char buff[256];
	char iface[30];
	int nl = 0;
	struct in_addr dest;
	struct in_addr gw;
	struct in_addr mask;
	int flgs, ref, use, metric, mtu, win, ir;
	char flags[64];
	unsigned long int d, g, m;
	STATICROUTE_T entry;
	DHCP_T dhcp;
	char *interface_WAN=NULL;
	char *interface_LAN=NULL;
	int isFound=0;
	int opmode=0;
	memset(&entry, '\0', sizeof(entry));
	apmib_get( MIB_WAN_DHCP, (void *)&dhcp);
	apmib_get(MIB_OP_MODE, (void *)&opmode);
	if ( dhcp == PPPOE || dhcp == PPTP || dhcp == L2TP) {
			interface_WAN = PPPOE_IF;
	}else
			interface_WAN = WAN_IF;
	if(opmode==2)
		interface_WAN = ("wlan0");
			
	interface_LAN = BRIDGE_IF;
	
	FILE *fp = fopen("/proc/net/route", "r");

	while (fgets(buff, sizeof(buff), fp) != NULL) {
		if (nl) {
			int ifl = 0;
			while (buff[ifl] != ' ' && buff[ifl] != '\t' && buff[ifl] != '\0')
				ifl++;
			strncpy(iface, buff, ifl);
			iface[ifl]='\0';
			if(!strcmp(iface, interface_WAN)){
				entry.interface=1;
			}
			if(!strcmp(iface, interface_LAN)){
				entry.interface=0;	
			}	
				
			buff[ifl] = 0;	/* interface */
			if (sscanf(buff + ifl + 1, "%lx%lx%X%d%d%d%lx%d%d%d",
					   &d, &g, &flgs, &ref, &use, &metric, &m, &mtu, &win,
					   &ir) != 10) {
				printf("Unsuported kernel route format\n");
			}
			ifl = 0;	/* parse flags */
			if (flgs & RTF_UP) {
				if (flgs & RTF_REJECT)
					flags[ifl++] = '!';
				else
					flags[ifl++] = 'U';
				if (flgs & RTF_GATEWAY)
					flags[ifl++] = 'G';
				if (flgs & RTF_HOST)
					flags[ifl++] = 'H';
				if (flgs & RTF_REINSTATE)
					flags[ifl++] = 'R';
				if (flgs & RTF_DYNAMIC)
					flags[ifl++] = 'D';
				if (flgs & RTF_MODIFIED)
					flags[ifl++] = 'M';
				flags[ifl] = 0;
				dest.s_addr = d;
				memcpy(&(entry.dstAddr), &(dest.s_addr), 4); 
				gw.s_addr = g;
				memcpy(&(entry.gateway), &(gw.s_addr), 4); 
				mask.s_addr = m;
			}
			
				if ( dhcp == PPPOE || dhcp == PPTP || dhcp == L2TP){
					if(checkentry->interface==1){
						if(!memcmp(checkentry->dstAddr, &entry.dstAddr, 4) && entry.interface == checkentry->interface ){
							isFound=1;		
							break;
						}
					}else{
						if(!memcmp(checkentry->dstAddr, &entry.dstAddr, 4) && !memcmp(checkentry->gateway, &entry.gateway, 4) && entry.interface == checkentry->interface ){
							isFound=1;		
							break;
						}
					}
				}else{//for lan interface
					if(!memcmp(checkentry->dstAddr, &entry.dstAddr, 4) && !memcmp(checkentry->gateway, &entry.gateway, 4) && entry.interface == checkentry->interface ){
						isFound=1;		
						break;
					}
				}
		}
		nl++;
		
	}
	if(fp)
		fclose(fp);
	if(isFound ==1)
		return 1;
	else
		return 0;
}


void formRoute(request *wp, char *path, char *query)
{
	char *submitUrl, *strAddRoute, *strDelRoute, *strVal, *strDelAllRoute;
	char *strIp, *strMask, *strGateway, *strRip, *tmpStr, *strMetric;
	char *strRip_Mode;
	char *strInterface;
	char *iface=NULL;
	char tmpBuf[100];
	char str1[30], str2[30], str3[30];
	int entryNum, intVal, i;
	STATICROUTE_T entry, checkentry;
	unsigned int * a1, *a2, *b1, *b2;
	int ok_msg =0 ;
#ifndef NO_ACTION
	int pid;
#endif
	int opmode=-1;
	int nat_mode=0;
	int get_wanip=0;
	DHCP_T dhcp;
	struct in_addr intaddr;
	RIP_OPMODE_T ripmode_tx;
	RIP_OPMODE_T ripmode_rx;
	unsigned long v1, v2, v3;
	int check_ip1=0;
	int check_ip2=0;
	unsigned long ipAddr, curIpAddr, curSubnet;
	int isExist=0;
	int enable_igmpproxy=0;
	apmib_get( MIB_WAN_DHCP, (void *)&dhcp);
	apmib_get(MIB_OP_MODE, (void *)&opmode);
	strAddRoute = req_get_cstream_var(wp, ("addRoute"), "");
	strDelRoute = req_get_cstream_var(wp, ("deleteSelRoute"), "");
	strDelAllRoute = req_get_cstream_var(wp, ("deleteAllRoute"), "");
	strRip	= req_get_cstream_var(wp, ("ripSetup"), "");
	memset(&entry, '\0', sizeof(entry));
	if(strRip[0]) {
		
		strVal = req_get_cstream_var(wp, ("enabled"), "");
		if ( !strcmp(strVal, "ON"))
                        intVal = 1;
                else
                        intVal = 0;	
		if ( apmib_set( MIB_RIP_ENABLED, (void *)&intVal) == 0) {
			strcpy(tmpBuf, ("Set enabled flag error!"));
			goto setErr_route;
		}
		
		if(intVal==0){
			nat_mode=1;
			if ( apmib_set(MIB_NAT_ENABLED, (void *)&nat_mode) == 0) {
				strcpy(tmpBuf, ("\"Set NAT enabled error!\""));
				goto setErr_route;
			}
			enable_igmpproxy = 0;	
			if ( apmib_set(MIB_IGMP_PROXY_DISABLED, (void *)&enable_igmpproxy) == 0) {
				strcpy(tmpBuf, ("\"Set IGMP PROXY disabled error!\""));
				goto setErr_route;
			}
			goto setOk_route;
		}
		
		tmpStr = req_get_cstream_var(wp, ("nat_enabled"), "");  
		if(tmpStr[0]){
			if(tmpStr[0] == '0')
				nat_mode=1;
			else
				nat_mode=0;
				
			if ( apmib_set(MIB_NAT_ENABLED, (void *)&nat_mode) == 0) {
				strcpy(tmpBuf, ("\"Set NAT enabled error!\""));
				goto setErr_route;
			}
			if(nat_mode==1)
				enable_igmpproxy = 0;
			else
				enable_igmpproxy = 1;	
			if ( apmib_set(MIB_IGMP_PROXY_DISABLED, (void *)&enable_igmpproxy) == 0) {
				strcpy(tmpBuf, ("\"Set IGMP PROXY disabled error!\""));
				goto setErr_route;
			}
			
			
		}	
		
		strRip_Mode= req_get_cstream_var(wp, ("rip_tx"), "");
		if(strRip_Mode[0]) {
			ripmode_tx = strRip_Mode[0] - '0' ;
		}else
			ripmode_tx = DISABLE_MODE;	
		if (apmib_set( MIB_RIP_LAN_TX, (void *)&ripmode_tx) == 0) {
				strcpy(tmpBuf, ("\"Set RIP LAN Tx error!\""));
				goto setErr_route;
		}
		
		if(nat_mode ==0){
			if (apmib_set( MIB_RIP_WAN_TX, (void *)&ripmode_tx) == 0) {
					strcpy(tmpBuf, ("\"Set RIP WAN Tx error!\""));
				goto setErr_route;
			}
		}

		strRip_Mode= req_get_cstream_var(wp, ("rip_rx"), "");
		if(strRip_Mode[0]) {
			ripmode_rx = strRip_Mode[0] - '0' ;
		}else
			ripmode_rx = DISABLE_MODE;
		
		if (apmib_set( MIB_RIP_LAN_RX, (void *)&ripmode_rx) == 0) {
			strcpy(tmpBuf, ("\"Set RIP LAN Rx error!\""));
				goto setErr_route;
			}
		if(nat_mode ==0){
			if ( apmib_set( MIB_RIP_WAN_RX, (void *)&ripmode_rx) == 0) {
					strcpy(tmpBuf, ("\"Set RIP WAN Rx error!\""));
				goto setErr_route;
			}
		}
		ok_msg = 1;
	}
	/* Add new static route table */
	if (strAddRoute[0]) {
		strVal = req_get_cstream_var(wp, ("enabled"), "");
		if ( !strcmp(strVal, "ON"))
			intVal = 1;
		else
			intVal = 0;
		if ( apmib_set( MIB_STATICROUTE_ENABLED, (void *)&intVal) == 0) {
			strcpy(tmpBuf, ("Set enabled flag error!"));
			goto setErr_route;
		}

		strIp = req_get_cstream_var(wp, ("ipAddr"), "");
		strMask= req_get_cstream_var(wp, ("subnet"), "");
		strGateway= req_get_cstream_var(wp, ("gateway"), "");
		strInterface=req_get_cstream_var(wp, ("iface"), "");
		strMetric=req_get_cstream_var(wp, ("metric"), "");
		if (!strIp[0] && !strMask[0] && !strGateway[0])
			goto setOk_route;

		if(strInterface[0]=='0')
			entry.interface=0;
		else if( strInterface[0]=='1')
			entry.interface=1;
		if(entry.interface==1){
			if ( dhcp == PPPOE || dhcp == PPTP || dhcp == L2TP) {
				iface = PPPOE_IF;
			}else
				iface = WAN_IF;
		if(opmode==2)
				iface = ("wlan0");		
		}else
			iface=BRIDGE_IF;	
			
		if(!strcmp(iface, "ppp0"))
			get_wanip = isConnectPPP();
		else
			get_wanip = getInAddr(iface, IP_ADDR, (void *)&intaddr);	
			
		inet_aton(strIp,(struct in_addr *)&entry.dstAddr);
		inet_aton(strMask, (struct in_addr *)&entry.netmask);
		inet_aton(strGateway, (struct in_addr *)&entry.gateway);
		entry.metric=(unsigned char)atoi(strMetric);
		memcpy((void *)&v1, (void *)entry.dstAddr, 4);
		memcpy((void *)&v2, (void *)entry.netmask, 4);
		v2 = ~ntohl(v2);
		if (v2 & (v2 + 1)) {
			sprintf(tmpBuf, "\"Invalid Netmask: %s \"", strMask);
			goto setErr_route;
		}
		memcpy((void *)&v2, (void *)entry.netmask, 4);	
		if (v1 & ~v2) {
			strcpy(tmpBuf, "\"Netmask doesn't match route address!\"");
			goto setErr_route;
		} 
		if(strcmp(iface, "ppp0")){
			memcpy((void *)&ipAddr, (void *)entry.gateway, 4);
			check_ip1= getInAddr(iface, IP_ADDR, (void *)&curIpAddr);
			check_ip2= getInAddr(iface, SUBNET_MASK, (void *)&curSubnet);
			v1 = ipAddr;
			v2 = curIpAddr;
			v3 = curSubnet;
			if (v1 && check_ip1 && check_ip2) {
				if ( (v1 & v3) != (v2 & v3) ) {
						if(entry.interface==1){
							strcpy(tmpBuf, ("Invalid Gateway address! It should be set within wan subnet."));
						}else if(entry.interface ==2)
							strcpy(tmpBuf, ("Invalid Gateway address! It should be set within wan physical subnet."));
						 else
							strcpy(tmpBuf, ("Invalid Gateway address! It should be set within lan subnet."));
						goto setErr_route;
					}
				}
		}
		
		if ( !apmib_get(MIB_STATICROUTE_TBL_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto setErr_route;
		}

		if ( (entryNum + 1) > MAX_ROUTE_NUM) {
			strcpy(tmpBuf, ("Cannot add new entry because table is full!"));
			goto setErr_route;
		}
		a1 = (unsigned int *) entry.dstAddr ;
		a2 = (unsigned int *) entry.netmask;
		for(i=1; i <= entryNum ; i++) {
			*((char *)&checkentry) = (char)i;
			if ( !apmib_get(MIB_STATICROUTE_TBL, (void *)&checkentry)){
				strcpy(tmpBuf, ("get entry error!"));
				goto setErr_route;
			}
			b1 = (unsigned int *) checkentry.dstAddr ;
			b2 = (unsigned int *) checkentry.netmask;
			if((*a1 & *a2) == (*b1&*b2)){
				sprintf(tmpBuf, ("Duplicate with entry %d!"),i);
				goto setErr_route;
			}
		}
#ifndef REBOOT_CHECK
		sprintf(tmpBuf, "route add -net %s netmask %s gw %s dev %s metric %d", strIp, strMask, strGateway, iface, entry.metric);
		if(system(tmpBuf) != 0){
			strcpy(tmpBuf, "Set Route error\n");
			goto setErr_route ;
		}
#endif
		// set to MIB. try to delete it first to avoid duplicate case
		apmib_set(MIB_STATICROUTE_DEL, (void *)&entry);
		if ( apmib_set(MIB_STATICROUTE_ADD, (void *)&entry) == 0) {
			strcpy(tmpBuf, ("Add table entry error!"));
			goto setErr_route;
		}
	}

	/* Delete entry */
	if (strDelRoute[0]) {
		
		
		if ( !apmib_get(MIB_STATICROUTE_TBL_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto setErr_route;
		}

		for (i=entryNum; i>0; i--) {
			snprintf(tmpBuf, 20, "select%d", i);
			strVal = req_get_cstream_var(wp, tmpBuf, "");
			if ( !strcmp(strVal, "ON") ) {
				*((char *)&entry) = (char)i;
				if ( !apmib_get(MIB_STATICROUTE_TBL, (void *)&entry)) {
					strcpy(tmpBuf, ("Get table entry error!"));
					goto setErr_route;
				}
						
				strcpy(str1, inet_ntoa(*((struct in_addr *)entry.dstAddr)));
				strcpy(str2, inet_ntoa(*((struct in_addr *)entry.netmask)));
				strcpy(str3, inet_ntoa(*((struct in_addr *)entry.gateway)));
				
						isExist = CheckkernelRouteList(&entry);
						if(isExist){	
							if(dhcp == PPPOE || dhcp ==PPTP || dhcp == L2TP){
								if(entry.interface == 1)	 
									sprintf(tmpBuf, "route del -net %s netmask %s dev %s metric %d", str1, str2, iface, entry.metric);	
							else
								sprintf(tmpBuf, "route del -net %s netmask %s gw %s metric %d", str1, str2, str3, entry.metric);
							}else
								sprintf(tmpBuf, "route del -net %s netmask %s gw %s metric %d", str1, str2, str3, entry.metric);
							
				system(tmpBuf);
						}	
				if ( !apmib_set(MIB_STATICROUTE_DEL, (void *)&entry)) {
					strcpy(tmpBuf, ("Delete table entry error!"));
					goto setErr_route;
				}
			}
		}
	}

	/* Delete all entry */
	if ( strDelAllRoute[0]) {
		if ( !apmib_get(MIB_STATICROUTE_TBL_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto setErr_route;
		}
		for(i=1; i <= entryNum ; i++) {
			*((char *)&entry) = (char)i;
			if ( !apmib_get(MIB_STATICROUTE_TBL, (void *)&entry)){
				strcpy(tmpBuf, ("get entry error!"));
				goto setErr_route;
			}
			strcpy(str1, inet_ntoa(*((struct in_addr *)entry.dstAddr)));
			strcpy(str2, inet_ntoa(*((struct in_addr *)entry.netmask)));
			strcpy(str3, inet_ntoa(*((struct in_addr *)entry.gateway)));
			sprintf(tmpBuf, "route del -net %s netmask %s gw %s metric %d", str1, str2, str3, entry.metric);
			system(tmpBuf);

		}
		if ( !apmib_set(MIB_STATICROUTE_DELALL, (void *)&entry)) {
			strcpy(tmpBuf, ("Delete all table error!"));
			goto setErr_route;
		}
	}

setOk_route:
	apmib_update_web(CURRENT_SETTING);

#ifndef NO_ACTION
	run_init_script("all");                
#endif

	submitUrl = req_get_cstream_var(wp, ("submit-url"), "");   // hidden page
		OK_MSG(submitUrl);	
		return ;

setErr_route:
	ERR_MSG(tmpBuf);
}

int staticRouteList(request *wp, int argc, char **argv)
{
	int	nBytesSent=0, entryNum, i;
	STATICROUTE_T entry;
	char	ip[30], netmask[30], gateway[30], *tmpStr;
	if ( !apmib_get(MIB_STATICROUTE_TBL_NUM, (void *)&entryNum)) {
  		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}
	nBytesSent += req_format_write(wp, "<tr>"
      	"<td align=center width=\"23%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Destination IP Address</b></font></td>\n"
      	"<td align=center width=\"23%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Netmask </b></font></td>\n"
      	"<td align=center width=\"23%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Gateway </b></font></td>\n"
      	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Metric </b></font></td>\n"
      	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Interface </b></font></td>\n"
      	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td></tr>\n");

	for (i=1; i<=entryNum; i++) {
		*((char *)&entry) = (char)i;
		if ( !apmib_get(MIB_STATICROUTE_TBL, (void *)&entry))
			return -1;

		tmpStr = inet_ntoa(*((struct in_addr *)entry.dstAddr));
		strcpy(ip, tmpStr);
		tmpStr = inet_ntoa(*((struct in_addr *)entry.netmask));
		strcpy(netmask, tmpStr);
		tmpStr = inet_ntoa(*((struct in_addr *)entry.gateway));
		strcpy(gateway, tmpStr);

		nBytesSent += req_format_write(wp, "<tr>"
			"<td align=center width=\"23%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"23%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"23%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%d</td>\n"
      			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n",
				ip, netmask, gateway,entry.metric,(entry.interface?"WAN":"LAN"), i);
	}
	return nBytesSent;
}

int CheckRouteType(STATICROUTE_Tp checkentry)
{
	int	entryNum, i;
	STATICROUTE_T entry;
	DHCP_T dhcp;
	
	int isFound=1;
	
	if ( !apmib_get(MIB_STATICROUTE_TBL_NUM, (void *)&entryNum)) {
		return -1;
	}
	apmib_get( MIB_WAN_DHCP, (void *)&dhcp);
	for (i=1; i<=entryNum; i++) {
		memset(&entry, '\0', sizeof(entry));
		*((char *)&entry) = (char)i;
		if ( !apmib_get(MIB_STATICROUTE_TBL, (void *)&entry)){
			fprintf(stderr,"Get route entry fail\n");
			return -1;
		}
		if (dhcp == PPPOE || dhcp == PPTP || dhcp == L2TP) {
			if(entry.interface ==1){
				if(!memcmp(checkentry->dstAddr, &entry.dstAddr, 4) && !memcmp(checkentry->netmask, &entry.netmask, 4) && entry.interface == checkentry->interface ){
					isFound=0;		
					break;
				}
			}else{
				if(!memcmp(checkentry->dstAddr, &entry.dstAddr, 4) && !memcmp(checkentry->netmask, &entry.netmask, 4) && !memcmp(checkentry->gateway, &entry.gateway, 4) && entry.interface == checkentry->interface){
					isFound=0;		
					break;
				}
			}
		}else{
			if(!memcmp(checkentry->dstAddr, &entry.dstAddr, 4) && !memcmp(checkentry->netmask, &entry.netmask, 4) && !memcmp(checkentry->gateway, &entry.gateway, 4) && entry.interface == checkentry->interface){
				isFound=0;		
				break;
			}
		}
	}
	if(isFound==0)
		return 1;
	else
		return 0;
}
int kernelRouteList(request *wp, int argc, char **argv)
{
	char buff[256];
	char iface[30];
	int nl = 0;
	struct in_addr dest;
	struct in_addr gw;
	struct in_addr mask;
	int flgs, ref, use, metric, mtu, win, ir;
	char flags[64];
	unsigned long int d, g, m;
	STATICROUTE_T entry;
	char sdest[16], sgw[16];
	int	nBytesSent=0;
	char *interface_WAN=NULL;
	char *interface_LAN=NULL;
	int opmode=0;
	int entry_type=0;
	int static_route_enabled=0;
	DHCP_T dhcp;
	char *interface_WANPhy="eth1";     	
	
	apmib_get( MIB_WAN_DHCP, (void *)&dhcp);
	apmib_get(MIB_OP_MODE, (void *)&opmode);
	apmib_get(MIB_STATICROUTE_ENABLED, (void *)&static_route_enabled);
	if ( dhcp == PPPOE || dhcp == PPTP || dhcp == L2TP) {
			interface_WAN = PPPOE_IF;
	}else
			interface_WAN = WAN_IF;
		
		
	interface_LAN = BRIDGE_IF;	
	
	if(opmode==2)
		interface_WAN = ("wlan0");


	FILE *fp = fopen("/proc/net/route", "r");
	nBytesSent += req_format_write(wp, "<tr>"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Destination </b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Gateway</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Genmask</b></font></td>\n"
      	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Metric </b></font></td>\n"
      	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Interface</b></font></td>"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Type</b></font></td></tr>\n");
	while (fgets(buff, sizeof(buff), fp) != NULL) {
		if (nl) {
			int ifl = 0;
			while (buff[ifl] != ' ' && buff[ifl] != '\t' && buff[ifl] != '\0')
				ifl++;
			memset(&entry, '\0', sizeof(entry));	
			strncpy(iface, buff, ifl);
			iface[ifl]='\0';
				
			if(!strcmp(iface, interface_WAN)){
					entry.interface=1;
			}
			if ( dhcp ==PPTP || dhcp == L2TP) {
					if(!strcmp(iface, interface_WANPhy)){
						entry.interface=1;
					}
			}
			if(!strcmp(iface, interface_LAN)){
				entry.interface=0;	
			}
				
			buff[ifl] = 0;	/* interface */
			
			if (sscanf(buff + ifl + 1, "%lx%lx%X%d%d%d%lx%d%d%d",
					   &d, &g, &flgs, &ref, &use, &metric, &m, &mtu, &win,
					   &ir) != 10) {
				printf("Unsuported kernel route format\n");
			}
			ifl = 0;	/* parse flags */
			if (flgs & RTF_UP) {
				if (flgs & RTF_REJECT)
					flags[ifl++] = '!';
				else
					flags[ifl++] = 'U';
				if (flgs & RTF_GATEWAY)
					flags[ifl++] = 'G';
				if (flgs & RTF_HOST)
					flags[ifl++] = 'H';
				if (flgs & RTF_REINSTATE)
					flags[ifl++] = 'R';
				if (flgs & RTF_DYNAMIC)
					flags[ifl++] = 'D';
				if (flgs & RTF_MODIFIED)
					flags[ifl++] = 'M';
				flags[ifl] = 0;
				dest.s_addr = d;
				gw.s_addr = g;
				mask.s_addr = m;
				strcpy(sdest, inet_ntoa(dest));
				strcpy(sgw, inet_ntoa(gw));
				
				memcpy(&(entry.dstAddr), &(dest.s_addr), 4); 
				memcpy(&(entry.gateway), &(gw.s_addr), 4); 
				memcpy(&(entry.netmask), &(mask.s_addr), 4); 
				
				if(static_route_enabled==1)
					entry_type=CheckRouteType(&entry);
				else
					entry_type=0;

	nBytesSent += req_format_write(wp, "<tr>"
      			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%-16s</td>\n"
      			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%-16s</td>\n"
      			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%-16s</td>\n"
      			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%d</td>\n"
      			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td></tr>\n",
			sdest, sgw, inet_ntoa(mask), metric, (entry.interface?"WAN":"LAN"),(entry_type?"Static":"Dynamic"));
			}
		}
		nl++;
	}
	return nBytesSent;
}
#endif

#endif //ROUTE_SUPPORT
