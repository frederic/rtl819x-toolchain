#include <linux/string.h>
#include "rtk_voip.h"
#include "mem.h"
#include "voip_types.h"
#include "voip_control.h"
#include "voip_dev.h"

//#include "dsp_main.h"
#include "cp3_profile.h"

#include "dtmf_dec.h"
#include "fskcid_gen.h"
#include "tone_det_i.h"

#include "../voip_dsp/ivr/ivr.h"
#include "codec_descriptor.h"

#include "section_def.h"

#include "con_register.h"
#include "dsp_rtk_define.h"

#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
#include "resampler.h"
#endif

#ifdef SUPPORT_LEC_G168_ISR
#include "lec.h"
#endif//SUPPORT_LEC_G168_ISR

#include "../voip_manager/voip_mgr_events.h"

static int caller_id_det[MAX_DSP_RTK_CH_NUM] = {[0 ... MAX_DSP_RTK_CH_NUM-1]=1};
static int fax_modem_det[MAX_DSP_RTK_CH_NUM] = {[0 ... MAX_DSP_RTK_CH_NUM-1]=1};
static int tone_det[MAX_DSP_RTK_CH_NUM] = {[0 ... MAX_DSP_RTK_CH_NUM-1]=1};
int clean_pcm_rx_flag[MAX_DSP_RTK_CH_NUM] = {[0 ... MAX_DSP_RTK_CH_NUM-1]=0};
const int * const ptr_fax_modem_det = fax_modem_det;

TstVoipFskT2Cid_det stVoipFskT2Cid_det[MAX_DSP_RTK_CH_NUM];


/****************** DTMF DET & REMOVAL RELATED DEFINE *************************/
#ifdef DTMF_DEC_ISR
#ifdef SUPPORT_SLIC_GAIN_CFG
extern uint32 g_txVolumneGain[MAX_DSP_CH_NUM];	// move to dsp_define.c 
extern uint32 g_rxVolumneGain[MAX_DSP_CH_NUM];	// move to dsp_define.c 
#endif
Word16 det_buff[MAX_DSP_RTK_CH_NUM][RX_BUF_SIZE/2*MAX_BAND_FACTOR];  // Word16
Dtmf_det_out dtmf_digit;
#endif
extern int16 tx_comp[];

#ifdef DAA_RX_DET
Word16 det_buff_daa[RX_BUF_SIZE/2] __attribute__((aligned(8)));
#endif

/*
 dtmf_mode = 0 : RFC2833
 dtmf_mode = 1 : INFO
 dtmf_mode = 2 : Inband
 dtmf_mode = 3 : DTMF delete
*/
char dtmf_mode[MAX_DSP_RTK_CH_NUM] = {[0 ... MAX_DSP_RTK_CH_NUM-1] = 2};

#ifdef DTMF_REMOVAL_ISR
unsigned char dtmf_removal[MAX_DSP_RTK_CH_NUM][2/*dir*/] = {[0 ... MAX_DSP_RTK_CH_NUM-1] = {0, 0}};// declare for dtmf removal, 0: disable  1: enable (value is assigned to 1 when dtmf tone is detected, otherwise, 0!)
unsigned char dtmf_removal_flag[MAX_DSP_RTK_CH_NUM] = {0};
unsigned char dtmf_removal_cnt[MAX_DSP_RTK_CH_NUM] = {[0 ... MAX_DSP_RTK_CH_NUM-1]=3};
//extern char dtmf_mode[]; /* 0:rfc2833  1: sip info  2: inband  3: delete */
extern int send_2833_by_ap[];
int clean_forward[MAX_DSP_RTK_CH_NUM] = {[0 ... MAX_DSP_RTK_CH_NUM-1]=1};
#endif

#ifdef SEND_RFC2833_ISR
#include "dsp_main.h"
extern unsigned char RtpOpen[];
extern int RtpTx_transmitEvent_ISR( uint32 chid, uint32 sid, int event);
int send_dtmf_flag[MAX_DSP_RTK_SS_NUM] = {0};
int g_digit[MAX_DSP_RTK_CH_NUM]={0};
#endif

#ifdef SUPPORT_FAX_PASS_ISR
extern unsigned char fax_offhook[]; 
extern int ced_check[];
extern char FaxFlag[];
extern char ModemFlag[];
extern void modem_det(unsigned int chid, unsigned short* page_addr);
extern void DisableDspFunctionsIfModemOrFaxIsDetected( int sid );
#endif
extern void answer_tone_det(unsigned int chid, unsigned short* page_addr, int dir);

extern unsigned char support_lec_g168[] ;	// 0: LEC disable  1: LEC enable

extern int pcm_check_If_In_LoopMode(unsigned char chid);

#ifdef SUPPORT_AES_ISR
static int aes_do_times[MAX_DSP_RTK_CH_NUM] = {0}, aes_do_cnt[MAX_DSP_RTK_CH_NUM] = {0};
static int aes_tx_fifo_size[MAX_DSP_RTK_CH_NUM] = {0};
extern unsigned char support_AES[];
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
Word16 Handfree_delaybuf[80+250];
#define T_TYPE_ECHO_CAN
#include "../voip_dsp/dsp_r1/include/lexra_radiax.h"
#endif

#ifdef VOICE_GAIN_ADJUST_IVR_TONE_VOICE
extern long voice_gain_spk[];//0 is mute, 1 is -31dB ~~~ 32 is 0dB , 33 is 1dB ~~~ 63 is 31dB
#endif

#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
// to reduce stack, we define this temporal variable. 
// Due to a temporal variable, just use it as a local variable. 
uint32 rxBuf_NB[ PCM_PERIOD_10MS_SIZE / 4 ] __attribute__((aligned(8))); 
#endif

#ifdef RTK_VOICE_RECORD
extern TstVoipdataget stVoipdataget[];
short Sin1KHz[8] = {0, 0x2D5D, 0x4027, 0x2D5C, 0, 0xD2A3, 0xBFD9, 0xD2A4};
//short Sin1KHz[8] = {0, 23109, 32681, 23108, 0, 0xA5BB, 0x8057, 0xA5BC}; // gain up 1.99x
extern unsigned long sound_c1_8k_rin5, sound_c1_8k_rin10, sound_c1_8k_rin15, sound_c1_8k_rin20, sound_c1_8k_rin30, sound_c1_8k_rin13p8;
short* ec128_pattern[] = {&sound_c1_8k_rin5, &sound_c1_8k_rin10, &sound_c1_8k_rin15, &sound_c1_8k_rin20, &sound_c1_8k_rin30, &sound_c1_8k_rin13p8};
int ec128_pattern_index;
int ec128_start_count;
#define EC128_SAMPLE_NUM 5601
#endif

#ifdef RTK_VOICE_PLAY
extern TstVoipdataput stVoipdataput[];
void voice_play_mix(int chid, Word16* poutput);
const codec_type_desc_t *FindStartCodecTypeDesc( uint32 chid );
#endif

