#include <stdio.h> 
#include <stdlib.h> 
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/wireless.h>

#define PROGRAM_NAME "Realtek site_survey app v1.0"
char WLAN_IF[]="wlan0";
#define SIOCGIWRTLSCANREQ		0x8B33	// scan request
#define SIOCGIWRTLGETBSSDB		0x8B34	// get bss data base

#define SSID_LEN	32
#define MESHID_LEN 32 
#define	MAX_BSS_DESC	64

typedef enum { BAND_11B=1, BAND_11G=2, BAND_11BG=3, BAND_11A=4, BAND_11N=8 } BAND_TYPE_T;

typedef enum _BssType {
    infrastructure = 1,
    independent = 2,
} BssType;

typedef enum _Capability {
    cESS 		= 0x01,
    cIBSS		= 0x02,
    cPollable		= 0x04,
    cPollReq		= 0x01,
    cPrivacy		= 0x10,
    cShortPreamble	= 0x20,
} Capability;

typedef	struct _IbssParms {
    unsigned short	atimWin;
} IbssParms;

typedef struct _OCTET_STRING {
    unsigned char *Octet;
    unsigned short Length;
} OCTET_STRING;

typedef struct _BssDscr {
    unsigned char bdBssId[6];
    unsigned char bdSsIdBuf[SSID_LEN];
    OCTET_STRING  bdSsId;

//#if defined(CONFIG_RTK_MESH) || defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198)//mark_issue	
	unsigned char bdMeshIdBuf[MESHID_LEN]; 
	OCTET_STRING bdMeshId; 
//#endif 
    BssType bdType;
    unsigned short bdBcnPer;			// beacon period in Time Units
    unsigned char bdDtimPer;			// DTIM period in beacon periods
    unsigned long bdTstamp[2];			// 8 Octets from ProbeRsp/Beacon
    IbssParms bdIbssParms;			// empty if infrastructure BSS
    unsigned short bdCap;				// capability information
    unsigned char ChannelNumber;			// channel number
    unsigned long bdBrates;
    unsigned long bdSupportRates;		
    unsigned char bdsa[6];			// SA address
    unsigned char rssi, sq;			// RSSI and signal strength
    unsigned char network;			// 1: 11B, 2: 11G, 4:11G
} BssDscr, *pBssDscr;


typedef struct _sitesurvey_status {
    unsigned char number;
    unsigned char pad[3];
    BssDscr bssdb[MAX_BSS_DESC];
} SS_STATUS_T, *SS_STATUS_Tp;


static inline int
iw_get_ext(int                  skfd,           /* Socket to the kernel */
           char *               ifname,         /* Device name */
           int                  request,        /* WE ID */
           struct iwreq *       pwrq)           /* Fixed part of the request */
{
  /* Set device name */
  strncpy(pwrq->ifr_name, ifname, IFNAMSIZ);
  /* Do the request */
  return(ioctl(skfd, request, pwrq));
}


//////////////////////////////////////////////////
static void show_help(void)
{
	printf("  Usage: %s [argument]...\n", PROGRAM_NAME);
/*
	printf("    Where arguments is optional as:\n");
	printf("\t-w WPS event listen mode\n");
	printf("\t-r Radius event listen modd\n");
	printf("\twps trigger WPS by pin/pbc if wscd exist\n");
	printf("\n");	
*/	
}

static int parse_argument(int argc, char *argv[])
{
#if 0
	int argNum=1;
	int pid;
	unsigned char line[100];
	char tmpbuf[100];
	int ip;

	while (argNum < argc) { 
		if ( !strcmp(argv[argNum], "-a")) {
			network_info->radsrvaddr.sin_addr.s_addr = inet_addr(argv[argNum]);
		} else if (!strcmp(argv[argNum], "-w")) {
			*type = 1;
		} else if (!strcmp(argv[argNum], "-r")) {
			*type = 2;
			network_info->radsrvaddr.sin_addr.s_addr = inet_addr(argv[argNum+1]);
		} else if (!strcmp(argv[argNum], "wps")) {
			show_help();
		} else {
			printf("invalid argument - %s\n", argv[argNum]);
			show_help();
			goto parse_err;
		}
		if (++argNum >= argc)
				break;
		argNum++;
	}
	return 0;
parse_err:
	return -1;
#endif	
return 0;
}

int getWlSiteSurveyRequest(char *interface, int *pStatus)
{
#ifndef NO_ACTION
    int skfd=0;
    struct iwreq wrq;
    unsigned char result;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
      /* If no wireless name : no wireless extensions */
      close( skfd );
        return -1;
	}
    wrq.u.data.pointer = (caddr_t)&result;
    wrq.u.data.length = sizeof(result);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLSCANREQ, &wrq) < 0){
    	close( skfd );
	return -1;
	}
    close( skfd );

    if ( result == 0xff )
    	*pStatus = -1;
    else
	*pStatus = (int) result;
