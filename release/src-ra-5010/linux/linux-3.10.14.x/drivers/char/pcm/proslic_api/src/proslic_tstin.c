/*
** Copyright (c) 2011-2012 by Silicon Laboratories
**
** proslic_tstin.c
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
** This is the interface file for the ProSLIC test-in functions.
**
**
*/
#include "si_voice_datatypes.h"
#include "si_voice_ctrl.h"
#include "si_voice_timer_intf.h"
#include "proslic.h"
#include "proslic_api_config.h"
#include "proslic_tstin.h"


#ifndef DISABLE_MALLOC
#include "stdlib.h" /* for malloc */
#include "string.h" /* memset */
#endif


#ifdef SI3217X
#include "si3217x.h"
#include "si3217x_intf.h"
#endif
#ifdef SI3226X
#include "si3226x.h"
#include "si3226x_intf.h"
#endif


/*
** Function Pointer Macros
*/
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

#define WriteRegX       deviceId->ctrlInterface->WriteRegister_fptr
#define ReadRegX        deviceId->ctrlInterface->ReadRegister_fptr
#define pProHWX         deviceId->ctrlInterface->hCtrl
#define DelayX          deviceId->ctrlInterface->Delay_fptr
#define pProTimerX      deviceId->ctrlInterface->hTimer
#define WriteRAMX       deviceId->ctrlInterface->WriteRAM_fptr
#define ReadRAMX        deviceId->ctrlInterface->ReadRAM_fptr
#define getTimeX        deviceId->ctrlInterface->getTime_fptr
#define TimeElapsedX    deviceId->ctrlInterface->timeElapsed_fptr

/*
* Valid Device Number Ranges
*/

#define TSTIN_VALID_SI3217X_PART_NUM (pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
#define TSTIN_VALID_SI3226X_PART_NUM (pProslic->deviceId->chipType >= SI32260 && pProslic->deviceId->chipType <= SI32269)
#define TSTIN_INVALID_PART_NUM  !(TSTIN_VALID_SI3217X_PART_NUM || TSTIN_VALID_SI3226X_PART_NUM)

ProSLIC_DCfeed_Cfg ringtripTestDCFeedPreset[] = {
{
0x196038D8L,	/* SLOPE_VLIM */
0x1D707651L,	/* SLOPE_RFEED */
0x0040A0E0L,	/* SLOPE_ILIM */
0x1E07FE48L,	/* SLOPE_DELTA1 */
0x1ED62D87L,	/* SLOPE_DELTA2 */
0x01A50724L,	/* V_VLIM (14.000 v) */
0x016EE54FL,	/* V_RFEED (12.200 v) */
0x012CBBF5L,	/* V_ILIM  (10.000 v) */
0x00AF8C10L,	/* CONST_RFEED (10.000 mA) */
0x0045CBBCL,	/* CONST_ILIM (15.000 mA) */
0x000D494BL,	/* I_VLIM (0.000 mA) */
0x005B0AFBL,	/* LCRONHK (10.000 mA) */
0x006D4060L,	/* LCROFFHK (12.000 mA) */
0x00008000L,	/* LCRDBI (5.000 ms) */
0x0048D595L,	/* LONGHITH (8.000 mA) */
0x003FBAE2L,	/* LONGLOTH (7.000 mA) */
0x00008000L,	/* LONGDBI (5.000 ms) */
0x000F0000L,	/* LCRMASK (150.000 ms) */
0x00080000L,	/* LCRMASK_POLREV (80.000 ms) */
0x00140000L,	/* LCRMASK_STATE (200.000 ms) */
0x00140000L,	/* LCRMASK_LINECAP (200.000 ms) */
0x01999999L,	/* VCM_OH (25.000 v) */
0x0051EB85L,	/* VOV_BAT (5.000 v) */
0x00418937L 	/* VOV_GND (4.000 v) */
} 
};

/*
** dB lookup table
*/
#define DB_TABLE_STEP_SIZE     250
#define MAX_DB_TABLE           279

static const uInt32 dBTable10_n60 [] = {
31623,
30726,
29854,
29007,
28184,
27384,
26607,
25852,
25119,
24406,
23714,
23041,
22387,
21752,
21135,
20535,
19953,
19387,
18836,
18302,
17783,
17278,
16788,
16312,
15849,
15399,
14962,
14538,
14125,
13725,
13335,
12957,
12589,
12232,
11885,
11548,
11220,
10902,
10593,
10292,
10000,
9716,
9441,
9173,
8913,
8660,
8414,
8175,
7943,
7718,
7499,
7286,
7079,
6879,
6683,
6494,
6310,
6131,
5957,
5788,
5623,
5464,
5309,
5158,
5012,
4870,
4732,
4597,
4467,
4340,
4217,
4097,
3981,
3868,
3758,
3652,
3548,
3447,
3350,
3255,
3162,
3073,
2985,
2901,
2818,
2738,
2661,
2585,
2512,
2441,
2371,
2304,
2239,
2175,
2113,
2054,
1995,
1939,
1884,
1830,
1778,
1728,
1679,
1631,
1585,
1540,
1496,
1454,
1413,
1372,
1334,
1296,
1259,
1223,
1189,
1155,
1122,
1090,
1059,
1029,
1000,
972,
944,
917,
891,
866,
841,
818,
794,
772,
750,
729,
708,
688,
668,
649,
631,
613,
596,
579,
562,
546,
531,
516,
501,
487,
473,
460,
447,
434,
422,
410,
398,
387,
376,
365,
355,
345,
335,
325,
316,
307,
299,
290,
282,
274,
266,
259,
251,
244,
237,
230,
224,
218,
211,
205,
200,
194,
188,
183,
178,
173,
168,
163,
158,
154,
150,
145,
141,
137,
133,
130,
126,
122,
119,
115,
112,
109,
106,
103,
100,
97,
94,
92,
89,
87,
84,
82,
79,
77,
75,
73,
71,
69,
67,
65,
63,
61,
60,
58,
56,
55,
53,
52,
50,
49,
47,
46,
45,
43,
42,
41,
40,
39,
38,
37,
35,
34,
33,
33,
32,
31,
30,
29,
28,
27,
27,
26,
25,
24,
24,
23,
22,
22,
21,
21,
20,
19,
19,
18,
18,
17,
17,
16,
16,
15,
15,
15,
14,
14,
13,
13,
13,
12,
12,
12,
11,
11,
11,
10,
10
};


