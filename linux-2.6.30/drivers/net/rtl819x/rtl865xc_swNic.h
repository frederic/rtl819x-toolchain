/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002
* All rights reserved.
*
* $Header: /home/cvsroot/linux-2.6.19/linux-2.6.x/drivers/net/re865x/rtl865xc_swNic.h,v 1.3 2008/04/11 10:12:38 bo_zhao Exp $
*
* Abstract: Switch core polling mode NIC header file.
*
* $Author: bo_zhao $
*
* $Log: rtl865xc_swNic.h,v $
* Revision 1.3  2008/04/11 10:12:38  bo_zhao
* *: swap nic drive to 8186 style
*
* Revision 1.2  2007/11/11 02:51:27  davidhsu
* Fix the bug that do not fre rx skb in rx descriptor when driver is shutdown
*
* Revision 1.1.1.1  2007/08/06 10:04:52  root
* Initial import source to CVS
*
* Revision 1.4  2006/09/15 03:53:39  ghhuang
* +: Add TFTP download support for RTL8652 FPGA
*
* Revision 1.3  2005/09/22 05:22:31  bo_zhao
* *** empty log message ***
*
* Revision 1.1.1.1  2005/09/05 12:38:24  alva
* initial import for add TFTP server
*
* Revision 1.2  2004/03/31 01:49:20  yjlou
* *: all text files are converted to UNIX format.
*
* Revision 1.1  2004/03/16 06:36:13  yjlou
* *** empty log message ***
*
* Revision 1.1.1.1  2003/09/25 08:16:56  tony
*  initial loader tree
*
* Revision 1.1.1.1  2003/05/07 08:16:07  danwu
* no message
*
* ---------------------------------------------------------------
*/


#ifndef RTL865XC_SWNIC_H
#define	RTL865XC_SWNIC_H

#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
//#define	RTL_MULTIPLE_RX_TX_RING		1	/*enable multiple input queue(multiple rx ring)*/
#endif

#define RTL865X_SWNIC_RXRING_HW_PKTDESC	6

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) 
#define RTL865X_SWNIC_TXRING_HW_PKTDESC	4
#else
#define RTL865X_SWNIC_TXRING_HW_PKTDESC	2
#endif

#define RESERVERD_MBUF_RING_NUM			8

#if defined(CONFIG_RTL_NFJROM_MP)
	#define MAX_PRE_ALLOC_RX_SKB		64
	#define NUM_RX_PKTHDR_DESC		8
	#define NUM_TX_PKTHDR_DESC		64
	#define	ETH_REFILL_THRESHOLD		8	// must < NUM_RX_PKTHDR_DESC
#elif defined(CONFIG_RTL_8198) && !defined(CONFIG_RTL_8198_AP_ROOT)
	#define MAX_PRE_ALLOC_RX_SKB		512
  #ifdef CONFIG_RTL_ULINKER
	#define NUM_RX_PKTHDR_DESC			510
  #else
	#define NUM_RX_PKTHDR_DESC			512
  #endif
	#define NUM_TX_PKTHDR_DESC			1024
	#define	ETH_REFILL_THRESHOLD		8	// must < NUM_RX_PKTHDR_DESC
#else
	#ifdef DELAY_REFILL_ETH_RX_BUF
	#define MAX_PRE_ALLOC_RX_SKB		192
	#define NUM_RX_PKTHDR_DESC			256
	#define	ETH_REFILL_THRESHOLD		8	// must < NUM_RX_PKTHDR_DESC
	#else
	#define MAX_PRE_ALLOC_RX_SKB		256
	#define NUM_RX_PKTHDR_DESC			256
	#endif
	#define NUM_TX_PKTHDR_DESC			128
#endif

#if defined(RTL_MULTIPLE_RX_TX_RING)

#define	RTL_CPU_QOS_ENABLED		1

#define	RTL865X_SWNIC_RXRING_MAX_PKTDESC    6
#define	RTL865X_SWNIC_TXRING_MAX_PKTDESC    2
#define	RTL_CPU_RX_RING_NUM			4
/*	By default, only using rxring 0 and rxring 5
*	in order to make different between low/high
*	priority
*/
#define	NUM_RX_PKTHDR_DESC1			2
#define	NUM_RX_PKTHDR_DESC2			2
#define	NUM_RX_PKTHDR_DESC3			2
#define	NUM_RX_PKTHDR_DESC4			2
#define	NUM_RX_PKTHDR_DESC5			NUM_RX_PKTHDR_DESC
#define	NUM_TX_PKTHDR_DESC1			NUM_TX_PKTHDR_DESC

