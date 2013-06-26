#ifndef _RTL_NFBI_H_
#define _RTL_NFBI_H_

/*================================================================*/
/* Compiling Flags */
//#define HOST_IS_PANABOARD
//#define SIMULATION            // defined to do simuation
#define MDCIO_GPIO_SIMULATION
#define NFBI_CMD_HANDSHAKE_POLLING_DEFAULT   1 //use polling method for command handshake

/* Constant Definitions */
#define DRIVER_NAME			"rtl_nfbi"
#define DRIVER_VER			"0.2"
#define IO_LEN				0x40
#define DRIVER_MAJOR		14
#define NFBI_BUFSIZE		516 //2 + 2 + 2*255 + 2
#define EV_QUE_MAX			128
#if 1
#define NFBI_POLLING_INTERVAL_DEFAULT           4  //40 ms, in jiffies(10 ms)
//#define CMD_TIME_OUT		            20  // timeout between each cmd word in jiffies
#define NFBI_INTERRUPT_TIMEOUT_DEFAULT		    10  //in jiffies(10 ms)
#define NFBI_RESPONSE_TIMEOUT_DEFAULT           300 //in jiffies(10 ms)
#define NFBI_RETRANSMIT_COUNT_DEFAULT		    2
#else
//#define CMD_TIME_OUT		            60000 // timeout between each cmd word in jiffies
#define NFBI_INTERRUPT_TIMEOUT_DEFAULT		    6000
#define NFBI_RESPONSE_TIMEOUT_DEFAULT           6000
#define NFBI_RETRANSMIT_COUNT_DEFAULT		    0
#endif

#define BIT(x)						(1 << (x))

// RTL8197B NFBI Register offset
#define NFBI_REG_BMCR				0x00	// Basic Mode Control Register
#define NFBI_REG_BMSR				0x01	// Basic Mode Status Register
#define NFBI_REG_PHYID1				0x02	// PHY Identifier Register 1
#define NFBI_REG_PHYID2				0x03	// PHY Identifier Register 2
#define NFBI_REG_ANAR				0x04	// Auto-Negotiation Advertisement Register
#define NFBI_REG_ANLPAR				0x05	// Auto-Negotiation Link Partner Ability Register
#define NFBI_REG_CMD				0x10	// Command Register
#define NFBI_REG_ADDH				0x11	// Address High Register
#define NFBI_REG_ADDL				0x12	// Address Low Register
#define NFBI_REG_DH	    			0x13	// Data High Register
#define NFBI_REG_DL 				0x14	// Dta Low Register
#define NFBI_REG_SCR				0x15	// Send Command Register
#define NFBI_REG_RSR				0x16	// Receive Status Register
#define NFBI_REG_SYSSR				0x17	// System Status Register
#define NFBI_REG_SYSCR				0x18	// System Control Register
#define NFBI_REG_IMR				0x19	// Interrupt Mask Register
#define NFBI_REG_ISR				0x1a	// Interrupt Status Register
#define NFBI_REG_DCH				0x1b	// DRAM Configuration High Register
#define NFBI_REG_DCL				0x1c	// DRAM Configuration Low Register
#define NFBI_REG_DTH				0x1d	// DRAM Timing High Register
#define NFBI_REG_DTL				0x1e	// DRAM Timing Low Register
#define NFBI_REG_RR				    0x1f	// reserved register

// Default value
#define NFBI_REG_PHYID1_DEFAULT		0x001c	// Default value of PHY Identifier Register 1
#define NFBI_REG_PHYID2_DEFAULT		0xcb61	// Default value of PHY Identifier Register 2
#define NFBI_REG_PHYID2_DEFAULT2	0xcb81	// Default value of PHY Identifier Register 2

// bitmask definition for CMD (0x10)
#define BM_CMDTYPE       		BIT(15)	// Command Type
#define BM_BUSY     		    BIT(14)	// Status of NFBI hardware
#define BM_INTLEVEL			    BIT(2)	// Select interrupt level
#define BM_SYSTEMRST			BIT(1)
#define BM_START_RUN_BOOTCODE 	BIT(0)

// bitmask definition for SYSSR (0x17)
#define BM_CHECKSUM_DONE		BIT(15)
#define BM_CHECKSUM_OK		    BIT(14)
#define BM_WLANLINK			    BIT(13)
#define BM_ETHLINK			    BIT(12)
#define BM_ETHPHY_STATUS_CHANGE	BIT(11)
#define BM_ALLSOFTWARE_READY	BIT(10)
#define BM_USBInsertStatus		BIT(7)
#define BM_USBRemoveStatus		BIT(6)
#define BM_BOOTCODE_READY   	BIT(5)

