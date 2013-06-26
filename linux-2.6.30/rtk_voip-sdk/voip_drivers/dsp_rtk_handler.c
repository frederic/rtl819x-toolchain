#include <linux/string.h>
#include "rtk_voip.h"
#include "voip_types.h"
#include "mem.h"

#include "codec_descriptor.h"	// VoipChannelSampleRate[], SAMPLE_WIDE_BAND

#include "v152_api.h"


#ifdef T38_STAND_ALONE_HANDLER
#include "t38_handler.h"
#endif

#include "con_bus_handler.h"
#include "con_register.h"
#include "dsp_rtk_define.h"
#include "dsp_rtk_mux.h"
#include "dsp_main.h"

#ifdef REDUCE_PCM_FIFO_MEMCPY
uint32* pRxBufTmp; // For reducing memcpy
uint32* pTxBufTmp;
#else
uint32 RxBufTmp[MAX_DSP_RTK_CH_NUM][RX_BUF_SIZE/4];
uint32 TxBufTmp[MAX_DSP_RTK_CH_NUM][TX_BUF_SIZE/4];
#endif

#ifdef PCM_LOOP_MODE_DRIVER
#define LOOP_FIFO_SIZE	6
uint32 RxLoopBuf[MAX_DSP_RTK_CH_NUM][LOOP_FIFO_SIZE*RX_BUF_SIZE/4*MAX_BAND_FACTOR];
static uint32 rx_loop_r[MAX_DSP_RTK_CH_NUM]={0}, rx_loop_w[MAX_DSP_RTK_CH_NUM]={0};
uint32 TxLoopBuf[MAX_DSP_RTK_CH_NUM][LOOP_FIFO_SIZE*RX_BUF_SIZE/4*MAX_BAND_FACTOR];
static uint32 tx_loop_r[MAX_DSP_RTK_CH_NUM]={0}, tx_loop_w[MAX_DSP_RTK_CH_NUM]={0};
#endif

#ifdef PCM_LOOP_MODE_DRIVER
#define TX_NO_TONE_FIFO_SIZE	6
static uint32 TxNoToneBuf[ MAX_DSP_RTK_CH_NUM ][TX_NO_TONE_FIFO_SIZE*RX_BUF_SIZE/4*MAX_BAND_FACTOR];
static uint32 tx_no_tone_r[ MAX_DSP_RTK_CH_NUM ], tx_no_tone_w[ MAX_DSP_RTK_CH_NUM ];
#endif

#ifdef PCM_LOOP_MODE_DRIVER
typedef struct 
{
	int mode;	//0: Not loop mode, 1: loop mode, 2: loop mode with VoIP 
	int main_ch;
	int mate_ch;
}
stPcmLoopMode;

typedef struct 
{
	int group;	// channel is in gruop "group", if group <0, means not in pcm loop mode
	int ch_role;	// channel role is 1: main, 2:mate, 0 means not in pcm loop mode
}
stPcmLoopModeChState;

extern int pcm_set_LoopMode(unsigned char group, unsigned int mode, unsigned char main_chid, unsigned char mate_chid);
extern int pcm_get_LoopMode(unsigned char group);
extern stPcmLoopModeChState pcm_get_If_In_LoopMode(unsigned char chid);
extern int pcm_check_If_In_LoopMode(unsigned char chid);

#define MAX_PCMLOOP_GROUP	4
static stPcmLoopMode pcm_LoopMode[MAX_PCMLOOP_GROUP] = {{0}};

#endif

#if defined( NEW_TONE_ENTRY_ARCH )
#define TONE_FIFO_SIZE	6
#ifdef NEW_REMOTE_TONE_ENTRY
static uint32 RxToneBuf[ MAX_DSP_RTK_CH_NUM ][TONE_FIFO_SIZE*RX_BUF_SIZE/4*MAX_BAND_FACTOR];
static uint32 rx_tone_r[ MAX_DSP_RTK_CH_NUM ], rx_tone_w[ MAX_DSP_RTK_CH_NUM ];
static uint32 rx_tone_sid[ MAX_DSP_RTK_CH_NUM ];
#endif
#ifdef NEW_LOCAL_TONE_ENTRY
static uint32 TxToneBuf[ MAX_DSP_RTK_CH_NUM ][TONE_FIFO_SIZE*TX_BUF_SIZE/4*MAX_BAND_FACTOR];
static uint32 tx_tone_r[ MAX_DSP_RTK_CH_NUM ], tx_tone_w[ MAX_DSP_RTK_CH_NUM ];
static uint32 tx_tone_sid[ MAX_DSP_RTK_CH_NUM ];
#endif
#endif

#define bus_handler( dch )		PCM_handler( dch )		// to fit older codeing 
extern int32 PCM_handler(unsigned int chid);

