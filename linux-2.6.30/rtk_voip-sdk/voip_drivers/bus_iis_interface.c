/*
 *	Realtek IIS Controller Driver
 *
 *
 *
 *
 *
 *	Copyright 2008 Realtek Semiconductor Corp.
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
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651

#define USE_MEM64_OP

#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM8186

#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM8671

#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM8672

#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM8676

#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM865xC

#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY
#include "gpio/gpio.h"
#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM89xxC
#include "gpio/gpio.h"
#endif

#include "voip_addrspace.h"
#include "voip_types.h"
#include "mem.h"
#include "rtk_voip.h"
#include "voip_proc.h"

#include "radiax_save.h"

#include "cp3_profile.h"

#include "bus_iis_interface.h"
//#include "spi.h"

#ifndef AUDIOCODES_VOIP
#include "codec_def.h"
#include "codec_descriptor.h"
//#include "../voip_dsp/include/dtmf_dec.h"

#include "../voip_dsp/dsp_r1/include/lexra_radiax.h"

//#ifdef FXO_CALLER_ID_DET
//#include "fsk_det.h"
//extern long cid_type[MAX_VOIP_CH_NUM];
//#endif
#endif /*AUDIOCODES_VOIP*/


//#ifdef 	CONFIG_RTK_VOIP_LED
//#include "led.h"
//#endif

//#include "../voip_dsp/ivr/ivr.h"

//#ifdef T38_STAND_ALONE_HANDLER
//#include "t38_handler.h"
//#endif

//#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
//#include "./iphone/ipphone_interface.h"
//#endif

#ifdef CONFIG_VOIP_COP3_PROFILE
#include "voip_debug.h"
extern st_CP3_VoIP_param cp3_voip_param;
#endif

#ifdef SUPPORT_VOIP_DBG_COUNTER
extern uint32 gVoipCounterEnable;
extern void IIS_tx_count(uint32 chid);
extern void IIS_rx_count(uint32 chid);
#endif
#include "voip_init.h"
#include "voip_dev.h"
#include "con_register.h"
#include "con_bus_handler.h"

static voip_bus_t bus_iis[ BUS_IIS_CH_NUM ];

#ifdef CONFIG_RTK_VOIP_DRIVERS_IIS_WIDEBAND
#define IIS_WIDEBAND_16KHZ 1
#else
#undef IIS_WIDEBAND_16KHZ
#endif

//#ifdef IIS_WIDEBAND_16KHZ
//extern void* pResampler_down_st;
//extern void* pResampler_up_st;
//int resampler_process_int(void *st,
//                                 uint32_t channel_index,
//                                 const int16_t *in,
//                                 uint32_t *in_len,
//                                 int16_t *out,
//                                 uint32_t *out_len);
//#endif

#define IIS_PAGE_NUM	2
#ifdef IIS_WIDEBAND_16KHZ
  #define IIS_SAMPLE_RATE	1	//0->8khz, 1->16khz
  #define IIS_PAGE_SIZE	(80*PCM_PERIOD)
#else
#define IIS_SAMPLE_RATE	0	//0->8khz, 1->16khz
  #define IIS_PAGE_SIZE	(40*PCM_PERIOD)	/* word 4-byte unit */
#endif
#define IIS_CH_NUM	1
//#define IIS_PERIOD	1	//unit:10ms, IIS_PERIOD remove, using PCM_PERIOD
#define MAX_IIS_CH_NUM 2
#define SUPPORT_LEC_G168_IIS_ISR



#ifdef OPTIMIZATION	/* byte unit */
#define iis_get_page_size(x) (IIS_PAGE_SIZE*4)
#endif




static int iis_tx_buf[IIS_PAGE_SIZE* IIS_PAGE_NUM] __attribute__((aligned(32))); /* dma uncache data and cpu cache data can't in the same cache line */
static int iis_rx_buf[IIS_PAGE_SIZE* IIS_PAGE_NUM] __attribute__((aligned(32))); /* dma uncache data and cpu cache data can't in the same cache line */

//extern int pcm_ch_for_DAA[];
//static int iis_caller_id_det[4] = {0, 0, 0, 0};
//static int iis_fax_modem_det[4] = {0, 0, 0, 0};
//static int iis_tone_det[4] = {0, 0, 0, 0};
//static int iis_clean_pcm_rx_flag[4] = {0, 0, 0, 0};
//const int * ptr_fax_modem_det = iis_fax_modem_det;

//#ifdef PCM_HANDLER_USE_TASKLET
//extern struct tasklet_struct pcm_handler_tasklet;
//void PCM_handler_2(unsigned long dummy);
//#endif


static int iis_txpage[MAX_IIS_CH_NUM];
static int iis_rxpage[MAX_IIS_CH_NUM];
static int iis_tr_cnt[MAX_IIS_CH_NUM];
static char iis_chanEnabled[MAX_IIS_CH_NUM];
//extern char chanEnabled[];

static int iis_isr_cnt = 0;

static unsigned long IISChanTxPage[4] = {IIS_TX_P0OK, IIS_TX_P1OK, IIS_TX_P2OK, IIS_TX_P3OK};
static unsigned long IISChanRxPage[4] = {IIS_RX_P0OK, IIS_RX_P1OK, IIS_RX_P2OK, IIS_RX_P3OK};

//unsigned long **piis_RxBuf, **piis_TxBuf;
unsigned long * piis_RxBuf[MAX_IIS_CH_NUM];
unsigned long * piis_TxBuf[MAX_IIS_CH_NUM];


//#ifdef SUPPORT_PCM_FIFO
//extern uint32 rx_fifo[][PCM_FIFO_SIZE][RX_BUF_SIZE/(sizeof(uint32))];
//extern uint32 tx_fifo[][PCM_FIFO_SIZE][TX_BUF_SIZE/(sizeof(uint32))];
//extern unsigned char tx_fifo_cnt_w[], tx_fifo_cnt_r[];
//extern unsigned char rx_fifo_cnt_w[], rx_fifo_cnt_r[];
//extern int fifo_start[];
//#define NEXT_FIFO_ENTRY(x) ( (x+1)==PCM_FIFO_SIZE? 0: (x+1) )
//#define PREV_FIFO_ENTRY(x) ( (x)==0? (PCM_FIFO_SIZE-1): (x-1) )
//#endif

