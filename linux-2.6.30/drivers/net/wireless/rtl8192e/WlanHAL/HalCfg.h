#ifndef __HALCFG_H__
#define __HALCFG_H__
/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	HalComDef.h
	
Abstract:
	Defined HAL Mapping Type
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-03-23 Filen            Create.	
--*/


//3  Driver provide some header files in order to eliminate warning message
#include "Wlan_TypeDef.h"
#include "Wlan_QoSType.h"
#include "8192cd_cfg.h"             /// ??????
#include "wifi.h"
#include "8192cd.h"
#include "8192cd_util.h"
#include "8192cd_headers.h"
#ifdef RTK_AC_SUPPORT
#include "8812_vht_gen.h"   // TODO: Filen, this name should be rename to 11AC related(independent with chip)
#endif
extern void delay_us(unsigned int t);
extern void delay_ms(unsigned int t);
extern unsigned char RTL_R8_F(struct rtl8192cd_priv *priv, unsigned int reg);
extern unsigned short RTL_R16_F(struct rtl8192cd_priv *priv, unsigned int reg);
extern unsigned int RTL_R32_F(struct rtl8192cd_priv *priv, unsigned int reg);
extern void RTL_W8_F(struct rtl8192cd_priv *priv, unsigned int reg, unsigned char val8);
extern void RTL_W16_F(struct rtl8192cd_priv *priv, unsigned int reg, unsigned short val16);
extern void RTL_W32_F(struct rtl8192cd_priv *priv, unsigned int reg, unsigned int val32);


//
//  CONFIG_WLAN_HAL_8881A / CONFIG_WLAN_HAL_8192EE / ...
//  Only used to here
//
#ifdef CONFIG_WLAN_HAL_8881A
#define	CODEBASE1	RTL8881AEM
#else
#define	CODEBASE1	0
#endif

#ifdef CONFIG_WLAN_HAL_8192EE
#define	CODEBASE2	RTL8192EE
#else
#define	CODEBASE2	0
#endif


//1 Configuration below are decided by Driver

//3 Setting Compile Option
#define HAL_CODE_BASE   (CODEBASE1|CODEBASE2)
#if !defined(HAL_CODE_BASE)
	#error "HAL_CODE_BASE is not yet defined!\n"
#endif

#define DEV_BUS_TYPE   (RT_EMBEDDED_INTERFACE | RT_PCI_INTERFACE)

//2 HAL Compile flag: 
//Prefix: CFG_HAL_XXXXXXXXXXX
#define CFG_HAL_DBG                          0

#ifdef DRVMAC_LB
#define CFG_HAL_MAC_LOOPBACK                 1
#else
#define CFG_HAL_MAC_LOOPBACK                 0
#endif

#ifdef MP_TEST
#define CFG_HAL_MP_TEST					1
#else
#define CFG_HAL_MP_TEST					0
#endif

#ifdef CLIENT_MODE
#define CFG_HAL_SUPPORT_CLIENT_MODE          1
#else
#define CFG_HAL_SUPPORT_CLIENT_MODE          0
#endif

#ifdef CONFIG_RTL_VAP_SUPPORT
#define CFG_HAL_SUPPORT_MBSSID               1
#else
#define CFG_HAL_SUPPORT_MBSSID               0
#endif

#ifdef UNIVERSAL_REPEATER
#define CFG_HAL_SUPPORT_UNIVERSAL_REPEATER   1
#else
#define CFG_HAL_SUPPORT_UNIVERSAL_REPEATER   0
#endif

#ifdef CHECK_SWAP
#define CFG_HAL_CHECK_SWAP                   1
#else
#define CFG_HAL_CHECK_SWAP                   0
#endif

#ifdef  WIFI_WMM
#define CFG_HAL_WIFI_WMM                     1
#else
#define CFG_HAL_WIFI_WMM                     0
#endif  //WIFI_WMM

#ifdef  RTL_MANUAL_EDCA
#define CFG_HAL_RTL_MANUAL_EDCA              1
#else
#define CFG_HAL_RTL_MANUAL_EDCA              0
#endif  //RTL_MANUAL_EDCA

#if CONFIG_RTL_HW_WAPI_SUPPORT
#define CFG_HAL_RTL_HW_WAPI_SUPPORT          1
else
#define CFG_HAL_RTL_HW_WAPI_SUPPORT          0
#endif  //CONFIG_RTL_HW_WAPI_SUPPORT

#ifdef TX_SHORTCUT
#define CFG_HAL_TX_SHORTCUT                  1
#else
#define CFG_HAL_TX_SHORTCUT                  0
#endif // TX_SHORTCUT

#define CFG_FW_VERIFICATION                  0

#ifdef RTK_AC_SUPPORT
#define CFG_HAL_RTK_AC_SUPPORT               1
#else
#define CFG_HAL_RTK_AC_SUPPORT               0
#endif // RTK_AC_SUPPORT

#define CFG_HAL_MEASURE_BEACON               0

#ifdef DELAY_REFILL_RX_BUF
#define CFG_HAL_DELAY_REFILL_RX_BUF          1
#else
#define CFG_HAL_DELAY_REFILL_RX_BUF          0
#endif // DELAY_REFILL_RX_BUF


//3 Configuration parameter Setting

// config Txpower Setting
#ifdef POWER_PERCENT_ADJUSTMENT
#define CFG_HAL_POWER_PERCENT_ADJUSTMENT    1
#else
#define CFG_HAL_POWER_PERCENT_ADJUSTMENT    0
#endif // POWER_PERCENT_ADJUSTMENT

