/*
 *  RTL8197B NFBI char driver
 *
 *	Copyright (C)2008, Realtek Semiconductor Corp. All rights reserved.
 *
 */
/*================================================================*/
/* Include Files */

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
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include "rtl_nfbi.h"

#ifdef SIMULATION
	#include <linux/proc_fs.h>
#endif	

//#ifdef __LINUX_2_6__
struct module *nfbi_owner;
//#endif

#ifndef HOST_IS_PANABOARD
/*================================================================*/
/* RTL8651C MDC/MDIO Control Register, @Pana_TBD */
#define SWMACCR_BASE                (0xBB804000)
#define MDCIOCR                     (0x004+SWMACCR_BASE) /* MDC/MDIO Command Register */
#define MDCIOSR                     (0x008+SWMACCR_BASE) /* MDC/MDIO Status Register */

/* GPIO Register Set */
#define GPIO_BASE                   (0xB8003500)
#define PABCDCNR_REG				(0x000 + GPIO_BASE) /* Port ABCD control */
#define PABCDPTYPE_REG				(0x004 + GPIO_BASE) /* Port ABCD type */
#define PABCDDIR_REG				(0x008 + GPIO_BASE) /* Port ABCD direction*/
#define PABCDDAT_REG				(0x00C + GPIO_BASE) /* Port ABCD data */
#define PABCDISR_REG				(0x010 + GPIO_BASE) /* Port ABCD interrupt status */
#define PABIMR_REG				    (0x014 + GPIO_BASE) /* Port AB interrupt mask */
#define PCDIMR_REG				    (0x018 + GPIO_BASE) /* Port CD interrupt mask */
#define PEFGHCNR_REG				(0x01C + GPIO_BASE) /* Port EFGH control */
#define PEFGHPTYPE_REG				(0x020 + GPIO_BASE) /* Port EFGH type */
#define PEFGHDIR_REG				(0x024 + GPIO_BASE) /* Port EFGH direction*/
#define PEFGHDAT_REG				(0x028 + GPIO_BASE) /* Port EFGH data */
#define PEFGHISR_REG				(0x02C + GPIO_BASE) /* Port EFGH interrupt status */
#define PEFIMR_REG				    (0x030 + GPIO_BASE) /* Port EF interrupt mask */
#define PGHIMR_REG				    (0x034 + GPIO_BASE) /* Port GH interrupt mask */

#define MDIO_TIMEOUT 2000000
#define DEFAULT_MDIO_PHYAD 16  /* selected by the hardware strapping pin of external Host CPU, @Pana_TBD */

#define REG32(reg) 			(*((volatile unsigned int *)(reg)))

#ifdef MDCIO_GPIO_SIMULATION
//RTL8651C - F0: MDC, F1: MDIO
//RTL8196  - E5: MDC, E6: MDIO
//RTL8954C V200 EVB - G1: MDC, G0: MDIO, E2: RESET
//RTL8954C V400 EVB - G1: MDC, F2: MDIO, E2: RESET
//#define RTL8651C_FAMILY
#define RTL89xxC_FAMILY
#endif

#ifdef RTL8651C_FAMILY
// RTL8651C GPIO B6 is connected with RESETn pin.
#define Set_NFBI_RESET_L()	(REG32(PABCDDAT_REG) =  REG32(PABCDDAT_REG)  & (~0x4000) )
#define Set_NFBI_RESET_H()	(REG32(PABCDDAT_REG) =  REG32(PABCDDAT_REG)  | 0x4000)
#elif defined (RTL89xxC_FAMILY)
// RTL89xxC GPIO E2 is connected with RESETn pin.
#define Set_NFBI_RESET_L()	(REG32(PEFGHDAT_REG) =  REG32(PEFGHDAT_REG)  & (~0x4) )
#define Set_NFBI_RESET_H()	(REG32(PEFGHDAT_REG) =  REG32(PEFGHDAT_REG)  | 0x4)

#endif
/*================================================================*/
#else
/*================================================================*/
#include "avev3.h"
extern struct net_device *avev3_dev;

void Set_NFBI_RESET_L(void)  /*Reset Start(Assert)*/
{
  unsigned long    reg;

  AVEV3_REG_READ(reg, _AVEV3_GRR);
  AVEV3_REG_WRITE( (reg | _AVEV3_GRR_PHYRST), _AVEV3_GRR);

  return;
}

void Set_NFBI_RESET_H(void)  /*Reset End(Negate)*/
{
  unsigned long    reg;

  AVEV3_REG_READ(reg, _AVEV3_GRR);
  AVEV3_REG_WRITE( (reg & ~_AVEV3_GRR_PHYRST), _AVEV3_GRR);

  return;
}
/*================================================================*/
#endif /*HOST_IS_PANABOARD*/

/*================================================================*/
/* Local Variables */

static int io=0xb8019000; //base address of 97B CPU internal register for NFBI
static int irq = 18;
static unsigned char mdio_phyaddr; 
static spinlock_t mdio_lock = SPIN_LOCK_UNLOCKED;

#ifdef SIMULATION
static unsigned char data_in[256];
static int data_in_len = 0, data_in_read_idx = 0;
static int msg_is_coming=0;

static unsigned short data_out;
static int data_out_len = 0, msg_is_fetched = 0;

struct semaphore    sim_sem;
wait_queue_head_t   sim_wq, sim_rq;
#endif

static struct nfbi_dev_priv *dev_priv=NULL;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,7)
/* copied from linux kernel 2.6.20 /include/linux/time.h */
/* Parameters used to convert the timespec values: */
#define MSEC_PER_SEC	1000L

/* copied from linux kernel 2.6.20 /include/linux/jiffies.h */
/*
 * Change timeval to jiffies, trying to avoid the
 * most obvious overflows..
 *
 * And some not so obvious.
 *
 * Note that we don't want to return MAX_LONG, because
 * for various timeout reasons we often end up having
 * to wait "jiffies+1" in order to guarantee that we wait
 * at _least_ "jiffies" - so "jiffies+1" had better still
 * be positive.
 */
#define MAX_JIFFY_OFFSET ((~0UL >> 1)-1)

/*
 * Convert jiffies to milliseconds and back.
 *
 * Avoid unnecessary multiplications/divisions in the
 * two most common HZ cases:
 */
static inline unsigned int _kc_jiffies_to_msecs(const unsigned long j)
{
#if HZ <= MSEC_PER_SEC && !(MSEC_PER_SEC % HZ)
	return (MSEC_PER_SEC / HZ) * j;
#elif HZ > MSEC_PER_SEC && !(HZ % MSEC_PER_SEC)
	return (j + (HZ / MSEC_PER_SEC) - 1)/(HZ / MSEC_PER_SEC);
#else
	return (j * MSEC_PER_SEC) / HZ;
#endif
}

static inline unsigned long _kc_msecs_to_jiffies(const unsigned int m)
{
	if (m > _kc_jiffies_to_msecs(MAX_JIFFY_OFFSET))
		return MAX_JIFFY_OFFSET;
#if HZ <= MSEC_PER_SEC && !(MSEC_PER_SEC % HZ)
	return (m + (MSEC_PER_SEC / HZ) - 1) / (MSEC_PER_SEC / HZ);
#elif HZ > MSEC_PER_SEC && !(HZ % MSEC_PER_SEC)
	return m * (HZ / MSEC_PER_SEC);
#else
	return (m * HZ + MSEC_PER_SEC - 1) / MSEC_PER_SEC;
#endif
}
#endif	//LINUX_VERSION_CODE < KERNEL_VERSION(2,6,7)

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9)
/**
 * msleep_interruptible - sleep waiting for waitqueue interruptions
 * @msecs: Time in milliseconds to sleep for
 */
#define msleep_interruptible _kc_msleep_interruptible
unsigned long _kc_msleep_interruptible(unsigned int msecs)
{
	unsigned long timeout = _kc_msecs_to_jiffies(msecs);

	while (timeout && !signal_pending(current)) {
		set_current_state(TASK_INTERRUPTIBLE);
		timeout = schedule_timeout(timeout);
	}
	return _kc_jiffies_to_msecs(timeout);
}
#endif	//LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9)


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


#ifdef SIMULATION
static unsigned short register_read_dw(unsigned short offset)
{
	unsigned short status = 0;
	unsigned short wdata = 0;

	if (offset == NFBI_REG_ISR) {
	    //down_interruptible(&sim_sem);
		//if (data_in_len > data_in_read_idx) {
		if (msg_is_coming) {
			status |= IP_NEWMSG_COMING;
			msg_is_coming = 0;
		}

		if (msg_is_fetched) {
			status |= IP_PREVMSG_FETCH;
			msg_is_fetched = 0;
		}
        //up(&sim_sem);
		return status;
	}	
	else if (offset == NFBI_REG_RSR) {
	    down_interruptible(&sim_sem);
		ASSERT(data_in_len > data_in_read_idx);
		memcpy(&wdata, &data_in[data_in_read_idx], 2);
		data_in_read_idx += 2;
		up(&sim_sem);
		wake_up_interruptible(&sim_wq);
		status |= wdata;
		return status;
	}
	else {
		ASSERT(0);
		return status;
	}	
}

static void register_write_dw(unsigned short offset, unsigned short data)
{
	if (offset == NFBI_REG_SCR) {
		unsigned short wData = data;
		
		down_interruptible(&sim_sem);
		
		memcpy(&data_out, &wData, 2);	
		data_out_len = 2;
		
		up(&sim_sem);
	}

}
#endif // SIMULATION

/*----------------------------------------------------------------------------*/
#ifdef MDCIO_GPIO_SIMULATION

#define DELAY		50

/*
 * MDC/MDIO API: use two GPIO pin to simulate MDC/MDIO signal
 *
 */
void _smiGpioInit(void)
{
	printk("Init GPIO for MDC/MDIO.\n");
#ifdef RTL8651C_FAMILY //F0: MDC, F1: MDIO
    //config as GPIO pin
    REG32(PEFGHCNR_REG) = REG32(PEFGHCNR_REG) & (~0x00000300);
	printk("0xB800351C=%X\n", REG32(PEFGHCNR_REG));
	//Disable F0, F0 interrupt
	REG32(PEFIMR_REG) = REG32(PEFIMR_REG) & (~0x000F0000);
	printk("0xB8003530=%X\n", REG32(PEFIMR_REG));
	//set F0, F1 output pin
	REG32(PEFGHDIR_REG) = REG32(PEFGHDIR_REG) | 0x00000300;
	printk("0xB8003524=%X\n", REG32(PEFGHDIR_REG));
#elif defined (RTL89xxC_FAMILY) 
	#ifdef CONFIG_RTK_VOIP_GPIO_8954C_V400	//G1: MDC, F2: MDIO, E2: RESET
	// PIN Mux select (register PIN_MUX_SEL bit 14, 8, 9)
	*((volatile unsigned int *)0xB8000040) = 
		*((volatile unsigned int *)0xB8000040) | 0x4300;
	//config as GPIO pin
	REG32(PEFGHCNR_REG) = REG32(PEFGHCNR_REG) & (~0x00020400);
	printk("PEFGHCNR_REG=%X\n", REG32(PEFGHCNR_REG));
	//Disable G1 interrupt
	REG32(PGHIMR_REG) = REG32(PGHIMR_REG) & (~0x0000000C);
	printk("PGHIMR_REG=%X\n", REG32(PGHIMR_REG));
	//Disable F2 interrupt
	REG32(PEFIMR_REG) = REG32(PEFIMR_REG) & (~0x00300000);
	printk("PEFIMR_REG=%X\n", REG32(PEFIMR_REG));
	//set G0, F2 output pin
	REG32(PEFGHDIR_REG) = REG32(PEFGHDIR_REG) | 0x00020400;
	printk("PEFGHDIR_REG=%X\n", REG32(PEFGHDIR_REG));
	#else 									//G1: MDC, G0: MDIO, E2: RESET
	// PIN Mux select (register PIN_MUX_SEL bit 14, 8, 9)
	*((volatile unsigned int *)0xB8000040) = 
		*((volatile unsigned int *)0xB8000040) | 0x4000 | 0x0300;
	//config as GPIO pin
	REG32(PEFGHCNR_REG) = REG32(PEFGHCNR_REG) & (~0x00030000);
	printk("PEFGHCNR_REG=%X\n", REG32(PEFGHCNR_REG));
	//Disable G0, G1 interrupt
	REG32(PGHIMR_REG) = REG32(PGHIMR_REG) & (~0x0000000F);
	printk("PGHIMR_REG=%X\n", REG32(PGHIMR_REG));
	//set G0, G1 output pin
	REG32(PEFGHDIR_REG) = REG32(PEFGHDIR_REG) | 0x00030000;
	printk("PEFGHDIR_REG=%X\n", REG32(PEFGHDIR_REG));
	#endif
#else
#error "Need implement for _smiGpioInit"
#endif
}
/*
static void _smiGenReadClk(void) 
{
	unsigned short i;
#ifdef RTL8651C_FAMILY
	REG32(PEFGHDIR_REG) = (REG32(PEFGHDIR_REG)& 0xFFFFFCFF) | 0x00000100;	// MDC=OUT, MDIO=IN
	REG32(PEFGHDAT_REG) = (REG32(PEFGHDAT_REG) & 0xFFFFFCFF) | 0x00000100;	// MDC=1, MDIO=0
	for(i=0; i< DELAY; i++);
	REG32(PEFGHDAT_REG) = (REG32(PEFGHDAT_REG) & 0xFFFFFCFF);	// MDC=0
#else
#error "Need implement for _smiGenReadClk"
#endif
}
*/
static void _smiGenWriteClk(void) 
{
	unsigned short i;
#ifdef RTL8651C_FAMILY //F0: MDC, F1: MDIO
	for(i=0; i< DELAY; i++);
	REG32(PEFGHDAT_REG) = REG32(PEFGHDAT_REG) & 0xFFFFFCFF;// MDC=0, MDIO=0
	for(i=0; i< DELAY; i++);
	REG32(PEFGHDAT_REG) = REG32(PEFGHDAT_REG) | 0x00000100;	// MDC=1
#elif defined (RTL89xxC_FAMILY) 
	#ifdef CONFIG_RTK_VOIP_GPIO_8954C_V400	//G1: MDC, F2: MDIO
	for(i=0; i< DELAY; i++);
	REG32(PEFGHDAT_REG) = REG32(PEFGHDAT_REG) & 0xFFFDFBFF;// MDC=0, MDIO=0
	for(i=0; i< DELAY; i++);
	REG32(PEFGHDAT_REG) = REG32(PEFGHDAT_REG) | 0x00020000;	// MDC=1
	#else									//G1: MDC, G0: MDIO
	for(i=0; i< DELAY; i++);
	REG32(PEFGHDAT_REG) = REG32(PEFGHDAT_REG) & 0xFFFCFFFF;// MDC=0, MDIO=0
	for(i=0; i< DELAY; i++);
	REG32(PEFGHDAT_REG) = REG32(PEFGHDAT_REG) | 0x00020000;	// MDC=1
	#endif
#else
#error "Need implement for _smiGenWriteClk"
#endif
}

