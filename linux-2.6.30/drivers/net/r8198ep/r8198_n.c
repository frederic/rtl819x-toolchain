/*
################################################################################
# 
# Copyright(c) Realtek Semiconductor Corp. All rights reserved.
# 
# This program is free software; you can redistribute it and/or modify it 
# under the terms of the GNU General Public License as published by the Free 
# Software Foundation; either version 2 of the License, or (at your option) 
# any later version.
# 
# This program is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
# more details.
# 
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 
# Temple Place - Suite 330, Boston, MA  02111-1307, USA.
# 
# The full GNU General Public License is included in this distribution in the
# file called LICENSE.
# 
################################################################################
*/

/*
 *  This product is covered by one or more of the following patents:
 *  US5,307,459, US5,434,872, US5,732,094, US6,570,884, US6,115,776, and US6,327,625.
 */

/*
 * This driver is modified from r8169.c in Linux kernel 2.6.18
 */

#include <linux/module.h>
#include <linux/version.h>
#include <linux/pci.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/if_vlan.h>
#include <linux/crc32.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/init.h>
#include <linux/rtnetlink.h>

#include <linux/proc_fs.h> //wei add
#include <linux/random.h> //wei add


#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
#include <linux/dma-mapping.h>
#include <linux/moduleparam.h>
#endif//LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>

#include "r8198.h"
#include "r8198_asf.h"








/* Maximum events (Rx packets, etc.) to handle at each interrupt. */
static const int max_interrupt_work = 20;

/* Maximum number of multicast addresses to filter (vs. Rx-all-multicast).
   The RTL chips use a 64 element hash table based on the Ethernet CRC. */
static const int multicast_filter_limit = 32;

#define _R(NAME,MAC,RCR,MASK, JumFrameSz) \
	{ .name = NAME, .mcfg = MAC, .RCR_Cfg = RCR, .RxConfigMask = MASK, .jumbo_frame_sz = JumFrameSz }


#undef _R

static struct pci_device_id rtl8198_pci_tbl[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_REALTEK,	0x8198), },	//wei add
	{0,},
};

MODULE_DEVICE_TABLE(pci, rtl8198_pci_tbl);

static int rx_copybreak = 200;
static int use_dac;
static struct {
	u32 msg_enable;
} debug = { -1 };

/* media options */
#define MAX_UNITS 8
static int speed[MAX_UNITS] = { -1, -1, -1, -1, -1, -1, -1, -1 };
static int duplex[MAX_UNITS] = { -1, -1, -1, -1, -1, -1, -1, -1 };
static int autoneg[MAX_UNITS] = { -1, -1, -1, -1, -1, -1, -1, -1 };

MODULE_AUTHOR("Realtek and the Linux r8168 crew <netdev@vger.kernel.org>");
MODULE_DESCRIPTION("RealTek RTL-8198 Slave PCIE Ethernet driver");

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
MODULE_PARM(speed, "1-" __MODULE_STRING(MAX_UNITS) "i");
MODULE_PARM(duplex, "1-" __MODULE_STRING(MAX_UNITS) "i");
MODULE_PARM(autoneg, "1-" __MODULE_STRING(MAX_UNITS) "i");
#else
static int num_speed = 0;
static int num_duplex = 0;
static int num_autoneg = 0;

module_param_array(speed, int, &num_speed, 0);
module_param_array(duplex, int, &num_duplex, 0);
module_param_array(autoneg, int, &num_autoneg, 0);
#endif

MODULE_PARM_DESC(speed, "force phy operation. Deprecated by ethtool (8).");
MODULE_PARM_DESC(duplex, "force phy operation. Deprecated by ethtool (8).");
MODULE_PARM_DESC(autoneg, "force phy operation. Deprecated by ethtool (8).");

module_param(rx_copybreak, int, 0);
MODULE_PARM_DESC(rx_copybreak, "Copy breakpoint for copy-only-tiny-frames");
module_param(use_dac, int, 0);
MODULE_PARM_DESC(use_dac, "Enable PCI DAC. Unsafe on 32 bit PCI slot.");

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
module_param_named(debug, debug.msg_enable, int, 0);
MODULE_PARM_DESC(debug, "Debug verbosity level (0=none, ..., 16=all)");
#endif//LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)

MODULE_LICENSE("GPL");

MODULE_VERSION(RTL8168_VERSION);

static void rtl8198_dsm(struct net_device *dev, int dev_state);

static void rtl8198_esd_timer(unsigned long __opaque);
static void rtl8198_link_timer(unsigned long __opaque);
static void rtl8198_tx_clear(struct rtl8198_private *tp);
static void rtl8198_rx_clear(struct rtl8198_private *tp);

static int rtl8198_open(struct net_device *dev);
static int rtl8198_start_xmit(struct sk_buff *skb, struct net_device *dev);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
static irqreturn_t rtl8198_interrupt(int irq, void *dev_instance, struct pt_regs *regs);
#else
static irqreturn_t rtl8198_interrupt(int irq, void *dev_instance);
#endif
static int rtl8198_init_ring(struct net_device *dev);
static void rtl8198_hw_start(struct net_device *dev);
static int rtl8198_close(struct net_device *dev);
static void rtl8198_set_rx_mode(struct net_device *dev);
static void rtl8198_tx_timeout(struct net_device *dev);
static struct net_device_stats *rtl8198_get_stats(struct net_device *dev);
//static int rtl8198_rx_interrupt(struct net_device *, struct rtl8198_private *, void __iomem *);
static int rtl8198_rx_interrupt(struct rtl8198_private *);
static int rtl8198_change_mtu(struct net_device *dev, int new_mtu);
static void rtl8198_down(struct net_device *dev);

static int rtl8198_set_mac_address(struct net_device *dev, void *p);
void rtl8198_rar_set(struct rtl8198_private *tp, uint8_t *addr, uint32_t index);
static void rtl8198_tx_desc_init(struct rtl8198_private *tp);
static void rtl8198_rx_desc_init(struct rtl8198_private *tp);

static void rtl8198_nic_reset(struct net_device *dev);

static void rtl8198_phy_power_up (struct net_device *dev);
static void rtl8198_phy_power_down (struct net_device *dev);
static int rtl8198_set_speed(struct net_device *dev, u8 autoneg,  u16 speed, u8 duplex);



//static void rtl8198_tx_interrupt(struct rtl8198_private *tp);
//static int rtl8198_rx_interrupt(struct rtl8198_private *tp);

#ifdef CONFIG_R8168_NAPI
static int rtl8198_poll(napi_ptr napi, napi_budget budget);
#endif

static u32 rtl8198_intr_mask = 0xffffffff;
// (SPE_DMA_ISR_SWINT |	SPE_DMA_ISR_TXDU |SPE_DMA_ISR_TXERR| SPE_DMA_ISR_TXOK |SPE_DMA_ISR_TXTMR | \
//											SPE_DMA_ISR_RXDU |SPE_DMA_ISR_RXERR| SPE_DMA_ISR_RXOK);
//--------------------------------------------------
//wei add


#define KERN_INFO "<0>"  //wei add
#define dprintf printk  //wei add
struct net_device *reNet;
unsigned int SPE_IntFlag =0;
int at_errcnt=0;

struct rtl8198_private *g_tp;

int rtl8198_hw_tx(void * buff, unsigned int length, 
			unsigned int fs, unsigned int ls, 
			unsigned int txinfo1, unsigned int txinfo2, unsigned int txinfo3,
			unsigned int offset);

int rtl8198_hw_rx(void** input, unsigned int* pLen,  
			unsigned int *prxinfo1, unsigned int *prxinfo2, unsigned int *prxinfo3, 
			unsigned int *offset, unsigned int *drop);


#include "boot98.h"
#include "mytest.c"

//static const u16 rtl8198_napi_event =RxOK | RxDescUnavail | RxFIFOOver | TxOK | TxErr;  //wei del

//#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,3)
#if (( LINUX_VERSION_CODE < KERNEL_VERSION(2,4,27) ) || \
     (( LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) ) && \
      ( LINUX_VERSION_CODE < KERNEL_VERSION(2,6,3) )))
/* copied from linux kernel 2.6.20 include/linux/netdev.h */
#define	NETDEV_ALIGN		32
#define	NETDEV_ALIGN_CONST	(NETDEV_ALIGN - 1)

static inline void *netdev_priv(struct net_device *dev)
{
	return (char *)dev + ((sizeof(struct net_device)
					+ NETDEV_ALIGN_CONST)
				& ~NETDEV_ALIGN_CONST);
}
#endif	//LINUX_VERSION_CODE < KERNEL_VERSION(2,6,3)


//#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,5)
#ifndef netif_msg_init
#define netif_msg_init _kc_netif_msg_init
/* copied from linux kernel 2.6.20 include/linux/netdevice.h */
static inline u32 netif_msg_init(int debug_value, int default_msg_enable_bits)
{
	/* use default */
	if (debug_value < 0 || debug_value >= (sizeof(u32) * 8))
		return default_msg_enable_bits;
	if (debug_value == 0)	/* no output */
		return 0;
	/* set low N bits */
	return (1 << debug_value) - 1;
}

#endif //LINUX_VERSION_CODE < KERNEL_VERSION(2,6,5)

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,22)
static inline void eth_copy_and_sum (struct sk_buff *dest,
				     const unsigned char *src,
				     int len, int base)
{
	memcpy (dest->data, src, len);
}
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,22)

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


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11)

/* copied from linux kernel 2.6.12.6 /include/linux/pm.h */
typedef int __bitwise pci_power_t;

/* copied from linux kernel 2.6.12.6 /include/linux/pci.h */
typedef u32 __bitwise pm_message_t;

#define PCI_D0	((pci_power_t __force) 0)
#define PCI_D1	((pci_power_t __force) 1)
#define PCI_D2	((pci_power_t __force) 2)
#define PCI_D3hot	((pci_power_t __force) 3)
#define PCI_D3cold	((pci_power_t __force) 4)
#define PCI_POWER_ERROR	((pci_power_t __force) -1)

/* copied from linux kernel 2.6.12.6 /drivers/pci/pci.c */
/**
 * pci_choose_state - Choose the power state of a PCI device
 * @dev: PCI device to be suspended
 * @state: target sleep state for the whole system. This is the value
 *	that is passed to suspend() function.
 *
 * Returns PCI power state suitable for given device and given system
 * message.
 */

pci_power_t pci_choose_state(struct pci_dev *dev, pm_message_t state)
{
	if (!pci_find_capability(dev, PCI_CAP_ID_PM))
		return PCI_D0;

	switch (state) {
	case 0: return PCI_D0;
	case 3: return PCI_D3hot;
	default:
		printk("They asked me for state %d\n", state);
//		BUG();
	}
	return PCI_D0;
}
#endif	//LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11)

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

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,7)
/* copied from linux kernel 2.6.20 include/linux/sched.h */
#ifndef __sched
#define __sched		__attribute__((__section__(".sched.text")))
#endif

/* copied from linux kernel 2.6.20 kernel/timer.c */
signed long __sched schedule_timeout_uninterruptible(signed long timeout)
{
	__set_current_state(TASK_UNINTERRUPTIBLE);
	return schedule_timeout(timeout);
}

/* copied from linux kernel 2.6.20 include/linux/mii.h */
#undef if_mii
#define if_mii _kc_if_mii
static inline struct mii_ioctl_data *if_mii(struct ifreq *rq)
{
	return (struct mii_ioctl_data *) &rq->ifr_ifru;
}
#endif	//LINUX_VERSION_CODE < KERNEL_VERSION(2,6,7)


//=================================================================
//=================================================================
#include <asm/cacheflush.h>
void spe_reboot()
{
	struct rtl8198_private *tp = netdev_priv(reNet);
	void __iomem *ioaddr = tp->mmio_addr;
	
	void (*jumpF)(void);
	jumpF = (void *)(0xbfc00000);
	
	#define GIMR_REG 0xb8003000
	#define REG32(reg) (*(volatile unsigned int *)(reg))	
	REG32(GIMR_REG)=0; // mask all interrupt	    
	//cli();

	//flush_cache(); 
	flush_cache_all();

	
	printk("\nreboot.......\n");
#if defined(RTL865X) || defined(RTL8196B) || defined(RTL8198)
	/* this is to enable 865xc watch dog reset */
//	*(volatile unsigned long *)(0xB800311c)=0; 
//	for(;;);
#endif
	/* reboot */
	jumpF();	
	for(;;);
}



//---------------------------------------------------------------------------
int spe_proc_read_procmem(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    int i, j, len = 0;
    int limit = count - 80; /* Don't print more than this */
	
        len += sprintf(buf+len,"\nDevice  \n" );
	printk("READ PROC\n");
    return len;

}
//----------------------------------------------------------------------------------
//wei entry
int spe_proc_write_procmem(struct file *file, const char *buf, unsigned long count, void *data)
{

	unsigned char tmp[200];

	struct net_device *dev=reNet;
	struct rtl8198_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
	

	int		argc ;
	char**		argv ;
	
	if (count < 2 || count > 200)
		return -EFAULT;

	memset(tmp, '\0', 200);
	if (buf && !copy_from_user(tmp, buf, count)) 
	{
		tmp[count-1]=0;

		argc =GetArgc( (const char *)tmp );
		argv =GetArgv( (const char *)tmp );;


		if(!strcmp(argv[0], "dw"))		{	CMD_dw(argc-1, argv+1);	}
		else if(!strcmp(argv[0], "ew"))	{	CMD_ew(argc-1, argv+1);	}
		else if(!strcmp(argv[0], "db"))	{	CMD_db(argc-1, argv+1);	}		
		else if(!strcmp(argv[0], "regr"))	{	SlvPCIe_RegRead(argc-1, argv+1);		}
		else if(!strcmp(argv[0], "regw"))	{	SlvPCIe_RegWrite(argc-1, argv+1);		}
		else if(!strcmp(argv[0], "dump"))	{	CMD_Dump(argc-1, argv+1);		}	
		else if(!strcmp(argv[0], "varr"))	{	CMD_VarRead(argc-1, argv+1);		}			
		else if(!strcmp(argv[0], "varw"))	{	CMD_VarWrite(argc-1, argv+1);		}	
		else if(!strcmp(argv[0], "tx"))		{	CMD_DMATx(argc-1, argv+1);		}	
		else if(!strcmp(argv[0], "rx"))		{	CMD_DMARx(argc-1, argv+1);		}	
		else if(!strcmp(argv[0], "pktgen"))	{	CMD_PktGen(argc-1, argv+1);		}
		else if(!strcmp(argv[0], "enirq"))	{	CMD_ENIRQ(argc-1, argv+1);		}	
		else if(!strcmp(argv[0], "dmaloop"))	{	CMD_DMALoop(argc-1, argv+1);	}	
		else if(!strcmp(argv[0], "start"))	{	netif_start_queue(dev);	 }	
		else if(!strcmp(argv[0], "stop"))	{	netif_stop_queue(dev);	}			
		else if(!strcmp(argv[0], "reboot"))	{	spe_reboot();		}			
#ifdef CONFIG_R8198EP_HOST
		else if(!strcmp(argv[0], "ismr"))	{	CMD_IsmRead(argc-1, argv+1);		}
		else if(!strcmp(argv[0], "ismw"))	{	CMD_IsmWrite(argc-1, argv+1);	}	
		else if(!strcmp(argv[0], "drst"))		{	CMD_Drst(argc-1, argv+1);			}		
		//------------------------------
		else if(!strcmp(argv[0], "tram"))	{	CMD_TESTRAM(argc-1, argv+1);	}
		else if(!strcmp(argv[0], "sendj"))	{	CMD_SetJumpCode(argc-1, argv+1);	}
		else if(!strcmp(argv[0], "sendf"))		{		CMD_SendF(argc-1, argv+1);	}
#endif	
		else
		{	//printk("=>%s, count=%d\n",(char *) tmp, (int)count);
			//printk("=>argc=%d, argv[0]=%s\n",argc, argv[0]);
			printk("p cmd \n");
			printk("cmd: dw,ew,db,regr,regw,dump,varr,varw,tx,rx,pktgen,enirq,dmaloop \n");
			
#ifdef CONFIG_R8198EP_HOST		
			printk("cmd: ismr, ismw, drst \n");			
			printk("cmd: tram, sendj, sendf \n");	
#endif			
		}		
	}
	//printk("WRITE PROC\n");
    return count;

}




