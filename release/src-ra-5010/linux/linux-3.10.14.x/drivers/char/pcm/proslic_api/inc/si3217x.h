/*
** Copyright (c) 2007 by Silicon Laboratories
**
** $Id: si3217x.h 3824 2013-02-05 16:46:08Z cdp $
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
*/

#ifndef SI3217XH_H
#define SI3217XH_H

#include "proslic.h"

/*
**
** Si3217x General Constants
**
*/
#define SI3217X_REVA                0
#define SI3217X_REVB                1
#define SI3217X_REVC                2
               
/*
** Calibration Constants
*/
#define SI3217XB_CAL_STD_CALR1       0xFF
#define SI3217XB_CAL_STD_CALR2       0xF8

#define SI3217XC_CAL_STD_CALR1       0xC0
#define SI3217XC_CAL_STD_CALR2       0x18

/* Timeouts in 10s of ms */
#define SI3217X_TIMEOUT_DCDC_UP     200
#define SI3217X_TIMEOUT_DCDC_DOWN   200


/*
** SI3217X DataTypes/Function Definitions 
*/

#define NUMIRQ 4

/*
** Initialization Sequence Location 
*/
typedef enum {
    INIT_SEQ_BEGINNING,
    INIT_SEQ_READ_ID,
    INIT_SEQ_PRE_PATCH_LOAD,
    INIT_SEQ_POST_PATCH_LOAD,
    INIT_SEQ_PRE_CAL,
    INIT_SEQ_POST_CAL,
    INIT_SEQ_END
}initSeqType;

/*
** Defines structure for configuring gpio
*/
typedef struct {
	uInt8 outputEn;
	uInt8 analog;
	uInt8 direction;
	uInt8 manual;
	uInt8 polarity;
	uInt8 openDrain;
	uInt8 batselmap;
} Si3217x_GPIO_Cfg;

typedef ProSLIC_DCfeed_Cfg Si3217x_DCfeed_Cfg;

/*
** New Si3217x General Parameter Struct Supporting Revs B and C 
*/
typedef struct {
    /* Flags */
    uInt8               device_key;
    bomOptionsType      bom_option;
    gateDriveType       gdrv_option;
	vdcRangeType        vdc_range; 
    vdaaSupportType     daa_enable;
    autoZcalType        zcal_en;
    pmBomType           pm_bom;
    /* Raw Parameters */
    ramData         i_oithresh;
    ramData         i_oithresh_lo;
    ramData         i_oithresh_hi;
    ramData         v_ovthresh;
    ramData         v_uvthresh;
    ramData         v_uvhyst;
    /* RAM Updates */
    ramData         dcdc_fsw_vthlo; 
    ramData         dcdc_fsw_vhyst;
	ramData         p_th_hvic;
	ramData         coef_p_hvic;
    ramData         bat_hyst;
	ramData         vbath_expect;         /* default - this is overwritten by dc feed preset */
	ramData         vbatr_expect;         /* default - this is overwritten by ring preset */
    ramData         pwrsave_timer;
    ramData         pwrsave_ofhk_thresh;
    ramData         vbat_track_min;       /* Same as DCDC_VREF_MIN */
    ramData         vbat_track_min_rng;   /* Same as DCDC_VREF_MIN_RNG */
	ramData         dcdc_fsw_norm; 
	ramData         dcdc_fsw_norm_lo; 
	ramData         dcdc_fsw_ring; 
	ramData         dcdc_fsw_ring_lo; 
	ramData         dcdc_din_lim; 
	ramData         dcdc_vout_lim; 
    ramData         dcdc_ana_scale;
    ramData         therm_dbi;
    ramData         vov_dcdc_slope;
    ramData         vov_dcdc_os;
    ramData         vov_ring_bat_dcdc;
    ramData         vov_ring_bat_max;
    ramData         dcdc_verr;
    ramData         dcdc_verr_hyst;
    ramData         pd_uvlo;
    ramData         pd_ovlo;
    ramData         pd_oclo;
    ramData         pd_swdrv;
    ramData         dcdc_uvpol;
    ramData         dcdc_rngtype; 
    ramData         dcdc_ana_toff; 
    ramData         dcdc_ana_tonmin; 
    ramData         dcdc_ana_tonmax; 
	uInt8           irqen1;
	uInt8           irqen2;
	uInt8           irqen3;
	uInt8           irqen4;
    uInt8           enhance;
	uInt8           daa_cntl;
	uInt8	        auto_reg;
} Si3217x_General_Cfg;

