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
 * This product is covered by one or more of the following patents:
 * US5,307,459, US5,434,872, US5,732,094, US6,570,884, US6,115,776, and US6,327,625.
 */


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
#define CHECKSUM_PARTIAL CHECKSUM_HW
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
 #define irqreturn_t void
 #define IRQ_HANDLED	1
 #define IRQ_NONE	0
 #define IRQ_RETVAL(x)	
#endif
#ifndef HAVE_FREE_NETDEV
#define free_netdev(x)	kfree(x)
#endif

#ifndef SET_NETDEV_DEV
#define SET_NETDEV_DEV(net, pdev)
#endif

#ifndef SET_MODULE_OWNER
#define SET_MODULE_OWNER(dev)
#endif

#ifndef SA_SHIRQ
#define SA_SHIRQ IRQF_SHARED
#endif

#ifndef NETIF_F_GSO
#define gso_size	tso_size
#define gso_segs	tso_segs
#endif


//#define TX_TASKLET
//#define RX_TASKLET

//Due to the hardware design of RTL8111B, the low 32 bit address of receive
//buffer must be 8-byte alignment.
#undef NET_IP_ALIGN

#ifdef CONFIG_R8198EP_HOST
#define NET_IP_ALIGN 0 //2   //wei add
#else
#define NET_IP_ALIGN 4 //wei add
#endif


#define WEI_DEBUG 1
#define RTL8198_DEBUG 1
//#define DEVICE_USING_FIXBUF 1


#define RTL8168_VERSION "0.1" //wei add
#define MODULENAME "r8198"  //wei add
#define PFX MODULENAME ": "

#ifdef RTL8198_DEBUG
#define assert(expr) \
        if(!(expr)) {					\
	        printk( "Assertion failed! %s,%s,%s,line=%d\n",	\
        	#expr,__FILE__,__FUNCTION__,__LINE__);		\
        }
#define dprintk(fmt, args...)	do { printk(PFX fmt, ## args); } while (0)
#else
#define assert(expr) do {} while (0)
#define dprintk(fmt, args...)	do {} while (0)
#endif /* RTL8168_DEBUG */

#define R8168_MSG_DEFAULT \
	(NETIF_MSG_DRV | NETIF_MSG_PROBE | NETIF_MSG_IFUP | NETIF_MSG_IFDOWN)

#define TX_BUFFS_AVAIL(tp) \
	(tp->dirty_tx + NUM_TX_DESC - tp->cur_tx - 1)



#define rtl8198_rx_hwaccel_skb		vlan_hwaccel_rx
//#define rtl8198_rx_quota(count, quota)	count


/* MAC address length */
#ifndef MAC_ADDR_LEN
#define MAC_ADDR_LEN	6
#endif

#ifndef MAC_PROTOCOL_LEN
#define MAC_PROTOCOL_LEN	2
#endif

#define Reserved2_data	7
#define RX_DMA_BURST	7	/* Maximum PCI burst, '6' is 1024 */
#define TX_DMA_BURST_unlimited	7
#define TX_DMA_BURST_1024	6
#define TX_DMA_BURST_512	5
#define TX_DMA_BURST_256	4
#define TX_DMA_BURST_128	3
#define TX_DMA_BURST_64		2
#define TX_DMA_BURST_32		1
#define TX_DMA_BURST_16		0
#define Reserved1_data 	0x3F
#define RxPacketMaxSize	0x3FE8	/* 16K - 1 - ETH_HLEN - VLAN - CRC... */
#define Jumbo_Frame_2k	(2 * 1024)
#define Jumbo_Frame_3k	(3 * 1024)
#define Jumbo_Frame_4k	(4 * 1024)
#define Jumbo_Frame_5k	(5 * 1024)
#define Jumbo_Frame_6k	(6 * 1024)
#define Jumbo_Frame_7k	(7 * 1024)
#define Jumbo_Frame_8k	(8 * 1024)
#define Jumbo_Frame_9k	(9 * 1024)
#define InterFrameGap	0x03	/* 3 means InterFrameGap = the shortest one */

#define R8168_REGS_SIZE		256

#define NUM_TX_DESC	8	/* Number of Tx descriptor registers */
#define NUM_RX_DESC	8	/* Number of Rx descriptor registers */
#define RX_BUF_SIZE	0x2000		/* Rx Buffer size */
#define TX_BUF_SIZE	0x2000		/* Tx Buffer size */  //wei add


#define R8168_TX_RING_BYTES	(NUM_TX_DESC * sizeof(struct TxDesc))
#define R8168_RX_RING_BYTES	(NUM_RX_DESC * sizeof(struct RxDesc))

//#define RTL8198_TX_TIMEOUT	(6 * HZ)
#define RTL8198_TX_TIMEOUT	(10 * HZ)
#define RTL8168_LINK_TIMEOUT	(1 * HZ)
#define RTL8168_ESD_TIMEOUT	(2 * HZ)



#define NODE_ADDRESS_SIZE 6

/* write/read MMIO register */
#define RTL_W8(reg, val8)	writeb ((val8), ioaddr + (reg))
#define RTL_W16(reg, val16)	writew ((val16), ioaddr + (reg))
#define RTL_W32(reg, val32)	writel ((val32), ioaddr + (reg))
#define RTL_R8(reg)		readb (ioaddr + (reg))
#define RTL_R16(reg)		readw (ioaddr + (reg))
#define RTL_R32(reg)		((unsigned long) readl (ioaddr + (reg)))


//wei add
#define REG8_W(reg, val8)	writeb ((val8), ioaddr + (reg))
#define REG16_W(reg, val16)	writew ((val16), ioaddr + (reg))
#define REG32_W(reg, val32)	writel ((val32), ioaddr + (reg))
#define REG8_R(reg)		readb (ioaddr + (reg))
#define REG16_R(reg)		readw (ioaddr + (reg))
#define REG32_R(reg)		((u32) readl (ioaddr + (reg)))





#ifndef	DMA_64BIT_MASK
#define DMA_64BIT_MASK	0xffffffffffffffffULL
#endif

#ifndef	DMA_32BIT_MASK
#define DMA_32BIT_MASK	0x00000000ffffffffULL
#endif

#ifndef	NETDEV_TX_OK
#define NETDEV_TX_OK 0		/* driver took care of packet */
#endif

#ifndef	NETDEV_TX_BUSY
#define NETDEV_TX_BUSY 1	/* driver tx path was busy*/
#endif

#ifndef	NETDEV_TX_LOCKED
#define NETDEV_TX_LOCKED -1	/* driver tx lock was already taken */
#endif

#ifndef	ADVERTISED_Pause
#define ADVERTISED_Pause	(1 << 13)
#endif

#ifndef	ADVERTISED_Asym_Pause
#define ADVERTISED_Asym_Pause	(1 << 14)
#endif

#ifndef	ADVERTISE_PAUSE_CAP
#define ADVERTISE_PAUSE_CAP	0x400
#endif

#ifndef	ADVERTISE_PAUSE_ASYM
#define ADVERTISE_PAUSE_ASYM	0x800
#endif

#ifndef	MII_CTRL1000
#define MII_CTRL1000		0x09
#endif

#ifndef	ADVERTISE_1000FULL
#define ADVERTISE_1000FULL	0x200
#endif

#ifndef	ADVERTISE_1000HALF
#define ADVERTISE_1000HALF	0x100
#endif

/*****************************************************************************/
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)
	#define	RTLDEV	tp