#else
	*pStatus = -1;
#endif
#ifdef CONFIG_RTK_MESH 	
	return (int)*(char*)wrq.u.data.pointer; 
#else
	return 0;
#endif
}

int getWlSiteSurveyResult(char *interface, SS_STATUS_Tp pStatus )
{
#ifndef NO_ACTION
    int skfd=0;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;
    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
      /* If no wireless name : no wireless extensions */
      close( skfd );
        return -1;
	}
    wrq.u.data.pointer = (caddr_t)pStatus;

    if ( pStatus->number == 0 )
    	wrq.u.data.length = sizeof(SS_STATUS_T);
    else
        wrq.u.data.length = sizeof(pStatus->number);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLGETBSSDB, &wrq) < 0){
    	close( skfd );
	return -1;
	}
    close( skfd );
#else
	return -1 ;
#endif

    return 0;
}

static int site_survey_request()
{
  int wait_time;
  char errBuf[100];
  int status;
  unsigned char res;
  wait_time = 0;

	// request wlan driver to site survey 	
	while (1)
	{			
		switch(getWlSiteSurveyRequest(WLAN_IF, &status)) 
		{ 
			case -2: 
				printf("-2\n"); 
				strcpy(errBuf, "Auto scan running!!please wait..."); 
				goto ss_err; 				
			case -1: 
				printf("-2\n"); 
				strcpy(errBuf, "Site-survey request failed!"); 
				goto ss_err; 
			default: 
				break; 
		} 

		if (status != 0) {	// not ready
			if (wait_time++ > 5) {
				strcpy(errBuf, "scan request timeout!");
				goto ss_err;
			}
		sleep(1);
		}
		else
			break;
	}

	// wait until scan completely
	wait_time = 0;
	while (1) {
		res = 1;	// only request request status
		if ( getWlSiteSurveyResult(WLAN_IF, (SS_STATUS_Tp)&res) < 0 ) {
			strcpy(errBuf, "Read site-survey status failed!");
			//free(pStatus);
			//pStatus = NULL;
			goto ss_err;
		}
		if (res == 0xff) {   // in progress
			if (wait_time++ > 10) {
			strcpy(errBuf, "scan timeout!");
			//free(pStatus);
			//pStatus = NULL;
			goto ss_err;
		}
			sleep(1);
	    }
		else
			break;
	}

	return 0;
ss_err :
	printf("%s",errBuf);
	return -1;
}

static void get_ap_mode(unsigned char network,char *tmp1Buf)
{
	if (network==BAND_11B)
			strcpy(tmp1Buf, "B");
		else if (network==BAND_11G)
			strcpy(tmp1Buf, "G");	
		else if (network==(BAND_11G|BAND_11B))
			strcpy(tmp1Buf, "B+G");
		else if (network==(BAND_11N))
			strcpy(tmp1Buf, "N");		
		else if (network==(BAND_11G|BAND_11N))
			strcpy(tmp1Buf, "G+N");	
		else if (network==(BAND_11G|BAND_11B | BAND_11N))
			strcpy(tmp1Buf, "B+G+N");	
		else if(network== BAND_11A)
			strcpy(tmp1Buf, "A");	
		else if(network== (BAND_11A | BAND_11N))
			strcpy(tmp1Buf, "A+N");	
}

static void get_ap_encrypt(BssDscr *pBss,char *tmp2Buf)
{
   char  wpa_tkip_aes[20],wpa2_tkip_aes[20];

   memset(wpa_tkip_aes,0x00,sizeof(wpa_tkip_aes));
   memset(wpa2_tkip_aes,0x00,sizeof(wpa2_tkip_aes));
		
   	if ((pBss->bdCap & cPrivacy) == 0)
		sprintf(tmp2Buf, "no");
	else {
		if (pBss->bdTstamp[0] == 0)
			sprintf(tmp2Buf, "WEP");
		else {
			int wpa_exist = 0, idx = 0;
			if (pBss->bdTstamp[0] & 0x0000ffff) {
				idx = sprintf(tmp2Buf, "WPA");
				if (((pBss->bdTstamp[0] & 0x0000f000) >> 12) == 0x4)
					idx += sprintf(tmp2Buf+idx, "-PSK");
				wpa_exist = 1;

				if (((pBss->bdTstamp[0] & 0x00000f00) >> 8) == 0x5)
					sprintf(wpa_tkip_aes,"%s","(aes/tkip)"); //mark_issue , cat with tmp2Buf?
				else if (((pBss->bdTstamp[0] & 0x00000f00) >> 8) == 0x4)
					sprintf(wpa_tkip_aes,"%s","(aes)");
				else if (((pBss->bdTstamp[0] & 0x00000f00) >> 8) == 0x1)
					sprintf(wpa_tkip_aes,"%s","(tkip)");

				if(wpa_tkip_aes[0] !=0)
					strcat(tmp2Buf,wpa_tkip_aes);				

			    }
			
				if (pBss->bdTstamp[0] & 0xffff0000) {
					if (wpa_exist)
						idx += sprintf(tmp2Buf+idx, "/");
					idx += sprintf(tmp2Buf+idx, "WPA2");
					if (((pBss->bdTstamp[0] & 0xf0000000) >> 28) == 0x4)
						idx += sprintf(tmp2Buf+idx, "-PSK");

					if (((pBss->bdTstamp[0] & 0x0f000000) >> 24) == 0x5)
						sprintf(wpa2_tkip_aes,"%s","(aes/tkip)");
					else if (((pBss->bdTstamp[0] & 0x0f000000) >> 24) == 0x4)
						sprintf(wpa2_tkip_aes,"%s","(aes)");
					else if (((pBss->bdTstamp[0] & 0x0f000000) >> 24) == 0x1)
						sprintf(wpa2_tkip_aes,"%s","(tkip)");

					if(wpa2_tkip_aes[0] !=0)
					strcat(tmp2Buf,wpa2_tkip_aes);	
				}
			}
		}
}