/* *********************************** */
static int setUserMode (proslicChanType *pProslic,BOOLEAN on){
    uInt8 data;
    if (SetSemaphore != NULL){
        while (!(SetSemaphore (pProHW,1)));
        if (on == TRUE){
            if (pProslic->deviceId->usermodeStatus<2)
                pProslic->deviceId->usermodeStatus++;
        } else {
            if (pProslic->deviceId->usermodeStatus>0)
                pProslic->deviceId->usermodeStatus--;
            if (pProslic->deviceId->usermodeStatus != 0)
                return -1;
        }
    }
    data = ProSLIC_ReadReg(pProslic,126);
    if (((data&1) != 0) == on)
        return 0;
    ProSLIC_WriteReg(pProslic,126,2);
    ProSLIC_WriteReg(pProslic,126,8);
    ProSLIC_WriteReg(pProslic,126,0xe);
    ProSLIC_WriteReg(pProslic,126,0);
    if (SetSemaphore != NULL)
        SetSemaphore(pProHW,0);
    return 0;
}

/* *********************************** */
static int setInternalTestLoad(proslicChanType *pProslic, int connect)
{
ramData ram_save;
ramData test_load_addr;
ramData test_load_bitmask;

	if(pProslic->deviceId->chipType >= SI32171 && pProslic->deviceId->chipType <= SI32179)
	{
		test_load_addr = 1516;
		test_load_bitmask = 0x00800000L;  /* bit 23 */
	}
	else 
	{
		test_load_addr = 1611;
		test_load_bitmask = 0x00040000L;  /* bit 18 */
	}

	setUserMode(pProslic,1);
	ram_save = ProSLIC_ReadRAM(pProslic,test_load_addr);

	if(connect)
		ram_save |= test_load_bitmask;
	else
		ram_save &= ~test_load_bitmask;

	ProSLIC_WriteRAM(pProslic,test_load_addr,ram_save);

	setUserMode(pProslic,0);

	return RC_NONE;
}

/* *********************************** */
static void setup1kHzBandpass(proslicChanType *pProslic)
{
/* 1kHz bandpass - Gain = -1.7055 */
    ProSLIC_WriteRAM(pProslic, 32, 0x180A50L);	/* TESTB0_1 */
    ProSLIC_WriteRAM(pProslic, 33, 0x0L);		/* TESTB1_1 */
    ProSLIC_WriteRAM(pProslic, 34, 0x1FE7F5B0L);/* TESTB2_1 */
    ProSLIC_WriteRAM(pProslic, 35, 0xB166220L);	/* TESTA1_1 */
    ProSLIC_WriteRAM(pProslic, 36, 0x185119D0L);/* TESTA2_1 */
    ProSLIC_WriteRAM(pProslic, 37, 0xAF624E0L);	/* TESTB0_2 */
    ProSLIC_WriteRAM(pProslic, 38, 0x0L);		/* TESTB1_2 */
    ProSLIC_WriteRAM(pProslic, 39, 0xAF624E0L);	/* TESTB2_2 */
    ProSLIC_WriteRAM(pProslic, 40, 0x0L); 		/* TESTA1_2 */
    ProSLIC_WriteRAM(pProslic, 41, 0x185119D0L);/* TESTA2_2 */
    ProSLIC_WriteRAM(pProslic, 42, 0x7C6E410L);	/* TESTB0_3 */
    ProSLIC_WriteRAM(pProslic, 43, 0xAFF8B80L);	/* TESTB1_3 */
    ProSLIC_WriteRAM(pProslic, 44, 0x7C6E410L);	/* TESTB2_3 */
    ProSLIC_WriteRAM(pProslic, 45, 0x14E99DE0L);/* TESTA1_3 */
    ProSLIC_WriteRAM(pProslic, 46, 0x185119D0L);/* TESTA2_3 */
    ProSLIC_WriteRAM(pProslic, 50, 0x40000L);	/* TESTAVBW */
    ProSLIC_WriteRAM(pProslic, 49, 0x1F40000L);	/* TESTWLN */
    ProSLIC_WriteRAM(pProslic, 54, 0x0L);		/* TESTAVTH */
    ProSLIC_WriteRAM(pProslic, 53, 0x0L);		/* TESTPKTH */
}

/* *********************************** */
/* Return value in dB*1000 (mdB) */
static int32 dBLookup(uInt32 number)
{
int i;
uInt32 err;

    if(number >= dBTable10_n60[0])
    {
        return 10000;  /* out of range - clamp at 10dB */
    }

    for(i=0;i<MAX_DB_TABLE;i++)  /* 139 */
    {
        if((number < dBTable10_n60[i])&&(number >= dBTable10_n60[i+1]))
        {
        /* See which level it is closest to */
            err = dBTable10_n60[i] - number;
            if(err < (number - dBTable10_n60[i+1]))
            {
               return (10000 - i*DB_TABLE_STEP_SIZE);
            }
            else
            {
               return (10000 - (i+1)*DB_TABLE_STEP_SIZE);
            }
        }
    }
    /* No solution found?  Return -60dB */
    return -60000;
}

/* *********************************** */
static int32 readAudioDiagLevel(proslicChanType *pProslic,int32 zero_dbm_mvpk)
{
int32 data;

	data = ProSLIC_ReadRAM(pProslic,47); /* TESTPKO */
#ifdef ENABLE_DEBUG
	if(pProslic->debugMode)
	{
		LOGPRINT("TESTPKO = %d\n", data);
	}
#endif
	data /= 40145;         /* 2^28 * 0.182 * 10^(Gfilt/20) */
	data *= 1000;
	data /= zero_dbm_mvpk;  
	data *= 10;

	return(dBLookup(data));
}

/* *********************************** */
static char *applyTestLimits(proslicTestObj *test)
{
	if((test->value >= test->lowerLimit)&&(test->value <= test->upperLimit))
	{
		test->testResult = RC_TEST_PASSED;
		return ("PASS");
	}
	else
	{
		test->testResult = RC_TEST_FAILED;
	    return ("FAIL");
	}
}

 

/* *********************************** */
static int logTest(proslicChanType *pProslic,proslicTestObj *test, const char *label)
{
char resultStr[10];

	strcpy(resultStr,applyTestLimits(test));
#ifdef ENABLE_DEBUG
	if(pProslic->debugMode)
	{
		LOGPRINT("ProSLIC : TestIn : %-14s = %-8d :: %s\n", label, test->value,resultStr);
	}
#endif
	return test->testResult;

}
/* *********************************** */
static int logStatus(proslicChanType *pProslic, int status,const char *label)
{
#ifdef ENABLE_DEBUG
	if(pProslic->debugMode)
	{
		LOGPRINT("ProSLIC : TestIn : %-14s = %d\n", label,status);
	}
#endif
	return RC_NONE;
}

/* *********************************** */
static uInt32 Isqrt32(uInt32 num)
{
uInt32 one,res,op;

	op=num;
	res=0;
	one = 1L << 30;
	while(one > op)
		one >>=2;

	while(one !=0)
	{
		if(op >= res + one)
		{
			op = op - (res + one);
			res = res + 2*one;
		}
		res >>= 1;
		one >>= 2;
	}

	return (res);
}


