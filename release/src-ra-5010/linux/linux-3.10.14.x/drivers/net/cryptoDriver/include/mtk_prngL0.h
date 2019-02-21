
#ifndef EIP93_PRNG_L0_H
#define EIP93_PRNG_L0_H

#include "mtk_baseDefs.h"             // BIT definitions, bool, uint32_t
#include "mtk_hwAccess.h"              // Read32, Write32, HWPAL_Device_t
#include "mtk_hwDmaAccess.h"          // Read32, Write32, HWPAL_DMAResource_t
#include "mtk_hwInterface.h"   // the HW interface (register map)



/*-----------------------------------------------------------------------------
 * PRNG register routines
 *
 * These routines write/read register values in PRNG register
 * in HW specific format.
 *
 * Note: if a function argument implies a flag ('f' is a prefix),
 *       then only the values 0 or 1 are allowed for this argument.
 */

static inline void
EIP93_Read32_PRNG_STATUS(
        HWPAL_Device_t Device,
        uint8_t * const fBusy,
        uint8_t * const fResultReady)
{
    uint32_t word = EIP93_Read32(Device, EIP93_REG_PRNG_STAT);
    if(fBusy)
        *fBusy = (word) & 1;
    if(fResultReady)
        *fResultReady = (word >> 1) & 1;
}

/*-----------------------------------------------------------------------------
 * PRNG_CTRL - Read/Write
 */
static inline void
EIP93_Write32_PRNG_CTRL(
        HWPAL_Device_t Device,
        const uint8_t fEnableManual,
        const uint8_t fAuto,
        const uint8_t fResult128)
{
    EIP93_Write32(Device, EIP93_REG_PRNG_CTRL,
            ((fEnableManual & 1) << 0) |
            ((fAuto & 1) << 1) |
            ((fResult128 & 1) << 2));
}


static inline void
EIP93_Read32_PRNG_CTRL(
        HWPAL_Device_t Device,
        uint8_t * const fEnableManual,
        uint8_t * const fAuto,
        uint8_t * const fResult128)
{
    uint32_t word = EIP93_Read32(Device, EIP93_REG_PRNG_CTRL);
    if(fEnableManual)
        *fEnableManual = word & BIT_0;
    if(fAuto)
        *fAuto = (word >> 1) & 1;
    if(fResult128)
        *fResult128 = (word >> 2) & 1;
}


#endif 