//#ifdef REDUCE_PCM_FIFO_MEMCPY
// uint32* piis_RxBufTmp; // For reducing memcpy
// uint32* piis_TxBufTmp;
//#else
// uint32 iis_RxBufTmp[MAX_IIS_CH_NUM][RX_BUF_SIZE/4];
// uint32 iis_TxBufTmp[MAX_IIS_CH_NUM][TX_BUF_SIZE/4];
//#endif

//unsigned int iis_rxlayerbuf[MAX_IIS_CH_NUM];
//unsigned int iis_txlayerbuf[MAX_IIS_CH_NUM];

//int iis_tx_mute[IIS_CH_NUM] = {0};
//int iis_rx_mute[IIS_CH_NUM] = {0};

/****************** IIS DTMF DET & REMOVAL RELATED DEFINE *************************/
//#ifdef DTMF_DEC_ISR_IIS
//extern unsigned char dtmf_chid[];
//int16_t iis_det_buff[MAX_IIS_CH_NUM][RX_BUF_SIZE/2];
//Dtmf_det_out iis_dtmf_digit;
//#endif /* DTMF_DEC_ISR_IIS */

//#ifdef DTMF_REMOVAL_ISR
//extern unsigned char dtmf_removal_flag[];
//extern char dtmf_mode[]; /* 0:rfc2833  1: sip info  2: inband  */
//extern int send_2833_by_ap[];
//#endif


//extern unsigned char support_lec_g168[] ;	// 0: LEC disable  1: LEC enable
//#ifdef LEC_G168_ISR_SYNC_P
//static short vpat[16]={32767,30272,23170,12539,0,-12539,-23170,-30272,-32767,-30272,-23170,-12539, 0,12539,23170,30272 };
//static char sync = 0;
//static int cnt_time=0, sync_frame=0, sync_frame_R=0, sync_start=0;
//static int level = 2000;
//#endif

#include "voip_types.h"
//#include "../voip_dsp/dsp_r1/include/typedef.h"
#include "voip_control.h"

//extern TstVoipFskT2Cid_det stVoipFskT2Cid_det[];

//#ifdef SEND_RFC2833_ISR
//#include "dsp_main.h"
//extern unsigned char RtpOpen[];
//extern int RtpTx_transmitEvent_ISR( uint32 chid, uint32 sid, int event);
//extern int g_digit[MAX_VOIP_CH_NUM];//={0};
//#endif

#ifdef RTK_VOICE_RECORD
char txdatabuf[DATAGETBUFSIZE];
char rxdatabuf[DATAGETBUFSIZE];
//char txdatabuf2[DATAGETBUFSIZE];
char rxdatabuf2[DATAGETBUFSIZE];
TstVoipdataget stVoipdataget[BUS_IIS_CH_NUM] = {[0 ... BUS_IIS_CH_NUM-1]={0,0,0,0,0,0,0,txdatabuf, rxdatabuf, rxdatabuf2}};
#endif /* RTK_VOICE_RECORD */

//#ifdef NEW_REMOTE_TONE_ENTRY
//extern void MixRxToneBuffer( uint32 chid, uint32 *pRxBuffer );
//#endif

#ifdef VOICE_GAIN_ADJUST_IVR_TONE_VOICE
extern long voice_gain_spk[];//0 is mute, 1 is -31dB ~~~ 32 is 0dB , 33 is 1dB ~~~ 63 is 31dB
#endif

static void iis_isr_reset(unsigned int chid);

static void iis_enableChan(unsigned int chid);
static void iis_disableChan(unsigned int chid);

//========================================================//


/* size :byte unit */
static int iis_set_page_size(unsigned int chid, unsigned int size)
{
	/* Write the reg IIS_SETTING to set pagesize. */
	
	unsigned int n_size;
	unsigned int temp;
	n_size = (size/4 - 1);
	temp = rtl_inl(IIS_SETTING) & (~0xFFF);
	rtl_outl(IIS_SETTING, temp | n_size );	//set pagesize

	//IISDBUG("set channel %d page size = %d\n", chid, size);
	// too many console message will cause R0, T0
	//printk("set channel %d page size = %d\n", chid, size);
	return 0;
}

#ifndef OPTIMIZATION
static unsigned int iis_get_page_size(unsigned int chid)
{
	/* Read the reg IIS_SETTING to get pagesize*/
	unsigned int pagesize, n_size;	/* Actual pagesize which can get from "iis_get_page_size()".
 					It's different from the IISPAGE_SIZE define in header file. */
	
	n_size =  rtl_inl(IIS_SETTING) & 0xFFF;
	pagesize = 4*(n_size + 1);

	//IISDBUG("get channel %d page size = %d\n", chid, pagesize);

	return pagesize;
}
#endif


/* Set Tx, Rx own bit to IIS Controller. */

static void iis_set_tx_own_bit(unsigned int pageindex)
{
	//printk("iis_tx:%d\n", pageindex);
	rtl_outl(IIS_TX_P0OWN + 4*pageindex, BIT(31));
	//printk("IIS_TX_P%dOWN= 0x%x\n", pageindex, rtl_inl(IIS_TX_P0OWN + 4*pageindex));
	//PDBUG("set iis tx own bit %d to HW \n", pageindex );
}

static void iis_set_rx_own_bit(unsigned int pageindex)
{
	//printk("rx:%d\n", pageindex);
	rtl_outl(IIS_RX_P0OWN + 4*pageindex, BIT(31));
	//printk("IIS_RX_P%dOWN= 0x%x\n", pageindex, rtl_inl(IIS_RX_P0OWN + 4*pageindex));
	//PDBUG("set iis rx own bit %d to HW \n", pageindex );
}

#if 0 // test
static void pcm_set_tx_own_bit_all(unsigned int chid)
{
	//printk("tx:%d\n", pageindex);
	pcm_outl(TX_BSA(chid), pcm_inl(TX_BSA(chid))|0x3);
	//printk("CH0TXBSA= 0x%x\n", pcm_inl(CH0TXBSA));
	//PDBUG("set chid %d tx own bit %d to HW \n",chid, pageindex );
}

static void pcm_set_rx_own_bit_all(unsigned int chid)
{
	//printk("rx:%d\n", pageindex);
	pcm_outl(RX_BSA(chid), pcm_inl(RX_BSA(chid))|0x3);
	//printk("CH0RXBSA= 0x%x\n", pcm_inl(CH0RXBSA));
	//PDBUG("set chid %d rx own bit %d to HW\n",chid, pageindex );
}


