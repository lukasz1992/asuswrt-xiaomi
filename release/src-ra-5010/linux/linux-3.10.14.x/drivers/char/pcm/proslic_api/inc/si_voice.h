/*
** Copyright (c) 2007-2010 by Silicon Laboratories
**
** $Id: si_voice.h 3810 2013-01-29 21:43:33Z cdp $
**
** si_voice.h
**
** Author(s): 
** laj
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** This file contains proprietary information.	 
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
** File Description:
** This is the header file for the ProSLIC driver.
**
** Dependancies:
** si_voice_datatypes.h
**
*/

#ifndef SI_VOICE_H
#define SI_VOICE_H

#include "proslic_api_config.h"
#include "si_voice_ctrl.h"
#include "si_voice_timer_intf.h"



/** @defgroup SIVOICE SiVoice definitions

 * @{
 */
/*****************************************************************************/
/** @defgroup CHIPFAM Chip family definitions 
 * @{
 */
#define SI321X_TYPE 0    /**< Single channel legacy chipsets - SI3210, SI3215, SI3216 */
#define SI324X_TYPE 1    /**< Quad channel chipset */
#define SI3220_TYPE 2    /**< Dual Channel chipset */
#define SI3226_TYPE 3    /**< Dual channel chipset */
#define SI3217X_TYPE 4   /**< Single channel chipset*/
#define SI3226X_TYPE 5   /**< Dual channel chipset */
#define SI3050_TYPE 20   /**< Single channel FXO chipset */

/** @} CHIPFAM */
/*****************************************************************************/


/* 
* Workaroud for building on windows systems
*/

#ifdef RC_NONE
#undef RC_NONE
#endif

#define SIVOICE_RC_NONE RC_NONE

/**
* This is the main Silicon Labs control interface object. do not access directly!
*/
typedef struct	{
		void *hCtrl;                                /**< user provided SPI/GCI structure */
		ctrl_Reset_fptr Reset_fptr;                 /**< user provided reset function */
		ctrl_WriteRegister_fptr WriteRegister_fptr; /**< user provided SPI/GCI write register function */
		ctrl_ReadRegister_fptr ReadRegister_fptr;   /**< user provided SPI/GI read register function */
		ctrl_WriteRAM_fptr WriteRAM_fptr;           /**< ProSLIC only - user provided SPI/GCI read RAM/Indirect register function */
		ctrl_ReadRAM_fptr ReadRAM_fptr;             /**< ProSLIC only */
		ctrl_Semaphore_fptr Semaphore_fptr;         /**< ProSLIC only */
		void *hTimer;                               /**< user provided Timer data structure */
		system_delay_fptr Delay_fptr;               /**< user supplied mSec delay function */
		system_timeElapsed_fptr timeElapsed_fptr;   /**< user provided mSec time elapsed function */
		system_getTime_fptr getTime_fptr;           /**< user provided timestamp function */
		int usermodeStatus;                         /**< ProSLIC only */
} SiVoiceControlInterfaceType;


typedef enum {
	SI3210, 
	SI3215,
	SI3216,
	SI3211, 	
	SI3212,
 	SI3210M,
 	SI3215M,
 	SI3216M,
	SI3240,
	SI3241,
	SI3242,
	SI3243,
	SI3245,
	SI3244,
	SI3246,
	SI3247,
	SI3220,
	SI3225,
	SI3226,
	SI3227,
	SI32171,
	SI32172,
	SI32174,
	SI32175,
	SI32176,
	SI32177,
	SI32178,
	SI32179,
    SI32260,
    SI32261,
    SI32262,
    SI32263,
    SI32264,
    SI32265,
    SI32266,
    SI32267,
    SI32268,
    SI32269,
    SI32360,
    SI32361,
    SI3050 = 100,
	UNSUPPORTED_PART_NUM = 255
}partNumberType;

/**
* Chip revision definition for easier readability in the source code
*/
typedef enum {
	A,
	B,
	C,
	D,
	E,
	F,
	G
}revisionType ;


