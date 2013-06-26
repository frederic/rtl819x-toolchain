/*
* Copyright c                  Realtek Semiconductor Corporation, 2009
* All rights reserved.
*
* Program : Switch table Layer2 switch driver,following features are included:
*	PHY/MII/Port/STP/QOS
* Abstract :
* Author : hyking (hyking_liu@realsil.com.cn)
*/
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
//#include "assert.h"
#include "rtl865x_asicBasic.h"
#include "rtl865x_asicCom.h"
#include "rtl865x_asicL2.h"
#include "asicRegs.h"
//#include "rtl_utils.h"
#include "rtl865x_hwPatch.h"

#include <linux/delay.h>

static uint8 fidHashTable[]={0x00,0x0f,0xf0,0xff};

__DRAM_FWD int32		rtl865x_wanPortMask;
int32		rtl865x_lanPortMask = RTL865X_PORTMASK_UNASIGNED;

int32		rtl865x_maxPreAllocRxSkb = RTL865X_PREALLOC_SKB_UNASIGNED;
int32		rtl865x_rxSkbPktHdrDescNum = RTL865X_PREALLOC_SKB_UNASIGNED;
int32		rtl865x_txSkbPktHdrDescNum = RTL865X_PREALLOC_SKB_UNASIGNED;
int32 	miiPhyAddress;
rtl8651_tblAsic_ethernet_t 	rtl8651AsicEthernetTable[9];//RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum

#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
static uint32 _rtl865xC_QM_orgDescUsage = 0;	/* Original Descriptor Usage in HW */
#endif

/* For Bandwidth control - RTL865xB Backward compatible only */
#define _RTL865XB_BANDWIDTHCTRL_X1			(1 << 0)
#define _RTL865XB_BANDWIDTHCTRL_X4			(1 << 1)
#define _RTL865XB_BANDWIDTHCTRL_X8			(1 << 2)
#define _RTL865XB_BANDWIDTHCTRL_CFGTYPE		2		/* Ingress (0) and Egress (1) : 2 types of configuration */
static int32 _rtl865xB_BandwidthCtrlMultiplier = _RTL865XB_BANDWIDTHCTRL_X1;
static uint32 _rtl865xB_BandwidthCtrlPerPortConfiguration[RTL8651_PORT_NUMBER][_RTL865XB_BANDWIDTHCTRL_CFGTYPE /* Ingress (0), Egress (1) */ ];
static uint32 _rtl865xC_BandwidthCtrlNum[] = {	0,	/* BW_FULL_RATE */
														131072,	/* BW_128K */
														262144,	/* BW_256K */
														524288,	/* BW_512K */
														1048576,	/* BW_1M */
														2097152,	/* BW_2M */
														4194304,	/* BW_4M */
														8388608	/* BW_8M */
														};

#define	RTL865XC_INGRESS_16KUNIT	16384
#define	RTL865XC_EGRESS_64KUNIT	65535

#ifdef CONFIG_RTL8196C_ETH_IOT
extern uint32 port_link_sts, port_linkpartner_eee;
#endif

#if defined(RTL8196C_EEE_MAC)
int eee_enabled = 1;
void eee_phy_enable(void);
void eee_phy_disable(void);
void eee_phy_enable_by_port(int port);
#elif defined(RTL8198_EEE_MAC)
int eee_enabled = 1;
void eee_phy_enable_98(void);
void eee_phy_disable_98(void);
#else
int eee_enabled = 0;
#endif

#if defined(CONFIG_RTL_8198_NFBI_BOARD)
#define RTL8198_NFBI_PORT5_GMII 1 //mark_nfbi , default port5 set to GMII , you can undef here to set to MII mode!!!
//#undef RTL8198_NFBI_PORT5_GMII  //mark_nfbi , default port5 set to GMII , you can undef here to set to MII mode!!!
#endif


#if defined(CONFIG_RTL_8198)

#define PORT5_RGMII_GMII		1

//static void qos_init(void);
#endif

#define	QNUM_IDX_123		0
#define	QNUM_IDX_45		1
#define	QNUM_IDX_6		2

#if !defined(CONFIG_RTL_819XD	) && !defined(CONFIG_RTL_8196E)
static int32 _rtl865x_setQosThresholdByQueueIdx(uint32 qidx);

#if 0
static rtl865xC_outputQueuePara_t	outputQueuePara[3] = {
										{
											1, 		/* default: Bandwidth Control Include/exclude Preamble & IFG */
											20, 		/* default: Per Queue Physical Length Gap = 20 */
											504, 		/* default: Descriptor Run Out Threshold = 504 */
											180, 		/*default: System shared buffer flow control turn off threshold = 212 */
											196,		/*default: System shared buffer flow control turn on threshold = 248 */
											500, 		/*default: system flow control turn off threshold = 500*/
											502,		/*default: system flow control turn on threshold = 502*/
											330, 		/*default: port base flow control turn off threshold = 0xf8*/
											400,		/*default: port base flow control turn on threshold = 0x108*/
											31, 		/* Queue-Descriptor=Based Flow Control turn off Threshold =0x14 */
											48, 		/* Queue-Descriptor=Based Flow Control turn on Threshold = 0x21 */
											0x03, 	/* Queue-Packet=Based Flow Control turn off Threshold = 0x03 */
											0x05	/* Queue-Packet=Based Flow Control turn on Threshold =0x05 */
										},
										{
											1, 		/* default: Bandwidth Control Include/exclude Preamble & IFG */
											20, 		/* default: Per Queue Physical Length Gap = 20 */
											504, 		/* default: Descriptor Run Out Threshold = 504 */
											120, 		/*default: System shared buffer flow control turn off threshold = 212 */
											136,		/*default: System shared buffer flow control turn on threshold = 248 */
											330, 		/*default: system flow control turn off threshold = 500*/
											344,		/*default: system flow control turn on threshold = 502*/
											248, 		/*default: port base flow control turn off threshold = 0xf8*/
											264,		/*default: port base flow control turn on threshold = 0x108*/
											20, 		/* Queue-Descriptor=Based Flow Control turn off Threshold =0x14 */
											33, 		/* Queue-Descriptor=Based Flow Control turn on Threshold = 0x21 */
											0x03, 	/* Queue-Packet=Based Flow Control turn off Threshold = 0x03 */
											0x05	/* Queue-Packet=Based Flow Control turn on Threshold =0x05 */
										},
										{
											1, 		/* default: Bandwidth Control Include/exclude Preamble & IFG */
											20, 		/* default: Per Queue Physical Length Gap = 20 */
											500, 		/* default: Descriptor Run Out Threshold = 504 */
											324, 		/*default: System shared buffer flow control turn off threshold = 212 */
											340,		/*default: System shared buffer flow control turn on threshold = 248 */
											330, 		/*default: system flow control turn off threshold = 500*/
											400,		/*default: system flow control turn on threshold = 502*/
											240, 		/*default: port base flow control turn off threshold = 0xf8*/
											282,		/*default: port base flow control turn on threshold = 0x108*/
											20, 		/* Queue-Descriptor=Based Flow Control turn off Threshold =0x14 */
											28, 		/* Queue-Descriptor=Based Flow Control turn on Threshold = 0x21 */
											10, 	/* Queue-Packet=Based Flow Control turn off Threshold = 0x03 */
											11	/* Queue-Packet=Based Flow Control turn on Threshold =0x05 */
										}
										};
#else
// sync from kernel 2.4 qos_init() -- 20110330
static rtl865xC_outputQueuePara_t	outputQueuePara[3] = {
										{
											1, 		/* default: Bandwidth Control Include/exclude Preamble & IFG */
											0x48, 		/* default: Per Queue Physical Length Gap = 20 */
											500, 		/* default: Descriptor Run Out Threshold = 504 */
											252, 		/*default: System shared buffer flow control turn off threshold = 212 */
											310,		/*default: System shared buffer flow control turn on threshold = 248 */
											280, 		/*default: system flow control turn off threshold = 500*/
											400,		/*default: system flow control turn on threshold = 502*/
											258, 		/*default: port base flow control turn off threshold = 0xf8*/
											320,		/*default: port base flow control turn on threshold = 0x108*/
											16, 		/* Queue-Descriptor=Based Flow Control turn off Threshold =0x14 */
											21, 		/* Queue-Descriptor=Based Flow Control turn on Threshold = 0x21 */
											5, 	/* Queue-Packet=Based Flow Control turn off Threshold = 0x03 */
											6	/* Queue-Packet=Based Flow Control turn on Threshold =0x05 */
										},
										{
											1, 		/* default: Bandwidth Control Include/exclude Preamble & IFG */
											0x48, 		/* default: Per Queue Physical Length Gap = 20 */
											500, 		/* default: Descriptor Run Out Threshold = 504 */
											252, 		/*default: System shared buffer flow control turn off threshold = 212 */
											310,		/*default: System shared buffer flow control turn on threshold = 248 */
											280, 		/*default: system flow control turn off threshold = 500*/
											400,		/*default: system flow control turn on threshold = 502*/
											258, 		/*default: port base flow control turn off threshold = 0xf8*/
											320,		/*default: port base flow control turn on threshold = 0x108*/
											16, 		/* Queue-Descriptor=Based Flow Control turn off Threshold =0x14 */
											21, 		/* Queue-Descriptor=Based Flow Control turn on Threshold = 0x21 */
											5, 	/* Queue-Packet=Based Flow Control turn off Threshold = 0x03 */
											6	/* Queue-Packet=Based Flow Control turn on Threshold =0x05 */
										},
										{
											1, 		/* default: Bandwidth Control Include/exclude Preamble & IFG */
											0x48, 		/* default: Per Queue Physical Length Gap = 20 */
											500, 		/* default: Descriptor Run Out Threshold = 504 */
											252, 		/*default: System shared buffer flow control turn off threshold = 212 */
											310,		/*default: System shared buffer flow control turn on threshold = 248 */
											280, 		/*default: system flow control turn off threshold = 500*/
											400,		/*default: system flow control turn on threshold = 502*/
											258, 		/*default: port base flow control turn off threshold = 0xf8*/
											320,		/*default: port base flow control turn on threshold = 0x108*/
											16, 		/* Queue-Descriptor=Based Flow Control turn off Threshold =0x14 */
											21, 		/* Queue-Descriptor=Based Flow Control turn on Threshold = 0x21 */
											5, 	/* Queue-Packet=Based Flow Control turn off Threshold = 0x03 */
											6	/* Queue-Packet=Based Flow Control turn on Threshold =0x05 */
										}
										};
#endif
#endif

static void _rtl8651_syncToAsicEthernetBandwidthControl(void);
#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
static int32 _rtl865xC_QM_init( void );
#endif

#if defined(RTL8196C_EEE_MAC)
// EEE PHY -- Page 4
// register 16
#define P4R16_eee_10_cap                           (1 << 13)	// enable EEE 10M
#define P4R16_eee_nway_en                           (1 << 12)	// enable Next Page exchange in nway for EEE 100M
#define P4R16_tx_quiet_en                            (1 << 9)	// enable ability to turn off pow100tx when TX Quiet state
#define P4R16_rx_quiet_en                            (1 << 8)	// enable ability to turn off pow100rx when RX Quiet state

// register 25
#define P4R25_rg_dacquiet_en                            (1 << 10)	// enable ability to turn off DAC when TX Quiet state
#define P4R25_rg_ldvquiet_en                            (1 << 9)		// enable ability to turn off line driver when TX Quiet state
#define P4R25_rg_eeeprg_rst                            (1 << 6)		// reset for EEE programmable finite state machine
#define P4R25_rg_ckrsel                            		(1 << 5)		// select ckr125 as RX 125MHz clock
#define P4R25_rg_eeeprg_en                            (1 << 4)		// enable EEE programmable finite state machine

static const unsigned short phy_data[]={
	0x5000,  // write, address 0
	0x6000,  // write, address 1
	0x7000,  // write, address 2
	0x4000,  // write, address 3
	0xD36C,  // write, address 4
	0xFFFF,  // write, address 5
	0x5060,  // write, address 6
	0x61C5,  // write, address 7
	0x7000,  // write, address 8
	0x4001,  // write, address 9
	0x5061,  // write, address 10
	0x87F5,  // write, address 11
	0xCE60,  // write, address 12
	0x0026,  // write, address 13
	0x8E03,  // write, address 14
	0xA021,  // write, address 15
	0x300B,  // write, address 16
	0x58A0,  // write, address 17
	0x629C,  // write, address 18
	0x7000,  // write, address 19
	0x4002,  // write, address 20
	0x58A1,  // write, address 21
	0x87EA,  // write, address 22
	0xAE25,  // write, address 23
	0xA018,  // write, address 24
	0x3016,  // write, address 25
	0x6894,  // write, address 26
	0x6094,  // write, address 27
	0x5123,  // write, address 28
	0x63C2,  // write, address 29
	0x5127,  // write, address 30
	0x4003,  // write, address 31
	0x87E0,  // write, address 32
	0x8EF3,  // write, address 33
	0xA10E,  // write, address 34
	0xCC40,  // write, address 35
	0x0007,  // write, address 36
	0xCA40,  // write, address 37
	0xFFE0,  // write, address 38
	0xA202,  // write, address 39
	0x3020,  // write, address 40
	0x7008,  // write, address 41
	0x3020,  // write, address 42
	0xCC44,  // write, address 43
	0xFFF4,  // write, address 44
	0xCC44,  // write, address 45
	0xFFF2,  // write, address 46
	0x3000,  // write, address 47
	0x5220,  // write, address 48
	0x4004,  // write, address 49
	0x3000,  // write, address 50
	0x64A0,  // write, address 51
	0x5429,  // write, address 52
	0x4005,  // write, address 53
	0x87CA,  // write, address 54
	0xCE18,  // write, address 55
	0xFFC8,  // write, address 56
	0xCE64,  // write, address 57
	0xFFD0,  // write, address 58
	0x3036,  // write, address 59
	0x65C0,  // write, address 60
	0x50A9,  // write, address 61
	0x4006,  // write, address 62
	0xA3DB,  // write, address 63
	0x303F,  // write, address 64
};

static int ram_code_done=0;

void set_ram_code(void)
{
	uint32 reg;
	int i, len=sizeof(phy_data)/sizeof(unsigned short);

	if (ram_code_done)
		return;

	rtl8651_getAsicEthernetPHYReg( 4, 0x19, &reg );

	// turn on rg_eeeprg_rst
	rtl8651_setAsicEthernetPHYReg(4, 0x19, ((reg & ~(P4R25_rg_eeeprg_en)) | P4R25_rg_eeeprg_rst));

	// turn on mem_mdio_mode
	rtl8651_setAsicEthernetPHYReg(4, 0x1c, 0x0180);

	// begin to write all RAM
	for(i=0;i<len;i++) {
		rtl8651_setAsicEthernetPHYReg(4, 0x1d, phy_data[i]);
	}

	for(i=0;i<63;i++) {
		rtl8651_setAsicEthernetPHYReg(4, 0x1d, 0);
	}

	// finish reading all RAM
	// turn off mem_mdio_mode
	rtl8651_setAsicEthernetPHYReg(4, 0x1c, 0x0080);

	// turn off rg_eeeprg_rst, enable EEE programmable finite state machine
	rtl8651_setAsicEthernetPHYReg(4, 0x19, ((reg & ~(P4R25_rg_eeeprg_rst)) | P4R25_rg_eeeprg_en));

	ram_code_done = 1;
}

static const unsigned short phy_data_b[]={
  0x5000,  // write, address 0
  0x6000,  // write, address 1
  0x7000,  // write, address 2
  0x4000,  // write, address 3
  0x8700,  // write, address 4
  0xD344,  // write, address 5
  0xFFFF,  // write, address 6
  0xCA6C,  // write, address 7
  0xFFFD,  // write, address 8
  0x5460,  // write, address 9
  0x61C5,  // write, address 10
  0x7000,  // write, address 11
  0x4001,  // write, address 12
  0x5461,  // write, address 13
  0x4001,  // write, address 14
  0x87F1,  // write, address 15
  0xCE60,  // write, address 16
  0x0026,  // write, address 17
  0x8E03,  // write, address 18
  0xA021,  // write, address 19
  0x300F,  // write, address 20
  0x5CA0,  // write, address 21
  0x629C,  // write, address 22
  0x7000,  // write, address 23
  0x4002,  // write, address 24
  0x5CA1,  // write, address 25
  0x87E6,  // write, address 26
  0xAE25,  // write, address 27
  0xA018,  // write, address 28
  0x301A,  // write, address 29
  0x6E94,  // write, address 30
  0x6694,  // write, address 31
  0x5523,  // write, address 32
  0x63C2,  // write, address 33
  0x5527,  // write, address 34
  0x4003,  // write, address 35
  0x87DC,  // write, address 36
  0x8EF3,  // write, address 37
  0xA10E,  // write, address 38
  0xCC40,  // write, address 39
  0x0007,  // write, address 40
  0xCA40,  // write, address 41
  0xFFDF,  // write, address 42
  0xA202,  // write, address 43
  0x3024,  // write, address 44
  0x7008,  // write, address 45
  0x3024,  // write, address 46
  0xCC44,  // write, address 47
  0xFFF4,  // write, address 48
  0xCC44,  // write, address 49
  0xFFF2,  // write, address 50
  0x3000,  // write, address 51
  0x5620,  // write, address 52
  0x4004,  // write, address 53
  0x3000,  // write, address 54
  0x64A0,  // write, address 55
  0x5429,  // write, address 56
  0x4005,  // write, address 57
  0x87C6,  // write, address 58
  0xCE18,  // write, address 59
  0xFFC4,  // write, address 60
  0xCE64,  // write, address 61
  0xFFCF,  // write, address 62
  0x303A,  // write, address 63
  0x65C0,  // write, address 64
  0x54A9,  // write, address 65
  0x4006,  // write, address 66
  0xA3DB,  // write, address 67
  0x3043,  // write, address 68
};

void set_ram_code_b(void)
{
	uint32 reg;
	int i, len=sizeof(phy_data_b)/sizeof(unsigned short);

	if (ram_code_done)
		return;

	rtl8651_getAsicEthernetPHYReg(4, 0x19, &reg );
	rtl8651_setAsicEthernetPHYReg(4, 0x19, ((reg & ~(P4R25_rg_eeeprg_en)) | P4R25_rg_eeeprg_rst));
	rtl8651_setAsicEthernetPHYReg(4, 0x1c, 0x0180);

	for(i=0;i<len;i++) {
		rtl8651_setAsicEthernetPHYReg(4, 0x1d, phy_data_b[i]);
	}

	rtl8651_setAsicEthernetPHYReg(4, 0x1c, 0x0080);
	rtl8651_setAsicEthernetPHYReg(4, 0x19, ((reg & ~(P4R25_rg_eeeprg_rst)) | P4R25_rg_eeeprg_en));

	ram_code_done = 1;
}

void eee_phy_enable_by_port(int port)
{
	uint32 reg;

	// change to page 4
	rtl8651_setAsicEthernetPHYReg(port, 31, 4);

	// enable EEE N-way & set turn off power at quiet state
	rtl8651_getAsicEthernetPHYReg( port, 16, &reg );
	reg |= (P4R16_eee_nway_en | P4R16_tx_quiet_en | P4R16_rx_quiet_en);

#ifdef CONFIG_RTL8196C_ETH_IOT
	reg |= P4R16_eee_10_cap;	// enable 10M_EEE also.
#endif
	rtl8651_setAsicEthernetPHYReg( port, 16, reg );

	// enable EEE turn off DAC and line driver when TX Quiet state
	rtl8651_getAsicEthernetPHYReg( port, 25, &reg );
//	reg = reg & 0xF9FF | P4R25_rg_dacquiet_en | P4R25_rg_ldvquiet_en;
	reg |= (P4R25_rg_dacquiet_en | P4R25_rg_ldvquiet_en | P4R25_rg_eeeprg_en);

	rtl8651_setAsicEthernetPHYReg( port, 25, reg );

	rtl8651_setAsicEthernetPHYReg( port, 17, 0xa2a2 );
	rtl8651_setAsicEthernetPHYReg( port, 19, 0xc5c2 );
	rtl8651_setAsicEthernetPHYReg( port, 24, 0xc0f3 );

	if ((REG32(REVR)==RTL8196C_REVISION_A) && (port==4)){
		set_ram_code();
	}
	else if ((REG32(REVR) == RTL8196C_REVISION_B) && (port == 4)) {
		set_ram_code_b();
	}

	// switch to page 0
	rtl8651_setAsicEthernetPHYReg(port, 31, 0 );
}

void eee_phy_enable(void)
{
	int i;
	uint32 reg;

	// EEE PHY enable
	for (i=0; i<MAX_PORT_NUMBER; i++)
	{
		// change to page 4
		rtl8651_setAsicEthernetPHYReg(i, 31, 4);

		// enable EEE N-way & set turn off power at quiet state
		rtl8651_getAsicEthernetPHYReg( i, 16, &reg );
		reg |= (P4R16_eee_nway_en | P4R16_tx_quiet_en | P4R16_rx_quiet_en);
		rtl8651_setAsicEthernetPHYReg( i, 16, reg );

		// enable EEE turn off DAC and line driver when TX Quiet state
		rtl8651_getAsicEthernetPHYReg( i, 25, &reg );
//		reg = reg & 0xF9FF | P4R25_rg_dacquiet_en | P4R25_rg_ldvquiet_en;
		reg |= (P4R25_rg_dacquiet_en | P4R25_rg_ldvquiet_en | P4R25_rg_eeeprg_en);
		rtl8651_setAsicEthernetPHYReg( i, 25, reg );

		rtl8651_setAsicEthernetPHYReg( i, 17, 0xa2a2 );
		rtl8651_setAsicEthernetPHYReg( i, 19, 0xc5c2 );
		rtl8651_setAsicEthernetPHYReg( i, 24, 0xc0f3 );

		if ((REG32(REVR)==RTL8196C_REVISION_A) && (i==4)){
			set_ram_code();
		}
		else if ((REG32(REVR) == RTL8196C_REVISION_B) && (i == 4)) {
			set_ram_code_b();
		}

		// switch to page 0
		rtl8651_setAsicEthernetPHYReg(i, 31, 0 );

		rtl8651_restartAsicEthernetPHYNway(i+1);
	}
}

void eee_phy_disable(void)
{
	int i;
	uint32 reg;

	// EEE PHY disable
	for (i=0; i<MAX_PORT_NUMBER; i++)
	{
		// change to page 4
		rtl8651_setAsicEthernetPHYReg(i, 31, 4);

		// disable (EEE N-way & turn off power at quiet state)
		rtl8651_getAsicEthernetPHYReg( i, 16, &reg );
		reg = reg & 0xECFF;
		rtl8651_setAsicEthernetPHYReg( i, 16, reg );

		// switch to page 0
		rtl8651_setAsicEthernetPHYReg(i, 31, 0 );

		rtl8651_restartAsicEthernetPHYNway(i+1);
	}

	// EEE MAC disable

}
#endif

#if defined(RTL8198_EEE_MAC)
// EEE PHY -- Page 4
void Set_GPHYWB(unsigned int phyid, unsigned int page, unsigned int reg, unsigned int mask, unsigned int val);

void eee_phy_enable_98(void)
{
	int i;

	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);

	// EEE PHY enable
	for (i=0; i<5; i++)
	{
		// change to page 4
		Set_GPHYWB(i,32,27,0xffff - 0xffff,0x2fce);
		Set_GPHYWB(i,32,21,0xffff - 0xffff,0x0100);
		Set_GPHYWB(i,5,5,0xffff - 0xffff,0x8b84);
		Set_GPHYWB(i,5,6,0xffff - 0xffff,0x0062);

		/* enable "EEE auto off" for JMicron's bug */
		Set_GPHYWB(i,5,5,0,0x857a);
		Set_GPHYWB(i,5,6,0,0x0770);

		rtl8651_restartAsicEthernetPHYNway(i+1);
	}

//	for(i=0; i<5; i++)
//		REG32(PCRP0+i*4) &= ~(EnForceMode);
}

void eee_phy_disable_98(void)
{
	int i;

	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);

	for (i=0; i<5; i++)
	{
		// change to page 4
		Set_GPHYWB(i,32,27,0xffff - 0xffff,0x2f4e);
		Set_GPHYWB(i,32,21,0xffff - 0xffff,0x0);
		Set_GPHYWB(i,5,5,0xffff - 0xffff,0x8b84);
		Set_GPHYWB(i,5,6,0xffff - 0xffff,0x0042);

		rtl8651_restartAsicEthernetPHYNway(i+1);
	}
	// EEE PHY disable

//	for(i=0; i<5; i++)
//		REG32(PCRP0+i*4) &= ~(EnForceMode);
}
#endif

#if defined(CONFIG_RTL_8198)
int rtl8198_power_saving_config(uint32 mode)
{
	unsigned long flags;
	int i, _8198_ALDPS, _8198_green_eth;

	if(mode == 0)
	{
		// 8198 green ethernet / EEE / ALDPS off
		eee_enabled = 0;
		_8198_green_eth = 0;
		_8198_ALDPS = 0;
	}
	else if(mode == 1)
	{
		// 8198 green ethernet / EEE / ALDPS on
		eee_enabled = 1;
		_8198_green_eth = 1;
		_8198_ALDPS = 1;
	}
	else if(mode == 2)
	{
		// 8198 green ethernet on / EEE off
		eee_enabled = 0;
		_8198_green_eth = 1;
		_8198_ALDPS = 1;
	}
	else if(mode == 3)
	{
		// 8198 green ethernet off / EEE on
		eee_enabled = 1;
		_8198_green_eth = 0;
		_8198_ALDPS = 1;
	}
	else {
		return (-1);
	}

	local_irq_save(flags);

	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);

	/*
		8198 ALDPS feature is on by default.
		It is in PHY page 44, reg 21, bit 0: 1 enable, 0 disable
	*/
	if (_8198_ALDPS) {
		Set_GPHYWB(999, 44, 21, 0xfffe, 1);
	}
	else {
		Set_GPHYWB(999, 44, 21, 0xfffe, 0);
	}

	/*
		write Page 5 Reg 5 = 0x85E4
		read Page 5 Reg 6; #bit0 = 1, for enable green Rx
		write Page 5 Reg 5 = 0x85E7
		read Page 5 Reg 6; #bit0 = 1, for enable green Tx

	*/
	if (_8198_green_eth) {
		Set_GPHYWB(999, 5, 5, 0, 0x85e4);
		Set_GPHYWB(999, 5, 6, 0xfffe, 1);

		Set_GPHYWB(999, 5, 5, 0, 0x85e7);
		Set_GPHYWB(999, 5, 6, 0xfffe, 1);
	}
	else {
		Set_GPHYWB(999, 5, 5, 0, 0x85e4);
		Set_GPHYWB(999, 5, 6, 0xfffe, 0);

		Set_GPHYWB(999, 5, 5, 0, 0x85e7);
		Set_GPHYWB(999, 5, 6, 0xfffe, 0);
	}

	if (eee_enabled) {
		eee_phy_enable_98();
	}
	else {
		eee_phy_disable_98();
	}

	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) &= ~(EnForceMode);

	local_irq_restore(flags);

	return 0;
}
#endif

#ifdef CONFIG_RTL8196C_GREEN_ETHERNET
void set_phy_pwr_save(int id, int val)
{
	uint32 reg_val;
	int i, start, end;

	if (id == 99)
		{ start=0; end=4; }
	else if (id <= 4)
		{ start = end = id; }
	else
		return;
	for(i=start; i<=end; i++)
	{
		rtl8651_getAsicEthernetPHYReg( i, 24, &reg_val);

		if (val == 1)
			rtl8651_setAsicEthernetPHYReg( i, 24, (reg_val | BIT(15)) );
		else
			rtl8651_setAsicEthernetPHYReg( i, 24, (reg_val & (~BIT(15))) );
	}
}
#endif

int32 mmd_read(uint32 phyId, uint32 devId, uint32 regId, uint32 *rData)
{
	rtl8651_setAsicEthernetPHYReg( phyId, 13, devId);
	rtl8651_setAsicEthernetPHYReg( phyId, 14, regId);
	rtl8651_setAsicEthernetPHYReg( phyId, 13, (devId | 0x4000));
	return (rtl8651_getAsicEthernetPHYReg(phyId, 14, rData));
}

int32 mmd_write(uint32 phyId, uint32 devId, uint32 regId, uint32 wData)
{
	rtl8651_setAsicEthernetPHYReg( phyId, 13, devId);
	rtl8651_setAsicEthernetPHYReg( phyId, 14, regId);
	rtl8651_setAsicEthernetPHYReg( phyId, 13, (devId | 0x4000));
	rtl8651_setAsicEthernetPHYReg( phyId, 14, wData);
	return SUCCESS;
}

uint32 rtl8651_filterDbIndex(ether_addr_t * macAddr,uint16 fid)
{
    return ( macAddr->octet[0] ^ macAddr->octet[1] ^
                    macAddr->octet[2] ^ macAddr->octet[3] ^
                    macAddr->octet[4] ^ macAddr->octet[5] ^fidHashTable[fid]) & 0xFF;
}

int32 rtl8651_setAsicL2Table(uint32 row, uint32 column, rtl865x_tblAsicDrv_l2Param_t *l2p)
{
	rtl865xc_tblAsic_l2Table_t entry;

	if((row >= RTL8651_L2TBL_ROW) || (column >= RTL8651_L2TBL_COLUMN) || (l2p == NULL))
		return FAILED;
	if(l2p->macAddr.octet[5] != ((row^(fidHashTable[l2p->fid])^ l2p->macAddr.octet[0] ^ l2p->macAddr.octet[1] ^ l2p->macAddr.octet[2] ^ l2p->macAddr.octet[3] ^ l2p->macAddr.octet[4] ) & 0xff))
		return FAILED;

	memset(&entry, 0,sizeof(entry));
	entry.mac47_40 = l2p->macAddr.octet[0];
	entry.mac39_24 = (l2p->macAddr.octet[1] << 8) | l2p->macAddr.octet[2];
	entry.mac23_8 = (l2p->macAddr.octet[3] << 8) | l2p->macAddr.octet[4];


#if 1 //chhuang: #ifdef CONFIG_RTL8650B
	if( l2p->memberPortMask  > RTL8651_PHYSICALPORTMASK) //this MAC is on extension port
		entry.extMemberPort = (l2p->memberPortMask >>RTL8651_PORT_NUMBER);
#endif /* CONFIG_RTL8650B */

	entry.memberPort = l2p->memberPortMask & RTL8651_PHYSICALPORTMASK;
	entry.toCPU = l2p->cpu==TRUE? 1: 0;
	entry.isStatic = l2p->isStatic==TRUE? 1: 0;
	entry.nxtHostFlag = l2p->nhFlag==TRUE? 1: 0;

	/* RTL865xC: modification of age from ( 2 -> 3 -> 1 -> 0 ) to ( 3 -> 2 -> 1 -> 0 ). modification of granularity 100 sec to 150 sec. */
	entry.agingTime = ( l2p->ageSec > 300 )? 0x03: ( l2p->ageSec <= 300 && l2p->ageSec > 150 )? 0x02: (l2p->ageSec <= 150 && l2p->ageSec > 0 )? 0x01: 0x00;

	entry.srcBlock = (l2p->srcBlk==TRUE)? 1: 0;
	entry.fid=l2p->fid;
	entry.auth=l2p->auth;
	return _rtl8651_forceAddAsicEntry(TYPE_L2_SWITCH_TABLE, row<<2 | column, &entry);
}

int32 rtl8651_delAsicL2Table(uint32 row, uint32 column)
{
	rtl865xc_tblAsic_l2Table_t entry;

	if(row >= RTL8651_L2TBL_ROW || column >= RTL8651_L2TBL_COLUMN)
		return FAILED;

	memset(&entry,0,sizeof(entry));
	return _rtl8651_forceAddAsicEntry(TYPE_L2_SWITCH_TABLE, row<<2 | column, &entry);
}


ether_addr_t cachedDA;
static uint32 cachedMbr;
unsigned int rtl8651_asicL2DAlookup(uint8 *dmac){
	uint32 column;
//	rtl8651_tblAsic_l2Table_t   entry;
	rtl865xc_tblAsic_l2Table_t	entry;

//	unsigned int row = dmac[0]^dmac[1]^dmac[2]^dmac[3]^dmac[4]^dmac[5];
	uint32 row = rtl8651_filterDbIndex((ether_addr_t *)dmac, 0);
	//rtlglue_printf("mac %02x %02x %02x %02x %02x %02x \n",	mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

#if 0 //chhuang:
	//cache hit...
	if(memcmp(&cachedDA, dmac, 6)==0)
		return cachedMbr;
#endif

	//cache miss...
	cachedMbr=0;
	for(column=0;column<RTL8651_L2TBL_COLUMN; column++) {
/* Should be fixed 		WRITE_MEM32(TEACR,READ_MEM32(TEACR)|0x1);ASIC patch: disable L2 Aging while reading L2 table */
		_rtl8651_readAsicEntry(TYPE_L2_SWITCH_TABLE, row<<2 | column, &entry);
/*		WRITE_MEM32(TEACR,READ_MEM32(TEACR)&~0x1); ASIC patch: enable L2 Aging aftrer reading L2 table */
		if(entry.agingTime == 0 && entry.isStatic == 0)
			continue;
		if(	dmac[0]==entry.mac47_40 &&
		    	dmac[1]==(entry.mac39_24>>8) &&
		    	dmac[2]==(entry.mac39_24 & 0xff)&&
		    	dmac[3]==(entry.mac23_8 >> 8)&&
		    	dmac[4]==(entry.mac23_8 & 0xff)&&
			dmac[5]== (row ^dmac[0]^dmac[1]^dmac[2]^dmac[3]^dmac[4])){

			cachedDA=*((ether_addr_t *)dmac);
			cachedMbr =(entry.extMemberPort<<RTL8651_PORT_NUMBER) | entry.memberPort;
			return cachedMbr;
		}
	}
	if(column==RTL8651_L2TBL_COLUMN)
		return 0xffffffff;//can't find this MAC, broadcast it.
	return cachedMbr;
}



int32 rtl8651_getAsicL2Table(uint32 row, uint32 column, rtl865x_tblAsicDrv_l2Param_t *l2p) {
	rtl865xc_tblAsic_l2Table_t   entry;

	if((row >= RTL8651_L2TBL_ROW) || (column >= RTL8651_L2TBL_COLUMN) || (l2p == NULL))
		return FAILED;

/*	RTL865XC should fix this problem.WRITE_MEM32(TEACR,READ_MEM32(TEACR)|0x1); ASIC patch: disable L2 Aging while reading L2 table */
	_rtl8651_readAsicEntry(TYPE_L2_SWITCH_TABLE, row<<2 | column, &entry);
	//WRITE_MEM32(TEACR,READ_MEM32(TEACR)&0x1); ASIC patch: enable L2 Aging aftrer reading L2 table */

	if(entry.agingTime == 0 && entry.isStatic == 0 )
		return FAILED;
	l2p->macAddr.octet[0] = entry.mac47_40;
	l2p->macAddr.octet[1] = entry.mac39_24 >> 8;
	l2p->macAddr.octet[2] = entry.mac39_24 & 0xff;
	l2p->macAddr.octet[3] = entry.mac23_8 >> 8;
	l2p->macAddr.octet[4] = entry.mac23_8 & 0xff;
	l2p->macAddr.octet[5] = row ^ l2p->macAddr.octet[0] ^ l2p->macAddr.octet[1] ^ l2p->macAddr.octet[2] ^ l2p->macAddr.octet[3] ^ l2p->macAddr.octet[4]  ^(fidHashTable[entry.fid]);
	l2p->cpu = entry.toCPU==1? TRUE: FALSE;
	l2p->srcBlk = entry.srcBlock==1? TRUE: FALSE;
	l2p->nhFlag = entry.nxtHostFlag==1? TRUE: FALSE;
	l2p->isStatic = entry.isStatic==1? TRUE: FALSE;
#if 1 //chhuang: #ifdef CONFIG_RTL8650B
	l2p->memberPortMask = (entry.extMemberPort<<RTL8651_PORT_NUMBER) | entry.memberPort;
#else
	l2p->memberPortMask = entry.memberPort;
#endif /* CONFIG_RTL8650B */

	/* RTL865xC: modification of age from ( 2 -> 3 -> 1 -> 0 ) to ( 3 -> 2 -> 1 -> 0 ). modification of granularity 100 sec to 150 sec. */
	l2p->ageSec = entry.agingTime * 150;

	l2p->fid=entry.fid;
	l2p->auth=entry.auth;
	return SUCCESS;
}

int32 rtl8651_setAsicPortMirror(uint32 mRxMask, uint32 mTxMask,uint32 mPortMask)
{
	uint32 pmcr = 0;
	pmcr = ((mTxMask<<MirrorTxPrtMsk_OFFSET)&MirrorTxPrtMsk_MASK) |
		((mRxMask << MirrorRxPrtMsk_OFFSET) & MirrorRxPrtMsk_MASK)|
		((mPortMask<<MirrorPortMsk_OFFSET) & MirrorPortMsk_MASK);

	WRITE_MEM32(PMCR,pmcr);

	return SUCCESS;
}

int32 rtl8651_getAsicPortMirror(uint32 *mRxMask, uint32 *mTxMask, uint32 *mPortMask)
{
	uint32 pmcr = READ_MEM32( PMCR );

	if ( mPortMask )
	{
		*mPortMask = ( pmcr & MirrorPortMsk_MASK ) >> MirrorPortMsk_OFFSET;
	}

	if ( mRxMask )
	{
		*mRxMask = ( pmcr & MirrorRxPrtMsk_MASK ) >> MirrorRxPrtMsk_OFFSET;
	}

	if ( mTxMask )
	{
		*mTxMask = ( pmcr & MirrorTxPrtMsk_MASK ) >> MirrorTxPrtMsk_OFFSET;
	}

	return SUCCESS;
}

int32 rtl8651_clearAsicL2Table(void)
{
	rtl8651_clearSpecifiedAsicTable(TYPE_L2_SWITCH_TABLE, RTL8651_L2TBL_ROW*RTL8651_L2TBL_COLUMN);
	rtl8651_clearSpecifiedAsicTable(TYPE_RATE_LIMIT_TABLE, RTL8651_RATELIMITTBL_SIZE);
	return SUCCESS;
}

inline int32 convert_setAsicL2Table(uint32 row, uint32 column, ether_addr_t * mac, int8 cpu,
		int8 srcBlk, uint32 mbr, uint32 ageSec, int8 isStatic, int8 nhFlag,int8 fid, int8 auth)
{
	rtl865x_tblAsicDrv_l2Param_t l2;

	memset(&l2,0,sizeof(rtl865x_tblAsicDrv_l2Param_t));

	l2.ageSec				= ageSec;
	l2.cpu				= cpu;
	l2.isStatic				= isStatic;
	l2.memberPortMask		= mbr;
	l2.nhFlag				= nhFlag;
	l2.srcBlk				= srcBlk;
//#ifdef RTL865XC_LAN_PORT_NUM_RESTRIT
//	if(enable4LanPortNumRestrict == TRUE)
	l2.fid=fid;
	l2.auth = auth;
//#endif
	memcpy(&l2.macAddr, mac, 6);
	return rtl8651_setAsicL2Table(row, column, &l2);
}

/*
 * <<RTL8651 version B Bug>>
 * RTL8651 L2 entry bug:
 *		For each L2 entry added by driver table as a static entry, the aging time
 *		will not be updated by ASIC
 * Bug fixed:
 *		To patch this bug, set the entry is a dynamic entry and turn on the 'nhFlag',
 *		then the aging time of this entry will be updated and once aging time expired,
 *		it won't be removed by ASIC automatically.
 */
int32 rtl8651_setAsicL2Table_Patch(uint32 row, uint32 column, ether_addr_t * mac, int8 cpu,
		int8 srcBlk, uint32 mbr, uint32 ageSec, int8 isStatic, int8 nhFlag, int8 fid,int8 auth)
{
#if 0
	ether_addr_t bcast_mac = { {0xff, 0xff, 0xff, 0xff, 0xff, 0xff} };
	ether_addr_t cpu_mac = {{0x00,0x00,0x0a,0x00,0x00,0x0f}};

	/*
		In RTL865xC, we need to turn on the CPU bit of broadcast mac to let broadcast packets being trapped to CPU.
	*/

	if ( memcmp( &bcast_mac, mac, sizeof(ether_addr_t) ) == 0 || memcmp(&cpu_mac,mac,sizeof(ether_addr_t)) == 0)
	{
		return convert_setAsicL2Table(
				row,
				column,
				mac,
				TRUE,	/* Set CPU bit to TRUE */
				FALSE,
				mbr,
				500,
				isStatic,	/* No one will be broadcast/multicast source */
				nhFlag,/* No one will be broadcast/multicast source */
				TRUE
		);
	}
#endif
	if(mac->octet[0]&0x1)
	{
		return convert_setAsicL2Table(
				row,
				column,
				mac,
				cpu,
				FALSE,
				mbr,
				ageSec,
				isStatic,	/* No one will be broadcast/multicast source */
				nhFlag,	/* No one will be broadcast/multicast source */
				fid,
				TRUE
		);
	}
	else {
		int8 dStatic=isStatic/*, dnhFlag=(isStatic==TRUE? TRUE: FALSE)*/;
		int8 dnhFlag = nhFlag;
#if defined(CONFIG_RTL865X_PPTPL2TP)||defined(CONFIG_RTL865XB_PPTPL2TP)
		extern rtl8651_tblDrv_miiTunneling_t tunnel;
		uint32 MBR = (1 << tunnel.loopbackPort);
		if (tunnel.valid && mbr==MBR) {
			dStatic = TRUE;
			dnhFlag = FALSE;
		}
#endif
		return convert_setAsicL2Table(
				row,
				column,
				mac,
				cpu,
				srcBlk,
				mbr,
				ageSec,
				dStatic,
				dnhFlag,
				fid,
				auth
//				FALSE,							/* patch here!! always dynamic entry */
//				(isStatic==TRUE? TRUE: FALSE)	/* patch here!! nhFlag always turned on if static entry*/
		);
	}
}


/*
 * <<RTL8651 version B Bug>>
 * RTL8651 L2 entry bug:
 *		For each L2 entry added by driver table as a static entry, the aging time
 *		will not be updated by ASIC
 * Bug fixed:
 *		To patch this bug, set the entry as a dynamic entry and turn on the 'nhFlag',
 *		then the aging time of this entry will be updated and once aging time expired,
 *		it won't be removed by ASIC automatically.
 */
#if 0
int32 rtl8651_getAsicL2Table_Patch(uint32 row, uint32 column, ether_addr_t * mac, int8 * cpu,
	int8 * srcBlk, int8 * isStatic, uint32 * mbr, uint32 * ageSec, int8 *nhFlag)
{
	rtl865x_tblAsicDrv_l2Param_t l2;

	int32 retval = rtl8651_getAsicL2Table(row, column, &l2);
	if (mac) memcpy(mac, &l2.macAddr, 6);
	if (cpu) *cpu = l2.cpu;
	if (srcBlk) *srcBlk = l2.srcBlk;
	if (isStatic) *isStatic = l2.isStatic;
	if (mbr) *mbr = l2.memberPortMask;
	if (ageSec) *ageSec = l2.ageSec;
	if (nhFlag) *nhFlag = l2.nhFlag;
	if (isStatic != NULL) *isStatic = TRUE; /* patch!!, always TRUE(static entry */
	if (nhFlag != NULL) *nhFlag = FALSE;  /* always false */
	return retval;
}
#else

int32 rtl8651_getAsicL2Table_Patch(uint32 row, uint32 column, rtl865x_tblAsicDrv_l2Param_t *asic_l2_t)
{
	int32 retval = rtl8651_getAsicL2Table(row, column, asic_l2_t);
#ifdef CONFIG_RTL865XB_EXP_INVALID
	if (retval == SUCCESS) {
		asic_l2_t->isStatic	= TRUE;
		asic_l2_t->nhFlag	= FALSE;
	}
#endif
	return retval;
}
#endif

#if !defined(CONFIG_RTL_819X)
#define	PAGE_SELECT_REGID		31
#define	PAGE_SELECT_OFFSET		0
#define	PAGE_SELECT_MASK		0xF
#define		EXTRTL_8214_REGBASE_1		CONFIG_EXTRTL8212_PHYID_P1
#define		EXTRTL_8214_REGBASE_3		CONFIG_EXTRTL8212_PHYID_P3
static inline unsigned int rtl865x_probeP1toP4GigaPHYChip(void)
{
	unsigned int uid,tmp;
	unsigned int i;

	/* Read */
	for(i=0; i<4; i++)  //probe p1-p4
	{
		rtl8651_getAsicEthernetPHYReg( CONFIG_EXTRTL8212_PHYID_P1+i, 2, &tmp );
		uid=tmp<<16;
		rtl8651_getAsicEthernetPHYReg( CONFIG_EXTRTL8212_PHYID_P1+i, 3, &tmp );
		uid=uid | tmp;

		if( uid==0x001CC912 )  //0x001cc912 is 8212 two giga port , 0x001cc940 is 8214 four giga port
		{
			return 1;
		}
		else if(uid==0x001CC940)
		{
			//printk("Find Port1-4 8214 PHY Chip! \r\n");
			//FixPHYChip();
			//RstGigaPhy();
			return 1;
		}
	}
	return 0;
}

static inline unsigned int rtl865x_probeP5GigaPHYChip(void)
{
	unsigned int uid,tmp;

	/* Read */
	rtl8651_getAsicEthernetPHYReg( CONFIG_EXTRTL8212_PHYID_P5, 0, &tmp );
	rtl8651_setAsicEthernetPHYReg(CONFIG_EXTRTL8212_PHYID_P5,0x10,0x01FE);

	/* Read */
	rtl8651_getAsicEthernetPHYReg( CONFIG_EXTRTL8212_PHYID_P5, 2, &tmp );
	uid=tmp<<16;
	rtl8651_getAsicEthernetPHYReg( CONFIG_EXTRTL8212_PHYID_P5, 3, &tmp );
	uid=uid | tmp;

	if( uid==0x001CC912 )  //0x001cc912 is 8212 two giga port , 0x001cc940 is 8214 four giga port
	{	//printk("Find Port5   have 8211 PHY Chip! \r\n");
		return 1;
	}

	return 0;
}

static void	rtl865x_fix8214Bug(void)
{
	/*	52_phy_write 0x12 9 21 0xDD0A	*/
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1, PAGE_SELECT_REGID, 0x0009);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1, 21, 0xDD0A);
	/*	52_phy_write 0x13 9 21 0xDD0A	*/
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1+1, PAGE_SELECT_REGID, 0x0009);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1+1, 21, 0xDD0A);
	/*	52_phy_write 0x14 8 28 0x0003	*/
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3, PAGE_SELECT_REGID, 0x0008);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3, 28, 0x0003);

	/*	mdcmdio_cmd w 0x12 31  0x0002
	*	mdcmdio_cmd w 0x12 8  0x3672
	*	mdcmdio_cmd w 0x12 9  0x8c00
	*	mdcmdio_cmd w 0x12 12  0x5b15
	*	mdcmdio_cmd w 0x12 18  0x0edd
	*	mdcmdio_cmd w 0x12 27  0x5c5c
	*/
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1, PAGE_SELECT_REGID, 0x0002);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1, 8, 0x3672);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1, 9, 0x8c00);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1, 12, 0x5b15);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1, 18, 0x0edd);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1, 27, 0x5c5c);

	/*	mdcmdio_cmd w 0x13 31  0x0002
	*	mdcmdio_cmd w 0x13 8  0x3672
	*	mdcmdio_cmd w 0x13 9  0x8c00
	*	mdcmdio_cmd w 0x13 12  0x5b15
	*	mdcmdio_cmd w 0x13 18  0x0edd
	*	mdcmdio_cmd w 0x13 27  0x5c5c
	*/
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1+1, PAGE_SELECT_REGID, 0x0002);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1+1, 8, 0x3672);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1+1, 9, 0x8c00);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1+1, 12, 0x5b15);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1+1, 18, 0x0edd);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1+1, 27, 0x5c5c);

	/*	mdcmdio_cmd w 0x14 31  0x0002
	*	mdcmdio_cmd w 0x14 8  0x3672
	*	mdcmdio_cmd w 0x14 9  0x8c00
	*	mdcmdio_cmd w 0x14 12  0x5b15
	*	mdcmdio_cmd w 0x14 18  0x0edd
	*	mdcmdio_cmd w 0x14 27  0x5c5c
	*/
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3, PAGE_SELECT_REGID, 0x0002);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3, 8, 0x3672);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3, 9, 0x8c00);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3, 12, 0x5b15);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3, 18, 0x0edd);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3, 27, 0x5c5c);
	/*	mdcmdio_cmd w 0x15 31  0x0002
	*	mdcmdio_cmd w 0x15 8  0x3672
	*	mdcmdio_cmd w 0x15 9  0x8c00
	*	mdcmdio_cmd w 0x15 12  0x5b15
	*	mdcmdio_cmd w 0x15 18  0x0edd
	*	mdcmdio_cmd w 0x15 27  0x5c5c
	*/
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3+1, PAGE_SELECT_REGID, 0x0002);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3+1, 8, 0x3672);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3+1, 9, 0x8c00);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3+1, 12, 0x5b15);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3+1, 18, 0x0edd);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3+1, 27, 0x5c5c);

	/*
	*	#RTL8214 Enable AUTO - K
	*	52_phy_write 0x12 0 20 0x8000
	*	52_phy_write 0x13 0 20 0x8000
	*	52_phy_write 0x14 0 20 0x8000
	*	52_phy_write 0x15 0 20 0x8000
	*/

	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1, PAGE_SELECT_REGID, 0x0000);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1, 20, 0x8000);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1+1, PAGE_SELECT_REGID, 0x0000);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1+1, 20, 0x8000);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3, PAGE_SELECT_REGID, 0x0000);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3, 20, 0x8000);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3+1, PAGE_SELECT_REGID, 0x0000);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3+1, 20, 0x8000);

	/*
	*	52_phy_write 0x12 0 20 0x8040
	*	52_phy_write 0x13 0 20 0x8040
	*	52_phy_write 0x14 0 20 0x8040
	*	52_phy_write 0x15 0 20 0x8040
	*/
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1, PAGE_SELECT_REGID, 0x0000);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1, 20, 0x8040);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1+1, PAGE_SELECT_REGID, 0x0000);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1+1, 20, 0x8040);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3, PAGE_SELECT_REGID, 0x0000);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3, 20, 0x8040);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3+1, PAGE_SELECT_REGID, 0x0000);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3+1, 20, 0x8040);
	/*
		#change to default page
		52_phy_read 0x12 8 0
	*/
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1, PAGE_SELECT_REGID, 0x0008);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1+1, PAGE_SELECT_REGID, 0x0008);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3, PAGE_SELECT_REGID, 0x0008);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3+1, PAGE_SELECT_REGID, 0x0008);

#if 0
	/* Test */
	rtl8651_getAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1+1, PAGE_SELECT_REGID, &rData);
	rtl8651_getAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1+1, 15, &rData);
	rtl865x_setAsicEthernetPHYPage(EXTRTL_8214_REGBASE_1+1, 8);
	printk("*********************\nrData 0x%x\n*********************\n", rData);
#endif
}
#endif

static int32 _rtl8651_initAsicPara( rtl8651_tblAsic_InitPara_t *para )
{
	memset(&rtl8651_tblAsicDrvPara, 0, sizeof(rtl8651_tblAsic_InitPara_t));

	if ( para )
	{
		/* Parameter != NULL, check its correctness */
		if (para->externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT1234_RTL8212)
		{
			ASICDRV_ASSERT(para->externalPHYId[1] != 0);
			ASICDRV_ASSERT(para->externalPHYId[2] != 0);
			ASICDRV_ASSERT(para->externalPHYId[3] != 0);
			ASICDRV_ASSERT(para->externalPHYId[4] != 0);
			ASICDRV_ASSERT(para->externalPHYId[2] == (para->externalPHYId[1] + 1));
			ASICDRV_ASSERT(para->externalPHYId[4] == (para->externalPHYId[3] + 1));
		}
		if (para->externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
		{
			ASICDRV_ASSERT(para->externalPHYId[5] != 0);
		}

		/* ============= Check passed : set it =============  */
		memcpy(&rtl8651_tblAsicDrvPara, para, sizeof(rtl8651_tblAsic_InitPara_t));
	}

	return SUCCESS;
}

#if defined(CONFIG_RTL8196C_REVISION_B) || defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
void Set_GPHYWB(unsigned int phyid, unsigned int page, unsigned int reg, unsigned int mask, unsigned int val)
{
	unsigned int data=0;
	unsigned int wphyid=0;	//start
	unsigned int wphyid_end=1;   //end

	if(phyid==999)
	{	wphyid=0;
		wphyid_end=5;    //total phyid=0~4
	}
	else
	{	wphyid=phyid;
		wphyid_end=phyid+1;
	}

	for(; wphyid<wphyid_end; wphyid++)
	{
		//change page
		if(page>=31)
		{	rtl8651_setAsicEthernetPHYReg( wphyid, 31, 7  );
			rtl8651_setAsicEthernetPHYReg( wphyid, 30, page  );
		}
		else
		{
			rtl8651_setAsicEthernetPHYReg( wphyid, 31, page  );
		}

		if(mask != 0)
		{
			rtl8651_getAsicEthernetPHYReg( wphyid, reg, &data);
			data = data&mask;
		}
		rtl8651_setAsicEthernetPHYReg( wphyid, reg, data|val  );


		//switch to page 0
		rtl8651_setAsicEthernetPHYReg( wphyid, 31, 0  );
	}
}
#endif

#ifdef CONFIG_RTL8196C_REVISION_B

#ifdef CONFIG_RTL8196C_ETH_IOT
void set_gray_code_by_port(int port)
{
        uint32 val;

        rtl8651_setAsicEthernetPHYReg( 4, 31, 1  );

        rtl8651_getAsicEthernetPHYReg( 4, 20, &val  );
        rtl8651_setAsicEthernetPHYReg( 4, 20, val + (0x1 << port)  );

        rtl8651_setAsicEthernetPHYReg( port, 31, 1  );

        rtl8651_setAsicEthernetPHYReg( port, 19,  0x5400 );
        if (port<4) rtl8651_setAsicEthernetPHYReg( port, 19,  0x5440 );
        if (port<3) rtl8651_setAsicEthernetPHYReg( port, 19,  0x54c0 );
        if (port<2) rtl8651_setAsicEthernetPHYReg( port, 19,  0x5480 );
        if (port<1) rtl8651_setAsicEthernetPHYReg( port, 19,  0x5580 );

        rtl8651_setAsicEthernetPHYReg( 4, 20, 0xb20  );
       rtl8651_setAsicEthernetPHYReg( port, 31, 0  );

        rtl8651_setAsicEthernetPHYReg( 4, 31, 0  );
}
#endif

void Setting_RTL8196C_PHY(void)
{
	int i=0;
	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);

	/*
	  #=========ADC Bias Current =========================
	  #RG1X_P4~0 [12:10] = Reg_pi_fix [2:0], 5 ->7
	  phywb all 1 17 12-10 0x7
	*/
	Set_GPHYWB(999, 1, 17, 0xffff-(7<<10), 0x7<<10);

	/*
	  #=========patch for eee============================
	  #1. page4¡Breg24¡Glpi_rx_ti_timer_cnt change to f3
	  phywb all 4 24 7-0 0xf3

	  #2. page4¡Breg16¡Grg_txqt_ps_sel change to 1
	  phywb all 4 16 3 1
	*/
	Set_GPHYWB(999, 4, 24, 0xff00, 0xf3);
	Set_GPHYWB(999, 4, 16, 0xffff-(1<<3), 1<<3);
	/*
	  #=========patch for IOL Tx amp.=====================
	  #<a>modify 100M DAC current default value:
	  #Port#0~#4(per port control)
	  #Page1,Reg19,bit[13:11]:
	  #original value 200uA(3b'100),--> change to 205u(3b'000)   => change to 205u(3b010)

	  phywb all 1 19 13-11 0x2

	  #<b>modify bandgap voltage default value:
	  #Port#0~#4 (Global all ports contorl setting),
	  #Page1,Reg23,bit[8:6],

	  #original value 1.312V(3b'110),-->change to 1.212V(3b'100).

	  phywb all 1 23 8-6 0x4

	  #<c>modify TX CS cap default value:
	  #Port#0~#4 (Global all ports contorl setting),
	  #Page1,Reg18,bit[5:3],

	  #original value Reserved bits(3b'000),-->change to 600fF(3b'011). =>change to 750fF(3b'110)
	  phywb all 1 18 5-3 0x6
	*/

	Set_GPHYWB(999, 1, 19, 0xffff-(7<<11), 0x2<<11);
	Set_GPHYWB(999, 1, 23, 0xffff-(7<<6)  , 0x4<<6);
	Set_GPHYWB(999, 1, 18, 0xffff-(7<<3), 0x6<<3);


	/* 20100223 from Maxod: 100M half duplex enhancement */
 	REG32(MACCR)= (REG32(MACCR) & ~CF_RXIPG_MASK) | 0x05;

	/* fix the link down / link up issue with SmartBit 3101B when DUT(8196c) set to Auto-negotiation
	    and SmartBit force to 100M Full-duplex */
 	REG32(MACCR)= (REG32(MACCR) & ~SELIPG_MASK) | SELIPG_11;

	/*20100222 from Anson:Switch Corner test pass setting*/
	 /*
		REG21 default=0x2c5
		After snr_ub(page0 reg21.7-4) = 3 and snr_lb(page0 reg21.3-0)=2 ,REG21=0x232
		REG22 default=0x5b85
		After adtune_lb(page0 reg22.6-4)=4 (10uA) ,REG21=0x5b45

		REG0 default=0x1100
		restart AN
		page0 reg0.9 =1 , ,REG0=0x1300
	*/
	//rtl8651_setAsicEthernetPHYReg( i, 0x15, 0x232 );
	//Set_GPHYWB(999, 0, 21, 0xffff-(0xff<<0), 0x32<<0);
	// test 96C to 96C restart AN 100 times, result is pass ==> page0 reg21.14(disable the equlizar)=1
#ifdef CONFIG_RTL8196C_ETH_IOT
	// enable "equalizer reset", i.e. page 0 , reg21, bit14= 0
	Set_GPHYWB(999, 0, 21, (~0x40ff), 0x0032);
#else
	Set_GPHYWB(999, 0, 21, (~0x40ff), 0x4032);
#endif

	//rtl8651_setAsicEthernetPHYReg( i, 0x16, 0x5b45 );
	//Set_GPHYWB(999, 0, 22, 0xffff-(7<<4), 0x4<<4);
	Set_GPHYWB(999, 0, 22, 0xffff-(7<<4), 0x5<<4);
	//rtl8651_setAsicEthernetPHYReg( i, 0x0, 0x1300 );
	Set_GPHYWB(999, 0, 0, 0xffff-(1<<9), 0x1<<9);

	/*20100225 from Anson:Switch Force cailibration
	#change calibration update method for patch first pkt no update impedance
	phywb all 1 29 1 0
	#--------------Patch for impedance update fail cause rx crc error with long calbe--------
	#Froce cailibration
	phywb all 1 29 2 1
	#Force impedance value = 0x8888
	phywb all 1 28 15-0 0x8888
	#-----------------------------------------------------------------------------------------
	#Select clock (ckt125[4]) edge trigger mlt3[1:0] = negative for patch four corner fail issue(only tx timing)
	phywb all 1 17 2-1 0x3
	*/
	//Set_GPHYWB(999, 1, 29, 0xffff-(1<<1), 0x0<<1);
	//Set_GPHYWB(999, 1, 29, 0xffff-(1<<2), 0x1<<2);
	//Set_GPHYWB(999, 1, 28, 0xffff-(0xffff), 0x8888);
	Set_GPHYWB(999, 1, 17, 0xffff-(3<<1), 0x3<<1);

	/*20100222 from Yozen:AOI TEST pass setting*/
	Set_GPHYWB(999, 1, 18, 0xffff-(0xffff), 0x9004);

	// for "DSP recovery fail when link partner = force 100F"
	Set_GPHYWB(999, 4, 26, 0xffff-(0xfff<<4), 0xff8<<4);

	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) &= ~(EnForceMode);

#ifdef CONFIG_RTL8196C_ETH_IOT
        for(i=0; i<5; i++) {
                set_gray_code_by_port(i);
        }
#endif
	printk("  Set 8196C PHY Patch OK\n");

}
#endif

#ifdef CONFIG_RTL_8198
static const unsigned int phy_para[]={
	//###################### PHY parameter patch ################################
	0x1f, 0x0005, //Page 5
	0x13, 0x0003, //Page 5 ########## EMA =3#############
	//0x01, 0x0700; #Page 5 Reg 1 = 0x0700, NEC ON	(20100112)
	0x05,0x8B82,//Page 5 Reg 5 = 0x8B82, Fix 100M re-link fail issue (20100110)
	0x06,0x05CB,//Page 5 Reg 6 = 0x05CB, Fix 100M re-link fail issue (20100110)
	0x1f,0x0002,//Page 2
	0x04,0x80C2,//Page 2 Reg 4 0x80C2, Fix 100M re-link fail issue (20100110)
	0x05,0x0938,//Page 2 Reg 5 0x0938, Disable 10M standby mode (20100112)

	0x1F,0x0003,//Page 3
	0x12,0xC4D2,//Page 3 Reg 18 = 0xC4D2, GAIN upper bond=24
	0x0D,0x0207,//Page 3 Reg 13 = 0x0207 (20100112)
	0x01,0x3554, //#Page 3 Reg  1 = 0x3554 (20100423)
	0x02,0x63E8, //#Page 3 Reg  2 = 0x63E8 (20100423)
	0x03,0x99C2, //#Page 3 Reg  3 = 0x99C2 (20100423)
	0x04,0x0113, //#Page 3 Reg  4 = 0x0113 (20100423)

	0x1f,0x0001,//Page 1
	0x07,0x267E,//Page 1 Reg  7 = 0x267E, Channel Gain offset (20100111)
	0x1C,0xE5F7,//Page 1 Reg 28 = 0xE5F7, Cable length offset (20100111)
	0x1B,0x0424,//Page 1 Reg 27 = 0x0424, SD threshold (20100111)

	//#Add by Gary for Channel Estimation fine tune 20100430
	//0x1f,0x0002, //# change to Page 1 (Global)
	//0x08,0x0574, //# Page1 Reg8 (CG_INITIAL_MASTER)
	//0x09,0x2724, //# Page1 Reg9 (CB0_INITIAL_GIGA)
	//0x1f,0x0003, //# change to Page 3 (Global)
	//0x1a,0x06f6, //# Page3 Reg26 (CG_INITIAL_SLAVE)

	//#Add by Gary for Channel Estimation fine tune 20100430
	//#Page1 Reg8 (CG_INITIAL_MASTER)
	//0x1f, 0x0005,
	//0x05, 0x83dd,
	//0x06, 0x0574,
	//#Page1 Reg9 (CB0_INITIAL_GIGA)
	//0x1f, 0x0005,
	//0x05, 0x83e0,
	//0x06, 0x2724,
	//#Page3 Reg26 (CG_INITIAL_SLAVE)
	//0x1f, 0x0005,
	//0x05, 0x843d,
	//0x06, 0x06f6 ,

	//#NC FIFO
	0x1f,0x0007,//ExtPage
	0x1e,0x0042,//ExtPage 66
	0x18,0x0000,//Page 66 Reg 24 = 0x0000, NC FIFO (20100111)
	0x1e,0x002D,//ExtPage 45
	0x18,0xF010,//Page 45 Reg 24 = 0xF010, Enable Giga Down Shift to 100M (20100118)

	0x1e,0x002c, //#ExtPage 44
	0x18,0x008B, //#Page 44 Reg 24 = 0x008B, Enable deglitch circuit (20100426)

	//############################ EEE giga patch ################################

	//0x1f 0x0007;
	0x1e,0x0028,
	0x16,0xf640,//phywb $phyID 40 22 15-0 0xF640

	0x1e,0x0021,
	0x19,0x2929,//phywb $phyID 33 25 15-0 0x2929

	0x1a,0x1005,//phywb $phyID 33 26 15-0 0x1005

	0x1e,0x0020,
	0x17,0x000a,//phywb $phyID 32 23 15-0 0x000a

	0x1b,0x2f4a,//Disable EEE PHY mode
	0x15,0x0100,//EEE ability, Disable EEEP

	0x1e,0x0040,//
	0x1a,0x5110,//	phywb $phyID 64 26 15-0 0x5110
	0x18,0x0000,// programable mode

	0x1e,0x0041,//
	0x15,0x0e02,//phywb $phyID 65 21 15-0 0x0e02

	0x16,0x2185,//phywb $phyID 65 22 15-0 0x2185
	0x17,0x000c,//phywb $phyID 65 23 15-0 0x000c
	0x1c,0x0008,//phywb $phyID 65 28 15-0 0x0008
	0x1e,0x0042,//
	0x15,0x0d00,//phywb $phyID 66 21 15-0 0x0d00

	#if 1
	//############################ EEE Run code patch #################################
	//###proc 67R_ram_code_20100211_inrx_uc_98_1
	//###proc ram_code_0223_uc {} {
	//#replace 0xfff6, lock uc ram code version!
	31, 0x5,
	5,  0xfff6,
	6,  0x0080,
	5,  0x8b6e,
	6,  0x0000,
	15, 0x0100,

	//### force MDI/MDIX
	0x1f, 0x0007,
	0x1e, 0x002d,
	0x18, 0xf030,

	//### pcs nctl patch code (0423)
	0x1f, 0x0007,
	0x1e, 0x0023,
	0x16, 0x0005,

	//### startpoint
	 0x15, 0x005c,
	0x19, 0x0068,
	0x15, 0x0082,
	0x19, 0x000a,
	0x15, 0x00a1,
	0x19, 0x0081,
	0x15 ,0x00af,
	0x19, 0x0080,
	0x15, 0x00d4,
	0x19, 0x0000,
	0x15, 0x00e4,
	0x19, 0x0081,
	0x15, 0x00e7,
	0x19, 0x0080,
	0x15, 0x010d,
	0x19, 0x0083,
	0x15, 0x0118,
	0x19, 0x0083,
	0x15, 0x0120,
	0x19, 0x0082,
	0x15, 0x019c,
	0x19, 0x0081,
	0x15, 0x01a4,
	0x19, 0x0080,
	0x15, 0x01cd,
	0x19, 0x0000,
	0x15, 0x01dd,
	0x19, 0x0081,
	0x15, 0x01e0,
	0x19, 0x0080,
	//### endpoint

	0x16, 0x0000,
	//### end of pcs nctl patch code

	//inrx
	0x1f, 0x0007,
	0x1e, 0x0040,
	0x18, 0x0004,
	0x1f,0x0000,
	0x17,0x2160,
	0x1f,0x0007,
	0x1e,0x0040,

	//### startpoint
	0x18,0x0004,
	0x19,0x4000,
	0x18,0x0014,
	0x19,0x7f00,
	0x18,0x0024,
	0x19,0x0000,
	0x18,0x0034,
	0x19,0x0100,
	0x18,0x0044,
	0x19,0xe000,
	0x18,0x0054,
	0x19,0x0000,
	0x18,0x0064,
	0x19,0x0000,
	0x18,0x0074,
	0x19,0x0000,
	0x18,0x0084,
	0x19,0x0400,
	0x18,0x0094,
	0x19,0x8000,
	0x18,0x00a4,
	0x19,0x7f00,
	0x18,0x00b4,
	0x19,0x4000,
	0x18,0x00c4,
	0x19,0x2000,
	0x18,0x00d4,
	0x19,0x0100,
	0x18,0x00e4,
	0x19,0x8400,
	0x18,0x00f4,
	0x19,0x7a00,
	0x18,0x0104,
	0x19,0x4000,
	0x18,0x0114,
	0x19,0x3f00,
	0x18,0x0124,
	0x19,0x0100,
	0x18,0x0134,
	0x19,0x7800,
	0x18,0x0144,
	0x19,0x0000,
	0x18,0x0154,
	0x19,0x0000,
	0x18,0x0164,
	0x19,0x0000,
	0x18,0x0174,
	0x19,0x0400,
	0x18,0x0184,
	0x19,0x8000,
	0x18,0x0194,
	0x19,0x7f00,
	0x18,0x01a4,
	0x19,0x8300,
	0x18,0x01b4,
	0x19,0x8300,
	0x18,0x01c4,
	0x19,0xe200,
	0x18,0x01d4,
	0x19,0x0a00,
	0x18,0x01e4,
	0x19,0x8800,
	0x18,0x01f4,
	0x19,0x0300,
	0x18,0x0204,
	0x19,0xe100,
	0x18,0x0214,
	0x19,0x4600,
	0x18,0x0224,
	0x19,0x4000,
	0x18,0x0234,
	0x19,0x7f00,
	0x18,0x0244,
	0x19,0x0000,
	0x18,0x0254,
	0x19,0x0100,
	0x18,0x0264,
	0x19,0x4000,
	0x18,0x0274,
	0x19,0x3e00,
	0x18,0x0284,
	0x19,0x0000,
	0x18,0x0294,
	0x19,0xe000,
	0x18,0x02a4,
	0x19,0x1200,
	0x18,0x02b4,
	0x19,0x8000,
	0x18,0x02c4,
	0x19,0x7f00,
	0x18,0x02d4,
	0x19,0x8900,
	0x18,0x02e4,
	0x19,0x8300,
	0x18,0x02f4,
	0x19,0xe000,
	0x18,0x0304,
	0x19,0x0000,
	0x18,0x0314,
	0x19,0x4000,
	0x18,0x0324,
	0x19,0x7f00,
	0x18,0x0334,
	0x19,0x0000,
	0x18,0x0344,
	0x19,0x2000,
	0x18,0x0354,
	0x19,0x4000,
	0x18,0x0364,
	0x19,0x3e00,
	0x18,0x0374,
	0x19,0xfd00,
	0x18,0x0384,
	0x19,0x0000,
	0x18,0x0394,
	0x19,0x1200,
	0x18,0x03a4,
	0x19,0xab00,
	0x18,0x03b4,
	0x19,0x0c00,
	0x18,0x03c4,
	0x19,0x0600,
	0x18,0x03d4,
	0x19,0xa000,
	0x18,0x03e4,
	0x19,0x3d00,
	0x18,0x03f4,
	0x19,0xfb00,
	0x18,0x0404,
	0x19,0xe000,
	0x18,0x0414,
	0x19,0x0000,
	0x18,0x0424,
	0x19,0x4000,
	0x18,0x0434,
	0x19,0x7f00,
	0x18,0x0444,
	0x19,0x0000,
	0x18,0x0454,
	0x19,0x0100,
	0x18,0x0464,
	0x19,0x4000,
	0x18,0x0474,
	0x19,0xc600,
	0x18,0x0484,
	0x19,0xff00,
	0x18,0x0494,
	0x19,0x0000,
	0x18,0x04a4,
	0x19,0x1000,
	0x18,0x04b4,
	0x19,0x0200,
	0x18,0x04c4,
	0x19,0x7f00,
	0x18,0x04d4,
	0x19,0x4000,
	0x18,0x04e4,
	0x19,0x7f00,
	0x18,0x04f4,
	0x19,0x0200,
	0x18,0x0504,
	0x19,0x0200,
	0x18,0x0514,
	0x19,0x5200,
	0x18,0x0524,
	0x19,0xc400,
	0x18,0x0534,
	0x19,0x7400,
	0x18,0x0544,
	0x19,0x0000,
	0x18,0x0554,
	0x19,0x1000,
	0x18,0x0564,
	0x19,0xbc00,
	0x18,0x0574,
	0x19,0x0600,
	0x18,0x0584,
	0x19,0xfe00,
	0x18,0x0594,
	0x19,0x4000,
	0x18,0x05a4,
	0x19,0x7f00,
	0x18,0x05b4,
	0x19,0x0000,
	0x18,0x05c4,
	0x19,0x0a00,
	0x18,0x05d4,
	0x19,0x5200,
	0x18,0x05e4,
	0x19,0xe400,
	0x18,0x05f4,
	0x19,0x3c00,
	0x18,0x0604,
	0x19,0x0000,
	0x18,0x0614,
	0x19,0x1000,
	0x18,0x0624,
	0x19,0x8a00,
	0x18,0x0634,
	0x19,0x7f00,
	0x18,0x0644,
	0x19,0x4000,
	0x18,0x0654,
	0x19,0x7f00,
	0x18,0x0664,
	0x19,0x0100,
	0x18,0x0674,
	0x19,0x2000,
	0x18,0x0684,
	0x19,0x0000,
	0x18,0x0694,
	//### 0x2219:0x4600 =>0xe600 #0309
	0x19,0xe600,
	0x18,0x06a4,
	//### 0x2219:0xfc00 =>0xff00 #0309
	0x19,0xff00,
	0x18,0x06b4,
	0x19,0x0000,
	0x18,0x06c4,
	0x19,0x5000,
	0x18,0x06d4,
	0x19,0x9d00,
	0x18,0x06e4,
	0x19,0xff00,
	0x18,0x06f4,
	0x19,0x4000,
	0x18,0x0704,
	0x19,0x7f00,
	0x18,0x0714,
	0x19,0x0000,
	0x18,0x0724,
	0x19,0x2000,
	0x18,0x0734,
	0x19,0x0000,
	0x18,0x0744,
	0x19,0xe600,
	0x18,0x0754,
	0x19,0xff00,
	0x18,0x0764,
	0x19,0x0000,
	0x18,0x0774,
	0x19,0x5000,
	0x18,0x0784,
	0x19,0x8500,
	0x18,0x0794,
	0x19,0x7f00,
	0x18,0x07a4,
	0x19,0xac00,
	0x18,0x07b4,
	0x19,0x0800,
	0x18,0x07c4,
	0x19,0xfc00,
	0x18,0x07d4,
	0x19,0x4000,
	0x18,0x07e4,
	0x19,0x7f00,
	0x18,0x07f4,
	0x19,0x0400,
	0x18,0x0804,
	0x19,0x0200,
	0x18,0x0814,
	0x19,0x0000,
	0x18,0x0824,
	0x19,0xff00,
	0x18,0x0834,
	0x19,0x7f00,
	0x18,0x0844,
	0x19,0x0000,
	0x18,0x0854,
	0x19,0x4200,
	0x18,0x0864,
	0x19,0x0500,
	0x18,0x0874,
	0x19,0x9000,
	0x18,0x0884,
	0x19,0x8000,
	0x18,0x0894,
	0x19,0x7d00,
	0x18,0x08a4,
	0x19,0x8c00,
	0x18,0x08b4,
	0x19,0x8300,
	0x18,0x08c4,
	0x19,0xe000,
	0x18,0x08d4,
	0x19,0x0000,
	0x18,0x08e4,
	0x19,0x4000,
	0x18,0x08f4,
	0x19,0x0400,
	0x18,0x0904,
	0x19,0xff00,
	0x18,0x0914,
	0x19,0x0500,
	0x18,0x0924,
	0x19,0x8500,
	0x18,0x0934,
	0x19,0x8c00,
	0x18,0x0944,
	0x19,0xfa00,
	0x18,0x0954,
	0x19,0xe000,
	0x18,0x0964,
	0x19,0x0000,
	0x18,0x0974,
	0x19,0x4000,
	0x18,0x0984,
	0x19,0x5f00,
	0x18,0x0994,
	0x19,0x0400,
	0x18,0x09a4,
	0x19,0x0000,
	0x18,0x09b4,
	0x19,0xfe00,
	0x18,0x09c4,
	0x19,0x7300,
	0x18,0x09d4,
	0x19,0x0d00,
	0x18,0x09e4,
	0x19,0x0300,
	0x18,0x09f4,
	0x19,0x4000,
	0x18,0x0a04,
	0x19,0x2000,
	0x18,0x0a14,
	0x19,0x0000,
	0x18,0x0a24,
	0x19,0x0400,
	0x18,0x0a34,
	0x19,0xda00,
	0x18,0x0a44,
	0x19,0x0600,
	0x18,0x0a54,
	0x19,0x7d00,
	0x18,0x0a64,
	0x19,0x4000,
	0x18,0x0a74,
	0x19,0x5f00,
	0x18,0x0a84,
	0x19,0x0400,
	0x18,0x0a94,
	0x19,0x0000,
	0x18,0x0aa4,
	0x19,0x0000,
	0x18,0x0ab4,
	0x19,0x7300,
	0x18,0x0ac4,
	0x19,0x0d00,
	0x18,0x0ad4,
	0x19,0x0300,
	0x18,0x0ae4,
	0x19,0x0400,
	0x18,0x0af4,
	0x19,0xce00,
	0x18,0x0b04,
	0x19,0x0900,
	0x18,0x0b14,
	0x19,0x9d00,
	0x18,0x0b24,
	0x19,0x0800,
	0x18,0x0b34,
	0x19,0x9000,
	0x18,0x0b44,
	0x19,0x0700,
	0x18,0x0b54,
	0x19,0x7900,
	0x18,0x0b64,
	0x19,0x4000,
	0x18,0x0b74,
	0x19,0x7f00,
	0x18,0x0b84,
	0x19,0x0400,
	0x18,0x0b94,
	0x19,0x0000,
	0x18,0x0ba4,
	0x19,0x0000,
	0x18,0x0bb4,
	0x19,0x0400,
	0x18,0x0bc4,
	0x19,0x7300,
	0x18,0x0bd4,
	0x19,0x0d00,
	0x18,0x0be4,
	0x19,0x0100,
	0x18,0x0bf4,
	0x19,0x0900,
	0x18,0x0c04,
	0x19,0x8e00,
	0x18,0x0c14,
	0x19,0x0800,
	0x18,0x0c24,
	0x19,0x7d00,
	0x18,0x0c34,
	0x19,0x4000,
	0x18,0x0c44,
	0x19,0x7f00,
	0x18,0x0c54,
	0x19,0x0000,
	0x18,0x0c64,
	0x19,0x0000,
	0x18,0x0c74,
	0x19,0x0200,
	0x18,0x0c84,
	0x19,0x0000,
	0x18,0x0c94,
	0x19,0x7000,
	0x18,0x0ca4,
	0x19,0x0c00,
	0x18,0x0cb4,
	0x19,0x0100,
	0x18,0x0cc4,
	0x19,0x0900,
	0x18,0x0cd4,
	0x19,0x7f00,
	0x18,0x0ce4,
	0x19,0x4000,
	0x18,0x0cf4,
	0x19,0x7f00,
	0x18,0x0d04,
	0x19,0x3400,
	0x18,0x0d14,
	0x19,0x8300,
	0x18,0x0d24,
	0x19,0x8200,
	0x18,0x0d34,
	0x19,0x0000,
	0x18,0x0d44,
	0x19,0x7000,
	0x18,0x0d54,
	0x19,0x0d00,
	0x18,0x0d64,
	0x19,0x0100,
	0x18,0x0d74,
	0x19,0x0f00,
	0x18,0x0d84,
	0x19,0x7f00,
	0x18,0x0d94,
	0x19,0x9a00,
	0x18,0x0da4,
	0x19,0x7d00,
	0x18,0x0db4,
	0x19,0x4000,
	0x18,0x0dc4,
	0x19,0x7f00,
	0x18,0x0dd4,
	0x19,0x1400,
	0x18,0x0de4,
	0x19,0x0000,
	0x18,0x0df4,
	0x19,0x8200,
	0x18,0x0e04,
	0x19,0x0000,
	0x18,0x0e14,
	0x19,0x7000,
	0x18,0x0e24,
	0x19,0x0f00,
	0x18,0x0e34,
	0x19,0x0100,
	0x18,0x0e44,
	0x19,0x9b00,
	0x18,0x0e54,
	0x19,0x7f00,
	0x18,0x0e64,
	0x19,0x4000,
	0x18,0x0e74,
	0x19,0x1f00,
	0x18,0x0e84,
	0x19,0x0200,
	0x18,0x0e94,
	0x19,0x0600,
	0x18,0x0ea4,
	0x19,0x7100,
	0x18,0x0eb4,
	0x19,0x1d00,
	0x18,0x0ec4,
	0x19,0x0100,
	0x18,0x0ed4,
	0x19,0x4000,
	0x18,0x0ee4,
	0x19,0x1f00,
	0x18,0x0ef4,
	0x19,0x0200,
	0x18,0x0f04,
	0x19,0x0600,
	0x18,0x0f14,
	0x19,0x7100,
	0x18,0x0f24,
	0x19,0x0d00,
	0x18,0x0f34,
	0x19,0x0100,
	0x18,0x0f44,
	0x19,0x4000,
	0x18,0x0f54,
	0x19,0x1f00,
	0x18,0x0f64,
	0x19,0x0200,
	0x18,0x0f74,
	0x19,0x0600,
	0x18,0x0f84,
	0x19,0x7100,
	0x18,0x0f94,
	0x19,0x0d00,
	0x18,0x0fa4,
	0x19,0x0100,
	0x18,0x0fb4,
	0x19,0x4000,
	0x18,0x0fc4,
	0x19,0x1f00,
	0x18,0x0fd4,
	0x19,0x0200,
	0x18,0x0fe4,
	0x19,0x0600,
	0x18,0x0ff4,
	0x19,0x7100,
	0x18,0x1004,
	0x19,0x0d00,
	0x18,0x1014,
	0x19,0x0100,
	0x18,0x1024,
	0x19,0x4000,
	0x18,0x1034,
	0x19,0x1f00,
	0x18,0x1044,
	0x19,0x0200,
	0x18,0x1054,
	0x19,0x0600,
	0x18,0x1064,
	0x19,0x7100,
	0x18,0x1074,
	0x19,0x0d00,
	0x18,0x1084,
	0x19,0x0100,
	0x18,0x1094,
	0x19,0x4000,
	0x18,0x10a4,
	0x19,0x1f00,
	0x18,0x10b4,
	0x19,0x0200,
	0x18,0x10c4,
	0x19,0x0600,
	0x18,0x10d4,
	0x19,0x7100,
	0x18,0x10e4,
	0x19,0x0d00,
	0x18,0x10f4,
	0x19,0x0100,
	0x18,0x1104,
	0x19,0x4000,
	0x18,0x1114,
	0x19,0x7f00,
	0x18,0x1124,
	0x19,0x0400,
	0x18,0x1134,
	0x19,0x9000,
	0x18,0x1144,
	0x19,0x0200,
	0x18,0x1154,
	0x19,0x0600,
	0x18,0x1164,
	0x19,0x7300,
	0x18,0x1174,
	0x19,0x0d00,
	0x18,0x1184,
	0x19,0x0100,
	0x18,0x1194,
	0x19,0x0b00,
	0x18,0x11a4,
	0x19,0x9500,
	0x18,0x11b4,
	0x19,0x9400,
	0x18,0x11c4,
	0x19,0x0400,
	0x18,0x11d4,
	0x19,0x4000,
	0x18,0x11e4,
	0x19,0x4000,
	0x18,0x11f4,
	0x19,0x0500,
	0x18,0x1204,
	0x19,0x8000,
	0x18,0x1214,
	0x19,0x7800,
	0x18,0x1224,
	0x19,0x4000,
	0x18,0x1234,
	0x19,0x7f00,
	0x18,0x1244,
	0x19,0x0400,
	0x18,0x1254,
	0x19,0x0000,
	0x18,0x1264,
	0x19,0x0200,
	0x18,0x1274,
	0x19,0x0000,
	0x18,0x1284,
	0x19,0x7000,
	0x18,0x1294,
	0x19,0x0f00,
	0x18,0x12a4,
	0x19,0x0100,
	0x18,0x12b4,
	0x19,0x9b00,
	0x18,0x12c4,
	0x19,0x7f00,
	0x18,0x12d4,
	0x19,0xe100,
	0x18,0x12e4,
	0x19,0x1000,
	0x18,0x12f4,
	0x19,0x4000,
	0x18,0x1304,
	0x19,0x7f00,
	0x18,0x1314,
	0x19,0x0500,
	0x18,0x1324,
	0x19,0x0000,
	0x18,0x1334,
	0x19,0x0000,
	0x18,0x1344,
	0x19,0x0600,
	0x18,0x1354,
	0x19,0x7300,
	0x18,0x1364,
	0x19,0x0d00,
	0x18,0x1374,
	0x19,0x0100,
	0x18,0x1384,
	0x19,0x0400,
	0x18,0x1394,
	0x19,0x0600,
	0x18,0x13a4,
	0x19,0x4000,
	0x18,0x13b4,
	0x19,0x4000,
	0x18,0x13c4,
	0x19,0x0400,
	0x18,0x13d4,
	0x19,0xe000,
	0x18,0x13e4,
	0x19,0x7d00,
	0x18,0x13f4,
	0x19,0x0500,
	0x18,0x1404,
	0x19,0x7800,
	0x18,0x1414,
	0x19,0x4000,
	0x18,0x1424,
	0x19,0x4000,
	0x18,0x1434,
	0x19,0x0400,
	0x18,0x1444,
	0x19,0xe000,
	0x18,0x1454,
	0x19,0x9700,
	0x18,0x1464,
	0x19,0x4000,
	0x18,0x1474,
	0x19,0x7f00,
	0x18,0x1484,
	0x19,0x0000,
	0x18,0x1494,
	0x19,0x0100,
	0x18,0x14a4,
	0x19,0x4400,
	0x18,0x14b4,
	0x19,0x0000,
	0x18,0x14c4,
	0x19,0x0000,
	0x18,0x14d4,
	0x19,0x0000,
	0x18,0x14e4,
	0x19,0x4000,
	0x18,0x14f4,
	0x19,0x8000,
	0x18,0x1504,
	0x19,0x7f00,
	0x18,0x1514,
	0x19,0x4000,
	0x18,0x1524,
	0x19,0x3f00,
	0x18,0x1534,
	0x19,0x0400,
	0x18,0x1544,
	0x19,0x5000,
	0x18,0x1554,
	0x19,0xf800,
	0x18,0x1564,
	0x19,0x0000,
	0x18,0x1574,
	0x19,0xe000,
	0x18,0x1584,
	0x19,0x4000,
	0x18,0x1594,
	0x19,0x8000,
	0x18,0x15a4,
	0x19,0x7f00,
	0x18,0x15b4,
	0x19,0x8900,
	0x18,0x15c4,
	0x19,0x8300,
	0x18,0x15d4,
	0x19,0xe000,
	0x18,0x15e4,
	0x19,0x0000,
	0x18,0x15f4,
	0x19,0x4000,
	0x18,0x1604,
	0x19,0x7f00,
	0x18,0x1614,
	0x19,0x0200,
	0x18,0x1624,
	0x19,0x1000,
	0x18,0x1634,
	0x19,0x0000,
	0x18,0x1644,
	0x19,0xfc00,
	0x18,0x1654,
	0x19,0xfd00,
	0x18,0x1664,
	0x19,0x0000,
	0x18,0x1674,
	0x19,0x4000,
	0x18,0x1684,
	0x19,0xbc00,
	0x18,0x1694,
	0x19,0x0e00,
	0x18,0x16a4,
	0x19,0xfe00,
	0x18,0x16b4,
	0x19,0x8a00,
	0x18,0x16c4,
	0x19,0x8300,
	0x18,0x16d4,
	0x19,0xe000,
	0x18,0x16e4,
	0x19,0x0000,
	0x18,0x16f4,
	0x19,0x4000,
	0x18,0x1704,
	0x19,0x7f00,
	0x18,0x1714,
	0x19,0x0100,
	0x18,0x1724,
	0x19,0xff00,
	0x18,0x1734,
	0x19,0x0000,
	0x18,0x1744,
	//### 0x2219 : 0xfc00 ##0309
	0x19,0xfc00,
	0x18,0x1754,
	//### 0x2219 : 0xff00 ##0309
	0x19,0xff00,
	0x18,0x1764,
	0x19,0x0000,
	0x18,0x1774,
	0x19,0x4000,
	0x18,0x1784,
	0x19,0x9d00,
	0x18,0x1794,
	0x19,0xff00,
	0x18,0x17a4,
	0x19,0x4000,
	0x18,0x17b4,
	0x19,0x7f00,
	0x18,0x17c4,
	0x19,0x0000,
	0x18,0x17d4,
	0x19,0xff00,
	0x18,0x17e4,
	0x19,0x0000,
	0x18,0x17f4,
	0x19,0xfc00,
	0x18,0x1804,
	0x19,0xff00,
	0x18,0x1814,
	0x19,0x0000,
	0x18,0x1824,
	0x19,0x4000,
	0x18,0x1834,
	0x19,0x8900,
	0x18,0x1844,
	0x19,0x8300,
	0x18,0x1854,
	0x19,0xe000,
	0x18,0x1864,
	0x19,0x0000,
	0x18,0x1874,
	0x19,0xac00,
	0x18,0x1884,
	0x19,0x0800,
	0x18,0x1894,
	0x19,0xfa00,
	0x18,0x18a4,
	0x19,0x4000,
	0x18,0x18b4,
	0x19,0x7f00,
	0x18,0x18c4,
	0x19,0x0400,
	0x18,0x18d4,
	0x19,0x0200,
	0x18,0x18e4,
	0x19,0x0000,
	0x18,0x18f4,
	0x19,0xfd00,
	0x18,0x1904,
	0x19,0x7f00,
	0x18,0x1914,
	0x19,0x0000,
	0x18,0x1924,
	0x19,0x4000,
	0x18,0x1934,
	0x19,0x0500,
	0x18,0x1944,
	0x19,0x9000,
	0x18,0x1954,
	0x19,0x8000,
	0x18,0x1964,
	0x19,0x7d00,
	0x18,0x1974,
	0x19,0x8c00,
	0x18,0x1984,
	0x19,0x8300,
	0x18,0x1994,
	0x19,0xe000,
	0x18,0x19a4,
	0x19,0x0000,
	0x18,0x19b4,
	0x19,0x4000,
	0x18,0x19c4,
	0x19,0x0400,
	0x18,0x19d4,
	0x19,0xff00,
	0x18,0x19e4,
	0x19,0x0500,
	0x18,0x19f4,
	0x19,0x8500,
	0x18,0x1a04,
	0x19,0x8c00,
	0x18,0x1a14,
	0x19,0xfa00,
	0x18,0x1a24,
	0x19,0xe000,
	0x18,0x1a34,
	0x19,0x0000,
	0x18,0x1a44,
	0x19,0x4000,
	0x18,0x1a54,
	0x19,0x5f00,
	0x18,0x1a64,
	0x19,0x0400,
	0x18,0x1a74,
	0x19,0x0000,
	0x18,0x1a84,
	0x19,0xfc00,
	0x18,0x1a94,
	0x19,0x7300,
	0x18,0x1aa4,
	0x19,0x0d00,
	0x18,0x1ab4,
	0x19,0x0100,
	0x18,0x1ac4,
	0x19,0x4000,
	0x18,0x1ad4,
	0x19,0x2000,
	0x18,0x1ae4,
	0x19,0x0000,
	0x18,0x1af4,
	0x19,0x0400,
	0x18,0x1b04,
	0x19,0xda00,
	0x18,0x1b14,
	0x19,0x0600,
	0x18,0x1b24,
	0x19,0x7d00,
	0x18,0x1b34,
	0x19,0x4000,
	0x18,0x1b44,
	0x19,0x5f00,
	0x18,0x1b54,
	0x19,0x0400,
	0x18,0x1b64,
	0x19,0x0000,
	0x18,0x1b74,
	0x19,0x0000,
	0x18,0x1b84,
	0x19,0x7300,
	0x18,0x1b94,
	0x19,0x0d00,
	0x18,0x1ba4,
	0x19,0x0100,
	0x18,0x1bb4,
	0x19,0x0400,
	0x18,0x1bc4,
	0x19,0xce00,
	0x18,0x1bd4,
	0x19,0x0800,
	0x18,0x1be4,
	0x19,0x9200,
	0x18,0x1bf4,
	0x19,0x0900,
	0x18,0x1c04,
	0x19,0x9b00,
	0x18,0x1c14,
	0x19,0x0700,
	0x18,0x1c24,
	0x19,0x7900,
	0x18,0x1c34,
	0x19,0x4000,
	0x18,0x1c44,
	0x19,0x7f00,
	0x18,0x1c54,
	0x19,0x0400,
	0x18,0x1c64,
	0x19,0x0000,
	0x18,0x1c74,
	0x19,0x0000,
	0x18,0x1c84,
	0x19,0x0400,
	0x18,0x1c94,
	0x19,0x7300,
	0x18,0x1ca4,
	0x19,0x0d00,
	0x18,0x1cb4,
	0x19,0x0100,
	0x18,0x1cc4,
	0x19,0x0900,
	0x18,0x1cd4,
	0x19,0x8e00,
	0x18,0x1ce4,
	0x19,0x0800,
	0x18,0x1cf4,
	0x19,0x7d00,
	0x18,0x1d04,
	0x19,0x4000,
	0x18,0x1d14,
	0x19,0x7f00,
	0x18,0x1d24,
	0x19,0x0000,
	0x18,0x1d34,
	0x19,0x0000,
	0x18,0x1d44,
	0x19,0x0000,
	0x18,0x1d54,
	0x19,0x0000,
	0x18,0x1d64,
	0x19,0x7000,
	0x18,0x1d74,
	0x19,0x0c00,
	0x18,0x1d84,
	0x19,0x0100,
	0x18,0x1d94,
	0x19,0x0900,
	0x18,0x1da4,
	0x19,0x7f00,
	0x18,0x1db4,
	0x19,0x4000,
	0x18,0x1dc4,
	0x19,0x7f00,
	0x18,0x1dd4,
	0x19,0x0400,
	0x18,0x1de4,
	0x19,0x0000,
	0x18,0x1df4,
	0x19,0x0000,
	0x18,0x1e04,
	0x19,0x0000,
	0x18,0x1e14,
	0x19,0x7000,
	0x18,0x1e24,
	0x19,0x0d00,
	0x18,0x1e34,
	0x19,0x0100,
	0x18,0x1e44,
	0x19,0x0b00,
	0x18,0x1e54,
	0x19,0x7f00,
	0x18,0x1e64,
	0x19,0x9a00,
	0x18,0x1e74,
	0x19,0x7f00,
	0x18,0x1e84,
	0x19,0x4000,
	0x18,0x1e94,
	0x19,0x7f00,
	0x18,0x1ea4,
	0x19,0x0400,
	0x18,0x1eb4,
	0x19,0x0000,
	0x18,0x1ec4,
	0x19,0x0000,
	0x18,0x1ed4,
	0x19,0x0000,
	0x18,0x1ee4,
	0x19,0x7100,
	0x18,0x1ef4,
	0x19,0x0d00,
	0x18,0x1f04,
	0x19,0x0100,
	0x18,0x1f14,
	0x19,0x9400,
	0x18,0x1f24,
	0x19,0x7f00,
	0x18,0x1f34,
	0x19,0x4000,
	0x18,0x1f44,
	0x19,0x7f00,
	0x18,0x1f54,
	0x19,0x0500,
	0x18,0x1f64,
	0x19,0x0000,
	0x18,0x1f74,
	0x19,0x0000,
	0x18,0x1f84,
	0x19,0x0000,
	0x18,0x1f94,
	0x19,0x7300,
	0x18,0x1fa4,
	0x19,0x0d00,
	0x18,0x1fb4,
	0x19,0x0100,
	0x18,0x1fc4,
	0x19,0x0500,
	0x18,0x1fd4,
	0x19,0x8800,
	0x18,0x1fe4,
	0x19,0x0400,
	0x18,0x1ff4,
	0x19,0x7d00,
	0x18,0x2004,
	0x19,0x4000,
	0x18,0x2014,
	0x19,0x4000,
	0x18,0x2024,
	0x19,0x0400,
	0x18,0x2034,
	0x19,0xe100,
	0x18,0x2044,
	0x19,0x8a00,
	0x18,0x2054,
	0x19,0x4000,
	0x18,0x2064,
	0x19,0x4000,
	0x18,0x2074,
	0x19,0x0400,
	0x18,0x2084,
	0x19,0xe100,
	0x18,0x2094,
	0x19,0xa400,
	0x18,0x20a4,
	0x19,0x4000,
	0x18,0x20b4,
	0x19,0x7f00,
	0x18,0x20c4,
	0x19,0x0000,
	0x18,0x20d4,
	0x19,0x0100,
	0x18,0x20e4,
	0x19,0x4000,
	0x18,0x20f4,
	0x19,0x3e00,
	0x18,0x2104,
	0x19,0x0000,
	0x18,0x2114,
	0x19,0xe000,
	0x18,0x2124,
	0x19,0x1200,
	0x18,0x2134,
	0x19,0x8000,
	0x18,0x2144,
	0x19,0x7f00,
	0x18,0x2154,
	0x19,0x8900,
	0x18,0x2164,
	0x19,0x8300,
	0x18,0x2174,
	0x19,0xe000,
	0x18,0x2184,
	0x19,0x0000,
	0x18,0x2194,
	0x19,0x4000,
	0x18,0x21a4,
	0x19,0x7f00,
	0x18,0x21b4,
	0x19,0x0000,
	0x18,0x21c4,
	0x19,0x2000,
	0x18,0x21d4,
	0x19,0x4000,
	0x18,0x21e4,
	0x19,0x3e00,
	0x18,0x21f4,
	0x19,0xff00,
	0x18,0x2204,
	0x19,0x0000,
	0x18,0x2214,
	0x19,0x1200,
	0x18,0x2224,
	0x19,0x8000,
	0x18,0x2234,
	0x19,0x7f00,
	0x18,0x2244,
	0x19,0x8600,
	0x18,0x2254,
	0x19,0x8500,
	0x18,0x2264,
	0x19,0x8900,
	0x18,0x2274,
	0x19,0xfd00,
	0x18,0x2284,
	0x19,0xe000,
	0x18,0x2294,
	0x19,0x0000,
	0x18,0x22a4,
	0x19,0x9500,
	0x18,0x22b4,
	0x19,0x0400,
	0x18,0x22c4,
	0x19,0x4000,
	0x18,0x22d4,
	0x19,0x4000,
	0x18,0x22e4,
	0x19,0x1000,
	0x18,0x22f4,
	0x19,0x4000,
	0x18,0x2304,
	0x19,0x3f00,
	0x18,0x2314,
	0x19,0x0200,
	0x18,0x2324,
	0x19,0x4000,
	0x18,0x2334,
	0x19,0x3700,
	0x18,0x2344,
	0x19,0x7f00,
	0x18,0x2354,
	0x19,0x0000,
	0x18,0x2364,
	0x19,0x0200,
	0x18,0x2374,
	0x19,0x0200,
	0x18,0x2384,
	0x19,0x9000,
	0x18,0x2394,
	0x19,0x8000,
	0x18,0x23a4,
	0x19,0x7d00,
	0x18,0x23b4,
	0x19,0x8900,
	0x18,0x23c4,
	0x19,0x8300,
	0x18,0x23d4,
	0x19,0xe000,
	0x18,0x23e4,
	0x19,0x0000,
	0x18,0x23f4,
	0x19,0x4000,
	0x18,0x2404,
	0x19,0x0400,
	0x18,0x2414,
	0x19,0xff00,
	0x18,0x2424,
	0x19,0x0200,
	0x18,0x2434,
	0x19,0x8500,
	0x18,0x2444,
	0x19,0x8900,
	0x18,0x2454,
	0x19,0xfa00,
	0x18,0x2464,
	0x19,0xe000,
	0x18,0x2474,
	0x19,0x0000,
	0x18,0x2484,
	0x19,0x4000,
	0x18,0x2494,
	0x19,0x7f00,
	0x18,0x24a4,
	0x19,0x0000,
	0x18,0x24b4,
	0x19,0x0000,
	0x18,0x24c4,
	0x19,0x4000,
	0x18,0x24d4,
	0x19,0x3700,
	0x18,0x24e4,
	0x19,0x7300,
	0x18,0x24f4,
	0x19,0x0500,
	0x18,0x2504,
	0x19,0x0200,
	0x18,0x2514,
	0x19,0x0100,
	0x18,0x2524,
	0x19,0xd800,
	0x18,0x2534,
	0x19,0x0400,
	0x18,0x2544,
	0x19,0x7d00,
	0x18,0x2554,
	0x19,0x4000,
	0x18,0x2564,
	0x19,0x7f00,
	0x18,0x2574,
	0x19,0x0000,
	0x18,0x2584,
	0x19,0x0000,
	0x18,0x2594,
	0x19,0x4000,
	0x18,0x25a4,
	0x19,0x0000,
	0x18,0x25b4,
	0x19,0x7200,
	0x18,0x25c4,
	0x19,0x0400,
	0x18,0x25d4,
	0x19,0x0000,
	0x18,0x25e4,
	0x19,0x0800,
	0x18,0x25f4,
	0x19,0x7f00,
	0x18,0x2604,
	0x19,0x4000,
	0x18,0x2614,
	0x19,0x7f00,
	0x18,0x2624,
	0x19,0x0000,
	0x18,0x2634,
	0x19,0x0000,
	0x18,0x2644,
	0x19,0xc000,
	0x18,0x2654,
	0x19,0x0000,
	0x18,0x2664,
	0x19,0x7200,
	0x18,0x2674,
	0x19,0x0500,
	0x18,0x2684,
	0x19,0x0000,
	0x18,0x2694,
	0x19,0x0400,
	0x18,0x26a4,
	0x19,0xeb00,
	0x18,0x26b4,
	0x19,0x8400,
	0x18,0x26c4,
	0x19,0x7d00,
	0x18,0x26d4,
	0x19,0x4000,
	0x18,0x26e4,
	0x19,0x7f00,
	0x18,0x26f4,
	0x19,0x0000,
	0x18,0x2704,
	0x19,0x0000,
	0x18,0x2714,
	0x19,0x4000,
	0x18,0x2724,
	0x19,0x0000,
	0x18,0x2734,
	0x19,0x7200,
	0x18,0x2744,
	0x19,0x0700,
	0x18,0x2754,
	0x19,0x0000,
	0x18,0x2764,
	0x19,0x0400,
	0x18,0x2774,
	0x19,0xde00,
	0x18,0x2784,
	0x19,0x9b00,
	0x18,0x2794,
	0x19,0x7d00,
	0x18,0x27a4,
	0x19,0x4000,
	0x18,0x27b4,
	0x19,0x7f00,
	0x18,0x27c4,
	0x19,0x0000,
	0x18,0x27d4,
	0x19,0x9000,
	0x18,0x27e4,
	0x19,0x4000,
	0x18,0x27f4,
	0x19,0x0400,
	0x18,0x2804,
	0x19,0x7300,
	0x18,0x2814,
	0x19,0x1500,
	0x18,0x2824,
	0x19,0x0000,
	0x18,0x2834,
	0x19,0x0400,
	0x18,0x2844,
	0x19,0xd100,
	0x18,0x2854,
	0x19,0x9400,
	0x18,0x2864,
	0x19,0x9200,
	0x18,0x2874,
	0x19,0x8000,
	0x18,0x2884,
	0x19,0x7b00,
	0x18,0x2894,
	0x19,0x4000,
	0x18,0x28a4,
	0x19,0x7f00,
	0x18,0x28b4,
	0x19,0x0000,
	0x18,0x28c4,
	0x19,0x0000,
	0x18,0x28d4,
	0x19,0x4000,
	0x18,0x28e4,
	0x19,0x0000,
	0x18,0x28f4,
	0x19,0x7200,
	0x18,0x2904,
	0x19,0x0700,
	0x18,0x2914,
	0x19,0x0000,
	0x18,0x2924,
	0x19,0x0400,
	0x18,0x2934,
	0x19,0xc200,
	0x18,0x2944,
	0x19,0x9b00,
	0x18,0x2954,
	0x19,0x7d00,
	0x18,0x2964,
	0x19,0xe200,
	0x18,0x2974,
	0x19,0x7a00,
	0x18,0x2984,
	0x19,0x4000,
	0x18,0x2994,
	0x19,0x7f00,
	0x18,0x29a4,
	0x19,0x0000,
	0x18,0x29b4,
	0x19,0x0000,
	0x18,0x29c4,
	0x19,0x4000,
	0x18,0x29d4,
	0x19,0x3700,
	0x18,0x29e4,
	0x19,0x7300,
	0x18,0x29f4,
	0x19,0x0500,
	0x18,0x2a04,
	0x19,0x0000,
	0x18,0x2a14,
	0x19,0x0100,
	0x18,0x2a24,
	0x19,0x0300,
	0x18,0x2a34,
	0x19,0xe200,
	0x18,0x2a44,
	0x19,0x2a00,
	0x18,0x2a54,
	0x19,0x0200,
	0x18,0x2a64,
	0x19,0x7b00,
	0x18,0x2a74,
	0x19,0xe200,
	0x18,0x2a84,
	0x19,0x4800,
	//### endpoint

	0x1f,0x0000,
	0x17,0x2100,
	0x1f,0x0007,
	0x1e,0x0040,
	0x18,0x0000,
	//### end of inrx dspctl patch code

	//### inrx eyesch patch code
	0x1f,0x0007,
	0x1e,0x0042,
	0x15,0x0f00,
	0x1f,0x0000,
	0x17,0x2160,
	0x1f,0x0001,
	0x10,0xf25e,
	0x1f,0x0007,
	0x1e,0x0042,

	//### startpoint
	0x15,0x0f00,
	0x16,0x7408,
	0x15,0x0e00,
	0x15,0x0f00,
	0x15,0x0f01,
	0x16,0x4000,
	0x15,0x0e01,
	0x15,0x0f01,
	0x15,0x0f02,
	0x16,0x9400,
	0x15,0x0e02,
	0x15,0x0f02,
	0x15,0x0f03,
	0x16,0x7408,
	0x15,0x0e03,
	0x15,0x0f03,
	0x15,0x0f04,
	0x16,0x4008,
	0x15,0x0e04,
	0x15,0x0f04,
	0x15,0x0f05,
	0x16,0x9400,
	0x15,0x0e05,
	0x15,0x0f05,
	0x15,0x0f06,
	0x16,0x0803,
	0x15,0x0e06,
	0x15,0x0f06,
	//### endpoint

	0x1f, 0x0001,
	0x10, 0xf05e,
	0x1f, 0x0007,
	0x1e, 0x0042,
	0x15,0x0d00,
	0x15,0x0100,
	0x1f,0x0000,
	0x17,0x2100,
	//### end of inrx eyesch patch code

	//### release MDI/MDIX force mode
	0x1f, 0x0007,
	0x1e, 0x002d,
	0x18, 0xf010,

	//### uc patch code (20110103 add foce giga mode)
	0x1f,0x0005,
	//### startpoint

	5, 0x8000,
	6, 0xeeff,
	6, 0xfc8b,
	6, 0xeeff,
	6, 0xfda0,
	6, 0x0280,
	6, 0x33f7,
	6, 0x00e0,
	6, 0xfff7,
	6, 0xa080,
	6, 0x02ae,
	6, 0xf602,
	6, 0x842b,
	6, 0x0201,
	6, 0x4802,
	6, 0x015b,
	6, 0x0280,
	6, 0xabe0,
	6, 0x8b8c,
	6, 0xe18b,
	6, 0x8d1e,
	6, 0x01e1,
	6, 0x8b8e,
	6, 0x1e01,
	6, 0xa000,
	6, 0xe4ae,
	6, 0xd8bf,
	6, 0x846c,
	6, 0xd785,
	6, 0x80d0,
	6, 0x6c02,
	6, 0x28b4,
	6, 0xeee1,
	6, 0x4477,
	6, 0xeee1,
	6, 0x4565,
	6, 0xee8b,
	6, 0x85c2,
	6, 0xee8a,
	6, 0xe86e,
	6, 0xee85,
	6, 0x7100,
	6, 0xee85,
	6, 0x7200,
	6, 0xee85,
	6, 0x7302,
	6, 0xee85,
	6, 0x7a03,
	6, 0xee85,
	6, 0x7bb8,
	6, 0xee85,
	6, 0x7400,
	6, 0xee85,
	6, 0x7500,
	6, 0xee85,
	6, 0x7000,
	6, 0xd407,
	6, 0xf7e4,
	6, 0x8b96,
	6, 0xe58b,
	6, 0x97d4,
	6, 0x0802,
	6, 0xe48b,
	6, 0x94e5,
	6, 0x8b95,
	6, 0xd100,
	6, 0xbf84,
	6, 0x5d02,
	6, 0x2959,
	6, 0xbf8b,
	6, 0x88ec,
	6, 0x0019,
	6, 0xa98b,
	6, 0x90f9,
	6, 0xeeff,
	6, 0xf600,
	6, 0xeeff,
	6, 0xf7fc,
	6, 0xe0e1,
	6, 0x40e1,
	6, 0xe141,
	6, 0xf72f,
	6, 0xf628,
	6, 0xe4e1,
	6, 0x40e5,
	6, 0xe141,
	6, 0x04f8,
	6, 0xe08b,
	6, 0x8ead,
	6, 0x2017,
	6, 0xf620,
	6, 0xe48b,
	6, 0x8e02,
	6, 0x25da,
	6, 0x0226,
	6, 0xb402,
	6, 0x8169,
	6, 0x0202,
	6, 0x3402,
	6, 0x82b1,
	6, 0x0283,
	6, 0x4ae0,
	6, 0x8b8e,
	6, 0xad23,
	6, 0x05f6,
	6, 0x23e4,
	6, 0x8b8e,
	6, 0xe08b,
	6, 0x8ead,
	6, 0x2408,
	6, 0xf624,
	6, 0xe48b,
	6, 0x8e02,
	6, 0x277d,
	6, 0xe08b,
	6, 0x8ead,
	6, 0x260b,
	6, 0xf626,
	6, 0xe48b,
	6, 0x8e02,
	6, 0x056e,
	6, 0x021c,
	6, 0x9a02,
	6, 0x80fb,
	6, 0x0281,
	6, 0x55fc,
	6, 0x04f8,
	6, 0xe08b,
	6, 0x83ad,
	6, 0x2321,
	6, 0xe0e0,
	6, 0x22e1,
	6, 0xe023,
	6, 0xad29,
	6, 0x20e0,
	6, 0x8b83,
	6, 0xad21,
	6, 0x06e1,
	6, 0x8b84,
	6, 0xad28,
	6, 0x3ce0,
	6, 0x8b85,
	6, 0xad21,
	6, 0x06e1,
	6, 0x8b84,
	6, 0xad29,
	6, 0x30bf,
	6, 0x3144,
	6, 0x0228,
	6, 0xe8ae,
	6, 0x28ee,
	6, 0x8ae2,
	6, 0x00ee,
	6, 0x8ae3,
	6, 0x00ee,
	6, 0x8ae4,
	6, 0x00ee,
	6, 0x8ae5,
	6, 0x00ee,
	6, 0x8b72,
	6, 0x00e0,
	6, 0x8b83,
	6, 0xad21,
	6, 0x08e0,
	6, 0x8b84,
	6, 0xf620,
	6, 0xe48b,
	6, 0x84bf,
	6, 0x3147,
	6, 0x0228,
	6, 0xe8fc,
	6, 0x04f8,
	6, 0xe0e0,
	6, 0x38e1,
	6, 0xe039,
	6, 0xac2e,
	6, 0x08ee,
	6, 0xe08e,
	6, 0x36ee,
	6, 0xe08f,
	6, 0x20fc,
	6, 0x04f8,
	6, 0xfaef,
	6, 0x69e0,
	6, 0x8b85,
	6, 0xad21,
	6, 0x39e0,
	6, 0xe022,
	6, 0xe1e0,
	6, 0x2358,
	6, 0xc059,
	6, 0x021e,
	6, 0x01e1,
	6, 0x8b72,
	6, 0x1f10,
	6, 0x9e26,
	6, 0xe48b,
	6, 0x72ad,
	6, 0x211d,
	6, 0xe18b,
	6, 0x84f7,
	6, 0x29e5,
	6, 0x8b84,
	6, 0xac27,
	6, 0x0dac,
	6, 0x2605,
	6, 0x0204,
	6, 0xa2ae,
	6, 0x0d02,
	6, 0x820c,
	6, 0xae08,
	6, 0x0282,
	6, 0x38ae,
	6, 0x0302,
	6, 0x81b1,
	6, 0xef96,
	6, 0xfefc,
	6, 0x04d1,
	6, 0x00bf,
	6, 0x845d,
	6, 0x0229,
	6, 0x59d1,
	6, 0x03bf,
	6, 0x316b,
	6, 0x0229,
	6, 0x59d1,
	6, 0x00bf,
	6, 0x316e,
	6, 0x0229,
	6, 0x59d1,
	6, 0x00bf,
	6, 0x8463,
	6, 0x0229,
	6, 0x59d1,
	6, 0x0fbf,
	6, 0x3162,
	6, 0x0229,
	6, 0x59d1,
	6, 0x01bf,
	6, 0x3165,
	6, 0x0229,
	6, 0x59d1,
	6, 0x01bf,
	6, 0x3168,
	6, 0x0229,
	6, 0x59e0,
	6, 0x8b85,
	6, 0xad24,
	6, 0x0ed1,
	6, 0x00bf,
	6, 0x3177,
	6, 0xad23,
	6, 0x03bf,
	6, 0x317a,
	6, 0x0229,
	6, 0x59e0,
	6, 0x8b83,
	6, 0xad22,
	6, 0x08d1,
	6, 0x00bf,
	6, 0x315c,
	6, 0x0229,
	6, 0x5904,
	6, 0xd102,
	6, 0xbf31,
	6, 0x6b02,
	6, 0x2959,
	6, 0xd106,
	6, 0xbf31,
	6, 0x6202,
	6, 0x2959,
	6, 0xd107,
	6, 0xbf31,
	6, 0x6502,
	6, 0x2959,
	6, 0xd107,
	6, 0xbf31,
	6, 0x6802,
	6, 0x2959,
	6, 0xd101,
	6, 0xbf84,
	6, 0x5d02,
	6, 0x2959,
	6, 0x0282,
	6, 0x7104,
	6, 0xd102,
	6, 0xbf31,
	6, 0x6b02,
	6, 0x2959,
	6, 0xd101,
	6, 0xbf84,
	6, 0x5d02,
	6, 0x2959,
	6, 0xd011,
	6, 0x0228,
	6, 0x0459,
	6, 0x03ef,
	6, 0x01d1,
	6, 0x00a0,
	6, 0x0002,
	6, 0xd101,
	6, 0xbf31,
	6, 0x6e02,
	6, 0x2959,
	6, 0xd111,
	6, 0xad20,
	6, 0x020c,
	6, 0x11ad,
	6, 0x2102,
	6, 0x0c12,
	6, 0xbf84,
	6, 0x6302,
	6, 0x2959,
	6, 0x04f8,
	6, 0xe08b,
	6, 0x85ad,
	6, 0x2437,
	6, 0xe0ea,
	6, 0xcae1,
	6, 0xeacb,
	6, 0xad29,
	6, 0x2ee0,
	6, 0xeacc,
	6, 0xe1ea,
	6, 0xcdad,
	6, 0x2925,
	6, 0xe0e0,
	6, 0x08e1,
	6, 0xe009,
	6, 0xad20,
	6, 0x1ce0,
	6, 0xe00a,
	6, 0xe1e0,
	6, 0x0bad,
	6, 0x2013,
	6, 0xd103,
	6, 0xbf31,
	6, 0x77e0,
	6, 0x8b85,
	6, 0xad23,
	6, 0x05d1,
	6, 0x01bf,
	6, 0x317a,
	6, 0x0229,
	6, 0x59fc,
	6, 0x04f8,
	6, 0xf9fb,
	6, 0xe08b,
	6, 0x85ad,
	6, 0x2522,
	6, 0xe0e0,
	6, 0x22e1,
	6, 0xe023,
	6, 0xe2e0,
	6, 0x36e3,
	6, 0xe037,
	6, 0x5ac4,
	6, 0x0d01,
	6, 0x5802,
	6, 0x1e20,
	6, 0xe385,
	6, 0x71ac,
	6, 0x3160,
	6, 0xac3a,
	6, 0x08ac,
	6, 0x3e26,
	6, 0xae67,
	6, 0xaf83,
	6, 0x46ad,
	6, 0x3761,
	6, 0xe085,
	6, 0x7210,
	6, 0xe485,
	6, 0x72e1,
	6, 0x8573,
	6, 0x1b10,
	6, 0x9e02,
	6, 0xae51,
	6, 0xd100,
	6, 0xbf84,
	6, 0x6002,
	6, 0x2959,
	6, 0xee85,
	6, 0x7200,
	6, 0xae43,
	6, 0xad36,
	6, 0x27e0,
	6, 0x857a,
	6, 0xe185,
	6, 0x7bef,
	6, 0x74e0,
	6, 0x8574,
	6, 0xe185,
	6, 0x751b,
	6, 0x749e,
	6, 0x2e14,
	6, 0xe485,
	6, 0x74e5,
	6, 0x8575,
	6, 0xef74,
	6, 0xe085,
	6, 0x7ae1,
	6, 0x857b,
	6, 0x1b47,
	6, 0x9e0f,
	6, 0xae19,
	6, 0xee85,
	6, 0x7400,
	6, 0xee85,
	6, 0x7500,
	6, 0xae0f,
	6, 0xac39,
	6, 0x0cd1,
	6, 0x01bf,
	6, 0x8460,
	6, 0x0229,
	6, 0x59ee,
	6, 0x8572,
	6, 0x00e6,
	6, 0x8571,
	6, 0xfffd,
	6, 0xfc04,
	6, 0xf8e0,
	6, 0x8b85,
	6, 0xad27,
	6, 0x30e0,
	6, 0xe036,
	6, 0xe1e0,
	6, 0x37e1,
	6, 0x8570,
	6, 0x1f10,
	6, 0x9e23,
	6, 0xe485,
	6, 0x70ac,
	6, 0x200b,
	6, 0xac21,
	6, 0x0dac,
	6, 0x250f,
	6, 0xac27,
	6, 0x11ae,
	6, 0x1202,
	6, 0x8383,
	6, 0xae0d,
	6, 0x0283,
	6, 0x88ae,
	6, 0x0802,
	6, 0x838f,
	6, 0xae03,
	6, 0x0283,
	6, 0xa2fc,
	6, 0x04ee,
	6, 0x8576,
	6, 0x0004,
	6, 0x0283,
	6, 0xaf02,
	6, 0x83f2,
	6, 0x04f8,
	6, 0xf9e0,
	6, 0x8b87,
	6, 0xad26,
	6, 0x08d1,
	6, 0x01bf,
	6, 0x8469,
	6, 0x0229,
	6, 0x59fd,
	6, 0xfc04,
	6, 0x0284,
	6, 0x1304,
	6, 0xee85,
	6, 0x7600,
	6, 0xee85,
	6, 0x7702,
	6, 0x04f8,
	6, 0xf9e0,
	6, 0x8b85,
	6, 0xad26,
	6, 0x38d0,
	6, 0x0b02,
	6, 0x2804,
	6, 0x5882,
	6, 0x7882,
	6, 0x9f2d,
	6, 0xe085,
	6, 0x76e1,
	6, 0x8577,
	6, 0x1f10,
	6, 0x9eb5,
	6, 0x10e4,
	6, 0x8576,
	6, 0xe0e0,
	6, 0x00e1,
	6, 0xe001,
	6, 0xf727,
	6, 0xe4e0,
	6, 0x00e5,
	6, 0xe001,
	6, 0xe2e0,
	6, 0x20e3,
	6, 0xe021,
	6, 0xad30,
	6, 0xf7f6,
	6, 0x27e4,
	6, 0xe000,
	6, 0xe5e0,
	6, 0x01fd,
	6, 0xfc04,
	6, 0xf8fa,
	6, 0xef69,
	6, 0xe08b,
	6, 0x86ad,
	6, 0x2212,
	6, 0xe0e0,
	6, 0x14e1,
	6, 0xe015,
	6, 0xad26,
	6, 0x89e1,
	6, 0x8578,
	6, 0xbf84,
	6, 0x6602,
	6, 0x2959,
	6, 0xef96,
	6, 0xfefc,
	6, 0x04f8,
	6, 0xfaef,
	6, 0x69e0,
	6, 0x8b86,
	6, 0xad22,
	6, 0x09e1,
	6, 0x8579,
	6, 0xbf84,
	6, 0x6602,
	6, 0x2959,
	6, 0xef96,
	6, 0xfefc,
	6, 0x04f8,
	6, 0xfaef,
	6, 0x69e0,
	6, 0x8b86,
	6, 0xac20,
	6, 0x1abf,
	6, 0x8580,
	6, 0xd06c,
	6, 0x0228,
	6, 0xbbe0,
	6, 0xe0e4,
	6, 0xe1e0,
	6, 0xe558,
	6, 0x0668,
	6, 0xc0d1,
	6, 0xd2e4,
	6, 0xe0e4,
	6, 0xe5e0,
	6, 0xe5ef,
	6, 0x96fe,
	6, 0xfc04,
	6, 0xa0e0,
	6, 0xeaf0,
	6, 0xe07c,
	6, 0x55e2,
	6, 0x3211,
	6, 0xe232,
	6, 0x88e2,
	6, 0x0070,
	6, 0xe426,
	6, 0x70e0,
	6, 0x7699,
	6, 0xe000,
	6, 0x2508,
	6, 0x0726,
	6, 0x4072,
	6, 0x2726,
	6, 0x7e28,
	6, 0x04b7,
	6, 0x2925,
	6, 0x762a,
	6, 0x68e5,
	6, 0x2bad,
	6, 0x002c,
	6, 0xdbf0,
	6, 0x2d67,
	6, 0xbb2e,
	6, 0x7b0f,
	6, 0x2f73,
	6, 0x6531,
	6, 0xaccc,
	6, 0x3223,
	6, 0x0033,
	6, 0x2d17,
	6, 0x347f,
	6, 0x5235,
	6, 0x1000,
	6, 0x3606,
	6, 0x0037,
	6, 0x0cc0,
	6, 0x387f,
	6, 0xce3c,
	6, 0xe5f7,
	6, 0x3d3d,
	6, 0xa465,
	6, 0x303e,
	6, 0x6700,
	6, 0x5369,
	6, 0xd20f,
	6, 0x6a01,
	6, 0x2c6c,
	6, 0x2b13,
	6, 0x6ee1,
	6, 0x006f,
	6, 0x12f7,
	6, 0x7100,
	6, 0x6b73,
	6, 0x06eb,
	6, 0x7494,
	6, 0xc776,
	6, 0x980a,
	6, 0x7750,
	6, 0x0078,
	6, 0x8a15,
	6, 0x797f,
	6, 0x6f7a,
	6, 0x06a6,
	//### endpoint

	//#unlock uc ramcode version
	5,  0xe142,
	6,  0x0701,
	5,  0xe140,
	6,  0x0405,
	15, 0x0000,

	//### end of uc patch code
	//#Enable negear EEE Nway ability autooff
	0x1f,0x0005,
	0x05,0x8b84,
	0x06,0x0026,
	0x1f,0x0000,

	//#lpi patch code-maxod-20110103
	31, 0x0007,
	30, 0x0023,
	22, 0x0006,
	//#quiet time to 22ms
	21, 0x147,
	25, 0x96,
	//#lpi_tx_wake_timer to 1.3us
	21, 0x16d,
	25, 0x26,
	22, 0x0002,
	31, 0x0000,

	//#Add by Gary for Channel Estimation fine tune 20100430
	//#Page1 Reg8 (CG_INITIAL_MASTER)
	0x1f, 0x0005,
	0x05, 0x83dd,
	0x06, 0x0574,
	//#Page1 Reg9 (CB0_INITIAL_GIGA)
	0x1f, 0x0005,
	0x05, 0x83e0,
	0x06, 0x2724,
	//#Page3 Reg26 (CG_INITIAL_SLAVE)
	0x1f, 0x0005,
	0x05, 0x843d,
	0x06, 0x06f6 ,

	0x1f, 0x0000,
	#endif
};

static const unsigned int default_val[]={
	999,0x1f,0x0002,

	2,0x11,0x7e00,

	3,0x1f,0x0002,
	3,0x17,0xff00,
	3,0x18,0x0005,
	3,0x19,0x0005,
	3,0x1a,0x0005,
	3,0x1b,0x0005,
	3,0x1c,0x0005,

	4,0x1f,0x0002,
	4,0x13,0x00aa,
	4,0x14,0x00aa,
	4,0x15,0x00aa,
	4,0x16,0x00aa,
	4,0x17,0x00aa,
	4,0x18,0x0f0a,
	4,0x19,0x50ab,
	4,0x1a,0x0000,
	4,0x1b,0x0f0f,

	999,0x1f,0x0000,
};

void Setting_RTL8198_GPHY(void)
{
	int i=0, port =0, len=0;

	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);

	if (REG32(BSP_REVR) == BSP_RTL8198_REVISION_A)
	{
		/*
		 #Access command format: phywb {all: phyID=0,1,2,3,4} {page} {RegAddr} {Bit location} {Bit value}
		 #when writing gpage 72, must do mdcmdio_cmd write $phyID 31 0x7, then mdcmdio_cmd write $phyID 30 $PageNum
		 phywb all 72 21 15-0 0x7092
		 phywb all 72 22 15-0 0x7092
		 phywb all 72 23 15-0 0x7092
		 phywb all 72 24 15-0 0x7092
		 phywb all 72 25 15-0 0x7092
		 phywb all 72 26 15-0 0x7092
		*/

		//	Set_GPHYWB(3, 2, 20, 0, 0x2000);

		Set_GPHYWB(999, 72, 21, 0, 0x7092);
		Set_GPHYWB(999, 72, 22, 0, 0x7092);
		Set_GPHYWB(999, 72, 23, 0, 0x7092);
		Set_GPHYWB(999, 72, 24, 0, 0x7092);
		Set_GPHYWB(999, 72, 25, 0, 0x7092);
		Set_GPHYWB(999, 72, 26, 0, 0x7092);

		/*
		 set PageNum 2; #All of GPHY register in the Page#2
		 #Array format = {{PhyID List1}  {RegAddr1 RegData1 RegAddr2 RegData2}, ...}

		set AFE_Reg     {{0 1 2 3 4} { 0 0x0000 1  0x065a 2 0x8c01  3  0x0428 4 0x80c8  5   0x0978  6  0x0678 7  0x3620 8 0x0000 9 0x0007 10 0x0000}
	       	               {2}         {11 0x0063 12 0xeb65 13 0x51d1 14 0x5dcb 15 0x3044 16 0x1000 17  0x7e00 18 0x0000}
	              	        {3}         {19 0x3d22 20 0x2000 21 0x6040 22 0x0000 23 0xff00 24 0x0005 25 0x0005 26 0x0005 27 0x0005 28 0x0005}
	                     	 {4}         {19 0x00aa 20 0x00aa 21 0x00aa 22 0x00aa 23 0x00aa 24 0x0f0a 25 0x5050 26 0x0000 27 0x0f0f }}
		*/

	       //phyid=all
		Set_GPHYWB(999, 2,  0, 0, 0x0000);
		Set_GPHYWB(999, 2,  1, 0, 0x065a);
		Set_GPHYWB(999, 2,  2, 0, 0x8c01);
		Set_GPHYWB(999, 2,  3, 0, 0x0428);
		Set_GPHYWB(999, 2,  4, 0, 0x80c8);
		Set_GPHYWB(999, 2,  5, 0, 0x0978);
		Set_GPHYWB(999, 2,  6, 0, 0x0678);
		Set_GPHYWB(999, 2,  7, 0, 0x3620);
		Set_GPHYWB(999, 2,  8, 0, 0x0000);
		Set_GPHYWB(999, 2,  9, 0, 0x0007);
		Set_GPHYWB(999, 2,  10, 0, 0x0000);

	       //phyid=2
		Set_GPHYWB( 2,   2, 11, 0, 0x0063);
		Set_GPHYWB( 2,   2, 12, 0, 0xeb65);
		Set_GPHYWB( 2,   2, 13, 0, 0x51d1);
		Set_GPHYWB( 2,   2, 14, 0, 0x5dcb);
		Set_GPHYWB( 2,   2, 15, 0, 0x3044);
		Set_GPHYWB( 2,   2, 16, 0, 0x1000);
		Set_GPHYWB( 2,   2, 17, 0, 0x7e00);
		Set_GPHYWB( 2,   2, 18, 0, 0x0000);

	       //phyid=3
		Set_GPHYWB( 3,   2, 19, 0, 0x3d22);
		Set_GPHYWB( 3,   2, 20, 0, 0x2000);
	   	Set_GPHYWB( 3,   2, 21, 0, 0x6040);
		Set_GPHYWB( 3,   2, 22, 0, 0x0000);
		Set_GPHYWB( 3,   2, 23, 0, 0xff00);
		Set_GPHYWB( 3,   2, 24, 0, 0x0005);
		Set_GPHYWB( 3,   2, 25, 0, 0x0005);
		Set_GPHYWB( 3,   2, 26, 0, 0x0005);
		Set_GPHYWB( 3,   2, 27, 0, 0x0005);
		Set_GPHYWB( 3,   2, 28, 0, 0x0005);

	       //phyid=4
		Set_GPHYWB( 4,   2, 19, 0, 0x00aa);
		Set_GPHYWB( 4,   2, 20, 0, 0x00aa);
		Set_GPHYWB( 4,   2, 21, 0, 0x00aa);
		Set_GPHYWB( 4,   2, 22, 0, 0x00aa);
		Set_GPHYWB( 4,   2, 23, 0, 0x00aa);
		Set_GPHYWB( 4,   2, 24, 0, 0x0f0a);
		Set_GPHYWB( 4,   2, 25, 0, 0x5050);
		Set_GPHYWB( 4,   2, 26, 0, 0x0000);
		Set_GPHYWB( 4,   2, 27, 0, 0x0f0f);

		/*
		 #=========== INRX Para. =================================

		 phywb all 0 21 0x1006
	           #dfse_mode[15:14]=3(full), Fine tune aagc_lvl_fnet[10:0]
	           phywb all 1 12 15-0 0xdbf0

	           #cb0_i_giga[12:0]
	           phywb all 1 9  15-0 0x2576
	           phywb all 1 7  15-0 0x287E
	           phywb all 1 10 15-0 0x68E5
	           phywb all 1 29 15-0 0x3DA4
	           phywb all 1 28 15-0 0xE7F7
	           phywb all 1 20 15-0 0x7F52
	           phywb all 1 24 15-0 0x7FCE
	           phywb all 1 8  15-0 0x04B7
	           phywb all 1 6  15-0 0x4072
	           phywb all 1 16 15-0 0xF05E
	           phywb all 1 27 15-0 0xB414
		*/

		Set_GPHYWB( 999,   1, 12, 0, 0xdbf0);

		Set_GPHYWB( 999,   1, 9, 0, 0x2576);
		Set_GPHYWB( 999,   1, 7, 0, 0x287E);
		Set_GPHYWB( 999,   1, 10, 0, 0x68E5);
		Set_GPHYWB( 999,   1, 29, 0, 0x3DA4);
		Set_GPHYWB( 999,   1, 28, 0, 0xE7F7);
		Set_GPHYWB( 999,   1, 20, 0, 0x7F52);
		Set_GPHYWB( 999,   1, 24, 0, 0x7FCE);
		Set_GPHYWB( 999,   1, 8, 0, 0x04B7);
		Set_GPHYWB( 999,   1, 6, 0, 0x4072);
		Set_GPHYWB( 999,   1, 16, 0, 0xF05E);
		Set_GPHYWB( 999,   1, 27, 0, 0xB414);

		/*
		 #=========== Cable Test =================================

		  phywb all 3 26 15-0 0x06A6
		  phywb all 3 16 15-0 0xF05E
		  phywb all 3 19 15-0 0x06EB
		  phywb all 3 18 15-0 0xF4D2
		  phywb all 3 14 15-0 0xE120
		  phywb all 3 0  15-0 0x7C00

		  phywb all 3 2  15-0 0x5FD0
		  phywb all 3 13 15-0 0x0207

		  #disable jabber detect
		   phywb all 0 16 15-0 0x05EF

		  #Patch for EEE GMII issue
		  phywb all 32 26 15-0 0x0103
		  phywb all 32 22 15-0 0x0004
		*/
		Set_GPHYWB( 999,   3, 26, 0, 0x06A6);
		Set_GPHYWB( 999,   3, 16, 0, 0xF05E);
		Set_GPHYWB( 999,   3, 19, 0, 0x06EB);
		Set_GPHYWB( 999,   3, 18, 0, 0xF4D2);
		Set_GPHYWB( 999,   3, 14, 0, 0xE120);
		Set_GPHYWB( 999,   3, 00, 0, 0x7C00);

		Set_GPHYWB( 999,   3, 02, 0, 0x5FD0);
		Set_GPHYWB( 999,   3, 13, 0, 0x0207);

		Set_GPHYWB( 999,   0, 16, 0, 0x05EF);

		Set_GPHYWB( 999,   3, 26, 0, 0x0103);
		Set_GPHYWB( 999,   3, 22, 0, 0x0004);

		/*
		 disable aldps_en, for power measurement
		 hywb all 44 21 15-0 0x0350
		*/
		Set_GPHYWB( 999,   44, 21, 0, 0x0350);
	}
	else
	{
		Set_GPHYWB(999, 0, 0, 0xffff-POWER_DOWN, POWER_DOWN); // set power down
	
		len=sizeof(default_val)/sizeof(unsigned int);
		for(i=0;i<len;i=i+3)
		{

			if(default_val[i]==999)
			{
				for(port=0; port<5; port++)
					rtl8651_setAsicEthernetPHYReg(port, default_val[i+1], default_val[i+2]);
			}
			else
			{
				port=default_val[i];
				rtl8651_setAsicEthernetPHYReg(port, default_val[i+1], default_val[i+2]);
			}
		}
		len=sizeof(phy_para)/sizeof(unsigned int);
		for(port=0; port<5; port++)
		{
			for(i=0;i<len;i=i+2)
			{
				rtl8651_setAsicEthernetPHYReg(port, phy_para[i], phy_para[i+1]);
			}
		}
		Set_GPHYWB( 999,   5, 5, 0, 0x8b84);
		Set_GPHYWB( 999,   5, 6, 0, 0x0006);
		Set_GPHYWB( 999,   2, 8, 0, 0x0020);

		// for the IOT issue with IC+ when EEE N-way.
		Set_GPHYWB( 999,   172, 24, 0, 0x0006);

#ifdef CONFIG_RTL_8198_ESD
		Set_GPHYWB(999, 44, 27, 0xffff-(0xf<<12), 0x4<<12);
#endif
	}

//	for(i=0; i<5; i++)
//		REG32(PCRP0+i*4) &= ~(EnForceMode);

	printk("==Set GPHY Parameter OK\n");
}

int rtl8198_force_giga(int port)
{
	if (port < 0 || port > 4)
		return 0;

	REG32(PCRP0+ port*4) |= (EnForceMode);

	rtl8651_setAsicEthernetPHYReg(port, 31, 0x5);
	rtl8651_setAsicEthernetPHYReg(port, 5, 0x8b86);
	rtl8651_setAsicEthernetPHYReg(port, 6, 0x0040);
	rtl8651_setAsicEthernetPHYReg(port, 31, 0x0);

	REG32(PCRP0+ port*4) = REG32(PCRP0+ port*4) & ~(EnForceMode | NwayAbility100MF | NwayAbility100MH | NwayAbility10MF | NwayAbility10MH); // disable Nway 10/100 ability

	rtl8651_restartAsicEthernetPHYNway(port);
	return 0;
}

int rtl8198_disable_force(int port)
{
	if (port < 0 || port > 4)
		return 0;

	REG32(PCRP0+ port*4) |= (EnForceMode);

	rtl8651_setAsicEthernetPHYReg(port, 31, 0x5);
	rtl8651_setAsicEthernetPHYReg(port, 5, 0x8b86);
	rtl8651_setAsicEthernetPHYReg(port, 6, 0x0000);
	rtl8651_setAsicEthernetPHYReg(port, 31, 0x0);

	REG32(PCRP0+ port*4) = (REG32(PCRP0+ port*4) & ~(EnForceMode)) | NwayAbility100MF | NwayAbility100MH | NwayAbility10MF | NwayAbility10MH; // enable Nway 10/100 ability

	rtl8651_restartAsicEthernetPHYNway(port);
	return 0;
}

#if defined(PORT5_RGMII_GMII)
unsigned int ExtP5GigaPhyMode=0;
void ProbeP5GigaPHYChip(void)
{
	unsigned int uid,tmp;
	unsigned int i;

	//printk("In Setting port5 \r\n");

	//REG32(0xB8000010)=0x01FFFCB9;

	for(i=0; i<=5; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);


	/* Read */
	rtl8651_setAsicEthernetPHYReg(GIGA_P5_PHYID,0x10,0x01FE);
	rtl8651_getAsicEthernetPHYReg(GIGA_P5_PHYID, 0, &tmp );

	//printk("Read port5 phyReg0= 0x%x \r\n",tmp);

	rtl8651_getAsicEthernetPHYReg( GIGA_P5_PHYID, 2, &tmp );
	//printk("Read port5 UPChipID= 0x%x \r\n",tmp);
	uid=tmp<<16;
	rtl8651_getAsicEthernetPHYReg( GIGA_P5_PHYID, 3, &tmp );
	//printk("Read port5 downChipID= 0x%x \r\n",tmp);
	uid=uid | tmp;

	if( uid==0x001CC912 )  //0x001cc912 is 8212 two giga port , 0x001cc940 is 8214 four giga port
	{
		//printk("Find Port5   have 8211 PHY Chip! \r\n");
		ExtP5GigaPhyMode=1;
		//return 1;
	}
	else
	{
		//printk("NO Find Port5 8211 PHY Chip! \r\n");
		//ExtP5GigaPhyMode=0;
		//return 1;
	}
	for(i=0; i<=5; i++)
		REG32(PCRP0+i*4) &= ~(EnForceMode);

}
#endif

void disable_phy_power_down(void)
{
	int i;
	uint32 statCtrlReg0;

	for (i=0; i<5; i++)
	{
		rtl8651_getAsicEthernetPHYReg( i, 0, &statCtrlReg0 );
 
		statCtrlReg0 &= (~POWER_DOWN);
 
		/* write PHY reg 0 */
		rtl8651_setAsicEthernetPHYReg( i, 0, statCtrlReg0 );

		REG32(PCRP0+i*4) &= ~(EnForceMode);
	}
	mdelay(3000);
}
#endif

int32 rtl865x_platform_check(void)
{
	uint32 bondOptReg=0;
	bondOptReg=REG32(0xB800000C);
	if(((bondOptReg&0x0F)!=0x7) && ((bondOptReg&0x0F)!=0x9))
	{
		printk("current chip doesn't supported,system halt...\n");
		while(1);
	}

	return SUCCESS;
}

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
unsigned int Get_P0_PhyMode(void)
{
	/*
		00: External  phy
		01: embedded phy
		10: olt
		11: deb_sel
	*/
	#define GET_BITVAL(v,bitpos,pat) ((v& ((unsigned int)pat<<bitpos))>>bitpos)
	#define RANG1 1
	#define RANG2 3
	#define RANG3  7
	#define RANG4 0xf

	#define SYS_HW_STRAP   (0xb8000000 +0x08)

	unsigned int v=REG32(SYS_HW_STRAP);
	unsigned int mode=GET_BITVAL(v, 6, RANG1) *2 + GET_BITVAL(v, 7, RANG1);

	return (mode&3);
}

unsigned int Get_P0_MiiMode(void)
{
	/*
		0: MII-PHY
		1: MII-MAC
		2: GMII-MAC
		3: RGMII
	*/
	#define GET_BITVAL(v,bitpos,pat) ((v& ((unsigned int)pat<<bitpos))>>bitpos)
	#define RANG1 1
	#define RANG2 3
	#define RANG3  7
	#define RANG4 0xf

	#define SYS_HW_STRAP   (0xb8000000 +0x08)

	unsigned int v=REG32(SYS_HW_STRAP);
	unsigned int mode=GET_BITVAL(v, 27, RANG2);

	return mode;
}

unsigned int Get_P0_RxDelay(void)
{
	#define GET_BITVAL(v,bitpos,pat) ((v& ((unsigned int)pat<<bitpos))>>bitpos)
	#define RANG1 1
	#define RANG2 3
	#define RANG3  7
	#define RANG4 0xf

	#define SYS_HW_STRAP   (0xb8000000 +0x08)

	unsigned int v=REG32(SYS_HW_STRAP);
	unsigned int val=GET_BITVAL(v, 29, RANG3);
	return val;
}

unsigned int Get_P0_TxDelay(void)
{
	#define GET_BITVAL(v,bitpos,pat) ((v& ((unsigned int)pat<<bitpos))>>bitpos)
	#define RANG1 1
	#define RANG2 3
	#define RANG3  7
	#define RANG4 0xf

	#define SYS_HW_STRAP   (0xb8000000 +0x08)

	unsigned int v=REG32(SYS_HW_STRAP);
	unsigned int val=GET_BITVAL(v, 17, RANG1);
	return val;
}

int Setting_RTL8197D_PHY(void)
{
	int i;

	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);

	/*
	  page 	addr rtl8197d-default 	rtl8197d-new value Purpose
	  0 		21 	0x02c5 			0x0232 			Green: up/low bond to 3/2
	  0 		22 	0x5b85 			0x5bd5 			Green: ad current from 2 to 3
	  1 		18 	0x901c 			0x9004 			finetune AOI waveform
	  1 		19 	0x4400 			0x5400 			finetune 100M DAC current
	  1 		25 	0x00da 			0x00d0 			enable pwdn10rx at pwr saving enable snr threshold = 18dB

	  4 		16 	0x4007 			0x737f 			enable EEE, fine tune EEE parameter
	  4 		24 	0xc0a0 			0xc0f3 			change EEE wake idle to 10us
	  4 		25 	0x0130 			0x0730 			turn off tx/rx pwr at LPI state
	*/

	// only do the PHY setting in this revision IC, no need for the new one.
	if (REG32(REVR) == 0x8197C000) {

		Set_GPHYWB(999, 0, 21, 0, 0x0232);

		/* purpose: to avoid 100M N-way link fail issue Set_p="1" */
		Set_GPHYWB(999, 0, 22, 0, 0x5bd5);

	 	/* purpose: to adjust AOI waveform */
		Set_GPHYWB(999, 1, 18, 0, 0x9004);

		/* purpose: to enhance ethernet 100Mhz output voltage about 1.0(v) */
		Set_GPHYWB(999, 1, 19, 0, 0x5400);

		Set_GPHYWB(999, 1, 25, 0, 0x00d0);  //enable pwdn10rx at pwr saving enable snr threshold = 18dB

		Set_GPHYWB(999, 4, 16, 0, 0x737f);// enable EEE, fine tune EEE parameter
		Set_GPHYWB(999, 4, 24, 0, 0xc0f3);	//change EEE wake idle to 10us
		Set_GPHYWB(999, 4, 25, 0, 0x0730);	 // turn off tx/rx pwr at LPI state
	}

	/* fine tune port on/off threshold to 160/148 */
	REG32(PBFCR0) = 0x009400A0;
	REG32(PBFCR1) = 0x009400A0;
	REG32(PBFCR2) = 0x009400A0;
	REG32(PBFCR3) = 0x009400A0;
	REG32(PBFCR4) = 0x009400A0;
	REG32(PBFCR6) = 0x009400A0;

	/* modify egress leaky bucket parameter, default inaccuracy is 5~10%, the new one is 1~2% after modification */
	REG32(ELBPCR) = 0x0000400B;
	REG32(ELBTTCR) = 0x000000C0;

	/* Fine tune minRx IPG from 6 to 5 byte */
	REG32(MACCR) = 0x80420185;

	/* default enable MAC EEE */
	REG32(EEECR) = 0x28739ce7;

	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) &= ~(EnForceMode);

	return 0;
}
#endif

#ifdef CONFIG_RTL_8196E
int Setting_RTL8196E_PHY(void)
{
	int i;

	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);

	// write page1, reg16, bit[15:13] Iq Current 110:175uA (default 100: 125uA)
	Set_GPHYWB(999, 1, 16, 0xffff-(0x7<<13), 0x6<<13);

	// disable power saving mode in A-cut only
	if (REG32(REVR) == 0x8196e000)
		Set_GPHYWB(999, 0, 0x18, 0xffff-(1<<15), 0<<15);

	/* B-cut and later,
	    just increase a little power in long RJ45 cable case for Green Ethernet feature.
	 */
	else 
	{
		// adtune_lb setting
		Set_GPHYWB(999, 0, 22, 0xffff-(0x7<<4), 0x4<<4);
		//Setting SNR lb and hb
		Set_GPHYWB(999, 0, 21, 0xffff-(0xff<<0), 0xc2<<0);
		//auto bais current
		Set_GPHYWB(999, 1, 19, 0xffff-(0x1<<0), 0x0<<0);
		Set_GPHYWB(999, 0, 22, 0xffff-(0x1<<3), 0x0<<3);
	}
	
	// fix Ethernet IOT issue
	if ((REG32(BOND_OPTION) & BOND_ID_MASK) != BOND_8196ES) {
		Set_GPHYWB(999, 0, 26, 0xffff-(0x1<<14), 0x0<<14);
		Set_GPHYWB(999, 0, 17, 0xffff-(0xf<<8), 0xe<<8);
	}
	
	/* 100M half duplex enhancement */
	/* fix SmartBits half duplex backpressure IOT issue */
 	REG32(MACCR)= (REG32(MACCR) & ~(CF_RXIPG_MASK | SELIPG_MASK)) | (0x05 | SELIPG_11); 
	
	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) &= ~(EnForceMode);

	return 0;
}
#endif

void enable_EEE(void)
{
#if defined(CONFIG_RTL_8198)
	eee_phy_enable_98();
#else

	int i;

	for(i=0; i<RTL8651_PHY_NUMBER; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);

#if defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_819XD)
	//enable 100M EEE and 10M EEE
	Set_GPHYWB(999, 4, 16, 0xffff-(0x3<<12), 0x3<<12);

	//enable MAC EEE
	REG32(EEECR) = 0x0E739CE7;
	
#elif defined(CONFIG_RTL_8196C)
	for(i=0; i<RTL8651_PHY_NUMBER; i++) {
		eee_phy_enable_by_port(i);
	}

	// set FRC_P0_EEE_100, EN_P0_TX_EEE and EN_P0_RX_EEE
	//REG32(EEECR) = 0x0E739CE7;		// consult with Jim and Anson, we do not use this setting.
	// set EN_P0_TX_EEE and EN_P0_RX_EEE
	REG32(EEECR) = 0x06318C63;

#ifdef CONFIG_POCKET_ROUTER_SUPPORT
	#define ETH_PORT_START		4
	#define ETH_PORT_END		4
#else
	#define ETH_PORT_START		0
	#define ETH_PORT_END		4
#endif
	for ( i = ETH_PORT_START ; i <= ETH_PORT_END; i++ ) {
		/* enable phy 100 eee ability */
		mmd_write(i, 7, 60, 0x2);
	}	
#endif

	for(i=0; i<RTL8651_PHY_NUMBER; i++)
		REG32(PCRP0+i*4) &= ~(EnForceMode);
#endif	
}

void disable_EEE(void)
{
#if defined(CONFIG_RTL_8198)
	eee_phy_disable_98();

#else
	// for CONFIG_RTL_8196C, CONFIG_RTL_819XD and CONFIG_RTL_8196E
	int i;

	for(i=0; i<RTL8651_PHY_NUMBER; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);

	//disable EEE MAC
	REG32(EEECR) = 0;

	//disable 100M EEE and 10M EEE
	Set_GPHYWB(999, 4, 16, 0xffff-(0x3<<12), 0x0<<12);
	
	for(i=0; i<RTL8651_PHY_NUMBER; i++)
		REG32(PCRP0+i*4) &= ~(EnForceMode);
#endif
}

/*patch for LED showing*/
#define BICOLOR_LED 1
#define REG32_ANDOR(x,y,z)   (REG32(x)=(REG32(x)& (y))|(z))
/*=========================================
  * init Layer2 Asic
  * rtl865x_initAsicL2 mainly configure basic&L2 Asic.
  * =========================================*/
int32 rtl865x_initAsicL2(rtl8651_tblAsic_InitPara_t *para)
{
	int32 index;

#ifdef BICOLOR_LED
#if defined (CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
#else
	unsigned int hw_val;
#endif
#endif

#ifdef CONFIG_RTL8196C_ETH_IOT
	port_link_sts = 0;
	port_linkpartner_eee = 0;
#endif

	//bond check
	//rtl865x_platform_check();

/*==============================
 *port
  ==============================*/
//#ifdef CONFIG_RTL8214_SUPPORT		/* Replaced by auto-detect */
#ifndef CONFIG_RTL_819X
//	rtl865x_wanPortMask = RTL865X_PORTMASK_UNASIGNED;
	if (rtl865x_probeP5GigaPHYChip())
	{
		para->externalPHYProperty |= RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B;
		para->externalPHYId[5] = CONFIG_EXTRTL8212_PHYID_P5;

#if defined(CONFIG_RTL8186_KB)
		/* wan/lan port mask if the RTL8186_KB defined */
		rtl865x_wanPortMask = 0x2;		/* port 1 */
		rtl865x_lanPortMask = 0x1dd;		/* port 5/2/3/4/6/7/8 */
#else
		rtl865x_wanPortMask = 0x20;		/* port 5 */
		rtl865x_lanPortMask = 0x1de;		/* port 1/2/3/4/6/7/8 */
#endif
	}

	if (rtl865x_probeP1toP4GigaPHYChip())
	{
		para->externalPHYProperty |= RTL8651_TBLASIC_EXTPHYPROPERTY_PORT1234_RTL8212;
		para->externalPHYId[1] = CONFIG_EXTRTL8212_PHYID_P1;
		para->externalPHYId[2] = CONFIG_EXTRTL8212_PHYID_P1+1;
		para->externalPHYId[3] = CONFIG_EXTRTL8212_PHYID_P3;
		para->externalPHYId[4] = CONFIG_EXTRTL8212_PHYID_P3+1;
	}
#endif

	if ((rtl865x_probeSdramSize())>(16<<20))
	{
#if !defined(CONFIG_RTL_819X)
		rtl865x_fix8214Bug();
		rtl865x_maxPreAllocRxSkb = 256;
		rtl865x_rxSkbPktHdrDescNum = 512;
		rtl865x_txSkbPktHdrDescNum = 1024;
#endif
	}


#if defined(RTL865X_TEST)
	char chipVersion[16];
	int rev;

	/* _rtl8651_mapToVirtualRegSpace(); to be removed */
	rtl8651_getChipVersion(chipVersion, sizeof(chipVersion), &rev);
	memset(pVirtualSWReg,0, VIRTUAL_SWCORE_REG_SIZE);
	memset(pVirtualSysReg,0, VIRTUAL_SYSTEM_REG_SIZE);
	memset(pVirtualHsb,0, HSB_SIZE);
	memset(pVirtualHsa,0, HSA_SIZE);
	memset(pVirtualSWTable,0,VIRTUAL_SWCORE_TBL_SIZE);
	memset(pVirtualMIIRegs,0,sizeof(pVirtualMIIRegs));
	rtl8651_setChipVersion(chipVersion,  &rev);

#elif 0 && defined(RTL865X_MODEL_USER) /* map to real reg space will cause VSV test crash in model code mode. */
#if defined(VSV)||defined(MIILIKE)
	_rtl8651_mapToRealRegSpace();
#else
	_rtl8651_mapToVirtualRegSpace(); /* for cleshell.c:main() only ! */
	modelIcSetDefaultValue();
#endif
#elif defined(RTL865X_MODEL_KERNEL)
#endif

	ASICDRV_INIT_CHECK(_rtl8651_initAsicPara(para));

	rtl8651_getChipVersion(RtkHomeGatewayChipName, sizeof(RtkHomeGatewayChipName), &RtkHomeGatewayChipRevisionID);
	rtl8651_getChipNameID(&RtkHomeGatewayChipNameID);

#ifndef RTL865X_TEST
	rtlglue_printf("chip name: %s, chip revid: %d\n", RtkHomeGatewayChipName, RtkHomeGatewayChipRevisionID);
#endif

	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT1234_RTL8212)
	{

		printk("\nEnable Port1~Port4 GigaPort.\n\n");
		/* Patch for 'RGMII port does not get descritpors'. Set to MII PHY mode first and later we'll change to RGMII mode again. */
		rtl865xC_setAsicEthernetMIIMode(0, LINK_MII_PHY);

		/*
			# According to Hardware SD: David & Maxod,

			Set Port5_GMII Configuration Register.
			- RGMII Output Timing compensation control : 0 ns
			- RGMII Input Timing compensation control : 0 ns
		*/
		rtl865xC_setAsicEthernetRGMIITiming(0, RGMII_TCOMP_0NS, RGMII_RCOMP_0NS);

		/* Set P1 - P4 to SerDes Interface. */
		WRITE_MEM32(PITCR, Port4_TypeCfg_SerDes | Port3_TypeCfg_SerDes | Port2_TypeCfg_SerDes | Port1_TypeCfg_SerDes );
	}
	else if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
	{
		/* Patch for 'RGMII port does not get descritpors'. Set to MII PHY mode first and later we'll change to RGMII mode again. */
		rtl865xC_setAsicEthernetMIIMode(RTL8651_MII_PORTNUMBER, LINK_MII_PHY);

		/*
			# According to Hardware SD: David & Maxod,

			Set Port5_GMII Configuration Register.
			- RGMII Output Timing compensation control : 0 ns
			- RGMII Input Timing compensation control : 0 ns
		*/
		rtl865xC_setAsicEthernetRGMIITiming(RTL8651_MII_PORTNUMBER, RGMII_TCOMP_0NS, RGMII_RCOMP_0NS);
	}

#ifdef CONFIG_RTL8196C_REVISION_B
	if (REG32(REVR) == RTL8196C_REVISION_B)
		Setting_RTL8196C_PHY();

#elif defined(CONFIG_RTL_8196E)
	Setting_RTL8196E_PHY();

#elif defined(CONFIG_RTL_819XD)
	Setting_RTL8197D_PHY();

#elif defined(CONFIG_RTL_8198)
#if 0//def PORT5_RGMII_GMII
	ProbeP5GigaPHYChip();
#endif

	Setting_RTL8198_GPHY();
#endif

#ifdef CONFIG_8198_PORT5_RGMII
	{
	/* define GPIO port */
	enum GPIO_PORT
	{
	GPIO_PORT_A = 0,
	GPIO_PORT_B,
	GPIO_PORT_C,
	GPIO_PORT_D,
	GPIO_PORT_E,
	GPIO_PORT_F,
	GPIO_PORT_G,
	GPIO_PORT_H,
	GPIO_PORT_I,
	GPIO_PORT_MAX,
	};

	#define REG_IOCFG_GPIO		0x00000018

	extern int32 smi_init(uint32 port, uint32 pinSCK, uint32 pinSDA);
	extern int RTL8370_init(void);

	// set to GPIO mode
	REG32(PIN_MUX_SEL) |= (REG_IOCFG_GPIO);


	WRITE_MEM32(PABCD_CNR, READ_MEM32(PABCD_CNR) & (~(0x0000000C)));	//set GPIO pin
	WRITE_MEM32(PABCD_DIR, READ_MEM32(PABCD_DIR) | ((0x0000000C))); //output pin

	smi_init(GPIO_PORT_A, 3, 2);

	RTL8370_init();
	}
#endif

	/* 	2006.12.12
		We turn on bit.10 (ENATT2LOG).

		* Current implementation of unnumbered pppoe in multiple session
		When wan type is multiple-session, and one session is unnumbered pppoe, WAN to unnumbered LAN is RP --> NPI.
		And user adds ACL rule to trap dip = unnumbered LAN to CPU.

		However, when pktOpApp of this ACL rule is set, it seems that this toCPU ACL does not work.
		Therefore, we turn on this bit (ENATT2LOG) to trap pkts (WAN --> unnumbered LAN) to CPU.

	*/
	WRITE_MEM32( SWTCR1, READ_MEM32( SWTCR1 ) | EnNATT2LOG );

	/*
	  * Turn on ENFRAG2ACLPT for Rate Limit. For those packets which need to be trapped to CPU, we turn on
	  * this bit to tell ASIC ACL and Protocol Trap to process these packets. If this bit is not turnned on, packets
	  * which need to be trapped to CPU will not be processed by ASIC ACL and Protocol Trap.
	  * NOTE: 	If this bit is turned on, the backward compatible will disable.
	  *																- chhuang
	  */
	WRITE_MEM32( SWTCR1, READ_MEM32( SWTCR1 ) | ENFRAGTOACLPT );

#ifdef CONFIG_RTL865X_LIGHT_ROMEDRV
	WRITE_MEM32( SWTCR1, READ_MEM32( SWTCR1 ) | L4EnHash1 );    /*Turn on Napt Enhanced hash1*/
#endif

	/*
	  * Cannot turn on EnNAP8651B due to:
	  * If turn on, NAT/LP/ServerPort will reference nexthop. This will result in referecing wrong L2 entry when
	  * the destination host is in the same subnet as WAN.
	  */


	/*Although chip is in 8650 compatible mode,
	some 865XB features are independent to compatibility register*/
	/*Initialize them here if needed*/

	{
		 int rev;
		char chipVersion[16];
		rtl8651_getChipVersion(chipVersion, sizeof(chipVersion), &rev);
		if(chipVersion[strlen(chipVersion)-1]=='B'
			|| chipVersion[strlen(chipVersion) - 1] == 'C' )
		{
			rtl8651_totalExtPortNum=3; //this replaces all RTL8651_EXTPORT_NUMBER defines
			rtl8651_allExtPortMask = 0x7<<RTL8651_MAC_NUMBER; //this replaces all RTL8651_EXTPORTMASK defines
#if !defined(RTL865X_TEST) && !defined(RTL865X_MODEL_USER)
			rtl8651_asicEthernetCableMeterInit();
#endif
		}

	}
	//Disable layer2, layer3 and layer4 function
	//Layer 2 enabled automatically when a VLAN is added
	//Layer 3 enabled automatically when a network interface is added.
	//Layer 4 enabled automatically when an IP address is setup.
	rtl8651_setAsicOperationLayer(1);
	rtl8651_clearAsicCommTable();
	rtl8651_clearAsicL2Table();
	//rtl8651_clearAsicAllTable();//MAY BE OMITTED. FULL_RST clears all tables already.
	rtl8651_setAsicSpanningEnable(FALSE);

#ifdef RTL865XB_URL_FILTER
	WRITE_MEM32( SWTCR1, READ_MEM32( SWTCR1 ) | EN_51B_CPU_REASON );	/* Use 8650B's new reason bit definition. */
#endif

#if !defined(RTL865X_TEST) && !defined(RTL865X_MODEL_USER)
	//mark_issue
	//WRITE_MEM32(0xbd012064,READ_MEM32(0xbd012064)|0xf0000000);//Enable Lexra bus timeout interrupt and timeout limit
	#if 0
	// shliu: Why we set PORT5_PHY_CONTROL to value "0x2c7"?? Should set "0x2c2"?!
	WRITE_MEM32(0xbc800020,0x000002c7);
	WRITE_MEM32(0xbc800008,0xbc8020a0);
	WRITE_MEM32(0xbc800000,0x00000009);
	#endif
#endif

    /* Init PHY LED style */
#ifdef BICOLOR_LED

#ifdef CONFIG_RTL_819X
#if defined(CONFIG_RTL8186_KB) && defined(CONFIG_RTL8186_KB_N)
    hw_val = read_gpio_hw_setting();
    REG32(PIN_MUX_SEL) =0x0fffff80;/*For Belkin_n board, not for demo board*/
    REG32(LEDCREG)=0;

#elif defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
	/*
		#LED = direct mode
		set mode 0x0
		swwb 0xbb804300 21-20 0x2 19-18 $mode 17-16 $mode 15-14 $mode 13-12 $mode 11-10 $mode 9-8 $mode
	*/
	#ifdef 	CONFIG_RTK_VOIP_BOARD	
	//for GMII/RGMII
	//REG32(PIN_MUX_SEL) &= ~( (3<<8) | (3<<10) | (3<<3) | (1<<15) );  //let P0 to mii mode
	REG32(PIN_MUX_SEL2) &= ~ ((3<<0) | (3<<3) | (3<<6) | (3<<9) | (3<<12) );  //LED0~LED4
	#else
	REG32(PIN_MUX_SEL) &= ~( (3<<8) | (3<<10) | (3<<3) | (1<<15) );  //let P0 to mii mode
	REG32(PIN_MUX_SEL2) &= ~ ((3<<0) | (3<<3) | (3<<6) | (3<<9) | (3<<12) | (7<<15) );  //S0-S3, P0-P1
	#endif
	REG32(LEDCREG)  = (2<<20) | (0<<18) | (0<<16) | (0<<14) | (0<<12) | (0<<10) | (0<<8);  //P0-P5

#elif defined (CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198)
#else
    hw_val = read_gpio_hw_setting();
    REG32(PIN_MUX_SEL) =0x00000380;
    REG32(LEDCREG)=0;
#endif
#else
     hw_val = read_gpio_hw_setting();
    if (hw_val == 0x2 || hw_val == 0x3 || hw_val == 0x6 || hw_val == 0x7)  // LED in matrix mode
     {
	REG32(LEDCREG)  = 0x155500;
     }
    REG32(TCR0) = 0x000002c2;
    REG32(SWTAA) = PORT5_PHY_CONTROL;
    REG32(SWTACR) = ACTION_START | CMD_FORCE;
    while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE ); /* Wait for command done */
#endif

#else
#if 0
    #if defined(BICOLOR_LED_VENDOR_BXXX)
        REG32(LEDCR) |= 0x00080000;

        REG32(PABCNR) &= ~0xc01f0000; /* set port a-7/6 & port b-4/3/2/1/0 to gpio */
        REG32(PABDIR) |=  0x401f0000; /* set port a-6 & port b-4/3/2/1/0 gpio direction-output */
        REG32(PABDIR) &= ~0x80000000; /* set port a-7 gpio direction-input */
    #else /* BICOLOR_LED_VENDOR_BXXX */
        REG32(LEDCR) = 0x00000000;
        REG32(TCR0) = 0x000002c7;
        REG32(SWTAA) = PORT5_PHY_CONTROL;
        REG32(SWTACR) = ACTION_START | CMD_FORCE;
        while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE ); /* Wait for command done */
    #endif /* BICOLOR_LED_VENDOR_BXXX */
#endif
#endif

#if !defined(CONFIG_RTL_819XD) && !defined(CONFIG_RTL_8196E)
	//MAC Control (0xBC803000)
/*	WRITE_MEM32(MACCR,READ_MEM32(MACCR)&~DIS_IPG);//Set IFG range as 96+-4bit time*/
	WRITE_MEM32(MACCR,READ_MEM32(MACCR)&~NORMAL_BACKOFF);//Normal backoff
	WRITE_MEM32(MACCR,READ_MEM32(MACCR)&~BACKOFF_EXPONENTIAL_3);//Exponential parameter is 9
	WRITE_MEM32(MACCR,READ_MEM32(MACCR)|INFINITE_PAUSE_FRAMES);//send pause frames infinitely.
	WRITE_MEM32(MACCR,READ_MEM32(MACCR)|DIS_MASK_CGST);
#endif

	miiPhyAddress = -1;		/* not ready to use mii port 5 */

	memset( &rtl8651AsicEthernetTable[0], 0, ( RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum ) * sizeof(rtl8651_tblAsic_ethernet_t) );
	/* Record the PHYIDs of physical ports. Default values are 0. */
	rtl8651AsicEthernetTable[0].phyId = 0;	/* Default value of port 0's embedded phy id -- 0 */
	rtl8651AsicEthernetTable[0].isGPHY = FALSE;

	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT1234_RTL8212)
	{
		rtl8651AsicEthernetTable[1].phyId = rtl8651_tblAsicDrvPara.externalPHYId[1];
		rtl8651AsicEthernetTable[2].phyId = rtl8651_tblAsicDrvPara.externalPHYId[2];
		rtl8651AsicEthernetTable[3].phyId = rtl8651_tblAsicDrvPara.externalPHYId[3];
		rtl8651AsicEthernetTable[4].phyId = rtl8651_tblAsicDrvPara.externalPHYId[4];
		rtl8651AsicEthernetTable[1].isGPHY = TRUE;
		rtl8651AsicEthernetTable[2].isGPHY = TRUE;
		rtl8651AsicEthernetTable[3].isGPHY = TRUE;
		rtl8651AsicEthernetTable[4].isGPHY = TRUE;
	} else
	{	/* USE internal 10/100 PHY */
		rtl8651AsicEthernetTable[1].phyId = 1;	/* Default value of port 1's embedded phy id -- 1 */
		rtl8651AsicEthernetTable[2].phyId = 2;	/* Default value of port 2's embedded phy id -- 2 */
		rtl8651AsicEthernetTable[3].phyId = 3;	/* Default value of port 3's embedded phy id -- 3 */
		rtl8651AsicEthernetTable[4].phyId = 4;	/* Default value of port 4's embedded phy id -- 4 */
		rtl8651AsicEthernetTable[1].isGPHY = FALSE;
		rtl8651AsicEthernetTable[2].isGPHY = FALSE;
		rtl8651AsicEthernetTable[3].isGPHY = FALSE;
		rtl8651AsicEthernetTable[4].isGPHY = FALSE;
	}

	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
	{
		rtl8651AsicEthernetTable[RTL8651_MII_PORTNUMBER].phyId = rtl8651_tblAsicDrvPara.externalPHYId[5];
		rtl8651AsicEthernetTable[RTL8651_MII_PORTNUMBER].isGPHY = TRUE;
		rtl8651_setAsicEthernetMII(	rtl8651AsicEthernetTable[RTL8651_MII_PORTNUMBER].phyId,
									P5_LINK_RGMII,
									TRUE );
	}

#if 0
	WRITE_MEM32(PCRP0, (rtl8651AsicEthernetTable[0].phyId<<ExtPHYID_OFFSET)|AcptMaxLen_16K|EnablePHYIf  ); /* Jumbo Frame */
	WRITE_MEM32(PCRP1, (rtl8651AsicEthernetTable[1].phyId<<ExtPHYID_OFFSET)|AcptMaxLen_16K|EnablePHYIf ); /* Jumbo Frame */
	WRITE_MEM32(PCRP2, (rtl8651AsicEthernetTable[2].phyId<<ExtPHYID_OFFSET)|AcptMaxLen_16K|EnablePHYIf ); /* Jumbo Frame */
	WRITE_MEM32(PCRP3, (rtl8651AsicEthernetTable[3].phyId<<ExtPHYID_OFFSET)|AcptMaxLen_16K|EnablePHYIf ); /* Jumbo Frame */
	WRITE_MEM32(PCRP4, (rtl8651AsicEthernetTable[4].phyId<<ExtPHYID_OFFSET)|AcptMaxLen_16K|EnablePHYIf ); /* Jumbo Frame */

	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
	{
		WRITE_MEM32(PCRP5, (rtl8651AsicEthernetTable[5].phyId<<ExtPHYID_OFFSET)|AcptMaxLen_16K|EnablePHYIf ); /* Jumbo Frame */
	}

#if 0	/* No need to set PHYID of port 6. Just use ASIC default value. */
	/*  Due to MSb of phyid has been added an inverter in b-cut,
	 *  although we want to set 6(0b00110) as phyid, we have to write 22(0b10110) instead. */
	WRITE_MEM32( PCRP6, ( 22 << ExtPHYID_OFFSET ) | AcptMaxLen_16K | EnablePHYIf );
#endif
	if(RTL865X_PHY6_DSP_BUG)
		WRITE_MEM32(PCRP6, (6<<ExtPHYID_OFFSET)|AcptMaxLen_16K|EnablePHYIf );
	/* Set PHYID 6 to PCRP6. (By default, PHYID of PCRP6 is 0. It will collide with PHYID of port 0. */
#endif



/*	WRITE_MEM32(MACCR,READ_MEM32(MACCR)&~(EN_FX_P4 | EN_FX_P3 | EN_FX_P2 | EN_FX_P1 | EN_FX_P0));//Disable FX mode (UTP mode)*/
	/* Initialize MIB counters */
	rtl8651_clearAsicCounter();

	rtl865xC_setNetDecisionPolicy( NETIF_VLAN_BASED );	/* Net interface Multilayer-Decision-Based Control -- Set to VLAN-Based mode. */
	//rtl865xC_setNetDecisionPolicy( NETIF_PORT_BASED );	/* Net interface Multilayer-Decision-Based Control -- Set to port-Based mode. */
	//WRITE_MEM32(PLITIMR,0);

	/*FIXME:Hyking init in Layer4 2*/
	//WRITE_MEM32(SWTCR0,READ_MEM32(SWTCR0)&~NAPTR_NOT_FOUND_DROP);//When reverse NAPT entry not found, CPU process it.
	//rtl8651_setAsicNaptAutoAddDelete(FALSE, TRUE);
	WRITE_MEM32( VCR0, READ_MEM32( VCR0 ) & (~EN_ALL_PORT_VLAN_INGRESS_FILTER) );		/* Disable VLAN ingress filter of all ports */ /* Please reference to the maintis bug 2656# */
	/*WRITE_MEM32( VCR0, READ_MEM32( VCR0 ) & (~EN_ALL_PORT_VLAN_INGRESS_FILTER) );*/		/* Enable VLAN ingress filter of all ports */
	WRITE_MEM32(SWTCR0,READ_MEM32(SWTCR0)&~WAN_ROUTE_MASK);//Set WAN route toEnable (Allow traffic from WAN port to WAN port)
	WRITE_MEM32(SWTCR0,READ_MEM32(SWTCR0)|NAPTF2CPU);//When packet destination to switch. Just send to CPU
	WRITE_MEM32(SWTCR0,(READ_MEM32(SWTCR0)&(~LIMDBC_MASK))|LIMDBC_VLAN);//When packet destination to switch. Just send to CPU

	/*FIXME:Hyking init in Layer3 1*/
	//rtl8651_setAsicMulticastEnable(TRUE); /* Enable multicast table */

	/* Enable unknown unicast / multicast packet to be trapped to CPU. */
//	WRITE_MEM32( FFCR, READ_MEM32( FFCR ) | EN_UNUNICAST_TOCPU );
	WRITE_MEM32( FFCR, READ_MEM32( FFCR ) & ~EN_UNUNICAST_TOCPU );
	WRITE_MEM32( FFCR, READ_MEM32( FFCR ) | EN_UNMCAST_TOCPU );
/*	WRITE_MEM32(SWTMCR,READ_MEM32(SWTMCR)&~MCAST_TO_CPU);*/
/*	WRITE_MEM32(SWTMCR,READ_MEM32(SWTMCR)|EN_BCAST);//Enable special broadcast handling*/
/*	WRITE_MEM32(SWTMCR,READ_MEM32(SWTMCR)&~BCAST_TO_CPU);//When EN_BCAST enables, this bit is invalid*/
/*	WRITE_MEM32(SWTMCR,READ_MEM32(SWTMCR)&~BRIDGE_PKT_TO_CPU);//Current no bridge protocol is supported*/

	/*FIXME:Hyking init in Layer3 1*/
	//WRITE_MEM32(ALECR, READ_MEM32(ALECR)|(uint32)EN_PPPOE);//enable PPPoE auto encapsulation
	WRITE_MEM32(CSCR,READ_MEM32(CSCR)&~ALLOW_L2_CHKSUM_ERR);//Don't allow chcksum error pkt be forwarded.

	WRITE_MEM32(CSCR,READ_MEM32(CSCR)&~ALLOW_L3_CHKSUM_ERR);
	WRITE_MEM32(CSCR,READ_MEM32(CSCR)&~ALLOW_L4_CHKSUM_ERR);
	WRITE_MEM32(CSCR,READ_MEM32(CSCR)|EN_ETHER_L3_CHKSUM_REC); //Enable L3 checksum Re-calculation
	WRITE_MEM32(CSCR,READ_MEM32(CSCR)|EN_ETHER_L4_CHKSUM_REC); //Enable L4 checksum Re-calculation

/*	WRITE_MEM32(MISCCR,READ_MEM32(MISCCR)&~FRAG2CPU);//IP fragment packet does not need to send to CPU when initial ASIC
	WRITE_MEM32(MISCCR,READ_MEM32(MISCCR)&~MULTICAST_L2_MTU_MASK);
	WRITE_MEM32(MISCCR,READ_MEM32(MISCCR)|(1522&MULTICAST_L2_MTU_MASK));//Multicast packet layer2 size 1522 at most*/
	/* follow RTL865xB's convention, we use 1522 as default multicast MTU */

	/*FIXME:Hyking init in Layer3 1*/
	//rtl8651_setAsicMulticastMTU(1522);

	//Set all Protocol-Based Reg. to 0

	for (index=0;index<32;index++)
		WRITE_MEM32(PBVCR0+index*4,  0x00000000);
	//Enable TTL-1
	/*FIXME:Hyking init in Layer3 1*/
	//WRITE_MEM32(TTLCR,READ_MEM32(TTLCR)|(uint32)EN_TTL1);//Don't hide this router. enable TTL-1 when routing on this gateway.


	for (index=0; index<RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum; index++) {



		if(	rtl8651_setAsicMulticastSpanningTreePortState(index, RTL8651_PORTSTA_FORWARDING))
			return FAILED;

		rtl865xC_setAsicSpanningTreePortState(index, RTL8651_PORTSTA_FORWARDING);

		rtl8651_setAsicEthernetBandwidthControl(index, TRUE, RTL8651_BC_FULL);
		rtl8651_setAsicEthernetBandwidthControl(index, FALSE, RTL8651_BC_FULL);
	}

	// 08-15-2012, set TRXRDY bit in rtl865x_start() in rtl865x_asicCom.c
	/* Enable TX/RX After ALL ASIC configurations are done */
	//WRITE_MEM32( SIRR, READ_MEM32(SIRR)| TRXRDY );

	/* Initiate Bandwidth control backward compatible mode : Set all of them to FULL-Rate */
	{
		int32 portIdx, typeIdx;
		_rtl865xB_BandwidthCtrlMultiplier = _RTL865XB_BANDWIDTHCTRL_X1;
		for ( portIdx = 0 ; portIdx < RTL8651_PORT_NUMBER ; portIdx ++ )
		{
			for ( typeIdx = 0 ; typeIdx < _RTL865XB_BANDWIDTHCTRL_CFGTYPE ; typeIdx ++ )
			{
				_rtl865xB_BandwidthCtrlPerPortConfiguration[portIdx][typeIdx] = BW_FULL_RATE;
			}
		}
		/* Sync the configuration to ASIC */
		_rtl8651_syncToAsicEthernetBandwidthControl();
	}



	/* ==================================================================================================
		Embedded PHY patch -- According to the designer, internal PHY's parameters need to be adjusted.
	 ================================================================================================== */
	if(RTL865X_PHY6_DSP_BUG) /*modified by Mark*/
	{
		rtl8651_setAsicEthernetPHYReg( 6, 9, 0x0505 );
		rtl8651_setAsicEthernetPHYReg( 6, 4, 0x1F10 );
		rtl8651_setAsicEthernetPHYReg( 6, 0, 0x1200 );
	}

	/* ===============================
	    =============================== */
	{
		uint32 port;
		uint32 maxPort;

		maxPort =	(rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)?
					RTL8651_MAC_NUMBER:
					RTL8651_PHY_NUMBER;

		for ( port = 0 ; port < maxPort ; port ++ )
		{
			rtl8651_setAsicFlowControlRegister(port, TRUE);
			rtl865xC_setAsicPortPauseFlowControl(port, TRUE, TRUE);
		}
	}

	/* ===============================
	 	EEE setup
	    =============================== */
#if defined(CONFIG_RTL_EEE_DISABLED) ||defined(CONFIG_MP_PSD_SUPPORT)	
	eee_enabled = 0;
#else
	eee_enabled = 1;
#endif

	if (eee_enabled) {	    
		enable_EEE();
	}
	else {
		disable_EEE();
	}

	/* ===============================
	 	(1) Handling port 0.
	    =============================== */
	rtl8651_restartAsicEthernetPHYNway(0);	/* Restart N-way of port 0 to let embedded phy patch take effect. */

	/* ===============================
	 	(2) Handling port 1 - port 4.
	    =============================== */
	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT1234_RTL8212)
	{

	} else
	{
		/* Restart N-way of port 1 - port 4 to let embedded phy patch take effect. */
		{
			uint32 port;

			/* Restart N-way of port 1 - port 4 */
			for ( port = 1; port < RTL8651_PHY_NUMBER; port++ )
			{
				rtl8651_restartAsicEthernetPHYNway(port);
			}
		}
	}

	/* ===============================
		(3) Handling port 5.
	    =============================== */


	/* =====================
		QoS-related patch
	    ===================== */
	{
		#define DEFAULT_ILB_UBOUND 0x3FBE  /*added by Mark for suggested Leacky Bucket value*/
		#define DEFAULT_ILB_LBOUND 0x3FBC

		uint32 token, tick, hiThreshold,i;
		rtl8651_getAsicLBParameter( &token, &tick, &hiThreshold );
		hiThreshold = 0x400;	/* New default vlue. */
		rtl8651_setAsicLBParameter( token, tick, hiThreshold );
		/*Mantis(2307): Ingress leaky bucket need to be initized with suggested value . added by mark*/
		WRITE_MEM32( ILBPCR1, DEFAULT_ILB_UBOUND << UpperBound_OFFSET | DEFAULT_ILB_LBOUND << LowerBound_OFFSET );
		for(i=0;i<=(RTL8651_PHY_NUMBER/2);i++) /*Current Token Register is 2 bytes per port*/
			WRITE_MEM32( ILB_CURRENT_TOKEN + 4*i , DEFAULT_ILB_UBOUND << UpperBound_OFFSET | DEFAULT_ILB_UBOUND );

	}

	#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
	/* Initiate Queue Management system */
	_rtl865xC_QM_init();
	#endif

	/*
		Init QUEUE Number configuration for RTL865xC : For Port 0~5 and CPU Port - All ports have 1 queue for each.
	*/
	{
#if !defined(CONFIG_RTL_819XD	) && !defined(CONFIG_RTL_8196E)
	/*	The default value was just as same as what we want	*/
		rtl865xC_lockSWCore();

		/* Enable Flow Control for each QUEUE / Port */
		{
			int32 port, queue;

			for ( port = PHY0 ; port <= CPU ; port ++ )
			{
				/*	Set the Ingress & Egress bandwidth control NO LIMIT */
				/* Has been done by _rtl8651_syncToAsicEthernetBandwidthControl() */
				/*
				rtl8651_setAsicPortIngressBandwidth(port, (IBWC_ODDPORT_MASK>>IBWC_ODDPORT_OFFSET));
				rtl8651_setAsicPortEgressBandwidth(port, (APR_MASK>>APR_OFFSET));
				*/

				/*
					Init QUEUE Number configuration for RTL865xC : For Port 0~5 and CPU Port - All ports have 1 queue for each.
				*/
				rtl8651_setAsicOutputQueueNumber(port, 1	/* According to DataSheet of QNUMCR : All ports use 1 queue by default */);

				for ( queue = QUEUE0 ; queue <= QUEUE5 ; queue ++ )
				{
					rtl8651_setAsicQueueFlowControlConfigureRegister( port, queue, TRUE);
					#if 0
					/*	Please keep the queue setting as above.
					*	Default set queue FC disable will corrupt wlan<->lan performance
					*	2011/01/10 ZhaoBo
					*/
					rtl8651_setAsicQueueFlowControlConfigureRegister( port, queue, FALSE);
					#endif
				}
			}
		}

		rtl865xC_waitForOutputQueueEmpty();
		rtl8651_resetAsicOutputQueue();
		rtl865xC_unLockSWCore();
#endif
		/* DSP bug (PHY-ID for DSP controller is set same as PHY 0 ) in RTL865xC A-Cut */
		if(RTL865X_PHY6_DSP_BUG)
		/* correct the default value of input queue flow control threshold */
			WRITE_MEM32( IQFCTCR, 0xC8 << IQ_DSC_FCON_OFFSET | 0x96 << IQ_DSC_FCOFF_OFFSET );

		if ( RTL865X_IQFCTCR_DEFAULT_VALUE_BUG )
		{
			rtl8651_setAsicSystemInputFlowControlRegister(0xc8, 0x96);	/* Configure the ASIC Input Queue Flow control threshold to the default value ( ASIC default value is opposite from correct ) */
		}

	}

	/* set default include IFG */
	WRITE_MEM32( QOSFCR, BC_withPIFG_MASK);

		{
		rtl8651_setAsicPriorityDecision(2, 1, 1, 1, 1);

		WRITE_MEM32(PBPCR, 0);

#if !defined(CONFIG_RTL_819XD	) && !defined(CONFIG_RTL_8196E)
		/* Set the threshold value for qos sytem */
		_rtl865x_setQosThresholdByQueueIdx(QNUM_IDX_123);
#endif

	/*	clear dscp priority assignment, otherwise, pkt with dscp value 0 will be assign priority 1		*/
		WRITE_MEM32(DSCPCR0,0);
		WRITE_MEM32(DSCPCR1,0);
		WRITE_MEM32(DSCPCR2,0);
		WRITE_MEM32(DSCPCR3,0);
		WRITE_MEM32(DSCPCR4,0);
		WRITE_MEM32(DSCPCR5,0);
		WRITE_MEM32(DSCPCR6,0);
	}


#if defined(RTL865X_TEST) || defined(RTL865X_MODEL_USER)
#if defined(VERA)||defined(VSV)||defined(MIILIKE)
	/* To speed up vera, we ignore set NAPT default value. */
#else
	/* ICMP Table: Set collision bit to 1 */
	memset( &naptIcmp, 0, sizeof(naptIcmp) );
	naptIcmp.isCollision = 1;
	for(flowTblIdx=0; flowTblIdx<RTL8651_ICMPTBL_SIZE; flowTblIdx++)
		rtl8651_setAsicNaptIcmpTable( TRUE, flowTblIdx, &naptIcmp );
#endif
#endif

#if 1
#if defined(CONFIG_RTL_8198_NFBI_BOARD)
    //WRITE_MEM32(PIN_MUX_SEL_2, 0); //for led control

	REG32(PCRP0) &= (0xFFFFFFFF-(0x00000000|MacSwReset));
	REG32(PCRP1) &= (0xFFFFFFFF-(0x00000000|MacSwReset));
	REG32(PCRP2) &= (0xFFFFFFFF-(0x00000000|MacSwReset));
	REG32(PCRP3) &= (0xFFFFFFFF-(0x00000000|MacSwReset));
	REG32(PCRP4) &= (0xFFFFFFFF-(0x00000000|MacSwReset));
	REG32(PCRP5) &= (0xFFFFFFFF-(0x00000000|MacSwReset));

	REG32(PCRP0) = REG32(PCRP0) | (0 << ExtPHYID_OFFSET) | EnablePHYIf | MacSwReset;
	REG32(PCRP1) = REG32(PCRP1) | (1 << ExtPHYID_OFFSET) | EnablePHYIf | MacSwReset;
	REG32(PCRP2) = REG32(PCRP2) | (2 << ExtPHYID_OFFSET) | EnablePHYIf | MacSwReset;
	REG32(PCRP3) = REG32(PCRP3) | (3 << ExtPHYID_OFFSET) | EnablePHYIf | MacSwReset;
	REG32(PCRP4) = REG32(PCRP4) | (4 << ExtPHYID_OFFSET) | EnablePHYIf | MacSwReset;

	//port5 STP forwarding?
	REG32(PITCR) = REG32(PITCR) & 0xFFFFF3FF; //configure port 5 to be a MII interface

	// for EMI issue, use "GMII/MII MAC auto mode" instead
	//rtl865xC_setAsicEthernetMIIMode(5, LINK_MII_PHY); //port 5 MII PHY mode
	rtl865xC_setAsicEthernetMIIMode(5, LINK_MII_MAC); //port 5 MII MAC mode
	REG32(P5GMIICR) = REG32(P5GMIICR) | 0x40; //Conf_done=1

#if defined(RTL8198_NFBI_PORT5_GMII) //GMII mode
           #define GMII_PIN_MUX 0xc0
           REG32(PIN_MUX_SEL)= REG32(PIN_MUX_SEL)&(~(GMII_PIN_MUX));
	REG32(PCRP5) = 0 | (0x10<<ExtPHYID_OFFSET) |
			EnForceMode| ForceLink|ForceSpeed1000M|ForceDuplex |
			MIIcfg_RXER | EnablePHYIf | MacSwReset;
#else //MII mode
	REG32(PCRP5) = 0 | (0x10<<ExtPHYID_OFFSET) |
			EnForceMode| ForceLink|ForceSpeed100M |ForceDuplex |
			MIIcfg_RXER | EnablePHYIf | MacSwReset;
#endif

#elif defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
	#define GMII_PIN_MUX 0xf00
       #if defined(CONFIG_8198_PORT5_GMII) || defined(CONFIG_8198_PORT5_RGMII)
           REG32(PIN_MUX_SEL)= REG32(PIN_MUX_SEL)&(~(GMII_PIN_MUX));
       #endif
	//WRITE_MEM32(PIN_MUX_SEL_2, 0);

#ifdef CONFIG_RTK_VOIP_GIGABYTE_PHY_LINK_MODE_100 // use 10/100 only
       WRITE_MEM32(PCRP0, READ_MEM32(PCRP0) & ~(1<<22));
       WRITE_MEM32(PCRP1, READ_MEM32(PCRP1) & ~(1<<22));
       WRITE_MEM32(PCRP2, READ_MEM32(PCRP2) & ~(1<<22));
       WRITE_MEM32(PCRP3, READ_MEM32(PCRP3) & ~(1<<22));
       WRITE_MEM32(PCRP4, READ_MEM32(PCRP4) & ~(1<<22));
#endif

       WRITE_MEM32(PCRP0, READ_MEM32(PCRP0) & ~MacSwReset);
       WRITE_MEM32(PCRP1, READ_MEM32(PCRP1) & ~MacSwReset);
       WRITE_MEM32(PCRP2, READ_MEM32(PCRP2) & ~MacSwReset);
       WRITE_MEM32(PCRP3, READ_MEM32(PCRP3) & ~MacSwReset);
       WRITE_MEM32(PCRP4, READ_MEM32(PCRP4) & ~MacSwReset);

       WRITE_MEM32(PCRP0, READ_MEM32(PCRP0) | ((0<<ExtPHYID_OFFSET)|EnablePHYIf|MacSwReset) ); /* Jumbo Frame */
       WRITE_MEM32(PCRP1, READ_MEM32(PCRP1) | ((1<<ExtPHYID_OFFSET)|EnablePHYIf|MacSwReset) ); /* Jumbo Frame */
       WRITE_MEM32(PCRP2, READ_MEM32(PCRP2) | ((2<<ExtPHYID_OFFSET)|EnablePHYIf|MacSwReset) ); /* Jumbo Frame */
       WRITE_MEM32(PCRP3, READ_MEM32(PCRP3) | ((3<<ExtPHYID_OFFSET)|EnablePHYIf|MacSwReset) ); /* Jumbo Frame */
       WRITE_MEM32(PCRP4, READ_MEM32(PCRP4) | ((4<<ExtPHYID_OFFSET)|EnablePHYIf|MacSwReset) ); /* Jumbo Frame */

#if defined(PORT5_RGMII_GMII)
		if(ExtP5GigaPhyMode)
		{
			REG32(PCRP5) &= (0x83FFFFFF-(0x00000000|MacSwReset));
			REG32(PCRP5) = REG32(PCRP5) | (GIGA_P5_PHYID << ExtPHYID_OFFSET) | EnablePHYIf | MacSwReset | 1<<20;
			REG32(P5GMIICR) = REG32(P5GMIICR) | Conf_done ;//| P5txdely;
		}
#endif
#if defined(CONFIG_8198_PORT5_GMII)
		REG32(PITCR) = REG32(PITCR) & 0xFFFFF3FF; //configure port 5 to be a MII interface
                rtl865xC_setAsicEthernetMIIMode(5, LINK_MII_MAC); //port 5  GMII/MII MAC auto mode
		REG32(P5GMIICR) = REG32(P5GMIICR) | 0x40; //Conf_done=1
                REG32(PCRP5) = 0 | (0x5<<ExtPHYID_OFFSET) |      //JSW@20100309:For external 8211BN GMII ,PHYID must be 5
                               	EnForceMode| ForceLink|ForceSpeed1000M |ForceDuplex |
                                MIIcfg_RXER | EnablePHYIf | MacSwReset;
#elif defined(CONFIG_8198_PORT5_RGMII)
		REG32(PITCR) = REG32(PITCR) & 0xFFFFF3FF; //configure port 5 to be a MII interface
		rtl865xC_setAsicEthernetMIIMode(5, LINK_RGMII); //port 5  GMII/MII MAC auto mode

		REG32(P5GMIICR) = REG32(P5GMIICR) | Conf_done;
                REG32(PCRP5) = 0 | (0x5<<ExtPHYID_OFFSET) |      //JSW@20100309:For external 8211BN GMII ,PHYID must be 5
                               	EnForceMode| ForceLink|ForceSpeed1000M |ForceDuplex |
                                MIIcfg_RXER | EnablePHYIf | MacSwReset;
#endif
	WRITE_MEM32(PCRP0,(REG32(PCRP0) & (0xFFFFFFFF-(MacSwReset))));
	WRITE_MEM32(PCRP0,(REG32(PCRP0) | (0 << ExtPHYID_OFFSET) | EnablePHYIf |MacSwReset));

#ifdef CONFIG_RTL_8197B
	/* disable MAC Gbe ability/Gbe 1000F ability of PHY. */
       WRITE_MEM32(PCRP0, READ_MEM32(PCRP0) & ~NwayAbility1000MF);
       WRITE_MEM32(PCRP1, READ_MEM32(PCRP1) & ~NwayAbility1000MF);
       WRITE_MEM32(PCRP2, READ_MEM32(PCRP2) & ~NwayAbility1000MF);
       WRITE_MEM32(PCRP3, READ_MEM32(PCRP3) & ~NwayAbility1000MF);
       WRITE_MEM32(PCRP4, READ_MEM32(PCRP4) & ~NwayAbility1000MF);
	Set_GPHYWB(999, 0, 9, 0, 0);
#endif

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
	{
	#define REG32_ANDOR(x,y,z)   (REG32(x)=(REG32(x)& (y))|(z))

#if defined(CONFIG_RTL_8211DS_SUPPORT)&&defined(CONFIG_RTL_8197D)
	int i;
	uint32 reg_tmp=0;
#endif

	//unsigned int rtl96d_P0phymode=Get_P0_PhyMode();
	unsigned int rtl96d_P0phymode=1;

#if defined(CONFIG_RTL_8211DS_SUPPORT)&&defined(CONFIG_RTL_8197D)
	rtl96d_P0phymode = 3;

#elif defined(CONFIG_RTL_8196E) //mark_es
	if ((REG32(BOND_OPTION) & BOND_ID_MASK) == BOND_8196ES) 
		rtl96d_P0phymode=Get_P0_PhyMode();	
#endif

	if(rtl96d_P0phymode==1)  //embedded phy
	{
		REG32(PCRP0) |=  (0 << ExtPHYID_OFFSET) | EnablePHYIf | MacSwReset;	//emabedded
	}
	else //external phy
	{
		unsigned int rtl96d_P0miimode=Get_P0_MiiMode();

	#if defined(CONFIG_RTL_8211DS_SUPPORT)&&defined(CONFIG_RTL_8197D)
		rtl96d_P0miimode = 3;
	#endif

		REG32(PCRP0) |=  (0x10 << ExtPHYID_OFFSET) | MIIcfg_RXER |  EnablePHYIf | MacSwReset;	//external

		if(rtl96d_P0miimode==0)
			REG32_ANDOR(P0GMIICR, ~(3<<23)  , LINK_MII_PHY<<23);
		else if(rtl96d_P0miimode==1)
			REG32_ANDOR(P0GMIICR, ~(3<<23)  , LINK_MII_MAC<<23);
		else if(rtl96d_P0miimode==2)
			REG32_ANDOR(P0GMIICR, ~(3<<23)  , LINK_MII_MAC<<23);  //GMII
		else if(rtl96d_P0miimode==3)
			REG32_ANDOR(P0GMIICR, ~(3<<23)  , LINK_RGMII<<23);

		if(rtl96d_P0miimode==3)
		{
			unsigned int rtl96d_P0txdly=Get_P0_TxDelay();
			unsigned int rtl96d_P0rxdly=Get_P0_RxDelay();
			REG32_ANDOR(P0GMIICR, ~((1<<4)|(3<<0)) , (rtl96d_P0txdly<<4) | (rtl96d_P0rxdly<<0) );

			#if defined(CONFIG_RTL_8211DS_SUPPORT)&&defined(CONFIG_RTL_8197D)
				//Set GPIOC0 to PHY reset.
				REG32_ANDOR(0xb8000044, 0xFFFFFFFF, (1<<20));
				REG32_ANDOR(0xb8003500, ~(1<<16), 0);
				REG32_ANDOR(0xb8003508, 0xFFFFFFFF, (1<<16));
				REG32_ANDOR(0xb800350c, ~(1<<16), 0); //Set Reset to low

				REG32(0xbb804104) &=0x3FFFFFF;
				REG32(0xbb804104) |=0x18FF0000;	//set PCR0 phyid
				REG32(0xbb80414c) =0x37d55;			//set port MII
				REG32(0xbb804100) =0x1;

				for(i=0; i<5; i++)
					REG32(PCRP0+i*4) |= (EnForceMode);

				REG32_ANDOR(0xb800350c, 0xFFFFFFFF, (1<<16));//Set Reset to high
				mdelay(30);
				//__delay(50000);

				rtl8651_getAsicEthernetPHYReg(0x6, 0, &reg_tmp);//Learning PHY ID

				for(i=0; i<5; i++)
					REG32(PCRP0+i*4) &= ~(EnForceMode);
			#endif
		}

		if(rtl96d_P0miimode==0)
			REG32_ANDOR(PCRP0, ~AutoNegoSts_MASK, EnForceMode| ForceLink|ForceSpeed100M |ForceDuplex) ;
		else if(rtl96d_P0miimode==1)
			REG32_ANDOR(PCRP0, ~AutoNegoSts_MASK, EnForceMode| ForceLink|ForceSpeed100M |ForceDuplex) ;
		else if(rtl96d_P0miimode==2)
			REG32_ANDOR(PCRP0, ~AutoNegoSts_MASK, EnForceMode| ForceLink|ForceSpeed1000M |ForceDuplex );
		#if !defined(CONFIG_RTL_8211DS_SUPPORT)
		else if(rtl96d_P0miimode==3)
			REG32_ANDOR(PCRP0, ~AutoNegoSts_MASK, EnForceMode| ForceLink|ForceSpeed1000M |ForceDuplex );
		#endif

		//REG32(P0GMIICR) |=(Conf_done); //mark_es
		REG32(PITCR) |= (1<<0);   //00: embedded , 01L GMII/MII/RGMII

		if((rtl96d_P0miimode==2)  ||(rtl96d_P0miimode==3)) {
			REG32(MACCR) |= (1<<12);   //giga link
		}
		REG32(P0GMIICR) |=(Conf_done); //mark_es
	}
	}
#endif

#elif defined(CONFIG_RTL_8196C)
    WRITE_MEM32(PCRP0, (READ_MEM32(PCRP0)&(0xFFFFFFFF-(0x00400000|MacSwReset))) );
	TOGGLE_BIT_IN_REG_TWICE(PCRP0,EnForceMode);

    WRITE_MEM32(PCRP1, (READ_MEM32(PCRP1)&(0xFFFFFFFF-(0x00400000|MacSwReset))) );
	TOGGLE_BIT_IN_REG_TWICE(PCRP1,EnForceMode);
    WRITE_MEM32(PCRP2, (READ_MEM32(PCRP2)&(0xFFFFFFFF-(0x00400000|MacSwReset))) );
	TOGGLE_BIT_IN_REG_TWICE(PCRP2,EnForceMode);
    WRITE_MEM32(PCRP3, (READ_MEM32(PCRP3)&(0xFFFFFFFF-(0x00400000|MacSwReset))) );
	TOGGLE_BIT_IN_REG_TWICE(PCRP3,EnForceMode);
    WRITE_MEM32(PCRP4, (READ_MEM32(PCRP4)&(0xFFFFFFFF-(0x00400000|MacSwReset))) );
	TOGGLE_BIT_IN_REG_TWICE(PCRP4,EnForceMode);

    WRITE_MEM32(PCRP0, (READ_MEM32(PCRP0)|(rtl8651AsicEthernetTable[0].phyId<<ExtPHYID_OFFSET)|EnablePHYIf|MacSwReset ) ); /* Jumbo Frame */
	TOGGLE_BIT_IN_REG_TWICE(PCRP0,EnForceMode);
    WRITE_MEM32(PCRP1, (READ_MEM32(PCRP1)|(rtl8651AsicEthernetTable[1].phyId<<ExtPHYID_OFFSET)|EnablePHYIf|MacSwReset ) ); /* Jumbo Frame */
	TOGGLE_BIT_IN_REG_TWICE(PCRP1,EnForceMode);
    WRITE_MEM32(PCRP2, (READ_MEM32(PCRP2)|(rtl8651AsicEthernetTable[2].phyId<<ExtPHYID_OFFSET)|EnablePHYIf|MacSwReset ) ); /* Jumbo Frame */
	TOGGLE_BIT_IN_REG_TWICE(PCRP2,EnForceMode);
    WRITE_MEM32(PCRP3, (READ_MEM32(PCRP3)|(rtl8651AsicEthernetTable[3].phyId<<ExtPHYID_OFFSET)|EnablePHYIf|MacSwReset ) ); /* Jumbo Frame */
	TOGGLE_BIT_IN_REG_TWICE(PCRP3,EnForceMode);
    WRITE_MEM32(PCRP4, (READ_MEM32(PCRP4)|(rtl8651AsicEthernetTable[4].phyId<<ExtPHYID_OFFSET)|EnablePHYIf|MacSwReset ) ); /* Jumbo Frame */
	TOGGLE_BIT_IN_REG_TWICE(PCRP4,EnForceMode);
#else
    WRITE_MEM32(PCRP0, (READ_MEM32(PCRP0)|(rtl8651AsicEthernetTable[0].phyId<<ExtPHYID_OFFSET)|EnablePHYIf ) ); /* Jumbo Frame */
    WRITE_MEM32(PCRP1, (READ_MEM32(PCRP1)|(rtl8651AsicEthernetTable[1].phyId<<ExtPHYID_OFFSET)|EnablePHYIf ) ); /* Jumbo Frame */
    WRITE_MEM32(PCRP2, (READ_MEM32(PCRP2)|(rtl8651AsicEthernetTable[2].phyId<<ExtPHYID_OFFSET)|EnablePHYIf ) ); /* Jumbo Frame */
    WRITE_MEM32(PCRP3, (READ_MEM32(PCRP3)|(rtl8651AsicEthernetTable[3].phyId<<ExtPHYID_OFFSET)|EnablePHYIf ) ); /* Jumbo Frame */
    WRITE_MEM32(PCRP4, (READ_MEM32(PCRP4)|(rtl8651AsicEthernetTable[4].phyId<<ExtPHYID_OFFSET)|EnablePHYIf ) ); /* Jumbo Frame */
#endif


    if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
    {
        WRITE_MEM32(PCRP5, ( (READ_MEM32(PCRP5))|(rtl8651AsicEthernetTable[5].phyId<<ExtPHYID_OFFSET)|EnablePHYIf ) ); /* Jumbo Frame */
    }

#if 0  /* No need to set PHYID of port 6. Just use ASIC default value. */
       /*  Due to MSb of phyid has been added an inverter in b-cut,
         *  although we want to set 6(0b00110) as phyid, we have to write 22(0b10110) instead. */
        WRITE_MEM32( PCRP6, (READ_MEM32(PCRP6)|( 22 << ExtPHYID_OFFSET ) | AcptMaxLen_16K | EnablePHYIf );
#endif
    if(RTL865X_PHY6_DSP_BUG)
        WRITE_MEM32(PCRP6, (READ_MEM32(PCRP6)|(6<<ExtPHYID_OFFSET)|EnablePHYIf ) );
    /* Set PHYID 6 to PCRP6. (By default, PHYID of PCRP6 is 0. It will collide with PHYID of port 0. */
#endif

#if defined(CONFIG_RTL865X_DIAG_LED)
	/* diagnosis led (gpio-porta-6) on */
	/* pull high by set portA-0(bit 30) as gpio-output-1, meaning: diag led OFF */
	REG32(PABDAT) |=  0x40000000;
#endif /* CONFIG_RTL865X_DIAG_LED */

#if !defined(CONFIG_RTL_8196C) && !defined(CONFIG_RTL_8198)
	//REG32(MDCIOCR) = 0x96181441;	// enable Giga port 8211B LED
#endif

	/*disable pattern match*/
	{
		int pnum;
		 for(pnum=0;pnum<RTL8651_PORT_NUMBER;pnum++)
	        {
	                rtl8651_setAsicPortPatternMatch(pnum, 0, 0, 0x2);
	        }
	}

#if defined(CONFIG_RTL_8196C)
	//enable RTL8196C 10M power saving mode
	{
		int tmp,idx;

		for(idx=0;idx<MAX_PORT_NUMBER;idx++) {
			rtl8651_getAsicEthernetPHYReg( idx, 0x18, &tmp );
#ifdef CONFIG_RTL8196C_GREEN_ETHERNET
			tmp |= BIT(15); //enable power saving mode
#else
			tmp &= ~BIT(15); //disable power saving mode
#endif
			rtl8651_setAsicEthernetPHYReg( idx, 0x18, tmp );
		}
	}
	REG32(MPMR) |= PM_MODE_ENABLE_AUTOMATIC_POWER_DOWN;

//	#define PIN_MUX_SEL 0xb8000040
	// Configure LED-SIG0/LED-SIG1/LED-SIG2/LED-SIG3/LED-PHASE0/LED-PHASE1/LED-PHASE2/LED-PHASE3 PAD as LED-SW

#ifndef CONFIG_POCKET_ROUTER_SUPPORT
	REG32(PIN_MUX_SEL) &= ~(0xFFFF);
#endif

#if defined(PATCH_GPIO_FOR_LED)
	REG32(PIN_MUX_SEL) |= (0xFFFF);
#endif

#endif  // end of defined(CONFIG_RTL_8196C)

#if defined(CONFIG_RTL_8198) && !defined(CONFIG_RTL_819XD)
        REG32(MPMR) |= PM_MODE_ENABLE_AUTOMATIC_POWER_DOWN;
#endif


#if defined(CONFIG_RTL_8196E) && defined(CONFIG_RTL_ULINKER) /* disable unused port for saving power */
	{
	uint32 statCtrlReg0;

	for (index=0; index<4; index++) {
		/* read current PHY reg 0 value */
		rtl8651_getAsicEthernetPHYReg( index, 0, &statCtrlReg0 );

		REG32(PCRP0+(index*4)) |= EnForceMode;
		statCtrlReg0 |= POWER_DOWN;

		/* write PHY reg 0 */
		rtl8651_setAsicEthernetPHYReg( index, 0, statCtrlReg0 );
	}
	}
#endif
#if  defined(CONFIG_RTL_8196E) //mark_es
	if ((REG32(BOND_OPTION) & BOND_ID_MASK) == BOND_8196ES) 
	{
		uint32 statCtrlReg0;

		for (index=1; index<5; index++) {
		/* read current PHY reg 0 value */
		rtl8651_getAsicEthernetPHYReg( index, 0, &statCtrlReg0 );

		REG32(PCRP0+(index*4)) |= EnForceMode;
		statCtrlReg0 |= POWER_DOWN;

		/* write PHY reg 0 */
		rtl8651_setAsicEthernetPHYReg( index, 0, statCtrlReg0 );              
		}
	}
#endif	

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
#if defined(PATCH_GPIO_FOR_LED)
	REG32(PIN_MUX_SEL2) |= (0x3FFF);
#endif
#endif

	return SUCCESS;
}

int32 rtl8651_setAsicPortPatternMatch(uint32 port, uint32 pattern, uint32 patternMask, int32 operation) {
	//not for ext port
	if(port>=RTL8651_PORT_NUMBER)
		return FAILED;

	if(pattern==0&&patternMask==0){
		//bit 0~13 is reseved...
		//if((READ_MEM32(PPMAR)&0x2000)==0) //system pattern match not enabled.
			//return SUCCESS;
		WRITE_MEM32(PPMAR,READ_MEM32(PPMAR)&~(1<<(port+26)));
		if((READ_MEM32(PPMAR)&0xfc000000)==0)
			WRITE_MEM32(PPMAR,READ_MEM32(PPMAR)&~(1<<13)); //turn off system pattern match switch.

		return SUCCESS;
	}
	if(operation>3)
		return FAILED; //valid operations: 0(drop), 1(mirror to cpu),2(fwd to cpu), 3(to mirror port)
	WRITE_MEM32(PPMAR,READ_MEM32(PPMAR)|((1<<(port+26))|(1<<13))); //turn on system pattern match and turn on pattern match on indicated port.
	WRITE_MEM32(PPMAR,(READ_MEM32(PPMAR) & (~(0x3<<(14+2*port))))|(operation<<(14+2*port)));   //specify operation
	WRITE_MEM32(PATP0+4*port,pattern);
	WRITE_MEM32(MASKP0+4*port,patternMask);
	return SUCCESS;
}

int32 rtl8651_getAsicPortPatternMatch(uint32 port, uint32 *pattern, uint32 *patternMask, int32 *operation)
{
	//not for ext port
	if(port>=RTL8651_PORT_NUMBER)
		return FAILED;
	if(((READ_MEM32(PPMAR)& (1<<13))==0)||((READ_MEM32(PPMAR)& (1<<(26+port)))==0))
		return FAILED;
	if(pattern)
		*pattern=READ_MEM32(PATP0+4*port);
	if(patternMask)
		*patternMask=READ_MEM32(MASKP0+4*port);
	if(operation)
		*operation=(READ_MEM32(PPMAR)>>(14+2*port))&0x3;
	return SUCCESS;
}

/*
@func int32		| rtl8651_setAsicSpanningEnable 	| Enable/disable ASIC spanning tree support
@parm int8		| spanningTreeEnabled | TRUE to indicate spanning tree is enabled; FALSE to indicate spanning tree is disabled.
@rvalue SUCCESS	| 	Success
@comm
Global switch to enable or disable ASIC spanning tree support.
If ASIC spanning tree support is enabled, further configuration would be refered by ASIC to prcoess packet forwarding / MAC learning.
If ASIC spanning tree support is disabled, all MAC learning and packet forwarding would be done regardless of port state.
Note that the configuration does not take effect for spanning tree BPDU CPU trapping. It is set in <p rtl8651_setAsicResvMcastAddrToCPU()>.
@xref <p rtl8651_setAsicMulticastSpanningTreePortState()>, <p rtl865xC_setAsicSpanningTreePortState()>, <p rtl8651_getAsicMulticastSpanningTreePortState()>, <p rtl865xC_getAsicSpanningTreePortState()>
 */
int32 rtl8651_setAsicSpanningEnable(int8 spanningTreeEnabled)
{
	if(spanningTreeEnabled == TRUE)
	{
		WRITE_MEM32(MSCR,READ_MEM32(MSCR)|(EN_STP));
		WRITE_MEM32(RMACR ,READ_MEM32(RMACR)|MADDR00);

	}else
	{
		WRITE_MEM32(MSCR,READ_MEM32(MSCR)&~(EN_STP));
		WRITE_MEM32(RMACR, READ_MEM32(RMACR)&~MADDR00);

	}
	return SUCCESS;
}

/*
@func int32		| rtl8651_getAsicSpanningEnable 	| Getting the ASIC spanning tree support status
@parm int8*		| spanningTreeEnabled | The pointer to get the status of ASIC spanning tree configuration status.
@rvalue FAILED	| 	Failed
@rvalue SUCCESS	| 	Success
@comm
Get the ASIC global switch to enable or disable ASIC spanning tree support.
The switch can be set by calling <p rtl8651_setAsicSpanningEnable()>
@xref <p rtl8651_setAsicSpanningEnable()>, <p rtl8651_setAsicMulticastSpanningTreePortState()>, <p rtl865xC_setAsicSpanningTreePortState()>, <p rtl8651_getAsicMulticastSpanningTreePortState()>, <p rtl865xC_getAsicSpanningTreePortState()>
 */
int32 rtl8651_getAsicSpanningEnable(int8 *spanningTreeEnabled)
{
	if(spanningTreeEnabled == NULL)
		return FAILED;
	*spanningTreeEnabled = (READ_MEM32(MSCR)&(EN_STP)) == (EN_STP)? TRUE: FALSE;
	return SUCCESS;
}

/*
@func int32		| rtl865xC_setAsicSpanningTreePortState 	| Configure Spanning Tree Protocol Port State
@parm uint32 | port | port number under consideration
@parm uint32 | portState | Spanning tree port state: RTL8651_PORTSTA_DISABLED, RTL8651_PORTSTA_BLOCKING, RTL8651_PORTSTA_LISTENING, RTL8651_PORTSTA_LEARNING, RTL8651_PORTSTA_FORWARDING
@rvalue SUCCESS	| 	Success
@rvalue FAILED | Failed
@comm
Config IEEE 802.1D spanning tree port sate into ASIC.
 */
int32 rtl865xC_setAsicSpanningTreePortState(uint32 port, uint32 portState)
{
	uint32 offset = port * 4;

	if ( port >= RTL865XC_PORT_NUMBER )
		return FAILED;

	switch(portState)
	{
		case RTL8651_PORTSTA_DISABLED:
			WRITE_MEM32( PCRP0 + offset, ( READ_MEM32( PCRP0 + offset ) & (~STP_PortST_MASK) ) | STP_PortST_DISABLE );
			break;
		case RTL8651_PORTSTA_BLOCKING:
		case RTL8651_PORTSTA_LISTENING:
			WRITE_MEM32( PCRP0 + offset, ( READ_MEM32( PCRP0 + offset ) & (~STP_PortST_MASK) ) | STP_PortST_BLOCKING );
			break;
		case RTL8651_PORTSTA_LEARNING:
			WRITE_MEM32( PCRP0 + offset, ( READ_MEM32( PCRP0 + offset ) & (~STP_PortST_MASK) ) | STP_PortST_LEARNING );
			break;
		case RTL8651_PORTSTA_FORWARDING:
			WRITE_MEM32( PCRP0 + offset, ( READ_MEM32( PCRP0 + offset ) & (~STP_PortST_MASK) ) | STP_PortST_FORWARDING );
			break;
		default:
			return FAILED;
	}

	TOGGLE_BIT_IN_REG_TWICE(PCRP0 + offset, EnForceMode);
	return SUCCESS;
}

/*
@func int32		| rtl865xC_getAsicSpanningTreePortState 	| Retrieve Spanning Tree Protocol Port State
@parm uint32 | port | port number under consideration
@parm uint32 | portState | pointer to memory to store the port state
@rvalue SUCCESS	| 	Success
@rvalue FAILED | Failed
@comm
Possible spanning tree port state: RTL8651_PORTSTA_DISABLED, RTL8651_PORTSTA_BLOCKING, RTL8651_PORTSTA_LISTENING, RTL8651_PORTSTA_LEARNING, RTL8651_PORTSTA_FORWARDING
 */
int32 rtl865xC_getAsicSpanningTreePortState(uint32 port, uint32 *portState)
{
	uint32 reg;
	uint32 offset = port * 4;

	if ( port >= RTL865XC_PORT_NUMBER || portState == NULL )
		return FAILED;

	reg = ( READ_MEM32( PCRP0 + offset ) & STP_PortST_MASK );

	switch(reg)
	{
		case STP_PortST_DISABLE:
			*portState = RTL8651_PORTSTA_DISABLED;
			break;
		case STP_PortST_BLOCKING:
			*portState = RTL8651_PORTSTA_BLOCKING;
			break;
		case STP_PortST_LEARNING:
			*portState = RTL8651_PORTSTA_LEARNING;
			break;
		case STP_PortST_FORWARDING:
			*portState = RTL8651_PORTSTA_FORWARDING;
			break;
		default:
			return FAILED;
	}
	return SUCCESS;

}

/*
@func int32		| rtl8651_setAsicMulticastSpanningTreePortState 	| Configure Multicast Spanning Tree Protocol Port State
@parm uint32 | port | port number under consideration
@parm uint32 | portState | Spanning tree port state: RTL8651_PORTSTA_DISABLED, RTL8651_PORTSTA_BLOCKING, RTL8651_PORTSTA_LISTENING, RTL8651_PORTSTA_LEARNING, RTL8651_PORTSTA_FORWARDING
@rvalue SUCCESS	| 	Success
@rvalue FAILED | Failed
@comm
In RTL865xC platform, Multicast spanning tree configuration is set by this API.
@xref  <p rtl865xC_setAsicSpanningTreePortState()>
 */
int32 rtl8651_setAsicMulticastSpanningTreePortState(uint32 port, uint32 portState)
{
#if 0	//Note: 96C/98 have remove these bits!!!
	uint32 offset = port * 4;

	if ( port >= RTL865XC_PORT_NUMBER )
	{
		return FAILED;
	}

	switch(portState)
	{
		case RTL8651_PORTSTA_DISABLED:
			WRITE_MEM32( PCRP0 + offset, ( READ_MEM32( PCRP0 + offset ) & (~IPMSTP_PortST_MASK) ) | IPMSTP_PortST_DISABLE );
			break;
		case RTL8651_PORTSTA_BLOCKING:
		case RTL8651_PORTSTA_LISTENING:
			WRITE_MEM32( PCRP0 + offset, ( READ_MEM32( PCRP0 + offset ) & (~IPMSTP_PortST_MASK) ) | IPMSTP_PortST_BLOCKING );
			break;
		case RTL8651_PORTSTA_LEARNING:
			WRITE_MEM32( PCRP0 + offset, ( READ_MEM32( PCRP0 + offset ) & (~IPMSTP_PortST_MASK) ) | IPMSTP_PortST_LEARNING );
			break;
		case RTL8651_PORTSTA_FORWARDING:
			WRITE_MEM32( PCRP0 + offset, ( READ_MEM32( PCRP0 + offset ) & (~IPMSTP_PortST_MASK) ) | IPMSTP_PortST_FORWARDING );
			break;
		default:
			return FAILED;
	}

	TOGGLE_BIT_IN_REG_TWICE(PCRP0 + offset,EnForceMode);
#endif

	return SUCCESS;
}

/*
@func int32		| rtl8651_getAsicMulticastSpanningTreePortState 	| Retrieve Spanning Tree Protocol Port State
@parm uint32 | port | port number under consideration
@parm uint32 | portState | pointer to memory to store the port state
@rvalue SUCCESS	| 	Success
@rvalue FAILED | Failed
@comm
In RTL865xC platform, Multicast spanning tree configuration is gotten by this API.
@xref  <p rtl865xC_getAsicSpanningTreePortState()>
 */
int32 rtl8651_getAsicMulticastSpanningTreePortState(uint32 port, uint32 *portState)
{
	uint32 reg;
	uint32 offset = port * 4;

	if ( port >= RTL865XC_PORT_NUMBER || portState == NULL )
		return FAILED;

	reg = ( READ_MEM32( PCRP0 + offset ) & IPMSTP_PortST_MASK );

	switch(reg)
	{
		case IPMSTP_PortST_DISABLE:
			*portState = RTL8651_PORTSTA_DISABLED;
			break;
		case IPMSTP_PortST_BLOCKING:
			*portState = RTL8651_PORTSTA_BLOCKING;
			break;
		case IPMSTP_PortST_LEARNING:
			*portState = RTL8651_PORTSTA_LEARNING;
			break;
		case IPMSTP_PortST_FORWARDING:
			*portState = RTL8651_PORTSTA_FORWARDING;
			break;
		default:
			return FAILED;
	}
	return SUCCESS;
}

/*=========================================
  * ASIC DRIVER API: MDC/MDIO Control
  *=========================================*/

int32 rtl8651_getAsicEthernetPHYReg(uint32 phyId, uint32 regId, uint32 *rData)
{
	uint32 status;

	WRITE_MEM32( MDCIOCR, COMMAND_READ | ( phyId << PHYADD_OFFSET ) | ( regId << REGADD_OFFSET ) );

#if defined(CONFIG_RTL_8198)
	if (REG32(REVR) == BSP_RTL8198_REVISION_A)
		mdelay(10);
#elif defined(CONFIG_RTL8196C_REVISION_B)
	if (REG32(REVR) == RTL8196C_REVISION_A)
		mdelay(10);	//wei add, for 8196C revision A. mdio data read will delay 1 mdc clock.
#endif

	do { status = READ_MEM32( MDCIOSR ); } while ( ( status & MDC_STATUS ) != 0 );

	status &= 0xffff;
	*rData = status;

	return SUCCESS;
}

int32 rtl8651_setAsicEthernetPHYReg(uint32 phyId, uint32 regId, uint32 wData)
{
	WRITE_MEM32( MDCIOCR, COMMAND_WRITE | ( phyId << PHYADD_OFFSET ) | ( regId << REGADD_OFFSET ) | wData );

	while( ( READ_MEM32( MDCIOSR ) & MDC_STATUS ) != 0 );		/* wait until command complete */

	return SUCCESS;
}

int32 rtl8651_getAsicEthernetPHYStatus(uint32 port, uint32 *rData)
{
	uint32 statCtrlReg1, phyid;

	/* port number validation */
	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
	{
		if ( port > RTL8651_MAC_NUMBER )
		{
			return FAILED;
		}
	} else
	{
		if ( port > RTL8651_PHY_NUMBER )
		{
			return FAILED;
		}
	}

	/* PHY id determination */
	phyid = rtl8651AsicEthernetTable[port].phyId;

	/* read current PHY reg 1*/
	rtl8651_getAsicEthernetPHYReg( phyid, 1, &statCtrlReg1 );

	/*assign value*/
	*rData=statCtrlReg1;
	return SUCCESS;
}

int32 rtl8651_restartAsicEthernetPHYNway(uint32 port)
{
	uint32 statCtrlReg0, phyid;

	/* port number validation */
	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
	{
		if ( port > RTL8651_MAC_NUMBER )
		{
			return FAILED;
		}
	} else
	{
		if ( port > RTL8651_PHY_NUMBER )
		{
			return FAILED;
		}
	}

	/* PHY id determination */
	phyid = rtl8651AsicEthernetTable[port].phyId;

	/* read current PHY reg 0 */
	rtl8651_getAsicEthernetPHYReg( phyid, 0, &statCtrlReg0 );

	/* enable 'restart Nway' bit */
	statCtrlReg0 |= RESTART_AUTONEGO;

	/* write PHY reg 0 */
	rtl8651_setAsicEthernetPHYReg( phyid, 0, statCtrlReg0 );

	return SUCCESS;
}

int32 rtl8651_setAsicEthernetPHYPowerDown( uint32 port, uint32 pwrDown )
{
	uint32 statCtrlReg0, phyid;

	/* port number validation */
	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
	{
		if ( port > RTL8651_MAC_NUMBER )
		{
			return FAILED;
		}
	} else
	{
		if ( port > RTL8651_PHY_NUMBER )
		{
			return FAILED;
		}
	}

	/* PHY id determination */
	phyid = rtl8651AsicEthernetTable[port].phyId;

	/* read current PHY reg 0 value */
	rtl8651_getAsicEthernetPHYReg( phyid, 0, &statCtrlReg0 );

	if ( pwrDown ){
		REG32(PCRP0+port*4) |= EnForceMode;
                REG32(PCRP0+port*4) &= ~ForceLink;
		statCtrlReg0 |= POWER_DOWN;
	}
	else{
		REG32(PCRP0+port*4) &= ~EnForceMode;
		statCtrlReg0 &= ~POWER_DOWN;
	}
	/* write PHY reg 0 */
	rtl8651_setAsicEthernetPHYReg( phyid, 0, statCtrlReg0 );

	return SUCCESS;

}

int32 rtl8651_setAsicEthernetPHYAdvCapality(uint32 port, uint32 capality)
{
	uint32 statCtrlReg4, phyid;

	/* port number validation */
	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
	{
		if ( port > RTL8651_MAC_NUMBER )
		{
			return FAILED;
		}
	} else
	{
		if ( port > RTL8651_PHY_NUMBER )
		{
			return FAILED;
		}
	}

	/* PHY id determination */
	phyid = rtl8651AsicEthernetTable[port].phyId;

	/* read current PHY reg 4 value */
	rtl8651_getAsicEthernetPHYReg( phyid, 4, &statCtrlReg4);

	/*Clear Duplex and Speed bits*/
	statCtrlReg4 &= ~(0xF<<5);

	if (capality & (1<<DUPLEX_100M))
	{
		statCtrlReg4 |= (1<<8);
	}
	if (capality & (1<<HALF_DUPLEX_100M))
	{
		statCtrlReg4 |= (1<<7);
	}
	if (capality & (1<<DUPLEX_10M))
	{
		statCtrlReg4 |= (1<<6);
	}
	if (capality & (1<<HALF_DUPLEX_10M))
	{
		statCtrlReg4 |= (1<<5);
	}
	if(capality & (1<<PORT_AUTO))
	{
		/*Set All Duplex and Speed All Supported*/
		statCtrlReg4 |=(0xF <<5);
	}

	/* write PHY reg 4 */
	rtl8651_setAsicEthernetPHYReg( phyid, 4, statCtrlReg4 );

	return SUCCESS;
}
int32 rtl8651_setAsicEthernetPHYSpeed( uint32 port, uint32 speed )
{
	uint32 statCtrlReg0, phyid;

	/* port number validation */
	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
	{
		if ( port > RTL8651_MAC_NUMBER )
		{
			return FAILED;
		}
	} else
	{
		if ( port > RTL8651_PHY_NUMBER )
		{
			return FAILED;
		}
	}

	/* PHY id determination */
	phyid = rtl8651AsicEthernetTable[port].phyId;

	/* read current PHY reg 0 value */
	rtl8651_getAsicEthernetPHYReg( phyid, 0, &statCtrlReg0 );

	if (0 == speed )
	{
		/*10M*/
		statCtrlReg0 &= ~SPEED_SELECT_100M;
	}
	else if (1 == speed)
	{
		/*100M*/
		statCtrlReg0 |= SPEED_SELECT_100M;
	}
	else if(2 == speed)
	{
		/*1000M*/
	}

	/* write PHY reg 0 */
	rtl8651_setAsicEthernetPHYReg( phyid, 0, statCtrlReg0 );

	return SUCCESS;

}

int32 rtl8651_setAsicEthernetPHYDuplex( uint32 port, uint32 duplex )
{
	uint32 statCtrlReg0, phyid;

	/* port number validation */
	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
	{
		if ( port > RTL8651_MAC_NUMBER )
		{
			return FAILED;
		}
	} else
	{
		if ( port > RTL8651_PHY_NUMBER )
		{
			return FAILED;
		}
	}

	/* PHY id determination */
	phyid = rtl8651AsicEthernetTable[port].phyId;

	/* read current PHY reg 0 value */
	rtl8651_getAsicEthernetPHYReg( phyid, 0, &statCtrlReg0 );

	if ( duplex )
		statCtrlReg0 |= SELECT_FULL_DUPLEX;
	else
		statCtrlReg0 &= ~SELECT_FULL_DUPLEX;

	/* write PHY reg 0 */
	rtl8651_setAsicEthernetPHYReg( phyid, 0, statCtrlReg0 );

	return SUCCESS;

}

int32 rtl8651_setAsicEthernetPHYAutoNeg( uint32 port, uint32 autoneg)
{
	uint32 statCtrlReg0, phyid;

	/* port number validation */
	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
	{
		if ( port > RTL8651_MAC_NUMBER )
		{
			return FAILED;
		}
	} else
	{
		if ( port > RTL8651_PHY_NUMBER )
		{
			return FAILED;
		}
	}

	/* PHY id determination */
	phyid = rtl8651AsicEthernetTable[port].phyId;

	/* read current PHY reg 0 value */
	rtl8651_getAsicEthernetPHYReg( phyid, 0, &statCtrlReg0 );

	if (autoneg)
		statCtrlReg0 |= ENABLE_AUTONEGO;
	else
		statCtrlReg0 &= ~ENABLE_AUTONEGO;

	/* write PHY reg 0 */
	rtl8651_setAsicEthernetPHYReg( phyid, 0, statCtrlReg0 );

	return SUCCESS;

}

int32 rtl865xC_setAsicPortPauseFlowControl(uint32 port, uint8 rxEn, uint8 txEn)
{
	uint32 offset = port<<2;
	uint32 pauseFC = 0;

	if(rxEn!=0)
		pauseFC |= PauseFlowControlDtxErx;
	if(txEn!=0)
		pauseFC |= PauseFlowControlEtxDrx;

	WRITE_MEM32(PCRP0+offset, (~(PauseFlowControl_MASK)&(READ_MEM32(PCRP0+offset)))|pauseFC);

	TOGGLE_BIT_IN_REG_TWICE(PCRP0 + offset,EnForceMode);
	return SUCCESS;
}

int32 rtl865xC_getAsicPortPauseFlowControl(uint32 port, uint8 *rxEn, uint8 *txEn)
{
	uint32 offset = port<<2;
	uint32 pauseFC = 0;

	pauseFC = ((PauseFlowControl_MASK)&(READ_MEM32(PCRP0+offset)));

	if (pauseFC&PauseFlowControlDtxErx)
		*rxEn = TRUE;

	if (pauseFC&PauseFlowControlEtxDrx)
		*txEn = TRUE;

	return SUCCESS;
}


int32 rtl8651_asicEthernetCableMeterInit(void)
{
	rtlglue_printf("NOT YET\n");

#if 0
	uint32 old_value;
	//set PHY6 Reg0 TxD latch internal clock phase
	WRITE_MEM32(SWTAA, 0xbc8020c0);
	WRITE_MEM32(TCR0,0x56);
 	WRITE_MEM32(SWTACR,ACTION_START | CMD_FORCE);
 #ifndef RTL865X_TEST
	while ( (READ_MEM32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command done
#endif /* RTL865X_TEST */
	//set PHY7 Reg 5
	WRITE_MEM32(SWTAA, 0xbc8020f4);
	WRITE_MEM32(TCR0,0x4b68);
 	WRITE_MEM32(SWTACR,ACTION_START | CMD_FORCE);
 #ifndef RTL865X_TEST
	while ( (READ_MEM32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command done
#endif /* RTL865X_TEST */
	//set PHY7 Reg 6
	WRITE_MEM32(SWTAA, 0xbc8020f8);
	WRITE_MEM32(TCR0,0x0380);
 	WRITE_MEM32(SWTACR,ACTION_START | CMD_FORCE);
 #ifndef RTL865X_TEST
	while ( (READ_MEM32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command done
#endif /* RTL865X_TEST */
	old_value=READ_MEM32(0xbc8020fc);
	//set PHY7 Reg 7
	WRITE_MEM32(SWTAA, 0xbc8020fc);
	WRITE_MEM32(TCR0,old_value|0x300);
 	WRITE_MEM32(SWTACR,ACTION_START | CMD_FORCE);
 #ifndef RTL865X_TEST
	while ( (READ_MEM32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command done
#endif /* RTL865X_TEST */
#endif
return SUCCESS;
}


/*=========================================
  * ASIC DRIVER API: ETHERNET MII
  *=========================================*/
int32 rtl865xC_setAsicEthernetMIIMode(uint32 port, uint32 mode)
{
	if ( port != 0 && port != RTL8651_MII_PORTNUMBER )
		return FAILED;
	if ( mode != LINK_RGMII && mode != LINK_MII_MAC && mode != LINK_MII_PHY )
		return FAILED;

	if ( port == 0 )
	{
		/* MII port MAC interface mode configuration */
		WRITE_MEM32( P0GMIICR, ( READ_MEM32( P0GMIICR ) & ~CFG_GMAC_MASK ) | ( mode << LINKMODE_OFFSET ) );
	}
	else
	{
		/* MII port MAC interface mode configuration */
		WRITE_MEM32( P5GMIICR, ( READ_MEM32( P5GMIICR ) & ~CFG_GMAC_MASK ) | ( mode << LINKMODE_OFFSET ) );
	}
	return SUCCESS;

}

int32 rtl865xC_setAsicEthernetRGMIITiming(uint32 port, uint32 Tcomp, uint32 Rcomp)
{
	if ( port != 0 && port != RTL8651_MII_PORTNUMBER )
		return FAILED;
	if ( Tcomp < RGMII_TCOMP_0NS || Tcomp > RGMII_TCOMP_7NS || Rcomp < RGMII_RCOMP_0NS || Rcomp > RGMII_RCOMP_2DOT5NS )
		return FAILED;

	if ( port == 0 )
	{
		WRITE_MEM32(P0GMIICR, ( ( ( READ_MEM32(P0GMIICR) & ~RGMII_TCOMP_MASK ) | Tcomp ) & ~RGMII_RCOMP_MASK ) | Rcomp );
	}
	else
	{
		WRITE_MEM32(P5GMIICR, ( ( ( READ_MEM32(P5GMIICR) & ~RGMII_TCOMP_MASK ) | Tcomp ) & ~RGMII_RCOMP_MASK ) | Rcomp );
	}

	return SUCCESS;
}

/* For backward-compatible issue, this API is used to set MII port 5. */
int32 rtl8651_setAsicEthernetMII(uint32 phyAddress, int32 mode, int32 enabled)
{
	/* Input validation */
	if ( phyAddress < 0 || phyAddress > 31 )
		return FAILED;
	if ( mode != P5_LINK_RGMII && mode != P5_LINK_MII_MAC && mode != P5_LINK_MII_PHY )
		return FAILED;

	/* Configure driver level information about mii port 5 */
	if ( enabled )
	{
		if ( miiPhyAddress >= 0 && miiPhyAddress != phyAddress )
			return FAILED;

		miiPhyAddress = phyAddress;
	}
	else
	{
		miiPhyAddress = -1;
	}

	/* MII port MAC interface mode configuration */
	WRITE_MEM32( P5GMIICR, ( READ_MEM32( P5GMIICR ) & ~CFG_GMAC_MASK ) | ( mode << P5_LINK_OFFSET ) );

	return SUCCESS;
}

int32 rtl8651_getAsicEthernetMII(uint32 *phyAddress)
{
	*phyAddress=miiPhyAddress;
	return SUCCESS;
}


/*vlan remark*/
int32 rtl8651_setAsicVlanRemark(enum PORTID port, enum PRIORITYVALUE priority, int remark)
{
	int regValue;
	if ((port < PHY0) || (port > CPU) || (priority < PRI0) || (priority > PRI7) ||(remark < PRI0) || (remark > PRI7))
		return FAILED;

	WRITE_MEM32(RLRC, (READ_MEM32(RLRC) &~(0x7)) |0x7);

	regValue = READ_MEM32(RMCR1P) & ~((0x7<<(3*priority)) |(0x1 << (24 + port)));
	regValue |= ((remark << (3*priority)) |(0x1 << (24 + port)));
	WRITE_MEM32( RMCR1P, regValue);

	return SUCCESS;
}

int32 rtl8651_getAsicVlanRemark(enum PORTID port, enum PRIORITYVALUE priority, int* remark)
{
	int regValue;
	if ((port < PHY0) || (port > CPU) || (priority < PRI0) || (priority > PRI7) ||(remark == NULL))
		return FAILED;

	regValue = READ_MEM32(RMCR1P);
	if(regValue & (0x1<<(24+port))){
		*remark = (regValue>>(3*priority)) & 0x7;
	}else{
			return FAILED;
	}

	return SUCCESS;
}


/*vlan remark*/
int32 rtl8651_setAsicDscpRemark(enum PORTID port, enum PRIORITYVALUE priority, int remark)
{
	int regValue0;
	if ((port < PHY0) || (port > CPU) || (priority < PRI0) || (priority > PRI7) ||(remark < 0) ||(remark > 63))
		return FAILED;

	WRITE_MEM32(RLRC, (READ_MEM32(RLRC) &~(0x7<<3)) |(0x7<<3));

	if(priority < 5){
		regValue0 = READ_MEM32(DSCPRM0) &~(0x3f<<(6*priority));
		regValue0 |= remark << (6*priority);
		WRITE_MEM32( DSCPRM0, regValue0);

		regValue0 = READ_MEM32(DSCPRM1) &~(0x1 << (23 + port));
		regValue0 |= 0x1 << (23 + port);
		WRITE_MEM32( DSCPRM1, regValue0);
	}else{
		regValue0 = READ_MEM32(DSCPRM1) & ~((0x3f<<(6*(priority -5))) |(0x1 << (23 + port)));
		regValue0 |= ((remark<<(6*(priority -5))) |(0x1 << (23 + port)));
		WRITE_MEM32( DSCPRM1, regValue0);
	}

	return SUCCESS;
}

int32 rtl8651_getAsicDscpRemark(enum PORTID port, enum PRIORITYVALUE priority, int* remark)
{
	int regValue0, regValue1;
	if ((port < PHY0) || (port > CPU) || (priority < PRI0) || (priority > PRI7) || (remark == NULL))
		return FAILED;

	regValue1 = READ_MEM32(DSCPRM1);
	if(regValue1 & (0x1 << (23 + port))){
		if(priority < 5){
			regValue0 = READ_MEM32(DSCPRM0);
			*remark = (regValue0>>(6*priority)) & 0x3f;
		}else{
			*remark = (regValue1>>(6*(priority-5))) & 0x3f;
		}
	}else{
			return FAILED;
	}

	return SUCCESS;
}



/*=========================================
  * ASIC DRIVER API: Packet Scheduling Control Register
  *=========================================*/
/*
@func int32 | rtl8651_setAsicPriorityDecision | set priority selection
@parm uint32 | portpri | output queue decision priority assign for Port Based Priority.
@parm uint32 | dot1qpri | output queue decision priority assign for 1Q Based Priority.
@parm uint32 | dscppri | output queue decision priority assign for DSCP Based Priority
@parm uint32 | aclpri | output queue decision priority assign for ACL Based Priority.
@parm uint32 | natpri | output queue decision priority assign for NAT Based Priority.
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_setAsicPriorityDecision( uint32 portpri, uint32 dot1qpri, uint32 dscppri, uint32 aclpri, uint32 natpri )
{
	/* Invalid input parameter */
	if ((portpri < 0) || (portpri > 0xF) || (dot1qpri < 0) || (dot1qpri > 0xF) ||
		(dscppri < 0) || (dscppri > 0xF) || (aclpri < 0) || (aclpri > 0xF) ||
		(natpri < 0) || (natpri > 0xF))
		return FAILED;

	WRITE_MEM32(QIDDPCR, (portpri << PBP_PRI_OFFSET) | (dot1qpri << BP8021Q_PRI_OFFSET) |
		                 (dscppri << DSCP_PRI_OFFSET) | (aclpri << ACL_PRI_OFFSET) |
		                 (natpri << NAPT_PRI_OFFSET));

	return SUCCESS;
}

int32 rtl8651_getAsicPriorityDecision( uint32* portpri, uint32* dot1qpri, uint32* dscppri, uint32* aclpri, uint32* natpri )
{
	uint32 temp;
	if((portpri == NULL) ||(dot1qpri == NULL) ||(dscppri == NULL) ||(aclpri == NULL) ||(natpri == NULL))
		return FAILED;

	temp = READ_MEM32(QIDDPCR);
	*portpri = (temp >> PBP_PRI_OFFSET) & 0xf;
	*dot1qpri = (temp >> BP8021Q_PRI_OFFSET) & 0xf;
	*dscppri = (temp >> DSCP_PRI_OFFSET) & 0xf;
	*aclpri = (temp >> ACL_PRI_OFFSET) & 0xf;
	*natpri  = (temp >> NAPT_PRI_OFFSET) & 0xf;

	return SUCCESS;
}


int32 rtl8651_setAsicQueueFlowControlConfigureRegister(enum PORTID port, enum QUEUEID queue, uint32 enable)
{
	switch (port)
	{
		case PHY0:
			WRITE_MEM32(FCCR0, (READ_MEM32(FCCR0) & ~(0x1<<(queue+Q_P0_EN_FC_OFFSET))) | (enable << (queue+Q_P0_EN_FC_OFFSET)));  break;
		case PHY1:
			WRITE_MEM32(FCCR0, (READ_MEM32(FCCR0) & ~(0x1<<(queue+Q_P1_EN_FC_OFFSET))) | (enable << (queue+Q_P1_EN_FC_OFFSET)));  break;
		case PHY2:
			WRITE_MEM32(FCCR0, (READ_MEM32(FCCR0) & ~(0x1<<(queue+Q_P2_EN_FC_OFFSET))) | (enable << (queue+Q_P2_EN_FC_OFFSET)));  break;
		case PHY3:
			WRITE_MEM32(FCCR0, (READ_MEM32(FCCR0) & ~(0x1<<(queue+Q_P3_EN_FC_OFFSET))) | (enable << (queue+Q_P3_EN_FC_OFFSET)));  break;
		case PHY4:
			WRITE_MEM32(FCCR1, (READ_MEM32(FCCR1) & ~(0x1<<(queue+Q_P4_EN_FC_OFFSET))) | (enable << (queue+Q_P4_EN_FC_OFFSET)));  break;
		case PHY5:
			WRITE_MEM32(FCCR1, (READ_MEM32(FCCR1) & ~(0x1<<(queue+Q_P5_EN_FC_OFFSET))) | (enable << (queue+Q_P5_EN_FC_OFFSET)));  break;
		case CPU:
			WRITE_MEM32(FCCR1, (READ_MEM32(FCCR1) & ~(0x1<<(queue+Q_P6_EN_FC_OFFSET))) | (enable << (queue+Q_P6_EN_FC_OFFSET)));  break;
		default:
			return FAILED;
	}

	return SUCCESS;
}

int32 rtl8651_getAsicQueueFlowControlConfigureRegister(enum PORTID port, enum QUEUEID queue, uint32 *enable)
{
	if (enable != NULL)
	{
		switch (port)
		{
			case PHY0:
				*enable = (READ_MEM32(FCCR0) & (0x1<<(queue+Q_P0_EN_FC_OFFSET))) >> (queue+Q_P0_EN_FC_OFFSET);  break;
			case PHY1:
				*enable = (READ_MEM32(FCCR0) & (0x1<<(queue+Q_P1_EN_FC_OFFSET))) >> (queue+Q_P1_EN_FC_OFFSET);  break;
			case PHY2:
				*enable = (READ_MEM32(FCCR0) & (0x1<<(queue+Q_P2_EN_FC_OFFSET))) >> (queue+Q_P2_EN_FC_OFFSET);  break;
			case PHY3:
				*enable = (READ_MEM32(FCCR0) & (0x1<<(queue+Q_P3_EN_FC_OFFSET))) >> (queue+Q_P3_EN_FC_OFFSET);  break;
			case PHY4:
				*enable = (READ_MEM32(FCCR1) & (0x1<<(queue+Q_P4_EN_FC_OFFSET))) >> (queue+Q_P4_EN_FC_OFFSET);  break;
			case PHY5:
				*enable = (READ_MEM32(FCCR1) & (0x1<<(queue+Q_P5_EN_FC_OFFSET))) >> (queue+Q_P5_EN_FC_OFFSET);  break;
			case CPU:
				*enable = (READ_MEM32(FCCR1) & (0x1<<(queue+Q_P6_EN_FC_OFFSET))) >> (queue+Q_P6_EN_FC_OFFSET);  break;
			default:
				return FAILED;
		}
	}

	return SUCCESS;
}

/*
@func int32 | rtl8651_setAsicLBParameter | set Leaky Bucket Paramters
@parm uint32 | token | Token is used for adding budget in each time slot.
@parm uint32 | tick | Tick is used for time slot size slot.
@parm uint32 | hiThreshold | leaky bucket token high-threshold register
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_setAsicLBParameter( uint32 token, uint32 tick, uint32 hiThreshold )
{
	WRITE_MEM32( ELBPCR, (READ_MEM32(ELBPCR) & ~(Token_MASK | Tick_MASK)) | (token << Token_OFFSET) | (tick << Tick_OFFSET));
	WRITE_MEM32( ELBTTCR, (READ_MEM32(ELBTTCR) & ~0xFFFF/*L2_MASK*/) | (hiThreshold << L2_OFFSET));
	WRITE_MEM32( ILBPCR2, (READ_MEM32(ILBPCR2) & ~(ILB_feedToken_MASK|ILB_Tick_MASK)) | (token << ILB_feedToken_OFFSET) | (tick << ILB_Tick_OFFSET) );
	return SUCCESS;
}


/*
@func int32 | rtl8651_getAsicLBParameter | get Leaky Bucket Paramters
@parm uint32* | pToken | pointer to return token
@parm uint32* | pTick | pointer to return tick
@parm uint32* | pHiThreshold | pointer to return hiThreshold
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_getAsicLBParameter( uint32* pToken, uint32* pTick, uint32* pHiThreshold )
{
	uint32 regValue;

	regValue = READ_MEM32(ELBPCR);

	if (pToken != NULL)
		*pToken = (regValue & Token_MASK) >> Token_OFFSET;
	if (pTick != NULL)
		*pTick = (regValue & Tick_MASK) >> Tick_OFFSET;
	if (pHiThreshold != NULL)
		*pHiThreshold = (READ_MEM32(ELBTTCR) & 0xFF) >> L2_OFFSET;

	return SUCCESS;
}

/*port based priority*/
int32 rtl8651_setAsicPortBasedPriority( enum PORTID port, enum PRIORITYVALUE priority )
{
	/* Invalid input parameter */
	if ((priority < PRI0) || (priority > PRI7) ||(port < PHY0) || (port> EXT3) )
		return FAILED;

	WRITE_MEM32(PBPCR, (READ_MEM32(PBPCR) & ~(0x7 << (port*3))) | (priority << (port*3)));

	return SUCCESS;
}

int32 rtl8651_getAsicPortBasedPriority( enum PORTID port, enum PRIORITYVALUE* priority )
{
	/* Invalid input parameter */
	if ((port < PHY0) || (port> EXT3) ||(priority == NULL))
		return FAILED;

	*priority = (READ_MEM32(PBPCR) >> (3*port)) & 0x7;

	return SUCCESS;
}



/*
@func int32 | rtl8651_setAsicQueueRate | set per queue rate
@parm enum PORTID | port | the port number
@parm enum QUEUEID | queueid | the queue ID wanted to set
@parm uint32 | pprTime | Peak Packet Rate (in times of APR). 0~6: PPR = (2^pprTime)*apr. 7: disable PPR
@parm uint32 | aprBurstSize | Bucket Burst Size of Average Packet Rate (unit: 1KByte). 0xFF: disable
@parm uint32 | apr | Average Packet Rate (unit: 64Kbps). 0x3FFF: unlimited rate
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_setAsicQueueRate( enum PORTID port, enum QUEUEID queueid, uint32 pprTime, uint32 aprBurstSize, uint32 apr )
{
	uint32 reg1, regValue;

	if ((port < PHY0) || (port > CPU) || (queueid < QUEUE0) || (queueid > QUEUE5))
		return FAILED;

	reg1 = P0Q0RGCR + (port * 0x18) + (queueid * 0x4);  /* offset to get corresponding register */

	regValue = READ_MEM32(reg1) & ~(PPR_MASK | L1_MASK | APR_MASK);
	regValue |= ((pprTime << PPR_OFFSET) | (aprBurstSize << L1_OFFSET) | (apr << APR_OFFSET));
	WRITE_MEM32( reg1, regValue);
	return SUCCESS;
}


/*
@func int32 | rtl8651_getAsicQueueRate | get per queue rate configuration
@parm enum PORTID | port | the port number
@parm enum QUEUEID | queueid | the queue ID wanted to set
@parm uint32* | pPprTime | pointer to Peak Packet Rate (in times of APR). 0~6: PPR = (2^pprTime)*apr. 7: disable PPR
@parm uint32* | pAprBurstSize | pointer to APR Burst Size (unit: 1KBytes). 0xff: disable
@parm uint32* | pApr | pointer to Average Packet Rate (unit: 64Kbps). 0x3FFF: unlimited rate
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_getAsicQueueRate( enum PORTID port, enum QUEUEID queueid, uint32* pPprTime, uint32* pAprBurstSize, uint32* pApr )
{
	uint32 reg1, regValue;

	if ((port < PHY0) || (port > CPU) || (queueid < QUEUE0) || (queueid > QUEUE5))
		return FAILED;

	reg1 = P0Q0RGCR + (port * 0x18) + (queueid * 0x4);  /* offset to get corresponding register */
	regValue = READ_MEM32(reg1);

	if (pPprTime != NULL)
		*pPprTime = (regValue & PPR_MASK) >> PPR_OFFSET;
	if (pAprBurstSize != NULL)
		*pAprBurstSize = (regValue & L1_MASK) >> L1_OFFSET;
	if (pApr != NULL)
		*pApr = (regValue & APR_MASK) >> APR_OFFSET;
	return SUCCESS;
}

/*
@func int32 | rtl8651_setAsicPortIngressBandwidth | set per-port total ingress bandwidth
@parm enum PORTID | port | the port number
@parm uint32 | bandwidth | the total ingress bandwidth (unit: 16Kbps), 0:disable
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_setAsicPortIngressBandwidth( enum PORTID port, uint32 bandwidth)
{
	uint32 reg1;

	/* For ingress bandwidth control, its only for PHY0 to PHY5 */
	if ((port < PHY0) || (port > PHY5))
		return FAILED;

	reg1 = IBCR0 + ((port / 2) * 0x04);		/* offset to get corresponding register */

	if ( port % 2)
	{	/* ODD-port */
		WRITE_MEM32( reg1, ((READ_MEM32(reg1) & ~(IBWC_ODDPORT_MASK)) | ((bandwidth << IBWC_ODDPORT_OFFSET) & IBWC_ODDPORT_MASK)));
	} else
	{	/* EVEN-port */
		WRITE_MEM32( reg1, ((READ_MEM32(reg1) & ~(IBWC_EVENPORT_MASK)) | ((bandwidth << IBWC_EVENPORT_OFFSET) & IBWC_EVENPORT_MASK)));
	}


	return SUCCESS;
}

/*
@func int32 | rtl8651_getAsicPortIngressBandwidth | get per-port total ingress bandwidth
@parm enum PORTID | port | the port number
@parm uint32* | pBandwidth | pointer to the returned total ingress bandwidth (unit: 16Kbps), 0:disable
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_getAsicPortIngressBandwidth( enum PORTID port, uint32* pBandwidth )
{
	uint32 reg1, regValue;

	/* For ingress bandwidth control, its only for PHY0 to PHY5 */
	if ((port < PHY0) || (port > PHY5))
		return FAILED;

	reg1 = IBCR0 + ((port / 2) * 0x04);		/* offset to get corresponding register */

	regValue = READ_MEM32(reg1);

	if (pBandwidth != NULL)
	{
		*pBandwidth = (port % 2)?
						/* Odd port */((regValue & IBWC_ODDPORT_MASK) >> IBWC_ODDPORT_OFFSET):
						/* Even port */((regValue & IBWC_EVENPORT_MASK) >> IBWC_EVENPORT_OFFSET);
	}

	return SUCCESS;
}


/*
@func int32 | rtl8651_setAsicPortEgressBandwidth | set per-port total egress bandwidth
@parm enum PORTID | port | the port number
@parm uint32 | bandwidth | the total egress bandwidth (unit: 64kbps). 0x3FFF: disable
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_setAsicPortEgressBandwidth( enum PORTID port, uint32 bandwidth )
{
	uint32 reg1;

#ifdef CONFIG_RTK_VOIP_QOS
	extern int wan_port_check(int port);
#endif
	if ((port < PHY0) || (port > CPU))
		return FAILED;

	reg1 = WFQRCRP0 + (port * 0xC);  /* offset to get corresponding register */
	WRITE_MEM32( reg1, (READ_MEM32(reg1) & ~(APR_MASK)) | (bandwidth << APR_OFFSET));

#ifdef CONFIG_RTK_VOIP_QOS
	if(wan_port_check(port))
	{
		if(bandwidth < 161 )
			rtl8651_cpu_tx_fc(0);//disable flow control
		else
			rtl8651_cpu_tx_fc(1);//enable flow control
	}
#endif
	return SUCCESS;
}


/*
@func int32 | rtl8651_getAsicPortEgressBandwidth | get per-port total egress bandwidth
@parm enum PORTID | port | the port number
@parm uint32* | pBandwidth | pointer to the returned total egress bandwidth (unit: 64kbps). 0x3FFF: disable
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_getAsicPortEgressBandwidth( enum PORTID port, uint32* pBandwidth )
{
	uint32 reg1, regValue;

	if ((port < PHY0) || (port > CPU))
		return FAILED;

	reg1 = WFQRCRP0 + (port * 0xC);  /* offset to get corresponding register */
	regValue = READ_MEM32(reg1);

	if (pBandwidth != NULL)
		*pBandwidth = (regValue & APR_MASK) >> APR_OFFSET;

	return SUCCESS;
}

/*set queue type as STRICT*/
int32 rtl8651_setAsicQueueStrict( enum PORTID port, enum QUEUEID queueid, enum QUEUETYPE queueType)
{
	uint32 reg1, regOFFSET, regValue;

	if ((port < PHY0) || (port > CPU) || (queueid < QUEUE0) || (queueid > QUEUE5))
		return FAILED;
	if ((queueType < STR_PRIO) || (queueType > WFQ_PRIO))
		return FAILED;

	reg1 = WFQWCR0P0 + (port * 0xC) + ((queueid >> 2) * 0x4);	/* offset to get corresponding register */
	regOFFSET = (queueid % 4) * 0x8;	/* used to offset register value */

	regValue = READ_MEM32(reg1) & ~((WEIGHT0_MASK | SCHE0_MASK) << regOFFSET);
	regValue |= (queueType << (SCHE0_OFFSET + regOFFSET));
	WRITE_MEM32( reg1, regValue);
	return SUCCESS;
}

int32 rtl8651_getAsicQueueStrict( enum PORTID port, enum QUEUEID queueid, enum QUEUETYPE *pQueueType)
{
	uint32 reg1, regOFFSET, regValue;

	if ((port < PHY0) || (port > CPU) || (queueid < QUEUE0) || (queueid > QUEUE5) ||(pQueueType == NULL))
		return FAILED;

	reg1 = WFQWCR0P0 + (port * 0xC) + ((queueid >> 2) * 0x4);  /* offset to get corresponding register */
	regOFFSET = (queueid % 4) * 0x8;	/* used to offset register value */
	regValue = READ_MEM32(reg1);

	if (pQueueType != NULL)
		*pQueueType = ((regValue & (SCHE0_MASK << regOFFSET)) >> SCHE0_OFFSET) >> regOFFSET;

	return SUCCESS;
}


/*
@func int32 | rtl8651_setAsicQueueWeight | set WFQ weighting
@parm enum PORTID | port | the port number
@parm enum QUEUEID | queueid | the queue ID wanted to set
@parm enum QUEUETYPE | queueType | the specified queue type
@parm uint32 | weight | the weight value wanted to set (valid:0~127)
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_setAsicQueueWeight( enum PORTID port, enum QUEUEID queueid, enum QUEUETYPE queueType, uint32 weight )
{
	uint32 reg1, regOFFSET, regValue;

	if ((port < PHY0) || (port > CPU) || (queueid < QUEUE0) || (queueid > QUEUE5))
		return FAILED;
	if ((queueType < STR_PRIO) || (queueType > WFQ_PRIO))
		return FAILED;
	if((weight < 0) && (weight > 127))
		return FAILED;

	reg1 = WFQWCR0P0 + (port * 0xC) + ((queueid >> 2) * 0x4);	/* offset to get corresponding register */
	regOFFSET = (queueid % 4) * 0x8;	/* used to offset register value */

	regValue = READ_MEM32(reg1) & ~((WEIGHT0_MASK | SCHE0_MASK) << regOFFSET);
	regValue |= ((queueType << (SCHE0_OFFSET + regOFFSET)) | (weight << (WEIGHT0_OFFSET + regOFFSET)));
	WRITE_MEM32( reg1, regValue);
	return SUCCESS;
}


/*
@func int32 | rtl8651_getAsicQueueWeight | get WFQ weighting
@parm enum PORTID | port | the port number
@parm enum QUEUEID | queueid | the queue ID wanted to set
@parm enum QUEUETYPE* | pQueueType | pointer to the returned queue type
@parm uint32* | pWeight | pointer to the returned weight value
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_getAsicQueueWeight( enum PORTID port, enum QUEUEID queueid, enum QUEUETYPE *pQueueType, uint32 *pWeight )
{
	uint32 reg1, regOFFSET, regValue;

	if ((port < PHY0) || (port > CPU) || (queueid < QUEUE0) || (queueid > QUEUE5))
		return FAILED;

	reg1 = WFQWCR0P0 + (port * 0xC) + ((queueid >> 2) * 0x4);  /* offset to get corresponding register */
	regOFFSET = (queueid % 4) * 0x8;	/* used to offset register value */
	regValue = READ_MEM32(reg1);

	if (pQueueType != NULL)
		*pQueueType = ((regValue & (SCHE0_MASK << regOFFSET)) >> SCHE0_OFFSET) >> regOFFSET;
	if (pWeight != NULL)
		*pWeight = ((regValue & (WEIGHT0_MASK << regOFFSET)) >> WEIGHT0_OFFSET) >> regOFFSET;

	return SUCCESS;
}

/*
@func int32 | rtl8651_setAsicOutputQueueNumber | set output queue number for a specified port
@parm enum PORTID | port | the port number (valid: physical ports(0~5) and CPU port(6) )
@parm enum QUEUENUM | qnum | the output queue number
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_setAsicOutputQueueNumber( enum PORTID port, enum QUEUENUM qnum )
{
	/* Invalid input parameter */

	enum QUEUENUM orgQnum;

	if ((port < PHY0) || (port > CPU) || (qnum < QNUM1) || (qnum > QNUM6))
		return FAILED;

	orgQnum = (READ_MEM32(QNUMCR) >> (3*port)) & 0x7;
	WRITE_MEM32(QNUMCR, (READ_MEM32(QNUMCR) & ~(0x7 << (3*port))) | (qnum << (3*port)));

#if !defined(CONFIG_RTL_819XD	) && !defined(CONFIG_RTL_8196E)
	if (qnum==6)
	{
		if (orgQnum!=6)
			_rtl865x_setQosThresholdByQueueIdx(QNUM_IDX_6);
	}
	else if (qnum>3)
	{
		if((orgQnum==6)||orgQnum<4)
			_rtl865x_setQosThresholdByQueueIdx(QNUM_IDX_45);
	}
	else
	{
		if(orgQnum>3)
			_rtl865x_setQosThresholdByQueueIdx(QNUM_IDX_123);
	}
#endif

	return SUCCESS;
}


/*
@func int32 | rtl8651_getAsicOutputQueueNumber | get output queue number for a specified port
@parm enum PORTID | port | the port number (valid: physical ports(0~5) and CPU port(6) )
@parm enum QUEUENUM | qnum | the output queue number
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_getAsicOutputQueueNumber( enum PORTID port, enum QUEUENUM *qnum )
{
	/* Invalid input parameter */
	if ((port < PHY0) || (port > CPU))
		return FAILED;

	if (qnum != NULL)
		*qnum = (READ_MEM32(QNUMCR) >> (3*port)) & 0x7;

	return SUCCESS;
}

/*
@func int32 | rtl8651_setAsicPriorityToQIDMappingTable | set user priority to QID mapping table parameter
@parm enum QUEUENUM | qnum | the output queue number
@parm enum PRIORITYVALUE | priority | priority
@parm enum QUEUEID | qid | queue ID
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_setAsicPriorityToQIDMappingTable( enum QUEUENUM qnum, enum PRIORITYVALUE priority, enum QUEUEID qid )
{
	/* Invalid input parameter */
	if ((priority < PRI0) || (priority > PRI7))
		return FAILED;
	if ((qid < QUEUE0) || (qid > QUEUE5))
		return FAILED;

	switch (qnum)
	{
		case QNUM1:
			WRITE_MEM32(UPTCMCR0, (READ_MEM32(UPTCMCR0) & ~(0x7 << (priority*3))) | (qid << (priority*3))); break;
		case QNUM2:
			WRITE_MEM32(UPTCMCR1, (READ_MEM32(UPTCMCR1) & ~(0x7 << (priority*3))) | (qid << (priority*3))); break;
		case QNUM3:
			WRITE_MEM32(UPTCMCR2, (READ_MEM32(UPTCMCR2) & ~(0x7 << (priority*3))) | (qid << (priority*3))); break;
		case QNUM4:
			WRITE_MEM32(UPTCMCR3, (READ_MEM32(UPTCMCR3) & ~(0x7 << (priority*3))) | (qid << (priority*3))); break;
		case QNUM5:
			WRITE_MEM32(UPTCMCR4, (READ_MEM32(UPTCMCR4) & ~(0x7 << (priority*3))) | (qid << (priority*3))); break;
		case QNUM6:
			WRITE_MEM32(UPTCMCR5, (READ_MEM32(UPTCMCR5) & ~(0x7 << (priority*3))) | (qid << (priority*3))); break;
		default:
			return FAILED;
	}

	return SUCCESS;
}

int32 rtl8651_getAsicPriorityToQIDMappingTable( enum QUEUENUM qnum, enum PRIORITYVALUE priority, enum QUEUEID* qid )
{
	/* Invalid input parameter */
	if ((priority < PRI0) || (priority > PRI7) ||(qid == NULL))
		return FAILED;

	switch (qnum)
	{
		case QNUM1:
			*qid = (READ_MEM32(UPTCMCR0) >> (priority*3)) & 0x7;
			break;
		case QNUM2:
			*qid = (READ_MEM32(UPTCMCR1) >> (priority*3)) & 0x7;
			break;
		case QNUM3:
			*qid = (READ_MEM32(UPTCMCR2) >> (priority*3)) & 0x7;
			break;
		case QNUM4:
			*qid = (READ_MEM32(UPTCMCR3) >> (priority*3)) & 0x7;
			break;
		case QNUM5:
			*qid = (READ_MEM32(UPTCMCR4) >> (priority*3)) & 0x7;
			break;
		case QNUM6:
			*qid = (READ_MEM32(UPTCMCR5) >> (priority*3)) & 0x7;
			break;
		default:
			return FAILED;
	}

	return SUCCESS;
}


/*
@func int32 | rtl8651_setAsicCPUPriorityToQIDMappingTable | set user priority to QID mapping table parameter based on destination port & priority information
@parm enum PORTID | port | the destination port
@parm enum PRIORITYVALUE | priority | priority
@parm enum QUEUEID | qid | queue ID
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_setAsicCPUPriorityToQIDMappingTable( enum PORTID port, enum PRIORITYVALUE priority, enum QUEUEID qid )
{
	uint32	reg;
	/* Invalid input parameter */
	if ((priority < PRI0) || (priority > PRI7))
		return FAILED;
	if ((qid < QUEUE0) || (qid > QUEUE5))
		return FAILED;
	if (port<CPU || port>MULTEXT)
		return FAILED;

	reg = (uint32)(((uint32*)CPUQIDMCR0) + (port-CPU));

	WRITE_MEM32(reg, (READ_MEM32(reg) & ~(0x7 << (priority<<2))) | (qid << (priority<<2)));

	return SUCCESS;
}


int32 rtl8651_setAsicSystemBasedFlowControlRegister(uint32 sharedON, uint32 sharedOFF, uint32 fcON, uint32 fcOFF, uint32 drop)
{
	/* Invalid input parameter */
	if ((sharedON > (SDC_FCON_MASK >> SDC_FCON_OFFSET)) ||
		(sharedOFF > (S_DSC_FCOFF_MASK >> S_DSC_FCOFF_OFFSET)) ||
		(fcON > ((S_Max_SBuf_FCON_MASK >> S_Max_SBuf_FCON_OFFSET))) ||
		(fcOFF > (S_Max_SBuf_FCOFF_MASK >> S_Max_SBuf_FCOFF_OFFSET)) ||
		(drop > (S_DSC_RUNOUT_MASK >> S_DSC_RUNOUT_OFFSET)))
		return FAILED;

	WRITE_MEM32(SBFCR0, (READ_MEM32(SBFCR0) & ~(S_DSC_RUNOUT_MASK)) | (drop << S_DSC_RUNOUT_OFFSET));
	WRITE_MEM32(SBFCR1, (READ_MEM32(SBFCR1) & ~(S_DSC_FCON_MASK | S_DSC_FCOFF_MASK)) | ( fcON<< S_DSC_FCON_OFFSET) | (fcOFF << S_DSC_FCOFF_OFFSET));
	WRITE_MEM32(SBFCR2, (READ_MEM32(SBFCR2) & ~(S_Max_SBuf_FCON_MASK | S_Max_SBuf_FCOFF_MASK)) | (sharedON << S_Max_SBuf_FCON_OFFSET) | (sharedOFF << S_Max_SBuf_FCOFF_OFFSET));
	return SUCCESS;
}

int32 rtl8651_setAsicQueueDescriptorBasedFlowControlRegister(enum PORTID port, enum QUEUEID queue, uint32 fcON, uint32 fcOFF)
{
	/* Invalid input parameter */
	if ((port < PHY0) || (port > CPU))
		return FAILED;

	if ((fcON > (QG_DSC_FCON_MASK >> QG_DSC_FCON_OFFSET)) ||
		(fcOFF > (QG_DSC_FCOFF_MASK >> QG_DSC_FCOFF_OFFSET)))
		return FAILED;


	switch (queue)
	{
		case QUEUE0:
			WRITE_MEM32((QDBFCRP0G0+(port*0xC)), (READ_MEM32(QDBFCRP0G0+(port*0xC)) & ~(QG_DSC_FCON_MASK | QG_DSC_FCOFF_MASK)) | (fcON << QG_DSC_FCON_OFFSET) | (fcOFF << QG_DSC_FCOFF_OFFSET));
			break;
		case QUEUE1:
		case QUEUE2:
		case QUEUE3:
		case QUEUE4:
			WRITE_MEM32((QDBFCRP0G1+(port*0xC)), (READ_MEM32(QDBFCRP0G1+(port*0xC)) & ~(QG_DSC_FCON_MASK | QG_DSC_FCOFF_MASK)) | (fcON << QG_DSC_FCON_OFFSET) | (fcOFF << QG_DSC_FCOFF_OFFSET));
			break;
		case QUEUE5:
			WRITE_MEM32((QDBFCRP0G2+(port*0xC)), (READ_MEM32(QDBFCRP0G2+(port*0xC)) & ~(QG_DSC_FCON_MASK | QG_DSC_FCOFF_MASK)) | (fcON << QG_DSC_FCON_OFFSET) | (fcOFF << QG_DSC_FCOFF_OFFSET));
			break;
		default:
			return FAILED;
	}
	return SUCCESS;
}

int32 rtl8651_setAsicQueuePacketBasedFlowControlRegister(enum PORTID port, enum QUEUEID queue, uint32 fcON, uint32 fcOFF)
{
	/* Invalid input parameter */
	if ((port < PHY0) || (port > CPU))
		return FAILED;

	if ((fcON > (QG_QLEN_FCON_MASK>> QG_QLEN_FCON_OFFSET)) ||
		(fcOFF > (QG_QLEN_FCOFF_MASK >> QG_QLEN_FCOFF_OFFSET)))
		return FAILED;

	switch (queue)
	{
		case QUEUE0:
			WRITE_MEM32((QPKTFCRP0G0+(port*0xC)), (READ_MEM32(QPKTFCRP0G0+(port*0xC)) & ~(QG_QLEN_FCON_MASK | QG_QLEN_FCOFF_MASK)) | (fcON << QG_QLEN_FCON_OFFSET) | (fcOFF << QG_QLEN_FCOFF_OFFSET));
			break;
		case QUEUE1:
		case QUEUE2:
		case QUEUE3:
		case QUEUE4:
			WRITE_MEM32((QPKTFCRP0G1+(port*0xC)), (READ_MEM32(QPKTFCRP0G1+(port*0xC)) & ~(QG_QLEN_FCON_MASK | QG_QLEN_FCOFF_MASK)) | (fcON << QG_QLEN_FCON_OFFSET) | (fcOFF << QG_QLEN_FCOFF_OFFSET));
			break;
		case QUEUE5:
			WRITE_MEM32((QPKTFCRP0G2+(port*0xC)), (READ_MEM32(QPKTFCRP0G2+(port*0xC)) & ~(QG_QLEN_FCON_MASK | QG_QLEN_FCOFF_MASK)) | (fcON << QG_QLEN_FCON_OFFSET) | (fcOFF << QG_QLEN_FCOFF_OFFSET));
			break;
		default:
			return FAILED;
	}

	return SUCCESS;
}

int32 rtl8651_setAsicPortBasedFlowControlRegister(enum PORTID port, uint32 fcON, uint32 fcOFF)
{
	/* Invalid input parameter */
	if ((fcON > (P_MaxDSC_FCON_MASK >> P_MaxDSC_FCON_OFFSET)) ||
		(fcOFF > (P_MaxDSC_FCOFF_MASK >> P_MaxDSC_FCOFF_OFFSET)))
		return FAILED;

	switch (port)
	{
		case PHY0:
			WRITE_MEM32(PBFCR0, (READ_MEM32(PBFCR0) & ~(P_MaxDSC_FCON_MASK | P_MaxDSC_FCOFF_MASK)) | (fcON << P_MaxDSC_FCON_OFFSET) | (fcOFF << P_MaxDSC_FCOFF_OFFSET)); break;
		case PHY1:
			WRITE_MEM32(PBFCR1, (READ_MEM32(PBFCR1) & ~(P_MaxDSC_FCON_MASK | P_MaxDSC_FCOFF_MASK)) | (fcON << P_MaxDSC_FCON_OFFSET) | (fcOFF << P_MaxDSC_FCOFF_OFFSET)); break;
		case PHY2:
			WRITE_MEM32(PBFCR2, (READ_MEM32(PBFCR2) & ~(P_MaxDSC_FCON_MASK | P_MaxDSC_FCOFF_MASK)) | (fcON << P_MaxDSC_FCON_OFFSET) | (fcOFF << P_MaxDSC_FCOFF_OFFSET)); break;
		case PHY3:
			WRITE_MEM32(PBFCR3, (READ_MEM32(PBFCR3) & ~(P_MaxDSC_FCON_MASK | P_MaxDSC_FCOFF_MASK)) | (fcON << P_MaxDSC_FCON_OFFSET) | (fcOFF << P_MaxDSC_FCOFF_OFFSET)); break;
		case PHY4:
			WRITE_MEM32(PBFCR4, (READ_MEM32(PBFCR4) & ~(P_MaxDSC_FCON_MASK | P_MaxDSC_FCOFF_MASK)) | (fcON << P_MaxDSC_FCON_OFFSET) | (fcOFF << P_MaxDSC_FCOFF_OFFSET)); break;
		case PHY5:
			WRITE_MEM32(PBFCR5, (READ_MEM32(PBFCR5) & ~(P_MaxDSC_FCON_MASK | P_MaxDSC_FCOFF_MASK)) | (fcON << P_MaxDSC_FCON_OFFSET) | (fcOFF << P_MaxDSC_FCOFF_OFFSET)); break;
		case CPU:
			WRITE_MEM32(PBFCR6, (READ_MEM32(PBFCR6) & ~(P_MaxDSC_FCON_MASK | P_MaxDSC_FCOFF_MASK)) | (fcON << P_MaxDSC_FCON_OFFSET) | (fcOFF << P_MaxDSC_FCOFF_OFFSET)); break;
		default:
			return FAILED;
	}

	return SUCCESS;
}

int32 rtl8651_setAsicPerQueuePhysicalLengthGapRegister(uint32 gap)
{
	/* Invalid input parameter */
	if (gap > (QLEN_GAP_MASK >> QLEN_GAP_OFFSET))
		return FAILED;

	WRITE_MEM32(PQPLGR, (READ_MEM32(PQPLGR) & ~(QLEN_GAP_MASK)) | (gap << QLEN_GAP_OFFSET));
	return SUCCESS;
}

/* 	note: the dynamic mechanism: adjust the flow control threshold value according to the number of Ethernet link up ports.
	buffer threshold setting:
	sys on = 208, share on = 192 for link port <=3
	0xbb804504 = 0x00c000d0
	0xbb804508 = 0x00b000c0

	sys on = 172, share on = 98 , for link port > 3
	0xbb804504 = 0x00A000AC
	0xbb804508 = 0x004A0062

	1. default threshold setting is link port <=3
	2. got link change interrupt and link port > 3, then change threhosld for link port > 3
	3. got link change interrupt and link port <= 3, then change threhosld for link port <= 3
 */

#ifdef CONFIG_RTL_8197D_DYN_THR
int32 rtl819x_setQosThreshold(uint32 old_sts, uint32 new_sts)
{
	int32 i, link_up_ports=0;
	uint32 j = new_sts;

	if (old_sts == new_sts)
		return SUCCESS;

	for (i=0; i<5; i++) {
		if ((j & 0x1) == 1)
			link_up_ports++;
		j = j >> 1;
	}

	if (link_up_ports <= DYN_THR_LINK_UP_PORTS) {
		WRITE_MEM32(SBFCR1, (READ_MEM32(SBFCR1) & ~(S_DSC_FCON_MASK | S_DSC_FCOFF_MASK)) | ( DYN_THR_AGG_fcON<< S_DSC_FCON_OFFSET) | (DYN_THR_AGG_fcOFF << S_DSC_FCOFF_OFFSET));
		WRITE_MEM32(SBFCR2, (READ_MEM32(SBFCR2) & ~(S_Max_SBuf_FCON_MASK | S_Max_SBuf_FCOFF_MASK)) | (DYN_THR_AGG_sharedON << S_Max_SBuf_FCON_OFFSET) | (DYN_THR_AGG_sharedOFF << S_Max_SBuf_FCOFF_OFFSET));
	}
	else {
		WRITE_MEM32(SBFCR1, (READ_MEM32(SBFCR1) & ~(S_DSC_FCON_MASK | S_DSC_FCOFF_MASK)) | ( DYN_THR_DEF_fcON<< S_DSC_FCON_OFFSET) | (DYN_THR_DEF_fcOFF << S_DSC_FCOFF_OFFSET));
		WRITE_MEM32(SBFCR2, (READ_MEM32(SBFCR2) & ~(S_Max_SBuf_FCON_MASK | S_Max_SBuf_FCOFF_MASK)) | (DYN_THR_DEF_sharedON << S_Max_SBuf_FCON_OFFSET) | (DYN_THR_DEF_sharedOFF << S_Max_SBuf_FCOFF_OFFSET));
	}

	return SUCCESS;
}
#endif

#if !defined(CONFIG_RTL_819XD	) && !defined(CONFIG_RTL_8196E)
static int32 _rtl865x_setQosThresholdByQueueIdx(uint32 qidx)
{
	/* Set the threshold value for qos sytem */
	int32 retval;
	int32	i,j;

	printk("Set threshould idx %d\n", qidx);
	retval = rtl8651_setAsicSystemBasedFlowControlRegister(outputQueuePara[qidx].systemSBFCON, outputQueuePara[qidx].systemSBFCOFF, outputQueuePara[qidx].systemFCON, outputQueuePara[qidx].systemFCOFF, outputQueuePara[qidx].drop);
	if (retval!= SUCCESS)
	{
		rtlglue_printf("Set System Base Flow Control Para Error.\n");
		return retval;
	}

	for(i =0; i < RTL8651_OUTPUTQUEUE_SIZE; i++)
	{
		retval = rtl8651_setAsicQueueDescriptorBasedFlowControlRegister(0, i, outputQueuePara[qidx].queueDescFCON, outputQueuePara[qidx].queueDescFCOFF);
		if (retval!= SUCCESS)
		{
			rtlglue_printf("Set Queue Descriptor Base Flow Control Para Error.\n");
			return retval;
		}
		for(j=1;j<=CPU;j++)
			rtl8651_setAsicQueueDescriptorBasedFlowControlRegister(PHY0+j, i, outputQueuePara[qidx].queueDescFCON, outputQueuePara[qidx].queueDescFCOFF);


		retval = rtl8651_setAsicQueuePacketBasedFlowControlRegister(0, i, outputQueuePara[qidx].queuePktFCON, outputQueuePara[qidx].queuePktFCOFF);
		if (retval!= SUCCESS)
		{
			rtlglue_printf("Set Queue Packet Base Flow Control Para Error.\n");
			return retval;
		}
		for(j=1;j<=CPU;j++)
			rtl8651_setAsicQueuePacketBasedFlowControlRegister(PHY0+j, i, outputQueuePara[qidx].queuePktFCON, outputQueuePara[qidx].queuePktFCOFF);

	}

	retval = rtl8651_setAsicPortBasedFlowControlRegister(0, outputQueuePara[qidx].portFCON, outputQueuePara[qidx].portFCOFF);
	if (retval!= SUCCESS)
	{
		rtlglue_printf("Set Port Base Flow Control Para Error.\n");
		return retval;
	}
	for(j=1;j<=CPU;j++)
		rtl8651_setAsicPortBasedFlowControlRegister(PHY0+j, outputQueuePara[qidx].portFCON, outputQueuePara[qidx].portFCOFF);

	retval = rtl8651_setAsicPerQueuePhysicalLengthGapRegister(outputQueuePara[qidx].gap);
	if (retval!= SUCCESS)
	{
		rtlglue_printf("Set Queue Physical Lenght Gap Reg Error.\n");
		return retval;
	}

	return SUCCESS;
}
#endif

#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
static int32 _rtl865xC_QM_init( void )
{
#if defined(RTL865X_TEST) || defined(RTL865X_MODEL_USER) || defined(RTL865X_MODEL_KERNEL)
#else
	uint32 originalDescGetReady;
	int32 cnt;

	rtlglue_printf("Start to initiate QM\n");

	/*
	   1. Get the original decriptor usage for QM.
	 */
	WRITE_MEM32( SIRR, READ_MEM32(SIRR)| TRXRDY );
 	rtl865xC_lockSWCore();

	_rtl865xC_QM_orgDescUsage = 0;	/* by default, set it to 0 */

	do
	{
		int32 idx;

		originalDescGetReady = TRUE;	/* by default, set it to TRUE */
		cnt = 0;

		for ( idx = 0 ; idx < RTL865XC_QM_DESC_READROBUSTPARAMETER ; idx ++ )
		{
			uint32 currentDescUsage;

			currentDescUsage = (READ_MEM32( GDSR0 ) & USEDDSC_MASK) >> USEDDSC_OFFSET;
			if (	( currentDescUsage == 0 /* It's impossible */ ) ||
				(( _rtl865xC_QM_orgDescUsage != 0 ) &&
				 ( currentDescUsage != _rtl865xC_QM_orgDescUsage) ) )
			{
				rtlglue_printf("INIT swCore descriptor count Failed : (%d [%d])\n", currentDescUsage, _rtl865xC_QM_orgDescUsage);
				originalDescGetReady = FALSE;
			} else
			{
				_rtl865xC_QM_orgDescUsage = currentDescUsage;
			}
		}
		cnt++;
	} while ( originalDescGetReady != TRUE && cnt< 200000);

	if (_rtl865xC_QM_orgDescUsage<12)
		_rtl865xC_QM_orgDescUsage = 12;

	rtl865xC_unLockSWCore();
#endif
	return SUCCESS;
}
#endif

/*
@func int32 | rtl865xC_waitForOutputQueueEmpty | wait until output queue empty
@rvalue SUCCESS |
@comm
	The function will not return until all the output queue is empty.
 */
int32 rtl865xC_waitForOutputQueueEmpty(void)
{
#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
	int32	currentDescUsage;
	int32	cnt, i;

	#if defined(CONFIG_RTL_8196C)
	while((READ_MEM32(LAGCR1)&OUTPUTQUEUE_STAT_MASK_CR1)!=OUTPUTQUEUE_STAT_MASK_CR1);
	#else
	while ( ((READ_MEM32(LAGCR0)&OUTPUTQUEUE_STAT_MASK_CR0)^OUTPUTQUEUE_STAT_MASK_CR0) ||
       	((READ_MEM32(LAGCR1)&OUTPUTQUEUE_STAT_MASK_CR1)^OUTPUTQUEUE_STAT_MASK_CR1) );
	#endif

	/*	There are something wrong when check the input queue is empty or not	*/
	currentDescUsage = (READ_MEM32( GDSR0 ) & USEDDSC_MASK) >> USEDDSC_OFFSET;
	cnt = (currentDescUsage-_rtl865xC_QM_orgDescUsage)<<10;
	i = 0;
	while ( ((currentDescUsage = ((READ_MEM32( GDSR0 ) & USEDDSC_MASK) >> USEDDSC_OFFSET)) > _rtl865xC_QM_orgDescUsage) && (i<cnt) )
	{
#if 0
		if (net_ratelimit() && i<120)
		rtlglue_printf("Waiting for input queue empty.  ==> currently used %d.\n", (currentDescUsage-_rtl865xC_QM_orgDescUsage));
#else
		;	/* do nothing */
#endif
		i++;
	}
#endif
       return SUCCESS;
}

/*
@func int32 | rtl8651_resetAsicOutputQueue | reset output queue
@rvalue SUCCESS |
@comm
	When reset is done, all queue pointer will be reset to the initial base address.
 */

 int32 rtl8651_resetAsicOutputQueue(void)
 {
	uint32	i;
	uint32	scr, pauseTicks;

 	WRITE_MEM32(QRR, 0x0);
	scr = (REG32(SCCR) & 0x00000070) >> 4;
	switch( scr )
	{
		case 0: pauseTicks = 12500000; break;
		case 1: pauseTicks = 25000000; break;
		case 2: pauseTicks = 31250000; break;
		case 3: pauseTicks = 32500000; break;
		case 4: pauseTicks = 33750000; break;
		case 5: pauseTicks = 35000000; break;
		case 6: pauseTicks = 36250000; break;
		case 7: pauseTicks = 37500000; break;
		default:pauseTicks = 25000000; break;
	}
	/* waiting 500ms */

	pauseTicks = pauseTicks<<2;

	for(i=pauseTicks;i<0;i--)
	{
		i = i;
	}

 	WRITE_MEM32(QRR, 0x1);

	for(i=pauseTicks;i<0;i--)
	{
		i = i;
	}

	WRITE_MEM32(QRR, 0x0);

 	return SUCCESS;
 }


/*
 *	_rtl8651_syncToAsicEthernetBandwidthControl()
 *
 *	Sync SW bandwidth control () configuration to ASIC:
 *
 *
 *		_rtl865xB_BandwidthCtrlPerPortConfiguration -----> Translate from RTL865xB Index to ACTUAL
 *														 	 token count in RTL865xC
 *																		|
 *																---------
 *																|
 *		_rtl865xB_BandwidthCtrlMultiplier	---- Translate using         ---->*
 *										 RTL865xB's mechanism		|
 *																|
 *											---------------------
 *											|
 *											-- > Actual Token count which need to set to ASIC.
 *												 => Set it to ASIC if value in SW is different from ASIC.
 *
*/
static void _rtl8651_syncToAsicEthernetBandwidthControl(void)
{
	uint32 port;
	uint32 cfgTypeIdx;
	int32 retval;

	for ( port = 0 ; port < RTL8651_PORT_NUMBER ; port ++ )
	{
		for ( cfgTypeIdx = 0 ; cfgTypeIdx < _RTL865XB_BANDWIDTHCTRL_CFGTYPE ; cfgTypeIdx ++ )
		{
			uint32 currentSwBandwidthCtrlBasicSetting;
			uint32 currentSwBandwidthCtrlMultiplier;
			uint32 currentSwBandwidthCtrlSetting;
			uint32 currentAsicBandwidthCtrlSetting;

			/*
				We would check for rate and _rtl865xB_BandwidthCtrlMultiplier for the rate-multiply.

				In RTL865xB, the bits definition is as below.

				SWTECR

				bit 14(x8)		bit 15 (x4)		Result
				=============================================
				0				0				x1
				0				1				x4
				1				0				x8
				1				1				x8
			*/
			if (_rtl865xB_BandwidthCtrlMultiplier & _RTL865XB_BANDWIDTHCTRL_X8)
			{	/* case {b'10, b'11} */
				currentSwBandwidthCtrlMultiplier = 8;
			} else if ( _rtl865xB_BandwidthCtrlMultiplier & _RTL865XB_BANDWIDTHCTRL_X4)
			{	/* case {b'01} */
				currentSwBandwidthCtrlMultiplier = 4;
			} else
			{	/* case {b'00} */
				currentSwBandwidthCtrlMultiplier = 1;
			}

			/* Calculate Current SW configuration : 0 : Full Rate */
			/* Mix BASIC setting and Multiplier -> to get the ACTUAL bandwidth setting */

			currentSwBandwidthCtrlBasicSetting = ((_rtl865xC_BandwidthCtrlNum[_rtl865xB_BandwidthCtrlPerPortConfiguration[port][cfgTypeIdx]])*(currentSwBandwidthCtrlMultiplier));
			currentSwBandwidthCtrlSetting = (cfgTypeIdx == 0)?
					/* Ingress */
					(((currentSwBandwidthCtrlBasicSetting%RTL865XC_INGRESS_16KUNIT)<(RTL865XC_INGRESS_16KUNIT>>1))?(currentSwBandwidthCtrlBasicSetting/RTL865XC_INGRESS_16KUNIT):((currentSwBandwidthCtrlBasicSetting/RTL865XC_INGRESS_16KUNIT)+1)):
					/* Egress */
					(((currentSwBandwidthCtrlBasicSetting%RTL865XC_EGRESS_64KUNIT)<(RTL865XC_EGRESS_64KUNIT>>1))?(currentSwBandwidthCtrlBasicSetting/RTL865XC_EGRESS_64KUNIT):((currentSwBandwidthCtrlBasicSetting/RTL865XC_EGRESS_64KUNIT)+1));

			/* Get Current ASIC configuration */
			retval = (cfgTypeIdx == 0)?
					/* Ingress */
					(rtl8651_getAsicPortIngressBandwidth(	port,
															&currentAsicBandwidthCtrlSetting)):
					/* Egress */
					(rtl8651_getAsicPortEgressBandwidth(		port,
															&currentAsicBandwidthCtrlSetting));

			if ( retval != SUCCESS )
			{
				assert(0);
				goto out;
			}

			/* SYNC configuration to HW if the configuration is different */
			if (	(!( (currentSwBandwidthCtrlSetting) == 0 && (currentAsicBandwidthCtrlSetting == 0x3fff) ) /* for FULL Rate case */) ||
				( currentSwBandwidthCtrlSetting != currentAsicBandwidthCtrlSetting ))
			{
#if 0
				if (cfgTypeIdx==0)
				{
					rtlglue_printf("set ingress bandwidth port %d, %d.\n", port, (currentSwBandwidthCtrlSetting == 0)?
																(0	/* For Ingress Bandwidth control, 0 means "disabled" */):
																(currentSwBandwidthCtrlSetting));
				}
				else
				{
					rtlglue_printf("set ingress bandwidth port %d, %d.\n", port, (currentSwBandwidthCtrlSetting == 0)?
																(0x3fff	/* For Egress Bandwidth control, 0x3fff means "disabled" */):
																(currentSwBandwidthCtrlSetting));
				}
#endif
				retval = (cfgTypeIdx == 0)?
					/* Ingress */
					(rtl8651_setAsicPortIngressBandwidth(	port,
															(currentSwBandwidthCtrlSetting == 0)?
																(0	/* For Ingress Bandwidth control, 0 means "disabled" */):
																(currentSwBandwidthCtrlSetting))		):
					/* Egress */
					(rtl8651_setAsicPortEgressBandwidth(		port,
															(currentSwBandwidthCtrlSetting == 0)?
																(0x3fff	/* For Egress Bandwidth control, 0x3fff means "disabled" */):
																(currentSwBandwidthCtrlSetting))		);

				if ( retval != SUCCESS )
				{
					assert(0);
					goto out;
				}
			}

		}
	}
out:
	return;
}


/*
@func int32 | rtl8651_setAsicEthernetBandwidthControl | set ASIC per-port total ingress bandwidth
@parm uint32 | port | the port number
@parm int8 | input | Ingress or egress control to <p port>
@parm uint32 | rate | rate to set.
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
The <p rate> can be set to several different values:
BW_FULL_RATE
BW_128K
BW_256K
BW_512K
BW_1M
BW_2M
BW_4M
BW_8M

Note: This function is backward compatible to RTL865xB.
 */
int32 rtl8651_setAsicEthernetBandwidthControl(uint32 port, int8 input, uint32 rate)
{
	uint32 *currentConfig_p;

	if ( port >= RTL8651_PORT_NUMBER )
	{
		goto err;
	}

	switch ( rate )
	{
		case BW_FULL_RATE:
		case BW_128K:
		case BW_256K:
		case BW_512K:
		case BW_1M:
		case BW_2M:
		case BW_4M:
		case BW_8M:
			break;
		default:
			goto err;
	}

	currentConfig_p = &(_rtl865xB_BandwidthCtrlPerPortConfiguration[port][(input)?0 /* Ingress */:1 /* Egress */]);

	/* We just need to re-config HW when it's updated */
	if ( *currentConfig_p != rate )
	{
		/* Update configuration table */
		*currentConfig_p = rate;

		/* sync the configuration to ASIC */
		_rtl8651_syncToAsicEthernetBandwidthControl();
	}

	return SUCCESS;
err:
	return FAILED;
}


int32 rtl8651_setAsicFlowControlRegister(uint32 port, uint32 enable)
{
	uint32 phyid, statCtrlReg4;

	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
	{
		if ( port > RTL8651_MAC_NUMBER )
		{
			return FAILED;
		}
	} else
	{
		if ( port > RTL8651_PHY_NUMBER )
		{
			return FAILED;
		}
	}
	/* phy id determination */
	phyid = rtl8651AsicEthernetTable[port].phyId;

	/* Read */
	rtl8651_getAsicEthernetPHYReg( phyid, 4, &statCtrlReg4 );

	if ( enable && ( statCtrlReg4 & CAPABLE_PAUSE ) == 0 )
	{
		statCtrlReg4 |= CAPABLE_PAUSE;
	}
	else if ( enable == 0 && ( statCtrlReg4 & CAPABLE_PAUSE ) )
	{
		statCtrlReg4 &= ~CAPABLE_PAUSE;
	}
	else
		return SUCCESS;	/* The configuration does not change. Do nothing. */

	rtl8651_setAsicEthernetPHYReg( phyid, 4, statCtrlReg4 );

	/* restart N-way. */
	rtl8651_restartAsicEthernetPHYNway(port);

	return SUCCESS;
}

int32 rtl8651_getAsicFlowControlRegister(uint32 port, uint32 *enable)
{
	uint32 phyid, statCtrlReg4;

	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
	{
		if ( port > RTL8651_MAC_NUMBER )
		{
			return FAILED;
		}
	} else
	{
		if ( port > RTL8651_PHY_NUMBER )
		{
			return FAILED;
		}
	}
	/* phy id determination */
	phyid = rtl8651AsicEthernetTable[port].phyId;

	/* Read */
	rtl8651_getAsicEthernetPHYReg( phyid, 4, &statCtrlReg4 );

	*enable = ( statCtrlReg4 & CAPABLE_PAUSE )? TRUE: FALSE;

	return SUCCESS;
}

/*
@func int32 | rtl8651_setAsicSystemInputFlowControlRegister | Set System input queue flow control register
@parm uint32 | fcON		| Threshold for Flow control OFF
@parm uint32 | fcOFF		| Threshold for Flow control ON
@rvalue SUCCESS |
@comm
Set input-queue flow control threshold on RTL865xC platform.
 */
int32 rtl8651_setAsicSystemInputFlowControlRegister(uint32 fcON, uint32 fcOFF)
{
	/* Check the correctness */
	if (	(fcON > ( IQ_DSC_FCON_MASK >> IQ_DSC_FCON_OFFSET )) ||
	   	(fcOFF > ( IQ_DSC_FCOFF_MASK >> IQ_DSC_FCOFF_OFFSET ))	)
	{
		return FAILED;
	}

	/* Write the flow control threshold value into ASIC */
	WRITE_MEM32(	IQFCTCR,
			(	(READ_MEM32(IQFCTCR) & ~(IQ_DSC_FCON_MASK | IQ_DSC_FCOFF_MASK)) |
				(fcON << IQ_DSC_FCON_OFFSET) |
				(fcOFF << IQ_DSC_FCOFF_OFFSET))	);
	return SUCCESS;
}
/*
@func int32 | rtl8651_getAsicSystemInputFlowControlRegister | Get System input queue flow control register
@parm uint32* | fcON		| pointer to get Threshold for Flow control OFF
@parm uint32* | fcOFF		| pointer to get Threshold for Flow control ON
@rvalue SUCCESS |
@comm
Set input-queue flow control threshold on RTL865xC platform.
 */
int32 rtl8651_getAsicSystemInputFlowControlRegister(uint32 *fcON, uint32 *fcOFF)
{
	uint32 iqfctcr;

	iqfctcr = READ_MEM32( IQFCTCR );

	if ( fcON )
	{
		*fcON = ( iqfctcr & IQ_DSC_FCON_MASK ) >> IQ_DSC_FCON_OFFSET;
	}

	if ( fcOFF )
	{
		*fcOFF = ( iqfctcr & IQ_DSC_FCOFF_MASK ) >> IQ_DSC_FCOFF_OFFSET;
	}

	return SUCCESS;
}


int32 rtl865xC_setAsicEthernetForceModeRegs(uint32 port, uint32 enForceMode, uint32 forceLink, uint32 forceSpeed, uint32 forceDuplex)
{
	uint32 offset = port * 4;
	uint32 PCR = READ_MEM32( PCRP0 + offset );

	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
	{
		if ( port > RTL8651_MAC_NUMBER )
		{
			return FAILED;
		}
	} else
	{
		if ( port > RTL8651_PHY_NUMBER )
		{
			return FAILED;
		}
	}

	PCR &= ~EnForceMode;
	PCR &= ~ForceLink;
	PCR &= ~ForceSpeedMask;
	PCR &= ~ForceDuplex;

	if ( enForceMode )
	{
		PCR |= EnForceMode;

#if defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
                //ForceMode with polling link status, disable Auto-Negotiation but polling phy's link status
                PCR |= PollLinkStatus;
#endif

		if ( forceLink )
			PCR |= ForceLink;

		if ( forceSpeed == 2 )
			PCR |= ForceSpeed1000M;
		else if ( forceSpeed == 1 )
			PCR |= ForceSpeed100M;
		else
			PCR |= ForceSpeed10M;

		if ( forceDuplex )
			PCR |= ForceDuplex;
	}

	WRITE_MEM32( PCRP0 + offset, PCR );
	mdelay(10);
	TOGGLE_BIT_IN_REG_TWICE(PCRP0 + offset,EnForceMode);
	return SUCCESS;

}


#define	MULTICAST_STORM_CONTROL	1
#define	BROADCAST_STORM_CONTROL	2
#define RTL865XC_MAXALLOWED_BYTECOUNT	30360	/* Used for BSCR in RTL865xC. Means max allowable byte count for 10Mbps port */

static int32 rtl865xC_setBrdcstStormCtrlRate(uint32 percentage)
{
	uint32 rate = RTL865XC_MAXALLOWED_BYTECOUNT * percentage / 100;

	WRITE_MEM32( BSCR, rate );
	return SUCCESS;
}


static int32 rtl8651_perPortStormControl(uint32 type, uint32 portNum, uint32 enable)
{
	uint32 regAddress;
	uint32 oldRegValue;
	uint32 newRegValue;
	uint32 totalExtPortNum=3;

	if(portNum>=RTL8651_PORT_NUMBER + totalExtPortNum)
	{
		rtlglue_printf("wrong port number\n");
		return FAILED;
	}

	regAddress=PCRP0 + portNum * 4;

	oldRegValue=READ_MEM32(regAddress);

	newRegValue=oldRegValue;

	if((type & BROADCAST_STORM_CONTROL) !=0)
	{
		if(enable == TRUE)
		{
			newRegValue = newRegValue |ENBCSC |BCSC_ENBROADCAST;
		}

		if(enable==FALSE)
		{
			newRegValue = newRegValue & (~BCSC_ENBROADCAST);
		}

	}

	if((type & MULTICAST_STORM_CONTROL) !=0)
	{
		if(enable == TRUE)
		{
			newRegValue = newRegValue | ENBCSC |BCSC_ENMULTICAST;
		}

		if(enable==FALSE)
		{
			newRegValue = newRegValue & (~BCSC_ENMULTICAST);
		}

	}

	if((newRegValue & (BCSC_ENMULTICAST |BCSC_ENBROADCAST ))==0)
	{
		/*no needn't storm control*/
		newRegValue = newRegValue & (~ENBCSC);
	}

	if(newRegValue!=oldRegValue)
	{
		WRITE_MEM32(regAddress, newRegValue);
	}

	TOGGLE_BIT_IN_REG_TWICE(regAddress,EnForceMode);

	return SUCCESS;


}

int32 rtl865x_setStormControl(uint32 type,uint32 enable,uint32 percentage)
{
	uint32 port;
	uint32 totalExtPortNum=3;
	for ( port = 0; port < RTL8651_PORT_NUMBER + totalExtPortNum; port++ )
	{
		if(enable==TRUE)
		{
			/*speed unit Mbps*/
			if(percentage>100)
			{
				rtl865xC_setBrdcstStormCtrlRate(100);
			}
			else
			{
				rtl865xC_setBrdcstStormCtrlRate(percentage);
			}

			rtl8651_perPortStormControl(type, port, TRUE);
		}
		else
		{
			rtl865xC_setBrdcstStormCtrlRate(100);
			rtl8651_perPortStormControl(type, port, FALSE);

		}

	}

	return SUCCESS;
}

/*
@func int32 | rtl8651_setAsic802D1xMacBaseAbility | set 802.1x mac based ability
@parm enum PORTID | port | the port number (physical port: 0~5, extension port: 6~8)
@parm uint32* | isEnable | 1: enabled, 0: disabled.
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_setAsic802D1xMacBaseAbility( enum PORTID port, uint32 isEnable )
{
	/* Invalid input parameter */
	if ((port < PHY0) || (port > EXT2))
		return FAILED;

	/* Invalid input parameter */
	if ((isEnable != TRUE) && (isEnable != FALSE))
		return FAILED;

	switch (port)
	{
		case PHY0:
		WRITE_MEM32(DOT1XMACCR, isEnable == TRUE ? (READ_MEM32(DOT1XMACCR) | ( Dot1xMAC_P0En)):
											(READ_MEM32(DOT1XMACCR) & ~( Dot1xMAC_P0En)));
			break;

		case PHY1:
		WRITE_MEM32(DOT1XMACCR, isEnable == TRUE ? (READ_MEM32(DOT1XMACCR) | ( Dot1xMAC_P1En)):
											(READ_MEM32(DOT1XMACCR) & ~( Dot1xMAC_P1En)));
			break;

		case PHY2:
		WRITE_MEM32(DOT1XMACCR, isEnable == TRUE ? (READ_MEM32(DOT1XMACCR) | ( Dot1xMAC_P2En)):
											(READ_MEM32(DOT1XMACCR) & ~( Dot1xMAC_P2En)));
			break;

		case PHY3:
		WRITE_MEM32(DOT1XMACCR, isEnable == TRUE ? (READ_MEM32(DOT1XMACCR) | ( Dot1xMAC_P3En)):
											(READ_MEM32(DOT1XMACCR) & ~( Dot1xMAC_P3En)));
			break;

		case PHY4:
		WRITE_MEM32(DOT1XMACCR, isEnable == TRUE ? (READ_MEM32(DOT1XMACCR) | ( Dot1xMAC_P4En)):
											(READ_MEM32(DOT1XMACCR) & ~( Dot1xMAC_P4En)));
			break;

		case PHY5:
		WRITE_MEM32(DOT1XMACCR, isEnable == TRUE ? (READ_MEM32(DOT1XMACCR) | ( Dot1xMAC_P5En)):
											(READ_MEM32(DOT1XMACCR) & ~( Dot1xMAC_P5En)));
			break;

		case CPU:
		WRITE_MEM32(DOT1XMACCR, isEnable == TRUE ? (READ_MEM32(DOT1XMACCR) | ( Dot1xMAC_P6En)):
											(READ_MEM32(DOT1XMACCR) & ~( Dot1xMAC_P6En)));
			break;

		case EXT1:
		WRITE_MEM32(DOT1XMACCR, isEnable == TRUE ? (READ_MEM32(DOT1XMACCR) | ( Dot1xMAC_P7En)):
											(READ_MEM32(DOT1XMACCR) & ~( Dot1xMAC_P7En)));
			break;

		case EXT2:
		WRITE_MEM32(DOT1XMACCR, isEnable == TRUE ? (READ_MEM32(DOT1XMACCR) | ( Dot1xMAC_P8En)):
											(READ_MEM32(DOT1XMACCR) & ~( Dot1xMAC_P8En)));
			break;

		case EXT3:
			return FAILED;

		case MULTEXT:
			return FAILED;

	}
	return SUCCESS;
}

/*
@func int32 | rtl8651_setAsic802D1xMacBaseDirection | set 802.1x mac based direction
@parm enum uint32 | dir | OperCOnntrolledDirections for MAC-Based ACCESS Control. 0:BOTH, 1:IN
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_setAsic802D1xMacBaseDirection(int32 dir)
{


	/* Invalid input parameter */
	if ((dir != Dot1xMAC_OPDIR_IN) && (dir != Dot1xMAC_OPDIR_BOTH))
	{
		return FAILED;
	}

	if(dir == Dot1xMAC_OPDIR_IN)
	{
		WRITE_MEM32(DOT1XMACCR,(READ_MEM32(DOT1XMACCR)) |Dot1xMAC_OPDIR);
	}
	else
	{
		WRITE_MEM32(DOT1XMACCR,(READ_MEM32(DOT1XMACCR)) &(~Dot1xMAC_OPDIR));
	}
	return SUCCESS;
}

/*
@func int32 | rtl8651_setAsicGuestVlanProcessControl | set guest vlan process control
@parm enum  uint32 | process |default process for unauthenticated client  (00~11)<<12
@rvalue SUCCESS
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_setAsicGuestVlanProcessControl( uint32 process)
{
	/* Invalid input parameter */
	if((process < Dot1xUNAUTHBH_DROP) || (process > Dot1xUNAUTHBH_RESERVED))
		return FAILED;

	WRITE_MEM32(GVGCR, ((READ_MEM32(GVGCR)) & 0x0fff) | process);

	return SUCCESS;
}


/*
@func int32 | rtl8651_setAsicDot1qAbsolutelyPriority | set 802.1Q absolutely priority
@parm enum PRIORITYVALUE | srcpriority | priority value
@parm enum PRIORITYVALUE | priority | absolute priority value
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_setAsicDot1qAbsolutelyPriority( enum PRIORITYVALUE srcpriority, enum PRIORITYVALUE priority )
{
	/* Invalid input parameter */
	if ((srcpriority < PRI0) || (srcpriority > PRI7) || (priority < PRI0) || (priority > PRI7))
		return FAILED;

	switch (srcpriority)
	{
		case PRI0:
			WRITE_MEM32(LPTM8021Q, (READ_MEM32(LPTM8021Q) & ~(EN_8021Q2LTMPRI0_MASK)) | (priority << EN_8021Q2LTMPRI0)); break;
		case PRI1:
			WRITE_MEM32(LPTM8021Q, (READ_MEM32(LPTM8021Q) & ~(EN_8021Q2LTMPRI1_MASK)) | (priority << EN_8021Q2LTMPRI1)); break;
		case PRI2:
			WRITE_MEM32(LPTM8021Q, (READ_MEM32(LPTM8021Q) & ~(EN_8021Q2LTMPRI2_MASK)) | (priority << EN_8021Q2LTMPRI2)); break;
		case PRI3:
			WRITE_MEM32(LPTM8021Q, (READ_MEM32(LPTM8021Q) & ~(EN_8021Q2LTMPRI3_MASK)) | (priority << EN_8021Q2LTMPRI3)); break;
		case PRI4:
			WRITE_MEM32(LPTM8021Q, (READ_MEM32(LPTM8021Q) & ~(EN_8021Q2LTMPRI4_MASK)) | (priority << EN_8021Q2LTMPRI4)); break;
		case PRI5:
			WRITE_MEM32(LPTM8021Q, (READ_MEM32(LPTM8021Q) & ~(EN_8021Q2LTMPRI5_MASK)) | (priority << EN_8021Q2LTMPRI5)); break;
		case PRI6:
			WRITE_MEM32(LPTM8021Q, (READ_MEM32(LPTM8021Q) & ~(EN_8021Q2LTMPRI6_MASK)) | (priority << EN_8021Q2LTMPRI6)); break;
		case PRI7:
			WRITE_MEM32(LPTM8021Q, (READ_MEM32(LPTM8021Q) & ~(EN_8021Q2LTMPRI7_MASK)) | (priority << EN_8021Q2LTMPRI7)); break;
	}

	return SUCCESS;
}


/*
@func int32 | rtl8651_getAsicDot1qAbsolutelyPriority | get 802.1Q absolutely priority
@parm enum PRIORITYVALUE | srcpriority | priority value
@parm enum PRIORITYVALUE* | pPriority | pPriority will return the absolute priority value
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_getAsicDot1qAbsolutelyPriority( enum PRIORITYVALUE srcpriority, enum PRIORITYVALUE *pPriority )
{

	/* Invalid input parameter */
	if ((srcpriority < PRI0) || (srcpriority > PRI7))
		return FAILED;

	if (pPriority != NULL)
	{
		switch (srcpriority)
		{
			case PRI0:
				*pPriority = (READ_MEM32(LPTM8021Q) & EN_8021Q2LTMPRI0_MASK) >> EN_8021Q2LTMPRI0;  break;
			case PRI1:
				*pPriority = (READ_MEM32(LPTM8021Q) & EN_8021Q2LTMPRI1_MASK) >> EN_8021Q2LTMPRI1;  break;
			case PRI2:
				*pPriority = (READ_MEM32(LPTM8021Q) & EN_8021Q2LTMPRI2_MASK) >> EN_8021Q2LTMPRI2;  break;
			case PRI3:
				*pPriority = (READ_MEM32(LPTM8021Q) & EN_8021Q2LTMPRI3_MASK) >> EN_8021Q2LTMPRI3;  break;
			case PRI4:
				*pPriority = (READ_MEM32(LPTM8021Q) & EN_8021Q2LTMPRI4_MASK) >> EN_8021Q2LTMPRI4;  break;
			case PRI5:
				*pPriority = (READ_MEM32(LPTM8021Q) & EN_8021Q2LTMPRI5_MASK) >> EN_8021Q2LTMPRI5;  break;
			case PRI6:
				*pPriority = (READ_MEM32(LPTM8021Q) & EN_8021Q2LTMPRI6_MASK) >> EN_8021Q2LTMPRI6;  break;
			case PRI7:
				*pPriority = (READ_MEM32(LPTM8021Q) & EN_8021Q2LTMPRI7_MASK) >> EN_8021Q2LTMPRI7;  break;
		}
	}

	return SUCCESS;
}


#if	defined(CONFIG_RTL_QOS_8021P_SUPPORT)

/*
@func int32 | rtl8651_flushAsicDot1qAbsolutelyPriority | set 802.1Q absolutely priority the default value 0
@parm void
@rvalue SUCCESS |
@comm
 */
int32 rtl8651_flushAsicDot1qAbsolutelyPriority(void)
{
	WRITE_MEM32(LPTM8021Q, 0);
	return SUCCESS;
}
#endif

#ifdef CONFIG_RTL_LINKCHG_PROCESS
/*
@func int32 | rtl8651_updateAsicLinkAggregatorLMPR | Arrange the table which maps hashed index to port.
@parm	uint32	|	portMask |  Specify the port mask for the aggregator.
@rvalue SUCCESS | Update the mapping table successfully.
@rvalue FAILED | When the port mask is invalid, return FAILED
@comm
RTL865x provides an aggregator port. This API updates the table which maps hashed index to port.
If portmask = 0: clear all aggregation port mappings.
Rearrange policy is round-robin. ie. if port a,b,c is in portmask, then hash block 0~7's port number is a,b,c,a,b,c,a,b
*/
int32  rtl8651_updateAsicLinkAggregatorLMPR(int32 portmask)
{
	uint32 hIdx, portIdx, reg;

	/* Clear all mappings */
	WRITE_MEM32( LAGHPMR0, 0 );

	if ( portmask == 0 )
	{
		return SUCCESS;
	}

	reg = 0;
	portIdx = 0;
	for ( hIdx = 0; hIdx < RTL865XC_LAGHASHIDX_NUMBER; hIdx++ )
	{
		while ( ( ( 1 << portIdx ) & portmask ) == 0 )	/* Don't worry about infinite loop because portmask won't be 0. */
		{
			portIdx = ( portIdx + 1 ) % ( RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum );
		}

		reg |= ( portIdx << ( hIdx * LAG_HASHIDX_BITNUM ) );
		portIdx = ( portIdx + 1 ) % ( RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum );
	}

	WRITE_MEM32( LAGHPMR0, reg );
	return SUCCESS;
}



#if 0
static uint32 _rtl8651_findAsicLinkupPortmask(uint32 portMask)
{
	uint32 port, lnkUp_portMask = portMask;
	for ( port = 0; port < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum; port++ )
	{
		if ( ( portMask & ( 1 << port ) ) && (rtl8651AsicEthernetTable[port].linkUp == FALSE) )
		{
			lnkUp_portMask &= ~( 1 << port );
		}
	}
	return lnkUp_portMask;
}
#endif

int32 rtl8651_setAsicEthernetLinkStatus(uint32 port, int8 linkUp)
{
	int8 notify;
//	uint32 portmask;

	if (port >= (RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum))
	{
		return FAILED;
	}

	notify = (rtl8651AsicEthernetTable[port].linkUp != ((linkUp==TRUE)? TRUE: FALSE))?TRUE:FALSE;


	rtl8651AsicEthernetTable[port].linkUp = (linkUp == TRUE)? TRUE: FALSE;

#if 0
	/*
		If the specified port is a member of the aggregator,
		update the table which maps hashed index to the port
		because the member port of the aggregator link changes.
	*/
	portmask = READ_MEM32( LAGCR0 ) & TRUNKMASK_MASK;
	if ( portmask & ( 1 << port ) )
	{
		/* Find the link-up portmask */
		uint32 lnkUp_portMask = _rtl8651_findAsicLinkupPortmask(portmask);
		rtl8651_updateAsicLinkAggregatorLMPR( lnkUp_portMask );
	}
#endif

	return SUCCESS;
}
#endif

#if defined (CONFIG_RTL_ENABLE_RATELIMIT_TABLE)
int32 rtl8651_setAsicRateLimitTable(uint32 index, rtl865x_tblAsicDrv_rateLimitParam_t *rateLimit_t)
{
	rtl8651_tblAsic_rateLimitTable_t entry;

	if (rateLimit_t == NULL || index >= RTL8651_RATELIMITTBL_SIZE)
		return FAILED;
	memset(&entry,0,sizeof(rtl8651_tblAsic_rateLimitTable_t));
	entry.maxToken				= rateLimit_t->maxToken&0xFFFFFF;
	entry.refill				= rateLimit_t->refill_number&0xFFFFFF;
	entry.refillTime			= rateLimit_t->t_intervalUnit&0x3F;
	entry.refillRemainTime		= rateLimit_t->t_remainUnit&0x3F;
	entry.token					= rateLimit_t->token&0xFFFFFF;
	return _rtl8651_forceAddAsicEntry(TYPE_RATE_LIMIT_TABLE, index, &entry);
}


int32 rtl8651_delAsicRateLimitTable(uint32 index)
{
	rtl8651_tblAsic_rateLimitTable_t entry;

	if (index >= RTL8651_RATELIMITTBL_SIZE)
		return FAILED;
	memset(&entry,0,sizeof(rtl8651_tblAsic_rateLimitTable_t));
	return _rtl8651_forceAddAsicEntry(TYPE_RATE_LIMIT_TABLE, index, &entry);
}


int32 rtl8651_getAsicRateLimitTable(uint32 index, rtl865x_tblAsicDrv_rateLimitParam_t *rateLimit_t)
{
	rtl8651_tblAsic_rateLimitTable_t entry;

	if (rateLimit_t == NULL || index >= RTL8651_RATELIMITTBL_SIZE)
		return FAILED;
	_rtl8651_readAsicEntry(TYPE_RATE_LIMIT_TABLE, index, &entry);
	if (entry.refillTime == 0)
		return FAILED;
	rateLimit_t->token			= entry.token & 0xFFFFFF;
	rateLimit_t->maxToken		= entry.maxToken & 0xFFFFFF;
	rateLimit_t->t_remainUnit = entry.refillRemainTime&0x3F;
	rateLimit_t->t_intervalUnit = entry.refillTime&0x3F;
	rateLimit_t->refill_number	= entry.refill&0xFFFFFF;
	return SUCCESS;
}
#endif


int32 rtl8651_setPortFlowControlConfigureRegister(enum PORTID port,uint32 enable)
{
	int enable_port = 0;
	if(enable)
		enable_port = 0x3f;
		
	switch (port)
	{
		case PHY0:
			WRITE_MEM32(FCCR0, (READ_MEM32(FCCR0) & ~(0x3F<<Q_P0_EN_FC_OFFSET)) | (enable_port << Q_P0_EN_FC_OFFSET));  break;
		case PHY1:
			WRITE_MEM32(FCCR0, (READ_MEM32(FCCR0) & ~(0x3F<<Q_P1_EN_FC_OFFSET)) | (enable_port << Q_P1_EN_FC_OFFSET));  break;
		case PHY2:
			WRITE_MEM32(FCCR0, (READ_MEM32(FCCR0) & ~(0x3F<<Q_P2_EN_FC_OFFSET)) | (enable_port << Q_P2_EN_FC_OFFSET));  break;
		case PHY3:
			WRITE_MEM32(FCCR0, (READ_MEM32(FCCR0) & ~(0x3F<<Q_P3_EN_FC_OFFSET)) | (enable_port << Q_P3_EN_FC_OFFSET));  break;
		case PHY4:
			WRITE_MEM32(FCCR1, (READ_MEM32(FCCR1) & ~(0x3F<<Q_P4_EN_FC_OFFSET)) | (enable_port << Q_P4_EN_FC_OFFSET));  break;
		case PHY5:
			WRITE_MEM32(FCCR1, (READ_MEM32(FCCR1) & ~(0x3F<<Q_P5_EN_FC_OFFSET)) | (enable_port << Q_P5_EN_FC_OFFSET));  break;
		case CPU:
			WRITE_MEM32(FCCR1, (READ_MEM32(FCCR1) & ~(0x1<<Q_P6_EN_FC_OFFSET)) | (enable_port << Q_P6_EN_FC_OFFSET));  break;
		default:
			return FAILED;
	}

	return SUCCESS;
}
/*
@func int32 | rtl8651_setAsicPortPriority | set port based priority
@parm enum PORTID | port | the port number (valid: physical ports(0~5) and extension ports(7~9) )
@parm enum PRIORITYVALUE | priority | priority value.
@rvalue SUCCESS | 
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_setAsicPortPriority( enum PORTID port, enum PRIORITYVALUE priority )
{
	/* Invalid input parameter */
	if ((priority < PRI0) || (priority > PRI7))
		return FAILED; 

	switch (port)
	{
		case PHY0:
			WRITE_MEM32(PBPCR, (READ_MEM32(PBPCR) & ~(PBPRI_P0_MASK)) | (priority << PBPRI_P0_OFFSET)); break;
		case PHY1:
			WRITE_MEM32(PBPCR, (READ_MEM32(PBPCR) & ~(PBPRI_P1_MASK)) | (priority << PBPRI_P1_OFFSET)); break;
		case PHY2:
			WRITE_MEM32(PBPCR, (READ_MEM32(PBPCR) & ~(PBPRI_P2_MASK)) | (priority << PBPRI_P2_OFFSET)); break;
		case PHY3:
			WRITE_MEM32(PBPCR, (READ_MEM32(PBPCR) & ~(PBPRI_P3_MASK)) | (priority << PBPRI_P3_OFFSET)); break;
		case PHY4:
			WRITE_MEM32(PBPCR, (READ_MEM32(PBPCR) & ~(PBPRI_P4_MASK)) | (priority << PBPRI_P4_OFFSET)); break;
		case PHY5:
			WRITE_MEM32(PBPCR, (READ_MEM32(PBPCR) & ~(PBPRI_P5_MASK)) | (priority << PBPRI_P5_OFFSET)); break;
		case EXT1:
			WRITE_MEM32(PBPCR, (READ_MEM32(PBPCR) & ~(PBPRI_P6_MASK)) | (priority << PBPRI_P6_OFFSET)); break;
		case EXT2:
			WRITE_MEM32(PBPCR, (READ_MEM32(PBPCR) & ~(PBPRI_P7_MASK)) | (priority << PBPRI_P7_OFFSET)); break;
		case EXT3:
			WRITE_MEM32(PBPCR, (READ_MEM32(PBPCR) & ~(PBPRI_P8_MASK)) | (priority << PBPRI_P8_OFFSET)); break;
		case CPU: /* fall thru */
		default:
			return FAILED;
	}

	return SUCCESS;
}
/*
@func int32 | rtl8651_getAsicPortPriority | get port based priority
@parm enum PORTID | port | the port number (valid: physical ports(0~5) and extension ports(7~9) )
@parm enum PRIORITYVALUE* | pPriority | pPriority will return the priority of the specified port.
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_getAsicPortPriority( enum PORTID port, enum PRIORITYVALUE *pPriority )
{
        if (pPriority != NULL)
        {
                switch (port)
                {
                        case PHY0:
                                *pPriority = (READ_MEM32(PBPCR) & PBPRI_P0_MASK) >> PBPRI_P0_OFFSET;  break;
                        case PHY1:
                                *pPriority = (READ_MEM32(PBPCR) & PBPRI_P1_MASK) >> PBPRI_P1_OFFSET;  break;
                        case PHY2:
                                *pPriority = (READ_MEM32(PBPCR) & PBPRI_P2_MASK) >> PBPRI_P2_OFFSET;  break;
                        case PHY3:
                                *pPriority = (READ_MEM32(PBPCR) & PBPRI_P3_MASK) >> PBPRI_P3_OFFSET;  break;
                        case PHY4:
                                *pPriority = (READ_MEM32(PBPCR) & PBPRI_P4_MASK) >> PBPRI_P4_OFFSET;  break;
                        case PHY5:
                                *pPriority = (READ_MEM32(PBPCR) & PBPRI_P5_MASK) >> PBPRI_P5_OFFSET;  break;
                        case EXT1:
                                *pPriority = (READ_MEM32(PBPCR) & PBPRI_P6_MASK) >> PBPRI_P6_OFFSET;  break;
                        case EXT2:
                                *pPriority = (READ_MEM32(PBPCR) & PBPRI_P7_MASK) >> PBPRI_P7_OFFSET;  break;
                        case EXT3:
                                *pPriority = (READ_MEM32(PBPCR) & PBPRI_P8_MASK) >> PBPRI_P8_OFFSET;  break;
                        case CPU: /* fall thru */
                        default:
                                return FAILED;
                }
        }

        return SUCCESS;
}

/*
 * @func int32 | rtl8651_setAsicDscpPriority | set DSCP-based priority
 * @parm uint32 | dscp | DSCP value
 * @parm enum PRIORITYVALUE | priority | priority value
 * @rvalue SUCCESS |
 * @rvalue FAILED | invalid parameter
 * @comm
 *  */
int32 rtl8651_setAsicDscpPriority( uint32 dscp, enum PRIORITYVALUE priority )
{
	/* Invalid input parameter */
	if ((dscp < 0) || (dscp > 63))
		return FAILED;
	if ((priority < PRI0) || (priority > PRI7))
		return FAILED;

	if ((0 <= dscp) && (dscp <= 9))
		WRITE_MEM32(DSCPCR0, (READ_MEM32(DSCPCR0) & ~(0x7 << (dscp*3))) | (priority << (dscp*3)));
	else if ((10 <= dscp) && (dscp <= 19))
		WRITE_MEM32(DSCPCR1, (READ_MEM32(DSCPCR1) & ~(0x7 << ((dscp%10)*3))) | (priority << ((dscp%10)*3)));
	else if ((20 <= dscp) && (dscp <= 29))
		WRITE_MEM32(DSCPCR2, (READ_MEM32(DSCPCR2) & ~(0x7 << ((dscp%10)*3))) | (priority << ((dscp%10)*3)));
	else if ((30 <= dscp) && (dscp <= 39))
		WRITE_MEM32(DSCPCR3, (READ_MEM32(DSCPCR3) & ~(0x7 << ((dscp%10)*3))) | (priority << ((dscp%10)*3)));
	else if ((40 <= dscp) && (dscp <= 49))
		WRITE_MEM32(DSCPCR4, (READ_MEM32(DSCPCR4) & ~(0x7 << ((dscp%10)*3))) | (priority << ((dscp%10)*3)));
	else if ((50 <= dscp) && (dscp <= 59))
		WRITE_MEM32(DSCPCR5, (READ_MEM32(DSCPCR5) & ~(0x7 << ((dscp%10)*3))) | (priority << ((dscp%10)*3)));
	else if ((60 <= dscp) && (dscp <= 63))
		WRITE_MEM32(DSCPCR6, (READ_MEM32(DSCPCR6) & ~(0x7 << ((dscp%10)*3))) | (priority << ((dscp%10)*3)));

	return SUCCESS;
}


/*
 * @func int32 | rtl8651_setAsicDscpPriority | set DSCP-based priority
 * @parm uint32 | dscp | DSCP value
 * @parm enum PRIORITYVALUE* | pPriority | pPriority will return the priority of the specified DSCP
 * @rvalue SUCCESS |
 * @rvalue FAILED | invalid parameter
 * @comm
 *  */
int32 rtl8651_getAsicDscpPriority( uint32 dscp, enum PRIORITYVALUE *pPriority )
{
	/* Invalid input parameter */
	if ((dscp < 0) || (dscp > 63))
		return FAILED;

	if (pPriority != NULL)
	{
		if ((0 <= dscp) && (dscp <= 9))
			*pPriority = (READ_MEM32(DSCPCR0) & (0x7 << (dscp*3))) >> (dscp*3);
		else if ((10 <= dscp) && (dscp <= 19))
			*pPriority = (READ_MEM32(DSCPCR1) & (0x7 << ((dscp%10)*3))) >> ((dscp%10)*3);
		else if ((20 <= dscp) && (dscp <= 29))
			*pPriority = (READ_MEM32(DSCPCR2) & (0x7 << ((dscp%10)*3))) >> ((dscp%10)*3);
		else if ((30 <= dscp) && (dscp <= 39))
			*pPriority = (READ_MEM32(DSCPCR3) & (0x7 << ((dscp%10)*3))) >> ((dscp%10)*3);
		else if ((40 <= dscp) && (dscp <= 49))
			*pPriority = (READ_MEM32(DSCPCR4) & (0x7 << ((dscp%10)*3))) >> ((dscp%10)*3);
		else if ((50 <= dscp) && (dscp <= 59))
			*pPriority = (READ_MEM32(DSCPCR5) & (0x7 << ((dscp%10)*3))) >> ((dscp%10)*3);
		else if ((60 <= dscp) && (dscp <= 63))
			*pPriority = (READ_MEM32(DSCPCR6) & (0x7 << ((dscp%10)*3))) >> ((dscp%10)*3);
	}

	return SUCCESS;
}

#ifdef CONFIG_RTK_VOIP_QOS

int32 rtl8651_reset_dscp_priority(void)
{
	//clear dscp priority assignment, otherwise pkt with dscp value 0 will be assign priority 1
	WRITE_MEM32(DSCPCR0,0);
	WRITE_MEM32(DSCPCR1,0);
	WRITE_MEM32(DSCPCR2,0);
	WRITE_MEM32(DSCPCR3,0);
	WRITE_MEM32(DSCPCR4,0);
	WRITE_MEM32(DSCPCR5,0);
	WRITE_MEM32(DSCPCR6,0);
	return 0;
}
int32 rtl8651_cpu_tx_fc(int enable)
{
        if(enable)
                REG32(PSRP6_RW) |= 0x40;
        else
                REG32(PSRP6_RW) &= 0xffffffbf;
        return 0;
}
int32 rtl8651_setQueueNumber(int port, int qnum)
{
        rtl865xC_lockSWCore();
        rtl8651_setAsicOutputQueueNumber(port, qnum);
        rtl865xC_waitForOutputQueueEmpty();
        rtl8651_resetAsicOutputQueue();
        rtl865xC_unLockSWCore();
        return SUCCESS;
}
#endif
#ifdef CONFIG_RTK_VOIP_PORT_LINK
int rtl8651_getAsicEthernetLinkStatus(uint32 port, int8 *linkUp)
{

        int status = READ_MEM32( PSRP0 + port * 4 );

        if(status & PortStatusLinkUp)
                *linkUp = TRUE;
        else
                *linkUp = FALSE;


	return SUCCESS;
}
#endif