/* Change clock to 1 */
static void _smiZBit(void) {
	unsigned short i;
#ifdef RTL8651C_FAMILY //F0: MDC, F1: MDIO
	REG32(PEFGHDIR_REG) = (REG32(PEFGHDIR_REG) & 0xFFFFFCFF) | 0x100;
	REG32(PEFGHDAT_REG) = (REG32(PEFGHDAT_REG) & 0xFFFFFCFF);
#elif defined (RTL89xxC_FAMILY) 
	#ifdef CONFIG_RTK_VOIP_GPIO_8954C_V400	//G1: MDC, F2: MDIO
	REG32(PEFGHDIR_REG) = (REG32(PEFGHDIR_REG) & 0xFFFDFBFF) | 0x20000;
	REG32(PEFGHDAT_REG) = (REG32(PEFGHDAT_REG) & 0xFFFDFBFF);
	#else									//G1: MDC, G0: MDIO
	REG32(PEFGHDIR_REG) = (REG32(PEFGHDIR_REG) & 0xFFFCFFFF) | 0x20000;
	REG32(PEFGHDAT_REG) = (REG32(PEFGHDAT_REG) & 0xFFFCFFFF);
	#endif
#else
    #ifdef GPIO_E56
	REG32(PEFGHDIR_REG) = (REG32(PEFGHDIR_REG) & 0xFFFFFF9F) | 0x20;
	REG32(PEFGHDAT_REG) = (REG32(PEFGHDAT_REG) & 0xFFFFFF9F);
    #elif defined GPIO_D67
    REG32(GPABCDDIR) = (REG32(GPABCDDIR) & 0x3FFFFFFF) | 0x40000000;// MDIO=IN, MDC=OUT
	REG32(GPABCDDATA) = (REG32(GPABCDDATA) & 0x3FFFFFFF);	// MDIO=0, MDC=0
    #endif
#endif
	for(i=0; i< DELAY; i++);
}

/* Generate  1 -> 0 transition and sampled at 1 to 0 transition time,
should not sample at 0->1 transition because some chips stop outputing
at the last bit at rising edge*/

static void _smiReadBit(unsigned short * pdata) {
	unsigned short i;
#ifdef RTL8651C_FAMILY //F0: MDC, F1: MDIO
	REG32(PEFGHDIR_REG) = (REG32(PEFGHDIR_REG)& 0xFFFFFCFF) | 0x100;
	REG32(PEFGHDAT_REG) = (REG32(PEFGHDAT_REG) & 0xFFFFFCFF) | 0x100;
	for(i=0; i< DELAY; i++);
	REG32(PEFGHDAT_REG) = (REG32(PEFGHDAT_REG) & 0xFFFFFCFF);
	*pdata = (REG32(PEFGHDAT_REG) & 0x200)?1:0; 
#elif defined (RTL89xxC_FAMILY) 
	#ifdef CONFIG_RTK_VOIP_GPIO_8954C_V400	//G1: MDC, F2: MDIO
	REG32(PEFGHDIR_REG) = (REG32(PEFGHDIR_REG)& 0xFFFDFBFF) | 0x20000;
	REG32(PEFGHDAT_REG) = (REG32(PEFGHDAT_REG) & 0xFFFDFBFF) | 0x20000;
	for(i=0; i< DELAY; i++);
	REG32(PEFGHDAT_REG) = (REG32(PEFGHDAT_REG) & 0xFFFDFBFF);
	*pdata = (REG32(PEFGHDAT_REG) & 0x0400)?1:0; 	// read F2
	#else									//G1: MDC, G0: MDIO
	REG32(PEFGHDIR_REG) = (REG32(PEFGHDIR_REG)& 0xFFFCFFFF) | 0x20000;
	REG32(PEFGHDAT_REG) = (REG32(PEFGHDAT_REG) & 0xFFFCFFFF) | 0x20000;
	for(i=0; i< DELAY; i++);
	REG32(PEFGHDAT_REG) = (REG32(PEFGHDAT_REG) & 0xFFFCFFFF);
	*pdata = (REG32(PEFGHDAT_REG) & 0x10000)?1:0; 
	#endif
#else
    #ifdef GPIO_E56
	REG32(PEFGHDIR_REG) = (REG32(PEFGHDIR_REG)& 0xFFFFFF9F) | 0x20;
	REG32(PEFGHDAT_REG) = (REG32(PEFGHDAT_REG) & 0xFFFFFF9F) | 0x20;
	for(i=0; i< DELAY; i++);
	REG32(PEFGHDAT_REG) = (REG32(PEFGHDAT_REG) & 0xFFFFFF9F);
	*pdata = (REG32(PEFGHDAT_REG) & 0x40)?1:0; 
    #elif defined GPIO_D67
	REG32(GPABCDDIR) = (REG32(GPABCDDIR)& 0x3FFFFFFF) | 0x40000000;	// MDIO=IN, MDC=OUT
	REG32(GPABCDDATA) = (REG32(GPABCDDATA) & 0x3FFFFFFF) | 0x40000000; // MDC=1, MDIO=0
	for(i=0; i< DELAY; i++);
	REG32(GPABCDDATA) = (REG32(GPABCDDATA) & 0x3FFFFFFF);	// MDC=0
	*pdata = (REG32(GPABCDDATA) & 0x80000000)?1:0; // Get MDIO value
    #endif
#endif
}

/* Generate  0 -> 1 transition and put data ready during 0 to 1 whole period */
static void _smiWriteBit(unsigned short data) {
	unsigned short i;
#ifdef RTL8651C_FAMILY //F0: MDC, F1: MDIO
	REG32(PEFGHDIR_REG) = REG32(PEFGHDIR_REG) | 0x300;
	if(data) {/* Write 1 */
		REG32(PEFGHDAT_REG) = (REG32(PEFGHDAT_REG) & 0xFFFFFCFF) | 0x200;
		for(i=0; i< DELAY; i++);
		REG32(PEFGHDAT_REG) = (REG32(PEFGHDAT_REG) & 0xFFFFFCFF) | 0x300;
	} else {
		REG32(PEFGHDAT_REG) = (REG32(PEFGHDAT_REG) & 0xFFFFFCFF);
		for(i=0; i< DELAY; i++);
		REG32(PEFGHDAT_REG) = (REG32(PEFGHDAT_REG) & 0xFFFFFCFF) | 0x100;
	}
#elif defined (RTL89xxC_FAMILY) 
	#ifdef CONFIG_RTK_VOIP_GPIO_8954C_V400	//G1: MDC, F2: MDIO
	REG32(PEFGHDIR_REG) = REG32(PEFGHDIR_REG) | 0x00020400;
	if(data) {/* Write 1 */
		REG32(PEFGHDAT_REG) = (REG32(PEFGHDAT_REG) & 0xFFFDFBFF) | 0x00000400;
		for(i=0; i< DELAY; i++);
		REG32(PEFGHDAT_REG) = (REG32(PEFGHDAT_REG) & 0xFFFDFBFF) | 0x00020400;
	} else {
		REG32(PEFGHDAT_REG) = (REG32(PEFGHDAT_REG) & 0xFFFDFBFF);
		for(i=0; i< DELAY; i++);
		REG32(PEFGHDAT_REG) = (REG32(PEFGHDAT_REG) & 0xFFFDFBFF) | 0x00020000;
	}
	#else									//G1: MDC, G0: MDIO
	REG32(PEFGHDIR_REG) = REG32(PEFGHDIR_REG) | 0x30000;
	if(data) {/* Write 1 */
		REG32(PEFGHDAT_REG) = (REG32(PEFGHDAT_REG) & 0xFFFCFFFF) | 0x10000;
		for(i=0; i< DELAY; i++);
		REG32(PEFGHDAT_REG) = (REG32(PEFGHDAT_REG) & 0xFFFCFFFF) | 0x30000;
	} else {
		REG32(PEFGHDAT_REG) = (REG32(PEFGHDAT_REG) & 0xFFFCFFFF);
		for(i=0; i< DELAY; i++);
		REG32(PEFGHDAT_REG) = (REG32(PEFGHDAT_REG) & 0xFFFCFFFF) | 0x20000;
	}
	#endif
#else
    #ifdef GPIO_E56
	REG32(PEFGHDIR_REG) = REG32(PEFGHDIR_REG) | 0x60;
	if(data) {/* Write 1 */
		REG32(PEFGHDAT_REG) = (REG32(PEFGHDAT_REG) & 0xFFFFFCFF) | 0x40;
		for(i=0; i< DELAY; i++);
		REG32(PEFGHDAT_REG) = (REG32(PEFGHDAT_REG) & 0xFFFFFCFF) | 0x60;
	} else {
		REG32(PEFGHDAT_REG) = (REG32(PEFGHDAT_REG) & 0xFFFFFCFF);
		for(i=0; i< DELAY; i++);
		REG32(PEFGHDAT_REG) = (REG32(PEFGHDAT_REG) & 0xFFFFFCFF) | 0x20;
	}
    #elif defined GPIO_D67
    	REG32(GPABCDDIR) = REG32(GPABCDDIR) | 0xC0000000; // set MDIO, MDC to output
	if(data) {/* Write 1 */
		REG32(GPABCDDATA) = (REG32(GPABCDDATA) & 0x3FFFFFFF) | 0x80000000; // MDC=0, MDIO=1
		for(i=0; i< DELAY; i++);
		REG32(GPABCDDATA) = (REG32(GPABCDDATA) & 0xBFFFFFFF) | 0x40000000; // MDC=1
	} else {
		REG32(GPABCDDATA) = (REG32(GPABCDDATA) & 0x3FFFFFFF);		// MDC=0, MDIO=0
		for(i=0; i< DELAY; i++);
		REG32(GPABCDDATA) = (REG32(GPABCDDATA) & 0xBFFFFFFF) | 0x40000000;// MDC=1
	}
    #endif
#endif
}