typedef enum {
    UNKNOWN, /**< Channel type has not been initalized or is still unknown */
	PROSLIC, /**< Channel type is a ProSLIC/FXS */
	DAA      /**< Channel type is a DAA/FXO */
} channelTypeType;

/**
* These are the error codes for ProSLIC failures
*/
typedef enum {
    RC_IGNORE = 0,         
    RC_NONE= 0,           /**< Means the function did not encounter an error */
	RC_TEST_PASSED = 0,
	RC_TEST_FAILED = 1,
    RC_COMPLETE_NO_ERR = 1,   /**< A test completed, no error detected */
    RC_POWER_ALARM_Q1,
    RC_POWER_ALARM_Q2,
    RC_POWER_ALARM_Q3,
    RC_POWER_ALARM_Q4,
    RC_POWER_ALARM_Q5,
    RC_POWER_ALARM_Q6,
    RC_SPI_FAIL,         /**< SPI Communications failure */
    RC_POWER_LEAK,
    RC_VBAT_UP_TIMEOUT,
    RC_VBAT_OUT_OF_RANGE,
    RC_VBAT_DOWN_TIMEOUT,
    RC_TG_RG_SHORT,
    RC_CM_CAL_ERR,
    RC_RING_FAIL_INT,
    RC_CAL_TIMEOUT,
    RC_PATCH_ERR,
    RC_BROADCAST_FAIL,           /**< Broadcast unavailable for requested operation */
    RC_UNSUPPORTED_FEATURE,      /**< Feature is not supported by the chipset*/
    RC_CHANNEL_TYPE_ERR,         /**< Channel type does not support called function */
    RC_GAIN_DELTA_TOO_LARGE,     /**< Requested gain delta too large */
    RC_GAIN_OUT_OF_RANGE,        /**< Gain requested exceeds range */
    RC_POWER_ALARM_HVIC,         /**< Power alarm on HVIC */
    RC_POWER_ALARM_OFFLD,        /**< Power alarm on offload transistor */
    RC_THERMAL_ALARM_HVIC,       /**< Thermal alarm detected */
    RC_NO_MEM,                   /**< Out of memory */
    RC_INVALID_GEN_PARAM,        /**< Invalid general parameter */
    RC_LINE_IN_USE,              /**< Line is in use (LCS detected) */
    RC_RING_V_LIMITED,           /**< Ringer voltage limited - signal may be clipped */
    RC_PSTN_CHECK_SINGLE_FAIL,   /**< PSTN detect single current exceeds limit */
    RC_PSTN_CHECK_AVG_FAIL,      /**< PSTN detect average current exceeds limit */
    RC_VDAA_ILOOP_OVLD,          /**< Overload detected */
    RC_UNSUPPORTED_OPTION,       /**< Function parameter is not supported at this time */
    RC_FDT_TIMEOUT,              /**< Timeout waiting for valid frame detect */
    RC_PSTN_OPEN_FEMF,           /**< Detected FEMF, device left in open state */
    RC_VDAA_PAR_HANDSET_DET,     /**< Parallel handset detected */
    RC_VDAA_PAR_HANDSET_NOT_DET, /**< Parallel handset not detected */
    RC_PATCH_RAM_VERIFY_FAIL,    /**< Patch RAM verification failure */
    RC_PATCH_ENTRY_VERIFY_FAIL,  /**< Patch entry table verification failure */ 
    RC_UNSUPPORTED_DEVICE_REV,   /**< Device revision not supported */
	RC_INVALID_PATCH,            /**< No patch for selected options */
	RC_INVALID_PRESET,           /**< Invalid Preset value */
	RC_TEST_DISABLED,            /**< Test Not enabled */
	RC_RING_START_FAIL,          /**< Ringing failed to start */
	RC_MWI_ENABLE_FAIL,          /**< Failed to enable MWI feature */
	RC_MWI_IN_USE,				 /**< MWI active and unable to be modified */
	RC_MWI_NOT_ENABLED,			 /**< MWI not enabled */
    RC_DCDC_SETUP_ERR,           /**< DCDC not properly initialized prior to powerup */
    RC_REINIT_REQUIRED = 255     /**< Soft Reset Required */
} errorCodeType;