#define	ETH_REFILL_THRESHOLD1	0	// must < NUM_RX_PKTHDR_DESC1
#define	ETH_REFILL_THRESHOLD2	0	// must < NUM_RX_PKTHDR_DESC2
#define	ETH_REFILL_THRESHOLD3	0	// must < NUM_RX_PKTHDR_DESC3
#define	ETH_REFILL_THRESHOLD4	0	// must < NUM_RX_PKTHDR_DESC4
#define	ETH_REFILL_THRESHOLD5	ETH_REFILL_THRESHOLD	// must < NUM_RX_PKTHDR_DESC5

#define	QUEUEID0_RXRING_MAPPING		0x0000
#define	QUEUEID1_RXRING_MAPPING		0x0000
#define	QUEUEID2_RXRING_MAPPING		0x5555
#define	QUEUEID3_RXRING_MAPPING		0x5555
#define	QUEUEID4_RXRING_MAPPING		0x5555
#define	QUEUEID5_RXRING_MAPPING		0x5555
#else
#define	RTL865X_SWNIC_RXRING_MAX_PKTDESC    1
#define	RTL865X_SWNIC_TXRING_MAX_PKTDESC    1
#define	RTL_CPU_RX_RING_NUM			1
#define	NUM_RX_PKTHDR_DESC1		2
#define	NUM_RX_PKTHDR_DESC2		2
#define	NUM_RX_PKTHDR_DESC3		2
#define	NUM_RX_PKTHDR_DESC4		2
#define	NUM_RX_PKTHDR_DESC5		2
#define	NUM_TX_PKTHDR_DESC1		2

#define	ETH_REFILL_THRESHOLD1	0	// must < NUM_RX_PKTHDR_DESC
#define	ETH_REFILL_THRESHOLD2	0	// must < NUM_RX_PKTHDR_DESC
#define	ETH_REFILL_THRESHOLD3	0	// must < NUM_RX_PKTHDR_DESC
#define	ETH_REFILL_THRESHOLD4	0	// must < NUM_RX_PKTHDR_DESC
#define	ETH_REFILL_THRESHOLD5	0	// must < NUM_RX_PKTHDR_DESC

#define	QUEUEID0_RXRING_MAPPING		0
#define	QUEUEID1_RXRING_MAPPING		0
#define	QUEUEID2_RXRING_MAPPING		0
#define	QUEUEID3_RXRING_MAPPING		0
#define	QUEUEID4_RXRING_MAPPING		0
#define	QUEUEID5_RXRING_MAPPING		0
#endif

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) 
#define	NUM_TX_PKTHDR_DESC2		2
#define	NUM_TX_PKTHDR_DESC3		2
#endif

/* refer to rtl865xc_swNic.c & rtl865xc_swNic.h
 */
#define	UNCACHE_MASK   0x20000000

/* rxPreProcess */
#define	RTL8651_CPU_PORT                0x07 /* in rtl8651_tblDrv.h */
#define	_RTL865XB_EXTPORTMASKS   7

typedef struct {
	uint16			vid;
	uint16			pid;
	uint16			len;
	uint16			priority:3;
	uint16			rxPri:3;
	void* 			input;
	struct dev_priv*	priv;
	uint32			isPdev;
#if defined(CONFIG_RTL_STP)
	int8				isStpVirtualDev;
#endif
}	rtl_nicRx_info;

typedef struct {
	uint16		vid;
	uint16		portlist;
	uint16		srcExtPort;
	uint16		flags;
	uint32		txIdx:1;
#if defined(CONFIG_RTL_HW_QOS_SUPPORT) || defined(CONFIG_RTL_QOS_PATCH)|| defined(CONFIG_RTK_VOIP_QOS)
	uint32		priority:3;
	uint32		queueId:3;
#endif
#if defined(CONFIG_RTK_VLAN_WAN_TAG_SUPPORT)
	uint16		tagport;
#endif
	void 			*out_skb;
}	rtl_nicTx_info;


#if defined(RTL_CPU_QOS_ENABLED)
#define	RTL_NIC_QUEUE_LEN					(32)
#define	RTL_CPUQOS_PKTCNT_THRESHOLD	(1000)
#define	RTL_ASSIGN_RX_PRIORITY			((highestPriority<cpuQosHoldLow)?((totalLowQueueCnt<RTL_CPUQOS_PKTCNT_THRESHOLD)?highestPriority:cpuQosHoldLow):highestPriority)

