/*
################################################################################
# 
# RTL8198 MDIO char driver header
# 
# Copyright(c) 2010 Realtek Semiconductor Corp. All rights reserved.
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
# this program; if not, see <http://www.gnu.org/licenses/>.
# 
# Author:
# Realtek WiFi AP software team <cn_sd8@realtek.com>
# No. 2, Innovation Road II, Hsinchu Science Park, Hsinchu 300, Taiwan
# 
################################################################################
*/
/*================================================================*/

#ifndef INCLUDE_RTL_MDIO_H
#define INCLUDE_RTL_MDIO_H


/*================================================================*/
/* Compiling Flags */

#define KDB_ERR								// defined to print out error message
//#define KDB_MSG							// defined to print out debug message
//#define SIMULATION						// defined to do simuation
#ifndef CONFIG_RTK_VOIP_ETHERNET_DSP_IS_DSP
#define JUMP_CMD							// defined to add jump ioctl cmd, for development purpose
#endif


/*================================================================*/
/* Constant Definitions */

#define DRIVER_NAME			"rtl_mdio"
#define DRIVER_VER			"0.2"
#define IO_LEN				0x40
#define DRIVER_MAJOR		14
#define MDIO_BUFSIZE		516 //2 + 2 + 2*255 + 2
#define EV_QUE_MAX			16
//#define BIT(x)				(1 << (x))

// RTL8197B IRQ number and Register offset
#define NFBI_BASE				0xb8019000				// NFBI base address
#define REG_RCR					(0x00+NFBI_BASE)	// Receive Command Register
#define REG_SSR					(0x04+NFBI_BASE)	// Send Status Register
#define REG_SYSCR				(0x08+NFBI_BASE)	// System Control Register
#define REG_SYSSR				(0x0C+NFBI_BASE)	// System Status Register
#define REG_IMR					(0x10+NFBI_BASE)	// Interrupt Mask Register
#define REG_ISR 				(0x14+NFBI_BASE)	// Interrupt Status Register
#define REG_BMCR				(0x20+NFBI_BASE)	// Basic Mode Control Register
#define REG_BMSR				(0x24+NFBI_BASE)	// Basic Mode Status Register
#define REG_ANAR				(0x28+NFBI_BASE)	// Auto-Negotiation Advertisement Register
#define REG_ANLPAR 			    (0x2C+NFBI_BASE) 	// Auto-Negotiation Link Partner Ability Register
#define REG_NFBIRR 			    (0x30+NFBI_BASE)	// NFBI Reset Control Register

// bitmask definition for ISR
#define IP_ISOLATION		BIT(15)	// ISOLATION Interrupt Pending
#define IP_ETHMAC			BIT(14)	// Enable/disable Ethernet MAC
#define IP_WLANMAC			BIT(13)	// Enable/disable WLAN MAC
#define IP_ETHPHY			BIT(12)	// Enable/disable Ethernet PHY
#define IP_WLANPHY			BIT(11)	// Enable/disable WLAN PHY
#define IP_SELMIICLK		BIT(10)	//	Select MII Clock Speed
#define IP_RSVD9			BIT(9)		//	Reserved
#define IP_CUSTOM8			BIT(8)		//	Customized used 8
#define IP_CUSTOM7			BIT(7)		//	Customized used 7
#define IP_CUSTOM6			BIT(6)		//	Customized used 6
#define IP_CUSTOM5			BIT(5)		//	Customized used 5
#define IP_CUSTOM4			BIT(4)		//	Customized used 4
#define IP_CUSTOM3			BIT(3)		//	Customized used 3
#define IP_MSGFETCH			BIT(2)		//	Previous msg has been fetched
#define IP_NEWMSG			BIT(1)		//	New msg has come
#define IP_RSVD0			BIT(0)		//	Reserved

// bitmask definition for SCR
#define CR_ISOLATION		BIT(15)	// ISOLATION control bit
#define CR_ETHMAC			BIT(14)	// Ethernet MAC control bit
#define CR_WLANMAC			BIT(13)	// WLAN MAC control bit
#define CR_ETHPHY			BIT(12)	// Ethernet PHY control bi
#define CR_WLANPHY			BIT(11)	// WLAN PHY control bit
#define CR_SELMIICLK		(BIT(10)|BIT(9))	//	Select MII Clock Speed control bit
#define CR_CUSTOM8			BIT(8)		//	Customized used 8
#define CR_CUSTOM7			BIT(7)		//	Customized used 7
#define CR_CUSTOM6			BIT(6)		//	Customized used 6
#define CR_CUSTOM5			BIT(5)		//	Customized used 5
#define CR_CUSTOM4			BIT(4)		//	Customized used 4
#define CR_CUSTOM3			BIT(3)		//	Customized used 3
#define CR_CUSTOM2			BIT(2)		//	Customized used 2
#define CR_CUSTOM1			BIT(1)		//	Customized used 1
#define CR_CUSTOM0			BIT(0)		//	Customized used 0