/* *********************************** */
static int storeDCFeedPreset(proslicChanType *pProslic,ProSLIC_DCfeed_Cfg *cfg)
{
	cfg->slope_vlim = ProSLIC_ReadRAM(pProslic,634);
	cfg->slope_rfeed = ProSLIC_ReadRAM(pProslic,635);
	cfg->slope_ilim = ProSLIC_ReadRAM(pProslic,636);
	cfg->delta1 = ProSLIC_ReadRAM(pProslic,638);
	cfg->delta2 = ProSLIC_ReadRAM(pProslic,639);
	cfg->v_vlim = ProSLIC_ReadRAM(pProslic,640);
	cfg->v_rfeed = ProSLIC_ReadRAM(pProslic,641);
	cfg->v_ilim = ProSLIC_ReadRAM(pProslic,642);
	cfg->const_rfeed = ProSLIC_ReadRAM(pProslic,643);
	cfg->const_ilim = ProSLIC_ReadRAM(pProslic,644);
	cfg->i_vlim = ProSLIC_ReadRAM(pProslic,645);
	cfg->lcronhk = ProSLIC_ReadRAM(pProslic,853);
	cfg->lcroffhk = ProSLIC_ReadRAM(pProslic,852);
	cfg->lcrdbi = ProSLIC_ReadRAM(pProslic,701);
	cfg->longhith = ProSLIC_ReadRAM(pProslic,858);
	cfg->longloth = ProSLIC_ReadRAM(pProslic,859);
	cfg->longdbi = ProSLIC_ReadRAM(pProslic,702);
	cfg->lcrmask = ProSLIC_ReadRAM(pProslic,854);
	cfg->lcrmask_polrev = ProSLIC_ReadRAM(pProslic,855);
	cfg->lcrmask_state = ProSLIC_ReadRAM(pProslic,856);
	cfg->lcrmask_linecap = ProSLIC_ReadRAM(pProslic,857);
	cfg->vcm_oh = ProSLIC_ReadRAM(pProslic,748);
	cfg->vov_bat = ProSLIC_ReadRAM(pProslic,752);
	cfg->vov_gnd = ProSLIC_ReadRAM(pProslic,751);

	return RC_NONE;
}


/* *********************************** */
int ProSLIC_createTestInObj(proslicTestInObjType_ptr *pTstin)
{
#ifndef DISABLE_MALLOC
	*pTstin = malloc(sizeof(proslicTestInObjType));
	memset(*pTstin,0,sizeof(proslicTestInObjType));
	return RC_NONE;
#else
	return RC_UNSUPPORTED_FEATURE;
#endif
}
/* *********************************** */
int ProSLIC_destroyTestInObj(proslicTestInObjType_ptr *pTstin)
{
#ifndef DISABLE_MALLOC
	free((proslicTestInObjType_ptr) *pTstin);
	*pTstin = NULL;
	return RC_NONE;
#else
	return RC_UNSUPPORTED_FEATURE;
#endif
}

/* *********************************** */
int ProSLIC_testInPCMLpbkEnable(proslicChanType *pProslic, proslicTestInObjType *pTstin)
{
uInt8 regData;

	/* Valid device check */
	if(TSTIN_INVALID_PART_NUM)
	{
		return RC_UNSUPPORTED_FEATURE;
	}

	/* Check if enabled */
	if(!pTstin->pcmLpbkTest.testEnable)
	{
		return RC_TEST_DISABLED;
	}
	/* Return if already enabled */
	if(pTstin->pcmLpbkTest.pcmLpbkEnabled)
	{
		return RC_NONE;
	}

	/* Store PCM Settings */
	pTstin->pcmLpbkTest.pcmModeSave = ProSLIC_ReadReg(pProslic,11); /* PCMMODE */


	/* Disable PCM bus before changing format */
	regData = pTstin->pcmLpbkTest.pcmModeSave & ~0x10; /* PCM_EN = 0 */
	ProSLIC_WriteReg(pProslic,11,regData);

	/* Configure for either 8 or 16bit linear */
	if(pTstin->pcmLpbkTest.pcm8BitLinear == PCM_8BIT)
	{
		regData |= 0x02;   /* PCM_FMT[1] = 1 */
		regData &= ~0x01;  /* PCM_FMT[0] = 0 */
	}
	else /* PCM_16BIT */
	{
		regData |= 0x03;  /* PCM_FMT[1:0] = 11 */
	}

	ProSLIC_WriteReg(pProslic,11,regData);

	/* Enable PCM Loopback */
	ProSLIC_WriteReg(pProslic,43,0x01);  /* LOOPBACK */

	/* Re-enable PCM Bus */
	ProSLIC_WriteReg(pProslic,11,regData|0x10);  /* PCMMODE */

    pTstin->pcmLpbkTest.pcmLpbkEnabled = 1;
#ifdef ENABLE_DEBUG
	if(pProslic->debugMode)
	{
		LOGPRINT("ProSLIC : TestIn : pcmLpbk : ENABLED\n");
	}
#endif
	return RC_NONE;
}

/* *********************************** */
int ProSLIC_testInPCMLpbkDisable(proslicChanType *pProslic, proslicTestInObjType *pTstin)
{
uInt8 regData;

	/* Valid device check */
	if(TSTIN_INVALID_PART_NUM)
	{
		return RC_UNSUPPORTED_FEATURE;
	}

	/* Check if enabled */
	if(!pTstin->pcmLpbkTest.testEnable)
	{
		return RC_TEST_DISABLED;
	}

	/* Return if already disabled */
	if(!pTstin->pcmLpbkTest.pcmLpbkEnabled)
	{
		return RC_NONE;
	}

	/* Disable PCM Bus */
	regData = ProSLIC_ReadReg(pProslic,11); /* PCMMODE */
	ProSLIC_WriteReg(pProslic,11,regData &= ~0x10);

	/* Disable PCM Loopback */
	ProSLIC_WriteReg(pProslic,43,0);

	/* Restore PCMMODE. Force disabled while changing format */
	ProSLIC_WriteReg(pProslic,11,pTstin->pcmLpbkTest.pcmModeSave &= ~0x10);
	ProSLIC_WriteReg(pProslic,11,pTstin->pcmLpbkTest.pcmModeSave);

	pTstin->pcmLpbkTest.pcmLpbkEnabled = 0;
#ifdef ENABLE_DEBUG
	if(pProslic->debugMode)
	{
		LOGPRINT("ProSLIC : TestIn : pcmLpbk : DISABLED\n");
	}
#endif
	return RC_NONE;
}

/* *************************************************** */

