/*
 *  Header file defines some common inline funtions
 *
 *  $Id: 8192cd_util.h,v 1.10.2.4 2010/11/09 09:10:03 button Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef _8192CD_UTIL_H_
#define _8192CD_UTIL_H_

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/spinlock.h>
#include <linux/circ_buf.h>
#endif

#include "./8192cd_cfg.h"
#include "./8192cd.h"
#include "./wifi.h"
#include "./8192cd_hw.h"

#ifdef CONFIG_RTK_MESH
#include "../mesh_ext/mesh_util.h"
#endif

#ifdef GREEN_HILL
#define	SAVE_INT_AND_CLI(x)		{ x = save_and_cli(); }
#define RESTORE_INT(x)			restore_flags(x)
#define SMP_LOCK(__x__)	
#define SMP_UNLOCK(__x__)
#define SMP_LOCK_XMIT(__x__)		
#define SMP_UNLOCK_XMIT(__x__)		
#define SMP_LOCK_SKB(__x__)			
#define SMP_UNLOCK_SKB(__x__)
#define SMP_LOCK_BUF(__x__)			
#define SMP_UNLOCK_BUF(__x__)	
#elif defined(SMP_SYNC) //Add these spin locks to avoid deadlock under SMP platforms.
#define SAVE_INT_AND_CLI(__x__)			
#define RESTORE_INT(__x__)				
#define SMP_LOCK(__x__)					spin_lock_irqsave(&priv->pshare->lock, (__x__))
#define SMP_UNLOCK(__x__)				spin_unlock_irqrestore(&priv->pshare->lock, (__x__))
#define SMP_LOCK_XMIT(__x__)			spin_lock_irqsave(&priv->pshare->lock_xmit, (__x__))
#define SMP_UNLOCK_XMIT(__x__)			spin_unlock_irqrestore(&priv->pshare->lock_xmit, (__x__))
#define SMP_LOCK_SKB(__x__)				//spin_lock_irqsave(&priv->pshare->lock_skb, (__x__))
#define SMP_UNLOCK_SKB(__x__)			//spin_unlock_irqrestore(&priv->pshare->lock_skb, (__x__))
#define SMP_LOCK_BUF(__x__)				spin_lock_irqsave(&priv->pshare->lock_buf, (__x__))
#define SMP_UNLOCK_BUF(__x__)			spin_unlock_irqrestore(&priv->pshare->lock_buf, (__x__))
#else
#define SAVE_INT_AND_CLI(__x__)		spin_lock_irqsave(&priv->pshare->lock, (__x__))
#define RESTORE_INT(__x__)			spin_unlock_irqrestore(&priv->pshare->lock, (__x__))
#ifndef __ECOS
#define SMP_LOCK(__x__)	
#define SMP_UNLOCK(__x__)
#endif
#define SMP_LOCK_XMIT(__x__)		
#define SMP_UNLOCK_XMIT(__x__)		
#define SMP_LOCK_SKB(__x__)			
#define SMP_UNLOCK_SKB(__x__)		
#define SMP_LOCK_BUF(__x__)			
#define SMP_UNLOCK_BUF(__x__)	
#endif

#ifdef __LINUX_2_6__
#ifdef __MIPSEB__
#ifdef virt_to_bus
	#undef virt_to_bus
	#define virt_to_bus			CPHYSADDR
#endif
#endif
#endif

#ifdef STA_EXT
#define REMAP_AID(p)   p->remapped_aid
#else
#define REMAP_AID(p)   p->aid
#endif

/*NOTE if 1.5 seconds should be RTL_SECONDS_TO_JIFFIES(15)/10 
  *RTL_MILISECONDS_TO_JIFFIES shoud consider the HZ value
  *for example HZ=100, x should large than 10
  */
#define RTL_SECONDS_TO_JIFFIES(x) (x*HZ)
#define RTL_MILISECONDS_TO_JIFFIES(x) ((x*HZ)/1000)
#define RTL_10MILISECONDS_TO_JIFFIES(x) ((x*HZ)/100)
#define RTL_JIFFIES_TO_MICROSECOND ((1000*1000)/HZ)
#define RTL_JIFFIES_TO_MILISECONDS(x) ((x*1000)/HZ)

