/*
** Copyright (c) 2007-2012 by Silicon Laboratories
**
** $Id: proslic.c 3820 2013-02-04 20:23:25Z cdp $
**
** Proslic.c
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
** This is the interface file for the ProSLIC drivers.
**
** Dependancies:
** proslic_datatypes.h
**
*/
#include "si_voice_datatypes.h"
#include "si_voice_ctrl.h"
#include "si_voice_timer_intf.h"
#include "proslic.h"
#include "proslic_api_config.h"

#ifdef WIN32
#include "stdlib.h"
#ifndef DISABLE_MALLOC
#include "memory.h" 
#endif /* DISABLE_MALLOC */
#include "stdlib.h"
#include "string.h"
#else
#ifndef DISABLE_MALLOC
#include <linux/slab.h>
#endif
#endif /* WIN32 */

#ifdef SI321X
#include "si321x.h"
#include "si321x_intf.h"
#endif
#ifdef SI324X
#include "si324x.h"
#include "si324x_intf.h"
#endif
#ifdef SI322X
#include "si3226.h"
#include "si3226_intf.h"
#endif
#ifdef SI3217X
#include "si3217x.h"
#include "si3217x_intf.h"
#endif
#ifdef SI3226X
#include "si3226x.h"
#include "si3226x_intf.h"
#endif
#define pCtrl(X)           (X)->deviceId->ctrlInterface
#define pProHW(X)          pCtrl((X))->hCtrl
#define WriteRAM(X)        (X)->deviceId->ctrlInterface->WriteRAM_fptr
#define ReadRAM(X)         (X)->deviceId->ctrlInterface->ReadRAM_fptr

/*
** Timers
*/
#define TimeElapsed pProslic->deviceId->ctrlInterface->timeElapsed_fptr
#define getTime pProslic->deviceId->ctrlInterface->getTime_fptr
#define pProTimer	pProslic->deviceId->ctrlInterface->hTimer


/*
**
** ProSLIC wrapper functions calling generic SiVoice 
** functions (see si_voice.c for function descriptions)
**
*/
int ProSLIC_createControlInterface (controlInterfaceType **pCtrlIntf){
	return SiVoice_createControlInterface(pCtrlIntf);
}
int ProSLIC_destroyControlInterface (controlInterfaceType **pCtrlIntf){
	return SiVoice_destroyControlInterface (pCtrlIntf);
}
int ProSLIC_createDevice (SiVoiceDeviceType **pDev){
	return SiVoice_createDevice(pDev);
}
int ProSLIC_destroyDevice (SiVoiceDeviceType **pDev){
	return SiVoice_destroyDevice (pDev);
}
int ProSLIC_createChannel (SiVoiceChanType_ptr *pChan){
    return SiVoice_createChannel (pChan);
}
int ProSLIC_destroyChannel (SiVoiceChanType_ptr *pChan){
    return SiVoice_destroyChannel (pChan);
}
int ProSLIC_setControlInterfaceCtrlObj (controlInterfaceType *pCtrlIntf, void *hCtrl){
	return SiVoice_setControlInterfaceCtrlObj(pCtrlIntf,hCtrl);
}
int ProSLIC_setControlInterfaceReset (controlInterfaceType *pCtrlIntf, ctrl_Reset_fptr Reset_fptr){
	return SiVoice_setControlInterfaceReset(pCtrlIntf,Reset_fptr);
}
int ProSLIC_setControlInterfaceWriteRegister (controlInterfaceType *pCtrlIntf, ctrl_WriteRegister_fptr WriteRegister_fptr){
	return SiVoice_setControlInterfaceWriteRegister(pCtrlIntf,WriteRegister_fptr);
}
int ProSLIC_setControlInterfaceReadRegister (controlInterfaceType *pCtrlIntf, ctrl_ReadRegister_fptr ReadRegister_fptr){
	return SiVoice_setControlInterfaceReadRegister (pCtrlIntf,ReadRegister_fptr);
}
int ProSLIC_setControlInterfaceWriteRAM (controlInterfaceType *pCtrlIntf, ctrl_WriteRAM_fptr WriteRAM_fptr){
	return SiVoice_setControlInterfaceWriteRAM(pCtrlIntf,WriteRAM_fptr);
}
int ProSLIC_setControlInterfaceReadRAM (controlInterfaceType *pCtrlIntf, ctrl_ReadRAM_fptr ReadRAM_fptr){
	return SiVoice_setControlInterfaceReadRAM(pCtrlIntf,ReadRAM_fptr);
}
int ProSLIC_setControlInterfaceTimerObj (controlInterfaceType *pCtrlIntf, void *hTimer){
	return SiVoice_setControlInterfaceTimerObj(pCtrlIntf,hTimer);
}
int ProSLIC_setControlInterfaceDelay (controlInterfaceType *pCtrlIntf, system_delay_fptr Delay_fptr){
	return SiVoice_setControlInterfaceDelay(pCtrlIntf,Delay_fptr);
}
int ProSLIC_setControlInterfaceSemaphore (controlInterfaceType *pCtrlIntf, ctrl_Semaphore_fptr semaphore_fptr){
	return SiVoice_setControlInterfaceSemaphore(pCtrlIntf,semaphore_fptr);
}
int ProSLIC_setControlInterfaceTimeElapsed (controlInterfaceType *pCtrlIntf, system_timeElapsed_fptr timeElapsed_fptr){
	return SiVoice_setControlInterfaceTimeElapsed(pCtrlIntf,timeElapsed_fptr);
}
int ProSLIC_setControlInterfaceGetTime (controlInterfaceType *pCtrlIntf, system_getTime_fptr getTime_fptr){
	return SiVoice_setControlInterfaceGetTime(pCtrlIntf,getTime_fptr);
}
int ProSLIC_SWInitChan (proslicChanType_ptr hProslic,int channel,int chipType, ProslicDeviceType*pDeviceObj, controlInterfaceType *pCtrlIntf){
    return SiVoice_SWInitChan (hProslic, channel,chipType,pDeviceObj,pCtrlIntf);
}
int ProSLIC_setSWDebugMode (proslicChanType_ptr hProslic, int debugEn){
    return SiVoice_setSWDebugMode(hProslic,debugEn);
}
int ProSLIC_getErrorFlag (proslicChanType_ptr hProslic, int*error){
	return SiVoice_getErrorFlag (hProslic,error);
}
int ProSLIC_clearErrorFlag (proslicChanType_ptr hProslic){
	return SiVoice_clearErrorFlag (hProslic);
}
int ProSLIC_setChannelEnable (proslicChanType_ptr hProslic, int chanEn){
	return SiVoice_setChannelEnable (hProslic,chanEn);
}
int ProSLIC_getChannelEnable (proslicChanType_ptr hProslic, int* chanEn){
	return SiVoice_getChannelEnable (hProslic,chanEn);
}
int ProSLIC_Reset (proslicChanType_ptr hProslic){
    return SiVoice_Reset(hProslic);
}


/*
** ProSLIC device driver adapters
*/

