/*
 *      Web server handler routines for VPN stuffs
 *
 *      Authors: Shun-chin Yang	<sc_yang@realtek.com.tw>
 *
 *      $Id
 *
 */

/*-- System inlcude files --*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <time.h>
#include <net/route.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <net/if.h>

#ifdef VPN_SUPPORT

/*-- Local inlcude files --*/
#include "apmib.h"
#include "apform.h"
#include "utility.h"

#ifdef __i386__
#define _LITTLE_ENDIAN_
#endif


/*-- Macro declarations --*/
#ifdef _LITTLE_ENDIAN_
#define ntohdw(v) ( ((v&0xff)<<24) | (((v>>8)&0xff)<<16) | (((v>>16)&0xff)<<8) | ((v>>24)&0xff) )

#else
#define ntohdw(v) (v)
#endif

#define _PATH_IPSEC_EROUTE ("/proc/net/ipsec_eroute")
#define _PATH_IPSEC_SPI ("/proc/net/ipsec_spi")
#define _PATH_IPSEC_SPIGRP ("/proc/net/ipsec_spigrp")
#define _PATH_IPSEC_VPN_STATUS  ("/tmp/vpnStatus")
//static int keyModeTmp= -1 ;
static int vpnTblIdx = 1;
//static int lastIdx = -1 ;

static int getTunnelEntry( int id, IPSECTUNNEL_T *entry );
///////////////////////////////////////////////////////////////////
int mask2Len(char *buf)
{
	struct in_addr  mask;
	int len =0 ;
	int i ;
	
	if(inet_aton(buf,&mask)==0)
		return -1 ;

	for(i=31 ;i>=0 ;i--){
		if((ntohl(mask.s_addr) >> i)& 0x1)
			len++;
		else
			break;
	}
	return len ;

}
///////////////////////////////////////////////////////////////////
void len2Mask(int len, char * mask )
{
	unsigned int addr=0 ;
	unsigned char *tmp ;
	int i ;

	for(i=31; i>=0 ; i--){
		if(len-- > 0)
			addr |= (1 << i);
		else
			break;
	}
	addr = htonl(addr);
	tmp = (unsigned char *) &addr ;
	sprintf(mask, "%d.%d.%d.%d", tmp[0] , tmp[1] , tmp[2] , tmp[3] );

}
///////////////////////////////////////////////////////////////////
unsigned int len2Mask2(int len)
{
	unsigned int addr=0 ;
	int i ;

	for(i=31; i>=0 ; i--){
		if(len-- > 0)
			addr |= (1 << i);
		else
			break;
	}
	addr = htonl(addr);
	return addr ;

}
///////////////////////////////////////////////////////////////////

int getIpsecInfo(IPSECTUNNEL_T *entry)
{
	return getTunnelEntry(vpnTblIdx, entry);
}
///////////////////////////////////////////////////////////////////
int getVpnTblIdx(void)
{
	return vpnTblIdx ;
}
///////////////////////////////////////////////////////////////////
static int getTunnelEntry( int id, IPSECTUNNEL_T *entry )
{
	int entryNum, i ;

	if ( !apmib_get(MIB_IPSECTUNNEL_NUM, (void *)&entryNum)) {
		return -1 ;
	}

	for (i=1; i<=entryNum; i++) {
		IPSECTUNNEL_T checkEntry;
		*((char *)&checkEntry) = (char)i;
		if ( !apmib_get(MIB_IPSECTUNNEL, (void *)&checkEntry)){
			return -1 ;        
		}
		if(checkEntry.tunnelId == id){
			memcpy(entry, &checkEntry, sizeof(IPSECTUNNEL_T));
			return 0;
		}
	}
	return -1 ;
}
#if 0
int checkDupEntry(IPSECTUNNEL_T *entry, char * msg)
{
	int entryNum, i;
	int lc_match=0, rt_match=0, gw_match=0;
	unsigned long v1, v2, v3, v4;

	if ( !apmib_get(MIB_IPSECTUNNEL_NUM, (void *)&entryNum)) {
		strcpy(msg, "Get Mib ipsec tunnel error\n");
		return -1 ;
	}

	for (i=1; i<=entryNum; i++) {
		IPSECTUNNEL_T checkEntry;
		lc_match=0; rt_match=0; gw_match=0;
		//don't deal with the same entry 
		*((char *)&checkEntry) = (char)i;
		if ( !apmib_get(MIB_IPSECTUNNEL, (void *)&checkEntry)){
			sprintf(msg, "Get Mib -ipsec tunnel entry %d error\n", checkEntry.tunnelId);
			return -1 ;        
		}
		if(entry->tunnelId == checkEntry.tunnelId)
			continue ;
		if(!strcmp(checkEntry.connName, entry->connName)){
			sprintf(msg, "Duplicate Connection Name with Tunnel %d\n", checkEntry.tunnelId);
			return -1 ;
		}
		if(checkEntry.lcType == entry->lcType){
			if(entry->lcType == SINGLE_ADDR){
				v1 = *((unsigned long *)checkEntry.lc_ipAddr);	
				v2 = *((unsigned long *)entry->lc_ipAddr);	
				if( v1 == v2)
					lc_match = 1;
			}
			else{ // SUBNET Address
				v1 = *((unsigned long *)checkEntry.lc_ipAddr) &len2Mask2(checkEntry.lc_maskLen);
				v2 = *((unsigned long *)entry->lc_ipAddr) & len2Mask2(entry->lc_maskLen);
				if(v1 == v2 )
					lc_match = 1;
			}
		}
		if(checkEntry.rtType == entry->rtType){
			if(entry->rtType == SINGLE_ADDR){
				v1 = *((unsigned long *)checkEntry.rt_ipAddr);	
				v2 = *((unsigned long *)entry->rt_ipAddr);	
				v3 = *((unsigned long *)checkEntry.rt_gwAddr);	
				v4 = *((unsigned long *)entry->rt_gwAddr);	

				if( (v1 == v2) && (v3== v4))
					rt_match = 1;
			}
			else if (entry->rtType == SUBNET_ADDR){ // SUBNET Address
				v1 = *((unsigned long *)checkEntry.rt_ipAddr) &len2Mask2(checkEntry.rt_maskLen);
				v2 = *((unsigned long *)entry->rt_ipAddr) & len2Mask2(entry->rt_maskLen);
				v3 = *((unsigned long *)checkEntry.rt_gwAddr);	
				v4 = *((unsigned long *)entry->rt_gwAddr);	

				if((v1 == v2)&& (v3==v4))
					rt_match = 1;
			}
			else // Any Address
					rt_match = 1;
		}
		if(lc_match & rt_match){
			sprintf(msg, "Error :Duplicate Local and Remote Setting with Tunnel %d", checkEntry.tunnelId);
			return -1 ;
		}
	}
	return 0 ;
}
#endif