#define GET_CHIP_VER(priv)		((priv->pshare->version_id&VERSION_MASK))
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(SUPPORT_RTL8188E_TC)
#define IS_TEST_CHIP(priv)		((priv->pshare->version_id&0x100))
#endif
#ifdef CONFIG_RTL_92C_SUPPORT
#define IS_88RE(priv)			((priv->pshare->version_id&0x200))
#endif
#define IS_UMC_A_CUT(priv)		((priv->pshare->version_id&0x4f0)==0x400)
#define IS_UMC_B_CUT(priv)		((priv->pshare->version_id&0x4f0)==0x410)
#ifdef CONFIG_RTL_92C_SUPPORT
#define IS_UMC_A_CUT_88C(priv)	(IS_UMC_A_CUT(priv) && (GET_CHIP_VER(priv) == VERSION_8188C))
#define IS_UMC_B_CUT_88C(priv)	(IS_UMC_B_CUT(priv) && (GET_CHIP_VER(priv) == VERSION_8188C))
#endif

#define RTL_SET_MASK(reg,mask,val,shift) (((reg)&(~(mask)))|((val)<<(shift)))

#ifdef USE_IO_OPS

#define get_desc(val)           (val)
#define set_desc(val)           (val)

#define RTL_R8(reg)             inb(((unsigned long)priv->pshare->ioaddr) + (reg))
#define RTL_R16(reg)            inw(((unsigned long)priv->pshare->ioaddr) + (reg))
#define RTL_R32(reg)            ((unsigned long)inl(((unsigned long)priv->pshare->ioaddr) + (reg)))
#define RTL_W8(reg, val8)       outb((val8), ((unsigned long)priv->pshare->ioaddr) + (reg))
#define RTL_W16(reg, val16)     outw((val16), ((unsigned long)priv->pshare->ioaddr) + (reg))
#define RTL_W32(reg, val32)     outl((val32), ((unsigned long)priv->pshare->ioaddr) + (reg))
#define RTL_W8_F                RTL_W8
#define RTL_W16_F               RTL_W16
#define RTL_W32_F               RTL_W32
#undef readb
#undef readw
#undef readl
#undef writeb
#undef writew
#undef writel
#define readb(addr)             inb((unsigned long)(addr))
#define readw(addr)             inw((unsigned long)(addr))
#define readl(addr)             inl((unsigned long)(addr))
#define writeb(val,addr)        outb((val), (unsigned long)(addr))
#define writew(val,addr)        outw((val), (unsigned long)(addr))
#define writel(val,addr)        outl((val), (unsigned long)(addr))

#else // !USE_IO_OPS

#define PAGE_NUM 15

#ifdef __LINUX_2_6__
	#define IO_TYPE_CAST	(unsigned char *)
#else
	#define IO_TYPE_CAST	(unsigned int)
#endif

extern unsigned char rfc1042_header[WLAN_LLC_HEADER_SIZE];
#ifdef CONFIG_RTL_8198
#ifndef REG32
	#define REG32(reg)	 	(*(volatile unsigned int *)(reg))
#endif
#endif
static __inline__ unsigned char RTL_R8_F(struct rtl8192cd_priv *priv, unsigned int reg)
{
	unsigned long ioaddr = priv->pshare->ioaddr;
	unsigned char val8 = 0;

#ifdef IO_MAPPING
	unsigned char page = ((unsigned char)(reg >> 8)) & PAGE_NUM;
	if (priv->pshare->io_mapping && page)
	{
		unsigned long x;
		SAVE_INT_AND_CLI(x);

		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) | page, IO_TYPE_CAST(ioaddr + _PSR_));
		val8 = readb(IO_TYPE_CAST(ioaddr + (reg & 0x000000ff)));
		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) & (~PAGE_NUM), IO_TYPE_CAST(ioaddr + _PSR_));

		RESTORE_INT(x);
	}
	else
#endif
	{
#ifdef CONFIG_RTL_8198
	unsigned int data=0;
	int swap[4]={0,8,16,24};
	int diff = reg&0x3;
        data=REG32((ioaddr + (reg&(0xFFFFFFFC)) ) );
        val8=(unsigned char)(( data>>swap[diff])&0xff);

#else
		val8 = readb(IO_TYPE_CAST(ioaddr + reg));
#endif
	}

	return val8;
}

