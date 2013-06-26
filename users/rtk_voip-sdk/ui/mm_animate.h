#ifndef __MM_ANIMATE_H__
#define __MM_ANIMATE_H__

#define MM_ANIMATE_PLAY_PERIOD_UNIT		500	/* 500ms */

typedef enum {
	ANI_ID_WAITING_DOT,

	NUM_OF_ANIMATION,
} animate_t;

/* Start animation */
extern int StartPlayingAnimation( int x, int y, animate_t idAni );

/* Stop animation */
extern int StopPlayingAnimation( void );

#endif /* __MM_ANIMATE_H__ */
