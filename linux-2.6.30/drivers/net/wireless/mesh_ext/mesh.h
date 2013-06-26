/*!	\file	mesh.h
	\brief	None

	\date 2006/12/22 
	\author	stanley, popen and owen
*/
#ifndef _8190S_MESH_PUB_HDR_H_
#define _8190S_MESH_PUB_HDR_H_

#ifdef __KERNEL__
#include <linux/list.h>
#include <linux/circ_buf.h>
#include <linux/pci.h>
#include <linux/random.h>
#include <linux/config.h>
#endif

#include "mesh_cfg.h"
#ifdef CONFIG_RTL8192CD
#include "../rtl8192cd/8192cd_cfg.h"
#include "../rtl8192cd/8192cd_debug.h"
#include "../rtl8192cd/wifi.h"
#else
#include "../rtl8190/8190n_cfg.h"
#include "../rtl8190/8190n_debug.h"
#include "../rtl8190/wifi.h"
#endif

#ifdef CONFIG_RTL8192CD
typedef struct rtl8192cd_priv DRV_PRIV;
#define DRV_FIRETX rtl8192cd_firetx
#define DRV_RX_MGNTFRAME rtl8192cd_rx_mgntframe
#define DRV_RX_DATA rtl8192cd_rx_dataframe
#define WLAN_TX rtl8192cd_wlantx
#define RTL_PRODUCT_CLEINT is_rtl8192cd_apclient
#else
typedef struct rtl8190_priv DRV_PRIV;
#define DRV_FIRETX rtl8190_firetx
#define DRV_RX_MGNTFRAME rtl8190_rx_mgntframe
#define DRV_RX_DATA rtl8190_rx_dataframe
#define WLAN_TX rtl8190_wlantx
#define RTL_PRODUCT_CLEINT is_rtl8190_apclient
#endif

//#define proxy_debug panic_printk
#define proxy_debug
//#define txsc_debug panic_printk
#define txsc_debug
//#define txsc_debug_dump hex_dump
#define txsc_debug_dump

#define RELAY_11S 8

/**
 *	@brief	define
 *
 */
//#define _MESH_DEBUG_ 
#undef _MESH_DEBUG_

#ifdef __LINUX_2_6__
#define DBG_NCTU_MESH 		0x1
#define MESH_DBG_COMPLEX	0x2
#define MESH_DBG_TEST		0x4
#define MESH_DBG_SIMPLE		0x8
#define MESH_DBG_LV		0xf9
#else
//#define _MESH_DEBUG_
#endif
//#define briansay(msg,args...) printk(msg, ## args);

#define MESH_DEBUG_MSG(msg, args...)		printk(msg,  ## args)

#ifdef  _MESH_DEBUG_

//#define MESH_DBG_COMPLEX	(1<<7)
//#define MESH_DBG_TEST		(1<<6)
//#define MESH_DBG_SIMPLE	(1)

#define MESH_DEBUG_MSG(msg, args...)		printk(KERN_ERR msg,  ## args)
//#define MESH_BOOTSEQ_STRESS_TEST

//#define MESH_DBG_LV		0
// #define MESH_DBG_LV		MESH_DBG_SIMPLE
//#define MESH_DBG_LV		(MESH_DBG_COMPLEX | MESH_DBG_SIMPLE)

#else
//#define MESH_DEBUG_MSG(msg, args...)
#undef MESH_BOOTSEQ_STRESS_TEST
#endif

/////////////////////////////////////////////
#undef _MESH_PATHSEL_DEBUG_

//#define _MESH_PATHSEL_DEBUG_

#ifdef  _MESH_PATHSEL_DEBUG_

#define MESH_PATHSEL_DBG_COMPLEX	(1<<7)
#define MESH_PATHSEL_DBG_TEST		(1<<6)
#define MESH_PATHSEL_DBG_SIMPLE		(1)

// #define MESH_DEBUG_MSG(msg, args...)		printk(KERN_ERR msg,  ## args)
//#define MESH_DEBUG_MSG(msg, args...)

// #define MESH_DBG_LV					0
//#define MESH_PATHSEL_DBG_LV			MESH_PATHSEL_DBG_SIMPLE
// #define MESH_DBG_LV					(MESH_DBG_COMPLEX | MESH_DBG_SIMPLE)

