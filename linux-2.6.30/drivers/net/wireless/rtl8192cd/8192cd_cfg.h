/*
 *	 Headler file defines some configure options and basic types
 *
 *	 $Id: 8192cd_cfg.h,v 1.59.2.26 2011/01/10 07:49:07 jerryko Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef _8192CD_CFG_H_
#define _8192CD_CFG_H_

#if defined(CONFIG_RTL_ULINKER_BRSC)
#include "linux/ulinker_brsc.h"
#endif

//#define _LITTLE_ENDIAN_
//#define _BIG_ENDIAN_

#ifdef __MIPSEB__

#ifndef _BIG_ENDIAN_
	#define _BIG_ENDIAN_
#endif

#ifdef _LITTLE_ENDIAN_
#undef _LITTLE_ENDIAN_
#endif
//### add by sen_liu 2011.4.14 CONFIG_NET_PCI defined in V2.4 and CONFIG_PCI
// define now to replace it. However,some modules still use CONFIG_NET_PCI
#ifdef CONFIG_PCI
#define CONFIG_NET_PCI
#endif

//### end
#endif	//__MIPSEB__

#ifdef __KERNEL__
#include <linux/version.h>

#if LINUX_VERSION_CODE >= 0x020615 // linux 2.6.21
	#define LINUX_2_6_21_
#endif

#if LINUX_VERSION_CODE >= 0x020616 // linux 2.6.22
	#define LINUX_2_6_22_
#endif

#if LINUX_VERSION_CODE >= 0x020618 // linux 2.6.24
	#define LINUX_2_6_24_
#endif

#if LINUX_VERSION_CODE >= 0x02061B // linux 2.6.27
	#define LINUX_2_6_27_
#endif

#if LINUX_VERSION_CODE > 0x020600
	#define __LINUX_2_6__
#endif

#ifdef LINUX_2_6_22_
#include <linux/autoconf.h>
#include <linux/jiffies.h>
#include <asm/param.h>
#else
#include <linux/config.h>
#endif
#endif // __KERNEL__

#include "typedef.h"

#ifdef __ECOS
	#include <pkgconf/system.h>
	#include <pkgconf/devs_eth_rltk_819x_wrapper.h>
	#include <pkgconf/devs_eth_rltk_819x_wlan.h>
	
#ifndef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
	#define CONFIG_RTL8190_PRIV_SKB
#endif
#endif


/*
 *	Following Definition Sync 2.4/2.6 SDK Definitions
 */

#if !defined(__LINUX_2_6__) && !defined(__ECOS)

#if defined(CONFIG_RTL8196B) || defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198)
#define CONFIG_RTL_819X
#endif
#if defined(CONFIG_RTL8198)
#define CONFIG_RTL_8198
#endif
#if defined(CONFIG_RTL8196C)
#define CONFIG_RTL_8196C
#endif

#define BSP_PCIE0_D_CFG0 	PCIE0_D_CFG0
#define BSP_PCIE1_D_CFG0 	PCIE0_D_CFG0
#define BSP_PCIE0_H_CFG 	PCIE0_H_CFG
#define BSP_PCIE0_H_EXT 	PCIE0_RC_EXT_BASE
#define BSP_PCIE0_H_MDIO	(BSP_PCIE0_H_EXT + 0x00)
#define BSP_PCIE0_H_INTSTR	(BSP_PCIE0_H_EXT + 0x04)
#define BSP_PCIE0_H_PWRCR	(BSP_PCIE0_H_EXT + 0x08)
#define BSP_PCIE0_H_IPCFG	(BSP_PCIE0_H_EXT + 0x0C)
#define BSP_PCIE0_H_MISC	(BSP_PCIE0_H_EXT + 0x10)
#define BSP_PCIE1_H_CFG 	PCIE1_H_CFG
#define BSP_PCIE1_H_EXT 	PCIE1_RC_EXT_BASE
#define BSP_PCIE1_H_MDIO	(BSP_PCIE1_H_EXT + 0x00)
#define BSP_PCIE1_H_INTSTR	(BSP_PCIE1_H_EXT + 0x04)
#define BSP_PCIE1_H_PWRCR	(BSP_PCIE1_H_EXT + 0x08)
#define BSP_PCIE1_H_IPCFG	(BSP_PCIE1_H_EXT + 0x0C)
#define BSP_PCIE1_H_MISC	(BSP_PCIE1_H_EXT + 0x10)

#endif // !defined(__LINUX_2_6__)


#if !defined(CONFIG_RTL_8198_AP_ROOT) && !defined(__ECOS)
#define CONFIG_RTL_CUSTOM_PASSTHRU
#endif

#define CONFIG_RTL_CUSTOM_PASSTHRU
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
//#define CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE

#define IP6_PASSTHRU_MASK 0x1
#if	defined(CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE)
#define PPPOE_PASSTHRU_MASK 0x1<<1
#endif

#if defined(CONFIG_RTL_92D_SUPPORT)
#define WISP_WLAN_IDX_MASK 0xF0
#define WISP_WLAN_IDX_RIGHT_SHIFT 4
#endif

#endif	/* CONFIG_RTL_CUSTOM_PASSTHRU	*/



#if (defined(CONFIG_RTL_92C_SUPPORT)&&defined(CONFIG_RTL_92D_SUPPORT))||defined(CONFIG_RTL_92D_DMDP)||defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
#define CONCURRENT_MODE
#ifdef CONFIG_RTL_92D_SMART_CONCURRENT
#define SMART_CONCURRENT_92D
#endif
#if defined(CONFIG_RTL_92D_DMDP) && !defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN) && !defined(CONFIG_RTL_92C_SUPPORT) && !defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN_D)
#define DUALBAND_ONLY
#endif
#endif

#if defined(CONFIG_RTL_92D_SUPPORT) && (defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD)) || defined(CONFIG_RTL_8196E)
/*	Currenlty 98 support 92D should use pcie slot 1	*/
	#if defined(CONFIG_RTK_VOIP_BOARD) || defined(CONFIG_RTL_8196D) || defined(CONFIG_RTL_8196E)
	#define	RTL_USED_PCIE_SLOT	0
	#else
	#define	RTL_USED_PCIE_SLOT	1
	#endif /* CONFIG_RTK_VOIP_BOARD */
#else
#define	RTL_USED_PCIE_SLOT	0
#endif

#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)

	#if defined(CONFIG_RTK_VOIP_BOARD) || defined(CONFIG_RTL_8196D) || defined(CONFIG_RTL_8196E)
	#define	RTL_MAX_PCIE_SLOT_NUM	1
	#else
	#define	RTL_MAX_PCIE_SLOT_NUM	2
	#endif /*CONFIG_RTK_VOIP_BOARD*/

	#if defined(CONFIG_RTL_92D_SUPPORT)

		#if defined(CONFIG_RTK_VOIP_BOARD) || defined(CONFIG_RTL_8196D) || defined(CONFIG_RTL_8196E)
		#define	RTL_USED_PCIE_SLOT	0
		#else
		#define	RTL_USED_PCIE_SLOT	1
		#endif /*ONFIG_RTK_VOIP_BOARD*/

	#else	/*	defined(CONFIG_RTL_92D_SUPPORT)	*/
		#define	RTL_USED_PCIE_SLOT	0
	#endif	/*	defined(CONFIG_RTL_92D_SUPPORT)	*/
#else	/*	defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)	*/
	#define	RTL_MAX_PCIE_SLOT_NUM	1
	#define	RTL_USED_PCIE_SLOT	0
#endif	/*	defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)	*/

#ifdef CONFIG_RTK_MESH
#include "../mesh_ext/mesh_cfg.h"
#endif

