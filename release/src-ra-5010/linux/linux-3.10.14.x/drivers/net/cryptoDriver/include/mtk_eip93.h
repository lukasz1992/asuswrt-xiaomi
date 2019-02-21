
#ifndef EIP93_EIP93_H
#define EIP93_EIP93_H

#include "mtk_baseDefs.h"
#include "mtk_hwAccess.h"

/*----------------------------------------------------------------------------
 * Physical (bus) address used by EIP93 device
 */
typedef struct
{
    // 32bit physical bus address
    uint32_t Addr;

    // upper 32bit part of a 64bit physical address
    // Note: this value has to be provided only for 64bit addresses,
    // in this case Addr field provides the lower 32bit part
    // of the 64bit address, for 32bit addresses this field is ignored,
    // and should be set to 0.
    uint32_t UpperAddr;
} EIP93_DeviceAddress_t;


/*----------------------------------------------------------------------------
 * ERROR codes returned by the API
 * EIP93_STATUS_OK : successful completion of the call.
 * EIP93_ERROR_BAD_ARGUMENT :  invalid argument for  function parameter.
 * EIP93_ERROR_BUSY_RETRY_LATER : Device is busy.
 * EIP93_ERROR_UNSUPPORTED_IN_THIS_STATE : a function cannot be called
 * EIP93_ERROR_NOT_IMPLEMENTED: functionality not implemented yet in this
 *                              state.
 * EIP93_ERROR_TOO_MUCH_DATA: trying to write more data than provided.
 *                            command descriptor length filed. Valid in
 *                            DHM only.
 * EIP93_ERROR_NO_MORE_DATA: trying read when there is no data  or all have
 *                           been.Valid in DHM only.
 * EIP93_ERROR_MEMORY_INSUFFICIENT: when provided buffer no enough to all data.
 *                                  Valid in DHM only.
 */

typedef enum
{
    EIP93_STATUS_OK = 0,
    EIP93_ERROR_BAD_ARGUMENT,
    EIP93_ERROR_BUSY_RETRY_LATER,
    EIP93_ERROR_UNSUPPORTED_IN_THIS_STATE,
    EIP93_ERROR_NOT_IMPLEMENTED,
    EIP93_ERROR_TOO_MUCH_DATA,
    EIP93_ERROR_NO_MORE_DATA,
    EIP93_ERROR_MEMORY_INSUFFICIENT
} EIP93_Status_t;


/*----------------------------------------------------------------------------
 * Capabilities Structure for EIP93
 */