#else
	#define	RTLDEV	dev
#endif	//LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)
/*****************************************************************************/
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
	typedef struct net_device *napi_ptr;
	typedef int *napi_budget;

	#define napi dev
	#define RTL_NAPI_CONFIG(ndev, priv, function, weig)	ndev->poll=function;	\
								ndev->weight=weig;
	#define RTL_NAPI_QUOTA(budget, ndev)			min(*budget, ndev->quota)
	#define RTL_GET_PRIV(stuct_ptr, priv_struct)		netdev_priv(stuct_ptr)
	#define RTL_GET_NETDEV(priv_ptr)			
	#define RTL_RX_QUOTA(ndev, budget)			ndev->quota
	#define RTL_NAPI_QUOTA_UPDATE(ndev, work_done, budget)	*budget -= work_done;	\
								ndev->quota -= work_done;
	#define RTL_NETIF_RX_COMPLETE(dev, napi)		netif_rx_complete(dev)
	#define RTL_NETIF_RX_SCHEDULE_PREP(dev, napi)		netif_rx_schedule_prep(dev)
	#define __RTL_NETIF_RX_SCHEDULE(dev, napi)		__netif_rx_schedule(dev)
	#define RTL_NAPI_RETURN_VALUE				work_done >= work_to_do
	#define RTL_NAPI_ENABLE(dev, napi)			netif_poll_enable(dev)
	#define RTL_NAPI_DISABLE(dev, napi)			netif_poll_disable(dev)
#else
	typedef struct napi_struct *napi_ptr;
	typedef int napi_budget;

	#define RTL_NAPI_CONFIG(ndev, priv, function, weight)	netif_napi_add(ndev, &priv->napi, function, weight)
	#define RTL_NAPI_QUOTA(budget, ndev)			min(budget, budget)
	#define RTL_GET_PRIV(stuct_ptr, priv_struct)		container_of(stuct_ptr, priv_struct, stuct_ptr)
	#define RTL_GET_NETDEV(priv_ptr)			struct net_device *dev = priv_ptr->dev;
	#define RTL_RX_QUOTA(ndev, budget)			budget
	#define RTL_NAPI_QUOTA_UPDATE(ndev, work_done, budget)
	#define RTL_NETIF_RX_COMPLETE(dev, napi)		netif_rx_complete(dev, napi)
	#define RTL_NETIF_RX_SCHEDULE_PREP(dev, napi)		netif_rx_schedule_prep(dev, napi)
	#define __RTL_NETIF_RX_SCHEDULE(dev, napi)		__netif_rx_schedule(dev, napi)
	#define RTL_NAPI_RETURN_VALUE work_done
	#define RTL_NAPI_ENABLE(dev, napi)			napi_enable(napi)
	#define RTL_NAPI_DISABLE(dev, napi)			napi_disable(napi)
#endif	//LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
/*****************************************************************************/
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9)
#ifdef __CHECKER__
#define __iomem	__attribute__((noderef, address_space(2)))
extern void __chk_io_ptr(void __iomem *);
#define __bitwise __attribute__((bitwise))
#else
#define __iomem
#define __chk_io_ptr(x) (void)0
#define __bitwise
#endif
#endif	//LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9)

/*****************************************************************************/
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,8)
#ifdef __CHECKER__
#define __force	__attribute__((force))
#else
#define __force
#endif
#endif	//LINUX_VERSION_CODE < KERNEL_VERSION(2,6,8)

#ifndef module_param
#define module_param(v,t,p) MODULE_PARM(v, "i");
#endif

#ifndef PCI_DEVICE
#define PCI_DEVICE(vend,dev) \
	.vendor = (vend), .device = (dev), \
	.subvendor = PCI_ANY_ID, .subdevice = PCI_ANY_ID
#endif

/*****************************************************************************/
/* 2.5.28 => 2.4.23 */
#if ( LINUX_VERSION_CODE < KERNEL_VERSION(2,5,28) )

static inline void _kc_synchronize_irq(void)
{
	synchronize_irq();
}
#undef synchronize_irq
#define synchronize_irq(X) _kc_synchronize_irq()

#include <linux/tqueue.h>
#define work_struct tq_struct
#undef INIT_WORK
#define INIT_WORK(a,b,c) INIT_TQUEUE(a,(void (*)(void *))b,c)
#undef container_of
#define container_of list_entry
#define schedule_work schedule_task
#define flush_scheduled_work flush_scheduled_tasks
#endif /* 2.5.28 => 2.4.17 */