static __inline__ unsigned short RTL_R16_F(struct rtl8192cd_priv *priv, unsigned int reg)
{
	unsigned long ioaddr = priv->pshare->ioaddr;
	unsigned short val16 = 0;

#ifdef IO_MAPPING
	unsigned char page = ((unsigned char)(reg >> 8)) & PAGE_NUM;
	if (priv->pshare->io_mapping && page)
	{
		unsigned long x;
		SAVE_INT_AND_CLI(x);

		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) | page, IO_TYPE_CAST(ioaddr + _PSR_));
		val16 = readw(IO_TYPE_CAST(ioaddr + (reg & 0x000000ff)));
		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) & (~PAGE_NUM), IO_TYPE_CAST(ioaddr + _PSR_));

		RESTORE_INT(x);
	}
	else
#endif
	{
#ifdef CONFIG_RTL_8198
	unsigned int data=0;
	int swap[4]={0,8,16,24};
	int diff = reg&0x3;
	data=REG32((ioaddr + (reg&(0xFFFFFFFC)) ) );
	val16=(unsigned short)(( data>>swap[diff])&0xffff);
#else
	val16 = readw(IO_TYPE_CAST(ioaddr + reg));
#endif
	}

#ifdef CHECK_SWAP
	if (priv->pshare->type & ACCESS_SWAP_IO)
		val16 = le16_to_cpu(val16);
#endif

	return val16;
}

static __inline__ unsigned int RTL_R32_F(struct rtl8192cd_priv *priv, unsigned int reg)
{
	unsigned long ioaddr = priv->pshare->ioaddr;
	unsigned int val32 = 0;

#ifdef IO_MAPPING
	unsigned char page = ((unsigned char)(reg >> 8)) & PAGE_NUM;
	if (priv->pshare->io_mapping && page)
	{
		unsigned long x;
		SAVE_INT_AND_CLI(x);

		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) | page, IO_TYPE_CAST(ioaddr + _PSR_));
		val32 = readl(IO_TYPE_CAST(ioaddr + (reg & 0x000000ff)));
		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) & (~PAGE_NUM), IO_TYPE_CAST(ioaddr + _PSR_));

		RESTORE_INT(x);
	}
	else
#endif
	{
		val32 = readl(IO_TYPE_CAST(ioaddr + reg));
	}

#ifdef CHECK_SWAP
	if (priv->pshare->type & ACCESS_SWAP_IO)
		val32 = le32_to_cpu(val32);
#endif

	return val32;
}

static __inline__ void RTL_W8_F(struct rtl8192cd_priv *priv, unsigned int reg, unsigned char val8)
{
	unsigned long ioaddr = priv->pshare->ioaddr;
#ifdef IO_MAPPING
	unsigned char page = ((unsigned char)(reg >> 8)) & PAGE_NUM;
	if (priv->pshare->io_mapping && page)
	{
		unsigned long x;
		SAVE_INT_AND_CLI(x);

		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) | page, IO_TYPE_CAST(ioaddr + _PSR_));
		writeb(val8, IO_TYPE_CAST(ioaddr + (reg & 0x000000ff)));
		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) & (~PAGE_NUM), IO_TYPE_CAST(ioaddr + _PSR_));

		RESTORE_INT(x);
	}
	else
#endif
	{
		writeb(val8, IO_TYPE_CAST(ioaddr + reg));
	}
}

static __inline__ void RTL_W16_F(struct rtl8192cd_priv *priv, unsigned int reg, unsigned short val16)
{
	unsigned long ioaddr = priv->pshare->ioaddr;
	unsigned short val16_n = val16;
#ifdef IO_MAPPING
	unsigned char page;
#endif

#ifdef CHECK_SWAP
	if (priv->pshare->type & ACCESS_SWAP_IO)
		val16_n = cpu_to_le16(val16);
#endif

#ifdef IO_MAPPING
	page = ((unsigned char)(reg >> 8)) & PAGE_NUM;
	if (priv->pshare->io_mapping && page)
	{
		unsigned long x;
		SAVE_INT_AND_CLI(x);

		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) | page, IO_TYPE_CAST(ioaddr + _PSR_));
		writew(val16_n, IO_TYPE_CAST(ioaddr + (reg & 0x000000ff)));
		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) & (~PAGE_NUM), IO_TYPE_CAST(ioaddr + _PSR_));

		RESTORE_INT(x);
	}
	else
