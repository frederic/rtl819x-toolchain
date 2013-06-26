#ifndef __UI_MODE_H__
#define __UI_MODE_H__

typedef union {		/* flags for configuration storing in flash */
	struct {
		unsigned long	keypressTone:1;		/* bit 0: turn on keypres tone */
		unsigned long	autoDial:1;			/* bit 1: auto dial */
		unsigned long	autoAnswer:1;		/* bit 2: auto answer */
		unsigned long	offHookAlarm:1;		/* bit 3: off-hook alarm */
		unsigned long	hotLine:1;			/* bit 4: hot line */
	} b;
	unsigned long all;
} fModeFlags_t;

extern fModeFlags_t fModeFlags;

/* auto dial time (second) */
#define MIN_AUTO_DIAL_TIME			3
#define MAX_AUTO_DIAL_TIME			9
#define DEF_AUTO_DIAL_TIME			5

/* auto answer time (second) */
#define MIN_AUTO_ANSWER_TIME		3
#define MAX_AUTO_ANSWER_TIME		9
#define DEF_AUTO_ANSWER_TIME		0

/* off-hook alarm time (second) */
#define MIN_OFF_HOOK_ALARM_TIME		10
#define MAX_OFF_HOOK_ALARM_TIME		60
#define DEF_OFF_HOOK_ALARM_TIME		30

/* in/out volume type */
typedef enum {
	INOUTVOL_TYPE_RECEIVER,
	INOUTVOL_TYPE_SPEAKER,
	INOUTVOL_TYPE_MIC_R,
	INOUTVOL_TYPE_MIC_S,
	NUM_OF_INOUTVOL_TYPE,
} inout_vol_type_t;

extern unsigned char nModeOutVolume[];

/* refresh out volume */
extern int RefreshAdjustOutVolumeXxxFrame( int bDrawVolumeOnly, unsigned char key, 
								 int bRefreshHardware, inout_vol_type_t type );

/* read auto dial time */
extern unsigned long ReadAutoDialTimeInMillisecond( void );

/* read auto answer time */
extern unsigned long ReadAutoAnswerTimeInMillisecond( void );

/* read off-hook alarm time */
extern unsigned long ReadOffHookAlarmTimeInMillisecond( void );


#endif /* __UI_MODE_H__ */