/*****************************************************************************/
/* 2.6.4 => 2.6.0 */
#if ( LINUX_VERSION_CODE < KERNEL_VERSION(2,6,4) )
#define MODULE_VERSION(_version) MODULE_INFO(version, _version)
#endif /* 2.6.4 => 2.6.0 */
/*****************************************************************************/
/* 2.6.0 => 2.5.28 */
#if ( LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0) )
#define MODULE_INFO(version, _version)
#ifndef CONFIG_E1000_DISABLE_PACKET_SPLIT
#define CONFIG_E1000_DISABLE_PACKET_SPLIT 1
#endif

#define pci_set_consistent_dma_mask(dev,mask) 1

#undef dev_put
#define dev_put(dev) __dev_put(dev)

#ifndef skb_fill_page_desc
#define skb_fill_page_desc _kc_skb_fill_page_desc
extern void _kc_skb_fill_page_desc(struct sk_buff *skb, int i, struct page *page, int off, int size);
#endif

#ifndef pci_dma_mapping_error
#define pci_dma_mapping_error _kc_pci_dma_mapping_error
static inline int _kc_pci_dma_mapping_error(dma_addr_t dma_addr)
{
	return dma_addr == 0;
}
#endif

#undef ALIGN
#define ALIGN(x,a) (((x)+(a)-1)&~((a)-1))

#endif /* 2.6.0 => 2.5.28 */

/*****************************************************************************/
/* 2.4.22 => 2.4.17 */
#if ( LINUX_VERSION_CODE < KERNEL_VERSION(2,4,22) )
#define pci_name(x)	((x)->slot_name)
#endif /* 2.4.22 => 2.4.17 */

/*****************************************************************************/
/* 2.6.5 => 2.6.0 */
#if ( LINUX_VERSION_CODE < KERNEL_VERSION(2,6,5) )
#define pci_dma_sync_single_for_cpu	pci_dma_sync_single
#define pci_dma_sync_single_for_device	pci_dma_sync_single_for_cpu
#endif /* 2.6.5 => 2.6.0 */

/*****************************************************************************/

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
/*
 * initialize a work-struct's func and data pointers:
 */
#define PREPARE_WORK(_work, _func, _data)			\
	do {							\
		(_work)->func = _func;				\
		(_work)->data = _data;				\
	} while (0)

#endif
/*****************************************************************************/
/* 2.6.4 => 2.6.0 */
#if ( LINUX_VERSION_CODE < KERNEL_VERSION(2,4,25) || \
    ( LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) && \
      LINUX_VERSION_CODE < KERNEL_VERSION(2,6,4) ) )
#define ETHTOOL_OPS_COMPAT
#endif /* 2.6.4 => 2.6.0 */

/*****************************************************************************/

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
#undef ethtool_ops
#define ethtool_ops _kc_ethtool_ops

struct _kc_ethtool_ops {
	int  (*get_settings)(struct net_device *, struct ethtool_cmd *);
	int  (*set_settings)(struct net_device *, struct ethtool_cmd *);
	void (*get_drvinfo)(struct net_device *, struct ethtool_drvinfo *);
	int  (*get_regs_len)(struct net_device *);
	void (*get_regs)(struct net_device *, struct ethtool_regs *, void *);
	void (*get_wol)(struct net_device *, struct ethtool_wolinfo *);
	int  (*set_wol)(struct net_device *, struct ethtool_wolinfo *);
	u32  (*get_msglevel)(struct net_device *);
	void (*set_msglevel)(struct net_device *, u32);
	int  (*nway_reset)(struct net_device *);
	u32  (*get_link)(struct net_device *);
	int  (*get_eeprom_len)(struct net_device *);
	int  (*get_eeprom)(struct net_device *, struct ethtool_eeprom *, u8 *);
	int  (*set_eeprom)(struct net_device *, struct ethtool_eeprom *, u8 *);
	int  (*get_coalesce)(struct net_device *, struct ethtool_coalesce *);
	int  (*set_coalesce)(struct net_device *, struct ethtool_coalesce *);
	void (*get_ringparam)(struct net_device *, struct ethtool_ringparam *);
	int  (*set_ringparam)(struct net_device *, struct ethtool_ringparam *);
	void (*get_pauseparam)(struct net_device *,
	                       struct ethtool_pauseparam*);
	int  (*set_pauseparam)(struct net_device *,
	                       struct ethtool_pauseparam*);
	u32  (*get_rx_csum)(struct net_device *);
	int  (*set_rx_csum)(struct net_device *, u32);
	u32  (*get_tx_csum)(struct net_device *);
	int  (*set_tx_csum)(struct net_device *, u32);
	u32  (*get_sg)(struct net_device *);
	int  (*set_sg)(struct net_device *, u32);
	u32  (*get_tso)(struct net_device *);
	int  (*set_tso)(struct net_device *, u32);
	int  (*self_test_count)(struct net_device *);
	void (*self_test)(struct net_device *, struct ethtool_test *, u64 *);
	void (*get_strings)(struct net_device *, u32 stringset, u8 *);
	int  (*phys_id)(struct net_device *, u32);
	int  (*get_stats_count)(struct net_device *);
	void (*get_ethtool_stats)(struct net_device *, struct ethtool_stats *,
	                          u64 *);
} *ethtool_ops = NULL;

#undef SET_ETHTOOL_OPS
#define SET_ETHTOOL_OPS(netdev, ops) (ethtool_ops = (ops))

#endif //LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)

/*****************************************************************************/
/* Installations with ethtool version without eeprom, adapter id, or statistics
 * support */

#ifndef ETH_GSTRING_LEN
#define ETH_GSTRING_LEN 32
#endif

#ifndef ETHTOOL_GSTATS
#define ETHTOOL_GSTATS 0x1d
#undef ethtool_drvinfo
#define ethtool_drvinfo k_ethtool_drvinfo
struct k_ethtool_drvinfo {
	u32 cmd;
	char driver[32];
	char version[32];
	char fw_version[32];
	char bus_info[32];
	char reserved1[32];
	char reserved2[16];
	u32 n_stats;
	u32 testinfo_len;
	u32 eedump_len;
	u32 regdump_len;
};

struct ethtool_stats {
	u32 cmd;
	u32 n_stats;
	u64 data[0];
};
#endif /* ETHTOOL_GSTATS */

