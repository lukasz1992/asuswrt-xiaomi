/*
 * Copyright (C) 2001 Palmchip Corporation.  All rights reserved.
 *
 * ########################################################################
 *
 *  This program is free software; you can distribute it and/or modify it
 *  under the terms of the GNU General Public License (Version 2) as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 * ########################################################################
 *
 */
#ifndef _SURFBOARD_H
#define _SURFBOARD_H

#include <asm/addrspace.h>



/*
 * Surfboard system clock.
 * This is the default value and maybe overidden by System Clock passed on the
 * command line (sysclk=).
 */
#define SURFBOARD_SYSTEM_CLOCK		(125000000)

/*
 * Surfboard UART base baud rate = System Clock / 16.
 * Ex. (14.7456 MHZ / 16) = 921600
 *     (32.0000 MHZ / 16) = 2000000
 */
#define SURFBOARD_BAUD_DIV	(16)
#define SURFBOARD_BASE_BAUD	(SURFBOARD_SYSTEM_CLOCK / SURFBOARD_BAUD_DIV)

/*
 * Maximum number of IDE Controllers
 * Surfboard only has one ide (ide0), so only 2 drives are
 * possible.  (no need to check for more hwifs.)
 */
//#define MAX_IDE_HWIFS		(1)	/* Surfboard/Wakeboard */
#define MAX_IDE_HWIFS		(2)	/* Graphite board */

#define GCMP_BASE_ADDR                  0x1fbf8000
#define GCMP_ADDRSPACE_SZ               (256 * 1024)

/*
 *  * GIC Specific definitions
 *   */
#define GIC_BASE_ADDR                   0x1fbc0000
#define GIC_ADDRSPACE_SZ                (128 * 1024)
#define MIPS_GIC_IRQ_BASE		(MIPS_CPU_IRQ_BASE)

/* GIC's Nomenclature for Core Interrupt Pins */
#define GIC_CPU_INT0            0 /* Core Interrupt 2   */
#define GIC_CPU_INT1            1 /* .                  */
#define GIC_CPU_INT2            2 /* .                  */
#define GIC_CPU_INT3            3 /* .                  */
#define GIC_CPU_INT4            4 /* .                  */
#define GIC_CPU_INT5            5 /* Core Interrupt 5   */

#endif /* !(_SURFBOARD_H) */