#ifdef HIGH_POWER_EXT_PA
#define CFG_HAL_HIGH_POWER_EXT_PA    1
#else
#define CFG_HAL_HIGH_POWER_EXT_PA    0
#endif // HIGH_POWER_EXT_PA

#ifdef ADD_TX_POWER_BY_CMD
#define CFG_HAL_ADD_TX_POWER_BY_CMD    1
#else
#define CFG_HAL_ADD_TX_POWER_BY_CMD    0
#endif // ADD_TX_POWER_BY_CMD


//4 AP Service Support
#if CFG_HAL_SUPPORT_MBSSID
#define HAL_NUM_VWLAN   RTL8192CD_NUM_VWLAN
#else
#define HAL_NUM_VWLAN   0
#endif

#if IS_RTL88XX_GENERATION

//3 Method Selection for RXTAG or TOTALRXPKTSIZE of RXBD
// 1: RXTAG
// 0: TOTALRXPKTSIZE
#define RXBD_READY_CHECK_METHOD     1

//4 HW Offset between TXBD Beacon
#if IS_RTL8192E_SERIES
#define TXBD_BEACON_OFFSET_8192E    128 //Filen: Cant's be modified (HW Fixed value)       
#endif  //IS_RTL8192E_SERIES

#if IS_RTL8881A_SERIES
#define TXBD_BEACON_OFFSET_8881A    64  //Filen: Cant's be modified (HW Fixed value)       
#endif  //IS_RTL8881A_SERIES


#define TXBD_BEACON_OFFSET_MAX      128 //Compare all Chip Beacon Offset

//4 TDECTRL
#define SECOND_BCN_PAGE_OFFSET          5   //including Beacon Page + Probe Response Page + .....

//4 RQPN
#if CFG_HAL_MAC_LOOPBACK
#define TX_PAGE_CNT_HPQ                 0x04                 
#define TX_PAGE_CNT_NPQ                 0x29
#define TX_PAGE_CNT_LPQ                 0x04
#define TX_PAGE_CNT_EPQ                 0x04
#define TX_PAGE_CNT_PUBQ                0x38
#define TX_PAGE_CNT_RSV                 (TX_PAGE_CNT_HPQ + TX_PAGE_CNT_NPQ + TX_PAGE_CNT_LPQ + TX_PAGE_CNT_EPQ + TX_PAGE_CNT_PUBQ)
#else
#if 1
#define TX_PAGE_CNT_HPQ                 0x0e
#define TX_PAGE_CNT_NPQ                 0x29
#define TX_PAGE_CNT_LPQ                 0x20
#define TX_PAGE_CNT_EPQ                 0x04
#define TX_PAGE_CNT_PUBQ                0x9a
#if CFG_HAL_SUPPORT_MBSSID
#define TX_PAGE_CNT_RSV                 (SECOND_BCN_PAGE_OFFSET << 1)
#else
#define TX_PAGE_CNT_RSV                 SECOND_BCN_PAGE_OFFSET
#endif // CFG_HAL_SUPPORT_MBSSID
#else
// TODO: AP Power Saving Mode
#define TX_PAGE_CNT_HPQ
#define TX_PAGE_CNT_NPQ
#define TX_PAGE_CNT_LPQ
#define TX_PAGE_CNT_EPQ
#define TX_PAGE_CNT_PUBQ
#define TX_PAGE_CNT_RSV
#endif
#endif

//4 Page Size
//
// we set RPQN pagecnt is less than LLT pagecnt,
// because we avoid some critical condition
//
#define TXPKTBUF_RQPN_PAGECNT           (TX_PAGE_CNT_HPQ + TX_PAGE_CNT_NPQ + TX_PAGE_CNT_LPQ + TX_PAGE_CNT_EPQ + TX_PAGE_CNT_PUBQ)
#define TXPKTBUF_LLT_PAGECNT            (TXPKTBUF_RQPN_PAGECNT+1)   
#define TXPKTBUF_TOTAL_PAGECNT          256


//4 PBP: Packet Buffer Page
//Size Selection:
//  64 / 128 / 256 / 512 / 1024
#define PBP_PSTX_SIZE                   256     // Filen: can't be modified in 92E/8881A
#define PBP_PSRX_SIZE                   128

//4 LLT Table
#define LLT_TABLE_INIT_POLLING_CNT      100

//4 TRX Pktbuf
#define MAC_RXFF_SIZE               0x3E7F  // 16*1024 = 16384, 384 for C2H Pkt, 16000-1=15999=3E7F

//4 TRX DMA Queue
#define TRX_DMA_QUEUE_MAP_PARA          0xC660

//4 Retry Limit
#define RETRY_LIMIT_SHORT_AP        0x10
#define RETRY_LIMIT_LONG_AP         0x10
#define RETRY_LIMIT_SHORT_CLIENT    0x30
#define RETRY_LIMIT_LONG_CLIENT     0x30

//4 CAM
#define HAL_TOTAL_CAM_ENTRY                 32
#define HAL_CAM_CONTENT_COUNT               8
#define HAL_CAM_CONTENT_USABLE_COUNT        6

