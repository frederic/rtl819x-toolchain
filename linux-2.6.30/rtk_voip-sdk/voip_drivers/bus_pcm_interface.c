/*
 *	Realtek RTL8186 PCM Controller Driver
 *
 *	Author : thlin@realtek.com.tw
 *
 *	2005.09.05	
 *
 *	Copyright 2005 Realtek Semiconductor Corp.
 */
#include <linux/config.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/errno.h>	
#include <linux/types.h>	
#include <linux/delay.h>  	
#include <linux/interrupt.h>	
#include <linux/ioctl.h>
#include <linux/version.h>
#include <asm/system.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#define SUPPRESSION_PCM_INT_ORDER	2

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
#define RTL865X_REG_BASE (0xBD010000)
#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM8186
#include <asm/rtl8186.h>
#define EXTERNEL_CLK
#define USE_MEM64_OP
#include "gpio/gpio.h"
#elif defined (CONFIG_RTK_VOIP_DRIVERS_PCM8671) || defined (CONFIG_RTK_VOIP_DRIVERS_PCM8672) || defined (CONFIG_RTK_VOIP_DRIVERS_PCM8676)
#include "gpio/gpio.h"
#include "../include/rtl8671.h"
#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM865xC
#include "gpio/gpio.h"

#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY
#include "gpio/gpio.h"

#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM89xxC
//#include "gpio/gpio.h"
#endif

#include "voip_addrspace.h"
#include "voip_types.h"
#include "mem.h"
#include "dmem_stack.h"
#include "rtk_voip.h"

#include "voip_init.h"
#include "voip_proc.h"

#include "radiax_save.h"

#include "cp3_profile.h"

#include "bus_pcm_interface.h"
//#include "spi.h"
//#include "si3210init.h"
//#include "Slic_api.h"
#include "snd_define.h"
//#include "Daa_api.h"
//#ifndef AUDIOCODES_VOIP
//#include "codec_def.h"
//#include "codec_descriptor.h"
//#include "../voip_dsp/include/dtmf_dec.h"

//#include "../voip_dsp/dsp_r1/include/lexra_radiax.h"

//#ifdef FXO_CALLER_ID_DET
//#include "fsk_det.h"
//extern long cid_type[MAX_VOIP_CH_NUM];
//#endif
//#include "fskcid_gen.h"

//#endif /*AUDIOCODES_VOIP*/

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
#include "voip_support.h"
#include "dsp_main.h"
voip_callback_t toRegister[BUS_PCM_CH_NUM];
#endif

//#ifdef 	CONFIG_RTK_VOIP_LED
//#include "led.h"
//#endif

//#include "../voip_dsp/ivr/ivr.h"


//#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
//#include "./iphone/ipphone_interface.h"
//#endif

#if !defined( BUS_PCM_CH_NUM ) || \
	( BUS_PCM_CH_NUM < 1 ) || ( BUS_PCM_CH_NUM > 8 )
#error "BUS_PCM_CH_NUM value is invalid!!!"
#endif

#include "voip_dev.h"
#include "con_register.h"
#include "con_bus_handler.h"
#include "con_mux.h"


#ifndef SUPPORT_CUT_DSP_PROCESS
extern Word16 nPcmIntPeriod[BUS_PCM_CH_NUM];            // pcm interrupt period (ms), set in INITIAL Cmd
#endif

//#define DISABLE_PCI	// kenny: temporary add. Shall be moved to somewhere, bootcode!?

extern int law;	// pkshih: move to bus_pcm_law.c

#ifdef CONFIG_VOIP_COP3_PROFILE
#include "voip_debug.h"
extern st_CP3_VoIP_param cp3_voip_param;
#endif

#ifdef SUPPORT_VOIP_DBG_COUNTER
extern uint32 gVoipCounterEnable;
extern void PCM_tx_count(uint32 chid);
extern void PCM_rx_count(uint32 chid);
#endif

#include "section_def.h"

#if defined (AUDIOCODES_VOIP)

#ifdef AUDIOCODES_VOTING_MECHANISM
#define AC_PCM_ISR_CTLWRD_RESET_CH(ch) (0xFF000000 >> ((ch)*8))
#define AC_PCM_ISR_WRITE(b) pcm_outl(PCMISR, (b));

#define AC_PCM_IMR_READ() pcm_inl(PCMIMR)
#define AC_PCM_IMR_WRITE(b) pcm_outl(PCMIMR,(b))
#define AC_PCM_IMR_CTLWRD_P0OK(ch) BIT((4*(MAXCH-(ch)-1)+3))
#define AC_PCM_IMR_CTLWRD_P1OK(ch) BIT((4*(MAXCH-(ch)-1)+2))
#define AC_PCM_IMR_CTLWRD_TBUA(ch) BIT((4*(MAXCH-(ch)-1)+1))
#define AC_PCM_IMR_CTLWRD_RBUA(ch) BIT(4*(MAXCH-(ch)-1))

#define AC_PCM_CHCNR_READ() pcm_inl(PCMCHCNR)
#define AC_PCM_CHCNR_WRITE(b) pcm_outl(PCMCHCNR,(b))
#define AC_PCM_CHCNR_CTLWRD_RX_TX(ch) (BIT((8*(MAXCH-(ch)-1)+1)) | BIT(8*(MAXCH-(ch)-1)))
#define AC_PCM_CHCNR_CTLWRD_TX(ch) BIT((8*(MAXCH-(ch)-1)+1))
#define AC_PCM_CHCNR_CTLWRD_RX(ch) BIT(8*(MAXCH-(ch)-1))
#endif

#endif

#ifdef SUPPRESSION_PCM_INT_ORDER
static uint32 IMR_enable = 0;	// each bit indicates channel is enabled 
static uint32 IMR_fold = 0;		// which IMR fold is enabled  
#if SUPPRESSION_PCM_INT_ORDER == 1		// half 
#define IMR_FOLD_BASE		0x55555555UL
#define IMR_FOLD_NUM		2
#define IMR_FOLD_GET( bch )	( bch & 0x01 )
#elif SUPPRESSION_PCM_INT_ORDER == 2	// quarter 
#define IMR_FOLD_BASE		0x11111111UL
#define IMR_FOLD_NUM		4
#define IMR_FOLD_GET( bch )	( bch & 0x03 )
#endif
#endif

// --------------------------------------------------------
// channel mapping architecture 
// --------------------------------------------------------

#include "rtk_voip.h"
#include "voip_init.h"
#include "con_register.h"

static voip_bus_t bus_pcm[ BUS_PCM_CH_NUM ];
static uint32 bus_pcm_wideband_bits = 0;

/************************************************************************/
/**********If you want to split off_hook and on_hook for solar(voip_mgr_netfilter.c),****/
/**********you have to define macro SPLIT_DAA_ON_OFF_HOOK.*******************************/
#define SPLIT_DAA_ON_OFF_HOOK
/*********************************************/
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
#define DEV_NAME	"pcmctrl"
#define PCM_MAJOR	244	//use 0 for assigning major number dynamicly.
int pcmctrl_major = PCM_MAJOR;		//device major number
#endif
static struct pcm_priv pcmctrl_devices[ BUS_PCM_CH_NUM ];
//#define reg_isr
#ifdef reg_isr
static int(*isr_callback)(unsigned int status);
#endif

static struct {
	uint8 tx[ BUFFER_SIZE ] __attribute__((aligned(32)));
	uint8 rx[ BUFFER_SIZE ] __attribute__((aligned(32)));
} pcm_dma[ BUS_PCM_CH_NUM ];

__pcmifdata10 static int txpage[BUS_PCM_CH_NUM];
__pcmifdata09 static int rxpage[BUS_PCM_CH_NUM];
__pcmifdata16 static int tr_cnt[BUS_PCM_CH_NUM];
__pcmifdata17 static char chanEnabled[BUS_PCM_CH_NUM]; 

#if defined (AUDIOCODES_VOIP)
__pcmifdata05 unsigned long _chVoting = 0;
#define AC_PCM_VOTING_CH_ON(ch) _chVoting |= (1<<(ch))
#define AC_PCM_VOTING_CH_OFF(ch) _chVoting &= ~(1<<(ch))
#define AC_PCM_VOTING_GET() _chVoting
#endif
#if BUS_PCM_CH_NUM > 4
__pcmifdata06 static unsigned long ChanTxPage[16] = {CH0P0TOK, CH0P1TOK, CH1P0TOK, CH1P1TOK, CH2P0TOK, CH2P1TOK, CH3P0TOK, CH3P1TOK, CH4P0TOK, CH4P1TOK, CH5P0TOK, CH5P1TOK, CH6P0TOK, CH6P1TOK, CH7P0TOK, CH7P1TOK};
__pcmifdata07 static unsigned long ChanRxPage[16] = {CH0P0ROK, CH0P1ROK, CH1P0ROK, CH1P1ROK, CH2P0ROK, CH2P1ROK, CH3P0ROK, CH3P1ROK, CH4P0ROK, CH4P1ROK, CH5P0ROK, CH5P1ROK, CH6P0ROK, CH6P1ROK, CH7P0ROK, CH7P1ROK};
#else
__pcmifdata06 static unsigned long ChanTxPage[8] = {CH0P0TOK, CH0P1TOK, CH1P0TOK, CH1P1TOK, CH2P0TOK, CH2P1TOK, CH3P0TOK, CH3P1TOK};
__pcmifdata07 static unsigned long ChanRxPage[8] = {CH0P0ROK, CH0P1ROK, CH1P0ROK, CH1P1ROK, CH2P0ROK, CH2P1ROK, CH3P0ROK, CH3P1ROK};
#endif

//__pcmifdata08 uint32 **pRxBuf, **pTxBuf;
__pcmifdata08 uint32 *pRxBuf[ BUS_PCM_CH_NUM ], *pTxBuf[ BUS_PCM_CH_NUM ];


//__pcmifdata18 uint32 rxlayerbuf[ BUS_PCM_CH_NUM ];
//__pcmifdata19 uint32 txlayerbuf[ BUS_PCM_CH_NUM ];


//#include "voip_types.h"
//#include "../voip_dsp/dsp_r1/include/typedef.h"
//#include "voip_control.h"

//uint32 g_txVolumneGain_DAA = 7; // init value: 0dB
//uint32 g_rxVolumneGain_DAA = 7; // init value: 0dB

//#if defined (AUDIOCODES_VOIP)
/*** Play Hold Tone for DAA Tx Path ***/
//unsigned int ToneBuf_DAA[TX_BUF_SIZE/4]={0};
//#else
//extern unsigned int ToneBuf_DAA[];
//#endif

#define PCM_DBG_SUPPRESSION
#define PCM_DBG_PROC	// put T0 R0 in /proc/voip/pcm_dbg 

#ifdef PCM_DBG_SUPPRESSION
	#define DBG_NUM_START	1200
	static int dbg_num = 0;
	static uint32 T[BUS_PCM_CH_NUM] = {0};
	static uint32 R[BUS_PCM_CH_NUM] = {0};
	static uint32 TE[BUS_PCM_CH_NUM] = {0};
	static uint32 RF[BUS_PCM_CH_NUM] = {0};

	static uint32 T_[BUS_PCM_CH_NUM] = {0};
	static uint32 R_[BUS_PCM_CH_NUM] = {0};

#ifdef PCM_DBG_PROC
	static uint32 pcm_dbg_sum = 0;
#endif
#endif

//#ifdef FXO_BUSY_TONE_DET
//#include "voip_types.h"
//#include "voip_control.h"
//#include "tone_det_i.h"
//#endif

#ifdef RTK_VOICE_RECORD
#include "voip_control.h"
char txdatabuf[DATAGETBUFSIZE];
char rxdatabuf[DATAGETBUFSIZE];
//char txdatabuf2[DATAGETBUFSIZE];
char rxdatabuf2[DATAGETBUFSIZE];
TstVoipdataget stVoipdataget[BUS_PCM_CH_NUM] = {[0 ... BUS_PCM_CH_NUM-1]={0,0,0,0,0,0,0,txdatabuf, rxdatabuf, rxdatabuf2}};
#endif //#ifdef RTK_VOICE_RECORD

#ifdef RTK_VOICE_PLAY
char txplaydatabuf[DATAPUTBUFSIZE];
TstVoipdataput stVoipdataput[BUS_PCM_CH_NUM] = {[0 ... BUS_PCM_CH_NUM-1]={0,0,0,txplaydatabuf}};
#endif //#ifdef RTK_VOICE_PLAY

static void pcm_disableChan(unsigned int chid);
static void PCM_init(void);
static void pcm_channel_slot(int pcm_mode);
static void print_pcm(void);

static void bus_pcm_set_format( voip_bus_t *this, uint32 _format );

//========================================================//

static int pcm_set_page_size(unsigned int chid, unsigned int size)  
{
	/* Write the reg PCMBSIZE to set pagesize. */
	
	unsigned int n_size;
	n_size = (size/4 - 1);
#if BUS_PCM_CH_NUM > 4
	if (chid & 4) {/* chid 4~7 */
		pcm_outl(PCMBSIZE47, pcm_inl(PCMBSIZE47) & (~(0xFF << (8 * (MAXCH-(chid&3)-1)))));//set to zero
		pcm_outl(PCMBSIZE47, pcm_inl(PCMBSIZE47)|(n_size << (8 * (MAXCH-(chid&3)-1))));	//set pagesize
	} else
#endif
	{
		pcm_outl(PCMBSIZE, pcm_inl(PCMBSIZE) & (~(0xFF << (8 * (MAXCH-chid-1)))));//set to zero
		pcm_outl(PCMBSIZE, pcm_inl(PCMBSIZE)|(n_size << (8 * (MAXCH-chid-1))));	//set pagesize
	}
	//PDBUG("set channel %d page size = %d\n", chid, size);
	// too many console message will cause R0, T0
	//printk("set channel %d page size = %d\n", chid, size);
	return 0;	
}

#ifndef OPTIMIZATION
static unsigned int pcm_get_page_size(unsigned int chid)
{
	/* Read the reg PCMBSIZE to get pagesize*/
	unsigned int pagesize, n_size;	/* Actual pagesize which can get from "pcm_get_page_size()". 
 					It's different from the PCMPAGE_SIZE define in header file. */
#if BUS_PCM_CH_NUM > 4
	if (chid & 4) {/* chid 4~7 */
		n_size = ((pcm_inl(PCMBSIZE47) >>( 8 * (MAXCH-(chid&3)-1)))) & 0xFF;
	} else
#endif
	{
		n_size = ((pcm_inl(PCMBSIZE) >>( 8 * (MAXCH-chid-1)))) & 0xFF;
	}
	pagesize = 4*(n_size + 1);

//	PDBUG("get channel %d page size = %d\n", chid, pagesize);

	return pagesize;
}
#else
#define pcm_get_page_size(x) (PCM_PERIOD*PCM_PERIOD_10MS_SIZE)
#endif

