#ifndef __ZARLINK_INT_H__
#define __ZARLINK_INT_H__

#include "zarlinkCommonInit.h"

#if defined( CONFIG_RTK_VOIP_SLIC_ZARLINK_880_SERIES )
#include "ve880_int.h"
#endif

#if defined(CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES )
#include "ve890_int.h"
#endif

int ZarlinkInit(int pcm_mode);
RTKLineObj * RtkGetLine(int chid);

#endif /* __ZARLINK_INT_H__ */