//===================================================================
static void 
mdio_write(void __iomem *ioaddr, 	   int RegAddr, 	   int value)
{
	int i;
#if 0
	RTL_W32(PHYAR, PHYAR_Write | 
		(RegAddr & PHYAR_Reg_Mask) << PHYAR_Reg_shift | 
		(value & PHYAR_Data_Mask));

	for (i = 0; i < 10; i++) {
		/* Check if the RTL8168 has completed writing to the specified MII register */
		if (!(RTL_R32(PHYAR) & PHYAR_Flag)) 
			break;
		udelay(100);
	}
#endif	
}

static int 
mdio_read(void __iomem *ioaddr,	  int RegAddr)
{
	int i, value = -1;
#if 0
	RTL_W32(PHYAR, 
		PHYAR_Read | (RegAddr & PHYAR_Reg_Mask) << PHYAR_Reg_shift);

	for (i = 0; i < 10; i++) {
		/* Check if the RTL8168 has completed retrieving data from the specified MII register */
		if (RTL_R32(PHYAR) & PHYAR_Flag) {
			value = (int) (RTL_R32(PHYAR) & PHYAR_Data_Mask);
			break;
		}
		udelay(100);
	}
#endif	
	return value;
}



//=================================================================


//-----------------------------------------------------------
static void 
rtl8198_irq_mask_and_ack(void __iomem *ioaddr)
{
	REG32_W(SPE_DMA_IMR, 0x0000);
}
//-----------------------------------------------------------
static void 
rtl8198_asic_down(struct net_device *dev)
{
	struct rtl8198_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;

	rtl8198_nic_reset(dev);
	rtl8198_irq_mask_and_ack(ioaddr);
}
//------------------------------------------------------------
static void 
rtl8198_nic_reset(struct net_device *dev)
{
	struct rtl8198_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
	int i;
#if 0 //#ifdef CONFIG_R8198EP_DEVICE // 0 //DMA_RST
	
	/* Soft reset the chip. */
	REG32_W(SPE_DMA_IOCMD, REG32_R(SPE_DMA_IOCMD)|SPE_DMA_IOCMD_RST);
	udelay(10);
	
	/* Check that the chip has finished the reset. */
	for (i = 1000; i > 0; i--) 
	{
		if ((REG32_R(SPE_DMA_IOCMD) & SPE_DMA_IOCMD_RST) == 0)
			break;
		udelay(10);
	}
#endif	
}
//------------------------------------------------------------
static unsigned int 
rtl8198_xmii_reset_pending(struct net_device *dev)
{
	struct rtl8198_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;

	unsigned long flags;
	unsigned int retval;
#if 0
	spin_lock_irqsave(&tp->phy_lock, flags);
	mdio_write(ioaddr, 0x1f, 0x0000);
	retval = mdio_read(ioaddr, MII_BMCR) & BMCR_RESET;
	spin_unlock_irqrestore(&tp->phy_lock, flags);
#endif
	return retval;
}

static unsigned int 
rtl8198_xmii_link_ok(struct net_device *dev)
{
	struct rtl8198_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
#if 0 //wei del
	mdio_write(ioaddr, 0x1f, 0x0000);  

	return RTL_R8(PHYstatus) & LinkStatus;
#endif	
}

static void 
rtl8198_xmii_reset_enable(struct net_device *dev)
{
	struct rtl8198_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
	unsigned long flags;
	int i;
#if 0
	spin_lock_irqsave(&tp->phy_lock, flags);
	mdio_write(ioaddr, 0x1f, 0x0000);
	mdio_write(ioaddr, MII_BMCR, mdio_read(ioaddr, MII_BMCR) | BMCR_RESET);

	for(i = 0; i < 2500; i++) {
		if(!(mdio_read(ioaddr, MII_BMSR) & BMCR_RESET))
			return;

		mdelay(1);
	}
	spin_unlock_irqrestore(&tp->phy_lock, flags);
#endif	
}
//----------------------------------------------------------------------
static void 
rtl8198_check_link_status(struct net_device *dev,
			  struct rtl8198_private *tp, 
			  void __iomem *ioaddr)
{
	unsigned long flags;

	spin_lock_irqsave(&tp->lock, flags);
	if (tp->link_ok(dev)) 
	{
		netif_carrier_on(dev);
		if (netif_msg_ifup(tp))
			printk(KERN_INFO PFX "%s: link up\n", dev->name);
	} else 
	{
		if (netif_msg_ifdown(tp))
			printk(KERN_INFO PFX "%s: link down\n", dev->name);
		netif_carrier_off(dev);
	}
	spin_unlock_irqrestore(&tp->lock, flags);
}

//----------------------------------------------------------------------
static void
rtl8198_powerdown_pll(struct net_device *dev)
{
#if 0 //wei add
	struct rtl8198_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;

	if (tp->wol_enabled == WOL_ENABLED)
		return;

	rtl8198_phy_power_down(dev);

	switch (tp->mcfg) {
	case CFG_METHOD_9:
	case CFG_METHOD_10:
		RTL_W8(PMCH, RTL_R8(PMCH) & ~BIT_7);
		break;
	}
#endif	
}

static void
rtl8198_powerup_pll(struct net_device *dev)
{
#if 0
	struct rtl8198_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;

	switch (tp->mcfg) {
	case CFG_METHOD_9:
	case CFG_METHOD_10:
		RTL_W8(PMCH, RTL_R8(PMCH) | BIT_7);
		break;
	}

	rtl8198_phy_power_up(dev);
	rtl8198_set_speed(dev, tp->autoneg, tp->speed, tp->duplex);
#endif	
}

static int 
rtl8198_set_speed_xmii(struct net_device *dev,
		       u8 autoneg, 
		       u16 speed, 
		       u8 duplex)
{
	struct rtl8198_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
	int auto_nego = 0;
	int giga_ctrl = 0;
	int bmcr_true_force = 0;
	unsigned long flags;

	if ((speed != SPEED_1000) && 
	    (speed != SPEED_100) && 
	    (speed != SPEED_10)) {
		speed = SPEED_1000;
		duplex = DUPLEX_FULL;
	}

	if ((autoneg == AUTONEG_ENABLE) || (speed == SPEED_1000)) {
		/*n-way force*/
		if ((speed == SPEED_10) && (duplex == DUPLEX_HALF)) {
			auto_nego |= ADVERTISE_10HALF;
		} else if ((speed == SPEED_10) && (duplex == DUPLEX_FULL)) {
			auto_nego |= ADVERTISE_10HALF |
				     ADVERTISE_10FULL;
		} else if ((speed == SPEED_100) && (duplex == DUPLEX_HALF)) {
			auto_nego |= ADVERTISE_100HALF | 
				     ADVERTISE_10HALF | 
				     ADVERTISE_10FULL;
		} else if ((speed == SPEED_100) && (duplex == DUPLEX_FULL)) {
			auto_nego |= ADVERTISE_100HALF | 
				     ADVERTISE_100FULL |
				     ADVERTISE_10HALF | 
				     ADVERTISE_10FULL;
		} else if (speed == SPEED_1000) {
			giga_ctrl |= ADVERTISE_1000HALF | 
				     ADVERTISE_1000FULL;

			auto_nego |= ADVERTISE_100HALF | 
				     ADVERTISE_100FULL |
				     ADVERTISE_10HALF | 
				     ADVERTISE_10FULL;
		}

		//disable flow contorol
		auto_nego &= ~ADVERTISE_PAUSE_CAP;
		auto_nego &= ~ADVERTISE_PAUSE_ASYM;

		tp->phy_auto_nego_reg = auto_nego;
		tp->phy_1000_ctrl_reg = giga_ctrl;

		tp->autoneg = autoneg;
		tp->speed = speed; 
		tp->duplex = duplex; 

		rtl8198_phy_power_up (dev);

		spin_lock_irqsave(&tp->phy_lock, flags);
		mdio_write(ioaddr, 0x1f, 0x0000);
		mdio_write(ioaddr, MII_ADVERTISE, auto_nego);
		mdio_write(ioaddr, MII_CTRL1000, giga_ctrl);
		mdio_write(ioaddr, MII_BMCR, BMCR_RESET | BMCR_ANENABLE | BMCR_ANRESTART);
		spin_unlock_irqrestore(&tp->phy_lock, flags);
	} else {
		/*true force*/
#ifndef BMCR_SPEED100
#define BMCR_SPEED100	0x0040
#endif

#ifndef BMCR_SPEED10
#define BMCR_SPEED10	0x0000
#endif
		if ((speed == SPEED_10) && (duplex == DUPLEX_HALF)) {
			bmcr_true_force = BMCR_SPEED10;
		} else if ((speed == SPEED_10) && (duplex == DUPLEX_FULL)) {
			bmcr_true_force = BMCR_SPEED10 | BMCR_FULLDPLX;
		} else if ((speed == SPEED_100) && (duplex == DUPLEX_HALF)) {
			bmcr_true_force = BMCR_SPEED100;
		} else if ((speed == SPEED_100) && (duplex == DUPLEX_FULL)) {
			bmcr_true_force = BMCR_SPEED100 | BMCR_FULLDPLX;
		}

		spin_lock_irqsave(&tp->phy_lock, flags);
		mdio_write(ioaddr, 0x1f, 0x0000);
		mdio_write(ioaddr, MII_BMCR, bmcr_true_force);
		spin_unlock_irqrestore(&tp->phy_lock, flags);
	}

	return 0;
}

static int 
rtl8198_set_speed(struct net_device *dev,
		  u8 autoneg, 
		  u16 speed, 
		  u8 duplex)
{
	struct rtl8198_private *tp = netdev_priv(dev);
	int ret;

	ret = tp->set_speed(dev, autoneg, speed, duplex);

	return ret;
}
#if 0 //wei del
static struct ethtool_ops rtl8198_ethtool_ops = {
	.get_drvinfo		= rtl8198_get_drvinfo,
	.get_regs_len		= rtl8198_get_regs_len,
	.get_link		= ethtool_op_get_link,
	.get_settings		= rtl8198_get_settings,
	.set_settings		= rtl8198_set_settings,
	.get_msglevel		= rtl8198_get_msglevel,
	.set_msglevel		= rtl8198_set_msglevel,
	.get_rx_csum		= rtl8198_get_rx_csum,
	.set_rx_csum		= rtl8198_set_rx_csum,
	.get_tx_csum		= rtl8198_get_tx_csum,
	.set_tx_csum		= rtl8198_set_tx_csum,
	.get_sg			= ethtool_op_get_sg,
	.set_sg			= ethtool_op_set_sg,
#ifdef NETIF_F_TSO
	.get_tso		= ethtool_op_get_tso,
	.set_tso		= ethtool_op_set_tso,
#endif
	.get_regs		= rtl8198_get_regs,
	.get_wol		= rtl8198_get_wol,
	.set_wol		= rtl8198_set_wol,
	.get_strings		= rtl8198_get_strings,
	.get_stats_count	= rtl8198_get_stats_count,
	.get_ethtool_stats	= rtl8198_get_ethtool_stats,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,23)
#ifdef ETHTOOL_GPERMADDR 
	.get_perm_addr		= ethtool_op_get_perm_addr,
#endif
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(2,6,23)
};
#endif

//----------------------------------------------------------------------

//----------------------------------------------------------------------

//----------------------------------------------------------------------
static inline void rtl8198_delete_esd_timer(struct net_device *dev, struct timer_list *timer)
{
	struct rtl8198_private *tp = netdev_priv(dev);

	spin_lock_irq(&tp->lock);
	del_timer_sync(timer);
	spin_unlock_irq(&tp->lock);
}
//----------------------------------------------------------------------

//----------------------------------------------------------------------
static inline void rtl8198_delete_link_timer(struct net_device *dev, struct timer_list *timer)
{
	struct rtl8198_private *tp = netdev_priv(dev);

	spin_lock_irq(&tp->lock);
	del_timer_sync(timer);
	spin_unlock_irq(&tp->lock);
}
//----------------------------------------------------------------------
static inline void rtl8198_request_link_timer(struct net_device *dev)
{
	struct rtl8198_private *tp = netdev_priv(dev);
	struct timer_list *timer = &tp->link_timer;

	init_timer(timer);
	timer->expires = jiffies + RTL8168_LINK_TIMEOUT;
	timer->data = (unsigned long)(dev);
	timer->function = rtl8198_link_timer;
	add_timer(timer);
}

#ifdef CONFIG_NET_POLL_CONTROLLER
/*
 * Polling 'interrupt' - used by things like netconsole to send skbs
 * without having to re-enable interrupts. It's not called while
 * the interrupt routine is executing.
 */
static void 
rtl8198_netpoll(struct net_device *dev)
{
	struct rtl8198_private *tp = netdev_priv(dev);
	struct pci_dev *pdev = tp->pci_dev;

	disable_irq(pdev->irq);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
	rtl8198_interrupt(pdev->irq, dev, NULL);
#else
	rtl8198_interrupt(pdev->irq, dev);
#endif
	enable_irq(pdev->irq);
}
#endif
//-----------------------------------------------------------------------
static void 
rtl8198_release_board(struct pci_dev *pdev, 
		      struct net_device *dev,
		      void __iomem *ioaddr)
{
#ifdef CONFIG_R8198EP_HOST
	iounmap(ioaddr);
	pci_release_regions(pdev);
	pci_disable_device(pdev);
#endif	
	free_netdev(dev);
}
//-----------------------------------------------------------------------
/**
 * rtl8198_set_mac_address - Change the Ethernet Address of the NIC
 * @dev: network interface device structure 
 * @p:   pointer to an address structure
 *
 * Return 0 on success, negative on failure
 **/
