#include <stdio.h> 
#include <fcntl.h> 
#include <sys/types.h> 
#include <sys/uio.h> 
#include <unistd.h> 
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include "../../../linux-2.6.30/drivers/char/rtl_nfbi/rtl_nfbi.h"
#include "nfbi_api.h"

//#define CONFIG_D8_16
//#define CONFIG_D16_16
#define CONFIG_D32_16

int nfbi_fd;

void DDR_NFBI_cali(void);

void real_sleep(unsigned int sec)
{
    unsigned int s;
    
	s = sec;
	while ((s = sleep(s)))
	    ;
}

int _atoi(char *s, int base)
{
	int k = 0;

	k = 0;
	if (base == 10) {
		while (*s != '\0' && *s >= '0' && *s <= '9') {
			k = 10 * k + (*s - '0');
			s++;
		}
	}
	else {
		while (*s != '\0') {			
			int v;
			if ( *s >= '0' && *s <= '9')
				v = *s - '0';
			else if ( *s >= 'a' && *s <= 'f')
				v = *s - 'a' + 10;
			else if ( *s >= 'A' && *s <= 'F')
				v = *s - 'A' + 10;
			else {
				printf("error hex format [%x]!\n", *s);
				return 0;
			}
			k = 16 * k + v;
			s++;
		}
	}
	return k;
}

int nfbi_set_hcd_pid(int pid)
{
    int param;

    param = pid;
	return ioctl(nfbi_fd, NFBI_IOCTL_HCD_PID, &param);
}

int nfbi_get_event_msg(struct evt_msg *evt)
{
	return ioctl(nfbi_fd, NFBI_IOCTL_GET_EVENT, evt);
}

int hwreset(void)
{
    int param;
    
    param = 2; //hardware reset
	if (0 == ioctl(nfbi_fd, NFBI_IOCTL_PRIV_CMD, &param))
	    return 0;
	else
	    return -1;
}

int eqreset(void)
{
    int param;
    
    param = 5; //event queue reset
	if (0 == ioctl(nfbi_fd, NFBI_IOCTL_PRIV_CMD, &param))
	    return 0;
	else
	    return -1;
}

int nfbi_register_read(int reg, int *pval)
{
    int param, ret;
    
    param = reg << 16; //put register address to high word
    ret = ioctl(nfbi_fd, NFBI_IOCTL_REGREAD, &param);
	if (0 == ret)
        *pval = param&0xffff;
    return ret;
}

int nfbi_register_mask_read(int reg, int mask, int *pval)
{
    int param, ret;
    
    param = reg << 16; //put register address to high word
    ret = ioctl(nfbi_fd, NFBI_IOCTL_REGREAD, &param);
	if (0 == ret)
        *pval = param&(mask&0xffff);
    return ret;
}

int nfbi_register_write(int reg, int val)
{
    int param;
    
    //put register address to high word and the value to low word
    param = (reg << 16) | (val & 0xffff);
	return ioctl(nfbi_fd, NFBI_IOCTL_REGWRITE, &param);
}

int nfbi_register_mask_write(int reg, int mask, int val)
{
    int param, ret;

    // read register first for the 1st kind of command
    param = reg << 16; //put register address to high word
    ret = ioctl(nfbi_fd, NFBI_IOCTL_REGREAD, &param);
	if (0 != ret)
	    return ret;
	
	mask &= 0xffff;
	val = ((param&0xffff)&(~mask)) | (val&mask);
    //put register address to high word and the value to low word
    param = (reg << 16) | (val & 0xffff);
	return ioctl(nfbi_fd, NFBI_IOCTL_REGWRITE, &param);
}

int nfbi_mem32_write(int addr, int val)
{
#if 1
    struct nfbi_mem32_param param;

    param.addr = addr;
    param.val = val;
	return ioctl(nfbi_fd, NFBI_IOCTL_MEM32_WRITE, &param);
#else
    int ret;
    
    if (0 != nfbi_register_write(NFBI_REG_CMD, 0x0000)) //write mode
        return -1;
    if (0 != nfbi_register_write(NFBI_REG_ADDH, (addr>>16)&0xffff)) //address H
        return -1;
    if (0 != nfbi_register_write(NFBI_REG_ADDL, addr&0xffff)) //address L
        return -1;
    if (0 != nfbi_register_write(NFBI_REG_DH, (val>>16)&0xffff)) //data H
        return -1;
    if (0 != nfbi_register_write(NFBI_REG_DL, val&0xffff)) //data L
        return -1;
		
#ifdef CHECK_NFBI_BUSY_BIT
        // check NFBI hardware status
    	do {
            ret = nfbi_register_mask_read(NFBI_REG_CMD, BM_BUSY, &val);
            if (ret != 0)
                return -1;
            if (val) printf("BUSY\n");
        } while (val); //wait busy bit to zero
#endif
    return 0;
#endif
}

int nfbi_mem32_read(int addr, int *pval)
{
#if 1
    int ret;
    struct nfbi_mem32_param param;

    param.addr = addr;
	ret = ioctl(nfbi_fd, NFBI_IOCTL_MEM32_READ, &param);
	if (0 == ret)
	    *pval = param.val;
	return ret;
#else
	int h_val, l_val, ret, val;

    if (0 != nfbi_register_write(NFBI_REG_CMD, 0x8000)) //read mode
        return -1;
    if (0 != nfbi_register_write(NFBI_REG_ADDH, (addr>>16)&0xffff)) //address H
        return -1;
    if (0 != nfbi_register_write(NFBI_REG_ADDL, addr&0xffff)) //address L
        return -1;
    if (0 != nfbi_register_read(NFBI_REG_DH, &h_val)) //data H
        return -1;
    if (0 != nfbi_register_read(NFBI_REG_DL, &l_val)) //data L
        return -1;

#ifdef CHECK_NFBI_BUSY_BIT
        // check NFBI hardware status
    	do {
            ret = nfbi_register_mask_read(NFBI_REG_CMD, BM_BUSY, &val);
            if (ret != 0)
                return -1;
            if (val) printf("BUSY\n");
        } while (val); //wait busy bit to zero
#endif
    
    *pval = ((h_val<<16)&0xffff0000) | (l_val&0xffff);
	return 0;
#endif
}