/**********************************************************************/
/**
 * @defgroup PROLSIC_BRD_DESIGN Board design settings 
 * @{
 */
/**
*  BOM Option Tag - refer to hardware design
*/
typedef enum {
	DEFAULT,                      /**< DCDC: Unspecified */
	SI321X_HV,                    /**< DCDC: Si321x high voltage design */
    BO_DCDC_FLYBACK,              /**< DCDC: flyback design */
    BO_DCDC_QCUK,                 /**< DCDC: quasi-cuk design */
    BO_DCDC_BUCK_BOOST,           /**< DCDC: BJT buck-boost design */
    BO_DCDC_LCQCUK,               /**< DCDC: low-cost quasi-cuk design */
	BO_DCDC_P_BUCK_BOOST_5V,      /**< DCDC: PMOS buck-boost 5v design (depricated) */
	BO_DCDC_P_BUCK_BOOST_12V,     /**< DCDC: PMOS buck-boost 12v design (depricated) */
    BO_DCDC_P_BUCK_BOOST_12V_HV,  /**< DCDC: PMOS buck-boost 12v design, high voltage (depricated) */
	BO_DCDC_CUK,                  /**< DCDC: full cuk design */
	BO_DCDC_PMOS_BUCK_BOOST,      /**< DCDC: PMOS buck-boost design */
} bomOptionsType;

/**
*  VDC input voltage range option tags - please refer hardware design
*/
typedef enum
{
	VDC_3P0_6P0,
	VDC_4P5_16P0,
	VDC_4P5_27P0,
	VDC_7P0_20P0,
	VDC_8P0_16P0,
	VDC_9P0_16P0,
	VDC_9P0_24P0,
	VDC_10P8_20P0,
	VDC_27P0_42P0
} vdcRangeType;

/**
*  Battery Rail option tags - please refer hardware design
*/
typedef enum 
{
    BO_DCDC_TSS,        /**< Used for Fixed Rail DC-DC supplies */
    BO_DCDC_TRACKING,   /**< Used for Tracking DC-DC supplies */
    BO_DCDC_EXTERNAL,   /**< Used for external fixed rail supplies */
    BO_DCDC_TSS_ISO,    /**< Used for isolated TSS supplies */
    BO_DCDC_FIXED_RAIL = BO_DCDC_TSS     /**< Fixed rail replaced by TSS */
#ifndef SIVOICE_CFG_NEWTYPES_ONLY
    ,FIXED = BO_DCDC_FIXED_RAIL /**< @deprecated use BO_DCDC_FIXED_RAIL */
    ,TRACKING = BO_DCDC_TRACKING  /**< @deprecated use BO_DCDC_TRACKING */
#endif
} batRailType;

/**
*  FET Gate Driver option tags
*/
typedef enum
{
    BO_GDRV_NOT_INSTALLED = 0,
    BO_GDRV_INSTALLED = 1
}gateDriveType;

/**
*  Auto ZCAL Enable option tags
*/
typedef enum
{
    AUTO_ZCAL_DISABLED = 0,
    AUTO_ZCAL_ENABLED = 1
}autoZcalType;


/**
*  VDAA Suppport option tags
*/
typedef enum
{
    VDAA_DISABLED = 0,
    VDAA_ENABLED = 1
}vdaaSupportType;


/**
*  PM BOM Support option tags
*/
typedef enum {
    BO_STD_BOM,
    BO_PM_BOM
}pmBomType;


/**
* Linefeed part number options - which front end is used.
*/
typedef enum {
	SI3208,
	SI3209,
	SI3203,
	SI3206,
	SI3205,
	SI3201,
	SI3210_DISCRETE
} linefeed;
/** @} PROSLIC_BRD_DESIGN */

