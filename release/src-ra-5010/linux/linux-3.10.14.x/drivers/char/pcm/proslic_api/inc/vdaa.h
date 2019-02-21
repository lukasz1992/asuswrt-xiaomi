/*
** Copyright (c) 2008-2010 by Silicon Laboratories
**
** $Id: vdaa.h 3411 2012-04-04 23:11:51Z nizajerk $
**
** Vdaa.h
** Vdaa  VoiceDAA interface header file
**
** Author(s): 
** naqamar, laj
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** This file contains proprietary information.	 
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
** File Description:
** This is the header file for the main  VoiceDAA API and is used 
** in the  VoiceDAA demonstration code. 
**
** Dependancies:
** Customer Drivers
**
*/
#ifndef VDAA_INTF_H
#define VDAA_INTF_H

#include "si_voice_datatypes.h"
#include "si_voice.h"
/*****************************************************************************/
/** @defgroup VDAA_API VDAA API
 * This section covers APIs and definitions related to the VDAA/FXO.
 * @{
 */

/*
** Constants
*/
#define BROADCAST 0xff
#define LCS_SCALE_NUM 33		/* Line Current Status Scale */
#define LCS_SCALE_DEN 10		/* Line Current Status Scale */


/*
**
** VDAA Initialization/Configuration Parameter Options
**
*/
/*
** This defines names for the PCM Data Format
*/
 typedef enum {
	A_LAW = 0,	/**< 00 = A-Law. Signed magnitude data format */
	U_LAW = 1,	/**< 01 = u-Law. Signed magnitude data format */
	LINEAR_16_BIT = 3 /**< 11 = 16-bit linear (2s complement data format) */
}tPcmFormat;

/*
** This defines names for the phcf bits
*/
 typedef enum {
	PCLK_1_PER_BIT = 0,
	PCLK_2_PER_BIT = 1
}tPHCF;


/*
** This defines names for the tri bit
*/
 typedef enum {
	TRI_POS_EDGE = 0,
	TRI_NEG_EDGE = 1
}tTRI;


/**
** This defines names for the AC impedance range
*/
 typedef enum {
	AC_600,				/**< 600 Ohms */
	AC_900,				/**< 900 O */
	AC_270__750_150,		/**< 270 O + (750 O || 150 nF) and 275 O + (780 O || 150 nF) */
	AC_220__820_120,		/**< 220 O + (820 O || 120 nF) and 220 O + (820 O || 115 nF) */
	AC_370__620_310,		/**< 370 O + (620 O || 310 nF) */
	AC_320__1050_230,		/**< 320 O + (1050 O || 230 nF) */
	AC_370__820_110,		/**< 370 O + (820 O || 110 nF) */
	AC_275__780_115,		/**< 275 O + (780 O || 115 nF) */
	AC_120__820_110,		/**< 120 O + (820 O || 110 nF) */
	AC_350__1000_210,		/**< 350 O + (1000 O || 210 nF) */
	AC_200__680_100,		/**< 200 O + (680 O || 100 nF) */
	AC_600__2160,			/**< 600 O + 2.16 uF */
	AC_900__1000,			/**< 900 O + 1 uF */
	AC_900__2160,			/**< 900 O + 2.16 uF */
	AC_600__1000,			/**< 600 O + 1 uF */
	AC_Global_impedance		/**< Global impedance */
}tAC_Z;

/**
** This defines names for the DC impedance range
*/
 typedef enum {
	 DC_50,					/**< 50 Ohms dc termination is selected */
	 DC_800					/**< 800 Ohms dc termination is selected */

 }tDC_Z;

/**
** This defines names for the ringer impedance range
*/
 typedef enum {
	 RZ_MAX = 0,					
	 RZ_SYNTH = 1					
 }tRZ;

/**
** This defines names for the dc voltage adjust
*/
 typedef enum {
	 DCV3_1 = 0,
	 DCV3_2 = 1,
	 DCV3_35 = 2,
	 DCV3_5 = 3
 }tDCV;

/**
** This defines names for the minimum loop current
*/
 typedef enum {
	 MINI_10MA = 0,
	 MINI_12MA = 1,
	 MINI_14MA = 2,
	 MINI_16MA = 3
 }tMINI;

/**
** This defines names for the current limiting enable bit
*/
 typedef enum {
	 ILIM_DISABLED = 0,
	 ILIM_ENABLED = 1
 }tILIM;

/**
** This defines names for the ring detect interupt mode
*/
 typedef enum {
	 RDI_BEG_BURST = 0,
	 RDI_BEG_END_BURST = 1
 }tRDI;

/**
** This defines names for the on hook speed / spark quenching
*/
 typedef enum {
	 OHS_LESS_THAN_0_5MS = 0,
	 OHS_3MS = 1,
	 OHS_26MS = 0xE
 }tOHS;

 /**
** This defines names for the hbe bit
*/
 typedef enum {
	HYBRID_DISABLED = 0,
	HYBRID_ENABLED  = 1
 }tHBE;


/**
** Gain/Attenuation Select
*/
typedef enum {
    XGA_GAIN,
    XGA_ATTEN
}tXGA;

/**
** MUTE Control Options
*/
typedef enum {
    MUTE_DISABLE_ALL,
    MUTE_DISABLE_RX,
    MUTE_DISABLE_TX,
    MUTE_ENABLE_RX,
    MUTE_ENABLE_TX,
    MUTE_ENABLE_ALL
}tMUTE;

/**
** This defines names for the ring delay setting
*/
 typedef enum {
	 RDLY_0MS = 0,
	 RDLY_256MS = 1,
	 RDLY_512MS = 2,
	 RDLY_768MS = 3,
	 RDLY_1024MS = 4,
	 RDLY_1280MS = 5,
	 RDLY_1536MS = 6,
	 RDLY_1792MS = 7
 }tRDLY;

