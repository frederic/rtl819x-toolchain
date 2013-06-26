/*
* ----------------------------------------------------------------
* $Header: /home/cvsroot/linux-2.6.19/linux-2.6.x/drivers/net/re865x/rtl865xc_swNic.c,v 1.11 2008/04/11 10:49:14 bo_zhao Exp $
*
* Abstract: Switch core polling mode NIC driver source code.
*
* $Author: bo_zhao $
*
*  Copyright (c) 2011 Realtek Semiconductor Corp.
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.
* ---------------------------------------------------------------
*/
#include <linux/skbuff.h>
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
#include "common/rtl_errno.h"
#include "AsicDriver/asicRegs.h"
#include "rtl865xc_swNic.h"
#include "common/mbuf.h"
#include "AsicDriver/rtl865x_asicCom.h"
#include "AsicDriver/rtl865x_asicL2.h"
#include "AsicDriver/rtl865xC_hs.h"
#ifdef	CONFIG_RTL865X_ROMEPERF
#include "romeperf.h"
#endif
#if defined(CONFIG_RTL_HW_VLAN_SUPPORT)
#include <linux/if_ether.h>
#endif

#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
#include <linux/if_ether.h>
#endif

extern void (*_dma_cache_wback_inv)(unsigned long start, unsigned long size);

/* RX Ring */
static uint32*  rxPkthdrRing[RTL865X_SWNIC_RXRING_HW_PKTDESC];                 /* Point to the starting address of RX pkt Hdr Ring */
__DRAM_FWD static uint32   rxPkthdrRingCnt[RTL865X_SWNIC_RXRING_HW_PKTDESC];              /* Total pkt count for each Rx descriptor Ring */
#if	defined(DELAY_REFILL_ETH_RX_BUF)
__DRAM_FWD static uint32   rxPkthdrRefillThreshold[RTL865X_SWNIC_RXRING_HW_PKTDESC];              /* Ether refill threshold for each Rx descriptor Ring */
#endif

/* TX Ring */
static uint32*  txPkthdrRing[RTL865X_SWNIC_TXRING_HW_PKTDESC];             /* Point to the starting address of TX pkt Hdr Ring */

#if defined(CONFIG_RTL8196C_REVISION_B)
__DRAM_FWD static uint32	rtl_chip_version;
static uint32*  txPkthdrRing_backup[RTL865X_SWNIC_TXRING_HW_PKTDESC];             /* Point to the starting address of TX pkt Hdr Ring */
#endif

__DRAM_FWD static uint32   txPkthdrRingCnt[RTL865X_SWNIC_TXRING_HW_PKTDESC];          /* Total pkt count for each Tx descriptor Ring */

#define txPktHdrRingFull(idx)   (((txPkthdrRingFreeIndex[idx] + 1) & (txPkthdrRingMaxIndex[idx])) == (txPkthdrRingDoneIndex[idx]))

/* Mbuf */
uint32* rxMbufRing;                                                     /* Point to the starting address of MBUF Ring */
__DRAM_FWD uint32  rxMbufRingCnt;                                                  /* Total MBUF count */

__DRAM_FWD static uint32  size_of_cluster;

/* descriptor ring tracing pointers */
__DRAM_FWD static int32   currRxPkthdrDescIndex[RTL865X_SWNIC_RXRING_HW_PKTDESC];      /* Rx pkthdr descriptor to be handled by CPU */
__DRAM_FWD static int32   currRxMbufDescIndex;        /* Rx mbuf descriptor to be handled by CPU */

__DRAM_FWD static int32   currTxPkthdrDescIndex[RTL865X_SWNIC_TXRING_HW_PKTDESC];      /* Tx pkthdr descriptor to be handled by CPU */
__DRAM_FWD static int32 txPktDoneDescIndex[RTL865X_SWNIC_TXRING_HW_PKTDESC];

/* debug counters */
//__DRAM_FWD static int32   rxPktCounter;
//__DRAM_FWD static int32   txPktCounter;

#if	defined(DELAY_REFILL_ETH_RX_BUF)
__DRAM_FWD static int32   rxDescReadyForHwIndex[RTL865X_SWNIC_RXRING_HW_PKTDESC];
__DRAM_FWD static int32   rxDescCrossBoundFlag[RTL865X_SWNIC_RXRING_HW_PKTDESC];
#endif

__DRAM_FWD static uint8 extPortMaskToPortNum[_RTL865XB_EXTPORTMASKS+1] =
{
	5, 6, 7, 5, 8, 5, 5, 5
};

#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
int set_portmask_tag;
int32 swNic_setVlanPortTag(int portmask){
        set_portmask_tag = portmask &0x3f;
        return 0;
}
#endif

#if defined(CONFIG_RTL_PROC_DEBUG)
unsigned int rx_noBuffer_cnt=0;
unsigned int tx_ringFull_cnt=0;
#endif

#if defined(CONFIG_RTL_HW_VLAN_SUPPORT)
int32 auto_set_tag_portmask;
int32 swNic_setVlanPortTag(int portmask)
{
        auto_set_tag_portmask = portmask &0x3f;
        return 0;
}
#endif


/*************************************************************************
*   FUNCTION
*       swNic_intHandler
*
*   DESCRIPTION
*       This function is the handler of NIC interrupts
*
*   INPUTS
*       intPending      Pending interrupt sources.
*
*   OUTPUTS
*       None
*************************************************************************/
void swNic_intHandler(uint32 intPending) {return;}
__MIPS16
__IRAM_FWD
inline int32 rtl8651_rxPktPreprocess(void *pkt, unsigned int *vid)
{
	struct rtl_pktHdr *m_pkthdr = (struct rtl_pktHdr *)pkt;
	uint32 srcPortNum;
	
	srcPortNum = m_pkthdr->ph_portlist;
	*vid = m_pkthdr->ph_vlanId;
   
	
	#if 0
	if (srcPortNum >= RTL8651_CPU_PORT)
	{
		if (m_pkthdr->ph_extPortList == 0)
		{
			/* No any destination ( extension port or CPU) : ASIC's BUG */
			return FAILED;
		}else if ((m_pkthdr->ph_extPortList & PKTHDR_EXTPORTMASK_CPU) == 0)
		{
			/*
				if dest Ext port 0x1 => to dst ext port 1 => from src port 1+5=6
				if dest Ext port 0x2 => to dst ext port 2 => from src port 2+5=7
				if dest Ext port 0x4 => to dst ext port 3 => from src port 3+5=8
			*/
			srcPortNum = extPortMaskToPortNum[m_pkthdr->ph_extPortList];
			#if	defined(CONFIG_RTL_HARDWARE_NAT)&&(defined(CONFIG_RTL8192SE)||defined(CONFIG_RTL8192CD))
			m_pkthdr->ph_portlist = srcPortNum;
			*vid = PKTHDR_EXTPORT_MAGIC;
			#else
			*vid = m_pkthdr->ph_vlanId;
			#endif
		}else
		{
			/* has CPU bit, pkt is original pkt from port 6~8 */
			srcPortNum = m_pkthdr->ph_srcExtPortNum + RTL8651_PORT_NUMBER - 1;
			m_pkthdr->ph_portlist = srcPortNum;
			#if	defined(CONFIG_RTL_HARDWARE_NAT)&&(defined(CONFIG_RTL8192SE)||defined(CONFIG_RTL8192CD))
			*vid = PKTHDR_EXTPORT_MAGIC2;
			#else
			*vid = m_pkthdr->ph_vlanId;
			#endif
		}
	}
       else
	#else
	if (srcPortNum < RTL8651_CPU_PORT)
	#endif
	{
		#if 0
		/* otherwise, pkt is rcvd from PHY */
		m_pkthdr->ph_srcExtPortNum = 0;
		*vid = m_pkthdr->ph_vlanId;
		if((m_pkthdr->ph_extPortList & PKTHDR_EXTPORTMASK_CPU) == 0)
		{	/* No CPU bit, only dest ext mbr port... */
			/*
				if dest Ext port 0x1 => to dst ext port 1 => from src port 1+5=6
				if dest Ext port 0x2 => to dst ext port 2 => from src port 2+5=7
				if dest Ext port 0x4 => to dst ext port 3 => from src port 3+5=8
			*/
			if(m_pkthdr->ph_extPortList&&5!=extPortMaskToPortNum[m_pkthdr->ph_extPortList])
			{
				/* redefine src port number */
				srcPortNum = extPortMaskToPortNum[m_pkthdr->ph_extPortList];
				#if	defined(CONFIG_RTL_HARDWARE_NAT)&&(defined(CONFIG_RTL8192SE)||defined(CONFIG_RTL8192CD))
				m_pkthdr->ph_portlist = srcPortNum;
				*vid = PKTHDR_EXTPORT_MAGIC;
				#else
				*vid = m_pkthdr->ph_vlanId;
				#endif
			}
		}
		#endif
	}		else {
		return FAILED;
	}

	return SUCCESS;
}

