#ifndef __MM_SUP_H__
#define __MM_SUP_H__

#include "ioctl_codec.h"	/* vpMicSpeacker_t */

/* Play tone according to key. */
extern void PlayKeypressTone( unsigned char key );

/* Set voice path and its volume */
extern void SetVoicePathAndItsVolume( vpMicSpeacker_t path );

#endif /* __MM_SUP_H__ */