/**
** This defines names for the ring timeouts
*/
 typedef enum {
	 RTO_128MS = 1,
	 RTO_256MS = 2,
	 RTO_384MS = 3,
	 RTO_512MS = 4,
	 RTO_640MS = 5,
	 RTO_768MS = 6,
	 RTO_896MS = 7,
	 RTO_1024MS = 8,
	 RTO_1152MS = 9,
	 RTO_1280MS = 10,
	 RTO_1408MS = 11,
	 RTO_1536MS = 12,
	 RTO_1664MS = 13,
	 RTO_1792MS = 14,
	 RTO_1920MS = 15
 }tRTO;

 /**
** This defines names for the ring timeouts
*/
 typedef enum {
	 RCC_100MS = 0,
	 RCC_150MS = 1,
	 RCC_200MS = 2,
	 RCC_256MS = 3,
	 RCC_384MS = 4,
	 RCC_512MS = 5,
	 RCC_640MS = 6,
	 RCC_1024MS = 7
 }tRCC;

/**
** This defines names for the ring validation modes
*/
 typedef enum {
	 RNGV_DISABLED = 0,
	 RNGV_ENABLED = 1
 }tRNGV;

/**
** This defines names for the rfwe bit
*/
 typedef enum {
	 RFWE_HALF_WAVE = 0,
	 RFWE_FULL_WAVE = 1,
	 RFWE_RNGV_RING_ENV = 0,
	 RFWE_RNGV_THRESH_CROSS = 1
 }tRFWE;

 /**
** This defines names for the rt and rt2 bit
*/
 typedef enum {
	 RT__13_5VRMS_16_5VRMS = 0,
	 RT__19_35VRMS_23_65VRMS = 1,
	 RT__40_5VRMS_49_5VRMS = 3
 }tRT;

/**
** This defines names for the rt and rt2 bit
*/
 typedef enum {
	 RGDT_ACTIVE_LOW = 0,
	 RGDT_ACTIVE_HI = 1
 }tRPOL;


/**
** This defines names for the interrupts
*/
 typedef enum {
	POLI,
	TGDI,
	LCSOI,
	DODI,
	BTDI,
	FTDI,
	ROVI,
	RDTI,
	CVI		/**< Current/Voltage Interrupt REGISTER#44 */
}vdaaInt;

/**
** Interrupt Bitmask Fields
*/
typedef enum {
    POLM = 1,
    TGDM = 2,   /**< Si3050  Only */
    LCSOM = 4,
    DODM = 8,
    BTDM = 16,
    FDTM = 32,
    ROVM = 64,
    RDTM = 128
}vdaaIntMask;


/**
** This defines names for the idl bit (obsolete)
*/
 typedef enum {
	 IDL_DISABLED = 0,
	 IDL_ENABLED = 1
 }tIDL;

/**
** This defines names for the ddl bit (obsolete)
*/
 typedef enum {
	 DDL_NORMAL_OPERATION = 0,
	 DDL_PCM_LOOPBACK = 1
 }tDDL;

/**
** Loopback Modes
*/
typedef enum {
    LPBK_NONE = 0,
    LPBK_IDL = 1,
    LPBK_DDL = 2,
    LPBK_PCML = 3
}tLpbkMode;

/**
** Loopback Status
*/
typedef enum {
    LPBK_DISABLED = 0,
    LPBK_ENABLED = 1
}tLpbkStatus;

/**
** This defines names for the interrupt pin modes
*/
 typedef enum {
	 INTE_DISABLED = 0,
	 INTE_ENABLED = 1
 }tInte;

/**
** This defines names for the interrupt pin polarities
*/
 typedef enum {
	 INTE_ACTIVE_LOW = 0,
	 INTE_ACTIVE_HIGH = 1
 }tIntePol;

/**
** This defines names for the pwm settings
*/
 typedef enum {
	PWM_DELTA_SIGMA = 0,
	PWM_CONVENTIONAL_16KHZ = 1,
	PWM_CONVENTIONAL_32KHZ = 2
 }tPwmMode;

/**
** PWME
*/
typedef enum {
    PWM_DISABLED = 0,
    PWM_ENABLED
}tPWME;

/**
** RCALD control
*/
typedef enum {
    RES_CAL_ENABLED = 0,
    RES_CAL_DISABLED
}tRCALD;

/**
** Voice DAA Hook states 
*/
enum {
VDAA_DIG_LOOPBACK = 1,
VDAA_ONHOOK = 2,
VDAA_OFFHOOK = 3,
VDAA_ONHOOK_MONITOR = 4
};

/**
** FDT Monitoring Options
*/
enum {
    FDT_MONITOR_OFF = 0,
    FDT_MONITOR_ON
};
    

/**
** Offhook Speed Select
*/
typedef enum {
    FOH_512,
    FOH_128,
    FOH_64,
    FOH_8
}tFOH;

/**
** Sample Rate Control
*/
typedef enum {
    FS_8KHZ,
    FS_16KHZ
}tHSSM;

/**
** Line Voltage Force Disable
*/
typedef enum {
    LVS_FORCE_ENABLED = 0,
    LVS_FORCE_DISABLED
}tLVFD;

/**
** Current/Voltage Monitor Select
*/
typedef enum {
    CVS_CURRENT,
    CVS_VOLTAGE
}tCVS;

/**
** Current/Voltage Interrupt Polarity
*/
typedef enum {
    CVP_BELOW,
    CVP_ABOVE
}tCVP;

/**
** Guarded Clear 
*/
typedef enum {
    GCE_DISABLED = 0,
    GCE_ENABLED
}tGCE;

/**
** SPI Mode (Si3050 Only)
*/
typedef enum {
    SPIM_TRI_CS,
    SPIM_TRI_SCLK
}tSPIM;

/**
**  FILT
*/
typedef enum {
    FILT_HPF_5HZ,
    FILT_HPF_200HZ
}tFILT;