#if	defined(DELAY_REFILL_ETH_RX_BUF)
inline int return_to_rxing_check(int ringIdx)
{
	int ret;
	//unsigned long flags;
	//local_irq_save(flags);
	ret = ((rxDescReadyForHwIndex[ringIdx]!=currRxPkthdrDescIndex[ringIdx]) && (rxPkthdrRingCnt[ringIdx]!=0))? 1:0;
	//local_irq_restore(flags);
	return ret;
}
static inline int buffer_reuse(int ringIdx)
{
	int index1,index2,gap;
	unsigned long flags;
	local_irq_save(flags);
	index1 = rxDescReadyForHwIndex[ringIdx];
	index2 = currRxPkthdrDescIndex[ringIdx]+1;
	gap = (index2 > index1) ? (index2 - index1) : (index2 + rxPkthdrRingCnt[ringIdx] - index1);

	if ((rxPkthdrRingCnt[ringIdx] - gap) < (rxPkthdrRefillThreshold[ringIdx]))
	{
		local_irq_restore(flags);
		return 1;
	}
	else
	{
		local_irq_restore(flags);
		return 0;
	}
}

static inline void set_RxPkthdrRing_OwnBit(uint32 rxRingIdx)
{
	rxPkthdrRing[rxRingIdx][rxDescReadyForHwIndex[rxRingIdx]] |= DESC_SWCORE_OWNED;

	if ( ++rxDescReadyForHwIndex[rxRingIdx] == rxPkthdrRingCnt[rxRingIdx] ) {
		rxDescReadyForHwIndex[rxRingIdx] = 0;
		#if	defined(DELAY_REFILL_ETH_RX_BUF)
		rxDescCrossBoundFlag[rxRingIdx]--;
		#endif
	}
}

__MIPS16
__IRAM_FWD
static void release_pkthdr(struct sk_buff  *skb, int idx)
{
	struct rtl_pktHdr *pReadyForHw;
	uint32 mbufIndex;
	//unsigned long flags;

	_dma_cache_wback_inv((unsigned long)skb->head, skb->truesize);
	//local_irq_save(flags);
	pReadyForHw = (struct rtl_pktHdr *)(rxPkthdrRing[idx][rxDescReadyForHwIndex[idx]] &
						~(DESC_OWNED_BIT | DESC_WRAP));
	mbufIndex = ((uint32)(pReadyForHw->ph_mbuf) - (rxMbufRing[0] & ~(DESC_OWNED_BIT | DESC_WRAP))) /
					(sizeof(struct rtl_mBuf));

	pReadyForHw->ph_mbuf->m_data = skb->data;
	pReadyForHw->ph_mbuf->m_extbuf = skb->data;
	pReadyForHw->ph_mbuf->skb = skb;

	rxMbufRing[mbufIndex] |= DESC_SWCORE_OWNED;
	set_RxPkthdrRing_OwnBit(idx);
	//local_irq_restore(flags);
}

__IRAM_FWD
int check_rx_pkthdr_ring(int idx, int *return_idx)
{
	int i;
	unsigned long flags;

	local_irq_save(flags);
	for(i = idx; i >= 0; i--) {
		if (return_to_rxing_check(i)) {
			*return_idx = i;
			local_irq_restore(flags);
			return SUCCESS;
		}
	}

	*return_idx = -1;
	local_irq_restore(flags);
	return FAILED;
}

__IRAM_FWD
int check_and_return_to_rx_pkthdr_ring(void *skb, int idx)
{
	unsigned long flags;

	local_irq_save(flags);
	if (return_to_rxing_check(idx)) {
		release_pkthdr(skb, idx);
		local_irq_restore(flags);
		return SUCCESS;
	} else {
		local_irq_restore(flags);
		return FAILED;
	}
}

/*
	return value: 1 ==> success, returned to rx pkt hdr desc
	return value: 0 ==> failed, no return ==> release to priv skb buf pool
 */
extern struct sk_buff *dev_alloc_8190_skb(unsigned char *data, int size);

__IRAM_FWD
int return_to_rx_pkthdr_ring(unsigned char *head)
{
	struct sk_buff *skb;
	int ret, i;
	unsigned long flags;

	ret=FAILED;
	local_irq_save(flags);
	for(i = RTL865X_SWNIC_RXRING_MAX_PKTDESC -1; i >= 0; i--)
	{
		if (return_to_rxing_check(i)) {
			skb = dev_alloc_8190_skb(head, CROSS_LAN_MBUF_LEN);
			if (skb == NULL) {
				goto _ret1;
			}

			skb_reserve(skb, RX_OFFSET);
			release_pkthdr(skb, i);
			ret = SUCCESS;
			REG32(CPUIISR) = (MBUF_DESC_RUNOUT_IP_ALL|PKTHDR_DESC_RUNOUT_IP_ALL);
			break;
		}
	}

_ret1:
	local_irq_restore(flags);
	return ret;
}
#endif //DELAY_REFILL_ETH_RX_BUF

__IRAM_FWD
static void increase_rx_idx_release_pkthdr(struct sk_buff  *skb, int idx)
{
	struct rtl_pktHdr *pReadyForHw;
	uint32 mbufIndex;
	unsigned long flags;

	_dma_cache_wback_inv((unsigned long)skb->head, skb->truesize);
	local_irq_save(flags);

#if	defined(DELAY_REFILL_ETH_RX_BUF)
	pReadyForHw = (struct rtl_pktHdr *)(rxPkthdrRing[idx][rxDescReadyForHwIndex[idx]] &
						~(DESC_OWNED_BIT | DESC_WRAP));
#else
	pReadyForHw = (struct rtl_pktHdr *)(rxPkthdrRing[idx][currRxPkthdrDescIndex[idx]] &
						~(DESC_OWNED_BIT | DESC_WRAP));

	rxPkthdrRing[idx][currRxPkthdrDescIndex[idx]] |= DESC_SWCORE_OWNED;
#endif

	if ( ++currRxPkthdrDescIndex[idx] == rxPkthdrRingCnt[idx] ) {
		currRxPkthdrDescIndex[idx] = 0;
		#if	defined(DELAY_REFILL_ETH_RX_BUF)
		rxDescCrossBoundFlag[idx]++;
		#endif
	}

	mbufIndex = ((uint32)(pReadyForHw->ph_mbuf) - (rxMbufRing[0] & ~(DESC_OWNED_BIT | DESC_WRAP))) /
					(sizeof(struct rtl_mBuf));

	pReadyForHw->ph_mbuf->m_data = skb->data;
	pReadyForHw->ph_mbuf->m_extbuf = skb->data;
	pReadyForHw->ph_mbuf->skb = skb;

	rxMbufRing[mbufIndex] |= DESC_SWCORE_OWNED;

#if	defined(DELAY_REFILL_ETH_RX_BUF)
	set_RxPkthdrRing_OwnBit(idx);
#endif

	local_irq_restore(flags);
}

