/*
** Copyright (c) 2011 by Silicon Laboratories
**
** proslic_tstin.h
**
** Author(s): 
** cdp
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** This file contains proprietary information.	 
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
** File Description:
** This is the header file for the inward test implementations.
**
** Dependancies:
** si_voice_datatypes.h. proslic.h
**
*/
#ifndef PROSLIC_TSTIN_H
#define PROSLIC_TSTIN_H

#include "si_voice_datatypes.h"
#include "proslic.h"

/** @addtogroup DIAGNOSTICS
 * @{
 */
/** @defgroup PROSLIC_TSTIN ProSLIC Inward Tests (Test-In)
 * This section documents functions and datastructures related to the 
 * ProSLIC/FXS inward test implementations.
 * @{
 */

#define MIN_RINGING_SAMPLES 16          /**< Minimum number of ringing samples for measuring ring amplitude */
#define MAX_RINGING_SAMPLES 255         /**< Maximum number of ringing samples for measuring ring amplitude */
#define MIN_RINGING_SAMPLE_INTERVAL 1   /**< Minimum sample interval for measuring ring amplitude */
#define MAX_RINGING_SAMPLE_INTERVAL 10  /**< Maximum sample interval for measuring ring amplitude */
 
/**
* Test-In results status
*/
enum {
    TSTIN_RESULTS_INVALID,
    TSTIN_RESULTS_VALID
};

/**
* PCM format options 
*/
enum {
    PCM_8BIT,
	PCM_16BIT
};

/**
* Abort if Line-in-use option
*/
enum {
	ABORT_LIU_DISABLED,
	ABORT_LIU_ENABLED
};

/**
* Check for loop closure option
*/
enum {
	LCR_CHECK_DISABLED,
	LCR_CHECK_ENABLED
};

/**
* Check for ringtrip option
*/
enum {
	RTP_CHECK_DISABLED,
	RTP_CHECK_ENABLED
};


/**
 * Defines generic test limit/value/status structure
 */
typedef struct {
	int32			lowerLimit;     /**< Lower test limit */
	int32			upperLimit;     /**< Upper test limit */
	int32			value;          /**< Numeric test result */
	uInt8			testResult;		/**< 0 - Fail, 1 - pass */
}proslicTestObj;

/** 
 * Defines structure for PCM Loopback Test
 */
typedef struct {
	BOOLEAN			testEnable;		/**< Gate execution/updating of results with this flag */
	BOOLEAN			pcmLpbkEnabled;	/**< Indicates if test data is valid (1) or stale (0) */
	BOOLEAN			pcm8BitLinear;  /**< Set to use 8 bit linear mode (used if normally using ulaw or alaw) */
	uInt8           pcmModeSave;    /**< Store entry PCMMODE value  */
	uInt8			testResult;		/**< OR of all test results in this structure */
}proslicPcmLpbkTest;


/** 
 * Defines structure for DC Feed Test
 */
typedef struct {
	BOOLEAN			testEnable;		    /**< Gate execution/updating of results with this flag */
	BOOLEAN			testDataValid;	    /**< Indicates if test data is valid (1) or stale (0) */
    BOOLEAN			abortIfLineInUse;   /**< Abort test if LCR set at the start of test. Leaves results invalid */
	BOOLEAN			applyLcrThresh;     /**< Apply alternate LCR thresholds to ensure LCR event occurs */
    uInt32          altLcrOffThresh;    /**< Optional LCROFFHK threshold to apply during test */
	uInt32			altLcrOnThresh;     /**< Optional LCRONHK threshold to apply during test */
	BOOLEAN			lcrStatus;		    /**< Indicates LCR status after applying test load */
	proslicTestObj	dcfeedVtipOnhook;   /**< On-hook VTIP test results */
	proslicTestObj	dcfeedVringOnhook;  /**< On-hook VRING test results */
	proslicTestObj	dcfeedVloopOnhook;  /**< On-hook VLOOP test results */
	proslicTestObj	dcfeedVbatOnhook;   /**< On-hook VBAT test results */
	proslicTestObj	dcfeedItipOnhook;   /**< On-hook ITIP test results */
	proslicTestObj	dcfeedIringOnhook;  /**< On-hook IRING test results */
	proslicTestObj	dcfeedIloopOnhook;  /**< On-hook ILOOP test results */
	proslicTestObj	dcfeedIlongOnhook;  /**< On-hook ILONG test results */
	proslicTestObj	dcfeedVtipOffhook;  /**< Off-hook VTIP test results */
	proslicTestObj	dcfeedVringOffhook; /**< Off-hook VRING test results */
	proslicTestObj	dcfeedVloopOffhook; /**< Off-hook VLOOP test results */
	proslicTestObj	dcfeedVbatOffhook;  /**< Off-hook VBAT test results */
	proslicTestObj	dcfeedItipOffhook;  /**< Off-hook ITIP test results */
	proslicTestObj	dcfeedIringOffhook; /**< Off-hook IRING test results */
	proslicTestObj	dcfeedIloopOffhook; /**< Off-hook ILOOP test results */
	proslicTestObj	dcfeedIlongOffhook; /**< Off-hook ILONG test results */
	uInt8			testResult;		    /**< OR of all test results in this structure */
}proslicDcFeedTest;  


