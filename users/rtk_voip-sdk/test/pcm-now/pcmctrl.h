/*
 *  pcmctrl.h - definitions for RTL8186 PCM Controller
 *
 *  Ziv Huang 2005.02.16
 */

#ifndef __RTL_PCMCTRL_H__
#define __RTL_PCMCTRL_H__


#define pcm_outb(address, value)	writeb(value, address)
#define pcm_outw(address, value)	writew(value, address)
#define pcm_outl(address, value)	writel(value, address)

#define pcm_inb(address)		readb(address)
#define pcm_inw(address)		readw(address)
#define pcm_inl(address)		readl(address)


//rtl_outx and rtl_inx base address is 0xbd010000 and pcm register base address is 0xbd280000,
//so we add 0x270000 to each register definition.

#define GIMR		(0xbd010000)	//Global interrupt Mask Register

#define PCMCR		(0xbd280000)	//Interface Control Register
#define PCMCHCNR	(0xbd280004)	//Channel specific Control Register
#define PCMTSR		(0xbd280008)	//Time Slot Assignment Register
#define PCMBSIZE	(0xbd28000C)	//Channels Buffer Size register
#define CH0TXBSA	(0xbd280010)	//Channel 0 TX buffer starting address pointer
#define CH1TXBSA	(0xbd280014)	//Channel 1 TX buffer starting address pointer
#define CH2TXBSA	(0xbd280018)	//Channel 2 TX buffer starting address pointer
#define CH3TXBSA	(0xbd28001C)	//Channel 3 TX buffer starting address pointer
#define CH0RXBSA	(0xbd280020)	//Channel 0 RX buffer starting address pointer
#define CH1RXBSA	(0xbd280024)	//Channel 1 RX buffer starting address pointer
#define CH2RXBSA	(0xbd280028)	//Channel 2 RX buffer starting address pointer
#define CH3RXBSA	(0xbd28002C)	//Channel 3 RX buffer starting address pointer
#define PCMIMR		(0xbd280030)	//Channels Interrupt Mask Register
#define PCMISR		(0xbd280034)	//Channels Interrupt Status Register

#define TX_BSA(channel)	(CH0TXBSA + (4*channel))
#define RX_BSA(channel)	(CH0RXBSA + (4*channel))

/*
#define PCMCR		(0x270000 + 0x00)	//Interface Control Register
#define PCMCHCNR	(0x270000 + 0x04)	//Channel specific Control Register
#define PCMTSR		(0x270000 + 0x08)	//Time Slot Assignment Register
#define PCMBSIZE	(0x270000 + 0x0C)	//Channels Buffer Size register
#define CH0TXBSA	(0x270000 + 0x10)	//Channel 0 TX buffer starting address pointer
#define CH1TXBSA	(0x270000 + 0x14)	//Channel 1 TX buffer starting address pointer
#define CH2TXBSA	(0x270000 + 0x18)	//Channel 2 TX buffer starting address pointer
#define CH3TXBSA	(0x270000 + 0x1C)	//Channel 3 TX buffer starting address pointer
#define CH0RXBSA	(0x270000 + 0x20)	//Channel 0 RX buffer starting address pointer
#define CH1RXBSA	(0x270000 + 0x24)	//Channel 1 RX buffer starting address pointer
#define CH2RXBSA	(0x270000 + 0x28)	//Channel 2 RX buffer starting address pointer
#define CH3RXBSA	(0x270000 + 0x2C)	//Channel 3 RX buffer starting address pointer
#define PCMIMR		(0x270000 + 0x30)	//Channels Interrupt Mask Register
#define PCMISR		(0x270000 + 0x34)	//Channels Interrupt Status Register
*/


//PCMCR
#define PCME	BIT(12)
#define CKDIR	BIT(11)
#define PXDSE	BIT(10)
#define FSINV	BIT(9)


//PCMCHCNR
#define CH3RE	BIT(0)
#define CH3TE	BIT(1)
#define CH3UA	BIT(2)
#define C3CMPE	BIT(3)

#define CH2RE	BIT(8)
#define CH2TE	BIT(9)
#define CH2UA	BIT(10)
#define C2CMPE	BIT(11)

#define CH1RE	BIT(16)
#define CH1TE	BIT(17)
#define CH1UA	BIT(18)
#define C1CMPE	BIT(19)

#define CH0RE	BIT(24)
#define CH0TE	BIT(25)
#define CH0UA	BIT(26)
#define C0CMPE	BIT(27)
#define C0ILBE	BIT(28)