static int
rtl8198_set_mac_address(struct net_device *dev, 
			void *p)
{
	struct rtl8198_private *tp = netdev_priv(dev);
	struct sockaddr *addr = p;

	printk("set mac address \n"); //wei add

	if (!is_valid_ether_addr(addr->sa_data))
		return -EADDRNOTAVAIL;

	memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);
	memcpy(tp->mac_addr, addr->sa_data, dev->addr_len);

	rtl8198_rar_set(tp, tp->mac_addr, 0);
	
	return 0;
}

/******************************************************************************
 * rtl8198_rar_set - Puts an ethernet address into a receive address register.
 *
 * tp - The private data structure for driver
 * addr - Address to put into receive address register
 * index - Receive address register to write
 *****************************************************************************/
void
rtl8198_rar_set(struct rtl8198_private *tp, 
		uint8_t *addr, 
		uint32_t index)
{
	void __iomem *ioaddr = tp->mmio_addr;
	uint32_t rar_low = 0;
	uint32_t rar_high = 0;

	rar_low = ((uint32_t) addr[0] |
		  ((uint32_t) addr[1] << 8) |
		  ((uint32_t) addr[2] << 16) | 
		  ((uint32_t) addr[3] << 24));

	rar_high = ((uint32_t) addr[4] | 
		   ((uint32_t) addr[5] << 8));
/*  //wei del
	RTL_W8(Cfg9346, Cfg9346_Unlock);
	RTL_W32(MAC0, rar_low);
	RTL_W32(MAC4, rar_high);
	RTL_W8(Cfg9346, Cfg9346_Lock);
*/	
}



static int 
rtl8198_do_ioctl(struct net_device *dev, 
		 struct ifreq *ifr, 
		 int cmd)
{
	struct rtl8198_private *tp = netdev_priv(dev);
	struct mii_ioctl_data *data = if_mii(ifr);
	unsigned long flags;
	void __iomem *ioaddr = tp->mmio_addr;

	printk("do ioctl \n");

	if (!netif_running(dev))
		return -ENODEV;
#if 0
	switch (cmd) {
	case SIOCGMIIPHY:
		data->phy_id = 32; /* Internal PHY */
		return 0;

	case SIOCGMIIREG:
		spin_lock_irqsave(&tp->phy_lock, flags);
		mdio_write(ioaddr, 0x1F, 0x0000);
		data->val_out = mdio_read(tp->mmio_addr, data->reg_num & 0x1f);
		spin_unlock_irqrestore(&tp->phy_lock, flags);
		return 0;

	case SIOCSMIIREG:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		spin_lock_irqsave(&tp->phy_lock, flags);
		mdio_write(ioaddr, 0x1F, 0x0000);
		mdio_write(tp->mmio_addr, data->reg_num & 0x1f, data->val_in);
		spin_unlock_irqrestore(&tp->phy_lock, flags);
		return 0;

	case SIOCDEVPRIVATE_RTLASF:
		return rtl8198_asf_ioctl(dev, ifr);
	default:
		return -EOPNOTSUPP;		
	}
#endif
	return -EOPNOTSUPP;
}

static void
rtl8198_phy_power_up (struct net_device *dev)
{
	struct rtl8198_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
	unsigned long flags;

	spin_lock_irqsave(&tp->phy_lock, flags);
	mdio_write(ioaddr, 0x1F, 0x0000);
	mdio_write(ioaddr, 0x0E, 0x0000);
	mdio_write(ioaddr, MII_BMCR, BMCR_ANENABLE);
	spin_unlock_irqrestore(&tp->phy_lock, flags);
}

static void
rtl8198_phy_power_down (struct net_device *dev)
{
	struct rtl8198_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
	unsigned long flags;

	spin_lock_irqsave(&tp->phy_lock, flags);
	mdio_write(ioaddr, 0x1F, 0x0000);
	mdio_write(ioaddr, 0x0E, 0x0200);
	mdio_write(ioaddr, MII_BMCR, BMCR_PDOWN);
	spin_unlock_irqrestore(&tp->phy_lock, flags);
}
//------------------------------------------------------------------
static int __devinit
rtl8198_init_board(struct pci_dev *pdev,  struct net_device **dev_out,   void __iomem **ioaddr_out)
{
	void __iomem *ioaddr;
	struct net_device *dev;
	struct rtl8198_private *tp;
	int rc = -ENOMEM, i, acpi_idle_state = 0, pm_cap;

	assert(ioaddr_out != NULL);

	/* dev zeroed in alloc_etherdev */
	dev = alloc_etherdev(sizeof (*tp));
	if (dev == NULL) 
	{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
		if (netif_msg_drv(&debug))
#ifdef CONFIG_R8198EP_HOST			
		{	dev_err(&pdev->dev, "unable to alloc new ethernet\n"); }
#endif
#ifdef CONFIG_R8198EP_DEVICE
		{	printk("unable to alloc new ethernet\n"); }
#endif
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
		goto err_out;
	}

	SET_MODULE_OWNER(dev);
#ifdef CONFIG_R8198EP_HOST	
	SET_NETDEV_DEV(dev, &pdev->dev);
#endif
	tp = netdev_priv(dev);
	tp->dev = dev;
	tp->msg_enable = netif_msg_init(debug.msg_enable, R8168_MSG_DEFAULT);

#ifdef CONFIG_R8198EP_HOST

	/* enable device (incl. PCI PM wakeup and hotplug setup) */
	rc = pci_enable_device(pdev);
	if (rc < 0) 
	{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
		if (netif_msg_probe(tp))
			dev_err(&pdev->dev, "enable failure\n");
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
		goto err_out_free_dev;
	}

	rc = pci_set_mwi(pdev);
	if (rc < 0)
		goto err_out_disable;


	dump_config(pdev, 0);   //wei add
	//-----------------------------------------------------
	/* save power state before pci_enable_device overwrites it */
	pm_cap = pci_find_capability(pdev, PCI_CAP_ID_PM);
	if (pm_cap) 
	{
		u16 pwr_command;
		pci_read_config_word(pdev, pm_cap + PCI_PM_CTRL, &pwr_command);
		acpi_idle_state = pwr_command & PCI_PM_CTRL_STATE_MASK;
	} 
	else 
	{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
		if (netif_msg_probe(tp)) 
		{
			dev_err(&pdev->dev, "PowerManagement capability not found.\n");
		}
#else
		printk("PowerManagement capability not found.\n");
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)

	}
	//-----------------------------------------------------
	/* make sure PCI base addr 1 is MMIO */
	if (!(pci_resource_flags(pdev, 1) & IORESOURCE_MEM)) 
	{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
		if (netif_msg_probe(tp))
			dev_err(&pdev->dev, "region #1 not an MMIO resource, aborting\n");
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
		rc = -ENODEV;
		goto err_out_mwi;
	}
	//-----------------------------------------------------
	/* check for weird/broken PCI region reporting */
	if (pci_resource_len(pdev, 1) < R8168_REGS_SIZE) 
	{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
		if (netif_msg_probe(tp))
			dev_err(&pdev->dev, "Invalid PCI region size(s), aborting\n");
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
		rc = -ENODEV;
		goto err_out_mwi;
	}
	//-----------------------------------------------------
	rc = pci_request_regions(pdev, MODULENAME);
	if (rc < 0) 
	{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
		if (netif_msg_probe(tp))
			dev_err(&pdev->dev, "could not request regions.\n");
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
		goto err_out_mwi;
	}
	//-----------------------------------------------------
	
	if ((sizeof(dma_addr_t) > 4) &&
	    !pci_set_dma_mask(pdev, DMA_64BIT_MASK) && use_dac) 
	{
		//dev->features |= NETIF_F_HIGHDMA;	//wei del
		printk("DMA MAP 64BIT \n");
	} 
	else 
	{
		rc = pci_set_dma_mask(pdev, DMA_32BIT_MASK);
		if (rc < 0) 
		{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
			if (netif_msg_probe(tp))
				dev_err(&pdev->dev, "DMA configuration failed.\n");
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
			goto err_out_free_res;
		}
		printk("DMA MAP 32BIT \n");
	}

	pci_set_master(pdev);
	//-----------------------------------------------------	
	/* ioremap MMIO region */
	ioaddr = ioremap(pci_resource_start(pdev, 1), R8168_REGS_SIZE);
	if (ioaddr == NULL) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
		if (netif_msg_probe(tp))
			dev_err(&pdev->dev, "cannot remap MMIO, aborting\n");
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
		rc = -EIO;
		goto err_out_free_res;
	}
	//printk("IOADDR=%08x \n", ioaddr);		//wei add
	//-----------------------------------------------------
#if 0	
	/* Identify chip attached to board */
	rtl8198_get_mac_version(tp, ioaddr);
	rtl8198_print_mac_version(tp);

	for (i = ARRAY_SIZE(rtl_chip_info) - 1; i >= 0; i--) 
	{
		if (tp->mcfg == rtl_chip_info[i].mcfg)
			break;
	}
		
	if (i < 0) 
	{
		/* Unknown chip: assume array element #0, original RTL-8168 */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
		if (netif_msg_probe(tp)) 
		{
			dev_printk(KERN_DEBUG, &pdev->dev, "unknown chip version, assuming %s\n", rtl_chip_info[0].name);
		}
#else
		printk("Realtek unknown chip version, assuming %s\n", rtl_chip_info[0].name);
#endif //LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
		i++;
	}
	
	tp->chipset = i;

#endif
	//DumpEPreg(tp, ioaddr);  //wei add
#endif

#ifdef CONFIG_R8198EP_DEVICE
	ioaddr=SPE_EP_CFG_BASE;
	rc=0;
#endif

	*ioaddr_out = ioaddr;
	*dev_out = dev;
out:
	return rc;
	
#ifdef CONFIG_R8198EP_HOST
err_out_free_res:
	pci_release_regions(pdev);

err_out_mwi:
	pci_clear_mwi(pdev);
err_out_disable:
	pci_disable_device(pdev);
#endif

err_out_free_dev:
	free_netdev(dev);
err_out:
	*ioaddr_out = NULL;
	*dev_out = NULL;
	goto out;
}
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
static void
rtl8198_link_timer(unsigned long __opaque)
{
	struct net_device *dev = (struct net_device *)__opaque;
	struct rtl8198_private *tp = netdev_priv(dev);
	struct timer_list *timer = &tp->link_timer;
	unsigned long timeout = RTL8168_LINK_TIMEOUT;

	if (tp->link_ok(dev) != tp->old_link_status)
		rtl8198_check_link_status(dev, tp, tp->mmio_addr);

	tp->old_link_status = tp->link_ok(dev);

	mod_timer(timer, jiffies + timeout);
}
//-------------------------------------------------------------------------
/* Cfg9346_Unlock assumed. */
#if 0
static unsigned rtl8198_try_msi(struct pci_dev *pdev, void __iomem *ioaddr)
{
	unsigned msi = 0;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,13)
	if (pci_enable_msi(pdev)) {
			dev_info(&pdev->dev, "no MSI. Back to INTx.\n");
	} else {
			msi |= RTL_FEATURE_MSI;
	}
#endif

	return msi;
}
//-------------------------------------------------------------------------
static void rtl8198_disable_msi(struct pci_dev *pdev, struct rtl8198_private *tp)
{
	if (tp->features & RTL_FEATURE_MSI) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,13)
		pci_disable_msi(pdev);
#endif
		tp->features &= ~RTL_FEATURE_MSI;
	}
}
//------------------------------------------------------------------

#endif

//#if 1//def HAVE_NET_DEVICE_OPS
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)   //meego
static const struct net_device_ops rtl8198_netdev_ops = {
	.ndo_open		= rtl8198_open,
	.ndo_stop		= rtl8198_close,
	.ndo_get_stats		= rtl8198_get_stats,
	.ndo_start_xmit		= rtl8198_start_xmit,
//	.ndo_tx_timeout		= rtl8198_tx_timeout,
	.ndo_change_mtu		= rtl8198_change_mtu,
	.ndo_set_mac_address	= rtl8198_set_mac_address,
//	.ndo_do_ioctl		= rtl8198_do_ioctl,
//	.ndo_set_multicast_list	= rtl8198_set_rx_mode,
#ifdef CONFIG_R8168_VLAN
//	.ndo_vlan_rx_register	= rtl8198_vlan_rx_register,
#endif
#ifdef CONFIG_NET_POLL_CONTROLLER
//	.ndo_poll_controller	= rtl8198_netpoll,
#endif
};
#endif //HAVE_NET_DEVICE_OPS


//-------------------------------------------------------------------
static int __devinit rtl8198_init_one(struct pci_dev *pdev, 	 const struct pci_device_id *ent)
{
	struct net_device *dev = NULL;
	struct rtl8198_private *tp;
	void __iomem *ioaddr = NULL;
	static int board_idx = -1;
	u8 autoneg, duplex;
	u16 speed;
	int i, rc;

	printk( "INIT ONE \n");

#ifdef CONFIG_R8198EP_HOST
	assert(pdev != NULL);
	assert(ent != NULL);
#endif
	board_idx++;

	if (netif_msg_drv(&debug)) 
	{
#ifdef CONFIG_R8198EP_HOST	
		printk( "%s Slave PCIe Host site ethernet driver %s loaded\n",       MODULENAME, RTL8168_VERSION);
#else
		printk( "%s Slave PCIe Device site ethernet driver %s loaded\n",       MODULENAME, RTL8168_VERSION);
#endif

	}

	rc = rtl8198_init_board(pdev, &dev, &ioaddr);
	if (rc)
		return rc;

	printk("Init board OK \n");



	tp = netdev_priv(dev);
	g_tp = tp;
	assert(ioaddr != NULL);

//	tp->set_speed = rtl8198_set_speed_xmii;
//	tp->get_settings = rtl8198_gset_xmii;
//	tp->phy_reset_enable = rtl8198_xmii_reset_enable;
//	tp->phy_reset_pending = rtl8198_xmii_reset_pending;
//	tp->link_ok = rtl8198_xmii_link_ok;

#if 0
	RTL_W8(Cfg9346, Cfg9346_Unlock);
	tp->features |= rtl8198_try_msi(pdev, ioaddr);
	RTL_W8(Cfg9346, Cfg9346_Lock);
#endif
	/* Get MAC address.  FIXME: read EEPROM */
	for (i = 0; i < MAC_ADDR_LEN; i++)
#ifdef CONFIG_R8198EP_HOST					
       {		dev->dev_addr[i] = 0x40+i;  }  //wei add 0x40-0x45  
#else					
       {		dev->dev_addr[i] = 0x00+i*2;  }  //wei add mac: 00:02:04:06:08:0a  
#endif

	
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,13)
	memcpy(dev->perm_addr, dev->dev_addr, dev->addr_len); //wei del
