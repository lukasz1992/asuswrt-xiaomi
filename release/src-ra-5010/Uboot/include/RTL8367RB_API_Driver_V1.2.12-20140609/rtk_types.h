#ifndef _RTL8367B_TYPES_H_
#define _RTL8367B_TYPES_H_

#ifdef EMBEDDED_SUPPORT

#include <common_types.h>

#else

typedef unsigned long long		rtk_uint64;
typedef long long				rtk_int64;
typedef unsigned int			rtk_uint32;
typedef int						rtk_int32;
typedef unsigned short			rtk_uint16;
typedef short					rtk_int16;
typedef unsigned char			rtk_uint8;
typedef char					rtk_int8;

#define CONST_T     const

#endif

#ifndef _RTL_TYPES_H

#if 0
typedef unsigned long long		uint64;
typedef long long				int64;
typedef unsigned int			uint32;
typedef int						int32;
typedef unsigned short			uint16;
typedef short					int16;
typedef unsigned char			uint8;
typedef char					int8;
#endif

typedef rtk_uint32          		ipaddr_t;
typedef rtk_uint32					memaddr;

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN		6
#endif

typedef struct ether_addr_s {
	rtk_uint8 octet[ETHER_ADDR_LEN];
} ether_addr_t;

#ifdef __KERNEL__
#define rtlglue_printf printk
#else
#define rtlglue_printf printf
#endif
#define PRINT			rtlglue_printf
#endif /*_RTL_TYPES_H*/

/* type abstraction */
#ifdef EMBEDDED_SUPPORT

typedef rtk_int16                   rtk_api_ret_t;
typedef rtk_int16                   ret_t;
typedef rtk_uint32                  rtk_u_long;

#else

typedef rtk_int32                   rtk_api_ret_t;
typedef rtk_int32                   ret_t;
typedef rtk_uint64                  rtk_u_long_t;

#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define CONST			const
#endif /* _RTL8367B_TYPES_H_ */