#if defined(CONFIG_RTL8196B) || defined(CONFIG_RTL_819X)
	#if defined(CONFIG_RTL8196B_AP_ROOT) || defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8196B_GW) || defined(CONFIG_RTL_8196C_GW) || defined(CONFIG_RTL_8198_GW) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196B_TLD) || defined(CONFIG_RTL8196C_AP_ROOT) || defined(CONFIG_RTL8196C_AP_HCM) || defined(CONFIG_RTL8198_AP_ROOT) || defined(CONFIG_RTL_8198_AP_ROOT) || defined(CONFIG_RTL8196C_CLIENT_ONLY) || defined(CONFIG_RTL_8198_NFBI_BOARD) || defined(CONFIG_RTL8196C_KLD) || defined(CONFIG_RTL8196C_EC) || defined(CONFIG_RTL_8196C_iNIC) || defined(CONFIG_RTL_8198_INBAND_AP) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8198B) || defined(__ECOS)
		#define USE_RTL8186_SDK
	#endif
#endif

#if defined(_BIG_ENDIAN_) || defined(_LITTLE_ENDIAN_)
#elif defined(_BIG_ENDIAN_) && defined(_LITTLE_ENDIAN_)
	#error "One one ENDIAN should be specified\n"
#else
	#error "Please specify your ENDIAN type!\n"
#endif

#define PCI_CONFIG_COMMAND			(wdev->conf_addr+4)
#define PCI_CONFIG_LATENCY			(wdev->conf_addr+0x0c)
#define PCI_CONFIG_BASE0			(wdev->conf_addr+0x10)
#define PCI_CONFIG_BASE1			(wdev->conf_addr+0x18)

#define MAX_NUM(_x_, _y_)	(((_x_)>(_y_))? (_x_) : (_y_))
#define MIN_NUM(_x_, _y_)	(((_x_)<(_y_))? (_x_) : (_y_))

#define POWER_MIN_CHECK(a,b)            (((a) > (b)) ? (b) : (a))
#define POWER_RANGE_CHECK(val)		(((val) > 0x3f)? 0x3f : ((val < 0) ? 0 : val))
#define COUNT_SIGN_OFFSET(val, oft)	(((oft & 0x08) == 0x08)? (val - (0x10 - oft)) : (val + oft))

//-------------------------------------------------------------
// Driver version information
//-------------------------------------------------------------
#define DRV_VERSION_H	1
#define DRV_VERSION_L	6
#define DRV_RELDATE		"2012-12-04"
#define DRV_NAME		"Realtek WLAN driver"


//-------------------------------------------------------------
// Will check type for endian issue when access IO and memory
//-------------------------------------------------------------
#define CHECK_SWAP


//-------------------------------------------------------------
// Defined when include proc file system
//-------------------------------------------------------------
#define INCLUDE_PROC_FS
#if defined(CONFIG_PROC_FS) && defined(INCLUDE_PROC_FS)
	#define _INCLUDE_PROC_FS_
#endif


//-------------------------------------------------------------
// Debug function
//-------------------------------------------------------------
//#define _DEBUG_RTL8192CD_		// defined when debug print is used
#define _IOCTL_DEBUG_CMD_		// defined when read/write register/memory command is used in ioctl


//-------------------------------------------------------------
// Defined when internal DRAM is used for sw encryption/decryption
//-------------------------------------------------------------
#ifdef __MIPSEB__
	// disable internal ram for nat speedup
	//#define _USE_DRAM_
#endif


//-------------------------------------------------------------
// Support 8188C/8192C test chip
//-------------------------------------------------------------
#if !defined(CONFIG_RTL8196B_GW_8M) //&&!defined(CONFIG_RTL_92D_SUPPORT)
#define TESTCHIP_SUPPORT
#endif


//-------------------------------------------------------------
// Support software tx queue
// ------------------------------------------------------------
#define SW_TX_QUEUE

//-------------------------------------------------------------
// Support Tx Report
//-------------------------------------------------------------
#define TXREPORT
#ifdef TXREPORT
#define DETECT_STA_EXISTANCE
#endif
//#define LEAVESTADETECT


//-------------------------------------------------------------
// PCIe power saving function
//-------------------------------------------------------------
#ifdef CONFIG_PCIE_POWER_SAVING
#if !defined(CONFIG_NET_PCI) && !defined(CONFIG_RTL_8196CS) && (!defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_ULINKER)) && !defined(CONFIG_RTL_88E_SUPPORT)
#define PCIE_POWER_SAVING
#endif

#endif

#ifdef _SINUX_
#define PCIE_POWER_SAVING
#endif

#ifdef PCIE_POWER_SAVING
	#define GPIO_WAKEPIN
	#define FIB_96C
//	#define PCIE_POWER_SAVING_DEBUG
//	#define ASPM_ENABLE
#ifdef PCIE_POWER_SAVING_DEBUG
	#define PCIE_L2_ENABLE
#endif

#define CONFIG_SLOT0H	0xb8b00000
#define CONFIG_SLOT0S	0xb8b10000
#define CONFIG_SLOT1H	0xb8b20000
#define CONFIG_SLOT1S	0xb8b30000


#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
#define CFG_92C_SLOTH		CONFIG_SLOT0H
#define CFG_92C_SLOTS		CONFIG_SLOT0S
#if (RTL_USED_PCIE_SLOT==1)
#define CFG_92D_SLOTH		CONFIG_SLOT1H
#define CFG_92D_SLOTS		CONFIG_SLOT1S
#else
#define CFG_92D_SLOTH		CONFIG_SLOT0H
#define CFG_92D_SLOTS		CONFIG_SLOT0S
#endif
#elif defined(CONFIG_RTL_8196C)
#define CFG_92C_SLOTH		CONFIG_SLOT0H
#define CFG_92C_SLOTS		CONFIG_SLOT0S
#define CFG_92D_SLOTH		CONFIG_SLOT0H
#define CFG_92D_SLOTS		CONFIG_SLOT0S
#endif


#endif


//-------------------------------------------------------------
// WDS function support
//-------------------------------------------------------------
#if defined(CONFIG_RTL_WDS_SUPPORT)
#define WDS
//	#define LAZY_WDS
#endif


//-------------------------------------------------------------
// Pass EAP packet by event queue
//-------------------------------------------------------------
#define EAP_BY_QUEUE
#undef EAPOLSTART_BY_QUEUE	// jimmylin: don't pass eapol-start up
							// due to XP compatibility issue
//#define USE_CHAR_DEV
#define USE_PID_NOTIFY


//-------------------------------------------------------------
// WPA Supplicant 
//-------------------------------------------------------------
//#define WIFI_WPAS

#ifdef WIFI_WPAS
#ifndef WIFI_HAPD
#define WIFI_HAPD
#endif

#ifndef CLIENT_MODE
#define CLIENT_MODE
#endif
#endif


//-------------------------------------------------------------
// Client mode function support
//-------------------------------------------------------------
#if defined(CONFIG_RTL_CLIENT_MODE_SUPPORT)
#define CLIENT_MODE
#endif

#ifdef CLIENT_MODE
	#define RTK_BR_EXT		// Enable NAT2.5 and MAC clone support
	#define CL_IPV6_PASS	// Enable IPV6 pass-through. RTK_BR_EXT must be defined
#endif

#ifdef CONFIG_RTL_SUPPORT_MULTI_PROFILE
	#define SUPPORT_MULTI_PROFILE		// support multiple AP profile
#endif


//-------------------------------------------------------------
// Defined when WPA2 is used
//-------------------------------------------------------------
#define RTL_WPA2
#define RTL_WPA2_PREAUTH


//-------------------------------------------------------------
// MP test
//-------------------------------------------------------------
#if (!defined(CONFIG_RTL8196B_GW_8M) || defined(CONFIG_RTL8196B_GW_MP)) && !defined(__ECOS)
#define MP_TEST
#ifdef CONFIG_RTL_92D_SUPPORT
//#define TEMP_MP_92D
#endif
#endif


//-------------------------------------------------------------
// MIC error test
//-------------------------------------------------------------
//#define MICERR_TEST


//-------------------------------------------------------------
// Log event
//-------------------------------------------------------------
#define EVENT_LOG