typedef struct
{
    // DES and Triple DES encryption standard indicator.
    uint8_t fDesTdes;
    // ARC4 encryption standard indicator.
    uint8_t fARC4;
    // AES encryption standard indicator.
    uint8_t fAes;
    //AES with 128 bit key indicator.
    uint8_t fAes128;
    //AES with 192 bit key indicator.
    uint8_t fAes192;
    //AES with 256 bit key indicator.
    uint8_t fAes256;
    // Kasumi f8 encryption standard indicator.
    uint8_t fKasumiF8;
    // DES OFB1-8-64 and CFB1-8-64 mode indicator.
    uint8_t fDesOfgCfb;
    // AES CFB1-8-128 mode indicator.
    uint8_t fAesCfb;
    // MD5 hash algorithm indicator.
    uint8_t fMd5;
    // SHA-1 hash algorithm indicator.
    uint8_t fSha1;
    // SHA-2 224 hash algorithm indicator.
    uint8_t fSha224;
    // SHA-2 256 hash algorithm indicator.
    uint8_t fSha256;
    // SHA-2 384 hash algorithm indicator.
    uint8_t fSha384;
    // SHA-2 512 hash algorithm indicator.
    uint8_t fSha512;
    // Kasumi f9 encryption standard indicator.
    uint8_t fKasumiF9;
    // 128-Bit AES-XCBC-MAC and AES-CCM mode indicator.
    uint8_t fAesXcbc;
    // Galois HASH and AES Galois Counter Mode (GCM) indicator
    uint8_t fGcm;
    // AES-Galois Message Authentication Code (GMAC) indicator.
    uint8_t fGmac;
    // AES CBC MAC indicator
    uint8_t fAesCbcMac;
    // AES CBC MAC with 128-bit key indicator
    uint8_t fAesCbcMac128;
    // AES CBC MAC with 192-bit key indicator
    uint8_t fAesCbcMac192;
    // AES CBC MAC with 256-bit key indicator
    uint8_t fAesCbcMac256;
    // Type of external interface.
    uint8_t IntFaceType;
    // 64-Bit Address indicator.
    uint8_t f64BitAddress;
    // Interrupt controller indicator.
    uint8_t fExtInterrupt;
    // Pseudo Random Number Generator indicator.
    uint8_t fPrng;
    // SA Revision 1 indicator.
    uint8_t fSARev1;
    // SA Revision 2 indicator.
    uint8_t fSARev2;
    // Dynamic SA indicator.
    uint8_t fDynamicSA;
    // extended sequence numner
    uint8_t fEsn;
    // ESP for IPv4 IPSec and IPv6 IPSec indicator.
    uint8_t fEsp;
    // AH for IPv4 and IPv6 IPSec indicator.
    uint8_t fAh;
    // Security Socket Layer (SSL)
    uint8_t fSsl;
    // Transport Layer Security (TLS) v1.0 or 1.1 indicator.
    uint8_t fTls;
    // Datagram Transport Layer Security (DTLS) 1.0
    uint8_t fDtls;
    // Secures Real-Time Protocol (SRTP)
    uint8_t fSrtp;
    // Media Access Control Security (MACSec)
    uint8_t fMacsec;
    // The basic EIP number.
    uint8_t EipNumber;
    // The complement of the basic EIP number.
    uint8_t ComplmtEipNumber;
    // Hardware Patch Level.
    uint8_t HWPatchLevel;
    // Minor Hardware revision.
    uint8_t MinHWRevision;
    // Major Hardware revision.
    uint8_t MajHWRevision;
} EIP93_Capabilities_t;


#define EIP93_IOAREA_REQUIRED_SIZE  32
typedef struct
{
    uint32_t placeholder[EIP93_IOAREA_REQUIRED_SIZE];
} EIP93_IOArea_t;


/*----------------------------------------------------------------------------
 *                         Common API
 *----------------------------------------------------------------------------
 */
/*----------------------------------------------------------------------------
 * EIP93_HWRevision_Get
 *
 * This function returns hardware revision information in three components.
 * Each can have a value in the range 0-9.
 *
 */
EIP93_Status_t
EIP93_HWRevision_Get(
        EIP93_IOArea_t * const  IOArea_p,
        EIP93_Capabilities_t * const Capabilities_p);


/*----------------------------------------------------------------------------
 * EIP93_Initialize
 *
 * Intialize the hardware platform for EIP93 register access.
 *
 * This function is only allowed to be called from the 'reset' state.
 * This function puts the device into the 'initialized' state.
 *
 * IOArea_p - the EIP93 Driver Library API external storage context data.
 * It has to be allocated by the user.
 *
 * (input)Device - a static resource handle obtained with HWPAL_Device_Find().
 */
EIP93_Status_t
EIP93_Initialize(
        EIP93_IOArea_t * const IOArea_p,
        const HWPAL_Device_t Device);



/*----------------------------------------------------------------------------
 * EIP93_Deactivate
 *
 * This function is only allowed to be called from
 * 'ARM activated', or 'DHM activated' states, to which
 * the device can be put by corresponding _Activate functions.
 * This function puts the device back to the 'initialized' state.
 */
EIP93_Status_t
EIP93_Deactivate(
        EIP93_IOArea_t * const IOArea_p);


/*----------------------------------------------------------------------------
 * EIP93_Shutdown
 *
 * This function is allowed to be called from any state.
 * This function puts the device in the 'reset' state, i.e. stops the device.
 */
EIP93_Status_t
EIP93_Shutdown(
        EIP93_IOArea_t * const IOArea_p);


/*----------------------------------------------------------------------------
 *                         Common Interrupt Service
 *----------------------------------------------------------------------------
 */

/*----------------------------------------------------------------------------
 * Interrupts of EIP93
 */
