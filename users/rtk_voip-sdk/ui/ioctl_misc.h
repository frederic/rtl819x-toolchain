#ifndef __IOCTL_MISC_H__
#define __IOCTL_MISC_H__

#include "mm_ring.h"

extern int SetPlayTone( tone_id_t idTone, int bEnable, int bInvolvePCM );

extern int EnablePCMtoRunDSP( unsigned int bEnable );

#endif /* __IOCTL_MISC_H__ */