#define D0    		0
#define D1    		1
#define D2    		2
#define D3    		3
#define D4    		4
#define D5    		5
#define D6    		6
#define D7    		7
#define D8    		8
#define D9    		9
#define DSTAR 		10  	 // DTMF *
#define DPND  		11	 // DTMF #
#define DA    		12
#define DB    		13
#define DC    		14
#define DD    		15
#define DSIL  		16	// silence
static char dtmf_conv_table[]={'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '*', '#', 'A', 'B', 'C', 'D'};

extern unsigned int ec_select;
extern unsigned int add_delayed_echo;
void AEC_g168(unsigned int chid, const int16_t *pRin, int16_t *pSin, int16_t *pEx);
//#define DELAY_ECHO_TEST 1
#if defined(DELAY_ECHO_TEST)
Word16 delayedecho_buf[MAX_DSP_RTK_CH_NUM][1024+80];
#endif

#ifdef CUSTOMIZE_DTMF_MINIMUM_ON_TIME
char dtmf_pre_stat[MAX_DSP_RTK_CH_NUM][2];
short dtmf_on_cnt_10ms[MAX_DSP_RTK_CH_NUM][2];
short dtmf_on_flag[MAX_DSP_RTK_CH_NUM][2];
#endif

// --------------------------------------------------------
// separator of declare and implement 
// --------------------------------------------------------



// isr handler 
void isr_bus_tx_start( voip_dsp_t *this )
{
}

void isr_bus_tx_process_pre( struct voip_dsp_s *this )
{	
}

void isr_bus_tx_process( voip_dsp_t *this, uint16 *pcm_tx )
{
	const uint32 dch = this ->dch;

	#ifndef LEC_G168_ISR_SYNC_P
#if defined (AUDIOCODES_VOIP)
	ACMWModemTx(chid, pcm_tx);
#endif		
	
	MixIvrSpeechWaveToPlayoutBuffer_Local( dch, ( uint32 * )pcm_tx );

#ifdef RTK_VOICE_PLAY
	if ( (stVoipdataput[dch].write_enable&2)
    #ifdef RTK_VOICE_PLAY_WAIT_CODEC_START		
		  && ( FindStartCodecTypeDesc( dch ) != NULL )
    #endif
	) {

		voice_play_mix(dch, pcm_tx);
	}
#endif

#ifndef AUDIOCODES_VOIP
	#ifdef FXO_CALLER_ID_DET
	if ( stVoipFskT2Cid_det[dch].cid_setup ==1 )
	{
		if ( stVoipFskT2Cid_det[dch].cid_states ==3 )
		{
			if (stVoipFskT2Cid_det[dch].d_play_cnt < 0)
				stVoipFskT2Cid_det[dch].d_play_cnt++;
			if (stVoipFskT2Cid_det[dch].d_play_cnt >= 0)
			{
				extern short dtmf_d_table[];
#ifdef USE_MEM64_OP
				memcpy64s(pcm_tx, &dtmf_d_table[stVoipFskT2Cid_det[dch].d_play_cnt*80], PCM_PERIOD_10MS_SIZE/2);
#else
				memcpy(pcm_tx, &dtmf_d_table[stVoipFskT2Cid_det[dch].d_play_cnt*80], PCM_PERIOD_10MS_SIZE);
#endif
				stVoipFskT2Cid_det[dch].d_play_cnt ++;

				if ( stVoipFskT2Cid_det[dch].d_play_cnt > 6 )
					stVoipFskT2Cid_det[dch].cid_states ++;
			}
		}
	
	}
	#endif		
#endif	// !AUDIOCODES_VOIP

#ifdef RTK_VOICE_RECORD
						int ii, k, i=0;
						if (stVoipdataget[dch].write_enable==2)
						{
							for (ii=0; ii < PCM_PERIOD; ii++)
							{
							for (k=0; k<80; k++)
							{
									*((short*)pcm_tx+i*(PCM_PERIOD_10MS_SIZE/2)+k) = Sin1KHz[k%8];
								}
							}
						}

						if (stVoipdataget[dch].write_enable&(32+64+128))
						{
							ec128_start_count++;
							if (ec128_start_count>200) {
								ec128_start_count=200;
								for (ii=0; ii < PCM_PERIOD; ii++)
								{
									for (k=0; k<80; k++)
									{
										*((short*)pcm_tx+i*(PCM_PERIOD_10MS_SIZE/2)+k) = ec128_pattern[((stVoipdataget[dch].write_enable&(32+64+128))>>5)-1][ec128_pattern_index++];
										if (ec128_pattern_index==EC128_SAMPLE_NUM)
											ec128_pattern_index=0;
									}
								}
							} else {
								for (ii=0; ii < PCM_PERIOD; ii++)
								{
									for (k=0; k<80; k++)
									{
										*((short*)pcm_tx+i*(PCM_PERIOD_10MS_SIZE/2)+k) = 0;
									}
									ec128_pattern_index=0;
								}
								
							}
						}
							
						if ((stVoipdataget[dch].write_enable&4) || (stVoipdataget[dch].write_enable==2))
						{
							for (ii=0; ii < PCM_PERIOD; ii++)
							{
								memcpy(&stVoipdataget[dch].txbuf[stVoipdataget[dch].tx_writeindex],pcm_tx + i*(PCM_PERIOD_10MS_SIZE/4),160);
								stVoipdataget[dch].tx_writeindex= (stVoipdataget[dch].tx_writeindex+160)%DATAGETBUFSIZE;
							}
						}
#endif //#ifdef RTK_VOICE_RECORD	


    #endif	// LEC_G168_ISR_SYNC_P


#ifdef VOICE_GAIN_ADJUST_IVR_TONE_VOICE
	voice_gain( txbuf, PCM_PERIOD*80, voice_gain_spk[ dch ]);
#endif
}

void isr_bus_tx_process_post( struct voip_dsp_s *this )
{
}

void isr_bus_rx_start( voip_dsp_t *this )
{
}

static int32 PCM_RX(uint32 chid, uint32 *rxBuf, const uint16 *lec_ref);

void isr_bus_rx_process( voip_dsp_t *this, uint16 *pcm_rx, const uint16 *lec_ref )
{
	PCM_RX( this ->dch, ( uint32 * )pcm_rx, lec_ref );
}
	

// --------------------------------------------------------
// older function   
// --------------------------------------------------------

extern const codec_type_desc_t *ppStartCodecTypeDesc[MAX_DSP_RTK_SS_NUM];



#ifdef CONFIG_VOIP_COP3_PROFILE
#include "voip_debug.h"
extern st_CP3_VoIP_param cp3_voip_param;
#endif

const codec_type_desc_t *FindStartCodecTypeDesc( uint32 chid )
{
	int j, SessNum;
	uint32 ssid;
	
	SessNum = chanInfo_GetRegSessionNum(chid);
	for (j=0; j < SessNum; j++)
	{
		ssid = chanInfo_GetRegSessionID(chid, j);

		if( ppStartCodecTypeDesc[ ssid ] )
			return ppStartCodecTypeDesc[ ssid ];
	}
	
	return NULL;
}

static int32 PCM_RX(uint32 chid, uint32 *rxBuf, const uint16 *lec_ref)
{
	const uint32 dch = chid;
	const int i=0;
	int j, SessNum;
	uint32 rx_rp, ssid;
#if defined(SUPPORT_SLIC_GAIN_CFG) || defined(SUPPORT_BANDPASS_FOR_DAA) || defined(FXO_CALLER_ID_DET) || defined(SUPPORT_AES_ISR) || defined(DELAY_ECHO_TEST)
	int ii;
	unsigned int stmp;
	Word16 s1, s2;
#endif
	uint32 *pRxFifoCurWriting;
	extern void fax_v21_dec(unsigned short* page_addr, unsigned int chid);
	extern char get_dtmf_dBFS(int chid, int dir, int reset);
#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
 	extern void NLP_g168(unsigned int chid, int16_t *pRin, int16_t *pSin, int16_t *pEx);
#endif
#ifdef T_TYPE_ECHO_CAN
	extern void highpass_process(
	  int chid,
	  short signal[],    /* input/output signal */ //align 8
	  short lg);          /* length of signal    */
#endif
 
	DECLARE_CON_FROM_RTK_DCH( dch );
	voip_snd_t * const p_snd = p_con ->snd_ptr;
	 __attribute__ ((__unused__))	// ignore pcm_ch_for_DAA unused warning 
	const int pcm_ch_for_DAA = ( p_snd ->snd_type == SND_TYPE_DAA ? 1 : 0 );

#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
	const int bWideband = ( VoipChannelSampleRate[ chid ] == SAMPLE_WIDE_BAND ? 1 : 0 );
	const int nWidebandFactor = ( bWideband ? 2 : 1 );
#else
	const int bWideband = 0;
	const int nWidebandFactor = 1;
#endif

#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
	uint32 *rxBuf_tmp;	// for loop local save only.
	//int in_length;
	//int out_length;
	//int err;
#endif

#ifdef FSK_TYPE2_ACK_CHECK
	extern unsigned int fsk_cid_type2_ack[];
	extern volatile char fsk_alert_done[];
#endif

#ifdef SEND_RFC2833_ISR  
	extern uint32 rfc2833_count_add(uint32 chid, uint32 cnt);
	extern unsigned char rfc2833_payload_type_local[];
	extern unsigned char rfc2833_payload_type_remote[];
	int send_2833_flag = 0;
	int bypass_2833_chid_limit = 0;
	static int dtmf_removal_pre[MAX_DSP_RTK_CH_NUM] = {0};
#endif

	extern int clean_pcm_rx_cnt[MAX_DSP_RTK_CH_NUM];
	extern char fax_2100_cnt_tx[MAX_DSP_RTK_CH_NUM];

	// I: 10.7502 (2.6678)
	// D: 10.7030 (2.4080)
	//     9.2410 (1.3275)
	// I:  8.2373 (1.9805) 1.5.4 @500
	//     8.8485 (2.4346) 1.5.4 @620
	//     9.3161 (2.6112) 1.5.5 @620
	// D:  8.2373 (1.3397) 1.5.4 @500
	//     8.8485 (2.1160) 1.5.4 @620
	//     9.3161 (1.7467) 1.5.5 @620
#ifdef FEATURE_COP3_PCM_RX
	if (cp3_voip_param.bCp3Count_PCM_RX == 1)
		ProfileEnterPoint(PROFILE_INDEX_PCM_RX);
#endif

#ifdef PCM_PERIOD
	//rx_rp = rxpage[chid] * (40*PCM_PERIOD);	//count page0/page1 offset
	rx_rp = 0;
#else		
#ifdef SUPPORT_CUT_DSP_PROCESS
	rx_rp = rxpage[chid] * 40;	//count page0/page1 offset

#else
	rx_rp = rxpage[chid] * (nPcmIntPeriod[chid] << 2);	//count page0/page1 offset
#endif
#endif

#ifdef SUPPORT_PCM_FIFO //==========================================================//

#ifdef SUPPORT_LEC_G168_ISR
#if DMEN_LEC || DMEN_STACK_LEC
	extern int dmem_size;
	set_DMEM(&__lec_dmem_start, dmem_size-1);
#endif
#if DMEN_LEC
	LEC_dmem_load(chid);	
#endif

#endif   	  

#ifdef SUPPORT_AES_ISR
	if (!bWideband && pcm_ch_for_DAA == 1)
	{
		aes_do_cnt[chid] = 0;
		aes_tx_fifo_size[chid] = pcm_get_tx_PCM_FIFO_SIZE(chid);
	
		if (aes_tx_fifo_size[chid] < PCM_PERIOD)
			aes_do_times[chid] = aes_tx_fifo_size[chid];
		else
			aes_do_times[chid] = PCM_PERIOD;
	
		//printk("%d \n", aes_do_times[chid]);
	}
#endif
	  
	//for (i=0; i < PCM_PERIOD; i++) 
	{
		
		/*** Check if Rx FIFO full or not ***/
		//if (((rx_fifo_cnt_w[chid]+1)%PCM_FIFO_SIZE) == rx_fifo_cnt_r[chid]) 
		//{
		//	// Full
        //           #ifdef PCM_DBG_SUPPRESSION
		//	RF[chid]++;
		//    #else
		//	printk("RF(%d) ", chid);
        //            #endif
		//	break;
		//}

		if( chid == 0 )
			ddinst_rw_auto( VOIP_DEV_PCM0_RX, ( char * )rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2), 160 );
		else if( chid == 1 )
			ddinst_rw_auto( VOIP_DEV_PCM1_RX, ( char * )rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2), 160 );

#if 1
		MixIvrSpeechWaveToPlayoutBuffer_Remote( chid, rxBuf+(nWidebandFactor*i*PCM_PERIOD_10MS_SIZE>>2) );
#endif

#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
		if( bWideband ) {

			resampler_process_int_ex(pResampler_down_st, 
				PCMRX_RESAMPLER_OFFSET + chid, // see MAX_NB_CHANNELS to know detail  
				( const int16_t * )rxBuf+(nWidebandFactor*i*PCM_PERIOD_10MS_SIZE>>2), PCM_PERIOD_10MS_SIZE/2*2, 
				( int16_t * )rxBuf_NB, PCM_PERIOD_10MS_SIZE/2,
				"PCM-RX down error = %d\n" );
		}
#endif
		    	
   
#ifdef DTMF_DEC_ISR
    	
    	/**************** Create det_buff from PCM RX FIFO *****************/
    	// pkshih: process narrowband data 
#ifdef SUPPORT_SLIC_GAIN_CFG
		
#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
		rxBuf_tmp = ( bWideband ? rxBuf_NB : rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2) );