typedef enum
{
    EIP93_INT_PE_CDRTHRESH_REQ =   BIT_0,
    EIP93_INT_PE_RDRTHRESH_REQ =   BIT_1,
    EIP93_INT_PE_OPERATION_DONE =  BIT_9,
    EIP93_INT_PE_INBUFTHRESH_REQ = BIT_10,
    EIP93_INT_PE_OUTBURTHRSH_REQ = BIT_11,
    EIP93_INT_PE_ERR_REG =         BIT_13,
    EIP93_INT_PE_RD_DONE_IRQ =     BIT_16

} EIP93_InterruptSource_t;

/*----------------------------------------------------------------------------
 * Bitmask for a set of interrupts of EIP93
 * This represents an 'OR'-ed combination of EIP93_InterruptSource_t values
 */
typedef uint32_t EIP93_INT_SourceBitmap_t;


/*----------------------------------------------------------------------------
 * EIP93_INT_Mask
 *
 * Masks  certain interrupt lines.
 *
 * WhichIntSources
 *      Specifies which interrupt sources to mask:
 *      '0' - means an interrupt source is left as is.
 *      '1' - means an interrupt source is masked (disabled)
 */
EIP93_Status_t
EIP93_INT_Mask(
        EIP93_IOArea_t * const IOArea_p,
        const EIP93_INT_SourceBitmap_t WhichIntSources);


/*----------------------------------------------------------------------------
 * EIP93_INT_UnMask
 *
 * Unmasks certain interrupt lines.
 *
 * WhichIntSources
 *      Specifies which interrupt sources to Unmask:
 *      '0' - means an interrupt source is left as is.
 *      '1' - means an interrupt source is Unmasked (enabled)
 */
EIP93_Status_t
EIP93_INT_UnMask(
        EIP93_IOArea_t * const IOArea_p,
        const EIP93_INT_SourceBitmap_t WhichIntSources);




/*----------------------------------------------------------------------------
 * EIP93_INT_IsRawActive
 *
 * Reads the raw status of all interrupt sources.
 * Raw means the status is provided before an interrupt mask is applied.
 *
 * PendingIntSources_p
 *      is the output parameter that on return contains the following
 *      status for each interrupt source:
 *      '0' - interrupt is not pending (inactive, not present)
 *      '1' - interrupt is pending (active, present)
 */
EIP93_Status_t
EIP93_INT_IsRawActive(
        EIP93_IOArea_t * const IOArea_p,
        EIP93_INT_SourceBitmap_t * const PendingIntSources_p);


/*----------------------------------------------------------------------------
 * EIP93_INT_IsActive
 *
 * Reads the status of all interrupt sources.
 * In this case the status is provided after an interrupt mask is applied.
 * So this means that although a raw interrupt might be pending, but it is
 * masked (disabled), this function will indicate that the interrupt is
 * not pending (inactive).
 *
 * PendingIntSources_p
 *      is the output parameter that on return contains the following
 *      status for each interrupt source:
 *      '0' - interrupt is not pending (inactive, not present)
 *      '1' - interrupt is pending (active, present)
 */
EIP93_Status_t
EIP93_INT_IsActive(
        EIP93_IOArea_t * const IOArea_p,
        EIP93_INT_SourceBitmap_t * const PendingIntSources_p);


/*----------------------------------------------------------------------------
 * EIP93_INT_Acknowledge
 *
 * Acknowledges certain interrupt lines.
 *
 * WhichIntSources
 *      Specifies which interrupt sources to acknowledge (to clear):
 *      '0' - means an interrupt source is not acknowledged
 *           (status is unchanged)
 *      '1' - means an interrupt source is acknowledged (status is cleared)
 */
EIP93_Status_t
EIP93_INT_Acknowledge(
        EIP93_IOArea_t * const IOArea_p,
        const EIP93_INT_SourceBitmap_t WhichIntSources);

/*----------------------------------------------------------------------------
 * EIP93_INT_Configure
 *
 * sets the type- pulse or level, for the interrupt output;
 * enable/disable auto clearing functionality of the interrupts.
 *
 */
EIP93_Status_t
EIP93_INT_Configure(
        EIP93_IOArea_t * const IOArea_p,
        const bool fPulsedInt,
        const bool fAutoClear);


#endif 