int smiRead(unsigned char phyad, unsigned short regad, unsigned short * data) {
	int i;
	unsigned short readBit;

	/* Configure port C pin 1, 0 to be GPIO and disable interrupts of these two pins */
#ifdef RTL8651C_FAMILY //F0: MDC, F1: MDIO
//	REG32(GPEFGHCNR) = REG32(GPEFGHCNR) & 0xFFFFFCFF;
//	REG32(GPEFIMR) = REG32(GPEFIMR) & 0xFFFFFFF;
#elif defined (RTL89xxC_FAMILY) 
	#ifdef CONFIG_RTK_VOIP_GPIO_8954C_V400	//G1: MDC, F2: MDIO
	#else 									//G1: MDC, G0: MDIO
	#endif
#else
    #ifdef GPIO_E56
	REG32(GPEFGHCNR) = REG32(GPEFGHCNR) & 0xFFFFFF9F;
	REG32(GPEFIMR) = REG32(GPEFIMR) & 0xFFFFFFF;
    #elif defined GPIO_D67
    	//REG32(GPABCDCNR) = REG32(GPABCDCNR) & 0xFFFFFF9F;
	//REG32(GPCDIMR) = REG32(GPCDIMR) & 0xFFFFFFF;
    #endif
#endif
	/* 32 continuous 1 as preamble*/
	for(i=0; i<32; i++)
		_smiWriteBit(1);
	/* ST: Start of Frame, <01>*/
	_smiWriteBit(0);
	_smiWriteBit(1);
	/* OP: Operation code, read is <10> */
	_smiWriteBit(1);
	_smiWriteBit(0);
	/* PHY Address */
	for(i=4; i>=0; i--) 
		_smiWriteBit((phyad>>i)&0x1);
	/* Register Address */
	for(i=4; i>=0; i--) 
		_smiWriteBit((regad>>i)&0x1);
	/* TA: Turnaround <z0> */
	_smiZBit();
	_smiReadBit(&readBit);
	/* Data */
	*data = 0;
	for(i=15; i>=0; i--) {
		_smiReadBit(&readBit);
		*data = (*data<<1) | readBit;
	}
        /*add  an extra clock cycles for robust reading , ensure partner stop output signal
        and not affect the next read operation, because TA steal a clock*/
	_smiWriteBit(1);
	_smiZBit();
    
	return 0;
}

int smiWrite(unsigned char phyad, unsigned short regad, unsigned short data) {
	int i;

	/* Configure port C pin 1, 0 to be GPIO and disable interrupts of these two pins */
#ifdef RTL8651C_FAMILY
//	REG32(GPEFGHCNR) = REG32(GPEFGHCNR) & 0xFFFFFCFF;
//	REG32(GPEFIMR) = REG32(GPEFIMR) & 0xFFFFFFF;
#elif defined (RTL89xxC_FAMILY) 
	#ifdef CONFIG_RTK_VOIP_GPIO_8954C_V400	//G1: MDC, F2: MDIO
	#else 									//G1: MDC, G0: MDIO
	#endif
#else
    #ifdef GPIO_E56
	REG32(GPEFGHCNR) = REG32(GPEFGHCNR) & 0xFFFFFF9F;
	REG32(GPEFIMR) = REG32(GPEFIMR) & 0xFFFFFFF;
    #elif defined GPIO_D67
    	//REG32(GPABCDCNR) = REG32(GPABCDCNR) & 0xFFFFFF9F;
	//REG32(GPCDIMR) = REG32(GPCDIMR) & 0xFFFFFFF;
    #endif
#endif
	/* 32 continuous 1 as preamble*/
	for(i=0; i<32; i++)
		_smiWriteBit(1);
	/* ST: Start of Frame, <01>*/
	_smiWriteBit(0);
	_smiWriteBit(1);
	/* OP: Operation code, write is <01> */
	_smiWriteBit(0);
	_smiWriteBit(1);
	/* PHY Address */
	for(i=4; i>=0; i--) 
		_smiWriteBit((phyad>>i)&0x1);
	/* Register Address */
	for(i=4; i>=0; i--) 
		_smiWriteBit((regad>>i)&0x1);
	/* TA: Turnaround <10> */
	_smiWriteBit(1);
	_smiWriteBit(0);
	/* Data */
	for(i=15; i>=0; i--) 
		_smiWriteBit((data>>i)&0x1);
	_smiGenWriteClk();
	_smiZBit();

	return 0;
}
#endif //MDCIO_GPIO_SIMULATION
/*----------------------------------------------------------------------------*/


//@Pana_TBD
#ifndef HOST_IS_PANABOARD
int rtl_mdio_write(unsigned short reg, unsigned short data)
{
    unsigned long flags;
#ifdef MDCIO_GPIO_SIMULATION
    int ret;
    
    del_timer(&dev_priv->mdc_timer);
    spin_lock_irqsave(&mdio_lock, flags);
    ret = smiWrite(mdio_phyaddr, reg, data);
    spin_unlock_irqrestore(&mdio_lock, flags);
    mod_timer(&dev_priv->mdc_timer, jiffies + 1);
    return ret;
#else
	volatile unsigned int val;
	int timeout = MDIO_TIMEOUT;

    spin_lock_irqsave(&mdio_lock, flags);
    REG32(MDCIOCR) = 0x80000000 | (mdio_phyaddr<<24) | ((reg&0x1f)<<16) | (data&0xffff);
	while (--timeout) {
	    val = REG32(MDCIOSR);
	    if ((val&0x80000000) == 0)
	        break;
	}
	spin_unlock_irqrestore(&mdio_lock, flags);
    
	if (timeout==0) {
		PDEBUG("MDIO Timeout! write data:%x to reg: %x Error!\n",data,reg);
        return -1;
	}
	else
	   return 0;
#endif
}

int rtl_mdio_mask_write(unsigned short reg, unsigned short mask, unsigned short data)
{
    unsigned long flags;
#ifdef MDCIO_GPIO_SIMULATION
    unsigned short regval;
    int ret;

    del_timer(&dev_priv->mdc_timer);
    spin_lock_irqsave(&mdio_lock, flags);
    ret = smiRead(mdio_phyaddr, reg, &regval);
    if (ret == 0) {
    	data = (regval&(~mask)) | (data&mask);
        ret = smiWrite(mdio_phyaddr, reg, data);
    }
    spin_unlock_irqrestore(&mdio_lock, flags);
    mod_timer(&dev_priv->mdc_timer, jiffies + 1);
    return ret;
#else
	volatile unsigned int val;
	unsigned short regval;
	int timeout = MDIO_TIMEOUT;

    spin_lock_irqsave(&mdio_lock, flags);
    REG32(MDCIOCR) = 0x7fffffff & ((mdio_phyaddr<<24) | ((reg&0x1f)<<16));
	while (--timeout) {
	    val = REG32(MDCIOSR);
	    if ((val&0x80000000) == 0)
	        break;
	}
    
	if (timeout==0) {
		PDEBUG("MDIO Timeout! read2 reg: %x Error!\r\n",reg);
		spin_unlock_irqrestore(&mdio_lock, flags);
        return -1;
	}
    regval = (unsigned short)(val & 0xffff);
	data = (regval&(~mask)) | (data&mask);
    REG32(MDCIOCR) = 0x80000000 | (mdio_phyaddr<<24) | ((reg&0x1f)<<16) | (data&0xffff);
	while (--timeout) {
	    val = REG32(MDCIOSR);
	    if ((val&0x80000000) == 0)
	        break;
	}
	spin_unlock_irqrestore(&mdio_lock, flags);
    
	if (timeout==0) {
		PDEBUG("MDIO Timeout! write2 data:%x to reg: %x Error!\n",data,reg);
        return -1;
	}
	else
	   return 0;
#endif
}

int rtl_mdio_read(unsigned short reg, unsigned short *pdata)
{
    unsigned long flags;
#ifdef MDCIO_GPIO_SIMULATION
    int ret;
    
    del_timer(&dev_priv->mdc_timer);
    spin_lock_irqsave(&mdio_lock, flags);
    ret = smiRead(mdio_phyaddr, reg, pdata);
    spin_unlock_irqrestore(&mdio_lock, flags);
    mod_timer(&dev_priv->mdc_timer, jiffies + 1);
    return ret;
#else
	volatile unsigned int val;
	int timeout = MDIO_TIMEOUT;

    spin_lock_irqsave(&mdio_lock, flags);
    REG32(MDCIOCR) = 0x7fffffff & ((mdio_phyaddr<<24) | ((reg&0x1f)<<16));
	while (--timeout) {
	    val = REG32(MDCIOSR);
	    if ((val&0x80000000) == 0)
	        break;
	}
    spin_unlock_irqrestore(&mdio_lock, flags);
    
	if (timeout==0) {
		PDEBUG("MDIO Timeout! read reg: %x Error!\r\n",reg);
        *pdata = 0;
        return -1;
	}
	else {
	   *pdata = val & 0xffff;
	   return 0;
    }
#endif
}
#else
int rtl_mdio_write(unsigned short reg, unsigned short data)
{
  avev3_mdio_write(avev3_dev, mdio_phyaddr, reg, data);
  return 0;
}

int rtl_mdio_mask_write(unsigned short reg, unsigned short mask, unsigned short data)
{
  unsigned short regval;

  regval = avev3_mdio_read(avev3_dev, mdio_phyaddr, reg);
  data = (regval&(~mask)) | (data&mask);
  avev3_mdio_write(avev3_dev, mdio_phyaddr, reg, data);
  return 0;
}

int rtl_mdio_read(unsigned short reg, unsigned short *pdata)
{
  *pdata = avev3_mdio_read(avev3_dev, mdio_phyaddr, reg);
  return 0;
}
#endif /*HOST_IS_PANABOARD*/

static void process_rx_msg(struct nfbi_priv *priv, unsigned short data)
{
    /*
	if (priv->rx_status_time && 
			(priv->state == STATE_RX_WAIT_LEN ||
				priv->state == STATE_RX_WAIT_DATA) &&
			TIME_DIFF(jiffies, priv->rx_status_time) > CMD_TIME_OUT) {
		PDEBUG("Rx status timeout [%ld], discard pending status!\n", TIME_DIFF(jiffies, priv->rx_status_time));
		RESET_RX_STATE;
	}
    */
	if ((data & FIRST_CMD_MASK) && (priv->state != STATE_RX_INIT)) {
		PDEBUG("Rx Sync bit but not in INIT_STATE, discard pending status!\n");
		dev_priv->rx_reset_by_sync_bit_errors++;
		RESET_RX_STATE;
	}
	
	if (priv->state == STATE_RX_INIT) {
		ASSERT(priv->data_in.len == 0);
		
		if (!(data & FIRST_CMD_MASK)) {
			PDEBUG("Not first word [0x%x], discard it!\n", data);
			dev_priv->rx_not_1st_word_errors++;
			goto invalid_cmd;
		}		
		PUT_IN_DATA(data);	// cmd id
		priv->state = STATE_RX_WAIT_LEN;		
		priv->rx_status_time = jiffies;
	}	
	else  { // STATE_RX_WAIT_LEN or STATE_RX_WAIT_DATA
		// check if first cmd byte is '0'	
		if (data & 0xff00) {
			PDEBUG("1st byte of rx word not zero [%x]!\n", data >> 8);
			dev_priv->rx_1st_byte_errors++;
			goto invalid_cmd;			
		}
		
		if (priv->state == STATE_RX_WAIT_LEN) {
			PUT_IN_DATA(data);
			priv->state = STATE_RX_WAIT_DATA;
			priv->rx_status_remain_len = (data + 1)*2;	// including checksum
			priv->rx_status_time = jiffies;
		}
		else { // in STATE_RX_WAIT_DATA
			ASSERT (priv->rx_status_remain_len > 0);

			PUT_IN_DATA(data);				
			priv->rx_status_remain_len -= 2;		
			if (priv->rx_status_remain_len  <= 0) { // rx last byte, calcuate checksum
				if (!is_checksum_ok(priv->data_in.buf, priv->data_in.len)) {
					PDEBUG("Rx frame cheksum error!\n");
					dev_priv->rx_checksum_errors++;
					goto invalid_cmd;
				}
				//chech if the command field of rx status frame is the same as tx command frame
				if (priv->data_in.buf[1] != priv->data_out.buf[1]) {
                    PDEBUG("Rx cmd field not match!\n");
                    dev_priv->rx_cmdid_not_match_errors++;
					goto invalid_cmd;
				}
				priv->data_in.len -= 2; // substract checksum length
				priv->state = STATE_RX_FINISHED;
			}
			else
				priv->rx_status_time = jiffies;
		}
	}
	
	return;
	
invalid_cmd:
    RESET_RX_STATE;
}

