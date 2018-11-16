/*
** Copyright (c) 2007-2012 by Silicon Laboratories
**
** $Id: si3226_intf.h 3713 2012-12-18 17:30:28Z cdp $
**
** Si3226_Intf.h
** Si3226 ProSLIC interface header file
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
** proslic_datatypes.h, Si3226_registers.h, ProSLIC.h
**
*/
#ifndef SI3226_INTF_H
#define SI3226_INTF_H


/*
**
** Si3226 General Constants
**
*/
#define CHAN_PER_DEVICE             2
  
/* Timeouts in 10s of ms */
#define SI3226_TIMEOUT_DCDC_UP             200
#define SI3226_TIMEOUT_DCDC_DOWN           200
 

#define SI3226_CAL_STD_CALR1               0xFF
#define SI3226_CAL_STD_CALR2               0xF8


/*
**
** PROSLIC INITIALIZATION FUNCTIONS
**
*/

/*
** Function: PROSLIC_Reset
**
** Description: 
** Resets the ProSLIC
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
**
** Return:
** none
*/
int Si3226_Reset (proslicChanType_ptr hProslic);

/*
** Function: PROSLIC_ShutdownChannel
**
** Description: 
** Safely shutdown channel w/o interruption to 
** other active channels
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
**
** Return:
** none
*/
int Si3226_ShutdownChannel (proslicChanType_ptr hProslic);

/*
** Function: PROSLIC_Init
**
** Description: 
** Initializes the ProSLIC
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
**
** Return:
** none
*/
int Si3226_Init (proslicChanType_ptr *hProslic,int size);

/*
** Function: PROSLIC_Reinit
**
** Description: 
** Peforms Soft Reset and reinitializes the ProSLIC
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
**
** Return:
** none
*/
int Si3226_Reinit (proslicChanType_ptr hProslic,int size);

/*
** Function: PROSLIC_Init_MultiBOM
**
** Description: 
** Initializes the ProSLIC with supplied general configuration preset
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
**
** Return:
** none
*/
int Si3226_Init_MultiBOM (proslicChanType_ptr *hProslic,int size,int preset);


int Si3226_VerifyControlInterface (proslicChanType_ptr hProslic);
uInt8 Si3226_ReadReg (proslicChanType_ptr hProslic,uInt8 addr);
int Si3226_WriteReg (proslicChanType_ptr hProslic,uInt8 addr,uInt8 data);
ramData Si3226_ReadRAM (proslicChanType_ptr hProslic,uInt16 addr);
int Si3226_WriteRAM (proslicChanType_ptr hProslic,uInt16 addr, ramData data);

/*
** Function: ProSLIC_PrintDebugData
**
** Description: 
** Register and RAM dump utility
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
**
** Return:
** none
*/
int Si3226_PrintDebugData (proslicChanType_ptr hProslic);

/*
** Function: ProSLIC_PrintDebugReg
**
** Description: 
** Register dump utility
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
**
** Return:
** none
*/
int Si3226_PrintDebugReg (proslicChanType_ptr hProslic);

/*
** Function: ProSLIC_PrintDebugRAM
**
** Description: 
** RAM dump utility
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
**
** Return:
** none
*/
int Si3226_PrintDebugRAM (proslicChanType_ptr hProslic);


/*
** Function: Si3226_PowerUpConverter
**
** Description: 
** Powers all DC/DC converters sequentially with delay to minimize
** peak power draw on VDC.
**
** Returns:
** int (error)
**
*/
int Si3226_PowerUpConverter(proslicChanType_ptr hProslic);

/*
** Function: Si3226_PowerDownConverter
**
** Description: 
** Power down DCDC converter (selected channel only)
**
** Returns:
** int (error)
**
*/
int Si3226_PowerDownConverter(proslicChanType_ptr hProslic);


/*
** Function: PROSLIC_Calibrate
**
** Description: 
** Calibrates the ProSLIC
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
**
** Return:
** none
*/
int Si3226_Calibrate (proslicChanType_ptr *hProslic, int size, uInt8 *calr, int maxTime);

