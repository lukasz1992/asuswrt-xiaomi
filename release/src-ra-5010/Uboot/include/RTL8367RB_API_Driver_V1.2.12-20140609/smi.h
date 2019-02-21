
#ifndef __SMI_H__
#define __SMI_H__

#include <rtk_types.h>
#include "rtk_error.h"

rtk_int32 smi_reset(rtk_uint32 port, rtk_uint32 pinRST);
typedef int gpioID;

rtk_int32 smi_init(rtk_uint32 port, rtk_uint32 pinSCK, rtk_uint32 pinSDA);
rtk_int32 smi_read(rtk_uint32 mAddrs, rtk_uint32 *rData);
rtk_int32 smi_write(rtk_uint32 mAddrs, rtk_uint32 rData);

#endif /* __SMI_H__ */