//PCMIMR
#define CH0P0OKIE BIT(15)
#define CH0P1OKIE BIT(14)
#define CH0TBUAIE BIT(13)
#define CH0RBUAIE BIT(12)

#define CH1P0OKIE BIT(11)
#define CH1P1OKIE BIT(10)
#define CH1TBUAIE BIT(9)
#define CH1RBUAIE BIT(8)

#define CH2P0OKIE BIT(7)
#define CH2P1OKIE BIT(6)
#define CH2TBUAIE BIT(5)
#define CH2RBUAIE BIT(4)

#define CH3P0OKIE BIT(3)
#define CH3P1OKIE BIT(2)
#define CH3TBUAIE BIT(1)
#define CH3RBUAIE BIT(0)

//PCMISR
#define CH3P1RBU	BIT(0)
#define CH3P0RBU	BIT(1)
#define CH3P1TBU	BIT(2)
#define CH3P0TBU	BIT(3)
#define CH3P1ROK	BIT(4)
#define CH3P0ROK	BIT(5)
#define CH3P1TOK	BIT(6)
#define CH3P0TOK	BIT(7)

#define CH2P1RBU	BIT(8)
#define CH2P0RBU	BIT(9)
#define CH2P1TBU	BIT(10)
#define CH2P0TBU	BIT(11)
#define CH2P1ROK	BIT(12)
#define CH2P0ROK	BIT(13)
#define CH2P1TOK	BIT(14)
#define CH2P0TOK	BIT(15)

#define CH1P1RBU	BIT(16)
#define CH1P0RBU	BIT(17)
#define CH1P1TBU	BIT(18)
#define CH1P0TBU	BIT(19)
#define CH1P1ROK	BIT(20)
#define CH1P0ROK	BIT(21)
#define CH1P1TOK	BIT(22)
#define CH1P0TOK	BIT(23)

#define CH0P1RBU	BIT(24)
#define CH0P0RBU	BIT(25)
#define CH0P1TBU	BIT(26)
#define CH0P0TBU	BIT(27)
#define CH0P1ROK	BIT(28)
#define CH0P0ROK	BIT(29)
#define CH0P1TOK	BIT(30)
#define CH0P0TOK	BIT(31)

//----------------------------------------------------

//#define BSIZE(channel, size)	((size << 24) >> (8*channel))	
#define BSIZE(channel, size)	( (size & 0xFF) << ((3-channel)*8))

#define P1RBU(channel)	(CH0P1RBU >> (8*channel))
#define P0RBU(channel)	(CH0P0RBU >> (8*channel))
#define P1TBU(channel)	(CH0P1TBU >> (8*channel))
#define P0TBU(channel)	(CH0P0TBU >> (8*channel))
#define P1ROK(channel)	(CH0P1ROK >> (8*channel))
#define P0ROK(channel)	(CH0P0ROK >> (8*channel))
#define P1TOK(channel)	(CH0P1TOK >> (8*channel))
#define P0TOK(channel)	(CH0P0TOK >> (8*channel))

#define ISR_MASK(channel)	(0xFF000000 >> (8*channel))
#define TOK_MASK(channel)	(0xC0000000 >> (8*channel))
#define ROK_MASK(channel)	(0x30000000 >> (8*channel))
#define TBU_MASK(channel)	( 0xC000000 >> (8*channel))
#define RBU_MASK(channel)	( 0x3000000 >> (8*channel))

#define CHxRE(channel)		(CH0RE   >> (8*channel))
#define CHxTE(channel)		(CH0TE   >> (8*channel))
#define CHxUA(channel)		(CH0UA   >> (8*channel))
#define CxCMPE(channel)		(C0CMPE  >> (8*channel))

#define CHxP0OKIE(channel)	(CH0P0OKIE >> (4*channel))
#define CHxP1OKIE(channel)	(CH0P1OKIE >> (4*channel))
#define CHxTBUAIE(channel)	(CH0TBUAIE >> (4*channel))
#define CHxRBUAIE(channel)	(CH0RBUATE >> (4*channel))
/*

#define CH0P0OKIP BIT(15)
#define CH0P1OKIP BIT(14)
#define CH0TBUAIP BIT(13)
#define CH0RBUAIP BIT(12)

#define CH1P0OKIP BIT(11)
#define CH1P1OKIP BIT(10)
#define CH1TBUAIP BIT(9)
#define CH1RBUAIP BIT(8)

#define CH2P0OKIP BIT(7)
#define CH2P1OKIP BIT(6)
#define CH2TBUAIP BIT(5)
#define CH2RBUAIP BIT(4)

#define CH3P0OKIP BIT(3)
#define CH3P1OKIP BIT(2)
#define CH3TBUAIP BIT(1)
#define CH3RBUAIP BIT(0)
*/