/*
** Function: PROSLIC_Cal
**
** Description: 
** Calibrates the ProSLIC
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
**
** Return:
** none
*/
int Si3226_Cal (proslicChanType_ptr *hProslic, int size);

/*
** Function: PROSLIC_LoadRegTables
**
** Description: 
** Loads registers and ram in the ProSLIC
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
** pRamTable: pointer to ram values to load
** pRegTable: pointer to register values to load
** 
**
** Return:
** none
*/
int Si3226_LoadRegTables (proslicChanType_ptr *hProslic, ProslicRAMInit *pRamTable, ProslicRegInit *pRegTable,int size);

/*
** Function: PROSLIC_LoadPatch
**
** Description: 
** Loads patch to the ProSLIC
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
** pPatch: pointer to patch data
**
** Return:
** none
*/
int Si3226_LoadPatch (proslicChanType_ptr hProslic, const proslicPatch *pPatch);

/*
** Function: PROSLIC_VerifyPatch
**
** Description: 
** Verifies patch to the ProSLIC
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
** pPatch: pointer to patch data
**
** Return:
** none
*/
int Si3226_VerifyPatch (proslicChanType_ptr hProslic, const proslicPatch *pPatch);

/*
** Function: PROSLIC_EnableInterrupts
**
** Description: 
** Enables interrupts
**
** Input Parameters: 
** hProslic: pointer to Proslic object
** 
** Return:
** 
*/
int Si3226_EnableInterrupts (proslicChanType_ptr hProslic);
int Si3226_DisableInterrupts (proslicChanType_ptr hProslic);

int Si3226_SetLoopbackMode (proslicChanType_ptr hProslic, ProslicLoopbackModes newMode);

int Si3226_SetMuteStatus (proslicChanType_ptr hProslic, ProslicMuteModes muteEn);
/*
**
** PROSLIC CONFIGURATION FUNCTIONS
**
*/