/**
** IIRE
*/
typedef enum {
    IIR_DISABLED = 0,
    IIR_ENABLED
}tIIRE;

/**
** FULL2
*/
typedef enum {
    FULL2_DISABLED = 0,
    FULL2_ENABLED
}tFULL2;

/**
** FULL
*/
typedef enum {
    FULL_DISABLED = 0,
    FULL_ENABLED
}tFULL;

/**
** RG1
*/
typedef enum {
    RG1_DISABLED = 0,
    RG1_ENABLED
}tRG1;


/**
** -----------------------------
** CONFIGURATION DATA STRUCTURES
** -----------------------------
*/

/**
** (Updated) Structure for General Parameters
*/
typedef struct {
    tInte inte;             /* INTE */
    tIntePol intp;          /* INTP */
    tRCALD rcald;           /* RCALD */
    tHSSM hssm;             /* HSSM */
    tFOH foh;               /* FOH */
    tLVFD lvfd;             /* LVFD */
    tCVS cvs;               /* CVS */
    tCVP cvp;               /* CVP */
    tGCE gce;               /* GCE */
    tIIRE iire;             /* IIRE */
    tFULL2 full2;           /* FULL2 */
    tFULL full;             /* FULL */
    tFILT filt;             /* FILT */
    tRG1 rg1;               /* RG1 */
    tPwmMode pwmm;          /* PWMM Si3050 Only */
    tPWME pwmEnable;        /* PWME Si3050 Only */
    tSPIM spim;             /* SPIM Si3050 Only */
} vdaa_General_Cfg;


/**
** (NEW) Structure for Country Presets
*/
typedef struct {
    tRZ rz;
	tDC_Z dcr; 
	tAC_Z acim;
	tDCV dcv;
	tMINI mini;
	tILIM ilim;
	tOHS ohs_sq;
	tHBE hbe;
} vdaa_Country_Cfg;

/**
** (NEW) Structure for Hybrid Presets
*/
typedef struct {
	uInt8 hyb1;
	uInt8 hyb2;
	uInt8 hyb3;
	uInt8 hyb4;
	uInt8 hyb5;
	uInt8 hyb6;
	uInt8 hyb7;
	uInt8 hyb8;
} vdaa_Hybrid_Cfg;

/**
** Structure for PCM configuration presets
*/
 typedef struct {
	tPcmFormat pcmFormat;
	tPHCF pcmHwy;	
	tTRI pcm_tri;
} vdaa_PCM_Cfg;

/**
** Defines structure for configuring impedence 
** @deprecated:  Replace with separate vdaa_Country_Cfg preset
**            for country-specific settings and vdaa_Hybrid_Cfg
**            presets for hybrid coefficients since it is likely
**            that multiple hybrid coefficient sets will be used
**            tried during echo training.
*/
 typedef struct {
	tRZ rz;
	tDC_Z dcr; 
	tAC_Z acim;
	uInt8 hyb1;
	uInt8 hyb2;
	uInt8 hyb3;
	uInt8 hyb4;
	uInt8 hyb5;
	uInt8 hyb6;
	uInt8 hyb7;
	uInt8 hyb8;
	tDCV dcv;
	tMINI mini;
	tILIM ilim;
	tOHS ohs_sq;
	tHBE hbe;
} vdaa_Impedance_Cfg;

/** @addtogroup VDAA_AUDIO 
 * @{
 */
/** @addtogroup VDAA_GAIN_CONTROL
 * @{
 */
/*
** (Updated) Structure for Audio path gain preset
*/
 typedef struct {
	uInt8		mute;
	tXGA		xga2;
	uInt8   	acgain2;
	tXGA		xga3;
	uInt8   	acgain3;
	uInt8		callProgress;
    BOOLEAN     cpEn;
} vdaa_audioGain_Cfg;

/** @} VDAA_GAIN_CONTROL*/
/** @} */
/*
** Structure for configuring ring detect config
*/

typedef struct {
	tRDLY rdly;
	tRT rt;
	uInt8 rmx;
	tRTO rto;
	tRCC rcc;
	tRNGV rngv;
	uInt8 ras;
	tRFWE rfwe;
	tRDI rdi;
	tRPOL rpol;
} vdaa_Ring_Detect_Cfg;


/*
** Defines structure of interrupt data
*/
typedef struct {
	vdaaInt *irqs;
	uInt8 number;
} vdaaIntType;

/*
** Defines structure for configuring Loop Back
*/
typedef struct {
	tIDL isoDigLB;
	tDDL digDataLB;
} vdaa_Loopback_Cfg;


/*
** Generic Flag
*/
typedef enum {
	VDAA_BIT_SET = 1,
	VDAA_BIT_CLEAR = 0
} tVdaaBit;

/*
** Defines structure for daa current status (ring detect/hook stat)
*/
typedef struct {
	tVdaaBit ringDetectedNeg;
	tVdaaBit ringDetectedPos;
	tVdaaBit ringDetected;
	tVdaaBit offhook;
	tVdaaBit onhookLineMonitor;
} vdaaRingDetectStatusType;


typedef SiVoiceControlInterfaceType vdaaControlInterfaceType;
typedef SiVoiceDeviceType vdaaDeviceType;
typedef SiVoiceChanType vdaaChanType;

/*
** This is the main  VoiceDAA interface object pointer
*/
typedef vdaaChanType *vdaaChanType_ptr;

/*
** Defines initialization data structures
*/
typedef struct {
	uInt8 address;
	uInt8 initValue;
} vdaaRegInit;


typedef enum {
   PAR_HANDSET_NOT_DETECTED = 0,
   PAR_HANDSET_DETECTED = 1
}vdaaPHDStatus;

/*
** Line In Use Configuration Structure
*/
typedef struct {
    vdaaPHDStatus status;
    int8 min_onhook_vloop;       
    int8 min_offhook_vloop;
    int16 min_offhook_iloop;
    int8 measured_vloop;
    int16 measured_iloop;
}vdaa_LIU_Config;

