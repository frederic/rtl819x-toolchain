/*
 *
 *  Copyright (c) 2011 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

/*================================================================*/
/* System Include Files */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/ioport.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/circ_buf.h>
#include <asm/uaccess.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>
#include <linux/interrupt.h>
#include "bspchip.h"


/*================================================================*/
/* Local Include Files */
															
#include "rtl_mdio.h"

//#define CONFIG_RTL8197B_B_CUT_PATCH
/*================================================================*/
/* Global Variables */

int cpu_suspend_enabled = 0;


/*================================================================*/
/* Local Variables */
static struct mdio_priv *dev_priv=NULL;

#ifdef SIMULATION
static unsigned char data_in[256];
static int data_in_len = 0, data_in_read_idx = 0;

static unsigned short data_out;
static int data_out_len = 0, msg_is_fetched = 0;
static unsigned long reg_scr =0, reg_isr=0; 
#endif




/*================================================================*/
#if 0
int pre_jiffies=1;

void* MAX_FUNCT;
unsigned int MAX_STACK=8*1024;
unsigned int PRINT_STACK=2428;
unsigned int COPY_STACK=2428;

unsigned int MAX_FUNCT_call[40];

#define PRINT_INTV (1*60*100)
/*  print max sp ever 3min  */
//extern volatile int jiffies;


void __attribute__((__no_instrument_function__))
__cyg_profile_func_enter(void *this_func, void *call_site)
{
	unsigned int sp_addr;
	unsigned int sp_size;

#if 0
	if (this_func == rtk_voip_dsp_init)
	{
		printk("rtk_voip_dsp_init enter\n");
	}
#endif

	__asm__ __volatile__("ori %0, $29, 0": "=r"(sp_addr) );

	sp_size = sp_addr & 8191;

	if(MAX_STACK > sp_size)
	{
		MAX_STACK = sp_size;
		MAX_FUNCT = this_func;

		if (COPY_STACK > sp_size) {
			COPY_STACK = sp_size;
			copy_trace(sp_addr);
		}


	}

	if ( ((int)jiffies - pre_jiffies) > PRINT_INTV ) {

		pre_jiffies = jiffies;
		printk("MAXSP,%x,%d.", MAX_FUNCT, MAX_STACK);
		
		if (PRINT_STACK > MAX_STACK) {
			int i;
			unsigned long flags;
			save_flags(flags); cli();
			PRINT_STACK = MAX_STACK;
			printk("\nCall Trace:");
			
			for (i=0 ; i<40 ; i++) {
				if (0==MAX_FUNCT_call[i])
					break;
				printk(" [<%08lx>]", MAX_FUNCT_call[i]);
				if ( 4==(i%5))
					printk("\n");
			}
			restore_flags(flags);
		}
		MAX_STACK = 8*1024;
	}
}

void __attribute__((__no_instrument_function__))
__cyg_profile_func_exit(void *this_func, void *call_site)
{
#if 0	
    if (this_func == rtk_voip_dsp_init)
    {
		printk("rtk_voip_dsp_init exit\n");
    }
#endif    
}

#include <asm-mips/uaccess.h>

void __attribute__((__no_instrument_function__)) copy_trace(unsigned int *sp)
{
	int i;
	//int column = 0;
	unsigned int *stack;
	unsigned long kernel_start, kernel_end;

	extern char _stext, _etext;

	stack = sp ;
	i = 0;

	kernel_start = (unsigned long) &_stext;
	kernel_end = (unsigned long) &_etext;


	//printk("\nCall Trace:");

	while ((unsigned long) stack & (PAGE_SIZE -1)) {
		unsigned long addr;

		if (__get_user(addr, stack++)) {
			printk(" (Bad stack address)\n");
			break;
		}

		/*
		 * If the address is either in the text segment of the
		 * kernel, or in the region which contains vmalloc'ed
		 * memory, it *may* be the address of a calling
		 * routine; if so, print it so that someone tracing
		 * down the cause of the crash will be able to figure
		 * out the call path that was taken.
		 */

		if (addr >= kernel_start && addr < kernel_end) { 

			MAX_FUNCT_call[i]=addr;
			//printk(" [<%08lx>]", addr);
			//if (column++ == 5) {
			//	printk("\n");
			//	column = 0;
			//}
			if (++i > 40) {
				//printk(" ...");
				break;
			}
		}
	
		
	}

	//if (column != 0)
	//	printk("\n");

	for ( ; i<40; i++)
		MAX_FUNCT_call[i] = 0;
}
#endif
/*================================================================*/




/*================================================================*/
/* Routine Implementations */

#ifdef SIMULATION
static unsigned long register_read_dw(int offset)
{
	unsigned long status = 0;
	unsigned short wdata = 0;

	if (offset == REG_ISR) {
		status = reg_isr;		
		
		if (data_in_len > data_in_read_idx) 
			status |= IP_NEWMSG;

		if (msg_is_fetched) {
			status |= IP_MSGFETCH;	
			msg_is_fetched = 0;
		}

		return status;			
	}	
	else if (offset == REG_RCR) {		
		ASSERT(data_in_len > data_in_read_idx);
		memcpy(&wdata, &data_in[data_in_read_idx], 2);				
		data_in_read_idx += 2;
		status |= wdata;			
		return status;			
	}
	else if (offset == REG_SYSCR) {		
		return reg_scr;		
	}	
	else {
		ASSERT(0);
		return status;	
	}	
}

static void register_write_dw(int offset, unsigned long data)
{
	if (offset ==	REG_SSR) {
		unsigned short wData = (unsigned short)data;
		
		memcpy(&data_out, &wData, 2);	
		data_out_len = 2;
	}	
	else if (offset ==	REG_ISR) {
		reg_isr &= ~data ;
	}
}
#endif // SIMULATION

