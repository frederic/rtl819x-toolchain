#ifndef __T38_HANDLER_H__
#define __T38_HANDLER_H__

typedef enum {
	T38_STOP = 0,
	T38_START,
} t38_running_state_t;

extern t38_running_state_t t38RunningState[MAX_DSP_CH_NUM];

extern int32 PCM_handler_T38( unsigned int chid );

/* T.38 codec API */
#include "t38_api.h"

#endif /* __T38_HANDLER_H__ */