// bitmask definition for SYSSR
#define SR_CheckSumDone				BIT(15)
#define SR_CheckSumOK				BIT(14)
#define SR_WLANLink					BIT(13)
#define SR_EthLink					BIT(12)
#define SR_EthPHYStatusChange		BIT(11)
#define SR_AllSoftwareReady			BIT(10)
#define SR_USBInsertStatus			BIT(7)
#define SR_USBRemoveStatus			BIT(6)
#define SR_BootcodeReady			BIT(5)

// rx cmd id bitmask
#define FIRST_CMD_MASK	BIT(15)

// All received interrupt mask
#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP_IS_DSP
#define NEEDED_IRQ_MASK	(IP_NEWMSG|IP_MSGFETCH|IP_SELMIICLK|IP_WLANPHY| \
														IP_ETHPHY|IP_WLANMAC|IP_ETHMAC|IP_ISOLATION|IP_CUSTOM3)
#else
#define NEEDED_IRQ_MASK	(IP_NEWMSG|IP_MSGFETCH|IP_SELMIICLK|IP_WLANPHY| \
														IP_ETHPHY|IP_WLANMAC|IP_ETHMAC|IP_ISOLATION)
#endif

/* Ether register base address */														
#define MACCR_BASE			                0xbb804000				/* Internal Ether MAC base address */
#define PCRAM_BASE 			                (MACCR_BASE+0x100)		/* Per-port Configuration Register */
#define PCRP0						        (0x004 + PCRAM_BASE)	/* Port Configuration Register of Port 0 */
#define PCRP3                               (0x010 + PCRAM_BASE)    /* Port Configuration Register of Port 3 */
#define P0GMIICR                            (0x04C + PCRAM_BASE)    /* Port-0 GMII Configuration Register */
#define PauseFlowControl	                8						/* Bit-shift number of PauseFlowControl */

/* MAC control register field definitions */
#define MDCIOCR					(0x004+MACCR_BASE)		/* MDC/MDIO Command */
#define MDCIOSR					(0x008+MACCR_BASE)		/* MDC/MDIO Status */
//#define STATUS				(1<<31)				/* 0: Process Done, 1: In progress */
//STATUS has been defined in linux-2.6.30/arch/rlx/include/asm/ptrace.h
#define MDCIO_STATUS                            (1<<31)                         /* 0: Process Done, 1: In progress */

/* MDCIOCR - MDC/MDIO Command */
#define COMMAND_READ	(0<<31)		/* 0:Read Access, 1:Write Access */
#define COMMAND_WRITE	(1<<31)		/* 0:Read Access, 1:Write Access */

#define PHYADD_OFFSET	(24)				/* PHY Address, said, PHY ID */
#define REGADD_OFFSET	(16)				/* PHY Register */

#define ETH_PORT_NUM		(3)				/* Port number of built-in Ether phy */
#define MII_PORT_NUM		(0)				/* Port number of NFBI MII */

// rx cmd state
enum { 
	STATE_RX_INIT, 
	STATE_RX_WAIT_LEN, 
	STATE_RX_WAIT_DATA, 
	STATE_RX_WAIT_DAEMON 
};

// tx status state
enum { 
	STATE_TX_INIT, 
	STATE_TX_IN_PROGRESS 
};

// indication event id
enum {
	IND_CMD_EV,
	IND_SYSCTL_EV
};

// cmd id of write data
enum {
	WRITE_MDIO,
	SET_CMD_TIMEOUT,
	SET_PHY_POLL_TIME,	
	SET_HOST_PID,	
};

// cmd id of ioctl 
#define MDIO_IOC_MAGIC  'k'  

