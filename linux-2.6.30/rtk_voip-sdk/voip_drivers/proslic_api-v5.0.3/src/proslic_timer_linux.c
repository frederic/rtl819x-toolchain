#include <linux/delay.h>
#include "voip_timer.h"
#include "sys_driv_type.h"

/*
** Function: SYSTEM_TimerInit
*/
void TimerInit (systemTimer_S *pTimerObj){

}

/*
** Function: SYSTEM_Delay
*/
int time_DelayWrapper (void *hTimer, int timeInMs){
	mdelay(timeInMs);
	return 0;
}


/*
** Function: SYSTEM_TimeElapsed
*/
int time_TimeElapsedWrapper (void *hTimer, timeStamp *startTime, int *timeInMs){
	unsigned long diff = timetick - startTime->time;
	*timeInMs = (int)(diff);
	return 0;
}

/*
** Function: SYSTEM_GetTime
*/
int time_GetTimeWrapper (void *hTimer, timeStamp *time){
	time->time = timetick;
	return 0;
}

