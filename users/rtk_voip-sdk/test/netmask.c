#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int netmask_main(int argc , char*argv[])
{
	struct in_addr ip, netmask, result;
	unsigned long ca;
	
	inet_aton(argv[1],&ip);
	inet_aton(argv[2],&netmask);
	result.s_addr = ip.s_addr & netmask.s_addr;

	ca= netmask.s_addr;
	
	int i = 0;
	while(ca!=0 && i++<33)
		ca <<= 1;
	
	printf("%s/%d\n ",inet_ntoa(result) ,i);	

	return 0;
}

