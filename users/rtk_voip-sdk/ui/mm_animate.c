#include <string.h>
#include "ui_config.h"
#include "ui_host.h"
#include "ui_flags.h"
#include "gs_lib.h"
#include "mm_animate.h"

static void DrawAnimationFrame( void );

typedef struct ani_frame_s {
	int dx;
	int dy;
	unsigned long time;		/* in unit of ms */
	unsigned char *pszDisplay;
} ani_frame_t;

typedef struct ani_entry_s {
	ani_frame_t		*pFrame;
	unsigned int	nFrameNum;
} ani_entry_t;

const ani_frame_t animation_waiting_dot[] = {		/* ANI_ID_WAITING_DOT == 0 */
	{ 0, 0, 1000 /* ms */ , ( unsigned char * )".  " },
	{ 0, 0, 1000 /* ms */ , ( unsigned char * )".. " },
	{ 0, 0, 1000 /* ms */ , ( unsigned char * )"..." },
};

#define _M_ANIMATION_ENTRY( x )		{ ( ani_frame_t * )x, ( sizeof( x ) / sizeof( x[ 0 ] ) ) }

const ani_entry_t animation_album[] = {
	_M_ANIMATION_ENTRY( animation_waiting_dot ),
};

#undef _M_ANIMATION_ENTRY

CT_ASSERT( NUM_OF_ANIMATION == ( sizeof( animation_album ) / sizeof( animation_album[ 0 ] ) ) );

/* variables */
static struct {
	int x;
	int y;
	ani_entry_t ani;
	uptime_t startTime;
	unsigned int snCurFrame;
} anivars;

int StartPlayingAnimation( int x, int y, animate_t idAni )
{
	if( fHostFlags.b.animatePlaying )
		StopPlayingAnimation();

	if( idAni >= NUM_OF_ANIMATION )
		return 0;

	anivars.x = x;
	anivars.y = y;
	anivars.ani = animation_album[ idAni ];
	anivars.snCurFrame = 0;

	fHostFlags.b.animatePlaying = 1;

	DrawAnimationFrame();

	ChangeTimerPeriodForApplication();

	return 1;
}

int StopPlayingAnimation( void )
{
	if( !fHostFlags.b.animatePlaying )
		return 0;

	fHostFlags.b.animatePlaying = 0;

	ChangeTimerPeriodForApplication();

	return 1;
}

static void DrawAnimationFrame( void )
{
	rect_t rect;
	const ani_frame_t *pFrame;
	int len;
	int x, y;

	/* prepare draw variables */
	pFrame = &anivars.ani.pFrame[ anivars.snCurFrame ];
	len = strlen( ( const char * )pFrame ->pszDisplay );

	x = anivars.x + pFrame ->dx;
	y = anivars.y + pFrame ->dy;
	rect.left = x * CHAR_WIDTH;
	rect.top = y * CHAR_HEIGHT;
	rect.right = ( rect.left + len ) * CHAR_WIDTH;
	rect.bottom = ( rect.top + 1 ) * CHAR_HEIGHT;

	/* draw the frame */
	GS_DrawOffScreen();

	GS_TextOut( x, y, pFrame ->pszDisplay, len );

	GS_DrawOnScreen( &rect );

	/* update working variables */
	anivars.startTime = GetUptimeInMillisecond();
}

int TimerEventPlayAnimation( void )
{
	const ani_frame_t *pFrame;

	if( !fHostFlags.b.animatePlaying )
		return 0;

	pFrame = &anivars.ani.pFrame[ anivars.snCurFrame ];

	if( CheckIfTimeoutInMillisecond( &anivars.startTime, pFrame ->time ) )
		return 1;	/* not timeout */

	if( ++ anivars.snCurFrame >= anivars.ani.nFrameNum )	/* wrap-around */
		anivars.snCurFrame = 0;

	DrawAnimationFrame();
	
	return 2;
}
