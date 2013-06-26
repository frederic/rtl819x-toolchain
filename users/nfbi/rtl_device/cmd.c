/*================================================================*/
/* System Include Files */

#include <stdio.h> 
#include <stdlib.h> 
#include <fcntl.h> 
#include <string.h>
#include <sys/types.h> 
#include <sys/uio.h> 
#include <unistd.h> 
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/wait.h> 
#include <net/if.h>
/*================================================================*/
/* Local Include Files */

#include "hcd.h"
#include "mib.h"
#include "cmd.h"
#include "wlan_if.h"
#include "../../../linux-2.6.30/drivers/char/rtl_mdio/rtl_mdio.h"
/*=Local variables===============================================================*/
WLAN_RATE_T rate_11n_table_20M_LONG[]={
	{MCS0, 	"6.5"},
	{MCS1, 	"13"},
	{MCS2, 	"19.5"},
	{MCS3, 	"26"},
	{MCS4, 	"39"},
	{MCS5, 	"52"},
	{MCS6, 	"58.5"},
	{MCS7, 	"65"},
	{MCS8, 	"13"},
	{MCS9, 	"26"},
	{MCS10, 	"39"},
	{MCS11, 	"52"},
	{MCS12, 	"78"},
	{MCS13, 	"104"},
	{MCS14, 	"117"},
	{MCS15, 	"130"},
	{0}
};
WLAN_RATE_T rate_11n_table_20M_SHORT[]={
	{MCS0, 	"7.2"},
	{MCS1, 	"14.4"},
	{MCS2, 	"21.7"},
	{MCS3, 	"28.9"},
	{MCS4, 	"43.3"},
	{MCS5, 	"57.8"},
	{MCS6, 	"65"},
	{MCS7, 	"72.2"},
	{MCS8, 	"14.4"},
	{MCS9, 	"28.9"},
	{MCS10, 	"43.3"},
	{MCS11, 	"57.8"},
	{MCS12, 	"86.7"},
	{MCS13, 	"115.6"},
	{MCS14, 	"130"},
	{MCS15, 	"144.5"},
	{0}
};
WLAN_RATE_T rate_11n_table_40M_LONG[]={
	{MCS0, 	"13.5"},
	{MCS1, 	"27"},
	{MCS2, 	"40.5"},
	{MCS3, 	"54"},
	{MCS4, 	"81"},
	{MCS5, 	"108"},
	{MCS6, 	"121.5"},
	{MCS7, 	"135"},
	{MCS8, 	"27"},
	{MCS9, 	"54"},
	{MCS10, 	"81"},
	{MCS11, 	"108"},
	{MCS12, 	"162"},
	{MCS13, 	"216"},
	{MCS14, 	"243"},
	{MCS15, 	"270"},
	{0}
};
WLAN_RATE_T rate_11n_table_40M_SHORT[]={
	{MCS0, 	"15"},
	{MCS1, 	"30"},
	{MCS2, 	"45"},
	{MCS3, 	"60"},
	{MCS4, 	"90"},
	{MCS5, 	"120"},
	{MCS6, 	"135"},
	{MCS7, 	"150"},
	{MCS8, 	"30"},
	{MCS9, 	"60"},
	{MCS10, 	"90"},
	{MCS11, 	"120"},
	{MCS12, 	"180"},
	{MCS13, 	"240"},
	{MCS14, 	"270"},
	{MCS15, 	"300"},
	{0}
};

char *lan_link_spec[2] = {"DOWN","UP"};
char *lan_speed_spec[3] = {"10M","100M","1G"};
char *enable_spec[2] = {"DIsable","Enable"};

/**/
static int _cfgread(char *cmd , int cmd_len);
static int _cfgwrite(char *cmd , int cmd_len);
static int _getstainfo(char *cmd , int cmd_len);
static int _getassostanum(char *cmd , int cmd_len);
static int _getbssinfo(char *cmd , int cmd_len);
static int _getwdsinfo(char *cmd , int cmd_len);	
static int _sysinit(char *cmd , int cmd_len);
static int _getstats(char *cmd , int cmd_len);	
static int _getlanstatus(char *cmd , int cmd_len);