#else
#define MESH_DEBUG_MSG1(msg, args...)
#endif




/**
 *	@brief	Time to  jiffies 
 *
 *	@param	time: (unit=1ms)
 *
 *	@retval	jiffies
 */
#define CALC_TIME_TO_JIFFIES(time)		((time*HZ)/1000)

// Define of time out jiffies
#define MESH_EXPIRE_TO				CALC_TIME_TO_JIFFIES(15000)	// MP MAX  idle time
#define MESH_TIMER_TO				CALC_TIME_TO_JIFFIES(250)	// MP mesh_unEstablish_hdr  peer link expire  timer.
#define MESH_LocalLinkStateANNOU_TO	CALC_TIME_TO_JIFFIES(5000)	// MP Local Link State Announcement time.

// Define of time out Stress test (Association expire time)
#ifdef	MESH_BOOTSEQ_STRESS_TEST
#define MESH_BS_STRESS_TEST_MIN_TIME	CALC_TIME_TO_JIFFIES(1000)
#define MESH_BS_STRESS_TEST_MAX_TIME	CALC_TIME_TO_JIFFIES(5000)
#endif	// MESH_BOOTSEQ_STRESS_TEST

#ifdef MESH_BOOTSEQ_AUTH
// Auth
#define MESH_AUTH_RETRY_TO			CALC_TIME_TO_JIFFIES(500)	// AUTH resend Authentication request time
#define MESH_AUTH_REQUEST_TO		CALC_TIME_TO_JIFFIES(1000)	// AUTH   after recived RSP , Betweend RSP and REQ time
#define MESH_AUTH_LISTEN_TO			CALC_TIME_TO_JIFFIES(5000)	// MP connection time
#endif

// peer link
#define MESH_PEER_LINK_RETRY_TO		CALC_TIME_TO_JIFFIES(500)	// start peer link resend Association request time
#define MESH_PEER_LINK_OPEN_TO		CALC_TIME_TO_JIFFIES(1000)	// peer link after recived  confirm, Between confirm and open time
#define MESH_PEER_LINK_CLOSE_TO		CALC_TIME_TO_JIFFIES(1000)	// peer link close wait time
#define MESH_PEER_LINK_LISTEN_TO	CALC_TIME_TO_JIFFIES(5000)	// MP connection time

// Retry 
#define MESH_AUTH_RETRY_LIMIT		6		// Retry AUTH count
#define MESH_PEER_LINK_RETRY_LIMIT	6		// Retry PeerLink count

/*
 *	Length setting
*/

#define MESH_ID_LEN 				32		// Mesh ID
#define MESH_PS_IDENT_OUI_LEN 		3		// Mesh Path Select Identifier OUI
#define MESH_PS_IDENT_VALUE_LEN 	1		// Mesh Path Select Identifier VALUE
#define MESH_LINK_ID_LEN 			4		// Mesh PeerLinkID & LocalLinkID
#define MESH_VERSION_LEN			1		// IE Version
#define MESH_IE_BASE_LEN			2		// IE  ID + LEN

// WLAN mesh capacity
#define	MESH_CAP_PROTOCOL_LEN		MESH_PS_IDENT_OUI_LEN + MESH_PS_IDENT_VALUE_LEN	// Active Protocol ID
#define	MESH_CAP_METRIC_LEN			MESH_PS_IDENT_OUI_LEN + MESH_PS_IDENT_VALUE_LEN	// Active Metric ID
#define	MESH_CAP_PEER_CAP_LEN		2		// Peer Capacity
#define	MESH_CAP_POWERSAVE_CAP_LEN	1		// Power Save Capacity
#define	MESH_CAP_SYNC_CAP_LEN		1		// Synchronization Capacity
#define	MESH_CAP_MDA_CAP_LEN		1		// MDA Capacity
#define	MESH_CAP_CH_PRECEDENCE_LEN	4		// Channel precedence