#ifndef ETHTOOL_PHYS_ID
#define ETHTOOL_PHYS_ID 0x1c
#endif /* ETHTOOL_PHYS_ID */

#ifndef ETHTOOL_GSTRINGS
#define ETHTOOL_GSTRINGS 0x1b
enum ethtool_stringset {
	ETH_SS_TEST             = 0,
	ETH_SS_STATS,
};
struct ethtool_gstrings {
	u32 cmd;            /* ETHTOOL_GSTRINGS */
	u32 string_set;     /* string set id e.c. ETH_SS_TEST, etc*/
	u32 len;            /* number of strings in the string set */
	u8 data[0];
};
#endif /* ETHTOOL_GSTRINGS */

#ifndef ETHTOOL_TEST
#define ETHTOOL_TEST 0x1a
enum ethtool_test_flags {
	ETH_TEST_FL_OFFLINE	= (1 << 0),
	ETH_TEST_FL_FAILED	= (1 << 1),
};
struct ethtool_test {
	u32 cmd;
	u32 flags;
	u32 reserved;
	u32 len;
	u64 data[0];
};
#endif /* ETHTOOL_TEST */

#ifndef ETHTOOL_GEEPROM
#define ETHTOOL_GEEPROM 0xb
#undef ETHTOOL_GREGS
struct ethtool_eeprom {
	u32 cmd;
	u32 magic;
	u32 offset;
	u32 len;
	u8 data[0];
};

struct ethtool_value {
	u32 cmd;
	u32 data;
};
#endif /* ETHTOOL_GEEPROM */

#ifndef ETHTOOL_GLINK
#define ETHTOOL_GLINK 0xa
#endif /* ETHTOOL_GLINK */

#ifndef ETHTOOL_GREGS
#define ETHTOOL_GREGS		0x00000004 /* Get NIC registers */
#define ethtool_regs _kc_ethtool_regs
/* for passing big chunks of data */
struct _kc_ethtool_regs {
	u32 cmd;
	u32 version; /* driver-specific, indicates different chips/revs */
	u32 len; /* bytes */
	u8 data[0];
};
#endif /* ETHTOOL_GREGS */

#ifndef ETHTOOL_GMSGLVL
#define ETHTOOL_GMSGLVL		0x00000007 /* Get driver message level */
#endif
#ifndef ETHTOOL_SMSGLVL
#define ETHTOOL_SMSGLVL		0x00000008 /* Set driver msg level, priv. */
#endif
#ifndef ETHTOOL_NWAY_RST
#define ETHTOOL_NWAY_RST	0x00000009 /* Restart autonegotiation, priv */
#endif
#ifndef ETHTOOL_GLINK
#define ETHTOOL_GLINK		0x0000000a /* Get link status */
#endif
#ifndef ETHTOOL_GEEPROM
#define ETHTOOL_GEEPROM		0x0000000b /* Get EEPROM data */
#endif
#ifndef ETHTOOL_SEEPROM
#define ETHTOOL_SEEPROM		0x0000000c /* Set EEPROM data */
#endif
#ifndef ETHTOOL_GCOALESCE
#define ETHTOOL_GCOALESCE	0x0000000e /* Get coalesce config */
/* for configuring coalescing parameters of chip */
#define ethtool_coalesce _kc_ethtool_coalesce
struct _kc_ethtool_coalesce {
	u32	cmd;	/* ETHTOOL_{G,S}COALESCE */

	/* How many usecs to delay an RX interrupt after
	 * a packet arrives.  If 0, only rx_max_coalesced_frames
	 * is used.
	 */
	u32	rx_coalesce_usecs;

	/* How many packets to delay an RX interrupt after
	 * a packet arrives.  If 0, only rx_coalesce_usecs is
	 * used.  It is illegal to set both usecs and max frames
	 * to zero as this would cause RX interrupts to never be
	 * generated.
	 */
	u32	rx_max_coalesced_frames;

	/* Same as above two parameters, except that these values
	 * apply while an IRQ is being serviced by the host.  Not
	 * all cards support this feature and the values are ignored
	 * in that case.
	 */
	u32	rx_coalesce_usecs_irq;
	u32	rx_max_coalesced_frames_irq;

	/* How many usecs to delay a TX interrupt after
	 * a packet is sent.  If 0, only tx_max_coalesced_frames
	 * is used.
	 */
	u32	tx_coalesce_usecs;

	/* How many packets to delay a TX interrupt after
	 * a packet is sent.  If 0, only tx_coalesce_usecs is
	 * used.  It is illegal to set both usecs and max frames
	 * to zero as this would cause TX interrupts to never be
	 * generated.
	 */
	u32	tx_max_coalesced_frames;

	/* Same as above two parameters, except that these values
	 * apply while an IRQ is being serviced by the host.  Not
	 * all cards support this feature and the values are ignored
	 * in that case.
	 */
	u32	tx_coalesce_usecs_irq;
	u32	tx_max_coalesced_frames_irq;

	/* How many usecs to delay in-memory statistics
	 * block updates.  Some drivers do not have an in-memory
	 * statistic block, and in such cases this value is ignored.
	 * This value must not be zero.
	 */
	u32	stats_block_coalesce_usecs;

	/* Adaptive RX/TX coalescing is an algorithm implemented by
	 * some drivers to improve latency under low packet rates and
	 * improve throughput under high packet rates.  Some drivers
	 * only implement one of RX or TX adaptive coalescing.  Anything
	 * not implemented by the driver causes these values to be
	 * silently ignored.
	 */
	u32	use_adaptive_rx_coalesce;
	u32	use_adaptive_tx_coalesce;

	/* When the packet rate (measured in packets per second)
	 * is below pkt_rate_low, the {rx,tx}_*_low parameters are
	 * used.
	 */
	u32	pkt_rate_low;
	u32	rx_coalesce_usecs_low;
	u32	rx_max_coalesced_frames_low;
	u32	tx_coalesce_usecs_low;
	u32	tx_max_coalesced_frames_low;

	/* When the packet rate is below pkt_rate_high but above
	 * pkt_rate_low (both measured in packets per second) the
	 * normal {rx,tx}_* coalescing parameters are used.
	 */