// return value, 0: owned by CPU, non-zero: owned by HW
static int pcm_get_tx_own_bit(unsigned int chid, unsigned int pageindex)
{
	return  ( pcm_inl(TX_BSA(chid)) & BIT(pageindex));
}

static int pcm_get_rx_own_bit(unsigned int chid, unsigned int pageindex)
{
	return  ( pcm_inl(RX_BSA(chid)) & BIT(pageindex));
}

#endif /* 0 */

/* clean tx interrupt pending bits */
static void iis_tx_clean_isr(unsigned int statusval)
{
	rtl_outl(IIS_TX_ISR, statusval);
	//IISDBUG("clean iis tx pending status bits.\n");
}


/* clean rx interrupt pending bits */
static void iis_rx_clean_isr(unsigned int statusval)
{
	rtl_outl(IIS_RX_ISR, statusval);
	//IISDBUG("clean iis rx pending status bits.\n");
}

/* Get the Tx, Rx base address */
static unsigned int iis_get_tx_base_addr(unsigned int chid)
{
	unsigned int txbaseaddr;
 	txbaseaddr = (rtl_inl(TX_PAGE_PTR) & 0xFFFFFFFC)|0xA0000000; // to virtual non-cached address
	
	IISDBUG(" get Tx base addresss = 0x%x\n", txbaseaddr);
	return txbaseaddr;
	
}


static unsigned int iis_get_rx_base_addr(unsigned int chid)
{
	unsigned int rxbaseaddr;
 	rxbaseaddr = ((rtl_inl(RX_PAGE_PTR)) & 0xFFFFFFFC)|0xA0000000; // to virtual non-cached address
 	IISDBUG(" get Rx base addresss = 0x%x\n", rxbaseaddr);
	return rxbaseaddr;
}



static void iis_enable(void)
{
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY
	/* config Share Pin as IIS */


#endif

	iis_isr_reset( 0 );/* ack all pending isr */

	rtl_outl(IISCR, SW_RSTN);	// pause IIS
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM89xxC
	asm volatile ("nop\n\t");// add nop fix test chip cpu 5281 bug, formal chip is ok. advised by yen@rtk
#endif
	rtl_outl(IISCR, 0);	// reset IIS
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM89xxC
	asm volatile ("nop\n\t");// add nop fix test chip cpu 5281 bug, formal chip is ok. advised by yen@rtk
#endif
	rtl_outl(IISCR, SW_RSTN);

	rtl_outl(IIS_SETTING, (IIS_PAGE_SIZE-1) | ((IIS_PAGE_NUM-1)<<12) | (IIS_SAMPLE_RATE << 14) );	//set page size, page number, sampling rate

	IISDBUG("Enable IIS interface. IISCR(%X) = 0x%X. IIS_SETTING(%X) = 0x%X\n", IISCR, rtl_inl(IISCR), IIS_SETTING, rtl_inl(IIS_SETTING));
}

static void iis_disable(void)
{
	rtl_outl(IISCR, 0x0000);	// reset IIS
	rtl_outl(IISCR, SW_RSTN);

	rtl_outl(IIS_SETTING, (IIS_PAGE_SIZE-1) | ((IIS_PAGE_NUM-1)<<12) | (IIS_SAMPLE_RATE << 14) );	//set page size, page number, sampling rate

	IISDBUG("Disable IIS interface. IISCR(%X) = 0x%X. IIS_SETTING(%X) = 0x%X\n", IISCR, rtl_inl(IISCR), IIS_SETTING, rtl_inl(IIS_SETTING));
}

static void iis_tx_rx_enable(unsigned int chid)
{
	unsigned int temp;

	iis_isr_reset(chid);/* ack all pending isr */

	temp = rtl_inl(IISCR) & (~0x7f);
	rtl_outl(IISCR, temp | IIS_WL_16BIT | IIS_MODE_MONO | IIS_EDGE_N | IIS_TXRXACT | IIS_ENABLE);

	IISDBUG("Enable IIS interface TX RX. IISCR(%X) = 0x%X. IIS_SETTING(%X) = 0x%X\n", IISCR, rtl_inl(IISCR), IIS_SETTING, rtl_inl(IIS_SETTING));
}

static void iis_isr_reset(unsigned int chid)
{
	//printk("1 IIS_TX_ISR= 0x%x. IIS_RX_ISR= 0x%x\n", rtl_inl(IIS_TX_ISR), rtl_inl(IIS_RX_ISR));
	rtl_outl(IIS_TX_ISR, 0x3f);
	rtl_outl(IIS_RX_ISR, 0x3f);
	//printk("2 IIS_TX_ISR= 0x%x. IIS_RX_ISR= 0x%x\n", rtl_inl(IIS_TX_ISR), rtl_inl(IIS_RX_ISR));
}

static void iis_imr_enable(unsigned int chid, unsigned char type)
{
	//IISDBUG("enable IIS IMR\n");
#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) || defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC)
	switch(type)
	{
		case P0OK_TX:
			rtl_outl(IIS_TX_IMR ,rtl_inl(IIS_TX_IMR)|(IIS_TX_P0OK));
			break;
	
		case P1OK_TX:
			rtl_outl(IIS_TX_IMR ,rtl_inl(IIS_TX_IMR)|(IIS_TX_P1OK));
			break;
	
		case P2OK_TX:
			rtl_outl(IIS_TX_IMR ,rtl_inl(IIS_TX_IMR)|(IIS_TX_P2OK));
			break;
	
		case P3OK_TX:
			rtl_outl(IIS_TX_IMR ,rtl_inl(IIS_TX_IMR)|(IIS_TX_P3OK));
			break;

		case TPUA:	/* tx page unavailable */
			rtl_outl(IIS_TX_IMR ,rtl_inl(IIS_TX_IMR)|(IIS_TX_PUA));
			break;
	
		case TFEM:	/* tx fifo empty */
			rtl_outl(IIS_TX_IMR ,rtl_inl(IIS_TX_IMR)|(IIS_TX_FEM));
			break;

		case P0OK_RX:
			rtl_outl(IIS_RX_IMR ,rtl_inl(IIS_RX_IMR)|(IIS_RX_P0OK));
			break;
	
		case P1OK_RX:
			rtl_outl(IIS_RX_IMR ,rtl_inl(IIS_RX_IMR)|(IIS_RX_P1OK));
			break;
	
		case P2OK_RX:
			rtl_outl(IIS_RX_IMR ,rtl_inl(IIS_RX_IMR)|(IIS_RX_P2OK));
			break;
	
		case P3OK_RX:
			rtl_outl(IIS_RX_IMR ,rtl_inl(IIS_RX_IMR)|(IIS_RX_P3OK));
			break;

		case RPUA:	/* rx page unavailable */
			rtl_outl(IIS_RX_IMR ,rtl_inl(IIS_RX_IMR)|(IIS_RX_PUA));
			break;
	
		case RFFU:	/* rx fifo full */
			rtl_outl(IIS_RX_IMR ,rtl_inl(IIS_RX_IMR)|(IIS_RX_FFU));
			break;

		default:
			printk("enable channel %d IMR type error!\n", chid);
			break;
	}

#endif

	//printk("IIS_IMR %X", rtl_inl(IIS_RX_IMR));
}