struct cmd_entry cmd_table[]={ \
/*Action cmd - ( name, func) */
	CMD_DEF(cfgwrite, _cfgwrite),
	CMD_DEF(cfgread, _cfgread),	
	CMD_DEF(getstainfo, _getstainfo),
	CMD_DEF(getassostanum, _getassostanum),
	CMD_DEF(getbssinfo, _getbssinfo),
	CMD_DEF(getwdsinfo, _getwdsinfo),	
	CMD_DEF(sysinit, _sysinit),
	CMD_DEF(getstats, _getstats),
	CMD_DEF(getlanstatus, _getlanstatus),
	/* last one type should be LAST_ENTRY - */   
	{0}
};


int parse_cmd_header(unsigned char *data, int len,unsigned char *cmd, int *cmd_len)
{
	unsigned int real_len=0,check_len=0,i,data_offset=0;
		
	if((data[TAG_FIELD] & SYNC_BIT) == 0 ) 
	{
		DEBUG_ERR("Invalid cmd frame [%d] !\n", len);
		return -1;
		
	}
	if (len < 4) {
		DEBUG_ERR("Invalid cmd len [%d] !\n", len);
		return -1;
	}
	
	if((data[TAG_FIELD] & EXTEND_LEN_BIT) == EXTEND_LEN_BIT ) 
	{
		real_len = ((unsigned short)(data[LEN_FIELD] << 8) + data[EXT_LEN_FIELD]) ;
		check_len =  real_len*2 + 6;
		data_offset = DATA1_FIELD;
	}
	else
	{
		real_len = data[LEN_FIELD];
		check_len =  real_len*2 + 4;
		data_offset = DATA0_FIELD;
	}		

	if (len != check_len) { //mark_issue ,check more?
		DEBUG_ERR("cmd length not matched [%d, %d] !\n", len, real_len);
		return -1;
	}

	*cmd_len = real_len;

	for (i=0; i<real_len; i++)	
		cmd[i] = data[i*2+data_offset];	

	return 0; //SUCCESS

}

static int pack_rsp_frame(int cmd_bad, unsigned char cmd_id, int len, unsigned char *in, unsigned char *out)
{
	int i,data_offset;
	out[TAG_FIELD] = SYNC_BIT;
	
	if (cmd_bad & BAD_CMD_RSP)
		out[TAG_FIELD] |= CMD_BAD_BIT;		
	out[CMD_FIELD] = cmd_id;
	out[2] = 0x00;
	data_offset =4 ; // data0 normal offset

	if(len < 256 ) // one byte len
		out[LEN_FIELD] = (unsigned char)len;
	else{
		out[TAG_FIELD] |= EXTEND_LEN_BIT;
		out[LEN_FIELD] = (unsigned char) ((len>>8)&0xff);
		out[4] = 0x00;
		out[EXT_LEN_FIELD] = (unsigned char) (len&0xff);
		data_offset =6;
	}
	//printf("Rsp len:%d, contend:",len);
	for (i=0; i<len; i++) {
		out[data_offset+i*2] = 0x00;
		out[data_offset+i*2+1] = in[i];
		printf("%d",in[i]);
	}
	//printf("\n");
	return (data_offset+i*2);
}