#endif
		
#ifdef SUPPORT_CUT_DSP_PROCESS        
		for (ii=0; ii<40; ii++)
#else
		for (ii=0; ii<(int)(nPcmIntPeriod[chid] << 2); ii++)
#endif
		{  
#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
			//rxBuf_tmp = ( bWideband ? rxBuf_NB : rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2) );
			stmp = *( rxBuf_tmp + ii );
#else
			stmp = *(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)+ii);
#endif
		
			s1 = (short)((stmp >> 16) & 0x0000ffff);
			s2 = (short)((stmp & 0x0000ffff));

			det_buff[chid][ii<<1] = (int) (s1 * tx_comp[g_txVolumneGain[chid]]) >> 8;
			det_buff[chid][(ii<<1)+1] = (int) (s2 * tx_comp[g_txVolumneGain[chid]]) >> 8;

		}
#endif

#ifdef T_TYPE_ECHO_CAN
		int mmd_backup;
		mmd_backup = read_32bit_radiax_register(RADIAX_MMD);
		write_32bit_radiax_register(RADIAX_MMD, SET_MMD_MS);
		highpass_process(chid, rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2), 80);
		write_32bit_radiax_register(RADIAX_MMD, mmd_backup);
#endif
#ifdef RTK_VOICE_RECORD
		if ((stVoipdataget[chid].write_enable&4)||(stVoipdataget[chid].write_enable==2))
		{
			memcpy(&stVoipdataget[chid].rxbuf[stVoipdataget[chid].rx_writeindex],rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2),160);
			stVoipdataget[chid].rx_writeindex= (stVoipdataget[chid].rx_writeindex+160)%DATAGETBUFSIZE;
		}
#endif //#ifdef RTK_VOICE_RECORD

	/**********************************************
		 *                                    *
		 * 	   DAA Bandpass	      	      *
		 *                                    *
		 ****************************************************/
		 // pkshih: process narrowband data 
#ifdef SUPPORT_BANDPASS_FOR_DAA
		 if (pcm_ch_for_DAA == 1)
		 {
#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
			// DAA will write back to rxBuf_tmp, but it is safe 
			// because DAA is always narrowband. 
			rxBuf_tmp = ( bWideband ? rxBuf_NB : rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2) );
#endif

#ifdef SUPPORT_CUT_DSP_PROCESS        
			for (j=0; j<40; j++)
#else
			for (j=0; j<(int)(nPcmIntPeriod[chid] << 2); j++)
#endif
			{
#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
				//rxBuf_tmp = ( bWideband ? rxBuf_NB : rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2) );
				stmp = *( rxBuf_tmp + j );
#else
				stmp = *(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)+j);
#endif
				s1 = (short)((stmp >> 16) & 0x0000ffff);
				s2 = (short)((stmp & 0x0000ffff));

				s1 = daa_bandpass(chid, s1);	//200-3400hz bandpass
				s2 = daa_bandpass(chid, s2);	//200-3400hz bandpass

				stmp = ((unsigned int)s1 << 16) + ((unsigned int)s2 & 0x0000ffff);

#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
				*( rxBuf_tmp + j ) = stmp;
#else
		        *(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)+j) = stmp;
#endif
			}
		}
#endif

		// I: 1.4587 (.5643)
		//     .3453 (.1986)	quick hook
		// D: 1.4685 (.7268)
		//     .3548 (.1137)	quick hook
		// pkshih: process narrowband data 
#ifdef FXO_CALLER_ID_DET
		 if (pcm_ch_for_DAA == 1)
		 {
		 	if( p_snd ->daa_ops ->DAA_Hook_Status( p_snd, 0 ) ==0)
		 	{
			 	long daa_sum;
			 	daa_sum=0;
#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
				rxBuf_tmp = ( bWideband ? rxBuf_NB : rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2) );
#endif

#ifdef SUPPORT_CUT_DSP_PROCESS        
				for (j=0; j<40; j++)
#else
				for (j=0; j<(int)(nPcmIntPeriod[chid] << 2); j++)
#endif
				{
#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
					//rxBuf_tmp = ( bWideband ? rxBuf_NB : rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2) );
					stmp = *( rxBuf_tmp + j );
#else
					stmp = *(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)+j);
#endif
					s1 = (short)((stmp >> 16) & 0x0000ffff);
					s2 = (short)((stmp & 0x0000ffff));
	
					daa_sum= daa_sum + s1 + s2;
				}
	
				daa_sum= daa_sum/80;
#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
				rxBuf_tmp = ( bWideband ? rxBuf_NB : rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2) );
#endif

#ifdef SUPPORT_CUT_DSP_PROCESS        
				for (j=0; j<40; j++)
#else
				for (j=0; j<(int)(nPcmIntPeriod[chid] << 2); j++)
#endif
				{
#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
					//rxBuf_tmp = ( bWideband ? rxBuf_NB : rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2) );
					stmp = *( rxBuf_tmp + j );
#else
					stmp = *(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)+j);
#endif
					s1 = (short)((stmp >> 16) & 0x0000ffff);
					s2 = (short)((stmp & 0x0000ffff));
	
					s1 = s1 - daa_sum;
					s2 = s2 - daa_sum;
	
					stmp = ((unsigned int)s1 << 16) + ((unsigned int)s2 & 0x0000ffff);

#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
					*( rxBuf_tmp + j ) = stmp;
#else
			        *(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)+j) = stmp;
#endif
				}
				dtmf_cid_lower_level(chid, 1, 0);
				dtmf_cid_lower_level(chid, 1, 1);
			}
			else
			{
				dtmf_cid_lower_level(chid, 0, 0);
				dtmf_cid_lower_level(chid, 0, 1);
			}
		}
#endif




	/**********************************************
	 *                                    *
	 * 	   DTMF Detection	      *
	 *                                    *
	 ****************************************************/		
	
	// I: 1.4288 (.6082)
	//    1.3869 (.5042) 2 imem .. (X)
	// D: 1.4377 (.2774)
	// pkshih: process narrowband data 
#ifdef DTMF_DET_PRIOR_LEC

	if (dtmf_chid[chid][0] == 1)
	{
			
	#ifdef FEATURE_COP3_DTMFDEC
		ProfileEnterPoint(PROFILE_INDEX_DTMFDEC);
	#endif
		long dtmf_det_threshold_current;
		extern long dtmf_det_threshold[];
		extern long talk_flag[];
		
		
		dtmf_det_threshold_current = dtmf_det_threshold[chid];
		
		if (talk_flag[chid] == 1)
		{
			// when talking using high level in order to avoid to detect the echo dtmf.
			if (dtmf_det_threshold_dBm_get(chid, 0) > 16 ) /* threshold < -16dBm */
			{
				dtmf_det_threshold_current  = dtmf_det_threshold_dBm2Hex(DTMF_POWER_LEVEL_MINUS_16DBM);
			}
		}

		
		#ifdef SUPPORT_SLIC_GAIN_CFG	
			dtmf_digit = dtmf_dec(det_buff[chid], PCM_PERIOD_10MS_SIZE, chid, dtmf_det_threshold_current, 0);
		#else	
		  #ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
			rxBuf_tmp = ( bWideband ? rxBuf_NB : rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2) );
			dtmf_digit = dtmf_dec((char *)(rxBuf_tmp), PCM_PERIOD_10MS_SIZE, chid, dtmf_det_threshold_current, 0);
		  #else
			dtmf_digit = dtmf_dec((char *)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)), PCM_PERIOD_10MS_SIZE, chid, dtmf_det_threshold_current, 0);
		  #endif
		#endif

#ifdef FSK_TYPE2_ACK_CHECK
		if ( (fsk_alert_done[chid] == 1) && (dtmf_digit.digit == gFskType2AckTone[chid]))
		{
			fsk_cid_type2_ack[chid] = 1;	// Get Type2 FSK Ack
			PRINT_MSG("Get Type2 FSK Ack %c-tone, ch=%d\n", dtmf_digit.digit, chid);
		}