//4 Firmware
#define	RT_FIRMWARE_HDR_SIZE	            32
#define DOWNLOAD_FIRMWARE_RETRY_TIMES       5
#define FW_DOWNLOAD_START_ADDRESS           0x1000
#define CHECK_FW_RAMCODE_READY_TIMES        10
#define CHECK_FW_RAMCODE_READY_DELAY_MS     5
#define H2CBUF_OCCUPY_DELAY_CNT             30
#define H2CBUF_OCCUPY_DELAY_US              10    

#if IS_EXIST_PCI
//4 PCIE Configuration
//TXDMA Burst Size selection default 7.
//0:16; 1:32; 2:64; 3:128; 4:256; 5:512; 6:1024; 7:2048 bytes.
#define PCIE_TXDMA_BURST_SIZE               0x7

//RXDMA Burst Size selection default 7.
//0:16bytes; 1:32bytes; 2:64bytes; 3~7: 128bytes
#define PCIE_RXDMA_BURST_SIZE               0x7
#endif  //IS_EXIST_PCI

#if IS_EXIST_EMBEDDED
//4 LBUS Configuration
//TXDMA Burst Size selection default 7.
//0:16; 1:32; 2:64; 3:128; 4:256; 5:512; 6:1024; 7:2048 bytes.
#define LBUS_TXDMA_BURST_SIZE               0x7

//RXDMA Burst Size selection default 7.
//0:16bytes; 1:32bytes; 2:64bytes; 3~7: 128bytes
#define LBUS_RXDMA_BURST_SIZE               0x7
#endif  //IS_EXIST_EMBEDDED

//4  TX/RX BD

//Avoid IO/DMA Racing Method
//  0: check totalpktlen
//  1: check tag
#define RXBD_AVOID_RACING_METHOD    0

// TXBD 32BIT/64BIT System Selection
// 0: 32BIT(8 Bytes each segment)
// 1: 64BIT(16 Bytes each segment)
#define TXBD_SEG_32_64_SEL      0

// TXBD Segment Selection
// 0: 2 Segment 
// 1: 4 Segment
// 2: 8 Segment
#define TX_VOQ_TXBD_MODE_SEL    1
#define TX_VIQ_TXBD_MODE_SEL    1    
#define TX_BEQ_TXBD_MODE_SEL    1        
#define TX_BKQ_TXBD_MODE_SEL    1
#define TX_MGQ_TXBD_MODE_SEL    1        
#define TX_BCNQ_TXBD_MODE_SEL   1
#define TX_HI0Q_TXBD_MODE_SEL   1        
#define TX_HI1Q_TXBD_MODE_SEL   1
#define TX_HI2Q_TXBD_MODE_SEL   1        
#define TX_HI3Q_TXBD_MODE_SEL   1
#define TX_HI4Q_TXBD_MODE_SEL   1        
#define TX_HI5Q_TXBD_MODE_SEL   1
#define TX_HI6Q_TXBD_MODE_SEL   1        
#define TX_HI7Q_TXBD_MODE_SEL   1

// TX_XXQ_TXBD_MODE_SEL: 0 ==> 2
// TX_XXQ_TXBD_MODE_SEL: 1 ==> 4
// TX_XXQ_TXBD_MODE_SEL: 2 ==> 8
#define TXBD_ELE_NUM            4


// Tx
#define TX_VOQ_TXBD_NUM		    NUM_TX_DESC
#define TX_VIQ_TXBD_NUM		    NUM_TX_DESC
#define TX_BEQ_TXBD_NUM		    NUM_TX_DESC
#define TX_BKQ_TXBD_NUM		    NUM_TX_DESC
#define TX_MGQ_TXBD_NUM         NUM_TX_DESC
#define TX_BCNQ_TXBD_NUM_8192E	(1+HAL_NUM_VWLAN)*(TXBD_BEACON_OFFSET_8192E/sizeof(TX_BUFFER_DESCRIPTOR))  // Root + VAP Num
#define TX_BCNQ_TXBD_NUM_8881A	(1+HAL_NUM_VWLAN)*(TXBD_BEACON_OFFSET_8881A/sizeof(TX_BUFFER_DESCRIPTOR))  // Root + VAP Num

#define TX_HI0Q_TXBD_NUM	    NUM_TX_DESC
#define TX_HI1Q_TXBD_NUM	    NUM_TX_DESC
#define TX_HI2Q_TXBD_NUM	    NUM_TX_DESC
#define TX_HI3Q_TXBD_NUM	    NUM_TX_DESC
#define TX_HI4Q_TXBD_NUM	    NUM_TX_DESC
#define TX_HI5Q_TXBD_NUM	    NUM_TX_DESC
#define TX_HI6Q_TXBD_NUM	    NUM_TX_DESC
#define TX_HI7Q_TXBD_NUM	    NUM_TX_DESC

//Get the max value in all queue
#define TX_Q_MAX_TXBD_NUM       NUM_TX_DESC

//Rx
#define RX_Q_RXBD_NUM		    NUM_RX_DESC

//Total
#define TOTAL_NUM_TXBD_NO_BCN   (TX_MGQ_TXBD_NUM + TX_VOQ_TXBD_NUM + TX_VIQ_TXBD_NUM + TX_BEQ_TXBD_NUM + TX_BKQ_TXBD_NUM + \
                                    TX_HI0Q_TXBD_NUM + TX_HI1Q_TXBD_NUM + TX_HI2Q_TXBD_NUM + TX_HI3Q_TXBD_NUM + \
                                    TX_HI4Q_TXBD_NUM + TX_HI5Q_TXBD_NUM + TX_HI6Q_TXBD_NUM + TX_HI7Q_TXBD_NUM)