/* Set Tx, Rx own bit to PCM Controller. */
#if defined (AUDIOCODES_VOIP)
//static inline
#endif
static void pcm_set_tx_own_bit(unsigned int chid, unsigned int pageindex)
{
	//printk("tx:%d\n", pageindex);
#if BUS_PCM_CH_NUM > 4
	if (chid & 4) {/* chid 4~7 */
		pcm_outl(CH47TX_BSA(chid), pcm_inl(CH47TX_BSA(chid))|BIT(pageindex));
	} else
#endif
	{
		pcm_outl(TX_BSA(chid), pcm_inl(TX_BSA(chid))|BIT(pageindex));
	}
	//printk("CH0TXBSA= 0x%x\n", pcm_inl(CH0TXBSA));
	//PDBUG("set chid %d tx own bit %d to HW \n",chid, pageindex );
}

#if defined (AUDIOCODES_VOIP)
//static inline
#endif
static void pcm_set_rx_own_bit(unsigned int chid, unsigned int pageindex)
{
	//printk("rx:%d\n", pageindex);
#if BUS_PCM_CH_NUM > 4
	if (chid & 4) {/* chid 4~7 */
		pcm_outl(CH47RX_BSA(chid), pcm_inl(CH47RX_BSA(chid))|BIT(pageindex));
	} else
#endif
	{
		pcm_outl(RX_BSA(chid), pcm_inl(RX_BSA(chid))|BIT(pageindex));
	}
	//printk("CH0RXBSA= 0x%x\n", pcm_inl(CH0RXBSA));
	//PDBUG("set chid %d rx own bit %d to HW\n",chid, pageindex );
}

#if 0 // test
void pcm_set_tx_own_bit_all(unsigned int chid)
{
	//printk("tx:%d\n", pageindex);
	pcm_outl(TX_BSA(chid), pcm_inl(TX_BSA(chid))|0x3);
	//printk("CH0TXBSA= 0x%x\n", pcm_inl(CH0TXBSA));
	//PDBUG("set chid %d tx own bit %d to HW \n",chid, pageindex );
}

void pcm_set_rx_own_bit_all(unsigned int chid)
{
	//printk("rx:%d\n", pageindex);
	pcm_outl(RX_BSA(chid), pcm_inl(RX_BSA(chid))|0x3);
	//printk("CH0RXBSA= 0x%x\n", pcm_inl(CH0RXBSA));
	//PDBUG("set chid %d rx own bit %d to HW\n",chid, pageindex );
}


// return value, 0: owned by CPU, non-zero: owned by HW
int pcm_get_tx_own_bit(unsigned int chid, unsigned int pageindex)
{
	return  ( pcm_inl(TX_BSA(chid)) & BIT(pageindex));
}

int pcm_get_rx_own_bit(unsigned int chid, unsigned int pageindex)
{
	return  ( pcm_inl(RX_BSA(chid)) & BIT(pageindex));
}

#endif // #if 0
/* clean interrupt pending bits */
#if 0
static void pcm_clean_isr(unsigned int statusval)
{
	pcm_outl(PCMISR, statusval);
	//PDBUG("clean pending status bits.\n");
}

static void pcm_clean_isr47(unsigned int statusval)
{
	pcm_outl(PCMISR47, statusval);
	//PDBUG("clean pending status bits.\n");
}
#endif

/* Get the Tx, Rx base address */
static unsigned int pcm_get_tx_base_addr(unsigned int chid)
{
	unsigned int txbaseaddr;
#if BUS_PCM_CH_NUM > 4
	if (chid & 4) {/* chid 4~7 */
	 	txbaseaddr = (pcm_inl(CH47TX_BSA(chid)) & 0xFFFFFFFC)|0xA0000000; // to virtual non-cached address
	} else
#endif
	{
	 	txbaseaddr = (pcm_inl(TX_BSA(chid)) & 0xFFFFFFFC)|0xA0000000; // to virtual non-cached address
	}
	PDBUG(" get Tx base addresss = 0x%x, ch=%d\n", txbaseaddr, chid);
	return txbaseaddr;
	
}


static unsigned int pcm_get_rx_base_addr(unsigned int chid)
{
	unsigned int rxbaseaddr;
#if BUS_PCM_CH_NUM > 4
	if (chid & 4) {/* chid 4~7 */
	 	rxbaseaddr = (pcm_inl(CH47RX_BSA(chid)) & 0xFFFFFFFC)|0xA0000000; // to virtual non-cached address
	} else
#endif
	{
 		rxbaseaddr = (pcm_inl(RX_BSA(chid)) & 0xFFFFFFFC)|0xA0000000; // to virtual non-cached address
	}
 	PDBUG(" get Rx base addresss = 0x%x, ch=%d\n", rxbaseaddr, chid);
	return rxbaseaddr;
}


static void pcm_enable(void)
{
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651_T	
	/* config LSB of GPIOB as PCM */
	pcm_outl(0xbd01200c, pcm_inl(0xbd01200c) | 0x000f0000UL);
	pcm_outl(0xbd012054, pcm_inl(0xbd012054) & (~0x000f0000UL));
	/*config MII as GPIOD/E*/
	pcm_outl(0xbc803018, pcm_inl(0xbc803018) | 0x30000000UL);
	/* config GPIOD/E */
	pcm_outl(0xbd012070, pcm_inl(0xbd012070) & 0x0000ffffUL);
#else
	/* config GPIO as PCM */
	//REG32(PABCCNR) |= 0x00ff0000UL;		// configure as dedicated peripheral
	pcm_outl(0xbd01200c, pcm_inl(0xbd01200c) | 0x00ff0000UL);
	//REG32(PABCPTCR) &= ~0x00ff0000UL;
	pcm_outl(0xbd012054, pcm_inl(0xbd012054) & (~0x00ff0000UL));
#if 0
	//REG32(0x203c) = 0xa5000000;
	pcm_outl(0xbd01203c, 0xa5000000);/*Watch dog disable*/
#endif	

#endif	//endif CONFIG_RTK_VOIP_DRIVERS_PCM8651_T
#endif	
	pcm_outl(PCMCR, PCM_ENABLE);		
#if CONFIG_RTK_VOIP_DRIVERS_PCM89xxC
	asm volatile ("nop\n\t");// add nop fix test chip cpu 5281 bug, formal chip is ok. advised by yen@rtk
#endif
	pcm_outl(PCMCR, 0);
#if CONFIG_RTK_VOIP_DRIVERS_PCM89xxC
	asm volatile ("nop\n\t");// add nop fix test chip cpu 5281 bug, formal chip is ok. advised by yen@rtk
#endif
	pcm_outl(PCMCR, PCM_ENABLE);		
	PDBUG("Enable PCM interface. PCMCR(%X) = 0x%X\n",PCMCR, pcm_inl(PCMCR));

#if 0//def CONFIG_RTK_VOIP_DRIVERS_IIS
	iis_enable();
#endif /* CONFIG_RTK_VOIP_DRIVERS_IIS */
}

static void pcm_disable(void)
{		
	pcm_outl(PCMCR, 0);
	PDBUG("Disable PCM interface. PCMCR = 0x%X\n", pcm_inl(PCMCR));
}


void pcm_tx_rx_enable( uint32 bch1, uint32 bch2 )
{
	uint32 bits;
	
#if BUS_PCM_CH_NUM > 4
	if (bch1 & 4) {/* chid 4~7 */
		bits =	BIT((8*(MAXCH-(bch1&3)-1)+1)) | BIT(8*(MAXCH-(bch1&3)-1)) |
				BIT((8*(MAXCH-(bch2&3)-1)+1)) | BIT(8*(MAXCH-(bch2&3)-1));
		pcm_outl(PCMCHCNR47, pcm_inl(PCMCHCNR47)|bits);
	} else
#endif
	{
		bits =	BIT((8*(MAXCH-bch1-1)+1)) | BIT(8*(MAXCH-bch1-1)) |
				BIT((8*(MAXCH-bch2-1)+1)) | BIT(8*(MAXCH-bch2-1));
		pcm_outl(PCMCHCNR, pcm_inl(PCMCHCNR)|bits);	
	}
}

#if 0
static void pcm_tx_enable( uint32 bch1, uint32 bch2 )
{
	uint32 bits;
	
#if BUS_PCM_CH_NUM > 4
	if (bch1 & 4) {/* chid 4~7 */
		bits =	BIT((8*(MAXCH-(bch1&3)-1)+1)) |
				BIT((8*(MAXCH-(bch2&3)-1)+1));
		pcm_outl(PCMCHCNR47, pcm_inl(PCMCHCNR47)|bits);
	} else
#endif
	{
		bits =	BIT((8*(MAXCH-bch1-1)+1)) |
				BIT((8*(MAXCH-bch2-1)+1));
		pcm_outl(PCMCHCNR, pcm_inl(PCMCHCNR)|bits);
	}
//	kenny: print induces multiple TOKs/ROKs
//	PDBUG("enable channel %d Tx\n", chid);
}
#endif

static void pcm_tx_disable(unsigned int chid)
{
#if BUS_PCM_CH_NUM > 4
	if (chid & 4) {/* chid 4~7 */
		pcm_outl(PCMCHCNR47, pcm_inl(PCMCHCNR47) & (~ BIT((8*(MAXCH-(chid&3)-1)+1))));
	} else
#endif
	{
		pcm_outl(PCMCHCNR, pcm_inl(PCMCHCNR) & (~ BIT((8*(MAXCH-chid-1)+1))));
	}
	//PDBUG("disable channel %d Tx\n", chid);
}

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
// disable WLAN Analog for lower temperature
void wlan_analog_disable(void)
{
	*(unsigned char *)0xbd400050 = 0xc0;
	*(unsigned char *)0xbd400059 = 0x44;
	*(unsigned int *)0xbd400054 = 0xa00fea59;
	PDBUG("disable wlan analog:(0xbd400054:%X) \n", *(unsigned int *)0xbd400054);
}
#endif

#if 0
static void pcm_rx_enable( uint32 bch1, uint32 bch2 )
{
	uint32 bits;
	
#if BUS_PCM_CH_NUM > 4
	if (bch1 & 4) {/* chid 4~7 */
		bits =	BIT(8*(MAXCH-(bch1&3)-1)) |
				BIT(8*(MAXCH-(bch2&3)-1));
		pcm_outl(PCMCHCNR47, pcm_inl(PCMCHCNR47)|bits);
	} else
#endif
	{
		bits =	BIT(8*(MAXCH-bch1-1)) |
				BIT(8*(MAXCH-bch2-1));
		pcm_outl(PCMCHCNR, pcm_inl(PCMCHCNR)|bits);
	}
//	kenny: print induces multiple TOKs/ROKs
//	PDBUG("enable channel %d Rx\n", chid);
}
#endif

static void pcm_rx_disable(unsigned int chid)
{
#if BUS_PCM_CH_NUM > 4
	if (chid & 4) {/* chid 4~7 */
		pcm_outl(PCMCHCNR47, pcm_inl(PCMCHCNR47) & (~ BIT(8*(MAXCH-(chid&3)-1))));
	} else
#endif
	{
		pcm_outl(PCMCHCNR, pcm_inl(PCMCHCNR) & (~ BIT(8*(MAXCH-chid-1))));
	}
	//PDBUG("disable channel %d Rx\n", chid);
}

static void pcm_isr_reset(unsigned int chid)
{
	//printk("1 PCMISR= 0x%x\n", pcm_inl(PCMISR));	
#if BUS_PCM_CH_NUM > 4
	if (chid & 4) {/* chid 4~7 */
		pcm_outl(PCMISR47, 0xFF000000 >> ((chid&3)*8));
	} else
#endif
	{
		pcm_outl(PCMISR, 0xFF000000 >> (chid*8));	
	}
	//printk("2 PCMISR= 0x%x\n", pcm_inl(PCMISR));	
}

static void pcm_isr_reset_tx_rx(unsigned int chid, int tx, int rx)
{
	uint32 bits = ( tx ? 0xF0000000 : 0 ) | ( rx ? 0x0F000000 : 0 );
	
	//printk("1 PCMISR= 0x%x\n", pcm_inl(PCMISR));	
#if BUS_PCM_CH_NUM > 4
	if (chid & 4) {/* chid 4~7 */
		pcm_outl(PCMISR47, bits >> ((chid&3)*8));
	} else
#endif
	{
		pcm_outl(PCMISR, bits >> (chid*8));	
	}
	//printk("2 PCMISR= 0x%x\n", pcm_inl(PCMISR));	
}

static void pcm_imr_enable(unsigned int chid, unsigned char type)
{
	//PDBUG("enable channel %d IMR\n", chid);
#if !defined(CONFIG_RTK_VOIP_DRIVERS_PCM865xC) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8672) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8676) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxD) 
	switch(type)
	{
		case P0OK:
			pcm_outl(PCMIMR, pcm_inl(PCMIMR)|BIT((4*(MAXCH-chid-1)+3)));
			//PDBUG("enable channel %d IMR P0OK\n", chid);
			break;

		case P1OK:
			pcm_outl(PCMIMR, pcm_inl(PCMIMR)|BIT((4*(MAXCH-chid-1)+2)));
			//PDBUG("enable channel %d IMR P1OK\n", chid);
			break;

		case TBUA:
			pcm_outl(PCMIMR, pcm_inl(PCMIMR)|BIT((4*(MAXCH-chid-1)+1)));
			//PDBUG("enable channel %d IMR TUUA\n", chid);
			break;

		case RBUA:
			pcm_outl(PCMIMR, pcm_inl(PCMIMR)|BIT(4*(MAXCH-chid-1)));
			//PDBUG("enable channel %d IMR RBUA\n", chid);
			break;

		default:
			printk("enable channel %d IMR type error!\n", chid);
			break;
			
	}
#else //for 89xxC, 8972B, 8651C and 8652 used
	switch(type)
	{
		case P0OK:
	#if BUS_PCM_CH_NUM > 4
			if (chid & 4) {/* chid 4~7 */
				pcm_outl(PCMIMR47, pcm_inl(PCMIMR47)|(0x05<<(31-8*(chid&3)-2)));
			} else
	#endif
			{
				pcm_outl(PCMIMR, pcm_inl(PCMIMR)|(0x05<<(31-8*chid-2)));
			}
			//PDBUG("enable channel %d IMR P0OK\n", chid);
			break;

		case P1OK:
	#if BUS_PCM_CH_NUM > 4
			if (chid & 4) {/* chid 4~7 */
				pcm_outl(PCMIMR47, pcm_inl(PCMIMR47)|(0x05<<(31-8*(chid&3)-3)));
			} else
	#endif
			{
				pcm_outl(PCMIMR, pcm_inl(PCMIMR)|(0x05<<(31-8*chid-3)));
			}
			//PDBUG("enable channel %d IMR P1OK\n", chid);
			break;

		case TBUA:
	#if BUS_PCM_CH_NUM > 4
			if (chid & 4) {/* chid 4~7 */
				pcm_outl(PCMIMR47, pcm_inl(PCMIMR47)|(0x05<<(31-8*(chid&3)-6)));
			} else
	#endif
			{
				pcm_outl(PCMIMR, pcm_inl(PCMIMR)|(0x05<<(31-8*chid-6)));
			}
			//PDBUG("enable channel %d IMR TUUA\n", chid);
			break;
	
		case RBUA:
	#if BUS_PCM_CH_NUM > 4
			if (chid & 4) {/* chid 4~7 */
				pcm_outl(PCMIMR47, pcm_inl(PCMIMR47)|(0x05<<(31-8*(chid&3)-7)));
			} else
	#endif
			{
				pcm_outl(PCMIMR, pcm_inl(PCMIMR)|(0x05<<(31-8*chid-7)));
			}
			//PDBUG("enable channel %d IMR RBUA\n", chid);
			break;

		default:
			printk("enable channel %d IMR type error!\n", chid);
			break;
			
	}
	
#endif	
//	print_pcm();
	
}