#endif
#ifdef CUSTOMIZE_DTMF_MINIMUM_ON_TIME
		extern char dtmf_pre_stat[][2];
		extern short dtmf_on_cnt_10ms[][2];
		extern short dtmf_on_flag[][2];
		if (dtmf_digit.digitOnOff!=DSIL) {
			dtmf_on_cnt_10ms[chid][0]++;
			if (dtmf_on_cnt_10ms[chid][0]==dtmf_det_on_time_get(chid, 0)) {
				voip_event_dtmf_in( chid, dtmf_conv_table[dtmf_digit.digitOnOff], 0, 0, 0 );
				dtmf_on_flag[chid][0]=1;
			}
		} else if (dtmf_pre_stat[chid][0]!=DSIL) {
		
		} else {
#ifdef DTMF_DET_DURATION_HIGH_ACCURACY
			unsigned int duration;
			if ( 1 == dtmf_det_duration_get(chid, 0, &duration))
			{
				voip_event_dtmf_in(chid, 'E', 0, get_dtmf_dBFS(chid, 0, 1), duration);
				get_dtmf_dBFS(chid, 0, 1);
				dtmf_on_cnt_10ms[chid][0]=0;
				dtmf_on_flag[chid][0]=0;
			}
#else
			if (dtmf_on_flag[chid][0])
				voip_event_dtmf_in(chid, 'E', 0, get_dtmf_dBFS(chid, 0, 1), 10*dtmf_on_cnt_10ms[chid][0]);
			get_dtmf_dBFS(chid, 0, 1);
			dtmf_on_cnt_10ms[chid][0]=0;
			dtmf_on_flag[chid][0]=0;
#endif
		}
		dtmf_pre_stat[chid][0] = dtmf_digit.digitOnOff;
#else
		voip_event_dtmf_in( chid, dtmf_digit.digit, 0, 0, 0 );
#endif		
	#ifdef FEATURE_COP3_DTMFDEC
		ProfileExitPoint(PROFILE_INDEX_DTMFDEC);
		ProfilePerDump(PROFILE_INDEX_DTMFDEC, cp3_voip_param.cp3_dump_period);
	#endif
	}
#endif	//DTMF_DET_PRIOR_LEC
                
#endif	//DTMF_DEC_ISR	

	/**************** Fax V21 Detection *****************/
	// I: 1.2893 (.4597)
	// D: 1.2239 (.2549)
	// pkshih: process narrowband data 
	if (fax_modem_det[chid] == 1)
	{
#ifdef SUPPORT_SLIC_GAIN_CFG
		fax_v21_dec(det_buff[chid], chid);
#else
	#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
		rxBuf_tmp = ( bWideband ? rxBuf_NB : rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2) );
		fax_v21_dec((short *)( rxBuf_tmp ), chid);
	#else
		fax_v21_dec((short *)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)), chid);
	#endif
#endif
	}

	/**************** Caller ID Detection *****************/
		
		// I: 7.4531 (1.0942)
		//    6.7275 (1.1098)	quick hook
		//    6.6420 (1.0560)	G -> L (O)
		//    6.2568 (.5266)	2 imem
		//    6.5613 (.9181)	1s imem
		//    6.5213 (1.0599)	dmem - sum
		// D: 7.4031 (1.2783)
		//    6.7025 (.7856)	quick hook
		//    6.6189 (.7322)	G -> L (O)
		//    6.2550 (.6474)	2 imem
		//    6.5756 (.7354)	1s imem
		//    6.4801 (.5879)	dmem - sum
		// GPIO: r=0, w=0
		// pkshih: process narrowband data 
#ifdef FXO_CALLER_ID_DET
	
		if ((caller_id_det[chid] == 1) && (pcm_ch_for_DAA == 1))	// only daa channel will do CID det. 
		{
			/* Move DTMF Caller ID Check after "DTMF_det after LEC" */

			if ( (auto_cid_det[chid] !=  AUTO_CID_DET_OFF) || (cid_type[chid] != CID_TYPE_DTMF) )
			{
				/* FSK CID DET */
	#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
				rxBuf_tmp = ( bWideband ? rxBuf_NB : rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2) );
				cid_det_si3050((unsigned short*)(rxBuf_tmp), chid);
	#else
				cid_det_si3050((unsigned short*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)), chid);
	#endif
			}
		}
#endif

	/**************** Busy Tone Detection *****************/
		// pkshih: process narrowband data 
#ifdef FXO_BUSY_TONE_DET
		extern unsigned int pre_daa_status[];
		if ((tone_det[chid] == 1) && (pcm_ch_for_DAA == 1) && (pre_daa_status[chid] == 1) )	// only daa channel will do Busy tone det. 
		{												/* only do tone detect whon DAA off-hook, pre_daa_status[chid]*/
	#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
			rxBuf_tmp = ( bWideband ? rxBuf_NB : rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2));
			busy_tone_det(chid, (char *)(rxBuf_tmp));
			dis_tone_det(chid, (char *)(rxBuf_tmp));
	#else
			busy_tone_det(chid, (char *)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)));
			dis_tone_det(chid, (char *)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)));
	#endif
		}
#endif
        
                

	/**********************************************
	 *                                    *
	 * 	FAX & MODEM Detection	      *
	 *                                    *
	 ****************************************************/

	// D: .5707 (.1160)
	// pkshih: process narrowband data 
#ifdef SUPPORT_FAX_PASS_ISR

	/* callee should detect CED, when it is off-hook instead of codec start. */
	//if ( ( FindStartCodecTypeDesc( chid ) != NULL ) )  /* 0:stop, 1:G711, 2:G7231, 3:G729, 4:G726 */
	if (fax_modem_det[chid] == 1)
	{
	#ifdef ANSTONE_DET_PRIOR_LEC
		/************** FAX & MODEM ***************/	
		if (!ModemFlag[chid] && !FaxFlag[chid])
		{
#ifdef OPTIMIZATION
			uint32 sid;
			sid = chanInfo_GetTranSessionID(chid);
			if(sid == SESSION_NULL)
				sid = chid;
#endif

	#ifdef SUPPORT_SLIC_GAIN_CFG	
			modem_det(chid, det_buff[chid]);
	#else	
	  #ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
		rxBuf_tmp = ( bWideband ? rxBuf_NB : rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2) );
		modem_det(chid, (short*)(rxBuf_tmp));
	  #else
		modem_det(chid, (short*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)));
	  #endif
	#endif	
		
			if( ModemFlag[chid] ) 
				DisableDspFunctionsIfModemOrFaxIsDetected( sid );
		}
			#ifdef SUPPORT_SLIC_GAIN_CFG	
				answer_tone_det(chid, det_buff[chid], 0);
			#else	
			  #ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
				rxBuf_tmp = ( bWideband ? rxBuf_NB : rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2) );
				answer_tone_det(chid, (short*)(rxBuf_tmp), 0);
			  #else
				answer_tone_det(chid, (short*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)), 0);
			  #endif
			#endif					
	#endif /* ANSTONE_DET_PRIOR_LEC */
	}

#endif //SUPPORT_FAX_PASS_ISR

		
		/**********************************************
		 *                                    *
		 * 	Line Echo Canceller	      *
		 *                                    *
		 ****************************************************/
		 
		// I: 4.5472 (.6106)
		// D: 4.7435 (.6419)
		// pkshih: process narrowband data 
		#ifdef FEATURE_COP3_LEC				
			/* Add CP3 to caculate cycle need by LEC */
			if (cp3_voip_param.bCp3Count_LEC == 1)	
				ProfileEnterPoint(PROFILE_INDEX_LEC);
		#endif	
		 
#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE

		//if ( support_lec_g168[chid] && ( FindStartCodecTypeDesc( chid ) != NULL ) && (pcm_ch_for_DAA == 0))
		if ( support_lec_g168[chid] && ( FindStartCodecTypeDesc( chid ) != NULL ))
		{
			if(0) {//rx_mute[chid] == 1)	// bus fifo do it 
#ifdef USE_MEM64_OP
                                //memset64s(rx_fifo[chid][rx_fifo_cnt_w[chid]], 0,  PCM_PERIOD_10MS_SIZE/2); 
#else			
                                //memset(rx_fifo[chid][rx_fifo_cnt_w[chid]], 0,  PCM_PERIOD_10MS_SIZE); 
#endif
			} else
			{
				extern int iphone_handfree;
				if(iphone_handfree==0 && (pcm_ch_for_DAA != 1))
				{
#ifndef T_TYPE_ECHO_CAN
#ifndef LEC_USE_CIRC_BUF
					NLP_g168( chid, lec_ref/*(Word16*)&LEC_RinBuf[chid][80*PCM_PERIOD*(FSIZE-sync_point[chid])+i*PCM_PERIOD_10MS_SIZE/2-sync_sample[chid]]*/, 
#else
					NLP_g168( chid, lec_ref/*(Word16*)&LEC_Rin_CircBuf[chid][80*PCM_PERIOD*LEC_buf_rindex[chid] +i*PCM_PERIOD_10MS_SIZE/2]*/, 
#endif
				   			(Word16*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)),
							//(Word16*)(rx_fifo[chid][rx_fifo_cnt_w[chid]]));
							(Word16*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)));
#else
					if (ec_select)
						AEC_g168( chid, lec_ref/*(Word16*)&LEC_Rin_CircBuf[chid][80*PCM_PERIOD*LEC_buf_rindex[chid] +i*PCM_PERIOD_10MS_SIZE/2+sync_sample_offset_daa[chid]]*/,
						          (Word16*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)),
						          (Word16*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)) );
					else
						LEC_g168( chid, lec_ref/*(Word16*)&LEC_Rin_CircBuf[chid][80*PCM_PERIOD*LEC_buf_rindex[chid] +i*PCM_PERIOD_10MS_SIZE/2+sync_sample_offset_daa[chid]]*/,
						          (Word16*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)),
						          (Word16*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)) );
#endif
                                }
                                else
                                {

#if 0
                                	//memcpy(rx_fifo[chid][rx_fifo_cnt_w[chid]], &pRxBuf[chid][rx_rp]+(i*PCM_PERIOD_10MS_SIZE>>2),  PCM_PERIOD_10MS_SIZE);
//#ifdef EXPER_NR
                                	//NR( chid, (Word16*)(rx_fifo[chid][rx_fifo_cnt_w[chid]]), (Word16*)(rx_fifo[chid][rx_fifo_cnt_w[chid]]));
                                	NR( chid, (Word16*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)), (Word16*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)));