#define TOTAL_NUM_RXBD          (RX_Q_RXBD_NUM)

// Note: no RX_DESC here.....
#define DESC_DMA_SIZE_NO_BCNQ \
    (TOTAL_NUM_RXBD * sizeof(RX_BUFFER_DESCRIPTOR) + \
     TOTAL_NUM_TXBD_NO_BCN * (sizeof(TX_BUFFER_DESCRIPTOR) + SIZE_TXDESC_88XX))

#define HAL_PAGE_SIZE   PAGE_SIZE

#if IS_RTL8192E_SERIES
#define DESC_DMA_SIZE_8192E (DESC_DMA_SIZE_NO_BCNQ + \
                             (1+HAL_NUM_VWLAN) * TXBD_BEACON_OFFSET_8192E + \
                             (1+HAL_NUM_VWLAN) * SIZE_TXDESC_88XX)

#define DESC_DMA_PAGE_SIZE_8192E    (DESC_DMA_SIZE_8192E + HAL_PAGE_SIZE)
#endif  // IS_RTL8192E_SERIES

#if IS_RTL8881A_SERIES
#define DESC_DMA_SIZE_8881A (DESC_DMA_SIZE_NO_BCNQ + \
                             (1+HAL_NUM_VWLAN) * TXBD_BEACON_OFFSET_8881A + \
                             (1+HAL_NUM_VWLAN) * SIZE_TXDESC_88XX)
    
#define DESC_DMA_PAGE_SIZE_8881A    (DESC_DMA_SIZE_8881A + HAL_PAGE_SIZE)
#endif  // IS_RTL8881A_SERIES

#define DESC_DMA_SIZE_MAX (DESC_DMA_SIZE_NO_BCNQ + \
                             (1+HAL_NUM_VWLAN) * TXBD_BEACON_OFFSET_MAX + \
                             (1+HAL_NUM_VWLAN) * SIZE_TXDESC_88XX)

#define DESC_DMA_PAGE_SIZE_MAX    (DESC_DMA_SIZE_MAX + HAL_PAGE_SIZE)


//Tx Condition Match
#define TX_CONDITION_MATCH_TXBD_CNT   10


//4 TXDESC
#define HAL_TXDESC_OFFSET_SIZE      40

//4 RXDESC

//4 IMEM
#define HAL_IMEM    __IRAM_IN_865X

//4 MIPS16
#define HAL_MIPS16  __MIPS16


#endif  //IS_RTL88XX_GENERATION


//3 Mapping Basic Type 
#define VOID void
#define PVOID void *

#if 0
//typedef unsigned char   		BOOLEAN,*PBOOLEAN;
typedef unsigned char               *PBOOLEAN;
//typedef unsigned char			UCHAR, *PUCHAR;
//typedef unsigned short			USHORT, *PUSHORT;
//typedef short					SHORT;
//typedef unsigned int			ULONG, *PULONG;
//typedef long					LONG;

//typedef long long				LONGLONG;
//typedef unsigned int			UINT;
//typedef unsigned long long		ULONGLONG;
//typedef unsigned long long  	LARGE_INTEGER;

typedef unsigned char			u1Byte,*pu1Byte;
typedef unsigned short			u2Byte,*pu2Byte;
typedef unsigned int			u4Byte,*pu4Byte;
typedef unsigned long long		u8Byte,*pu8Byte;

typedef signed char				s1Byte,*ps1Byte;
typedef signed short			s2Byte,*ps2Byte;
typedef signed int				s4Byte,*ps4Byte;
typedef signed long long		s8Byte,*ps8Byte;
typedef unsigned long long		ULONG64,*PULONG64;

//typedef unsigned char			UINT8;
//typedef unsigned short			UINT16;
//typedef unsigned int			UINT32;
//typedef signed int				INT32;
//typedef signed char				INT8;
//typedef signed int				INT;

typedef const unsigned char	cu8;

typedef __signed char s8;
//typedef unsigned char u8;

typedef __signed short s16;
//typedef unsigned short u16;

typedef __signed int s32;
//typedef unsigned int u32;

typedef __signed__ long s64;
//typedef unsigned long u64;
#endif

//3  Mapping IO
#define HAL_PADAPTER    PRTL8192CD_PRIV
typedef struct stat_info        _HAL_STA_INFO,*P_HAL_STA_INFO;

#define HAL_PSTAINFO    P_HAL_STA_INFO
#define HAL_RTL_R8(reg)		\
    (RTL_R8_F(Adapter, reg))

#define HAL_RTL_R16(reg)	\
    (RTL_R16_F(Adapter, reg))

#define HAL_RTL_R32(reg)	\
    (RTL_R32_F(Adapter, reg))

#define HAL_RTL_W8(reg, val8)	\
    do { \
        RTL_W8_F(Adapter, reg, val8); \
    } while (0)

#define HAL_RTL_W16(reg, val16)	\
    do { \
        RTL_W16_F(Adapter, reg, val16); \
    } while (0)

#define HAL_RTL_W32(reg, val32)	\
    do { \
        RTL_W32_F(Adapter, reg, val32) ; \
    } while (0)


#define PlatformEFIORead1Byte(Adapter,reg)        \
            (RTL_R8_F(Adapter, reg))
#define PlatformEFIORead2Byte(Adapter,reg)        \
            (RTL_R16_F(Adapter, reg))