//-------------------------------------------------------------
// Tx/Rx data path shortcut
//-------------------------------------------------------------
#define TX_SHORTCUT
#define RX_SHORTCUT
#if defined(CONFIG_RTK_MESH) && defined(RX_SHORTCUT)
#define RX_RL_SHORTCUT
#endif
#ifdef __ECOS
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_BRSC
#define BR_SHORTCUT
#endif
#elif !defined(CONFIG_RTL_FASTBRIDGE)
#ifdef CONFIG_RTL8672
#ifndef CONFIG_RTL8672_BRIDGE_FASTPATH
#define BR_SHORTCUT
#endif
#else
#ifndef CONFIG_RTL_8198B
#define BR_SHORTCUT
#define BR_SHORTCUT_C2
#endif
#endif
#endif
#if defined(CONFIG_RTK_MESH) && defined(TX_SHORTCUT)
	#define MESH_TX_SHORTCUT
#endif

#define TX_SC_ENTRY_NUM		1


//-------------------------------------------------------------
// back to back test
//-------------------------------------------------------------
//#define B2B_TEST


//-------------------------------------------------------------
// enable e-fuse read write
//-------------------------------------------------------------
#ifdef CONFIG_ENABLE_EFUSE
#define EN_EFUSE
#endif


//-------------------------------------------------------------
// new Auto channel
//-------------------------------------------------------------
#define CONFIG_RTL_NEW_AUTOCH


//-------------------------------------------------------------
// new IQ calibration for 92c / 88c
//-------------------------------------------------------------
#define CONFIG_RTL_NEW_IQK


//-------------------------------------------------------------
// noise control
//-------------------------------------------------------------
#ifdef CONFIG_RTL_92C_SUPPORT
//#define CONFIG_RTL_NOISE_CONTROL_92C
#endif


//-------------------------------------------------------------
// Universal Repeater (support AP + Infra client concurrently)
//-------------------------------------------------------------
#if defined(CONFIG_RTL_REPEATER_MODE_SUPPORT)
#define UNIVERSAL_REPEATER
#define SMART_REPEATER_MODE
#ifdef SMART_REPEATER_MODE
	#define CHECK_VXD_AP_TIMEOUT	RTL_SECONDS_TO_JIFFIES(60)
#endif
//#define	SWITCH_CHAN
#endif


//-------------------------------------------------------------
// Check hangup for Tx queue
//-------------------------------------------------------------
#define CHECK_HANGUP
#ifdef CHECK_HANGUP
	#define CHECK_TX_HANGUP
	#define CHECK_RX_DMA_ERROR
	#define FAST_RECOVERY
#endif


//-------------------------------------------------------------
// DFS
//-------------------------------------------------------------
#ifdef CONFIG_RTL_DFS_SUPPORT
#define DFS
#endif


//-------------------------------------------------------------
// Driver based WPA PSK feature
//-------------------------------------------------------------
#define INCLUDE_WPA_PSK


//-------------------------------------------------------------
// RF Fine Tune
//-------------------------------------------------------------
//#define RF_FINETUNE


//-------------------------------------------------------------
// Wifi WMM
//-------------------------------------------------------------
#define WIFI_WMM
#ifdef WIFI_WMM
	#define WMM_APSD	// WMM Power Save
	#define RTL_MANUAL_EDCA		// manual EDCA parameter setting
#endif


//-------------------------------------------------------------
// Hostapd 
//-------------------------------------------------------------
//#ifdef CONFIG_RTL_HOSTAPD_SUPPORT 
//#define WIFI_HAPD
//#endif
#ifdef CONFIG_RTL_P2P_SUPPORT
#define P2P_SUPPORT  //  support for WIFI_Direct
//#define P2P_DEBUGMSG
#endif

#ifdef WIFI_HAPD

//#define HAPD_DRV_PSK_WPS

#ifndef CONFIG_RTL_COMAPI_WLTOOLS
#define CONFIG_RTL_COMAPI_WLTOOLS
#endif

#ifndef HAPD_DRV_PSK_WPS
#undef EAP_BY_QUEUE
#undef INCLUDE_WPA_PSK
#endif

#endif


//-------------------------------------------------------------
// IO mapping access
//-------------------------------------------------------------
//#define IO_MAPPING


//-------------------------------------------------------------
// Wifi Simple Config support
//-------------------------------------------------------------
#define WIFI_SIMPLE_CONFIG
/* WPS2DOTX   */
#define WPS2DOTX
#define OUI_LEN					4

#ifdef WPS2DOTX
#define SUPPORT_PROBE_REQ_REASSEM	//for AP mode
#define SUPPORT_PROBE_RSP_REASSEM	// for STA mode
//#define WPS2DOTX_DEBUG
#endif

#ifdef	WPS2DOTX_DEBUG	  //0614 for wps2.0  trace
#define SECU_DEBUG(fmt, args...)  printk("[secu]%s %d:"fmt, __FUNCTION__,__LINE__, ## args)
#define SME_DEBUG(fmt, args...) printk("[sme]%s %d:"fmt,__FUNCTION__ , __LINE__ , ## args)

#else
#define SECU_DEBUG(fmt, args...)
#define SME_DEBUG(fmt, args...) 
#endif
/* WPS2DOTX   */


//-------------------------------------------------------------
// Support Multiple BSSID
//-------------------------------------------------------------
#if defined(CONFIG_RTL_VAP_SUPPORT)
#define MBSSID
#endif
#ifdef MBSSID
#define RTL8192CD_NUM_VWLAN  4
//#define FW_SW_BEACON
#else
#ifdef CONFIG_RTL8671
#define RTL8192CD_NUM_VWLAN  0
#endif
#endif


//-------------------------------------------------------------
// Support Tx Descriptor Reservation for each interface
//-------------------------------------------------------------
#ifdef CONFIG_RTL_TX_RESERVE_DESC
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
#define RESERVE_TXDESC_FOR_EACH_IF
#endif
#endif


//-------------------------------------------------------------
// Group BandWidth Control
//-------------------------------------------------------------
//#define GBWC


//-------------------------------------------------------------
// Support 802.11 SNMP MIB
//-------------------------------------------------------------
//#define SUPPORT_SNMP_MIB


//-------------------------------------------------------------
// Driver-MAC loopback
//-------------------------------------------------------------
//#define DRVMAC_LB


//-------------------------------------------------------------
// Use perfomance profiling
//-------------------------------------------------------------
//#define PERF_DUMP


//-------------------------------------------------------------
// 1x1 Antenna Diversity
//-------------------------------------------------------------
#ifdef CONFIG_ANT_SWITCH
//#define SW_ANT_SWITCH
#define HW_ANT_SWITCH
//#define GPIO_ANT_SWITCH
#ifdef HW_ANT_SWITCH
#define HW_DIV_ENABLE	(priv->pshare->rf_ft_var.antHw_enable&1)
#endif
#ifdef SW_ANT_SWITCH
#define SW_DIV_ENABLE	(priv->pshare->rf_ft_var.antSw_enable&1)
#endif
#endif


//-------------------------------------------------------------
// WPAI performance issue
//-------------------------------------------------------------
#ifdef CONFIG_RTL_WAPI_SUPPORT
	//#define IRAM_FOR_WIRELESS_AND_WAPI_PERFORMANCE
	#if defined(CONFIG_RTL8192CD)
	// if CONFIG_RTL_HW_WAPI_SUPPORT defined, { SWCRYPTO=1: sw wapi; SWCRYPTO=0: hw wapi. }
	// if CONFIG_RTL_HW_WAPI_SUPPORT not defined, { SWCRYPTO=1: sw wapi;  SWCRYPTO=0: should not be used! }
	#define	CONFIG_RTL_HW_WAPI_SUPPORT		1
	#endif
#endif


//-------------------------------------------------------------
// Use local ring for pre-alloc Rx buffer.
// If no defined, will use kernel skb que
//-------------------------------------------------------------
#ifndef __ECOS
#define RTK_QUE
#endif


//-------------------------------------------------------------
//Support IP multicast->unicast
//-------------------------------------------------------------
#ifndef __ECOS
#define SUPPORT_TX_MCAST2UNI
#define MCAST2UI_REFINE
#endif

