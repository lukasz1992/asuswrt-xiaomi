
#ifndef EIP93_LEVEL0_H
#define EIP93_LEVEL0_H

#include "mtk_baseDefs.h"             // BIT definitions, bool, uint32_t
#include "mtk_hwAccess.h"              // Read32, Write32, HWPAL_Device_t
#include "mtk_hwDmaAccess.h"          // Read32, Write32, HWPAL_DMAResource_t
#include "mtk_arm.h"            // the API we will implement
#include "mtk_hwInterface.h"   // the HW interface (register map)

/*-----------------------------------------------------------------------------
 * EIP93_Read32
 *
 * This routine writes to a Register  location in the EIP93.
 */
static inline uint32_t
EIP93_Read32(
        HWPAL_Device_t Device,
        const unsigned int Offset)
{
    return HWPAL_Device_Read32(Device, Offset);
}


/*-----------------------------------------------------------------------------
 * EIP93_Write32
 *
 * This routine writes to a Register location in the EIP93.
 */
static inline void
EIP93_Write32(
        HWPAL_Device_t Device,
        const unsigned int  Offset,
        const uint32_t Value)
{
    HWPAL_Device_Write32(Device, Offset, Value);
}


/*----------------------------------------------------------------------------
 * EIP93_Read32Array
 *
 * This routine reads from a array of Register/OUT_RAM  memory locations in
 * the EIP93.
 */
static inline void
EIP93_Read32Array(
        HWPAL_Device_t Device,
        unsigned int Offset,            // read starts here, +4 increments
        uint32_t * MemoryDst_p,         // writing starts here
        const int Count)               // number of uint32's to transfer
{
    HWPAL_Device_Read32Array(Device, Offset, MemoryDst_p, Count);
}


/*----------------------------------------------------------------------------
 * EIP93_Write32Array
 *
 * This routine writes to a array of Register/IN_RAM memory locations in
 * the EIP93.
 */
static inline void
EIP93_Write32Array(
        HWPAL_Device_t Device,
        unsigned int Offset,            // write starts here, +4 increments
        uint32_t * MemorySrc_p,         // writing starts here
        const int Count)               // number of uint32's to transfer
{
    HWPAL_Device_Write32Array(Device, Offset, MemorySrc_p, Count);
}


/*-----------------------------------------------------------------------------
 * EIP93 register routines
 *
 * These routines write/read register values in EIP93 registers
 * in HW specific format.
 *
 * Note: if a function argument implies a flag ('f' is a prefix),
 *       then only the values 0 or 1 are allowed for this argument.
 */
static inline void
EIP93_Write32_PE_CFG(
        HWPAL_Device_t Device,
        const uint8_t fRstPacketEngine,
        const uint8_t fResetRing,
        const uint8_t PE_Mode,
        const uint8_t fBO_PD_en,
        const uint8_t fBO_SA_en,
        const uint8_t fBO_Data_en,
        const uint8_t fBO_TD_en,
    const uint8_t fEnablePDRUpdate)
{
    EIP93_Write32(
            Device,
            EIP93_REG_PE_CONFIG,
            (fRstPacketEngine & 1) |
            ((fResetRing & 1) << 1) |
            ((PE_Mode &(BIT_2-1)) << 8) |
            ((fBO_PD_en & 1) << 16) |
            ((fBO_SA_en & 1) << 17) |
            ((fBO_Data_en  & 1) << 18) |
            ((fBO_TD_en & 1) << 20) |
            ((fEnablePDRUpdate & 1) << 10));
}


static inline void
EIP93_Read32_PE_CFG(
        HWPAL_Device_t Device,
        uint8_t * const fBO_PD_en,
        uint8_t * const fBO_SA_en,
        uint8_t * const fBO_Data_en,
        uint8_t * const fBO_SGPD_en,
        uint8_t * const fBO_TD_en)
{
    uint32_t word = EIP93_Read32(Device, EIP93_REG_PE_CONFIG);
    if(fBO_PD_en)
        *fBO_PD_en = (word >> 16) & 1;
    if(fBO_SA_en)
        *fBO_SA_en = (word >> 17) & 1;
    if(fBO_Data_en)
        *fBO_Data_en = (word >> 18) & 1;
    if(fBO_SGPD_en)
        *fBO_SGPD_en = (word >> 19) & 1;
    if(fBO_TD_en)
        *fBO_TD_en = (word >> 21) & 1;
}


