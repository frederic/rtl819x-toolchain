#include "rtk_voip.h"
#include "voip_types.h"

#if ! defined (CONFIG_RTK_VOIP_G7231) || defined (CONFIG_AUDIOCODES_VOIP)
void InitializeIvr723Player( unsigned int chid, void *pIvrPlay )
{
}

unsigned int PutDataIntoIvr723Player( void *pIvrPlay, 
								   const unsigned char *pData, 
								   unsigned int nCount )
{
	return 0;
}

int RunIvr723Player( unsigned int chid, void *pIvrPlay, const Word16 **ppOut )
{
	return 0;
}

unsigned int GetPredictionPeriodOfG723( const void *pIvrPlay )
{
	return 0;
}
#endif /* !CONFIG_RTK_VOIP_G7231 || CONFIG_AUDIOCODES_VOIP */

#if !defined( CONFIG_RTK_VOIP_G729AB ) || defined( CONFIG_AUDIOCODES_VOIP )
void InitializeIvr729Player( unsigned int chid, void *pIvrPlay )
{
}

unsigned int PutDataIntoIvr729Player( void *pIvrPlay, 
									  const unsigned char *pData, 
									  unsigned int nCount )
{
	return 0;
}								 

int RunIvr729Player( unsigned int chid, void *pIvrPlay, 
					 const Word16 **ppOut )
{
	return 0;
}

unsigned int GetPredictionPeriodOfG729( const void *pIvrPlay )
{
	return 0;
}
#endif /* !CONFIG_RTK_VOIP_G729AB || CONFIG_AUDIOCODES_VOIP */


#ifdef CONFIG_AUDIOCODES_VOIP 
void InitializeIvr711Player( unsigned int chid, void *pIvrPlay )
{
}

unsigned int PutDataIntoIvr711Player( void *pIvrPlay, const unsigned char *pData, unsigned int nCount )
{
	return 0;
}								 

int RunIvr711Player( unsigned int chid, void *pIvrPlay, const Word16 **ppOut )
{
	return 0;
}

unsigned int GetPredictionPeriodOfG711( const void *pIvrPlay )
{
	return 0;
}
#endif


#ifdef CONFIG_AUDIOCODES_VOIP
void ivr_gain(int chid, short* pIvrBuf)
{

}
#else
extern long voice_gain_spk[];//0 is mute, 1 is -31dB ~~~ 32 is 0dB , 33 is 1dB ~~~ 63 is 31dB

void ivr_gain(int chid, short* pIvrBuf)
{
	#ifdef VOICE_GAIN_ADJUST_IVR
	extern void voice_gain(int16_t * pBuf, uint32_t data_number, int32_t voicegain);
	voice_gain( pIvrBuf, PCM_PERIOD_10MS_SIZE/2, voice_gain_spk[chid]);
	#endif
}
#endif