//#endif

					NLP_hf_g168( chid, (Word16*)&Handfree_delaybuf[0], 
				   			(Word16*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)),
                                                	(Word16*)(rx_fifo[chid][rx_fifo_cnt_w[chid]]));

					memmove(&Handfree_delaybuf[0], &Handfree_delaybuf[80], 170*2); //(250-80) short
#ifndef LEC_USE_CIRC_BUF
					memcpy(&Handfree_delaybuf[170], lec_ref/*(Word16*)&LEC_RinBuf[chid][80*PCM_PERIOD*(FSIZE-sync_point[chid])+i*PCM_PERIOD_10MS_SIZE/2-sync_sample[chid]]*/, PCM_PERIOD_10MS_SIZE);
#else
					memcpy(&Handfree_delaybuf[170], lec_ref/*(Word16*)&LEC_Rin_CircBuf[chid][80*PCM_PERIOD*LEC_buf_rindex[chid] +i*PCM_PERIOD_10MS_SIZE/2]*/, PCM_PERIOD_10MS_SIZE);
#endif

#else

					if (ec_select)
						AEC_g168( chid, lec_ref/*(Word16*)&LEC_Rin_CircBuf[chid][80*PCM_PERIOD*LEC_buf_rindex[chid] +i*PCM_PERIOD_10MS_SIZE/2+sync_sample_offset_daa[chid]]*/,
						          (Word16*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)),
						          (Word16*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)) );
					else
						LEC_g168( chid, lec_ref/*(Word16*)&LEC_Rin_CircBuf[chid][80*PCM_PERIOD*LEC_buf_rindex[chid] +i*PCM_PERIOD_10MS_SIZE/2+sync_sample_offset_daa[chid]]*/,
						          (Word16*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)),
						          (Word16*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)) );

#endif

                                }
#ifdef RTK_VOICE_PLAY
				if (stVoipdataput[chid].write_enable&4) {
					voice_play_mix(chid, (Word16*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)));
				}
#endif

                        }
		}
		else
		 
#elif defined (SUPPORT_LEC_G168_ISR)


		if ( (support_lec_g168[chid] && ( FindStartCodecTypeDesc( chid ) != NULL )  )
#ifdef PCM_LOOP_MODE_DRIVER
			|| pcm_check_If_In_LoopMode(chid)
#endif
		   )
		{
			if(0)//rx_mute[chid] == 1)	// bus_fifo do it 
			{
#ifdef USE_MEM64_OP
                                //memset64s(rx_fifo[chid][rx_fifo_cnt_w[chid]], 0,  PCM_PERIOD_10MS_SIZE/2); 
#else			
                                //memset(rx_fifo[chid][rx_fifo_cnt_w[chid]], 0,  PCM_PERIOD_10MS_SIZE); 
#endif                                
			} else
			{	
#ifdef EXPER_AEC
#ifndef LEC_USE_CIRC_BUF
				AEC_g168( chid, lec_ref/*(Word16*)&LEC_RinBuf[chid][80*PCM_PERIOD*(FSIZE-sync_point[chid])+i*PCM_PERIOD_10MS_SIZE/2-sync_sample[chid]]*/, 
#else
				AEC_g168( chid, lec_ref/*(Word16*)&LEC_Rin_CircBuf[chid][80*PCM_PERIOD*LEC_buf_rindex[chid] +i*PCM_PERIOD_10MS_SIZE/2]*/, 
#endif
				   		(Word16*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)),
                                                (Word16*)(rx_fifo[chid][rx_fifo_cnt_w[chid]]));
#else
#ifndef LEC_USE_CIRC_BUF
				LEC_g168( chid, lec_ref/*(Word16*)&LEC_RinBuf[chid][80*PCM_PERIOD*(FSIZE-sync_point[chid])+i*PCM_PERIOD_10MS_SIZE/2-sync_sample[chid]]*/, 
#else
	#if 1
		#ifdef DELAY_ECHO_TEST

				if (add_delayed_echo) {
					memmove( &delayedecho_buf[chid][0], &delayedecho_buf[chid][80], 2*(1024) /*byte*/);
					
					for (ii=0; ii< 40; ii++) {
						delayedecho_buf[chid][(add_delayed_echo>>16)+ii*2] = 
						           (lec_ref[i*PCM_PERIOD_10MS_SIZE/2+ii*2]*(add_delayed_echo&0x7fff))>>15;
						delayedecho_buf[chid][(add_delayed_echo>>16)+ii*2+1] = 
						           (lec_ref[i*PCM_PERIOD_10MS_SIZE/2+ii*2+1]*(add_delayed_echo&0x7fff))>>15;

						stmp = *(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)+ii);

						s1 = (short)((stmp >> 16) & 0x0000ffff);
						s2 = (short)((stmp & 0x0000ffff));
						if ( s1 + delayedecho_buf[chid][ii*2]>0x7fff )
							s1=0x7fff;
						else if (s1 + delayedecho_buf[chid][ii*2]<-32768)
							s1=-32768;
						else
							s1 += delayedecho_buf[chid][ii*2];
						if ( s2 + delayedecho_buf[chid][ii*2+1]>0x7fff )
							s2=0x7fff;
						else if (s2 + delayedecho_buf[chid][ii*2+1]<-32768)
							s2=-32768;
						else
							s2 += delayedecho_buf[chid][ii*2+1];
						//*(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)+ii) = (s1<<16) | (s2&0xffff);
						*(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)+ii) = (delayedecho_buf[chid][ii*2]<<16)| (delayedecho_buf[chid][ii*2+1]&0xffff);
					}
				}
		#endif
				
				if (ec_select)	
					AEC_g168( chid, lec_ref/*(Word16*)&LEC_Rin_CircBuf[chid][80*PCM_PERIOD*LEC_buf_rindex[chid] +i*PCM_PERIOD_10MS_SIZE/2+sync_sample_offset_daa[chid]]*/,
				   		(Word16*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)),
                                                (Word16*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)) );
				else
					LEC_g168( chid, lec_ref/*(Word16*)&LEC_Rin_CircBuf[chid][80*PCM_PERIOD*LEC_buf_rindex[chid] +i*PCM_PERIOD_10MS_SIZE/2+sync_sample_offset_daa[chid]]*/,
					          (Word16*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)),
					          (Word16*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)) );
	#else
					frequency_echo_cancellation( chid, lec_ref/*(Word16*)&LEC_Rin_CircBuf[chid][80*PCM_PERIOD*LEC_buf_rindex[chid] +i*PCM_PERIOD_10MS_SIZE/2+sync_sample_offset_daa[chid]]*/,
//					( bWideband ? rxBuf_NB : (Word16*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)) ),
					(Word16*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)),
//#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
//					( bWideband ? rxBuf_NB : (Word16*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)) ));
//#else
//					(Word16*)(rx_fifo[chid][rx_fifo_cnt_w[chid]]));
//#endif
					(Word16*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)) );
	#endif
#endif
#endif

#ifdef RTK_VOICE_PLAY
					if (stVoipdataput[chid].write_enable&4) {
						voice_play_mix(chid, (Word16*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)));
					}
#endif

#ifdef EXPER_NR
				extern int iphone_handfree;
				if(iphone_handfree==1)
					NR( chid, (Word16*)(rx_fifo[chid][rx_fifo_cnt_w[chid]]), (Word16*)(rx_fifo[chid][rx_fifo_cnt_w[chid]]));
#endif

			}
		}
		else
#endif //SUPPORT_LEC_G168_ISR
		{
#if 0	// chmap 
			if(rx_mute[chid] == 1)
			{
#ifdef USE_MEM64_OP
                                memset64s(rx_fifo[chid][rx_fifo_cnt_w[chid]], 0,  PCM_PERIOD_10MS_SIZE/2);
#else                                
                                memset(rx_fifo[chid][rx_fifo_cnt_w[chid]], 0,  PCM_PERIOD_10MS_SIZE);
#endif                                
			} else {
#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
				// do nothing
#else
#ifdef USE_MEM64_OP
                                memcpy64s(rx_fifo[chid][rx_fifo_cnt_w[chid]], rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2),  PCM_PERIOD_10MS_SIZE/2);
#else                                
                                memcpy(rx_fifo[chid][rx_fifo_cnt_w[chid]], rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2),  PCM_PERIOD_10MS_SIZE);
#endif                                
#endif
			}
#endif
		}

		#ifdef FEATURE_COP3_LEC    
			if (cp3_voip_param.bCp3Count_LEC == 1)
			{        
				ProfileExitPoint(PROFILE_INDEX_LEC);
				ProfilePerDump(PROFILE_INDEX_LEC, cp3_voip_param.cp3_dump_period);
				//ProfileDump(PROFILE_INDEX_LEC, PROFILE_INDEX_LEC,1000);
			}
		#endif


	/**********************************************
		 *                                    *
		 * 	Acoustic Echo Suppression     *
		 *                                    *
		 ****************************************************/
#ifdef SUPPORT_AES_ISR
		if (!bWideband && pcm_ch_for_DAA == 1)
		{
		
		if ((support_AES[chid] == 0) || (FindStartCodecTypeDesc(chid) == NULL ) || (aes_do_cnt[chid] == aes_do_times[chid]))
		{
			goto SKIP_AES;
		}
		
#ifdef SUPPORT_CUT_DSP_PROCESS	
		for (ii=0; ii<40; ii++)
#else
		for (ii=0; ii<(int)(nPcmIntPeriod[chid] << 2); ii++)
#endif
        	{ 	
        		Word16 s1, s2, s3, s4;
			// read from pcm controller Rx page 
     			stmp = *((uint32 *)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)+ii));
			s1 = (short)((stmp >> 16) & 0x0000ffff);
			s2 = (short)((stmp & 0x0000ffff));

			// read from Tx fifo			
                        stmp = tx_fifo[chid][(tx_fifo_cnt_r[chid]+aes_do_cnt[chid])%PCM_FIFO_SIZE][ii];
			s3 = (short)((stmp >> 16) & 0x0000ffff);
			s4 = (short)((stmp & 0x0000ffff));

			AES(chid, (Word16*)&s1, (Word16*)&s3);
			AES(chid, (Word16*)&s2, (Word16*)&s4);
		
			// write to Rx fifo
			stmp = ((unsigned int)s1 << 16) + ((unsigned int)s2 & 0x0000ffff);
                        rx_fifo[chid][rx_fifo_cnt_w[chid]][ii] = stmp;
			
			// write to Tx fifo
			stmp = ((unsigned int)s3 << 16) + ((unsigned int)s4 & 0x0000ffff);			
                        tx_fifo[chid][(tx_fifo_cnt_r[chid]+aes_do_cnt[chid])%PCM_FIFO_SIZE][ii] = stmp;

		}