#ifdef KDB_MSG
static void inline debugk_out(unsigned char *label, unsigned char *data, int data_length)
{
    int i,j;
    int num_blocks;
    int block_remainder;

    num_blocks = data_length >> 4;
    block_remainder = data_length & 15;

	if (label) 
	    DEBUGK_OUT("%s\n", label);	

	if (data==NULL || data_length==0)
		return;

    for (i=0; i<num_blocks; i++) {   
        printk("\t");
        for (j=0; j<16; j++) 
            printk("%02x ", data[j + (i<<4)]);
        printk("\n");
    }

    if (block_remainder > 0) {    
        printk("\t");
        for (j=0; j<block_remainder; j++)
            printk("%02x ", data[j+(num_blocks<<4)]);
        printk("\n");
    }
}
#endif // KDB_MSG

unsigned long static get_ether_phy_reg(unsigned long phyId, unsigned long regId)
{
	unsigned long status;
	
	WRITE_MEM32((void *)MDCIOCR, COMMAND_READ | ( phyId << PHYADD_OFFSET ) | ( regId << REGADD_OFFSET ));

	do { status = READ_MEM32( MDCIOSR ); } while ( ( status & MDCIO_STATUS ) != 0 );

	status &= 0xffff;
	return status;
}

static void set_ether_phy_reg(unsigned long phyId, unsigned long regId, unsigned long wData)
{
	WRITE_MEM32(MDCIOCR, COMMAND_WRITE | ( phyId << PHYADD_OFFSET ) | ( regId << REGADD_OFFSET ) | wData);

	while( ( READ_MEM32( MDCIOSR ) & MDCIO_STATUS ) != 0 );		/* wait until command complete */
}

static int is_checksum_ok(unsigned char *data, int len)
{
	int i;
	unsigned char sum=0;

	for (i=0; i<len; i++)
		sum += data[i];

	if (sum == 0)
		return 1;
	else
		return 0;
}

static unsigned char append_checksum(unsigned char *data, int len)
{
	int i;
	unsigned char sum=0;

	for (i=0; i<len; i++)
		sum += data[i];

	sum = ~sum + 1;
	return sum;
}

static int indicate_evt(struct mdio_priv *priv, int id, unsigned char *data, int data_len)
{
	int size;

	size = CIRC_SPACE(priv->evt_que_head, priv->evt_que_tail, EV_QUE_MAX);
	if (size == 0) {
		DEBUGK_ERR("Indication queue full, drop event!\n");

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
	if (priv->host_pid != -1)
		kill_proc(priv->host_pid, SIGUSR1, 1);
#else
	if (priv->host_pid != NULL)
		kill_pid(priv->host_pid, SIGUSR1, 1);
#endif

		return 0;
	}
	
	ASSERT(data_len < MDIO_BUFSIZE);
		
	priv->ind_evt_que[priv->evt_que_head].id = id;
	priv->ind_evt_que[priv->evt_que_head].len = data_len;	
	memcpy(&priv->ind_evt_que[priv->evt_que_head].buf, data, data_len);
	priv->evt_que_head = (priv->evt_que_head + 1) & (EV_QUE_MAX - 1);
#if 0 //mark_nfbi , now we use polling instead of singal in HCD
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
	if (priv->host_pid != -1)
		kill_proc(priv->host_pid, SIGUSR1, 1);
#else
	if (priv->host_pid != NULL)
		kill_pid(priv->host_pid, SIGUSR1, 1);
#endif
#endif    
	return 1;
}

static int retrieve_evt(struct mdio_priv *priv, unsigned char *out)
{
	int len = 0;
	
	if (CIRC_CNT(priv->evt_que_head, priv->evt_que_tail, EV_QUE_MAX) > 0) { // more than one evt pending
		len = EVT_BUF_OFFSET + priv->ind_evt_que[priv->evt_que_tail].len;		
		memcpy(out, &priv->ind_evt_que[priv->evt_que_tail], len);
		priv->evt_que_tail = (priv->evt_que_tail + 1) & (EV_QUE_MAX - 1);		
	}
	return len;
}

static void process_rx_cmd(struct mdio_priv *priv, unsigned short data)
{
	if (priv->rx_cmd_time && 
			(priv->rx_cmd_state == STATE_RX_WAIT_LEN ||
				priv->rx_cmd_state == STATE_RX_WAIT_DATA) &&				
				(priv->cmd_timeout && TIME_DIFF(jiffies, priv->rx_cmd_time) > priv->cmd_timeout)) {
		DEBUGK_ERR("Rx cmd timeout [%ld][0x%x], discard pending cmd!\n", TIME_DIFF(jiffies, priv->rx_cmd_time), data);
		RESET_RX_STATE;
		return;
	}
	
	if ((data & FIRST_CMD_MASK) && (priv->rx_cmd_state != STATE_RX_INIT)) {
		DEBUGK_ERR("Rx Sync bit but not in INIT_STATE [%d][0x%x], discard pending cmd!\n", priv->rx_cmd_state, data);
		RESET_RX_STATE;
	}

	if (priv->rx_cmd_state == STATE_RX_INIT) {
		ASSERT(priv->data_in.len == 0);
		
		if (!(data & FIRST_CMD_MASK)) {
			DEBUGK_ERR("Got invalid rx cmd id [0x%x], discard it!\n", data);
			goto invalid_cmd;
		}
		PUT_IN_DATA(data);	// cmd id
		priv->rx_cmd_state = STATE_RX_WAIT_LEN;
		priv->rx_cmd_time = jiffies;
	}
	else  { // STATE_RX_WAIT_LEN or STATE_RX_WAIT_DATA
		// check if first cmd byte is '0'	
		if (data & 0xff00) {
			DEBUGK_ERR("1st byte of rx cmd not zero [%x]!\n", data >> 8);
			goto invalid_cmd;			
		}
		
		if (priv->rx_cmd_state == STATE_RX_WAIT_LEN) {
			PUT_IN_DATA(data);		
			priv->rx_cmd_state = STATE_RX_WAIT_DATA;
			priv->rx_cmd_remain_len = (data + 1)*2;	// including checksum		
			priv->rx_cmd_time = jiffies;			
		}
		else { // in STATE_RX_WAIT_DATA
			ASSERT (priv->rx_cmd_remain_len > 0);

			PUT_IN_DATA(data);				
			priv->rx_cmd_remain_len -= 2;		
			if (priv->rx_cmd_remain_len  <= 0) { // rx last bye, calcuate checksum
				if (!is_checksum_ok(priv->data_in.buf, priv->data_in.len)) {
					DEBUGK_ERR("Rx cmd cheksum error!\n");
					goto invalid_cmd;					
				}				
				priv->data_in.len -= 2; // substract checksum length

				indicate_evt(priv, IND_CMD_EV, priv->data_in.buf, priv->data_in.len);

				RESET_RX_STATE;				
			}
			else 
				priv->rx_cmd_time = jiffies;
		}		
	}
	
	return;
	
invalid_cmd:
	RESET_RX_STATE;
}