#endif
	memcpy(dev->dev_addr, dev->dev_addr, dev->addr_len);

//#if 1//def HAVE_NET_DEVICE_OPS
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)  //meego

	dev->netdev_ops = &rtl8198_netdev_ops;
#else
	dev->open = rtl8198_open;
	dev->hard_start_xmit = rtl8198_start_xmit;
	dev->get_stats = rtl8198_get_stats;
//	SET_ETHTOOL_OPS(dev, &rtl8198_ethtool_ops);		//wei del
	dev->stop = rtl8198_close;
	dev->tx_timeout = rtl8198_tx_timeout;
	//dev->set_multicast_list = rtl8198_set_rx_mode;
	dev->watchdog_timeo = RTL8198_TX_TIMEOUT;
	
//	dev->change_mtu = rtl8198_change_mtu;
	dev->set_mac_address = rtl8198_set_mac_address;
//	dev->do_ioctl = rtl8198_do_ioctl;	
	
#endif
	  
#ifdef CONFIG_R8198EP_HOST	
	dev->irq = pdev->irq;
#else
	dev->irq = SPE_EP_IRQ_NO;
#endif


	dev->base_addr = (unsigned long) ioaddr;




//	dev->features |= NETIF_F_IP_CSUM;	//wei del


	//tp->cp_cmd |= RxChkSum;
	//tp->cp_cmd |= RTL_R16(CPlusCmd);

	tp->intr_mask = rtl8198_intr_mask;
#ifdef CONFIG_R8198EP_HOST	
	tp->pci_dev = pdev;
#endif
	tp->mmio_addr = ioaddr;
//	tp->max_jumbo_frame_size = Jumbo_Frame_4k;

	spin_lock_init(&tp->lock);
	spin_lock_init(&tp->phy_lock);

	rc = register_netdev(dev);
	if (rc) 
	{	printk("net register fail \n");
		rtl8198_release_board(pdev, dev, ioaddr);
		return rc;
	}

	if (netif_msg_probe(tp)) 
	{
		printk(KERN_DEBUG "%s: Identified chip type is '%s'.\n",       dev->name, "RTL8198");
	}

#ifdef CONFIG_R8198EP_HOST
	pci_set_drvdata(pdev, dev);
#endif
	if (netif_msg_probe(tp)) 
	{
		printk(KERN_INFO "%s: %s at 0x%lx, "
		       "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x, "
		       "IRQ %d\n",
		       dev->name,
		       "8198",
		       dev->base_addr,
		       dev->dev_addr[0], dev->dev_addr[1],
		       dev->dev_addr[2], dev->dev_addr[3],
		       dev->dev_addr[4], dev->dev_addr[5], dev->irq);
	}
#ifdef CONFIG_R8198EP_HOST
	pci_write_config_byte(pdev, PCI_LATENCY_TIMER, 0x40); 
#endif


#ifdef CONFIG_R8198EP_HOST   //fix interrupt INTA patch
	u16 cmd;
	pci_read_config_word(pdev, PCI_COMMAND, &cmd);
	cmd &= (0xffff)-(1<<10);
	pci_write_config_word(pdev, PCI_COMMAND, cmd);