#if defined( SUPPORT_V152_VBD ) && !defined( AUDIOCODES_VOIP )
void V152_CheckFaxDetectFlagAndSwitchCodec( void )
{
	int sid;
	extern int CED_routine_BySid( uint32 ssid );
	
	for( sid = 0; sid < dsp_rtk_ss_num; sid ++ ) {
		if( CED_routine_BySid( sid ) == 1 ) {
			V152_StateTransition( sid, REASON_SIG_VBD_CED );
		} 
		
		//if( V152_CheckBidirectionalSilence( sid ) ) {
		//	V152_StateTransition( sid, REASON_VOC_BI_SILENCE );
		//}
	}
	
	V152_SwitchCodecIfNecessary();
}
#endif	// SUPPORT_V152_VBD && !AUDIOCODES_VOIP


static inline int check_ready_to_get_rx( uint32 dch, 
								voip_con_t * const p_con )
{
#ifdef DTMF_REMOVAL_FORWARD	
	extern unsigned char dtmf_removal_flag[];
#endif

	const int rx_fifo_size = 
			p_con ->con_ops ->dsp_read_bus_rx_get_fifo_size( p_con );
	
#ifdef DTMF_REMOVAL_FORWARD	
	/* 
	 * Forward remove DTMF_REMOVAL_FORWARD_SIZE*10 ms. 
	 * The larger, DTMF removal more clean, but longer delay.
	 */
	if (dtmf_removal_flag[ dch ] == 1) // RFC2833, SIP_INFO, DTMF delete
	{ 
		//if (pcm_get_rx_PCM_FIFO_SIZE(chid) > DTMF_REMOVAL_FORWARD_SIZE) 
		if( rx_fifo_size > DTMF_REMOVAL_FORWARD_SIZE )
		{
			return 1;
		}
		else
			return 0;
	}	
#endif //DTMF_REMOVAL_FORWARD

	if( rx_fifo_size > 0 )
		return 1;
	
	return 0;
}


#ifdef PCM_LOOP_MODE_DRIVER


int pcm_set_LoopMode(unsigned char group, unsigned int mode, unsigned char main_chid, unsigned char mate_chid)
{
	int i, j = 0;
	if (mode==0)
	{
		PRINT_MSG("Set Loop Mode=0 for group%d\n", group);
		pcm_LoopMode[group].mode = mode;	//0: Not loop mode, 1: loop mode, 2: loop mode with VoIP
		return 1;
	}
	
	if ((mode != 0) && (pcm_LoopMode[group].mode != 0))
	{
		PRINT_R("Wrong Setting for PCM Loop Mode. Group%d has been set to mode%d\n", group, pcm_LoopMode[group].mode);
		return 0;
	}

	if (main_chid == mate_chid)
	{
		PRINT_R("Wrong Setting for PCM Loop Mode. main_chid= %d, mate_chid= %d\n", main_chid, mate_chid);
		return 0;
	}

	//if (pcm_LoopMode[group].mode != mode)
	{
		PRINT_MSG("Enter Loop Mode=%d, ch= %d, %d for group%d\n", mode, main_chid, mate_chid, group);
		pcm_LoopMode[group].mode = mode;
		pcm_LoopMode[group].main_ch = main_chid;
		pcm_LoopMode[group].mate_ch = mate_chid;
		
		for(i=0; i<2; i++)
		{
			if (i ==0)
				j = main_chid;
			else if (i ==1)
				j = mate_chid;
			
			rx_loop_r[j]=0;
			rx_loop_w[j]=0;
			tx_loop_r[j]=0;
			tx_loop_w[j]=0;
		}
	}
	
	return 1;

}

int pcm_get_LoopMode(unsigned char group)
{
	return pcm_LoopMode[group].mode;
}

stPcmLoopModeChState pcm_get_If_In_LoopMode(unsigned char chid)
{
	int i;
	stPcmLoopModeChState stat;
	
	for (i = 0; i < MAX_PCMLOOP_GROUP; i++)
	{
		if (pcm_LoopMode[i].mode != 0)
		{
			stat.group = i;
			//PRINT_Y("G=%d\n", i);
			
			if (pcm_LoopMode[i].main_ch == chid)
			{
				stat.ch_role = 1; /* chid is main channel */
				//PRINT_Y("1");
				return stat;
			}
			else if (pcm_LoopMode[i].mate_ch == chid)
			{
				stat.ch_role = 2; /* chid is mate channel */
				//PRINT_Y("2");
				return stat;
			}
		}
	}
	
	/* This channle is not in pcm loop mode */
	stat.group = -1;
	stat.ch_role = 0;
	
	return stat;
}

