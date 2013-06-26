#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <linux/delay.h>  	// udelay
#include <net/sock.h>

#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_control.h"
#include "voip_params.h"
#include "voip_mgr_define.h"
#include "voip_mgr_netfilter.h"
#include "voip_mgr_events.h"
#ifndef CONFIG_DEFAULTS_KERNEL_2_6
#include "voip_mgr_do_dsp.h"
#endif

#ifdef T38_STAND_ALONE_HANDLER
#include "../voip_drivers/t38_handler.h"
#endif /* T38_STAND_ALONE_HANDLER */
#include "../voip_dsp/ivr/ivr.h"
//#include "../voip_drivers/Daa_api.h"
#include "snd_mux_daa.h"
#include "snd_help.h"
#include "../voip_drivers/fsk.h"
#ifndef AUDIOCODES_VOIP
#include "../voip_dsp/include/fskcid_gen.h"
#endif
#ifdef SW_DTMF_CID
#include "../voip_drivers/dsp_rtk_caller.h"
#endif
#ifdef CONFIG_RTK_VOIP_LED
#include "../voip_drivers/led.h"
#endif

#if ! defined (AUDIOCODES_VOIP)
#ifdef SUPPORT_LEC_G168_ISR
#include "../voip_dsp/include/lec.h"
#endif
#include "../voip_dsp/dsp_r0/dspparam.h"
#include "../voip_dsp/include/dtmf_dec.h"
#include "../voip_dsp/dsp_r0/dspcodec_0.h"
#ifdef FXO_BUSY_TONE_DET
#include "../voip_dsp/include/tone_det_i.h"
#endif

#include "silence_det.h"

#else

#include "RTK_AC49xApi_Interface.h"

#endif

#ifdef VOIP_RESOURCE_CHECK
#include "voip_resource_check.h"
#endif

#include "v152_api.h"

#include "con_mux.h"

#include "snd_define.h"

#if ! defined (AUDIOCODES_VOIP)

#ifdef FXO_CALLER_ID_DET
extern int fsk_cid_valid[MAX_DSP_RTK_CH_NUM];
extern int dtmf_cid_valid[MAX_DSP_RTK_CH_NUM];
#endif

TstVoipPayLoadTypeConfig astVoipPayLoadTypeConfig[MAX_DSP_RTK_SS_NUM];

#else

TstDtmfClid dtmf_cid_info[MAX_DSP_AC_CH_NUM];


#endif

extern long voice_gain_spk[];//0 is mute, 1 is -31dB ~~~ 32 is 0dB , 33 is 1dB ~~~ 63 is 31dB
extern long voice_gain_mic[];

int eng_det_flag[MAX_DSP_CH_NUM] = {0};

#if ! defined (AUDIOCODES_VOIP)
#ifdef PCM_LOOP_MODE_CONTROL
extern TstVoipLoopBcakInfo LoopBackInfo[DSP_RTK_SS_NUM];
extern int loop_3way[DSP_RTK_CH_NUM];
#endif

#ifdef VOIP_RESOURCE_CHECK
extern int resource_weight[DSP_RTK_SS_NUM];// = {0};
#endif

int g_dynamic_pt_remote[DSP_RTK_SS_NUM] = {0};
int g_dynamic_pt_local[DSP_RTK_SS_NUM]={0};
#ifdef SUPPORT_V152_VBD
int g_dynamic_pt_remote_vbd[DSP_RTK_SS_NUM] = {0};
int g_dynamic_pt_local_vbd[DSP_RTK_SS_NUM]={0};
#endif
#endif	// !AUDIOCODES_VOIP

#ifdef CONFIG_RTK_VOIP_IPC_ARCH
#include "ipc_arch_tx.h"
#endif

// Audiocodes also use our T.38
unsigned int fax_end_flag[MAX_DSP_CH_NUM];      //for t.38 fax end detect.

//extern TstTwoChannelCfg astTwoChannelCfg[VOIP_CH_NUM];	// pkshih: unused now

extern int g_SIP_Info_play[];		/* 0: stop 1: start play */
extern int g_SIP_Info_tone_buf[][10];
extern int g_SIP_Info_time_buf[][10];
extern int g_SIP_Info_buf_w[];
extern int g_SIP_Info_buf_r[];

extern uint32 cust;

extern int Is_DAA_Channel(int chid);
extern void init_softfskcidGen(uint32_t chid);

//-------------- For FAX Detection -------------
uint32 fax_modem_det_mode[MAX_DSP_CH_NUM] = {[0 ... MAX_DSP_CH_NUM-1] = 3}; /* fax modem det mode, 0:auto. 1:fax. 2:modem, 3: auto2 */

#if ! defined (AUDIOCODES_VOIP)

//-------------- For FAX Detection -------------
extern unsigned char fax_offhook[];
//----------- For dtmf cid generation ----------
#ifdef SW_DTMF_CID
extern char dtmf_cid_state[];
extern char cid_str[];
#endif
//----------- For dtmf removal -----------------
extern char dtmf_mode[];
extern unsigned char dtmf_removal_flag[];
//----------------------------------------------

extern char cid_dtmf_mode[];

/* agc */
extern unsigned char spk_agc_mode[];
extern unsigned char spk_agc_lvl[];
extern unsigned char spk_agc_gup[];
extern unsigned char spk_agc_gdown[];
extern unsigned char spk_agc_adaptive_threshold[];
extern unsigned char mic_agc_mode[];
extern unsigned char mic_agc_lvl[];
extern unsigned char mic_agc_gup[];
extern unsigned char mic_agc_gdown[];
extern unsigned char mic_agc_adaptive_threshold[];
#else

extern uint32 gSetByassMode[];

extern char dtmf_mode[MAX_DSP_AC_CH_NUM];
long auto_cid_det[MAX_DSP_AC_CH_NUM] = {0};
long cid_type[MAX_DSP_AC_CH_NUM] = {0};

int fsk_spec_areas[MAX_DSP_AC_CH_NUM];	// bit0-2: FSK Type
			// bit 3: Normal Ring
			// bit 4: Fsk Alert Tone
			// bit 5: Short Ring
			// bit 6: Line Reverse
			// bit 7: Date, Time Sync and Name
			// bit 8: Auto SLIC Action
int fsk_cid_gain[MAX_DSP_AC_CH_NUM] = {1};
char dtmf_cid_state[MAX_DSP_AC_CH_NUM]={0};
unsigned char cid_str[21];
char cid_dtmf_mode[MAX_DSP_AC_CH_NUM];     // for DTMF start/end digit
int tone_idx;
int CustomToneTable[8][24];
timetick_t gFirstRingOffTimeOut[MAX_DSP_AC_CH_NUM];
unsigned char gRingGenAfterCid[MAX_DSP_AC_CH_NUM] = {0};
extern int gRingCadOff[MAX_DSP_AC_CH_NUM];


/* agc */
unsigned char spk_agc_mode[MAX_DSP_AC_CH_NUM];
unsigned char mic_agc_mode[MAX_DSP_AC_CH_NUM];
unsigned char agc_enable[MAX_DSP_AC_CH_NUM]={0};
unsigned char spk_agc_gup;
unsigned char spk_agc_gdown;
unsigned char mic_agc_gup;
unsigned char mic_agc_gdown;

#endif /* AUDIOCODES_VOIP */

extern int pulse_dial_in_cch(uint32 ch_id, char input);

timetick_t gChkRingDelay = 0;

#ifdef AUDIOCODES_VOIP
int SaveCustomTone(TstVoipToneCfg *pToneCfg)
{
	/*
	TONE_TYPE_ADDITIVE = 0
    	TONE_TYPE_MODULATED = 1
    	TONE_TYPE_SUCC = 2
    	TONE_TYPE_SUCC_ADD =3
    	*/

	if (pToneCfg->toneType == 2)//SUCC
	{
		CustomToneTable[tone_idx][0] = CALL_PROGRESS_SIGNAL_TYPE__SPECIAL_INFORMATION_TONE ;
		//PRINT_MSG("SPECIAL_INFO...\n");
	}
	else
	{
		if (pToneCfg->cycle == 0)
		{
			CustomToneTable[tone_idx][0] = CALL_PROGRESS_SIGNAL_TYPE__CONTINUOUS;
			//PRINT_MSG("CONTINUOUS...\n");
		}
		else if (pToneCfg->cycle == 1)
		{
			CustomToneTable[tone_idx][0] = CALL_PROGRESS_SIGNAL_TYPE__BURST; // play one cycle
			//PRINT_MSG("BURST...\n");
		}
		else if (pToneCfg->cycle > 1)
		{
			CustomToneTable[tone_idx][0] = CALL_PROGRESS_SIGNAL_TYPE__CADENCE;
			//PRINT_MSG("CADENCE...\n");
		}
	}

	CustomToneTable[tone_idx][1] = pToneCfg->Freq1;		//ToneAFrequency
	CustomToneTable[tone_idx][2] = pToneCfg->Freq2;		//ToneB_OrAmpModulationFrequency
	CustomToneTable[tone_idx][3] = 10;			//TwistThreshold

	if (pToneCfg->toneType == 2)//SUCC
		CustomToneTable[tone_idx][4] = pToneCfg->Freq3;	//ThirdToneOfTripleBatchDurationTypeFrequency
	else
		CustomToneTable[tone_idx][4] = 0;
	//PRINT_MSG("Freq3=%d\n", CustomToneTable[tone_idx][4]);


	CustomToneTable[tone_idx][5] = 0;			//HighEnergyThreshold
	CustomToneTable[tone_idx][6] = 35;			//LowEnergyThreshold
	CustomToneTable[tone_idx][7] = 15;			//SignalToNoiseRatioThreshold
	CustomToneTable[tone_idx][8] = 10;			//FrequencyDeviationThreshold
	CustomToneTable[tone_idx][9] = pToneCfg->Gain1;		//ToneALevel

	if ((pToneCfg->toneType == 1) || pToneCfg->toneType == 2)//Modulate or SUCC
		CustomToneTable[tone_idx][10] = 0;		//ToneBLevel
	else
		CustomToneTable[tone_idx][10] = pToneCfg->Gain2;

	//PRINT_MSG("ToneBLevel=%d\n", CustomToneTable[tone_idx][10]);

	/*
	AM Factor is the AM Modulation index, its range us between 0%-100%
	AMFACTOR field range is between 0-50 (unit 0.02) corresponding to 0%-100%.
	In the example I chose AMFACTOR=25 (50%)
	*/
       	if (pToneCfg->toneType == 1)//Modulate
		CustomToneTable[tone_idx][11] = 25;		//AmFactor
	else
		CustomToneTable[tone_idx][11] = 0;

	//PRINT_MSG("AmFactor=%d\n", CustomToneTable[tone_idx][11]);

	CustomToneTable[tone_idx][12] = (pToneCfg->CadOn0)/10;	//DetectionTimeOrCadenceFirstOnOrBurstDuration
	CustomToneTable[tone_idx][13] = (pToneCfg->CadOff0)/10;	//CadenceFirstOffDuration
	CustomToneTable[tone_idx][14] = (pToneCfg->CadOn1)/10;	//CadenceSecondOnDuration
	CustomToneTable[tone_idx][15] = (pToneCfg->CadOff1)/10;	//CadenceSecondOffDuration
	CustomToneTable[tone_idx][16] = (pToneCfg->CadOn2)/10;	//CadenceThirdOnDuration
	CustomToneTable[tone_idx][17] = (pToneCfg->CadOff2)/10;	//CadenceThirdOffDuration
	CustomToneTable[tone_idx][18] = (pToneCfg->CadOn3)/10;	//CadenceFourthOnDuration
	CustomToneTable[tone_idx][19] = (pToneCfg->CadOff3)/10;	//CadenceFourthOffDuration
	CustomToneTable[tone_idx][20] = 0;			//CadenceVoiceAddedWhileFirstOff
	CustomToneTable[tone_idx][21] = 0;			//CadenceVoiceAddedWhileSecondOff
	CustomToneTable[tone_idx][22] = 0;			//CadenceVoiceAddedWhileThirdOff
	CustomToneTable[tone_idx][23] = 0;			//CadenceVoiceAddedWhileFourthOff

#if 0
	pToneCfg->cadNUM;
	pToneCfg->PatternOff;
	pToneCfg->ToneNUM;
#endif
	return 0;
}
#endif /* AUDIOCODES_VOIP */