static void transmit_msg(struct nfbi_priv *priv)
{
	unsigned short data;

	if (priv->data_out.len <= 0 || priv->tx_cmd_transmitting_len >= priv->data_out.len) 
		return;
#ifndef HOST_IS_PANABOARD
	memcpy(&data, priv->data_out.buf+priv->tx_cmd_transmitting_len, 2);
#else
	data = ( *(priv->data_out.buf+priv->tx_cmd_transmitting_len) << 8 ) |
			 *(priv->data_out.buf+priv->tx_cmd_transmitting_len + 1);
#endif
	dev_priv->tx_msg_is_fetched = 0;
#ifdef SIMULATION
	register_write_dw(NFBI_REG_SCR, data);
#else
    rtl_mdio_write(NFBI_REG_SCR, data);
#endif
    dev_priv->tx_words++;
	priv->tx_cmd_transmitting_len += 2;
}
 
static int indicate_evt(struct evt_msg *evt)
{
	int size;

	if (dev_priv->hcd_pid == -1)
	    return 0;
	    
	size = CIRC_SPACE(dev_priv->evt_que_head, dev_priv->evt_que_tail, EV_QUE_MAX);
	if (size == 0) {
		PDEBUG("Indication queue full, drop event!\n");
        //send SIGUSR1 signal to notify the host control deamon process
		//if (dev_priv->hcd_pid != -1)
	       //kill_proc(dev_priv->hcd_pid, SIGUSR1, 1);

		return 0;
	}
	
	dev_priv->ind_evt_que[dev_priv->evt_que_head].id = evt->id;
	dev_priv->ind_evt_que[dev_priv->evt_que_head].status = evt->status;
	dev_priv->ind_evt_que[dev_priv->evt_que_head].value = evt->value;
	dev_priv->evt_que_head = (dev_priv->evt_que_head + 1) & (EV_QUE_MAX - 1);

    //send SIGUSR1 signal to notify the host control deamon process
	//if (dev_priv->hcd_pid != -1)
		//kill_proc(dev_priv->hcd_pid, SIGUSR1, 1);
	
	return 1;
}

//static int retrieve_evt(struct nfbi_priv *priv, struct evt_msg *evt)
static int retrieve_evt(struct evt_msg *evt)
{
	if (CIRC_CNT(dev_priv->evt_que_head, dev_priv->evt_que_tail, EV_QUE_MAX) > 0) { // more than one evt pending
	    evt->id = dev_priv->ind_evt_que[dev_priv->evt_que_tail].id;
	    evt->status = dev_priv->ind_evt_que[dev_priv->evt_que_tail].status;
	    evt->value = dev_priv->ind_evt_que[dev_priv->evt_que_tail].value;
		dev_priv->evt_que_tail = (dev_priv->evt_que_tail + 1) & (EV_QUE_MAX - 1);
	}
	else {
        evt->id = 0;
	    evt->status = 0;
	    evt->value = 0;
	}
	return 0;
}

static void nfbi_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
    unsigned short status, data, mask;
    struct evt_msg evt;
	//struct nfbi_dev_priv *priv = (struct nfbi_dev_priv *)dev_id;

#ifdef HOST_IS_PANABOARD
    if( mdio_phyaddr == 0xff ){
      /* Can't found 8197B */
      return;/* Do noting */
    }
#endif /*HOST_IS_PANABOARD*/

    if (!dev_priv->ready) {
        //==========================================================================
	    // @Pana_TBD
#ifndef HOST_IS_PANABOARD
	    REG32(PABCDISR_REG) = REG32(PABCDISR_REG) | 0x8000; //write 1 to clear the interrupt at B7  (bit 15)
#endif
	    //==========================================================================
	    return;
    }
        
    //printk("nfbi_interrupt: irq=%d\n", irq);
#ifdef SIMULATION
	status = register_read_dw(NFBI_REG_ISR);
#else
    rtl_mdio_read(NFBI_REG_ISR, &status);
    rtl_mdio_read(NFBI_REG_IMR, &mask);
#endif

    if (!(status&mask)) {
        //pana suggest us to print this message for debugging
		printk("Unknown interrupt: status=0x%04x mask=0x%04x\n", status, mask);
	}
    status &= mask;
    
	while (status) {
	    //printk("status=%x mask=%x\n", status, mask);
		// clear interrupt
#ifdef SIMULATION
		register_write_dw(NFBI_REG_ISR, status);
#else
        rtl_mdio_write(NFBI_REG_ISR, status);
#endif

        if (dev_priv->cmd_handshake_polling) {
    		if (status & (IP_PREVMSG_FETCH|IP_NEWMSG_COMING|
    		              IP_CHECKSUM_DONE|IP_CHECKSUM_OK|IP_WLANLINK|IP_ETHLINK|
    		              IP_ETHPHY_STATUS_CHANGE|IP_ALLSOFTWARE_READY|
    		              IP_USBInsertStatus|IP_USBRemoveStatus|
    		              IP_BOOTCODE_READY|IP_NEED_BOOTCODE)) {
    		    evt.id = 1;
    		    evt.status = status & (IP_PREVMSG_FETCH|IP_NEWMSG_COMING|
    		                        IP_CHECKSUM_DONE|IP_CHECKSUM_OK|IP_WLANLINK|IP_ETHLINK|
    		                        IP_ETHPHY_STATUS_CHANGE|IP_ALLSOFTWARE_READY|
    		                        IP_USBInsertStatus|IP_USBRemoveStatus|
    		                        IP_BOOTCODE_READY|IP_NEED_BOOTCODE);
    		    //if (status & IP_NEED_BOOTCODE)
    		    //    printk("IP_NEED_BOOTCODE\n");
    		    rtl_mdio_read(NFBI_REG_SYSSR, &data);
    		    evt.value = data;
    		    indicate_evt(&evt);
    		}
        }
        else {
    		if (status & IP_PREVMSG_FETCH) {
    		    dev_priv->tx_msg_is_fetched = 1;
    			wake_up_interruptible(&dev_priv->wq);
    		}
    		if (status & IP_NEWMSG_COMING) {
    		    dev_priv->rx_msg_is_coming = 1;
    			wake_up_interruptible(&dev_priv->wq);
    		}

    		if (status & (IP_CHECKSUM_DONE|IP_CHECKSUM_OK|IP_WLANLINK|IP_ETHLINK|
    		              IP_ETHPHY_STATUS_CHANGE|IP_ALLSOFTWARE_READY|
    		              IP_USBInsertStatus|IP_USBRemoveStatus|
    		              IP_BOOTCODE_READY|IP_NEED_BOOTCODE)) {
    		    evt.id = 1;
    		    evt.status = status & (IP_CHECKSUM_DONE|IP_CHECKSUM_OK|IP_WLANLINK|IP_ETHLINK|
    		                        IP_ETHPHY_STATUS_CHANGE|IP_ALLSOFTWARE_READY|
    		                        IP_USBInsertStatus|IP_USBRemoveStatus|
    		                        IP_BOOTCODE_READY|IP_NEED_BOOTCODE);
    		    //if (status & IP_NEED_BOOTCODE)
    		    //    printk("IP_NEED_BOOTCODE\n");
    		    rtl_mdio_read(NFBI_REG_SYSSR, &data);
    		    evt.value = data;
    		    indicate_evt(&evt);
    		}
    	}
		
		if (status  & ~(IP_PREVMSG_FETCH|IP_NEWMSG_COMING|
		                IP_CHECKSUM_DONE|IP_CHECKSUM_OK|IP_WLANLINK|IP_ETHLINK|
		                IP_ETHPHY_STATUS_CHANGE|IP_ALLSOFTWARE_READY|
		                IP_USBInsertStatus|IP_USBRemoveStatus|
		                IP_BOOTCODE_READY|IP_NEED_BOOTCODE)) {
			PDEBUG("Got satus=0x%x, not supported yet!\n", (unsigned int)status);
		}
        
#ifdef SIMULATION
		status = register_read_dw(NFBI_REG_ISR);
#else
        rtl_mdio_read(NFBI_REG_ISR, &status);
        status &= mask;
#endif
	}
	//==========================================================================
	// @Pana_TBD
#ifndef HOST_IS_PANABOARD
	REG32(PABCDISR_REG) = REG32(PABCDISR_REG) | 0x8000; //write 1 to clear the interrupt at B7  (bit 15)
#endif
	//==========================================================================
}

#ifdef HOST_IS_PANABOARD
/* Call from AVE network driver interrupt handler */
//==========================================================================
void nfbi_interrupt_dummy_avev3(void){
  nfbi_interrupt(0, (void *)dev_priv, (struct pt_regs *)NULL);
}
//==========================================================================
#endif

static int nfbi_open(struct inode *inode, struct file *filp)
{
    struct nfbi_priv *priv;

	//printk("%s: major=%d, minor=%d\n", __FUNCTION__, MAJOR(inode->i_rdev), MINOR(inode->i_rdev));
    //printk("filp=%p\n", filp);
	priv = (struct nfbi_priv *)kmalloc(sizeof(struct nfbi_priv), GFP_KERNEL);
	if(!priv) {
	    printk(KERN_ERR DRIVER_NAME": unable to kmalloc for nfbi_priv\n");
		return -ENOMEM;
	}
	memset((void *)priv, 0, sizeof (struct nfbi_priv));
	priv->state = STATE_TX_INIT;
    priv->retransmit_count = NFBI_RETRANSMIT_COUNT_DEFAULT;
    priv->response_timeout = NFBI_RESPONSE_TIMEOUT_DEFAULT;
    
	filp->private_data = priv;

//#ifdef __LINUX_2_6__
#if 1
	if(!try_module_get(nfbi_owner)) {
		printk(KERN_ALERT "Module increasing error!!\n");
		return -ENODEV;
	}
#else
	MOD_INC_USE_COUNT;
#endif

	return 0;;
}


static int nfbi_release(struct inode *inode, struct file *filp)
{
	//printk("%s: major=%d, minor=%d\n", __FUNCTION__,  MAJOR(inode->i_rdev), MINOR(inode->i_rdev));	

    kfree((struct nfbi_priv *)filp->private_data);
        
//#ifdef __LINUX_2_6__
#if 1
    module_put(nfbi_owner);
#else
	MOD_DEC_USE_COUNT;
#endif

	return 0;
}

static void nfbi_timer_fn(unsigned long arg)
{
    wake_up_interruptible(&dev_priv->wq);
    dev_priv->timer_expired = 1;
}

#ifdef MDCIO_GPIO_SIMULATION
static void mdc_timer_fn(unsigned long arg)
{
    _smiWriteBit(1); // generate MDC clock
    mod_timer(&dev_priv->mdc_timer, jiffies + 1);
}
#endif

static ssize_t nfbi_read (struct file *filp, char *buf, size_t count, loff_t *offset)
{
	struct nfbi_priv *priv = (struct nfbi_priv *)filp->private_data;

	if (!buf)
		return 0;

	if (priv->state != STATE_RX_FINISHED) {
		PDEBUG("To read status frame, but not in a valid state!\n");
		return 0;
	}

	if (copy_to_user((void *)buf, (void *)priv->data_in.buf, priv->data_in.len)) {
		PDEBUG("copy_to_user() error!\n");
		count = -EFAULT;
	}
	else 
		count = priv->data_in.len;

	return count;
}