#define	MESH_CAP_VERSION_OFFSET		MESH_IE_BASE_LEN
#define	MESH_CAP_PROTOCOL_OFFSET	MESH_CAP_VERSION_OFFSET + MESH_VERSION_LEN
#define	MESH_CAP_METRIC_OFFSET		MESH_CAP_PROTOCOL_OFFSET + MESH_CAP_PROTOCOL_LEN
#define	MESH_CAP_PEER_CAP_OFFSET	MESH_CAP_METRIC_OFFSET + MESH_CAP_METRIC_LEN
#define	MESH_CAP_POWERSAVE_OFFSET	MESH_CAP_PEER_CAP_OFFSET + MESH_CAP_PEER_CAP_LEN
#define	MESH_CAP_SYNC_CAP_OFFSET	MESH_CAP_POWERSAVE_OFFSET + MESH_CAP_POWERSAVE_CAP_LEN
#define	MESH_CAP_MDA_CAP_OFFSET		MESH_CAP_SYNC_CAP_OFFSET + MESH_CAP_SYNC_CAP_LEN
#define	MESH_CAP_CH_PRECEDENCE_OFFSET	MESH_CAP_MDA_CAP_OFFSET + MESH_CAP_MDA_CAP_LEN

#define NUM_AVAILABLE_PEER			32		// Hard code  system  peer link Maxium
#define NUM_MESH					1		// How many /dev/meshX devices

#define MESH_PEER_LINK_CAP_CAPACITY_MASK	(BIT(12)|BIT(11)|BIT(10)|BIT(9)|BIT(8)|BIT(7)|BIT(6)|BIT(5)|BIT(4)|BIT(3)|BIT(2)|BIT(1)|BIT(0))	// MASK of Peer capacity(capacity)
#define MESH_PEER_LINK_CAP_FLAGS_MASK		(BIT(7)|BIT(6)|BIT(5))	// MASK of Peer capacity (flags, Because 1byte)


#define MAX_MPP_NUM 				32
#define DATA_SKB_BUFFER_SIZE 		7 		// acutal size is power of 2
#define AODV_RREQ_TABLE_SIZE 		256 	// must equal 2^DATA_SKB_BUFFER_SIZE
#define PROXY_TABLE_SIZE 			7  		// acutal size is power of 2
// #define PATHSEL_TABLE_SIZE			128

#define _MESH_HEADER_TTL_  			20

// the following two parameters must be power of 2
#define SZ_HASH_IDX1 8
#define SZ_HASH_IDX2 128

#ifdef	MESH_ESTABLISH_RSSI_THRESHOLD
#define	DEFAULT_ESTABLISH_RSSI_THRESHOLD	25;	//RSSI range : 0~100 (Decimal)
#endif

#ifdef _MESH_ACL_ENABLE_
#define NUM_MESH_ACL NUM_ACL
#endif


enum dot11_MP_NEIGHBOR_STATE {
	// Here have virtual "0" and MP_LISTEN Set "1" Because keep table integrity, Table using MP_UNUSED , CAUTION!!  :The value don't modify !!
	MP_UNUSED = 0, // a state to indicate whether an entry is invalid or not, CAUTION!!  :The value don't modify( About initial default and table) !!
	// the following state is defiend by the table of "State Values" in page 62, D0.02
	// hybrid page 116 D1.0 state transitions
	MP_LISTEN,
	MP_OPEN_SENT,
	MP_CONFIRM_RCVD,
	MP_CONFIRM_SENT,
	MP_SUBORDINATE_LINK_DOWN_E,
	MP_SUBORDINATE_LINK_UP,
	MP_SUPERORDINATE_LINK_DOWN,
	MP_SUPERORDINATE_LINK_UP,
	MP_HOLDING
};


/*
  *	@brief	 Mesh EVENT
  *	With sme.c  PeerLink_states_table synchronic
 */
enum MESH_PEER_LINK_EVENT {
	CancelPeerLink = 0,
	PassivePeerLinkOpen,
	ActivePeerLinkOpen,
	CloseReceived,
	OpenReceived,
	ConfirmReceived,
	TimeOut
};

enum MESH_PEER_LINK_CLOSE_REASON {
	CANCELLED = 0,
	CLOSE_RECEIVED,
	INVALID_PARAMETERS,
	EXCEED_MAXIMUM_RETRIES,
	TIMEOUT
};


/*
 *	@brief	Mesh Neighbor Table
 *
 */
struct MESH_Neighbor_Entry {
	// when state = MP_UNUSED, the entry is invalid	
	enum dot11_MP_NEIGHBOR_STATE	State; // type of dot11_MP_NEIGHBOR_STATE
		
