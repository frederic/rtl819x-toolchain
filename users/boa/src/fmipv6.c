/*Web server handler routines for IPv6
  *
  *Authors hf_shi	(hf_shi@realsil.com.cn) 2008.1.24
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
#include "asp_page.h"
#include "apmib.h"
#include "apform.h"
#include "utility.h"

#ifdef HOME_GATEWAY
#ifdef CONFIG_IPV6

int getRadvdInfo(radvdCfgParam_t *entry)
{
	if ( !apmib_get(MIB_IPV6_RADVD_PARAM,(void *)entry)){
		return -1 ;        
	}
	return 0;
}

int getDnsv6Info(dnsv6CfgParam_t *entry)
{
	if ( !apmib_get(MIB_IPV6_DNSV6_PARAM,(void *)entry)){
		return -1 ;        
	}
	return 0;
}

int getDhcpv6sInfo(dhcp6sCfgParam_t *entry)
{
	if ( !apmib_get(MIB_IPV6_DHCPV6S_PARAM,(void *)entry)){
		return -1 ;        
	}
	return 0;
}

int getTunnel6Info(tunnelCfgParam_t *entry)
{
	if ( !apmib_get(MIB_IPV6_TUNNEL_PARAM,(void *)entry)){
		return -1 ;        
	}
	return 0;
}

int create_Dhcp6CfgFile(dhcp6sCfgParam_t *dhcp6sCfg)
{
	FILE *fp;
	/*open /var/radvd.conf*/
	fp = fopen("/var/dhcp6s.conf", "w");
	if(NULL == fp)
		return -1;
	
	fprintf(fp,"#Dns\n");
	fprintf(fp,"option domain-name-servers %s;\n\n",dhcp6sCfg->DNSaddr6);

	fprintf(fp,"#Interface\n");
	fprintf(fp,"interface %s {\n",dhcp6sCfg->interfaceNameds);
	fprintf(fp,"	address-pool pool1 3600;\n");
	fprintf(fp,"};\n\n");

	fprintf(fp,"#Addrs Pool\n");
	fprintf(fp,"pool pool1 {\n");
	fprintf(fp,"	range %s to %s ;\n",dhcp6sCfg->addr6PoolS,dhcp6sCfg->addr6PoolE);
	fprintf(fp,"};\n\n");
	
      	fclose(fp);
	return 0;
}

int getAddr6Info(addrIPv6CfgParam_t *entry)
{
	if ( !apmib_get(MIB_IPV6_ADDR_PARAM,(void *)entry)){
		return -1 ;        
	}
	return 0;
}

int set_RadvdInterfaceParam(request *wp,  char *path, char *query, radvdCfgParam_t *pradvdCfgParam)
{
	char *tmp;
	uint32 value;

	/*check if enabled*/
	/*get cfg data from web*/
	tmp=req_get_cstream_var(wp,"interfacename","");
	if(strcmp(tmp,pradvdCfgParam->interface.Name))
	{
		/*interface name changed*/
		strcpy(pradvdCfgParam->interface.Name, tmp);
	}
	value =atoi(req_get_cstream_var(wp,"MaxRtrAdvInterval",""));
	if(value != pradvdCfgParam->interface.MaxRtrAdvInterval)
	{
		pradvdCfgParam->interface.MaxRtrAdvInterval = value;
	}
	value =atoi(req_get_cstream_var(wp,"MinRtrAdvInterval",""));
	if(value != pradvdCfgParam->interface.MinRtrAdvInterval)
	{
		pradvdCfgParam->interface.MinRtrAdvInterval = value;
	}
	value =atoi(req_get_cstream_var(wp,"MinDelayBetweenRAs",""));
	if(value != pradvdCfgParam->interface.MinDelayBetweenRAs)
	{
		pradvdCfgParam->interface.MinDelayBetweenRAs = value;
	}
	value =atoi(req_get_cstream_var(wp,"AdvManagedFlag",""));
	if(value > 0)
	{
		pradvdCfgParam->interface.AdvManagedFlag = 1;
	}
	else
	{
		pradvdCfgParam->interface.AdvManagedFlag =0; 
	}
	value =atoi(req_get_cstream_var(wp,"AdvOtherConfigFlag",""));
	if(value >0)
	{
		pradvdCfgParam->interface.AdvOtherConfigFlag = 1;
	}
	else
	{
		pradvdCfgParam->interface.AdvOtherConfigFlag =0;
	}
	value =atoi(req_get_cstream_var(wp,"AdvLinkMTU",""));
	if(value != pradvdCfgParam->interface.AdvLinkMTU)
	{
		pradvdCfgParam->interface.AdvLinkMTU = value;
	}
	/*replace atoi by strtoul to support max value test of ipv6 phase 2 test*/
	tmp = req_get_cstream_var(wp,"AdvReachableTime","");
	value = strtoul(tmp,NULL,10);
	if(value != pradvdCfgParam->interface.AdvReachableTime)
	{
		pradvdCfgParam->interface.AdvReachableTime = value;
	}
	
	/*replace atoi by strtoul to support max value test of ipv6 phase 2 test*/
	tmp = req_get_cstream_var(wp,"AdvRetransTimer","");
	value = strtoul(tmp,NULL,10);	
	if(value != pradvdCfgParam->interface.AdvRetransTimer)
	{
		pradvdCfgParam->interface.AdvRetransTimer = value;
	}
	value =atoi(req_get_cstream_var(wp,"AdvCurHopLimit",""));
	if(value != pradvdCfgParam->interface.AdvCurHopLimit)
	{
		pradvdCfgParam->interface.AdvCurHopLimit = value;
	}
	value =atoi(req_get_cstream_var(wp,"AdvDefaultLifetime",""));
	if(value != pradvdCfgParam->interface.AdvDefaultLifetime)
	{
		pradvdCfgParam->interface.AdvDefaultLifetime = value;
	}
	tmp=req_get_cstream_var(wp,"AdvDefaultPreference","");
	if(strcmp(tmp,pradvdCfgParam->interface.AdvDefaultPreference))
	{
		/*interface name changed*/
		strcpy(pradvdCfgParam->interface.AdvDefaultPreference, tmp);
	}
	value =atoi(req_get_cstream_var(wp,"AdvSourceLLAddress",""));
	
	if(value > 0)
	{
		pradvdCfgParam->interface.AdvSourceLLAddress = 1;
	}
	else
	{
		pradvdCfgParam->interface.AdvSourceLLAddress=0; 
	}
	value =atoi(req_get_cstream_var(wp,"UnicastOnly",""));
	if(value > 0)
	{
		pradvdCfgParam->interface.UnicastOnly = 1;
	}
	else
	{
		pradvdCfgParam->interface.UnicastOnly =0;
	}

	return 0;
}