static void transmit_msg(struct mdio_priv *priv)
{
	unsigned short data;

	if (priv->data_out.len <= 0 || priv->tx_status_transmitting_len >= priv->data_out.len) 
		return;

	memcpy(&data, priv->data_out.buf+priv->tx_status_transmitting_len, 2);
	register_write_dw(REG_SSR, (unsigned long)data);

	priv->tx_status_transmitting_len += 2;

	if (priv->tx_status_transmitting_len >= priv->data_out.len) 
		priv->tx_status_state = STATE_TX_INIT;
	else
		priv->tx_status_state = STATE_TX_IN_PROGRESS;	
}

#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP_IS_DSP

static irqreturn_t mdio_interrupt(int irq, void *dev_id)
{
	struct mdio_priv *priv = (struct mdio_priv *)dev_id;

	unsigned long status, data;

	while (1) {
		status = register_read_dw(REG_ISR);
		if (!status)
			break;
		
		register_write_dw(REG_ISR, status);	// clear interrupt

		unsigned long st_dsp_id;
		extern int Set_Ethernet_DSP_ID(unsigned char dsp_id);
		extern unsigned int Get_Ethernet_DSP_ID(void);
		printk("in mdio_interrupt, status = 0x%x\n", status);

		if (status & 0x8)//bit3
		{
			data = register_read_dw(REG_SYSCR) & 0xF;// use SYSCR bit 0~3 to decide DSP ID for each DSP. up to 16 DSP.
			
			if((data>=0) && (data<=3))
			{
			Set_Ethernet_DSP_ID((unsigned char)data);
			//printk("Set DSP ID to %d\n", data);
				
				// Singal AP process to chage DSP MAC addr
				printk("signal to hcd to change MAC Addr\n");
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
				if (priv->host_pid != -1)
				       kill_proc(priv->host_pid, SIGUSR2, 1);		
#else
				if (priv->host_pid != -1)
				       kill_pid(priv->host_pid, SIGUSR2, 1);		
#endif
			}
			else
				printk("%s: NOT support dsp_id=%d\n", __FUNCTION__, data);
		}
		else
		{
			printk("Get unknown mdio interrupt status = 0x%x\n", status);
		}

		if (status & IP_NEWMSG) {
			data = register_read_dw(REG_RCR);			
			process_rx_cmd(priv, (unsigned short)data);					
		}
		if (status & IP_MSGFETCH) {
			transmit_msg(priv);	
		}
		if (status & (IP_ISOLATION|IP_ETHMAC|IP_WLANMAC|IP_ETHPHY |IP_WLANPHY|IP_SELMIICLK)) {
			data = register_read_dw(REG_SYSCR);
			data &= (CR_ISOLATION |CR_ETHMAC|CR_WLANMAC|CR_ETHPHY|CR_WLANPHY|CR_SELMIICLK);
			indicate_evt(priv, IND_SYSCTL_EV, (unsigned char *)&data, sizeof(data));							
		}
#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP_IS_DSP
		if (status  & ~(IP_MSGFETCH|IP_NEWMSG|IP_ISOLATION|IP_ETHMAC|
								IP_WLANMAC|IP_ETHPHY |IP_WLANPHY |IP_SELMIICLK |IP_CUSTOM3)) {
#else
		if (status  & ~(IP_MSGFETCH|IP_NEWMSG|IP_ISOLATION|IP_ETHMAC|
								IP_WLANMAC|IP_ETHPHY |IP_WLANPHY|IP_SELMIICLK)) {
#endif
			DEBUGK_ERR("Got satus=0x%x, not supported yet!\n", (unsigned int)status);	
		}
	}	
	return IRQ_HANDLED;	
}

#else

//static void mdio_interrupt(int irq, void *dev_id, struct pt_regs *regs)
static irqreturn_t mdio_interrupt(int irq, void *dev_id)
{
	struct mdio_priv *priv = (struct mdio_priv *)dev_id;

	unsigned long status, data;

	while (1) {
		status = register_read_dw(REG_ISR);
		if (!status)
			break;
		
		register_write_dw(REG_ISR, status);	// clear interrupt

		if (status & IP_NEWMSG) {
			data = register_read_dw(REG_RCR);			
			process_rx_cmd(priv, (unsigned short)data);					
		}
		if (status & IP_MSGFETCH) {
			transmit_msg(priv);	
		}
		if (status & (IP_ISOLATION|IP_ETHMAC|IP_WLANMAC|IP_ETHPHY |IP_WLANPHY|IP_SELMIICLK)) {
			data = register_read_dw(REG_SYSCR);
			data &= (CR_ISOLATION |CR_ETHMAC|CR_WLANMAC|CR_ETHPHY|CR_WLANPHY|CR_SELMIICLK);
			indicate_evt(priv, IND_SYSCTL_EV, (unsigned char *)&data, sizeof(data));
		}
		if (status  & ~(IP_MSGFETCH|IP_NEWMSG|IP_ISOLATION|IP_ETHMAC|
								IP_WLANMAC|IP_ETHPHY |IP_WLANPHY|IP_SELMIICLK)) {
			DEBUGK_ERR("Got satus=0x%x, not supported yet!\n", (unsigned int)status);
		}
	}
	return IRQ_HANDLED;	
}

#endif

void toggle_usb_device_insert_bit(void)
{
    unsigned long flags;
    
    if (dev_priv == NULL)
        return;
    spin_lock_irqsave(&dev_priv->reglock, flags);
	if (REG32(REG_SYSSR) & SR_USBInsertStatus)
	    REG32(REG_SYSSR) =  REG32(REG_SYSSR) & (~SR_USBInsertStatus);
	else
	    REG32(REG_SYSSR) =  REG32(REG_SYSSR) | SR_USBInsertStatus;
	spin_unlock_irqrestore(&dev_priv->reglock, flags);
    //printk("usb device inserted\n");
}

void toggle_usb_device_remove_bit(void)
{
    unsigned long flags;
    
    if (dev_priv == NULL)
        return;
    spin_lock_irqsave(&dev_priv->reglock, flags);
	if (REG32(REG_SYSSR) & SR_USBRemoveStatus)
	    REG32(REG_SYSSR) =  REG32(REG_SYSSR) & (~SR_USBRemoveStatus);
	else
	    REG32(REG_SYSSR) =  REG32(REG_SYSSR) | SR_USBRemoveStatus;
	spin_unlock_irqrestore(&dev_priv->reglock, flags);
    //printk("usb device removed\n");
}

void set_wlanlink_bit(int val)
{
    unsigned long flags;
    
    if (dev_priv == NULL)
        return;
    spin_lock_irqsave(&dev_priv->reglock, flags);
	if (val)
	    REG32(REG_SYSSR) =  REG32(REG_SYSSR) | SR_WLANLink;
	else
	    REG32(REG_SYSSR) =  REG32(REG_SYSSR) & (~SR_WLANLink);
	spin_unlock_irqrestore(&dev_priv->reglock, flags);
}

static void mdio_reg_poll_timer(unsigned long task_priv)
{
    unsigned long flags;
    unsigned long reg1, reg2;
	struct mdio_priv *priv = (struct mdio_priv *)task_priv;

    if (!priv->poll_timer_up)
        return;
        
	if 	(priv->reg_BMCR_write != REG32(REG_BMCR)) {
	    //printk("1. priv->reg_BMCR_write=%x\n", priv->reg_BMCR_write);
	    reg1 = reg2 = REG32(REG_BMCR);
        if ((reg1&0x8000) && (priv->reg_BMCR_write&0x8000)) //check "reset" bit
            reg1 &= 0x7fff;
        if ((reg1&0x0200) && (priv->reg_BMCR_write&0x0200)) //check "restart auto negotiation" bit
            reg1 &= 0xfdff;

        if (priv->force_power_down)
            reg1 |= 0x0800; //power down
		priv->reg_BMCR_write = reg2;
		set_ether_phy_reg(ETH_PORT_NUM, 0, reg1);
        if (reg1 & 0x1000) {
            if ((priv->reg_ANAR_write & 0x0180) == 0x0000)
    		    set_ether_phy_reg(6, 0, 0x120c);
    		else
    		    set_ether_phy_reg(6, 0, 0x1208);
        }
#ifdef CONFIG_RTL8197B_B_CUT_PATCH
        else {
    		if (reg1 & 0x2000) //100M
    		    set_ether_phy_reg(6, 0, 0x120c);
    		else               //10M
    		    set_ether_phy_reg(6, 0, 0x1208);
        }
#endif
		//printk("2. priv->reg_BMCR_write=%x\n", priv->reg_BMCR_write);
	}
	
	if (priv->reg_BMCR_read != get_ether_phy_reg(ETH_PORT_NUM, 0)) {
	    //printk("1. priv->reg_BMCR_read=%x\n", priv->reg_BMCR_read);
		priv->reg_BMCR_read = get_ether_phy_reg(ETH_PORT_NUM, 0);
		if (priv->reg_BMCR_write & 0x0800) //power down
		    REG32(REG_BMCR) = priv->reg_BMCR_read;
        else
            REG32(REG_BMCR) = priv->reg_BMCR_read & 0xf7ff;
		//printk("2. priv->reg_BMCR_read=%x\n", priv->reg_BMCR_read);
	}
	
	if (priv->reg_BMSR_read != get_ether_phy_reg(ETH_PORT_NUM, 1)) {
	    reg1 = get_ether_phy_reg(ETH_PORT_NUM, 1);
	    if ((priv->reg_BMSR_read ^ reg1) & (~BIT(2))) {
	        //toggle EthPHYStatusChange bit to generate interrupt
            spin_lock_irqsave(&priv->reglock, flags);
	        if (REG32(REG_SYSSR) & SR_EthPHYStatusChange)
	            REG32(REG_SYSSR) =  REG32(REG_SYSSR) & (~SR_EthPHYStatusChange);
	        else
	            REG32(REG_SYSSR) =  REG32(REG_SYSSR) | SR_EthPHYStatusChange;
	        spin_unlock_irqrestore(&priv->reglock, flags);
			//printk("EthPhy status changed!\n");
	    }
	    
		priv->reg_BMSR_read = reg1;
		REG32(REG_BMSR) = priv->reg_BMSR_read;

		if (priv->eth_phy_link_status != ((priv->reg_BMSR_read & BIT(2)) ? 1 : 0)) {
			priv->eth_phy_link_status = ((priv->reg_BMSR_read & BIT(2)) ? 1 : 0);
			spin_lock_irqsave(&priv->reglock, flags);
			if (priv->eth_phy_link_status)
			    REG32(REG_SYSSR) =  REG32(REG_SYSSR) | SR_EthLink;
			else
			    REG32(REG_SYSSR) =  REG32(REG_SYSSR) & (~SR_EthLink);
            spin_unlock_irqrestore(&priv->reglock, flags);			    
			//DEBUGK_OUT("Ether Link changed [%d]!\n", priv->eth_phy_link_status);
			//printk("Ether Link changed [%d]!\n", priv->eth_phy_link_status);
		}
	}
    
    if (!(priv->reg_BMCR_write&0x8000)) {
    	if (priv->reg_ANAR_write != REG32(REG_ANAR)) {
    	    //printk("1. priv->reg_ANAR_write=%x\n", priv->reg_ANAR_write);
    		priv->reg_ANAR_write = REG32(REG_ANAR);
    		//printk("2. priv->reg_ANAR_write=%x\n", priv->reg_ANAR_write);
    		set_ether_phy_reg(ETH_PORT_NUM, 4, priv->reg_ANAR_write);
    		//printk("3. priv->reg_ANAR_write=%x\n", priv->reg_ANAR_write);
    		if (priv->reg_BMCR_write&0x1000) {
        		if ((priv->reg_ANAR_write & 0x0180) == 0x0000)
        		    set_ether_phy_reg(6, 0, 0x120c);
        		else
        		    set_ether_phy_reg(6, 0, 0x1208);
            }
    	}
    }
    
	if (priv->reg_ANAR_read != get_ether_phy_reg(ETH_PORT_NUM, 4)) {
	    //printk("1. priv->reg_ANAR_read=%x\n", priv->reg_ANAR_read);
		priv->reg_ANAR_read = get_ether_phy_reg(ETH_PORT_NUM, 4);
		//printk("2. priv->reg_ANAR_read=%x\n", priv->reg_ANAR_read);
		REG32(REG_ANAR) = priv->reg_ANAR_read;
	}

	if (priv->reg_ANLPAR_read != get_ether_phy_reg(ETH_PORT_NUM, 5)) {
		priv->reg_ANLPAR_read = get_ether_phy_reg(ETH_PORT_NUM, 5);
		REG32(REG_ANLPAR) = priv->reg_ANLPAR_read;						
	}

	mod_timer(&priv->reg_poll_timer, jiffies + priv->phy_reg_poll_time);
}

static int mdio_open(struct inode *inode, struct file *filp)
{
	filp->private_data = dev_priv;
	//MOD_INC_USE_COUNT;
	return 0;
}

static int mdio_close(struct inode *inode, struct file *filp)
{
	//MOD_DEC_USE_COUNT;
	return 0;
}

static ssize_t mdio_read (struct file *filp, char *buf, size_t count, loff_t *offset)
{
	struct mdio_priv *priv = (struct mdio_priv *)filp->private_data;

	if (!buf)
		return 0;

	count = retrieve_evt(priv, buf);

	return count;
}

static ssize_t mdio_write (struct file *filp, const char *buf, size_t count, loff_t *offset)
{
	struct mdio_priv *priv = (struct mdio_priv *)filp->private_data;
	unsigned short last_word = 0;

	if (!buf) {
		DEBUGK_ERR("buf = NULL!\n");		
		goto ret;
	}

	if (count > MDIO_BUFSIZE) {
		DEBUGK_ERR("write length too big!\n");		
		count = -EFAULT;
		goto ret;
	}

	if (priv->tx_status_state != STATE_TX_INIT) {
		DEBUGK_ERR("Transmit status, but not in valid state [%d]. Reset state!\n", priv->tx_status_state);		
		priv->tx_status_state = STATE_TX_INIT;
	}		
	
	if (count %2) {
		DEBUGK_ERR("Invalid Tx size [%d]!\n", count);
		count = -EFAULT;		
		goto ret;
	}

	if (copy_from_user((void *)priv->data_out.buf, buf, count)) {
		DEBUGK_ERR("copy_from_user() error!\n");		
		count = -EFAULT;
		goto ret;		
	}
	
#ifdef KDB_MSG
	debugk_out("write data", priv->data_out.buf, count);
#endif

	last_word = (unsigned short)	append_checksum(priv->data_out.buf, count);
	memcpy(&priv->data_out.buf[count], &last_word, 2);	
		
	priv->data_out.len = count + sizeof(last_word);		
	priv->tx_status_transmitting_len = 0;
	priv->tx_status_state = STATE_TX_INIT;

	transmit_msg(priv);
	
ret:
	return count;
}

static void dump_private_data(void)
{
	//int i;

	printk("cmd_timeout=%d\n", dev_priv->cmd_timeout);
	printk("poll_timer_up=%d\n", dev_priv->poll_timer_up);
	printk("phy_reg_poll_time=%d\n", dev_priv->phy_reg_poll_time);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
	printk("host_pid=%d\n", dev_priv->host_pid);
#else
	printk("host_pid=0x%p\n", dev_priv->host_pid);
#endif	
	printk("force_power_down=%d\n", dev_priv->force_power_down);
	printk("eth_phy_link_status=%d\n", dev_priv->eth_phy_link_status);
	printk("\nreg_BMCR_write=0x%04lx\n", dev_priv->reg_BMCR_write);
	printk("reg_BMCR_read=0x%04lx\n", dev_priv->reg_BMCR_read);
	printk("reg_BMSR_read=0x%04lx\n", dev_priv->reg_BMSR_read);
	printk("reg_ANAR_write=0x%04lx\n", dev_priv->reg_ANAR_write);
	printk("reg_ANAR_read=0x%04lx\n", dev_priv->reg_ANAR_read);
	printk("reg_ANLPAR_read=0x%04lx\n", dev_priv->reg_ANLPAR_read);

	printk("\nevt_que_head=%d\n", dev_priv->evt_que_head);
	printk("evt_que_tail=%d\n", dev_priv->evt_que_tail);
	printk("evt_que count=%d\n", CIRC_CNT(dev_priv->evt_que_head, dev_priv->evt_que_tail, EV_QUE_MAX));
	printk("evt_que space=%d\n", CIRC_SPACE(dev_priv->evt_que_head, dev_priv->evt_que_tail, EV_QUE_MAX));
	
	printk("\ntx_status_state:");
	switch(dev_priv->tx_status_state) {
    	  case STATE_TX_INIT:
    	  	printk("STATE_TX_INIT\n");
    	  	break;
    	  case STATE_TX_IN_PROGRESS:
    	  	printk("STATE_TX_IN_PROGRESS\n");
    	  	break;
	}
	printk("tx_status_transmitting_len=%d\n", dev_priv->tx_status_transmitting_len);
    
	printk("rx_cmd_state:");
	switch(dev_priv->rx_cmd_state) {
          case STATE_RX_INIT:
    	  	printk("STATE_RX_INIT\n");
    	  	break;
    	  case STATE_RX_WAIT_LEN:
    	  	printk("STATE_RX_WAIT_LEN\n");
    	  	break;
    	  case STATE_RX_WAIT_DATA:
    	  	printk("STATE_RX_WAIT_DATA\n");
    	  	break;
    	  case STATE_RX_WAIT_DAEMON:
    	  	printk("STATE_RX_WAIT_DAEMON\n");
    	  	break;
	}
   	printk("rx_cmd_remain_len=%d\n", dev_priv->rx_cmd_remain_len);
	printk("rx_cmd_time=%lx jiffies=%lx (%d sec)\n", dev_priv->rx_cmd_time, jiffies, (int)(jiffies-dev_priv->rx_cmd_time)/HZ);
}

void mdio_private_command(int type)
{
    switch(type) {
      case 0: //stop reg poll timer
        dev_priv->poll_timer_up = 0;
        break;
      case 1: //start reg poll timer
        dev_priv->poll_timer_up = 1;
        mod_timer(&dev_priv->reg_poll_timer, jiffies + dev_priv->phy_reg_poll_time);
        break;
      case 2: //force power down: on
        dev_priv->force_power_down = 1;
        dev_priv->reg_BMCR_write = 0xf0000; //force poll timer to do power down
        break;
      case 3: //force power down: off
        dev_priv->force_power_down = 0;
        dev_priv->reg_BMCR_write = 0xf0000; //force poll timer to do power down
        break;
      case 4:
        dump_private_data();
        break;
      case 5: //WLAN link up
        set_wlanlink_bit(1);
        break;
      case 6: //WLAN link down
        set_wlanlink_bit(0);
        break;
    }
}

static int mdio_ioctl(struct inode *inode, struct file *filp,
                 unsigned int cmd, unsigned long arg)
{
    unsigned long flags;
	struct mdio_priv *priv = (struct mdio_priv *)filp->private_data;
	int val, retval = 0;
	unsigned char bval;
	struct mdio_mem32_param mem_param;
	struct reg_param regparam;

	/*
	 * extract the type and number bitfields, and don't decode
	 * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
	 */
	if (_IOC_TYPE(cmd) != MDIO_IOC_MAGIC) return -ENOTTY;
	if (_IOC_NR(cmd) > MDIO_IOCTL_MAXNR) return -ENOTTY;

	switch (cmd) {
	    case MDIO_IOCTL_PRIV_CMD:
    	    retval = get_user(val, (int *)arg);
    	    if (retval == 0) {
    	        mdio_private_command(val);
    	    }
    	    break;
    	case MDIO_IOCTL_GET_REG:
        	retval = copy_from_user(&regparam, (struct reg_param *)arg, sizeof(struct reg_param));
        	if (retval == 0) {
        	    //printk("1regparam.addr=%x regparam.val=%x\n", regparam.addr, regparam.val);
        	    regparam.val = get_ether_phy_reg(((regparam.addr>>16)&0xffff), (regparam.addr&0xffff));
        	    //printk("2regparam.addr=%x regparam.val=%x\n", regparam.addr, regparam.val);
    	    }
    	    retval = copy_to_user((struct reg_param *)arg, &regparam, sizeof(struct reg_param));
    	    break;
        case MDIO_IOCTL_SET_REG:
        	retval = copy_from_user(&regparam, (struct reg_param *)arg, sizeof(struct reg_param));
        	if (retval == 0) {
        	    //printk("regparam.addr=%x regparam.val=%x\n", regparam.addr, regparam.val);
        	    set_ether_phy_reg(((regparam.addr>>16)&0xffff), (regparam.addr&0xffff), regparam.val);
    	    }
    	    break;
		case MDIO_IOCTL_SET_HOST_PID:
			retval = copy_from_user((void *)&val, (void *)arg, 4);
			if (retval == 0) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
				priv->host_pid = val;
#elif LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30)
				priv->host_pid = find_pid_ns(val,0);
#else
				// TODO : Check in stable 2.6.27
				priv->host_pid = task_pid(find_task_by_vpid(val));
				//priv->host_pid = task_pid(current);
#endif
				DEBUGK_OUT("set pid=%d\n", val);
				printk("set pid=%d priv->host_pid=%p\n", val, priv->host_pid);
			}
			spin_lock_irqsave(&priv->reglock, flags);
			REG32(REG_SYSSR) =  REG32(REG_SYSSR) | SR_AllSoftwareReady;
			spin_unlock_irqrestore(&priv->reglock, flags);
			break;			
		case MDIO_IOCTL_SET_CMD_TIMEOUT:
			retval = copy_from_user((void *)&bval, (void *)arg, 1);
			if (retval == 0) {
				priv->cmd_timeout = (int)bval;				
				DEBUGK_OUT("set cmd_timeout=%d\n", priv->cmd_timeout); 
			}
			break;			
		case MDIO_IOCTL_SET_PHY_POLL_TIME:
			retval = copy_from_user( (void *)&bval, (void *)arg, 1);
			if (retval == 0) {
				priv->phy_reg_poll_time = (int)bval;
				mod_timer(&priv->reg_poll_timer, jiffies + priv->phy_reg_poll_time);
				DEBUGK_OUT("set poll_time=%d\n", priv->phy_reg_poll_time); 
			}
			break;			
		case MDIO_IOCTL_READ_MEM:
			retval = copy_from_user( (void *)&mem_param.addr, (void *)arg, 4);
			if (retval == 0) {
				mem_param.val =	READ_MEM32(mem_param.addr);				
				retval = copy_to_user((void *)arg, (void *)&mem_param.val, 4);
				DEBUGK_OUT("read_mem: addr=0x%x, data=0x%x\n", (int)mem_param.addr, (int)mem_param.val); 
			}
			break;
		case MDIO_IOCTL_WRITE_MEM:
			retval = copy_from_user((void *)&mem_param, (void *)arg, sizeof(mem_param));
			if (retval == 0) {
				WRITE_MEM32(mem_param.addr, mem_param.val);
				DEBUGK_OUT("write_mem: addr=0x%x, data=0x%x\n", (int)mem_param.addr, (int)mem_param.val);
			}			
			break;
		case MDIO_IOCTL_SET_MII_PAUSE:
			retval = copy_from_user( (void *)&bval, (void *)arg, 1);
			if (retval == 0) {
				if (bval == 0) // disable pause
					WRITE_MEM32(PCRP0, (~(0x3<<PauseFlowControl))&READ_MEM32(PCRP0));
				else
					WRITE_MEM32(PCRP0, (3<<PauseFlowControl)|READ_MEM32(PCRP0));
				DEBUGK_OUT("set mii_pause=%d\n", bval); 			
			}
			break;
		case MDIO_IOCTL_SET_ETH_PAUSE:
			retval = copy_from_user( (void *)&bval, (void *)arg, 1);
			if (retval == 0) {
				if (bval == 0) // disable pause
					WRITE_MEM32(PCRP3, (~(0x3<<PauseFlowControl))&READ_MEM32(PCRP3));
				else
					WRITE_MEM32(PCRP3, (3<<PauseFlowControl)|READ_MEM32(PCRP3));
				DEBUGK_OUT("set eth_pause=%d\n", bval); 			
			}
			break;
		case MDIO_IOCTL_SET_MII_CLK:
			retval = copy_from_user( (void *)&val, (void *)arg, 4);
			if (retval == 0) {
				if (val == 0) { //00: 25MHz at 100M mode
					WRITE_MEM32(PCRP0, (~(0x3<<20))&READ_MEM32(PCRP0));     //force 100Mbps for port 0
					WRITE_MEM32(P0GMIICR, (~(1<<22))&READ_MEM32(P0GMIICR)); //Turbo MII off
				}
				else if (val == 1) { //01: 2.5MHz at 10M mode
					WRITE_MEM32(PCRP0, (1<<20)|((~(0x3<<20))&READ_MEM32(PCRP0))); //force 10Mbps for port 0
					WRITE_MEM32(P0GMIICR, (~(1<<22))&READ_MEM32(P0GMIICR));       //Turbo MII off
				}
				else if (val == 2) { //10: 50MHz at Turbo-MII mode
					WRITE_MEM32(PCRP0, (~(0x3<<20))&READ_MEM32(PCRP0));  //force 100Mbps for port 0
					WRITE_MEM32(P0GMIICR, (1<<22)|READ_MEM32(P0GMIICR)); //using Turbo MII
				}
				//printk("set mii clk=%d\n", val);
			}
			break;
		case MDIO_IOCTL_SET_SUSPEND:
			retval = copy_from_user( (void *)&bval, (void *)arg, 1);
			if (retval == 0) {
				cpu_suspend_enabled = (int) bval;
				DEBUGK_OUT("set cpu_suspend=%d\n", bval); 
			}
			break;			
		case MDIO_IOCTL_READ_SCR:
			val = register_read_dw(REG_SYSCR);			
			retval = copy_to_user((void *)arg, (void *)&val, sizeof(val));			
			DEBUGK_OUT("read_src src=0x%x\n", val); 
			break;
#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP_IS_DSP
		case MDIO_IOCTL_READ_DSP_ID:
		{
			unsigned int id;
			extern unsigned int Get_Ethernet_DSP_ID(void);
			id = Get_Ethernet_DSP_ID();			
			retval = copy_to_user((void *)arg, (void *)&id, sizeof(id));			
			printk("Get DSP ID = %d\n", id);
			break;
		}
#endif
#ifdef JUMP_CMD
		case MDIO_IOCTL_JUMP_ADDR:			
			retval = copy_from_user( (void *)&val, (void *)arg, sizeof(val));
			if (retval == 0) {						
				extern void setup_reboot_addr(unsigned long addr);
				extern int is_fault;
				
				DEBUGK_OUT("jump to addr=0x%x\n", val);
				setup_reboot_addr((unsigned long)val);
				is_fault = 1; // cause watchdog reset
			}
			break;
#endif			
	  default:  /* redundant, as cmd was checked against MAXNR */
			DEBUGK_ERR("Invalid ioctl cmd [0x%x]!\n", cmd);
			return -ENOTTY;
	}
	return retval;
}

