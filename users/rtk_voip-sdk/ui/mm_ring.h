#ifndef __MM_RING_H__
#define __MM_RING_H__

#define MM_TONE_PLAY_PERIOD_UNIT	100	/* 100ms */

typedef enum {
	TONE_ID_0,
	TONE_ID_1,
	TONE_ID_2,
	TONE_ID_3,
	TONE_ID_4,
	TONE_ID_5,
	TONE_ID_6,
	TONE_ID_7,
	TONE_ID_8,
	TONE_ID_9,
	TONE_ID_STARSIGN,
	TONE_ID_HASHSIGN,
	TONE_ID_DIAL,		/* edit dial */
	TONE_ID_RING,		/* outoging call ring */
	TONE_ID_IN_RING,	/* incoming call ring */
	TONE_ID_BUSY,
	
	NUM_OF_TONE_ID,
} tone_id_t;

extern void MM_Initialize( void );
extern void MM_Terminate( void );

extern int MM_StartTonePlayingEx( tone_id_t idTone, unsigned int period /* ms */, 
						   int bForce, int bSpeakerOut );
extern int MM_StartTonePlaying( tone_id_t idTone, unsigned int period );
extern int MM_StopTonePlaying( void );


#endif /* __MM_RING_H__ */