int set_RadvdPrefixParam(request *wp,  char *path, char *query, radvdCfgParam_t *pradvdCfgParam)
{
	/*get cfg data from web*/
	char *tmpstr;
	char tmpname[30]={0};
	char tmpaddr[30]={0};
	uint32 value;
	int i,j;

	for(j=0;j<MAX_PREFIX_NUM;j++)
	{
		/*get prefix j*/
		sprintf(tmpname,"Enabled_%d",j);
		value=atoi(req_get_cstream_var(wp,tmpname,""));
		if(value >0)
		{
			pradvdCfgParam->interface.prefix[j].enabled = 1;
		}
		else
		{
			pradvdCfgParam->interface.prefix[j].enabled = 0;
		}
		
		for(i=0;i<8;i++)
		{			
			sprintf(tmpname,"radvdprefix%d_%d",j, i+1);
			sprintf(tmpaddr,"0x%s",req_get_cstream_var(wp, tmpname, ""));
			value =strtol(tmpaddr,NULL,16);
			pradvdCfgParam->interface.prefix[j].Prefix[i]= value;
		}

		sprintf(tmpname,"radvdprefix%d_len",j);
		value =atoi(req_get_cstream_var(wp,tmpname,""));
		if(value != pradvdCfgParam->interface.prefix[j].PrefixLen)
		{
			pradvdCfgParam->interface.prefix[j].PrefixLen = value;
		}
		sprintf(tmpname,"AdvOnLinkFlag_%d",j);
		value =atoi(req_get_cstream_var(wp,tmpname,""));
		if(value >0)
		{
			pradvdCfgParam->interface.prefix[j].AdvOnLinkFlag = 1;
		}
		else
		{
			pradvdCfgParam->interface.prefix[j].AdvOnLinkFlag = 0;
		}

		sprintf(tmpname,"AdvAutonomousFlag_%d",j);
		value =atoi(req_get_cstream_var(wp,tmpname,""));
		if(value >0)
		{
			pradvdCfgParam->interface.prefix[j].AdvAutonomousFlag = 1;
		}
		else
		{					
			pradvdCfgParam->interface.prefix[j].AdvAutonomousFlag = 0;
		}		
		sprintf(tmpname,"AdvValidLifetime_%d",j);
		tmpstr = req_get_cstream_var(wp,tmpname,"");
		/*replace atoi by strtoul to support max value test of ipv6 phase 2 test*/
		value = strtoul(tmpstr,NULL,10);
		if(value != pradvdCfgParam->interface.prefix[j].AdvValidLifetime)
		{
			pradvdCfgParam->interface.prefix[j].AdvValidLifetime = value;
		}
		sprintf(tmpname,"AdvPreferredLifetime_%d",j);
		tmpstr = req_get_cstream_var(wp,tmpname,"");
		/*replace atoi by strtoul to support max value test of ipv6 phase 2 test*/
		value = strtoul(tmpstr,NULL,10);
		if(value != pradvdCfgParam->interface.prefix[j].AdvPreferredLifetime)
		{
			pradvdCfgParam->interface.prefix[j].AdvPreferredLifetime = value;
		}
		sprintf(tmpname,"AdvRouterAddr_%d",j);
		value =atoi(req_get_cstream_var(wp,tmpname,""));
		if(value >0)
		{
			pradvdCfgParam->interface.prefix[j].AdvRouterAddr = 1;
		}
		else
		{
			pradvdCfgParam->interface.prefix[j].AdvRouterAddr=0;
		}
		sprintf(tmpname,"if6to4_%d",j);
		tmpstr =req_get_cstream_var(wp,tmpname,"");
		if(strcmp(pradvdCfgParam->interface.prefix[j].if6to4, tmpstr))
		{
			/*interface name changed*/
			strcpy(pradvdCfgParam->interface.prefix[j].if6to4, tmpstr);
		}
	}

	return 0;
}

int  set_RadvdParam(request *wp, char *path, char *query, radvdCfgParam_t *pradvdCfgParam)
{
	
	int enable;
	/*get the configured paramter*/

	/*check if enabled*/
	/*get cfg data from web*/
	enable=atoi(req_get_cstream_var(wp,"enable_radvd",""));
	if(enable ^ pradvdCfgParam->enabled )
	{
       	pradvdCfgParam->enabled = enable;
	}
	if(enable)
	{
		/*set interface data*/
		set_RadvdInterfaceParam(wp, path, query,pradvdCfgParam);
		/*set prefix data*/
		set_RadvdPrefixParam(wp, path, query,pradvdCfgParam);
	}
	return 0;
}