// bitmask definition for SYSCR (0x18)
#define BM_ISOLATION		BIT(15)	// ISOLATION Interrupt Pending
#define BM_ETHMAC			BIT(14)	// Enable/disable Ethernet MAC
#define BM_WLANMAC			BIT(13)	// Enable/disable WLAN MAC
#define BM_ETHPHY			BIT(12)	// Enable/disable Ethernet PHY
#define BM_WLANPHY			BIT(11)	// Enable/disable WLAN PHY
#define BM_SELMIICLK		BIT(10)	//	Select MII Clock Speed
#define BM_RSVD9			BIT(9)		//	Reserved
#define BM_CUSTOM8			BIT(8)		//	Customized used 8
#define BM_CUSTOM7			BIT(7)		//	Customized used 7
#define BM_CUSTOM6			BIT(6)		//	Customized used 6
#define BM_CUSTOM5			BIT(5)		//	Customized used 5
#define BM_CUSTOM4			BIT(4)		//	Customized used 4
#define BM_CUSTOM3			BIT(3)		//	Customized used 3
#define BM_CUSTOM2			BIT(2)		//	Previous msg has been fetched
#define BM_CUSTOM1			BIT(1)		//	New msg has come
#define BM_CUSTOM0			BIT(0)		//	Reserved

// bitmask definition for IMR (0x19)
#define IM_CHECKSUM_DONE		BIT(15)
#define IM_CHECKSUM_OK		    BIT(14)
#define IM_WLANLINK			    BIT(13)
#define IM_ETHLINK			    BIT(12)
#define IM_ETHPHY_STATUS_CHANGE	BIT(11)
#define IM_ALLSOFTWARE_READY	BIT(10)
#define IM_USBInsertStatus		BIT(7)
#define IM_USBRemoveStatus		BIT(6)
#define IM_BOOTCODE_READY   	BIT(5)
#define IM_PREVMSG_FETCH	    BIT(2)
#define IM_NEWMSG_COMING		BIT(1)
#define IM_NEED_BOOTCODE		BIT(0)

// bitmask definition for ISR (0x1a)
#define IP_CHECKSUM_DONE		BIT(15)
#define IP_CHECKSUM_OK		    BIT(14)
#define IP_WLANLINK			    BIT(13)
#define IP_ETHLINK			    BIT(12)
#define IP_ETHPHY_STATUS_CHANGE	BIT(11)
#define IP_ALLSOFTWARE_READY	BIT(10)
#define IP_USBInsertStatus		BIT(7)
#define IP_USBRemoveStatus		BIT(6)
#define IP_BOOTCODE_READY   	BIT(5)
#define IP_PREVMSG_FETCH	    BIT(2)	//	Previous msg has been fetched
#define IP_NEWMSG_COMING		BIT(1)	//	New msg has coming
#define IP_NEED_BOOTCODE		BIT(0)

/*
 * Ioctl definitions
 */
/* Use 'k' as magic number */
#define NFBI_IOC_MAGIC  'k'  /* @Pana_TBD */

struct nfbi_mem32_param
{
    int addr;
    int val;
};

#define NFBI_MAX_BULK_MEM_SIZE 512
struct nfbi_bulk_mem_param
{
    int addr;
    int len;
    char buf[NFBI_MAX_BULK_MEM_SIZE];
};

struct evt_msg {
	int id;			// event id
	int status;
	int value;
};

#define NFBI_IOCTL_PRIV_CMD             _IOW(NFBI_IOC_MAGIC,  0, int) //private command
#define NFBI_IOCTL_REGREAD              _IOWR(NFBI_IOC_MAGIC, 1, int)
#define NFBI_IOCTL_REGWRITE             _IOW(NFBI_IOC_MAGIC,  2, int)
#define NFBI_IOCTL_HCD_PID              _IOW(NFBI_IOC_MAGIC,  3, int) //set host control deamon PID to driver
#define NFBI_IOCTL_GET_EVENT            _IOR(NFBI_IOC_MAGIC,  4, struct evt_msg)
#define NFBI_IOCTL_MEM32_WRITE          _IOW(NFBI_IOC_MAGIC,  5, struct nfbi_mem32_param)
#define NFBI_IOCTL_MEM32_READ           _IOWR(NFBI_IOC_MAGIC, 6, struct nfbi_mem32_param)
#define NFBI_IOCTL_BULK_MEM_WRITE       _IOW(NFBI_IOC_MAGIC,  7, struct nfbi_bulk_mem_param)
#define NFBI_IOCTL_BULK_MEM_READ        _IOWR(NFBI_IOC_MAGIC, 8, struct nfbi_bulk_mem_param)
#define NFBI_IOCTL_TX_CMDWORD_INTERVAL  _IOWR(NFBI_IOC_MAGIC, 9, int)
#define NFBI_IOCTL_INTERRUPT_TIMEOUT    _IOWR(NFBI_IOC_MAGIC, 10, int)
#define NFBI_IOCTL_RETRANSMIT_COUNT     _IOWR(NFBI_IOC_MAGIC, 11, int)
#define NFBI_IOCTL_RESPONSE_TIMEOUT     _IOWR(NFBI_IOC_MAGIC, 12, int)
#define NFBI_IOCTL_MDIO_PHYAD           _IOWR(NFBI_IOC_MAGIC, 13, int)
#define NFBI_IOCTL_CMD_HANDSHAKE_POLLING _IOWR(NFBI_IOC_MAGIC, 14, int)
#define NFBI_IOCTL_EW                   _IOW(NFBI_IOC_MAGIC,  15, struct nfbi_mem32_param)
#define NFBI_IOCTL_DW                   _IOWR(NFBI_IOC_MAGIC, 16, struct nfbi_mem32_param)
#define NFBI_IOCTL_MAXNR 16


