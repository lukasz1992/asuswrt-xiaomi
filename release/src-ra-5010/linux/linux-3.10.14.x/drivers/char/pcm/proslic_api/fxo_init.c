#include "proslic.h"
#include "si_voice_datatypes.h"
#include "si_voice_ctrl.h"
#include "si_voice.h"
#include "spi.h"
#include "timer.h"
#include "../pcm_ctrl.h"

extern pcm_config_type* ppcm_config;
/*
**
** Multi-port ProSLIC Initialization
**
*/
#include "slic_init.h"

int ProSLIC_HWInit(void)
{
	int i;
	ctrl_S spiGciObj; /* User¡¦s control interface object */
	systemTimer_S timerObj; /* User¡¦s timer object */
	chanState ports[NUMBER_OF_CHAN]; /* User¡¦s channel object, which has
	** a member defined as
	** SiVoiceChanType_ptr VoiceObj;
	*/
	/* Define Voice control interface object */
	SiVoiceControlInterfaceType *VoiceHWIntf;
	/* Define array of Voice device objects */
	SiVoiceDeviceType *VoiceDevices[NUMBER_OF_DEVICES];
	/* Define array of ProSLIC channel object pointers */
	SiVoiceChanType_ptr arrayOfVoiceChans[NUMBER_OF_CHAN];
	/*
	** Step 1: (optional)
	** Initialize user¡¦s control interface and timer objects ¡V this
	** may already be done, if not, do it here
	*/
	/*
	** Step 2: (required)
	** Create Voice Control Interface Object
	*/
	SiVoice_createControlInterface(&VoiceHWIntf);
	/*
	** Step 3: (required)
	** Create Voice Device Objects
	*/
	for(i=0;i<NUMBER_OF_DEVICES;i++)
	{
		SiVoice_createDevice(&(VoiceDevices[i]));
	}
	/*
	** Step 4: (required)
	** Create and initialize Voice channel objects
	** Also initialize array pointers to user¡¦s proslic channel object
	** members to simplify initialization process.
	*/
	for(i=0;i<NUMBER_OF_CHAN;i++)
	{
		SiVoice_createChannel(&(ports[i].VoiceObj));
		SiVoice_SWInitChan(ports[i].VoiceObj,i,VOICE_DEVICE_TYPE,
		VoiceDevices[i/CHAN_PER_DEVICE],VoiceHWIntf);
		arrayOfVoiceChans[i] = ports[i].VoiceObj;
		SiVoice_setSWDebugMode(ports[i].VoiceObj,TRUE); /* optional */
	}
	/*
	** Step 5: (required)
	** Establish linkage between host objects/functions and
	** API
	*/
	SiVoice_setControlInterfaceCtrlObj (VoiceHWIntf, &spiGciObj);
	SiVoice_setControlInterfaceReset (VoiceHWIntf, ctrl_ResetWrapper);
	SiVoice_setControlInterfaceWriteRegister (VoiceHWIntf, ctrl_WriteRegisterWrapper);
	SiVoice_setControlInterfaceReadRegister (VoiceHWIntf, ctrl_ReadRegisterWrapper);
	SiVoice_setControlInterfaceWriteRAM (VoiceHWIntf, ctrl_WriteRAMWrapper);
	SiVoice_setControlInterfaceReadRAM (VoiceHWIntf, ctrl_ReadRAMWrapper);
	SiVoice_setControlInterfaceTimerObj (VoiceHWIntf, &timerObj);
	SiVoice_setControlInterfaceDelay (VoiceHWIntf, time_DelayWrapper);
	SiVoice_setControlInterfaceTimeElapsed (VoiceHWIntf, time_TimeElapsedWrapper);
	SiVoice_setControlInterfaceGetTime (VoiceHWIntf, time_GetTimeWrapper);
	SiVoice_setControlInterfaceSemaphore (VoiceHWIntf, NULL);
	/*
	** Step 6: (system dependent)
	** Assert hardware Reset ¡V ensure VDD, PCLK, and FSYNC are present and stable
	** before releasing reset
	*/
	SiVoice_Reset(ports[0].VoiceObj);
	/*
	** Step 7: (required)
	** Initialize channels (loading of general parameters, calibrations,
	** dc-dc powerup, etc.)
	** Note that VDAA channels are ignored by ProSLIC_Init and ProSLIC
	** channels are ignored by Vdaa_Init.
	*/
	ProSLIC_Init(arrayOfVoiceChans,NUMBER_OF_CHAN);
	Vdaa_Init(arrayOfVoiceChans,NUMBER_OF_CHAN);
	/*
	** Step 8: (design dependent)
	** Execute longitudinal balance calibration
	** or reload coefficients from factory LB cal
	**
	** Note: all batteries should be up and stable prior to
	** executing the lb cal
	*/
	ProSLIC_LBCal(arrayOfVoiceChans,NUMBER_OF_CHAN);
	/*
	** Step 9: (design dependent)
	** Load custom configuration presets(generated using
	** ProSLIC API Config Tool)
	*/
	for(i=0;i<NUMBER_OF_CHAN;i++)
	{
		ProSLIC_DCFeedSetup(ports[i].VoiceObj,DCFEED_48V_20MA);
		ProSLIC_RingSetup(ports[i].VoiceObj,RING_F20_45VRMS_0VDC_LPR);
		ProSLIC_PCMSetup(ports[i].VoiceObj,PCM_16LIN);
		ProSLIC_ZsynthSetup(ports[i].VoiceObj,ZSYN_600_0_0_30_0);
		ProSLIC_ToneGenSetup(ports[i].VoiceObj,TONEGEN_FCC_DIAL);
		Vdaa_CountrySetup(ports[i].VoiceObj,COU_USA);
		Vdaa_HybridSetup(ports[i].VoiceObj,HYB_600_0_0_500FT_24AWG);
		Vdaa_PCMSetup(ports[i].VoiceObj,PCM_16LIN);
	}
	for(i=0;i<NUMBER_OF_CHAN;i++)
	{
		ProSLIC_PCMStart(ports[i].VoiceObj);
	}	
	for(i=0;i<NUMBER_OF_CHAN;i++)
	{
		ProSLIC_SetLinefeedStatus(ports[i].VoiceObj,LF_FWD_ACTIVE);
	}	
	return 1;
}	
/*
** END OF TYPICAL INITIALIZATION
*/