/* 
 */
 
#include <stdio.h>
#include <unistd.h>
#include "sysconf.h"
#include "sys_utility.h"
    
#define FIRSTDDNS "/var/firstddns"

static int isDaemon=0;



int main(int argc, char *argv[])
{
	int i;
	unsigned char command[100];
	//unsigned short fail_wait_time = 300;
	//unsigned int succ_wait_time = 86400;
	unsigned char ddns_type[10];
	unsigned char ddns_domanin_name[51];
	unsigned char ddns_user_name[51];
	unsigned char ddns_password[51];

	for(i=1; i<argc; i++)
	{
		if(argv[i][0]!='-')
		{
			fprintf(stderr, "%s: Unknown option\n", argv[i]);
		}
		else 
			switch(argv[i][1])
			{
				case 'x':
					isDaemon = 1;
					break;
				
				default:
					fprintf(stderr, "%s: Unknown option\n", argv[i]);
			}
	}

	sprintf((char *)ddns_type, "%s", argv[2]);
	sprintf((char *)ddns_user_name, "%s", argv[3]);
	sprintf((char *)ddns_password, "%s", argv[4]);
	sprintf((char *)ddns_domanin_name, "%s", argv[5]);

	sprintf((char *)command, "%s:%s", ddns_user_name, ddns_password);
	if(isDaemon==1){
		if (daemon(0, 1) == -1) {
			perror("ntp_inet fork error");
			return 0;
		}
	}
	
	RunSystemCmd(FIRSTDDNS, "echo", "pass", NULL_STR);
	for (;;) {
		//unsigned char cmdBuffer[100];
		int ret;

		ret = RunSystemCmd(NULL_FILE, "updatedd", ddns_type, command, ddns_domanin_name, NULL_STR);

		if(ret == 0) // success
		{
			RunSystemCmd(NULL_FILE, "echo", "DDNS update successfully", NULL_STR);
			sleep(86430);
		}
		else // fail
		{
			sleep(300);
		}
		
	}
	
	
	return 0;
}



