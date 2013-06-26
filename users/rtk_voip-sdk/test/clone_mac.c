#include "voip_manager.h"
#include<stdio.h>
static unsigned int Value(char c);
int clone_mac_main(int argc, char* arg[])
{
	unsigned char MAC[6] ={0};
	int i = 0;
	unsigned char *MAC_ADD;
	
	if(argc != 2)
	{
		printf("wrong use of clone_mac: clone_mac MAC_ADDRESS\n");
		return 1;
	}
	MAC_ADD = (unsigned char*)arg[1];
	
	for(i = 0 ; i< 12; i+=2)
	{
		if(Value(MAC_ADD[i])==-1 || Value(MAC_ADD[i+1])==-1 )
		{
			printf("wrong MAC_ADDRESS\n");	
			return 1;
		}
			MAC[i/2] = ( Value(MAC_ADD[i]) << 4) + Value(MAC_ADD[i+1]);
	}
	//printf("MAC: %x%x%x%x%x%x\n", MAC[0],MAC[1],MAC[2],MAC[3],MAC[4],MAC[5]);
	rtk_WAN_Clone_MAC(MAC);
	return 0;
}
unsigned int Value(char c){
	     if(c == '0') return 0;
	else if(c == '1') return 1;
	else if(c == '2') return 2;
	else if(c == '3') return 3;
	else if(c == '4') return 4;
	else if(c == '5') return 5;
	else if(c == '6') return 6;
	else if(c == '7') return 7;
	else if(c == '8') return 8;
	else if(c == '9') return 9;
	else if(c == 'a' || c == 'A')	return 10;
	else if(c == 'b' || c == 'B')	return 11;
	else if(c == 'c' || c == 'C')	return 12;
	else if(c == 'd' || c == 'D')	return 13;
	else if(c == 'e' || c == 'E')	return 14;
	else if(c == 'f' || c == 'F')	return 15;
	else 
		return -1;	
}
