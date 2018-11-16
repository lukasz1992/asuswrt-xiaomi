/*
** Copyright (c) 2007 by Silicon Laboratories
**
** $Id: si_voice_timer_intf.h 3411 2012-04-04 23:11:51Z nizajerk $
**
** si_voice_timer_intf.h
** System specific functions header file
**
** Author(s): 
** laj
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** File Description:
** This is the header file for the system specific functions like timer functions.
**
**
*/


#ifndef TIMER_INTF_H
#define TIMER_INTF_H

/** @addtogroup PROSLIC_CUSTOMER_APIS
 * @{
 * @defgroup PROSLIC_CUSTOMER_TIMER Customer implemented timer functions
 * This section has documentation related to timers: delays and timestamps.  The void * is a customer
 * supplied timer data structure that the user specifies in @ref SiVoice_setControlInterfaceTimerObj. 
 * These functions need to be associated with the functions documented in @ref SIVOICE_TIMER
 *
 * @note For the majority of implementations, the only function that is required is the delay function.
 *
 * @{
 */

/** 
 * @brief
 * System time delay function pointer 
 * 
 * @param[in] hTimer - the system timer object
 * @param[in] timeInMS - number of mSec to suspend the task/thread executing.
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
*/

typedef int (*system_delay_fptr) (void *hTimer, int timeInMs);

/**
 * @brief
 * System time elapsed function pointer
 *
 * @details
 * This function combined with @ref system_getTime_fptr function allow one to determine
 * the time elapsed between the two event times.
 *
 * The example psuedo code below shows how one may see this function used in conjunction with
 * the function pointed by @ref system_getTime_fptr to do some processing while waiting
 * for a time to expire.
 *
 * @code
 * my_timer_obj time_now;
 *
 * getTime(my_prosclic_obj->timer_obj_ptr, (void *)(&time_now));
 * 
 *  Some events occur here...
 * 
 * do
 * {
 *     time_elapsed(my_proslic_obj->timer_obj_ptr,(void *)(&time_now), &elapsed_time_mSec);
 *     
 *     Do something here...
 *
 *  }while(elapsed_time_mSec < Desired delay);
 *
 * Change state...
 *
 * @endcode
 *
 * @param[in] *hTimer - the system timer object
 * @param[in] *startTime - the time object returned by @ref system_getTime_fptr - the "start time" of the event.
 * @param[out] *timeInMs - the time in mSec between the "start time" and the current time.
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 * @sa system_getTime_fptr
*/

typedef int (*system_timeElapsed_fptr) (void *hTimer, void *startTime, int *timeInMs);

/**
 * @brief
 * Retrieve system time in mSec resolution.
 *
 * @details
 * This function combined with @ref system_timeElapsed_fptr function allow one to determine
 * the time elapsed between the two event times.
 *
 * @param[in] *hTimer - the system timer object
 * @param[in] *time - the timestamp object needed by @ref system_timeElapsed_fptr
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 * @sa system_timeElapsed_fptr 
*/

typedef int (*system_getTime_fptr) (void *hTimer, void *time);

/** @} 
 * @} */
#endif