int nfbi_bulk_mem_write(int addr, int len, char *buf)
{
    struct nfbi_bulk_mem_param param;

    // 0 < len <= NFBI_MAX_BULK_MEM_SIZE is a must
    if ((len <=0) || (len > NFBI_MAX_BULK_MEM_SIZE))
        return -1;
    param.addr = addr;
    param.len = len;
    memcpy(param.buf, buf, len);
	return ioctl(nfbi_fd, NFBI_IOCTL_BULK_MEM_WRITE, &param);
}

int nfbi_bulk_mem_read(int addr, int len, char *buf)
{
    int ret;
    struct nfbi_bulk_mem_param param;

    // 0 < len <= NFBI_MAX_BULK_MEM_SIZE is a must
    if ((len <=0) || (len > NFBI_MAX_BULK_MEM_SIZE))
        return -1;
    param.addr = addr;
    param.len = len;
	ret = ioctl(nfbi_fd, NFBI_IOCTL_BULK_MEM_READ, &param);
	if (0 == ret) {
	    memcpy(buf, param.buf, len);
	}
	return ret;
}

/*
int create_ddr_tmp_file(void)
{
    int fd;
    
    // Create lock file
    if ((fd = open(DDR_TMP_FILE_NAME, O_RDWR|O_CREAT|O_EXCL, 0444)) < 0) {
        perror("fail to create ddr file");
        return -1;
    }

    close(fd);
    return 0;
}

int check_ddr_tmp_file(void)
{
    int fd;

    // Check lock file
    if ((fd = open(DDR_TMP_FILE_NAME, O_RDONLY)) < 0)
        return 0;   // No exist, SDR
    
    close(fd);
    return 1;       // exist, DDR

}
*/

// only for 8198 NFBI
int get_dram_type(void)
{
	int val;

	// read hw_strap register
	nfbi_mem32_read(0x18000008, &val);
	if (val & 0x2) { //bit 1
		if (val & 0x40) //bit 6
			return 2; //DDR2
		else
			return 1; //DDR1
	}
	else {
		return 0; //SDR
	}
}

#define ENABLE_IPCLK (0x01FFFC00)   //ALL IPs

int do_dram_settings(void)
{
    int ret, val;

    // check if PHYID2 is equal to 0xcb61
    ret = nfbi_register_read(NFBI_REG_PHYID2, &val);
    if ((ret != 0) || ((val != NFBI_REG_PHYID2_DEFAULT)&&(val != NFBI_REG_PHYID2_DEFAULT2)))
        return -1;

    if (val == NFBI_REG_PHYID2_DEFAULT) {
        //Setting TX RX delay
        //if (0 != nfbi_mem32_write(0x18000010, 0x00FFFFD6)) { // for 162MHz DRAM
        //if (0 != nfbi_mem32_write(0x18000010, 0x00FFFFCB)) {   // for 148MHz DRAM
        if (0 != nfbi_mem32_write(0x18000010, 0x00038396)) {
            printf("fail to set TX RX delay\n");
            return -1;
        }
        real_sleep(1);
        //---------------------------------------------------------------
    	//setting proper DRAM config and timing registers
    	/*
    	if (0 != nfbi_register_write(NFBI_REG_DTH, DRAM_TIMING_VALH)) {
    	    printf("DTH write fail!\n");
            return -1;
        }
    	ret = nfbi_register_read(NFBI_REG_DTH, &val);
        if ((ret != 0) || (val != DRAM_TIMING_VALH)) {
            printf("DTH read fail!\n");
            return -1;
        }
    	if (0 != nfbi_register_write(NFBI_REG_DTL, DRAM_TIMING_VALL)) {
    	    printf("DTL write fail!\n");
            return -1;
        }
        ret = nfbi_register_read(NFBI_REG_DTL, &val);
        if ((ret != 0) || (val != DRAM_TIMING_VALL)) {
            printf("DTL read fail!\n");
            return -1;
        }
        */
        //---------------------------------------------------------------
    	if (0 != nfbi_register_write(NFBI_REG_DCH, DRAM_CONFIG_VALH)) {
    	    printf("DCH write fail!\n");
            return -1;
        }
    	ret = nfbi_register_read(NFBI_REG_DCH, &val);
        if ((ret != 0) || (val != DRAM_CONFIG_VALH)) {
            printf("DCH read fail!\n");
            return -1;
        }
    
    	if (0 != nfbi_register_write(NFBI_REG_DCL, DRAM_CONFIG_VALL)) {
    	    printf("DCL write fail!\n");
            return -1;
        }
    	ret = nfbi_register_read(NFBI_REG_DCL, &val);
        if ((ret != 0) || (val != DRAM_CONFIG_VALL)) {
            printf("DCL read fail!\n");
            return -1;
        }
        real_sleep(1);
    }
    else {
	int v;

	nfbi_mem32_read(0x18000048, &v);
	v &= 0xF83FFFBF;
	if (0 != nfbi_mem32_write(0x18000048, v)) {
		printf("fail to set 0x18001048\n");
		return -1;
	}
	
	//if (check_ddr_tmp_file()) { //DDR
	if (get_dram_type() == 1) { //DDR1
		//Setting TX RX delay
		if (0 != nfbi_mem32_write(0x18000010, (ENABLE_IPCLK | (14<<5) | (25<<0)))) {
	            printf("fail to set TX RX delay\n");
	            return -1;
	        }
	}
	else if (get_dram_type() == 2) { //DDR2
		//Setting TX RX delay
		if (0 != nfbi_mem32_write(0x18000010, (ENABLE_IPCLK | (9<<5) | (14<<0)))) {
	            printf("fail to set TX RX delay\n");
	            return -1;
	        }
	}
	else { //SDR
		//Setting TX RX delay
	        if (0 != nfbi_mem32_write(0x18000010, (ENABLE_IPCLK | (9<<5) | (10<<0)))) {
	            printf("fail to set TX RX delay\n");
	            return -1;
	        }
	}
        real_sleep(1);
                
        //DTR
        if (get_dram_type() == 1) { //DDR1
        	//REG32_W(0xb8001008, 0xFFFF05C0);  //FPGA safe parameter 
        	if (0 != nfbi_mem32_write(0x18001008, 0x6CEDA480)) {
	            printf("fail to set 0x18001008\n");
	            return -1;
	        }
        }
        else if (get_dram_type() == 2) { //DDR2
        	//REG32_W(0xb8001008, 0xFFFF05C0);  //FPGA safe parameter 
  		if (0 != nfbi_mem32_write(0x18001008, 0xFFFF05C0)) {
        	//if (0 != nfbi_mem32_write(0x18001008, 0x6D0FA4C0)) {
	            printf("fail to set 0x18001008\n");
	            return -1;
	        }
        }
        else {	//SDR
        	//if (0 != nfbi_mem32_write(0x18001008, 0xffff05c0)) {
	        if (0 != nfbi_mem32_write(0x18001008, 0x6cca8480)) {
	            printf("fail to set 0x18001008\n");
	            return -1;
	        }
	}
        real_sleep(1);

	//DCR
#if defined(CONFIG_D8_16)
	if (0 != nfbi_mem32_write(0x18001004, 0x52080000)) //8M
#elif defined(CONFIG_D16_16)
	if (0 != nfbi_mem32_write(0x18001004, 0x52480000)) //16M
#elif defined(CONFIG_D32_16)
	if (0 != nfbi_mem32_write(0x18001004, 0x54480000)) //32M
#elif defined(CONFIG_D64_16)
	if (0 != nfbi_mem32_write(0x18001004, 0x54880000)) //64M
#elif defined(CONFIG_D8_16x2)
	if (0 != nfbi_mem32_write(0x18001004, 0x5a080000)) //8MBx2
#elif defined(CONFIG_D16_16x2)
	if (0 != nfbi_mem32_write(0x18001004, 0x5a480000)) //16MBx2
#elif defined(CONFIG_D32_16x2)
	if (0 != nfbi_mem32_write(0x18001004, 0x5c480000)) //32MBx2
#elif defined(CONFIG_D64_16x2)
	if (0 != nfbi_mem32_write(0x18001004, 0x5c880000)) //64MBx2
#endif
	{
		printf("fail to set 0x18001004\n");
		return -1;
	}
	real_sleep(1);
	
        //if (check_ddr_tmp_file()) { //DDR
	if (get_dram_type()) { //DDR
            //do DDR calibration
            DDR_NFBI_cali();
        }
    }
    return 0;
}

