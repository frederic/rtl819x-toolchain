/* Profile utilities based on COP3 */
/* Modiied from rtl8651 ROME driver */

#include "cp3_profile.h"
#include "voip_types.h"
#include "voip_init.h"

//#define VOIP_ENABLE_GCC_INSTRUMENT_MEASUER	1
/* must define if enable gcc '-finstrument-functions' to profiling g.729 encode */


#ifdef FEATURE_COP3_PROFILE

enum CP3_COUNTER
{
	CP3CNT_CYCLES = 0,
	CP3CNT_NEW_INST_FECTH,
	CP3CNT_NEW_INST_FETCH_CACHE_MISS,
	CP3CNT_NEW_INST_MISS_BUSY_CYCLE,
	CP3CNT_DATA_STORE_INST,
	CP3CNT_DATA_LOAD_INST,
	CP3CNT_DATA_LOAD_OR_STORE_INST,
	CP3CNT_EXACT_RETIRED_INST,
	CP3CNT_RETIRED_INST_FOR_PIPE_A,
	CP3CNT_RETIRED_INST_FOR_PIPE_B,
	CP3CNT_DATA_LOAD_OR_STORE_CACHE_MISS,
	CP3CNT_DATA_LOAD_OR_STORE_MISS_BUSY_CYCLE,
	CP3CNT_RESERVED12,
	CP3CNT_RESERVED13,
	CP3CNT_RESERVED14,
	CP3CNT_RESERVED15,
};

/* Local variables */
static volatile uint64 tempVariable64;
static uint32 tempVariable32;
static uint64 currCnt[4];

/* Global variables */
//uint64 cnt1, cnt2;
profile_stat_t ProfileStat[PROFILE_INDEX_MAX];
profile_stat_t TempProfStat[PROFILE_INDEX_MAX];

static uint32 profile_inited = 0;
static uint32 profile_enable = TRUE;

#define cp3_printf printk

void flush_IDCache(void){

	__asm__ __volatile__ (

		"mfc0 $9,$20\n\t"
		"nop\n\t"
		"la $10,0xFFFFFFFC\n\t" 
		"and $9,$10\n\t"
		"mtc0 $9,$20\n\t"
		"nop\n\t"
		"nop\n\t"

		"mfc0 $9,$20\n\t"
		"nop\n\t"
		"la $10,0x0000000F\n\t"
		"or $9,$10\n\t"
		"mtc0 $9,$20\n\t"
		"nop\n\t"
		"nop\n\t"
		);
}

static void CP3_COUNTER0_INIT( void )
{
__asm__ __volatile__ \
("  ;\
	mfc0	$8, $12			;\
	la		$9, 0x80000000	;\
	or		$8, $9			;\
	mtc0	$8, $12			;\
");
}

#if 0 
static uint32 CP3_COUNTER0_IS_INITED( void )
{
__asm__ __volatile__ \
("  ;\
	mfc0	$8, $12			;\
	la		$9, tempVariable32;\
	sw		$8, 0($9)		;\
");
	return tempVariable32;
}
#endif

static void CP3_COUNTER0_START( void )
{
#ifndef CONFIG_VOIP_COP3_PROFILE
	tempVariable32 = /* Counter0 */((0x10|CP3CNT_CYCLES)<< 0) |
	                 /* Counter1 */((0x10|CP3CNT_NEW_INST_FECTH)<< 8) |
	                 /* Counter2 */((0x10|CP3CNT_DATA_LOAD_OR_STORE_MISS_BUSY_CYCLE)<<16) |
	                 /* Counter3 */((0x10|CP3CNT_NEW_INST_MISS_BUSY_CYCLE)<<24);
#else
	extern unsigned int gCp3Params;
	tempVariable32 = gCp3Params;
#endif
	                 
__asm__ __volatile__ \
("  ;\
	la		$8, tempVariable32	;\
	lw		$8, 0($8)			;\
	nop					;\
	ctc3 	$8, $0				;\
");
}

static void CP3_COUNTER0_STOP( void )
{
__asm__ __volatile__ \
("	;\
	ctc3 	$0, $0			;\
");
}

void CP3_COUNTER0_PASUE( void )
{
#if 0 /* 5181 or 5281 */
__asm__ __volatile__ \
("	;\
	mfc0	$14, $12			;\
	la		$15, 0x80000000	;\
	or		$14, $15			;\
	mtc0	$14, $12			;\
	li 	$14, 0x0f0f0f0f		;\
	cfc3 	$15, $0			;\
	nop				;\
	nop				;\
	and	$15, $14		;\
	ctc3 	$15, $0			;\
": /* no outputs */ : /* no inputs */ :"$14", "$15");
#else
__asm__ __volatile__ \
("	;\
	mfc0	$14, $12			;\
	la		$15, 0x80000000	;\
	or		$14, $15			;\
	mtc0	$14, $12			;\
	li 	$14, 0x80808080		;\
	cfc3 	$15, $0			;\
	nop				;\
	nop				;\
	or	$15, $14		;\
	ctc3 	$15, $0			;\
": /* no outputs */ : /* no inputs */ :"$14", "$15");
#endif
}