/*
** Function Declarations
*/

/*****************************************************************************/
/** @defgroup VDAA_IF_CONFIG VDAA System control interface functions
 * This group of functions is called for allocating memory and configuring 
 * the ProSLIC API to call specific control interfaces that the user implemented.
 * @deprecated One should use the SiVoice equivalent functions. @ref SIVOICE_IF_CFG
 * @{
 */

/*****************************************************************************/
/** @defgroup VDAA_MEM Memory allocation/deallocation 
 * @deprecated One should use the SiVoice equivalent functions located: @ref SIVOICE_MEMORY_IF
 * @{
 */

/**
 @brief
 *  Allocate memory and initialize the given structure.
 *
 * @param[in,out] pCtrlIntf - the structure to initialize
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 * @deprecated Use @ref SiVoice_createControlInterface
 * 
 */

int Vdaa_createControlInterface (vdaaControlInterfaceType **pCtrlIntf);

/**
 @brief
 *  Destroys the given structure and deallocates memory.
 *
 * @param[in,out] pCtrlIntf  - the structure to destroy/deallocate
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 * @deprecated Use @ref SiVoice_destroyControlInterface
* 
*/

int Vdaa_destroyControlInterface (vdaaControlInterfaceType **pCtrlIntf);

/**
 * @brief
 *  Allocate memory and initialize the given structure.
 *
 * @param[in,out] **pDev - the structure to initialize
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 * @deprecated Use @ref SiVoice_createDevice
 */

int Vdaa_createDevice (vdaaDeviceType **pDev);

/**
 * @brief
 *  Destroys the given structure and deallocates memory.
 *
 * @param[in,out] **pDev - the structure to initialize
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 * @deprecated Use @ref SiVoice_destroyDevice
 */

int Vdaa_destroyDevice (vdaaDeviceType **pDev);

/**
 * @brief
 *  Allocate memory and initialize the given structure.
 *
 * @param[in,out] pVdaa - the structure to initialize
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 * @deprecated Use @ref SiVoice_createChannel
 */

int Vdaa_createChannel (vdaaChanType **pVdaa);

/**
 * @brief
 *  Destroys the given structure and deallocates memory.
 *
 * @param[in,out] pVdaa - the structure to initialize
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 * @deprecated Use @ref Vdaa_destroyChannel
 */

int Vdaa_destroyChannel (vdaaChanType **pVdaa);

/** @} */

/*****************************************************************************/
/** @defgroup VDAA_IO SPI/GCI access routines
 * This group of functions are used to associcate the transport mechanism (SPI, GCI) functions with the API.  The actual
 * functions being references are normally implemented by the customer for their particualr OS and platform.
 *
 * @deprecated One should use the SiVoice equivalent functions located: @ref SIVOICE_IO
 * @{
 */

/**
 * @brief
 *  Associate a interface object with a user supplied datastructure.  This 
 *  structure is passed to all the I/O routines that the ProSLIC API calls.
 *
 * @param[in,out] *pCtrlIntf - which interface to associate 
 *                the user supplied structure with.
 * @param[in] hCtrl - the user supplied structure.
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 * @deprecated Use SiVoice_setControlInterfaceCtrlObj
 */

int Vdaa_setControlInterfaceCtrlObj (vdaaControlInterfaceType *pCtrlIntf, void *hCtrl);

/**
 * @brief
 *  Associate a interface object with the reset function.
 *
 * @param[in,out] pCtrlIntf - which interface to associate 
 *                the user supplied function with.
 * @param[in] Reset_fptr - the reset function pointer
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 * @deprecated Use @ref SiVoice_setControlInterfaceReset
 */

int Vdaa_setControlInterfaceReset (vdaaControlInterfaceType *pCtrlIntf, ctrl_Reset_fptr Reset_fptr);

/**
 * @brief
 *  Associate a interface object with the register write function.
 *
 * @param[in,out] pCtrlIntf - which interface to associate 
 *                the user supplied function with.
 * @param[in] WriteRegister_fptr - the register write function pointer
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 * @deprecated Use @ref SiVoice_setControlInterfaceWriteRegister
 */

int Vdaa_setControlInterfaceWriteRegister (vdaaControlInterfaceType *pCtrlIntf, ctrl_WriteRegister_fptr WriteRegister_fptr);

/**
 * @brief
 *  Associate a interface object with the register read function.
 *
 * @param[in,out] pCtrlIntf - which interface to associate 
 *                the user supplied function with.
 * @param[in] ReadRegister_fptr- the register read function pointer
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 * @deprecated Use @ref SiVoice_setControlInterfaceReadRegister
 */

int Vdaa_setControlInterfaceReadRegister (vdaaControlInterfaceType *pCtrlIntf, ctrl_ReadRegister_fptr ReadRegister_fptr);

/**
 * @brief
 *  Associate a interface object with the write RAM function.
 *
 * @param[in,out] pCtrlIntf - which interface to associate 
 *                the user supplied function with.
 * @param[in] WriteRAM_fptr - the reset function pointer
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 * @deprecated Use @ref SiVoice_setControlInterfaceWriteRAM
 */

int Vdaa_setControlInterfaceWriteRAM (vdaaControlInterfaceType *pCtrlIntf, ctrl_WriteRAM_fptr WriteRAM_fptr);

/**
 * @brief
 *  Associate a interface object with the read RAM function.
 *
 * @param[in,out] pCtrlIntf - which interface to associate 
 *                the user supplied function with.
 * @param[in] ReadRAM_fptr - the read RAM function pointer
 *
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 * @deprecated Use SiVoice_setControlInterfaceReadRAM
 */

int Vdaa_setControlInterfaceReadRAM (vdaaControlInterfaceType *pCtrlIntf, ctrl_ReadRAM_fptr ReadRAM_fptr);

/** @} VDAA_IO */