static int get_site_survey_result()
{  
  char errBuf[100]; //per bssinfo or error msg  
  char tmpbuf[100],tmpbuf2[50]; //per bssinfo or error msg
  char ssr_result[MAX_BSS_DESC*200]; //mark_issue , use alloc ?
  SS_STATUS_Tp pStatus=NULL;
  BssDscr *pBss;
  int i;
    
  if (pStatus==NULL) {
		pStatus = calloc(1, sizeof(SS_STATUS_T));
		if ( pStatus == NULL ) {
			strcpy(errBuf, "Allocate buffer failed!\n");
			goto ssr_err_out;
		}
  }

  pStatus->number = 0; // request BSS DB

  if ( getWlSiteSurveyResult(WLAN_IF, pStatus) < 0 ) {
		strcpy(errBuf, "Read site-survey status failed!\n");
		goto ssr_err;
  }

  if(pStatus->number<0 || pStatus->number > MAX_BSS_DESC)
  {
  	strcpy(errBuf, "invalid scanned ap num!\n");
  	goto ssr_err;
  }	

  sprintf(ssr_result,"total_ap_num=%d\n",pStatus->number);  

  for (i=0; i<pStatus->number && pStatus->number!=0xff; i++) {

		//add ##########################
		strcat(ssr_result,"##########\n");

		pBss = &pStatus->bssdb[i];		
		// fill ap index
		sprintf(tmpbuf,"ap_index=%d\n",i+1);
		strcat(ssr_result,tmpbuf);

		// fill ap ssid
   		
		memcpy(tmpbuf2, pBss->bdSsIdBuf, pBss->bdSsId.Length);
		tmpbuf2[pBss->bdSsId.Length] = '\0';
		
		sprintf(tmpbuf,"ap_ssid=%s\n",tmpbuf2);		
		strcat(ssr_result,tmpbuf);
   
		// fill ap mac
		sprintf(tmpbuf,"ap_bssid=%02x:%02x:%02x:%02x:%02x:%02x\n",
			pBss->bdBssId[0], pBss->bdBssId[1], pBss->bdBssId[2],
			pBss->bdBssId[3], pBss->bdBssId[4], pBss->bdBssId[5]);
		strcat(ssr_result,tmpbuf);
		
		// fill ap mode B,G,N
		get_ap_mode(pBss->network,tmpbuf2);
		sprintf(tmpbuf,"ap_mode=%s\n",tmpbuf2);		
		strcat(ssr_result,tmpbuf);
	
		// fill ap channel
		sprintf(tmpbuf,"ap_channel=%d\n",pBss->ChannelNumber);
		strcat(ssr_result,tmpbuf);
	
		// fill ap encryption type
		get_ap_encrypt(pBss,tmpbuf2);
		sprintf(tmpbuf,"ap_encrypt=%s\n",tmpbuf2);		
		strcat(ssr_result,tmpbuf);
	
		// fill ap signal strength
		sprintf(tmpbuf,"ap_signal=%d\n",pBss->rssi);
		strcat(ssr_result,tmpbuf);

  }		

  printf("%s",ssr_result);
  free(pStatus); 
  return 0;
  
ssr_err :
	free(pStatus); 
ssr_err_out :
	printf("%s",errBuf);
	return -1;
}
int main(int argc, char *argv[])
{ 	

#if 0
	if(argc < 2) {
		show_help();
		return -1;
	}
#endif
	if( parse_argument(argc, argv) < 0)
		return -1;

	site_survey_request();
	get_site_survey_result();

	return 0;
}