void CP3_COUNTER0_RESUME( void )
{
#if 0 /* 5181 or 5281 */
__asm__ __volatile__ \
("	;\
	li 	$14, 0x10101010		;\
	cfc3 	$15, $0			;\
	nop				;\
	nop				;\
	or	$15, $14		;\
	ctc3 	$15, $0			;\
": /* no outputs */ : /* no inputs */ :"$14", "$15");
#else
__asm__ __volatile__ \
("	;\
	li 	$14, 0x7f7f7f7f		;\
	cfc3 	$15, $0			;\
	nop				;\
	nop				;\
	and	$15, $14		;\
	ctc3 	$15, $0			;\
": /* no outputs */ : /* no inputs */ :"$14", "$15");
#endif
}

static uint64 CP3_COUNTER0_GET( void )
{
__asm__ __volatile__ \
("	;\
	la		$8, tempVariable64;\
	mfc3	$9, $9			;\
	sw		$9, 0($8)		;\
	mfc3	$9, $8			;\
	sw		$9, 4($8)		;\
");
	return tempVariable64;
}

static void CP3_COUNTER0_GET_ALL( void )
{
__asm__ __volatile__ \
("	;\
	la		$4, currCnt		;\
	mfc3	$9, $9			;\
	sw		$9, 0x00($4)	;\
	mfc3	$9, $8			;\
	sw		$9, 0x04($4)	;\
	mfc3	$9, $11			;\
	sw		$9, 0x08($4)	;\
	mfc3	$9, $10			;\
	sw		$9, 0x0C($4)	;\
	mfc3	$9, $13			;\
	sw		$9, 0x10($4)	;\
	mfc3	$9, $12			;\
	sw		$9, 0x14($4)	;\
	mfc3	$9, $15			;\
	sw		$9, 0x18($4)	;\
	mfc3	$9, $14			;\
	sw		$9, 0x1C($4)	;\
" : : : "$4");
}

