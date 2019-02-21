/*
 * wm8960.h  --  WM8960 Soc Audio driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _WM8960_H
#define _WM8960_H

/* WM8960 register space */


#define WM8960_CACHEREGNUM 	56

#define WM8960_LINVOL		0x0
#define WM8960_RINVOL		0x1
#define WM8960_LOUT1		0x2
#define WM8960_ROUT1		0x3
#define WM8960_CLOCK1		0x4
#define WM8960_DACCTL1		0x5
#define WM8960_DACCTL2		0x6
#define WM8960_IFACE1		0x7
#define WM8960_CLOCK2		0x8
#define WM8960_IFACE2		0x9
#define WM8960_LDAC		0xa
#define WM8960_RDAC		0xb

#define WM8960_RESET		0xf
#define WM8960_3D		0x10
#define WM8960_ALC1		0x11
#define WM8960_ALC2		0x12
#define WM8960_ALC3		0x13
#define WM8960_NOISEG		0x14
#define WM8960_LADC		0x15
#define WM8960_RADC		0x16
#define WM8960_ADDCTL1		0x17
#define WM8960_ADDCTL2		0x18
#define WM8960_POWER1		0x19
#define WM8960_POWER2		0x1a
#define WM8960_ADDCTL3		0x1b
#define WM8960_APOP1		0x1c
#define WM8960_APOP2		0x1d

#define WM8960_LINPATH		0x20
#define WM8960_RINPATH		0x21
#define WM8960_LOUTMIX		0x22

#define WM8960_ROUTMIX		0x25
#define WM8960_MONOMIX1		0x26
#define WM8960_MONOMIX2		0x27
#define WM8960_LOUT2		0x28
#define WM8960_ROUT2		0x29
#define WM8960_MONO		0x2a
#define WM8960_INBMIX1		0x2b
#define WM8960_INBMIX2		0x2c
#define WM8960_BYPASS1		0x2d
#define WM8960_BYPASS2		0x2e
#define WM8960_POWER3		0x2f
#define WM8960_ADDCTL4		0x30
#define WM8960_CLASSD1		0x31

#define WM8960_CLASSD3		0x33
#define WM8960_PLL1		0x34
#define WM8960_PLL2		0x35
#define WM8960_PLL3		0x36
#define WM8960_PLL4		0x37


/*
 * WM8960 Clock dividers
 */
#define WM8960_SYSCLKDIV 		0
#define WM8960_DACDIV			1
#define WM8960_OPCLKDIV			2
#define WM8960_DCLKDIV			3
#define WM8960_TOCLKSEL			4

#define WM8960_SYSCLK_DIV_1		(0 << 1)
#define WM8960_SYSCLK_DIV_2		(2 << 1)

#define WM8960_SYSCLK_MCLK		(0 << 0)
#define WM8960_SYSCLK_PLL		(1 << 0)

#define WM8960_DAC_DIV_1		(0 << 3)
#define WM8960_DAC_DIV_1_5		(1 << 3)
#define WM8960_DAC_DIV_2		(2 << 3)
#define WM8960_DAC_DIV_3		(3 << 3)
#define WM8960_DAC_DIV_4		(4 << 3)
#define WM8960_DAC_DIV_5_5		(5 << 3)
#define WM8960_DAC_DIV_6		(6 << 3)

#define WM8960_DCLK_DIV_1_5		(0 << 6)
#define WM8960_DCLK_DIV_2		(1 << 6)
#define WM8960_DCLK_DIV_3		(2 << 6)
#define WM8960_DCLK_DIV_4		(3 << 6)
#define WM8960_DCLK_DIV_6		(4 << 6)
#define WM8960_DCLK_DIV_8		(5 << 6)
#define WM8960_DCLK_DIV_12		(6 << 6)
#define WM8960_DCLK_DIV_16		(7 << 6)

#define WM8960_TOCLK_F19		(0 << 1)
#define WM8960_TOCLK_F21		(1 << 1)

#define WM8960_OPCLK_DIV_1		(0 << 0)
#define WM8960_OPCLK_DIV_2		(1 << 0)
#define WM8960_OPCLK_DIV_3		(2 << 0)
#define WM8960_OPCLK_DIV_4		(3 << 0)
#define WM8960_OPCLK_DIV_5_5		(4 << 0)
#define WM8960_OPCLK_DIV_6		(5 << 0)

#ifdef CONFIG_SND_RALINK_SOC
#define ADDITIONAL1_DATSEL(x)         	(((x) & 0x3) << 2)
#define PWRMGMT3_LMIC			(1<<5)
#define PWRMGMT3_RMIC                   (1<<4)
#define PWRMGMT2_DACL               	(1 << 8)
#define PWRMGMT2_DACR                   (1 << 7)
#define PWRMGMT2_LOUT1                  (1 << 6)
#define PWRMGMT2_ROUT1                  (1 << 5)
#define PWRMGMT2_SPKL                   (1 << 4)
#define PWRMGMT2_SPKR                   (1 << 3)
#define PWRMGMT2_OUT3                   (1 << 1)
#define PWRMGMT2_PLL_EN                 (1 << 0)
#define PWRMGMT3_LOMIX                  (1<<3)
#define PWRMGMT3_ROMIX                  (1<<2)
#define PWRMGMT1_VMIDSEL_DISABLED   	(0 << 7)
#define PWRMGMT1_VMIDSEL_50K        	(1 << 7)
#define PWRMGMT1_VMIDSEL_250K       	(2 << 7)
#define PWRMGMT1_VMIDSEL_5K         	(3 << 7)
#define PWRMGMT1_VREF                   (1 << 6)
#define PWRMGMT1_AINL                   (1 << 5)
#define PWRMGMT1_AINR                   (1 << 4)
#define PWRMGMT1_ADCL                   (1 << 3)
#define PWRMGMT1_ADCR                   (1 << 2)
#define PWRMGMT1_MICB                   (1 << 1)
#define PWRMGMT1_DIGENB                 (1 << 0)
#define AINTFCE1_WL_16               	(0 << 2)
#define AINTFCE1_FORMAT_I2S             (2 << 0)
#define CLOCKING1_SYSCLKDIV_2		(2 << 1)
#define CLOCKING1_CLKSEL_PLL		(1 << 0)


#endif
#endif
