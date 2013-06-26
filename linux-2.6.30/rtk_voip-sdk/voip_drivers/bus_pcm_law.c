#include "rtk_voip.h"
#include "snd_define.h"

// pkshih: Move PCM law to this file, because IPC:Host need if not fully offload to DSP. 

#if defined( CONFIG_RTK_VOIP_DRIVERS_PCM_ULAW_8K )
int law = BUSDATFMT_PCM_ULAW;
#elif defined( CONFIG_RTK_VOIP_DRIVERS_PCM_ALAW_8K )
int law = BUSDATFMT_PCM_ALAW;
#elif defined( CONFIG_RTK_VOIP_DRIVERS_PCM_LINEAR_8K )
int law = BUSDATFMT_PCM_LINEAR;
#elif defined( CONFIG_RTK_VOIP_DRIVERS_PCM_ULAW_16K )
int law = BUSDATFMT_PCM_WIDEBAND_ULAW;
#elif defined( CONFIG_RTK_VOIP_DRIVERS_PCM_ALAW_16K )
int law = BUSDATFMT_PCM_WIDEBAND_ALAW;
#elif defined( CONFIG_RTK_VOIP_DRIVERS_PCM_LINEAR_16K )
int law = BUSDATFMT_PCM_WIDEBAND_LINEAR;
#endif

#if 0
#ifdef CONFIG_RTK_VOIP_DECT_DSPG_SUPPORT
int law = BUSDATFMT_PCM_LINEAR;
#else
#if 1
int law = BUSDATFMT_PCM_WIDEBAND_LINEAR;
#else
int law = BUSDATFMT_PCM_ALAW; // linear: 0 , a-law: 1, u-law: 2. It set the pcm controller and slic to desired mode.
#endif
#endif
#endif