#define PlatformEFIORead4Byte(Adapter,reg)        \
            (RTL_R32_F(Adapter, reg))   


#define PlatformEFIOWrite1Byte(Adapter,reg,val8)        \
    do { \
        RTL_W8_F(Adapter, reg, val8); \
    } while (0)            
#define PlatformEFIOWrite2Byte(Adapter,reg,val16)        \
    do { \
        RTL_W16_F(Adapter, reg, val16); \
    } while (0)
#define PlatformEFIOWrite4Byte(Adapter,reg,val32)        \
    do { \
        RTL_W32_F(Adapter, reg, val32) ; \
    } while (0)    


//3 Mapping General Function
#define HALMalloc(Adapter, Size)        kmalloc(Size, GFP_ATOMIC)
#define HAL_free(x)                     kfree(x)
#define PlatformZeroMemory(Ptr, Size)   memset(Ptr, 0, Size)
#define HAL_delay_ms(t)                 delay_ms(t)
#define HAL_delay_us(t)                 delay_us(t)
#define HAL_memcpy(dst, src, cnt)       memcpy(dst, src, cnt)
#define HAL_memcmp(src1, src2, size)    memcmp(src1, src2, size)
#define HAL_memset(Ptr, Content, Size)  memset(Ptr, Content, Size)

//LX MIPS MMU
#define HAL_TO_NONCACHE_ADDR(addr)      (addr|0x20000000)
// TODO: Filen, replace HAL_VIRT_TO_BUS
#define HAL_VIRT_TO_BUS(ptr)                                    virt_to_bus(ptr)
#define HAL_VIRT_TO_BUS1(Adapter, ptr, size, direction)         get_physical_addr(Adapter, ptr, size, direction)
//direction
#define HAL_PCI_DMA_TODEVICE    PCI_DMA_TODEVICE

#ifdef CONFIG_NET_PCI
#define HAL_CACHE_SYNC_WBACK(Adapter, start, size, direction)   rtl_cache_sync_wback(Adapter, bus_to_virt(start), size, direction)
#else
#define HAL_CACHE_SYNC_WBACK(Adapter, start, size, direction)   rtl_cache_sync_wback(Adapter, start, size, direction)
#endif

#define HAL_CIRC_CNT_RTK(head,tail,size)	((head>=tail)?(head-tail):(size-tail+head))
#define HAL_CIRC_SPACE_RTK(head,tail,size)  HAL_CIRC_CNT_RTK((tail),((head)+1),(size))


//3 Mapping AP Function
#if (CFG_HAL_SUPPORT_UNIVERSAL_REPEATER) || (CFG_HAL_SUPPORT_MBSSID)
#define HAL_GET_ROOT_PRIV(Adapter)          GET_ROOT_PRIV(Adapter)
#define HAL_IS_ROOT_INTERFACE(Adapter)      IS_ROOT_INTERFACE(Adapter)
#define HAL_GET_ROOT(Adapter)               GET_ROOT(Adapter)								
#else
#define HAL_GET_ROOT(Adapter)               GET_ROOT(Adapter)
#endif

#if CFG_HAL_SUPPORT_UNIVERSAL_REPEATER
#define HAL_GET_VXD_PRIV(Adapter)           GET_VXD_PRIV(Adapter)
#if CFG_HAL_SUPPORT_MBSSID
#define HAL_IS_VXD_INTERFACE(Adapter)       IS_VXD_INTERFACE(Adapter)
#else
#define HAL_IS_VXD_INTERFACE(Adapter)       IS_VXD_INTERFACE(Adapter)
#endif
#endif // CFG_HAL_SUPPORT_UNIVERSAL_REPEATER

#if CFG_HAL_SUPPORT_MBSSID
#define HAL_IS_VAP_INTERFACE(Adapter)       IS_VAP_INTERFACE(Adapter)
#endif

#ifdef CFG_HAL_POWER_PERCENT_ADJUSTMENT
#define  HAL_PwrPercent2PwrLevel(percentage)                PwrPercent2PwrLevel(percentage)
#endif //CFG_HAL_POWER_PERCENT_ADJUSTMENT

#define HAL_ASSIGN_TX_POWER_OFFSET(offset, setting)         ASSIGN_TX_POWER_OFFSET(offset, setting)
#define HAL_POWER_RANGE_CHECK(val)                          POWER_RANGE_CHECK(val)
#define HAL_COUNT_SIGN_OFFSET(base, offset)                 COUNT_SIGN_OFFSET(base, offset)
#define HAL_PHY_SetBBReg(Adpater,RegAddr,BitMask,Data)      PHY_SetBBReg(Adpater,RegAddr,BitMask,Data)
#define HAL_PHY_QueryBBReg(Adpater,RegAddr,BitMask)                 PHY_QueryBBReg(Adpater,RegAddr,BitMask)
#define HAL_PHY_SetRFReg(Adpater,eRFPath,RegAddr,BitMask,Data)      PHY_SetRFReg(Adpater,eRFPath,RegAddr,BitMask,Data)
#define HAL_PHY_QueryRFReg(Adpater,eRFPath,RegAddr,BitMask,dbg_avoid)         PHY_QueryRFReg(Adpater,eRFPath,RegAddr,BitMask,dbg_avoid)


//3 Mapping Protocol Function
#define HAL_IS_MCAST(da)                    IS_MCAST(da)


//3  Mapping Linker Section
#define __HAL_MIPS16__
#define __HAL_FAST__
#define __HAL_MIDIUM__
#define __HAL_LOW__