int write_V6Prefix_V6Addr(char *buf, uint16 prefix[] , uint8 len)	
{
	/*valid check*/
	if(NULL == buf )
		return -1;
	if(len>128)
		return -1;
	/*an ipv6 address.full form*/
	sprintf(buf,"%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x",prefix[0], prefix[1], prefix[2], prefix[3],
		prefix[4], prefix[5], prefix[6], prefix[7]);
	if(len<128)
		sprintf(buf+strlen(buf),"/%d",len);
	return 0;
}
int create_RadvdPrefixCfg(FILE *fp,struct AdvPrefix *prefix)
{
	char tmp[256];
	if(NULL == fp)
		return -1;
	/*create prefix part of radvd.conf file*/
 	memset(tmp,0,256);
	write_V6Prefix_V6Addr(tmp,prefix->Prefix,prefix->PrefixLen);
      fprintf(fp,"prefix %s\n",tmp);
      fprintf(fp,"{\n"); 
      /*on/off*/
      if(prefix->AdvOnLinkFlag)
	  	fprintf(fp,"AdvOnLink on;\n");
	/*on/off*/
      if(prefix->AdvAutonomousFlag)
	  	fprintf(fp,"AdvAutonomous on;\n");
	/*seconds|infinity Default: 2592000 seconds (30 days)*/
      /*if(prefix->AdvValidLifetime)*/
	fprintf(fp,"AdvValidLifetime %u;\n",prefix->AdvValidLifetime);
	/*seconds|infinity Default: 604800 seconds (7 days)*/
      /*if(prefix->AdvPreferredLifetime)*/
 	fprintf(fp,"AdvPreferredLifetime %u;\n",prefix->AdvPreferredLifetime);
        /* Mobile IPv6 extensions on/off*/
       if(prefix->AdvRouterAddr)
 	  	fprintf(fp,"AdvRouterAddr on;\n");
        /*6to4 interface*/
	 if(prefix->if6to4[0])
	 	fprintf(fp,"Base6to4Interface %s;\n",prefix->if6to4);
       fprintf(fp,"};\n");
	 return 0;
}
int create_radvdIntCfg(FILE *fp,struct Interface *interface)
{
	int i;
	if(NULL == fp)
		return -1;
	
	/*write the conf file according the radvdcfg*/
	fprintf(fp,"interface %s \n{\n",interface->Name);
	/*default send advertisement*/
	fprintf(fp,"AdvSendAdvert on;\n");
	/*seconds*/
	/*if the parameter default value !=0. but now not specified we take it as 0*/
	/*if(interface->MaxRtrAdvInterval)*/
	fprintf(fp,"MaxRtrAdvInterval %u;\n",interface->MaxRtrAdvInterval);
	/*seconds*/
      /*if(interface->MinRtrAdvInterval)*/
	 fprintf(fp,"MinRtrAdvInterval %u;\n",interface->MinRtrAdvInterval);
	/*seconds*/
      /*if(interface->MinDelayBetweenRAs)*/
	fprintf(fp,"MinDelayBetweenRAs %u;\n",interface->MinDelayBetweenRAs);
	/*on/off*/
      if(interface->AdvManagedFlag)
	  	fprintf(fp,"AdvManagedFlag on;\n");
	/*on/off*/
      if(interface->AdvOtherConfigFlag)
	  	fprintf(fp,"AdvOtherConfigFlag on;\n");
	/*integer*/
      /*if(interface->AdvLinkMTU)*/
	fprintf(fp,"AdvLinkMTU %d;\n",interface->AdvLinkMTU);
	/*milliseconds*/
	/*the following 2  default value is 0.*/
      //if(interface->AdvReachableTime)
	fprintf(fp,"AdvReachableTime %u;\n",interface->AdvReachableTime);
	/*milliseconds*/
      //if(interface->AdvRetransTimer)
 	fprintf(fp,"AdvRetransTimer %u;\n",interface->AdvRetransTimer);
	/*integer*/
      /*if(interface->AdvCurHopLimit)*/
	fprintf(fp,"AdvCurHopLimit %d;\n",interface->AdvCurHopLimit);
	/*seconds*/
      /*if(interface->AdvDefaultLifetime)*/
	fprintf(fp,"AdvDefaultLifetime %d;\n",interface->AdvDefaultLifetime);
      /*low,medium,high default medium*/
      if(interface->AdvDefaultPreference[0])
	  	fprintf(fp,"AdvDefaultPreference %s;\n",interface->AdvDefaultPreference);
	/*on/off*/
      if(interface->AdvSourceLLAddress)
	  	fprintf(fp,"AdvSourceLLAddress on;\n");
	/*on/off*/
      if(interface->UnicastOnly)
	  	fprintf(fp,"UnicastOnly on;\n");

      /*write prefix cfg*/
	for(i=0;i<MAX_PREFIX_NUM;i++)
	{
		if(interface->prefix[i].enabled)
			create_RadvdPrefixCfg(fp,&(interface->prefix[i]));
	}
	fprintf(fp,"};\n");
	return 0;
}
int create_RadvdCfgFile(radvdCfgParam_t *radvdcfg)
{
	FILE *fp;
	/*open /var/radvd.conf*/
	fp = fopen("/var/radvd.conf", "w");
	if(NULL == fp)
		return -1;
	create_radvdIntCfg(fp,&(radvdcfg->interface));
      fclose(fp);
	return 0;
}
void formRadvd(request *wp, char *path, char *query)
{
	int pid;
	char tmpBuf[256];
	char *submitUrl;
	char* value;
	radvdCfgParam_t radvdCfgParam;
	/*Get parameters*/
	getRadvdInfo(&radvdCfgParam);
	
	/*Set parameters*/
	value=req_get_cstream_var(wp,"submit","");
	if(0 == strcmp(value,"Save"))
	{
		set_RadvdParam(wp, path, query,&radvdCfgParam);
	}
	
	/*Set to pMIb*/
	apmib_set(MIB_IPV6_RADVD_PARAM,&radvdCfgParam);
	
	/*Update it to flash*/
setOk_radvd:
	apmib_update(CURRENT_SETTING);

	/*create the config file*/
	create_RadvdCfgFile(&radvdCfgParam);
	/*start the Daemon*/
#ifndef NO_ACTION
	pid = fork();
        if (pid) {
	      	waitpid(pid, NULL, 0);
	}
        else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _IPV6_RADVD_SCRIPT_PROG);
		execl( tmpBuf, _IPV6_RADVD_SCRIPT_PROG, NULL);
               	exit(1);
        }
#endif

	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page
	OK_MSG(submitUrl);

  	return;

setErr_radvd:
	ERR_MSG(tmpBuf);
	return;
}

int  set_DnsParam(request *wp, char *path, char *query, dnsv6CfgParam_t *pdnsv6CfgParam)
{
	char *value;
	int enable;
	/*check if enabled*/
	enable=atoi(req_get_cstream_var(wp,"enable_dnsv6",""));
	if(enable ^ pdnsv6CfgParam->enabled )
	{
       	pdnsv6CfgParam->enabled = enable;
	}
	if(enable)
	{
		value = req_get_cstream_var(wp,"routername","");
		strcpy(pdnsv6CfgParam->routerName,value);
	}
	return 0;
}

int  set_DhcpSParam(request *wp, char *path, char *query, dhcp6sCfgParam_t *dhcp6sCfgParam)
{
	char *value;
	int enable;
	/*check if enabled*/
	enable=atoi(req_get_cstream_var(wp,"enable_dhcpv6s",""));
	if(enable ^ dhcp6sCfgParam->enabled )
	{
       	dhcp6sCfgParam->enabled = enable;
	}

	value = req_get_cstream_var(wp,"dnsaddr","");
	strcpy(dhcp6sCfgParam->DNSaddr6,value);
	
	value = req_get_cstream_var(wp,"interfacenameds","");
	if(!strcmp(value,""))
	{
		sprintf(dhcp6sCfgParam->interfaceNameds,"%s","br0");
	}
	else
	{
		strcpy(dhcp6sCfgParam->interfaceNameds,value);
	}

	value = req_get_cstream_var(wp,"addrPoolStart","");
	strcpy(dhcp6sCfgParam->addr6PoolS,value);

	value = req_get_cstream_var(wp,"addrPoolEnd","");
	strcpy(dhcp6sCfgParam->addr6PoolE,value);
	
	return 0;
}

