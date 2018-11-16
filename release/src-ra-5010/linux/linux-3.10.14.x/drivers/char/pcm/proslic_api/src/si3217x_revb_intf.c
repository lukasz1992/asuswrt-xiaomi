/*
** Copyright (c) 2007-2013 by Silicon Laboratories
**
** $Id: si3217x_intf.c 3746 2013-01-09 19:39:49Z cdp $
**
** SI3217X_RevB_Intf.c
** SI3217X Rev B ProSLIC interface implementation file
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
** This is the implementation file for the main ProSLIC API and is used 
** in the ProSLIC demonstration code. 
**
*/

#include "si_voice_datatypes.h"
#include "si_voice_ctrl.h"
#include "si_voice_timer_intf.h"
#include "proslic.h"
#include "si3217x.h"
#include "si3217x_revb_intf.h"
#include "si3217x_common_registers.h"
#include "si3217x_revb_registers.h"
#include "proslic_api_config.h"

#define PRAM_ADDR (334 + 0x400)
#define PRAM_DATA (335 + 0x400)

#define WriteReg        pProslic->deviceId->ctrlInterface->WriteRegister_fptr
#define ReadReg         pProslic->deviceId->ctrlInterface->ReadRegister_fptr
#define pProHW          pProslic->deviceId->ctrlInterface->hCtrl
#define Reset           pProslic->deviceId->ctrlInterface->Reset_fptr
#define Delay           pProslic->deviceId->ctrlInterface->Delay_fptr
#define pProTimer       pProslic->deviceId->ctrlInterface->hTimer
#define WriteRAM        pProslic->deviceId->ctrlInterface->WriteRAM_fptr
#define ReadRAM         pProslic->deviceId->ctrlInterface->ReadRAM_fptr
#define TimeElapsed     pProslic->deviceId->ctrlInterface->timeElapsed_fptr
#define getTime         pProslic->deviceId->ctrlInterface->getTime_fptr
#define SetSemaphore    pProslic->deviceId->ctrlInterface->Semaphore_fptr


extern Si3217x_General_Cfg Si3217x_General_Configuration;
#ifdef SIVOICE_MULTI_BOM_SUPPORT
extern const proslicPatch SI3217X_PATCH_B_FLBK;
extern const proslicPatch SI3217X_PATCH_B_BKBT;
extern const proslicPatch SI3217X_PATCH_B_PBB;
extern Si3217x_General_Cfg Si3217x_General_Configuration_MultiBOM[];
extern int si3217x_genconf_multi_max_preset;
#else
extern const proslicPatch SI3217X_PATCH_B_DEFAULT;
#endif

/* Pulse Metering */
#ifndef DISABLE_PULSE_SETUP
extern Si3217x_PulseMeter_Cfg Si3217x_PulseMeter_Presets [];
#endif

#define SI3217X_COM_RAM_DCDC_DCFF_ENABLE SI3217X_COM_RAM_GENERIC_8
#define GCONF Si3217x_General_Configuration

#ifdef ENABLE_DEBUG
static const char LOGPRINT_PREFIX[] = "Si3217x_B: ";
#endif

/*
** Patch Support RAM Redefinitions
*/
#define PSR_DCDC_OVTHRESH_NEW     SI3217X_COM_RAM_UNUSED1012

/*
** Constants
*/
#define CONST_DCDC_VREF_CTRL_REVB      0x00600000L
#define CONST_DCDC_ANA_GAIN_REVB       0x00300000L
#define BIT20LSB                       1048576L
#define OITHRESH_OFFS                  900L
#define OITHRESH_SCALE                 100L
#define OVTHRESH_OFFS                  70000L
#define OVTHRESH_SCALE                 6000L
#define UVTHRESH_OFFS                  5000L
#define UVTHRESH_SCALE                 335L
#define UVHYST_OFFS                    1000L
#define UVHYST_SCALE                   200L



