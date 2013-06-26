/*
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "sysconf.h"
#include "sys_utility.h"



#include "../apmib/apmib.h"



#define DHCPD_PID_FILE "/var/run/udhcpd.pid"

static int isDaemon=0;
static int WanType=0;
static int ConnectType=0;

#ifdef MULTI_PPPOE
extern int apmib_get(int id, void *value);

// input:process name output: 1--exist 0---not exist
int IsExistProcess(char * proName)
{
	char command[50];
	int pronum = -1;
	FILE *pF;
	sprintf(command,"ps -ef|grep \"%s\" | grep -v grep | wc -l > /tmp/proNum",proName);
	system(command);
	if((pF = fopen("/tmp/proNum","r"))==NULL)
	{
		printf("can't open the file\n");
		return 0;
	}	
	fscanf(pF,"%d",&pronum);
	close(pF);
	system("rm /tmp/proNum >/dev/null 2>&1");
	if(pronum == 0)
		return 0;
	else if(pronum >= 1)
		return 1;
}

int NeedCreate(char* order)
{
	PPP_CONNECT_TYPE_T type;
	if(!strcmp(order,"first")){
		if(!apmib_get( MIB_PPP_CONNECT_TYPE, (void *)&type))
			return 0;			
		if(type ==CONTINUOUS){
			return 1;
		}else if(type == CONNECT_ON_DEMAND){
			return 1;
		}else if(type == MANUAL){
			if(isFileExist("/etc/ppp/connfile1")){
				system("rm /etc/ppp/connfile1 >/dev/null 2>&1");
				return 1;
			}else if(isFileExist("/etc/ppp/disconnect_trigger1")){
				return 0;
			}			
		}
	}else if(!strcmp(order,"second")){
		if(!apmib_get( MIB_PPP_CONNECT_TYPE2, (void *)&type))
			return 0;	
		if(type ==CONTINUOUS){
			return 1;
		}else if(type == CONNECT_ON_DEMAND){
			return 1;
		}else if(type == MANUAL){
			if(isFileExist("/etc/ppp/connfile2")){
				system("rm /etc/ppp/connfile2 >/dev/null 2>&1");
				return 1;
			}else if(isFileExist("/etc/ppp/disconnect_trigger2")){
				return 0;
			}		
		}		
	}else if(!strcmp(order,"third")){
		if(!apmib_get( MIB_PPP_CONNECT_TYPE3, (void *)&type))
			return 0;	
		if(type ==CONTINUOUS){
			return 1;
		}else if(type == CONNECT_ON_DEMAND){
			return 1;
		}else if(type == MANUAL){
			if(isFileExist("/etc/ppp/connfile3")){
				system("rm /etc/ppp/connfile3 >/dev/null 2>&1");
				return 1;
			}else if(isFileExist("/etc/ppp/disconnect_trigger3")){
				return 0;
			}			
		}		
	}else if(!strcmp(order,"forth")){
		if(!apmib_get( MIB_PPP_CONNECT_TYPE4, (void *)&type))
			return 0;	
		if(type ==CONTINUOUS){
			return 1;
		}else if(type == CONNECT_ON_DEMAND){
			return 1;
		}else if(type == MANUAL){
			if(isFileExist("/etc/ppp/connfile4")){
				system("rm /etc/ppp/connfile4 >/dev/null 2>&1");
				return 1;
			}else if(isFileExist("/etc/ppp/disconnect_trigger4")){
				return 0;
			}				
		}		
	}
	return 0;
}
#endif

int main(int argc, char *argv[])
{
	int i;
	int cnt;//patch for l2tp dial-on-demand wantype
#ifdef MULTI_PPPOE
	FILE * pF =NULL;
	int index = -1;
	char command[100];
	int connectNumber ;
	int cur_conn_num = -1;
	apmib_init();
#endif

	for(i=1; i<argc; i++)
	{
		if(argv[i][0]!='-')
		{
			fprintf(stderr, "%s: Unknown option\n", argv[i]);
		}
		else switch(argv[i][1])
		{
		case 'c':
			ConnectType = atoi(argv[++i]);
			break;
		case 't':
			WanType = atoi(argv[++i]);
			break;
		case 'x':
			isDaemon = 1;
			break;
		default:
			fprintf(stderr, "%s: Unknown option\n", argv[i]);
		}
	}

	if(isDaemon==1){
		if (daemon(0, 1) == -1) {
			perror("ppp_inet fork error");
			return 0;
		}
	}

	cnt=0;//patch for l2tp dial-on-demand wantype
	for (;;) {
		if((isFileExist(PPP_CONNECT_FILE)==0) && (isFileExist(PPP_PATCH_FILE)==0)){			
			int enable_dhcpd = 0;
			sleep(3);	//To avoid ppp1
			if(isFileExist(DHCPD_PID_FILE)==0)
			{
				enable_dhcpd = 1;
				system("killall -9 udhcpd 2> /dev/null");
				system("rm -f /var/run/udhcpd.pid 2> /dev/null");
			}
			if(WanType==3){
				if(isFileExist("/var/disc")==0){
					
					//RunSystemCmd(PPP_CONNECT_FILE, "echo", "pass", NULL_STR);
					#ifdef MULTI_PPPOE						
						apmib_get(MIB_PPP_CONNECT_COUNT, (void *)&connectNumber);
						system("ifconfig |grep 'ppp'| cut -d ' ' -f 1 |  wc -l > /etc/ppp/lineNumber");
						if(connectNumber >= 1 && isFileExist("/etc/ppp/link")==0 
							&& !IsExistProcess("pppd -1") && NeedCreate("first"))
						{
							system("pppd -1 &");
						}
						if(connectNumber >= 2 && isFileExist("/etc/ppp/link2")==0
							&& !IsExistProcess("pppd -2") && NeedCreate("second"))
						{				
							system("pppd -2 &");
						}
						if(connectNumber >= 3 && isFileExist("/etc/ppp/link3")==0
							&& !IsExistProcess("pppd -3") && NeedCreate("third"))
						{				
							system("pppd -3 &");
						}
						if(connectNumber >= 4 && isFileExist("/etc/ppp/link4")==0
							&& !IsExistProcess("pppd -4") && NeedCreate("forth"))
						{
							system("pppd -4 &");
						}							
						
						if((pF=fopen("/etc/ppp/lineNumber","r+")) != NULL){							
							fscanf(pF,"%d",&cur_conn_num);
							if(cur_conn_num == connectNumber)
								RunSystemCmd(PPP_CONNECT_FILE, "echo", "pass", NULL_STR);
						}											
					#else
						RunSystemCmd(PPP_CONNECT_FILE, "echo", "pass", NULL_STR);
						system("pppd &");
					#endif
				}
			}

			if(WanType==4){
				if(isFileExist("/var/disc")==0){
					RunSystemCmd(PPP_CONNECT_FILE, "echo", "pass", NULL_STR);
					system("pppd call rpptp &");
				}
			}

			if(WanType==6){
				if(isFileExist("/var/disc")==0){
#if defined(CONFIG_DYNAMIC_WAN_IP)
					usleep(1200000); //wait l2tpd init finish
#endif
					RunSystemCmd(PPP_CONNECT_FILE, "echo", "pass", NULL_STR);
					system("echo \"c client\" > /var/run/l2tp-control &");
				}
			}
        #ifdef RTK_USB3G
            if(WanType==16){
                if(isFileExist("/var/disc")==0){
                    RunSystemCmd(PPP_CONNECT_FILE, "echo", "pass", NULL_STR);
                    system("pppd file /var/usb3g.option &");
                }
            }
        #endif /* #ifdef RTK_USB3G */
        if(enable_dhcpd)
					system("udhcpd \"/var/udhcpd.conf\"");
		}else{
			if(WanType==6 && ConnectType==1){
				if(isFileExist(PPPD_PID_FILE)==0){

					//patch for l2tp dial-on-demand wantype
					//after 3 times, restart l2tpd
					if(cnt<3){
						cnt++;
					}else{
						RunSystemCmd(NULL_FILE, "killall", "-9", "l2tpd", NULL_STR);
						sleep(1);
						system("l2tpd &");
					}

			  		unlink(PPP_CONNECT_FILE); /*force start pppd*/
	  			}
	  		}
  		}

		if(ConnectType==2)
			break;
		sleep(5);
	}
	return 0;
}