/*
** Defines structure for configuring pcm
*/
typedef struct {
    uInt8 pcmFormat;
    uInt8 widebandEn;
    uInt8 pcm_tri;
    uInt8 tx_edge;
    uInt8 alaw_inv;
} Si3217x_PCM_Cfg;

/*
** Defines structure for configuring pulse metering
*/
typedef struct {
	ramData pm_amp_thresh;
	uInt8 pmFreq;
	uInt8 pmRampRate;
    uInt8 pmCalForce;
    uInt8 pmPwrSave;
    uInt8 pmAuto;
    ramData pmActive;
    ramData pmInactive;
} Si3217x_PulseMeter_Cfg;
/*
** Defines structure for configuring FSK generation
*/
typedef struct {
	ramData fsk01;
	ramData fsk10;
	ramData fskamp0;
	ramData fskamp1;
	ramData fskfreq0;
	ramData fskfreq1;
	uInt8 eightBit;
	uInt8 fskdepth;
} Si3217x_FSK_Cfg;

/*
** Defines structure for configuring dtmf decode
*/
typedef struct {
	ramData dtmfdtf_b0_1;
	ramData dtmfdtf_b1_1;
	ramData dtmfdtf_b2_1;
	ramData dtmfdtf_a1_1;
	ramData dtmfdtf_a2_1;
	ramData dtmfdtf_b0_2;
	ramData dtmfdtf_b1_2;
	ramData dtmfdtf_b2_2;
	ramData dtmfdtf_a1_2;
	ramData dtmfdtf_a2_2;
	ramData dtmfdtf_b0_3;
	ramData dtmfdtf_b1_3;
	ramData dtmfdtf_b2_3;
	ramData dtmfdtf_a1_3;
	ramData dtmfdtf_a2_3;
} Si3217x_DTMFDec_Cfg;

/*
** Defines structure for configuring impedence synthesis
*/
typedef struct {
	ramData zsynth_b0;
	ramData zsynth_b1;
	ramData zsynth_b2;
	ramData zsynth_a1;
	ramData zsynth_a2;
	uInt8 ra;
} Si3217x_Zsynth_Cfg;

/*
** Defines structure for configuring hybrid
*/
typedef struct {
	ramData ecfir_c2;
	ramData ecfir_c3;
	ramData ecfir_c4;
	ramData ecfir_c5;
	ramData ecfir_c6;
	ramData ecfir_c7;
	ramData ecfir_c8;
	ramData ecfir_c9;
	ramData ecfir_b0;
	ramData ecfir_b1;
	ramData ecfir_a1;
	ramData ecfir_a2;
} Si3217x_hybrid_Cfg;

/*
** Defines structure for configuring GCI CI bits
*/
typedef struct {
	uInt8 gci_ci;
} Si3217x_CI_Cfg;

/*
** Defines structure for configuring modem tone detect
*/
typedef struct {
	ramData rxmodpwr;
	ramData rxpwr;
	ramData modem_gain;
	ramData txmodpwr;
	ramData txpwr;
} Si3217x_modemDet_Cfg;

/*
** Defines structure for configuring audio eq
*/
typedef struct {
	ramData txaceq_c0;
	ramData txaceq_c1;
	ramData txaceq_c2;
	ramData txaceq_c3;

	ramData rxaceq_c0;
	ramData rxaceq_c1;
	ramData rxaceq_c2;
	ramData rxaceq_c3;
} Si3217x_audioEQ_Cfg;