/*
** Function: Si3217x_RevB_GenParamUpdate
**
** Update general parameters
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object array
** fault: error code
**
** Return:
** error code
*/
int Si3217x_RevB_GenParamUpdate(proslicChanType_ptr pProslic,initSeqType seq)
{
 uInt8 data;
 ramData ram_data;

    switch(seq)
    {
    case INIT_SEQ_PRE_CAL:
        /* 
        ** Force pwrsave off and disable AUTO-tracking - set to user configured state after cal 
        */
        WriteReg(pProHW, pProslic->channel,SI3217X_COM_REG_ENHANCE,0);
        WriteReg(pProHW, pProslic->channel,SI3217X_COM_REG_AUTO,0x2F); 
     
        /* 
        ** General Parameter Updates 
        */
        WriteRAM(pProHW, pProslic->channel,SI3217X_REVB_RAM_DCDC_FSW_VTHLO, Si3217x_General_Configuration.dcdc_fsw_vthlo);
        WriteRAM(pProHW, pProslic->channel,SI3217X_REVB_RAM_DCDC_FSW_VHYST, Si3217x_General_Configuration.dcdc_fsw_vhyst);
        WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_P_TH_HVIC, Si3217x_General_Configuration.p_th_hvic);
        WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_COEF_P_HVIC, Si3217x_General_Configuration.coef_p_hvic);
        WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_BAT_HYST, Si3217x_General_Configuration.bat_hyst);
        WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_VBATH_EXPECT, Si3217x_General_Configuration.vbath_expect);
        WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_VBATR_EXPECT, Si3217x_General_Configuration.vbatr_expect);
        WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_PWRSAVE_TIMER, Si3217x_General_Configuration.pwrsave_timer);
        WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_OFFHOOK_THRESH, Si3217x_General_Configuration.pwrsave_ofhk_thresh);
        WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_VBAT_TRACK_MIN, Si3217x_General_Configuration.vbat_track_min);
        WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_VBAT_TRACK_MIN_RNG, Si3217x_General_Configuration.vbat_track_min_rng);
        WriteRAM(pProHW, pProslic->channel,SI3217X_REVB_RAM_DCDC_FSW_NORM, Si3217x_General_Configuration.dcdc_fsw_norm);
        WriteRAM(pProHW, pProslic->channel,SI3217X_REVB_RAM_DCDC_FSW_NORM_LO, Si3217x_General_Configuration.dcdc_fsw_norm_lo);
        WriteRAM(pProHW, pProslic->channel,SI3217X_REVB_RAM_DCDC_FSW_RINGING, Si3217x_General_Configuration.dcdc_fsw_ring);
        WriteRAM(pProHW, pProslic->channel,SI3217X_REVB_RAM_DCDC_FSW_RINGING_LO, Si3217x_General_Configuration.dcdc_fsw_ring_lo);
        WriteRAM(pProHW, pProslic->channel,SI3217X_REVB_RAM_DCDC_DIN_LIM, Si3217x_General_Configuration.dcdc_din_lim);
        WriteRAM(pProHW, pProslic->channel,SI3217X_REVB_RAM_DCDC_VOUT_LIM, Si3217x_General_Configuration.dcdc_vout_lim);
        WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_DCDC_ANA_SCALE, Si3217x_General_Configuration.dcdc_ana_scale);
        WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_PD_UVLO, Si3217x_General_Configuration.pd_uvlo);
        WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_PD_OVLO, Si3217x_General_Configuration.pd_ovlo);
        WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_PD_OCLO, Si3217x_General_Configuration.pd_oclo);
        WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_PD_SWDRV, Si3217x_General_Configuration.pd_swdrv);
        WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_DCDC_UVPOL, Si3217x_General_Configuration.dcdc_uvpol);
        WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_DCDC_RNGTYPE, Si3217x_General_Configuration.dcdc_rngtype);
        WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_DCDC_ANA_TOFF, Si3217x_General_Configuration.dcdc_ana_toff);
        WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_DCDC_ANA_TONMIN, Si3217x_General_Configuration.dcdc_ana_tonmin);
        WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_DCDC_ANA_TONMAX, Si3217x_General_Configuration.dcdc_ana_tonmax);


        /*
        ** Hardcoded RAM
        */
        
        /* OITHRESH */
        ram_data = (GCONF.i_oithresh > OITHRESH_OFFS)?(GCONF.i_oithresh - OITHRESH_OFFS)/OITHRESH_SCALE:0L;
        ram_data *= BIT20LSB;
        WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_DCDC_OITHRESH,ram_data);

        /* OVTHRESH */
        ram_data = (GCONF.v_ovthresh > OVTHRESH_OFFS)?(GCONF.v_ovthresh - OVTHRESH_OFFS)/OVTHRESH_SCALE:0L;
        ram_data *= BIT20LSB;
        WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_DCDC_OVTHRESH,ram_data);

        /* UVTHRESH */
        ram_data = (GCONF.v_uvthresh > UVTHRESH_OFFS)?(GCONF.v_uvthresh - UVTHRESH_OFFS)/UVTHRESH_SCALE:0L;
        ram_data *= BIT20LSB;
        WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_DCDC_UVTHRESH,ram_data);

        /* UVHYST */
        ram_data = (GCONF.v_uvhyst > UVHYST_OFFS)?(GCONF.v_uvhyst - UVHYST_OFFS)/UVHYST_SCALE:0L;
        ram_data *= BIT20LSB;
        WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_DCDC_UVHYST,ram_data);

        /* PM BOM dependent audio gain scaling */
        if(Si3217x_General_Configuration.pm_bom == BO_PM_BOM)
        {
            WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_SCALE_KAUDIO,BOM_KAUDIO_PM);
            WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_AC_ADC_GAIN,BOM_AC_ADC_GAIN_PM);
        }
        else
        {
            WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_SCALE_KAUDIO,BOM_KAUDIO_NO_PM);
            WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_AC_ADC_GAIN,BOM_AC_ADC_GAIN_NO_PM);
        }

        /* Default GPIO config - coarse sensors */
        data = ReadReg(pProHW, pProslic->channel,SI3217X_COM_REG_GPIO_CFG1);
        data &= 0xF9;  /* Clear DIR for GPIO 1&2 */
        data |= 0x60;  /* Set ANA mode for GPIO 1&2 */
        WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_GPIO_CFG1,data);          /* coarse sensors analog mode */


        /* Misc modifications to default settings */
        WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_PDN,0x80);                /* madc powered in open state */
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_TXACHPF_A1_1,0x71EB851L); /* Fix HPF corner */
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_ROW0_C2,0x723F235L);      /* improved DTMF det */
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_ROW1_C2,0x57A9804L);      /* improved DTMF det */
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_PD_REF_OSC,0x200000L);    /* PLL freerun workaround */
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_ILOOPLPF,0x4EDDB9L);      /* 20pps pulse dialing enhancement */
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_ILONGLPF,0x806D6L);       /* 20pps pulse dialing enhancement */
        WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_VDIFFLPF,0x10038DL);      /* 20pps pulse dialing enhancement */
        WriteRAM(pProHW,pProslic->channel,SI3217X_REVB_RAM_LKG_STBY_OFFSET,0x4000000L); /* Increase OHT standby leakage */
        WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_DCDC_VREF_CTRL, CONST_DCDC_VREF_CTRL_REVB);
        WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_DCDC_ANA_GAIN, CONST_DCDC_ANA_GAIN_REVB);


        /*
	    ** Modified PSR Setting for PSR_DCDC_OVTHRESH_NEW based on BOM option -
	    ** this will allow for a single patch for all BOM options
        ** RAM locations used by patch are locally defined
	    */
	    if(Si3217x_General_Configuration.bom_option == BO_DCDC_BUCK_BOOST)
	    {
		    WriteRAM(pProHW,pProslic->channel,PSR_DCDC_OVTHRESH_NEW,0x400000L);
	    }
	    else if(Si3217x_General_Configuration.bom_option == BO_DCDC_PMOS_BUCK_BOOST)
	    {
		    if(Si3217x_General_Configuration.vdc_range == VDC_9P0_24P0)
		    {
			    WriteRAM(pProHW,pProslic->channel,PSR_DCDC_OVTHRESH_NEW,0x700000L);
		    }
		    else
		    {
			    WriteRAM(pProHW,pProslic->channel,PSR_DCDC_OVTHRESH_NEW,0x300000L);
		    }
	    }
	    else
	    {
		    WriteRAM(pProHW,pProslic->channel,PSR_DCDC_OVTHRESH_NEW,0xB00000L);
	    }
        break;


    case INIT_SEQ_POST_CAL:
        WriteReg(pProHW, pProslic->channel,SI3217X_COM_REG_ENHANCE, Si3217x_General_Configuration.enhance);
        WriteReg(pProHW, pProslic->channel,SI3217X_REVB_REG_DAA_CNTL, Si3217x_General_Configuration.daa_cntl);
        WriteReg(pProHW, pProslic->channel,SI3217X_COM_REG_AUTO, Si3217x_General_Configuration.auto_reg);
        break;

    }
    return RC_NONE;
}