/**
* Initialization options to allow init flow and/or content to be
* altered at runtime
*/
typedef enum {
	INIT_NO_OPT,	     /**<  No initialization option */
	INIT_REINIT,         /**<  Reinitialization option */
	INIT_NO_CAL,         /**<  Skip calibration only */
	INIT_NO_PATCH_LOAD   /**<  Skip patch load */
} initOptionsType;

/**
** This is the main Voice device object
*/
typedef struct	{
		SiVoiceControlInterfaceType *ctrlInterface; /**< How do we talk to the system? User supplied functions are connected here */
		revisionType chipRev; /**< What revision is the chip? */
		partNumberType chipType; /**< What member of the particular family is this chip? */
		uInt8 lsRev; /**< DAA: what rev is the line side chip? */
		uInt8 lsType; /**< DAA: what type is the line side chip */
		int usermodeStatus; /**< Used for patch code */
} SiVoiceDeviceType;

typedef SiVoiceDeviceType *SiVoiceDeviceType_ptr; /**< Shortcut typedef */


/**
** This is the main ProSLIC channel object
*/
typedef struct	{
		SiVoiceDeviceType_ptr deviceId; /**< Information about the device associated with this channel */
		uInt8 channel; /**< Which channel is this device? This is a system parameter meaning if you have 2 dual channel devices in your system, this number would then typically go from 0-3 */
		channelTypeType channelType; /**< Is this a ProSLIC or a DAA */
		errorCodeType error;  /**< Storage for the current error state - used only for APIs that don't return back an error */
		int debugMode; /**< Are we debugging on this channel? */
		int channelEnable; /**<  is the channel enabled or not? If not, this channel is not initialized */
		bomOptionsType bomOption; /**< Device/PCB specific */
        uInt8 dcdc_polarity_invert;  /**<  Invert DCDC polarity from what is provided in general parameters */
} SiVoiceChanType;


typedef SiVoiceChanType *SiVoiceChanType_ptr;

/*****************************************************************************/
/** @defgroup SIVOICE_IF_CFG  SiVoice System control interface functions
 *
 * These functions are used by the ProSLIC API to access system resources
 * such as SPI/GCI access, memory allocation/deallocation and timers.
 *
 * @{
 */
/*****************************************************************************/
/** @defgroup SIVOICE_MEMORY_IF SiVoice Memory allocation/deallocation 
 *
 * These functions dynamically allocate and deallocate memory for the given
 * structures. malloc() and memset() are called for allocation and free() is
 * called for deallocation.  These functions are typically called during 
 * initialization of the API and at the tear down of the API.  If customers
 * prefer to use statically allocated structures, then they must ensure that
 * the strucutre elements are zero'ed out.
 * 
 * @{
 */

/**
 @brief
 *  Allocate memory and initialize the given structure.
 *
 * @param[in,out] pCtrlIntf - the structure to initialize
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */

int SiVoice_createControlInterface (SiVoiceControlInterfaceType **pCtrlIntf);

/**
 @brief
 *  Destroys the given structure and deallocates memory.
 *
 * @param[in,out] pCtrlIntf  - the structure to destroy/deallocate
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */

int SiVoice_destroyControlInterface (SiVoiceControlInterfaceType **pCtrlIntf);

/**
 @brief
 *  Allocate memory and initialize the given structure.
 *
 * @param[in,out] pDev - the structure to initialize
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */

int SiVoice_createDevice (SiVoiceDeviceType **pDev);

/**
 @brief
 *  Destroys the given structure and deallocates memory.
 *
 * @param[in,out] pDev - the structure to destroy/deallocate
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */

int SiVoice_destroyDevice (SiVoiceDeviceType **pDev);

/**
 @brief
 *  Allocate memory and initialize the given structure.
 *
 * @param[in,out] pChan - the structure to initialize
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */

int SiVoice_createChannel (SiVoiceChanType_ptr *pChan);

/**
 @brief
 *  Destroys the given structure and deallocates memory.
 *
 * @param[in,out] pChan - the structure to destroy/deallocate
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */

int SiVoice_destroyChannel (SiVoiceChanType_ptr *pChan);
/** @} SIVOICE_MEMORY_IF */

/*****************************************************************************/
/** @defgroup SIVOICE_IO SiVoice SPI/GCI access routines
 * This group of functions are used to associcate the transport mechanism (SPI, GCI) functions with the API.  The actual
 * functions being references are normally implemented by the customer for their particualr OS and platform.
 *
 * @{
 */

/**
 @brief
 *  Associate a interface object with a user supplied datastructure.  This 
 *  structure is passed to all the I/O routines that the ProSLIC API calls.
 *
 * @param[in,out] pCtrlIntf - which interface to associate 
 *                the user supplied structure with.
 * @param[in] *hCtrl - the user supplied structure that is passed to the IO functions.
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */

int SiVoice_setControlInterfaceCtrlObj (SiVoiceControlInterfaceType *pCtrlIntf, void *hCtrl);

/**
 @brief
 *  Associate a interface object with the reset function.
 *
 * @param[in,out] pCtrlIntf - which interface to associate 
 *                the user supplied function with.
 * @param[in] Reset_fptr - the reset function pointer
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */

int SiVoice_setControlInterfaceReset (SiVoiceControlInterfaceType *pCtrlIntf, ctrl_Reset_fptr Reset_fptr);

/**
 @brief
 *  Associate a interface object with the register write function.
 *
 * @param[in,out] pCtrlIntf - which interface to associate 
 *                the user supplied function with.
 * @param[in] WriteRegister_fptr - the register write function pointer
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */

int SiVoice_setControlInterfaceWriteRegister (SiVoiceControlInterfaceType *pCtrlIntf, ctrl_WriteRegister_fptr WriteRegister_fptr);

/**
 @brief
 *  Associate a interface object with the register read function.
 *
 * @param[in,out] pCtrlIntf - which interface to associate 
 *                the user supplied function with.
 * @param[in] ReadRegister_fptr- the register read function pointer
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */

int SiVoice_setControlInterfaceReadRegister (SiVoiceControlInterfaceType *pCtrlIntf, ctrl_ReadRegister_fptr ReadRegister_fptr);

/**
 @brief
 *  Associate a interface object with the write RAM function.
 *
 * @param[in,out] pCtrlIntf - which interface to associate 
 *                the user supplied function with.
 * @param[in] WriteRAM_fptr - the reset function pointer
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */

int SiVoice_setControlInterfaceWriteRAM (SiVoiceControlInterfaceType *pCtrlIntf, ctrl_WriteRAM_fptr WriteRAM_fptr);

/**
 @brief
 *  Associate a interface object with the read RAM function.
 *
 * @param[in,out] pCtrlIntf - which interface to associate 
 *                the user supplied function with.
 * @param[in]  ReadRAM_fptr - the read RAM function pointer
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */

int SiVoice_setControlInterfaceReadRAM (SiVoiceControlInterfaceType *pCtrlIntf, ctrl_ReadRAM_fptr ReadRAM_fptr);
/** @} SIVOICE_IO */

/*****************************************************************************/
/** @defgroup SIVOICE_TIMER SiVoice Timer functions 
 *
 * This group of functions associates the customer supplied timer routines with 
 * the ProSLIC API. Please note, for most applications, only the delay routine 
 * is required.  The other routines are needed for the pulse digit API.
 * @{
 */

/**
 @brief
 *  This function associates a timer object - which is user defined, but it is 
 *  used with ALL channels of the particular control interface.
 *
 *  @param[in] pCtrlIntf which control interface to associate the given timer object with.
 *  @param[in] *hTimer - the timer ojbect that is passed to all timer functions.
 *  @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 *
 *  @sa SiVoice_setControlInterfaceDelay SiVoice_setControlInterfaceTimeElapsed SiVoice_setControlInterfaceGetTime PROSLIC_TIMER
 */

