#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/if_ether.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/spinlock.h>
#include <asm/unaligned.h>
#include <linux/netfilter.h>

#include "rtk_voip.h"
#include "voip_types.h"
#include "../voip_rx/rtk_trap.h"
#include "voip_feature.h"
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
#include "ipc_arch_tx.h"
#elif defined( CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP )
#include "ipc_arch_help_dsp.h"
#include "ipc_arch_tx.h"
#endif
#include "con_mux.h"
#include "snd_define.h"
#include "../include/voip_control.h"
#include "../include/voip_params.h"
#include "../include/voip_version.h"
#include "../voip_dsp/rtp/rtpTypes.h"
#include "voip_init.h"

#include "voip_mgr_events.h"

char bDebugMsg = 1;	// enable or disable debug message. Default is enable.
char benableDbg = 0;	// enable or disable debug message. Default is disable.
char bBootMsg = 0;
extern int dsp_init_first;
#ifdef CONFIG_RTK_VOIP_IPC_ARCH
extern int voip_ch_num;
#endif

#if defined( CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST )
#elif defined( CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP )
//int rtkVoipIsEthernetDsp = 1;
void ( * fn_ipc_RtpTx )(RtpPacket* pst) = ipc_RtpTx;
void ( * fn_ipc_RtcpTx )(RtcpPacket* pst) = ipc_RtcpTx;
void ( * fn_ipc_T38Tx )( unsigned int chid, unsigned int sid, void* packet, unsigned int pktLen) = ipc_T38Tx;
#else
//int rtkVoipIsEthernetDsp = 0;
#ifndef AUDIOCODES_VOIP
void ( * fn_ipc_RtpTx )(RtpPacket* pst) = NULL;
void ( * fn_ipc_RtcpTx )(RtcpPacket* pst) = NULL;
#endif
void ( * fn_ipc_T38Tx )( unsigned int chid, unsigned int sid, void* packet, unsigned int pktLen) = NULL;
#endif

#if defined (CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP) && defined (CONFIG_RTK_VOIP_LED)
extern volatile unsigned int sip_registed[];     //register ok:1, no register:0
extern volatile unsigned int daa_hook_status[];  //off-hook:1 ,on-hook:0
extern volatile unsigned int slic_hook_status[]; //off-hook:1 ,on-hook:0 ,flash-hook:2
extern volatile unsigned int fxs_ringing[];                      //no ring:0 ,voip incoming ring:1
extern volatile unsigned int daa_ringing;
#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA
extern unsigned int pstn_ringing;	//pstn incoming ring:1 ,no ring and voip incoming ring:0
extern char relay_2_PSTN_flag[]; /* 1: relay to PSTN , 0: relay to SLIC */
#elif defined CONFIG_RTK_VOIP_DRIVERS_SI3050
extern unsigned int pstn_ringing[];
#else
extern unsigned int pstn_ringing[];
#endif
#endif
extern uint32 fax_modem_det_mode[]; /* fax modem det mode, 0:auto. 1:fax. 2:modem */

//shlee move from pcm_interface.c due to host side doesnt include voip_driver folder.
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
/* Configure Cache at write allocate mode */
void set_write_allocate( void )
{
	unsigned int  temp;
__asm__ __volatile__ \
("	;\n\t"
	"mfc0	$8, $20		;\n\t"
	"nop					;\n\t"
	"or 		$8, $8, 0x80	;\n\t"
	"nop					;\n\t"
	"mtc0	$8, $20		;\n\t"
	"nop					;\n\t"
	"nop					;\n\t"
	"mfc0	%0, $20		;\n\t"
	:"=r" (temp)
	:
);
	//printk("\r\nset_write_allocate : temp:%X", temp);

}

void read_write_allocate( void )
{
	unsigned int  temp;
__asm__ __volatile__ \
("	;\n\t"
	"nop					;\n\t"
	"mfc0	%0, $20		;\n\t"
	:"=r" (temp)
	:
);
	printk("\r\nread_write_allocate : temp:%X", temp);

}
#endif

int announce_SIP_event( const char *ev_str )
{
#if !defined( CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP )
	// ev_str should like "ef\n"
	
	const char * const filename = "/var/run/solar_control.fifo";
	struct file *filp;
	mm_segment_t oldfs;
	int ret = 0;
	int r;
	
	oldfs = get_fs();
	set_fs( KERNEL_DS );
	
	filp = filp_open( filename, O_RDWR | O_APPEND, 0777 );
	
	if( IS_ERR( filp ) ) {
		ret = -1;
		goto label_open_error;
	}
	
	// write some thing 
	r = filp->f_op->write( filp, ev_str, 3, &filp->f_pos );
	
	filp_close( filp, NULL );
	
label_open_error:
	set_fs( oldfs );		
	
	return ret;
#else
	return 0;
#endif
}