#endif
	//rtl8198_set_speed(dev, autoneg, speed, duplex);
	//create_proc_read_entry("spe",0,NULL,spe_proc_read_procmem,NULL);

	//wei add, for test
	struct proc_dir_entry *entry=create_proc_entry("spe", 0, NULL);
	if(entry)
	{
		entry->write_proc=spe_proc_write_procmem;
		entry->read_proc=spe_proc_read_procmem;
	}
	reNet=dev;
	return 0;
}
//---------------------------------------------------------------------------
static void __devexit
rtl8198_remove_one(struct pci_dev *pdev)
{
#ifdef CONFIG_R8198EP_HOST
	struct net_device *dev = pci_get_drvdata(pdev);
#endif
#ifdef CONFIG_R8198EP_DEVICE
	struct net_device *dev =reNet;
#endif	

	struct rtl8198_private *tp = netdev_priv(dev);

	assert(dev != NULL);
	assert(tp != NULL);

	flush_scheduled_work();

	unregister_netdev(dev);

//	rtl8198_disable_msi(pdev, tp);
	rtl8198_release_board(pdev, dev, tp->mmio_addr);
#ifdef CONFIG_R8198EP_HOST	
	pci_set_drvdata(pdev, NULL);
#endif		
	remove_proc_entry("spe", NULL);

}
//---------------------------------------------------------------------------
static void 
rtl8198_set_rxbufsize(struct rtl8198_private *tp,   struct net_device *dev)
{
	void __iomem *ioaddr = tp->mmio_addr;
	unsigned int mtu = dev->mtu;

	tp->rx_buf_sz = RX_BUF_SIZE;


}
//====================================================================
static int 
rtl8198_open(struct net_device *dev)
{
	struct rtl8198_private *tp = netdev_priv(dev);
#ifdef CONFIG_R8198EP_HOST	
	struct pci_dev *pdev = tp->pci_dev;
#endif
	int retval;

	printk("=>rtl8198_open \n");

	//-----------------------------------------
	//pre-setting
	rtl8198_set_rxbufsize(tp, dev);

	void __iomem *ioaddr = tp->mmio_addr;  //wei add
	rtl8198_irq_mask_and_ack(ioaddr);	//wei add, mask =0


	//----------------------------------------
	//alloc irq
	retval = request_irq(dev->irq, rtl8198_interrupt, (tp->features & RTL_FEATURE_MSI) ? 0 : SA_SHIRQ, dev->name, dev);
	
	if (retval < 0)
		goto out;

	retval = -ENOMEM;

#ifdef RX_TASKLET
                tasklet_init(&tp->rx_tasklet, rtl8198_rx_interrupt, (unsigned long)tp);
#endif

#ifdef TX_TASKLET
                tasklet_init(&tp->tx_tasklet, rtl8198_tx_interrupt, (unsigned long)tp);
#endif


	//----------------------------------------------
	//alloc desc
	/*
	 * Rx and Tx desscriptors needs 256 bytes alignment.
	 * pci_alloc_consistent provides more.
	 */
#ifdef CONFIG_R8198EP_HOST	 
	tp->TxDescArray = pci_alloc_consistent(pdev, R8168_TX_RING_BYTES,    &tp->TxPhyAddr);
#else // CONFIG_R8198EP_DEVICE
	//tp->TxDescArray=kmalloc(R8168_TX_RING_BYTES,GFP_ATOMIC);
	//tp->TxDescArray=(tp->TxDescArray);
	tp->TxDescArray= ((((unsigned int)(tp->TxDescBuff))+0x10)& 0xfffffff0)|UNCACHE_MASK;
	tp->TxPhyAddr=tp->TxDescArray;
#endif
	if (!tp->TxDescArray)
		goto err_free_irq;

	
#ifdef CONFIG_R8198EP_HOST	
	tp->RxDescArray = pci_alloc_consistent(pdev, R8168_RX_RING_BYTES,   &tp->RxPhyAddr);
#else //CONFIG_R8198EP_DEVICE
	//tp->RxDescArray=kmalloc(R8168_RX_RING_BYTES,GFP_ATOMIC);
	//tp->RxDescArray=(tp->RxDescArray);
	tp->RxDescArray= ((( (unsigned int)(tp->RxDescBuff))+0x10)& 0xfffffff0)|UNCACHE_MASK;
	tp->RxPhyAddr=tp->RxDescArray;
#endif

	if (!tp->RxDescArray)
		goto err_free_tx;



	printk("TxDescArray=%x, TxPhyAddr=%x \n", (int)tp->TxDescArray,(int) tp->TxPhyAddr);
	printk("RxDescArray=%x, RxPhyAddr=%x \n", (int)tp->RxDescArray, (int)tp->RxPhyAddr);

	tp->info1=0;
	tp->info2=0;
	tp->info3=0;
	tp->txoffset=0;
	//--------------------------------------
	//fill skb  ring
	retval = rtl8198_init_ring(dev);
	if (retval < 0)
		goto err_free_rx;
	//------------------------------------
	
#if 0
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
	INIT_WORK(&tp->task, NULL, dev);
#else
	INIT_DELAYED_WORK(&tp->task, NULL);
#endif
#endif
	//-------------------------------------
	
	tp->dmsg=0; //debug message
	
	rtl8198_hw_start(dev);


//	rtl8198_request_link_timer(dev);
//	rtl8198_dsm(dev, DSM_IF_UP);
//	rtl8198_check_link_status(dev, tp, tp->mmio_addr);

	retval = 0;	
out: return retval;


err_free_rx:
	printk("err_free_rx \n");
#ifdef CONFIG_R8198EP_HOST	
	pci_free_consistent(pdev, R8168_RX_RING_BYTES, tp->RxDescArray,	tp->RxPhyAddr);
#endif
#ifdef CONFIG_R8198EP_DEVICE
	kfree( tp->RxDescArray);
#endif

err_free_tx:
	printk("err_free_tx \n");

#ifdef CONFIG_R8198EP_HOST		
	pci_free_consistent(pdev, R8168_TX_RING_BYTES, tp->TxDescArray,	tp->TxPhyAddr);
#endif
#ifdef CONFIG_R8198EP_DEVICE
	kfree( tp->TxDescArray);
#endif
	
err_free_irq:
	free_irq(dev->irq, dev);
	goto out;

}
//---------------------------------------------------------------------------
static void 
rtl8198_hw_reset(struct net_device *dev)
{
	struct rtl8198_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;

	/* Disable interrupts */
	rtl8198_irq_mask_and_ack(ioaddr);

	rtl8198_nic_reset(dev);
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
static void rtl8198_hw_start(struct net_device *dev)
{
	struct rtl8198_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
#ifdef CONFIG_R8198EP_HOST	
	struct pci_dev *pdev = tp->pci_dev;
#endif
	u8 device_control, options1, options2;
	u16 ephy_data;
	u32 csi_tmp;

	rtl8198_nic_reset(dev);
	
#ifdef CONFIG_R8198EP_HOST	
	REG32_W(SPE_DMA_IOCMD, REG32_R(SPE_DMA_IOCMD) | SPE_DMA_IOCMD_DSECSWAP );	
	REG32_W(SPE_NFBI_CMD, REG32_R(SPE_NFBI_CMD) | NFBI_CMD_SWAP );
#endif	

#ifdef CONFIG_R8198EP_DEVICE
	//init ISM	
	tp->ISM_len= RX_BUF_SIZE;	
	REG32_W(SPE_ISM_LR,tp->ISM_len);
	tp->ISM_buff= (((int)&(tp->ISM_temp[0]))&0xfffffffc) +4;
	REG32_W(SPE_ISM_BAR,tp->ISM_buff);

#endif

	//-------------------------
	//fill desc
	REG32_W(SPE_DMA_TXFDP, tp->TxPhyAddr);
	REG32_W(SPE_DMA_RXFDP, tp->RxPhyAddr);

	//	REG32_W(SPE_DMA_TXFDP, cpu_to_le32(tp->TxDescArray) );
	//	REG32_W(SPE_DMA_RXFDP, cpu_to_le32(tp->RxDescArray) );	

	//------------------------
	/* Set Rx Config register */
	//rtl8198_set_rx_mode(dev);

	/* Clear the interrupt status register. */
	//REG32_W(SPE_DMA_ISR, 0xFFFFFFFF);

	if (tp->rx_fifo_overflow == 0) 
	{
		/* Enable all known interrupts by setting the interrupt mask. */
		//REG32_W(SPE_DMA_IMR, rtl8198_intr_mask);
		REG32_W(SPE_DMA_IMR, 0xFFFFFFFF);
		netif_start_queue(dev);
	}


#if 0 //def CONFIG_R8198EP_HOST
	if (!tp->pci_cfg_is_read) 
	{
		pci_read_config_byte(pdev, PCI_COMMAND, &tp->pci_cfg_space.cmd);
		pci_read_config_byte(pdev, PCI_CACHE_LINE_SIZE, &tp->pci_cfg_space.cls);
		pci_read_config_word(pdev, PCI_BASE_ADDRESS_0, &tp->pci_cfg_space.io_base_l);
		pci_read_config_word(pdev, PCI_BASE_ADDRESS_0 + 2, &tp->pci_cfg_space.io_base_h);
		pci_read_config_word(pdev, PCI_BASE_ADDRESS_2, &tp->pci_cfg_space.mem_base_l);
		pci_read_config_word(pdev, PCI_BASE_ADDRESS_2 + 2, &tp->pci_cfg_space.mem_base_h);
		pci_read_config_byte(pdev, PCI_INTERRUPT_LINE, &tp->pci_cfg_space.ilr);
		pci_read_config_word(pdev, PCI_BASE_ADDRESS_4, &tp->pci_cfg_space.resv_0x20_l);
		pci_read_config_word(pdev, PCI_BASE_ADDRESS_4 + 2, &tp->pci_cfg_space.resv_0x20_h);
		pci_read_config_word(pdev, PCI_BASE_ADDRESS_5, &tp->pci_cfg_space.resv_0x24_l);
		pci_read_config_word(pdev, PCI_BASE_ADDRESS_5 + 2, &tp->pci_cfg_space.resv_0x24_h);
		tp->pci_cfg_is_read = 1;
	}
#endif
	printk("HW Start \n");
	dump_rx_desc(tp);
	dump_tx_desc(tp);	
	dump_EPreg();


	REG32_W(SPE_DMA_IOCMD, REG32_R(SPE_DMA_IOCMD) | SPE_DMA_IOCMD_RXEN);  //start rx	
	udelay(10);
}
//---------------------------------------------------------------------------
static int rtl8198_change_mtu(struct net_device *dev, 	   int new_mtu)
{
	struct rtl8198_private *tp = netdev_priv(dev);
	int ret = 0;


	printk("change mtu \n");
	
	if (new_mtu < ETH_ZLEN || new_mtu > tp->max_jumbo_frame_size)
		return -EINVAL;

	if (!netif_running(dev))
		goto out;

	rtl8198_down(dev);

	dev->mtu = new_mtu;

	rtl8198_set_rxbufsize(tp, dev);

	ret = rtl8198_init_ring(dev);

	if (ret < 0)
		goto out;

#ifdef CONFIG_R8168_NAPI
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
	RTL_NAPI_ENABLE(dev, &tp->napi);
#endif
#endif//CONFIG_R8168_NAPI 

	rtl8198_hw_start(dev);

out:
	return ret;
}
//---------------------------------------------------------------------------
static inline void rtl8198_make_unusable_by_asic(struct RxDesc *desc)
{
	desc->addr = 0x0badbadbadbadbadull;
#ifdef CONFIG_R8198EP_HOST	
	desc->flag &= ~cpu_to_le32(DescOwn | RsvdMask);
#endif

#ifdef CONFIG_R8198EP_DEVICE	
	desc->flag &= ~cpu_to_be32(DescOwn | RsvdMask);
#endif


}
//---------------------------------------------------------------------------
static void 
rtl8198_free_rx_skb(struct rtl8198_private *tp,
		    struct sk_buff **sk_buff, 
		    struct RxDesc *desc)
{
	struct pci_dev *pdev = tp->pci_dev;
#ifdef CONFIG_R8198EP_HOST
	pci_unmap_single(pdev, le32_to_cpu(desc->addr), tp->rx_buf_sz,		 PCI_DMA_FROMDEVICE);
#endif

	dev_kfree_skb(*sk_buff);
	*sk_buff = NULL;
	rtl8198_make_unusable_by_asic(desc);
}
//---------------------------------------------------------------------------
static inline void 
rtl8198_mark_to_asic(struct RxDesc *desc,      u32 rx_buf_sz)
{
#ifdef CONFIG_R8198EP_HOST
	u32 eor = le32_to_cpu(desc->flag) & RingEnd;
	desc->flag = cpu_to_le32(DescOwn | eor | rx_buf_sz);
#endif
#ifdef CONFIG_R8198EP_DEVICE
	u32 eor = be32_to_cpu(desc->flag) & RingEnd;
	desc->flag = cpu_to_be32(DescOwn | eor | rx_buf_sz);
#endif

//	u32 addr=le32_to_cpu(desc->addr)&~3;//wei add
//	desc->addr = cpu_to_le32(addr);	 //wei add, to reset addr pointer.
}
//---------------------------------------------------------------------------
static inline void rtl8198_map_to_asic(struct RxDesc *desc,     dma_addr_t mapping,	    u32 rx_buf_sz)
{
#ifdef CONFIG_R8198EP_HOST
	desc->addr = cpu_to_le32(mapping);	//wei add, skb address fill to descriptor
#endif
#ifdef CONFIG_R8198EP_DEVICE
	desc->addr = Virtual2Physical(cpu_to_be32(mapping));	//wei add, skb address fill to descriptor
#endif

	wmb();
	rtl8198_mark_to_asic(desc, rx_buf_sz);
}
//---------------------------------------------------------------------------
static int rtl8198_alloc_rx_skb(struct pci_dev *pdev, 
		     struct sk_buff **sk_buff,
		     struct RxDesc *desc, 
		     int rx_buf_sz)
{
	struct sk_buff *skb;
	dma_addr_t mapping;
	int ret = 0;

	skb = dev_alloc_skb(rx_buf_sz + NET_IP_ALIGN);
	if (!skb)
		goto err_out;

	skb_reserve(skb, NET_IP_ALIGN);
	*sk_buff = skb;

#if 1//def CONFIG_R8198EP_HOST
	mapping = pci_map_single(pdev, skb->data, rx_buf_sz, PCI_DMA_FROMDEVICE);
#else// CONFIG_R8198EP_DEVICE
	mapping = skb->data;
#endif
	rtl8198_map_to_asic(desc, mapping, rx_buf_sz);

out:
	return ret;

err_out:
	ret = -ENOMEM;
	rtl8198_make_unusable_by_asic(desc);
	goto out;
}
//-----------------------------------------------------------------------------
static void rtl8198_rx_clear(struct rtl8198_private *tp)
{
	int i;
	for (i = 0; i < NUM_RX_DESC; i++) 
	{
		if (tp->Rx_skbuff[i]) 
		{
			rtl8198_free_rx_skb(tp, tp->Rx_skbuff + i,    tp->RxDescArray + i);
		}
	}
}
//-----------------------------------------------------------------------------
static u32 rtl8198_rx_fill(struct rtl8198_private *tp, 	struct net_device *dev,	u32 start, 	u32 end)
{
	u32 cur;
	
	for (cur = start; end - cur > 0; cur++)   //end-start=0, will not do.
	{
		int ret, i = cur % NUM_RX_DESC;
//#ifdef CONFIG_R8198EP_HOST
#ifndef DEVICE_USING_FIXBUF
		if (tp->Rx_skbuff[i])
			continue;
			
		ret = rtl8198_alloc_rx_skb(tp->pci_dev, tp->Rx_skbuff + i,  tp->RxDescArray + i, tp->rx_buf_sz);
		if (ret < 0)
		{	printk("rtl8198_alloc_rx_skb fail! \n");
			break;
		}
#else  // CONFIG_R8198EP_DEVICE   //Using Self buff

		rtl8198_map_to_asic(tp->RxDescArray + i, tp->pRxBuffPtr[i], tp->rx_buf_sz);
#endif
	}
	return cur - start;
}
//--------------------------------------------------------------------------
static inline void rtl8198_mark_as_last_descriptor(struct RxDesc *desc)
{
#ifdef CONFIG_R8198EP_HOST
	desc->flag |= cpu_to_le32(RingEnd);
#else // CONFIG_R8198EP_DEVICE
	desc->flag |= cpu_to_be32(RingEnd);
#endif


}
//--------------------------------------------------------------------------
static void rtl8198_init_ring_indexes(struct rtl8198_private *tp)
{
	tp->dirty_tx = 0;
	tp->dirty_rx = 0;
	tp->cur_tx = 0;
	tp->cur_rx = 0;
}
//--------------------------------------------------------------------------
static void rtl8198_tx_desc_init(struct rtl8198_private *tp)
{
	int i = 0;	
	memset(tp->TxDescArray, 0x0, NUM_TX_DESC * sizeof(struct TxDesc));
	for (i = 0; i < NUM_TX_DESC; i++)
	{
		if(i == (NUM_TX_DESC - 1))
#ifdef CONFIG_R8198EP_HOST				
			tp->TxDescArray[i].flag= cpu_to_le32(RingEnd);
#else // CONFIG_R8198EP_DEVICE	
			tp->TxDescArray[i].flag= cpu_to_be32(RingEnd);
#endif

	}
}
//--------------------------------------------------------------------------
static void rtl8198_rx_desc_init(struct rtl8198_private *tp)
{
	int i = 0;
	memset(tp->RxDescArray, 0x0, NUM_RX_DESC * sizeof(struct RxDesc));
	for (i = 0; i < NUM_RX_DESC; i++) 
	{
		if(i == (NUM_RX_DESC - 1))
#ifdef CONFIG_R8198EP_HOST			
			tp->RxDescArray[i].flag = cpu_to_le32((DescOwn | RingEnd) |     (unsigned long)tp->rx_buf_sz);
		else
			tp->RxDescArray[i].flag = cpu_to_le32(DescOwn |     (unsigned long)tp->rx_buf_sz);
#else// CONFIG_R8198EP_DEVICE			
			tp->RxDescArray[i].flag = cpu_to_be32((DescOwn | RingEnd) |     (unsigned long)tp->rx_buf_sz);
		else
			tp->RxDescArray[i].flag = cpu_to_be32(DescOwn |     (unsigned long)tp->rx_buf_sz);
#endif

	}
}
//--------------------------------------------------------------------------
static int rtl8198_init_ring(struct net_device *dev)
{
	struct rtl8198_private *tp = netdev_priv(dev);

	rtl8198_init_ring_indexes(tp);

	memset(tp->tx_skb, 0x0, NUM_TX_DESC * sizeof(struct ring_info));
	memset(tp->Rx_skbuff, 0x0, NUM_RX_DESC * sizeof(struct sk_buff *));

#ifdef  CONFIG_R8198EP_DEVICE  //self buff
#ifdef DEVICE_USING_FIXBUF
	int i;
	for(i=0;i<NUM_RX_DESC;i++)
	{	//pRxBuffPtr[i]=0xa0680000+i*BUF_SIZE;
		tp->pRxBuffPtr[i]=(struct slvpcie_rxbuff_t *)Virtual2NonCache(&(tp->RxDataBuff[i][0]));
	}

	for(i=0;i<NUM_TX_DESC;i++)
	{	//pTxBuffPtr[i]=0xa0600ffc+i*BUF_SIZE;
		tp->pTxBuffPtr[i]=(struct slvpcie_txbuff_t *)Virtual2NonCache(&(tp->TxDataBuff[i][0]));
	}
	//printk("tx desc size=%x \n", sizeof(struct TxDesc ));
	//printk("rx desc size=%x \n", sizeof(struct RxDesc ));	
	//printk("Tx Buff Ptr[0]=%x \n",tp->pTxBuffPtr[0]);	
	//printk("Tx Buff Ptr[1]=%x \n",tp->pTxBuffPtr[1]);		
	//printk("Rx Buff Ptr=%x \n",tp->pRxBuffPtr[0]);
#endif
#endif

	rtl8198_tx_desc_init(tp);
	rtl8198_rx_desc_init(tp);

	if (rtl8198_rx_fill(tp, dev, 0, NUM_RX_DESC) != NUM_RX_DESC)   //alloc skb here
		goto err_out;

	rtl8198_mark_as_last_descriptor(tp->RxDescArray + NUM_RX_DESC - 1);

	//wei add
	//dump_rx_desc(tp);
	//dump_tx_desc(tp);	
	
	return 0;

err_out:
	rtl8198_rx_clear(tp);
	return -ENOMEM;
}
//--------------------------------------------------------------------------
static void rtl8198_unmap_tx_skb(struct pci_dev *pdev,      struct ring_info *tx_skb,	 struct TxDesc *desc)
{
	unsigned int len = tx_skb->len;
#ifdef CONFIG_R8198EP_HOST
	pci_unmap_single(pdev, le32_to_cpu(desc->addr), len, PCI_DMA_TODEVICE);
#endif
	desc->flag = 0x00;
	desc->addr = 0x00;	
	desc->info2 = 0x00;
	desc->info3 = 0x00; //wei add	
	tx_skb->len = 0;
}
//--------------------------------------------------------------------------
static void rtl8198_tx_clear(struct rtl8198_private *tp)
{
	unsigned int i;
	struct net_device *dev = tp->dev;

	for (i = tp->dirty_tx; i < tp->dirty_tx + NUM_TX_DESC; i++) 
	{
		unsigned int entry = i % NUM_TX_DESC;
		struct ring_info *tx_skb = tp->tx_skb + entry;
		unsigned int len = tx_skb->len;

		if (len) 
		{
			struct sk_buff *skb = tx_skb->skb;

			rtl8198_unmap_tx_skb(tp->pci_dev, tx_skb,	 tp->TxDescArray + entry);
			if (skb) 
			{
				dev_kfree_skb(skb);
				tx_skb->skb = NULL;
			}
			RTLDEV->stats.tx_dropped++;
		}
	}
	tp->cur_tx = tp->dirty_tx = 0;
}
//--------------------------------------------------------------------------

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
static void rtl8198_schedule_work(struct net_device *dev, void (*task)(void *))
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
	struct rtl8198_private *tp = netdev_priv(dev);

	PREPARE_WORK(&tp->task, task, dev);
	schedule_delayed_work(&tp->task, 4);
#endif //LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
}
#else
static void rtl8198_schedule_work(struct net_device *dev, work_func_t task)
{
	struct rtl8198_private *tp = netdev_priv(dev);

	PREPARE_DELAYED_WORK(&tp->task, task);
	schedule_delayed_work(&tp->task, 4);
}
#endif
//-----------------------------------------------------------------------
static void 
rtl8198_wait_for_quiescence(struct net_device *dev)
{
	struct rtl8198_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;

	synchronize_irq(dev->irq);

	/* Wait for any pending NAPI task to complete */
#ifdef CONFIG_R8168_NAPI
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
	RTL_NAPI_DISABLE(dev, &tp->napi);
#endif
#endif//CONFIG_R8168_NAPI

	rtl8198_irq_mask_and_ack(ioaddr);

#ifdef CONFIG_R8168_NAPI
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
	RTL_NAPI_ENABLE(dev, &tp->napi);
#endif
#endif//CONFIG_R8168_NAPI
}
//-----------------------------------------------------------------------
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
static void rtl8198_reinit_task(void *_data)
#else
static void rtl8198_reinit_task(struct work_struct *work)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
	struct net_device *dev = _data;
#else
	struct rtl8198_private *tp =
		container_of(work, struct rtl8198_private, task.work);
	struct net_device *dev = tp->dev;