#endif
	{
		writew(val16_n, IO_TYPE_CAST(ioaddr + reg));
	}
}

static __inline__ void RTL_W32_F(struct rtl8192cd_priv *priv, unsigned int reg, unsigned int val32)
{
	unsigned long ioaddr = priv->pshare->ioaddr;
	unsigned int val32_n = val32;
#ifdef IO_MAPPING
	unsigned char page;
#endif

#ifdef CHECK_SWAP
	if (priv->pshare->type & ACCESS_SWAP_IO)
		val32_n = cpu_to_le32(val32);
#endif

#ifdef IO_MAPPING
	page = ((unsigned char)(reg >> 8)) & PAGE_NUM;
	if (priv->pshare->io_mapping && page)
	{
		unsigned long x;
		SAVE_INT_AND_CLI(x);

		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) | page, IO_TYPE_CAST(ioaddr + _PSR_));
		writel(val32_n, IO_TYPE_CAST(ioaddr + (reg & 0x000000ff)));
		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) & (~PAGE_NUM), IO_TYPE_CAST(ioaddr + _PSR_));

		RESTORE_INT(x);
	}
	else
#endif
	{
		writel(val32_n, IO_TYPE_CAST(ioaddr + reg));
	}
}

#ifdef PCIE_POWER_SAVING


#define RTL_R8(reg)		\
	(( priv->pwr_state==L2  || priv->pwr_state==L1) ? 0 :(RTL_R8_F(priv, reg)) )

#define RTL_R16(reg)	\
	(( priv->pwr_state==L2  || priv->pwr_state==L1) ? 0 : (RTL_R16_F(priv, reg)))

#define RTL_R32(reg)	\
	(( priv->pwr_state==L2  || priv->pwr_state==L1) ? 0 : (RTL_R32_F(priv, reg)))

#define RTL_W8(reg, val8)	\
	do { \
	if( priv->pwr_state==L2  || priv->pwr_state==L1) \
		{  	printk("Error!!! w8:%x,%x in L%d\n", reg, val8, priv->pwr_state);} \
	else \
		RTL_W8_F(priv, reg, val8); \
	} while (0)

#define RTL_W16(reg, val16)	\
	do { \
	if( priv->pwr_state==L2  || priv->pwr_state==L1) \
		printk("Err!!! w16:%x,%x in L%d\n", reg, val16, priv->pwr_state); \
	else \
		RTL_W16_F(priv, reg, val16); \
	} while (0)

#define RTL_W32(reg, val32)	\
	do { \
	if( priv->pwr_state==L2  || priv->pwr_state==L1) \
		printk("Err!!! w32:%x,%x in L%d\n", reg, (unsigned int)val32, priv->pwr_state); \
	else \
		RTL_W32_F(priv, reg, val32) ; \
	} while (0)

#else

#define RTL_R8(reg)		\
	(RTL_R8_F(priv, reg))

#define RTL_R16(reg)	\
	(RTL_R16_F(priv, reg))

#define RTL_R32(reg)	\
	(RTL_R32_F(priv, reg))

#define RTL_W8(reg, val8)	\
	do { \
		RTL_W8_F(priv, reg, val8); \
	} while (0)

#define RTL_W16(reg, val16)	\
	do { \
		RTL_W16_F(priv, reg, val16); \
	} while (0)

#define RTL_W32(reg, val32)	\
	do { \
		RTL_W32_F(priv, reg, val32) ; \
	} while (0)

#endif


#ifdef CHECK_SWAP
#define get_desc(val)	((priv->pshare->type & ACCESS_SWAP_MEM) ? le32_to_cpu(val) : val)
#define set_desc(val)	((priv->pshare->type & ACCESS_SWAP_MEM) ? cpu_to_le32(val) : val)
#else
#define get_desc(val)	(val)
#define set_desc(val)	(val)
#endif

#endif // USE_IO_OPS


#define get_tofr_ds(pframe)	((GetToDs(pframe) << 1) | GetFrDs(pframe))