#ifdef CLIENT_MODE
#define SUPPORT_RX_UNI2MCAST
#endif

/* for cameo feature*/
#ifdef CONFIG_RTL865X_CMO
	#define IGMP_FILTER_CMO
#endif

// Support  IPV6 multicast->unicast
#ifdef	SUPPORT_TX_MCAST2UNI
	#define	TX_SUPPORT_IPV6_MCAST2UNI
#endif


//-------------------------------------------------------------
// Support  USB tx rate adaptive
//-------------------------------------------------------------
// define it always for object code release
#ifdef CONFIG_USB
	#define USB_PKT_RATE_CTRL_SUPPORT
#endif


//-------------------------------------------------------------
// Support Tx AMSDU
//-------------------------------------------------------------
//#define SUPPORT_TX_AMSDU


//-------------------------------------------------------------
// Mesh Network
//-------------------------------------------------------------
#ifdef CONFIG_RTK_MESH
#define _MESH_ACL_ENABLE_

/*need check Tx AMSDU dependency ; 8196B no support now */
#ifdef	SUPPORT_TX_AMSDU
#define MESH_AMSDU
#endif
//#define MESH_ESTABLISH_RSSI_THRESHOLD
//#define MESH_BOOTSEQ_AUTH
#endif // CONFIG_RTK_MESH


//-------------------------------------------------------------
// Realtek proprietary wake up on wlan mode
//-------------------------------------------------------------
//#define RTK_WOW


//-------------------------------------------------------------
// Support 802.11d
//-------------------------------------------------------------
//#define DOT11D


//-------------------------------------------------------------
// Use static buffer for STA private buffer
//-------------------------------------------------------------
#if !defined(CONFIG_RTL8196B_GW_8M) && !defined(CONFIG_WIRELESS_LAN_MODULE)
#define PRIV_STA_BUF
#endif

//-------------------------------------------------------------
// Do not drop packet immediately when rx buffer empty
//-------------------------------------------------------------
#ifdef CONFIG_RTL8190_PRIV_SKB
	#define DELAY_REFILL_RX_BUF
#endif


//-------------------------------------------------------------
// WiFi 11n 20/40 coexistence
//-------------------------------------------------------------
#define WIFI_11N_2040_COEXIST


//-------------------------------------------------------------
// Add TX power by command
//-------------------------------------------------------------
#define ADD_TX_POWER_BY_CMD


//-------------------------------------------------------------
// Do Rx process in tasklet
//-------------------------------------------------------------
//#define RX_TASKLET


//-------------------------------------------------------------
// Support external high power PA
//-------------------------------------------------------------
#ifdef CONFIG_HIGH_POWER_EXT_PA
#define HIGH_POWER_EXT_PA
#endif


//-------------------------------------------------------------
// Cache station info for bridge
//-------------------------------------------------------------
#define RTL_CACHED_BR_STA


//-------------------------------------------------------------
// Use default keys of WEP (instead of keymapping keys)
//-------------------------------------------------------------
//#define USE_WEP_DEFAULT_KEY


//-------------------------------------------------------------
// Concurrent support up to 64 clients
//-------------------------------------------------------------
//#define STA_EXT


//-------------------------------------------------------------
// Auto test support
//-------------------------------------------------------------
#ifdef CONFIG_RTL_92C_SUPPORT
#define AUTO_TEST_SUPPORT
#endif


//-------------------------------------------------------------
// to prevent broadcast storm attacks
//-------------------------------------------------------------
#define PREVENT_BROADCAST_STORM	1

#ifdef PREVENT_BROADCAST_STORM
/*
 *	NOTE: The driver will skip the other broadcast packets if the system free memory is less than FREE_MEM_LOWER_BOUND 
 *		   and the broadcast packet amount is larger than BROADCAST_STORM_THRESHOLD in one second period.
 */

#define BROADCAST_STORM_THRESHOLD	8
#define FREE_MEM_LOWER_BOUND			800 //uint: KBytes
#endif


//-------------------------------------------------------------
// Video streaming refine
//-------------------------------------------------------------
#ifdef CONFIG_RTK_VLC_SPEEDUP_SUPPORT
	#define VIDEO_STREAMING_REFINE
#endif


//-------------------------------------------------------------
// Rx buffer gather feature
//-------------------------------------------------------------
#ifdef CONFIG_RTL8672
//#define RX_BUFFER_GATHER
#else
#define RX_BUFFER_GATHER
#endif


//-------------------------------------------------------------
// A4 client support
//-------------------------------------------------------------
//#define A4_STA

#if defined(A4_STA) && defined(WDS)
	#error "A4_STA and WDS can't be used together\n"
#endif


//-------------------------------------------------------------
// Avoid deadlock for SMP(Symmetrical Multi-Processing) architecture
//-------------------------------------------------------------
//#define SMP_SYNC


//-------------------------------------------------------------
// NOT use Realtek specified BSP
//-------------------------------------------------------------
//#define NOT_RTK_BSP


//-------------------------------------------------------------
// No padding between members in the structure for specified CPU
//-------------------------------------------------------------
//#define PACK_STRUCTURE


//-------------------------------------------------------------
// customers proprietary info display
//-------------------------------------------------------------
//#define TLN_STATS


//-------------------------------------------------------------
// Tx early mode
//-------------------------------------------------------------
//#ifdef CONFIG_RTL_TX_EARLY_MODE_SUPPORT
#ifdef CONFIG_RTL_88E_SUPPORT
	#define TX_EARLY_MODE
#endif


//-------------------------------------------------------------
// Dynamically switch LNA for 97d High Power in MP mode to pass Rx Test
//-------------------------------------------------------------
#if defined(CONFIG_RTL_819XD) && !defined(CONFIG_RTL_8196D) && defined(CONFIG_HIGH_POWER_EXT_PA) //for 97d High Power only
//#define MP_SWITCH_LNA
#endif


//-------------------------------------------------------------
// RTL8188E GPIO Control
//-------------------------------------------------------------
#define RTLWIFINIC_GPIO_CONTROL


//-------------------------------------------------------------
// Tx power limit function
//-------------------------------------------------------------
#ifdef CONFIG_TXPWR_LMT
#define TXPWR_LMT
#endif


/*********************************************************************/
/* some definitions in 8192cd driver, we set them as NULL definition */
/*********************************************************************/
#ifdef USE_RTL8186_SDK
#ifdef CONFIG_WIRELESS_LAN_MODULE
#define __DRAM_IN_865X
#define __IRAM_IN_865X
#else
#define __DRAM_IN_865X		__attribute__ ((section(".dram-rtkwlan")))
#define __IRAM_IN_865X		__attribute__ ((section(".iram-rtkwlan")))
#endif

#define RTL8190_DIRECT_RX						/* For packet RX : directly receive the packet instead of queuing it */
#define RTL8190_ISR_RX							/* process RXed packet in interrupt service routine: It become useful only when RTL8190_DIRECT_RX is defined */

#ifndef CONFIG_WIRELESS_LAN_MODULE
#ifndef CONCURRENT_MODE
#define RTL8192CD_VARIABLE_USED_DMEM			/* Use DMEM for some critical variables */
#endif
#endif

#else // not USE_RTL8186_SDK

#define __DRAM_IN_865X
#define __IRAM_IN_865X
#endif


#undef __MIPS16
#ifdef __ECOS
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_USE_MIPS16
#define __MIPS16			__attribute__ ((mips16))
#else
#define __MIPS16
#endif
#else
#if defined(CONFIG_WIRELESS_LAN_MODULE)
#define __MIPS16
#else
#define __MIPS16			__attribute__ ((mips16))
#endif
#endif

#ifdef IRAM_FOR_WIRELESS_AND_WAPI_PERFORMANCE
#define __IRAM_WLAN_HI		__attribute__ ((section(".iram-wapi")))
#define __DRAM_WLAN_HI		__attribute__ ((section(".dram-wapi")))
#endif