	// The following entries represents the "MP Meighbor Table Entry" in page 61, D0.02
	// UINT8	NeighborMACADDR[MACADDRLEN]; 	// in fact, this info can be obtained by this.pstate->hwaddr
	// UINT8	PrimaryMACADDR[MACADDRLEN];		// (popen) No need,Because interface have  priv 	
	unsigned long	LocalLinkID;		// peer link local link id (Identify connect by myself)
	unsigned long	PeerLinkID;			// peer link Peer link id (Identify connect by peer MP)  (PS:Some process allow NULL,  Check NULL before, If no, compare  match or not.)
	UINT8			Co;					// operating channel
	UINT32			Pl;					// CPI
	UINT16			r;					// byte rate (PS:Undefine use byte number !!)
	UINT16			ept;				// error rate
	UINT16			Q;					// strength or quality (PS:Undefine use byte number !!)
	
	// expire time counter (upcount, use system jiffies)
	unsigned long		expire;					// Connect successful MP expire timer.
	unsigned long		BSexpire_LLSAperiod;	// 1.boot sequence process Peer link retry, open, cancel timer, 2.period send LLS timer.
	
	// The following entries are internal data structure
	// a counter
	//   Set it to zero, when State changed from MP_UNUSED to Start Peer Link process.
	//   Path selection or data transmission can reuse it for their propose
	//   Peer link open REQ_MAX
	UINT8			retry;			// retry counter

	UINT32			metric;				// recorded metric
#ifdef MESH_USE_METRICOP
	UINT8                   retryMetric;
	UINT8                   isAsym; // if neighbor is non-Realtek device
	atomic_t                isMetricTesting; // if a testing is performing: 1/2: sender, 4: receiver
	UINT32                  timeMetricUpdate; // jiffies when metric should be updated
	struct {
		UINT32  toRx; // jiffies indicating a peer's test is expired
		UINT32  toTx; // jiffies indicating my test should be terminated
		UINT8   rateRx; // the data rate of the testing traffic been testing by peer
		UINT8   rateTx; // data rate used when issuing test traffic
		UINT8   prioRx, prioTx; // priority during test
		UINT16  lenRx0, lenRx1, cntRx0, cntRx1; // Rx0: from action frame; cntRx1: counted by rx, lenRx1: total received len
		UINT16  lenTx0, cntTx0, cntTx1; // Tx0: parameters for issue_xxx, cntTx1: # of pkt already sent
	} spec11kv;
#endif
	
	//Use  Local Link Announcement Packet error rate calculate(Not use temporary)
	// unsigned int		tx_pkts_pre;
	// unsigned int		tx_fail_pre;
} __WLAN_ATTRIB_PACK__;


/**
 *	@brief	Path selection protocol ID
 *
 *	+-----+------------------------------------+	\n
 *	| OUI | Path selection protocol identifier |	\n
 *	+-----+------------------------------------+	\n
 *	(PACK)
 */
__PACK struct	PathSelectProtocolID_t {
	UINT8	OUI[MESH_PS_IDENT_OUI_LEN];  // defined by owen: {0x00FAC}; ???
	UINT8	value;
} __WLAN_ATTRIB_PACK__;


/**
 *	@brief	Path selection protocol ID
 *
 *	+-----+------------------------------------+	\n
 *	| OUI | Path selection protocol identifier |	\n
 *	+-----+------------------------------------+	\n
 *	(PACK)
 */
__PACK struct	PathSelectMetricID_t {
	UINT8	OUI[MESH_PS_IDENT_OUI_LEN]; // defined by owen:  {000FAC};	???
	UINT8	value;
} __WLAN_ATTRIB_PACK__;


/**
 *	@brief	MESH profile 
 */
struct MESH_Profile { // mesh_profile Configure by WEB in the future, Maybe delete, Preservation before delete
	UINT8				used;  								// unused: 0, used: 1
	// struct list_head		mesh_profile_ptr; 						// Not use now, (2006/12/22), Keep for expand in the future
	UINT8				priority;							// Mesh profile priority(maybe set from web interface)
	struct PathSelectProtocolID_t 	PathSelectProtocolID;	// Undefine(OUI)
	struct PathSelectMetricID_t	PathSelectMetricID;			// Undefine(OUI)
};


