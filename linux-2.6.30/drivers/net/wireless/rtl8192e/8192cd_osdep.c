/*
 *  Routines to handle OS dependent jobs and interfaces
 *
 *  $Id: 8192cd_osdep.c,v 1.61.2.28 2011/01/11 13:48:37 button Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_OSDEP_C_

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/in.h>
#include <linux/if.h>
#include <linux/ip.h>
#include <asm/io.h>
#include <linux/skbuff.h>
#include <linux/socket.h>
#include <linux/fs.h>
#include <linux/major.h>
#include <linux/fcntl.h>
#include <linux/signal.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#endif

#ifdef __DRAYTEK_OS__
#include <draytek/softimer.h>
#include <draytek/skbuff.h>
#include <draytek/wl_dev.h>
#endif

#include "./8192cd_cfg.h"

#ifdef _BROADLIGHT_FASTPATH_
int (*send_packet_to_upper_layer)(struct sk_buff *skb) = netif_rx ;
#endif
#ifdef __KERNEL__
#ifdef __LINUX_2_6__
#include <linux/syscalls.h>
#include <linux/file.h>
#include <asm/unistd.h>
#endif
#if defined(RTK_BR_EXT) || defined(BR_SHORTCUT)
#ifdef __LINUX_2_6__
#include <linux/syscalls.h>
#else
#include <linux/fs.h>
#endif
#endif
#include <../net/bridge/br_private.h>
#elif defined(__ECOS)
#include <cyg/hal/plf_intr.h>
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
#include <cyg/io/eth/rltk/819x/wrapper/wrapper.h>
#else
#include "./sys-support.h"
#endif

#include "./8192cd.h"
#include "./8192cd_hw.h"
#include "./8192cd_headers.h"
#include "./8192cd_rx.h"
#include "./8192cd_debug.h"

#ifdef NOT_RTK_BSP
#include <../net/bridge/br_private.h>
#endif
#ifdef RTL8192CD_VARIABLE_USED_DMEM
#include "./8192cd_dmem.h"
#endif

#ifdef CONFIG_RTL_92D_DMDP
u32 if_priv[NUM_WLAN_IFACE];
#ifdef NOT_RTK_BSP
static int rtl8192D_idx=0;
#endif
#endif

#ifdef	CONFIG_RTK_MESH
#include "../mesh_ext/mesh_route.h"	// Note : Not include in  #ifdef CONFIG_RTK_MESH, Because use in wlan_device[]
#include "../mesh_ext/mesh_util.h"
#endif	// CONFIG_RTK_MESH

#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
#include <asm/mips16_lib.h>
#endif

#ifndef CONFIG_RTK_MESH	//move below from mesh_route.h ; plus 0119
#define MESH_SHIFT			0
#define MESH_NUM_CFG		0
#else
#define MESH_SHIFT			8 // ACCESS_MASK uses 2 bits, WDS_MASK use 4 bits
#define MESH_MASK			0x3
#define MESH_NUM_CFG		NUM_MESH
#endif


#if defined(CONFIG_RTL_8198)
#define CONFIG_RTL8198_REVISION_B 1
//#define CONFIG_PHY_EAT_40MHZ 1
#endif
#ifdef CONFIG_RTL8672
	#ifdef USE_RLX_BSP
		#include <bspchip.h>
		#include <gpio.h>

		#ifdef CONFIG_RTL_8196C
		#undef CONFIG_RTL_8196C
		#endif
		#ifdef CONFIG_RTL8196C_REVISION_B
		#undef CONFIG_RTL8196C_REVISION_B
		#endif
	#else
		#include <platform.h>
		#include "../../../arch/mips/realtek/rtl8672/gpio.h"
	#endif
#else
	#if !defined(CONFIG_NET_PCI) && defined(CONFIG_RTL8196B)
	#include <asm/rtl865x/platform.h>
	#endif

	#if !defined(CONFIG_NET_PCI) && defined(CONFIG_RTL8196C)
	#ifdef __KERNEL__
	#include <asm/rtl865x/platform.h>
	#endif
	#endif
#endif

#if defined(CONFIG_RTL_819X) && defined(__LINUX_2_6__)
#if !defined(USE_RLX_BSP)
#ifndef __ECOS
#include <platform.h>
#endif
#else
#include <bsp/bspchip.h>
#endif
#endif

#if defined(CONFIG_RTL_WAPI_SUPPORT)
#include "wapiCrypto.h"
#endif
#ifdef CONFIG_RTL_CUSTOM_PASSTHRU //mark_test
#include <linux/proc_fs.h> 
#endif
#if defined(CONFIG_WIRELESS_LAN_MODULE) && !defined(NOT_RTK_BSP)
extern int (*wirelessnet_hook)(void);
#ifdef BR_SHORTCUT
extern struct net_device* (*wirelessnet_hook_shortcut)(unsigned char *da);
#endif
#ifdef PERF_DUMP
extern int rtl8651_romeperfEnterPoint(unsigned int index);
extern int rtl8651_romeperfExitPoint(unsigned int index);
extern int (*Fn_rtl8651_romeperfEnterPoint)(unsigned int index);
extern int (*Fn_rtl8651_romeperfExitPoint)(unsigned int index);
#endif
#ifdef CONFIG_RTL8190_PRIV_SKB
extern int (*wirelessnet_hook_is_priv_buf)(void);
extern void (*wirelessnet_hook_free_priv_buf)(unsigned char *head);
#endif
#endif // CONFIG_WIRELESS_LAN_MODULE


// TODO: Filen, move to BSP Setting
#ifdef CONFIG_WLAN_HAL_8881A //CONFIG_SOC_RTL8881A
/*
 * Wlan LBus Address
 */

#define BSP_WLAN_BASE_ADDR      0xB8640000
#define BSP_WLAN_CONF_ADDR      NULL
#endif  //CONFIG_SOC_RTL8881A

#ifdef __KERNEL__

struct _device_info_ wlan_device[] =
{
#ifdef  CONFIG_WLAN_HAL
    #ifdef CONFIG_WLAN_HAL_8881A
    {
        (MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_EMBEDDED<<TYPE_SHIFT) | ACCESS_SWAP_MEM, 
        BSP_WLAN_CONF_ADDR, 
        BSP_WLAN_BASE_ADDR, 
        BSP_WLAN_MAC_IRQ, 
        NULL
    },

#if defined(CONFIG_USE_PCIE_SLOT_0) 
	{
		(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, 
		BSP_PCIE0_D_CFG0, 
		BSP_PCIE0_D_MEM, 
		BSP_PCIE_IRQ, 
		NULL
	},
#endif

#endif  //CONFIG_WLAN_HAL_8881A


    #ifdef CONFIG_WLAN_HAL_8192EE

#if !defined(NOT_RTK_BSP)
    {
        (MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, 
        BSP_PCIE0_D_CFG0, 
        BSP_PCIE0_D_MEM, 
        BSP_PCIE_IRQ, 
        NULL
    },
#else
	{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_BIOS<<TYPE_SHIFT) | ACCESS_SWAP_MEM, 0, 0, 0, NULL},
	{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_BIOS<<TYPE_SHIFT) | ACCESS_SWAP_MEM, 0, 0, 0, NULL},
	{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_BIOS<<TYPE_SHIFT) | ACCESS_SWAP_MEM, 0, 0, 0, NULL}
#endif //#if !defined(NOT_RTK_BSP)

    #endif  //CONFIG_WLAN_HAL_8192EE
#else

#if defined(USE_RTL8186_SDK)
	//{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_EMBEDDED<<TYPE_SHIFT), 0, 0xbd400000, 2, NULL},
	#ifdef CONFIG_NET_PCI
		{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_BIOS<<TYPE_SHIFT) | ACCESS_SWAP_MEM, 0, 0, 0, NULL},
	#else
		#ifdef IO_MAPPING
			{(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, PCI_SLOT1_CONFIG_ADDR, 0xb8C00000, 3, NULL},
		#else
			#if defined(CONFIG_RTL8196B) || defined(CONFIG_RTL8196C) || defined(CONFIG_RTL_819X) || defined(CONFIG_RTL_8196E)
			  #if defined(__LINUX_2_6__) && defined(USE_RLX_BSP) 
	#if defined(CONFIG_RTL_92D_SUPPORT)
		 #if (RTL_USED_PCIE_SLOT==0)
				{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE0_D_CFG0, BSP_PCIE0_D_MEM, BSP_PCIE_IRQ, NULL},
          #ifdef CONFIG_RTL_92D_DMDP
				{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE0_D_CFG0, BSP_PCIE0_F1_D_MEM, BSP_PCIE_IRQ, NULL},
          #endif
          #if defined(CONFIG_USE_PCIE_SLOT_0) && defined(CONFIG_USE_PCIE_SLOT_1)
          		{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE1_D_CFG0, BSP_PCIE1_D_MEM, BSP_PCIE2_IRQ, NULL},
          #endif
         #elif (RTL_USED_PCIE_SLOT==1)
				{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE1_D_CFG0, BSP_PCIE1_D_MEM, BSP_PCIE2_IRQ, NULL},
				#ifdef CONFIG_RTL_92D_DMDP
				{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE1_D_CFG0, BSP_PCIE1_F1_D_MEM, BSP_PCIE2_IRQ, NULL},
				#endif
				#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN_D) || defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
				{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE0_D_CFG0, BSP_PCIE0_D_MEM, BSP_PCIE_IRQ, NULL},
		 		#endif
		 #else
				#error "define pcie error"
         #endif
	#else
#if defined(CONFIG_RTL_88E_SUPPORT)||defined(CONFIG_RTL_8812_SUPPORT)
#if defined(CONFIG_USE_PCIE_SLOT_0) && defined(CONFIG_USE_PCIE_SLOT_1)
#if defined(CONFIG_SLOT_0_8812) || defined(CONFIG_SLOT_0_92D)
		{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE0_D_CFG0, BSP_PCIE0_D_MEM, BSP_PCIE_IRQ, NULL},
		{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE1_D_CFG0, BSP_PCIE1_D_MEM, BSP_PCIE2_IRQ, NULL},
#else
		{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE1_D_CFG0, BSP_PCIE1_D_MEM, BSP_PCIE2_IRQ, NULL},		
		{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE0_D_CFG0, BSP_PCIE0_D_MEM, BSP_PCIE_IRQ, NULL},
#endif
#elif (RTL_USED_PCIE_SLOT==1)
		{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE1_D_CFG0, BSP_PCIE1_D_MEM, BSP_PCIE2_IRQ, NULL},		
#else
				{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE0_D_CFG0, BSP_PCIE0_D_MEM, BSP_PCIE_IRQ, NULL},
#endif
#else
		#if (!defined(CONFIG_USE_PCIE_SLOT_0) && defined(CONFIG_USE_PCIE_SLOT_1))
				{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE1_D_CFG0, BSP_PCIE1_D_MEM, BSP_PCIE2_IRQ, NULL},		
		#else		
				{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE0_D_CFG0, BSP_PCIE0_D_MEM, BSP_PCIE_IRQ, NULL},
		#endif		
		 #if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
				{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE1_D_CFG0, BSP_PCIE1_D_MEM, BSP_PCIE2_IRQ, NULL}
		 #endif
#endif
	#endif
#else
				{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, PCIE0_D_CFG0, PCIE0_D_MEM, PCIE_IRQ, NULL},
	    #ifdef CONFIG_RTL_92D_DMDP
				{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, PCIE0_D_CFG0, PCIE0_F1_D_MEM, PCIE_IRQ, NULL},
		#endif
#endif
				//{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_BIOS<<TYPE_SHIFT) | ACCESS_SWAP_MEM, 0, 0, 0, NULL},
			#else
				{(MESH_NUM_CFG<<MESH_SHIFT) | (WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, PCI_SLOT1_CONFIG_ADDR, 0xb9000000, 3, NULL},
			#endif
		#endif
	#endif
#elif defined(NOT_RTK_BSP)
	{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_BIOS<<TYPE_SHIFT) | ACCESS_SWAP_MEM, 0, 0, 0, NULL},
	{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_BIOS<<TYPE_SHIFT) | ACCESS_SWAP_MEM, 0, 0, 0, NULL},
	{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_BIOS<<TYPE_SHIFT) | ACCESS_SWAP_MEM, 0, 0, 0, NULL}
#elif defined(CONFIG_RTL8671)
	#ifdef IO_MAPPING
	{(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, PCI_SLOT0_CONFIG_ADDR, 0xbd200000, 7, NULL},
	#else
	{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, PCI_SLOT0_CONFIG_ADDR, 0xbd300000, 7, NULL},
	#endif
#else
	#error No_System_Defined
#endif
#endif  //CONFIG_WLAN_HAL
};

static int wlan_index=0;

#elif defined(__ECOS)
struct _device_info_ wlan_device[] = {
	{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, PCIE0_D_CFG0, PCIE0_D_MEM, CYGNUM_HAL_INTERRUPT_PCIE, NULL}
};
#endif /* __KERNEL__ */

#ifdef CONFIG_RTL8672
// Added by Mason Yu
// MBSSID Port Mapping
#ifdef CONFIG_RTL_92D_DMDP
struct port_map wlanDev[(RTL8192CD_NUM_VWLAN+2)*2];
#else
struct port_map wlanDev[RTL8192CD_NUM_VWLAN+2];		// Root(1)+vxd(1)+VAPs(4)
#endif
static int wlanDevNum=0;
#endif

#ifdef PRIV_STA_BUF
#ifdef CONCURRENT_MODE
static struct rtl8192cd_hw hw_info[NUM_WLAN_IFACE];
static struct priv_shared_info shared_info[NUM_WLAN_IFACE];
static struct wlan_hdr_poll hdr_pool[NUM_WLAN_IFACE];
static struct wlanllc_hdr_poll llc_pool[NUM_WLAN_IFACE];
static struct wlanbuf_poll buf_pool[NUM_WLAN_IFACE];
static struct wlanicv_poll icv_pool[NUM_WLAN_IFACE];
static struct wlanmic_poll mic_pool[NUM_WLAN_IFACE];
static unsigned char desc_buf[NUM_WLAN_IFACE][DESC_DMA_PAGE_SIZE];
static int wlandev_idx = 0;

#else
static struct rtl8192cd_hw hw_info;
static struct priv_shared_info shared_info;
static struct wlan_hdr_poll hdr_pool;
static struct wlanllc_hdr_poll llc_pool;
static struct wlanbuf_poll buf_pool;
static struct wlanicv_poll icv_pool;
static struct wlanmic_poll mic_pool;
static unsigned char desc_buf[DESC_DMA_PAGE_SIZE];
#endif
#endif

// init and remove char device
#ifdef CONFIG_WIRELESS_LAN_MODULE
extern int rtl8192cd_chr_init(void);
extern void rtl8192cd_chr_exit(void);
#else
int rtl8192cd_chr_init(void);
void rtl8192cd_chr_exit(void);
#endif
struct rtl8192cd_priv *rtl8192cd_chr_reg(unsigned int minor, struct rtl8192cd_chr_priv *priv);
void rtl8192cd_chr_unreg(unsigned int minor);
int rtl8192cd_fileopen(const char *filename, int flags, int mode);

__MIPS16
__IRAM_IN_865X
struct net_device *get_shortcut_dev(unsigned char *da);

void force_stop_wlan_hw(void);


#if defined(_INCLUDE_PROC_FS_) && defined(PERF_DUMP)
#include "romeperf.h"
static int read_perf_dump(struct file *file, const char *buffer,
              unsigned long count, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	unsigned long x;

	SAVE_INT_AND_CLI(x);

	rtl8651_romeperfDump(1, 1);

	RESTORE_INT(x);
    return count;
}


static int flush_perf_dump(struct file *file, const char *buffer,
              unsigned long count, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	unsigned long x;

	SAVE_INT_AND_CLI(x);

	rtl8651_romeperfReset();

	RESTORE_INT(x);
    return count;
}
#endif // _INCLUDE_PROC_FS_ && PERF_DUMP


static void rtl8192cd_bcnProc(struct rtl8192cd_priv *priv, unsigned int bcnInt,
				unsigned int bcnOk, unsigned int bcnErr, unsigned int status
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT)
				, unsigned int status_ext
#endif
				)
{
#ifdef MBSSID
	int i;
#endif
#ifdef UNIVERSAL_REPEATER
		struct rtl8192cd_priv *priv_root=NULL;
#endif

	/* ================================================================
			Process Beacon OK/ERROR interrupt
		================================================================ */
	if ( bcnOk || bcnErr)
	{

#ifdef UNIVERSAL_REPEATER
		struct rtl8192cd_priv *priv_root=NULL;
		if ((OPMODE & WIFI_STATION_STATE) && GET_VXD_PRIV(priv) &&
						(GET_VXD_PRIV(priv)->drv_state & DRV_STATE_VXD_AP_STARTED)) {
			priv_root = priv;
			priv = GET_VXD_PRIV(priv);
		}
#endif

		//
		// Statistics and LED counting
		//
		if (bcnOk) {
			// for SW LED
			if (priv->pshare->LED_cnt_mgn_pkt)
				priv->pshare->LED_tx_cnt++;
#ifdef MBSSID
			if (priv->pshare->bcnDOk_priv)
				priv->pshare->bcnDOk_priv->ext_stats.beacon_ok++;
#else
			priv->ext_stats.beacon_ok++;
#endif
			SNMP_MIB_INC(dot11TransmittedFragmentCount, 1);

			// disable high queue limitation
			if ((OPMODE & WIFI_AP_STATE) && (priv->pshare->bcnDOk_priv)) {
				if (*((unsigned char *)priv->pshare->bcnDOk_priv->beaconbuf + priv->pshare->bcnDOk_priv->timoffset + 4) & 0x01)  {
					RTL_W16(RD_CTRL, RTL_R16(RD_CTRL) | HIQ_NO_LMT_EN);
				}
			}

		} else if (bcnErr) {
#ifdef MBSSID
			if (priv->pshare->bcnDOk_priv)
				priv->pshare->bcnDOk_priv->ext_stats.beacon_er++;
#else
			priv->ext_stats.beacon_er++;
#endif
		}

#ifdef UNIVERSAL_REPEATER
		if (priv_root != NULL)
			priv = priv_root;
#endif
	}


#ifdef PCIE_POWER_SAVING
		if ((OPMODE & WIFI_AP_STATE) && (status & HIMR_BCNDOK0)) {
			if ((priv->offload_ctrl & 1) && (priv->offload_ctrl >> 7) && priv->pshare->rf_ft_var.power_save) {
				priv->offload_ctrl &= (~1);
					update_beacon(priv);
				delay_us(100);
				RTL_W16(CR , RTL_R16(CR) & ~ENSWBCN);
				return;
			}
		}
#endif	
	

	/* ================================================================
			Process Beacon interrupt
	    ================================================================ */
	//
	// Update beacon content
	//
	if (bcnInt) {
		unsigned char val8;
		if (
#ifdef CONFIG_RTL_88E_SUPPORT
			(GET_CHIP_VER(priv)==VERSION_8188E)?(status & HIMR_88E_BcnInt):
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
			(GET_CHIP_VER(priv)==VERSION_8812E)?(status & HIMR_92E_BcnInt):
#endif
			(status & HIMR_BCNDMA0)) {
#ifdef UNIVERSAL_REPEATER
		if ((OPMODE & WIFI_STATION_STATE) && GET_VXD_PRIV(priv) &&
			(GET_VXD_PRIV(priv)->drv_state & DRV_STATE_VXD_AP_STARTED)) {
			if (GET_VXD_PRIV(priv)->timoffset) {
				update_beacon(GET_VXD_PRIV(priv));
			}
			} else
#endif
			{
#if 0 //TESTCHIP_SUPPORT
#ifdef SMART_CONCURRENT_92D // -- fwdebug
				if (priv->pshare->wlandev_idx == 1) {
					DMDP_RTL_W8(0, 0x663, DMDP_RTL_R8(0, 0x663)|BIT(2));  // write MAC'0 0x663 to notify DRVERLY_INT 
					DMDP_RTL_W8(0, 0x1cf, DMDP_RTL_R8(0, 0x1cf)|BIT(4));  // write MAC'0 0x1cf to notify firmware INT
					if (priv->smcc_state==0) {
						unsigned int now, timeout;
						now = RTL_R32(TSFTR);
						timeout = now + (priv->pmib->dot11RFEntry.smcc_t * 1000);
						//printk("wlan%d start timer now=%d timeout=%d timeout1=%d\n", priv->pshare->wlandev_idx, now, timeout, timeout1);
						setup_timer2(priv, timeout);
#ifdef PHASE2_TEST
						timeout = now + (priv->pmib->dot11RFEntry.smcc_p * 1000);
						setup_timer1(priv, timeout);
#endif
					}
				}
#endif
#endif
#if 0//def SMART_CONCURRENT_92D
				priv->pshare->bcnCount++;
				if ((priv->pshare->bcnCount % 3)==0)
					smcc_signin_linkstate(priv, 1, 5, priv->smcc_state);
				else if ((priv->pshare->bcnCount % 3)==1)
					smcc_signin_linkstate(priv, 1, priv->pmib->dot11RFEntry.smcc_t, priv->smcc_state);
#endif

				if (priv->timoffset) {
					update_beacon(priv);
				}
			}
		}
#ifdef MBSSID
		else {
			if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
				for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
					if ((priv->pvap_priv[i]->vap_init_seq > 0) && IS_DRV_OPEN(priv->pvap_priv[i])
						&& (
#ifdef CONFIG_RTL_88E_SUPPORT
						(GET_CHIP_VER(priv)==VERSION_8188E)?(status_ext & (HIMRE_88E_BCNDMAINT1 <<
						(priv->pvap_priv[i]->vap_init_seq-1))):
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
						(GET_CHIP_VER(priv)==VERSION_8812E)?(status_ext & (HIMRE_92E_BCNDMAINT1 <<
						(priv->pvap_priv[i]->vap_init_seq-1))):
#endif
						(status & (HIMR_BCNDMA1 << (priv->pvap_priv[i]->vap_init_seq-1))))) {
						if (priv->pvap_priv[i]->timoffset) {
							update_beacon(priv->pvap_priv[i]);
						}
					}
				}
			}
		}
#endif

		//
		// Polling highQ as there is multicast waiting for tx...
		//
#ifdef UNIVERSAL_REPEATER
		if ((OPMODE & WIFI_STATION_STATE) && GET_VXD_PRIV(priv) &&
			(GET_VXD_PRIV(priv)->drv_state & DRV_STATE_VXD_AP_STARTED)) {
			priv_root = priv;
			priv = GET_VXD_PRIV(priv);
		}
#endif

		if ((OPMODE & WIFI_AP_STATE)) {
			if (
#ifdef CONFIG_RTL_88E_SUPPORT
				(GET_CHIP_VER(priv)==VERSION_8188E)?(status & HIMR_88E_BcnInt):
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
				(GET_CHIP_VER(priv)==VERSION_8812E)?(status & HIMR_92E_BcnInt):
#endif

				(status & HIMR_BCNDMA0)) {
				val8 = *((unsigned char *)priv->beaconbuf + priv->timoffset + 4);
				if (val8 & 0x01) {
					if(RTL_R8(BCN_CTRL) & DIS_ATIM)
						RTL_W8(BCN_CTRL, (RTL_R8(BCN_CTRL) & (~DIS_ATIM)));
#ifdef __ECOS
					priv->pshare->has_triggered_process_mcast_dzqueue = 1;
#else
					process_mcast_dzqueue(priv);
					priv->pkt_in_dtimQ = 0;
#endif
				} else {
					if(!(RTL_R8(BCN_CTRL) & DIS_ATIM))
						RTL_W8(BCN_CTRL, (RTL_R8(BCN_CTRL) | DIS_ATIM));				
				}	
//#ifdef MBSSID
				priv->pshare->bcnDOk_priv = priv;
//#endif
			}
#ifdef MBSSID
			else if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
				for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
					if ((priv->pvap_priv[i]->vap_init_seq > 0) && IS_DRV_OPEN(priv->pvap_priv[i])
						&& (
#ifdef CONFIG_RTL_88E_SUPPORT
						(GET_CHIP_VER(priv)==VERSION_8188E)?(status_ext & (HIMRE_88E_BCNDMAINT1 <<
						(priv->pvap_priv[i]->vap_init_seq-1))):
#endif
						(status & (HIMR_BCNDMA1 << (priv->pvap_priv[i]->vap_init_seq-1))))) {
						val8 = *((unsigned char *)priv->pvap_priv[i]->beaconbuf + priv->pvap_priv[i]->timoffset + 4);
						if (val8 & 0x01) {
							if(RTL_R8(BCN_CTRL) & DIS_ATIM)
								RTL_W8(BCN_CTRL, (RTL_R8(BCN_CTRL) & (~DIS_ATIM)));
							process_mcast_dzqueue(priv->pvap_priv[i]);
							priv->pvap_priv[i]->pkt_in_dtimQ = 0;
						} else {
							if(!(RTL_R8(BCN_CTRL) & DIS_ATIM))
								RTL_W8(BCN_CTRL, (RTL_R8(BCN_CTRL) | DIS_ATIM));
						}

						priv->pshare->bcnDOk_priv = priv->pvap_priv[i];
					}
				}
			}
#endif

		}

//		if (priv->pshare->pkt_in_hiQ) {
		if (priv->pshare->bcnDOk_priv && priv->pshare->bcnDOk_priv->pkt_in_hiQ) {
			int pre_head = get_txhead(priv->pshare->phw, MCAST_QNUM);
			do {
				txdesc_rollback(&pre_head);
			} while ((get_txdesc_info(priv->pshare->pdesc_info, MCAST_QNUM) + pre_head)->type != _PRE_ALLOCLLCHDR_);
			if (get_desc((get_txdesc(priv->pshare->phw, MCAST_QNUM) + pre_head)->Dword0) & TX_OWN) {
				unsigned short *phdr = (unsigned short *)((get_txdesc_info(priv->pshare->pdesc_info, MCAST_QNUM) + pre_head)->pframe);
#ifdef __MIPSEB__
				phdr = (unsigned short *)KSEG1ADDR(phdr);
#endif
				ClearMData(phdr);
			}
			tx_poll(priv, MCAST_QNUM);
//				priv->pshare->pkt_in_hiQ = 0;
		}


#ifdef UNIVERSAL_REPEATER
		if (priv_root != NULL)
			priv = priv_root;
#endif

	}

	
#ifdef CLIENT_MODE
	//
	// Ad-hoc beacon status
	//
	if (OPMODE & WIFI_ADHOC_STATE) {
		if (bcnOk)
			priv->ibss_tx_beacon = TRUE;
		if (bcnErr)
			priv->ibss_tx_beacon = FALSE;
	}
#endif
}

#ifdef CONFIG_WLAN_HAL

#if CFG_HAL_MEASURE_BEACON
static VOID
CalcBeaconVariation(
    struct rtl8192cd_priv *priv
)
{
    u4Byte		tsfVal,tsf,beaconVarationTime,i;
    static u4Byte      maxVal[8]= {0};
            
	tsf = RTL_R32(REG_TSFTR);


    tsf = tsf - RTL_R8(0x556)*1024*(priv->vap_init_seq);

    if(priv->vap_init_seq ==2)
    {
     
        beaconVarationTime = (tsf%102400);
            
        if(beaconVarationTime > maxVal[priv->vap_init_seq])
        {
             maxVal[priv->vap_init_seq] = beaconVarationTime;

            if(priv->vap_init_seq == 0 )
            {
                printk("Root maxVal = %d \n", maxVal[0]);
            }
            else
            {
                printk("VAP[%d] maxVal = %d \n",priv->vap_init_seq,maxVal[priv->vap_init_seq]);
            }          
        }

        //RTL_W32(0x1b8,beaconVarationTime);

        printk("VAP[%d] beaconVarationTime = %d TSF =%d\n",priv->vap_init_seq,beaconVarationTime,tsf);
    }
}

#endif  // #if CFG_HAL_MEASURE_BEACON
#endif //#ifdef CONFIG_WLAN_HAL

#ifdef CONFIG_WLAN_HAL
static void 
rtl88XX_bcnProc(
        struct rtl8192cd_priv *priv, 
        unsigned int bcnInt,
        unsigned int bcnOk, 
        unsigned int bcnErr
)
{
#ifdef MBSSID
	int i;
#endif


	/* ================================================================
			Process Beacon OK/ERROR interrupt
		================================================================ */
	if ( bcnOk || bcnErr)
	{

#ifdef UNIVERSAL_REPEATER
		struct rtl8192cd_priv *priv_root=NULL;
		if ((OPMODE & WIFI_STATION_STATE) && GET_VXD_PRIV(priv) &&
						(GET_VXD_PRIV(priv)->drv_state & DRV_STATE_VXD_AP_STARTED)) {
			priv_root = priv;
			priv = GET_VXD_PRIV(priv);
		}
#endif

		//
		// Statistics and LED counting
		//
		if (bcnOk) {
			// for SW LED
			if (priv->pshare->LED_cnt_mgn_pkt)
				priv->pshare->LED_tx_cnt++;
#ifdef MBSSID
			if (priv->pshare->bcnDOk_priv)
			{              
				priv->pshare->bcnDOk_priv->ext_stats.beacon_ok++;
#if CFG_HAL_MEASURE_BEACON            
                CalcBeaconVariation(priv->pshare->bcnDOk_priv);
#endif //#if CFG_HAL_MEASURE_BEACON
			}            
#else
			priv->ext_stats.beacon_ok++;
#if CFG_HAL_MEASURE_BEACON
            CalcBeaconVariation(priv);
#endif //#if CFG_HAL_MEASURE_BEACON

#endif
			SNMP_MIB_INC(dot11TransmittedFragmentCount, 1);

			// disable high queue limitation
			if ((OPMODE & WIFI_AP_STATE) && (priv->pshare->bcnDOk_priv)) {
				if (*((unsigned char *)priv->pshare->bcnDOk_priv->beaconbuf + priv->pshare->bcnDOk_priv->timoffset + 4) & 0x01)  {
					RTL_W16(RD_CTRL, RTL_R16(RD_CTRL) | HIQ_NO_LMT_EN);
				}
			}

		} else if (bcnErr) {
#ifdef MBSSID
			if (priv->pshare->bcnDOk_priv)
				priv->pshare->bcnDOk_priv->ext_stats.beacon_er++;
#else
			priv->ext_stats.beacon_er++;
#endif
		}

#ifdef UNIVERSAL_REPEATER
		if (priv_root != NULL)
			priv = priv_root;
#endif
	}


#ifdef PCIE_POWER_SAVING
// TODO: we should modify code below
		if ((OPMODE & WIFI_AP_STATE) && (status & HIMR_BCNDOK0)) {
			if ((priv->offload_ctrl & 1) && (priv->offload_ctrl >> 7) && priv->pshare->rf_ft_var.power_save) {
				priv->offload_ctrl &= (~1);
					update_beacon(priv);
				delay_us(100);
				RTL_W16(CR , RTL_R16(CR) & ~ENSWBCN);
				return;
			}
		}
#endif	
	

	/* ================================================================
			Process Beacon interrupt
	    ================================================================ */
	//
	// Update beacon content
	//
	if (bcnInt) {

		unsigned char val8;
		if ( _TRUE == GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_BcnInt) ) {
#ifdef UNIVERSAL_REPEATER
		if ((OPMODE & WIFI_STATION_STATE) && GET_VXD_PRIV(priv) &&
			(GET_VXD_PRIV(priv)->drv_state & DRV_STATE_VXD_AP_STARTED)) {
			if (GET_VXD_PRIV(priv)->timoffset) {
				update_beacon(GET_VXD_PRIV(priv));
			}
		} else
#endif
		{
#if 0 //TESTCHIP_SUPPORT
#ifdef SMART_CONCURRENT_92D // -- fwdebug
				if (priv->pshare->wlandev_idx == 1) {
					DMDP_RTL_W8(0, 0x663, DMDP_RTL_R8(0, 0x663)|BIT(2));  // write MAC'0 0x663 to notify DRVERLY_INT 
					DMDP_RTL_W8(0, 0x1cf, DMDP_RTL_R8(0, 0x1cf)|BIT(4));  // write MAC'0 0x1cf to notify firmware INT
					if (priv->smcc_state==0) {
						unsigned int now, timeout;
						now = RTL_R32(TSFTR);
						timeout = now + (priv->pmib->dot11RFEntry.smcc_t * 1000);
						//printk("wlan%d start timer now=%d timeout=%d timeout1=%d\n", priv->pshare->wlandev_idx, now, timeout, timeout1);
						setup_timer2(priv, timeout);
#ifdef PHASE2_TEST
						timeout = now + (priv->pmib->dot11RFEntry.smcc_p * 1000);
						setup_timer1(priv, timeout);
#endif
					}
				}
#endif
#endif
#if 0//def SMART_CONCURRENT_92D
				priv->pshare->bcnCount++;
				if ((priv->pshare->bcnCount % 3)==0)
					smcc_signin_linkstate(priv, 1, 5, priv->smcc_state);
				else if ((priv->pshare->bcnCount % 3)==1)
					smcc_signin_linkstate(priv, 1, priv->pmib->dot11RFEntry.smcc_t, priv->smcc_state);
#endif

				if (priv->timoffset) {
					update_beacon(priv);
				}
			}
		}
#ifdef MBSSID
		else {
			if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
				for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
					if ((priv->pvap_priv[i]->vap_init_seq > 0) && IS_DRV_OPEN(priv->pvap_priv[i])
						&& (_TRUE == GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_BcnInt1 + (priv->pvap_priv[i]->vap_init_seq-1)))) 
						{
						if (priv->pvap_priv[i]->timoffset) {
							update_beacon(priv->pvap_priv[i]);
						}
					}
				}
			}
		}
#endif

		//
		// Polling highQ as there is multicast waiting for tx...
		//
#ifdef UNIVERSAL_REPEATER
		struct rtl8192cd_priv *priv_root=NULL;
		if ((OPMODE & WIFI_STATION_STATE) && GET_VXD_PRIV(priv) &&
			(GET_VXD_PRIV(priv)->drv_state & DRV_STATE_VXD_AP_STARTED)) {
			priv_root = priv;
			priv = GET_VXD_PRIV(priv);
		}
#endif

		if ((OPMODE & WIFI_AP_STATE)) {
			if ( _TRUE == GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_BcnInt) ) {
                // TODO: modify code below
				val8 = *((unsigned char *)priv->beaconbuf + priv->timoffset + 4);
				if (val8 & 0x01) {
#ifdef __ECOS //mark_ecos
					priv->pshare->has_triggered_process_mcast_dzqueue = 1;
#else
					process_mcast_dzqueue(priv);
					priv->pkt_in_dtimQ = 0;
#endif					
				}	
//#ifdef MBSSID
				priv->pshare->bcnDOk_priv = priv;
//#endif
			}
#ifdef MBSSID
			else if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
				for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
					if ((priv->pvap_priv[i]->vap_init_seq > 0) && IS_DRV_OPEN(priv->pvap_priv[i])
						&& (_TRUE == GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_BcnInt1 + (priv->pvap_priv[i]->vap_init_seq-1)))){
						val8 = *((unsigned char *)priv->pvap_priv[i]->beaconbuf + priv->pvap_priv[i]->timoffset + 4);
                       // TODO: modify code below                        
						if (val8 & 0x01) {
							process_mcast_dzqueue(priv->pvap_priv[i]);
							priv->pvap_priv[i]->pkt_in_dtimQ = 0;
						}
						priv->pshare->bcnDOk_priv = priv->pvap_priv[i];
					}
				}
			}
#endif

		}


#if ! defined(CONFIG_WLAN_HAL)
//		if (priv->pshare->pkt_in_hiQ) {
		if (priv->pshare->bcnDOk_priv && priv->pshare->bcnDOk_priv->pkt_in_hiQ) {
			int pre_head = get_txhead(priv->pshare->phw, MCAST_QNUM);
			do {
				txdesc_rollback(&pre_head);
			} while ((get_txdesc_info(priv->pshare->pdesc_info, MCAST_QNUM) + pre_head)->type != _PRE_ALLOCLLCHDR_);
			if (get_desc((get_txdesc(priv->pshare->phw, MCAST_QNUM) + pre_head)->Dword0) & TX_OWN) {
				unsigned short *phdr = (unsigned short *)((get_txdesc_info(priv->pshare->pdesc_info, MCAST_QNUM) + pre_head)->pframe);
#ifdef __MIPSEB__
				phdr = (unsigned short *)KSEG1ADDR(phdr);
#endif
				ClearMData(phdr);
			}
			tx_poll(priv, MCAST_QNUM);
//				priv->pshare->pkt_in_hiQ = 0;
		}
#endif

#ifdef UNIVERSAL_REPEATER
		if (priv_root != NULL)
			priv = priv_root;
#endif

	}

	
#ifdef CLIENT_MODE
	//
	// Ad-hoc beacon status
	//
	if (OPMODE & WIFI_ADHOC_STATE) {
		if (bcnOk)
			priv->ibss_tx_beacon = TRUE;
		if (bcnErr)
			priv->ibss_tx_beacon = FALSE;
	}
#endif
}

__MIPS16
#ifndef CONFIG_ETHWAN
__IRAM_IN_865X
#endif
static __inline__ 
VOID
InterruptRxHandle(
    struct rtl8192cd_priv *priv, BOOLEAN caseRxRDU
)
{
#ifdef __KERNEL__
#if defined(RTL8190_ISR_RX) && defined(RTL8190_DIRECT_RX)
#if defined(RX_TASKLET)
    if (!priv->pshare->has_triggered_rx_tasklet) {
        priv->pshare->has_triggered_rx_tasklet = 1;
        {
            #ifdef  CONFIG_WLAN_HAL
            GET_HAL_INTERFACE(priv)->DisableRxRelatedInterruptHandler(priv);
            #else
            RTL_W32(HIMR, priv->pshare->InterruptMask & ~(HIMR_RXFOVW | HIMR_ROK));
            #endif
        }
        tasklet_hi_schedule(&priv->pshare->rx_tasklet);
    }
#else
    rtl8192cd_rx_isr(priv);
#endif

#else	// !(defined RTL8190_ISR_RX && RTL8190_DIRECT_RX)
    if (caseRxRDU) {
        rtl8192cd_rx_isr(priv);
        if (priv->pshare->rxInt_useTsklt)
        tasklet_hi_schedule(&priv->pshare->rx_tasklet);
        else
            rtl8192cd_rx_dsr((unsigned long)priv);
    }
    else {
        if (priv->pshare->rxInt_useTsklt)
            tasklet_hi_schedule(&priv->pshare->rx_tasklet);
        else
            rtl8192cd_rx_dsr((unsigned long)priv);
    }
#endif

#elif defined(__ECOS) //mark_ecos
	#ifdef  CONFIG_WLAN_HAL
            GET_HAL_INTERFACE(priv)->DisableRxRelatedInterruptHandler(priv);
       #else
            RTL_W32(HIMR, priv->pshare->InterruptMask & ~(HIMR_RXFOVW | HIMR_ROK));
        #endif		
	 #ifdef ISR_DIRECT
	      rtl8192cd_rx_tkl_isr((unsigned long)priv);
	  #else
		priv->pshare->has_triggered_rx_tasklet = 1;
	  #endif
//end  defined(__ECOS)
#else	// !__KERNEL__
    rtl8192cd_rx_dsr((unsigned long)priv);
#endif
}

__MIPS16
#ifndef CONFIG_ETHWAN
__IRAM_IN_865X
#endif
static __inline__ 
VOID
InterruptTxHandle(
   struct rtl8192cd_priv *priv
)
{
    struct rtl8192cd_hw *phw;
    
    phw = GET_HW(priv);
    
#ifdef MP_TEST
#ifdef SMP_SYNC
    if (OPMODE & WIFI_MP_STATE){
        if (!priv->pshare->has_triggered_tx_tasklet) {
            tasklet_schedule(&priv->pshare->tx_tasklet);
            priv->pshare->has_triggered_tx_tasklet = 1;
        }
    }
#else
    if (OPMODE & WIFI_MP_STATE)
        rtl8192cd_tx_dsr((unsigned long)priv);
#endif
    else
#endif
    
    if (GET_HAL_INTERFACE(priv)->QueryTxConditionMatchHandler(priv)) {
#ifdef __KERNEL__
        if (!priv->pshare->has_triggered_tx_tasklet) {
            tasklet_schedule(&priv->pshare->tx_tasklet);
            priv->pshare->has_triggered_tx_tasklet = 1;
        }
#elif defined(__ECOS) //mark_ecos
		priv->pshare->has_triggered_tx_tasklet = 1;		
#else
#ifdef SMP_SYNC
        if (!priv->pshare->has_triggered_tx_tasklet) {
            tasklet_schedule(&priv->pshare->tx_tasklet);
            priv->pshare->has_triggered_tx_tasklet = 1;
        }
#else
        rtl8192cd_tx_dsr((unsigned long)priv);
#endif
#endif
    }     
}


static __inline__ VOID
InterruptPSTimer2Handle(
   struct rtl8192cd_priv *priv
)
{
#if defined (SUPPORT_TX_AMSDU) || defined (SMART_CONCURRENT_92D)
        unsigned long current_value, timeout;
#endif
    
#if 0   // TODO: Modify Code  below
#ifdef SUPPORT_TX_AMSDU
    RTL_W32(IMR, RTL_R32(IMR) & ~IMR_TIMEOUT2);

    current_value = RTL_R32(TSFR) ;
    timeout = RTL_R32(TIMER1);
    if (TSF_LESS(current_value, timeout))
        setup_timer2(priv, timeout);
    else
        amsdu_timeout(priv, current_value);
#endif

#ifdef P2P_SUPPORT
    if( OPMODE&WIFI_P2P_SUPPORT &&  (P2PMODE==P2P_CLIENT)) {
        RTL_W32(HIMR, RTL_R32(HIMR) & ~HIMR_TIMEOUT2);
        p2p_noa_timer(priv);
    }
#endif
#endif
}


#if 1 //Filen_Test
#define PRINT_DATA(_TitleString, _HexData, _HexDataLen)						\
{												\
	char			*szTitle = _TitleString;					\
	pu1Byte		pbtHexData = _HexData;							\
	u4Byte		u4bHexDataLen = _HexDataLen;						\
	u4Byte		__i;									\
	DbgPrint("%s", szTitle);								\
	for (__i=0;__i<u4bHexDataLen;__i++)								\
	{											\
		if ((__i & 15) == 0) 								\
		{										\
			DbgPrint("\n");								\
		}										\
		DbgPrint("%02X%s", pbtHexData[__i], ( ((__i&3)==3) ? "  " : " ") );			\
	}											\
	DbgPrint("\n");										\
}
#endif


#ifdef TXREPORT
extern void requestTxReport88XX(struct rtl8192cd_priv *priv);
void C2H_isr_88XX(struct rtl8192cd_priv *priv)
{
	struct tx_rpt rpt1;
	int j, C2H_ID;
	unsigned char MacID = 0xff;
	int idx = 0x1a2;
	unsigned long flags;

	//WDEBUG("\n");

	SAVE_INT_AND_CLI(flags);
	C2H_ID = RTL_R8(0x1a0);

	if ( (C2H_ID) == 0x04 ) {

		for (j = 0; j < 2; j++) {

			MacID = RTL_R8(idx);
			if (MacID == 0xff)
				continue;

			rpt1.macid =  MacID & (0x1f);

			if (rpt1.macid) {
#ifdef _BIG_ENDIAN_
				rpt1.txok = le16_to_cpu(RTL_R16(idx + 1));
				rpt1.txfail = le16_to_cpu(RTL_R16(idx + 3));
#else
				rpt1.txok = be16_to_cpu(RTL_R16(idx + 1));
				rpt1.txfail = be16_to_cpu(RTL_R16(idx + 3));
#endif
				rpt1.initil_tx_rate = RTL_R8(idx + 5);
				txrpt_handler(priv, &rpt1);	// add inital tx rate handle for 8812E
			}
			idx += 6;
		}
	}


	RTL_W8( 0x1af, 0);
	requestTxReport88XX(priv);
	RESTORE_INT(flags);

}
#endif

#define	RTL_WLAN_INT_RETRY_CNT_MAX	(32)
__MIPS16
#ifndef CONFIG_ETHWAN
__IRAM_IN_865X
#endif
__inline__ static int __rtl_wlan_interrupt(void *dev_instance)

{
    struct net_device       *dev;
    struct rtl8192cd_priv   *priv;

    unsigned int status, status_ext, retry_cnt = 0;
    BOOLEAN caseBcnInt, caseBcnStatusOK, caseBcnStatusER;
    BOOLEAN caseRxRDU, caseRxOK, caseRxFOVW, caseTxFOVW , caseTxErr , caseRxErr;
	BOOLEAN caseC2HIsr;

#if 1   //Filen: temp
    static unsigned int caseRxRDUCnt=0, caseRxOKCnt=0;
#endif
    
#if	defined(SUPPORT_TX_AMSDU) || defined(P2P_SUPPORT)
    BOOLEAN caseTimer2;
#endif

    dev = (struct net_device *)dev_instance;
#ifdef NETDEV_NO_PRIV
    priv = ((struct rtl8192cd_priv *)netdev_priv(dev))->wlan_priv;
#else
    priv = (struct rtl8192cd_priv *)dev->priv;
#endif

    if(_FALSE == GET_HAL_INTERFACE(priv)->InterruptRecognizedHandler(priv, NULL, 0)) {
        printk("NULL Wlan Interrupt !?\n");
        return SUCCESS;
    }

    //Break Condition, satisfy one of condtion below:
    //   1.) retry cnt until our setting value
    //   2.) No interupt pending
    while(1) {
        //4 Initialize
        caseBcnInt          = FALSE;
        caseBcnStatusOK     = FALSE;
        caseBcnStatusER     = FALSE;

        caseRxRDU           = FALSE;
        caseRxOK            = FALSE;
        caseRxFOVW          = FALSE;

#if	defined(SUPPORT_TX_AMSDU) || defined(P2P_SUPPORT)
        caseTimer2 = FALSE;
#endif

        //4 Check interrupt handler
        // 1.) Beacon
        caseBcnInt      = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_BcnInt_MBSSID);
        caseBcnStatusOK = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_TBDOK);
        caseBcnStatusER = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_TBDER);

        if (TRUE == caseBcnInt || TRUE == caseBcnStatusOK || TRUE == caseBcnStatusER) {
            rtl88XX_bcnProc(priv, caseBcnInt, caseBcnStatusOK, caseBcnStatusER);
        }

#ifdef TXREPORT
		caseC2HIsr = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_C2HCMD);
		if (TRUE == caseC2HIsr) {
			C2H_isr_88XX(priv);
		}
#endif

        // 2.) Rx
        caseRxRDU = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_RDU);
        if (TRUE == caseRxRDU) {
            caseRxRDUCnt++; //filen: temp
            priv->ext_stats.rx_rdu++;
            priv->pshare->skip_mic_chk = SKIP_MIC_NUM;            
        }
        
        caseRxOK = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_RX_OK);
        if (TRUE == caseRxOK) {
            caseRxOKCnt++; //filen: temp
        }
        
        caseRxFOVW = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_RXFOVW);
        if (TRUE == caseRxFOVW) {
            priv->ext_stats.rx_fifoO++;
            priv->pshare->skip_mic_chk = SKIP_MIC_NUM;            
        }

        if (TRUE == caseRxRDU || TRUE == caseRxOK || TRUE == caseRxFOVW) {
            InterruptRxHandle(priv, caseRxRDU);
        }

        // 3.) Tx
        InterruptTxHandle(priv);

        // 4.) check DMA error
        caseTxFOVW = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_TXFOVW);
        caseTxErr  = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_TXERR);
        caseRxErr  = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_RXERR);

        if (TRUE == caseTxFOVW || TRUE == caseTxErr) {
            // check Tx DMA error
            u4Byte  TxDMAStatus = 0;
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_NUM_TXDMA_STATUS, (pu4Byte)&TxDMAStatus);

            if(TxDMAStatus)
            {
               printk("TXDMA Error TxDMAStatus =%x\n",TxDMAStatus);
               priv->ext_stats.tx_dma_err++;		               
               GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_NUM_TXDMA_STATUS, (pu4Byte)&TxDMAStatus);            
            }            
        }        

        if (TRUE == caseRxFOVW || TRUE == caseRxErr) {
            // check Rx DMA error            
            u1Byte  RxDMAStatus = 0;
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_NUM_RXDMA_STATUS, (pu1Byte)&RxDMAStatus);

            if(RxDMAStatus)
            {
               printk("RXDMA Error RxDMAStatus =%x\n",RxDMAStatus);
               priv->ext_stats.rx_dma_err++;		               
               GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_NUM_RXDMA_STATUS, (pu1Byte)&RxDMAStatus);            
            }        
        }

        // 4.) TX_AMSDU & P2P
#if 0   // TODO: Check Code       
#if	defined(SUPPORT_TX_AMSDU) || defined(P2P_SUPPORT)        
        caseTimer2  = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_PSTIMEOUT2);
        if ( TRUE == caseTimer2 ) {
            InterruptPSTimer2Handle(priv);
        }
#endif
#endif

        //4 Check Break Condition
        if(_FALSE == GET_HAL_INTERFACE(priv)->InterruptRecognizedHandler(priv, NULL, 0)) {
            break;
        }
        else {
            retry_cnt++;

            if ( retry_cnt >= RTL_WLAN_INT_RETRY_CNT_MAX ) {
                break;
            }
            else {
#if defined(CONFIG_RTL865X_WTDOG) || defined(CONFIG_RTL_WTDOG)
                REG32(BSP_WDTCNR) |=  1 << 23;
#endif
            }
        }
    }

    return SUCCESS;
}

#endif  //CONFIG_WLAN_HAL
void check_dma_error(struct rtl8192cd_priv *priv, unsigned int status, unsigned int status_ext)
{
	unsigned char reg_rxdma;
	unsigned int reg_txdma;
	int clear_isr=0, check_tx_dma=0, check_rx_dma=0;

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
		if ((status_ext & HIMRE_88E_RXFOVW) | (status_ext & HIMRE_88E_RXERR))
			check_rx_dma++;
		if ((status_ext & HIMRE_88E_TXFOVW) | (status_ext & HIMRE_88E_TXERR))
			check_tx_dma++;
	} else
#endif
	{
		if ((status & HIMR_RXFOVW) | (status_ext & HIMRE_RXERR))
			check_rx_dma++;

		if ((status & HIMR_TXFOVW) | (status_ext & HIMRE_TXERR))
			check_tx_dma++;
	}

	if (check_rx_dma) {
		reg_rxdma = RTL_R8(RXDMA_STATUS);
		if (reg_rxdma) {
			RTL_W8(RXDMA_STATUS, reg_rxdma);
			//panic_printk("RXDMA_STATUS %02x\n", reg_rxdma);
			priv->ext_stats.rx_dma_err++;
			clear_isr = 1;
		}
	}

	if (check_tx_dma) {
		reg_txdma = RTL_R32(TXDMA_STATUS);
		if (reg_txdma) {
			RTL_W32(TXDMA_STATUS, reg_txdma);
			//panic_printk("TXDMA_STATUS %08x\n", reg_txdma);
			priv->ext_stats.tx_dma_err++;			
			clear_isr = 1;
		}
	}

	if (clear_isr) {
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8188E) {
			RTL_W32(REG_88E_HISR, status);
			RTL_W32(REG_88E_HISRE, status_ext);
		} else
#endif
		{
			RTL_W32(HISR, status);
			RTL_W32(HISRE, status_ext);
		}
	}
}


#define	RTL_WLAN_INT_RETRY_MAX	(32)
__MIPS16
#ifndef CONFIG_ETHWAN
__IRAM_IN_865X
#endif
#ifdef __ECOS
int __rtl8192cd_interrupt(struct net_device *dev_instance)
#else
__inline__ static int __rtl8192cd_interrupt(void *dev_instance)
#endif
{
	struct net_device *dev;
	struct rtl8192cd_priv *priv;
	struct rtl8192cd_hw *phw;

	unsigned int status, status_ext, retry_cnt;
	unsigned int caseBcnInt, caseBcnStatusOK, caseBcnStatusER, caseBcnDmaOK=0;
	unsigned int caseRxStatus, caseRxRDU;
	#if 0 //TESTCHIP_SUPPORT || defined(SMART_CONCURRENT_92D) // --fwdebug
	unsigned int caseTimer1;
	#endif
	
	#if	defined(SUPPORT_TX_AMSDU) || defined(P2P_SUPPORT)
	unsigned int caseTimer2;
	#endif
#if defined (SUPPORT_TX_AMSDU) || defined (SMART_CONCURRENT_92D)
	unsigned long current_value, timeout;
#endif

	dev = (struct net_device *)dev_instance;
#ifdef NETDEV_NO_PRIV
	priv = ((struct rtl8192cd_priv *)netdev_priv(dev))->wlan_priv;
#else
	priv = (struct rtl8192cd_priv *)dev->priv;
#endif

#ifdef __ECOS
#ifdef  CONFIG_WLAN_HAL //mark_ecos
	if (IS_HAL_CHIP(priv)) 
	{
	    __rtl_wlan_interrupt(dev_instance);
	    goto int_exit;  	
	}	
#endif
#endif
#if defined(CONFIG_RTL_92D_DMDP) && !defined(NOT_RTK_BSP)
	if (GET_CHIP_VER(priv)==VERSION_8192D) {
	#if (RTL_USED_PCIE_SLOT==1)
	 	if (!((REG32(0xb8b21004)& 0x01) && (priv->pshare->wlandev_idx ==0)) &&
			!((REG32(0xb8b21004)& 0x02) && (priv->pshare->wlandev_idx ==1))) {
			//printk("INT=[%02x] WLAN(%d)\n",(REG32(0xb8b21004)& 0x0f),(priv->pshare->wlandev_idx));
			goto int_exit;
		}
	#else
	 	if (!((REG32(0xb8b01004)& 0x01) && (priv->pshare->wlandev_idx ==0)) &&
			!((REG32(0xb8b01004)& 0x02) && (priv->pshare->wlandev_idx ==1))) {
			//printk("INT=[%02x] WLAN(%d)\n",(REG32(0xb8b21004)& 0x0f),(priv->pshare->wlandev_idx));
			goto int_exit;
		}
	#endif
	}
#endif 

#ifdef PCIE_POWER_SAVING
	if ((priv->pwr_state == L2) || (priv->pwr_state == L1)) {
#ifdef CONFIG_RTL_92D_DMDP
		REG32(0xb8003000) = REG32(0xb8003000)|BIT(22);
#endif
		goto int_exit;
	}
#endif

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
		status = RTL_R32(REG_88E_HISR);
		RTL_W32(REG_88E_HISR, status);
		if (status & HIMR_88E_HISR1_IND_INT) {
			status_ext = RTL_R32(REG_88E_HISRE);
			RTL_W32(REG_88E_HISRE, status_ext);
		} else {
			status_ext = 0;
		}
	} else
#endif

#ifdef CONFIG_RTL_8812_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8812E) {
		status = RTL_R32(REG_HISR0_8812);
		RTL_W32(REG_HISR0_8812, status);
			
		status_ext = RTL_R32(REG_HISR1_8812);
		RTL_W32(REG_HISR1_8812, status_ext);
	} else
#endif
#if defined(CONFIG_WLAN_HAL_8192EE)
	if (GET_CHIP_VER(priv)==VERSION_8192E) {
		status = RTL_R32(REG_92E_HISR);
		RTL_W32(REG_92E_HISR, status);
		
		status_ext = RTL_R32(REG_92E_HISRE);
		RTL_W32(REG_92E_HISRE, status_ext);
	} else
#endif
	{
		status = RTL_R32(HISR);
		RTL_W32(HISR, status);
		status_ext = RTL_R32(HISRE);
		RTL_W32(HISRE, status_ext);
	}
	if (status == 0 && status_ext == 0) {
		goto int_exit;
	}
	
	retry_cnt = 0;
retry_process:
	
	caseBcnInt = caseBcnStatusOK = caseBcnStatusER = caseBcnDmaOK = 0;
	caseRxStatus = caseRxRDU = 0;
	#if 0 //TESTCHIP_SUPPORT || defined(SMART_CONCURRENT_92D) // --fwdebug
	caseTimer1 = 0;
	#endif
	
	#if	defined(SUPPORT_TX_AMSDU) || defined(P2P_SUPPORT)
	caseTimer2 = 0;
	#endif
	
#if defined(TXREPORT) 
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
	if (CHIP_VER_92X_SERIES(priv)) {		
		if(status_ext & BIT(9))
#ifdef __ECOS
			priv->pshare->has_triggered_C2H_isr = 1;
#else
			C2H_isr(priv);
#endif

	}
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
	if(GET_CHIP_VER(priv)==VERSION_8812E){		
		if(status & HIMR_92E_C2HCMD){
			C2H_isr_8812(priv);
		}
	}
#endif	
#endif


	if(GET_CHIP_VER(priv)!= VERSION_8812E)
	{
		check_dma_error(priv, status, status_ext);
	}

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
		if ((status & HIMR_88E_BcnInt) || (status_ext & (HIMRE_88E_BCNDMAINT1 | HIMRE_88E_BCNDMAINT2
			| HIMRE_88E_BCNDMAINT3 | HIMRE_88E_BCNDMAINT4 | HIMRE_88E_BCNDMAINT5
			| HIMRE_88E_BCNDMAINT6 | HIMRE_88E_BCNDMAINT7))) {
			caseBcnInt = 1;
		}

		if (status & HIMR_88E_TBDOK)
			caseBcnStatusOK = 1;

		if (status & HIMR_88E_TBDER)
			caseBcnStatusER = 1;

		if (status & (HIMR_88E_ROK | HIMR_88E_RDU)) {
			caseRxStatus = 1;

			if (status & HIMR_88E_RDU) {
				priv->ext_stats.rx_rdu++;
				caseRxRDU = 1;
				priv->pshare->skip_mic_chk = SKIP_MIC_NUM;
			}
		}

		if (status_ext & HIMRE_88E_RXFOVW) {
			priv->ext_stats.rx_fifoO++;
			priv->pshare->skip_mic_chk = SKIP_MIC_NUM;
		}
	} else
#endif
#if defined(CONFIG_RTL_8812_SUPPORT)
	if((GET_CHIP_VER(priv)==VERSION_8812E)) {
		if ((status & HIMR_92E_BcnInt) || (status_ext & (HIMRE_92E_BCNDMAINT1 | HIMRE_92E_BCNDMAINT2
			| HIMRE_92E_BCNDMAINT3 | HIMRE_92E_BCNDMAINT4 | HIMRE_92E_BCNDMAINT5
			| HIMRE_92E_BCNDMAINT6 | HIMRE_92E_BCNDMAINT7))) {
			caseBcnInt = 1;
		}
	
		if (status & HIMR_92E_TBDOK)
			caseBcnStatusOK = 1;
	
		if (status & HIMR_92E_TBDER)
			caseBcnStatusER = 1;
	
		if (status & (HIMR_92E_ROK | HIMR_92E_RDU)) {
			caseRxStatus = 1;
	
    		if (status & HIMR_92E_RDU) {
	    		priv->ext_stats.rx_rdu++;
		    	caseRxRDU = 1;
			    priv->pshare->skip_mic_chk = SKIP_MIC_NUM;
			}
		}
	
		if (status_ext & HIMRE_92E_RXFOVW) {
			priv->ext_stats.rx_fifoO++;
			priv->pshare->skip_mic_chk = SKIP_MIC_NUM;
		}
	} else
#endif
	{
		if (status & (HIMR_BCNDMA0 | HIMR_BCNDMA1 | HIMR_BCNDMA2 | HIMR_BCNDMA3 | HIMR_BCNDMA4 | HIMR_BCNDMA5 | HIMR_BCNDMA6 | HIMR_BCNDMA7))
			caseBcnInt = 1;
		if (status & (HIMR_BCNDOK0 | HIMR_BCNDOK1 | HIMR_BCNDOK2 | HIMR_BCNDOK3 | HIMR_BCNDOK4 | HIMR_BCNDOK5 | HIMR_BCNDOK6 | HIMR_BCNDOK7))
			caseBcnDmaOK = 1;

		if (status & HIMR_TXBCNOK)
			caseBcnStatusOK = 1;

		if (status & HIMR_TXBCNERR)
			caseBcnStatusER = 1;

		if (status & (HIMR_ROK | HIMR_RDU))
			caseRxStatus = 1;

		if (status & HIMR_RDU) {
			priv->ext_stats.rx_rdu++;
			caseRxRDU = 1;
			priv->pshare->skip_mic_chk = SKIP_MIC_NUM;
		}

		if (status & HIMR_RXFOVW) {
			priv->ext_stats.rx_fifoO++;
			priv->pshare->skip_mic_chk = SKIP_MIC_NUM;
		}
	}

	#if 0 //TESTCHIP_SUPPORT || defined(SMART_CONCURRENT_92D) // --fwdebug
	if (status & HIMR_TIMEOUT1)
		caseTimer1 = 1;
	#endif

	#if	defined(SUPPORT_TX_AMSDU) || defined(P2P_SUPPORT)

#ifdef CONFIG_RTL_8812_SUPPORT
	if ( (GET_CHIP_VER(priv)== VERSION_8812E)) {
		if (status & IMR_TIMER2_8812)
				caseTimer2 = 1;
	} else 
#endif		
	{
		if (status & HIMR_TIMEOUT2)
		caseTimer2 = 1;
	}
	#endif

	if (caseBcnInt || caseBcnStatusOK || caseBcnStatusER || caseBcnDmaOK){
                rtl8192cd_bcnProc(priv, caseBcnInt, caseBcnStatusOK, caseBcnStatusER, status
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT)
			, status_ext
#endif
			);
        }

	//
	// Rx interrupt
	//
	if (caseRxStatus)
	{
		// stop RX first
#ifdef __KERNEL__
#if defined(RTL8190_ISR_RX) && defined(RTL8190_DIRECT_RX)
#if defined(RX_TASKLET)
		if (!priv->pshare->has_triggered_rx_tasklet) {
			priv->pshare->has_triggered_rx_tasklet = 1;
#ifdef CONFIG_RTL_88E_SUPPORT
			if (GET_CHIP_VER(priv)==VERSION_8188E) {
				RTL_W32(REG_88E_HIMR, priv->pshare->InterruptMask & ~HIMR_88E_ROK);
				RTL_W32(REG_88E_HIMRE, priv->pshare->InterruptMaskExt & ~HIMRE_88E_RXFOVW);
			} else
#endif
			{
				//RTL_W32(HIMR, priv->pshare->InterruptMask & ~(HIMR_RXFOVW | HIMR_RDU | HIMR_ROK));

#ifdef CONFIG_RTL_8812_SUPPORT
				if(GET_CHIP_VER(priv)== VERSION_8812E){
					RTL_W32(REG_HIMR0_8812, priv->pshare->InterruptMask & ~(HIMR_92E_ROK ));
					RTL_W32(REG_HIMR1_8812, priv->pshare->InterruptMaskExt & ~( HIMRE_92E_RXFOVW));
				}
				else
#endif
				RTL_W32(HIMR, priv->pshare->InterruptMask & ~(HIMR_RXFOVW | HIMR_ROK));
			}
			tasklet_hi_schedule(&priv->pshare->rx_tasklet);
		}
#else
		rtl8192cd_rx_isr(priv);
#endif

#else	// !(defined RTL8190_ISR_RX && RTL8190_DIRECT_RX)
		if (caseRxRDU) {
			rtl8192cd_rx_isr(priv);
			tasklet_hi_schedule(&priv->pshare->rx_tasklet);
		}
		else {
			if (priv->pshare->rxInt_useTsklt)
				tasklet_hi_schedule(&priv->pshare->rx_tasklet);
			else
				rtl8192cd_rx_dsr((unsigned long)priv);
		}
#endif
#elif defined(__ECOS)
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8188E) {
			RTL_W32(REG_88E_HIMR, priv->pshare->InterruptMask & ~HIMR_88E_ROK);
			RTL_W32(REG_88E_HIMRE, priv->pshare->InterruptMaskExt & ~HIMRE_88E_RXFOVW);
		} else
#endif
		{
			RTL_W32(HIMR, priv->pshare->InterruptMask & ~(HIMR_RXFOVW | HIMR_ROK));
		}
#ifdef ISR_DIRECT
		rtl8192cd_rx_tkl_isr((unsigned long)priv);
#else
		priv->pshare->has_triggered_rx_tasklet = 1;
#endif
#else	// !__KERNEL__
		rtl8192cd_rx_dsr((unsigned long)priv);
#endif
	}

	//
	// Tx interrupt
	//
	phw = GET_HW(priv);
#ifdef MP_TEST
#ifdef SMP_SYNC
	if (OPMODE & WIFI_MP_STATE){
		if (!priv->pshare->has_triggered_tx_tasklet) {
			tasklet_schedule(&priv->pshare->tx_tasklet);
			priv->pshare->has_triggered_tx_tasklet = 1;
		}
	}
#else
	if (OPMODE & WIFI_MP_STATE)
		rtl8192cd_tx_dsr((unsigned long)priv);
#endif
	else
#endif
	if ((CIRC_CNT_RTK(phw->txhead0, phw->txtail0, CURRENT_NUM_TX_DESC) > 10) ||
		(CIRC_CNT_RTK(phw->txhead1, phw->txtail1, CURRENT_NUM_TX_DESC) > 10) ||
		(CIRC_CNT_RTK(phw->txhead2, phw->txtail2, CURRENT_NUM_TX_DESC) > 10) ||
		(CIRC_CNT_RTK(phw->txhead3, phw->txtail3, CURRENT_NUM_TX_DESC) > 10) ||
		(CIRC_CNT_RTK(phw->txhead4, phw->txtail4, CURRENT_NUM_TX_DESC) > 10) ||
		(CIRC_CNT_RTK(phw->txhead5, phw->txtail5, CURRENT_NUM_TX_DESC) > 10)
	) {
#ifdef __KERNEL__
		if (!priv->pshare->has_triggered_tx_tasklet) {
			tasklet_schedule(&priv->pshare->tx_tasklet);
			priv->pshare->has_triggered_tx_tasklet = 1;
		}
#elif defined(__ECOS)
		priv->pshare->has_triggered_tx_tasklet = 1;
#else
#ifdef SMP_SYNC
		if (!priv->pshare->has_triggered_tx_tasklet) {
			tasklet_schedule(&priv->pshare->tx_tasklet);
			priv->pshare->has_triggered_tx_tasklet = 1;
		}
#else
		rtl8192cd_tx_dsr((unsigned long)priv);
#endif
#endif
	}

#if 0 //TESTCHIP_SUPPORT
#ifdef SMART_CONCURRENT_92D // --fwdebug

#ifdef PHASE2_TEST
	if (caseTimer1 && priv->pshare->wlandev_idx == 1) {
		cancel_timer1(priv);
		current_value = RTL_R32(TSFTR) ;
		timeout = RTL_R32(TIMER0);
		if (TSF_LESS(current_value, timeout)){
			setup_timer1(priv, timeout);
		}else {
			//printk("TIMER1 %d \n", current_value);
			RTL_W8_Phy0(priv, 0x1cf, RTL_R8_Phy0(priv, 0x1cf)|BIT(2));  // write MAC'0 0x1cf to notify firmware h2c cmd
		}
	}
#endif	
	if (caseTimer2 && priv->pshare->wlandev_idx == 1) {
		cancel_timer2(priv);
		
		current_value = RTL_R32(TSFTR) ;
		timeout = RTL_R32(TIMER1);
		if (TSF_LESS(current_value, timeout)){
			setup_timer2(priv, timeout);
		}else {
			//printk("TIMER2 %d \n", current_value);
			DMDP_RTL_W8(0, 0x1cf, DMDP_RTL_R8(0, 0x1cf)|BIT(3));  // write MAC'0 0x1cf to notify firmware h2c cmd
		}
	}
#endif
#endif

#ifdef SUPPORT_TX_AMSDU
	if (caseTimer2) {

#ifdef CONFIG_RTL_8812_SUPPORT
		if(GET_CHIP_VER(priv)== VERSION_8812E)
			RTL_W32(REG_HIMR0_8812, RTL_R32(REG_HIMR0_8812) & ~	IMR_TIMER2_8812	);
		else
#endif
		RTL_W32(HIMR, RTL_R32(HIMR) & ~HIMR_TIMEOUT2);

		current_value = RTL_R32(TSFTR) ;
		timeout = RTL_R32(TIMER1);
		if (TSF_LESS(current_value, timeout))
			setup_timer2(priv, timeout);
		else
			amsdu_timeout(priv, current_value);
	}
#endif


#ifdef P2P_SUPPORT
	if( OPMODE&WIFI_P2P_SUPPORT &&	(P2PMODE==P2P_CLIENT)) {
		if (caseTimer2) {
			RTL_W32(HIMR, RTL_R32(HIMR) & ~HIMR_TIMEOUT2);
			p2p_noa_timer(priv);
		}
	}
#endif

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
		status = RTL_R32(REG_88E_HISR);
		if (status & HIMR_88E_HISR1_IND_INT)
			status_ext = RTL_R32(REG_88E_HISRE);
		else
			status_ext = 0;
	} else
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8812E) {
		status = RTL_R32(REG_HISR0_8812);
		if (status & HIMR_92E_HISR1_IND_INT)
			status_ext = RTL_R32(REG_HISR1_8812);
		else
			status_ext = 0;
	} else
#endif

	{
		status = RTL_R32(HISR);
		status_ext = RTL_R32(HISRE);
	}

	if ((status!=0||status_ext!=0) && ((retry_cnt++)<RTL_WLAN_INT_RETRY_MAX)) {
		#if defined(CONFIG_RTL865X_WTDOG) || defined(CONFIG_RTL_WTDOG)
#ifdef CONFIG_RTL_8198B
		REG32(BSP_WDTCNTRR) |= BSP_WDT_KICK;
#else
		REG32(BSP_WDTCNR) |=  1 << 23;
#endif
		#endif
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8188E) {
			RTL_W32(REG_88E_HISR, status);
			RTL_W32(REG_88E_HISRE, status_ext);
		} else
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8812E) {
			RTL_W32(REG_HISR0_8812, status);
			RTL_W32(REG_HISR1_8812, status_ext);
		} else
#endif

		{
			RTL_W32(HISR, status);
			RTL_W32(HISRE, status_ext);
		}

		goto retry_process;
	}

int_exit:
	return SUCCESS;
}


#ifndef __ECOS
#ifdef __LINUX_2_6__
__MIPS16
__IRAM_IN_865X
irqreturn_t rtl8192cd_interrupt(int irq, void *dev_instance)
{
#ifdef  CONFIG_WLAN_HAL
	struct net_device *dev = (struct net_device *)dev_instance;
#ifdef NETDEV_NO_PRIV
	struct rtl8192cd_priv *priv = ((struct rtl8192cd_priv *)netdev_priv(dev))->wlan_priv;
#else
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
#endif	
	if (IS_HAL_CHIP(priv)) 
	    __rtl_wlan_interrupt(dev_instance);
	else
#endif	
	__rtl8192cd_interrupt(dev_instance);
	return IRQ_HANDLED;
}
#else
__MIPS16
__IRAM_IN_865X
void rtl8192cd_interrupt(int irq, void *dev_instance, struct pt_regs *regs)
{
#ifdef  CONFIG_WLAN_HAL
	struct net_device *dev = (struct net_device *)dev_instance;
#ifdef NETDEV_NO_PRIV
	struct rtl8192cd_priv *priv = ((struct rtl8192cd_priv *)netdev_priv(dev))->wlan_priv;
#else
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
#endif	
	if (IS_HAL_CHIP(priv)) 
        __rtl_wlan_interrupt(dev_instance);
	else
#endif  //CONFIG_WLAN_HAL
        __rtl8192cd_interrupt(dev_instance);

	return;
}
#endif
#endif /* ! __ECOS */


#ifdef __KERNEL__
static void rtl8192cd_set_rx_mode(struct net_device *dev)
{

}
#endif /* __KERNEL__ */


static struct net_device_stats *rtl8192cd_get_stats(struct net_device *dev)
{
#ifdef NETDEV_NO_PRIV
	struct rtl8192cd_priv *priv = ((struct rtl8192cd_priv *)netdev_priv(dev))->wlan_priv;
#else
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
#endif

#ifdef SMP_SYNC
	unsigned long flags;
#endif

#ifdef WDS
	int idx;
	struct stat_info *pstat;
#endif

	SMP_LOCK(flags);

#ifdef WDS
	if (dev->base_addr == 0) {
		idx = getWdsIdxByDev(priv, dev);
		if (idx < 0) {
			memset(&priv->wds_stats[NUM_WDS-1], 0, sizeof(struct net_device_stats));
			SMP_UNLOCK(flags);
			return &priv->wds_stats[NUM_WDS-1];
		}

		if (netif_running(dev) && netif_running(priv->dev)) {
			pstat = get_stainfo(priv, priv->pmib->dot11WdsInfo.entry[idx].macAddr);
			if (pstat == NULL) {
				DEBUG_ERR("%s: get_stainfo() wds fail!\n", (char *)__FUNCTION__);
				memset(&priv->wds_stats[idx], 0, sizeof(struct net_device_stats));
			}
			else {
				priv->wds_stats[idx].tx_packets = pstat->tx_pkts;
				priv->wds_stats[idx].tx_errors = pstat->tx_fail;
				priv->wds_stats[idx].tx_bytes = pstat->tx_bytes;
				priv->wds_stats[idx].rx_packets = pstat->rx_pkts;
				priv->wds_stats[idx].rx_bytes = pstat->rx_bytes;
			}
		}
		SMP_UNLOCK(flags);
		return &priv->wds_stats[idx];
	}
#endif

#ifdef CONFIG_RTK_MESH

	if (dev->base_addr == 1) {
		if(priv->mesh_dev != dev)
		{
			SMP_UNLOCK(flags);
			return NULL;
		}

		SMP_UNLOCK(flags);
		return &priv->mesh_stats;
	}
#endif // CONFIG_RTK_MESH

	SMP_UNLOCK(flags);
	return &(priv->net_stats);
}

#ifdef CONFIG_WLAN_HAL
static void
rtl88xx_init_swtxdec(
    struct rtl8192cd_priv *priv
)
{
    struct rtl8192cd_hw *phw=NULL; 
    struct tx_desc_info *tx_info;
    u4Byte max_qnum = HIGH_QUEUE7;
    u4Byte QueueIdx;
    u4Byte i;

    phw = GET_HW(priv);

    for (QueueIdx=0; QueueIdx<=max_qnum; QueueIdx++) {
        tx_info         = get_txdesc_info(&phw->tx_info, QueueIdx);
        tx_info->type   = _RESERVED_FRAME_TYPE_;

        for (i=0; i<(TXBD_ELE_NUM-2);i++) {
            tx_info->buf_type[i] = _RESERVED_FRAME_TYPE_;
        }
    }
}

#endif //CONFIG_WLAN_HAL

static int rtl8192cd_init_sw(struct rtl8192cd_priv *priv)
{
	// All the index/counters should be reset to zero...
	struct rtl8192cd_hw *phw=NULL;
	unsigned long offset;
	unsigned int  i;
	struct sk_buff	*pskb;
	unsigned char	*page_ptr;
	struct wlan_hdr_poll	*pwlan_hdr_poll;
	struct wlanllc_hdr_poll	*pwlanllc_hdr_poll;
	struct wlanbuf_poll		*pwlanbuf_poll;
	struct wlanicv_poll		*pwlanicv_poll;
	struct wlanmic_poll		*pwlanmic_poll;
	struct wlan_acl_poll	*pwlan_acl_poll;
#ifdef _MESH_ACL_ENABLE_
	struct mesh_acl_poll	*pmesh_acl_poll;
#endif
	unsigned long ring_virt_addr;
	unsigned long ring_dma_addr;
	unsigned int  ring_buf_len;
	unsigned char MIMO_TR_hw_support;
	unsigned int NumTotalRFPath;
#ifndef PRIV_STA_BUF
	unsigned long alloc_dma_buf;
#endif
#if defined(CLIENT_MODE) && defined(CHECK_HANGUP)
	unsigned char *pbackup=NULL;
	unsigned long backup_len=0;
#endif
#ifdef _11s_TEST_MODE_
	struct Galileo_poll 	*pgalileo_poll;
#endif

#ifndef USE_RTL8186_SDK
	unsigned int tx_dma_start;
	struct tx_desc *tx_desc_ptr;
#endif

#ifdef DFS
	/*
	 *	For JAPAN : prevent switching to channels 52, 56, 60, and 64 in adhoc mode
	 */
	if (((priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK) ||
		 (priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK3)) &&
		(OPMODE & WIFI_ADHOC_STATE)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) {
			/* block channels 52~64 and place them in NOP_chnl */
			if (!timer_pending(&priv->ch52_timer))
				InsertChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 52);
			if (!timer_pending(&priv->ch56_timer))
				InsertChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 56);
			if (!timer_pending(&priv->ch60_timer))
				InsertChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 60);
			if (!timer_pending(&priv->ch64_timer))
				InsertChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 64);
		}

		/* if users select an illegal channel, the driver will switch to channel 36~48 */
		if ((priv->pmib->dot11RFEntry.dot11channel >= 52) && (priv->pmib->dot11RFEntry.dot11channel <= 64)) {
			PRINT_INFO("Channel %d is illegal in ad-hoc mode in Japan!\n", priv->pmib->dot11RFEntry.dot11channel);
			priv->pmib->dot11RFEntry.dot11channel = DFS_SelectChannel(priv);
			PRINT_INFO("Swiching to channel %d!\n", priv->pmib->dot11RFEntry.dot11channel);
		}
	}

	/* if users select a blocked channel, the driver will switch to unblocked channel */
	if (!priv->pmib->dot11DFSEntry.disable_DFS &&
		((timer_pending(&priv->ch52_timer) && (priv->pmib->dot11RFEntry.dot11channel == 52)) ||
		 (timer_pending(&priv->ch56_timer) && (priv->pmib->dot11RFEntry.dot11channel == 56)) ||
		 (timer_pending(&priv->ch60_timer) && (priv->pmib->dot11RFEntry.dot11channel == 60)) ||
		 (timer_pending(&priv->ch64_timer) && (priv->pmib->dot11RFEntry.dot11channel == 64)) || 
		 (timer_pending(&priv->ch100_timer) && (priv->pmib->dot11RFEntry.dot11channel == 100)) ||
		 (timer_pending(&priv->ch104_timer) && (priv->pmib->dot11RFEntry.dot11channel == 104)) ||
		 (timer_pending(&priv->ch108_timer) && (priv->pmib->dot11RFEntry.dot11channel == 108)) ||
		 (timer_pending(&priv->ch112_timer) && (priv->pmib->dot11RFEntry.dot11channel == 112)) ||
		 (timer_pending(&priv->ch116_timer) && (priv->pmib->dot11RFEntry.dot11channel == 116)) ||
		 (timer_pending(&priv->ch120_timer) && (priv->pmib->dot11RFEntry.dot11channel == 120)) ||
		 (timer_pending(&priv->ch124_timer) && (priv->pmib->dot11RFEntry.dot11channel == 124)) ||
		 (timer_pending(&priv->ch128_timer) && (priv->pmib->dot11RFEntry.dot11channel == 128)) ||
		 (timer_pending(&priv->ch132_timer) && (priv->pmib->dot11RFEntry.dot11channel == 132)) ||
		 (timer_pending(&priv->ch136_timer) && (priv->pmib->dot11RFEntry.dot11channel == 136)) ||
		 (timer_pending(&priv->ch140_timer) && (priv->pmib->dot11RFEntry.dot11channel == 140)))) {
		PRINT_INFO("Channel %d is still in none occupancy period!\n", priv->pmib->dot11RFEntry.dot11channel);
		priv->pmib->dot11RFEntry.dot11channel = DFS_SelectChannel(priv);
		PRINT_INFO("Swiching to channel %d!\n", priv->pmib->dot11RFEntry.dot11channel);
	}

	/* disable all of the transmissions during channel availability check */
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
		priv->pmib->dot11DFSEntry.disable_tx = 0;
		if (!priv->pmib->dot11DFSEntry.disable_DFS &&
			(((priv->pmib->dot11RFEntry.dot11channel >= 52) &&
			(priv->pmib->dot11RFEntry.dot11channel <= 64))  || 
			((priv->pmib->dot11RFEntry.dot11channel >= 100) &&
			(priv->pmib->dot11RFEntry.dot11channel <= 140))) &&
			(OPMODE & WIFI_AP_STATE))
			priv->pmib->dot11DFSEntry.disable_tx = 1;
	}
#endif

#ifdef ENABLE_RTL_SKB_STATS
 	rtl_atomic_set(&priv->rtl_tx_skb_cnt, 0);
 	rtl_atomic_set(&priv->rtl_rx_skb_cnt, 0);
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#ifdef __KERNEL__
#ifdef DFS
		/* will not initialize the tasklet if the driver is rebooting due to the detection of radar */
		if (!priv->pmib->dot11DFSEntry.DFS_detected)
#endif
		{
#ifdef CHECK_HANGUP
			if (!priv->reset_hangup)
#endif
			{
#ifdef SMART_REPEATER_MODE
				if (!priv->pshare->switch_chan_rp)
#endif
				{
#ifdef PCIE_POWER_SAVING
					tasklet_init(&priv->pshare->ps_tasklet, PCIe_power_save_tasklet, (unsigned long)priv);
#endif
#if !(defined(RTL8190_ISR_RX) && defined(RTL8190_DIRECT_RX))
					tasklet_init(&priv->pshare->rx_tasklet, rtl8192cd_rx_dsr, (unsigned long)priv);
#else
#ifdef RX_TASKLET
					tasklet_init(&priv->pshare->rx_tasklet, rtl8192cd_rx_tkl_isr, (unsigned long)priv);
#endif
#endif
					tasklet_init(&priv->pshare->tx_tasklet, rtl8192cd_tx_dsr, (unsigned long)priv);
					tasklet_init(&priv->pshare->oneSec_tasklet, rtl8192cd_expire_timer, (unsigned long)priv);
				}
			}
		}
#endif // __KERNEL__

#ifdef DFS
		if (priv->pmib->dot11DFSEntry.DFS_detected)
			priv->pmib->dot11DFSEntry.DFS_detected = 0;
#endif

		phw = GET_HW(priv);

		// save descriptor virtual address before reset, david
		ring_virt_addr = phw->ring_virt_addr;
		ring_dma_addr = phw->ring_dma_addr;
		ring_buf_len = phw->ring_buf_len;
#ifndef PRIV_STA_BUF
		alloc_dma_buf = phw->alloc_dma_buf;
#endif

		// save RF related settings before reset
		MIMO_TR_hw_support = phw->MIMO_TR_hw_support;
		NumTotalRFPath = phw->NumTotalRFPath;

		memset((void *)phw, 0, sizeof(struct rtl8192cd_hw));
		phw->ring_virt_addr = ring_virt_addr;
		phw->ring_buf_len = ring_buf_len;
#ifndef PRIV_STA_BUF
		phw->alloc_dma_buf = alloc_dma_buf;
#endif

#ifdef CONFIG_NET_PCI
		if (IS_PCIBIOS_TYPE)
			phw->ring_dma_addr = ring_dma_addr;
#endif
		phw->MIMO_TR_hw_support = MIMO_TR_hw_support;
		phw->NumTotalRFPath = NumTotalRFPath;

#if defined(DUALBAND_ONLY) && defined(CONFIG_RTL8190_PRIV_SKB)
		if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY) 
			split_pool(priv);
		else
			merge_pool(priv);
#endif
	}

#if defined(CLIENT_MODE) && defined(CHECK_HANGUP) && defined(RTK_BR_EXT)
	if (priv->reset_hangup &&
			(OPMODE & (WIFI_STATION_STATE | WIFI_ADHOC_STATE))) {
		backup_len = ((unsigned long)&((struct rtl8192cd_priv *)0)->br_ip) -
				 ((unsigned long)&((struct rtl8192cd_priv *)0)->join_res)+4;
		pbackup = kmalloc(backup_len, GFP_ATOMIC);
		if (pbackup)
			memcpy(pbackup, &priv->join_res, backup_len);
	}
#endif

	offset = (unsigned long)(&((struct rtl8192cd_priv *)0)->net_stats);
	// zero all data members below (including) stats
	memset((void *)((unsigned long)priv + offset), 0, sizeof(struct rtl8192cd_priv)-offset);

#ifdef CHECK_HANGUP
	if (!priv->reset_hangup)
#endif
	{
#ifdef SMART_REPEATER_MODE
		if (!priv->pshare->switch_chan_rp)
#endif
		{
		
			priv->up_time = 0;
		}
	}

#if defined(CLIENT_MODE) && defined(CHECK_HANGUP)
	if (priv->reset_hangup && pbackup) {
		memcpy(&priv->join_res, pbackup, backup_len);
		kfree(pbackup);
	}
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
		// zero all data members below (including) LED_Timer of share_info
		offset = (unsigned long)(&((struct priv_shared_info*)0)->LED_Timer);
		memset((void *)((unsigned long)priv->pshare+ offset), 0, sizeof(struct priv_shared_info)-offset);

#ifdef CONFIG_RTK_MESH
		memset((void *)&priv->pshare->meshare, 0, sizeof(struct MESH_Share));
		get_random_bytes((void *)&priv->pshare->meshare.seq, sizeof(priv->pshare->meshare.seq));
#if (MESH_DBG_LV & MESH_DBG_COMPLEX)
		init_timer(&priv->pshare->meshare.mesh_test_sme_timer);
		priv->pshare->meshare.mesh_test_sme_timer.data = (unsigned long) priv;
		priv->pshare->meshare.mesh_test_sme_timer.function = mesh_test_sme_timer;
		mod_timer(&priv->pshare->meshare.mesh_test_sme_timer, jiffies + RTL_SECONDS_TO_JIFFIES(2));
#endif // (MESH_DBG_LV & MESH_DBG_COMPLEX)

#if (MESH_DBG_LV & MESH_DBG_TEST)
		init_timer(&priv->pshare->meshare.mesh_test_sme_timer2);
		priv->pshare->meshare.mesh_test_sme_timer2.data = (unsigned long) priv;
		priv->pshare->meshare.mesh_test_sme_timer2.function = mesh_test_sme_timer2;
		mod_timer(&priv->pshare->meshare.mesh_test_sme_timer2, jiffies + RTL_SECONDS_TO_JIFFIES(50));
#endif // (MESH_DBG_LV & MESH_DBG_TEST)

#endif // CONFIG_RTK_MESH

		pwlan_hdr_poll = priv->pshare->pwlan_hdr_poll;
		pwlanllc_hdr_poll = priv->pshare->pwlanllc_hdr_poll;
		pwlanbuf_poll = priv->pshare->pwlanbuf_poll;
		pwlanicv_poll = priv->pshare->pwlanicv_poll;
		pwlanmic_poll = priv->pshare->pwlanmic_poll;

#ifdef _11s_TEST_MODE_
		pgalileo_poll = priv->pshare->galileo_poll;
		pgalileo_poll->count = AODV_RREQ_TABLE_SIZE;
#endif
		pwlan_hdr_poll->count = PRE_ALLOCATED_HDR;
		pwlanllc_hdr_poll->count = PRE_ALLOCATED_HDR;
		pwlanbuf_poll->count = PRE_ALLOCATED_MMPDU;
		pwlanicv_poll->count = PRE_ALLOCATED_HDR;
		pwlanmic_poll->count = PRE_ALLOCATED_HDR;

		// initialize all the hdr/buf node, and list to the poll_list
		INIT_LIST_HEAD(&priv->pshare->wlan_hdrlist);
		INIT_LIST_HEAD(&priv->pshare->wlanllc_hdrlist);
		INIT_LIST_HEAD(&priv->pshare->wlanbuf_list);
		INIT_LIST_HEAD(&priv->pshare->wlanicv_list);
		INIT_LIST_HEAD(&priv->pshare->wlanmic_list);

#ifdef _11s_TEST_MODE_	//Galileo

		memset(priv->rvTestPacket, 0, 3000);

		INIT_LIST_HEAD(&priv->pshare->galileo_list);
		INIT_LIST_HEAD(&priv->mtb_list);

		for(i=0; i< AODV_RREQ_TABLE_SIZE; i++)
		{
			INIT_LIST_HEAD(&(pgalileo_poll->node[i].list));
			list_add_tail(&(pgalileo_poll->node[i].list), &priv->pshare->galileo_list);
			init_timer(&pgalileo_poll->node[i].data.expire_timer);
			pgalileo_poll->node[i].data.priv = priv;
			pgalileo_poll->node[i].data.expire_timer.function = galileo_timer;
		}
#endif

		for(i=0; i< PRE_ALLOCATED_HDR; i++)
		{
			INIT_LIST_HEAD(&(pwlan_hdr_poll->hdrnode[i].list));
			list_add_tail(&(pwlan_hdr_poll->hdrnode[i].list), &priv->pshare->wlan_hdrlist);

			INIT_LIST_HEAD(&(pwlanllc_hdr_poll->hdrnode[i].list));
			list_add_tail( &(pwlanllc_hdr_poll->hdrnode[i].list), &priv->pshare->wlanllc_hdrlist);

			INIT_LIST_HEAD(&(pwlanicv_poll->hdrnode[i].list));
			list_add_tail( &(pwlanicv_poll->hdrnode[i].list), &priv->pshare->wlanicv_list);

			INIT_LIST_HEAD(&(pwlanmic_poll->hdrnode[i].list));
			list_add_tail( &(pwlanmic_poll->hdrnode[i].list), &priv->pshare->wlanmic_list);
		}

		for(i=0; i< PRE_ALLOCATED_MMPDU; i++)
		{
			INIT_LIST_HEAD(&(pwlanbuf_poll->hdrnode[i].list));
			list_add_tail( &(pwlanbuf_poll->hdrnode[i].list), &priv->pshare->wlanbuf_list);
		}

		DEBUG_INFO("hdrlist=%x, llc_hdrlist=%x, buf_list=%x, icv_list=%x, mic_list=%X\n",
			(UINT)&priv->pshare->wlan_hdrlist, (UINT)&priv->pshare->wlanllc_hdrlist, (UINT)&priv->pshare->wlanbuf_list,
			(UINT)&priv->pshare->wlanicv_list, (UINT)&priv->pshare->wlanmic_list);

		page_ptr = (unsigned char *)phw->ring_virt_addr;
		memset(page_ptr, 0, phw->ring_buf_len); // this is vital!

#ifndef __KERNEL__
		if ((unsigned int)page_ptr & (~(PAGE_SIZE - 1)))
			page_ptr = (unsigned char *)(((unsigned int)page_ptr & (~(PAGE_SIZE - 1))) + PAGE_SIZE);
#endif

#ifdef CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {
	        phw->ring_dma_addr   = 0;
	        phw->rx_ring_addr    = 0;
	        phw->tx_ring0_addr   = 0;
	        phw->tx_ring1_addr   = 0;
	        phw->tx_ring2_addr   = 0;
	        phw->tx_ring3_addr   = 0;
	        phw->tx_ring4_addr   = 0;
	        phw->tx_ring5_addr   = 0;
	        phw->tx_ringB_addr   = 0;
	        phw->rxcmd_ring_addr = 0;
	        phw->txcmd_ring_addr = 0;
        
	        memset(&phw->tx_desc0_dma_addr,   0, sizeof(unsigned long) * NUM_TX_DESC);
	        memset(&phw->tx_desc1_dma_addr,   0, sizeof(unsigned long) * NUM_TX_DESC);
	        memset(&phw->tx_desc2_dma_addr,   0, sizeof(unsigned long) * NUM_TX_DESC);
	        memset(&phw->tx_desc3_dma_addr,   0, sizeof(unsigned long) * NUM_TX_DESC);
	        memset(&phw->tx_desc4_dma_addr,   0, sizeof(unsigned long) * NUM_TX_DESC);
	        memset(&phw->tx_desc5_dma_addr,   0, sizeof(unsigned long) * NUM_TX_DESC);
	        memset(&phw->tx_descB_dma_addr,   0, sizeof(unsigned long) * NUM_TX_DESC);
	        memset(&phw->rx_descL_dma_addr,   0, sizeof(unsigned long) * NUM_RX_DESC);
	        memset(&phw->rxcmd_desc_dma_addr, 0, sizeof(unsigned long) * NUM_CMD_DESC);
	        memset(&phw->txcmd_desc_dma_addr, 0, sizeof(unsigned long) * NUM_CMD_DESC);        
            
	        phw->rx_descL   = NULL;
	        phw->tx_desc0   = NULL;
	        phw->tx_desc1   = NULL;
	        phw->tx_desc2   = NULL;
	        phw->tx_desc3   = NULL;
	        phw->tx_desc4   = NULL;
	        phw->tx_desc5   = NULL;
	        phw->tx_descB   = NULL;
	        phw->rxcmd_desc = NULL;
	        phw->txcmd_desc = NULL;
        
	        memset(&phw->rx_descL_dma_addr,   0, sizeof(unsigned long) * NUM_RX_DESC) ;
	        memset(&phw->rxcmd_desc_dma_addr, 0, sizeof(unsigned long) * NUM_CMD_DESC) ;
	        memset(&phw->txcmd_desc_dma_addr, 0, sizeof(unsigned long) * NUM_CMD_DESC) ;        
		} else
#endif
		{
#ifdef CONFIG_NET_PCI
		if (!IS_PCIBIOS_TYPE)
#endif
			phw->ring_dma_addr = virt_to_bus(page_ptr);

		phw->rx_ring_addr  = phw->ring_dma_addr;
		phw->tx_ring0_addr = phw->ring_dma_addr + NUM_RX_DESC * sizeof(struct rx_desc);
		phw->tx_ring1_addr = phw->tx_ring0_addr + NUM_TX_DESC * sizeof(struct tx_desc);
		phw->tx_ring2_addr = phw->tx_ring1_addr + NUM_TX_DESC * sizeof(struct tx_desc);
		phw->tx_ring3_addr = phw->tx_ring2_addr + NUM_TX_DESC * sizeof(struct tx_desc);
		phw->tx_ring4_addr = phw->tx_ring3_addr + NUM_TX_DESC * sizeof(struct tx_desc);
		phw->tx_ring5_addr = phw->tx_ring4_addr + NUM_TX_DESC * sizeof(struct tx_desc);
		phw->rxcmd_ring_addr = phw->tx_ring5_addr + NUM_TX_DESC * sizeof(struct tx_desc);
		phw->txcmd_ring_addr = phw->rxcmd_ring_addr + NUM_CMD_DESC * sizeof(struct rx_desc);
		phw->tx_ringB_addr = phw->txcmd_ring_addr + NUM_CMD_DESC * sizeof(struct tx_desc);

		phw->rx_descL = (struct rx_desc *)page_ptr;
		phw->tx_desc0 = (struct tx_desc *)(page_ptr + NUM_RX_DESC * sizeof(struct rx_desc));
		phw->tx_desc1 = (struct tx_desc *)((unsigned long)phw->tx_desc0 + NUM_TX_DESC * sizeof(struct tx_desc));
		phw->tx_desc2 = (struct tx_desc *)((unsigned long)phw->tx_desc1 + NUM_TX_DESC * sizeof(struct tx_desc));
		phw->tx_desc3 = (struct tx_desc *)((unsigned long)phw->tx_desc2 + NUM_TX_DESC * sizeof(struct tx_desc));
		phw->tx_desc4 = (struct tx_desc *)((unsigned long)phw->tx_desc3 + NUM_TX_DESC * sizeof(struct tx_desc));
		phw->tx_desc5 = (struct tx_desc *)((unsigned long)phw->tx_desc4 + NUM_TX_DESC * sizeof(struct tx_desc));
		phw->rxcmd_desc = (struct rx_desc *)((unsigned long)phw->tx_desc5 + NUM_TX_DESC * sizeof(struct tx_desc));
		phw->txcmd_desc = (struct tx_desc *)((unsigned long)phw->rxcmd_desc + NUM_CMD_DESC * sizeof(struct rx_desc));
		phw->tx_descB = (struct tx_desc *)((unsigned long)phw->txcmd_desc + NUM_CMD_DESC * sizeof(struct tx_desc));

		/* To set the DMA address for both RX/TX ring */
		{
#ifndef USE_RTL8186_SDK
			int txDescRingIdx;

			struct tx_desc *tx_desc_array[] = {
				phw->tx_desc0,
				phw->tx_desc1,
				phw->tx_desc2,
				phw->tx_desc3,
				phw->tx_desc4,
				phw->tx_desc5,
				phw->tx_descB,
				0
				};
#ifdef CONFIG_NET_PCI
			unsigned long *tx_desc_dma_array[] = {
				(unsigned long*)(phw->tx_desc0_dma_addr),
				(unsigned long*)(phw->tx_desc1_dma_addr),
				(unsigned long*)(phw->tx_desc2_dma_addr),
				(unsigned long*)(phw->tx_desc3_dma_addr),
				(unsigned long*)(phw->tx_desc4_dma_addr),
				(unsigned long*)(phw->tx_desc5_dma_addr),
				(unsigned long*)(phw->tx_descB_dma_addr),
				(unsigned long*)0
				};
#endif
#endif // !USE_RTL8186_SDK

			/* RX RING */
#if defined(NOT_RTK_BSP)
			for (i=0; i<NUM_RX_DESC; i++) {
				phw->rx_descL_dma_addr[i] = phw->rx_ring_addr + i*(sizeof(struct rx_desc));
			}
#else
			for (i=0; i<NUM_RX_DESC; i++) {
				phw->rx_descL_dma_addr[i] = get_physical_addr(priv, (void *)(&phw->rx_descL[i]),
					sizeof(struct rx_desc), PCI_DMA_TODEVICE);
			}
#endif

#ifndef USE_RTL8186_SDK

#if defined(NOT_RTK_BSP)
			tx_dma_start = phw->ring_dma_addr + NUM_RX_DESC * sizeof(struct rx_desc);
#endif

			/* TX RING */
			txDescRingIdx = 0;

			while (tx_desc_array[txDescRingIdx] != 0) {
#ifdef CONFIG_NET_PCI
				unsigned long *tx_desc_dma_ptr = tx_desc_dma_array[txDescRingIdx];
#endif
				tx_desc_ptr = tx_desc_array[txDescRingIdx];

#if defined(NOT_RTK_BSP)
				for (i=0; i<CURRENT_NUM_TX_DESC; i++) {
					tx_desc_dma_ptr[i]= tx_dma_start + txDescRingIdx*NUM_TX_DESC * sizeof(struct tx_desc) + i*sizeof(struct tx_desc);
				}
#else
				for (i=0; i<CURRENT_NUM_TX_DESC; i++) {
					tx_desc_dma_ptr[i] = get_physical_addr(priv, (void *)(&(tx_desc_ptr[i])),
						sizeof(struct tx_desc), PCI_DMA_TODEVICE);
				}
#endif

				txDescRingIdx ++;
			}
#endif	// !USE_RTL8186_SDK

#if defined(NOT_RTK_BSP)
			for (i=0; i<NUM_CMD_DESC; i++) {
				phw->rxcmd_desc_dma_addr[i] = phw->rxcmd_ring_addr + i*sizeof(struct rx_desc);
				phw->txcmd_desc_dma_addr[i] = phw->txcmd_ring_addr + i*sizeof(struct tx_desc);
			}

#else
			for (i=0; i<NUM_CMD_DESC; i++) {
				phw->rxcmd_desc_dma_addr[i] = get_physical_addr(priv, (void *)(&phw->rxcmd_desc[i]),
					sizeof(struct rx_desc), PCI_DMA_TODEVICE);
				phw->txcmd_desc_dma_addr[i] = get_physical_addr(priv, (void *)(&phw->txcmd_desc[i]),
					sizeof(struct tx_desc), PCI_DMA_TODEVICE);
			}
#endif
		}

		DEBUG_INFO("rx_descL=%08x tx_desc0=%08x, tx_desc1=%08x, tx_desc2=%08x, tx_desc3=%08x, tx_desc4=%08x, "
			"tx_desc5=%08x, rxcmd_desc=%08x, txcmd_desc=%08x, tx_descB=%08x\n",
			(UINT)phw->rx_descL, (UINT)phw->tx_desc0, (UINT)phw->tx_desc1, (UINT)phw->tx_desc2,
			(UINT)phw->tx_desc3, (UINT)phw->tx_desc4, (UINT)phw->tx_desc5,
			(UINT)phw->rxcmd_desc, (UINT)phw->txcmd_desc, (UINT)phw->tx_descB);
		}
#ifdef RTK_QUE
		rtk_queue_init(&priv->pshare->skb_queue);
#else
		skb_queue_head_init(&priv->pshare->skb_queue);
#endif

#ifdef CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {
	        // this three functions must be called in this seqence...it cannot be moved arbitrarily
	        GET_HAL_INTERFACE(priv)->InitHCIDMAMemHandler(priv);
	        // RX_BUF_LEN must include RX_DESC and Payload
	        GET_HAL_INTERFACE(priv)->PrepareRXBDHandler(priv, RX_BUF_LEN, init_rxdesc_88XX);
	        GET_HAL_INTERFACE(priv)->PrepareTXBDHandler(priv);

            //Filen, init SW TXDESC Type
            //To avoid recycle error
            rtl88xx_init_swtxdec(priv);

// TODO: check the following compile flag...ex: USE_TXQUEUE...  
		} else 
#endif
		{
		
		// Now for Rx desc...
		for(i=0; i<NUM_RX_DESC; i++)
		{
			pskb = rtl_dev_alloc_skb(priv, RX_BUF_LEN, _SKB_RX_, 1);
			if (pskb == NULL) {
				printk("can't allocate skbuff for RX, abort!\n");
				return 1;
			}
			init_rxdesc(pskb, i, priv);
		}

		// Nothing to do for Tx desc...
		for(i=0; i<CURRENT_NUM_TX_DESC; i++)
		{
			init_txdesc(priv, phw->tx_desc0, phw->tx_ring0_addr, i);
			init_txdesc(priv, phw->tx_desc1, phw->tx_ring1_addr, i);
			init_txdesc(priv, phw->tx_desc2, phw->tx_ring2_addr, i);
			init_txdesc(priv, phw->tx_desc3, phw->tx_ring3_addr, i);
			init_txdesc(priv, phw->tx_desc4, phw->tx_ring4_addr, i);
			init_txdesc(priv, phw->tx_desc5, phw->tx_ring5_addr, i);
		}

#ifdef MBSSID
		for(i=0; i<(RTL8192CD_NUM_VWLAN+1); i++) {
			if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
				if (i == RTL8192CD_NUM_VWLAN)
					(phw->tx_descB + i)->Dword10 = set_desc(phw->tx_ringB_addr);
					//(phw->tx_descB + i)->NextTxDescAddress = set_desc(phw->tx_ringB_addr);
				else
					(phw->tx_descB + i)->Dword10 = set_desc(phw->tx_ringB_addr + (i+1) * sizeof(struct tx_desc));
					//(phw->tx_descB + i)->NextTxDescAddress = set_desc(phw->tx_ringB_addr + (i+1) * sizeof(struct tx_desc));
			}
		}
#endif
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
		//Family add: must re-initialize use_txdesc_cnt after realloc txdesc
#ifdef RESERVE_TXDESC_FOR_EACH_IF
		for(i=0; i<=HIGH_QUEUE; ++i) {
			priv->use_txdesc_cnt[i] = 0;
		}

#ifdef USE_TXQUEUE
		for(i=0; i<=HIGH_QUEUE; i++) {
			priv->use_txq_cnt[i] = 0;
		}
#endif
		
#ifdef MBSSID
		if (GET_ROOT(priv)->pmib->miscEntry.vap_enable){
			int j;
			for (i=0; i<RTL8192CD_NUM_VWLAN; ++i){
				for (j=0; j<=HIGH_QUEUE; ++j){
					priv->pvap_priv[i]->use_txdesc_cnt[j] = 0;
				}
#ifdef USE_TXQUEUE
				for (j=0; j<=HIGH_QUEUE; j++) {
					priv->pvap_priv[i]->use_txq_cnt[j] = 0;
				}
#endif
			}
		}
#endif	// #ifdef MBSSID
#ifdef UNIVERSAL_REPEATER
		if (GET_VXD_PRIV(priv)) {
			for(i=0; i<=HIGH_QUEUE; ++i) {
				GET_VXD_PRIV(priv)->use_txdesc_cnt[i] = 0;
			}
#ifdef USE_TXQUEUE
			for(i=0; i<=HIGH_QUEUE; ++i) {
				GET_VXD_PRIV(priv)->use_txq_cnt[i] = 0;
			}
#endif
		}
#endif  // #ifdef UNIVERSAL_REPEATER
#endif	// #ifdef RESERVE_TXDESC_FOR_EACH_IF
#endif	// defined(UNIVERSAL_REPEATER) || defined(MBSSID)
		for(i=0; i<NUM_CMD_DESC; i++) {
			if (i == (NUM_CMD_DESC - 1))// set NextAddrs
				(phw->txcmd_desc + i)->Dword9 = set_desc(phw->txcmd_ring_addr);
			else
				(phw->txcmd_desc + i)->Dword9 = set_desc(phw->txcmd_ring_addr + (i+1) * sizeof(struct tx_desc));
		}
		}

		priv->pshare->amsdu_timer_head = priv->pshare->amsdu_timer_tail = 0;

#ifdef RX_BUFFER_GATHER
		INIT_LIST_HEAD(&priv->pshare->gather_list);
#endif	

#ifdef USE_TXQUEUE
		if (init_txq_pool(&priv->pshare->txq_pool, &priv->pshare->txq_pool_addr)) {
			printk("Can not init tx queue pool.\n");
			return 1;
		}
		for (i=0; i<7; i++)
			init_txq_head(&(priv->pshare->txq_list[i]));
		priv->pshare->txq_isr = 0;
		priv->pshare->txq_stop = 0;
		priv->pshare->txq_check = 0;
#endif	

		if (priv->pmib->dot11RFEntry.tx2path) {
			if (priv->pmib->dot11RFEntry.txbf) {
				priv->pmib->dot11RFEntry.tx2path = 0;
				DEBUG_INFO("Disable tx2path due to txbf\n");
			}
			if (priv->pmib->dot11nConfigEntry.dot11nSTBC) {
				priv->pmib->dot11RFEntry.tx2path = 0;
				DEBUG_INFO("Disable tx2path due to stbc\n");
			}
		}
		if (priv->pmib->dot11RFEntry.txbf) {
			if (priv->pmib->dot11nConfigEntry.dot11nSTBC) {
				priv->pmib->dot11nConfigEntry.dot11nSTBC = 0;
				DEBUG_INFO("Disable stbc due to txbf\n");
			}
		}
#ifdef TX_EARLY_MODE
		//if ((GET_CHIP_VER(priv) == VERSION_8192C || GET_CHIP_VER(priv) == VERSION_8188C)
		//		&& GET_TX_EARLY_MODE)
		if ((GET_CHIP_VER(priv) != VERSION_8188E) && GET_TX_EARLY_MODE)
			GET_TX_EARLY_MODE = 0;			
#endif
	}

	INIT_LIST_HEAD(&priv->wlan_acl_list);
	INIT_LIST_HEAD(&priv->wlan_aclpolllist);

	pwlan_acl_poll = priv->pwlan_acl_poll;
	for(i=0; i< NUM_ACL; i++)
	{
		INIT_LIST_HEAD(&(pwlan_acl_poll->aclnode[i].list));
		list_add_tail(&(pwlan_acl_poll->aclnode[i].list), &priv->wlan_aclpolllist);
	}

	// copy acl from mib to link list
	for (i=0; i<priv->pmib->dot11StationConfigEntry.dot11AclNum; i++)
	{
		struct list_head *pnewlist;
		struct wlan_acl_node *paclnode;

		pnewlist = priv->wlan_aclpolllist.next;
		list_del_init(pnewlist);

		paclnode = list_entry(pnewlist,	struct wlan_acl_node, list);
		memcpy((void *)paclnode->addr, priv->pmib->dot11StationConfigEntry.dot11AclAddr[i], 6);
		paclnode->mode = (unsigned char)priv->pmib->dot11StationConfigEntry.dot11AclMode;

		list_add_tail(pnewlist, &priv->wlan_acl_list);
	}

	for(i=0; i<NUM_STAT; i++)
		INIT_LIST_HEAD(&(priv->stat_hash[i]));

#ifdef	CONFIG_RTK_MESH
	/*
	 * CAUTION !! These statement meshX(virtual interface) ONLY, Maybe modify....
	*/
#ifdef	_MESH_ACL_ENABLE_	// copy acl from mib to link list (below code copy above ACL code)
	INIT_LIST_HEAD(&priv->mesh_acl_list);
	INIT_LIST_HEAD(&priv->mesh_aclpolllist);

	pmesh_acl_poll = priv->pmesh_acl_poll;
	for(i=0; i< NUM_MESH_ACL; i++)
	{
		INIT_LIST_HEAD(&(pmesh_acl_poll->meshaclnode[i].list));
		list_add_tail(&(pmesh_acl_poll->meshaclnode[i].list), &priv->mesh_aclpolllist);
	}

	for (i=0; i<priv->pmib->dot1180211sInfo.mesh_acl_num; i++)
	{
		struct list_head *pnewlist;
		struct wlan_acl_node *paclnode;

		pnewlist = priv->mesh_aclpolllist.next;
		list_del_init(pnewlist);

		paclnode = list_entry(pnewlist, struct wlan_acl_node, list);
		memcpy((void *)paclnode->addr, priv->pmib->dot1180211sInfo.mesh_acl_addr[i], MACADDRLEN);
		paclnode->mode = (unsigned char)priv->pmib->dot1180211sInfo.mesh_acl_mode;

		list_add_tail(pnewlist, &priv->mesh_acl_list);
	}
#endif

#ifdef MESH_BOOTSEQ_AUTH
	INIT_LIST_HEAD(&(priv->mesh_auth_hdr));
#endif

	INIT_LIST_HEAD(&(priv->mesh_unEstablish_hdr));
	INIT_LIST_HEAD(&(priv->mesh_mp_hdr));

	priv->mesh_profile[0].used = FALSE; // Configure by WEB in the future, Maybe delete, Preservation before delete
#endif

	INIT_LIST_HEAD(&(priv->asoc_list));
	INIT_LIST_HEAD(&(priv->auth_list));
	INIT_LIST_HEAD(&(priv->sleep_list));
	INIT_LIST_HEAD(&(priv->defrag_list));
	INIT_LIST_HEAD(&(priv->wakeup_list));
	INIT_LIST_HEAD(&(priv->rx_datalist));
	INIT_LIST_HEAD(&(priv->rx_mgtlist));
	INIT_LIST_HEAD(&(priv->rx_ctrllist));
	INIT_LIST_HEAD(&(priv->addRAtid_list));	// to avoid add RAtid fail
	INIT_LIST_HEAD(&(priv->addrssi_list));

#ifdef SMP_SYNC
	spin_lock_init(&(priv->rx_datalist_lock));
	spin_lock_init(&(priv->rx_mgtlist_lock));
	spin_lock_init(&(priv->rx_ctrllist_lock));
#endif
#ifdef A4_STA
	INIT_LIST_HEAD(&(priv->a4_sta_list));
#endif

#ifdef CHECK_BEACON_HANGUP
	if (priv->reset_hangup)
	   	priv->pshare->beacon_wait_cnt = 1;
	else
	   	priv->pshare->beacon_wait_cnt = 2;
#endif

#ifdef CHECK_HANGUP
	if (priv->reset_hangup) {
		get_available_channel(priv);
		validate_oper_rate(priv);
		get_oper_rate(priv);
		DOT11_InitQueue(priv->pevent_queue);
		return 0;
	}
#endif

	// construct operation and basic rates set
	{
		// validate region domain
		if ((priv->pmib->dot11StationConfigEntry.dot11RegDomain < DOMAIN_FCC) ||
				(priv->pmib->dot11StationConfigEntry.dot11RegDomain >= DOMAIN_MAX)) {
			PRINT_INFO("invalid region domain, use default value [DOMAIN_FCC]!\n");
			priv->pmib->dot11StationConfigEntry.dot11RegDomain = DOMAIN_FCC;
		}

		// validate band
		if (priv->pmib->dot11BssType.net_work_type == 0) {
			PRINT_INFO("operation band is not set, use G+B as default!\n");
			priv->pmib->dot11BssType.net_work_type = WIRELESS_11B | WIRELESS_11G;
		}
		if ((OPMODE & WIFI_AP_STATE) && (priv->pmib->dot11BssType.net_work_type & (WIRELESS_11B | WIRELESS_11G))) {
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) {
				priv->pmib->dot11BssType.net_work_type &= (WIRELESS_11B | WIRELESS_11G);
				PRINT_INFO("operation band not appropriate, use G/B as default!\n");
			}
		}

		if (should_forbid_Nmode(priv)) {
#ifdef SUPPORT_MULTI_PROFILE
			if (!((OPMODE & WIFI_STATION_STATE) && 
				priv->pmib->ap_profile.enable_profile && priv->pmib->ap_profile.profile_num > 0))
#endif						
				priv->pmib->dot11BssType.net_work_type &= ~(WIRELESS_11N|WIRELESS_11AC);
		}

		// validate channel number
		if (get_available_channel(priv) == FAIL) {
			PRINT_INFO("can't get operation channels, abort!\n");
			return 1;
		}
		if (priv->pmib->dot11RFEntry.dot11channel != 0) {
			for (i=0; i<priv->available_chnl_num; i++)
				if (priv->pmib->dot11RFEntry.dot11channel == priv->available_chnl[i])
					break;
			if (i == priv->available_chnl_num) {
				priv->pmib->dot11RFEntry.dot11channel = priv->available_chnl[0];

				PRINT_INFO("invalid channel number, use default value [%d]!\n",
					priv->pmib->dot11RFEntry.dot11channel);
			}
			priv->auto_channel = 0;
			priv->auto_channel_backup = priv->auto_channel;
#ifdef SIMPLE_CH_UNI_PROTOCOL
			SET_PSEUDO_RANDOM_NUMBER(priv->mesh_ChannelPrecedence);
#endif
		}
		else {
#ifdef SIMPLE_CH_UNI_PROTOCOL
			if(GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
				priv->auto_channel = 16;
				priv->auto_channel_backup = priv->auto_channel;
			}
			else
#endif
			{
#ifdef DFS
				if ((OPMODE & WIFI_AP_STATE) && !priv->pmib->dot11DFSEntry.disable_DFS && (priv->pshare->dfsSwitchChannel != 0)) {
					priv->pmib->dot11RFEntry.dot11channel = priv->pshare->dfsSwitchChannel;
					priv->pshare->dfsSwitchChannel = 0;
					priv->auto_channel = 0;
					priv->auto_channel_backup = 1;
				}
				else
#endif
				{
					if (OPMODE & WIFI_AP_STATE)
						priv->auto_channel = 1;
					else
						priv->auto_channel = 2;
					priv->pmib->dot11RFEntry.dot11channel = priv->available_chnl[0];
					priv->auto_channel_backup = priv->auto_channel;
				}
			}
		}
		//priv->auto_channel_backup = priv->auto_channel;

		if (priv->pmib->dot11RFEntry.dot11channel <= 14)
			priv->pmib->dot11RFEntry.phyBandSelect = PHY_BAND_2G;
		else
			priv->pmib->dot11RFEntry.phyBandSelect = PHY_BAND_5G;		

		// validate hi and low channel
		if (priv->pmib->dot11RFEntry.dot11ch_low != 0) {
			for (i=0; i<priv->available_chnl_num; i++)
				if (priv->pmib->dot11RFEntry.dot11ch_low == priv->available_chnl[i])
					break;
			if (i == priv->available_chnl_num) {
				priv->pmib->dot11RFEntry.dot11ch_low = priv->available_chnl[0];

				PRINT_INFO("invalid low channel number, use default value [%d]!\n",
					priv->pmib->dot11RFEntry.dot11ch_low);
			}
		}
		if (priv->pmib->dot11RFEntry.dot11ch_hi != 0) {
			for (i=0; i<priv->available_chnl_num; i++)
				if (priv->pmib->dot11RFEntry.dot11ch_hi == priv->available_chnl[i])
					break;
			if (i == priv->available_chnl_num) {
				priv->pmib->dot11RFEntry.dot11ch_hi = priv->available_chnl[priv->available_chnl_num-1];

				PRINT_INFO("invalid hi channel number, use default value [%d]!\n",
					priv->pmib->dot11RFEntry.dot11ch_hi);
			}
		}

// Mark the code to auto disable N mode in WEP encrypt
#if 0
		if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) &&
				(priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_ ||
					priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_))
			priv->pmib->dot11BssType.net_work_type &= ~WIRELESS_11N;
#endif
//------------------------------ david+2008-01-11

		// support cck only in channel 14
		if ((priv->pmib->dot11RFEntry.disable_ch14_ofdm) &&
			(priv->pmib->dot11RFEntry.dot11channel == 14)) {
			priv->pmib->dot11BssType.net_work_type = WIRELESS_11B;
			PRINT_INFO("support cck only in channel 14!\n");
		}

		// validate and get support and basic rates
		validate_oper_rate(priv);
		get_oper_rate(priv);

		if ((priv->pmib->dot11nConfigEntry.dot11nUse40M==2) &&
			(!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC))) {
			PRINT_INFO("enable 80M but not in AC mode! back to 40M\n");
			priv->pmib->dot11nConfigEntry.dot11nUse40M = 1;
		}

		if (priv->pmib->dot11nConfigEntry.dot11nUse40M &&
			(!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11N))) {
			PRINT_INFO("enable 40M but not in N mode! back to 20M\n");
			priv->pmib->dot11nConfigEntry.dot11nUse40M = 0;
		}

		// check deny band
		if ((priv->pmib->dot11BssType.net_work_type & (~priv->pmib->dot11StationConfigEntry.legacySTADeny)) == 0) {
			PRINT_INFO("legacySTADeny %d not suitable! set to 0\n", priv->pmib->dot11StationConfigEntry.legacySTADeny);
			priv->pmib->dot11StationConfigEntry.legacySTADeny = 0;
		}
	}

	if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)) {
		if (AMSDU_ENABLE)
			AMSDU_ENABLE = 0;
		if (AMPDU_ENABLE)
			AMPDU_ENABLE = 0;
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
		if (IS_ROOT_INTERFACE(priv))
#endif
		{
			priv->pshare->is_40m_bw = 0;
			priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_DONTCARE;
		}
	}
	else {
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
		if (IS_ROOT_INTERFACE(priv))
#endif
		{
			priv->pshare->is_40m_bw = priv->pmib->dot11nConfigEntry.dot11nUse40M;
			if (priv->pshare->is_40m_bw == 0)
				priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_DONTCARE;
			else {
#if defined(RTK_5G_SUPPORT) 
				if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
					priv->pshare->offset_2nd_chan = priv->pmib->dot11nConfigEntry.dot11n2ndChOffset;
				} else
#endif
				{
					if ((priv->pmib->dot11RFEntry.dot11channel < 5) &&
							(priv->pmib->dot11nConfigEntry.dot11n2ndChOffset == HT_2NDCH_OFFSET_BELOW))
						priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_ABOVE;
					else if ((priv->pmib->dot11RFEntry.dot11channel > 9) &&
							(priv->pmib->dot11nConfigEntry.dot11n2ndChOffset == HT_2NDCH_OFFSET_ABOVE))
						priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_BELOW;
					else
						priv->pshare->offset_2nd_chan = priv->pmib->dot11nConfigEntry.dot11n2ndChOffset;
				}
			}
		}

		// force wmm enabled if n mode
		// so hostapd should always set wmm_enabled=1 if n mode.
		QOS_ENABLE = 1;
	}

	// set wep key length
	if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_)
		priv->pmib->dot1180211AuthEntry.dot11PrivacyKeyLen = 8;
	else if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_)
		priv->pmib->dot1180211AuthEntry.dot11PrivacyKeyLen = 16;

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
		init_crc32_table();	// for sw encryption
	}

#ifdef WIFI_WMM
	if (QOS_ENABLE) {
		if ((OPMODE & WIFI_AP_STATE)
#ifdef CLIENT_MODE
			|| (OPMODE & WIFI_ADHOC_STATE)
#endif
			) {
			GET_EDCA_PARA_UPDATE = 0;
/*			
			//BK
			GET_STA_AC_BK_PARA.AIFSN = 7;
			GET_STA_AC_BK_PARA.TXOPlimit = 0;
			GET_STA_AC_BK_PARA.ACM = 0;
			GET_STA_AC_BK_PARA.ECWmin = 4;
			GET_STA_AC_BK_PARA.ECWmax = 10;
			//BE
			GET_STA_AC_BE_PARA.AIFSN = 3;
			GET_STA_AC_BE_PARA.TXOPlimit = 0;
			GET_STA_AC_BE_PARA.ACM = 0;
			GET_STA_AC_BE_PARA.ECWmin = 4;
			GET_STA_AC_BE_PARA.ECWmax = 10;
			//VI
			GET_STA_AC_VI_PARA.AIFSN = 2;
			if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) ||
				(priv->pmib->dot11BssType.net_work_type & WIRELESS_11A))
				GET_STA_AC_VI_PARA.TXOPlimit = 94; // 3.008ms
			else
				GET_STA_AC_VI_PARA.TXOPlimit = 188; // 6.016ms
			GET_STA_AC_VI_PARA.ACM = 0;
			GET_STA_AC_VI_PARA.ECWmin = 3;
			GET_STA_AC_VI_PARA.ECWmax = 4;
			//VO
			GET_STA_AC_VO_PARA.AIFSN = 2;
			if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) ||
				(priv->pmib->dot11BssType.net_work_type & WIRELESS_11A))
				GET_STA_AC_VO_PARA.TXOPlimit = 47; // 1.504ms
			else
				GET_STA_AC_VO_PARA.TXOPlimit = 102; // 3.264ms
			GET_STA_AC_VO_PARA.ACM = 0;
			GET_STA_AC_VO_PARA.ECWmin = 2;
			GET_STA_AC_VO_PARA.ECWmax = 3;
*/
			default_WMM_para(priv);

			//init WMM Para ie in beacon
			init_WMM_Para_Element(priv, priv->pmib->dot11QosEntry.WMM_PARA_IE);
		}
#ifdef CLIENT_MODE
		else if (OPMODE & WIFI_STATION_STATE) {
			init_WMM_Para_Element(priv, priv->pmib->dot11QosEntry.WMM_IE);  //  WMM STA
		}
#endif

		if (AMSDU_ENABLE || AMPDU_ENABLE) {
			if (priv->pmib->dot11nConfigEntry.dot11nTxNoAck) {
				priv->pmib->dot11nConfigEntry.dot11nTxNoAck = 0;
				PRINT_INFO("Tx No Ack is off because aggregation is enabled.\n");
			}
		}
	}
#endif

	i = priv->pmib->dot11ErpInfo.ctsToSelf;
	memset(&priv->pmib->dot11ErpInfo, '\0', sizeof(struct erp_mib)); // reset ERP mib
	priv->pmib->dot11ErpInfo.ctsToSelf = i;

	if ( (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) ||
			(priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) )
		priv->pmib->dot11ErpInfo.shortSlot = 1;
	else
		priv->pmib->dot11ErpInfo.shortSlot = 0;

	if (OPMODE & WIFI_AP_STATE) {
		memcpy(priv->pmib->dot11StationConfigEntry.dot11Bssid,
				priv->pmib->dot11OperationEntry.hwaddr, 6);
		//priv->oper_band = priv->pmib->dot11BssType.net_work_type;
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
		if (IS_ROOT_INTERFACE(priv))
#endif
		{
			if (!priv->pshare->ra40MLowerMinus && (priv->pshare->rf_ft_var.raGoUp40MLower > 5) &&
				(priv->pshare->rf_ft_var.raGoDown40MLower > 5)) {
				priv->pshare->rf_ft_var.raGoUp40MLower-=5;
				priv->pshare->rf_ft_var.raGoDown40MLower-=5;
				priv->pshare->ra40MLowerMinus++;
			}
#ifdef HIGH_POWER_EXT_PA
			if (!priv->pshare->raThdHP_Minus) {

				if( priv->pshare->rf_ft_var.use_ext_pa )  {
					if(priv->pshare->rf_ft_var.raGoDownUpper > RSSI_DIFF_PA) {
						priv->pshare->rf_ft_var.raGoDownUpper -= RSSI_DIFF_PA;
						priv->pshare->rf_ft_var.raGoUpUpper -= RSSI_DIFF_PA;
					}
					if(priv->pshare->rf_ft_var.raGoDown20MLower > RSSI_DIFF_PA) {					
						priv->pshare->rf_ft_var.raGoDown20MLower -= RSSI_DIFF_PA;
						priv->pshare->rf_ft_var.raGoUp20MLower -= RSSI_DIFF_PA;
					}
					if(priv->pshare->rf_ft_var.raGoDown40MLower > RSSI_DIFF_PA) {	
						priv->pshare->rf_ft_var.raGoDown40MLower -= RSSI_DIFF_PA;
						priv->pshare->rf_ft_var.raGoUp40MLower -= RSSI_DIFF_PA;		
					}
				}
				++priv->pshare->raThdHP_Minus;
			}
#endif
		}
	}
#ifdef CLIENT_MODE
	else {
		if (priv->pmib->dot11StationConfigEntry.dot11DefaultSSIDLen == 0) {
			priv->pmib->dot11StationConfigEntry.dot11DefaultSSIDLen = 11;
			memcpy(priv->pmib->dot11StationConfigEntry.dot11DefaultSSID, "defaultSSID", 11);
		}
		memset(priv->pmib->dot11StationConfigEntry.dot11Bssid, 0, 6);
		priv->join_res = STATE_Sta_No_Bss;

// Add mac clone address manually ----------
#ifdef RTK_BR_EXT
		if (priv->pmib->ethBrExtInfo.macclone_enable == 2) {
			extern void mac_clone(struct rtl8192cd_priv *priv, unsigned char *addr);
			mac_clone(priv, priv->pmib->ethBrExtInfo.nat25_dmzMac);
			priv->macclone_completed = 1;
		}
#endif
//------------------------- david+2007-5-31

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
		if (IS_ROOT_INTERFACE(priv))
#endif
		{
			if (priv->pshare->ra40MLowerMinus) {
				priv->pshare->rf_ft_var.raGoUp40MLower+=5;
				priv->pshare->rf_ft_var.raGoDown40MLower+=5;
				priv->pshare->ra40MLowerMinus = 0;
			}
		}
	}
#endif

	// initialize event queue
	DOT11_InitQueue(priv->pevent_queue);
#ifdef CONFIG_RTL_WAPI_SUPPORT
	DOT11_InitQueue(priv->wapiEvent_queue);
#ifdef MBSSID
  if (IS_ROOT_INTERFACE(priv)&&priv->pmib->miscEntry.vap_enable)  {
		for (i=0; i<RTL8192CD_NUM_VWLAN; i++)
			  DOT11_InitQueue(priv->pvap_priv[i]->wapiEvent_queue);
		  }
#endif

#endif

#ifdef CONFIG_RTK_MESH
	if(GET_MIB(priv)->dot1180211sInfo.mesh_enable == 1)	// plus add 0217, not mesh mode should not do below function
	{
	DOT11_InitQueue2(priv->pathsel_queue, MAXQUEUESIZE2, MAXDATALEN2);
#ifdef	_11s_TEST_MODE_
	DOT11_InitQueue2(priv->receiver_queue, MAXQUEUESIZE2, MAXDATALEN2);
#endif
		//modify by Joule for SECURITY
		i = priv->pmib->dot11sKeysTable.dot11Privacy;
		memset(&priv->pmib->dot11sKeysTable, '\0', sizeof(struct Dot11KeyMappingsEntry)); // reset key
		priv->pmib->dot11sKeysTable.dot11Privacy = i;
	 }	//
#endif

#ifdef __DRAYTEK_OS__
	if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _TKIP_PRIVACY_ &&
		priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _CCMP_PRIVACY_ &&
		priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _WEP_WPA_MIXED_PRIVACY_) {
#ifdef UNIVERSAL_REPEATER
		if (IS_ROOT_INTERFACE(priv))
#endif
		{
			priv->pmib->dot11RsnIE.rsnielen = 0;	// reset RSN IE length
			memset(&priv->pmib->dot11GroupKeysTable, '\0', sizeof(struct Dot11KeyMappingsEntry)); // reset group key
#ifdef UNIVERSAL_REPEATER
			if (GET_VXD_PRIV(priv))
				GET_VXD_PRIV(priv)->pmib->dot11RsnIE.rsnielen = 0;
#endif
		}
	}
#endif

	i = RC_ENTRY_NUM;
	for (;;) {
		if (priv->pmib->reorderCtrlEntry.ReorderCtrlWinSz >= i) {
			priv->pmib->reorderCtrlEntry.ReorderCtrlWinSz = i;
			break;
		}
		else if (i > 8)
			i = i / 2;
		else {
			priv->pmib->reorderCtrlEntry.ReorderCtrlWinSz = 8;
			break;
		}
	}

	// Realtek proprietary IE
	memcpy(&(priv->pshare->rtk_ie_buf[0]), Realtek_OUI, 3);
	priv->pshare->rtk_ie_buf[3] = 2;
	priv->pshare->rtk_ie_buf[4] = 1;
	priv->pshare->rtk_ie_buf[5] = 0;
	priv->pshare->rtk_ie_buf[5] |= RTK_CAP_IE_WLAN_88C92C | RTK_CAP_IE_WLAN_8192SE;
#ifdef CLIENT_MODE
	if (OPMODE & WIFI_STATION_STATE)
		priv->pshare->rtk_ie_buf[5] |= RTK_CAP_IE_AP_CLIENT;
#endif
	priv->pshare->rtk_ie_len = 6;


#ifdef WIFI_HAPD
	if ((priv->pmib->dot1180211AuthEntry.dot11EnablePSK == 0)
		&& (priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm)
		&& (priv->pmib->dot1180211AuthEntry.dot11WPACipher || priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher))
			rsn_init(priv);
#endif

#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
	if (priv->pmib->dot1180211AuthEntry.dot11EnablePSK)
		psk_init(priv);

#ifdef WDS
#ifdef UNIVERSAL_REPEATER
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#ifdef LAZY_WDS
		if (priv->pmib->dot11WdsInfo.wdsEnabled == WDS_LAZY_ENABLE) {
			priv->pmib->dot11WdsInfo.wdsNum = 0;
			memset(priv->pmib->dot11WdsInfo.entry, '\0', sizeof(struct wdsEntry)*NUM_WDS);
		}
#endif	
		if ((priv->pmib->dot11WdsInfo.wdsEnabled) && (priv->pmib->dot11WdsInfo.wdsNum > 0) &&
			((priv->pmib->dot11WdsInfo.wdsPrivacy == _TKIP_PRIVACY_) ||
			 (priv->pmib->dot11WdsInfo.wdsPrivacy == _CCMP_PRIVACY_)))
			wds_psk_init(priv);
	}
#endif
#endif

#ifdef WIFI_WPAS

	{
		WPAS_ASSOCIATION_INFO Assoc_Info;
		memset((void *)&Assoc_Info, 0, sizeof(struct _WPAS_ASSOCIATION_INFO));
		Assoc_Info.ReqIELen = priv->pmib->dot11RsnIE.rsnie[1]+ 2;
		memcpy(Assoc_Info.ReqIE, priv->pmib->dot11RsnIE.rsnie, Assoc_Info.ReqIELen);

		//event_indicate_wpas(priv, NULL, WPAS_ASSOC_INFO, (UINT8 *)&Assoc_Info);
	}

#endif 


#ifdef GBWC
	priv->GBWC_tx_queue.head = 0;
	priv->GBWC_tx_queue.tail = 0;
	priv->GBWC_rx_queue.head = 0;
	priv->GBWC_rx_queue.tail = 0;
	priv->GBWC_tx_count = 0;
	priv->GBWC_rx_count = 0;
	priv->GBWC_consuming_Q = 0;
#endif

	priv->release_mcast = 0;

#ifdef USB_PKT_RATE_CTRL_SUPPORT //mark_test
	priv->change_toggle = 0;
	priv->pre_pkt_cnt = 0;
	priv->pkt_nsec_diff = 0;
	priv->poll_usb_cnt = 0;
	priv->auto_rate_mask = 0;
#endif

#ifdef STA_EXT
#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E)
		priv->pshare->fw_free_space = RTL8188E_NUM_STAT - 2;
	else
#endif
		priv->pshare->fw_free_space = FW_NUM_STAT - 2; // One for MAGANEMENT_AID, one for other STAs
#endif

#ifdef CONFIG_RTK_VLAN_SUPPORT
	if (priv->pmib->vlan.global_vlan)
		priv->pmib->dot11OperationEntry.disable_brsc = 1;
#endif


#ifdef CONFIG_RTL_WAPI_SUPPORT
	if (priv->pmib->wapiInfo.wapiType!=wapiDisable)
	{
		/*	set NMK	*/
		GenerateRandomData(priv->wapiNMK, WAPI_KEY_LEN);
		priv->wapiMCastKeyId = 0;
		priv->wapiMCastKeyUpdate = 0;
		priv->wapiWaiTxSeq = 0;
		wapiInit(priv);
	}

#ifdef MBSSID
  if (IS_ROOT_INTERFACE(priv)&&priv->pmib->miscEntry.vap_enable)  {
	  for (i=0; i<RTL8192CD_NUM_VWLAN; i++)
			if (priv->pvap_priv[i]->pmib->wapiInfo.wapiType!=wapiDisable) {
			  /*  set NMK */
			  GenerateRandomData(priv->pvap_priv[i]->wapiNMK, WAPI_KEY_LEN);
			  priv->pvap_priv[i]->wapiMCastKeyId = 0;
			  priv->pvap_priv[i]->wapiMCastKeyUpdate = 0;
			  priv->wapiWaiTxSeq = 0;
			  wapiInit(priv->pvap_priv[i]);
		  }
  }
#endif
#endif

#ifdef MBSSID
	// if vap enabled, set beacon int to 100 at minimun when guest ssid num <= 4
    // if vap enabled, set beacon int to 200 at minimun when guest ssid num > 4
#ifdef  CONFIG_WLAN_HAL
	if (!IS_HAL_CHIP(priv))		// close by Eric, check why use this config 
#endif	
	{
		int ssid_num = 1, minbcn_period;
		priv->bcn_period_bak = priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod;

		if ((OPMODE & WIFI_AP_STATE) && GET_ROOT(priv)->pmib->miscEntry.vap_enable)
		{
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++)
	        {
		        if (GET_ROOT(priv)->pvap_priv[i] && IS_DRV_OPEN(GET_ROOT(priv)->pvap_priv[i]))
			    {
				    ssid_num++;
	            }
		    }
	    }
		
		if (ssid_num >= 5)
			minbcn_period = 200;
		else
			minbcn_period = 100;

		// if vap enabled, set beacon int to 100 at minimun
		if ((OPMODE & WIFI_AP_STATE) && GET_ROOT(priv)->pmib->miscEntry.vap_enable
			&& priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod < minbcn_period)
			priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod = minbcn_period;

		if ((OPMODE & WIFI_AP_STATE) && GET_ROOT(priv)->pmib->miscEntry.vap_enable)
        {
            for (i=0; i<RTL8192CD_NUM_VWLAN; i++)
            {
                if (GET_ROOT(priv)->pvap_priv[i])
                {
                        GET_ROOT(priv)->pvap_priv[i]->pmib->dot11StationConfigEntry.dot11BeaconPeriod = priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod;
						GET_ROOT(priv)->pvap_priv[i]->update_bcn_period = 1;
                }
            }

            GET_ROOT(priv)->pmib->dot11StationConfigEntry.dot11BeaconPeriod = priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod;
			GET_ROOT(priv)->update_bcn_period = 1;

        }
	}
#endif

#ifdef DOT11D
{
	extern COUNTRY_IE_ELEMENT countryIEArray[];
	int found = 0;
	char *CStringPtr =	priv->pmib->dot11dCountry.dot11CountryString ;

	if ((OPMODE & WIFI_AP_STATE) && COUNTRY_CODE_ENABLED) {
		for (i=0; i<COUNTRYNUMBER; i++) {
			if (!memcmp(CStringPtr, countryIEArray[i].countryA2, 2)) {
				priv->pshare->countryTabIdx = i;
				found = 1;
				break;
			}
		}

		if (found == 0) {
			priv->pshare->countryTabIdx = 1;
			printk("can't found country code(%s),use default region\n",CStringPtr);
		}

		i = priv->pshare->countryTabIdx;

		if (countryIEArray[i].A_Band_Region == 0) {
			priv->pshare->countryBandUsed=0; /*2.4G*/
		} else {
			/*5G*/
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) {
				priv->pshare->countryBandUsed = 1;	/*5G*/
				//printk("2.4G and 5G both enabled, select 5G \n");
				} else {
				priv->pshare->countryBandUsed = 0;	/*2.4G*/
				//printk("2.4G and 5G both enabled, select 2.4G \n");
			}
		}
	}
}
#endif

#ifdef RTL_MANUAL_EDCA
	for (i=0; i<8; i++) {
		if ((priv->pmib->dot11QosEntry.TID_mapping[i] < 1) || (priv->pmib->dot11QosEntry.TID_mapping[i] > 4))
			priv->pmib->dot11QosEntry.TID_mapping[i] = 2;
	}
#endif

#if defined(TXREPORT) && (defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT))
	if (
#ifdef CONFIG_RTL_92C_SUPPORT
		(GET_CHIP_VER(priv)==VERSION_8192C) || (GET_CHIP_VER(priv)==VERSION_8188C)
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
#ifdef CONFIG_RTL_92C_SUPPORT
		|| 
#endif
		(GET_CHIP_VER(priv)==VERSION_8192D)
#endif
		) {
		priv->pshare->sta_query_idx=-1;

		// Init StaDetectInfo to detect disappearing STA. Added by Annie, 2010-08-10.
		priv->pmib->staDetectInfo.txRprDetectPeriod = 1;
	}
#endif

	priv->pshare->sta_query_idx=-1;

#ifdef	INCLUDE_WPS
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
		if (IS_ROOT_INTERFACE(priv))
#endif
			wps_init(priv);
#endif
#ifdef P2P_SUPPORT
	if(OPMODE & WIFI_P2P_SUPPORT ){
		p2p_init(priv);
		P2P_DEBUG("%s under P2P mode now\n",priv->dev->name);		
	}
#endif

#ifdef CONFIG_RTK_MESH
		/*
		 * CAUTION !! These statement meshX(virtual interface) ONLY, Maybe modify....
		 * These statment is initial information, (If "ZERO" no need set it, because all cleared to ZERO)
		 */
		if(init_mesh(priv) <0)
			return 1;
#endif

#ifdef TLN_STATS
	if (priv->pshare->rf_ft_var.stats_time_interval)
		priv->stats_time_countdown = priv->pshare->rf_ft_var.stats_time_interval;
#endif
	
#if defined (SUPPORT_TX_MCAST2UNI)
		/*ipv4 mdns*/
		priv->pshare->rf_ft_var.mc2u_flood_mac[0].macAddr[0]=0x01;
		priv->pshare->rf_ft_var.mc2u_flood_mac[0].macAddr[1]=0x00;
		priv->pshare->rf_ft_var.mc2u_flood_mac[0].macAddr[2]=0x5e;
		priv->pshare->rf_ft_var.mc2u_flood_mac[0].macAddr[3]=0x00;
		priv->pshare->rf_ft_var.mc2u_flood_mac[0].macAddr[4]=0x00;
		priv->pshare->rf_ft_var.mc2u_flood_mac[0].macAddr[5]=0xfb;
	
		/*ipv4 upnp&m-search*/
		priv->pshare->rf_ft_var.mc2u_flood_mac[1].macAddr[0]=0x01;
		priv->pshare->rf_ft_var.mc2u_flood_mac[1].macAddr[1]=0x00;
		priv->pshare->rf_ft_var.mc2u_flood_mac[1].macAddr[2]=0x5e;
		priv->pshare->rf_ft_var.mc2u_flood_mac[1].macAddr[3]=0x7f;
		priv->pshare->rf_ft_var.mc2u_flood_mac[1].macAddr[4]=0xff;
		priv->pshare->rf_ft_var.mc2u_flood_mac[1].macAddr[5]=0xfa;
	
		/*ipv6 mdns*/
		priv->pshare->rf_ft_var.mc2u_flood_mac[2].macAddr[0]=0x33;
		priv->pshare->rf_ft_var.mc2u_flood_mac[2].macAddr[1]=0x33;
		priv->pshare->rf_ft_var.mc2u_flood_mac[2].macAddr[2]=0x00;
		priv->pshare->rf_ft_var.mc2u_flood_mac[2].macAddr[3]=0x00;
		priv->pshare->rf_ft_var.mc2u_flood_mac[2].macAddr[4]=0x00;
		priv->pshare->rf_ft_var.mc2u_flood_mac[2].macAddr[5]=0xfb;
	
		/*ipv6 upnp&m-search*/
		priv->pshare->rf_ft_var.mc2u_flood_mac[3].macAddr[0]=0x33;
		priv->pshare->rf_ft_var.mc2u_flood_mac[3].macAddr[1]=0x33;
		priv->pshare->rf_ft_var.mc2u_flood_mac[3].macAddr[2]=0x7f;
		priv->pshare->rf_ft_var.mc2u_flood_mac[3].macAddr[3]=0xff;
		priv->pshare->rf_ft_var.mc2u_flood_mac[3].macAddr[4]=0xff;
		priv->pshare->rf_ft_var.mc2u_flood_mac[3].macAddr[5]=0xfa;
		
		priv->pshare->rf_ft_var.mc2u_flood_mac_num=4;
#endif

#ifdef _TRACKING_TABLE_FILE
		if (GET_CHIP_VER(priv) == VERSION_8812E)
			priv->pshare->rf_ft_var.pwr_track_file = 1;
#endif
	return 0;
}


static int rtl8192cd_stop_sw(struct rtl8192cd_priv *priv)
{
	struct rtl8192cd_hw *phw;
	unsigned long	flags;
	int	i;

#ifdef CONFIG_WLAN_HAL
    int                         halQnum;
    PHCI_TX_DMA_MANAGER_88XX    ptx_dma;
#endif // CONFIG_WLAN_HAL

	// we hope all this can be done in critical section
	SAVE_INT_AND_CLI(flags);

#ifdef INCLUDE_WPS
   	priv->pshare->WSC_CONT_S.wait_reinit = 1 ;
#endif

	if (timer_pending(&priv->frag_to_filter))
		del_timer_sync(&priv->frag_to_filter);

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#ifdef DETECT_STA_EXISTANCE
		// Added by Annie for Retry Limit Recovery Timer, 2010-08-10.
		if (timer_pending(&priv->pshare->rl_recover_timer))
			del_timer_sync (&priv->pshare->rl_recover_timer);
#endif		
		if (timer_pending(&priv->expire_timer))
			del_timer_sync(&priv->expire_timer);
#ifdef 	SW_ANT_SWITCH
		if (timer_pending(&priv->pshare->swAntennaSwitchTimer))
			del_timer_sync(&priv->pshare->swAntennaSwitchTimer);
#endif		
		if (timer_pending(&priv->pshare->rc_sys_timer))
			del_timer_sync(&priv->pshare->rc_sys_timer);
#if 0
		if (timer_pending(&priv->pshare->phw->tpt_timer))
			del_timer_sync(&priv->pshare->phw->tpt_timer);
#endif
#ifdef CONFIG_RTL_WAPI_SUPPORT
		if (priv->pmib->wapiInfo.wapiType!=wapiDisable
#ifdef CHECK_HANGUP
		&&(!priv->reset_hangup)
#endif
#ifdef SMART_REPEATER_MODE
		&&(!priv->pshare->switch_chan_rp)
#endif
		)
		{
			wapiExit(priv);
		}


#ifdef MBSSID
	if( priv->pmib->miscEntry.vap_enable) {
	  for (i=0; i<RTL8192CD_NUM_VWLAN; i++){
		  if (priv->pvap_priv[i]->pmib->wapiInfo.wapiType!=wapiDisable
#ifdef CHECK_HANGUP
			&&(!priv->pvap_priv[i]->reset_hangup)
#endif
			) {
			  wapiExit(priv->pvap_priv[i]);
		  }
	  }
  }
#endif
#endif

#ifdef PCIE_POWER_SAVING
	if (timer_pending(&priv->ps_timer))
		del_timer_sync(&priv->ps_timer);
#endif		

#ifdef CONFIG_RTK_MESH
		/*
		 * CAUTION !! These statement meshX(virtual interface) ONLY, Maybe modify....
		 */
		if (timer_pending(&priv->mesh_peer_link_timer))
			del_timer_sync(&priv->mesh_peer_link_timer);

#ifdef MESH_BOOTSEQ_AUTH
		if (timer_pending(&priv->mesh_auth_timer))
			del_timer_sync(&priv->mesh_auth_timer);
#endif
#endif

#if defined(CONFIG_RTL_92D_SUPPORT) && defined(DPK_92D)
		if (GET_CHIP_VER(priv) == VERSION_8192D){
			if (timer_pending(&priv->pshare->DPKTimer))
				del_timer_sync(&priv->pshare->DPKTimer);
		}
#endif

#ifdef SMART_REPEATER_MODE
		if (timer_pending(&priv->pshare->check_vxd_ap))			
			del_timer_sync(&priv->pshare->check_vxd_ap);
#endif
	}
	if (timer_pending(&priv->ss_timer))
		del_timer_sync(&priv->ss_timer);
	if (timer_pending(&priv->MIC_check_timer))
		del_timer_sync(&priv->MIC_check_timer);
	if (timer_pending(&priv->assoc_reject_timer))
		del_timer_sync(&priv->assoc_reject_timer);
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)
	// to avoid add RAtid fail
	if (timer_pending(&priv->add_RATid_timer))
		del_timer_sync(&priv->add_RATid_timer);
	if (timer_pending(&priv->add_rssi_timer))
		del_timer_sync(&priv->add_rssi_timer);
#endif	
	if (timer_pending(&priv->add_ps_timer))
		del_timer_sync(&priv->add_ps_timer);


#if defined(CONFIG_RTL_92D_SUPPORT) && defined(CONFIG_RTL_NOISE_CONTROL)
	if (GET_CHIP_VER(priv) == VERSION_8192D){
		if (timer_pending(&priv->dnc_timer))
			del_timer_sync(&priv->dnc_timer);
	}
#endif

#ifdef SMART_CONCURRENT_92D
	if (GET_CHIP_VER(priv) == VERSION_8192D){
		if (timer_pending(&priv->smcc_prb_timer))
			del_timer_sync(&priv->smcc_prb_timer);
	}
#endif

#ifdef CLIENT_MODE
	if (timer_pending(&priv->reauth_timer))
		del_timer_sync(&priv->reauth_timer);
	if (timer_pending(&priv->reassoc_timer))
		del_timer_sync(&priv->reassoc_timer);
	if (timer_pending(&priv->idle_timer))
		del_timer_sync(&priv->idle_timer);
#endif

#ifdef GBWC
	if (timer_pending(&priv->GBWC_timer))
		del_timer_sync(&priv->GBWC_timer);
	while (CIRC_CNT(priv->GBWC_tx_queue.head, priv->GBWC_tx_queue.tail, NUM_TXPKT_QUEUE))
	{
		struct sk_buff *pskb = priv->GBWC_tx_queue.pSkb[priv->GBWC_tx_queue.tail];
		rtl_kfree_skb(priv, pskb, _SKB_TX_);
		priv->GBWC_tx_queue.tail++;
		priv->GBWC_tx_queue.tail = priv->GBWC_tx_queue.tail & (NUM_TXPKT_QUEUE - 1);
	}
	while (CIRC_CNT(priv->GBWC_rx_queue.head, priv->GBWC_rx_queue.tail, NUM_TXPKT_QUEUE))
	{
		struct sk_buff *pskb = priv->GBWC_rx_queue.pSkb[priv->GBWC_rx_queue.tail];
		rtl_kfree_skb(priv, pskb, _SKB_RX_);
		priv->GBWC_rx_queue.tail++;
		priv->GBWC_rx_queue.tail = priv->GBWC_rx_queue.tail & (NUM_TXPKT_QUEUE - 1);
	}
#endif

#ifdef INCLUDE_WPA_PSK
	if (timer_pending(&priv->wpa_global_info->GKRekeyTimer))
		del_timer_sync(&priv->wpa_global_info->GKRekeyTimer);
#endif

#ifdef USE_TXQUEUE
	for (i=0; i<7; i++) {
		struct txq_node *pnode, *phead, *pnext;

		phead = (struct txq_node *)&(priv->pshare->txq_list[i].list);
		pnode = phead->list.next;
		pnext = pnode;
		while (pnext != phead)
		{
			pnode = pnext;
			pnext = pnext->list.next;
			if (pnode->skb && pnode->dev && pnode->dev->priv == priv) {
				unlink_txq(&(priv->pshare->txq_list[i]), pnode);
				rtl_kfree_skb(priv, pnode->skb, _SKB_TX_);
				pnode->skb = pnode->dev = NULL;
				list_add_tail(&pnode->list, &priv->pshare->txq_pool);
			}
		}
	}

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
#if defined(RESERVE_TXDESC_FOR_EACH_IF)
	for (i=0; i<7; i++) {
		priv->use_txq_cnt[i] = 0;
	}
#endif

	if (IS_ROOT_INTERFACE(priv))
#endif
	{
		free_txq_pool(&priv->pshare->txq_pool,priv->pshare->txq_pool_addr);
	}
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#ifdef DFS
		if (timer_pending(&priv->DFS_timer))
			del_timer_sync(&priv->DFS_timer);

		if (timer_pending(&priv->ch_avail_chk_timer))
			del_timer_sync(&priv->ch_avail_chk_timer);

		if (timer_pending(&priv->dfs_chk_timer))
			del_timer_sync(&priv->dfs_chk_timer);

		/*
		 *	when we disable the DFS function dynamically, we also remove the channel
		 *	from NOP_chnl while the driver is rebooting
		 */
		if (timer_pending(&priv->ch52_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch52_timer);
			RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 52);
		}

		if (timer_pending(&priv->ch56_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch56_timer);
			RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 56);
		}

		if (timer_pending(&priv->ch60_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch60_timer);
			RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 60);
		}

		if (timer_pending(&priv->ch64_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch64_timer);
			RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 64);
		}

		if (timer_pending(&priv->ch100_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch100_timer);
			RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 100);
		}

		if (timer_pending(&priv->ch104_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch104_timer);
			RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 104);
		}

		if (timer_pending(&priv->ch108_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch108_timer);
			RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 108);
		}

		if (timer_pending(&priv->ch112_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch112_timer);
			RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 112);
		}

		if (timer_pending(&priv->ch116_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch116_timer);
			RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 116);
		}

		if (timer_pending(&priv->ch120_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch120_timer);
			RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 120);
		}

		if (timer_pending(&priv->ch124_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch124_timer);
			RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 124);
		}

		if (timer_pending(&priv->ch128_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch128_timer);
			RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 128);
		}

		if (timer_pending(&priv->ch132_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch128_timer);
			RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 128);
		}

		if (timer_pending(&priv->ch136_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch136_timer);
			RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 136);
		}

		if (timer_pending(&priv->ch140_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch140_timer);
			RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 140);
		}

		/*
		 *	For JAPAN in adhoc mode
		 */
		if (((priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK)	||
			 (priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK3)) &&
			 (OPMODE & WIFI_ADHOC_STATE)) {
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) {
				if (!timer_pending(&priv->ch52_timer))
					RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 52);
				if (!timer_pending(&priv->ch56_timer))
					RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 56);
				if (!timer_pending(&priv->ch60_timer))
					RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 60);
				if (!timer_pending(&priv->ch64_timer))
					RemoveChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 64);
			}
		}
#endif // DFS

		// for SW LED
		if ((LED_TYPE >= LEDTYPE_SW_LINK_TXRX) && (LED_TYPE < LEDTYPE_SW_MAX))
			disable_sw_LED(priv);

#ifdef __KERNEL__
#ifdef DFS
		/* prevent killing tasklet issue in interrupt */
		if (!priv->pmib->dot11DFSEntry.DFS_detected)
#endif
		{
#ifdef CHECK_HANGUP
			if (!priv->reset_hangup)
#endif
			{
#ifdef P2P_SUPPORT
				if((OPMODE&WIFI_P2P_SUPPORT))
				{

				}else
#endif				
				{
#ifdef SMART_REPEATER_MODE
					if (!priv->pshare->switch_chan_rp)
#endif
					{				
						tasklet_kill(&priv->pshare->rx_tasklet);
						tasklet_kill(&priv->pshare->tx_tasklet);				
						tasklet_kill(&priv->pshare->oneSec_tasklet);
					}
				}

			}
		}
#endif // __KERNEL__

		phw = GET_HW(priv);

#ifdef DELAY_REFILL_RX_BUF
		priv->pshare->phw->cur_rx_refill = priv->pshare->phw->cur_rx = 0;       // avoid refill to rx ring
#if defined(CONFIG_WLAN_HAL)
		{
			PHCI_RX_DMA_MANAGER_88XX    prx_dma =     prx_dma = (PHCI_RX_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PRxDMA88XX);
		    PHCI_RX_DMA_QUEUE_STRUCT_88XX	cur_q   = &(prx_dma->rx_queue[0]);
			cur_q->host_idx = cur_q->cur_host_idx = 0;
			cur_q->rxbd_ok_cnt = 0;
		}
#endif
#endif
		for (i=0; i<NUM_RX_DESC; i++)
		{
			if (phw->rx_infoL[i].pbuf != NULL) {
#ifdef CONFIG_NET_PCI
				// if pci bios, then pci_unmap_single and dev_kfree_skb
				if (IS_PCIBIOS_TYPE)
					pci_unmap_single(priv->pshare->pdev, phw->rx_infoL[i].paddr, (RX_BUF_LEN - sizeof(struct rx_frinfo)), PCI_DMA_FROMDEVICE);
#endif
				rtl_kfree_skb(priv, (struct sk_buff*)(phw->rx_infoL[i].pbuf), _SKB_RX_);
			}
		}

		// free the skb buffer in Low and Hi queue
		DEBUG_INFO("free tx Q0 head %d tail %d\n", phw->txhead0, phw->txtail0);
		DEBUG_INFO("free tx Q1 head %d tail %d\n", phw->txhead1, phw->txtail1);
		DEBUG_INFO("free tx Q2 head %d tail %d\n", phw->txhead2, phw->txtail2);
		DEBUG_INFO("free tx Q3 head %d tail %d\n", phw->txhead3, phw->txtail3);
		DEBUG_INFO("free tx Q4 head %d tail %d\n", phw->txhead4, phw->txtail4);
		DEBUG_INFO("free tx Q5 head %d tail %d\n", phw->txhead5, phw->txtail5);

		for (i=0; i<CURRENT_NUM_TX_DESC; i++)
		{
			// free tx queue skb
			struct tx_desc_info *tx_info;
			int j;
			int	head, tail;
			int max_qnum = HIGH_QUEUE;
#if defined(CONFIG_WLAN_HAL) && defined(MBSSID)
			if(IS_HAL_CHIP(priv) && GET_ROOT(priv)->pmib->miscEntry.vap_enable)
				max_qnum = HIGH_QUEUE7;
#endif		

			for (j=0; j<=max_qnum; j++) {
#ifdef CONFIG_WLAN_HAL
				if (IS_HAL_CHIP(priv)) {
	                halQnum = GET_HAL_INTERFACE(priv)->MappingTxQueueHandler(priv, j);
	                ptx_dma = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PTxDMA88XX);
                    #if 0
	                head    = GET_HAL_INTERFACE(priv)->GetTxQueueHWIdxHandler(priv, j);
                    #else
	                head    = ptx_dma->tx_queue[halQnum].host_idx;
                    #endif
	                tail    = ptx_dma->tx_queue[halQnum].hw_idx;
				} else 
#endif				
				{
					head = get_txhead(phw, j);
					tail = get_txtail(phw, j);
				}
//				if (i <tail || i >= head)
				if( (tail <= head) ? (i <tail || i >= head) :(i <tail && i >= head))
					continue;

				tx_info = get_txdesc_info(priv->pshare->pdesc_info, j);

#if defined(RESERVE_TXDESC_FOR_EACH_IF) && (defined(UNIVERSAL_REPEATER) || defined(MBSSID))	//Family add
				if (tx_info[i].priv)
					tx_info[i].priv->use_txdesc_cnt[j]--;
#endif
				
#if defined(CONFIG_WLAN_HAL)
                if (IS_HAL_CHIP(priv)) {
    				if (tx_info[i].buf_pframe[0] && (tx_info[i].buf_type[0] == _SKB_FRAME_TYPE_)) { // should be buf_paddr
#if defined(CONFIG_NET_PCI) && !defined(USE_RTL8186_SDK)
    					if (IS_PCIBIOS_TYPE && (tx_info[i].buf_len[0]!=0 && tx_info[i].buf_paddr[0]!=0))
    						pci_unmap_single(priv->pshare->pdev, tx_info[i].buf_paddr[0],(tx_info[i].buf_len[0])&0xffff, PCI_DMA_TODEVICE);
#endif
#ifdef MP_TEST
    					if ((OPMODE & (WIFI_MP_STATE|WIFI_MP_CTX_BACKGROUND))==(WIFI_MP_STATE|WIFI_MP_CTX_BACKGROUND)) {
    						priv->pshare->skb_tail = (priv->pshare->skb_tail + 1) & (NUM_MP_SKB - 1);
    					}
    					else
#endif
    					{
    						rtl_kfree_skb(priv, tx_info[i].buf_pframe[0], _SKB_TX_);
    						DEBUG_INFO("free skb in queue %d\n", j);
    					}		
    				}
                } else
#endif // defined(CONFIG_WLAN_HAL)
                {
    				if (tx_info[i].pframe && (tx_info[i].type == _SKB_FRAME_TYPE_)) {
#ifdef CONFIG_NET_PCI
    					if (IS_PCIBIOS_TYPE)
    						pci_unmap_single(priv->pshare->pdev, tx_info[i].paddr, (tx_info[i].len), PCI_DMA_TODEVICE);
#endif

#if 1//def CONFIG_RTL8672
#ifdef MP_TEST
    					if ((OPMODE & (WIFI_MP_STATE|WIFI_MP_CTX_BACKGROUND))==(WIFI_MP_STATE|WIFI_MP_CTX_BACKGROUND)) {
    						priv->pshare->skb_tail = (priv->pshare->skb_tail + 1) & (NUM_MP_SKB - 1);
    					}
    					else
#endif
    					{
    						rtl_kfree_skb(priv, tx_info[i].pframe, _SKB_TX_);
    						DEBUG_INFO("free skb in queue %d\n", j);
    					}
#else //CONFIG_RTL8672
    					rtl_kfree_skb(priv, tx_info[i].pframe, _SKB_TX_);
    					DEBUG_INFO("free skb in queue %d\n", j);
#endif //CONFIG_RTL8672
    				}
                }
			}
		} // TX descriptor Free


#if 1//def CONFIG_RTL8672
#ifdef MP_TEST
		if ((OPMODE & (WIFI_MP_STATE|WIFI_MP_CTX_BACKGROUND))==(WIFI_MP_STATE|WIFI_MP_CTX_BACKGROUND)) {
			OPMODE &= ~WIFI_MP_CTX_BACKGROUND;
			
			for (i=0; i<NUM_MP_SKB; i++)
				kfree(priv->pshare->skb_pool[i]->head);
			kfree(priv->pshare->skb_pool_ptr);
			
			DEBUG_INFO("[%s %d] skb_head/skb_tail=%d/%d\n",
					__FUNCTION__, __LINE__, priv->pshare->skb_head, priv->pshare->skb_tail);
		}
#endif
#endif

		// unmap  beacon buffer
#if defined(CONFIG_NET_PCI)
#ifdef CONFIG_WLAN_HAL
		if (IS_PCIBIOS_TYPE) {
			PHCI_TX_DMA_MANAGER_88XX ptx_dma = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PTxDMA88XX);;
			u32 halQNum = GET_HAL_INTERFACE(priv)->MappingTxQueueHandler(priv, BEACON_QUEUE);
			PHCI_TX_DMA_QUEUE_STRUCT_88XX cur_q = &(ptx_dma->tx_queue[halQNum]);
			PTX_BUFFER_DESCRIPTOR cur_txbd = cur_q->pTXBD_head;
			
			pci_unmap_single(priv->pshare->pdev, get_desc(cur_txbd->TXBD_ELE[1].Dword1), 128*sizeof(unsigned int), PCI_DMA_TODEVICE);
		}
#else
		if (IS_PCIBIOS_TYPE) {
			pci_unmap_single(priv->pshare->pdev, get_desc(phw->tx_descB->Dword8), 128*sizeof(unsigned int), PCI_DMA_TODEVICE);
		}
#endif
#endif // defined(CONFIG_NET_PCI)

#ifdef RX_BUFFER_GATHER
		flush_rx_list(priv);
#endif
	} // if (IS_ROOT_INTERFACE(priv))

	for (i=0; i<NUM_STAT; i++)
	{
		if (priv->pshare->aidarray[i]) {
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
			if (priv != priv->pshare->aidarray[i]->priv)
				continue;
			else
#endif
			{
				if (priv->pshare->aidarray[i]->used == TRUE)
					if (free_stainfo(priv, &(priv->pshare->aidarray[i]->station)) == FALSE)
					DEBUG_ERR("free station %d fails\n", i);

#if defined(WIFI_WMM) && defined(WMM_APSD)
#ifdef PRIV_STA_BUF
				free_sta_que(priv, priv->pshare->aidarray[i]->station.VO_dz_queue);
				free_sta_que(priv, priv->pshare->aidarray[i]->station.VI_dz_queue);
				free_sta_que(priv, priv->pshare->aidarray[i]->station.BE_dz_queue);
				free_sta_que(priv, priv->pshare->aidarray[i]->station.BK_dz_queue);

#else
				kfree(priv->pshare->aidarray[i]->station.VO_dz_queue);
				kfree(priv->pshare->aidarray[i]->station.VI_dz_queue);
				kfree(priv->pshare->aidarray[i]->station.BE_dz_queue);
				kfree(priv->pshare->aidarray[i]->station.BK_dz_queue);
#endif
#endif

#if defined(WIFI_WMM)
#ifdef PRIV_STA_BUF
				free_sta_mgt_que(priv, priv->pshare->aidarray[i]->station.MGT_dz_queue);
#else
				kfree(priv->pshare->aidarray[i]->station.MGT_dz_queue);
#endif
#endif

#ifdef INCLUDE_WPA_PSK
#ifdef PRIV_STA_BUF
				free_wpa_buf(priv, priv->pshare->aidarray[i]->station.wpa_sta_info);
#else
				kfree(priv->pshare->aidarray[i]->station.wpa_sta_info);
#endif
#endif
#ifdef RTL8192CD_VARIABLE_USED_DMEM
			{
				unsigned int index = (unsigned int)i;
				rtl8192cd_dmem_free(AID_OBJ, &index);
			}
#else
#ifdef PRIV_STA_BUF
				free_sta_obj(priv, priv->pshare->aidarray[i]);
#else
				kfree(priv->pshare->aidarray[i]);
#endif
#endif
				priv->pshare->aidarray[i] = NULL;
			}
		}
	}

#ifndef __DRAYTEK_OS__
	// reset rsnie and group key from open to here, david
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#ifdef WIFI_SIMPLE_CONFIG
		if (!priv->pmib->dot11OperationEntry.keep_rsnie) {
			priv->pmib->wscEntry.beacon_ielen = 0;
			priv->pmib->wscEntry.probe_rsp_ielen = 0;
			priv->pmib->wscEntry.probe_req_ielen = 0;
			priv->pmib->wscEntry.assoc_ielen = 0;
		}

//		if (!(OPMODE & WIFI_AP_STATE))
//			priv->pmib->dot11OperationEntry.keep_rsnie = 1;
#endif
	}

	if (!priv->pmib->dot11OperationEntry.keep_rsnie) {
		priv->pmib->dot11RsnIE.rsnielen = 0;	// reset RSN IE length
		memset(&priv->pmib->dot11GroupKeysTable, '\0', sizeof(struct Dot11KeyMappingsEntry)); // reset group key
#ifdef UNIVERSAL_REPEATER
		if (GET_VXD_PRIV(priv))
			GET_VXD_PRIV(priv)->pmib->dot11RsnIE.rsnielen = 0;
#endif
		priv->auto_channel_backup = 0;
	}
	else {
		// When wlan scheduling and auto-chan case, it will disable/enable
		// wlan interface directly w/o re-set mib. Therefore, we need use
		// "keep_rsnie" flag to keep auto-chan value

		if (
#ifdef CHECK_HANGUP
			!priv->reset_hangup &&
#endif
#ifdef SMART_REPEATER_MODE
			!priv->pshare->switch_chan_rp &&
#endif
			priv->auto_channel_backup)
			priv->pmib->dot11RFEntry.dot11channel = 0;
	}
#endif

#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
	{
		if (IS_VAP_INTERFACE(priv) && !priv->pmib->dot11OperationEntry.keep_rsnie) {
			priv->pmib->dot11RsnIE.rsnielen = 0;	// reset RSN IE length
			memset(&priv->pmib->dot11GroupKeysTable, '\0', sizeof(struct Dot11KeyMappingsEntry)); // reset group key
		}
	}
#ifdef  CONFIG_WLAN_HAL
	if (!IS_HAL_CHIP(priv))	// mark by Pedro: because priv->bcn_period_bak is zero currently.
#endif	
	priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod = priv->bcn_period_bak;
#endif

#ifdef RTK_BR_EXT
	if (OPMODE & (WIFI_STATION_STATE | WIFI_ADHOC_STATE)) {
#ifdef CHECK_HANGUP
		if (!priv->reset_hangup)
#endif
			nat25_db_cleanup(priv);
	}
#endif

#ifdef A4_STA
	if (OPMODE & WIFI_AP_STATE) {
#ifdef CHECK_HANGUP
		if (!priv->reset_hangup)
#endif
			a4_sta_cleanup(priv);
	}
#endif

	{
		int				hd, tl;
		struct sk_buff	*pskb;

		hd = priv->dz_queue.head;
		tl = priv->dz_queue.tail;
		while (CIRC_CNT(hd, tl, NUM_TXPKT_QUEUE))
		{
			pskb = priv->dz_queue.pSkb[tl];
			rtl_kfree_skb(priv, pskb, _SKB_TX_);
			tl++;
			tl = tl & (NUM_TXPKT_QUEUE - 1);
		}
		priv->dz_queue.head = 0;
		priv->dz_queue.tail = 0;
	}

/*#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) 
    	GET_HAL_INTERFACE(priv)->StopSWHandler(priv);
#endif  //CONFIG_WLAN_HAL*/

#ifdef USE_OUT_SRC
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif		
#ifdef _OUTSRC_COEXIST
		if(IS_OUTSRC_CHIP(priv))
#endif
		ODM_CancelAllTimers(ODMPTR);
#endif
#if !(defined(RTL8190_ISR_RX) && defined(RTL8190_DIRECT_RX))
	flush_rx_queue(priv);
#endif

RESTORE_INT(flags);
	return 0;
}


#ifdef MBSSID
static void rtl8192cd_init_vap_mib(struct rtl8192cd_priv *priv)
{
#if 0
	unsigned char tmpbuf[36], hwaddr[6];
	int len;
	//unsigned int AclMode;
	//unsigned char AclAddr[NUM_ACL][MACADDRLEN];
	//unsigned int AclNum;
	struct Dot1180211AuthEntry dot1180211AuthEntry;
	struct Dot118021xAuthEntry dot118021xAuthEntry;
	struct Dot11DefaultKeysTable dot11DefaultKeysTable;
	struct Dot11RsnIE dot11RsnIE;

	// backup mib that can differ from root interface
	memcpy(hwaddr, GET_MY_HWADDR, 6);
	len = SSID_LEN;
	memcpy(tmpbuf, SSID, len);
	//AclMode = priv->pmib->dot11StationConfigEntry.dot11AclMode;
	//memcpy(AclAddr, priv->pmib->dot11StationConfigEntry.dot11AclAddr, sizeof(AclAddr));
	//AclNum = priv->pmib->dot11StationConfigEntry.dot11AclNum;
	memcpy(&dot1180211AuthEntry, &priv->pmib->dot1180211AuthEntry, sizeof(struct Dot1180211AuthEntry));
	memcpy(&dot118021xAuthEntry, &priv->pmib->dot118021xAuthEntry, sizeof(struct Dot118021xAuthEntry));
	memcpy(&dot11DefaultKeysTable, &priv->pmib->dot11DefaultKeysTable, sizeof(struct Dot11DefaultKeysTable));
	memcpy(&dot11RsnIE, &priv->pmib->dot11RsnIE, sizeof(struct Dot11RsnIE));

	// copy mib from root interface
	memcpy(priv->pmib, GET_ROOT_PRIV(priv)->pmib, sizeof(struct wifi_mib));

	// restore the different part
	memcpy(GET_MY_HWADDR, hwaddr, 6);
	SSID_LEN = len;
	memcpy(SSID, tmpbuf, len);
	SSID2SCAN_LEN = len;
	memcpy(SSID2SCAN, SSID, len);
	//priv->pmib->dot11StationConfigEntry.dot11AclMode = AclMode;
	//memcpy(priv->pmib->dot11StationConfigEntry.dot11AclAddr, AclAddr, sizeof(AclAddr));
	//priv->pmib->dot11StationConfigEntry.dot11AclNum = AclNum;
	memcpy(&priv->pmib->dot1180211AuthEntry, &dot1180211AuthEntry, sizeof(struct Dot1180211AuthEntry));
	memcpy(&priv->pmib->dot118021xAuthEntry, &dot118021xAuthEntry, sizeof(struct Dot118021xAuthEntry));
	memcpy(&priv->pmib->dot11DefaultKeysTable, &dot11DefaultKeysTable, sizeof(struct Dot11DefaultKeysTable));
	memcpy(&priv->pmib->dot11RsnIE, &dot11RsnIE, sizeof(struct Dot11RsnIE));
#endif

	// copy mib_rf from root interface
	memcpy(&priv->pmib->dot11RFEntry, &GET_ROOT_PRIV(priv)->pmib->dot11RFEntry, sizeof(struct Dot11RFEntry));

	// special mib that need to set
#ifdef WIFI_WMM
	//QOS_ENABLE = 0;
#ifdef WMM_APSD
	APSD_ENABLE = 0;
#endif
#endif

#ifdef WDS
	// always disable wds in vap
	priv->pmib->dot11WdsInfo.wdsEnabled = 0;
	priv->pmib->dot11WdsInfo.wdsPure = 0;
#endif
#ifdef CONFIG_RTK_MESH
	// in current release, mesh can be only run upon wlan0, so we disable the following flag in vap
	priv->pmib->dot1180211sInfo.mesh_enable = 0;
#endif
}


static void rtl8192cd_init_mbssid(struct rtl8192cd_priv *priv)
{
	int i, j;
	unsigned int camData[2];
	unsigned char *macAddr = GET_MY_HWADDR;

	if (IS_ROOT_INTERFACE(priv))
	{
		//camData[0] = 0x00800000 | (macAddr[5] << 8) | macAddr[4];
		camData[0] = MBIDCAM_POLL | MBIDWRITE_EN | MBIDCAM_VALID | (macAddr[5] << 8) | macAddr[4];
		camData[1] = (macAddr[3] << 24) | (macAddr[2] << 16) | (macAddr[1] << 8) | macAddr[0];
//		for (j=0; j<2; j++) {
		for (j=1; j>=0; j--) {
			//RTL_W32((_MBIDCAMCONTENT_+4)-4*j, camData[j]);
			RTL_W32((MBIDCAMCFG+4)-4*j, camData[j]);
		}
		//RTL_W8(_MBIDCAMCFG_, BIT(7) | BIT(6));

		// clear the rest area of CAM
		//camData[0] = 0;
		camData[1] = 0;
		for (i=1; i<8; i++) {
			camData[0] = MBIDCAM_POLL | MBIDWRITE_EN | (i&MBIDCAM_ADDR_Mask)<<MBIDCAM_ADDR_SHIFT;
//			for (j=0; j<2; j++) {
			for (j=1; j>=0; j--) {
				RTL_W32((MBIDCAMCFG+4)-4*j, camData[j]);
			}
//			RTL_W8(_MBIDCAMCFG_, BIT(7) | BIT(6) | (unsigned char)i);
		}

		// set MBIDCTRL & MBID_BCN_SPACE by cmd
//		set_fw_reg(priv, 0xf1000101, 0, 0);
		RTL_W32(MBSSID_BCN_SPACE,
			(priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod & BCN_SPACE2_Mask)<<BCN_SPACE2_SHIFT
			|(priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod & BCN_SPACE1_Mask)<<BCN_SPACE1_SHIFT);

		RTL_W8(BCN_CTRL, 0);
		RTL_W8(0x553, 1);

#ifdef CONFIG_RTL_92C_SUPPORT
		if (IS_TEST_CHIP(priv))
			RTL_W8(BCN_CTRL, EN_BCN_FUNCTION |EN_MBSSID| DIS_SUB_STATE | DIS_TSF_UPDATE|EN_TXBCN_RPT);
		else
#endif
			RTL_W8(BCN_CTRL, EN_BCN_FUNCTION | DIS_SUB_STATE_N | DIS_TSF_UPDATE_N|EN_TXBCN_RPT);

		RTL_W32(RCR, RTL_R32(RCR) | RCR_MBID_EN);	// MBSSID enable
/*
#ifdef CLIENT_MODE
		if ((OPMODE & WIFI_STATION_STATE) || (OPMODE & WIFI_ADHOC_STATE))
			RTL_W32(RCR, RTL_R32(RCR) | RCR_CBSSID);
#endif
*/
	}
	else if (IS_VAP_INTERFACE(priv))
	{
//		priv->vap_init_seq = (RTL_R8(_MBIDCTRL_) & (BIT(4) | BIT(5) | BIT(6))) >> 4;
//		priv->vap_init_seq++;
//		set_fw_reg(priv, 0xf1000001 | ((priv->vap_init_seq + 1)&0xffff)<<8, 0, 0);

		priv->vap_init_seq = RTL_R8(MBID_NUM) & MBID_BCN_NUM_Mask;
		priv->vap_init_seq++;

#if defined(CONFIG_RTL_8812_SUPPORT) //eric_8812 ??
//		printk("init swq=%d\n", priv->vap_init_seq);
		if(GET_CHIP_VER(priv)==VERSION_8192E|| GET_CHIP_VER(priv)==VERSION_8812E)
		{
		switch (priv->vap_init_seq)
		{
			case 1:
				RTL_W16(REG_92E_ATIMWND1, 0x03); //0x3C);		
				RTL_W8(REG_92E_DTIM_COUNT_VAP1, priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod);
				break;
			case 2:
				RTL_W8(REG_92E_ATIMWND2, 0x3C);
				RTL_W8(REG_92E_DTIM_COUNT_VAP2, priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod);
				break;
			case 3:
				RTL_W8(REG_92E_ATIMWND3, 0x3C);
				RTL_W8(REG_92E_DTIM_COUNT_VAP3, priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod);
				break;
			case 4:
				RTL_W8(REG_92E_ATIMWND4, 0x3C);
				RTL_W8(REG_92E_DTIM_COUNT_VAP4, priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod);
				break;
			case 5:
				RTL_W8(REG_92E_ATIMWND5, 0x3C);
				RTL_W8(REG_92E_DTIM_COUNT_VAP5, priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod);
				break;
			case 6:
				RTL_W8(REG_92E_ATIMWND6, 0x3C);
				RTL_W8(REG_92E_DTIM_COUNT_VAP6, priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod);
				break;
			case 7:
				RTL_W8(REG_92E_ATIMWND7, 0x3C);
				RTL_W8(REG_92E_DTIM_COUNT_VAP7, priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod);
				break;
		}
		}
#endif
		
		camData[0] = MBIDCAM_POLL | MBIDWRITE_EN | MBIDCAM_VALID |
				(priv->vap_init_seq&MBIDCAM_ADDR_Mask)<<MBIDCAM_ADDR_SHIFT |
				(macAddr[5] << 8) | macAddr[4];
		camData[1] = (macAddr[3] << 24) | (macAddr[2] << 16) | (macAddr[1] << 8) | macAddr[0];
		for (j=1; j>=0; j--) {
			RTL_W32((MBIDCAMCFG+4)-4*j, camData[j]);
		}
//		RTL_W8(_MBIDCAMCFG_, BIT(7) | BIT(6) | ((unsigned char)priv->vap_init_seq & 0x1f));
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8188E) {
			unsigned int vap_bcn_offset = (priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod-
				((priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod/(priv->vap_init_seq+1))*priv->vap_init_seq)-1)
				& BCN_SPACE2_Mask;

			if (vap_bcn_offset > 200)
				vap_bcn_offset = 200;
			RTL_W32(MBSSID_BCN_SPACE, (vap_bcn_offset & BCN_SPACE2_Mask)<<BCN_SPACE2_SHIFT
				|(priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod & BCN_SPACE1_Mask)<<BCN_SPACE1_SHIFT);
		} else
#endif
#if defined(CONFIG_RTL_8812_SUPPORT)
		if ((GET_CHIP_VER(priv)==VERSION_8812E)) {
			unsigned int vap_bcn_offset = (priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod-
				((priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod/(priv->vap_init_seq+1))*priv->vap_init_seq)-1)
				& BCN_SPACE2_Mask;

			if (vap_bcn_offset > 200)
				vap_bcn_offset = 200;
			RTL_W32(MBSSID_BCN_SPACE, (vap_bcn_offset & BCN_SPACE2_Mask)<<BCN_SPACE2_SHIFT
				|(priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod & BCN_SPACE1_Mask)<<BCN_SPACE1_SHIFT);
		} else
#endif
		{
			RTL_W32(MBSSID_BCN_SPACE,
				((priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod-
				((priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod/(priv->vap_init_seq+1))*priv->vap_init_seq))
				& BCN_SPACE2_Mask)<<BCN_SPACE2_SHIFT
				|((priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod/(priv->vap_init_seq+1)) & BCN_SPACE1_Mask)
				<<BCN_SPACE1_SHIFT);
		}
		RTL_W8(BCN_CTRL, 0);
		RTL_W8(0x553, 1);

#ifdef CONFIG_RTL_92C_SUPPORT
		if (IS_TEST_CHIP(priv))
			RTL_W8(BCN_CTRL, EN_BCN_FUNCTION |EN_MBSSID| DIS_SUB_STATE | DIS_TSF_UPDATE|EN_TXBCN_RPT);
		else
#endif
			RTL_W8(BCN_CTRL, EN_BCN_FUNCTION | DIS_SUB_STATE_N | DIS_TSF_UPDATE_N|EN_TXBCN_RPT);

#if defined(CONFIG_RTL_8812_SUPPORT) //eric_8812 ??
		if(GET_CHIP_VER(priv)==VERSION_8812E)
		RTL_W8(MBID_NUM, (RTL_R8(MBID_NUM) & ~MBID_BCN_NUM_Mask) | (priv->vap_init_seq & MBID_BCN_NUM_Mask));
		else
#endif
		RTL_W8(MBID_NUM, priv->vap_init_seq & MBID_BCN_NUM_Mask);
		
		RTL_W32(RCR, RTL_R32(RCR) & ~RCR_MBID_EN);
		RTL_W32(RCR, RTL_R32(RCR) | RCR_MBID_EN);	// MBSSID enable
	}
}


static void rtl8192cd_stop_mbssid(struct rtl8192cd_priv *priv)
{
	int i, j;
	unsigned int camData[2];
	camData[1] = 0;

	if (IS_ROOT_INTERFACE(priv))
	{
		// clear the rest area of CAM
		for (i=0; i<8; i++) {
			camData[0] = MBIDCAM_POLL | MBIDWRITE_EN | (i&MBIDCAM_ADDR_Mask)<<MBIDCAM_ADDR_SHIFT;
			for (j=1; j>=0; j--) {
				RTL_W32((MBIDCAMCFG+4)-4*j, camData[j]);
			}
//			RTL_W8(_MBIDCAMCFG_, BIT(7) | BIT(6) | (unsigned char)i);
		}

//		set_fw_reg(priv, 0xf1000001, 0, 0);
		RTL_W32(RCR, RTL_R32(RCR) & ~RCR_MBID_EN);	// MBSSID disable
		RTL_W32(MBSSID_BCN_SPACE,
			(priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod & BCN_SPACE1_Mask)<<BCN_SPACE1_SHIFT);

		RTL_W8(BCN_CTRL, 0);
		RTL_W8(0x553, 1);
#ifdef CONFIG_RTL_92C_SUPPORT
		if (IS_TEST_CHIP(priv))
			RTL_W8(BCN_CTRL, EN_BCN_FUNCTION | DIS_SUB_STATE | DIS_TSF_UPDATE| EN_TXBCN_RPT);
		else
#endif
			RTL_W8(BCN_CTRL, EN_BCN_FUNCTION | DIS_SUB_STATE_N | DIS_TSF_UPDATE_N| EN_TXBCN_RPT);

	}
	else if (IS_VAP_INTERFACE(priv) && (priv->vap_init_seq >= 0))
	{
//		set_fw_reg(priv, 0xf1000001 | (((RTL_R8(_MBIDCTRL_) & (BIT(4) | BIT(5) | BIT(6))) >> 4)&0xffff)<<8, 0, 0);
		camData[0] = MBIDCAM_POLL | MBIDWRITE_EN |
			(priv->vap_init_seq&MBIDCAM_ADDR_Mask)<<MBIDCAM_ADDR_SHIFT;
		for (j=1; j>=0; j--) {
			RTL_W32((MBIDCAMCFG+4)-4*j, camData[j]);
		}
//		RTL_W8(_MBIDCAMCFG_, BIT(7) | BIT(6) | ((unsigned char)priv->vap_init_seq & 0x1f));

		if (RTL_R8(MBID_NUM) & MBID_BCN_NUM_Mask) {
#if defined(CONFIG_RTL_8812_SUPPORT) //eric_8812 ??
			if( GET_CHIP_VER(priv)==VERSION_8812E)
				RTL_W8(MBID_NUM, ((RTL_R8(MBID_NUM) & MBID_BCN_NUM_Mask)-1) & MBID_BCN_NUM_Mask);
			else
#endif			
			RTL_W8(MBID_NUM, (RTL_R8(MBID_NUM)-1) & MBID_BCN_NUM_Mask);
			
#ifdef CONFIG_RTL_88E_SUPPORT
			if (GET_CHIP_VER(priv)==VERSION_8188E) {
				unsigned int vap_bcn_offset = (priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod-
					((priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod/(RTL_R8(MBID_NUM)+1))
					*RTL_R8(MBID_NUM))-1) & BCN_SPACE2_Mask;

				if (vap_bcn_offset > 200)
					vap_bcn_offset = 200;
				RTL_W32(MBSSID_BCN_SPACE, (vap_bcn_offset & BCN_SPACE2_Mask)<<BCN_SPACE2_SHIFT|
					(priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod & BCN_SPACE1_Mask)<<BCN_SPACE1_SHIFT);
			} else
#endif
#if defined(CONFIG_RTL_8812_SUPPORT) //eric_8812 ??
			if ((GET_CHIP_VER(priv)==VERSION_8192E) || (GET_CHIP_VER(priv)==VERSION_8812E)) {
				RTL_W32(MBSSID_BCN_SPACE,
				((priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod-
				((priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod/((RTL_R8(MBID_NUM) & MBID_BCN_NUM_Mask)+1))*(RTL_R8(MBID_NUM)&MBID_BCN_NUM_Mask)))
				& BCN_SPACE2_Mask)<<BCN_SPACE2_SHIFT
				|((priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod/((RTL_R8(MBID_NUM) & MBID_BCN_NUM_Mask)+1)) & BCN_SPACE1_Mask)
				<<BCN_SPACE1_SHIFT);
			} else
#endif
			{
				RTL_W32(MBSSID_BCN_SPACE,
				((priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod-
				((priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod/(RTL_R8(MBID_NUM)+1))*RTL_R8(MBID_NUM)))
				& BCN_SPACE2_Mask)<<BCN_SPACE2_SHIFT
				|((priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod/(RTL_R8(MBID_NUM)+1)) & BCN_SPACE1_Mask)
				<<BCN_SPACE1_SHIFT);
			}

			RTL_W8(BCN_CTRL, 0);
			RTL_W8(0x553, 1);
#ifdef CONFIG_RTL_92C_SUPPORT
			if (IS_TEST_CHIP(priv))
				RTL_W8(BCN_CTRL, EN_BCN_FUNCTION | DIS_SUB_STATE | DIS_TSF_UPDATE| EN_TXBCN_RPT);
			else
#endif
				RTL_W8(BCN_CTRL, EN_BCN_FUNCTION | DIS_SUB_STATE_N | DIS_TSF_UPDATE_N| EN_TXBCN_RPT);

		}
		RTL_W32(RCR, RTL_R32(RCR) & ~RCR_MBID_EN);
		RTL_W32(RCR, RTL_R32(RCR) | RCR_MBID_EN);
		priv->vap_init_seq = -1;
	}
}
#endif


#ifdef WDS
#ifdef LAZY_WDS
void delete_wds_entry(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int i;
	
	for (i=0; i<NUM_WDS; i++) {
		if (!memcmp(priv->pmib->dot11WdsInfo.entry[i].macAddr, pstat->hwaddr, MACADDRLEN)) {
			memcpy(priv->pmib->dot11WdsInfo.entry[i].macAddr, 
							NULL_MAC_ADDR, MACADDRLEN);			
			free_stainfo(priv, pstat);				
			priv->pmib->dot11WdsInfo.wdsNum--;			
			break;
		}	
	}
}
#endif

struct stat_info *add_wds_entry(struct rtl8192cd_priv *priv, int idx, unsigned char *mac)
{
	struct stat_info *pstat;
	DOT11_SET_KEY Set_Key;
#ifdef LAZY_WDS
	int i;
	
	if (mac != NULL) {
		for (i=0; i<NUM_WDS; i++) {
			if (!memcmp(priv->pmib->dot11WdsInfo.entry[i].macAddr, 
						NULL_MAC_ADDR, MACADDRLEN)) {		
				memcpy(	priv->pmib->dot11WdsInfo.entry[i].macAddr, mac, MACADDRLEN);
				idx = i;				
				priv->pmib->dot11WdsInfo.wdsNum++;					
				break;
			}
		}	
		if (i == NUM_WDS) {
			DEBUG_ERR("WDS table is full!!!\n");
			return NULL;	
		}
	}
#endif

	pstat = alloc_stainfo(priv, priv->pmib->dot11WdsInfo.entry[idx].macAddr, -1);
	if (pstat == NULL) {
		DEBUG_ERR("alloc_stainfo() fail!\n");
		return NULL;
	}


	
	// use self supported rate for wds
	memcpy(pstat->bssrateset, AP_BSSRATE, AP_BSSRATE_LEN);
	pstat->bssratelen = AP_BSSRATE_LEN;
	pstat->state = WIFI_WDS;
	
	//WDEBUG("priv->pmib->dot11WdsInfo.entry[%d].txRate=%04x\n\n",idx , priv->pmib->dot11WdsInfo.entry[idx].txRate);
	
	if (  priv->pmib->dot11WdsInfo.entry[idx].txRate & BIT31 ) {
		//WDEBUG("under AC data rate\n");
	}else{
		if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11N))
			priv->pmib->dot11WdsInfo.entry[idx].txRate &= 0x0000fff;	// mask HT rates

		if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) &&
			!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11G))
			priv->pmib->dot11WdsInfo.entry[idx].txRate &= 0xffff00f;	// mask OFDM rates

		if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11B))
			priv->pmib->dot11WdsInfo.entry[idx].txRate &= 0xffffff0;	// mask CCK rates
	}

	//WDEBUG("priv->pmib->dot11WdsInfo.entry[%d].txRate=%04x\n\n", idx , priv->pmib->dot11WdsInfo.entry[idx].txRate);

#ifdef LAZY_WDS
	if (priv->pmib->dot11WdsInfo.wdsEnabled == WDS_LAZY_ENABLE)
		pstat->state |= WIFI_WDS_LAZY;		

	if ((priv->pmib->dot11WdsInfo.wdsEnabled == WDS_LAZY_ENABLE) &&
		(priv->pmib->dot11WdsInfo.wdsPrivacy == _TKIP_PRIVACY_ ||
			priv->pmib->dot11WdsInfo.wdsPrivacy == _CCMP_PRIVACY_))
		wds_psk_set(priv, idx, NULL);
#endif

			if (priv->pmib->dot11WdsInfo.wdsPrivacy == _WEP_40_PRIVACY_ ||
					priv->pmib->dot11WdsInfo.wdsPrivacy == _WEP_104_PRIVACY_ ) {
#ifndef CONFIG_RTL8186_KB
		memcpy(Set_Key.MACAddr, priv->pmib->dot11WdsInfo.entry[idx].macAddr, 6);
				Set_Key.KeyType = DOT11_KeyType_Pairwise;
				Set_Key.EncType = priv->pmib->dot11WdsInfo.wdsPrivacy;
				Set_Key.KeyIndex = priv->pmib->dot11WdsInfo.wdsKeyId;
				DOT11_Process_Set_Key(priv->dev, NULL, &Set_Key, priv->pmib->dot11WdsInfo.wdsWepKey);
#endif
			}
			else if ((priv->pmib->dot11WdsInfo.wdsPrivacy == _TKIP_PRIVACY_ ||
						priv->pmib->dot11WdsInfo.wdsPrivacy == _CCMP_PRIVACY_) &&
					(priv->pmib->dot11WdsInfo.wdsMappingKeyLen[idx]&0x80000000) ) {
		priv->pmib->dot11WdsInfo.wdsMappingKeyLen[idx] &= ~0x80000000;
		memcpy(Set_Key.MACAddr, priv->pmib->dot11WdsInfo.entry[idx].macAddr, 6);
				Set_Key.KeyType = DOT11_KeyType_Pairwise;
				Set_Key.EncType = priv->pmib->dot11WdsInfo.wdsPrivacy;
				Set_Key.KeyIndex = priv->pmib->dot11WdsInfo.wdsKeyId;
		DOT11_Process_Set_Key(priv->dev, NULL, &Set_Key, priv->pmib->dot11WdsInfo.wdsMapingKey[idx]);
			}

#ifdef CONFIG_RTL_88E_SUPPORT
			if (GET_CHIP_VER(priv)==VERSION_8188E) {
#ifdef TXREPORT
				add_RATid(priv, pstat);
#endif
			} else
#endif
			{
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)			
				add_update_RATid(priv, pstat);
#endif
			}
	pstat->wds_idx = idx;
			assign_tx_rate(priv, pstat, NULL);
			assign_aggre_mthod(priv, pstat);
			assign_aggre_size(priv, pstat);

			list_add_tail(&pstat->asoc_list, &priv->asoc_list);

	return pstat;
}

static void create_wds_tbl(struct rtl8192cd_priv *priv)
{
	int i;
	struct stat_info *pstat;

#ifdef FAST_RECOVERY
	if (priv->reset_hangup)
		return;
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (!IS_ROOT_INTERFACE(priv))
		return;
#endif

	if (priv->pmib->dot11WdsInfo.wdsEnabled) {
		for (i=0; i<priv->pmib->dot11WdsInfo.wdsNum; i++) {
			pstat = add_wds_entry(priv, i, NULL);
			if (pstat == NULL)
				break;
		}
	}
}
#endif //  WDS


void validate_fixed_tx_rate(struct rtl8192cd_priv *priv)
{
	if (!priv->pmib->dot11StationConfigEntry.autoRate) {
		if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC)) {
			priv->pmib->dot11StationConfigEntry.fixedTxRate &= ~ BIT(31);
		} 
		
		if(priv->pmib->dot11StationConfigEntry.fixedTxRate & BIT(31)) {
			if ((get_rf_mimo_mode(priv) == MIMO_1T1R) && (priv->pmib->dot11StationConfigEntry.fixedTxRate & 0x1f)>9)
				priv->pmib->dot11StationConfigEntry.fixedTxRate = 0;
		}
		else {
			if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11N))				
				priv->pmib->dot11StationConfigEntry.fixedTxRate &= 0x0000fff;	// mask HT rates

			if((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) &&
				((get_rf_mimo_mode(priv) == MIMO_1T2R) || (get_rf_mimo_mode(priv) == MIMO_1T1R)))	
					priv->pmib->dot11StationConfigEntry.fixedTxRate &= 0x00fffff;	// mask MCS8 - MCS15
			if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) &&
				!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11G))				
				priv->pmib->dot11StationConfigEntry.fixedTxRate &= 0xffff00f;	// mask OFDM rates
			if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11B))				
				priv->pmib->dot11StationConfigEntry.fixedTxRate &= 0xffffff0;	// mask CCK rates
		}

		if (priv->pmib->dot11StationConfigEntry.fixedTxRate==0) {
			priv->pmib->dot11StationConfigEntry.autoRate=1;
			PRINT_INFO("invalid fixed tx rate, use auto rate!\n");
		}
		else
			priv->pshare->current_tx_rate = get_rate_from_bit_value(priv->pmib->dot11StationConfigEntry.fixedTxRate);
	}
}


#if defined(RESERVE_TXDESC_FOR_EACH_IF) && (defined(UNIVERSAL_REPEATER) || defined(MBSSID))
void recalc_txdesc_limit(struct rtl8192cd_priv *priv)
{
	struct rtl8192cd_priv *root_priv = NULL;
	int i, num, total_if = 0;

	if (IS_ROOT_INTERFACE(priv))
		root_priv = priv;
	else
		root_priv = GET_ROOT_PRIV(priv);

	if (IS_DRV_OPEN(root_priv))
		total_if++;

#ifdef UNIVERSAL_REPEATER
	if (IS_DRV_OPEN(root_priv->pvxd_priv))
		total_if++;
#endif

#ifdef MBSSID
	for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
		if (IS_DRV_OPEN(root_priv->pvap_priv[i]))
			total_if++;
	}
#endif

	if (total_if <= 1) {
		root_priv->pshare->num_txdesc_cnt = CURRENT_NUM_TX_DESC - 2;  // 2 for space...
		root_priv->pshare->num_txdesc_upper_limit = CURRENT_NUM_TX_DESC - 2;
		root_priv->pshare->num_txdesc_lower_limit = 0;
#ifdef USE_TXQUEUE
		root_priv->pshare->num_txq_cnt = TXQUEUE_SIZE;
		root_priv->pshare->num_txq_upper_limit = TXQUEUE_SIZE;
		root_priv->pshare->num_txq_lower_limit = 0;
#endif
		return;
	}
	
	num = (CURRENT_NUM_TX_DESC * IF_TXDESC_UPPER_LIMIT) / 100;
	root_priv->pshare->num_txdesc_upper_limit = num;
	
	num = ((CURRENT_NUM_TX_DESC - 2) - num) / (total_if - 1);
	root_priv->pshare->num_txdesc_lower_limit = num;

	num = root_priv->pshare->num_txdesc_upper_limit + 
			root_priv->pshare->num_txdesc_lower_limit * (total_if - 1);
	root_priv->pshare->num_txdesc_cnt = num;

#ifdef USE_TXQUEUE
	num = (TXQUEUE_SIZE * IF_TXQ_UPPER_LIMIT) / 100;
	root_priv->pshare->num_txq_upper_limit = num;

	num = (TXQUEUE_SIZE - num) / (total_if - 1);
	root_priv->pshare->num_txq_lower_limit = num;

	num = root_priv->pshare->num_txq_upper_limit +
			root_priv->pshare->num_txq_lower_limit * (total_if - 1);
	root_priv->pshare->num_txq_cnt = num;
#endif
}
#endif

#if defined(CONFIG_RTL_8196E)	
#if defined(__ECOS) //mark_ecos
extern int bonding_type; //from switch
int sys_bonding_type()
{
	return bonding_type;
}
#else
extern int sys_bonding_type(void); //from rtl_gpio.c
#endif
#define 	BOND_8196ES	(0xD)
void rtl_8196es_gpio_init(void)
{
	//printk("rtl_8196es_gpio_init\n");
	// 8196ES GPIO 
	// WAKE# --> GPIOB5 , in    (not yet , FIXME )
	// 8188ER GPIO definition 	, Out = 0x10  In= 0x01
	// WPS button  --> GPIO7 ,In
	RTLWIFINIC_GPIO_config(7, 0x01);
	
	// WPS LED /  Reset LED   --> GPIO4 ,out
	RTLWIFINIC_GPIO_config(4, 0x10);

	// Reset button  --> GPIO0 , in
	RTLWIFINIC_GPIO_config(0, 0x01);
}
#endif

int rtl8192cd_open(struct net_device *dev)
{
	struct rtl8192cd_priv *priv;	// recuresively used, can't be static
	int rc;
    unsigned int errorFlag;
#ifdef MBSSID	
	int i;
#endif
	unsigned long x;
#if defined(CONFIG_RTL865X_WTDOG) || (defined(CONFIG_RTL_WTDOG) && defined(CONFIG_RTL_92D_SUPPORT))
#if !(defined(CONFIG_RTL8196B) || defined(CONFIG_RTL_819X))
	unsigned long wtval;
#endif
#endif
#ifdef CHECK_HANGUP
	int is_reset;
#endif
	int init_hw_cnt = 0;

	DBFENTER;

#ifdef NETDEV_NO_PRIV
	priv = ((struct rtl8192cd_priv *)netdev_priv(dev))->wlan_priv;
#else
	priv = dev->priv;
#endif

	#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
	/*to open custom pass through pseudo device*/
	if(dev==wlan_device[passThruWanIdx].priv->pWlanDev)
	{
		/*root device is opened*/
		if(priv->drv_state&DRV_STATE_OPEN)
		{
			netif_start_queue(dev);	
			return 0;
		}
		
	}
	#endif

#ifdef CONFIG_WLAN_HAL_8881A
    if(GET_CHIP_VER(priv)==VERSION_8881A) {
        //Enable MAC_System(BIT(0)), MAC_Lextra_Bus(BIT(1))
        REG32(0xB80000DC)= 0x03;
    }
#endif //CONFIG_WLAN_HAL_8881A
	
#if 0
//#ifdef PCIE_POWER_SAVING
	if (((REG32(CLK_MANAGE) & BIT(11)) == 0)
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
		&& IS_ROOT_INTERFACE(priv) 
#endif
	) {
		extern void setBaseAddressRegister(void);
		REG32(CLK_MANAGE) |=  BIT(11);
		delay_ms(10);
		PCIE_reset_procedure(0, 0, 1, priv->pshare->ioaddr);
		setBaseAddressRegister();		
	}
#endif

// for Virtual interface...
#ifdef CONFIG_WLAN_HAL
	if((GET_CHIP_VER(priv)== VERSION_8881A) || (GET_CHIP_VER(priv)== VERSION_8192E)) {
		priv->pshare->use_hal = 1;
	}
	else {
		priv->pshare->use_hal = 0;
	}
#endif

#ifdef USE_OUT_SRC
	if((GET_CHIP_VER(priv)== VERSION_8812E) || (GET_CHIP_VER(priv)== VERSION_8188E) || (IS_HAL_CHIP(priv)))
	{
		priv->pshare->use_outsrc = 1;
//		printk("use out source!!\n");
	}
	else
	{
		priv->pshare->use_outsrc = 0;
//		printk("NOT use out source!!\n");
	}
#endif

#ifdef CHECK_HANGUP
	is_reset = priv->reset_hangup;
#endif

// init mib from cfg file, we only need to load cfg file once - chris 2010/02
#ifdef CONFIG_RTL_COMAPI_CFGFILE
#ifdef WDS
	if (dev->base_addr) //root
#endif
	{
		//printk(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>dev %s set_default\n", dev->name);
		//memset(priv->pmib, 0, sizeof(struct wifi_mib));
		//set_mib_default(priv);

		CfgFileProc(dev);
	}
#endif

// register iw_handler - chris 2010/02
#ifdef CONFIG_RTL_COMAPI_WLTOOLS
	dev->wireless_handlers = (struct iw_handler_def *) &rtl8192cd_iw_handler_def;
#endif

	memcpy((void *)dev->dev_addr, priv->pmib->dot11OperationEntry.hwaddr, 6);

#ifdef WDS
	if (dev->base_addr == 0)
	{
#ifdef BR_SHORTCUT
		extern struct net_device *cached_wds_dev;
		cached_wds_dev = NULL;
#endif

		netif_start_queue(dev);
		return 0;
	}
#endif

#ifdef CONFIG_RTK_MESH
	if (dev->base_addr == 1) {
		netif_start_queue(dev);
		return 0;
	}
#endif // CONFIG_RTK_MESH

#ifdef PCIE_POWER_SAVING
		if((priv->pwr_state == L1) || (priv->pwr_state == L2)) {
			PCIeWakeUp(priv, (POWER_DOWN_T0<<3));
		}
#endif		

	// stop h/w in the very beginning
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{			
#ifdef  CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {
	        BOOLEAN     bVal;
        
	        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_MAC_IO_ENABLE, &bVal);
	        if ( bVal ) {
                if (RT_STATUS_SUCCESS == GET_HAL_INTERFACE(priv)->StopHWHandler(priv)) {
                    printk("StopHW Succeed\n");
                }
                else {
                    GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);            
                    errorFlag |= DRV_ER_CLOSE_STOP_HW;
                    GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);                     
                    panic_printk("StopHW Failed\n");
                }
	        }
	        else {
                //printk("%s(%d), Can't write MACID register\n", __FUNCTION__, __LINE__);
	        }
		} else
#endif  //CONFIG_WLAN_HAL
		{
	        if ( check_MAC_IO_Enable(priv) ) {
#if	defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT)
			if((GET_CHIP_VER(priv)==VERSION_8812E)||(GET_CHIP_VER(priv)==VERSION_8192C) 
				|| (GET_CHIP_VER(priv)==VERSION_8188C) || (GET_CHIP_VER(priv)==VERSION_8192D)
				|| (GET_CHIP_VER(priv) == VERSION_8188E)) 
			{
				RTL_W8(CR,0);
				RTL_W8(SYS_FUNC_EN+1, RTL_R8(SYS_FUNC_EN+1) & 0xfe);
				RTL_W8(SYS_FUNC_EN+1, RTL_R8(SYS_FUNC_EN+1) |1);				
				printk("\n[%s %d]Reset PCIE interface when reinitone \n\n", __FUNCTION__, __LINE__);	
			}
#endif						
	            rtl8192cd_stop_hw(priv);
	        }
		}

	}

#ifdef UNIVERSAL_REPEATER
	// If vxd interface, see if some mandatory mib is set. If ok, backup these
	// mib, and copy all mib from root interface. Then, restore the backup mib
	// to current.

	if (IS_VXD_INTERFACE(priv)) {
		DEBUG_INFO("Open request from vxd\n");
		if (!IS_DRV_OPEN(GET_ROOT_PRIV(priv))) {
			printk("Open vxd error! Root interface should be opened in advanced.\n");
			return 0;
		}

#ifdef PCIE_POWER_SAVING
		if((GET_ROOT_PRIV(priv)->pwr_state == L1) || (GET_ROOT_PRIV(priv)->pwr_state == L2)) {
			PCIeWakeUp(GET_ROOT_PRIV(priv), (POWER_DOWN_T0<<3));
		}
#endif	

		if (!(priv->drv_state & DRV_STATE_VXD_INIT)) {
// Mark following code. MIB copy will be executed through ioctl -------------
#if 0
			unsigned char tmpbuf[36];
			int len, encyption, is_1x, mac_clone, nat25;
			struct Dot11RsnIE rsnie;

			len = SSID_LEN;
			memcpy(tmpbuf, SSID, len);
			encyption = priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm;
			is_1x = IEEE8021X_FUN;
			mac_clone = priv->pmib->ethBrExtInfo.macclone_enable;
			nat25 = priv->pmib->ethBrExtInfo.nat25_disable;
			memcpy((char *)&rsnie, (char *)&priv->pmib->dot11RsnIE, sizeof(rsnie));

			memcpy(priv->pmib, GET_ROOT_PRIV(priv)->pmib, sizeof(struct wifi_mib));

			SSID_LEN = len;
			memcpy(SSID, tmpbuf, len);
			SSID2SCAN_LEN = len;
			memcpy(SSID2SCAN, SSID, len);
			priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = encyption;
			IEEE8021X_FUN = is_1x;
			priv->pmib->ethBrExtInfo.macclone_enable = mac_clone;
			priv->pmib->ethBrExtInfo.nat25_disable = nat25;
			memcpy((char *)&priv->pmib->dot11RsnIE, (char *)&rsnie, sizeof(rsnie));
#ifdef WDS
			// always disable wds in vxd
			priv->pmib->dot11WdsInfo.wdsEnabled = 0;
			priv->pmib->dot11WdsInfo.wdsPure = 0;
#endif

			// if root interface is AP mode, set infra-client in vxd
			// if root interfeace is a infra-client, set AP in vxd
			if (OPMODE & WIFI_AP_STATE) {
				OPMODE = WIFI_STATION_STATE;
#if defined(WIFI_WMM) && defined(WMM_APSD)
				APSD_ENABLE = 0;
#endif
				DEBUG_INFO("Set vxd as an infra-client\n");
			}
			else if (OPMODE & WIFI_STATION_STATE) {
				OPMODE = WIFI_AP_STATE;
				priv->auto_channel = 0;
				DEBUG_INFO("Set vxd as an AP\n");
			}
			else {
				DEBUG_ERR("Invalid opmode for vxd!\n");
				return 0;
			}
#endif
//---------------------------------------------------------- david+2008-03-17

			// correct RSN IE will be set later for WPA/WPA2
#ifdef CHECK_HANGUP
			if (!is_reset)
#endif
				memset(&priv->pmib->dot11RsnIE, 0, sizeof(struct Dot11RsnIE));

#ifdef WDS
			// always disable wds in vxd
			priv->pmib->dot11WdsInfo.wdsEnabled = 0;
			priv->pmib->dot11WdsInfo.wdsPure = 0;
#endif

#ifdef CONFIG_RTK_MESH
			// always disable mesh in vxd (for dev)
			GET_MIB(priv)->dot1180211sInfo.mesh_enable = 0;
#endif // CONFIG_RTK_MESH
			priv->drv_state |= DRV_STATE_VXD_INIT;	// indicate the mib of vxd driver has been initialized
		}
	}
#endif // UNIVERSAL_REPEATER

#ifdef CHECK_HANGUP
	if (!is_reset)
#endif
	{
#ifdef P2P_SUPPORT
		int p2p_support_mode=0;
		if (OPMODE & WIFI_P2P_SUPPORT)
			p2p_support_mode=1;	
#endif
		if (OPMODE & WIFI_AP_STATE) {
			OPMODE = WIFI_AP_STATE;
		}
#ifdef CLIENT_MODE
		else if (OPMODE & WIFI_STATION_STATE) {
			OPMODE = WIFI_STATION_STATE;
		} else if (OPMODE & WIFI_ADHOC_STATE) {
			OPMODE = WIFI_ADHOC_STATE;
#if defined(WIFI_WMM) && defined(WMM_APSD)
			APSD_ENABLE = 0;
#endif
		}
#endif
		else {
			printk("Undefined state... using AP mode as default\n");
			OPMODE = WIFI_AP_STATE;
		}

#ifdef P2P_SUPPORT
		if(p2p_support_mode)
			OPMODE |= WIFI_P2P_SUPPORT;
#endif		
	}

#if defined(UNIVERSAL_REPEATER) && defined(CLIENT_MODE)
	if (IS_VXD_INTERFACE(priv) &&
		((GET_MIB(GET_ROOT_PRIV(priv)))->dot11OperationEntry.opmode & WIFI_STATION_STATE)) {
		if (!chklink_wkstaQ(GET_ROOT_PRIV(priv))) {
			printk("Root interface does not link yet!\n");
			return 0;
		}
	}
#endif

#ifdef WIFI_WMM
#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8192D) 
			if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY && priv->pmib->dot11OperationEntry.wifi_specific)
				priv->pshare->rf_ft_var.wifi_beq_iot = 1;
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8188E)
		if (priv->pmib->dot11OperationEntry.wifi_specific)
			priv->pshare->rf_ft_var.wifi_beq_iot = 1;
#endif
#endif

#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
	{
		if (IS_VAP_INTERFACE(priv)) {
			if (!IS_DRV_OPEN(GET_ROOT_PRIV(priv))) {
				printk("Open vap error! Root interface should be opened in advanced.\n");
				return -1;
			}

			if ((GET_ROOT_PRIV(priv)->pmib->dot11OperationEntry.opmode & WIFI_AP_STATE) == 0) {
				printk("Fail to open VAP under non-AP mode!\n");
				return -1;
			}
			else {
#ifdef CONFIG_RTL8672
				do {
					if (GET_ROOT_PRIV(priv)->pmib->dot11OperationEntry.opmode & WIFI_WAIT_FOR_CHANNEL_SELECT) {
						DEBUG_INFO("wait for root interface ss_timer!!\n");
						delay_ms(1);
					}
					else {
						DEBUG_INFO("channel=%x\n", GET_ROOT_PRIV(priv)->pmib->dot11RFEntry.dot11channel);
						break;
					}
				}while(1);
#endif
				rtl8192cd_init_vap_mib(priv);
			}
		}
	}
#endif

// check phyband and channel match or not
#ifdef MP_TEST
	if(priv->pshare->rf_ft_var.mp_specific) //For MP nfjrom to open WLAN0 successfully
	{
		if ((priv->pmib->dot11RFEntry.dot11channel <= 14) && (priv->pmib->dot11RFEntry.phyBandSelect != PHY_BAND_2G))
		{
			priv->pmib->dot11BssType.net_work_type = WIRELESS_11A | WIRELESS_11N;
			priv->pmib->dot11RFEntry.phyBandSelect = PHY_BAND_5G;
			priv->pmib->dot11RFEntry.dot11channel = 36;
		}
		else if((priv->pmib->dot11RFEntry.dot11channel > 14) && (priv->pmib->dot11RFEntry.phyBandSelect != PHY_BAND_5G))
		{
			priv->pmib->dot11BssType.net_work_type = WIRELESS_11B | WIRELESS_11G | WIRELESS_11N;
			priv->pmib->dot11RFEntry.phyBandSelect = PHY_BAND_2G;
			priv->pmib->dot11RFEntry.dot11channel = 1;
		}

		if((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C))
		{
			priv->pmib->dot11BssType.net_work_type = WIRELESS_11B | WIRELESS_11G | WIRELESS_11N;
			priv->pmib->dot11RFEntry.phyBandSelect = PHY_BAND_2G;
			priv->pmib->dot11RFEntry.dot11channel = 1;
		}

        // TODO: Should we add some code here??  By Filen
	} 
#endif

#if defined(BR_SHORTCUT)
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if(IS_ROOT_INTERFACE(priv))
#endif
		clear_shortcut_cache();
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
		priv->pshare->current_num_tx_desc = NUM_TX_DESC;

#if defined (CONFIG_RTL_92D_SUPPORT) && defined(CONFIG_RTL_92D_DMDP)
		if ((GET_CHIP_VER(priv) == VERSION_8192D) && 
			(priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY)) 
			priv->pshare->current_num_tx_desc = (NUM_TX_DESC>MAX_NUM_TX_DESC_DMDP)?
				(MAX_NUM_TX_DESC_DMDP):(NUM_TX_DESC);
#endif
	}

	rc = rtl8192cd_init_sw(priv);
	if (rc) {
		printk("ERROR : rtl8192cd_init_sw failure\n");
        return rc;
	}

#if defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8196C_EC)
	if (!priv->auto_channel) {
		LOG_START_MSG();
	}
#endif
//#ifdef CONFIG_RTL865X_AC
#if defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
	if (!priv->auto_channel) {
		LOG_START_MSG();
	}
#endif

	validate_fixed_tx_rate(priv);

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#ifdef __KERNEL__
#ifdef CHECK_HANGUP
		if (!is_reset)
#endif
		{
#if !defined(CONFIG_RTL8671)
	#if defined(__LINUX_2_6__)
		#if defined(CONFIG_RTL_92D_DMDP) || defined(NOT_RTK_BSP)
			rc = request_irq(dev->irq, rtl8192cd_interrupt, IRQF_SHARED|IRQF_DISABLED, dev->name, dev);
		#else
			rc = request_irq(dev->irq, rtl8192cd_interrupt, IRQF_DISABLED, dev->name, dev);
		#endif
	#else
			rc = request_irq(dev->irq, rtl8192cd_interrupt, SA_SHIRQ, dev->name, dev);
	#endif
#else
			rc = request_irq(dev->irq, rtl8192cd_interrupt, SA_INTERRUPT, dev->name, dev);
#endif

#if defined(PCIE_POWER_SAVING) && defined(GPIO_WAKEPIN)
			rc |= request_irq_for_wakeup_pin(dev);
#endif

			if (rc) {
				DEBUG_ERR("some issue in request_irq, rc=%d\n", rc);
			}
		}
#endif

		SAVE_INT_AND_CLI(x);
		SMP_LOCK(x);

#ifdef CONFIG_RTL865X_WTDOG
#if !(defined(CONFIG_RTL8196B) || defined(CONFIG_RTL_819X))
		wtval = *((volatile unsigned long *)0xB800311C);
		*((volatile unsigned long *)0xB800311C) = 0xA5f00000;
#endif
#endif

do_hw_init:

#ifdef EN_EFUSE
		{
			int i, readEfuse=0;
			for(i=0;i<MAX_2G_CHANNEL_NUM;i++)	{
				if(	priv->pmib->dot11RFEntry.pwrlevelCCK_A[i]==0 
				||	priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] ==0) {
					readEfuse = 1;
					break;
				}
			}
			if(readEfuse)
				ReadTxPowerInfoFromHWPG(priv);

			if(priv->pmib->dot11RFEntry.ther==0) {
				ReadThermalMeterFromEfuse(priv);

#ifdef CONFIG_RTL_92D_SUPPORT
				if (GET_CHIP_VER(priv) == VERSION_8192D) {
					ReadCrystalCalibrationFromEfuse(priv);
					ReadDeltaValFromEfuse(priv);
					ReadTRSWPAPEFromEfuse(priv);
				}
#endif			
			}
		}
#endif


#if defined(CONFIG_AUTO_PCIE_PHY_SCAN) && (defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_819XD))
#ifdef CONFIG_RTL_88E_SUPPORT
		if(GET_CHIP_VER(priv)==VERSION_8188E)
		{
		if ((REG32(0xb8000008)&0x2000000)==0x2000000)  //40MHz
		{
			printk("\n\n 88E 40M TEST 0x11=0x5b \n\n");
			
			//RTL_W8(0x11, 0x5b);
			//RTL_W8(0x2c, ((RTL_R8(0x2c) & 0xf0) | 0x1));
		}
		else //25MHz
		{
			printk("\n\n 88E 25M TEST 0x11=0x5b, 0x2c[0:3]=0x1 \n\n");
			
			//RTL_W8(0x11, 0x5b);
			RTL_W8(0x2c, ((RTL_R8(0x2c) & 0xf0) | 0x1));
		}
		}
#endif
#endif
#if !defined(CONFIG_AUTO_PCIE_PHY_SCAN) && !defined(CONFIG_PHY_EAT_40MHZ) && defined(CONFIG_RTL_8197DL)
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8188E) {
			//25MHz
			printk("\n\n 88E 25M TEST 0x11=0x5b, 0x2c[0:3]=0x1 \n\n");
			
			//RTL_W8(0x11, 0x5b);
			RTL_W8(0x2c, ((RTL_R8(0x2c) & 0xf0) | 0x1));
		}
#endif
#endif

		rc = rtl8192cd_init_hw_PCI(priv);
		//delay_ms(200);		// TODO: need refinement, for 98 watchdog time out

		// write IDR0, IDR4 here
		{
#ifdef  CONFIG_WLAN_HAL
			if (IS_HAL_CHIP(priv)) {
        	    GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_ETHER_ADDR, (pu1Byte)dev->dev_addr);
			} else
#endif
			{
				unsigned long reg = 0;
				reg = *(unsigned long *)(dev->dev_addr);
	//			RTL_W32(IDR0, (cpu_to_le32(reg)));
				RTL_W32(MACID, (cpu_to_le32(reg)));
				reg = *(unsigned short *)((unsigned long)dev->dev_addr + 4);
	//			RTL_W32(IDR4, (cpu_to_le32(reg)));
				RTL_W16(MACID+4, (cpu_to_le16(reg)));
	#ifdef CONFIG_RTL_92D_SUPPORT
				if (GET_CHIP_VER(priv) == VERSION_8192D) {
					RTL_W32(MACID1 , RTL_R32(MACID));
					RTL_W16(MACID1+4, RTL_R16(MACID+4));
				}
	#endif
			}
		}

		if (rc && ++init_hw_cnt < 5) {
#ifndef NOT_RTK_BSP
#if defined(CONFIG_RTL865X_WTDOG) || (defined(CONFIG_RTL_WTDOG) && defined(CONFIG_RTL_92D_SUPPORT))
		if (GET_CHIP_VER(priv)==VERSION_8192D) {
#ifdef CONFIG_RTL_8198B
			REG32(BSP_WDTCNTRR) |= BSP_WDT_KICK;
#else
			*((volatile unsigned long *)0xB800311C) |=  1 << 23;
#endif
		}
#endif
#endif
			goto do_hw_init;
		}

#if defined(CONFIG_RTL865X_WTDOG) || (defined(CONFIG_RTL_WTDOG) && defined(CONFIG_RTL_92D_SUPPORT))
if (GET_CHIP_VER(priv)==VERSION_8192D) {
#if !(defined(CONFIG_RTL8196B) || defined(CONFIG_RTL_819X))
		*((volatile unsigned long *)0xB800311C) |=  1 << 23;
		*((volatile unsigned long *)0xB800311C) = wtval;
#endif
}
#endif

		RESTORE_INT(x);
		SMP_UNLOCK(x);

		if (rc) {
			DEBUG_ERR("init hw failed!\n");
#ifdef CONFIG_RTL_92D_SUPPORT
			if (GET_CHIP_VER(priv)==VERSION_8192D){
				RTL_W8(RSV_MAC0_CTRL, RTL_R8(RSV_MAC0_CTRL)&(~MAC0_EN));
				RTL_W8(RSV_MAC1_CTRL, RTL_R8(RSV_MAC1_CTRL)&(~MAC1_EN));
			}
#endif //CONFIG_RTL_92D_SUPPORT
			force_stop_wlan_hw();
#ifdef CONFIG_RTL_92D_SUPPORT
			if (GET_CHIP_VER(priv)!=VERSION_8192D)	/* do not trigger wtdog if 92D open fail */
#endif
			{
#ifdef LINUX_2_6_21_
			local_irq_disable();
#else
			cli();
#endif
#ifndef NOT_RTK_BSP
#ifdef CONFIG_RTL_8198B
			REG32(BSP_WDTCTRLR) = BSP_WDT_ENABLE;
#else
			*(volatile unsigned long *)(0xB800311c) = 0; /* enable watchdog reset now */
#endif
			for(;;)
				;
#endif
			}
			return rc;
		}
	}
#if 0 // defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	else {
		if (get_rf_mimo_mode(priv) == MIMO_1T1R)
			GET_MIB(priv)->dot11nConfigEntry.dot11nSupportedMCS &= 0x00ff;
	}
#endif

#ifdef MBSSID
	if ((OPMODE & WIFI_AP_STATE) && (GET_ROOT(priv)->pmib->miscEntry.vap_enable)) {
#ifdef  CONFIG_WLAN_HAL
			if (IS_HAL_CHIP(priv))
            	GET_HAL_INTERFACE(priv)->InitMBSSIDHandler(priv);
			else
#endif
			rtl8192cd_init_mbssid(priv);

		if (IS_VAP_INTERFACE(priv)) {
			// set BcnDmaInt & BcnOk of different VAP in IMR
#ifdef  CONFIG_WLAN_HAL
			if (IS_HAL_CHIP(priv)) {
	            GET_HAL_INTERFACE(priv)->InitVAPIMRHandler(priv, priv->vap_init_seq);
			} else
#endif
			{
#ifdef CONFIG_RTL_88E_SUPPORT
			if (GET_CHIP_VER(priv) == VERSION_8188E) {
				priv->pshare->InterruptMaskExt |= (HIMRE_88E_BCNDMAINT1 << (priv->vap_init_seq-1));
				RTL_W32(REG_88E_HIMRE, priv->pshare->InterruptMaskExt);
			} else
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
			if (GET_CHIP_VER(priv) == VERSION_8812E){
				priv->pshare->InterruptMaskExt |= (HIMRE_92E_BCNDMAINT1 << (priv->vap_init_seq-1));
				RTL_W32(REG_HIMR1_8812, priv->pshare->InterruptMaskExt);
			} else
#endif
			{
				priv->pshare->InterruptMask |= (HIMR_BCNDMA1 << (priv->vap_init_seq-1));
				RTL_W32(HIMR, priv->pshare->InterruptMask);
			}
			}

			if (GET_ROOT_PRIV(priv)->auto_channel == 0) {
				priv->pmib->dot11RFEntry.dot11channel = GET_ROOT_PRIV(priv)->pmib->dot11RFEntry.dot11channel;
				priv->ht_cap_len = 0;	// re-construct HT IE

				init_beacon(priv);
			}
		}
	}
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
    if(IS_ROOT_INTERFACE(priv))
#endif //defined(UNIVERSAL_REPEATER) || defined(MBSSID)
    {
#ifdef CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv))	{
	        u1Byte HIQ_En=0;
	        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_HIQ_NO_LMT_EN, &HIQ_En);         
	        HIQ_En = HIQ_En | BIT0;
	        GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_HIQ_NO_LMT_EN, &HIQ_En);
		}
#endif //CONFIG_WLAN_HAL
    }


// new added to reset keep_rsnie flag
	if (priv->pmib->dot11OperationEntry.keep_rsnie)
		priv->pmib->dot11OperationEntry.keep_rsnie = 0;
//------------------- david+2006-06-30

priv->drv_state |= DRV_STATE_OPEN;      // set driver as has been opened, david


#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
    	GET_HAL_INTERFACE(priv)->EnableIMRHandler(priv);
	}
#endif

#if defined(RESERVE_TXDESC_FOR_EACH_IF) && (defined(UNIVERSAL_REPEATER) || defined(MBSSID))
	recalc_txdesc_limit(priv);
#endif

	//memcpy((void *)dev->dev_addr, priv->pmib->dot11OperationEntry.hwaddr, 6);

	// below is for site_survey timer
#ifdef __KERNEL__
	init_timer(&priv->ss_timer);
	priv->ss_timer.data = (unsigned long) priv;
	priv->ss_timer.function = rtl8192cd_ss_timer;
#elif defined(__ECOS)
	init_timer(&priv->ss_timer, (unsigned long)priv, rtl8192cd_ss_timer);
#endif


#ifdef P2P_SUPPORT
	init_timer(&priv->p2p_listen_timer_t);
	init_timer(&priv->p2p_search_timer_t);		
	priv->p2p_search_timer_t.data = (unsigned long) priv;
	priv->p2p_search_timer_t.function = p2p_search_timer;	
	priv->p2p_listen_timer_t.data = (unsigned long) priv;
	priv->p2p_listen_timer_t.function = P2P_listen_timer;
#endif
#ifdef CONFIG_RTL_COMAPI_WLTOOLS
	init_waitqueue_head(&priv->ss_wait);
#endif

#ifdef CLIENT_MODE
#ifdef __KERNEL__
	init_timer(&priv->reauth_timer);
	priv->reauth_timer.data = (unsigned long) priv;
	priv->reauth_timer.function = rtl8192cd_reauth_timer;
#elif defined(__ECOS)
	init_timer(&priv->reauth_timer, (unsigned long)priv, rtl8192cd_reauth_timer);
#endif

#ifdef __KERNEL__
	init_timer(&priv->reassoc_timer);
	priv->reassoc_timer.data = (unsigned long) priv;
	priv->reassoc_timer.function = rtl8192cd_reassoc_timer;
#elif defined(__ECOS)
	init_timer(&priv->reassoc_timer, (unsigned long)priv, rtl8192cd_reassoc_timer);
#endif

#ifdef __KERNEL__
	init_timer(&priv->idle_timer);
	priv->idle_timer.data = (unsigned long) priv;
	priv->idle_timer.function = rtl8192cd_idle_timer;
#ifdef DFS
	init_timer(&priv->dfs_cntdwn_timer);
	priv->dfs_cntdwn_timer.data = (unsigned long) priv;
	priv->dfs_cntdwn_timer.function = rtl8192cd_dfs_cntdwn_timer;
#endif
#elif defined(__ECOS)
	init_timer(&priv->idle_timer, (unsigned long)priv, rtl8192cd_idle_timer);
#ifdef DFS
	init_timer(&priv->dfs_cntdwn_timer, (unsigned long)priv, rtl8192cd_dfs_cntdwn_timer);
#endif
#endif	
#endif

	priv->frag_to = 0;
#ifdef __KERNEL__
	init_timer(&priv->frag_to_filter);
	priv->frag_to_filter.expires = jiffies + FRAG_TO;
	priv->frag_to_filter.data = (unsigned long) priv;
	priv->frag_to_filter.function = rtl8192cd_frag_timer;
#elif defined(__ECOS)
	init_timer(&priv->frag_to_filter,(unsigned long)priv, rtl8192cd_frag_timer);
#endif
	mod_timer(&priv->frag_to_filter, jiffies + FRAG_TO);

	priv->auth_to = AUTH_TO / HZ;
	priv->assoc_to = ASSOC_TO / HZ;

#ifdef PCIE_POWER_SAVING_DEBUG
	priv->expire_to = 60;
#else
	priv->expire_to = (EXPIRETIME > 100)? (EXPIRETIME / 100) : 86400; /*10ms to 1s*/
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#if defined(DETECT_STA_EXISTANCE) && (defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT))
#ifdef __KERNEL__
		// Added by Annie for Retry Limit Recovery Timer, 2010-08-10.
		init_timer(&priv->pshare->rl_recover_timer);
		priv->pshare->rl_recover_timer.data = (unsigned long) priv;
		priv->pshare->rl_recover_timer.function = RetryLimitRecovery;
#elif defined(__ECOS)
		init_timer(&priv->pshare->rl_recover_timer,(unsigned long)priv, RetryLimitRecovery);
#endif
		priv->pshare->bRLShortened = FALSE;
#endif
		
#ifdef PCIE_POWER_SAVING
#ifdef __KERNEL__
		init_timer(&priv->ps_timer);
		priv->ps_timer.expires = jiffies + POWER_DOWN_T0;
		priv->ps_timer.data = (unsigned long) priv;
		priv->ps_timer.function = PCIe_power_save_timer;
#elif defined(__ECOS)
		init_timer(&priv->ps_timer, (unsigned long)priv, PCIe_power_save_timer);
#endif
		mod_timer(&priv->ps_timer, jiffies + POWER_DOWN_T0);
//		priv->ps_ctrl = 0x11;
#endif

#ifdef __KERNEL__
		init_timer(&priv->expire_timer);
		priv->expire_timer.expires = jiffies + EXPIRE_TO;
		priv->expire_timer.data = (unsigned long) priv;
		priv->expire_timer.function = rtl8192cd_1sec_timer;
#ifdef 	SW_ANT_SWITCH
		init_timer(&priv->pshare->swAntennaSwitchTimer);
		priv->pshare->swAntennaSwitchTimer.data = (unsigned long) priv;
		priv->pshare->swAntennaSwitchTimer.function = dm_SW_AntennaSwitchCallback;
#endif
#elif defined(__ECOS)
		init_timer(&priv->expire_timer, (unsigned long)priv, rtl8192cd_1sec_timer);
#endif
		mod_timer(&priv->expire_timer, jiffies + EXPIRE_TO);

#ifdef __KERNEL__
		init_timer(&priv->pshare->rc_sys_timer);
		priv->pshare->rc_sys_timer.data = (unsigned long) priv;
		priv->pshare->rc_sys_timer.function = reorder_ctrl_timeout;
#elif defined(__ECOS)
		init_timer(&priv->pshare->rc_sys_timer, (unsigned long)priv, reorder_ctrl_timeout);
#endif
		priv->pshare->rc_timer_tick = priv->pmib->reorderCtrlEntry.ReorderCtrlTimeout / RTL_JIFFIES_TO_MICROSECOND;
		if (priv->pshare->rc_timer_tick == 0)
			priv->pshare->rc_timer_tick = 1;

#if 0
		init_timer(&priv->pshare->phw->tpt_timer);
		priv->pshare->phw->tpt_timer.data = (unsigned long)priv;
		priv->pshare->phw->tpt_timer.function = rtl8192cd_tpt_timer;
#endif
#if defined(CONFIG_RTL_92D_SUPPORT) && defined(DPK_92D)
		if (GET_CHIP_VER(priv) == VERSION_8192D){
			init_timer(&priv->pshare->DPKTimer);
			priv->pshare->DPKTimer.data = (unsigned long) priv;
			priv->pshare->DPKTimer.function = rtl8192cd_DPK_timer;
		}
#endif

	}

#ifdef __KERNEL__
#ifdef SMART_REPEATER_MODE
	if (IS_ROOT_INTERFACE(priv)) {
		init_timer(&priv->pshare->check_vxd_ap);
		priv->pshare->check_vxd_ap.expires = jiffies + CHECK_VXD_AP_TIMEOUT;
		priv->pshare->check_vxd_ap.data = (unsigned long) priv;
		priv->pshare->check_vxd_ap.function = check_vxd_ap_timer;
		if (OPMODE == WIFI_AP_STATE)		
			mod_timer(&priv->pshare->check_vxd_ap, jiffies + CHECK_VXD_AP_TIMEOUT);
	}
#endif

	// for MIC check
	init_timer(&priv->MIC_check_timer);
	priv->MIC_check_timer.data = (unsigned long) priv;
	priv->MIC_check_timer.function = DOT11_Process_MIC_Timerup;
	init_timer(&priv->assoc_reject_timer);
	priv->assoc_reject_timer.data = (unsigned long) priv;
	priv->assoc_reject_timer.function = DOT11_Process_Reject_Assoc_Timerup;
#elif defined(__ECOS)
	init_timer(&priv->MIC_check_timer, (unsigned long)priv, DOT11_Process_MIC_Timerup);
	init_timer(&priv->assoc_reject_timer, (unsigned long)priv, DOT11_Process_Reject_Assoc_Timerup);
#ifdef SMART_REPEATER_MODE
	if (IS_ROOT_INTERFACE(priv)) {
		init_timer(&priv->pshare->check_vxd_ap, (unsigned long)priv, check_vxd_ap_timer);
		if (OPMODE == WIFI_AP_STATE)
			mod_timer(&priv->pshare->check_vxd_ap, jiffies + CHECK_VXD_AP_TIMEOUT);
	}
#endif
#endif
	priv->MIC_timer_on = FALSE;
	priv->assoc_reject_on = FALSE;

#ifdef GBWC
	init_timer(&priv->GBWC_timer);
	priv->GBWC_timer.data = (unsigned long) priv;
	priv->GBWC_timer.function = rtl8192cd_GBWC_timer;
	if (priv->pmib->gbwcEntry.GBWCMode != GBWC_MODE_DISABLE)
		mod_timer(&priv->GBWC_timer, jiffies + GBWC_TO);
#endif

#ifdef __KERNEL__
	// to avoid add RAtid fail
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)
	init_timer(&priv->add_RATid_timer);
	priv->add_RATid_timer.data = (unsigned long) priv;
	priv->add_RATid_timer.function = add_RATid_timer;

	init_timer(&priv->add_rssi_timer);
	priv->add_rssi_timer.data = (unsigned long) priv;
	priv->add_rssi_timer.function = add_rssi_timer;
#endif
	init_timer(&priv->add_ps_timer);
	priv->add_ps_timer.data = (unsigned long) priv;
	priv->add_ps_timer.function = add_ps_timer;
#elif defined(__ECOS)
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)
	init_timer(&priv->add_RATid_timer, (unsigned long)priv, add_RATid_timer);
	init_timer(&priv->add_rssi_timer, (unsigned long)priv, add_rssi_timer);
#endif
	init_timer(&priv->add_ps_timer, (unsigned long)priv, add_ps_timer);
#endif

#if defined(CONFIG_RTL_92D_SUPPORT) && defined(CONFIG_RTL_NOISE_CONTROL)
	if (GET_CHIP_VER(priv) == VERSION_8192D){
		init_timer(&priv->dnc_timer);
		priv->dnc_timer.data = (unsigned long) priv;
		priv->dnc_timer.function = dnc_timer;
	}
#endif

#ifdef SMART_CONCURRENT_92D
	if (GET_CHIP_VER(priv) == VERSION_8192D){
		init_timer(&priv->smcc_prb_timer);
		priv->smcc_prb_timer.data = (unsigned long) priv;
		priv->smcc_prb_timer.function = smcc_prb_timer;
	}

#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
		// for HW/SW LED
		if ((LED_TYPE >= LEDTYPE_HW_TX_RX) && (LED_TYPE <= LEDTYPE_HW_LINKACT_INFRA))
			enable_hw_LED(priv, LED_TYPE);
		else if ((LED_TYPE >= LEDTYPE_SW_LINK_TXRX) && (LED_TYPE < LEDTYPE_SW_MAX)) {
			if (LED_TYPE == LEDTYPE_SW_RESERVED)
				LED_TYPE = LEDTYPE_SW_LED2_GPIO10_LINKTXRX_92D;

			if ((LED_TYPE == LEDTYPE_SW_LINK_TXRX) ||
				(LED_TYPE <= LEDTYPE_SW_LINKTXRX) ||
				(LED_TYPE == LEDTYPE_SW_LED2_GPIO8_LINKTXRX) ||
				(LED_TYPE == LEDTYPE_SW_LED2_GPIO10_LINKTXRX) ||
				(LED_TYPE == LEDTYPE_SW_LED1_GPIO9_LINKTXRX_92D) ||
				(LED_TYPE == LEDTYPE_SW_LED2_GPIO10_LINKTXRX_92D))
				priv->pshare->LED_cnt_mgn_pkt = 1;

			enable_sw_LED(priv, 1);
		}

#ifdef CONFIG_RTL_ULINKER
		{
			extern void enable_sys_LED(struct rtl8192cd_priv *priv);
			enable_sys_LED(priv);
		}
#endif
		
#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
#ifdef _OUTSRC_COEXIST
	if(!IS_OUTSRC_CHIP(priv))
#endif
	{
#ifdef SW_ANT_SWITCH
		dm_SW_AntennaSwitchInit(priv);	// SW Ant Switch use LED pin to control TRX Antenna
#endif
#if defined(HW_ANT_SWITCH)
		dm_HW_AntennaSwitchInit(priv);
#endif
	}
#endif

#ifdef DFS
		if (!priv->pmib->dot11DFSEntry.disable_DFS &&
			(OPMODE & WIFI_AP_STATE) &&
			(((priv->pmib->dot11RFEntry.dot11channel >= 52) &&
			(priv->pmib->dot11RFEntry.dot11channel <= 64)) || 
			((priv->pmib->dot11RFEntry.dot11channel >= 100) &&
			(priv->pmib->dot11RFEntry.dot11channel <= 140)))) {
			init_timer(&priv->ch_avail_chk_timer);
			priv->ch_avail_chk_timer.data = (unsigned long) priv;
			priv->ch_avail_chk_timer.function = rtl8192cd_ch_avail_chk_timer;
			mod_timer(&priv->ch_avail_chk_timer, jiffies + CH_AVAIL_CHK_TO);

			init_timer(&priv->DFS_timer);
			priv->DFS_timer.data = (unsigned long) priv;
			priv->DFS_timer.function = rtl8192cd_DFS_timer;

			/* DFS activated after 5 sec; prevent switching channel due to DFS false alarm */
			mod_timer(&priv->DFS_timer, jiffies + RTL_SECONDS_TO_JIFFIES(5));

			DFS_SetReg(priv);
		}
#endif

#ifdef SUPPORT_SNMP_MIB
		mib_init(priv);
#endif
#if defined(CONFIG_RTL_8196E)	 
   	    if(sys_bonding_type() == BOND_8196ES ) 
    		rtl_8196es_gpio_init(); 		
#endif	
	}

#if defined(BR_SHORTCUT) && defined(CLIENT_MODE)
	if (OPMODE & WIFI_STATION_STATE) {
		extern struct net_device *cached_sta_dev;
		cached_sta_dev = NULL;
	}
#endif

#if	defined(BR_SHORTCUT) && defined(RTL_CACHED_BR_STA)
	{
		extern unsigned char cached_br_sta_mac[MACADDRLEN];
		extern struct net_device *cached_br_sta_dev;
		memset(cached_br_sta_mac, 0, MACADDRLEN);
		cached_br_sta_dev = NULL;
	}
#endif

	//if (OPMODE & WIFI_AP_STATE)  //in case of station mode, queue will start only after assoc.
		netif_start_queue(dev);		// start queue always
#ifdef WDS
	create_wds_tbl(priv);
#endif

#ifdef CHECK_HANGUP
	if (priv->reset_hangup)
		priv->reset_hangup = 0;
#endif

#if defined(INCLUDE_WPA_PSK) && defined(CLIENT_MODE)
	if (OPMODE & WIFI_ADHOC_STATE)
		if (priv->pmib->dot1180211AuthEntry.dot11EnablePSK)
			ToDrv_SetGTK(priv);
#endif

#ifdef MBSSID
	if (IS_ROOT_INTERFACE(priv))
#endif
	if ((OPMODE & WIFI_AP_STATE) && priv->auto_channel) {
		if (((priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _TKIP_PRIVACY_) &&
			  (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _CCMP_PRIVACY_) &&
			  (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _WEP_WPA_MIXED_PRIVACY_)) ||
			 (priv->pmib->dot11RsnIE.rsnielen > 0)) {
			priv->ss_ssidlen = 0;
			DEBUG_INFO("start_clnt_ss, trigger by %s, ss_ssidlen=0\n", (char *)__FUNCTION__);
#ifdef CONFIG_RTL8672
			OPMODE |= WIFI_WAIT_FOR_CHANNEL_SELECT;
#endif
			start_clnt_ss(priv);
		}
	}

#ifdef CLIENT_MODE
	if (OPMODE & (WIFI_STATION_STATE | WIFI_ADHOC_STATE)) {
#ifdef RTK_BR_EXT
#ifdef __ECOS
		memcpy(priv->br_mac, GET_MY_HWADDR, MACADDRLEN);
#else
		if (priv->dev->br_port) {
#ifdef __LINUX_2_6__
			memcpy(priv->br_mac, priv->dev->br_port->br->dev->dev_addr, MACADDRLEN);
#else
			memcpy(priv->br_mac, priv->dev->br_port->br->dev.dev_addr, MACADDRLEN);
#endif
		}
#endif
#endif
		if (!IEEE8021X_FUN || (IEEE8021X_FUN && (priv->pmib->dot11RsnIE.rsnielen > 0))) {
#ifdef CHECK_HANGUP
			if (!is_reset || priv->join_res == STATE_Sta_No_Bss ||
					priv->join_res == STATE_Sta_Roaming_Scan || priv->join_res == 0)
#endif
			{
#ifdef CHECK_HANGUP
				if (is_reset)
					OPMODE &= ~WIFI_SITE_MONITOR;
#endif
				start_clnt_lookup(priv, 1);
			}
		}
	}
#endif

#ifdef UNIVERSAL_REPEATER
	if (IS_ROOT_INTERFACE(priv) &&
#ifdef __ECOS
		GET_VXD_PRIV(priv) &&
#endif
		netif_running(GET_VXD_PRIV(priv)->dev)) {
		SAVE_INT_AND_CLI(x);
		rtl8192cd_open(GET_VXD_PRIV(priv)->dev);
		RESTORE_INT(x);
	}
	if (IS_VXD_INTERFACE(priv) &&
		(GET_ROOT_PRIV(priv)->pmib->dot11OperationEntry.opmode&WIFI_STATION_STATE) &&
		(GET_ROOT_PRIV(priv)->pmib->dot11OperationEntry.opmode&WIFI_ASOC_STATE) &&
#ifdef RTK_BR_EXT
		!(GET_ROOT_PRIV(priv)->pmib->ethBrExtInfo.macclone_enable && !priv->macclone_completed) &&
#endif
		!(priv->drv_state & DRV_STATE_VXD_AP_STARTED) )
		enable_vxd_ap(priv);
#endif

#ifdef MBSSID
	if (IS_ROOT_INTERFACE(priv)) {
		if (priv->pmib->miscEntry.vap_enable) {
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
				if (netif_running(priv->pvap_priv[i]->dev))
					rtl8192cd_open(priv->pvap_priv[i]->dev);
			}
		}
	}
#endif

#ifdef PCIE_POWER_SAVING
#ifdef CHECK_HANGUP
	if(!is_reset)
#endif
#ifdef MBSSID
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL8672)
		if (1)
#else
		if (REG32(REVR) == RTL8196C_REVISION_B)
#endif
			init_pcie_power_saving(priv);
		else
			priv->pshare->rf_ft_var.power_save = 0;
	}
#endif

	DBFEXIT;

	return 0;
}


int  rtl8192cd_set_hwaddr(struct net_device *dev, void *addr)
{
	unsigned long flags;
#ifdef NETDEV_NO_PRIV
	struct rtl8192cd_priv *priv = ((struct rtl8192cd_priv *)netdev_priv(dev))->wlan_priv;
#else
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
#endif
	unsigned long reg;
	unsigned char *p;
#ifdef WDS
	int i;
#endif
#ifdef  CONFIG_WLAN_HAL
    BOOLEAN     bVal;
#endif  //CONFIG_WLAN_HAL


#ifdef __KERNEL__
	p = ((struct sockaddr *)addr)->sa_data;
#else
	p = (unsigned char *)addr;
#endif

	SAVE_INT_AND_CLI(flags);
	SMP_LOCK(flags);

	memcpy(priv->dev->dev_addr, p, 6);
	memcpy(GET_MY_HWADDR, p, 6);
#ifndef __ECOS
    memset(dev->broadcast, 0xff, ETH_ALEN);
#endif
#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
	{
		if (IS_VAP_INTERFACE(priv)) {
			RESTORE_INT(flags);
			SMP_UNLOCK(flags);
			return 0;
		}
	}
#endif

#ifdef WDS
	for (i=0; i<NUM_WDS; i++)
		if (priv->wds_dev[i])
			memcpy(priv->wds_dev[i]->dev_addr, p, 6);
#endif
#ifdef CONFIG_RTK_MESH
	if(NUM_MESH>0)
		if (priv->mesh_dev)
			memcpy(priv->mesh_dev->dev_addr, p, 6);
#endif

#ifdef UNIVERSAL_REPEATER
	if (IS_ROOT_INTERFACE(priv)) {
		if (GET_VXD_PRIV(priv)) {
			memcpy(GET_VXD_PRIV(priv)->dev->dev_addr, p, 6);
			memcpy(GET_VXD_PRIV(priv)->pmib->dot11OperationEntry.hwaddr, p, 6);
		}
	}
	else if (IS_VXD_INTERFACE(priv)) {
		memcpy(GET_ROOT_PRIV(priv)->dev->dev_addr, p, 6);
		memcpy(GET_ROOT_PRIV(priv)->pmib->dot11OperationEntry.hwaddr, p, 6);
	}
#endif
#ifdef     CONFIG_WLAN_HAL 
	if (IS_HAL_CHIP(priv)) {
	    GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_MAC_IO_ENABLE, &bVal);
	    if ( bVal ) {
	        GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_ETHER_ADDR, p);        
	    }
	    else {
        printk("%s(%d): Can't write MACID register\n", __FUNCTION__, __LINE__);
	    }
	} else
#endif
	{
	    if (check_MAC_IO_Enable(priv)) {
	        reg = *(unsigned long *)(dev->dev_addr);
	        RTL_W32(MACID, (cpu_to_le32(reg)));
	        reg = *(unsigned short *)((unsigned long)dev->dev_addr + 4);
	        RTL_W16(MACID+4, (cpu_to_le16(reg)));
	#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8192D) {
			RTL_W32(MACID1 , RTL_R32(MACID));
			RTL_W16(MACID1+4, RTL_R16(MACID+4));
		}
	#endif
		}
	}
#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
	{
		if (OPMODE & WIFI_AP_STATE) {
#ifdef     CONFIG_WLAN_HAL 
			if (IS_HAL_CHIP(priv))
	            GET_HAL_INTERFACE(priv)->InitMBSSIDHandler(priv);
			else
#endif
			rtl8192cd_init_mbssid(priv);

		}
	}
#endif

	RESTORE_INT(flags);
	SMP_UNLOCK(flags);

#ifdef CLIENT_MODE
	if (!(OPMODE & WIFI_AP_STATE) && netif_running(priv->dev)) {
		int link_status = chklink_wkstaQ(priv);
		if (link_status)
			start_clnt_join(priv);
	}
#endif

	return 0;
}


int rtl8192cd_close(struct net_device *dev)
{
    unsigned int errorFlag;
#ifdef NETDEV_NO_PRIV
	struct rtl8192cd_priv *priv = ((struct rtl8192cd_priv *)netdev_priv(dev))->wlan_priv;
#else
    struct rtl8192cd_priv *priv = dev->priv;
#endif
#ifdef UNIVERSAL_REPEATER
	struct rtl8192cd_priv *priv_vxd;
#endif

	unsigned long flags=0;

	DBFENTER;

	if (!(priv->drv_state & DRV_STATE_OPEN)
#ifdef WDS
	 && dev->base_addr
#endif
) {
		DBFEXIT;
		return 0;
	}

	SAVE_INT_AND_CLI(flags);
	SMP_LOCK(flags);

#ifdef WDS
	if(dev->base_addr)
#endif
		priv->drv_state &= ~DRV_STATE_OPEN;     // set driver as has been closed, david

#ifdef PCIE_POWER_SAVING
	if (timer_pending(&priv->ps_timer))
		del_timer_sync(&priv->ps_timer);
	if((priv->pwr_state == L1) || (priv->pwr_state == L2)) {
		PCIeWakeUp(priv, (POWER_DOWN_T0<<3));
	}
#endif

#if defined(RESERVE_TXDESC_FOR_EACH_IF) && (defined(UNIVERSAL_REPEATER) || defined(MBSSID))
	recalc_txdesc_limit(priv);
#endif

#ifdef UNIVERSAL_REPEATER
	if (IS_ROOT_INTERFACE(priv)) {
		priv_vxd = GET_VXD_PRIV(priv);

		// if vxd interface is opened, close it first
		if (IS_DRV_OPEN(priv_vxd)) {
			SMP_UNLOCK(flags);
			rtl8192cd_close(priv_vxd->dev);
			SMP_LOCK(flags);
		}
	}
	else {
#ifdef MBSSID
/*
		if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
*/
		if (priv->vap_id < 0)
#endif
		disable_vxd_ap(priv);
	}
#endif

    netif_stop_queue(dev);

#ifdef WDS
	if (dev->base_addr == 0)
	{
		RESTORE_INT(flags);
		SMP_UNLOCK(flags);
		DBFEXIT;
		return 0;
	}
#endif

#ifdef CONFIG_RTK_MESH
	if (dev->base_addr == 1)
	{
		RESTORE_INT(flags);
		SMP_UNLOCK(flags);
		return 0;
	}
#endif // CONFIG_RTK_MESH

#ifdef CHECK_HANGUP
	if (!priv->reset_hangup)
#endif
	{
		if (OPMODE & WIFI_AP_STATE) {
			int i;
			for(i=0; i<NUM_STAT; i++)
			{
				if (priv->pshare->aidarray[i] && (priv->pshare->aidarray[i]->used == TRUE)
#ifdef WDS
					&& !(priv->pshare->aidarray[i]->station.state & WIFI_WDS)
#endif
				) {
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
					if (priv != priv->pshare->aidarray[i]->priv)
						continue;
#endif
					issue_deauth(priv, priv->pshare->aidarray[i]->station.hwaddr, _RSON_DEAUTH_STA_LEAVING_);
				}
			}

			delay_ms(10);
		}
#ifdef CLIENT_MODE	/* WPS2DOTX   */
		else if((OPMODE & (WIFI_STATION_STATE|WIFI_AUTH_SUCCESS|WIFI_ASOC_STATE))==(WIFI_STATION_STATE|WIFI_AUTH_SUCCESS|WIFI_ASOC_STATE)){
				//issue_disassoc(priv,BSSID,_RSON_DEAUTH_STA_LEAVING_);
				//OS_DEBUG("issue_deauth to AP\n");
				//printMac(BSSID);
				issue_deauth(priv,BSSID,_RSON_DEAUTH_STA_LEAVING_);			
				delay_ms(50);//make sure before issue_disassoc then TX be close		
				OPMODE &= ~(WIFI_AUTH_SUCCESS|WIFI_ASOC_STATE) ;
		}
#endif    /* WPS2DOTX   */
	}

#ifdef MBSSID
	if (IS_ROOT_INTERFACE(priv)) {
		int i;
		if (priv->pmib->miscEntry.vap_enable) {
			SMP_UNLOCK(flags);
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
				if (IS_DRV_OPEN(priv->pvap_priv[i]))
					rtl8192cd_close(priv->pvap_priv[i]->dev);
			}
			SMP_LOCK(flags);
		}
	}

	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
#ifdef  CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv))
	        GET_HAL_INTERFACE(priv)->StopMBSSIDHandler(priv);
		else
#endif  //CONFIG_WLAN_HAL
		rtl8192cd_stop_mbssid(priv);

	}
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#ifdef RTK_QUE
		free_rtk_queue(priv, &priv->pshare->skb_queue);
#else
		free_skb_queue(priv, &priv->pshare->skb_queue);
#endif

#ifdef CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {

            //Check Error Flag
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
            if (errorFlag != 0x0) {
                panic_printk("Error Flag: 0x%x\n", errorFlag);
            }
                       
        	if (RT_STATUS_SUCCESS == GET_HAL_INTERFACE(priv)->StopHWHandler(priv)) {
                printk("StopHW Succeed\n");
        	}
            else {
                #if 0 //Filen, we can't dump event after StopHW. except dump register is at PON section
                GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
                errorFlag |= DRV_ER_CLOSE_STOP_HW;
                GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
                #endif
                panic_printk("StopHW Failed\n");
            }

#ifdef CONFIG_WLAN_HAL_8881A
            if(GET_CHIP_VER(priv)==VERSION_8881A) {
                //Disable MAC_System(BIT(0)), MAC_Lextra_Bus(BIT(1))                
                REG32(0xB80000DC)= 0x00;
            }
#endif //#ifdef CONFIG_WLAN_HAL_8881A
            
		}
        else
#endif
		rtl8192cd_stop_hw(priv);

#ifdef __KERNEL__
#ifdef CHECK_HANGUP
		if (!priv->reset_hangup)
#endif
		{
			free_irq(dev->irq, dev);
#ifdef PCIE_POWER_SAVING
#ifdef GPIO_WAKEPIN			
#ifdef RTL8676_WAKE_GPIO
		{
			int gpio_num, irq_num;

			get_wifi_wake_pin(&gpio_num);
			irq_num = gpioGetBspIRQNum(gpio_num);				
			REG32(BSP_GIMR) &= ~ BIT(irq_num);

			free_irq(irq_num, dev);
			gpioClearISR(gpio_num); // clear GPIO interrupt status
		}
	#else

#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
			REG32(0xb8003000) &= ~ BIT(16);		// GIMR
#else
			if (REG32(REVR) == RTL8196C_REVISION_B) 	
				REG32(0xb8003000) &= ~ BIT(9);		// GIMR
#endif
#if defined(__LINUX_2_6__)
			free_irq(BSP_GPIO_ABCD_IRQ, dev);
#else
			free_irq(1, dev);
#endif	

#endif	
#endif
#endif

		}			
#endif



#ifdef UNIVERSAL_REPEATER
#ifdef __ECOS
		if (GET_VXD_PRIV(priv))
#endif
			GET_VXD_PRIV(priv)->drv_state &= ~DRV_STATE_VXD_INIT;
#endif
	}

	rtl8192cd_stop_sw(priv);

#ifdef ENABLE_RTL_SKB_STATS
	DEBUG_INFO("skb_tx_cnt =%d\n", rtl_atomic_read(&priv->rtl_tx_skb_cnt));
	DEBUG_INFO("skb_rx_cnt =%d\n", rtl_atomic_read(&priv->rtl_rx_skb_cnt));
#endif

#if defined(CONFIG_RTL_92D_DMDP) && defined(CHECK_HANGUP)
#ifdef MBSSID
        if (IS_ROOT_INTERFACE(priv))
#endif
        if ((GET_CHIP_VER(priv) == VERSION_8192D)
			&& (priv->pshare->rf_ft_var.peerReinit)
        	&& (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY))
                reset_dmdp_peer(priv);
#endif

	RESTORE_INT(flags);
	SMP_UNLOCK(flags);

#ifdef  CONFIG_WLAN_HAL
#if defined( MBSSID) || defined(UNIVERSAL_REPEATER)
        if (IS_ROOT_INTERFACE(priv))
#endif
	if (IS_HAL_CHIP(priv)){
		GET_HAL_INTERFACE(priv)->StopSWHandler(priv);
	}
#endif  //CONFIG_WLAN_HAL

#if 0
//#ifdef PCIE_POWER_SAVING
	if(!IS_UMC_A_CUT_88C(priv))
#ifdef CHECK_HANGUP
	if (!priv->reset_hangup)
#endif
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	HostPCIe_Close();
#endif

	DBFEXIT;
    return 0;
}


#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
static void MDL_DEVINIT set_mib_default(struct rtl8192cd_priv *priv)
{
	unsigned char *p;
#ifdef __KERNEL__
	struct sockaddr addr;
	p = addr.sa_data;
#else
	unsigned char tmpbuf[10];
	p = (unsigned char *)tmpbuf;
#endif

	priv->pmib->mib_version = MIB_VERSION;
	set_mib_default_tbl(priv);

	// others that are not types of byte and int
	strcpy((char *)priv->pmib->dot11StationConfigEntry.dot11DesiredSSID, "RTL8186-default");
	priv->pmib->dot11StationConfigEntry.dot11DesiredSSIDLen = strlen("RTL8186-default");
	memcpy(p, "\x00\xe0\x4c\x81\x86\x86", MACADDRLEN);

#ifdef	DOT11D
	// set countryCode for 11d
	strcpy(priv->pmib->dot11dCountry.dot11CountryString, "US");
#endif

#ifdef 	__KERNEL__
	rtl8192cd_set_hwaddr(priv->dev, (void *)&addr);
#else
	rtl8192cd_set_hwaddr(priv->dev, (void *)p);
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#ifdef DFS
		init_timer(&priv->ch52_timer);
		priv->ch52_timer.data = (unsigned long) priv;
		priv->ch52_timer.function = rtl8192cd_ch52_timer;

		init_timer(&priv->ch56_timer);
		priv->ch56_timer.data = (unsigned long) priv;
		priv->ch56_timer.function = rtl8192cd_ch56_timer;

		init_timer(&priv->ch60_timer);
		priv->ch60_timer.data = (unsigned long) priv;
		priv->ch60_timer.function = rtl8192cd_ch60_timer;

		init_timer(&priv->ch64_timer);
		priv->ch64_timer.data = (unsigned long) priv;
		priv->ch64_timer.function = rtl8192cd_ch64_timer;

		init_timer(&priv->ch100_timer);
		priv->ch100_timer.data = (unsigned long) priv;
		priv->ch100_timer.function = rtl8192cd_ch100_timer;

		init_timer(&priv->ch104_timer);
		priv->ch104_timer.data = (unsigned long) priv;
		priv->ch104_timer.function = rtl8192cd_ch104_timer;

		init_timer(&priv->ch108_timer);
		priv->ch108_timer.data = (unsigned long) priv;
		priv->ch108_timer.function = rtl8192cd_ch108_timer;

		init_timer(&priv->ch112_timer);
		priv->ch112_timer.data = (unsigned long) priv;
		priv->ch112_timer.function = rtl8192cd_ch112_timer;

		init_timer(&priv->ch116_timer);
		priv->ch116_timer.data = (unsigned long) priv;
		priv->ch116_timer.function = rtl8192cd_ch116_timer;

		init_timer(&priv->ch120_timer);
		priv->ch120_timer.data = (unsigned long) priv;
		priv->ch120_timer.function = rtl8192cd_ch120_timer;

		init_timer(&priv->ch124_timer);
		priv->ch124_timer.data = (unsigned long) priv;
		priv->ch124_timer.function = rtl8192cd_ch124_timer;

		init_timer(&priv->ch128_timer);
		priv->ch128_timer.data = (unsigned long) priv;
		priv->ch128_timer.function = rtl8192cd_ch128_timer;

		init_timer(&priv->ch132_timer);
		priv->ch132_timer.data = (unsigned long) priv;
		priv->ch132_timer.function = rtl8192cd_ch132_timer;

		init_timer(&priv->ch136_timer);
		priv->ch136_timer.data = (unsigned long) priv;
		priv->ch136_timer.function = rtl8192cd_ch136_timer;

		init_timer(&priv->ch140_timer);
		priv->ch140_timer.data = (unsigned long) priv;
		priv->ch140_timer.function = rtl8192cd_ch140_timer;
#endif

		if (((priv->pshare->type>>TYPE_SHIFT) & TYPE_MASK) == TYPE_EMBEDDED) {
			// not implement yet
		} else {
#ifdef IO_MAPPING
			priv->pshare->io_mapping = 1;
#endif
		}
	}

#ifdef CONFIG_RTL_92D_SUPPORT
	if (priv->pshare->version_id == VERSION_8192D) {
		if (priv->pshare->rf_ft_var.use_ext_lna)
			priv->pshare->rf_ft_var.use_ext_lna = 0;
	}
#endif
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A)
    if ((priv->pshare->version_id == VERSION_8812E) || (priv->pshare->version_id == VERSION_8881A)) {
        if (priv->pshare->rf_ft_var.use_ext_lna)
            priv->pshare->rf_ft_var.use_ext_lna = 0;
		//if (priv->pshare->rf_ft_var.use_ext_pa)
         //   priv->pshare->rf_ft_var.use_ext_pa = 0;
    }
#endif



#ifdef HIGH_POWER_EXT_PA

#if CONFIG_USE_PCIE_SLOT_1
//=========================
//SLOT0=5G, wlan0=pcie0, wlan1=pcie1
#ifdef CONFIG_RTL_5G_SLOT_0
#ifdef CONFIG_SLOT_0_EXT_PA
	if(priv->pshare->wlandev_idx == 0)
		priv->pshare->rf_ft_var.use_ext_pa = 1;
#endif
#ifdef CONFIG_SLOT_1_EXT_PA
	if(priv->pshare->wlandev_idx == 1)
		priv->pshare->rf_ft_var.use_ext_pa = 1;
#endif
#endif
//=========================
//SLOT0=5G, wlan0=pcie1, wlan1=pcie0
#ifdef CONFIG_RTL_5G_SLOT_1
#ifdef CONFIG_SLOT_0_EXT_PA
	if(priv->pshare->wlandev_idx == 1)
		priv->pshare->rf_ft_var.use_ext_pa = 1;
#endif
#ifdef CONFIG_SLOT_1_EXT_PA
	if(priv->pshare->wlandev_idx == 0)
		priv->pshare->rf_ft_var.use_ext_pa = 1;
#endif
#endif
//=========================
//No 5G interface, wlan0=pcie0, wlan1=pcie1
#if !defined(CONFIG_RTL_5G_SLOT_0) && !defined(CONFIG_RTL_5G_SLOT_1)
#ifdef CONFIG_SLOT_0_EXT_PA
	if(priv->pshare->wlandev_idx == 0)
		priv->pshare->rf_ft_var.use_ext_pa = 1;
#endif
#ifdef CONFIG_SLOT_1_EXT_PA
	if(priv->pshare->wlandev_idx == 1)
		priv->pshare->rf_ft_var.use_ext_pa = 1;
#endif
#endif
//==========================
//Only ONE SLOT, always enable HIGH Power 
#else //CONFIG_USE_PCIE_SLOT_1
	priv->pshare->rf_ft_var.use_ext_pa = 1;
#endif //CONFIG_USE_PCIE_SLOT_1
//===========================

#endif //HIGH_POWER_EXT_PA

#ifdef HIGH_POWER_EXT_LNA
#if CONFIG_USE_PCIE_SLOT_1
//=========================
//SLOT0=5G, wlan0=pcie0, wlan1=pcie1
#ifdef CONFIG_RTL_5G_SLOT_0
#ifdef CONFIG_SLOT_0_EXT_LNA
	if(priv->pshare->wlandev_idx == 0)
		priv->pshare->rf_ft_var.use_ext_lna = 1;
#endif
#ifdef CONFIG_SLOT_1_EXT_LNA
	if(priv->pshare->wlandev_idx == 1)
		priv->pshare->rf_ft_var.use_ext_lna = 1;
#endif
#endif
//=========================
//SLOT0=5G, wlan0=pcie1, wlan1=pcie0
#ifdef CONFIG_RTL_5G_SLOT_1
#ifdef CONFIG_SLOT_0_EXT_LNA
	if(priv->pshare->wlandev_idx == 1)
		priv->pshare->rf_ft_var.use_ext_lna = 1;
#endif
#ifdef CONFIG_SLOT_1_EXT_LNA
	if(priv->pshare->wlandev_idx == 0)
		priv->pshare->rf_ft_var.use_ext_lna = 1;
#endif
#endif
//=========================
//No 5G interface, wlan0=pcie0, wlan1=pcie1
#if !defined(CONFIG_RTL_5G_SLOT_0) && !defined(CONFIG_RTL_5G_SLOT_1)
#ifdef CONFIG_SLOT_0_EXT_LNA
	if(priv->pshare->wlandev_idx == 0)
		priv->pshare->rf_ft_var.use_ext_lna = 1;
#endif
#ifdef CONFIG_SLOT_1_EXT_LNA
	if(priv->pshare->wlandev_idx == 1)
		priv->pshare->rf_ft_var.use_ext_lna = 1;
#endif
#endif
//==========================
//Only ONE SLOT, always enable HIGH Power 
#else //CONFIG_USE_PCIE_SLOT_1
	priv->pshare->rf_ft_var.use_ext_lna = 1;
#endif //CONFIG_USE_PCIE_SLOT_1
//===========================

#endif //HIGH_POWER_EXT_LNA


}

#if defined(__LINUX_2_6__) && !defined(CONFIG_COMPAT_NET_DEV_OPS)
static const struct net_device_ops rtl8192cd_netdev_ops = {
        .ndo_open               = rtl8192cd_open,
        .ndo_stop               = rtl8192cd_close,
        .ndo_set_mac_address    = rtl8192cd_set_hwaddr,
        .ndo_set_multicast_list = rtl8192cd_set_rx_mode,
        .ndo_get_stats          = rtl8192cd_get_stats,
        .ndo_do_ioctl           = rtl8192cd_ioctl,
        .ndo_start_xmit         = rtl8192cd_start_xmit,
};
#endif


#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
#ifdef __KERNEL__
static int MDL_DEVINIT rtl8192cd_init_one(struct pci_dev *pdev,
                  const struct pci_device_id *ent, struct _device_info_ *wdev, int vap_idx)
#else
void *rtl8192cd_init_one(void *pdev, void *ent, struct _device_info_ *wdev, int vap_idx)
#endif
{
#if defined(NOT_RTK_BSP)	
    unsigned long page_align_phy=0;
#endif
    struct net_device *dev;
#ifndef __KERNEL__
    struct net_device *tmp_dev;
#endif
    struct rtl8192cd_priv *priv;
    void *regs;
	struct wifi_mib 		*pmib;
	DOT11_QUEUE				*pevent_queue;
#ifdef CONFIG_RTL_WAPI_SUPPORT
	DOT11_QUEUE				*wapiEvent_queue;
	#if	defined(MBSSID)
	DOT11_QUEUE				*wapiVapEvent_queue;
	#endif

#endif
	struct rtl8192cd_hw		*phw;
	struct rtl8192cd_tx_desc_info		*ptxdesc;
	struct wlan_hdr_poll	*pwlan_hdr_poll;
	struct wlanllc_hdr_poll	*pwlanllc_hdr_poll;
	struct wlanbuf_poll		*pwlanbuf_poll;
	struct wlanicv_poll		*pwlanicv_poll;
	struct wlanmic_poll		*pwlanmic_poll;
	struct wlan_acl_poll	*pwlan_acl_poll;
	DOT11_EAP_PACKET		*Eap_packet;
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
	WPA_GLOBAL_INFO			*wpa_global_info;
#endif
#ifdef  CONFIG_RTK_MESH
#ifdef _MESH_ACL_ENABLE_
	struct mesh_acl_poll	*pmesh_acl_poll = NULL;
#endif
#ifdef _11s_TEST_MODE_
	struct Galileo_poll		*pgalileo_poll = NULL;
#endif
#ifndef WDS
	char baseDevName[8];
#endif
	int mesh_num;

	struct hash_table		*proxy_table;
#ifdef PU_STANDARD
	//pepsi
	struct hash_table		*proxyupdate_table;
#endif
	struct mpp_tb			*pann_mpp_tb;
	struct hash_table		*mesh_rreq_retry_queue;
	// add by chuangch 2007.09.13
	struct hash_table		*pathsel_table;

	DOT11_QUEUE2			*pathsel_queue ;
#ifdef	_11s_TEST_MODE_
	DOT11_QUEUE2			*receiver_queue = NULL;
#endif
#endif	// CONFIG_RTK_MESH



#ifdef CONFIG_NET_PCI
    u8 cache_size;
    u16 pci_command;
#ifndef USE_IO_OPS
    u32 pciaddr;
#endif
	u32 pmem_len;
#endif

#ifdef WDS
	int wds_num;
	char baseDevName[8];
#endif
#if defined(CONFIG_RTK_MESH) || defined(WDS)
	int i;
#endif

	unsigned char *page_ptr;
	struct priv_shared_info *pshare;	// david

#ifdef CONFIG_WLAN_HAL
    BOOLEAN     bVal;
	unsigned int errorFlag;
#endif  //CONFIG_WLAN_HAL
    int rc=0;
    priv = NULL;
    regs = NULL;
	pmib = NULL;
	pevent_queue = NULL;
#ifdef CONFIG_RTL_WAPI_SUPPORT
	wapiEvent_queue = NULL;
	#if	defined(MBSSID)
	wapiVapEvent_queue = NULL;
	#endif

#endif
	phw = NULL;
	ptxdesc = NULL;
	pwlan_hdr_poll = NULL;
	pwlanllc_hdr_poll = NULL;
	pwlanbuf_poll = NULL;
	pwlanicv_poll = NULL;
	pwlanmic_poll = NULL;
	pwlan_acl_poll = NULL;
	Eap_packet = NULL;
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
	wpa_global_info = NULL;
#endif




#ifdef CONFIG_NET_PCI
    cache_size=0;
#ifndef USE_IO_OPS
    pciaddr=0;
#endif
	pmem_len=0;
#endif


	pshare = NULL;	// david

	printk("=====>>INSIDE rtl8192cd_init_one <<=====\n");
	dev = alloc_etherdev(sizeof(struct rtl8192cd_priv));
	if (!dev) {
		printk(KERN_ERR "alloc_etherdev() error!\n");
#ifdef __KERNEL__
       	return -ENOMEM;
#else
		return NULL;
#endif
	}

#ifndef __KERNEL__
	tmp_dev = dev;
#endif
	// now, allocating memory for pmib
#ifdef RTL8192CD_VARIABLE_USED_DMEM
	pmib = (struct wifi_mib *)rtl8192cd_dmem_alloc(PMIB, NULL);
#else
	pmib = (struct wifi_mib *)kmalloc((sizeof(struct wifi_mib)), GFP_ATOMIC);
#endif
	if (!pmib) {
		rc = -ENOMEM;
		printk(KERN_ERR "Can't kmalloc for wifi_mib (size %d)\n", sizeof(struct wifi_mib));
		goto err_out_free;
	}
	memset(pmib, 0, sizeof(struct wifi_mib));

	pevent_queue = (DOT11_QUEUE *)kmalloc((sizeof(DOT11_QUEUE)), GFP_ATOMIC);
	if (!pevent_queue) {
		rc = -ENOMEM;
		printk(KERN_ERR "Can't kmalloc for DOT11_QUEUE (size %d)\n", sizeof(DOT11_QUEUE));
		goto err_out_free;
	}
	memset((void *)pevent_queue, 0, sizeof(DOT11_QUEUE));
#ifdef CONFIG_RTL_WAPI_SUPPORT
	wapiEvent_queue = (DOT11_QUEUE *)kmalloc((sizeof(DOT11_QUEUE)), GFP_ATOMIC);
	if (!wapiEvent_queue) {
		rc = -ENOMEM;
		printk(KERN_ERR "Can't kmalloc for DOT11_QUEUE (size %d)\n", sizeof(DOT11_QUEUE));
		goto err_out_free;
	}
	memset((void *)wapiEvent_queue, 0, sizeof(DOT11_QUEUE));
#ifdef MBSSID
	wapiVapEvent_queue = (DOT11_QUEUE *)kmalloc((sizeof(DOT11_QUEUE)*RTL8192CD_NUM_VWLAN), GFP_ATOMIC);
	if (!wapiVapEvent_queue) {
		rc = -ENOMEM;
		printk(KERN_ERR "Can't kmalloc for DOT11_QUEUE (size %d)\n", sizeof(DOT11_QUEUE)*RTL8192CD_NUM_VWLAN);
		goto err_out_free;
	}
	memset((void *)wapiVapEvent_queue, 0, sizeof(DOT11_QUEUE)*RTL8192CD_NUM_VWLAN);
	#endif

#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (wdev->priv == NULL) // root interface
#endif
	{
#ifdef PRIV_STA_BUF
#ifdef CONCURRENT_MODE
		phw = &hw_info[wlandev_idx];
#else
		phw = &hw_info;
#endif
#else
		phw = (struct rtl8192cd_hw *)kmalloc((sizeof(struct rtl8192cd_hw)), GFP_ATOMIC);
		if (!phw) {
			rc = -ENOMEM;
			printk(KERN_ERR "Can't kmalloc for rtl8192cd_hw (size %d)\n", sizeof(struct rtl8192cd_hw));
			goto err_out_free;
		}
#endif
		memset((void *)phw, 0, sizeof(struct rtl8192cd_hw));
		ptxdesc = &phw->tx_info;

#ifdef PRIV_STA_BUF
#ifdef CONCURRENT_MODE
		pshare = &shared_info[wlandev_idx];
#else
		pshare = &shared_info;
#endif
#else
		pshare = (struct priv_shared_info *)kmalloc(sizeof(struct priv_shared_info), GFP_ATOMIC);
		if (!pshare) {
			rc = -ENOMEM;
			printk(KERN_ERR "Can't kmalloc for priv_shared_info (size %d)\n", sizeof(struct priv_shared_info));
			goto err_out_free;
		}
#endif
		memset((void *)pshare, 0, sizeof(struct priv_shared_info));

#ifdef PRIV_STA_BUF
#ifdef CONCURRENT_MODE
		pwlan_hdr_poll = &hdr_pool[wlandev_idx];
#else
		pwlan_hdr_poll = &hdr_pool;
#endif
#else
		pwlan_hdr_poll = (struct wlan_hdr_poll *)
						kmalloc((sizeof(struct wlan_hdr_poll)), GFP_ATOMIC);
		if (!pwlan_hdr_poll) {
			rc = -ENOMEM;
			printk(KERN_ERR "Can't kmalloc for wlan_hdr_poll (size %d)\n", sizeof(struct wlan_hdr_poll));
			goto err_out_free;
		}
#endif

#ifdef PRIV_STA_BUF
#ifdef CONCURRENT_MODE
		pwlanllc_hdr_poll = &llc_pool[wlandev_idx];
#else
		pwlanllc_hdr_poll = &llc_pool;
#endif
#else
		pwlanllc_hdr_poll = (struct wlanllc_hdr_poll *)
						kmalloc((sizeof(struct wlanllc_hdr_poll)), GFP_ATOMIC);
		if (!pwlanllc_hdr_poll) {
			rc = -ENOMEM;
			printk(KERN_ERR "Can't kmalloc for wlanllc_hdr_poll (size %d)\n", sizeof(struct wlanllc_hdr_poll));
			goto err_out_free;
		}
#endif

#ifdef PRIV_STA_BUF
#ifdef CONCURRENT_MODE
		pwlanbuf_poll = &buf_pool[wlandev_idx];
#else
		pwlanbuf_poll = &buf_pool;
#endif
#else
		pwlanbuf_poll = (struct	wlanbuf_poll *)
						kmalloc((sizeof(struct	wlanbuf_poll)), GFP_ATOMIC);
		if (!pwlanbuf_poll) {
			rc = -ENOMEM;
			printk(KERN_ERR "Can't kmalloc for wlanbuf_poll (size %d)\n", sizeof(struct wlanbuf_poll));
			goto err_out_free;
		}
#endif

#ifdef PRIV_STA_BUF
#ifdef CONCURRENT_MODE
		pwlanicv_poll = &icv_pool[wlandev_idx];
#else
		pwlanicv_poll = &icv_pool;
#endif
#else
		pwlanicv_poll = (struct	wlanicv_poll *)
						kmalloc((sizeof(struct	wlanicv_poll)), GFP_ATOMIC);
		if (!pwlanicv_poll) {
			rc = -ENOMEM;
			printk(KERN_ERR "Can't kmalloc for wlanicv_poll (size %d)\n", sizeof(struct wlanicv_poll));
			goto err_out_free;
		}
#endif

#ifdef PRIV_STA_BUF
#ifdef CONCURRENT_MODE
		pwlanmic_poll = &mic_pool[wlandev_idx];
		wlandev_idx++;
#else
		pwlanmic_poll = &mic_pool;
#endif
#else
		pwlanmic_poll = (struct	wlanmic_poll *)
						kmalloc((sizeof(struct	wlanmic_poll)), GFP_ATOMIC);
		if (!pwlanmic_poll) {
			rc = -ENOMEM;
			printk(KERN_ERR "Can't kmalloc for wlanmic_poll (size %d)\n", sizeof(struct wlanmic_poll));
			goto err_out_free;
		}
#endif
	}

	pwlan_acl_poll = (struct wlan_acl_poll *)
					kmalloc((sizeof(struct wlan_acl_poll)), GFP_ATOMIC);
	if (!pwlan_acl_poll) {
		rc = -ENOMEM;
		printk(KERN_ERR "Can't kmalloc for wlan_acl_poll (size %d)\n", sizeof(struct wlan_acl_poll));
		goto err_out_free;
	}

#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_)	// below code copy above ACL code
	pmesh_acl_poll = (struct mesh_acl_poll *)
					kmalloc((sizeof(struct mesh_acl_poll)), GFP_ATOMIC);
	if (!pmesh_acl_poll) {
		rc = -ENOMEM;
		printk(KERN_ERR "Can't kmalloc for Mesh wlan_acl_poll (size %d)\n", sizeof(struct mesh_acl_poll));
		goto err_out_free;
	}
#endif

	Eap_packet = (DOT11_EAP_PACKET *)
					kmalloc((sizeof(DOT11_EAP_PACKET)), GFP_ATOMIC);
	if (!Eap_packet) {
		rc = -ENOMEM;
		printk(KERN_ERR "Can't kmalloc for Eap_packet (size %d)\n", sizeof(DOT11_EAP_PACKET));
		goto err_out_free;
	}
	memset((void *)Eap_packet, 0, sizeof(DOT11_EAP_PACKET));

#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
	wpa_global_info = (WPA_GLOBAL_INFO *)
					kmalloc((sizeof(WPA_GLOBAL_INFO)), GFP_ATOMIC);
	if (!wpa_global_info) {
		rc = -ENOMEM;
		printk(KERN_ERR "Can't kmalloc for wpa_global_info (size %d)\n", sizeof(WPA_GLOBAL_INFO));
		goto err_out_free;
	}
	memset((void *)wpa_global_info, 0, sizeof(WPA_GLOBAL_INFO));
#endif

#ifndef __DRAYTEK_OS__
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (wdev->priv) {
#ifdef UNIVERSAL_REPEATER
		if (vap_idx < 0)
			sprintf(dev->name, "%s-vxd", wdev->priv->dev->name);
#endif
#ifdef MBSSID
		if (vap_idx >= 0)
#ifdef CONFIG_RTL8672
			sprintf(dev->name, "%s-vap%d", wdev->priv->dev->name, vap_idx);
#else
			sprintf(dev->name, "%s-va%d", wdev->priv->dev->name, vap_idx);
#endif
#endif
	}
	else
#endif
		strcpy(dev->name, "wlan%d");
#endif

#ifdef LINUX_2_6_24_
	/*SET_MODULE_OWNER is obsolete from 2.6.24*/
#else
	SET_MODULE_OWNER(dev);
#endif

#ifdef NETDEV_NO_PRIV
	priv = (struct rtl8192cd_priv *)netdev_priv(dev);
	priv->wlan_priv = priv;
#else
	priv = dev->priv;
#endif

	priv->pmib = pmib;
#if 0/*defined(CONFIG_RTL_WAPI_SUPPORT)*/
	/*	only for test	*/
	priv->pmib->wapiInfo.wapiType = wapiDisable;
	priv->pmib->wapiInfo.wapiUpdateMCastKeyType = wapi_disable_update;
	priv->pmib->wapiInfo.wapiUpdateUCastKeyTimeout = WAPI_KEY_UPDATE_PERIOD;
	priv->pmib->wapiInfo.wapiUpdateUCastKeyPktNum = WAPI_KEY_UPDATE_PKTCNT;
#endif
	priv->pevent_queue = pevent_queue;
#ifdef CONFIG_RTL_WAPI_SUPPORT
	priv->wapiEvent_queue= wapiEvent_queue;
#endif
	priv->pwlan_acl_poll = pwlan_acl_poll;
	priv->Eap_packet = Eap_packet;
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
	priv->wpa_global_info = wpa_global_info;
#endif
#ifdef MBSSID
	priv->vap_id = -1;
#endif
#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_)	// below code copy above ACL code
	priv->pmesh_acl_poll = pmesh_acl_poll;
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (wdev->priv) {
		priv->pshare = wdev->priv->pshare;
		GET_ROOT_PRIV(priv) = wdev->priv;
#ifdef UNIVERSAL_REPEATER
		if (vap_idx < 0) // create for vxd
			GET_VXD_PRIV(wdev->priv) = priv;
#endif
#ifdef MBSSID
		if (vap_idx >= 0)  { // create for vap
			GET_ROOT_PRIV(priv)->pvap_priv[vap_idx] = priv;
			priv->vap_id = vap_idx;
			priv->vap_init_seq = -1;

			#if 0 /*defined(CONFIG_RTL_WAPI_SUPPORT)*/
			priv->pmib->wapiInfo.wapiType = wapiDisable;
			priv->pmib->wapiInfo.wapiUpdateMCastKeyType = wapi_disable_update;
			priv->pmib->wapiInfo.wapiUpdateUCastKeyTimeout = WAPI_KEY_UPDATE_PERIOD;
			priv->pmib->wapiInfo.wapiUpdateUCastKeyPktNum = WAPI_KEY_UPDATE_PKTCNT;
			priv->wapiEvent_queue= &wapiVapEvent_queue[vap_idx];
			printk("dev[%s]:wapiType[%d] UCastKeyType[%d] psk[%s] len[%d]\n",
				priv->dev->name, priv->pmib->wapiInfo.wapiType,
				priv->pmib->wapiInfo.wapiUpdateMCastKeyType,
				priv->pmib->wapiInfo.wapiPsk.octet,
				priv->pmib->wapiInfo.wapiPsk.len);
			#endif


		}
#endif
#ifdef  CONFIG_WLAN_HAL
        priv->HalFunc = GET_ROOT_PRIV(priv)->HalFunc;
        priv->HalData = GET_ROOT_PRIV(priv)->HalData;
#endif
	}
	else
#endif
	{
		priv->pshare = pshare;	// david
		priv->pshare->phw = phw;
#ifdef CONCURRENT_MODE
		priv->pshare->wlandev_idx = wlan_index;
#endif
		priv->pshare->pdesc_info = ptxdesc;
		priv->pshare->pwlan_hdr_poll = pwlan_hdr_poll;
		priv->pshare->pwlanllc_hdr_poll = pwlanllc_hdr_poll;
		priv->pshare->pwlanbuf_poll = pwlanbuf_poll;
		priv->pshare->pwlanicv_poll = pwlanicv_poll;
		priv->pshare->pwlanmic_poll = pwlanmic_poll;
		wdev->priv = priv;
#ifdef __KERNEL__
		spin_lock_init(&priv->pshare->lock);
#endif

#ifdef CONFIG_RTK_MESH
		spin_lock_init(&priv->pshare->lock_queue);
		spin_lock_init(&priv->pshare->lock_Rreq);
#endif

#ifdef SMP_SYNC
		spin_lock_init(&priv->pshare->lock_xmit);
		spin_lock_init(&priv->pshare->lock_skb);
		spin_lock_init(&priv->pshare->lock_buf);
		spin_lock_init(&priv->pshare->lock_recv);
#endif

#ifdef CONFIG_NET_PCI
		if (((wdev->type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS)
			priv->pshare->pdev = pdev;
#endif
		priv->pshare->type = wdev->type;
#ifdef USE_RTL8186_SDK
#if defined(CONFIG_RTL8196B) || defined(CONFIG_RTL_819X)
		priv->pshare->have_hw_mic = 1;
#else
		priv->pshare->have_hw_mic = 0;
#endif
#else
		priv->pshare->have_hw_mic = 0;
#endif
//		priv->pshare->is_giga_exist  = is_giga_board();
	}

	priv->dev = dev;

#ifndef __DRAYTEK_OS__
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (GET_ROOT_PRIV(priv)) { // is a vxd or vap
		dev->base_addr = GET_ROOT_PRIV(priv)->dev->base_addr;
		goto register_driver;
	}
#endif
#ifdef CONFIG_NET_PCI
	if (((wdev->type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS)
	{
		rc = pci_enable_device(pdev);
		if (rc)
			goto err_out_free;
#ifndef USE_IO_OPS
		rc = pci_request_regions(pdev, DRV_NAME);
#endif
		if (rc)
			goto err_out_disable;

		if (pdev->irq < 2) {
			rc = -EIO;
#ifdef __LINUX_2_6__
			printk(KERN_ERR "invalid irq (%d) for pci dev\n", pdev->irq);
#else
			printk(KERN_ERR "invalid irq (%d) for pci dev %s\n", pdev->irq, pdev->slot_name);
#endif
			goto err_out_res;
		}
#ifdef USE_IO_OPS
		{
			unsigned long pio_start, pio_len, pio_flags;

			pio_start = (unsigned long)pci_resource_start(pdev, 0);
			pio_len = (unsigned long)pci_resource_len(pdev, 0);
			pio_flags = (unsigned long)pci_resource_flags(pdev, 0);
////			pio_start = (unsigned long)pci_resource_start(pdev, 2);
////			pio_len = (unsigned long)pci_resource_len(pdev, 2);
////			pio_flags = (unsigned long)pci_resource_flags(pdev, 2);


			if (!(pio_flags & IORESOURCE_IO)) {
				rc = -EIO;
#ifdef __LINUX_2_6__
				printk(KERN_ERR "pci: region #0 not a PIO resource, aborting\n");
#else
				printk(KERN_ERR "%s: region #0 not a PIO resource, aborting\n", pdev->slot_name);
#endif
				goto err_out_res;
			}

			if (!request_region(pio_start, pio_len, DRV_NAME)) {
				rc = -EIO;
				printk(KERN_ERR "request_region failed!\n");
				goto err_out_res;
			}

			if (pio_len < RTL8192CD_REGS_SIZE) {
				rc = -EIO;
#ifdef __LINUX_2_6__
				printk(KERN_ERR "PIO resource (%lx) too small on pci dev\n", pio_len);
#else
				printk(KERN_ERR "PIO resource (%lx) too small on pci dev %s\n", pio_len, pdev->slot_name);
#endif
				goto err_out_res;
			}

			dev->base_addr = pio_start;
			priv->pshare->ioaddr = pio_start; // device I/O address
		}
#else
#ifdef IO_MAPPING
		pciaddr = pci_resource_start(pdev, 0);
////		pciaddr = pci_resource_start(pdev, 2);
#else
//		pciaddr = pci_resource_start(pdev, 1);
		pciaddr = pci_resource_start(pdev, 2);
#endif
		if (!pciaddr) {
			rc = -EIO;
#ifdef __LINUX_2_6__
			printk(KERN_ERR "no MMIO resource for pci dev");
#else
			printk(KERN_ERR "no MMIO resource for pci dev %s\n", pdev->slot_name);
#endif
			goto err_out_res;
		}

//		if ((pmem_len = pci_resource_len(pdev, 1)) < RTL8192CD_REGS_SIZE) {
		if ((pmem_len = pci_resource_len(pdev, 2)) < RTL8192CD_REGS_SIZE) {
			rc = -EIO;
#ifdef __LINUX_2_6__
			printk(KERN_ERR "MMIO resource () too small on pci dev\n");
#else
			printk(KERN_ERR "MMIO resource (%lx) too small on pci dev %s\n", (unsigned long)pmem_len, pdev->slot_name);
#endif
			goto err_out_res;
		}

		regs = ioremap_nocache(pciaddr, pmem_len);
		if (!regs) {
			rc = -EIO;
#ifdef __LINUX_2_6__
			printk(KERN_ERR "Cannot map PCI MMIO () on pci dev \n");
#else
			printk(KERN_ERR "Cannot map PCI MMIO (%lx@%lx) on pci dev %s\n", (unsigned long)pmem_len, (long)pciaddr, pdev->slot_name);
#endif
			goto err_out_res;
		}

		dev->base_addr = (unsigned long)regs;
		priv->pshare->ioaddr = (UINT)regs;		
		check_chipID_MIMO(priv);
#endif // USE_IO_OPS
	printk("%s(%d), dev->base_addr:%08x,ioaddr:%08x\n", __FUNCTION__, __LINE__, dev->base_addr, priv->pshare->ioaddr);
#if 0 //defined(CONFIG_RTL_8812_SUPPORT) && (RTL_USED_PCIE_SLOT==1)
				u32 vendor_deivce_id, config_base;

		`		dev->base_addr==0xba000000;
				vendor_deivce_id = dev->base_addr;
				if(dev->base_addr==0xba000000)
				{
					vendor_deivce_id= *((volatile unsigned long *)(0xb8b30000));
				}
				else if(dev->base_addr==0xb9000000)
				{
					vendor_deivce_id= *((volatile unsigned long *)(0xb8b10000));
				}
//				printk("   vendor_deivce_id=%x\n", vendor_deivce_id);
#endif
#if !defined(NOT_RTK_BSP)
#ifdef CONFIG_RTL_92D_SUPPORT
			{
				u32 vendor_deivce_id, config_base;
			
				vendor_deivce_id = dev->base_addr;
				if(dev->base_addr==0xba000000)
				{
					vendor_deivce_id= *((volatile unsigned long *)(0xb8b30000));
				}
				else if(dev->base_addr==0xb9000000)
				{
					vendor_deivce_id= *((volatile unsigned long *)(0xb8b10000));
				}
				printk("   vendor_deivce_id=%x\n", vendor_deivce_id);
				if (vendor_deivce_id == ((unsigned long)((0x8193<<16)|PCI_VENDOR_ID_REALTEK)))
					priv->pshare->version_id = VERSION_8192D;
				else
					priv->pshare->version_id =0x1234;
			}
#endif
#else/*NOT_RTK_BSP*/

#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8192EE)
		{
			struct pci_dev *pdev = priv->pshare->pdev;
			u16 vendor_id,device_id;
			pci_read_config_word( pdev , PCI_VENDOR_ID  , &vendor_id);
			pci_read_config_word( pdev , PCI_DEVICE_ID  , &device_id);

			printk("version_id=%x, device_id:%x\n", vendor_id, device_id);
			if( (vendor_id==0x10ec) && (device_id==0x8193) ) 
			{
				printk("version_id=VERSION_8192D\n");
				priv->pshare->version_id = VERSION_8192D;
				#ifdef CONFIG_RTL_92D_DMDP
					priv->pshare->wlandev_idx = rtl8192D_idx;/*timmy_modify*/
					if( rtl8192D_idx == 0 )
					{
						priv->pmib->dot11RFEntry.phyBandSelect = PHY_BAND_5G;
						priv->pmib->dot11BssType.net_work_type = WIRELESS_11A | WIRELESS_11N ;
	}
	else
					{
						priv->pmib->dot11RFEntry.phyBandSelect = PHY_BAND_2G;
						priv->pmib->dot11BssType.net_work_type = WIRELESS_11B | WIRELESS_11G;
					}
					rtl8192D_idx++;
					printk("macPhyMode=DUALMAC_DUALPHY  MIMO_1T1R\n");
					priv->pmib->dot11RFEntry.macPhyMode = DUALMAC_DUALPHY;/*timmy_dbg*/
					priv->pshare->phw->MIMO_TR_hw_support = MIMO_1T1R;
				#else
					printk("macPhyMode=DUALMAC_DUALPHY MIMO_2T2R\n");
					priv->pmib->dot11RFEntry.macPhyMode = SINGLEMAC_SINGLEPHY;/*timmy_dbg*/
					priv->pshare->phw->MIMO_TR_hw_support = MIMO_2T2R;
				#endif
			} else if( (vendor_id==0x10ec) && (device_id==0x8812) ) 
			{
				printk("version_id=VERSION_8812E found\n");
				priv->pshare->version_id = VERSION_8812E;
				priv->pmib->dot11RFEntry.macPhyMode = SINGLEMAC_SINGLEPHY;
				priv->pshare->phw->MIMO_TR_hw_support = MIMO_2T2R;
			} else if( (vendor_id==0x10ec) && (device_id==0x8170 || device_id==0x818b) ) 
			{
				printk("version_id=VERSION_8192E found\n");
				priv->pshare->version_id = VERSION_8192E;
				priv->pmib->dot11RFEntry.macPhyMode = SINGLEMAC_SINGLEPHY;
				priv->pshare->phw->MIMO_TR_hw_support = MIMO_2T2R;
#ifdef  CONFIG_WLAN_HAL_8192EE
				if (RT_STATUS_FAILURE == HalAssociateNic(priv, TRUE)) {
					printk("HalAssociateNic Failed \n");
				} else {
					printk("HalAssociateNic OK \n");
				}
				priv->pshare->use_hal = 1;
				GET_HW(priv)->MIMO_TR_hw_support = GET_HAL_INTERFACE(priv)->GetChipIDMIMOHandler(priv);
       
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_MAC_IO_ENABLE, &bVal);
				if ( bVal ) {
					if (RT_STATUS_SUCCESS == GET_HAL_INTERFACE(priv)->StopHWHandler(priv)) {
						printk("StopHW Succeed\n");
					} else {
						GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);            
						errorFlag |= DRV_ER_CLOSE_STOP_HW;
						GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);                     
						panic_printk("StopHW Failed\n");
					}                
				} else {
					printk("Can't write MACID register\n");
				}
#endif
			}
		}
	#endif
#endif/*NOT_RTK_BSP*/
	}
	else
#endif
	{
		regs = (void *)wdev->base_addr;
		dev->base_addr = (unsigned long)wdev->base_addr;
		priv->pshare->ioaddr = (UINT)regs;

		if (((priv->pshare->type>>TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_DIRECT)
		{
			int i ;
#ifdef CONFIG_RTL_92D_SUPPORT
			u32  config_base, base_addr;
			config_base = wdev->conf_addr;
			base_addr = wdev->base_addr;
#endif

			_DEBUG_INFO("INIT PCI config space directly\n");

#if 0 //defined(CONFIG_RTL_8812_SUPPORT) && (RTL_USED_PCIE_SLOT==1)
			wdev->conf_addr = BSP_PCIE1_D_CFG0;
#endif


#if !defined(CONFIG_NET_PCI) && (defined(CONFIG_RTL8196B) || defined(CONFIG_RTL_819X) || defined(CONFIG_RTL_8196E))
			#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_819X) || defined(CONFIG_RTL_8196E)

#if 0 //defined(CONFIG_RTL_8812_SUPPORT) && (RTL_USED_PCIE_SLOT==1)
			if(1)
#else
			if (BSP_PCIE0_D_CFG0 == wdev->conf_addr)

			{
#ifdef CONFIG_RTL_8198B //mark_apo
				 PCIE_reset_procedure(1, 0, 1, BSP_PCIE1_D_CFG0); 
#endif				
				if ((PCIE_reset_procedure(0, 0, 1, wdev->conf_addr))== FAIL)
				{
					rc = -ENODEV;
					goto err_out_free;
				}
			}
			else if (BSP_PCIE1_D_CFG0 == wdev->conf_addr)
#endif				
			{
				if ((PCIE_reset_procedure(1, 0, 1, wdev->conf_addr))== FAIL)
				{
					rc = -ENODEV;
					goto err_out_free;
				}
			}
			else
			{
				goto err_out_free;
			}
			#else
			if (rtl8196b_pci_reset(wdev->conf_addr) == FAIL)
				goto err_out_free;
			#endif
			//REG32(0xb8000040)=0xf;  // port 5 support !!!!!
#endif
#ifdef __ECOS
			/* Fix the issue to use memory under 1M */
			REG32(0xb8b00000+0x1c)=(2<<4) | (0<<12);   // [7:4]=base [15:12]=limit
			REG32(0xb8b00000+0x20)=(2<<4) | (0<<20);   // [7:4]=base [15:12]=limit
			REG32(0xb8b00000+0x24)=(2<<4) | (0<<20);   // [7:4]=base [15:12]=limit
#endif

			{
				u32 vendor_deivce_id, config_base;
				config_base = wdev->conf_addr;
				vendor_deivce_id = *((volatile unsigned long *)(config_base+0));
				printk("vendor_deivce_id=%x\n", vendor_deivce_id);
				if (
#if defined(CONFIG_RTL_92D_SUPPORT)
					(vendor_deivce_id != ((unsigned long)((0x8193<<16)|PCI_VENDOR_ID_REALTEK))) &&
#endif
					(vendor_deivce_id != ((unsigned long)((0x8191<<16)|PCI_VENDOR_ID_REALTEK))) &&
					(vendor_deivce_id != ((unsigned long)((0x8171<<16)|PCI_VENDOR_ID_REALTEK))) &&
					(vendor_deivce_id != ((unsigned long)((0x8178<<16)|PCI_VENDOR_ID_REALTEK))) &&
					(vendor_deivce_id != ((unsigned long)((0x8174<<16)|PCI_VENDOR_ID_REALTEK))) &&
					(vendor_deivce_id != ((unsigned long)((0x8176<<16)|PCI_VENDOR_ID_REALTEK)))
#ifdef CONFIG_RTL_88E_SUPPORT
					&& (vendor_deivce_id != ((unsigned long)((0x8179<<16)|PCI_VENDOR_ID_REALTEK)))
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
					&& (vendor_deivce_id != ((unsigned long)((0x8812<<16)|PCI_VENDOR_ID_REALTEK)))
#endif
#if defined(CONFIG_WLAN_HAL_8192EE)
					&& (vendor_deivce_id != ((unsigned long)((0x8170<<16)|PCI_VENDOR_ID_REALTEK)))
					&& (vendor_deivce_id != ((unsigned long)((0x818b<<16)|PCI_VENDOR_ID_REALTEK)))
#endif
					) {
					_DEBUG_ERR("vendor_deivce_id=%x not match\n", vendor_deivce_id);
					rc = -EIO;
					goto err_out_free;
				}
					
#if defined (CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8192EE)
#ifdef CONFIG_RTL_92D_SUPPORT
				if (vendor_deivce_id == ((unsigned long)((0x8193<<16)|PCI_VENDOR_ID_REALTEK)))
					priv->pshare->version_id = VERSION_8192D;
				else
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
				if (vendor_deivce_id == ((unsigned long)((0x8179<<16)|PCI_VENDOR_ID_REALTEK))) {
					printk("\n found 8188E !!! \n");
					priv->pshare->version_id = VERSION_8188E;
				} else
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
				if (vendor_deivce_id == ((unsigned long)((0x8812<<16)|PCI_VENDOR_ID_REALTEK)))
				{
					printk("\n found 8812 !!! \n");
					priv->pshare->version_id = VERSION_8812E;
				}
				else
#endif
#if defined(CONFIG_WLAN_HAL_8192EE)
				if (vendor_deivce_id == ((unsigned long)((0x8170<<16)|PCI_VENDOR_ID_REALTEK))
                    || vendor_deivce_id == ((unsigned long)((0x818b<<16)|PCI_VENDOR_ID_REALTEK))) {
					priv->pshare->version_id = VERSION_8192E;
					panic_printk("\n found 8192E !!! \n");
					priv->pshare->use_hal = 1;
				}
				else
#endif
				{
					priv->pshare->version_id = 0;
				}
#endif
			}

			*((volatile unsigned long *)PCI_CONFIG_BASE1) = virt_to_bus((void *)dev->base_addr);
			//DEBUG_INFO("...config_base1 = 0x%08lx\n", *((volatile unsigned long *)PCI_CONFIG_BASE1));
			for(i=0; i<1000000; i++);
			*((volatile unsigned char *)PCI_CONFIG_COMMAND) = 0x07;
			//DEBUG_INFO("...command = 0x%08lx\n", *((volatile unsigned long *)PCI_CONFIG_COMMAND));
			for(i=0; i<1000000; i++);
			*((volatile unsigned short *)PCI_CONFIG_LATENCY) = 0x2000;
			for(i=0; i<1000000; i++);
			//DEBUG_INFO("...latency = 0x%08lx\n", *((volatile unsigned long *)PCI_CONFIG_LATENCY));

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8192EE)
			if ((GET_CHIP_VER(priv)!=VERSION_8188E) && (GET_CHIP_VER(priv)!=VERSION_8192E) && (GET_CHIP_VER(priv)!=VERSION_8812E))
#endif
			{
#if defined(CONFIG_RTL8196C_REVISION_B) || defined(CONFIG_RTL_8196C)
#if defined(CONFIG_NET_PCI)
#define REVR                                    0xB8000000
#define RTL8196C_REVISION_A     0x80000001
#define RTL8196C_REVISION_B     0x80000002
#endif
				if (REG32(REVR) == RTL8196C_REVISION_B) {
					REG32(0xb8b01000)=0x0f0f0f01; //Enhance for RTL8192c signal  
					for(i=0; i<1000000; i++);
					i=REG32(0xb8b01000);
					REG32(0xb9000354)=0xc940; //Card PCIE PHY initial  parameter for rtl8196c revision B
					REG32(0xb9000358)=0x24;
					for(i=0; i<1000000; i++);
					REG32(0xb9000354)=0x4270;
					REG32(0xb9000358)=0x25;
					for(i=0; i<1000000; i++);
					REG32(0xb9000354)=0x019E; //Card PCIE PHY initial  parameter for rtl8196c revision B
					REG32(0xb9000358)=0x23;
				}
#endif
#if defined(CONFIG_RTL8198_REVISION_B) || defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
#if !defined(CONFIG_RTL_92D_SUPPORT)
#if defined(CONFIG_NET_PCI)
#define BSP_REVR        0xB8000000
#define BSP_RTL8198_REVISION_A	0xC0000000
#define BSP_RTL8198_REVISION_B	0xC0000001 
#endif
#if !defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)

#if defined(CONFIG_RTL_8196E)
				if ((REG32(BSP_REVR) & 0xFFFFF000) == BSP_RTL8196E)
#else
				if ((REG32(BSP_REVR) >= BSP_RTL8198_REVISION_B) || ((REG32(BSP_REVR) & 0xFFFFF000) == BSP_RTL8197D)) 
#endif
				{
					if (priv->pshare->version_id!= VERSION_8192D) {
						REG32(dev->base_addr+0x354)=0xc940; //Card PCIE PHY initial  parameter for rtl8196c revision B
						REG32(dev->base_addr+0x358)=0x24;
					for(i=0; i<1000000; i++);
							REG32(dev->base_addr+0x354)=0x4270;
						REG32(dev->base_addr+0x358)=0x25;
					for(i=0; i<1000000; i++);
							REG32(dev->base_addr+0x354)=0x019E; //Card PCIE PHY initial  parameter for rtl8196c revision B
						REG32(dev->base_addr+0x358)=0x23;
					}
				}
#endif
#endif
#endif
			}
			#if defined(CONFIG_RTL_88E_SUPPORT)
                        else
			{
					REG32(dev->base_addr+0x354)=0x4104; //Card PCIE PHY initial  parameter for rtl8196c revision B
                                        for(i=0; i<1000000; i++);
                    	REG32(dev->base_addr+0x358)=0x24;
			}
			#endif
#ifndef NOT_RTK_BSP
#ifdef CONFIG_RTL_8198B
			REG32(BSP_WDTCNTRR) |= BSP_WDT_KICK;
#else
			REG32(0xB800311C) |=  1 << 23;
#endif
#endif
#ifdef  CONFIG_WLAN_HAL_8192EE
		if (GET_CHIP_VER(priv)==VERSION_8192E) {
            if (RT_STATUS_FAILURE == HalAssociateNic(priv, TRUE)) {
                printk("HalAssociateNic Failed \n");
            } 
            else {
                printk("HalAssociateNic OK \n");
            }

            GET_HW(priv)->MIMO_TR_hw_support = GET_HAL_INTERFACE(priv)->GetChipIDMIMOHandler(priv);
       
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_MAC_IO_ENABLE, &bVal);
            if ( bVal ) {
                if (RT_STATUS_SUCCESS == GET_HAL_INTERFACE(priv)->StopHWHandler(priv)) {
                    printk("StopHW Succeed\n");
                }
                else {
                    GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);            
                    errorFlag |= DRV_ER_CLOSE_STOP_HW;
                    GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);                     
                    panic_printk("StopHW Failed\n");
                }                
            }
            else {
                //printk("Can't write MACID register\n");
            }
	   } else
#endif
		{
			check_chipID_MIMO(priv);
			//Exception Case
			if ( check_MAC_IO_Enable(priv) ) {
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
				if((GET_CHIP_VER(priv)==VERSION_8192C) || (GET_CHIP_VER(priv)==VERSION_8188C) || (GET_CHIP_VER(priv)==VERSION_8192D)) {
					RTL_W8(CR,0);
					RTL_W8(SYS_FUNC_EN+1, RTL_R8(SYS_FUNC_EN+1) & 0xfe);
					RTL_W8(SYS_FUNC_EN+1, RTL_R8(SYS_FUNC_EN+1) |1);
				}
#endif			
				rtl8192cd_stop_hw(priv);
			}
		}
		}
       else if (TYPE_EMBEDDED == ((priv->pshare->type>>TYPE_SHIFT) & TYPE_MASK) ) {
            printk("TYPE_EMBEDDED\n");
				priv->pshare->version_id = VERSION_8881A;
				priv->pshare->use_hal = 1;
//#ifdef  CONFIG_SOC_RTL8881A
#if !defined(NOT_RTK_BSP)
            // TODO: Filen_debug, code below is temporary
            //Reset Letrx Bus and WL MAC
            #if 0 //96D
            //REG32(0xB8000010)|= 0x00008000;
            #else //8881A
            REG32(0xB8000010)|= 0x00003800;
            //Enable MAC_System(BIT(0)), MAC_Lextra_Bus(BIT(1))
            REG32(0xB80000DC)= 0x03;
            #endif
#endif  //CONFIG_SOC_RTL8881A

#ifdef  CONFIG_WLAN_HAL
            if (RT_STATUS_FAILURE == HalAssociateNic(priv, TRUE)) {
                printk("HalAssociateNic Failed \n");
            } 
            else {
                printk("HalAssociateNic OK \n");
            }

            GET_HW(priv)->MIMO_TR_hw_support = GET_HAL_INTERFACE(priv)->GetChipIDMIMOHandler(priv);

            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_MAC_IO_ENABLE, &bVal);
            if ( bVal ) {
                if (RT_STATUS_SUCCESS == GET_HAL_INTERFACE(priv)->StopHWHandler(priv)) {
                    printk("StopHW Succeed\n");
                }
                else {
                    GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);            
                    errorFlag |= DRV_ER_CLOSE_STOP_HW;
                    GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);                                      
                    panic_printk("StopHW Failed\n");
                }                
            }
            else {
                //printk("Can't write MACID register\n");
            }            
#endif  //CONFIG_WLAN_HAL
        }		
	}
/*	==========>> maybe later
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
		rtl8192cd_ePhyInit(priv);
*/

#ifdef CONFIG_NET_PCI
	if (((wdev->type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS) {
		dev->irq = pdev->irq;
		pci_set_drvdata(pdev, dev);
	}
	else
#endif
	{
		dev->irq = wdev->irq;
	}

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
register_driver:
#endif

#if !defined(__LINUX_2_6__) || defined(CONFIG_COMPAT_NET_DEV_OPS)
	dev->open = rtl8192cd_open;
	dev->stop = rtl8192cd_close;
#ifdef __KERNEL__
	dev->set_multicast_list = rtl8192cd_set_rx_mode;
#endif
	dev->hard_start_xmit = rtl8192cd_start_xmit;
	dev->get_stats = rtl8192cd_get_stats;
	dev->do_ioctl = rtl8192cd_ioctl;
#ifdef __KERNEL__
	dev->set_mac_address = rtl8192cd_set_hwaddr;
#endif
#else
	dev->netdev_ops = &rtl8192cd_netdev_ops;
#endif

#ifdef CONFIG_RTL8672
	dev->priv_flags = IFF_DOMAIN_WLAN;
#endif

#ifdef __ECOS
	dev->isr = __rtl8192cd_interrupt;
	dev->dsr= interrupt_dsr;
	dev->can_xmit = can_xmit;
#endif

#ifdef __KERNEL__
	rc = register_netdev(dev);
	if (rc)
		goto err_out_iomap;
#endif

#ifdef CONFIG_RTL8672
	// Added by Mason Yu
	// MBSSID Port Mapping
	wlanDev[wlanDevNum].dev_pointer = dev;
	wlanDev[wlanDevNum].dev_ifgrp_member = 0;
	wlanDevNum++;
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (GET_ROOT_PRIV(priv) == NULL)  // is root interface
#endif
		DEBUG_INFO("Init %s, base_addr=%08x, irq=%d\n",
			dev->name, (UINT)dev->base_addr,  dev->irq);

#else //  __DRAYTEK_OS__
	regs = (void *)wdev->base_addr;
	dev->base_addr = (unsigned long)wdev->base_addr;
	priv->pshare->ioaddr = (UINT)regs;

#ifdef UNIVERSAL_REPEATER
	if (GET_ROOT_PRIV(priv) == NULL)  // is root interface
#endif
		DEBUG_INFO("Init %s, base_addr=%08x\n",
			dev->name, (UINT)dev->base_addr);

#endif // __DRAYTEK_OS__


#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (GET_ROOT_PRIV(priv) == NULL)  // is root interface
#endif
	{
#ifdef CONFIG_NET_PCI

#if defined(CONFIG_RTL8196C_REVISION_B) || defined(CONFIG_RTL_8196C)
			#if defined(CONFIG_NET_PCI)
				#define REVR                                    0xB8000000
				#define RTL8196C_REVISION_A     0x80000001
				#define RTL8196C_REVISION_B     0x80000002
			#endif
					if (REG32(REVR) == RTL8196C_REVISION_B) {
						REG32(0xb9000354)=0xc940; //Card PCIE PHY initial  parameter for rtl8196c revision B
								REG32(0xb9000358)=0x24;
						for(i=0; i<1000000; i++);
								REG32(0xb9000354)=0x4270;
								REG32(0xb9000358)=0x25;
						for(i=0; i<1000000; i++);
						REG32(0xb9000354)=0x019E; //Card PCIE PHY initial  parameter for rtl8196c revision B
								REG32(0xb9000358)=0x23;
					}
#endif
		if (((wdev->type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS) {
			if (cache_size != SMP_CACHE_BYTES) {
				printk(KERN_INFO "%s: PCI cache line size set incorrectly (%i bytes) by BIOS/FW, ", dev->name, cache_size);
		        if (cache_size > SMP_CACHE_BYTES)
	    	        printk("expecting %i\n", SMP_CACHE_BYTES);
	        	else {
	            	printk("correcting to %i\n", SMP_CACHE_BYTES);
					pci_write_config_byte(pdev, PCI_CACHE_LINE_SIZE, SMP_CACHE_BYTES >> 2);
	    	    }
		    }

	    	/* enable busmastering and memory-write-invalidate */
		    pci_read_config_word(pdev, PCI_COMMAND, &pci_command);
	    	if (!(pci_command & PCI_COMMAND_INVALIDATE)) {
	        	pci_command |= PCI_COMMAND_INVALIDATE;
		        pci_write_config_word(pdev, PCI_COMMAND, pci_command);
	    	}
		    pci_set_master(pdev);
		}
#endif
	}

#ifdef __ECOS
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_WLAN_HAL_8192EE) //mark_ecos
	RTLWIFINIC_GPIO_init_priv(priv); 
#endif
#else
#ifdef _INCLUDE_PROC_FS_
#ifdef __KERNEL__
	rtl8192cd_proc_init(dev);
#ifdef PERF_DUMP
	{
		#include <linux/proc_fs.h>

		struct proc_dir_entry *res;
	    res = create_proc_entry("perf_dump", 0, NULL);
	    if (res) {
    	    res->read_proc = read_perf_dump;
    	    res->write_proc = flush_perf_dump;
			res->data = (void *)dev;
	    }
	}
#endif
#endif
#endif
#endif

	// set some default value of mib
	set_mib_default(priv);

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (GET_ROOT_PRIV(priv) == NULL)  // is root interface
#endif
	{
#ifndef __DRAYTEK_OS__
#ifdef WDS
		wds_num = (priv->pshare->type>>WDS_SHIFT) & WDS_MASK;
		strcpy(baseDevName, dev->name);

		for (i=0; i<wds_num; i++) {
#ifdef NETDEV_NO_PRIV
			struct rtl8192cd_wds_priv* priv_wds;
			dev = alloc_etherdev(sizeof(struct rtl8192cd_wds_priv));
#else
			dev = alloc_etherdev(0);
#endif
			if (!dev) {
				printk(KERN_ERR "alloc_etherdev() wds error!\n");
				rc = -ENOMEM;
	    	   	goto err_out_dev;
			}

#if !defined(__LINUX_2_6__) || defined(CONFIG_COMPAT_NET_DEV_OPS)
		    dev->open = rtl8192cd_open;
		    dev->stop = rtl8192cd_close;
		    dev->hard_start_xmit = rtl8192cd_start_xmit;
		    dev->get_stats = rtl8192cd_get_stats;
#ifdef __KERNEL__
		    dev->set_mac_address = rtl8192cd_set_hwaddr;
#endif
#else
		    dev->netdev_ops = &rtl8192cd_netdev_ops;
#endif

#ifdef CONFIG_RTL8672
			dev->priv_flags = IFF_DOMAIN_WLAN;
#endif

#ifdef __ECOS
			//dev->isr = __rtl8192cd_interrupt;
			//dev->dsr= interrupt_dsr;
			dev->can_xmit = can_xmit;
#endif

			priv->wds_dev[i] = dev;
			strcpy(dev->name, baseDevName);
			strcat(dev->name, "-wds%d");
			
#ifdef NETDEV_NO_PRIV
			priv_wds = (struct rtl8192cd_wds_priv*)netdev_priv(dev);
			priv_wds->wlan_priv = priv;
#else
			dev->priv = priv;
#endif
#ifdef __KERNEL__
		    rc = register_netdev(dev);
			if (rc) {
				printk(KERN_ERR "register_netdev() wds error!\n");
				goto err_out_dev;
			}
#endif
		}
#endif // WDS

#ifdef CONFIG_RTK_MESH
		mesh_num = (priv->pshare->type>>MESH_SHIFT) & MESH_MASK;

#ifndef WDS
		strcpy(baseDevName, dev->name);
#endif
		if(mesh_num>0) {
			GET_MIB(priv)->dot1180211sInfo.mesh_enable = 1;
			dev = alloc_etherdev(0);	// mesh allocate ethernet device BUT don't have priv memory (Because share root priv)
			if (!dev) {
				printk(KERN_ERR "alloc_etherdev() mesh error!\n");
				rc = -ENOMEM;
				goto err_out_iomap;
			}
			dev->base_addr = 1;
#if !defined(__LINUX_2_6__) || defined(CONFIG_COMPAT_NET_DEV_OPS)
			dev->open = rtl8192cd_open;
			dev->stop = rtl8192cd_close;
			dev->hard_start_xmit = rtl8192cd_start_xmit;
			dev->get_stats = rtl8192cd_get_stats;
			dev->set_mac_address = rtl8192cd_set_hwaddr;
			dev->do_ioctl = rtl8192cd_ioctl;
#else
			dev->netdev_ops = &rtl8192cd_netdev_ops;
#endif

#ifdef CONFIG_RTL8672
			dev->priv_flags = IFF_DOMAIN_WLAN;
#endif

			priv->mesh_dev = dev; // NO priv zone dev
			strcpy(dev->name, baseDevName);
			strcat(dev->name, "-msh%d");
			dev->priv = priv;		// mesh priv pointer to root's priv
			rc = register_netdev(dev);
			if (rc) {
				printk(KERN_ERR "register_netdev() mesh error!\n");
				goto err_out_iomap;
			}
		} // end of if(mesh_num>0)

			priv->RreqEnd = 0;
			priv->RreqBegin = 0;

			pann_mpp_tb = (struct mpp_tb*)kmalloc(sizeof(struct mpp_tb), GFP_ATOMIC);
			if(!pann_mpp_tb)
			{
				rc = -ENOMEM;
				printk("allocate pann_mpp_tb error!!\n");
				goto err_out_free;
			}
			init_mpp_pool(pann_mpp_tb);
			proxy_table = (struct hash_table*)kmalloc(sizeof(struct hash_table), GFP_ATOMIC);
			if(!proxy_table)
			{
				rc = -ENOMEM;
				printk("allocate proxy_table error!!\n");
				goto err_out_free;
			}
			memset((void*)proxy_table, 0, sizeof(struct hash_table));

#ifdef PU_STANDARD
			//pepsi
			proxyupdate_table = (struct hash_table*)kmalloc(sizeof(struct hash_table), GFP_ATOMIC);
			if(!proxyupdate_table)
			{
				rc = -ENOMEM;
				printk("allocate proxyupdate_table error!!\n");
				goto err_out_free;
			}
			memset((void*)proxyupdate_table, 0, sizeof(struct hash_table));
#endif

			pathsel_queue = (DOT11_QUEUE2 *)kmalloc((sizeof(DOT11_QUEUE2)), GFP_ATOMIC);
			if (!pathsel_queue) {
				rc = -ENOMEM;
				printk(KERN_ERR "Can't kmalloc for PATHSELECTION_QUEUE (size %d)\n", sizeof(DOT11_QUEUE));
				goto err_out_free;
			}
			memset((void *)pathsel_queue, 0, sizeof (DOT11_QUEUE2));
#ifdef _11s_TEST_MODE_
			receiver_queue = (DOT11_QUEUE2 *)kmalloc((sizeof(DOT11_QUEUE2)), GFP_ATOMIC);
			if (!receiver_queue) {
				rc = -ENOMEM;
				printk(KERN_ERR "Can't kmalloc for receiver_queue (size %d)\n", sizeof(DOT11_QUEUE));
				goto err_out_free;
			}
			memset((void *)receiver_queue, 0, sizeof (DOT11_QUEUE2));
			pgalileo_poll = (struct Galileo_poll *)	kmalloc((sizeof( struct Galileo_poll)), GFP_ATOMIC);
			if (!pgalileo_poll) {
				rc = -ENOMEM;
				printk(KERN_ERR "Can't kmalloc for pgalileo_poll (size %d)\n", sizeof(struct Galileo_poll));
				goto err_out_free;
			}
#endif
			pathsel_table = (struct hash_table*)kmalloc(sizeof(struct hash_table), GFP_ATOMIC);
			if(!pathsel_table)
			{
				rc = -ENOMEM;
				printk("allocate pathsel_table error!!\n");
				goto err_out_free;
			}
			memset((void*)pathsel_table, 0, sizeof(struct hash_table));

			mesh_rreq_retry_queue = (struct hash_table*)kmalloc(sizeof(struct hash_table), GFP_ATOMIC);
			if(!mesh_rreq_retry_queue)
			{
				rc = -ENOMEM;
				printk("allocate mesh_rreq_retry_queue error!!\n");
				goto err_out_free;
			}
			memset((void*)mesh_rreq_retry_queue, 0, sizeof(struct hash_table));

			rc = init_hash_table(proxy_table, PROXY_TABLE_SIZE, MACADDRLEN, sizeof(struct proxy_table_entry), crc_hashing, search_default, insert_default, delete_default,traverse_default);
			if(rc == HASH_TABLE_FAILED)
			{
				printk("init_hash_table \"proxy_table\" error!!\n");
			}

#ifdef PU_STANDARD
			//pepsi
			rc = init_hash_table(proxyupdate_table, 8, sizeof(UINT8), sizeof(struct proxyupdate_table_entry), PU_hashing, search_default, insert_default, delete_default,traverse_default);
			if(rc == HASH_TABLE_FAILED)
			{
				printk("init_hash_table \"proxyupdate_table\" error!!\n");
			}
#endif
			rc = init_hash_table(pathsel_table, 8, MACADDRLEN, sizeof(struct path_sel_entry), crc_hashing, search_default, insert_default, delete_default,traverse_default);
			if(rc == HASH_TABLE_FAILED)
			{
				printk("init_hash_table \"pathsel_table\" error!!\n");
			}

			rc = init_hash_table(mesh_rreq_retry_queue, DATA_SKB_BUFFER_SIZE, MACADDRLEN, sizeof(struct mesh_rreq_retry_entry), crc_hashing, search_default, insert_default, delete_default,traverse_default);
			if(rc == HASH_TABLE_FAILED)
			{
				printk("init_hash_table \"mesh_rreq_retry_queue\" error!!\n");
			}

			for(i = 0; i < (1 << mesh_rreq_retry_queue->table_size_power); i++)
			{
				(((struct mesh_rreq_retry_entry*)(mesh_rreq_retry_queue->entry_array[i].data))->ptr) = (struct pkt_queue*)kmalloc(sizeof(struct pkt_queue), GFP_ATOMIC);
				if (!(((struct mesh_rreq_retry_entry*)(mesh_rreq_retry_queue->entry_array[i].data))->ptr)) {
					rc = -ENOMEM;
					printk(KERN_ERR "Can't kmalloc for mesh_rreq_retry_entry (size %d)\n", sizeof(struct pkt_queue));
					goto err_out_free;
				}
				memset((void *)((((struct mesh_rreq_retry_entry*)(mesh_rreq_retry_queue->entry_array[i].data))->ptr)), 0, sizeof (struct pkt_queue));
			}

#ifdef PU_STANDARD
			priv->proxyupdate_table = proxyupdate_table;
#endif
#ifdef _11s_TEST_MODE_
			priv->receiver_queue = receiver_queue;
			priv->pshare->galileo_poll = pgalileo_poll ;
#endif
			priv->proxy_table = proxy_table;
			priv->pathsel_queue = pathsel_queue;
			priv->pann_mpp_tb = pann_mpp_tb;
			priv->pathsel_table = pathsel_table;
			priv->mesh_rreq_retry_queue = mesh_rreq_retry_queue;
			//=========================================================
#endif // CONFIG_RTK_MESH


#endif  // __DRAYTEK_OS__


        //3 Require Descriptor Memory
        //Method:
        //  1.) Static Memory
        //  2.) Allocate memory from OS
#ifdef  CONFIG_WLAN_HAL
		if (GET_CHIP_VER(priv)==VERSION_8192E || GET_CHIP_VER(priv)==VERSION_8881A) {
        // Do Nothing, encapsulate in WlanHAL
		} else
#endif
		{
#ifdef PRIV_STA_BUF
#ifdef CONCURRENT_MODE
		page_ptr = (unsigned char *)
			(((unsigned long)&desc_buf[priv->pshare->wlandev_idx]) + (PAGE_SIZE - (((unsigned long)&desc_buf[priv->pshare->wlandev_idx]) & (PAGE_SIZE-1))));
		phw->ring_buf_len = ((unsigned long)&desc_buf[priv->pshare->wlandev_idx]) + (sizeof(desc_buf)/NUM_WLAN_IFACE) - ((unsigned long)page_ptr);
		phw->ring_dma_addr = virt_to_bus(page_ptr);
		page_ptr = (unsigned char *)KSEG1ADDR(page_ptr);
#else
		page_ptr = (unsigned char *)
			(((unsigned long)desc_buf) + (PAGE_SIZE - (((unsigned long)desc_buf) & (PAGE_SIZE-1))));
		phw->ring_buf_len = (unsigned long)desc_buf + sizeof(desc_buf) - (unsigned long)page_ptr;
		phw->ring_dma_addr = virt_to_bus(page_ptr);
		page_ptr = (unsigned char *)KSEG1ADDR(page_ptr);
#endif		
#else
#ifdef CONFIG_NET_PCI
		if (IS_PCIBIOS_TYPE)
			page_ptr = pci_alloc_consistent(priv->pshare->pdev, DESC_DMA_PAGE_SIZE, (dma_addr_t *)&phw->ring_dma_addr);
		else
#endif
		{
#ifdef __DRAYTEK_OS__
		page_ptr = rtl8185_malloc(DESC_DMA_PAGE_SIZE, 1);	// allocate non-cache buffer
#else
		page_ptr = kmalloc(DESC_DMA_PAGE_SIZE, GFP_ATOMIC);
#endif
		}

		if (page_ptr == NULL) {
			rc = -ENOMEM;
			printk(KERN_ERR "can't allocate descriptior page, abort!\n");
			goto err_out_dev;
		}

		phw->alloc_dma_buf = (unsigned long)page_ptr;
#if defined(NOT_RTK_BSP)		
		page_align_phy = (PAGE_SIZE - (((unsigned long)page_ptr) & (PAGE_SIZE-1)));
#endif	
		page_ptr = (unsigned char *)
			(((unsigned long)page_ptr) + (PAGE_SIZE - (((unsigned long)page_ptr) & (PAGE_SIZE-1))));
		phw->ring_buf_len = phw->alloc_dma_buf + DESC_DMA_PAGE_SIZE - ((unsigned long)page_ptr);
#if defined(NOT_RTK_BSP)
		phw->ring_dma_addr = phw->ring_dma_addr + page_align_phy;  
#else
		phw->ring_dma_addr = virt_to_bus(page_ptr);
#endif

#ifdef __MIPSEB__
		page_ptr = (unsigned char *)KSEG1ADDR(page_ptr);
#endif
#endif

		DEBUG_INFO("page_ptr=%lx, size=%ld\n",  (unsigned long)page_ptr, (unsigned long)DESC_DMA_PAGE_SIZE);
		phw->ring_virt_addr = (unsigned long)page_ptr;
		}

#ifdef CONFIG_RTL8190_PRIV_SKB
		init_priv_skb_buf(priv);
#endif

#ifdef PRIV_STA_BUF
		init_priv_sta_buf(priv);
#endif

#ifdef CONFIG_RTL_92D_DMDP
		if_priv[priv->pshare->wlandev_idx] = (u32)priv;
#endif

	}

	INIT_LIST_HEAD(&priv->asoc_list); // init assoc_list first because webs may get sta_num even it is not open,
																// and it will cause exception if it is not init, david+2008-03-05
#ifdef EN_EFUSE
		ReadAdapterInfo8192CE(priv);
#endif	
	printk("=====>>EXIT rtl8192cd_init_one <<=====\n");

#if defined(CONFIG_RTL_8196D) || defined(CONFIG_RTL_8196E)
	if(priv && RTL_R32(GPIO_PIN_CTRL))
	{
		printk("<%s>LZQ: GPIO_PIN_CTRL[0x%x] \n",__FUNCTION__, GPIO_PIN_CTRL);
		printk("<%s>LZQ: before read tmpReg[0x%x] \n",__FUNCTION__, RTL_R32(GPIO_PIN_CTRL));
		RTL_W32(GPIO_PIN_CTRL,RTL_R32(GPIO_PIN_CTRL)&(0x0));
		printk("<%s>LZQ: after read tmpReg[0x%x] \n",__FUNCTION__, RTL_R32(GPIO_PIN_CTRL));
	}
#endif
	
#ifdef __KERNEL__
	return 0;
#else
	//return (void *)dev;
	return (void *)tmp_dev;
#endif

err_out_dev:

#ifdef __KERNEL__
	unregister_netdev(dev);

err_out_iomap:
#endif

#ifdef CONFIG_NET_PCI
	if (((wdev->type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS) {
#ifndef USE_IO_OPS
	    iounmap(regs);
#endif
	}

err_out_res:

	if (((wdev->type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS) {
#ifdef USE_IO_OPS
		release_region(dev->base_addr, pci_resource_len(pdev, 0));
////	release_region(dev->base_addr, pci_resource_len(pdev, 2));
#else
	    pci_release_regions(pdev);
#endif
	}

err_out_disable:

	if (((wdev->type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS)
	    pci_disable_device(pdev);
#endif // CONFIG_NET_PCI

err_out_free:
	if (pmib){
#ifdef RTL8192CD_VARIABLE_USED_DMEM
		rtl8192cd_dmem_free(PMIB, pmib);
#else
		kfree(pmib);
#endif
	}

#ifdef  CONFIG_RTK_MESH
	if(proxy_table)
	{
		remove_hash_table(proxy_table);
		kfree(proxy_table);
	}
	if(mesh_rreq_retry_queue)
	{
		for (i=0; i< (1 << mesh_rreq_retry_queue->table_size_power); i++)
		{
			if(((struct mesh_rreq_retry_entry*)(mesh_rreq_retry_queue->entry_array[i].data))->ptr)
			{
				kfree(((struct mesh_rreq_retry_entry*)(mesh_rreq_retry_queue->entry_array[i].data))->ptr);
			}
		}
		remove_hash_table(mesh_rreq_retry_queue);
		kfree(mesh_rreq_retry_queue);
	}

	// add by chuangch 2007.09.13
	if(pathsel_table)
	{
		remove_hash_table(pathsel_table);
		kfree(pathsel_table);
	}

	if(pann_mpp_tb)
		kfree(pann_mpp_tb);

	if (pathsel_queue)
		kfree(pathsel_queue);
#ifdef	_11s_TEST_MODE_
	if (receiver_queue)
		kfree(receiver_queue);
#endif
#endif	// CONFIG_RTK_MESH

#ifdef P2P_SUPPORT
	if(priv->p2pPtr)
		kfree(priv->p2pPtr);
#endif

	if (pevent_queue)
		kfree(pevent_queue);
#ifdef CONFIG_RTL_WAPI_SUPPORT
	if (wapiEvent_queue)
		kfree(wapiEvent_queue);
	#if	defined(MBSSID)
	if (wapiVapEvent_queue)
		kfree(wapiVapEvent_queue);
	#endif

#endif

#ifndef PRIV_STA_BUF
	if (phw)
		kfree(phw);
	if (pshare)	// david
		kfree(pshare);
	if (pwlan_hdr_poll)
		kfree(pwlan_hdr_poll);
	if (pwlanllc_hdr_poll)
		kfree(pwlanllc_hdr_poll);
	if (pwlanbuf_poll)
		kfree(pwlanbuf_poll);
	if (pwlanicv_poll)
		kfree(pwlanicv_poll);
	if (pwlanmic_poll)
		kfree(pwlanmic_poll);
#endif
	if (pwlan_acl_poll)
		kfree(pwlan_acl_poll);

#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_)	// below code copy above ACL code
	if (pmesh_acl_poll)
		kfree(pmesh_acl_poll);
#endif

	if (Eap_packet)
		kfree(Eap_packet);
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
	if (wpa_global_info)
		kfree(wpa_global_info);
#endif

#ifdef __LINUX_2_6__
	free_netdev(dev);
#else
    kfree(dev);
#endif
    wdev->priv = NULL;
	printk("=====>>EXIT rtl8192cd_init_one2(%d) <<=====\n", rc);

#ifdef __KERNEL__
    return rc;
#else
	return NULL;
#endif
}


#ifdef CONFIG_RTL_STP
extern int rtl865x_wlanIF_Init(struct net_device *dev);
static int rtl_pseudo_dev_set_hwaddr(struct net_device *dev, void *addr)
{
	unsigned long flags;
	int i;
	unsigned char *p;

	p = ((struct sockaddr *)addr)->sa_data;
 	local_irq_save(flags);
	for (i = 0; i<MACADDRLEN; ++i) {
		dev->dev_addr[i] = p[i];
	}
	local_irq_restore(flags);
	return SUCCESS;
}

#if defined(__LINUX_2_6__) && !defined(CONFIG_COMPAT_NET_DEV_OPS)
static const struct net_device_ops rtl8192cd_rtl_pseudodev_ops = {
        .ndo_open               = rtl8192cd_open,
        .ndo_stop               = rtl8192cd_close,
        .ndo_set_mac_address    = rtl_pseudo_dev_set_hwaddr,
        .ndo_get_stats          = rtl8192cd_get_stats,
        .ndo_do_ioctl           = rtl8192cd_ioctl,
        .ndo_start_xmit         = rtl8192cd_start_xmit,
};
#endif

void rtl_pseudo_dev_init(void* priv)
{
	struct net_device *dev;

/*	printk("[%s][%d] priv of %s\n", __FUNCTION__, __LINE__, ((struct rtl8192cd_priv*)priv)->dev->name);*/
	dev = alloc_etherdev(0);
	if (dev == NULL) {
		printk("alloc_etherdev() pseudo port5 error!\n");
		return;
	}

#if !defined(__LINUX_2_6__) || defined(CONFIG_COMPAT_NET_DEV_OPS)
	dev->open = rtl8192cd_open;
	dev->stop = rtl8192cd_close;
	dev->hard_start_xmit = rtl8192cd_start_xmit;
	dev->get_stats = rtl8192cd_get_stats;
	dev->do_ioctl = rtl8192cd_ioctl;
	dev->set_mac_address = rtl_pseudo_dev_set_hwaddr;
#else
	dev->netdev_ops = &rtl8192cd_rtl_pseudodev_ops;
#endif
	dev->priv = priv;
	strcpy(dev->name, "port5");
	memcpy((char*)dev->dev_addr,"\x00\xe0\x4c\x81\x86\x86", MACADDRLEN);
	if (register_netdev(dev)) {
		printk(KERN_ERR "register_netdev() wds error!\n");
	}
	rtl865x_wlanIF_Init(dev);
}
#endif

#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
__DRAM_IN_865X int passThruStatusWlan;
int passThruWanIdx;
static struct proc_dir_entry *resPassThruWlan=NULL;
static char passThru_flag_wlan[10];

static int rtl_passthru_pseudo_dev_set_hwaddr(struct net_device *dev, void *addr)
{
	unsigned long flags;
	int i;
	unsigned char *p;

	p = ((struct sockaddr *)addr)->sa_data;
 	local_irq_save(flags);
	for (i = 0; i<MACADDRLEN; ++i) {
		dev->dev_addr[i] = p[i];
	}
	local_irq_restore(flags);
	return SUCCESS;
}

#if defined(__LINUX_2_6__) && !defined(CONFIG_COMPAT_NET_DEV_OPS)
static const struct net_device_ops rtl8192cd_pseudodev_ops = {
        .ndo_open               = rtl8192cd_open,
        .ndo_stop               = rtl8192cd_close,
        .ndo_set_mac_address    = rtl_passthru_pseudo_dev_set_hwaddr,
        .ndo_set_multicast_list = rtl8192cd_set_rx_mode,
        .ndo_get_stats          = rtl8192cd_get_stats,
        .ndo_do_ioctl           = rtl8192cd_ioctl,
        .ndo_start_xmit         = rtl8192cd_start_xmit,
};
#endif

void rtl_passthru_pseudo_dev_init(void *priv)
{
	struct net_device *dev;
	//struct rtl8192cd_priv *dp;

	dev = alloc_etherdev(sizeof(struct rtl8192cd_priv));
	if (dev == NULL) {
		printk("alloc_etherdev() pseudo pwlan0 error!\n");
		return;
	}

#if !defined(__LINUX_2_6__) || defined(CONFIG_COMPAT_NET_DEV_OPS)
		dev->open = rtl8192cd_open;
		dev->stop = rtl8192cd_close;
#ifndef __ECOS
		dev->set_multicast_list = rtl8192cd_set_rx_mode;
#endif
		dev->hard_start_xmit = rtl8192cd_start_xmit;
		dev->get_stats = rtl8192cd_get_stats;
		dev->do_ioctl = rtl8192cd_ioctl;
#ifndef __ECOS
		dev->set_mac_address = rtl_passthru_pseudo_dev_set_hwaddr;
#endif
#else
		dev->netdev_ops = &rtl8192cd_pseudodev_ops;
#endif

#ifndef NETDEV_NO_PRIV
	dev->priv = priv;
#endif

	strcpy(dev->name, "pwlan%d");
	memcpy((char*)dev->dev_addr,"\x00\xe0\x4c\x81\x96\x96", MACADDRLEN);
	if (register_netdev(dev)) {
		printk(KERN_ERR "register_netdev() pwlan0 error!\n");
	}

	((struct rtl8192cd_priv *)priv)->pWlanDev=dev;	//pWlanDev point to the virtual pwlan0
}


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

static int wlan_custom_Passthru_read_proc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
	int len;	
	len = sprintf(page, "%s\n", passThru_flag_wlan);
	if (len <= off+count) 
		*eof = 1;

	*start = page + off;
	len -= off;

	if (len>count) 
		len = count;

	if (len<0) len = 0;

	return len;
}

static int wlan_custom_Passthru_write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	int flag /*,i*/;

	if (buffer && !copy_from_user(&passThru_flag_wlan, buffer, count))
	{			
		flag=(int)atoi_dec(passThru_flag_wlan);
		
		passThruStatusWlan=flag;
		
		return count;
	}
	return -EFAULT;
}

int rtl_wlan_customPassthru_init(void)
{
	int	wlan_idx;

	for(wlan_idx=0;wlan_idx<sizeof(wlan_device)/sizeof(struct _device_info_);wlan_idx++)
	{
		if (wlan_device[wlan_idx].priv!=NULL)
			break;
	}

	if (wlan_idx==sizeof(wlan_device)/sizeof(struct _device_info_))
		return -EINVAL;

	passThruWanIdx = wlan_idx;
	//initial
	memset(passThru_flag_wlan,0, sizeof(passThru_flag_wlan));
	//strcpy(passThru_flag_wlan,"0");
	passThruStatusWlan=0;
	
	resPassThruWlan = create_proc_entry("custom_Passthru_wlan", 0, NULL);	
	if(resPassThruWlan)
	{
		resPassThruWlan->read_proc = wlan_custom_Passthru_read_proc;
		resPassThruWlan->write_proc = wlan_custom_Passthru_write_proc;
	}

		
	rtl_passthru_pseudo_dev_init(wlan_device[passThruWanIdx].priv);
	
	return 0;
}

void __exit rtl_wlan_customPassthru_exit(void)
{
	if (resPassThruWlan) {
		remove_proc_entry("custom_Passthru_wlan", resPassThruWlan);				
		resPassThruWlan = NULL;
	}
}
#endif



#if defined(__DRAYTEK_OS__) && defined(WDS)
int rtl8192cd_add_wds(struct net_device *dev, struct net_device *wds_dev, unsigned char *addr)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	int wds_num=priv->pmib->dot11WdsInfo.wdsNum;

	priv->pmib->dot11WdsInfo.dev[wds_num] = wds_dev;
	memcpy(priv->pmib->dot11WdsInfo.entry[wds_num].macAddr, addr, 6);
	wds_dev->priv = priv;
	wds_dev->base_addr = 0;
	priv->pmib->dot11WdsInfo.wdsNum++;

	if (!priv->pmib->dot11WdsInfo.wdsEnabled)
		priv->pmib->dot11WdsInfo.wdsEnabled = 1;

	if (netif_running(priv->dev))
		create_wds_tbl(priv);

	DEBUG_INFO("\r\nAdd WDS: %02x%02x	%02x%02x%02x%02x\n",
		addr[0],addr[1],addr[2],addr[3],addr[4],addr[5]);
}
#endif


#ifdef CONFIG_NET_PCI
static int MDL_DEVINIT rtl8192cd_init_pci(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	//int dev_num=0;//timmy_modify
	int ret=0;
#ifdef MBSSID
	int i;
#endif

	printk("rtl8192cd_init_pci in!!!!!!!\n");

#ifdef RTL8192CD_VARIABLE_USED_DMEM
	/* For D-MEM allocation system's initialization : It would before ALL processes */
	rtl8192cd_dmem_init();
#endif

	if (wlan_index<sizeof(wlan_device)/sizeof(struct _device_info_)) {	
		ret = rtl8192cd_init_one(pdev, ent, &wlan_device[wlan_index], -1);
#ifdef UNIVERSAL_REPEATER
		if (ret == 0) {
			ret = rtl8192cd_init_one(pdev, ent, &wlan_device[wlan_index], -1);
		}
#endif
#ifdef MBSSID
		if (ret == 0) {
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
				ret = rtl8192cd_init_one(pdev, ent, &wlan_device[wlan_index], i);
				if (ret != 0) {
					printk("Init fail!\n");
					return ret;
				}
			}
		}

		if (ret == 0) 
		wlan_index++;
#endif
	} else {
		printk("Too many in, current is :%d, support is %d\n",wlan_index, sizeof(wlan_device)/sizeof(struct _device_info_));
	}
	
		return ret;
}


static void MDL_DEVEXIT rtk_remove_one(struct pci_dev *pdev)
{
    struct net_device *dev = pci_get_drvdata(pdev);
	
#ifdef NETDEV_NO_PRIV
    struct rtl8192cd_priv *priv = ((struct rtl8192cd_priv *)netdev_priv(dev))->wlan_priv;
#else
    struct rtl8192cd_priv *priv = dev->priv;
#endif

    if (!dev)
        BUG();

    iounmap((void *)priv->dev->base_addr);
    pci_release_regions(pdev);
    pci_disable_device(pdev);
    pci_set_drvdata(pdev, NULL);

	if (wlan_index > 0) 
		wlan_index--;
}


static struct pci_device_id MDL_DEVINITDATA rtl8192cd_pci_tbl[] =
{
/*
	{ PCI_VENDOR_ID_REALTEK, 0x8190,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
*/
#ifdef CONFIG_RTL_92D_SUPPORT
	{ PCI_VENDOR_ID_REALTEK, 0x8193,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
#endif
	{ PCI_VENDOR_ID_REALTEK, 0x8191,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },

	{ PCI_VENDOR_ID_REALTEK, 0x8171,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },

	{ PCI_VENDOR_ID_REALTEK, 0x8178,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },

	{ PCI_VENDOR_ID_REALTEK, 0x8176,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },

	{ PCI_VENDOR_ID_REALTEK, 0x8174,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },

#ifdef CONFIG_RTL_88E_SUPPORT
    { PCI_VENDOR_ID_REALTEK, 0x8179,
      PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
#endif
#if defined(CONFIG_WLAN_HAL_8192EE)
    { PCI_VENDOR_ID_REALTEK, 0x8170,
      PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
    { PCI_VENDOR_ID_REALTEK, 0x818b,//0x817b
      PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },      
#endif
#ifdef CONFIG_RTL_8812_SUPPORT //eric_8812 ??
	{ PCI_VENDOR_ID_REALTEK, 0x8812,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
#endif
	{ },
};


MODULE_DEVICE_TABLE(pci, rtl8192cd_pci_tbl);

static struct pci_driver rtl8192cd_driver = {
	name:		DRV_NAME,
	id_table:	rtl8192cd_pci_tbl,
	probe:		rtl8192cd_init_pci,
	remove:		__devexit_p(rtk_remove_one),
};
#endif // CONFIG_NET_PCI


#ifdef CONFIG_WIRELESS_LAN_MODULE
int GetCpuCanSuspend(void)
{
	extern int gCpuCanSuspend;
	return gCpuCanSuspend;
}
#endif


#if defined(CONFIG_RTL8196B)
//System identification for CHIP
#define CHIP_OEM_ID	0xb8000000
#define DDR_SELECT	0xb8000008
#define C_CUT		2
#define DDR_BOOT	2
int no_ddr_patch;
#endif

#if defined(CONFIG_RTL_8196D) || defined(CONFIG_RTL_8196E)
unsigned int get_8192cd_gpio0_7(void)
{
	int i;
	struct rtl8192cd_priv *priv;
	struct net_device *dev;
	unsigned int reg = 0; 
	dev = NULL;
	for (i=0; (i<sizeof(wlan_device)/sizeof(struct _device_info_)) && (dev==NULL); i++) 
	{
		if (wlan_device[i].priv && netif_running(wlan_device[i].priv->dev)) 
		{
			priv = wlan_device[i].priv;
			reg = RTL_R32(GPIO_PIN_CTRL);
			printk("<%s>LZQ: read 8192cd gpio reg [0x%x]!\n", __FUNCTION__, reg);			
			return reg;
		}
	}	
	printk("<%s>LZQ: read 8192cd gpio reg ERROR!\n", __FUNCTION__);
	return 0;
}
#endif

#ifndef __ECOS
int MDL_INIT __rtl8192cd_init(unsigned long base_addr)
{
#ifdef CONFIG_NET_PCI
	int pci_reg=0;
#endif
	int rc;
#ifdef MBSSID
	int i;
#endif

#if defined(CONFIG_RTL8196B)
	//System identification for CHIP
	no_ddr_patch = !((REG32(CHIP_OEM_ID)<C_CUT)&(REG32(DDR_SELECT)&&DDR_BOOT));
#endif

#ifdef __KERNEL__
#if defined(EAP_BY_QUEUE) && defined(USE_CHAR_DEV)
// for module, 2005-12-26 -----------
	extern struct rtl8192cd_priv* (*rtl8192cd_chr_reg_hook)(unsigned int minor, struct rtl8192cd_chr_priv *priv);
	extern void (*rtl8192cd_chr_unreg_hook)(unsigned int minor);
//------------------------------------
#endif
#if defined(CONFIG_WIRELESS_LAN_MODULE) && !defined(NOT_RTK_BSP)
	wirelessnet_hook = GetCpuCanSuspend;
#ifdef BR_SHORTCUT
	wirelessnet_hook_shortcut = get_shortcut_dev;
#endif
#ifdef PERF_DUMP
	Fn_rtl8651_romeperfEnterPoint = rtl8651_romeperfEnterPoint;
	Fn_rtl8651_romeperfExitPoint = rtl8651_romeperfExitPoint;
#endif
#ifdef CONFIG_RTL8190_PRIV_SKB
	wirelessnet_hook_is_priv_buf = is_rtl8190_priv_buf;
	wirelessnet_hook_free_priv_buf = free_rtl8190_priv_buf;
#endif
#endif // CONFIG_WIRELESS_LAN_MODULE
#endif // __KERNEL__

#ifndef GREEN_HILL
#ifdef CONFIG_RTL8671
	printk("%s driver version %d.%d.%d (%s)\n", DRV_NAME, DRV_VERSION_H, DRV_VERSION_L, DRV_VERSION_SUBL, DRV_RELDATE);
#else
	panic_printk("%s driver version %d.%d (%s)\n", DRV_NAME, DRV_VERSION_H, DRV_VERSION_L, DRV_RELDATE);
#endif
#endif

#if defined(__KERNEL__) && !defined(CONFIG_WIRELESS_LAN_MODULE)
	for (wlan_index=0; wlan_index<sizeof(wlan_device)/sizeof(struct _device_info_); wlan_index++)
#else
	if (wlan_index<sizeof(wlan_device)/sizeof(struct _device_info_))
#endif
	{
	printk(" wlan_index:%d  %d, %d,0x%x,0x%x,%d\n", wlan_index, (((wlan_device[wlan_index].type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS),
			wlan_device[wlan_index].type, wlan_device[wlan_index].base_addr,wlan_device[wlan_index].conf_addr, wlan_device[wlan_index].irq);
		if (((wlan_device[wlan_index].type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS) {
#ifdef CONFIG_NET_PCI
			if(!pci_reg){
				pci_reg=1;
#ifdef LINUX_2_6_21_
				rc = pci_register_driver(&rtl8192cd_driver);
				if (rc) {
					printk("pci_register_driver() fail!(%d)\n", rc);
				}else{
					printk("pci_register_driver() OK!(%d)\n", rc);
				}
#else
				pci_module_init(&rtl8192cd_driver);
#endif
			}
#endif
		}
		else {
#ifdef __DRAYTEK_OS__
			wlan_device[wlan_index].base_addr = base_addr;
			wlan_device[wlan_index].type = (TYPE_PCI_DIRECT<<TYPE_SHIFT);
#endif
			rc = rtl8192cd_init_one(NULL, NULL, &wlan_device[wlan_index], -1);

			if (rc)
				printk("init_one fail!!!   rc=%d\n",rc);

#ifdef UNIVERSAL_REPEATER
			if (rc == 0)
				rc = rtl8192cd_init_one(NULL, NULL, &wlan_device[wlan_index], -1);
#endif
#ifdef MBSSID
			if (rc == 0) {
				for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
					rc = rtl8192cd_init_one(NULL, NULL, &wlan_device[wlan_index], i);
					if (rc != 0) {
						printk("Init fail! rc=%d\n", rc);
						break;
					}
				}
			}
#endif
#if defined(CONFIG_RTL8672) && defined(CONFIG_RTL_92C_SUPPORT)
					if (rc == 0) {
						// switch XTAL_BSEL to NAND only for ADSL platform because external 40M crystal only used for wifi chip
						struct rtl8192cd_priv *priv = wlan_device[wlan_index].priv;
		
						if ((GET_CHIP_VER(priv) == VERSION_8188C) || (GET_CHIP_VER(priv) == VERSION_8192C))
						{
							if (RTL_R8(AFE_XTAL_CTRL) & BIT(1)) {	
								unsigned long	flags;
		
								SAVE_INT_AND_CLI(flags);
								
								rtl8192cd_open(priv->dev);
								rtl8192cd_close(priv->dev);
								
								RESTORE_INT(flags);
							}
						}
					}
#endif

		}

#if defined(__DRAYTEK_OS__)
		if (rc != 0)
			return rc;
#endif

#ifndef __KERNEL__
		wlan_index++;
#endif
	}

#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
	rtl_wlan_customPassthru_init();
#endif

#if 0
//#ifdef PCIE_POWER_SAVING	
	HostPCIe_Close();	
#endif


#ifdef CONFIG_RTL_STP
	rtl_pseudo_dev_init(wlan_device[0].priv);
#endif

#ifdef _USE_DRAM_
	{
	extern unsigned char *en_cipherstream;
	extern unsigned char *tx_cipherstream;
	extern char *rc4sbox, *rc4kbox;
	extern unsigned char *pTkip_Sbox_Lower, *pTkip_Sbox_Upper;
	extern unsigned char Tkip_Sbox_Lower[256], Tkip_Sbox_Upper[256];

#ifdef CONFIG_RTL8671
	extern void r3k_enable_DRAM(void);    //6/7/04' hrchen, for 8671 DRAM init
	r3k_enable_DRAM();    //6/7/04' hrchen, for 8671 DRAM init
#endif

	en_cipherstream = (unsigned char *)(DRAM_START_ADDR);
	tx_cipherstream = en_cipherstream;

	rc4sbox = (char *)(DRAM_START_ADDR + 2048);
	rc4kbox = (char *)(DRAM_START_ADDR + 2048 + 256);
	pTkip_Sbox_Lower = (unsigned char *)(DRAM_START_ADDR + 2048 + 256*2);
	pTkip_Sbox_Upper = (unsigned char *)(DRAM_START_ADDR + 2048 + 256*3);

	memcpy(pTkip_Sbox_Lower, Tkip_Sbox_Lower, 256);
	memcpy(pTkip_Sbox_Upper, Tkip_Sbox_Upper, 256);
	}
#endif

#ifdef __KERNEL__
#if defined(EAP_BY_QUEUE) && defined(USE_CHAR_DEV)
// for module, 2005-12-26 -----------
	rtl8192cd_chr_reg_hook = rtl8192cd_chr_reg;
	rtl8192cd_chr_unreg_hook = rtl8192cd_chr_unreg;
//------------------------------------
	rtl8192cd_chr_init();
#endif
#endif

#ifdef CONFIG_RTL8671
	//turn off AP LED
	{
		unsigned char wlanreg = *(volatile unsigned char *)0xbd30005e;
		*(volatile unsigned char *)0xbd30005e = (wlanreg | ((1<<5)));
	}
#endif

#ifdef PERF_DUMP
	rtl8651_romeperfInit();
#endif

#ifdef USB_PKT_RATE_CTRL_SUPPORT
	register_usb_hook = (register_usb_pkt_cnt_fn)(register_usb_pkt_cnt_f);
#endif

	printk("init ok!!!wlan_index:%d\n",wlan_index);
	return 0;
}
#endif


#ifdef __DRAYTEK_OS__
int rtl8192cd_init(unsigned long base_addr)
{
	return __rtl8192cd_init(base_addr);
}
#elif !defined(__ECOS)
int MDL_INIT rtl8192cd_init(void)
{
#if defined(CONFIG_RTL_ULINKER_WLAN_DELAY_INIT)
	static char initated = 0;
	if (initated == 0)
		initated = 1;
	else
		return 0;
#endif

#ifdef CONFIG_RTL8671
	gpioConfig(10,2);
	gpioClear(10);
	delay_ms(10);
	gpioSet(10);
#endif

	return __rtl8192cd_init(0);
}
#endif


#ifndef __ECOS
#ifdef __KERNEL__
#if !defined(CONFIG_RTL_ULINKER_WLAN_DELAY_INIT)
static
#endif
void MDL_EXIT rtl8192cd_exit (void)
{
	struct net_device *dev;
	struct rtl8192cd_priv *priv;
	int idx;
#if defined(WDS) || defined(MBSSID)
	int i;
#endif

#if defined(CONFIG_WIRELESS_LAN_MODULE) && !defined(NOT_RTK_BSP)
	wirelessnet_hook = NULL;
#ifdef BR_SHORTCUT
	wirelessnet_hook_shortcut = NULL;
#endif
#ifdef PERF_DUMP
	Fn_rtl8651_romeperfEnterPoint = NULL;
	Fn_rtl8651_romeperfExitPoint = NULL;
 #endif
#ifdef CONFIG_RTL8190_PRIV_SKB
	wirelessnet_hook_is_priv_buf = NULL;
	wirelessnet_hook_free_priv_buf = NULL;
#endif
#endif // CONFIG_WIRELESS_LAN_MODULE

#if defined(EAP_BY_QUEUE) && defined(USE_CHAR_DEV)
// for module, 2005-12-26 ------------
	extern struct rtl8192cd_priv* (*rtl8192cd_chr_reg_hook)(unsigned int minor, struct rtl8192cd_chr_priv *priv);
	extern void (*rtl8192cd_chr_unreg_hook)(unsigned int minor);
//------------------------------------
#endif

#ifdef WDS
	int num;

	for (idx=0; idx<sizeof(wlan_device)/sizeof(struct _device_info_); idx++) {
		if (wlan_device[idx].priv) {
			num = (wlan_device[idx].type >> WDS_SHIFT) & WDS_MASK;
			for (i=0; i<num; i++) {
				unregister_netdev(wlan_device[idx].priv->wds_dev[i]);
#ifndef NETDEV_NO_PRIV
				wlan_device[idx].priv->wds_dev[i]->priv = NULL;
#endif
#ifdef __LINUX_2_6__
				free_netdev(wlan_device[idx].priv->wds_dev[i]);
#else
				kfree(wlan_device[idx].priv->wds_dev[i]);
#endif
			}
		}
	}
#endif

#ifdef CONFIG_RTK_MESH
#ifndef WDS
	int num;
#endif

	for (idx=0; idx<sizeof(wlan_device)/sizeof(struct _device_info_); idx++) {
		num = (wlan_device[idx].type >> MESH_SHIFT) & MESH_MASK;
		if(num > 0) { // num is always 0 or 1 in this time
		// for (i=0; i<num; i++) {
			if (wlan_device[idx].priv) {
				wlan_device[idx].priv->mesh_dev->priv = NULL;
				unregister_netdev(wlan_device[idx].priv->mesh_dev);
				kfree(wlan_device[idx].priv->mesh_dev);
			}
		} // end of if(num > 0)
	}

#endif // CONFIG_RTK_MESH


#ifdef UNIVERSAL_REPEATER
	for (idx=0; idx<sizeof(wlan_device)/sizeof(struct _device_info_); idx++) {
		if (wlan_device[idx].priv) {
			struct rtl8192cd_priv *vxd_priv = GET_VXD_PRIV(wlan_device[idx].priv);
			if (vxd_priv) {
				unregister_netdev(vxd_priv->dev);
#ifdef RTL8192CD_VARIABLE_USED_DMEM
				rtl8192cd_dmem_free(PMIB, vxd_priv->pmib);
#else
				kfree(vxd_priv->pmib);
#endif
				kfree(vxd_priv->pevent_queue);
#ifdef CONFIG_RTL_WAPI_SUPPORT
				kfree(vxd_priv->wapiEvent_queue);
#endif
				kfree(vxd_priv->pwlan_acl_poll);
				kfree(vxd_priv->Eap_packet);
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
				kfree(vxd_priv->wpa_global_info);
#endif
#ifdef __LINUX_2_6__
				free_netdev(vxd_priv->dev);
#else
				kfree(vxd_priv->dev);
#endif
				wlan_device[idx].priv->pvxd_priv = NULL;
			}
		}
	}
#endif

#ifdef MBSSID
	for (idx=0; idx<sizeof(wlan_device)/sizeof(struct _device_info_); idx++) {
		if (wlan_device[idx].priv) {
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
				struct rtl8192cd_priv *vap_priv = wlan_device[idx].priv->pvap_priv[i];
				if (vap_priv) {
					unregister_netdev(vap_priv->dev);
					rtl8192cd_proc_remove(vap_priv->dev);
#ifdef RTL8192CD_VARIABLE_USED_DMEM
					rtl8192cd_dmem_free(PMIB, vap_priv->pmib);
#else
					kfree(vap_priv->pmib);
#endif
					kfree(vap_priv->pevent_queue);
#ifdef CONFIG_RTL_WAPI_SUPPORT
					kfree(vap_priv->wapiEvent_queue);
#endif
					kfree(vap_priv->pwlan_acl_poll);
					kfree(vap_priv->Eap_packet);
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
					kfree(vap_priv->wpa_global_info);
#endif
#ifdef __LINUX_2_6__
					free_netdev(vap_priv->dev);
#else
					kfree(vap_priv->dev);
#endif
					wlan_device[idx].priv->pvap_priv[i] = NULL;
				}
			}
		}
	}
#endif

#ifdef CONFIG_NET_PCI
	pci_unregister_driver (&rtl8192cd_driver);
#endif

	for (idx=0; idx<sizeof(wlan_device)/sizeof(struct _device_info_); idx++)
	{
		if (wlan_device[idx].priv == NULL)
			continue;
		priv = wlan_device[idx].priv;
		dev = priv->dev;

#ifdef _INCLUDE_PROC_FS_
		rtl8192cd_proc_remove(dev);
#endif

		unregister_netdev(dev);

#ifndef PRIV_STA_BUF
#ifdef CONFIG_NET_PCI
		if (IS_PCIBIOS_TYPE)
		{
			unsigned long page_align_phy = (PAGE_SIZE - (((unsigned long)priv->pshare->phw->alloc_dma_buf) & (PAGE_SIZE-1)));
			pci_free_consistent(priv->pshare->pdev, DESC_DMA_PAGE_SIZE, (void*)priv->pshare->phw->alloc_dma_buf,
				(dma_addr_t)(priv->pshare->phw->ring_dma_addr-page_align_phy));
		}
		else
#endif
			kfree((void *)priv->pshare->phw->alloc_dma_buf);
#endif

#ifdef RTL8192CD_VARIABLE_USED_DMEM
		rtl8192cd_dmem_free(PMIB, priv->pmib);
#else
		kfree(priv->pmib);
#endif
		kfree(priv->pevent_queue);
#ifdef CONFIG_RTL_WAPI_SUPPORT
		kfree(priv->wapiEvent_queue);
		#ifdef MBSSID
		if (IS_ROOT_INTERFACE(priv)&&priv->pmib->miscEntry.vap_enable){
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++)
				kfree(priv->pvap_priv[i]->wapiEvent_queue);
		}
		#endif

#endif

#ifdef P2P_SUPPORT
	if(priv->p2pPtr)
		kfree(priv->p2pPtr);
#endif

#ifdef CONFIG_RTK_MESH
		kfree(priv->pathsel_queue);
#ifdef _11s_TEST_MODE_
		kfree(priv->receiver_queue);

		for(i=0; i< AODV_RREQ_TABLE_SIZE; i++)
			del_timer(&priv->pshare->galileo_poll->node[i].data.expire_timer);

		kfree(priv->pshare->galileo_poll);
#endif
#ifdef	_MESH_ACL_ENABLE_
		kfree(priv->pmesh_acl_poll);
#endif
#endif

#ifndef PRIV_STA_BUF
		kfree(priv->pshare->phw);
		kfree(priv->pshare->pwlan_hdr_poll);
		kfree(priv->pshare->pwlanllc_hdr_poll);
		kfree(priv->pshare->pwlanbuf_poll);
		kfree(priv->pshare->pwlanicv_poll);
		kfree(priv->pshare->pwlanmic_poll);
#endif
		kfree(priv->pwlan_acl_poll);
		kfree(priv->Eap_packet);
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
		kfree(priv->wpa_global_info);
#endif
#ifndef PRIV_STA_BUF
		kfree(priv->pshare);	// david
#endif
#ifdef __LINUX_2_6__
		free_netdev(dev);
#else
		kfree(dev);
#endif
		wlan_device[idx].priv = NULL;
	}

#if defined(EAP_BY_QUEUE) && defined(USE_CHAR_DEV)
// for module, 2005-12-26 ------------
	rtl8192cd_chr_reg_hook = NULL;
	rtl8192cd_chr_unreg_hook = NULL;
//------------------------------------

	rtl8192cd_chr_exit();
#endif
}
#else // not __KERNEL__
void MDL_EXIT rtl8192cd_exit(void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = dev->priv;
	int idx, i;

	for (idx=0; idx<sizeof(wlan_device)/sizeof(struct _device_info_); idx++)
		if (wlan_device[idx].priv == priv)
			break;

	if (idx == sizeof(wlan_device)/sizeof(struct _device_info_))
		return;		// wrong argument!!

#ifdef WDS
	{
		int num;

		num = (wlan_device[idx].type >> WDS_SHIFT) & WDS_MASK;
		for (i=0; i<num; i++) {
			wlan_device[idx].priv->pmib->dot11WdsInfo.dev[i]->priv = NULL;
			unregister_netdev(wlan_device[idx].priv->pmib->dot11WdsInfo.dev[i]);
			kfree(wlan_device[idx].priv->pmib->dot11WdsInfo.dev[i]);
		}
	}
#endif

	unregister_netdev(dev);

	kfree(priv->pmib);

#ifdef	CONFIG_RTK_MESH

	if(priv->proxy_table)
	{
		remove_hash_table(priv->proxy_table);
		kfree(priv->proxy_table);
	}
	if(priv->mesh_rreq_retry_queue)
	{
		for (i = 0; i < (1 << priv->mesh_rreq_retry_queue->table_size_power); i++) {
			if(((struct mesh_rreq_retry_entry*)(priv->mesh_rreq_retry_queue->entry_array[i].data))->ptr)
				kfree(((struct mesh_rreq_retry_entry*)(priv->mesh_rreq_retry_queue->entry_array[i].data))->ptr);
		}
		remove_hash_table(priv->mesh_rreq_retry_queue);
		kfree(priv->mesh_rreq_retry_queue);
	}

	// add by chuangch 2007.09.13
	if(priv->pathsel_table)
	{
		remove_hash_table(priv->pathsel_table);
		kfree(priv->pathsel_table);
	}

	if(priv->pann_mpp_tb)
		kfree(priv->pann_mpp_tb);

	kfree(priv->pathsel_queue);
#ifdef _11s_TEST_MODE_
	kfree(priv->receiver_queue);
	for(i=0; i< AODV_RREQ_TABLE_SIZE; i++)
		del_timer(&priv->pshare->galileo_poll->node[i].data.expire_timer);

	kfree(priv->pshare->galileo_poll);
#endif

#ifdef	_MESH_ACL_ENABLE_
	kfree(priv->pmesh_acl_poll);
#endif
#endif	// CONFIG_RTK_MESH

	kfree(priv->pevent_queue);
#ifdef CONFIG_RTL_WAPI_SUPPORT
//	kfree(vxd_priv->wapiEvent_queue);
	kfree(priv->wapiEvent_queue);
	#ifdef MBSSID
	if (IS_ROOT_INTERFACE(priv)&&priv->pmib->miscEntry.vap_enable)	{
		for (i=0; i<RTL8192CD_NUM_VWLAN; i++)
			kfree(priv->pvap_priv[i]->wapiEvent_queue);
	}
	#endif

#endif
#ifndef PRIV_STA_BUF
	kfree((void *)priv->pshare->phw->alloc_dma_buf);

	kfree(priv->pshare->phw);
	kfree(priv->pshare->pwlan_hdr_poll);
	kfree(priv->pshare->pwlanllc_hdr_poll);
	kfree(priv->pshare->pwlanbuf_poll);
	kfree(priv->pshare->pwlanicv_poll);
	kfree(priv->pshare->pwlanmic_poll);
#endif
	kfree(priv->pwlan_acl_poll);
	kfree(priv->Eap_packet);
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
	kfree(priv->wpa_global_info);
#endif
#ifndef PRIV_STA_BUF
	kfree(priv->pshare);	// david
#endif
	kfree(dev);
	wlan_device[idx].priv = NULL;

	wlan_index--;
}
#endif
#endif // !__EOCS


#ifdef __KERNEL__
#ifdef USE_CHAR_DEV
struct rtl8192cd_priv *rtl8192cd_chr_reg(unsigned int minor, struct rtl8192cd_chr_priv *priv)
{
	if (wlan_device[minor].priv)
		wlan_device[minor].priv->pshare->chr_priv = priv;
	return wlan_device[minor].priv;
}


void rtl8192cd_chr_unreg(unsigned int minor)
{
	if (wlan_device[minor].priv)
		wlan_device[minor].priv->pshare->chr_priv = NULL;
}
#endif


#ifdef RTL_WPA2_PREAUTH
void wpa2_kill_fasync(void)
{
	int wlan_index = 0;
	struct _device_info_ *wdev = &wlan_device[wlan_index];
	struct rtl8192cd_priv *priv = wdev->priv;
	event_indicate(priv, NULL, -1);
}


void wpa2_preauth_packet(struct sk_buff	*pskb)
{
	// ****** NOTICE **********
	int wlan_index = 0;
	struct _device_info_ *wdev = &wlan_device[wlan_index];
	// ****** NOTICE **********

	struct rtl8192cd_priv *priv = wdev->priv;

	unsigned char		szEAPOL[] = {0x02, 0x01, 0x00, 0x00};
	DOT11_EAPOL_START	Eapol_Start;

	if (priv == NULL) {
		PRINT_INFO("%s: priv == NULL\n", (char *)__FUNCTION__);
		return;
	}

#ifndef WITHOUT_ENQUEUE
	if (!memcmp(pskb->data, szEAPOL, sizeof(szEAPOL)))
	{
		Eapol_Start.EventId = DOT11_EVENT_EAPOLSTART_PREAUTH;
		Eapol_Start.IsMoreEvent = FALSE;
#ifdef LINUX_2_6_22_
		memcpy(&Eapol_Start.MACAddr, pskb->mac_header + MACADDRLEN, WLAN_ETHHDR_LEN);
#else
		memcpy(&Eapol_Start.MACAddr, pskb->mac.raw + MACADDRLEN, WLAN_ETHHDR_LEN);
#endif
		DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (unsigned char*)&Eapol_Start, sizeof(DOT11_EAPOL_START));
	}
	else
	{
		unsigned short		pkt_len;

		pkt_len = WLAN_ETHHDR_LEN + pskb->len;
		priv->Eap_packet->EventId = DOT11_EVENT_EAP_PACKET_PREAUTH;
		priv->Eap_packet->IsMoreEvent = FALSE;
		memcpy(&(priv->Eap_packet->packet_len), &pkt_len, sizeof(unsigned short));
#ifdef LINUX_2_6_22_
		memcpy(&(priv->Eap_packet->packet[0]), pskb->mac_header, WLAN_ETHHDR_LEN);
#else
		memcpy(&(priv->Eap_packet->packet[0]), pskb->mac.raw, WLAN_ETHHDR_LEN);
#endif
		memcpy(&(priv->Eap_packet->packet[WLAN_ETHHDR_LEN]), pskb->data, pskb->len);
		DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (unsigned char*)priv->Eap_packet, sizeof(DOT11_EAP_PACKET));
	}
#endif // WITHOUT_ENQUEUE

	event_indicate(priv, NULL, -1);

	// let dsr to free this skb
}
#endif // RTL_WPA2_PREAUTH
#endif // __KERNEL__

#if defined(CONFIG_RTL_ETH_PRIV_SKB_DEBUG)
__MIPS16 __IRAM_FWD  extern int is_rtl865x_eth_priv_buf(unsigned char *head);
extern void dump_sta_dz_queue_num(struct rtl8192cd_priv *priv, struct stat_info *pstat);

int dump_wlan_dz_queue_num(const char *name)
{
	int i,j,txCnt=0;
	//int queueCnt,idx;
	struct rtl8192cd_priv *priv;
	//struct tx_desc_info *pdescinfoH,*pdescinfo;
	//struct tx_desc	*pdescH, *pdesc;
	//struct sk_buff *skb = NULL;
	//struct rtl8192cd_hw	*phw;
	int 			hd, tl;
					
	for (j=0; (j<sizeof(wlan_device)/sizeof(struct _device_info_)); j++)
	{
		//if(counted)
			//break;
		
		if (wlan_device[j].priv && netif_running(wlan_device[j].priv->dev) && strcmp(wlan_device[j].priv->dev->name,name)==0)
		{
			priv = wlan_device[j].priv;
			if (OPMODE & WIFI_AP_STATE) 
			{
				hd = priv->dz_queue.head;
				tl = priv->dz_queue.tail;
				printk("priv->dz_queue:%d\n",CIRC_CNT(hd, tl, NUM_TXPKT_QUEUE));

				for (i=0; i<NUM_STAT; i++)
				{
					if (priv->pshare->aidarray[i]) {
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
						if (priv != priv->pshare->aidarray[i]->priv)
							continue;
						else
#endif
						{
							if (priv->pshare->aidarray[i]->used == TRUE)
							{
								dump_sta_dz_queue_num(priv, &(priv->pshare->aidarray[i]->station));
							}

						}
					}
				}
			}
		}
	}

	return txCnt;
}
int get_nic_buf_in_wireless_tx(const char *name)
{
	int i,txCnt;
	int queueCnt,idx;
	struct rtl8192cd_priv *priv;
	struct tx_desc_info *pdescinfoH,*pdescinfo;
	struct tx_desc	*pdescH, *pdesc;
	struct sk_buff *skb = NULL;
	struct rtl8192cd_hw	*phw;

	txCnt = 0;
	for (i=0; (i<sizeof(wlan_device)/sizeof(struct _device_info_)); i++)
	{
		//if(counted)
			//break;
		
		if (wlan_device[i].priv && netif_running(wlan_device[i].priv->dev) && strcmp(wlan_device[i].priv->dev->name,name)==0)
		{
			priv = wlan_device[i].priv;
			if (OPMODE & WIFI_AP_STATE) 
			{
				for(queueCnt = 0; queueCnt <= HIGH_QUEUE;queueCnt++)
				{
					phw=GET_HW(priv);
					pdescH		= get_txdesc(phw, queueCnt);					
					pdescinfoH = get_txdesc_info(priv->pshare->pdesc_info,queueCnt);
					for(idx = 0; idx < CURRENT_NUM_TX_DESC; idx++)
					{
						pdesc = pdescH + idx;
						pdescinfo = pdescinfoH + idx;
						//if (!pdesc || (get_desc(pdesc->Dword0) & TX_OWN))
							//continue;
						if(pdescinfo->type == _SKB_FRAME_TYPE_ || pdescinfo->type == _RESERVED_FRAME_TYPE_)
							skb = (struct sk_buff *)(pdescinfo->pframe);
						else
							continue;
						
						if(skb && is_rtl865x_eth_priv_buf(skb->head))
							txCnt++;
					}
				}

				//counted = 1;
			}
		}
	}

	return txCnt;
}
#endif


#if defined(CONFIG_RTK_VLAN_SUPPORT) && defined(CONFIG_RTK_VLAN_FOR_CABLE_MODEM)
struct net_device* get_dev_by_vid(int vid)
{
	int i;
	struct rtl8192cd_priv *priv;

	for (i=0; (i<sizeof(wlan_device)/sizeof(struct _device_info_)); i++)
	{
		if (wlan_device[i].priv && netif_running(wlan_device[i].priv->dev))
		{
			priv = wlan_device[i].priv;
			if ((OPMODE & WIFI_AP_STATE) && priv->pmib->miscEntry.vap_enable) {
					int j;
					for (j=0; j<RTL8192CD_NUM_VWLAN; j++) {
						if (IS_DRV_OPEN(priv->pvap_priv[j])) {
							if(priv->pvap_priv[j]->pmib->vlan.vlan_enable && priv->pvap_priv[j]->pmib->vlan.vlan_id == vid)
								return priv->pvap_priv[j]->dev;
						}
					}
				}
		}
	}

	return NULL;
}
#endif


__MIPS16
__IRAM_IN_865X
struct net_device *get_shortcut_dev(unsigned char *da)
{
	int i;
#ifdef MBSSID
	int j;
#endif
	struct rtl8192cd_priv *priv;
	struct rtl8192cd_priv *vxd_priv;
	struct stat_info *pstat;
	struct net_device *dev;

#if defined(BR_SHORTCUT)
	#ifdef CONFIG_RTK_MESH	//11 mesh no support shortcut now
	{
		extern unsigned char cached_mesh_mac[MACADDRLEN];
		extern struct net_device *cached_mesh_dev;
		if (cached_mesh_dev && !memcmp(da, cached_mesh_mac, MACADDRLEN))
			return cached_mesh_dev;
	}
	#endif

	#ifdef WDS
	{
		extern unsigned char cached_wds_mac[MACADDRLEN];
		extern struct net_device *cached_wds_dev;
		if (cached_wds_dev && !memcmp(da, cached_wds_mac, MACADDRLEN))
			return cached_wds_dev;
	}
	#endif

	#ifdef CLIENT_MODE
	{
		extern unsigned char cached_sta_mac[MACADDRLEN];
		extern struct net_device *cached_sta_dev;
		if (cached_sta_dev && !memcmp(da, cached_sta_mac, MACADDRLEN))
			return cached_sta_dev;
	}
	#endif

	#if defined(RTL_CACHED_BR_STA)
	{
		extern unsigned char cached_br_sta_mac[MACADDRLEN];
		extern struct net_device *cached_br_sta_dev;
		if ((cached_br_sta_dev!=NULL)&&!memcmp(da, cached_br_sta_mac, MACADDRLEN))
			return cached_br_sta_dev;
	}
	#endif
#endif

	dev = NULL;
	for (i=0; (i<sizeof(wlan_device)/sizeof(struct _device_info_)) && (dev==NULL); i++) {
		if (wlan_device[i].priv && netif_running(wlan_device[i].priv->dev)) {
			priv = wlan_device[i].priv;

			if (priv
				//2010-5-10
#ifdef __KERNEL__
				#if !defined( _SINUX_ ) && LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,35)
				// if sinux, no linux bridge, so should don't depend on br_port if use br_shortcut (John Qian 2010/6/24) 
				&& (priv->dev->br_port)
				&& !(priv->dev->br_port->br->stp_enabled)
				#endif
#endif				
			) {
				if (!priv->pmib->dot11OperationEntry.disable_brsc) {
					pstat = get_stainfo(priv, da);
					if (pstat && pstat->tx_pkts > 1) {	/* Make sure it must have some packets go theough bridge module before shortcut */
						#ifdef WDS
						if (!(pstat->state & WIFI_WDS))	// if WDS peer
						#endif
						{
							#ifdef CONFIG_RTK_MESH
							if( isMeshPoint(pstat))
								{dev = priv->mesh_dev;}
							else
							#endif
								{dev = priv->dev;}
							break;
						}
					}
				}
				#ifdef MBSSID
			      if ((OPMODE & WIFI_AP_STATE) && priv->pmib->miscEntry.vap_enable) {
					for (j=0; j<RTL8192CD_NUM_VWLAN; j++) {
						if ((priv->pvap_priv[j]->assoc_num > 0) && IS_DRV_OPEN(priv->pvap_priv[j]) &&
							!(priv->pvap_priv[j]->pmib->dot11OperationEntry.disable_brsc)) {
							pstat = get_stainfo(priv->pvap_priv[j], da);
							if (pstat && pstat->tx_pkts > 1) {
								dev = priv->pvap_priv[j]->dev;
								break;
							}
						}
					}
				}
				#endif
				#ifdef UNIVERSAL_REPEATER
				vxd_priv = GET_VXD_PRIV(priv);
				if((OPMODE & WIFI_STATION_STATE) && (vxd_priv->assoc_num > 0) && IS_DRV_OPEN(vxd_priv) &&
					!vxd_priv->pmib->dot11OperationEntry.disable_brsc) {		
					pstat = get_stainfo(vxd_priv, da);
					if (pstat && pstat->tx_pkts > 1) {
						dev = vxd_priv->dev;
						break;
					}	
				}
				#endif
			}
		}
	}

#if	defined(RTL_CACHED_BR_STA) && defined(BR_SHORTCUT)
	if (dev!=NULL) {
	extern unsigned char cached_br_sta_mac[MACADDRLEN];
	extern struct net_device *cached_br_sta_dev;		
	memcpy(cached_br_sta_mac, da, MACADDRLEN);
	cached_br_sta_dev = dev;
	}
#endif

	return dev;
}

#if defined(BR_SHORTCUT)
void clear_shortcut_cache(void)
{
#ifdef CONFIG_RTK_MESH
	{
		extern struct net_device *cached_mesh_dev;
		extern unsigned char cached_mesh_mac[MACADDRLEN];
		cached_mesh_dev	= NULL;
		memset(cached_mesh_mac,0,MACADDRLEN);
	}
#endif

#ifdef WDS
	{
		extern struct net_device *cached_wds_dev;
		extern unsigned char cached_wds_mac[MACADDRLEN];
		cached_wds_dev= NULL;
		memset(cached_wds_mac,0,MACADDRLEN);
	}
#endif

#ifdef CLIENT_MODE
	{
		extern struct net_device *cached_sta_dev;
		extern unsigned char cached_sta_mac[MACADDRLEN];
		cached_sta_dev = NULL;
		memset(cached_sta_mac,0,MACADDRLEN);
	}
#endif

#if	defined(RTL_CACHED_BR_STA)
	{
		extern struct net_device *cached_br_sta_dev;
		extern unsigned char cached_br_sta_mac[MACADDRLEN];
		cached_br_sta_dev = NULL;
		memset(cached_br_sta_mac,0,MACADDRLEN);
	}
#endif

#if defined(CONFIG_RTL_819X) && (defined(__LINUX_2_6__) || defined(__ECOS))
	{
		extern 	 unsigned char cached_eth_addr[MACADDRLEN];
		extern struct net_device *cached_dev;
		cached_dev = NULL;
		memset(cached_eth_addr,0,MACADDRLEN);
	}
#endif

#ifdef BR_SHORTCUT_C2
	{
		extern 	 unsigned char cached_eth_addr2[MACADDRLEN];
		extern struct net_device *cached_dev2;
		cached_dev2 = NULL;
		memset(cached_eth_addr2,0,MACADDRLEN);
	}
#endif

#ifdef CONFIG_RTL8672
	{
		extern void clear_cached_eth_mac_addr(void);
		clear_cached_eth_mac_addr();
	}
#endif
}
#endif // BR_SHORTCUT


void update_fwtbl_asoclst(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	unsigned char tmpbuf[16];
	int i;

#if defined(__KERNEL__) && !defined(NOT_RTK_BSP) 
	struct sk_buff *skb = NULL;
	struct wlan_ethhdr_t *e_hdr;
	unsigned char xid_cmd[] = {0, 0, 0xaf, 0x81, 1, 2};

	// update forwarding table of bridge module
	if (priv->dev->br_port) {
		skb = dev_alloc_skb(64);
		if (skb != NULL) {
			skb->dev = priv->dev;
			skb_put(skb, 60);
			e_hdr = (struct wlan_ethhdr_t *)skb->data;
			memset(e_hdr, 0, 64);
			memcpy(e_hdr->daddr, priv->dev->dev_addr, MACADDRLEN);
			memcpy(e_hdr->saddr, pstat->hwaddr, MACADDRLEN);
			e_hdr->type = 8;
			memcpy(&skb->data[14], xid_cmd, sizeof(xid_cmd));
			skb->protocol = eth_type_trans(skb, priv->dev);
			#if defined(__LINUX_2_6__) && defined(RX_TASKLET)&& !defined(CONFIG_RTL8672)
				netif_receive_skb(skb);
			#else
				netif_rx(skb);
			#endif
		}
	}
#endif

	// update association lists of the other WLAN interfaces
	for (i=0; i<sizeof(wlan_device)/sizeof(struct _device_info_); i++) {
		if (wlan_device[i].priv && (wlan_device[i].priv != priv)) {
			if (wlan_device[i].priv->pmib->dot11OperationEntry.opmode & WIFI_AP_STATE) {
				sprintf((char *)tmpbuf, "%02x%02x%02x%02x%02x%02x",
					pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5]);
				del_sta(wlan_device[i].priv, tmpbuf);
			}
		}
	}

#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
		for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
			if (priv->pvap_priv[i] && IS_DRV_OPEN(priv->pvap_priv[i]) && (priv->pvap_priv[i] != priv) &&
				(priv->vap_init_seq >= 0)) {
				if (priv->pvap_priv[i]->pmib->dot11OperationEntry.opmode & WIFI_AP_STATE) {
					sprintf(tmpbuf, "%02x%02x%02x%02x%02x%02x",
						pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5]);
					del_sta(priv->pvap_priv[i], tmpbuf);
				}
			}
		}
	}
#endif

#ifdef __ECOS
#ifdef CONFIG_RTL_819X_SWCORE
	{
	/* 02-17-2012: move the called function "update_hw_l2table" from Bridge module to here to avoid hacking the Linux kernel or the other kernel */	
	extern void update_hw_l2table(const char *srcName,const unsigned char *addr);
	update_hw_l2table("wlan", (const unsigned char *)pstat->hwaddr); /* RTL_WLAN_NAME */
	}
#endif
#else
#ifdef CONFIG_RTL_819X
#ifndef CONFIG_RTL_8198B
	{
	/* 02-17-2012: move the called function "update_hw_l2table" from Bridge module to here to avoid hacking the Linux kernel or the other kernel */	
	extern void update_hw_l2table(const char *srcName,const unsigned char *addr);
	update_hw_l2table("wlan", (const unsigned char *)pstat->hwaddr); /* RTL_WLAN_NAME */
	}
#endif
#endif
#endif
}


// quick fix for warn reboot fail issue
#ifndef CONFIG_RTL_8198B
#define CLK_MANAGE     0xb8000010
#endif
void force_stop_wlan_hw(void)
{
	int i=0;
#ifdef CONFIG_WLAN_HAL	
    unsigned int errorFlag;
#endif	
//	int temp;

#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN) || defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN_D) || defined(CONFIG_RTL_92D_DMDP)
	for (i=0; i<sizeof(wlan_device)/sizeof(struct _device_info_); i++) 
#endif
	{
		if (wlan_device[i].priv) {
			struct rtl8192cd_priv *priv = wlan_device[i].priv;
#ifdef PCIE_POWER_SAVING
			if ((REG32(CLK_MANAGE) & BIT(11)) == 0)
			{
				extern void setBaseAddressRegister(void);
				REG32(CLK_MANAGE) |= BIT(11);
				delay_ms(10);
				PCIE_reset_procedure(0, 0, 1, wlan_device[i].base_addr);
				setBaseAddressRegister();
			}
#endif
#ifdef  CONFIG_WLAN_HAL
			if (GET_CHIP_VER(priv)==VERSION_8192E || GET_CHIP_VER(priv)==VERSION_8881A) {
	            BOOLEAN     bVal;

	            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_MAC_IO_ENABLE, &bVal);
	            if ( bVal ) {
                    if (RT_STATUS_SUCCESS == GET_HAL_INTERFACE(priv)->StopHWHandler(priv)) {
                        printk("StopHW Succeed\n");
                    }
                    else {  
                        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);            
                        errorFlag |= DRV_ER_CLOSE_STOP_HW;
                        GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);                                         
                        panic_printk("StopHW Failed\n");
                    }                    
	            }
	            else {
	                printk("(%d), Can't write MACID register\n", __LINE__);
	            }
			} else
#endif
			{
				if ( check_MAC_IO_Enable(priv) ) {
					rtl8192cd_stop_hw(priv);
				}
			}
		}
	}
}

#ifdef _BROADLIGHT_FASTPATH_
void replace_upper_layer_packet_destination( void * xi_destination_ptr )
{
	printk(KERN_INFO"start fastpath\n");
	send_packet_to_upper_layer = xi_destination_ptr ;
}
EXPORT_SYMBOL(replace_upper_layer_packet_destination) ;
#endif

#ifdef __KERNEL__
#if defined(CONFIG_WIRELESS_LAN_MODULE)
MODULE_LICENSE("GPL");
#endif
#if defined(CONFIG_RTL_ULINKER_WLAN_DELAY_INIT)
	/* don't init wlan while kernel startup */
#else
module_init(rtl8192cd_init);
module_exit(rtl8192cd_exit);
#endif /* #if defined(CONFIG_RTL_ULINKER_WLAN_DELAY_INIT) */
#endif