static void pcm_imr_disable(unsigned int chid, unsigned char type)
{
	//PDBUG("disable channel %d IMR\n", chid);
#if !defined(CONFIG_RTK_VOIP_DRIVERS_PCM865xC) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8672) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8676) &&  !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxD)
	switch(type)
	{
		case P0OK:
			pcm_outl(PCMIMR, pcm_inl(PCMIMR) & (~ BIT((4*(MAXCH-chid-1)+3))));
			//PDBUG("disable channel %d IMR P0OK\n", chid);
			break;

		case P1OK:
			pcm_outl(PCMIMR, pcm_inl(PCMIMR) & (~ BIT((4*(MAXCH-chid-1)+2))));
			//PDBUG("disable channel %d IMR P0OK\n", chid);
			break;

		case TBUA:
			pcm_outl(PCMIMR, pcm_inl(PCMIMR) & (~ BIT((4*(MAXCH-chid-1)+1))));
			//PDBUG("disable channel %d IMR P0OK\n", chid);
			break;

		case RBUA:
			pcm_outl(PCMIMR, pcm_inl(PCMIMR) & (~ BIT(4*(MAXCH-chid-1))));
			//PDBUG("disable channel %d IMR P0OK\n", chid);
			break;

		default:
			printk("disable channel %d IMR type error!\n", chid);
			break;
			
	}
#else
	switch(type)
	{
		case P0OK:
	#if BUS_PCM_CH_NUM > 4
			if (chid & 4) {/* chid 4~7 */
				pcm_outl(PCMIMR47, pcm_inl(PCMIMR47)&(~(0x05<<(31-8*(chid&3)-2))));
			} else
	#endif
			{
				pcm_outl(PCMIMR, pcm_inl(PCMIMR)&(~(0x05<<(31-8*chid-2))));
			}
			//PDBUG("enable channel %d IMR P0OK\n", chid);
			break;

		case P1OK:
	#if BUS_PCM_CH_NUM > 4
			if (chid & 4) {/* chid 4~7 */
				pcm_outl(PCMIMR47, pcm_inl(PCMIMR47)&(~(0x05<<(31-8*(chid&3)-3))));
			} else
	#endif
			{
				pcm_outl(PCMIMR, pcm_inl(PCMIMR)&(~(0x05<<(31-8*chid-3))));
			}
			//PDBUG("enable channel %d IMR P1OK\n", chid);
			break;

		case TBUA:
	#if BUS_PCM_CH_NUM > 4
			if (chid & 4) {/* chid 4~7 */
				pcm_outl(PCMIMR47, pcm_inl(PCMIMR47)&(~(0x05<<(31-8*(chid&3)-6))));
			} else
	#endif
			{
				pcm_outl(PCMIMR, pcm_inl(PCMIMR)&(~(0x05<<(31-8*chid-6))));
			}
			//PDBUG("enable channel %d IMR TUUA\n", chid);
			break;

		case RBUA:
	#if BUS_PCM_CH_NUM > 4
			if (chid & 4) {/* chid 4~7 */
				pcm_outl(PCMIMR47, pcm_inl(PCMIMR47)&(~(0x05<<(31-8*(chid&3)-7))));
			} else
	#endif
			{
				pcm_outl(PCMIMR, pcm_inl(PCMIMR)&(~(0x05<<(31-8*chid-7))));
			}
			//PDBUG("enable channel %d IMR RBUA\n", chid);
			break;

		default:
			printk("enable channel %d IMR type error!\n", chid);
			break;
			
	}
#endif
	
}

static void EnaPcmIntr(uint32 chid)
{	
	//pcm_imr_enable(chid, P0OK | P1OK | TBUA | RBUA);
	pcm_isr_reset(chid);

#ifdef SUPPRESSION_PCM_INT_ORDER
	IMR_enable |= ( 1 << chid );
	
	// change fold if need 
	if( !( ( IMR_FOLD_BASE << IMR_fold ) & IMR_enable ) )
		IMR_fold = IMR_FOLD_GET( chid );
	
	// this chid in fold? 
	if( !( ( IMR_FOLD_BASE << IMR_fold ) & ( 1 << chid ) ) )
		return;
#endif

	pcm_imr_enable(chid, P0OK);
	pcm_imr_enable(chid, P1OK );
	pcm_imr_enable(chid, TBUA );
	pcm_imr_enable(chid, RBUA );
}

static void DisPcmIntr(uint32 chid)
{
#ifdef SUPPRESSION_PCM_INT_ORDER
	int i;

	IMR_enable &= ~( 1 << chid );
	
	// change fold if need
	if( !( ( IMR_FOLD_BASE << IMR_fold ) & IMR_enable ) )
	{
		for( i = 0; i < IMR_FOLD_NUM; i ++ )
			if( ( IMR_FOLD_BASE << i ) & IMR_enable )
				break;
		
		if( i == IMR_FOLD_NUM )
			goto label_half_pcm_int_done;
		else
			IMR_fold = i;
		
		for( i = 0; i < BUS_PCM_CH_NUM; i ++ )
			if( ( ( IMR_FOLD_BASE << IMR_fold ) & IMR_enable & ( 1 << i ) ) )
			{
				pcm_imr_enable(i, P0OK);
				pcm_imr_enable(i, P1OK );
				pcm_imr_enable(i, TBUA );
				pcm_imr_enable(i, RBUA );
			}
	}
	
label_half_pcm_int_done:
	;
#endif

	//pcm_imr_disable(chid, P0OK | P1OK | TBUA | RBUA);
	pcm_isr_reset(chid);

	pcm_imr_disable(chid, P0OK );
	pcm_imr_disable(chid, P1OK );
	pcm_imr_disable(chid, TBUA );
	pcm_imr_disable(chid, RBUA );
}

static void init_var(void)
{
	int i;

	for(i=0; i<BUS_PCM_CH_NUM; i++)
	{
		chanEnabled[i] = FALSE;
		
		//FXS_FXO_functionality_init(i);

//#ifdef SUPPORT_PCM_FIFO
//		PCM_FIFO_Init(i);
//#endif	

		//rxlayerbuf[i] = (uint32)((uint32)pcm_get_rx_base_addr(i));  // Get virtual non-cached address
		//txlayerbuf[i] = (uint32)((uint32)pcm_get_tx_base_addr(i));  // Get virtual non-cached address
		pRxBuf[i] = (uint32 *)pcm_get_rx_base_addr(i);
		pTxBuf[i] = (uint32 *)pcm_get_tx_base_addr(i);

		txpage[i] = 0;
		rxpage[i] = 0;
		tr_cnt[i] = 0;

		//InitializeIVR( i );
		

		//printk("----> sync_sample_offset_daa[%d] = %d\n", i, sync_sample_offset_daa[i]);
		
		//if (pcm_ch_for_DAA[i] == 1)
		//	AES_ON(i);
		
		
		
	}
	

	//pRxBuf = (uint32**)((uint32)&rxlayerbuf[0]);
	//pTxBuf = (uint32**)((uint32)&txlayerbuf[0]);

	for (i=0; i<BUS_PCM_CH_NUM; i++)
	{
		BOOT_MSG("pTxBuf[%d]=%p\n", i, pTxBuf[i]);
		BOOT_MSG("pRxBuf[%d]=%p\n", i, pRxBuf[i]);
	}
	
	//ProfileInit();
}

#if 0	// chmap 
void PCM_recovery(unsigned int chid, unsigned int *pcm_isr, unsigned int *tr_cnt, unsigned int *txpage,  unsigned int *rxpage) {
	//printk(" P_RECOVERY ");
	printk(".");
	*pcm_isr = 0;
	*tr_cnt = 0;
	*txpage = 0;
	*rxpage = 0;
#if 1
	pcm_disableChan(chid);
	PCM_restart(chid);
#else
	unsigned int pcmimr, pcmimr47;
	
	pcmimr = pcm_inl(PCMIMR);
#if BUS_PCM_CH_NUM > 4
	pcmimr47 = pcm_inl(PCMIMR47);
#endif
#if BUS_PCM_CH_NUM > 4
	if (chid & 4) {/* chid 4~7 */
		pcm_outl(PCMCHCNR47,pcm_inl(PCMCHCNR47)&(~(0x03000000>>((chid&3)*8))));
	} else
#endif
	{
		pcm_outl(PCMCHCNR,pcm_inl(PCMCHCNR)&(~(0x03000000>>(chid*8))));
	}

#if BUS_PCM_CH_NUM > 4
	pcm_outl(PCMIMR47,0x0);
	pcm_outl(PCMIMR,0x0);
	if (chid & 4) {/* chid 4~7 */
		pcm_clean_isr47((0xFF000000>>((chid&3)*8)));
	} else
#endif
	{
		pcm_outl(PCMIMR,0x0);
		pcm_clean_isr((0xFF000000>>(chid*8)));
	}
	pcm_set_tx_own_bit(chid,0);
	pcm_set_tx_own_bit(chid,1);
	pcm_set_rx_own_bit(chid,0);
	pcm_set_rx_own_bit(chid,1);
#if BUS_PCM_CH_NUM > 4
	if (chid & 4) {/* chid 4~7 */
		pcm_outl(PCMCHCNR47,pcm_inl(PCMCHCNR47)|(0x03000000>>((chid&3)*8)));
	} else
#endif
	{
		pcm_outl(PCMCHCNR,pcm_inl(PCMCHCNR)|(0x03000000>>(chid*8)));	
	}
#if BUS_PCM_CH_NUM > 4
	pcm_outl(PCMIMR47,pcmimr47);
#endif
	pcm_outl(PCMIMR,pcmimr);

	return;	
#endif	
}
#endif

#ifdef RTK_VOICE_PLAY
void voice_play_mix(int chid, Word16* poutput)
{
	int temp_writeindex, temp_readindex;


	temp_readindex=stVoipdataput[chid].readindex;
	temp_writeindex=stVoipdataput[chid].writeindex;
	//if (temp_writeindex==(temp_readindex+160)%DATAPUTBUFSIZE) {
	if (temp_writeindex==temp_readindex) {
		printk("VPE");
		memset(poutput, 0, 160);
	} else {
		memcpy(poutput, &stVoipdataput[chid].txbuf[temp_readindex], 160);
		stVoipdataput[chid].readindex=(temp_readindex+160)%DATAPUTBUFSIZE;
	}
}
#endif //ifedf RTK_VOICE_PLAY

#if 0	// chmap: restart will set page own by dsp 
void reset_pcmsp(uint32 chid)
{
	int j;
	j = pcm_get_page_size(chid);
	memset(pRxBuf[chid], 0, j);
	memset(pTxBuf[chid], 0, j);
}
#endif

#ifdef reg_isr
int pcm_isr_register(int(*callback)(unsigned int status_val))
{
	//myreg=(struct my_reg *) kmalloc(sizeof(struct my_reg), GFP_ATOMIC);
	//myreg->isr_callback = (void *)callback;
	isr_callback = (void *)callback;
	return 0;
}
#endif



//#if defined (AUDIOCODES_VOIP)
//#include "AC49xDrv_Config.h"
//#define PCM_10MSLEN_INSHORT 80
//
//typedef volatile struct {
//	unsigned short *RxBuff;
//	unsigned short *TxBuff;
//}Tacmw_transiverBuff;
//
//static Tacmw_transiverBuff transiverBuff[ACMW_MAX_NUM_CH];
//extern int ACMWPcmProcess(const Tacmw_transiverBuff *transiverBuff, const unsigned int maxCh, const unsigned int size);
//#endif