int pcm_check_If_In_LoopMode(unsigned char chid)
{
	int i;
	
	for (i = 0; i < MAX_PCMLOOP_GROUP; i++)
	{
		if (pcm_LoopMode[i].mode != 0)
		{
			if (pcm_LoopMode[i].main_ch == chid)
			{
				return 1;
			}
			else if (pcm_LoopMode[i].mate_ch == chid)
			{
				return 1;
			}
		}
	}
	
	/* This channle is not in pcm loop mode */
	return 0;
}

#endif

uint32 * GetTxNoToneWritingBaseAddr( uint32 chid )
{
#ifdef PCM_LOOP_MODE_DRIVER
	if (((tx_no_tone_w[chid]+1)%TX_NO_TONE_FIFO_SIZE) == tx_no_tone_r[chid])
	{
		// Full
		//printk("*");
		return NULL;
	}
	
	return &TxNoToneBuf[chid][tx_no_tone_w[chid] * RX_BUF_SIZE/4];
#else
	return NULL;
#endif
}

void TxNoToneBufferWritingDone( uint32 chid )
{
#ifdef PCM_LOOP_MODE_DRIVER
	tx_no_tone_w[chid]=(tx_no_tone_w[chid]+1 )%TX_NO_TONE_FIFO_SIZE;
#endif
}

uint32 * GetTxNoToneReadingBaseAddr( uint32 chid )
{
#ifdef PCM_LOOP_MODE_DRIVER
	if (tx_no_tone_r[chid] == tx_no_tone_w[chid])
	{
		// Full
		printk(".");
		return NULL;
	}
	
	return &TxNoToneBuf[chid][tx_no_tone_r[chid] * RX_BUF_SIZE/4];
#else
	return NULL;
#endif
}

void TxNoToneBufferReadingDone( uint32 chid )
{
#ifdef PCM_LOOP_MODE_DRIVER
	tx_no_tone_r[chid]=(tx_no_tone_r[chid]+1 )%TX_NO_TONE_FIFO_SIZE;
#endif
}

#if defined( NEW_TONE_ENTRY_ARCH ) 
static inline uint32 * GetToneWritingBaseAddr_Core( uint32 chid, uint32 sid,
		uint32 ToneBuf[][TONE_FIFO_SIZE*RX_BUF_SIZE/4*MAX_BAND_FACTOR],
		uint32 tone_r[], uint32 tone_w[],
		uint32 tone_sid[] )
{
	CT_ASSERT( RX_BUF_SIZE == TX_BUF_SIZE );	// make sure the two are the same 
	
	uint32 next_wi;
	
	if( tone_w[ chid ] == tone_r[ chid ] )
		tone_sid[ chid ] = sid;	/* fifo is empty, so accept this sid */
	else if( tone_sid[ chid ] == sid )
		;	/* fifo is not empty, so accept identical sid only */
	else {
		printk( "tone sid=%d,%d\n", tone_sid[ chid ], sid );	/* fifo is full, or sid not match */
		return NULL;
	}
	
	next_wi = ( tone_w[ chid ] + 1 ) % TONE_FIFO_SIZE;
	
	if( next_wi == tone_r[ chid ] ) {
		printk( "tF(%d) ", chid );
		return NULL;
	}
	
	return &ToneBuf[ chid ][tone_w[ chid ] * RX_BUF_SIZE/4 * MAX_BAND_FACTOR ];
}

#ifdef NEW_REMOTE_TONE_ENTRY
uint32 * GetRxToneWritingBaseAddr( uint32 chid, uint32 sid )
{
	return GetToneWritingBaseAddr_Core( chid, sid,
			RxToneBuf,
			rx_tone_r, rx_tone_w,
			rx_tone_sid );
}
#endif

#ifdef NEW_LOCAL_TONE_ENTRY
uint32 * GetTxToneWritingBaseAddr( uint32 chid, uint32 sid )
{
	return GetToneWritingBaseAddr_Core( chid, sid,
			TxToneBuf,
			tx_tone_r, tx_tone_w,
			tx_tone_sid );
}
#endif

static inline void ToneBufferWritingDone_Core( uint32 chid, 
			uint32 tone_w[] )
{
	tone_w[ chid ] = ( tone_w[ chid ] + 1 ) % TONE_FIFO_SIZE;
}

#ifdef NEW_REMOTE_TONE_ENTRY
void RxToneBufferWritingDone( uint32 chid )
{
	ToneBufferWritingDone_Core( chid, rx_tone_w );
}
#endif

#ifdef NEW_LOCAL_TONE_ENTRY
void TxToneBufferWritingDone( uint32 chid )
{
	ToneBufferWritingDone_Core( chid, tx_tone_w );
}
#endif

#define __asm__ asm
#define ASM __asm__ volatile