#ifdef RTK_VOICE_RECORD

		if(stVoipdataget[chid].write_enable & 8)
		{
			memcpy(&stVoipdataget[chid].txbuf[stVoipdataget[chid].rx_writeindex],tx_fifo[chid][(tx_fifo_cnt_r[chid]+aes_do_cnt[chid])%PCM_FIFO_SIZE],160);
			memcpy(&stVoipdataget[chid].rxbuf[stVoipdataget[chid].rx_writeindex],(&pRxBuf[chid][rx_rp]+(i*PCM_PERIOD_10MS_SIZE>>2)),160);
			if(stVoipdataget[chid].write_enable & 16)
				memcpy(&stVoipdataget[chid].rxbuf2[stVoipdataget[chid].rx_writeindex],rx_fifo[chid][rx_fifo_cnt_w[chid]],160);
			
			stVoipdataget[chid].rx_writeindex= (stVoipdataget[chid].rx_writeindex+160)%DATAGETBUFSIZE;
			stVoipdataget[chid].tx_writeindex= stVoipdataget[chid].rx_writeindex;
			if(stVoipdataget[chid].write_enable & 16)
				stVoipdataget[chid].rx_writeindex2= stVoipdataget[chid].rx_writeindex;
		}
#endif //#ifdef RTK_VOICE_RECORD

		aes_do_cnt[chid]++;
		

SKIP_AES:
		}
#endif
#ifdef SUPPORT_FAX_PASS_ISR
	/* ===============  answer tone detect after lec   ==================*/
		if (fax_modem_det[chid] == 1)
		{
	#ifndef ANSTONE_DET_PRIOR_LEC
	    #ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
			if( bWideband ) {
				resampler_process_int_ex(pResampler_down_st, 
					PCMRX_LEC_RESAMPLER_OFFSET + chid, // see MAX_NB_CHANNELS to know detail  
					rxBuf+(nWidebandFactor*i*PCM_PERIOD_10MS_SIZE>>2), PCM_PERIOD_10MS_SIZE/2*2, 
					rxBuf_NB, PCM_PERIOD_10MS_SIZE/2,
					"PCM-RX-post-LEC down error = %d\n" );
			}
			rxBuf_tmp = ( bWideband ? rxBuf_NB : rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2) );
			answer_tone_det(chid, (short*)(rxBuf_tmp), 0);
	    #else
	    		answer_tone_det(chid, (short*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)), 0);
	    #endif
	    #ifdef SUPPORT_SLIC_GAIN_CFG
	    	#error
	    #endif


		/************** FAX & MODEM ***************/	
			if (!ModemFlag[chid] && !FaxFlag[chid])
			{
#ifdef OPTIMIZATION
				uint32 sid;
				sid = chanInfo_GetTranSessionID(chid);
				if(sid == SESSION_NULL)
					sid = chid;
#endif
			#ifdef SUPPORT_SLIC_GAIN_CFG	
			    #error
			#else	
			  #ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
				rxBuf_tmp = ( bWideband ? rxBuf_NB : rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2) );
				modem_det(chid, (short*)(rxBuf_tmp));
			  #else
				modem_det(chid, (short*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)));
			  #endif
			#endif	
				
				if( ModemFlag[chid] ) 
					DisableDspFunctionsIfModemOrFaxIsDetected( sid );
			}
	#endif /* ANSTONE_DET_PRIOR_LEC */
		}
#endif /* SUPPORT_FAX_PASS_ISR */


	/**********************************************
		 *                                    *
		 * 	   DTMF Detection	      *
		 *                                    *
		 ****************************************************/		
		
		// I: 1.4288 (.6082)
		//    1.3869 (.5042) 2 imem .. (X)
		// D: 1.4377 (.2774)
		// pkshih: process narrowband data 
#ifndef DTMF_DET_PRIOR_LEC

		if (dtmf_chid[chid][0] == 1)
		{
	
		#ifdef FEATURE_COP3_DTMFDEC
			ProfileEnterPoint(PROFILE_INDEX_DTMFDEC);
		#endif
			long dtmf_det_threshold_current;
			extern long dtmf_det_threshold[MAX_DSP_RTK_CH_NUM][2];
			extern long talk_flag[];
		

			dtmf_det_threshold_current = dtmf_det_threshold[chid][0];
			
			if (talk_flag[chid] == 1)
			{
				// when talking using high level in order to avoid to detect the echo dtmf.
				if (dtmf_det_threshold_dBm_get(chid, 0) > 16 ) /* threshold < -16dBm */
				{
					dtmf_det_threshold_current  = dtmf_det_threshold_dBm2Hex(DTMF_POWER_LEVEL_MINUS_16DBM);
				}
			}

		
		#ifdef SUPPORT_SLIC_GAIN_CFG	
			dtmf_digit = dtmf_dec(det_buff[chid], PCM_PERIOD_10MS_SIZE, chid, dtmf_det_threshold_current, 0);
		#else	
		  #ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
			rxBuf_tmp = ( bWideband ? rxBuf_NB : rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2) );
			dtmf_digit = dtmf_dec((char *)(rxBuf_tmp), PCM_PERIOD_10MS_SIZE, chid, dtmf_det_threshold_current, 0);
		  #else
			dtmf_digit = dtmf_dec((char *)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)), PCM_PERIOD_10MS_SIZE, chid, dtmf_det_threshold_current, 0);
		  #endif
		#endif

#ifdef FSK_TYPE2_ACK_CHECK
		extern unsigned int gtype2_waiting_for_ack_time[];
		
		if ( (fsk_alert_done[chid] == 1) && (dtmf_digit.digit == gFskType2AckTone[chid]))
		{
			fsk_cid_type2_ack[chid] = 1;	// Get Type2 FSK Ack
			gtype2_waiting_for_ack_time[ chid ] = timetick - gtype2_waiting_for_ack_time[ chid ];
			PRINT_MSG("Get Type2 FSK Ack %c-tone, ch=%d\n", dtmf_digit.digit, chid);
		}
#endif
#ifdef CUSTOMIZE_DTMF_MINIMUM_ON_TIME
		extern char dtmf_pre_stat[][2];
		extern short dtmf_on_cnt_10ms[][2];
		extern short dtmf_on_flag[][2];
		if (dtmf_digit.digitOnOff!=DSIL) {
			dtmf_on_cnt_10ms[chid][0]++;
			if (dtmf_on_cnt_10ms[chid][0]==dtmf_det_on_time_get(chid, 0)) {
				voip_event_dtmf_in( chid, dtmf_conv_table[dtmf_digit.digitOnOff], 0, 0, 0 );
				dtmf_on_flag[chid][0]=1;
			}
		} else if (dtmf_pre_stat[chid][0]!=DSIL) {

		} else {
#ifdef DTMF_DET_DURATION_HIGH_ACCURACY
			unsigned int duration;
			if ( 1 == dtmf_det_duration_get(chid, 0, &duration))
			{
				voip_event_dtmf_in( chid, 'E', 0, get_dtmf_dBFS(chid, 0, 1), duration );
				get_dtmf_dBFS(chid, 0, 1);
				dtmf_on_cnt_10ms[chid][0]=0;
				dtmf_on_flag[chid][0]=0;
			}
#else
			if (dtmf_on_flag[chid][0])
				voip_event_dtmf_in(chid, 'E', 0, get_dtmf_dBFS(chid, 0, 1), 10*dtmf_on_cnt_10ms[chid][0]);
			get_dtmf_dBFS(chid, 0, 1);
			dtmf_on_cnt_10ms[chid][0]=0;
			dtmf_on_flag[chid][0]=0;
#endif
		}
		dtmf_pre_stat[chid][0] = dtmf_digit.digitOnOff;
#else
		voip_event_dtmf_in( chid, dtmf_digit.digit, 0, 0, 0 );
#endif		
		#ifdef FEATURE_COP3_DTMFDEC
			ProfileExitPoint(PROFILE_INDEX_DTMFDEC);
			ProfilePerDump(PROFILE_INDEX_DTMFDEC, cp3_voip_param.cp3_dump_period);
                #endif
                }
#endif	//DTMF_DET_PRIOR_LEC

#ifdef FXO_CALLER_ID_DET
	
		/* DTMF Caller ID Check */
		if ((caller_id_det[chid] == 1) && (pcm_ch_for_DAA == 1))	// only daa channel will do CID det. 
		{
			if ( (auto_cid_det[chid] !=  AUTO_CID_DET_OFF) || (cid_type[chid] == CID_TYPE_DTMF) )
			{
				TstVoipciddet cid_res;
				cid_res = dtmf_cid_check(chid, dtmf_digit.digit);
				//if (cid_res.cid_valid == 1)
				//{
				//	flush_dtmf_fifo(chid);	//thlin: move to dtmf_cid_check
				//}
			}
		}
#endif


