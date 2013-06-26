#ifndef __IVR_H__
#define __IVR_H__

#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_params.h"
#include "voip_control.h"

#define MAX_IVR_SPEECH_TEXT		40
#define	IVR_BUF_SIZE 			PCM_PERIOD_10MS_SIZE
#define _IVR_711A_SPEECH		/* use 711 a-law speech to reduce size */

#ifdef _IVR_711A_SPEECH
 #define SPEECH_WAVE_10MS_SIZE	( PCM_PERIOD_10MS_SIZE / 2 )
#else
 #define SPEECH_WAVE_10MS_SIZE	PCM_PERIOD_10MS_SIZE
#endif

typedef struct ivr_speech_wave_info_s{
	unsigned int length;
	unsigned char *pSpeechWave;
} ivr_speech_wave_info_t;

enum {
	IVR_SPEECH_ID_NUM_0,
	IVR_SPEECH_ID_NUM_1,
	IVR_SPEECH_ID_NUM_2,
	IVR_SPEECH_ID_NUM_3,
	IVR_SPEECH_ID_NUM_4,
	IVR_SPEECH_ID_NUM_5,
	IVR_SPEECH_ID_NUM_6,
	IVR_SPEECH_ID_NUM_7,
	IVR_SPEECH_ID_NUM_8,
	IVR_SPEECH_ID_NUM_9,
	IVR_SPEECH_ID_DOT,
	IVR_SPEECH_ID_ALPHA_A,
	IVR_SPEECH_ID_ALPHA_B,
	IVR_SPEECH_ID_ALPHA_C,
	IVR_SPEECH_ID_ALPHA_D,
	IVR_SPEECH_ID_ALPHA_E,
	IVR_SPEECH_ID_ALPHA_F,
	IVR_SPEECH_ID_ALPHA_G,
	IVR_SPEECH_ID_ALPHA_H,
	IVR_SPEECH_ID_ALPHA_I,
	IVR_SPEECH_ID_ALPHA_J,
	IVR_SPEECH_ID_ALPHA_K,
	IVR_SPEECH_ID_ALPHA_L,
	IVR_SPEECH_ID_ALPHA_M,
	IVR_SPEECH_ID_ALPHA_N,
	IVR_SPEECH_ID_ALPHA_O,
	IVR_SPEECH_ID_ALPHA_P,
	IVR_SPEECH_ID_ALPHA_Q,
	IVR_SPEECH_ID_ALPHA_R,
	IVR_SPEECH_ID_ALPHA_S,
	IVR_SPEECH_ID_ALPHA_T,
	IVR_SPEECH_ID_ALPHA_U,
	IVR_SPEECH_ID_ALPHA_V,
	IVR_SPEECH_ID_ALPHA_W,
	IVR_SPEECH_ID_ALPHA_X,
	IVR_SPEECH_ID_ALPHA_Y,
	IVR_SPEECH_ID_ALPHA_Z,
	IVR_SPEECH_ID_TXT_DHCP,
	IVR_SPEECH_ID_TXT_FIX_IP,
	IVR_SPEECH_ID_TXT_NO_RESOURCE,
	IVR_SPEECH_ID_TXT_PLZ_ENTER_NUMBER,
	IVR_SPEECH_ID_TXT_PLEASE_ENTER_PASSWORD,
	///<<&&ID3&&>>	/* DON'T remove this line, it helps wizard to generate ID. */
	//IVR_SPEECH_ID_USER1,
	NUM_OF_IVR_SPEECH_WAVE,
};

/* Initialize IVR vars */
extern void InitializeIVR( unsigned int chid );

/* AP ask to play a text, and fill playing period in unit of 10ms */
extern void PlayIvrDispatcher( TstVoipPlayIVR_Header *pHeader, const void *user );

/* AP poll IVR is playing or not */
extern unsigned int PollIvrPlaying( unsigned int chid );

/* AP ask to stop playing (off-hook is a possible reason.) */
extern void StopIvrPlaying( unsigned int chid );

/* PCM handler mix speech wave into local/remote playout buffer. */
extern void MixIvrSpeechWaveToPlayoutBuffer_Local( unsigned int chid, uint32 *pMixerTxBuffer );
extern void MixIvrSpeechWaveToPlayoutBuffer_Remote( unsigned int chid, uint32 *pMixerRxBuffer );

/* === Used by IVR module === */
/* Convert string character ID to internal ID */
extern unsigned int GetWaveIndexFromTextChar( unsigned char ch );

/* For IVR File Play */
extern unsigned int PlayIvrText2Speech( unsigned int chid, IvrPlayDir_t dir, const unsigned char *pText2Speech );
extern unsigned int PlayIvrG72363( unsigned int chid, unsigned int nFrameCount, const unsigned char *pData, unsigned int *pCopiedFrame );
extern unsigned int PlayIvrG729( unsigned int chid, unsigned int nFrameCount, const unsigned char *pData, unsigned int *pCopiedFrame );
extern unsigned int PlayIvrG711( unsigned int chid, unsigned int nFrameCount, const unsigned char *pData, unsigned int *pCopiedFrame );
extern unsigned int PlayIvrLinear8k( unsigned int chid, unsigned int nFrameCount, const unsigned char *pData, unsigned int *pCopiedFrame );
extern void ivr_gain(int chid, short* pIvrBuf);
#endif /* __IVR_H__ */