	/* When the packet rate is (measured in packets per second)
	 * is above pkt_rate_high, the {rx,tx}_*_high parameters are
	 * used.
	 */
	u32	pkt_rate_high;
	u32	rx_coalesce_usecs_high;
	u32	rx_max_coalesced_frames_high;
	u32	tx_coalesce_usecs_high;
	u32	tx_max_coalesced_frames_high;

	/* How often to do adaptive coalescing packet rate sampling,
	 * measured in seconds.  Must not be zero.
	 */
	u32	rate_sample_interval;
};
#endif /* ETHTOOL_GCOALESCE */

#ifndef ETHTOOL_SCOALESCE
#define ETHTOOL_SCOALESCE	0x0000000f /* Set coalesce config. */
#endif
#ifndef ETHTOOL_GRINGPARAM
#define ETHTOOL_GRINGPARAM	0x00000010 /* Get ring parameters */
/* for configuring RX/TX ring parameters */
#define ethtool_ringparam _kc_ethtool_ringparam
struct _kc_ethtool_ringparam {
	u32	cmd;	/* ETHTOOL_{G,S}RINGPARAM */

	/* Read only attributes.  These indicate the maximum number
	 * of pending RX/TX ring entries the driver will allow the
	 * user to set.
	 */
	u32	rx_max_pending;
	u32	rx_mini_max_pending;
	u32	rx_jumbo_max_pending;
	u32	tx_max_pending;

	/* Values changeable by the user.  The valid values are
	 * in the range 1 to the "*_max_pending" counterpart above.
	 */
	u32	rx_pending;
	u32	rx_mini_pending;
	u32	rx_jumbo_pending;
	u32	tx_pending;
};
#endif /* ETHTOOL_GRINGPARAM */

#ifndef ETHTOOL_SRINGPARAM
#define ETHTOOL_SRINGPARAM	0x00000011 /* Set ring parameters, priv. */
#endif
#ifndef ETHTOOL_GPAUSEPARAM
#define ETHTOOL_GPAUSEPARAM	0x00000012 /* Get pause parameters */
/* for configuring link flow control parameters */
#define ethtool_pauseparam _kc_ethtool_pauseparam
struct _kc_ethtool_pauseparam {
	u32	cmd;	/* ETHTOOL_{G,S}PAUSEPARAM */

	/* If the link is being auto-negotiated (via ethtool_cmd.autoneg
	 * being true) the user may set 'autonet' here non-zero to have the
	 * pause parameters be auto-negotiated too.  In such a case, the
	 * {rx,tx}_pause values below determine what capabilities are
	 * advertised.
	 *
	 * If 'autoneg' is zero or the link is not being auto-negotiated,
	 * then {rx,tx}_pause force the driver to use/not-use pause
	 * flow control.
	 */
	u32	autoneg;
	u32	rx_pause;
	u32	tx_pause;
};
#endif /* ETHTOOL_GPAUSEPARAM */

#ifndef ETHTOOL_SPAUSEPARAM
#define ETHTOOL_SPAUSEPARAM	0x00000013 /* Set pause parameters. */
#endif
#ifndef ETHTOOL_GRXCSUM
#define ETHTOOL_GRXCSUM		0x00000014 /* Get RX hw csum enable (ethtool_value) */
#endif
#ifndef ETHTOOL_SRXCSUM
#define ETHTOOL_SRXCSUM		0x00000015 /* Set RX hw csum enable (ethtool_value) */
#endif
#ifndef ETHTOOL_GTXCSUM
#define ETHTOOL_GTXCSUM		0x00000016 /* Get TX hw csum enable (ethtool_value) */
#endif
#ifndef ETHTOOL_STXCSUM
#define ETHTOOL_STXCSUM		0x00000017 /* Set TX hw csum enable (ethtool_value) */
#endif
#ifndef ETHTOOL_GSG
#define ETHTOOL_GSG		0x00000018 /* Get scatter-gather enable
					    * (ethtool_value) */
#endif
#ifndef ETHTOOL_SSG
#define ETHTOOL_SSG		0x00000019 /* Set scatter-gather enable
					    * (ethtool_value). */
#endif
#ifndef ETHTOOL_TEST
#define ETHTOOL_TEST		0x0000001a /* execute NIC self-test, priv. */
#endif
#ifndef ETHTOOL_GSTRINGS
#define ETHTOOL_GSTRINGS	0x0000001b /* get specified string set */
#endif
#ifndef ETHTOOL_PHYS_ID
#define ETHTOOL_PHYS_ID		0x0000001c /* identify the NIC */
#endif
#ifndef ETHTOOL_GSTATS
#define ETHTOOL_GSTATS		0x0000001d /* get NIC-specific statistics */
#endif
#ifndef ETHTOOL_GTSO
#define ETHTOOL_GTSO		0x0000001e /* Get TSO enable (ethtool_value) */
#endif
#ifndef ETHTOOL_STSO
#define ETHTOOL_STSO		0x0000001f /* Set TSO enable (ethtool_value) */
#endif

#ifndef ETHTOOL_BUSINFO_LEN
#define ETHTOOL_BUSINFO_LEN	32
#endif

/*****************************************************************************/

//-------------------------------------------------

enum RTL8198_registers  //wei add
{
	SPE_DMA_TXFDP = 0x00,		
	SPE_DMA_TXCDO = 0x04,
	SPE_DMA_RXFDP = 0x08,		
	SPE_DMA_RXCDO = 0x0C,
	
	SPE_DMA_TXOKCNT = 0x10,
	SPE_DMA_RXOKCNT = 0x14,
	
	SPE_DMA_IOCMD = 0x20,
	
	SPE_DMA_IM = 0x24,
	SPE_DMA_IMR = 0x28,
	SPE_DMA_ISR = 0x2C,
	
	SPE_DMA_SIZE = 0x30,

	//Indirect Share Memory	
	SPE_ISM_LR = 0x40,
	SPE_ISM_OR = 0x44,  //Host site
	SPE_ISM_BAR = 0x44,	  //Device site
	SPE_ISM_DR = 0x48,