/*****************************************************************************/
/** @defgroup VDAA_TIMER Timer functions
 *
 * This group of functions associates the customer supplied timer routines with the ProSLIC API.
 *
 * @deprecated One should use the SiVoice equivalent functions located: @ref SIVOICE_TIMER
 * @{
 */

/**
 * @brief
 *  This function associates a timer object - which is user defined, but it is 
 *  used with ALL channels of the particular control interface.
 *
 * @param[in] pCtrlIntf - which control interface to associate the given timer object with.
 * @param[in] hTimer  - the timer ojbect that is passed to all timer functions.
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 *
 * @sa SIVOICE_TIMER Vdaa_setControlInterfaceDelay Vdaa_setControlInterfaceTimeElapsed Vdaa_setControlInterfaceGetTime
 * @deprecated Use SiVoice_setControlInterfaceTimerObj
 */

int Vdaa_setControlInterfaceTimerObj (vdaaControlInterfaceType *pCtrlIntf, void *hTimer);

/** 
 * @brief
 *  Associate a timer delay function with a given control interface.  The 
 *  delay function takes in an argument of the timer object and the time in mSec
 *  and delays the thread/task for at least the time requested.
 *
 *  @param[in] pCtrlIntf - which control interface to associate the function with.
 *  @param[in] Delay_fptr - the pointer to the delay function.
 *  @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 *
 *  @sa SIVOICE_TIMER Vdaa_setControlInterfaceTimerObj Vdaa_setControlInterfaceTimeElapsed Vdaa_setControlInterfaceGetTime
 * @deprecated Use @ref SiVoice_setControlInterfaceDelay
 */

int Vdaa_setControlInterfaceDelay (vdaaControlInterfaceType *pCtrlIntf, system_delay_fptr Delay_fptr);

/** 
 *  @brief
 *   Associate a time elapsed function with a given control interface.  The
 *   time elapsed function uses the values from the function specified in
 *   @ref SiVoice_setControlInterfaceGetTime and computes the delta time
 *   in mSec.
 *   @param[in] pCtrlIntf - which control interface to associate the function with.
 *   @param[in] timeElapsed_fptr     - the pointer to the elapsed time function.
 *   @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 *   @sa SIVOICE_TIMER Vdaa_setControlInterfaceTimerObj Vdaa_setControlInterfaceDelay Vdaa_setControlInterfaceGetTime
 * @deprecated Use @ref SiVoice_setControlInterfaceTimeElapsed
 */

int Vdaa_setControlInterfaceTimeElapsed (vdaaControlInterfaceType *pCtrlIntf, system_timeElapsed_fptr timeElapsed_fptr);

/** 
 *  @brief
 *   Associate a time get function with a given control interface.  The
 *   time get function returns a value in a form of a void pointer that 
 *   is suitable to be used with the function specified in @ref SiVoice_setControlInterfaceTimeElapsed .
 *   This is typically used as a timestamp of when an event started. The resolution needs to be in terms
 *   of mSec.
 *
 *   @param[in] pCtrlIntf - which control interface to associate the function with.
 *   @param[in] getTime_fptr -  the pointer to the get time function.
 *   @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 *   @sa SIVOICE_TIMER Vdaa_setControlInterfaceTimerObj Vdaa_setControlInterfaceDelay Vdaa_setControlInterfaceTimeElapsed
 * @deprecated Use @ref SiVoice_setControlInterfaceGetTime
 */

int Vdaa_setControlInterfaceGetTime (vdaaControlInterfaceType *pCtrlIntf, system_getTime_fptr getTime_fptr);

/** @} VDAA_TIMER */

/*****************************************************************************/
/** @defgroup VDAA_PROCESS VDAA Process control
 * @{
 */

/**
 * @brief
 *  This function assoicates a user defined semaphore/critical section
 *  function with the given interface.
 *
 * @param[in,out] pCtrlIntf - the interface to associate the function with.
 * @param[in] semaphore_fptr - the function pointer for semaphore control.
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 *
 * @deprecated Use @ref SiVoice_setControlInterfaceSemaphore
 */

int Vdaa_setControlInterfaceSemaphore (vdaaControlInterfaceType *pCtrlIntf, ctrl_Semaphore_fptr semaphore_fptr);

/** @} */
/** @} VDAA_IF_CONFIG */

/*****************************************************************************/
/** @defgroup VDAA_DEBUG Debug
 * * This group of functions enables/disables debug messages as well as dump 
 * register contents.
 * @{
 */
/**
 * @brief
 * This function enables or disables the debug mode, assuming @ref ENABLE_DEBUG is set in the configuration file.
 *
 * @param[in] pVdaa - which channel to set the debug flag.
 * @param[in] debugEn - 0 = Not set, 1 = set.
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 * @deprecated Use @ref SiVoice_setSWDebugMode
 *
 */

int Vdaa_setSWDebugMode (vdaaChanType_ptr pVdaa, int32 debugEn);

/**
 * @brief
 * This function dumps to console the register contents of several
 * registers and RAM locations.
 *
 * @param[in] pVdaa - which channel to dump the register contents of.
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 *
 */

int Vdaa_PrintDebugData (vdaaChanType *pVdaa);

/** @} VDAA_DEBUG */

/*****************************************************************************/
/** @defgroup VDAA_ERROR Return code functions
 * This group of functions are used for when the ProSLIC API function does
 * not reutrn a standard error value and instead returns back some other value.
 * You may call these functions to see if there was an error preset and to clear 
 * the error state.
 * @{
 */
/**
 * @brief
 * This function returns the error flag that may be set by some function in where
 * @ref errorCodeType is not returned.  
 *
 * @note For functions that DO return errorCodeType, the return value here is undefined.
 *
 * @param[in] pVdaa - which channel to clear the error flag
 * @param[in,out] error - The current value of error flag.  
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 * @deprecated Use @ref SiVoice_getErrorFlag
 *
 * @sa ProSLIC_clearErrorFlag
 */