int do_cmd(int id , char *cmd ,int cmd_len ,int relply)
{
	int i=0,ret=-1,len=0;
	unsigned char rsp_packet[MAX_HOST_PKT_LEN];

	while (cmd_table[i].id != LAST_ENTRY_ID) {
		if ((cmd_table[i].id == id))	{
			ret = cmd_table[i].func(cmd,cmd_len);
			break;
		}	
		i++;
	}
	//no reply
	if(!relply)
		return ret;

	//reply rsp pkt
	if (ret >= 0) { 
		if (ret == 0) { 
			cmd[0] = '\0';
			ret = 1;
		}
		len = pack_rsp_frame(GOOD_CMD_RSP, (unsigned char)id, ret, cmd, rsp_packet);		
#ifdef CONFIG_SYSTEM_MII_INBAND_CTL
		inband_write_data(rsp_packet, len);
#else
		mdio_write_data(rsp_packet, len);
#endif
	}
	else{ //error rsp		
		cmd[0] = (unsigned char)( ~ret + 1);			
		len = pack_rsp_frame(BAD_CMD_RSP, (unsigned char)id, 1, cmd, rsp_packet);
#ifdef CONFIG_SYSTEM_MII_INBAND_CTL
		inband_write_data(rsp_packet, len);
#else		
		mdio_write_data(rsp_packet, len);
#endif
	}			
	
	return ret;
}


static int _cfgwrite(char *cmd , int cmd_len)
{
	char *param,*val;	
	int ret = RET_OK,access_flag=ACCESS_MIB_SET | ACCESS_MIB_BY_NAME;
	char *intf, *tmp;

	/*
	intf = strchr(cmd,' ');
	tmp = (char *)malloc(intf-cmd)+1;
	memset(tmp,0,intf-cmd+1);
	memcpy(tmp, cmd, intf-cmd);
	tmp[intf-cmd] = '\0';
	intf = tmp;
	cmd = strchr(cmd,' ')+1;

	param = cmd;
	val = strchr(cmd,'=');
	if(val) //find it
	{
		*val ='\0';
		val = val + 1;
	}
	else
		return -1;
	*/
	intf = cmd;
	param = strchr(intf,' ')+1;
	val = strchr(param,'=')+1;
	intf[param-intf-1] = '\0';
	param[val-param-1] = '\0';

	//printf(">>> set %s=%s to %s\n",param,val,intf);
	ret = access_config_mib(access_flag,param,val,intf);  //ret the

	//free(intf);
	return ret;
}

static int _cfgread(char *cmd , int cmd_len)
{
	char *param;	
	int ret = RET_OK,access_flag=ACCESS_MIB_GET | ACCESS_MIB_BY_NAME;
	char *intf, *tmp;

	/*
	intf = strchr(cmd,' ');
	tmp = (char *)malloc(intf-cmd)+1;
	memset(tmp,0,intf-cmd+1);
	memcpy(tmp, cmd, intf-cmd);
	tmp[intf-cmd] = '\0';
	intf = tmp;
	cmd = strchr(cmd,' ')+1;

	param = cmd;
	*/
	intf = cmd;
	param = strchr(intf,' ')+1;
	intf[param-intf-1] = '\0';
	//printf(">>> read %s from %s\n",param,intf);
	ret = access_config_mib(access_flag,param,cmd,intf);  //return value in cmd

	//free(intf);
	return ret;
}