	//NFBI
	SPE_NFBI_CMD = 0x80,
	SPE_NFBI_ADDR =	0x84,
	SPE_NFBI_DR = 0x88,

	SPE_NFBI_SYSSR = 0x8c,
	SPE_NFBI_SYSCR = 0x90,

	//Memory
	SPE_NFBI_DCR = 0xA4,
	SPE_NFBI_DTR = 0xA8,
	SPE_NFBI_DDCR =	0xAC,
	SPE_NFBI_TRXDLY = 0xB0,


};


enum RTL8198_register_content {
	//IOCMD 
	 SPE_DMA_IOCMD_DSECSWAP = (1<<5),	
	 SPE_DMA_IOCMD_DATASWAP = (1<<4),
	 SPE_DMA_IOCMD_SWINT =(1<<3),
	 SPE_DMA_IOCMD_RXEN= (1<<2),
	 SPE_DMA_IOCMD_TXPOLL= (1<<1),
	 SPE_DMA_IOCMD_RST =(1<<0),


	//IMR
	 SPE_DMA_IMR_NEEDBTCODE =(1<<31),
	 SPE_DMA_IMR_SWINT= (1<<23),
	 SPE_DMA_IMR_TXDU =(1<<22),
	 SPE_DMA_IMR_TXERR =(1<<21),
	 SPE_DMA_IMR_TXOK= (1<<20),
	 SPE_DMA_IMR_TXTMR= (1<<19),
	 SPE_DMA_IMR_RXDU= (1<<18),
	 SPE_DMA_IMR_RXERR= (1<<17),
	 SPE_DMA_IMR_RXOK =(1<<17),

	SPE_DMA_IMR_DMAALL= (SPE_DMA_IMR_SWINT |	SPE_DMA_IMR_TXDU |SPE_DMA_IMR_TXERR| SPE_DMA_IMR_TXOK |SPE_DMA_IMR_TXTMR | \
											SPE_DMA_IMR_RXDU |SPE_DMA_IMR_RXERR| SPE_DMA_IMR_RXOK),
	//ISR
	 SPE_DMA_ISR_NEEDBTCODE= (1<<31),
	 SPE_DMA_ISR_SWINT =(1<<23),
	 SPE_DMA_ISR_TXDU =(1<<22),
	 SPE_DMA_ISR_TXERR =(1<<21),
	 SPE_DMA_ISR_TXOK =(1<<20),
	 SPE_DMA_ISR_TXTMR =(1<<19),
	 SPE_DMA_ISR_RXDU =(1<<18),
	 SPE_DMA_ISR_RXERR =(1<<17),
	 SPE_DMA_ISR_RXOK =(1<<16),

	SPE_DMA_ISR_CUSTUSE15=(1<<15),
	SPE_DMA_ISR_CUSTUSE14=(1<<14),
	SPE_DMA_ISR_CUSTUSE13=(1<<13),
	SPE_DMA_ISR_CUSTUSE12=(1<<12),
	SPE_DMA_ISR_CUSTUSE11=(1<<11),
	SPE_DMA_ISR_CUSTUSE10=(1<<10),
	SPE_DMA_ISR_CUSTUSE09=(1<<9),
	SPE_DMA_ISR_CUSTUSE08=(1<<8),
	SPE_DMA_ISR_CUSTUSE07=(1<<7),
	SPE_DMA_ISR_CUSTUSE06=(1<<6),
	SPE_DMA_ISR_CUSTUSE05=(1<<5),
	SPE_DMA_ISR_CUSTUSE04=(1<<4),
	SPE_DMA_ISR_CUSTUSE03=(1<<3),
	SPE_DMA_ISR_CUSTUSE02=(1<<2),	
	SPE_DMA_ISR_CUSTUSE01=(1<<1),
	SPE_DMA_ISR_CUSTUSE00=(1<<0),	

	SPE_DMA_ISR_CUSTUSEALL =(0xffff),
	SPE_DMA_ISR_DMAALL =(SPE_DMA_ISR_SWINT |	SPE_DMA_ISR_TXDU |SPE_DMA_ISR_TXERR| SPE_DMA_ISR_TXOK |SPE_DMA_ISR_TXTMR | \
											SPE_DMA_ISR_RXDU |SPE_DMA_ISR_RXERR| SPE_DMA_ISR_RXOK),



	//NFBI_CMD
	NFBI_CMD_DRAMTYPE_DDR =(1<<3),
	 NFBI_CMD_SWAP= (1<<2),
	 NFBI_CMD_SYSRST= (1<<1),	 

	//NFBI SYSCR
 	SPE_NFBI_CUSTUSE15 = (1<<15),
	SPE_NFBI_CUSTUSE14 = (1<<14),
	SPE_NFBI_CUSTUSE13 = (1<<13),
 	SPE_NFBI_CUSTUSE12 =(1<<12),
	SPE_NFBI_CUSTUSE11 = (1<<11),
	SPE_NFBI_CUSTUSE10 = (1<<10),
	SPE_NFBI_CUSTUSE09 = (1<<9),
	SPE_NFBI_CUSTUSE08 = (1<<8),
	SPE_NFBI_CUSTUSE07 = (1<<7),
	SPE_NFBI_CUSTUSE06 = (1<<6),
	SPE_NFBI_CUSTUSE05 = (1<<5),
	SPE_NFBI_CUSTUSE04 = (1<<4),
 	SPE_NFBI_CUSTUSE03 =(1<<3),
 	SPE_NFBI_CUSTUSE02 = (1<<2),
 	SPE_NFBI_CUSTUSE01 = (1<<1),
 	SPE_NFBI_CUSTUSE00 = (1<<0),