#if 0
///////////////////////////////////////////////////////////////////
void dumpIpsecTunnel( IPSECTUNNEL_T entry)
{
	char tmpBuf[20];
	unsigned char *ptr ;

	printf("\nEnable=%d \n",entry.enable);
	printf("tunnelId=%d \n",entry.tunnelId);
	printf("Connection Name=%s \n",entry.connName);
	printf("========Local Info============\n");
	printf("lcType=%d \n",entry.lcType);
	ptr =(unsigned char *) (entry.lc_ipAddr) ;
	sprintf(tmpBuf,"%u.%u.%u.%u",ptr[0], ptr[1],ptr[2], ptr[3]);
	printf("lc_ipAddr=%s\n",tmpBuf);
	printf("lc_maskLen=%d \n", entry.lc_maskLen);

	printf("========Remote Info============\n");
	printf("rtType=%d \n",entry.rtType);
	ptr =(char*)(entry.rt_ipAddr) ;
	sprintf(tmpBuf,"%u.%u.%u.%u",ptr[0], ptr[1], ptr[2], ptr[3]);
	printf("rt_ipAddr=%s\n",tmpBuf);
	printf("rt_maskLen=%d \n", entry.rt_maskLen);
	ptr =(char*)(entry.rt_gwAddr) ;
	sprintf(tmpBuf,"%u.%u.%u.%u",ptr[0], ptr[1], ptr[2], ptr[3]);
	printf("rt_gwAddr=%s\n",tmpBuf);

	printf("========Key Info============\n");
	printf("keymode=%d \n", entry.keyMode);
	//printf("espAh=%d \n", entry.espAh);
	printf("espEncr=%d \n", entry.espEncr);
	printf("espAuth=%d \n", entry.espAuth);
	//printf("ahAuth=%d \n", entry.ahAuth);
	printf("========Auto Key============\n");
	printf("ConType=%d \n", entry.conType);
	printf("Pskey =%s \n", entry.psKey);
	printf("ikeEncr =%d \n", entry.ikeEncr);
	printf("ikeAuth =%d \n", entry.ikeAuth);
	printf("ikeLifeTime =%lu \n", entry.ikeLifeTime);
	printf("ikeKeyGroup =%d \n", entry.ikeKeyGroup);
	printf("ipsecLifeTime =%lu \n", entry.ipsecLifeTime);
	printf("ipsecPfs =%d \n", entry.ipsecPfs);
	printf("========Manual Key============\n");
	printf("spi=%s \n", entry.spi);
	printf("encrKey=%s \n", entry.encrKey);
	printf("authKey=%s \n", entry.authKey);
	


}
#endif
///////////////////////////////////////////////////////////////////
//#undef NO_ACTION
void formVpnSetup(request *wp, char *path, char *query)
{
	char  *strTmp, *submitUrl;
	char *strLcIp, *strLcType, *strLcMask, *strRtType, *strRtIp, *strRtMask, *strRtGw;
	char *strKeyMode, *strEspEncr, *strEspAuth;
	char *strSpi, *strEncrKey, *strAuthKey ;
	char *strPsKey, *strConnType;
	char *strEnabled, *strTunnelId, *strConnName;
	IPSECTUNNEL_T entry;
	int intVal;
	char tmpBuf[100];
	int buttonMode = -1 ;
#if 0
	struct in_addr curIpAddr, curSubnet;
	unsigned long v1, v2, v3;
#endif
	DHCP_T curDhcp;
	char *wan_port ;
#ifndef NO_ACTION
	int pid;
#endif

	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page
	strTmp=  req_get_cstream_var(wp, ("vpnConnect"), "");
	if(strTmp[0]){
		buttonMode = 0 ;
		goto vpn_connect;
	}
	strTmp=  req_get_cstream_var(wp, ("vpnDisconnect"), "");
	if(strTmp[0]){
		buttonMode = 1 ;
		goto vpn_connect;
	}

	strEnabled =  req_get_cstream_var(wp, ("tunnelEnabled"), "");
	strTunnelId =  req_get_cstream_var(wp, ("vpnTblIdx"), "");

	strLcType =  req_get_cstream_var(wp, ("ipsecLocalType"), "");
	strLcIp =  req_get_cstream_var(wp, ("ipsecLocalIp"), "");
	strLcMask =  req_get_cstream_var(wp, ("ipsecLocalIpMask"), "");
	strRtType  =  req_get_cstream_var(wp, ("ipsecRemoteType"), "");
	strRtIp =  req_get_cstream_var(wp, ("ipsecRemoteIp"), "");
	strRtMask =  req_get_cstream_var(wp, ("ipsecRemoteIpMask"), "");
	strRtGw =  req_get_cstream_var(wp, ("ipsecRemoteGateway"), "");

	//strEspAh = req_get_cstream_var(wp, ("ipsecEspAh"), "");
	strEspEncr = req_get_cstream_var(wp, ("ipsecEspEncr"), "");
	strEspAuth = req_get_cstream_var(wp, ("ipsecEspAuth"), "");
	//strAhAuth = req_get_cstream_var(wp, ("ipsecAhAuth"), "");

	strKeyMode =  req_get_cstream_var(wp, ("ipsecKeyMode"), "");

	strConnName = req_get_cstream_var(wp, ("ipsecConnName"), "");

	if(getTunnelEntry(vpnTblIdx, &entry) < 0){
		// no such entry
		memset(&entry, 0 , sizeof(IPSECTUNNEL_T));
		entry.espEncr = TRI_DES_ALGO; // set default 
		entry.ikeEncr = TRI_DES_ALGO; // set default 
		entry.ikeLifeTime = 3600; // set default 
		entry.ipsecLifeTime = 28800; // set default 
	}else{
		//delete old vpn connection first
		if(entry.enable){
			if(entry.keyMode == IKE_MODE )
				snprintf(tmpBuf, 100, "ipsec auto --delete %s", entry.connName);
			else
				snprintf(tmpBuf, 100, "ipsec manual --down %s", entry.connName);
			printf("%s in %s\n", tmpBuf, __FUNCTION__);
			system(tmpBuf);
		}
	}

	if(!strcmp(strEnabled, "ON"))
		entry.enable = 1 ;
	else
		entry.enable = 0 ;
	//get Connection Name =================================================
	if ( strConnName[0] ) {
			if (strlen(strConnName) > (MAX_NAME_LEN-1)) {
				strcpy(tmpBuf, ("Error! Connection Name length too long."));
				goto setErr_vpn;
			}
			strcpy(entry.connName, strConnName);
	}
	// get Tunnel ID
	if ( !string_to_dec(strTunnelId, &intVal) || intVal<1 || intVal>10) {
		strcpy(tmpBuf, ("Error! Invalid value of tunnel id"));
		goto setErr_vpn;
	}
	entry.tunnelId = intVal ;
	// Set Auth type =================================================
	strTmp = req_get_cstream_var(wp, ("ipsecAuthType"), "");
	if(strTmp[0])
		entry.authType = strTmp[0] - '0'  ;
	//save RSA key
	if(entry.authType ==1){ //RSA
		strTmp = req_get_cstream_var(wp, ("rtRsaKey"), "");
		strncpy(entry.rsaKey, strTmp, MAX_RSA_KEY_LEN);
	}
	// Set local id type =================================================	
	strTmp = req_get_cstream_var(wp, ("ipsecLocalIdType"), "");
	if(strTmp[0])
		entry.lcIdType = strTmp[0] - '0'  ;
	if(entry.lcIdType != 0){ // not IP type
		strTmp = req_get_cstream_var(wp, ("ipsecLocalId"), "");
		strcpy(entry.lcId, strTmp);
	}

	strTmp = req_get_cstream_var(wp, ("ipsecRemoteIdType"), "");
	if(strTmp[0])
		entry.rtIdType = strTmp[0] - '0'  ;
	if(entry.rtIdType != 0){ // not IP type
		strTmp = req_get_cstream_var(wp, ("ipsecRemoteId"), "");
		strcpy(entry.rtId, strTmp);
	}

	// Set local Site Info =================================================	
	strTmp = req_get_cstream_var(wp, ("ipsecRemoteIdType"), "");
	if(strLcType[0])
		entry.lcType  = strLcType[0] - '0'  ; // Single= 0 , subnet =1 
	if(strLcIp[0])
		inet_aton(strLcIp, (struct in_addr *)(entry.lc_ipAddr));

	if(entry.lcType == SUBNET_ADDR){ // subnet Address
		if(strLcMask[0]){
			if((intVal = mask2Len(strLcMask)) == -1){
				strcpy(tmpBuf, ("Error! Invalid value of Local IP mask"));
				goto setErr_vpn;
			}
			entry.lc_maskLen = intVal ; 
		}
	}else{  // single address
		entry.lc_maskLen = 32 ;
	}

	// Set Remote Site Info ================================================		
	if(strRtType[0])
		entry.rtType = strRtType[0] - '0';// Single= 0 , subnet =1  , any = 2
	if(entry.rtType != ANY_ADDR && strRtIp[0])
		inet_aton(strRtIp, (struct in_addr *)(entry.rt_ipAddr));
		
	// check if any existing entry has been set ANY_ADDR or NATT_ADDR
	if (strRtType[0] && 
		((entry.rtType == ANY_ADDR) || (entry.rtType == NATT_ADDR))) {
		int entryNum, i ;
		IPSECTUNNEL_T checkEntry;
		apmib_get(MIB_IPSECTUNNEL_NUM, (void *)&entryNum);
		for (i=1; i<=entryNum; i++) {
			*((char *)&checkEntry) = (char)i;
			apmib_get(MIB_IPSECTUNNEL, (void *)&checkEntry);
			if ((entry.tunnelId != checkEntry.tunnelId)
				&& (checkEntry.rtType == ANY_ADDR || checkEntry.rtType == NATT_ADDR)
				&& (checkEntry.authType == 0) && (checkEntry.enable)//AuthType is PSK
			   ) {
				strcpy(tmpBuf, ("Error! Cannot set Any Address in Remote Site more than one enabled-PSK entry."));
				goto setErr_vpn;
			}
		}

		if (entry.authType == 0){ //AuthType is PSK
			for (i=1; i<=entryNum; i++) {
				*((char *)&checkEntry) = (char)i;
				apmib_get(MIB_IPSECTUNNEL, (void *)&checkEntry);
				if ((entry.tunnelId != checkEntry.tunnelId)
					&& (checkEntry.rtType == ANY_ADDR || checkEntry.rtType == NATT_ADDR)
					&& (checkEntry.authType == 1)&&(checkEntry.enable)) { //AuthType is RSA
					strcpy(tmpBuf, ("Error! Cannot set (PSK) Any Address in Remote Site more than one enabled-RSA entry"));
					goto setErr_vpn;
				}
			}
		}
	}		
	
	if(strRtGw[0] &&  (entry.rtType != ANY_ADDR || entry.rtType != NATT_ADDR) ) // not any address
		inet_aton(strRtGw, (struct in_addr *)(entry.rt_gwAddr));

	if((entry.rtType ==  SUBNET_ADDR || entry.rtType == NATT_ADDR) 
	&& strRtMask[0]){ // subnet Address
		if((intVal = mask2Len(strRtMask)) == -1){
			strcpy(tmpBuf, ("Error! Invalid value of RemoteIP mask"));
			goto setErr_vpn;
		}
		entry.rt_maskLen = intVal ;
	}else if (entry.rtType == SINGLE_ADDR)
		entry.rt_maskLen = 32;
	// Ike Info ================================================		
	if(strKeyMode[0])
		entry.keyMode = (strKeyMode[0] == '0')? 0:1 ; // IKE =0, Manual =1

	// Setup ESP AH  encryption , authentication algorithm
	//entry.espAh = (strEspAh[0] == '0')? 0:1 ; // ESP =0 , AH =1 
	if(strEspEncr[0])
		entry.espEncr=  strEspEncr[0] - '0';  // DES =0, 3DES=1, AES=2 , NULL =3
	if(strEspAuth[0])
		entry.espAuth = strEspAuth[0] - '0' ;  // MD5 =0 , SHA1 =1
		//entry.ahAuth = strAhAuth[0] - '0' ;  // MD5 =0 , SHA1 =1
	if(entry.keyMode == IKE_MODE){ // Ike mode

		strPsKey = req_get_cstream_var(wp, ("ikePsKey"), "");
		strConnType = req_get_cstream_var(wp, ("vpnConnectType"), "");
		entry.conType = strConnType[0] - '0' ;
		if ( strPsKey[0]&& entry.authType ==0){ //PSK
			if (strlen(strPsKey) > (MAX_NAME_LEN-1)) {
				strcpy(tmpBuf, ("Error! Preshared Key length too long."));
				goto setErr_vpn;
			}
			strcpy(entry.psKey, strPsKey);
		}
		strTmp = req_get_cstream_var(wp, ("ikeLifeTime"), "");
		if (strTmp[0]){
			entry.ikeLifeTime = strtol(strTmp,(char **)NULL, 10);
		}
		strTmp = req_get_cstream_var(wp, ("ikeEncr"), "");
		if (strTmp[0]){
			entry.ikeEncr= strTmp[0] - '0' ;
		}
		strTmp = req_get_cstream_var(wp, ("ikeAuth"), "");
		if (strTmp[0]){
			entry.ikeAuth= strTmp[0] - '0' ;
		}
		strTmp = req_get_cstream_var(wp, ("ikeKeyGroup"), "");
		if (strTmp[0]){
			entry.ikeKeyGroup= strTmp[0] - '0' ;
		}
		strTmp = req_get_cstream_var(wp, ("ipsecLifeTime"), "");
		if (strTmp[0]){
			entry.ipsecLifeTime = strtol(strTmp, (char **)NULL, 10);
		}
		strTmp = req_get_cstream_var(wp, ("ipsecPfs"), "");
		if (strTmp[0]){
			entry.ipsecPfs= strTmp[0] - '0' ;
		}
	}
	else{ // manual mode
		strSpi = req_get_cstream_var(wp, ("ipsecSpi"), "");
		if(strSpi[0]){
		       	if (strlen(strSpi) > (MAX_SPI_LEN-1) ) {
				strcpy(tmpBuf, ("Error! SPI length too long."));
				goto setErr_vpn;
			}
			strcpy(entry.spi, strSpi);
		}
		strEncrKey = req_get_cstream_var(wp, ("ipsecEncrKey"), "");
		strAuthKey = req_get_cstream_var(wp, ("ipsecAuthKey"), "");

		if ( strEncrKey[0] ) {
			if (strlen(strEncrKey) > (MAX_ENCRKEY_LEN-1)) {
				strcpy(tmpBuf, ("Error! Encryption Key length too long."));
				goto setErr_vpn;
			}
			strcpy(entry.encrKey, strEncrKey);
		}
		if ( strAuthKey[0] ) { // ESP/AH authenticaiton key 
			if (strlen(strAuthKey) > (MAX_AUTHKEY_LEN-1)) {
				strcpy(tmpBuf, ("Error! Authentication Key length too long."));
				goto setErr_vpn;
			}
			strcpy(entry.authKey, strAuthKey);
		}
	}

	// get entry number to see if it exceeds max
	if ( !apmib_get(MIB_IPSECTUNNEL_NUM, (void *)&intVal)) {
		strcpy(tmpBuf, ("Get entry number error!"));
		goto setErr_vpn;
	}
	if ( (intVal + 1) > MAX_TUNNEL_NUM) {
		strcpy(tmpBuf, ("Cannot add new entry because table is full!"));
		goto setErr_vpn;
	}
#if 0
	if(checkDupEntry(&entry, tmpBuf) < 0){
		goto setErr_vpn;
	}
#endif

	// set to MIB. try to delete it first to avoid duplicate case
	apmib_set(MIB_IPSECTUNNEL_DEL, (void *)&entry);
#if 0
	printf("set entry %d  ------\n", entry.tunnelId);
	dumpIpsecTunnel(entry);
#endif
	if ( apmib_set(MIB_IPSECTUNNEL_ADD, (void *)&entry) == 0) {
		strcpy(tmpBuf, ("Add table entry error!"));
		goto setErr_vpn;
	}
#if 0 // only for the result check
	// get entry number to see if it exceeds max
	if ( !apmib_get(MIB_IPSECTUNNEL_NUM, (void *)&intVal)) {
		strcpy(tmpBuf, ("Get entry number error!"));
		goto setErr_vpn;
	}
	printf("after Addentry number =%d\n", intVal);
	memset(&entry, 0 , sizeof(IPSECTUNNEL_T));
	*((char *)&entry) = (char)intVal;
	if ( !apmib_get(MIB_IPSECTUNNEL, (void *)&entry)){
		strcpy(tmpBuf,  ("Get entry error!"));
		goto setErr_vpn;
	}
	printf("get entry %d------\n", entry.tunnelId);
	dumpIpsecTunnel(entry);
#endif

	apmib_update_web(CURRENT_SETTING);	// update configuration to flash
	

	//  check wan interface ready
	if ( !apmib_get(MIB_WAN_DHCP, (void *)&curDhcp)) {
		strcpy(tmpBuf, ("Get entry number error!"));
		goto setErr_vpn;
	}
	if(curDhcp == DHCP_DISABLED || curDhcp == DHCP_CLIENT)
		wan_port = "eth1" ;
	else if (curDhcp == PPPOE)
		wan_port = "ppp0" ;
	else{
		strcpy(tmpBuf, ("Can't set ipsec with pptp wan type."));		
		goto setErr_vpn;
	}
#if 0 //sc_yang
	if ( !getInAddr(wan_port, IP_ADDR , (void *)&curIpAddr)){
		strcpy(tmpBuf, ("Get WAN IP error!"));
		goto setErr_vpn;
	}
	// check the local site ip setting with LAN config	
	//getInAddr(BRIDGE_IF, IP_ADDR, (void *)&curIpAddr);
	//getInAddr(BRIDGE_IF, SUBNET_MASK, (void *)&curSubnet);
	if ( !apmib_get( MIB_IP_ADDR,  (void *)&curIpAddr) ){
		strcpy(tmpBuf, "Get Lan Ip Error");
		goto setErr_vpn;
	}

	if ( !apmib_get( MIB_SUBNET_MASK,  (void *)&curSubnet) ){
		strcpy(tmpBuf, "Get Lan Mask Error");
		goto setErr_vpn;
	}
	v1 = *((unsigned long *)entry.lc_ipAddr);
	v2 = *((unsigned long *)&curIpAddr);
	v3 = *((unsigned long *)&curSubnet);

	if ( (v1 & v3) != (v2 & v3) ) {
		strcpy(tmpBuf, ("Invalid Local Site IP address! It should be set within the current subnet."));
		goto setErr_vpn;
	}
#endif
#ifndef NO_ACTION
	pid = fork();
        if (pid)
                waitpid(pid, NULL, 0);
        else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _VPN_SCRIPT_PROG);
		execl( tmpBuf, _CONFIG_SCRIPT_PROG, NULL);
                exit(1);
        }