//-------------------------------------------------------------
// Kernel 2.6 specific config
//-------------------------------------------------------------
#ifdef __LINUX_2_6__

#define USE_RLX_BSP
#ifndef CONCURRENT_MODE
#define RTL8192CD_VARIABLE_USED_DMEM
#endif

#ifndef RX_TASKLET
	#define	RX_TASKLET
#endif

#endif


#if 0
//-------------------------------------------------------------
// TR define flag
//-------------------------------------------------------------
#if defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8196C_EC)

#ifndef INCLUDE_WPA_PSK
	#define INCLUDE_WPA_PSK
#endif

#ifdef UNIVERSAL_REPEATER
	#undef UNIVERSAL_REPEATER
#endif

#ifdef CLIENT_MODE
	#undef CLIENT_MODE
	#undef RTK_BR_EXT
#endif

#ifndef WIFI_SIMPLE_CONFIG
	#define WIFI_SIMPLE_CONFIG
#endif

#ifdef GBWC
	#undef GBWC
#endif

#ifdef SUPPORT_SNMP_MIB
	#undef SUPPORT_SNMP_MIB
#endif

#endif // CONFIG_RTL8196B_TR


//-------------------------------------------------------------
// AC define flag
//-------------------------------------------------------------
#ifdef CONFIG_RTL865X_AC

#ifndef INCLUDE_WPA_PSK
	#define INCLUDE_WPA_PSK
#endif

#ifdef UNIVERSAL_REPEATER
	#undef UNIVERSAL_REPEATER
#endif

#ifdef CLIENT_MODE
	#undef CLIENT_MODE
	#undef RTK_BR_EXT
#endif

#ifndef WIFI_SIMPLE_CONFIG
	#define WIFI_SIMPLE_CONFIG
#endif

#ifdef GBWC
	#undef GBWC
#endif

#ifdef SUPPORT_SNMP_MIB
	#undef SUPPORT_SNMP_MIB
#endif

#endif // CONFIG_RTL865X_AC
#endif //#if 0


//-------------------------------------------------------------
// Config Little Endian CPU
//-------------------------------------------------------------
#ifdef _LITTLE_ENDIAN_

#ifndef NOT_RTK_BSP
#define NOT_RTK_BSP
#endif

#ifndef CONFIG_WIRELESS_LAN_MODULE
#define CONFIG_WIRELESS_LAN_MODULE
#endif

#ifdef __MIPSEB__
	#undef __MIPSEB__
#endif

#ifdef _BIG_ENDIAN_
	#undef _BIG_ENDIAN_
#endif

#endif //_LITTLE_ENDIAN_


//-------------------------------------------------------------
// Config if NOT use Realtek specified BSP
//-------------------------------------------------------------
#ifdef NOT_RTK_BSP

#ifndef CONFIG_NET_PCI
#define CONFIG_NET_PCI
#endif

#ifndef __LINUX_2_6__
#define del_timer_sync del_timer
#endif

#if LINUX_VERSION_CODE >= 0x02061D // linux 2.6.29
#define NETDEV_NO_PRIV
#endif

#ifndef LINUX_2_6_22_
#define CONFIG_COMPAT_NET_DEV_OPS
#endif

#ifdef CONFIG_RTL_CUSTOM_PASSTHRU
#undef CONFIG_RTL_CUSTOM_PASSTHRU
#endif

#ifdef CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE
#undef CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE
#endif

#ifdef _USE_DRAM_
	#undef _USE_DRAM_
#endif

#ifdef CHECK_SWAP
	#undef CHECK_SWAP
#endif

#ifdef EVENT_LOG
	#undef EVENT_LOG
#endif

#ifdef BR_SHORTCUT
	#undef BR_SHORTCUT
#endif

#ifdef RTK_BR_EXT
	#undef RTK_BR_EXT
#endif

#ifdef UNIVERSAL_REPEATER
	#undef UNIVERSAL_REPEATER
#endif

#ifdef GBWC
	#undef GBWC
#endif

#ifdef USE_IO_OPS
	#undef USE_IO_OPS
#endif

#ifdef IO_MAPPING
	#undef IO_MAPPING
#endif

#ifdef RTK_QUE
	#undef RTK_QUE
#endif

#ifdef USE_RLX_BSP
#undef USE_RLX_BSP
#endif

#ifdef RX_BUFFER_GATHER
#undef RX_BUFFER_GATHER
#endif

#endif //NOT_RTK_BSP


//-------------------------------------------------------------
// Define flag of EC system
//-------------------------------------------------------------
#ifdef CONFIG_RTL8196C_EC

#ifndef USE_WEP_DEFAULT_KEY
	#define USE_WEP_DEFAULT_KEY
#endif

#ifdef TESTCHIP_SUPPORT
	#undef TESTCHIP_SUPPORT
#endif

#endif 


//-------------------------------------------------------------
// MSC define flag
//-------------------------------------------------------------
#ifdef _SINUX_
#define CONFIG_MSC 
#endif

#ifdef CONFIG_MSC
#define INCLUDE_WPS

#ifdef CHECK_HANGUP
	#undef CHECK_HANGUP
#endif
#ifndef USE_WEP_DEFAULT_KEY         //2010.4.23 Fred Fu open it to support wep key index 1 2 3
	#define USE_WEP_DEFAULT_KEY
#endif

#ifdef WIFI_SIMPLE_CONFIG
#ifdef	INCLUDE_WPS
#define USE_PORTING_OPENSSL
#endif
#endif

#endif

//-------------------------------------------------------------
// Define flag of 8672 system
//-------------------------------------------------------------
#ifdef CONFIG_RTL8672

#undef DRV_RELDATE
#define DRV_RELDATE		"2010-03-31/2011-07-08"

#ifndef RX_TASKLET
	#define RX_TASKLET
#endif

#ifdef RTL8190_DIRECT_RX
	#undef RTL8190_DIRECT_RX
#endif

#ifdef RTL8190_ISR_RX
	#undef RTL8190_ISR_RX
#endif

#if defined(USE_RLX_BSP) && !defined(LINUX_2_6_22_)
	#undef USE_RLX_BSP
#endif

#undef TX_SC_ENTRY_NUM
#define TX_SC_ENTRY_NUM		2

#ifdef __DRAM_IN_865X
	#undef __DRAM_IN_865X
#endif
#define __DRAM_IN_865X		__attribute__ ((section(".dram-rtkwlan")))

#ifdef __IRAM_IN_865X
	#undef __IRAM_IN_865X
#endif
#define __IRAM_IN_865X		__attribute__ ((section(".iram-rtkwlan")))

#ifdef __IRAM_IN_865X_HI
	#undef __IRAM_IN_865X_HI
#endif
#define __IRAM_IN_865X_HI	__attribute__ ((section(".iram-tx")))

#define USE_TXQUEUE
#ifdef USE_TXQUEUE
	#define TXQUEUE_SIZE	512
#endif

// Support dynamically adjust TXOP in low throughput feature
#define LOW_TP_TXOP

// Support four different AC stream
#define WMM_VIBE_PRI

// Resist interference 
#define INTERFERENCE_CONTROL
#ifdef PCIE_POWER_SAVING
	#undef PCIE_POWER_SAVING
#endif

#ifdef CONFIG_RTL_8196C
	#undef CONFIG_RTL_8196C
#endif
#ifdef CONFIG_RTL8196C_REVISION_B
	#undef CONFIG_RTL8196C_REVISION_B
#endif


#endif // CONFIG_RTL8672


//-------------------------------------------------------------
// Define flag of rtl8192d features
//-------------------------------------------------------------
#ifdef CONFIG_RTL_92D_SUPPORT
#define SW_LCK_92D
#define DPK_92D

#define RX_GAIN_TRACK_92D
//#define CONFIG_RTL_NOISE_CONTROL
#ifdef CONFIG_RTL_92D_DMDP
//#define NON_INTR_ANTDIV
#endif