/** 
 * Defines structure for Ringing Test
 */
typedef struct {
	BOOLEAN			testEnable;		    /**< Gate execution/updating of results with this flag */
	BOOLEAN			testDataValid;	    /**< Indicates if test data is valid (1) or stale (0) */
	BOOLEAN			abortIfLineInUse;   /**< Abort test if LCR set at the start of test. Leaves results invalid */
	uInt16			numSamples;         /**< Number of samples taken */
	uInt8           sampleInterval;     /**< Sample interval (in ms - range 1 to 100) */
	BOOLEAN			ringtripTestEnable; /**< Enable ringtrip test */
	BOOLEAN			rtpStatus;          /**< RTP Bit */
	proslicTestObj	ringingVac;         /**< Ringing AC Voltage test results */
	proslicTestObj	ringingVdc;         /**< Ringing DC Voltage test results */
	uInt8			testResult;		    /**< OR of all test results in this structure */
}proslicRingingTest;

/** 
 * Defines structure for Battery Test
 */
typedef struct {
	BOOLEAN			testEnable;		/**< Gate execution/updating of results with this flag */
	BOOLEAN			testDataValid;	/**< Indicates if test data is valid (1) or stale (0) */
	proslicTestObj	vbat;           /**< VBAT test results */
	uInt8			testResult;		/**< OR of all test results in this structure */
}proslicBatteryTest;

/** 
 * Defines structure for Audio Test
 */
typedef struct {
	BOOLEAN			testEnable;		  /**< Gate execution/updating of results with this flag */
	BOOLEAN			testDataValid;	  /**< Indicates if test data is valid (1) or stale (0) */
	BOOLEAN			abortIfLineInUse; /**< Abort test if LCR set at the start of test. Leaves results invalid */
	int32           zerodBm_mVpk;     /**< 0dBm voltage (in mVpk) of ref impedance */
	proslicTestObj	txGain;		      /**< TX path gain test results */
	proslicTestObj  rxGain;           /**< RX path gain test results */
	uInt8			testResult;		  /**< OR of all test results in this structure */
}proslicAudioTest;


/** 
 * Defines structure for all tests
 */
typedef struct {
	proslicPcmLpbkTest	pcmLpbkTest;        
	proslicDcFeedTest	dcFeedTest;
	proslicRingingTest	ringingTest;
	proslicBatteryTest	batteryTest;
	proslicAudioTest	audioTest;
}proslicTestInObjType;

typedef proslicTestInObjType *proslicTestInObjType_ptr;


/**
 * @brief
 *  Allocate memory and initialize the given structure.
 *
 * @param[in,out] *pTstin - the structure to initialize
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */
int ProSLIC_createTestInObj(proslicTestInObjType_ptr *pTstin);

/**
 * @brief
 *  Free memory reserved by the given structure.
 *
 * @param[in,out] *pTstin - the structure to initialize
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */
int ProSLIC_destroyTestInObj(proslicTestInObjType_ptr *pTstin);

/**
 * @brief
 *  Enable PCM loopback.
 *
 * @param[in]      pProslic  -  channel data structure
 * @param[in,out]  pTstin->pcmLpbkTest - all control, limits, and results
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */
int ProSLIC_testInPCMLpbkEnable(proslicChanType_ptr pProslic, proslicTestInObjType_ptr pTstin);


/**
 * @brief
 *  Disable PCM loopback.
 *
 * @param[in]      pProslic  -  channel data structure
 * @param[in,out]  pTstin->pcmLpbkTest - all control, limits, and results
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */
int ProSLIC_testInPCMLpbkDisable(proslicChanType_ptr pProslic, proslicTestInObjType_ptr pTstin);


/**
 * @brief
 * DC Feed Test
 *
 * @param[in]      pProslic  -  channel data structure
 * @param[in,out]  pTstin->dcFeedTest - all control, limits, and results
 *
 * @retval int - error from @ref errorCodeType  
 * @retval RC_TEST_PASSED indicates test completed and passed
 * @retval RC_TEST_FAILED indicates test completed and failed
 * @retval RC_UNSUPPORTED_FEATURE indicates feature not supported on this device
 * @retval RC_TEST_DISABLED indicates test has not been initialized/enabled
 * @retval RC_LINE_IN_USE indicates LCS already set 
 *
 */
int ProSLIC_testInDCFeed(proslicChanType_ptr pProslic, proslicTestInObjType_ptr pTstin);