/*
** Function: PROSLIC_RingSetup
**
** Description: 
** configure ringing
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pRingSetup: pointer to ringing config structure
**
** Return:
** none
*/
int Si3226_RingSetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_ToneGenSetup
**
** Description: 
** configure tone generators
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pTone: pointer to tones config structure
**
** Return:
** none
*/
int Si3226_ToneGenSetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_FSKSetup
**
** Description: 
** configure fsk
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pFsk: pointer to fsk config structure
**
** Return:
** none
*/
int Si3226_FSKSetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_DTMFDecodeSetup
**
** Description: 
** configure dtmf decode
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pDTMFDec: pointer to dtmf decoder config structure
**
** Return:
** none
*/
int Si3226_DTMFDecodeSetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_SetProfile
**
** Description: 
** set country profile of the proslic
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pCountryData: pointer to country config structure
**
** Return:
** none
*/
int Si3226_SetProfile (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_ZsynthSetup
**
** Description: 
** configure impedence synthesis
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pZynth: pointer to zsynth config structure
**
** Return:
** none
*/
int Si3226_ZsynthSetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_GciCISetup
**
** Description: 
** configure CI bits (GCI mode)
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pCI: pointer to ringing config structure
**
** Return:
** none
*/
int Si3226_GciCISetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_ModemDetSetup
**
** Description: 
** configure modem detector
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pModemDet: pointer to modem det config structure
**
** Return:
** none
*/
int Si3226_ModemDetSetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_AudioGainSetup
**
** Description: 
** configure audio gains
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pAudio: pointer to audio gains config structure
**
** Return:
** none
*/
int Si3226_TXAudioGainSetup (proslicChanType *pProslic, int preset);
int Si3226_RXAudioGainSetup (proslicChanType *pProslic, int preset);
#define Si3226_AudioGainSetup ProSLIC_AudioGainSetup
int Si3226_TXAudioGainScale (proslicChanType *pProslic, int preset, uInt32 pga_scale, uInt32 eq_scale);
int Si3226_RXAudioGainScale (proslicChanType *pProslic, int preset, uInt32 pga_scale, uInt32 eq_scale);
/*
** Function: PROSLIC_HybridSetup
**
** Description: 
** configure Proslic hybrid
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pHybridCfg: pointer to ringing config structure
**
** Return:
** none
*/
int Si3226_HybridSetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_AudioEQSetup
**
** Description: 
** configure audio equalizers
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pAudioEQ: pointer to ringing config structure
**
** Return:
** none
*/
int Si3226_AudioEQSetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_DCFeedSetup
**
** Description: 
** configure dc feed
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pDcFeed: pointer to dc feed config structure
**
** Return:
** none
*/
int Si3226_DCFeedSetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_GPIOSetup
**
** Description: 
** configure gpio
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pGpio: pointer to gpio config structure
**
** Return:
** none
*/
int Si3226_GPIOSetup (proslicChanType *pProslic);

/*
** Function: PROSLIC_PCMSetup
**
** Description: 
** configure pcm
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pPcm: pointer to pcm config structure
**
** Return:
** none
*/
int Si3226_PCMSetup (proslicChanType *pProslic, int preset);
int Si3226_PCMTimeSlotSetup (proslicChanType *pProslic, uInt16 rxcount, uInt16 txcount);

/*
**
** PROSLIC CONTROL FUNCTIONS
**
*/

/*
** Function: PROSLIC_GetInterrupts
**
** Description: 
** Enables interrupts
**
** Input Parameters: 
** hProslic: pointer to Proslic object
** pIntData: pointer to interrupt info retrieved
** 
** Return:
** 
*/
int Si3226_GetInterrupts (proslicChanType_ptr hProslic, proslicIntType *pIntData);

/*
** Function: PROSLIC_ReadHookStatus
**
** Description: 
** Determine hook status
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pHookStat: current hook status
**
** Return:
** none
*/
int Si3226_ReadHookStatus (proslicChanType *pProslic,uInt8 *pHookStat);

/*
** Function: PROSLIC_WriteLinefeed
**
** Description: 
** Sets linefeed state
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** newLinefeed: new linefeed state
**
** Return:
** none
*/
int Si3226_SetLinefeedStatus (proslicChanType *pProslic,uInt8 newLinefeed);

/*
** Function: PROSLIC_PolRev
**
** Description: 
** Sets polarity reversal state
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** abrupt: set this to 1 for abrupt pol rev
** newPolRevState: new pol rev state
**
** Return:
** none
*/
int Si3226_PolRev (proslicChanType *pProslic,uInt8 abrupt, uInt8 newPolRevState);

/*
** Function: PROSLIC_GPIOControl
**
** Description: 
** Sets gpio of the proslic
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pGpioData: pointer to gpio status
** read: set to 1 to read status, 0 to write
**
** Return:
** none
*/
int Si3226_GPIOControl (proslicChanType *pProslic,uInt8 *pGpioData, uInt8 read);

/*
** Function: PROSLIC_MWI
**
** Description: 
** implements message waiting indicator
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** lampOn: 0 = turn lamp off, 1 = turn lamp on
**
** Return:
** none
*/
int Si3226_MWI (proslicChanType *pProslic,uInt8 lampOn);

/*
** Function: PROSLIC_StartGenericTone
**
** Description: 
** Initializes and start tone generators
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** timerEn: specifies whether to enable the tone generator timers
**
** Return:
** none
*/
int Si3226_ToneGenStart (proslicChanType *pProslic, uInt8 timerEn);


/*
** Function: PROSLIC_StopTone
**
** Description: 
** Stops tone generators
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si3226_ToneGenStop (proslicChanType *pProslic);


/*
** Function: PROSLIC_StartRing
**
** Description: 
** Initializes and start ring generator
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si3226_RingStart (proslicChanType *pProslic);


/*
** Function: PROSLIC_StopRing
**
** Description: 
** Stops ring generator
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si3226_RingStop (proslicChanType *pProslic);

/*
** Function: PROSLIC_EnableCID
**
** Description: 
** enable fsk
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si3226_EnableCID (proslicChanType *pProslic);

/*
** Function: PROSLIC_DisableCID
**
** Description: 
** disable fsk
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si3226_DisableCID (proslicChanType *pProslic);

/*
** Function: PROSLIC_SendCID
**
** Description: 
** send fsk data
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** buffer: buffer to send
** numBytes: num of bytes in the buffer
**
** Return:
** none
*/
int Si3226_SendCID (proslicChanType *pProslic, uInt8 *buffer, uInt8 numBytes);

int Si3226_CheckCIDBuffer (proslicChanType *pProslic, uInt8 *fsk_buf_avail);

/*
** Function: PROSLIC_StartPCM
**
** Description: 
** Starts PCM
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si3226_PCMStart (proslicChanType *pProslic);


/*
** Function: PROSLIC_StopPCM
**
** Description: 
** Disables PCM
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si3226_PCMStop (proslicChanType *pProslic);

/*
** Function: PROSLIC_DialPulseDetect
**
** Description: 
** implements pulse dial detection and should be called at every off/on hook transistion
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pPulsedialCfg: pointer to pulse dial config
** pPulseDialData: pointer to pulse dial state for current channel
**
** Return:
** none
*/
int Si3226_DialPulseDetect (proslicChanType *pProslic, pulseDial_Cfg *pPulsedialCfg, pulseDialType *pPulseDialData);

/*
** Function: PROSLIC_ReadDTMFDigit
**
** Description: 
** Read DTMF digit (would be called after DTMF interrupt to collect digit)
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pDigit: digit read
**
** Return:
** none
*/
int Si3226_DTMFReadDigit (proslicChanType *pProslic,uInt8 *pDigit);

/*
** Function: PROSLIC_PLLFreeRunStart
**
** Description: 
** initiates pll free run mode
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si3226_PLLFreeRunStart (proslicChanType *pProslic);

/*
** Function: PROSLIC_PLLFreeRunStop
**
** Description: 
** exit pll free run mode
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si3226_PLLFreeRunStop (proslicChanType *pProslic);

int Si3226_LBCal (proslicChanType_ptr *pProslic, int size);
int Si3226_GetLBCalResult (proslicChanType *pProslic,int32*result1,int32*result2,int32 *result3,int32 *result4);
int Si3226_GetLBCalResultPacked (proslicChanType *pProslic,int32 *result);
int Si3226_LoadPreviousLBCal (proslicChanType *pProslic,int32 result1,int32 result2,int32 result3,int32 result4);
int Si3226_LoadPreviousLBCalPacked (proslicChanType *pProslic,int32 *result);
int Si3226_SetLinefeedStatusBroadcast (proslicChanType_ptr hProslic, uInt8 newLinefeed);

/*
** Function: PROSLIC_dbgSetDCFeed
**
** Description: 
** provisionary function for setting up
** dcfeed given desired open circuit voltage 
** and loop current.
*/
int Si3226_dbgSetDCFeed (proslicChanType *pProslic, uInt32 v_vlim_val, uInt32 i_ilim_val, int32 preset);

/*
** Function: PROSLIC_dbgSetDCFeedVopen
**
** Description: 
** provisionary function for setting up
** dcfeed given desired open circuit voltage 
** and loop current.
*/
int Si3226_dbgSetDCFeedVopen (proslicChanType *pProslic, uInt32 v_vlim_val, int32 preset);


/*
** Function: PROSLIC_dbgSetDCFeedIloop
**
** Description: 
** provisionary function for setting up
** dcfeed given desired open circuit voltage 
** and loop current.
*/
int Si3226_dbgSetDCFeedIloop (proslicChanType *pProslic, uInt32 i_ilim_val, int32 preset);


/*
** Function: PROSLIC_dbgRingingSetup
**
** Description: 
** Provisionary function for setting up
** Ring type, frequency, amplitude and dc offset.
** Main use will be by peek/poke applications.
*/

int Si3226_dbgSetRinging (proslicChanType *pProslic, ProSLIC_dbgRingCfg *ringCfg, int preset);

int Si3226_dbgSetRXGain (proslicChanType *pProslic, int32 gain, int impedance_preset, int audio_gain_preset);
int Si3226_dbgSetTXGain (proslicChanType *pProslic, int32 gain, int impedance_preset, int audio_gain_preset);

int Si3226_LineMonitor (proslicChanType *pProslic, proslicMonitorType *monitor);
int Si3226_PSTNCheck (proslicChanType *pProslic,proslicPSTNCheckObjType *pPSTNCheck);

#endif