int ProfileInit( void )
{
	CP3_COUNTER0_INIT();
#ifndef VOIP_ENABLE_GCC_INSTRUMENT_MEASUER
	CP3_COUNTER0_START();
#endif

	profile_inited = TRUE;
	profile_enable = TRUE;
	memset( (void*)&ProfileStat, 0, sizeof( ProfileStat ) );

	printk("ProfileInit\n");

	/* pcm865x.c */
	ProfileStat[PROFILE_INDEX_PCMISR].desc = "pcm_isr";
	ProfileStat[PROFILE_INDEX_PCMISR].valid = 1;
	ProfileStat[PROFILE_INDEX_PCM_RX].desc = "PCM_RX";
	ProfileStat[PROFILE_INDEX_PCM_RX].valid = 1;
	ProfileStat[PROFILE_INDEX_PCM_HANDLER].desc = "PCM_Handler";
	ProfileStat[PROFILE_INDEX_PCM_HANDLER].valid = 1;
	ProfileStat[PROFILE_INDEX_DTMFDEC].desc = "dtmfdec";
	ProfileStat[PROFILE_INDEX_DTMFDEC].valid = 1;
	ProfileStat[PROFILE_INDEX_LEC].desc = "G168_EchoCanceller";
	ProfileStat[PROFILE_INDEX_LEC].valid = 1;
	ProfileStat[PROFILE_INDEX_DSPPROCESS].desc = "DspProcess";
	ProfileStat[PROFILE_INDEX_DSPPROCESS].valid = 1;
	/* r1_main.c */
	ProfileStat[PROFILE_INDEX_PLAYTONE].desc = "playtone";
	ProfileStat[PROFILE_INDEX_PLAYTONE].valid = 1;
	ProfileStat[PROFILE_INDEX_G711DECPHASE].desc = "G711DecPhase_jbc";
	ProfileStat[PROFILE_INDEX_G711DECPHASE].valid = 1;
	ProfileStat[PROFILE_INDEX_G729DECPHASE].desc = "G729DecPhase_jbc";
	ProfileStat[PROFILE_INDEX_G729DECPHASE].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231DECPHASE].desc = "G7231DecPhase_jbc";
	ProfileStat[PROFILE_INDEX_G7231DECPHASE].valid = 1;
	ProfileStat[PROFILE_INDEX_G726DECPHASE].desc = "G726DecPhase";
	ProfileStat[PROFILE_INDEX_G726DECPHASE].valid = 1;
	ProfileStat[PROFILE_INDEX_GSMFRDECPHASE].desc = "GSMFRDecPhase";
	ProfileStat[PROFILE_INDEX_GSMFRDECPHASE].valid = 1;
	ProfileStat[PROFILE_INDEX_ILBC30MSDECPHASE].desc = "iLBC30msDecPhase";
	ProfileStat[PROFILE_INDEX_ILBC30MSDECPHASE].valid = 1;
	ProfileStat[PROFILE_INDEX_ILBC20MSDECPHASE].desc = "iLBC20msDecPhase";
	ProfileStat[PROFILE_INDEX_ILBC20MSDECPHASE].valid = 1;
	ProfileStat[PROFILE_INDEX_T38DECPHASE].desc = "T38DecPhase";
	ProfileStat[PROFILE_INDEX_T38DECPHASE].valid = 1;
	ProfileStat[PROFILE_INDEX_AES].desc = "AES";
	ProfileStat[PROFILE_INDEX_AES].valid = 1;
	ProfileStat[PROFILE_INDEX_G711ENCPHASE].desc = "G711EncPhase";
	ProfileStat[PROFILE_INDEX_G711ENCPHASE].valid = 1;
	ProfileStat[PROFILE_INDEX_G729ENCPHASE].desc = "G729EncPhase";
	ProfileStat[PROFILE_INDEX_G729ENCPHASE].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231ENCPHASE].desc = "G7231EncPhase";
	ProfileStat[PROFILE_INDEX_G7231ENCPHASE].valid = 1;
	ProfileStat[PROFILE_INDEX_G726ENCPHASE].desc = "G726EncPhase";
	ProfileStat[PROFILE_INDEX_G726ENCPHASE].valid = 1;
	ProfileStat[PROFILE_INDEX_GSMFRENCPHASE].desc = "GSMFREncPhase";
	ProfileStat[PROFILE_INDEX_GSMFRENCPHASE].valid = 1;
	ProfileStat[PROFILE_INDEX_ILBC30MSENCPHASE].desc = "iLBC30msEncPhase";
	ProfileStat[PROFILE_INDEX_ILBC30MSENCPHASE].valid = 1;
	ProfileStat[PROFILE_INDEX_ILBC20MSENCPHASE].desc = "iLBC20msEncPhase";
	ProfileStat[PROFILE_INDEX_ILBC20MSENCPHASE].valid = 1;
	ProfileStat[PROFILE_INDEX_T38ENCPHASE].desc = "T38EncPhase";
	ProfileStat[PROFILE_INDEX_T38ENCPHASE].valid = 1;
	ProfileStat[PROFILE_INDEX_G729JBC].desc = "G729_JBCtrl";
	ProfileStat[PROFILE_INDEX_G729JBC].valid = 1;
	ProfileStat[PROFILE_INDEX_G729DEC].desc = "G729Dec";
	ProfileStat[PROFILE_INDEX_G729DEC].valid = 1;
	ProfileStat[PROFILE_INDEX_G722ENCPHASE].desc = "G722EncPhase";
	ProfileStat[PROFILE_INDEX_G722ENCPHASE].valid = 1;	
	ProfileStat[PROFILE_INDEX_G722DECPHASE].desc = "G722DecPhase_jbc";
	ProfileStat[PROFILE_INDEX_G722DECPHASE].valid = 1;
	ProfileStat[PROFILE_INDEX_AMRNBENCPHASE].desc = "AMR_NB_EncPhase";
	ProfileStat[PROFILE_INDEX_AMRNBENCPHASE].valid = 1;
	ProfileStat[PROFILE_INDEX_AMRNBDECPHASE].desc = "AMR_NB_DecPhase_jbc";
	ProfileStat[PROFILE_INDEX_AMRNBENCPHASE].valid = 1;
	ProfileStat[PROFILE_INDEX_SPEEXNBENCPHASE].desc = "SPEEX_NB_EncPhase";
	ProfileStat[PROFILE_INDEX_SPEEXNBENCPHASE].valid = 1;
	ProfileStat[PROFILE_INDEX_SPEEXNBDECPHASE].desc = "SPEEX_NB_DecPhase_jbc";
	ProfileStat[PROFILE_INDEX_SPEEXNBDECPHASE].valid = 1;
	ProfileStat[PROFILE_INDEX_G7111NBENCPHASE].desc = "G7111NBEncPhase";
	ProfileStat[PROFILE_INDEX_G7111NBENCPHASE].valid = 1;
	ProfileStat[PROFILE_INDEX_G7111NBDECPHASE].desc = "G7111NBDecPhase";
	ProfileStat[PROFILE_INDEX_G7111NBDECPHASE].valid = 1;
	ProfileStat[PROFILE_INDEX_G7111WBENCPHASE].desc = "G7111WBEncPhase";
	ProfileStat[PROFILE_INDEX_G7111WBENCPHASE].valid = 1;
	ProfileStat[PROFILE_INDEX_G7111WBDECPHASE].desc = "G7111WBDecPhase";
	ProfileStat[PROFILE_INDEX_G7111WBDECPHASE].valid = 1;


	
	
	/* G729 Encoder */ 
	ProfileStat[PROFILE_INDEX_G729AB_ENCODER].desc = "G729ab_Encoder";
	ProfileStat[PROFILE_INDEX_G729AB_ENCODER].valid = 1;
	ProfileStat[PROFILE_INDEX_G729AB_PRE_PROCESSS].desc = "Pre_Processs";
	ProfileStat[PROFILE_INDEX_G729AB_PRE_PROCESSS].valid = 1;
	ProfileStat[PROFILE_INDEX_G729AB_CODER_LD8A].desc = "Coder_ld8a";
	ProfileStat[PROFILE_INDEX_G729AB_CODER_LD8A].valid = 1;
	ProfileStat[PROFILE_INDEX_G729AB_PRM2BITS].desc = "prm2bits_ld8k";
	ProfileStat[PROFILE_INDEX_G729AB_PRM2BITS].valid = 1;
	ProfileStat[PROFILE_INDEX_G729AB_AUDOCORRS].desc = "Autocorrs";
	ProfileStat[PROFILE_INDEX_G729AB_AUDOCORRS].valid = 1;
	ProfileStat[PROFILE_INDEX_G729AB_LEVINSONS].desc = "Levinsons";
	ProfileStat[PROFILE_INDEX_G729AB_LEVINSONS].valid = 1;
	ProfileStat[PROFILE_INDEX_G729AB_AZ_LSP].desc = "Az_lsp";
	ProfileStat[PROFILE_INDEX_G729AB_AZ_LSP].valid = 1;
	ProfileStat[PROFILE_INDEX_G729AB_LSP_LSFS].desc = "Lsp_lsfs";
	ProfileStat[PROFILE_INDEX_G729AB_LSP_LSFS].valid = 1;
	ProfileStat[PROFILE_INDEX_G729AB_VAD].desc = "vad";
	ProfileStat[PROFILE_INDEX_G729AB_VAD].valid = 1;
	ProfileStat[PROFILE_INDEX_G729AB_COD_CNG].desc = "Cod_cng";
	ProfileStat[PROFILE_INDEX_G729AB_COD_CNG].valid = 1;
	ProfileStat[PROFILE_INDEX_G729AB_RESIDUS].desc = "Residus";
	ProfileStat[PROFILE_INDEX_G729AB_RESIDUS].valid = 1;
	ProfileStat[PROFILE_INDEX_G729AB_WEIGHT_AZS].desc = "Weight_Azs";
	ProfileStat[PROFILE_INDEX_G729AB_WEIGHT_AZS].valid = 1;
	ProfileStat[PROFILE_INDEX_G729AB_SYN_FILTS].desc = "Syn_filts";
	ProfileStat[PROFILE_INDEX_G729AB_SYN_FILTS].valid = 1;
	ProfileStat[PROFILE_INDEX_G729AB_QUA_LSP].desc = "Qua_lsp";
	ProfileStat[PROFILE_INDEX_G729AB_QUA_LSP].valid = 1;
	ProfileStat[PROFILE_INDEX_G729AB_INT_QLPCS].desc = "Int_qlpcs";
	ProfileStat[PROFILE_INDEX_G729AB_INT_QLPCS].valid = 1;
	ProfileStat[PROFILE_INDEX_G729AB_PITCH_OL].desc = "Pitch_ol_fasts";
	ProfileStat[PROFILE_INDEX_G729AB_PITCH_OL].valid = 1;
	ProfileStat[PROFILE_INDEX_G729AB_PITCH_FR3].desc = "Pitch_fr3_fast";
	ProfileStat[PROFILE_INDEX_G729AB_PITCH_FR3].valid = 1;
	ProfileStat[PROFILE_INDEX_G729AB_ENC_LAG3].desc = "Enc_lag3";
	ProfileStat[PROFILE_INDEX_G729AB_ENC_LAG3].valid = 1;
	ProfileStat[PROFILE_INDEX_G729AB_G_PITCHS].desc = "G_pitchs";
	ProfileStat[PROFILE_INDEX_G729AB_G_PITCHS].valid = 1;
	ProfileStat[PROFILE_INDEX_G729AB_COD_LD8A_S1].desc = "g729_cod_ld8a_sub1s";
	ProfileStat[PROFILE_INDEX_G729AB_COD_LD8A_S1].valid = 1;
	ProfileStat[PROFILE_INDEX_G729AB_ACELP].desc = "ACELP_Code_A";
	ProfileStat[PROFILE_INDEX_G729AB_ACELP].valid = 1;
	ProfileStat[PROFILE_INDEX_G729AB_CORRXY2S].desc = "Corr_xy2s";
	ProfileStat[PROFILE_INDEX_G729AB_CORRXY2S].valid = 1;
	ProfileStat[PROFILE_INDEX_G729AB_COD_LD8A_S2].desc = "g729_cod_ld8a_sub2s";
	ProfileStat[PROFILE_INDEX_G729AB_COD_LD8A_S2].valid = 1;
	ProfileStat[PROFILE_INDEX_G729AB_QUA_GAINS].desc = "Qua_gains";
	ProfileStat[PROFILE_INDEX_G729AB_QUA_GAINS].valid = 1;
	ProfileStat[PROFILE_INDEX_G729AB_COPY].desc = "Copy";
	ProfileStat[PROFILE_INDEX_G729AB_COPY].valid = 1;
	ProfileStat[PROFILE_INDEX_G729AB_CROSSCONV].desc = "g729_crossconvs";
	ProfileStat[PROFILE_INDEX_G729AB_CROSSCONV].valid = 1;
	/* G729 Decoder */ 
	ProfileStat[PROFILE_INDEX_G729AB_DECODER].desc = "G729ab_Decoder";
	ProfileStat[PROFILE_INDEX_G729AB_DECODER].valid = 1;


	/* G7231 Encoder */
	ProfileStat[PROFILE_INDEX_G7231_ENCODER].desc = "G7231Enc";
	ProfileStat[PROFILE_INDEX_G7231_ENCODER].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_REM_DC].desc = "Rem_Dc";
	ProfileStat[PROFILE_INDEX_G7231_REM_DC].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_COMP_LPC].desc = "Comp_Lpc";
	ProfileStat[PROFILE_INDEX_G7231_COMP_LPC].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_ATOLSP].desc = "AtoLsps";
	ProfileStat[PROFILE_INDEX_G7231_ATOLSP].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_COMP_VAD].desc = "Comp_Vad";
	ProfileStat[PROFILE_INDEX_G7231_COMP_VAD].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_LSP_QNT].desc = "Lsp_Qnt";
	ProfileStat[PROFILE_INDEX_G7231_LSP_QNT].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_MEM_SHIFT].desc = "Mem_Shift";
	ProfileStat[PROFILE_INDEX_G7231_MEM_SHIFT].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_WGHT_LPC].desc = "Wght_Lpcs";
	ProfileStat[PROFILE_INDEX_G7231_WGHT_LPC].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_ERROR_WGHT].desc = "Error_Wghts";
	ProfileStat[PROFILE_INDEX_G7231_ERROR_WGHT].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_VEC_NORM].desc = "Vec_Norms";
	ProfileStat[PROFILE_INDEX_G7231_VEC_NORM].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_ESTPITCHLOOP].desc = "Estim_Pitchs";
	ProfileStat[PROFILE_INDEX_G7231_ESTPITCHLOOP].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_COMP_PWS_LOOP].desc = "Comp_Pws";
	ProfileStat[PROFILE_INDEX_G7231_COMP_PWS_LOOP].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_FILT_PW_LOOP].desc = "Filt_Pws";
	ProfileStat[PROFILE_INDEX_G7231_FILT_PW_LOOP].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_LSP_INQ].desc = "Lsp_Inq";
	ProfileStat[PROFILE_INDEX_G7231_LSP_INQ].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_COMP_IR].desc = "Comp_Irs";
	ProfileStat[PROFILE_INDEX_G7231_COMP_IR].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_SUB_RING].desc = "Sub_Rings";
	ProfileStat[PROFILE_INDEX_G7231_SUB_RING].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_FIND_ACBK].desc = "Find_Acbks";
	ProfileStat[PROFILE_INDEX_G7231_FIND_ACBK].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_FIND_FCBK].desc = "Find_Fcbk";
	ProfileStat[PROFILE_INDEX_G7231_FIND_FCBK].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_DECOD_ACBK].desc = "Decod_Acbks";
	ProfileStat[PROFILE_INDEX_G7231_DECOD_ACBK].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_UPDATE_ERR].desc = "Update_Err";
	ProfileStat[PROFILE_INDEX_G7231_UPDATE_ERR].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_UPD_RING].desc = "Upd_Rings";
	ProfileStat[PROFILE_INDEX_G7231_UPD_RING].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_LINE_PACK].desc = "Line_Pack";
	ProfileStat[PROFILE_INDEX_G7231_LINE_PACK].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_CROSSCONV].desc = "g7231_crossconvs";
	ProfileStat[PROFILE_INDEX_G7231_CROSSCONV].valid = 1;

	/* G7231 Decoder */
	ProfileStat[PROFILE_INDEX_G7231_DECODER].desc = "G7231Dec";
	ProfileStat[PROFILE_INDEX_G7231_DECODER].valid = 1;	
	ProfileStat[PROFILE_INDEX_G7231_LINE_UNPK].desc = "Line_Unpk";
	ProfileStat[PROFILE_INDEX_G7231_LINE_UNPK].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_DEC_CNG].desc = "Dec_Cng";
	ProfileStat[PROFILE_INDEX_G7231_DEC_CNG].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_DEC_LSP_INQ].desc = "Lsp_Inq";
	ProfileStat[PROFILE_INDEX_G7231_DEC_LSP_INQ].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_LSP_INT].desc = "Lsp_Int";
	ProfileStat[PROFILE_INDEX_G7231_LSP_INT].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_FCBK_UNPK].desc = "Fcbk_Unpk";
	ProfileStat[PROFILE_INDEX_G7231_FCBK_UNPK].valid = 1;
	//ProfileStat[PROFILE_INDEX_G7231_DECOD_ACBK].desc = "Decod_Acbks";
	//ProfileStat[PROFILE_INDEX_G7231_DECOD_ACBK].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_COMP_INFO].desc = "Comp_Info";
	ProfileStat[PROFILE_INDEX_G7231_COMP_INFO].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_COMP_LPF].desc = "Comp_Lpf";
	ProfileStat[PROFILE_INDEX_G7231_COMP_LPF].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_FILT_LPF].desc = "Filt_Lpfs";
	ProfileStat[PROFILE_INDEX_G7231_FILT_LPF].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_REGEN].desc = "Regen";
	ProfileStat[PROFILE_INDEX_G7231_REGEN].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_SYNT].desc = "Synts2";
	ProfileStat[PROFILE_INDEX_G7231_SYNT].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_SPF].desc = "Spf";
	ProfileStat[PROFILE_INDEX_G7231_SPF].valid = 1;
	ProfileStat[PROFILE_INDEX_G7231_SCALE].desc = "Scales";
	ProfileStat[PROFILE_INDEX_G7231_SCALE].valid = 1;
	
	/* G726 Encoder */
	ProfileStat[PROFILE_INDEX_G726_ENCODER].desc = "G726Enc";
	ProfileStat[PROFILE_INDEX_G726_ENCODER].valid = 1;
	
	/* G726 Decoder */
	ProfileStat[PROFILE_INDEX_G726_DECODER].desc = "G726Dec";
	ProfileStat[PROFILE_INDEX_G726_DECODER].valid = 1;
	
	/* iLBC Encoder */
	ProfileStat[PROFILE_INDEX_ILBC_ENCODER].desc = "iLBCEnc";
	ProfileStat[PROFILE_INDEX_ILBC_ENCODER].valid = 1;
	
	/* iLBC Decoder */
	ProfileStat[PROFILE_INDEX_ILBC_DECODER].desc = "iLBCDec";
	ProfileStat[PROFILE_INDEX_ILBC_DECODER].valid = 1;

	/* Temp */
	ProfileStat[PROFILE_INDEX_TEMP].desc = "Temp";
	ProfileStat[PROFILE_INDEX_TEMP].valid = 1;
	ProfileStat[PROFILE_INDEX_TEMP0].desc = "Temp0";
	ProfileStat[PROFILE_INDEX_TEMP0].valid = 1;

	ProfileStat[PROFILE_INDEX_TEMP200].desc = "Temp200";
	ProfileStat[PROFILE_INDEX_TEMP200].valid = 1;
	ProfileStat[PROFILE_INDEX_TEMP201].desc = "Temp201";
	ProfileStat[PROFILE_INDEX_TEMP201].valid = 1;
	ProfileStat[PROFILE_INDEX_TEMP202].desc = "Temp202";
	ProfileStat[PROFILE_INDEX_TEMP202].valid = 1;
	ProfileStat[PROFILE_INDEX_TEMP203].desc = "Temp203";
	ProfileStat[PROFILE_INDEX_TEMP203].valid = 1;
	ProfileStat[PROFILE_INDEX_TEMP204].desc = "Temp204";
	ProfileStat[PROFILE_INDEX_TEMP204].valid = 1;
	ProfileStat[PROFILE_INDEX_TEMP205].desc = "Temp205";
	ProfileStat[PROFILE_INDEX_TEMP205].valid = 1;
	ProfileStat[PROFILE_INDEX_TEMP206].desc = "Temp206";
	ProfileStat[PROFILE_INDEX_TEMP206].valid = 1;
	ProfileStat[PROFILE_INDEX_TEMP207].desc = "Temp207";
	ProfileStat[PROFILE_INDEX_TEMP207].valid = 1;
	ProfileStat[PROFILE_INDEX_TEMP208].desc = "Temp208";
	ProfileStat[PROFILE_INDEX_TEMP208].valid = 1;
	ProfileStat[PROFILE_INDEX_TEMP209].desc = "Temp209";
	ProfileStat[PROFILE_INDEX_TEMP209].valid = 1;
	ProfileStat[PROFILE_INDEX_TEMP210].desc = "Temp210";
	ProfileStat[PROFILE_INDEX_TEMP210].valid = 1;
	ProfileStat[PROFILE_INDEX_TEMP211].desc = "Temp211";
	ProfileStat[PROFILE_INDEX_TEMP211].valid = 1;
	ProfileStat[PROFILE_INDEX_TEMP212].desc = "Temp212";
	ProfileStat[PROFILE_INDEX_TEMP212].valid = 1;
	ProfileStat[PROFILE_INDEX_TEMP213].desc = "Temp213";
	ProfileStat[PROFILE_INDEX_TEMP213].valid = 1;
	ProfileStat[PROFILE_INDEX_TEMP214].desc = "Temp214";
	ProfileStat[PROFILE_INDEX_TEMP214].valid = 1;
	ProfileStat[PROFILE_INDEX_TEMP215].desc = "Temp215";
	ProfileStat[PROFILE_INDEX_TEMP215].valid = 1;
	ProfileStat[PROFILE_INDEX_TEMP216].desc = "Temp216";
	ProfileStat[PROFILE_INDEX_TEMP216].valid = 1;
	ProfileStat[PROFILE_INDEX_TEMP217].desc = "Temp217";
	ProfileStat[PROFILE_INDEX_TEMP217].valid = 1;
	ProfileStat[PROFILE_INDEX_TEMP218].desc = "Temp218";
	ProfileStat[PROFILE_INDEX_TEMP218].valid = 1;
	ProfileStat[PROFILE_INDEX_TEMP219].desc = "Temp219";
	ProfileStat[PROFILE_INDEX_TEMP219].valid = 1;

	return SUCCESS;
}