static inline void
EIP93_Write32_PE_IO_THRESHOLD(
        HWPAL_Device_t Device,
        const uint16_t InputThreshold,
        const uint16_t OutputThreshold)
{
    EIP93_Write32(
            Device,
           EIP93_REG_PE_BUF_THRESH,
            (InputThreshold & (BIT_10-1)) |
        ((OutputThreshold & (BIT_10-1)) << 16));
}


static inline void
EIP93_Read32_PE_IO_THRESHOLD(
        HWPAL_Device_t Device,
        uint16_t * const InputThreshold,
        uint16_t * const OutputThreshold)
{
    uint32_t word = EIP93_Read32(Device, EIP93_REG_PE_BUF_THRESH);
    if(InputThreshold)
        *InputThreshold = word & (BIT_10-1);
    if(OutputThreshold)
        *OutputThreshold = (word >> 16) & (BIT_10-1);
}




static inline void
EIP93_Write32_INT_CFG(
        HWPAL_Device_t Device,
        const uint8_t fIntHostOutputType,
        const uint8_t fIntPulseClear)
{
    EIP93_Write32(Device, EIP93_REG_INT_CFG,
                  (fIntHostOutputType & 1 ) |
                  ((fIntPulseClear << 1) & 1));
}

static inline void
EIP93_Read32_INT_CFG(
        HWPAL_Device_t Device,
        uint8_t * const fIntHostOutputType,
        uint8_t * const fIntPulseClear)
{
    uint32_t word = EIP93_Read32(Device, EIP93_REG_INT_CFG);
    if(fIntHostOutputType)
        *fIntHostOutputType = word & 1;
    if(fIntPulseClear)
        *fIntPulseClear = (word >> 1) & 1;
}


static inline void
EIP93_Write32_MST_BYTE_ORDER_CFG(
        HWPAL_Device_t Device,
        const uint8_t ByteOrderPD,
        const uint8_t ByteOrderSA,
        const uint8_t ByteOrderData,
        const uint8_t ByteOrderTD)
{
    EIP93_Write32(
            Device,
            EIP93_REG_PE_ENDIAN_CONFIG,
            (ByteOrderPD & (BIT_4-1)) |
            ((ByteOrderSA & (BIT_4-1)) << 4) |
            ((ByteOrderData & (BIT_4-1)) << 8) |
            ((ByteOrderTD & (BIT_2-1)) << 16));
}

static inline void
EIP93_Read32_MST_BYTE_ORDER_CFG(
        HWPAL_Device_t Device,
        uint8_t * const ByteOrderPD,
        uint8_t * const ByteOrderSA,
        uint8_t * const ByteOrderData,
        uint8_t * const ByteOrderTD)
{
    uint32_t word = EIP93_Read32(Device, EIP93_REG_PE_ENDIAN_CONFIG);
    if(ByteOrderPD)
        *ByteOrderPD = word & (BIT_4-1);
    if(ByteOrderSA)
        *ByteOrderSA = (word >> 4) & (BIT_4-1);
    if(ByteOrderData)
        *ByteOrderData = (word >> 8) & (BIT_4-1);
    if(ByteOrderTD)
        *ByteOrderTD = (word >> 16) & (BIT_2-1);
}

static inline void
EIP93_Read32_PE_OPTIONS_1(
        HWPAL_Device_t Device,
        uint8_t * const fDesTdes,
        uint8_t * const fARC4,
        uint8_t * const fAES,
        uint8_t * const fAES128,
    uint8_t * const fAES192,
    uint8_t * const fAES256,
        uint8_t * const fKasumiF8,
        uint8_t * const fDesOfgCfg,
        uint8_t * const fAesCfg,
        uint8_t * const fMD5,
        uint8_t * const fSHA1,
        uint8_t * const fSHA224,
        uint8_t * const fSHA256,
        uint8_t * const fSHA384,
        uint8_t * const fSHA512,
        uint8_t * const fKasumiF9,
        uint8_t * const fAesXcbc,
        uint8_t * const fGCM,
        uint8_t * const fGMAC,
        uint8_t * const fAesCbcMac,
    uint8_t * const fAesCbcMac128,
    uint8_t * const fAesCbcMac192,
    uint8_t * const fAesCbcMac256)