#ifdef CONFIG_REALTEK_VOIP
#if !defined( CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST )

#define RFC2833_FIFO_SIZE 10


static TstRfc2833event rfc2833_event_fifo[MAX_DSP_RTK_SS_NUM][RFC2833_FIFO_SIZE]={{{0}}};
static char rfc2833_event_wp[MAX_DSP_RTK_SS_NUM]={0},rfc2833_event_rp[MAX_DSP_RTK_SS_NUM]={0};

int rfc2833_event_fifo_wrtie(uint32 s_id, unsigned int event)
{
	extern uint32 chanInfo_GetChannelbySession(uint32 sid);
	extern int GetFaxModem_RFC2833RxPlay(uint32 chid);
	uint32 ch;
	
	ch = chanInfo_GetChannelbySession(s_id);
	
	if (GetFaxModem_RFC2833RxPlay(ch) == 1)
	{
		if ( (event < 0) || ((event >16) && (event <32)) || (event > 49))
		{
			PRINT_R("Not support event %d for RFC2833 event FIFO\n", event);
			return -1;
		}
	}
	else
	{
		if ((event < 0) || (event >16))
		{
			if ((event <32) || (event > 49))
				PRINT_R("Not support event %d for RFC2833 event FIFO\n", event);
			return -1;
		}	
	}

	if ((rfc2833_event_wp[s_id]+1)%RFC2833_FIFO_SIZE != rfc2833_event_rp[s_id])
	{
		rfc2833_event_fifo[s_id][(unsigned char)(rfc2833_event_wp[s_id])].event = event;
		rfc2833_event_fifo[s_id][(unsigned char)(rfc2833_event_wp[s_id])].marker = 1;
		rfc2833_event_fifo[s_id][(unsigned char)(rfc2833_event_wp[s_id])].play = 0;
		rfc2833_event_wp[s_id] = (rfc2833_event_wp[s_id]+1) % RFC2833_FIFO_SIZE;
		//PRINT_Y("W");
	}
	else
	{
		PRINT_R("RFC2833 Event FIFO overflow,(%d)\n", s_id);
	}

	return 0;
}

unsigned int Read_current_event_fifo_state(uint32 s_id, TstRfc2833event* pEvent)
{
	unsigned int ret = 0;
	
	if (rfc2833_event_fifo[s_id][(unsigned char)(rfc2833_event_rp[s_id])].marker == 1)
	{
		//pEvent = &rfc2833_event_fifo[s_id][rfc2833_event_rp[s_id]];
		memcpy(pEvent, &rfc2833_event_fifo[s_id][(unsigned char)(rfc2833_event_rp[s_id])], sizeof(TstRfc2833event));
		ret = 1;
	}
	else
	{
		//pEvent = NULL;
		ret = 0;
	}
	
	return ret;
}

void Update_current_event_fifo_state(uint32 s_id, RtpEventDTMFRFC2833* pEvent)
{
	rfc2833_event_fifo[s_id][(unsigned char)(rfc2833_event_rp[s_id])].edge = pEvent->edge;
	rfc2833_event_fifo[s_id][(unsigned char)(rfc2833_event_rp[s_id])].volume = pEvent->volume;
	rfc2833_event_fifo[s_id][(unsigned char)(rfc2833_event_rp[s_id])].duration = pEvent->duration;
	//PRINT_Y("U");
	
}

void Update_current_event_play_state(uint32 s_id, unsigned int play)
{
	rfc2833_event_fifo[s_id][(unsigned char)(rfc2833_event_rp[s_id])].play = play;
	//PRINT_Y("P");
	
}

int rfc2833_event_fifo_read(uint32 s_id)
{
	//int output;

	if ( rfc2833_event_wp[s_id] == rfc2833_event_rp[s_id])
	{
		// FIFO empty
		return -1;
	}
	else
	{
		rfc2833_event_fifo[s_id][(unsigned char)(rfc2833_event_rp[s_id])].marker = 0;
		rfc2833_event_fifo[s_id][(unsigned char)(rfc2833_event_rp[s_id])].edge = 0;
		rfc2833_event_fifo[s_id][(unsigned char)(rfc2833_event_rp[s_id])].duration = 0;
		rfc2833_event_fifo[s_id][(unsigned char)(rfc2833_event_rp[s_id])].event = 0;
		rfc2833_event_fifo[s_id][(unsigned char)(rfc2833_event_rp[s_id])].play = 0;
		rfc2833_event_rp[s_id] = (rfc2833_event_rp[s_id]+1) % RFC2833_FIFO_SIZE;
		return 0;
	}
}