int SiVoice_setControlInterfaceTimerObj (SiVoiceControlInterfaceType *pCtrlIntf, void *hTimer);

/** 
 @brief
 *  Associate a timer delay function with a given control interface.  The 
 *  delay function takes in an argument of the timer object and the time in mSec
 *  and delays the thread/task for at least the time requested.
 *
 * @param[in] pCtrlIntf - which control interface to associate the function with.
 * @param[in] Delay_fptr - the pointer to the delay function.
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 *
 *  @sa SiVoice_setControlInterfaceTimerObj SiVoice_setControlInterfaceTimeElapsed SiVoice_setControlInterfaceGetTime PROSLIC_TIMER
 */

int SiVoice_setControlInterfaceDelay (SiVoiceControlInterfaceType *pCtrlIntf, system_delay_fptr Delay_fptr);

/** 
 *  Description:
 *   Associate a time elapsed function with a given control interface.  The
 *   time elapsed function uses the values from the function specified in
 *   @ref SiVoice_setControlInterfaceGetTime and computes the delta time
 *   in mSec.
 *  @param[in] pCtrlIntf - which control interface to associate the function with.
 *  @param[in] timeElapsed_fptr - the pointer to the elapsed time function.
 *  @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 *  @sa SiVoice_setControlInterfaceTimerObj SiVoice_setControlInterfaceGetTime SiVoice_setControlInterfaceDelay PROSLIC_TIMER
 */

int SiVoice_setControlInterfaceTimeElapsed (SiVoiceControlInterfaceType *pCtrlIntf, system_timeElapsed_fptr timeElapsed_fptr);

/** 
 *  Description:
 *   Associate a time get function with a given control interface.  The
 *   time get function returns a value in a form of a void pointer that 
 *   is suitable to be used with the function specified in @ref SiVoice_setControlInterfaceTimeElapsed .
 *   This is typically used as a timestamp of when an event started. The resolution needs to be in terms
 *   of mSec.
 *
 *  @param[in] pCtrlIntf - which control interface to associate the function with.
 *  @param[in] getTime_fptr -  the pointer to the get time function.
 *  @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 *  @sa SiVoice_setControlInterfaceTimerObj SiVoice_setControlInterfaceTimeElapsed SiVoice_setControlInterfaceDelay PROSLIC_TIMER
 */

int SiVoice_setControlInterfaceGetTime (SiVoiceControlInterfaceType *pCtrlIntf, system_getTime_fptr getTime_fptr);

/** @} SIVOICE_TIMER */

/*****************************************************************************/
/** @defgroup SIVOICE_PROC_CONTROL SiVoice Process control
 * @{
 */

/**
 @brief
 *  This function assoicates a user defined semaphore/critical section
 *  function with the given interface.
 *
 * @param[in,out] pCtrlIntf the interface to assoicate the function with.
 * @param[in] semaphore_fptr - the function pointer for semaphore control.
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */

int SiVoice_setControlInterfaceSemaphore (SiVoiceControlInterfaceType *pCtrlIntf, ctrl_Semaphore_fptr semaphore_fptr);

/** @} SIVOICE_PROC_CONTROL */
/*****************************************************************************/
/** @} SIVOICE_IF_CFG*/
/*****************************************************************************/
/** @defgroup SIVOICE_INIT Initialization routines
 * @{
 */

/**
 @brief
 *  This function initializes the various channel structure elements.  
 *  This function does not access the chipset directly, so SPI/GCI
 *  does not need to be up during this function call.
 *
 * @param[in,out] hProslic - which channel to initialize.
 * @param[in] channel - Which channel index is this.  For example, for a 4 channel system, this would typically range from 0 to 3.
 * @param[in] chipType - chipset family type for example @ref SI321X_TYPE or @ref SI3217X_TYPE
 * @param[in] pDeviceObj - Device structure pointer associated with this channel
 * @param[in] pCtrlIntf - Control interface associated with this channel
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 *
 * @sa ProSLIC_SWInitChan
 */

