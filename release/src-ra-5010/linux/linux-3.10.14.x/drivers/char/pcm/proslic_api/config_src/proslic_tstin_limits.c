/*
** Copyright (c) 2011 Silicon Laboratories, Inc.
** 2011-09-08 08:51:02
**
**  ProSLIC API Configuration Tool Version 2.6.0
**
** Test-In Configuration Utility
*/



#include "proslic_tstin.h"

proslicPcmLpbkTest ProSLIC_testIn_PcmLpbk_Test = {
0,		/* Enable/Disable (read only) */
0,		/* LPBK state (read only) */
PCM_8BIT,   /* PCM Format */
0,		/* PCM MODE (read only) */
0 		/* Test Result (read only) */
};

proslicDcFeedTest ProSLIC_testIn_DcFeed_Test = {
0,              /* Enable/Disable (read only) */
TSTIN_RESULTS_INVALID,              /* Data Valid Status(read only) */
ABORT_LIU_ENABLED,              /* Abort if Line-In-Use */
LCR_CHECK_ENABLED,              /* LCR Check Enable/Disable */
0x0048D595L,    /* Alt LCROFFHK for LCR Test */
0x0036A030L,    /* Alt LCRONFHK for LCR Test */
0,              /* LCR State(read only) */
{  1000,    5000,  0,  0},	/* vtip onhook */
{ 45000,   58000,  0,  0},	/* vring onhook */
{ 43000,   52000,  0,  0},	/* vloop onhook */
{ 43000,   62000,  0,  0},	/* vbat onhook */
{ -2000,    2000,  0,  0},	/* itip onhook */
{ -2000,    2000,  0,  0},	/* iring onhook */
{ -1500,    1500,  0,  0},	/* iloop onhook */
{ -2000,    2000,  0,  0},	/* ilong onhook */
{  2000,   10000,  0,  0},	/* vtip offhook */
{ 44000,   51000,  0,  0},	/* vring offhook */
{ 34000,   46000,  0,  0},	/* vloop offhook */
{ 36000,   60000,  0,  0},	/* vbat offhook */
{  8000,   20000,  0,  0},	/* itip offhook */
{-20000,   -8000,  0,  0},	/* iring offhook */
{  8000,   20000,  0,  0},	/* iloop offhook */
{ -2000,    2000,  0,  0},	/* ilong offhook */
0              /* Cumulative Test Result(read only) */
};

proslicRingingTest ProSLIC_testIn_Ringing_Test = {
0,              /* Enable/Disable (read only) */
TSTIN_RESULTS_INVALID,       /* Data Valid Status(read only) */
ABORT_LIU_ENABLED,       /* Abort if Line-In-Use */
90,             /* Number of Samples */
10,             /* Sample Interval (ms) */
RTP_CHECK_ENABLED,      /* RTP Check Enable/Disable */
0,              /* RTP State(read only) */
{ 45000,   68000,  0,  0},	/* VAC */
{ -2000,    2000,  0,  0},	/* VDC */
0              /* Cumulative Test Result(read only) */
};

proslicBatteryTest ProSLIC_testIn_Battery_Test = {
0,              /* Enable/Disable (read only) */
TSTIN_RESULTS_INVALID,       /* Data Valid Status(read only) */
{ 50000,   60000,  0,  0},	/* VBAT */
0              /* Cumulative Test Result(read only) */
};

proslicAudioTest ProSLIC_testIn_Audio_Test = {
0,              /* Enable/Disable (read only) */
TSTIN_RESULTS_INVALID,       /* Data Valid Status(read only) */
ABORT_LIU_ENABLED,              /* Abort if Line-In-Use */
1095,              /* 0dBm Voltage (mvpk) */
{-11000,    1000,  0,  0},	/* TX Path Gain */
{-11000,    1000,  0,  0},	/* RX Path Gain */
0              /* Cumulative Test Result(read only) */
};