#ifdef SIMULATION
static int read_proc(char *buf, char **start, off_t off,
				int count, int *eof, void *data)
{
	int size = 0;

	if (data_out_len > 0) {	
		
fetch_again:		
		while (data_out_len > 0) {
			data_out_len = 0;
			size += sprintf(&buf[size], "%04x ", data_out);			
		}

		msg_is_fetched = 1;
		
		mdio_interrupt(0, (void *)dev_priv, (struct pt_regs *)NULL);			

		if (data_out_len > 0)
			goto fetch_again;		

		strcat(&buf[size++], "\n");	
	}

    return size;
}

static unsigned short _atoi(char *s, int base)
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
				DEBUGK_ERR("error hex format [%x]!\n", *s);
				return 0;
			}
			k = 16 * k + v;
			s++;
		}
	}
	return (unsigned short)k;
}


static int write_proc(struct file *file, const char *buffer,
              unsigned long count, void *data)
{	
	char tmp[100];
	int len=count;
	unsigned short in_data;

	if (!memcmp(buffer, "cmd: ", 5)) {	
		buffer += 5;
		len -= 5;
		data_in_len = 0;
		while (len > 0) {
			memcpy(tmp, buffer, 4);
			tmp[4] = '\0';

			in_data = _atoi(tmp, 16);

			memcpy(&data_in[data_in_len], &in_data, 2);
			data_in_len += 2;
			len -= 5;
			buffer += 5;
		}

		data_in_read_idx = 0;
		mdio_interrupt(0, (void *)dev_priv, (struct pt_regs *)NULL);		
	}
	else if (!memcmp(buffer, "scr: ", 5)) {				
		buffer += 5;
		memcpy(tmp, buffer, 4);
		tmp[4] = '\0';
		reg_scr = _atoi(tmp, 16);
	}
	else if (!memcmp(buffer, "isr: ", 5)) {				
		buffer += 5;
		memcpy(tmp, buffer, 4);
		tmp[4] = '\0';
		reg_isr = _atoi(tmp, 16);
		
		mdio_interrupt(0, (void *)dev_priv, (struct pt_regs *)NULL);				
	}	
	else {
		printk("Invalid cmd!\n");		
	}
	
    return count;
}
#endif

 
static void __exit mdio_exit(void)
{
   	//DEBUGK_OUT("%s: major=%d, minor=%d\n", __FUNCTION__,  MAJOR(inode->i_rdev), MINOR(inode->i_rdev));

	REG32(REG_IMR) = 0;

	free_irq(BSP_NFBI_IRQ, dev_priv);

	del_timer_sync(&dev_priv->reg_poll_timer);

	kfree(dev_priv);
	
	dev_priv = NULL;
}