__IRAM_FWD
static int __swNic_geRxRingIdx(uint32 rxRingIdx, uint32 *currRxPktDescIdx)
{
	unsigned long flags;

	if(rxPkthdrRingCnt[rxRingIdx] == 0) {
		return FAILED;
	}

	local_irq_save(flags);
	#if	defined(DELAY_REFILL_ETH_RX_BUF)
	if ( (rxDescCrossBoundFlag[rxRingIdx]==0&&(currRxPkthdrDescIndex[rxRingIdx]>=rxDescReadyForHwIndex[rxRingIdx]))
		|| (rxDescCrossBoundFlag[rxRingIdx]==1&&(currRxPkthdrDescIndex[rxRingIdx]<rxDescReadyForHwIndex[rxRingIdx])) )
	#endif
	{
		if((rxPkthdrRing[rxRingIdx][currRxPkthdrDescIndex[rxRingIdx]] & DESC_OWNED_BIT) == DESC_RISC_OWNED)
		{
			local_irq_restore(flags);
			*currRxPktDescIdx = currRxPkthdrDescIndex[rxRingIdx];
			return SUCCESS;
		}
	}

	local_irq_restore(flags);
	return FAILED;
}

#if defined(RTL_MULTIPLE_RX_TX_RING)
/*	It's the caller's responsibility to make sure "rxRingIdx" and
*	"currRxPktDescIdx" NOT NULL, since the callee never check
*	sanity of the parameters, in order to speed up.
*/
/*
	Should protected by Caller
*/
__MIPS16
__IRAM_FWD
static inline int32 swNic_getRxringIdx(uint32 *rxRingIdx, uint32 *currRxPktDescIdx,uint32 policy)
{
	int32	i;
	int32	priority;


	priority = policy;

	for(i = RTL865X_SWNIC_RXRING_MAX_PKTDESC -1; i >= priority; i--)
	{
		if (__swNic_geRxRingIdx(i, currRxPktDescIdx)==SUCCESS) {
			*rxRingIdx = i;
			return SUCCESS;
		}
	}
	return FAILED;
}
#endif

__IRAM_FWD
int swNic_increaseRxIdx(int rxRingIdx)
{
	unsigned long	flags;
	//int32		nextIdx;

	local_irq_save(flags);
	if ( ++currRxPkthdrDescIndex[rxRingIdx] == rxPkthdrRingCnt[rxRingIdx] ) {
		currRxPkthdrDescIndex[rxRingIdx] = 0;
		#if	defined(DELAY_REFILL_ETH_RX_BUF)
		rxDescCrossBoundFlag[rxRingIdx]++;
		#endif
	}
	local_irq_restore(flags);

	return SUCCESS;
}

#if defined(CONFIG_RTL_ETH_PRIV_SKB_DEBUG)
int mbuf_pending_times = 0;
int get_mbuf_pending_times(void)
{
	return mbuf_pending_times;
}
int get_nic_txRing_buf(void)
{
	int txCnt = 0;
	int i,j;
	struct rtl_pktHdr *pPkthdr;
	for(i = RTL865X_SWNIC_TXRING_HW_PKTDESC -1; i >= 0; i--)
	{
		if(txPkthdrRingCnt[i] == 0)
			continue;

		for(j = 0; j <txPkthdrRingCnt[i]; j++)
		{

			pPkthdr = (struct rtl_pktHdr *) ((int32) txPkthdrRing[i][j]& ~(DESC_OWNED_BIT | DESC_WRAP));

			if(pPkthdr->ph_mbuf->skb)
			{
				if(is_rtl865x_eth_priv_buf(((struct sk_buff *)pPkthdr->ph_mbuf->skb)->head))
					txCnt++;
			}
		}
	}

	return txCnt;
}

int get_nic_rxRing_buf(void)
{
	int rxCnt = 0;
	int i,j;
	struct rtl_pktHdr *pPkthdr;
	for(i = RTL865X_SWNIC_RXRING_HW_PKTDESC -1; i >= 0; i--)
	{
		if(rxPkthdrRingCnt[i] == 0)
			continue;

		for(j = 0; j < rxPkthdrRingCnt[i]; j++)
		{
			{
				pPkthdr = (struct rtl_pktHdr *) (rxPkthdrRing[i][j] & ~(DESC_OWNED_BIT | DESC_WRAP));
				if(pPkthdr->ph_mbuf->skb)
				{
					if(is_rtl865x_eth_priv_buf(((struct sk_buff *)pPkthdr->ph_mbuf->skb)->head))
						rxCnt++;
				}
			}
		}
	}

	return rxCnt;
}
#endif

int32 swNic_flushRxRingByPriority(int priority)
{
	int32	i;
	struct rtl_pktHdr * pPkthdr;
	void *skb;
	unsigned long flags;

	#if defined(CONFIG_RTL865X_WTDOG)
	REG32(WDTCNR) |=  WDTCLR; /* reset watchdog timer */
	#endif
	local_irq_save(flags);
	for(i = priority -1; i >= 0; i--)
	{
		if(rxPkthdrRingCnt[i] == 0)
			continue;

		while((rxPkthdrRing[i][currRxPkthdrDescIndex[i]] & DESC_OWNED_BIT) == DESC_RISC_OWNED)
		{
			pPkthdr = (struct rtl_pktHdr *) (rxPkthdrRing[i][currRxPkthdrDescIndex[i]] & ~(DESC_OWNED_BIT | DESC_WRAP));
			skb = pPkthdr->ph_mbuf->skb;
			increase_rx_idx_release_pkthdr((void*)skb, i);
		}
	}
	local_irq_restore(flags);
	REG32(CPUIISR) = (MBUF_DESC_RUNOUT_IP_ALL|PKTHDR_DESC_RUNOUT_IP_ALL);
	return SUCCESS;
}


/*************************************************************************
*   FUNCTION
*       swNic_receive
*
*   DESCRIPTION
*       This function reads one packet from rx descriptors, and return the
*       previous read one to the switch core. This mechanism is based on
*       the assumption that packets are read only when the handling
*       previous read one is done.
*
*   INPUTS
*       None
*
*   OUTPUTS
*       None
*************************************************************************/
#define	RTL_NIC_RX_RETRY_MAX		(256)

//#if	defined(DELAY_REFILL_ETH_RX_BUF)
#define	RTL_ETH_NIC_DROP_RX_PKT_RESTART		\
	do {\
		increase_rx_idx_release_pkthdr(pPkthdr->ph_mbuf->skb, rxRingIdx); \
		REG32(CPUIISR) = (MBUF_DESC_RUNOUT_IP_ALL|PKTHDR_DESC_RUNOUT_IP_ALL); \
	} while(0)
//#else
//#define	RTL_ETH_NIC_DROP_RX_PKT_RESTART		#error	"check here"
//#endif