static void iis_imr_disable(unsigned int chid, unsigned char type)
{
#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) || defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC)
	switch(type)
	{
		case P0OK_TX:
			rtl_outl(IIS_TX_IMR ,rtl_inl(IIS_TX_IMR)& (~(IIS_TX_P0OK)));
			break;
	
		case P1OK_TX:
			rtl_outl(IIS_TX_IMR ,rtl_inl(IIS_TX_IMR)& (~(IIS_TX_P1OK)));
			break;
	
		case P2OK_TX:
			rtl_outl(IIS_TX_IMR ,rtl_inl(IIS_TX_IMR)& (~(IIS_TX_P2OK)));
			break;
	
		case P3OK_TX:
			rtl_outl(IIS_TX_IMR ,rtl_inl(IIS_TX_IMR)& (~(IIS_TX_P3OK)));
			break;

		case TPUA:	/* tx page unavailable */
			rtl_outl(IIS_TX_IMR ,rtl_inl(IIS_TX_IMR)& (~(IIS_TX_PUA)));
			break;
	
		case TFEM:	/* tx fifo empty */
			rtl_outl(IIS_TX_IMR ,rtl_inl(IIS_TX_IMR)& (~(IIS_TX_FEM)));
			break;

		case P0OK_RX:
			rtl_outl(IIS_RX_IMR ,rtl_inl(IIS_RX_IMR)& (~(IIS_RX_P0OK)));
			break;
	
		case P1OK_RX:
			rtl_outl(IIS_RX_IMR ,rtl_inl(IIS_RX_IMR)& (~(IIS_RX_P1OK)));
			break;
	
		case P2OK_RX:
			rtl_outl(IIS_RX_IMR ,rtl_inl(IIS_RX_IMR)& (~(IIS_RX_P2OK)));
			break;
	
		case P3OK_RX:
			rtl_outl(IIS_RX_IMR ,rtl_inl(IIS_RX_IMR)& (~(IIS_RX_P3OK)));
			break;

		case RPUA:	/* rx page unavailable */
			rtl_outl(IIS_RX_IMR ,rtl_inl(IIS_RX_IMR)& (~(IIS_RX_PUA)));
			break;
	
		case RFFU:	/* rx fifo full */
			rtl_outl(IIS_RX_IMR ,rtl_inl(IIS_RX_IMR)& (~(IIS_RX_FFU)));
			break;

		default:
			printk("disable channel %d IMR type error!\n", chid);
			break;
	}
#endif /* (CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY) || (CONFIG_RTK_VOIP_DRIVERS_PCM89xxC) */
}

static void EnaIisIntr(uint32 chid)
{
	iis_isr_reset(chid);
	iis_imr_enable(chid, P0OK_TX);
	iis_imr_enable(chid, P1OK_TX);
	//iis_imr_enable(chid, P2OK_TX);
	//iis_imr_enable(chid, P3OK_TX);
	iis_imr_enable(chid, P0OK_RX);
	iis_imr_enable(chid, P1OK_RX);
	//iis_imr_enable(chid, P2OK_RX);
	//iis_imr_enable(chid, P3OK_RX);
	iis_imr_enable(chid, TPUA);
	iis_imr_enable(chid, RPUA);
	//iis_imr_enable(chid, TFEM);
	//iis_imr_enable(chid, RFFU);
}

static void DisIisIntr(uint32 chid)
{
	iis_imr_disable(chid, P0OK_TX);
	iis_imr_disable(chid, P1OK_TX);
	iis_imr_disable(chid, P2OK_TX);
	iis_imr_disable(chid, P3OK_TX);
	iis_imr_disable(chid, P0OK_RX);
	iis_imr_disable(chid, P1OK_RX);
	iis_imr_disable(chid, P2OK_RX);
	iis_imr_disable(chid, P3OK_RX);
	iis_imr_disable(chid, TPUA);
	iis_imr_disable(chid, RPUA);
	iis_imr_disable(chid, TFEM);
	iis_imr_disable(chid, RFFU);

	iis_isr_reset(chid);
}

static void iis_init_var(void)
{

	int i;
	//extern unsigned int flash_hook_time[];
	//extern unsigned int flash_hook_min_time[];

	for(i=0; i<IIS_CH_NUM; i++)
	{
		iis_chanEnabled[i] = FALSE;

		//Init_Hook_Polling(i);
		//flash_hook_time[i] = 30;
		//flash_hook_min_time[i] = 0;

#ifdef SUPPORT_PCM_FIFO
		//PCM_FIFO_Init(i);
#endif

		piis_RxBuf[i] = (unsigned long *)((uint32)iis_get_rx_base_addr(i));  // Get virtual non-cached address
		piis_TxBuf[i] = (unsigned long *)((uint32)iis_get_tx_base_addr(i));  // Get virtual non-cached address

		iis_txpage[i] = 0;
		iis_rxpage[i] = 0;
		iis_tr_cnt[i] = 0;

		//InitializeIVR( i );

		//sync_point_iis[i] = SYNC_POINT;
		//sync_sample_iis[i] = SYNC_SAMPLE;
		//sync_sample_offset_daa_iis[i] = 0;

		//printk("----> sync_sample_offset_daa[%d] = %d\n", i, sync_sample_offset_daa[i]);
		
		//if (pcm_ch_for_DAA[i] == 1)
		//	AES_ON(i);

		//iis_fax_modem_det[i] = 0;
		//iis_caller_id_det[i] = 0;
		//iis_tone_det[i] = 0;

	}


	//piis_RxBuf = (uint32**)((uint32)&iis_rxlayerbuf[0]);
	//piis_TxBuf = (uint32**)((uint32)&iis_txlayerbuf[0]);

	for (i=0; i<IIS_CH_NUM; i++)
	{
		BOOT_MSG("piis_TxBuf[%d]=%p\n", i, piis_TxBuf[i]);
		BOOT_MSG("piis_RxBuf[%d]=%p\n", i, piis_RxBuf[i]);
	}

	//ProfileInit();
}

