/*
 *      Web server handler routines for firewall
 *
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *
 *      $Id: fmfwall.c,v 1.20 2009/07/09 03:21:23 keith_huang Exp $
 *
 */

/*-- System inlcude files --*/
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/wait.h>

#include "boa.h"
#include "globals.h"
#include "apform.h"
#include "apmib.h"
#include "utility.h"
#include "asp_page.h"

#if defined(CONFIG_RTL_8198_AP_ROOT) && defined(VLAN_CONFIG_SUPPORTED)
struct nameMapping
{
	char display[32];
	char ifname[16];
};
static struct nameMapping vlanNameMapping[10] =
{
	{"Ethernet Port1","eth0"},
	{"Ethernet Port2","eth1"},
	{"Ethernet Port3","eth2"},
	{"Ethernet Port4","eth3"},
	{"Ethernet Port5","eth4"},
	{"Wireless Primary AP","wlan0"},
	{"Virtual AP1","wlan0-va0"},
	{"Virtual AP2","wlan0-va1"},
	{"Virtual AP3","wlan0-va2"},
	{"Virtual AP4","wlan0-va3"},
};

static struct nameMapping* findNameMapping(const char *display)
{
	int i;
	for(i = 0; i < MAX_IFACE_VLAN_CONFIG;i++)
	{
		if(strcmp(display,vlanNameMapping[i].display) == 0)
			return &vlanNameMapping[i];
	}
	return NULL;
}

int vlanList(request *wp, int idx)
{
	VLAN_CONFIG_T entry;
	char *strToken;
	int cmpResult=0;
	//char *tmpStr0;
	int  index=0;
	char IfaceName[32];
	OPMODE_T opmode=-1;
	char wanLan[8];
	char bufStr[128];

#if defined(CONFIG_RTK_VLAN_NEW_FEATURE) ||defined(CONFIG_RTL_HW_VLAN_SUPPORT)
	unsigned char forwarding_rule;
#endif

	memset(IfaceName,0x00,sizeof(IfaceName));
	memset(wanLan,0x00,sizeof(wanLan));
	memset(bufStr,0x00,sizeof(bufStr));

	index = idx;

	if( index <= MAX_IFACE_VLAN_CONFIG && index != 0) /* ignore item 0 */
	{
		*((char *)&entry) = (char)index;
		if ( !apmib_get(MIB_VLANCONFIG_TBL, (void *)&entry))
		{
			fprintf(stderr,"Get vlan entry fail\n");
			return -1;
		}

#if defined(CONFIG_RTK_VLAN_NEW_FEATURE) ||defined(CONFIG_RTL_HW_VLAN_SUPPORT)
		forwarding_rule = entry.forwarding_rule;
#endif
		apmib_get( MIB_OP_MODE, (void *)&opmode);

		switch(index)
		{
			case 1:
			case 2:
			case 3:
			case 4:
				sprintf(IfaceName,"%s%d","Ethernet Port",index);
				sprintf(wanLan,"%s","LAN");
				break;
			case 5:
				sprintf(IfaceName,"%s","Wireless 1 Primary AP");
				if(opmode == WISP_MODE)
				{
					sprintf(wanLan,"%s","WAN");

#if defined(CONFIG_RTK_VLAN_NEW_FEATURE)
					forwarding_rule = VLAN_FORWARD_NAT;
#endif
				}
				else
				{
					sprintf(wanLan,"%s","LAN");
				}
				break;
			case 6:
			case 7:
			case 8:
			case 9:
				sprintf(IfaceName,"%s%d","Virtual AP",index-5);
				sprintf(wanLan,"%s","LAN");
				break;
			case 10:
				sprintf(IfaceName,"%s","Wireless 2 Primary AP");
				sprintf(wanLan,"%s","LAN");
				break;
			case 11:
			case 12:
			case 13:
			case 14:
				sprintf(IfaceName,"%s%d","Virtual AP",index-10);
				sprintf(wanLan,"%s","LAN");
				break;

			case 15:
				sprintf(IfaceName,"%s","Ethernet Port5");
#ifdef RTK_USB3G_PORT5_LAN
				if(opmode == WISP_MODE || opmode == BRIDGE_MODE || wan_dhcp == USB3G)
#else
				if(opmode == WISP_MODE || opmode == BRIDGE_MODE)
#endif
				{
					sprintf(wanLan,"%s","LAN");
				}
				else
				{
					sprintf(wanLan,"%s","WAN");

#if defined(CONFIG_RTK_VLAN_NEW_FEATURE) ||defined(CONFIG_RTL_HW_VLAN_SUPPORT)
					forwarding_rule = VLAN_FORWARD_NAT;
#endif
				}
				break;
			case 16:
			sprintf(IfaceName,"%s","Local Host/WAN");
				sprintf(wanLan,"%s","LAN");
				break;
		}

		/* enabled/netIface/tagged/untagged/priority/cfi/groupId/vlanId/LanWan */
		//req_format_write(wp, ("%d|%s|%d|%d|%d|%d|%d|%d|%s"), entry.enabled,IfaceName,entry.tagged,0,entry.priority,entry.cfi,0,entry.vlanId,wanLan);
#if defined(CONFIG_RTK_VLAN_NEW_FEATURE) ||defined(CONFIG_RTL_HW_VLAN_SUPPORT)
		sprintf(bufStr, "token[%d] =\'%d|%s|%d|%d|%d|%d|%d|%d|%s|%d\';\n",idx, entry.enabled,IfaceName,entry.tagged,0,entry.priority,entry.cfi,0,entry.vlanId,wanLan, forwarding_rule);
#else
		sprintf(bufStr, "token[%d] =\'%d|%s|%d|%d|%d|%d|%d|%d|%s\';\n",idx,entry.enabled,IfaceName,entry.tagged,0,entry.priority,entry.cfi,0,entry.vlanId,wanLan);
#endif

	}
	else
	{
#if defined(CONFIG_RTK_VLAN_NEW_FEATURE) ||defined(CONFIG_RTL_HW_VLAN_SUPPORT)
		sprintf(bufStr, "token[%d] =\'0|none|0|0|0|0|0|0|LAN|0\';\n", idx);
#else
		sprintf(bufStr, "token[%d] =\'0|none|0|0|0|0|0|0|LAN\';\n", idx);
#endif
	}
	req_format_write(wp, bufStr);
	return 0;
}

int getVlanList(request *wp, int argc, char **argv)
{
	int i, maxWebVlanNum;

#if defined(CONFIG_RTL_8198_AP_ROOT) && defined(GMII_ENABLED)
	maxWebVlanNum = MAX_IFACE_VLAN_CONFIG-2;
#else
	maxWebVlanNum = MAX_IFACE_VLAN_CONFIG-1;
#endif
	for (i=0; i<=maxWebVlanNum; i++) {
		vlanList(wp, i);
	}
}

void formVlan(request *wp, char *path, char *query)
{
	VLAN_CONFIG_T entry;
	char *submitUrl,*strTmp;
	int	i, vlan_onoff;
	struct nameMapping *mapping;
	char tmpBuf[100];

	//displayPostDate(wp->post_data);

	strTmp= req_get_cstream_var(wp, ("vlan_onoff"), "");
	if(strTmp[0])
	{
		vlan_onoff = atoi(strTmp);
	}

	if (!apmib_set(MIB_VLANCONFIG_ENABLED, (void *)&vlan_onoff))
	{
		strcpy(tmpBuf, ("set  MIB_VLANCONFIG_ENABLED error!"));
		goto setErr;
	}
	if(vlan_onoff == 1)
	{
		if ( !apmib_set(MIB_VLANCONFIG_DELALL, (void *)&entry))
		{
			strcpy(tmpBuf, ("Delete all table error!"));
			goto setErr;
		}

		for(i=1; i<=MAX_IFACE_VLAN_CONFIG ; i++)
		{
			memset(&entry, '\0', sizeof(entry));

			*((char *)&entry) = (char)i;
			apmib_get(MIB_VLANCONFIG_TBL, (void *)&entry);

			memset(tmpBuf,0x00, sizeof(tmpBuf));
			sprintf(tmpBuf,"vlan_iface_%d",i);
			strTmp = req_get_cstream_var(wp, tmpBuf, "");

			if(strTmp[0])
			{
				//strcpy(entry.netIface,strTmp);
				mapping = findNameMapping(strTmp);
				if(mapping)
				{
					strcpy(entry.netIface,mapping->ifname);
				}
			}
			else
			{
				if ( apmib_set(MIB_VLANCONFIG_ADD, (void *)&entry) == 0)
				{
					strcpy(tmpBuf, ("Add table entry error!"));
					goto setErr;
				}
				continue;
			}

			memset(tmpBuf,0x00, sizeof(tmpBuf));
			sprintf(tmpBuf,"vlan_enable_%d",i);
			strTmp = req_get_cstream_var(wp, tmpBuf, "");
			if(strTmp[0])
			{
				entry.enabled = atoi(strTmp);
			}

			memset(tmpBuf,0x00, sizeof(tmpBuf));
			sprintf(tmpBuf,"vlan_tag_%d",i);
			strTmp = req_get_cstream_var(wp, tmpBuf, "");
			if(strTmp[0])
			{
				entry.tagged = atoi(strTmp);
			}

			memset(tmpBuf,0x00, sizeof(tmpBuf));
			sprintf(tmpBuf,"vlan_cfg_%d",i);
			strTmp = req_get_cstream_var(wp, tmpBuf, "");
			if(strTmp[0])
			{
				entry.cfi = atoi(strTmp);
			}

			memset(tmpBuf,0x00, sizeof(tmpBuf));
			sprintf(tmpBuf,"vlan_id_%d",i);
			strTmp = req_get_cstream_var(wp, tmpBuf, "");
			if(strTmp[0])
			{
				entry.vlanId = atoi(strTmp);
			}

			memset(tmpBuf,0x00, sizeof(tmpBuf));
			sprintf(tmpBuf,"vlan_priority_%d",i);
			strTmp = req_get_cstream_var(wp, tmpBuf, "");
			if(strTmp[0])
			{
				entry.priority = atoi(strTmp);
			}

#if defined(CONFIG_RTK_VLAN_NEW_FEATURE) ||defined(CONFIG_RTL_HW_VLAN_SUPPORT)
			memset(tmpBuf,0x00, sizeof(tmpBuf));
			sprintf(tmpBuf,"vlan_forward_%d",i);
			strTmp = req_get_cstream_var(wp, tmpBuf, "");
			if(strTmp[0])
			{
				entry.forwarding_rule = atoi(strTmp);
			}
#endif

			if ( apmib_set(MIB_VLANCONFIG_ADD, (void *)&entry) == 0)
			{
				strcpy(tmpBuf, ("Add table entry error!"));
				printf("--%s(%d)--\n", __FUNCTION__, __LINE__);
				goto setErr;
			}




		}

	}

	apmib_update_web(CURRENT_SETTING);

#ifndef NO_ACTION
	run_init_script("all");
#endif

	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page
	if (submitUrl[0])
	{
		OK_MSG(submitUrl);
	}
  	return;

setErr:
	ERR_MSG(tmpBuf);
	return;

}
#endif

#ifdef HOME_GATEWAY