__MIPS16
__IRAM_FWD
int32 swNic_receive(rtl_nicRx_info *info, int retryCount)
{
	struct rtl_pktHdr * pPkthdr;
	unsigned char *buf;
	void *skb;
	uint32 rxRingIdx;
	uint32 currRxPktDescIdx;
	#if defined(CONFIG_RTL_HARDWARE_NAT)
	uint32	vid;
	#endif

get_next:
	 /* Check OWN bit of descriptors */
	#if defined(RTL_MULTIPLE_RX_TX_RING)
	if (swNic_getRxringIdx(&rxRingIdx,&currRxPktDescIdx,info->priority) == SUCCESS )
	#else
	rxRingIdx = 0;
	if (__swNic_geRxRingIdx(rxRingIdx,&currRxPktDescIdx)==SUCCESS)
	#endif
	{
		/* Fetch pkthdr */
		pPkthdr = (struct rtl_pktHdr *) (rxPkthdrRing[rxRingIdx][currRxPktDescIdx] & ~(DESC_OWNED_BIT | DESC_WRAP));

		/* Increment counter */
		//rxPktCounter++;

		/*	checksum error drop it	*/
		if ((pPkthdr->ph_flags & (CSUM_TCPUDP_OK | CSUM_IP_OK)) != (CSUM_TCPUDP_OK | CSUM_IP_OK))
		{
			RTL_ETH_NIC_DROP_RX_PKT_RESTART;
			goto get_next;
		}

#if defined(CONFIG_RTL_HARDWARE_NAT)
		if (rtl8651_rxPktPreprocess(pPkthdr, &vid) != 0) {
			RTL_ETH_NIC_DROP_RX_PKT_RESTART;
			goto get_next;
		} else {
			buf = alloc_rx_buf(&skb, size_of_cluster);
		}
		info->vid = vid;
#else
		/*
		 * vid is assigned in rtl8651_rxPktPreprocess()
		 * do not update it when CONFIG_RTL_HARDWARE_NAT is defined
		 */
		info->vid=pPkthdr->ph_vlanId;
		buf = alloc_rx_buf(&skb, size_of_cluster);
#endif
		info->pid=pPkthdr->ph_portlist;
		if (buf)
		{
			info->input = pPkthdr->ph_mbuf->skb;
			info->len = pPkthdr->ph_len - 4;
//			#if	defined(DELAY_REFILL_ETH_RX_BUF)
			/* Increment index */
			increase_rx_idx_release_pkthdr(skb, rxRingIdx);
//			#else
//			pPkthdr->ph_mbuf->m_data = pPkthdr->ph_mbuf->m_extbuf = buf;
//			pPkthdr->ph_mbuf->skb = skb;
//			#endif
			REG32(CPUIISR) = (MBUF_DESC_RUNOUT_IP_ALL|PKTHDR_DESC_RUNOUT_IP_ALL);
		}
#if	defined(DELAY_REFILL_ETH_RX_BUF)
		else if (!buffer_reuse(rxRingIdx)) {
			info->input = pPkthdr->ph_mbuf->skb;
			info->len = pPkthdr->ph_len - 4;
			//printk("====CPU is pending====\n");
			#if defined(CONFIG_RTL_ETH_PRIV_SKB_DEBUG)
			mbuf_pending_times ++;
			pPkthdr->ph_mbuf->skb = NULL;
			#endif
			/* Increment index */
			swNic_increaseRxIdx(rxRingIdx);
		} else {
			#if defined(CONFIG_RTL_PROC_DEBUG)
				rx_noBuffer_cnt++;
			#endif
			#if 0
			if (retryCount>RTL_NIC_RX_RETRY_MAX) {
				/* drop pkt */
				increase_rx_idx_release_pkthdr((void*)pPkthdr->ph_mbuf->skb, rxRingIdx);
			}
			return RTL_NICRX_REPEAT;
			#else
			return RTL_NICRX_NULL;
			#endif
		}
#endif

		#if defined(RTL_MULTIPLE_RX_TX_RING)
		info->priority = rxRingIdx;
		#endif
		return RTL_NICRX_OK;
	} else {
		return RTL_NICRX_NULL;
	}
}

#undef	RTL_ETH_NIC_DROP_RX_PKT_RESTART

/*************************************************************************
*   FUNCTION
*       swNic_send
*
*   DESCRIPTION
*       This function writes one packet to tx descriptors, and waits until
*       the packet is successfully sent.
*
*   INPUTS
*       None
*
*   OUTPUTS
*       None
*************************************************************************/
__MIPS16
__IRAM_FWD  inline int32 _swNic_send(void *skb, void * output, uint32 len,rtl_nicTx_info *nicTx)
{
	struct rtl_pktHdr * pPkthdr;
	int next_index, ret;

	if ((currTxPkthdrDescIndex[nicTx->txIdx]+1)==txPkthdrRingCnt[nicTx->txIdx])
		next_index = 0;
	else
		next_index = currTxPkthdrDescIndex[nicTx->txIdx]+1;

	if (next_index == txPktDoneDescIndex[nicTx->txIdx])	{
		/*	TX ring full	*/
			return -1;
	}

	/* Fetch packet header from Tx ring */
	pPkthdr = (struct rtl_pktHdr *) ((int32) txPkthdrRing[nicTx->txIdx][currTxPkthdrDescIndex[nicTx->txIdx]]
                                                & ~(DESC_OWNED_BIT | DESC_WRAP));

	/* Pad small packets and add CRC */
	if ( len < 60 )
		len = 64;
	else
		len += 4;

	pPkthdr->ph_mbuf->m_len  = len;
	pPkthdr->ph_mbuf->m_extsize = len;
	pPkthdr->ph_mbuf->skb = skb;
	pPkthdr->ph_len = len;

	pPkthdr->ph_vlanId = nicTx->vid;
	#if defined(CONFIG_8198_PORT5_GMII) || defined(CONFIG_8198_PORT5_RGMII)
	pPkthdr->ph_portlist = nicTx->portlist&0x3f;
	#else
	pPkthdr->ph_portlist = nicTx->portlist&0x1f;
	#endif
	pPkthdr->ph_srcExtPortNum = nicTx->srcExtPort;
	pPkthdr->ph_flags = nicTx->flags;
#if	defined(CONFIG_RTL_HW_QOS_SUPPORT) || defined(CONFIG_RTK_VOIP_QOS)
	pPkthdr->ph_txPriority = nicTx->priority;
#endif
#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
if (*((unsigned short *)((unsigned char*)output+ETH_ALEN*2)) != __constant_htons(ETH_P_8021Q))
	pPkthdr->ph_txCVlanTagAutoAdd = set_portmask_tag;
else
	pPkthdr->ph_txCVlanTagAutoAdd = 0;
#endif

#if defined(CONFIG_RTL_HW_VLAN_SUPPORT)
	if (*((unsigned short *)((unsigned char*)output+ETH_ALEN*2)) != __constant_htons(ETH_P_8021Q))
			pPkthdr->ph_txCVlanTagAutoAdd = auto_set_tag_portmask;
	else
			pPkthdr->ph_txCVlanTagAutoAdd = 0;
#endif


	/* Set cluster pointer to buffer */
	pPkthdr->ph_mbuf->m_data    = (output);
	pPkthdr->ph_mbuf->m_extbuf = (output);

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
	pPkthdr->ph_ptpPkt = 0;
#endif

	ret = currTxPkthdrDescIndex[nicTx->txIdx];
	currTxPkthdrDescIndex[nicTx->txIdx] = next_index;
	/* Give descriptor to switch core */
	txPkthdrRing[nicTx->txIdx][ret] |= DESC_SWCORE_OWNED;

#if 0
	memDump((void*)output, 64, "TX");
	printk("index %d address 0x%p, 0x%x 0x%p.\n", ret, &txPkthdrRing[nicTx->txIdx][ret], (*(volatile uint32 *)&txPkthdrRing[nicTx->txIdx][ret]), pPkthdr);
	printk("Flags 0x%x proto 0x%x portlist 0x%x vid %d extPort %d srcExtPort %d len %d.\n",
		pPkthdr->ph_flags, pPkthdr->ph_proto, pPkthdr->ph_portlist, pPkthdr->ph_vlanId,
		pPkthdr->ph_extPortList, pPkthdr->ph_srcExtPortNum, pPkthdr->ph_len);
#endif

	/* Set TXFD bit to start send */
	REG32(CPUICR) |= TXFD;

	return ret;
}

__IRAM_FWD
int32 swNic_send(void *skb, void * output, uint32 len,rtl_nicTx_info *nicTx)
{
	int	ret;
	unsigned long flags;

	local_irq_save(flags);
	ret = _swNic_send(skb, output, len, nicTx);
	local_irq_restore(flags);
	return ret;
}