#define PCM_ENABLE	(PCME | CKDIR)
#define PCMIP		BIT(7)
#define PCMIE		BIT(7)

#define OWN_MASK	0x03

#define DEV_NAME	"pcmctrl"
#define NAME_SIZE	10

#define CBUF_PAGE_NUM	17	//number of pages of circular buffer
#define PCM_SIZE_N	127
#define PCMPAGE_SIZE	(4*(PCM_SIZE_N + 1))	//one pages, 4(n + 1)
#define BUFFER_SIZE	(PCMPAGE_SIZE*2)	//two pages

#define CIRC_BUF_SIZE	(CUBF_PAGE_NUM * PCMPAGE_SIZE + 1) 

#define PCM_MAJOR	244	//use 0 for assigning major number dynamicly.
#define PCM_DEVS	1	//4 channels, 4 device


#define IMR_ALL(channel)	(0xF000 >> (channel*4))	
#define IMR_OK_ONLY(channel)	(0xC000 >> (channel*4))
#define IMR_TBU(channel)	(0x2000 >> (channel*4))
#define IMR_RBU(channel)	(0x1000 >> (channel*4))



#define BOOL	unsigned char
#define TRUE	1
#define FALSE	0

//---------------------------------------------------------------------------------------------
//	Debug
//---------------------------------------------------------------------------------------------

//#define PCM_DEBUG

#undef PDBUG

#ifdef PCM_DEBUG
	#define PDBUG(fmt, args...) printk("-%s:" fmt, __FUNCTION__, ## args)
#else
	#define PDBUG(fmt, args...)
#endif


#define PERR(fmt, args...)	printk(KERN_ERR "PCM - " fmt, ## args)

#define UCHAR	unsigned char
#define USHORT	unsigned short
#define UINT	unsigned int
#define ULON	unsigned long


#define PCM_IRQ	7


//-----------------------------------------------------------------------
//	codec related definitions.
//-----------------------------------------------------------------------
extern void codec_init(unsigned int mode);

//These definitions must be the same as definitions of codec.c.
#define LINEAR	0
#define U_LAW	1
#define A_LAW	2

//---------------------------------------
//	Operation Mode.
//---------------------------------------

//#define IMR_ALL_ON
#define IMR_OK_ONLY_ON

//#define LINEAR_MODE


//#define LOOPBACK
//#define READ_TEST

//#define USE_TASKLET

//#define CH0_LOOP

#define BLOCK_MODE

//#define TIMER_MODE






struct pcm_priv{
	unsigned int channel;
	unsigned int txpindex;	//tx page index.
	unsigned int rxpindex;	//rx page index
	unsigned int pagepointer;	//used for read. use this to move in the circular buffer page
	char name[NAME_SIZE];
	unsigned char* tx_buffer;
	unsigned char* rx_buffer;
	unsigned char*	tx_allocated;	//buffer for tx
	unsigned char*	rx_allocated;	//buffer for rx
	
	unsigned char* rx_circular;	//rx circular buffer.

//	unsigned char* tx_cb[CBUF_PAGE_NUM];	//tx buffer pointer array
//	unsigned char* rx_cb[CBUF_PAGE_NUM];	//rx buffer pointer array
//	unsigned int tx_W;	//tx write index
//	unsigned int tx_R;	//tx read index
	unsigned int rx_W;	//rx write index
	unsigned int rx_R;	//rx read index
	BOOL rx_overflow;	//rx circular buffer overflow.
	unsigned int CHCNRValue;

	unsigned int mode;	//operation mode - linear, a-law or u-law
	BOOL sw_init;
	
#ifdef BLOCK_MODE
	wait_queue_head_t	wait_q;
#endif

#ifdef USE_TASKLET
	struct tasklet_struct	rx_tasklet;
#endif

#ifdef TIMER_MODE
	struct timer_list timer;
	spinlock_t lock;
#endif

};


int pcmctrl_init_dev_sw(struct pcm_priv* pcm_dev, unsigned int channel);
void pcmctrl_init_dev_hw(struct pcm_priv* pcm_dev, unsigned int channel);
void pcmctrl_init_hardware(unsigned int mode);
void pcm_interrupt(int irq, void *dev_id, struct pt_regs *regs);
void pcm_rx_isr(unsigned long task_priv);

#ifdef TIMER_MODE
#define TIME_INTERVAL	((PCMPAGE_SIZE * HZ * 5)/32000)
void pcmctrl_rx_timer(unsigned long task_priv);
#endif


#endif