int  check_Addr6Param(request *wp, char *path, char *query,addrIPv6CfgParam_t *addrIPv6CfgParam)
{
	char *tmpvalue;
	int iLoop;
	char paramName[20];
	uint32 validFlag=0;
	uint32 changeFlag=0;
	

	for(iLoop=1;iLoop<=8;iLoop++)
	{
		sprintf(paramName,"addr_1_%d",iLoop);
		tmpvalue = req_get_cstream_var(wp,paramName,"");
		if((strtol(tmpvalue,NULL,16) != 0x0)) break;
		//bzero(tmpvalue,sizeof(tmpvalue));
		bzero(paramName,sizeof(paramName));
	}		
	if(iLoop < 9)	validFlag |= 0x10;	
	
	for(iLoop=1;iLoop<=8;iLoop++)
	{
		sprintf(paramName,"addr_2_%d",iLoop);
		tmpvalue = req_get_cstream_var(wp,paramName,"");
		if((strtol(tmpvalue,NULL,16) != 0x0)) break;
		//bzero(tmpvalue,sizeof(tmpvalue));
		bzero(paramName,sizeof(paramName));
	}		
	if(iLoop < 9)	validFlag |= 0x20;	

	if(validFlag != 0x0) 
	{
		if((validFlag & 0x00f0) & 0x10)
		{
			for(iLoop=1;iLoop<=8;iLoop++)
			{
				sprintf(paramName,"addr_1_%d",iLoop);
				tmpvalue = req_get_cstream_var(wp,paramName,"");
				if((strtol(tmpvalue,NULL,16) != addrIPv6CfgParam->addrIPv6[0][iLoop-1])) 
				{
					changeFlag |=0x1;
					break;
				}
				//bzero(tmpvalue,sizeof(tmpvalue));
				bzero(paramName,sizeof(paramName));
			}	
			
			tmpvalue = req_get_cstream_var(wp,"prefix_len_1","");
			if((atoi(tmpvalue) != addrIPv6CfgParam->prefix_len[0])) 
			{
				addrIPv6CfgParam->prefix_len[0]=atoi(tmpvalue);
				changeFlag |=0x4;
			}
			//bzero(tmpvalue,sizeof(tmpvalue));
		}

		if((validFlag & 0x00f0) & 0x20)
		{
			for(iLoop=1;iLoop<=8;iLoop++)
			{
				sprintf(paramName,"addr_2_%d",iLoop);
				tmpvalue = req_get_cstream_var(wp,paramName,"");
				if((strtol(tmpvalue,NULL,16) != addrIPv6CfgParam->addrIPv6[1][iLoop-1])) 
				{
					changeFlag |=0x2;
					break;
				}
				//bzero(tmpvalue,sizeof(tmpvalue));
				bzero(paramName,sizeof(paramName));
			}	

			tmpvalue = req_get_cstream_var(wp,"prefix_len_2","");
			if((atoi(tmpvalue) != addrIPv6CfgParam->prefix_len[1])) 
			{
				addrIPv6CfgParam->prefix_len[1]=atoi(tmpvalue);
				changeFlag |=0x8;
			}
			//bzero(tmpvalue,sizeof(tmpvalue));
		}
	}
	
	changeFlag |= validFlag;	
	return changeFlag;		
}

void  del_PreAddr6Param(addrIPv6CfgParam_t addrIPv6CfgParam,uint32 _changFlag)
{
	#ifndef NO_ACTION
	char tmpBuf[256];
//	if(addrIPv6CfgParam.enabled == 1)	
	{
		if((_changFlag & 0x1) || (_changFlag & 0x4))
		{
			sprintf(tmpBuf,"ifconfig %s del %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\/%d",_IPV6_LAN_INTERFACE, 
							addrIPv6CfgParam.addrIPv6[0][0],addrIPv6CfgParam.addrIPv6[0][1],addrIPv6CfgParam.addrIPv6[0][2],addrIPv6CfgParam.addrIPv6[0][3],
							addrIPv6CfgParam.addrIPv6[0][4],addrIPv6CfgParam.addrIPv6[0][5],addrIPv6CfgParam.addrIPv6[0][6],addrIPv6CfgParam.addrIPv6[0][7],
							addrIPv6CfgParam.prefix_len[0]);
			system(tmpBuf);
			bzero(tmpBuf,sizeof(tmpBuf));
		}
		if((_changFlag & 0x2) || (_changFlag & 0x8))
		{
			sprintf(tmpBuf,"ifconfig %s del %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\/%d",_IPV6_WAN_INTERFACE, 
							addrIPv6CfgParam.addrIPv6[1][0],addrIPv6CfgParam.addrIPv6[1][1],addrIPv6CfgParam.addrIPv6[1][2],addrIPv6CfgParam.addrIPv6[1][3],
							addrIPv6CfgParam.addrIPv6[1][4],addrIPv6CfgParam.addrIPv6[1][5],addrIPv6CfgParam.addrIPv6[1][6],addrIPv6CfgParam.addrIPv6[1][7],
							addrIPv6CfgParam.prefix_len[1]);
			system(tmpBuf);
			bzero(tmpBuf,sizeof(tmpBuf));
		}
	}
	#endif	
	return;
}