static ssize_t nfbi_write (struct file *filp, const char *buf, size_t count, loff_t *offset)
{
	struct nfbi_priv *priv = (struct nfbi_priv *)filp->private_data;
	unsigned short last_word = 0;
	unsigned short data;
	//unsigned short imr;
	int retransmit_count;
	unsigned long curr_time;
	unsigned short status;

	if (!buf)
		return 0;

	if (priv->state != STATE_TX_INIT) {
		//PDEBUG("Transmit status, but not in valid state. Reset state!\n");
		priv->state = STATE_TX_INIT;
	}

	if (count > NFBI_BUFSIZE) {
		PDEBUG("Tx size too big [%d]!\n", count);
		return 0;
	}
	if (count %2) {
		PDEBUG("Invalid Tx size [%d]!\n", count);
		return 0;
	}
	
	if (copy_from_user((void *)priv->data_out.buf, (void *)buf, count)) {
		PDEBUG("copy_from_user() error!\n");		
		return -EFAULT;
	}

	last_word = (unsigned short)append_checksum(priv->data_out.buf, count);
#ifndef HOST_IS_PANABOARD
	memcpy(&priv->data_out.buf[count], &last_word, 2);
#else
	priv->data_out.buf[count] =   (char)( (last_word>>8)&0x00ff );
	priv->data_out.buf[count+1] = (char)( last_word&0x00ff );
#endif
	priv->data_out.len = count + sizeof(last_word);

	if (down_interruptible(&dev_priv->sem))
		return -ERESTARTSYS;

    if (dev_priv->cmd_handshake_polling) {
        //disable PREVMSG_FETCH & NEWMSG_COMING
        rtl_mdio_mask_write(NFBI_REG_IMR, (IM_PREVMSG_FETCH|IM_NEWMSG_COMING), 0x0000);
    }
    else{
        //enable PREVMSG_FETCH & NEWMSG_COMING
        //rtl_mdio_mask_write(NFBI_REG_IMR, (IM_PREVMSG_FETCH|IM_NEWMSG_COMING), (IM_PREVMSG_FETCH|IM_NEWMSG_COMING));
    }
    
    dev_priv->filp = filp;
    dev_priv->tx_command_frames++;
    retransmit_count = priv->retransmit_count;
    //transmit command frame
retransmit:
    if (dev_priv->cmd_handshake_polling)
        rtl_mdio_write(NFBI_REG_ISR, (IP_PREVMSG_FETCH|IP_NEWMSG_COMING));

	priv->tx_cmd_transmitting_len = 0;
	priv->state = STATE_TX_IN_PROGRESS;
    transmit_msg(priv);
	do {
        if (dev_priv->cmd_handshake_polling) {
            if (msleep_interruptible(dev_priv->interrupt_timeout*10)) { //in ms
                priv->state = STATE_TX_INIT;
    			dev_priv->filp = NULL;
    			dev_priv->tx_stop_by_signals++;
    		    up(&dev_priv->sem); /* release the lock */
    			return -ERESTARTSYS; /* signal: tell the fs layer to handle it */
    		}
            rtl_mdio_read(NFBI_REG_ISR, &status);
            if (!(status & IP_PREVMSG_FETCH)) {
    	        priv->state = STATE_TX_INIT;
    	        dev_priv->tx_interupt_timeouts++;
    	        if (retransmit_count > 0) {
    	            retransmit_count--;
    	            dev_priv->tx_retransmit_counts++;
    	            printk(".\n");
    	            goto retransmit;
    	        }
    	        dev_priv->filp = NULL;
    	        up(&dev_priv->sem); // release the lock
    			return -ETIME;
    		}
    		rtl_mdio_write(NFBI_REG_ISR, IP_PREVMSG_FETCH);
        }
        else {
    		//PDEBUG("\"%s\"(pid %i) writing: going to sleep\n", current->comm, current->pid);
    	    /* register the timer */
    	    dev_priv->timer_expired = 0; /* if timer expired, wake up processes in the wait queue */
    	    dev_priv->timer.data = 0;
    	    dev_priv->timer.function = nfbi_timer_fn;
    	    dev_priv->timer.expires = jiffies + dev_priv->interrupt_timeout; /* in jiffies */
    	    //curr_time = jiffies;
    	    add_timer(&dev_priv->timer);
    		if (wait_event_interruptible(dev_priv->wq, (dev_priv->tx_msg_is_fetched || dev_priv->timer_expired))) {
    			priv->state = STATE_TX_INIT;
    			del_timer(&dev_priv->timer);
    			dev_priv->filp = NULL;
    			dev_priv->tx_stop_by_signals++;
    			//rtl_mdio_read(NFBI_REG_IMR, &imr);
    			//rtl_mdio_write(NFBI_REG_IMR, (imr&0xfff9)); //disable PREVMSG_FETCH & NEWMSG_COMING
    		    up(&dev_priv->sem); /* release the lock */
    			return -ERESTARTSYS; /* signal: tell the fs layer to handle it */
    		}
    		if (dev_priv->timer_expired) { //timeout
    	        //printk("tx curr_time=%lx jiffies=%lx, %ld ms\n", curr_time, jiffies, (jiffies-curr_time)*1000/HZ);
    	        priv->state = STATE_TX_INIT;
    	        dev_priv->tx_interupt_timeouts++;
    	        if (retransmit_count > 0) {
    	            retransmit_count--;
    	            dev_priv->tx_retransmit_counts++;
    	            printk(".\n");
    	            goto retransmit;
    	        }
    	        dev_priv->filp = NULL;
    	        //rtl_mdio_read(NFBI_REG_IMR, &imr);
    	        //rtl_mdio_write(NFBI_REG_IMR, (imr&0xfff9)); //disable PREVMSG_FETCH & NEWMSG_COMING
    	        up(&dev_priv->sem); // release the lock
    			return -ETIME;
    		}
    		else
    		    del_timer(&dev_priv->timer);
    	}

    	if (priv->tx_cmd_transmitting_len >= priv->data_out.len) 
		    priv->state = STATE_RX_INIT;
        else {
            if (dev_priv->tx_cmdword_interval > 0) {
                if (dev_priv->cmd_handshake_polling) {
                    if(dev_priv->tx_cmdword_interval > dev_priv->interrupt_timeout)
                        mdelay((dev_priv->tx_cmdword_interval-dev_priv->interrupt_timeout)*10);
                }
                else {
                    mdelay(dev_priv->tx_cmdword_interval*10);
                }
            }
            transmit_msg(priv);
        }
	} while (priv->state != STATE_RX_INIT);

    dev_priv->tx_done_command_frames++;
    
    //receive status frame
    RESET_RX_STATE;
	while (priv->state != STATE_RX_FINISHED) {
        if (dev_priv->cmd_handshake_polling) {
            curr_time = jiffies;
wait_response:
            if (msleep_interruptible(dev_priv->interrupt_timeout*10)) { //in ms
    			priv->state = STATE_TX_INIT;
    			dev_priv->rx_stop_by_signals++;
    		    up(&dev_priv->sem); /* release the lock */
    			return -ERESTARTSYS; /* signal: tell the fs layer to handle it */
    		}
            rtl_mdio_read(NFBI_REG_ISR, &status);
            if (!(status & IP_NEWMSG_COMING)) {
    	        if (priv->state == STATE_RX_INIT) {
    	            if (jiffies < (priv->response_timeout+curr_time))
    	                goto wait_response;
    	            dev_priv->rx_response_timeouts++;
    	        }
    	        else
    	            dev_priv->rx_interupt_timeouts++;
    	        priv->state = STATE_TX_INIT;
    	        if (retransmit_count > 0) {
    	            retransmit_count--;
    	            dev_priv->tx_retransmit_counts++;
    	            printk(".\n");
    	            goto retransmit;
    	        }
    	        up(&dev_priv->sem); // release the lock
    	        PDEBUG("ETIME=%d\n", ETIME);
    			return -ETIME;
    		}
    		rtl_mdio_write(NFBI_REG_ISR, IP_NEWMSG_COMING);
    	}
        else {
    		//PDEBUG("\"%s\"(pid %i) reading: going to sleep\n", current->comm, current->pid);
    		/* register the timer */
    	    dev_priv->timer_expired = 0; /* if timer expired, wake up processes in the wait queue */
    	    dev_priv->timer.data = 0;
    	    dev_priv->timer.function = nfbi_timer_fn;
    	    if (priv->state == STATE_RX_INIT)
                dev_priv->timer.expires = jiffies + priv->response_timeout; /* in jiffies */
    	    else /* STATE_RX_WAIT_LEN or STATE_RX_WAIT_DATA*/
    	        dev_priv->timer.expires = jiffies + dev_priv->interrupt_timeout; /* in jiffies */
    	    //curr_time = jiffies;
    	    add_timer(&dev_priv->timer);
    		if (wait_event_interruptible(dev_priv->wq, (dev_priv->rx_msg_is_coming || dev_priv->timer_expired))) {
    			priv->state = STATE_TX_INIT;
    			del_timer(&dev_priv->timer);
    			dev_priv->rx_stop_by_signals++;
    			//rtl_mdio_read(NFBI_REG_IMR, &imr);
    			//rtl_mdio_write(NFBI_REG_IMR, (imr&0xfff9)); //disable PREVMSG_FETCH & NEWMSG_COMING
    		    up(&dev_priv->sem); /* release the lock */
    			return -ERESTARTSYS; /* signal: tell the fs layer to handle it */
    		}
    
    	    if (dev_priv->timer_expired) { //timeout
    	        //printk("rx curr_time=%lx jiffies=%lx, %ld ms\n", curr_time, jiffies, (jiffies-curr_time)*1000/HZ);
    	        if (priv->state == STATE_RX_INIT)
    	            dev_priv->rx_response_timeouts++;
    	        else
    	            dev_priv->rx_interupt_timeouts++;
    	        priv->state = STATE_TX_INIT;
    	        if (retransmit_count > 0) {
    	            retransmit_count--;
    	            dev_priv->tx_retransmit_counts++;
    	            printk(".\n");
    	            goto retransmit;
    	        }
    	        //rtl_mdio_read(NFBI_REG_IMR, &imr);
    	        //rtl_mdio_write(NFBI_REG_IMR, (imr&0xfff9)); //disable PREVMSG_FETCH & NEWMSG_COMING
    	        up(&dev_priv->sem); // release the lock
    	        PDEBUG("ETIME=%d\n", ETIME);
    			return -ETIME;
    		}
    		else
    		    del_timer(&dev_priv->timer);
    	}

        dev_priv->rx_msg_is_coming = 0;
#ifdef SIMULATION
		data = register_read_dw(NFBI_REG_RSR);
#else
		rtl_mdio_read(NFBI_REG_RSR, &data);
#endif
        dev_priv->rx_words++;
		//printk("data=%04x\n", data);
	    process_rx_msg(priv, data);
	}
    dev_priv->rx_status_frames++;
    dev_priv->filp = NULL;
    //rtl_mdio_read(NFBI_REG_IMR, &imr);
    //rtl_mdio_write(NFBI_REG_IMR, (imr&0xfff9)); //disable PREVMSG_FETCH & NEWMSG_COMING
    
    if (dev_priv->cmd_handshake_polling)
        rtl_mdio_write(NFBI_REG_ISR, (IP_PREVMSG_FETCH|IP_NEWMSG_COMING));
        
    up(&dev_priv->sem);
    
	return count;
}


#ifdef SIMULATION
static int read_proc(char *buf, char **start, off_t off,
				int count, int *eof, void *data)
{
	int size = 0;

    //printk("buf=%p *start=%p count=%d off=%d\n", buf, *start, count, off);
    
    if (down_interruptible(&sim_sem))
		return -ERESTARTSYS;

	if ((data_out_len > 0) && (off==0)) {	
		
//fetch_again:		
		//while (data_out_len > 0) {
			data_out_len = 0;
			size += sprintf(&buf[size], "%04x ", data_out);			
		//}

		msg_is_fetched = 1;
		nfbi_interrupt(0, (void *)dev_priv, (struct pt_regs *)NULL);

//		if (data_out_len > 0)
//			goto fetch_again;		

		strcat(&buf[size++], "\n");	
	}

    up(&sim_sem);
    *eof = 1;
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
				PDEBUG("error hex format [%x]!\n", *s);
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
	//int i;
	
	//printk("file=%p buffer=%p count=%d\n", file, buffer, count);
	/*
	printk("buffer=");
	for(i=0;i<count; i++) {
	    printk("%02x ", buffer[i]);
	}
	printk("\n");
	*/
	//data_in_len = 0;
	
	while (len > 0) {
		memcpy(tmp, buffer, 4);
		tmp[4] = '\0';

		in_data = _atoi(tmp, 16);

        if (down_interruptible(&sim_sem))
    		return -ERESTARTSYS;
    		
        data_in_len = 0;
		memcpy(&data_in[data_in_len], &in_data, 2);
		data_in_len = 2;
        data_in_read_idx = 0;
    	msg_is_coming = 1;
	    nfbi_interrupt(0, (void *)dev_priv, (struct pt_regs *)NULL);
        up(&sim_sem);

        interruptible_sleep_on(&sim_wq);
        //if (interruptible_sleep_on(&sim_wq)) {
	        //return -ERESTARTSYS; /* signal: tell the fs layer to handle it */
		//}

		len -= 5;
		buffer += 5;
	}
	
    return count;
}
#endif

#define RTL8198_PATCH1
#ifdef RTL8198_PATCH1
static int last_addr;
#endif
/*================================================================*/
/* Description :
 *   char *buf : byte array should be big endian image for RTL8197B
 */