int Vdaa_getErrorFlag (vdaaChanType_ptr pVdaa, int *error);

/**
 * @brief
 *  This function clears the error flag that may be set by some function in where
 *  @ref errorCodeType is not returned.
 *
 * @param[in,out] pVdaa - which channel to clear the error flag
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 * @deprecated Use @ref SiVoice_clearErrorFlag
 *
 * @sa ProSLIC_getErrorFlag
 */

int Vdaa_clearErrorFlag (vdaaChanType_ptr pVdaa);

/** @} VDAA_ERROR */

/*****************************************************************************/
/** @defgroup VDAA_AUDIO Audio
 * @{
 */
/** @defgroup VDAA_GAIN_CONTROL Gain Control
 * This section covers functions that allow one to adjust the audio
 * gains - with TX toward the network/SOC/DSP and RX toward the tip/ring.
 *
 * @{
 */

/** 
 * @brief
 *  Sets the TX audio gain (toward the network). 
 *
 * @param[in] pVdaa - which channel to configure
 * @param[in] preset - which preset to use (this may be from the constants file)
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */

int Vdaa_TXAudioGainSetup (vdaaChanType *pVdaa,int32 preset);

/** 
 * @brief
 *  Sets the RX audio gain (toward the tip/ring). 
 *
 * @param[in] pVdaa - which channel to configure
 * @param[in] preset - which preset to use (this may be from the constants file)
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */

int Vdaa_RXAudioGainSetup (vdaaChanType *pVdaa,int32 preset);

/** @} VDAA_GAIN_CONTROL */

/*****************************************************************************/
/** @defgroup VDAA_AUDIO_CONTROL Audio control/configuration
 * This group of functions is used to configure and control the PCM bus.  It is essential that @ref Vdaa_PCMSetup,
 * @ref Vdaa_PCMTimeSlotSetup and @ref Vdaa_PCMStart are called prior to any audio processing.
 * @{
 */

/** 
 * @brief 
 *  This configures the PCM bus with parameters such as companding and data latching timing.
 *
 * @param[in] pVdaa - which channel should be configured
 * @param[in] preset - which preset to use from the constants file (see configuration tool, PCM dialog box)
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 *
 * @sa Vdaa_PCMTimeSlotSetup ProSLIC_PCMStart
 */

int Vdaa_PCMSetup (vdaaChanType *pVdaa,int32 preset);

/** 
 * @brief 
 *  This configures the ProSLIC to latch and send the PCM data at a particular timeslot in terms of PCM clocks (PCLK).
 *
 *  Typically, for aLaw and uLaw, one can use the following calculation to set the rxcount and txcount parameters:
 *
 *  rxcount = txcount = (channel_number)*8;
 *
 *  For 16 bit linear, one can do the following:
 *
 *  rxcount = txcount = (channel_number)*16;
 *
 * where channel_number = which ProSLIC channel on the PCM bus.  For example, if one were to have 2 dual channel
 * VDAA's on the same PCM bus, this value would range from 0 to 3. 
 *
 * @param[in] pVdaa - which channel should be configured
 * @param[in] rxcount -  how many clocks until reading data from the PCM bus
 * @param[in] txcount -  how many clocks until writing data to the PCM bus.
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 *
 * @sa Vdaa_PCMSetup Vdaa_PCMStart
 */

int Vdaa_PCMTimeSlotSetup (vdaaChanType *pVdaa, uInt16 rxcount, uInt16 txcount);

/**
 * @brief
 *  This enables PCM transfer on the given ProSLIC channel.
 *
 * @param[in] pVdaa - - which channel should be enabled
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 *
 * @sa ProSLIC_PCMSetup ProSLIC_PCMTimeSlotSetup ProSLIC_PCMStop
 */

int Vdaa_PCMStart (vdaaChanType *pVdaa);

/**
 * @brief
 *  This disables PCM transfer on the given VDAA channel. Typically, this is called for debugging
 *  purposes only.
 *
 * @param[in] pVdaa - - which channel should be disabled
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 *
 * @sa Vdaa_PCMSetup Vdaa_PCMTimeSlotSetup Vdaa_PCMStart
 */

int Vdaa_PCMStop (vdaaChanType *pVdaa);

/**
 * @brief 
 * Program desired mute mode for the given channel.
 *
 * @param[in] pVdaa - which channel should be modified 
 * @param[in] mute - what is the desired mode.
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 *
 */

int Vdaa_SetAudioMute(vdaaChanType *pVdaa, tMUTE mute);

/**
 * @brief 
 * Program desired loopback test mode for the given channel.
 *
 * @param[in] pVdaa -  which channel should be modified 
 * @param[in] lpbk_mode - what is the desired loopback mode.
 * @param[in] lpbk_status - is this to enable or disable the loopback mode.
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 * @deprecated This API is depricated as of 5.2.0
 * @sa Vdaa_LoopbackSetup
 *
 */

int Vdaa_SetLoopbackMode(vdaaChanType_ptr pVdaa, tLpbkMode lpbk_mode, tLpbkStatus lpbk_status);

/**
 * @brief 
 * Program desired loopback test mode for the given channel.
 *
 * @param[in] pVdaa -  which channel should be modified 
 * @param[in] preset - which of the API configuration tool's preset to use.
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 *
 */

int Vdaa_LoopbackSetup (vdaaChanType *pVdaa, int32 preset);

/*****************************************************************************/
/** @} VDAA_AUDIO */
/** @} */
/** @defgroup VDAA_RING Ring detection
 * @{
 */

/**
 * @brief This function configures ringing detect for the Vdaa.
 * @param[in] pVdaa - which channel to program
 * @param[in] preset - Index of predefined ring detect setup configuration
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */

int Vdaa_RingDetectSetup (vdaaChanType *pVdaa,int32 preset);


