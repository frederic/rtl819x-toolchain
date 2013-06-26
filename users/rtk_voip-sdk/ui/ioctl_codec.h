#ifndef __IOCTL_CODEC_H__
#define __IOCTL_CODEC_H__

#define _DSP_VOLUME_CONTROLLER		/* Use software volume */
#define MAX_VOLUME_VALUE			63	/* 0 ~ 63 */

/* Set DAC volume */
extern int SetVolumeOfDAC( unsigned char volmic, unsigned char volspk );

#if 0
/* Get DAC *DEFAULT* volume (NOT up to date) */
extern unsigned char GetDefaultVolumeOfDAC( void );
#endif

/* Voice path value */
typedef enum {
	VOC_PATH_MIC1_SPEAKER,
	VOC_PATH_MIC2_MONO,
	VOC_PATH_SPEAKER_ONLY,
	VOC_PATH_MONO_ONLY,
	NUM_OF_VOC_PATH,
} vpMicSpeacker_t;

/* Set voice path */
extern int SetCodecVoicePath( vpMicSpeacker_t path );

#endif /* __IOCTL_CODEC_H__ */