#define add_inline(var1,var2)	\
({	\
	long __result;	\
	ASM ("addr2.s	$15, %0, %1" : /* no outputs */ : "d" ((short)(var1)), "d" ((short)(var2)):"$15");	\
	ASM ("sll		$15, $15, 16" : /* no outputs */ : /* no inputs */ :"$15");	\
	ASM ("sra		%0, $15, 16" : "=d" ((long)__result) : /* no inputs */:"$15");	\
	__result; \
})

typedef enum {
	MIX_TYPE_ADD, 
	MIX_TYPE_ASSIGN,
} mix_type_t;

static inline void MixToneBuffer_Core( uint32 chid, uint32 *pBuffer,
	uint32 ToneBuf[][TONE_FIFO_SIZE*RX_BUF_SIZE/4*MAX_BAND_FACTOR],
	uint32 tone_r[], uint32 tone_w[],
	uint32 tone_sid[] )
{
	uint32 i;
	uint16 *pToneBuf;
	uint16 *pMixTarget = ( uint16 * )pBuffer;
#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
	const uint32 band_factor = ( VoipChannelSampleRate[chid] == SAMPLE_WIDE_BAND ? 2 : 1 );
	const uint32 samples = band_factor * RX_BUF_SIZE / 2;
#else
	const uint32 band_factor = 1;
	const uint32 samples = RX_BUF_SIZE / 2;
#endif
	uint32 sid_tone;

#ifdef CONFIG_RTK_VOIP_IP_PHONE
	mix_type_t mix_type = MIX_TYPE_ADD;
#else
	mix_type_t mix_type;
#endif

#ifndef CONFIG_RTK_VOIP_IP_PHONE
	extern Flag fWait[];
	extern char fsk_cid_state[];
	extern int outband_dmtf_play_state[];
#endif

	CT_ASSERT( RX_BUF_SIZE == TX_BUF_SIZE );	// make sure the two are the same 
	
	if( tone_r[ chid ] == tone_w[ chid ] ) {
		//printk( "tE(%d) ", chid );
		return;
	}

#ifndef CONFIG_RTK_VOIP_IP_PHONE
	// decide mix type 	
	sid_tone = tone_sid[ chid ];
	
	if( fWait[sid_tone] == 0 ) {
		if( (1==fsk_cid_state[chid]) || (1==outband_dmtf_play_state[sid_tone]) ) {
			mix_type = MIX_TYPE_ASSIGN;
		} else
			mix_type = MIX_TYPE_ADD;
	} else 
		mix_type = MIX_TYPE_ASSIGN;
#endif
		
	pToneBuf = (uint16 *)&ToneBuf[ chid ][ tone_r[ chid ] * RX_BUF_SIZE/4 * MAX_BAND_FACTOR ];

	if( mix_type == MIX_TYPE_ADD ) {
		for( i = 0; i < samples; i ++ ) {
			*pMixTarget = add_inline( *pMixTarget, *pToneBuf );	// add
			pMixTarget ++;
			pToneBuf ++;
		}
	} else {
		for( i = 0; i < samples; i ++ ) {
			*pMixTarget = *pToneBuf;	// assign 
			pMixTarget ++;
			pToneBuf ++;
		}
	}

	tone_r[ chid ] = ( tone_r[ chid ] + 1 ) % TONE_FIFO_SIZE;
}

#ifdef NEW_REMOTE_TONE_ENTRY
void MixRxToneBuffer( uint32 chid, uint32 *pRxBuffer )
{
	MixToneBuffer_Core( chid, pRxBuffer,
			RxToneBuf, 
			rx_tone_r, rx_tone_w,
			rx_tone_sid );
}
#endif

#ifdef NEW_LOCAL_TONE_ENTRY
void MixTxToneBuffer( uint32 chid, uint32 *pTxBuffer )
{
	MixToneBuffer_Core( chid, pTxBuffer,
			TxToneBuf,
			tx_tone_r, tx_tone_w,
			tx_tone_sid );
}
#endif
#endif /* NEW_TONE_ENTRY_ARCH */

