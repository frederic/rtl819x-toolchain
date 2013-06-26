#ifndef __IVR_HANDLER_H__
#define __IVR_HANDLER_H__

#include "ivripc.h"

extern int IsInstructionForIVR( const do_ivr_ins_t *msg );
extern void InitializeIVR( void );

#endif /* __IVR_HANDLER_H__ */