#ifdef SEND_RFC2833_ISR  

	/**********************************************                 
		 *                                    *                 
		 * 	    Send RFC2833 Packet	      *                 
		 *                                    *                 
		 ****************************************************/	
		               
		        /* ASCII Table: char 0-9 -> Dec# 48-57
		         *		char A-D -> Dec# 65-68 
		         *		char  *  -> Dec# 42 
		         *		char  #  -> Dec# 35 
		         */
		        
		        /* RTP Evevt:   0-9   ->  0-9
		         *		*     ->  10	
		         *		#     ->  11
		         *		A-D   ->  12-15
		         *		Flash ->  16
		         */
		
		extern uint32 rfc2833_count_add(uint32 sid, uint32 cnt);
		extern unsigned char rfc2833_dtmf_pt_local[];
		extern unsigned char rfc2833_dtmf_pt_remote[];
		extern uint32 chanInfo_IsConference(uint32 chid);
		extern int32 chanInfo_GetRegSessionRank(uint32 chid, uint32 rank[]);
		int send_2833_flag_ss = 0;
		int send_2833_flag_ch = 0;
		//int bypass_2833_chid_limit = 0;
		static int dtmf_removal_pre[MAX_DSP_RTK_CH_NUM-1] = {0};
		uint32 isConference;
		uint32 pSessRank[ 2 ];
		 
		/* 
		 * send_2833_flag_ss is used to decide to send RFC2833 packets or not.
		 * According to payload type of local and remote UA, it will change the send_2833_flag_ss result.
		 * If payload type of local and remote UA is equal to ZERO, it means DTMF Inband.
		 */
		
		isConference = chanInfo_IsConference(chid);
		chanInfo_GetRegSessionRank(chid, pSessRank);
		
		if (isConference)
			SessNum = chanInfo_GetRegSessionNum(chid);
		else
			SessNum = 1;
			
		for (j=0; j < SessNum; j++)
		{
			if( ppStartCodecTypeDesc[pSessRank[j]] == NULL )
				continue;
				
			if( pSessRank[j] == SESSION_NULL )
				continue;
				
			//if( bypass_2833_chid_limit )
			//	goto label_send_2833_now;

			if (rfc2833_dtmf_pt_local[pSessRank[j]]!=0 && rfc2833_dtmf_pt_remote[pSessRank[j]]!=0)//Not DTMF Inband
			{
				send_2833_flag_ss = 1;
				send_2833_flag_ch = 1;	// flag set means at least 1 session of this channel send 2833
			}
			else
			{
				// if local, or remote is Inband DTMF, then not send 2833 pkts.
				send_2833_flag_ss = 0;
				//Must not set send_2833_flag_ch = 0 here, send_2833_flag_ch is for this channel DTMF removal
			}

			if ((dtmf_mode[chid] != 1)/* not SIP INFO */ 
				&& (dtmf_mode[chid] != 3))/* not DTMF delete */
			{
				/* DTMF inband may send 2833. RTK SIP follow caller.
				   If callee is inband, caller is 2833, the callee's local/remote PT != 0*/

				if (send_2833_flag_ss == 1)
				{

#ifdef CONFIG_RTK_VOIP_IP_PHONE
	  				send_2833_by_ap[chid] = 1;  				
#endif
			  		if (send_2833_by_ap[chid] == 0)
			  		{
						if (dtmf_digit.digit >= 48 && dtmf_digit.digit <= 57 && send_dtmf_flag[pSessRank[j]] == 0)	/* 0 to 9  */
						{
							send_dtmf_flag[pSessRank[j]] = 1;
							g_digit[chid] = dtmf_digit.digit-48;
							//printk("->%d\n",g_digit[chid]);
						}
						else if (dtmf_digit.digit >= 65 && dtmf_digit.digit <= 68 && send_dtmf_flag[pSessRank[j]] == 0)	/* A to D  */
						{
							send_dtmf_flag[pSessRank[j]] = 1;
							g_digit[chid] = dtmf_digit.digit-65+12;
							//printk("->%d\n",g_digit[chid]);
						}
						else if (dtmf_digit.digit == 42 && send_dtmf_flag[pSessRank[j]] == 0)	/* (*) */
						{
							send_dtmf_flag[pSessRank[j]] = 1;
							g_digit[chid] = dtmf_digit.digit-42+10;
							//printk("->%d\n",g_digit[chid]);
						}
						else if (dtmf_digit.digit == 35 && send_dtmf_flag[pSessRank[j]] == 0)	/* (#) */
						{
							send_dtmf_flag[pSessRank[j]] = 1;
							g_digit[chid] = dtmf_digit.digit-35+11;
							//printk("->%d\n",g_digit[chid]);
						}
						else if (dtmf_digit.digit == 'Z' && send_dtmf_flag[pSessRank[j]] == 0)
						{
							g_digit[chid] = -1;
						}
					}
	
					if (send_dtmf_flag[pSessRank[j]] == 1 && (i%PCM_PERIOD) == 0 && (g_digit[chid] != -1))
					{
						//bypass_2833_chid_limit = 1;
						
label_send_2833_now:
						//printk("%d ", pSessRank[j]);
						
						if ((dtmf_removal[chid][0] == 1) && (dtmf_removal_pre[chid] == 0))
							rfc2833_count_add(pSessRank[j], 2); // add 20ms 
						else if ((dtmf_removal[chid][0] == 1) && (dtmf_removal_pre[chid] == 1))
							rfc2833_count_add(pSessRank[j], 1); // add 10ms 
	
						if (RtpOpen[pSessRank[j]])
							RtpTx_transmitEvent_ISR(chid, pSessRank[j], g_digit[chid]);
					}
				}

			}
		} /* SessNum */

		if ((dtmf_mode[chid] != 1)/* not SIP INFO */ 
			&& (dtmf_mode[chid] != 3))/* not DTMF delete */
		{
#ifdef SUPPORT_RFC2833_TRANSFER
			static int dtmf_remote_play[MAX_DSP_RTK_SS_NUM] = {0};

			if ( send_2833_flag_ch && isConference)
			{
				for (j=0; j < SessNum; j++)
				{
					if( ppStartCodecTypeDesc[pSessRank[j]] == NULL )
						continue;
						
					if( pSessRank[j] == SESSION_NULL )
						continue;
						
					//if( bypass_2833_chid_limit )
					//	goto label_send_2833_now;
		
					//if (rfc2833_dtmf_pt_local[pSessRank[j]]==0 && rfc2833_dtmf_pt_remote[pSessRank[j]]==0)
					if (rfc2833_dtmf_pt_local[pSessRank[j]]==0 || rfc2833_dtmf_pt_remote[pSessRank[j]]==0)
					{
						if ((dtmf_digit.digitOnOff != 16) && (dtmf_remote_play[pSessRank[j]] == 0))
						{
							if ((dtmf_digit.digitOnOff >= 0) && (dtmf_digit.digitOnOff <= 9))
								hc_SetPlayTone(chid, pSessRank[j], dtmf_digit.digitOnOff + DSPCODEC_TONE_0_CONT, 1, DSPCODEC_TONEDIRECTION_REMOTE);
							else if (dtmf_digit.digitOnOff == 10)
								hc_SetPlayTone(chid, pSessRank[j], DSPCODEC_TONE_STARSIGN_CONT, 1, DSPCODEC_TONEDIRECTION_REMOTE);
							else if (dtmf_digit.digitOnOff == 11)
								hc_SetPlayTone(chid, pSessRank[j], DSPCODEC_TONE_HASHSIGN_CONT, 1, DSPCODEC_TONEDIRECTION_REMOTE);
							
							dtmf_remote_play[pSessRank[j]] = 1;
							//PRINT_MSG("Play on, sid=%d\n", pSessRank[j]);
									
						}
						else if ((dtmf_digit.digitOnOff == 16) && (dtmf_remote_play[pSessRank[j]] == 1))
						{
							hc_SetPlayTone(chid, pSessRank[j], DSPCODEC_TONE_HASHSIGN_CONT, 0, DSPCODEC_TONEDIRECTION_REMOTE);
							dtmf_remote_play[pSessRank[j]] = 0;
							//PRINT_MSG("Play off, sid=%d\n", pSessRank[j]);
						}
					}
				}//end of for loop
			}
#endif
		}
		
		
 		dtmf_removal_pre[chid] = dtmf_removal[chid][0];
			
#endif	//SEND_RFC2833_ISR

	/**********************************************
		 *                                    *
		 * 	    DTMF REMOVAL	      *
		 *                                    *
		 ****************************************************/		
		