#define MDIO_IOCTL_SET_HOST_PID				_IOW(MDIO_IOC_MAGIC,  0, int)
#define MDIO_IOCTL_SET_CMD_TIMEOUT		    _IOW(MDIO_IOC_MAGIC,  1, char)
#define MDIO_IOCTL_SET_PHY_POLL_TIME	    _IOW(MDIO_IOC_MAGIC,  2, char)
#define MDIO_IOCTL_READ_MEM					_IOWR(MDIO_IOC_MAGIC, 3, int)
#define MDIO_IOCTL_WRITE_MEM				_IOW(MDIO_IOC_MAGIC,  4, struct mdio_mem32_param)
#define MDIO_IOCTL_SET_MII_PAUSE			_IOW(MDIO_IOC_MAGIC,  5, char)
#define MDIO_IOCTL_SET_SUSPEND				_IOW(MDIO_IOC_MAGIC,  6, char)
#define MDIO_IOCTL_READ_SCR					_IOR(MDIO_IOC_MAGIC,  7, int)
#define MDIO_IOCTL_JUMP_ADDR				_IOW(MDIO_IOC_MAGIC,  8, int)
#define MDIO_IOCTL_PRIV_CMD                 _IOW(MDIO_IOC_MAGIC,  9, int)
#define MDIO_IOCTL_GET_REG                  _IOWR(MDIO_IOC_MAGIC, 10, struct reg_param)
#define MDIO_IOCTL_SET_REG                  _IOW(MDIO_IOC_MAGIC,  11, struct reg_param)
#define MDIO_IOCTL_SET_MII_CLK  			_IOW(MDIO_IOC_MAGIC,  12, int)
#define MDIO_IOCTL_SET_ETH_PAUSE			_IOW(MDIO_IOC_MAGIC,  13, char)
#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP_IS_DSP
#define MDIO_IOCTL_READ_DSP_ID			_IOW(MDIO_IOC_MAGIC, 14, unsigned int)
#define MDIO_IOCTL_MAXNR 					14
#else
#define MDIO_IOCTL_MAXNR 					13
#endif

#define TIME_DIFF(a, b)					((a >= b)? (a - b):(0xffffffff - b + a + 1))
#define EVT_BUF_OFFSET				((int)(long *)&(((struct evt_msg *)0)->buf))
#ifdef REG32
	#undef REG32
#endif
#define REG32(reg)							(*((volatile unsigned long *)(reg)))

#ifdef WRITE_MEM32
	#undef WRITE_MEM32
	#undef READ_MEM32	
#endif
#define WRITE_MEM32(reg,val)	REG32(reg)=val
#define READ_MEM32(reg)				REG32(reg)

#ifndef SIMULATION
#define register_read_dw(offset)			 	(REG32(offset))
#define register_write_dw(offset, data)	(REG32(offset)=data)
#endif

#define PUT_IN_DATA(data) { \
		memcpy(&priv->data_in.buf[priv->data_in.len], &data, 2); \
		priv->data_in.len += 2; \
}

#define RESET_RX_STATE { \
		priv->rx_cmd_state = STATE_RX_INIT; \
		priv->data_in.len = 0; \
		priv->rx_cmd_time = 0; \
}

#ifdef KDB_MSG
	#define DEBUGK_OUT(fmt, args...)		printk("%s_%s: "fmt, DRIVER_NAME, __FUNCTION__, ## args)

	#define ASSERT(expr) \
        if(!(expr)) {					\
  			printk( "\033[33;41m%s:%d: assert(%s)\033[m\n",	\
	        __FILE__,__LINE__,#expr);		\
        }	
#else
	#define DEBUGK_OUT(fmt, args...)

	#define ASSERT(expr)	
#endif

#ifdef KDB_ERR
	#define DEBUGK_ERR(fmt, args...)		printk("%s_%s_ERR: "fmt, DRIVER_NAME, __FUNCTION__, ## args)
#else
	#define DEBUGK_ERR(fmt, args...)
#endif

/*================================================================*/
/* Structure Definition */

struct buf_ar {
	int len;
	unsigned char buf[MDIO_BUFSIZE];	
};

struct evt_msg {
	int id;			// event id
	int len;		// length in buf
	unsigned char buf[MDIO_BUFSIZE];	
};

struct mdio_mem32_param {
	unsigned long addr;
	unsigned long val;
};

struct reg_param {
	unsigned long addr;
	unsigned long val;
};

#ifdef __KERNEL__
struct mdio_priv {	
	int cmd_timeout;	// in 10ms
	int phy_reg_poll_time;	// in 10ms
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
	int host_pid;	// pid of host-daemon
#else
    	struct pid *host_pid;
#endif
	int                     poll_timer_up;
	int                     force_power_down;
	struct timer_list reg_poll_timer;
	unsigned long		reg_BMCR_read, reg_BMCR_write;
	unsigned long 		reg_BMSR_read;
	unsigned long		reg_ANAR_read, reg_ANAR_write;
	unsigned long		reg_ANLPAR_read;	
	int						eth_phy_link_status;
	struct	file			*filp;	
	struct buf_ar 		data_out;
	struct buf_ar 		data_in;
	int						rx_cmd_state;
	unsigned long		rx_cmd_time;
	int						rx_cmd_remain_len;
	int						tx_status_state;
	int						tx_status_transmitting_len;
	int						evt_que_head, evt_que_tail;
	struct evt_msg	ind_evt_que[EV_QUE_MAX];
	spinlock_t      reglock;
};
#endif

#endif // INCLUDE_RTL_MDIO_H