//3 Mapping Critical Section Protection Method
#define HAL_SAVE_INT_AND_CLI(x)     SAVE_INT_AND_CLI(x)
#define HAL_RESTORE_INT(x)          RESTORE_INT(x)


//3 Mapping Endian Transformer

/*
 *	Call endian free function when
 *		1. Read/write packet content.
 *		2. Before write integer to IO.
 *		3. After read integer from IO.
*/


//#define HAL_cpu_to_le64
//#define HAL_le64_to_cpu
#define HAL_cpu_to_le32     cpu_to_le32
#define HAL_le32_to_cpu     le32_to_cpu
#define HAL_cpu_to_le16     cpu_to_le16
#define HAL_le16_to_cpu     le16_to_cpu
//#define HAL_cpu_to_be64
//#define HAL_be64_to_cpu
#define HAL_cpu_to_be32     cpu_to_be32
#define HAL_be32_to_cpu     be32_to_cpu
#define HAL_cpu_to_be16     cpu_to_be16
#define HAL_be16_to_cpu     be16_to_cpu

//
// Byte Swapping routine.
//
#define EF1Byte	
#define EF2Byte 	le16_to_cpu
#define EF4Byte     le32_to_cpu

//
// Read LE format data from memory
//
#define ReadEF1Byte(_ptr)		EF1Byte(*((u1Byte *)(_ptr)))
#define ReadEF2Byte(_ptr)		EF2Byte(*((u2Byte *)(_ptr)))
#define ReadEF4Byte(_ptr)		EF4Byte(*((u4Byte *)(_ptr)))

//
// Write LE data to memory
//
#define WriteEF1Byte(_ptr, _val)	(*((u1Byte *)(_ptr)))=EF1Byte(_val)
#define WriteEF2Byte(_ptr, _val)	(*((u2Byte *)(_ptr)))=EF2Byte(_val)
#define WriteEF4Byte(_ptr, _val)	(*((u4Byte *)(_ptr)))=EF4Byte(_val)

//3 Mapping OS API
#define HAL_OS_malloc(Adapter, size, flag, could_alloc_from_kerenl) rtl_dev_alloc_skb(Adapter, size, flag, could_alloc_from_kerenl);
#define PHAL_BUF                                                    struct sk_buff *
#define GET_BUF_DATA_PTR(ptr)                                       (ptr->data)


//3 Mapping Debug 
#if     CFG_HAL_DBG
#define DbgPrint    printk
#else
#define DbgPrint
#endif


//3 Mapping Variable
//each adapter
#define HAL_GET_MIB(Adapter)        (Adapter->pmib)
#define HAL_VAR_MANUAL_EDCA         (Adapter->pmib->dot11QosEntry.ManualEDCA)
#define HAL_VAR_MY_HWADDR           ((HAL_GET_MIB(Adapter))->dot11OperationEntry.hwaddr)
#define HAL_OPMODE                  ((HAL_GET_MIB(Adapter))->dot11OperationEntry.opmode)
#define HAL_VAR_BCN_INTERVAL        (Adapter->pmib->dot11StationConfigEntry.dot11BeaconPeriod)
#define HAL_VAR_DTIM_PERIOD         (Adapter->pmib->dot11StationConfigEntry.dot11DTIMPeriod)
#define HAL_VAR_NETWORK_TYPE        (Adapter->pmib->dot11BssType.net_work_type)

#define HAL_VAR_VAP_INIT_SEQ        (Adapter->vap_init_seq)
#define HAL_VAR_VAP_ID              (Adapter->vap_id)
#define HAL_VAR_TX_BEACON_LEN       (Adapter->tx_beacon_len)
#define HAL_VAR_TIM_OFFSET          (Adapter->timoffset)

//Shared, only one
#define HAL_VAR_RETRY_LIMIT_SHORT       (Adapter->pshare->RLShort)
#define HAL_VAR_RETRY_LIMIT_LONG        (Adapter->pshare->RLLong)
#define HAL_VAR_IS_40M_BW               (Adapter->pshare->is_40m_bw)
#define HAL_VAR_IS_40M_BW_BAK           (Adapter->pshare->is_40m_bw_bak)
#define HAL_VAR_OFFSET_2ND_CHANNEL      (Adapter->pshare->offset_2nd_chan)
#define HAL_VAR_MP_SPECIFIC             (Adapter->pshare->rf_ft_var.mp_specific)
#define HAL_VAR_pre_channel             (Adapter->pshare->pre_channel)
#define HAL_VAR_pa_type                 (Adapter->pmib->dot11RFEntry.pa_type)