struct MESH_Share {
	unsigned short	seq;
	UINT8			PUseq;
	unsigned short	RecentSeq[SZ_HASH_IDX1][SZ_HASH_IDX2];

#if (MESH_DBG_LV & MESH_DBG_COMPLEX)
	struct timer_list		mesh_test_sme_timer; // issue one kind of mgt frame /sec
#endif // (MESH_DBG_LV & MESH_DBG_COMPLEX)

#if (MESH_DBG_LV & MESH_DBG_SIMPLE)
	struct timer_list		mesh_test_sme_timer2; // issue 100 probe req / sec
#endif  // (MESH_DBG_LV & MESH_DBG_SIMPLE)
};



/**
 *	@brief	MESH fake MIB
 *
 *  		Under all one maybe place on MIB in the future.
 */
struct MESH_FAKE_MIB_T {
	UINT8			establish_rssi_threshold;				// Mesh check RSSI before establish sequence.
	// the following fields are for 11s-metric project
	//
#ifdef MESH_USE_METRICOP
	// note: The type of active metric is already defined in mesh profile. Here we only define
	//       the mechanism that is used to report metric
	// 0: (maybe) 11v+11k approach; 1:  Link Metric Reporting procedure;
	UINT8                   isPure11s;
	// note: indicate metric mode;
	// 0 : RSSI (original)
	// 1 : RSSI (before: 09/03/11)
	// 2 : 11s default(air time)
	UINT8                   metricID;

	// The Link metric protocol is invoked immediately during boot-sequence completeness
	// until current 'jiffies' exceeds priv->toMeshMetricAuto + this field
	UINT32                  intervalMetricAuto; // (jiffies)

	struct {
		// default values for test packet
		UINT32                  defPktTO; // (units of 100 TU/ms in spec and our implementation respectively)
		UINT16                  defPktLen; // (bytes) Bt = 1024 bytes
		UINT16                  defPktCnt; // (packet count)
		UINT8                   defPktPri; // (priority: 5-AC_VI by default (in tx.c).. ps: no aggregation test now )
	} spec11kv;
#endif
};



#if (MESH_DBG_LV & MESH_DBG_COMPLEX)
extern void	mesh_test_sme_timer(unsigned long task_priv);
extern unsigned char mesh_test_dst_mac[6];
#endif // (MESH_DBG_LV & MESH_DBG_COMPLEX)

#ifdef	_MESH_DEBUG_
extern UINT8 mesh_proc_MAC[MACADDRLEN];
#endif	// _MESH_DEBUG_

#if (MESH_DBG_LV & MESH_DBG_SIMPLE)
extern void	mesh_test_sme_timer2(unsigned long task_priv);
#endif // (MESH_DBG_LV & MESH_DBG_SIMPLE)

#ifdef  _11s_TEST_MODE_
struct tx_insn;
extern int mesh_debug_tx1(struct net_device *dev, DRV_PRIV *priv, struct sk_buff *skb);
extern int mesh_debug_tx2(DRV_PRIV *priv, struct sk_buff *skb);
extern int mesh_debug_tx3(struct net_device *dev, DRV_PRIV *priv, struct sk_buff *skb);
extern int mesh_debug_tx4(DRV_PRIV *priv, struct tx_insn* txcfg);
extern int mesh_debug_tx5(DRV_PRIV *priv, struct tx_insn* txcfg);
extern int mesh_debug_tx6(DRV_PRIV *priv, struct tx_insn* txcfg);
extern int mesh_debug_tx7(DRV_PRIV *priv,  struct tx_desc *pdesc);
extern int mesh_debug_tx8(DRV_PRIV *priv,  struct tx_desc *pdesc);
extern int mesh_debug_tx9(struct tx_insn* txcfg, struct tx_desc_info *pdescinfo);
extern int mesh_debug_tx10(struct tx_insn* txcfg, struct tx_desc_info *pndescinfo);

extern int mesh_debug_rx1(DRV_PRIV *priv, struct sk_buff *pskb);
extern int mesh_debug_rx2(DRV_PRIV *priv, unsigned int cmd);

extern int mesh_debug_sme1(DRV_PRIV *priv);
extern int mesh_debug_sme2(DRV_PRIV *priv, unsigned int *rate);

extern int pid_receiver;
extern void clean_for_join(DRV_PRIV *priv);
#endif // _11s_TEST_MODE_

extern void hex_dump(void *data, int size);


#endif // _8190S_MESH_PUB_HDR_H_