__IRAM_FWD
int32 swNic_txDone(int idx)
{
	struct rtl_pktHdr	*pPkthdr;
	//int				free_num;
	unsigned long flags;

	local_irq_save(flags);
	//free_num = 0;
	{
		while (txPktDoneDescIndex[idx] != currTxPkthdrDescIndex[idx]) {
		if ( (*(volatile uint32 *)&txPkthdrRing[idx][txPktDoneDescIndex[idx]]
			& DESC_OWNED_BIT) == DESC_RISC_OWNED )
		{
			#ifdef CONFIG_RTL8196C_REVISION_B
			if (rtl_chip_version == RTL8196C_REVISION_A)
				txPkthdrRing[idx][txPktDoneDescIndex[idx]] =txPkthdrRing_backup[idx][txPktDoneDescIndex[idx]] ;
			#endif

			pPkthdr = (struct rtl_pktHdr *) ((int32) txPkthdrRing[idx][txPktDoneDescIndex[idx]]
				& ~(DESC_OWNED_BIT | DESC_WRAP));

			if (pPkthdr->ph_mbuf->skb)
			{
				local_irq_restore(flags);
				#if defined(CONFIG_RTL_FAST_BRIDGE)
				tx_done_callback(pPkthdr->ph_mbuf->skb);
				#else
				dev_kfree_skb_any((struct sk_buff *)pPkthdr->ph_mbuf->skb);
				#endif
				local_irq_save(flags);
				pPkthdr->ph_mbuf->skb = NULL;
			}


			if (++txPktDoneDescIndex[idx] == txPkthdrRingCnt[idx])
				txPktDoneDescIndex[idx] = 0;

			//free_num++;
		}
		else
			break;
		}
	}

	local_irq_restore(flags);
	return 0; //free_num;
}

#ifdef  CONFIG_RTL865X_MODEL_TEST_FT2
int32 swNic_send_portmbr(void * output, uint32 len, uint32 portmbr)
{
    struct rtl_pktHdr * pPkthdr;
    uint8 pktbuf[2048];
    uint8* pktbuf_alligned = (uint8*) (( (uint32) pktbuf & 0xfffffffc) | 0xa0000000);

    /* Copy Packet Content */
    memcpy(pktbuf_alligned, output, len);

    ASSERT_CSP( ((int32) txPkthdrRing[0][currTxPkthdrDescIndex] & DESC_OWNED_BIT) == DESC_RISC_OWNED );

    /* Fetch packet header from Tx ring */
    pPkthdr = (struct rtl_pktHdr *) ((int32) txPkthdrRing[0][currTxPkthdrDescIndex]
                                                & ~(DESC_OWNED_BIT | DESC_WRAP));

    /* Pad small packets and add CRC */
    if ( len < 60 )
        pPkthdr->ph_len = 64;
    else
        pPkthdr->ph_len = len + 4;

    pPkthdr->ph_mbuf->m_len = pPkthdr->ph_len;
    pPkthdr->ph_mbuf->m_extsize = pPkthdr->ph_len;

    /* Set cluster pointer to buffer */
    pPkthdr->ph_mbuf->m_data = pktbuf_alligned;
    pPkthdr->ph_mbuf->m_extbuf = pktbuf_alligned;

    /* Set destination port */
    pPkthdr->ph_portlist = portmbr;

    /* Give descriptor to switch core */
    txPkthdrRing[0][currTxPkthdrDescIndex] |= DESC_SWCORE_OWNED;

    /* Set TXFD bit to start send */
    REG32(CPUICR) |= TXFD;

    /* Wait until packet is successfully sent */
#if 1
    while ( (*(volatile uint32 *)&txPkthdrRing[0][currTxPkthdrDescIndex]
                    & DESC_OWNED_BIT) == DESC_SWCORE_OWNED );
#endif
    //txPktCounter++;

    if ( ++currTxPkthdrDescIndex == txPkthdrRingCnt[0] )
        currTxPkthdrDescIndex = 0;

    return 0;
}
#endif


void swNic_freeRxBuf(void)
{
	int i;
	//int idx;
	//struct rtl_pktHdr * pPkthdr;

	/* Initialize index of Tx pkthdr descriptor */
	for (i=0;i<RTL865X_SWNIC_TXRING_HW_PKTDESC;i++)
	{
		currTxPkthdrDescIndex[i] = 0;
		txPktDoneDescIndex[i]=0;
	}

	for(i=RTL865X_SWNIC_RXRING_HW_PKTDESC-1; i >= 0 ; i--)
	{
		/* Initialize index of current Rx pkthdr descriptor */
		currRxPkthdrDescIndex[i] = 0;
		/* Initialize index of current Rx Mbuf descriptor */
		currRxMbufDescIndex = 0;
		#ifdef DELAY_REFILL_ETH_RX_BUF
		rxDescReadyForHwIndex[i] = 0;
		rxDescCrossBoundFlag[i] = 0;
		#endif
#if 0		
		for (idx=0; idx<rxPkthdrRingCnt[i]; idx++)
		{
			if (!((rxPkthdrRing[i][idx] & DESC_OWNED_BIT) == DESC_RISC_OWNED)) {
				pPkthdr = (struct rtl_pktHdr *) (rxPkthdrRing[i][idx] &
					~(DESC_OWNED_BIT | DESC_WRAP));

				/*if(pPkthdr == NULL || pPkthdr->ph_mbuf == NULL)
				*	continue;
				*/
				if (pPkthdr->ph_mbuf->skb)
				{
					free_rx_buf(pPkthdr->ph_mbuf->skb);
					pPkthdr->ph_mbuf->skb = NULL;
				}

				pPkthdr->ph_mbuf->m_data = NULL;
				pPkthdr->ph_mbuf->m_extbuf = NULL;
		    	}
	    	}
#endif
	}
	if (rxMbufRing) {
		struct rtl_mBuf *pMbuf;

		for (i=0;i<rxMbufRingCnt;i++)
		{
			pMbuf = (struct rtl_mBuf *)(rxMbufRing[i] & ~(DESC_OWNED_BIT | DESC_WRAP));

			if (pMbuf->skb)
			{
				free_rx_buf(pMbuf->skb);
				pMbuf->skb = NULL;
		    	}
	    	}
	}
}

#if defined(CONFIG_RTL_REINIT_SWITCH_CORE)

#if	defined(DELAY_REFILL_ETH_RX_BUF)
int swNic_refillRxRing(void)
{
	unsigned long flags;
	unsigned int i;
	void *skb;
	unsigned char *buf;

	local_irq_save(flags);
	for(i =  0; i <RTL865X_SWNIC_RXRING_MAX_PKTDESC; i++)
	{
		while (return_to_rxing_check(i)) {
			skb=NULL;
			buf = alloc_rx_buf(&skb, size_of_cluster);

			if ((buf == NULL) ||(skb==NULL) ) {
				local_irq_restore(flags);
				return -1;
			}

			release_pkthdr(skb, i);
		}
		REG32(CPUIISR) = (MBUF_DESC_RUNOUT_IP_ALL|PKTHDR_DESC_RUNOUT_IP_ALL);
	}

	local_irq_restore(flags);
	return 0;
}
#endif

void swNic_freeTxRing(void)
{
	struct rtl_pktHdr	*pPkthdr;
	uint32 idx;
	unsigned long flags;

	local_irq_save(flags);
	//free_num = 0;

	/* Initialize index of Tx pkthdr descriptor */
	for (idx=0;idx<RTL865X_SWNIC_TXRING_HW_PKTDESC;idx++)
	{
			while (txPktDoneDescIndex[idx] != currTxPkthdrDescIndex[idx]) {

			#ifdef CONFIG_RTL8196C_REVISION_B
			if (rtl_chip_version == RTL8196C_REVISION_A)
				txPkthdrRing[idx][txPktDoneDescIndex[idx]] =txPkthdrRing_backup[idx][txPktDoneDescIndex[idx]] ;
			#endif

			pPkthdr = (struct rtl_pktHdr *) ((int32) txPkthdrRing[idx][txPktDoneDescIndex[idx]]
				& ~(DESC_OWNED_BIT | DESC_WRAP));

			if (pPkthdr->ph_mbuf->skb)
			{

				#if defined(CONFIG_RTL_FAST_BRIDGE)
				tx_done_callback(pPkthdr->ph_mbuf->skb);
				#else
				dev_kfree_skb_any((struct sk_buff *)pPkthdr->ph_mbuf->skb);
				#endif

				pPkthdr->ph_mbuf->skb = NULL;
			}

			txPkthdrRing[idx][txPktDoneDescIndex[idx]] &= ~DESC_SWCORE_OWNED;

			if (++txPktDoneDescIndex[idx] == txPkthdrRingCnt[idx])
				txPktDoneDescIndex[idx] = 0;

		}


	}


	local_irq_restore(flags);
	return ; //free_num;
}