#if defined (CONFIG_RTK_VOIP_DRIVERS_PCM8672)
__IRAM void dsp_rtk_bus_handler( void )
#else
void dsp_rtk_bus_handler( void )
#endif
{
	unsigned int dch = 0;
	static unsigned int last_dch = 0;
	unsigned int f_cnt;
	unsigned int i;
	extern Word16 add (Word16 var1,Word16 var2);

#ifdef SUPPORT_V152_VBD
	V152_CheckFaxDetectFlagAndSwitchCodec();
#endif

	dch = (++last_dch)%DSP_RTK_CH_NUM;
	//for (f_cnt = 0; f_cnt < (VOIP_CH_NUM*PCM_FIFO_SIZE*2); f_cnt++, dch = (++dch)%VOIP_CH_NUM)
	for (f_cnt = 0; f_cnt < (DSP_RTK_CH_NUM*PCM_PERIOD*2); f_cnt++, dch = (dch+1)%DSP_RTK_CH_NUM)
	{
		voip_dsp_t * const p_dsp = get_voip_rtk_dsp( dch );
		voip_con_t * const p_con = p_dsp ->con_ptr;
		
		//if (chanEnabled[dch] == 0)
		//	continue;	
		if( !p_dsp ->enabled )
			continue;
			
#if defined (AUDIOCODES_VOIP)			
		for(i=0; i < ACMW_MAX_NUM_CH; i++)
		{
			transiverBuff[i].TxBuff = NULL;
			transiverBuff[i].RxBuff = NULL;
		}
#endif			
				
				

		//printk("2: %d-%X\n", pcm_get_read_rx_fifo_addr(dch, &pRxBufTmp), pRxBufTmp);
//#ifdef REDUCE_PCM_FIFO_MEMCPY
//		if( pcm_get_read_rx_fifo_addr(dch, (void*)&pRxBufTmp))
//#else
//		if( pcm_read_rx_fifo(dch, (void*)&RxBufTmp[dch][0]))
//#endif			
//		{
//			break;
//		}
//		else if (pcm_get_write_tx_fifo_addr(dch,(void**) &pTxBufTmp))
//		{
//			break;
//		}
#ifdef REDUCE_PCM_FIFO_MEMCPY		
		if( ( pRxBufTmp = ( uint32* )
			p_con ->con_ops ->dsp_read_bus_rx_get_addr( p_con ) ) 
			== NULL )
		{
			continue;
		} else if( !check_ready_to_get_rx( dch, p_con ) ||
				( ( pTxBufTmp = ( uint32* )
					p_con ->con_ops ->dsp_write_bus_tx_get_addr( p_con ) )
					== NULL ) )
		{
			continue;
		}
#else
		???
#endif

#ifdef NEW_REMOTE_TONE_ENTRY
		MixRxToneBuffer( dch, pRxBufTmp );
#endif
	
#ifdef T38_STAND_ALONE_HANDLER
		if( t38RunningState[ dch ] == T38_START )
			PCM_handler_T38( dch );
		else
#endif	 
		{
			
#if defined (AUDIOCODES_VOIP)
			pcm_get_read_rx_fifo_addr(dch, (void*)&transiverBuff[dch].RxBuff);
			pcm_get_write_tx_fifo_addr(dch, (void*)&transiverBuff[dch].TxBuff);
			
			#ifdef FEATURE_COP3_PCM_HANDLER
			ProfileEnterPoint(PROFILE_INDEX_PCM_HANDLER);
			#endif
			//activate AC MiddelWare		
			ACMWPcmProcess( &transiverBuff[0], ACMW_MAX_NUM_CH, PCM_10MSLEN_INSHORT );
			
			#ifdef FEATURE_COP3_PCM_HANDLER
			ProfileExitPoint(PROFILE_INDEX_PCM_HANDLER);
			ProfilePerDump(PROFILE_INDEX_PCM_HANDLER, cp3_voip_param.cp3_dump_period);
			#endif
			
			if(transiverBuff[dch].TxBuff != NULL)
				pcm_write_tx_fifo_done(dch);

#else

#ifdef PCM_LOOP_MODE_DRIVER
			unsigned int g;
			stPcmLoopModeChState loop_st;
			
			loop_st = pcm_get_If_In_LoopMode(dch);
			if (loop_st.group >= 0)
			{
				g = loop_st.group;
				//PRINT_R("%d", g);
			}
			else
				goto NO_PCM_LOOP;
				
			
			if ( pcm_LoopMode[g].mode == 1)
			{
				if (dch == pcm_LoopMode[g].main_ch )
				{
					if (((rx_loop_w[pcm_LoopMode[g].mate_ch]+1)%LOOP_FIFO_SIZE) == rx_loop_r[pcm_LoopMode[g].mate_ch])
					{
						//printk("LF(%d)\n", pcm_LoopMode[g].mate_ch);
					}
					else
					{
						memcpy(&RxLoopBuf[pcm_LoopMode[g].mate_ch][rx_loop_w[pcm_LoopMode[g].mate_ch]*(RX_BUF_SIZE>>2)], pRxBufTmp, RX_BUF_SIZE);
						rx_loop_w[pcm_LoopMode[g].mate_ch] = (rx_loop_w[pcm_LoopMode[g].mate_ch] + 1)%LOOP_FIFO_SIZE;
					}
					
					bus_handler(dch);
					
					if (rx_loop_r[pcm_LoopMode[g].main_ch] == rx_loop_w[pcm_LoopMode[g].main_ch])
					{
						//printk("LE(%d)\n", pcm_LoopMode[g].main_ch);
						memset(pTxBufTmp, 0, RX_BUF_SIZE);
					}
					else
					{
						for (i=0; i<(RX_BUF_SIZE>>1); i++)
							*((short*)pTxBufTmp+i) = add(*((short*)pTxBufTmp+i), *((short*)(&RxLoopBuf[pcm_LoopMode[g].main_ch][rx_loop_r[pcm_LoopMode[g].main_ch]*(RX_BUF_SIZE>>2)])+i));
						rx_loop_r[pcm_LoopMode[g].main_ch] = (rx_loop_r[pcm_LoopMode[g].main_ch] + 1)%LOOP_FIFO_SIZE;
					}
				}
				else if (dch == pcm_LoopMode[g].mate_ch)
				{
					if (((rx_loop_w[pcm_LoopMode[g].main_ch]+1)%LOOP_FIFO_SIZE) == rx_loop_r[pcm_LoopMode[g].main_ch])
					{
						//printk("LF(%d)\n", pcm_LoopMode[g].main_ch);
					}
					else
					{
						memcpy(&RxLoopBuf[pcm_LoopMode[g].main_ch][rx_loop_w[pcm_LoopMode[g].main_ch]*(RX_BUF_SIZE>>2)], pRxBufTmp, RX_BUF_SIZE);
						rx_loop_w[pcm_LoopMode[g].main_ch] = (rx_loop_w[pcm_LoopMode[g].main_ch] + 1)%LOOP_FIFO_SIZE;
					}
				
					bus_handler(dch);
					
					if (rx_loop_r[pcm_LoopMode[g].mate_ch] == rx_loop_w[pcm_LoopMode[g].mate_ch])
					{
						//printk("LE(%d)\n", pcm_LoopMode[g].mate_ch);
						memset(pTxBufTmp, 0, RX_BUF_SIZE);
					}
					else
					{
						for (i=0; i<(RX_BUF_SIZE>>1); i++)
							*((short*)pTxBufTmp+i) = add(*((short*)pTxBufTmp+i), *((short*)(&RxLoopBuf[pcm_LoopMode[g].mate_ch][rx_loop_r[pcm_LoopMode[g].mate_ch]*(RX_BUF_SIZE>>2)])+i));
						rx_loop_r[pcm_LoopMode[g].mate_ch] = (rx_loop_r[pcm_LoopMode[g].mate_ch] + 1)%LOOP_FIFO_SIZE;
					}
				}
				else
					bus_handler(dch);
				
			}
			else if ( pcm_LoopMode[g].mode == 2)
			{
				if (dch == pcm_LoopMode[g].main_ch )
				{
					// Step 1 : Main Rx to Main RxLoopBuf(w)
					if (((rx_loop_w[pcm_LoopMode[g].main_ch]+1)%LOOP_FIFO_SIZE) == rx_loop_r[pcm_LoopMode[g].main_ch])
					{
						//printk("LF(%d)\n", pcm_LoopMode[g].mate_ch);
						//printk("1");
					}
					else
					{
						memcpy(&RxLoopBuf[pcm_LoopMode[g].main_ch][rx_loop_w[pcm_LoopMode[g].main_ch]*(RX_BUF_SIZE>>2)], pRxBufTmp, RX_BUF_SIZE);
						rx_loop_w[pcm_LoopMode[g].main_ch] = (rx_loop_w[pcm_LoopMode[g].main_ch] + 1)%LOOP_FIFO_SIZE;
					}

					
					// Step 2: Main Rx = Main Rx + Mate RxLoopBuf(r)
					if (rx_loop_r[pcm_LoopMode[g].mate_ch] == rx_loop_w[pcm_LoopMode[g].mate_ch])
					{
						// Mate RE
						//printk("2");
					}
					else
					{
						for (i=0; i<(RX_BUF_SIZE>>1); i++)
							*((short*)pRxBufTmp+i) = add(*((short*)pRxBufTmp+i), *((short*)(&RxLoopBuf[pcm_LoopMode[g].mate_ch][rx_loop_r[pcm_LoopMode[g].mate_ch]*(RX_BUF_SIZE>>2)])+i));
					}
					
					// Step 3:
					bus_handler(dch);
					
					// Step 4: Main Tx(without add tone) to Main TxLoopBuf(w)
					if (((tx_loop_w[pcm_LoopMode[g].main_ch]+1)%LOOP_FIFO_SIZE) == tx_loop_r[pcm_LoopMode[g].main_ch])
					{
						//printk("LF(%d)\n", pcm_LoopMode[g].mate_ch);
						//printk("3");
					}
					else
					{
						Word16 *pTxNoToneBufTmp;
						pTxNoToneBufTmp = (Word16*)GetTxNoToneReadingBaseAddr(pcm_LoopMode[g].main_ch);

						if ( pTxNoToneBufTmp != NULL)
						{
							memcpy(&TxLoopBuf[pcm_LoopMode[g].main_ch][tx_loop_w[pcm_LoopMode[g].main_ch]*(RX_BUF_SIZE>>2)], pTxNoToneBufTmp, RX_BUF_SIZE);
							tx_loop_w[pcm_LoopMode[g].main_ch] = (tx_loop_w[pcm_LoopMode[g].main_ch] + 1)%LOOP_FIFO_SIZE;
							TxNoToneBufferReadingDone(pcm_LoopMode[g].main_ch);
						}
					}
					
					// Step 5: Main Tx = Main Tx + Mate RxLoopBuf(r)
					if (rx_loop_r[pcm_LoopMode[g].mate_ch] == rx_loop_w[pcm_LoopMode[g].mate_ch])
					{
						// Mate RE
						//printk("4");
					}
					else
					{
						for (i=0; i<(RX_BUF_SIZE>>1); i++)
							*((short*)pTxBufTmp+i) = add(*((short*)pTxBufTmp+i), *((short*)(&RxLoopBuf[pcm_LoopMode[g].mate_ch][rx_loop_r[pcm_LoopMode[g].mate_ch]*(RX_BUF_SIZE>>2)])+i));
						rx_loop_r[pcm_LoopMode[g].mate_ch] = (rx_loop_r[pcm_LoopMode[g].mate_ch]+1)%LOOP_FIFO_SIZE;
					}
				}
				else if (dch == pcm_LoopMode[g].mate_ch)
				{
					// Step 1 : Mate Rx to Mate RxLoopBuf(w)
					if (((rx_loop_w[pcm_LoopMode[g].mate_ch]+1)%LOOP_FIFO_SIZE) == rx_loop_r[pcm_LoopMode[g].mate_ch])
					{
						//printk("LF(%d)\n", pcm_LoopMode[g].mate_ch);
						//printk("5");
					}
					else
					{
						memcpy(&RxLoopBuf[pcm_LoopMode[g].mate_ch][rx_loop_w[pcm_LoopMode[g].mate_ch]*(RX_BUF_SIZE>>2)], pRxBufTmp, RX_BUF_SIZE);
						rx_loop_w[pcm_LoopMode[g].mate_ch] = (rx_loop_w[pcm_LoopMode[g].mate_ch] + 1)%LOOP_FIFO_SIZE;
					}

								
					// Step 2:
					bus_handler(dch);
					
					// Step 3: Mate Tx = Mate Tx + Main RxLoopBuf(r) + Main TxLoopBuf(r)
					if (rx_loop_r[pcm_LoopMode[g].main_ch] == rx_loop_w[pcm_LoopMode[g].main_ch])
					{
						// Mate RE
						//printk("6");
					}
					else
					{
						for (i=0; i<(RX_BUF_SIZE>>1); i++)
							*((short*)pTxBufTmp+i) = add(*((short*)pTxBufTmp+i), *((short*)(&RxLoopBuf[pcm_LoopMode[g].main_ch][rx_loop_r[pcm_LoopMode[g].main_ch]*(RX_BUF_SIZE>>2)])+i));
						rx_loop_r[pcm_LoopMode[g].main_ch] = (rx_loop_r[pcm_LoopMode[g].main_ch] + 1)%LOOP_FIFO_SIZE;
					}
					
					if (tx_loop_r[pcm_LoopMode[g].main_ch] == tx_loop_w[pcm_LoopMode[g].main_ch])
					{
						// Mate RE
						//printk("7");
					}
					else
					{
						for (i=0; i<(RX_BUF_SIZE>>1); i++)
							*((short*)pTxBufTmp+i) = add(*((short*)pTxBufTmp+i), *((short*)(&TxLoopBuf[pcm_LoopMode[g].main_ch][tx_loop_r[pcm_LoopMode[g].main_ch]*(RX_BUF_SIZE>>2)])+i));
						tx_loop_r[pcm_LoopMode[g].main_ch] = (tx_loop_r[pcm_LoopMode[g].main_ch] + 1)%LOOP_FIFO_SIZE;
					}
				}
				else
					bus_handler(dch);
			}
			else if ( pcm_LoopMode[g].mode == 0)
			{
NO_PCM_LOOP:
				bus_handler(dch);
			}
			
		}
#else

			bus_handler(dch);
#endif // PCM_LOOP_MODE_DRIVER

#endif // AUDIOCODES_VOIP


#ifdef NEW_LOCAL_TONE_ENTRY
		MixTxToneBuffer( dch, pTxBufTmp );
#endif

#ifdef VOICE_GAIN_ADJUST_TONE_VOICE
#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
		const uint32 band_factor = ( VoipChannelSampleRate[dch] == SAMPLE_WIDE_BAND ? 2 : 1 );
		const uint32 samples = band_factor * RX_BUF_SIZE / 2;
#else
		const uint32 band_factor = 1;
		const uint32 samples = RX_BUF_SIZE / 2;
#endif
		extern long voice_gain_spk[];//0 is mute, 1 is -31dB ~~~ 32 is 0dB , 33 is 1dB ~~~ 63 is 31dB
 		extern void voice_gain(int16_t * pBuf, uint32_t data_number, int32_t voicegain);
		voice_gain( pTxBufTmp, samples, voice_gain_spk[dch]);
#endif //ifdef VOICE_GAIN_ADJUST_TONE_VOICE
		
#ifdef REDUCE_PCM_FIFO_MEMCPY
		//pcm_read_rx_fifo_done(dch);
		p_con ->con_ops ->dsp_read_bus_rx_done( p_con );
		
		// move from PCM_Handler 
		p_con ->con_ops ->dsp_write_bus_tx_done( p_con );
#endif			
	
		last_dch = dch;
	} // for
	
#ifdef PCM_HANDLER_USE_TASKLET	
	for ( dch = 0; dch < DSP_RTK_CH_NUM; dch ++ )
	{
		voip_dsp_t * const p_dsp = get_voip_rtk_dsp( dch );
		voip_con_t * const p_con = p_dsp ->con_ptr;
		
		//if( p_con_ops == NULL )
		//	continue;
		
		//if( !chanEnabled[ dch ] )
		//	continue;
		if( !p_dsp ->enabled )
			continue;
		
		if( check_ready_to_get_rx( dch, p_con ) ) {
			dsp_schedule_bus_handler();
			break;		
		}
	}
#endif

}