//#ifdef CONFIG_RTL_92D_INT_PA
#define RTL8192D_INT_PA
//#endif

#ifdef RTL8192D_INT_PA
//Use Gain Table with suffix '_new'  for purpose
//1. refine the large gap between power index 39 &40
//#define RTL8192D_INT_PA_GAIN_TABLE_NEW //for both Non-USB & USB Power

//Use Gain Table with suffix '_new1' for purpose
//1. refine the large gap between power index 39 &40
//2. increase tx power 
//#define RTL8192D_INT_PA_GAIN_TABLE_NEW1 //for USB Power only
#endif

#endif


//-------------------------------------------------------------
// Define flag of RTL8188E features
//-------------------------------------------------------------
#ifdef CONFIG_RTL_88E_SUPPORT
/* RTL8188E test chip support*/
#define SUPPORT_RTL8188E_TC
#ifndef CALIBRATE_BY_ODM

//for 8188E IQK
#define IQK_BB_REG_NUM		9		

//for 88e tx power tracking
#define	index_mapping_NUM_88E	15
#define AVG_THERMAL_NUM_88E		4
#define IQK_Matrix_Settings_NUM	1+24+21
#define	AVG_THERMAL_NUM 8
#define	HP_THERMAL_NUM 8

#define	RF_T_METER_88E			0x42
//===
#endif

#endif


#if 0
//-------------------------------------------------------------
// TLD define flag
//-------------------------------------------------------------
#ifdef CONFIG_RTL8196B_TLD

#ifndef STA_EXT
	#define STA_EXT
#endif

#ifdef GBWC
	#undef GBWC
#endif

#ifdef SUPPORT_SNMP_MIB
	#undef SUPPORT_SNMP_MIB
#endif

#ifdef DRVMAC_LB
	#undef DRVMAC_LB
#endif

#ifdef HIGH_POWER_EXT_PA
	#undef HIGH_POWER_EXT_PA
#endif

#ifdef ADD_TX_POWER_BY_CMD
	#undef ADD_TX_POWER_BY_CMD
#endif

#endif // CONFIG_RTL8196B_TLD
#endif



//-------------------------------------------------------------
// KLD define flag
//-------------------------------------------------------------
#if defined(CONFIG_RTL8196C_KLD)

#ifndef INCLUDE_WPA_PSK
	#define INCLUDE_WPA_PSK
#endif

#ifdef UNIVERSAL_REPEATER
	#undef UNIVERSAL_REPEATER
#endif

#ifndef WIFI_SIMPLE_CONFIG
	#define WIFI_SIMPLE_CONFIG
#endif

#ifdef GBWC
	#undef GBWC
#endif

#ifdef SUPPORT_SNMP_MIB
	#undef SUPPORT_SNMP_MIB
#endif

#ifdef DRVMAC_LB
	#undef DRVMAC_LB
#endif

#ifdef MBSSID
	#undef RTL8192CD_NUM_VWLAN
	#define RTL8192CD_NUM_VWLAN  1
#endif

//#ifdef HIGH_POWER_EXT_PA
//	#undef HIGH_POWER_EXT_PA
//#endif

//#ifdef ADD_TX_POWER_BY_CMD
//	#undef ADD_TX_POWER_BY_CMD
//#endif

#endif // CONFIG_RTL8196C_KLD


//-------------------------------------------------------------
// eCos define flag
//-------------------------------------------------------------
#ifdef __ECOS
	//#undef CLIENT_MODE
	//#undef UNIVERSAL_REPEATER
	#undef USE_PID_NOTIFY
	//#undef EAP_BY_QUEUE
	#undef EVENT_LOG
	#undef MP_TEST
	#undef SUPPORT_TX_MCAST2UNI
	//#undef MBSSID
	#undef AUTO_TEST_SUPPORT
	#undef SUPPORT_RX_UNI2MCAST
#if !defined(UNIVERSAL_REPEATER) && !defined(MBSSID)
	#define USE_WEP_DEFAULT_KEY
#endif	
	#undef USE_WLAN_TIMER_FOR_RC		/* use wlan nic timer for reoder-control timer */

#ifdef RTL8192CD_NUM_VWLAN
	#undef RTL8192CD_NUM_VWLAN
	#define RTL8192CD_NUM_VWLAN	RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM
#endif
	
#ifdef CYGSEM_HAL_IMEM_SUPPORT
	#define __IRAM_IN_865X		__attribute__ ((section(".iram-rtkwlan")))
#else
	#define __IRAM_IN_865X
#endif
#ifdef CYGSEM_HAL_DMEM_SUPPORT
	#define __DRAM_IN_865X		__attribute__ ((section(".dram-rtkwlan")))
#else
	#define __DRAM_IN_865X
#endif

	#define dev_kfree_skb_any(skb)	kfree_skb_chk_key(skb, wlan_device[0].priv->dev);
#endif /* __ECOS */


//-------------------------------------------------------------
// Dependence check of define flag
//-------------------------------------------------------------
#if defined(B2B_TEST) && !defined(MP_TEST)
	#error "Define flag error, MP_TEST is not defined!\n"
#endif


#if defined(UNIVERSAL_REPEATER) && !defined(CLIENT_MODE)
	#error "Define flag error, CLIENT_MODE is not defined!\n"
#endif


#if defined(TX_EARLY_MODE) && !defined(SW_TX_QUEUE)
	#error "Define flag error, SW_TX_QUEUE is not defined!\n"
#endif


/*=============================================================*/
/*------ Compiler Portability Macros --------------------------*/
/*=============================================================*/
#ifdef EVENT_LOG
	extern int scrlog_printk(const char * fmt, ...);
#ifdef CONFIG_RTK_MESH
/*
 *	NOTE: dot1180211sInfo.log_enabled content from webpage MIB_LOG_ENABLED (bitmap) (in AP/goahead-2.1.1/LINUX/fmmgmt.c  formSysLog)
 */
	#define _LOG_MSG(fmt, args...)	if (1 & GET_MIB(priv)->dot1180211sInfo.log_enabled) scrlog_printk(fmt, ## args)
	#define LOG_MESH_MSG(fmt, args...)	if (16 & GET_MIB(priv)->dot1180211sInfo.log_enabled) _LOG_MSG("%s: " fmt, priv->mesh_dev->name, ## args)
#else
	#define _LOG_MSG(fmt, args...)	scrlog_printk(fmt, ## args)
#endif
#if defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8196C_EC)
	#define _NOTICE	"NOTICElog_num:13;msg:"
	#define _DROPT	"DROPlog_num:13;msg:"
	#define _SYSACT "SYSACTlog_num:13;msg:"

	#define LOG_MSG_NOTICE(fmt, args...) _LOG_MSG("%s" fmt, _NOTICE, ## args)
	#define LOG_MSG_DROP(fmt, args...) _LOG_MSG("%s" fmt, _DROPT, ## args)
	#define LOG_MSG_SYSACT(fmt, args...) _LOG_MSG("%s" fmt, _SYSACT, ## args)
	#define LOG_MSG(fmt, args...)	{}

	#define LOG_START_MSG() { \
			char tmpbuf[10]; \
			LOG_MSG_NOTICE("Access Point: %s started at channel %d;\n", \
				priv->pmib->dot11StationConfigEntry.dot11DesiredSSID, \
				priv->pmib->dot11RFEntry.dot11channel); \
			if (priv->pmib->dot11StationConfigEntry.autoRate) \
				strcpy(tmpbuf, "best"); \
			else \
				sprintf(tmpbuf, "%d", get_rate_from_bit_value(priv->pmib->dot11StationConfigEntry.fixedTxRate)/2); \
			LOG_MSG_SYSACT("AP 2.4GHz mode Ready. Channel : %d TxRate : %s SSID : %s;\n", \
				priv->pmib->dot11RFEntry.dot11channel, \
				tmpbuf, priv->pmib->dot11StationConfigEntry.dot11DesiredSSID); \
	}

