
//#include <linux/config.h>
#include <stdio.h>
#include <string.h> //davidhsu
#include <stdarg.h>
#include "boa.h"

#ifdef SUPPORT_ASP

#include "asp_page.h"
#include "apmib.h"
#include "apform.h"

extern void create_chklist_file(int type);
extern int isPackExist;
temp_mem_t root_temp;
char *query_temp_var=NULL; 
char str_boundry[MAX_BOUNDRY_LEN]; //used to store boundry


char *WAN_IF;
char *BRIDGE_IF;
char *ELAN_IF;
char *ELAN2_IF;
char *ELAN3_IF;
char *ELAN4_IF;
char *PPPOE_IF;
char WLAN_IF[20];
int wlan_num;
#ifdef MBSSID
int vwlan_num=0;
int mssid_idx=0;
#endif
int last_wantype=-1;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
asp_name_t root_asp[] = {
	{"getInfo", getInfo},
	{"getIndex", getIndex},
#if defined(CONFIG_RTL_P2P_SUPPORT)
	{"getWifiP2PState", getWifiP2PState},
	{"wlP2PScanTbl", wlP2PScanTbl},
#endif
	{"wirelessClientList", wirelessClientList},
	{"wlSiteSurveyTbl", wlSiteSurveyTbl},
	{"wlWdsList", wlWdsList},
#if defined(WLAN_PROFILE)	
	{"wlProfileList", wlProfileList},
	{"wlProfileTblList", wlProfileTblList},
#endif //#if defined(WLAN_PROFILE)	
	{"wdsList", wdsList},
#ifdef MBSSID
	{"getVirtualIndex", getVirtualIndex},
	{"getVirtualInfo", getVirtualInfo},
#endif
#if defined(NEW_SCHEDULE_SUPPORT)
	{"wlSchList", wlSchList},
#endif
	{"getScheduleInfo", getScheduleInfo},
	{"wlAcList", wlAcList},
	//modify by nctu
	{"getModeCombobox", getModeCombobox},
	{"getDHCPModeCombobox", getDHCPModeCombobox},
#ifdef CONFIG_RTK_MESH
#ifdef 	_11s_TEST_MODE_
	{"wlRxStatics", wlRxStatics},
#endif
#ifdef _MESH_ACL_ENABLE_
	{"wlMeshAcList", wlMeshAcList},
#endif
	{"wlMeshNeighborTable", wlMeshNeighborTable},
	{"wlMeshRoutingTable", wlMeshRoutingTable},
	{"wlMeshProxyTable", wlMeshProxyTable},
	{"wlMeshRootInfo", wlMeshRootInfo},
	{"wlMeshPortalTable", wlMeshPortalTable},
#endif
#ifdef TLS_CLIENT	
	{"certRootList", certRootList},	
	{"certUserList", certUserList},
#endif
	{"dhcpClientList", dhcpClientList},
	{"dhcpRsvdIp_List", dhcpRsvdIp_List},
#if defined(POWER_CONSUMPTION_SUPPORT)
	{"getPowerConsumption", getPowerConsumption},
#endif
#if defined(CONFIG_RTL_8198_AP_ROOT) && defined(VLAN_CONFIG_SUPPORTED)
	{"getVlanList", getVlanList},
#endif
#ifdef HOME_GATEWAY
#if defined(VLAN_CONFIG_SUPPORTED)
	{"getVlanList", getVlanList},
#endif
#if 0  //sc_yang
	{"showWanPage", showWanPage},
#endif
	{"portFwList", portFwList},
	{"ipFilterList", ipFilterList},
	{"portFilterList", portFilterList},
	{"macFilterList", macFilterList},
	{"urlFilterList", urlFilterList},
	//{"triggerPortList", triggerPortList},
#ifdef ROUTE_SUPPORT
	{"staticRouteList", staticRouteList},
	{"kernelRouteList", kernelRouteList},
#endif
#if defined(GW_QOS_ENGINE)
	{"qosList", qosList},
#elif defined(QOS_BY_BANDWIDTH)	
	{"ipQosList", ipQosList},
	{"l7QosList", l7QosList},
#endif
#ifdef CONFIG_IPV6
	{"getIPv6Info", getIPv6Info},
	{"getIPv6BasicInfo", getIPv6BasicInfo},	
#endif
	{"getWlProfileInfo", getWlProfileInfo},
#endif //HOME_GATEWAY
	{"sysLogList", sysLogList},
	{"sysCmdLog", sysCmdLog},
#ifdef CONFIG_APP_TR069
	{"TR069ConPageShow", TR069ConPageShow},
#endif
#ifdef HTTP_FILE_SERVER_SUPPORTED
	{"dump_directory_index", dump_directory_index},
	{"Check_directory_status", Check_directory_status},
#endif
#ifdef VOIP_SUPPORT
	{"voip_general_get", asp_voip_GeneralGet},
	{"voip_dialplan_get", asp_voip_DialPlanGet},
	{"voip_tone_get", asp_voip_ToneGet},
	{"voip_ring_get", asp_voip_RingGet},
	{"voip_other_get", asp_voip_OtherGet},
	{"voip_config_get", asp_voip_ConfigGet},
	{"voip_fwupdate_get", asp_voip_FwupdateGet},
	{"voip_net_get", asp_voip_NetGet},
#ifdef CONFIG_RTK_VOIP_SIP_TLS
	{"voip_TLSGetCertInfo", asp_voip_TLSGetCertInfo},
#endif
#endif
	{NULL, NULL}
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
form_name_t root_form[] = {
	{"formWlanSetup", formWlanSetup},
	{"formWlanRedirect", formWlanRedirect},
#if 0
	{"formWep64", formWep64},
	{"formWep128", formWep128},
#endif
	{"formWep", formWep},
#ifdef MBSSID
	{"formWlanMultipleAP", formWlanMultipleAP},
#endif
#ifdef CONFIG_RTK_MESH
	{"formMeshSetup", formMeshSetup},
	{"formMeshProxy", formMeshProxy},
	//{"formMeshProxyTbl", formMeshProxyTbl},
	{"formMeshStatus", formMeshStatus},
#ifdef 	_11s_TEST_MODE_
	{"asdfgh", formEngineeringMode},
	{"zxcvbnm", formEngineeringMode2},
#endif
#ifdef _MESH_ACL_ENABLE_
	{"formMeshACLSetup", formMeshACLSetup},
#endif
#endif
	{"formTcpipSetup", formTcpipSetup},
	{"formPasswordSetup", formPasswordSetup},
	{"formLogout", formLogout},
	{"formUpload", formUpload},
#if defined(CONFIG_USBDISK_UPDATE_IMAGE)
	{"formUploadFromUsb", formUploadFromUsb},
#endif
#ifdef CONFIG_RTL_WAPI_SUPPORT
	{"formWapiReKey", formWapiReKey},
	{"formUploadWapiCert", formUploadWapiCert},
	{"formUploadWapiCertAS0", formUploadWapiCertAS0},
	{"formUploadWapiCertAS1", formUploadWapiCertAS1},
	{"formWapiCertManagement", formWapiCertManagement},
	{"formWapiCertDistribute", formWapiCertDistribute},
#endif
#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
	{"formUpload8021xUserCert", formUpload8021xUserCert},
#endif
#ifdef TLS_CLIENT	
	{"formCertUpload", formCertUpload},
#endif
	{"formWlAc", formWlAc},
	{"formAdvanceSetup", formAdvanceSetup},
	{"formReflashClientTbl", formReflashClientTbl},
	{"formWlEncrypt", formWlEncrypt},
	{"formStaticDHCP", formStaticDHCP},
#if defined(CONFIG_RTL_92D_SUPPORT)
        {"formWlanBand2G5G", formWlanBand2G5G},
#endif

#if defined(CONFIG_RTL_8198_AP_ROOT) && defined(VLAN_CONFIG_SUPPORTED)
	{"formVlan", formVlan},
#endif
#ifdef HOME_GATEWAY
#if defined(VLAN_CONFIG_SUPPORTED)
	{"formVlan", formVlan},
#endif
#if defined(CONFIG_RTK_VLAN_WAN_TAG_SUPPORT)
	{"formVlanWAN", formVlanWAN},
#endif
	{"formWanTcpipSetup", formWanTcpipSetup},
#ifdef ROUTE_SUPPORT
	{"formRoute", formRoute},
#endif
	{"formPortFw", formPortFw},
	{"formFilter", formFilter},
	//{"formTriggerPort", formTriggerPort},
	{"formDMZ", formDMZ},
	{"formDdns", formDdns},
	{"formOpMode", formOpMode},
#if defined(CONFIG_RTL_ULINKER)
	{"formUlkOpMode", formUlkOpMode},
#endif	
#if defined(CONFIG_RTL_FLASH_DUAL_IMAGE_ENABLE)	
	{"formDualFirmware", formDualFirmware},
#endif
#if defined(GW_QOS_ENGINE)
	{"formQoS", formQoS},
#elif defined(QOS_BY_BANDWIDTH)	
	{"formIpQoS", formIpQoS},
#endif
#ifdef CONFIG_RTL_BT_CLIENT
	{"formBTBasicSetting",formBTBasicSetting},
	{"formBTClientSetting",formBTClientSetting},
	{"formBTFileSetting",formBTFileSetting},
	{"formBTNewTorrent",formBTNewTorrent},
#endif
#ifdef DOS_SUPPORT
	{"formDosCfg", formDosCfg},
#endif
#ifdef CONFIG_IPV6
	{"formRadvd", formRadvd},
	{"formDnsv6", formDnsv6},
	{"formDhcpv6s", formDhcpv6s},
	{"formIPv6Addr", formIPv6Addr},
	{"formTunnel6", formTunnel6},	
#endif
#else
	{"formSetTime", formSetTime},
#endif //HOME_GATEWAY
	// by sc_yang
	{"formNtp", formNtp},
	{"formWizard", formWizard},
	{"formPocketWizard", formPocketWizard},
#ifdef REBOOT_CHECK	
	{"formRebootCheck", formRebootCheck},
#if defined(WLAN_PROFILE)	
	{"formSiteSurveyProfile", formSiteSurveyProfile},	
#endif //#if defined(WLAN_PROFILE)		
#endif	
	{"formSysCmd", formSysCmd},
	{"formSysLog", formSysLog},
#if defined(CONFIG_SNMP)
	{"formSetSNMP", formSetSNMP},
#endif
	{"formSaveConfig", formSaveConfig},
	{"formUploadConfig", formUploadConfig},
	{"formSchedule", formSchedule},
#if defined(NEW_SCHEDULE_SUPPORT)	
	{"formNewSchedule", formNewSchedule},	
#endif 
#if defined(CONFIG_RTL_P2P_SUPPORT)
	{"formWiFiDirect", formWiFiDirect},
	{"formWlP2PScan", formWlP2PScan},
#endif
	{"formWirelessTbl", formWirelessTbl},
	{"formStats", formStats},
	{"formWlSiteSurvey", formWlSiteSurvey},
	{"formWlWds", formWlWds},
	{"formWdsEncrypt", formWdsEncrypt},
#ifdef WLAN_EASY_CONFIG
	{"formAutoCfg", formAutoCfg},
#endif
#ifdef WIFI_SIMPLE_CONFIG
	{"formWsc", formWsc},
#endif
#ifdef LOGIN_URL
	{"formLogin", formLogin},
#endif
#ifdef CONFIG_APP_TR069
	{"formTR069Config", formTR069Config},
#ifdef CONFIG_USER_CWMP_WITH_MATRIXSSL
	{"formTR069CertUpload", formTR069CertUpload},
#endif
#endif
#ifdef HTTP_FILE_SERVER_SUPPORTED
	{"formusbdisk_uploadfile", formusbdisk_uploadfile},
#endif
#ifdef VOIP_SUPPORT
	{"voip_general_set", asp_voip_GeneralSet},
	{"voip_dialplan_set", asp_voip_DialPlanSet},
	{"voip_tone_set", asp_voip_ToneSet},
	{"voip_ring_set", asp_voip_RingSet},
	{"voip_other_set", asp_voip_OtherSet},
	{"voip_config_set", asp_voip_ConfigSet},
	{"voip_fw_set", asp_voip_FwSet},
	{"voip_net_set", asp_voip_NetSet},
#ifdef CONFIG_RTK_VOIP_IVR
	{"voip_ivrreq_set", asp_voip_IvrReqSet},
#endif
#ifdef CONFIG_RTK_VOIP_SIP_TLS
	{voip_TLSCertUpload, asp_voip_TLSCertUpload},
#endif
#endif
	{NULL, NULL}
};

/******************************************************************************/
int addTempStr(char *str)
{
	temp_mem_t *temp,*newtemp;

	temp = &root_temp;
	while (temp->next) {
		temp = temp->next;
	}
	newtemp = (temp_mem_t *)malloc(sizeof(temp_mem_t));
	if (newtemp==NULL)
		return FAILED;
	newtemp->str = str;
	newtemp->next = NULL;
	temp->next = newtemp;	
	return SUCCESS;
}

void freeAllTempStr(void)
{
	temp_mem_t *temp,*ntemp;
	
	temp = root_temp.next;
	root_temp.next = NULL;
	while (1) {
		if (temp==NULL)
			break;
		ntemp = temp->next;
		free(temp->str);
		free(temp);
		temp = ntemp;
	}
}

int getcgiparam(char *dst,char *query_string,char *param,int maxlen)
{
	int len,plen;
	int y;

	plen = strlen(param);
	while (*query_string) {
		len = strlen(query_string);
		if ((len=strlen(query_string)) > plen) {
			if (!strncmp(query_string,param,plen)) {
				if (query_string[plen] == '=') { //copy parameter
					query_string += plen+1;
					y = 0;
					while ((*query_string)&&(*query_string!='&')) {
						if ((*query_string=='%') && (strlen(query_string)>2)) {
							if ((isxdigit(query_string[1])) && (isxdigit(query_string[2]))) {
								if (y<maxlen) {
									dst[y++] = ((toupper(query_string[1])>='A'?toupper(query_string[1])-'A'+0xa:toupper(query_string[1])-'0') << 4)
									+ (toupper(query_string[2])>='A'?toupper(query_string[2])-'A'+0xa:toupper(query_string[2])-'0');
								}
								query_string += 3;
								continue;
							}
						}
						if (*query_string=='+')	{
							if (y < maxlen)
								dst[y++] = ' ';
							query_string++;
							continue;
						}
						if (y<maxlen)
							dst[y++] = *query_string;
						query_string++;
					}
					if (y<maxlen)
						dst[y] = 0;
					return y;
				}
			}
		}
		while ((*query_string)&&(*query_string!='&'))
			query_string++;
		query_string++;
	}
	if (maxlen)
		dst[0] = 0;
	return -1;
}
char *memstr(char *membuf, char *param, int memsize)
{
	char charfind;
	char charmem;
	char charfindfisrt;
	char *findpos;
	char *mempos;
	int  mmsz;

	if ((charfindfisrt = *param++) == 0) {
        return membuf;
    }

    while (memsize-- > 0) {
        charmem = *membuf++;
        if (charmem == charfindfisrt) {
            findpos = param;
            mempos = membuf;
            mmsz = memsize;

            while ((charfind = *findpos++) != 0) {
                if (mmsz-- <= 0) {
                    return NULL;
                }
                charmem = *mempos++;

                if (charmem != charfind) {
                    break;
                }
            }

            if (charfind == 0) {
                return (membuf - 1);
            }
        }
    }
    return NULL;
}


int rtl_mime_get_boundry(char *query_string, int query_string_len)
{
	char *substr_start,*ptr;
	char *substr="----------------------------";
	substr_start=memstr(query_string,substr,query_string_len);
	if(substr_start==NULL)
	{
		printf("can't get boundry string\n");
		return -1;
	}
	int i=0;
	ptr=substr_start;
	while( (ptr[i] !=0x0d || ptr[i+1] !=0x0a) && i<MAX_BOUNDRY_LEN)
	{	
		str_boundry[i]=ptr[i];
		i++;
	}
	if(i>=MAX_BOUNDRY_LEN)
	{
		printf("the string of boundry is too long\n");
		return -1;
	}
	str_boundry[i]=0;
	return 0;
}
	
char *rtl_mime_find_boundry(char *query_string, int query_string_len)
{
	char *substr_start;	
	substr_start=memstr(query_string,str_boundry,query_string_len);
	if(substr_start==NULL)
		return NULL;
	int len=strlen(str_boundry);
	substr_start+=len;	 
	return (substr_start+2);
}
	
char *rtl_mime_get_name(char *query_string,char *param,int query_string_len)
{
	char *ptr=NULL;
	ptr=memstr(query_string,param,query_string_len);
	if(ptr==NULL)
		return NULL;
	return ptr;
}

int mime_get_var(char *dst,char *query_string, int query_string_len,char *param,int maxlen)
{
	int len,plen;
	int i,reval;
	char *cur_boundry,*next_boundry,*pname;
	
	char *query_string_orig=query_string;
	plen = strlen(param);
	reval=rtl_mime_get_boundry(query_string,query_string_len);
	if(reval<0)
		return -1;
	int boundry_len=strlen(str_boundry);	
	cur_boundry=rtl_mime_find_boundry(query_string,query_string_len);	
	if(cur_boundry==NULL)
	{
		printf("can't find the first boundry\n");
		return -1;
	}
	query_string_len-=(cur_boundry-query_string_orig);
	while (query_string_len > 0) {		
		
		if(query_string_len<boundry_len+2)
		{
			printf("the query_string_len is less than boundry_len\n");
			break;
		}
		next_boundry=rtl_mime_find_boundry(cur_boundry,query_string_len);
		if(next_boundry==NULL)
		{
			printf("can't find the next boundry\n");
			return -1;
		}
		len=next_boundry-cur_boundry;
		if((pname=rtl_mime_get_name(cur_boundry,param,len))!=NULL)
		{
			pname+=plen+1;	
		
			while((pname[0]) != 0x0d || (pname[1] !=0x0a) || (pname[2] != 0x0d) || (pname[3] != 0x0a))
				pname++;
		/*Skip 0xd 0xa 0xd 0xa*/
			if ((pname[0]) == 0x0d && (pname[1]==0x0a) &&  (pname[2] == 0x0d) && (pname[3] == 0x0a)) { //copy parameter
				pname += 4;			 
				for(i=0;i<next_boundry-pname-boundry_len-2-2;i++)
					dst[i]=pname[i];
			
				dst[i]=0;			 
 		
				return i;
			}
		}
		else 
		{
			cur_boundry=next_boundry;
			query_string_len-=len;			
		}			
	}
	if (maxlen)
		dst[0] = 0;
	return -1;
}
char *req_get_cstream_var_in_mime(request *req, char *var, char *defaultGetValue,int *data_len)
{
	char *buf;	
	int ret;
	if (req->method == M_POST) {		
		int i;
		buf = (char *)malloc(req->upload_len+1);
		if (buf == NULL)
			return (char *)defaultGetValue;
		req->post_data_idx = 0;
		memcpy(buf, req->upload_data, req->upload_len);
		buf[req->upload_len] = 0;
	
		i = req->upload_len -1;
		
		while (i > 0) {
			if ((buf[i]==0x0a)||(buf[i]==0x0d))
				buf[i]=0;
			else
				break;
			i--;
		}
	
	}

	if(buf != NULL) {
		ret = mime_get_var(query_temp_var,buf,req->upload_len,var,MAX_QUERY_TEMP_VAL_SIZE);
	}

	if (ret < 0)
		return (char *)defaultGetValue;
	buf = (char *)malloc(ret+1);
	if(data_len!=NULL)		
		*data_len=ret;
	memcpy(buf, query_temp_var, ret);
	buf[ret] = 0;
	addTempStr(buf);
	return (char *)buf; //this buffer will be free by freeAllTempStr().
}

char *req_get_cstream_var(request *req, char *var, char *defaultGetValue)
{
	char *buf;
#ifndef NEW_POST
	struct stat statbuf;
#endif
	int ret=-1;
	
	if (req->method == M_POST) {
		int i;
// davidhsu --------------------------------
#ifndef NEW_POST
		fstat(req->post_data_fd, &statbuf);
		buf = (char *)malloc(statbuf.st_size+1);
		if (buf == NULL)
			return (char *)defaultGetValue;
		lseek(req->post_data_fd, SEEK_SET, 0);
		read(req->post_data_fd,buf,statbuf.st_size);
		buf[statbuf.st_size] = 0;
		i = statbuf.st_size - 1;
#else
		buf = (char *)malloc(req->post_data_len+1);
		if (buf == NULL)
			return (char *)defaultGetValue;
		req->post_data_idx = 0;
		memcpy(buf, req->post_data, req->post_data_len);
		buf[req->post_data_len] = 0;
		i = req->post_data_len -1;
#endif
//-------------------------------------------
		while (i > 0) {
			if ((buf[i]==0x0a)||(buf[i]==0x0d))
				buf[i]=0;
			else
				break;
			i--;
		}
	}
	else {
		buf = req->query_string;
	}

	if (buf != NULL) {
		ret = getcgiparam(query_temp_var,buf,var,MAX_QUERY_TEMP_VAL_SIZE);	
	}
	
	if (req->method == M_POST)
		free(buf);
	
	if (ret < 0)
		return (char *)defaultGetValue;
	buf = (char *)malloc(ret+1);
	memcpy(buf, query_temp_var, ret);
	buf[ret] = 0;
	addTempStr(buf);
	return (char *)buf; //this buffer will be free by freeAllTempStr().
}

void asp_init(int argc,char **argv)
{
	int i, num;
	char interface[10];
	extern int getWlStaNum(char *interface, int *num);

	root_temp.next=NULL;
	root_temp.str=NULL;	
	
	// david ---- queury number of wlan interface ----------------
	wlan_num = 0;
	for (i=0; i<NUM_WLAN_INTERFACE; i++) {
		sprintf(interface, "wlan%d", i);
		if (getWlStaNum(interface, &num) < 0)
			break;
		wlan_num++;
	}
	
#if defined(VOIP_SUPPORT) && defined(ATA867x)
	// no wlan interface in ATA867x
#else
	if (wlan_num==0)
		wlan_num = 1;	// set 1 as default
#endif

#ifdef MBSSID
	vwlan_num = NUM_VWLAN_INTERFACE; 
#endif
	//---------------------------------------------------------

//conti:
	root_temp.next = NULL;
	root_temp.str = NULL;	

	if (apmib_init() == 0) {
		printf("Initialize AP MIB failed!\n");
		return;
	}

	save_cs_to_file();
	apmib_get(MIB_WAN_DHCP, (void *)&last_wantype);

	/* determine interface name by mib value */
	WAN_IF = "eth1";
	BRIDGE_IF = "br0";
	ELAN_IF = "eth0";
	ELAN2_IF = "eth2";
	ELAN3_IF = "eth3";
	ELAN4_IF = "eth4";

#ifdef HOME_GATEWAY
	PPPOE_IF = "ppp0";
#elif defined(VOIP_SUPPORT) && defined(ATA867x)
	BRIDGE_IF = "eth0";
	ELAN_IF = "eth0";
#else
	BRIDGE_IF = "br0";
	ELAN_IF = "eth0";
#endif
	strcpy(WLAN_IF,"wlan0");
	//---------------------------

	query_temp_var = (char *)malloc(MAX_QUERY_TEMP_VAL_SIZE);
	if (query_temp_var==NULL)
		exit(0);
	return;
	
//main_end:
	//shmdt(pRomeCfgParam);
	exit(0);
}

int allocNewBuffer(request *req)
{
	char *newBuffer;

	newBuffer = (char *)malloc(req->max_buffer_size*2+1);
	if (newBuffer == NULL)
		return FAILED;
	memcpy(newBuffer, req->buffer, req->max_buffer_size);
	req->max_buffer_size <<= 1;	
	free(req->buffer);	
	req->buffer = newBuffer;
	return SUCCESS;
}

int update_content_length(request *req)
{
	char *content_len_orig1;
	char *content_len_orig2;
	int orig_char_length=0;
	int exact_content_len=0;
	char *content_len_start;
	char buff[32];
	
	content_len_orig1 = strstr(req->buffer, "Content-Length:");
	if (content_len_orig1==NULL) //do nothing
		return 0;
	content_len_orig2 = strstr(content_len_orig1, "\r\n");
	content_len_start = strstr((content_len_orig2+2), "\r\n")+strlen("\r\n");
	content_len_orig1 = content_len_orig1 + strlen("Content-Length: ");
	orig_char_length = content_len_orig2 - content_len_orig1;
	if (orig_char_length < 32) {
		memcpy(buff, content_len_orig1, orig_char_length);
		buff[orig_char_length] = '\0';
	}
	else {
		memcpy(buff, content_len_orig1, 31);
		buff[31] = '\0';
	}
	/*
	fprintf(stderr, "buff=%s orig_char_length=%d\n", buff, orig_char_length);
	fprintf(stderr, "req->buffer_start=%d req->buffer_end=%d\n", req->buffer_start, req->buffer_end);
	fprintf(stderr, "req->buffer=%p\n", req->buffer);
	fprintf(stderr, "req->buffer+req->buffer_start=%p\n", req->buffer+req->buffer_start);
	fprintf(stderr, "req->buffer+req->buffer_end=%p\n", req->buffer+req->buffer_end);
	fprintf(stderr, "content_len_start=%p\n", content_len_start);
	*/
	exact_content_len = ((req->buffer+req->buffer_end) - content_len_start);
	//fprintf(stderr, "exact_content_len=%d\n", exact_content_len);
	if(atoi(buff) != exact_content_len) {
		//fprintf(stderr, "need to update the content length!\n");
		sprintf(buff, "%d", exact_content_len);
		if ((strlen(buff)+1) <= orig_char_length){
			memset(content_len_orig1,0,orig_char_length);
			memcpy(content_len_orig1, buff, strlen(buff));
		}
	}
	return exact_content_len;
}
		
void handleForm(request *req)
{
	char *ptr;
	int i;
	#define SCRIPT_ALIAS "/boafrm/"
	
//fprintf(stderr, "###%s:%d req->request_uri=%s###\n", __FILE__, __LINE__, req->request_uri);
	ptr = strstr(req->request_uri, SCRIPT_ALIAS);
	apmib_get(MIB_WAN_DHCP, (void *)&last_wantype);
	if (ptr==NULL) {
		send_r_not_found(req);
		return;
	}
	else {
		form_name_t *now_form;
		ptr+=strlen(SCRIPT_ALIAS);

		for (i=0; root_form[i].name!=NULL; i++) {
			now_form = &root_form[i];
			if ((strlen(ptr) == strlen(now_form->name)) &&
			    (memcmp(ptr,now_form->name,strlen(now_form->name))==0)) {
				send_r_request_ok2(req);		/* All's well */	
/*#ifdef USE_AUTH
				if (req->auth_flag == 1) { // user
					//brad add for wizard
					if(!strcmp(req->request_uri,"/boafrm/formSetEnableWizard")){
						send_redirect_perm(req, "/wizard_back.htm");
					}else{
						//ptr= (char *)req_get_cstream_var(req, ("webpage"), (""));					
						//strcpy(last_url, ptr);				
						//sprintf(ok_msg,"%s","\"Only <b>admin</b> account can change the settings.<br>\""); 				
						//send_redirect_perm(req, WEB_PAGE_APPLY_OK);	
						ptr= (char *)req_get_cstream_var(req, ("webpage"), (""));
						if(ptr ==NULL)
							sprintf(last_url, "/%s", directory_index);
						else
							strcpy(last_url, ptr);	
										
						if(!strcmp(req->request_uri,"/boafrm/formSetRestorePrev") || !strcmp(req->request_uri,"/boafrm/formSetFactory")){
							sprintf(last_url, "%s","util_savesetting.asp");
							if (req->upload_data)
								free(req->upload_data);	
						}
						if(!strcmp(req->request_uri,"/boafrm/formFirmwareUpgrade")){
							sprintf(last_url, "%s","util_firmware.asp");	
							if (req->upload_data)
								free(req->upload_data);	
						}	
						if(!strcmp(req->request_uri,"/boafrm/formWPS")){
							wps_action= (char *)req_get_cstream_var(req, ("wpsAction"), (""));
							if(!strcmp(wps_action, "pbc") || !strcmp(wps_action, "pin")){
								now_form->function(req,NULL,NULL);
								freeAllTempStr();
								return;
							}
						}	
						sprintf(ok_msg,"%s","\"Only <b>admin</b> account can change the settings.<br>\"");
						send_redirect_perm(req, WEB_PAGE_APPLY_OK);	
						}			
				}
				else
#endif*/
				now_form->function(req,NULL,NULL);
				update_content_length(req);
				freeAllTempStr();
				return;
			}
		}
	}
	send_r_not_found(req);
}

void handleScript(request *req,char *left1,char *right1)
{
	char *left=left1,*right=right1;
	asp_name_t *now_asp;
	unsigned int funcNameLength;
	int i, j;
	
	left += 2;
	right -= 1;
	while (1) {
		while (*left==' ') {if(left>=right) break;++left;}
		while (*left==';') {if(left>=right) break;++left;}
		while (*left=='(') {if(left>=right) break;++left;}
		while (*left==')') {if(left>=right) break;++left;}
		while (*left==',') {if(left>=right) break;++left;}
		if (left >= right)
			break;

		/* count the function name length */
		{
			char *ptr = left;

			funcNameLength = 0;
			while (*ptr!='(' && *ptr!=' ') {
				ptr++;
				funcNameLength++;
				if ((unsigned int )ptr >= (unsigned int)right) {
					break;
				}
			}
		}

		for (j=0; root_asp[j].name!=NULL; j++) {
			now_asp = &root_asp[j];
			if ((strlen(now_asp->name) == funcNameLength) &&
			    (memcmp(left,now_asp->name,strlen(now_asp->name))==0)) {
				char *leftc,*rightc;
				int argc=0;
				char *argv[8]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
				
				left += strlen(now_asp->name);
				while (1) {
					int size,exit=0;
					while (1) {
						if (*left==')') {
							exit=1;
							break;
						}
						if (*left=='\"')
							break;
						if ((unsigned int)left > (unsigned int)right) {
							exit=1;
							break;
						}
						left++;	
					}
					
					if (exit==1)
						break;					
					leftc = left;
					leftc++;
					rightc = strstr(leftc,"\"");
					if (rightc==NULL)
						break;
					size = (unsigned int)rightc-(unsigned int)leftc+1;
					argv[argc] = (char *)malloc(size);
					if (argv[argc]==NULL)
						break;
					memcpy(argv[argc],leftc,size-1);
					argv[argc][size-1] = '\0';
					argc++;
					left = rightc + 1;
				}
				//fprintf(stderr, "###%s:%d now_asp->name=%s###\n", __FILE__, __LINE__, now_asp->name);
				now_asp->function(req,argc,argv);
				for (i=0; i<argc; i++)
					free(argv[i]);
				break;
			}
		}
		++left;
	}
}
#endif

extern request inner_req;
extern char inner_req_buff[1024];
extern int middle_segment; //Brad add for update content length
int req_format_write(request *req, char *format, ...)
{
	int bob;
	va_list args;
	char temp[1024];

	if (!req || !format)
		return 0;
	va_start(args, format);
	vsnprintf(temp, sizeof(temp), format, args);
	va_end(args);
	bob = strlen(temp);
	if ((void*)req == (void*)(&inner_req)) {
		strcpy(inner_req_buff, temp);
	}
	else {
#ifndef SUPPORT_ASP
		if ((bob+req->buffer_end) > BUFFER_SIZE)
			bob = BUFFER_SIZE - req->buffer_end;
#else
		while ((bob+req->buffer_end+10) > req->max_buffer_size) {  //Brad modify
			int ret;
			ret = allocNewBuffer(req);
			if (ret==FAILED) {
				bob = BUFFER_SIZE - req->buffer_end;
				break;
			}
		}
#endif
		middle_segment = middle_segment + bob; //brad add for update exact length of asp file
		if (bob > 0) {
			memcpy((req->buffer+req->buffer_end), temp, bob);	
			req->buffer_end += bob;
		}
	}
	return bob;
}
