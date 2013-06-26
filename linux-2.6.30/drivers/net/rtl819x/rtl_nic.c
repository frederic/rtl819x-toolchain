/*
 * Copyright c                Realtek Semiconductor Corporation, 2003
 * All rights reserved.
 *
 * $Header: /home/cvsroot/linux-2.6.19/linux-2.6.x/drivers/net/re865x/rtl_nic.c,v 1.22 2008/04/11 10:49:14 bo_zhao Exp $
 *
 * $Author: bo_zhao $
 *
 * Abstract: Pure L2 NIC driver, without RTL865X's advanced L3/4 features.
 *
 *   re865x_nic.c: NIC driver for the RealTek 865*
 *
 */

#define DRV_RELDATE		"Mar 25, 2004"
#include <linux/config.h>
#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/compiler.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/etherdevice.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/if_vlan.h>
#include <linux/crc32.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/signal.h>
#include <linux/proc_fs.h>
#include <linux/time.h>
#include <linux/rtc.h>
#include <bsp/bspchip.h>
#include <linux/timer.h>
#if defined(CONFIG_RTK_VLAN_SUPPORT)
#include <net/rtl/rtk_vlan.h>
#endif

#if	defined(CONFIG_RTL8196_RTL8366) && defined(CONFIG_RTL_IGMP_SNOOPING)
#undef	CONFIG_RTL_IGMP_SNOOPING
#endif

//if you need to use the fake eth driver, please also disable the "CONFIG_RTL_ETH_PRIV_SK" in kernel
//#define CONFIG_RTK_FAKE_ETH 1

#include "version.h"
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>

#include "AsicDriver/asicRegs.h"
#include "AsicDriver/rtl865x_asicCom.h"
#include "AsicDriver/rtl865x_asicL2.h"
#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER_L3
#include "AsicDriver/rtl865x_asicL3.h"
#endif

#include "common/mbuf.h"
#include <net/rtl/rtl_queue.h>
#include "common/rtl_errno.h"
#include "rtl865xc_swNic.h"

/*common*/
#include "common/rtl865x_vlan.h"
#include <net/rtl/rtl865x_netif.h>
#include "common/rtl865x_netif_local.h"

/*l2*/
#ifdef CONFIG_RTL_LAYERED_DRIVER_L2
#include "l2Driver/rtl865x_fdb.h"
#include <net/rtl/rtl865x_fdb_api.h>
#endif

/*l3*/
#ifdef CONFIG_RTL_LAYERED_DRIVER_L3
#include "l3Driver/rtl865x_ip.h"
#include "l3Driver/rtl865x_nexthop.h"
#include <net/rtl/rtl865x_ppp.h>
#include "l3Driver/rtl865x_ppp_local.h"
#include "l3Driver/rtl865x_route.h"
#include "l3Driver/rtl865x_arp.h"
#include <net/rtl/rtl865x_nat.h>
#endif

/*l4*/
#ifdef	CONFIG_RTL865X_ROMEPERF
#include "romeperf.h"
#endif
#include <net/rtl/rtl_nic.h>
#if defined(CONFIG_RTL_FASTBRIDGE)
#include <net/rtl/features/fast_bridge.h>
#endif
#if defined(CONFIG_RTL_HW_QOS_SUPPORT) && defined(CONFIG_NET_SCHED) && defined(CONFIG_RTL_LAYERED_DRIVER)
#include <net/rtl/rtl865x_outputQueue.h>
#endif

#ifdef CONFIG_RTL_STP
#include <net/rtl/rtk_stp.h>
#endif

#if defined(CONFIG_RTL_HW_STP)
#include <net/rtl/rtk_stp.h>
#endif

#if defined(CONFIG_RTL_8196C_ESD) || defined(CONFIG_RTL_8198_ESD)
#include <linux/reboot.h>
#endif

#if defined (CONFIG_RTL_IGMP_SNOOPING)
#include <net/rtl/rtl865x_igmpsnooping.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#if defined (CONFIG_RTL_MLD_SNOOPING)
#include <linux/ipv6.h>
int mldSnoopEnabled;
#endif
uint32 nicIgmpModuleIndex=0xFFFFFFFF;
extern int  igmpsnoopenabled;
extern uint32 brIgmpModuleIndex;
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
extern struct net_bridge *bridge0;
extern uint32 br0SwFwdPortMask;
#endif
#endif

#if defined(CONFIG_RTK_VLAN_WAN_TAG_SUPPORT)
uint32 nicIgmpModuleIndex_2=0xFFFFFFFF;
extern uint32 brIgmpModuleIndex_2;
#define VLAN_CONFIG_SIZE sizeof(vlanconfig)/sizeof(struct rtl865x_vlanConfig)
#define VLAN_CONFIG_PPPOE_INDEX (VLAN_CONFIG_SIZE-1)
#endif

#if defined (CONFIG_RTL_8198_INBAND_AP) || defined (CONFIG_RTL_8198_NFBI_BOARD)
#define CONFIG_819X_PHY_RW 1
#endif

static unsigned int curLinkPortMask=0;
static unsigned int newLinkPortMask=0;

#ifdef CONFIG_RTL_8197D_DYN_THR
static int _8197d_link_check = 0;
#endif

#define SET_MODULE_OWNER(dev) do { } while (0)

#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
#include <net/rtl/rtl865x_multicast.h>
#endif

#ifdef CONFIG_RTL8196_RTL8366
#include "RTL8366RB_DRIVER/gpio.h"
#include "RTL8366RB_DRIVER/rtl8366rb_apiBasic.h"
#endif

#ifdef CONFIG_RTK_VOIP_PORT_LINK
#include <net/netlink.h>
#include <linux/rtnetlink.h>
static int rtnl_fill_ifinfo_voip(struct sk_buff *skb, struct net_device *dev,
			int type, u32 pid, u32 seq, u32 change, unsigned int flags);
static void rtmsg_ifinfo_voip(int type, struct net_device *dev, unsigned change);
#endif

#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP
void voip_dsp_L2_pkt_rx(unsigned char* eth_pkt);
#endif

#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP
//merged from r8627 may conflict later
void ( *voip_dsp_L2_pkt_rx_trap )(unsigned char* eth_pkt, unsigned long size) = NULL;	// pkshih: eth_pkt content may be modified!!
#endif

#if (defined(CONFIG_RTL_CUSTOM_PASSTHRU) && !defined(CONFIG_RTL8196_RTL8366))
__DRAM_FWD static int oldStatus;
static struct proc_dir_entry *res=NULL;
static char passThru_flag[1];
static int32 __init rtl8651_customPassthru_init(void);
static int32 rtl8651_initStormCtrl(void);
static inline int32 rtl_isPassthruFrame(uint8 *data);
#endif

#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
static struct proc_dir_entry *phyTest_entry=NULL;
#endif

#if defined(CONFIG_RTL_ETH_PRIV_SKB) && (defined(CONFIG_NET_WIRELESS_AGN) || defined(CONFIG_NET_WIRELESS_AG) || defined(CONFIG_WIRELESS))
#include <net/dst.h>
#endif

#if 0
#define DEBUG_ERR printk
#else
#define DEBUG_ERR(format, args...)
#endif

#if defined (CONFIG_RTL_LOCAL_PUBLIC)
#include <net/rtl/rtl865x_localPublic.h>
#endif

static int32 __865X_Config;
#if defined(DYNAMIC_ADJUST_TASKLET) || defined(BR_SHORTCUT)
#if 0
static int eth_flag=12; // 0 dynamic tasklet, 1 - disable tasklet, 2 - always tasklet , bit2 - bridge shortcut enabled
#endif
#endif
#if defined(BR_SHORTCUT)
__DRAM_FWD  unsigned char cached_eth_addr[ETHER_ADDR_LEN];
EXPORT_SYMBOL(cached_eth_addr);
__DRAM_FWD  struct net_device *cached_dev;
EXPORT_SYMBOL(cached_dev);
#if defined(CONFIG_WIRELESS_LAN_MODULE)
struct net_device* (*wirelessnet_hook_shortcut)(unsigned char *da) = NULL;
EXPORT_SYMBOL(wirelessnet_hook_shortcut);
int (*wirelessnet_hook)(void) = NULL;
EXPORT_SYMBOL(wirelessnet_hook);
#endif
#endif

#if defined(BR_SHORTCUT_C2)
__DRAM_FWD  unsigned char cached_eth_addr2[ETHER_ADDR_LEN];
EXPORT_SYMBOL(cached_eth_addr2);
__DRAM_FWD  struct net_device *cached_dev2;
EXPORT_SYMBOL(cached_dev2);
int last_used = 1;
#endif

#if defined(CONFIG_RTL_REINIT_SWITCH_CORE)
#define STATE_NO_ERROR 0
#define STATE_SW_CLK_ENABLE_WAITING 1
#define STATE_TO_REINIT_SWITCH_CORE  2
int rtl865x_duringReInitSwtichCore=0;
int rtl865x_reInitState=STATE_NO_ERROR;
int rtl865x_reInitWaitCnt=0;
#endif
#if defined (CONFIG_RTL_UNKOWN_UNICAST_CONTROL)
static rtlMacRecord	macRecord[RTL_MAC_RECORD_NUM];
static uint32	macRecordIdx;
static uint8	lanIfName[NETIF_NUMBER];
static void	rtl_unkownUnicastUpdate(uint8 *mac);
static void rtl_unkownUnicastTimer(unsigned long data);
#endif

#if defined(DYNAMIC_ADJUST_TASKLET) || defined(CONFIG_RTL8186_TR) || defined(RTL8196C_EEE_MAC) || defined(CONFIG_RTL_8198_ESD)
static void one_sec_timer(unsigned long task_priv);
#endif

#ifdef CONFIG_RTL8196C_GREEN_ETHERNET
static void power_save_timer(unsigned long task_priv);
#endif

//#define CONFIG_RTL_LINKSTATE
#if defined(CONFIG_RTL_LINKSTATE)
static struct timer_list s_timer;
static void linkup_time_handle(unsigned long arg);
static int32 initPortStateCtrl(void);
static void  exitPortStateCtrl(void);
#endif

#ifdef CONFIG_RTL_HW_VLAN_SUPPORT
#define PORT_NUMBER 6
struct hw_vlan_port_setting{
	  int32 vlan_port_enabled;
	  int32 vlan_port_bridge;
	  int32 vlan_port_tag;
	  int32 vlan_port_vid;
};
int rtl_hw_vlan_ignore_tagged_mc = 1;

struct hw_vlan_port_setting hw_vlan_info[PORT_NUMBER];

int32     rtl_hw_vlan_enable = 0;
#endif


#if defined(CONFIG_RTL_ETH_PRIV_SKB)
__MIPS16 __IRAM_FWD static struct sk_buff *dev_alloc_skb_priv_eth(unsigned int size);
static void init_priv_eth_skb_buf(void);
#endif

#if defined(CONFIG_RTK_QOS_FOR_CABLE_MODEM)
static void rtl_initVlanTableForCableMode(void);
#endif

static int32 rtl819x_eee_proc_init(void);

__DRAM_FWD static struct ring_que	rx_skb_queue;
int skb_num=0;

#if defined(CONFIG_RTL_MULTIPLE_WAN)
#define MULTICAST_NETIF_VLAN_ID		678
static char multiCastNetIf[20]={"multiCastNetIf"};
static char multiCastNetIfMac[6]={ 0x00, 0x11, 0x12, 0x13, 0x14, 0x15 };
static struct net_device *rtl_multiWan_net_dev;
static int rtl_regist_multipleWan_dev(void);
static int rtl_config_multipleWan_netif(int32 cmd);
static int rtl_port_used_by_device(uint32 portMask);
static int rtl865x_addMultiCastNetif(void);
#if 0
static int rtl865x_delMultiCastNetif(void);
int rtl865x_setMultiCastSrcMac(unsigned char *srcMac);
#endif
#endif

int32 rtl865x_init(void);
int32 rtl865x_config(struct rtl865x_vlanConfig vlanconfig[]);

/* These identify the driver base version and may not be removed. */
MODULE_DESCRIPTION("RealTek RTL-8650 series 10/100 Ethernet driver");
MODULE_LICENSE("GPL");

#ifdef CONFIG_DEFAULTS_KERNEL_2_6
static char* multicast_filter_limit = "maximum number of filtered multicast addresses";
module_param (multicast_filter_limit,charp, S_IRUGO);
#else
/* Maximum number of multicast addresses to filter (vs. Rx-all-multicast).
   The RTL chips use a 64 element hash table based on the Ethernet CRC.  */
MODULE_PARM (multicast_filter_limit, "i");
MODULE_PARM_DESC (multicast_filter_limit, "maximum number of filtered multicast addresses");
#endif

#define DRV_NAME		"re865x_nic"
#define PFX			DRV_NAME ": "
#define DRV_VERSION		"0.1"
#define TX_TIMEOUT		(10*HZ)
#define BDINFO_ADDR 0xbe3fc000

#define CONFIG_RTL_REPORT_LINK_STATUS

#if defined (CONFIG_RTL_PHY_POWER_CTRL)
static int32 rtl865x_initPhyPowerCtrl(void);
#endif

#if defined(RX_TASKLET)
__DRAM_GEN int rtl_rx_tasklet_running;
#endif
#if defined(TX_TASKLET)
__DRAM_GEN int rtl_tx_tasklet_running;
#endif

__IRAM_GEN static inline void rtl_rx_interrupt_process(unsigned int status, struct dev_priv *cp);
__IRAM_GEN static inline void rtl_tx_interrupt_process(unsigned int status, struct dev_priv *cp);
#if	defined(CONFIG_RTL_IGMP_SNOOPING)||defined(CONFIG_RTL_LINKCHG_PROCESS) || defined (CONFIG_RTL_PHY_PATCH)
__IRAM_GEN static inline void rtl_link_change_interrupt_process(unsigned int status, struct dev_priv *cp);
#endif

static int rtl_rxTxDoneCnt = 0;
static atomic_t rtl_devOpened;

#if defined(CONFIG_RTL_PROC_DEBUG)
extern unsigned int tx_ringFull_cnt;
#endif

#if defined(CONFIG_RTL_819XD)
static int rtl_port0Refined = 0;
#endif

__MIPS16 __IRAM_GEN void rtl_rxSetTxDone(int enable)
{
	if (unlikely(rtl_devOpened.counter==0))
		return;

	if (FALSE==enable)
	{
		rtl_rxTxDoneCnt--;
		if (rtl_rxTxDoneCnt==-1)
			REG32(CPUIIMR) &= ~(TX_ALL_DONE_IE_ALL);
	}
	else
	{
		rtl_rxTxDoneCnt++;
		if (rtl_rxTxDoneCnt==0)
			REG32(CPUIIMR) |= (TX_ALL_DONE_IE_ALL);
	}
}

#define NEXT_DEV(cp)			(cp->dev_next ? cp->dev_next : cp->dev_prev)
#define NEXT_CP(cp)			((struct dev_priv *)((NEXT_DEV(cp))->priv))
#define IS_FIRST_DEV(cp)	(NEXT_CP(cp)->opened ? 0 : 1)
#define GET_IRQ_OWNER(cp) (cp->irq_owner ? cp->dev : NEXT_DEV(cp))

#define MAX_PORT_NUM 9

static unsigned int rxRingSize[RTL865X_SWNIC_RXRING_HW_PKTDESC] =
	{NUM_RX_PKTHDR_DESC,
	NUM_RX_PKTHDR_DESC1,
	NUM_RX_PKTHDR_DESC2,
	NUM_RX_PKTHDR_DESC3,
	NUM_RX_PKTHDR_DESC4,
	NUM_RX_PKTHDR_DESC5};

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) 
static unsigned int txRingSize[RTL865X_SWNIC_TXRING_HW_PKTDESC] =
	{NUM_TX_PKTHDR_DESC,
	NUM_TX_PKTHDR_DESC1,
	NUM_TX_PKTHDR_DESC2,
	NUM_TX_PKTHDR_DESC3
	};
#else
static unsigned int txRingSize[RTL865X_SWNIC_TXRING_HW_PKTDESC] =
	{NUM_TX_PKTHDR_DESC,
	NUM_TX_PKTHDR_DESC1};
#endif

#if defined (CONFIG_RTL_MULTI_LAN_DEV)||defined(CONFIG_RTK_VLAN_SUPPORT)
static  struct rtl865x_vlanConfig packedVlanConfig[NETIF_NUMBER];
#endif

#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	int  vlan_tag;
	int  vlan_host_pri;
	int  vlan_bridge_tag;
	int  vlan_bridge_port;
    int  vlan_bridge_multicast_tag;
#endif

/*
linux protocol stack netif VS rtl819x driver network interface
the name of ps netif maybe different with driver.
*/
static ps_drv_netif_mapping_t ps_drv_netif_mapping[NETIF_NUMBER];

static struct rtl865x_vlanConfig vlanconfig[] = {
/*      	ifName  W/L      If type		VID	 FID	   Member Port	UntagSet		mtu		MAC Addr	is_slave								*/
/*		=====  ===   =======	===	 ===   =========   =======	====	====================================	*/

#ifdef CONFIG_BRIDGE
#if defined (CONFIG_RTL_MULTI_LAN_DEV)
	{ 	RTL_DRV_LAN_P0_NETIF_NAME,	0,	IF_ETHER,	RTL_LANVLANID,		RTL_LAN_FID,		RTL_LANPORT_MASK_4,		RTL_LANPORT_MASK_4,		1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x90 } }, 0	},
	{	RTL_DRV_LAN_P4_NETIF_NAME,	1,	IF_ETHER,	RTL_WANVLANID,		RTL_WAN_FID,	RTL_WANPORT_MASK,		RTL_WANPORT_MASK,		1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x91 } }, 0	},
	{	RTL_DRV_LAN_P1_NETIF_NAME,	0,	IF_ETHER,	RTL_LANVLANID,		RTL_LAN_FID,		RTL_LANPORT_MASK_3,		RTL_LANPORT_MASK_3,		1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x90 } }, 0	},
	{	RTL_DRV_LAN_P2_NETIF_NAME,	0,	IF_ETHER,	RTL_LANVLANID,		RTL_LAN_FID,		RTL_LANPORT_MASK_2,		RTL_LANPORT_MASK_2,		1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x90 } }, 0	},
	{	RTL_DRV_LAN_P3_NETIF_NAME,	0,	IF_ETHER,	RTL_LANVLANID,		RTL_LAN_FID, 	RTL_LANPORT_MASK_1,		RTL_LANPORT_MASK_1,		1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x90 } }, 0	},
#else	/*CONFIG_RTL_MULTI_LAN_DEV*/
#if defined(CONFIG_RTK_VLAN_SUPPORT)
	{ 	RTL_DRV_LAN_NETIF_NAME,	 0,   IF_ETHER, 	RTL_LANVLANID, 	   	RTL_LAN_FID, 	RTL_LANPORT_MASK_4, 	RTL_LANPORT_MASK_4,		1500, 	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x90 } }, 0	},
#else
	{ 	RTL_DRV_LAN_NETIF_NAME,	 0,   IF_ETHER, 	RTL_LANVLANID, 	   	RTL_LAN_FID, 	RTL_LANPORT_MASK, 		RTL_LANPORT_MASK,		1500, 	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x90 } }, 0	},
#endif
#if defined(CONFIG_RTL_PUBLIC_SSID)
	{	RTL_GW_WAN_DEVICE_NAME,	 1,   IF_ETHER,	RTL_WANVLANID,	   RTL_WAN_FID,		RTL_WANPORT_MASK,		RTL_WANPORT_MASK,		1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x91 } }, 0	},
#else
	{	RTL_DRV_WAN0_NETIF_NAME,	 1,   IF_ETHER,	RTL_WANVLANID,		RTL_WAN_FID,	RTL_WANPORT_MASK,		RTL_WANPORT_MASK,		1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x91 } }, 0	},
#endif
#if defined(CONFIG_RTK_VLAN_SUPPORT)
	{	RTL_DRV_LAN_P1_NETIF_NAME,	0,   IF_ETHER, 	RTL_LANVLANID,	RTL_LAN_FID, 	RTL_LANPORT_MASK_3, 	RTL_LANPORT_MASK_3,		1500, 	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x92 } }, 0	},
	{	RTL_DRV_LAN_P2_NETIF_NAME, 	0,   IF_ETHER, 	RTL_LANVLANID,	RTL_LAN_FID, 	RTL_LANPORT_MASK_2, 	RTL_LANPORT_MASK_2,		1500, 	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x93 } }, 0	},
	{	RTL_DRV_LAN_P3_NETIF_NAME, 	0,   IF_ETHER, 	RTL_LANVLANID,	RTL_LAN_FID, 	RTL_LANPORT_MASK_1, 	RTL_LANPORT_MASK_1,		1500, 	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x94 } }, 0	},
#ifdef CONFIG_8198_PORT5_GMII
	{	RTL_DRV_LAN_P5_NETIF_NAME, 	0,	 IF_ETHER,	RTL_LANVLANID,	RTL_LAN_FID,		RTL_LANPORT_MASK_5, 	RTL_LANPORT_MASK_5, 	1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x95 } }, 0	},
#endif //CONFIG_8198_PORT5_GMII
#ifdef CONFIG_RTK_VLAN_NEW_FEATURE
	{	RTL_DRV_LAN_P7_NETIF_NAME,	0,	 IF_ETHER,	RTL_LANVLANID,	RTL_LAN_FID,	0, 	0, 	1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x97 } }, 0	},
#endif
#endif
#endif
#else	/*CONFIG_BRIDGE*/
#if defined (CONFIG_RTL_MULTI_LAN_DEV)
	{ 	RTL_DRV_LAN_P0_NETIF_NAME,	0,	IF_ETHER,	RTL_LANVLANID,		RTL_LAN_FID,		RTL_LANPORT_MASK_4,		RTL_LANPORT_MASK_4,		1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x90 } }, 0	},
	{	RTL_DRV_LAN_P4_NETIF_NAME,	1,	IF_ETHER,	RTL_WANVLANID,		RTL_WAN_FID,	RTL_WANPORT_MASK,		RTL_WANPORT_MASK,		1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x91 } }, 0	},
	{	RTL_DRV_LAN_P1_NETIF_NAME,	0,	IF_ETHER,	RTL_LANVLANID,		RTL_LAN_FID,		RTL_LANPORT_MASK_3,		RTL_LANPORT_MASK_3,		1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x90 } }, 0	},
	{	RTL_DRV_LAN_P2_NETIF_NAME,	0,	IF_ETHER,	RTL_LANVLANID,		RTL_LAN_FID,		RTL_LANPORT_MASK_2,		RTL_LANPORT_MASK_2,		1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x90 } }, 0	},
	{	RTL_DRV_LAN_P3_NETIF_NAME,	0,	IF_ETHER,	RTL_LANVLANID,		RTL_LAN_FID, 	RTL_LANPORT_MASK_1,		RTL_LANPORT_MASK_1,		1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x90 } }, 0	},
#else /*CONFIG_RTL_MULTI_LAN_DEV*/
	{ 	RTL_DRV_LAN_NETIF_NAME,	 0,   IF_ETHER, 	RTL_LANVLANID, 	   	RTL_LAN_FID, 	RTL_LANPORT_MASK, 		RTL_LANPORT_MASK,		1500, 	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x90 } }, 0	},
	{	RTL_DRV_WAN0_NETIF_NAME,	 1,   IF_ETHER,	RTL_WANVLANID,	   	RTL_WAN_FID,	RTL_WANPORT_MASK,		RTL_WANPORT_MASK,		1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x91 } }, 0	},
#if defined(CONFIG_RTK_VLAN_SUPPORT)
	{	RTL_DRV_LAN_P1_NETIF_NAME,	0,   IF_ETHER, 	RTL_LANVLANID,	RTL_LAN_FID,		RTL_LANPORT_MASK_3, 	RTL_LANPORT_MASK_3,		1500, 	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x92 } }, 0	},
	{	RTL_DRV_LAN_P2_NETIF_NAME, 	0,   IF_ETHER, 	RTL_LANVLANID,	RTL_LAN_FID,		RTL_LANPORT_MASK_2, 	RTL_LANPORT_MASK_2,		1500, 	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x93 } }, 0	},
	{	RTL_DRV_LAN_P3_NETIF_NAME, 	0,   IF_ETHER, 	RTL_LANVLANID, 	RTL_LAN_FID,		RTL_LANPORT_MASK_1, 	RTL_LANPORT_MASK_1,		1500, 	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x94 } }, 0	},
#ifdef CONFIG_RTK_VLAN_NEW_FEATURE
	{	RTL_DRV_LAN_P7_NETIF_NAME,	0,	 IF_ETHER,	RTL_LANVLANID,	RTL_LAN_FID,	0,	0,	1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x97 } }, 0	},
#endif
#endif
#endif
#endif
	{	RTL_DRV_PPP_NETIF_NAME, 1,   IF_PPPOE,    	RTL_WANVLANID,		RTL_WAN_FID,    	RTL_WANPORT_MASK,         	RTL_WANPORT_MASK,     	1500, { { 0x00, 0x12, 0x34, 0x56, 0x78, 0x91 } }, 1 },
	RTL865X_CONFIG_END,
};

#if defined(CONFIG_RTL_MULTIPLE_WAN)
static struct rtl865x_vlanConfig rtl_multiWan_config = { RTL_DRV_WAN1_NETIF_NAME,	 1,   IF_ETHER, RTL_WAN_1_VLANID,		RTL_WAN_FID,	RTL_WANPORT_MASK,		RTL_WANPORT_MASK,		1500,	{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x99 } }, 0	};
#endif

#if (defined(CONFIG_PROC_FS) && defined(CONFIG_NET_SCHED) && defined(CONFIG_RTL_LAYERED_DRIVER))||(defined (CONFIG_RTL_HW_QOS_SUPPORT))
static uint8	netIfName[NETIF_NUMBER][IFNAMSIZ] = {{0}};
#endif

#if defined(CONFIG_RTL_ETH_PRIV_SKB)
/*	The following structure's field orders was arranged for special purpose,
	it should NOT be modify	*/
struct priv_skb_buf2 {
	unsigned char magic[ETH_MAGIC_LEN];
	void			*buf_pointer;
	/* the below 2 filed MUST together */
	struct list_head	list;
	unsigned char buf[ETH_SKB_BUF_SIZE];
};

static struct priv_skb_buf2 eth_skb_buf[MAX_ETH_SKB_NUM+1];
__DRAM_FWD static struct list_head eth_skbbuf_list;
__DRAM_FWD int eth_skb_free_num;
EXPORT_SYMBOL(eth_skb_free_num);
extern struct sk_buff *dev_alloc_8190_skb(unsigned char *data, int size);
struct sk_buff *priv_skb_copy(struct sk_buff *skb);
#endif

#if defined(CONFIG_RTK_VLAN_NEW_FEATURE)
	struct vlan_info management_vlan;
#endif

#ifdef CONFIG_POCKET_AP_SUPPORT
int rtl865x_curOpMode=BRIDGE_MODE;
#else
int rtl865x_curOpMode=GATEWAY_MODE;
#endif

__DRAM_FWD static  struct re865x_priv _rtl86xx_dev;

#ifdef CONFIG_RTL8196C_GREEN_ETHERNET
 static   struct timer_list expire_timer2;
#endif

#ifdef CONFIG_RTL_STP
static unsigned char STPmac[] = { 1, 0x80, 0xc2, 0,0,0};
#ifdef CONFIG_RTK_MESH
int8 STP_PortDev_Mapping[MAX_RE865X_STP_PORT] ={NO_MAPPING, NO_MAPPING, NO_MAPPING, NO_MAPPING, NO_MAPPING, WLAN_PSEUDO_IF_INDEX, WLAN_MESH_PSEUDO_IF_INDEX};
#else
int8 STP_PortDev_Mapping[MAX_RE865X_STP_PORT] ={NO_MAPPING, NO_MAPPING, NO_MAPPING, NO_MAPPING, NO_MAPPING, WLAN_PSEUDO_IF_INDEX};
#endif
static int re865x_stp_get_pseudodevno(uint32 port_num);
static int getVidByPort(uint32 port_num);
#endif

static int re865x_ioctl (struct net_device *dev, struct ifreq *rq, int cmd);
static int32 reinit_vlan_configure(struct rtl865x_vlanConfig new_vlanconfig[]);
#if defined(CONFIG_RTL_REPORT_LINK_STATUS)
/*FIX ME if mutiple-wan*/
static unsigned char wan_linkStatus[2];
static int32 rtk_link_status_read( char *page, char **start, off_t off, int count, int *eof, void *data );
static int32 rtk_link_status_write( struct file *filp, const char *buff,unsigned long len, void *data );
#endif
//static void rtl_print_vlanconfig(struct rtl865x_vlanConfig new_vlanconfig[]);
#if defined(CONFIG_RTK_VLAN_SUPPORT)
static int read_proc_vlan(char *page, char **start, off_t off,int count, int *eof, void *data);
static int write_proc_vlan(struct file *file, const char *buffer,unsigned long count, void *data);
static int32 rtk_vlan_support_read( char *page, char **start, off_t off, int count, int *eof, void *data );
static int32 rtk_vlan_support_write( struct file *filp, const char *buff,unsigned long len, void *data );
#if defined(CONFIG_RTK_VLAN_NEW_FEATURE)
static int rtk_vlan_management_read(char *page, char **start, off_t off, int count, int *eof, void *data);
static int rtk_vlan_management_write(struct file *file, const char *buffer, unsigned long len, void *data);
#endif

//__DRAM_FWD int rtk_vlan_support_enable;
int rtk_vlan_support_enable;


#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
static int32 rtk_vlan_wan_tag_getportmask(int bridge_port);
static int32 rtk_vlan_wan_tag_support_read( char *page, char **start, off_t off, int count, int *eof, void *data );
static int32 rtk_vlan_wan_tag_support_write( struct file *filp, const char *buff,unsigned long len, void *data );
#endif


#if defined(CONFIG_819X_PHY_RW) //#if defined(CONFIG_RTK_VLAN_FOR_CABLE_MODEM)
static int32 rtl_phy_status_read( char *page, char **start, off_t off, int count, int *eof, void *data );
static int32 rtl_phy_status_write( struct file *filp, const char *buff,unsigned long len, void *data );
static int32 port_mibStats_read_proc( char *page, char **start, off_t off, int count, int *eof, void *data );
static int32 port_mibStats_write_proc( struct file *filp, const char *buff,unsigned long len, void *data );

struct port_mibStatistics  {

	/*here is in counters  definition*/
	uint32 ifInOctets;
	uint64 ifHCInOctets;
	uint32 ifInUcastPkts;
	uint64 ifHCInUcastPkts;
	uint32 ifInMulticastPkts;
	uint64 ifHCInMulticastPkts;
	uint64 ifHCInBroadcastPkts;
	uint32 ifInBroadcastPkts;
	uint32 ifInDiscards;
	uint32 ifInErrors;
	uint64 etherStatsOctets;
	uint32 etherStatsUndersizePkts;
	uint32 etherStatsFraments;
	uint32 etherStatsPkts64Octets;
	uint32 etherStatsPkts65to127Octets;
	uint32 etherStatsPkts128to255Octets;
	uint32 etherStatsPkts256to511Octets;
	uint32 etherStatsPkts512to1023Octets;
	uint32 etherStatsPkts1024to1518Octets;
	uint32 etherStatsOversizePkts;
	uint32 etherStatsJabbers;
	uint32 dot1dTpPortInDiscards;
	uint32 etherStatusDropEvents;
	uint32 dot3FCSErrors;
	uint32 dot3StatsSymbolErrors;
	uint32 dot3ControlInUnknownOpcodes;
	uint32 dot3InPauseFrames;

	/*here is out counters  definition*/
	uint32 ifOutOctets;
	uint64 ifHCOutOctets;
	uint32 ifOutUcastPkts;
	uint64 ifHCOutUcastPkts;
	uint64 ifHCOutMulticastPkts;
	uint64 ifHCOutBroadcastPkts;
	uint32 ifOutMulticastPkts;
	uint32 ifOutBroadcastPkts;
	uint32 ifOutDiscards;
	uint32 ifOutErrors;
	uint32 dot3StatsSingleCollisionFrames;
	uint32 dot3StatsMultipleCollisionFrames;
	uint32 dot3StatsDefferedTransmissions;
	uint32 dot3StatsLateCollisions;
	uint32 dot3StatsExcessiveCollisions;
	uint32 dot3OutPauseFrames;
	uint32 dot1dBasePortDelayExceededDiscards;
	uint32 etherStatsCollisions;

	/*here is whole system couters definition*/
	uint32 dot1dTpLearnedEntryDiscards;
	uint32 etherStatsCpuEventPkts;
	uint32 ifInUnknownProtos;
	uint32 ifSpeed;
	uint32 ifHighSpeed;
	uint32 ifConnectorPresent;
	uint32 ifCounterDiscontinuityTime;
};
#endif	//#if defined(CONFIG_819X_PHY_RW)
#endif

#if defined(CONFIG_RTL_HW_VLAN_SUPPORT)
static int32 rtl_hw_vlan_support_read( char *page, char **start, off_t off, int count, int *eof, void *data );
static int32 rtl_hw_vlan_support_write( struct file *filp, const char *buff,unsigned long len, void *data );
static int32 rtl_hw_vlan_tagged_bridge_multicast_read( char *page, char **start, off_t off, int count, int *eof, void *data );
static int32 rtl_hw_vlan_tagged_bridge_multicast_write( struct file *filp, const char *buff,unsigned long len, void *data );
int rtl_process_hw_vlan_tx(rtl_nicTx_info *txInfo);
#endif


#if defined(RTL8196C_EEE_MAC)
extern int eee_enabled;
extern void eee_phy_enable(void);
extern void eee_phy_disable(void);
#ifdef CONFIG_RTL8196C_REVISION_B
static unsigned char prev_port_sts[MAX_PORT_NUMBER] = { 0, 0, 0, 0, 0 };
#endif
#endif
#if defined(CONFIG_RTL_LOCAL_PUBLIC)
static int32 rtl865x_getPortlistByMac(const unsigned char *mac,uint32 *portlist);
#endif
static inline int rtl_isWanDev(struct dev_priv *cp);

#ifdef CONFIG_RTL8196C_ETH_IOT
uint32 port_link_sts = 0;	// the port which already linked up does not need to check ...
uint32 port_linkpartner_eee = 0;
#endif

#ifdef CONFIG_RTL_8196C_ESD
int _96c_esd_counter = 0;
int _96c_esd_reboot_counter = 0;
#endif

#if defined(PATCH_GPIO_FOR_LED)
#define MIB_RX_PKT_CNT	0
#define MIB_TX_PKT_CNT	1

#define PORT_PABCD_BASE	10	// Base of P0~P1 at PABCD
#define P0_PABCD_BIT		10
#define P1_PABCD_BIT		11
#define P2_PABCD_BIT		12
#define P3_PABCD_BIT		13
#define P4_PABCD_BIT		14

#define SUCCESS 0
#define FAILED -1

#define GPIO_LED_NOBLINK_TIME		(12*HZ/10)	// time more than 1-sec timer interval
#define GPIO_LED_ON_TIME			(4*HZ/100)	// 40ms
#define GPIO_LED_ON					0
#define GPIO_LED_OFF					1
#define GPIO_LINK_STATUS			1
#define GPIO_LINK_STATE_CHANGE 0x80000000

#define GPIO_UINT32_DIFF(a, b)		((a >= b)? (a - b):(0xffffffff - b + a + 1))

struct ctrl_led {
	struct timer_list	LED_Timer;
	unsigned int			LED_Interval;
	unsigned char		LED_Toggle;
	unsigned char		LED_ToggleStart;
	unsigned int			LED_tx_cnt_log;
	unsigned int			LED_rx_cnt_log;
	unsigned int			LED_tx_cnt;
	unsigned int			LED_rx_cnt;
	unsigned int			link_status;
	unsigned char			blink_once_done;		// 1: blink once done
} led_cb[5];

static int32 rtl819x_getAsicMibCounter(int port, uint32 counter, uint32 *value)
{
	rtl865x_tblAsicDrv_simpleCounterParam_t simpleCounter;
	rtl8651_getSimpleAsicMIBCounter(port, &simpleCounter);

	switch(counter){
		case MIB_RX_PKT_CNT:
			*value=simpleCounter.rxPkts;
			break;
		case MIB_TX_PKT_CNT:
			*value=simpleCounter.txPkts;
			break;
		default:
			return FAILED;
	}
	return SUCCESS;
}
static void gpio_set_led(int port, int flag){
	if (flag == GPIO_LED_OFF){
/*		RTL_W32(PABCD_CNR, RTL_R32(PABCD_CNR) & (~((0x1<<port)<<PORT_PABCD_BASE)));	//set GPIO pin
*		RTL_W32(PABCD_DIR, RTL_R32(PABCD_DIR) | ((0x1<<port)<<PORT_PABCD_BASE));//output pin
*/
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) | ((0x1<<port)<<PORT_PABCD_BASE)));//set 1
	}
	else{
/*		RTL_W32(PABCD_CNR, RTL_R32(PABCD_CNR) & (~((0x1<<port)<<PORT_PABCD_BASE)));	//set GPIO pin
*		RTL_W32(PABCD_DIR, RTL_R32(PABCD_DIR) | ((0x1<<port)<<PORT_PABCD_BASE));//output pin
*/
		RTL_W32(PABCD_DAT, (RTL_R32(PABCD_DAT) & (~((0x1<<port)<<PORT_PABCD_BASE))));//set 0
	}
}

static void gpio_led_Interval_timeout(unsigned long port)
{
	struct ctrl_led *cb	= &led_cb[port];
	unsigned long flags;

	local_irq_save(flags);

	if (cb->link_status & GPIO_LINK_STATE_CHANGE) {
		cb->link_status &= ~GPIO_LINK_STATE_CHANGE;
		if (cb->link_status & GPIO_LINK_STATUS)
			cb->LED_ToggleStart = GPIO_LED_ON;
		else
			cb->LED_ToggleStart = GPIO_LED_OFF;
		cb->LED_Toggle = cb->LED_ToggleStart;
		gpio_set_led(port, cb->LED_Toggle);
	}
	else {
		if (cb->link_status & GPIO_LINK_STATUS)
			gpio_set_led(port, cb->LED_Toggle);
	}

	if (cb->link_status & GPIO_LINK_STATUS)	 {
		if (cb->LED_Toggle == cb->LED_ToggleStart){
			mod_timer(&cb->LED_Timer, jiffies + cb->LED_Interval);
			cb->blink_once_done=1;
		}
		else{
			mod_timer(&cb->LED_Timer, jiffies + GPIO_LED_ON_TIME);
			cb->blink_once_done=0;
		}

		cb->LED_Toggle = (cb->LED_Toggle + 1) & 0x1;	//cb->LED_Toggle = (cb->LED_Toggle + 1) % 2;
	}
	local_irq_restore(flags);
}

void calculate_led_interval(int port)
{
	struct ctrl_led *cb	= &led_cb[port];

	unsigned int delta = 0;

	/* calculate counter delta	*/
	delta += GPIO_UINT32_DIFF(cb->LED_tx_cnt, cb->LED_tx_cnt_log);
	delta += GPIO_UINT32_DIFF(cb->LED_rx_cnt, cb->LED_rx_cnt_log);
	cb->LED_tx_cnt_log = cb->LED_tx_cnt;
	cb->LED_rx_cnt_log = cb->LED_rx_cnt;

	/* update interval according to delta	*/
	if (delta == 0) {
		if (cb->LED_Interval == GPIO_LED_NOBLINK_TIME)
			mod_timer(&(cb->LED_Timer), jiffies + cb->LED_Interval);
		else{
			cb->LED_Interval = GPIO_LED_NOBLINK_TIME;
			if(cb->blink_once_done==1){
				mod_timer(&(cb->LED_Timer), jiffies + cb->LED_Interval);
				cb->blink_once_done=0;
			}
		}
	}
	else
	{
		if(delta>25){		//That is: 200/delta-GPIO_LED_ON_TIME < GPIO_LED_ON_TIME
			cb->LED_Interval = GPIO_LED_ON_TIME;
		}
		else{
			/*	if delta is odd, should be +1 into even.		*/
			/*	just make led blink more stable and smooth.	*/
			if((delta & 0x1) == 1)
				delta++;

			cb->LED_Interval=200/delta-GPIO_LED_ON_TIME;		/* rx 1pkt + tx 1pkt => blink one time!	*/

/*			if (cb->LED_Interval < GPIO_LED_ON_TIME)
*				cb->LED_Interval = GPIO_LED_ON_TIME;
*/
		}
	}
}

void update_mib_counter(int port)
{
	uint32 regVal;
	struct ctrl_led *cb	= &led_cb[port];

	regVal=READ_MEM32(PSRP0+(port<<2));
	if((regVal&PortStatusLinkUp)!=0){
		//link up
		if (!(cb->link_status & GPIO_LINK_STATUS)) {
			cb->link_status = GPIO_LINK_STATE_CHANGE | 1;
		}
		rtl819x_getAsicMibCounter(port, MIB_TX_PKT_CNT, (uint32 *)&cb->LED_tx_cnt);
		rtl819x_getAsicMibCounter(port, MIB_RX_PKT_CNT, (uint32 *)&cb->LED_rx_cnt);
	}
	else{
		//link down
		if (cb->link_status & GPIO_LINK_STATUS) {
			cb->link_status = GPIO_LINK_STATE_CHANGE;
		}
	}
}

void init_led_ctrl(int port)
{
	struct ctrl_led *cb	= &led_cb[port];

	RTL_W32(PABCD_CNR, RTL_R32(PABCD_CNR) & (~((0x1<<port)<<PORT_PABCD_BASE)));	//set GPIO pin
	RTL_W32(PABCD_DIR, RTL_R32(PABCD_DIR) | ((0x1<<port)<<PORT_PABCD_BASE));//output pin

	memset(cb, '\0', sizeof(struct ctrl_led));

	update_mib_counter(port);
	calculate_led_interval(port);
	cb->link_status |= GPIO_LINK_STATE_CHANGE;

	init_timer(&cb->LED_Timer);
	cb->LED_Timer.expires = jiffies + cb->LED_Interval;
	cb->LED_Timer.data = (unsigned long)port;
	cb->LED_Timer.function = gpio_led_Interval_timeout;
	mod_timer(&cb->LED_Timer, jiffies + cb->LED_Interval);

	gpio_led_Interval_timeout(port);
}

void disable_led_ctrl(int port)
{
	struct ctrl_led *cb	= &led_cb[port];
	gpio_set_led(port, GPIO_LED_OFF);

	if (timer_pending(&cb->LED_Timer))
		del_timer_sync(&cb->LED_Timer);
}
#endif // PATCH_GPIO_FOR_LED

#if defined(CONFIG_RTL_819XD)&&defined(CONFIG_RTL_8211DS_SUPPORT)&&defined(CONFIG_RTL_8197D)
int lanPortMask = 0x10f;
int wanPortMask = 0x10;
#if defined(CONFIG_RTK_VLAN_SUPPORT) || defined (CONFIG_RTL_MULTI_LAN_DEV)
int  lanPortMask1 = 0x8;
int  lanPortMask2 = 0x4;
int  lanPortMask3 = 0x2;
int  lanPortMask4 = 0x1;
#endif

#undef	RTL_WANPORT_MASK
#undef	RTL_LANPORT_MASK
#if defined(CONFIG_RTK_VLAN_SUPPORT) || defined (CONFIG_RTL_MULTI_LAN_DEV)
#undef 	RTL_LANPORT_MASK_1
#undef	RTL_LANPORT_MASK_2
#undef 	RTL_LANPORT_MASK_3
#undef 	RTL_LANPORT_MASK_4
#endif

#define	RTL_WANPORT_MASK		wanPortMask
#define	RTL_LANPORT_MASK		lanPortMask
#if defined(CONFIG_RTK_VLAN_SUPPORT) || defined (CONFIG_RTL_MULTI_LAN_DEV)
#define 	RTL_LANPORT_MASK_1	lanPortMask1
#define	RTL_LANPORT_MASK_2	lanPortMask2
#define 	RTL_LANPORT_MASK_3	lanPortMask3
#define 	RTL_LANPORT_MASK_4	lanPortMask4
#endif

void rtl_setPppMask(void)
{
	int i;
	int totalVlans;
	totalVlans=((sizeof(vlanconfig))/(sizeof(struct rtl865x_vlanConfig)))-1;
	for(i=0;i<totalVlans;i++)
	{
		if(vlanconfig[i].if_type==IF_PPPOE){
			vlanconfig[i].memPort = 0x1;
			vlanconfig[i].untagSet = 0x1;
		}
	}
}

void rtl_resetRegisterNotFound8211ds(void)
{
	REG32(0xbb804104) =0x00FF2039;
	REG32(0xbb80414c) =0;
	REG32(0xbb804100) =0;
}

void rtl_setPortMask(uint32 reg_data)
{
	if((reg_data != 0)&&(reg_data != 0xFFFF))
	{
		/*8211ds is found*/
		lanPortMask   = 0x11e;
		wanPortMask = 0x1;
		#if defined(CONFIG_RTK_VLAN_SUPPORT) || defined (CONFIG_RTL_MULTI_LAN_DEV)
		lanPortMask1 = 0x10;
		lanPortMask2 = 0x8;
		lanPortMask3 = 0x4;
		lanPortMask4 = 0x2;
		#endif
		rtl_setPppMask();

		// Flow control DSC tolerance: change to 32 pages to fix "port 0 (8211D) has Rx CRC" issue.
		REG32(MACCR) = (REG32(MACCR) & ~CF_FCDSC_MASK) | (0x20 << CF_FCDSC_OFFSET);
	}
	else
	{
		/*8211ds is not found*/
		lanPortMask   = 0x10f;
		wanPortMask = 0x10;
		#if defined(CONFIG_RTK_VLAN_SUPPORT) || defined (CONFIG_RTL_MULTI_LAN_DEV)
		lanPortMask1 = 0x8;
		lanPortMask2 = 0x4;
		lanPortMask3 = 0x2;
		lanPortMask4 = 0x1;
		#endif
		rtl_resetRegisterNotFound8211ds();
	}
}
#endif


/*
device mapping mainten
*/
struct rtl865x_vlanConfig * rtl_get_vlanconfig_by_netif_name(const char *name)
{
	int i;
	for(i= 0; vlanconfig[i].vid != 0;i++)
	{
		if(memcmp(vlanconfig[i].ifname,name,strlen(name)) == 0)
			return &vlanconfig[i];
	}

	#if defined(CONFIG_RTL_MULTIPLE_WAN)
	//check multiple wan device
	if(memcmp(rtl_multiWan_config.ifname,name,strlen(name)) == 0)
		return &rtl_multiWan_config;
	#endif

	return NULL;
}

#if 1
void rtl_ps_drv_netif_mapping_show(void)
{
	int i;
	DEBUG_ERR("linux netif name VS driver netif name mapping:\n");
	for(i = 0; i < NETIF_NUMBER;i++)
	{
		DEBUG_ERR("valid(%d),linux netif name(%s) <---->drv netif name(%s)\n",ps_drv_netif_mapping[i].valid,
			ps_drv_netif_mapping[i].ps_netif?ps_drv_netif_mapping[i].ps_netif->name:NULL,ps_drv_netif_mapping[i].drvName);
	}
}
#endif

static int rtl_ps_drv_netif_mapping_init(void)
{
	memset(ps_drv_netif_mapping,0,NETIF_NUMBER * sizeof(ps_drv_netif_mapping_t));
	return SUCCESS;
}

int rtl_get_ps_drv_netif_mapping_by_psdev_name(const char *psname, char *netifName)
{
	int i;
	if(netifName == NULL || strlen(psname) >= MAX_IFNAMESIZE )
		return FAILED;

	for(i = 0; i < NETIF_NUMBER;i++)
	{
		if(ps_drv_netif_mapping[i].valid == 1 && memcmp(ps_drv_netif_mapping[i].ps_netif->name,psname,strlen(psname)) == 0)
		{
			memcpy(netifName,ps_drv_netif_mapping[i].drvName,MAX_IFNAMESIZE);
			return SUCCESS;
		}
	}

	//back compatible,user use br0 to get lan netif
	if(memcmp(psname,RTL_PS_BR0_DEV_NAME,strlen(RTL_PS_BR0_DEV_NAME)) == 0)
	{
		for(i = 0; i < NETIF_NUMBER;i++)
		{
			if(ps_drv_netif_mapping[i].valid == 1 &&
				memcmp(ps_drv_netif_mapping[i].drvName,RTL_DRV_LAN_NETIF_NAME,strlen(RTL_DRV_LAN_NETIF_NAME)) == 0)
				{
					memcpy(netifName,ps_drv_netif_mapping[i].drvName,MAX_IFNAMESIZE);
					return SUCCESS;
				}
		}
	}

	return FAILED;
}

ps_drv_netif_mapping_t* rtl_get_ps_drv_netif_mapping_by_psdev(struct net_device *dev)
{
	int i;
	for(i = 0; i < NETIF_NUMBER;i++)
	{
		if(ps_drv_netif_mapping[i].valid == 1 && ps_drv_netif_mapping[i].ps_netif == dev)
			return &ps_drv_netif_mapping[i];
	}

	//back compatible,user use br0 to get lan netif
	if(memcmp(dev->name,RTL_PS_BR0_DEV_NAME,strlen(RTL_PS_BR0_DEV_NAME)) == 0)
	{
		for(i = 0; i < NETIF_NUMBER;i++)
		{
			if(ps_drv_netif_mapping[i].valid == 1 &&
				memcmp(ps_drv_netif_mapping[i].drvName,RTL_DRV_LAN_NETIF_NAME,strlen(RTL_DRV_LAN_NETIF_NAME)) == 0)
				return &ps_drv_netif_mapping[i];
		}
	}

	return NULL;
}

int rtl_add_ps_drv_netif_mapping(struct net_device *dev, const char *name)
{
	int i;

	//duplicate check
	if(rtl_get_ps_drv_netif_mapping_by_psdev(dev) !=NULL)
		return FAILED;

	for(i = 0; i < NETIF_NUMBER;i++)
	{
		if(ps_drv_netif_mapping[i].valid == 0)
			break;
	}

	if(i == NETIF_NUMBER)
		return FAILED;

	ps_drv_netif_mapping[i].ps_netif = dev;
	memcpy(ps_drv_netif_mapping[i].drvName,name,strlen(name));
	ps_drv_netif_mapping[i].valid = 1;
	return SUCCESS;
}

int rtl_del_ps_drv_netif_mapping(struct net_device *dev)
{
	ps_drv_netif_mapping_t *entry;
	entry = rtl_get_ps_drv_netif_mapping_by_psdev(dev);
	if(entry)
		entry->valid = 0;

	return SUCCESS;
}

/*
 * Disable TX/RX through IO_CMD register
 */
static void rtl8186_stop_hw(struct net_device *dev, struct dev_priv *cp)
{

#if defined(PATCH_GPIO_FOR_LED)
	if (cp->id == RTL_LANVLANID) {
		int port;
		for (port=0; port<RTL8651_PHY_NUMBER; port++)
			disable_led_ctrl(port);
	}
#endif

}


/* Set or clear the multicast filter for this adaptor.
 * This routine is not state sensitive and need not be SMP locked.
 */
static void re865x_set_rx_mode (struct net_device *dev){
/*	Not yet implemented.
	unsigned long flags;
	spin_lock_irqsave (&_rtl86xx_dev.lock, flags);
	spin_unlock_irqrestore (&_rtl86xx_dev.lock, flags);
*/
}

#if defined (CONFIG_RTL_NIC_HWSTATS)
 void  re865x_accumulate_port_stats(uint32 portnum, struct net_device_stats *net_stats)
{
	uint32 addrOffset_fromP0 =0;

	if( portnum < 0 ||  portnum > CPU)
			return ;
	addrOffset_fromP0 = portnum * MIB_ADDROFFSETBYPORT;
	/* rx_pkt = rx_unicast +rx_multicast + rx_broadcast */
	net_stats->rx_packets += rtl8651_returnAsicCounter( OFFSET_IFINUCASTPKTS_P0 + addrOffset_fromP0 ) ;
	net_stats->rx_packets += rtl8651_returnAsicCounter( OFFSET_ETHERSTATSMULTICASTPKTS_P0 + addrOffset_fromP0 ) ;
	net_stats->rx_packets += rtl8651_returnAsicCounter( OFFSET_ETHERSTATSBROADCASTPKTS_P0 + addrOffset_fromP0 ) ;
	/* tx_pkt = tx_unicast +tx_multicast + tx_broadcast*/
	net_stats->tx_packets += rtl8651_returnAsicCounter( OFFSET_IFOUTUCASTPKTS_P0 + addrOffset_fromP0 ) ;
	net_stats->tx_packets += rtl8651_returnAsicCounter( OFFSET_IFOUTMULTICASTPKTS_P0 + addrOffset_fromP0 ) ;
	net_stats->tx_packets += rtl8651_returnAsicCounter( OFFSET_IFOUTBROADCASTPKTS_P0 + addrOffset_fromP0 ) ;


	net_stats->rx_bytes += rtl8651_returnAsicCounter( OFFSET_IFINOCTETS_P0 + addrOffset_fromP0 ) ;
	net_stats->rx_bytes += (rtl8651_returnAsicCounter( OFFSET_IFINOCTETS_P0+4 + addrOffset_fromP0 )<<22);
	net_stats->tx_bytes += rtl8651_returnAsicCounter( OFFSET_IFOUTOCTETS_P0 + addrOffset_fromP0 ) ;
	net_stats->tx_bytes += (rtl8651_returnAsicCounter( OFFSET_IFOUTOCTETS_P0+4 + addrOffset_fromP0 )<<22) ;

	/*rx_errors = CRC error + Jabber error  + Fragment error*/
	net_stats->rx_errors += rtl8651_returnAsicCounter( OFFSET_DOT3STATSFCSERRORS_P0 + addrOffset_fromP0 ) ;
	net_stats->rx_errors += rtl8651_returnAsicCounter( OFFSET_ETHERSTATSJABBERS_P0 + addrOffset_fromP0 ) ;
	net_stats->rx_errors += rtl8651_returnAsicCounter( OFFSET_ETHERSTATSOVERSIZEPKTS_P0 + addrOffset_fromP0 ) ;
	//OFFSET_DOT1DTPPORTINDISCARDS_P0?
	//net_stats->rx_dropped += rtl8651_returnAsicCounter( OFFSET_DOT1DTPPORTINDISCARDS_P0 + addrOffset_fromP0 ) ;
	net_stats->tx_dropped += rtl8651_returnAsicCounter( OFFSET_IFOUTDISCARDS + addrOffset_fromP0 ) ;
	net_stats->multicast += rtl8651_returnAsicCounter( OFFSET_ETHERSTATSMULTICASTPKTS_P0 + addrOffset_fromP0 ) ;

	net_stats->collisions += rtl8651_returnAsicCounter( OFFSET_ETHERSTATSCOLLISIONS_P0 + addrOffset_fromP0 ) ;
	net_stats->rx_crc_errors += rtl8651_returnAsicCounter( OFFSET_DOT3STATSFCSERRORS_P0 + addrOffset_fromP0 ) ;
	return;
}

void re865x_get_hardwareStats(struct dev_priv  *priv)
{
	uint32 portmask;
	uint32 port;

	portmask = priv->portmask;
	//rx_dropped = priv->net_stats.rx_dropped;
	//memset( &priv->net_stats, 0, sizeof(struct net_device_stats) );
	/* reset counters to 0 */
	priv->net_stats.rx_packets = 0;
	priv->net_stats.tx_packets = 0;
	priv->net_stats.rx_bytes = 0;
	priv->net_stats.tx_bytes = 0;
	priv->net_stats.rx_errors = 0;
	priv->net_stats.tx_dropped = 0;
	priv->net_stats.multicast = 0;
	priv->net_stats.collisions = 0;
	priv->net_stats.rx_crc_errors = 0;
	for( port = 0; port < CPU; port++)
	{
		if((1<<port) & portmask)
		{
			re865x_accumulate_port_stats(port, &priv->net_stats);
		}

	}
	return;
}
#endif

static struct net_device_stats *re865x_get_stats(struct net_device *dev){
	struct dev_priv  *dp;
	dp = dev->priv;
	#if defined (CONFIG_RTL_NIC_HWSTATS)
		re865x_get_hardwareStats(dp);
	#endif
	return &dp->net_stats;
}

#if defined (CONFIG_RTL_MULTI_LAN_DEV) || defined(CONFIG_RTK_VLAN_SUPPORT)
static int32 re865x_packVlanConfig(struct rtl865x_vlanConfig vlanConfig1[],  struct rtl865x_vlanConfig vlanConfig2[])
{
	int i, j;
	uint32 vlanCnt=0;
	uint32 found=FALSE;
	/*get input vlan config entry number*/
	for(i=0; vlanConfig1[i].ifname[0] != '\0'; i++)
	{
		if(vlanConfig1[i].vid != 0)
			vlanCnt++;
	}
	if(vlanCnt+1 > NETIF_NUMBER)
		DEBUG_ERR("ERROR,vlanCnt(%d) > max size %d\n",vlanCnt,NETIF_NUMBER-1);

	/*initialize output vlan config*/
	memset(vlanConfig2, 0 , (vlanCnt+1)*sizeof(struct rtl865x_vlanConfig));

	for(i=0; vlanConfig1[i].ifname[0] != '\0'; i++)
		{
		found=FALSE;

		if(vlanConfig1[i].vid == 0)
			continue;

		for(j=0; j<vlanCnt; j++)
		{
			if(vlanConfig1[i].if_type != vlanConfig2[j].if_type)
			{
				continue;
			}
			else
			{
				if(vlanConfig1[i].if_type==IF_ETHER)
				{
					/*if multiple vlan config has the same vlan id*/
					/*the first one will decide the  real network interface name/asic mtu/hardware address*/

					/*ethernet interface*/
					if(vlanConfig1[i].vid!=vlanConfig2[j].vid)
					{
						continue;
					}
					else
					{
						found=TRUE;
						break;
					}

				}
				else
				{
					/*PPP interface*/
					if(strcmp(vlanConfig1[i].ifname, vlanConfig2[j].ifname))
					{
						continue;
					}
					else
					{
						found=TRUE;
						break;
					}
				}
			}
		}

		if(found==TRUE)
		{
			/*merge port mask*/
			vlanConfig2[j].memPort |=vlanConfig1[i].memPort;
			vlanConfig2[j].untagSet |=vlanConfig1[i].untagSet;
		}
		else
		{
			/*find an empty entry to store this vlan config*/
			for(j=0; j<vlanCnt; j++)
			{
				if(vlanConfig2[j].vid==0)
				{
					memcpy(&vlanConfig2[j], &vlanConfig1[i], sizeof(struct rtl865x_vlanConfig));
					if(( vlanConfig1[i].if_type==IF_ETHER) && (vlanConfig1[i].isWan==FALSE) )
					{
						/*add cpu port to lan member list*/
						vlanConfig2[j].memPort|=0x100;
						vlanConfig2[j].untagSet|=0x100;
					}
					break;
				}
			}
		}

	}

	return SUCCESS;
}
#endif

static void rtl865x_disableDevPortForward(struct net_device *dev, struct dev_priv *cp)
{
	int port;
	for(port=0;port<RTL8651_AGGREGATOR_NUMBER;port++)
	{
		if((1<<port) & cp->portmask)
		{
#if 1
			REG32(PCRP0+(port<<2))= ((REG32(PCRP0+(port<<2)))&(~ForceLink));
			REG32(PCRP0+(port<<2))= ((REG32(PCRP0+(port<<2)))&(~EnablePHYIf));
			TOGGLE_BIT_IN_REG_TWICE(PCRP0+(port<<2),EnablePHYIf);
			TOGGLE_BIT_IN_REG_TWICE(PCRP0+(port<<2),ForceLink);
			TOGGLE_BIT_IN_REG_TWICE(PCRP0+(port<<2),EnForceMode);
#else
			rtl865xC_setAsicEthernetForceModeRegs(port, TRUE, FALSE, 1, TRUE);
#endif
		}
	}
#ifdef CONFIG_RTL_8196C_ESD
	if ((cp->portmask) & 0x10)			// port 4
		_96c_esd_counter = 0;		// stop counting
#endif
}

#if !defined(CONFIG_RTL_8196C)
static void rtl865x_restartDevPHYNway(struct net_device *dev, struct dev_priv *cp)
{
	int port;
	for(port=0;port<RTL8651_AGGREGATOR_NUMBER;port++)
	{
		if((1<<port) & cp->portmask)
		{
			rtl8651_restartAsicEthernetPHYNway(port);
		}
	}
	return;
}
#endif

#if defined(CONFIG_819X_PHY_RW) || defined(CONFIG_RTL_HW_VLAN_SUPPORT)
static void rtl865x_setPortForward(int port_num, int forward)
{
	if(port_num < 0 || port_num >= RTL8651_AGGREGATOR_NUMBER)
		return;

	if(forward == FALSE) {
		REG32(PCRP0+(port_num<<2))= ((REG32(PCRP0+(port_num<<2)))&(~EnablePHYIf));
#ifdef CONFIG_RTL_8196C_ESD
		if (port_num == 4)				// port 4
			_96c_esd_counter = 0;		// stop counting
#endif
	}
	else {
		REG32(PCRP0+(port_num<<2))= ((REG32(PCRP0+(port_num<<2)))|(EnablePHYIf));

#ifdef CONFIG_RTL_8196C_ESD
		if (port_num == 4) {				// port 4
			_96c_esd_counter = 1;		// start counting and check ESD
			_96c_esd_reboot_counter = 0;	// reset counter
		}
#endif
	}
	TOGGLE_BIT_IN_REG_TWICE(PCRP0+(port_num<<2),EnForceMode);

}
#endif	//#if defined(CONFIG_819X_PHY_RW)
static void rtl865x_enableDevPortForward(struct net_device *dev, struct dev_priv *cp)
{
	int port;
	for(port=0;port<RTL8651_AGGREGATOR_NUMBER;port++)
	{
		if((1<<port) & cp->portmask)
		{
#if 1
			REG32(PCRP0+(port<<2))= ((REG32(PCRP0+(port<<2)))|(ForceLink));
			REG32(PCRP0+(port<<2))= ((REG32(PCRP0+(port<<2)))|(EnablePHYIf));
			TOGGLE_BIT_IN_REG_TWICE(PCRP0+(port<<2),EnablePHYIf);
			TOGGLE_BIT_IN_REG_TWICE(PCRP0+(port<<2),ForceLink);
#else
			rtl865xC_setAsicEthernetForceModeRegs(port, FALSE, TRUE, 1, TRUE);
			rtl8651_restartAsicEthernetPHYNway(port);
#endif
		}
	}
#ifdef CONFIG_RTL_8196C_ESD
	if ((cp->portmask) & 0x10) {			// port 4
		_96c_esd_counter = 1;		// start counting and check ESD
		_96c_esd_reboot_counter = 0;	// reset counter
	}
#endif
}

static void rtl865x_disableInterrupt(void)
{
	REG32(CPUICR) = 0;
	REG32(CPUIIMR) = 0;
	REG32(CPUIISR) = REG32(CPUIISR);
}

static void rtk_queue_init(struct ring_que *que)
{
	memset(que, 0, sizeof(struct ring_que));
	que->ring = (struct sk_buff **)kmalloc(
		(sizeof(struct skb_buff*)*(rtl865x_maxPreAllocRxSkb+1))
		,GFP_ATOMIC);
	memset(que->ring, 0, (sizeof(struct sk_buff *))*(rtl865x_maxPreAllocRxSkb+1));
	que->qmax = rtl865x_maxPreAllocRxSkb;
}

static void rtk_queue_exit(struct ring_que *que)
{

	if(que->ring!=NULL)
	{		
		kfree(que->ring);
		que->ring=NULL;
	}
}


__MIPS16
__IRAM_FWD
static int rtk_queue_tail(struct ring_que *que, struct sk_buff *skb)
{
	int next;


	if (que->head == que->qmax)
		next = 0;
	else
		next = que->head + 1;

	if (que->qlen >= que->qmax || next == que->tail) {
		return 0;
	}

	que->ring[que->head] = skb;
	que->head = next;
	que->qlen++;

	return 1;
}

__MIPS16
__IRAM_FWD
static struct sk_buff *rtk_dequeue(struct ring_que *que)
{
	struct sk_buff *skb;

	if (que->qlen <= 0 || que->tail == que->head)
	{
		return NULL;
	}

	skb = que->ring[que->tail];

	if (que->tail == que->qmax)
		que->tail  = 0;
	else
		que->tail++;

	que->qlen--;

	return (struct sk_buff *)skb;
}

#if defined(CONFIG_RTL_ETH_PRIV_SKB_DEBUG)
int get_buf_in_rx_skb_queue(void)
{
	return rx_skb_queue.qlen;
}

int get_buf_in_poll(void)
{
	return eth_skb_free_num;
}
#endif

//__MIPS16
__IRAM_FWD
static void refill_rx_skb(void)
{
	struct sk_buff	*skb;
	unsigned long	flags;
	int			idx;

	idx = RTL865X_SWNIC_RXRING_MAX_PKTDESC -1;

#ifdef DELAY_REFILL_ETH_RX_BUF
	while (rx_skb_queue.qlen < rtl865x_maxPreAllocRxSkb || ((idx>=0)&&(SUCCESS==check_rx_pkthdr_ring(idx, &idx))))
#else
	while (rx_skb_queue.qlen < rtl865x_maxPreAllocRxSkb)
#endif
	{
		#if defined(CONFIG_RTL_ETH_PRIV_SKB)
		skb = dev_alloc_skb_priv_eth(CROSS_LAN_MBUF_LEN);
		#else
		skb = dev_alloc_skb(CROSS_LAN_MBUF_LEN);
		#endif

		if (skb == NULL) {
			DEBUG_ERR("EthDrv: dev_alloc_skb() failed!\n");
			return;
		}
		skb_reserve(skb, RX_OFFSET);

		#ifdef DELAY_REFILL_ETH_RX_BUF
		// return to rx descriptor ring first if the rings still have the OWN bit which be RISC_OWNED.
		if (idx>=0) {
			if  (SUCCESS==check_and_return_to_rx_pkthdr_ring(skb, idx)) {
				continue;
			}
		}
		#endif

		#if defined(RTK_QUE)
		local_irq_save(flags);
		rtk_queue_tail(&rx_skb_queue, skb);
		local_irq_restore(flags);
		#else
		__skb_queue_tail(&rx_skb_queue, skb);
		#endif
	}
}

//---------------------------------------------------------------------------
static void free_rx_skb(void)
{
	struct sk_buff *skb;

	swNic_freeRxBuf();

	while  (rx_skb_queue.qlen > 0) {
#if defined(RTK_QUE)
		skb = rtk_dequeue(&rx_skb_queue);
#else
		skb = __skb_dequeue(&rx_skb_queue);
#endif
		dev_kfree_skb_any(skb);
	}
}

//---------------------------------------------------------------------------
__IRAM_FWD
unsigned char *alloc_rx_buf(void **skb, int buflen)
{
	struct sk_buff *new_skb;
	unsigned long flags;

	if (rx_skb_queue.qlen == 0) {
		#if defined(CONFIG_RTL_ETH_PRIV_SKB)
		new_skb = dev_alloc_skb_priv_eth(CROSS_LAN_MBUF_LEN);
		#else
		new_skb = dev_alloc_skb(CROSS_LAN_MBUF_LEN);
		#endif
		if (new_skb == NULL) {
			DEBUG_ERR("EthDrv: alloc skb failed!\n");
		}
		else
			skb_reserve(new_skb, RX_OFFSET);
	}
	else {
		#if defined(RTK_QUE)
		local_irq_save(flags);
		new_skb = rtk_dequeue(&rx_skb_queue);
		local_irq_restore(flags);
		#else
		new_skb = __skb_dequeue(&rx_skb_queue);
		#endif
	}

	if (new_skb == NULL)
		return NULL;

#if 0//defined(CONFIG_RTL_ULINKER_BRSC)
	skb_reserve(new_skb, 2);
#endif

	*skb = new_skb;

	return new_skb->data;
}

//---------------------------------------------------------------------------
void free_rx_buf(void *skb)
{
	dev_kfree_skb_any((struct sk_buff *)skb);
}

//---------------------------------------------------------------------------
#if defined(CONFIG_RTL_FAST_BRIDGE)
void tx_done_callback(void *skb)
{
#if defined(CONFIG_RTL_FAST_BRIDGE)
	#define RTL_PRIV_DATA_SIZE	128
	unsigned long flags;
	//hyking:
	//queue private skb and buffer
	if (((struct sk_buff*)skb)->fast_br_forwarding_flags == 1 && is_rtl865x_eth_priv_buf(((struct sk_buff *)skb)->head))
	{
		if(!(skb_cloned(skb)))
		{
			//disable irq
			local_irq_save(flags);
			if(rtk_queue_tail(&rx_skb_queue,(struct sk_buff *)skb))
			{
				unsigned char *data;
				struct skb_shared_info *shinfo;
				int size;
				data = (((struct sk_buff *)skb)->head);

				 memset(skb, 0, offsetof(struct sk_buff, truesize));
			        atomic_set(&((struct sk_buff *)skb)->users, 1);
			        ((struct sk_buff *)skb)->head = data;
			        ((struct sk_buff *)skb)->data = data;
			        ((struct sk_buff *)skb)->tail = data;

				size = (CROSS_LAN_MBUF_LEN+128+NET_SKB_PAD);

			        ((struct sk_buff *)skb)->end  = data + size;
			        ((struct sk_buff *)skb)->truesize = size + sizeof(struct sk_buff);

			        /* make sure we initialize shinfo sequentially */
			        shinfo = skb_shinfo(skb);
			        atomic_set(&shinfo->dataref, 1);
			        shinfo->nr_frags  = 0;
			        shinfo->gso_size = 0;
			        shinfo->gso_segs = 0;
			        shinfo->gso_type = 0;
			        shinfo->ip6_frag_id = 0;
			        shinfo->frag_list = NULL;

#ifdef CONFIG_RTK_VOIP_VLAN_ID
			        ((struct sk_buff *)skb)->rx_vlan = 0;
			        ((struct sk_buff *)skb)->rx_wlan = 0;
			        ((struct sk_buff *)skb)->priority = 0;
#endif

#ifdef CONFIG_RTL_HARDWARE_MULTICAST
				((struct sk_buff *)skb)->srcPort=0xFFFF;
				((struct sk_buff *)skb)->srcVlanId=0;
#endif

#if	defined(CONFIG_RTL_QOS_8021P_SUPPORT)
				((struct sk_buff *)skb)->srcVlanPriority=0;
#endif

#if defined(CONFIG_NETFILTER_XT_MATCH_PHYPORT)|| defined(CONFIG_RTL_FAST_FILTER) || defined(CONFIG_RTL_QOS_PATCH) || defined(CONFIG_RTK_VOIP_QOS) || defined(CONFIG_RTK_VLAN_WAN_TAG_SUPPORT) ||defined(CONFIG_RTL_MAC_FILTER_CARE_INPORT)
				((struct sk_buff *)skb)->srcPhyPort=0xFF;
				((struct sk_buff *)skb)->dstPhyPort=0xFF;
#endif

#if defined(CONFIG_RTK_VLAN_SUPPORT)
				((struct sk_buff *)skb)->tag.v = 0;
#endif

#if defined (CONFIG_RTL_LOCAL_PUBLIC)
				((struct sk_buff *)skb)->srcLocalPublicIp=0;
				((struct sk_buff *)skb)->fromLocalPublic=0;
				((struct sk_buff *)skb)->toLocalPublic=0;
#endif

#ifdef CONFIG_RTK_VOIP_VLAN_ID
				skb_reserve(((struct sk_buff *)skb), RTL_PRIV_DATA_SIZE+4); // for VLAN TAG insertion
#else
				skb_reserve(((struct sk_buff *)skb), RTL_PRIV_DATA_SIZE);
#endif
				//reserve for 4 byte alignment
				skb_reserve(((struct sk_buff *)skb), RX_OFFSET);
				//enable irq
				local_irq_restore(flags);
				return;
			}

			//enable irq
			local_irq_restore(flags);
		}
	}
#endif
	dev_kfree_skb_any((struct sk_buff *)skb);
}
#endif

#if defined (CONFIG_RTL_IGMP_SNOOPING)
#if defined (CONFIG_BRIDGE)
extern void br_signal_igmpProxy(void);
#endif

static inline struct iphdr * re865x_getIpv4Header(uint8 *macFrame)
{
	uint8 *ptr;
	struct iphdr *iph=NULL;

	ptr=macFrame+12;
	if(*(int16 *)(ptr)==(int16)htons(ETH_P_8021Q))
	{
		ptr=ptr+4;
	}

	/*it's not ipv4 packet*/
	if(*(int16 *)(ptr)!=(int16)htons(ETH_P_IP))
	{
		return NULL;
	}

	iph=(struct iphdr *)(ptr+2);

	return iph;
}

#if defined (CONFIG_RTL_MLD_SNOOPING)
static inline struct ipv6hdr* re865x_getIpv6Header(uint8 *macFrame)
{
	uint8 *ptr;
	struct ipv6hdr *ipv6h=NULL;

	ptr=macFrame+12;
	if(*(int16 *)(ptr)==(int16)htons(ETH_P_8021Q))
	{
		ptr=ptr+4;
	}

	/*it's not ipv6 packet*/
	if(*(int16 *)(ptr)!=(int16)htons(ETH_P_IPV6))
	{
		return NULL;
	}

	ipv6h=(struct ipv6hdr *)(ptr+2);

	return ipv6h;
}

#define IPV6_ROUTER_ALTER_OPTION 0x05020000
#define  HOP_BY_HOP_OPTIONS_HEADER 0
#define ROUTING_HEADER 43
#define  FRAGMENT_HEADER 44
#define DESTINATION_OPTION_HEADER 60

#define PIM_PROTOCOL 103
#define MOSPF_PROTOCOL 89
#define TCP_PROTOCOL 6
#define UDP_PROTOCOL 17
#define NO_NEXT_HEADER 59
#define ICMP_PROTOCOL 58

#define MLD_QUERY 130
#define MLDV1_REPORT 131
#define MLDV1_DONE 132
#define MLDV2_REPORT 143

#define IS_IPV6_PIM_ADDR(ipv6addr) ((ipv6addr[0] == 0xFF020000)&&(ipv6addr[1] == 0x00000000)&&(ipv6addr[2] == 0x00000000)&&(ipv6addr[3] ==0x0000000D))
#define IS_IPV6_MOSPF_ADDR1(ipv6addr) ((ipv6addr[0] == 0xFF020000)&&(ipv6addr[1] == 0x00000000)&&(ipv6addr[2] == 0x00000000)&&(ipv6addr[3] ==0x00000005))
#define IS_IPV6_MOSPF_ADDR2(ipv6addr) ((ipv6addr[0] == 0xFF020000)&&(ipv6addr[1] == 0x00000000)&&(ipv6addr[2] == 0x00000000)&&(ipv6addr[3] ==0x00000006))


int re865x_getIpv6TransportProtocol(struct ipv6hdr* ipv6h)
{

	unsigned char *ptr=NULL;
	unsigned char *startPtr=NULL;
	unsigned char *lastPtr=NULL;
	unsigned char nextHeader=0;
	unsigned short extensionHdrLen=0;

	unsigned char  optionDataLen=0;
	unsigned char  optionType=0;
	unsigned int ipv6RAO=0;

	if(ipv6h==NULL)
	{
		return -1;
	}

	if(ipv6h->version!=6)
	{
		return -1;
	}

	startPtr= (unsigned char *)ipv6h;
	lastPtr=startPtr+sizeof(struct ipv6hdr)+(ipv6h->payload_len);
	nextHeader= ipv6h ->nexthdr;
	ptr=startPtr+sizeof(struct ipv6hdr);

	while(ptr<lastPtr)
	{
		switch(nextHeader)
		{
			case HOP_BY_HOP_OPTIONS_HEADER:
				/*parse hop-by-hop option*/
				nextHeader=ptr[0];
				extensionHdrLen=((uint16)(ptr[1])+1)*8;
				ptr=ptr+2;

				while(ptr<(startPtr+extensionHdrLen+sizeof(struct ipv6hdr)))
				{
					optionType=ptr[0];
					/*pad1 option*/
					if(optionType==0)
					{
						ptr=ptr+1;
						continue;
					}

					/*padN option*/
					if(optionType==1)
					{
						optionDataLen=ptr[1];
						ptr=ptr+optionDataLen+2;
						continue;
					}

					/*router altert option*/
					if(ntohl(*(uint32 *)(ptr))==IPV6_ROUTER_ALTER_OPTION)
					{
						ipv6RAO=IPV6_ROUTER_ALTER_OPTION;
						ptr=ptr+4;
						continue;
					}

					/*other TLV option*/
					if((optionType!=0) && (optionType!=1))
					{
						optionDataLen=ptr[1];
						ptr=ptr+optionDataLen+2;
						continue;
					}


				}

				break;

			case ROUTING_HEADER:
				nextHeader=ptr[0];
				extensionHdrLen=((uint16)(ptr[1])+1)*8;
                            ptr=ptr+extensionHdrLen;
				break;

			case FRAGMENT_HEADER:
				nextHeader=ptr[0];
				ptr=ptr+8;
				break;

			case DESTINATION_OPTION_HEADER:
				nextHeader=ptr[0];
				extensionHdrLen=((uint16)(ptr[1])+1)*8;
				ptr=ptr+extensionHdrLen;
				break;

			case ICMP_PROTOCOL:
				nextHeader=NO_NEXT_HEADER;
				if((ptr[0]==MLD_QUERY) ||(ptr[0]==MLDV1_REPORT) ||(ptr[0]==MLDV1_DONE) ||(ptr[0]==MLDV2_REPORT))
				{
					return ICMP_PROTOCOL;

				}
				break;

			case PIM_PROTOCOL:
				nextHeader=NO_NEXT_HEADER;
				if(IS_IPV6_PIM_ADDR(ipv6h->daddr.s6_addr32))
				{
					return PIM_PROTOCOL;
				}

				break;

			case MOSPF_PROTOCOL:
				nextHeader=NO_NEXT_HEADER;

				if(IS_IPV6_MOSPF_ADDR1(ipv6h->daddr.s6_addr32) || IS_IPV6_MOSPF_ADDR2(ipv6h->daddr.s6_addr32))
				{
					return MOSPF_PROTOCOL;
				}
				break;

			case TCP_PROTOCOL:
				nextHeader=NO_NEXT_HEADER;
				return TCP_PROTOCOL;

				break;

			case UDP_PROTOCOL:
				nextHeader=NO_NEXT_HEADER;
				return UDP_PROTOCOL;

				break;

			default:
				/*not ipv6 multicast protocol*/
				return -1;
				break;
		}

	}
	return -1;
}
#endif
static inline void re865x_relayTrappedMCast(struct sk_buff *skb, unsigned int vid, unsigned int mcastFwdPortMask, unsigned int keepOrigSkb)
{
	rtl_nicTx_info	nicTx;
 	struct sk_buff *skb2=NULL;

    #ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	struct sk_buff *skb_wan=NULL;
	rtl_nicTx_info	nicTx_wan;
	#endif

	if(mcastFwdPortMask==0)
	{
		return;
	}

	if(keepOrigSkb==TRUE)
	{
		skb2= skb_clone(skb, GFP_ATOMIC);
	}
	else
	{
		skb2=skb;
	}

       if(skb2!=NULL)
       {

#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
		if( vlan_bridge_tag && vlan_bridge_multicast_tag &&(vid == vlan_bridge_tag)&&(mcastFwdPortMask & RTL_WANPORT_MASK))
		{
			mcastFwdPortMask &= (~RTL_WANPORT_MASK);
			skb_wan = skb_copy(skb, GFP_ATOMIC);
			 if(skb_wan!=NULL)
		     {
		       	nicTx_wan.txIdx=0;
                nicTx_wan.vid = vlan_bridge_multicast_tag;
				nicTx_wan.portlist = RTL_WANPORT_MASK;
				nicTx_wan.srcExtPort = 0;
				
				nicTx_wan.tagport = RTL_WANPORT_MASK;
				
				// flush cache 0515 by tim
				_dma_cache_wback_inv((unsigned long) skb_wan->data, skb_wan->len);
				if (swNic_send((void *)skb_wan, skb_wan->data, skb_wan->len, &nicTx_wan) < 0)
				{
					dev_kfree_skb_any(skb_wan);
				}
		}
	}

		if(!mcastFwdPortMask)
		{
			dev_kfree_skb_any(skb2);
			return;
	}
#endif
       	nicTx.txIdx=0;
#if defined(CONFIG_RTL_QOS_PATCH)|| defined(CONFIG_RTK_VOIP_QOS)
	if(((struct sk_buff *)skb)->srcPhyPort == QOS_PATCH_RX_FROM_LOCAL)
{
		nicTx.priority = QOS_PATCH_HIGH_QUEUE_PRIO;
		nicTx.txIdx=RTL865X_SWNIC_TXRING_MAX_PKTDESC-1;	//use the highest tx ring index, note: not RTL865X_SWNIC_TXRING_HW_PKTDESC-1
	}
#endif
		nicTx.vid = vid;
		nicTx.portlist = mcastFwdPortMask;
		nicTx.srcExtPort = 0;
		nicTx.flags = (PKTHDR_USED|PKT_OUTGOING);
	#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT	
		if( (vlan_tag && vid == vlan_tag) || (vlan_bridge_tag && (vid == vlan_bridge_tag)))
		{
			nicTx.tagport = RTL_WANPORT_MASK;
		}	
	#endif	
		_dma_cache_wback_inv((unsigned long)skb2->data, skb2->len);
		if (swNic_send((void *)skb2, skb2->data, skb2->len, &nicTx) < 0)
		{
			dev_kfree_skb_any(skb2);
		}

	}
	return;
}

void rtl865x_igmpSyncLinkStatus(void)
{
	rtl_igmpPortInfo_t portInfo;

	portInfo.linkPortMask=newLinkPortMask;
	rtl865x_igmpLinkStatusChangeCallback(nicIgmpModuleIndex, &portInfo);

	#if defined (CONFIG_BRIDGE)
	if((newLinkPortMask & (~curLinkPortMask))!=0)
	{
		/*there is some port linking up*/
		/*notice igmp proxy daemon to send general query to newly link up client*/
		#ifdef CONFIG_RTL8196BU_8186SDK_MP_SPI
		#else
		br_signal_igmpProxy();
		#endif
	}
	#endif

	return;

}

#endif	/*	defined (CONFIG_RTL_IGMP_SNOOPING)	*/


#if defined (CONFIG_RTL_IGMP_SNOOPING)
#if defined (CONFIG_NETFILTER)
unsigned int (*IgmpRxFilter_Hook)(struct sk_buff *skb,
	     unsigned int hook,
	     const struct net_device *in,
	     const struct net_device *out,
	     struct xt_table *table);
EXPORT_SYMBOL(IgmpRxFilter_Hook);

static bool rtl_MulticastRxFilterOff(struct sk_buff *skb, int ipversion)
{
	bool ret =  true;
	if(IgmpRxFilter_Hook == NULL)
	{
		DEBUG_ERR("IgmpRxFilter_hook is NULL\n");
		return false;
	}
	if(ipversion ==4)
		skb->network_header = (sk_buff_data_t)re865x_getIpv4Header(skb->data);
	else if(ipversion ==6)
		skb->network_header = (sk_buff_data_t)re865x_getIpv6Header(skb->data);
	else
		return ret;//error shouldn't happen
#ifdef NET_SKBUFF_DATA_USES_OFFSET
	skb->mac_header = (sk_buff_data_t)(skb->data - skb->head);
#else
	skb->mac_header = (sk_buff_data_t)skb->data;
#endif

	//data should point to l3 header while doing iptables check
		skb->data = skb->data+ETH_HLEN;

	if(ipversion ==4)
	{
		struct net_device	*origDev=skb->dev;
		if((skb->dev->br_port!=NULL))
		{
			skb->dev=__dev_get_by_name(dev_net(skb->dev),RTL_PS_BR0_DEV_NAME);
			
		}
		ret = ((IgmpRxFilter_Hook(skb, NF_INET_PRE_ROUTING,  skb->dev, NULL,dev_net(skb->dev)->ipv4.iptable_filter)) !=NF_ACCEPT);
		skb->dev=origDev;
	}
	else if(ipversion ==6)
		ret = false;//ipv6 hava no iptables rule now

	if(ret)
	{
		DEBUG_ERR(" filter a v%d pkt\n", ipversion);
	}
	// return point to l2 header
	skb->data = skb->data-ETH_HLEN;
	return ret;
}
#endif

__MIPS16
int  rtl_MulticastRxCheck(struct sk_buff *skb,rtl_nicRx_info *info)
{

	unsigned int  vlanRelayPortMask=0;
	struct iphdr *iph=NULL;
	struct rtl_multicastDataInfo multicastDataInfo;
	struct rtl_multicastFwdInfo nicMCastFwdInfo;
	struct rtl_multicastFwdInfo br0MCastFwdInfo;

#if defined (CONFIG_RTL_MLD_SNOOPING)
	struct ipv6hdr *ipv6h=NULL;
#endif
	unsigned int l4Protocol=0;
	int ret=FAILED;
#if defined (CONFIG_RTL_MLD_SNOOPING)
	struct dev_priv *cp_this=info->priv;
#endif
	int vid=info->vid;
	int pid=info->pid;

	if((skb->data[0] &0x01) ==0)
	{
		return -1;
	}

	/*set flooding port mask first*/
	vlanRelayPortMask=rtl865x_getVlanPortMask(vid) & (~(1<<pid)) & ((1<<RTL8651_MAC_NUMBER)-1);

	if((skb->data[0]==0x01) && (skb->data[1]==0x00)&& (skb->data[2]==0x5e))
	{
		#if defined(CONFIG_RTK_VLAN_SUPPORT)
		if(rtk_vlan_support_enable)
		{
			/*let bridge handle it*/
			return 0;
		}
		#endif
		#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		skb->srcVlanId=vid;
		skb->srcPort=pid;
		#endif

		/*hardware ip multicast table will trap 0x01-00-5e-XX-XX-XX type packets*/
		/*how about other packets not trapped by hardware multicast table?---->we assume it has been flooded by l2 table*/
		iph=re865x_getIpv4Header(skb->data);
		if(iph!=NULL)
		{
			/*udp or tcp packet*/
			l4Protocol=iph->protocol;
			if((l4Protocol==IPPROTO_UDP) || (l4Protocol==IPPROTO_TCP))
			{


				/*relay packets which are trapped by hardware multicast table*/
				#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
                #ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
				if(vlan_bridge_tag && !strcmp(cp_this->dev->name,RTL_PS_ETH_NAME_ETH2))
				{
					if(igmpsnoopenabled && (nicIgmpModuleIndex_2!=0xFFFFFFFF))
					{
						multicastDataInfo.ipVersion=4;
						multicastDataInfo.sourceIp[0]=  (uint32)(iph->saddr);
						multicastDataInfo.groupAddr[0]=  (uint32)(iph->daddr);
						ret=rtl_getMulticastDataFwdInfo(nicIgmpModuleIndex_2, &multicastDataInfo, &nicMCastFwdInfo);
						vlanRelayPortMask=rtl865x_getVlanPortMask(vid)& (~(1<<pid)) & nicMCastFwdInfo.fwdPortMask & ((1<<RTL8651_MAC_NUMBER)-1);
						if(ret==SUCCESS)
						{

						}
						else
						{
							ret=rtl_getMulticastDataFwdInfo(brIgmpModuleIndex_2, &multicastDataInfo, &br0MCastFwdInfo);
							if(ret==SUCCESS)
							{
								/*there is wireless client,can not flooding in vlan */
								vlanRelayPortMask=0;
							}
						}

					}
				}
				else
				#endif
				if(igmpsnoopenabled && (nicIgmpModuleIndex!=0xFFFFFFFF))
				{
					multicastDataInfo.ipVersion=4;
				#if defined(CONFIG_RTL_ULINKER_BRSC) // assign value will cause coredump
					memcpy(&multicastDataInfo.sourceIp, &iph->saddr, 4);
					memcpy(&multicastDataInfo.groupAddr, &iph->daddr, 4);
				#else
					multicastDataInfo.sourceIp[0]=  (uint32)(iph->saddr);
					multicastDataInfo.groupAddr[0]=  (uint32)(iph->daddr);
				#endif

					ret=rtl_getMulticastDataFwdInfo(nicIgmpModuleIndex, &multicastDataInfo, &nicMCastFwdInfo);
					vlanRelayPortMask=rtl865x_getVlanPortMask(vid)& (~(1<<pid)) & nicMCastFwdInfo.fwdPortMask &cp_this->portmask& ((1<<RTL8651_MAC_NUMBER)-1);

					if(ret==SUCCESS)
					{

					}
					else
					{
						ret=rtl_getMulticastDataFwdInfo(brIgmpModuleIndex, &multicastDataInfo, &br0MCastFwdInfo);
						if(ret==SUCCESS)
						{
							/*there is wireless client,can not flooding in vlan */
							vlanRelayPortMask=0;
						}

					}
				}
				re865x_relayTrappedMCast( skb, vid, vlanRelayPortMask, TRUE);
				#endif/*end of CONFIG_RTL_HARDWARE_MULTICAST*/
			}
			else if(l4Protocol==IPPROTO_IGMP)
			{
                #ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
				if(vlan_bridge_tag && !strcmp(cp_this->dev->name,RTL_PS_ETH_NAME_ETH2))
				{

					if(igmpsnoopenabled && (nicIgmpModuleIndex_2!=0xFFFFFFFF))
					{
						rtl_igmpMldProcess(nicIgmpModuleIndex_2, skb->data, pid, &vlanRelayPortMask);
						//just flooding
						vlanRelayPortMask=rtl865x_getVlanPortMask(vid) & (~(1<<pid)) & ((1<<RTL8651_MAC_NUMBER)-1);
					}
				}
				else
				{	
				#endif
				if(igmpsnoopenabled && (nicIgmpModuleIndex!=0xFFFFFFFF))
				{
					/*igmp packet*/
				#if defined (CONFIG_NETFILTER)
					if(rtl_MulticastRxFilterOff(skb, 4) == true)
						return 0;//filter by iptables
				#endif
					rtl_igmpMldProcess(nicIgmpModuleIndex, skb->data, pid, &vlanRelayPortMask);
					vlanRelayPortMask=rtl865x_getVlanPortMask(vid) & vlanRelayPortMask & ((1<<RTL8651_MAC_NUMBER)-1);
				}
		#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
				}
		#endif
				re865x_relayTrappedMCast( skb, vid, vlanRelayPortMask, TRUE);

			}
			else
			{

				#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
				re865x_relayTrappedMCast( skb, vid, vlanRelayPortMask, TRUE);
				#endif

			}
		}


	}
#if defined (CONFIG_RTL_MLD_SNOOPING)
	else if ((skb->data[0]==0x33) && (skb->data[1]==0x33) && (skb->data[2]!=0xff))
	{
		#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		skb->srcVlanId=0;
		skb->srcPort=0xFFFF;
		#endif

		if(mldSnoopEnabled!=TRUE)
		{
			return 0;
		}

		#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
		if ((rtl_isPassthruFrame(skb->data)==SUCCESS)&&(rtl_isWanDev(cp_this)==TRUE))
		{
			/*don't relay it,let linux protocol stack bridge handle it*/
			return 0;
		}
		#endif

		#if defined(CONFIG_RTK_VLAN_SUPPORT)
		if(rtk_vlan_support_enable)
		{
			/*let bridge handle it*/
			return 0;
		}
		#endif

		/*when enable mld snooping, gateway will add acl to trap packet with dmac equal to 0x33-33-xx-xx-xx-xx */
		ipv6h=re865x_getIpv6Header(skb->data);
		if(ipv6h!=NULL)
		{
			l4Protocol=re865x_getIpv6TransportProtocol(ipv6h);
			/*udp or tcp packet*/
			if((l4Protocol==IPPROTO_UDP) || (l4Protocol==IPPROTO_TCP))
			{

				if(igmpsnoopenabled && (nicIgmpModuleIndex!=0xFFFFFFFF))
				{
					multicastDataInfo.ipVersion=6;
					memcpy(&multicastDataInfo.sourceIp, &ipv6h->saddr, 16);
					memcpy(&multicastDataInfo.groupAddr, &ipv6h->daddr, 16);

					ret=rtl_getMulticastDataFwdInfo(nicIgmpModuleIndex, &multicastDataInfo, &nicMCastFwdInfo);

					vlanRelayPortMask=rtl865x_getVlanPortMask(vid)& (~(1<<pid)) & nicMCastFwdInfo.fwdPortMask &cp_this->portmask& ((1<<RTL8651_MAC_NUMBER)-1);
					if(ret==SUCCESS)
					{

					}
					else
					{
						ret=rtl_getMulticastDataFwdInfo(brIgmpModuleIndex, &multicastDataInfo, &br0MCastFwdInfo);
						if(ret==SUCCESS)
						{
							/*there is wireless client,can not flooding in vlan */
							vlanRelayPortMask=0;
						}
					}
				}

				re865x_relayTrappedMCast( skb, vid, vlanRelayPortMask, TRUE);

			}
			else if(l4Protocol==IPPROTO_ICMPV6)
			{
				/*icmp packet*/
				if(igmpsnoopenabled && (nicIgmpModuleIndex!=0xFFFFFFFF))
				{
					rtl_igmpMldProcess(nicIgmpModuleIndex, skb->data, pid, &vlanRelayPortMask);
					vlanRelayPortMask=rtl865x_getVlanPortMask(vid) & vlanRelayPortMask & ((1<<RTL8651_MAC_NUMBER)-1);
				}

				re865x_relayTrappedMCast( skb, vid, vlanRelayPortMask, TRUE);

			}
			else
			{
				re865x_relayTrappedMCast( skb, vid, vlanRelayPortMask, TRUE);
			}
		}
		else
		{
			re865x_relayTrappedMCast( skb, vid, vlanRelayPortMask, TRUE);
		}

	}
#endif
	else
	{
		#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		skb->srcVlanId=0;
		skb->srcPort=0xFFFF;
		#endif
	}

	#if 0
	if(rx_skb_queue.qlen < (rtl865x_maxPreAllocRxSkb/3))
	{
		refill_rx_skb();
	}
	#endif

	return 0;
}
#endif	/*end of CONFIG_RTL865X_IGMP_SNOOPING*/

static inline int32 rtl_isWanDev(struct dev_priv *cp)
{
#if defined(CONFIG_RTK_VLAN_SUPPORT)
	return (!cp->vlan_setting.is_lan);
#else
	#if defined(CONFIG_RTL_MULTIPLE_WAN)
		return (cp->id==RTL_WANVLANID || cp->id == RTL_WAN_1_VLANID);
	#else
		return (cp->id==RTL_WANVLANID );
	#endif
#endif
}

#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
static inline int32 rtl_isPassthruFrame(uint8 *data)
{
	int	ret;

	ret = FAILED;
	if (oldStatus)
	{
		if (oldStatus&IP6_PASSTHRU_MASK)
		{
			if ((*((uint16*)(data+(ETH_ALEN<<1)))==__constant_htons(ETH_P_IPV6)) ||
				((*((uint16*)(data+(ETH_ALEN<<1)))==__constant_htons(ETH_P_8021Q))&&(*((uint16*)(data+(ETH_ALEN<<1)+VLAN_HLEN))==__constant_htons(ETH_P_IPV6))))
			{
				ret = SUCCESS;
			}
		}
		#if defined(CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE)
		if (oldStatus&PPPOE_PASSTHRU_MASK)
		{
			if (((*((uint16*)(data+(ETH_ALEN<<1)))==__constant_htons(ETH_P_PPP_SES))||(*((uint16*)(data+(ETH_ALEN<<1)))==__constant_htons(ETH_P_PPP_DISC))) ||
				((*((uint16*)(data+(ETH_ALEN<<1)))==__constant_htons(ETH_P_8021Q))&&((*((uint16*)(data+(ETH_ALEN<<1)+VLAN_HLEN))==__constant_htons(ETH_P_PPP_SES))||(*((uint16*)(data+(ETH_ALEN<<1)+VLAN_HLEN))==__constant_htons(ETH_P_PPP_DISC)))))
			{
				ret = SUCCESS;
			}
		}
		#endif
	}

	return ret;
}
#endif

#if defined(RTL_CPU_QOS_ENABLED)
#define MAX_HIGH_PRIO_TRY	20	//Try to rx high prio pkt for MAX_HIGH_PRIO_TRY times.
__DRAM_FWD static 	int	highPrioRxTryCnt;
__DRAM_FWD	static	int	highestPriority;
__DRAM_FWD static 	int	cpuQosHoldLow;
__DRAM_FWD static 	int	totalLowQueueCnt;

static rtl_queue_entry	pktQueueByPri[RTL865X_SWNIC_RXRING_MAX_PKTDESC] = {{0}};

__MIPS16
__IRAM_FWD
int rtl_enqueueSkb(rtl_nicRx_info *info)
{
	rtl_queue_entry	*entry;

	entry = &pktQueueByPri[info->priority];

	if (info->priority<cpuQosHoldLow)
		totalLowQueueCnt++;

	memcpy(&entry->entry[entry->end], info, sizeof(rtl_nicRx_info));
	entry->cnt++;
	entry->end = (entry->end==(RTL_NIC_QUEUE_LEN-1))?0:(entry->end+1);

	if (entry->cnt==RTL_NIC_QUEUE_LEN)
		return SUCCESS;
	else
		return FAILED;
}

__MIPS16
__IRAM_FWD
int rtl_dequeueSkb(rtl_nicRx_info *info)
{
	rtl_queue_entry	*entry;

	entry = &pktQueueByPri[info->priority];

	if(entry->cnt==0)
	{
		return FAILED;
	}
	else
	{
		memcpy(info, &entry->entry[entry->start], sizeof(rtl_nicRx_info));
		entry->cnt--;
		entry->start = (entry->start==(RTL_NIC_QUEUE_LEN-1))?0:(entry->start+1);
		return SUCCESS;
	}
}
#endif

#if defined(RTL_CPU_QOS_ENABLED)
__MIPS16
__IRAM_FWD
static inline int32 rtl_processReceivedInfo(rtl_nicRx_info *info, int nicRxRet)
{
	int	ret;

	ret = RTL_RX_PROCESS_RETURN_BREAK;
	switch(nicRxRet)
	{
		case RTL_NICRX_OK:
			{
				if (highestPriority<info->priority)
				{
					highestPriority = info->priority;
					cpuQosHoldLow = highestPriority;
				}

				if (info->priority==(RTL865X_SWNIC_RXRING_MAX_PKTDESC-1))
				{
					ret = RTL_RX_PROCESS_RETURN_SUCCESS;
				}
				else	if (rtl_enqueueSkb(info) == SUCCESS)
				{
					rtl_dequeueSkb(info);
					ret = RTL_RX_PROCESS_RETURN_SUCCESS;
				}
				else
					ret = RTL_RX_PROCESS_RETURN_CONTINUE;

				break;
			}
		case RTL_NICRX_NULL:
			{
				info->priority = cpuQosHoldLow;
				if (rtl_dequeueSkb(info)==SUCCESS)
				{
					ret = RTL_RX_PROCESS_RETURN_SUCCESS;
				}
				else if((highestPriority>0) && ((--highPrioRxTryCnt)<0))
				{
					//Only for using rxring 0 and rxring 5
					highPrioRxTryCnt=MAX_HIGH_PRIO_TRY;
					highestPriority=0;
					cpuQosHoldLow=highestPriority;
					ret = RTL_RX_PROCESS_RETURN_CONTINUE;
				}
				else if(highestPriority==0)
				{
					/* highestPriority=0 */
					if (cpuQosHoldLow>0)
					{
						swNic_flushRxRingByPriority(cpuQosHoldLow);
					}
					ret = RTL_RX_PROCESS_RETURN_BREAK;
				}

				break;
			}
		case RTL_NICRX_REPEAT:
			ret = RTL_RX_PROCESS_RETURN_BREAK;
			break;
	}
	return ret;
}
#else	/*	defined(RTL_CPU_QOS_ENABLED)	*/
__MIPS16
__IRAM_FWD
static inline int32 rtl_processReceivedInfo(rtl_nicRx_info *info, int nicRxRet)
{
	int	ret;

	ret = RTL_RX_PROCESS_RETURN_BREAK;
	switch(nicRxRet)
	{
		case RTL_NICRX_OK:
			{
				ret = RTL_RX_PROCESS_RETURN_SUCCESS;
				break;
			}
		case RTL_NICRX_NULL:
		case RTL_NICRX_REPEAT:
			break;
	}
	return ret;
}
#endif	/*	defined(RTL_CPU_QOS_ENABLED)	*/

__MIPS16
__IRAM_FWD
static inline int32 rtl_decideRxDevice(rtl_nicRx_info *info)
{
	struct dev_priv	*cp;
	int32			pid, i, ret;
	struct sk_buff 	*skb;
	uint8*			data;
	#if defined(CONFIG_RTL_STP)
	int32 			dev_no;
	#endif
	#if defined(CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE)
	unsigned char dest_mac[MAX_ADDR_LEN];
	#endif

#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	int32 vid=0;
	vid = info->vid;
#endif
	pid = info->pid;
	skb = info->input;
	data = skb->data;
	#if defined(CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE)
	memcpy(dest_mac, data, 6);
	#endif

	info->isPdev=FALSE;
	ret = SUCCESS;

	#if defined(CONFIG_RTL_STP)
	info->isStpVirtualDev=FALSE;
	if ((data[0]&0x01) && !memcmp(data, STPmac, 5) && !(data[5] & 0xF0))
	{
		/* It's a BPDU */
		dev_no = re865x_stp_get_pseudodevno(pid);
		if (dev_no != NO_MAPPING)
		{
			info->priv = _rtl86xx_dev.stp_port[dev_no]->priv;
			info->isStpVirtualDev=TRUE;
		}
		else
		{
			dev_kfree_skb_any(skb);
			ret = FAILED;
		}
	}
	else
	#endif
	{
		#if defined(CONFIG_RTL_MULTIPLE_WAN)
		if(rtl865x_curOpMode == GATEWAY_MODE){
			//mac based decision
			for(i = 0; i < ETH_INTF_NUM; i++)
			{
	                        cp = ((struct dev_priv *)_rtl86xx_dev.dev[i]->priv);
	                        if(cp && cp->opened && memcmp(skb->data,cp->dev->dev_addr,6) == 0)
	                        {
	                                info->priv = cp;
	                                goto out;
	                        }
	              }
			//rtl_multiWan_config
			if(rtl_multiWan_net_dev)
			{
				cp = (struct dev_priv *)rtl_multiWan_net_dev->priv;
				if(cp && cp->opened && memcmp(skb->data,cp->dev->dev_addr,6) == 0)
		               {
		               	info->priv = cp;
		                      goto out;
		              }
			}
		}
		#endif

		for(i = 0; i < ETH_INTF_NUM; i++)
		{
			cp = ((struct dev_priv *)_rtl86xx_dev.dev[i]->priv);
			//printk("=========%s(%d),cp(%s),i(%d)\n",__FUNCTION__,__LINE__,cp->dev->name,i);
 #ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
			if(*((unsigned short *)(skb->data+(ETH_ALEN<<1)))== __constant_htons(ETH_P_8021Q))   
        	{
        		vid = *((unsigned short *)(skb->data+(ETH_ALEN<<1)+2))&0x0fff;
			
				if( vlan_tag && vid == vlan_tag)
				{
					if(rtl865x_curOpMode == BRIDGE_MODE)
						vid = RTL_LANVLANID;
				}
				else if (vlan_bridge_tag && vid == vlan_bridge_tag)
       			{
					vid = vlan_bridge_tag;
				}
				else if(vlan_bridge_multicast_tag && vid == vlan_bridge_multicast_tag)
                {
					vid = vlan_bridge_tag;
                }
				#ifdef CONFIG_VLAN_8021Q
				else
                {
					vid = vlan_tag; //default from WAN eth1
                }
				#endif
				if(cp && cp->opened && (vid==cp->id))
				{
					info->priv = cp;
					ret = SUCCESS;
					break;
				}
			}
			else
			{
				if(cp && cp->opened && (cp->portmask & (1<<pid)))
				{
						info->priv = cp;
						ret = SUCCESS;
						break;
					}
				}
		#else
			if(cp && cp->opened && (cp->portmask & (1<<pid)))
			{
				info->priv = cp;
				break;
			}
		#endif
		}

		//printk("====%s(%d),dev(%s),i(%d)\n",__FUNCTION__,__LINE__,cp->dev->name,i);
		if(ETH_INTF_NUM==i)
		{
			info->priv = NULL;
			dev_kfree_skb_any(skb);
			ret = FAILED;
		}
		#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
		else if (SUCCESS==rtl_isPassthruFrame(data)&&(rtl_isWanDev(cp)==TRUE)
			#if defined(CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE)
			&& (compare_ether_addr((char* )cp->dev->dev_addr, (char*)dest_mac))
			#endif
			)
		{
			info->priv = _rtl86xx_dev.pdev->priv;
			info->isPdev=TRUE;
		}
		#endif
	}
#if defined(CONFIG_RTL_MULTIPLE_WAN)
out:
#endif
	return ret;
}

#if defined(CONFIG_RTL_ULINKER_BRSC)
#include "linux/ulinker_brsc.h"
#endif

#if defined(BR_SHORTCUT)
__MIPS16
__IRAM_FWD
static inline int32 rtl_processBridgeShortCut(struct sk_buff *skb, struct dev_priv *cp_this, rtl_nicRx_info *info)
{
	struct net_device *dev;
	/*2011-09-13 fix wlan sta can not access internet when wan mac clone sta mac*/
	if(rtl_isWanDev(cp_this) && (rtl865x_curOpMode == GATEWAY_MODE))
	{
		return FAILED;
	}

	if (
		#if 0
		(eth_flag & BIT(2)) &&
		#endif
#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT) || defined(CONFIG_RTL_ULINKER)
				(skb->data[37] != 68) && /*port 68 is dhcp dest port. In order to hack dns ip, so dhcp packag                                                           can't enter bridge short cut.*/
#endif
	#if defined(CONFIG_WIRELESS_LAN_MODULE)
		(wirelessnet_hook_shortcut !=NULL ) && ((dev = wirelessnet_hook_shortcut(skb->data)) != NULL)
	#else
	  #if defined(CONFIG_RTL_ULINKER_BRSC)
		(((dev=brsc_get_cached_dev(0, skb->data))!=NULL) || ((dev = get_shortcut_dev(skb->data)) != NULL)) 
	  #else
		((dev = get_shortcut_dev(skb->data)) != NULL)
	  #endif
	#endif
    #ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
		&& rtl865x_same_root(skb->dev,dev)
	#endif
	)
	{
	#if defined(CONFIG_RTL_ULINKER_BRSC)
		if (cached_usb.dev && dev == cached_usb.dev) {
			BRSC_COUNTER_UPDATE(tx_eth_sc);		
			BDBG_BRSC("BRSC: get shortcut dev[%s]\n", cached_usb.dev->name);

			if (skb->dev)
				brsc_cache_dev(2, skb->dev, skb->data+ETH_ALEN);
		}
		else
	#endif /* #if defined(CONFIG_RTL_ULINKER_BRSC) */
	{
		#if defined(CONFIG_RTL_HARDWARE_NAT)
		if (memcmp(&skb->data[ETH_ALEN], cp_this->dev->dev_addr, ETH_ALEN))
		#endif
		{
			#ifdef BR_SHORTCUT_C2
			/*
			if (cached_dev == NULL) {
				memcpy(cached_eth_addr, &skb->data[ETH_ALEN], ETH_ALEN);
				cached_dev = cp_this->dev;
				last_used = 0;
			}

			else if (cached_dev2 == NULL) {
				memcpy(cached_eth_addr2, &skb->data[ETH_ALEN], ETH_ALEN);
				cached_dev2 = cp_this->dev;
				last_used = 1;
			}

			else */
			if (memcmp(cached_eth_addr, &skb->data[ETH_ALEN], ETH_ALEN) == 0) {
				//memcpy(cached_eth_addr, &skb->data[ETH_ALEN], ETH_ALEN);
				cached_dev = cp_this->dev;
				last_used = 0;
			}
			else if (memcmp(cached_eth_addr2, &skb->data[ETH_ALEN], ETH_ALEN) == 0) {
				//memcpy(cached_eth_addr2, &skb->data[ETH_ALEN], ETH_ALEN);
				cached_dev2 = cp_this->dev;
				last_used = 1;
			}
			else if (last_used == 1) {
				memcpy(cached_eth_addr, &skb->data[ETH_ALEN], ETH_ALEN);
				cached_dev = cp_this->dev;
				last_used = 0;
			}
			else if (last_used == 0) {
				memcpy(cached_eth_addr2, &skb->data[ETH_ALEN], ETH_ALEN);
				cached_dev2 = cp_this->dev;
				last_used = 1;
			}


			#else
			memcpy(cached_eth_addr, &skb->data[ETH_ALEN], ETH_ALEN);
			cached_dev = cp_this->dev;
			#endif
		}
	}
		#if defined(CONFIG_RTL_HW_QOS_SUPPORT) && defined(CONFIG_RTL_HARDWARE_NAT)
		skb->cb[0] = info->rxPri;
		#endif
		/*skb->dev = dev;*/ /* for performance */
		#if defined(CONFIG_COMPAT_NET_DEV_OPS)
		dev->hard_start_xmit(skb, dev);
		#else
		dev->netdev_ops->ndo_start_xmit(skb,dev);
		#endif
		DEBUG_ERR("[%s][%d]-[%s]\n", __FUNCTION__, __LINE__, skb->dev->name);
		return SUCCESS;
	#if 0
	} else if (info->vid == PKTHDR_EXTPORT_MAGIC) {
		/*	the pkt should be tx out by a wlanx interface.
		*	But we don't not which one...
		*	Pass it to protocol stack.
		*/
		/*
		cp_this->net_stats.rx_dropped++;
		dev_kfree_skb_any(skb);
		return SUCCESS;
		*/
	#endif
	}
	return FAILED;
}
#elif defined(CONFIG_RTL_HARDWARE_NAT)&&(defined(CONFIG_RTL8192SE)||defined(CONFIG_RTL8192CD))
#if 0
__MIPS16
__IRAM_FWD
static inline int32 rtl_processExtensionPortShortCut(struct sk_buff *skb, struct dev_priv *cp_this, rtl_nicRx_info *info)
{
	struct net_device *dev;

	if ((PKTHDR_EXTPORT_MAGIC!=info->vid)||(info->pid!=PKTHDR_EXTPORT_P3))
		return FAILED;

	dev = get_shortcut_dev(skb->data);

	if (dev!=NULL) {
		#if defined(CONFIG_RTL_HW_QOS_SUPPORT) && defined(CONFIG_RTL_HARDWARE_NAT)
		skb->cb[0] = info->rxPri;
		#endif
		/*skb->dev = dev;*/ /* for performance */
		#if defined(CONFIG_COMPAT_NET_DEV_OPS)
		dev->hard_start_xmit(skb, dev);
		#else
		dev->netdev_ops->ndo_start_xmit(skb,dev);
		#endif
		DEBUG_ERR("[%s][%d]-[%s]\n", __FUNCTION__, __LINE__, skb->dev->name);

		return SUCCESS;
	#if 0
	} else {
		/*	the pkt should be tx out by a wlanx interface.
		*	But we don't not which one...
		*	Pass it to protocol stack.
		*/
		/*
		cp_this->net_stats.rx_dropped++;
		dev_kfree_skb_any(skb);
		*/
	#endif
	}

	return FAILED;
}
#endif
#endif

__MIPS16
__IRAM_FWD
static inline void rtl_processRxToProtcolStack(struct sk_buff *skb, struct dev_priv *cp_this)
{
	skb->protocol = eth_type_trans(skb, skb->dev);
	skb->ip_summed = CHECKSUM_NONE;
	//printk("[%s][%d]-skb->dev[%s],proto(0x%x)\n", __FUNCTION__, __LINE__, skb->dev->name,skb->protocol);

#if defined(RX_TASKLET)
	#if defined(CONFIG_RTL_LOCAL_PUBLIC)
	skb->localPublicFlags = 0;
	#endif
	#if defined(CONFIG_RTL_FAST_BRIDGE)
	skb->fast_br_forwarding_flags = 0;
	#endif
	netif_receive_skb(skb);
#else	/*	defined(RX_TASKLET)	*/
	netif_rx(skb);
#endif	/*	defined(RX_TASKLET)	*/
}

__MIPS16
__IRAM_FWD
static inline void rtl_processRxFrame(rtl_nicRx_info *info)
{
	struct dev_priv	*cp_this;
	struct sk_buff 	*skb;
	uint32			vid, pid, len;
	uint8			*data;

	cp_this = info->priv;
	skb = info->input;
	vid = info->vid;
	data = skb->tail = skb->data;

#if defined(CONFIG_RTL_STP)
	if(info->isStpVirtualDev){
		pid = info->pid;
		len = info->len;
		skb->len = 0;
		skb_put(skb, len);
		skb->dev=info->priv->dev;
		goto RxToPs;
	}
#endif
#if 0
	/*	sanity check	*/
	if  ((memcmp(&data[ETH_ALEN], cp_this->dev->dev_addr, ETH_ALEN)==0)||PKTHDR_EXTPORT_MAGIC2==vid||PKTHDR_EXTPORT_MAGIC==vid)// check source mac
	{
		#if	defined(CONFIG_RTL_HARDWARE_NAT)&&(defined(CONFIG_RTL8192SE)||defined(CONFIG_RTL8192CD))
                #ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
		if ((PKTHDR_EXTPORT_MAGIC!=vid)||(info->pid!=PKTHDR_EXTPORT_P3 &&info->pid!=PKTHDR_EXTPORT_P2))
                 #else
		if ((PKTHDR_EXTPORT_MAGIC!=vid)||(info->pid!=PKTHDR_EXTPORT_P3))
		#endif
		#endif
		{
			cp_this->net_stats.rx_dropped++;
			dev_kfree_skb_any(skb);
			return;
		}
	}
#endif
	if (skb->head==NULL||skb->end==NULL)
	{
		dev_kfree_skb_any(skb);
		return;
	}
	/*	sanity check end 	*/

	pid = info->pid;
	len = info->len;
	skb->len = 0;
	skb_put(skb, len);
	skb->dev=info->isPdev?_rtl86xx_dev.pdev:info->priv->dev;
	//skb->dev=cp_this->dev;

#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP
	//merged from r8627 may conflict later
	//printk("type: %x, skb->mac=%p, skb->data=%p\n", skb->mac.ethernet->h_proto, skb->mac.ethernet, skb->data);
	//extern void voip_dsp_L2_pkt_rx(unsigned char* eth_pkt);
	if (*(uint16*)(&(skb->data[12])) == htons(0x8899) && voip_dsp_L2_pkt_rx_trap )
	{
		//dsp_id = *(uint16*)(&(skb->data[14]);
		voip_dsp_L2_pkt_rx_trap(skb->data, skb->len);
		dev_kfree_skb_any(skb);
		//printk("0x%x ", *(uint16*)(&(skb->data[12])));
		return;
	}
#endif



#if defined(CONFIG_NETFILTER_XT_MATCH_PHYPORT) || defined(CONFIG_RTL_FAST_FILTER) || defined(CONFIG_RTL_QOS_PATCH) || defined(CONFIG_RTK_VOIP_QOS) || defined(CONFIG_RTK_VLAN_WAN_TAG_SUPPORT) ||defined(CONFIG_RTL_MAC_FILTER_CARE_INPORT)
	skb->srcPhyPort=(uint8)pid;
#endif
	//printk("=======%s(%d),cp_this(%s)\n",__FUNCTION__,__LINE__,cp_this->dev->name);
	/*	vlan process (including strip vlan tag)	*/
	#if defined(CONFIG_RTK_VLAN_SUPPORT)
	if (rtk_vlan_support_enable && cp_this->vlan_setting.global_vlan)
	{
	#if defined(CONFIG_RTK_VLAN_NEW_FEATURE)
		struct sk_buff *new_skb = NULL;
		int vlan_check;
		vlan_check = rx_vlan_process(cp_this->dev, &cp_this->vlan_setting, skb, &new_skb);
		if((vlan_check==0) && new_skb){
			rtl_processRxToProtcolStack(new_skb, cp_this);
		}else if(vlan_check == 1){
			cp_this->net_stats.rx_dropped++;
			dev_kfree_skb_any(skb);
			return;
		}else if((vlan_check == 2) && new_skb){
			dev_kfree_skb_any(new_skb);
		}
	#else
		if (rx_vlan_process(cp_this->dev, &cp_this->vlan_setting, skb))
		{
			cp_this->net_stats.rx_dropped++;
			dev_kfree_skb_any(skb);
			return;
		}
	#endif

		#if defined(CONFIG_RTK_VLAN_FOR_CABLE_MODEM)
		if(rtk_vlan_support_enable == 2)
		{
			DEBUG_ERR("====%s(%d),cp(%s),cp->vlan.id(%d),skb->tag.vid(%d)\n",__FUNCTION__,__LINE__,
				cp_this->dev->name,cp_this->vlan_setting.id,skb->tag.f.pci&0xfff);
			if(cp_this->vlan_setting.vlan && skb->tag.f.tpid == htons(ETH_P_8021Q) && (skb->tag.f.pci & 0xfff) != cp_this->vlan_setting.id)
			{
				if(cp_this->vlan_setting.is_lan == 0)
				{
					struct net_device* vap;
					/*	look up vap	*/
					vap = get_dev_by_vid(skb->tag.f.pci & 0xfff);
					if(vap)
					{

						skb->dev = vap;
						vap->netdev_ops->ndo_start_xmit(skb,vap);
						return;
					}
				}
			}
		}
		#endif
	}
	#endif	/*	defined(CONFIG_RTK_VLAN_SUPPORT)	*/

	if (*((uint16*)(skb->data+(ETH_ALEN<<1))) == __constant_htons(ETH_P_8021Q)) {
		#if defined(CONFIG_RTL_HW_VLAN_SUPPORT)
		if((skb->data[0]&1)||((skb->data[0]==0x33)&&(skb->data[1]==0x33)&&(skb->data[2]!=0xFF)))
		{
			memcpy(&skb->tag, skb->data+ETH_ALEN*2, sizeof(struct vlan_tag));
		}
		#endif
		vid = *((unsigned short *)(data+(ETH_ALEN<<1)+2));
		#if	defined(CONFIG_RTL_QOS_8021P_SUPPORT)
		skb->srcVlanPriority = (vid>>13)&0x7;
		#endif
		vid &= 0x0fff;

	#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
		//for later arp reply to WAN port 
		//memcpy(&(skb->tag), skb->data+ETH_ALEN*2, VLAN_HLEN);
	
		if(vlan_tag && vid == vlan_tag)
		{
			goto Remove_tag;
		}
		else if(vlan_bridge_tag && vid == vlan_bridge_tag)
		{
			if(vlan_bridge_multicast_tag)
			{ 
				if((skb->data[0]==0x01) && (skb->data[1]==0x00) && (skb->data[2]==0x5e))
				{
					cp_this->net_stats.rx_dropped++;
					dev_kfree_skb_any(skb);
					return;
				}
			}
			goto Remove_tag;
		}
		else if(vlan_bridge_multicast_tag && vid == vlan_bridge_multicast_tag)
		{
			if((skb->data[0]==0x01) && (skb->data[1]==0x00) && (skb->data[2]==0x5e))
			{
				info->vid = vlan_bridge_tag;//for later check ???
				goto Remove_tag;
			}
		}
		else
		{
		#ifndef CONFIG_VLAN_8021Q
			cp_this->net_stats.rx_dropped++;
			dev_kfree_skb_any(skb);
			return; 
		#endif
		}
	#ifdef CONFIG_VLAN_8021Q
		goto Reserve_tag;
	#endif
Remove_tag:
	#endif
		memmove(data + VLAN_HLEN, data, VLAN_ETH_ALEN<<1);
		skb_pull(skb, VLAN_HLEN);

#ifdef CONFIG_VLAN_8021Q
Reserve_tag:
	;//don't remove tag
#endif
	
	}
	/*	vlan process end (vlan tag has already stripped)	*/

	/*	update statistics	*/
	#if !defined(CONFIG_RTL_NIC_HWSTATS)
	cp_this->net_stats.rx_packets++;
	cp_this->net_stats.rx_bytes += skb->len;
	#endif
	cp_this->dev->last_rx = jiffies;
	/*	update statistics end	*/

	if (0==(data[0]&0x01))		/*	unicast pkt only	*/
	{
		/*	shortcut process	*/
		#if defined(BR_SHORTCUT)
		if (SUCCESS==rtl_processBridgeShortCut(skb,cp_this, info)) {
			return;
		}
		#elif defined(CONFIG_RTL_HARDWARE_NAT)&&(defined(CONFIG_RTL8192SE)||defined(CONFIG_RTL8192CD))
		/*	
		if (SUCCESS==rtl_processExtensionPortShortCut(skb,cp_this, info)) {
			return;
		}
		*/
		#endif

		#if defined(CONFIG_RTL_FASTBRIDGE)
		if (RTL_FB_RETURN_SUCCESS==rtl_fb_process_in_nic(skb, cp_this->dev))
			return;
		#endif
		/*	shortcut process end	*/

		/*	unknow unicast control	*/
		#if defined (CONFIG_RTL_UNKOWN_UNICAST_CONTROL)
		rtl_unkownUnicastUpdate(skb->data);
		#endif
		/*	unknow unicast control end	*/
	}
	else		/*	multicast		*/
	{
		/*	multicast process	*/
		#if defined (CONFIG_RTL_IGMP_SNOOPING)
		//rtl_MulticastRxCheck(skb, cp_this, vid, pid);
		rtl_MulticastRxCheck(skb, info);
		#endif	/*end of CONFIG_RTL865X_IGMP_SNOOPING*/
		/*	multicast process end	*/
	}

#if defined(CONFIG_RTL_STP)
RxToPs:
#endif
	/*	finally successfuly rx to protocol stack	*/
	rtl_processRxToProtcolStack(skb, cp_this);
}

#if defined (CONFIG_RTK_VOIP_QOS)
int ( *check_voip_channel_loading )( void );
#endif

__DRAM_FWD rtlInterruptRxData	RxIntData;

#if defined (CONFIG_RTK_VOIP_QOS) && !defined (CONFIG_RTK_VOIP_ETHERNET_DSP_IS_HOST)
__IRAM_FWD
static inline int32 interrupt_dsr_rx_per_packe_check(rtlInterruptRxData *rxData, unsigned long task_priv)
{
	if (( (rxData->voip_rx_cnt++ > 100) || ((jiffies - rxData->voip_rx_start_time) >= 1 ))&& (check_voip_channel_loading && (check_voip_channel_loading() > 0)))
	{
	#ifdef RX_TASKLET
		// avoid cp  change!
		tasklet_schedule(&(( struct dev_priv *)task_priv)->rx_dsr_tasklet);
	#endif
		return RTL_RX_PROCESS_RETURN_BREAK;
	}
	return RTL_RX_PROCESS_RETURN_SUCCESS;
}
#endif

#if defined (CONFIG_RTK_VOIP_QOS) && !defined (CONFIG_RTK_VOIP_ETHERNET_DSP_IS_HOST)
static inline int32 interrupt_dsr_rx_check(rtlInterruptRxData *rxData)
{
	rxData->voip_rx_start_time = jiffies;
	rxData->voip_rx_cnt = 0;
	return RTL_RX_PROCESS_RETURN_SUCCESS;
}
#endif

__IRAM_FWD static void interrupt_dsr_rx_done(rtlInterruptRxData *rxData)
{
#ifdef RX_TASKLET
	unsigned long flags;

	local_irq_save(flags);
	rtl_rxSetTxDone(TRUE);
    	REG32(CPUIIMR) |= (RX_DONE_IE_ALL | PKTHDR_DESC_RUNOUT_IE_ALL);
	rtl_rx_tasklet_running = 0;
	local_irq_restore(flags);
#endif
}

__MIPS16
__IRAM_FWD
static int32 interrupt_dsr_rx(unsigned long task_priv)
{
	static __DRAM_FWD rtl_nicRx_info	info;
	int	ret, count;

	#if defined (CONFIG_RTK_VOIP_QOS) && !defined (CONFIG_RTK_VOIP_ETHERNET_DSP_IS_HOST)
	interrupt_dsr_rx_check(&RxIntData);
	#endif
	while (1)
	{
		#if defined (CONFIG_RTK_VOIP_QOS) && !defined (CONFIG_RTK_VOIP_ETHERNET_DSP_IS_HOST)
		if (RTL_RX_PROCESS_RETURN_BREAK==interrupt_dsr_rx_per_packe_check(&RxIntData, task_priv))
			break;
		#endif

		#if defined(RTL_MULTIPLE_RX_TX_RING)
		info.priority = RTL_ASSIGN_RX_PRIORITY;
		#endif

		count = 0;
		do {
			ret = swNic_receive(&info, count++);
		} while (ret==RTL_NICRX_REPEAT);

		switch(rtl_processReceivedInfo(&info,  ret)) {
			case RTL_RX_PROCESS_RETURN_SUCCESS:
				if (SUCCESS==rtl_decideRxDevice(&info)) {
					rtl_processRxFrame(&info);
				}
				break;
			case RTL_RX_PROCESS_RETURN_BREAK:
				{goto rx_out;}
			default:
				break;
		}
	}

rx_out:
	interrupt_dsr_rx_done(&RxIntData);

	return RTL_NICRX_OK;
}


__IRAM_FWD
static void interrupt_dsr_tx(unsigned long task_priv)
{
	int32	idx;
	#ifdef TX_TASKLET
	unsigned long flags;
	#endif

	for(idx=RTL865X_SWNIC_TXRING_MAX_PKTDESC-1;idx>=0;idx--)
	{
		swNic_txDone(idx);
	}

	refill_rx_skb();
	#ifdef TX_TASKLET
	local_irq_save(flags);
	rtl_rxSetTxDone(TRUE);
	rtl_tx_tasklet_running = 0;
	local_irq_restore(flags);
	#endif
}

#ifdef CONFIG_RTL8196C_ETH_IOT
static int re865x_setPhyGrayCode(void)
{
    uint32 agc, cb0, snr, new_port_link_sts=0, val;
    uint32 DUT_eee, Linkpartner_eee;
    int i;

    /************ Link down check ************/
    for(i=0; i<RTL8651_PHY_NUMBER; i++) {
		if ((REG32(PSRP0 + (i * 4)) & LinkDownEventFlag) != 0){	// !!! LinkDownEventFlag is a read clear bit, so these code must ahead of "Link up check"
				if ((REG32(PSRP0 + (i * 4)) & PortStatusLinkUp) == 0) {
					/*=========== ###01 ===========*/
					extern void set_gray_code_by_port(int);
					set_gray_code_by_port(i);

					/*=========== ###03 ===========*/
					// read DUT eee 100 ability
					rtl8651_setAsicEthernetPHYReg( i, 13, 0x7 );
					rtl8651_setAsicEthernetPHYReg( i, 14, 0x3c );
					rtl8651_setAsicEthernetPHYReg( i, 13, 0x4007 );
					rtl8651_getAsicEthernetPHYReg( i, 14, &DUT_eee );

					// due to the RJ45 cable is plug out now, we can't read the eee 100 ability of link partner.
					// we use the variable port_linkpartner_eee to keep link partner's eee 100 ability after RJ45 cable is plug in.
					if (  ( ((DUT_eee & 0x2) ) == 0)  || ((port_linkpartner_eee & (1<<i)) == 0) )  {
						rtl8651_getAsicEthernetPHYReg( i, 21, &val );
						val = val & ~(0x4000);
						rtl8651_setAsicEthernetPHYReg( i, 21, val );
					}
				}
			}
	}

	/************ Link up check ************/
	for(i=0; i<RTL8651_PHY_NUMBER; i++) {
		if ((REG32(PSRP0 + (i * 4)) & PortStatusLinkUp) != 0) {

			if ((port_link_sts & (1<<i)) == 0) {	// the port which already linked up does not need to check ...

				/*=========== ###03 ===========*/
				// read DUT eee 100 ability
				rtl8651_setAsicEthernetPHYReg( i, 13, 0x7 );
				rtl8651_setAsicEthernetPHYReg( i, 14, 0x3c );
				rtl8651_setAsicEthernetPHYReg( i, 13, 0x4007 );
				rtl8651_getAsicEthernetPHYReg( i, 14, &DUT_eee );

				// read Link partner eee 100 ability
				rtl8651_setAsicEthernetPHYReg( i, 13, 0x7 );
				rtl8651_setAsicEthernetPHYReg( i, 14, 0x3d );
				rtl8651_setAsicEthernetPHYReg( i, 13, 0x4007 );
				rtl8651_getAsicEthernetPHYReg( i, 14, &Linkpartner_eee );

				if (  ( ((DUT_eee & 0x2) ) != 0)  && ( ((Linkpartner_eee & 0x2) ) != 0) )  {
					rtl8651_getAsicEthernetPHYReg( i, 21, &snr );
					snr = snr | 0x4000;
					rtl8651_setAsicEthernetPHYReg( i, 21, snr );
				}

				if ( ((Linkpartner_eee & 0x2) ) != 0)
					port_linkpartner_eee |= (1 << i);
				else
					port_linkpartner_eee &= ~(1 << i);

				/*=========== ###02 ===========*/
				/*
				  1.      reg17 = 0x1f10，read reg29, for SNR
				  2.      reg17 =  0x1f11，read reg29, for AGC
				  3.      reg17 = 0x1f18，read reg29, for cb0
				 */
				// 1. for SNR
				snr = 0;
				for(val=0; val<3; val++) {
					rtl8651_getAsicEthernetPHYReg(i, 29, &agc);
					snr += agc;
				}
				snr = snr / 3;

				// 3. for cb0
				rtl8651_getAsicEthernetPHYReg( i, 17, &val );
				val = (val & 0xfff0) | 0x8;
				rtl8651_setAsicEthernetPHYReg( i, 17, val );
				rtl8651_getAsicEthernetPHYReg( i, 29, &cb0 );

				// 2. for AGC
				val = (val & 0xfff0) | 0x1;
				rtl8651_setAsicEthernetPHYReg( i, 17, val );
				rtl8651_getAsicEthernetPHYReg( i, 29, &agc );

				// set bit 3~0 to 0x0 for reg 17
				val = val & 0xfff0;
				rtl8651_setAsicEthernetPHYReg( i, 17, val );

				if ( ( (    ((agc & 0x70) >> 4) < 4    ) && ((cb0 & 0x80) != 0) ) || (snr > 4150) ) { // "> 4150" = "< 18dB"
					rtl8651_restartAsicEthernetPHYNway(i);
				}

			}

			new_port_link_sts = new_port_link_sts | (1 << i);
		}
	}

	port_link_sts = new_port_link_sts;

	return SUCCESS;
}
#endif

#if defined (CONFIG_RTL_PHY_PATCH)
#define SNR_THRESHOLD 1000

//db = -(10 * log10(sum/262144));
//18db:SNR_THRESHOLD=4155
//20db:SNR_THRESHOLD=2621
//21db:SNR_THRESHOLD=2082
//22db:SNR_THRESHOLD=1654
//24.18db:SNR_THRESHOLD=1000
#define MAX_RESTART_NWAY_INTERVAL		(60*HZ)
#define MAX_RESTART_NWAY_CNT		3

struct rtl_nWayCtrl_s
{
	unsigned long restartNWayTime;
	unsigned long restartNWayCnt;
};

struct rtl_nWayCtrl_s re865x_restartNWayCtrl[RTL8651_PHY_NUMBER];

unsigned int re865x_getPhySnr(unsigned int port)
{
	unsigned int sum=0;
	unsigned int j;
	unsigned int regData;

	if(port >= RTL8651_PHY_NUMBER)
	{
		return -1;
	}

	if (REG32(PSRP0 + (port * 4)) & PortStatusLinkUp)
	{
		for (j=0, sum=0;j<10;j++)
		{
			rtl8651_getAsicEthernetPHYReg(port, 29, &regData);
			sum += regData;
			mdelay(10);
		}
		sum /= 10;
		return sum;
	}

	return 0;
}

#if defined(CONFIG_RTL_8196C)
#if 1
static int re865x_checkPhySnr(void)
{

	unsigned int port;
	unsigned int snr=0;
	unsigned int  val, cb0, agc;
	for (port=0; port<RTL8651_PHY_NUMBER; port++)
	{
		if((1<<port) & (newLinkPortMask & (~curLinkPortMask)) )
		{
			snr=re865x_getPhySnr(port);

			// 3. for cb0
                        rtl8651_getAsicEthernetPHYReg( port, 17, &val );
                        val = (val & 0xfff0) | 0x8;
                        rtl8651_setAsicEthernetPHYReg( port, 17, val );
                        rtl8651_getAsicEthernetPHYReg( port, 29, &cb0 );

			  // 2. for AGC
                        val = (val & 0xfff0) | 0x1;
                        rtl8651_setAsicEthernetPHYReg( port, 17, val );
                        rtl8651_getAsicEthernetPHYReg(port, 29, &agc );
			//printk("snr is %d\n",snr);
			//printk("cb0 is 0x%x,agc is 0x%x\n",cb0,agc);
			//if( ((cb0 & 0x80) != 0)|| (snr>4155))
		  	if ( ( ( ((agc & 0x70) >> 4) < 4    ) && ((cb0 & 0x80) != 0) ) || (snr > 4155) )
			{
				//printk("restart nway\n");
				rtl8651_restartAsicEthernetPHYNway(port);
			}
			val = val & 0xfff0;
			rtl8651_setAsicEthernetPHYReg( port, 17, val );

		}

	}
	return 0;

}
#else
static int re865x_checkPhySnr(void)
{

	unsigned int port;
	unsigned int snr=0;

	for (port=0; port<RTL8651_PHY_NUMBER; port++)
	{
		if((1<<port) & (newLinkPortMask & (~curLinkPortMask)))
		{
			snr=re865x_getPhySnr(port);
			//printk("%s:%d, port is %d, snr is %d\n",__FUNCTION__,__LINE__,port,snr);
			if(snr>SNR_THRESHOLD)
			{
				/*poor snr, we should restart n-way*/
				if(re865x_restartNWayCtrl[port].restartNWayTime==0)
				{
					/*last time snr is good*/
					re865x_restartNWayCtrl[port].restartNWayTime=jiffies;
					re865x_restartNWayCtrl[port].restartNWayCnt=1;

					rtl8651_restartAsicEthernetPHYNway(port);

				}
				else if(time_after(jiffies,re865x_restartNWayCtrl[port].restartNWayTime+MAX_RESTART_NWAY_INTERVAL) )
				{
					/*last time SNR is bad, but interval is long enough, we can take another restart n-way action*/
					re865x_restartNWayCtrl[port].restartNWayTime=jiffies;
					re865x_restartNWayCtrl[port].restartNWayCnt=1;
					rtl8651_restartAsicEthernetPHYNway(port);

				}
				else if (re865x_restartNWayCtrl[port].restartNWayCnt>MAX_RESTART_NWAY_CNT)
				{
					/*within restart n-way interval and exceed max try cnt*/
					/*shouldn't do it any more, otherwise it will cause phy link up/down repeatly*/
					//printk("%s:%d,restartNWayCnt>MAX_RESTART_NWAY_CNT,stop do restart n-way\n",__FUNCTION__,__LINE__);
				}
				else
				{
					//printk("%s:%d,restartNWayCnt is %lu\n",__FUNCTION__,__LINE__,re865x_restartNWayCtrl[port].restartNWayCnt);
					re865x_restartNWayCtrl[port].restartNWayCnt++;
					rtl8651_restartAsicEthernetPHYNway(port);
				}
			}
			else
			{
				re865x_restartNWayCtrl[port].restartNWayTime=0;
				re865x_restartNWayCtrl[port].restartNWayCnt=0;
			}

		}

	}
	return 0;

}
#endif
#endif
#endif

unsigned int rtl865x_getPhysicalPortLinkStatus(void)
{
	unsigned int port_num=0;
	unsigned int linkPortMask=0;
	for(port_num=0;port_num<=RTL8651_PHY_NUMBER;port_num++)
	{
		if((READ_MEM32(PSRP0+(port_num<<2))&PortStatusLinkUp)!=0)
		{
			linkPortMask |= 1<<port_num;
		}

	}
	return linkPortMask;
}

#if defined(CONFIG_RTL_REPORT_LINK_STATUS)
unsigned int rtl865x_setLinkStatusFlag(unsigned int curLinkPortMask, unsigned int newportmask)
{
	int i;
	struct dev_priv * cp=NULL;
	if(rtl865x_curOpMode == GATEWAY_MODE)
	{
		for(i = 0; i < ETH_INTF_NUM; i++)
		{
			cp = ((struct dev_priv *)_rtl86xx_dev.dev[i]->priv);
			if(cp &&  cp->opened && rtl_isWanDev(cp) && ((cp->portmask & curLinkPortMask)==0) && (cp->portmask & newportmask))
			{
				wan_linkStatus[0] = 1;
				return SUCCESS;
			}
		}
	}
	return 0;
}
#endif


#ifdef CONFIG_RTL_819X_SWCORE
int cnt_swcore = 0;
int cnt_swcore_tx = 0;
int cnt_swcore_rx = 0;
int cnt_swcore_link = 0;
int cnt_swcore_err = 0;
#endif


#ifdef CONFIG_RTL_8198_ESD
static uint32 phy_reg30[RTL8651_PHY_NUMBER] = { 0, 0, 0, 0, 0};
static int one_second_counter = 0;
static int first_time_read_reg6 = 1;
static int need_to_check_esd2 = 1;
static int esd3_skip_one = 1;

inline static int diff_more_than_1(uint32 a, uint32 b)
{
	uint32 c;

	if (a==b)
		return 0;
	if (a<b)
		{ c=a; a=b; b=c; }
	if ((a-b) >=2)
		return 1;
	else
		return 0;
}

static int esd_recovery(void)
{
	uint32 val;
	int i;

	for (i=0; i<RTL8651_PHY_NUMBER; i++) {
		/************ Link down  ************/
		if ((REG32(PSRP0 + (i * 4)) & PortStatusLinkUp) == 0) {
			phy_reg30[i] = 0;
		}
		/************ Link up  ************/
		else if (phy_reg30[i] == 0) {
			rtl8651_getAsicEthernetPHYReg( i, 22, &val );
			rtl8651_setAsicEthernetPHYReg( i, 22, ((val & (0xff00)) | 0x17) );
			rtl8651_getAsicEthernetPHYReg( i, 30, &val );
			phy_reg30[i] = BIT(31) | (val & 0xfff);
		}
	}
	return SUCCESS;
}
#endif

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
/*
krammer add this according to " Jim Hsieh/5458 "'s algorithm
Description:
when phy0's link partner link in force mode, we will force to half duplex mode,
in this condition, if there is some traffic in port0's rx, sometimes port0 will die.
so we add this patch, only work when phy0's link state change, if link partner
is link in force mode, we will change our port0 into full duplex.
*/
void rtl819x_port0_force_refined(void){
	unsigned int regData;

	if((curLinkPortMask & 0x01) == (newLinkPortMask & 0x01)){
		//no change -> return
		return;
	}

	//default disable phy i/f(0xbb804104 b0=0)
	REG32(PCRP0) &= ~(EnablePHYIf);

	//if port#0=link up
	if(newLinkPortMask & 0x01){
		//if AN =0(reg6[0])
		rtl8651_getAsicEthernetPHYReg(rtl8651_tblAsicDrvPara.externalPHYId[0], 6, &regData);
		if((regData & 0x01) == 0){
			rtl8651_getAsicEthernetPHYReg(rtl8651_tblAsicDrvPara.externalPHYId[0], 5, &regData);
			if(regData & 0x080){//if spd = 100M(reg5[7])
				//force MAC 100M Full duplex
				//(set 0xbb804104 b25=1 b24=1 b23=0 b20-19=1 b18=1 b17-16=0x3)
				regData = REG32(PCRP0);
				regData |= (EnForceMode | PollLinkStatus | ForceDuplex | PauseFlowControlEtxErx);
				regData &= ~(ForceLink | ForceSpeedMask);
				regData |= (ForceSpeed100M);
				//panic_printk("phy0 link partner is force 100M, we force to 100M Full duplex!regData = 0x%x\n", regData);
				REG32(PCRP0) = regData;
			}
			else if(regData & 0x20){//else spd = 10M(reg5[5])
				//force MAC 10M Full duplex
				//(set 0xbb804104 b25=1 b24=1 b23=0 b20-19=0 b18=1 b17-16=0x3)
				regData = REG32(PCRP0);
				regData |= (EnForceMode | PollLinkStatus | ForceDuplex | PauseFlowControlEtxErx);
				regData &= ~(ForceLink | ForceSpeedMask);
				regData |= (ForceSpeed10M);
				//panic_printk("phy0 link partner is force 10M, we force to 10M Full duplex!regData = 0x%x\n", regData);
				REG32(PCRP0) = regData;
			}
		}
		//enable phy i/f
		//set 0xbb804104 b0=1
		REG32(PCRP0) |= EnablePHYIf;
		//panic_printk("phy0 link up, PCRP0 = 0x%x\n", REG32(PCRP0));
	}
	else{//if port#0=link down
		//recovery MAC AN setting and disable phy i/f
		//(set 0xbb804104 b25=0 b20-18 0x7 b0=0)
		regData = REG32(PCRP0);
		regData &= ~(EnForceMode | EnablePHYIf);
		regData |= (ForceSpeed1000M | ForceDuplex);
		//panic_printk("phy0 link down, we reset phy0!regData = 0x%x\n", regData);
		REG32(PCRP0) = regData;
	}
}
#endif

static void interrupt_dsr_link(unsigned long task_priv)
{
#ifdef CONFIG_RTL_8198_ESD
	esd_recovery();
#endif

#ifdef CONFIG_RTL8196C_ETH_IOT
/*LinkDownEventFlag is a read clear bit, so re865x_setPhyGrayCode() should be call headmost */
		re865x_setPhyGrayCode();
#endif

	newLinkPortMask=rtl865x_getPhysicalPortLinkStatus();

#ifdef CONFIG_RTL_8197D_DYN_THR
	rtl819x_setQosThreshold(curLinkPortMask, newLinkPortMask);
#endif

#if defined(CONFIG_RTL_REPORT_LINK_STATUS)
//		rtl865x_setLinkStatusFlag(newLinkPortMask);
		rtl865x_setLinkStatusFlag(curLinkPortMask, newLinkPortMask);
#endif
#if defined(CONFIG_RTL_IGMP_SNOOPING)
		rtl865x_igmpSyncLinkStatus();
#endif

#if defined(CONFIG_RTL_LINKCHG_PROCESS)
		rtl865x_LinkChange_Process();
#endif

#if defined(CONFIG_RTL_8196C) && defined(CONFIG_RTL_PHY_PATCH)
		re865x_checkPhySnr();
#endif

#if defined(CONFIG_RTL_819XD)
	if (rtl_port0Refined == 1)
	{
		rtl819x_port0_force_refined();
	}
#endif

	curLinkPortMask=newLinkPortMask;

#ifdef LINK_TASKLET
	REG32(CPUIIMR) |= (LINK_CHANGE_IP);
#endif

	return;
}


__IRAM_GEN static inline void rtl_rx_interrupt_process(unsigned int status, struct dev_priv *cp)
{
	#if defined(CONFIG_RTL_REINIT_SWITCH_CORE)
	if(rtl865x_duringReInitSwtichCore==1) {
		return;
	}
	#endif

	//if (status & (RX_DONE_IP_ALL | PKTHDR_DESC_RUNOUT_IP_ALL))
	{
		#if defined(RX_TASKLET)
		if (rtl_rx_tasklet_running==0) {
			rtl_rx_tasklet_running=1;
			REG32(CPUIIMR) &= ~(RX_DONE_IE_ALL | PKTHDR_DESC_RUNOUT_IE_ALL);
			rtl_rxSetTxDone(FALSE);
			tasklet_hi_schedule(&cp->rx_dsr_tasklet);
		}
		#else
		interrupt_dsr_rx((unsigned long)cp);
		#endif
	}
}

__IRAM_GEN static inline void rtl_tx_interrupt_process(unsigned int status, struct dev_priv *cp)
{
	if (status & TX_ALL_DONE_IP_ALL)
	{
  		#if defined(TX_TASKLET)
		if (rtl_tx_tasklet_running==0) {
			rtl_tx_tasklet_running=1;
			rtl_rxSetTxDone(FALSE);
			tasklet_hi_schedule(&cp->tx_dsr_tasklet);
		}
  		#else
		interrupt_dsr_tx((unsigned long)cp);
 		#endif
	}
}

#if	defined(CONFIG_RTL_IGMP_SNOOPING)||defined(CONFIG_RTL_LINKCHG_PROCESS) || defined (CONFIG_RTL_PHY_PATCH)
__IRAM_GEN static inline void rtl_link_change_interrupt_process(unsigned int status, struct dev_priv *cp)
{
	if (status & LINK_CHANGE_IP)
	{
  		#if defined(LINK_TASKLET)
			#if defined(CONFIG_RTK_VOIP_BOARD)
	  		REG32(CPUIIMR) &= ~LINK_CHANGE_IP;
			//tasklet_schedule(&cp->link_dsr_tasklet);
			REG32(CPUIIMR) |= (LINK_CHANGE_IP);
			#else
	  		REG32(CPUIIMR) &= ~LINK_CHANGE_IP;
			tasklet_schedule(&cp->link_dsr_tasklet);
			#endif //CONFIG_RTK_VOIP_BOARD
	  	#else
			interrupt_dsr_link((unsigned long)cp);
  		#endif
  		#ifdef CONFIG_RTK_VOIP_PORT_LINK
		if(cp->opened)
			rtmsg_ifinfo_voip(RTM_LINKCHANGE, cp->dev, ~0U);
		#endif
	}
}
#endif

__MIPS16
__IRAM_FWD
irqreturn_t interrupt_isr(int irq, void *dev_instance)
{
	struct net_device *dev = dev_instance;
	struct dev_priv *cp;
	unsigned int status;
	cp = dev->priv;
	status = REG32(CPUIISR);
	REG32(CPUIISR) = status;
	status &= REG32(CPUIIMR);

#ifdef CONFIG_RTL_819X_SWCORE
    cnt_swcore++;
	if (status & (RX_DONE_IP_ALL))
 		cnt_swcore_rx++;
	if (status & TX_ALL_DONE_IP_ALL)
		cnt_swcore_tx++;
	if (status&LINK_CHANGE_IP)
		cnt_swcore_link++;
    if (status&(RX_DONE_IP_ALL|TX_ALL_DONE_IP_ALL|LINK_CHANGE_IP))
        cnt_swcore_err++;
#endif

	rtl_rx_interrupt_process(status, cp);

	rtl_tx_interrupt_process(status, cp);

	#if	defined(CONFIG_RTL_IGMP_SNOOPING)||defined(CONFIG_RTL_LINKCHG_PROCESS) || defined (CONFIG_RTL_PHY_PATCH)
	if (status & LINK_CHANGE_IP)
	rtl_link_change_interrupt_process(status, cp);
	#endif

	return IRQ_HANDLED;
}

static int rtl865x_init_hw(void)
{
	unsigned int mbufRingSize;
	int	i;

	mbufRingSize = rtl865x_rxSkbPktHdrDescNum;	/* rx ring 0	*/
	for(i=1;i<RTL865X_SWNIC_RXRING_HW_PKTDESC;i++)
	{
		mbufRingSize += rxRingSize[i];
	}

	/* Initialize NIC module */
	if (swNic_init(rxRingSize, mbufRingSize, txRingSize, MBUF_LEN))
	{
		printk("865x-nic: swNic_init failed!\n");
		return FAILED;
	}

	return SUCCESS;
}

#ifdef CONFIG_RTL_HARDWARE_NAT
static void reset_hw_mib_counter(struct net_device *dev)
{
	int i, port;
	int32 totalVlans=((sizeof(vlanconfig))/(sizeof(struct rtl865x_vlanConfig)))-1;

	for(i=0;i<totalVlans;i++)
	{
		if (IF_ETHER!=vlanconfig[i].if_type)
		{
			continue;
		}
		if (!memcmp(vlanconfig[i].mac.octet, dev->dev_addr, 6))
		{
			for (port=0; port<RTL8651_AGGREGATOR_NUMBER; port++)
			{
				if (vlanconfig[i].memPort & (1<<port))
				   WRITE_MEM32(MIB_CONTROL, (1<<port*2) | (1<<(port*2+1)));
				return;
			}
		}
	}
}
#endif

#if defined (CONFIG_RTL_8197D)
extern void rtl819x_poll_sw(void);
#endif

#ifdef CONFIG_RTL8196C_GREEN_ETHERNET
int total_time_for_5_port = 15*HZ;
int port_pwr_save_low = 0;
#endif

#ifdef CONFIG_RTL_8196E
void	refine_phy_setting(void)
{
	int i, start_port = 0;
	uint32 val;

	val = (REG32(BOND_OPTION) & BOND_ID_MASK);
	if (val == BOND_8196ES)
		return;
	
	if (val == BOND_8196EU)
		start_port = 4;
	
	for (i=start_port; i<5; i++) {
		rtl8651_setAsicEthernetPHYReg( i, 25, 0x6964);
		rtl8651_getAsicEthernetPHYReg(i, 26, &val);
		rtl8651_setAsicEthernetPHYReg(i, 26, ((val & (0xff00)) | 0x9E) );

		rtl8651_getAsicEthernetPHYReg(i, 17, &val);
		rtl8651_setAsicEthernetPHYReg( i, 17, ((val & (0xfff0)) | 0x8)  );

		rtl8651_getAsicEthernetPHYReg( i, 29, &val );
		if ((val & 0x8080) == 0x8080) {
			rtl8651_getAsicEthernetPHYReg( i, 21, &val );
			rtl8651_setAsicEthernetPHYReg( i, 21, (val  | 0x8000) );
			rtl8651_setAsicEthernetPHYReg( i, 21, (val & ~0x8000) );
		}
	}	
	return;
}
#endif

#if defined(DYNAMIC_ADJUST_TASKLET) || defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL8196C_REVISION_B) || defined(CONFIG_RTL_8198) || defined(RTL8196C_EEE_MAC) || defined(RTL_CPU_QOS_ENABLED) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
static void one_sec_timer(unsigned long task_priv)
{
	unsigned long		flags;
	struct dev_priv	*cp;
#if defined (CONFIG_RTL_REINIT_SWITCH_CORE)
	int i;
#endif

	cp = ((struct net_device *)task_priv)->priv;

	//spin_lock_irqsave (&cp->lock, flags);
	local_irq_save(flags);

#if defined(PATCH_GPIO_FOR_LED)
	if (((struct net_device *)task_priv)->name[3] == '0' ) {	// eth0
		int port;

		for (port=0; port<RTL8651_PHY_NUMBER; port++) {
			update_mib_counter(port);
			calculate_led_interval(port);
			if (led_cb[port].link_status & GPIO_LINK_STATE_CHANGE)
			{
				gpio_led_Interval_timeout(port);
			}
		}
	}
#endif

#if defined(DYNAMIC_ADJUST_TASKLET)
    if (((struct net_device *)task_priv)->name[3] == '0' && rx_pkt_thres > 0 &&
									((eth_flag&TASKLET_MASK) == 0))  {
        if (rx_cnt > rx_pkt_thres) {
			if (!rx_tasklet_enabled) {
				rx_tasklet_enabled = 1;
            }
        }
        else {
			if (rx_tasklet_enabled) { // tasklet enabled
				rx_tasklet_enabled = 0;
			}
        }
        rx_cnt = 0;
    }
#endif

#ifdef CONFIG_RTL8186_TR
    cp->tx_avarage = (cp->tx_avarage/10)*7 + (cp->tx_byte_cnt/10)*3;
    if (cp->tx_avarage > cp->tx_peak)
        cp->tx_peak = cp->tx_avarage;
    cp->tx_byte_cnt = 0;

    cp->rx_avarage = (cp->rx_avarage/10)*7 + (cp->rx_byte_cnt/10)*3;
    if (cp->rx_avarage > cp->rx_peak)
        cp->rx_peak = cp->rx_avarage;
    cp->rx_byte_cnt = 0;
#endif

#ifdef CONFIG_RTL_8197D_DYN_THR
	if (_8197d_link_check == 0) {
		newLinkPortMask=rtl865x_getPhysicalPortLinkStatus();

		rtl819x_setQosThreshold(curLinkPortMask, newLinkPortMask);

		curLinkPortMask=newLinkPortMask;
		_8197d_link_check = 1;
	}
#endif

#ifdef CONFIG_RTL8196C_REVISION_B
	if ((REG32(REVR) == RTL8196C_REVISION_A) && (eee_enabled)) {
			int i, curr_sts;
			uint32 reg;

			/*
				prev_port_sts[] = 0, current = 0	:	do nothing
				prev_port_sts[] = 0, current = 1	:	update prev_port_sts[]
				prev_port_sts[] = 1, current = 0	:	update prev_port_sts[], disable EEE
				prev_port_sts[] = 1, current = 1	:	enable EEE if EEE is disabled
			 */
			for (i=0; i<MAX_PORT_NUMBER; i++)
			{
				curr_sts = (REG32(PSRP0 + (i * 4)) & PortStatusLinkUp) >> 4;

				if ((prev_port_sts[i] == 1) && (curr_sts == 0)) {
					// disable EEE MAC
					REG32(EEECR) = (REG32(EEECR) & ~(0x1f << (i * 5))) |
						((EN_P0_FRC_EEE|FRC_P0_EEE_100) << (i * 5));
					//printk("	disable EEE for port %d\n", i);
				}
				else if ((prev_port_sts[i] == 1) && (curr_sts == 1)) {
					reg = REG32(EEECR);
					if ((reg & (1 << (i * 5))) == 0) {
						// enable EEE MAC
						REG32(EEECR) = (reg & ~(0x1f << (i * 5))) |
							((FRC_P0_EEE_100|EN_P0_TX_EEE|EN_P0_RX_EEE) << (i * 5));
						//printk("	enable EEE for port %d\n", i);
					}
				}
				prev_port_sts[i] = curr_sts;
			}
			//printk(" %d %d %d %d %d\n", port_sts[0], port_sts[1], port_sts[2], port_sts[3], port_sts[4]);
		}
#endif

#if defined(RTL_CPU_QOS_ENABLED)
	totalLowQueueCnt = 0;
#endif

#ifdef CONFIG_RTL_8198_ESD
#if defined(CONFIG_PRINTK)
#define panic_printk         printk
#endif
	{
	int phy;
	uint32 val;
	if (first_time_read_reg6) {
		first_time_read_reg6 = 0;

		for (phy=0; phy<5; phy++) {
			rtl8651_setAsicEthernetPHYReg( phy, 31, 5  );
			rtl8651_getAsicEthernetPHYReg(phy, 1, &val);
			rtl8651_setAsicEthernetPHYReg(phy, 1, val | 0x4);

			rtl8651_setAsicEthernetPHYReg(phy, 5, 0xfff6);
			rtl8651_getAsicEthernetPHYReg(phy, 6, &val);
			rtl8651_setAsicEthernetPHYReg( phy, 31, 0  );

			if ((val & 0xff) == 0xFF) {
				need_to_check_esd2 = 0;
			}
			if (phy_reg30[phy] != 0) {
				rtl8651_getAsicEthernetPHYReg( phy, 22, &val );
				rtl8651_setAsicEthernetPHYReg( phy, 22, ((val & (0xff00)) | 0x17) );
				rtl8651_getAsicEthernetPHYReg( phy, 30, &val );

				phy_reg30[phy] = BIT(31) | (val & 0xfff);
			}
		}
	}

	if (++one_second_counter >= 10) {
		for (phy=0; phy<5; phy++)
		{
			if (phy_reg30[phy] != 0) {
				rtl8651_setAsicEthernetPHYReg( phy, 31, 5  );
				rtl8651_setAsicEthernetPHYReg(phy, 5, 0);

				rtl8651_getAsicEthernetPHYReg(phy, 6, &val);
				rtl8651_setAsicEthernetPHYReg( phy, 31, 0  );

				if ((val & 0xffff) != 0xAE04)
				{
					panic_printk("  ESD-1\n");
					//do {} while(1); // reboot
					machine_restart(NULL);
				}

				if (need_to_check_esd2) {
					rtl8651_setAsicEthernetPHYReg( phy, 31, 5  );
					rtl8651_setAsicEthernetPHYReg(phy, 5, 0xfff6);

					rtl8651_getAsicEthernetPHYReg(phy, 6, &val);
					rtl8651_setAsicEthernetPHYReg( phy, 31, 0  );

					if ((val & 0xff) != 0xFC)
					{
						panic_printk("  ESD-2\n");
						//do {} while(1); // reboot
						machine_restart(NULL);
					}
				}

				rtl8651_getAsicEthernetPHYReg( phy, 22, &val );
				rtl8651_setAsicEthernetPHYReg( phy, 22, ((val & (0xff00)) | 0x17) );
				rtl8651_getAsicEthernetPHYReg( phy, 30, &val );

				if (esd3_skip_one == 1) {
					phy_reg30[phy] = BIT(31) | (val & 0xfff);
				}
				else
				if ((phy_reg30[phy] & 0xfff) != (val & 0xfff)) {
					if (diff_more_than_1((phy_reg30[phy] & 0xf), (val & 0xf)) ||
						diff_more_than_1(((phy_reg30[phy] >> 4) & 0xf), ((val >> 4) & 0xf)) ||
						diff_more_than_1(((phy_reg30[phy] >> 8) & 0xf), ((val >> 8) & 0xf))
						) {
						panic_printk("  ESD-3: old= 0x%x, new= 0x%x\n", phy_reg30[phy] & 0xfff, val & 0xfff);
						//do {} while(1); // reboot
						machine_restart(NULL);
					}
					phy_reg30[phy] = BIT(31) | (val & 0xfff);
				}
			}

		}
		if (esd3_skip_one == 1) {
			esd3_skip_one = 0;
		}
		one_second_counter = 0;
	}
	}
#endif

#ifdef CONFIG_RTL_8196C_ESD
#if defined(CONFIG_PRINTK)
#define panic_printk         printk
#endif
	if (_96c_esd_counter) {
		if( (RTL_R32(PCRP4) & EnablePHYIf) == 0)
		{
			if (++_96c_esd_reboot_counter >= 20) {
				panic_printk("  ESD reboot...\n");
				machine_restart(NULL);
			}
		}
		else {
			_96c_esd_reboot_counter = 0;
		}
	}
#endif

#if defined (CONFIG_RTL_REINIT_SWITCH_CORE)
	for(i = 0; i < ETH_INTF_NUM; i++)
	{
		struct dev_priv *tmp_cp;

		int portnum, startport=0;
		tmp_cp = ((struct dev_priv *)_rtl86xx_dev.dev[i]->priv);
		if(tmp_cp && tmp_cp->portmask && tmp_cp->opened) {

#if defined(CONFIG_RTL_819XD)
			if (rtl_port0Refined == 1)
			{
				startport = 1;
			}
#endif
			for(portnum=startport;portnum<5;portnum++)
			{
				if(tmp_cp->portmask & (1<<portnum))
					break;
			}
			if(5 == portnum)
				continue;
			if((RTL_R32(PCRP0+portnum*4) & EnablePHYIf) == 0)
			{
				switch(rtl865x_reInitState)
				{
					case STATE_NO_ERROR:
						if(( REG32(SYS_CLK_MAG) & SYS_SW_CLK_ENABLE)==0)
						{
							rtl865x_reInitState=STATE_SW_CLK_ENABLE_WAITING;
							rtl865x_reInitWaitCnt=2;
							REG32(SYS_CLK_MAG)=REG32(SYS_CLK_MAG)|SYS_SW_CLK_ENABLE;
						}
						else
						{
							rtl865x_reinitSwitchCore();
							rtl865x_reInitState=STATE_NO_ERROR;
						}
						break;

					case STATE_SW_CLK_ENABLE_WAITING:
						rtl865x_reInitWaitCnt--;
						if(rtl865x_reInitWaitCnt<=0)
						{
							rtl865x_reInitWaitCnt=2;
							rtl865x_reInitState=STATE_TO_REINIT_SWITCH_CORE;

						}
						break;

					case STATE_TO_REINIT_SWITCH_CORE:
						rtl865x_reInitWaitCnt--;
						if(rtl865x_reInitWaitCnt<=0)
						{
							rtl865x_reinitSwitchCore();
							rtl865x_reInitState=STATE_NO_ERROR;
						}
						break;

					default :
						rtl865x_reinitSwitchCore();
						rtl865x_reInitState=STATE_NO_ERROR;
						break;
				}
				break;
			}
		}
	}
	
#if defined (CONFIG_RTL_8197D) || defined(CONFIG_RTL_8196D)
	rtl819x_poll_sw();
#endif

#endif
#ifdef CONFIG_RTL_8196E
	refine_phy_setting();
#endif

	mod_timer(&cp->expire_timer, jiffies + HZ);

	//spin_unlock_irqrestore(&cp->lock, flags);
	local_irq_restore(flags);
}
#endif

#ifdef CONFIG_RTL8196C_GREEN_ETHERNET
static void power_save_timer(unsigned long task_priv)
{
	unsigned long		flags;
	//struct dev_priv	*cp;

	//cp = ((struct net_device *)task_priv)->priv;

	//spin_lock_irqsave (&cp->lock, flags);
	local_irq_save(flags);

	set_phy_pwr_save(port_pwr_save_low, 1);

	port_pwr_save_low = (port_pwr_save_low + 1) % 5;
	set_phy_pwr_save(port_pwr_save_low, 0);

	//mod_timer(&cp->expire_timer2, jiffies + (total_time_for_5_port / 5 / 10));
	mod_timer(&expire_timer2, jiffies + (total_time_for_5_port / 5 / 10));

	//spin_unlock_irqrestore(&cp->lock, flags);
	local_irq_restore(flags);
}
#endif
static struct net_device *irqDev=NULL;
static int re865x_open (struct net_device *dev)
{
	struct dev_priv *cp;
	unsigned long flags;
	int rc;
	#if defined(CONFIG_RTL_MULTIPLE_WAN)
	char drvNetif_name[MAX_IFNAMESIZE];
	#endif

	cp = dev->priv;
	if (cp->opened)
		return SUCCESS;

	/*	The first device be opened	*/
	if (atomic_read(&rtl_devOpened)==0)
	{
		/* this is the first open dev */
		/* should not call rtl865x_down() here */
		/* rtl865x_down();*/
#ifdef CONFIG_RTL_8198
		{
		extern void disable_phy_power_down(void);	
		disable_phy_power_down();
		}
#endif
		//spin_lock_irqsave(&cp->lock, flags);
		local_irq_save(flags);
		rtk_queue_init(&rx_skb_queue);
		rc = rtl865x_init_hw();
		local_irq_restore(flags);
		refill_rx_skb();
		//spin_unlock_irqrestore(&cp->lock, flags);
		if (rc) {
			//printk("rtl865x_init_hw() failed!\n");
			return FAILED;
		}

#if defined(RX_TASKLET)
		tasklet_init(&cp->rx_dsr_tasklet, (void *)interrupt_dsr_rx, (unsigned long)cp);
#endif
#ifdef TX_TASKLET
		tasklet_init(&cp->tx_dsr_tasklet, interrupt_dsr_tx, (unsigned long)cp);
#endif

#ifdef LINK_TASKLET
		tasklet_init(&cp->link_dsr_tasklet, interrupt_dsr_link, (unsigned long)cp);
#endif

#ifdef CONFIG_RTL_PHY_PATCH
		memset(re865x_restartNWayCtrl,0, sizeof(re865x_restartNWayCtrl));
#endif


		rc = request_irq(dev->irq, interrupt_isr, IRQF_DISABLED, dev->name, dev);
		if (rc)
		{
			printk("request_irq() error!\n");
			goto err_out_hw;
		}
		irqDev=dev;
		//cp->irq_owner =1;
		rtl865x_start();
	}

	atomic_inc(&rtl_devOpened);
	cp->opened = 1;

#ifdef CONFIG_RTL_HARDWARE_NAT
	reset_hw_mib_counter(dev);
#endif

	netif_start_queue(dev);

#if defined(DYNAMIC_ADJUST_TASKLET) || defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL8196C_REVISION_B)|| defined(CONFIG_RTL_8198) || defined(RTL8196C_EEE_MAC) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
	#if !defined(CONFIG_RTL8186_TR)
	if (dev->name[3] == '0')
	#endif
	{
		init_timer(&cp->expire_timer);
		cp->expire_timer.expires = jiffies + HZ;
		cp->expire_timer.data = (unsigned long)dev;
		cp->expire_timer.function = one_sec_timer;
		mod_timer(&cp->expire_timer, jiffies + HZ);
#ifdef DYNAMIC_ADJUST_TASKLET
		rx_cnt = 0;
#endif
	}
#endif

#ifdef CONFIG_RTL8196C_GREEN_ETHERNET
	if (REG32(REVR) == RTL8196C_REVISION_B) {
		//if (dev->name[3] == '0')
		if (atomic_read(&rtl_devOpened)==1)
		{
			#if 0
			init_timer(&cp->expire_timer2);
			cp->expire_timer2.expires = jiffies + HZ;
			cp->expire_timer2.data = (unsigned long)dev;
			cp->expire_timer2.function = power_save_timer;
			mod_timer(&cp->expire_timer2, jiffies + HZ);
			#else
			init_timer(&expire_timer2);
			expire_timer2.expires = jiffies + HZ;
			expire_timer2.data = (unsigned long)dev;
			expire_timer2.function = power_save_timer;
			mod_timer(&expire_timer2, jiffies + HZ);
			#endif
		}
	}
#endif

#ifdef CONFIG_RTL_LAYERED_DRIVER_L3
	/*FIXME_hyking: should add default route to cpu....*/
	if(rtl865x_curOpMode == GATEWAY_MODE)
#if defined(CONFIG_RTL_PUBLIC_SSID)
		rtl865x_addRoute(0,0,0,RTL_GW_WAN_DEVICE_NAME,0);
#else
		rtl865x_addRoute(0,0,0,RTL_DRV_WAN0_NETIF_NAME,0);
#endif
#endif

	#if defined(CONFIG_RTL_MULTIPLE_WAN)
	memset(drvNetif_name,0,MAX_IFNAMESIZE);
	if(rtl_get_ps_drv_netif_mapping_by_psdev_name(dev->name,drvNetif_name) == SUCCESS)
		rtl_enable_advRt_by_netifName(drvNetif_name);
	#endif
	rtl865x_enableDevPortForward( dev, cp);

	return SUCCESS;

err_out_hw:
    rtl8186_stop_hw(dev, cp);
    rtl865x_down();
    return rc;
}

static int re865x_close (struct net_device *dev)
{
	struct dev_priv *cp;
#if defined(CONFIG_RTL_MULTIPLE_WAN)
	char drvNetif_name[MAX_IFNAMESIZE];
#endif

	cp = dev->priv;
//	cp = netdev_priv(dev);
	if (!cp->opened)
		return SUCCESS;

	netif_stop_queue(dev);
	/* The last opened device	*/
	if (atomic_read(&rtl_devOpened)==1)
	{
		/*	warning:
			1.if we don't reboot,we shouldn't hold switch core from rx/tx, otherwise there will be some problem during change operation mode
			2.only when two devices go down,can we shut down nic interrupt
			3.the interrupt will be re_enable by rtl865x_start()
		*/
		rtl865x_disableInterrupt();

		//free_irq(dev->irq, GET_IRQ_OWNER(cp));
		//((struct dev_priv *)((GET_IRQ_OWNER(cp))->priv))->irq_owner = 0;
		free_irq(dev->irq, irqDev);
		//((struct dev_priv *)(irqDev->priv))->irq_owner = 0;


#ifdef RX_TASKLET
		tasklet_kill(&cp->rx_dsr_tasklet);
#endif

#ifdef TX_TASKLET
		tasklet_kill(&cp->tx_dsr_tasklet);
#endif

#ifdef LINK_TASKLET
		tasklet_kill(&cp->link_dsr_tasklet);
#endif

#ifdef CONFIG_RTL_PHY_PATCH
		memset(re865x_restartNWayCtrl,0, sizeof(re865x_restartNWayCtrl));
#endif
		free_rx_skb();

		rtk_queue_exit(&rx_skb_queue);

	}

	memset(&cp->net_stats, '\0', sizeof(struct net_device_stats));
	atomic_dec(&rtl_devOpened);
	cp->opened = 0;

	#if defined(CONFIG_RTL_MULTIPLE_WAN)
	memset(drvNetif_name,0,MAX_IFNAMESIZE);
	if(rtl_get_ps_drv_netif_mapping_by_psdev_name(dev->name,drvNetif_name) == SUCCESS)
		rtl_disable_advRt_by_netifName(drvNetif_name);

	if(rtl_port_used_by_device(cp->portmask) == FAILED)
	#endif
	{
		rtl865x_disableDevPortForward(dev, cp);
#if !defined(CONFIG_RTL_8196C)
		/*for lan dhcp client to renew ip address*/
		rtl865x_restartDevPHYNway(dev, cp);
#endif
		rtl8186_stop_hw(dev, cp);
	}

#if defined(DYNAMIC_ADJUST_TASKLET) || defined(CONFIG_RTL8186_TR) || defined(BR_SHORTCUT) || defined(CONFIG_RTL8196C_REVISION_B) || defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
    if (timer_pending(&cp->expire_timer))
        del_timer_sync(&cp->expire_timer);
#endif

#ifdef CONFIG_RTL8196C_GREEN_ETHERNET
	if (atomic_read(&rtl_devOpened)==0)
	{
		if (REG32(REVR) == RTL8196C_REVISION_B) {
			#if 0
     			if (timer_pending(&cp->expire_timer2))
                    	del_timer_sync(&cp->expire_timer2);
			#else
			if (timer_pending(&expire_timer2))
				del_timer_sync(&expire_timer2);
			#endif
		}
	}
#endif

#ifdef BR_SHORTCUT
	if (dev == cached_dev)
		cached_dev=NULL;
#endif

#ifdef BR_SHORTCUT_C2
	if (dev == cached_dev2)
		cached_dev2=NULL;
#endif

#ifdef CONFIG_RTL_HARDWARE_NAT
	reset_hw_mib_counter(dev);
#endif

	return SUCCESS;
}

#if defined(CONFIG_RTL_STP) || (defined(CONFIG_RTL_CUSTOM_PASSTHRU))
static int re865x_pseudo_open (struct net_device *dev)
{
	struct dev_priv *cp;

	cp = dev->priv;
	//cp = netdev_priv(dev);
	if (cp->opened)
		return SUCCESS;

	cp->opened = 1;
	netif_start_queue(dev);
	return SUCCESS;
}


static int re865x_pseudo_close (struct net_device *dev)
{
	struct dev_priv *cp;

	cp = dev->priv;
//	cp = netdev_priv(dev);

	if (!cp->opened)
		return SUCCESS;
	netif_stop_queue(dev);

	memset(&cp->net_stats, '\0', sizeof(struct net_device_stats));
	cp->opened = 0;

#ifdef BR_SHORTCUT
	if (dev == cached_dev)
		cached_dev=NULL;
#endif
#ifdef BR_SHORTCUT_C2
	if (dev == cached_dev2)
		cached_dev2=NULL;
#endif
	return SUCCESS;
}
#endif

#if defined(CONFIG_RTL_STP)
static int re865x_stp_mapping_init(void)
{
	int i, j, k, totalVlans;
	totalVlans=((sizeof(vlanconfig))/(sizeof(struct rtl865x_vlanConfig)))-1;

	for (i = 0; i < MAX_RE865X_STP_PORT; i++)
	{
		STP_PortDev_Mapping[i] = NO_MAPPING;
	}

	STP_PortDev_Mapping[WLAN_PSEUDO_IF_INDEX] = WLAN_PSEUDO_IF_INDEX;
	#ifdef CONFIG_RTK_MESH
	STP_PortDev_Mapping[WLAN_MESH_PSEUDO_IF_INDEX] = WLAN_MESH_PSEUDO_IF_INDEX;
	#endif

	j = 0;
	for(k=0;k<totalVlans;k++)
	{
		if (vlanconfig[k].isWan == FALSE)
		{
			for(i=0; i< MAX_RE865X_ETH_STP_PORT ; i++)
			{
				if ( (1<<i) & vlanconfig[k].memPort )
				{
					STP_PortDev_Mapping[j] = i;
					j++;
				}
			}

			break;
		}
	}

	return SUCCESS;
}

static int re865x_stp_mapping_reinit(void)
{
	int i, j, k, totalVlans;
	totalVlans=((sizeof(vlanconfig))/(sizeof(struct rtl865x_vlanConfig)))-1;

	for (i = 0; i < MAX_RE865X_STP_PORT; i++)
	{
		STP_PortDev_Mapping[i] = NO_MAPPING;
	}

	STP_PortDev_Mapping[WLAN_PSEUDO_IF_INDEX] = WLAN_PSEUDO_IF_INDEX;
	#ifdef CONFIG_RTK_MESH
	STP_PortDev_Mapping[WLAN_MESH_PSEUDO_IF_INDEX] = WLAN_MESH_PSEUDO_IF_INDEX;
	#endif

	j = 0;
	for(k=0;k<totalVlans;k++)
	{
		if (vlanconfig[k].isWan == FALSE)
		{
			for(i=0; i< MAX_RE865X_ETH_STP_PORT; i++)
			{
				if ( (1<<i) & vlanconfig[k].memPort )
				{
					STP_PortDev_Mapping[j] = i;
					j++;
				}
			}

			break;
		}
	}

	return SUCCESS;
}

static int re865x_stp_get_pseudodevno(uint32 port_num)
{
	int i, dev_no;
	for(i=0; i< MAX_RE865X_STP_PORT-1 ; i++)
	{
		if( STP_PortDev_Mapping[i] == port_num)
		{
			dev_no = i;
			return dev_no;
		}
	}
	return NO_MAPPING;

}

static int getVidByPort(uint32 port_num)
{
	int i, totalVlans, retVid;

	retVid=0;
	totalVlans=((sizeof(vlanconfig))/(sizeof(struct rtl865x_vlanConfig)))-1;

	for(i=0;i<totalVlans;i++)
	{
		if((1<<port_num) & vlanconfig[i].memPort)
		{
			retVid=vlanconfig[i].vid;
			break;
		}
	}

	return retVid;
}
#endif

#if defined(CONFIG_RTL_LOCAL_PUBLIC)
//hyking:this function should move to rtl865x_fdb.c
//implement it at here just for releaae to natami...
//2010-02-22
static int32 rtl865x_getPortlistByMac(const unsigned char *mac,uint32 *portlist)
{
	int32 found = FAILED;
	ether_addr_t *macAddr;
	int32 column;
	rtl865x_tblAsicDrv_l2Param_t	fdbEntry;

	macAddr = (ether_addr_t *)(mac);
	found = rtl865x_Lookup_fdb_entry(0, macAddr, FDB_DYNAMIC, &column, &fdbEntry);
	if(found == SUCCESS)
	{
		if(portlist)
			*portlist = fdbEntry.memberPortMask;
	}

	return found;

}
#endif

#if defined (CONFIG_RTL_IGMP_SNOOPING)
int re865x_setMCastTxInfo(struct sk_buff *skb,struct net_device *dev, rtl_nicTx_info	*nicTx)
{
	int32 ret;
	 struct dev_priv *cp;
	struct iphdr *iph=NULL;
	#if defined (CONFIG_RTL_MLD_SNOOPING)
	struct ipv6hdr *ipv6h=NULL;
	#endif
	unsigned int l4Protocol=0;
	struct rtl_multicastDataInfo multicastDataInfo;
	struct rtl_multicastFwdInfo multicastFwdInfo;

#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	struct sk_buff *skb_wan=NULL;
	rtl_nicTx_info	nicTx_wan;
#endif

	if((skb==NULL) || (dev==NULL) ||(nicTx==NULL))
	{
		return -1;
	}

	if((igmpsnoopenabled==0) || (nicIgmpModuleIndex==0xFFFFFFFF))
	{
		return -1;
	}


	cp = dev->priv;

	nicTx->portlist=cp->portmask;

	if((skb->data[0]==0x01) && (skb->data[1]==0x00) && (skb->data[2]==0x5e))
	{
		iph = re865x_getIpv4Header(skb->data);
		if(iph)
		{
			l4Protocol=iph->protocol;
			if((l4Protocol==IPPROTO_UDP) || (l4Protocol==IPPROTO_TCP) )
			{
				if(cp->portnum<=1)
				{
					return -1;
				}

				/*only process tcp/udp in igmp snooping data plane*/
				multicastDataInfo.ipVersion=4;
				memcpy(multicastDataInfo.sourceIp,&(iph->saddr),4);
				memcpy(multicastDataInfo.groupAddr,&(iph->daddr),4);
				/*
				multicastDataInfo.sourceIp[0]=  (uint32)(iph->saddr);
				multicastDataInfo.groupAddr[0]=  (uint32)(iph->daddr);
				*/
				ret= rtl_getMulticastDataFwdInfo(nicIgmpModuleIndex, &multicastDataInfo, &multicastFwdInfo);
				nicTx->portlist = multicastFwdInfo.fwdPortMask& cp->portmask & ((1<<RTL8651_MAC_NUMBER)-1);



			}
            #ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
			//fix tim; upnp
                	if( vlan_bridge_multicast_tag)
				{
					if((cp->id == vlan_bridge_tag) && (nicTx->portlist & RTL_WANPORT_MASK))
					{
						nicTx->portlist &= (~RTL_WANPORT_MASK);
						skb_wan = skb_copy(skb, GFP_ATOMIC);
						 if(skb_wan!=NULL)
					     {
					       	nicTx_wan.txIdx=0;
                            nicTx_wan.vid = vlan_bridge_multicast_tag;
							nicTx_wan.portlist = RTL_WANPORT_MASK;
							nicTx_wan.srcExtPort = 0;
							nicTx_wan.flags = (PKTHDR_USED|PKT_OUTGOING);
						nicTx_wan.tagport = RTL_WANPORT_MASK;
							_dma_cache_wback_inv((unsigned long) skb_wan->data, skb_wan->len);
							if (swNic_send((void *)skb_wan, skb_wan->data, skb_wan->len, &nicTx_wan) < 0)
							{
								dev_kfree_skb_any(skb_wan);
							}
						}
			}

					}//hw-vlan
			#endif

		}
	}
#if defined (CONFIG_RTL_MLD_SNOOPING)
	else if ((skb->data[0]==0x33) && (skb->data[1]==0x33) && (skb->data[2]!=0xff))
	{

		if(mldSnoopEnabled!=TRUE)
		{
			return -1;
		}
		if(cp->portnum<=1)
		{
			return -1;
		}

		ipv6h=re865x_getIpv6Header(skb->data);
		if(ipv6h!=NULL)
		{
			l4Protocol=re865x_getIpv6TransportProtocol(ipv6h);
			/*udp or tcp packet*/
			if((l4Protocol==IPPROTO_UDP) || (l4Protocol==IPPROTO_TCP))
			{
					/*only process tcp/udp in igmp snooping data plane*/
				multicastDataInfo.ipVersion=6;
				memcpy(&multicastDataInfo.sourceIp, &ipv6h->saddr, 16);
				memcpy(&multicastDataInfo.groupAddr, &ipv6h->daddr, 16);
				ret= rtl_getMulticastDataFwdInfo(nicIgmpModuleIndex, &multicastDataInfo, &multicastFwdInfo);
				nicTx->portlist = multicastFwdInfo.fwdPortMask& cp->portmask & ((1<<RTL8651_MAC_NUMBER)-1);

			}

		}


	}
#endif
	return 0;
}
#endif

//#if defined(CONFIG_RTK_VLAN_SUPPORT) && defined(CONFIG_RTK_VLAN_FOR_CABLE_MODEM)
struct net_device* re865x_get_netdev_by_name(const char* name)
{
	int i;
	for(i = 0; i < ETH_INTF_NUM; i++)
	{
		if(strcmp(_rtl86xx_dev.dev[i]->name,name) == 0)
			return _rtl86xx_dev.dev[i];
	}
	return NULL;
}
//#endif

#if defined(CONFIG_RTL_STP)
static inline int rtl_process_stp_tx(rtl_nicTx_info *txInfo)
{
	struct net_device *dev;
	struct sk_buff *skb = NULL;
	struct dev_priv *cp;
	uint8 stpPortNum;

	skb = txInfo->out_skb;
	dev = skb->dev;
	cp = dev->priv;
	if(!dev->irq){
		//virtual interfaces have no IRQ assigned.
		//We use device name to identify STP port interfaces(virtual devices).
		if(memcmp((void *)(dev->name), "port", 4)==0)
		{
			if ((skb->data[0]&0x01) && !memcmp(&(skb->data[0]), STPmac, 5) && !(skb->data[5] & 0xF0))
			{
				stpPortNum= dev->name[strlen(dev->name)-1]-'0';
				if (STP_PortDev_Mapping[stpPortNum] != NO_MAPPING)
				{
					cp->id=getVidByPort(stpPortNum);
					cp->portmask = 1<<STP_PortDev_Mapping[stpPortNum];
					cp->portnum = 1;	//Multicast process will check this entry.
				}
				else
				{
					dev_kfree_skb_any(skb);
					return FAILED;
				}
			}
			else
			{
				//To STP port interfaces(virtual devices), drop non bpdu tx pkt.
				dev_kfree_skb_any(skb);
				return FAILED;
			}
		}
	}

	return SUCCESS;
}
#endif

#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
static inline int rtl_process_vlan_tag(rtl_nicTx_info *txInfo)
{

	struct sk_buff *skb = NULL;	
	struct sk_buff *newskb;
	struct net_device *dev;
	struct dev_priv *cp;
	
	skb = txInfo->out_skb;
	dev = skb->dev;
	cp = dev->priv;	
	
	if(vlan_tag && skb->srcPhyPort == RX_FROM_LOCAL)
	{
		if((rtl865x_curOpMode == GATEWAY_MODE && cp->id == vlanconfig[1].vid )||(rtl865x_curOpMode == BRIDGE_MODE))
		{
		
			newskb = NULL;
			if (skb_cloned(skb))
			{
				newskb = skb_copy(skb, GFP_ATOMIC);
				if (newskb == NULL) 
				{
					cp->net_stats.tx_dropped++;
					dev_kfree_skb_any(skb);
					return FAILED;		
				}
				dev_kfree_skb_any(skb);
				skb = newskb;
				txInfo->out_skb = skb;
			}
			if (*((unsigned short *)(skb->data+ETH_ALEN*2)) != __constant_htons(ETH_P_8021Q))
			{
				if (skb_headroom(skb) < VLAN_HLEN && skb_cow(skb, VLAN_HLEN) !=0 )
				{
					printk("%s-%d: error! (skb_headroom(skb) == %d < %d). Enlarge it!\n",
					__FUNCTION__, __LINE__, skb_headroom(skb), VLAN_HLEN);
					while (1) ;
				}
				skb_push(skb, VLAN_HLEN);
				memmove(skb->data, skb->data + VLAN_HLEN, VLAN_ETH_ALEN<<1);
				*(uint16*)(&(skb->data[12])) = __constant_htons(ETH_P_8021Q); 
				*(uint16*)(&(skb->data[14])) = htons(vlan_tag)&0x0fff;// VID 
				*(uint8*)(&(skb->data[14])) |= (((uint8)vlan_host_pri)&0x7) << 5;
			}
		}
	}
	return SUCCESS;
}
#endif

//__MIPS16
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
static inline int rtl_process_passthru_tx(rtl_nicTx_info *txInfo)
{
	struct net_device *dev;
	struct sk_buff *skb = NULL;
 	struct dev_priv *cp;

	if(oldStatus)
	{
		skb = txInfo->out_skb;
		dev = skb->dev;
		if (dev==_rtl86xx_dev.pdev)
		{
			if (SUCCESS==rtl_isPassthruFrame(skb->data))
			{
				cp = _rtl86xx_dev.pdev->priv;
				skb->dev=cp->dev;

			}
			else
			{
				dev_kfree_skb_any(skb);
					return FAILED;
			}
		}
	}

	return SUCCESS;
}
#endif

#if defined(CONFIG_RTK_VLAN_SUPPORT)
static inline int rtl_process_rtk_vlan_tx(rtl_nicTx_info *txInfo)
{
	struct net_device *dev;
	struct sk_buff *skb = NULL;
	struct sk_buff *newskb;
	struct dev_priv *cp;
	if(rtk_vlan_support_enable)
	{
		skb = txInfo->out_skb;
		dev = skb->dev;
		cp = dev->priv;
		if (cp->vlan_setting.global_vlan)
		{
			newskb = NULL;
			if (skb_cloned(skb))
			{
				#if defined(CONFIG_RTL_ETH_PRIV_SKB)
				newskb = priv_skb_copy(skb);
				#else
				newskb = skb_copy(skb, GFP_ATOMIC);
				#endif
				if (newskb == NULL)
				{
					cp->net_stats.tx_dropped++;
					dev_kfree_skb_any(skb);
					return FAILED;
				}
				dev_kfree_skb_any(skb);
				skb = newskb;
				txInfo->out_skb = skb;
			}

			if (tx_vlan_process(dev, &cp->vlan_setting, skb, 0))
			{
				cp->net_stats.tx_dropped++;
				dev_kfree_skb_any(skb);
				return FAILED;
			}
		}
	}
	return SUCCESS;
}
#endif

static inline int rtl_pstProcess_xmit(struct dev_priv *cp,int len)
{
#if !defined(CONFIG_RTL_NIC_HWSTATS)
	cp->net_stats.tx_packets++;
	cp->net_stats.tx_bytes += len;
#endif
	cp->dev->trans_start = jiffies;
	return SUCCESS;
}

static inline int rtl_preProcess_xmit(rtl_nicTx_info *txInfo)
{
	int retval = FAILED;
	#if defined(CONFIG_RTL_STP)
	retval = rtl_process_stp_tx(txInfo);
	if(FAILED == retval)
		return retval;
	#endif
	#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
	retval = rtl_process_passthru_tx(txInfo);
	if(FAILED == retval)
		return retval;
	#endif
	#if defined(CONFIG_RTL_HW_VLAN_SUPPORT)
	retval = rtl_process_hw_vlan_tx(txInfo);
	if(FAILED == retval)
		return retval;
	#endif
	#if defined(CONFIG_RTK_VLAN_SUPPORT)
	retval = rtl_process_rtk_vlan_tx(txInfo);
	if(FAILED == retval)
		return retval;
	#endif
        #ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	retval = rtl_process_vlan_tag(txInfo);
		return retval;
	#endif
	#if defined(CONFIG_RTL_REINIT_SWITCH_CORE)
	if(rtl865x_duringReInitSwtichCore==1) {
		dev_kfree_skb_any(txInfo->out_skb);
		return FAILED;
	}
	#endif

	return SUCCESS;
}

static inline void rtl_direct_txInfo(uint32 port_mask,rtl_nicTx_info *txInfo)
{
	txInfo->portlist = port_mask & 0x3f;
	txInfo->srcExtPort = 0;		//PKTHDR_EXTPORT_LIST_CPU;
	txInfo->flags = (PKTHDR_USED|PKT_OUTGOING);
}

static inline void rtl_hwLookup_txInfo(rtl_nicTx_info *txInfo)
{
	txInfo->portlist = RTL8651_CPU_PORT;		/* must be set 0x7 */
	txInfo->srcExtPort = PKTHDR_EXTPORT_LIST_CPU;
	txInfo->flags = (PKTHDR_USED|PKTHDR_HWLOOKUP|PKTHDR_BRIDGING|PKT_OUTGOING);
}

#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
static inline void rtl_hwLookup_txInfo2(rtl_nicTx_info *txInfo)
{
	txInfo->portlist = RTL8651_CPU_PORT;		/* must be set 0x7 */
	txInfo->srcExtPort = PKTHDR_EXTPORT_LIST_P2;//2->7
	txInfo->flags = (PKTHDR_USED|PKTHDR_HWLOOKUP|PKTHDR_BRIDGING|PKT_OUTGOING);
}
#endif
static inline int rtl_ip_option_check(struct sk_buff *skb)
{
	int flag = FALSE;
	if (((*((unsigned short *)(skb->data+ETH_ALEN*2)) == __constant_htons(ETH_P_IP))
		&& ((*((unsigned char*)(skb->data+ETH_ALEN*2+2)) != __constant_htons(0x45)))) ||
		((*((unsigned short *)(skb->data+ETH_ALEN*2)) == __constant_htons(ETH_P_8021Q))&&
		(*((unsigned short *)(skb->data+ETH_ALEN*2+2)) == __constant_htons(ETH_P_IP))&&
		((*((unsigned char*)(skb->data+ETH_ALEN*2+4)) != __constant_htons(0x45)))))
			flag = TRUE;
	return flag;
}
static inline int rtl_isHwlookup(struct sk_buff *skb, struct dev_priv *cp, uint32 *portlist)
{
	int flag = FALSE;

#if defined (CONFIG_RTL_MULTI_LAN_DEV) ||defined (CONFIG_POCKET_ROUTER_SUPPORT)
	goto assign_portmask;
#elif defined(CONFIG_RTK_VLAN_SUPPORT)
	if(rtk_vlan_support_enable ==1) {
		goto assign_portmask;
	}
#endif

	if ((rtl_isWanDev(cp)!=TRUE) && (rtl_ip_option_check(skb) != TRUE)) {
		flag = TRUE;
	} else {
		flag = FALSE;
	}

#if defined(CONFIG_RTL_HW_VLAN_SUPPORT)
	if((rtk_vlan_support_enable == 0) && ((!memcmp(cp->dev->name, "eth2", 5)) ||
	  (!memcmp(cp->dev->name, "eth3", 5)) ||(!memcmp(cp->dev->name, "eth4", 5))))
		flag = FALSE;
#endif

#if defined (CONFIG_RTL_LOCAL_PUBLIC)
	//hyking:
	//when hw local public and sw localpublic exist at same time,
	//pkt to sw local public would be trap to cpu by default ACL
	//2010-02-22
	flag = FALSE;
	if(rtl_isWanDev(cp)!=TRUE)
	{
		//hyking: default acl issue, direct tx now...
		rtl865x_getPortlistByMac(skb->data, portlist);
	} else
#endif
	if (flag==FALSE) {
#if defined(CONFIG_RTL_MULTI_LAN_DEV) ||defined (CONFIG_POCKET_ROUTER_SUPPORT)||defined(CONFIG_RTK_VLAN_SUPPORT)
assign_portmask:
#endif
		*portlist = cp->portmask;
	}

	return flag;
}
static inline int rtl_fill_txInfo(rtl_nicTx_info *txInfo)
{
	uint32 portlist;
	struct sk_buff *skb = txInfo->out_skb;
	struct dev_priv *cp;
	cp = skb->dev->priv;
	txInfo->vid = cp->id;

	#if defined(CONFIG_RTK_VOIP_QOS)
	txInfo->priority  = 0 ;
	#endif
	#if	defined(CONFIG_RTL_HW_QOS_SUPPORT)
	txInfo->priority= rtl_qosGetPriorityByVid(cp->id, skb->mark);
	#endif

	//default output queue is 0
	txInfo->txIdx = 0;

	#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	txInfo->tagport = 0 ;
	#endif
	
	if((skb->data[0]&0x01)==0)
	{

		if(rtl_isHwlookup(skb, cp, &portlist) == TRUE)
		{
        #ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
			if ( vlan_bridge_tag && cp->id == vlan_bridge_tag)
			{
				rtl_hwLookup_txInfo2(txInfo);
			}
			else
		#endif
			rtl_hwLookup_txInfo(txInfo);
		}
		else
		{
			rtl_direct_txInfo(portlist, txInfo);
		}
	}
	else 
	{
		/*multicast process*/
		rtl_direct_txInfo(cp->portmask,txInfo);
#if defined (CONFIG_RTL_IGMP_SNOOPING)
		/*multicast process*/
		if(	((skb->data[0]==0x01) && (skb->data[1]==0x00) && (skb->data[2]==0x5e))
#if defined (CONFIG_RTL_MLD_SNOOPING)
			||	((skb->data[0]==0x33) && (skb->data[1]==0x33) && (skb->data[2]!=0xFF))
#endif
		)
		{
			re865x_setMCastTxInfo(skb,cp->dev, txInfo);
		}
#endif

	}
	
	//for WAN Tag
	#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	if((vlan_bridge_tag && cp->id == vlan_bridge_tag) || (vlan_tag && cp->id == vlan_tag))
		txInfo->tagport = RTL_WANPORT_MASK;
	#endif		
	
	if(txInfo->portlist==0)
	{
		dev_kfree_skb_any(skb);
		return FAILED;
	}
	return SUCCESS;
}

#if defined(CONFIG_RTK_VLAN_NEW_FEATURE)
static int rtl_bridge_wan_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct net_device *wan_dev = NULL;
	if(!memcmp(lan_macaddr,skb->data+6, 6)) {
		dev_kfree_skb_any(skb);
		return 0;
	}
	//printk("[%s][%d]-skb->dev[%s],proto(0x%x)\n", __FUNCTION__, __LINE__, skb->dev->name,skb->protocol);
	wan_dev = rtl_get_wan_from_vlan_info();

	if(wan_dev){
		skb->dev = wan_dev;
		wan_dev->netdev_ops->ndo_start_xmit(skb, wan_dev);
		}
	return 0;
}
#endif


#define	RTL_NIC_TX_RETRY_MAX		(128)
__MIPS16
__IRAM_FWD
static int re865x_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	int retval, tx_retry_cnt;
	struct dev_priv *cp;
	struct sk_buff *tx_skb;
	rtl_nicTx_info	nicTx;


	nicTx.out_skb = skb;
	retval = rtl_preProcess_xmit(&nicTx);

	if(FAILED == retval)
		return 0;

	tx_skb = nicTx.out_skb;
	cp = tx_skb->dev->priv;

	if((cp->id==0) || (cp->portmask ==0)) {
		dev_kfree_skb_any(tx_skb);
		return 0;
	}


	retval = rtl_fill_txInfo(&nicTx);
	if(FAILED == retval)
		return 0;

#if defined(CONFIG_RTL_QOS_PATCH) || defined(CONFIG_RTK_VOIP_QOS)
	if(((struct sk_buff *)tx_skb)->srcPhyPort == QOS_PATCH_RX_FROM_LOCAL){
		nicTx.priority = QOS_PATCH_HIGH_QUEUE_PRIO;
		nicTx.txIdx=RTL865X_SWNIC_TXRING_MAX_PKTDESC-1;	//use the highest tx ring index, note: not RTL865X_SWNIC_TXRING_HW_PKTDESC-1
	}
#endif

	_dma_cache_wback_inv((unsigned long) tx_skb->data, tx_skb->len);
	tx_retry_cnt = 0;
	while(swNic_send((void *)tx_skb, tx_skb->data, tx_skb->len, &nicTx) < 0)
	{
		#if defined(CONFIG_RTL_PROC_DEBUG)
		tx_ringFull_cnt++;
		#endif
		swNic_txDone(nicTx.txIdx);
		if ((tx_retry_cnt++)>RTL_NIC_TX_RETRY_MAX) {
			dev_kfree_skb_any(tx_skb);
			return 0;
		}
	}



	rtl_pstProcess_xmit(cp,tx_skb->len);
	//cp->net_stats.tx_packets++;
	//cp->net_stats.tx_bytes += tx_skb->len;

    return 0;
}


static void re865x_tx_timeout (struct net_device *dev)
{
	rtlglue_printf("Tx Timeout!!! Can't send packet\n");
}

#if defined(RTL819X_PRIV_IOCTL_ENABLE)
 int rtl819x_get_port_status(int portnum , struct lan_port_status *port_status)
{
		uint32	regData;
		uint32	data0;

		if( portnum < 0 ||  portnum > CPU)
			return -1;

		regData = READ_MEM32(PSRP0+((portnum)<<2));

		//printk("rtl819x_get_port_status port = %d data=%x\n", portnum,regData); //mark_debug
		data0 = regData & PortStatusLinkUp;
		if (data0)
			port_status->link =1;
		else
			port_status->link =0;

		data0 = regData & PortStatusNWayEnable;
		if (data0)
			port_status->nway=1;
		else
			port_status->nway =0;

		data0 = regData & PortStatusDuplex;
		if (data0)
			port_status->duplex=1;
		else
			port_status->duplex =0;

		data0 = (regData&PortStatusLinkSpeed_MASK)>>PortStatusLinkSpeed_OFFSET;
		port_status->speed = data0 ; // 0 = 10M , 1= 100M , 2=1G ,

	       return 0;
}

 int rtl819x_get_port_stats(int portnum , struct port_statistics *port_stats)
 {

	uint32 addrOffset_fromP0 =0;

	//printk("rtl819x_get_port_stats port = %d \n", portnum); //mark_debug
	if( portnum < 0 ||  portnum > CPU)
			return -1;

	addrOffset_fromP0 = portnum * MIB_ADDROFFSETBYPORT;

	//port_stats->rx_bytes =(uint32) (rtl865xC_returnAsicCounter64( OFFSET_IFINOCTETS_P0 + addrOffset_fromP0 )) ;
	port_stats->rx_bytes =rtl8651_returnAsicCounter( OFFSET_IFINOCTETS_P0 + addrOffset_fromP0 ) ;
	port_stats->rx_unipkts= rtl8651_returnAsicCounter( OFFSET_IFINUCASTPKTS_P0 + addrOffset_fromP0 ) ;
	port_stats->rx_mulpkts= rtl8651_returnAsicCounter( OFFSET_ETHERSTATSMULTICASTPKTS_P0 + addrOffset_fromP0 ) ;
	port_stats->rx_bropkts= rtl8651_returnAsicCounter( OFFSET_ETHERSTATSBROADCASTPKTS_P0 + addrOffset_fromP0 ) ;
	port_stats->rx_discard= rtl8651_returnAsicCounter( OFFSET_DOT1DTPPORTINDISCARDS_P0 + addrOffset_fromP0 ) ;
	port_stats->rx_error= (rtl8651_returnAsicCounter( OFFSET_DOT3STATSFCSERRORS_P0 + addrOffset_fromP0 ) +
						 rtl8651_returnAsicCounter( OFFSET_ETHERSTATSJABBERS_P0 + addrOffset_fromP0 ));

	//port_stats->tx_bytes =(uint32) (rtl865xC_returnAsicCounter64( OFFSET_IFOUTOCTETS_P0 + addrOffset_fromP0 )) ;
	port_stats->tx_bytes =rtl8651_returnAsicCounter( OFFSET_IFOUTOCTETS_P0 + addrOffset_fromP0 ) ;
	port_stats->tx_unipkts= rtl8651_returnAsicCounter( OFFSET_IFOUTUCASTPKTS_P0 + addrOffset_fromP0 ) ;
	port_stats->tx_mulpkts= rtl8651_returnAsicCounter( OFFSET_IFOUTMULTICASTPKTS_P0 + addrOffset_fromP0 ) ;
	port_stats->tx_bropkts= rtl8651_returnAsicCounter( OFFSET_IFOUTBROADCASTPKTS_P0 + addrOffset_fromP0 ) ;
	port_stats->tx_discard= rtl8651_returnAsicCounter( OFFSET_IFOUTDISCARDS + addrOffset_fromP0 ) ;
	port_stats->tx_error= (rtl8651_returnAsicCounter( OFFSET_ETHERSTATSCOLLISIONS_P0 + addrOffset_fromP0 ) +
						 rtl8651_returnAsicCounter( OFFSET_DOT3STATSDEFERREDTRANSMISSIONS_P0 + addrOffset_fromP0 ));

	return 0;
 }

int re865x_priv_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	int32 rc = 0;
	unsigned long *data_32;
	int portnum=0;
	struct lan_port_status port_status;
	struct port_statistics port_stats;

	data_32 = (unsigned long *)rq->ifr_data;
	if (copy_from_user(&portnum, data_32, 1*sizeof(unsigned long)))
	{
		return -EFAULT;
	}

	switch (cmd)
	{
	     case RTL819X_IOCTL_READ_PORT_STATUS:
		 	rc = rtl819x_get_port_status(portnum,&port_status); //portnumber
		 	if(rc != 0) {
				return -EFAULT;
			}
		 	if (copy_to_user((void *)rq->ifr_data, (void *)&port_status, sizeof(struct lan_port_status))) {
				return -EFAULT;
			}
		 	break;
	     case RTL819X_IOCTL_READ_PORT_STATS:
		 	rc = rtl819x_get_port_stats(portnum,&port_stats); //portnumber
		 	if(rc != 0)
				return -EFAULT;
		 	if (copy_to_user((void *)rq->ifr_data, (void *)&port_stats, sizeof(struct port_statistics)))
				return -EFAULT;
		 	break;
		 	break;
	     default :
		 	rc = -EOPNOTSUPP;
			break;
	}
	return SUCCESS;

}

#endif
int re865x_ioctl (struct net_device *dev, struct ifreq *rq, int cmd)
{
	int32 rc = 0;
	unsigned long *data;
	int32 args[4];
	int32  * pRet;
	#if defined(CONFIG_RTL8186_KB)||defined(CONFIG_RTL8186_GR)
	uint32	*pU32;
	#endif

	if (cmd != SIOCDEVPRIVATE)
	{
		#if defined(RTL819X_PRIV_IOCTL_ENABLE)
		rc = re865x_priv_ioctl(dev,rq,cmd);
 		return rc;
		#else
 		goto normal;
		#endif
	}

	data = (unsigned long *)rq->ifr_data;

	if (copy_from_user(args, data, 4*sizeof(unsigned long)))
	{
		return -EFAULT;
	}

	switch (args[0])
	{

#ifdef CONFIG_RTL8196_RTL8366
		case RTL8651_IOCTL_GETWANLINKSTATUS:
			{
				uint32 phyNum;
				uint32 linkStatus;

				pRet = (int32 *)args[3];
				*pRet = FAILED;
				rc = SUCCESS;

				phyNum=4;//8366 WAN port
				rc=rtl8366rb_getPHYLinkStatus(phyNum, &linkStatus);

				if(rc==SUCCESS)
				{
					if(linkStatus==1)
					{
						*pRet = SUCCESS;
					}
				}

				break;
			}
#else
		case RTL8651_IOCTL_GETWANLINKSTATUS:
			{
				int i;
				int wanPortMask;
				int32 totalVlans;

				pRet = (int32 *)args[3];
				*pRet = FAILED;
				rc = SUCCESS;

				wanPortMask = 0;
				totalVlans=((sizeof(vlanconfig))/(sizeof(struct rtl865x_vlanConfig)))-1;
				for(i=0;i<totalVlans;i++)
				{
					if(vlanconfig[i].isWan==TRUE)
						wanPortMask = vlanconfig[i].memPort;
				}
				if (wanPortMask==0)
				{
					/* no wan port exist */
					break;
				}

				for(i=0;i<RTL8651_AGGREGATOR_NUMBER;i++)
				{
					if( (1<<i)&wanPortMask )
					{
						if((READ_MEM32(PSRP0+(i<<2))&PortStatusLinkUp)!=0)
						{
							*pRet = SUCCESS;
						}
						break;
					}
				}

				break;
			}
#endif
		case RTL8651_IOCTL_GETWANLINKSPEED:
			{
				int i;
				int wanPortMask;
				int32 totalVlans;

				pRet = (int32 *)args[3];
				*pRet = FAILED;
				rc = FAILED;

				wanPortMask = 0;
				totalVlans=((sizeof(vlanconfig))/(sizeof(struct rtl865x_vlanConfig)))-1;
				for(i=0;i<totalVlans;i++)
				{
					if(vlanconfig[i].isWan==TRUE)
						wanPortMask = vlanconfig[i].memPort;
				}

				if (wanPortMask==0)
				{
					/* no wan port exist */
					break;
				}

				for(i=0;i<RTL8651_AGGREGATOR_NUMBER;i++)
				{
					if( (1<<i)&wanPortMask )
					{
						break;
					}
				}

				switch(READ_MEM32(PSRP0 + (i<<2)) & PortStatusLinkSpeed_MASK)
				{
					case PortStatusLinkSpeed10M:
						*pRet = PortStatusLinkSpeed10M;
						rc = SUCCESS;
						break;
					case PortStatusLinkSpeed100M:
						*pRet = PortStatusLinkSpeed100M;
						rc = SUCCESS;
						break;
					case  PortStatusLinkSpeed1000M:
						*pRet = PortStatusLinkSpeed1000M;
						rc = SUCCESS;
						break;
					default:
						break;
				}
				break;
			}
#if defined(CONFIG_RTL8186_KB)|| defined(CONFIG_RTL8186_GR)
		case RTL8651_IOCTL_GETLANLINKSTATUS:
			{
				int i;
				int lanPortMask;
				int32 totalVlans;

				pRet = (int32 *)args[3];
				*pRet = FAILED;
				rc = SUCCESS;

				lanPortMask = 0;
				totalVlans=((sizeof(vlanconfig))/(sizeof(struct rtl865x_vlanConfig)))-1;
				for(i=0;i<totalVlans;i++)
				{
					if(vlanconfig[i].isWan==FALSE)
					{
						lanPortMask = vlanconfig[i].memPort;
						if (lanPortMask==0)
						{
							/* no wan port exist */
							continue;
						}

						for(i=0;i<=RTL8651_PHY_NUMBER;i++)
						{
							if( (1<<i)&lanPortMask )
							{
								if((READ_MEM32(PSRP0+(i<<2))&PortStatusLinkUp)!=0)
								{
									//rtlglue_printf("Lan port i=%d\n",i);//Added for test
									*pRet = SUCCESS;
									return rc;
								}
							}
						}
					}
				}

				break;
			}
#if defined(CONFIG_RTL8186_KB)
		case RTL8651_IOCTL_GETWANTHROUGHPUT:
			{
				static unsigned long last_jiffies = 0;
				static unsigned long last_rxtx = 0;
				int i;
				int32 totalVlans;
				struct dev_priv *cp;
				int32 *throughputLevel;
				unsigned long	diff_jiffies;
				pRet = (int32 *)args[3];

				diff_jiffies = (jiffies-last_jiffies);
				if (diff_jiffies>HZ)
				{
					pU32 = (uint32*)args[1];
					throughputLevel = (uint32*)pU32[0];
					rc = SUCCESS;

					cp = NULL;
					totalVlans=((sizeof(vlanconfig))/(sizeof(struct rtl865x_vlanConfig)))-1;
					for(i=0;i<totalVlans;i++)
					{
						if(vlanconfig[i].isWan==TRUE)
						{
							cp = _rtl86xx_dev.dev[i]->priv;
							break;
						}
					}

					if (cp==NULL)
					{
						/* no wan port exist */
						rc = FAILED;
					}

					for(i=1;i<20;i++)
					{
						if (diff_jiffies < (HZ<<i))
							break;
					}
					/* get the throughput level */
					*throughputLevel = (((cp->net_stats.rx_bytes + cp->net_stats.tx_bytes)-last_rxtx)>>(17+i));

					last_jiffies = jiffies;
					last_rxtx = (cp->net_stats.rx_bytes + cp->net_stats.tx_bytes);
					*pRet = SUCCESS;
				}
				else
				{
					*pRet = FAILED;
				}
			}
#endif
#endif
#ifdef CONFIG_RTL_LAYERED_DRIVER
#if defined(CONFIG_RTL8186_GR)
		case RTL8651_IOCTL_SETWANLINKSTATUS:
			{
				int i;
				int wanPortMask;
				int32 totalVlans;
				int portStatusToSet;
				int forceMode;
				int forceLink;
				int forceLinkSpeed;
				int forceDuplex;
				uint32 regValue;
				uint32 advCapability;
				#define SPEED10M 	0
				#define SPEED100M 	1
				#define SPEED1000M 	2

				pRet = (int32 *)args[3];
				*pRet = FAILED;
				rc = SUCCESS;

				pU32 = (uint32*)args[1];
				portStatusToSet = *(uint32*)pU32[0];

				wanPortMask = 0;
				totalVlans=((sizeof(vlanconfig))/(sizeof(struct rtl865x_vlanConfig)))-1;
				for(i=0;i<totalVlans;i++)
				{
					if(vlanconfig[i].isWan==TRUE)
						wanPortMask = vlanconfig[i].memPort;
				}

				if (wanPortMask==0)
				{
					/* no wan port exist */
					break;
				}

				for(i=0;i<RTL8651_AGGREGATOR_NUMBER;i++)
				{
					if( (1<<i)&wanPortMask )
					{
						/*write register*/

						if(HALF_DUPLEX_10M == portStatusToSet)
						{
							forceMode=TRUE;
							forceLink=TRUE;
							forceLinkSpeed=SPEED10M;
							forceDuplex=FALSE;
							advCapability=(1<<HALF_DUPLEX_10M);
						}else if(HALF_DUPLEX_100M == portStatusToSet)
						{
							forceMode=TRUE;
							forceLink=TRUE;
							forceLinkSpeed=SPEED100M;
							forceDuplex=FALSE;
							advCapability=(1<<HALF_DUPLEX_100M);
						}else if(HALF_DUPLEX_1000M == portStatusToSet)
						{
							forceMode=TRUE;
							forceLink=TRUE;
							forceLinkSpeed=SPEED1000M;
							forceDuplex=FALSE;
							advCapability=(1<<HALF_DUPLEX_1000M);
						}else if(DUPLEX_10M == portStatusToSet)
						{
							forceMode=TRUE;
							forceLink=TRUE;
							forceLinkSpeed=SPEED10M;
							forceDuplex=TRUE;
							advCapability=(1<<DUPLEX_10M);
						}else if(DUPLEX_100M == portStatusToSet)
						{
							forceMode=TRUE;
							forceLink=TRUE;
							forceLinkSpeed=SPEED100M;
							forceDuplex=TRUE;
							advCapability=(1<<DUPLEX_100M);
						}else if(DUPLEX_1000M == portStatusToSet)
						{
							forceMode=TRUE;
							forceLink=TRUE;
							forceLinkSpeed=SPEED1000M;
							forceDuplex=TRUE;
							advCapability=(1<<DUPLEX_1000M);
						}else if(PORT_AUTO == portStatusToSet)
						{
							forceMode=FALSE;
							forceLink=TRUE;
							/*all capality*/
							advCapability=(1<<PORT_AUTO);

						}else
						{
							forceMode=FALSE;
							forceLink=TRUE;
						}
						rtl865xC_setAsicEthernetForceModeRegs(i, forceMode, forceLink, forceLinkSpeed, forceDuplex);

						/*Set PHY Register*/
						rtl8651_setAsicEthernetPHYSpeed(i,forceLinkSpeed);
						rtl8651_setAsicEthernetPHYDuplex(i,forceDuplex);
						rtl8651_setAsicEthernetPHYAutoNeg(i,TRUE);
						rtl8651_setAsicEthernetPHYAdvCapality(i,advCapability);
						rtl8651_restartAsicEthernetPHYNway(i);
						break;
					}
				}

				break;
			}
		case RTL8651_IOCTL_GETLANPORTLINKSTATUS:
			{
				int i;
                            int lanPortMask;
                            int32 totalVlans;
				int32 *lanportnum;
				int32 lanPortTypeMask;
				uint32 regVal;
				uint32 portLinkSpeed;

                            pRet = (int32 *)args[3];
                            *pRet = FAILED;
                            rc = SUCCESS;

				pU32 = (uint32*)args[1];
				lanportnum = (uint32*)pU32[0];

                                lanPortMask = 0;
                                totalVlans=((sizeof(vlanconfig))/(sizeof(struct rtl865x_vlanConfig)))-1;
                                for(i=0;i<totalVlans;i++)
                                {
                                        if(vlanconfig[i].isWan==FALSE)
                                        {
                                                lanPortMask = vlanconfig[i].memPort;
                                                if (lanPortMask==0)
                                                {
                                                        /* no wan port exist */
                                                        continue;
                                                }

                                                for(i=0;i<=RTL8651_PHY_NUMBER;i++)
                                                {
                                                        if( (1<<i)&lanPortMask )
                                                        {
                                                        	regVal=READ_MEM32(PSRP0+(i<<2));
                                                                if((regVal&PortStatusLinkUp)!=0)
                                                                {
										if(i==(*lanportnum))
										{
                                                                        	*pRet = SUCCESS;

											if((regVal&PortStatusDuplex)!=0)
											{
												lanPortTypeMask=1;
												*pRet |= lanPortTypeMask;
											}

											portLinkSpeed=regVal&PortStatusLinkSpeed_MASK;
											if(PortStatusLinkSpeed100M==portLinkSpeed)
											{
												lanPortTypeMask=4;
												*pRet |= lanPortTypeMask;
											}
											else if(PortStatusLinkSpeed1000M==portLinkSpeed)
											{
												lanPortTypeMask=8;
												*pRet |= lanPortTypeMask;
											}
											else
											{
												lanPortTypeMask=2;
												*pRet |= lanPortTypeMask;
											}

                                                                        	return rc;
										}
                                                                }
                                                        }
                                                }
                                        }
                                }

                                break;
			}

		case RTL8651_IOCTL_GETWANPORTLINKSTATUS:
			{
				int i;
				int wanPortMask;
				int32 totalVlans;
				int32 wanPortTypeMask;
				uint32 regVal;
				uint32 portLinkSpeed;

				pRet = (int32 *)args[3];
				*pRet = FAILED;
				rc = SUCCESS;

				wanPortMask = 0;
				totalVlans=((sizeof(vlanconfig))/(sizeof(struct rtl865x_vlanConfig)))-1;
				for(i=0;i<totalVlans;i++)
				{
					if(vlanconfig[i].isWan==TRUE)
						wanPortMask = vlanconfig[i].memPort;
				}

				if (wanPortMask==0)
				{
					/* no wan port exist */
					break;
				}

				for(i=0;i<RTL8651_AGGREGATOR_NUMBER;i++)
				{
					if( (1<<i)&wanPortMask )
					{
						/*check phy status link up or down*/
						rtl8651_getAsicEthernetPHYStatus(i,&regVal);
						if(regVal & (1<<2))
						{
							regVal=READ_MEM32(PSRP0+(i<<2));
							if((regVal&PortStatusLinkUp)!=0)
							{
								*pRet = SUCCESS;

								if((regVal&PortStatusDuplex)!=0)
								{
										wanPortTypeMask=1;
										*pRet |= wanPortTypeMask;
								}

								portLinkSpeed=regVal&PortStatusLinkSpeed_MASK;
								if(PortStatusLinkSpeed100M==portLinkSpeed)
								{
										wanPortTypeMask=4;
										*pRet |= wanPortTypeMask;
								}
								else if(PortStatusLinkSpeed1000M==portLinkSpeed)
								{
										wanPortTypeMask=8;
										*pRet |= wanPortTypeMask;
								}
								else
								{
										wanPortTypeMask=2;
										*pRet |= wanPortTypeMask;
								}
							}
						}
						break;
					}
				}

				break;
			}
#endif
#endif
		default:
			rc = SUCCESS;
			break;
	}

	return rc;
#if !defined(RTL819X_PRIV_IOCTL_ENABLE)
normal:
#endif
	if (!netif_running(dev))
		return -EINVAL;
	switch (cmd)
        {
	    default:
		rc = -EOPNOTSUPP;
		break;
	}
	return rc;
}

static int rtl865x_set_hwaddr(struct net_device *dev, void *addr)
{
	unsigned long flags;
	int i;
	unsigned char *p;
	ps_drv_netif_mapping_t * mapp_entry;
	struct rtl865x_vlanConfig *vlancfg_entry;

	p = ((struct sockaddr *)addr)->sa_data;
 	local_irq_save(flags);

	for (i = 0; i<ETHER_ADDR_LEN; ++i) {
		dev->dev_addr[i] = p[i];
	}

	mapp_entry = rtl_get_ps_drv_netif_mapping_by_psdev(dev);
	if(mapp_entry == NULL)
		goto out;

	vlancfg_entry = rtl_get_vlanconfig_by_netif_name(mapp_entry->drvName);
	if(vlancfg_entry == NULL)
		goto out;

	if(vlancfg_entry->vid != 0)
	{
		rtl865x_netif_t netif;
		memcpy(vlancfg_entry->mac.octet,dev->dev_addr,ETHER_ADDR_LEN);
		memcpy(netif.macAddr.octet,vlancfg_entry->mac.octet,ETHER_ADDR_LEN);
		memcpy(netif.name,vlancfg_entry->ifname,MAX_IFNAMESIZE);
		rtl865x_setNetifMac(&netif);
	}

out:
	local_irq_restore(flags);
	return SUCCESS;
}

#if defined(CONFIG_RTL8186_LINK_CHANGE)
static int rtl865x_set_link(struct net_device *dev, int enable)
{
	int32 i;
	struct dev_priv *cp;
	int32 portmask;
	int32 totalVlans;

	cp = dev->priv;
#if defined (CONFIG_RTL_MULTI_LAN_DEV)
	portmask=cp->portmask;
#else
	portmask=0;
	totalVlans=((sizeof(vlanconfig))/(sizeof(struct rtl865x_vlanConfig)))-1;

	for(i=0;i<totalVlans;i++)
	{
		if(vlanconfig[i].vid==cp->id)
		{
			portmask = vlanconfig[i].memPort;
			break;
		}
	}
#endif
	if (portmask)
	{
		if (enable)
		{
			for(i=0;i<RTL8651_PHY_NUMBER;i++)
			{
				if (portmask & (1<<i))
				{
					rtl865xC_setAsicEthernetForceModeRegs(i, FALSE, TRUE, 1, TRUE);
					rtl8651_restartAsicEthernetPHYNway(i);
				}
			}
		}
		else
		{
			for(i=0;i<RTL8651_PHY_NUMBER;i++)
			{
				if (portmask & (1<<i))
					rtl865xC_setAsicEthernetForceModeRegs(i, TRUE, FALSE, 1, TRUE);
			}
		}
	}

	return SUCCESS;
}
#endif

static int rtl865x_set_mtu(struct net_device *dev, int new_mtu)
{
	unsigned long flags;
	ps_drv_netif_mapping_t * mapp_entry;
	struct rtl865x_vlanConfig *vlancfg_entry;

 	local_irq_save(flags);
	dev->mtu = new_mtu;

	mapp_entry = rtl_get_ps_drv_netif_mapping_by_psdev(dev);
	if(mapp_entry == NULL)
		goto out;

	vlancfg_entry = rtl_get_vlanconfig_by_netif_name(mapp_entry->drvName);
	if(vlancfg_entry == NULL)
		goto out;

	if(vlancfg_entry->vid !=0)
	{
		rtl865x_netif_t netif;
		vlancfg_entry->mtu = new_mtu;
		netif.mtu = new_mtu;
		memcpy(netif.name,vlancfg_entry->ifname,MAX_IFNAMESIZE);

		rtl865x_setNetifMtu(&netif);
	}
	#ifdef CONFIG_HARDWARE_NAT_DEBUG
	/*2007-12-19*/
		rtlglue_printf("%s:%d:new_mtu is %d\n",__FUNCTION__,__LINE__,new_mtu);
	#endif

out:
	local_irq_restore(flags);

	return SUCCESS;
}

#if defined(CONFIG_COMPAT_NET_DEV_OPS)
#else
static const struct net_device_ops rtl819x_netdev_ops = {
	.ndo_open		= re865x_open,
	.ndo_stop		= re865x_close,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_set_mac_address 	= rtl865x_set_hwaddr,
	.ndo_set_multicast_list	= re865x_set_rx_mode,
	.ndo_get_stats		= re865x_get_stats,
	.ndo_do_ioctl		= re865x_ioctl,
	.ndo_start_xmit		= re865x_start_xmit,
	.ndo_tx_timeout		= re865x_tx_timeout,
#if defined(CP_VLAN_TAG_USED)
	.ndo_vlan_rx_register	= cp_vlan_rx_register,
#endif
	.ndo_change_mtu		= rtl865x_set_mtu,

};

#if !defined(CONFIG_COMPAT_NET_DEV_OPS) && defined(CONFIG_RTK_VLAN_NEW_FEATURE)
static const struct net_device_ops rtl819x_netdev_ops_bridge = {
	.ndo_open		= re865x_open,
	.ndo_stop		= re865x_close,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_set_mac_address 	= rtl865x_set_hwaddr,
	.ndo_set_multicast_list	= re865x_set_rx_mode,
	.ndo_get_stats		= re865x_get_stats,
	.ndo_do_ioctl		= re865x_ioctl,
	.ndo_start_xmit		= rtl_bridge_wan_start_xmit,
	.ndo_tx_timeout		= re865x_tx_timeout,
#if defined(CP_VLAN_TAG_USED)
	.ndo_vlan_rx_register	= cp_vlan_rx_register,
#endif
	.ndo_change_mtu		= rtl865x_set_mtu,

};
#endif

#endif


#if defined (CONFIG_RTL_MLD_SNOOPING)
static int rtl865x_addAclForMldSnooping(struct rtl865x_vlanConfig* vlanConfig)
{
	int i;
	#if defined (CONFIG_RTL_MULTI_LAN_DEV) || defined(CONFIG_RTK_VLAN_SUPPORT)
	struct rtl865x_vlanConfig tmpVlanConfig[NETIF_NUMBER];
	#endif
	struct rtl865x_vlanConfig *pVlanConfig=NULL;
	rtl865x_AclRule_t	rule;
	int ret=FAILED;

	if(vlanConfig==NULL)
	{
		return FAILED;
	}

#if defined (CONFIG_RTL_MULTI_LAN_DEV) ||defined(CONFIG_RTK_VLAN_SUPPORT)
	re865x_packVlanConfig(vlanConfig, tmpVlanConfig);
	pVlanConfig=tmpVlanConfig;
#else
	pVlanConfig=vlanConfig;
#endif

	for(i=0; pVlanConfig[i].vid != 0; i++)
	{
		if (IF_ETHER!=pVlanConfig[i].if_type)
		{
			continue;
		}


		if(pVlanConfig[i].isWan==0)/*lan config*/
		{
			rtl865x_regist_aclChain(pVlanConfig[i].ifname, RTL865X_ACL_IPV6_USED, RTL865X_ACL_INGRESS);
			/*ping6 issue*/
			bzero((void*)&rule,sizeof(rtl865x_AclRule_t));
			rule.ruleType_ = RTL865X_ACL_MAC;
			rule.actionType_		= RTL865X_ACL_PERMIT;
			rule.pktOpApp_ 		= RTL865X_ACL_ALL_LAYER;
			rule.dstMac_.octet[0]=0x33;
			rule.dstMac_.octet[1]=0x33;
			rule.dstMac_.octet[2]=0xFF;

			rule.dstMacMask_.octet[0]=0xFF;
			rule.dstMacMask_.octet[1]=0xFF;
			rule.dstMacMask_.octet[2]=0xFF;

			ret= rtl865x_add_acl(&rule, pVlanConfig[i].ifname, RTL865X_ACL_IPV6_USED);

			/*ipv6 multicast data issue*/
			bzero((void*)&rule,sizeof(rtl865x_AclRule_t));
			rule.ruleType_ = RTL865X_ACL_MAC;
			rule.actionType_		= RTL865X_ACL_TOCPU;
			rule.pktOpApp_ 		= RTL865X_ACL_ALL_LAYER;
			rule.dstMac_.octet[0]=0x33;
			rule.dstMac_.octet[1]=0x33;
			rule.dstMac_.octet[2]=0x00;
			rule.dstMac_.octet[3]=0x00;
			rule.dstMac_.octet[4]=0x00;
			rule.dstMac_.octet[5]=0x00;

			rule.dstMacMask_.octet[0]=0xFF;
			rule.dstMacMask_.octet[1]=0xFF;

			ret= rtl865x_add_acl(&rule, pVlanConfig[i].ifname, RTL865X_ACL_IPV6_USED);

		}
		else/*wan config*/
		{
			rtl865x_regist_aclChain(pVlanConfig[i].ifname, RTL865X_ACL_IPV6_USED, RTL865X_ACL_INGRESS);
			/*ipv6 multicast data issue*/
			bzero((void*)&rule,sizeof(rtl865x_AclRule_t));
			rule.ruleType_ = RTL865X_ACL_MAC;
			rule.actionType_		= RTL865X_ACL_TOCPU;
			rule.pktOpApp_ 		= RTL865X_ACL_ALL_LAYER;
			rule.dstMac_.octet[0]=0x33;
			rule.dstMac_.octet[1]=0x33;
			rule.dstMac_.octet[2]=0x00;
			rule.dstMac_.octet[3]=0x00;
			rule.dstMac_.octet[4]=0x00;
			rule.dstMac_.octet[5]=0x00;

			rule.dstMacMask_.octet[0]=0xFF;
			rule.dstMacMask_.octet[1]=0xFF;

			ret= rtl865x_add_acl(&rule, pVlanConfig[i].ifname, RTL865X_ACL_IPV6_USED);

		}

		#if defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
		if(rtl865x_curOpMode == WISP_MODE)
		#endif
			rtl865x_reConfigDefaultAcl(pVlanConfig[i].ifname);

		
	}

	return SUCCESS;
}

static int rtl865x_removeAclForMldSnooping(struct rtl865x_vlanConfig* vlanConfig)
{
	int i;
	#if defined (CONFIG_RTL_MULTI_LAN_DEV) ||defined(CONFIG_RTK_VLAN_SUPPORT)
	struct rtl865x_vlanConfig tmpVlanConfig[NETIF_NUMBER];
	#endif

	struct rtl865x_vlanConfig *pVlanConfig=NULL;

	if(vlanConfig==NULL)
	{
		return FAILED;
	}
#if defined (CONFIG_RTL_MULTI_LAN_DEV) ||defined(CONFIG_RTK_VLAN_SUPPORT)
	re865x_packVlanConfig(vlanConfig, tmpVlanConfig);
	pVlanConfig=tmpVlanConfig;
#else
	pVlanConfig=vlanConfig;
#endif

	for(i=0; pVlanConfig[i].vid != 0; i++)
	{
		if (IF_ETHER!=pVlanConfig[i].if_type)
		{
			continue;
		}

		if(pVlanConfig[i].isWan==0)/*lan config*/
		{
			rtl865x_unRegist_aclChain(pVlanConfig[i].ifname, RTL865X_ACL_IPV6_USED, RTL865X_ACL_INGRESS);
		}
		else/*wan config*/
		{
			rtl865x_unRegist_aclChain(pVlanConfig[i].ifname, RTL865X_ACL_IPV6_USED, RTL865X_ACL_INGRESS);
		}

	}

#if defined (CONFIG_RTL_IPTABLES_RULE_2_ACL)

#else
#if defined (CONFIG_RTK_VLAN_SUPPORT)
		if(rtk_vlan_support_enable==0)
		{
			rtl865x_setDefACLForAllNetif(RTL865X_ACLTBL_PERMIT_ALL, RTL865X_ACLTBL_PERMIT_ALL, RTL865X_ACLTBL_PERMIT_ALL, RTL865X_ACLTBL_PERMIT_ALL);
		}
		else
		{
			rtl865x_setDefACLForAllNetif(RTL865X_ACLTBL_ALL_TO_CPU, RTL865X_ACLTBL_ALL_TO_CPU, RTL865X_ACLTBL_PERMIT_ALL, RTL865X_ACLTBL_PERMIT_ALL);
		}
#else
		rtl865x_setDefACLForAllNetif(RTL865X_ACLTBL_PERMIT_ALL, RTL865X_ACLTBL_PERMIT_ALL, RTL865X_ACLTBL_PERMIT_ALL, RTL865X_ACLTBL_PERMIT_ALL);
#endif
#endif

	return SUCCESS;
}
#endif


#if !defined(CONFIG_COMPAT_NET_DEV_OPS) && (defined(CONFIG_RTL_CUSTOM_PASSTHRU) || defined(CONFIG_RTL_STP))
static const struct net_device_ops rtl819x_pseudodev_ops = {
	.ndo_open		= re865x_pseudo_open,
	.ndo_stop		= re865x_pseudo_close,

	.ndo_get_stats	= re865x_get_stats,
	.ndo_do_ioctl		= re865x_ioctl,
	.ndo_start_xmit	= re865x_start_xmit,
};
#endif

int  __init re865x_probe (void)
{
/*2007-12-19*/
	int32 i, j;
	int32 totalVlans=((sizeof(vlanconfig))/(sizeof(struct rtl865x_vlanConfig)))-1;
#if defined (CONFIG_RTL_IGMP_SNOOPING)
	int32 retVal;
	int32 igmpInitFlag=FAILED;
	struct rtl_mCastSnoopingGlobalConfig mCastSnoopingGlobalConfig;
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	rtl865x_mCastConfig_t mCastConfig;
	#endif
#endif
#if defined(CONFIG_RTL_REPORT_LINK_STATUS)  ||  defined(CONFIG_RTK_VLAN_SUPPORT)
	struct proc_dir_entry *res_stats_root;
#endif
#if defined(CONFIG_RTL_819XD)&&defined(CONFIG_RTL_8211DS_SUPPORT)&&defined(CONFIG_RTL_8197D)
	uint32 reg_tmp=0;
#endif

#if defined(CONFIG_RTL_REPORT_LINK_STATUS)
	struct proc_dir_entry *rtk_link_status_entry;
#endif

#if defined(CONFIG_RTK_VLAN_SUPPORT)
	struct proc_dir_entry *res_stats;
	struct proc_dir_entry *rtk_vlan_support_entry;
#if defined(CONFIG_RTK_VLAN_NEW_FEATURE)
	struct proc_dir_entry *rtk_vlan_management_entry;
#endif
#if defined(CONFIG_819X_PHY_RW)//#if defined(CONFIG_RTK_VLAN_FOR_CABLE_MODEM)
	uint32 portnum;
	char port_mibEntry_name[10];
	struct proc_dir_entry *rtl_phy;
	struct proc_dir_entry *port_mibStats_root;
	struct proc_dir_entry *port_mibStats_entry;
#endif	//#if defined(CONFIG_819X_PHY_RW)
#endif

#if defined(CONFIG_RTL_HW_VLAN_SUPPORT)
	struct proc_dir_entry *rtl_hw_vlan_support_entry;
	struct proc_dir_entry *rtl_hw_vlan_tagged_bridge_multicast_entry;
	memset(hw_vlan_info, 0, PORT_NUMBER*sizeof(struct hw_vlan_port_setting));
#endif

#if defined (CONFIG_RTL_LOCAL_PUBLIC)
	struct rtl865x_interface_info ifInfo;
#endif

#if defined(PATCH_GPIO_FOR_LED)
	int port;
#endif

#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	struct proc_dir_entry *rtk_vlan_wan_tag_support_entry;
#endif

	//WRITE_MEM32(PIN_MUX_SEL_2, 0x7<<21);

	rtlglue_printf("\n\n\nProbing RTL8186 10/100 NIC-kenel stack size order[%d]...\n", THREAD_SIZE_ORDER);
    	REG32(CPUIIMR) = 0x00;
    	REG32(CPUICR) &= ~(TXCMD | RXCMD);
	rxMbufRing=NULL;

#if defined(CONFIG_RTL_819XD)
	if ((REG32(REVR) == 0x8197C000) || (REG32(REVR) == 0x8197C001))
	{
		rtl_port0Refined = 1;
	}
#endif

	/*Initial ASIC table*/
	FullAndSemiReset();
	{
		rtl8651_tblAsic_InitPara_t para;

		memset(&para, 0, sizeof(rtl8651_tblAsic_InitPara_t));

		/*
			For DEMO board layout, RTL865x platform define corresponding PHY setting and PHYID.
		*/

		rtl865x_wanPortMask = RTL865X_PORTMASK_UNASIGNED;

		INIT_CHECK(rtl865x_initAsicL2(&para));

#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER_L3
		INIT_CHECK(rtl865x_initAsicL3());
#endif
#if defined(CONFIG_RTL_LAYERED_ASIC_DRIVER_L4) && (defined(CONFIG_RTL_8198) ||defined(CONFIG_RTL_8196CT) ||defined(CONFIG_RTL_819XDT))
		INIT_CHECK(rtl865x_initAsicL4());
#endif

		/*
			Re-define the wan port according the wan port detection result.
			NOTE:
				There are a very strong assumption that if port5 was giga port,
				then wan port was port 5.
		*/
		if (RTL865X_PORTMASK_UNASIGNED==rtl865x_wanPortMask)
		{
			/* keep the original mask */
			assert(RTL865X_PORTMASK_UNASIGNED==rtl865x_lanPortMask);
			rtl865x_wanPortMask = RTL_WANPORT_MASK;
			rtl865x_lanPortMask = RTL_LANPORT_MASK;
		}
		else
		{
			/* redefine wan port mask */
			assert(RTL865X_PORTMASK_UNASIGNED!=rtl865x_lanPortMask);
			for(i=0;i<totalVlans;i++)
			{
				if (TRUE==vlanconfig[i].isWan)
				{
					vlanconfig[i].memPort = vlanconfig[i].untagSet = rtl865x_wanPortMask;
				}
				else
				{
					vlanconfig[i].memPort = vlanconfig[i].untagSet = rtl865x_lanPortMask;
				}
			}
		}
#if 1		/*	10/100 & giga use the same pre-allocated skb number */
		/*
			Re-define the pre-allocated skb number according the wan
			port detection result.
			NOTE:
				There are a very strong assumption that if port1~port4 were
				all giga port, then the sdram was 32M.
		*/
		{
			if (RTL865X_PREALLOC_SKB_UNASIGNED==rtl865x_maxPreAllocRxSkb)
			{
				assert(rtl865x_rxSkbPktHdrDescNum==
					rtl865x_txSkbPktHdrDescNum==
					RTL865X_PREALLOC_SKB_UNASIGNED);

				rtl865x_maxPreAllocRxSkb = MAX_PRE_ALLOC_RX_SKB;
				rtl865x_rxSkbPktHdrDescNum = NUM_RX_PKTHDR_DESC;
				rtl865x_txSkbPktHdrDescNum = NUM_TX_PKTHDR_DESC;
			}
			else
			{
				assert(rtl865x_rxSkbPktHdrDescNum!=RTL865X_PREALLOC_SKB_UNASIGNED);
				assert(rtl865x_txSkbPktHdrDescNum!=RTL865X_PREALLOC_SKB_UNASIGNED);
				/* Assigned value in function of rtl8651_initAsic() */
				rxRingSize[0] = rtl865x_rxSkbPktHdrDescNum;
				txRingSize[0] = rtl865x_txSkbPktHdrDescNum;
			}

			for(i=1;i<RTL865X_SWNIC_RXRING_HW_PKTDESC;i++)
			{
				rtl865x_maxPreAllocRxSkb += rxRingSize[i];
			}
		}
#else
		{
			rtl865x_maxPreAllocRxSkb = MAX_PRE_ALLOC_RX_SKB;
			rtl865x_rxSkbPktHdrDescNum = NUM_RX_PKTHDR_DESC;
			rtl865x_txSkbPktHdrDescNum = NUM_TX_PKTHDR_DESC;
		}
#endif
	}


#ifdef BR_SHORTCUT
	cached_dev=NULL;
#endif
#ifdef BR_SHORTCUT_C2
	cached_dev2=NULL;
#endif
	/*init PHY LED style*/
#if !defined(CONFIG_RTL_819XD) && !defined(CONFIG_RTL_8196E)
#if defined(CONFIG_RTL865X_BICOLOR_LED)
	#ifdef BICOLOR_LED_VENDOR_BXXX
	REG32(LEDCR) |= (1 << 19); // 5 ledmode set to 1 for bi-color LED
	REG32(PABCNR) &= ~0x001f0000; /* set port port b-4/3/2/1/0 to gpio */
	REG32(PABDIR) |=  0x001f0000; /* set port port b-4/3/2/1/0 gpio direction-output */
	#else
	//8650B demo board default: Bi-color 5 LED
	WRITE_MEM32(LEDCR, READ_MEM32(LEDCR) | 0x01180000 ); // bi-color LED
	#endif
	/* config LED mode */
	WRITE_MEM32(SWTAA, PORT5_PHY_CONTROL);
	WRITE_MEM32(TCR0, 0x000002C2); //8651 demo board default: 15 LED boards
	WRITE_MEM32(SWTACR, CMD_FORCE | ACTION_START); // force add
#else /* CONFIG_RTL865X_BICOLOR_LED */

	/* config LED mode */
#if defined(CONFIG_RTK_VOIP_BOARD)
		WRITE_MEM32(LEDCR, 0x00055500 ); // 15 LED
		//avoiv bad voip quality
		WRITE_MEM32(0xb8000048, REG32(0xb8000048)&0xfffffff3);
#else
	WRITE_MEM32(LEDCR, 0x00000000 ); // 15 LED
#endif
	WRITE_MEM32(SWTAA, PORT5_PHY_CONTROL);
	WRITE_MEM32(TCR0, 0x000002C7); //8651 demo board default: 15 LED boards
	WRITE_MEM32(SWTACR, CMD_FORCE | ACTION_START); // force add
#endif /* CONFIG_RTL865X_BICOLOR_LED */
#endif

/*2007-12-19*/
#if defined(CONFIG_RTK_VLAN_SUPPORT)
		//port based decision
	rtl865xC_setNetDecisionPolicy(NETIF_PORT_BASED);
	WRITE_MEM32(PLITIMR,0);

#endif

#if defined(CONFIG_RTL_819XD)&&defined(CONFIG_RTL_8211DS_SUPPORT)&&defined(CONFIG_RTL_8197D)
	rtl8651_getAsicEthernetPHYReg(0x6, 0, &reg_tmp);
	rtl_setPortMask(reg_tmp);
#endif

	INIT_CHECK(rtl865x_init());

#if defined (CONFIG_RTL_MULTI_LAN_DEV) || defined(CONFIG_RTK_VLAN_SUPPORT)
	re865x_packVlanConfig(vlanconfig, packedVlanConfig);
	INIT_CHECK(rtl865x_config(packedVlanConfig));
#else
	INIT_CHECK(rtl865x_config(vlanconfig));
#endif

	/* create all default VLANs */
//	rtlglue_printf("	creating eth0~eth%d...\n",totalVlans-1 );
#if defined (CONFIG_RTL_IGMP_SNOOPING)
	memset(&mCastSnoopingGlobalConfig, 0, sizeof(struct rtl_mCastSnoopingGlobalConfig));
	mCastSnoopingGlobalConfig.maxGroupNum=256;
	mCastSnoopingGlobalConfig.maxSourceNum=300;
	mCastSnoopingGlobalConfig.hashTableSize=64;

	mCastSnoopingGlobalConfig.groupMemberAgingTime=260;
	mCastSnoopingGlobalConfig.lastMemberAgingTime=2;
	mCastSnoopingGlobalConfig.querierPresentInterval=260;

	mCastSnoopingGlobalConfig.dvmrpRouterAgingTime=120;
	mCastSnoopingGlobalConfig.mospfRouterAgingTime=120;
	mCastSnoopingGlobalConfig.pimRouterAgingTime=120;

	igmpInitFlag=rtl_initMulticastSnooping(mCastSnoopingGlobalConfig);
#endif

	for(i=0;i<totalVlans;i++)
	{
		struct net_device *dev;
		struct dev_priv	  *dp;
		int rc;

		if (IF_ETHER!=vlanconfig[i].if_type)
		{
			continue;
		}
		dev = alloc_etherdev(sizeof(struct dev_priv));
		if (!dev) {
			printk("failed to allocate dev %d", i);
			return -1;
		}
		SET_MODULE_OWNER(dev);
		dp = dev->priv;
		memset(dp,0,sizeof(*dp));
		dp->dev = dev;
		dp->id = vlanconfig[i].vid;
		dp->portmask =  vlanconfig[i].memPort;
		dp->portnum  = 0;
		#if defined(CONFIG_RTK_VLAN_SUPPORT)
		dp->vlan_setting.is_lan = (dp->id!=RTL_WANVLANID);
		#endif
		for(j=0;j<RTL8651_AGGREGATOR_NUMBER;j++){
			if(dp->portmask & (1<<j))
				dp->portnum++;
		}

		memcpy((char*)dev->dev_addr,(char*)(&(vlanconfig[i].mac)),ETHER_ADDR_LEN);
#if defined(CONFIG_COMPAT_NET_DEV_OPS)
		dev->open = re865x_open;
		dev->stop = re865x_close;
		dev->set_multicast_list = re865x_set_rx_mode;
		#if defined(CONFIG_RTK_VLAN_NEW_FEATURE)
		if(memcmp((vlanconfig[i].ifname), RTL_DRV_LAN_P7_NETIF_NAME, 4) == 0){
			memcpy((char*)dev->name, (char*)(&(vlanconfig[i].ifname)), 5);
			dev->hard_start_xmit = rtl_bridge_wan_start_xmit;
		}
		else
		#endif
		dev->hard_start_xmit = re865x_start_xmit;
		dev->get_stats = re865x_get_stats;
		dev->do_ioctl = re865x_ioctl;
		dev->tx_timeout = re865x_tx_timeout;
		dev->set_mac_address = rtl865x_set_hwaddr;
		dev->change_mtu = rtl865x_set_mtu;
#if defined(CONFIG_RTL8186_LINK_CHANGE)
		dev->change_link = rtl865x_set_link;
#endif
#ifdef CP_VLAN_TAG_USED
		dev->features |= NETIF_F_HW_VLAN_TX | NETIF_F_HW_VLAN_RX;
		dev->vlan_rx_register = cp_vlan_rx_register;
		dev->vlan_rx_kill_vid = cp_vlan_rx_kill_vid;
#endif

#else
		#if defined(CONFIG_RTK_VLAN_NEW_FEATURE)
			if(memcmp((vlanconfig[i].ifname), RTL_DRV_LAN_P7_NETIF_NAME, 4) == 0){
				memcpy((char*)dev->name, (char*)(&(vlanconfig[i].ifname)), 5);
				dev->netdev_ops = &rtl819x_netdev_ops_bridge;
			}
			else
		#endif
			dev->netdev_ops = &rtl819x_netdev_ops;
#endif
		dev->watchdog_timeo = TX_TIMEOUT;
#if 0
		dev->features |= NETIF_F_SG | NETIF_F_IP_CSUM;
#endif
#ifdef CP_VLAN_TAG_USED
		dev->features |= NETIF_F_HW_VLAN_TX | NETIF_F_HW_VLAN_RX;
#endif

		dev->irq = BSP_SWCORE_IRQ;
		rc = register_netdev(dev);

		if(!rc){
			_rtl86xx_dev.dev[i]=dev;
			rtl_add_ps_drv_netif_mapping(dev,vlanconfig[i].ifname);
			/*2007-12-19*/
			rtlglue_printf("eth%d added. vid=%d Member port 0x%x...\n", i,vlanconfig[i].vid ,vlanconfig[i].memPort );
		}else
			rtlglue_printf("Failed to allocate eth%d\n", i);

#if defined(CONFIG_RTK_VLAN_SUPPORT) || defined(CONFIG_RTL_REPORT_LINK_STATUS)
		res_stats_root = proc_mkdir(dev->name, NULL);
		if (res_stats_root == NULL)
		{
			printk("proc_mkdir failed!\n");
		}
#endif

#if defined(CONFIG_RTK_VLAN_SUPPORT)
		if ((res_stats = create_proc_read_entry("mib_vlan", 0644, res_stats_root,
			read_proc_vlan, (void *)dev)) == NULL)
		{
			printk("create_proc_read_entry failed!\n");
		}
		res_stats->write_proc = write_proc_vlan;

#endif

#if defined(CONFIG_RTL_REPORT_LINK_STATUS)
		/*FIXME if mutliple-WAN*/
		wan_linkStatus[0]=0;
		rtk_link_status_entry=create_proc_entry("up_event",0,res_stats_root);
		if(rtk_link_status_entry)
		{
			rtk_link_status_entry->read_proc=rtk_link_status_read;
		    	rtk_link_status_entry->write_proc=rtk_link_status_write;
		}
#endif


	}

#if defined(CONFIG_RTL_MULTIPLE_WAN)
	rtl_config_multipleWan_netif(RTL_MULTIWAN_ADD);
	rtl865x_addMultiCastNetif();
	rtl_regist_multipleWan_dev();
#endif

#if defined (CONFIG_RTL_IGMP_SNOOPING)
	retVal=rtl_registerIgmpSnoopingModule(&nicIgmpModuleIndex);
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(retVal==SUCCESS)
	{
		rtl_multicastDeviceInfo_t devInfo;
		memset(&devInfo, 0 , sizeof(rtl_multicastDeviceInfo_t));
		strcpy(devInfo.devName, "eth*");
		for(i=0;i<totalVlans;i++)
		{
			if( vlanconfig[i].if_type==IF_ETHER)
			{
				devInfo.portMask|=vlanconfig[i].memPort;
			}
		}
		devInfo.swPortMask=devInfo.portMask & (~ ((1<<RTL8651_MAC_NUMBER)-1));
		rtl_setIgmpSnoopingModuleDevInfo(nicIgmpModuleIndex, &devInfo);
	}
	#endif
	rtl_setIpv4UnknownMCastFloodMap(nicIgmpModuleIndex, 0xFFFFFFFF);
	rtl_setIpv6UnknownMCastFloodMap(nicIgmpModuleIndex, 0xFFFFFFFF);

	curLinkPortMask=rtl865x_getPhysicalPortLinkStatus();

	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	memset(&mCastConfig, 0, sizeof(rtl865x_mCastConfig_t));
	for(i=0;i<totalVlans;i++)
	{
		if (TRUE==vlanconfig[i].isWan)
		{
			mCastConfig.externalPortMask |=vlanconfig[i].memPort;
		}
	}
	rtl865x_initMulticast(&mCastConfig);

	#endif
#endif

#ifdef CONFIG_RTL_STP
	printk("Configuration ether driver to process port 0 ~ port %d for Spanning tree process\n",MAX_RE865X_ETH_STP_PORT-1);
	//Initial: disable Realtek Hardware STP
	rtl865x_setSpanningEnable(FALSE);

	for ( i = 0 ; i < MAX_RE865X_ETH_STP_PORT; i ++ )
	{
		struct net_device *dev;
		struct dev_priv *dp;
		int rc;
		struct re865x_priv *rp;

		rp = &_rtl86xx_dev;
		dev = alloc_etherdev(sizeof(struct dev_priv));
		if (!dev){
			rtlglue_printf("failed to allocate dev %d", i);
			return -1;
		}
		strcpy(dev->name, "port%d");
		memcpy((char*)dev->dev_addr,(char*)(&(vlanconfig[0].mac)),ETHER_ADDR_LEN);
		dp = dev->priv;
		memset(dp,0,sizeof(*dp));
		dp->dev = dev;
#ifdef CONFIG_COMPAT_NET_DEV_OPS
		dev->open = re865x_pseudo_open;
		dev->stop = re865x_pseudo_close;
		dev->set_multicast_list = NULL;
		dev->hard_start_xmit = re865x_start_xmit;
		dev->get_stats = re865x_get_stats;
		dev->do_ioctl = re865x_ioctl;
		dev->tx_timeout = NULL;
#else
		dev->netdev_ops = &rtl819x_pseudodev_ops;
#endif
		dev->watchdog_timeo = TX_TIMEOUT;
		dev->irq = 0;				/* virtual interfaces has no IRQ allocated */
		rc = register_netdev(dev);
		if (rc == 0)
		{
			_rtl86xx_dev.stp_port[i] = dev;
			printk("=> [stp pseudo port%d] done\n", i);
		} else
		{
			printk("=> Failed to register [stp pseudo port%d]", i);
			return -1;
		}
	}

	re865x_stp_mapping_init();

#endif
#if defined(CONFIG_RTL_HW_STP)
	//Initial: disable Realtek Hardware STP
	rtl865x_setSpanningEnable(FALSE);
#endif

	((struct dev_priv*)((_rtl86xx_dev.dev[0])->priv))->dev_next = _rtl86xx_dev.dev[1];
	((struct dev_priv*)((_rtl86xx_dev.dev[1])->priv))->dev_prev = _rtl86xx_dev.dev[0];

#if defined(CONFIG_RTL_ETH_PRIV_SKB)
	init_priv_eth_skb_buf();
#endif

#if (defined(CONFIG_RTL_CUSTOM_PASSTHRU) && !defined(CONFIG_RTL8196_RTL8366))
	//cary
	rtl8651_customPassthru_init();
#endif
	rtl8651_initStormCtrl();

	rtl819x_eee_proc_init();

#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
	// initial proc for phyRegTest
	phyRegTest_init();
#endif

#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER_L3
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	rtl8651_setAsicMulticastEnable(TRUE);
#else
	rtl8651_setAsicMulticastEnable(FALSE);
#endif
#endif

#if defined(CONFIG_RTK_VLAN_SUPPORT)

	rtk_vlan_support_enable= 0;
	rtk_vlan_support_entry=create_proc_entry("rtk_vlan_support",0,NULL);
	if (rtk_vlan_support_entry)
	{
	    rtk_vlan_support_entry->read_proc=rtk_vlan_support_read;
	    rtk_vlan_support_entry->write_proc=rtk_vlan_support_write;
	}

#if defined(CONFIG_RTK_VLAN_NEW_FEATURE)
	memset(&management_vlan, 0, sizeof(struct vlan_info*));
	rtk_vlan_management_entry = create_proc_entry("rtk_vlan_management_entry", 0, NULL);
	if(rtk_vlan_management_entry)
	{
		rtk_vlan_management_entry->read_proc=rtk_vlan_management_read;
		rtk_vlan_management_entry->write_proc=rtk_vlan_management_write;
	}
#endif
#endif

#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	rtk_vlan_wan_tag_support_entry = create_proc_entry("rtk_vlan_wan_tag",0,NULL);
	if (rtk_vlan_wan_tag_support_entry)
	{
	    rtk_vlan_wan_tag_support_entry->read_proc=rtk_vlan_wan_tag_support_read;
	    rtk_vlan_wan_tag_support_entry->write_proc=rtk_vlan_wan_tag_support_write;
	}
#endif

#if defined(CONFIG_RTL_HW_VLAN_SUPPORT)
	rtl_hw_vlan_support_entry = create_proc_entry("rtl_hw_vlan_support", 0, NULL);
	if (rtl_hw_vlan_support_entry)
	{
	    rtl_hw_vlan_support_entry->read_proc=rtl_hw_vlan_support_read;
	    rtl_hw_vlan_support_entry->write_proc=rtl_hw_vlan_support_write;
	}
	rtl_hw_vlan_tagged_bridge_multicast_entry = create_proc_entry("rtl_hw_vlan_tagged_mc", 0, NULL);
	if (rtl_hw_vlan_tagged_bridge_multicast_entry)
	{
	    rtl_hw_vlan_tagged_bridge_multicast_entry->read_proc=rtl_hw_vlan_tagged_bridge_multicast_read;
	    rtl_hw_vlan_tagged_bridge_multicast_entry->write_proc=rtl_hw_vlan_tagged_bridge_multicast_write;
	}
#endif


#if defined(CONFIG_819X_PHY_RW)
//#if defined(CONFIG_RTK_VLAN_FOR_CABLE_MODEM)
	rtl_phy = create_proc_entry("rtl_phy_status",0,NULL);
	if(rtl_phy)
	{
		rtl_phy->read_proc = rtl_phy_status_read;
		rtl_phy->write_proc = rtl_phy_status_write;
	}
	port_mibStats_root = proc_mkdir("ethPort_mibStats", NULL);
	if (port_mibStats_root == NULL)
	{
		printk("proc_mkdir failed!\n");
	}
	for(portnum=0; portnum<CPU; portnum++)
	{
		sprintf(&port_mibEntry_name[0], "port%u", portnum);
		port_mibStats_entry = create_proc_entry(port_mibEntry_name, 0, port_mibStats_root);
		port_mibStats_entry->data = (void *)portnum;
		if(port_mibStats_entry)
		{
			port_mibStats_entry->read_proc = port_mibStats_read_proc;
			port_mibStats_entry->write_proc = port_mibStats_write_proc;
		}
	}
#endif	//#if defined(CONFIG_819X_PHY_RW)

#if defined (CONFIG_RTL_LOCAL_PUBLIC)
	rtl865x_initLocalPublic(NULL);

	memset(&ifInfo, 0 , sizeof(struct rtl865x_interface_info));
#if defined(CONFIG_RTL_PUBLIC_SSID)
	strcpy(ifInfo.ifname,RTL_GW_WAN_DEVICE_NAME);
#else
	strcpy(ifInfo.ifname, RTL_DRV_WAN0_NETIF_NAME);
#endif
	ifInfo.isWan=1;
	for(i=0;i<totalVlans;i++)
	{
		if ((TRUE==vlanconfig[i].isWan) && (vlanconfig[i].if_type==IF_ETHER))
		{
			ifInfo.memPort |= vlanconfig[i].memPort;
			ifInfo.fid=vlanconfig[i].fid;
		}
	}
	rtl865x_setLpIfInfo(&ifInfo);

	memset(&ifInfo, 0 , sizeof(struct rtl865x_interface_info));
	strcpy(ifInfo.ifname,RTL_DRV_LAN_NETIF_NAME);
	ifInfo.isWan=0;
	for(i=0;i<totalVlans;i++)
	{
		if ((FALSE==vlanconfig[i].isWan) && (vlanconfig[i].if_type==IF_ETHER))
		{
			ifInfo.memPort|=vlanconfig[i].memPort;
			ifInfo.fid=vlanconfig[i].fid;
		}
	}
	rtl865x_setLpIfInfo(&ifInfo);
#endif
	rtl_rxTxDoneCnt=0;
	atomic_set(&rtl_devOpened, 0);

#if defined(PATCH_GPIO_FOR_LED)
	for (port=0; port<RTL8651_PHY_NUMBER; port++)
		init_led_ctrl(port);
#endif

#if defined(CONFIG_RTL_LINKSTATE)
	initPortStateCtrl();
#endif

#if defined (CONFIG_RTL_MLD_SNOOPING)
	rtl8651_initMldSnooping();
#endif
#if defined (CONFIG_RTL_PHY_POWER_CTRL)
	rtl865x_initPhyPowerCtrl();
#endif

#if defined(RTL_CPU_QOS_ENABLED)
	highPrioRxTryCnt = MAX_HIGH_PRIO_TRY;
	highestPriority = 0;
	cpuQosHoldLow = 0;
	totalLowQueueCnt = 0;
	memset(pktQueueByPri, 0, sizeof(rtl_queue_entry)*(RTL865X_SWNIC_RXRING_MAX_PKTDESC));
#endif

	rtl865x_config_callback_for_get_drv_netifName(rtl_get_ps_drv_netif_mapping_by_psdev_name);
#if defined (CONFIG_RTL_REINIT_SWITCH_CORE)
	rtl865x_creatReInitSwitchCoreProc();
#endif

	#if defined(RX_TASKLET)
	rtl_rx_tasklet_running=0;
	#endif
	#if defined(TX_TASKLET)
	rtl_tx_tasklet_running=0;
	#endif
#if defined (CONFIG_RTL_SOCK_DEBUG)
	rtl865x_creatSockDebugProc();
#endif

#if 0//def CONFIG_RTL_ULINKER //led
	REG32(PIN_MUX_SEL_2) = REG32(PIN_MUX_SEL_2) | 0x00003000;
	REG32(PABCD_CNR)     = REG32(PABCD_CNR) & ~(0x00004000);
	REG32(PABCD_DIR)     = REG32(PABCD_DIR) | 0x00004000;
	REG32(PABCD_DAT)     = REG32(PABCD_DAT) & ~(0x00004000);
#endif

	memset(&rx_skb_queue, 0, sizeof(struct ring_que));

	return 0;
}

#if defined(CONFIG_RTL_ETH_PRIV_SKB)

//---------------------------------------------------------------------------
static void init_priv_eth_skb_buf(void)
{
	int i;

	DEBUG_ERR("Init priv skb.\n");
	memset(eth_skb_buf, '\0', sizeof(struct priv_skb_buf2)*(MAX_ETH_SKB_NUM));
	INIT_LIST_HEAD(&eth_skbbuf_list);
	eth_skb_free_num=MAX_ETH_SKB_NUM;

	for (i=0; i<MAX_ETH_SKB_NUM; i++)  {
		memcpy(eth_skb_buf[i].magic, ETH_MAGIC_CODE, ETH_MAGIC_LEN);
		eth_skb_buf[i].buf_pointer = (void*)(&eth_skb_buf[i]);
		INIT_LIST_HEAD(&eth_skb_buf[i].list);
		list_add_tail(&eth_skb_buf[i].list, &eth_skbbuf_list);
	}
}

static __inline__ unsigned char *get_buf_from_poll(struct list_head *phead, unsigned int *count)
{
	unsigned long flags;
	unsigned char *buf;
	struct list_head *plist;

	local_irq_save(flags);

	if (list_empty(phead)) {
		local_irq_restore(flags);
		DEBUG_ERR("eth_drv: phead=%X buf is empty now!\n", (unsigned int)phead);
		DEBUG_ERR("free count %d\n", *count);
		return NULL;
	}

	if (*count == 1) {
		local_irq_restore(flags);
		DEBUG_ERR("eth_drv: phead=%X under-run!\n", (unsigned int)phead);
		return NULL;
	}

	*count = *count - 1;
	plist = phead->next;
	list_del_init(plist);
	buf = (unsigned char *)((unsigned int)plist + sizeof (struct list_head));
	local_irq_restore(flags);
	return buf;
}

static __inline__ void release_buf_to_poll(unsigned char *pbuf, struct list_head	*phead, unsigned int *count)
{
	unsigned long flags;
	struct list_head *plist;

	local_irq_save(flags);

	*count = *count + 1;
	plist = (struct list_head *)((unsigned int)pbuf - sizeof(struct list_head));
	list_add_tail(plist, phead);
	local_irq_restore(flags);
}

__IRAM_GEN void free_rtl865x_eth_priv_buf(unsigned char *head)
{
	#ifdef DELAY_REFILL_ETH_RX_BUF
	if (FAILED==return_to_rx_pkthdr_ring(head))
	#endif
	{release_buf_to_poll(head, &eth_skbbuf_list, (unsigned int *)&eth_skb_free_num);}
}

__MIPS16
__IRAM_FWD
static struct sk_buff *dev_alloc_skb_priv_eth(unsigned int size)
{
	struct sk_buff *skb;
	unsigned char *data;

	/* first argument is not used */
	if(eth_skb_free_num>0)
	{
		data = get_buf_from_poll(&eth_skbbuf_list, (unsigned int *)&eth_skb_free_num);
		if (data == NULL) {
			DEBUG_ERR("eth_drv: priv_skb buffer empty!\n");
			return NULL;
		}

		skb = dev_alloc_8190_skb(data, size);

		if (skb == NULL) {
			//free_rtl865x_eth_priv_buf(data);
			release_buf_to_poll(data, &eth_skbbuf_list, (unsigned int *)&eth_skb_free_num);
			DEBUG_ERR("alloc linux_skb buff failed!\n");
			return NULL;
		}
		return skb;
	}

	return NULL;
}

__MIPS16
__IRAM_FWD
int is_rtl865x_eth_priv_buf(unsigned char *head)
{
	unsigned long offset = (unsigned long)(&((struct priv_skb_buf2 *)0)->buf);
	struct priv_skb_buf2 *priv_buf = (struct priv_skb_buf2 *)(((unsigned long)head) - offset);

	if ((!memcmp(priv_buf->magic, ETH_MAGIC_CODE, ETH_MAGIC_LEN)) &&
		(priv_buf->buf_pointer==(void*)(priv_buf))) {
		return 1;
	}
	else {
		return 0;
	}
}

#if defined(CONFIG_RTL_ETH_PRIV_SKB) || defined(CONFIG_NET_WIRELESS_AGN) || defined(CONFIG_NET_WIRELESS_AG)
struct sk_buff *priv_skb_copy(struct sk_buff *skb)
{
	struct sk_buff *n;
	unsigned long flags;

	if (rx_skb_queue.qlen == 0) {
		n = dev_alloc_skb_priv_eth(CROSS_LAN_MBUF_LEN);
	}
	else {
		#if defined(RTK_QUE)
		local_irq_save(flags);
		n = rtk_dequeue(&rx_skb_queue);
		local_irq_restore(flags);
		#else
		n = __skb_dequeue(&rx_skb_queue);
		#endif
	}

	if (n == NULL) {
		return NULL;
	}

	/* Set the tail pointer and length */
	skb_put(n, skb->len);
	n->csum = skb->csum;
	n->ip_summed = skb->ip_summed;
	memcpy(n->data, skb->data, skb->len);

	copy_skb_header(n, skb);
	return n;
}
EXPORT_SYMBOL(priv_skb_copy);
#endif // defined(CONFIG_NET_WIRELESS_AGN) || defined(CONFIG_NET_WIRELESS_AG)
#endif // CONFIG_RTL_ETH_PRIV_SKB

static void __exit re865x_exit (void)
{

#if defined(CONFIG_RTL_PROC_DEBUG)||defined(CONFIG_RTL_DEBUG_TOOL)

	rtl865x_proc_debug_cleanup();
#endif


#if defined(CONFIG_PROC_FS) && defined(CONFIG_NET_SCHED) && defined(CONFIG_RTL_LAYERED_DRIVER)
#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
	rtl865x_exitOutputQueue();
#endif
#endif

#if defined (CONFIG_RTL_IGMP_SNOOPING)
	rtl_exitMulticastSnooping();
#endif

#if defined(CONFIG_RTL_LINKSTATE)
	exitPortStateCtrl();
#endif
	return;
}

#ifdef CONFIG_RTK_FAKE_ETH

int32 rtl8651_EthernetPowerDown(void)
{
 uint32 statCtrlReg0;
 int i;

 //## from rtl8651_setAllAsicEthernetPHYPowerDown
 for (i=0; i<5; i++) {

  /* read current PHY reg 0 value */
  rtl8651_getAsicEthernetPHYReg( i, 0, &statCtrlReg0 );

  REG32(PCRP0+(i*4)) |= EnForceMode;
  statCtrlReg0 |= POWER_DOWN;

  /* write PHY reg 0 */
  rtl8651_setAsicEthernetPHYReg( i, 0, statCtrlReg0 );
 }
 //#######################################

 //then set bit 9 of 0xb800-0010 to 0. deactive switch core

 REG32(SYS_CLK_MAG)&=(~(SYS_SW_CLK_ENABLE));

 return SUCCESS;
}

int re865x_ioctl_fake (struct net_device *dev, struct ifreq *rq, int cmd)
{
	int32 rc = 0;
	unsigned long *data;
	int32 args[4];
	int32  * pRet;

	data = (unsigned long *)rq->ifr_data;

	if (copy_from_user(args, data, 4*sizeof(unsigned long)))
	{
		return -EFAULT;
	}

	switch (args[0])
	{
		case RTL8651_IOCTL_GETWANLINKSTATUS:
			{
				pRet = (int32 *)args[3];
				*pRet = FAILED;
				rc = SUCCESS;

				break;
			}

		case RTL8651_IOCTL_GETWANLINKSPEED:
			{
				int wanPortMask;

				pRet = (int32 *)args[3];
				*pRet = FAILED;
				rc = SUCCESS;

				wanPortMask = 0;

				/* no wan port exist */
				break;
			}
		default:
			rc = SUCCESS;
			break;
	}

	return rc;

}


static int rtl865x_set_mtu_fake(struct net_device *dev, int new_mtu)
{
	unsigned long flags;

 	local_irq_save(flags);
	dev->mtu = new_mtu;

	local_irq_restore(flags);

	return SUCCESS;
}

static int rtl865x_set_hwaddr_fake(struct net_device *dev, void *addr)
{
	unsigned long flags;
	int i;
	unsigned char *p;

	p = ((struct sockaddr *)addr)->sa_data;
 	local_irq_save(flags);

	for (i = 0; i<ETHER_ADDR_LEN; ++i) {
		dev->dev_addr[i] = p[i];
	}

	local_irq_restore(flags);
	return SUCCESS;
}

static int re865x_start_xmit_fake(struct sk_buff *skb, struct net_device *dev)
{

	dev_kfree_skb_any(skb);
	return 0;

}

static int re865x_open_fake (struct net_device *dev)
{
	struct dev_priv *cp;

	cp = dev->priv;
	//cp = netdev_priv(dev);
	if (cp->opened)
		return SUCCESS;

	cp->opened = 1;
	netif_start_queue(dev);
	return SUCCESS;
}


static int re865x_close_fake (struct net_device *dev)
{
	struct dev_priv *cp;

	cp = dev->priv;
//	cp = netdev_priv(dev);

	if (!cp->opened)
		return SUCCESS;
	netif_stop_queue(dev);

//	memset(&cp->net_stats, '\0', sizeof(struct net_device_stats));
	cp->opened = 0;

#ifdef BR_SHORTCUT
	if (dev == cached_dev)
		cached_dev=NULL;
#endif
	return SUCCESS;
}

#if defined(CONFIG_COMPAT_NET_DEV_OPS)
#else
static const struct net_device_ops rtl819x_netdev_ops_fake = {
	.ndo_open		= re865x_open_fake,
	.ndo_stop		= re865x_close_fake,
//	.ndo_validate_addr	= eth_validate_addr_fake,
	.ndo_set_mac_address 	= rtl865x_set_hwaddr_fake,
//	.ndo_set_multicast_list	= re865x_set_rx_mode_fake,
//	.ndo_get_stats		= re865x_get_stats_fake,
	.ndo_do_ioctl		= re865x_ioctl_fake,
	.ndo_start_xmit		= re865x_start_xmit_fake,
//	.ndo_tx_timeout		= re865x_tx_timeout_fake,
//#if defined(CP_VLAN_TAG_USED)
//	.ndo_vlan_rx_register	= cp_vlan_rx_register,
//endif
	.ndo_change_mtu		= rtl865x_set_mtu_fake,

};
#endif


int  __init re865x_probe_fake (void)
{
	struct net_device *dev;
    struct dev_priv	  *dp;
	int rc;

	dev = alloc_etherdev(sizeof(struct dev_priv));
	if (!dev) {
		printk("failed to allocate dev %d", 0);
		return -1;
	}

    dp = dev->priv;
	memset(dp,0,sizeof(*dp));
	dp->dev = dev;

	memcpy((char*)dev->dev_addr,(char*)(&(vlanconfig[0].mac)),ETHER_ADDR_LEN); //mark_FIXME.

#if defined(CONFIG_COMPAT_NET_DEV_OPS)
		dev->open = re865x_open_fake;
		dev->stop = re865x_close_fake;
		//dev->set_multicast_list = re865x_set_rx_mode;
		dev->hard_start_xmit = re865x_start_xmit_fake;
		//dev->get_stats = re865x_get_stats;
		dev->do_ioctl = re865x_ioctl_fake;
		//dev->tx_timeout = re865x_tx_timeout;
		dev->set_mac_address = rtl865x_set_hwaddr_fake;
		dev->change_mtu = rtl865x_set_mtu_fake;
#else
		dev->netdev_ops = &rtl819x_netdev_ops_fake;
#endif

	dev->watchdog_timeo = TX_TIMEOUT;

	rc = register_netdev(dev);

#ifdef BR_SHORTCUT
	cached_dev=NULL;
#endif

	if(rc)
		rtlglue_printf("Failed to allocate eth%d\n", 0);

	rc = rtl8651_EthernetPowerDown();

	return 0;
}
static void __exit re865x_exit_fake (void)
{
	return ;
}
#endif

#ifdef CONFIG_RTK_FAKE_ETH
module_init(re865x_probe_fake);
module_exit(re865x_exit_fake);
#else
module_init(re865x_probe);
module_exit(re865x_exit);
#endif

/*
@func enum RTL_RESULT | rtl865x_init | Initialize light rome driver and RTL865x ASIC.
@rvalue RTL_SUCCESS | Initial success.
@comm
	Its important to call this API before using the driver. Note taht you can not call this API twice !
*/
int32 rtl865x_init(void)
{
	int32 retval = 0;


	__865X_Config = 0;

#ifdef CONFIG_RTL8196_RTL8366
	/*	configure 8366 */
	{
		int ret;
		int i;
		rtl8366rb_phyAbility_t phy;

		REG32(PEFGHCNR_REG) = REG32(PEFGHCNR_REG)& (~(1<<11) ); //set byte F GPIO3 = gpio
		REG32(PEFGHDIR_REG) = REG32(PEFGHDIR_REG) | (1<<11);  //0 input, 1 output, set F bit 3 output
		REG32(PEFGHDAT_REG) = REG32(PEFGHDAT_REG) |( (1<<11) ); //F3 GPIO
		mdelay(150);

		ret = smi_init(GPIO_PORT_F, 2, 1);
		ret = rtl8366rb_initChip();
		ret = rtl8366rb_initVlan();
		ret = smi_write(0x0f09, 0x0020);
		ret = smi_write(0x0012, 0xe0ff);

		memset(&phy, 0, sizeof(rtl8366rb_phyAbility_t));
		phy.Full_1000 = 1;
		phy.Full_100 = 1;
		phy.Full_10 = 1;
		phy.Half_100 = 1;
		phy.Half_10 = 1;
		phy.FC = 1;
		phy.AsyFC = 1;
		phy.AutoNegotiation = 1;
		for(i=0;i<5;i++)
		{
			ret = rtl8366rb_setEthernetPHY(i,&phy);
		}
	}

 	 REG32(0xb8010000)=REG32(0xb8010000)&(0x20000000);
        REG32(0xbb80414c)=0x00037d16;
        REG32(0xbb804100)=1;
        REG32(0xbb804104)=0x00E80367;
#endif

/*common*/
	retval = rtl865x_initNetifTable();
	retval = rtl865x_initVlanTable();
#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
	retval = rtl865x_init_acl();
#endif
	retval = rtl865x_initEventMgr(NULL);

/*l2*/
 #ifdef CONFIG_RTL_LAYERED_DRIVER_L2
	retval = rtl865x_layer2_init();
 #endif


/*layer3*/
#ifdef CONFIG_RTL_LAYERED_DRIVER_L3
	retval = rtl865x_initIpTable();
	retval = rtl865x_initPppTable();
	retval = rtl865x_initRouteTable();
	retval = rtl865x_initNxtHopTable();
	retval = rtl865x_arp_init();
#endif

/*layer4*/
#if defined(CONFIG_RTL_LAYERED_DRIVER_L4) && (defined(CONFIG_RTL_8198) ||defined(CONFIG_RTL_8196CT) ||defined(CONFIG_RTL_819XDT))
	rtl865x_nat_init();
#endif

 	/*queue id & rx ring descriptor mapping*/
 	/*queue id & rx ring descriptor mapping*/
 	REG32(CPUQDM0)=QUEUEID1_RXRING_MAPPING|(QUEUEID0_RXRING_MAPPING<<16);
	REG32(CPUQDM2)=QUEUEID3_RXRING_MAPPING|(QUEUEID2_RXRING_MAPPING<<16);
	REG32(CPUQDM4)=QUEUEID5_RXRING_MAPPING|(QUEUEID4_RXRING_MAPPING<<16);

	rtl8651_setAsicOutputQueueNumber(CPU, RTL_CPU_RX_RING_NUM);

/*CONFIG_RTL_PROC_DEBUG:marco forproc debug. CONFIG_RTL_DEBUG_TOOL: marco for debug tool*/
#if defined(CONFIG_RTL_PROC_DEBUG)||defined(CONFIG_RTL_DEBUG_TOOL)
	rtl865x_proc_debug_init();
#endif

#if defined(PATCH_GPIO_FOR_LED)
	rtl8651_resetAllAsicMIBCounter();
#endif

	rtl_ps_drv_netif_mapping_init();

	return SUCCESS;
}



/*
@func enum RTL_RESULT | rtl865x_config | Configure light rome driver. Create VLAN and Network interface.
@parm struct rtl865x_vlanConfig * | vlanconfig |
@rvlaue RTL_SUCCESS | Sucessful configuration.
@rvalue RTL_INVVID | Invalid VID.
@comm
	struct rtl865x_vlanConfig is defined as follows:

			ifname:		Layer 3 Network Interface name, eg: eth0, eth1, ppp0...etc,. If it is specified, both layer 2 vlan and layer 3
						netwrok interface are created and bound together. It also can be a NULL value. In this case, only a layer 2 VLAN
						is created.
			isWan:		1 for WAN interface and 0 for LAN interface in a layer 4 mode.
			if_type:		IF_ETHER sets a network interface to be ETHER type. Instead, IF_PPPOE sets a netwrok to be PPPoE type.
						This field is meaningful only when the ifname is specified.
			vid:			VLAN ID to create a vlan.
			memPort:		VLAN member port.
			untagSet:	VLAN untag Set.
			mtu:			MTU.
			mac:		MAC address of the VLAN or network interface.
	eg1:

	struct rtl865x_vlanConfig vlanconfig[] = {
		{ 	"eth0",	 1,   IF_ETHER, 	8, 	   1, 	0x01, 		0x01,		1500, 	{ { 0x00, 0x00, 0xda, 0xcc, 0xcc, 0x08 } }	},
		{	"eth1",	 0,   IF_ETHER,	9,	   1,		0x1e,		0x1e,		1500,	{ { 0x00, 0x00, 0xda, 0xcc, 0xcc, 0x09 } }	},

		LRCONFIG_END,
	}
*/
int32 rtl865x_config(struct rtl865x_vlanConfig vlanconfig[])
{
	uint16 pvid;
	int32 i, j;
	int32 retval = 0;
	uint32 valid_port_mask = 0;

	if (!vlanconfig[0].vid)
		return RTL_EINVALIDVLANID;

	INIT_CHECK(rtl8651_setAsicOperationLayer(2));

	for(i=0; vlanconfig[i].vid != 0; i++)
	{
		rtl865x_netif_t netif;

		if(vlanconfig[i].memPort == 0)
			continue;

		valid_port_mask = vlanconfig[i].memPort;
		if(vlanconfig[i].isWan == 0)
			valid_port_mask |= 0x100;

		/*add vlan*/
		retval = rtl865x_addVlan(vlanconfig[i].vid);

		if(retval == SUCCESS)
		{
			rtl865x_addVlanPortMember(vlanconfig[i].vid,vlanconfig[i].memPort & valid_port_mask);
			rtl865x_setVlanFilterDatabase(vlanconfig[i].vid,vlanconfig[i].fid);
		}
		/*add network interface*/
		memset(&netif, 0, sizeof(rtl865x_netif_t));
		memcpy(netif.name,vlanconfig[i].ifname,MAX_IFNAMESIZE);
		memcpy(netif.macAddr.octet,vlanconfig[i].mac.octet,ETHER_ADDR_LEN);
		netif.mtu = vlanconfig[i].mtu;
		netif.if_type = vlanconfig[i].if_type;
		netif.vid = vlanconfig[i].vid;
		netif.is_wan = vlanconfig[i].isWan;
		netif.is_slave = vlanconfig[i].is_slave;
		#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		netif.enableRoute=1;
		#endif
		retval = rtl865x_addNetif(&netif);

		if(netif.is_slave == 1)
#if defined(CONFIG_RTL_PUBLIC_SSID)
			rtl865x_attachMasterNetif(netif.name,RTL_GW_WAN_DEVICE_NAME);
#else
			rtl865x_attachMasterNetif(netif.name, RTL_DRV_WAN0_NETIF_NAME);
#endif
#if defined(CONFIG_PROC_FS) && defined(CONFIG_NET_SCHED) && defined(CONFIG_RTL_LAYERED_DRIVER)
		memcpy(&netIfName[i][0], vlanconfig[i].ifname, sizeof(vlanconfig[i].ifname));
#endif

#if defined (CONFIG_RTL_UNKOWN_UNICAST_CONTROL)
		if (vlanconfig[i].isWan==0)
			memcpy(lanIfName, vlanconfig[i].ifname, sizeof(vlanconfig[i].ifname));
#endif
		if(retval != SUCCESS && retval != RTL_EVLANALREADYEXISTS)
			return retval;

	}

	/*this is a one-shot config*/
	if ((++__865X_Config) == 1)
	{
		for(i=0; i<RTL8651_PORT_NUMBER + 3; i++)
		{
			/* Set each port's PVID */
			for(j=0,pvid=0; vlanconfig[j].vid != 0; j++)
			{
				if ( (1<<i) & vlanconfig[j].memPort )
				{
					pvid = vlanconfig[j].vid;
					break;
				}
			}

			if (pvid!=0)
			{
	#ifdef CONFIG_HARDWARE_NAT_DEBUG
	/*2007-12-19*/
			rtlglue_printf("%s:%d:lrconfig[j].vid is %d,pvid is %d, j is %d,i is %d\n",__FUNCTION__,__LINE__,vlanconfig[j].vid,pvid,j, i);
	#endif

			CONFIG_CHECK(rtl8651_setAsicPvid(i, pvid));
	#if defined(CONFIG_RTK_VLAN_SUPPORT)
			rtl865x_setPortToNetif(vlanconfig[j].ifname,i);
	#endif
			}
		}
	}

	#if defined (CONFIG_RTL_HW_QOS_SUPPORT)
	rtl865x_initOutputQueue((uint8 **)netIfName);
	#endif

	#ifdef CONFIG_RTK_VOIP_QOS
	rtl8651_setAsicOutputQueueNumber(0,QNUM3);
	rtl8651_setAsicOutputQueueNumber(1,QNUM3);
	rtl8651_setAsicOutputQueueNumber(2,QNUM3);
	rtl8651_setAsicOutputQueueNumber(3,QNUM3);
	rtl8651_setAsicOutputQueueNumber(4,QNUM3);
	rtl8651_setAsicOutputQueueNumber(6,QNUM3);
	REG32(CPUQIDMCR0)=0x55221000;
	rtl8651_setAsicPriorityDecision(2, 1, 2, 1, 1);
	#endif


	#if defined (CONFIG_RTL_UNKOWN_UNICAST_CONTROL)
	{
		rtl865x_tblAsicDrv_rateLimitParam_t	asic_rl;
		/*
	  	* Designer said: The time unit used to achieve rate limit is 1.67s (5/3), hence here we change
	  	* the time unit to 1 sec.
	  	*/
		bzero(&asic_rl, sizeof(rtl865x_tblAsicDrv_rateLimitParam_t));
		asic_rl.maxToken			= RTL_MAC_REFILL_TOKEN;
		asic_rl.refill_number		= RTL_MAC_REFILL_TOKEN;
		asic_rl.t_intervalUnit		= 1;
		asic_rl.t_remainUnit		= 1;
		asic_rl.token				= RTL_MAC_REFILL_TOKEN;
		rtl8651_setAsicRateLimitTable(0, &asic_rl);

		macRecordIdx = 0;
		bzero(macRecord, RTL_MAC_RECORD_NUM*sizeof(rtlMacRecord));

		for(i=0;i<RTL_MAC_RECORD_NUM;i++)
		{
			init_timer(&macRecord[i].timer);
			macRecord[i].timer.function = rtl_unkownUnicastTimer;
		}

		WRITE_MEM32(TEACR, (READ_MEM32(TEACR)|EnRateLimitTbAging));
	}
	#endif

	return SUCCESS;
}

#if defined (CONFIG_RTL_UNKOWN_UNICAST_CONTROL)
static void rtl_unkownUnicastTimer(unsigned long data)
{
	rtlMacRecord	*record;
	rtl865x_AclRule_t	rule;

	record = (rtlMacRecord*)data;
	bzero((void*)&rule,sizeof(rtl865x_AclRule_t));
	rule.ruleType_ = RTL865X_ACL_MAC;
	rule.pktOpApp_ = RTL865X_ACL_ONLY_L2;
	rule.actionType_ = RTL865X_ACL_DROP_RATE_EXCEED_PPS;
	memset(rule.dstMacMask_.octet, 0xff, ETHER_ADDR_LEN);
	memcpy(rule.dstMac_.octet, record->mac, ETHER_ADDR_LEN);
	rtl865x_del_acl(&rule, lanIfName, RTL865X_ACL_SYSTEM_USED);

	bzero(record, sizeof(rtlMacRecord));
	init_timer(&record->timer);
	record->timer.function = rtl_unkownUnicastTimer;
}

static void	rtl_unkownUnicastUpdate(uint8 *mac)
{
	int	idx;
	rtl865x_AclRule_t	rule;

	for(idx=0;idx<RTL_MAC_RECORD_NUM;idx++)
	{
		if (macRecord[idx].enable==0||memcmp(mac, macRecord[idx].mac, ETHER_ADDR_LEN))
			continue;

		/*	The mac has already recorded	*/
		if (macRecord[idx].cnt==RTL_MAC_THRESHOLD||++macRecord[idx].cnt<RTL_MAC_THRESHOLD)
			return;

		break;
	}

	/*	add/del the rules at lan side */
	bzero((void*)&rule,sizeof(rtl865x_AclRule_t));
	rule.ruleType_ = RTL865X_ACL_MAC;
	rule.pktOpApp_ = RTL865X_ACL_ONLY_L2;
	rule.actionType_ = RTL865X_ACL_DROP_RATE_EXCEED_PPS;
	memset(rule.dstMacMask_.octet, 0xff, ETHER_ADDR_LEN);

	if (idx==RTL_MAC_RECORD_NUM)
	{
		if (macRecord[macRecordIdx].enable!=0&&macRecord[macRecordIdx].cnt>RTL_MAC_THRESHOLD)
		{
			memcpy(rule.dstMac_.octet, macRecord[macRecordIdx].mac, ETHER_ADDR_LEN);
			rtl865x_del_acl(&rule, lanIfName, RTL865X_ACL_SYSTEM_USED);
			init_timer(&macRecord[macRecordIdx].timer);
			macRecord[macRecordIdx].timer.function = rtl_unkownUnicastTimer;
		}
		else
		{
			macRecord[macRecordIdx].enable = 1;
		}
		macRecord[macRecordIdx].cnt = 0;
		memcpy(macRecord[macRecordIdx].mac, mac, ETHER_ADDR_LEN);
		macRecordIdx = (macRecordIdx+1)&(RTL_MAC_RECORD_NUM-1);
	}
	else
	{
		memcpy(rule.dstMac_.octet, mac, ETHER_ADDR_LEN);
		rtl865x_add_acl(&rule, lanIfName, RTL865X_ACL_SYSTEM_USED);
		macRecord[idx].timer.data = (unsigned long)&(macRecord[idx]);
		mod_timer(&macRecord[idx].timer, jiffies+RTL_MAC_TIMEOUT);
	}
}
#endif

#if defined (CONFIG_RTL_IGMP_SNOOPING)

static int re865x_reInitIgmpSetting(int mode)
{
	#if defined (CONFIG_RTL_MULTI_LAN_DEV)
	#else
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	rtl_multicastDeviceInfo_t devInfo;
	uint32 externalPortMask=0;
	#endif
	int32 i;
	int32 totalVlans=((sizeof(vlanconfig))/(sizeof(struct rtl865x_vlanConfig)))-1;

	for(i=0; i<totalVlans; i++)
	{
		#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		if (TRUE==vlanconfig[i].isWan)
		{
			externalPortMask |=vlanconfig[i].memPort;
		}
		else
		{
			devInfo.portMask|=vlanconfig[i].memPort ;
		}
		#endif

		if(mode==GATEWAY_MODE)
		{
			rtl_setIgmpSnoopingModuleStaticRouterPortMask(nicIgmpModuleIndex, 0);
		}
		else
		{
			rtl_setIgmpSnoopingModuleStaticRouterPortMask(nicIgmpModuleIndex, 0x01);
		}

		//rtl_setIgmpSnoopingModuleUnknownMCastFloodMap(nicIgmpModuleIndex, 0x0);
		rtl_setIpv4UnknownMCastFloodMap(nicIgmpModuleIndex, 0xFFFFFFFF);
		rtl_setIpv6UnknownMCastFloodMap(nicIgmpModuleIndex, 0xFFFFFFFF);

	}
	#endif

	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	rtl865x_reinitMulticast();
	//rtl865x_setMulticastExternalPortMask(externalPortMask);
	rtl865x_setMulticastExternalPortMask(0);
	#endif
	rtl865x_igmpSyncLinkStatus();
	return SUCCESS;
}
#endif

#if defined (CONFIG_RTL_MULTI_LAN_DEV)
unsigned int rtl865x_getEthDevLinkStatus(struct net_device *dev)
{
	if(dev!=NULL)
	{
		struct dev_priv *cp =dev->priv;
		//struct dev_priv *cp=netdev_priv(dev);
		return (cp->portmask & rtl865x_getPhysicalPortLinkStatus());

	}
	else
	{
		return 0;
	}
}
#endif

static int32 rtl_reinit_hw_table(void)
{
	/*re-init sequence eventmgr->l4->l3->l2->common is to make sure delete asic entry,
	if not following this sequence,
	some asic entry can't be deleted due to reference count is not zero*/

	/*to-do:in each layer reinit function, memset all software entry to zero,
	and force to clear all asic entry of own module,
	then the re-init sequence can be common->l2->l3->l4 */
	/* FullAndSemiReset should not be called here
	  * it will make switch core action totally wrong
        */

	/*event management */
	rtl865x_reInitEventMgr();

	/*l4*/
	#if defined(CONFIG_RTL_LAYERED_DRIVER_L4) && (defined(CONFIG_RTL_8198) ||defined(CONFIG_RTL_8196CT) ||defined(CONFIG_RTL_819XDT))
	rtl865x_nat_reinit();
	#endif

	/*l3*/
	#ifdef CONFIG_RTL_LAYERED_DRIVER_L3
	rtl865x_reinitRouteTable();
	rtl865x_reinitNxtHopTable();
	rtl865x_reinitIpTable();
	rtl865x_reinitPppTable();
	rtl865x_arp_reinit();
	#endif

	/*l2*/
	#ifdef CONFIG_RTL_LAYERED_DRIVER_L2
	rtl865x_layer2_reinit();
	#endif

	/*common*/
	rtl865x_reinitNetifTable();
	rtl865x_reinitVlantable();
	rtl865x_reinit_acl();

	/*queue id & rx ring descriptor mapping*/
	//Use REG32 instead of REG16 because CPUQDM4 is set to 0 unexpectedly when use REG16
	REG32(CPUQDM0)=QUEUEID1_RXRING_MAPPING|(QUEUEID0_RXRING_MAPPING<<16);
	REG32(CPUQDM2)=QUEUEID3_RXRING_MAPPING|(QUEUEID2_RXRING_MAPPING<<16);
	REG32(CPUQDM4)=QUEUEID5_RXRING_MAPPING|(QUEUEID4_RXRING_MAPPING<<16);

	WRITE_MEM32(PLITIMR,0);

	rtl8651_setAsicOperationLayer(2);

	return SUCCESS;

}
#if defined(CONFIG_RTK_VLAN_SUPPORT) || defined (CONFIG_RTL_MULTI_LAN_DEV)
static int rtl_config_perport_perdev_vlanconfig(int mode)
{
	vlanconfig[0].vid = RTL_LANVLANID;
	vlanconfig[0].memPort = RTL_LANPORT_MASK_4;
	vlanconfig[0].untagSet= RTL_LANPORT_MASK_4;
	((struct dev_priv *)_rtl86xx_dev.dev[0]->priv)->portmask = RTL_LANPORT_MASK_4; //eth0
	((struct dev_priv *)_rtl86xx_dev.dev[0]->priv)->id = RTL_LANVLANID_1; //eth0
	((struct dev_priv *)_rtl86xx_dev.dev[0]->priv)->portnum = 1;

	if((mode == BRIDGE_MODE) || (mode== WISP_MODE))
	{
		vlanconfig[1] .vid = RTL_LANVLANID;
		((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->id = RTL_LANVLANID;
	}
	else
	{
		vlanconfig[1] .vid = RTL_WANVLANID;
		((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->id = RTL_WANVLANID;
	}
	vlanconfig[1].memPort = RTL_WANPORT_MASK;
	vlanconfig[1].untagSet= RTL_WANPORT_MASK;
	((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->portmask = RTL_WANPORT_MASK; //eth1
	((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->id = RTL_WANVLANID; //eth1
	((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->portnum = 1;

	vlanconfig[2] .vid = RTL_LANVLANID;
	vlanconfig[2].memPort = RTL_LANPORT_MASK_3;
	vlanconfig[2].untagSet = RTL_LANPORT_MASK_3;
	((struct dev_priv *)_rtl86xx_dev.dev[2]->priv)->portmask = RTL_LANPORT_MASK_3; //eth2
	((struct dev_priv *)_rtl86xx_dev.dev[2]->priv)->id = RTL_LANVLANID_2; //eth2
	((struct dev_priv *)_rtl86xx_dev.dev[2]->priv)->portnum = 1;

	vlanconfig[3] .vid = RTL_LANVLANID;
	vlanconfig[3].memPort = RTL_LANPORT_MASK_2;
	vlanconfig[3].untagSet = RTL_LANPORT_MASK_2;
	((struct dev_priv *)_rtl86xx_dev.dev[3]->priv)->portmask = RTL_LANPORT_MASK_2; //eth3
	((struct dev_priv *)_rtl86xx_dev.dev[3]->priv)->id = RTL_LANVLANID_3; //eth3
	((struct dev_priv *)_rtl86xx_dev.dev[3]->priv)->portnum = 1;

	vlanconfig[4] .vid = RTL_LANVLANID;
	vlanconfig[4].memPort = RTL_LANPORT_MASK_1;
	vlanconfig[4].untagSet = RTL_LANPORT_MASK_1;
	((struct dev_priv *)_rtl86xx_dev.dev[4]->priv)->portmask = RTL_LANPORT_MASK_1; //eth4
	((struct dev_priv *)_rtl86xx_dev.dev[4]->priv)->id = RTL_LANVLANID_4; //eth4
	((struct dev_priv *)_rtl86xx_dev.dev[4]->priv)->portnum = 1;

	#if defined(CONFIG_8198_PORT5_GMII)
	vlanconfig[5] .vid = RTL_LANVLANID;
	vlanconfig[5].memPort = RTL_LANPORT_MASK_5;
	vlanconfig[5].untagSet = RTL_LANPORT_MASK_5;
	((struct dev_priv *)_rtl86xx_dev.dev[5]->priv)->portmask = RTL_LANPORT_MASK_5; //eth5
	((struct dev_priv *)_rtl86xx_dev.dev[5]->priv)->id = RTL_LANVLANID_5; //eth5
	((struct dev_priv *)_rtl86xx_dev.dev[5]->priv)->portnum = 1;
	#endif
	return SUCCESS;
}
#endif
static int rtl_config_lanwan_dev_vlanconfig(int mode)
{
	/*lan config*/
#if defined (CONFIG_RTK_VLAN_SUPPORT)
	{
		vlanconfig[2].vid = 0; //eth2
		vlanconfig[3].vid = 0; //eth3
		vlanconfig[4].vid = 0; //eth4
		#if defined(CONFIG_8198_PORT5_GMII)
		vlanconfig[5].vid = 0;
		#endif
	}
#endif

#if defined (CONFIG_RTL_IVL_SUPPORT)
	#if defined(CONFIG_POCKET_ROUTER_SUPPORT)
		if(mode == GATEWAY_MODE)
		{
			vlanconfig[0].memPort = 0;
			vlanconfig[0].vid=RTL_LANVLANID;
			vlanconfig[0].untagSet = 0;
			((struct dev_priv *)_rtl86xx_dev.dev[0]->priv)->portmask = 0; //eth0
			((struct dev_priv *)_rtl86xx_dev.dev[0]->priv)->id = RTL_LANVLANID;
			vlanconfig[1].vid = RTL_WANVLANID;
			vlanconfig[1].memPort = RTL_WANPORT_MASK;
			vlanconfig[1].untagSet = RTL_WANPORT_MASK;
			((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->portmask = RTL_WANPORT_MASK; //eth1
			((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->id = RTL_WANVLANID; //eth1
		}
		else
		{
			vlanconfig[0].memPort = RTL_LANPORT_MASK;
			vlanconfig[0].vid=RTL_LANVLANID;
			vlanconfig[0].untagSet = RTL_LANPORT_MASK;
			((struct dev_priv *)_rtl86xx_dev.dev[0]->priv)->portmask = RTL_LANPORT_MASK; //eth0
			((struct dev_priv *)_rtl86xx_dev.dev[0]->priv)->id = RTL_LANVLANID;

			vlanconfig[1].vid = 0;
			vlanconfig[1].memPort = 0;
			vlanconfig[1].untagSet = 0;
			((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->portmask = 0; //eth1
			((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->id = RTL_WANVLANID; //eth1
		}

	#else //else CONFIG_POCKET_ROUTER_SUPPORT
		vlanconfig[0].memPort = RTL_LANPORT_MASK;
		vlanconfig[0].vid=RTL_LANVLANID;
		vlanconfig[0].untagSet = RTL_LANPORT_MASK;
		((struct dev_priv *)_rtl86xx_dev.dev[0]->priv)->portmask = RTL_LANPORT_MASK; //eth0
		((struct dev_priv *)_rtl86xx_dev.dev[0]->priv)->id = RTL_LANVLANID;

		vlanconfig[1].vid = RTL_WANVLANID;
		vlanconfig[1].memPort = RTL_WANPORT_MASK;
		vlanconfig[1].untagSet = RTL_WANPORT_MASK;
		((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->portmask = RTL_WANPORT_MASK; //eth1
		((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->id = RTL_WANVLANID; //eth1

	#endif //endif CONFIG_POCKET_ROUTER_SUPPORT

#else
	if(mode == BRIDGE_MODE || mode== WISP_MODE)
	{
		vlanconfig[0].memPort = RTL_LANPORT_MASK |RTL_WANPORT_MASK;
		vlanconfig[0].untagSet = RTL_LANPORT_MASK |RTL_WANPORT_MASK;
		vlanconfig[0].vid=RTL_LANVLANID;
		((struct dev_priv *)_rtl86xx_dev.dev[0]->priv)->portmask = RTL_LANPORT_MASK |RTL_WANPORT_MASK; //eth0
		((struct dev_priv *)_rtl86xx_dev.dev[0]->priv)->id = RTL_LANVLANID;

		vlanconfig[1].vid = 0;
		vlanconfig[1].memPort=0x0;
		vlanconfig[1].untagSet=0x0;
		((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->portmask = 0x0; //eth1
		((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->id = 0;
	}
	else
	{
		vlanconfig[0].memPort = RTL_LANPORT_MASK;
		vlanconfig[0].vid=RTL_LANVLANID;
		vlanconfig[0].untagSet = RTL_LANPORT_MASK;
		((struct dev_priv *)_rtl86xx_dev.dev[0]->priv)->portmask = RTL_LANPORT_MASK; //eth0
		((struct dev_priv *)_rtl86xx_dev.dev[0]->priv)->id = RTL_LANVLANID;

		vlanconfig[1].vid = RTL_WANVLANID;
		vlanconfig[1].memPort = RTL_WANPORT_MASK;
		vlanconfig[1].untagSet = RTL_WANPORT_MASK;
		((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->portmask = RTL_WANPORT_MASK; //eth1
		((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->id = RTL_WANVLANID; //eth1
	}
#endif

	return SUCCESS;
}
#if	defined (CONFIG_RTL_MULTI_LAN_DEV)
static int rtl_config_multi_lan_dev_vlanconfig(int mode)
{
	rtl_config_perport_perdev_vlanconfig(mode);
	return SUCCESS;

}
#endif

#if defined(CONFIG_RTK_VLAN_SUPPORT)
static int rtl_config_rtkVlan_vlanconfig(int mode)
{
	if(!rtk_vlan_support_enable)
		rtl_config_lanwan_dev_vlanconfig(mode);
	else if(rtk_vlan_support_enable == 1)
	{
		rtl_config_perport_perdev_vlanconfig(mode);
	}
	else if(rtk_vlan_support_enable == 2)
		rtl_config_lanwan_dev_vlanconfig(mode);

	return SUCCESS;
}
#endif

static int rtl_config_operation_layer(int mode)
{
#if defined(CONFIG_RTL_LAYERED_DRIVER)
	switch(mode)
	{
		case GATEWAY_MODE:
			rtl8651_setAsicOperationLayer(4);
			break;
		case BRIDGE_MODE:
		case WISP_MODE:
			rtl8651_setAsicOperationLayer(3);
			break;
		default:
			rtl8651_setAsicOperationLayer(2);
	}
#endif
	return SUCCESS;
}

static int rtl_config_vlanconfig(int mode)
{
	#if	defined (CONFIG_RTL_MULTI_LAN_DEV)
		rtl_config_multi_lan_dev_vlanconfig(mode);
	#else
		#if defined(CONFIG_RTK_VLAN_SUPPORT)
			rtl_config_rtkVlan_vlanconfig(mode);
		#else
			rtl_config_lanwan_dev_vlanconfig(mode);
		#endif
	#endif

	return SUCCESS;

}

int32 rtl865x_changeOpMode(int mode)
{
#ifdef CONFIG_RTK_FAKE_ETH
	return SUCCESS;
#endif
#if defined (CONFIG_RTL_LOCAL_PUBLIC)
	struct rtl865x_interface_info ifInfo;
#endif

#if defined(CONFIG_RTK_VLAN_SUPPORT)
	if(rtk_vlan_support_enable==0)
	{
		if(mode==rtl865x_curOpMode)
		{
			return SUCCESS;
		}
	}
#else
	if(mode==rtl865x_curOpMode)
	{
		return SUCCESS;
	}
#endif

	/*config vlan config*/
	rtl_config_vlanconfig(mode);

	if (!vlanconfig[0].vid && !vlanconfig[1].vid )
		return RTL_EINVALIDVLANID;

#if defined (CONFIG_RTL_LOCAL_PUBLIC)
	if(mode==GATEWAY_MODE)
	{
		memset(&ifInfo, 0 , sizeof(struct rtl865x_interface_info));
#if defined(CONFIG_RTL_PUBLIC_SSID)
		strcpy(ifInfo.ifname,RTL_GW_WAN_DEVICE_NAME);
#else
		strcpy(ifInfo.ifname, RTL_DRV_WAN0_NETIF_NAME);
#endif
		ifInfo.isWan=1;
		for(i=0;vlanconfig[i].vid!=0; i++)
		{
			if ((TRUE==vlanconfig[i].isWan) && (vlanconfig[i].if_type==IF_ETHER))
			{
				ifInfo.memPort|=vlanconfig[i].memPort;
				ifInfo.fid=vlanconfig[i].fid;
			}
		}
		rtl865x_setLpIfInfo(&ifInfo);

		memset(&ifInfo, 0 , sizeof(struct rtl865x_interface_info));
		strcpy(ifInfo.ifname, RTL_DRV_LAN_NETIF_NAME);
		ifInfo.isWan=0;
		for(i=0;vlanconfig[i].vid!=0;i++)
		{
			if ((FALSE==vlanconfig[i].isWan) && (vlanconfig[i].if_type==IF_ETHER))
			{
				ifInfo.memPort|=vlanconfig[i].memPort;
				ifInfo.fid=vlanconfig[i].fid;
			}
		}
		rtl865x_setLpIfInfo(&ifInfo);
	}
	else if(mode==WISP_MODE)
	{
		memset(&ifInfo, 0 , sizeof(struct rtl865x_interface_info));
		strcpy(ifInfo.ifname, RTL_PS_WLAN0_DEV_NAME);
		ifInfo.isWan=1;
		rtl865x_setLpIfInfo(&ifInfo);


		memset(&ifInfo, 0 , sizeof(struct rtl865x_interface_info));
		strcpy(ifInfo.ifname,RTL_DRV_LAN_NETIF_NAME);
		ifInfo.isWan=0;
		for(i=0;vlanconfig[i].vid!=0;i++)
		{
			if ((FALSE==vlanconfig[i].isWan) && (vlanconfig[i].if_type==IF_ETHER))
			{
				ifInfo.memPort|=vlanconfig[i].memPort;
				ifInfo.fid=vlanconfig[i].fid;
			}
		}
		rtl865x_setLpIfInfo(&ifInfo);
	}
	else
	{
		memset(&ifInfo, 0 , sizeof(struct rtl865x_interface_info));
		ifInfo.isWan=1;
		rtl865x_setLpIfInfo(&ifInfo);

		memset(&ifInfo, 0 , sizeof(struct rtl865x_interface_info));
		strcpy(ifInfo.ifname, RTL_DRV_LAN_NETIF_NAME);
		ifInfo.isWan=0;
		for(i=0;vlanconfig[i].vid!=0;i++)
		{
			if ((FALSE==vlanconfig[i].isWan) && (vlanconfig[i].if_type==IF_ETHER))
			{
				ifInfo.memPort|=vlanconfig[i].memPort;
				ifInfo.fid=vlanconfig[i].fid;
			}
		}
		rtl865x_setLpIfInfo(&ifInfo);
	}
#endif

#if defined (CONFIG_RTL_MULTI_LAN_DEV) ||defined(CONFIG_RTK_VLAN_SUPPORT)
	re865x_packVlanConfig(vlanconfig, packedVlanConfig);
#endif

	/*reinit hw tables*/
	rtl_reinit_hw_table();

#if defined (CONFIG_RTL_MULTI_LAN_DEV) ||defined(CONFIG_RTK_VLAN_SUPPORT)
	reinit_vlan_configure(packedVlanConfig);
#else
	reinit_vlan_configure(vlanconfig);
#endif

#if defined (CONFIG_RTL_IGMP_SNOOPING)
	re865x_reInitIgmpSetting(mode);
#if defined (CONFIG_RTL_MLD_SNOOPING)
#if defined(CONFIG_RTK_VLAN_SUPPORT)
	if((mldSnoopEnabled==TRUE)&& (rtk_vlan_support_enable==0))
#else
	if(mldSnoopEnabled==TRUE)
#endif
	{
#if defined (CONFIG_RTL_MULTI_LAN_DEV)
		rtl865x_addAclForMldSnooping(packedVlanConfig);
#else
		rtl865x_addAclForMldSnooping(vlanconfig);
#endif
	}
#endif
#endif

#if defined (CONFIG_RTL_IVL_SUPPORT)
	if(mode==GATEWAY_MODE)
	{
		WRITE_MEM32( FFCR, READ_MEM32( FFCR ) & ~EN_UNUNICAST_TOCPU );
	}
	else	if((mode==BRIDGE_MODE) ||(mode==WISP_MODE))
	{
		WRITE_MEM32( FFCR, READ_MEM32( FFCR ) | EN_UNUNICAST_TOCPU );
	}
	else
	{
		WRITE_MEM32( FFCR, READ_MEM32( FFCR ) & ~EN_UNUNICAST_TOCPU );
	}
#endif

	/*update current operation mode*/
	rtl865x_curOpMode=mode;

	//setAsicOperationLayer
	rtl_config_operation_layer(mode);
	#if defined(CONFIG_RTL_MULTIPLE_WAN)
	if(rtl865x_curOpMode != GATEWAY_MODE)
		rtl865xC_setNetDecisionPolicy(NETIF_PORT_BASED);
	#endif

#ifdef CONFIG_RTL_LAYERED_DRIVER_L3
	//always init the default route...
	if(rtl8651_getAsicOperationLayer() >2)
	{
		rtl865x_addRoute(0,0,0,RTL_DRV_WAN0_NETIF_NAME,0);
	}
#endif

	//checksum control register
	switch(mode)
	{
		case GATEWAY_MODE:
			WRITE_MEM32(CSCR,READ_MEM32(CSCR)&~ALLOW_L2_CHKSUM_ERR);
			WRITE_MEM32(CSCR,READ_MEM32(CSCR)&~ALLOW_L3_CHKSUM_ERR);
			WRITE_MEM32(CSCR,READ_MEM32(CSCR)&~ALLOW_L4_CHKSUM_ERR);
			break;
		case BRIDGE_MODE:
		case WISP_MODE:
			WRITE_MEM32(CSCR,READ_MEM32(CSCR)&~ALLOW_L2_CHKSUM_ERR);
			WRITE_MEM32(CSCR,READ_MEM32(CSCR)|ALLOW_L3_CHKSUM_ERR);
			WRITE_MEM32(CSCR,READ_MEM32(CSCR)|ALLOW_L4_CHKSUM_ERR);
			break;
		default:
			WRITE_MEM32(CSCR,READ_MEM32(CSCR)&~ALLOW_L2_CHKSUM_ERR);
			WRITE_MEM32(CSCR,READ_MEM32(CSCR)&~ALLOW_L3_CHKSUM_ERR);
			WRITE_MEM32(CSCR,READ_MEM32(CSCR)&~ALLOW_L4_CHKSUM_ERR);
	}

	return SUCCESS;
}


int  rtl865x_reChangeOpMode (void)
{
	unsigned long flags;
	local_irq_save(flags);
	if(rtl865x_curOpMode==GATEWAY_MODE)
	{
		rtl865x_changeOpMode(BRIDGE_MODE);
		rtl865x_changeOpMode(GATEWAY_MODE);
	}
	else if (rtl865x_curOpMode==BRIDGE_MODE)
	{
		rtl865x_changeOpMode(GATEWAY_MODE);
		rtl865x_changeOpMode(BRIDGE_MODE);
	}
	else if (rtl865x_curOpMode==WISP_MODE)
	{
		rtl865x_changeOpMode(GATEWAY_MODE);
		rtl865x_changeOpMode(WISP_MODE);
	}
	local_irq_restore(flags);
	return 0;
}


static int32 reinit_vlan_configure(struct rtl865x_vlanConfig new_vlanconfig[])
{
	uint16 pvid;
	int32 i, j;
	uint32 valid_port_mask = 0;
	struct rtl865x_vlanConfig *pvlanconfig = NULL;
	int32 totalVlans = 0;
	pvlanconfig = new_vlanconfig;


	/*get input vlan config entry number*/
	for(i=0; pvlanconfig[i].ifname[0] != '\0' ; i++)
	{
		if(pvlanconfig[i].vid != 0)
			totalVlans++;
	}
	//because the new_vlanconfig should be packedVlanConfig
	totalVlans = totalVlans > NETIF_NUMBER? NETIF_NUMBER:totalVlans;

	for(i=0; i<totalVlans; i++)
	{
		rtl865x_netif_t netif;

		if(pvlanconfig[i].vid == 0)
			continue;

		valid_port_mask = pvlanconfig[i].memPort;
		if(pvlanconfig[i].isWan == 0)
			valid_port_mask |= 0x100;

		/*add vlan*/
		if(pvlanconfig[i].if_type==IF_ETHER)
		{
			rtl865x_addVlan(pvlanconfig[i].vid);
			rtl865x_addVlanPortMember(pvlanconfig[i].vid,pvlanconfig[i].memPort & valid_port_mask);
			rtl865x_setVlanFilterDatabase(pvlanconfig[i].vid,pvlanconfig[i].fid);
		}

		/*add network interface*/
		memset(&netif, 0, sizeof(rtl865x_netif_t));
		memcpy(netif.name,pvlanconfig[i].ifname,MAX_IFNAMESIZE);
		memcpy(netif.macAddr.octet,pvlanconfig[i].mac.octet,ETHER_ADDR_LEN);
		netif.mtu = pvlanconfig[i].mtu;
		netif.if_type = pvlanconfig[i].if_type;
		netif.vid = pvlanconfig[i].vid;
		netif.is_wan = pvlanconfig[i].isWan;
		netif.is_slave = pvlanconfig[i].is_slave;
		#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		netif.enableRoute=1;
		#endif
		rtl865x_addNetif(&netif);
		if(netif.is_slave == 1)
#if defined(CONFIG_RTL_PUBLIC_SSID)
			rtl865x_attachMasterNetif(netif.name,RTL_GW_WAN_DEVICE_NAME);
#else
			rtl865x_attachMasterNetif(netif.name,RTL_DRV_WAN0_NETIF_NAME);
#endif
	}

	#if defined(CONFIG_RTL_MULTIPLE_WAN)
	rtl_config_multipleWan_netif(RTL_MULTIWAN_ADD);
	rtl865x_addMultiCastNetif();
	#endif

#if defined(CONFIG_RTK_VLAN_SUPPORT)
		if(rtk_vlan_support_enable == 1)
			rtl865x_enable_acl(0);
		else
			rtl865x_enable_acl(1);
#endif


	for(i=0; i<RTL8651_PORT_NUMBER + 3; i++)
	{
		/* Set each port's PVID */
		for(j=0,pvid=0; pvlanconfig[j].vid != 0; j++)
		{
			if ( (1<<i) & pvlanconfig[j].memPort )
			{
				pvid = pvlanconfig[j].vid;
				break;
			}
		}

		if (pvid!=0)
		{
			CONFIG_CHECK(rtl8651_setAsicPvid(i,pvid));
			rtl865x_setPortToNetif(pvlanconfig[j].ifname, i);
		}
	}

#if defined(CONFIG_RTL_HW_VLAN_SUPPORT)
        if(rtl_hw_vlan_enable)
        {
        	if((hw_vlan_info[1].vlan_port_enabled)&&hw_vlan_info[1].vlan_port_tag){
            		 rtl865x_setVlanPortTag(vlanconfig[1].vid,RTL_WANPORT_MASK,1); //eth1 vlan port 1 tag
        	}
	         swNic_setVlanPortTag(RTL_WANPORT_MASK);//packet from CPU, HW add tag

                if((hw_vlan_info[0].vlan_port_enabled)&&(hw_vlan_info[0].vlan_port_bridge == 1)){
                        rtl865x_addVlan(hw_vlan_info[0].vlan_port_vid);
                        rtl865x_addVlanPortMember(hw_vlan_info[0].vlan_port_vid, RTL_LANPORT_MASK_4|RTL_WANPORT_MASK);
                        rtl865x_setVlanPortTag(hw_vlan_info[0].vlan_port_vid, RTL_WANPORT_MASK,1); //wan port tag
			  if(hw_vlan_info[0].vlan_port_tag){
					rtl865x_setVlanPortTag(hw_vlan_info[0].vlan_port_vid, RTL_LANPORT_MASK_4,1); //wan port tag
			  }
			  rtl865x_setVlanFilterDatabase(hw_vlan_info[0].vlan_port_vid,0);
                        rtl8651_setAsicPvid(0, hw_vlan_info[0].vlan_port_vid); //port vid
			  rtl865x_setPortForward(RTL_LANPORT_MASK_4, TRUE);
                }
                if((hw_vlan_info[2].vlan_port_enabled)&&(hw_vlan_info[2].vlan_port_bridge == 1)){
                        rtl865x_addVlan(hw_vlan_info[2].vlan_port_vid);
                        rtl865x_addVlanPortMember(hw_vlan_info[2].vlan_port_vid, RTL_LANPORT_MASK_3|RTL_WANPORT_MASK);
                        rtl865x_setVlanPortTag(hw_vlan_info[2].vlan_port_vid, RTL_WANPORT_MASK,1); //wan port tag
			  if(hw_vlan_info[2].vlan_port_tag){
					 rtl865x_setVlanPortTag(hw_vlan_info[2].vlan_port_vid, RTL_LANPORT_MASK_3,1); //wan port tag
			  }
			  rtl865x_setVlanFilterDatabase(hw_vlan_info[2].vlan_port_vid,0);
                        rtl8651_setAsicPvid(1, hw_vlan_info[2].vlan_port_vid); //port vid
                        rtl865x_setPortForward(RTL_LANPORT_MASK_3, TRUE);
                }
                if((hw_vlan_info[3].vlan_port_enabled)&&(hw_vlan_info[3].vlan_port_bridge == 1)){
                        rtl865x_addVlan(hw_vlan_info[3].vlan_port_vid);
                        rtl865x_addVlanPortMember(hw_vlan_info[3].vlan_port_vid, RTL_LANPORT_MASK_2|RTL_WANPORT_MASK);
                        rtl865x_setVlanPortTag(hw_vlan_info[3].vlan_port_vid, RTL_WANPORT_MASK,1); //wan port tag
			  if(hw_vlan_info[3].vlan_port_tag){
				 	rtl865x_setVlanPortTag(hw_vlan_info[3].vlan_port_vid, RTL_LANPORT_MASK_2,1); //wan port tag
			  }
			  rtl865x_setVlanFilterDatabase(hw_vlan_info[3].vlan_port_vid,0);
                        rtl8651_setAsicPvid(2, hw_vlan_info[3].vlan_port_vid); //port vid
                        rtl865x_setPortForward(RTL_LANPORT_MASK_2, TRUE);
                }
		 if((hw_vlan_info[4].vlan_port_enabled)&&(hw_vlan_info[4].vlan_port_bridge == 1)){
                        rtl865x_addVlan(hw_vlan_info[4].vlan_port_vid);
                        rtl865x_addVlanPortMember(hw_vlan_info[4].vlan_port_vid, RTL_LANPORT_MASK_1|RTL_WANPORT_MASK);
                        rtl865x_setVlanPortTag(hw_vlan_info[4].vlan_port_vid, RTL_WANPORT_MASK,1); //wan port tag
			  if(hw_vlan_info[4].vlan_port_tag){
				 	rtl865x_setVlanPortTag(hw_vlan_info[4].vlan_port_vid, RTL_LANPORT_MASK_1,1); //wan port tag
			  }
			  rtl865x_setVlanFilterDatabase(hw_vlan_info[4].vlan_port_vid,0);
                        rtl8651_setAsicPvid(3, hw_vlan_info[4].vlan_port_vid); //port vid
                        rtl865x_setPortForward(RTL_LANPORT_MASK_1, TRUE);
                }
        }
        else{
                swNic_setVlanPortTag(0);
        }
#endif

#ifdef CONFIG_RTL_STP
	re865x_stp_mapping_reinit();
#endif

	rtl_config_operation_layer(rtl865x_curOpMode);

	#if defined(CONFIG_RTL_MULTIPLE_WAN)
	if(rtl865x_curOpMode != GATEWAY_MODE)
		rtl865xC_setNetDecisionPolicy(NETIF_PORT_BASED);
	#endif

	return SUCCESS;
}
#if defined(CONFIG_RTL_REPORT_LINK_STATUS)
static int32 rtk_link_status_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
		int len;
	len = sprintf(page, "%d\n",wan_linkStatus[0]);

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count)
		len = count;
	if (len<0)
	  	len = 0;

	return len;
}
static int32 rtk_link_status_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char 		tmpbuf[4];
	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
		if(0 == (tmpbuf[0]-'0'))
			wan_linkStatus[0] = 0;
	}
	return len;
}
#endif
#if defined(CONFIG_RTK_VLAN_SUPPORT)
#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
static int32 rtk_vlan_wan_tag_getportmask(int bridge_port)
{
	int32 port_mask = 0x80;// PKTHDR_EXTPORT_LIST_P2 2=> bit 7

	if(vlan_bridge_port&(1<<3))
		port_mask |= RTL_LANPORT_MASK_1;

	if(vlan_bridge_port&(1<<2))
		port_mask |= RTL_LANPORT_MASK_2;

	if(vlan_bridge_port&(1<<1))
		port_mask |= RTL_LANPORT_MASK_3;

	if(vlan_bridge_port&(1<<0))
		port_mask |= RTL_LANPORT_MASK_4;

	return port_mask;
}
static int32 rtk_vlan_wan_tag_support_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char tmpbuf[100];
	int lan_portmask;
	int num ;
	int i=0;
	int j=0;
	struct net_device *dev;
	struct dev_priv	  *dp;
	#ifdef CONFIG_RTL_IGMP_SNOOPING
		int ret = 0;
		rtl_multicastDeviceInfo_t devInfo;
	#endif

        if (buff && !copy_from_user(tmpbuf, buff, len))
        {

		num = sscanf(tmpbuf, "%d %d %d %d %d", &vlan_tag,  &vlan_host_pri, &vlan_bridge_tag, &vlan_bridge_port,&vlan_bridge_multicast_tag);
		if (num !=  5) {
                    printk("invalid rtk_vlan_wan_tag_support_write parameter!\n");
                    return len;
            }
		if(rtl865x_curOpMode == WISP_MODE) //not support in WISP mode
			{
			vlan_tag = 0;
			vlan_bridge_tag = 0;
			}
		else if(rtl865x_curOpMode == BRIDGE_MODE)
		{
			vlan_bridge_tag = 0;
		}
		if(vlan_bridge_tag == vlan_tag)
			vlan_bridge_tag = 0;

			rtl_config_rtkVlan_vlanconfig(rtl865x_curOpMode);

		//Reset PPPoE vlan id to original
		vlanconfig[VLAN_CONFIG_PPPOE_INDEX].vid = RTL_WANVLANID;
	
	
	            lan_portmask =  0;

		if(vlan_bridge_tag)  
	            {
                    lan_portmask = rtk_vlan_wan_tag_getportmask(vlan_bridge_port);
	            	vlanconfig[2] .vid = vlan_bridge_tag;
	            	vlanconfig[2].memPort = lan_portmask|RTL_WANPORT_MASK;
					vlanconfig[2].untagSet = lan_portmask;

					//need verify
					vlanconfig[2].fid = 1;
					((struct dev_priv *)_rtl86xx_dev.dev[2]->priv)->portmask = lan_portmask|RTL_WANPORT_MASK;
					((struct dev_priv *)_rtl86xx_dev.dev[2]->priv)->id = vlan_bridge_tag; //eth2
			//vlanconfig[0].vid= RTL_LANVLANID;			
	            vlanconfig[0].memPort = RTL_LANPORT_MASK &(~lan_portmask);
	            vlanconfig[0].untagSet = RTL_LANPORT_MASK &(~lan_portmask);
	            ((struct dev_priv *)_rtl86xx_dev.dev[0]->priv)->portmask = RTL_LANPORT_MASK &(~lan_portmask); //eth0
			//((struct dev_priv *)_rtl86xx_dev.dev[0]->priv)->id = RTL_LANVLANID;
		}

		if(rtl865x_curOpMode == GATEWAY_MODE && vlan_tag)
		{
	            vlanconfig[1].vid = vlan_tag;
	            vlanconfig[1].memPort = RTL_WANPORT_MASK;
	            vlanconfig[1].untagSet = 0; // need tag
	            ((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->portmask = RTL_WANPORT_MASK; //eth1
	            ((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->id = vlan_tag; //eth1
	             //PPPOE
			vlanconfig[VLAN_CONFIG_PPPOE_INDEX].vid = vlan_tag;	
	        }

		re865x_packVlanConfig(vlanconfig, packedVlanConfig);
			rtl_reinit_hw_table();
		if(vlan_bridge_tag)
	        {
			packedVlanConfig[2].memPort &= (~0x100); //eth2 use port 7, not port 8
			packedVlanConfig[2].untagSet &= (~0x100);
		}
		reinit_vlan_configure(packedVlanConfig);

		if(rtl865x_curOpMode == GATEWAY_MODE && vlan_tag)
		{
			rtl865x_setVlanPortTag(vlan_tag, RTL_WANPORT_MASK, true); //eth1 vlan port 1 tag
		}
		if(rtl865x_curOpMode == BRIDGE_MODE && vlan_tag)
				{
			rtl865x_addVlan(vlan_tag);
			rtl865x_addVlanPortMember(vlan_tag,RTL_LANPORT_MASK | RTL_WANPORT_MASK);
			rtl865x_setVlanPortTag(vlan_tag, (RTL_LANPORT_MASK|RTL_WANPORT_MASK)&(~0x100),1);
			rtl865x_setVlanFilterDatabase(vlan_tag,0);
		}

		if(vlan_bridge_tag)
		{
			rtl865x_setVlanPortTag(vlan_bridge_tag, RTL_WANPORT_MASK, true); //eth1 vlan port 1 tag
					if(nicIgmpModuleIndex_2==0xFFFFFFFF)
					{
						ret = rtl_registerIgmpSnoopingModule(&nicIgmpModuleIndex_2);
					#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
						memset(&devInfo, 0, sizeof(rtl_multicastDeviceInfo_t ));
						strcpy(devInfo.devName, RTL_PS_ETH_NAME_ETH2);
						if(ret==0)
							 rtl_setIgmpSnoopingModuleDevInfo(nicIgmpModuleIndex_2,&devInfo);
					#endif
					}
			if(vlan_bridge_multicast_tag)
			{
				if (vlan_bridge_multicast_tag == vlan_bridge_tag)
				{
					vlan_bridge_multicast_tag = 0;
				}
				else
					{
                        rtl865x_addVlan(vlan_bridge_multicast_tag);
                        rtl865x_addVlanPortMember(vlan_bridge_multicast_tag,RTL_WANPORT_MASK);
                        rtl865x_setVlanPortTag(vlan_bridge_multicast_tag,RTL_WANPORT_MASK,1); //wan port tag
					rtl865x_setVlanFilterDatabase(vlan_bridge_multicast_tag,1);
				}
                    }
				}
				else
				{
					if(nicIgmpModuleIndex_2!=0xFFFFFFFF)
						rtl_unregisterIgmpSnoopingModule(nicIgmpModuleIndex_2);
				//rtl865x_setVlanPortTag(vlanconfig[1].vid,RTL_WANPORT_MASK,false); //eth1 vlan port 1 tag
            #ifdef CONFIG_RTL_IGMP_SNOOPING
				if(nicIgmpModuleIndex_2!=0xFFFFFFFF)
					rtl_unregisterIgmpSnoopingModule(nicIgmpModuleIndex_2);
			#endif
	        }

				/*update dev port number*/
				for(i=0; vlanconfig[i].vid != 0; i++)
				{
					if (IF_ETHER!=vlanconfig[i].if_type)
					{
						continue;
					}

					dev=_rtl86xx_dev.dev[i];
					dp = dev->priv;
					dp->portnum  = 0;
					for(j=0;j<RTL8651_AGGREGATOR_NUMBER;j++)
					{
						if(dp->portmask & (1<<j))
							dp->portnum++;
					}

				}

			#if defined (CONFIG_RTL_IGMP_SNOOPING)
				re865x_reInitIgmpSetting(rtl865x_curOpMode);
				#if defined (CONFIG_RTL_MLD_SNOOPING)
				if(mldSnoopEnabled)
				{
					re865x_packVlanConfig(vlanconfig, packedVlanConfig);
					rtl865x_addAclForMldSnooping(packedVlanConfig);
				}
				#endif
			#endif

				//always init the default route...
				if(rtl8651_getAsicOperationLayer() >2)
				{
#ifdef CONFIG_RTL_LAYERED_DRIVER_L3
					rtl865x_addRoute(0,0,0,RTL_DRV_WAN0_NETIF_NAME,0);
#endif
				}
		WRITE_MEM32(SWTCR0,(READ_MEM32(SWTCR0)| EnUkVIDtoCPU));


        }

        return len;
}
static int32 rtk_vlan_wan_tag_support_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
        int len;
        len = sprintf(page, "vlan_tag: %d vlan_host_pri: %d \nvlan_bridge_tag: %d, vlan_bridge_port: 0x%x vlan_bridge_stream_tag: %d\n",
        vlan_tag, vlan_host_pri, vlan_bridge_tag, vlan_bridge_port,vlan_bridge_multicast_tag);
        if (len <= off+count) *eof = 1;
        *start = page + off;
        len -= off;
        if (len>count)
                len = count;
        if (len<0)
                len = 0;
        return len;
}

#endif
static int32 rtk_vlan_support_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;
	len = sprintf(page, "%s %d\n", "rtk_vlan_support_enable:",rtk_vlan_support_enable);


	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count)
		len = count;
	if (len<0)
	  	len = 0;

	return len;
}
static int32 rtk_vlan_support_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char 		tmpbuf[32];
	int i=0;
	int j=0;
	struct net_device *dev;
	struct dev_priv	  *dp;

	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
		tmpbuf[len] = '\0';
		#if defined (CONFIG_RTL_IGMP_SNOOPING) && defined (CONFIG_RTL_MLD_SNOOPING)
		if(mldSnoopEnabled)
		{
			rtl865x_removeAclForMldSnooping(vlanconfig);
		}
		#endif
		if(tmpbuf[0] == '0')
		{
			rtk_vlan_support_enable = 0;

			rtl_config_rtkVlan_vlanconfig(rtl865x_curOpMode);
			re865x_packVlanConfig(vlanconfig, packedVlanConfig);
			rtl_reinit_hw_table();
			reinit_vlan_configure(packedVlanConfig);

			//unknow vlan drop
			REG32(SWTCR0) &= ~(1 << 15);

#if defined(CONFIG_RTL_LAYERED_DRIVER_ACL)
			rtl865x_enable_acl(1); //enable acl feature
#endif
		}
		else if(tmpbuf[0] == '1')
		{
			rtk_vlan_support_enable = 1;

			rtl_config_rtkVlan_vlanconfig(rtl865x_curOpMode);
			re865x_packVlanConfig(vlanconfig, packedVlanConfig);
			rtl_reinit_hw_table();
			reinit_vlan_configure(packedVlanConfig);

			//unknow vid to cpu
			REG32(SWTCR0) |= 1 << 15;
#if defined(CONFIG_RTL_LAYERED_DRIVER_ACL)
			rtl865x_enable_acl(0); //disable acl feature
#endif
		}
#if defined(CONFIG_RTK_VLAN_FOR_CABLE_MODEM)
		else if(tmpbuf[0] == '2')
		{
			rtk_vlan_support_enable = 2;
			//unknow vid to cpu
			REG32(SWTCR0) |= 1 << 15;
		}
#endif
		else
		{
			printk("current support: 0/1/2\n");
			return len;
		}

		/*update dev port number*/
		for(i=0; vlanconfig[i].vid != 0; i++)
		{
			if (IF_ETHER!=vlanconfig[i].if_type)
			{
				continue;
			}

			dev=_rtl86xx_dev.dev[i];
			dp = dev->priv;
			dp->portnum  = 0;
			for(j=0;j<RTL8651_AGGREGATOR_NUMBER;j++)
			{
				if(dp->portmask & (1<<j))
					dp->portnum++;
			}

		}

	#if defined (CONFIG_RTL_IGMP_SNOOPING)
		re865x_reInitIgmpSetting(rtl865x_curOpMode);
		#if defined (CONFIG_RTL_MLD_SNOOPING)
		if(mldSnoopEnabled && (rtk_vlan_support_enable==0))
		{
			re865x_packVlanConfig(vlanconfig, packedVlanConfig);
			rtl865x_addAclForMldSnooping(packedVlanConfig);
		}
		#endif
	#endif

		//always init the default route...
		if(rtl8651_getAsicOperationLayer() >2)
		{
#ifdef CONFIG_RTL_LAYERED_DRIVER_L3
			rtl865x_addRoute(0,0,0,RTL_DRV_WAN0_NETIF_NAME,0);
#endif
		}

	}
	return len;
}

#if defined(CONFIG_819X_PHY_RW) //#if defined(CONFIG_RTK_VLAN_FOR_CABLE_MODEM)
static int32 rtl_phy_status_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int		len;
	uint32	regData;
	uint32	data0;
	int		port;

	len = sprintf(page, "Port Status:\n");

	for(port=PHY0;port<CPU;port++)
	{
		regData = READ_MEM32(PSRP0+((port)<<2));

		len += sprintf(page+len, "Port%d ", port);
		data0 = regData & PortStatusLinkUp;

		if (data0)
			len += sprintf(page+len, "LinkUp | ");
		else
		{
			len += sprintf(page+len, "LinkDown\n\n");
			continue;
		}
		data0 = regData & PortStatusDuplex;
		len += sprintf(page+len, "	Duplex %s | ", data0?"Enabled":"Disabled");
		data0 = (regData&PortStatusLinkSpeed_MASK)>>PortStatusLinkSpeed_OFFSET;
		len += sprintf(page+len, "Speed %s\n\n", data0==PortStatusLinkSpeed100M?"100M":
			(data0==PortStatusLinkSpeed1000M?"1G":
				(data0==PortStatusLinkSpeed10M?"10M":"Unkown")));
	}


	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count)
		len = count;
	if (len<0)
	  	len = 0;

	return len;
}



static int32 rtl_phy_status_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char 		tmpbuf[64];
	uint32	port_mask;
	char		*strptr, *cmd_addr;
	char		*tokptr;
	int		type;
	int 		port;
	int forceMode = 0;
	int forceLink = 0;
	int forceLinkSpeed = 0;
	int forceDuplex = 0;
	uint32 advCapability = 0;
	int forwardEnable = TRUE;

	#define SPEED10M 	0
	#define SPEED100M 	1
	#define SPEED1000M 	2

	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		tmpbuf[len-1] = '\0';
		strptr=tmpbuf;
		cmd_addr = strsep(&strptr," ");
		if (cmd_addr==NULL)
		{
			goto errout;
		}
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}

		if (!memcmp(cmd_addr, "port", 4))
		{
			port_mask=simple_strtol(tokptr, NULL, 0);
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}

			if(strcmp(tokptr,"10_half") == 0)
				type = HALF_DUPLEX_10M;
			else if(strcmp(tokptr,"100_half") == 0)
				type = HALF_DUPLEX_100M;
			else if(strcmp(tokptr,"1000_half") == 0)
				type = HALF_DUPLEX_1000M;
			else if(strcmp(tokptr,"10_full") == 0)
				type = DUPLEX_10M;
			else if(strcmp(tokptr,"100_full") == 0)
				type = DUPLEX_100M;
			else if(strcmp(tokptr,"1000_full") == 0)
				type = DUPLEX_1000M;
			else
				type = PORT_AUTO;

			tokptr =  strsep(&strptr," ");
			if(tokptr == NULL)
				goto errout;

			forwardEnable = simple_strtol(tokptr,NULL,0);

			switch(type)
			{
				case HALF_DUPLEX_10M:
				{
					forceMode=TRUE;
					forceLink=TRUE;
					forceLinkSpeed=SPEED10M;
					forceDuplex=FALSE;
					advCapability=(1<<HALF_DUPLEX_10M);
					break;
				}
				case HALF_DUPLEX_100M:
				{
					forceMode=TRUE;
					forceLink=TRUE;
					forceLinkSpeed=SPEED100M;
					forceDuplex=FALSE;
					advCapability=(1<<HALF_DUPLEX_100M);
					break;
				}
				case HALF_DUPLEX_1000M:
				{
					forceMode=TRUE;
					forceLink=TRUE;
					forceLinkSpeed=SPEED1000M;
					forceDuplex=FALSE;
					advCapability=(1<<HALF_DUPLEX_1000M);
					break;
				}
				case DUPLEX_10M:
				{
					forceMode=TRUE;
					forceLink=TRUE;
					forceLinkSpeed=SPEED10M;
					forceDuplex=TRUE;
					advCapability=(1<<DUPLEX_10M);
					break;
				}
				case DUPLEX_100M:
				{
					forceMode=TRUE;
					forceLink=TRUE;
					forceLinkSpeed=SPEED100M;
					forceDuplex=TRUE;
					advCapability=(1<<DUPLEX_100M);
					break;
				}
				case DUPLEX_1000M:
				{
					forceMode=TRUE;
					forceLink=TRUE;
					forceLinkSpeed=SPEED1000M;
					forceDuplex=TRUE;
					advCapability=(1<<DUPLEX_1000M);
					break;
				}
				default:
				{
					forceMode=FALSE;
					forceLink=TRUE;
					/*all capality*/
					advCapability=(1<<PORT_AUTO);
				}
			}


			for(port = 0; port < CPU; port++)
			{
				if((1<<port) & port_mask)
				{
					rtl865xC_setAsicEthernetForceModeRegs(port, forceMode, forceLink, forceLinkSpeed, forceDuplex);

					/*Set PHY Register*/
					rtl8651_setAsicEthernetPHYSpeed(port,forceLinkSpeed);
					rtl8651_setAsicEthernetPHYDuplex(port,forceDuplex);
					rtl8651_setAsicEthernetPHYAutoNeg(port,forceMode?FALSE:TRUE);
					rtl8651_setAsicEthernetPHYAdvCapality(port,advCapability);
					rtl8651_restartAsicEthernetPHYNway(port);
					rtl865x_setPortForward(port,forwardEnable);
				}
			}

		}
		else
		{
			goto errout;
		}
	}
	else
	{
errout:
		printk("port status only support \"port\" as the first parameter\n");
		printk("format:	\"port port_mask 10_half/100_half/10_full/100_full/1000_full/auto\"\n");
	}
	return len;
}

//mib counter
 int rtl819x_get_port_mibStats(int port , struct port_mibStatistics *port_stats)

  {
	  uint32 addrOffset_fromP0 =0;

	  if((port>CPU)||(port_stats==NULL) )
	  {
		  return FAILED;
	  }

	  addrOffset_fromP0= port* MIB_ADDROFFSETBYPORT;

	  memset(port_stats,0,sizeof(struct port_mibStatistics));
	 /* update the mib64 counters from 32bit counters*/
	  //rtl8651_updateAdvancedMibCounter((unsigned long)(&rtl865x_updateMib64Param)); //mark_rm

	 port_stats->ifInOctets=rtl8651_returnAsicCounter(OFFSET_IFINOCTETS_P0 + addrOffset_fromP0);
	 //rtl865xC_getAsicCounter64(OFFSET_IFINOCTETS_P0 + addrOffset_fromP0, &(port_stats->ifHCInOctets)); //mark_rm
	 port_stats->ifHCInOctets = port_stats->ifInOctets;

	 port_stats->ifInUcastPkts=rtl8651_returnAsicCounter(OFFSET_IFINUCASTPKTS_P0 + addrOffset_fromP0) ;
	 //port_stats->ifHCInUcastPkts = rtl865x_updateMib64Param[port].ifHCInUcastPkts;
	 port_stats->ifHCInUcastPkts = port_stats->ifInUcastPkts;

	 port_stats->ifInDiscards=rtl8651_returnAsicCounter(OFFSET_DOT1DTPPORTINDISCARDS_P0+ addrOffset_fromP0);//??
	 port_stats->ifInErrors=(rtl8651_returnAsicCounter( OFFSET_DOT3STATSFCSERRORS_P0 + addrOffset_fromP0 ) +
						  rtl8651_returnAsicCounter( OFFSET_ETHERSTATSJABBERS_P0 + addrOffset_fromP0 ));
	 port_stats->ifInMulticastPkts= rtl8651_returnAsicCounter( OFFSET_ETHERSTATSMULTICASTPKTS_P0 + addrOffset_fromP0 ) ;
	 //port_stats->ifHCInMulticastPkts= rtl865x_updateMib64Param[port].ifHCInMulticastPkts;
	 port_stats->ifHCInMulticastPkts = port_stats->ifInMulticastPkts;

	 port_stats->ifInBroadcastPkts= rtl8651_returnAsicCounter( OFFSET_ETHERSTATSBROADCASTPKTS_P0 + addrOffset_fromP0 ) ;
	 //port_stats->ifHCInBroadcastPkts= rtl865x_updateMib64Param[port].ifHCInBroadcastPkts;
  	  port_stats->ifHCInBroadcastPkts = port_stats->ifInBroadcastPkts;

	 //rtl865xC_getAsicCounter64(OFFSET_ETHERSTATSOCTETS_P0 + addrOffset_fromP0, &port_stats->etherStatsOctets );
	 port_stats->etherStatsOctets = rtl8651_returnAsicCounter(OFFSET_ETHERSTATSOCTETS_P0 + addrOffset_fromP0); //replace above , mark_rm
	 //port_stats->etherStatsOctets=rtl865xC_returnAsicCounter64(OFFSET_ETHERSTATSOCTETS_P0 + addrOffset_fromP0); //not by mark_rm
	 port_stats->etherStatsUndersizePkts=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSUNDERSIZEPKTS_P0 + addrOffset_fromP0);
	 port_stats->etherStatsFraments=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSFRAGMEMTS_P0 + addrOffset_fromP0);
	 port_stats->etherStatsPkts64Octets=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS64OCTETS_P0 + addrOffset_fromP0);
	 port_stats->etherStatsPkts65to127Octets=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS65TO127OCTETS_P0 + addrOffset_fromP0);
	 port_stats->etherStatsPkts128to255Octets=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS128TO255OCTETS_P0 + addrOffset_fromP0);
	 port_stats->etherStatsPkts256to511Octets=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS256TO511OCTETS_P0 + addrOffset_fromP0);
	 port_stats->etherStatsPkts512to1023Octets=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS512TO1023OCTETS_P0 + addrOffset_fromP0);
	 port_stats->etherStatsPkts1024to1518Octets=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS1024TO1518OCTETS_P0 + addrOffset_fromP0);
	 port_stats->etherStatsOversizePkts=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSOVERSIZEPKTS_P0 + addrOffset_fromP0);
	 port_stats->etherStatsJabbers=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSJABBERS_P0 + addrOffset_fromP0);
	 port_stats->etherStatusDropEvents=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSDROPEVENTS_P0 + addrOffset_fromP0);
	 port_stats->dot3FCSErrors=rtl8651_returnAsicCounter( OFFSET_DOT3STATSFCSERRORS_P0 + addrOffset_fromP0);
	 port_stats->dot3StatsSymbolErrors=rtl8651_returnAsicCounter( OFFSET_DOT3STATSSYMBOLERRORS_P0 + addrOffset_fromP0);
	 port_stats->dot3ControlInUnknownOpcodes=rtl8651_returnAsicCounter( OFFSET_DOT3CONTROLINUNKNOWNOPCODES_P0 + addrOffset_fromP0);
	 port_stats->dot3InPauseFrames=rtl8651_returnAsicCounter( OFFSET_DOT3INPAUSEFRAMES_P0 + addrOffset_fromP0);

	 port_stats->ifOutOctets=rtl8651_returnAsicCounter(OFFSET_IFOUTOCTETS_P0 + addrOffset_fromP0);
	 //rtl865xC_getAsicCounter64(OFFSET_IFOUTOCTETS_P0 + addrOffset_fromP0, &port_stats->ifHCOutOctets);
	  port_stats->ifHCOutOctets = port_stats->ifOutOctets;

	 // port_stats->ifHCOutOctets=rtl865xC_returnAsicCounter64(OFFSET_IFOUTOCTETS_P0 + addrOffset_fromP0);//not by mark_rm
	 port_stats->ifOutUcastPkts=rtl8651_returnAsicCounter(OFFSET_IFOUTUCASTPKTS_P0 + addrOffset_fromP0);
	 //port_stats->ifHCOutUcastPkts = rtl865x_updateMib64Param[port].ifHCOutUcastPkts;
	 port_stats->ifHCOutUcastPkts = port_stats->ifOutUcastPkts;
	 port_stats->ifOutDiscards=rtl8651_returnAsicCounter(OFFSET_IFOUTDISCARDS+ addrOffset_fromP0);
	 port_stats->ifOutErrors=(rtl8651_returnAsicCounter( OFFSET_ETHERSTATSCOLLISIONS_P0 + addrOffset_fromP0 ) +
						  rtl8651_returnAsicCounter( OFFSET_DOT3STATSDEFERREDTRANSMISSIONS_P0 + addrOffset_fromP0 ));
	 port_stats->ifOutMulticastPkts=rtl8651_returnAsicCounter(OFFSET_IFOUTMULTICASTPKTS_P0 + addrOffset_fromP0);
	 port_stats->ifOutBroadcastPkts=rtl8651_returnAsicCounter(OFFSET_IFOUTBROADCASTPKTS_P0 + addrOffset_fromP0);
	 //port_stats->ifHCOutMulticastPkts=rtl865x_updateMib64Param[port].ifHCOutMulticastPkts;
	 port_stats->ifHCOutMulticastPkts = port_stats->ifOutMulticastPkts;
	 //port_stats->ifHCOutBroadcastPkts=rtl865x_updateMib64Param[port].ifHCOutBroadcastPkts;
	 port_stats->ifHCOutBroadcastPkts = port_stats->ifOutBroadcastPkts;

	 port_stats->ifOutDiscards=rtl8651_returnAsicCounter(OFFSET_IFOUTDISCARDS + addrOffset_fromP0);
	 port_stats->dot3StatsSingleCollisionFrames=rtl8651_returnAsicCounter(OFFSET_DOT3STATSSINGLECOLLISIONFRAMES_P0+ addrOffset_fromP0);
	 port_stats->dot3StatsMultipleCollisionFrames=rtl8651_returnAsicCounter(OFFSET_DOT3STATSMULTIPLECOLLISIONFRAMES_P0 + addrOffset_fromP0);
	 port_stats->dot3StatsDefferedTransmissions=rtl8651_returnAsicCounter(OFFSET_DOT3STATSDEFERREDTRANSMISSIONS_P0 + addrOffset_fromP0);
	 port_stats->dot3StatsLateCollisions=rtl8651_returnAsicCounter(OFFSET_DOT3STATSLATECOLLISIONS_P0 + addrOffset_fromP0);
	 port_stats->dot3StatsExcessiveCollisions=rtl8651_returnAsicCounter(OFFSET_DOT3STATSEXCESSIVECOLLISIONS_P0 + addrOffset_fromP0);
	 port_stats->dot3OutPauseFrames=rtl8651_returnAsicCounter(OFFSET_DOT3OUTPAUSEFRAMES_P0 + addrOffset_fromP0);
	 port_stats->dot1dBasePortDelayExceededDiscards=rtl8651_returnAsicCounter(OFFSET_DOT1DBASEPORTDELAYEXCEEDEDDISCARDS_P0 + addrOffset_fromP0);
	 port_stats->etherStatsCollisions=rtl8651_returnAsicCounter(OFFSET_ETHERSTATSCOLLISIONS_P0 + addrOffset_fromP0);


	 //port_stats->ifInUnknownProtos = ethPortInUnknownProtos[port]; //mark_rm
	 port_stats->ifInUnknownProtos = 0;
	 port_stats->dot1dTpLearnedEntryDiscards=rtl8651_returnAsicCounter(MIB_ADDROFFSETBYPORT);
	 port_stats->etherStatsCpuEventPkts=rtl8651_returnAsicCounter(MIB_ADDROFFSETBYPORT);

	  return SUCCESS;
  }

static int show_port_mibStats(struct port_mibStatistics *port_stats, int port, char *page, int off)
{
	int len;
	//struct lan_port_status tmp_port_status;
	len = off;
	/*here is in counters  definition*/
	len += sprintf(page+len, "%s%u\n", "ifInOctets=", port_stats->ifInOctets);
	len += sprintf(page+len, "%s%llu\n", "ifHCInOctets=", port_stats->ifHCInOctets);
	len += sprintf(page+len, "%s%u\n", "ifInUcastPkts=", port_stats->ifInUcastPkts);
	len += sprintf(page+len, "%s%llu\n", "ifHCInUcastPkts=", port_stats->ifHCInUcastPkts);
	len += sprintf(page+len, "%s%u\n", "ifInMulticastPkts=", port_stats->ifInMulticastPkts);
	len += sprintf(page+len, "%s%llu\n", "ifHCInMulticastPkts=", port_stats->ifHCInMulticastPkts);
	len += sprintf(page+len, "%s%u\n", "ifInBroadcastPkts=", port_stats->ifInBroadcastPkts);
	len += sprintf(page+len, "%s%llu\n", "ifHCInBroadcastPkts=", port_stats->ifHCInBroadcastPkts);
	len += sprintf(page+len, "%s%u\n", "ifInDiscards=", port_stats->ifInDiscards);
	len += sprintf(page+len, "%s%u\n", "ifInErrors=", port_stats->ifInErrors);
	len += sprintf(page+len, "%s%llu\n", "etherStatsOctets=", port_stats->etherStatsOctets);
	len += sprintf(page+len, "%s%u\n", "etherStatsUndersizePkts=", port_stats->etherStatsUndersizePkts);
	len += sprintf(page+len, "%s%u\n", "etherStatsFraments=", port_stats->etherStatsFraments);
	len += sprintf(page+len, "%s%u\n", "etherStatsPkts64Octets=", port_stats->etherStatsPkts64Octets);
	len += sprintf(page+len, "%s%u\n", "etherStatsPkts65to127Octets=", port_stats->etherStatsPkts65to127Octets);
	len += sprintf(page+len, "%s%u\n", "etherStatsPkts128to255Octets=", port_stats->etherStatsPkts128to255Octets);
	len += sprintf(page+len, "%s%u\n", "etherStatsPkts256to511Octets=", port_stats->etherStatsPkts256to511Octets);
	len += sprintf(page+len, "%s%u\n", "etherStatsPkts512to1023Octets=", port_stats->etherStatsPkts512to1023Octets);
	len += sprintf(page+len, "%s%u\n", "etherStatsPkts1024to1518Octets=", port_stats->etherStatsPkts1024to1518Octets);
	len += sprintf(page+len, "%s%u\n", "etherStatsOversizePkts=", port_stats->etherStatsOversizePkts);
	len += sprintf(page+len, "%s%u\n", "etherStatsJabbers=", port_stats->etherStatsJabbers);
	len += sprintf(page+len, "%s%u\n", "dot1dTpPortInDiscards=", port_stats->dot1dTpPortInDiscards);
	len += sprintf(page+len, "%s%u\n", "etherStatusDropEvents=", port_stats->etherStatusDropEvents);
	len += sprintf(page+len, "%s%u\n", "dot3FCSErrors=", port_stats->dot3FCSErrors);
	len += sprintf(page+len, "%s%u\n", "dot3StatsSymbolErrors=", port_stats->dot3StatsSymbolErrors);
	len += sprintf(page+len, "%s%u\n", "dot3ControlInUnknownOpcodes=", port_stats->dot3ControlInUnknownOpcodes);
	len += sprintf(page+len, "%s%u\n", "dot3InPauseFrames=", port_stats->dot3InPauseFrames);
	/*here is out counters  definition*/
	len += sprintf(page+len, "%s%u\n", "ifOutOctets=", port_stats->ifOutOctets);
	len += sprintf(page+len, "%s%llu\n", "ifHCOutOctets=", port_stats->ifHCOutOctets);
	len += sprintf(page+len, "%s%u\n", "ifOutUcastPkts=", port_stats->ifOutUcastPkts);
	len += sprintf(page+len, "%s%llu\n", "ifHCOutUcastPkts=", port_stats->ifHCOutUcastPkts);
	len += sprintf(page+len, "%s%u\n", "ifOutMulticastPkts=", port_stats->ifOutMulticastPkts);
	len += sprintf(page+len, "%s%llu\n", "ifHCOutMulticastPkts=", port_stats->ifHCOutMulticastPkts);
	len += sprintf(page+len, "%s%u\n", "ifOutBroadcastPkts=", port_stats->ifOutBroadcastPkts);
	len += sprintf(page+len, "%s%llu\n", "ifHCOutBroadcastPkts=", port_stats->ifHCOutBroadcastPkts);
	len += sprintf(page+len, "%s%u\n", "ifOutDiscards=", port_stats->ifOutDiscards);
	len += sprintf(page+len, "%s%u\n", "ifOutErrors=", port_stats->ifOutErrors);
	len += sprintf(page+len, "%s%u\n", "dot3StatsSingleCollisionFrames=", port_stats->dot3StatsSingleCollisionFrames);
	len += sprintf(page+len, "%s%u\n", "dot3StatsMultipleCollisionFrames=", port_stats->dot3StatsMultipleCollisionFrames);
	len += sprintf(page+len, "%s%u\n", "dot3StatsDefferedTransmissions=", port_stats->dot3StatsDefferedTransmissions);
	len += sprintf(page+len, "%s%u\n", "dot3StatsLateCollisions=", port_stats->dot3StatsLateCollisions);
	len += sprintf(page+len, "%s%u\n", "dot3StatsExcessiveCollisions=", port_stats->dot3StatsExcessiveCollisions);
	len += sprintf(page+len, "%s%u\n", "dot3OutPauseFrames=", port_stats->dot3OutPauseFrames);
	len += sprintf(page+len, "%s%u\n", "dot1dBasePortDelayExceededDiscards=", port_stats->dot1dBasePortDelayExceededDiscards);
	len += sprintf(page+len, "%s%u\n", "etherStatsCollisions=", port_stats->etherStatsCollisions);
	/*here is whole system couters definition*/
	//mark_rm len += sprintf(page+len, "%s%u\n", "dot1dTpLearnedEntryDiscards=", port_stats->dot1dTpLearnedEntryDiscards);
	//mark_rm len += sprintf(page+len, "%s%u\n", "etherStatsCpuEventPkts=", port_stats->etherStatsCpuEventPkts);
	//len += sprintf(page+len, "%s%u\n", "ifInUnknownProtos=", port_stats->ifInUnknownProtos); //mark_rm
	//len += sprintf(page+len, "%s%u\n", "ifAdminStatus=", PortAdminStatus[port]);//mark_rm
	//len += sprintf(page+len, "%s%u\n", "ifOperStatus=", PortifOperStatus[port]);//mark_rm
	//len += sprintf(page+len, "%s%u\n", "ifLastChange=", PortLastChange[port]);//mark_rm
	//len += sprintf(page+len, "%s%u\n", "ifSpeed=",port_stats->ifSpeed);
	//len += sprintf(page+len, "%s%u\n", "ifHighSpeed=",port_stats->ifHighSpeed);
	//len += sprintf(page+len, "%s%u\n", "ifCounterDiscontinuityTime=", port_stats->ifCounterDiscontinuityTime);
	/*//mark_rm
	rtl819x_get_port_status(port, &tmp_port_status);
	if(tmp_port_status.link == 1)
		port_stats->ifConnectorPresent = 1;//link
	else
		port_stats->ifConnectorPresent = 2;//unlink
	len += sprintf(page+len, "%s%u\n", "ifConnectorPresent=", port_stats->ifConnectorPresent);
		*/

	return len;
}

static int port_mibStats_read_proc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{//TODO
	int len;
	uint32 port;
	struct port_mibStatistics port_stats;
	len = 0;
	port = (uint32)data;
	if(port > CPU)
		return 0;
	rtl819x_get_port_mibStats(port , &port_stats);
	len = show_port_mibStats(&port_stats, port, page, len);

	return len;
}

static int32 port_mibStats_write_proc( struct file *filp, const char *buff,unsigned long len, void *data )
{
	if (len < 2)
		return -EFAULT;

	rtl8651_clearAsicCounter();
	return len;
}


#endif	//#if defined(CONFIG_819X_PHY_RW)


static int read_proc_vlan(char *page, char **start, off_t off,
        int count, int *eof, void *data)
{

    struct net_device *dev = (struct net_device *)data;
    struct dev_priv *cp;
    int len;

	cp = dev->priv;
#if defined(CONFIG_RTK_VLAN_NEW_FEATURE)
		len = sprintf(page, "gvlan=%d, lan=%d, vlan=%d, tag=%d, vid=%d, priority=%d, cfi=%d, forwarding_rule=%d\n",
#else
		len = sprintf(page, "gvlan=%d, lan=%d, vlan=%d, tag=%d, vid=%d, priority=%d, cfi=%d\n",
#endif
		cp->vlan_setting.global_vlan, cp->vlan_setting.is_lan, cp->vlan_setting.vlan, cp->vlan_setting.tag,
		cp->vlan_setting.id, cp->vlan_setting.pri, cp->vlan_setting.cfi
#if defined(CONFIG_RTK_VLAN_NEW_FEATURE)
		,cp->vlan_setting.forwarding_rule
#endif
		);

    if (len <= off+count)
        *eof = 1;
    *start = page + off;
    len -= off;
    if (len > count)
        len = count;
    if (len < 0)
        len = 0;
    return len;
}

static int write_proc_vlan(struct file *file, const char *buffer,
              unsigned long count, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct dev_priv *cp;
	#define	VLAN_MAX_INPUT_LEN	128
	char *tmp;

	tmp = kmalloc(VLAN_MAX_INPUT_LEN, GFP_KERNEL);
	if (count < 2 || tmp==NULL)
		goto out;

	cp = dev->priv;
	if(rtk_vlan_support_enable == 0)
		goto out;

	if (buffer && !copy_from_user(tmp, buffer, VLAN_MAX_INPUT_LEN))
	{
#if defined(CONFIG_RTK_VLAN_NEW_FEATURE)
			int num = sscanf(tmp, "%d %d %d %d %d %d %d %d",
#else
			int num = sscanf(tmp, "%d %d %d %d %d %d %d",
#endif
			&cp->vlan_setting.global_vlan, &cp->vlan_setting.is_lan,
			&cp->vlan_setting.vlan, &cp->vlan_setting.tag,
			&cp->vlan_setting.id, &cp->vlan_setting.pri,
			&cp->vlan_setting.cfi
#if defined(CONFIG_RTK_VLAN_NEW_FEATURE)
			, &cp->vlan_setting.forwarding_rule
#endif
			);

#if defined(CONFIG_RTK_VLAN_NEW_FEATURE)
		if (num !=8)
#else
		if (num !=7)
#endif
		{
			printk("invalid vlan parameter!\n");
			goto out;
		}

#if defined(CONFIG_RTK_VLAN_NEW_FEATURE)
		rtl_add_vlan_info(&cp->vlan_setting, dev);
#endif
		#if 0
		printk("===%s(%d), cp->name(%s),global_vlan(%d),is_lan(%d),vlan(%d),tag(%d),id(%d),pri(%d),cfi(%d)",__FUNCTION__,__LINE__,
			cp->dev->name,cp->vlan_setting.global_vlan,cp->vlan_setting.is_lan,cp->vlan_setting.vlan,cp->vlan_setting.tag,
			cp->vlan_setting.id,cp->vlan_setting.pri,cp->vlan_setting.cfi);
		printk("-------------%s(%d),cp->portmask(%d)\n",__FUNCTION__,__LINE__,cp->portmask);
		#endif
	}
out:
	if(tmp)
		kfree(tmp);

	return count;
}
#endif // CONFIG_RTK_VLAN_SUPPORT

#if defined(CONFIG_RTK_VLAN_NEW_FEATURE)
static int rtk_vlan_management_read(char *page, char **start, off_t off,
        int count, int *eof, void *data)
{

    int len;

    len = sprintf(page, "Management vlan: vid=%d, priority=%d, cfi=%d\n",
		management_vlan.id, management_vlan.pri, management_vlan.cfi
		);

    if (len <= off+count)
        *eof = 1;
    *start = page + off;
    len -= off;
    if (len > count)
        len = count;
    if (len < 0)
        len = 0;
    return len;
}

static int rtk_vlan_management_write(struct file *file, const char *buffer,
              unsigned long len, void *data)
{
	char tmpbuf[128];
	int num;

	if (buffer && !copy_from_user(tmpbuf, buffer, len))
	{
		tmpbuf[len] = '\0';

		num = sscanf(tmpbuf, "%d %d %d",
			&management_vlan.id, &management_vlan.pri,
			&management_vlan.cfi);

		if (num !=  3) {
			printk("invalid vlan parameter!\n");
		}

	}

	return len;
}

#endif

#if defined(CONFIG_RTL_HW_VLAN_SUPPORT)

uint32 rtl_hw_vlan_get_tagged_portmask(void)
{
	uint32 portmask = 0;
	int i, temp;

	for(i=0; i<PORT_NUMBER; i++)
	{
		if((hw_vlan_info[i].vlan_port_enabled) && (hw_vlan_info[i].vlan_port_tag))
		{
			temp = (i==0)?0:(i-1);
			portmask |= 1<<temp;
		}
	}

	return portmask;
}

int rtl_process_hw_vlan_tx(rtl_nicTx_info *txInfo)
{
	int i, temp;
	int flag = 0;
	unsigned short vid;
	struct sk_buff *newskb;
	struct sk_buff *skb = NULL;
	unsigned short vid;
	struct vlan_tag tag, port_tag;
	struct vlan_tag *adding_tag = NULL;
	struct net_device *dev;
	struct dev_priv *cp;

	memset(&tag, 0, sizeof(struct vlan_tag));
	memset(&port_tag, 0, sizeof(struct vlan_tag));
	skb = txInfo->out_skb;
	dev = skb->dev;
	cp = dev->priv;

	newskb = NULL;
	if (skb_cloned(skb))
	{
		newskb = skb_copy(skb, GFP_ATOMIC);
		if (newskb == NULL)
		{
			cp->net_stats.tx_dropped++;
			dev_kfree_skb_any(skb);
			return FAILED;
		}
		dev_kfree_skb_any(skb);
		skb = newskb;
		txInfo->out_skb = skb;
	}

	//if((skb->data[0]&1) ||((skb->data[0]==0x33) &&(skb->data[1]==0x33) && (skb->data[2]!=0xFF)))
	{
		if(skb->tag.f.tpid == htons(ETH_P_8021Q))
		{
			vid = ntohs(skb->tag.f.pci & 0xfff);
			for(i=0; i<PORT_NUMBER; i++)
			{
				temp = (i==0)?i:(i-1);
				if((((struct dev_priv*)skb->dev->priv)->portmask & (0x1<<temp))&&(vid == hw_vlan_info[i].vlan_port_vid)&&
					(hw_vlan_info[i].vlan_port_bridge == 1)&&(hw_vlan_info[i].vlan_port_tag)&&(hw_vlan_info[i].vlan_port_enabled))
					{
						flag = 1;
						break;
					}
			}

			if(flag)
			{
				//printk("dev name is %s, portmask is 0x%x, i is %d, vid is %d\n", skb->dev->name, ((struct dev_priv*)skb->dev->priv)->portmask, i, vid);
				adding_tag = &skb->tag;
			}
		}else{
			for(i=0; i<PORT_NUMBER; i++)
			{
				if(((struct dev_priv*)skb->dev->priv)->portmask & (0x1<<i))
				{
					flag = 1;
					break;
				}
			}
			temp = (i==0)?i:(i+1);
			if((flag)&&(hw_vlan_info[temp].vlan_port_bridge == 1)&& (hw_vlan_info[temp].vlan_port_tag) &&
			   (hw_vlan_info[temp].vlan_port_enabled))
			{
				port_tag.f.tpid =  htons(ETH_P_8021Q);
				port_tag.f.pci = (unsigned short) ((0x3 << 13) |(0x1 << 12) |
							((unsigned short)(hw_vlan_info[temp].vlan_port_vid&0xfff)));
				port_tag.f.pci =  htons(port_tag.f.pci);
				adding_tag = &port_tag;
			}
		}

		if(adding_tag != NULL)
		{
			memcpy(&tag, skb->data+ETH_ALEN*2, VLAN_HLEN);
			if (tag.f.tpid !=  htons(ETH_P_8021Q)) { // tag not existed, insert tag
			if (skb_headroom(skb) < VLAN_HLEN && skb_cow(skb, VLAN_HLEN) !=0 ) {
				printk("%s-%d: error! (skb_headroom(skb) == %d < 4). Enlarge it!\n",
				__FUNCTION__, __LINE__, skb_headroom(skb));
				while (1) ;
			}
			skb_push(skb, VLAN_HLEN);
			memmove(skb->data, skb->data+VLAN_HLEN, ETH_ALEN*2);
			}

			memcpy(skb->data+ETH_ALEN*2, adding_tag, VLAN_HLEN);
		}

	}

	return SUCCESS;

}

static int32 rtl_hw_vlan_support_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
        int len;
	 int i;

	 len = sprintf(page, "rtl_hw_vlan_enable: %d\n", rtl_hw_vlan_enable);
	 for(i=0; i<PORT_NUMBER; i++)
	 {
	 	if(i == 1)
			continue;

		len += sprintf(page + len, "lan_%d_bridge_enabled: %d, lan_%d_bridge: %d, lan_%d_tag: %d, lan_%d_vid: %d\n",
			((i==0)? i: i-1), hw_vlan_info[i].vlan_port_enabled, ((i==0)? i: i-1), hw_vlan_info[i].vlan_port_bridge, ((i==0)? i: i-1), hw_vlan_info[i].vlan_port_tag,
			((i==0)? i: i-1), hw_vlan_info[i].vlan_port_vid);
	 }

	len += sprintf(page + len, "wan_port_enabled: %d, wan_port_tag: %d, wan_port_vid: %d\n", hw_vlan_info[1].vlan_port_enabled, hw_vlan_info[1].vlan_port_tag, hw_vlan_info[1].vlan_port_vid);

	if (len <= off+count) *eof = 1;
        *start = page + off;
        len -= off;
        if (len>count)
                len = count;
        if (len<0)
                len = 0;
        return len;
}


static int32 rtl_hw_vlan_support_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
        char tmpbuf[100];
        int lan_portmask = 0;
        int num=0 ;
	 int i=0;
	 int j=0;
	 int bridge_num = 0;
	 struct net_device *dev;
	 struct dev_priv	  *dp;

        if (buff && !copy_from_user(tmpbuf, buff, len))
        {
		num = sscanf(tmpbuf, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
			&rtl_hw_vlan_enable, &hw_vlan_info[1].vlan_port_enabled, &hw_vlan_info[1].vlan_port_tag, &hw_vlan_info[1].vlan_port_vid,
			&hw_vlan_info[0].vlan_port_enabled, &hw_vlan_info[0].vlan_port_bridge, &hw_vlan_info[0].vlan_port_tag, &hw_vlan_info[0].vlan_port_vid,
			&hw_vlan_info[2].vlan_port_enabled, &hw_vlan_info[2].vlan_port_bridge, &hw_vlan_info[2].vlan_port_tag, &hw_vlan_info[2].vlan_port_vid,
			&hw_vlan_info[3].vlan_port_enabled, &hw_vlan_info[3].vlan_port_bridge, &hw_vlan_info[3].vlan_port_tag, &hw_vlan_info[3].vlan_port_vid,
			&hw_vlan_info[4].vlan_port_enabled, &hw_vlan_info[4].vlan_port_bridge, &hw_vlan_info[4].vlan_port_tag, &hw_vlan_info[4].vlan_port_vid
			);

               if (num !=  20) {
                    printk("invalid rtl_hw_vlan_support_write parameter!\n");
                    return len;
               }

		rtl_config_rtkVlan_vlanconfig(rtl865x_curOpMode);

		/*get rid of port who bridge with wan from RTL_LANPORT_MASK*/
	       if(rtl_hw_vlan_enable && (rtl865x_curOpMode == GATEWAY_MODE))
	       {
	            lan_portmask =  RTL_LANPORT_MASK;

			if((hw_vlan_info[0].vlan_port_enabled == 1)&&(hw_vlan_info[0].vlan_port_bridge == 1))
				lan_portmask &= ~RTL_LANPORT_MASK_4;  //port0 is bridge, so port0 need to get out of lan_port_mask
			if((hw_vlan_info[2].vlan_port_enabled == 1)&&(hw_vlan_info[2].vlan_port_bridge == 1))
				lan_portmask &= ~RTL_LANPORT_MASK_3;
			if((hw_vlan_info[3].vlan_port_enabled == 1)&&(hw_vlan_info[3].vlan_port_bridge == 1))
				lan_portmask &= ~RTL_LANPORT_MASK_2;
			if((hw_vlan_info[4].vlan_port_enabled == 1)&&(hw_vlan_info[4].vlan_port_bridge == 1))
				lan_portmask &= ~RTL_LANPORT_MASK_1;

	            vlanconfig[0].memPort = lan_portmask;
	            vlanconfig[0].vid= RTL_LANVLANID;
	            vlanconfig[0].untagSet = lan_portmask;
	            ((struct dev_priv *)_rtl86xx_dev.dev[0]->priv)->portmask = lan_portmask; //eth0
	            ((struct dev_priv *)_rtl86xx_dev.dev[0]->priv)->id = RTL_LANVLANID;

	            vlanconfig[1].vid = hw_vlan_info[1].vlan_port_vid;
	            vlanconfig[1].memPort = RTL_WANPORT_MASK;
	            vlanconfig[1].untagSet = 0; // need tag
	            ((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->portmask = RTL_WANPORT_MASK; //eth1
	            ((struct dev_priv *)_rtl86xx_dev.dev[1]->priv)->id = hw_vlan_info[1].vlan_port_vid; //eth1

			for(i=0; i<PORT_NUMBER; i++)
			{
				if(i==1)	/*bypass eth1*/
					continue;

				if((hw_vlan_info[i].vlan_port_enabled == 1)&&(hw_vlan_info[i].vlan_port_bridge == 1))
				{
					if(i == 0)
					{
						vlanconfig[2+bridge_num].memPort = 1<<i;
						vlanconfig[2+bridge_num].vid = hw_vlan_info[i].vlan_port_vid;
						vlanconfig[2+bridge_num].untagSet = 1<<i;
						((struct dev_priv *)_rtl86xx_dev.dev[2+bridge_num]->priv)->portmask = 1<<i;
						((struct dev_priv *)_rtl86xx_dev.dev[2+bridge_num]->priv)->id = hw_vlan_info[i].vlan_port_vid;
						bridge_num ++;
					}else{
						vlanconfig[2+bridge_num].memPort = 1<<(i-1);
						vlanconfig[2+bridge_num].vid = hw_vlan_info[i].vlan_port_vid;
						vlanconfig[2+bridge_num].untagSet = 1<<(i-1);
						((struct dev_priv *)_rtl86xx_dev.dev[2+bridge_num]->priv)->portmask = 1<<(i-1);
						((struct dev_priv *)_rtl86xx_dev.dev[2+bridge_num]->priv)->id = hw_vlan_info[i].vlan_port_vid;
						bridge_num ++;
					}
				}
			}
			//PPPOE
			   vlanconfig[5].vid = hw_vlan_info[1].vlan_port_vid;
	        }


		re865x_packVlanConfig(vlanconfig, packedVlanConfig);

		rtl_reinit_hw_table();
		reinit_vlan_configure(packedVlanConfig);

		//unknow vlan drop
		//REG32(SWTCR0) &= ~(1 << 15);

		/*update dev port number*/
		for(i=0; vlanconfig[i].vid != 0; i++)
		{
			if (IF_ETHER!=vlanconfig[i].if_type)
			{
				continue;
			}

			dev=_rtl86xx_dev.dev[i];
			dp = dev->priv;
			dp->portnum  = 0;
			for(j=0;j<RTL8651_AGGREGATOR_NUMBER;j++)
			{
				if(dp->portmask & (1<<j))
					dp->portnum++;
			}

		}

		#if defined (CONFIG_RTL_IGMP_SNOOPING)
			re865x_reInitIgmpSetting(rtl865x_curOpMode);
		#if defined (CONFIG_RTL_MLD_SNOOPING)
			if(mldSnoopEnabled && (rtk_vlan_support_enable==0))
			{
				re865x_packVlanConfig(vlanconfig, packedVlanConfig);
				rtl865x_addAclForMldSnooping(packedVlanConfig);
			}
		#endif
		#endif

		//always init the default route...
		if(rtl8651_getAsicOperationLayer() >2)
		{
			rtl865x_addRoute(0,0,0,RTL_DRV_WAN0_NETIF_NAME,0);
		}

               rtl8651_setAsicMulticastEnable(TRUE);
        }
        return len;
}


static int32 rtl_hw_vlan_tagged_bridge_multicast_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;
	len = sprintf(page, "%s %d\n", "rtl_hw_vlan_tagged_mc:",rtl_hw_vlan_ignore_tagged_mc);


	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count)
		len = count;
	if (len<0)
	  	len = 0;

	return len;
}

static int32 rtl_hw_vlan_tagged_bridge_multicast_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char 		tmpbuf[32];

	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
		tmpbuf[len] = '\0';

		rtl_hw_vlan_ignore_tagged_mc = tmpbuf[0] - '0';
	}
	return len;
}


#endif


#if (defined(CONFIG_RTL_CUSTOM_PASSTHRU) && !defined(CONFIG_RTL8196_RTL8366))

static unsigned long atoi_dec(char *s)
{
	unsigned long k = 0;

	k = 0;
	while (*s != '\0' && *s >= '0' && *s <= '9') {
		k = 10 * k + (*s - '0');
		s++;
	}
	return k;
}

static int custom_Passthru_read_proc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
	int len;
	len = sprintf(page, "%s\n", passThru_flag);
	if (len <= off+count)
		*eof = 1;

	*start = page + off;
	len -= off;

	if (len>count)
		len = count;

	if (len<0) len = 0;

	return len;
}

static int custom_createPseudoDevForPassThru(void)
{
		struct net_device *dev, *wanDev;
		struct dev_priv *dp;
		int rc, i;
		unsigned long		flags;

		wanDev = NULL;
		/*	find wan device first	*/
		for(i=0;i<ETH_INTF_NUM;i++)
		{
			dp = _rtl86xx_dev.dev[i]->priv;
			if (rtl_isWanDev(dp)==TRUE)
			{
				wanDev = _rtl86xx_dev.dev[i];
				break;
			}
		}

		/*	can't find any wan device, just return	*/
		if (wanDev==NULL)
			return -1;

		dev = alloc_etherdev(sizeof(struct dev_priv));
		if (!dev){
			rtlglue_printf("failed to allocate passthru pseudo dev.\n");
			return -1;
		}
		strcpy(dev->name, "peth%d");
		/*	default set lan side mac		*/
		memcpy((char*)dev->dev_addr,(char*)(&(vlanconfig[0].mac)),ETHER_ADDR_LEN);
		dp = dev->priv;
		memset(dp,0,sizeof(*dp));
		dp->dev = wanDev;
#if defined(CONFIG_COMPAT_NET_DEV_OPS)
		dev->open = re865x_pseudo_open;
		dev->stop = re865x_pseudo_close;
		dev->set_multicast_list = NULL;
		dev->hard_start_xmit = re865x_start_xmit;
		dev->get_stats = re865x_get_stats;
		dev->do_ioctl = re865x_ioctl;
#ifdef CP_VLAN_TAG_USED
		dev->features |= NETIF_F_HW_VLAN_TX | NETIF_F_HW_VLAN_RX;
		dev->vlan_rx_register = cp_vlan_rx_register;
		dev->vlan_rx_kill_vid = cp_vlan_rx_kill_vid;
#endif

#else
		dev->netdev_ops = &rtl819x_pseudodev_ops;
#endif

#ifdef CP_VLAN_TAG_USED
		dev->features |= NETIF_F_HW_VLAN_TX | NETIF_F_HW_VLAN_RX;
#endif
		dev->watchdog_timeo = TX_TIMEOUT;

		dev->irq = 0;
		rc = register_netdev(dev);
		if(!rc){
			local_irq_save(flags);
			//spin_lock_irqsave(&_rtl86xx_dev.lock, flags);
			_rtl86xx_dev.pdev = dev;
			//spin_unlock_irqrestore(&_rtl86xx_dev.lock, flags);
			local_irq_restore(flags);
			rtlglue_printf("[%s] added, mapping to [%s]...\n", dev->name, dp->dev->name);
		}else
			rtlglue_printf("Failed to allocate [%s]\n", dev->name);

		return 0;
}

static int custom_Passthru_write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	int flag,i;

#if	!defined(CONFIG_RTL_LAYERED_DRIVER)
	struct rtl865x_vlanConfig  passThruVlanConf = {"passThru",0,IF_ETHER,PASSTHRU_VLAN_ID,0,
		rtl865x_lanPortMask|rtl865x_wanPortMask,
		rtl865x_lanPortMask|rtl865x_wanPortMask,
		1500,{ { 0x00, 0x12, 0x34, 0x56, 0x78, 0x90 } } };
#endif

	if (buffer && !copy_from_user(&passThru_flag, buffer, count))
	{
		flag=(int)atoi_dec(passThru_flag);

		if(flag ^ oldStatus)
		{
			//IPv6 PassThru
			if((flag & IP6_PASSTHRU_MASK) ^ (oldStatus & IP6_PASSTHRU_MASK))
			{
				if(flag & IP6_PASSTHRU_MASK)
				{//add
					for(i=0; i<RTL865XC_PORT_NUMBER; i++)
					{
						rtl8651_setProtocolBasedVLAN(IP6_PASSTHRU_RULEID, i, TRUE, PASSTHRU_VLAN_ID);
					}
				}
				else
				{//delete
					for(i=0; i<RTL865XC_PORT_NUMBER; i++)
					{
						rtl8651_setProtocolBasedVLAN(IP6_PASSTHRU_RULEID, i, FALSE, PASSTHRU_VLAN_ID);
					}
				}
			}

			#if defined(CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE)
			//PPPoE PassThru
			if((flag & PPPOE_PASSTHRU_MASK) ^ (oldStatus & PPPOE_PASSTHRU_MASK))
			{
				if(flag & PPPOE_PASSTHRU_MASK)
				{//add
					for(i=0; i<RTL865XC_PORT_NUMBER; i++)
					{
						rtl8651_setProtocolBasedVLAN(RTL8651_PBV_RULE_PPPOE_CONTROL, i, TRUE, PASSTHRU_VLAN_ID);
						rtl8651_setProtocolBasedVLAN(RTL8651_PBV_RULE_PPPOE_SESSION, i, TRUE, PASSTHRU_VLAN_ID);
					}
				}
				else
				{//delete
					for(i=0; i<RTL865XC_PORT_NUMBER; i++)
					{
						rtl8651_setProtocolBasedVLAN(RTL8651_PBV_RULE_PPPOE_CONTROL, i, FALSE, PASSTHRU_VLAN_ID);
						rtl8651_setProtocolBasedVLAN(RTL8651_PBV_RULE_PPPOE_SESSION, i, FALSE, PASSTHRU_VLAN_ID);
					}
				}
			}
			#endif
		}

		//passthru vlan
		if(flag==0)
		{
			//To del passthru vlan
			//Do nothing here because change_op_mode reinit vlan already
		}
		else
            {
            		//To add passthru vlan
			rtl865x_addVlan(PASSTHRU_VLAN_ID);
			#if defined(CONFIG_POCKET_ROUTER_SUPPORT)
			rtl865x_addVlanPortMember(PASSTHRU_VLAN_ID, rtl865x_lanPortMask|rtl865x_wanPortMask|0x100);
			#else
			rtl865x_addVlanPortMember(PASSTHRU_VLAN_ID, rtl865x_lanPortMask|rtl865x_wanPortMask);
			#endif
            }

		oldStatus=flag;
		return count;
	}
	return -EFAULT;
}

static int32 rtl8651_customPassthru_init(void)
{
	oldStatus=0;
	res = create_proc_entry("custom_Passthru", 0, NULL);
	if(res)
	{
		res->read_proc = custom_Passthru_read_proc;
		res->write_proc = custom_Passthru_write_proc;
	}
	rtl8651_defineProtocolBasedVLAN( IP6_PASSTHRU_RULEID, 0x0, __constant_htons(ETH_P_IPV6) );
	#if defined(CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE)
	rtl8651_defineProtocolBasedVLAN( PPPOE_PASSTHRU_RULEID1, 0x0, __constant_htons(ETH_P_PPP_SES) );
	rtl8651_defineProtocolBasedVLAN( PPPOE_PASSTHRU_RULEID2, 0x0, __constant_htons(ETH_P_PPP_DISC) );
	#endif
	custom_createPseudoDevForPassThru();
	return 0;
}

static void __exit rtl8651_customPassthru_exit(void)
{
	if (res) {
		remove_proc_entry("custom_Passthru", res);
		res = NULL;
	}
}
#define	MULTICAST_STORM_CONTROL	1
#define	BROADCAST_STORM_CONTROL	2

static struct proc_dir_entry *stormCtrlProc=NULL;
static int rtl865x_stormCtrlReadProc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
	int len=0;
	if (len <= off+count)
		*eof = 1;

	*start = page + off;
	len -= off;

	if (len>count)
		len = count;

	if (len<0) len = 0;

	return len;
}

static int rtl865x_stormCtrlWriteProc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	uint32 tmpBuf[32];
	uint32 stormCtrlType=MULTICAST_STORM_CONTROL;
	uint32 enableStormCtrl=FALSE;
	uint32 percentage=0;
	uint32 uintVal;

	if (buffer && !copy_from_user(tmpBuf, buffer, count))
	{
		tmpBuf[count-1]=0;
		uintVal=simple_strtol((const char *)tmpBuf, NULL, 0);
		//printk("%s(%d),tmpBuf=%s,count=%d,uintVal=%d\n",__FUNCTION__,__LINE__,
		//	tmpBuf,count,uintVal);//Added for test
		if(uintVal>100)
		{
			enableStormCtrl=FALSE;
			percentage=0;
		}
		else
		{
			enableStormCtrl=TRUE;
			percentage=uintVal;
		}
		rtl865x_setStormControl(stormCtrlType,enableStormCtrl,percentage);
		return count;
	}
	return -EFAULT;
}

int32 rtl8651_initStormCtrl(void)
{
	stormCtrlProc = create_proc_entry("StormCtrl", 0, NULL);
	if(stormCtrlProc)
	{
		stormCtrlProc->read_proc = rtl865x_stormCtrlReadProc;
		stormCtrlProc->write_proc = rtl865x_stormCtrlWriteProc;
	}

	return 0;
}

void __exit rtl8651_exitStormCtrl(void)
{
	if (stormCtrlProc) {
		remove_proc_entry("StormCtrl", stormCtrlProc);
		stormCtrlProc = NULL;
	}
}


#endif

#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
static int32 proc_phyTest_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	return 0;
}

static int32 proc_phyTest_write( struct file *filp, const char *buff,unsigned long len1, void *data )
{
	char 		tmpbuf[64];
	uint32	phyId=0, regId=0,iNo=0,iPort=0,len=0,regData=0;
	char		*strptr, *cmd_addr;
	char		*tokptr;
	int32	i=0,port=0;
	int32 	ret=FAILED;
	if (buff && !copy_from_user(tmpbuf, buff, len1)) {
		tmpbuf[len1-1] = '\0';
		strptr=tmpbuf;
		cmd_addr = strsep(&strptr," ");
		if (cmd_addr==NULL)
		{
			goto errout;
		}

		if (!memcmp(cmd_addr, "read", 4))
		{

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);

			ret=rtl8651_getAsicEthernetPHYReg(phyId, regId, &regData);
			if(ret==SUCCESS)
			{
				printk("read phyId(%d), regId(%d),regData:0x%x\n", phyId, regId, regData);
			}
			else
			{
				printk("error input!\n");
//				goto errout;
			}
		}
		else if (!memcmp(cmd_addr, "test", 4))
		{
			printk("\r\n");
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			iNo=simple_strtol(tokptr, NULL, 0);
			if(iNo==1)//test mode 1
			{

				unsigned int default_val_t1[]={
				4,0x1f,0x0002,
				4,0x13,0xAA00,
				4,0x14,0xAA00,
				4,0x15,0xAA00,
				4,0x16,0xFA00,
				4,0x17,0xAF00
				};
				printk("PHY Test 1 Mode: No\n");
				for(i=0; i<5; i++)
				REG32(PCRP0+i*4) |= (EnForceMode);
				len=sizeof(default_val_t1)/sizeof(unsigned int);

				for(i=0;i<len;i=i+3)
				{
				port=default_val_t1[i];
				rtl8651_setAsicEthernetPHYReg(port, default_val_t1[i+1], default_val_t1[i+2]);
				}
				for(i=0; i<5; i++)
				{
					rtl8651_setAsicEthernetPHYReg(i, 0x1f, 0x0000);
					rtl8651_setAsicEthernetPHYReg(i, 0x09, 0x2E00);
				}
				for(i=0; i<5; i++)
				REG32(PCRP0+i*4) &= ~(EnForceMode);
				printk("Please reset the target after leaving Test Mode\n");
				ret=SUCCESS;
			}
	#if 0
			else if(iNo==2)//test mode 2
			{

				unsigned int default_val_t2[]={
				1, 0x1f, 0x0002,
				1, 0x11, 0x5E00,
				1, 0x1f, 0x0000,
				4, 0x1f, 0x0002,
				4, 0x11, 0x5E00,
				4, 0x1f, 0x0000

				};
				printk("PHY Test 2 Mode: No\n");
				for(i=0; i<5; i++)
				REG32(PCRP0+i*4) |= (EnForceMode);
				len=sizeof(default_val_t2)/sizeof(unsigned int);

				for(i=0;i<len;i=i+3)
				{
				port=default_val_t2[i];
				rtl8651_setAsicEthernetPHYReg(port, default_val_t2[i+1], default_val_t2[i+2]);
				}
				for(i=0; i<5; i++)
				{
					rtl8651_setAsicEthernetPHYReg(i, 0x1f, 0x0000);
					rtl8651_setAsicEthernetPHYReg(i, 0x09, 0x4E00);
				}
				for(i=0; i<5; i++)
				REG32(PCRP0+i*4) &= ~(EnForceMode);
				printk("Please reset the target after leaving Test Mode\n");
				ret=SUCCESS;
			}
			else if(iNo==3)//test mode 2
			{
				unsigned int default_val_t3[]={
				1, 0x1f, 0x0002,
				1, 0x11, 0x4000,
				};
				for(i=0; i<5; i++)
				REG32(PCRP0+i*4) |= (EnForceMode);

				printk("PHY Test 3 Mode: No  Port{0~4} Channel{A,B,C,D}\n");
				tokptr = strsep(&strptr," ");
				if (tokptr==NULL)
				{
					goto errout;
				}
				iPort=simple_strtol(tokptr, NULL, 0);
				tokptr = strsep(&strptr," ");
				if (tokptr==NULL)
				{
					goto errout;
				}
				len=sizeof(default_val_t3)/sizeof(unsigned int);
				for(i=0;i<len;i=i+3)
				{
					port=default_val_t3[i];
					rtl8651_setAsicEthernetPHYReg(port, default_val_t3[i+1], default_val_t3[i+2]);
				}

				switch(*tokptr)
				{

					case 'A':
						rtl8651_setAsicEthernetPHYReg(1, 0x10, 0x1100);
						rtl8651_setAsicEthernetPHYReg(1, 0x1f, 0x0000);
						printk("A channel\n");

					break;
					case 'B':
						rtl8651_setAsicEthernetPHYReg(1, 0x10, 0x1300);
						rtl8651_setAsicEthernetPHYReg(1, 0x1f, 0x0000);
						printk("B channel\n");
					break;
					case 'C':
						rtl8651_setAsicEthernetPHYReg(1, 0x10, 0x1500);
						rtl8651_setAsicEthernetPHYReg(1, 0x1f, 0x0000);
						printk("C channel\n");
					break;
					case 'D':
						rtl8651_setAsicEthernetPHYReg(1, 0x10, 0x1700);
						rtl8651_setAsicEthernetPHYReg(1, 0x1f, 0x0000);
						printk("D channel\n");
					break;
				}
				rtl8651_setAsicEthernetPHYReg(iPort, 0x1f, 0x0000);
				rtl8651_setAsicEthernetPHYReg(iPort, 0x09, 0x6e00);
				for(i=0; i<5; i++)
				REG32(PCRP0+i*4) &= ~(EnForceMode);
				printk("Please reset the target after leaving Test Mode\n");
				ret=SUCCESS;
			}
	#endif
			else if(iNo==4)//test mode 2
			{
				unsigned int default_val_t4[]={
				0, 0x1f, 0x0002,
				0, 0x07, 0x3678,
				0, 0x1f, 0x0000,
				0, 0x09, 0x8e00
				};

				for(i=0; i<5; i++)
				REG32(PCRP0+i*4) |= (EnForceMode);

				len=sizeof(default_val_t4)/sizeof(unsigned int);



				tokptr = strsep(&strptr," ");
				if (tokptr==NULL)
				{
					goto errout;
				}
				iPort=simple_strtol(tokptr, NULL, 0);
                                printk("PHY Test 4 Mode: No:%d  Port:%d \n",iNo,iPort);
				for(i=0;i<len;i=i+3)
				{
					rtl8651_setAsicEthernetPHYReg(iPort, default_val_t4[i+1], default_val_t4[i+2]);
				}

				for(i=0; i<5; i++)
				REG32(PCRP0+i*4) &= ~(EnForceMode);
				printk("Please reset the target after leaving Test Mode\n");
				ret=SUCCESS;
			}


#if 0
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regData=simple_strtol(tokptr, NULL, 0);

			ret=rtl8651_setAsicEthernetPHYReg(phyId, regId, regData);
#endif
			if(ret==SUCCESS)
			{
				//printk("Write phyId(%d), regId(%d), regData:0x%x\n", phyId, regId, regData);
			}
			else
			{
				printk("error input!\n");
//				goto errout;
			}
		}
		else
		{
			goto errout;
		}

		return len1;

errout:
		printk("\nTest  format:	\"Test 1~4\"\n");
		printk("PHY Test 1 Mode: No\n");
		//printk("PHY Test 2 Mode: No\n");
		//printk("PHY Test 3 Mode: No  Port{0~4} Channel{A,B,C,D}\n");
		printk("PHY Test 4 Mode: No  Port{0~4} \n");

		return len1;
	}

	return -EFAULT;
}

int32 phyRegTest_init(void)
{
	phyTest_entry= create_proc_entry("phyRegTest", 0, NULL);
	if(phyTest_entry != NULL)
	{
		phyTest_entry->read_proc = proc_phyTest_read;
		phyTest_entry->write_proc = proc_phyTest_write;
	}

	return 0;
}

void __exit phyRegTest_exit(void)
{
	if (phyTest_entry)
	{
		remove_proc_entry("phyTest_entry", phyTest_entry);
		phyTest_entry = NULL;
	}
}
#endif

#if defined (CONFIG_RTL_MLD_SNOOPING)

static struct proc_dir_entry *mldSnoopingProc=NULL;


static int rtl865x_mldSnoopingReadProc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
	int len;
	len = sprintf(page, "%s\n", (mldSnoopEnabled==TRUE)?"enable":"disable");
	if (len <= off+count)
		*eof = 1;

	*start = page + off;
	len -= off;

	if (len>count)
		len = count;

	if (len<0) len = 0;

	return len;
}

static int rtl865x_mldSnoopingWriteProc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	uint32 tmpBuf[32];
	uint32 uintVal;

	if (buffer && !copy_from_user(tmpBuf, buffer, count))
	{
		tmpBuf[count-1]=0;
		uintVal=simple_strtol((const char *)tmpBuf, NULL, 0);
		if(uintVal!=0)
		{
			if(mldSnoopEnabled==FALSE)
			{
				rtl865x_removeAclForMldSnooping(vlanconfig);
				rtl865x_addAclForMldSnooping(vlanconfig);
				mldSnoopEnabled=TRUE;
			}
		}
		else
		{
			if(mldSnoopEnabled==TRUE)
			{
				rtl865x_removeAclForMldSnooping(vlanconfig);
				mldSnoopEnabled=FALSE;
			}
		}

		return count;
	}
	return -EFAULT;
}

int32 rtl8651_initMldSnooping(void)
{
	mldSnoopingProc = create_proc_entry("br_mldsnoop", 0, NULL);
	if(mldSnoopingProc)
	{
		mldSnoopingProc->read_proc = rtl865x_mldSnoopingReadProc;
		mldSnoopingProc->write_proc = rtl865x_mldSnoopingWriteProc;
	}
	 mldSnoopEnabled=FALSE;
	return 0;
}

void __exit rtl8651_exitMldSnoopingCtrl(void)
{
	if (mldSnoopingProc) {
		remove_proc_entry("br_mldsnoop", mldSnoopingProc);
		mldSnoopingProc = NULL;
	}
}

#endif

#if defined (CONFIG_RTL_PHY_POWER_CTRL)
static struct proc_dir_entry *phyPowerCtrlProc=NULL;
#define PHY_POWER_OFF 0
#define PHY_POWER_ON 1
static int rtl865x_setPhyPowerOff(unsigned int port)
{
	unsigned int offset = port * 4;
	unsigned int pcr = 0;
	unsigned int regData;
	unsigned int macLinkStatus;
	unsigned int phyLinkStatus;
	if(port >=RTL8651_PHY_NUMBER)
	{
		return -1;
	}

	/*must set mac force link down first*/
	pcr=READ_MEM32( PCRP0 + offset );
	pcr |= EnForceMode;
	pcr &= ~ForceLink;
	WRITE_MEM32( PCRP0 + offset, pcr );
	TOGGLE_BIT_IN_REG_TWICE(PCRP0 + offset,EnForceMode);

	rtl8651_getAsicEthernetPHYReg(port, 0, &regData);
	regData=regData |(1<<11);
	rtl8651_setAsicEthernetPHYReg(port, 0, regData);

	/*confirm shutdown phy power successfully*/
	regData = READ_MEM32(PSRP0+offset);
	macLinkStatus = regData & PortStatusLinkUp;

	rtl8651_getAsicEthernetPHYReg(port, 0, &regData);
	phyLinkStatus=!(regData & (1<<11));
	while((macLinkStatus) || (phyLinkStatus) )
	{
		//printk("port is %d,macLinkStatus is %d,phyLinkStatus is %d\n",port,macLinkStatus,phyLinkStatus);
		pcr=READ_MEM32( PCRP0 + offset );
		pcr |= EnForceMode;
		pcr &= ~ForceLink;
		WRITE_MEM32( PCRP0 + offset, pcr );
		TOGGLE_BIT_IN_REG_TWICE(PCRP0 + offset,EnForceMode);

		rtl8651_getAsicEthernetPHYReg(port, 0, &regData);
		regData=regData |(1<<11);
		rtl8651_setAsicEthernetPHYReg(port, 0, regData);

		regData = READ_MEM32(PSRP0+offset);
		macLinkStatus = regData & PortStatusLinkUp;

		rtl8651_getAsicEthernetPHYReg(port, 0, &regData);
		phyLinkStatus=!(regData & (1<<11));
	}
	return 0;

}

static int rtl865x_setPhyPowerOn(unsigned int port)
{
	unsigned int  offset = port * 4;
	unsigned int pcr = 0;

	unsigned int regData;

	if(port >=RTL8651_PHY_NUMBER)
	{
		return -1;
	}

	pcr=READ_MEM32( PCRP0 + offset );
	pcr |= EnForceMode;

	WRITE_MEM32( PCRP0 + offset, pcr );
	TOGGLE_BIT_IN_REG_TWICE(PCRP0 + offset,EnForceMode);

	rtl8651_getAsicEthernetPHYReg(port, 0, &regData);
	regData=regData &(~(1<<11));
	rtl8651_setAsicEthernetPHYReg(port, 0, regData);

	pcr &=~EnForceMode;
	WRITE_MEM32( PCRP0 + offset, pcr);
	TOGGLE_BIT_IN_REG_TWICE(PCRP0 + offset,EnForceMode);
	return 0;

}


static int rtl865x_phyPowerCtrlReadProc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
	int len=0;
	int port;
	unsigned int regData;
	for(port=0;port<RTL8651_PHY_NUMBER;port++)
	{
		rtl8651_getAsicEthernetPHYReg(port, 0, &regData);
		if(regData & (1<<11))
		{
			len += sprintf(page+len, "port[%d] phy is power off\n",port );
		}
		else
		{
			len += sprintf(page+len, "port[%d] phy is power on\n",port );
		}
	}

	if (len <= off+count)
		*eof = 1;

	*start = page + off;
	len -= off;

	if (len>count)
		len = count;

	if (len<0) len = 0;

	return len;
}

static int rtl865x_phyPowerCtrlWriteProc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	char tmpBuf[256];
	char		*strptr;
	char		*tokptr;
	unsigned int  portMask;
	unsigned int action;
	int i;
	if (buffer && !copy_from_user(tmpBuf, buffer, count))
	{
		tmpBuf[count-1]=0;
		strptr=tmpBuf;

		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errOut;
		}
		portMask=simple_strtol(tokptr, NULL, 0);

		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errOut;
		}
		action=simple_strtol(tokptr, NULL, 0);

		for(i=0;i<RTL8651_PHY_NUMBER;i++)
		{
			if((1<<i) &portMask)
			{
				if(action==PHY_POWER_OFF)
				{
					rtl865x_setPhyPowerOff(i);
				}
				else if (action==PHY_POWER_ON)
				{
					rtl865x_setPhyPowerOn(i);
				}
			}

		}

		return count;
	}

errOut:
	return -EFAULT;
}


static int32 rtl865x_initPhyPowerCtrl(void)
{
	phyPowerCtrlProc = create_proc_entry("phyPower", 0, NULL);
	if(phyPowerCtrlProc)
	{
		phyPowerCtrlProc->read_proc = rtl865x_phyPowerCtrlReadProc;
		phyPowerCtrlProc->write_proc = rtl865x_phyPowerCtrlWriteProc;
	}

	return 0;
}

void __exit rtl865x_exitPhyPowerCtrl(void)
{
	if (phyPowerCtrlProc) {
		remove_proc_entry("phyPower", phyPowerCtrlProc);
		phyPowerCtrlProc = NULL;
	}
}

#endif


#if defined(CONFIG_RTL_LINKSTATE)

static struct proc_dir_entry *portStateProc=NULL;
static unsigned int linkUpTime[RTL8651_PHY_NUMBER] = {0,0,0,0,0};

static void init_linkup_time(void)
{
	init_timer(&s_timer);
	s_timer.function = &linkup_time_handle;
	s_timer.expires = jiffies + HZ;
	add_timer(&s_timer);
}

static void linkup_time_handle(unsigned long arg)
{
	int port;
	uint32	regData;
	uint32	data0;
	mod_timer(&s_timer,jiffies +HZ);
	for(port=PHY0;port<PHY5;port++)
	{
		regData = READ_MEM32(PSRP0+((port)<<2));
		data0 = regData & PortStatusLinkUp;
		if (data0)
		{
			linkUpTime[port]++;
		}
		else
		{
			linkUpTime[port]=0;
		}
	}
}

static int32 port_state_read_proc( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int		len;
	uint32	regData;
	uint32	data0;
	int		port;

	len = sprintf(page, "Port Link State:\n");

	for(port=PHY0;port<PHY5;port++)
	{
		regData = READ_MEM32(PSRP0+((port)<<2));

		//if (port==CPU)
			//len += sprintf(page+len, "CPUPort ");
		//else
		len += sprintf(page+len, "Port[%d]:", port);
		data0 = regData & PortStatusLinkUp;

		if (data0)
		{
			len += sprintf(page+len, "LinkUp|");
		}
		else
		{
			len += sprintf(page+len, "LinkDown\n");
			continue;
		}
		//data0 = regData & PortStatusNWayEnable;
		//len += sprintf(page+len, "NWay Mode %s\n", data0?"Enabled":"Disabled");
		data0 = regData & PortStatusRXPAUSE;
		len += sprintf(page+len, "RXPause:%s|", data0?"Enable":"Disable");
		data0 = regData & PortStatusTXPAUSE;
		len += sprintf(page+len, "TXPause:%s|", data0?"Enable":"Disable");
		data0 = regData & PortStatusDuplex;
		len += sprintf(page+len, "Duplex:%s|", data0?"Enable":"Disable");
		data0 = (regData&PortStatusLinkSpeed_MASK)>>PortStatusLinkSpeed_OFFSET;
		len += sprintf(page+len, "Speed:%s|", data0==PortStatusLinkSpeed100M?"100M":
			(data0==PortStatusLinkSpeed1000M?"1G":
				(data0==PortStatusLinkSpeed10M?"10M":"Unkown")));
		len += sprintf(page+len, "UpTime:%ds\n", linkUpTime[port]);
	}

	return len;
}

static int32 port_state_write_proc( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}

static int32 initPortStateCtrl(void)
{
	portStateProc = create_proc_entry("portState", 0, NULL);
	if(portStateProc)
	{
		portStateProc->read_proc = port_state_read_proc;
		portStateProc->write_proc = port_state_write_proc;
	}
	memset(linkUpTime,0,sizeof(linkUpTime));
	init_linkup_time();

	return 0;
}

static void  exitPortStateCtrl(void)
{
	if (portStateProc) {
		remove_proc_entry("portState", portStateProc);
		portStateProc = NULL;
	}
	del_timer(&s_timer);
}
#endif

#if defined(CONFIG_RTL_MULTIPLE_WAN)
static int rtl_port_used_by_device(uint32 portMask)
{
	int i;
	struct dev_priv *cp;
	int retval = FAILED;

	for(i = 0; i < ETH_INTF_NUM; i++)
	{
		cp = ((struct dev_priv *)_rtl86xx_dev.dev[i]->priv);
		if(cp && cp->opened && (cp->portmask & portMask))
		{
			retval = SUCCESS;
			break;
		}

	}

	cp = (struct dev_priv *)rtl_multiWan_net_dev->priv;
	if(cp && cp->opened && (cp->portmask & portMask))
		retval = SUCCESS;

	return retval;
}


static int rtl_regist_multipleWan_dev(void)
{
	int j;
	struct net_device *dev;
	struct dev_priv   *dp;
	int rc;

	dev = alloc_etherdev(sizeof(struct dev_priv));
	if (!dev) {
		printk("====%s(%d) failed to allocate\n",__FUNCTION__,__LINE__);
		return FAILED;
	}
	SET_MODULE_OWNER(dev);
	dp = dev->priv;
	memset(dp,0,sizeof(*dp));
	dp->dev = dev;
	dp->id = rtl_multiWan_config.vid;
	dp->portmask =	rtl_multiWan_config.memPort;
	dp->portnum  = 0;
	for(j=0;j<RTL8651_AGGREGATOR_NUMBER;j++){
		if(dp->portmask & (1<<j))
			dp->portnum++;
	}

	memcpy((char*)dev->dev_addr,(char*)(&(rtl_multiWan_config.mac)),ETHER_ADDR_LEN);
#if defined(CONFIG_COMPAT_NET_DEV_OPS)
	dev->open = re865x_open;
	dev->stop = re865x_close;
	dev->set_multicast_list = re865x_set_rx_mode;
	dev->hard_start_xmit = re865x_start_xmit;
	dev->get_stats = re865x_get_stats;
	dev->do_ioctl = re865x_ioctl;
	dev->tx_timeout = re865x_tx_timeout;
	dev->set_mac_address = rtl865x_set_hwaddr;
	dev->change_mtu = rtl865x_set_mtu;
#if defined(CONFIG_RTL8186_LINK_CHANGE)
	dev->change_link = rtl865x_set_link;
#endif
#ifdef CP_VLAN_TAG_USED
	dev->features |= NETIF_F_HW_VLAN_TX | NETIF_F_HW_VLAN_RX;
	dev->vlan_rx_register = cp_vlan_rx_register;
	dev->vlan_rx_kill_vid = cp_vlan_rx_kill_vid;
#endif

#else
	dev->netdev_ops = &rtl819x_netdev_ops;
#endif
	dev->watchdog_timeo = TX_TIMEOUT;

#ifdef CP_VLAN_TAG_USED
	dev->features |= NETIF_F_HW_VLAN_TX | NETIF_F_HW_VLAN_RX;
#endif

	dev->irq = 0;
	memcpy((char*)dev->name,rtl_multiWan_config.ifname,MAX_IFNAMESIZE);
	rc = register_netdev(dev);
	if(!rc){
		rtl_multiWan_net_dev = dev;
		rtl_add_ps_drv_netif_mapping(dev,rtl_multiWan_config.ifname);
		printk("==%s(%d) %s added. vid=%d Member port 0x%x...\n",__FUNCTION__,__LINE__,dev->name,rtl_multiWan_config.vid ,rtl_multiWan_config.memPort );
	}else
		printk("===%s(%d) Failed to allocate,rc(%d)\n",__FUNCTION__,__LINE__,rc);

	return SUCCESS;
	//
}
#if 0
static int rtl_unregist_multipleWan_dev(void)
{
	if(rtl_multiWan_net_dev)
		unregister_netdev(rtl_multiWan_net_dev);

	rtl_multiWan_net_dev = NULL;
	return SUCCESS;
}
#endif
static int rtl_config_multipleWan_netif(int32 cmd)
{
	int retval = FAILED;
	switch(cmd)
	{
		case RTL_MULTIWAN_ADD:
			{
				rtl865x_netif_t netif;

				/*add vlan*/
				retval = rtl865x_addVlan(rtl_multiWan_config.vid);
				if(retval == SUCCESS)
				{
					rtl865x_addVlanPortMember(rtl_multiWan_config.vid,rtl_multiWan_config.memPort);
					rtl865x_setVlanFilterDatabase(rtl_multiWan_config.vid,rtl_multiWan_config.fid);

				}
				else if (retval == RTL_EVLANALREADYEXISTS)
				{
					//only add memberport to vlan
					rtl865x_addVlanPortMember(rtl_multiWan_config.vid,rtl_multiWan_config.memPort);
				}

				/*add network interface*/
				memset(&netif, 0, sizeof(rtl865x_netif_t));
				memcpy(netif.name,rtl_multiWan_config.ifname,MAX_IFNAMESIZE);
				memcpy(netif.macAddr.octet, rtl_multiWan_config.mac.octet,ETHER_ADDR_LEN);
				netif.mtu = rtl_multiWan_config.mtu;
				netif.if_type = rtl_multiWan_config.if_type;
				netif.vid = rtl_multiWan_config.vid;
				netif.is_wan = rtl_multiWan_config.isWan;
				netif.is_slave = rtl_multiWan_config.is_slave;
				#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
				netif.enableRoute=1;
				#endif
				retval = rtl865x_addNetif(&netif);
				rtl865x_reConfigDefaultAcl(netif.name);

				if(retval != SUCCESS && retval != RTL_EVLANALREADYEXISTS)
				{
					printk("=======%s(%d),retval(%d)\n",__FUNCTION__,__LINE__,retval);
				}
			}
			rtl_init_advRt();
			break;
		case RTL_MULTIWAN_DEL:
			rtl_exit_advRt();
			rtl865x_delNetif(rtl_multiWan_config.ifname);
			rtl865x_delVlanPortMember(rtl_multiWan_config.vid,rtl_multiWan_config.memPort);
		break;
		default:
			retval = FAILED;
	}

	return retval;
}

static int rtl865x_addMultiCastNetif(void)
{
	rtl865x_netif_t netif;
	int ret=FAILED;

	rtl865x_delVlan(MULTICAST_NETIF_VLAN_ID);
	rtl865x_addVlan(MULTICAST_NETIF_VLAN_ID);
	ret=rtl865x_addVlanPortMember(MULTICAST_NETIF_VLAN_ID, rtl_multiWan_config.memPort);
	ret=rtl865x_setVlanFilterDatabase(MULTICAST_NETIF_VLAN_ID, rtl_multiWan_config.fid);

	if(_rtl865x_getNetifByName(multiCastNetIf)!=NULL)
	{
		return SUCCESS;
	}

	memset(&netif, 0, sizeof(rtl865x_netif_t));
	strcpy(netif.name, multiCastNetIf);
	memcpy(&netif.macAddr, multiCastNetIfMac, 6);
	netif.mtu = 1500;
	netif.if_type = IF_ETHER;
	netif.vid = MULTICAST_NETIF_VLAN_ID;
	netif.is_wan = 1;
	netif.is_slave = 0;
	netif.enableRoute=1;
	netif.forMacBasedMCast=TRUE;
	//printk("%s:%d,entry->lpNetif is %s \n",__FUNCTION__,__LINE__,entry->lpNetif);
	ret = rtl865x_addNetif(&netif);
	if(ret!=SUCCESS)
	{
		rtl865x_delVlan(MULTICAST_NETIF_VLAN_ID);
		return FAILED;
	}

#if defined(CONFIG_RTK_VLAN_SUPPORT)
	{
		rtl865x_AclRule_t	rule;
		bzero((void*)&rule,sizeof(rtl865x_AclRule_t));
		rule.ruleType_ = RTL865X_ACL_MAC;
		rule.pktOpApp_ = RTL865X_ACL_ALL_LAYER;
		rule.actionType_ = RTL865X_ACL_PERMIT;
		ret=rtl865x_add_acl(&rule, multiCastNetIf, RTL865X_ACL_SYSTEM_USED);
	}
#endif

	return ret;
}

#if 0
static int rtl865x_delMultiCastNetif(void)
{
	int ret=FAILED;

#if defined(CONFIG_RTK_VLAN_SUPPORT)
	{
		rtl865x_AclRule_t	rule;
		bzero((void*)&rule,sizeof(rtl865x_AclRule_t));
		rule.ruleType_ = RTL865X_ACL_MAC;
		rule.pktOpApp_ = RTL865X_ACL_ALL_LAYER;
		rule.actionType_ = RTL865X_ACL_PERMIT;
		ret=rtl865x_del_acl(&rule, multiCastNetIf, RTL865X_ACL_SYSTEM_USED);
	}
#endif

	ret=rtl865x_delNetif(multiCastNetIf);

	if(ret==SUCCESS)
	{
		ret=rtl865x_delVlan(MULTICAST_NETIF_VLAN_ID);
	}
	return ret;

}

int rtl865x_setMultiCastSrcMac(unsigned char *srcMac)
{

	if(srcMac==NULL)
	{
		return FAILED;
	}

	memcpy(multiCastNetIfMac, srcMac, 6);

	if(_rtl865x_getNetifByName(multiCastNetIf)!=NULL)
	{
		rtl865x_delMCastNetif();
		rtl865x_addMCastNetif();
	}

	return SUCCESS;
}
#endif

#endif


#if defined(CONFIG_RTL_REINIT_SWITCH_CORE)
int  re865x_reProbe (void)
{
	rtl8651_tblAsic_InitPara_t para;
	unsigned long flags;
#if defined(CONFIG_RTL_819XD)&&defined(CONFIG_RTL_8211DS_SUPPORT)&&defined(CONFIG_RTL_8197D)
	uint32 reg_tmp=0;
#endif

	local_irq_save(flags);
	//WRITE_MEM32(PIN_MUX_SEL_2, 0x7<<21);
	/*Initial ASIC table*/
	FullAndSemiReset();

	memset(&para, 0, sizeof(rtl8651_tblAsic_InitPara_t));

	/*
		For DEMO board layout, RTL865x platform define corresponding PHY setting and PHYID.
	*/

	INIT_CHECK(rtl865x_initAsicL2(&para));

#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER_L3
	INIT_CHECK(rtl865x_initAsicL3());
#endif
#if defined(CONFIG_RTL_LAYERED_ASIC_DRIVER_L4) && (defined(CONFIG_RTL_8198) ||defined(CONFIG_RTL_8196CT) ||defined(CONFIG_RTL_819XDT))
	INIT_CHECK(rtl865x_initAsicL4());
#endif


	/*init PHY LED style*/
#if !defined(CONFIG_RTL_819XD) && !defined(CONFIG_RTL_8196E)
#if defined(CONFIG_RTL865X_BICOLOR_LED)
	#ifdef BICOLOR_LED_VENDOR_BXXX
	REG32(LEDCR) |= (1 << 19); // 5 ledmode set to 1 for bi-color LED
	REG32(PABCNR) &= ~0x001f0000; /* set port port b-4/3/2/1/0 to gpio */
	REG32(PABDIR) |=  0x001f0000; /* set port port b-4/3/2/1/0 gpio direction-output */
	#else
	//8650B demo board default: Bi-color 5 LED
	WRITE_MEM32(LEDCR, READ_MEM32(LEDCR) | 0x01180000 ); // bi-color LED
	#endif
	/* config LED mode */
	WRITE_MEM32(SWTAA, PORT5_PHY_CONTROL);
	WRITE_MEM32(TCR0, 0x000002C2); //8651 demo board default: 15 LED boards
	WRITE_MEM32(SWTACR, CMD_FORCE | ACTION_START); // force add
#else /* CONFIG_RTL865X_BICOLOR_LED */

	/* config LED mode */
	WRITE_MEM32(LEDCR, 0x00000000 ); // 15 LED
	WRITE_MEM32(SWTAA, PORT5_PHY_CONTROL);
	WRITE_MEM32(TCR0, 0x000002C7); //8651 demo board default: 15 LED boards
	WRITE_MEM32(SWTACR, CMD_FORCE | ACTION_START); // force add
#endif /* CONFIG_RTL865X_BICOLOR_LED */
#endif

/*2007-12-19*/
#if defined(CONFIG_RTK_VLAN_SUPPORT)
		//port based decision
	rtl865xC_setNetDecisionPolicy(NETIF_PORT_BASED);
	WRITE_MEM32(PLITIMR,0);
#endif

#if defined(CONFIG_RTL_819XD)&&defined(CONFIG_RTL_8211DS_SUPPORT)&&defined(CONFIG_RTL_8197D)
	rtl8651_getAsicEthernetPHYReg(0x6, 0, &reg_tmp);
	rtl_setPortMask(reg_tmp);
#endif

	/*queue id & rx ring descriptor mapping*/
	REG32(CPUQDM0)=QUEUEID1_RXRING_MAPPING|(QUEUEID0_RXRING_MAPPING<<16);
	REG32(CPUQDM2)=QUEUEID3_RXRING_MAPPING|(QUEUEID2_RXRING_MAPPING<<16);
	REG32(CPUQDM4)=QUEUEID5_RXRING_MAPPING|(QUEUEID4_RXRING_MAPPING<<16);

	rtl8651_setAsicOutputQueueNumber(CPU, RTL_CPU_RX_RING_NUM);



#if defined(CONFIG_RTL_HW_STP)
	//Initial: disable Realtek Hardware STP
	rtl865x_setSpanningEnable(FALSE);
#endif


#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER_L3
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	rtl8651_setAsicMulticastEnable(TRUE);
#else
	rtl8651_setAsicMulticastEnable(FALSE);
#endif
#endif

#ifdef CONFIG_RTL_8198_ESD
	esd3_skip_one = 1;
	one_second_counter = 0;
	first_time_read_reg6 = 1;
	phy_reg30[0] = phy_reg30[1] = phy_reg30[2] = phy_reg30[3] = phy_reg30[4] = 0;
#endif		

#ifdef CONFIG_RTL_8198
	{
	extern void disable_phy_power_down(void);
	disable_phy_power_down();
	}
#endif

	local_irq_restore(flags);
	return 0;
}

int rtl865x_reinitSwitchCore(void)
{
	/*enable switch core clock*/
	rtl865x_duringReInitSwtichCore=1;
	/*disable switch core interrupt*/
	REG32(CPUICR) = 0;
	REG32(CPUIIMR) = 0;
	REG32(GIMR) &= ~(BSP_SW_IE);

	re865x_reProbe();
	swNic_reInit();
	rtl865x_reChangeOpMode();

	/*enable switch core interrupt*/

	REG32(CPUICR) = TXCMD | RXCMD | BUSBURST_32WORDS | MBUF_2048BYTES;
	REG32(CPUIIMR) = RX_DONE_IE_ALL | TX_ALL_DONE_IE_ALL | LINK_CHANGE_IE | PKTHDR_DESC_RUNOUT_IE_ALL;
	REG32(SIRR) |= TRXRDY;
	REG32(GIMR) |= (BSP_SW_IE);

	rtl865x_duringReInitSwtichCore=0;
	return 0;
}

static struct proc_dir_entry *reInitSwitchCoreProc=NULL;

static int rtl865x_reInitSwitchCoreReadProc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
	int len=0;

	if (len <= off+count)
		*eof = 1;

	*start = page + off;
	len -= off;

	if (len>count)
		len = count;

	if (len<0) len = 0;

	return len;
}

static int rtl865x_reInitSwitchCoreWriteProc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	char tmpBuf[256];
	char		*strptr;
	char		*tokptr;
	unsigned int action;

	if (buffer && !copy_from_user(tmpBuf, buffer, count))
	{
		tmpBuf[count-1]=0;
		strptr=tmpBuf;

		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errOut;
		}

		if(rtl865x_duringReInitSwtichCore)
		{
			goto errOut;
		}
		//printk("here to reset switch core\n");
		action=simple_strtol(tokptr, NULL, 0);
		if(action==1)
		{
			rtl865x_reinitSwitchCore();
		}

		return count;
	}

errOut:
	return -EFAULT;
}


int  rtl865x_creatReInitSwitchCoreProc(void)
{
	reInitSwitchCoreProc = create_proc_entry("reInitSwitchCore", 0, NULL);
	if(reInitSwitchCoreProc)
	{
		reInitSwitchCoreProc->read_proc = rtl865x_reInitSwitchCoreReadProc;
		reInitSwitchCoreProc->write_proc = rtl865x_reInitSwitchCoreWriteProc;
	}

	return 0;
}

void __exit rtl865x_destroyReInitResetSwitchCore(void)
{
	if (reInitSwitchCoreProc) {
		remove_proc_entry("reInitSwitchCore", reInitSwitchCoreProc);
		reInitSwitchCoreProc = NULL;
	}
}

#endif

#ifdef CONFIG_RTK_VOIP_QOS
int wan_port_check(int port)
{
	if(1<<port & RTL_WANPORT_MASK)
		return TRUE;
	else
		return FALSE;
} 
#endif

#ifdef CONFIG_RTK_VOIP_PORT_LINK
static int rtnl_fill_ifinfo_voip(struct sk_buff *skb, struct net_device *dev,
                            int type, u32 pid, u32 seq, u32 change,
                            unsigned int flags)
{
        struct ifinfomsg *ifm;
        struct nlmsghdr *nlh;

        nlh = nlmsg_put(skb, pid, seq, type, sizeof(*ifm), flags);
        if (nlh == NULL)
                return -ENOBUFS;

        ifm = nlmsg_data(nlh);
        ifm->ifi_family = AF_UNSPEC;
        ifm->__ifi_pad = 0;
        ifm->ifi_type = dev->type;
        ifm->ifi_index = dev->ifindex;
        ifm->ifi_flags = dev_get_flags(dev);
        ifm->ifi_change = change;
        return nlmsg_end(skb, nlh);
}
static void rtmsg_ifinfo_voip(int type, struct net_device *dev, unsigned change)
{
        struct net *net = dev_net(dev);
        struct sk_buff *skb;
        int err = -ENOBUFS;

        skb = nlmsg_new(NLMSG_GOODSIZE, GFP_ATOMIC);
        if (skb == NULL)
                goto errout;

        err = rtnl_fill_ifinfo_voip(skb, dev, type, 0, 0, change, 0);
        if (err < 0) {
                /* -EMSGSIZE implies BUG in if_nlmsg_size() */
                WARN_ON(err == -EMSGSIZE);
                kfree_skb(skb);
                goto errout;
        }
        rtnl_notify(skb, net, 0, RTNLGRP_LINK, NULL, GFP_ATOMIC);
        return;
errout:
        if (err < 0)
                rtnl_set_sk_err(net, RTNLGRP_LINK, err);
}
#endif


#if defined (CONFIG_RTL_SOCK_DEBUG)
static struct proc_dir_entry *rtl865x_sockDebugProc=NULL;
extern int dumpRawSockInfo(void);
extern int dumpUdpSockInfo(void);
extern int dumpTcpSockInfo(void);
static int rtl865x_sockDebugReadProc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
	int len=0;
	len = 0;

	if (len <= off+count)
		*eof = 1;

	*start = page + off;
	len -= off;

	if (len>count)
		len = count;

	if (len<0) len = 0;

	return len;
}

static int rtl865x_sockDebugWriteProc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	char tmpBuf[256];
	char		*strptr;
	char		*tokptr;
	unsigned int tmp;

	if (buffer && !copy_from_user(tmpBuf, buffer, count))
	{
		tmpBuf[count-1]=0;
		strptr=tmpBuf;

		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errOut;
		}

		//printk("here to reset switch core\n");
		tmp=simple_strtol(tokptr, NULL, 0);
		if(tmp==1)
		{
			dumpRawSockInfo();
			dumpUdpSockInfo();
			dumpTcpSockInfo();

		}


		return count;
	}

errOut:
	return -EFAULT;
}


int  rtl865x_creatSockDebugProc(void)
{
	rtl865x_sockDebugProc = create_proc_entry("sockDebug", 0, NULL);
	if(rtl865x_sockDebugProc)
	{
		rtl865x_sockDebugProc->read_proc = rtl865x_sockDebugReadProc;
		rtl865x_sockDebugProc->write_proc = rtl865x_sockDebugWriteProc;
	}

	return 0;
}

void __exit rtl865x_removeSockDebugProc(void)
{
	if (rtl865x_sockDebugProc) {
		remove_proc_entry("sockDebug", rtl865x_sockDebugProc);
		rtl865x_sockDebugProc = NULL;
	}
}
#endif

#ifdef CONFIG_RTL_ULINKER
void eth_led_recover(void)
{
	REG32(PIN_MUX_SEL_2) = REG32(PIN_MUX_SEL_2) & ~(0x00003000);
}
#endif

extern int eee_enabled;
#if defined(RTL8198_EEE_MAC)
extern void eee_phy_enable_98(void);
extern void eee_phy_disable_98(void);
#endif

static struct proc_dir_entry *res_eee=NULL;
static int eee_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data);
static int eee_write_proc(struct file *file, const char *buffer, unsigned long count, void *data);
extern void enable_EEE(void);
extern void disable_EEE(void);

static int32 rtl819x_eee_proc_init(void)
{
	res_eee = create_proc_entry("eee", 0, NULL);
	if(res_eee)
	{
		res_eee->read_proc = eee_read_proc;
		res_eee->write_proc = eee_write_proc;
	}
	return 0;
}

void __exit rtl819x_eee_proc_exit(void)
{
	if (res_eee) {
		remove_proc_entry("eee", res_eee);
		res_eee = NULL;
	}
}

static int eee_read_proc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
	int len=0;

	len += sprintf(page+len, "eee %sabled.\n", ((eee_enabled) ? "en" : "dis")  );

	if (len <= off+count)
		*eof = 1;

	*start = page + off;
	len -= off;

	if (len>count)
		len = count;

	if (len<0) len = 0;

	return len;
}

static int eee_write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	char tmpBuf[32];

	if (buffer && !copy_from_user(tmpBuf, buffer, count))
	{
		if (tmpBuf[0] == '0') {
			eee_enabled = FALSE;
			disable_EEE();
		}
		else if (tmpBuf[0] == '1') {
			eee_enabled = TRUE;
			enable_EEE();
		}
#ifdef CONFIG_RTL_8198_ESD
		esd3_skip_one = 1;
#endif		
		return count;
	}
	return -EFAULT;
}
#ifdef CONFIG_RTL865X_LANPORT_RESTRICTION
char *rtl_getDevNameByPort(int32 port_num)
{
	int i;
	struct dev_priv *cp;

	for(i = 0; i < ETH_INTF_NUM; i++)
	{
		cp = ((struct dev_priv *)_rtl86xx_dev.dev[i]->priv);
		if(cp && cp->opened && (cp->portmask & (1<<port_num)))
			return cp->dev->name;
	}

	return NULL;
}
#endif