//#define DRAM_CONFIG_VAL 0x54480000  //default: 58080000 //32MB DRAM
#define DRAM_CONFIG_VAL 0x52080000  //default: 58080000   //8MB DRAM
//#define DRAM_TIMING_VAL 0xffff05c0  //default: FFFF0FC0
#define DRAM_TIMING_VAL 0x6cea0a80
#define DRAM_CONFIG_VALH ((DRAM_CONFIG_VAL>>16)&0x0000ffff)
#define DRAM_CONFIG_VALL (DRAM_CONFIG_VAL&0x0000ffff)
#define DRAM_TIMING_VALH ((DRAM_TIMING_VAL>>16)&0x0000ffff)
#define DRAM_TIMING_VALL (DRAM_TIMING_VAL&0x0000ffff)

#define NFBI_BOOTADDR 0x007f0000
#define NFBI_KERNADDR 0x00700000

#define CHECK_NFBI_BUSY_BIT

#define NFBI_DEBUG
#undef PDEBUG             /* undef it, just in case */
#ifdef NFBI_DEBUG
#ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
//#define PDEBUG(fmt, args...) printk( KERN_DEBUG DRIVER_NAME": " fmt, ## args)
#define PDEBUG(fmt, args...) printk( KERN_ERR DRIVER_NAME": " fmt, ## args)
#else
     /* This one for user space */
//#define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#define PDEBUG(fmt, args...) printf(fmt, ## args)
#endif
#else
#define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif


// rx cmd id bitmask
#define FIRST_CMD_MASK	BIT(15)

// tx cmd state & rx status state
enum { STATE_TX_INIT, STATE_TX_IN_PROGRESS, STATE_RX_INIT, STATE_RX_WAIT_LEN, STATE_RX_WAIT_DATA, STATE_RX_FINISHED };


#define TIME_DIFF(a, b)		((a >= b)? (a - b):(0xffffffff - b + a + 1))

#ifndef HOST_IS_PANABOARD
#define PUT_IN_DATA(data) { \
		memcpy(&priv->data_in.buf[priv->data_in.len], &data, 2); \
		priv->data_in.len += 2; \
}
#else
#define PUT_IN_DATA(data) { \
		priv->data_in.buf[priv->data_in.len]   = (char)( (data>>8)&0x00ff );    \
		priv->data_in.buf[priv->data_in.len+1] = (char)( data&0x00ff );         \
		priv->data_in.len += 2; \
}
#endif

#define RESET_RX_STATE { \
		priv->state = STATE_RX_INIT; \
		priv->data_in.len = 0; \
		priv->rx_status_time = 0; \
}

#define ASSERT(expr) \
        if(!(expr)) {					\
  			printk( "\033[33;41m%s:%d: assert(%s)\033[m\n",	\
	        __FILE__,__LINE__,#expr);		\
        }
        
/*================================================================*/
/* Structure Definition */
struct buf_ar {
	int len;
	unsigned char buf[NFBI_BUFSIZE];
};

#ifdef __KERNEL__
struct nfbi_priv {
	struct buf_ar 		data_out;
	struct buf_ar 		data_in;
	int					state;
	unsigned long		rx_status_time;
	int					rx_status_remain_len;
	int					tx_cmd_transmitting_len;
	int                 retransmit_count;
	int                 response_timeout; //in jiffies(10 ms)
};

struct nfbi_dev_priv {
    int                 ready;
    int                 hcd_pid;
	struct	file		*filp;
    int					tx_cmdword_interval;	//in jiffies(10 ms), 0~1000
    int                 tx_msg_is_fetched;
    int                 rx_msg_is_coming;
    //statistics
    int                 tx_command_frames;
    int                 tx_done_command_frames;
    int                 tx_retransmit_counts;
    int                 tx_words;
    int                 tx_interupt_timeouts;
    int                 tx_stop_by_signals;
    int                 rx_status_frames;
    int                 rx_words;
    int                 rx_response_timeouts;
    int                 rx_interupt_timeouts;
    int                 rx_stop_by_signals;
    int                 rx_not_1st_word_errors;
    int                 rx_1st_byte_errors;
    int                 rx_cmdid_not_match_errors;
    int                 rx_reset_by_sync_bit_errors;
    int                 rx_checksum_errors;
        
	struct semaphore    sem;            /* mutual exclusion semaphore */
    int                 cmd_handshake_polling;
	wait_queue_head_t   wq;             /* wait queue */
	struct timer_list   timer;
	int                 timer_expired;  /*1-expired*/
	int                 interrupt_timeout; //in jiffies(10 ms), 0~1000
	int					evt_que_head, evt_que_tail;	
	struct evt_msg	    ind_evt_que[EV_QUE_MAX];
#ifdef MDCIO_GPIO_SIMULATION
	struct timer_list   mdc_timer;
#endif
};
#endif

#endif // _RTL_NFBI_H_