int ProSLIC_testInDCFeed(proslicChanType *pProslic, proslicTestInObjType *pTstin)
{
uInt8 enhanceRegSave;
proslicMonitorType monitor;
ramData lcroffhk_save;
ramData lcronhk_save;

	/* Valid device check */
	if(TSTIN_INVALID_PART_NUM)
	{
		return RC_UNSUPPORTED_FEATURE;
	}

	/* Check if enabled */
	if(!pTstin->dcFeedTest.testEnable)
	{
		return RC_TEST_DISABLED;
	}

	/* Invalidate last test results */
	pTstin->dcFeedTest.testDataValid = TSTIN_RESULTS_INVALID;

	/* Verify line not in use */
	if(ProSLIC_ReadReg(pProslic,34) & 0x02)  /* LCR */
	{
#ifdef ENABLE_DEBUG
		if(pProslic->debugMode)
		{
			LOGPRINT("\nProSLIC : TestIn : DC Feed : Line in Use\n");
		}
#endif
		if(pTstin->dcFeedTest.abortIfLineInUse==ABORT_LIU_ENABLED)
		{
			return RC_LINE_IN_USE;
		}
	}

	/* Disable Powersave */
	enhanceRegSave = ProSLIC_ReadReg(pProslic,47);
	ProSLIC_WriteReg(pProslic,47,0x20);
	Delay(pProTimer,10);

	/* Onhook measurement */
	ProSLIC_LineMonitor(pProslic,&monitor);
	
	pTstin->dcFeedTest.dcfeedVtipOnhook.value = monitor.vtip;
	pTstin->dcFeedTest.dcfeedVringOnhook.value = monitor.vring;
	pTstin->dcFeedTest.dcfeedVloopOnhook.value = monitor.vtr;
	pTstin->dcFeedTest.dcfeedVbatOnhook.value = monitor.vbat;
	pTstin->dcFeedTest.dcfeedItipOnhook.value = monitor.itip;
	pTstin->dcFeedTest.dcfeedIringOnhook.value = monitor.iring;
	pTstin->dcFeedTest.dcfeedIloopOnhook.value = monitor.itr;
	pTstin->dcFeedTest.dcfeedIlongOnhook.value = monitor.ilong;

	/* Modify LCR threshold (optional) before connecting test load */
	if(pTstin->dcFeedTest.applyLcrThresh == LCR_CHECK_ENABLED)
	{
		lcroffhk_save = ProSLIC_ReadRAM(pProslic,852);
		lcronhk_save = ProSLIC_ReadRAM(pProslic,853);
		ProSLIC_WriteRAM(pProslic,852,pTstin->dcFeedTest.altLcrOffThresh);
		ProSLIC_WriteRAM(pProslic,853,pTstin->dcFeedTest.altLcrOnThresh);
	}

	/* Connect internal test load for 2nd dc feed i/v point */
	setInternalTestLoad(pProslic,1);
	Delay(pProTimer,50);
	/* Offhook measurement */
	ProSLIC_LineMonitor(pProslic,&monitor);
	
	pTstin->dcFeedTest.dcfeedVtipOffhook.value = monitor.vtip;
	pTstin->dcFeedTest.dcfeedVringOffhook.value = monitor.vring;
	pTstin->dcFeedTest.dcfeedVloopOffhook.value = monitor.vtr;
	pTstin->dcFeedTest.dcfeedVbatOffhook.value = monitor.vbat;
	pTstin->dcFeedTest.dcfeedItipOffhook.value = monitor.itip;
	pTstin->dcFeedTest.dcfeedIringOffhook.value = monitor.iring;
	pTstin->dcFeedTest.dcfeedIloopOffhook.value = monitor.itr;
	pTstin->dcFeedTest.dcfeedIlongOffhook.value = monitor.ilong;

	pTstin->dcFeedTest.testResult = RC_TEST_PASSED;  /* initialize */
	/* Read LCR */
	if(ProSLIC_ReadReg(pProslic,34) & 0x07)  /* LCRRTP */
	{
		pTstin->dcFeedTest.lcrStatus = 1;
	}
	else
	{
		pTstin->dcFeedTest.lcrStatus = 0;
	}

	/* Only fail check if enabled */
	if(pTstin->dcFeedTest.applyLcrThresh == LCR_CHECK_ENABLED)
	{
		pTstin->dcFeedTest.testResult |= !pTstin->dcFeedTest.lcrStatus;
	}

	/* Disconnect Test Load */
	setInternalTestLoad(pProslic,0);

	/* Restore LCR thresholds */
	if(pTstin->dcFeedTest.applyLcrThresh == LCR_CHECK_ENABLED)
	{
		ProSLIC_WriteRAM(pProslic,852,lcroffhk_save);
		ProSLIC_WriteRAM(pProslic,853,lcronhk_save);
	}

	/* Restore enhance reg */
    ProSLIC_WriteReg(pProslic,47,enhanceRegSave);  

	/* Process Results */
	pTstin->dcFeedTest.testResult |= logTest(pProslic,&(pTstin->dcFeedTest.dcfeedVtipOnhook),"DcFeed : Vtip Onhook");
	pTstin->dcFeedTest.testResult |= logTest(pProslic,&(pTstin->dcFeedTest.dcfeedVringOnhook),"DcFeed : Vring Onhook");
	pTstin->dcFeedTest.testResult |= logTest(pProslic,&(pTstin->dcFeedTest.dcfeedVloopOnhook),"DcFeed : Vloop Onhook");
	pTstin->dcFeedTest.testResult |= logTest(pProslic,&(pTstin->dcFeedTest.dcfeedVbatOnhook),"DcFeed : Vbat Onhook");
	pTstin->dcFeedTest.testResult |= logTest(pProslic,&(pTstin->dcFeedTest.dcfeedItipOnhook),"DcFeed : Itip Onhook");
	pTstin->dcFeedTest.testResult |= logTest(pProslic,&(pTstin->dcFeedTest.dcfeedIringOnhook),"DcFeed : Iring Onhook");
	pTstin->dcFeedTest.testResult |= logTest(pProslic,&(pTstin->dcFeedTest.dcfeedIloopOnhook),"DcFeed : Iloop Onhook");
	pTstin->dcFeedTest.testResult |= logTest(pProslic,&(pTstin->dcFeedTest.dcfeedIlongOnhook),"DcFeed : Ilong Onhook");

	pTstin->dcFeedTest.testResult |= logTest(pProslic,&(pTstin->dcFeedTest.dcfeedVtipOffhook),"DcFeed : Vtip Offhook");
	pTstin->dcFeedTest.testResult |= logTest(pProslic,&(pTstin->dcFeedTest.dcfeedVringOffhook),"DcFeed : Vring Offhook");
	pTstin->dcFeedTest.testResult |= logTest(pProslic,&(pTstin->dcFeedTest.dcfeedVloopOffhook),"DcFeed : Vloop Offhook");
	pTstin->dcFeedTest.testResult |= logTest(pProslic,&(pTstin->dcFeedTest.dcfeedVbatOffhook),"DcFeed : Vbat Offhook");
	pTstin->dcFeedTest.testResult |= logTest(pProslic,&(pTstin->dcFeedTest.dcfeedItipOffhook),"DcFeed : Itip Offhook");
	pTstin->dcFeedTest.testResult |= logTest(pProslic,&(pTstin->dcFeedTest.dcfeedIringOffhook),"DcFeed : Iring Offhook");
	pTstin->dcFeedTest.testResult |= logTest(pProslic,&(pTstin->dcFeedTest.dcfeedIloopOffhook),"DcFeed : Iloop Offhook");
	pTstin->dcFeedTest.testResult |= logTest(pProslic,&(pTstin->dcFeedTest.dcfeedIlongOffhook),"DcFeed : Ilong Offhook");

	logStatus(pProslic,pTstin->dcFeedTest.lcrStatus,"DcFeed : LCR");


	pTstin->dcFeedTest.testDataValid = 1;   /* New valid results */

	/* return cumulative pass/fail result */

	return (pTstin->dcFeedTest.testResult);
}
/* *********************************** */
int ProSLIC_testInRinging(proslicChanType *pProslic, proslicTestInObjType *pTstin)
{
uInt8 ringcon_save,enhance_save;
int32 vtr[MAX_RINGING_SAMPLES];
int i;
uInt8 lf;
uInt32 rtper_save, ringfr_save,ringamp_save,ringof_save,rtacth_save,rtdcth_save;
ProSLIC_DCfeed_Cfg dcfeedCfg;

	/* Valid device check */
	if(TSTIN_INVALID_PART_NUM)
	{
		return RC_UNSUPPORTED_FEATURE;
	}

	/* Check if enabled */
	if(!pTstin->ringingTest.testEnable)
	{
		return RC_TEST_DISABLED;
	}

	/* Verify line not in use */
	if(ProSLIC_ReadReg(pProslic,34) & 0x02)  /* LCR */
	{
#ifdef ENABLE_DEBUG
		if(pProslic->debugMode)
		{
			LOGPRINT("\nProSLIC : TestIn : Ringing : Line in Use\n");
		}
#endif
		if(pTstin->ringingTest.abortIfLineInUse)
		{
			return RC_LINE_IN_USE;
		}
	}

	/* Invalidate last test results */
	pTstin->ringingTest.testDataValid = TSTIN_RESULTS_INVALID;

	/* Check sample size/rate */
	if(pTstin->ringingTest.numSamples > MAX_RINGING_SAMPLES)
		pTstin->ringingTest.numSamples = MAX_RINGING_SAMPLES;

	if(pTstin->ringingTest.sampleInterval > MAX_RINGING_SAMPLE_INTERVAL)
		pTstin->ringingTest.sampleInterval = MAX_RINGING_SAMPLE_INTERVAL;

	if(pTstin->ringingTest.sampleInterval < MIN_RINGING_SAMPLE_INTERVAL)
		pTstin->ringingTest.sampleInterval = MIN_RINGING_SAMPLE_INTERVAL;

	/* Disable Powersave */
	enhance_save = ProSLIC_ReadReg(pProslic,47);
	ProSLIC_WriteReg(pProslic,47,0x20);
	Delay(pProTimer,10);

	/* Disable ring cadencing */
	ringcon_save = ProSLIC_ReadReg(pProslic,38); /* RINGCON */
	ProSLIC_WriteReg(pProslic,38,ringcon_save&0xE7); /* RINGCON */

	/* Must enter ringing through active state */
	lf = ProSLIC_ReadReg(pProslic,30);  /* LINEFEED */
	ProSLIC_SetLinefeedStatus(pProslic,LF_FWD_ACTIVE);
	Delay(pProTimer,20); /* settle */

	/* Start ringing */
	ProSLIC_SetLinefeedStatus(pProslic,LF_RINGING);
	Delay(pProTimer,500);

	/* Verify Ring Started */
	if(ProSLIC_ReadReg(pProslic,30) != 0x44)
	{
		ProSLIC_SetLinefeedStatus(pProslic,LF_FWD_ACTIVE);
		ProSLIC_SetLinefeedStatus(pProslic,LF_OPEN);
        ProSLIC_WriteReg(pProslic,38,ringcon_save);
		ProSLIC_WriteReg(pProslic,47,enhance_save);
		ProSLIC_SetLinefeedStatus(pProslic,lf);
#ifdef ENABLE_DEBUG
		if(pProslic->debugMode)
		{
			LOGPRINT("ProSLIC : TestIn : Ringing : Ring Start Fail\n");
		}
#endif
		pTstin->ringingTest.testResult = RC_TEST_FAILED;
		return RC_RING_START_FAIL;
	}

	/* Capture samples */
	pTstin->ringingTest.ringingVdc.value = 0;
	for(i=0;i<pTstin->ringingTest.numSamples;i++)
	{
		vtr[i] = ProSLIC_ReadMADCScaled(pProslic,69,0); /* VDIFF_FILT */
		pTstin->ringingTest.ringingVdc.value += vtr[i];
#ifdef ENABLE_DEBUG
		if(pProslic->debugMode)
		{
			LOGPRINT("ProSLIC : TestIn : Ringing : Vtr[%d] = %d\n",i,vtr[i]);
		}
#endif
		Delay(pProTimer,pTstin->ringingTest.sampleInterval);
	}
	
	/* Restore linefeed */
	ProSLIC_SetLinefeedStatus(pProslic,LF_FWD_ACTIVE);
	Delay(pProTimer,20);

    /* Process Results */
	pTstin->ringingTest.ringingVdc.value /= pTstin->ringingTest.numSamples;
	for(i=0;i<pTstin->ringingTest.numSamples;i++)
	{
		vtr[i] -= pTstin->ringingTest.ringingVdc.value;
		pTstin->ringingTest.ringingVac.value += ((vtr[i]/100L) * (vtr[i]/100L));
	}


	pTstin->ringingTest.ringingVac.value /= pTstin->ringingTest.numSamples;
	pTstin->ringingTest.ringingVac.value = Isqrt32(pTstin->ringingTest.ringingVac.value);
	pTstin->ringingTest.ringingVac.value *= 100L;

	pTstin->ringingTest.testResult = RC_TEST_PASSED;
	pTstin->ringingTest.testResult |= logTest(pProslic,&(pTstin->ringingTest.ringingVdc),"Ringing : VDC");
	pTstin->ringingTest.testResult |= logTest(pProslic,&(pTstin->ringingTest.ringingVac),"Ringing : VAC");
	/*
	** Optional Ringtrip Test 
	*/

	if(pTstin->ringingTest.ringtripTestEnable == RTP_CHECK_ENABLED)
	{
		/* Setup low voltage linefeed so low level ringing may be used */
		ProSLIC_SetLinefeedStatus(pProslic,LF_OPEN);
		storeDCFeedPreset(pProslic,&dcfeedCfg);
		ProSLIC_DCFeedSetupCfg(pProslic,ringtripTestDCFeedPreset,0);

		/* Optional Ringtrip Test (modified ringer settings to use test load to trip) */
		rtper_save = ProSLIC_ReadRAM(pProslic,755);
		ringfr_save = ProSLIC_ReadRAM(pProslic,844);
		ringamp_save = ProSLIC_ReadRAM(pProslic,845);
		ringof_save = ProSLIC_ReadRAM(pProslic,843);
		rtacth_save = ProSLIC_ReadRAM(pProslic,848);
		rtdcth_save = ProSLIC_ReadRAM(pProslic,847);

		ProSLIC_WriteRAM(pProslic,755,0x50000L);  /* RTPER */
		ProSLIC_WriteRAM(pProslic,844,0x7EFE000L);/* RINGFR */
		ProSLIC_WriteRAM(pProslic,845,0xD6307L);  /* RINGAMP */
		ProSLIC_WriteRAM(pProslic,843,0x0L);      /* RINGOF */
		ProSLIC_WriteRAM(pProslic,848,0x7827FL);  /* RTACTH */
		ProSLIC_WriteRAM(pProslic,847,0xFFFFFFFL);/* RTDCTH */

		/* Start ringing from active state */
		ProSLIC_SetLinefeedStatus(pProslic,LF_FWD_ACTIVE);
		Delay(pProTimer,20);
		ProSLIC_SetLinefeedStatus(pProslic,LF_RINGING);
		Delay(pProTimer,200);

			/* Verify Ring Started */
		if(ProSLIC_ReadReg(pProslic,30) != 0x44)
		{
			ProSLIC_SetLinefeedStatus(pProslic,LF_FWD_ACTIVE);
			ProSLIC_SetLinefeedStatus(pProslic,LF_OPEN);
			ProSLIC_WriteReg(pProslic,38,ringcon_save);
			ProSLIC_WriteReg(pProslic,47,enhance_save);
			ProSLIC_SetLinefeedStatus(pProslic,lf);
#ifdef ENABLE_DEBUG
			if(pProslic->debugMode)
			{
				LOGPRINT("ProSLIC : TestIn : Ringtrip : Ring Start Fail\n");
			}
#endif
			pTstin->ringingTest.testResult=RC_TEST_FAILED;
			return RC_RING_START_FAIL;
		}

		/* Connect Test Load to cause ringtrip */
	    setInternalTestLoad(pProslic,1);
		Delay(pProTimer,200);

		/* Check for RTP */
		if(ProSLIC_ReadReg(pProslic,34) & 0x01)  /* LCRRTP */
		{
			pTstin->ringingTest.rtpStatus = 1;
		    pTstin->ringingTest.testResult |= RC_TEST_PASSED;
		}
		else
		{
			pTstin->ringingTest.rtpStatus = 0;
			pTstin->ringingTest.testResult |= RC_TEST_FAILED;
			ProSLIC_SetLinefeedStatus(pProslic,LF_FWD_ACTIVE);
		}		
	    setInternalTestLoad(pProslic,0);
		Delay(pProTimer,20);

		logStatus(pProslic,pTstin->ringingTest.rtpStatus,"Ringing : RTP");

		/* Restore DC Feed */
		ProSLIC_DCFeedSetupCfg(pProslic,&dcfeedCfg,0);

		/* Restore Ring Settings */
		ProSLIC_WriteRAM(pProslic,755,rtper_save);/* RTPER */
		ProSLIC_WriteRAM(pProslic,844,ringfr_save);/*RINGFR */
		ProSLIC_WriteRAM(pProslic,845,ringamp_save); /* RINGAMP */
		ProSLIC_WriteRAM(pProslic,843,ringof_save); /* RINGOF */
		ProSLIC_WriteRAM(pProslic,848,rtacth_save);/* RTACTH */
		ProSLIC_WriteRAM(pProslic,847,rtdcth_save);/* RTDCTH */

	}/* end of ringtrip test 

	/* Restore Linefeed */
	ProSLIC_SetLinefeedStatus(pProslic,lf);

	/* Restore RINGCON and ENHANCE */
	ProSLIC_WriteReg(pProslic,38,ringcon_save); 
    ProSLIC_WriteReg(pProslic,47,enhance_save);  

	pTstin->ringingTest.testDataValid = TSTIN_RESULTS_VALID;

	return (pTstin->ringingTest.testResult);
}