int ProfileReset(void)
{
	ProfileInit();
	
	return SUCCESS;
}


int ProfileGet( uint64 *pGet )
{
	if ( profile_inited == FALSE ) return FAILED;

	/* Louis patch: someone will disable CP3 in somewhere. */
	CP3_COUNTER0_INIT();

#ifndef VOIP_ENABLE_GCC_INSTRUMENT_MEASUER
	CP3_COUNTER0_STOP();
#endif
	*pGet = CP3_COUNTER0_GET();
	CP3_COUNTER0_START();
	
	return SUCCESS;
}

int ProfilePause( void )
{
	if ( profile_inited == FALSE ) return FAILED;

	profile_enable = FALSE;
	
	/* Louis patch: someone will disable CP3 in somewhere. */
	CP3_COUNTER0_INIT();

	CP3_COUNTER0_STOP();
	
	return SUCCESS;
}

int ProfileResume( void )
{
	if ( profile_inited == FALSE ) return FAILED;

	profile_enable = TRUE;
	
	/* Louis patch: someone will disable CP3 in somewhere. */
	CP3_COUNTER0_INIT();
 	
	CP3_COUNTER0_START();
	
	return SUCCESS;
}

int ProfileEnterPoint( uint32 index )
{
#ifndef VOIP_ENABLE_GCC_INSTRUMENT_MEASUER
	if ( profile_inited == FALSE ||
	     profile_enable == FALSE ) return FAILED;
#endif
	if ( index >= (sizeof(ProfileStat)/sizeof(profile_stat_t)) )
		return FAILED;

	/* Louis patch: someone will disable CP3 in somewhere. */
	CP3_COUNTER0_INIT();

#ifndef VOIP_ENABLE_GCC_INSTRUMENT_MEASUER
	CP3_COUNTER0_STOP();
#else
	CP3_COUNTER0_PASUE();
#endif
	CP3_COUNTER0_GET_ALL();
	ProfileStat[index].tempCycle[0] = currCnt[0];
	ProfileStat[index].tempCycle[1] = currCnt[1];
	ProfileStat[index].tempCycle[2] = currCnt[2];
	ProfileStat[index].tempCycle[3] = currCnt[3];
	ProfileStat[index].hasTempCycle = TRUE;
#ifndef VOIP_ENABLE_GCC_INSTRUMENT_MEASUER
	CP3_COUNTER0_START();
#else
	CP3_COUNTER0_RESUME();
#endif

	return SUCCESS;
}