#endif

	submitUrl = req_get_cstream_var(wp, ("top-url"), "");
	OK_MSG(submitUrl);	
	return;
vpn_connect:
	strConnName = req_get_cstream_var(wp, ("ipsecConnName"), "");
	if(buttonMode == 0){  // Connect
		//snprintf(tmpBuf, 100, "ipsec auto --add %s", strConnName);
		snprintf(tmpBuf, 100, "ipsec auto --replace %s", strConnName);
		system(tmpBuf);
		snprintf(tmpBuf, 100, "ipsec auto --up %s", strConnName);
		//system(tmpBuf);
		system(tmpBuf);
	#ifndef NO_ACTION
		pid = fork();
		if (pid){
		       //waitpid(pid, NULL, 0); //don't wait child to return
		}
		else if (pid == 0) {
			snprintf(tmpBuf, 100, "%s/%s", "/sbin", "ipsec");
			execl( tmpBuf, "ipsec" , "auto", "--up" , strConnName);
			exit(1);
		}
	#endif
	}
	else{
		snprintf(tmpBuf, 100, "ipsec auto --down %s", strConnName);
		system(tmpBuf);
	}
	submitUrl = req_get_cstream_var(wp, ("top-url"), "");
	OK_MSG(submitUrl);	
	return;
setErr_vpn:
	ERR_MSG(tmpBuf);
}
#if 0
/////////////////////////////////////////////////////////////////////////////
static int  getSpiAlgo(char *in_tunnel, char *out_algo, int* spi, char *rt_gw)
{
        FILE *fp;
        char  tunnel[40], buf[150], esp_tunnel[40];
	int ret=-1 ;

        sscanf(in_tunnel,"tun%x@%s",spi, rt_gw);
        fp = fopen(_PATH_IPSEC_SPIGRP, "r");
        if ( fp == NULL )
                goto err1;
        while(fgets(buf,150,fp)){
                if(sscanf(buf, "%s %s",tunnel, esp_tunnel) !=2)
			continue ;
                if(!strcmp(in_tunnel, tunnel))
                        break;
        }
        fclose(fp);

        fp = fopen(_PATH_IPSEC_SPI, "r");
        if ( fp == NULL )
                goto err1;
        while(fgets(buf,150,fp)){
                if(sscanf(buf, "%s %s: ",tunnel, out_algo) != 2)
			continue ;
                if(!strcmp(esp_tunnel, tunnel)){
                        out_algo[strlen(out_algo)-1]  = '\0' ;
			ret = 0 ;
			goto err1 ;
                }
        }
	ret = -1 ;
err1:
        fclose(fp);
        return ret ;
}
static int  getConnName(char *in_tunnel, char *connName)
{
	char tmpBuf[50],tunnel[40], buf[150];
        FILE *fp;
	int ret=-1 ;

	sprintf(tmpBuf, "ipsec auto --status | grep %s > %s", in_tunnel, _PATH_IPSEC_VPN_STATUS);
	system(tmpBuf);

        fp = fopen(_PATH_IPSEC_VPN_STATUS, "r");
        if ( fp == NULL )
                goto err1;
        while(fgets(buf,150,fp)){
                if(sscanf(buf, "%*s %*s \"%s %*s %*s %s",connName,  tunnel ) != 2)
			continue ;
                if(!strcmp(tunnel, in_tunnel)){
                        connName[strlen(connName)-1]  = '\0' ;
			ret =0 ;
                }
        }
	ret = -1 ;
err1:
        fclose(fp);
        return ret ;



}
#endif
/////////////////////////////////////////////////////////////////////////////
#define _PATH_IPSEC_VPN_STATUS  ("/tmp/vpnStatus")
#define _PLUTO_FILE ("/var/run/pluto.ctl")
int  getConnStat(char *in_connName)
{
	char  connName[40], buf[150];
        FILE *fp;
	struct stat status;
	int ret=-1 ;
	int i ;

	if ( stat(_PLUTO_FILE, &status) < 0)
		return -1 ;
	sprintf(buf, "ipsec whack --status | grep %s  > %s", in_connName, _PATH_IPSEC_VPN_STATUS);
	system(buf);
        fp = fopen(_PATH_IPSEC_VPN_STATUS, "r");
        if ( fp == NULL )
                goto err1;
        while(fgets(buf,150,fp)){
                if(sscanf(buf, "%*s %*s \"%s",connName) != 1)
			continue ;
		for(i=0 ; i< strlen(connName) ;i++)
			if(connName[i] == '"'){
				connName[i] = '\0' ; 
				break;
			}

                if(!strcmp(in_connName, connName)){
			if(strstr(buf, "STATE_QUICK_I2") || strstr(buf, "STATE_QUICK_R2")) 
				return 0;
                }
        }
	ret = -1 ;
err1:
        fclose(fp);
        return ret ;



}
/////////////////////////////////////////////////////////////////////////////
#if 0
int vpnStatList(request *wp, int argc, char **argv)
{
	FILE *fp;
	int nBytesSent=0;
	int element=0;
	unsigned int pktCnt, spi;
	char dstNet[20], srcNet[20], tunnel[20], algo[20], buf[100], rtGw[20];

	fp = fopen(_PATH_IPSEC_EROUTE, "r");
	if ( fp == NULL )
		goto err;

	while (fgets(buf,100,fp)) {
		sscanf(buf, "%u %s -> %s   => %s",&pktCnt, srcNet, dstNet, tunnel);
		if(getSpiAlgo(tunnel, algo, &spi, rtGw)){
			// not successful 
			spi=0;			
			strcpy(algo,"");
		}		
		element++;
		nBytesSent += req_format_write(wp,
			("<tr bgcolor=#b7b7b7 align=center>\n\
			<td width=19><font size=2>%d</font></td>\n\
			<td width=60><font size=2>0x%x</font></td>\n\
			<td width=88><font size=2>%s</font></td>\n\
			<td width=88><font size=2>%s</font></td>\n\
			<td width=98><font size=2>%s</font></td>\n\
			<td width=87><font size=2>%s</font></td>\n\
			<td width=87><font size=2>%u</font></td></tr>\n"),
			 element ,  spi, srcNet, dstNet, rtGw, algo, pktCnt);
		 
	}
err:
	if (element == 0) {
		nBytesSent += req_format_write(wp,
		("<tr bgcolor=#b7b7b7 align=center>\n\
		<td width=19><font size=2>None</font></td>\n\
		<td width=60><font size=2>----</font></td>\n\
		<td width=88><font size=2>----</font></td>\n\
		<td width=88><font size=2>----</font></td>\n\
		<td width=98><font size=2>----</font></td>\n\
		<td width=87><font size=2>----</font></td>\n\
		<td width=87><font size=2>----</font></td></tr>\n"));
	}


	fclose(fp);
	return nBytesSent;
}
#endif
/////////////////////////////////////////////////////////////////////////////
#if 0
#define _PATH_IPSEC_VPN_LOG "/var/log/secure"
int vpnShowLog(request *wp, int argc, char **argv)
{
        FILE *fp;
	char  buf[150];
	int nBytesSent=0;

        fp = fopen(_PATH_IPSEC_VPN_LOG, "r");
        if ( fp == NULL )
                goto err1;
        while(fgets(buf,150,fp)){
		nBytesSent += req_format_write(wp, ("%s"), buf);
        }
	fclose(fp);
err1:
	return nBytesSent;
}
#endif
/////////////////////////////////////////////////////////////////////////////
int vpnConnList(request *wp, int argc, char **argv)
{
	int nBytesSent=0;
	int element=0;
	char strCheck[20], strConName[31], strStatus[20], strActive[20];
	char strLocalIp[20], strRemoteIp[20], strRemoteGw[20];
	IPSECTUNNEL_T entry ;

	for(element =1 ; element <=  MAX_TUNNEL_NUM ; element ++){
		if(element ==  vpnTblIdx)
			strcpy(strCheck, "checked");
		else
			strcpy(strCheck, "");

		if(getTunnelEntry(element, &entry) < 0){ //fail
			strcpy(strConName,"<center>-</center>");	
			strcpy(strStatus,"<center>-</center>");	
			strcpy(strLocalIp,"<center>-</center>");	
			strcpy(strRemoteIp,"<center>-</center>");	
			strcpy(strRemoteGw,"<center>-</center>");	
			strcpy(strActive,"-");
		}
		else{
			strcpy(strConName,entry.connName);	
			// Active String setup
			if(entry.enable)
				strcpy(strActive,"Y");
			else	
				strcpy(strActive,"N");
			// Local IP String setup
			if(entry.lcType == SINGLE_ADDR) // single address 
				sprintf(strLocalIp,"%s/%d",inet_ntoa(*(struct in_addr *)entry.lc_ipAddr), 32);	
			else  // subnet address
				sprintf(strLocalIp,"%s/%d",inet_ntoa(*(struct in_addr *)entry.lc_ipAddr), entry.lc_maskLen);	

			sprintf(strRemoteGw, "%s", inet_ntoa(*(struct in_addr *)entry.rt_gwAddr));	
			// Remote IP string Setup
			if(entry.rtType == SINGLE_ADDR) // single address 
				sprintf(strRemoteIp,"%s/%d",inet_ntoa(*(struct in_addr *)entry.rt_ipAddr), 32);	
			else if(entry.rtType == SUBNET_ADDR) // subnet address
				sprintf(strRemoteIp,"%s/%d",inet_ntoa(*(struct in_addr *)entry.rt_ipAddr), entry.rt_maskLen);	
			else {
				strcpy(strRemoteIp, "Any");
				strcpy(strRemoteGw, "Any");

			}
			// Status string Setup
			if(entry.keyMode == IKE_MODE ){
				if(getConnStat(strConName))
					strcpy(strStatus, "Disconnected");
				else
					strcpy(strStatus, "Connected");
			}else
				strcpy(strStatus, "<center>-<center>");

		}
		nBytesSent += req_format_write(wp,
		("\n<tr bgcolor=#808080 >\n\
		<td width=19><font size=2><input type=radio name=vpnTblIdx value=%d %s>\n\
		</td><td width=19 bgcolor=#C0C0C0><font size=2><center>%d</center></td>\n\
		<td width=60 bgcolor=#C0C0C0><font size=2>%s<font></td>\n\
		<td width=19 bgcolor=#C0C0C0><font size=2>%s<font></td>\n\
		<td width=88 bgcolor=#C0C0C0><font size=2>%s</font></td>\n\
		<td width=100 bgcolor=#C0C0C0><font size=2>%s</font></td>\n\
		<td width=100 bgcolor=#C0C0C0><font size=2>%s</font></td>\n\
		<td width=70 bgcolor=#C0C0C0><font size=2 color=red>%s</font></td></tr>\n")
		, element, strCheck,  element, strConName, strActive,
		strLocalIp, strRemoteIp, strRemoteGw, strStatus);
	}
	return nBytesSent;
}
/////////////////////////////////////////////////////////////////////////////
void formVpnConn(request *wp, char *path, char *query)
{
	char  *strTmp, *submitUrl;
	char tmpBuf[100];
	int intVal,enabled;
	
	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page

	 strTmp = req_get_cstream_var(wp, ("vpnTblIdx"), "");
	 if(strTmp[0])
	 	vpnTblIdx = atoi(strTmp);

	strTmp = req_get_cstream_var(wp, ("enabled"), "");
	if ( !strcmp(strTmp, ("ON")))
		enabled = 1 ;
	else
		enabled = 0 ;

	if ( apmib_set( MIB_IPSECTUNNEL_ENABLED, (void *)&enabled) == 0) {
		strcpy(tmpBuf, ("Set enabled flag error!"));
		goto setErr_vpnConn;
	}
	if(enabled){
		strTmp = req_get_cstream_var(wp, ("natt_enabled"), "");
		if ( !strcmp(strTmp, ("ON")))
			intVal = 1 ;
		else
			intVal = 0 ;

		if ( apmib_set( MIB_IPSEC_NATT_ENABLED, (void *)&intVal) == 0) {
			strcpy(tmpBuf, ("Set enabled flag error!"));
			goto setErr_vpnConn;
		}
	}
	strTmp = req_get_cstream_var(wp, ("editVpn"), "");  
	if(strTmp[0]){
		apmib_update_web(CURRENT_SETTING);	// update configuration to flash
		send_redirect_perm(wp, ("vpnsetup.htm"));
		return ;
	}
	//sc_yang
	strTmp = req_get_cstream_var(wp, ("rsa_key"), "");  
	if(strTmp[0]){
		struct stat sbuf;
		char *rsaBuf;
		int fh;
		system("ipsec newhostkey --output /tmp/rsaKey --bits 1024");	
		if (stat("/tmp/rsaKey", &sbuf) == -1 ) {
		       strcpy(tmpBuf, ("open rsa file error"));
			goto setErr_vpnConn;
		}
		if(sbuf.st_size > MAX_RSA_FILE_LEN){
		       strcpy(tmpBuf, ("RSA len exceed len"));
			goto setErr_vpnConn;
		}
		rsaBuf = malloc(sizeof(unsigned char) * MAX_RSA_FILE_LEN);
		memset(rsaBuf,' ', MAX_RSA_FILE_LEN);
		fh = open("/tmp/rsaKey", O_RDWR);
		if (fh == -1) {
			printf("Create WPA config file error!\n");
			return;
		}
		read(fh, rsaBuf, MAX_RSA_FILE_LEN);
		if ( apmib_set( MIB_IPSEC_RSA_FILE, (void *)rsaBuf) == 0) {
		       strcpy(tmpBuf, ("Set RSA file error!"));
			goto setErr_vpnConn;
		}
		free(rsaBuf);
		close(fh);
		system("rm -f /tmp/rsaKey");
	}
	

	strTmp = req_get_cstream_var(wp, ("deleteVpn"), "");
	if(strTmp[0]){
		IPSECTUNNEL_T entry ;

		if(getTunnelEntry( vpnTblIdx, &entry ) < 0)
			goto setOk_vpnConn;
		//delete the vpn connection
		if(entry.keyMode == IKE_MODE )
			snprintf(tmpBuf, 100, "ipsec auto --delete %s", entry.connName);
		else
			snprintf(tmpBuf, 100, "ipsec manual --down %s", entry.connName);
		system(tmpBuf);

		apmib_set(MIB_IPSECTUNNEL_DEL, (void *)&entry);
		goto setOk_vpnConn;
	}

	apmib_update_web(CURRENT_SETTING);	// update configuration to flash

#ifndef NO_ACTION
	run_init_script("all");
#endif                
        
setOk_vpnConn:
	submitUrl = req_get_cstream_var(wp, "submit-url", "");
	OK_MSG(submitUrl);

	return;

setErr_vpnConn:
	ERR_MSG(tmpBuf);
}
/////////////////////////////////////////////////////////////////////////////
#if 0
void formVpnLog(request *wp, char *path, char *query)
{
	char  *submitUrl;
	char tmpBuf[100];

	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page

#ifndef NO_ACTION
		snprintf(tmpBuf, 100, "echo \" \" > %s", _PATH_IPSEC_VPN_LOG );
		system(tmpBuf);
		send_redirect_perm(wp, submitUrl);
#endif
	return;
}
#endif
/////////////////////////////////////////////////////////////////////////////
int vpnRsaList(request *wp, int argc, char **argv)
{
	 FILE *fp;
        char  buf[150];
        int nBytesSent=0;

	system("flash rsa /tmp/tmpRsa");
	system("cat /tmp/tmpRsa | grep pubkey| cut -b 10- > /tmp/tmpRsa");
        fp = fopen("/tmp/tmpRsa", "r");
        if ( fp == NULL )
                goto err1;
        while(fgets(buf,150,fp)){
                nBytesSent += req_format_write(wp, ("%s"), buf);
        }
        fclose(fp);
err1:
	system("rm /tmp/tmpRsa");
	return nBytesSent;
}

#endif // VPN_SUPPORT