/////////////////////////////////////////////////////////////////////////////
void formPortFw(request *wp, char *path, char *query)
{
	char *submitUrl, *strAddPort, *strDelPort, *strVal, *strDelAllPort;
	char *strIp, *strFrom, *strTo, *strComment;
	char tmpBuf[100];
	int entryNum, intVal, i;
	PORTFW_T entry;
	struct in_addr curIpAddr, curSubnet;
	unsigned long v1, v2, v3;
#ifndef NO_ACTION
	int pid;
#endif

	strAddPort = req_get_cstream_var(wp, ("addPortFw"), "");
	strDelPort = req_get_cstream_var(wp, ("deleteSelPortFw"), "");
	strDelAllPort = req_get_cstream_var(wp, ("deleteAllPortFw"), "");

	memset(&entry, '\0', sizeof(entry));

	/* Add new port-forwarding table */
	if (strAddPort[0]) {
		strVal = req_get_cstream_var(wp, ("enabled"), "");
		if ( !strcmp(strVal, "ON"))
			intVal = 1;
		else
			intVal = 0;
		if ( apmib_set( MIB_PORTFW_ENABLED, (void *)&intVal) == 0) {
			strcpy(tmpBuf, ("Set enabled flag error!"));
			goto setErr_portfw;
		}

		strIp = req_get_cstream_var(wp, ("ip"), "");
		strFrom = req_get_cstream_var(wp, ("fromPort"), "");
		strTo = req_get_cstream_var(wp, ("toPort"), "");
		strComment = req_get_cstream_var(wp, ("comment"), "");

		if (!strIp[0] && !strFrom[0] && !strTo[0] && !strComment[0])
			goto setOk_portfw;

		if (!strIp[0]) {
			strcpy(tmpBuf, ("Error! No ip address to set."));
			goto setErr_portfw;
		}

		inet_aton(strIp, (struct in_addr *)&entry.ipAddr);
		getInAddr(BRIDGE_IF, IP_ADDR, (void *)&curIpAddr);
		getInAddr(BRIDGE_IF, SUBNET_MASK, (void *)&curSubnet);

		v1 = *((unsigned long *)entry.ipAddr);
		v2 = *((unsigned long *)&curIpAddr);
		v3 = *((unsigned long *)&curSubnet);

		if ( (v1 & v3) != (v2 & v3) ) {
			strcpy(tmpBuf, ("Invalid IP address! It should be set within the current subnet."));
			goto setErr_portfw;
		}

		if ( !strFrom[0] ) { // if port-forwarding, from port must exist
			strcpy(tmpBuf, ("Error! No from-port value to be set."));
			goto setErr_portfw;
		}
		if ( !string_to_dec(strFrom, &intVal) || intVal<1 || intVal>65535) {
			strcpy(tmpBuf, ("Error! Invalid value of from-port."));
			goto setErr_portfw;
		}
		entry.fromPort = (unsigned short)intVal;

		if ( !strTo[0] )
			entry.toPort = entry.fromPort;
		else {
			if ( !string_to_dec(strTo, &intVal) || intVal<1 || intVal>65535) {
				strcpy(tmpBuf, ("Error! Invalid value of to-port."));
				goto setErr_portfw;
			}
		}
		entry.toPort = (unsigned short)intVal;

		if ( entry.fromPort  > entry.toPort ) {
			strcpy(tmpBuf, ("Error! Invalid port range."));
			goto setErr_portfw;
		}

		strVal = req_get_cstream_var(wp, ("protocol"), "");
		if (strVal[0]) {
			if ( strVal[0] == '0' )
				entry.protoType = PROTO_BOTH;
			else if ( strVal[0] == '1' )
				entry.protoType = PROTO_TCP;
			else if ( strVal[0] == '2' )
				entry.protoType = PROTO_UDP;
			else {
				strcpy(tmpBuf, ("Error! Invalid protocol type."));
				goto setErr_portfw;
			}
		}
		else {
			strcpy(tmpBuf, ("Error! Protocol type cannot be empty."));
			goto setErr_portfw;
		}

		if ( strComment[0] ) {
			if (strlen(strComment) > COMMENT_LEN-1) {
				strcpy(tmpBuf, ("Error! Comment length too long."));
				goto setErr_portfw;
			}
			strcpy((char *)entry.comment, strComment);
		}
		if ( !apmib_get(MIB_PORTFW_TBL_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto setErr_portfw;
		}

		if ( (entryNum + 1) > MAX_FILTER_NUM) {
			strcpy(tmpBuf, ("Cannot add new entry because table is full!"));
			goto setErr_portfw;
		}

		// Check if there is any port overlapped
		for (i=1; i<=entryNum; i++) {
			PORTFW_T checkEntry;
			*((char *)&checkEntry) = (char)i;
			if ( !apmib_get(MIB_PORTFW_TBL, (void *)&checkEntry)) {
				strcpy(tmpBuf, ("Get table entry error!"));
				goto setErr_portfw;
			}
			if ( ( (entry.fromPort <= checkEntry.fromPort &&
					entry.toPort >= checkEntry.fromPort) ||
			       (entry.fromPort >= checkEntry.fromPort &&
				entry.fromPort <= checkEntry.toPort)
			     )&&
			       (entry.protoType & checkEntry.protoType) ) {
				strcpy(tmpBuf, ("Setting port range has overlapped with used port numbers!"));
				goto setErr_portfw;
			}
		}

		// set to MIB. try to delete it first to avoid duplicate case
		apmib_set(MIB_PORTFW_DEL, (void *)&entry);
		if ( apmib_set(MIB_PORTFW_ADD, (void *)&entry) == 0) {
			strcpy(tmpBuf, ("Add table entry error!"));
			goto setErr_portfw;
		}
	}

	/* Delete entry */
	if (strDelPort[0]) {
		if ( !apmib_get(MIB_PORTFW_TBL_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto setErr_portfw;
		}

		for (i=entryNum; i>0; i--) {
			snprintf(tmpBuf, 20, "select%d", i);

			strVal = req_get_cstream_var(wp, tmpBuf, "");
			if ( !strcmp(strVal, "ON") ) {
				*((char *)&entry) = (char)i;
				if ( !apmib_get(MIB_PORTFW_TBL, (void *)&entry)) {
					strcpy(tmpBuf, ("Get table entry error!"));
					goto setErr_portfw;
				}
				if ( !apmib_set(MIB_PORTFW_DEL, (void *)&entry)) {
					strcpy(tmpBuf, ("Delete table entry error!"));
					goto setErr_portfw;
				}
			}
		}
	}

	/* Delete all entry */
	if ( strDelAllPort[0]) {
		if ( !apmib_set(MIB_PORTFW_DELALL, (void *)&entry)) {
			strcpy(tmpBuf, ("Delete all table error!"));
			goto setErr_portfw;
		}
	}

setOk_portfw:
	apmib_update_web(CURRENT_SETTING);

#ifndef NO_ACTION
	pid = fork();
        if (pid) {
	      	waitpid(pid, NULL, 0);
	}
        else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _FIREWALL_SCRIPT_PROG);
		execl( tmpBuf, _FIREWALL_SCRIPT_PROG, NULL);
               	exit(1);
        }
#endif

	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page

#ifdef REBOOT_CHECK
	if(needReboot == 1)
	{
		OK_MSG(submitUrl);
		return;
	}
#endif

	if (submitUrl[0])
		send_redirect_perm(wp, submitUrl);
  	return;

setErr_portfw:
	ERR_MSG(tmpBuf);
}