static int _sysinit(char *cmd , int cmd_len)
{
	//now , only support init all
	if(!strcmp(cmd, "all"))
		init_system(INIT_ALL);		
	else if(!strcmp(cmd, "lan"))		
		init_system(INIT_ALL);
	else if(!strcmp(cmd, "wlan"))
		init_system(INIT_ALL);
	else
		return -1;

	return 0;
}
//sta_info frame , {sta_num}{sta_info1}{sta_info2}........... 
//first byre will be total sta_info numer in this reply
static int _getstainfo(char *cmd , int cmd_len)
{
	char *buff,*tmpInfo;	
	WLAN_STA_INFO_Tp pInfo;
	int sta_num =0,i,ret;
	
	buff = calloc(1, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1));
	tmpInfo = cmd +1 ; // first byte reserve for sta_info num

	if ( buff == 0 ) {
		printf("Allocate buffer failed!\n");
		return -1;
	}
	//buf < MAX_HOST_CMD_LEN   //mark_issue
	if ( get_wlan_stainfo(cmd,  (WLAN_STA_INFO_Tp)buff ) < 0 ) {
		printf("Read wlan sta info failed!\n");

		return -1;
	}

	for (i=1; i<=MAX_STA_NUM; i++) {
		pInfo = (WLAN_STA_INFO_Tp)&buff[i*sizeof(WLAN_STA_INFO_T)];
		if (pInfo->aid && (pInfo->flag & STA_INFO_FLAG_ASOC)) {//sta exist
			memcpy(tmpInfo+ (sta_num*sizeof(WLAN_STA_INFO_T)),(char *)pInfo, sizeof(WLAN_STA_INFO_T));
			sta_num++;
		}		
	}	

	cmd[0]= (unsigned char)(sta_num&0xff);
	ret = sta_num*sizeof(WLAN_STA_INFO_T) + 1;		
	
	return ret;
}
static int _getassostanum(char *cmd , int cmd_len)
{
	int num=0;

	if (get_wlan_stanum(cmd, &num) < 0)
		return -1;

	cmd[0]=(unsigned char)(num&0xff);	

	return 1; // return len=1  to show sta num
	
}
static int _getbssinfo(char *cmd , int cmd_len)
{	
	WLAN_BSS_INFO_T bss;
	int bss_len=sizeof(WLAN_BSS_INFO_T);

	if ( get_wlan_bssinfo(cmd, &bss) < 0)
			return -1;

	memcpy(cmd,(char *)&bss,bss_len);
	
	return bss_len;
}
static int _getwdsinfo(char *cmd , int cmd_len)
{
	//TBD
	return -1;
}

int get_lanport_stats(int portnum,struct port_statistics *port_stats)
{
	struct ifreq ifr;
	 int sockfd;
	 char *name="eth0";	 
	 struct port_statistics stats;
	 unsigned int *args;	

	 if(portnum > 5)
	 	return -1;	 	
	 
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
 	{
      		printf("fatal error socket\n");
      		return -3;
       }
	args = (unsigned int *)&stats;
	((unsigned int *)(&ifr.ifr_data))[0] =(struct port_statistics *)&stats;
	*args = portnum;
	
	strcpy((char*)&ifr.ifr_name, name);       

    if (ioctl(sockfd, RTL819X_IOCTL_READ_PORT_STATS, &ifr)<0)
    {
      		printf("device ioctl:");
      		close(sockfd);
     		 return -1;
     }
     close(sockfd);   	     
     memcpy(port_stats,(char *)&stats,sizeof(struct port_statistics));

    return 0;	 

}

int get_lanport_status(int portnum,struct lan_port_status *port_status)
{
	struct ifreq ifr;
	 int sockfd;
	 char *name="eth0";	 
	 struct lan_port_status status;
	 unsigned int *args;	

	 if(portnum > 5)
	 	return -1;	 	
	 
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
 	{
      		printf("fatal error socket\n");
      		return -3;
       }
	args = (unsigned int *)&status;
	((unsigned int *)(&ifr.ifr_data))[0] =(struct lan_port_status *)&status;
	*args = portnum;
	
	strcpy((char*)&ifr.ifr_name, name);       

    if (ioctl(sockfd, RTL819X_IOCTL_READ_PORT_STATUS, &ifr)<0)
    {
      		printf("device ioctl:");
      		close(sockfd);
     		 return -1;
     }
     close(sockfd);   	
     memcpy((char *)port_status,(char *)&status,sizeof(struct lan_port_status));

    return 0;	 

}