/*
** Defines structure for configuring audio gain
*/
typedef ProSLIC_audioGain_Cfg Si3217x_audioGain_Cfg;



typedef struct {
	Si3217x_audioEQ_Cfg audioEQ;
	Si3217x_hybrid_Cfg hybrid;
    Si3217x_Zsynth_Cfg zsynth;
	ramData txgain;
	ramData rxgain;
	ramData rxachpf_b0_1;
	ramData  rxachpf_b1_1;
	ramData  rxachpf_a1_1;
	int16 txgain_db; /*overall gain associated with this configuration*/
	int16 rxgain_db;
} Si3217x_Impedance_Cfg;



/*
** Defines structure for configuring tone generator
*/
typedef struct {
	Oscillator_Cfg osc1;
	Oscillator_Cfg osc2;
	uInt8 omode;
} Si3217x_Tone_Cfg; 

/*
** Defines structure for configuring ring generator
*/
typedef struct {
	ramData rtper;
	ramData freq;
	ramData amp;
	ramData phas;
	ramData offset;
	ramData slope_ring;
    ramData iring_lim;
    ramData rtacth;
	ramData rtdcth;
	ramData rtacdb;
	ramData rtdcdb;
	ramData vov_ring_bat;
	ramData vov_ring_gnd;
    ramData vbatr_expect;
	uInt8 talo;
	uInt8 tahi;
	uInt8 tilo;
	uInt8 tihi;
	ramData adap_ring_min_i;
    ramData counter_iring_val;
	ramData counter_vtr_val;
    ramData ar_const28;
    ramData ar_const32;
    ramData ar_const38;
    ramData ar_const46;
	ramData rrd_delay;
	ramData rrd_delay2;
    ramData dcdc_vref_min_rng;
	uInt8 ringcon;
    uInt8 userstat;
	ramData vcm_ring;
    ramData vcm_ring_fixed;
    ramData delta_vcm;
    ramData dcdc_rngtype;
} Si3217x_Ring_Cfg;



/*
** This defines names for the interrupts in the ProSLIC
*/
typedef enum {
/* IRQ1 */
IRQ_OSC1_T1_SI3217X = 0,   
IRQ_OSC1_T2_SI3217X,
IRQ_OSC2_T1_SI3217X,
IRQ_OSC2_T2_SI3217X,
IRQ_RING_T1_SI3217X,
IRQ_RING_T2_SI3217X,
IRQ_FSKBUF_AVAIL_SI3217X,
IRQ_VBAT_SI3217X,
/* IRQ2 */
IRQ_RING_TRIP_SI3217X = 8,
IRQ_LOOP_STAT_SI3217X,
IRQ_LONG_STAT_SI3217X,
IRQ_VOC_TRACK_SI3217X,
IRQ_DTMF_SI3217X,
IRQ_INDIRECT_SI3217X,
IRQ_TXMDM_SI3217X,
IRQ_RXMDM_SI3217X,
/* IRQ3 */
IRQ_P_HVIC_SI3217X = 16,  
IRQ_P_THERM_SI3217X,
IRQ_PQ3_SI3217X,
IRQ_PQ4_SI3217X,
IRQ_PQ5_SI3217X,
IRQ_PQ6_SI3217X,
IRQ_DSP_SI3217X,
IRQ_MADC_FS_SI3217X,
/* IRQ4 */
IRQ_USER_0_SI3217X = 24, 
IRQ_USER_1_SI3217X,
IRQ_USER_2_SI3217X,
IRQ_USER_3_SI3217X,
IRQ_USER_4_SI3217X,
IRQ_USER_5_SI3217X,
IRQ_USER_6_SI3217X,
IRQ_USER_7_SI3217X
}Si3217xProslicInt;

#endif