static inline void rtk_dsp_10ms_timer_do_session( voip_dsp_t * p_dsp, uint32 session )
{
#ifdef SUPPORT_RTCP
	extern unsigned char RtcpOpen[MAX_DSP_RTK_SS_NUM];
	extern void RtpSession_processRTCP (uint32 sid);
	
	//printk("--- RtcpOpen[%d] = %d --- \n", session, RtcpOpen[session]);
	if(RtcpOpen[session] == 1)	//thlin: if RtcpOpen =1 , do RtpSession_processRTCP, eles NOT.
	{
		RtpSession_processRTCP(session);
	}
#endif

}

static inline void rtk_dsp_10ms_timer_do_channel( voip_dsp_t * p_dsp )
{

}

static inline void _rtk_dsp_10ms_timer( voip_dsp_t * p_dsp )
{
	const uint32 dch = p_dsp ->dch;
	uint32 sid, ssid;
	int isConference;
	uint32 pSessRank[2];
	int i;
	uint32 SessNum;
	
	// process this channel 
	rtk_dsp_10ms_timer_do_channel( p_dsp );
	
	// process its session 
	sid = chanInfo_GetTranSessionID(dch);

	if(sid == SESSION_NULL)
		sid = dch;

#ifdef SUPPORT_3WAYS_AUDIOCONF
	isConference = chanInfo_IsConference( dch );

	if(isConference)
	{
		SessNum = chanInfo_GetRegSessionNum(dch);
		//pSessRank = chanInfo_GetRegSessionRank(dch);
		chanInfo_GetRegSessionRank( dch, pSessRank );
	}
	else
#endif
	{
		SessNum = 1;
	}

#ifdef SUPPORT_3WAYS_AUDIOCONF
	for(i=0; i<SessNum; i++)
#endif
	{
		if(isConference)
		{
			ssid = chanInfo_GetRegSessionID(dch, i);
			if(ssid == SESSION_NULL)
				ssid = sid;
		}
		else
			ssid = sid;
		
		rtk_dsp_10ms_timer_do_session( p_dsp, ssid );
	}
}

void rtk_dsp_10ms_timer( void )
{
	uint32 dch;
	
	for ( dch = 0; dch < DSP_RTK_CH_NUM; dch ++ )
	{
		voip_dsp_t * const p_dsp = get_voip_rtk_dsp( dch );
		
		if( !p_dsp ->enabled )
			continue;
		
		_rtk_dsp_10ms_timer( p_dsp );
	}	
}


void dsp_rtk_handler_init_var( void )
{
	// move from init_var() of pcm_interface.c 
	int i;
	
#ifdef PCM_LOOP_MODE_DRIVER
	for (i=0; i < MAX_PCMLOOP_GROUP; i++)
		pcm_set_LoopMode(i, 0, 0, 0);
#endif
}