static int get_dev_fields(int type, char *bp, struct user_net_device_stats *pStats)
{
    switch (type) {
    case 3:
	sscanf(bp,
	"%Lu %Lu %lu %lu %lu %lu %lu %lu %Lu %Lu %lu %lu %lu %lu %lu %lu",
	       &pStats->rx_bytes,
	       &pStats->rx_packets,
	       &pStats->rx_errors,
	       &pStats->rx_dropped,
	       &pStats->rx_fifo_errors,
	       &pStats->rx_frame_errors,
	       &pStats->rx_compressed,
	       &pStats->rx_multicast,

	       &pStats->tx_bytes,
	       &pStats->tx_packets,
	       &pStats->tx_errors,
	       &pStats->tx_dropped,
	       &pStats->tx_fifo_errors,
	       &pStats->collisions,
	       &pStats->tx_carrier_errors,
	       &pStats->tx_compressed);
	break;

    case 2:
	sscanf(bp, "%Lu %Lu %lu %lu %lu %lu %Lu %Lu %lu %lu %lu %lu %lu",
	       &pStats->rx_bytes,
	       &pStats->rx_packets,
	       &pStats->rx_errors,
	       &pStats->rx_dropped,
	       &pStats->rx_fifo_errors,
	       &pStats->rx_frame_errors,

	       &pStats->tx_bytes,
	       &pStats->tx_packets,
	       &pStats->tx_errors,
	       &pStats->tx_dropped,
	       &pStats->tx_fifo_errors,
	       &pStats->collisions,
	       &pStats->tx_carrier_errors);
	pStats->rx_multicast = 0;
	break;

    case 1:
	sscanf(bp, "%Lu %lu %lu %lu %lu %Lu %lu %lu %lu %lu %lu",
	       &pStats->rx_packets,
	       &pStats->rx_errors,
	       &pStats->rx_dropped,
	       &pStats->rx_fifo_errors,
	       &pStats->rx_frame_errors,

	       &pStats->tx_packets,
	       &pStats->tx_errors,
	       &pStats->tx_dropped,
	       &pStats->tx_fifo_errors,
	       &pStats->collisions,
	       &pStats->tx_carrier_errors);
	pStats->rx_bytes = 0;
	pStats->tx_bytes = 0;
	pStats->rx_multicast = 0;
	break;
    }
    return 0;
}

static char *get_name(char *name, char *p)
{
    while (isspace(*p))
	p++;
    while (*p) {
	if (isspace(*p))
	    break;
	if (*p == ':') {	/* could be an alias */
	    char *dot = p, *dotname = name;
	    *name++ = *p++;
	    while (isdigit(*p))
		*name++ = *p++;
	    if (*p != ':') {	/* it wasn't, backup */
		p = dot;
		name = dotname;
	    }
	    if (*p == '\0')
		return NULL;
	    p++;
	    break;
	}
	*name++ = *p++;
    }
    *name++ = '\0';
    return p;
}


void fill_statistics(struct user_net_device_stats *pStats, struct port_statistics *statistics)
{
	statistics->rx_bropkts = 0;
	statistics->rx_bytes = pStats->rx_bytes;
	statistics->rx_discard = pStats->rx_dropped;
	statistics->rx_error = pStats->rx_errors;
	statistics->rx_mulpkts = pStats->rx_multicast;
	statistics->rx_unipkts = pStats->rx_packets-pStats->rx_multicast;
	statistics->tx_bropkts = 0;
	statistics->tx_bytes = pStats->tx_bytes;
	statistics->tx_discard = pStats->tx_dropped;
	statistics->tx_error = pStats->tx_errors;
	statistics->tx_mulpkts = 0;
	statistics->tx_unipkts = pStats->tx_packets;
}


int get_wlan_stats(char *intf, struct port_statistics *statistic)
{
 	FILE *fh;
  	char buf[512];
	int type;
	struct user_net_device_stats pStats;

	fh = fopen(_PATH_PROCNET_DEV, "r");
	if (!fh) {
		printf("Warning: cannot open %s\n",_PATH_PROCNET_DEV);
		return -1;
	}
	fgets(buf, sizeof buf, fh);	/* eat line */
	fgets(buf, sizeof buf, fh);

  	if (strstr(buf, "compressed"))
		type = 3;
	else if (strstr(buf, "bytes"))
		type = 2;
	else
		type = 1;

	while (fgets(buf, sizeof buf, fh)) {
		char *s, name[40];
		s = get_name(name, buf);
		if ( strcmp(intf, name))
			continue;
		get_dev_fields(type, s, &pStats);
		fill_statistics(&pStats,statistic);
		fclose(fh);
		return 0;
    }
	fclose(fh);
	return -1;
}