#ifdef DTMF_REMOVAL_ISR

		if ( FindStartCodecTypeDesc( chid ) != NULL )
		{
			if ((send_2833_flag_ch == 1) 
				|| (dtmf_mode[chid] == 1 /*SIP INFO*/)
				|| (dtmf_mode[chid] == 3 /*DTMF delete*/))
			{

				if(dtmf_removal[chid][0]) 
				{
					
#ifdef DTMF_REMOVAL_FORWARD
					if (clean_forward[chid] == 1 )
					{
#if 1
						int fifo_size = p_con ->con_ops ->dsp_read_bus_rx_get_fifo_size( p_con );
						uint16 *rx;
						
						for( j = 0; j < fifo_size; j ++ ) {
							rx = p_con ->con_ops ->dsp_read_bus_rx_peek_addr( p_con, j );
							
							if( rx == NULL )
								break;
							
#ifdef USE_MEM64_OP
							memset64s(rx, 0, PCM_PERIOD_10MS_SIZE/2*nWidebandFactor);
#else
							memset(rx, 0, PCM_PERIOD_10MS_SIZE*nWidebandFactor);
#endif
												
						}
#else	
						for (j=1; j <= pcm_get_rx_PCM_FIFO_SIZE(chid); j++)
						{
							if (rx_fifo_cnt_w[chid]-j >= 0)
							{
#ifdef USE_MEM64_OP
								memset64s(rx_fifo[chid][(rx_fifo_cnt_w[chid]-j)], 0, PCM_PERIOD_10MS_SIZE/2);
#else
								memset(rx_fifo[chid][(rx_fifo_cnt_w[chid]-j)], 0, PCM_PERIOD_10MS_SIZE);
#endif
								//printk("%d\n", rx_fifo_cnt_w[chid]-j);
							}
							else
							{
#ifdef USE_MEM64_OP
								memset64s(rx_fifo[chid][(PCM_FIFO_SIZE-j+rx_fifo_cnt_w[chid])], 0, PCM_PERIOD_10MS_SIZE/2);
#else                                                                
								memset(rx_fifo[chid][(PCM_FIFO_SIZE-j+rx_fifo_cnt_w[chid])], 0, PCM_PERIOD_10MS_SIZE);
#endif                                                                
								//printk("%d\n", PCM_FIFO_SIZE-j+rx_fifo_cnt_w[chid]);
							}
						}
#endif // #if 1							
	
						clean_forward[chid] = 0;
					}
	
#endif //DTMF_REMOVAL_FORWARD
#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
					//if( bWideband )
					//	memset( rxBuf_NB, 0, PCM_PERIOD_10MS_SIZE );
					//else
						memset(rxBuf, 0, PCM_PERIOD_10MS_SIZE*nWidebandFactor);
#else
#ifdef USE_MEM64_OP
					memset64s(rxBuf, 0, PCM_PERIOD_10MS_SIZE/2);
#else                                        
					memset(rxBuf, 0, PCM_PERIOD_10MS_SIZE);
#endif                                        
#endif
					dtmf_removal_cnt[chid] = 0;
				}
				else	
				{	/* To futher remove 30 ms after dtmf_removal =0 
				         * to remove the de-bounce annoying sound of the 
				         * telephone number buttons.
				         */
				         
					if (dtmf_removal_cnt[chid]< 3)
					{
#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
						//if( bWideband )
						//	memset(rxBuf_NB, 0, PCM_PERIOD_10MS_SIZE);
						//else
							memset(rxBuf, 0, PCM_PERIOD_10MS_SIZE * nWidebandFactor);
#else
#ifdef USE_MEM64_OP
						memset64s(rxBuf, 0, PCM_PERIOD_10MS_SIZE/2);
#else
						memset(rxBuf, 0, PCM_PERIOD_10MS_SIZE);
#endif                                                
#endif
						dtmf_removal_cnt[chid]++;
					}
					else
					{
						clean_forward[chid] = 1;
					}
				}
			
			}
		}	
		
#endif // DTMF_REMOVAL_ISR

		
#if 0//def CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
		//if( bWideband )
		//	pRxFifoCurWriting = rxBuf_NB;
		//else
		//	pRxFifoCurWriting = rxBuf;
#else
			pRxFifoCurWriting = rxBuf;
#endif

		/***** Clean PCM Rx (CED) *****/
		
		/*
		 * Clean PCM rx:
		 * 1. detect fax ced tone     -> clean 2 seconds
		 * 2. 2.1k cnt > 2 (80ms)     -> clean, until modem or fax flag  
		 */
		if( clean_pcm_rx_flag[ chid ] &&
			( ( clean_pcm_rx_cnt[ chid ] ) || ( fax_2100_cnt_tx[chid] > 2 ) ) )
		{
			if( clean_pcm_rx_cnt[ chid ] )
				clean_pcm_rx_cnt[ chid ] --;
			
			/* clean CED and its gap while switching codec */
#ifdef USE_MEM64_OP
			memset64s(pRxFifoCurWriting, 0, PCM_PERIOD_10MS_SIZE/2 * nWidebandFactor);
#else			
			memset(pRxFifoCurWriting, 0, PCM_PERIOD_10MS_SIZE * nWidebandFactor);
#endif			
		} 

//label_skip_clean_PCM_RX:
//		;

		// pkshih: process narrowband data 
#ifdef FSK_TYPE2_ACK_CHECK
		if (fsk_alert_done[chid] == 1)
		{
#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
			//if( bWideband )
			//	memset(rxBuf_NB, 0, PCM_PERIOD_10MS_SIZE);
			//else
				memset(rxBuf, 0, PCM_PERIOD_10MS_SIZE * nWidebandFactor);
#else
#ifdef USE_MEM64_OP
			memset64s(rxBuf, 0, PCM_PERIOD_10MS_SIZE/2);
#else
			memset(rxBuf, 0, PCM_PERIOD_10MS_SIZE);
#endif
#endif
		}
#endif


#if 0//def CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
		if( bWideband ) {
			
			resampler_process_int_ex(pResampler_up_st, 
				PCMRX_RESAMPLER_OFFSET + chid, // see MAX_NB_CHANNELS to know detail  
				rxBuf_NB, PCM_PERIOD_10MS_SIZE/2, 
				rxBuf+(nWidebandFactor*i*PCM_PERIOD_10MS_SIZE>>2), PCM_PERIOD_10MS_SIZE/2*2,
				"PCM-RX up error = %d\n" );
		}
#endif

	/**** Count the Rx FIFO write index ****/
	        //rx_fifo_cnt_w[chid] = (rx_fifo_cnt_w[chid] +1)% PCM_FIFO_SIZE;  

        
	}	//for (i=0; i < PCM_PERIOD; i++) 
	



#ifdef SUPPORT_LEC_G168_ISR
#if DMEN_LEC
	LEC_dmem_save(chid);
#endif
#endif

		


#else	//=======================================================================//

#ifdef SUPPORT_CUT_DSP_PROCESS

	for (i=0; i<40; i++)
#else
	for (i=0; i<(int)(nPcmIntPeriod[chid] << 2); i++)
#endif
	{   
		RxBufTmp[chid][i] = rxBuf[rx_rp++];
	}	
		
#endif	//SUPPORT_PCM_FIFO
	//=======================================================================//
	
	//printk("(%d, %d) ", rx_fifo_cnt_w[chid], rx_fifo_cnt_r[chid]);
	
#ifdef FEATURE_COP3_PCM_RX
	if (cp3_voip_param.bCp3Count_PCM_RX == 1)
	{
		ProfileExitPoint(PROFILE_INDEX_PCM_RX);
		ProfilePerDump(PROFILE_INDEX_PCM_RX, cp3_voip_param.cp3_dump_period);
	}
#endif
	
	return SUCCESS;
}

static int lec_flag_bak[MAX_DSP_RTK_CH_NUM] = {[0 ... MAX_DSP_RTK_CH_NUM-1]=1};
static int fax_modem_flag_bak[MAX_DSP_RTK_CH_NUM] = {[0 ... MAX_DSP_RTK_CH_NUM-1]=1};
//static int dtmf_flag_bak[MAX_DSP_RTK_CH_NUM] = {[0 ... MAX_DSP_RTK_CH_NUM-1]=1};
static int cid_det_flag_bak[MAX_DSP_RTK_CH_NUM] = {[0 ... MAX_DSP_RTK_CH_NUM-1]=1};
static int tone_det_flag_bak[MAX_DSP_RTK_CH_NUM] = {[0 ... MAX_DSP_RTK_CH_NUM-1]=1};

void DisableDspInPcmIsr( voip_dsp_t *this )
{
	const uint32 chid = this ->dch;
	
	// backup
	lec_flag_bak[chid] = support_lec_g168[chid];
	fax_modem_flag_bak[chid] = fax_modem_det[chid];
	//dtmf_flag_bak[chid] = dtmf_chid[chid];
	cid_det_flag_bak[chid] = caller_id_det[chid];
	tone_det_flag_bak[chid] = tone_det[chid];
	
	// FXS
	support_lec_g168[chid] = 0;	// lec
	fax_modem_det[chid] = 0;	// fax/modem
	//dtmf_chid[chid] = 0;		// dtmf det
	
	// FXO
	caller_id_det[chid] = 0;	// caller id det
	tone_det[chid] = 0;		// tone det

}

void RestoreDspInPcmIsr( voip_dsp_t *this )
{
	const uint32 chid = this ->dch;
	
	support_lec_g168[chid] = lec_flag_bak[chid];
	fax_modem_det[chid] = fax_modem_flag_bak[chid];
	//dtmf_chid[chid] = dtmf_flag_bak[chid];
	caller_id_det[chid] = cid_det_flag_bak[chid];
	tone_det[chid] = tone_det_flag_bak[chid];	
}

void dsp_rtk_isr_init_var( void )
{
	// move from init_var() of pcm_interface.c 
	extern voip_dsp_t *get_voip_rtk_dsp( uint32 dch );
	
	int i;
	voip_dsp_t *p_dsp;
	voip_con_t *p_con;
	voip_snd_t *p_snd;
	
	for( i = 0; i < DSP_RTK_CH_NUM; i ++ ) {
		
		p_dsp = get_voip_rtk_dsp( i );
		p_con = p_dsp ->con_ptr;
		p_snd = ( p_con ? p_con ->snd_ptr : NULL );
	
		//if ((i - (SLIC_CH_NUM-1)) > 0)	// DAA
		if( Is_DAA_snd( p_snd ) )
		{
			caller_id_det[i] = 1;
			tone_det[i] = 1;
			fax_modem_det[i] = 0;
		}
		else	// SLIC
		{
			caller_id_det[i] = 0;
			tone_det[i] = 0;
			fax_modem_det[i] = 1;
		}
		
		// init for DSP backup flag
		//DisableDspInPcmIsr_dch(i);
		//RestoreDspInPcmIsr_dch(i);
		p_dsp ->dsp_ops ->disable_dsp_in_ISR( p_dsp );
		p_dsp ->dsp_ops ->restore_dsp_in_ISR( p_dsp );

#ifdef SUPPORT_SLIC_GAIN_CFG
		g_txVolumneGain[ i ] = 6;// init value: 0dB
 		g_rxVolumneGain[ i ] = 6;// init value: 0dB	
#endif

	}
}