#define is_qos_data(pframe)	((GetFrameSubType(pframe) & (WIFI_DATA_TYPE | BIT(7))) == (WIFI_DATA_TYPE | BIT(7)))

#define UINT32_DIFF(a, b)		((a >= b)? (a - b):(0xffffffff - b + a + 1))

static __inline__ struct list_head *dequeue_frame(struct rtl8192cd_priv *priv, struct list_head *head)
{
	unsigned long flags;
	struct list_head *pnext;

	SAVE_INT_AND_CLI(flags);
	if (list_empty(head)) {
		RESTORE_INT(flags);
		return (void *)NULL;
	}

	pnext = head->next;
	list_del_init(pnext);

	RESTORE_INT(flags);

	return pnext;
}

static __inline__ int wifi_mac_hash(unsigned char *mac)
{
	unsigned long x;

	x = mac[0];
	x = (x << 2) ^ mac[1];
	x = (x << 2) ^ mac[2];
	x = (x << 2) ^ mac[3];
	x = (x << 2) ^ mac[4];
	x = (x << 2) ^ mac[5];

	x ^= x >> 8;

	return x & (NUM_STAT - 1);
}

#define get_pfrinfo(pskb)		((struct rx_frinfo *)((unsigned long)(pskb->data) - sizeof(struct rx_frinfo)))

#define get_pskb(pfrinfo)		(pfrinfo->pskb)

#define get_pframe(pfrinfo)		((unsigned char *)((unsigned int)(pfrinfo->pskb->data)))

#ifdef CONFIG_NET_PCI
#define IS_PCIBIOS_TYPE		(((priv->pshare->type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS)
#endif

#define rtl_atomic_inc(ptr_atomic_t)	atomic_inc(ptr_atomic_t)
#define rtl_atomic_dec(ptr_atomic_t)	atomic_dec(ptr_atomic_t)
#define rtl_atomic_read(ptr_atomic_t)	atomic_read(ptr_atomic_t)
#define rtl_atomic_set(ptr_atomic_t, i)	atomic_set(ptr_atomic_t,i)

enum _skb_flag_ {
	_SKB_TX_ = 1,
	_SKB_RX_ = 2,
	_SKB_RX_IRQ_ = 4,
	_SKB_TX_IRQ_ = 8
};

// Allocate net device socket buffer
static __inline__ struct sk_buff *rtl_dev_alloc_skb(struct rtl8192cd_priv *priv,
				unsigned int length, int flag, int could_alloc_from_kerenl)
{
	struct sk_buff *skb = NULL;

//	skb = dev_alloc_skb(length);
#ifndef __ECOS
 __MIPS16 
#endif
__IRAM_IN_865X 	extern  struct sk_buff *alloc_skb_from_queue(struct rtl8192cd_priv *priv);

	skb = alloc_skb_from_queue(priv);

	if (skb == NULL && could_alloc_from_kerenl)
		skb = dev_alloc_skb(length);

#ifdef ENABLE_RTL_SKB_STATS
	if (flag & (_SKB_TX_ | _SKB_TX_IRQ_))
		rtl_atomic_inc(&priv->rtl_tx_skb_cnt);
	else
		rtl_atomic_inc(&priv->rtl_rx_skb_cnt);
#endif

	return skb;
}

// Free net device socket buffer
static __inline__ void rtl_kfree_skb(struct rtl8192cd_priv *priv, struct sk_buff *skb, int flag)
{
#ifdef ENABLE_RTL_SKB_STATS
	if (flag & (_SKB_TX_ | _SKB_TX_IRQ_))
		rtl_atomic_dec(&priv->rtl_tx_skb_cnt);
	else
		rtl_atomic_dec(&priv->rtl_rx_skb_cnt);
#endif

	dev_kfree_skb_any(skb);
}

static __inline__ int is_CCK_rate(unsigned char rate)
{
	if ((rate == 2) || (rate == 4) || (rate == 11) || (rate == 22))
		return TRUE;
	else
		return FALSE;
}


static __inline__ int is_MCS_rate(unsigned char rate)
{
	if (rate & 0x80)
		return TRUE;
	else
		return FALSE;
}