int ProfileExitPoint( uint32 index )
{
#ifndef VOIP_ENABLE_GCC_INSTRUMENT_MEASUER
	if ( profile_inited == FALSE ||
	     profile_enable == FALSE ) return FAILED;
	if ( index >= (sizeof(ProfileStat)/sizeof(profile_stat_t)) )
		return FAILED;
#endif
	if ( ProfileStat[index].hasTempCycle == FALSE )
		return FAILED;

	/* Louis patch: someone will disable CP3 in somewhere. */
	CP3_COUNTER0_INIT();
	
#ifndef VOIP_ENABLE_GCC_INSTRUMENT_MEASUER
	CP3_COUNTER0_STOP();
#else
	CP3_COUNTER0_PASUE();
#endif
	CP3_COUNTER0_GET_ALL();

	ProfileStat[index].accCycle[0] += currCnt[0]-ProfileStat[index].tempCycle[0];
	if(ProfileStat[index].maxCycle[0]<currCnt[0]-ProfileStat[index].tempCycle[0])
		ProfileStat[index].maxCycle[0] = currCnt[0]-ProfileStat[index].tempCycle[0];

	ProfileStat[index].accCycle[1] += currCnt[1]-ProfileStat[index].tempCycle[1];
	if(ProfileStat[index].maxCycle[1]<currCnt[1]-ProfileStat[index].tempCycle[1])
		ProfileStat[index].maxCycle[1] = currCnt[1]-ProfileStat[index].tempCycle[1];

	ProfileStat[index].accCycle[2] += currCnt[2]-ProfileStat[index].tempCycle[2];
	if(ProfileStat[index].maxCycle[2]<currCnt[2]-ProfileStat[index].tempCycle[2])
		ProfileStat[index].maxCycle[2] = currCnt[2]-ProfileStat[index].tempCycle[2];
	
	ProfileStat[index].accCycle[3] += currCnt[3]-ProfileStat[index].tempCycle[3];
	if(ProfileStat[index].maxCycle[3]<currCnt[3]-ProfileStat[index].tempCycle[3])
		ProfileStat[index].maxCycle[3] = currCnt[3]-ProfileStat[index].tempCycle[3];

	ProfileStat[index].hasTempCycle = FALSE;
	ProfileStat[index].executedNum++;
#ifndef VOIP_ENABLE_GCC_INSTRUMENT_MEASUER
	CP3_COUNTER0_START();
#else
	CP3_COUNTER0_RESUME();
#endif
	
	return SUCCESS;
}