/**
 * @ingroup VOIP_DSP_GENERAL
 * @brief Re-initialize variables when on-hook
 * @param TstVoipCfg.ch_id Channel ID
 * @see VOIP_MGR_ON_HOOK_RE_INIT TstVoipCfg
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_ON_HOOK_RE_INIT( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	extern void Init_CED_Det(unsigned char CH);	//thlin+ 2006-02-08
	extern void AEC_re_init(unsigned int chid);
	extern void NLP_g168_init(unsigned int chid);
	extern void NR_re_init(unsigned int chid);
	extern int reinit_answer_tone_det(unsigned int chid);

#ifdef FXO_CALLER_ID_DET
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	int i;
#endif
#endif
	TstVoipCfg stVoipCfg;
	int ret;

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	uint32 SessNum, ssid, j;
#endif

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

	PRINT_MSG("VOIP_MGR_ON_HOOK_RE_INIT:ch_id = %d, enable = %d\n", stVoipCfg.ch_id, stVoipCfg.enable);

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward, also do below
#else

	Init_CED_Det(stVoipCfg.ch_id);
	reinit_answer_tone_det(stVoipCfg.ch_id);

#ifdef SUPPORT_LEC_G168_ISR
	LEC_re_init(stVoipCfg.ch_id);
#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
#ifdef T_TYPE_ECHO_CAN
	LEC_re_init(3);
#endif
#endif
#endif

//#ifdef EXPER_AEC
	AEC_re_init(stVoipCfg.ch_id);
//#endif

#ifdef RTK_VOICE_RECORD
	extern int ec128_pattern_index;
	extern int ec128_start_count;
	ec128_pattern_index = 0;
	ec128_start_count = 0;
#endif

#ifdef EXPER_NR
	NR_re_init(stVoipCfg.ch_id);
#endif
#if 0
	frequency_echo_state_reset(stVoipCfg.ch_id);
#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
	AEC_re_init(stVoipCfg.ch_id);
	NLP_g168_init(stVoipCfg.ch_id);
	NR_re_init(stVoipCfg.ch_id);
#endif

	SessNum = chanInfo_GetRegSessionNum(stVoipCfg.ch_id);
	for (j=0; j < SessNum; j++)
	{
		ssid = chanInfo_GetRegSessionID(stVoipCfg.ch_id, j);
		RFC2833_receiver_init(ssid);
//#ifdef RFC2833_PROC_DEBUG_SUPPORT
		extern void rfc2833_proc_tx_cnt_reset(uint32 sid);
		rfc2833_proc_tx_cnt_reset(ssid);
//#endif
	}

#ifdef SUPPORT_FAX_V21_DETECT
	init_fax_v21(stVoipCfg.ch_id);
#endif
	//add reset t.38 detect fax end flag when oh-hook
	fax_end_flag[stVoipCfg.ch_id]=0;
	//add reset t.38 detect fax end flag when oh-hook
	fax_end_flag[stVoipCfg.ch_id]=0;

	// restore LEC setting
	extern unsigned char support_lec_g168[], support_lec_g168_bk[];
	support_lec_g168[stVoipCfg.ch_id] = support_lec_g168_bk[stVoipCfg.ch_id];
#ifdef FXO_CALLER_ID_DET
	dmtf_cid_det_init(stVoipCfg.ch_id);
	init_cid_det_si3500(stVoipCfg.ch_id);
	for (i=0; i < CON_CH_NUM; i++)
	{
		if( get_snd_type_cch( i ) != SND_TYPE_FXS )
			continue;

		fsk_cid_valid[i] = 0;
		dtmf_cid_valid[i] = 0;
	}
#endif

#endif

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	voip_event_flush_fax_modem_fifo(stVoipCfg.ch_id, 0);
#else
	voip_event_flush_fax_modem_fifo(stVoipCfg.ch_id, 1);
#endif

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	fsk_cid_state[stVoipCfg.ch_id] = 0;
	init_softfskcidGen(stVoipCfg.ch_id);

#endif


	return 0;
}
#else
int do_mgr_VOIP_MGR_ON_HOOK_RE_INIT( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCfg stVoipCfg;
	int ret;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward, also do below
#else
	dtmf_cid_state[stVoipCfg.ch_id] = 0;
	gSetByassMode[stVoipCfg.ch_id] = 0;
#endif
	//add reset t.38 detect fax end flag when oh-hook
	fax_end_flag[stVoipCfg.ch_id]=0;

	//add reset t.38 detect fax end flag when oh-hook
	fax_end_flag[stVoipCfg.ch_id]=0;
	// restore LEC setting
	extern unsigned char support_lec_g168[], support_lec_g168_bk[];
	support_lec_g168[stVoipCfg.ch_id] = support_lec_g168_bk[stVoipCfg.ch_id];

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	voip_event_flush_fax_modem_fifo(stVoipCfg.ch_id, 0);
#else
	voip_event_flush_fax_modem_fifo(stVoipCfg.ch_id, 1);
#endif

	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_GENERAL
 * @brief Set voice (speaker/mic) gain
 * @param TstVoipValue.value Speaker gain <br>
 *        (0 is mute, 1 is -31dB ~~~ 32 is 0dB , 33 is 1dB ~~~ 63 is 31dB)
 * @param TstVoipValue.value1 MIC gain <br>
 *        (0 is mute, 1 is -31dB ~~~ 32 is 0dB , 33 is 1dB ~~~ 63 is 31dB)
 * @see VOIP_MGR_SET_VOICE_GAIN TstVoipValue
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_VOICE_GAIN( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	voice_gain_spk[stVoipValue.ch_id]=stVoipValue.value+32;//0 is mute, 1 is -31dB ~~~ 32 is 0dB , 33 is 1dB ~~~ 63 is 31dB
	voice_gain_mic[stVoipValue.ch_id]=stVoipValue.value1+32;//0 is mute, 1 is -31dB ~~~ 32 is 0dB , 33 is 1dB ~~~ 63 is 31dB
	//stVoipValue.ret_val = 0;
#endif
	//copy_to_user(user, &stVoipValue, sizeof(TstVoipValue));
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_VOICE_GAIN( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	Tac49xVoiceGain mic_gain, spk_gain;
#endif

	/* Voice Gain Adjustment */
	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	spk_gain = stVoipValue.value+32;  //0 is mute, 1 is -31dB ~~~ 32 is 0dB , 33 is 1dB ~~~ 63 is 31dB
	mic_gain = stVoipValue.value1+32; //0 is mute, 1 is -31dB ~~~ 32 is 0dB , 33 is 1dB ~~~ 63 is 31dB

	RtkAc49xApiSetVoiceGain(stVoipValue.ch_id, mic_gain, spk_gain);
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_GENERAL
 * @brief Enable/disable energy detection, and retrieve detected energy
 * @param TstVoipValue.ch_id Channel ID
 * @param TstVoipValue.value 1: enable, 0: disable - energy detect
 * @see VOIP_MGR_ENERGY_DETECT TstVoipValue
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_ENERGY_DETECT( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef ENERGY_DET_PCM_IN
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	unsigned long flags;
#endif
	TstVoipValue stVoipValue;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	save_flags(flags); cli();
	/*
	energy_out return value : 0~ 91 dB.
	If return value = -1, it means FIFO is empty.
	*/
	eng_det_flag[stVoipValue.ch_id] = stVoipValue.value; // 1: enable, 0: disable - energy detect
	restore_flags(flags);
#endif
	return COPY_TO_USER(user, &stVoipValue, sizeof(TstVoipValue), cmd, seq_no);
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_ENERGY_DETECT( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef ENERGY_DET_PCM_IN
	unsigned long flags;
	TstVoipValue stVoipValue;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	save_flags(flags); cli();
	/*
	energy_out return value : max is 0dBm.
	If return value = -1, it means FIFO is empty.
	*/
	eng_det_flag[stVoipValue.ch_id] = stVoipValue.value; // 1: enable, 0: disable - energy detect
	restore_flags(flags);
#endif
	return COPY_TO_USER(user, &stVoipValue, sizeof(TstVoipValue), cmd, seq_no);
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_GENERAL
 * @brief Retrieve VoIP Event, such as DTMF, hook, FAX/MODEM, and so on.
 * @param [in] TstVoipEvent.ch_id Channel ID
 * @param [in] TstVoipEvent.type Event type filter 
 * @param [in/out] TstVoipEvent.mask Mask session for RFC2833 and DSP, or in/out for DTMF 
 * @param [out] TstVoipEvent.id Event ID
 * @param [out] TstVoipEvent.p0 Event parameter 0
 * @param [out] TstVoipEvent.p1 Event parameter 1
 * @param [out] TstVoipEvent.time Event time (in unit of ms)
 * @see VOIP_MGR_GET_VOIP_EVENT TstVoipEvent 
 */
int do_mgr_VOIP_MGR_GET_VOIP_EVENT( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	unsigned long flags;
	TstVoipEvent stVoipEvent;
	
	COPY_FROM_USER(&stVoipEvent, (TstVoipEvent *)user, sizeof(TstVoipEvent));
	
#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP_IS_DSP
	// Host only 
#else
	save_flags(flags); cli();
	
	voip_mgr_event_out( &stVoipEvent );
	
	restore_flags(flags);
#endif
	return COPY_TO_USER(user, &stVoipEvent, sizeof(TstVoipEvent), cmd, seq_no);

}

/**
 * @ingroup VOIP_DSP_GENERAL
 * @brief Flushing VoIP Event, such as DTMF, hook, FAX/MODEM, and so on.
 * @param [in] TstFlushVoipEvent.ch_id Channel ID
 * @param [in] TstFlushVoipEvent.type Event type filter 
 * @param [in] TstFlushVoipEvent.mask Mask session for RFC2833 and DSP, or in/out for DTMF 
 * @see VOIP_MGR_FLUSH_VOIP_EVENT TstVoipEvent 
 */
int do_mgr_VOIP_MGR_FLUSH_VOIP_EVENT( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	unsigned long flags;
	TstFlushVoipEvent stFlushVoipEvent;
	
	COPY_FROM_USER(&stFlushVoipEvent, (TstFlushVoipEvent *)user, sizeof(TstFlushVoipEvent));
	
	save_flags(flags); cli();
	
	voip_mgr_event_flush( &stFlushVoipEvent );
	
	restore_flags(flags);
	
	return COPY_TO_USER(user, &stFlushVoipEvent, sizeof(TstFlushVoipEvent), cmd, seq_no);
}

/**
 * @ingroup VOIP_DSP_CODEC
 * @brief Set payload type (codec type), jitter buffer factor, VAD, frames per packet, PCM mode, and so on
 * @see VOIP_MGR_SETRTPPAYLOADTYPE TstVoipPayLoadTypeConfig
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SETRTPPAYLOADTYPE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	extern int g_force_codec;
	extern int g_force_vad;
	extern int g_force_ptime;
	extern int g_force_PcmMode;
	extern int g_force_g7111mode;

	unsigned long flags;
	uint32 ch_id, m_id, s_id;
#ifdef VOIP_RESOURCE_CHECK
	int pltype;
#endif
#endif
	TstVoipPayLoadTypeConfig stVoipPayLoadTypeConfig;
	int ret = 0;
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	unsigned int mgr_chid;
#endif


	COPY_FROM_USER(&stVoipPayLoadTypeConfig, (TstVoipPayLoadTypeConfig *)user, sizeof(TstVoipPayLoadTypeConfig));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

	PRINT_MSG("VOIP_MGR_SETRTPPAYLOADTYPE\n");

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST

#ifdef T38_STAND_ALONE_HANDLER
	if( stVoipPayLoadTypeConfig.uPktFormat == rtpPayloadT38_Virtual )
	{
		//T38_API_Initialize( ch_id, NULL );
		PRINT_MSG("MGR: Initialize T38(%d)\n", stVoipPayLoadTypeConfig.ch_id);
		t38RunningState[ stVoipPayLoadTypeConfig.ch_id ] = T38_START;
		//enable_silence_det( stVoipPayLoadTypeConfig.ch_id, 1 );	// dsp side only 
	}
	else
	{
		//enable_silence_det( stVoipPayLoadTypeConfig.ch_id, 0 );	// dsp side only 
		t38RunningState[ stVoipPayLoadTypeConfig.ch_id ] = T38_STOP;
		PRINT_MSG("T38 Stop(%d)\n", stVoipPayLoadTypeConfig.ch_id);
	}
#endif

	// Send Control Packet and wait Response Packet
	mgr_chid = stVoipPayLoadTypeConfig.ch_id;
	stVoipPayLoadTypeConfig.ch_id = API_get_DSP_CH(cmd, stVoipPayLoadTypeConfig.ch_id);	// convert to DSP chid
	ret = ipcSentControlPacket(cmd, mgr_chid, &stVoipPayLoadTypeConfig, sizeof(TstVoipPayLoadTypeConfig));
	stVoipPayLoadTypeConfig.ch_id = mgr_chid;
#else

	ch_id = stVoipPayLoadTypeConfig.ch_id;
	m_id = stVoipPayLoadTypeConfig.m_id;
	s_id = API_GetSid(ch_id, m_id);
#ifdef PCM_LOOP_MODE_CONTROL
	if(LoopBackInfo[s_id].isLoopBack == 1) {
 #ifdef CONFIG_RTK_VOIP_SILENCE
		stVoipPayLoadTypeConfig.uPktFormat = rtpPayloadSilence;
 #else
		return 0;
 #endif
	}
#endif

#ifdef VOIP_RESOURCE_CHECK
	pltype = stVoipPayLoadTypeConfig.uPktFormat;

	if ( 1 == GetCurrentVoipResourceStatus(pltype))//VOIP_RESOURCE_AVAILABLE
	{
		SetVoipResourceWeight( s_id, pltype );
	}
	else //VOIP_RESOURCE_UNAVAILABLE
	{
		/* play IVR to user*/
#ifdef CONFIG_RTK_VOIP_IVR_TEXT
		char text[]={IVR_TEXT_ID_NO_RESOURCE, '\0'};
		extern unsigned int PlayIvrText2Speech( unsigned int chid, IvrPlayDir_t dir, const unsigned char *pText2Speech );
		PlayIvrText2Speech(ch_id, IVR_DIR_LOCAL, text);

		while(PollIvrPlaying(ch_id)){};
#endif
		hc_SetPlayTone(ch_id, s_id, DSPCODEC_TONE_SIT_NOCIRCUIT, true, DSPCODEC_TONEDIRECTION_LOCAL);

		stVoipPayLoadTypeConfig.uPktFormat = rtpPayloadSilence;
		resource_weight[s_id] = DEFAULT_WEIGHT;
	}


#endif

#if 0
	PRINT_MSG("ch_id = %d\n",ch_id);
	PRINT_MSG("m_id = %d\n",m_id);
	PRINT_MSG("s_id = %d\n",s_id);
	PRINT_MSG("uPktFormat = %d\n",stVoipPayLoadTypeConfig.uPktFormat);
	PRINT_MSG("nFramePerPacket= %d\n",stVoipPayLoadTypeConfig.nFramePerPacket);
	PRINT_MSG("nG723Type = %d\n",stVoipPayLoadTypeConfig.nG723Type);
	PRINT_MSG("bVAD = %d\n",stVoipPayLoadTypeConfig.bVAD);
	PRINT_MSG("nG726Packing = %d\n",stVoipPayLoadTypeConfig.nG726Packing);
	PRINT_MSG("nG7111Mode = %d\n",stVoipPayLoadTypeConfig.nG7111Mode);
	PRINT_MSG("nPcmMode = %d\n",stVoipPayLoadTypeConfig.nPcmMode);
#endif

#if 0 // hard code to set G.729
	stVoipPayLoadTypeConfig.uPktFormat = 18;
	g_dynamic_pt_remote[s_id] = 18;
	g_dynamic_pt_local[s_id] = 18;
	PRINT_MSG("remote pt=%d, locat_pt=%d, sid=%d\n",
		g_dynamic_pt_remote[s_id], g_dynamic_pt_local[s_id], s_id);
#else
	// force codec
	if (g_force_codec != -1)
	{
		stVoipPayLoadTypeConfig.uPktFormat = g_force_codec;
		stVoipPayLoadTypeConfig.remote_pt = g_force_codec;
		stVoipPayLoadTypeConfig.local_pt = g_force_codec;
#ifdef CONFIG_RTK_VOIP_PCM_LINEAR_16K
		if (stVoipPayLoadTypeConfig.uPktFormat == 120)
		{
			stVoipPayLoadTypeConfig.nPcmMode = 3; //WB for PCM Linear 16K
		}
#endif
	}

	g_dynamic_pt_remote[s_id] = stVoipPayLoadTypeConfig.remote_pt;
	g_dynamic_pt_local[s_id] = stVoipPayLoadTypeConfig.local_pt;
	PRINT_MSG("remote pt=%d, locat_pt=%d, sid=%d\n",
		g_dynamic_pt_remote[s_id], g_dynamic_pt_local[s_id], s_id);

	// force vad
	if (g_force_vad != -1)
		stVoipPayLoadTypeConfig.bVAD = g_force_vad;

	// force pktime
	if (g_force_ptime != -1)
		stVoipPayLoadTypeConfig.nFramePerPacket = g_force_ptime;

	// force PCM mode (0: no action, 1: DSP auto, 2: NB, 3:WB)
	if (g_force_PcmMode != -1)
		stVoipPayLoadTypeConfig.nPcmMode = g_force_PcmMode;

	// force g7111 mode
	if (g_force_g7111mode != -1)
		stVoipPayLoadTypeConfig.nG7111Mode = g_force_g7111mode;

#endif

#ifdef SUPPORT_V152_VBD
	switch( stVoipPayLoadTypeConfig.uPktFormat_vbd ) {
	case rtpPayloadPCMU:
	case rtpPayloadPCMA:
		g_dynamic_pt_remote_vbd[s_id] = stVoipPayLoadTypeConfig.remote_pt_vbd;
		g_dynamic_pt_local_vbd[s_id] = stVoipPayLoadTypeConfig.local_pt_vbd;
		break;

	case rtpPayloadUndefined:
	default:
		// others are not support
		stVoipPayLoadTypeConfig.uPktFormat_vbd = rtpPayloadUndefined;
		g_dynamic_pt_remote_vbd[s_id] = g_dynamic_pt_local_vbd[s_id] = rtpPayloadUndefined;
		break;
	}

	PRINT_MSG("VBD: remote pt=%d, locat_pt=%d, PktFormat=%d, sid=%d\n",
		g_dynamic_pt_remote_vbd[s_id], g_dynamic_pt_local_vbd[s_id], stVoipPayLoadTypeConfig.uPktFormat_vbd, s_id);
#endif

	astVoipPayLoadTypeConfig[s_id] = stVoipPayLoadTypeConfig;

#if 0
	/* Now, it is merely an experimental parameter. */
	stVoipPayLoadTypeConfig.nJitterFactor = 7;
#endif

#ifdef SUPPORT_V152_VBD
	V152_InitializeSession( s_id,
		( stVoipPayLoadTypeConfig.uPktFormat_vbd != rtpPayloadUndefined ) );
#endif

	save_flags(flags); cli();
	DSP_CodecRestart(ch_id, s_id,
					 stVoipPayLoadTypeConfig.uPktFormat,
					 stVoipPayLoadTypeConfig.nFramePerPacket,
					 stVoipPayLoadTypeConfig.nG723Type,
					 stVoipPayLoadTypeConfig.bVAD,
					 stVoipPayLoadTypeConfig.bPLC,
					 stVoipPayLoadTypeConfig.nJitterDelay,
					 stVoipPayLoadTypeConfig.nMaxDelay,
					 stVoipPayLoadTypeConfig.nJitterFactor,
					 stVoipPayLoadTypeConfig.nG726Packing,
					 stVoipPayLoadTypeConfig.nG7111Mode,
					 stVoipPayLoadTypeConfig.nPcmMode);
	restore_flags(flags);

#ifdef T38_STAND_ALONE_HANDLER
	if( stVoipPayLoadTypeConfig.uPktFormat == rtpPayloadT38_Virtual ) {
		t38Param_t t38Param = T38_DEFAULT_PARAM_LIST();
		if( stVoipPayLoadTypeConfig.bT38ParamEnable ) {
			t38Param.nMaxBuffer = stVoipPayLoadTypeConfig.nT38MaxBuffer;
			t38Param.nRateManagement = stVoipPayLoadTypeConfig.nT38RateMgt;
			t38Param.nMaxRate = ( stVoipPayLoadTypeConfig.nT38MaxRate > 5 ? 5 : stVoipPayLoadTypeConfig.nT38MaxRate );
			t38Param.pEnableECM = stVoipPayLoadTypeConfig.bT38EnableECM;
			t38Param.pECC_Signal = ( stVoipPayLoadTypeConfig.nT38ECCSignal > 7 ? 5 : stVoipPayLoadTypeConfig.nT38ECCSignal );
			t38Param.pECC_Data = ( stVoipPayLoadTypeConfig.nT38ECCData > 2 ? 2 : stVoipPayLoadTypeConfig.nT38ECCData );
			t38Param.bEnableSpoof = stVoipPayLoadTypeConfig.bT38EnableSpoof;
			t38Param.nDuplicateNum = ( stVoipPayLoadTypeConfig.nT38DuplicateNum > 2 ? 2 : stVoipPayLoadTypeConfig.nT38DuplicateNum );
		}
		T38_API_Initialize( ch_id, &t38Param );
		PRINT_MSG("MGR: Initialize T38\n");
		t38RunningState[ ch_id ] = T38_START;
		enable_silence_det( ch_id, 1 );
	} else {
		enable_silence_det( ch_id, 0 );
		t38RunningState[ ch_id ] = T38_STOP;
	}
#endif
#endif
	return ret;
}
#else
int do_mgr_VOIP_MGR_SETRTPPAYLOADTYPE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	unsigned long flags;
	uint32 ch_id, s_id;
	TstVoipPayLoadTypeConfig stVoipPayLoadTypeConfig;
	int ret = 0;
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	unsigned int mgr_chid;
#else
	int dynamic_pt_remote;
	int dynamic_pt_local;
#endif


	COPY_FROM_USER(&stVoipPayLoadTypeConfig, (TstVoipPayLoadTypeConfig *)user, sizeof(TstVoipPayLoadTypeConfig));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

	PRINT_MSG("VOIP_MGR_SETRTPPAYLOADTYPE(ch=%d, mid=%d)\n", stVoipPayLoadTypeConfig.ch_id, stVoipPayLoadTypeConfig.m_id);

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST

	// Send Control Packet and wait Response Packet
	mgr_chid = stVoipPayLoadTypeConfig.ch_id;
	stVoipPayLoadTypeConfig.ch_id = API_get_DSP_CH(cmd, stVoipPayLoadTypeConfig.ch_id);	// convert to DSP chid
	ret = ipcSentControlPacket(cmd, mgr_chid, &stVoipPayLoadTypeConfig, sizeof(TstVoipPayLoadTypeConfig));
	stVoipPayLoadTypeConfig.ch_id = mgr_chid;

#else
	ch_id = stVoipPayLoadTypeConfig.ch_id;
	s_id = (2*stVoipPayLoadTypeConfig.ch_id + PROTOCOL__RTP - 1);
	save_flags(flags); cli();
	#ifdef T38_STAND_ALONE_HANDLER
	if( stVoipPayLoadTypeConfig.uPktFormat == rtpPayloadT38_Virtual )
	{
		t38Param_t t38Param = T38_DEFAULT_PARAM_LIST();
		if( stVoipPayLoadTypeConfig.bT38ParamEnable ) {
			t38Param.nMaxBuffer = stVoipPayLoadTypeConfig.nT38MaxBuffer;
			t38Param.nRateManagement = stVoipPayLoadTypeConfig.nT38RateMgt;
			t38Param.nMaxRate = ( stVoipPayLoadTypeConfig.nT38MaxRate > 5 ? 5 : stVoipPayLoadTypeConfig.nT38MaxRate );
			t38Param.pEnableECM = stVoipPayLoadTypeConfig.bT38EnableECM;
			t38Param.pECC_Signal = ( stVoipPayLoadTypeConfig.nT38ECCSignal > 7 ? 5 : stVoipPayLoadTypeConfig.nT38ECCSignal );
			t38Param.pECC_Data = ( stVoipPayLoadTypeConfig.nT38ECCData > 2 ? 2 : stVoipPayLoadTypeConfig.nT38ECCData );
			t38Param.bEnableSpoof = stVoipPayLoadTypeConfig.bT38EnableSpoof;
			t38Param.nDuplicateNum = ( stVoipPayLoadTypeConfig.nT38DuplicateNum > 2 ? 2 : stVoipPayLoadTypeConfig.nT38DuplicateNum );
		}
		T38_API_Initialize( ch_id, &t38Param );
		PRINT_MSG("MGR: Initialize T38(%d)\n", stVoipPayLoadTypeConfig.ch_id);
		t38RunningState[ stVoipPayLoadTypeConfig.ch_id ] = T38_START;
		enable_silence_det( stVoipPayLoadTypeConfig.ch_id, 1 );
		restore_flags(flags);
		//stVoipPayLoadTypeConfig.ret_val = 0;
		//copy_to_user(user, &stVoipPayLoadTypeConfig, sizeof(TstVoipPayLoadTypeConfig));
		return 0;
	}
	else {
		enable_silence_det( stVoipPayLoadTypeConfig.ch_id, 0 );
		t38RunningState[ stVoipPayLoadTypeConfig.ch_id ] = T38_STOP;
	}
	#endif

#if 0 // hard code to set G.729
	stVoipPayLoadTypeConfig.uPktFormat = 4;//18;
	g_dynamic_pt_remote[s_id] = 4;//18;
	g_dynamic_pt_local[s_id] = 4;//18;
	PRINT_MSG("remote pt=%d, locat_pt=%d, sid=%d\n",
		g_dynamic_pt_remote[s_id], g_dynamic_pt_local[s_id], s_id);
#else
	dynamic_pt_remote = stVoipPayLoadTypeConfig.remote_pt;
	dynamic_pt_local = stVoipPayLoadTypeConfig.local_pt;
	PRINT_MSG("remote pt=%d, locat_pt=%d, sid=%d\n",
		dynamic_pt_remote, dynamic_pt_local, s_id);
#endif

#ifdef SUPPORT_V152_VBD
	switch( stVoipPayLoadTypeConfig.uPktFormat_vbd ) {
	case rtpPayloadPCMU:
	case rtpPayloadPCMA:
		g_dynamic_pt_remote_vbd[s_id] = stVoipPayLoadTypeConfig.remote_pt_vbd;
		g_dynamic_pt_local_vbd[s_id] = stVoipPayLoadTypeConfig.local_pt_vbd;
		break;

	case rtpPayloadUndefined:
	default:
		// others are not support
		stVoipPayLoadTypeConfig.uPktFormat_vbd = rtpPayloadUndefined;
		g_dynamic_pt_remote_vbd[s_id] = g_dynamic_pt_local_vbd[s_id] = rtpPayloadUndefined;
		break;
	}

	PRINT_MSG("VBD: remote pt=%d, locat_pt=%d, PktFormat=%d, sid=%d\n",
		g_dynamic_pt_remote_vbd[s_id], g_dynamic_pt_local_vbd[s_id], stVoipPayLoadTypeConfig.uPktFormat_vbd, s_id);
#endif

	RtkAc49xApiSetRtpChannelConfiguration(stVoipPayLoadTypeConfig.ch_id, s_id, dynamic_pt_remote, stVoipPayLoadTypeConfig.nG723Type, stVoipPayLoadTypeConfig.bVAD);
	if (CHANNEL_STATE__ACTIVE_RTP != RtkAc49xGetChannelState(stVoipPayLoadTypeConfig.ch_id))
		RtkAc49xApiSetVoiceJBDelay(stVoipPayLoadTypeConfig.ch_id, 10*stVoipPayLoadTypeConfig.nMaxDelay, 10*stVoipPayLoadTypeConfig.nJitterDelay, stVoipPayLoadTypeConfig.nJitterFactor);

	if (gSetByassMode[stVoipPayLoadTypeConfig.ch_id] == 1) // fax bypass
	{
		RtkAc49xApiSetIntoBypassMode(stVoipPayLoadTypeConfig.ch_id, FAX_BYPASS);
	}
	else if (gSetByassMode[stVoipPayLoadTypeConfig.ch_id] == 2) // modem bypass
	{
		RtkAc49xApiSetIntoBypassMode(stVoipPayLoadTypeConfig.ch_id, MODEM_BYPASS);
	}

	restore_flags(flags);
#endif
	return ret;
}
#endif