/*
** Function: Si3217x_RevB_SelectPatch
**
** Select patch based on general parameters
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object array
** fault: error code
** patch:    Pointer to proslicPatch pointer
**
** Return:
** error code
*/
int Si3217x_RevB_SelectPatch(proslicChanType_ptr pProslic, const proslicPatch **patch)
{

#ifdef SIVOICE_MULTI_BOM_SUPPORT
	if(Si3217x_General_Configuration.bom_option == BO_DCDC_FLYBACK)
	{
		*patch = &(SI3217X_PATCH_B_FLBK);
	}
	else if(Si3217x_General_Configuration.bom_option == BO_DCDC_BUCK_BOOST)
	{
		*patch = &(SI3217X_PATCH_B_BKBT);
	}
	else if(Si3217x_General_Configuration.bom_option == BO_DCDC_PMOS_BUCK_BOOST)
	{
		*patch = &(SI3217X_PATCH_B_PBB);
	}
	else
	{
        #ifdef ENABLE_DEBUG
		if (pProslic->debugMode)
		{
			LOGPRINT("%sChannel %d : Invalid Patch\n", LOGPRINT_PREFIX, pProslic->channel);
		}
        #endif
		pProslic->channelEnable = 0;
		pProslic->error = RC_INVALID_PATCH;
		return RC_INVALID_PATCH;
	}
#else
	*patch = &(SI3217X_PATCH_B_DEFAULT);
#endif

    return RC_NONE;
}