/* *********************************** */
int ProSLIC_testInBattery(proslicChanType *pProslic, proslicTestInObjType *pTstin)
{
proslicMonitorType monitor;

	/* Valid device check */
	if(TSTIN_INVALID_PART_NUM)
	{
		return RC_UNSUPPORTED_FEATURE;
	}

	/* Check if enabled */
	if(!pTstin->batteryTest.testEnable)
	{
		return RC_TEST_DISABLED;
	}

	/* Invalidate last test results */
	pTstin->batteryTest.testDataValid = TSTIN_RESULTS_INVALID;

	/* Measure Battery */
	ProSLIC_LineMonitor(pProslic,&monitor);

	pTstin->batteryTest.vbat.value = monitor.vbat;

	pTstin->batteryTest.testResult = logTest(pProslic,&(pTstin->batteryTest.vbat),"Battery : VBAT");

	pTstin->batteryTest.testDataValid = TSTIN_RESULTS_VALID;   /* New valid results */

	return (pTstin->batteryTest.testResult);
}

/* *********************************** */
int ProSLIC_testInAudio(proslicChanType *pProslic, proslicTestInObjType *pTstin)
{
uInt8 enhanceRegSave;
uInt8 lf;
int32 data;
int32 gainMeas1,gainMeas2;
int32 gainMeas3 = 0;
ProSLIC_audioGain_Cfg gainCfg;
int32 Pin = -3980;   /* -10dBm + 6.02dB (since OHT w/ no AC load) */

	/* Valid device check */
	if(TSTIN_INVALID_PART_NUM)
	{
		return RC_UNSUPPORTED_FEATURE;
	}

	/* Check if enabled */
	if(!pTstin->audioTest.testEnable)
	{
		return RC_TEST_DISABLED;
	}

	/* Invalidate last test results */
	pTstin->audioTest.testDataValid = TSTIN_RESULTS_INVALID;

	/* Verify line not in use */
	if(ProSLIC_ReadReg(pProslic,34) & 0x02)  /* LCR */
	{
#ifdef ENABLE_DEBUG
		if(pProslic->debugMode)
		{
			LOGPRINT("\nProSLIC : TestIn : Audio : Line in Use\n");
		}
#endif
		if(pTstin->audioTest.abortIfLineInUse == ABORT_LIU_ENABLED)
		{
			return RC_LINE_IN_USE;
		}
	}

	/* Disable Powersave */
	enhanceRegSave = ProSLIC_ReadReg(pProslic,47);
	ProSLIC_WriteReg(pProslic,47,0x20);
	Delay(pProTimer,10);


	/* Setup Audio Filter, enable audio in OHT */
	lf = ProSLIC_ReadReg(pProslic,30);  /* LINEFEED */
	ProSLIC_SetLinefeedStatus(pProslic,LF_FWD_ACTIVE);
	Delay(pProTimer,20); /* settle */
	setup1kHzBandpass(pProslic);
	ProSLIC_SetLinefeedStatus(pProslic,LF_FWD_OHT);

	/* Setup osc1 for 1kHz -10dBm tone, disable hybrid, enable filters */
	ProSLIC_WriteRAM(pProslic,26,0x5A80000L); /* OSC1FREQ */
	ProSLIC_WriteRAM(pProslic,27,0x5D8000L);  /* OSC1AMP */
	ProSLIC_WriteReg(pProslic,48,0x02);       /* OMODE */
	ProSLIC_WriteReg(pProslic,49,0x01);       /* OCON */
	ProSLIC_WriteReg(pProslic,44,0x10);       /* DIGCON */
	ProSLIC_WriteReg(pProslic,71,0x10);       /* DIAG1 */

	/* Settle */
	Delay(pProTimer,800);

	/* Read first gain measurement (Gtx + Grx + Gzadj) */
	gainMeas1 = readAudioDiagLevel(pProslic,pTstin->audioTest.zerodBm_mVpk);

	/* Bypass TXACHPF and set TXACEQ to unity */
	gainCfg.acgain = ProSLIC_ReadRAM(pProslic,544);  /* TXACGAIN */
	gainCfg.aceq_c0 = ProSLIC_ReadRAM(pProslic,540); /* TXACEQ_C0 */
	gainCfg.aceq_c1 = ProSLIC_ReadRAM(pProslic,541); /* TXACEQ_C1 */
	gainCfg.aceq_c2 = ProSLIC_ReadRAM(pProslic,542); /* TXACEQ_C2 */
	gainCfg.aceq_c3 = ProSLIC_ReadRAM(pProslic,543); /* TXACEQ_C3 */
	ProSLIC_WriteRAM(pProslic,544,0x8000000L); 
	ProSLIC_WriteRAM(pProslic,543,0x0L);
	ProSLIC_WriteRAM(pProslic,542,0x0L);
	ProSLIC_WriteRAM(pProslic,541,0x0L);
	ProSLIC_WriteRAM(pProslic,540,0x8000000L);
	ProSLIC_WriteReg(pProslic,44,0x18);

	/* Settle */
	Delay(pProTimer,800);

	/* Read second level measurement (RX level only) */
	gainMeas2 = readAudioDiagLevel(pProslic,pTstin->audioTest.zerodBm_mVpk);

	/* Adjust txgain if TXACGAIN wasn't unity during gainMeas1 */
	if(gainCfg.acgain != 0x8000000L)
	{
		data = (gainCfg.acgain*10)/134217;
		gainMeas3 = dBLookup(data);
	}

	/* Computations */
	pTstin->audioTest.rxGain.value = gainMeas2 - Pin;
	pTstin->audioTest.txGain.value = gainMeas1 - gainMeas2 + gainMeas3;
	
#ifdef ENABLE_DEBUG
	if(pProslic->debugMode)
	{
		LOGPRINT("ProSLIC : TestIn : Audio : gainMeas1 = %d\n", gainMeas1);
		LOGPRINT("ProSLIC : TestIn : Audio : gainMeas2 = %d\n", gainMeas2);
		LOGPRINT("ProSLIC : TestIn : Audio : gainMeas3 = %d\n", gainMeas3);
	}
#endif
	pTstin->audioTest.testResult = RC_TEST_PASSED;
	pTstin->audioTest.testResult |= logTest(pProslic,&(pTstin->audioTest.rxGain),"RX Path Gain");
	pTstin->audioTest.testResult |= logTest(pProslic,&(pTstin->audioTest.txGain),"TX Path Gain");



	/*
	** Restore 
	*/

	/* Need to store/restore all modified reg/RAM */
	ProSLIC_WriteRAM(pProslic,544,gainCfg.acgain); 
	ProSLIC_WriteRAM(pProslic,540,gainCfg.aceq_c0);
	ProSLIC_WriteRAM(pProslic,541,gainCfg.aceq_c1);
	ProSLIC_WriteRAM(pProslic,542,gainCfg.aceq_c2);
	ProSLIC_WriteRAM(pProslic,543,gainCfg.aceq_c3);

	ProSLIC_WriteReg(pProslic,71,0x0);  /* DIAG1 */
	ProSLIC_WriteReg(pProslic,44,0x0);  /* DIGCON */
	ProSLIC_WriteReg(pProslic,48,0x0);  /* OMODE */
	ProSLIC_WriteReg(pProslic,49,0x0);  /* OCON */
	ProSLIC_SetLinefeedStatus(pProslic,LF_FWD_ACTIVE);
	ProSLIC_WriteReg(pProslic,47,enhanceRegSave);
	ProSLIC_SetLinefeedStatus(pProslic,lf);

	/* Validate last test results */
	pTstin->audioTest.testDataValid = TSTIN_RESULTS_VALID;

	return pTstin->audioTest.testResult;
}