/* before sending anything to ram, it's necessary to call do_dram_settings() to configure DRAM first */
void send_foreverloop_to_ram(int ram_addr)
{
	int i,off=0;

	unsigned int jmpcode[10]={
	    			//just for debug
				0x3c085a5a, 	// lui	t0,0x55aa
				0x35085a5a, 	// ori	t0,t0,0x5500
				0x3c09b800, 	// lui	t1,0xb800
				0x35293104, 	// ori	t1,t1,0x3104
				0xad280000,     // sw	t0,0(t1)
							
				0x3c1abfc0, 	// lui	k0,0xa070 
				0x375a0000,     // ori	k0,k0,0x0000  
				0x03400008,     // jr	k0 
				0x0		// nop
				};


	ram_addr = ram_addr&0x0fffffff;
	for(i=0;i<9;i++)
	    nfbi_mem32_write(ram_addr+(off++)*4, jmpcode[i]);
}

void send_jumpcmmand_to_ram(int ram_addr, int jump_addr)
{
	int val, v;
	int i,off=0;
	unsigned int dramcode[25]={
                    		0x3c080f0a,  // lui t0,0f0a   
                    		0x3508dfff,  // ori t0,t0,0xdfff
                    		0x3c09b800,  // lui t1,0xb800
                    		0x35290048,  // ori t1,t1,0x0048
                     		0xad280000,  // sw t0,0(t1)
                     		
                    		0x3c080003,  // lui t0,0003   
                    		0x35088396,  // ori t0,t0,0x8396 
                    		0x3c09b800,  // lui t1,0xb800
                    		0x35290010,  // ori t1,t1,0x0010
                     		0xad280000,  // sw t0,0(t1)
 		
				0x3c086cea, 	// lui	t0,0x6cea
				0x35080a80, 	// ori	t0,t0,0x0a80
				0x3c09b800, 	// lui	t1,0xb800
				0x35291008, 	// ori	t1,t1,0x1008
				0xad280000, 	// sw	t0,0(t1)

				//0x3c085448, 	// lui	t0,0x5448 //32MB DRAM
				0x3c085208,     // lui	t0,0x5208 //8MB  DRAM
				0x35080000, 	// ori	t0,t0,0x0000							
				0x3c09b800, 	// lui	t1,0xb800
				0x35291004, 	// ori	t1,t1,0x1004
				0xad280000, 	// sw	t0,0(t1)
														
				//just for debug
				0x3c0855aa, 	// lui	t0,0x55aa
				0x35085500, 	// ori	t0,t0,0x5500
				0x3c09b800, 	// lui	t1,0xb800
				0x35293104, 	// ori	t1,t1,0x3104
				0xad280000,     // sw	t0,0(t1)
				};

	unsigned int jmpcode[4]={
				0x3c1aa070, 	// lui	k0,0xa070 
				0x375a0000,     // ori	k0,k0,0x0000  
				0x03400008,     // jr	k0 
				0x0			    // nop
				};

	// check if PHYID2 is equal to 0xcb61
	nfbi_register_read(NFBI_REG_PHYID2, &val);
	if (val == NFBI_REG_PHYID2_DEFAULT) {
		// setting DCR and DTR register
		dramcode[10]=(dramcode[10] &0xffff0000) | DRAM_TIMING_VALH;
		dramcode[11]=(dramcode[11] &0xffff0000) | DRAM_TIMING_VALL;
		dramcode[15]=(dramcode[15] &0xffff0000) | DRAM_CONFIG_VALH;
		dramcode[16]=(dramcode[16] &0xffff0000) | DRAM_CONFIG_VALL;
	}
	else {
		//8198
		// DTR
		if (get_dram_type() == 1) { //DDR1
        		//REG32_W(0xb8001008, 0xFFFF05C0);  //FPGA safe parameter 
  			//REG32_W(0xb8001008,0x6CEDA480);
			dramcode[10]=(dramcode[10] &0xffff0000) | 0x6CED;
			dramcode[11]=(dramcode[11] &0xffff0000) | 0xA480;
		}
		else if (get_dram_type() == 2) { //DDR2
        		//REG32_W(0xb8001008, 0xFFFF05C0);  //FPGA safe parameter 
  			//REG32_W(0xb8001008,0x6D0FA4C0);
			dramcode[10]=(dramcode[10] &0xffff0000) | 0xFFFF;
			dramcode[11]=(dramcode[11] &0xffff0000) | 0x05C0;
		}
		else {
			//0xFFFF05C0 or 0x6cca8480
			dramcode[10]=(dramcode[10] &0xffff0000) | 0x6cca;
			dramcode[11]=(dramcode[11] &0xffff0000) | 0x8480;
		}

		//DCR
#if defined(CONFIG_D8_16)
		dramcode[15]=(dramcode[15] &0xffff0000) | 0x5208; //8M
#elif defined(CONFIG_D16_16)
		dramcode[15]=(dramcode[15] &0xffff0000) | 0x5248; //16M
#elif defined(CONFIG_D32_16)
		dramcode[15]=(dramcode[15] &0xffff0000) | 0x5448; //32M
#elif defined(CONFIG_D64_16)
		dramcode[15]=(dramcode[15] &0xffff0000) | 0x5488; //64M
#elif defined(CONFIG_D8_16x2)
		dramcode[15]=(dramcode[15] &0xffff0000) | 0x5a08; //8MBx2
#elif defined(CONFIG_D16_16x2)
		dramcode[15]=(dramcode[15] &0xffff0000) | 0x5a48; //16MBx2
#elif defined(CONFIG_D32_16x2)
		dramcode[15]=(dramcode[15] &0xffff0000) | 0x5c48; //32Mx2
#elif defined(CONFIG_D64_16x2)
		dramcode[15]=(dramcode[15] &0xffff0000) | 0x5c88; //64MBx2
#endif
		dramcode[16]=(dramcode[16] &0xffff0000) | 0x0000;

		nfbi_mem32_read(0x18000048, &v);
		v &= 0xF83FFFBF;
		dramcode[0]=(dramcode[0] &0xffff0000) | ((v>>16)&0xffff);
		dramcode[1]=(dramcode[1] &0xffff0000) | (v&0xffff);
			
		//if (check_ddr_tmp_file())
		if (get_dram_type() == 1) { //DDR1
			// TX RX delay
			v = ENABLE_IPCLK | (14<<5) | (25<<0);
		}
		else if (get_dram_type() == 2) { //DDR2
			// TX RX delay
			v = ENABLE_IPCLK | (9<<5) | (14<<0);
		}
		else { //SDR
			// TX RX delay
			v = ENABLE_IPCLK | (9<<5) | (10<<0);
		}
		dramcode[5]=(dramcode[5] &0xffff0000) | ((v>>16)&0xffff);
		dramcode[6]=(dramcode[6] &0xffff0000) | (v&0xffff);
	}

	jmpcode[0]=(jmpcode[0] &0xffff0000) | ((jump_addr>>16) & 0xffff);
	jmpcode[1]=(jmpcode[1] &0xffff0000) | (jump_addr & 0xffff);

	ram_addr = ram_addr&0x0fffffff;
	if (val == NFBI_REG_PHYID2_DEFAULT) {
		for(i=5;i<25;i++)
			nfbi_mem32_write(ram_addr+(off++)*4, dramcode[i]);
	}
	else {
		//8198
		for(i=0;i<25;i++)
			nfbi_mem32_write(ram_addr+(off++)*4, dramcode[i]);
	}
	for(i=0;i<4;i++)
	    nfbi_mem32_write(ram_addr+(off++)*4, jmpcode[i]);
}