/** 
 * @brief This function reads ring detect/hook status and populates the structure passed via pStatus.
 * @param[in] pVdaa - Pointer to Voice DAA channel structure
 * @param[out] pStatus - updated ring status.
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */

int Vdaa_ReadRingDetectStatus (vdaaChanType *pVdaa,vdaaRingDetectStatusType *pStatus);

/** @} VDAA_RING */

/*****************************************************************************/
/** @defgroup VDAA_LINE_STATE Line state 
 * @{
 */


/** 
 * @brief This function sets the Voice DAA hook status
 * @param[in]  pVdaa - Pointer to Voice DAA channel structure
 * @param[in]  newHookStatus - new hook state (VDAA_ONHOOK, VDAA_OFFHOOK, VDAA_DIG_LOOPBACK or VDAA_ONHOOK_MONITOR)
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */

int Vdaa_SetHookStatus (vdaaChanType *pVdaa,uInt8 newHookStatus);

/** 
 * @brief This function powers up the Voice DAA lineside device.
 * @param[in]  pVdaa - Pointer to Voice DAA channel structure
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */

int Vdaa_PowerupLineside(vdaaChanType_ptr pVdaa);

/** 
 * @brief This function powers down the Voice DAA lineside device.
 * @param[in]  pVdaa - Pointer to Voice DAA channel structure
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */

int Vdaa_PowerdownLineside(vdaaChanType_ptr pVdaa);

/**
 * @brief Read VDAA Hook Status
 * @param[in] pVdaa - which channel to return the hook status.
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */

uInt8 Vdaa_GetHookStatus (vdaaChanType *pVdaa);

/** @} VDAA_LINE_STATE */

/*****************************************************************************/
/** @defgroup VDAA_DIAG Diagnostics 
 * @{
 */

/** 
 * @brief This function returns the Voice DAA linefeed status.
 * @param[in] pVdaa - Pointer to Voice DAA channel structure
 * @param[in,out] vloop - Pointer to loop voltage variable (set by this function) - in mV
 * @param[in,out] iloop - Pointer to loop current variable (set by this function) - in uA
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */

int Vdaa_ReadLinefeedStatus (vdaaChanType *pVdaa,int8 *vloop, int16 *iloop);

/** 
 * @brief This function initializes the vdaa_LIU_Config datastructure for line in use feature
 * @param[in,out] liuCfg - Pointer to vdaa_LIU_Config structure (user allocates)
 * @param[in] minOnV - minimum onhook loop voltage that indicates no parallel handset is present (mV)
 * @param[in] minOffV - minimum offhook loop voltage that indicates no parallel handset is present (mV)
 * @param[in] minOffI - minimum offhook loop current that indicates no parallel handset is preset (uA)
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 * @sa Vdaa_CheckForLineInUse
 */

int Vdaa_InitLineInUseCheck(vdaa_LIU_Config *liuCfg,int8 minOnV,int8 minOffV,int16 minOffI );

/** 
 * @brief Monitor LVCS to detect intrusion or parallel handset
 * @param[in] pVdaa - Pointer to Voice DAA channel structure
 * @param[in,out] liuCfg - Pointer to vdaa_LIU_Config structure 
 * @retval VDAA_ONHOOK or VDAA_OFFHOOK (in use)
 * @sa Vdaa_InitLineInUseCheck
 */

uInt8 Vdaa_CheckForLineInUse(vdaaChanType *pVdaa, vdaa_LIU_Config *liuCfg);

/** 
 * @brief This function can be used to verify that the SPI interface is functioning properly.
 * @param[in] pVdaa - Pointer to Voice DAA channel structure
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 * @deprecated Deprecated in 5.2.0
 */

int Vdaa_VerifyControlInterface (vdaaChanType *pVdaa);

/** 
 * @brief This function returns the status of the Frame Detect (FDT) bit.
 * @param[in] pVdaa - Pointer to Voice DAA channel structure
 * @retval int - 0 - Frame NOT detected, 1 = Frame detected
 */

int Vdaa_ReadFDTStatus(vdaaChanType_ptr pVdaa);

/** @} VDAA_DIAG */

/*****************************************************************************/
/** @defgroup VDAA_INTERRUPTS Interrupts
 * @{
 */

/** 
 * @brief Enables ALL interrupts
 * @param[in] pVdaa - Pointer to Voice DAA channel structure
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 * @deprecated Replaced by @ref Vdaa_SetInterruptMask
 */

int Vdaa_EnableInterrupts (vdaaChanType *pVdaa);

/** 
 * @brief Enables interrupts based on passed 9-bit bitmask.  Bit values defined by vdaaIntMask enum.
 * @param[in] pVdaa - Pointer to Voice DAA channel structure
 * @param[in] bitmask - a logical or of @ref vdaaIntMask enum
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */

int Vdaa_SetInterruptMask(vdaaChanType *pVdaa, vdaaIntMask bitmask);

/** 
 * @brief Returns the current interrupt status.
 * @param[in] pVdaa - Pointer to Voice DAA channel structure
 * @param[out]  pIntData - interrupt mask showing which interrupts are set.
 * @retval int - the number of interrupts set or RC_IGNORE
 */

int Vdaa_GetInterrupts (vdaaChanType *pVdaa,vdaaIntType *pIntData);

/** 
 * @brief Clears ALL interrupts
 * @param[in] pVdaa - Pointer to Voice DAA channel structure
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */

int Vdaa_ClearInterrupts (vdaaChanType *pVdaa);

/** @} VDAA_INTERRUPTS */

/*****************************************************************************/
/** @defgroup VDAA_INIT Initialization
 * @{
 *
 * @defgroup VDAA_SOFT_INIT Software initialization 
 * @{
 */