/**
 * @brief
 * Ringing and Ringtrip Test
 *
 * @param[in]      pProslic  -  channel data structure
 * @param[in,out]  pTstin->ringingTest - all control, limits, and results
 *
 * @retval int - error from @ref errorCodeType 
 * @retval RC_TEST_PASSED indicates test completed and passed
 * @retval RC_TEST_FAILED indicates test completed and failed
 * @retval RC_UNSUPPORTED_FEATURE indicates feature not supported on this device
 * @retval RC_TEST_DISABLED indicates test has not been initialized/enabled
 * @retval RC_LINE_IN_USE indicates LCS already set 
 *
 */
int ProSLIC_testInRinging(proslicChanType_ptr pProslic, proslicTestInObjType_ptr pTstin);

/**
 * @brief
 * Battery Test
 *
 * @param[in]  **pTstin                          - the structure to initialize
 * @param[out] **pTstin->batteryTest.testResult  - Pass/Fail Result
 *
 * @retval int - error from @ref errorCodeType 
 * @retval RC_TEST_PASSED indicates test completed and passed
 * @retval RC_TEST_FAILED indicates test completed and failed
 * @retval RC_UNSUPPORTED_FEATURE indicates feature not supported on this device
 * @retval RC_TEST_DISABLED indicates test has not been initialized/enabled
 *
 */
int ProSLIC_testInBattery(proslicChanType_ptr pProslic, proslicTestInObjType_ptr pTstin);

/**
 * @brief
 * Audio level inward test.
 *
 * @param[in]      pProslic  -  channel data structure
 * @param[in,out]  pTstin->audioTest - all control, limits, and results
 *
 * @retval int - error from @ref errorCodeType 
 * @retval RC_TEST_PASSED indicates test completed and passed
 * @retval RC_TEST_FAILED indicates test completed and failed
 * @retval RC_UNSUPPORTED_FEATURE indicates feature not supported on this device
 * @retval RC_TEST_DISABLED indicates test has not been initialized/enabled
 * @retval RC_LINE_IN_USE indicates LCS already set 
 *
 */
int ProSLIC_testInAudio(proslicChanType_ptr pProslic, proslicTestInObjType_ptr pTstin);

/**
 * @brief
 *  Initialize/Enable PCM Loopback Test.  Links test config/limits
 *  to inward test data structure.
 *
 * @param[in,out]  pTstin->pcmLpbkTest - all control, limits, and results
 * @param[in]      pcmLpbkTest -  test config and limits to link to pTstin->pcmLpbkTest
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */
int ProSLIC_testInPcmLpbkSetup(proslicTestInObjType *pTstin, proslicPcmLpbkTest *pcmLpbkTest);

/**
 * @brief
 *  Initialize/Enable DC Feed Test.  Links test config/limits
 *  to inward test data structure.
 *
 * @param[in,out]  pTstin->dcFeedTest - all control, limits, and results
 * @param[in]      dcFeedTest -  test config and limits to link to pTstin->dcFeedTest
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */
int ProSLIC_testInDcFeedSetup(proslicTestInObjType *pTstin,proslicDcFeedTest *dcFeedTest);

/**
 * @brief
 *  Initialize/Enable Ringing Test.  Links test config/limits
 *  to inward test data structure.
 *
 * @param[in,out]  pTstin->ringingTest - all control, limits, and results
 * @param[in]      ringingTest -  test config and limits to link to pTstin->ringingTest
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */
int ProSLIC_testInRingingSetup(proslicTestInObjType *pTstin, proslicRingingTest *ringingTest);

/**
 * @brief
 *  Initialize/Enable Battery Test.  Links test config/limits
 *  to inward test data structure.
 *
 * @param[in,out]  pTstin->batteryTest - all control, limits, and results
 * @param[in]      batteryTest -  test config and limits to link to pTstin->batteryTest
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */
int ProSLIC_testInBatterySetup(proslicTestInObjType *pTstin, proslicBatteryTest *batteryTest);

/**
 * @brief
 *  Initialize/Enable Audio Test.  Links test config/limits
 *  to inward test data structure.
 *
 * @param[in,out]  pTstin->audioTest - all control, limits, and results
 * @param[in]      audioTest -  test config and limits to link to pTstin->audioTest
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */
int ProSLIC_testInAudioSetup(proslicTestInObjType *pTstin, proslicAudioTest *audioTest);


/**
 * @brief
 *  Debug utility to log presently loaded test limits
 *
 * @param[in]      pProslic  -  channel data structure
 * @param[in]      pTstin - inward test data structure
 *
 * @retval int - error from @ref errorCodeType  
 * @retval RC_NONE indicates no error.
 * @retval RC_UNSUPPORTED_FEATURE indicates feature not supported on this device
 */
int ProSLIC_testInPrintLimits(proslicChanType *pProslic,proslicTestInObjType *pTstin);

/** @} PROSLIC_TSTIN */
/** @} DIAGNOSTICS */
#endif