int portname_to_num(char *name)
{
	int portnum=0;
	if(!strncmp(name,"p0",2)) 
		portnum=0;
	else if(!strncmp(name,"p1",2)) 
		portnum=1;
	else if(!strncmp(name,"p2",2)) 
		portnum=2;
	else if(!strncmp(name,"p3",2)) 
		portnum=3;
	else if(!strncmp(name,"p4",2)) 
		portnum=4;
	else if(!strncmp(name,"p5",2)) 
		portnum=5;

	return portnum;
}

static int _getlanstatus(char *cmd , int cmd_len)
{
	struct lan_port_status port_status;
	int len=sizeof(struct lan_port_status);
	int portnum;

	portnum = portname_to_num(cmd);
	
	if ( get_lanport_status(portnum, &port_status) < 0)
			return -1;
	
	memcpy(cmd,(char *)&port_status,len);
	
	return len;
}

static int _getstats(char *cmd , int cmd_len)
{
	struct port_statistics statistics;
	int len=sizeof(struct port_statistics);
	int portnum;
	char *intf, *tmp;

	//printf(">>> %s\n",cmd);
	
	if(!strncmp(cmd,"p",1)) {
		portnum = portname_to_num(cmd);
		
		if ( get_lanport_stats(portnum, &statistics) < 0)
				return -1;		
		memcpy(cmd,(char *)&statistics,len);
	} else {
		//get statistics of wlan
		if ( get_wlan_stats(cmd, &statistics) < 0)
				return -1;

		memcpy(cmd,(char *)&statistics,len);
	}
	
	return len;
}


//-------------------------------------------------------------------

void print_stainfo(char *stainfo_rsp)
{
	WLAN_STA_INFO_Tp pInfo;
	int i=0,rateid=0,sta_num=stainfo_rsp[0];
	char mode_buf[20],txrate[20];


	if(sta_num <= 0)
		printf("No Associated  station now!!!!\n ",i+1);
	for(i=0;i<sta_num;i++)
	{
		pInfo = (WLAN_STA_INFO_T *)&(stainfo_rsp[i*sizeof(WLAN_STA_INFO_T)+1]);
		printf("-----------------------------------------------\n");
		printf("station No.%d info\n",i+1);
		printf("MAC address : %02x:%02x:%02x:%02x:%02x:%02x \n",pInfo->addr[0],pInfo->addr[1],pInfo->addr[2],pInfo->addr[3],pInfo->addr[4],pInfo->addr[5]);

		if(pInfo->network & BAND_11N)
			sprintf(mode_buf, "%s", " 11n");
		else if (pInfo->network & BAND_11G)
			sprintf(mode_buf,"%s",  " 11g");	
		else if (pInfo->network & BAND_11B)
			sprintf(mode_buf, "%s", " 11b");
		else if (pInfo->network& BAND_11A)
			sprintf(mode_buf, "%s", " 11a");	
		else
			sprintf(mode_buf, "%s", " ---");	

		printf("Mode:%s \n",mode_buf);

		printf("TX packets:%d , RX packets:%d \n",pInfo->tx_packets, pInfo->rx_packets);

		if((pInfo->txOperaRates & 0x80) != 0x80){	
			if(pInfo->txOperaRates%2){
				sprintf(txrate, "%d%s",pInfo->txOperaRates/2, ".5"); 
			}else{
				sprintf(txrate, "%d",pInfo->txOperaRates/2); 
			}
		}else{
			if((pInfo->ht_info & 0x1)==0){ //20M
				if((pInfo->ht_info & 0x2)==0){//long
					for(rateid=0; rateid<16;rateid++){
						if(rate_11n_table_20M_LONG[rateid].id == pInfo->txOperaRates){
							sprintf(txrate, "%s", rate_11n_table_20M_LONG[rateid].rate);
							break;
						}
					}
				}else if((pInfo->ht_info & 0x2)==0x2){//short
					for(rateid=0; rateid<16;rateid++){
						if(rate_11n_table_20M_SHORT[rateid].id == pInfo->txOperaRates){
							sprintf(txrate, "%s", rate_11n_table_20M_SHORT[rateid].rate);
							break;
						}
					}
				}
			}else if((pInfo->ht_info & 0x1)==0x1){//40M
				if((pInfo->ht_info & 0x2)==0){//long
					
					for(rateid=0; rateid<16;rateid++){
						if(rate_11n_table_40M_LONG[rateid].id == pInfo->txOperaRates){
							sprintf(txrate, "%s", rate_11n_table_40M_LONG[rateid].rate);
							break;
						}
					}
				}else if((pInfo->ht_info & 0x2)==0x2){//short
					for(rateid=0; rateid<16;rateid++){
						if(rate_11n_table_40M_SHORT[rateid].id == pInfo->txOperaRates){
							sprintf(txrate, "%s", rate_11n_table_40M_SHORT[rateid].rate);
							break;
						}
					}
				}
			}
		   }
		printf("TX Rate : %s \n",txrate);
		printf("Sleep : %s \n",( (pInfo->flag & STA_INFO_FLAG_ASLEEP) ? "yes" : "no"));
		printf("Expired time : %d seconds \n",pInfo->expired_time/100);

	}

}