int  set_Addr6Param(request *wp, char *path, char *query, addrIPv6CfgParam_t *addrIPv6CfgParam,uint32 _changFlag)
{
	char *value;

	if(_changFlag & 0x1)
	{
		value = req_get_cstream_var(wp,"addr_1_1","");
		addrIPv6CfgParam->addrIPv6[0][0]=strtol(value,NULL,16);	
		value = req_get_cstream_var(wp,"addr_1_2","");
		addrIPv6CfgParam->addrIPv6[0][1]=strtol(value,NULL,16);
		value = req_get_cstream_var(wp,"addr_1_3","");
		addrIPv6CfgParam->addrIPv6[0][2]=strtol(value,NULL,16);	
		value = req_get_cstream_var(wp,"addr_1_4","");
		addrIPv6CfgParam->addrIPv6[0][3]=strtol(value,NULL,16);	
		value = req_get_cstream_var(wp,"addr_1_5","");
		addrIPv6CfgParam->addrIPv6[0][4]=strtol(value,NULL,16);
		value = req_get_cstream_var(wp,"addr_1_6","");
		addrIPv6CfgParam->addrIPv6[0][5]=strtol(value,NULL,16);
		value = req_get_cstream_var(wp,"addr_1_7","");
		addrIPv6CfgParam->addrIPv6[0][6]=strtol(value,NULL,16);	
		value = req_get_cstream_var(wp,"addr_1_8","");
		addrIPv6CfgParam->addrIPv6[0][7]=strtol(value,NULL,16);	
	}	
		
	if(_changFlag & 0x2)
	{
		value = req_get_cstream_var(wp,"addr_2_1","");
		addrIPv6CfgParam->addrIPv6[1][0]=strtol(value,NULL,16);	
		value = req_get_cstream_var(wp,"addr_2_2","");
		addrIPv6CfgParam->addrIPv6[1][1]=strtol(value,NULL,16);
		value = req_get_cstream_var(wp,"addr_2_3","");
		addrIPv6CfgParam->addrIPv6[1][2]=strtol(value,NULL,16);	
		value = req_get_cstream_var(wp,"addr_2_4","");
		addrIPv6CfgParam->addrIPv6[1][3]=strtol(value,NULL,16);	
		value = req_get_cstream_var(wp,"addr_2_5","");
		addrIPv6CfgParam->addrIPv6[1][4]=strtol(value,NULL,16);
		value = req_get_cstream_var(wp,"addr_2_6","");
		addrIPv6CfgParam->addrIPv6[1][5]=strtol(value,NULL,16);
		value = req_get_cstream_var(wp,"addr_2_7","");
		addrIPv6CfgParam->addrIPv6[1][6]=strtol(value,NULL,16);	
		value = req_get_cstream_var(wp,"addr_2_8","");
		addrIPv6CfgParam->addrIPv6[1][7]=strtol(value,NULL,16);	
	}

	addrIPv6CfgParam->enabled=1;
	return 0;
}
int get_v6address(uint16 addr[])
{
	unsigned char mac[6];
	unsigned char zero[6]={0};
	apmib_get(MIB_ELAN_MAC_ADDR,mac);
	if(!memcmp(mac,zero,6))
		apmib_get(MIB_HW_NIC0_ADDR,mac);
	
	addr[0]=0xfe80;
	addr[1]=0x0000;
	addr[2]=0x0000;
	addr[3]=0x0000;
	addr[4]=(mac[0]<<8 | mac[1]) | 0x0200;
	addr[5]=0x00ff  |(mac[2]<<8);
	addr[6]=0xfe00 | (mac[3]);
	addr[7]=(mac[4]<<8 | mac[5]);
	return 0;
}
int create_Dnsv6CfgFile(dnsv6CfgParam_t *dnsv6cfg)
{
      FILE *fp;
	uint16 v6linkaddr[8];
	/*open /var/dnsmasq.conf*/
	fp = fopen("/var/dnsmasq.conf","w");
	if(NULL == fp)
		return -1;
	/*Never forward plain names (without a dot or domain part)*/
	fprintf(fp,"domain-needed\n");
	/*Never forward addresses in the non-routed address spaces*/
	fprintf(fp,"bogus-priv\n");
	/*refer to /etc/resolv.conf*/
	fprintf(fp,"resolv-file=/etc/resolv.conf\n");
	/*strict-order disable*/
	fprintf(fp,"#strict-order\n");
	/*no resolv disable*/
	fprintf(fp,"#no-resolv\n");
	/*no poll disable*/
	fprintf(fp,"#no-poll\n");
	
	/*add router name and link-local address for ipv6 address query*/
	get_v6address(v6linkaddr);

	/*get route address eth1 link local ?*/
	if(dnsv6cfg->routerName[0])
	{
		
		fprintf(fp,"address=/%s/%x::%x:%x:%x:%x\n",dnsv6cfg->routerName,v6linkaddr[0],v6linkaddr[4],
			v6linkaddr[5],v6linkaddr[6],v6linkaddr[7]);	
	}
	else/*default name myrouter*/
	{
		fprintf(fp,"address=/myrouter/%x::%x:%x:%x:%x",v6linkaddr[0],v6linkaddr[4],
			v6linkaddr[5],v6linkaddr[6],v6linkaddr[7]);
	}
	fprintf(fp,"#listen-address=\n");
	fprintf(fp,"#bind-interfaces\n");
	fprintf(fp,"#no-hosts\n");
	fclose(fp);
	return 0;
}

void formDnsv6(request *wp, char *path, char *query)
{
	int pid;
	char tmpBuf[256];
	char *submitUrl;
	char* value;
	dnsv6CfgParam_t dnsCfgParam;

	/*Get parameters*/
	getDnsv6Info(&dnsCfgParam);

	/*Set to Parameters*/
	value=req_get_cstream_var(wp,"submit","");
	if(0 == strcmp(value, "Save"))
	{
		set_DnsParam(wp, path, query,&dnsCfgParam);
	}
	
	/*Set to pMIb*/
	apmib_set(MIB_IPV6_DNSV6_PARAM,&dnsCfgParam);

	/*Update it to flash*/
	apmib_update(CURRENT_SETTING);

	/*create the config file*/
	create_Dnsv6CfgFile(&dnsCfgParam);
	
	/*start the Daemon*/
#ifndef NO_ACTION
	pid = fork();
        if (pid) {
	      	waitpid(pid, NULL, 0);
	}
        else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _IPV6_DNSMASQ_SCRIPT_PROG);
		execl( tmpBuf, _IPV6_DNSMASQ_SCRIPT_PROG, NULL);
               	exit(1);
        }
#endif

	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page
	OK_MSG(submitUrl);
	return;
}

void formDhcpv6s(request *wp, char *path, char *query)
{
	dhcp6sCfgParam_t dhcp6sCfgParam;
	char tmpBuf[256];
	char *submitUrl;
	char* value;
	
	/*Get parameters**/
	getDhcpv6sInfo(&dhcp6sCfgParam);

	/*Set to Parameters*/
	value=req_get_cstream_var(wp,"submit","");
	if(0 == strcmp(value, "Save"))
	{
		set_DhcpSParam(wp, path, query,&dhcp6sCfgParam);
	}
	
	/*Set to pMIb*/
	apmib_set(MIB_IPV6_DHCPV6S_PARAM,&dhcp6sCfgParam);

	/*Update it to flash*/
	apmib_update(CURRENT_SETTING);	
	
	/*create the config file*/
	create_Dhcp6CfgFile(&dhcp6sCfgParam);	
	
	/*start the Daemon*/
#ifndef NO_ACTION
	sprintf(tmpBuf,"%s %s",_IPV6_DHCPV6S_SCRIPT_PROG, dhcp6sCfgParam.interfaceNameds);
	system(tmpBuf);
#endif

	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page
	OK_MSG(submitUrl);
	return;
}