#endif
	int ret;

	if (netif_running(dev)) {
		rtl8198_wait_for_quiescence(dev);
		rtl8198_close(dev);
	}

	ret = rtl8198_open(dev);
	if (unlikely(ret < 0)) {
		if (net_ratelimit()) {
			struct rtl8198_private *tp = netdev_priv(dev);

			if (netif_msg_drv(tp)) {
				printk(PFX KERN_ERR
				       "%s: reinit failure (status = %d)."
				       " Rescheduling.\n", dev->name, ret);
			}
		}
		rtl8198_schedule_work(dev, rtl8198_reinit_task);
	}
}
//-----------------------------------------------------------------------
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
static void rtl8198_reset_task(void *_data)
{
	struct net_device *dev = _data;
	struct rtl8198_private *tp = netdev_priv(dev);
#else
static void rtl8198_reset_task(struct work_struct *work)
{
	struct rtl8198_private *tp =
		container_of(work, struct rtl8198_private, task.work);
	struct net_device *dev = tp->dev;
#endif

	if (!netif_running(dev))
		return;

	rtl8198_wait_for_quiescence(dev);

//	rtl8198_rx_interrupt(dev, tp, tp->mmio_addr);
	rtl8198_rx_interrupt(tp);
	rtl8198_tx_clear(tp);

	if (tp->dirty_rx == tp->cur_rx) {
		rtl8198_init_ring_indexes(tp);
		rtl8198_hw_start(dev);
		netif_wake_queue(dev);
	} else {
		if (net_ratelimit()) {
			struct rtl8198_private *tp = netdev_priv(dev);

			if (netif_msg_intr(tp)) {
				printk(PFX KERN_EMERG
				       "%s: Rx buffers shortage\n", dev->name);
			}
		}
		rtl8198_schedule_work(dev, rtl8198_reset_task);
	}
}
//-----------------------------------------------------------------------
static void 
rtl8198_tx_timeout(struct net_device *dev)
{
	printk("tx timeout! \n"); //wei add
//	rtl8198_interrupt(0, dev);
	RTLDEV->stats.tx_errors++;	
	netif_wake_queue(dev); //wei add	
	
	//rtl8198_hw_reset(dev);

	/* Let's wait a bit while any (async) irq lands on */
	//rtl8198_schedule_work(dev, rtl8198_reset_task);
}
//-----------------------------------------------------------------------
static int 
rtl8198_xmit_frags(struct rtl8198_private *tp, 
		   struct sk_buff *skb,
		   u32 flag)
{
	struct skb_shared_info *info = skb_shinfo(skb);
	unsigned int cur_frag, entry;
	struct TxDesc *txd = NULL;

	entry = tp->cur_tx;
	for (cur_frag = 0; cur_frag < info->nr_frags; cur_frag++) 
	{
		skb_frag_t *frag = info->frags + cur_frag;
		dma_addr_t mapping;
		u32 status, len;
		void *addr;

		entry = (entry + 1) % NUM_TX_DESC;

		txd = tp->TxDescArray + entry;
		len = frag->size;
		addr = ((void *) page_address(frag->page)) + frag->page_offset;
		mapping = pci_map_single(tp->pci_dev, addr, len, PCI_DMA_TODEVICE);

		/* anti gcc 2.95.3 bugware (sic) */
		status = flag | len | (RingEnd * !((entry + 1) % NUM_TX_DESC));
#ifdef CONFIG_R8198EP_HOST
		txd->flag = cpu_to_le32(status);
		txd->addr = cpu_to_le32(mapping);
#endif
#ifdef CONFIG_R8198EP_DEVICE
		txd->flag = cpu_to_be32(status);
		txd->addr = cpu_to_be32(mapping);
#endif


		tp->tx_skb[entry].len = len;
	}

	if (cur_frag) 
	{
		tp->tx_skb[entry].skb = skb;
		//txd->flag |= cpu_to_le32(LastFrag);  //wei del

#ifdef CONFIG_R8198EP_HOST		
		txd->flag |= cpu_to_le32(FirstFrag | LastFrag);
#endif
#ifdef CONFIG_R8198EP_DEVICE		
		txd->flag |= cpu_to_be32(FirstFrag | LastFrag);
#endif


	}

	return cur_frag;
}
//--------------------------------------------------------------------------------
//Wei add, for test  function use
int rtl8198_hw_tx(void * buff, unsigned int len, 
			unsigned int fs, unsigned int ls, 
			unsigned int txinfo1, unsigned int txinfo2, unsigned int txinfo3,
			unsigned int offset)
{
 	struct net_device *dev=reNet;
	struct rtl8198_private *tp = netdev_priv(dev);
	unsigned int entry = tp->cur_tx % NUM_TX_DESC;
	struct TxDesc *txd = tp->TxDescArray + entry;
	void __iomem *ioaddr = tp->mmio_addr;
	
	dma_addr_t mapping;
	u32 flag = 0;

	int ret = 1;


#ifdef CONFIG_R8198EP_HOST
	if ((le32_to_cpu(txd->flag) & DescOwn)==DescOwn)
		goto err_stop;

#else // CONFIG_R8198EP_DEVICE
	if ((be32_to_cpu(txd->flag) & DescOwn)==DescOwn)
		goto err_stop;

#endif

#ifdef CONFIG_R8198EP_HOST

	//alloc skb
	struct sk_buff *skb;
	skb= dev_alloc_skb(len);
	if (!skb) 
	{
		printk(" tx: fail, no mem \n");		
		return NULL;
	}
	skb_put(skb, len+offset);
	memcpy(skb->data+offset, buff, len);
	skb->dev = tp->dev;	
	skb->len=len+offset; //tp->DMA_len;   //using key in len

	
	tp->tx_skb[entry].skb = skb;
	tp->tx_skb[entry].len = len+offset;
	
	mapping = pci_map_single(tp->pci_dev, skb->data+offset, skb->len, PCI_DMA_TODEVICE);  //wei add note, will do cache flush 
	/*
	if(offset)
	{	mapping+=(offset&0x3);
	}
	*/
#else // CONFIG_R8198EP_DEVICE
	
#ifdef DEVICE_USING_FIXBUF	
	unsigned char *pData=tp->pTxBuffPtr[entry];
	if(offset)
	{	pData+=(offset&0x3);
		printk("pData=%x \n", pData);
	}
	memcpy(pData, buff, len);	
	mapping=Virtual2Physical(pData);
#endif	

#endif


	flag = DescOwn;
	if(fs) flag|=FirstFrag;
	if(ls) flag|=LastFrag;
	flag |=(txinfo1<< Info1_Offset)&Info1_Mask;
	flag |= len | (RingEnd * !((entry + 1) % NUM_TX_DESC));
	
#ifdef CONFIG_R8198EP_HOST	

	txd->addr = cpu_to_le32(mapping);
	txd->info2 = cpu_to_le32(txinfo2);	
	txd->info3 = cpu_to_le32(txinfo3);
	wmb();
	txd->flag = cpu_to_le32(flag);   //set own bit
	
#else // CONFIG_R8198EP_DEVICE
	txd->addr = cpu_to_be32(mapping);
	txd->info2 = cpu_to_be32(txinfo2);	
	txd->info3 = cpu_to_be32(txinfo3);
	wmb();
	txd->flag = cpu_to_be32(flag);   //set own bit
	
#endif

	dev->trans_start = jiffies;
	tp->cur_tx += 1;
	smp_wmb();

	
	if(tp->dmsg) //debug	
	{
	 	printk("-------------------------");
		printk("=>rtl8198_hw_tx \n"); //wei add
		
		dump_tx_desc(tp);  //wei add	
#ifdef CONFIG_R8198EP_HOST	
		ddump(skb->data+offset, len); //wei add
#else
#ifdef DEVICE_USING_FIXBUF
		ddump( (int)(tp->pTxBuffPtr[entry])+offset  , len); //wei add
#endif		
#endif		
	}

	REG32_W(SPE_DMA_IOCMD, REG32_R(SPE_DMA_IOCMD)|SPE_DMA_IOCMD_TXPOLL);	/* set polling bit */


out:
	return ret;
err_stop:
#if 0
	netif_stop_queue(dev);
	ret = (-1);
#endif	
	RTLDEV->stats.tx_dropped++;
	goto out;

}

//====================================================================
//Linux tx callback function
static int rtl8198_start_xmit(struct sk_buff *skb, 	   struct net_device *dev)
{
/*
	char * tmp = skb->data;
	int i100 = 0;
	
	for(i100 = 0 ; i100< 14;i100++)
	{
		printk("%02X",tmp[i100]);
	}
	printk("\n");
*/	
	struct rtl8198_private *tp = netdev_priv(dev);
	unsigned int frags, entry = tp->cur_tx % NUM_TX_DESC;
	struct TxDesc *txd = tp->TxDescArray + entry;
	void __iomem *ioaddr = tp->mmio_addr;
	dma_addr_t mapping;
	u32  len;
	u32 flag = 0;


	spin_lock_irq(&tp->lock);  //wei add
	//extra tx param
	u32 info1=(tp->info1<<Info1_Offset)&Info1_Mask;
	u32 info2 = (tp->info2);
	u32 info3 = (tp->info3);	
	u32 offset = (tp->txoffset);

	int ret = NETDEV_TX_OK;

	 if(tp->dmsg)//debug	
	 {	printk("-------------------------");
		printk("=>rtl8198_start_xmit \n"); //wei add
	 }


	//Work around for rx fifo overflow
	//if (tp->rx_fifo_overflow == 1)
	//	goto err_stop;
	
#if 0 //def CONFIG_R8198EP_HOST
       //printk("nr_frags=%d\n",skb_shinfo(skb)->nr_frags);
	if (unlikely(TX_BUFFS_AVAIL(tp) < skb_shinfo(skb)->nr_frags)) 
	{
		if (netif_msg_drv(tp)) 
		{
			printk(KERN_ERR      "%s: BUG! Tx Ring full when queue awake!\n",       dev->name);
		}
		printk(KERN_ERR      "%s: BUG! Tx Ring full when queue awake!\n",       dev->name);  //wei add
		goto err_stop;
	}
#endif

#ifdef CONFIG_R8198EP_HOST
	if (unlikely(le32_to_cpu(txd->flag) & DescOwn))
		goto err_stop;

#else // CONFIG_R8198EP_DEVICE
	if (unlikely(be32_to_cpu(txd->flag) & DescOwn))
		goto err_stop;

#endif

	flag = DescOwn;

#if 0
	frags = rtl8198_xmit_frags(tp, skb, flag);
	if (frags) 
	{
		len = skb_headlen(skb);
		//flag |= FirstFrag;
		flag |= FirstFrag | LastFrag; //wei add
	} 
	else
#endif	
	frags=0; //wei add
	{
		len = skb->len;
		flag |= FirstFrag | LastFrag;
		tp->tx_skb[entry].skb = skb;
	}
#if 1//def CONFIG_R8198EP_HOST
	mapping = pci_map_single(tp->pci_dev, skb->data, len, PCI_DMA_TODEVICE);  //wei add note, will do cache flush 
	
	if(offset)
	{	mapping+=(offset&0x3);
	}
	
#else // CONFIG_R8198EP_DEVICE

//#if 0  //from skb
#ifndef DEVICE_USING_FIXBUF
	mapping=Virtual2Physical(skb->data);
#else //copy skb to local buffer	
	unsigned char *pData=tp->pTxBuffPtr[entry];
	if(offset)
	{	pData+=(offset&0x3);
		printk("pData=%x \n", pData);
	}
	memcpy(pData, skb->data, skb->len);	
	mapping=Virtual2Physical(pData);
	
	/*  //no offset
	memcpy(tp->pTxBuffPtr[entry], skb->data, skb->len);	
	mapping=Virtual2Physical(tp->pTxBuffPtr[entry]);
	*/
#endif

#endif
	tp->tx_skb[entry].len = len;
	
#ifdef CONFIG_R8198EP_HOST	

	txd->addr = cpu_to_le32(mapping);
	txd->info2 = cpu_to_le32(info2);	
	txd->info3 = cpu_to_le32(info3);

	wmb();

	flag |=  info1 |len | (RingEnd * !((entry + 1) % NUM_TX_DESC));
	txd->flag = cpu_to_le32(flag);   //set own bit
	
#else // CONFIG_R8198EP_DEVICE
	txd->addr = cpu_to_be32(mapping);
	txd->info2 = cpu_to_be32(info2);	
	txd->info3 = cpu_to_be32(info3);

	wmb();

	flag |= info1 |len | (RingEnd * !((entry + 1) % NUM_TX_DESC));
	txd->flag = cpu_to_be32(flag);   //set own bit
#endif



	dev->trans_start = jiffies;

	tp->cur_tx += (frags + 1);

	smp_wmb();

	
	if(tp->dmsg) //debug	
	{
		dump_tx_desc(tp);  //wei add	
#ifdef CONFIG_R8198EP_HOST	
		ddump(skb->data+offset, len); //wei add
#else
#ifndef DEVICE_USING_FIXBUF
		ddump( (int)Physical2NonCache(mapping)  , len); //wei add, from skb->data
#else		
		ddump( (int)(tp->pTxBuffPtr[entry])+offset  , len); //wei add, from local buffer
#endif		
#endif		
	}

	REG32_W(SPE_DMA_IOCMD, REG32_R(SPE_DMA_IOCMD)|SPE_DMA_IOCMD_TXPOLL);	/* set polling bit */
#if 0
      printk("TX_BUFFS_AVAIL(tp)=%d, MAX_SKB_FRAGS=%d ",TX_BUFFS_AVAIL(tp) , MAX_SKB_FRAGS);
	if (TX_BUFFS_AVAIL(tp) < MAX_SKB_FRAGS) 
	{
		netif_stop_queue(dev);
		smp_rmb();
		if (TX_BUFFS_AVAIL(tp) >= MAX_SKB_FRAGS)
			netif_wake_queue(dev);
	}
#endif	

out:
	spin_unlock_irq(&tp->lock);  //wei add
	return ret;
err_stop:
	printk("tx err_stop\n");

	netif_stop_queue(dev);  
//	ret = NETDEV_TX_BUSY;   //wei del

	RTLDEV->stats.tx_dropped++;
	dev_kfree_skb(skb);		//wei add
	goto out;
}
//--------------------------------------------------------------------------------
static void 
rtl8198_pcierr_interrupt(struct net_device *dev)
{
	struct rtl8198_private *tp = netdev_priv(dev);
	struct pci_dev *pdev = tp->pci_dev;
	u16 pci_status, pci_cmd;

	pci_read_config_word(pdev, PCI_COMMAND, &pci_cmd);
	pci_read_config_word(pdev, PCI_STATUS, &pci_status);

	if (netif_msg_intr(tp)) {
		printk(KERN_ERR
		       "%s: PCI error (cmd = 0x%04x, status = 0x%04x).\n",
		       dev->name, pci_cmd, pci_status);
	}

	/*
	 * The recovery sequence below admits a very elaborated explanation:
	 * - it seems to work;
	 * - I did not see what else could be done.
	 *
	 * Feel free to adjust to your needs.
	 */
	pci_write_config_word(pdev, PCI_COMMAND,
			      pci_cmd | PCI_COMMAND_SERR | PCI_COMMAND_PARITY);

	pci_write_config_word(pdev, PCI_STATUS,
		pci_status & (PCI_STATUS_DETECTED_PARITY |
		PCI_STATUS_SIG_SYSTEM_ERROR | PCI_STATUS_REC_MASTER_ABORT |
		PCI_STATUS_REC_TARGET_ABORT | PCI_STATUS_SIG_TARGET_ABORT));

	rtl8198_hw_reset(dev);
}
//--------------------------------------------------------------------------------
static void
rtl8198_tx_interrupt(struct net_device *dev,   struct rtl8198_private *tp,    void __iomem *ioaddr)
//rtl8198_tx_interrupt(struct rtl8198_private *tp)
{
//	struct net_device *dev = tp->dev;
//	void __iomem *ioaddr = tp->mmio_addr;
	
	unsigned int dirty_tx, tx_left;

	assert(dev != NULL);
	assert(tp != NULL);
	assert(ioaddr != NULL);

	if(tp->dmsg)
	{	printk("rtl8198_tx_interrupt \n");
	}

	spin_lock_irq(&tp->lock);  //wei add

	
	dirty_tx = tp->dirty_tx;
	smp_rmb();
	tx_left = tp->cur_tx - dirty_tx;

	while (tx_left > 0) 
	{
		unsigned int entry = dirty_tx % NUM_TX_DESC;	
		struct ring_info *tx_skb = tp->tx_skb + entry;
		u32 len = tx_skb->len;
		u32 status;

		rmb();
#ifdef CONFIG_R8198EP_HOST	
		status = le32_to_cpu(tp->TxDescArray[entry].flag);
#else	
		status = be32_to_cpu(tp->TxDescArray[entry].flag);
#endif	
		if (status & DescOwn)
			break;

		RTLDEV->stats.tx_bytes += len;
		RTLDEV->stats.tx_packets++;
		
//#ifdef CONFIG_R8198EP_HOST
#if 1
		rtl8198_unmap_tx_skb(tp->pci_dev, 	     tx_skb,      tp->TxDescArray + entry);	
		//if (status & LastFrag) 
		{
			if(tx_skb->skb)
			dev_kfree_skb_irq(tx_skb->skb);
			tx_skb->skb = NULL;
		}
#endif		
		dirty_tx++;
		tx_left--;
	} //end while()

	if (tp->dirty_tx != dirty_tx) 
	{
		tp->dirty_tx = dirty_tx;
		smp_wmb();
		
		//if (netif_queue_stopped(dev) &&    (TX_BUFFS_AVAIL(tp) >= MAX_SKB_FRAGS)) 	//wei del
		{
			netif_wake_queue(dev);
		}
	}
//	REG32_W(SPE_DMA_IMR, REG32_R(SPE_DMA_IMR)|SPE_DMA_IMR_TXDU);
//	REG32_W(SPE_DMA_IMR, REG32_R(SPE_DMA_IMR)|SPE_DMA_IMR_TXERR);
//	REG32_W(SPE_DMA_IMR, REG32_R(SPE_DMA_IMR)|SPE_DMA_IMR_TXOK);
	
	spin_unlock_irq(&tp->lock);//wei add	
}
//--------------------------------------------------------------------------------
rtl8198_software_interrupt(struct net_device *dev,   struct rtl8198_private *tp,    void __iomem *ioaddr)
{
	
#ifdef CONFIG_R8198EP_DEVICE

//for mem
#define MEM32_R(reg)		((u32) readl ((reg)))
#define MEM32_W(reg, val32)	writel ((val32),  (reg))


	unsigned int *p=Virtual2NonCache(tp->ISM_buff);

	if(tp->dmsg)
	{
		dwdump(p, 4);
	}

	if( (p[0]&0xffffff00) !=SPE_ISM_MAGNUMI )  //magic input
	{	dprintf("Fail ISM MAG=%x \n", p[0]);
		return 0;
	};	

	unsigned char mode=(unsigned char) (p[0]&0xff);
	
	
	unsigned int a1=p[1];
	unsigned int a2=p[2];
	unsigned int a3=p[3];	
	
	
	switch(mode)
	{
		case SPE_ISM_MAG_REGR:		p[3]=MEM32_R(a1); break;
		case SPE_ISM_MAG_REGW:     MEM32_W(a1, (MEM32_R(a1)&a2)|a3);  break;
		case SPE_ISM_MAG_DMARST: 
						//REG32_W(SPE_DMA_IOCMD, REG32_R(SPE_DMA_IOCMD) | SPE_DMA_IOCMD_RST); 				
						//rtl8198_nic_reset(dev);
						DMA_Restart(dev);
						break;
		default: return ;
	}

	p[0]=('w'<<24) | ('e'<<16) | ('o'<<8) | (mode<<0);
	REG32_W(SPE_DMA_IOCMD, REG32_R(SPE_DMA_IOCMD) |SPE_DMA_IOCMD_SWINT);  //trigger	
#endif

}
//--------------------------------------------------------------------------------
rtl8198_system_ctrlstatus_interrupt(struct net_device *dev,   struct rtl8198_private *tp,    void __iomem *ioaddr, u32 syscr)
{
#ifdef CONFIG_R8198EP_DEVICE
		int i;
		for(i=0 ; i<16; i++)
		{
			if(syscr &  (1<<i) )
			{	if(tp->dmsg)	dprintf("=> Got IRQ CUSTUSR#%02d \n", i);
				//REG32(SPE_NFBI_SYSSR)^= (1<<i);
			}
		}		
		REG32_W(SPE_NFBI_SYSSR, REG32_R(SPE_NFBI_SYSCR));	
#endif
}
//--------------------------------------------------------------------------------
static inline int 
rtl8198_try_rx_copy(struct sk_buff **sk_buff,     int pkt_size,    struct RxDesc *desc, 	    int rx_buf_sz)
{
	int ret = -1;
#ifdef CONFIG_R8198EP_HOST
	int offset=le32_to_cpu(desc->addr)&03;
#else 
	int src_addr=Physical2NonCache(be32_to_cpu(desc->addr));
#endif

//	if (pkt_size < rx_copybreak) 	//wei del, for always copy
//	if(0)  //wei add
	{
		struct sk_buff *skb;

		skb = dev_alloc_skb(pkt_size + NET_IP_ALIGN);
		if (skb) 
		{
			skb_reserve(skb, NET_IP_ALIGN);
#ifdef CONFIG_R8198EP_HOST			
			eth_copy_and_sum(skb, sk_buff[0]->data+offset, pkt_size, 0);
			

			//printk("skb=%x, sk_buff[0]->data=%x, pk_size=%d (0x%x) \n", skb, sk_buff[0]->data, pkt_size,pkt_size);
			
			
			//wei add
			dev_kfree_skb(sk_buff[0]);
#else
			memcpy(skb->data, src_addr, pkt_size);
#endif

			*sk_buff = skb;
			rtl8198_mark_to_asic(desc, rx_buf_sz);
			ret = 0;
		}
	}
	return ret;
}

//--------------------------------------------------------------------------------
//wei add , for test function
int rtl8198_hw_rx(void** input, unsigned int* pLen,  
			unsigned int *prxinfo1, unsigned int *prxinfo2, unsigned int *prxinfo3, 
			unsigned int *poffset, unsigned int *drop)
{

	unsigned int  count = 0;
	u32 offset;

	struct net_device *dev= reNet;
	struct rtl8198_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;

	
	assert(dev != NULL);
	assert(tp != NULL);
	assert(ioaddr != NULL);



	//check
	if ((tp->RxDescArray == NULL) || (tp->Rx_skbuff == NULL)) 
	{
		goto rx_out;
	}

		unsigned int entry = tp->cur_rx % NUM_RX_DESC;
		struct RxDesc *desc = tp->RxDescArray + entry;
		u32 flag;


		rmb();
#ifdef CONFIG_R8198EP_HOST	
		flag = le32_to_cpu(desc->flag);
		offset= le32_to_cpu(desc->addr)&03;
		if(prxinfo2)		*prxinfo2= le32_to_cpu(desc->info2);
		if(prxinfo3)		*prxinfo3= le32_to_cpu(desc->info3);	
		
#else	
		flag = be32_to_cpu(desc->flag);
		offset= be32_to_cpu(desc->addr)&03;	
		if(prxinfo2)	*prxinfo2=  be32_to_cpu(desc->info2);
		if(prxinfo3)	*prxinfo3= be32_to_cpu(desc->info3);	
	
#endif	
		
 		if(pLen)	*pLen=(flag & 0x0000FFFF);
		if(drop)		*drop=(flag&Drop)>>Drop_Offset;
		if(prxinfo1)		*prxinfo1= (flag&Info1_Mask)>>Info1_Offset;
		
		if(poffset)	*poffset=offset;
		

		if (flag & DescOwn)
		{	//printk("own=0, Not belong to CPU! \n");  //wei add
			return -1;
		}
		
		if (flag &  Drop)
		{
			rtl8198_mark_to_asic(desc, tp->rx_buf_sz);
			return 1;
		} 
		else
		{
			struct sk_buff *skb = tp->Rx_skbuff[entry];
			int pkt_size = (flag & 0x0000FFFF)  ;
			void (*pci_action)(struct pci_dev *, dma_addr_t, size_t, int) = pci_dma_sync_single_for_device;


#ifdef CONFIG_R8198EP_HOST				
			pci_dma_sync_single_for_cpu(tp->pci_dev,le32_to_cpu(desc->addr), tp->rx_buf_sz,	PCI_DMA_FROMDEVICE);
#endif
		if(tp->dmsg)
		{
			dprintf("-----------------------------=> Rx \n");
			dump_rx_desc(tp);  //wei add		

#ifdef CONFIG_R8198EP_HOST				
			ddump(skb->data+offset, pkt_size);  //wei add
#else
			ddump( Physical2NonCache(desc->addr), pkt_size);  //wei ad
#endif
		}
		

#ifdef CONFIG_R8198EP_HOST	
			memcpy(tp->DMA_rxbuff, skb->data+offset, pkt_size);
#else
			memcpy(tp->DMA_rxbuff, Physical2NonCache(be32_to_cpu(desc->addr)), pkt_size);
#endif
			rtl8198_mark_to_asic(desc, tp->rx_buf_sz);
			*input=tp->DMA_rxbuff;
			
#ifdef CONFIG_R8198EP_HOST			
			pci_action(tp->pci_dev, le32_to_cpu(desc->addr),	   tp->rx_buf_sz, PCI_DMA_FROMDEVICE);
#endif

		}

	tp->dirty_rx = entry;
	tp->cur_rx = entry+1;

	rtl8198_rx_fill(tp, dev, tp->dirty_rx, tp->cur_rx);

	//wei add, always rxen
	REG32_W(SPE_DMA_IOCMD, REG32_R(SPE_DMA_IOCMD) | SPE_DMA_IOCMD_RXEN);   //wei add
rx_out:
	return 1;
}
//--------------------------------------------------------------------------------
//rx buff -> copy to new skb -> kernel free
//static int rtl8198_rx_interrupt(struct net_device *dev,      struct rtl8198_private *tp,     void __iomem *ioaddr)
static int rtl8198_rx_interrupt(struct rtl8198_private *tp)
{
     	void __iomem *ioaddr = tp->mmio_addr;
     	struct net_device *dev = tp->dev;
     	
	unsigned int cur_rx, rx_left;
	unsigned int delta, count = 0;


	assert(dev != NULL);
	assert(tp != NULL);
	assert(ioaddr != NULL);

	if(tp->dmsg)
	{	
		printk("rtl8198_rx_interrupt \n");
	}

//	spin_lock_irq(&tp->lock);

	cur_rx = tp->cur_rx;
	rx_left = NUM_RX_DESC + tp->dirty_rx - cur_rx;
	//rx_left = rtl8198_rx_quota(rx_left, (u32) rx_quota);  //wei del


	if(tp->dmsg)
	{	dprintf("-----------------------------=> Rx \n");	
		dump_rx_desc(tp);  //wei add
	}
	
	//check
	if ((tp->RxDescArray == NULL) || (tp->Rx_skbuff == NULL)) 
	{	printk("RxDescArray=NULL or Rx_skbuff=NULL\n");
		goto rx_out;
	}

	for (; rx_left > 0; rx_left--, cur_rx++) 
	{
		unsigned int entry = cur_rx % NUM_RX_DESC;
		struct RxDesc *desc = tp->RxDescArray + entry;
		
		if(desc==NULL)
		{	printk("rx desc=NULL\n");
			break;
		}
			
		u32 status;
		u32 offset;
		u32 info1,info2,info3;

		rmb();
		
#ifdef CONFIG_R8198EP_HOST	
		status = le32_to_cpu(desc->flag);
		offset= le32_to_cpu(desc->addr)&03;
		info2= le32_to_cpu(desc->info2);
		info3= le32_to_cpu(desc->info3);
#else	
		status = be32_to_cpu(desc->flag);
		offset= be32_to_cpu(desc->addr)&03;
		info2= be32_to_cpu(desc->info2);	
		info3= be32_to_cpu(desc->info3);		
#endif		

		info1= ((status) & Info1_Mask ) >> Info1_Offset;

	if(tp->dmsg)
	{	
		//printk("cur_rx=%d, entry=%d, rx_left=%d \n", cur_rx, entry, rx_left);  //wei add
		//printk("rx flag=%x \n", status);  //wei add
	}

		if (status & DescOwn)
		{	//printk("own=1, belong to NIC, Not belong to CPU! \n");  //wei add
			break;
		}
		
		if (status &  Drop)
		{
			//if (netif_msg_rx_err(tp)) 
			{
				printk(KERN_INFO   "%s: Rx ERROR. status = %08x\n",	   dev->name, status);
			}
			RTLDEV->stats.rx_dropped++;
			//RTLDEV->stats.rx_errors++;
			rtl8198_mark_to_asic(desc, tp->rx_buf_sz);
		} 
		else
		{
			struct sk_buff *skb = tp->Rx_skbuff[entry];
			int pkt_size = (status & 0x0000FFFF)  ;
			void (*pci_action)(struct pci_dev *, dma_addr_t, size_t, int) = pci_dma_sync_single_for_device;

			/*
			 * The driver does not support incoming fragmented
			 * frames. They are seen as a symptom of over-mtu
			 * sized frames.
			 */
			//if (unlikely(rtl8198_fragmented_frame(status))) 
			#if 0		
			if (status & (FirstFrag | LastFrag)) != (FirstFrag | LastFrag)		
			{
				RTLDEV->stats.rx_dropped++;
				RTLDEV->stats.rx_length_errors++;
				rtl8198_mark_to_asic(desc, tp->rx_buf_sz);
				continue;
			}
			#endif

#ifdef CONFIG_R8198EP_HOST				
			pci_dma_sync_single_for_cpu(tp->pci_dev,le32_to_cpu(desc->addr), tp->rx_buf_sz,	PCI_DMA_FROMDEVICE);
#else
			pci_dma_sync_single_for_cpu(tp->pci_dev,be32_to_cpu(desc->addr), tp->rx_buf_sz,	PCI_DMA_FROMDEVICE);
#endif




		if(tp->dmsg)
		{
			//wei add
			//ddump(desc->addr, pkt_size);  //wei add, have error
			dprintf("rx pklen=0x%x \n", pkt_size);
#ifdef CONFIG_R8198EP_HOST				
			ddump(skb->data+offset, pkt_size);  //wei add
#else
			ddump( Physical2NonCache(desc->addr), pkt_size);  //wei ad
#endif
		}
#if defined(CONFIG_R8198EP_HOST) || defined(DEVICE_USING_FIXBUF)
			if (rtl8198_try_rx_copy(&skb, pkt_size, desc,		tp->rx_buf_sz)==0)  //do copy, reutrn 0, not do copy, reutnr -1 //wei del
#endif			
			{
				//copy success
				pci_action = pci_unmap_single;
				tp->Rx_skbuff[entry] = NULL;
				
			}

 
			//ddump(skb->data, pkt_size);  //wei add
#ifdef CONFIG_R8198EP_HOST			
			pci_action(tp->pci_dev, le32_to_cpu(desc->addr),	   tp->rx_buf_sz, PCI_DMA_FROMDEVICE);
#else
			pci_action(tp->pci_dev, be32_to_cpu(desc->addr),	   tp->rx_buf_sz, PCI_DMA_FROMDEVICE);
#endif
#ifndef CONFIG_R8198EP_HOST
		skb->data += offset; 
#endif
			skb->dev = dev;
	
			
			skb_put(skb, pkt_size);
			skb->protocol = eth_type_trans(skb, dev);			
			skb->ip_summed = CHECKSUM_UNNECESSARY;  //wei add


			//if (rtl8198_rx_vlan_skb(tp, desc, skb) < 0)
			if(netif_rx(skb)!=NET_RX_SUCCESS)
			{
				printk("netif_rx fail \n");
			};

			dev->last_rx = jiffies;
			RTLDEV->stats.rx_bytes += pkt_size;
			RTLDEV->stats.rx_packets++;
		}
	}  //end for()

	count = cur_rx - tp->cur_rx;
	tp->cur_rx = cur_rx;

	delta = rtl8198_rx_fill(tp, dev, tp->dirty_rx, tp->cur_rx);
	if (!delta && count && netif_msg_intr(tp))
	{	printk(KERN_INFO "%s: no Rx buffer allocated\n", dev->name);
	}
	
		
	if(delta!=count)
	{	printk("delta=%x count=%x\n", delta,count);		
	}
	tp->dirty_rx += delta;

	/*
	 * FIXME: until there is periodic timer to try and refill the ring,
	 * a temporary shortage may definitely kill the Rx process.
	 * - disable the asic to try and avoid an overflow and kick it again
	 *   after refill ?
	 * - how do others driver handle this condition (Uh oh...).
	 */
	if ((tp->dirty_rx + NUM_RX_DESC == tp->cur_rx) && netif_msg_intr(tp))
	{	printk(KERN_EMERG "%s: Rx buffers exhausted\n", dev->name);
		printk("Rx buffers exhausted \n");
	}


	//wei add, always rxen
	REG32_W(SPE_DMA_IOCMD, REG32_R(SPE_DMA_IOCMD) | SPE_DMA_IOCMD_RXEN);   //wei add
rx_out:
//	spin_unlock_irq(&tp->lock);
#ifdef RX_TASKLET
//	REG32_W(SPE_DMA_IMR, REG32_R(SPE_DMA_IMR)|SPE_DMA_IMR_RXDU);
//	REG32_W(SPE_DMA_IMR, REG32_R(SPE_DMA_IMR)|SPE_DMA_IMR_RXERR);
	REG32_W(SPE_DMA_IMR, REG32_R(SPE_DMA_IMR)|SPE_DMA_IMR_RXOK);
#endif
	return count;
}
//=====================================================================
/* 
 *The interrupt handler does all of the Rx thread work and cleans up after 
 *the Tx thread.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
static irqreturn_t rtl8198_interrupt(int irq, void *dev_instance, struct pt_regs *regs)
#else
static irqreturn_t rtl8198_interrupt(int irq, void *dev_instance)
#endif
{
	struct net_device *dev = (struct net_device *) dev_instance;	
	struct rtl8198_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
	
	int boguscnt = max_interrupt_work;

	u32 status;
	
//	u32 imr = REG32_R(SPE_DMA_IMR);
	int handled = 0;
	/*
	u16 intr_clean_mask = SYSErr | PCSTimeout | SWInt | 
			      LinkChg | RxDescUnavail | 
			      TxErr | TxOK | RxErr | RxOK;
	*/
//-------------------------------------------
	//tp->intr_mask=REG32_R(SPE_DMA_IMR);
#ifndef RX_TASKLET	
	REG32_W(SPE_DMA_IMR, 0x00000000);
#endif
	do {
		status = REG32_R(SPE_DMA_ISR);
//		if (status & ~(SPE_DMA_ISR_RXOK | SPE_DMA_ISR_RXDU | SPE_DMA_ISR_RXERR | SPE_DMA_ISR_TXOK | SPE_DMA_ISR_TXDU|SPE_DMA_ISR_TXERR))
//			printk("status :[%x]",status);	

		/* hotplug/major error/no more work/shared irq */
		if ((status == 0xFFFFFFFF) || !status)
			break;

		handled = 1;
#if 0
		if (unlikely(!netif_running(dev))) 
		{	printk("ASIC DOWN!\n"); //wei add
			rtl8198_asic_down(dev);
			goto out;
		}
#endif
		//status &= (tp->intr_mask | SPE_DMA_ISR_TXDU);
		//REG32_W(SPE_DMA_ISR, intr_clean_mask);
		REG32_W(SPE_DMA_ISR,status);
		
		#if 0
		if (!(status & rtl8198_intr_mask))
			break;

		//Work around for rx fifo overflow

		if (unlikely(status & RxFIFOOver))
			if (tp->mcfg == CFG_METHOD_1) 
			{
				tp->rx_fifo_overflow = 1;
				netif_stop_queue(dev);
				udelay(300);
				rtl8198_rx_clear(tp);
				rtl8198_init_ring(dev);
				rtl8198_hw_start(dev);
				RTL_W16(IntrStatus, RxFIFOOver);
				netif_wake_queue(dev);
				tp->rx_fifo_overflow = 0;
			}
		#endif
		#if 0
		if (unlikely(status & SYSErr)) 
		{
			rtl8198_pcierr_interrupt(dev);
			break;
		}

		if (status & LinkChg)
			rtl8198_check_link_status(dev, tp, ioaddr);
		#endif


		/* Rx interrupt */
		if (status & (SPE_DMA_ISR_RXOK | SPE_DMA_ISR_RXDU | SPE_DMA_ISR_RXERR)) 
		{
			if(tp->dmsg)
			{
				if(status & SPE_DMA_ISR_RXDU)			printk("RXDU !\n");		
				if(status & SPE_DMA_ISR_RXERR)		printk("RXERR !\n");
			}
			
		#ifdef RX_TASKLET
		//	imr &= ~ SPE_DMA_IMR_RXDU;
		//	imr &= ~ SPE_DMA_IMR_RXERR;
		//	imr &= ~ SPE_DMA_IMR_RXOK;
			
			REG32_W(SPE_DMA_IMR,(REG32_R(SPE_DMA_IMR)&~ SPE_DMA_IMR_RXOK));
				
			tasklet_schedule(&tp->rx_tasklet);
		#else
			rtl8198_rx_interrupt((unsigned long)tp);
			//rtl8198_rx_interrupt(dev, tp, tp->mmio_addr);
                	//interrupt_dsr_rx((unsigned long)cp);
		#endif

		
		}
		/* Tx interrupt */
		if (status & (SPE_DMA_ISR_TXOK | SPE_DMA_ISR_TXDU|SPE_DMA_ISR_TXERR))
		{
			if(tp->dmsg)
			{
				if(status & SPE_DMA_ISR_TXERR)		printk("TXERR !\n");
				if(status & SPE_DMA_ISR_TXDU)			printk("TXDU !\n");
			}
		#ifdef TX_TASKLET
			imr &= ~SPE_DMA_IMR_TXDU;
			imr &= ~SPE_DMA_IMR_TXERR;
			imr &= ~SPE_DMA_IMR_TXOK;
			tasklet_schedule(&tp->tx_tasklet);
		#else
			rtl8198_tx_interrupt(dev, tp, ioaddr);
			//rtl8198_tx_interrupt((unsigned long)tp);
		#endif			
			
		}
		
		if (status & SPE_DMA_ISR_SWINT)
		{	rtl8198_software_interrupt(dev, tp, ioaddr);
		}


		if(status & SPE_DMA_ISR_CUSTUSEALL)
		{
			rtl8198_system_ctrlstatus_interrupt(dev, tp, ioaddr,status& SPE_DMA_ISR_CUSTUSEALL);
		}	
		
		boguscnt--;
	} 
	while (boguscnt > 0);

	if (boguscnt <= 0) 
	{
		if (netif_msg_intr(tp) && net_ratelimit() ) 
		{
			printk(KERN_WARNING	       "%s: Too much work at interrupt!\n", dev->name);
		}
		/* Clear all interrupt sources. */
		REG32_W(SPE_DMA_ISR, 0xffffffff);
	}

out:
#ifndef RX_TASKLET
	REG32_W(SPE_DMA_IMR, tp->intr_mask);
	//REG32_W(SPE_DMA_IMR, imr);
#endif
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
	return IRQ_RETVAL(handled);
#else
	return;
#endif


}
//------------------------------------------------------------------------------------
static void rtl8198_down(struct net_device *dev)
{
	printk("rtl8198_down \n"); //wei add
	
	struct rtl8198_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
	unsigned int poll_locked = 0;

#if 0 //def CONFIG_R8198EP_HOST
//	rtl8198_dsm(dev, DSM_IF_DOWN);

//	rtl8198_powerdown_pll(dev);
#endif

#ifdef RX_TASKLET
                tasklet_kill(&tp->rx_tasklet);
#endif

#ifdef TX_TASKLET
                tasklet_kill(&tp->tx_tasklet);
#endif


	netif_stop_queue(dev);

//	rtl8198_delete_esd_timer(dev, &tp->esd_timer);
//	rtl8198_delete_link_timer(dev, &tp->link_timer);

	flush_scheduled_work();
#if 0 
#ifdef  CONFIG_R8198EP_HOST  // 0 //wei add
#ifdef CONFIG_R8168_NAPI
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,23)
	napi_disable(&tp->napi);