void print_bssinfo(char *bssinfo_rsp)
{
	WLAN_BSS_INFO_Tp bssInfo;
	char *pMsg;
	
	bssInfo = (WLAN_BSS_INFO_T *)bssinfo_rsp;
	printf("BSSID : %02x:%02x:%02x:%02x:%02x:%02x \n",bssInfo->bssid[0],bssInfo->bssid[1],bssInfo->bssid[2],
													bssInfo->bssid[3],bssInfo->bssid[4],bssInfo->bssid[5]);
	printf("SSID : %s \n",bssInfo->ssid);

	switch (bssInfo->state) {
		case STATE_DISABLED:
			pMsg = "Disabled";
			break;
		case STATE_IDLE:
			pMsg = "Idle";
			break;
		case STATE_STARTED:
			pMsg = "Started";
			break;
		case STATE_CONNECTED:
			pMsg = "Connected";
			break;
		case STATE_WAITFORKEY:
			pMsg = "Waiting for keys";
			break;
		case STATE_SCANNING:
			pMsg = "Scanning";
			break;
		default:
			pMsg=NULL;
		}

	printf("State : %s \n",pMsg);

	printf("Channel : %d \n",bssInfo->channel);		
	
}

void print_port_status(char *status)
{
	struct lan_port_status *port_status;
	
	port_status = (struct lan_port_status *)status;
	
	printf("Link = %s\n",lan_link_spec[port_status->link]);	
	printf("Speed = %s\n",lan_speed_spec[port_status->speed]);
	printf("Nway mode = %s\n",enable_spec[port_status->nway]);	
	printf("Duplex = %s\n",enable_spec[port_status->duplex]);
		
}
void print_port_stats(char *stats)
{
	struct port_statistics *port_stats;
	port_stats = (struct port_statistics *)stats;

	printf("rx bytes=%d\n",port_stats->rx_bytes);
	printf("rx unicast packets =%d\n",port_stats->rx_unipkts);
	printf("rx multicast packets =%d\n",port_stats->rx_mulpkts);
	printf("rx brocast packets =%d\n",port_stats->rx_bropkts);
	printf("rx discard packets  =%d\n",port_stats->rx_discard);
	printf("rx error packets  =%d\n",port_stats->rx_error);
	printf("tx bytes=%d\n",port_stats->tx_bytes);
	printf("tx unicast packets =%d\n",port_stats->tx_unipkts);
	printf("tx multicast packets =%d\n",port_stats->tx_mulpkts);
	printf("tx brocast packets =%d\n",port_stats->tx_bropkts);
	printf("tx discard packets  =%d\n",port_stats->tx_discard);
	printf("tx error packets  =%d\n",port_stats->tx_error);	
}