static struct file_operations mdio_fops = {
		read:		mdio_read, 
		write:		mdio_write,
		ioctl:		mdio_ioctl,		
		open:		mdio_open,
		release:	mdio_close,
};

static int __init mdio_init(void)
{
    struct mdio_priv *priv;	
    	
	if (register_chrdev(DRIVER_MAJOR, DRIVER_NAME, &mdio_fops)) {
		DEBUGK_ERR(KERN_ERR DRIVER_NAME": unable to get major %d\n", DRIVER_MAJOR);
		return -EIO;
	}	

#ifdef SIMULATION
	struct proc_dir_entry *res;
    res = create_proc_entry("mdio_flag", 0, NULL);
    if (res) {
   	    res->read_proc = read_proc;
   	    res->write_proc = write_proc;
	}
	else {
		DEBUGK_ERR(KERN_ERR DRIVER_NAME": unable to create /proc/mdio_flag\n");
		return -1;
	}		
#endif
	
	printk(KERN_INFO DRIVER_NAME" driver "DRIVER_VER" at %x (Interrupt %d)\n", NFBI_BASE, BSP_NFBI_IRQ);

	DEBUGK_OUT("%s: major=%d, minor=%d\n", __FUNCTION__, MAJOR(inode->i_rdev), MINOR(inode->i_rdev));

	priv  = (struct mdio_priv *)kmalloc(sizeof (struct mdio_priv), GFP_KERNEL);
	if(!priv)
		return -ENOMEM;	
	
	memset((void *)priv, 0, sizeof (struct mdio_priv));
	priv->reglock = SPIN_LOCK_UNLOCKED;
	//if (request_irq(BSP_NFBI_IRQ, mdio_interrupt, SA_INTERRUPT, DRIVER_NAME, (void *)priv)) {
	//if (request_irq(BSP_NFBI_IRQ, mdio_interrupt, IRQF_DISABLED, DRIVER_NAME, (void *)priv)) {
	if (request_irq(BSP_NFBI_IRQ, mdio_interrupt, 0, DRIVER_NAME, (void *)priv)) {
		DEBUGK_ERR(KERN_ERR DRIVER_NAME": IRQ %d is not free.\n", BSP_NFBI_IRQ);
		return -1;
	}
	else
		printk("Request BSP_NFBI_IRQ successfully.\n");

	REG32(REG_IMR) = NEEDED_IRQ_MASK;

#if 1
    //force the poll time to update the register
    priv->reg_BMCR_write = 0xf0000;
    priv->reg_BMCR_read = 0xf0000;
    priv->reg_BMSR_read = 0xf0000;
    priv->reg_ANAR_write = 0xf0000;
    priv->reg_ANAR_read = 0xf0000;
    priv->reg_ANLPAR_read = 0xf0000;
    priv->eth_phy_link_status = -1;
#else
	priv->reg_BMCR_write = REG32(REG_BMCR);
	set_ether_phy_reg(ETH_PORT_NUM, 0, priv->reg_BMCR_write);

	priv->reg_BMCR_read = get_ether_phy_reg(ETH_PORT_NUM, 0);
	REG32(REG_BMCR) = priv->reg_BMCR_read;

	priv->reg_BMSR_read = get_ether_phy_reg(ETH_PORT_NUM, 1);
	REG32(REG_BMSR) = priv->reg_BMSR_read;

	priv->reg_ANAR_write = REG32(REG_ANAR);
	set_ether_phy_reg(ETH_PORT_NUM, 4, priv->reg_ANAR_write);
	//printk("priv->reg_ANAR_write=%x\n", priv->reg_ANAR_write);
	
	priv->reg_ANAR_read = get_ether_phy_reg(ETH_PORT_NUM, 4);
	REG32(REG_ANAR) = priv->reg_ANAR_read;
	//printk("priv->reg_ANAR_read=%x\n", priv->reg_ANAR_read);
	
	priv->reg_ANLPAR_read = get_ether_phy_reg(ETH_PORT_NUM, 5);
	REG32(REG_ANLPAR) = priv->reg_ANLPAR_read;		

	priv->eth_phy_link_status = ((priv->reg_BMSR_read & BIT(2)) ? 1 : 0);	
#endif

	init_timer(&priv->reg_poll_timer);
	priv->reg_poll_timer.data = (unsigned long)priv;
	priv->reg_poll_timer.function = mdio_reg_poll_timer;
	priv->poll_timer_up = 0;
	priv->force_power_down = 0;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
	priv->host_pid = -1;
#else
	priv->host_pid = NULL;
#endif
	dev_priv = priv;

	return 0;
}


/*================================================================*/

module_init(mdio_init);
module_exit(mdio_exit);

MODULE_DESCRIPTION("Driver for RTL8197B MDC/MDIO");
MODULE_LICENSE("none-GPL");
//EXPORT_NO_SYMBOLS;