/* before sending anything to ram, it's necessary to call do_dram_settings() to configure DRAM first */
//int send_file_to_ram(int fd, unsigned int addr, int verify)
int send_file_to_ram(char *filename, unsigned int ram_addr, int verify)
{
    struct stat ffstat;
	//int val, h_val, l_val, ret;
	int flen=0;
	int rc, i, j;
	int fd;
	char buf[NFBI_MAX_BULK_MEM_SIZE], buf2[NFBI_MAX_BULK_MEM_SIZE];
	int num;
	unsigned int addr;

    fd = open(filename, O_RDONLY);
	if (fd < 0)	{
		printf("Cannot Open file %s!\n", filename);
		return -1;
	}
	
	//get file length	
	fstat(fd, &ffstat);
	flen = ffstat.st_size;
	//printf("File Length=%d bytes\n", flen);
    addr = ram_addr;
#if 1
    num = flen/NFBI_MAX_BULK_MEM_SIZE;
    for (i=0; i<num; i++) {
        rc = read(fd, buf, NFBI_MAX_BULK_MEM_SIZE);
  	    if (rc != NFBI_MAX_BULK_MEM_SIZE) {
  	        printf("Reading error\n");
  	        return -1;
  	    }
  	    rc = nfbi_bulk_mem_write(addr, NFBI_MAX_BULK_MEM_SIZE, buf);
        if (rc != 0) {
            printf("error: %s %d\n", __FUNCTION__, __LINE__);
  	        return -1;
  	    }
        addr += NFBI_MAX_BULK_MEM_SIZE;
        //printf(".");
    }
    num = flen%NFBI_MAX_BULK_MEM_SIZE;
    if (num > 0) {
        rc = read(fd, buf, num);
  	    if (rc != num) {
  	        printf("Reading error, %s %d\n", __FUNCTION__, __LINE__);
  	        return -1;
  	    }
  	    rc = nfbi_bulk_mem_write(addr, num, buf);
        if (rc != 0) {
            printf("error: %s %d\n", __FUNCTION__, __LINE__);
  	        return -1;
  	    }
    }
    printf("\n");
        
#else
    if (flen%4) {
		printf("size is not multiple of 4\n");
		return -1;
	}

    if (0 != nfbi_register_write(NFBI_REG_CMD, 0x0000)) //write mode
        return -1;
    if (0 != nfbi_register_write(NFBI_REG_ADDH, (addr>>16)&0xffff)) //address H
        return -1;
    if (0 != nfbi_register_write(NFBI_REG_ADDL, addr&0xffff)) //address L
        return -1;
	for (i=0; i<flen; i+=4)	{
        rc = read(fd, (char *)&val, 4);
  	    if (rc != 4) {
  	        printf("Reading error\n");
  	        return -1;
  	    }
  	    if (0 != nfbi_register_write(NFBI_REG_DH, (val>>16)&0xffff)) //data H
            return -1;
        if (0 != nfbi_register_write(NFBI_REG_DL, val&0xffff)) //data L
            return -1;
		
#ifdef CHECK_NFBI_BUSY_BIT
        // check NFBI hardware status
    	do {
            ret = nfbi_register_mask_read(NFBI_REG_CMD, BM_BUSY, &val);
            if (ret != 0)
                return -1;
            if (val) printf("BUSY\n");
        } while (val); //wait busy bit to zero
#endif
		if ((i%1024)==0)
		    printf(".");
	}
	printf("\n");
#endif /* #if 1 */

    close(fd);
    
    if (!verify)
        return 0;

    real_sleep(1);
    //sets the file position indicator to the beginning of the file
    //fseek(fd, 0, SEEK_SET);

    fd = open(filename, O_RDONLY);
	if (fd < 0)	{
		printf("Cannot Open file %s!\n", filename);
		return -1;
	}
    addr = ram_addr;
#if 1
    num = flen/NFBI_MAX_BULK_MEM_SIZE;
    for (i=0; i<num; i++) {
        rc = read(fd, buf, NFBI_MAX_BULK_MEM_SIZE);
  	    if (rc != NFBI_MAX_BULK_MEM_SIZE) {
  	        printf("Reading error\n");
  	        return -1;
  	    }
  	    rc = nfbi_bulk_mem_read(addr, NFBI_MAX_BULK_MEM_SIZE, buf2);
        if (rc != 0) {
            printf("error: %s %d\n", __FUNCTION__, __LINE__);
  	        return -1;
  	    }
        
        for (j=0; j<NFBI_MAX_BULK_MEM_SIZE; j++) {
            if (buf[j] != buf2[j]) {
                printf("Error! at addr=%x value=%x Expect val=%x\n", 
                        addr+j, buf2[j], buf[j]);
                close(fd);
	            return -1;
            }
        }
        /*
        if (memcmp(buf, buf2, NFBI_MAX_BULK_MEM_SIZE) != 0) {
            printf("Error! at addr=%x\n", addr);
        }
        */
        addr += NFBI_MAX_BULK_MEM_SIZE;

        //printf(".");
    }
    num = flen%NFBI_MAX_BULK_MEM_SIZE;
    if (num > 0) {
        rc = read(fd, buf, num);
  	    if (rc != num) {
  	        printf("Reading error, %s %d\n", __FUNCTION__, __LINE__);
  	        return -1;
  	    }
  	    rc = nfbi_bulk_mem_read(addr, num, buf2);
        if (rc != 0) {
            printf("error: %s %d\n", __FUNCTION__, __LINE__);
  	        return -1;
  	    }
        for (j=0; j<num; j++) {
            if (buf[j] != buf2[j]) {
                printf("Error! at addr=%x value=%x Expect val=%x\n", 
                        addr+j, buf2[j], buf[j]);
                close(fd);
	            return -1;
            }
        }
        /*
        if (memcmp(buf, buf2, NFBI_MAX_BULK_MEM_SIZE) != 0) {
            printf("Error! at addr=%x\n", addr);
        }
        */
    }
    printf("\n");
        
#else
    //starting the verification
    if (0 != nfbi_register_write(NFBI_REG_CMD, 0x8000)) //read mode
        return -1;
    if (0 != nfbi_register_write(NFBI_REG_ADDH, (addr>>16)&0xffff)) //address H
        return -1;
    if (0 != nfbi_register_write(NFBI_REG_ADDL, addr&0xffff)) //address L
        return -1;
	for (i=0; i<flen; i+=4)	{

#ifdef CHECK_NFBI_BUSY_BIT
        // check NFBI hardware status
    	do {
            ret = nfbi_register_mask_read(NFBI_REG_CMD, BM_BUSY, &val);
            if (ret != 0)
                return -1;
            if (val) printf("BUSY\n");
        } while (val); //wait busy bit to zero
#endif

        rc = read(fd, (char *)&val, 4);
  	    if (rc != 4) {
  	        printf("Reading error\n");
  	        return -1;
  	    }

        if (0 != nfbi_register_read(NFBI_REG_DH, &h_val)) //data H
            return -1;
        if (0 != nfbi_register_read(NFBI_REG_DL, &l_val)) //data L
            return -1;
        
        if (val != (((h_val<<16)&0xffff0000) | (l_val&0xffff))) {
            printf("Error! at addr=%x value=%x Expect val=%x\n", 
                        i, (((h_val<<16)&0xffff0000) | (l_val&0xffff)), val);
            close(fd);
            return -1;
        }
    
		if ((i%1024)==0)
		    printf(".");
	}
	printf("\n");
#endif /* #if 1 */

	close(fd);
			
	return 0;
}