// phy config
#define HAL_VAR_dot11nUse40M                (Adapter->pmib->dot11nConfigEntry.dot11nUse40M)
#define HAL_VAR_pwrlevelCCK_A(chIdx)        (*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrlevelCCK_A)+chIdx))
#define HAL_VAR_pwrlevelCCK_B(chIdx)        (*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrlevelCCK_B)+chIdx))
#define HAL_VAR_pwrlevelHT40_1S_A(chIdx)    (*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrlevelHT40_1S_A)+chIdx))
#define HAL_VAR_pwrlevelHT40_1S_B(chIdx)    (*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrlevelHT40_1S_B)+chIdx))
#define HAL_VAR_pwrdiffHT40_2S(chIdx)       (*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiffHT40_2S)+chIdx))
#define HAL_VAR_pwrdiffHT20(chIdx)          (*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiffHT20)+chIdx))
#define HAL_VAR_pwrdiffOFDM(chIdx)          (*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiffOFDM)+chIdx))
#define HAL_VAR_pwrlevel5GHT40_1S_A(chIdx)  (*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A)+chIdx))
#define HAL_VAR_pwrlevel5GHT40_1S_B(chIdx)  (*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B)+chIdx))
#define HAL_VAR_pwrdiff5GHT40_2S(chIdx)     (*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff5GHT40_2S)+chIdx))
#define HAL_VAR_pwrdiff5GHT20(chIdx)        (*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff5GHT20)+chIdx))
#define HAL_VAR_pwrdiff5GOFDM(chIdx)        (*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff5GOFDM)+chIdx))
#define HAL_VAR_pwrdiff_20BW1S_OFDM1T_A(chIdx)  	(*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_20BW1S_OFDM1T_A)+chIdx))	
#define HAL_VAR_pwrdiff_40BW2S_20BW2S_A(chIdx)  	(*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_40BW2S_20BW2S_A)+chIdx))	
#define HAL_VAR_pwrdiff_OFDM2T_CCK2T_A(chIdx)       (*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_OFDM2T_CCK2T_A)+chIdx))
#define HAL_VAR_pwrdiff_40BW3S_20BW3S_A(chIdx)  	(*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_40BW3S_20BW3S_A)+chIdx))	
#define HAL_VAR_pwrdiff_4OFDM3T_CCK3T_A(chIdx)  	(*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_OFDM3T_CCK3T_A)+chIdx))
#define HAL_VAR_pwrdiff_40BW4S_20BW4S_A(chIdx)  	(*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_40BW4S_20BW4S_A)+chIdx))	
#define HAL_VAR_pwrdiff_OFDM4T_CCK4T_A(chIdx)       (*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_OFDM4T_CCK4T_A)+chIdx))
#define HAL_VAR_pwrdiff_5G_20BW1S_OFDM1T_A(chIdx)	(*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_5G_20BW1S_OFDM1T_A)+chIdx))
#define HAL_VAR_pwrdiff_5G_40BW2S_20BW2S_A(chIdx)	(*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_5G_40BW2S_20BW2S_A)+chIdx))
#define HAL_VAR_pwrdiff_5G_40BW3S_20BW3S_A(chIdx)	(*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_5G_40BW3S_20BW3S_A)+chIdx))
#define HAL_VAR_pwrdiff_5G_40BW4S_20BW4S_A(chIdx)	(*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_5G_40BW4S_20BW4S_A)+chIdx))
#define HAL_VAR_pwrdiff_5G_RSVD_OFDM4T_A(chIdx)	    (*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_5G_RSVD_OFDM4T_A)+chIdx))	
#define HAL_VAR_pwrdiff_5G_80BW1S_160BW1S_A(chIdx)	(*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_5G_80BW1S_160BW1S_A)+chIdx))	
#define HAL_VAR_pwrdiff_5G_80BW2S_160BW2S_A(chIdx)	(*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_5G_80BW2S_160BW2S_A)+chIdx))	
#define HAL_VAR_pwrdiff_5G_80BW3S_160BW3S_A(chIdx)	(*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_5G_80BW3S_160BW3S_A)+chIdx))	
#define HAL_VAR_pwrdiff_5G_80BW4S_160BW4S_A(chIdx)	(*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_5G_80BW4S_160BW4S_A)+chIdx))	
#define HAL_VAR_pwrdiff_20BW1S_OFDM1T_B(chIdx)  	(*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_20BW1S_OFDM1T_B)+chIdx))	
#define HAL_VAR_pwrdiff_40BW2S_20BW2S_B(chIdx)  	(*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_40BW2S_20BW2S_B)+chIdx))	
#define HAL_VAR_pwrdiff_OFDM2T_CCK2T_B(chIdx)       (*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_OFDM2T_CCK2T_B)+chIdx))
#define HAL_VAR_pwrdiff_40BW3S_20BW3S_B(chIdx)  	(*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_40BW3S_20BW3S_B)+chIdx))	
#define HAL_VAR_pwrdiff_4OFDM3T_CCK3T_B(chIdx)  	(*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_OFDM3T_CCK3T_B)+chIdx))
#define HAL_VAR_pwrdiff_40BW4S_20BW4S_B(chIdx)  	(*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_40BW4S_20BW4S_B)+chIdx))	
#define HAL_VAR_pwrdiff_OFDM4T_CCK4T_B(chIdx)       (*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_OFDM4T_CCK4T_B)+chIdx))
#define HAL_VAR_pwrdiff_5G_20BW1S_OFDM1T_B(chIdx)	(*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_5G_20BW1S_OFDM1T_B)+chIdx))
#define HAL_VAR_pwrdiff_5G_40BW2S_20BW2S_B(chIdx)	(*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_5G_40BW2S_20BW2S_B)+chIdx))
#define HAL_VAR_pwrdiff_5G_40BW3S_20BW3S_B(chIdx)	(*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_5G_40BW3S_20BW3S_B)+chIdx))
#define HAL_VAR_pwrdiff_5G_40BW4S_20BW4S_B(chIdx)	(*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_5G_40BW4S_20BW4S_B)+chIdx))
#define HAL_VAR_pwrdiff_5G_RSVD_OFDM4T_B(chIdx)	    (*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_5G_RSVD_OFDM4T_B)+chIdx))	
#define HAL_VAR_pwrdiff_5G_80BW1S_160BW1S_B(chIdx)	(*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_5G_80BW1S_160BW1S_B)+chIdx))	
#define HAL_VAR_pwrdiff_5G_80BW2S_160BW2S_B(chIdx)	(*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_5G_80BW2S_160BW2S_B)+chIdx))	
#define HAL_VAR_pwrdiff_5G_80BW3S_160BW3S_B(chIdx)	(*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_5G_80BW3S_160BW3S_B)+chIdx))	
#define HAL_VAR_pwrdiff_5G_80BW4S_160BW4S_B(chIdx)	(*(((u1Byte *)Adapter->pmib->dot11RFEntry.pwrdiff_5G_80BW4S_160BW4S_B)+chIdx))	