/////////////////////////////////////////////////////////////////////////////
void formFilter(request *wp, char *path, char *query)
{
	char *strAddIp, *strAddPort, *strAddMac, *strDelPort, *strDelIp, *strDelMac;
	char *strDelAllPort, *strDelAllIp, *strDelAllMac, *strVal, *submitUrl, *strComment;
	char *strFrom, *strTo;
	char tmpBuf[100];
	int entryNum, intVal, i;
	IPFILTER_T ipEntry;
	PORTFILTER_T portEntry;
	MACFILTER_T macEntry;
	struct in_addr curIpAddr, curSubnet;
	void *pEntry;
	unsigned long v1, v2, v3;
	int num_id, get_id, add_id, del_id, delall_id, enable_id;
	char *strAddUrl, *strDelUrl;
	char *strDelAllUrl,*strUrlMode;
	int mode;
	URLFILTER_T urlEntry;
#ifndef NO_ACTION
	int pid;
#endif

	strAddIp = req_get_cstream_var(wp, ("addFilterIp"), "");
	strDelIp = req_get_cstream_var(wp, ("deleteSelFilterIp"), "");
	strDelAllIp = req_get_cstream_var(wp, ("deleteAllFilterIp"), "");

	strAddPort = req_get_cstream_var(wp, ("addFilterPort"), "");
	strDelPort = req_get_cstream_var(wp, ("deleteSelFilterPort"), "");
	strDelAllPort = req_get_cstream_var(wp, ("deleteAllFilterPort"), "");

	strAddMac = req_get_cstream_var(wp, ("addFilterMac"), "");
	strDelMac = req_get_cstream_var(wp, ("deleteSelFilterMac"), "");
	strDelAllMac = req_get_cstream_var(wp, ("deleteAllFilterMac"), "");

	strAddUrl = req_get_cstream_var(wp, ("addFilterUrl"), "");
	strDelUrl = req_get_cstream_var(wp, ("deleteSelFilterUrl"), "");
	strDelAllUrl = req_get_cstream_var(wp, ("deleteAllFilterUrl"), "");

	if (strAddIp[0] || strDelIp[0] || strDelAllIp[0]) {
		num_id = MIB_IPFILTER_TBL_NUM;
		get_id = MIB_IPFILTER_TBL;
		add_id = MIB_IPFILTER_ADD;
		del_id = MIB_IPFILTER_DEL;
		delall_id = MIB_IPFILTER_DELALL;
		enable_id = MIB_IPFILTER_ENABLED;
		memset(&ipEntry, '\0', sizeof(ipEntry));
		pEntry = (void *)&ipEntry;
	}
	else if (strAddPort[0] || strDelPort[0] || strDelAllPort[0]) {
		num_id = MIB_PORTFILTER_TBL_NUM;
		get_id = MIB_PORTFILTER_TBL;
		add_id = MIB_PORTFILTER_ADD;
		del_id = MIB_PORTFILTER_DEL;
		delall_id = MIB_PORTFILTER_DELALL;
		enable_id = MIB_PORTFILTER_ENABLED;
		memset(&portEntry, '\0', sizeof(portEntry));
		pEntry = (void *)&portEntry;
	}
	else if (strAddMac[0] || strDelMac[0] || strDelAllMac[0]) {
		num_id = MIB_MACFILTER_TBL_NUM;
		get_id = MIB_MACFILTER_TBL;
		add_id = MIB_MACFILTER_ADD;
		del_id = MIB_MACFILTER_DEL;
		delall_id = MIB_MACFILTER_DELALL;
		enable_id = MIB_MACFILTER_ENABLED;
		memset(&macEntry, '\0', sizeof(macEntry));
		pEntry = (void *)&macEntry;
	}
	else {
		num_id = MIB_URLFILTER_TBL_NUM;
		get_id = MIB_URLFILTER_TBL;
		add_id = MIB_URLFILTER_ADD;
		del_id = MIB_URLFILTER_DEL;
		delall_id = MIB_URLFILTER_DELALL;
		enable_id = MIB_URLFILTER_ENABLED;
		memset(&urlEntry, '\0', sizeof(urlEntry));
		pEntry = (void *)&urlEntry;
	}
	// Set enable flag
	if ( strAddIp[0] || strAddPort[0] || strAddMac[0] || strAddUrl[0]) {
		strVal = req_get_cstream_var(wp, ("enabled"), "");
		if ( !strcmp(strVal, "ON"))
			intVal = 1;
		else
			intVal = 0;

		if ( apmib_set(enable_id, (void *)&intVal) == 0) {
			strcpy(tmpBuf, ("Set enabled flag error!"));
			goto setErr_filter;
		}
	}

	strComment = req_get_cstream_var(wp, ("comment"), "");

	/* Add IP filter */
	if (strAddIp[0]) {
		strVal = req_get_cstream_var(wp, ("ip"), "");
		if (!strVal[0] && !strComment[0])
			goto setOk_filter;

		if (!strVal[0]) {
			strcpy(tmpBuf, ("Error! No ip address to set."));
			goto setErr_filter;
		}
		inet_aton(strVal, (struct in_addr *)&ipEntry.ipAddr);
		getInAddr(BRIDGE_IF, IP_ADDR, (void *)&curIpAddr);
		getInAddr(BRIDGE_IF, SUBNET_MASK, (void *)&curSubnet);

		v1 = *((unsigned long *)ipEntry.ipAddr);
		v2 = *((unsigned long *)&curIpAddr);
		v3 = *((unsigned long *)&curSubnet);

		if ( (v1 & v3) != (v2 & v3) ) {
			strcpy(tmpBuf, ("Invalid IP address! It should be set within the current subnet."));
			goto setErr_filter;
		}
	}

	/* Add port filter */
	if (strAddPort[0]) {
		strFrom = req_get_cstream_var(wp, ("fromPort"), "");
		strTo = req_get_cstream_var(wp, ("toPort"), "");
		if (!strFrom[0] && !strTo[0] && !strComment[0])
			goto setOk_filter;

		if (!strFrom[0]) { // if port-forwarding, from port must exist
			strcpy(tmpBuf, ("Error! No from-port value to be set."));
			goto setErr_filter;
		}
		if ( !string_to_dec(strFrom, &intVal) || intVal<1 || intVal>65535) {
			strcpy(tmpBuf, ("Error! Invalid value of from-port."));
			goto setErr_filter;
		}
		portEntry.fromPort = (unsigned short)intVal;

		if ( !strTo[0] )
			portEntry.toPort = portEntry.fromPort;
		else {
			if ( !string_to_dec(strTo, &intVal) || intVal<1 || intVal>65535) {
				strcpy(tmpBuf, ("Error! Invalid value of to-port."));
				goto setErr_filter;
			}
			portEntry.toPort = (unsigned short)intVal;
		}

		if ( portEntry.fromPort  > portEntry.toPort ) {
			strcpy(tmpBuf, ("Error! Invalid port range."));
			goto setErr_filter;
		}
	}

	if (strAddPort[0] || strAddIp[0]) {
		strVal = req_get_cstream_var(wp, ("protocol"), "");
		if (strVal[0]) {
			if ( strVal[0] == '0' ) {
				if (strAddPort[0])
					portEntry.protoType = PROTO_BOTH;
				else
					ipEntry.protoType = PROTO_BOTH;
			}
			else if ( strVal[0] == '1' ) {
				if (strAddPort[0])
					portEntry.protoType = PROTO_TCP;
				else
					ipEntry.protoType = PROTO_TCP;
			}
			else if ( strVal[0] == '2' ) {
				if (strAddPort[0])
					portEntry.protoType = PROTO_UDP;
				else
					ipEntry.protoType = PROTO_UDP;
			}
			else {
				strcpy(tmpBuf, ("Error! Invalid protocol type."));
				goto setErr_filter;
			}
		}
		else {
			strcpy(tmpBuf, ("Error! Protocol type cannot be empty."));
			goto setErr_filter;
		}
	}

	if (strAddMac[0]) {
		strVal = req_get_cstream_var(wp, ("mac"), "");
		if (!strVal[0] && !strComment[0])
			goto setOk_filter;

		if ( !strVal[0] ) {
			strcpy(tmpBuf, ("Error! No mac address to set."));
			goto setErr_filter;
		}
		if (strlen(strVal)!=12 || !string_to_hex(strVal, macEntry.macAddr, 12)) {
			strcpy(tmpBuf, ("Error! Invalid MAC address."));
			goto setErr_filter;
		}
	}

	if (strAddUrl[0]) {
		strUrlMode = req_get_cstream_var(wp, "urlFilterMode", "");
		if(strUrlMode){
			mode=atoi(strUrlMode);
			if ( apmib_set(MIB_URLFILTER_MODE, (void *)&mode) == 0) {
		    	strcpy(tmpBuf, ("Set mode flag error!"));
			    goto setErr_filter;
		    }
        }
		strVal = req_get_cstream_var(wp, "url", "");
		if (!strVal[0])// && !strComment[0])
			goto setOk_filter;

		if ( !strVal[0] ) {
			strcpy(tmpBuf, ("Error! No url keyword to set."));
			goto setErr_filter;
		}
		else
		{
			strcpy((char *)urlEntry.urlAddr, strVal);
			urlEntry.ruleMode=mode;
		}
	}

	if (strAddIp[0] || strAddPort[0] || strAddMac[0] || strAddUrl[0]) {
		if ( strComment[0] ) {
			if (strlen(strComment) > COMMENT_LEN-1) {
				strcpy(tmpBuf, ("Error! Comment length too long."));
				goto setErr_filter;
			}
			if (strAddIp[0])
				strcpy((char *)ipEntry.comment, strComment);
			else if (strAddPort[0])
				strcpy((char *)portEntry.comment, strComment);
			else if (strAddMac[0])
				strcpy((char *)macEntry.comment, strComment);
		}

		if ( !apmib_get(num_id, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto setErr_filter;
		}
		if (strAddUrl[0])
		{
			if ( (entryNum + 1) > MAX_URLFILTER_NUM) {
				strcpy(tmpBuf, ("Cannot add new URL entry because table is full!"));
				goto setErr_filter;
			}
		}
		else
		{
			if ( (entryNum + 1) > MAX_FILTER_NUM) {
				strcpy(tmpBuf, ("Cannot add new entry because table is full!"));
				goto setErr_filter;
			}
		}

		// set to MIB. try to delete it first to avoid duplicate case
		apmib_set(del_id, pEntry);
		if ( apmib_set(add_id, pEntry) == 0) {
			strcpy(tmpBuf, ("Add table entry error!"));
			goto setErr_filter;
		}
	}


	/* Delete entry */
	if (strDelPort[0] || strDelIp[0] || strDelMac[0] || strDelUrl[0]) {
		if ( !apmib_get(num_id, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto setErr_filter;
		}
		for (i=entryNum; i>0; i--) {
			snprintf(tmpBuf, 20, "select%d", i);

			strVal = req_get_cstream_var(wp, tmpBuf, "");
			if ( !strcmp(strVal, "ON") ) {

				*((char *)pEntry) = (char)i;
				if ( !apmib_get(get_id, pEntry)) {
					strcpy(tmpBuf, ("Get table entry error!"));
					goto setErr_filter;
				}
				if ( !apmib_set(del_id, pEntry)) {
					strcpy(tmpBuf, ("Delete table entry error!"));
					goto setErr_filter;
				}
			}
		}
	}

	/* Delete all entry */
	if ( strDelAllPort[0] || strDelAllIp[0] || strDelAllMac[0] || strDelAllUrl[0]) {
		if ( !apmib_set(delall_id, pEntry)) {
			strcpy(tmpBuf, ("Delete all table error!"));
			goto setErr_filter;
		}
	}
setOk_filter:
	apmib_update_web(CURRENT_SETTING);

#ifndef NO_ACTION
	pid = fork();
        if (pid) {
	      	waitpid(pid, NULL, 0);
	}
        else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _FIREWALL_SCRIPT_PROG);
		execl( tmpBuf, _FIREWALL_SCRIPT_PROG, NULL);
               	exit(1);
        }
#endif

	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page

#ifdef REBOOT_CHECK
	if(needReboot == 1)
	{
		OK_MSG(submitUrl);
		return;
	}
#endif

	if (submitUrl[0])
		send_redirect_perm(wp, submitUrl);
  	return;

setErr_filter:
	ERR_MSG(tmpBuf);
}