int SiVoice_SWInitChan (SiVoiceChanType_ptr hProslic,int channel,int chipType, SiVoiceDeviceType *pDeviceObj, SiVoiceControlInterfaceType *pCtrlIntf);

/**
 @brief
 * This function calls the user supplied reset function to put and take out the channel from reset. This is
 * typically done during initialization and may be assumed to be a "global" reset - that is 1 reset per
 * daisychain vs. 1 per device.
 *
 * @note This function can take more than 500 mSec to complete.
 *
 * @param[in] pChan - which channel to reset, if a "global reset", then any channel on the daisy chain is sufficient.
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 *
 */

int SiVoice_Reset (SiVoiceChanType_ptr pChan);

/** @} SIVOICE_INIT */

/*****************************************************************************/
/** @defgroup SIVOICE_DEBUG Debug
 * @{
 */

/**
 @brief
 * This function enables or disables the debug mode, assuming @ref ENABLE_DEBUG is set in the configuration file.
 *
 * @param[in] pChan - which channel to set the debug flag.
 * @param[in] debugEn - 0 = Not set, 1 = set.
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 *
 */

int SiVoice_setSWDebugMode (SiVoiceChanType_ptr pChan, int debugEn);

/** @} SIVOICE_DEBUG */

/*****************************************************************************/
/** @defgroup SIVOICE_ERROR Return code functions
 * @{
 */

/** 
 @brief
 * This function returns the error flag that may be set by some function in where
 * @ref errorCodeType is not returned.  
 *
 * @note For functions that DO return errorCodeType, the return value here is undefined.
 *
 * @param[in] pChan - which channel to clear the error flag
 * @param[in,out] *error - The current value of error flag.  
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 *
 * @sa SiVoice_clearErrorFlag
 */

int SiVoice_getErrorFlag (SiVoiceChanType_ptr pChan, int *error);

/**
 @brief
 *  This function clears the error flag that may be set by some function in where
 *  @ref errorCodeType is not returned.
 *
 * @param[in,out] pChan - which channel to clear the error flag
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 *
 * @sa SiVoice_getErrorFlag
 */

int SiVoice_clearErrorFlag (SiVoiceChanType_ptr pChan);

/** @} SIVOICE_ERROR */

/*****************************************************************************/
/** @defgroup SIVOICE_ENABLE Enable/disable channels (for init)
 * @{
 */
/**
 @brief
 *  This function sets the channel enable status.  If NOT set, then when
 *  the various initialization routines such as @ref SiVoice_SWInitChan is called,
 *  then this particular channel will NOT be initialized.
 *
 *  This function does not access the chipset directly, so SPI/GCI
 *  does not need to be up during this function call.
 *
 * @param[in,out] pChan - which channel to return the status.
 * @param[in] chanEn - The new value of the channel enable field. 0 = NOT enabled, 1 = enabled.
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 *
 * @sa SiVoice_getChannelEnable
 */

int SiVoice_setChannelEnable (SiVoiceChanType_ptr pChan, int chanEn);

/**
 @brief
 *  This function returns back if the channel is enabled or not.
 *  This function does not access the chipset directly, so SPI/GCI
 *  does not need to be up during this function call.
 *
 * @param[in] pChan - which channel to return the status.
 * @param[in,out] chanEn* - The current value of if the channel is enabled. 0 = NOT enabled, 1 = enabled.
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 *
 * @sa SiVoice_setChannelEnable
 */

int SiVoice_getChannelEnable (SiVoiceChanType_ptr pChan, int* chanEn);

/** @} SIVOICE_ENABLE */
/*****************************************************************************/
/** @} SIVOICE */
#endif

