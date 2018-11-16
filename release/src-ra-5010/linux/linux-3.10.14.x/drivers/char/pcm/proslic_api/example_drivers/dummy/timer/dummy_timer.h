/*
** $Id: dummy_timer.h 3411 2012-04-04 23:11:51Z nizajerk $
** 
** This file is system specific and should be edited for your hardware platform
**
** 
*/
#ifndef TIME_TYPE_H
#define TIME_TYPE_H



/*
** System timer interface structure 
*/
typedef struct{
	int timerInfo;
} systemTimer_S;

/*
** System time stamp
*/
typedef struct{
	int timestamp;
} timeStamp;

/*
** Function: SYSTEM_TimerInit
**
** Description: 
** Initializes the timer used in the delay and time measurement functions
** by doing a long inaccurate sleep and counting the ticks
**
** Input Parameters: 
**
** Return:
** none
*/
void TimerInit (systemTimer_S *pTimerObj);
/*
** Function: DelayWrapper
**
** Description: 
** Waits the specified number of ms
**
** Input Parameters: 
** hTimer: timer pointer
** timeInMs: time in ms to wait
**
** Return:
** none
*/
int time_DelayWrapper (void *hTimer, int timeInMs);


/*
** Function: TimeElapsedWrapper
**
** Description: 
** Calculates number of ms that have elapsed
**
** Input Parameters: 
** hTImer: pointer to timer object
** startTime: timer value when function last called
** timeInMs: pointer to time elapsed
**
** Return:
** timeInMs: time elapsed since start time
*/
int time_TimeElapsedWrapper (void *hTimer, void *startTime, int *timeInMs);

int time_GetTimeWrapper (void *hTimer, void *time);
#endif
/*
** $Log: dummy_timer.h,v $
** Revision 1.1  2007/10/22 21:38:32  lajordan
** fixed some warnings
**
** Revision 1.1  2007/10/22 20:49:21  lajordan
** no message
**
**
*/