#if 0
/////////////////////////////////////////////////////////////////////////////
void formTriggerPort(request *wp, char *path, char *query)
{
	char *strAddPort, *strDelAllPort, *strDelPort, *strVal, *submitUrl;
	char *strTriFrom, *strTriTo, *strIncFrom, *strIncTo, *strComment;
	char tmpBuf[100];
	int entryNum, intVal, i;
	TRIGGERPORT_T entry;

	memset(&entry, '\0', sizeof(entry));

	/* Add port filter */
	strAddPort = req_get_cstream_var(wp, ("addPort"), "");
	if (strAddPort[0]) {
		strVal = req_get_cstream_var(wp, ("enabled"), "");
		if ( !strcmp(strVal, "ON"))
			intVal = 1;
		else
			intVal = 0;

		if ( apmib_set(MIB_TRIGGERPORT_ENABLED, (void *)&intVal) == 0) {
			strcpy(tmpBuf, ("Set enabled flag error!"));
			goto setErr_triPort;
		}

		strTriFrom = req_get_cstream_var(wp, ("triFromPort"), "");
		strTriTo = req_get_cstream_var(wp, ("triToPort"), "");
		strIncFrom = req_get_cstream_var(wp, ("incFromPort"), "");
		strIncTo = req_get_cstream_var(wp, ("incToPort"), "");
		strComment = req_get_cstream_var(wp, ("comment"), "");

		if (!strTriFrom[0] && !strTriTo[0] && !strIncFrom[0] &&
					!strIncTo[0] && !strComment[0])
			goto setOk_triPort;

		// get trigger port range and protocol
		if (!strTriFrom[0]) { // from port must exist
			strcpy(tmpBuf, ("Error! No from-port value to be set."));
			goto setErr_triPort;
		}
		if ( !string_to_dec(strTriFrom, &intVal) || intVal<1 || intVal>65535) {
			strcpy(tmpBuf, ("Error! Invalid value of trigger from-port."));
			goto setErr_triPort;
		}
		entry.tri_fromPort = (unsigned short)intVal;

		if ( !strTriTo[0] )
			entry.tri_toPort = entry.tri_fromPort;
		else {
			if ( !string_to_dec(strTriTo, &intVal) || intVal<1 || intVal>65535) {
				strcpy(tmpBuf, ("Error! Invalid value of trigger to-port."));
				goto setErr_triPort;
			}
			entry.tri_toPort = (unsigned short)intVal;
		}

		if ( entry.tri_fromPort  > entry.tri_toPort ) {
			strcpy(tmpBuf, ("Error! Invalid trigger port range."));
			goto setErr_triPort;
		}

		strVal = req_get_cstream_var(wp, ("triProtocol"), "");
		if (strVal[0]) {
			if ( strVal[0] == '0' ) {
				if (strAddPort[0])
					entry.tri_protoType = PROTO_BOTH;
				else
					entry.tri_protoType = PROTO_BOTH;
			}
			else if ( strVal[0] == '1' ) {
				if (strAddPort[0])
					entry.tri_protoType = PROTO_TCP;
				else
					entry.tri_protoType = PROTO_TCP;
			}
			else if ( strVal[0] == '2' ) {
				if (strAddPort[0])
					entry.tri_protoType = PROTO_UDP;
				else
					entry.tri_protoType = PROTO_UDP;
			}
			else {
				strcpy(tmpBuf, ("Error! Invalid trigger-port protocol type."));
				goto setErr_triPort;
			}
		}
		else {
			strcpy(tmpBuf, ("Error! trigger-port protocol type cannot be empty."));
			goto setErr_triPort;
		}

		// get incoming port range and protocol
		if (!strIncFrom[0]) { // from port must exist
			strcpy(tmpBuf, ("Error! No from-port value to be set."));
			goto setErr_triPort;
		}
		if ( !string_to_dec(strIncFrom, &intVal) || intVal<1 || intVal>65535) {
			strcpy(tmpBuf, ("Error! Invalid value of incoming from-port."));
			goto setErr_triPort;
		}
		entry.inc_fromPort = (unsigned short)intVal;

		if ( !strIncTo[0] )
			entry.inc_toPort = entry.inc_fromPort;
		else {
			if ( !string_to_dec(strIncTo, &intVal) || intVal<1 || intVal>65535) {
				strcpy(tmpBuf, ("Error! Invalid value of incoming to-port."));
				goto setErr_triPort;
			}
			entry.inc_toPort = (unsigned short)intVal;
		}

		if ( entry.inc_fromPort  > entry.inc_toPort ) {
			strcpy(tmpBuf, ("Error! Invalid incoming port range."));
			goto setErr_triPort;
		}


		strVal = req_get_cstream_var(wp, ("incProtocol"), "");
		if (strVal[0]) {
			if ( strVal[0] == '0' ) {
				if (strAddPort[0])
					entry.inc_protoType = PROTO_BOTH;
				else
					entry.inc_protoType = PROTO_BOTH;
			}
			else if ( strVal[0] == '1' ) {
				if (strAddPort[0])
					entry.inc_protoType = PROTO_TCP;
				else
					entry.inc_protoType = PROTO_TCP;
			}
			else if ( strVal[0] == '2' ) {
				if (strAddPort[0])
					entry.inc_protoType = PROTO_UDP;
				else
					entry.inc_protoType = PROTO_UDP;
			}
			else {
				strcpy(tmpBuf, ("Error! Invalid incoming-port protocol type."));
				goto setErr_triPort;
			}
		}
		else {
			strcpy(tmpBuf, ("Error! incoming-port protocol type cannot be empty."));
			goto setErr_triPort;
		}

		// get comment
		if ( strComment[0] ) {
			if (strlen(strComment) > COMMENT_LEN-1) {
				strcpy(tmpBuf, ("Error! Comment length too long."));
				goto setErr_triPort;
			}
			strcpy(entry.comment, strComment);
		}

		// get entry number to see if it exceeds max
		if ( !apmib_get(MIB_TRIGGERPORT_TBL_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto setErr_triPort;
		}
		if ( (entryNum + 1) > MAX_FILTER_NUM) {
			strcpy(tmpBuf, ("Cannot add new entry because table is full!"));
			goto setErr_triPort;
		}

		// Check if there is any port overlapped
		for (i=1; i<=entryNum; i++) {
			TRIGGERPORT_T checkEntry;
			*((char *)&checkEntry) = (char)i;
			if ( !apmib_get(MIB_TRIGGERPORT_TBL, (void *)&checkEntry)) {
				strcpy(tmpBuf, ("Get table entry error!"));
				goto setErr_triPort;
			}
			if ( ( (entry.tri_fromPort <= checkEntry.tri_fromPort &&
					entry.tri_toPort >= checkEntry.tri_fromPort) ||
			       (entry.tri_fromPort >= checkEntry.tri_fromPort &&
				entry.tri_fromPort <= checkEntry.tri_toPort)
			     )&&
			       (entry.tri_protoType & checkEntry.tri_protoType) ) {
				strcpy(tmpBuf, ("Trigger port range has overlapped with used port numbers!"));
				goto setErr_triPort;
			}
		}

		// set to MIB. try to delete it first to avoid duplicate case
		apmib_set(MIB_TRIGGERPORT_DEL, (void *)&entry);
		if ( apmib_set(MIB_TRIGGERPORT_ADD, (void *)&entry) == 0) {
			strcpy(tmpBuf, ("Add table entry error!"));
			goto setErr_triPort;
		}
	}

	/* Delete entry */
	strDelPort = req_get_cstream_var(wp, ("deleteSelPort"), "");
	if (strDelPort[0]) {
		if ( !apmib_get(MIB_TRIGGERPORT_TBL_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto setErr_triPort;
		}
		for (i=entryNum; i>0; i--) {
			snprintf(tmpBuf, 20, "select%d", i);

			strVal = req_get_cstream_var(wp, tmpBuf, "");
			if ( !strcmp(strVal, "ON") ) {

				*((char *)&entry) = (char)i;
				if ( !apmib_get(MIB_TRIGGERPORT_TBL, (void *)&entry)) {
					strcpy(tmpBuf, ("Get table entry error!"));
					goto setErr_triPort;
				}
				if ( !apmib_set(MIB_TRIGGERPORT_DEL, (void *)&entry)) {
					strcpy(tmpBuf, ("Delete table entry error!"));
					goto setErr_triPort;
				}
			}
		}
	}

	/* Delete all entry */
	strDelAllPort = req_get_cstream_var(wp, ("deleteAllPort"), "");
	if ( strDelAllPort[0]) {
		if ( !apmib_set(MIB_TRIGGERPORT_DELALL, (void *)&entry)) {
			strcpy(tmpBuf, ("Delete all table error!"));
			goto setErr_triPort;
		}
	}

setOk_triPort:
	apmib_update_web(CURRENT_SETTING);

	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page
	if (submitUrl[0])
		send_redirect_perm(wp, submitUrl);
  	return;

setErr_triPort:
	ERR_MSG(tmpBuf);
}
#endif

#if defined(VLAN_CONFIG_SUPPORTED)
struct nameMapping
{
	char display[32];
	char ifname[16];
};
static struct nameMapping vlanNameMapping[15] =
{
	{"Ethernet Port1","eth0"},
	{"Ethernet Port2","eth2"},
	{"Ethernet Port3","eth3"},
	{"Ethernet Port4","eth4"},
	{"Ethernet Port5","eth1"},
	{"Wireless 1 Primary AP","wlan0"},
	{"Wireless 1 Virtual AP1","wlan0-va0"},
	{"Wireless 1 Virtual AP2","wlan0-va1"},
	{"Wireless 1 Virtual AP3","wlan0-va2"},
	{"Wireless 1 Virtual AP4","wlan0-va3"},
	{"Wireless 2 Primary AP","wlan1"},
	{"Wireless 2 Virtual AP1","wlan1-va0"},
	{"Wireless 2 Virtual AP2","wlan1-va1"},
	{"Wireless 2 Virtual AP3","wlan1-va2"},
	{"Wireless 2 Virtual AP4","wlan1-va3"},
};

static struct nameMapping* findNameMapping(const char *display)
{
	int i;
	for(i = 0; i < MAX_IFACE_VLAN_CONFIG;i++)
	{
		if(strcmp(display,vlanNameMapping[i].display) == 0)
			return &vlanNameMapping[i];
	}
	return NULL;
}

int vlanList(request *wp, int idx)
{
	VLAN_CONFIG_T entry;
	char *strToken;
	int cmpResult=0;
	//char *tmpStr0;
	int  index=0;
	char IfaceName[32];
	OPMODE_T opmode=-1;
	char wanLan[8];
	char bufStr[128];

#if defined(CONFIG_RTK_VLAN_NEW_FEATURE) ||defined(CONFIG_RTL_HW_VLAN_SUPPORT)
	unsigned char forwarding_rule;
#endif

	memset(IfaceName,0x00,sizeof(IfaceName));
	memset(wanLan,0x00,sizeof(wanLan));
	memset(bufStr,0x00,sizeof(bufStr));

	index = idx;

	if( index <= MAX_IFACE_VLAN_CONFIG && index != 0) /* ignore item 0 */
	{

    #ifdef RTK_USB3G_PORT5_LAN
        DHCP_T wan_dhcp = -1;
        apmib_get( MIB_DHCP, (void *)&wan_dhcp);
    #endif

		*((char *)&entry) = (char)index;

		if ( !apmib_get(MIB_VLANCONFIG_TBL, (void *)&entry))
		{
			fprintf(stderr,"Get vlan entry fail\n");
			return -1;
		}
#if defined(CONFIG_RTK_VLAN_NEW_FEATURE) ||defined(CONFIG_RTL_HW_VLAN_SUPPORT)
		forwarding_rule = entry.forwarding_rule;
#endif
		apmib_get( MIB_OP_MODE, (void *)&opmode);

		switch(index)
		{
			case 1:
			case 2:
			case 3:
			case 4:
				sprintf(IfaceName,"%s%d","Ethernet Port",index);
				sprintf(wanLan,"%s","LAN");
				break;
			case 5:
				sprintf(IfaceName,"%s","Wireless 1 Primary AP");
				if(opmode == WISP_MODE)
				{
					sprintf(wanLan,"%s","WAN");
#if defined(CONFIG_RTK_VLAN_NEW_FEATURE)
					forwarding_rule = VLAN_FORWARD_NAT;
#endif
				}
				else
				{
					sprintf(wanLan,"%s","LAN");
				}
				break;
			case 6:
			case 7:
			case 8:
			case 9:
				sprintf(IfaceName,"%s%d","Wireless 1 Virtual AP",index-5);
				sprintf(wanLan,"%s","LAN");
				break;
			case 10:
				sprintf(IfaceName,"%s","Wireless 2 Primary AP");
				sprintf(wanLan,"%s","LAN");
				break;
			case 11:
			case 12:
			case 13:
			case 14:
				sprintf(IfaceName,"%s%d","Wireless 2 Virtual AP",index-10);
				sprintf(wanLan,"%s","LAN");
				break;

			case 15:
				sprintf(IfaceName,"%s","Ethernet Port5");
#ifdef RTK_USB3G_PORT5_LAN
				if(opmode == WISP_MODE || opmode == BRIDGE_MODE || wan_dhcp == USB3G)
#else
				if(opmode == WISP_MODE || opmode == BRIDGE_MODE)
#endif
				{
					sprintf(wanLan,"%s","LAN");
				}
				else
				{
					sprintf(wanLan,"%s","WAN");

#if defined(CONFIG_RTK_VLAN_NEW_FEATURE) ||defined(CONFIG_RTL_HW_VLAN_SUPPORT)
					forwarding_rule = VLAN_FORWARD_NAT;
#endif
				}
				break;
			case 16:
			sprintf(IfaceName,"%s","Local Host/WAN");
				sprintf(wanLan,"%s","LAN");
				break;
		}

		/* enabled/netIface/tagged/untagged/priority/cfi/groupId/vlanId/LanWan */
		//req_format_write(wp, ("%d|%s|%d|%d|%d|%d|%d|%d|%s"), entry.enabled,IfaceName,entry.tagged,0,entry.priority,entry.cfi,0,entry.vlanId,wanLan);
#if defined(CONFIG_RTK_VLAN_NEW_FEATURE) ||defined(CONFIG_RTL_HW_VLAN_SUPPORT)
		sprintf(bufStr, "token[%d] =\'%d|%s|%d|%d|%d|%d|%d|%d|%s|%d\';\n",idx,entry.enabled,IfaceName,entry.tagged,0,entry.priority,entry.cfi,0,entry.vlanId,wanLan, forwarding_rule);
#else
		sprintf(bufStr, "token[%d] =\'%d|%s|%d|%d|%d|%d|%d|%d|%s\';\n",idx, entry.enabled,IfaceName,entry.tagged,0,entry.priority,entry.cfi,0,entry.vlanId,wanLan);
#endif
	}
	else
	{
#if defined(CONFIG_RTK_VLAN_NEW_FEATURE) ||defined(CONFIG_RTL_HW_VLAN_SUPPORT)
		sprintf(bufStr, "token[%d] =\'0|none|0|0|0|0|0|0|LAN|0\';\n", idx);
#else
		sprintf(bufStr, "token[%d] =\'0|none|0|0|0|0|0|0|LAN\';\n", idx);
#endif
	}
	req_format_write(wp, bufStr);
	return 0;
}

int getVlanList(request *wp, int argc, char **argv)
{
	int i, maxWebVlanNum;

#if defined(CONFIG_RTL_8198_AP_ROOT) && defined(GMII_ENABLED)
	maxWebVlanNum = MAX_IFACE_VLAN_CONFIG-2;
#else
	maxWebVlanNum = MAX_IFACE_VLAN_CONFIG-1;
#endif
	for (i=0; i<=maxWebVlanNum; i++) {
		vlanList(wp, i);
	}
	return 0;
}

void formVlan(request *wp, char *path, char *query)
{
	VLAN_CONFIG_T entry;
	char *submitUrl,*strTmp;
	int	i, vlan_onoff;
	struct nameMapping *mapping;
	char tmpBuf[100];

	//displayPostDate(wp->post_data);
	//printf("--%s(%d)--\n", __FUNCTION__, __LINE__);

	strTmp= req_get_cstream_var(wp, ("vlan_onoff"), "");
	if(strTmp[0])
	{
		vlan_onoff = atoi(strTmp);
	}

	if (!apmib_set(MIB_VLANCONFIG_ENABLED, (void *)&vlan_onoff))
	{
		strcpy(tmpBuf, ("set  MIB_VLANCONFIG_ENABLED error!"));
	//	printf("--%s(%d)--\n", __FUNCTION__, __LINE__);
		goto setErr;
	}
	if(vlan_onoff == 1)
	{
		if ( !apmib_set(MIB_VLANCONFIG_DELALL, (void *)&entry))
		{
			strcpy(tmpBuf, ("Delete all table error!"));
		//	printf("--%s(%d)--\n", __FUNCTION__, __LINE__);
			goto setErr;
		}

		for(i=1; i<=MAX_IFACE_VLAN_CONFIG ; i++)
		{
			memset(&entry, '\0', sizeof(entry));
		//	printf("--%s(%d)--i is %d\n", __FUNCTION__, __LINE__, i);

			*((char *)&entry) = (char)i;
			apmib_get(MIB_VLANCONFIG_TBL, (void *)&entry);

			memset(tmpBuf,0x00, sizeof(tmpBuf));
			sprintf(tmpBuf,"vlan_iface_%d",i);
			strTmp = req_get_cstream_var(wp, tmpBuf, "");

			if(strTmp[0])
			{
				//strcpy(entry.netIface,strTmp);

				mapping = findNameMapping(strTmp);

				if(mapping)
				{
					strcpy((char *)entry.netIface,mapping->ifname);
				}
			}
			else
			{
		//	printf("--%s(%d)--\n", __FUNCTION__, __LINE__);
				if ( apmib_set(MIB_VLANCONFIG_ADD, (void *)&entry) == 0)
				{
					strcpy(tmpBuf, ("Add table entry error!"));
	//				printf("--%s(%d)--\n", __FUNCTION__, __LINE__);
					goto setErr;
				}
	//			printf("--%s(%d)--\n", __FUNCTION__, __LINE__);
				continue;
			}

			memset(tmpBuf,0x00, sizeof(tmpBuf));
			sprintf(tmpBuf,"vlan_enable_%d",i);
			strTmp = req_get_cstream_var(wp, tmpBuf, "");
			if(strTmp[0])
			{
				entry.enabled = atoi(strTmp);
			}

			memset(tmpBuf,0x00, sizeof(tmpBuf));
			sprintf(tmpBuf,"vlan_tag_%d",i);
			strTmp = req_get_cstream_var(wp, tmpBuf, "");
			if(strTmp[0])
			{
				entry.tagged = atoi(strTmp);
			}

			memset(tmpBuf,0x00, sizeof(tmpBuf));
			sprintf(tmpBuf,"vlan_cfg_%d",i);
			strTmp = req_get_cstream_var(wp, tmpBuf, "");
			if(strTmp[0])
			{
				entry.cfi = atoi(strTmp);
			}

			memset(tmpBuf,0x00, sizeof(tmpBuf));
			sprintf(tmpBuf,"vlan_id_%d",i);
			strTmp = req_get_cstream_var(wp, tmpBuf, "");
			if(strTmp[0])
			{
				entry.vlanId = atoi(strTmp);
			}

			memset(tmpBuf,0x00, sizeof(tmpBuf));
			sprintf(tmpBuf,"vlan_priority_%d",i);
			strTmp = req_get_cstream_var(wp, tmpBuf, "");
			if(strTmp[0])
			{
				entry.priority = atoi(strTmp);
			}
#if defined(CONFIG_RTK_VLAN_NEW_FEATURE) || defined(CONFIG_RTL_HW_VLAN_SUPPORT)

			memset(tmpBuf,0x00, sizeof(tmpBuf));
			sprintf(tmpBuf,"vlan_forward_%d",i);
			strTmp = req_get_cstream_var(wp, tmpBuf, "");
			if(strTmp[0])
			{
				entry.forwarding_rule = atoi(strTmp);
			}
#endif

			if ( apmib_set(MIB_VLANCONFIG_ADD, (void *)&entry) == 0)
			{
				strcpy(tmpBuf, ("Add table entry error!"));
//				printf("--%s(%d)--\n", __FUNCTION__, __LINE__);
				goto setErr;
			}




		}

	}

	apmib_update_web(CURRENT_SETTING);

#ifndef NO_ACTION
	run_init_script("all");
#endif

	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page
	if (submitUrl[0])
	{
		OK_MSG(submitUrl);
	}
  	return;

setErr:
	ERR_MSG(tmpBuf);
	return;

}
#endif
#if defined(CONFIG_RTK_VLAN_WAN_TAG_SUPPORT)
void formVlanWAN(request *wp, char *path, char *query)
{
	VLAN_CONFIG_T entry;
	char *submitUrl,*strTmp;
	int	value;
	struct nameMapping *mapping;
	char tmpBuf[100];
	
	value =  atoi(req_get_cstream_var(wp, ("vlan_wan_tag"), ("0")));
	if(strcmp(req_get_cstream_var(wp, ("vlan_wan_enable"), ("")), "on"))
		value = 0;

	if (!apmib_set(MIB_VLAN_WAN_TAG, (void *)&value))
	{
		strcpy(tmpBuf, ("set  MIB_VLAN_WAN_TAG error!"));
		goto setErr;
	}

	value =  atoi(req_get_cstream_var(wp, ("vlan_wan_host_pri"), ("0")));
	if(strcmp(req_get_cstream_var(wp, ("vlan_wan_enable"), ("")), "on"))
		value = 0;
	if (!apmib_set(MIB_VLAN_WAN_HOST_PRI, (void *)&value))
	{
		strcpy(tmpBuf, ("set  MIB_VLAN_WAN_HOST_PRI error!"));
		goto setErr;
	}

	value =  atoi(req_get_cstream_var(wp, ("vlan_wan_bridge_tag"), ("0")));
	if(strcmp(req_get_cstream_var(wp, ("vlan_wan_bridge_enable"), ("")), "on"))
		value = 0;

	if (!apmib_set(MIB_VLAN_WAN_BRIDGE_TAG, (void *)&value))
	{
		strcpy(tmpBuf, ("set  MIB_VLAN_WAN_BRIDGE_TAG error!"));
		goto setErr;
	}

	value =  atoi(req_get_cstream_var(wp, ("vlan_wan_bridge_multicast_tag"), ("0")));
	if(strcmp(req_get_cstream_var(wp, ("vlan_wan_bridge_multicast_enable"), ("")), "on"))
		value = 0;

	if (!apmib_set(MIB_VLAN_WAN_BRIDGE_MULTICAST_TAG, (void *)&value))
	{
		strcpy(tmpBuf, ("set  MIB_VLAN_WAN_BRIDGE_MULTICAST_TAG error!"));
		goto setErr;
	}
	
	value = 0;
	value |= (!strcmp(req_get_cstream_var(wp, ("vlan_wan_bridge_port_0"), ("")), "on"))<<3;
	value |= (!strcmp(req_get_cstream_var(wp, ("vlan_wan_bridge_port_1"), ("")), "on"))<<2;
	value |= (!strcmp(req_get_cstream_var(wp, ("vlan_wan_bridge_port_2"), ("")), "on"))<<1;
	value |= (!strcmp(req_get_cstream_var(wp, ("vlan_wan_bridge_port_3"), ("")), "on"))<<0;
	value |= (!strcmp(req_get_cstream_var(wp, ("vlan_wan_bridge_port_wifi_root"), ("")), "on"))<<6;
	value |= (!strcmp(req_get_cstream_var(wp, ("vlan_wan_bridge_port_wifi_vap0"), ("")), "on"))<<7;
	value |= (!strcmp(req_get_cstream_var(wp, ("vlan_wan_bridge_port_wifi_vap1"), ("")), "on"))<<8;
	value |= (!strcmp(req_get_cstream_var(wp, ("vlan_wan_bridge_port_wifi_vap2"), ("")), "on"))<<9;
	value |= (!strcmp(req_get_cstream_var(wp, ("vlan_wan_bridge_port_wifi_vap3"), ("")), "on"))<<10;
	if (!apmib_set(MIB_VLAN_WAN_BRIDGE_PORT, (void *)&value))
	{
		strcpy(tmpBuf, ("set  MIB_VLAN_WAN_BRIDGEPORT error!"));
		goto setErr;
	}


	apmib_update_web(CURRENT_SETTING);

	#ifndef NO_ACTION
		run_init_script("all");
	#endif

	OK_MSG("/vlan_wan.htm");
	return;

	setErr:
	ERR_MSG(tmpBuf);

	return;
	
}
#endif

/////////////////////////////////////////////////////////////////////////////
void formDMZ(request *wp, char *path, char *query)
{
	char *submitUrl, *strSave, *strVal;
	char tmpBuf[100];
	int intVal;
	struct in_addr ipAddr, curIpAddr, curSubnet;
	unsigned long v1, v2, v3;
#ifndef NO_ACTION
	int pid;
#endif

	strSave = req_get_cstream_var(wp, ("save"), "");

	if (strSave[0]) {
		strVal = req_get_cstream_var(wp, ("enabled"), "");
		if ( !strcmp(strVal, "ON"))
			intVal = 1;
		else
			intVal = 0;

		if ( apmib_set(MIB_DMZ_ENABLED, (void *)&intVal) == 0) {
			strcpy(tmpBuf, ("Set enabled flag error!"));
			goto setErr_dmz;
		}

		strVal = req_get_cstream_var(wp, ("ip"), "");
		if (!strVal[0]) {
			goto setOk_dmz;
		}
		inet_aton(strVal, &ipAddr);
		getInAddr(BRIDGE_IF, IP_ADDR, (void *)&curIpAddr);
		getInAddr(BRIDGE_IF, SUBNET_MASK, (void *)&curSubnet);

		v1 = *((unsigned long *)&ipAddr);
		v2 = *((unsigned long *)&curIpAddr);
		v3 = *((unsigned long *)&curSubnet);
		if (v1) {
			if ( (v1 & v3) != (v2 & v3) ) {
				strcpy(tmpBuf, ("Invalid IP address! It should be set within the current subnet."));
				goto setErr_dmz;
			}
		}
		if ( apmib_set(MIB_DMZ_HOST, (void *)&ipAddr) == 0) {
			strcpy(tmpBuf, ("Set DMZ MIB error!"));
			goto setErr_dmz;
		}
	}

setOk_dmz:
	apmib_update_web(CURRENT_SETTING);

#ifndef NO_ACTION
	pid = fork();
        if (pid) {
	      	waitpid(pid, NULL, 0);
	}
        else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _FIREWALL_SCRIPT_PROG);
		execl( tmpBuf, _FIREWALL_SCRIPT_PROG, NULL);
               	exit(1);
        }
#endif

	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page
//	OK_MSG(submitUrl);
	if (submitUrl[0])
		send_redirect_perm(wp, submitUrl);
  	return;

setErr_dmz:
	ERR_MSG(tmpBuf);
}