#endif
#endif//CONFIG_R8168_NAPI
#endif

core_down:
	spin_lock_irq(&tp->lock);
	rtl8198_asic_down(dev);
	spin_unlock_irq(&tp->lock);

	synchronize_irq(dev->irq);

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,23)) && (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0))
	if (!poll_locked) 
	{
		netif_poll_disable(dev);
		poll_locked++;
	}
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,11)
	/* Give a racing hard_start_xmit a few cycles to complete. */
	synchronize_sched();  /* FIXME: should this be synchronize_irq()? */
#endif

	/*
	 * And now for the 50k$ question: are IRQ disabled or not ?
	 *
	 * Two paths lead here:
	 * 1) dev->close
	 *    -> netif_running() is available to sync the current code and the
	 *       IRQ handler. See rtl8198_interrupt for details.
	 * 2) dev->change_mtu
	 *    -> rtl8198_poll can not be issued again and re-enable the
	 *       interruptions. Let's simply issue the IRQ down sequence again.
	 */
//	if (RTL_R16(IntrMask))
//		goto core_down;
#endif
	rtl8198_tx_clear(tp);
	rtl8198_rx_clear(tp);
}
//=================================================================
static int rtl8198_close(struct net_device *dev)
{
	printk("rtl8198_close \n"); //wei add

	struct rtl8198_private *tp = netdev_priv(dev);
#ifdef CONFIG_R8198EP_HOST	
	struct pci_dev *pdev = tp->pci_dev;
#endif


	rtl8198_down(dev);	
	free_irq(dev->irq, dev);

	
#ifdef CONFIG_R8198EP_HOST	
	pci_free_consistent(pdev, R8168_RX_RING_BYTES, tp->RxDescArray,   tp->RxPhyAddr);
	pci_free_consistent(pdev, R8168_TX_RING_BYTES, tp->TxDescArray,    tp->TxPhyAddr);
#endif
#ifdef CONFIG_R8198EP_DEVICE
	//kfree( tp->RxDescArray);
	//kfree( tp->TxDescArray);
#endif
	tp->TxDescArray = NULL;
	tp->RxDescArray = NULL;

	return 0;
}
//--------------------------------------------------------------------------------
static void rtl8198_set_rx_mode(struct net_device *dev)
{
	struct rtl8198_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;


	//spin_lock_irqsave(&tp->lock, flags);

	
	//spin_unlock_irqrestore(&tp->lock, flags);
}
//--------------------------------------------------------------------------------
/**
 *  rtl8198_get_stats - Get rtl8198 read/write statistics
 *  @dev: The Ethernet Device to get statistics for
 *
 *  Get TX/RX statistics for rtl8198
 */