{
    uint32_t word = EIP93_Read32(Device,EIP93_REG_PE_OPTION_1);
    if(fDesTdes)
        *fDesTdes = word & 1;
    if(fARC4)
        *fARC4 = (word >> 1) & 1;
    if(fAES)
        *fAES = (word >> 2) & 1;
    if(fAES128)
        *fAES128 = (word >> 13) & 1;
    if(fAES192)
        *fAES192 = (word >> 14) & 1;
    if(fAES256)
        *fAES256 = (word >> 15) & 1;
    if(fKasumiF8)
        *fKasumiF8 = 0;
    if(fDesOfgCfg)
        *fDesOfgCfg = 0;
    if(fAesCfg)
        *fAesCfg = 0;
    if(fMD5)
        *fMD5 = (word >> 16) & 1;
    if(fSHA1)
        *fSHA1 = (word >> 17) & 1;
    if(fSHA224)
        *fSHA224 = (word >> 18) & 1;
    if(fSHA256)
        *fSHA256 = (word >> 19) & 1;
    if(fSHA384)
        *fSHA384 = 0;
    if(fSHA512)
        *fSHA512 = 0;
    if(fKasumiF9)
        *fKasumiF9 = 0;
    if(fAesXcbc)
        *fAesXcbc = (word >> 23) & 1;
    if(fGCM)
        *fGCM = 0;
    if(fGMAC)
        *fGMAC = 0;
    if(fAesCbcMac)
        *fAesCbcMac = (word >> 28) & 1;
    if(fAesCbcMac128)
        *fAesCbcMac128 = (word >> 29) & 1;
    if(fAesCbcMac192)
        *fAesCbcMac192 = (word >> 30) & 1;
    if(fAesCbcMac256)
        *fAesCbcMac256 = (word >> 31) & 1;

}

static inline void
EIP93_Read32_PE_OPTIONS_0(
        HWPAL_Device_t Device,
        uint8_t * const fInterfaceType,
        uint8_t * const f64BitAdrIndicator,
        uint8_t * const fExtInterupt,
        uint8_t * const fPRNG,
        uint8_t * const fSARev1,
        uint8_t * const fSARev2,
        uint8_t * const fDynamicSA,
        uint8_t * const fESN,
        uint8_t * const fESP,
        uint8_t * const fAH,
        uint8_t * const fSSL,
        uint8_t * const fTLS,
        uint8_t * const fDTLS,
        uint8_t * const fSRTP,
        uint8_t * const fMacSec)
{
    uint32_t word = EIP93_Read32(Device, EIP93_REG_PE_OPTION_0);
    if(fInterfaceType)
        *fInterfaceType = word & (BIT_3-1);
    if(f64BitAdrIndicator)
        *f64BitAdrIndicator = 0;
    if(fExtInterupt)
        *fExtInterupt = 0;
    if(fPRNG)
        *fPRNG = (word >> 6) & 1;
    if(fSARev1)
        *fSARev1 =  1;
    if(fSARev2)
        *fSARev2 = 0;
    if(fDynamicSA)
        *fDynamicSA = 0;
    if(fESN)
        *fESN = (word >> 15) & 1;
    if(fESP)
        *fESP = (word >> 16) & 1;
    if(fAH)
        *fAH = (word >> 17) & 1;
    if(fSSL)
        *fSSL = (word >> 20) & 1;
    if(fTLS)
        *fTLS = (word >> 21) & 1;
    if(fDTLS)
        *fDTLS = (word >> 22) & 1;
    if(fSRTP)
        *fSRTP = (word >> 24) & 1;
    if(fMacSec)
        *fMacSec = (word >> 25) & 1;
}