void formIPv6Addr(request *wp, char *path, char *query)
{
	addrIPv6CfgParam_t addrIPv6CfgParam,addrIPv6CfgParamBak;
	char tmpBuf[256];
	char *submitUrl;
	char *msg;
	char* value;
	uint32 isChangFlag=0;

	/*Get parameters**/
	getAddr6Info(&addrIPv6CfgParam);
	addrIPv6CfgParamBak = addrIPv6CfgParam;

	/*Set to Parameters*/
	value=req_get_cstream_var(wp,"submit","");
	if(0 == strcmp(value, "Save"))
	{
		isChangFlag=check_Addr6Param(wp, path, query,&addrIPv6CfgParam);

		//Case: invalid address
		if((isChangFlag & 0x00f0) == 0x0)
		{
			msg = "Invalid Addresses!";
			goto FAIL;
		}
		
		//Case: No change
		if((isChangFlag & 0x000f) == 0x0)
		{
			/*
			if((isChangFlag & 0x00f0)&0x20)
				msg = "Br's Address is invalid!";
			else if((isChangFlag & 0x00f0)&0x10)
				msg = "Eth0's Address is invalid!";
			else
			*/
				msg = "No Address Changed!";
			goto FAIL;
		}		
		
		//set to Parameters		
		set_Addr6Param(wp, path, query,&addrIPv6CfgParam,isChangFlag);
		
	}
	
	/*Set to pMIb*/
	apmib_set(MIB_IPV6_ADDR_PARAM,&addrIPv6CfgParam);

	/*Update it to flash*/
	apmib_update(CURRENT_SETTING);		

	#ifndef NO_ACTION	
	//Del Old Addr6
		del_PreAddr6Param(addrIPv6CfgParamBak,isChangFlag);
	//Add New Addr6
	if((isChangFlag & 0x1) ||(isChangFlag & 0x4))
	{
		sprintf(tmpBuf,"ifconfig %s %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\/%d",_IPV6_LAN_INTERFACE, 
						addrIPv6CfgParam.addrIPv6[0][0],addrIPv6CfgParam.addrIPv6[0][1],addrIPv6CfgParam.addrIPv6[0][2],addrIPv6CfgParam.addrIPv6[0][3],
						addrIPv6CfgParam.addrIPv6[0][4],addrIPv6CfgParam.addrIPv6[0][5],addrIPv6CfgParam.addrIPv6[0][6],addrIPv6CfgParam.addrIPv6[0][7],
						addrIPv6CfgParam.prefix_len[0]);
		system(tmpBuf);
		bzero(tmpBuf,sizeof(tmpBuf));
	}

	if((isChangFlag & 0x2)||(isChangFlag & 0x8))
	{
		sprintf(tmpBuf,"ifconfig %s %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\/%d",_IPV6_WAN_INTERFACE, 
						addrIPv6CfgParam.addrIPv6[1][0],addrIPv6CfgParam.addrIPv6[1][1],addrIPv6CfgParam.addrIPv6[1][2],addrIPv6CfgParam.addrIPv6[1][3],
						addrIPv6CfgParam.addrIPv6[1][4],addrIPv6CfgParam.addrIPv6[1][5],addrIPv6CfgParam.addrIPv6[1][6],addrIPv6CfgParam.addrIPv6[1][7],
						addrIPv6CfgParam.prefix_len[1]);
		system(tmpBuf);
		bzero(tmpBuf,sizeof(tmpBuf));
	}
	#endif
	
	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page
	//OK_MSG(submitUrl);
	//return;

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


FAIL:
	ERR_MSG(msg);
	return;
}
void formTunnel6(request *wp, char *path, char *query)
{
	tunnelCfgParam_t tunnelCfgParam;
	char tmpBuf[256];
	char *submitUrl;
	char* value;
	unsigned char buffer[50],wanIP[50];
	int enable;
	
	/*Get parameters**/
	getTunnel6Info(&tunnelCfgParam);

	/*Set to Parameters*/
	value=req_get_cstream_var(wp,"submit","");
	if(0 == strcmp(value, "Save"))
	{
		enable=atoi(req_get_cstream_var(wp,"enable_tunnel6",""));
		if(enable ^ tunnelCfgParam.enabled )
		{
	       	tunnelCfgParam.enabled = enable;
		}
	}

	/*Set to pMIb*/
	apmib_set(MIB_IPV6_TUNNEL_PARAM,&tunnelCfgParam);

	/*Update it to flash*/
	apmib_update(CURRENT_SETTING);	
	
	/*start the Daemon*/
#ifndef NO_ACTION
	//tunnel add 
	if ( !apmib_get( MIB_WAN_IP_ADDR,  (void *)buffer) ) goto setErr_tunnel;
	sprintf(wanIP,"%s",inet_ntoa(*((struct in_addr *)buffer)));
	sprintf(tmpBuf,"ip tunnel add tun mode sit remote any local %s",wanIP);
	system(tmpBuf);	

	bzero(tmpBuf,sizeof(tmpBuf));
	sprintf(tmpBuf,"ifconfig tun up");
	system(tmpBuf);

	char *p1,*p2,*p3,*p4;
	p1=strtok(wanIP,".");
	p2=strtok(NULL,".");
	p3=strtok(NULL,".");
	p4=strtok(NULL,".");

	bzero(tmpBuf,sizeof(tmpBuf));
	sprintf(tmpBuf,"ifconfig tun 2002:%02x%02x:%02x%02x:1::1\/16",atoi(p1),atoi(p2),atoi(p3),atoi(p4));
	system(tmpBuf);

	//br0
	bzero(tmpBuf,sizeof(tmpBuf));
	sprintf(tmpBuf,"ifconfig br0 2002:%02x%02x:%02x%02x:2::1\/64",atoi(p1),atoi(p2),atoi(p3),atoi(p4));
	system(tmpBuf);
	
#endif
	submitUrl = req_get_cstream_var(wp, "submit-url", "");   // hidden page
	OK_MSG(submitUrl);
	return;
setErr_tunnel:
	
	return;
}
/* 
  *  Get constant string :Router Advertisement Setting
  */