#ifdef SUPPORT_PCM_FIFO
static void iis_ISR(uint32 iis_txisr, uint32 iis_rxisr)
{
	const uint32 bch = 0;
	uint32 i, j;

	//uint32 tx_isrpage, rx_isrpage;


	for (i=0; i < IIS_PAGE_NUM; i++) // page0/page1/page2/page3
	{
	    //int need_IIS_RX;
//#ifndef SUPPORT_SYS_DMEM_STACK
//	static
//#endif
//		__attribute__((aligned(16)))
//#ifdef IIS_WIDEBAND_16KHZ
//		uint32 iis_rxBuf[2][2*PCM_PERIOD*PCM_PERIOD_10MS_SIZE/sizeof(uint32)];
//#else
//		uint32 iis_rxBuf[2][PCM_PERIOD*PCM_PERIOD_10MS_SIZE/sizeof(uint32)];
//#endif
		//memset(&need_IIS_RX, 0, sizeof(need_IIS_RX));
		extern void isr_bus_reset_need_rx_vars( void );
    	isr_bus_reset_need_rx_vars();
    	
    	voip_bus_t * const p_bus = &bus_iis[ bch ];
    	voip_con_t * const p_con = p_bus ->con_ptr;
    	
		//if (chanEnabled[chid] == FALSE)
		//	continue;
		if( !p_bus ->enabled )
			continue;

		if ( iis_txisr & IISChanTxPage[iis_txpage[0]] )
		{
			//uint32* txbuf = &pTxBuf[bch][txpage[bch]*(pcm_get_page_size(bch)>>2)];
			uint32* txbuf = &piis_TxBuf[bch][iis_txpage[bch]*(iis_get_page_size(bch)>>2)];
					
			if( p_con ->con_ops ->isr_bus_read_tx( p_con, 
						p_bus, ( uint16 * )txbuf ) == NULL )
			{
				// TX FIFO is empty!
				memset(txbuf, 0, iis_get_page_size(bch));	
				printk("I_TE(%d) ", bch);
			} else {
				if( bch == 0 )
					ddinst_rw_auto( VOIP_DEV_PCM0_TX, ( char * )txbuf, 160 );
				
#ifdef SUPPORT_VOIP_DBG_COUNTER
				if (gVoipCounterEnable == 1)
					IIS_tx_count(bch);
#endif
				//int i;
#ifdef IIS_WIDEBAND_16KHZ
				//int in_length = 80*PCM_PERIOD, out_length = 160*PCM_PERIOD;
				//short tmp_buf[80*PCM_PERIOD]={0};
//
				//iis_read_tx_fifo(bch, tmp_buf);
				//if ( 0 != resampler_process_int(pResampler_up_st, 8/* using 8ssid avoid */, tmp_buf, &in_length, txbuf, &out_length) ) {
				//	memset(txbuf, 0, iis_get_page_size(bch));
				//	printk("Up error \n");
				//}
#ifdef VOICE_GAIN_ADJUST_IVR_TONE_VOICE
				//voice_gain( txbuf, PCM_PERIOD*80*2, voice_gain_spk[bch]);
#endif
#else
				//iis_read_tx_fifo(chid, txbuf);
#ifdef VOICE_GAIN_ADJUST_IVR_TONE_VOICE
				//voice_gain( txbuf, PCM_PERIOD*80, voice_gain_spk[bch]);
#endif
#endif
#if 0
//#ifdef RTK_VOICE_RECORD
				int k;
				if (stVoipdataget[bch].write_enable==2)
				{
					for (i=0; i < PCM_PERIOD; i++)
					{
#ifdef IIS_WIDEBAND_16KHZ
						for (k=0; k<160; k++)
#else
						for (k=0; k<80; k++)
#endif
						{
							*((short*)txbuf+i*(PCM_PERIOD_10MS_SIZE/2)+k) = Sin1KHz[k%8];
						}
					}
				}
					
				if ((stVoipdataget[bch].write_enable==4) || (stVoipdataget[bch].write_enable==2))
				{
					for (i=0; i < PCM_PERIOD; i++)
					{
#ifdef IIS_WIDEBAND_16KHZ
						memcpy(&stVoipdataget[bch].txbuf[stVoipdataget[bch].tx_writeindex],txbuf + i*(2*PCM_PERIOD_10MS_SIZE/4),320);
						stVoipdataget[bch].tx_writeindex= (stVoipdataget[bch].tx_writeindex+320)%DATAGETBUFSIZE;
#else
						memcpy(&stVoipdataget[bch].txbuf[stVoipdataget[bch].tx_writeindex],txbuf + i*(PCM_PERIOD_10MS_SIZE/4),160);
						stVoipdataget[bch].tx_writeindex= (stVoipdataget[bch].tx_writeindex+160)%DATAGETBUFSIZE;
#endif
					}
				}
#endif //#ifdef RTK_VOICE_RECORD
			}
			iis_set_tx_own_bit(iis_txpage[0]);
			iis_txisr &= ~IISChanTxPage[iis_txpage[0]];
			iis_txpage[0] = (iis_txpage[0] +1 ) % IIS_PAGE_NUM;

			iis_tr_cnt[0]++;
		} // end of tx

		if ( iis_rxisr & IISChanRxPage[iis_rxpage[0]] ) {
			
			// iis_set_rx_own_bit ASAP helps a lot!
			if( p_con ->con_ops ->isr_bus_write_rx_TH( p_con, p_bus, 
					( uint16 * )&piis_RxBuf[bch][iis_rxpage[bch]*(iis_get_page_size(bch)>>2)] )
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
					IIS_rx_count(bch);
			}
#endif
	
			iis_set_rx_own_bit(iis_rxpage[0]);
//#ifdef LEC_USE_CIRC_BUF
//			LEC_buf_rindex_IIS[chid]=(LEC_buf_rindex_IIS[chid]+1)%FSIZE;
//#endif


			//need_IIS_RX = 1;
			iis_rxisr &= ~IISChanRxPage[iis_rxpage[0]];
			iis_rxpage[0] = (iis_rxpage[0]+1) % IIS_PAGE_NUM;

			iis_tr_cnt[0]--;
		} // end of for j
		
		// Do IIS_RX()
		for (j = 0; j < 1; j++) 
		{
			const voip_bus_t * const p_bus = &bus_iis[ j ];
	    	voip_con_t * const p_con = p_bus ->con_ptr;
			
			if( !p_bus ->enabled )
				continue;
			
			p_con ->con_ops ->isr_bus_write_rx_BH( p_con );
			
#if 0
			if (need_IIS_RX) {
				need_IIS_RX = 0;
#ifdef IIS_WIDEBAND_16KHZ
				int in_length = 160, out_length = 80;
				short tmp_buf[80]={0};
	
				if ( 0 != resampler_process_int(pResampler_down_st, 8, iis_rxBuf[j], &in_length, tmp_buf, &out_length)) {
					printk("Down error \n");
					memset(tmp_buf, 0, 160);
				}
				IIS_RX(j, tmp_buf);
#else
				IIS_RX(j, iis_rxBuf[j]);
#endif
			}
#endif
		}
	} // end of for i

#if 1 
	if ((iis_rxisr != 0) | (iis_txisr != 0))
		printk(" iis_txisr = %X, iis_rxisr = %X ", iis_txisr, iis_rxisr);
#endif	


//#ifdef PCM_HANDLER_USE_TASKLET
//	tasklet_hi_schedule(&pcm_handler_tasklet);	
//#else
//	PCM_handler_2(NULL);
//#endif
	isr_schedule_bus_handler();

	return;
}
#else
#error
#endif

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

