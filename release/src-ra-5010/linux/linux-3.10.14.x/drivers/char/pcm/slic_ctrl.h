/*
** $Id: slic_ctrl.h,v 1.2 2012-06-27 08:55:26 qwert Exp $
*/
/************************************************************************
 *
 *	Copyright (C) 2008 Trendchip Technologies, Corp.
 *	All Rights Reserved.
 *
 * Trendchip Confidential; Need to Know only.
 * Protected as an unpublished work.
 *
 * The computer program listings, specifications and documentation
 * herein are the property of Trendchip Technologies, Co. and shall
 * not be reproduced, copied, disclosed, or used in whole or in part
 * for any reason without the prior express written permission of
 * Trendchip Technologeis, Co.
 *
 *************************************************************************/
/*
** $Log: slic_ctrl.h,v $
** Revision 1.2  2012-06-27 08:55:26  qwert
** Add SPI CTRL for SLIC example
**
** Revision 1.1  2012-02-10 03:36:26  qwert
** Add looback mode for release version
**
** Revision 1.2  2012-01-10 07:27:14  qwert
** Add GPIO >= 16 case
**
** Revision 1.1  2011-11-14 07:49:29  qwert
** Add RT63365 pcm driver
**
** Revision 1.1.1.1  2010/09/30 21:14:56  josephxu
** modules/public, private
**
** Revision 1.2  2010/06/17 11:41:50  jrchen_hc
** Add TC3182 PCM and SLIC support for VoIP
**
** Revision 1.1  2010/04/09 11:05:00  feiyan
** tclinux_phoenix new Trunk
** Add all voip driver in one folder
**
** Revision 1.1.1.1  2009/12/17 01:47:52  josephxu
** 20091217, from Hinchu ,with VoIP
**
 */
#ifndef _TC3262SLIC_H
#define _TC3262SLIC_H

#include <linux/autoconf.h>

#if defined (CONFIG_ARCH_MT7623) || defined (CONFIG_ARCH_MT7622)
#include <asm/rt2880/rt_mmap.h>
#else
#include <asm/mach-ralink/rt_mmap.h>
#endif				
/*********************************
 * Return Values *
 *********************************/
#define S_OK		0
#define S_ERROR		-1

/*********************************
 * Reset Values *
 *********************************/
#ifndef PCMDRIVER_H
#define RESET		0
#define ENABLE		1
#endif

/*********************************
 * Register Access Prefix *
 *********************************/
#define VPint			*(volatile unsigned int *)

/*********************************
 * Related GPIO Module Registers *
 *********************************/
#define CR_GPIO_BASE       	(RALINK_PIO_BASE)
#define CR_GPIO_CTRL	    (CR_GPIO_BASE + 0x00)
#define CR_GPIO_DATA	    (CR_GPIO_BASE + 0x04)
#define CR_GPIO_ODRAIN      (CR_GPIO_BASE + 0x14)
#define CR_GPIO_CTRL1		(CR_GPIO_BASE + 0x20)

/*********************************
 * Related GPIO Number *
 *********************************/
// 3262 ASIC
//#ifdef TC3262_PCM
//#define SLIC1_GPIO       	10
//#elif TC3182_PCM
//#define SLIC1_GPIO       	13
//#else
//#define SLIC1_GPIO       	15
//#endif
//3262 FPGA: 
//#define SLIC1_GPIO       	14
//#define SLIC2_GPIO		    15
//ASIC:
#if defined (CONFIG_RALINK_MT7628)
#define SLIC1_GPIO       	6
#endif

#if defined (CONFIG_RALINK_MT7621)
#define SLIC1_GPIO          6
#endif

#define SLIC2_GPIO		    30//2	//30
#define SLIC_SILICON		1


//#define TC3262_PCM			1
/*********************************
 * Related SPI Module Registers *
 *********************************/
#define CR_SPI_BASE     	(RALINK_SPI_BASE)
#define SPI_FLASH_CTL       0x00
#define SPI_FLASH_OPCODE    0x04
#define SPI_FLASH_DATA      0x08
#define SPI_FLASH_MM        0x28
#define SPI_FLASH_MBC		0x2c

#define SPI_CTL_START       0x00000100
#define SPI_CTL_BUSY        0x00010000


#endif /* _TC3262GMAC_H */