/**
 * @ingroup VOIP_DSP_CODEC
 * @brief Stop codec
 * @param TstVoipValue.ch_id Channel ID
 * @param TstVoipValue.m_id Media ID
 * @see VOIP_MGR_DSPCODECSTOP TstVoipValue
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_DSPCODECSTOP( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	unsigned long flags;
	uint32 s_id;
#else
	unsigned int mgr_chid;
#endif
	TstVoipValue stVoipValue;
	int ret = 0;

	PRINT_MSG("VOIP_MGR_DSPCODECSTOP\n");
	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST

#ifdef T38_STAND_ALONE_HANDLER
	//enable_silence_det( stVoipValue.ch_id, 0 );	// dsp side only 
	t38RunningState[ stVoipValue.ch_id ] = T38_STOP;
	PRINT_MSG("T38 Stop(%d)\n", stVoipValue.ch_id);
#endif
	// Send Control Packet and wait Response Packet
	mgr_chid = stVoipValue.ch_id;
	stVoipValue.ch_id = API_get_DSP_CH(cmd, stVoipValue.ch_id);	// convert to DSP chid
	ret = ipcSentControlPacket(cmd, mgr_chid, &stVoipValue, sizeof(TstVoipValue));
	stVoipValue.ch_id = mgr_chid;

#else

	s_id = API_GetSid(stVoipValue.ch_id, stVoipValue.m_id);
#if 1
	PRINT_MSG("ch_id =%d, m_id=%d, s_id=%d\n", stVoipValue.ch_id, stVoipValue.m_id, s_id);
#endif
	save_flags(flags); cli();
	DspcodecStop(s_id);
#ifdef T38_STAND_ALONE_HANDLER
	enable_silence_det( stVoipValue.ch_id, 0 );
	t38RunningState[ stVoipValue.ch_id ] = T38_STOP;
#endif
#ifdef SUPPORT_V152_VBD
	V152_InitializeSession( s_id, 0 );
#endif
	restore_flags(flags);
#endif
	return ret;
}
#else
int do_mgr_VOIP_MGR_DSPCODECSTOP( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	unsigned long flags;
#ifdef SUPPORT_V152_VBD
	uint32 s_id;
#endif
	TstVoipValue stVoipValue;
	int ret = 0;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	unsigned int mgr_chid;
#endif

	PRINT_MSG("VOIP_MGR_DSPCODECSTOP\n");
	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST

	// Send Control Packet and wait Response Packet
	mgr_chid = stVoipValue.ch_id;
	stVoipValue.ch_id = API_get_DSP_CH(cmd, stVoipValue.ch_id);	// convert to DSP chid
	ret = ipcSentControlPacket(cmd, mgr_chid, &stVoipValue, sizeof(TstVoipValue));
	stVoipValue.ch_id = mgr_chid;

#else
	save_flags(flags); cli();
#ifdef T38_STAND_ALONE_HANDLER
	enable_silence_det( stVoipValue.ch_id, 0 );
	t38RunningState[ stVoipValue.ch_id ] = T38_STOP;
#endif
#ifdef SUPPORT_V152_VBD
	V152_InitializeSession( s_id, 0 );
#endif
	restore_flags(flags);
#endif
	return ret;
}
#endif

/**
 * @ingroup VOIP_DSP_CODEC
 * @brief Set *global* VAD/CNG threshold 
 * @param 
 * @param 
 * @see VOIP_MGR_DSPCODECSTOP TstVoipValue 
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_VAD_CNG_THRESHOLD( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifndef CONFIG_RTK_VOIP_ETHERNET_DSP_IS_HOST
	extern int G711_SetThresholdVADCNG( int32 sid, int VADthres, int CNGthres, int32 SIDmode, int32 SIDlevel, int32 SIDgain );
	uint32 s_id;
	TstVoipThresVadCngConfig stVoipThresVadCngConfig;

	COPY_FROM_USER(&stVoipThresVadCngConfig, (TstVoipThresVadCngConfig *)user, sizeof(TstVoipThresVadCngConfig));

	s_id = API_GetSid(stVoipThresVadCngConfig.ch_id, stVoipThresVadCngConfig.m_id);

	G711_SetThresholdVADCNG( s_id, stVoipThresVadCngConfig.nThresVAD, stVoipThresVadCngConfig.nThresCNG,
				  stVoipThresVadCngConfig.nSIDMode, stVoipThresVadCngConfig.nSIDLevel, stVoipThresVadCngConfig.nSIDGain );
	
	return COPY_TO_USER(user, &stVoipThresVadCngConfig, sizeof(TstVoipThresVadCngConfig), cmd, seq_no);
#else
	//Host auto forward
	return 0;
#endif
}
#else
int do_mgr_VOIP_MGR_SET_VAD_CNG_THRESHOLD(struct sock *sk, int cmd, void *user, unsigned int len)
{
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_FAXMODEM
 * @brief Set FAX off-hook flag, when phone was off-hook
 * @param TstVoipCfg.ch_id Channel ID
 * @param TstVoipCfg.enable FAX hook status (0: on-hook, 1: off-hook)
 * @see VOIP_MGR_FAX_OFFHOOK TstVoipCfg
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_FAX_OFFHOOK( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCfg stVoipCfg;
	int ret;

	//when phone offhook, set fax_offhook[]=1;
	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	fax_offhook[stVoipCfg.ch_id] = stVoipCfg.enable;
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_FAX_OFFHOOK( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	//PRINT_MSG("This IO Ctrl is NOT support at AudioCodes solution.\n");
	return NO_COPY_TO_USER( cmd, seq_no );
	//return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_FAXMODEM
 * @brief Determine whether FAX is complete
 * @param TstVoipCfg.ch_id Channel ID
 * @param TstVoipCfg.enable FAX end flag (1: end, 0: not end)
 * @see VOIP_MGR_FAX_END_DETECT TstVoipCfg
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_FAX_END_DETECT( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCfg stVoipCfg;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	stVoipCfg.enable = fax_end_flag[stVoipCfg.ch_id];
#endif
	return COPY_TO_USER(user, &stVoipCfg, sizeof(TstVoipCfg), cmd, seq_no);
	//return 0;//t.38 fax end detect, 1:fax end.
}
#else
int do_mgr_VOIP_MGR_FAX_END_DETECT( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	// no handler
	TstVoipCfg stVoipCfg;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	return COPY_TO_USER(user, &stVoipCfg, sizeof(TstVoipCfg), cmd, seq_no);
	//return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_FAXMODEM
 * @brief Set FAX or modem tone detection
 * @param TstVoipCfg.ch_id Channel ID
 * @param TstVoipCfg.cfg Enable (1) or disable (0) tone detection
 * @see VOIP_MGR_SET_FAX_MODEM_DET TstVoipCfg
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_FAX_MODEM_DET( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCfg stVoipCfg;
	int ret;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward, but also do below
#endif

	/* fax modem det mode, 0:auto-hi-speed-fax. 1:fax. 2:modem, 3: auto-low-speed-fax */
	fax_modem_det_mode[stVoipCfg.ch_id] = stVoipCfg.cfg;

	PRINT_MSG("VOIP_MGR_SET_FAX_MODEM_DET = %d\n", stVoipCfg.cfg);
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_FAX_MODEM_DET( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	// no handler
	return NO_COPY_TO_USER( cmd, seq_no );
	//return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_FAXMODEM
 * @brief Set answer tone detect
 * @param TstVoipCfg.ch_id Channel ID
 * @param TstVoipCfg.cfg CNG, ANS, ANSAM, ANSBAR, ANSAMBAR, BELLANS, V22, V8bisCre, V21flag tone detection
 * @see VOIP_MGR_SET_ANSWERTONE_DET TstVoipCfg
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_ANSWERTONE_DET( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCfg stVoipCfg;
	int ret;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP_IS_HOST
	// Host auto forward
#else
	extern int set_answer_tone_det(unsigned int chid, unsigned int config);
	set_answer_tone_det(stVoipCfg.ch_id, stVoipCfg.cfg);
	extern int set_answer_tone_threshold(unsigned int chid, int threshold);
	set_answer_tone_threshold(stVoipCfg.ch_id, stVoipCfg.cfg2);
#endif
	PRINT_MSG("VOIP_MGR_SET_ANSWERTONE_DET = %x\n", stVoipCfg.cfg);
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_ANSWERTONE_DET( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	// no handler
	return NO_COPY_TO_USER( cmd, seq_no );
	//return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_FAXMODEM
 * @brief Set FAX silence detect 
 * @param TstVoipCfg.ch_id Channel ID 
 * @param TstVoipCfg.m_id Media ID 
 * @param TstVoipCfg.cfg Energy 0~127 indicate -0~-127. 
 *                       This is a negative value, and -60 is suggested. 
 * @param TstVoipCfg.cfg2 Silence period in unit of ms. 
 * @see VOIP_MGR_SET_FAX_SILENCE_DET TstVoipCfg 
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_FAX_SILENCE_DET( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCfg stVoipCfg;
	int ret;
#ifndef CONFIG_RTK_VOIP_ETHERNET_DSP_IS_HOST
	uint32 s_id;
#endif
	
	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP_IS_HOST
	// Host auto forward 
#else
	s_id = API_GetSid(stVoipCfg.ch_id, stVoipCfg.m_id);
	
	set_silence_det_threshold( s_id, stVoipCfg.cfg, stVoipCfg.cfg2 );
#endif
	PRINT_MSG("VOIP_MGR_SET_FAX_SILENCE_DET = %u, %u\n", stVoipCfg.cfg, stVoipCfg.cfg2);
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_ANSWERTONE_DET( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	// no handler 
	return NO_COPY_TO_USER( cmd, seq_no );
	//return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_FAXMODEM
 * @brief Report FAX DIS detection status
 * @param TstVoipCfg.ch_id Channel ID
 * @param TstVoipCfg.enable DIS detection status (1: detected, 0: not detected)
 * @see VOIP_MGR_FAX_DIS_DETECT TstVoipCfg
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_FAX_DIS_DETECT( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	extern int fax_v21_dis_get(unsigned int chid);
#endif

	TstVoipCfg stVoipCfg;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	stVoipCfg.enable = fax_v21_dis_get(stVoipCfg.ch_id);
#endif
	return COPY_TO_USER(user, &stVoipCfg, sizeof(TstVoipCfg), cmd, seq_no);
	//return 0;//t.38 v21 dis detect, 1:dis(Digital Identification Signal) detected.
}
#else
int do_mgr_VOIP_MGR_FAX_DIS_DETECT( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	// no handler
	TstVoipCfg stVoipCfg;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
	return COPY_TO_USER(user, &stVoipCfg, sizeof(TstVoipCfg), cmd, seq_no);
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_FAXMODEM
 * @brief Report FAX DIS TX detection status
 * @param TstVoipCfg.ch_id Channel ID
 * @param TstVoipCfg.enable DIS TX detection status{network -> ATA -> FAX} (1: detected, 0: not detected)
 * @see VOIP_MGR_FAX_DIS_DETECT TstVoipCfg
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_FAX_DIS_TX_DETECT( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	extern unsigned int Fax_DIS_TX_Event_from_pkt[];
#else
	extern int fax_v21_dis_tx_get(unsigned int chid);
	//int ret;
#endif
	TstVoipCfg stVoipCfg;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST

#if 1
	/* 1: fax, 2: local-modem, 3: remote-modem */
	stVoipCfg.enable = Fax_DIS_TX_Event_from_pkt[stVoipCfg.ch_id];
	Fax_DIS_TX_Event_from_pkt[stVoipCfg.ch_id] = 0;	// clean flag
	//stVoipCfg.ret_val = 0;
#else
	// Send Control Packet and wait Response Packet
	mgr_chid = stVoipCfg.ch_id;
	stVoipCfg.ch_id = API_get_DSP_CH(cmd, stVoipCfg.ch_id);	// convert to DSP chid
	ret = ipcSentControlPacket(cmd, mgr_chid, &stVoipCfg, sizeof(TstVoipCfg));

	if( ret < 0 )
		return ret;

	// Ckeck Response Packet (need for copy_to_user)
	unsigned short dsp_id;
	ipcCheckRespPacket(cmd, &stVoipCfg, &dsp_id);
	stVoipCfg.ch_id = API_get_Host_CH( dsp_id, stVoipCfg.ch_id);/* Get Host chid */

#endif
#else
	stVoipCfg.enable = fax_v21_dis_tx_get(stVoipCfg.ch_id);
	//stVoipCfg.ret_val = 0;
#endif
	return COPY_TO_USER(user, &stVoipCfg, sizeof(TstVoipCfg), cmd, seq_no);
	//return 0;//t.38 v21 dis tx direction detect, 1:dis(Digital Identification Signal) detected.
}
#else
int do_mgr_VOIP_MGR_FAX_DIS_TX_DETECT( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	// no handler
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_FAXMODEM
 * @brief Report FAX DIS RX detection status
 * @param TstVoipCfg.ch_id Channel ID
 * @param TstVoipCfg.enable DIS RX detection status{FAX -> ATA -> network} (1: detected, 0: not detected)
 * @see VOIP_MGR_FAX_DIS_DETECT TstVoipCfg
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_FAX_DIS_RX_DETECT( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	extern unsigned int Fax_DIS_RX_Event_from_pkt[];
#else
	extern int fax_v21_dis_rx_get(unsigned int chid);
	//int ret;
#endif

	TstVoipCfg stVoipCfg;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));


#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST

#if 1
	/* 1: fax, 2: local-modem, 3: remote-modem */
	stVoipCfg.enable = Fax_DIS_RX_Event_from_pkt[stVoipCfg.ch_id];
	Fax_DIS_RX_Event_from_pkt[stVoipCfg.ch_id] = 0;	// clean flag
#else
	// Send Control Packet and wait Response Packet
	mgr_chid = stVoipCfg.ch_id;
	stVoipCfg.ch_id = API_get_DSP_CH(cmd, stVoipCfg.ch_id);	// convert to DSP chid
	ret = ipcSentControlPacket(cmd, mgr_chid, &stVoipCfg, sizeof(TstVoipCfg));

	if( ret < 0 )
		return ret;

	// Ckeck Response Packet (need for copy_to_user)
	unsigned short dsp_id;
	ipcCheckRespPacket(cmd, &stVoipCfg, &dsp_id);
	stVoipCfg.ch_id = API_get_Host_CH( dsp_id, stVoipCfg.ch_id);/* Get Host chid */
#endif
#else
	stVoipCfg.enable = fax_v21_dis_rx_get(stVoipCfg.ch_id);
#endif
	return COPY_TO_USER(user, &stVoipCfg, sizeof(TstVoipCfg), cmd, seq_no);
	//return 0;//t.38 v21 dis rx direction detect, 1:dis(Digital Identification Signal) detected.
}
#else
int do_mgr_VOIP_MGR_FAX_DIS_RX_DETECT( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	// no handler
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_FAXMODEM
 * @brief Report FAX DCN detection status
 * @param TstVoipCfg.ch_id Channel ID
 * @param TstVoipCfg.enable DCN detection status (1: detected, 0: not detected)
 * @see VOIP_MGR_FAX_DCN_DETECT TstVoipCfg
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_FAX_DCN_DETECT( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	extern int fax_v21_dcn_get(unsigned int chid);
#endif
	TstVoipCfg stVoipCfg;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	stVoipCfg.enable = fax_v21_dcn_get(stVoipCfg.ch_id);
#endif
	return COPY_TO_USER(user, &stVoipCfg, sizeof(TstVoipCfg), cmd, seq_no);
	//return 0;//t.38 v21 dcn detect, 1:dis(Disconnect) detected.
}
#else
int do_mgr_VOIP_MGR_FAX_DCN_DETECT( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	// no handler
	TstVoipCfg stVoipCfg;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
	return COPY_TO_USER(user, &stVoipCfg, sizeof(TstVoipCfg), cmd, seq_no);
	//return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_FAXMODEM
 * @brief Report FAX DCN TX detection status
 * @param TstVoipCfg.ch_id Channel ID
 * @param TstVoipCfg.enable DCN TX detection status{network -> ATA -> FAX} (1: detected, 0: not detected)
 * @see VOIP_MGR_FAX_DCN_DETECT TstVoipCfg
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_FAX_DCN_TX_DETECT( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	extern int fax_v21_dcn_tx_get(unsigned int chid);
	TstVoipCfg stVoipCfg;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));


#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST

#if 0
	???
	extern unsigned int Fax_DIS_TX_Event_from_pkt[];
        /* 1: fax, 2: local-modem, 3: remote-modem */
        stVoipCfg.enable = Fax_DIS_TX_Event_from_pkt[stVoipCfg.ch_id];
        Fax_DIS_TX_Event_from_pkt[stVoipCfg.ch_id] = 0;	// clean flag
#else
	// Host auto forward
#endif
#else
	stVoipCfg.enable = fax_v21_dcn_tx_get(stVoipCfg.ch_id);
#endif
	return COPY_TO_USER(user, &stVoipCfg, sizeof(TstVoipCfg), cmd, seq_no);
	//return 0;//t.38 v21 dcn tx direction detect, 1:dcn(Disconnect) detected.
}
#else
int do_mgr_VOIP_MGR_FAX_DCN_TX_DETECT( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	// no handler
	TstVoipCfg stVoipCfg;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
	return COPY_TO_USER(user, &stVoipCfg, sizeof(TstVoipCfg), cmd, seq_no);
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_FAXMODEM
 * @brief Report FAX DCN RX detection status
 * @param TstVoipCfg.ch_id Channel ID
 * @param TstVoipCfg.enable DCN RX detection status{FAX -> ATA -> network} (1: detected, 0: not detected)
 * @see VOIP_MGR_FAX_DCN_DETECT TstVoipCfg
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_FAX_DCN_RX_DETECT( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	extern int fax_v21_dcn_rx_get(unsigned int chid);
	TstVoipCfg stVoipCfg;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));


#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST

#if 0
	??
	extern unsigned int Fax_DIS_RX_Event_from_pkt[];
        /* 1: fax, 2: local-modem, 3: remote-modem */
        stVoipCfg.enable = Fax_DIS_RX_Event_from_pkt[stVoipCfg.ch_id];
        Fax_DIS_RX_Event_from_pkt[stVoipCfg.ch_id] = 0;	// clean flag
#else
	// Host auto forward
#endif
#else
	stVoipCfg.enable = fax_v21_dcn_rx_get(stVoipCfg.ch_id);
#endif
	return COPY_TO_USER(user, &stVoipCfg, sizeof(TstVoipCfg), cmd, seq_no);
	//return 0;//t.38 v21 dcn rx direction detect, 1:dcn(Disconnect) detected.
}
#else
int do_mgr_VOIP_MGR_FAX_DCN_RX_DETECT( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	// no handler
	TstVoipCfg stVoipCfg;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
	return COPY_TO_USER(user, &stVoipCfg, sizeof(TstVoipCfg), cmd, seq_no);
	//return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_LEC
 * @brief Set tail length of LEC 
 * @param TstVoipValue.ch_id Channel ID 
 * @param TstVoipValue.value Tail length (ms) 
 * @param TstVoipValue.value1 Non-linear Processor Mode 0: NLP off, 1: NLP_mute, 2: NLP_shift, 3: NLP_cng 
 * @see VOIP_MGR_SET_ECHO_TAIL_LENGTH TstVoipValue 
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_ECHO_TAIL_LENGTH( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	extern void AEC_g168_set_TailLength(unsigned int chid, unsigned int tail_length);
	extern void AEC_g168_init(unsigned int chid, unsigned char type);
#endif

	TstVoipValue stVoipValue;
	unsigned char nlp = 0, nlp_mode = 0;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	if (stVoipValue.value1 != 0) //NLP on
	{
		nlp = LEC_NLP;

		switch (stVoipValue.value1)
		{
			case 1:
				nlp_mode = LEC_NLP_MUTE;
				break;
			case 2:
				nlp_mode = LEC_NLP_SHIFT;
				break;
			case 3:
				nlp_mode = LEC_NLP_CNG;
				break;
			default:
				nlp_mode = LEC_NLP_SHIFT;
				break;
		}
	}
	else	// NLP off
	{
		nlp = 0;
		nlp_mode = LEC_NLP_SHIFT;
	}
	//stVoipValue.value : unit is ms
//#ifdef EXPER_AEC
	if (stVoipValue.value<32) {
		PRINT_MSG("NEW ec128 using 32ms length\n");
		AEC_g168_set_TailLength(stVoipValue.ch_id, 32);
	#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
		AEC_g168_set_TailLength(3, 32);// for handset.
	#endif
	} else {
		AEC_g168_set_TailLength(stVoipValue.ch_id, stVoipValue.value);
	#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
		AEC_g168_set_TailLength(3, stVoipValue.value);// for handset.
	#endif
	}
	AEC_g168_init(stVoipValue.ch_id, LEC|LEC_NLP);
	#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
	AEC_g168_init(3, LEC|LEC_NLP);// for handset.
	#endif
//#endif
#ifdef SUPPORT_LEC_G168_ISR
	#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
	LEC_g168_set_TailLength(stVoipValue.ch_id, 16);
	//printk("ippohne_lec_tail\n");
		#ifdef T_TYPE_ECHO_CAN
	LEC_g168_set_TailLength(3, 16);// for handset.
	LEC_g168_init(3, LEC|nlp|nlp_mode);// for handset
		#endif
	#else
	LEC_g168_set_TailLength(stVoipValue.ch_id, stVoipValue.value);
	#endif

	#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
		#ifdef T_TYPE_ECHO_CAN
	LEC_g168_init(stVoipValue.ch_id, LEC|nlp|nlp_mode);
		#else
	LEC_g168_init(stVoipValue.ch_id, LEC|nlp|LEC_NLP_CNG);
		#endif
	#else
	LEC_g168_init(stVoipValue.ch_id, LEC|nlp|nlp_mode);
	#endif
	extern unsigned char lec_g168_nlp_bk[];
	lec_g168_nlp_bk[stVoipValue.ch_id] = LEC|nlp|nlp_mode;
#endif
#endif
	PRINT_MSG("Set CH%d Echo Tail Length = %dms, NLP=%d\n", stVoipValue.ch_id, stVoipValue.value, stVoipValue.value1);
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_ECHO_TAIL_LENGTH( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	Tac49xEchoCancelerLength ec_length;
#endif

	/** Echo Canceller Length Config **/
	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	if (stVoipValue.value == 4) //stVoipValue.value : unit is ms
		ec_length = ECHO_CANCELER_LENGTH__4_MSEC;
	else if (stVoipValue.value == 8)
		ec_length = ECHO_CANCELER_LENGTH__8_MSEC;
	else if (stVoipValue.value == 16)
		ec_length = ECHO_CANCELER_LENGTH__16_MSEC;
	else if (stVoipValue.value == 24)
		ec_length = ECHO_CANCELER_LENGTH__24_MSEC;
	else if (stVoipValue.value == 32)
		ec_length = ECHO_CANCELER_LENGTH__32_MSEC;
	else
	{
		ec_length = ECHO_CANCELER_LENGTH__4_MSEC;
		PRINT_G("Warning: LEC tail length %d isn't supported. Use default length = %d msec\n", stVoipValue.value, 4*(ec_length+1));
	}
	RtkAc49xApiUpdateEchoCancellerLength(stVoipValue.ch_id, ec_length);
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_LEC
 * @brief Turn on or off LEC 
 * @param TstVoipCfg.ch_id Channel ID 
 * @param TstVoipCfg.enable Turn on (1) or off (0) LEC 
 * @param TstVoipCfg.cfg Turn on (1) or off (0) VBD Auto LEC Change, (255) is no action
 * @param TstVoipCfg.cfg2 LEC restore value for VBD Auto LEC mode is enable
 * @see VOIP_MGR_SET_G168_LEC_CFG TstVoipCfg 
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_G168_LEC_CFG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCfg stVoipCfg;
	int ret;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	PRINT_MSG("VOIP_MGR_SET_G168_LEC_CFG: ch=%d, enable=%d\n", stVoipCfg.ch_id, stVoipCfg.enable);

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	/* 0: LEC disable  1: LEC enable */
	if (stVoipCfg.enable == 1)
#ifdef SUPPORT_LEC_G168_ISR
	{
		LEC_g168_enable(stVoipCfg.ch_id);
#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
		#ifdef T_TYPE_ECHO_CAN
		LEC_g168_enable(3);// for handset.
		#endif
#endif
	}
	else if (stVoipCfg.enable == 0)
	{
		LEC_g168_disable(stVoipCfg.ch_id);
#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
		#ifdef T_TYPE_ECHO_CAN
		LEC_g168_disable(3);// for handset.
		#endif
#endif
	}
	
#endif
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_G168_LEC_CFG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCfg stVoipCfg;
	int ret;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	/* 0: LEC disable  1: LEC enable */
	if (stVoipCfg.enable == 1)
	{
		RtkAc49xApi_LEC_enable(stVoipCfg.ch_id);
	}
	else if (stVoipCfg.enable == 0)
	{
		RtkAc49xApi_LEC_disable(stVoipCfg.ch_id);
	}
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_LEC
 * @brief Set the Echo Canceller Mode for VBD 
 * @param TstVoipCfg.ch_id The channel number
 * @param TstVoipCfg.cfg 0: EC auto change is off, 1: EC auto off, 2: EC auto restore with NLP_mute mode
 * @param TstVoipCfg.cfg2 0: EC auto change is off, 1: EC auto off, 2: EC auto restore with NLP_mute mode
 * @param TstVoipCfg.cfg3 EC restore value for low/high VBD EC auto restore mode is enable
 * @see VOIP_MGR_SET_VBD_EC TstVoipCfg 
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_VBD_EC( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCfg stVoipCfg;
	int ret;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
	
	PRINT_MSG("VOIP_MGR_SET_G168_LEC_CFG: ch=%d, enable=%d\n", stVoipCfg.ch_id, stVoipCfg.enable);

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;
#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP_IS_HOST
	// Host auto forward 
#else
	extern int LEC_g168_vbd_auto(char chid, int vbd_high, int vbd_low, int lec_bk);
	LEC_g168_vbd_auto(stVoipCfg.ch_id, stVoipCfg.cfg, stVoipCfg.cfg2, stVoipCfg.cfg3);
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_VBD_EC( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCfg stVoipCfg;
	int ret;

	// no handler 
	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
	return COPY_TO_USER(user, &stVoipCfg, sizeof(TstVoipCfg), cmd, seq_no);

}
#endif

//#define AEC_TEST 1
#define FEATURE_COP3_PROFILE 1
#include "cp3_profile.h"

#if ! defined (AUDIOCODES_VOIP)
void AEC_g168(unsigned int chid, const int16_t *pRin, int16_t *pSin, int16_t *pEx);
void aec_process_block_10ms(unsigned int chid, int16_t *output, int16_t *est_echo_out, const int16_t *input, const int16_t *echo);
void AEC_g168_set_TailLength(unsigned int chid, unsigned int tail_length);
void AEC_g168_init(unsigned int chid, unsigned char type);
void AEC_re_init(unsigned int chid);
void ec128_clear_h_register(unsigned int chid);
void ec128_set_adaption_mode(unsigned int chid, unsigned int adaption_mode);

#define INIT_MODE_AEC   2
#define INIT_MODE_CLEAN_H_REGISTER      0x8
#define INIT_MODE_ADAPT_CONFIG          0x4

void LEC_g168(char chid, const Word16 *pRin, Word16 *pSin, Word16 *pEx);
void LEC_g168_set_TailLength(unsigned int chid, unsigned int tail_length);
void LEC_g168_init(unsigned char chid, unsigned char type);

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
static int ec_test_mode;
#endif

int do_mgr_VOIP_MGR_GET_EC_DEBUG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// standalone cmd
#else
	TstVoipEcDebug	stVoipEcDebug __attribute__((aligned (8)));
	unsigned long flags;
	unsigned short input[80] __attribute__((aligned (8)));
	//unsigned short est_echo[80] __attribute__((aligned (8)));

	COPY_FROM_USER(&stVoipEcDebug, (TstVoipEcDebug*)user, sizeof(TstVoipEcDebug));

	memcpy(input, stVoipEcDebug.buf1, 160);

	if (stVoipEcDebug.mode&0x1) {//stVoipLecDebug.mode = 1, do ec
		save_flags(flags); cli();
		if (ec_test_mode==2) {
#ifdef AEC_TEST
			ProfileEnterPoint(PROFILE_INDEX_LEC);
			AEC_g168( 3, input, stVoipEcDebug.buf2, stVoipEcDebug.buf1)
			ProfileExitPoint(PROFILE_INDEX_LEC);
			ProfilePerDump(PROFILE_INDEX_LEC, 1024);;
#endif
			//aec_process_block_10ms(3, stVoipEcDebug.buf1, est_echo, input, stVoipEcDebug.buf2);
		} else {
			LEC_g168( 3, input, stVoipEcDebug.buf2, stVoipEcDebug.buf1);
		//
		}
		restore_flags(flags);
		return COPY_TO_USER(user,&stVoipEcDebug, sizeof(TstVoipEcDebug), cmd, seq_no);
	} else if (stVoipEcDebug.mode&INIT_MODE_AEC) {//stVoipEcDebug.mode = INIT_MODE_AEC, do aec init
    #ifdef AEC_TEST
		if (stVoipEcDebug.mode&INIT_MODE_CLEAN_H_REGISTER)
			ec128_clear_h_register(3);
		else if (stVoipEcDebug.mode&INIT_MODE_ADAPT_CONFIG) {
			ec128_set_adaption_mode(3, (stVoipEcDebug.mode>>4)&7);
		} else {
			AEC_g168_set_TailLength(3, 128);
			//AEC_g168_set_TailLength(3, 64);
			AEC_g168_init(3, 1);
			AEC_re_init(3);
		}
    #endif
		ec_test_mode = 2;
	} else {//stVoipEcDebug.mode = 0, do ec init
		LEC_g168_set_TailLength(3, 16);
		LEC_g168_init( 3, LEC|LEC_NLP|LEC_NLP_MUTE);
		ec_test_mode = 0;
	}
#endif
	return 0;
}
#else

int do_mgr_VOIP_MGR_GET_EC_DEBUG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	// no handler for acmw
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_DTMF
 * @brief Set DTMF detection parameters
 * @param TstDtmfDetPara.ch_id Channel ID
 * @param TstDtmfDetPara.threshold DTMF detection threshold
 * @param TstDtmfDetPara.dir DTMF detection direction, 0: TDM, 1: IP
 * @param TstDtmfDetPara.on_time DTMF detection minimum on time, on_time_10ms (unit 10ms)
 * @param TstDtmfDetPara.fortwist DTMF detection acceptable fore-twist (dB)
 * @param TstDtmfDetPara.revtwist DTMF detection acceptable rev-twist (dB)
 * @see VOIP_MGR_DTMF_DET_PARAM TstDtmfDetPara
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_DTMF_DET_PARAM( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstDtmfDetPara stDtmfDetPara;
	int ret;

	COPY_FROM_USER(&stDtmfDetPara, (TstDtmfDetPara *)user, sizeof(TstDtmfDetPara));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

	PRINT_MSG("do_mgr_VOIP_MGR_DTMF_DET_PARAM:ch_id = %d, dir=%d, threshold = -%d dBm, dtmf_on_time=%d0ms, fore_twist=%dB, rev_twist=%dB\n", 
		stDtmfDetPara.ch_id, stDtmfDetPara.dir, stDtmfDetPara.thres, stDtmfDetPara.on_time, stDtmfDetPara.fore_twist, stDtmfDetPara.rev_twist);

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	dtmf_det_threshold_set(stDtmfDetPara.ch_id, stDtmfDetPara.thres, stDtmfDetPara.dir);
	dtmf_det_on_time_set(stDtmfDetPara.ch_id, stDtmfDetPara.dir, stDtmfDetPara.on_time);
	dtmf_det_twist_dB_set(stDtmfDetPara.ch_id, stDtmfDetPara.fore_twist, stDtmfDetPara.rev_twist, stDtmfDetPara.dir);
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_DTMF_DET_PARAM( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstDtmfDetPara stDtmfDetPara;
	int ret;

	COPY_FROM_USER(&stDtmfDetPara, (TstDtmfDetPara *)user, sizeof(TstDtmfDetPara));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

	PRINT_R("VOIP_MGR_DTMF_DET_THRESHOLD: not support(or implement) for ACMW\n");

	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_DTMF
 * @brief Enable or disable DTMF detection 
 * @param TstDtmfDetPara.ch_id Channel ID 
 * @param TstDtmfDetPara.enable Enable (1) or disable (0) DTMF detection 
 * @see VOIP_MGR_DTMF_CFG TstDtmfDetPara 
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_DTMF_CFG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstDtmfDetPara stDtmfDetPara;
	int ret;

	COPY_FROM_USER(&stDtmfDetPara, (TstDtmfDetPara *)user, sizeof(TstDtmfDetPara));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

	PRINT_MSG("VOIP_MGR_DTMF_CFG:ch_id = %d, enable = %d\n", stDtmfDetPara.ch_id, stDtmfDetPara.enable);

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else

	if(stDtmfDetPara.enable == 1 || stDtmfDetPara.enable == 2)
		dtmf_start((char)stDtmfDetPara.ch_id, stDtmfDetPara.dir);
	else if ( stDtmfDetPara.enable == 0)
		dtmf_stop((char) stDtmfDetPara.ch_id, stDtmfDetPara.dir);
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_DTMF_CFG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstDtmfDetPara stDtmfDetPara;
	int ret;

	COPY_FROM_USER(&stDtmfDetPara, (TstDtmfDetPara *)user, sizeof(TstDtmfDetPara));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

	/** DTMF Event **/
		//PRINT_MSG("VOIP_MGR_DTMF_CFG:ch_id = %d, enable = %d\n", stDtmfDetPara.ch_id, stDtmfDetPara.enable);
		//RtkAc49xApiSetDtmfDetection(stDtmfDetPara.ch_id, stDtmfDetPara.enable);
		// NOTE:
		// Can NOT call RtkAc49xApiSetDtmfDetection() here, or it will cause acmw to close regular rtp fail.
		// Developer should make sure that enable DTMF det after acmw channel open, and disable DTMF det before acmw channel close.
		// Or it will not work and may result some bad effect.
		// Now, enable / disable DTMF det is integrated in IO Ctrl VOIP_MGR_SLIC_ONHOOK_ACTION and VOIP_MGR_SLIC_OFFHOOK_ACTION.
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_DTMF
 * @brief Set DTMF mode to send out
 * @param TstVoipCfg.ch_id Channel ID
 * @param TstVoipCfg.enable DTMF mode. (0:rfc2833  1: sip info  2: inband  3: DTMF delete)
 * @see VOIP_MGR_SET_DTMF_MODE TstVoipCfg
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_DTMF_MODE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCfg stVoipCfg;
	int ret;

	// 0:rfc2833  1: sip info  2: inband 3: DTMF delete
	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else

	if (stVoipCfg.enable == 3)
	{
		dtmf_removal_flag[stVoipCfg.ch_id] = 1; /* Do DTMF removal */
		dtmf_mode[stVoipCfg.ch_id] = 3;
	}
	
	else if (stVoipCfg.enable == 2)
	{
		dtmf_removal_flag[stVoipCfg.ch_id] = 0; /* No DTMF removal */
		dtmf_mode[stVoipCfg.ch_id] = 2;
	}
	else if (stVoipCfg.enable == 1)
	{
		dtmf_removal_flag[stVoipCfg.ch_id] = 1; /* Do DTMF removal */
		dtmf_mode[stVoipCfg.ch_id] = 1;
	}
	else if (stVoipCfg.enable == 0)
	{
		dtmf_removal_flag[stVoipCfg.ch_id] = 1; /* Do DTMF removal */
		dtmf_mode[stVoipCfg.ch_id] = 0;
	}
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_DTMF_MODE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCfg stVoipCfg;
	int ret;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else

	if (stVoipCfg.enable == 3) // DTMF delete
	{
		dtmf_mode[stVoipCfg.ch_id] = 3;
		RtkAc49xApiSetIbsTransferMode(stVoipCfg.ch_id, IBS_TRANSFER_MODE__RELAY_DISABLE_VOICE_MUTE);
	}
	else if (stVoipCfg.enable == 2) // Inband
	{
		dtmf_mode[stVoipCfg.ch_id] = 2;
		RtkAc49xApiSetIbsTransferMode(stVoipCfg.ch_id, IBS_TRANSFER_MODE__TRANSPARENT_THROUGH_VOICE);
	}
	else if (stVoipCfg.enable == 1) //SIP Info
	{
		dtmf_mode[stVoipCfg.ch_id] = 1;
		RtkAc49xApiSetIbsTransferMode(stVoipCfg.ch_id, IBS_TRANSFER_MODE__RELAY_DISABLE_VOICE_MUTE);
	}
	else if (stVoipCfg.enable == 0) // RFC2833
	{
		dtmf_mode[stVoipCfg.ch_id] = 0;
		/*
		 * Here, NOT set the ACMW Ibs Transfer Mode to RFC2833.
		 * The time to set ACMW Ibs Transfer Mode to RFC2833 is in time of setting RTP session.
		 * See the IO control VOIP_MGR_SET_SESSION.
		 */
	}
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_DTMF
 * @brief Send RFC2833 DTMF packet
 * @see VOIP_MGR_SEND_RFC2833_PKT_CFG TstVoip2833
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SEND_RFC2833_PKT_CFG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	extern int send_2833_by_ap[];
#endif
	TstVoip2833 stRFC2833;
	int ret;
	uint32 s_id;

	COPY_FROM_USER(&stRFC2833, (TstVoip2833 *)user, sizeof(TstVoip2833));
	//PRINT_MSG( "VOIP_MGR_SEND_RFC2833_PKT_CFG:chid=%d,sid=%d\n", stRFC2833.ch_id, stRFC2833.sid );

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else

#ifdef SUPPORT_RFC_2833

	#ifdef SEND_RFC2833_ISR
	#ifdef CONFIG_RTK_VOIP_IP_PHONE
	send_2833_by_ap[stRFC2833.ch_id] = 1;
	#endif
	if (send_2833_by_ap[stRFC2833.ch_id] == 1)
	{
		extern unsigned char send_2833_count_down[MAX_DSP_RTK_SS_NUM];
		extern int send_dtmf_flag[];
		extern int g_digit[MAX_DSP_RTK_SS_NUM];

		s_id = API_GetSid(stRFC2833.ch_id, stRFC2833.m_id);

		send_dtmf_flag[ s_id ] = 1;
		g_digit[ stRFC2833.ch_id ] = stRFC2833.digit;
		send_2833_count_down[ s_id ] = stRFC2833.duration / ( PCM_PERIOD * 10 );
		PRINT_R("set send_2833_count_down[%d] to %d\n", s_id, send_2833_count_down[ s_id ]);
		// = 100 / ( PCM_PERIOD * 10 );	/* 100ms */
	}
	#endif
	/* Thlin: Send RFC2833 packets has been move to PCM_RX() */
#endif
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_SEND_RFC2833_PKT_CFG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoip2833 stRFC2833;
	int ret;

	COPY_FROM_USER(&stRFC2833, (TstVoip2833 *)user, sizeof(TstVoip2833));
	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;
	RtkAc49xApiSendOutbandDtmfEvent(stRFC2833.ch_id, stRFC2833.digit, stRFC2833.duration);
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_DTMF
 * @brief Decide RFC2833 sent by AP or DSP
 * @param TstVoipCfg.ch_id Channel ID
 * @param TstVoipCfg.enable Sent by DSP (0) by AP (1)
 * @see VOIP_MGR_SEND_RFC2833_BY_AP TstVoipCfg
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SEND_RFC2833_BY_AP( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	extern int send_2833_by_ap[];
#endif

	TstVoipCfg stVoipCfg;
	int ret;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

	//PRINT_MSG( "VOIP_MGR_SEND_RFC2833_BY_AP:chid=%d\n", stVoipCfg.ch_id);

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	send_2833_by_ap[stVoipCfg.ch_id] = stVoipCfg.enable; /* 0: by DSP 1: by AP */
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_SEND_RFC2833_BY_AP( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	// no handler
	return NO_COPY_TO_USER( cmd, seq_no );
	//return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_DTMF
 * @brief Set the RFC2833 TX configuration
 * @see VOIP_MGR_SET_RFC2833_TX_VOLUME TstVoip2833
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_RFC2833_TX_VOLUME( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	extern int send_2833_by_ap[];
	extern unsigned int gRfc2833_volume_dsp_auto[];
	extern unsigned int gRfc2833_volume[];

	uint32 SessNum, sid, i;
#endif

	TstVoip2833 stRFC2833;
	int ret;

	COPY_FROM_USER(&stRFC2833, (TstVoip2833 *)user, sizeof(TstVoip2833));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	SessNum = chanInfo_GetRegSessionNum(stRFC2833.ch_id);

	for(i=0; i<SessNum; i++)
	{
		sid = chanInfo_GetRegSessionID(stRFC2833.ch_id, i);
		
		if (stRFC2833.bEnable == RFC2833_VOLUME_DSP_ATUO)
		{
			if (send_2833_by_ap[stRFC2833.ch_id] == 1)
			{
				gRfc2833_volume_dsp_auto[sid] = 0;
				PRINT_R("VOIP_MGR_SET_RFC2833_TX_VOLUME: invalid setting. RFC2833 TX mode is AP mode, but volume is DSP auto. sid=%d\n", sid);
			}
			else
				gRfc2833_volume_dsp_auto[sid] = 1;
		}
		else
		{
			gRfc2833_volume_dsp_auto[sid] = 0;
			gRfc2833_volume[sid] = stRFC2833.volume;
			PRINT_MSG("gRfc2833_volume[%d] =  %d dBm\n", sid, gRfc2833_volume[sid]);
		}
	}

	//stRFC2833.ret_val = 0;
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_RFC2833_TX_VOLUME( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoip2833 stRFC2833;
	int ret;

	COPY_FROM_USER(&stRFC2833, (TstVoip2833 *)user, sizeof(TstVoip2833));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

	/* This IO ctrl is not support for acmw. */
	//stRFC2833.ret_val = -1;
	return -1;
}
#endif

/**
 * @ingroup VOIP_DSP_DTMF
 * @brief Limit the Max. RFC2833 DTMF Duration
 * @see VOIP_MGR_LIMIT_MAX_RFC2833_DTMF_DURATION TstVoip2833
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_LIMIT_MAX_RFC2833_DTMF_DURATION( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef SUPPORT_RFC2833_PLAY_LIMIT
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	uint32 SessNum, sid, i;
	extern int bRfc2833_play_limit[];
	extern int rfc2833_play_limit_ms[];
#endif
#endif

	TstVoip2833 stRFC2833;
	int ret;

	COPY_FROM_USER(&stRFC2833, (TstVoip2833 *)user, sizeof(TstVoip2833));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;
#ifdef SUPPORT_RFC2833_PLAY_LIMIT
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else

	SessNum = chanInfo_GetRegSessionNum(stRFC2833.ch_id);

	for(i=0; i<SessNum; i++)
	{
		sid = chanInfo_GetRegSessionID(stRFC2833.ch_id, i);
		bRfc2833_play_limit[sid] = stRFC2833.bEnable;
		rfc2833_play_limit_ms[sid] = stRFC2833.duration;
		//PRINT_MSG("limit: %d, ms: %d, sid=%d\n", bRfc2833_play_limit[sid], rfc2833_play_limit_ms[sid], sid);
	}
#endif
#else
	PRINT_R("VOIP_MGR_LIMIT_MAX_RFC2833_DTMF_DURATION is not supported.\n");
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_LIMIT_MAX_RFC2833_DTMF_DURATION( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoip2833 stRFC2833;

	COPY_FROM_USER(&stRFC2833, (TstVoip2833 *)user, sizeof(TstVoip2833));
	/* This IO ctrl is not support for acmw. */
	return NO_COPY_TO_USER( cmd, seq_no );
	//return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_DTMF
 * @brief FAX/Modem RFC2833 configuration
 * @param TstVoipCfg.ch_id Channel ID
 * @param TstVoipCfg.cfg 0: Doesn't send, 1: Send -- the RFC2833 packets when DSP detect TDM Fax/Modem tone
 * @param TstVoipCfg.cfg2 0: Doesn't removal, 1: Removal -- the inband RTP packet when Fax/Modem RFC2833 packets are sending
 * @param TstVoipCfg.cfg3 0: Doesn't play, 1: Play -- the Fax/Modem tone when receiving Fax/Modem RFC2833 packets
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_FAX_MODEM_RFC2833_CFG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCfg stVoipCfg;
	int ret;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

	//PRINT_MSG( "VOIP_MGR_SEND_RFC2833_BY_AP:chid=%d\n", stVoipCfg.ch_id);

#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP_IS_HOST
	// Host auto forward
#else
	extern void answer_tone_set_rfc2833_relay(unsigned int chid, unsigned int flag);
	extern int SetFaxModem_RtpRemoval(uint32 chid, uint32 flag);
	extern int SetFaxModem_RFC2833RxPlay(uint32 chid, uint32 flag);

	PRINT_MSG("VOIP_MGR_FAX_MODEM_RFC2833_CFG: %d, %d, %d, ch%d\n",
			stVoipCfg.cfg, stVoipCfg.cfg2, stVoipCfg.cfg3, stVoipCfg.ch_id);
			
	answer_tone_set_rfc2833_relay(stVoipCfg.ch_id, stVoipCfg.cfg);
	SetFaxModem_RtpRemoval(stVoipCfg.ch_id, stVoipCfg.cfg2);
	SetFaxModem_RFC2833RxPlay(stVoipCfg.ch_id, stVoipCfg.cfg3);
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_FAX_MODEM_RFC2833_CFG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	// no handler
	return NO_COPY_TO_USER( cmd, seq_no );
	//return 0;
}
#endif


/**
 * @ingroup VOIP_DSP_DTMF
 * @brief RFC2833 Packet Interval configuration
 * @param TstVoipCfg.ch_id Channel ID
 * @param TstVoipCfg.m_id Media ID
 * @param TstVoipCfg.cfg 0: DTMF, 1: Fax/Modem
 * @param TstVoipCfg.cfg2 Packet Interval(msec), must be multiple of 10 msec
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_RFC2833_PKT_INTERVAL_CFG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCfg stVoipCfg;
	uint32 s_id;
	int ret;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

	//PRINT_MSG( "VOIP_MGR_SEND_RFC2833_BY_AP:chid=%d\n", stVoipCfg.ch_id);

#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP_IS_HOST
	// Host auto forward
#else
	extern void SetDtmfRfc2833PktInterval(unsigned int sid, unsigned int interval);
	extern void SetFaxModemRfc2833PktInterval(unsigned int sid, unsigned int interval);

	s_id = API_GetSid(stVoipCfg.ch_id, stVoipCfg.m_id);
	
	PRINT_MSG("VOIP_MGR_RFC2833_PKT_INTERVAL_CFG, sid%d: type=%d, interval=%d\n",
			s_id, stVoipCfg.cfg, stVoipCfg.cfg2);
			
	if (stVoipCfg.cfg == 0)
		SetDtmfRfc2833PktInterval(s_id, stVoipCfg.cfg2);
	else if (stVoipCfg.cfg == 1)
		SetFaxModemRfc2833PktInterval(s_id, stVoipCfg.cfg2);
	else
		PRINT_R("Error type=%d in %s.\n", stVoipCfg.cfg, __FUNCTION__);
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_RFC2833_PKT_INTERVAL_CFG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	// no handler
	return NO_COPY_TO_USER( cmd, seq_no );
	//return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_DTMF
 * @brief Play SIP info DTMF locally
 * @param TstVoipValue.ch_id Channel ID
 * @param TstVoipValue.m_id Media ID
 * @param TstVoipValue.value DTMF digit
 * @param TstVoipValue.value5 DTMF duration (ms)
 * @see VOIP_MGR_PLAY_SIP_INFO TstVoipValue
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_PLAY_SIP_INFO( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	uint32 s_id;
#endif
	TstVoipValue stVoipValue;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else

#if defined (PULSE_DIAL_GEN) && defined (OUTBAND_AUTO_PULSE_DIAL_GEN)
	if ( (1 == DAA_Get_Dial_Mode(stVoipValue.ch_id)) && ( 1 == Is_DAA_Channel(stVoipValue.ch_id)) )
	{
		pulse_dial_in_cch(stVoipValue.ch_id, stVoipValue.value);

	}
	else
#endif
	{
		int index;

		s_id = API_GetSid(stVoipValue.ch_id, stVoipValue.m_id);
		if ( ((g_SIP_Info_buf_w[s_id]+1)%10) == g_SIP_Info_buf_r[s_id])
		{
			PRINT_MSG("SIP Info Buffer Overflow, sid= %d\n", s_id);
			return 0;
		}
		index = g_SIP_Info_buf_w[s_id];
		g_SIP_Info_tone_buf[s_id][index] = stVoipValue.value;
		g_SIP_Info_time_buf[s_id][index] = stVoipValue.value5/10;
		g_SIP_Info_buf_w[s_id] = (g_SIP_Info_buf_w[s_id] + 1)%10;
		//printk("(%d, %d)\n", stVoipValue.value5, g_SIP_Info_time_buf[s_id][index]);
		//printk("(%d, %d)\n", s_id, index);
	}
#endif

	return 0;
}
#else
int do_mgr_VOIP_MGR_PLAY_SIP_INFO( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else

	RtkAc49xApiGenDtmfTone(stVoipValue.ch_id, stVoipValue.value, stVoipValue.value5, 0 /*Off-duration*/, IBS_STRING_GENERATOR_REDIRECTION__INTO_DECODER_OUTPUT);
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_CALLERID
 * @brief Generate DTMF caller ID
 * @param TstVoipCID.ch_id Channel ID
 * @param TstVoipCID.string Caller ID - phonenumber
 * @see VOIP_MGR_DTMF_CID_GEN_CFG TstVoipCID
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_DTMF_CID_GEN_CFG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef SW_DTMF_CID
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	extern unsigned char ioctrl_ring_set[];
#ifdef CONFIG_RTK_VOIP_LED
	extern volatile unsigned int fxs_ringing[];
	extern volatile unsigned int daa_ringing;
#endif
#endif
#endif
	//ring_struct ring;
	TstVoipCID stCIDstr;
	int ret;

#ifdef SW_DTMF_CID
	COPY_FROM_USER(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

	PRINT_MSG("VOIP_MGR_DTMF_CID_GEN_CFG, ch=%d\n", stCIDstr.ch_id);

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else

	if ( 1 == Is_DAA_Channel(stCIDstr.ch_id))
	{
		//printk("chid%d is not for SLIC, can not gen DTMF caller ID!\n", stCIDstr.ch_id);
		return 0;
	}

	if ( (strcmp(stCIDstr.string, "P") != 0) && (strcmp(stCIDstr.string, "O") != 0) )
	{
		//ring.CH = stCIDstr.ch_id;
		//ring.ring_set = 1;

		if (dtmf_cid_info[stCIDstr.ch_id].bAuto_Ring)
		{
			if(dtmf_cid_info[stCIDstr.ch_id].bBefore1stRing == 0)// send DTMF callerid after ring.
			{
				//FXS_Ring(&ring);
				FXS_Ring( stCIDstr.ch_id, 1 );
				//ioctrl_ring_set[stCIDstr.ch_id] = ring.ring_set + (0x1<<1);
				//printk("m:ioctrl_ring_set[%d]=%d\n", stCIDstr.ch_id, ioctrl_ring_set[stCIDstr.ch_id]);
				//gChkRingDelay = jiffies + 20; // th: add delay before check 1st Ring off.
				gChkRingDelay = timetick + 200; // th: add delay before check 1st Ring off.
				printk("1st ring..\n");
	
#ifdef CONFIG_RTK_VOIP_LED
				if (daa_ringing == 0)
					fxs_ringing[stCIDstr.ch_id] = 1;
				led_state_watcher(stCIDstr.ch_id);
#endif
			}
			else {
				//gChkRingDelay = jiffies - 1;
				gChkRingDelay = timetick - 10;
			}
			
			ioctrl_ring_set[stCIDstr.ch_id] = /*ring.ring_set*/1 + (0x1<<1); //Must to keep DSP auto-Ring normal.
		}

		dtmf_cid_state[stCIDstr.ch_id]=1;
		strcpy(dtmf_cid_info[stCIDstr.ch_id].data, stCIDstr.string);
	}
	else
	{
		if (dtmf_cid_info[stCIDstr.ch_id].bAuto_Ring)
		{
			//ring.CH = stCIDstr.ch_id;
			//ring.ring_set = 1;
			//FXS_Ring(&ring);
			FXS_Ring( stCIDstr.ch_id, 1 );
			ioctrl_ring_set[stCIDstr.ch_id] = /*ring.ring_set*/1 + (0x1<<1);
		}
		
		PRINT_MSG("Not Support 'Private' DTMF Caller ID(ch=%d).\n", stCIDstr.ch_id);
	}
#endif
#else
	return NO_COPY_TO_USER( cmd, seq_no );
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_DTMF_CID_GEN_CFG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	//ring_struct ring;
	unsigned char ring_set = 0;
	TstVoipCID stCIDstr;
	extern unsigned char ioctrl_ring_set[];
#ifdef CONFIG_RTK_VOIP_LED
	extern volatile unsigned int fxs_ringing[];
	extern volatile unsigned int daa_ringing;
#endif
	int ret;

	COPY_FROM_USER(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else

	if ( 1 == Is_DAA_Channel(stCIDstr.ch_id))
	{
		//printk("chid%d is not for SLIC, can not gen DTMF caller ID!\n", stCIDstr.ch_id);
		return 0;
	}

	if ( (strcmp(stCIDstr.string, "P") != 0) && (strcmp(stCIDstr.string, "O") != 0) )
	{
		//ring.CH = stCIDstr.ch_id;
		//ring.ring_set = 1;
		ring_set = 1;

		if(dtmf_cid_info[stCIDstr.ch_id].bBefore1stRing == 0)// send DTMF callerid after ring.
		{
			//FXS_Ring(&ring);
			FXS_Ring( stCIDstr.ch_id, 1 );
			//ioctrl_ring_set[stCIDstr.ch_id] = ring.ring_set + (0x1<<1);
			//printk("m:ioctrl_ring_set[%d]=%d\n", stCIDstr.ch_id, ioctrl_ring_set[stCIDstr.ch_id]);
			mdelay(100);// th: add delay before check 1st Ring off.
			printk("1st ring..\n");

#ifdef CONFIG_RTK_VOIP_LED
			if (daa_ringing == 0)
				fxs_ringing[stCIDstr.ch_id] = 1;
			led_state_watcher(stCIDstr.ch_id);
#endif
		}

		ioctrl_ring_set[stCIDstr.ch_id] = ring_set/*ring.ring_set*/ + (0x1<<1); //Must to keep DSP auto-Ring normal.

		dtmf_cid_state[stCIDstr.ch_id]=1;
		strcpy(dtmf_cid_info[stCIDstr.ch_id].data, stCIDstr.string);
		RtkAc49xApiSendDtmfCallerId(stCIDstr.ch_id, 0, dtmf_cid_info[stCIDstr.ch_id].data);

		/* Auto Ring */
		if (gRingGenAfterCid[stCIDstr.ch_id] == 0)
		{
			//gFirstRingOffTimeOut[stCIDstr.ch_id] = jiffies + (HZ*gRingCadOff[stCIDstr.ch_id]/1000);
			gFirstRingOffTimeOut[stCIDstr.ch_id] = timetick + gRingCadOff[stCIDstr.ch_id];
			//printk("=>%d, J=%d\n", gFirstRingOffTimeOut[stCIDstr.ch_id], jiffies);
			gRingGenAfterCid[stCIDstr.ch_id] = 1;
			//printk("1: gRingGenAfterCid[%d] = %d\n", stCIDstr.ch_id, gRingGenAfterCid[stCIDstr.ch_id]);
		}
	}
	else
	{
		//ring.CH = stCIDstr.ch_id;
		//ring.ring_set = 1;
		//FXS_Ring(&ring);
		ring_set = 1;
		FXS_Ring( stCIDstr.ch_id, 1 );
		ioctrl_ring_set[stCIDstr.ch_id] = ring_set/*ring.ring_set*/ + (0x1<<1);
		PRINT_MSG("Not Support 'Private' DTMF Caller ID(ch=%d).\n", stCIDstr.ch_id);
	}
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_CALLERID
 * @brief Get caller ID state
 * @param TstVoipCID.ch_id Channel ID
 * @param [out] TstVoipCID.cid_state Caller ID state
 * @see VOIP_MGR_GET_CID_STATE_CFG TstVoipCID
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_GET_CID_STATE_CFG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCID stCIDstr;
#ifdef SW_DTMF_CID
	COPY_FROM_USER(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	stCIDstr.cid_state = dtmf_cid_state[stCIDstr.ch_id];
#endif
	return COPY_TO_USER(user, &stCIDstr, sizeof(TstVoipCID), cmd, seq_no);
	//PRINT_MSG("VOIP_MGR_GET_CID_STATE_CFG = %d\n", stCIDstr.cid_state);
#else
	COPY_FROM_USER(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));
	stCIDstr.cid_state = 0;
	return COPY_TO_USER(user, &stCIDstr, sizeof(TstVoipCID), cmd, seq_no);
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_GET_CID_STATE_CFG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCID stCIDstr;

	COPY_FROM_USER(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else

	stCIDstr.cid_state = dtmf_cid_state[stCIDstr.ch_id];
#endif
	return COPY_TO_USER(user, &stCIDstr, sizeof(TstVoipCID), cmd, seq_no);
	//return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_CALLERID
 * @brief Generate FSK caller ID
 * @param TstVoipCID.ch_id Channel ID
 * @param TstVoipCID.cid_mode Service type (0-type 1 (on-hook), 1-type 2(off-hook))
 * @param TstVoipCID.string Caller ID - Phonenumber
 * @param TstVoipCID.string2 Caller ID - Date and time
 * @param TstVoipCID.cid_name Caller ID - Name
 * @see VOIP_MGR_FSK_CID_GEN_CFG TstVoipCID
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_FSK_CID_GEN_CFG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCID stCIDstr;
	int ret;

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	TstFskClidData cid_data[FSK_MDMF_SIZE];
#endif

	COPY_FROM_USER(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	if ( 1 == Is_DAA_Channel(stCIDstr.ch_id))
	{
		//printk("chid%d is not for SLIC, can not gen FSK caller ID!\n", stCIDstr.ch_id);
		return 0;
	}

	if (stCIDstr.string[0] == 'P')
	{
		cid_data[0].type = FSK_PARAM_CLI_ABS;
		strcpy(cid_data[0].data, "P");	// Private
	}
	else if (stCIDstr.string[0] == 'O')
	{
		cid_data[0].type = FSK_PARAM_CLI_ABS;
		strcpy(cid_data[0].data, "O");	// Out of area or Unavailable
	}
	else if (stCIDstr.string[0] != 0)
	{
		cid_data[0].type = FSK_PARAM_CLI;
		strcpy(cid_data[0].data, stCIDstr.string);
	}
	else
	{
		cid_data[0].type = FSK_PARAM_NULL;
		cid_data[0].data[0] = 0;
	}

	if (stCIDstr.string2[0] != 0)
	{
		cid_data[1].type = FSK_PARAM_DATEnTIME;
		strcpy(cid_data[1].data, stCIDstr.string2);
	}
	else
	{
		cid_data[1].type = FSK_PARAM_NULL;
		cid_data[1].data[0] = 0;
	}

	if (stCIDstr.cid_name[0] != 0)
	{
		cid_data[2].type = FSK_PARAM_CLI_NAME;
		strcpy(cid_data[2].data, stCIDstr.cid_name);
	}
	else
	{
		cid_data[2].type = FSK_PARAM_NULL;
		cid_data[2].data[0] = 0;
	}

	cid_data[3].type = FSK_PARAM_NULL;
	cid_data[3].data[0] = 0;
	cid_data[4].type = FSK_PARAM_NULL;
	cid_data[4].data[0] = 0;

	SLIC_gen_FSK_CID(stCIDstr.ch_id, stCIDstr.cid_mode, FSK_MSG_CALLSETUP , cid_data);

#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_FSK_CID_GEN_CFG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	//ring_struct ring;
	unsigned char ring_set = 0;
	TstVoipCID stCIDstr;
	//unsigned long flags;
	extern unsigned char ioctrl_ring_set[];
#ifdef CONFIG_RTK_VOIP_LED
	extern volatile unsigned int fxs_ringing[];
	extern volatile unsigned int daa_ringing;
#endif
	//extern char fsk_cid_enable[MAX_DSP_AC_CH_NUM];
	int ret;

	COPY_FROM_USER(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	if ( 1 == Is_DAA_Channel(stCIDstr.ch_id))
	{
		//printk("chid%d is not for SLIC, can not gen FSK caller ID!\n", stCIDstr.ch_id);
		return 0;
	}


	//ring.CH = stCIDstr.ch_id;
	//ring.ring_set = 1;
	ring_set = 1;

	if(!(fsk_spec_areas[stCIDstr.ch_id]&0x08))// send DTMF callerid after ring.
	{
		//FXS_Ring(&ring);
		FXS_Ring( stCIDstr.ch_id, 1 );
		//ioctrl_ring_set[stCIDstr.ch_id] = ring.ring_set + (0x1<<1);
		//printk("m:ioctrl_ring_set[%d]=%d\n", stCIDstr.ch_id, ioctrl_ring_set[stCIDstr.ch_id]);
		mdelay(20);// th: add delay before check 1st Ring off.
		printk("1st ring..\n");

#ifdef CONFIG_RTK_VOIP_LED
		if (daa_ringing == 0)
			fxs_ringing[stCIDstr.ch_id] = 1;
		led_state_watcher(stCIDstr.ch_id);
#endif
	}
#if 0
	save_flags(flags); cli();
	ioctrl_ring_set[stCIDstr.ch_id] = ring.ring_set + (0x1<<1); //Must to keep DSP auto-Ring normal.
	fsk_cid_state[stCIDstr.ch_id] = 1;
	fsk_cid_enable[stCIDstr.ch_id] = 1;

	RtkAc49xApiSetFskCallerIdParam(stCIDstr.ch_id, stCIDstr.cid_mode, FSK_MSG_CALLSETUP, stCIDstr.string,  stCIDstr.string2, stCIDstr.cid_name);
	restore_flags(flags);
#else
	RtkAc49xApiSendFskCallerId(stCIDstr.ch_id, stCIDstr.cid_mode, FSK_MSG_CALLSETUP, stCIDstr.string,  stCIDstr.string2, stCIDstr.cid_name);
	//PRINT_MSG("\n <RTK> CID = %s\n", stCIDstr.string);

	ioctrl_ring_set[stCIDstr.ch_id] = ring_set/*ring.ring_set*/ + (0x1<<1); //Must to keep DSP auto-Ring normal.
	fsk_cid_state[stCIDstr.ch_id] = 1;

	/* Auto Ring */
	if (gRingGenAfterCid[stCIDstr.ch_id] == 0)
	{
		//gFirstRingOffTimeOut[stCIDstr.ch_id] = jiffies + (HZ*gRingCadOff[stCIDstr.ch_id]/1000);
		gFirstRingOffTimeOut[stCIDstr.ch_id] = timetick + gRingCadOff[stCIDstr.ch_id];
		//printk("=>%d, J=%d\n", gFirstRingOffTimeOut[stCIDstr.ch_id], jiffies);
		gRingGenAfterCid[stCIDstr.ch_id] = 1;
		//printk("1: gRingGenAfterCid[%d] = %d\n", stCIDstr.ch_id, gRingGenAfterCid[stCIDstr.ch_id]);
	}
#endif
#endif
	return 0;
}
#endif


/**
 * @ingroup VOIP_DSP_CALLERID
 * @brief FSK Caller ID Generation
 * @param chid The FXS channel number.
 * @param pClid The pointer of FSK Caller ID Data
 * @param num_clid_element The number of FSK Caller ID element
 * @see VOIP_MGR_FSK_CID_MDMF_GEN TstFskClid
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_FSK_CID_MDMF_GEN( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstFskClid stFskClid;
	int ret;

	COPY_FROM_USER(&stFskClid, (TstFskClid *)user, sizeof(TstFskClid));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	if ( 1 == Is_DAA_Channel(stFskClid.ch_id))
	{
		//printk("chid%d is not for SLIC, can not gen FSK caller ID!\n", stFskClid.ch_id);
		return 0;
	}

	SLIC_gen_FSK_CID(stFskClid.ch_id, stFskClid.service_type, FSK_MSG_CALLSETUP , ( TstFskClidData * )stFskClid.cid_data);

#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_FSK_CID_MDMF_GEN( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstFskClid stFskClid;
	int ret;

	COPY_FROM_USER(&stFskClid, (TstFskClid *)user, sizeof(TstFskClid));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	if ( 1 == Is_DAA_Channel(stFskClid.ch_id))
	{
		//printk("chid%d is not for SLIC, can not gen FSK caller ID!\n", stFskClid.ch_id);
		return 0;
	}
	fsk_cid_state[stFskClid.ch_id] = 1;
	PRINT_R("Need implement for do_mgr_VOIP_MGR_FSK_CID_MDMF_GEN.\n");

#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_CALLERID
 * @brief Generate FSK VMWI
 * @param TstVoipCID.ch_id Channel ID
 * @param TstVoipCID.string Caller ID - Phonenumber
 * @see VOIP_MGR_SET_FSK_VMWI_STATE TstVoipCID
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_FSK_VMWI_STATE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCID stCIDstr;
	int ret;

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	TstFskClidData cid_data[FSK_MDMF_SIZE];
#endif

	// thlin +
	COPY_FROM_USER(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	if ( 1 == Is_DAA_Channel(stCIDstr.ch_id))
	{
		//printk("chid%d is not for SLIC, can not gen FSK caller ID!\n", stCIDstr.ch_id);
		return 0;
	}

	cid_data[0].type = FSK_PARAM_MW;
	strcpy(cid_data[0].data, stCIDstr.string);

	cid_data[1].type = FSK_PARAM_NULL;
	cid_data[1].data[0] = 0;

	SLIC_gen_VMWI(stCIDstr.ch_id, cid_data);

#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_FSK_VMWI_STATE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCID stCIDstr;
	int ret;

	COPY_FROM_USER(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	fsk_cid_state[stCIDstr.ch_id] = 1;
	RtkAc49xApiSendVmwi(stCIDstr.ch_id, FSK_MSG_MWSETUP, stCIDstr.string);
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_CALLERID
 * @brief Set FSK area
 * @param TstVoipCID.ch_id Channel ID
 * @param TstVoipCID.cid_gain Caller ID gain. Only support multiple 1~5.
 * @param TstVoipCID.cid_mode Caller ID mode. <br>
 *        bit 0-2: FSK Type. (0:Bellcore 1:ETSI 2:BT 3:NTT) <br>
 *        bit 3: Caller ID Prior First Ring <br>
 *        bit 4: Dual Tone before Caller ID (Fsk Alert Tone) <br>
 *        bit 5: Short Ring before Caller ID <br>
 *        bit 6: Reverse Polarity before Caller ID (Line Reverse) <br>
 *        bit 7: FSK Date & Time Sync and Display Name <br>
 * @see VOIP_MGR_SET_FSK_AREA TstVoipCfg 
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_FSK_AREA( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCfg stVoipCfg;
	int ret;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	fsk_spec_areas[stVoipCfg.ch_id] = stVoipCfg.cfg; /* 0:Bellcore 1:ETSI 2:BT 3:NTT */
	// fsk_spec_areas[]:
	// bit0-2: FSK Type
	// bit 3: Caller ID Prior First Ring
	// bit 4: Dual Tone before Caller ID (Fsk Alert Tone)
	// bit 5: Short Ring before Caller ID
	// bit 6: Reverse Polarity before Caller ID (Line Reverse)
	// bit 7: FSK Date & Time Sync and Display Name
	// bit 8: Auto SLIC Action


	if ((stVoipCfg.cfg&7) == 0)      PRINT_MSG("Set FSK Caller ID Area to Bellcore, ch=%d.\n", stVoipCfg.ch_id);
   	else if ((stVoipCfg.cfg&7) == 1) PRINT_MSG("Set FSK Caller ID Area to ETSI, ch=%d.\n", stVoipCfg.ch_id);
	else if ((stVoipCfg.cfg&7) == 2) PRINT_MSG("Set FSK Caller ID Area to BT, ch=%d.\n", stVoipCfg.ch_id);
	else if ((stVoipCfg.cfg&7) == 3) PRINT_MSG("Set FSK Caller ID Area to NTT, ch=%d.\n", stVoipCfg.ch_id);
    else				     PRINT_MSG("NOT Support this FSK Area.\n");
	PRINT_MSG("\nReset the fsk setting %d\n",fsk_spec_areas[stVoipCfg.ch_id]);

	if ( 1 == Is_DAA_Channel(stVoipCfg.ch_id))
	{
		if (stVoipCfg.cfg&0x08)
			ring_times_set(stVoipCfg.ch_id, 1, 0); // chid, ring_on_times, ring_off_times
		else
			ring_times_set(stVoipCfg.ch_id, 2, 1);
	}
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_FSK_AREA( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCfg stVoipCfg;
	int ret;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	fsk_spec_areas[stVoipCfg.ch_id] = stVoipCfg.cfg; /* 0:Bellcore 1:ETSI 2:BT 3:NTT */
	// fsk_spec_areas[]:
	// bit0-2: FSK Type
	// bit 3: Caller ID Prior First Ring
	// bit 4: Dual Tone before Caller ID (Fsk Alert Tone)
	// bit 5: Short Ring before Caller ID
	// bit 6: Reverse Polarity before Caller ID (Line Reverse)
	// bit 7: FSK Date & Time Sync and Display Name
	// bit 8: Auto SLIC Ringing
	
	fsk_cid_gain[stVoipCfg.ch_id] = 1; /* Only support multiple 1~5 */

	if ((stVoipCfg.cfg&7) == 0)      PRINT_MSG("Set FSK Caller ID Area to Bellcore, ch=%d.\n", stVoipCfg.ch_id);
    else if ((stVoipCfg.cfg&7) == 1) PRINT_MSG("Set FSK Caller ID Area to ETSI, ch=%d.\n", stVoipCfg.ch_id);
    else if ((stVoipCfg.cfg&7) == 2) PRINT_MSG("Set FSK Caller ID Area to BT, ch=%d.\n", stVoipCfg.ch_id);
    else if ((stVoipCfg.cfg&7) == 3) PRINT_MSG("Set FSK Caller ID Area to NTT, ch=%d.\n", stVoipCfg.ch_id);
    else				     PRINT_MSG("NOT Support this FSK Area.\n");
	PRINT_MSG("\nReset the fsk setting %d\n",fsk_spec_areas[stVoipCfg.ch_id]);

	if ( 1 == Is_DAA_Channel(stVoipCfg.ch_id))
	{
		if (stVoipCfg.cfg&0x08)
			ring_times_set(stVoipCfg.ch_id, 1, 0); // chid, ring_on_times, ring_off_times
		else
			ring_times_set(stVoipCfg.ch_id, 2, 1);
	}
#endif
	return 0;
}
#endif


/**
 * @ingroup VOIP_FXS_CALLERID
 * @brief Set the FSK Caller ID Parameters
 * @param chid The FXS channel number.
 * @param para The variable pointer of FSK Caller ID Parameters
 * @see VOIP_MGR_SET_FSK_CLID_PARA TstVoipFskPara
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_FSK_CLID_PARA( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipFskPara stVoipFskPara;
	int ret;

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	TstFskClidPara clid_para;
#endif

	COPY_FROM_USER(&stVoipFskPara, (TstVoipFskPara *)user, sizeof(TstVoipFskPara));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	clid_para.ch_seizure_cnt = stVoipFskPara.CS_cnt;
	clid_para.mark_cnt = stVoipFskPara.mark_cnt;
	clid_para.mark_gain = stVoipFskPara.mark_gain;
	clid_para.space_gain = stVoipFskPara.space_gain;
	clid_para.type2_expected_ack_tone = stVoipFskPara.type2_expected_ack_tone;
	clid_para.delay_after_1st_ring = stVoipFskPara.delay_after_1st_ring;
	clid_para.delay_before_2nd_ring = stVoipFskPara.delay_before_2nd_ring;
	clid_para.silence_before_sas = stVoipFskPara.silence_before_sas;
	clid_para.sas_time = stVoipFskPara.sas_time;
	clid_para.delay_after_sas = stVoipFskPara.delay_after_sas;
	clid_para.cas_time = stVoipFskPara.cas_time;
	clid_para.type1_delay_after_cas = stVoipFskPara.type1_delay_after_cas;
	clid_para.ack_waiting_time = stVoipFskPara.ack_waiting_time;
	clid_para.delay_after_ack_recv = stVoipFskPara.delay_after_ack_recv;
	clid_para.delay_after_type2_fsk = stVoipFskPara.delay_after_type2_fsk;

	fsk_cid_para_set(stVoipFskPara.ch_id, stVoipFskPara.area, &clid_para);
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_FSK_CLID_PARA( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	PRINT_R("Not support or need implement for do_mgr_VOIP_MGR_SET_FSK_CLID_PARA().\n");
	return NO_COPY_TO_USER( cmd, seq_no );
}
#endif

/**
 * @ingroup VOIP_FXS_CALLERID
 * @brief Get the FSK Caller ID Parameters
 * @param chid The FXS channel number.
 * @param para The variable pointer to save current FSK Caller ID Parameters
 * @see VOIP_MGR_GET_FSK_CLID_PARA TstVoipFskPara
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_GET_FSK_CLID_PARA( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipFskPara stVoipFskPara;

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	TstFskClidPara para;
#endif

	COPY_FROM_USER(&stVoipFskPara, (TstVoipFskPara *)user, sizeof(TstVoipFskPara));

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	fsk_cid_para_get(stVoipFskPara.ch_id, stVoipFskPara.area, &para);

	stVoipFskPara.CS_cnt = para.ch_seizure_cnt;
	stVoipFskPara.mark_cnt = para.mark_cnt;
	stVoipFskPara.mark_gain = para.mark_gain;
	stVoipFskPara.space_gain = para.space_gain;
	stVoipFskPara.type2_expected_ack_tone = para.type2_expected_ack_tone;
	stVoipFskPara.delay_after_1st_ring = para.delay_after_1st_ring;
	stVoipFskPara.delay_before_2nd_ring = para.delay_before_2nd_ring;
	stVoipFskPara.silence_before_sas = para.silence_before_sas;
	stVoipFskPara.sas_time = para.sas_time;
	stVoipFskPara.delay_after_sas = para.delay_after_sas;
	stVoipFskPara.cas_time = para.cas_time;
	stVoipFskPara.type1_delay_after_cas = para.type1_delay_after_cas;
	stVoipFskPara.ack_waiting_time = para.ack_waiting_time;
	stVoipFskPara.delay_after_ack_recv = para.delay_after_ack_recv;
	stVoipFskPara.delay_after_type2_fsk = para.delay_after_type2_fsk;
#endif

	return COPY_TO_USER(user, &stVoipFskPara, sizeof(TstVoipFskPara), cmd, seq_no);
	//return 0;
}
#else
int do_mgr_VOIP_MGR_GET_FSK_CLID_PARA( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipFskPara stVoipFskPara;
	COPY_FROM_USER(&stVoipFskPara, (TstVoipFskPara *)user, sizeof(TstVoipFskPara));
	PRINT_R("Not support or need implement for do_mgr_VOIP_MGR_GET_FSK_CLID_PARA().\n");
	return COPY_TO_USER(user, &stVoipFskPara, sizeof(TstVoipFskPara), cmd, seq_no);
}
#endif

/**
 * @ingroup VOIP_DSP_CALLERID
 * @brief Generate FSK alert
 * @param TstVoipCID.string Caller ID - Phonenumber
 * @see VOIP_MGR_FSK_ALERT_GEN_CFG TstVoipCID
 */
  // not used for DSP
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_FSK_ALERT_GEN_CFG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCID stCIDstr;
	int ret;

	COPY_FROM_USER(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	//fsk_alert_state[stCIDstr.ch_id] = 1;
	strcpy(dtmf_cid_info[stCIDstr.ch_id].data, stCIDstr.string);
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_FSK_ALERT_GEN_CFG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCID stCIDstr;
	int ret;

	COPY_FROM_USER(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

	//fsk_alert_state[stCIDstr.ch_id] = 1;
	//strcpy(cid_str, stCIDstr.string);
	return 0;
}
#endif

/**
 * @ingroup VOIP_MGR_STOP_CID
 * @brief Stop DTMF/FSK Caller ID
 * @param TstVoipCfg.ch_id Channel ID 
 * @param TstVoipCfg.cfg 0: DTMF Caller ID, 1: FSK Caller ID, 2: both DTMF and FSK Caller ID
 * @see VOIP_MGR_STOP_CID TstVoipCfg 
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_STOP_CID( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCfg stVoipCfg;
	int ret;
	extern void cid_dtmf_gen_init(int32 chid);

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_ETHERNET_DSP_IS_HOST
	// Host auto forward 
#else
	if (stVoipCfg.cfg == 0) // DTMF
	{
#ifdef SW_DTMF_CID
		cid_dtmf_gen_init(stVoipCfg.ch_id);
#endif
	}
	else if (stVoipCfg.cfg == 1) // FSK
	{
		fsk_gen_init(stVoipCfg.ch_id);
	}
	else if (stVoipCfg.cfg == 2) // DTMF and FSK
	{
#ifdef SW_DTMF_CID
		cid_dtmf_gen_init(stVoipCfg.ch_id);
#endif
		fsk_gen_init(stVoipCfg.ch_id);
	}
#endif

	return 0;
}
#else
int do_mgr_VOIP_MGR_STOP_CID( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	// no handler 
	return NO_COPY_TO_USER( cmd, seq_no );
}
#endif

/**
 * @ingroup VOIP_DSP_CALLERID
 * @brief Set DTMF mode of caller ID
 * @param TstVoipCID.ch_id Channel ID
 * @param TstVoipCID.cid_mode Caller ID mode <br>
 *        bit0-2: FSK Type <br>
 *        bit 3: Normal Ring <br>
 *        bit 4: Fsk Alert Tone <br>
 *        bit 5: Short Ring <br>
 *        bit 6: Line Reverse <br>
 *        bit 7: Date, Time Sync and Name <br>
 * @param TstVoipCID.cid_dtmf_mode Caller ID in DTMF mode. <br>
 *        0-1 bits for starting digit, and 2-3 bits for ending digit. <br>
 *        00: A, 01: B, 02: C, 03: D
 * @see VOIP_MGR_SET_CID_DTMF_MODE TstVoipCID
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_CID_DTMF_MODE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCID stCIDstr;
	int ret;

	COPY_FROM_USER(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else

	dtmf_cid_info[stCIDstr.ch_id].on_duration = stCIDstr.cid_dtmf_on_ms/10;
	dtmf_cid_info[stCIDstr.ch_id].pause_duration = stCIDstr.cid_dtmf_pause_ms/10;
	dtmf_cid_info[stCIDstr.ch_id].pre_silence_duration = stCIDstr.cid_dtmf_pre_silence_ms/10;
	dtmf_cid_info[stCIDstr.ch_id].end_silence_duration = stCIDstr.cid_dtmf_end_silence_ms/10;
	//printk("on=%d, off=%d\n", stCIDstr.cid_dtmf_on_ms, stCIDstr.cid_dtmf_pause_ms);
	

	//PRINT_G("cid_dtmf_mode[%d]=0x%x", stCIDstr.ch_id, stCIDstr.cid_dtmf_mode);
	dtmf_cid_info[stCIDstr.ch_id].bAuto_Ring = (stCIDstr.cid_dtmf_mode&0x20)>>5;
	dtmf_cid_info[stCIDstr.ch_id].bAuto_SLIC_action = (stCIDstr.cid_dtmf_mode&0x80)>>7;
	dtmf_cid_info[stCIDstr.ch_id].bBefore1stRing = (stCIDstr.cid_dtmf_mode&0x40)>>6; 	/* set the before ring or after ring send cid */
	dtmf_cid_info[stCIDstr.ch_id].bAuto_StartEnd = (stCIDstr.cid_dtmf_mode&0x10)>>4;
	dtmf_cid_info[stCIDstr.ch_id].start_digit =  stCIDstr.cid_dtmf_mode&0x03;
	dtmf_cid_info[stCIDstr.ch_id].end_digit =  (stCIDstr.cid_dtmf_mode&0xC)>>2;
	
	if ( 1 == Is_DAA_Channel(stCIDstr.ch_id))
	{
		if (dtmf_cid_info[stCIDstr.ch_id].bBefore1stRing)
			ring_times_set(stCIDstr.ch_id, 1, 0); // chid, ring_on_times, ring_off_times
		else
			ring_times_set(stCIDstr.ch_id, 2, 1);
	}
#endif
	//PRINT_MSG("fsk_spec_areas[%d]=0x%x\n", stCIDstr.ch_id, stCIDstr.cid_mode);
	//PRINT_MSG("cid_dtmf_mode[%d]=0x%x", stCIDstr.ch_id, stCIDstr.cid_dtmf_mode);
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_CID_DTMF_MODE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCID stCIDstr;
	int ret;

	COPY_FROM_USER(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	dtmf_cid_info[stCIDstr.ch_id].bBefore1stRing = (stCIDstr.cid_dtmf_mode&0x08)>>3;	/* set the before ring or after ring send cid */
	dtmf_cid_info[stCIDstr.ch_id].bAuto_StartEnd = (stCIDstr.cid_dtmf_mode&0x10)>>4;
	dtmf_cid_info[stCIDstr.ch_id].start_digit =  stCIDstr.cid_dtmf_mode&0x3;
	dtmf_cid_info[stCIDstr.ch_id].end_digit =  (stCIDstr.cid_dtmf_mode&0xC)>>2;

	if ( 1 == Is_DAA_Channel(stCIDstr.ch_id))
	{
		if (stCIDstr.cid_mode&0x08)
			ring_times_set(stCIDstr.ch_id, 1, 0); // chid, ring_on_times, ring_off_times
		else
			ring_times_set(stCIDstr.ch_id, 2, 1);
	}
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_CALLERID
 * @brief Set caller ID detection mode
 * @param TstVoipCfg.ch_id Channel ID
 * @param TstVoipCfg.enable Auto mode selection. <br>
 *        (0: off, 1: auto mode (NTT support), 2: auto mode (NTT not support))
 * @param TstVoipCfg.cfg If auto mode is off, use this parameter to choose detection mode. <br>
 *        0: Bellcore FSK <br>
 *        1: ETSI FSK <br>
 *        2: BT FSK <br>
 *        3: NTT FSK <br>
 *        4: DTMF <br>
 * @see VOIP_MGR_SET_CID_DET_MODE TstVoipCfg
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_CID_DET_MODE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef FXO_CALLER_ID_DET
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	unsigned long flags;
#endif
	TstVoipCfg stVoipCfg;
	int ret;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	save_flags(flags); cli();
	auto_cid_det[stVoipCfg.ch_id] = stVoipCfg.enable;
	cid_type[stVoipCfg.ch_id] = stVoipCfg.cfg;
	restore_flags(flags);

	if (stVoipCfg.enable == 0)
	{
		if (stVoipCfg.cfg == 0)
	                    PRINT_MSG("Set Caller ID Detection Mode to Bellcore FSK.\n");
		else if (stVoipCfg.cfg == 1)
	                	PRINT_MSG("Set Caller ID Detection Mode to ETSI FSK.\n");
		else if (stVoipCfg.cfg == 2)
	                	PRINT_MSG("Set Caller ID Detection Mode to BT FSK.\n");
		else if (stVoipCfg.cfg == 3)
	                	PRINT_MSG("Set Caller ID Detection Mode to NTT FSK.\n");
		else if (stVoipCfg.cfg == 4)
	                	PRINT_MSG("Set Caller ID Detection Mode to DTMF.\n");
	}
	else if (stVoipCfg.enable == 1)
	        	PRINT_MSG("Enable Auto Caller ID Detection Mode (NTT Support).\n");
	else if (stVoipCfg.enable == 2)
	        	PRINT_MSG("Enable Auto Caller ID Detection Mode (NTT Not Support).\n");
#endif
#else
	return NO_COPY_TO_USER( cmd, seq_no );
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_CID_DET_MODE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef FXO_CALLER_ID_DET
	unsigned long flags;
	TstVoipCfg stVoipCfg;
	int ret;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	save_flags(flags); cli();
	auto_cid_det[stVoipCfg.ch_id] = stVoipCfg.enable;
	cid_type[stVoipCfg.ch_id] = stVoipCfg.cfg;
	restore_flags(flags);

	if (stVoipCfg.enable == 0)
	{
		PRINT_MSG("Set CH%d Caller ID Detection Mode to ", stVoipCfg.ch_id);

		if (stVoipCfg.cfg == 0)
		{
			PRINT_MSG("Bellcore FSK.\n");
			RtkAc49xApiSetCallerIdDetection( stVoipCfg.ch_id, CONTROL__ENABLE, CONTROL__ENABLE, CALLER_ID_STANDARD__TELCORDIA_BELLCORE);
		}
		else if (stVoipCfg.cfg == 1)
		{
			PRINT_MSG("ETSI FSK.\n");
			RtkAc49xApiSetCallerIdDetection( stVoipCfg.ch_id, CONTROL__ENABLE, CONTROL__ENABLE, CALLER_ID_STANDARD__ETSI);
		}
		else if (stVoipCfg.cfg == 2)
		{
			PRINT_MSG("BT FSK.\n");
			RtkAc49xApiSetCallerIdDetection( stVoipCfg.ch_id, CONTROL__ENABLE, CONTROL__ENABLE, CALLER_ID_STANDARD__ETSI);
		}
		else if (stVoipCfg.cfg == 3)
		{
			PRINT_MSG("NTT FSK.\n");
			RtkAc49xApiSetCallerIdDetection( stVoipCfg.ch_id, CONTROL__ENABLE, CONTROL__ENABLE, CALLER_ID_STANDARD__NTT);
		}
		else if (stVoipCfg.cfg == 4)
		{
			PRINT_MSG("DTMF.\n");
			RtkAc49xApiSetCallerIdDetection( stVoipCfg.ch_id, CONTROL__ENABLE, CONTROL__ENABLE, CALLER_ID_STANDARD__DTMF_CLIP_ETSI);
		}
	}
	else if (stVoipCfg.enable == 1)
	{
		RtkAc49xApiSetCallerIdDetection( stVoipCfg.ch_id, CONTROL__ENABLE, CONTROL__ENABLE, CALLER_ID_STANDARD__DTMF_CLIP_ETSI);
		printk(AC_FORE_GREEN "Warning: ACMW doesn't support Auto Caller ID Detection Mode."
		"Set Caller ID Detection Mode to DTMF.\n" AC_RESET);
		//PRINT_MSG("Enable Auto Caller ID Detection Mode (NTT Support).\n");
	}
	else if (stVoipCfg.enable == 2)
	{
		RtkAc49xApiSetCallerIdDetection( stVoipCfg.ch_id, CONTROL__ENABLE, CONTROL__ENABLE, CALLER_ID_STANDARD__DTMF_CLIP_ETSI);
		printk(AC_FORE_GREEN "Warning: ACMW doesn't support Auto Caller ID Detection Mode."
		"Set Caller ID Detection Mode to DTMF.\n" AC_RESET);
		//PRINT_MSG("Enable Auto Caller ID Detection Mode (NTT Not Support).\n");
	}
#endif
#else
	return NO_COPY_TO_USER( cmd, seq_no );
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_CALLERID
 * @brief Get FSK caller ID state
 * @param TstVoipCID.ch_id Channel ID
 * @param TstVoipCID.cid_state Caller ID state
 * @see VOIP_MGR_GET_FSK_CID_STATE_CFG TstVoipCID
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_GET_FSK_CID_STATE_CFG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCID stCIDstr;

	COPY_FROM_USER(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	stCIDstr.cid_state = fsk_cid_state[stCIDstr.ch_id];

		if (!fsk_cid_state[stCIDstr.ch_id])
			init_softfskcidGen(stCIDstr.ch_id);

#endif
	return COPY_TO_USER(user, &stCIDstr, sizeof(TstVoipCID), cmd, seq_no);
	//return 0;
}
#else
int do_mgr_VOIP_MGR_GET_FSK_CID_STATE_CFG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCID stCIDstr;

	// Get FSK CID state
	COPY_FROM_USER(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	stCIDstr.cid_state = fsk_cid_state[stCIDstr.ch_id];
#endif
	return COPY_TO_USER(user, &stCIDstr, sizeof(TstVoipCID), cmd, seq_no);
	//return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_CALLERID
 * @brief Set caller ID FSK generation mode
 * @param TstVoipCfg.ch_id Channel ID
 * @param TstVoipCfg.enable Generation mode (0: hardware FSK caller id, 1:software FSK caller id)
 * @see VOIP_MGR_SET_CID_FSK_GEN_MODE TstVoipCfg
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_CID_FSK_GEN_MODE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	// Now, SW FSK caller generation is used as default.
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_CID_FSK_GEN_MODE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	// no handler
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_CALLERID
 * @brief Generate FSK VMWI
 * @param TstVoipCID.ch_id Channel ID
 * @param TstVoipCID.cid_mode Caller ID mode (0:on-hook type1, 1:off-hook type2)
 * @param TstVoipCID.cid_msg_type Message type <br>
 *        0x80: Call Set-up <br>
 *        0x82: Message Waiting (VMWI) <br>
 *        0x86: Advice of Charge <br>
 *        0x89: Short Message Service <br>
 * @param TstVoipCID.string Caller ID - phonenumber
 * @param TstVoipCID.string2 Caller ID - Date and time
 * @param TstVoipCID.cid_name caller ID - Name
 * @see VOIP_MGR_FSK_CID_VMWI_GEN_CFG TstVoipCID
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_FSK_CID_VMWI_GEN_CFG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	//extern void genSoftFskCID (uint32_t chid, char mode, unsigned char msg_type, char *str, char *str2, char *cid_name);	// hcv, generate Caller ID
	TstVoipCID stCIDstr;
	int ret = 0;

	COPY_FROM_USER(&stCIDstr, (TstVoipCID *)user, sizeof(TstVoipCID));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
#if 1
	PRINT_R("Need implement for do_mgr_VOIP_MGR_FSK_CID_VMWI_GEN_CFG\n");
	//stCIDstr.ret_val = -1;
	ret = -1;
#else
	fsk_cid_state[stCIDstr.ch_id]=1;
	// remember set slic in transmit mode, enable DSP pcm.
	init_softfskcidGen(stCIDstr.ch_id);
	//genSoftFskCID(stCIDstr.ch_id, stCIDstr.cid_mode, stCIDstr.cid_msg_type, stCIDstr.string, stCIDstr.string2, stCIDstr.cid_name);
	//stCIDstr.ret_val = 0;

#endif
	PRINT_R("Need implement for do_mgr_VOIP_MGR_FSK_CID_VMWI_GEN_CFG\n");
#endif
	return ret;
}
#else
int do_mgr_VOIP_MGR_FSK_CID_VMWI_GEN_CFG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	// no handler
	return NO_COPY_TO_USER( cmd, seq_no );
	//return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_TONE
 * @brief Play a tone
 * @see VOIP_MGR_SETPLAYTONE TstVoipPlayToneConfig
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SETPLAYTONE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	unsigned long flags;
	uint32 ch_id, m_id, s_id;
#endif
	TstVoipPlayToneConfig stVoipPlayToneConfig;
	int ret;

	COPY_FROM_USER(&stVoipPlayToneConfig, (TstVoipPlayToneConfig*)user, sizeof(TstVoipPlayToneConfig));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#if 0
	if( stVoipPlayToneConfig.nTone == 13 )
		return 0;
#endif

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else

	ch_id = stVoipPlayToneConfig.ch_id;
	m_id = stVoipPlayToneConfig.m_id;

	s_id = API_GetSid(ch_id, m_id);
	PRINT_MSG("VOIP_MGR_SETPLAYTONE:ch_id=%d, m_id=%d, s_id=%d, nTone=%d, bFlag=%x, path=%d\n", ch_id, m_id, s_id, stVoipPlayToneConfig.nTone, stVoipPlayToneConfig.bFlag, stVoipPlayToneConfig.path);

	save_flags(flags); cli();
	hc_SetPlayTone(ch_id, s_id, stVoipPlayToneConfig.nTone, stVoipPlayToneConfig.bFlag, stVoipPlayToneConfig.path);
	restore_flags(flags);
/*
	save_flags(flags); cli();

	if(stVoipPlayToneConfig.bFlag)
	{
		if(s_nCurPlayTone[ch_id] != -1)
		{
			//if(pcfg->nTone != nCurPlayTone)
			DspcodecPlayTone(ch_id, s_nCurPlayTone[ch_id], 0, stVoipPlayToneConfig.path);
			// before open a new tone, must close old first
		}
		s_nCurPlayTone[ch_id] = stVoipPlayToneConfig.nTone;
		DspcodecPlayTone(ch_id, s_nCurPlayTone[ch_id], stVoipPlayToneConfig.bFlag, stVoipPlayToneConfig.path);
	}
	else
	{
		if(s_nCurPlayTone[ch_id] != -1)
		{
			DspcodecPlayTone(ch_id, s_nCurPlayTone[ch_id], stVoipPlayToneConfig.bFlag, stVoipPlayToneConfig.path);
			s_nCurPlayTone[ch_id] = -1;
			//if(pcfg->nTone != nCurPlayTone)
			//DspcodecPlayTone(chid, nCurPlayTone, 0, pcfg->path);
			//before open a new tone, must close old first
		}
	}
	s_tonepath[ch_id] = stVoipPlayToneConfig.path;

	restore_flags(flags);
*/
	//PRINT_MSG("%s-%d\n", __FUNCTION__, __LINE__);
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_SETPLAYTONE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	unsigned long flags;
	uint32 s_id;
	TstVoipPlayToneConfig stVoipPlayToneConfig;
	int ret;

	/** Play Tone **/
	COPY_FROM_USER(&stVoipPlayToneConfig, (TstVoipPlayToneConfig*)user, sizeof(TstVoipPlayToneConfig));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	save_flags(flags); cli();
	s_id = 2*stVoipPlayToneConfig.ch_id + PROTOCOL__RTP - 1;

	if ((stVoipPlayToneConfig.bFlag == 0x80) ||(stVoipPlayToneConfig.bFlag == 0x81))
		stVoipPlayToneConfig.bFlag = stVoipPlayToneConfig.bFlag - 0x80;

	RtkAc49xApiPlayTone(stVoipPlayToneConfig.ch_id, s_id, stVoipPlayToneConfig.nTone, stVoipPlayToneConfig.bFlag, stVoipPlayToneConfig.path);

	restore_flags(flags);
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_TONE
 * @brief Set the tone, busy tone detection parametes, and SLIC impedance according to the Country
 * @param TstVoipValue.value Country
 *        - 0: USA
 *        - 1: UK
 *        - 2: Australia
 *        - 3: HK
 *        - 4: Japan
 *        - 5: Sweden
 *        - 6: Germany
 *        - 7: France
 *        - 8: Taiwan
 *        - 9: Belgium
 *        - 10: Finland
 *        - 11: Italy
 *        - 12: China
 *        - 13: Extend #1
 *        - 14: Extend #2
 *        - 15: Extend #3
 *        - 16: Extend #4
 *        - 17: Customer
 * @see VOIP_MGR_SET_COUNTRY TstVoipValue
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_COUNTRY( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	uint32 sid, chid;
#endif

#ifdef FXO_BUSY_TONE_DET
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	extern void ring_tone_det_cfg_apply( void );
	extern ToneCfgParam_t ToneTable[];

	int cad_on, cad_off, er_on, er_off;
#endif
#endif

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	for(sid=0; sid<DSP_RTK_SS_NUM; sid++)	//Set the same country for each session.
		DspcodecSetCountry( sid, /*country*/stVoipValue.value);
	//for (chid=0; chid<SLIC_CH_NUM; chid++)
	for (chid=0; chid<CON_CH_NUM; chid++) {
		if( get_snd_type_cch( chid ) != SND_TYPE_FXS )
			continue;
		SLIC_Set_Impendance_Country(chid, stVoipValue.value /*country*/, 0 /* impedance value: reserved */);
	}

#ifdef FXO_BUSY_TONE_DET
	switch (stVoipValue.value)
	{
		case DSPCODEC_COUNTRY_USA://USA
		case DSPCODEC_COUNTRY_HK://HK
		case DSPCODEC_COUNTRY_TW://Taiwan
			det_freq[0] = FREQ_480HZ;
			det_freq[1] = FREQ_620HZ;
			break;
		case DSPCODEC_COUNTRY_FR://France
		case DSPCODEC_COUNTRY_BE://Belgium
			det_freq[0] = FREQ_440HZ;
			det_freq[1] = FREQ_NA;
			break;
		case DSPCODEC_COUNTRY_UK://UK
		case DSPCODEC_COUNTRY_AUSTRALIA://Australia
		case DSPCODEC_COUNTRY_JP://Japan
			det_freq[0] = FREQ_400HZ;
			det_freq[1] = FREQ_NA;
			break;
		case DSPCODEC_COUNTRY_SE://Sweden
		case DSPCODEC_COUNTRY_GR://Germany
		case DSPCODEC_COUNTRY_FL://Finland
		case DSPCODEC_COUNTRY_IT://Italy
			det_freq[0] = FREQ_425HZ;
			det_freq[1] = FREQ_NA;
			break;
		case DSPCODEC_COUNTRY_CN://China
			det_freq[0] = FREQ_450HZ;
			det_freq[1] = FREQ_NA;
			break;
#ifdef COUNTRY_TONE_RESERVED
		case DSPCODEC_COUNTRY_RESERVE:
#endif
		case DSPCODEC_COUNTRY_EX1:
		case DSPCODEC_COUNTRY_EX2:
		case DSPCODEC_COUNTRY_EX3:
		case DSPCODEC_COUNTRY_EX4:
		case DSPCODEC_COUNTRY_CUSTOME://Customer
			//det_freq[0] = FREQ_450HZ;
			//det_freq[1] = FREQ_NA;
			break;

		default://USA
			det_freq[0] = FREQ_480HZ;
			det_freq[1] = FREQ_620HZ;
			break;
	}

	stVoiptonedet_parm.frequency1 = ToneTable[ (USA_DIAL-1) + 25*stVoipValue.value + 6].Freq0;
	stVoiptonedet_parm.frequency2 = ToneTable[ (USA_DIAL-1) + 25*stVoipValue.value + 6].Freq1;

	cad_on = ( ToneTable[ (USA_DIAL-1) + 25*stVoipValue.value + 6].CadOn0 )/10;
	cad_off = ( ToneTable[ (USA_DIAL-1) + 25*stVoipValue.value + 6].CadOff0 )/10;
	/* 12.5% inaccuracy */
	er_on = ( cad_on >> 3 );
	er_off = ( cad_off >> 3 );

	stVoiptonedet_parm.busytone_on_low_limit = cad_on - er_on;
	stVoiptonedet_parm.busytone_on_up_limit = cad_on + er_on;
	stVoiptonedet_parm.busytone_off_low_limit = cad_off - er_off;
	stVoiptonedet_parm.busytone_off_up_limit = cad_off + er_off;

	busy_tone_det_cfg_apply();

    //printk("%s(%d)country=%d, busy freq1=%d, freq2=%d\n",
    //	__FUNCTION__,__LINE__,stVoipValue.value,stVoiptonedet_parm.frequency1, stVoiptonedet_parm.frequency2);

	stVoiptonedet_parm.frequency1 = ToneTable[ (USA_DIAL-1) + 25*stVoipValue.value + 5].Freq0;
	stVoiptonedet_parm.frequency2 = ToneTable[ (USA_DIAL-1) + 25*stVoipValue.value + 5].Freq1;

	cad_on = ( ToneTable[ (USA_DIAL-1) + 25*stVoipValue.value + 5].CadOn0 )/10;
	cad_off = ( ToneTable[ (USA_DIAL-1) + 25*stVoipValue.value + 5].CadOff0 )/10;
	/* 12.5% inaccuracy */
	er_on = ( cad_on >> 3 );
	er_off = ( cad_off >> 3 );

	stVoiptonedet_parm.busytone_on_low_limit = cad_on - er_on;
	stVoiptonedet_parm.busytone_on_up_limit = cad_on + er_on;
	stVoiptonedet_parm.busytone_off_low_limit = cad_off - er_off;
	stVoiptonedet_parm.busytone_off_up_limit = cad_off + er_off;

	ring_tone_det_cfg_apply();

    //printk("%s(%d)country=%d, ring freq1=%d, freq2=%d\n",
    //  __FUNCTION__,__LINE__,stVoipValue.value,stVoiptonedet_parm.frequency1, stVoiptonedet_parm.frequency2);

#endif
	switch (stVoipValue.value)
	{
		case DSPCODEC_COUNTRY_USA://USA
			PRINT_MSG("Set Tone of Country to USA\n");
			break;
		case DSPCODEC_COUNTRY_UK://UK
			PRINT_MSG("Set Tone of Country to UK\n");
			break;
		case DSPCODEC_COUNTRY_AUSTRALIA://Australia
			PRINT_MSG("Set Tone of Country to AUSTRALIA\n");
			break;
		case DSPCODEC_COUNTRY_HK://HK
			PRINT_MSG("Set Tone of Country to HONG KONG\n");
			break;
		case DSPCODEC_COUNTRY_JP://Japan
			PRINT_MSG("Set Tone of Country to JAPAN\n");
			break;
		case DSPCODEC_COUNTRY_SE://Sweden
			PRINT_MSG("Set Tone of Country to SWEDEN\n");
			break;
		case DSPCODEC_COUNTRY_GR://Germany
			PRINT_MSG("Set Tone of Country to GERMANY\n");
			break;
		case DSPCODEC_COUNTRY_FR://France
			PRINT_MSG("Set Tone of Country to FRANCE\n");
			break;
		case DSPCODEC_COUNTRY_TW://Taiawn
		//case DSPCODEC_COUNTRY_TR://TR57
			//PRINT_MSG("Set Tone of Country to TR57\n");
			PRINT_MSG("Set Tone of Country to TAIWAN\n");
			break;
		case DSPCODEC_COUNTRY_BE://Belgium
			PRINT_MSG("Set Tone of Country to BELGIUM\n");
			break;
		case DSPCODEC_COUNTRY_FL://Finland
			PRINT_MSG("Set Tone of Country to FINLAND\n");
			break;
		case DSPCODEC_COUNTRY_IT://Italy
			PRINT_MSG("Set Tone of Country to ITALY\n");
			break;
		case DSPCODEC_COUNTRY_CN://China
			PRINT_MSG("Set Tone of Country to CHINA\n");
			break;
		case DSPCODEC_COUNTRY_EX1://extend #1
			PRINT_MSG("Set Tone of Country to extend #1\n");
			break;
		case DSPCODEC_COUNTRY_EX2://extend #2
			PRINT_MSG("Set Tone of Country to extend #2\n");
			break;
		case DSPCODEC_COUNTRY_EX3://extend #3
			PRINT_MSG("Set Tone of Country to extend #3\n");
			break;
		case DSPCODEC_COUNTRY_EX4://extend #4
			PRINT_MSG("Set Tone of Country to extend #4\n");
			break;
#ifdef COUNTRY_TONE_RESERVED
		case DSPCODEC_COUNTRY_RESERVE:
			PRINT_MSG("Set Tone of Country to Reserve\n");
			break;
#endif
		case DSPCODEC_COUNTRY_CUSTOME://Customer
			PRINT_MSG("Set Tone of Country to CUSTOMER\n");
			break;
		default:
			PRINT_MSG("The tone you select is not support!\n");
			break;
	}

	if (stVoipValue.value == DSPCODEC_COUNTRY_JP) // Japan
	{
#if defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3210 ) || defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215 )
		LEC_NLP_Config(2);
#elif defined (CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226) || defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3217x ) || defined (CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226x)
		LEC_NLP_Config(2);
#elif defined CONFIG_RTK_VOIP_DRIVERS_SLIC_W682388
		#error "Need to do NTT echo test for SLIC W682388"
#endif
	}
	else
	{
#if defined (CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226) || defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3217x ) || defined (CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226x)
		LEC_NLP_Config(5);
#else
		LEC_NLP_Config(6);
#endif
	}
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_COUNTRY( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	if ( DSPCODEC_COUNTRY_CUSTOME != stVoipValue.value)
	{
		uint32 chid;

		RtkAc49xApiSetCountryTone(stVoipValue.value);
		for (chid=0; chid<CON_CH_NUM; chid++) {
			if( get_snd_type_cch( chid ) != SND_TYPE_FXS )
				continue;

			SLIC_Set_Impendance_Country(chid, stVoipValue.value /*country*/, 0 /* impedance value: reserved */);
		}
	}
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_FXS
 * @brief Set the impedance of country
 * @param TstVoipValue.value Country
 *        - 0: USA
 *        - 1: UK
 *        - 2: Australia
 *        - 3: HK
 *        - 4: Japan
 *        - 5: Sweden
 *        - 6: Germany
 *        - 7: France
 *        - 8: Taiwan
 *        - 9: Belgium
 *        - 10: Finland
 *        - 11: Italy
 *        - 12: China
 *        - 13: Extend #1
 *        - 14: Extend #2
 *        - 15: Extend #3
 *        - 16: Extend #4
 *        - 17: Customer
 * @see VOIP_MGR_SET_COUNTRY_IMPEDANCE TstVoipValue
 */
int do_mgr_VOIP_MGR_SET_COUNTRY_IMPEDANCE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_FULLY_OFFLOAD
	uint32 chid;
#endif

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_FULLY_OFFLOAD
	// Host auto forward, and run this body
#else
	//for (chid=0; chid<SLIC_CH_NUM; chid++)
	for (chid=0; chid<CON_CH_NUM; chid++) {
		if( get_snd_type_cch( chid ) != SND_TYPE_FXS )
			continue;

		SLIC_Set_Impendance_Country(chid, stVoipValue.value /*country*/, 0 /* impedance value: reserved */);
	}

#endif
	return 0;
}


/**
 * @ingroup VOIP_DSP_TONE
 * @brief Set tone of country
 * @param TstVoipValue.value Country
 *        - 0: USA
 *        - 1: UK
 *        - 2: Australia
 *        - 3: HK
 *        - 4: Japan
 *        - 5: Sweden
 *        - 6: Germany
 *        - 7: France
 *        - 8: Taiwan
 *        - 9: Belgium
 *        - 10: Finland
 *        - 11: Italy
 *        - 12: China
 *        - 13: Extend #1
 *        - 14: Extend #2
 *        - 15: Extend #3
 *        - 16: Extend #4
 *        - 17: Customer
 * @see VOIP_MGR_SET_COUNTRY_TONE TstVoipValue
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_COUNTRY_TONE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	uint32 sid;
#endif

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	for(sid=0; sid<DSP_RTK_SS_NUM; sid++)	//Set the same country for each session.
		DspcodecSetCountry( sid, /*country*/stVoipValue.value);

	switch (stVoipValue.value)
	{
		case DSPCODEC_COUNTRY_USA://USA
			PRINT_MSG("Set Tone of Country to USA\n");
			break;
		case DSPCODEC_COUNTRY_UK://UK
			PRINT_MSG("Set Tone of Country to UK\n");
			break;
		case DSPCODEC_COUNTRY_AUSTRALIA://Australia
			PRINT_MSG("Set Tone of Country to AUSTRALIA\n");
			break;
		case DSPCODEC_COUNTRY_HK://HK
			PRINT_MSG("Set Tone of Country to HONG KONG\n");
			break;
		case DSPCODEC_COUNTRY_JP://Japan
			PRINT_MSG("Set Tone of Country to JAPAN\n");
			break;
		case DSPCODEC_COUNTRY_SE://Sweden
			PRINT_MSG("Set Tone of Country to SWEDEN\n");
			break;
		case DSPCODEC_COUNTRY_GR://Germany
			PRINT_MSG("Set Tone of Country to GERMANY\n");
			break;
		case DSPCODEC_COUNTRY_FR://France
			PRINT_MSG("Set Tone of Country to FRANCE\n");
			break;
		case DSPCODEC_COUNTRY_TW://Taiwan
		//case DSPCODEC_COUNTRY_TR://TR57
			//PRINT_MSG("Set Tone of Country to TR57\n");
			PRINT_MSG("Set Tone of Country to TAIWAN\n");
			break;
		case DSPCODEC_COUNTRY_BE://Belgium
			PRINT_MSG("Set Tone of Country to BELGIUM\n");
			break;
		case DSPCODEC_COUNTRY_FL://Finland
			PRINT_MSG("Set Tone of Country to FINLAND\n");
			break;
		case DSPCODEC_COUNTRY_IT://Italy
			PRINT_MSG("Set Tone of Country to ITALY\n");
			break;
		case DSPCODEC_COUNTRY_CN://China
			PRINT_MSG("Set Tone of Country to CHINA\n");
			break;
		case DSPCODEC_COUNTRY_EX1://extend #1
			PRINT_MSG("Set Tone of Country to extend #1\n");
			break;
		case DSPCODEC_COUNTRY_EX2://extend #2
			PRINT_MSG("Set Tone of Country to extend #2\n");
			break;
		case DSPCODEC_COUNTRY_EX3://extend #3
			PRINT_MSG("Set Tone of Country to extend #3\n");
			break;
		case DSPCODEC_COUNTRY_EX4://extend #4
			PRINT_MSG("Set Tone of Country to extend #4\n");
			break;
#ifdef COUNTRY_TONE_RESERVED
		case DSPCODEC_COUNTRY_RESERVE://Reserve
			PRINT_MSG("Set Tone of Country to Reserve\n");
			break;
#endif
		case DSPCODEC_COUNTRY_CUSTOME://Customer
			PRINT_MSG("Set Tone of Country to CUSTOMER\n");
			break;
		default:
			PRINT_MSG("The tone you select is not support!\n");
			break;
	}

	if (stVoipValue.value == 4) // Japan
	{
#if defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3210 ) || defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215 )
		LEC_NLP_Config(2);
#elif defined (CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226) || defined (CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226x)
		LEC_NLP_Config(2);
#elif defined CONFIG_RTK_VOIP_DRIVERS_SLIC_W682388
		#error "Need to do NTT echo test for SLIC W682388"
#endif
	}
	else
	{
#if defined (CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226) || defined (CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226x)
		LEC_NLP_Config(5);
#else
		LEC_NLP_Config(6);
#endif
	}
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_COUNTRY_TONE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	if ( DSPCODEC_COUNTRY_CUSTOME != stVoipValue.value)
	{
		RtkAc49xApiSetCountryTone(stVoipValue.value);
	}
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_TONE
 * @brief Set customize tone
 * @param TstVoipValue.value Use n-th customer tone
 * @see VOIP_MGR_SET_TONE_OF_CUSTOMIZE TstVoipValue
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_TONE_OF_CUSTOMIZE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	cust = stVoipValue.value;
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_TONE_OF_CUSTOMIZE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	tone_idx = stVoipValue.value;
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_TONE
 * @brief Set customize tone parameters
 * @see VOIP_MGR_SET_CUST_TONE_PARAM TstVoipValue
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_CUST_TONE_PARAM( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipToneCfg stVoipToneCfg;
	int ret;

	COPY_FROM_USER(&stVoipToneCfg, (TstVoipToneCfg *)user, sizeof(TstVoipToneCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else

#if 0
	/*
	Because AudioCodes doesn't support play tone with fixd cycle, RTK could.
	To be identical, when RTK get cycle=2,
	change it to 0(continuous play tone with cadence)
	*/
	if (stVoipToneCfg.cycle == 0)	//Continuous
	{
		//set CadOn0 to non-zero value to ensure play continuous tone
		stVoipToneCfg.CadOn0 = 100;
		// set other CadOn/Off to zero to ensure not enter cadence tone
		stVoipToneCfg.CadOff0 = 0;
		stVoipToneCfg.CadOn1 = 0;
		stVoipToneCfg.CadOff1 = 0;
		stVoipToneCfg.CadOn2 = 0;
		stVoipToneCfg.CadOff2 = 0;
		stVoipToneCfg.CadOn3 = 0;
		stVoipToneCfg.CadOff3 = 0;
	}
	else if (stVoipToneCfg.cycle == 2)	//Cadence
		stVoipToneCfg.cycle = 0;	//Continuous

	//web setting unit is (-dBm)
	stVoipToneCfg.Gain1 = (-1)*stVoipToneCfg.Gain1;

	// if tone type is succeed, use the Gain1 value as the gain of other frequency.
	if (stVoipToneCfg.toneType == 2)//SUCC
	{
		stVoipToneCfg.Gain2 = stVoipToneCfg.Gain1;
		stVoipToneCfg.Gain3 = stVoipToneCfg.Gain1;
		stVoipToneCfg.Gain4 = stVoipToneCfg.Gain1;
	}
	else
	{
		stVoipToneCfg.Gain2 = (-1)*stVoipToneCfg.Gain2;
		stVoipToneCfg.Gain3 = (-1)*stVoipToneCfg.Gain3;
		stVoipToneCfg.Gain4 = (-1)*stVoipToneCfg.Gain4;
	}
#else
	//web setting unit is (-dBm)
	stVoipToneCfg.Gain1 = (-1)*stVoipToneCfg.Gain1;
	stVoipToneCfg.Gain2 = (-1)*stVoipToneCfg.Gain2;
	stVoipToneCfg.Gain3 = (-1)*stVoipToneCfg.Gain3;
	stVoipToneCfg.Gain4 = (-1)*stVoipToneCfg.Gain4;

#endif
	setTone( ( aspToneCfgParam_t * )&stVoipToneCfg);
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_CUST_TONE_PARAM( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipToneCfg stVoipToneCfg;
	int ret;

	COPY_FROM_USER(&stVoipToneCfg, (TstVoipToneCfg *)user, sizeof(TstVoipToneCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	SaveCustomTone(&stVoipToneCfg);
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_TONE
 * @brief Use customize tone
 * @param TstVoipValue.value1 Customer dial tone
 * @param TstVoipValue.value2 Customer ringing tone
 * @param TstVoipValue.value3 Customer busy tone
 * @param TstVoipValue.value4 Customer call waiting tone
 * @see VOIP_MGR_USE_CUST_TONE TstVoipValue
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_USE_CUST_TONE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	extern RESULT DspcodecSetCustomTone(DSPCODEC_TONE nTone, ToneCfgParam_t *pToneCfg);
	short *pToneTable;
#endif

	TstVoipValue stVoipValue;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else

	pToneTable = (short*)&ToneTable[CUSTOM_TONE1+stVoipValue.value1];	//dial
	DspcodecSetCustomTone(DSPCODEC_TONE_DIAL, (ToneCfgParam_t *)pToneTable);

	pToneTable = (short*)&ToneTable[CUSTOM_TONE1+stVoipValue.value2];	//ring
	DspcodecSetCustomTone(DSPCODEC_TONE_RINGING, (ToneCfgParam_t *)pToneTable);

	pToneTable = (short*)&ToneTable[CUSTOM_TONE1+stVoipValue.value3];	//busy
	DspcodecSetCustomTone(DSPCODEC_TONE_BUSY, (ToneCfgParam_t *)pToneTable);

	pToneTable = (short*)&ToneTable[CUSTOM_TONE1+stVoipValue.value4];	//waiting
	DspcodecSetCustomTone(DSPCODEC_TONE_CALL_WAITING, (ToneCfgParam_t *)pToneTable);
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_USE_CUST_TONE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	extern int RtkAc49xApiSetCustomTone(TstVoipValue * custom_tone);
	TstVoipValue stVoipValue;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else

	RtkAc49xApiSetCustomTone(&stVoipValue);
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_TONE
 * @brief Enable disconnection tone detection
 * @see VOIP_MGR_SET_DIS_TONE_DET TstVoipdistonedet_parm
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_DIS_TONE_DET( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef FXO_BUSY_TONE_DET
	TstVoipdistonedet_parm stVoipdistonedet_parm_mgr;
	int ret;

	COPY_FROM_USER(&stVoipdistonedet_parm_mgr, (TstVoipdistonedet_parm *)user, sizeof(TstVoipdistonedet_parm));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	memcpy(&stVoipdistonedet_parm, &stVoipdistonedet_parm_mgr, sizeof(TstVoipdistonedet_parm));

	dis_tone_det_cfg_apply();
#endif
#else
	return NO_COPY_TO_USER( cmd, seq_no );
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_DIS_TONE_DET( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	// no handler
	return NO_COPY_TO_USER( cmd, seq_no );
	//return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_AGC
 * @brief Enable / disable speaker AGC
 * @param TstVoipValue.value Enable (1) or disable (0) speaker AGC
 * @see VOIP_MGR_SET_SPK_AGC TstVoipValue
 * @param TstVoipValue.value1 AGC adaptive threshold Available range 0 ot 70 (0 ~ -70dB) unit: dB, default 55
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_SPK_AGC( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	spk_agc_mode[stVoipValue.ch_id]=stVoipValue.value;
	spk_agc_adaptive_threshold[stVoipValue.ch_id]=91-stVoipValue.value1;
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_SPK_AGC( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	spk_agc_mode[stVoipValue.ch_id]=stVoipValue.value;
	agc_enable[stVoipValue.ch_id] =  spk_agc_mode[stVoipValue.ch_id];
	RtkAc49xApiAgcConfig(stVoipValue.ch_id, agc_enable[stVoipValue.ch_id], AGC_LOCATION__AT_DECODER_OUTPUT);
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_AGC
 * @brief Set speaker AGC level
 * @param TstVoipValue.value Speaker AGC level
 * @see VOIP_MGR_SET_SPK_AGC_LVL TstVoipValue
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_SPK_AGC_LVL( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	spk_agc_lvl[stVoipValue.ch_id]=stVoipValue.value;
#endif
	//PRINT_MSG("spk_agc_lvl[%d]=%d\n", stVoipValue.ch_id, stVoipValue.value);
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_SPK_AGC_LVL( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	if (spk_agc_mode[stVoipValue.ch_id]==1)// SPK AGC is enable
	{
		Tac49xAgcTargetEnergy tar_eng;
		/* stVoipValue.value range: 0(small:-25dBm) to 8(large:-1dBm), space: 3dBm */
		tar_eng = AGC_TARGET_ENERGY__minus25_DBM - 3*stVoipValue.value;
		RtkAc49xApiAgcEnergySlope(stVoipValue.ch_id, tar_eng, AGC_GAIN_SLOPE__1_00_DB_SEC);
	}
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_AGC
 * @brief Set speaker AGC gain up
 * @param TstVoipValue.value Speaker AGC gain up value
 * @see VOIP_MGR_SET_SPK_AGC_GUP TstVoipValue
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_SPK_AGC_GUP( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	spk_agc_gup[stVoipValue.ch_id]=stVoipValue.value;
#endif
	//PRINT_MSG("spk_agc_gup[%d]=%d\n", stVoipValue.ch_id, stVoipValue.value);
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_SPK_AGC_GUP( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	spk_agc_gup = stVoipValue.value;
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_AGC
 * @brief Set speaker AGC gain down
 * @param TstVoipValue.value Speaker AGC gain down value
 * @see VOIP_MGR_SET_SPK_AGC_GDOWN TstVoipValue
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_SPK_AGC_GDOWN( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	spk_agc_gdown[stVoipValue.ch_id]=stVoipValue.value;
#endif
	//PRINT_MSG("spk_agc_gdown[%d]=%d\n", stVoipValue.ch_id, stVoipValue.value);
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_SPK_AGC_GDOWN( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	spk_agc_gdown = stVoipValue.value;
	//Note: IO ctrl VOIP_MGR_SET_SPK_AGC_GUP should be called first.
	if (spk_agc_mode[stVoipValue.ch_id] == 1)
		RtkAc49xApiAgcDeviceConfig(spk_agc_gdown+1, spk_agc_gup+1);
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_AGC
 * @brief Enable / disable MIC AGC
 * @param TstVoipValue.value Enable (1) or disable (0) MIC AGC
 * @see VOIP_MGR_SET_MIC_AGC TstVoipValue
 * @param TstVoipValue.value1 AGC adaptive threshold Available range 0 ot 70 (0 ~ -70dB) unit: dB, default 55
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_MIC_AGC( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	mic_agc_mode[stVoipValue.ch_id]=stVoipValue.value;
	mic_agc_adaptive_threshold[stVoipValue.ch_id]=91-stVoipValue.value1;
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_MIC_AGC( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	mic_agc_mode[stVoipValue.ch_id]=stVoipValue.value;
	agc_enable[stVoipValue.ch_id] = agc_enable[stVoipValue.ch_id] | mic_agc_mode[stVoipValue.ch_id];
	if (mic_agc_mode[stVoipValue.ch_id] == 1)
		RtkAc49xApiAgcConfig(stVoipValue.ch_id, agc_enable[stVoipValue.ch_id], AGC_LOCATION__AT_ENCODER_INPUT);
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_AGC
 * @brief Set MIC AGC level
 * @param TstVoipValue.value MIC AGC level
 * @see VOIP_MGR_SET_MIC_AGC_LVL TstVoipValue
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_MIC_AGC_LVL( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	mic_agc_lvl[stVoipValue.ch_id]=stVoipValue.value;
#endif
	//PRINT_MSG("mic_agc_lvl[%d]=%d\n", stVoipValue.ch_id, stVoipValue.value);
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_MIC_AGC_LVL( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	if (mic_agc_mode[stVoipValue.ch_id]==1)// MIC AGC is enable
	{
		Tac49xAgcTargetEnergy tar_eng;
		/* stVoipValue.value range: 0(small:-25dBm) to 8(large:-1dBm), space: 3dBm */
		tar_eng = AGC_TARGET_ENERGY__minus25_DBM - 3*stVoipValue.value;
		RtkAc49xApiAgcEnergySlope(stVoipValue.ch_id, tar_eng, AGC_GAIN_SLOPE__1_00_DB_SEC);
	}
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_AGC
 * @brief Set MIC AGC gain up
 * @param TstVoipValue.value MIC AGC up value
 * @see VOIP_MGR_SET_MIC_AGC_GUP TstVoipValue
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_MIC_AGC_GUP( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	mic_agc_gup[stVoipValue.ch_id]=stVoipValue.value;
#endif
	//PRINT_MSG("mic_agc_gup[%d]=%d\n", stVoipValue.ch_id, stVoipValue.value);
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_MIC_AGC_GUP( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	mic_agc_gup = stVoipValue.value;
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_AGC
 * @brief Set MIC AGC gain down
 * @param TstVoipValue.value MIC AGC down value
 * @see VOIP_MGR_SET_MIC_AGC_GDOWN TstVoipValue
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_SET_MIC_AGC_GDOWN( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	mic_agc_gdown[stVoipValue.ch_id]=stVoipValue.value;
#endif
	//PRINT_MSG("mic_agc_gdown[%d]=%d\n", stVoipValue.ch_id, stVoipValue.value);
	return 0;
}
#else
int do_mgr_VOIP_MGR_SET_MIC_AGC_GDOWN( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipValue stVoipValue;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	mic_agc_gdown = stVoipValue.value;
	//Note: IO ctrl VOIP_MGR_SET_MIC_AGC_GUP should be called first.
	if (mic_agc_mode[stVoipValue.ch_id] == 1)
		RtkAc49xApiAgcDeviceConfig(mic_agc_gdown+1, mic_agc_gup+1);
#endif
	return 0;
}
#endif

#if 0
// TODO Tempory put it here. Should be memoved to somewhere
typedef struct {
    unsigned int pause_time;    // unit in ms
    unsigned int break_min_ths; // unit in ms
    unsigned int break_max_ths; // unit in ms
}
stPulseDetParam;
#endif

/**
 * @ingroup VOIP_DSP_PLUSEDIAL
 * @brief Enable / disable pluse digits detection
 * @param TstVoipCfg.ch_id Channel ID
 * @param TstVoipCfg.enable Enable (1) or disable (0) pluse digits detection
 * @param TstVoipCfg.cfg Pause time (ms)
 * @see VOIP_MGR_SET_PULSE_DIGIT_DET TstVoipCfg
 */
int do_mgr_VOIP_MGR_SET_PULSE_DIGIT_DET( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef PULSE_DIAL_DET
	TstVoipCfg stVoipCfg;
	int ret;

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	stPulseDetParam param;
#endif

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	param.pause_time = stVoipCfg.cfg;
	param.break_min_ths = stVoipCfg.cfg2;
	param.break_max_ths = stVoipCfg.cfg3;
	set_pulse_det(stVoipCfg.ch_id, stVoipCfg.enable, &param); /* 0: disable 1: enable Pulse Digit Detection */
#endif
#else
	return NO_COPY_TO_USER( cmd, seq_no );
#endif
	return 0;
}

/**
 * @ingroup VOIP_DSP_PLUSEDIAL
 * @brief Turn on / off pluse dial
 * @param TstVoipCfg.ch_id Channel ID
 * @param TstVoipCfg.enable Enable (1) or disable (0) pluse digits detection
 * @see VOIP_MGR_SET_DIAL_MODE TstVoipCfg
 */
int do_mgr_VOIP_MGR_SET_DIAL_MODE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
	TstVoipCfg stVoipCfg;
	int ret;

	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef PULSE_DIAL_GEN
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	DAA_Set_PulseDial_cch(stVoipCfg.ch_id, stVoipCfg.cfg /* 0: disable 1: enable Pulse dial */);
#endif
#endif
#else
	return NO_COPY_TO_USER( cmd, seq_no );
#endif
	return 0;
}

/**
 * @ingroup VOIP_DSP_PLUSEDIAL
 * @brief Check whether dial mode is pluse dial
 * @param TstVoipCfg.ch_id Channel ID
 * @param [out] TstVoipCfg.cfg Dial mode (0: disable 1: enable Pulse dial)
 * @see VOIP_MGR_GET_DIAL_MODE TstVoipCfg
 */
int do_mgr_VOIP_MGR_GET_DIAL_MODE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipCfg stVoipCfg;

#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
#ifdef PULSE_DIAL_GEN
	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	stVoipCfg.cfg = DAA_Get_Dial_Mode(stVoipCfg.ch_id); /* 0: disable 1: enable Pulse dial */
#endif
	return COPY_TO_USER(user, &stVoipCfg, sizeof(TstVoipCfg), cmd, seq_no);
#endif
#else
	COPY_FROM_USER(&stVoipCfg, (TstVoipCfg *)user, sizeof(TstVoipCfg));
	stVoipCfg.cfg = 0;
	return COPY_TO_USER(user, &stVoipCfg, sizeof(TstVoipCfg), cmd, seq_no);
#endif
	return 0;
}

/**
 * @ingroup VOIP_DSP_PLUSEDIAL
 * @brief Configure plus dial generation
 * @param TstVoipValue.value PPS
 * @param TstVoipValue.value5 Make duration (ms)
 * @param TstVoipValue.value6 Pause duration (ms)
 * @see VOIP_MGR_PULSE_DIAL_GEN_CFG TstVoipValue
 */
int do_mgr_VOIP_MGR_PULSE_DIAL_GEN_CFG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
#ifdef PULSE_DIAL_GEN
	TstVoipValue stVoipValue;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	DAA_PulseDial_Gen_Cfg(stVoipValue.value/*pps*/, stVoipValue.value5/*make duration*/, stVoipValue.value6/*pause duration*/);
#endif
#endif
#else
	return NO_COPY_TO_USER( cmd, seq_no );
#endif
	return 0;
}

/**
 * @ingroup VOIP_DSP_PLUSEDIAL
 * @brief Generate pluse dial
 * @param TstVoipValue.ch_id Channel ID
 * @param TstVoipValue.value Digit
 * @see VOIP_MGR_GEN_PULSE_DIAL TstVoipValue
 */
int do_mgr_VOIP_MGR_GEN_PULSE_DIAL( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
#ifdef PULSE_DIAL_GEN
	TstVoipValue stVoipValue;
	int ret;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	pulse_dial_in_cch(stVoipValue.ch_id, stVoipValue.value/*digit*/);
#endif
#endif
#else
	return NO_COPY_TO_USER( cmd, seq_no );
#endif
	return 0;
}

/**
 * @ingroup VOIP_DSP_IVR
 * @brief Play textual, G.711, G.729 and G.723 IVR
 * @note This function can play four kinds of IVR, and echo of them
 *       use its structure.
 * @see VOIP_MGR_PLAY_IVR TstVoipPlayIVR_Header TstVoipPlayIVR_G711 TstVoipPlayIVR_G729 TstVoipPlayIVR_G72363 TstVoipPlayIVR_Text
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_PLAY_IVR( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipPlayIVR_Header stVoipPlayIVR_Header;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	COPY_FROM_USER(&stVoipPlayIVR_Header, (TstVoipPlayIVR_Header *)user, sizeof(TstVoipPlayIVR_Header));
	PlayIvrDispatcher( &stVoipPlayIVR_Header, user );
#endif
	COPY_TO_USER(user, &stVoipPlayIVR_Header, sizeof(TstVoipPlayIVR_Header), cmd, seq_no);

	return 0;
}
#else
int do_mgr_VOIP_MGR_PLAY_IVR( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	extern void SetTextPlayBuf(unsigned int chid, unsigned char* pText);
	TstVoipPlayIVR_Header stVoipPlayIVR_Header;

#ifndef ACMW_PLAYBACK
	COPY_FROM_USER(&stVoipPlayIVR_Header, (TstVoipPlayIVR_Header *)user, sizeof(TstVoipPlayIVR_Header));
	PlayIvrDispatcher( &stVoipPlayIVR_Header, user );
	return COPY_TO_USER(user, &stVoipPlayIVR_Header, sizeof(TstVoipPlayIVR_Header), cmd, seq_no);
#else
	{
		union
		{
			TstVoipPlayIVR_Text	stVoipPlayIVR_Text;
			TstVoipPlayIVR_G72363	stVoipPlayIVR_G72363;
			TstVoipPlayIVR_G729	stVoipPlayIVR_G729;
			TstVoipPlayIVR_G711	stVoipPlayIVR_G711;
		} save;

		TstVoipPlayIVR_Header * const pHeader = &stVoipPlayIVR_Header;
		TstVoipPlayIVR_G72363 * const pHeaderG723 = ( TstVoipPlayIVR_G72363 * )&stVoipPlayIVR_Header;
		TstVoipPlayIVR_G729 * const pHeaderG729 = ( TstVoipPlayIVR_G729 * )&stVoipPlayIVR_Header;
		TstVoipPlayIVR_G711 * const pHeaderG711 = ( TstVoipPlayIVR_G711 * )&stVoipPlayIVR_Header;
		COPY_FROM_USER(pHeader, (TstVoipPlayIVR_Header *)user, sizeof(TstVoipPlayIVR_Header));

		switch( pHeader ->type )
		{
			case IVR_PLAY_TYPE_G723_63:
			{
				COPY_FROM_USER(&save.stVoipPlayIVR_G72363, (TstVoipPlayIVR_G72363 *)user, sizeof(TstVoipPlayIVR_G72363));
				unsigned int ch = save.stVoipPlayIVR_G72363.ch_id;
				extern int play_g723h_flag[];

				pHeaderG723->nRetCopiedFrames = IvrPlayBufWrite(ch, (char*)save.stVoipPlayIVR_G72363.data, save.stVoipPlayIVR_G72363.nFramesCount, G723_FRAME_SIZE);
				//PRINT_R("[%d]\n", pHeaderG723->nRetCopiedFrames);// The length which write to play buf

				if (play_g723h_flag[ch] == 0)
				{
					play_g723h_flag[ch] = 1;
					RtkAc49xApiPlayIvrTdmStart(ch, CODER__G723HIGH);
					PRINT_MSG("Play G723-6.3K File Start!\n");
				}

				break;
			}

			case IVR_PLAY_TYPE_G729:
			{
				COPY_FROM_USER(&save.stVoipPlayIVR_G729, (TstVoipPlayIVR_G729 *)user, sizeof(TstVoipPlayIVR_G729));
				unsigned int ch = save.stVoipPlayIVR_G729.ch_id;
				extern int play_g729_flag[];

				pHeaderG729->nRetCopiedFrames = IvrPlayBufWrite(ch, (char*)save.stVoipPlayIVR_G729.data, save.stVoipPlayIVR_G729.nFramesCount, G729_FRAME_SIZE);
				//PRINT_R("[%d]\n", pHeaderG729->nRetCopiedFrames);// The length which write to play buf

				if (play_g729_flag[ch] == 0)
				{
					play_g729_flag[ch] = 1;
					RtkAc49xApiPlayIvrTdmStart(ch, CODER__G729);
					PRINT_MSG("Play G729 File Start!\n");
				}

				break;
			}

			case IVR_PLAY_TYPE_G711A:
			{
				COPY_FROM_USER(&save.stVoipPlayIVR_G711, (TstVoipPlayIVR_G711 *)user, sizeof(TstVoipPlayIVR_G711));
				unsigned int ch = save.stVoipPlayIVR_G711.ch_id;
				extern int play_g711_flag[];

				pHeaderG711->nRetCopiedFrames = IvrPlayBufWrite(ch, (char*)save.stVoipPlayIVR_G711.data, save.stVoipPlayIVR_G711.nFramesCount, G711_FRAME_SIZE);
				//PRINT_R("[%d]\n", pHeaderG711->nRetCopiedFrames);// The length which write to play buf

				if (play_g711_flag[ch] == 0)
				{
					play_g711_flag[ch] = 1;
					RtkAc49xApiPlayIvrTdmStart(ch, CODER__G711ALAW);
					PRINT_MSG("Play G711 File Start!\n");
				}

				break;
			}

			case IVR_PLAY_TYPE_TEXT:
			{
				COPY_FROM_USER(&save.stVoipPlayIVR_Text, (TstVoipPlayIVR_Text *)user, sizeof(TstVoipPlayIVR_Text));
				SetTextPlayBuf(save.stVoipPlayIVR_Text.ch_id, save.stVoipPlayIVR_Text.szText2speech);
				break;
			}

			default:
				break;
		}

		return COPY_TO_USER(user, &stVoipPlayIVR_Header, sizeof(TstVoipPlayIVR_Header), cmd, seq_no);
	}
#endif
	return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_IVR
 * @brief Check if IVR is still playing
 * @see VOIP_MGR_POLL_IVR TstVoipPollIVR
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_POLL_IVR( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipPollIVR stVoipPollIVR;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else

	COPY_FROM_USER(&stVoipPollIVR, (TstVoipPollIVR *)user, sizeof(TstVoipPollIVR));
	stVoipPollIVR.bPlaying =
			PollIvrPlaying( stVoipPollIVR.ch_id );
#endif
	return COPY_TO_USER(user, &stVoipPollIVR, sizeof(TstVoipPollIVR), cmd, seq_no);

	//return 0;
}
#else
int do_mgr_VOIP_MGR_POLL_IVR( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipPollIVR stVoipPollIVR;

	COPY_FROM_USER(&stVoipPollIVR, (TstVoipPollIVR *)user, sizeof(TstVoipPollIVR));
#ifndef ACMW_PLAYBACK
	stVoipPollIVR.bPlaying =
		PollIvrPlaying( stVoipPollIVR.ch_id );
#else
	/*
		#define PB_END	0
		#define PB_TDM	1
		#define PB_NET	2
		#define PB_SIL	3
	*/

	if ( RtkAc49xApiPollIvr(stVoipPollIVR.ch_id) == 0 )
		stVoipPollIVR.bPlaying = 0;
	else
		stVoipPollIVR.bPlaying = 1;
#endif
	return COPY_TO_USER(user, &stVoipPollIVR, sizeof(TstVoipPollIVR), cmd, seq_no);
	//return 0;
}
#endif

/**
 * @ingroup VOIP_DSP_IVR
 * @brief Stop IVR playing
 * @see VOIP_MGR_STOP_IVR TstVoipStopIVR
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_STOP_IVR( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipStopIVR stVoipStopIVR;
	int ret;

	COPY_FROM_USER(&stVoipStopIVR, (TstVoipStopIVR *)user, sizeof(TstVoipStopIVR));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward
#else
	StopIvrPlaying( stVoipStopIVR.ch_id );
#endif
	return 0;
}
#else
int do_mgr_VOIP_MGR_STOP_IVR( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	TstVoipStopIVR stVoipStopIVR;
	int ret;

	COPY_FROM_USER(&stVoipStopIVR, (TstVoipStopIVR *)user, sizeof(TstVoipStopIVR));

	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifndef ACMW_PLAYBACK
	StopIvrPlaying( stVoipStopIVR.ch_id );
#else
	RtkAc49xApiPlayIvrEnd(stVoipStopIVR.ch_id);
#endif
	return 0;
}
#endif