#undef SUPPORT_SYS_DMEM_STACK
#ifdef SUPPORT_SYS_DMEM_STACK
#include "../../voip_dsp/dsp_r1/common/util/codec_mem.h"
#endif



#define CHECK_IIS_ISR_AGAIN
#define CHECK_IIS_ISR_REENTRY
#ifdef CHECK_IIS_ISR_REENTRY
int in_iis_isr = 0;
#endif

static void iis_twiddle(void)
{
	static int twiddle_count = 0;
        static const char tiddles[]="-\\|/";
#if 1
        printk("%c", tiddles[(twiddle_count++)&3]);
        printk("%c", '\b');
#else
        putchar(tiddles[(twiddle_count++)&3]);
        putchar('\b');
#endif
}

#ifdef CONFIG_DEFAULTS_KERNEL_2_6
static irq_handler_t iis_interrupt(int32 irq, void *dev_instance, struct pt_regs *regs)
#else
static void iis_interrupt(int32 irq, void *dev_instance, struct pt_regs *regs)
#endif
{
	unsigned int status_val_tx;
	unsigned int status_val_rx;

#if 0
	if ((status_val_tx = rtl_inl(IIS_TX_ISR)) | (status_val_rx = rtl_inl(IIS_RX_ISR))) {
	
		rtl_outl(IIS_TX_ISR, status_val_tx);
		rtl_outl(IIS_RX_ISR, status_val_rx);

		memset(&piis_TxBuf[0][0*iis_get_page_size(chid)>>2], 0xaa, iis_get_page_size(chid)*2);	
#if 0
		int k;
		for (k=0; k<160; k++)
		{
			*(((short*)&piis_TxBuf[0][0*iis_get_page_size(chid)>>2])+k) = Sin1KHz[k%8];
		}
#endif
			iis_set_tx_own_bit(0);
			iis_set_tx_own_bit(1);
			iis_set_rx_own_bit(0);
			iis_set_rx_own_bit(1);
		iis_twiddle();
	}
#endif
	//printk("IISa");
#if 1
#ifdef CHECK_IIS_ISR_REENTRY
	if (in_iis_isr == 1) {
		printk("IIS ISR re-entry\n");
		while (1) ;
	}
	in_iis_isr = 1;	
#endif

#ifndef AUDIOCODES_VOIP
	save_radiax_reg(); /* save radiax register value */
#else
	volatile struct pt_radiax radiax_regs;
	save_radiax_reg(&radiax_regs); /* save radiax register value */
#endif

#ifdef SUPPORT_SYS_DMEM_STACK
//#error
	extern int dmem_size;
	set_DMEM(&__sys_dmem_start, dmem_size-1);

	sys_dmem_sp = &(sys_dmem_stack[SYS_DMEM_SSIZE]);
	entry_dmem_stack(&sys_orig_sp, &sys_dmem_sp);
#endif    

#ifdef FEATURE_COP3_PCMISR
	if (cp3_voip_param.bCp3Count_PCM_ISR == 1)
		ProfileEnterPoint(PROFILE_INDEX_PCMISR);
#endif
#ifdef CHECK_IIS_ISR_AGAIN
	//int iis_isr_cnt = 0;
	while ((status_val_tx = rtl_inl(IIS_TX_ISR)) | (status_val_rx = rtl_inl(IIS_RX_ISR)))
#else
	if ((status_val_tx = rtl_inl(IIS_TX_ISR)) | (status_val_rx = rtl_inl(IIS_RX_ISR)))
#endif	
	{
#ifdef CHECK_IIS_ISR_AGAIN
#if 0
		iis_isr_cnt++;
#endif		
#endif
		rtl_outl(IIS_TX_ISR, status_val_tx);
		rtl_outl(IIS_RX_ISR, status_val_rx);

		if ((status_val_tx & 0x0F) | (status_val_rx & 0x0F))	// TOK and ROK only
			iis_ISR(status_val_tx & 0x0F, status_val_rx & 0x0F);

		if ( (status_val_tx & 0x30) || (status_val_rx & 0x30)) // Buffer/Fifo Unavailable only
		{
			if (status_val_tx & 0x10)
				printk("TBU ");
			if (status_val_rx & 0x10)
				printk("RBU ");
			//if (status_val_tx & 0x20)
			//	printk("TFU ");
			//if (status_val_rx & 0x20)
			//	printk("RFU ");
			printk("\n");
		}
	}

#ifdef CHECK_IIS_ISR_AGAIN
#if 0
	if ( iis_isr_cnt > 1)
		printk(" (%d) ", iis_isr_cnt);
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

#ifdef CHECK_IIS_ISR_REENTRY
	in_iis_isr = 0;	
#endif
#endif

#ifdef CONFIG_DEFAULTS_KERNEL_2_6
    return IRQ_HANDLED;
#endif
}