int bootcode_download3(char *filename)
{
    int ret, val;

    //disable ChecksumDone, ChecksumOk, AllSoftwareReady, NeedBootCodeIP interrupts
    //to make sure the status bit of ISR would not be clear by NFBI driver(interrupt service routine)
    ret = nfbi_register_mask_write(NFBI_REG_IMR,
                                   (IM_CHECKSUM_DONE|IM_CHECKSUM_OK|IM_ALLSOFTWARE_READY|IM_NEED_BOOTCODE),
                                    0x0);
    if (ret != 0)
        return -1;

    //clear all bits of ISR
    ret = nfbi_register_write(NFBI_REG_ISR, 0xffff);
    if (ret != 0)
        return -1;

    //write 1 to SystemRst bit to reset the 8197B
    ret = nfbi_register_write(NFBI_REG_CMD, BM_SYSTEMRST);
    if (ret != 0)
        return -1;
        

    /* When mdio access is high speed, it's possible that
     * it begin to send bootcode before reset complete.
     * To make sure hardware reset finish,
     * check if BM_SYSTEMRST bit automatically retun to zero.
     */
    do {
        ret = nfbi_register_read(NFBI_REG_CMD, &val);
        if (ret != 0)
            return -1;
    } while( (val & BM_SYSTEMRST) == BM_SYSTEMRST );

    // check if PHYID2 is equal to 0xcb61
    ret = nfbi_register_read(NFBI_REG_PHYID2, &val);
    if ((ret != 0) || ((val != NFBI_REG_PHYID2_DEFAULT)&&(val != NFBI_REG_PHYID2_DEFAULT2)))
        return -1;

    //check if NeedBootCodeIP is equal to 1
    ret = nfbi_register_mask_read(NFBI_REG_ISR, IP_NEED_BOOTCODE, &val);
    if (ret != 0)
        return -1;
    if (val != IP_NEED_BOOTCODE) {
        printf("IP_NEED_BOOTCODE = %d\n", val);
        return -1;
    }

    //write 1 to clear NeedBootCodeIP
    ret = nfbi_register_write(NFBI_REG_ISR, IP_NEED_BOOTCODE);
    if (ret != 0)
        return -1;

	//setting proper DRAM config and timing registers
	if (0 != do_dram_settings())
        return -1;

    send_jumpcmmand_to_ram(0x00008000, (0xa0000000+NFBI_BOOTADDR));
    
    system("date");
    //upload the bootcode to 8197B DRAM
    //if (0 != send_file_to_ram(filename, NFBI_BOOTADDR, 1)) {
	if (0 != send_file_to_ram(filename, NFBI_BOOTADDR, 0)) {
		printf("send_file_to_ram fail\n");
		return -1;
	}
	system("date");
	
	//run the bootcode, write 1 to StartRunBootCode bit
    ret = nfbi_register_mask_write(NFBI_REG_CMD, BM_START_RUN_BOOTCODE, BM_START_RUN_BOOTCODE);
    if (ret != 0)
        return -1;
	
    return 0;
}