void flush_rfc2833_event_fifo(uint32 s_id)
{
	int i;
	for (i=0; i< RFC2833_FIFO_SIZE; i++)
	{
		rfc2833_event_fifo[s_id][i].marker = 0;
		rfc2833_event_fifo[s_id][i].edge = 0;
		rfc2833_event_fifo[s_id][i].duration = 0;
		rfc2833_event_fifo[s_id][i].event = 0;
		rfc2833_event_fifo[s_id][i].play = 0;
	}
	rfc2833_event_wp[s_id] = 0;
	rfc2833_event_rp[s_id] = 0;
}

#endif // !CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
#endif // CONFIG_REALTEK_VOIP

/****************************************************************/

//#if VOIP_CH_NUM > SLIC_CH_NUM
//int pcm_ch_for_DAA[VOIP_CH_NUM] = {[0 ... DECT_CH_NUM+SLIC_CH_NUM-1] = 0, [DECT_CH_NUM+SLIC_CH_NUM ... VOIP_CH_NUM-1] = 1};
//#else
//int pcm_ch_for_DAA[VOIP_CH_NUM] = {[0 ... DECT_CH_NUM+SLIC_CH_NUM-1] = 0};
//#endif

static int pcm_ch_for_DAA[ CON_CH_NUM ];

static void pcm_ch_attribute_init(void)
{
	int cch;
	
	for( cch = 0; cch < CON_CH_NUM; cch ++ ) {
		pcm_ch_for_DAA[ cch ] = 
				( get_snd_type_cch( cch ) == SND_TYPE_DAA ? 1 : 0 );
	}
	
#if 0
//	for (ch = 0; ch < SLIC_CH_NUM + DECT_CH_NUM; ch++)
//	{
//		pcm_ch_for_DAA[ch] = 0;	// this pcm channel is for SLIC(FXS).
//		//printk("pcm_ch_for_DAA[%d] = %d\n", ch, pcm_ch_for_DAA[ch]);
//	}
//
//	for (/*ch = SLIC_CH_NUM + DECT_CH_NUM*/; ch < VOIP_CH_NUM; ch++)
//	{
//		pcm_ch_for_DAA[ch] = 1;	// this pcm channel is for DAA(FXO).
//		//printk("pcm_ch_for_DAA[%d] = %d\n", ch, pcm_ch_for_DAA[ch]);
//	}
#endif
}

int Is_DAA_Channel(int chid)
{
	if( chid >= CON_CH_NUM )
		return 0;
		
	return pcm_ch_for_DAA[chid];
}

/****************************************************************/
static int __init rtk_voip_mgr_init_module(void)
{
	extern int rtk_voip_dsp_init(void);

	printk("=============RTK VOIP SUITE=============\n");
#ifdef CONFIG_VOIP_SDK
	printk("SDK VoIP Version: %s \n", VOIP_VERSION);
#else
	printk("RTK VoIP Version: %s \n", VOIP_VERSION);
#endif
    printk("Board CFG Model : %s \n", CONFIG_BOARD_CONFIG_MODEL);

  printk("INITIAL VOIP MANAGER PROGRAM\n");
  //printk("===========================================\n");

	pcm_ch_attribute_init();

#ifdef CONFIG_RTK_VOIP_MODULE
	pcmctrl_init();
	rtk_trap_init_module();
#endif
#if ! defined (AUDIOCODES_VOIP)
#if ! defined( CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST )
	// standalone or dsp 
	rtk_voip_dsp_init();
	dsp_init_first = 1;
#endif
#endif
	
	return 0;
}

/****************************************************************/
static void __exit rtk_voip_mgr_cleanup_module(void)
{
#if ! defined (AUDIOCODES_VOIP)
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
	extern void rtk_voip_dsp_exit(void);
#endif
#endif
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	//extern void voip_mgr_help_host_exit(void);
#endif

	printk("=============RTK VOIP SUITE============\n");
	printk("Remove VOIP MANAGER PROGRAM\n");
	printk("===========================================\n");
	
#if ! defined (AUDIOCODES_VOIP)
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
	rtk_voip_dsp_exit();
#endif
#endif

#ifdef CONFIG_RTK_VOIP_MODULE
	rtk_trap_cleanup_module();
	pcmctrl_cleanup();
#endif

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	//voip_mgr_help_host_exit();
#endif
}

/****************************************************************/
voip_initcall(rtk_voip_mgr_init_module);
voip_exitcall(rtk_voip_mgr_cleanup_module);