int nfbi_mem_write(int addr, int len, char *buf)
{
    int ret, i;
    unsigned short val;
    char tmp[4];

    if (len > NFBI_MAX_BULK_MEM_SIZE)
        return -1;
    if (0 != rtl_mdio_write(NFBI_REG_CMD, 0x0000)) //write mode
        return -1;
    if (0 != rtl_mdio_write(NFBI_REG_ADDH, (addr>>16)&0xffff)) //address H
        return -1;
    if (0 != rtl_mdio_write(NFBI_REG_ADDL, addr&0xffff)) //address L
        return -1;

#ifdef RTL8198_PATCH1
    if ((last_addr&0xfffff000)!= (addr&0xfffff000)) {
      	    if (0 != rtl_mdio_write(NFBI_REG_DH, 0x0000)) //data H
                return -1;
            if (0 != rtl_mdio_write(NFBI_REG_DL, 0x0000)) //data L
                return -1;
#ifdef CHECK_NFBI_BUSY_BIT
            // check NFBI hardware status
        	do {
                ret = rtl_mdio_read(NFBI_REG_CMD, &val);
                if (ret != 0)
                    return -1;
            } while (val&BM_BUSY); //wait busy bit to zero
#endif
            //set address again
    	    if (0 != rtl_mdio_write(NFBI_REG_ADDH, (addr>>16)&0xffff)) //address H
        	return -1;
    	    if (0 != rtl_mdio_write(NFBI_REG_ADDL, addr&0xffff)) //address L
        	return -1;
    }
    last_addr = addr;
#endif
    
    if (len >= 4) {
    	for (i=0; i<len; i+=4) {
#ifndef HOST_IS_PANABOARD
      	    if (0 != rtl_mdio_write(NFBI_REG_DH, *(unsigned short *)(buf+i))) //data H
                return -1;
            if (0 != rtl_mdio_write(NFBI_REG_DL, *(unsigned short *)(buf+i+2))) //data L
                return -1;
#else
            val  = (*(buf+i)<<8)&0xff00;
            val |= (*(buf+i+1))&0x00ff;
      	    if (0 != rtl_mdio_write(NFBI_REG_DH, val)) //data H
                return -1;
            val  = (*(buf+i+2)<<8)&0xff00;
            val |= (*(buf+i+3))&0x00ff;
            if (0 != rtl_mdio_write(NFBI_REG_DL, val)) //data L
                return -1;
#endif /*HOST_IS_PANABOARD*/
#ifdef CHECK_NFBI_BUSY_BIT
            // check NFBI hardware status
        	do {
                ret = rtl_mdio_read(NFBI_REG_CMD, &val);
                if (ret != 0)
                    return -1;
            } while (val&BM_BUSY); //wait busy bit to zero
#endif
        }
	}
	
	if ( (len%4) )
	{
	    memset(tmp, 0, 4);
		memcpy(tmp, (buf+(len/4)*4),(len%4));
#ifndef HOST_IS_PANABOARD
        if (0 != rtl_mdio_write(NFBI_REG_DH, *(unsigned short *)tmp)) //data H
            return -1;
        if (0 != rtl_mdio_write(NFBI_REG_DL, *(unsigned short *)(tmp+2))) //data L
            return -1;
#else
        val  = (tmp[0]<<8)&0xff00;
        val |= (tmp[1])&0x00ff;
        if (0 != rtl_mdio_write(NFBI_REG_DH, val)) //data H
            return -1;
        val  = (tmp[2]<<8)&0xff00;
        val |= (tmp[3])&0x00ff;
        if (0 != rtl_mdio_write(NFBI_REG_DL, val)) //data L
            return -1;
#endif /*HOST_IS_PANABOARD*/
#ifdef CHECK_NFBI_BUSY_BIT
        // check NFBI hardware status
        do {
            ret = rtl_mdio_read(NFBI_REG_CMD, &val);
            if (ret != 0)
                return -1;
        } while (val&BM_BUSY); //wait busy bit to zero
#endif
    }
    return 0;
}

int nfbi_mem_read(int addr, int len, char *buf)
{
    int ret, i;
    unsigned short val;
    char tmp[4];

    if (len > NFBI_MAX_BULK_MEM_SIZE)
        return -1;
    if (0 != rtl_mdio_write(NFBI_REG_CMD, 0x8000)) //read mode
        return -1;
    if (0 != rtl_mdio_write(NFBI_REG_ADDH, (addr>>16)&0xffff)) //address H
        return -1;
    if (0 != rtl_mdio_write(NFBI_REG_ADDL, addr&0xffff)) //address L
        return -1;

#ifdef RTL8198_PATCH1
    if ((last_addr&0xfffff000)!= (addr&0xfffff000)) {
    	    if (0 != rtl_mdio_read(NFBI_REG_DH, (unsigned short *)buf)) //data H
                return -1;
            if (0 != rtl_mdio_read(NFBI_REG_DL, (unsigned short *)(buf+2))) //data L
                return -1;
#ifdef CHECK_NFBI_BUSY_BIT
            // check NFBI hardware status
        	do {
                ret = rtl_mdio_read(NFBI_REG_CMD, &val);
                if (ret != 0)
                    return -1;
            } while (val&BM_BUSY); //wait busy bit to zero
#endif
            //set address again
    	    if (0 != rtl_mdio_write(NFBI_REG_ADDH, (addr>>16)&0xffff)) //address H
        	return -1;
    	    if (0 != rtl_mdio_write(NFBI_REG_ADDL, addr&0xffff)) //address L
        	return -1;
    }
    last_addr = addr;
#endif

    if (len >= 4) {
    	for (i=0; i<len; i+=4) {
#ifndef HOST_IS_PANABOARD
      	    if (0 != rtl_mdio_read(NFBI_REG_DH, (unsigned short *)(buf+i))) //data H
                return -1;
            if (0 != rtl_mdio_read(NFBI_REG_DL, (unsigned short *)(buf+i+2))) //data L
                return -1;
#else
      	    if (0 != rtl_mdio_read(NFBI_REG_DH, &val)) //data H
                return -1;
            *(buf+i)   = (char)( (val>>8)&0x00ff );
            *(buf+i+1) = (char)( val&0x00ff );
            if (0 != rtl_mdio_read(NFBI_REG_DL, &val)) //data L
                return -1;
            *(buf+i+2)   = (char)( (val>>8)&0x00ff );
            *(buf+i+3) = (char)( val&0x00ff );
#endif /*HOST_IS_PANABOARD*/
#ifdef CHECK_NFBI_BUSY_BIT
            // check NFBI hardware status
        	do {
                ret = rtl_mdio_read(NFBI_REG_CMD, &val);
                if (ret != 0)
                    return -1;
            } while (val&BM_BUSY); //wait busy bit to zero
#endif
        }
	}
	
	if ( (len%4) )
	{
#ifndef HOST_IS_PANABOARD
        if (0 != rtl_mdio_read(NFBI_REG_DH, (unsigned short *)tmp)) //data H
            return -1;
        if (0 != rtl_mdio_read(NFBI_REG_DL, (unsigned short *)(tmp+2))) //data L
            return -1;
#else
        if (0 != rtl_mdio_read(NFBI_REG_DH, &val)) //data H
            return -1;
        tmp[0]  = (char)( (val>>8)&0x00ff );
        tmp[1]  = (char)( val&0x00ff );
        if (0 != rtl_mdio_read(NFBI_REG_DL, &val)) //data L
            return -1;
        tmp[2]  = (char)( (val>>8)&0x00ff );
        tmp[3]  = (char)( val&0x00ff );
#endif
#ifdef CHECK_NFBI_BUSY_BIT
        // check NFBI hardware status
        do {
            ret = rtl_mdio_read(NFBI_REG_CMD, &val);
            if (ret != 0)
                return -1;
        } while (val&BM_BUSY); //wait busy bit to zero
#endif
        memcpy((buf+(len/4)*4), tmp, (len%4));
    }
    return 0;
}

void nfbi_hw_reset(int type)
{
    switch(type) {
      case 0:  //pull low
        Set_NFBI_RESET_L();
        break;
      case 1:  //pull high
        Set_NFBI_RESET_H();
        break;
      case 2: //hardware reset
#ifndef HOST_IS_PANABOARD
	// In 8198 platform, interrupt will come very soon after pull-low reset pin.
	// When the interrupt occurs, the hw reset sequence will be interleaved by ISR
	// and that would cause the reset action unable to compete.
	// Therefore, it's necessary to disable interrupt before hardware reset.
	REG32(PABIMR_REG) = REG32(PABIMR_REG) & 0x3fffffff; //set B7 falling interrupt(bit31,30), 0x00 diable, 0x01 falling, 0x02 rasing, 0x03 both
#endif
        Set_NFBI_RESET_L();
        //mdelay(400); //To reset RTL8197B CPU, the RESETn pin must be pull-low at least 350 ms.
        mdelay(1000);
        Set_NFBI_RESET_H();
		mdelay(1000);
#ifdef HOST_IS_PANABOARD
        /* Ebina add for Micrel Isolation */
        avev3_mdio_write(avev3_dev, 0x1, MII_BMCR, 0x3400);
#endif
#ifndef HOST_IS_PANABOARD
	// enable interrupt after hardware reset
	REG32(PABIMR_REG) = REG32(PABIMR_REG) | (0x01 <<30); //set B7 falling interrupt(bit31,30), 0x00 diable, 0x01 falling, 0x02 rasing, 0x03 both
#endif
        break;
    }
}

unsigned char nfbi_probephyaddr(void)
{
	int i;
	unsigned int val = 0;
	int reg = NFBI_REG_PHYID2; //PHYID2
	
#ifndef HOST_IS_PANABOARD
#ifdef MDCIO_GPIO_SIMULATION
	unsigned short regval;
#else
	int timeout = MDIO_TIMEOUT;
#endif
#endif
	unsigned char phyaddr;

	for (i=0; i<32; i++) {
		phyaddr=i;
		
#ifndef HOST_IS_PANABOARD
#ifdef MDCIO_GPIO_SIMULATION
        smiRead(phyaddr, reg, &regval);
        val = regval;
#else
		REG32(MDCIOCR) = 0x7fffffff & ((phyaddr<<24) | ((reg&0x1f)<<16));
    	while (--timeout) {
    	    val = REG32(MDCIOSR);
    	    if ((val&0x80000000) == 0)
    	        break;
    	}
		if (timeout==0)
			printk("Timeout! Probe PHY Addr=%x fail!\n",phyaddr);
#endif
#else
		val = avev3_mdio_read(avev3_dev, phyaddr, reg);
#endif
		printk("Probe PHY ADDR=0x%x  reg3=0x%x\n",phyaddr, val&0xffff);
		if (((val&0xffff)==NFBI_REG_PHYID2_DEFAULT) ||
		    ((val&0xffff)==NFBI_REG_PHYID2_DEFAULT2)) {  //PHY Addr
		    printk("Pass! Probe PHY ADDR = %d\n", phyaddr);
		    dev_priv->ready = 1;
			return phyaddr;
		}
	}

	if(i==32)
	    printk("Error! NFBI maybe not connect!\n");
	
	dev_priv->ready = 0;
	return 0xff;
}