/* *********************************** */

int ProSLIC_testInPcmLpbkSetup(proslicTestInObjType *pTstin,proslicPcmLpbkTest *pcmLpbkTest)
{
	pTstin->pcmLpbkTest = *pcmLpbkTest;
	pTstin->pcmLpbkTest.testEnable = 1;

	return RC_NONE;
}

/* *********************************** */

int ProSLIC_testInDcFeedSetup(proslicTestInObjType *pTstin,proslicDcFeedTest *dcFeedTest)
{
	pTstin->dcFeedTest = *dcFeedTest;
	pTstin->dcFeedTest.testEnable = 1;

	return RC_NONE;
}

/* *********************************** */

int ProSLIC_testInRingingSetup(proslicTestInObjType *pTstin, proslicRingingTest *ringingTest)
{
	/* Copy limits per-channel */

	pTstin->ringingTest = *ringingTest;
	pTstin->ringingTest.testEnable = 1;

	return RC_NONE;
}
/* *********************************** */

int ProSLIC_testInBatterySetup(proslicTestInObjType *pTstin, proslicBatteryTest *batteryTest)
{
	/* Copy limits per-channel */

	pTstin->batteryTest = *batteryTest;
	pTstin->batteryTest.testEnable = 1;

	return RC_NONE;
}