typedef struct {
	int	cnt;
	int	start;
	int	end;
	rtl_nicRx_info	entry[RTL_NIC_QUEUE_LEN];
}	rtl_queue_entry;
#else
#define	RTL_ASSIGN_RX_PRIORITY			0
#endif
/* --------------------------------------------------------------------
 * ROUTINE NAME - swNic_init
 * --------------------------------------------------------------------
 * FUNCTION: This service initializes the switch NIC.
 * INPUT   :
        userNeedRxPkthdrRingCnt[RTL865X_SWNIC_RXRING_MAX_PKTDESC]: Number of Rx pkthdr descriptors. of each ring
        userNeedRxMbufRingCnt: Number of Rx mbuf descriptors.
        userNeedTxPkthdrRingCnt[RTL865X_SWNIC_TXRING_MAX_PKTDESC]: Number of Tx pkthdr descriptors. of each ring
        clusterSize: Size of a mbuf cluster.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns ENOERR.
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swNic_init(uint32 userNeedRxPkthdrRingCnt[],
                 uint32 userNeedRxMbufRingCnt,
                 uint32 userNeedTxPkthdrRingCnt[],
                 uint32 clusterSize);



/* --------------------------------------------------------------------
 * ROUTINE NAME - swNic_intHandler
 * --------------------------------------------------------------------
 * FUNCTION: This function is the NIC interrupt handler.
 * INPUT   :
		intPending: Pending interrupts.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
void swNic_intHandler(uint32 intPending);
int32 swNic_flushRxRingByPriority(int priority);
__MIPS16 __IRAM_FWD int32 swNic_receive(rtl_nicRx_info *info, int retryCount);
int32 swNic_send(void *skb, void * output, uint32 len, rtl_nicTx_info *nicTx);
//__MIPS16
int32 swNic_txDone(int idx);
void swNic_freeRxBuf(void);
int32	swNic_txRunout(void);
#if defined(DELAY_REFILL_ETH_RX_BUF)
extern int check_rx_pkthdr_ring(int idx, int *return_idx);
extern int check_and_return_to_rx_pkthdr_ring(void *skb, int idx);
extern int return_to_rx_pkthdr_ring(unsigned char *head);
#endif
extern	uint32* rxMbufRing;
extern unsigned char *alloc_rx_buf(void **skb, int buflen);
extern void free_rx_buf(void *skb);
#if defined(CONFIG_RTL_FAST_BRIDGE)
extern void tx_done_callback(void *skb);
#endif
extern void eth_save_and_cli(unsigned long *flags);
extern void eth_restore_flags(unsigned long flags);

#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
int32 swNic_setVlanPortTag(int portmask);
#endif

#define	RTL8651_IOCTL_GETWANLINKSTATUS			2000
#define	RTL8651_IOCTL_GETLANLINKSTATUS			2001
#define	RTL8651_IOCTL_GETWANTHROUGHPUT			2002
#define	RTL8651_IOCTL_GETLANPORTLINKSTATUS		2003
#define	RTL8651_IOCTL_GETWANPORTLINKSTATUS		2004
#define 	RTL8651_IOCTL_GETWANLINKSPEED 			2100
//#define 	RTL8651_IOCTL_SETWANLINKSPEED 			2101

#define RTL8651_IOCTL_GETLANLINKSTATUSALL		2105

#define	RTL8651_IOCTL_SETWANLINKSTATUS			2200


#define	RTL_NICRX_OK	0
#define	RTL_NICRX_REPEAT	-2
#define	RTL_NICRX_NULL	-1
#if defined(CONFIG_RTL_REINIT_SWITCH_CORE)
int32 swNic_reInit(void);
#endif
#if 	defined(CONFIG_RTL_PROC_DEBUG)
int32	rtl_dumpRxRing(void);
int32	rtl_dumpTxRing(void);
int32	rtl_dumpMbufRing(void);
int32	rtl_dumpIndexs(void);
#endif

struct ring_que {
	int qlen;
	int qmax;
	int head;
	int tail;
	struct sk_buff **ring;
};

static inline void *UNCACHED_MALLOC(int size)
{
	return ((void *)(((uint32)kmalloc(size, GFP_ATOMIC)) | UNCACHE_MASK));
}

#endif /* _SWNIC_H */