int ProfileDump( uint32 start, uint32 end, uint32 period )
{
	char szBuf[128];
	int i;

	profile_stat_t* statSnapShot = (profile_stat_t*)&TempProfStat[0];

	if(period > 0)
	{
		
		if( statSnapShot == NULL )
		{
			cp3_printf("statSnapShot mem alloc failed\n");
			return FAILED;
		}
		
		ProfileStat[start].per_count++;
			
		if(ProfileStat[start].per_count==period)
		{
	
			sprintf(szBuf, "index %30s %12s %8s %10s\n", "description", "accCycle", "totalNum", "Average" );
			cp3_printf(szBuf);
		
			for( i = start; i <= end; i++ )
			{
				if(ProfileStat[i].valid==0)
					continue;
				int j;
				for( j =0; j < sizeof(ProfileStat[i].accCycle)/sizeof(ProfileStat[i].accCycle[0]); j++ )
				{
					statSnapShot[i].accCycle[j]  = ProfileStat[i].accCycle[j];
					statSnapShot[i].tempCycle[j] = ProfileStat[i].tempCycle[j];
#ifdef VOIP_ENABLE_GCC_INSTRUMENT_MEASUER
					ProfileStat[i].accCycle[j]=0;
					ProfileStat[i].tempCycle[j]=0;;
#endif
				}
				statSnapShot[i].executedNum  = ProfileStat[i].executedNum;
				statSnapShot[i].hasTempCycle = ProfileStat[i].hasTempCycle;
#ifdef VOIP_ENABLE_GCC_INSTRUMENT_MEASUER
				ProfileStat[i].executedNum=0;
				ProfileStat[i].hasTempCycle=0;
#endif
			}
		
			for( i = start; i <= end; i++ )
			{
				if(ProfileStat[i].valid==0)
					continue;
				if ( statSnapShot[i].executedNum == 0 )
				{
					sprintf(szBuf, "[%3d] %30s %12s %8s %10s\n", i, ProfileStat[i].desc, "--", "--", "--" );
					cp3_printf(szBuf);
				}
				else
				{
					int j;
					sprintf(szBuf, "[%3d] %30s ", i, ProfileStat[i].desc );
					cp3_printf(szBuf);
					for( j =0; j < sizeof(statSnapShot[i].accCycle)/sizeof(statSnapShot[i].accCycle[0]); j++ )
					{
						uint32 *pAccCycle = (uint32*)&statSnapShot[i].accCycle[j];
						uint32 avrgCycle = /* Hi-word */ (pAccCycle[0]*(0xffffffff/statSnapShot[i].executedNum)) +
						                   /* Low-word */(pAccCycle[1]/statSnapShot[i].executedNum);
						sprintf(szBuf,  "%12llu %8u %10u\n",
							statSnapShot[i].accCycle[j], 
							statSnapShot[i].executedNum, 
							avrgCycle );
						cp3_printf(szBuf);
		
						sprintf(szBuf, " %3s  %30s ", "", "" );
						cp3_printf(szBuf);
					}
					cp3_printf( "\r" );
				}
			}
			
			
			ProfileStat[start].per_count = 0;
			
			for (i=0; i<4; i++)
			{
				ProfileStat[start].accCycle[i]=0;
				ProfileStat[start].executedNum=0;
				ProfileStat[start].hasTempCycle=0;
				ProfileStat[start].tempCycle[i]=0;
				
				statSnapShot[start].accCycle[i]=0;
				statSnapShot[start].tempCycle[i]=0;
				statSnapShot[start].executedNum=0;
				statSnapShot[start].hasTempCycle=0;
			}
			
			return SUCCESS;
		}
		


	}

	return SUCCESS;

}