#ifdef SUPPORT_PCM_FIFO
__pcmIsr01  
//#define CHECK_TR_CNT // Let TX and RX in order
#if BUS_PCM_CH_NUM > 4
static int32 pcm_ISR(uint32 pcm_isr03, uint32 pcm_isr47)
#else
static int32 pcm_ISR(uint32 pcm_isr)
#endif
{
	uint32 bch = 0, i, j;
	static uint32 last_bch = 0;
	uint32 tx_isrpage, rx_isrpage;
#if BUS_PCM_CH_NUM > 4
	uint32* pcm_isr = NULL;
#endif

	//bch = (++last_bch) % BUS_PCM_CH_NUM;	
	bch = ( ++last_bch >= BUS_PCM_CH_NUM ? 0 : last_bch );
	
	for (i=0; i < 2; i++) // page0/page1
	{
	    //int need_PCM_RX[ BUS_PCM_CH_NUM ];
#ifndef SUPPORT_SYS_DMEM_STACK
	//static
#endif
	    //__attribute__((aligned(8)))
		//uint32 rxBuf[ BUS_PCM_CH_NUM ][PCM_PERIOD*PCM_PERIOD_10MS_SIZE/sizeof(uint32)];
	    //memset(need_PCM_RX, 0, sizeof(need_PCM_RX));
	    extern void isr_bus_reset_need_rx_vars( void );
    	isr_bus_reset_need_rx_vars();
	    
	    //for (j = 0; j < BUS_PCM_CH_NUM; j++, bch = (bch+1)%BUS_PCM_CH_NUM)
	    for (j = 0; j < BUS_PCM_CH_NUM; j++, bch = ( bch+1 >= BUS_PCM_CH_NUM ? 0 : bch+1 ) )
	    {
	    	voip_bus_t * const p_bus = &bus_pcm[ bch ];
	    	voip_con_t * const p_con = p_bus ->con_ptr;
	    	
#if !defined (AUDIOCODES_VOTING_MECHANISM)
	    	//if (chanEnabled[bch] == FALSE)
			//	continue;		
			if( !p_bus ->enabled )
				continue;
#endif

#if BUS_PCM_CH_NUM > 4
			if (bch < 4)
		    	pcm_isr = &pcm_isr03;
			else
		    	pcm_isr = &pcm_isr47;
#endif
			tx_isrpage = 2*bch + txpage[bch];
			rx_isrpage = 2*bch + rxpage[bch];

#if 0 // debug only
			if ( 	tr_cnt[bch] == 1
				&& (pcm_isr & ChanTxPage[tx_isrpage])
				&& (pcm_isr & ChanRxPage[rx_isrpage]) == 0
			) {
				printk("+");
				int pcm_isr2 = pcm_inl(PCMISR) & ChanRxPage[rx_isrpage]; 
				if (pcm_isr2 != 0) {
					pcm_clean_isr(pcm_isr2);
					pcm_isr |= pcm_isr2;
					printk("-");
				}
			}
#endif			
			
#ifdef CHECK_TR_CNT
	#if BUS_PCM_CH_NUM > 4
			if( ((*pcm_isr) & ChanTxPage[tx_isrpage]) && tr_cnt[bch] == 0 )
	#else
			// check if PCM TX and RX is in order, too.
			if( (pcm_isr & ChanTxPage[tx_isrpage]) && tr_cnt[bch] == 0 )
	#endif
#else
	#if BUS_PCM_CH_NUM > 4
			if( ((*pcm_isr) & ChanTxPage[tx_isrpage]) )
	#else
			if( (pcm_isr & ChanTxPage[tx_isrpage]) )
	#endif
#endif
			{
				voip_snd_t * const p_snd = p_con ->snd_ptr;

#if defined (AUDIOCODES_VOTING_MECHANISM)	
				if (chanEnabled[chid] == TRUE)	{
#endif
					uint32* txbuf = &pTxBuf[bch][txpage[bch]*(pcm_get_page_size(bch)>>2)];
					
					if( p_con ->con_ops ->isr_bus_read_tx( p_con, 
								p_bus, ( uint16 * )txbuf ) == NULL )
					{
						// TX FIFO is empty!
						memset(txbuf, 0, pcm_get_page_size(bch));	
					#ifdef PCM_DBG_SUPPRESSION
						TE[bch]++;
					#else
						printk("TE(%d) ", bch);
					#endif
					} else {

						if( bch == 0 ) {
							ddinst_rw_auto( VOIP_DEV_PCM0_TX, ( char * )txbuf, 160 );
							//printk(".");
						}
						else if( bch == 2 ) {
							ddinst_rw_auto( VOIP_DEV_PCM1_TX, ( char * )txbuf, 160 );
							//printk("x");
						}

#ifdef SUPPORT_VOIP_DBG_COUNTER
						if (gVoipCounterEnable == 1)
							PCM_tx_count(bch);
#endif
					}

#if defined (AUDIOCODES_VOTING_MECHANISM)	
				}	// chanEnabled
#endif
#if 1//defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116) && defined(CONFIG_RTK_VOIP_DRIVERS_DAA_SUPPORT)
				if( p_snd ->snd_flags.b.attenuateTx_6dB ) {
					int jj, tmp, pcm_sample;
					short *tmp_ptr;
					pcm_sample = pcm_get_page_size(bch)>>1;
					tmp_ptr = (short*)txbuf;
					for (jj=0; jj<(pcm_sample); jj++) {
						tmp = (*tmp_ptr)>>1;
						*tmp_ptr++=tmp;
					}
				}
#endif
				pcm_set_tx_own_bit(bch, txpage[bch]);
#if BUS_PCM_CH_NUM > 4
				(*pcm_isr) &= ~ChanTxPage[tx_isrpage];
#else
				pcm_isr &= ~ChanTxPage[tx_isrpage];
#endif
				txpage[bch] ^= 1;
							
				tr_cnt[bch]++;
			} // end of tx
			
			
#if BUS_PCM_CH_NUM > 4
			if( (*pcm_isr) & ChanRxPage[rx_isrpage] ) {
#else
			if( pcm_isr & ChanRxPage[rx_isrpage] ) {
#endif
				// pcm_set_rx_own_bit ASAP helps a lot!
				if( p_con ->con_ops ->isr_bus_write_rx_TH( p_con, p_bus, 
						( uint16 * )&pRxBuf[bch][rxpage[bch]*(pcm_get_page_size(bch)>>2)] )
						== NULL )
				{
					#ifdef PCM_DBG_SUPPRESSION
					RF[ bch ] ++;
					#else
					printk("RF(%d) ", bch);
					#endif
				}
#ifdef SUPPORT_VOIP_DBG_COUNTER
				else
				{
					if (gVoipCounterEnable == 1)
						PCM_rx_count(bch);
				}
#endif
				
				pcm_set_rx_own_bit(bch, rxpage[bch]);

#if BUS_PCM_CH_NUM > 4
				(*pcm_isr) &= ~ChanRxPage[rx_isrpage];
#else
				pcm_isr &= ~ChanRxPage[rx_isrpage];
#endif
				rxpage[bch] ^= 1;
#ifdef CHECK_TR_CNT
				if (tr_cnt[bch] != 1) {
					//printk("tr_cnt[%d]=%d ", bch, tr_cnt[bch]);
					printk("tr%d-%d ", bch, tr_cnt[bch]);
					tr_cnt[bch] = 1;
				}	
#endif				
				tr_cnt[bch]--;
				last_bch = bch;
			}
				
	    } // end of for j

	    // Do PCM_RX()
	    for (j = 0; j < BUS_PCM_CH_NUM; j++) 
	    {	
	    	const voip_bus_t * const p_bus = &bus_pcm[ j ];
	    	voip_con_t * const p_con = p_bus ->con_ptr;
			
			if( !p_bus ->enabled )
				continue;
			
			p_con ->con_ops ->isr_bus_write_rx_BH( p_con );
	    }
	} // end of for i
		
#if 1 
#if BUS_PCM_CH_NUM > 4
	if (pcm_isr03 != 0 || pcm_isr47 != 0) {
		printk(" pcm_isr03 = %X \n", pcm_isr03);
		printk(" pcm_isr47 = %X \n", pcm_isr47);
	}
#else
	if (pcm_isr != 0)
		printk(" pcm_isr = %X ", pcm_isr);
#endif
#endif	

	isr_schedule_bus_handler();
	
	return 0;
}
#else // SUPPORT_PCM_FIFO

#if BUS_PCM_CH_NUM > 4
static int32 pcm_ISR(uint32 pcm_isr03, uint32 pcm_isr47)
#else
static int32 pcm_ISR(uint32 pcm_isr)
#endif
{
	uint32 chid, tx_isrpage, rx_isrpage;
#if BUS_PCM_CH_NUM > 4
	uint32* pcm_isr = NULL;
	static uint32 last_isr03=0, last_isr47=0;
	uint32 orig_isr03 = pcm_isr03, orig_isr47 = pcm_isr47;
#else
	static uint32 last_isr=0;
	uint32 orig_isr = pcm_isr;
#endif


#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651

#if 1
	if( pcm_isr & 0x0F0F0F0F )
	{
		if(pcm_isr & 0x0F000000)
			printk("BU0 ");
		if(pcm_isr & 0x000F0000)
			printk("BU1 ");
	}
#endif
	pcm_isr &= 0xF0F0F0F0;
#endif


	for(chid=0; chid<BUS_PCM_CH_NUM; chid++)
	{
		if (chanEnabled[chid] == FALSE)
			continue;

#if BUS_PCM_CH_NUM > 4
	    	if (chid < 4)
	    		pcm_isr = &pcm_isr03;
		else
	    		pcm_isr = &pcm_isr47;
#endif
			
#if BUS_PCM_CH_NUM > 4
	    while ( (*pcm_isr) & (0xF0000000 >> ((chid&3)*8))) {
#else
	    while ( pcm_isr & (0xF0000000 >> (chid*8))) {
#endif
// Handle Tx First	
		tx_isrpage = 2*chid + txpage[chid];

#if BUS_PCM_CH_NUM > 4
		if( ((*pcm_isr) & ChanTxPage[tx_isrpage]) && (tr_cnt[chid] == 0))
#else
		if( (pcm_isr & ChanTxPage[tx_isrpage]) && (tr_cnt[chid] == 0))
#endif
		{
			if (tr_cnt[chid] > 0) {
#if BUS_PCM_CH_NUM > 4
				printk("TX: tr_cnt[chid]=%d, isr03=%X(%X), isr47=%X(%X),page(%d,%d)\n", tr_cnt[chid], orig_isr03, last_isr03, orig_isr47, last_isr47, txpage[chid], rxpage[chid]);
#else
				printk("TX: tr_cnt[chid]=%d, isr=%X(%X), page(%d,%d)\n", tr_cnt[chid], orig_isr, last_isr, txpage[chid], rxpage[chid]);
#endif
				while(1) ;			
			}
			tr_cnt[chid]++;
			memcpy( &pTxBuf[chid][txpage[chid]*pcm_get_page_size(chid)>>2], &TxBufTmp[chid][0] , pcm_get_page_size(chid) );
			pcm_set_tx_own_bit(chid, txpage[chid]);
#if BUS_PCM_CH_NUM > 4
			(*pcm_isr) &= ~ChanTxPage[tx_isrpage];
#else
			pcm_isr &= ~ChanTxPage[tx_isrpage];
#endif
			txpage[chid] ^= 1;
			//printk(" t%d ", frame_count%10);
			//printk("t");
		} 

		// Check ROK to avoid system down!
		rx_isrpage = 2*chid + rxpage[chid];
#if BUS_PCM_CH_NUM > 4
		if ( ((*pcm_isr) & ChanRxPage[rx_isrpage]) == 0 ) {
#else
		if ( (pcm_isr & ChanRxPage[rx_isrpage]) == 0 ) {
#endif
#if BUS_PCM_CH_NUM > 4
			int pcm_isr2;
			if (chid & 4) {
				pcm_isr2 = pcm_inl(PCMISR47) & ChanRxPage[rx_isrpage]; // ROK only
				pcm_clean_isr47(pcm_isr2);
			} else {
				pcm_isr2  = pcm_inl(PCMISR) & ChanRxPage[rx_isrpage]; // ROK only
				pcm_clean_isr(pcm_isr2);
			}
			pcm_isr |= pcm_isr2;
#else
			int pcm_isr2 = pcm_inl(PCMISR) & ChanRxPage[rx_isrpage]; // ROK only
			pcm_clean_isr(pcm_isr2);
			pcm_isr |= pcm_isr2;
			//printk("R\n");
#endif
		}
				
#if BUS_PCM_CH_NUM > 4
		if( (*pcm_isr) & ChanRxPage[rx_isrpage] ) {
#else
		if( pcm_isr & ChanRxPage[rx_isrpage] ) {	
#endif
			//printk("r");
			if (tr_cnt[chid] == 0) { // wait for TOK: for db-121 un-buffed board only?
				//PCM_recovery(chid, &pcm_isr, &tr_cnt[chid], &txpage[chid], &rxpage[chid]);
				//continue;
				printk(".");
				tr_cnt[chid]++;
			}

			tr_cnt[chid]--;
			
			PCM_RX(chid);
			pcm_set_rx_own_bit(chid, rxpage[chid]);

			//pcm_set_rx_own_bit(chid, rxpage[chid]);
#if BUS_PCM_CH_NUM > 4
			(*pcm_isr) &= ~ChanRxPage[rx_isrpage];
#else
			pcm_isr &= ~ChanRxPage[rx_isrpage];
#endif
			rxpage[chid] ^= 1;
		#ifdef FEATURE_COP3_PCM_HANDLER
			ProfileEnterPoint(PROFILE_INDEX_PCM_HANDLER);
		#endif					
			PCM_handler(chid);
			//printk(" r2%d ", frame_count%10);
		#ifdef FEATURE_COP3_PCM_HANDLER
			ProfileExitPoint(PROFILE_INDEX_PCM_HANDLER);
			ProfilePerDump(PROFILE_INDEX_PCM_HANDLER, cp3_voip_param.cp3_dump_period);
		#endif
			// Check TOK to avoid TU
			tx_isrpage = 2*chid + txpage[chid];
#if BUS_PCM_CH_NUM > 4
			if ( ((*pcm_isr) & ChanTxPage[tx_isrpage]) == 0 ) {
#else
			if ( (pcm_isr & ChanTxPage[tx_isrpage]) == 0 ) {
#endif

#if BUS_PCM_CH_NUM > 4

#endif
				int pcm_isr2;
#if BUS_PCM_CH_NUM > 4
				if (chid & 4)
					pcm_isr2 = pcm_inl(PCMISR47) & ChanTxPage[tx_isrpage]; 
				else
#endif
					pcm_isr2 = pcm_inl(PCMISR) & ChanTxPage[tx_isrpage]; 
				if (pcm_isr2 != 0) {

#if BUS_PCM_CH_NUM > 4
					if (chid & 4)
						pcm_clean_isr47(pcm_isr2);
					else
#endif
						pcm_clean_isr(pcm_isr2);

#if BUS_PCM_CH_NUM > 4
					(*pcm_isr) |= pcm_isr2;
#else
					pcm_isr |= pcm_isr2;
#endif

					tr_cnt[chid]++;

					memcpy( &pTxBuf[chid][txpage[chid]*pcm_get_page_size(chid)>>2], &TxBufTmp[chid][0] , pcm_get_page_size(chid) );

					pcm_set_tx_own_bit(chid, txpage[chid]);
#if BUS_PCM_CH_NUM > 4
					(*pcm_isr) &= ~ChanTxPage[tx_isrpage];
#else
					pcm_isr &= ~ChanTxPage[tx_isrpage];
#endif
					txpage[chid] ^= 1;
					//printk("T");
				}	
				//printk("R\n");
			}

		}

	    } // end of while
	    

	
	
	    
	} // end of for
		
#if BUS_PCM_CH_NUM > 4
	last_isr03 = orig_isr03;
	last_isr47 = orig_isr47;
	if ((*pcm_isr) != 0)
		printk(" %X ", (*pcm_isr));
#else
	last_isr = orig_isr;
	if (pcm_isr != 0)
		printk(" %X ", pcm_isr);
#endif

	return 0;
}
#endif // SUPPORT_PCM_FIFO

#ifdef CONFIG_RTK_VOIP_MODULE
#undef SUPPORT_SYS_DMEM_STACK
#else
#if ! defined (AUDIOCODES_VOIP)
#ifdef PCM_HANDLER_USE_TASKLET
#ifdef VOCODER_INT
#if PCM_PERIOD != 1
#define SUPPORT_SYS_DMEM_STACK
#endif
#else // !VOCODER_INT
#define SUPPORT_SYS_DMEM_STACK
#endif // VOCODER_INT
#endif
#endif
#endif

#ifdef SUPPORT_SYS_DMEM_STACK
#include "../../voip_dsp/dsp_r1/common/util/codec_mem.h"
#endif


#if !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC)&&  !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxD)
#define CHECK_PCM_ISR_AGAIN
#endif

#define CHECK_PCM_ISR_REENTRY
#ifdef CHECK_PCM_ISR_REENTRY
static int in_pcm_isr = 0;
#endif


#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8672
#define PCM_ISR_PERIOD_TEST
#endif

#ifdef PCM_ISR_PERIOD_TEST

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8672
#define TC1_DISABLE	*((volatile unsigned int *)0xb8003110) = 0xC0000000;
#define TC1_ENABLE	*((volatile unsigned int *)0xb8003110) = 0xE0000000;
#define TC1_COUNTER_R	*((volatile unsigned int *)0xb800310C)
#define FIBER_MODE	*((volatile unsigned int *)0xb801805C) = 0x841CC0E0;
#define UTP_MODE	*((volatile unsigned int *)0xb801805C) = 0x841C40E0;
#define FIBER_MODE_R	*((volatile unsigned int *)0xb801805C)
#else
#error "Not implement for this platform"
#endif

#define CNT 6
#define PCM_PAGE_MS	10

#if ( PCM_PAGE_MS == 10 )
#define NORMAL_PCLK_MIN_THRES	455000	//460800
#define ABNORMAL_PCLK_MAX_THRES	450000	//445184
#else
#error "PCLK threshold not define"
#endif


static int pcm_chk_start = 0;

static void pcm_rx_start(int ch)
{
	int i, reg_pbsize;

	pcm_chk_start = 1;
			
	TC1_ENABLE;
	i = TC1_COUNTER_R;

	reg_pbsize = PCM_PAGE_MS * 8 * 2;	// 8 samples/ms * 2 bytes/sample
	pcm_set_page_size( ch, reg_pbsize);

	for(i=0; i<2; i++)
	{
		//pcm_set_tx_own_bit( ch, i);
		pcm_set_rx_own_bit( ch, i);
	}

	pcm_isr_reset(ch);		// Clear Interrupt Status Register
	pcm_imr_enable(ch, P0OK);	// Enable Tx/Rx P0OK Interrupt Mask
	pcm_imr_enable(ch, P1OK );	// Enable Tx/Rx P1OK Interrupt Mask
	//pcm_imr_enable(chid, TBUA );
	pcm_imr_enable(ch, RBUA );	// Enable RBUA Interrupt Mask

	pcm_rx_enable(ch, ch);		// Only Enable PCM RX
}


static void pcm_rx_stop(int ch)
{

	pcm_isr_reset(ch);
	pcm_imr_disable(ch, P0OK );
	pcm_imr_disable(ch, P1OK );
	pcm_imr_disable(ch, TBUA );
	pcm_imr_disable(ch, RBUA );

	pcm_rx_disable(ch);
}

static void pcm_all_stop(void)
{
	// This function disable all PCM channel Tx, Rx, and Interrupt Mask.
	int ch;
	for (ch = 0; ch < 8; ch++)
	{
		pcm_isr_reset(ch);
		pcm_imr_disable(ch, P0OK );
		pcm_imr_disable(ch, P1OK );
		pcm_imr_disable(ch, TBUA );
		pcm_imr_disable(ch, RBUA );

		pcm_rx_disable(ch);
		pcm_rx_disable(ch);
	}
}


static int tc1_array[100] = {0};
static int acc = 0;

static unsigned int check_pcm_period(void)
{
	unsigned int status_val;
	int i;
	unsigned int avg_cnt = 0;
	unsigned int tmp;

	
	while (acc < CNT)
	{
		status_val = pcm_inl(PCMISR);

		while (status_val & 0x30000000)
		{
			pcm_outl(PCMISR, status_val);
			// Record Timer 1 counter
			tc1_array[acc++] = TC1_COUNTER_R;
	
			// Set own bit to PCM ctrl
			if (status_val & 0x20000000)
			{
				pcm_set_rx_own_bit(0, 0);
				//PRINT_Y("0");
			}
			else if (status_val & 0x10000000)
			{
				pcm_set_rx_own_bit(0, 1);
				//PRINT_Y("1");
			}
	
			// Buffer Unavailable
			if(status_val & 0x0F0F0F0F) // Buffer Unavailable only
				PRINT_R("*");

			status_val = 0;
		}
	}

	if (acc == CNT)
	{
		// Dump Result
		for (i=0; i < CNT; i++)
		{
			//printk("[%d]: %d\n", i, tc1_array[i]);
		}

		for (i=0; i < (CNT-1); i++)
		{
			tmp = tc1_array[i+1] - tc1_array[i];
			avg_cnt += tmp;
			//printk("[%d]: %d\n", i, tmp);
		}
		avg_cnt = avg_cnt/(CNT-1);
		
		// Reset
		acc = 0;
		TC1_DISABLE;
		pcm_chk_start = 0;
	}

	return avg_cnt;
}


int PcmClkCheck(void)
{
        unsigned long flags;
        save_flags(flags); cli();
	unsigned int avg_cnt;

	pcm_all_stop();
	pcm_rx_start(0);
	avg_cnt = check_pcm_period();
	pcm_rx_stop(0);

        restore_flags(flags);
	
	printk("Average Counter = %d\n", avg_cnt);

	if (avg_cnt > NORMAL_PCLK_MIN_THRES)
	{
		//printk("PCM CLK Pass!\n");
		return 1;
	}
	else if (avg_cnt < ABNORMAL_PCLK_MAX_THRES)
	{
		//printk("PCM CLK Fail!\n");
		return -1;
	}
	else
	{
		printk("%s: undefine result\n", __FUNCTION__);
		return 0;
	}
}

#endif //PCM_ISR_PERIOD_TEST


__pcmIsr00 
#ifdef CONFIG_DEFAULTS_KERNEL_2_6
static irqreturn_t pcm_interrupt(int32 irq, void *dev_instance)
#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM8672 || defined CONFIG_RTK_VOIP_DRIVERS_PCM8676 
__IRAM static irq_handler_t pcm_interrupt(int32 irq, void *dev_instance)
#else
static void pcm_interrupt(int32 irq, void *dev_instance, struct pt_regs *regs)
#endif

{
#ifndef AUDIOCODES_VOIP
	extern int save_radiax_reg(void);
	extern int load_radiax_reg(void);
#endif
#ifdef SUPPORT_SYS_DMEM_STACK
	extern int dmem_size;
#endif

	unsigned int status_val;
#if BUS_PCM_CH_NUM > 4
	unsigned int status_val47;
#endif

#if ! defined (AUDIOCODES_VOTING_MECHANISM)
	unsigned int maskval;
	int channel=0;
#endif
	
#ifdef CHECK_PCM_ISR_REENTRY
	if (in_pcm_isr == 1) {
		printk("PCM ISR re-entry\n");
		while (1) ;
	}
	in_pcm_isr = 1;	
#endif	

#ifndef AUDIOCODES_VOIP
	save_radiax_reg(); /* save radiax register value */
#else
	volatile struct pt_radiax radiax_regs;
	save_radiax_reg(&radiax_regs); /* save radiax register value */
#endif
	
#ifdef SUPPORT_SYS_DMEM_STACK
	set_DMEM((unsigned long)&__sys_dmem_start, (unsigned long)(dmem_size-1));

	sys_dmem_sp = (unsigned long)&(sys_dmem_stack[SYS_DMEM_SSIZE]);
	entry_dmem_stack(&sys_orig_sp, &sys_dmem_sp);
#endif    

#ifdef FEATURE_COP3_PCMISR
	if (cp3_voip_param.bCp3Count_PCM_ISR == 1)
		ProfileEnterPoint(PROFILE_INDEX_PCMISR);
#endif		
	
	//printk("1 PCMISR= 0x%x\n", pcm_inl(PCMISR));

#ifdef CHECK_PCM_ISR_AGAIN
	//int pcm_isr_cnt = 0;
    #if BUS_PCM_CH_NUM > 4
	while ((status_val = pcm_inl(PCMISR)) || (status_val47 = pcm_inl(PCMISR47)))
    #else
	while((status_val = pcm_inl(PCMISR)))
    #endif
#else
    #if BUS_PCM_CH_NUM > 4
	status_val = pcm_inl(PCMISR);
	status_val47 = pcm_inl(PCMISR47);
	if ((status_val) || (status_val47))
    #else
	if((status_val = pcm_inl(PCMISR)))
    #endif
#endif	
	{
#ifdef CHECK_PCM_ISR_AGAIN
    #if BUS_PCM_CH_NUM > 4
		status_val47 = pcm_inl(PCMISR47);/* status_val47 may not update when PCMISR true */
    #endif
#if 0
		pcm_isr_cnt++;
#endif		
#endif
#if defined (AUDIOCODES_VOTING_MECHANISM)
		AC_PCM_ISR_WRITE(status_val);
#else
    #if BUS_PCM_CH_NUM > 4
		pcm_outl(PCMISR47, status_val47);
    #endif
		pcm_outl(PCMISR, status_val);
#endif
		//if ((status_val& pcm_inl(PCMIMR)) & 0xF0F0F0F0) { // TOK and ROK only
#if BUS_PCM_CH_NUM > 4
		if ( ((status_val) & 0xF0F0F0F0) || ((status_val47) & 0xF0F0F0F0) ) { // TOK and ROK only
			pcm_ISR(status_val & 0xF0F0F0F0, status_val47 & 0xF0F0F0F0);
		}
#else
		if ((status_val) & 0xF0F0F0F0) { // TOK and ROK only
			pcm_ISR(status_val & 0xF0F0F0F0);
		}
#endif
	
			
#if ! defined (AUDIOCODES_VOTING_MECHANISM)
	
#if 1
		//if((maskval = (status_val & ISR_MASK(channel))))
		if((maskval = (status_val & 0x0F0F0F0F))) // Buffer Unavailable only
		{
#if 0		
			if(maskval & ROK_MASK(channel))
				printk("ROK.....");
			if(maskval & TOK_MASK(channel))
				printk("TOK.....");
#endif		

#ifdef PCM_DBG_SUPPRESSION
			for( channel = 0; channel < 4; channel ++ ) {
				if(maskval & TBU_MASK(channel))	{
					T[channel]++;
					T_[channel]++;
				}
				if(maskval & RBU_MASK(channel)) {
					R[channel]++;
					R_[channel]++;
				}
			}
#else		
			channel=0;
			if(maskval & TBU_MASK(channel))	
				printk("T0 ");
			if(maskval & RBU_MASK(channel))
				printk("R0 ");
			channel=1;
			if(maskval & TBU_MASK(channel))	
				printk("T1 ");
			if(maskval & RBU_MASK(channel))
				printk("R1 ");
			channel=2;
			if(maskval & TBU_MASK(channel))	
				printk("T2 ");
			if(maskval & RBU_MASK(channel))
				printk("R2 ");
			channel=3;
			if(maskval & TBU_MASK(channel))	
				printk("T3 ");
			if(maskval & RBU_MASK(channel))
				printk("R3 ");
	
			printk("\n");
#endif	
		}
    #if BUS_PCM_CH_NUM > 4
		if((maskval = (status_val47 & 0x0F0F0F0F))) // Buffer Unavailable only
		{
#if 0
			if(maskval & ROK_MASK(channel))
				printk("ROK.....");
			if(maskval & TOK_MASK(channel))
				printk("TOK.....");
#endif

	#ifdef PCM_DBG_SUPPRESSION
			for( channel = 4; channel < 8; channel ++ ) {
				if(maskval & TBU_MASK(channel))	{
					T[channel]++;
					T_[channel]++;
				}
				if(maskval & RBU_MASK(channel)) {
					R[channel]++;
					R_[channel]++;
				}
			}
	#else		
			channel=4;
			if(maskval & TBU_MASK(channel))
				printk("T4 ");
			if(maskval & RBU_MASK(channel))
				printk("R4 ");
			channel=5;
			if(maskval & TBU_MASK(channel))
				printk("T5 ");
			if(maskval & RBU_MASK(channel))
				printk("R5 ");
			channel=6;
			if(maskval & TBU_MASK(channel))
				printk("T6 ");
			if(maskval & RBU_MASK(channel))
				printk("R6 ");
			channel=7;
			if(maskval & TBU_MASK(channel))
				printk("T7 ");
			if(maskval & RBU_MASK(channel))
				printk("R7 ");

			printk("\n");
	#endif
		}
    #endif
#endif
#endif /*AUDIOCODES_VOTING_MECHANISM*/	

	}
#if 0
//#ifdef PCM_DBG_SUPPRESSION
	dbg_num++;
	int i;
	if(dbg_num == 1200)
	{
		for(i = 0; i <BUS_PCM_CH_NUM; i++)
		{
			if(T[i])
				printk("T%d[%d]", i,T[i]);
			if(R[i])
				printk("R%d[%d]", i,R[i]);
			if(TE[i])
				printk("TE%d[%d]", i,TE[i]);
			if(RF[i])
				printk("RF%d[%d]", i,RF[i]);
			T[i] = R[i] = TE[i] = RF[i] = 0;
		}
		dbg_num = 0;
	}	
#endif
 
#ifdef CHECK_PCM_ISR_AGAIN
#if 0
	if ( pcm_isr_cnt > 1)
		printk(" (%d) ", pcm_isr_cnt);
#endif		
#endif

#ifdef FEATURE_COP3_PCMISR
	if (cp3_voip_param.bCp3Count_PCM_ISR == 1)
		ProfileExitPoint(PROFILE_INDEX_PCMISR);
#endif

#ifdef SUPPORT_SYS_DMEM_STACK
	leave_dmem_stack(&sys_orig_sp);
	sys_orig_sp = 0;
#endif

#ifndef AUDIOCODES_VOIP
	load_radiax_reg(); /* load saved radiax register value */
#else
	restore_radiax_reg(&radiax_regs);
#endif
	
#ifdef FEATURE_COP3_PCMISR
	if (cp3_voip_param.bCp3Count_PCM_ISR == 1)
		ProfilePerDump(PROFILE_INDEX_PCMISR, cp3_voip_param.cp3_dump_period);
#endif	

#ifdef CHECK_PCM_ISR_REENTRY
	in_pcm_isr = 0;	
#endif

#ifdef CONFIG_DEFAULTS_KERNEL_2_6
	return IRQ_HANDLED;
#endif
}

#ifdef PCM_DBG_SUPPRESSION
static void printk_pcm_dbg_in_tasklet( void )
{
	int i;
#ifdef PCM_DBG_PROC
	uint32 sum = 0;
#else
	int type;
#endif
	
	dbg_num++;
	if(dbg_num < DBG_NUM_START)
		return;

#ifdef PCM_DBG_PROC
	dbg_num = 0;
	
	for( i = 0; i < BUS_PCM_CH_NUM; i ++ ) {
		sum += T[ i ];
		sum += R[ i ];
		sum += TE[ i ];
		sum += RF[ i ];
	}
	
	if( pcm_dbg_sum != sum ) {
		pcm_dbg_sum = sum;
		printk( "T0" );
	}
#else		
	i = ( dbg_num - DBG_NUM_START ) >> 2;
	type = ( dbg_num - DBG_NUM_START ) & 0x03;
	
	if( i >= BUS_PCM_CH_NUM ) {
		dbg_num = 0;
		goto label_pcm_dbg_done;
	}
	
	if( T[i] && type == 0 )
	{
		printk("T%d[%d]", i,T[i]);
		T[i] = 0;
	}
	else if( R[i] && type == 1 )
	{
		printk("R%d[%d]", i,R[i]);
		R[i] = 0;
	}
	else if( TE[i] && type == 2 )
	{
		printk("TE%d[%d]", i,TE[i]);
		TE[i] = 0;
	}
	else if( RF[i] && type == 3 )
	{
		printk("RF%d[%d]", i,RF[i]);
		RF[i] = 0;
	}
	
label_pcm_dbg_done:
	;
#endif
}
#endif

#if 1
static uint32 reseting_debouncing = 0;

static inline void reset_pcm_BH_in_timer( void )
{
	uint32 reseting;
	uint32 bch;
	
	if( reseting_debouncing == 0 )
		return;
	
	if( -- reseting_debouncing > 0 )
		return;
	
	reseting_debouncing = 0;	// be more safe! 
	
	for( bch = 0; bch < BUS_PCM_CH_NUM; bch ++ ) {
		voip_bus_t *p_bus = &bus_pcm[ bch ];
		
		if( !p_bus ->reseting )
			continue;
		
		reseting = p_bus ->reseting;
		p_bus ->reseting = 0;
		
		if( p_bus ->enabled != 2 )
			continue;
		
		p_bus ->bus_ops ->reset_BH( p_bus, reseting & 0x02, reseting & 0x01 );
	}	
}

void bus_pcm_checking_timer( void )
{
	if( bus_pcm_wideband_bits )
		reset_pcm_BH_in_timer();
}

static void reset_pcm_in_tasklet( void )
{
	static uint32 reset_wait = 0;
	uint32 bch;
	int count = 0;
	
	if( reset_wait ++ < 1200 )
		goto label_bus_pcm_reset_done;
	
	reset_wait = 0;
	
	// ok. parse every pcm 
	for( bch = 0; bch < BUS_PCM_CH_NUM; bch ++ ) {
		voip_bus_t *p_bus = &bus_pcm[ bch ];
		voip_con_t *p_con;
		uint32 bch2;
		
		if( p_bus ->enabled != 2 )
			continue;
		
		if( T_[ bch ] == 0 && R_[ bch ] == 0 )
			continue;
		
		//printk( "reset:%d,e:%d,%d%d\n", bch, p_bus ->enabled, !!T_[ bch ], !!R_[ bch ] );
		
		// reset pcm ch 
		bch2 = p_bus ->bus_partner_ptr ->bch;
		
		p_con = p_bus ->con_ptr;
	
		p_bus ->bus_ops ->reset_TH( p_bus, T_[ bch ], R_[ bch ] );
		
		p_con ->con_ops ->bus_fifo_flush( p_con, T_[ bch ], R_[ bch ] );
		
#if 1
		// do BH in reset_pcm_BH_in_timer() 
		p_bus ->reseting = 	( T_[ bch ] ? 0x02 : 0 ) |	// assign on first channel only! 
							( R_[ bch ] ? 0x01 : 0 );
#else
		mdelay( 1 );
		
		p_bus ->bus_ops ->reset_BH( p_bus, T_[ bch ], R_[ bch ] );
#endif		
		//printk( "reset:%d,e:%d ok\n", bch, p_bus ->enabled );
		
		// reset counter 
		bch2 = p_bus ->bus_partner_ptr ->bch;
		
		T_[ bch ] = R_[ bch ] = T_[ bch2 ] = R_[ bch2 ] = 0;
		count ++;
	}
	
	// if any 
	if( count ) {
		reseting_debouncing = 2;
		//isr_schedule_bus_handler();
	}
	
label_bus_pcm_reset_done:
	;	
}
#endif 

void bus_pcm_processor_in_tasklet( void )
{
#ifdef PCM_DBG_SUPPRESSION
	printk_pcm_dbg_in_tasklet();
#endif

#if 1
	if( bus_pcm_wideband_bits )
		reset_pcm_in_tasklet();
#endif
}

static void pcmdev_malloc(struct pcm_priv* pcm_dev, unsigned char CH)
{
		/*  BUFFER_SIZE : define the allocated memory size for PCM tx, rx buffer(2 page) */
		/*  pcmdev_malloc() transfer allocated tx, rx buffer to noncache region and map to physical address */
				
		//pcm_dev->tx_allocated = kmalloc(BUFFER_SIZE, GFP_KERNEL);
		pcm_dev->tx_allocated = pcm_dma[ CH ].tx;
		
		if(pcm_dev->tx_allocated)
		{
			//pcm_dev->rx_allocated = kmalloc(BUFFER_SIZE, GFP_KERNEL);
			pcm_dev->rx_allocated = pcm_dma[ CH ].rx;
			
			if(pcm_dev->rx_allocated)
			{
				//assign non-cached address.
				pcm_dev->tx_buffer = (unsigned char *)Virtual2NonCache(pcm_dev->tx_allocated);
				pcm_dev->rx_buffer = (unsigned char *)Virtual2NonCache(pcm_dev->rx_allocated);

				PDBUG("Tx get noncache addr at 0x%X\n", (unsigned int)pcm_dev->tx_buffer);
				PDBUG("Rx get noncache addr at 0x%X\n", (unsigned int)pcm_dev->rx_buffer);

#if BUS_PCM_CH_NUM > 4
				if (CH & 4) // ch 4 ~ 7
				{
					//set TX, Rx buffer
					pcm_outl(CH47TX_BSA(CH), (unsigned int)(Virtual2Physical(pcm_dev->tx_buffer)));	
					pcm_outl(CH47RX_BSA(CH), (unsigned int)(Virtual2Physical(pcm_dev->rx_buffer)));
					PDBUG("Virtula2Physical OK!\n");	

				}
				else

#endif
				{
					//set TX, Rx buffer
					pcm_outl(TX_BSA(CH), (unsigned int)(Virtual2Physical(pcm_dev->tx_buffer)));	
					pcm_outl(RX_BSA(CH), (unsigned int)(Virtual2Physical(pcm_dev->rx_buffer)));
					PDBUG("Virtula2Physical OK!\n");	
				}
				
				//set all allocated buffer to 0
				memset(pcm_dev->tx_buffer, 0, BUFFER_SIZE);
				memset(pcm_dev->rx_buffer, 0, BUFFER_SIZE);
				PDBUG("Set all allocated buffer to 0 OK!\n");
			}
		}
}
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186

static int pcm_fasync_fun(int fd, struct file *filp, int mode)
{
	struct pcm_priv *pcm_dev;
	pcm_dev = &(pcmctrl_devices[0]);
	return fasync_helper(fd, filp, mode, &(pcm_dev->pcm_fasync));
}

struct file_operations pcmctrl_fops = {
	fasync:         pcm_fasync_fun,
};
#endif

int __init pcmctrl_init(void)	
{
#if 0	// enable G.726 ITU test vector verification
	extern int g726_itu_verify(void);
	g726_itu_verify();
#endif
	
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
	int i;
	int ret;
#endif
	int result; 
	unsigned char ch = 0;
	char strbuf[NAME_SIZE];
	struct pcm_priv* pcm_dev;

	printk("\n====== RTK PCM Controller Initialization =======\n");

//extern int SLIC_init(int pcm_mode);

//#if defined (AUDIOCODES_VOIP)
	PCM_init();
//#endif

	//for (chid=0; chid<BUS_PCM_CH_NUM; chid++)
	//{
	//	pcm_set_tx_mute(chid, FALSE);
	//	pcm_set_rx_mute(chid, FALSE);
	//}
	
#ifdef EXTERNEL_CLK
	unsigned long tmp;
	tmp = pcm_inl(0xbd01003c);
	BOOT_MSG("before 0xbd01003c content = %x\n", tmp);
	tmp |= 0x40000;
	pcm_outl(0xbd01003c, tmp);
	tmp = pcm_inl(0xbd01003c);
	BOOT_MSG("after 0xbd01003c content = %x\n", tmp);
	//rtl_outl(0xbd01003c, rtl_inl(0xbd01003c) | 0x40000 );
	
	//tmp = rtl_inl(0xbd01003c);
#endif

#ifdef DISABLE_PCI
	tmp = pcm_inl(0xbd01003c);
	BOOT_MSG("before PCI disabled, 0xbd01003c content = %x\n", tmp);
	tmp |= 0xC0000000;
	pcm_outl(0xbd01003c, tmp);
	tmp = pcm_inl(0xbd01003c);
	BOOT_MSG("after PCI disabled, 0xbd01003c content = %x\n", tmp);
#endif

	//pcmctrl_devices = kmalloc((BUS_PCM_CH_NUM) * sizeof(struct pcm_priv), GFP_KERNEL);
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
	int res = register_chrdev(pcmctrl_major, DEV_NAME, &pcmctrl_fops);

	if(res < 0){
		PERR("Can't register PCM controller devices.");
		return res;	
	}
#endif
	
//========================== Memory Allocated and Request for IRQ =======================//	
	if(1)//pcmctrl_devices)
	{
		memset(pcmctrl_devices, 0, (BUS_PCM_CH_NUM) * sizeof(struct pcm_priv));
		
		pcm_enable();	// pcm must be enable before init SLIC, or it will fail during initialization.

		for (ch = 0; ch <BUS_PCM_CH_NUM; ch++)
		{
			sprintf(strbuf, "pcm%d", ch);
			memcpy(pcmctrl_devices[ch].name, strbuf, NAME_SIZE);
			pcm_dev = &pcmctrl_devices[ch];
			pcmdev_malloc(pcm_dev, ch);


			//====================== Set Compander =======================//
			bus_pcm_set_format( &bus_pcm[ ch ], law );
			
		}
		
		init_var();

#if 0 // chmap
                //=================== SLIC Initialization ==============//
		if (SLIC_init(law) != 0)
		{
			PRINT_R("<<< SLIC Init Fail >>>\n");
			return -1;
		}

#if !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8672) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) && !defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC)
		//channel 1 compander enable
		BOOT_MSG("set channel 1 compander\n");
		pcm_outl(PCMCHCNR, pcm_inl(PCMCHCNR)|BIT((8*(MAXCH-2)+3))); //chiminer modify
		BOOT_MSG("Set U-law, PCMCHCNR = 0x%x \n", pcm_inl(PCMCHCNR)); //chiminer modify
#endif
#endif // chmap 
                //=================== DAA Initialization ==============//
#if 0	// chmap
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
#ifndef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA
#ifndef CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3217x
		for (ch = slic_ch_num; ch < voip_ch_num; ch++)
		{
			if (ch == slic_ch_num)
			{
				init_spi(DAA0_SPI_DEV); //spi init for DAA port 0
			}
			else if (ch == (slic_ch_num+1))
			{
				init_spi(DAA1_SPI_DEV); //spi init for DAA port 1
			}				
			DAA_Init(ch, law);	// DAA initialization
		}
#endif
#endif
#endif
#endif	

//===================IP_phone Initialization======================//
#if 0
#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
		ipphone_init( law );
#endif	
#endif
		result = 0;
	}
	else
	{
		printk("Allocate Memory failed. \n");
		result = -ENOMEM;
	}
	
//******************PCM time slot setup***********//
	pcm_channel_slot(law);  //chiminer modify	
	
	pcm_dev = pcmctrl_devices;
	#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
		result = request_irq(PCM_IRQ, pcm_interrupt, IRQF_DISABLED | IRQF_PERCPU | IRQF_TIMER, 
			pcm_dev->name, pcm_dev);// NULL OK
	#else
		result = request_irq(PCM_IRQ, pcm_interrupt, SA_INTERRUPT, pcm_dev->name, pcm_dev);// NULL OK
	#endif
		if(result)
		{
			printk("Can't request IRQ for channel %d.\n", ch);
			//PERR(KERN_ERR "Can't request IRQ for channel %d.\n", ch);
			return result;
				
		}
		printk("Request IRQ for PCM Controller OK!.\n");

#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_RX)
	for( i = 0 ; i < BUS_PCM_CH_NUM ; i++ )
	{
		toRegister[i].msTick = 10000; /* tick service is triggered every 100ms. */
		toRegister[i].msRemain = 0;
		toRegister[i].chid = i;

		toRegister[i].fpTick  = NULL; /*tick*/
		toRegister[i].fpPktRx = DSP_pktRx; /* packet recv */
		toRegister[i].fpPcmInt= pcm_ISR; /* pcm interrupt */
		//toRegister[i].fpPcmInt = NULL;
		ret = voip_register_callback( i, &toRegister[i] );
		printk("ROME driver register result ch[%d]: %d\n",i,ret);
	}
	printk("Registered into ROME voip_support\n");
#endif
	
	print_pcm();

// For OC 8186 EVB, switch Relay to SLIC
#if defined (CONFIG_RTK_VOIP_DRIVERS_PCM8186V_OC) || defined (CONFIG_RTK_VOIP_DRIVERS_PCM8651_2S_OC)
	Legerity_GPIO_dir_set(0, 1, 1);
	Legerity_GPIO_dir_set(1, 1, 1);
	Legerity_GPIO_data(0, 1, 1);
	Legerity_GPIO_data(1, 1, 1);
#endif

	printk("================= FISISH ===============\n");

#ifdef PCM_ISR_PERIOD_TEST

#if 0	//Let PCM CLK at wrong state
	FIBER_MODE;
	mdelay(50);
	UTP_MODE;
	mdelay(50);
	FIBER_MODE;
	mdelay(50);
	printk("-->0x%x\n", FIBER_MODE_R);
#endif

#if 1	//Check PCM CLK
	int chk_ret;
	chk_ret = PcmClkCheck();
	if (chk_ret == 1)
		PRINT_Y("PCM CLK Pass!\n");
	else if (chk_ret == -1)
	{
		PRINT_R("PCM CLK Fail!\n");

#if 0	//Fix PCM CLK, and check again
		PRINT_G("Recovery it...");
		UTP_MODE;
		mdelay(50);
		FIBER_MODE;
		PRINT_G("Done!\n");
		
		mdelay(50);
		PcmClkCheck();
#endif
	}
	else if (chk_ret == 0)
		PRINT_R("PCM CLK Unknow!\n");
#endif	
#endif	//PCM_ISR_PERIOD_TEST

	return result;
		
}

static void __exit pcmctrl_cleanup(void)
{
	pcm_disable();
	//kfree(pcmctrl_devices);
}

static void print_pcm(void)	// thlin+ for debug
{
	BOOT_MSG("PCMCR= 0x%x\n", pcm_inl(PCMCR));
	BOOT_MSG("PCMCHCNR= 0x%x\n", pcm_inl(PCMCHCNR));
	BOOT_MSG("PCMBSIZE= 0x%x\n", pcm_inl(PCMBSIZE));
	BOOT_MSG("CH0TXBSA= 0x%x\n", pcm_inl(CH0TXBSA));
	BOOT_MSG("CH0RXBSA= 0x%x\n", pcm_inl(CH0RXBSA));
	BOOT_MSG("CH1TXBSA= 0x%x\n", pcm_inl(CH1TXBSA));
	BOOT_MSG("CH1RXBSA= 0x%x\n", pcm_inl(CH1RXBSA));
	BOOT_MSG("CH2TXBSA= 0x%x\n", pcm_inl(CH2TXBSA));
	BOOT_MSG("CH2RXBSA= 0x%x\n", pcm_inl(CH2RXBSA));
	BOOT_MSG("PCMTSR= 0x%x\n", pcm_inl(PCMTSR));
	BOOT_MSG("PCMIMR= 0x%x\n", pcm_inl(PCMIMR));
	BOOT_MSG("PCMISR= 0x%x\n", pcm_inl(PCMISR));	
#if BUS_PCM_CH_NUM > 4
	BOOT_MSG("PCMCHCNR47= 0x%x\n", pcm_inl(PCMCHCNR47));
	BOOT_MSG("PCMBSIZE47= 0x%x\n", pcm_inl(PCMBSIZE47));
	BOOT_MSG("CH4TXBSA= 0x%x\n", pcm_inl(CH4TXBSA));
	BOOT_MSG("CH4RXBSA= 0x%x\n", pcm_inl(CH4RXBSA));
	BOOT_MSG("CH5TXBSA= 0x%x\n", pcm_inl(CH5TXBSA));
	BOOT_MSG("CH5RXBSA= 0x%x\n", pcm_inl(CH5RXBSA));
	BOOT_MSG("CH6TXBSA= 0x%x\n", pcm_inl(CH6TXBSA));
	BOOT_MSG("CH6RXBSA= 0x%x\n", pcm_inl(CH6RXBSA));
	BOOT_MSG("PCMTSR47= 0x%x\n", pcm_inl(PCMTSR47));
	BOOT_MSG("PCMIMR47= 0x%x\n", pcm_inl(PCMIMR47));
	BOOT_MSG("PCMISR47= 0x%x\n", pcm_inl(PCMISR47));
#endif
}



static void PCM_init(void)
{
	unsigned long flags;
	uint32 chid;

#if defined (AUDIOCODES_VOTING_MECHANISM)
	int nPeriod, reg_pbsize, i;
#endif
	
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY
	
	if ( !((*((volatile unsigned int *)0xb800000c))&0x08) )
	{
		*((volatile unsigned int *)0xb8000010) = 
			*((volatile unsigned int *)0xb8000010) | 0x00200000;

		//printk("==> 0x%x\n", *((volatile unsigned int *)0xb8000010));
	}
#endif

	save_flags(flags); cli();

#if defined (CONFIG_RTK_VOIP_DRIVERS_PCM8672) || defined (CONFIG_RTK_VOIP_DRIVERS_PCM8676)
	int val;
	//do we need any settings here?
	//writel(0xb800330C,readl(0xb800330C)|BIT(8));
	val = *(volatile int*)0xb800330C;
	val |=  0x100;
	*(volatile int*)0xb800330C = val;
	//printk("PCM:[%08x]=%08x",0xb800330C, *(volatile int*)0xb800330C );
	val = *(volatile int*)0xb8003300;
	val |=  0x400000;//bit22
	*(volatile int*)0xb8003300 = val;
	//BOOT_MSG("sys config.=%x\n",pcm_inl(0xb9c04000));
#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8671
	/*enable pcm interrupt of GIMR */
	//pcm_outw(0xb9c03010,pcm_inw(0xb9c03010)|0x10);
	//printk("GIMR=%x\n",pcm_inw(0xb9c03010));
	/*enable pcm module of rtl8671 and set gpio for mimic-spi used not JTAG*/
	pcm_outl(0xb9c04000,(pcm_inl(0xb9c04000)&0xffffffef)|0x4000);
	BOOT_MSG("sys config.=%x\n",pcm_inl(0xb9c04000));
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM865xC
	/*set GPIO B[3~0] to pcm*/
	pcm_outl(0xB8003500, (pcm_inl(0xB8003500)|0x0f00));
	pcm_outl(0xB8003504, (pcm_inl(0xB8003504)&0xfffff0ff));
	/* set IO pad driving of PCM to 4mA */
#if 0
	pcm_outl(0xB8003310, (pcm_inl(0xB8003310)|BIT(9)));
	PRINT_MSG("pcm_inl(0xB8003310) = %X\n", pcm_inl(0xB8003310));
#endif
#ifdef CONFIG_RTK_VOIP_GPIO_8962 // for RTL8962 EVB, enable GPIO E F G H
	pcm_outl(0xB800351C,0);
	PRINT_MSG("pcm_inl(0xB800351C) = %X\n", pcm_inl(0xB800351C));
#endif
#endif

#if 0
//#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY
	/* Change driving from 12mA to 8mA */
	pcm_outl(0xB8000034, (pcm_inl(0xB8000034)&0xfffffff0));
	printk("(pcm_inl(0xB8000034) = 0x%x\n", (pcm_inl(0xB8000034)));
#endif
        //init_var();     // can get gloable tx rx base address
        for(chid=0; chid<BUS_PCM_CH_NUM; chid++)
        {
#if ! defined (AUDIOCODES_VOTING_MECHANISM)   
                pcm_rx_disable(chid);
                pcm_tx_disable(chid);
#else
		pcm_outl(PCMCHCNR, pcm_inl(PCMCHCNR) & (~(AC_PCM_CHCNR_CTLWRD_RX(chid))));
		pcm_outl(PCMCHCNR, pcm_inl(PCMCHCNR) & (~(AC_PCM_CHCNR_CTLWRD_TX(chid))));
#endif

        }

#ifndef AUDIOCODES_VOIP
#ifdef SUPPORT_AES_ISR
	Init_AES();
#endif
#endif


        restore_flags(flags);

        //print_pcm();
        //return;
}

static void __exit PCM_shutdown(void)
{
	uint32 chid;

	for (chid=0; chid<BUS_PCM_CH_NUM; chid++)
	{
		pcm_disableChan(chid);
	}
	
	kill_bus_handler();
	
}

#if !defined (AUDIOCODES_VOTING_MECHANISM)
static void pcm_enableChan( uint32 bch1, uint32 bch2 )
{
	int reg_pbsize;
	Word16 nPeriod;
	int		i;
	//voip_bus_t * const p_bus = &bus_pcm[ chid ];
	//voip_con_t * const p_con = p_bus ->con_ptr;

	// chan id from 0 ~ (VOIP_CH_NUM - 1)
	if( bch1 >= BUS_PCM_CH_NUM || bch2 >= BUS_PCM_CH_NUM )
		return;
	if( chanEnabled[ bch1 ] == TRUE && chanEnabled[ bch2 ] == TRUE )
		return;
	
	// only real enable 
	T_[ bch1 ] = T_[ bch2 ] = R_[ bch1 ] = R_[ bch2 ] = 0;
	
#if 0//def CONFIG_RTK_VOIP_DRIVERS_IIS
	if (chid == 0) {
		extern void iis_enableChan(unsigned int chid);
		iis_enableChan(chid);
		return;
	}
#endif
	txpage[ bch1 ] = txpage[ bch2 ] =0;
	rxpage[ bch1 ] = rxpage[ bch2 ] = 0;
	
	// move to dsp_ops ->enable
	//p_con ->con_ops ->handle_event_bus_enable( p_con, 1 /* enable */ ); 	
	
#ifdef PCM_PERIOD
	nPeriod = 10*PCM_PERIOD; // 10ms * PCM_PERIOD
#else	
#ifdef SUPPORT_CUT_DSP_PROCESS
	nPeriod = 10; //10ms
#else
        extern Word16 nPcmIntPeriod[];
	nPeriod = nPcmIntPeriod[chid];
#endif
#endif
	reg_pbsize = nPeriod * 8 * 2;		// 8 samples/ms * 2 bytes/sample
	pcm_set_page_size( bch1, reg_pbsize);
	pcm_set_page_size( bch2, reg_pbsize);
	//pcm_get_page_size(chid);
#if 1
	for(i=0; i<2; i++)
	{
		pcm_set_tx_own_bit( bch1, i);
		pcm_set_rx_own_bit( bch1, i);
		pcm_set_tx_own_bit( bch2, i);
		pcm_set_rx_own_bit( bch2, i);
	}
#endif

#if 1	// clean TX data 
	memset( Virtual2NonCache( pcm_dma[ bch1 ].tx ), 0, BUFFER_SIZE );
	if( bch1 != bch2 )
		memset( Virtual2NonCache( pcm_dma[ bch2 ].tx ), 0, BUFFER_SIZE );
#endif

	chanEnabled[ bch1 ] = chanEnabled[ bch2 ] = TRUE;
	EnaPcmIntr( bch1 );
	EnaPcmIntr( bch2 );
	pcm_tx_rx_enable( bch1, bch2 );
	
	PRINT_MSG("PCM ch%d,%d enable\n", bch1, bch2 );

}
#else //defined (AUDIOCODES_VOTING_MECHANISM)



static void pcm_enableChan(unsigned int chid)
{
	unsigned long ac_isr = 0,ac_imr = 0,ac_chnr=0,j,voting;
	int reg_pbsize;
	Word16 nPeriod;
	int		i;

	// chan id from 0 ~ (VOIP_CH_NUM - 1)
	if(chid >= BUS_PCM_CH_NUM)
		return;

	if(chanEnabled[chid] == TRUE)
		return;

#ifdef SUPPORT_PCM_FIFO
	PCM_FIFO_Init(chid);
#endif	
	

	chanEnabled[chid] = TRUE;
	voting = AC_PCM_VOTING_GET();
	AC_PCM_VOTING_CH_ON(chid);	
	if (voting != 0) return;

	nPeriod = 10*PCM_PERIOD; // 10ms * PCM_PERIOD
	reg_pbsize = nPeriod * 8 * 2;		// 8 samples/ms * 2 bytes/sample
	for(j=0; j<BUS_PCM_CH_NUM; j++)
	{
		pcm_set_page_size(j, reg_pbsize);
		for(i=0; i<2; i++)
		{
			pcm_set_tx_own_bit(j, i);
			pcm_set_rx_own_bit(j, i);
		}
		txpage[j] = 0;
		rxpage[j] = 0;
	}

	
	for(i=0; i<BUS_PCM_CH_NUM; i++) 
	{
		ac_chnr |= AC_PCM_CHCNR_CTLWRD_RX_TX(i);
		ac_imr |=(	AC_PCM_IMR_CTLWRD_P0OK(i)|
				AC_PCM_IMR_CTLWRD_P1OK(i)|
				AC_PCM_IMR_CTLWRD_TBUA(i)|
				AC_PCM_IMR_CTLWRD_RBUA(i));
		ac_isr |=AC_PCM_ISR_CTLWRD_RESET_CH(i);
	}
	//pcm_tx_rx_enable
	AC_PCM_CHCNR_WRITE(AC_PCM_CHCNR_READ()|(ac_chnr));

	//EnaPcmIntr
	AC_PCM_ISR_WRITE(ac_isr);
	AC_PCM_IMR_WRITE(AC_PCM_IMR_READ()|(ac_imr));

}
#endif

#if !defined (AUDIOCODES_VOTING_MECHANISM)
static void pcm_disableChan(unsigned int chid)
{
	if(chanEnabled[chid] == FALSE)
		return;

	// chan id from 0 ~ (VOIP_CH_NUM - 1)
	if(chid >= BUS_PCM_CH_NUM)
		return;

#if 0//def CONFIG_RTK_VOIP_DRIVERS_IIS
	if (chid == 0) {
		extern void iis_disableChan(unsigned int chid);
		iis_disableChan(chid);
		chanEnabled[chid] = FALSE;
		return;
	}
#endif
	
	pcm_rx_disable(chid);
	pcm_tx_disable(chid);

	DisPcmIntr(chid);

	chanEnabled[chid] = FALSE;
	PRINT_MSG("PCM ch%d disable\n", chid);
}
#else
static void pcm_disableChan(unsigned int chid)
{
	unsigned long ac_isr = 0,ac_imr = 0,ac_chnr=0,i,j;

	if (AC_PCM_VOTING_GET() == 0) 
	{
		return; //already all off;
	}

	AC_PCM_VOTING_CH_OFF(chid);

	if (AC_PCM_VOTING_GET() == 0)
	{
		ac_imr = AC_PCM_IMR_READ();
		for(i=0; i<BUS_PCM_CH_NUM; i++) 
		{
			ac_imr |=(	AC_PCM_IMR_CTLWRD_P0OK(i)|
						AC_PCM_IMR_CTLWRD_P1OK(i)|
						AC_PCM_IMR_CTLWRD_TBUA(i)|
						AC_PCM_IMR_CTLWRD_RBUA(i));
			ac_isr |=AC_PCM_ISR_CTLWRD_RESET_CH(i);
		}
		AC_PCM_IMR_WRITE(AC_PCM_IMR_READ()&(~(ac_imr)));
		AC_PCM_ISR_WRITE(ac_isr);
	}

	if (AC_PCM_VOTING_GET() == 0)
	{
		for(i=0; i<BUS_PCM_CH_NUM; i++) ac_chnr |= AC_PCM_CHCNR_CTLWRD_RX(i);
		AC_PCM_CHCNR_WRITE(AC_PCM_CHCNR_READ()&(~(ac_chnr)));			
		ac_chnr = 0;
		for(i=0; i<BUS_PCM_CH_NUM; i++) ac_chnr |= AC_PCM_CHCNR_CTLWRD_TX(i);		
		AC_PCM_CHCNR_WRITE(AC_PCM_CHCNR_READ()&(~(ac_chnr)));			
	}
	chanEnabled[chid] = FALSE;
}
#endif

static void PCM_restart( uint32 bch1, uint32 bch2 )
{
#if ! defined (AUDIOCODES_VOTING_MECHANISM)
	txpage[ bch1 ] = txpage[ bch2 ] = 0;
	rxpage[ bch1 ] = rxpage[ bch2 ] = 0;
#endif	
	tr_cnt[ bch1 ] = tr_cnt[ bch2 ] = 0;
	pcm_enableChan( bch1, bch2 );

}



static void pcm_channel_slot(int pcm_mode)
{
	switch(pcm_mode)
	{
		case BUSDATFMT_PCM_LINEAR:
		case BUSDATFMT_PCM_ALAW:
		case BUSDATFMT_PCM_ULAW:
		case BUSDATFMT_PCM_WIDEBAND_LINEAR:
		case BUSDATFMT_PCM_WIDEBAND_ALAW:
		case BUSDATFMT_PCM_WIDEBAND_ULAW:
			pcm_outl(PCMTSR,0x00020406); //ch0:slot0 ,ch1:slot2 ,ch2:slot4 ,ch3:slot6
#if BUS_PCM_CH_NUM > 4
			pcm_outl(PCMTSR47,0x080a0c0e); //ch4:slot8 ,ch5:slot10 ,ch6:slot12 ,ch7:slot14
#endif
			break;
			
#if 0
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_SILAB
			pcm_outl(PCMTSR,0x00021012); //ch0:slot0 + ch2:slot16, ch1:slot2 + ch3:slot18
#if BUS_PCM_CH_NUM > 4
			pcm_outl(PCMTSR47,0x04061416); //ch4:slot4 + ch6:slot20, ch5:slot6 + ch7:slot22
#endif
#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK
			pcm_outl(PCMTSR,0x00020406); //ch0:slot0 + ch2:slot2, ch1:slot4 + ch3:slot6
#if BUS_PCM_CH_NUM > 4
			pcm_outl(PCMTSR47,0x080a0c0e); //ch4:slot8 + ch6:slot10, ch5:slot12 + ch7:slot14
#endif
#endif
#endif
			break;
		
		default:
			pcm_outl(PCMTSR,0x00020406);
#if BUS_PCM_CH_NUM > 4
			pcm_outl(PCMTSR47,0x080a0c0e);
#endif
			PRINT_R("Error pcm mode in %s, line%d\n", __FUNCTION__, __LINE__);
			break;
	
	}
	

	return;
}	

static int bus_pcm_enable( voip_bus_t *this, int enable )
{
	voip_bus_t * const p_bus_partner = 
		( enable == 2 || enable == 0 ? this ->bus_partner_ptr : NULL );
	const uint32 bch2 = 
		( p_bus_partner ? p_bus_partner ->bch : this ->bch );
	
	// wideband bits mask 
	if( enable == 2 ) {
		bus_pcm_wideband_bits |= 
					( ( 1 << this ->bch  ) | ( 1 << bch2 ) );
	} else if( enable == 0 ) {
		bus_pcm_wideband_bits &= 
					~( ( 1 << this ->bch  ) | ( 1 << bch2 ) );
	}
	
	// assign role 
	this ->role_var = ( enable == 2 ? this ->role_bind : BUS_ROLE_SINGLE );
	//if( p_bus_partner )	// binding decide it! 
	//	p_bus_partner ->role_var = p_bus_partner ->role_bind;
	
	if( enable ) {
		PCM_restart( this ->bch, bch2 );
	} else {
		pcm_disableChan( this ->bch );
		if( p_bus_partner )
			pcm_disableChan( bch2 );
	}
	
	return 0;
}

static int bus_pcm_reset_TH( voip_bus_t *this, int tx, int rx )
{
	// turn off interrupt 
	const uint32 bch1 = this ->bch;
	const uint32 bch2 = ( this ->enabled == 2 ? this ->bus_partner_ptr ->bch : this ->bch );
	int i;
	
	if( rx && tx ) {
		//DisPcmIntr( bch1 );
		//DisPcmIntr( bch2 );
	}
		
	if( rx ) {
		pcm_rx_disable( bch1 );
		pcm_rx_disable( bch2 );
	}
	
	if( tx ) {
		pcm_tx_disable( bch1 );
		pcm_tx_disable( bch2 );
	}
	
	//mdelay( 10 );
	
	// clean PCM_ISR 
	pcm_isr_reset_tx_rx( bch1, tx, rx );
	pcm_isr_reset_tx_rx( bch2, tx, rx );

	// reset software variables (reference pcm_enableChan())
	if( tx )
		txpage[ bch1 ] = txpage[ bch2 ] = 0;
		
	if( rx )
		rxpage[ bch1 ] = rxpage[ bch2 ] = 0;
	
	for(i=0; i<2; i++)
	{
		if( tx ) {
			pcm_set_tx_own_bit( bch1, i);
			pcm_set_tx_own_bit( bch2, i);
		}
		
		if( rx ) {
			pcm_set_rx_own_bit( bch1, i);
			pcm_set_rx_own_bit( bch2, i);
		}
	}
		
	return 0;
}
	
static int bus_pcm_reset_BH( voip_bus_t *this, int tx, int rx )
{
	const uint32 bch1 = this ->bch;
	const uint32 bch2 = ( this ->enabled == 2 ? this ->bus_partner_ptr ->bch : this ->bch );
	
	//int i;
	
	// IMR 
	//EnaPcmIntr( bch1 );
	//EnaPcmIntr( bch2 );
	
	// turn on tx/rx 
	pcm_tx_rx_enable( bch1, bch2 );
	
	return 0;
}

static void bus_pcm_set_timeslot( voip_bus_t *this, uint32 ts1, uint32 ts2 )
{
	// ts2 is valid in next generation 
	const uint32 bch = this ->bch;
	uint32 PCMTSR_tmp, mask, shift;
	uint32 PCMTSR_sel;
	
	this ->TS1_var = ts1;
	this ->TS2_var = ts2;
	
	if( this ->band_mode_bind == BAND_MODE_16K ) {
		// for next generation 
	}
	
	if( bch < 4 ) {
		// 0:[24-28], 1:[16-20], 2:[8-12], 3:[0-4]
		shift = ( ( 3 - bch ) * 8 );
		PCMTSR_sel = PCMTSR;
	} else if( bch < 8 ) {
		// 4:[24-28], 5:[16-20], 6:[8-12], 7:[0-4]
		shift = ( ( 7 - bch ) * 8 );
		PCMTSR_sel = PCMTSR47;
	} else {
		printk( "set timeslot on bch (%d)!?\n", bch );
		return;
	}
	
	mask = 0x1F << shift;
	PCMTSR_tmp = pcm_inl( PCMTSR_sel );
	PCMTSR_tmp = ( PCMTSR_tmp & ~mask ) | ( ( ts1 & 0x1F ) << shift );
	pcm_outl( PCMTSR_sel, PCMTSR_tmp );
}

static void bus_pcm_set_format( voip_bus_t *this, uint32 _format )
{
	const BUS_DATA_FORMAT format = ( BUS_DATA_FORMAT )_format;
	uint32 ch = this ->bch;
	
	switch(format)
	{
		case BUSDATFMT_PCM_LINEAR:// Linear
		case BUSDATFMT_PCM_WIDEBAND_LINEAR:// wideband linear 
		#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8672) || defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) || defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC) || defined(CONFIG_RTK_VOIP_DRIVERS_PCM8676) ||  defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxD)
		pcm_outl(PCMCR, pcm_inl(PCMCR)|BIT(13));
		BOOT_MSG("Set linear mode, PCMCR = 0x%x \n", pcm_inl(PCMCR));
		#else
		pcm_outl(PCMCHCNR,0);
		BOOT_MSG("Set linear mode, PCMCHCNR = 0x%x \n", pcm_inl(PCMCHCNR));
		#endif
		break;
		case BUSDATFMT_PCM_ALAW:// A-law
		case BUSDATFMT_PCM_WIDEBAND_ALAW:// wideband A-law
		#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8672) || defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) || defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC) || defined(CONFIG_RTK_VOIP_DRIVERS_PCM8676) ||  defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxD)
		pcm_outl(PCMCR, pcm_inl(PCMCR)&~BIT(13));
#if BUS_PCM_CH_NUM > 4
		if (ch & 4) {/* chid 4~7 */
			pcm_outl(PCMCHCNR47, pcm_inl(PCMCHCNR47)|BIT((8*(MAXCH-(ch&3)-1)+2)));
			BOOT_MSG("Set A-law, PCMCHCNR47 = 0x%x \n", pcm_inl(PCMCHCNR47));
		} else
#endif
			pcm_outl(PCMCHCNR, pcm_inl(PCMCHCNR)|BIT((8*(MAXCH-ch-1)+2)));
		#else
		pcm_outl(PCMCHCNR, pcm_inl(PCMCHCNR)|BIT((8*(MAXCH-ch-1)+3))|BIT((8*(MAXCH-ch-1)+2)));
		#endif
		BOOT_MSG("Set A-law, PCMCHCNR = 0x%x \n", pcm_inl(PCMCHCNR));
		break;
		
		case BUSDATFMT_PCM_ULAW:// U-law
		case BUSDATFMT_PCM_WIDEBAND_ULAW:// wideband U-law
		#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8672) || defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) || defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC) || defined(CONFIG_RTK_VOIP_DRIVERS_PCM8676) || defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxD)
		pcm_outl(PCMCR, pcm_inl(PCMCR)&~BIT(13));
#if BUS_PCM_CH_NUM > 4
		if (ch & 4) {/* chid 4~7 */
			pcm_outl(PCMCHCNR47, pcm_inl(PCMCHCNR47) & (~BIT((8*(MAXCH-(ch&3)-1)+2)))  );
			BOOT_MSG("Set U-law, PCMCHCNR47 = 0x%x \n", pcm_inl(PCMCHCNR47));
		} else
#endif
			pcm_outl(PCMCHCNR, pcm_inl(PCMCHCNR) & (~BIT((8*(MAXCH-ch-1)+2)))  );
		#else
		pcm_outl(PCMCHCNR, pcm_inl(PCMCHCNR)|BIT((8*(MAXCH-ch-1)+3)));
		#endif
		BOOT_MSG("Set U-law, PCMCHCNR = 0x%x \n", pcm_inl(PCMCHCNR));
		break;
	default:
		BOOT_MSG("Set Unknown law\n");
		break;
	}
	
}

// --------------------------------------------------------
// channel mapping 
// --------------------------------------------------------

#define IMPLEMENT_PCM_OPS_type0( func, rtype )			\
static rtype func##_PCM( voip_snd_t *this )				\
{														\
	return func( this ->sch );							\
}

#define IMPLEMENT_PCM_OPS_type0_void( func )			\
static void func##_PCM( voip_snd_t *this )				\
{														\
	func( this ->sch );									\
}

#define IMPLEMENT_PCM_OPS_type1( func, rtype, v1type )	\
static rtype func##_PCM( voip_snd_t *this, v1type v1 )	\
{														\
	return func( this ->sch, v1 );						\
}

#define IMPLEMENT_PCM_OPS_type1_void( func, v1type )	\
static void func##_PCM( voip_snd_t *this, v1type v1 )	\
{														\
	func( this ->sch, v1 );								\
}

#define IMPLEMENT_PCM_OPS_type2_void( func, v1type, v2type )	\
static void func##_PCM( voip_snd_t *this, v1type v1, v2type v2 )		\
{														\
	func( this ->sch, v1, v2 );							\
}


const bus_ops_t bus_pcm_ops = {
	// common operation 
	.enable = bus_pcm_enable,
	.reset_TH = bus_pcm_reset_TH,
	.reset_BH = bus_pcm_reset_BH,
	
	.set_timeslot = bus_pcm_set_timeslot,
	.set_format = bus_pcm_set_format,
};


static int __init voip_bus_pcm_init( void )
{
	extern int __init pcmctrl_init( void );
	extern const bus_ops_t bus_pcm_ops;
	int i;
	
	for( i = 0; i < BUS_PCM_CH_NUM; i ++ ) {
		bus_pcm[ i ].bch = i;
		bus_pcm[ i ].name = "pcm";
		bus_pcm[ i ].bus_type = BUS_TYPE_PCM;
		bus_pcm[ i ].band_mode_sup = BAND_MODE_8K;
		bus_pcm[ i ].bus_group = ( i < 4 ? BUS_GROUP_PCM03 : BUS_GROUP_PCM47 );
		bus_pcm[ i ].bus_ops = &bus_pcm_ops;
	}
	
	register_voip_bus( &bus_pcm[ 0 ], BUS_PCM_CH_NUM );
	
	pcmctrl_init();
	
	return 0;
}

static void __exit voip_bus_pcm_exit( void )
{
	PCM_shutdown();
	pcmctrl_cleanup();
}


voip_initcall_bus( voip_bus_pcm_init );
voip_exitcall( voip_bus_pcm_exit );

// --------------------------------------------------------
// proc 
// --------------------------------------------------------

static int voip_pcm_regs_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
	int n = 0;
	
	n += sprintf( buf + n, "PCMCR= 0x%08x\n", pcm_inl(PCMCR));
	n += sprintf( buf + n, "PCMCHCNR= 0x%08x\n", pcm_inl(PCMCHCNR));
	n += sprintf( buf + n, "PCMBSIZE= 0x%08x\n", pcm_inl(PCMBSIZE));
	n += sprintf( buf + n, "CH0TXBSA= 0x%08x\n", pcm_inl(CH0TXBSA));
	n += sprintf( buf + n, "CH0RXBSA= 0x%08x\n", pcm_inl(CH0RXBSA));
	n += sprintf( buf + n, "CH1TXBSA= 0x%08x\n", pcm_inl(CH1TXBSA));
	n += sprintf( buf + n, "CH1RXBSA= 0x%08x\n", pcm_inl(CH1RXBSA));
	n += sprintf( buf + n, "CH2TXBSA= 0x%08x\n", pcm_inl(CH2TXBSA));
	n += sprintf( buf + n, "CH2RXBSA= 0x%08x\n", pcm_inl(CH2RXBSA));
	n += sprintf( buf + n, "PCMTSR= 0x%08x\n", pcm_inl(PCMTSR));
	n += sprintf( buf + n, "PCMIMR= 0x%08x (order:%d)\n", pcm_inl(PCMIMR), SUPPRESSION_PCM_INT_ORDER);
	n += sprintf( buf + n, "PCMISR= 0x%08x\n", pcm_inl(PCMISR));	
	n += sprintf( buf + n, "PCMCHCNR47= 0x%08x\n", pcm_inl(PCMCHCNR47));
	n += sprintf( buf + n, "PCMBSIZE47= 0x%08x\n", pcm_inl(PCMBSIZE47));
	n += sprintf( buf + n, "CH4TXBSA= 0x%08x\n", pcm_inl(CH4TXBSA));
	n += sprintf( buf + n, "CH4RXBSA= 0x%08x\n", pcm_inl(CH4RXBSA));
	n += sprintf( buf + n, "CH5TXBSA= 0x%08x\n", pcm_inl(CH5TXBSA));
	n += sprintf( buf + n, "CH5RXBSA= 0x%08x\n", pcm_inl(CH5RXBSA));
	n += sprintf( buf + n, "CH6TXBSA= 0x%08x\n", pcm_inl(CH6TXBSA));
	n += sprintf( buf + n, "CH6RXBSA= 0x%08x\n", pcm_inl(CH6RXBSA));
	n += sprintf( buf + n, "PCMTSR47= 0x%08x\n", pcm_inl(PCMTSR47));
	n += sprintf( buf + n, "PCMIMR47= 0x%08x (order:%d)\n", pcm_inl(PCMIMR47), SUPPRESSION_PCM_INT_ORDER);
	n += sprintf( buf + n, "PCMISR47= 0x%08x\n", pcm_inl(PCMISR47));	
	
	n += sprintf( buf + n, "\n" );
	n += sprintf( buf + n, "wideband_bits=%08X\n", bus_pcm_wideband_bits );
	
	*eof = 1;
	return n;
}

#ifdef PCM_DBG_PROC
static int voip_pcm_dbg_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
	int n = 0;
	int i;
	
	n += sprintf( buf + n, "CH      T[]      R[]     TE[]     RF[]     T_[]     R_[]\n" );
	n += sprintf( buf + n, "-- -------- -------- -------- -------- -------- --------\n" );
	
	for( i = 0; i < BUS_PCM_CH_NUM; i ++ ) {
		n += sprintf( buf + n, "%2d %8X %8X %8X %8X %8X %8X\n",
				i, T[ i ], R[ i ], TE[ i ], RF[ i ], T_[ i ], R_[ i ] );	
	}
	
	n += sprintf( buf + n, "---\nSUM=%X\n", pcm_dbg_sum );
	
	*eof = 1;
	return n;	
}
#endif

static int voip_pcm_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
	int ch;//, ss;
	int n = 0;

	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}
	
	if( IS_CH_PROC_DATA( data ) ) {
		ch = CH_FROM_PROC_DATA( data );
		n = sprintf( buf, "channel=%d\n", ch );
		n += sprintf( buf + n, "enable: %s\n", ( chanEnabled[ ch ] ?
												"yes" : "no" ) );
	} else {
		//ss = SS_FROM_PROC_DATA( data );
		//n = sprintf( buf, "session=%d\n", ss );
	}
	
	*eof = 1;
	return n;
}

static int __init voip_pcm_proc_init( void )
{
	create_proc_read_entry( PROC_VOIP_DIR "/" PROC_VOIP_PCM_DIR "/regs", 0, NULL, voip_pcm_regs_read_proc, NULL );

#ifdef PCM_DBG_PROC
	create_proc_read_entry( PROC_VOIP_DIR "/" PROC_VOIP_PCM_DIR "/dbg", 0, NULL, voip_pcm_dbg_read_proc, NULL );	
#endif
	
	create_voip_channel_proc_read_entry( "pcm", voip_pcm_read_proc );
	
	return 0;
}

static void __exit voip_pcm_proc_exit( void )
{
	remove_voip_proc_entry( PROC_VOIP_DIR "/" PROC_VOIP_PCM_DIR "/regs", NULL );

#ifdef PCM_DBG_PROC
	remove_voip_proc_entry( PROC_VOIP_DIR "/" PROC_VOIP_PCM_DIR "/dbg", NULL );
#endif
	
	remove_voip_channel_proc_entry( "pcm" );
}

voip_initcall_proc( voip_pcm_proc_init );
voip_exitcall( voip_pcm_proc_exit );

