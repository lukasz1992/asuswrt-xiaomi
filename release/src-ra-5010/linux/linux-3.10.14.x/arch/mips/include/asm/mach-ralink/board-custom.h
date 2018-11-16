/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef __ARCH_ARM_MACH_MT6575_CUSTOM_BOARD_H
#define __ARCH_ARM_MACH_MT6575_CUSTOM_BOARD_H

#include <linux/autoconf.h>

/*=======================================================================*/
/* MT6575 SD                                                             */
/*=======================================================================*/
#ifdef MTK_EMMC_SUPPORT
#define CFG_DEV_MSDC0
#endif
#define CFG_DEV_MSDC1
#define CFG_DEV_MSDC2
#define CFG_DEV_MSDC3
#if defined(CONFIG_MTK_COMBO) || defined(CONFIG_MTK_COMBO_MODULE)
/*
SDIO slot index number used by connectivity combo chip:
0: invalid (used by memory card)
1: MSDC1
2: MSDC2
*/
#define CONFIG_MTK_WCN_CMB_SDIO_SLOT  (2) /* MSDC2 */
#else
#undef CONFIG_MTK_WCN_CMB_SDIO_SLOT
#endif

#if 0 /* FIXME. */
/*=======================================================================*/
/* MT6575 UART                                                           */
/*=======================================================================*/
#define CFG_DEV_UART1
#define CFG_DEV_UART2
#define CFG_DEV_UART3
#define CFG_DEV_UART4

#define CFG_UART_PORTS          (4)

/*=======================================================================*/
/* MT6575 I2C                                                            */
/*=======================================================================*/
#define CFG_DEV_I2C
//#define CFG_I2C_HIGH_SPEED_MODE
//#define CFG_I2C_DMA_MODE

/*=======================================================================*/
/* MT6575 ADB                                                            */
/*=======================================================================*/
#define ADB_SERIAL "E1K"

#endif

/*=======================================================================*/
/* MT6575 NAND FLASH                                                     */
/*=======================================================================*/
#if 0
#define RAMDOM_READ 1<<0
#define CACHE_READ  1<<1
/*******************************************************************************
 * NFI & ECC Configuration 
 *******************************************************************************/
typedef struct
{
    u16 id;			//deviceid+menuid
    u8  addr_cycle;
    u8  iowidth;
    u16 totalsize;	
    u16 blocksize;
    u16 pagesize;
    u32 timmingsetting;
    char devciename[14];
    u32 advancedmode;   //
}flashdev_info,*pflashdev_info;

static const flashdev_info g_FlashTable[]={
    //micro
    {0xAA2C,  5,  8,  256,	128,  2048,  0x01113,  "MT29F2G08ABD",	0},
    {0xB12C,  4,  16, 128,	128,  2048,  0x01113,  "MT29F1G16ABC",	0},
    {0xBA2C,  5,  16, 256,	128,  2048,  0x01113,  "MT29F2G16ABD",	0}, 
    {0xAC2C,  5,  8,  512,	128,  2048,  0x01113,  "MT29F4G08ABC",	0},
    {0xBC2C,  5,  16, 512,	128,  2048,  0x44333,  "MT29F4G16ABD",	0},
    //samsung 
    {0xBAEC,  5,  16, 256,	128,  2048,  0x01123,  "K522H1GACE",	0},
    {0xBCEC,  5,  16, 512,	128,  2048,  0x01123,  "K524G2GACB",	0},
    {0xDAEC,  5,  8,  256,	128,  2048,  0x33222,  "K9F2G08U0A",	RAMDOM_READ},
    {0xF1EC,  4,  8,  128,	128,  2048,  0x01123,  "K9F1G08U0A",	RAMDOM_READ},
    {0xAAEC,  5,  8,  256,	128,  2048,  0x01123,  "K9F2G08R0A",	0},
    //hynix
    {0xD3AD,  5,  8,  1024, 256,  2048,  0x44333,  "HY27UT088G2A",	0},
    {0xA1AD,  4,  8,  128,	128,  2048,  0x01123,  "H8BCSOPJOMCP",	0},
    {0xBCAD,  5,  16, 512,	128,  2048,  0x01123,  "H8BCSOUNOMCR",	0},
    {0xBAAD,  5,  16, 256,	128,  2048,  0x01123,  "H8BCSOSNOMCR",	0},
    //toshiba
    {0x9598,  5,  16, 816,	128,  2048,  0x00113,  "TY9C000000CMG", 0},
    {0x9498,  5,  16, 375,	128,  2048,  0x00113,  "TY9C000000CMG", 0},
    {0xC198,  4,  16, 128,	128,  2048,  0x44333,  "TC58NWGOS8C",	0},
    {0xBA98,  5,  16, 256,	128,  2048,  0x02113,  "TC58NYG1S8C",	0},
    //st-micro
    {0xBA20,  5,  16, 256,	128,  2048,  0x01123,  "ND02CGR4B2DI6", 0},

    // elpida
    {0xBC20,  5,  16, 512,  128,  2048,  0x01123,  "04GR4B2DDI6",   0},
    {0x0000,  0,  0,  0,	0,	  0,	 0, 	   "xxxxxxxxxxxxx", 0}
};
#endif
	
	
#define NFI_DEFAULT_ACCESS_TIMING        (0x44333)

//uboot only support 1 cs
#define NFI_CS_NUM                  (2)
#define NFI_DEFAULT_CS				(0)

#define USE_AHB_MODE                	(1)

#define PLATFORM_EVB                (1)

#endif /* __ARCH_ARM_MACH_MT6575_CUSTOM_BOARD_H */