int32 swNic_reConfigRxTxRing(void)
{
	uint32 i,j,k;
	//struct rtl_pktHdr	*pPkthdr;
	unsigned long flags;

	local_irq_save(flags);

	k = 0;

	for (i = 0; i < RTL865X_SWNIC_RXRING_HW_PKTDESC; i++)
	{
		for (j = 0; j < rxPkthdrRingCnt[i]; j++)
		{
			/* Setup descriptors */
			rxPkthdrRing[i][j] = rxPkthdrRing[i][j] | DESC_SWCORE_OWNED;
			rxMbufRing[k] = rxMbufRing[k]  | DESC_SWCORE_OWNED;
			k++;
		}

		/* Initialize index of current Rx pkthdr descriptor */
		currRxPkthdrDescIndex[i] = 0;

		/* Initialize index of current Rx Mbuf descriptor */
		currRxMbufDescIndex = 0;

		/* Set wrap bit of the last descriptor */
		if(rxPkthdrRingCnt[i] > 0)
			rxPkthdrRing[i][rxPkthdrRingCnt[i] - 1] |= DESC_WRAP;

		#ifdef DELAY_REFILL_ETH_RX_BUF
		rxDescReadyForHwIndex[i] = 0;
		rxDescCrossBoundFlag[i] = 0;
		#endif
	}

	rxMbufRing[rxMbufRingCnt - 1] |= DESC_WRAP;


	for (i=0;i<RTL865X_SWNIC_TXRING_HW_PKTDESC;i++)
	{
		currTxPkthdrDescIndex[i] = 0;
		txPktDoneDescIndex[i]=0;
	}

	/* Fill Tx packet header FDP */
	REG32(CPUTPDCR0) = (uint32) txPkthdrRing[0];
	REG32(CPUTPDCR1) = (uint32) txPkthdrRing[1];
	
#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
	REG32(CPUTPDCR2) = (uint32) txPkthdrRing[2];
	REG32(CPUTPDCR3) = (uint32) txPkthdrRing[3];
#endif


	/* Fill Rx packet header FDP */
	REG32(CPURPDCR0) = (uint32) rxPkthdrRing[0];
	REG32(CPURPDCR1) = (uint32) rxPkthdrRing[1];
	REG32(CPURPDCR2) = (uint32) rxPkthdrRing[2];
	REG32(CPURPDCR3) = (uint32) rxPkthdrRing[3];
	REG32(CPURPDCR4) = (uint32) rxPkthdrRing[4];
	REG32(CPURPDCR5) = (uint32) rxPkthdrRing[5];

	REG32(CPURMDCR0) = (uint32) rxMbufRing;

	local_irq_restore(flags);

	return 0;
}
int32 swNic_reInit(void)
{
	swNic_freeTxRing();
#if	defined(DELAY_REFILL_ETH_RX_BUF)
	swNic_refillRxRing();
#endif
	swNic_reConfigRxTxRing();
	return SUCCESS;
}

#endif
//#pragma ghs section text=default
/*************************************************************************
*   FUNCTION
*       swNic_init
*
*   DESCRIPTION
*       This function initializes descriptors and data structures.
*
*   INPUTS
*       userNeedRxPkthdrRingCnt[RTL865X_SWNIC_RXRING_HW_PKTDESC] :
*          Number of Rx pkthdr descriptors of each ring.
*       userNeedRxMbufRingCnt :
*          Number of Tx mbuf descriptors.
*       userNeedTxPkthdrRingCnt[RTL865X_SWNIC_TXRING_HW_PKTDESC] :
*          Number of Tx pkthdr descriptors of each ring.
*       clusterSize :
*          Size of cluster.
*
*   OUTPUTS
*       Status.
*************************************************************************/