/* *********************************** */

int ProSLIC_testInAudioSetup(proslicTestInObjType *pTstin, proslicAudioTest *audioTest)
{
	/* Copy limits per-channel */

	pTstin->audioTest = *audioTest;
	pTstin->audioTest.testEnable = 1;

	return RC_NONE;
}


/* *********************************** */
int ProSLIC_testInPrintLimits(proslicChanType *pProslic, proslicTestInObjType *pTstin)
{
	/* Valid device check */
	if(TSTIN_INVALID_PART_NUM)
	{
		return RC_UNSUPPORTED_FEATURE;
	}
#ifdef ENABLE_DEBUG
	if(pProslic->debugMode)
	{
		LOGPRINT("\n");
		LOGPRINT("************   Test-In Test Limits **************\n");
		LOGPRINT("----------------------------------------------------------------\n");
		LOGPRINT("ProSLIC : Test-In : Limits : vtip_on_min (mv) = %d\n",pTstin->dcFeedTest.dcfeedVtipOnhook.lowerLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : vtip_on_max (mv) = %d\n",pTstin->dcFeedTest.dcfeedVtipOnhook.upperLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : vring_on_min (mv) = %d\n",pTstin->dcFeedTest.dcfeedVringOnhook.lowerLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : vring_on_max (mv) = %d\n",pTstin->dcFeedTest.dcfeedVringOnhook.upperLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : vloop_on_min (mv) = %d\n",pTstin->dcFeedTest.dcfeedVloopOnhook.lowerLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : vloop_on_max (mv) = %d\n",pTstin->dcFeedTest.dcfeedVloopOnhook.upperLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : vbat_on_min (mv) = %d\n",pTstin->dcFeedTest.dcfeedVbatOnhook.lowerLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : vbat_on_max (mv) = %d\n",pTstin->dcFeedTest.dcfeedVbatOnhook.upperLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : itip_on_min (ua) = %d\n",pTstin->dcFeedTest.dcfeedItipOnhook.lowerLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : itip_on_max (ua) = %d\n",pTstin->dcFeedTest.dcfeedItipOnhook.upperLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : iring_on_min (ua) = %d\n",pTstin->dcFeedTest.dcfeedIringOnhook.lowerLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : iring_on_max (ua) = %d\n",pTstin->dcFeedTest.dcfeedIringOnhook.upperLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : iloop_on_min (ua) = %d\n",pTstin->dcFeedTest.dcfeedIloopOnhook.lowerLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : iloop_on_max (ua) = %d\n",pTstin->dcFeedTest.dcfeedIloopOnhook.upperLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : ilong_on_min (ua) = %d\n",pTstin->dcFeedTest.dcfeedIlongOnhook.lowerLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : ilong_on_max (ua) = %d\n",pTstin->dcFeedTest.dcfeedIlongOnhook.upperLimit);
		LOGPRINT("----------------------------------------------------------------\n");
		LOGPRINT("ProSLIC : Test-In : Limits : vtip_off_min (mv) = %d\n",pTstin->dcFeedTest.dcfeedVtipOffhook.lowerLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : vtip_off_max (mv) = %d\n",pTstin->dcFeedTest.dcfeedVtipOffhook.upperLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : vring_off_min (mv) = %d\n",pTstin->dcFeedTest.dcfeedVringOffhook.lowerLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : vring_off_max (mv) = %d\n",pTstin->dcFeedTest.dcfeedVringOffhook.upperLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : vloop_off_min (mv) = %d\n",pTstin->dcFeedTest.dcfeedVloopOffhook.lowerLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : vloop_off_max (mv) = %d\n",pTstin->dcFeedTest.dcfeedVloopOffhook.upperLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : vbat_off_min (mv) = %d\n",pTstin->dcFeedTest.dcfeedVbatOffhook.lowerLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : vbat_off_max (mv) = %d\n",pTstin->dcFeedTest.dcfeedVbatOffhook.upperLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : itip_off_min (ua) = %d\n",pTstin->dcFeedTest.dcfeedItipOffhook.lowerLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : itip_off_max (ua) = %d\n",pTstin->dcFeedTest.dcfeedItipOffhook.upperLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : iring_off_min (ua) = %d\n",pTstin->dcFeedTest.dcfeedIringOffhook.lowerLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : iring_off_max (ua) = %d\n",pTstin->dcFeedTest.dcfeedIringOffhook.upperLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : iloop_off_min (ua) = %d\n",pTstin->dcFeedTest.dcfeedIloopOffhook.lowerLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : iloop_off_max (ua) = %d\n",pTstin->dcFeedTest.dcfeedIloopOffhook.upperLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : ilong_off_min (ua) = %d\n",pTstin->dcFeedTest.dcfeedIlongOffhook.lowerLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : ilong_off_max (ua) = %d\n",pTstin->dcFeedTest.dcfeedIlongOffhook.upperLimit);
		LOGPRINT("----------------------------------------------------------------\n");
		LOGPRINT("ProSLIC : Test-In : Limits : ringing_vac_min (mv) = %d\n",pTstin->ringingTest.ringingVac.lowerLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : ringing_vac_max (mv) = %d\n",pTstin->ringingTest.ringingVac.upperLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : ringing_vdc_min (mv) = %d\n",pTstin->ringingTest.ringingVdc.lowerLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : ringing_vdc_max (mv) = %d\n",pTstin->ringingTest.ringingVdc.upperLimit);
		LOGPRINT("----------------------------------------------------------------\n");
		LOGPRINT("ProSLIC : Test-In : Limits : battery_vbat_min (mv) = %d\n",pTstin->batteryTest.vbat.lowerLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : battery_vbat_max (mv) = %d\n",pTstin->batteryTest.vbat.upperLimit);
		LOGPRINT("----------------------------------------------------------------\n");
		LOGPRINT("ProSLIC : Test-In : Limits : audio_txgain_min (mdB) = %d\n",pTstin->audioTest.txGain.lowerLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : audio_txgain_max (mdB) = %d\n",pTstin->audioTest.txGain.upperLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : audio_rxgain_min (mdB) = %d\n",pTstin->audioTest.rxGain.lowerLimit);
		LOGPRINT("ProSLIC : Test-In : Limits : audio_rxgain_max (mdB) = %d\n",pTstin->audioTest.rxGain.upperLimit);
		LOGPRINT("----------------------------------------------------------------\n");
		LOGPRINT("\n");
	}
#endif
	return RC_NONE;
}