void dump_private_data(void)
{
	int i;
	struct nfbi_priv *priv;
    
    printk("ready=%d\n", dev_priv->ready);
    printk("mdio_phyaddr=%d\n", mdio_phyaddr);
    printk("cmd_handshake_polling=%d\n", dev_priv->cmd_handshake_polling);
	printk("hcd_pid=%d\n", dev_priv->hcd_pid);
	printk("evt_que_head=%d\n", dev_priv->evt_que_head);
    printk("evt_que_tail=%d\n", dev_priv->evt_que_tail);
    printk("evt_que count=%d\n", CIRC_CNT(dev_priv->evt_que_head, dev_priv->evt_que_tail, EV_QUE_MAX));
    printk("evt_que space=%d\n", CIRC_SPACE(dev_priv->evt_que_head, dev_priv->evt_que_tail, EV_QUE_MAX));
	printk("tx_cmdword_interval:%d (in 10ms)\n", dev_priv->tx_cmdword_interval);
	printk("tx_msg_is_fetched=%d\n", dev_priv->tx_msg_is_fetched);
    printk("rx_msg_is_coming=%d\n", dev_priv->rx_msg_is_coming);
    printk("interrupt_timeout=%d (in 10ms)\n", dev_priv->interrupt_timeout);
	printk("dev_priv->filp:%p\n", dev_priv->filp);
	
	if (dev_priv->filp != NULL) {
	    priv = (struct nfbi_priv *)dev_priv->filp->private_data;
    	printk("state:");
    	switch(priv->state) {
    	  case STATE_TX_INIT:
    	  	printk("STATE_TX_INIT\n");
    	  	break;
    	  case STATE_TX_IN_PROGRESS:
    	  	printk("STATE_TX_IN_PROGRESS\n");
    	  	break;
          case STATE_RX_INIT:
    	  	printk("STATE_RX_INIT\n");
    	  	break;
    	  case STATE_RX_WAIT_LEN:
    	  	printk("STATE_RX_WAIT_LEN\n");
    	  	break;
    	  case STATE_RX_WAIT_DATA:
    	  	printk("STATE_RX_WAIT_DATA\n");
    	  	break;
    	  case STATE_RX_FINISHED:
    	  	printk("STATE_RX_FINISHED\n");
    	  	break;
    	}
        printk("Tx:\n");
    	printk("data_out:");
    	for(i=0;i<priv->data_out.len;i++) {
    		printk("%02x ", priv->data_out.buf[i]);
    	}
    	printk("\n");
    	printk("tx_cmd_transmitting_len=%d\n", priv->tx_cmd_transmitting_len);
    
    	printk("Rx:\n");
    	printk("data_in:");
    	for(i=0;i<priv->data_in.len;i++) {
    		printk("%02x ", priv->data_in.buf[i]);
    	}
    	printk("\n");
    	printk("rx_status_remain_len=%d\n", priv->rx_status_remain_len);
    	printk("rx_status_time=%lx jiffies=%lx (%d sec)\n", priv->rx_status_time, jiffies, (int)(jiffies-priv->rx_status_time)/HZ);
    }
    printk("===============================================\n");
    printk("Statistics:\n");
    printk("tx_command_frames=%d\n", dev_priv->tx_command_frames);
    printk("tx_done_command_frames=%d\n", dev_priv->tx_done_command_frames);
    printk("tx_retransmit_counts=%d\n", dev_priv->tx_retransmit_counts);
    printk("tx_words=%d\n", dev_priv->tx_words);
    printk("tx_interupt_timeouts=%d\n", dev_priv->tx_interupt_timeouts);
    printk("tx_stop_by_signals=%d\n", dev_priv->tx_stop_by_signals);
    printk("rx_status_frames=%d\n", dev_priv->rx_status_frames);
    printk("rx_words=%d\n", dev_priv->rx_words);
    printk("rx_response_timeouts=%d\n", dev_priv->rx_response_timeouts);
    printk("rx_interupt_timeouts=%d\n", dev_priv->rx_interupt_timeouts);
    printk("rx_stop_by_signals=%d\n", dev_priv->rx_stop_by_signals);
    printk("rx_not_1st_word_errors=%d\n", dev_priv->rx_not_1st_word_errors);
    printk("rx_1st_byte_errors=%d\n", dev_priv->rx_1st_byte_errors);
    printk("rx_cmdid_not_match_errors=%d\n", dev_priv->rx_cmdid_not_match_errors);
    printk("rx_reset_by_sync_bit_errors=%d\n", dev_priv->rx_reset_by_sync_bit_errors);
    printk("rx_checksum_errors=%d\n", dev_priv->rx_checksum_errors);

#ifdef SIMULATION
    printk("===============================================\n");
    printk("msg_is_comming=%d\n", msg_is_coming);
    printk("data_in_read_idx=%d\n", data_in_read_idx);
    for(i=0;i<data_in_len;i++) {
		printk("%02x ", data_in[i]);
	}
	printk("\n");
	
    printk("msg_is_fetched=%d\n", msg_is_fetched);
    printk("data_out_len=%d\n", data_out_len);
    printk("data_out=%04x\n", data_out);
#endif
}

/*================================================================*/

void nfbi_private_command(int type)
{
    switch(type) {
      case 0: //pull low
      case 1: //pull high
      case 2: //hardware reset
        nfbi_hw_reset(type);
        break;
      case 3:
        nfbi_probephyaddr();
        break;
      case 4: //dump private data
      	dump_private_data();
        break;
      case 5: //reset event queue
        dev_priv->evt_que_head = 0;
        dev_priv->evt_que_tail = 0;
        break;
    }
}

#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP_IS_HOST

void SetCurrentPhyId(unsigned char dsp_id)
{
	if (dsp_id == 0)
	{
		mdio_phyaddr = 8;
	}
	else if (dsp_id == 1)
	{
		mdio_phyaddr = 16;
	}
	//printk("Set current phy ID to %d.\n", mdio_phyaddr);
}

unsigned char GetCurrentPhyId(void)
{
	return mdio_phyaddr;
}

unsigned char CheckDspIfAllSoftwareReady(void)
{
	unsigned short val;
	int retval = 0;
	
	retval = rtl_mdio_read(0x17 & 0xffff, &val);//SYSSR
	//printk("val = 0x%x\n", val);
	//printk("val&0x400 = 0x%x\n", val&0x400);

	if (retval == 0)
	{
		if (val & 0x400)
			return 1;	// AllSoftwareReady
		else
			return 0;	// AllSoftware NOT Ready
	}
	return 0;;
}

unsigned char SetDspIdToDsp(int dsp_id)
{
	unsigned short val;
	int retval = 0;
	
	retval = rtl_mdio_read(0x18 & 0xffff, &val);//SYSCR
	if (retval != 0)
	{
		printk("SetDspIdToDsp Fail\n");
		return 0;//Fail
	}
	
	val = (val & 0xFFF0) | dsp_id;
	retval = rtl_mdio_write(0x18 & 0xffff, val&0xffff);
	if (retval != 0)
	{
		printk("SetDspIdToDsp Fail\n");
		return 0;//Fail
	}

	printk("SetDspIdToDsp%d \n", dsp_id);

	return 1;
}

#endif

/*
 * The ioctl() implementation
 * Refer to the book "Linux Device Drivers" by Alessandro Rubini and Jonathan Corbet, 
 * published by O'Reilly & Associates.
 */
static int nfbi_ioctl(struct inode *inode, struct file *filp,
                 unsigned int cmd, unsigned long arg)
{
	//int err = 0;
	int retval = 0, tmp;
	unsigned short val;
	struct nfbi_bulk_mem_param *pbulkmem;
	struct nfbi_mem32_param mem32_param;
	struct evt_msg evt;
	struct nfbi_priv *priv;
    
	/*
	 * extract the type and number bitfields, and don't decode
	 * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
	 */
	if (_IOC_TYPE(cmd) != NFBI_IOC_MAGIC) return -ENOTTY;
	if (_IOC_NR(cmd) > NFBI_IOCTL_MAXNR) return -ENOTTY;

	/*
	 * the direction is a bitmask, and VERIFY_WRITE catches R/W
	 * transfers. `Type' is user-oriented, while
	 * access_ok is kernel-oriented, so the concept of "read" and
	 * "write" is reversed
	 */
	/*
	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err =  !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));
	if (err) return -EFAULT;
    */
    
	switch(cmd) {
	  case NFBI_IOCTL_PRIV_CMD:
	    retval = get_user(tmp, (int *)arg);
	    if (retval == 0) {
	        nfbi_private_command(tmp);
	    }
	    break;
	  case NFBI_IOCTL_REGREAD:
	    retval = get_user(tmp, (int *)arg);
	    if (retval == 0) {
            /* The register address to be read is stored at the high word of tmp. */
    	    retval = rtl_mdio_read((tmp >> 16) & 0xffff, &val); //@Pana_TBD
            if (retval == 0) {
                /* 
        	     * stored register address at the high word of tmp and
        	     * value at the low word of tmp, and then copy to user space
            	 */
                tmp = (tmp & 0xffff0000) | val;
        		retval = put_user(tmp, (int *)arg);
    	    }
	    }
		break;
	  case NFBI_IOCTL_REGWRITE:
	    retval = get_user(tmp, (int *)arg);
	    if (retval == 0) {
    	    /* 
    	     * The register address is stored at the high word of tmp.
        	 * The value to be written is stored at the low word of tmp.
        	 */
    	    retval = rtl_mdio_write((tmp >> 16) & 0xffff, tmp&0xffff); //@Pana_TBD
    	}
		break;
      case NFBI_IOCTL_HCD_PID:
	    if (0 == get_user(tmp, (int *)arg)) {
	        dev_priv->hcd_pid = tmp;
	        //PDEBUG("hcd_pid=%d\n", dev_priv->hcd_pid);
	    }
        break;
      case NFBI_IOCTL_GET_EVENT:
		//retrieve_evt(priv, &evt);
		retrieve_evt(&evt);
        retval = copy_to_user((struct evt_msg *)arg, &evt, sizeof(struct evt_msg));
        break;
#ifndef HOST_IS_PANABOARD
      case NFBI_IOCTL_MEM32_WRITE:
    	retval = copy_from_user(&mem32_param, (struct nfbi_mem32_param *)arg, sizeof(struct nfbi_mem32_param));
	    //printk("mem32_param.addr=%x mem32_param.val=%x\n", mem32_param.addr, mem32_param.val);
        if (retval == 0)
	        retval = nfbi_mem_write(mem32_param.addr, sizeof(int), (char *)&(mem32_param.val));
        break;
      case NFBI_IOCTL_MEM32_READ:
    	retval = copy_from_user(&mem32_param, (struct nfbi_mem32_param *)arg, sizeof(struct nfbi_mem32_param));
        if (retval == 0)
	        retval = nfbi_mem_read(mem32_param.addr, sizeof(int), (char *)&(mem32_param.val));
        if (retval == 0)
	        retval = copy_to_user((struct nfbi_mem32_param *)arg, &mem32_param, sizeof(struct nfbi_mem32_param));
	    //printk("mem32_param.addr=%x mem32_param.val=%x\n", mem32_param.addr, mem32_param.val);
        break;
#else
      case NFBI_IOCTL_MEM32_WRITE:
    	retval = copy_from_user(&mem32_param, (struct nfbi_mem32_param *)arg, sizeof(struct nfbi_mem32_param));
	    //printk("mem32_param.addr=%x mem32_param.val=%x\n", mem32_param.addr, mem32_param.val);
	    tmp = htonl(mem32_param.val);
        if (retval == 0)
	        retval = nfbi_mem_write(mem32_param.addr, sizeof(int), (char *)&(tmp));
        break;
      case NFBI_IOCTL_MEM32_READ:
    	retval = copy_from_user(&mem32_param, (struct nfbi_mem32_param *)arg, sizeof(struct nfbi_mem32_param));
        if (retval == 0)
	        retval = nfbi_mem_read(mem32_param.addr, sizeof(int), (char *)&(tmp));
	    mem32_param.val = ntohl (tmp);
        if (retval == 0)
	        retval = copy_to_user((struct nfbi_mem32_param *)arg, &mem32_param, sizeof(struct nfbi_mem32_param));
	    //printk("mem32_param.addr=%x mem32_param.val=%x\n", mem32_param.addr, mem32_param.val);
        break;
#endif/*HOST_IS_PANABOARD*/
      case NFBI_IOCTL_BULK_MEM_WRITE:
        pbulkmem  = (struct nfbi_bulk_mem_param *)kmalloc(sizeof(struct nfbi_bulk_mem_param), GFP_KERNEL);
	    if(!pbulkmem) {
    	    printk(KERN_ERR DRIVER_NAME": unable to kmalloc\n");
    		return -ENOMEM;
    	}
    	retval = copy_from_user(pbulkmem, (struct nfbi_bulk_mem_param *)arg, sizeof(struct nfbi_bulk_mem_param));
	    //printk("pbulkmem->addr=%x pbulkmem->len=%d\n", pbulkmem->addr, pbulkmem->len);
        if (retval == 0)
	        retval = nfbi_mem_write(pbulkmem->addr, pbulkmem->len, pbulkmem->buf);
	    kfree(pbulkmem);
	    printk(">");
        break;
      case NFBI_IOCTL_BULK_MEM_READ:
        pbulkmem  = (struct nfbi_bulk_mem_param *)kmalloc(sizeof(struct nfbi_bulk_mem_param), GFP_KERNEL);
	    if(!pbulkmem) {
    	    printk(KERN_ERR DRIVER_NAME": unable to kmalloc\n");
    		return -ENOMEM;
    	}
    	//retval = copy_from_user(pbulkmem, (struct nfbi_bulk_mem_param *)arg, sizeof(struct nfbi_bulk_mem_param));
    	retval = copy_from_user(pbulkmem, (struct nfbi_bulk_mem_param *)arg, 8);
    	if (retval == 0) {
    	    //printk("pbulkmem->addr=%x pbulkmem->len=%d\n", pbulkmem->addr, pbulkmem->len);
	        retval = nfbi_mem_read(pbulkmem->addr, pbulkmem->len, pbulkmem->buf);
	    }
	    if (retval == 0)
	        retval = copy_to_user((struct nfbi_bulk_mem_param *)arg, pbulkmem, sizeof(struct nfbi_bulk_mem_param));
	    kfree(pbulkmem);
	    printk("<");
        break;
      case NFBI_IOCTL_TX_CMDWORD_INTERVAL:
        retval = get_user(tmp, (int *)arg);
	    if (retval == 0) {
	        if (tmp & 0x10000)
	            dev_priv->tx_cmdword_interval = tmp & 0xffff; //set
	        else {
	            tmp = dev_priv->tx_cmdword_interval; //get
	            retval = put_user(tmp, (int *)arg);
	        }
	    }
        break;
      case NFBI_IOCTL_INTERRUPT_TIMEOUT:
        retval = get_user(tmp, (int *)arg);
	    if (retval == 0) {
	        if (tmp & 0x10000)
	            dev_priv->interrupt_timeout = tmp & 0xffff; //set
	        else {
	            tmp = dev_priv->interrupt_timeout; //get
	            retval = put_user(tmp, (int *)arg);
	        }
	    }
        break;
      case NFBI_IOCTL_RETRANSMIT_COUNT:
        retval = get_user(tmp, (int *)arg);
	    if (retval == 0) {
	        priv = (struct nfbi_priv *)filp->private_data;
            priv->retransmit_count = tmp;
            //PDEBUG("priv->retransmit_count=%d\n", priv->retransmit_count);
	    }
        break;
      case NFBI_IOCTL_RESPONSE_TIMEOUT:
        retval = get_user(tmp, (int *)arg);
	    if (retval == 0) {
	        priv = (struct nfbi_priv *)filp->private_data;
            priv->response_timeout = tmp;
            //PDEBUG("priv->response_timeout=%d\n", priv->response_timeout);
	    }
        break;
      case NFBI_IOCTL_MDIO_PHYAD:
        retval = get_user(tmp, (int *)arg);
	    if (retval == 0) {
            mdio_phyaddr = (unsigned char)tmp;
            printk("set mdio_phyaddr = %d\n", mdio_phyaddr);
            if ((mdio_phyaddr==8) || (mdio_phyaddr==16))
                dev_priv->ready = 1;
            else
                dev_priv->ready = 0;
            //PDEBUG("mdio_phyaddr=%d\n", mdio_phyaddr);
	    }
        break;
      case NFBI_IOCTL_CMD_HANDSHAKE_POLLING:
        retval = get_user(tmp, (int *)arg);
	    if (retval == 0) {
	        if (tmp & 0x10000) {
	            dev_priv->cmd_handshake_polling = tmp & 0xffff; //set
	            if (dev_priv->cmd_handshake_polling) {
	                dev_priv->interrupt_timeout = NFBI_POLLING_INTERVAL_DEFAULT;
	                //disable PREVMSG_FETCH & NEWMSG_COMING
                    rtl_mdio_mask_write(NFBI_REG_IMR, (IM_PREVMSG_FETCH|IM_NEWMSG_COMING), 0x0000);
	            }
	            else {
	                dev_priv->interrupt_timeout = NFBI_INTERRUPT_TIMEOUT_DEFAULT;
	                //enable PREVMSG_FETCH & NEWMSG_COMING
                    rtl_mdio_mask_write(NFBI_REG_IMR, (IM_PREVMSG_FETCH|IM_NEWMSG_COMING), (IM_PREVMSG_FETCH|IM_NEWMSG_COMING));
	            }
	        }
	        else {
	            tmp = dev_priv->cmd_handshake_polling; //get
	            retval = put_user(tmp, (int *)arg);
	        }
	    }
        break;
#ifndef HOST_IS_PANABOARD
      //just for debugging purpose
      case NFBI_IOCTL_EW:
    	retval = copy_from_user(&mem32_param, (struct nfbi_mem32_param *)arg, sizeof(struct nfbi_mem32_param));
	    //printk("mem32_param.addr=%x mem32_param.val=%x\n", mem32_param.addr, mem32_param.val);
        if (retval == 0)
            REG32(mem32_param.addr) = mem32_param.val;
        break;
      case NFBI_IOCTL_DW:
    	retval = copy_from_user(&mem32_param, (struct nfbi_mem32_param *)arg, sizeof(struct nfbi_mem32_param));
        if (retval == 0)
            mem32_param.val = REG32(mem32_param.addr);
        if (retval == 0)
	        retval = copy_to_user((struct nfbi_mem32_param *)arg, &mem32_param, sizeof(struct nfbi_mem32_param));
	    //printk("mem32_param.addr=%x mem32_param.val=%x\n", mem32_param.addr, mem32_param.val);
        break;
#endif
	  default:  /* redundant, as cmd was checked against MAXNR */
		return -ENOTTY;
	}
	return retval;
}