int32 swNic_init(uint32 userNeedRxPkthdrRingCnt[RTL865X_SWNIC_RXRING_HW_PKTDESC],
                 uint32 userNeedRxMbufRingCnt,
                 uint32 userNeedTxPkthdrRingCnt[RTL865X_SWNIC_TXRING_HW_PKTDESC],
                 uint32 clusterSize)
{
	uint32 i, j, k;
	static uint32 totalRxPkthdrRingCnt = 0, totalTxPkthdrRingCnt = 0;
	static struct rtl_pktHdr *pPkthdrList_start;
	static struct rtl_mBuf *pMbufList_start;
	struct rtl_pktHdr *pPkthdrList;
	struct rtl_mBuf *pMbufList;
	struct rtl_pktHdr * pPkthdr;
	struct rtl_mBuf * pMbuf;
	unsigned long flags;
	int	ret;

	/* init const array for rx pre-process	*/
	extPortMaskToPortNum[0] = 5;
	extPortMaskToPortNum[1] = 6;
	extPortMaskToPortNum[2] = 7;
	extPortMaskToPortNum[3] = 5;
	extPortMaskToPortNum[4] = 8;
	extPortMaskToPortNum[5] = 5;
	extPortMaskToPortNum[6] = 5;
	extPortMaskToPortNum[7] = 5;

#if	defined(DELAY_REFILL_ETH_RX_BUF)
	rxPkthdrRefillThreshold[0] = ETH_REFILL_THRESHOLD;
	rxPkthdrRefillThreshold[1] = ETH_REFILL_THRESHOLD1;
	rxPkthdrRefillThreshold[2] = ETH_REFILL_THRESHOLD2;
	rxPkthdrRefillThreshold[3] = ETH_REFILL_THRESHOLD3;
	rxPkthdrRefillThreshold[4] = ETH_REFILL_THRESHOLD4;
	rxPkthdrRefillThreshold[5] = ETH_REFILL_THRESHOLD5;
#endif

	#if defined(CONFIG_RTL8196C_REVISION_B)
	rtl_chip_version = REG32(REVR);
	#endif

	ret = SUCCESS;
	local_irq_save(flags);
	if (rxMbufRing == NULL)
	{
		size_of_cluster = clusterSize;

		/* Allocate Rx descriptors of rings */
		for (i = 0; i < RTL865X_SWNIC_RXRING_HW_PKTDESC; i++) {
			rxPkthdrRingCnt[i] = userNeedRxPkthdrRingCnt[i];
			if (rxPkthdrRingCnt[i] == 0)
			{
				rxPkthdrRing[i] = NULL;
				continue;
			}

			rxPkthdrRing[i] = (uint32 *) UNCACHED_MALLOC(rxPkthdrRingCnt[i] * sizeof(uint32*));
			ASSERT_CSP( (uint32) rxPkthdrRing[i] & 0x0fffffff );

			totalRxPkthdrRingCnt += rxPkthdrRingCnt[i];
		}

		if (totalRxPkthdrRingCnt == 0) {
			ret = EINVAL;
			goto out;
		}

		/* Allocate Tx descriptors of rings */
		for (i = 0; i < RTL865X_SWNIC_TXRING_HW_PKTDESC; i++) {
			txPkthdrRingCnt[i] = userNeedTxPkthdrRingCnt[i];

			if (txPkthdrRingCnt[i] == 0)
			{
				txPkthdrRing[i] = NULL;
				continue;
			}

			txPkthdrRing[i] = (uint32 *) UNCACHED_MALLOC(txPkthdrRingCnt[i] * sizeof(uint32*));
			#ifdef CONFIG_RTL8196C_REVISION_B
			if (rtl_chip_version == RTL8196C_REVISION_A)
				txPkthdrRing_backup[i]=(uint32 *) UNCACHED_MALLOC(txPkthdrRingCnt[i] * sizeof(uint32));
			#endif

			ASSERT_CSP( (uint32) txPkthdrRing[i] & 0x0fffffff );

			totalTxPkthdrRingCnt += txPkthdrRingCnt[i];
		}

		if (totalTxPkthdrRingCnt == 0) {
			ret = EINVAL;
			goto out;
		}

		/* Allocate MBuf descriptors of rings */
		rxMbufRingCnt = userNeedRxMbufRingCnt;

		if (userNeedRxMbufRingCnt == 0) {
			ret = EINVAL;
			goto out;
		}

		rxMbufRing = (uint32 *) UNCACHED_MALLOC((rxMbufRingCnt+RESERVERD_MBUF_RING_NUM) * sizeof(uint32*));
		ASSERT_CSP( (uint32) rxMbufRing & 0x0fffffff );

		/* Allocate pkthdr */
		pPkthdrList_start = (struct rtl_pktHdr *) UNCACHED_MALLOC(
		(totalRxPkthdrRingCnt + totalTxPkthdrRingCnt) * sizeof(struct rtl_pktHdr));
		ASSERT_CSP( (uint32) pPkthdrList_start & 0x0fffffff );

		/* Allocate mbufs */
		pMbufList_start = (struct rtl_mBuf *) UNCACHED_MALLOC(
		(rxMbufRingCnt+RESERVERD_MBUF_RING_NUM+ totalTxPkthdrRingCnt) * sizeof(struct rtl_mBuf));
		ASSERT_CSP( (uint32) pMbufList_start & 0x0fffffff );

	}

	/* Initialize interrupt statistics counter */
	//rxPktCounter = txPktCounter = 0;

	/* Initialize index of Tx pkthdr descriptor */
	for (i=0;i<RTL865X_SWNIC_TXRING_HW_PKTDESC;i++)
	{
		currTxPkthdrDescIndex[i] = 0;
		txPktDoneDescIndex[i]=0;
	}

	pPkthdrList = pPkthdrList_start;
	pMbufList = pMbufList_start;

	/* Initialize Tx packet header descriptors */
	for (i = 0; i < RTL865X_SWNIC_TXRING_HW_PKTDESC; i++)
	{
		for (j = 0; j < txPkthdrRingCnt[i]; j++)
		{
			/* Dequeue pkthdr and mbuf */
			pPkthdr = pPkthdrList++;
			pMbuf = pMbufList++;

			bzero((void *) pPkthdr, sizeof(struct rtl_pktHdr));
			bzero((void *) pMbuf, sizeof(struct rtl_mBuf));

			pPkthdr->ph_mbuf = pMbuf;
			pPkthdr->ph_len = 0;
			pPkthdr->ph_flags = PKTHDR_USED | PKT_OUTGOING;
			pPkthdr->ph_type = PKTHDR_ETHERNET;
			pPkthdr->ph_portlist = 0;

			pMbuf->m_next = NULL;
			pMbuf->m_pkthdr = pPkthdr;
			pMbuf->m_flags = MBUF_USED | MBUF_EXT | MBUF_PKTHDR | MBUF_EOR;
			pMbuf->m_data = NULL;
			pMbuf->m_extbuf = NULL;
			pMbuf->m_extsize = 0;

			txPkthdrRing[i][j] = (int32) pPkthdr | DESC_RISC_OWNED;
			#ifdef CONFIG_RTL8196C_REVISION_B
			if (rtl_chip_version == RTL8196C_REVISION_A)
				txPkthdrRing_backup[i][j]=(int32) pPkthdr | DESC_RISC_OWNED;
			#endif
		}

		if(txPkthdrRingCnt[i] > 0)
		{
			/* Set wrap bit of the last descriptor */
			txPkthdrRing[i][txPkthdrRingCnt[i] - 1] |= DESC_WRAP;
			#ifdef CONFIG_RTL8196C_REVISION_B
			if (rtl_chip_version == RTL8196C_REVISION_A)
				txPkthdrRing_backup[i][txPkthdrRingCnt[i] - 1] |= DESC_WRAP;
			#endif
		}

	}

	/* Fill Tx packet header FDP */
	REG32(CPUTPDCR0) = (uint32) txPkthdrRing[0];
	REG32(CPUTPDCR1) = (uint32) txPkthdrRing[1];

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
	REG32(CPUTPDCR2) = (uint32) txPkthdrRing[2];
	REG32(CPUTPDCR3) = (uint32) txPkthdrRing[3];
#endif

	/* Initialize Rx packet header descriptors */
	k = 0;

	for (i = 0; i < RTL865X_SWNIC_RXRING_HW_PKTDESC; i++)
	{
		for (j = 0; j < rxPkthdrRingCnt[i]; j++)
		{
			/* Dequeue pkthdr and mbuf */
			pPkthdr = pPkthdrList++;
			pMbuf = pMbufList++;

			bzero((void *) pPkthdr, sizeof(struct rtl_pktHdr));
			bzero((void *) pMbuf, sizeof(struct rtl_mBuf));

			/* Setup pkthdr and mbuf */
			pPkthdr->ph_mbuf = pMbuf;
			pPkthdr->ph_len = 0;
			pPkthdr->ph_flags = PKTHDR_USED | PKT_INCOMING;
			pPkthdr->ph_type = PKTHDR_ETHERNET;
			pPkthdr->ph_portlist = 0;
			pMbuf->m_next = NULL;
			pMbuf->m_pkthdr = pPkthdr;
			pMbuf->m_len = 0;
			pMbuf->m_flags = MBUF_USED | MBUF_EXT | MBUF_PKTHDR | MBUF_EOR;
			pMbuf->m_extsize = size_of_cluster;
			pMbuf->m_data = pMbuf->m_extbuf = alloc_rx_buf(&pPkthdr->ph_mbuf->skb, size_of_cluster);

			/* Setup descriptors */
			rxPkthdrRing[i][j] = (int32) pPkthdr | DESC_SWCORE_OWNED;
			rxMbufRing[k++] = (int32) pMbuf | DESC_SWCORE_OWNED;
		}

		/* Initialize index of current Rx pkthdr descriptor */
		currRxPkthdrDescIndex[i] = 0;

		/* Initialize index of current Rx Mbuf descriptor */
		currRxMbufDescIndex = 0;

		/* Set wrap bit of the last descriptor */
		if(rxPkthdrRingCnt[i] > 0)
			rxPkthdrRing[i][rxPkthdrRingCnt[i] - 1] |= DESC_WRAP;

		#if	defined(DELAY_REFILL_ETH_RX_BUF)
		rxDescReadyForHwIndex[i] = 0;
		rxDescCrossBoundFlag[i] = 0;
		#endif
	}

	rxMbufRing[rxMbufRingCnt - 1] |= DESC_WRAP;
	#if 0
	for (i = 0; i < RESERVERD_MBUF_RING_NUM; i++)
	{
		pMbuf = pMbufList++;
		bzero((void *) pMbuf, sizeof(struct rtl_mBuf));
		pMbuf->m_next = NULL;
		pMbuf->m_len = 0;
		pMbuf->m_flags = MBUF_USED | MBUF_EXT | MBUF_PKTHDR | MBUF_EOR;
		pMbuf->m_extsize = size_of_cluster;
		pMbuf->m_data = pMbuf->m_extbuf = alloc_rx_buf(&pMbuf->skb, size_of_cluster);
		rxMbufRing[rxMbufRingCnt+i] = (int32) pMbuf | DESC_SWCORE_OWNED|DESC_WRAP;
	}
	#endif
	/* Fill Rx packet header FDP */
	REG32(CPURPDCR0) = (uint32) rxPkthdrRing[0];
	REG32(CPURPDCR1) = (uint32) rxPkthdrRing[1];
	REG32(CPURPDCR2) = (uint32) rxPkthdrRing[2];
	REG32(CPURPDCR3) = (uint32) rxPkthdrRing[3];
	REG32(CPURPDCR4) = (uint32) rxPkthdrRing[4];
	REG32(CPURPDCR5) = (uint32) rxPkthdrRing[5];

	REG32(CPURMDCR0) = (uint32) rxMbufRing;

out:
	local_irq_restore(flags);
	return ret;
}