static __inline__ int is_fixedMCSTxRate(struct rtl8192cd_priv *priv)
{
	return (priv->pmib->dot11StationConfigEntry.fixedTxRate & 0xffff000);
}


static __inline__ int is_MCS_1SS_rate(unsigned char rate)
{
	if ((rate & 0x80) && (rate < _MCS8_RATE_))
		return TRUE;
	else
		return FALSE;
}

static __inline__ int is_MCS_2SS_rate(unsigned char rate)
{
	if ((rate & 0x80) && (rate > _MCS7_RATE_))
		return TRUE;
	else
		return FALSE;
}

#ifdef __MIPSEB__
static __inline__ void rtl_cache_sync_wback(struct rtl8192cd_priv *priv, unsigned int start,
				unsigned int size, int direction)
{
		if (0 == size) return;	// if the size of cache sync is equal to zero, don't do sync action
#ifdef __LINUX_2_6__
		start = CPHYSADDR(start);
#endif
#if defined(CONFIG_NET_PCI) && !defined(USE_RTL8186_SDK)
		if (IS_PCIBIOS_TYPE) {
#ifdef __LINUX_2_6__
			pci_dma_sync_single_for_cpu(priv->pshare->pdev, start, size, direction);
#else
			pci_dma_sync_single(priv->pshare->pdev, start, size, direction);
#endif
		}
		else
			dma_cache_wback_inv((unsigned long)bus_to_virt(start), size);
#else
		dma_cache_wback_inv((unsigned long)bus_to_virt(start), size);
#endif
}
#else
static __inline__ void rtl_cache_sync_wback(struct rtl8192cd_priv *priv, unsigned int start,
				unsigned int size, int direction)
{

#ifdef __LINUX_2_6__
		pci_dma_sync_single_for_cpu(priv->pshare->pdev, start, size, direction);
#else
		pci_dma_sync_single(priv->pshare->pdev, start, size, direction);
#endif

}
#endif//__MIPSEB__


static __inline__ unsigned long get_physical_addr(struct rtl8192cd_priv *priv, void *ptr,
				unsigned int size, int direction)
{
#if defined(CONFIG_NET_PCI) && !defined(USE_RTL8186_SDK)
	if ((IS_PCIBIOS_TYPE) && (0 != size))
		return pci_map_single(priv->pshare->pdev, ptr, size, direction);
	else
#endif
		return virt_to_bus(ptr);
}


static __inline__ int get_rf_mimo_mode(struct rtl8192cd_priv *priv)
{
	if ((priv->pshare->phw->MIMO_TR_hw_support == MIMO_1T1R) ||
		(priv->pmib->dot11RFEntry.MIMO_TR_mode == MIMO_1T1R))
		return MIMO_1T1R;
#ifdef CONFIG_RTL_92D_SUPPORT
	else if ((priv->pshare->phw->MIMO_TR_hw_support == MIMO_1T2R) ||
		(priv->pmib->dot11RFEntry.MIMO_TR_mode == MIMO_1T2R)) 
		return MIMO_1T2R;
#endif
	else 
		return MIMO_2T2R;
}

static __inline__ unsigned int get_supported_mcs(struct rtl8192cd_priv *priv)
{
	if (get_rf_mimo_mode(priv) == MIMO_1T1R 
#ifdef SMART_CONCURRENT_92D  //-- fwdebug
		&& priv->pmib->dot11RFEntry.smcc==0
#endif
		)
		return (priv->pmib->dot11nConfigEntry.dot11nSupportedMCS & 0x00ff);
	else
		return (priv->pmib->dot11nConfigEntry.dot11nSupportedMCS & 0xffff);
}