static struct net_device_stats *rtl8198_get_stats(struct net_device *dev)
{
	struct rtl8198_private *tp = netdev_priv(dev);
	unsigned long flags;
//	printk("=>get stats \n"); //wei add
#if 1 //wei add	
	if (netif_running(dev)) 
	{
		spin_lock_irqsave(&tp->lock, flags);
		spin_unlock_irqrestore(&tp->lock, flags);
	}
#endif		
	return &RTLDEV->stats;
}
//-------------------------------------------------------------------------------
#ifdef CONFIG_PM

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11)
static int rtl8198_suspend(struct pci_dev *pdev, u32 state)
#else
static int 
rtl8198_suspend(struct pci_dev *pdev, 		pm_message_t state)
#endif
{
	struct net_device *dev = pci_get_drvdata(pdev);
	struct rtl8198_private *tp = netdev_priv(dev);
	void __iomem *ioaddr = tp->mmio_addr;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
	u32 pci_pm_state = pci_choose_state(pdev, state);
#endif

	if (!netif_running(dev))
		goto out;



	rtl8198_powerdown_pll(dev);

	netif_device_detach(dev);
	netif_stop_queue(dev);

	spin_lock_irq(&tp->lock);

	rtl8198_asic_down(dev);

	spin_unlock_irq(&tp->lock);

out:

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
	pci_save_state(pdev, &pci_pm_state);
#else
	pci_save_state(pdev);
#endif
	pci_enable_wake(pdev, pci_choose_state(pdev, state), tp->wol_enabled);
	pci_set_power_state(pdev, pci_choose_state(pdev, state));

	return 0;
}
//------------------------------------------------------------------
static int rtl8198_resume(struct pci_dev *pdev)
{
	struct net_device *dev = pci_get_drvdata(pdev);
	struct rtl8168_private *tp = netdev_priv(dev);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
	u32 pci_pm_state = PCI_D0;
#endif

	pci_set_power_state(pdev, PCI_D0);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
	pci_restore_state(pdev, &pci_pm_state);
#else
	pci_restore_state(pdev);
#endif
	pci_enable_wake(pdev, PCI_D0, 0);

	if (!netif_running(dev))
		goto out;



	netif_device_attach(dev);

	rtl8198_schedule_work(dev, rtl8198_reset_task);

	rtl8198_powerup_pll(dev);
out:
	return 0;
}

#endif /* CONFIG_PM */
//------------------------------------------------------------------
static struct pci_driver rtl8198_pci_driver = {
	.name		= MODULENAME,
	.id_table	= rtl8198_pci_tbl,
	.probe		= rtl8198_init_one,
	.remove		= __devexit_p(rtl8198_remove_one),
#if 0 //wei add	
#ifdef CONFIG_PM
	.suspend	= rtl8198_suspend,
	.resume		= rtl8198_resume,
#endif
#endif
};
//------------------------------------------------------------------
static int __init rtl8198_init_module(void)
{
#ifdef CONFIG_R8198EP_HOST
	printk( KERN_INFO "\n\n===================\nINIT Module \n");
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
	return pci_register_driver(&rtl8198_pci_driver);
#else
	return pci_module_init(&rtl8198_pci_driver);
#endif
#endif


#ifdef CONFIG_R8198EP_DEVICE
	rtl8198_init_one(NULL,NULL);
#endif
}
//-------------------------------------------------------------------
static void __exit rtl8198_cleanup_module(void)
{
	printk( KERN_INFO "CLEANUP Module \n");
#ifdef CONFIG_R8198EP_HOST
	pci_unregister_driver(&rtl8198_pci_driver);
#endif 	


#ifdef CONFIG_R8198EP_DEVICE
	rtl8198_remove_one(NULL);
#endif


}
//------------------------------------------------------------------
module_init(rtl8198_init_module);
module_exit(rtl8198_cleanup_module);

//=====================================================================