/*
** Function: Si3217x_RevB_ConverterSetup
**
** Description: 
** Program revision specific settings before powering converter
**
** Specifically, from general parameters and knowledge that this
** is Rev B, setup dcff drive and gate drive polarity.
**
** Returns:
** int (error)
**
*/
int Si3217x_RevB_ConverterSetup(proslicChanType_ptr pProslic)
{
ramData inv_on;
ramData inv_off;

    /* Option to add a per-channel inversion for maximum flexibility */
    if(pProslic->dcdc_polarity_invert)
    {
        inv_on  = 0x0L;
        inv_off = 0x100000L;
    }
    else
    {
        inv_on  = 0x100000L;
        inv_off = 0x0L;
    }

    switch(Si3217x_General_Configuration.bom_option)
    {
    case BO_DCDC_FLYBACK:
        /*
        ** All revB flyback designs have gate driver, 
        ** inverted polarity, and no dcff drive
        */
        WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_DCDC_DCFF_ENABLE,0x0L);
        WriteRAM(pProHW, pProslic->channel,SI3217X_REVB_RAM_DCDC_SWFET,0x300000L);
        WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_DCDC_SWDRV_POL,inv_on);
        break;

    case BO_DCDC_BUCK_BOOST:
        /*
        ** All revB bjt buck boost designs have dcff drive,
        ** no gate driver, and non-inverted polarity
        */
        WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_DCDC_DCFF_ENABLE,0x10000000L);
        WriteRAM(pProHW, pProslic->channel,SI3217X_REVB_RAM_DCDC_SWFET,0x300000L);
        WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_DCDC_SWDRV_POL,inv_off);
        break;

    case BO_DCDC_PMOS_BUCK_BOOST:
        /*
        ** RevB pmos buck boost designs have gate driver,
        ** non-inverted polarity, and dcff drive on low voltage option only
        */
        if(Si3217x_General_Configuration.vdc_range == VDC_3P0_6P0)
        {
            WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_DCDC_DCFF_ENABLE,0x10000000L);
        }
        else
        {
            WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_DCDC_DCFF_ENABLE,0x0L);
        }
        WriteRAM(pProHW, pProslic->channel,SI3217X_REVB_RAM_DCDC_SWFET,0x300000L);
        WriteRAM(pProHW, pProslic->channel,SI3217X_COM_RAM_DCDC_SWDRV_POL,inv_off);
        break;

    default:
        return RC_DCDC_SETUP_ERR;
    }

    return RC_NONE;
}

/*
** Function: Si3217x_RevB_PulseMeterSetup
**
** Description: 
** configure pulse metering
*/
#ifndef DISABLE_PULSE_SETUP
int Si3217x_RevB_PulseMeterSetup (proslicChanType *pProslic, int preset){
    uInt8 reg;

    if(pProslic->channelType != PROSLIC) {
        return RC_CHANNEL_TYPE_ERR;
    }
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_PMCON,0x00);  /* Disable PM */
    WriteRAM(pProHW,pProslic->channel,SI3217X_COM_RAM_PM_AMP_THRESH,Si3217x_PulseMeter_Presets[preset].pm_amp_thresh);
    reg = (Si3217x_PulseMeter_Presets[preset].pmFreq     << 1) | 
          (Si3217x_PulseMeter_Presets[preset].pmRampRate << 4) |
          (Si3217x_PulseMeter_Presets[preset].pmCalForce << 3) |
          (Si3217x_PulseMeter_Presets[preset].pmPwrSave  << 7) ;
    WriteReg(pProHW,pProslic->channel,SI3217X_COM_REG_PMCON,reg);

    return RC_NONE;
}

#endif