static inline void
EIP93_Read32_REVISION_REG(
        HWPAL_Device_t Device,
        uint8_t * const EIPNumber,
        uint8_t * const ComplEIPNumber,
        uint8_t * const HWPatchLevel,
        uint8_t * const MinHWRevision,
        uint8_t * const MajHWRevision)
{
    uint32_t word = EIP93_Read32(Device, EIP93_REG_PE_REVISION);
    if(EIPNumber)
        *EIPNumber = word & (BIT_8-1);
    if(ComplEIPNumber)
        *ComplEIPNumber = (word >> 8) & (BIT_8-1);
    if(HWPatchLevel)
        *HWPatchLevel = (word >> 16) & (BIT_4-1);
    if(MinHWRevision)
        *MinHWRevision = (word >> 20) & (BIT_4-1);
    if(MajHWRevision)
        *MajHWRevision = (word >> 24) & (BIT_4-1);
}

/*-----------------------------------------------------------------------------
 * PRNG_SEED (0-3) - Write Only
 */
static inline void
EIP93_Write32_PRNG_SEED_0(
        HWPAL_Device_t Device,
        const uint32_t SecretSeed)
{
    EIP93_Write32(Device,  EIP93_REG_PRNG_SEED_0, SecretSeed);
}


static inline void
EIP93_Write32_PRNG_SEED_1(
        HWPAL_Device_t Device,
        const uint32_t SecretSeed)
{
    EIP93_Write32(Device,  EIP93_REG_PRNG_SEED_1, SecretSeed);
}
static inline void
EIP93_Write32_PRNG_SEED_2(
        HWPAL_Device_t Device,
        const uint32_t SecretSeed)
{
    EIP93_Write32(Device, EIP93_REG_PRNG_SEED_2, SecretSeed);
}


static inline void
EIP93_Write32_PRNG_SEED_3(
        HWPAL_Device_t Device,
        const uint32_t SecretSeed)
{
    EIP93_Write32(Device,  EIP93_REG_PRNG_SEED_3, SecretSeed);
}


/*-----------------------------------------------------------------------------
 * PRNG_KEY (0-3) - Write Only
 */
static inline void
EIP93_Write32_PRNG_KEY_0(
        HWPAL_Device_t Device,
        const uint32_t DESKey0)
{
    EIP93_Write32(Device,EIP93_REG_PRNG_KEY_0, DESKey0);
}


static inline void
EIP93_Write32_PRNG_KEY_1(
        HWPAL_Device_t Device,
        const uint32_t DESKey1)
{
    EIP93_Write32(Device, EIP93_REG_PRNG_KEY_1, DESKey1);
}

static inline void
EIP93_Write32_PRNG_KEY_2(
        HWPAL_Device_t Device,
        const uint32_t DESKey2)
{
    EIP93_Write32(Device,EIP93_REG_PRNG_KEY_2, DESKey2);
}


static inline void
EIP93_Write32_PRNG_KEY_3(
        HWPAL_Device_t Device,
        const uint32_t DESKey3)
{
    EIP93_Write32(Device, EIP93_REG_PRNG_KEY_3, DESKey3);
}



/*-----------------------------------------------------------------------------
 * PRNG_LFSR (0-1) - Read/Write
 */
static inline void
EIP93_Write32_PRNG_LFSR_0(
        HWPAL_Device_t Device,
        const uint32_t LFSRValue)
{
    EIP93_Write32(Device, EIP93_REG_PRNG_LFSR_0, LFSRValue);
}

static inline void
EIP93_Read32_PRNG_LFSR_0(
        HWPAL_Device_t Device,
        uint32_t * const LFSRValue)
{
    if(LFSRValue)
        *LFSRValue = EIP93_Read32(Device, EIP93_REG_PRNG_LFSR_0);
}

static inline void
EIP93_Write32_PRNG_LFSR_1(
        HWPAL_Device_t Device,
        const uint32_t LFSRValue)
{
    EIP93_Write32(Device, EIP93_REG_PRNG_LFSR_1, LFSRValue);
}

static inline void
EIP93_Read32_PRNG_LFSR_1(
        HWPAL_Device_t Device,
        uint32_t * const LFSRValue)
{
    if(LFSRValue)
        *LFSRValue = EIP93_Read32(Device, EIP93_REG_PRNG_LFSR_1);
}

#endif 