#ifdef FAT_CODE
/*************************************************************************
*   FUNCTION
*       swNic_resetDescriptors
*
*   DESCRIPTION
*       This function resets descriptors.
*
*   INPUTS
*       None.
*
*   OUTPUTS
*       None.
*************************************************************************/
void swNic_resetDescriptors(void)
{
    /* Disable Tx/Rx and reset all descriptors */
    REG32(CPUICR) &= ~(TXCMD | RXCMD);
    return;
}
#endif//FAT_CODE

#if 	defined(CONFIG_RTL_PROC_DEBUG)
/*	dump the rx ring info	*/
int32	rtl_dumpRxRing(void)
{
	int	idx, cnt;
	struct rtl_pktHdr * pPkthdr;

	for(idx=0;idx<RTL865X_SWNIC_RXRING_HW_PKTDESC;idx++)
	{
#if	defined(DELAY_REFILL_ETH_RX_BUF)
		printk("**********************************************\nRxRing%d: cnt %d crossFlags[%d]\n",
			idx, rxPkthdrRingCnt[idx], rxDescCrossBoundFlag[idx]);
#else
		printk("**********************************************\nRxRing%d: cnt %d\n",
			idx, rxPkthdrRingCnt[idx]);
#endif
		/*	skip the null rx ring */
		if (rxPkthdrRingCnt[idx]==0)
			continue;

		/*	dump all the pkt header	*/
		for(cnt=0;cnt<rxPkthdrRingCnt[idx];cnt++)
		{
			pPkthdr = (struct rtl_pktHdr *) (rxPkthdrRing[idx][cnt] & ~(DESC_OWNED_BIT | DESC_WRAP));

#if	defined(DELAY_REFILL_ETH_RX_BUF)
			printk("  idx[%03d]: 0x%p-->mbuf[0x%p],skb[0x%p]%s%s%s%s\n",  cnt, pPkthdr, pPkthdr->ph_mbuf, pPkthdr->ph_mbuf->skb,
				(rxPkthdrRing[idx][cnt]&DESC_OWNED_BIT)==DESC_RISC_OWNED?" :CPU":" :SWCORE",
				(rxPkthdrRing[idx][cnt]&DESC_WRAP)!=0?" :WRAP":"",
				cnt==currRxPkthdrDescIndex[idx]?"  <===currIdx":"",
				cnt ==rxDescReadyForHwIndex[idx]?" <===readyForHw":"");
#else
			printk("  idx[%03d]: 0x%p-->mbuf[0x%p],skb[0x%p]%s%s%s\n",  cnt, pPkthdr, pPkthdr->ph_mbuf, pPkthdr->ph_mbuf->skb,
				(rxPkthdrRing[idx][cnt]&DESC_OWNED_BIT)==DESC_RISC_OWNED?" :CPU":" :SWCORE",
				(rxPkthdrRing[idx][cnt]&DESC_WRAP)!=0?" :WRAP":"",
				cnt==currRxPkthdrDescIndex[idx]?"  <===currIdx":"");
#endif
		}
	}
	return SUCCESS;
}

/*	dump the tx ring info	*/
int32	rtl_dumpTxRing(void)
{
	int	idx, cnt;
	struct rtl_pktHdr * pPkthdr = NULL;

	for(idx=0;idx<RTL865X_SWNIC_TXRING_HW_PKTDESC;idx++)
	{
		printk("**********************************************\nTxRing%d: cnt %d\n",
			idx, txPkthdrRingCnt[idx]);

		/*	skip the null rx ring */
		if (txPkthdrRingCnt[idx]==0)
			continue;

		/*	dump all the pkt header	*/
		for(cnt=0;cnt<txPkthdrRingCnt[idx];cnt++)
		{
 #ifdef CONFIG_RTL8196C_REVISION_B
		  if (rtl_chip_version == RTL8196C_REVISION_A)
			pPkthdr = (struct rtl_pktHdr *) (txPkthdrRing_backup[idx][cnt] & ~(DESC_OWNED_BIT | DESC_WRAP));
		  else
#endif
			pPkthdr = (struct rtl_pktHdr *) (txPkthdrRing[idx][cnt] & ~(DESC_OWNED_BIT | DESC_WRAP));

			printk("  idx[%03d]: 0x%p-->mbuf[0x%p],skb[0x%p]%s%s%s%s\n",  cnt, pPkthdr, pPkthdr->ph_mbuf, pPkthdr->ph_mbuf->skb,
				(txPkthdrRing[idx][cnt]&DESC_OWNED_BIT)==DESC_RISC_OWNED?" :CPU":" :SWCORE",
				(txPkthdrRing[idx][cnt]&DESC_WRAP)!=0?" :WRAP":"",
				cnt==currTxPkthdrDescIndex[idx]?"  <===currIdx":"",
				cnt==txPktDoneDescIndex[idx]?"  <===txDoneIdx":"");
		}
	}
	return SUCCESS;
}

/*	dump the tx ring info	*/
int32	rtl_dumpMbufRing(void)
{
	int	idx;
	struct rtl_mBuf *mbuf;

	idx = 0;
	printk("**********************************************\nMbufRing:\n");
	while(1)
	{
		mbuf = (struct rtl_mBuf *)(rxMbufRing[idx] & ~(DESC_OWNED_BIT | DESC_WRAP));
		printk("mbuf[%03d]: 0x%p: ==> pkthdr[0x%p] ==> skb[0x%p]%s%s%s\n", idx, mbuf, mbuf->m_pkthdr,
				mbuf->skb,
				(rxMbufRing[idx]&DESC_OWNED_BIT)==DESC_RISC_OWNED?" :CPU":" :SWCORE",
				(rxMbufRing[idx]&DESC_WRAP)==DESC_ENG_OWNED?" :WRAP":"",
				idx==currRxMbufDescIndex?"  <===currIdx":"");
			if ((rxMbufRing[idx]&DESC_WRAP)!=0)
				break;
			idx++;
	}
	return SUCCESS;
}

/* dump brief info */
int32 rtl_dumpIndexs(void)
{
	int	i;

	printk("Dump RX infos:\n");
#if	defined(DELAY_REFILL_ETH_RX_BUF)
	for(i=0;i<RTL865X_SWNIC_RXRING_HW_PKTDESC;i++) {
		printk("	TotalCnt: %d, currPkthdrIdx: %d currMbufIdx: %d readyForHwIdx: %d crossBoundFlag: %d.\n",
			rxPkthdrRingCnt[i], currRxPkthdrDescIndex[i], currRxMbufDescIndex, rxDescReadyForHwIndex[i], rxDescCrossBoundFlag[i]);
	}
#else
	for(i=0;i<RTL865X_SWNIC_RXRING_HW_PKTDESC;i++) {
		printk("	TotalCnt: %d, currPkthdrIdx: %d currMbufIdx: %d\n",
			rxPkthdrRingCnt[i], currRxPkthdrDescIndex[i], currRxMbufDescIndex);
	}
#endif

	printk("\nDump TX infos:\n");
	for(i=0;i<RTL865X_SWNIC_TXRING_HW_PKTDESC;i++) {
		printk("	TotalCnt: %d, currPkthdrIdx: %d pktDoneIdx: %d.\n",
			txPkthdrRingCnt[i], currTxPkthdrDescIndex[i], txPktDoneDescIndex[i]);
	}

	return SUCCESS;
}
#endif