/////////////////////////////////////////////////////////////////////////////
int portFwList(request *wp, int argc, char **argv)
{
	int	nBytesSent=0, entryNum, i;
	PORTFW_T entry;
	char	*type, portRange[20], *ip;

	if ( !apmib_get(MIB_PORTFW_TBL_NUM, (void *)&entryNum)) {
  		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}

	nBytesSent += req_format_write(wp, ("<tr>"
      	"<td align=center width=\"25%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Local IP Address</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Protocol</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Port Range</b></font></td>\n"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Comment</b></font></td>\n"
      	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td></tr>\n"));

	for (i=1; i<=entryNum; i++) {
		*((char *)&entry) = (char)i;
		if ( !apmib_get(MIB_PORTFW_TBL, (void *)&entry))
			return -1;

		ip = inet_ntoa(*((struct in_addr *)entry.ipAddr));
		if ( !strcmp(ip, "0.0.0.0"))
			ip = "----";

		if ( entry.protoType == PROTO_BOTH )
			type = "TCP+UDP";
		else if ( entry.protoType == PROTO_TCP )
			type = "TCP";
		else
			type = "UDP";

		if ( entry.fromPort == 0)
			strcpy(portRange, "----");
		else if ( entry.fromPort == entry.toPort )
			snprintf(portRange, 20, "%d", entry.fromPort);
		else
			snprintf(portRange, 20, "%d-%d", entry.fromPort, entry.toPort);

		nBytesSent += req_format_write(wp, ("<tr>"
			"<td align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
     			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n"),
				ip, type, portRange, entry.comment, i);
	}
	return nBytesSent;
}


/////////////////////////////////////////////////////////////////////////////
int portFilterList(request *wp, int argc, char **argv)
{
	int	nBytesSent=0, entryNum, i;
	PORTFILTER_T entry;
	char	*type, portRange[20];

	if ( !apmib_get(MIB_PORTFILTER_TBL_NUM, (void *)&entryNum)) {
  		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}

	nBytesSent += req_format_write(wp, ("<tr>"
      	"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Port Range</b></font></td>\n"
      	"<td align=center width=\"25%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Protocol</b></font></td>\n"
	"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Comment</b></font></td>\n"
      	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td></tr>\n"));

	for (i=1; i<=entryNum; i++) {
		*((char *)&entry) = (char)i;
		if ( !apmib_get(MIB_PORTFILTER_TBL, (void *)&entry))
			return -1;

		if ( entry.protoType == PROTO_BOTH )
			type = "TCP+UDP";
		else if ( entry.protoType == PROTO_TCP )
			type = "TCP";
		else
			type = "UDP";

		if ( entry.fromPort == 0)
			strcpy(portRange, "----");
		else if ( entry.fromPort == entry.toPort )
			snprintf(portRange, 20, "%d", entry.fromPort);
		else
			snprintf(portRange, 20, "%d-%d", entry.fromPort, entry.toPort);

		nBytesSent += req_format_write(wp, ("<tr>"
			"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
   			"<td align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
     			"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n"),
				portRange, type, entry.comment, i);
	}
	return nBytesSent;
}


/////////////////////////////////////////////////////////////////////////////
int ipFilterList(request *wp, int argc, char **argv)
{
	int	nBytesSent=0, entryNum, i;
	IPFILTER_T entry;
	char	*type, *ip;

	if ( !apmib_get(MIB_IPFILTER_TBL_NUM, (void *)&entryNum)) {
  		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}

	nBytesSent += req_format_write(wp, ("<tr>"
      	"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Local IP Address</b></font></td>\n"
      	"<td align=center width=\"25%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Protocol</b></font></td>\n"
      	"<td align=center width=\"25%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Comment</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td></tr>\n"));

	for (i=1; i<=entryNum; i++) {
		*((char *)&entry) = (char)i;
		if ( !apmib_get(MIB_IPFILTER_TBL, (void *)&entry))
			return -1;

		ip = inet_ntoa(*((struct in_addr *)entry.ipAddr));
		if ( !strcmp(ip, "0.0.0.0"))
			ip = "----";

		if ( entry.protoType == PROTO_BOTH )
			type = "TCP+UDP";
		else if ( entry.protoType == PROTO_TCP )
			type = "TCP";
		else
			type = "UDP";

		nBytesSent += req_format_write(wp, ("<tr>"
			"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n"),
				ip, type, entry.comment, i);
	}
	return nBytesSent;
}


/////////////////////////////////////////////////////////////////////////////
int macFilterList(request *wp, int argc, char **argv)
{
	int nBytesSent=0, entryNum, i;
	MACFILTER_T entry;
	char tmpBuf[100];

	if ( !apmib_get(MIB_MACFILTER_TBL_NUM, (void *)&entryNum)) {
  		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}

	nBytesSent += req_format_write(wp, ("<tr>"
      	"<td align=center width=\"50%%\" bgcolor=\"#808080\"><font size=\"2\"><b>MAC Address</b></font></td>\n"
      	"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Comment</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td></tr>\n"));

	for (i=1; i<=entryNum; i++) {
		*((char *)&entry) = (char)i;
		if ( !apmib_get(MIB_MACFILTER_TBL, (void *)&entry))
			return -1;

		snprintf(tmpBuf, 100, ("%02x:%02x:%02x:%02x:%02x:%02x"),
			entry.macAddr[0], entry.macAddr[1], entry.macAddr[2],
			entry.macAddr[3], entry.macAddr[4], entry.macAddr[5]);

		nBytesSent += req_format_write(wp, ("<tr>"
			"<td align=center width=\"50%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
       			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n"),
				tmpBuf, entry.comment, i);
	}
	return nBytesSent;
}

/////////////////////////////////////////////////////////////////////////////
int urlFilterList(request *wp, int argc, char **argv)
{
	int nBytesSent=0, entryNum, i;
	URLFILTER_T entry;
	int mode;
	if ( !apmib_get(MIB_URLFILTER_TBL_NUM, (void *)&entryNum)) {
  		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}

	if ( !apmib_get(MIB_URLFILTER_MODE, (void *)&mode)) {
  		fprintf(stderr, "Get URL Filter mode error!\n");
		return -1;
	}
	
	nBytesSent += req_format_write(wp, ("<tr>"
      	"<td align=center width=\"70%%\" bgcolor=\"#808080\"><font size=\"2\"><b>URL Address</b></font></td>\n"
      	"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td></tr>\n"));

	for (i=1; i<=entryNum; i++) {
		*((char *)&entry) = (char)i;
		if ( !apmib_get(MIB_URLFILTER_TBL, (void *)&entry))
			return -1;
		if(mode!=entry.ruleMode)
			continue;
		nBytesSent += req_format_write(wp, ("<tr>"
			"<td align=center width=\"70%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			//"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
       			"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n"),
			entry.urlAddr, i); //tmpBuf
			//entry.urlAddr, entry.comment, i); //tmpBuf
	}
	return nBytesSent;

}

#if 0
/////////////////////////////////////////////////////////////////////////////
int triggerPortList(request *wp, int argc, char **argv)
{

	int	nBytesSent=0, entryNum, i;
	TRIGGERPORT_T entry;
	char	*triType, triPortRange[20], *incType, incPortRange[20];

	if ( !apmib_get(MIB_TRIGGERPORT_TBL_NUM, (void *)&entryNum)) {
  		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}

	nBytesSent += req_format_write(wp, ("<tr>"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Trigger-port Range</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Trigger-port Protocol</b></font></td>\n"
     	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Incoming-port Range</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Incoming-port Protocol</b></font></td>\n"
	"<td align=center width=\"14%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Comment</b></font></td>\n"
      	"<td align=center width=\"6%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td></tr>\n"));


#if 0
	nBytesSent += req_format_write(wp, ("<tr>"
	"<td align=center width=\"25%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Trigger-port Range</b></font></td>\n"
      	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Trigger-port Protocol</b></font></td>\n")
	"<td align=center width=\"25%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Incoming-port Range</b></font></td>\n"
      	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Incoming-port Protocol</b></font></td>\n"
	"<td align=center width=\"14%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Comment</b></font></td>\n"
      	"<td align=center width=\"6%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td></tr>\n"));

#endif
	for (i=1; i<=entryNum; i++) {
		*((char *)&entry) = (char)i;
		if ( !apmib_get(MIB_TRIGGERPORT_TBL, (void *)&entry))
			return -1;

		if ( entry.tri_protoType == PROTO_BOTH )
			triType = "TCP+UDP";
		else if ( entry.tri_protoType == PROTO_TCP )
			triType = "TCP";
		else
			triType = "UDP";

		if ( entry.tri_fromPort == 0)
			strcpy(triPortRange, "----");
		else if ( entry.tri_fromPort == entry.tri_toPort )
			snprintf(triPortRange, 20, "%d", entry.tri_fromPort);
		else
			snprintf(triPortRange, 20, "%d-%d", entry.tri_fromPort, entry.tri_toPort);

		if ( entry.inc_protoType == PROTO_BOTH )
			incType = "TCP+UDP";
		else if ( entry.inc_protoType == PROTO_TCP )
			incType = "TCP";
		else
			incType = "UDP";

		if ( entry.inc_fromPort == 0)
			strcpy(incPortRange, "----");
		else if ( entry.inc_fromPort == entry.inc_toPort )
			snprintf(incPortRange, 20, "%d", entry.inc_fromPort);
		else
			snprintf(incPortRange, 20, "%d-%d", entry.inc_fromPort, entry.inc_toPort);


		nBytesSent += req_format_write(wp, ("<tr>"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
   			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
   			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
     			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"6%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n"),
				triPortRange, triType, incPortRange, incType, entry.comment, i);
	}
	return nBytesSent;
}
#endif

#ifdef GW_QOS_ENGINE
/////////////////////////////////////////////////////////////////////////////
int qosList(request *wp, int argc, char **argv)
{
	int	entryNum;
	QOS_T entry;
	char buffer[120];
	char tmpBuf[80];
	int index;

	if ( !apmib_get(MIB_QOS_RULE_TBL_NUM, (void *)&entryNum)) {
		goto ret_empty;
	}
	index= atoi(argv[0]); // index shoud be 0 ~ 9
	index += 1;

	if( index <= entryNum)
	{
		*((char *)&entry) = (char)index;
		if ( !apmib_get(MIB_QOS_RULE_TBL, (void *)&entry))
		{
			goto ret_empty;
		}

              strcpy(tmpBuf, inet_ntoa(*((struct in_addr*)entry.local_ip_start)));
              strcpy(&tmpBuf[20], inet_ntoa(*((struct in_addr*)entry.local_ip_end)));
              strcpy(&tmpBuf[40], inet_ntoa(*((struct in_addr*)entry.remote_ip_start)));
              strcpy(&tmpBuf[60], inet_ntoa(*((struct in_addr*)entry.remote_ip_end)));
		 sprintf(buffer, "%d-%d-%d-%s-%s-%d-%d-%s-%s-%d-%d-%s", entry.enabled, entry.priority, entry.protocol,
                        tmpBuf, &tmpBuf[20],entry.local_port_start, entry.local_port_end,
                        &tmpBuf[40], &tmpBuf[60], entry.remote_port_start, entry.remote_port_end, entry.entry_name );

		req_format_write(wp, ("%s"), buffer);
	      return 0;
	}

ret_empty:
	req_format_write(wp, ("%s"), "");
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
#define _PROTOCOL_TCP   6
#define _PROTOCOL_UDP   17
#define _PROTOCOL_BOTH   257
#define _PORT_MIN       0
#define _PORT_MAX       65535

static QOS_T entry_for_save[MAX_QOS_RULE_NUM];

void formQoS(request *wp, char *path, char *query)
{
#ifndef NO_ACTION
    int pid;
#endif

    char *submitUrl;
    char tmpBuf[100];

    char *strIp, *endIp, *tmpStr, *strEnabled;
    char varName[48];
    int index=1, protocol_others;
    int intVal, valid_num;
    QOS_T entry;
    struct in_addr curIpAddr, curSubnet;
    unsigned long v1, v2, v3, v4;

    strEnabled = req_get_cstream_var(wp, ("config.qos_enabled"), "");
    if( !strcmp(strEnabled, "true"))
    {
        intVal=1;
    }
    else
        intVal=0;
    if ( apmib_set( MIB_QOS_ENABLED, (void *)&intVal) == 0) {
        strcpy(tmpBuf, ("Set QoS enabled flag error!"));
        goto setErr_qos;
    }
    if (intVal==0)
         goto setOk_qos;
    strEnabled = req_get_cstream_var(wp, ("config.qos_auto_trans_rate"), "");
    if( !strcmp(strEnabled, "true"))
        intVal=1;
    else
        intVal=0;
    if ( apmib_set( MIB_QOS_AUTO_UPLINK_SPEED, (void *)&intVal) == 0) {
        strcpy(tmpBuf, ("Set QoS error!"));
        goto setErr_qos;
    }

    if( intVal == 0)
    {
        tmpStr = req_get_cstream_var(wp, ("config.qos_max_trans_rate"), "");
          string_to_dec(tmpStr, &intVal);
        if ( apmib_set(MIB_QOS_MANUAL_UPLINK_SPEED, (void *)&intVal) == 0) {
            strcpy(tmpBuf, ("Set QoS error!"));
            goto setErr_qos;
        }
    }


/*    if ( !apmib_set(MIB_QOS_DELALL, (void *)&entry)) {
        strcpy(tmpBuf, ("Delete all table error!"));
        goto setErr_qos;
    } */

    for(index=0, valid_num=0; index<MAX_QOS_RULE_NUM; index++)
    {
        sprintf(varName, "config.qos_rules[%d].enabled", index);
        tmpStr = req_get_cstream_var(wp, varName, "");
        if( !strcmp(tmpStr, "true"))
            intVal=1;
        else
            intVal=0;
        entry.enabled = (unsigned char)intVal;

        sprintf(varName, "config.qos_rules[%d].entry_name", index);
        tmpStr = req_get_cstream_var(wp, varName, "");
        strcpy(entry.entry_name, tmpStr);

        if (intVal == 0 && tmpStr[0] == 0)
             continue;

        sprintf(varName, "config.qos_rules[%d].priority", index);
        tmpStr = req_get_cstream_var(wp, varName, "");
        string_to_dec(tmpStr, &intVal);
        entry.priority = (unsigned char)intVal;

        sprintf(varName, "config.qos_rules[%d].protocol_menu", index);
        tmpStr = req_get_cstream_var(wp, varName, "");
        if (!strcmp(tmpStr, "-1"))
            protocol_others = 1;
        else
            protocol_others = 0;

        sprintf(varName, "config.qos_rules[%d].protocol", index);
        tmpStr = req_get_cstream_var(wp, varName, "");
        string_to_dec(tmpStr, &intVal);
        entry.protocol = (unsigned short)intVal;

        sprintf(varName, "config.qos_rules[%d].local_ip_start", index);
        strIp = req_get_cstream_var(wp, varName, "");
        inet_aton(strIp, (struct in_addr *)&entry.local_ip_start);
        sprintf(varName, "config.qos_rules[%d].local_ip_end", index);
        endIp = req_get_cstream_var(wp, varName, "");
        inet_aton(endIp, (struct in_addr *)&entry.local_ip_end);
        getInAddr(BRIDGE_IF, IP_ADDR, (void *)&curIpAddr);
        getInAddr(BRIDGE_IF, SUBNET_MASK, (void *)&curSubnet);

        v1 = *((unsigned long *)entry.local_ip_start);
        v2 = *((unsigned long *)&curIpAddr);
        v3 = *((unsigned long *)&curSubnet);
        if ( (v1 & v3) != (v2 & v3) ) {
            sprintf(tmpBuf, "\'%s\': Local IP start \'%s\' is not in the LAN subnet",
                        entry.entry_name, strIp);
            goto setErr_qos;
        }
        v4 = *((unsigned long *)entry.local_ip_end);
        if ( (v4 & v3) != (v2 & v3) ) {
            sprintf(tmpBuf, "\'%s\': Local IP end \'%s\' is not in the LAN subnet",
                        entry.entry_name, endIp);
            goto setErr_qos;
        }
        if ( v1 > v4 ) {
            sprintf(tmpBuf, "\'%s\': Local IP start, \'%s\', must be less than or equal to local IP end, \'%s\'",
                        entry.entry_name, strIp, endIp);
            goto setErr_qos;
        }


        sprintf(varName, "config.qos_rules[%d].remote_ip_start", index);
        strIp = req_get_cstream_var(wp, varName, "");
        inet_aton(strIp, (struct in_addr *)&entry.remote_ip_start);
        sprintf(varName, "config.qos_rules[%d].remote_ip_end", index);
        endIp = req_get_cstream_var(wp, varName, "");
        inet_aton(endIp, (struct in_addr *)&entry.remote_ip_end);
        v1 = *((unsigned long *)entry.remote_ip_start);
        v4 = *((unsigned long *)entry.remote_ip_end);
        if ( (v1 & v3) == (v2 & v3) ) {
            sprintf(tmpBuf, "\'%s\': Remote IP start \'%s\' is in the LAN subnet",
                        entry.entry_name, strIp);
            goto setErr_qos;
        }
        if ( (v4 & v3) == (v2 & v3) ) {
            sprintf(tmpBuf, "\'%s\': Remote IP end \'%s\' is in the LAN subnet",
                        entry.entry_name, endIp);
            goto setErr_qos;
        }
        if ( v1 > v4 ) {
            sprintf(tmpBuf, "\'%s\': Remote IP start, \'%s\', must be less than or equal to remote IP end, \'%s\'",
                        entry.entry_name, strIp, endIp);
            goto setErr_qos;
        }

/*        if ((!protocol_others) &&
            ( entry.protocol  == _PROTOCOL_TCP || entry.protocol  == _PROTOCOL_UDP ||entry.protocol  == _PROTOCOL_BOTH)) */
        {
            sprintf(varName, "config.qos_rules[%d].local_port_start", index);
            tmpStr = req_get_cstream_var(wp, varName, "");
            string_to_dec(tmpStr, &intVal);
            entry.local_port_start = (unsigned short)intVal;
            sprintf(varName, "config.qos_rules[%d].local_port_end", index);
            tmpStr = req_get_cstream_var(wp, varName, "");
            string_to_dec(tmpStr, &intVal);
            entry.local_port_end = (unsigned short)intVal;

            sprintf(varName, "config.qos_rules[%d].remote_port_start", index);
            tmpStr = req_get_cstream_var(wp, varName, "");
            string_to_dec(tmpStr, &intVal);
            entry.remote_port_start = (unsigned short)intVal;
            sprintf(varName, "config.qos_rules[%d].remote_port_end", index);
            tmpStr = req_get_cstream_var(wp, varName, "");
            string_to_dec(tmpStr, &intVal);
            entry.remote_port_end = (unsigned short)intVal;

        }

/*        *((char *)&entry_existed) = (char)index;
        if ( !apmib_get(MIB_QOS_RULE_TBL, (void *)&entry_existed)) {
		strcpy(tmpBuf, ("Get table entry error!"));
		goto setErr_qos;
        }
        if ( !apmib_set(MIB_QOS_DEL, (void *)&entry_existed)) {
		strcpy(tmpBuf, ("Delete table entry error!"));
		goto setErr_qos;
        } */

/*        if ( apmib_set(MIB_QOS_ADD, (void *)&entry) == 0) {
            strcpy(tmpBuf, ("Add table entry error!"));
            goto setErr_qos;
        } */
        memcpy(&entry_for_save[valid_num], &entry, sizeof(QOS_T));
        valid_num++;

    }


    if ( !apmib_set(MIB_QOS_DELALL, (void *)&entry)) {
        strcpy(tmpBuf, ("Delete all table error!"));
        goto setErr_qos;
    }

    for(index=0; index<valid_num; index++)
    {
        if ( apmib_set(MIB_QOS_ADD, (void *)&entry_for_save[index]) == 0) {
            strcpy(tmpBuf, ("Add table entry error!"));
            goto setErr_qos;
        }
    }

setOk_qos:
    apmib_update_web(CURRENT_SETTING);

#ifndef NO_ACTION
    pid = fork();
    if (pid) {
        waitpid(pid, NULL, 0);
    }
    else if (pid == 0) {
        snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _QOS_SCRIPT_PROG);
        execl( tmpBuf, _QOS_SCRIPT_PROG, NULL);
        exit(1);
    }
#endif

    submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page
    if (submitUrl[0])
        send_redirect_perm(wp, submitUrl);
    return;

setErr_qos:
    ERR_MSG(tmpBuf);
}
#endif

#ifdef QOS_BY_BANDWIDTH
static const char _md1[] = "Guaranteed minimum bandwidth", _md2[] = "Restricted maximum bandwidth";
static const char s4dashes[] = "----";

#define QOS_BW_CHECK_FAIL				-1
#define QOS_BW_NOT_OVERSIZE			0
#define QOS_UPLINK_BW_OVERSIZE		0x1
#define QOS_DOWNLINK_BW_OVERSIZE		0x2
#define QOS_BOTHLINK_BW_OVERSIZE		0x3

// Only for "Guaranteed minimum bandwidth",
// to check current uplink or downlink bandwidth added uplink & downlink bandwidth at previous rules
// whether larger than totoal uplink or downlink bandwidth
int checkQosRuleBw(unsigned long curUplinkBw, unsigned long curDownlinkBw, unsigned long totalUplinkBw, unsigned long totalDownlinkBw)
{
	int	entryNum, i, ret;
	IPQOS_T entry;
	unsigned long tmpTotolUplinkBw, tmpTotalDownlinkBw;

	if ( !apmib_get(MIB_QOS_RULE_TBL_NUM, (void *)&entryNum)) {
		return QOS_BW_CHECK_FAIL;
	}

	tmpTotolUplinkBw=curUplinkBw;
	tmpTotalDownlinkBw=curDownlinkBw;
	for (i=1; i<=entryNum; i++) {
		*((char *)&entry) = (char)i;
		if ( !apmib_get(MIB_QOS_RULE_TBL, (void *)&entry))
			return QOS_BW_CHECK_FAIL;

		if ( (entry.mode & QOS_RESTRICT_MIN)  != 0){
			//Do check for "Guaranteed minimum bandwidth"
			tmpTotolUplinkBw += entry.bandwidth;
			tmpTotalDownlinkBw += entry.bandwidth_downlink;
		}
	}

	ret=QOS_BW_NOT_OVERSIZE;
	if(tmpTotolUplinkBw > totalUplinkBw)
		ret += QOS_UPLINK_BW_OVERSIZE;

	if(tmpTotalDownlinkBw > totalDownlinkBw)
		ret += QOS_DOWNLINK_BW_OVERSIZE;

	return ret;
}

/////////////////////////////////////////////////////////////////////////////
int ipQosList(request *wp, int argc, char **argv)
{
	int	nBytesSent=0, entryNum, i;
	IPQOS_T entry;
	char	*mode, bandwidth[10], bandwidth_downlink[10];
	char	mac[20], ip[40], *tmpStr;

	if ( !apmib_get(MIB_QOS_RULE_TBL_NUM, (void *)&entryNum)) {
  		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}

	nBytesSent += req_format_write(wp, ("<tr>"
      	"<td align=center width=\"\" bgcolor=\"#808080\"><font size=\"2\"><b>Local IP Address</b></font></td>\n"
      	"<td align=center width=\"\" bgcolor=\"#808080\"><font size=\"2\"><b>MAC Address</b></font></td>\n"
#if defined(CONFIG_NETFILTER_XT_MATCH_LAYER7)
				"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Layer 7 Rule</b></font></td>\n"
#endif
      	"<td align=center width=\"\" bgcolor=\"#808080\"><font size=\"2\"><b>Mode</b></font></td>\n"
      	"<td align=center width=\"\" bgcolor=\"#808080\"><font size=\"2\"><b>Uplink Bandwidth</b></font></td>\n"
      	"<td align=center width=\"\" bgcolor=\"#808080\"><font size=\"2\"><b>Downlink Bandwidth</b></font></td>\n"
	"<td align=center width=\"\" bgcolor=\"#808080\"><font size=\"2\"><b>Comment</b></font></td>\n"
      	"<td align=center width=\"\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td></tr>\n"));

	for (i=1; i<=entryNum; i++) {
		*((char *)&entry) = (char)i;
		if ( !apmib_get(MIB_QOS_RULE_TBL, (void *)&entry))
			return -1;

		if ( (entry.mode & QOS_RESTRICT_IP)  != 0) {
			tmpStr = inet_ntoa(*((struct in_addr *)entry.local_ip_start));
			strcpy(mac, tmpStr);
			tmpStr = inet_ntoa(*((struct in_addr *)entry.local_ip_end));
			sprintf(ip, "%s - %s", mac, tmpStr);

			strcpy(mac, s4dashes);
		}
		else if ( (entry.mode & QOS_RESTRICT_MAC)  != 0) {
			sprintf(mac, "%02x%02x%02x%02x%02x%02x",
				entry.mac[0],entry.mac[1],entry.mac[2],entry.mac[3],entry.mac[4],entry.mac[5]);
			strcpy(ip, s4dashes);
		}
		else //all
		{
			strcpy(ip, s4dashes);
			strcpy(mac, s4dashes);
		}

		if ( (entry.mode & QOS_RESTRICT_MIN)  != 0)
			mode = (char *)_md1;
		else
			mode = (char *)_md2;

    if(entry.bandwidth == 0)
    	sprintf(bandwidth, "%s", "-");
		else
			snprintf(bandwidth, 10, "%ld", entry.bandwidth);

		if(entry.bandwidth_downlink == 0)
    	sprintf(bandwidth_downlink, "%s", "-");
		else
			snprintf(bandwidth_downlink, 10, "%ld", entry.bandwidth_downlink);

		nBytesSent += req_format_write(wp, ("<tr>"
			"<td align=center width=\"\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
#if defined(CONFIG_NETFILTER_XT_MATCH_LAYER7)
      			"<td align=center width=\"\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
#endif      			
      			"<td align=center width=\"\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
     			"<td align=center width=\"\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n"),
				ip, mac, 
#if defined(CONFIG_NETFILTER_XT_MATCH_LAYER7)      	
				entry.l7_protocol,
#endif				
				mode, bandwidth, bandwidth_downlink, entry.entry_name, i);
	}
	return nBytesSent;
}

int l7QosList(request *wp, int argc, char **argv)
{
	int	nBytesSent=0, entryNum, i;
	IPQOS_T entry;
	char	*mode, bandwidth[10], bandwidth_downlink[10];
	char	mac[20], ip[40], *tmpStr;

	nBytesSent += req_format_write(wp, ("<option value=\"Disable\">Disable</option>"));

#if defined(CONFIG_NETFILTER_XT_MATCH_LAYER7)
	if(0)
	{
		nBytesSent += req_format_write(wp, ("<option value=\"http\">http</option>"
		"<option value=\"bittorrent\">bittorrent</option>"
		"<option value=\"msnmessenger\">msnmessenger</option>"
		"<option value=\"doom3\">doom3</option>"
		));
	}
	else
	{
	
		#define READ_BUF_SIZE 512
		DIR *dir;
		struct dirent *next;
		
		pid_t   *pidList;
		int i=0,n=0,j=0;
		
		dir = opendir("/etc/l7-protocols/protocols");
		if (!dir)
		{
		        printf("find_pid_by_name: Cannot open /proc");
		        exit(1);
		}
		pidList = malloc(sizeof(*pidList)*5);
		while ((next = readdir(dir)) != NULL) {
			FILE *status;
		  char filename[READ_BUF_SIZE];
		  char buffer[READ_BUF_SIZE];
		  char name[READ_BUF_SIZE];
		  
		  char *lineptr = NULL;
		  char *str;
		  
		  /* Must skip ".." since that is outside /proc */
		  if (strcmp(next->d_name, "..") == 0)
		  	continue;
		  	
		  if (strstr(next->d_name, ".pat") == NULL)
		  	continue;
		
			lineptr = next->d_name;
			
			str = strsep(&lineptr,".");
			
			nBytesSent += req_format_write(wp, ("<option value=\"%s\">%s</option>"),str,str);
		
		}
	}
	
#endif //#if defined(CONFIG_NETFILTER_XT_MATCH_LAYER7)
	
	return nBytesSent;
}

/////////////////////////////////////////////////////////////////////////////
void formIpQoS(request *wp, char *path, char *query)
{
	char *submitUrl, *strAdd, *strDel, *strVal, *strDelAll;
	char *strIpStart, *strIpEnd, *strMac, *strBandwidth, *strBandwidth_downlink, *strComment, *strL7Protocol;
	char tmpBuf[100];
	int entryNum, intVal, i;
	IPQOS_T entry;
#ifndef NO_ACTION
	int pid;
#endif
	unsigned long totalUplinkBw, totalDownlinkBw;
	int ret;

//displayPostDate(wp->post_data);

	strAdd = req_get_cstream_var(wp, ("addQos"), "");
	strDel = req_get_cstream_var(wp, ("deleteSel"), "");
	strDelAll = req_get_cstream_var(wp, ("deleteAll"), "");

	memset(&entry, '\0', sizeof(entry));

	if (strAdd[0]) {
		strVal = req_get_cstream_var(wp, ("enabled"), "");
		if ( !strcmp(strVal, "ON"))
			intVal = 1;
		else
			intVal = 0;
		if ( apmib_set( MIB_QOS_ENABLED, (void *)&intVal) == 0) {
			strcpy(tmpBuf, ("Set enabled flag error!"));
			goto setErr;
		}

		if (intVal == 0)
			goto setOk;

		strVal = req_get_cstream_var(wp, ("automaticUplinkSpeed"), "");
		if ( !strcmp(strVal, "ON"))
			intVal = 1;
		else
			intVal = 0;
		if ( apmib_set( MIB_QOS_AUTO_UPLINK_SPEED, (void *)&intVal) == 0) {
			strcpy(tmpBuf, ("Set mib error!"));
			goto setErr;
		}

		if (intVal == 0) {
			strVal = req_get_cstream_var(wp, ("manualUplinkSpeed"), "");
			string_to_dec(strVal, &intVal);
			if ( apmib_set( MIB_QOS_MANUAL_UPLINK_SPEED, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set mib error!"));
				goto setErr;
			}
			totalUplinkBw=intVal;
		}
		else{
			// Auto uplink speed
#ifdef CONFIG_RTL_8198
			totalUplinkBw=1024000;		// 1000Mbps
#else
			totalUplinkBw=102400;		// 100Mbps
#endif
		}

		strVal = req_get_cstream_var(wp, ("automaticDownlinkSpeed"), "");
		if ( !strcmp(strVal, "ON"))
			intVal = 1;
		else
			intVal = 0;

		if ( apmib_set( MIB_QOS_AUTO_DOWNLINK_SPEED, (void *)&intVal) == 0) {
			strcpy(tmpBuf, ("Set mib error!"));
			goto setErr;
		}

		if (intVal == 0) {
			strVal = req_get_cstream_var(wp, ("manualDownlinkSpeed"), "");
			string_to_dec(strVal, &intVal);
			if ( apmib_set( MIB_QOS_MANUAL_DOWNLINK_SPEED, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set mib error!"));
				goto setErr;
			}
			totalDownlinkBw=intVal;
		}
		else{
			// Auto uplink speed
#ifdef CONFIG_RTL_8198
			totalDownlinkBw=1024000;		// 1000Mbps
#else
			totalDownlinkBw=102400;		// 100Mbps
#endif
		}

		strIpStart = req_get_cstream_var(wp, ("ipStart"), "");
		strIpEnd = req_get_cstream_var(wp, ("ipEnd"), "");
		strMac = req_get_cstream_var(wp, ("mac"), "");
		strBandwidth = req_get_cstream_var(wp, ("bandwidth"), "");
		strBandwidth_downlink = req_get_cstream_var(wp, ("bandwidth_downlink"), "");
		strComment = req_get_cstream_var(wp, ("comment"), "");
		strL7Protocol = req_get_cstream_var(wp, ("l7_protocol"), "");

		if (!strIpStart[0] && !strIpEnd[0] && !strMac[0] && !strBandwidth[0] && !strBandwidth_downlink[0] && !strComment[0])
			goto setOk;

		if ( strL7Protocol[0] ) {
			strcpy((char *)entry.l7_protocol, strL7Protocol);
		}

		strVal = req_get_cstream_var(wp, ("addressType"), "");
		string_to_dec(strVal, &intVal);
		if (intVal == 0) { // IP
			inet_aton(strIpStart, (struct in_addr *)&entry.local_ip_start);
			inet_aton(strIpEnd, (struct in_addr *)&entry.local_ip_end);
			entry.mode |= QOS_RESTRICT_IP;
		}
		else if (intVal == 1) { //MAC
			if (!string_to_hex(strMac, entry.mac, 12))
			{
				strcpy(tmpBuf, ("MAC input fail!"));
				goto setErr;
			}
			entry.mode |= QOS_RESTRICT_MAC;
		}
		else
		{
			entry.mode |= QOS_RESTRICT_ALL;
		}

		strVal = req_get_cstream_var(wp, ("mode"), "");
		if (strVal[0] == '1')
			entry.mode |= QOS_RESTRICT_MIN;
		else
			entry.mode |= QOS_RESTRICT_MAX;

		string_to_dec(strBandwidth, &intVal);
		entry.bandwidth = (unsigned long)intVal;

		string_to_dec(strBandwidth_downlink, &intVal);
		entry.bandwidth_downlink = (unsigned long)intVal;

		//To check uplink & downlink guaranteed minimum bandwidth
		if(entry.mode &  QOS_RESTRICT_MIN){
			ret=checkQosRuleBw(entry.bandwidth, entry.bandwidth_downlink, totalUplinkBw, totalDownlinkBw);
			if(ret==QOS_BW_CHECK_FAIL){
				strcpy(tmpBuf, ("checkQosRuleBw fail!"));
				goto setErr;
			}
			else if(ret==QOS_BOTHLINK_BW_OVERSIZE){
				strcpy(tmpBuf, ("Error: for guaranteed minimum bandwidth of both uplink and downlink, the sum bandwidth of all qos rules are larger than the total bandwidth!"));
				goto setErr;
			}
			else if(ret==QOS_DOWNLINK_BW_OVERSIZE){
				strcpy(tmpBuf, ("Error: for guaranteed minimum bandwidth of downlink, the sum bandwidth of all qos rules is larger than the total downlink bandwidth!"));
				goto setErr;
			}
			else if(ret==QOS_UPLINK_BW_OVERSIZE){
				strcpy(tmpBuf, ("Error: for guaranteed minimum bandwidth of uplink, the sum bandwidth of all qos rules is larger than the total uplink bandwidth!"));
				goto setErr;
			}
		}

		if ( strComment[0] ) {
			strcpy((char *)entry.entry_name, strComment);
		}
		entry.enabled = 1;
		if ( !apmib_get(MIB_QOS_RULE_TBL_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto setErr;
		}

		if ( (entryNum + 1) > MAX_QOS_RULE_NUM) {
			strcpy(tmpBuf, ("Cannot add new entry because table is full!"));
			goto setErr;
		}

		// set to MIB. try to delete it first to avoid duplicate case
		apmib_set(MIB_QOS_DEL, (void *)&entry);
		if ( apmib_set(MIB_QOS_ADD, (void *)&entry) == 0) {
			strcpy(tmpBuf, ("Add table entry error!"));
			goto setErr;
		}
	}

	/* Delete entry */
	if (strDel[0]) {
		if ( !apmib_get(MIB_QOS_RULE_TBL_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto setErr;
		}

		for (i=entryNum; i>0; i--) {
			snprintf(tmpBuf, 20, "select%d", i);

			strVal = req_get_cstream_var(wp, tmpBuf, "");
			if ( !strcmp(strVal, "ON") ) {
				*((char *)&entry) = (char)i;
				if ( !apmib_get(MIB_QOS_RULE_TBL, (void *)&entry)) {
					strcpy(tmpBuf, ("Get table entry error!"));
					goto setErr;
				}
				if ( !apmib_set(MIB_QOS_DEL, (void *)&entry)) {
					strcpy(tmpBuf, ("Delete table entry error!"));
					goto setErr;
				}
			}
		}
	}

	/* Delete all entry */
	if ( strDelAll[0]) {
		if ( !apmib_set(MIB_QOS_DELALL, (void *)&entry)) {
			strcpy(tmpBuf, ("Delete all table error!"));
			goto setErr;
		}
	}

setOk:
	apmib_update(CURRENT_SETTING);

#ifndef NO_ACTION
	pid = fork();
        if (pid) {
	      	waitpid(pid, NULL, 0);
	}
        else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _QOS_SCRIPT_PROG);
		execl( tmpBuf, _QOS_SCRIPT_PROG, NULL);
             exit(1);
        }
#endif

	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page

#ifdef REBOOT_CHECK
	if(needReboot == 1)
	{
		OK_MSG(submitUrl);
		return;
	}
#endif

	if (submitUrl[0])
		send_redirect_perm(wp, submitUrl);
  	return;

setErr:
	ERR_MSG(tmpBuf);

}
#endif
#endif // HOME_GATEWAY