uint32 getIPv6Info(request *wp, int argc, char **argv)
{
	char	*name;
	radvdCfgParam_t radvdCfgParam;
	dnsv6CfgParam_t dnsv6CfgParam;
	dhcp6sCfgParam_t dhcp6sCfgParam;
	tunnelCfgParam_t tunnelCfgParam;
	
	//printf("get parameter=%s\n", argv[0]);
	name = argv[0];
	if (name == NULL) {
   		fprintf(stderr, "Insufficient args\n");
   		return -1;
   	}
	
//////MENU//////////////////////////////////////////////////
	if(!strcmp(name,"IPv6_Menu"))
	{
		req_format_write(wp,"menu.addItem(\"ipv6\");");
		req_format_write(wp,"ipv6 = new MTMenu();");
		req_format_write(wp,"ipv6.addItem(\"IPv6 Basic Setting\", \"ipv6_basic.htm\", \"\", \"Configure IPv6 Basic Setting\");\n");
		req_format_write(wp,"ipv6.addItem(\"DHCP Daemon\", \"dhcp6s.htm\", \"\", \"Setup Dhcp Daemon\");\n");
		req_format_write(wp,"ipv6.addItem(\"Router Advertisement Daemon\", \"radvd.htm\", \"\", \"Setup Radvd Daemon\");\n");
		req_format_write(wp,"ipv6.addItem(\"DNS Proxy Daemon\", \"dnsv6.htm\", \"\", \"Setup Dnsmasq Daemon\");\n");
		req_format_write(wp,"ipv6.addItem(\"Tunnel (6 over 4)\", \"tunnel6.htm\", \"\", \"Tunnel (6to4)\");\n"); 
		req_format_write(wp,"menu.makeLastSubmenu(ipv6);\n");
		return 0;
	}

	if(!strcmp(name,"IPv6_nojs_Menu"))
	{
		req_format_write(wp,"<tr><td><b>IPv6</b></td></tr>");
		req_format_write(wp,"<tr><td><a href=\"ipv6_basic.htm\" target=\"view\">IPv6 Basic Setting</a></td></tr>");
		req_format_write(wp,"<tr><td><a href=\"dhcp6s.htm\" target=\"view\">DHCP Daemon</a></td></tr>");
		req_format_write(wp,"<tr><td><a href=\"radvd.htm\" target=\"view\">Router Advertisement Daemon</a></td></tr>");
		req_format_write(wp,"<tr><td><a href=\"dnsv6.htm\" target=\"view\">DNS Proxy Daemon</a></td></tr>");
		req_format_write(wp,"<tr><td><a href=\"tunnel6.htm\" target=\"view\">Tunnel 6over4</a></td></tr>");
		return 0;
	}
//////////radvd///////////////////////////////////////////////////////////////
	if(getRadvdInfo(&radvdCfgParam)<0)
	{
		req_format_write(wp,"Read Radvd Configuration Error");
		return -1;
	}
      if(!strcmp(name,"enable_radvd"))
      {
       	if(radvdCfgParam.enabled)
        		req_format_write(wp,"checked");
      }
      else if(!strcmp(name,"radvdinterfacename"))
        {
        	req_format_write(wp,"%s",radvdCfgParam.interface.Name);
        }
      else if(!strcmp(name,"MaxRtrAdvInterval"))
        {
        	req_format_write(wp,"%u",radvdCfgParam.interface.MaxRtrAdvInterval);
        }
	else  if(!strcmp(name,"MinRtrAdvInterval"))
        {
        	req_format_write(wp,"%u",radvdCfgParam.interface.MinRtrAdvInterval);
        }
	else  if(!strcmp(name,("MinDelayBetweenRAs")))
        {
        	req_format_write(wp,"%u",radvdCfgParam.interface.MinDelayBetweenRAs);
        }
	else  if(!strcmp(name,("AdvManagedFlag")))
        {
		if(radvdCfgParam.interface.AdvManagedFlag)
        		req_format_write(wp,"checked");
        }
	 else if(!strcmp(name,("AdvOtherConfigFlag")))
        {
        	if(radvdCfgParam.interface.AdvOtherConfigFlag)
        		req_format_write(wp,"checked");
        }
	else  if(!strcmp(name,("AdvLinkMTU")))
        {
        	req_format_write(wp,"%d",radvdCfgParam.interface.AdvLinkMTU);
        }
	 else if(!strcmp(name,("AdvReachableTime")))
        {
        	req_format_write(wp,"%u",radvdCfgParam.interface.AdvReachableTime);
        }
	else  if(!strcmp(name,("AdvRetransTimer")))
        {
        	req_format_write(wp,"%u",radvdCfgParam.interface.AdvRetransTimer);
        }
	 else if(!strcmp(name,("AdvCurHopLimit")))
        {
        	req_format_write(wp,"%d",radvdCfgParam.interface.AdvCurHopLimit);
        }
	else  if(!strcmp(name,("AdvDefaultLifetime")))
        {
        	req_format_write(wp,"%d",radvdCfgParam.interface.AdvDefaultLifetime);
        }
	 else if(!strcmp(name,("AdvDefaultPreference_high")))
        {
        	if(!strcmp("high",radvdCfgParam.interface.AdvDefaultPreference))
			req_format_write(wp,"selected");	
         }
	 else if(!strcmp(name,("AdvDefaultPreference_medium")))
        {
        	if(!strcmp("medium",radvdCfgParam.interface.AdvDefaultPreference))
			req_format_write(wp,"selected");	
        }
	 else if(!strcmp(name,("AdvDefaultPreference_low")))
        {
        	if(!strcmp("low",radvdCfgParam.interface.AdvDefaultPreference))
			req_format_write(wp,"selected");	
        }
	 else if(!strcmp(name,("AdvSourceLLAddress")))
        {
                if(radvdCfgParam.interface.AdvSourceLLAddress)
        		req_format_write(wp,"checked");
        }
	else  if(!strcmp(name,("UnicastOnly")))
        {
                	if(radvdCfgParam.interface.UnicastOnly)
        		req_format_write(wp,"checked");
        }

	 /*prefix0*/
	else if(!strcmp(name,("Enabled_0")))
	{
		if(radvdCfgParam.interface.prefix[0].enabled)
			req_format_write(wp,"checked");
	}
	else if(!strcmp(name,("radvdprefix0_1")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[0].Prefix[0]);
	 }
	else if(!strcmp(name,("radvdprefix0_2")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[0].Prefix[1]);
	 }
	else if(!strcmp(name,("radvdprefix0_3")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[0].Prefix[2]);
	 }
	else if(!strcmp(name,("radvdprefix0_4")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[0].Prefix[3]);
	 }
	else if(!strcmp(name,("radvdprefix0_5")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[0].Prefix[4]);
	 }
	else if(!strcmp(name,("radvdprefix0_6")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[0].Prefix[5]);
	 }
	else if(!strcmp(name,("radvdprefix0_7")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[0].Prefix[6]);
	 }
	else if(!strcmp(name,("radvdprefix0_8")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[0].Prefix[7]);
	 }
	else if(!strcmp(name,("radvdprefix0_len")))
	 {
	 	req_format_write(wp,"%d",radvdCfgParam.interface.prefix[0].PrefixLen);
	 }


	else if(!strcmp(name,("AdvOnLinkFlag_0")))
	 {
	       if(radvdCfgParam.interface.prefix[0].AdvOnLinkFlag)
        		req_format_write(wp,"checked");
	 }
	
	else if(!strcmp(name,("AdvAutonomousFlag_0")))
	 {
	 	 if(radvdCfgParam.interface.prefix[0].AdvAutonomousFlag)
        		req_format_write(wp,"checked");
	 }
	else if(!strcmp(name,("AdvValidLifetime_0")))
	 {
	 	req_format_write(wp,"%u",radvdCfgParam.interface.prefix[0].AdvValidLifetime);
	 }
	else if(!strcmp(name,("AdvPreferredLifetime_0")))
	 {
	 	req_format_write(wp,"%u",radvdCfgParam.interface.prefix[0].AdvPreferredLifetime);
	 }
	else if(!strcmp(name,("AdvRouterAddr_0")))
	 {
	 	if(radvdCfgParam.interface.prefix[0].AdvRouterAddr)
        		req_format_write(wp,"checked");
	 }
	else if(!strcmp(name,("if6to4_0")))
	 {
	 	req_format_write(wp,"%s",radvdCfgParam.interface.prefix[0].if6to4);
	 }
	  
	 /*prefix1*/
	else if(!strcmp(name,("Enabled_1")))
	{
		if(radvdCfgParam.interface.prefix[1].enabled)
			req_format_write(wp,"checked");
	}
      else  if(!strcmp(name,("radvdprefix1_1")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[1].Prefix[0]);
	 }
	else if(!strcmp(name,("radvdprefix1_2")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[1].Prefix[1]);
	 }
	else if(!strcmp(name,("radvdprefix1_3")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[1].Prefix[2]);
	 }
	else if(!strcmp(name,("radvdprefix1_4")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[1].Prefix[3]);
	 }
	else if(!strcmp(name,("radvdprefix1_5")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[1].Prefix[4]);
	 }
	else if(!strcmp(name,("radvdprefix1_6")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[1].Prefix[5]);
	 }
	else if(!strcmp(name,("radvdprefix1_7")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[1].Prefix[6]);
	 }
	else if(!strcmp(name,("radvdprefix1_8")))
	 {
	 	req_format_write(wp,"%04x",radvdCfgParam.interface.prefix[1].Prefix[7]);
	 }
	else if(!strcmp(name,("radvdprefix1_len")))
	 {
	 	req_format_write(wp,"%d",radvdCfgParam.interface.prefix[1].PrefixLen);
	 }
	else if(!strcmp(name,("AdvOnLinkFlag_1")))
	 {
	 	if(radvdCfgParam.interface.prefix[1].AdvOnLinkFlag)
			req_format_write(wp,"checked");
	 }
	else if(!strcmp(name,("AdvAutonomousFlag_1")))
	 {
	 	 if(radvdCfgParam.interface.prefix[1].AdvAutonomousFlag)
			req_format_write(wp,"checked");
	 }
	else if(!strcmp(name,("AdvValidLifetime_1")))
	 {
	 	req_format_write(wp,"%u",radvdCfgParam.interface.prefix[1].AdvValidLifetime);
	 }
	else if(!strcmp(name,("AdvPreferredLifetime_1")))
	 {
	 	req_format_write(wp,"%u",radvdCfgParam.interface.prefix[1].AdvPreferredLifetime);
	 }
	else if(!strcmp(name,("AdvRouterAddr_1")))
	 {
	 	 if(radvdCfgParam.interface.prefix[1].AdvRouterAddr)
			req_format_write(wp,"checked");
	 }
	else if(!strcmp(name,("if6to4_1")))
	 {
	 	req_format_write(wp,"%s",radvdCfgParam.interface.prefix[1].if6to4);
	 }
////////////dnsmasq///////////////////////////////////////
	if(getDnsv6Info(&dnsv6CfgParam)<0)
	{
		req_format_write(wp,"Read Dnsmasq Configuration Error");
		return -1;
	}
	 if(!strcmp(name,("enable_dnsv6")))
        {
        	if(dnsv6CfgParam.enabled)
        		req_format_write(wp,"checked");
        }
        else if(!strcmp(name,("routername")))
        {
        	req_format_write(wp,"%s",dnsv6CfgParam.routerName);
        }
///////////////DHCPv6//////////////////////////////////////
	if(getDhcpv6sInfo(&dhcp6sCfgParam)<0)
	{
		req_format_write(wp,"Read Dnsmasq Configuration Error");
		return -1;
	}	
	
	if(!strcmp(name,("enable_dhcpv6s")))
      {
      		if(dhcp6sCfgParam.enabled)
      			req_format_write(wp,"checked");
      }
      else if(!strcmp(name,("interfacenameds")))
      {
      		req_format_write(wp,"%s",dhcp6sCfgParam.interfaceNameds);
      }
	else if(!strcmp(name,("dnsaddr")))
      {
      		req_format_write(wp,"%s",dhcp6sCfgParam.DNSaddr6);
      }	
	else if(!strcmp(name,("addrPoolStart")))
      {
      		req_format_write(wp,"%s",dhcp6sCfgParam.addr6PoolS);
      }	
	else if(!strcmp(name,("addrPoolEnd")))
      {
      		req_format_write(wp,"%s",dhcp6sCfgParam.addr6PoolE);
      }	
	///////////////Tunnel//////////////////////////////////////
	if(!strcmp(name,("enable_tunnel6")))
      {
      		if(getTunnel6Info(&tunnelCfgParam)<0)
		{
			req_format_write(wp,"Read Tunnel Configuration Error");
			return -1;
		}	
      		if(tunnelCfgParam.enabled)
	      		req_format_write(wp,"checked");
      }	
	return 0;
}

