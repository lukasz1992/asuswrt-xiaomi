/*
** $Id: proslic_timer_intf_win.c 3735 2013-01-08 19:58:25Z nizajerk $
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
#include "si_voice_datatypes.h"
#include "si_voice_timer_intf.h"
#include "time.h"
#include "timer.h"
#include "proslic.h"

/*
** First we define the local functions
*/
static _int64 readTSC () //read precision timer register from PC
{
#ifndef __GNUC__ 
	union {
		_int64 extralong;
		unsigned long longish[2];
	} t;
	unsigned long a,b;
	_asm {
	_emit 0x0f;
	_emit 0x31;
	mov a,eax;
	mov b,edx;
	}
	t.longish[0]=a;t.longish[1]=b;
	return t.extralong;
#else
	register _int64 tsc asm("eax");
   	asm volatile (".byte 15, 49" : : : "eax", "edx");
   	return tsc;
#endif
}

static void sleep( uInt32 wait ) //inaccurate sleep to gauge PC speed in timerInit
{
	uInt32 goal;
	goal = wait + clock();
	while( goal > (uInt32)clock() );
}

/*
** These are the global functions
*/

/*
** Function: SYSTEM_TimerInit
*/
void TimerInit (systemTimer_S *pTimerObj){
	static _int64 ticks_per_second = 0;
	
	if(ticks_per_second == 0)
	{
		_int64 time0, time1;
		sleep(1);
		time0= readTSC();
		sleep (1800);
		time1 = readTSC();
		ticks_per_second = ((time1-time0)/1800000)*1000000;
	}

	pTimerObj->ticksPerSecond= ticks_per_second;
}


/*
** Function: SYSTEM_Delay
*/
int time_DelayWrapper (void *hTimer, int timeInMs){
	_int64 target = readTSC() + (((systemTimer_S *)(hTimer))->ticksPerSecond * timeInMs ) /1000 ;
	while (readTSC() < target) ;
	return 0;
}

/*
** Function: SYSTEM_TimeElapsed
*/
int time_TimeElapsedWrapper (void *hTimer, void *startTime, int *timeInMs){
	_int64 diff = readTSC() - ((timeStamp *)startTime)->time;
	*timeInMs = (int)(diff / (((systemTimer_S *)hTimer)->ticksPerSecond/1000));
	return 0;
}

/*
** Function: SYSTEM_GetTime
*/
int time_GetTimeWrapper (void *hTimer, void *time){
    SILABS_UNREFERENCED_PARAMETER(hTimer);
	((timeStamp *)time)->time = readTSC();
	return 0;
}

/*
** $Log: proslic_timer_intf_win.c,v $
** Revision 1.5  2008/07/24 21:06:16  lajordan
** no message
**
** Revision 1.4  2007/03/22 18:53:43  lajordan
** fixed warningg
**
** Revision 1.3  2007/02/26 16:46:16  lajordan
** cleaned up some warnings
**
** Revision 1.2  2007/02/16 23:55:07  lajordan
** no message
**
** Revision 1.1.1.1  2006/07/13 20:26:08  lajordan
** no message
**
** Revision 1.1  2006/07/07 21:38:56  lajordan
** no message
**
** Revision 1.1.1.1  2006/07/06 22:06:23  lajordan
** no message
**
** Revision 1.1  2006/06/29 19:17:21  laj
** no message
**
** Revision 1.1  2006/06/21 22:42:26  laj
** new api style
**
**
*/