#elif defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
	#define _NOTICE	"NOTICElog_num:13;msg:"
	#define _DROPT	"DROPlog_num:13;msg:"
	#define _SYSACT "SYSACTlog_num:13;msg:"

	#define LOG_MSG_NOTICE(fmt, args...) _LOG_MSG("%s" fmt, _NOTICE, ## args)
	#define LOG_MSG_DROP(fmt, args...) _LOG_MSG("%s" fmt, _DROPT, ## args)
	#define LOG_MSG_SYSACT(fmt, args...) _LOG_MSG("%s" fmt, _SYSACT, ## args)
	#define LOG_MSG(fmt, args...)	{}

	#define LOG_START_MSG() { \
			char tmpbuf[10]; \
			LOG_MSG_NOTICE("Access Point: %s started at channel %d;\n", \
				priv->pmib->dot11StationConfigEntry.dot11DesiredSSID, \
				priv->pmib->dot11RFEntry.dot11channel); \
			if (priv->pmib->dot11StationConfigEntry.autoRate) \
				strcpy(tmpbuf, "best"); \
			else \
				sprintf(tmpbuf, "%d", get_rate_from_bit_value(priv->pmib->dot11StationConfigEntry.fixedTxRate)/2); \
			LOG_MSG_SYSACT("AP 2.4GHz mode Ready. Channel : %d TxRate : %s SSID : %s;\n", \
				priv->pmib->dot11RFEntry.dot11channel, \
				tmpbuf, priv->pmib->dot11StationConfigEntry.dot11DesiredSSID); \
	}
#elif defined(CONFIG_RTL8196B_TLD)
	#define LOG_MSG_DEL(fmt, args...)	_LOG_MSG(fmt, ## args)
	#define LOG_MSG(fmt, args...)	{}
#else
	#define LOG_MSG(fmt, args...)	_LOG_MSG("%s: "fmt, priv->dev->name, ## args)
#endif
#else
	#if defined(__GNUC__) || defined(GREEN_HILL)
		#define LOG_MSG(fmt, args...)	{}
	#else
		#define LOG_MSG
	#endif
#endif // EVENT_LOG

#ifdef _USE_DRAM_
	#define DRAM_START_ADDR		0x81000000	// start address of internal data ram
#endif

#ifndef __ECOS
#ifdef __GNUC__
#define __WLAN_ATTRIB_PACK__		__attribute__ ((packed))
#define __PACK
#endif
#endif

#ifdef __arm
#define __WLAN_ATTRIB_PACK__
#define __PACK	__packed
#endif

#if defined(GREEN_HILL) || defined(__ECOS)
#define __WLAN_ATTRIB_PACK__
#define __PACK
#endif


/*=============================================================*/
/*-----------_ Driver module flags ----------------------------*/
/*=============================================================*/
#if defined(CONFIG_WIRELESS_LAN_MODULE) || defined(CONFIG_RTL_ULINKER_WLAN_DELAY_INIT)
	#define	MODULE_NAME		"Realtek WirelessLan Driver"
	#define	MODULE_VERSION	"v1.00"

	#define MDL_DEVINIT
	#define MDL_DEVEXIT
	#define MDL_INIT
	#define MDL_EXIT
	#define MDL_DEVINITDATA
#else
	#define MDL_DEVINIT		__devinit
	#define MDL_DEVEXIT		__devexit
	#define MDL_INIT		__init
	#define MDL_EXIT		__exit
	#define MDL_DEVINITDATA	__devinitdata
#endif

#define RADIUS_ACCOUNTING


/*=============================================================*/
/*----------- System configuration ----------------------------*/
/*=============================================================*/
#if defined(CONFIG_RTL8196B_GW_8M)
#define NUM_TX_DESC		200
#else
#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) 
#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN_D)
#define NUM_TX_DESC		640
#else
#define NUM_TX_DESC		512
#endif
#else
#define NUM_TX_DESC		256		// kmalloc max size issue
#endif
#endif

#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) 
	#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN_D)
		#define NUM_RX_DESC		256
	#else
		#define NUM_RX_DESC		128
	#endif
#elif defined(CONFIG_RTL_92D_SUPPORT)
	#define NUM_RX_DESC		80
#elif defined(__ECOS)
	#define NUM_RX_DESC		64
	#undef NUM_TX_DESC
	#define NUM_TX_DESC		512

#else
	#ifdef RX_BUFFER_GATHER
		#define NUM_RX_DESC		64
	#else
		#define NUM_RX_DESC		32
	#endif
#endif

#define CURRENT_NUM_TX_DESC	priv->pshare->current_num_tx_desc
#if defined (CONFIG_RTL_92D_SUPPORT) && defined(CONFIG_RTL_92D_DMDP)
#define MAX_NUM_TX_DESC_DMDP		256
#endif

#ifdef DELAY_REFILL_RX_BUF
	#define REFILL_THRESHOLD	NUM_RX_DESC
#endif

#define FW_NUM_STAT 32
#ifdef CONFIG_RTL_88E_SUPPORT
#define RTL8188E_NUM_STAT 64
#endif

#if (defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)) && defined(MBSSID)
#define NUM_CMD_DESC	2
#else
#define NUM_CMD_DESC	16
#endif

#ifdef STA_EXT
#define NUM_STAT		64
#else
#ifdef CONFIG_RTL_88E_SUPPORT
#define NUM_STAT		(RTL8188E_NUM_STAT - 1)
#else
#define NUM_STAT		(FW_NUM_STAT - 1)
#endif
#endif
#define MAX_GUEST_NUM   NUM_STAT

#define NUM_TXPKT_QUEUE			64
#define NUM_APSD_TXPKT_QUEUE	32
#define NUM_DZ_MGT_QUEUE	16

#define PRE_ALLOCATED_HDR		NUM_TX_DESC
#ifdef DRVMAC_LB
#define PRE_ALLOCATED_MMPDU		32
#define PRE_ALLOCATED_BUFSIZE	(2048/4)		// 600 bytes long should be enough for mgt! Declare as unsigned int
#else
#define PRE_ALLOCATED_MMPDU		64
#define PRE_ALLOCATED_BUFSIZE	((600+128)/4)		// 600 bytes long should be enough for mgt! Declare as unsigned int
#endif

#define MAX_BSS_NUM		64


#define MAX_NUM_WLANIF		4
#define WLAN_MISC_MAJOR		13

#define MAX_FRAG_COUNT		10

#define NUM_MP_SKB		32

#define SUPPORT_CH_NUM	59

// unit of time out: 10 msec
#define AUTH_TO			RTL_SECONDS_TO_JIFFIES(5)
#define ASSOC_TO		RTL_SECONDS_TO_JIFFIES(5)
#define FRAG_TO			RTL_SECONDS_TO_JIFFIES(20)
#define SS_TO			RTL_MILISECONDS_TO_JIFFIES(50)
#define SS_PSSV_TO		RTL_MILISECONDS_TO_JIFFIES(120)			// passive scan for 120 ms


#ifdef CONFIG_RTL_NEW_AUTOCH
#define SS_AUTO_CHNL_TO		RTL_MILISECONDS_TO_JIFFIES(200)
#endif

#ifdef CONFIG_RTK_MESH
//GANTOE for automatic site survey 2008/12/10
#define SS_RAND_DEFER		300
#endif
#ifdef LINUX_2_6_22_
#define EXPIRE_TO		RTL_SECONDS_TO_JIFFIES(1)
#else
#define EXPIRE_TO		RTL_SECONDS_TO_JIFFIES(1)
#endif
#define REAUTH_TO		RTL_SECONDS_TO_JIFFIES(5)
#define REASSOC_TO		RTL_SECONDS_TO_JIFFIES(5)
#define REAUTH_LIMIT	6
#define REASSOC_LIMIT	6

#define DEFAULT_OLBC_EXPIRE		60

#define GBWC_TO			RTL_MILISECONDS_TO_JIFFIES(250)

#ifdef __DRAYTEK_OS__
#define SS_COUNT		2
#else
#define SS_COUNT		3
#endif