#ifdef CONFIG_RTL8672
static __inline__ void tx_sum_up(struct rtl8192cd_priv *priv, struct stat_info *pstat, int pktlen, struct tx_insn* txcfg)
#else
static __inline__ void tx_sum_up(struct rtl8192cd_priv *priv, struct stat_info *pstat, int pktlen)
#endif
{
	struct net_device_stats *pnet_stats;

	if (priv) {
		pnet_stats = &(priv->net_stats);
		pnet_stats->tx_packets++;
		pnet_stats->tx_bytes += pktlen;

		priv->ext_stats.tx_byte_cnt += pktlen;

		// bcm old 11n chipset iot debug, and TXOP enlarge
		priv->pshare->current_tx_bytes += pktlen;

#ifdef USE_OUT_SRC
		if (pstat)
			priv->pshare->NumTxBytesUnicast += pktlen;
#endif		
	}

	if (pstat) {

#if defined(TXREPORT) && (defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT))
#if defined(TESTCHIP_SUPPORT) && defined(CONFIG_RTL_92C_SUPPORT)
		if (IS_TEST_CHIP(priv)) {
			pstat->tx_pkts++;
		} else
#endif
#ifdef STA_EXT
		if (pstat->remapped_aid == FW_NUM_STAT-1)
#else
		if (pstat->aid == FW_NUM_STAT)	
#endif			
#endif
		{
#ifdef CONFIG_RTL8672
		if (txcfg->fr_type == _SKB_FRAME_TYPE_
#ifdef SUPPORT_TX_MCAST2UNI
			&& !txcfg->isMC2UC
#endif		
		)
#endif
		pstat->tx_pkts++;
		}
		pstat->tx_bytes += pktlen;
		pstat->tx_byte_cnt += pktlen;
	}
}


static __inline__ void rx_sum_up(struct rtl8192cd_priv *priv, struct stat_info *pstat, int pktlen, int retry)
{
	struct net_device_stats *pnet_stats;

	if (priv) {
		pnet_stats = &(priv->net_stats);
		pnet_stats->rx_packets++;
		pnet_stats->rx_bytes += pktlen;

		if (retry)
			priv->ext_stats.rx_retrys++;

		priv->ext_stats.rx_byte_cnt += pktlen;

		// bcm old 11n chipset iot debug
		priv->pshare->current_rx_bytes += pktlen;
	}

	if (pstat) {
		pstat->rx_pkts++;
		pstat->rx_bytes += pktlen;
		pstat->rx_byte_cnt += pktlen;
	}
}


static __inline__ unsigned char get_cck_swing_idx(unsigned int bandwidth, unsigned char ofdm_swing_idx)
{
	unsigned char cck_swing_idx;

	if (bandwidth == HT_CHANNEL_WIDTH_20) {
		if (ofdm_swing_idx >= TxPwrTrk_CCK_SwingTbl_Len)
			cck_swing_idx = TxPwrTrk_CCK_SwingTbl_Len - 1;
		else
			cck_swing_idx = ofdm_swing_idx;
	}
	else {	// 40M bw
		if (ofdm_swing_idx < 12)
			cck_swing_idx = 0;
		else if (ofdm_swing_idx > (TxPwrTrk_CCK_SwingTbl_Len - 1 + 12))
			cck_swing_idx = TxPwrTrk_CCK_SwingTbl_Len - 1;
		else
			cck_swing_idx = ofdm_swing_idx - 12;
	}

	return cck_swing_idx;
}

#if defined(CONFIG_RTL_WAPI_SUPPORT)
void wapi_event_indicate(struct rtl8192cd_priv *priv);
#endif

#define CIRC_CNT_RTK(head,tail,size)	((head>=tail)?(head-tail):(size-tail+head))
#define CIRC_SPACE_RTK(head,tail,size)	CIRC_CNT_RTK((tail),((head)+1),(size))

#if defined(USE_PID_NOTIFY) && defined(LINUX_2_6_27_)
extern struct pid *_wlanapp_pid;
#endif

#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
INT32 rtl_isPassthruFrame(UINT8 *data);
#endif

#ifdef USE_TXQUEUE
int init_txq_pool(struct list_head *head, unsigned char **ppool);
void free_txq_pool(struct list_head *head, unsigned char *ppool);
void append_skb_to_txq_head(struct txq_list_head *head, struct rtl8192cd_priv *priv, struct sk_buff *skb, struct net_device *dev, struct list_head *pool);
void append_skb_to_txq_tail(struct txq_list_head *head, struct rtl8192cd_priv *priv, struct sk_buff *skb, struct net_device *dev, struct list_head *pool);
void remove_skb_from_txq(struct txq_list_head *head, struct sk_buff **pskb, struct net_device **pdev, struct list_head *pool);
#endif
void mem_dump(unsigned char *ptitle, unsigned char *pbuf, int len);

#endif // _8192CD_UTIL_H_

