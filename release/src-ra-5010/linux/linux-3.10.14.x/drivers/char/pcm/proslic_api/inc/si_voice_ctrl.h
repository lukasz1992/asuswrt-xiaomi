/*
** Copyright (c) 2007-2010 by Silicon Laboratories
**
** $Id: si_voice_ctrl.h 3411 2012-04-04 23:11:51Z nizajerk $
**
** si_voice_ctrl.h
** SPI driver header file
**
** Author(s): 
** laj
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** File Description:
** This is the header file for the control driver used 
** in the ProSLIC demonstration code.
**
** Dependancies:
** si_voice_datatypes.h 
**
*/
#ifndef CTRL_H
#define CTRL_H

/** @defgroup PROSLIC_CUSTOMER_APIS Customer implemented functions
 * This section has documentation related to APIs that the customer
 * needs to implement to allow the ProSLIC API to function correctly.
 *
 * @note Please note, in addition to the functions mentioned here, the
 * customer will need to review/modify si_voice_datatypes.h to match
 * the native datatypes of their system.
 *
 * @{
 */
/*****************************************************************************/
/** @defgroup PROSLIC_CUSTOMER_CONTROL Customer implemented control functions
 * This group of function pointer prototypes need to be implemented by the 
 * customer in order for the ProSLIC API to function correctly.  These functions
 * need to be associated with the API by the functions documented in @ref SIVOICE_IO
 * @{
 */

/**
* @brief
* Sets/clears the reset pin of all the ProSLICs/VDAAs 
*
* @param[in] *hCtrl - which interface to reset (this is a customer supplied structure)
* @param[in] in_reset -  0 = Take the device out of reset, 1 = place the device(s) in reset
* @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
*/

typedef int (*ctrl_Reset_fptr) (void *hCtrl, int in_reset);

/**
* @brief 
* Register write function pointer
*
* @param[in] *hCtrl - which interface to communicate through (this is a customer supplied structure)
* @param[in] channel - ProSLIC channel to write to (this is the value passed is the same as found in @ref SiVoice_SWInitChan)
* @param[in] regAddr -  Address of register to write
* @param[in] data - data to write to register
* @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
*
*/

typedef int (*ctrl_WriteRegister_fptr) (void *hCtrl, uInt8 channel, uInt8 regAddr, uInt8 data);

/**
* @brief 
* RAM write function pointer
*
* @param[in] *hCtrl - which interface to communicate through (this is a customer supplied structure)
* @param[in] channel - ProSLIC channel to write to (this is the value passed is the same as found in @ref SiVoice_SWInitChan)
* @param[in] ramAddr - Address of the RAM location to write
* @param[in] ramData - data to write to the RAM location
* @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
*
*/

typedef int (*ctrl_WriteRAM_fptr) (void *hCtrl, uInt8 channel, uInt16 ramAddr, ramData data);

/**
* @brief 
* Register read function pointer
*
* @param[in] *hCtrl - which interface to communicate through (this is a customer supplied structure)
* @param[in] channel - ProSLIC channel to read from (this is the value passed is the same as found in @ref SiVoice_SWInitChan)
* @param[in] regAddr -  Address of register to read from
* @retval uInt8 - the value read. If an error occurs, this value is undefined.
*/

typedef uInt8 (*ctrl_ReadRegister_fptr) (void *hCtrl, uInt8 channel, uInt8 regAddr);

/**
* @brief 
* RAM read function pointer
*
* @param[in] *hCtrl - which interface to communicate through (this is a customer supplied structure)
* @param[in] channel - ProSLIC channel to read from (this is the value passed is the same as found in @ref SiVoice_SWInitChan)
* @param[in] ramAddr - Address of the RAM location to read from
* @retval ramData - the value read. If an error occurs, this value is undefined.
*/

typedef ramData (*ctrl_ReadRAM_fptr) (void *hCtrl, uInt8 channel, uInt16 ramAddr);

/**
 * @brief
 *  Critical Section/Semaphore function pointer
 *
* @param[in] *hCtrl - which interface to communicate through (this is a customer supplied structure)
* @param[in] in_critical_section - request to lock the critical section 1 = lock, 0 = unlock
* @retval 1 = success, 0 = failed
*/

typedef int (*ctrl_Semaphore_fptr) (void *hCtrl, int in_critical_section);

/** @} PROSLIC_CUSTOMER_CONTROL */
/** @} */
#endif