/**
 * @brief
 *  This function initializes the various channel structure elements.  
 *  This function does not access the chipset directly, so SPI/GCI
 *  does not need to be up during this function call.
 *
 *  It is suggested to migrate to the @ref SiVoice_SWInitChan in new
 *  implementations.
 *
 * @param[in,out] pVdaa - which channel to initialize.
 * @param[in] channel - Which channel index is this.  For example, for a 4 channel system, this would typically range from 0 to 3.
 * @param[in] chipType - chipset family type for example @ref SI321X_TYPE or @ref SI3217X_TYPE
 * @param[in] deviceObj - Device structure pointer associated with this channel
 * @param[in] pCtrlIntf - Control interface associated with this channel
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 * @deprecated Use @ref SiVoice_SWInitChan
 */

int Vdaa_SWInitChan (vdaaChanType_ptr pVdaa,int32 channel,int chipType, SiVoiceDeviceType*deviceObj,SiVoiceControlInterfaceType *pCtrlIntf);

/**
 * @brief
 *  This function sets the channel enable status.  If NOT set, then when
 *  the various initialization routines such as @ref Vdaa_SWInitChan is called,
 *  then this particular channel will NOT be initialized.
 *
 *  This function does not access the chipset directly, so SPI/GCI
 *  does not need to be up during this function call.
 *
 * @param[in,out] pVdaa - which channel to return the status.
 * @param[in] chanEn - The new value of the channel enable field. 0 = NOT enabled, 1 = enabled.
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 *
 * @sa Vdaa_getChannelEnable SiVoice_SWInitChan
 */

int Vdaa_setChannelEnable (vdaaChanType_ptr pVdaa, int chanEn);


/**
 * @brief
 *  This function returns back if the channel is enabled or not.
 *  This function does not access the chipset directly, so SPI/GCI
 *  does not need to be up during this function call.
 *
 * @param[in] pVdaa - which channel to return the status.
 * @param[in,out] chanEn - The current value of if the channel is enabled. 0 = NOT enabled, 1 = enabled.
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 *
 * @sa Vdaa_setChannelEnable SiVoice_SWInitChan
 */

int Vdaa_getChannelEnable (vdaaChanType_ptr pVdaa, int* chanEn);

/** @} VDAA_SOFT_INIT */

/*****************************************************************************/
/** @defgroup VDAA_HW_INIT Hardware/line initialization
 * @{
 */

/**
 * @brief
 *  This function configures the Voice DAA with a predefined country configuration preset.
 *
 * @param[in] pVdaa - which channel to initialize
 * @param[in] preset - The preset to use to configure the VDAA with.
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 *
 */

int Vdaa_CountrySetup (vdaaChanType *pVdaa,int32 preset);

/**
 * @brief
 *  This function configures the Voice DAA digital hybrid with a predefined preset.
 *
 * @param[in] pVdaa - which channel to initialize
 * @param[in] preset - Index of predefined digital hybrid preset
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 * @sa Vdaa_SetHybridEnable
 */

int Vdaa_HybridSetup (vdaaChanType *pVdaa,int32 preset);

/**
 * @brief
 *  This function sets the impedance coefficients of the Vdaa.
 *
 * @param[in] pVdaa - which channel to initialize
 * @param[in] preset - Index of predefined digital hybrid preset
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 *
 */

int Vdaa_ImpedanceSetup (vdaaChanType *pVdaa,int32 preset);

/**
 * @brief
 *  This function fully initializes and loads the general config parameters to all Vdaa devices 
 *  a given daisychain up to a given number.
 *
 * @param[in] pVdaa - which channel to initialize or start from if size>1
 * @param[in] size - the number of channels to initialize, should be at least 1
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 *
 */

int Vdaa_Init (vdaaChanType_ptr *pVdaa,int size);

/**
 * @brief
 *  This function fully initializes and loads the general config parameters to all Vdaa devices on
 *  a given daisychain.
 *
 * @param[in] pVdaa - Vdaa channel
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 *
 */

int Vdaa_InitBroadcast (vdaaChanType_ptr pVdaa);

/**
 * @brief
 *  This function calibrates the ADC (analog to digital converter) of the Vdaa device(s).
 *
 * @param[in] pVdaa - which channel to calibrate or starting channel if size>1
 * @param[in] size - the number of channels to calibrate.
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 *
 */

int Vdaa_ADCCal (vdaaChanType_ptr pVdaa, int32 size);

/**
 * @brief
 *  This function controls the Voice DAA digital hybrid.
 *
 * @param[in] pVdaa - which channel to set/unset
 * @param[in] enable - TRUE: enable digital hybrid, FALSE: disable digital hybrid
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 *
 */

int Vdaa_SetHybridEnable(vdaaChanType_ptr pVdaa, int enable);

/** @} VDAA_HW_INIT */
/*****************************************************************************/

/**
 * @brief Execute a reset on a given channel/daisychain.
 * @param[in] pVdaa - the channel to reset
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 * @deprecated Use @ref SiVoice_Reset
 */

int Vdaa_Reset (vdaaChanType *pVdaa);

/**
 * @brief  Enables watchdog timer
 * @param[in] pVdaa - the channel to enable the watchdog
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */

int Vdaa_EnableWatchdog(vdaaChanType_ptr pVdaa);

/**
 * @brief Execute a soft reset on a given channel.
 * @param[in] pVdaa - the channel to reset
 * @retval int - error from @ref errorCodeType  @ref RC_NONE indicates no error.
 */

int Vdaa_SoftReset(vdaaChanType_ptr pVdaa);/** @} VDAA_INIT */

/**
 * @brief
 *  This function returns back a string with the current version of the 
 *  ProSLIC API.
 *
 * @details
 *  The string is in the following format MM.mm.vv[.rcX] format, where
 *  MM is the major number, mm is the minor number and vv is the sub-minor number.
 *  If this is a release candidate, a .RCx is tacked on - such as RC1 for release
 *  candidate 1.
 *
 *  @retval char * - returns back a constant string.
 */
char *Vdaa_Version(void);

/** @} MISC */

/** @} VDAA_API */
#endif