int bootcode_download2(char *filename, int offset)
{
    int ret, val;

    // check if PHYID2 is equal to 0xcb61
    ret = nfbi_register_read(NFBI_REG_PHYID2, &val);
    if ((ret != 0) || ((val != NFBI_REG_PHYID2_DEFAULT)&&(val != NFBI_REG_PHYID2_DEFAULT2)))
        return -1;
    
    //check if NeedBootCodeIP is equal to 1
    /*
    ret = nfbi_register_mask_read(NFBI_REG_ISR, IP_NEED_BOOTCODE, &val);
    if (ret != 0)
        return -1;
    if (val != IP_NEED_BOOTCODE) {
        printf("IP_NEED_BOOTCODE = %d\n", val);
        return 1;
    }
    */
    
    //write 1 to clear NeedBootCodeIP
    ret = nfbi_register_write(NFBI_REG_ISR, IP_NEED_BOOTCODE);
    if (ret != 0)
        return -1;

	//setting proper DRAM config and timing registers
	if (0 != do_dram_settings())
        return -1;

    send_jumpcmmand_to_ram(0x00008000, (0xa0000000+offset));

    //upload the file to 8197B DRAM
	if (0 != send_file_to_ram(filename, offset, 0)) {
		printf("send_file_to_ram fail\n");
		return -1;
	}
	
	//run the bootcode, write 1 to StartRunBootCode bit
    ret = nfbi_register_mask_write(NFBI_REG_CMD, BM_START_RUN_BOOTCODE, BM_START_RUN_BOOTCODE);
    if (ret != 0)
        return -1;
	
    return 0;
}

int bootcode_download(int verify, char *filename)
{
    int ret, val, ret2, count;
    
    ret2 = 0;
    count = 0;
begin:
    if (ret2)
        printf("count=%d ret2=%d\n", count, ret2);
    if (count < BOOTCODE_DOWNLOAD_RETRY_MAX)
        count++;
    else
        return ret2;
#if 0
	// VoIP may connect more than one DSP, 
    hwreset();
    real_sleep(1);
#endif
    
    // check if PHYID2 is equal to 0xcb61
    ret = nfbi_register_read(NFBI_REG_PHYID2, &val);
    if ((ret != 0) || ((val != NFBI_REG_PHYID2_DEFAULT)&&(val != NFBI_REG_PHYID2_DEFAULT2))) {
        ret2 = -101;
        goto begin;
    }

    // check if StartRunBootCode bit is equal to 0
    if (0 != nfbi_register_read(NFBI_REG_CMD, &val)) {
        ret2 = -102;
        goto begin;
    }
    if (val&BM_START_RUN_BOOTCODE) {
        ret2 = -103;
        goto begin;
    }
    
    //MII non-isolation, write 0 to isolation bit
    ret = nfbi_register_mask_write(NFBI_REG_SYSCR, BM_ISOLATION, 0x0000);
    if (ret != 0) {
        ret2 = -104;
        goto begin;
    }
    
    ret = nfbi_register_write(NFBI_REG_IMR, 0x0);
    if (ret != 0) {
        ret2 = -105;
        goto begin;
    }
       
    //clear all bits of ISR
    ret = nfbi_register_write(NFBI_REG_ISR, 0xffff);
    if (ret != 0) {
        ret2 = -106;
        goto begin;
    }
    
	//setting proper DRAM config and timing registers
	if (0 != do_dram_settings()) {
        ret2 = -107;
        goto begin;
    }
    real_sleep(1);
    send_jumpcmmand_to_ram(0x00008000, (0xa0000000+NFBI_BOOTADDR));
    
    //upload the bootcode to 8197B DRAM
	if (0 != send_file_to_ram(filename, NFBI_BOOTADDR, verify)) {
		printf("send_file_to_ram fail\n");
        ret2 = -108;
        //for debugging
        //send_foreverloop_to_ram(0x00008000);
        //nfbi_register_mask_write(NFBI_REG_CMD, BM_START_RUN_BOOTCODE, BM_START_RUN_BOOTCODE);
        goto begin;
	}
	
    //dump_misc("StartRunBootCode000");
	//run the bootcode, write 1 to StartRunBootCode bit
    ret = nfbi_register_mask_write(NFBI_REG_CMD, BM_START_RUN_BOOTCODE, BM_START_RUN_BOOTCODE);
    if (ret != 0) {
        ret2 = -109;
        goto begin;
    }
	
	//wait for bootcode to be ready
	real_sleep(3);
    //dump_misc("StartRunBootCode111");

    //check if bit 5 is equal to 1
    if (0 != nfbi_register_read(NFBI_REG_ISR, &val)) {
        ret2 = -110;
        goto begin;
    }

    if (!(val&IP_BOOTCODE_READY)) {
        dump_misc("info");
        ret2 = -111;
        goto begin;
    }
    
    //Write 1 to clear the IP_BOOTCODE_READY bit of ISR
    ret = nfbi_register_write(NFBI_REG_ISR, IP_BOOTCODE_READY);
    if (ret != 0) {
        ret2 = -112;
        goto begin;
    }
    return 0;
}