// 11n OFDM setting
#define HAL_VAR_power_percent               (Adapter->pmib->dot11RFEntry.power_percent)
#define HAL_VAR_use_ext_pa                  (Adapter->pshare->rf_ft_var.use_ext_pa)
#define HAL_VAR_CurrentChannelBW            (Adapter->pshare->CurrentChannelBW)
#define HAL_VAR_txPowerPlus_ofdm_6          (Adapter->pshare->rf_ft_var.txPowerPlus_ofdm_6)
#define HAL_VAR_txPowerPlus_ofdm_9          (Adapter->pshare->rf_ft_var.txPowerPlus_ofdm_9)
#define HAL_VAR_txPowerPlus_ofdm_12         (Adapter->pshare->rf_ft_var.txPowerPlus_ofdm_12)
#define HAL_VAR_txPowerPlus_ofdm_18         (Adapter->pshare->rf_ft_var.txPowerPlus_ofdm_18)
#define HAL_VAR_txPowerPlus_ofdm_24         (Adapter->pshare->rf_ft_var.txPowerPlus_ofdm_24)
#define HAL_VAR_txPowerPlus_ofdm_36         (Adapter->pshare->rf_ft_var.txPowerPlus_ofdm_36)
#define HAL_VAR_txPowerPlus_ofdm_48         (Adapter->pshare->rf_ft_var.txPowerPlus_ofdm_48)
#define HAL_VAR_txPowerPlus_ofdm_54         (Adapter->pshare->rf_ft_var.txPowerPlus_ofdm_54)
#define HAL_VAR_txPowerPlus_mcs_0           (Adapter->pshare->rf_ft_var.txPowerPlus_mcs_0)
#define HAL_VAR_txPowerPlus_mcs_1           (Adapter->pshare->rf_ft_var.txPowerPlus_mcs_1)
#define HAL_VAR_txPowerPlus_mcs_2           (Adapter->pshare->rf_ft_var.txPowerPlus_mcs_2)
#define HAL_VAR_txPowerPlus_mcs_3           (Adapter->pshare->rf_ft_var.txPowerPlus_mcs_3)
#define HAL_VAR_txPowerPlus_mcs_4           (Adapter->pshare->rf_ft_var.txPowerPlus_mcs_4)
#define HAL_VAR_txPowerPlus_mcs_5           (Adapter->pshare->rf_ft_var.txPowerPlus_mcs_5)
#define HAL_VAR_txPowerPlus_mcs_6           (Adapter->pshare->rf_ft_var.txPowerPlus_mcs_6)
#define HAL_VAR_txPowerPlus_mcs_7           (Adapter->pshare->rf_ft_var.txPowerPlus_mcs_7)
#define HAL_VAR_OFDMTxAgcOffset_A(idx)      (*(((u1Byte *)Adapter->pshare->phw->OFDMTxAgcOffset_A)+idx))
#define HAL_VAR_OFDMTxAgcOffset_B(idx)      (*(((u1Byte *)Adapter->pshare->phw->OFDMTxAgcOffset_B)+idx))
#define HAL_VAR_MCSTxAgcOffset_A(idx)       (*(((u1Byte *)Adapter->pshare->phw->MCSTxAgcOffset_A)+idx))
#define HAL_VAR_MCSTxAgcOffset_B(idx)       (*(((u1Byte *)Adapter->pshare->phw->MCSTxAgcOffset_B)+idx))

// 11n CCK setting 
#define HAL_cck_pwr_max                     (Adapter->pshare->rf_ft_var.cck_pwr_max)
#define HAL_txPowerPlus_cck_1               (Adapter->pshare->rf_ft_var.txPowerPlus_cck_1)
#define HAL_txPowerPlus_cck_2               (Adapter->pshare->rf_ft_var.txPowerPlus_cck_2)
#define HAL_txPowerPlus_cck_5               (Adapter->pshare->rf_ft_var.txPowerPlus_cck_5)
#define HAL_txPowerPlus_cck_11              (Adapter->pshare->rf_ft_var.txPowerPlus_cck_11)
#define HAL_CCKTxAgc_A(idx)                 (*(((u1Byte *)Adapter->pshare->phw->CCKTxAgc_A)+idx))
#define HAL_CCKTxAgc_B(idx)                 (*(((u1Byte *)Adapter->pshare->phw->CCKTxAgc_B)+idx))



#if     IS_RTL88XX_GENERATION
u4Byte
MappingTxQueue88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          TxQNum      //enum _TX_QUEUE_
);

void
MappingVariable88XX(
    IN  HAL_PADAPTER    Adapter
);

#endif  //IS_RTL88XX_GENERATION


#endif  //#ifndef __HALCFG_H__


