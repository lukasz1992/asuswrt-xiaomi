/*
** Copyright (c) 2007-2010 by Silicon Laboratories
**
** $Id: si_voice_datatypes.h 3411 2012-04-04 23:11:51Z nizajerk $
**
** si_voice_datatypes.h
** ProSLIC datatypes file
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
** This is the header file that contains
** type definitions for the data types
** used in the demonstration code.
**
** Dependancies:
** 
**
*/
#ifndef DATATYPES_H
#define DATATYPES_H

#ifndef TRUE
#define TRUE (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

#ifndef NULL
#define NULL ((void *) 0)
#endif

#if defined(__linux__) && defined(__KERNEL__)
#include <linux/types.h>
typedef u_int8_t           BOOLEAN;
typedef int8_t			    int8;
typedef u_int8_t            uInt8;
typedef uInt8				uChar;
typedef int16_t  			int16;
typedef u_int16_t           uInt16;
typedef int32_t             int32;
typedef u_int32_t   		uInt32;
#elif defined(WIN32)
#include <wtypes.h>
typedef char				int8;
typedef unsigned char		uInt8;
typedef uInt8				uChar;
typedef short int			int16;
typedef unsigned short int	uInt16;
typedef long				int32;
typedef unsigned long		uInt32;
#else
#include <stdint.h>
typedef uint8_t           BOOLEAN;
typedef int8_t			    int8;
typedef uint8_t            uInt8;
typedef uint8_t				uChar;
typedef int16_t  			int16;
typedef uint16_t           uInt16;
typedef int32_t             int32;
typedef uint32_t   		uInt32;
#endif
/*
** RAM data 
*/

#define ramData uInt32

#endif