	//NFBI SYSSR
 	SPE_NFBI_FWUSR15 = (1<<15),
	SPE_NFBI_FWUSR14 = (1<<14),
	SPE_NFBI_FWUSR13 = (1<<13),
 	SPE_NFBI_FWUSR12 =(1<<12),
	SPE_NFBI_FWUSR11 = (1<<11),
	SPE_NFBI_FWUSR10 = (1<<10),
	SPE_NFBI_FWUSR09 = (1<<9),
	SPE_NFBI_FWUSR08 = (1<<8),
	SPE_NFBI_FWUSR07 = (1<<7),
	SPE_NFBI_FWUSR06 = (1<<6),
	SPE_NFBI_FWUSR05 = (1<<5),
	SPE_NFBI_FWUSR04 = (1<<4),
 	SPE_NFBI_FWUSR03 =(1<<3),
 	SPE_NFBI_FWUSR02 = (1<<2),
 	SPE_NFBI_FWUSR01 = (1<<1),
 	SPE_NFBI_FWUSR00 = (1<<0),
 	


};

enum _DescStatusBit {

	/* Tx private */
	/*------ offset 0 of tx descriptor ------*/	
	DescOwn		= (1 << 31), /* Descriptor is owned by NIC */
	RingEnd		= (1 << 30), /* End of descriptor ring */
	FirstFrag	= (1 << 29), /* First segment of a packet */
	LastFrag	= (1 << 28), /* Final segment of a packet */
	Info1_Offset = (16),	
	Info1_Mask = (0xfff<<16),

	/*------ offset 4 of tx descriptor ------*/
	/*------ offset 8 of tx descriptor ------*/	
	/*------ offset 12 of tx descriptor ------*/	


	/* Rx private */
	/*------ offset 0 of rx descriptor ------*/
	Drop		= (1<<29),
	Drop_Offset		= (29),




};

enum features {
//	RTL_FEATURE_WOL	= (1 << 0),
	RTL_FEATURE_MSI	= (1 << 1),
};

enum wol_capability {
	WOL_DISABLED = 0,
	WOL_ENABLED = 1
};

enum bits {
	BIT_0 = (1 << 0),
	BIT_1 = (1 << 1),
	BIT_2 = (1 << 2),
	BIT_3 = (1 << 3),
	BIT_4 = (1 << 4),
	BIT_5 = (1 << 5),
	BIT_6 = (1 << 6),
	BIT_7 = (1 << 7),
	BIT_8 = (1 << 8),
	BIT_9 = (1 << 9),
	BIT_10 = (1 << 10),
	BIT_11 = (1 << 11),
	BIT_12 = (1 << 12),
	BIT_13 = (1 << 13),
	BIT_14 = (1 << 14),
	BIT_15 = (1 << 15),
	BIT_16 = (1 << 16),
	BIT_17 = (1 << 17),
	BIT_18 = (1 << 18),
	BIT_19 = (1 << 19),
	BIT_20 = (1 << 20),
	BIT_21 = (1 << 21),
	BIT_22 = (1 << 22),
	BIT_23 = (1 << 23),
	BIT_24 = (1 << 24),
	BIT_25 = (1 << 25),
	BIT_26 = (1 << 26),
	BIT_27 = (1 << 27),
	BIT_28 = (1 << 28),
	BIT_29 = (1 << 29),
	BIT_30 = (1 << 30),
	BIT_31 = (1 << 31)
};

#define RsvdMask	0x3fff0000//0x3fffc000

struct TxDesc {


	//wei add
	u32 flag;
	u32 addr;
	u32 info2;
	u32 info3;
/*
	//old
	u32 opts1;
	u32 opts2;
	u64 addr;	
*/	
};

struct RxDesc {

	//wei add
	u32 flag;
	u32 addr;
	u32 info2;
	u32 info3;	
/*
	//old
	u32 opts1;
	u32 opts2;
	u64 addr;
*/	
	
};

struct ring_info {
	struct sk_buff	*skb;
	u32		len;
	u8		__pad[sizeof(void *) - sizeof(u32)];
};

struct pci_resource {
	u8	cmd;
	u8	cls;
	u16	io_base_h;
	u16	io_base_l;
	u16	mem_base_h;
	u16	mem_base_l;
	u8	ilr;
	u16	resv_0x20_h;
	u16	resv_0x20_l;
	u16	resv_0x24_h;
	u16	resv_0x24_l;
};

//wei add
struct slvpcie_rxbuff_t
{	unsigned char buff[RX_BUF_SIZE];  
};
struct slvpcie_txbuff_t
{	unsigned char buff[TX_BUF_SIZE];  
};

struct rtl8198_private {

#ifdef RX_TASKLET
    struct tasklet_struct   rx_tasklet;
#endif

#ifdef TX_TASKLET
    struct tasklet_struct   tx_tasklet;
#endif

	void __iomem *mmio_addr;	/* memory map physical address */
	struct pci_dev *pci_dev;	/* Index of PCI device */
	struct net_device *dev;

	struct net_device_stats stats;	/* statistics of net device */
	spinlock_t lock;		/* spin lock flag */
	spinlock_t phy_lock;		/* spin lock flag for GPHY */
	u32 msg_enable;
	int max_jumbo_frame_size;
	//int chipset;
	//int mcfg;
	u32 cur_rx; /* Index into the Rx descriptor buffer of next Rx pkt. */
	u32 cur_tx; /* Index into the Tx descriptor buffer of next Rx pkt. */
	u32 dirty_rx;
	u32 dirty_tx;
	u8 dummy1[32];  //avoid uncache prt
	struct TxDesc *TxDescArray;	/* 256-aligned Tx descriptor ring */  //16 bytes alignment
	u8 dummy2[32];  //avoid uncache prt	
	struct RxDesc *RxDescArray;	/* 256-aligned Rx descriptor ring */
	u8 dummy3[32];  //avoid uncache prt	
	dma_addr_t TxPhyAddr;
	dma_addr_t RxPhyAddr;
	struct sk_buff *Rx_skbuff[NUM_RX_DESC];	/* Rx data buffers */
	struct ring_info tx_skb[NUM_TX_DESC];	/* Tx data buffers */
	unsigned int rx_buf_sz;	
	int rx_fifo_overflow;
	struct timer_list esd_timer;
	struct timer_list link_timer;
	int old_link_status;
	struct pci_resource pci_cfg_space;
	unsigned int esd_flag;
	unsigned int pci_cfg_is_read;
	unsigned int rtl8198_rx_config;
	//u16 cp_cmd;
	u32 intr_mask; //wei add
	int phy_auto_nego_reg;
	int phy_1000_ctrl_reg;
	u8 mac_addr[NODE_ADDRESS_SIZE];