static struct file_operations nfbi_fops = {
        owner:      THIS_MODULE,
		read:		nfbi_read,
		write:		nfbi_write,
		ioctl:  	nfbi_ioctl,
		open:		nfbi_open,
		release:	nfbi_release,
};

#ifndef HOST_IS_PANABOARD
static void __exit nfbi_exit(void)
{
    free_irq(irq, dev_priv);
    kfree(dev_priv);
    dev_priv = NULL;
}
#else
void nfbi_exit(void)
{
    //free_irq(irq, dev_priv); /*don't free*/
    kfree(dev_priv);
    dev_priv = NULL;
}
#endif

#ifndef HOST_IS_PANABOARD
static int __init nfbi_init(void)
#else
int nfbi_init(void)
#endif
{
#ifdef MDCIO_GPIO_SIMULATION
    _smiGpioInit();
#endif

	dev_priv = (struct nfbi_dev_priv *)kmalloc(sizeof (struct nfbi_dev_priv), GFP_KERNEL);
	if(!dev_priv) {
	    printk(KERN_ERR DRIVER_NAME": unable to kmalloc for nfbi_dev_priv\n");
		return -ENOMEM;
	}
	memset((void *)dev_priv, 0, sizeof (struct nfbi_dev_priv));
    dev_priv->hcd_pid = -1;
    dev_priv->cmd_handshake_polling = NFBI_CMD_HANDSHAKE_POLLING_DEFAULT;
    if (dev_priv->cmd_handshake_polling)
        dev_priv->interrupt_timeout = NFBI_POLLING_INTERVAL_DEFAULT;
    else
        dev_priv->interrupt_timeout = NFBI_INTERRUPT_TIMEOUT_DEFAULT;
    init_waitqueue_head(&(dev_priv->wq));
    init_timer(&dev_priv->timer);
	init_MUTEX(&dev_priv->sem);
	
#ifdef MDCIO_GPIO_SIMULATION
    init_timer(&dev_priv->mdc_timer);
    dev_priv->mdc_timer.data = 0;
    dev_priv->mdc_timer.function = mdc_timer_fn;
    dev_priv->mdc_timer.expires = jiffies + 1; /* in jiffies */
    add_timer(&dev_priv->mdc_timer);
#endif

	if (register_chrdev(DRIVER_MAJOR, DRIVER_NAME, &nfbi_fops)) {
		printk(KERN_ERR DRIVER_NAME": unable to get major %d\n", DRIVER_MAJOR);
		kfree(dev_priv);
		return -EIO;
	}
    
    //nfbi_hw_reset(2); //for Pana's Reset-line modification (from pull-up to pull-down)

#ifdef RTL89xxC_FAMILY
    nfbi_hw_reset(2); //for Pana's Reset-line modification (from pull-up to pull-down)
#endif
    
#ifdef SIMULATION
    init_waitqueue_head(&sim_wq);
    init_waitqueue_head(&sim_rq);
    init_MUTEX(&sim_sem);
    
	struct proc_dir_entry *res;
    res = create_proc_entry("nfbi_flag", 0, NULL);
    if (res) {
   	    res->read_proc = read_proc;
   	    res->write_proc = write_proc;
	}
	else {
		printk(KERN_ERR DRIVER_NAME": unable to create /proc/nfbi_flag\n");
		return -1;
	}
#else
#ifndef HOST_IS_PANABOARD
    //register interrupt handler
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
	if (request_irq(irq, nfbi_interrupt, IRQF_DISABLED, DRIVER_NAME, (void *)dev_priv))
	{
		printk(KERN_ERR DRIVER_NAME": IRQ %d is not free.\n", irq);
		kfree(dev_priv);
		return -1;
	}
#else

	//if (request_irq(irq, nfbi_interrupt, SA_INTERRUPT, DRIVER_NAME, (void *)dev_priv)) {
	if (request_irq(irq, nfbi_interrupt, 0, DRIVER_NAME, (void *)dev_priv)) {	    
		printk(KERN_ERR DRIVER_NAME": IRQ %d is not free.\n", irq);
		kfree(dev_priv);
		return -1;
	}
#endif
	printk("NFBI request_irq success!\n");

#ifndef RTL89xxC_FAMILY
    //================================================================
    // RTL8651C GPIO B7 is connected with MDIO_INT pin.
    // configure GPIO pin as input for hardware reset
    // @Pana_TBD
    REG32(PABCDCNR_REG) = REG32(PABCDCNR_REG) & (~0x8000); //B7, 0=Configured as GPIO pin
    REG32(PABCDDIR_REG) = REG32(PABCDDIR_REG) & (~0x8000); //B7, 0=input, 1=output, Configured as input pin
    REG32(PABIMR_REG) = (0x01 <<30); //set B7 falling interrupt(bit31,30), 0x00 diable, 0x01 falling, 0x02 rasing, 0x03 both
    REG32(PABCDISR_REG) = REG32(PABCDISR_REG) | 0x8000;    //write 1 to clear the interrupt at B7  (bit 15)

	//REG32(IRR2_REG) = (REG32(IRR2_REG) &~(0x0f<<NFBI_IRR_OFFSET)) | (NFBI_IRR_NO<<NFBI_IRR_OFFSET);
#endif
#endif
#endif

    /*================================================================*/
#ifndef HOST_IS_PANABOARD
#ifdef RTL8651C_FAMILY
	// RTL8651C GPIO B6 is connected with RESETn pin.
	// configure GPIO pin as output for hardware reset
	// @Pana_TBD
	REG32(PABCDCNR_REG) = REG32(PABCDCNR_REG) & (~0x4000); //B6, 0=Configured as GPIO pin
	REG32(PABCDDIR_REG) = REG32(PABCDDIR_REG) | 0x4000;    //B6, 0=input, 1=output, Configured as output pin
#elif defined (RTL89xxC_FAMILY)
	// RTL89xxC GPIO E2 is connected with RESETn pin.
	// configure GPIO pin as output for hardware reset
	REG32(PEFGHCNR_REG) = REG32(PEFGHCNR_REG) & (~0x4); //E2, 0=Configured as GPIO pin
	REG32(PEFGHDIR_REG) = REG32(PEFGHDIR_REG) | 0x4;    //E2, 0=input, 1=output, Configured as output pin
#endif
#endif

    // set the pre-selected phy address for MDIO address or probe the correct one
#if 0
	mdio_phyaddr = DEFAULT_MDIO_PHYAD;
	dev_priv->ready = 1;
#else
	mdio_phyaddr = nfbi_probephyaddr();
#endif
    /*================================================================*/
    //rtl_mdio_write(NFBI_REG_CMD, 0x0004); //configure interrupt level as high trigger
    if (dev_priv->cmd_handshake_polling)
        rtl_mdio_write(NFBI_REG_IMR, 0x0000);
    else
        rtl_mdio_write(NFBI_REG_IMR, 0x0006); //PREVMSG_FETCH & NEWMSG_COMING

	printk(KERN_INFO DRIVER_NAME" driver "DRIVER_VER" at %X (Interrupt %d)\n", io, irq);
	return 0;
}

#ifndef HOST_IS_PANABOARD
module_init(nfbi_init);
module_exit(nfbi_exit);

MODULE_AUTHOR("Michael Lo");
MODULE_DESCRIPTION("Driver for RTL8197B NFBI");
MODULE_LICENSE("none-GPL");
EXPORT_NO_SYMBOLS;
#endif

