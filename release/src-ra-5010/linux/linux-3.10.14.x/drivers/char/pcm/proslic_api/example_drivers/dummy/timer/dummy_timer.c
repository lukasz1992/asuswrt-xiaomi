/*
** $Id: dummy_timer.c 3411 2012-04-04 23:11:51Z nizajerk $
**
** system.c
** System specific functions implementation file
**
** Author(s): 
** laj
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** File Description:
** This is the implementation file for the system specific functions like timer functions.
**
** Dependancies:
** datatypes.h
**
*/
#include "proslic_datatypes.h"
#include "proslic_timer_intf.h"
#include "dummy_timer.h"

/*
** Function: SYSTEM_TimerInit
*/
void TimerInit (systemTimer_S *pTimerObj){

}


/*
** Function: SYSTEM_Delay
*/
int time_DelayWrapper (void *hTimer, int timeInMs){

	return 0;
}


/*
** Function: SYSTEM_TimeElapsed
*/
int time_TimeElapsedWrapper (void *hTimer, void *startTime, int *timeInMs){	*timeInMs = 1000;
	return 0;
}

/*
** Function: SYSTEM_GetTime
*/
int time_GetTimeWrapper (void *hTimer, void *time){
//	time->timestamp=0;
	return 0;
}

/*
** $Log: dummy_timer.c,v $
** Revision 1.3  2008/03/13 18:40:03  lajordan
** fixed for si3226
**
** Revision 1.2  2007/10/22 21:38:31  lajordan
** fixed some warnings
**
** Revision 1.1  2007/10/22 20:49:21  lajordan
** no message
**
**
*/