	u8 autoneg;
	u16 speed;
	u8 duplex;

	//extra tx param
	unsigned int info1; //wei add
	unsigned int info2; //wei add
	unsigned int info3; //wei add	
	unsigned int txoffset; //wei add
	
	unsigned int dmsg;
	u8 DMA_buff[TX_BUF_SIZE];
	u32 DMA_len;
	u32 DMA_mode;	

	u8 DMA_rxbuff[RX_BUF_SIZE];
	u32 DMA_rxlen;
	
#ifdef CONFIG_R8198EP_DEVICE

	struct TxDesc TxDescBuff[NUM_TX_DESC+1];
	//u8 dummy4[32];  //avoid uncache prt
	
	struct RxDesc RxDescBuff[NUM_RX_DESC+1];
	//u8 dummy5[32];  //avoid uncache prt	
#ifdef DEVICE_USING_FIXBUF	
	u8 TxDataBuff[NUM_TX_DESC+1][TX_BUF_SIZE];
	//u8 dummy6[32];  //avoid uncache prt
	
	u8 RxDataBuff[NUM_RX_DESC+1][RX_BUF_SIZE];
	//u8 dummy7[32];  //avoid uncache prt	
	
	struct slvpcie_txbuff_t *pTxBuffPtr[NUM_TX_DESC];
	//u8 dummy8[32];  //avoid uncache prt	
	
	struct slvpcie_rxbuff_t *pRxBuffPtr[NUM_RX_DESC];
	//u8 dummy9[32];  //avoid uncache prt	
#endif	
	u8 ISM_temp[RX_BUF_SIZE+4];
	u8 *ISM_buff;
	u32 ISM_len;
#endif

	
	int (*set_speed)(struct net_device *, u8 autoneg, u16 speed, u8 duplex);
//	void (*get_settings)(struct net_device *, struct ethtool_cmd *);		//wei del
//	void (*phy_reset_enable)(struct net_device *);	//wei del
//	unsigned int (*phy_reset_pending)(struct net_device *);	//wei del
	unsigned int (*link_ok)(struct net_device *);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
	struct work_struct task;
#else
	struct delayed_work task;
#endif
	unsigned wol_enabled;
	unsigned features;
};


int rtl8198_eri_read(void __iomem *ioaddr, int addr, int len, int type);
int rtl8198_eri_write(void __iomem *ioaddr, int addr, int len, int value, int type);








//=================================================================
//#define DRAM_CONFIG_VAL 0x52080000  //8M  //0x52080000 //(8M)     //default: 0x58080000 
//#define DRAM_CONFIG_VAL 0x52480000  //16M
#define DRAM_CONFIG_VAL 0x54480000  //32M
//#define DRAM_CONFIG_VAL 0x54880000  //64M

//#define DRAM_TIMING_VAL 0xffff05c0  //default:  FFFF0FC0
#define DRAM_TIMING_VAL 0x6cca8480  //SDRAM

#define DRAM_TRXDLY_VAL_SDR ((9<<5) | (10<<0))   //SDR
#define DRAM_TRXDLY_VAL_DDR1 ((14<<5) | (25<<0))   //DDR1
#define DRAM_TRXDLY_VAL_DDR2 ((9<<5) | (14<<0))   //DDR2

#define DRAM_CONFIG_VALH (DRAM_CONFIG_VAL>>16)
#define DRAM_CONFIG_VALL (DRAM_CONFIG_VAL&0x0000ffff)
#define DRAM_TIMING_VALH (DRAM_TIMING_VAL>>16)
#define DRAM_TIMING_VALL (DRAM_TIMING_VAL&0x0000ffff)
#define DRAM_TRXDLY_VALH (DRAM_TRXDLY_VAL>>16)
#define DRAM_TRXDLY_VALL (DRAM_TRXDLY_VAL&0x0000ffff)

#define NFBI_CPUBOOTADDR 0x00008000
#define NFBI_BOOTADDR 0x00008000
#define NFBI_KERNADDR 0x00700000


//=================================================================



#define DMAMODE_DEFAULT 0
#define DMAMODE_HOSTTX_LOOP 1
#define DMAMODE_DEVICETX_LOOP 2
#define DMAMODE_TXSEQ 4
#define DMAMODE_ISM 8
/*
DMAMODE_TXSEQ mode
= v, v+1, v+2, v+3, v+4 ......len is random.
*/

#define PKTGEN_ValInc (1<<0)
#define PKTGEN_ValFix (1<<1)
#define PKTGEN_ValRand  (1<<2)

#define PKTGEN_LenInc  (1<<3)
#define PKTGEN_LenFix  (1<<4)
#define PKTGEN_LenRand  (1<<5)
#define PKTGEN_LenRand4B  (1<<6)

#define PLTGEN_AllRand (PKTGEN_LenRand|PKTGEN_ValRand)


//===================================================
//Configuration layout

#define Virtual2Physical(x)		(((u32)x) & 0x1fffffff)
#define Physical2Virtual(x)		(((u32)x) | 0x80000000)
#define Virtual2NonCache(x)		(((u32)x) | 0x20000000)
#define Physical2NonCache(x)		(((u32)x) | 0xa0000000)
#define UNCACHE_MASK			0x20000000  //wei add



#define SPE_EP_CFG_BASE (0xb8b41000)
#define SPE_EP_ECFG_BASE (SPE_EP_CFG_BASE + 0x100)  //Extended Configuration


//#define SPE_EP_IRQ_NO 22   //BOOTCODE
#define SPE_EP_IRQ_NO 11  //RLX

//==============================================
//Customer ISM
#define SPE_ISM_MAGNUMI (('w'<<24)|('e' <<16) | ('i'<<8))
#define SPE_ISM_MAGNUMO (('w'<<24)|('e' <<16) | ('o'<<8))
#define SPE_ISM_MAG_REGR 0x01
#define SPE_ISM_MAG_REGW 0x02
#define SPE_ISM_MAG_WDOGRST 0x05
#define SPE_ISM_MAG_DMARST 0x06