int getIPv6BasicInfo(request *wp, int argc, char **argv)
{
	char	*name;
	addrIPv6CfgParam_t addrIPv6CfgParam;
	
	//printf("get parameter=%s\n", argv[0]);
	name = argv[0];
	if (name == NULL) {
   		fprintf(stderr, "Insufficient args\n");
   		return -1;
   	}

	if(getAddr6Info(&addrIPv6CfgParam)<0)
	{
		req_format_write(wp,"Read Dnsmasq Configuration Error");
		return -1;
	}
	
	if(!strcmp(name,("addrIPv6_1_1")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[0][0]);
      }
	else if(!strcmp(name,("addrIPv6_1_2")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[0][1]);
      }
	else if(!strcmp(name,("addrIPv6_1_3")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[0][2]);
      }
	else if(!strcmp(name,("addrIPv6_1_4")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[0][3]);
      }
	else if(!strcmp(name,("addrIPv6_1_5")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[0][4]);
      }
	else if(!strcmp(name,("addrIPv6_1_6")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[0][5]);
      }
	else if(!strcmp(name,("addrIPv6_1_7")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[0][6]);
      }
	else if(!strcmp(name,("addrIPv6_1_8")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[0][7]);
      }
	else if(!strcmp(name,("prefix_len_1")))
      {
		req_format_write(wp,"%d",addrIPv6CfgParam.prefix_len[0]);
      }

	if(!strcmp(name,("addrIPv6_2_1")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[1][0]);
      }
	else if(!strcmp(name,("addrIPv6_2_2")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[1][1]);
      }
	else if(!strcmp(name,("addrIPv6_2_3")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[1][2]);
      }
	else if(!strcmp(name,("addrIPv6_2_4")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[1][3]);
      }
	else if(!strcmp(name,("addrIPv6_2_5")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[1][4]);
      }
	else if(!strcmp(name,("addrIPv6_2_6")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[1][5]);
      }
	else if(!strcmp(name,("addrIPv6_2_7")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[1][6]);
      }
	else if(!strcmp(name,("addrIPv6_2_8")))
      {
		req_format_write(wp,"%04x",addrIPv6CfgParam.addrIPv6[1][7]);
      }
	else if(!strcmp(name,("prefix_len_2")))
      {
		req_format_write(wp,"%d",addrIPv6CfgParam.prefix_len[1]);
      }
	return 0;
}

#endif
#endif