static void print_iis(void);

static int __init iisctrl_init(void)
{
#if 0	// enable G.726 ITU test vector verification
	extern int g726_itu_verify(void);
	g726_itu_verify();
#endif

	int chid;

	//for (chid=0; chid<1; chid++)
	//{
	//	iis_set_tx_mute(chid, FALSE);
	//	iis_set_rx_mute(chid, FALSE);
	//}





	printk("\n====== RTK IIS Controller Initialization =======\n ");
	int result; 
	unsigned char ch = 0;

	iis_enable();



//========================== Memory Allocated and Request for IRQ =======================//

	rtl_outl(TX_PAGE_PTR, (unsigned int)(Virtual2Physical(iis_tx_buf)));
	rtl_outl(RX_PAGE_PTR, (unsigned int)(Virtual2Physical(iis_rx_buf)));

//===================IP_phone Initialization======================//
#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
	//ipphone_init( 0 );//iis only support liner mode
#endif
	result = 0;

	iis_init_var();

    #if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
	result = request_irq(IIS_IRQ, iis_interrupt, IRQF_DISABLED, "iis_voice", NULL);// NULL OK
    #else
	result = request_irq(IIS_IRQ, iis_interrupt, SA_INTERRUPT, "iis_voice", NULL);// NULL OK
    #endif
	if(result)
	{
		printk("Can't request IRQ for IIS.\n");
			//PERR(KERN_ERR "Can't request IRQ for channel %d.\n", ch);
		return result;

	}
	printk("Request IRQ for IIS Controller OK!.\n");

	print_iis();


	printk("================= FISISH ===============\n ");
	
	return result;
		
}


static void __exit iisctrl_cleanup(void)
{
	iis_disable();
}

static void print_iis(void)	//  for debug
{
	BOOT_MSG("IISCR= 0x%x\n", rtl_inl(IISCR));
	BOOT_MSG("TX_PAGE_PTR= 0x%x\n", rtl_inl(TX_PAGE_PTR));
	BOOT_MSG("RX_PAGE_PTR= 0x%x\n", rtl_inl(RX_PAGE_PTR));
	BOOT_MSG("IIS_SETTING= 0x%x\n", rtl_inl(IIS_SETTING));
	BOOT_MSG("IIS_TX_IMR= 0x%x\n", rtl_inl(IIS_TX_IMR));
	BOOT_MSG("IIS_TX_ISR= 0x%x\n", rtl_inl(IIS_TX_ISR));
	BOOT_MSG("IIS_RX_IMR= 0x%x\n", rtl_inl(IIS_RX_IMR));
	BOOT_MSG("IIS_RX_ISR= 0x%x\n", rtl_inl(IIS_RX_ISR));
	BOOT_MSG("IIS_TX_P0OWN= 0x%x\n", rtl_inl(IIS_TX_P0OWN));
	BOOT_MSG("IIS_TX_P1OWN= 0x%x\n", rtl_inl(IIS_TX_P1OWN));
	BOOT_MSG("IIS_RX_P0OWN= 0x%x\n", rtl_inl(IIS_RX_P0OWN));
	BOOT_MSG("IIS_RX_P1OWN= 0x%x\n", rtl_inl(IIS_RX_P1OWN));	
}



static void IIS_init(void)
{
        unsigned long flags;
#if defined (AUDIOCODES_VOTING_MECHANISM)
	int nPeriod, reg_pbsize, i;
#endif
        save_flags(flags); cli();

        uint32 chid;

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8676

#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8672

#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8671

#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM865xC

#endif

        //init_var();     // can get gloable tx rx base address
        {
#if ! defined (AUDIOCODES_VOTING_MECHANISM)   
                iis_disable();
#else

#endif

        }

#ifndef AUDIOCODES_VOIP
#ifdef SUPPORT_AES_ISR
	Init_AES();
#endif
#endif

#ifdef PCM_HANDLER_USE_TASKLET
	//tasklet_init(&pcm_handler_tasklet, PCM_handler_2, NULL);
	//BOOT_MSG("=== PCM Handler Tasklet Initialization ===\n");
#endif	

        restore_flags(flags);

        //print_pcm();
        //return;
}

static void IIS_shutdown(void)
{
	uint32 chid;

	for (chid=0; chid<IIS_CH_NUM; chid++)
	{
		iis_disableChan(chid);
	}

#ifdef PCM_HANDLER_USE_TASKLET
	//tasklet_kill(&pcm_handler_tasklet);
#endif

}

#if !defined (AUDIOCODES_VOTING_MECHANISM)
static void iis_enableChan(unsigned int chid)
{
	int reg_pbsize;
	Word16 nPeriod;
	int		i;

	// chan id from 0 ~ (VOIP_CH_NUM - 1)
	if(chid >= BUS_IIS_CH_NUM)
		return;
	if(iis_chanEnabled[chid] == TRUE)
		return;

	iis_txpage[chid] = 0;
	iis_rxpage[chid] = 0;
#ifdef LEC_USE_CIRC_BUF	
	//LEC_buf_windex_IIS[chid] =sync_point_iis[chid]-1;
	//LEC_buf_windex[chid] =2;    /* initial value=2, tested in 8186 and 8651B */
	//LEC_buf_rindex_IIS[chid] =0;
#endif
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
#ifdef IIS_WIDEBAND_16KHZ
	iis_set_page_size(chid, reg_pbsize<<1);
#else
	iis_set_page_size(chid, reg_pbsize);
#endif
	//pcm_get_page_size(chid);
#if 1
	for (i=0 ; i<4 ; i++)
	{
		iis_set_tx_own_bit(i);
		iis_set_rx_own_bit(i);
	}
#endif
#ifdef SUPPORT_PCM_FIFO
	//PCM_FIFO_Init(chid);
#endif	
	iis_chanEnabled[chid] = TRUE;
	//printk("TXADR=%X",&piis_TxBuf[0][0*iis_get_page_size(chid)>>2]);
	iis_tx_rx_enable(chid);
	EnaIisIntr(chid);
	//PRINT_MSG("IIS ch%d enable\n", chid);

}
#else //defined (AUDIOCODES_VOTING_MECHANISM)



