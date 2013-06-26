#include <linux/kernel.h>
#include "voip_types.h"
#include "con_register.h"
#include "voip_debug.h"
#include "snd_define.h"
#include "zarlink_api.h"

// --------------------------------------------------------
// zarlink common ops 
// --------------------------------------------------------

void SOLAC_PCMSetup_priv_ops( voip_snd_t *this, int enable )
{
	RTKLineObj * const pLine = (RTKLineObj * )this ->priv;
	int pcm_mode = pLine->pcm_law_save;
	
	if( enable ==0 )
		return;
	
#if 0
	PRINT_Y( "%s()line#%d, enable=%d,pLine->pcm_law_save=%d\n",\
			 __FUNCTION__,pLine->ch_id, enable, pLine->pcm_law_save );
#endif

	// check if need to switch between narrowband/wideband mode 
	if (enable == 2) { /* Change to wideband mode */
		//PRINT_Y("Switch to Wideband mode\n");
		switch( pLine->pcm_law_save ) {
			case BUSDATFMT_PCM_LINEAR:
				pcm_mode =  BUSDATFMT_PCM_WIDEBAND_LINEAR; 
				break;
			case BUSDATFMT_PCM_ALAW:
				pcm_mode =  BUSDATFMT_PCM_WIDEBAND_ALAW; 
				break;
			case BUSDATFMT_PCM_ULAW:
				pcm_mode =  BUSDATFMT_PCM_WIDEBAND_ULAW; 
				break;
			default:
			/* do nothing */
			break;
		}
	}
		
	if (enable == 1) { /* Change to narrow band mode */
		//PRINT_Y("Switch to Narrow band mode\n");
		switch( pLine->pcm_law_save ) {
			case BUSDATFMT_PCM_WIDEBAND_ALAW: 	
			PRINT_R("%s() Doesn't support PCMMODE_WIDEBAND_ALAW\n", __FUNCTION__);
			pcm_mode = 	BUSDATFMT_PCM_ALAW;
			break;

			case BUSDATFMT_PCM_WIDEBAND_ULAW: 
			PRINT_R("%s() Doesn't support PCMMODE_WIDEBAND_ULAW\n", __FUNCTION__);
			pcm_mode = 	BUSDATFMT_PCM_ULAW;
			break;

			case BUSDATFMT_PCM_WIDEBAND_LINEAR:
			pcm_mode = 	BUSDATFMT_PCM_LINEAR;
			break;

			default:
			/* do nothing */
			break;
		}
	}

	if (pcm_mode == pLine->pcm_law_save ) {
		/* The same mode as the previous mode */
		//PRINT_Y("The same mode %d(new)\n",pcm_mode);
		return;
	}
	
	if( pcm_mode >= 0 ) {
		ZarlinkSetFxsPcmMode(pLine, pcm_mode);
		ZarlinkSetFxsAcProfileByBand(pLine, pcm_mode);

		/* VE8911 datasheet 3.3.3 
		 * Both the FXS and FXO channels must operate in the same 
		 * (Normal or Wideband) at the same time.
		 */

	} else {
		PRINT_R("%s() pcm_mode %d error\n", __FUNCTION__, pcm_mode);
	}		
}