int firmware_download_w_boot(int verify, char *fw_filename, char *boot_filename)
{
    int ret, val, timeout;
    char temp[256];

    //download bootcode to memory
    ret = bootcode_download(verify, boot_filename);
    if (ret != 0)
        return -1;

	//upload the firmware to 8197B by TFTP
	sprintf(temp, "tftp -p 192.168.1.97 -l %s", fw_filename);
	system(temp);

	//wait for bootcode to calculate the checksum of firmware
	//real_sleep(5);

    //check if ChecksumDoneIP is equal to 1
    //if (0 != nfbi_register_read(NFBI_REG_ISR, &val))
    //    return -2;
    timeout = 10;
	while (timeout > 0) {
	    real_sleep(1);
        if (0 != nfbi_register_read(NFBI_REG_ISR, &val)) {
            printf("fail to read ISR\n");
            timeout--;
            continue;
        }
        if (val&IP_CHECKSUM_DONE)
            break;
        timeout--;
    }
    printf("ISR=0x%04x timeout=%d\n", val, timeout);
    if (timeout <= 0) {
        printf("timeout\n");
        return -2;
    }
    if (!(val&IP_CHECKSUM_DONE)) {
        return -3;
    }
    real_sleep(1);
    //Write 1 to clear the CheckSumDoneIP bit of ISR
    ret = nfbi_register_write(NFBI_REG_ISR, IP_CHECKSUM_DONE);
    if (ret != 0)
        return -4;
    if (0 != nfbi_register_read(NFBI_REG_SYSSR, &val))
        return -5;
    printf("SYSSR=0x%04x\n", val);
    if (!(val&BM_CHECKSUM_DONE))
        return -6;

    real_sleep(1);
    //check if ChecksumOKIP is equal to 1
    if (0 != nfbi_register_read(NFBI_REG_ISR, &val))
        return -7;
    printf("ISR=0x%04x\n", val);
    if (!(val&IP_CHECKSUM_OK))
        return -8;
    
    //Write 1 to clear the ChecksumOKIP bit of ISR
    ret = nfbi_register_write(NFBI_REG_ISR, IP_CHECKSUM_OK);
    if (ret != 0)
        return -9;
    if (0 != nfbi_register_read(NFBI_REG_SYSSR, &val))
        return -10;
    printf("SYSSR=0x%04x\n", val);
    if (!(val&BM_CHECKSUM_OK))
        return -11;
    
	//wait for firmware running to be ready
	//real_sleep(3);

    //check if AllSoftwareReadyIP is equal to 1
    //if (0 != nfbi_register_read(NFBI_REG_ISR, &val))
    //    return -12;
    
    timeout = 10;
	while (timeout > 0) {
	    real_sleep(1);
        if (0 != nfbi_register_read(NFBI_REG_ISR, &val)) {
            printf("fail to read ISR\n");
            timeout--;
            continue;
        }
        if (val&IP_ALLSOFTWARE_READY)
            break;
        timeout--;
    }
    printf("ISR=0x%04x timeout=%d\n", val, timeout);
    if (timeout <= 0) {
        printf("timeout\n");
        return -12;
    }
    if (!(val&IP_ALLSOFTWARE_READY))
        return -13;

    //Write 1 to clear the AllSoftwareReadyIP bit of ISR
    ret = nfbi_register_write(NFBI_REG_ISR, IP_ALLSOFTWARE_READY);
    if (ret != 0)
        return -14;

    return 0;
}

int firmware_download_wo_boot(int verify, char *fw_filename)
{
    int ret, val;

    //disable ChecksumDone, ChecksumOk, AllSoftwareReady, NeedBootCodeIP interrupts
    //to make sure the status bit of ISR would not be clear by NFBI driver(interrupt service routine)
    ret = nfbi_register_mask_write(NFBI_REG_IMR,
                                   (IM_CHECKSUM_DONE|IM_CHECKSUM_OK|IM_ALLSOFTWARE_READY|IM_NEED_BOOTCODE),
                                    0x0);
    if (ret != 0)
        return -1;

    //clear all bits of ISR
    ret = nfbi_register_write(NFBI_REG_ISR, 0xffff);
    if (ret != 0)
        return -2;

    //write 1 to SystemRst bit to set the 8197B
    ret = nfbi_register_write(NFBI_REG_CMD, BM_SYSTEMRST);
    if (ret != 0)
        return -3;

    /* When mdio access is high speed, it's possible that
     * it begin to send bootcode before reset complete.
     * To make sure hardware reset finish,
     * check if BM_SYSTEMRST bit automatically retun to zero.
     */
    do {
        ret = nfbi_register_read(NFBI_REG_CMD, &val);
        if (ret != 0)
            return -4;
    } while( (val & BM_SYSTEMRST) == BM_SYSTEMRST );

    // check if PHYID2 is equal to 0xcb61
    ret = nfbi_register_read(NFBI_REG_PHYID2, &val);
    if ((ret != 0) || ((val != NFBI_REG_PHYID2_DEFAULT)&&(val != NFBI_REG_PHYID2_DEFAULT2)))
        return -5;
    
    //check if NeedBootCodeIP is equal to 1
    ret = nfbi_register_mask_read(NFBI_REG_ISR, IP_NEED_BOOTCODE, &val);
    if (ret != 0)
        return -6;
    if (val != IP_NEED_BOOTCODE) {
        printf("IP_NEED_BOOTCODE = %d\n", val);
        return -7;
    }

    //write 1 to clear NeedBootCodeIP
    ret = nfbi_register_write(NFBI_REG_ISR, IP_NEED_BOOTCODE);
    if (ret != 0)
        return -8;

	//setting proper DRAM config and timing registers
	if (0 != do_dram_settings())
        return -9;

    send_jumpcmmand_to_ram(0x00008000, (0xa0000000+NFBI_KERNADDR));
    
    //upload the file to 8197B DRAM
    if (fw_filename != NULL) {
    	if (0 != send_file_to_ram(fw_filename, (NFBI_KERNADDR-0x10), verify)) {
    		printf("send_file_to_ram fail\n");
    		return -10;
    	}
	}
	
	//run the bootcode, write 1 to StartRunBootCode bit
    ret = nfbi_register_mask_write(NFBI_REG_CMD, BM_START_RUN_BOOTCODE, BM_START_RUN_BOOTCODE);
    if (ret != 0)
        return -11;

    return 0;
}

void dump_misc(char *msg)
{
    printf("%s\n", msg);
    printf("----------------------------\n");
    //printf("hw strap/clk manage/GISR/timer 1/memory/memory\n");
    printf("GISR/timer 1/memory/memory\n");
	//system("nfbi memread 0x18000008"); //hw strap	
	//system("nfbi memread 0x18000010"); //clk manage	
	system("nfbi memread 0x18003004"); //GISR
	system("nfbi memread 0x18003104"); //timer 1
    system("nfbi memread 0x00008000"); //memory
    system("nfbi memread 0x007f0000"); //memory
    printf("----------------------------\n");
}