#define TUPLE_WINDOW	64

#define RC_TIMER_NUM	64
#define RC_ENTRY_NUM	128
#define AMSDU_TIMER_NUM	64
#define AMPDU_TIMER_NUM	64

#define ROAMING_DECISION_PERIOD_INFRA	5
#define ROAMING_DECISION_PERIOD_ADHOC	10
#define ROAMING_DECISION_PERIOD_ARRAY 	(MAX_NUM(ROAMING_DECISION_PERIOD_ADHOC,ROAMING_DECISION_PERIOD_INFRA)+1)
#define ROAMING_THRESHOLD		1	// roaming will be triggered when rx
									// beacon percentage is less than the value
#define FAST_ROAMING_THRESHOLD	40

/* below is for security.h  */
#define MAXDATALEN		1560
#define MAXQUEUESIZE	8	//WPS2DOTX
#define MAXRSNIELEN		128
#define E_DOT11_2LARGE	-1
#define E_DOT11_QFULL	-2
#define E_DOT11_QEMPTY	-3
#ifdef WIFI_SIMPLE_CONFIG
#define PROBEIELEN		260
#endif

// for SW LED
#define LED_MAX_PACKET_CNT_B	400
#define LED_MAX_PACKET_CNT_AG	1200
#define LED_MAX_SCALE			100
#define LED_NOBLINK_TIME		RTL_SECONDS_TO_JIFFIES(15)/10	// time more than watchdog interval
#define LED_INTERVAL_TIME		RTL_MILISECONDS_TO_JIFFIES(500)	// 500ms
#define LED_ON_TIME				RTL_MILISECONDS_TO_JIFFIES(40)	// 40ms
#define LED_ON					0
#define LED_OFF					1
#define LED_0 					0
#define LED_1 					1
#define LED_2					2

// for counting association number
#define INCREASE		1
#define DECREASE		0

// DFS
#define CH_AVAIL_CHK_TO			RTL_SECONDS_TO_JIFFIES(62)	 // 62 seconds
#define CH_AVAIL_CHK_TO_CE              RTL_SECONDS_TO_JIFFIES(602)      // 602 seconds

#ifdef RX_BUFFER_GATHER
#ifdef __LINUX_2_6__
#define MAX_SKB_BUF     2280
#else
#define MAX_SKB_BUF     2048
#endif

#define MAX_RX_BUF_LEN	(MAX_SKB_BUF -sizeof(struct skb_shared_info) - 128)
#define MIN_RX_BUF_LEN MAX_RX_BUF_LEN	

#else
#define MAX_RX_BUF_LEN	8400
#define MIN_RX_BUF_LEN	4400
#endif

/* for RTL865x suspend mode */
#define TP_HIGH_WATER_MARK 55 //80 /* unit: Mbps */
#define TP_LOW_WATER_MARK 35 //40 /* unit: Mbps */

#define FW_BOOT_SIZE	400
#define FW_MAIN_SIZE	52000
#define FW_DATA_SIZE	850

#define AGC_TAB_SIZE	1600
#define PHY_REG_SIZE	2048
#define MAC_REG_SIZE	1200
#ifdef CONFIG_RTL_92D_SUPPORT
#define PHY_REG_PG_SIZE 2560
#else
#define PHY_REG_PG_SIZE 256
#endif

#define PHY_REG_1T2R	256
#define PHY_REG_1T1R	256
#define FW_IMEM_SIZE	40*(1024)
#define FW_EMEM_SIZE	50*(1024)
#define FW_DMEM_SIZE	48

// for PCIe power saving
#define POWER_DOWN_T0	(10*HZ)
#define PKT_PAGE_SZ 	128
#define TX_DESC_SZ 		32


#ifdef SUPPORT_TX_MCAST2UNI
#define MAX_IP_MC_ENTRY		8
#define MAX_FLOODING_MAC_NUM 32
#endif

#ifndef CALIBRATE_BY_ODM
#define IQK_ADDA_REG_NUM	16
#define MAX_TOLERANCE		5
#define	IQK_DELAY_TIME		1		//ms
#define IQK_MAC_REG_NUM		4
#endif


#define SKIP_MIC_NUM	300


// for dynamic mechanism of reserving tx desc
#if defined(RESERVE_TXDESC_FOR_EACH_IF) && (defined(UNIVERSAL_REPEATER) || defined(MBSSID))
#define IF_TXDESC_UPPER_LIMIT	70	// percentage
#ifdef USE_TXQUEUE
#define IF_TXQ_UPPER_LIMIT		85	// percentage
#endif
#endif

// for dynamic mechanism of retry count
#define RETRY_TRSHLD_H	3750000
#define RETRY_TRSHLD_L	3125000
#define MP_PSD_SUPPORT 1
//-------------------------------------------------------------
// Define flag for 8M gateway configuration
//-------------------------------------------------------------
#if defined(CONFIG_RTL8196B_GW_8M)

#ifdef MBSSID
	#undef RTL8192CD_NUM_VWLAN
	#define RTL8192CD_NUM_VWLAN  1
#endif

#undef NUM_STAT
#define NUM_STAT		16

#endif // CONFIG_RTL8196B_GW_8M


#if defined(CONFIG_RTL8196C_AP_ROOT) || defined(CONFIG_RTL8198_AP_ROOT)

#ifdef DOT11D
	#undef DOT11D
#endif


#ifdef MBSSID
	#undef RTL8192CD_NUM_VWLAN
	#define RTL8192CD_NUM_VWLAN  1
#endif

#undef NUM_STAT
#define NUM_STAT		16


#endif //defined(CONFIG_RTL8196C_AP_ROOT)
#ifdef CONFIG_RTL_8198_NFBI_RTK_INBAND_AP //mark_nfbi_inband_ap
#ifdef MBSSID
        #undef RTL8192CD_NUM_VWLAN
        #define RTL8192CD_NUM_VWLAN  7
#endif
#endif


#if defined(CONFIG_RTL8196C_CLIENT_ONLY)

#ifdef DOT11D
	#undef DOT11D
#endif

#endif

#ifdef CONCURRENT_MODE
#define NUM_WLAN_IFACE 2
#endif

#ifdef CONFIG_NET_PCI
#ifndef NUM_WLAN_IFACE
#define NUM_WLAN_IFACE 2
#endif
#endif

#if defined(CONFIG_RTL_92D_SUPPORT) && defined(CONFIG_USB_POWER_BUS)
#define USB_POWER_SUPPORT
#endif

#ifdef WIFI_SIMPLE_CONFIG
#define MAX_WSC_IE_LEN		(256+128)
#define MAX_WSC_PROBE_STA	10
#endif


#ifdef CONFIG_WIRELESS_LAN_MODULE //BE_MODULE
#undef __DRAM_IN_865X
#undef __IRAM_IN_865X
#undef __MIPS16

#define __DRAM_IN_865X  
#define __IRAM_IN_865X  
#define __MIPS16 

#undef BR_SHORTCUT
#undef CONFIG_RTL865X_ETH_PRIV_SKB
#undef CONFIG_RTL_ETH_PRIV_SKB
#undef CONFIG_RTK_VLAN_SUPPORT
#endif
#if defined(CONFIG_RTL8672) && defined(LINUX_2_6_22_) && defined(CONFIG_MEM_LIMITATION)
	#undef NUM_STAT
	#define NUM_STAT	16
#endif

//-------------------------------------------------------------
// OUT SOURCE
//-------------------------------------------------------------
#ifdef CONFIG_RTL_88E_SUPPORT
#ifdef CONFIG_RTL_ODM_WLAN_DRIVER
#define USE_OUT_SRC					1
#define CONFIG_MACBBRF_BY_ODM		1
#define RATEADAPTIVE_BY_ODM			1
#define CALIBRATE_BY_ODM			1

// Support four different AC stream
#ifndef WMM_VIBE_PRI
#define WMM_VIBE_PRI
#endif
#define WMM_BEBK_PRI
#endif
#endif

#endif // _8192CD_CFG_H_