int ProSLIC_ShutdownChannel (proslicChanType_ptr hProslic){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return 1;
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return 1;
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_ShutdownChannel(hProslic);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_ShutdownChannel(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_ShutdownChannel(hProslic);
#endif
	return 1;
}


int32 ProSLIC_ReadMADCScaled(proslicChanType_ptr hProslic,uInt16 addr,int32 scale){
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_ReadMADCScaled(hProslic,addr,scale);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_ReadMADCScaled(hProslic,addr,scale);
#endif
return 255;
}

uInt8 ProSLIC_ReadReg(proslicChanType_ptr hProslic,uInt8 addr){
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_ReadReg(hProslic,addr);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_ReadReg(hProslic,addr);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_ReadReg(hProslic,addr);
#endif
return 255;
}

int ProSLIC_WriteReg(proslicChanType_ptr hProslic,uInt8 addr,uInt8 data){
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_WriteReg(hProslic,addr,data);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_WriteReg(hProslic,addr,data);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_WriteReg(hProslic,addr,data);
#endif
return 255;
}

ramData ProSLIC_ReadRAM(proslicChanType_ptr hProslic,uInt16 addr){
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return (ReadRAM(hProslic)(pProHW(hProslic), hProslic->channel, addr));
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return (ReadRAM(hProslic)(pProHW(hProslic), hProslic->channel, addr));
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return (ReadRAM(hProslic)(pProHW(hProslic), hProslic->channel, addr));
#endif
	return 0xFF;
}

int ProSLIC_WriteRAM(proslicChanType_ptr hProslic,uInt16 addr,ramData data){
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return(WriteRAM(hProslic)(pProHW(hProslic), hProslic->channel,addr,data));
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return(WriteRAM(hProslic)(pProHW(hProslic), hProslic->channel,addr,data));
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return(WriteRAM(hProslic)(pProHW(hProslic), hProslic->channel,addr,data));
#endif
	return RC_UNSUPPORTED_FEATURE;
}

int ProSLIC_PrintDebugData(proslicChanType_ptr hProslic){
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_PrintDebugData(hProslic);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_PrintDebugData(hProslic);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_PrintDebugData(hProslic);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_PrintDebugData(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_PrintDebugData(hProslic);
#endif
	return RC_UNSUPPORTED_FEATURE;
}

int ProSLIC_PrintDebugReg(proslicChanType_ptr hProslic){
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
        return RC_UNSUPPORTED_FEATURE;
		/* return Si324x_PrintDebugReg(hProslic); */
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
        return RC_UNSUPPORTED_FEATURE;
		/* return Si321x_PrintDebugReg(hProslic); */
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_PrintDebugReg(hProslic);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_PrintDebugReg(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_PrintDebugReg(hProslic);
#endif
	return RC_UNSUPPORTED_FEATURE;
}

int ProSLIC_PrintDebugRAM(proslicChanType_ptr hProslic){
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_PrintDebugRAM(hProslic);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_PrintDebugRAM(hProslic);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_PrintDebugRAM(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_PrintDebugRAM(hProslic);
#endif
    return RC_UNSUPPORTED_FEATURE;
}

int ProSLIC_VerifyControlInterface(proslicChanType_ptr hProslic){
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_VerifyControlInterface(hProslic);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_VerifyControlInterface(hProslic);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_VerifyControlInterface(hProslic);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_VerifyControlInterface(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_VerifyControlInterface(hProslic);
#endif
	return 1;
}

#ifdef SIVOICE_MULTI_BOM_SUPPORT
int ProSLIC_Init_MultiBOM (proslicChanType_ptr *hProslic,int size, int preset){
#ifdef SI324X
	if ((*hProslic)->deviceId->chipType >= SI3240 && (*hProslic)->deviceId->chipType <= SI3247)
		return Si324x_Init(hProslic,size);
#endif
#ifdef SI322X
	if ((*hProslic)->deviceId->chipType >= SI3226 && (*hProslic)->deviceId->chipType <= SI3227)
		return Si3226_Init_MultiBOM(hProslic,size,preset);
#endif
#ifdef SI321X
	if ((*hProslic)->deviceId->chipType >= SI3210 && (*hProslic)->deviceId->chipType <= SI3216M)
		return Si321x_Init(hProslic,size);
#endif
#ifdef SI3217X
	if ((*hProslic)->deviceId->chipType >= SI32171 && (*hProslic)->deviceId->chipType <= SI32179)
		return Si3217x_Init_MultiBOM(hProslic,size,preset);
#endif
#ifdef SI3226X
	if ((*hProslic)->deviceId->chipType >= SI32260 && (*hProslic)->deviceId->chipType <= SI32269)
		return Si3226x_Init_MultiBOM(hProslic,size,preset);
#endif
	return 1;
}
#endif

int ProSLIC_Init (proslicChanType_ptr *hProslic,int size){
#ifdef SI324X
	if ((*hProslic)->deviceId->chipType >= SI3240 && (*hProslic)->deviceId->chipType <= SI3247)
		return Si324x_Init(hProslic,size);
#endif
#ifdef SI322X
	if ((*hProslic)->deviceId->chipType >= SI3226 && (*hProslic)->deviceId->chipType <= SI3227)
		return Si3226_Init(hProslic,size);
#endif
#ifdef SI321X
	if ((*hProslic)->deviceId->chipType >= SI3210 && (*hProslic)->deviceId->chipType <= SI3216M)
		return Si321x_Init(hProslic,size);
#endif
#ifdef SI3217X
	if ((*hProslic)->deviceId->chipType >= SI32171 && (*hProslic)->deviceId->chipType <= SI32179)
		return Si3217x_Init(hProslic,size);
#endif
#ifdef SI3226X
	if ((*hProslic)->deviceId->chipType >= SI32260 && (*hProslic)->deviceId->chipType <= SI32269)
		return Si3226x_Init(hProslic,size);
#endif
	return 1;
}

/* Deprecated */
int ProSLIC_InitBroadcast (proslicChanType_ptr *hProslic){
#ifdef SI324X
	if ((*hProslic)->deviceId->chipType >= SI3240 && (*hProslic)->deviceId->chipType <= SI3247)
		return Si324x_InitBroadcast(hProslic);
#endif
#ifdef SI321X
	if ((*hProslic)->deviceId->chipType >= SI3210 && (*hProslic)->deviceId->chipType <= SI3216M)
		return Si321x_InitBroadcast(hProslic);
#endif
    SILABS_UNREFERENCED_PARAMETER(hProslic);
	return RC_UNSUPPORTED_FEATURE;
}


int ProSLIC_Reinit (proslicChanType_ptr hProslic,int size){
#ifdef SI324X
	if ((hProslic)->deviceId->chipType >= SI3240 && (hProslic)->deviceId->chipType <= SI3247)
		return Si324x_Init(&hProslic,size);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_Reinit(hProslic,size);
#endif
#ifdef SI321X
	if ((hProslic)->deviceId->chipType >= SI3210 && (hProslic)->deviceId->chipType <= SI3216M)
		return Si321x_Init(&hProslic,size);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_Reinit(hProslic,size);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_Reinit(hProslic,size);
#endif
	return 1;
}



int ProSLIC_LoadRegTables (proslicChanType_ptr *hProslic,ProslicRAMInit *pRamTable, ProslicRegInit *pRegTable,int size){
		if((*hProslic)->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if ((*hProslic)->deviceId->chipType >= SI3240 && (*hProslic)->deviceId->chipType <= SI3247)
		return Si324x_LoadRegTables(hProslic,pRamTable,pRegTable,size);
#endif
#ifdef SI322X
	if ((*hProslic)->deviceId->chipType >= SI3226 && (*hProslic)->deviceId->chipType <= SI3227)
		return Si3226_LoadRegTables(hProslic,pRamTable,pRegTable,size);
#endif
#ifdef SI321X
	if ((*hProslic)->deviceId->chipType >= SI3210 && (*hProslic)->deviceId->chipType <= SI3216M)
		return Si321x_LoadRegTables(hProslic,pRamTable,pRegTable,size);
#endif
#ifdef SI3217X
	if ((*hProslic)->deviceId->chipType >= SI32171 && (*hProslic)->deviceId->chipType <= SI32179)
		return Si3217x_LoadRegTables(hProslic,pRamTable,pRegTable,size);
#endif
#ifdef SI3226X
	if ((*hProslic)->deviceId->chipType >= SI32260 && (*hProslic)->deviceId->chipType <= SI32269)
		return Si3226x_LoadRegTables(hProslic,pRamTable,pRegTable,size);
#endif
	return 1;
}

int ProSLIC_LoadPatch (proslicChanType_ptr hProslic,proslicPatch *pPatch){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_LoadPatch(hProslic,pPatch);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_LoadPatch(hProslic,pPatch);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_LoadPatch(hProslic,pPatch);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_LoadPatch(hProslic,pPatch);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_LoadPatch(hProslic,pPatch);
#endif
	return 1;
}

int ProSLIC_VerifyPatch (proslicChanType_ptr hProslic,proslicPatch *pPatch){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_VerifyPatch(hProslic,pPatch);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_VerifyPatch(hProslic,pPatch);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return 1;
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_VerifyPatch(hProslic,pPatch);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_VerifyPatch(hProslic,pPatch);
#endif
	return 1;
}

int ProSLIC_SetMuteStatus (proslicChanType_ptr hProslic, ProslicMuteModes muteEn){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_SetMuteStatus(hProslic,muteEn);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_SetMuteStatus(hProslic,muteEn);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_SetMuteStatus(hProslic,muteEn);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_SetMuteStatus(hProslic,muteEn);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_SetMuteStatus(hProslic,muteEn);
#endif
	return 1;
}

int ProSLIC_SetLoopbackMode (proslicChanType_ptr hProslic, ProslicLoopbackModes newMode){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_SetLoopbackMode(hProslic,newMode);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_SetLoopbackMode(hProslic,newMode);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_SetLoopbackMode(hProslic,newMode);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_SetLoopbackMode(hProslic,newMode);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_SetLoopbackMode(hProslic,newMode);
#endif
	return 1;
}

int ProSLIC_EnableInterrupts (proslicChanType_ptr hProslic){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_EnableInterrupts(hProslic);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_EnableInterrupts(hProslic);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_EnableInterrupts(hProslic);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_EnableInterrupts(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_EnableInterrupts(hProslic);
#endif
	return 1;
}

int ProSLIC_DisableInterrupts (proslicChanType_ptr hProslic){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_DisableInterrupts(hProslic);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_DisableInterrupts(hProslic);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_DisableInterrupts(hProslic);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_DisableInterrupts(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_DisableInterrupts(hProslic);
#endif
	return RC_IGNORE;
}

int ProSLIC_RingSetup (proslicChanType_ptr hProslic,int preset){
#ifndef DISABLE_RING_SETUP
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_RingSetup(hProslic,preset);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_RingSetup(hProslic,preset);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_RingSetup(hProslic,preset);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_RingSetup(hProslic,preset);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_RingSetup(hProslic,preset);
#endif
#endif /*DISABLE_RING_SETUP*/
	return 1;
}

int ProSLIC_ToneGenSetup (proslicChanType_ptr hProslic,int preset){
#ifndef DISABLE_TONE_SETUP
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_ToneGenSetup(hProslic,preset);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_ToneGenSetup(hProslic,preset);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_ToneGenSetup(hProslic,preset);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_ToneGenSetup(hProslic,preset);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_ToneGenSetup(hProslic,preset);
#endif
#endif /*DISABLE_TONE_SETUP*/
	return 1;
}

int ProSLIC_FSKSetup (proslicChanType_ptr hProslic,int preset){
#ifndef DISABLE_FSK_SETUP
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_FSKSetup(hProslic,preset);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_FSKSetup(hProslic,preset);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_FSKSetup(hProslic,preset);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_FSKSetup(hProslic,preset);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_FSKSetup(hProslic,preset);
#endif
#endif /*DISABLE_FSK_SETUP*/
	return 1;
}

int ProSLIC_DTMFDecodeSetup (proslicChanType_ptr hProslic,int preset){
#ifndef DISABLE_DTMF_SETUP
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_DTMFDecodeSetup(hProslic,preset);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_DTMFDecodeSetup(hProslic,preset);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_DTMFDecodeSetup(hProslic,preset);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_DTMFDecodeSetup(hProslic,preset);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_DTMFDecodeSetup(hProslic,preset);
#endif
#endif /*DISABLE_DTMF_SETUP*/
    SILABS_UNREFERENCED_PARAMETER(hProslic);
    SILABS_UNREFERENCED_PARAMETER(preset);
    return 1;
}

int ProSLIC_ZsynthSetup (proslicChanType_ptr hProslic,int preset){
#ifndef DISABLE_ZSYNTH_SETUP
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_ZsynthSetup(hProslic,preset);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_ZsynthSetup(hProslic,preset);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_ZsynthSetup(hProslic,preset);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_ZsynthSetup(hProslic,preset);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_ZsynthSetup(hProslic,preset);
#endif
#endif /*DISABLE_ZSYNTH_SETUP*/
	return 1;
}

int ProSLIC_GciCISetup (proslicChanType_ptr hProslic,int preset){
#ifndef DISABLE_CI_SETUP
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_GciCISetup(hProslic,preset);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_GciCISetup(hProslic,preset);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_GciCISetup(hProslic,preset);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_GciCISetup(hProslic,preset);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_GciCISetup(hProslic,preset);
#endif
#endif /*DISABLE_CI_SETUP*/
	return 1;
}

int ProSLIC_ModemDetSetup (proslicChanType_ptr hProslic,int preset){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_ModemDetSetup(hProslic,preset);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_ModemDetSetup(hProslic,preset);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_ModemDetSetup(hProslic,preset);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_ModemDetSetup(hProslic,preset);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_ModemDetSetup(hProslic,preset);
#endif
	return 1;
}

int ProSLIC_TXAudioGainSetup (proslicChanType_ptr hProslic,int preset){
#ifndef DISABLE_AUDIOGAIN_SETUP
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_TXAudioGainSetup(hProslic,preset);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_TXAudioGainSetup(hProslic,preset);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_TXAudioGainSetup(hProslic,preset);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_TXAudioGainSetup(hProslic,preset);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_TXAudioGainSetup(hProslic,preset);
#endif
#endif /*DISABLE_AUDIOGAIN_SETUP*/
	return 1;
}

int ProSLIC_RXAudioGainSetup (proslicChanType_ptr hProslic,int preset){
#ifndef DISABLE_AUDIOGAIN_SETUP
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_RXAudioGainSetup(hProslic,preset);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_RXAudioGainSetup(hProslic,preset);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_RXAudioGainSetup(hProslic,preset);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_RXAudioGainSetup(hProslic,preset);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_RXAudioGainSetup(hProslic,preset);
#endif
#endif /*DISABLE_AUDIOGAIN_SETUP*/
	return 1;
}

int ProSLIC_TXAudioGainScale (proslicChanType_ptr hProslic,int preset, uInt32 pga_scale, uInt32 eq_scale){
#ifndef DISABLE_AUDIOGAIN_SETUP
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return RC_UNSUPPORTED_FEATURE;
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_TXAudioGainScale(hProslic,preset,pga_scale,eq_scale);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return RC_UNSUPPORTED_FEATURE;
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_TXAudioGainScale(hProslic,preset,pga_scale,eq_scale);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_TXAudioGainScale(hProslic,preset,pga_scale,eq_scale);
#endif
#endif /*DISABLE_AUDIOGAIN_SETUP*/
	return 1;
}

int ProSLIC_RXAudioGainScale (proslicChanType_ptr hProslic,int preset, uInt32 pga_scale, uInt32 eq_scale){
#ifndef DISABLE_AUDIOGAIN_SETUP
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return RC_UNSUPPORTED_FEATURE;
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_RXAudioGainScale(hProslic,preset,pga_scale,eq_scale);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return RC_UNSUPPORTED_FEATURE;
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_RXAudioGainScale(hProslic,preset,pga_scale,eq_scale);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_RXAudioGainScale(hProslic,preset,pga_scale,eq_scale);
#endif
#endif /*DISABLE_AUDIOGAIN_SETUP*/
	return 1;
}

int ProSLIC_AudioGainSetup (proslicChanType_ptr pProslic,int32 rxgain, int32 txgain,int preset){
    int rc = RC_IGNORE;
#ifndef DISABLE_AUDIOGAIN_SETUP
    int atx_preset = TXACGAIN_SEL;
    int arx_preset = RXACGAIN_SEL;

#ifdef SI321X
	if (pProslic->deviceId->chipType >= SI3210 && pProslic->deviceId->chipType <= SI3216M)
		Si321x_CalcAG(pProslic,rxgain, txgain, &arx_preset, &atx_preset);
#endif
   
    rc = ProSLIC_dbgSetTXGain(pProslic,txgain,preset,atx_preset);

    if( rc  == RC_NONE) {
        rc = ProSLIC_TXAudioGainSetup(pProslic,TXACGAIN_SEL);
    }

    if( rc  == RC_NONE) {
        rc = ProSLIC_dbgSetRXGain(pProslic,rxgain,preset,arx_preset);
    }

    if( rc  == RC_NONE) {
        rc = ProSLIC_RXAudioGainSetup(pProslic,RXACGAIN_SEL);
    }

#endif /*DISABLE_AUDIOGAIN_SETUP*/
	return rc;
}

int ProSLIC_DCFeedSetup (proslicChanType_ptr hProslic,int preset){
#ifndef DISABLE_DCFEED_SETUP
	if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_DCFeedSetup(hProslic,preset);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_DCFeedSetup(hProslic,preset);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_DCFeedSetup(hProslic,preset);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_DCFeedSetup(hProslic,preset);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_DCFeedSetup(hProslic,preset);
#endif
#endif /*DISABLE_DCFEED_SETUP*/
	return 1;
}


int ProSLIC_DCFeedSetupCfg (proslicChanType_ptr hProslic, ProSLIC_DCfeed_Cfg *cfg, int preset){
#ifndef DISABLE_DCFEED_SETUP
	if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_DCFeedSetupCfg(hProslic,cfg,preset);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_DCFeedSetupCfg(hProslic,cfg,preset);
#endif
#endif /*DISABLE_DCFEED_SETUP*/
	return 1;
}

int ProSLIC_GPIOSetup (proslicChanType_ptr hProslic){
#ifndef DISABLE_GPIO_SETUP
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_GPIOSetup(hProslic);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_GPIOSetup(hProslic);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return 1;
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_GPIOSetup(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_GPIOSetup(hProslic);
#endif
#endif /*DISABLE_GPIO_SETUP*/
	return 1;
}

int ProSLIC_PulseMeterSetup (proslicChanType_ptr hProslic,int preset){
#ifndef DISABLE_PULSE_SETUP
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_PulseMeterSetup(hProslic,preset);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return 1;
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_PulseMeterSetup(hProslic,preset);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_PulseMeterSetup(hProslic,preset);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_PulseMeterSetup(hProslic,preset);
#endif
#endif /*DISABLE_PULSE_SETUP*/
	return 1;
}

int ProSLIC_PCMSetup (proslicChanType_ptr hProslic,int preset){
#ifndef DISABLE_PCM_SETUP
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_PCMSetup(hProslic,preset);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_PCMSetup(hProslic,preset);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_PCMSetup(hProslic,preset);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_PCMSetup(hProslic,preset);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_PCMSetup(hProslic,preset);
#endif
#endif /*DISABLE_PCM_SETUP*/
	return 1;
}

int ProSLIC_PCMTimeSlotSetup (proslicChanType_ptr hProslic, uInt16 rxcount, uInt16 txcount){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_PCMTimeSlotSetup(hProslic,rxcount,txcount);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_PCMTimeSlotSetup(hProslic,rxcount,txcount);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_PCMTimeSlotSetup(hProslic,rxcount,txcount);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_PCMTimeSlotSetup(hProslic,rxcount,txcount);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_PCMTimeSlotSetup(hProslic,rxcount,txcount);
#endif
	return 1;
}

int ProSLIC_GetInterrupts (proslicChanType_ptr hProslic,proslicIntType *pIntData){
	pIntData->number=0;
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_GetInterrupts(hProslic,pIntData);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_GetInterrupts(hProslic,pIntData);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_GetInterrupts(hProslic,pIntData);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_GetInterrupts(hProslic,pIntData);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_GetInterrupts(hProslic,pIntData);
#endif
	return 1;
}

int ProSLIC_ReadHookStatus (proslicChanType_ptr hProslic,uInt8 *pHookStat){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_ReadHookStatus(hProslic,pHookStat);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_ReadHookStatus(hProslic,pHookStat);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_ReadHookStatus(hProslic,pHookStat);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_ReadHookStatus(hProslic,pHookStat);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_ReadHookStatus(hProslic,pHookStat);
#endif
	return 1;
}

int ProSLIC_SetLinefeedStatus (proslicChanType_ptr hProslic, uInt8 newLinefeed){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_SetLinefeedStatus(hProslic,newLinefeed);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_SetLinefeedStatus(hProslic,newLinefeed);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_SetLinefeedStatus(hProslic,newLinefeed);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_SetLinefeedStatus(hProslic,newLinefeed);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_SetLinefeedStatus(hProslic,newLinefeed);
#endif
	return 1;
}

int ProSLIC_SetLinefeedStatusBroadcast (proslicChanType_ptr hProslic, uInt8 newLinefeed){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_SetLinefeedStatusBroadcast(hProslic,newLinefeed);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_SetLinefeedStatusBroadcast(hProslic,newLinefeed);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_SetLinefeedStatusBroadcast(hProslic,newLinefeed);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_SetLinefeedStatusBroadcast(hProslic,newLinefeed);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_SetLinefeedStatusBroadcast(hProslic,newLinefeed);
#endif
	return 1;
}

int ProSLIC_PolRev (proslicChanType_ptr hProslic,uInt8 abrupt,uInt8 newPolRevState){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_PolRev(hProslic,abrupt,newPolRevState);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_PolRev(hProslic,abrupt,newPolRevState);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_PolRev(hProslic,abrupt,newPolRevState);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_PolRev(hProslic,abrupt,newPolRevState);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_PolRev(hProslic,abrupt,newPolRevState);
#endif
	return 1;
}

int ProSLIC_GPIOControl (proslicChanType_ptr hProslic,uInt8 *pGpioData, uInt8 read){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_GPIOControl(hProslic,pGpioData,read);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_GPIOControl(hProslic,pGpioData,read);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_GPIOControl(hProslic,pGpioData,read);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_GPIOControl(hProslic,pGpioData,read);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_GPIOControl(hProslic,pGpioData,read);
#endif
	return 1;
}

/*
** Optional Neon Message Waiting Support
*/
#ifdef SIVOICE_NEON_MWI_SUPPORT
int ProSLIC_MWISetup (proslicChanType_ptr hProslic, uInt16 vpk_mag, uInt16 lcrmask_mwi){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return RC_UNSUPPORTED_FEATURE;
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return RC_UNSUPPORTED_FEATURE;
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return RC_UNSUPPORTED_FEATURE;
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_MWISetup(hProslic,vpk_mag,lcrmask_mwi);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_MWISetup(hProslic,vpk_mag,lcrmask_mwi);
#endif
	return 1;
}
#endif

#ifdef SIVOICE_NEON_MWI_SUPPORT
int ProSLIC_MWIEnable (proslicChanType_ptr hProslic){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return RC_UNSUPPORTED_FEATURE;
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return RC_UNSUPPORTED_FEATURE;
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return RC_UNSUPPORTED_FEATURE;
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_MWIEnable(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_MWIEnable(hProslic);
#endif
	return 1;
}
#endif

#ifdef SIVOICE_NEON_MWI_SUPPORT
int ProSLIC_MWIDisable (proslicChanType_ptr hProslic){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return RC_UNSUPPORTED_FEATURE;
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return RC_UNSUPPORTED_FEATURE;
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return RC_UNSUPPORTED_FEATURE;
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_MWIDisable(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_MWIDisable(hProslic);
#endif
	return 1;
}
#endif

#ifdef SIVOICE_NEON_MWI_SUPPORT
int ProSLIC_SetMWIState (proslicChanType_ptr hProslic,uInt8 flash_on){
	if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return RC_UNSUPPORTED_FEATURE;
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return RC_UNSUPPORTED_FEATURE;
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return RC_UNSUPPORTED_FEATURE;
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_SetMWIState(hProslic,flash_on);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_SetMWIState(hProslic,flash_on);
#endif
	return 1;
}
#endif


#ifdef SIVOICE_NEON_MWI_SUPPORT
int ProSLIC_GetMWIState (proslicChanType_ptr hProslic){
	if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return RC_UNSUPPORTED_FEATURE;
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return RC_UNSUPPORTED_FEATURE;
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return RC_UNSUPPORTED_FEATURE;
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_GetMWIState(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_GetMWIState(hProslic);
#endif
	return 1;
}
#endif

#ifdef SIVOICE_NEON_MWI_SUPPORT
int ProSLIC_MWI (proslicChanType_ptr hProslic,uInt8 lampOn){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_MWI(hProslic,lampOn);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_MWI(hProslic,lampOn);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_MWI(hProslic,lampOn);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_MWI(hProslic,lampOn);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_MWI(hProslic,lampOn);
#endif
	return 1;
}
#endif


int ProSLIC_ToneGenStart (proslicChanType_ptr hProslic,uInt8 timerEn){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_ToneGenStart(hProslic,timerEn);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_ToneGenStart(hProslic,timerEn);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_ToneGenStart(hProslic,timerEn);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_ToneGenStart(hProslic,timerEn);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_ToneGenStart(hProslic,timerEn);
#endif
	return 1;
}

int ProSLIC_ToneGenStop (proslicChanType_ptr hProslic){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_ToneGenStop(hProslic);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_ToneGenStop(hProslic);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_ToneGenStop(hProslic);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_ToneGenStop(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_ToneGenStop(hProslic);
#endif
	return 1;
}

int ProSLIC_RingStart (proslicChanType_ptr hProslic){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_RingStart(hProslic);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_RingStart(hProslic);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_RingStart(hProslic);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_RingStart(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_RingStart(hProslic);
#endif
	return 1;
}

int ProSLIC_RingStop (proslicChanType_ptr hProslic){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_RingStop(hProslic);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_RingStop(hProslic);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_RingStop(hProslic);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_RingStop(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_RingStop(hProslic);
#endif
	return 1;
}

int ProSLIC_CheckCIDBuffer (proslicChanType_ptr hProslic, uInt8 *fsk_buf_avail){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_CheckCIDBuffer(hProslic,fsk_buf_avail);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_CheckCIDBuffer(hProslic,fsk_buf_avail);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_CheckCIDBuffer(hProslic,fsk_buf_avail);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_CheckCIDBuffer(hProslic,fsk_buf_avail);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_CheckCIDBuffer(hProslic,fsk_buf_avail);
#endif
	return 1;

}

int ProSLIC_EnableCID (proslicChanType_ptr hProslic){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_EnableCID(hProslic);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_EnableCID(hProslic);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_EnableCID(hProslic);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_EnableCID(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_EnableCID(hProslic);
#endif
	return 1;
}

int ProSLIC_DisableCID (proslicChanType_ptr hProslic){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_DisableCID(hProslic);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_DisableCID(hProslic);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_DisableCID(hProslic);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_DisableCID(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_DisableCID(hProslic);
#endif
	return 1;
}

int ProSLIC_SendCID (proslicChanType_ptr hProslic, uInt8 *buffer, uInt8 numBytes){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_SendCID(hProslic,buffer,numBytes);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_SendCID(hProslic,buffer,numBytes);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_SendCID(hProslic,buffer,numBytes);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_SendCID(hProslic,buffer,numBytes);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_SendCID(hProslic,buffer,numBytes);
#endif
	return 1;
}


int ProSLIC_ModifyCIDStartBits(proslicChanType_ptr hProslic, uInt8 enable_startStop)
{
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_ModifyCIDStartBits(hProslic, enable_startStop);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_ModifyCIDStartBits(hProslic, enable_startStop);
#endif

	return RC_UNSUPPORTED_FEATURE;

}

int ProSLIC_PCMStart (proslicChanType_ptr hProslic){
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_PCMStart(hProslic);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_PCMStart(hProslic);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_PCMStart(hProslic);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_PCMStart(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_PCMStart(hProslic);
#endif
	return 1;
}

int ProSLIC_PCMStop (proslicChanType_ptr hProslic){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_PCMStop(hProslic);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_PCMStop(hProslic);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_PCMStop(hProslic);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_PCMStop(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_PCMStop(hProslic);
#endif
	return 1;
}

/*
** Function: ProSLIC_InitializeDialPulseDetect
**
** Description: 
** Initialize dial pulse detection parameters
*/
int ProSLIC_InitializeDialPulseDetect(pulseDialType *pPulse,void *offHookTime,void *onHookTime){
	pPulse->offHookTime = offHookTime;
	pPulse->onHookTime = onHookTime;
	pPulse->currentPulseDigit = 0;
    return 0;
}

/*
** Function: PROSLIC_DialPulseDetect
**
** Description: 
** implements pulse dial detection and should be called at every hook transistion
*/
int ProSLIC_DialPulseDetect (proslicChanType *pProslic, pulseDial_Cfg *pPulsedialCfg, pulseDialType *pPulseDialData){
	uInt8 hookStat;
	int breaktime;
	int offHk, onHk;
	

	
	TimeElapsed(pProTimer,pPulseDialData->onHookTime,&onHk); /*get onhook time*/
	
	TimeElapsed(pProTimer,pPulseDialData->offHookTime,&offHk); /*get offhook time*/
	
	ProSLIC_ReadHookStatus(pProslic,&hookStat);
	if (hookStat == PROSLIC_ONHOOK){
		/*we are on-hook. */
		
		getTime(pProTimer,pPulseDialData->onHookTime); /*set onhooktime*/
	}
	else{
		/*we are off-hook.*/
		
		breaktime = onHk;
		if ((breaktime >= (pPulsedialCfg->minOnHook)) && (breaktime <= (pPulsedialCfg->maxOnHook))){
       		pPulseDialData->currentPulseDigit++;
			
		}
		else {
			
			return 1;
		}
		getTime(pProTimer,pPulseDialData->offHookTime); 
	}
	
	return 0; 
}
int ProSLIC_DialPulseDetectTimeout (proslicChanType *pProslic, pulseDial_Cfg *pPulsedialCfg, pulseDialType *pPulseDialData){
	/*Pulse dial detect handling code start*/
		uInt8 HkStat;
		int time; uInt8 digit=0;
		ProSLIC_ReadHookStatus(pProslic,&HkStat);
		if (HkStat == PROSLIC_ONHOOK){
			TimeElapsed(pProTimer,pPulseDialData->onHookTime,&time);
			if (time >  pPulsedialCfg->maxOnHook){
					
				return ON_HOOK_TIMEOUT;
			}
			    
		}
		if (HkStat == PROSLIC_OFFHOOK && pPulseDialData->currentPulseDigit > 0){
			TimeElapsed(pProTimer,pPulseDialData->offHookTime,&time);
			if(time > pPulsedialCfg->maxOffHook){
				digit = pPulseDialData->currentPulseDigit;
				pPulseDialData->currentPulseDigit = 0;
					
				return digit;
			}
		}
		/*Pulse dial detect handling code end*/
		return 0;
}


/*
** Function: ProSLIC_ResetDialPulseDetect
**
** Description: 
** reset dial pulse detection state machine (helper function for 
** ProSLIC_InitializeHookChangeDetect.
*/
static void ProSLIC_ResetDialPulseDetect(hookChangeType *pPulse)
{
	pPulse->currentPulseDigit = 0;
	pPulse->lookingForTimeout = 0;
	pPulse->last_hook_state = 5; /* this is invalid */
}

/*
** Function: ProSLIC_InitializeHookChangeDetect
**
** Description: 
** Initialize dial pulse detection parameters
*/
int ProSLIC_InitializeHookChangeDetect(hookChangeType *pPulse,void *hookTime)
{
	pPulse->hookTime = hookTime;
    pPulse->last_state_reported =  SI_HC_NO_ACTIVITY;
	ProSLIC_ResetDialPulseDetect(pPulse);
	return RC_NONE;
}

/*
** Function: ProSLIC_HookChangeDetect
**
** Description: 
** implements pulse dial detection and should be called at every hook transistion
*/
uInt8 ProSLIC_HookChangeDetect (proslicChanType *pProslic, hookChange_Cfg *pHookChangeCfg, hookChangeType *pHookChangeData){
	uInt8 hookStat;
	int delta_time;
	
	TimeElapsed(pProTimer,pHookChangeData->hookTime,&delta_time); 
	ProSLIC_ReadHookStatus(pProslic,&hookStat);

	/* Did we have a hook state change? */
	if(hookStat !=  pHookChangeData->last_hook_state)
	{
		pHookChangeData->last_hook_state = hookStat;
		getTime(pProTimer,pHookChangeData->hookTime); 
	    pHookChangeData->lookingForTimeout = 1;

		if (hookStat == PROSLIC_OFFHOOK)
		{
			if ((delta_time >= (pHookChangeCfg->minOnHook)) && (delta_time <= (pHookChangeCfg->maxOnHook)))
			{
				pHookChangeData->currentPulseDigit++;
			}
            else
            {
                /* Did we see a hook flash? */
                if( (delta_time >= pHookChangeCfg->minHookFlash) && (delta_time <= pHookChangeCfg->maxHookFlash) )
                {
                    pHookChangeData->last_state_reported = SI_HC_HOOKFLASH;
                    ProSLIC_ResetDialPulseDetect(pHookChangeData);
                    return SI_HC_HOOKFLASH;
                }
            }
        }	

		return SI_HC_NEED_MORE_POLLS; 
	}

    if( (pHookChangeData->lookingForTimeout == 1) 
                && (delta_time >=  pHookChangeCfg->minInterDigit) )
    {

            if(delta_time  > pHookChangeCfg->minHook)
            {
                if(pHookChangeData->last_hook_state == PROSLIC_ONHOOK)
                {
	                ProSLIC_ResetDialPulseDetect(pHookChangeData);
                    pHookChangeData->last_state_reported = SI_HC_ONHOOK_TIMEOUT;
                    return SI_HC_ONHOOK_TIMEOUT;
                }

                if(pHookChangeData->last_hook_state == PROSLIC_OFFHOOK)
                {
	                ProSLIC_ResetDialPulseDetect(pHookChangeData);

                    /* Check if we saw either a pulse digit or hook flash prior to this,
                     * if so, we're already offhook, so do not report a offhook event,
                     * just stop polling.
                     */
                    if((pHookChangeData->last_state_reported == SI_HC_ONHOOK_TIMEOUT)
                            || (pHookChangeData->last_state_reported == SI_HC_NO_ACTIVITY) )
                    {
                        pHookChangeData->last_state_reported = SI_HC_OFFHOOK_TIMEOUT;
                        return SI_HC_OFFHOOK_TIMEOUT;
                    }
                    else
                    {
                        return SI_HC_NO_ACTIVITY;
                    }
                }
            }
            else
            {
                int last_digit = pHookChangeData->currentPulseDigit;

                if(last_digit)
                {
                    pHookChangeData->last_state_reported = last_digit;
	                ProSLIC_ResetDialPulseDetect(pHookChangeData);
				    return last_digit;
                }
            }
		    return SI_HC_NEED_MORE_POLLS; 
    }
 	
    return SI_HC_NEED_MORE_POLLS; 
}

int ProSLIC_DTMFReadDigit (proslicChanType_ptr hProslic,uInt8 *pDigit){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_DTMFReadDigit (hProslic,pDigit);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_DTMFReadDigit (hProslic,pDigit);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_DTMFReadDigit (hProslic,pDigit);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_DTMFReadDigit(hProslic,pDigit);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_DTMFReadDigit(hProslic,pDigit);
#endif
	return 1;
}

int ProSLIC_PLLFreeRunStart (proslicChanType_ptr hProslic){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_PLLFreeRunStart (hProslic);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_PLLFreeRunStart (hProslic);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_PLLFreeRunStart (hProslic);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_PLLFreeRunStart(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_PLLFreeRunStart(hProslic);
#endif
	return 1;
}

int ProSLIC_PLLFreeRunStop (proslicChanType_ptr hProslic){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_PLLFreeRunStop (hProslic);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_PLLFreeRunStop (hProslic);
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_PLLFreeRunStop (hProslic);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_PLLFreeRunStop(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_PLLFreeRunStop(hProslic);
#endif
	return 1;
}

int ProSLIC_PulseMeterEnable (proslicChanType_ptr hProslic){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_PulseMeterEnable (hProslic);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return 1;
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_PulseMeterEnable (hProslic);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_PulseMeterEnable(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_PulseMeterEnable(hProslic);
#endif
	return 1;
}


int ProSLIC_PulseMeterDisable (proslicChanType_ptr hProslic){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_PulseMeterDisable (hProslic);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return 1;
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_PulseMeterDisable (hProslic);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_PulseMeterDisable(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_PulseMeterDisable(hProslic);
#endif
	return 1;
}


int ProSLIC_PulseMeterStart (proslicChanType_ptr hProslic){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_PulseMeterStart (hProslic);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return 1;
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_PulseMeterStart (hProslic);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_PulseMeterStart(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_PulseMeterStart(hProslic);
#endif
	return 1;
}

int ProSLIC_PulseMeterStop (proslicChanType_ptr hProslic){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return Si324x_PulseMeterStop (hProslic);
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return 1;
#endif
#ifdef SI321X
	if (hProslic->deviceId->chipType >= SI3210 && hProslic->deviceId->chipType <= SI3216M)
		return Si321x_PulseMeterStop (hProslic);
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_PulseMeterStop(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_PulseMeterStop(hProslic);
#endif
	return 1;
}


int ProSLIC_SetDCDCInversionFlag (proslicChanType_ptr hProslic, uInt8 flag){
	if(hProslic->channelType != PROSLIC) return RC_IGNORE;
    hProslic->dcdc_polarity_invert = flag;
    return 1;
}


int ProSLIC_PowerUpConverter (proslicChanType_ptr hProslic){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (hProslic->deviceId->chipType >= SI3240 && hProslic->deviceId->chipType <= SI3247)
		return 1;
#endif
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_PowerUpConverter(hProslic);;
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_PowerUpConverter(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_PowerUpConverter(hProslic);
#endif
	return 1;
}

int ProSLIC_PowerDownConverter (proslicChanType_ptr hProslic){
		if(hProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI322X
	if (hProslic->deviceId->chipType >= SI3226 && hProslic->deviceId->chipType <= SI3227)
		return Si3226_PowerDownConverter(hProslic);;
#endif
#ifdef SI3217X
	if (hProslic->deviceId->chipType >= SI32171 && hProslic->deviceId->chipType <= SI32179)
		return Si3217x_PowerDownConverter(hProslic);
#endif
#ifdef SI3226X
	if (hProslic->deviceId->chipType >= SI32260 && hProslic->deviceId->chipType <= SI32269)
		return Si3226x_PowerDownConverter(hProslic);
#endif
	return 1;
}
int ProSLIC_LBCal (proslicChanType_ptr *pProslic, int size){
		if((*pProslic)->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI322X
	if (pProslic[0]->deviceId->chipType >= SI3226 && pProslic[0]->deviceId->chipType <= SI3227)
		return Si3226_LBCal (pProslic,size);
#endif
#ifdef SI3217X
	if (pProslic[0]->deviceId->chipType >= SI32171 && pProslic[0]->deviceId->chipType <= SI32179)
		return Si3217x_LBCal(pProslic,size);
#endif
#ifdef SI3226X
	if (pProslic[0]->deviceId->chipType >= SI32260 && pProslic[0]->deviceId->chipType <= SI32269)
		return Si3226x_LBCal(pProslic,size);
#endif
	return 1;
}

int ProSLIC_GetLBCalResult (proslicChanType *pProslic,int32 *result1,int32 *result2, int32 *result3, int32 *result4){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (pProslic->deviceId->chipType >= SI3240 && pProslic->deviceId->chipType <= SI3247)
		return Si324x_GetLBCalResult (pProslic, result1,result2,result3,result4);
#endif
#ifdef SI322X
	if (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
		return Si3226_GetLBCalResult (pProslic,result1,result2,result3,result4);
#endif
#ifdef SI3217X
	if (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
		return Si3217x_GetLBCalResult (pProslic,result1,result2,result3,result4);
#endif
#ifdef SI3226X
	if (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
		return Si3226x_GetLBCalResult (pProslic,result1,result2,result3,result4);
#endif
	return 1;
}

int ProSLIC_GetLBCalResultPacked (proslicChanType *pProslic,int32 *result){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (pProslic->deviceId->chipType >= SI3240 && pProslic->deviceId->chipType <= SI3247)
		return Si324x_GetLBCalResultPacked (pProslic, result);
#endif
#ifdef SI322X
	if (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
		return Si3226_GetLBCalResultPacked (pProslic,result);
#endif
#ifdef SI3217X
	if (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
		return Si3217x_GetLBCalResultPacked (pProslic,result);
#endif
#ifdef SI3226X
	if (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
		return Si3226x_GetLBCalResultPacked (pProslic,result);
#endif
	return 1;
}

int ProSLIC_LoadPreviousLBCal (proslicChanType *pProslic,int32 result1,int32 result2,int32 result3,int32 result4){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (pProslic->deviceId->chipType >= SI3240 && pProslic->deviceId->chipType <= SI3247)
		return Si324x_LoadPreviousLBCal (pProslic,result1,result2,result3,result4);
#endif
#ifdef SI322X
	if (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
		return Si3226_LoadPreviousLBCal (pProslic,result1,result2,result3,result4);
#endif
#ifdef SI3217X
	if (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
		return Si3217x_LoadPreviousLBCal (pProslic,result1,result2,result3,result4);
#endif
#ifdef SI3226X
	if (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
		return Si3226x_LoadPreviousLBCal (pProslic,result1,result2,result3,result4);
#endif
	return 1;
}

int ProSLIC_LoadPreviousLBCalPacked (proslicChanType *pProslic,int32 *result){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (pProslic->deviceId->chipType >= SI3240 && pProslic->deviceId->chipType <= SI3247)
		return Si324x_LoadPreviousLBCalPacked (pProslic,result);
#endif
#ifdef SI322X
	if (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
		return Si3226_LoadPreviousLBCalPacked (pProslic,result);
#endif
#ifdef SI3217X
	if (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
		return Si3217x_LoadPreviousLBCalPacked (pProslic,result);
#endif
#ifdef SI3226X
	if (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
		return Si3226x_LoadPreviousLBCalPacked (pProslic,result);
#endif
	return 1;
}

int ProSLIC_dbgSetDCFeedVopen (proslicChanType *pProslic, uInt32 v_vlim_val,int32 preset){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI321X
	if (pProslic->deviceId->chipType >= SI3210 && pProslic->deviceId->chipType <= SI3216M)
		return Si321x_dbgSetDCFeedVopen(pProslic,v_vlim_val,preset);
#endif
#ifdef SI322X
	if (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
		return Si3226_dbgSetDCFeedVopen (pProslic,v_vlim_val,preset);
#endif
#ifdef SI3217X
	if (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
		return Si3217x_dbgSetDCFeedVopen (pProslic,v_vlim_val,preset);
#endif
#ifdef SI3226X
	if (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
		return Si3226x_dbgSetDCFeedVopen (pProslic,v_vlim_val,preset);
#endif
	return 1;
}



int ProSLIC_dbgSetDCFeedIloop (proslicChanType *pProslic, uInt32 i_ilim_val, int32 preset){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI322X
	if (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
		return Si3226_dbgSetDCFeedIloop (pProslic,i_ilim_val,preset);
#endif
#ifdef SI3217X
	if (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
		return Si3217x_dbgSetDCFeedIloop (pProslic,i_ilim_val,preset);
#endif
#ifdef SI3226X
	if (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
		return Si3226x_dbgSetDCFeedIloop (pProslic,i_ilim_val,preset);
#endif
#ifdef SI321X
	if (pProslic->deviceId->chipType >= SI3210 && pProslic->deviceId->chipType <= SI3216M)
		return Si321x_dbgSetDCFeedIloop(pProslic,i_ilim_val,preset);
#endif
	return 1;
}


int ProSLIC_dbgSetRinging (proslicChanType *pProslic, ProSLIC_dbgRingCfg *ringCfg, int preset){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI322X
	if (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
		return Si3226_dbgSetRinging (pProslic,ringCfg,preset);
#endif
#ifdef SI3217X
	if (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
		return Si3217x_dbgSetRinging (pProslic,ringCfg,preset);
#endif
#ifdef SI3226X
	if (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
		return Si3226x_dbgSetRinging (pProslic,ringCfg,preset);
#endif
	return 1;
}

int ProSLIC_dbgSetRXGain (proslicChanType *pProslic, int32 gain, int impedance_preset, int audio_gain_preset){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI321X
	if (pProslic->deviceId->chipType >= SI3210 && pProslic->deviceId->chipType <= SI3216M)
		return Si321x_dbgSetGain(pProslic,gain,impedance_preset,audio_gain_preset);
#endif
#ifdef SI322X
	if (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
		return Si3226_dbgSetRXGain (pProslic,gain,impedance_preset,audio_gain_preset);
#endif
#ifdef SI3217X
	if (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
		return Si3217x_dbgSetRXGain (pProslic,gain,impedance_preset,audio_gain_preset);
#endif
#ifdef SI3226X
	if (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
		return Si3226x_dbgSetRXGain (pProslic,gain,impedance_preset,audio_gain_preset);
#endif
#ifdef SI324X
	if (pProslic->deviceId->chipType >= SI3240 && pProslic->deviceId->chipType <= SI3247)
		return Si324x_dbgSetRXGain(pProslic,gain,impedance_preset,audio_gain_preset);
#endif
	return 1;
}

int ProSLIC_dbgSetTXGain (proslicChanType *pProslic, int32 gain, int impedance_preset, int audio_gain_preset){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;

#ifdef SI321X
	if (pProslic->deviceId->chipType >= SI3210 && pProslic->deviceId->chipType <= SI3216M)
		return Si321x_dbgSetGain(pProslic,gain,impedance_preset,audio_gain_preset);
#endif
#ifdef SI322X
	if (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
		return Si3226_dbgSetTXGain (pProslic,gain,impedance_preset,audio_gain_preset);
#endif
#ifdef SI3217X
	if (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
		return Si3217x_dbgSetTXGain (pProslic,gain,impedance_preset,audio_gain_preset);
#endif
#ifdef SI3226X
	if (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
		return Si3226x_dbgSetTXGain (pProslic,gain,impedance_preset,audio_gain_preset);
#endif
#ifdef SI324X
	if (pProslic->deviceId->chipType >= SI3240 && pProslic->deviceId->chipType <= SI3247)
		return Si324x_dbgSetTXGain(pProslic,gain,impedance_preset,audio_gain_preset);
#endif
	return 1;
}


/*
** Function: ProSLIC_LineMonitor
**
** Description: 
** Generic monitoring function
**
** Returns:
** 0
*/
int ProSLIC_LineMonitor (proslicChanType *pProslic, proslicMonitorType *monitor){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI324X
	if (pProslic->deviceId->chipType >= SI3240 && pProslic->deviceId->chipType <= SI3247)
		return Si324x_LineMonitor (pProslic,monitor);
#endif
#ifdef SI322X
	if (pProslic->deviceId->chipType >= SI3226 && pProslic->deviceId->chipType <= SI3227)
		return Si3226_LineMonitor (pProslic,monitor);
#endif
#ifdef SI3217X
	if (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
		return Si3217x_LineMonitor (pProslic, monitor);
#endif
#ifdef SI3226X
	if (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
		return Si3226x_LineMonitor (pProslic, monitor);
#endif
	return 1;
}



/*
** Function: ProSLIC_PSTNCheck
**
** Description: 
** Monitor for excessive longitudinal current, which
** would be present if a live pstn line was connected
** to the port.
**
** Returns:
** 0 - no pstn detected
** 1 - pstn detected
*/
int ProSLIC_PSTNCheck (proslicChanType *pProslic, proslicPSTNCheckObjType *pPSTNCheck){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI322X
#define SI_PSTN_IMPLEMENTED
    return Si3226_PSTNCheck (pProslic,pPSTNCheck);
#endif
#ifdef SI3217X
#define SI_PSTN_IMPLEMENTED
    return Si3217x_PSTNCheck (pProslic,pPSTNCheck);
#endif
#ifdef SI3226X
#define SI_PSTN_IMPLEMENTED
    return Si3226x_PSTNCheck (pProslic,pPSTNCheck);
#endif
#ifndef SI_PSTN_IMPLEMENTED
    return 1;
#endif
#undef SI_PSTN_IMPLEMENTED
}


/*
** Function: ProSLIC_DiffPSTNCheck
**
** Description: 
** Monitor for excessive longitudinal current, which
** would be present if a live pstn line was connected
** to the port.
**
** Returns:
** 0 - test in progress
** 1 - test complete
** 
*/
#ifdef PSTN_DET_ENABLE
int ProSLIC_DiffPSTNCheck (proslicChanType *pProslic, proslicDiffPSTNCheckObjType *pPSTNCheck){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI3217X
    return Si3217x_DiffPSTNCheck (pProslic,pPSTNCheck);
#endif
#ifdef SI3226X
    return Si3226x_DiffPSTNCheck (pProslic,pPSTNCheck);
#endif
    return 1;
}
#endif

/*
** Function: ProSLIC_CreatePSTNCheckObj
**
** Description: 
** Allocate memory for pstnCheckObj
**
** Returns:
** 0
** -1 if malloc disabled
*/
int ProSLIC_CreatePSTNCheckObj(proslicPSTNCheckObjType_ptr *pstnCheckObj)
{
#ifndef DISABLE_MALLOC
  	*pstnCheckObj = kmalloc(sizeof(proslicPSTNCheckObjType), GFP_KERNEL);
	memset (*pstnCheckObj,0,sizeof(proslicPSTNCheckObjType));
    return RC_NONE;
#else
	return -1;
#endif
}

/*
** Function: ProSLIC_CreateDiffPSTNCheckObj
**
** Description: 
** Allocate memory for proslicDiffPstnCheckObj
**
** Returns:
** 0
** -1 if malloc disabled
*/
#ifdef PSTN_DET_ENABLE
int ProSLIC_CreateDiffPSTNCheckObj(proslicDiffPSTNCheckObjType_ptr *pstnCheckObj)
{
#ifndef DISABLE_MALLOC
  	*pstnCheckObj = kmalloc(sizeof(proslicDiffPSTNCheckObjType), GFP_KERNEL);
	memset (*pstnCheckObj,0,sizeof(proslicDiffPSTNCheckObjType));
    return 0;
#else
	return -1;
#endif
}
#endif
/*
** Function: ProSLIC_DestroyPSTNCheckObj
**
** Description: 
** Free memory for pstnCheckObj
**
** Returns:
** 0
** -1 if malloc disabled
*/
int ProSLIC_DestroyPSTNCheckObj(proslicPSTNCheckObjType_ptr *pstnCheckObj)
{
#ifndef DISABLE_MALLOC
	kfree ((proslicPSTNCheckObjType_ptr)*pstnCheckObj);
    return 0;
#else
	return -1;
#endif
}


/*
** Function: ProSLIC_DestroyDiffPSTNCheckObj
**
** Description: 
** Free memory for pstnDiffCheckObj
**
** Returns:
** 0
** -1 if malloc disabled
*/
#ifdef PSTN_DET_ENABLE
int ProSLIC_DestroyDiffPSTNCheckObj(proslicDiffPSTNCheckObjType_ptr *pstnCheckObj)
{
#ifndef DISABLE_MALLOC
	kfree ((proslicDiffPSTNCheckObjType_ptr)*pstnCheckObj);
    return 0;
#else
	return -1;
#endif
}
#endif

/*
** Function: ProSLIC_InitPSTNCheckObj
**
** Description: 
** Initialize pstnCheckObj structure memebers
**
** Returns:
** 0
*/
int ProSLIC_InitPSTNCheckObj(proslicPSTNCheckObjType_ptr pstnCheckObj, int32 avgThresh, int32 singleThresh, uInt8 samples)
{
    pstnCheckObj->avgThresh = avgThresh;
    pstnCheckObj->singleThresh = singleThresh;
    pstnCheckObj->samples = samples;
    pstnCheckObj->avgIlong = 0;
    pstnCheckObj->count = 0;
    pstnCheckObj->buffFull = 0;
    
    return RC_NONE;
}

/*
** Function: ProSLIC_InitDiffPSTNCheckObj
**
** Description: 
** Initialize pstnCheckObj structure memebers
**
** Returns:
** 0
*/
#ifdef PSTN_DET_ENABLE
int ProSLIC_InitDiffPSTNCheckObj(proslicDiffPSTNCheckObjType_ptr pstnDiffCheckObj, 
                                int preset1,
                                int preset2,
                                int entry_preset,
                                int femf_enable)
{
    pstnDiffCheckObj->samples = PSTN_DET_DIFF_SAMPLES;
    pstnDiffCheckObj->max_femf_vopen = PSTN_DET_MAX_FEMF;
    pstnDiffCheckObj->entryDCFeedPreset = entry_preset;
    pstnDiffCheckObj->dcfPreset1 = preset1;
    pstnDiffCheckObj->dcfPreset2 = preset2;
    pstnDiffCheckObj->femf_enable = femf_enable;
    pstnDiffCheckObj->pState.stage = 0;
    pstnDiffCheckObj->pState.sampleIterations = 0;
    pstnDiffCheckObj->pState.waitIterations = 0;
    
    return 0;
}
#endif


/*
** Function: ProSLIC_SetPwrsaveMode
**
** Description: 
** Enable or disable powersave mode
**
** Returns:
** RC_NONE
*/
int ProSLIC_SetPowersaveMode (proslicChanType *pProslic, int pwrsave){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI3217X
#define ProSLIC_SetPowersaveMode_IMPLEMENTED
    return Si3217x_SetPowersaveMode (pProslic,pwrsave);
#endif
#ifdef SI3226X
#define ProSLIC_SetPowersaveMode_IMPLEMENTED
    return Si3226x_SetPowersaveMode (pProslic,pwrsave);
#endif

#ifndef ProSLIC_SetPowersaveMode_IMPLEMENTED
    return 1;
#else
#undef ProSLIC_SetPowersaveMode_IMPLEMENTED
#endif
}

/*
** Function: ProSLIC_SetDAAEnable
**
** Description: 
** Enable or disable adjacent FXO (Si32178 only)
**
** Returns:
** RC_NONE
*/
int ProSLIC_SetDAAEnable (proslicChanType *pProslic, int enable){
		if(pProslic->channelType != PROSLIC) return RC_IGNORE;
#ifdef SI3217X
    return Si3217x_SetDAAEnable (pProslic,enable);
#endif
    SILABS_UNREFERENCED_PARAMETER(enable);
    return RC_NONE;
}


/*
** Function: ProSLIC_Version
**
** Description: 
** Return API version
**
** Returns:
** 0
*/
extern const char *SiVoiceAPIVersion;
char *ProSLIC_Version()
{
	return (char *)SiVoiceAPIVersion;
}