// int  DDCR_ADR = 0xB8001050;//Virtual
// int  DDCR_ADR = 0x18001050;//Physical
#define DTR_REG_PHYSICAL 0x18001008
#define DCR_REG_PHYSICAL 0x18001004
#define DDCR_ADR 0x18001050

#define PADDR(i)                 ((i) & 0x1FFFFFFF)
#define DDR_DBG 0
//#define IMEM_DDR_CALI_LIMITS 1
#define IMEM_DDR_CALI_LIMITS 100

// Calibration Code
void DDR_NFBI_cali(void)
{
  int i, k;
  int val;

  int L0 = 0, R0 = 33, L1 = 0, R1 = 33;

  // int  DRAM_ADR = 0xa0400000;//0xa0400000 is virtual
  int  DRAM_ADR = 0x400000;//Physical address

  int  DRAM_VAL = 0x5A5AA5A5;

  unsigned int  DDCR_VAL = 0x80000000; //Digital
  //unsigned int  DDCR_VAL = 0x0; //Analog (JSW: N/A for 8198 FPGA)

  printf("Setting DTR/DCR in DDR_NFBI_cali\r\n");
  /*Set DRAM DTR Parameter*/
  //nfbi_mem32_write(DTR_REG_PHYSICAL, 0xffff05c0);

  /*Set DRAM DCR Parameter*/
  //nfbi_mem32_write(DCR_REG_PHYSICAL, DRAM_CONFIG_VAL);   // 32MB,enable CS0 and CS1

  // set R/W pattern
  nfbi_mem32_write(DRAM_ADR, DRAM_VAL);   

  //while( (NFBI_READ_MEM32(DDCR_ADR)& 0x40000000) != 0x40000000);


  for (k = 1;k <= IMEM_DDR_CALI_LIMITS;k++) { //Calibration times
//DDR_Calibration_Guard:
    // Calibrate for DQS0
    for (i = 1; i <= 31; i++) {
#if DDR_DBG
      //real_sleep(1);
      nfbi_mem32_read(DRAM_ADR, &val);
      //printf("\nDQS0(i=%d),(DDCR=0x%x)\n", i, val);
#endif

      nfbi_mem32_write(DDCR_ADR, (DDCR_VAL & 0x80000000) | ((i - 1) << 25));
      nfbi_mem32_write(DDCR_ADR, (DDCR_VAL & 0x80000000) | ((i - 1) << 25));
      if (L0 == 0) {
        nfbi_mem32_read(DRAM_ADR, &val);
        if ((val & 0x00FF00FF) == 0x005A00A5) {
          L0 = i;
        }
      }
      else {
#if DDR_DBG
        nfbi_mem32_read(DRAM_ADR, &val);
        printf("\nDRAM(0x%x)=%x\n", DRAM_ADR, val);
#endif
        nfbi_mem32_read(DRAM_ADR, &val);
        if ((val & 0x00FF00FF) != 0x005A00A5) {
          //dprintf("\n\n\nError!DQS0(i=%d),(DDCR=0x%x)\n", i,READ_MEM32(DDCR_ADR));
#if DDR_DBG
          nfbi_mem32_read(DRAM_ADR, &val);
          printf("DRAM(0x%x)=%x\n\n\n", DRAM_ADR, val);
#endif
          R0 = i - 1;
          //R0 = i - 3;  //JSW
          break;
        }
      }
    }

    DDCR_VAL = (DDCR_VAL & 0xC0000000) | (((L0 + R0) >> 1) << 25); // ASIC
    printf("\nDDCR_VAL(0) =%x\n", DDCR_VAL);
    nfbi_mem32_write(DDCR_ADR,  DDCR_VAL);
    nfbi_mem32_write(DDCR_ADR,  DDCR_VAL);


    // Calibrate for DQS1
    for (i = 1; i <= 31; i++) {
#if DDR_DBG
      //real_sleep(1);
      nfbi_mem32_read(DRAM_ADR, &val);
      //printf("\nDQS1(i=%d),(DDCR=0x%x)\n", i, val);
#endif

      nfbi_mem32_write(DDCR_ADR, (DDCR_VAL & 0xFE000000) | ((i - 1) << 20));
      nfbi_mem32_write(DDCR_ADR, (DDCR_VAL & 0xFE000000) | ((i - 1) << 20));

      if (L1 == 0) {
        nfbi_mem32_read(DRAM_ADR, &val);
        if ((val & 0xFF00FF00) == 0x5A00A500) {
          L1 = i;
        }
      }
      else {
#if DDR_DBG
        nfbi_mem32_read(DRAM_ADR, &val);
        printf("\nDRAM(0x%x)=%x\n", DRAM_ADR, val);
#endif
        nfbi_mem32_read(DRAM_ADR, &val);
        if ((val & 0xFF00FF00) != 0x5A00A500) {
          //dprintf("\n\n\nError!DQS1(i=%d),(DDCR=0x%x)\n", i,READ_MEM32(DDCR_REG));
          // dprintf("DRAM(0x%x)=%x\n\n\n", DRAM_ADR,READ_MEM32(DRAM_ADR));
          R1 = i - 1;
          //R1 = i - 3;
          break;
        }
      }
    }

    DDCR_VAL = (DDCR_VAL & 0xFE000000) | (((L1 + R1) >> 1) << 20); // ASIC
    printf("\nDDCR_VAL(1) =%x\n", DDCR_VAL);
    
    nfbi_mem32_write(DDCR_ADR,  DDCR_VAL);
    nfbi_mem32_write(DDCR_ADR,  DDCR_VAL);

    /* wait a little bit time, necessary */
    //real_sleep(1);
#if 1
    printf("\nR0:%d L0:%d C0:%d\n", R0, L0, (L0 + R0) >> 1);
    printf("\nR1:%d L1:%d C1:%d\n", R1, L1, (L1 + R1) >> 1);
    nfbi_mem32_read(DRAM_ADR, &val);
    printf("\n=>After NFBI Cali,DDCR(%d)=0x%x\n\n", k , val);
    //printf("\n=================================\n");
#endif

	/*DDR_Calibration_Guard*/
	nfbi_mem32_write(DRAM_ADR, DRAM_VAL);   // set R/W pattern
	nfbi_mem32_read(DRAM_ADR, &val);
	if ((val & 0xFFFFFFFF) == 0x5A5AA5A5) {
		//goto DDR_Calibration_Guard;
		printf("###NFBI DDR calibration ok###\n");
		break;
	}

  }//end of while(1)
}

