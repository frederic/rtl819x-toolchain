#include "apmib.h"
#include "mibtbl.h"
#include <signal.h>

static unsigned int time_count;

void timeout_handler() 
{
	char tmpBuf[128];
	time_count++;
	if(!(time_count%1))
	{

	}	

	if(!(time_count%60))
 	{

	}
 	alarm(1);
}

int main(int argc, char** argv)
{
	char	line[300];
	char action[16];
	int i;
	if ( !apmib_init()) {
		printf("Initialize AP MIB failed !\n");
		return -1;
	}	
	signal(SIGALRM,timeout_handler);
	alarm(1);
	while(1)
	{
		#ifdef SEND_GRATUITOUS_ARP
		checkWanStatus();
		#endif
		sleep(1);
	}
}