#endif

#if !defined (AUDIOCODES_VOTING_MECHANISM)
static void iis_disableChan(unsigned int chid)
{
	DisIisIntr(chid);
	if (iis_chanEnabled[chid] == FALSE)
		return;

	// chan id from 0 ~ (VOIP_CH_NUM - 1)
	if (chid >= BUS_IIS_CH_NUM)
		return;

	iis_disable();

	iis_chanEnabled[chid] = FALSE;
}
#else


#endif


static void IIS_restart(unsigned int chid)
{
#if ! defined (AUDIOCODES_VOTING_MECHANISM)
	iis_txpage[chid] = 0;
	iis_rxpage[chid] = 0;
#endif	
	iis_tr_cnt[chid] = 0;
	iis_enableChan(chid);

}

//#ifndef CONFIG_RTK_VOIP_MODULE
//voip_initcall(iisctrl_init);
//voip_exitcall(iisctrl_cleanup);
//#endif

// --------------------------------------------------------

void bus_iis_processor_in_tasklet( void )
{

}

static int bus_iis_enable( voip_bus_t *this, int enable )
{
	const uint32 bch = this ->bch;
	
	if( enable )
		iis_enableChan( bch );
	else
		iis_disableChan( bch );

	return 0;
}

static void bus_iis_set_timeslot( voip_bus_t *this, uint32 ts1, uint32 ts2 )
{
	//printk( "bus_iis_set_timeslot not implement\n" );
}

static void bus_iis_set_format( voip_bus_t *this, uint32 format )
{
#if 0
	const BUS_DATA_FORMAT format = ( BUS_DATA_FORMAT )_format;
#endif
	
}

// --------------------------------------------------------
// channel mapping 
// --------------------------------------------------------

const bus_ops_t bus_iis_ops = {
	// common operation 
	.enable = bus_iis_enable,
	.reset_TH = NULL,	// IIS don't need this 
	.reset_BH = NULL,	// IIS don't need this 
	
	.set_timeslot = bus_iis_set_timeslot,
	.set_format = bus_iis_set_format,
};

static int __init voip_bus_iis_init( void )
{
	extern int __init iisctrl_init( void );
	extern const bus_ops_t bus_iis_ops;
	int i;
	
	iisctrl_init();
	
	for( i = 0; i < BUS_IIS_CH_NUM; i ++ ) {
		bus_iis[ i ].bch = i;
		bus_iis[ i ].name = "iis";
		bus_iis[ i ].bus_type = BUS_TYPE_IIS;
#ifdef IIS_WIDEBAND_16KHZ
		bus_iis[ i ].band_mode_sup = BAND_MODE_16K;
#else
		bus_iis[ i ].band_mode_sup = BAND_MODE_8K;
#endif
		bus_iis[ i ].bus_group = 0;
		bus_iis[ i ].bus_ops = &bus_iis_ops;
	}
	
	register_voip_bus( &bus_iis[ 0 ], BUS_IIS_CH_NUM );
	
	return 0;
}

static void __exit voip_bus_iis_exit( void )
{
	IIS_shutdown();
	iisctrl_cleanup();
}


voip_initcall_bus( voip_bus_iis_init );
voip_exitcall( voip_bus_iis_exit );

// --------------------------------------------------------
// proc 
// --------------------------------------------------------

int voip_iis_regs_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
	int n = 0;

	n += sprintf( buf + n, "IISCR= 0x%08x\n", rtl_inl(IISCR));
	n += sprintf( buf + n, "TX_PAGE_PTR= 0x%08x\n", rtl_inl(TX_PAGE_PTR));
	n += sprintf( buf + n, "RX_PAGE_PTR= 0x%08x\n", rtl_inl(RX_PAGE_PTR));
	n += sprintf( buf + n, "IIS_SETTING= 0x%08x\n", rtl_inl(IIS_SETTING));
	n += sprintf( buf + n, "IIS_TX_IMR= 0x%08x\n", rtl_inl(IIS_TX_IMR));
	n += sprintf( buf + n, "IIS_TX_ISR= 0x%08x\n", rtl_inl(IIS_TX_ISR));
	n += sprintf( buf + n, "IIS_RX_IMR= 0x%08x\n", rtl_inl(IIS_RX_IMR));
	n += sprintf( buf + n, "IIS_RX_ISR= 0x%08x\n", rtl_inl(IIS_RX_ISR));
	n += sprintf( buf + n, "IIS_TX_P0OWN= 0x%08x\n", rtl_inl(IIS_TX_P0OWN));
	n += sprintf( buf + n, "IIS_TX_P1OWN= 0x%08x\n", rtl_inl(IIS_TX_P1OWN));
	n += sprintf( buf + n, "IIS_RX_P0OWN= 0x%08x\n", rtl_inl(IIS_RX_P0OWN));
	n += sprintf( buf + n, "IIS_RX_P1OWN= 0x%08x\n", rtl_inl(IIS_RX_P1OWN));	
	
	*eof = 1;
	return n;
}

int voip_iis_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
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
		n += sprintf( buf + n, "enable: %s\n", ( iis_chanEnabled[ ch ] ?
												"yes" : "no" ) );
	} else {
		//ss = SS_FROM_PROC_DATA( data );
		//n = sprintf( buf, "session=%d\n", ss );
	}
	
	*eof = 1;
	return n;
}

int __init voip_iis_proc_init( void )
{
	create_proc_read_entry( PROC_VOIP_DIR "/iis_regs", 0, NULL, voip_iis_regs_read_proc, NULL );
	
	create_voip_channel_proc_read_entry( "iis", voip_iis_read_proc );
	
	return 0;
}

void __exit voip_iis_proc_exit( void )
{
	remove_voip_proc_entry( PROC_VOIP_DIR "/iis_regs", NULL );
	
	remove_voip_channel_proc_entry( "iis" );
}

voip_initcall_proc( voip_iis_proc_init );
voip_exitcall( voip_iis_proc_exit );