int ProfilePerDump( uint32 start, uint32 period )
{
	int i, j;
	char szBuf[128];
	profile_stat_t* statSnapShot = (profile_stat_t*)&TempProfStat[0];
	
	if(period > 0)
	{	
		if( statSnapShot == NULL )
		{
			cp3_printf("statSnapShot mem alloc failed\n");
			return FAILED;
		}
		
		if(ProfileStat[start].valid==0)
			return FAILED;

		ProfileStat[start].per_count++;
		if(ProfileStat[start].per_count>=period)
		{
			for( i = start; i <= start; i++ )
			{
				if(ProfileStat[i].valid==0)
					continue;

				for( j =0; j < sizeof(ProfileStat[i].accCycle)/sizeof(ProfileStat[i].accCycle[0]); j++ )
				{
					statSnapShot[i].maxCycle[j] = ProfileStat[i].maxCycle[j];
					statSnapShot[i].accCycle[j]  = ProfileStat[i].accCycle[j];
					statSnapShot[i].tempCycle[j] = ProfileStat[i].tempCycle[j];
				}

				statSnapShot[i].executedNum  = ProfileStat[i].executedNum;
				statSnapShot[i].hasTempCycle = ProfileStat[i].hasTempCycle;
			}
		
			for( i = start; i <= start; i++ )
			{
				if(ProfileStat[i].valid==0)
					continue;

				for( j =0; j < sizeof(statSnapShot[i].accCycle)/sizeof(statSnapShot[i].accCycle[0]); j++ )
				//for( j =0; j < 1; j++ )
				{
					uint32 *pAccCycle = (uint32*)&statSnapShot[i].accCycle[j];
					uint32 avrgCycle = /* Hi-word */ (pAccCycle[0]*(0xffffffff/statSnapShot[i].executedNum)) +
					                   /* Low-word */(pAccCycle[1]/statSnapShot[i].executedNum);

					if ( statSnapShot[i].executedNum == 0 )
					{
						sprintf(szBuf,"%d. %llu, ---\n",
							i, ProfileStat[i].maxCycle[j]);
					}
					else
					{
						sprintf(szBuf,"%d. %llu, %u\n",
							i, ProfileStat[i].maxCycle[j], avrgCycle);
					}

					cp3_printf(szBuf);

					ProfileStat[i].maxCycle[j] = 0;
					ProfileStat[i].accCycle[j] = 0;
					ProfileStat[i].tempCycle[j] = 0;

					statSnapShot[i].maxCycle[j] = 0;				
					statSnapShot[i].accCycle[j] = 0;
					statSnapShot[i].tempCycle[j] = 0;
				}

				ProfileStat[i].executedNum = 0;
				ProfileStat[i].hasTempCycle = 0;

				statSnapShot[i].executedNum = 0;
				statSnapShot[i].hasTempCycle = 0;
			}

			ProfileStat[start].per_count = 0;
		}
		return SUCCESS;
	}
	return SUCCESS;
}

voip_initcall( ProfileInit );
#endif /* FEATURE_COP3_PROFILE */
