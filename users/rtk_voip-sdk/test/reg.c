#include "voip_manager.h"
#include <string.h>

#ifndef MIN
#define MIN(x,y) (x>y?y:x)
#endif
int hex_char_to_nibble(unsigned char c) {
  switch(c) {
  case ('0'): return 0x0;
  case ('1'): return 0x1;
  case ('2'): return 0x2;
  case ('3'): return 0x3;
  case ('4'): return 0x4;
  case ('5'): return 0x5;
  case ('6'): return 0x6;
  case ('7'): return 0x7;
  case ('8'): return 0x8;
  case ('9'): return 0x9;
  case ('a'): return 0xa;
  case ('A'): return 0xa;
  case ('b'): return 0xb;
  case ('B'): return 0xb;
  case ('c'): return 0xc;
  case ('C'): return 0xc;
  case ('d'): return 0xd;
  case ('D'): return 0xd;
  case ('e'): return 0xe;
  case ('E'): return 0xe;
  case ('f'): return 0xf;
  case ('F'): return 0xf;
  default: return -1;   /* this flags an error */
  }
  /* NOTREACHED */
  return -1;  /* this keeps compilers from complaining */
}

int hex_string_to_octet_string(char *raw, char *hex, int len) {
  unsigned char x;
  int tmp;

    tmp = hex_char_to_nibble(hex[0]);

    if (tmp == -1) 
        return -1; 

    if (len<=1) {
        *raw = tmp;
        return 0;
    }   
    x = (tmp << 4); 
    tmp = hex_char_to_nibble(hex[1]);

    if (tmp == -1) 
        return -1; 

    x |= tmp;
    *raw = x;

  return 0;
}

int reg_main(int argc, char *argv[])
{
	int i, len;
	unsigned char val;
	unsigned char *ptr;
	unsigned char regdata[16];
	
	if (argc == 3)
	{
		int chid, reg;
		int reg_start, reg_end;

		chid = atoi(argv[1]);

		if (argv[2][0] == 'r')
		{
			rtk_reset_slic(chid, 1); // 1: set to A-law
			printf("reset slic %d done\n", chid);
		}
		else
		{
			ptr = argv[2];
			if (strlen(ptr)>2 && *ptr=='0' && *(ptr+1)=='x') {
				ptr+=2; //skip 0x
				hex_string_to_octet_string(&val, ptr, strlen(ptr)); 
				reg = val;
			} else {
				reg = atoi(argv[2]);
			}
			
			if( reg < 0 ) {
				reg_start = 0;
				reg_end = reg * ( -1 );
			} else {
				reg_start = reg_end = reg;
			}
			
			while( reg_start <= reg_end ) {
			
				len = rtk_Get_SLIC_Reg_Val(chid, reg_start, &regdata);
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_SILAB
				if ((reg == 32260) || (reg == 32261)) {
					; //do nothing
				}else {
					printf("read: chid = %d, reg =%d, val = 0x%02X\n", chid, reg_start, regdata[0] );
				}
#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK
				if (len>0) {
					printf("read: chid = %d, reg =0x%02X (%d)\n", chid, reg_start, reg_start);
	
					for (i=0;i<len;i++)
						printf("val[%d] = 0x%02X (%d)\n",i , regdata[i], regdata[i]);
				}
#endif
				reg_start ++;
			} // while 
		}
	}
	else if (argc >= 4)
	{
		int chid, reg, val;

		chid = atoi(argv[1]);
		reg = atoi(argv[2]);
		val = atoi(argv[3]);
		len = argc-3;

		/* register */

		ptr = argv[2];
		if (strlen(ptr)>2 && *ptr=='0' && *(ptr+1)=='x') {
			ptr+=2; //skip 0x
			hex_string_to_octet_string(&val, ptr, strlen(ptr)); 
			reg = val;
		} else {
			reg = atoi(argv[2]);
		}

		/* value */
		for (i=0; i<MIN((argc-3),16);i++) {
	
			ptr = argv[i+3];
			if (strlen(ptr)>2 && *ptr=='0' && *(ptr+1)=='x') {
				ptr+=2; //skip 0x
				hex_string_to_octet_string(&regdata[i], ptr, strlen(ptr)); 
			} else {
				regdata[i] = atoi(argv[i+3]);
			}
		}

		rtk_Set_SLIC_Reg_Val(chid, reg, len, &regdata);

#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_SILAB
		printf("write: chid = %d, reg = %d, val = 0x%02X\n", 
			chid, reg, regdata[0]);
#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK
		printf("write: chid = %d, reg = 0x%02X (%d)\n",chid, reg, reg);
		for (i=0;i<len;i++)
			printf("val[%d] = 0x%02X (%d)\n",i , regdata[i], regdata[i]);

#endif
	}
	else
	{
		printf("Usage: %s chid reg_num [reg_val]\n", argv[0]);
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_SILAB
		printf("If you want to enter(leave) user mode for Si3226,\n");
		printf("you can use: reg chid 32261(32260)\n");
#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK
		printf("use reg 0 <880|890> to dump Zarlink register\n");
#endif
	}

	return 0;
}

