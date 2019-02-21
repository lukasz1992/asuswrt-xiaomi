#include "mtk_baseDefs.h"      // uint8_t, IDENTIFIER_NOT_USED, etc.
#include "mtk_cLib.h"                   // C Lib API
#include "mtk_hwAccess.h"              // HW access API
#include "mtk_eip93.h"                // the API we will implement
#include "mtk_cEip93.h"              // configration options
#include "mtk_L0.h"         // macros and functions to access EIP93 reg

#include "mtk_prngL0.h"    // macros and functions to access PRNG_CTRL
#include "mtk_internal.h"       // internal API
#include "mtk_descp.h"     // the Descriptor API we also implement

#ifdef EIP93_STRICT_ARGS

#define EIP93_INSERTCODE_FUNCTION_ENTRY_CODE_SL \
    EIP93_Status_t res = EIP93_STATUS_OK; \
    EIP93_Device_t* Device_p = NULL; \
    EIP93_ARM_Mode_t* ARM_p = NULL; \
    EIP93_DHM_Mode_t* DHM_p = NULL; \
    EIP93_CHECK_POINTER(IOArea_p); \
    Device_p = (EIP93_Device_t*)IOArea_p; \
    ARM_p = &Device_p->extras.ARM_mode; \
    DHM_p = &Device_p->extras.DHM_mode; \
    IDENTIFIER_NOT_USED(ARM_p);   \
    IDENTIFIER_NOT_USED(DHM_p);

#else

#define EIP93_INSERTCODE_FUNCTION_ENTRY_CODE_SL \
    EIP93_Status_t res = EIP93_STATUS_OK; \
    EIP93_Device_t* Device_p = (EIP93_Device_t*)IOArea_p; \
    EIP93_ARM_Mode_t* ARM_p = &Device_p->extras.ARM_mode; \
    EIP93_DHM_Mode_t* DHM_p = &Device_p->extras.DHM_mode; \
    IDENTIFIER_NOT_USED(ARM_p); \
    IDENTIFIER_NOT_USED(DHM_p);

#endif //EIP93_STRICT_ARGS

/* Function to Set the PRNG is auto mode */
static inline void
EIP93_Internal_PRNG_Activate(
        const HWPAL_Device_t Device,
        bool IsInit)
{
    if(IsInit == true)
    {
        EIP93_Write32_PRNG_SEED_0(Device, EIP93_SEED_0);
        EIP93_Write32_PRNG_SEED_1(Device, EIP93_SEED_1);
        EIP93_Write32_PRNG_SEED_2(Device, EIP93_SEED_2);
        EIP93_Write32_PRNG_SEED_3(Device, EIP93_SEED_3);

        EIP93_Write32_PRNG_KEY_0(Device, EIP93_KEY_0);
        EIP93_Write32_PRNG_KEY_1(Device, EIP93_KEY_1);
        EIP93_Write32_PRNG_KEY_2(Device, EIP93_KEY_2);
        EIP93_Write32_PRNG_KEY_3(Device, EIP93_KEY_3);

        EIP93_Write32_PRNG_LFSR_0(Device, EIP93_LFSR_0);
        EIP93_Write32_PRNG_LFSR_1(Device, EIP93_LFSR_1);
    }
    
    EIP93_Write32_PRNG_CTRL(Device,
            EIP93_PRNG_MANUAL_OFF,
            EIP93_PRNG_AUTO_ON,
            EIP93_PRNG_RESULT64);
}

/*----------------------------------------------------------------------------
 * EIP93_HWRevision_Get
 *
 *  See header file for function specification.
 */
EIP93_Status_t
EIP93_HWRevision_Get(
        EIP93_IOArea_t * const  IOArea_p,
        EIP93_Capabilities_t * const Capabilities_p)
{
    EIP93_INSERTCODE_FUNCTION_ENTRY_CODE_SL;
    EIP93_CHECK_POINTER(Capabilities_p);

    EIP93_Read32_PE_OPTIONS_1(Device_p->Device,
            &(Capabilities_p->fDesTdes),
            &(Capabilities_p->fARC4),
            &(Capabilities_p->fAes),
            &(Capabilities_p->fAes128),
            &(Capabilities_p->fAes192),
            &(Capabilities_p->fAes256),
            &(Capabilities_p->fKasumiF8),
            &(Capabilities_p->fDesOfgCfb),
            &(Capabilities_p->fAesCfb),
            &(Capabilities_p->fMd5),
            &(Capabilities_p->fSha1),
            &(Capabilities_p->fSha224),
            &(Capabilities_p->fSha256),
            &(Capabilities_p->fSha384),
            &(Capabilities_p->fSha512),
            &(Capabilities_p->fKasumiF9),
            &(Capabilities_p->fAesXcbc),
            &(Capabilities_p->fGcm),
            &(Capabilities_p->fGmac),
            &(Capabilities_p->fAesCbcMac),
            &(Capabilities_p->fAesCbcMac128),
            &(Capabilities_p->fAesCbcMac192),
            &(Capabilities_p->fAesCbcMac256));

    EIP93_Read32_PE_OPTIONS_0(Device_p->Device,
            &(Capabilities_p->IntFaceType),
            &(Capabilities_p->f64BitAddress),
            &(Capabilities_p->fExtInterrupt),
            &(Capabilities_p->fPrng),
            &(Capabilities_p->fSARev1),
            &(Capabilities_p->fSARev2),
            &(Capabilities_p->fDynamicSA),
            &(Capabilities_p->fEsn),
            &(Capabilities_p->fEsp),
            &(Capabilities_p->fAh),
            &(Capabilities_p->fSsl),
            &(Capabilities_p->fTls),
            &(Capabilities_p->fDtls),
            &(Capabilities_p->fSrtp),
            &(Capabilities_p->fMacsec));

    EIP93_Read32_REVISION_REG(Device_p->Device,
            &(Capabilities_p->EipNumber),
            &(Capabilities_p->ComplmtEipNumber),
            &(Capabilities_p->HWPatchLevel),
            &(Capabilities_p->MinHWRevision),
            &(Capabilities_p->MajHWRevision));

    EIP93_INSERTCODE_FUNCTION_EXIT_CODE;
}

/*----------------------------------------------------------------------------
 * EIP93_Initialize
 *
 *  See header file for function specification.
 */
EIP93_Status_t
EIP93_Initialize(
        EIP93_IOArea_t * const IOArea_p,
        const HWPAL_Device_t Device)
{
    uint8_t EipNumber = 0;
    EIP93_INSERTCODE_FUNCTION_ENTRY_CODE_SL;
    EIP93_CHECK_DEVICE_IS_RESET;

    memset(Device_p, 0, sizeof(EIP93_Device_t));
    Device_p->Device = Device;

    HWPAL_Device_Init(Device);

    // reset the the Packet Engine
    EIP93_Write32_PE_CFG(Device_p->Device,
              1, // Rst PE: yes
              1, // Reset PDR: yes
              0, 0, 0, 0, 0, 0 );

    // Start: Do the device communication test
    // Read the EIP number and see that it matches the expected "0x5E"
    EIP93_Read32_REVISION_REG(Device, &EipNumber, NULL, NULL, NULL, NULL);
    if(EipNumber != 0x5D)
    {
#ifdef RT_EIP93_DRIVER_DEBUG
        printk("EipNumber: %d\n", EipNumber);
#endif
        return EIP93_ERROR_BAD_ARGUMENT;
    }

    // Initilaize the PRNG in AUTO Mode
    EIP93_Internal_PRNG_Activate(Device_p->Device, true);

    // Initialize the BYTE_ORDER_CFG register
    EIP93_Write32_MST_BYTE_ORDER_CFG(Device,
            (uint8_t) EIP93_BYTE_ORDER_PD,
            (uint8_t) EIP93_BYTE_ORDER_SA,
            (uint8_t) EIP93_BYTE_ORDER_DATA,
            (uint8_t) EIP93_BYTE_ORDER_TD);

    // Initialize the INT_CFG register
    EIP93_Write32_INT_CFG(Device,
        (uint8_t)(EIP93_INT_HOST_OUTPUT_TYPE ? 1 : 0),
        (uint8_t)((EIP93_INT_HOST_OUTPUT_TYPE > 0) ?
        (EIP93_INT_PULSE_CLEAR > 0 ? 1 : 0) : 0));

     // Clock Control, must for DHM, optional for ARM
     EIP93_Write32(Device, 0X1E8, 0x1 );
    Device_p->CurrentMode = EIP93_MODE_INITIALIZED;

    EIP93_INSERTCODE_FUNCTION_EXIT_CODE;
}



/*----------------------------------------------------------------------------
 * EIP93_Deactivate
 *
 *  See header file for function specification.
 */
EIP93_Status_t
EIP93_Deactivate(
        EIP93_IOArea_t * const IOArea_p)
{
    EIP93_INSERTCODE_FUNCTION_ENTRY_CODE_SL;
    EIP93_CHECK_DEVICE_IS_NOT_RESET;

    if (Device_p->CurrentMode == EIP93_MODE_ARM)
    {
        // clean the ARM IOArea
        memset(ARM_p, 0, sizeof(*ARM_p));
        // reset the the Packet Descriptor Ring state machine
        EIP93_Write32_PE_CFG(Device_p->Device,
                             1, // Rst PE: Yes
                             1, // Reset PDR: yes
                             0, 0, 0, 0, 0, 0 );

    }
    else if (Device_p->CurrentMode == EIP93_MODE_DHM)
        {
            // clean the ARM IOArea
            memset(ARM_p, 0, sizeof(*DHM_p));
        EIP93_Write32_PE_CFG(Device_p->Device,
                             1, // Rst PE: Yes
                             0, // Reset PDR:No
                             0, 0, 0, 0, 0, 0 );

        }


    // change the mode
    Device_p->CurrentMode = EIP93_MODE_INITIALIZED;
    EIP93_INSERTCODE_FUNCTION_EXIT_CODE;
}


/*----------------------------------------------------------------------------
 * EIP93_Shutdown
 *
 *  See header file for function specification.
 */
EIP93_Status_t
EIP93_Shutdown(
        EIP93_IOArea_t * const IOArea_p)
{
    EIP93_INSERTCODE_FUNCTION_ENTRY_CODE_SL;


     // De-Initilaize the PRNG from AUTO Mode
     EIP93_Internal_PRNG_Activate(Device_p->Device, false);



    if (Device_p->CurrentMode == EIP93_MODE_ARM
        || Device_p->CurrentMode == EIP93_MODE_DHM
       )
    {
        EIP93_Deactivate(IOArea_p);
    }

    if (Device_p->CurrentMode == EIP93_MODE_INITIALIZED)
    {
        // reset the the Packet Engine
        EIP93_Write32_PE_CFG(Device_p->Device,
                             1, // Rst PE: yes
                             1, // Reset PDR: yes
                             0, 0, 0, 0, 0, 0);
        // clean the device IOArea
        memset(Device_p, 0, sizeof(*Device_p));
    }

    EIP93_INSERTCODE_FUNCTION_EXIT_CODE;
}


/*----------------------------------------------------------------------------
 *                      Descriptor Helper functions API
 *----------------------------------------------------------------------------
 */

/*----------------------------------------------------------------------------
 * EIP93_CommandDescriptor_Control_MakeWord
 *
 * See eip93_descriptor.h header file for description.
 */
uint32_t
EIP93_CommandDescriptor_Control_MakeWord(
        const EIP93_CommandDescriptor_Control_t * const  CommandCtrl_p)
{
    uint32_t word = 0;
    word |= CommandCtrl_p->PadControl << 24;
    word |= CommandCtrl_p->NextHeaderValue << 8;
    word |= (CommandCtrl_p->fHashFinal ? 1 : 0) << 4;
    return word;
}


/*----------------------------------------------------------------------------
 * EIP93_ResultDescriptor_Status_InterpretWord
 *
 * See eip93_descriptor.h header file for description.
 */
void
EIP93_ResultDescriptor_Status_InterpretWord(
        const uint32_t StatusWord,
        EIP93_ResultDescriptor_Status_t * const  ResultStatus_p)
{
    ResultStatus_p->RawStatus = (StatusWord >> 16) &  (BIT_8-1);
    ResultStatus_p->PadStatus = (StatusWord >> 24) &  (BIT_8-1);
    ResultStatus_p->PadValue = (StatusWord >> 8) &  (BIT_8-1);
    ResultStatus_p->ExtendedErrorCode =
            (ResultStatus_p->RawStatus >> 4) & (BIT_4-1);
    ResultStatus_p->fErrorORNotificationValid =
            (ResultStatus_p->RawStatus >> 3) & 1;
    ResultStatus_p->fSequenceNumberFail =
            (ResultStatus_p->RawStatus >> 2) & 1;
    ResultStatus_p->fCryptoPadFail =
            (ResultStatus_p->RawStatus >> 1) & 1;
    ResultStatus_p->fAuthenticationFail =
            ResultStatus_p->RawStatus & 1;
}




/*----------------------------------------------------------------------------
 * EIP93_INT_Mask
 */

EIP93_Status_t
EIP93_INT_Mask(
        EIP93_IOArea_t * const IOArea_p,
        const EIP93_INT_SourceBitmap_t WhichIntSources)

{

    EIP93_INSERTCODE_FUNCTION_ENTRY_CODE_SL;
    EIP93_CHECK_VALID_INTERRUPT(WhichIntSources);

    EIP93_Write32(
                 Device_p->Device,
                 EIP93_REG_MASK_DISABLE,
                 WhichIntSources);

    EIP93_INSERTCODE_FUNCTION_EXIT_CODE;
}

/*----------------------------------------------------------------------------
 * EIP93_INT_UnMask
 */

EIP93_Status_t
EIP93_INT_UnMask(
        EIP93_IOArea_t * const IOArea_p,
        const EIP93_INT_SourceBitmap_t WhichIntSources)

{

    EIP93_INSERTCODE_FUNCTION_ENTRY_CODE_SL;
    EIP93_CHECK_VALID_INTERRUPT(WhichIntSources);
    EIP93_Write32(
                 Device_p->Device,
                 EIP93_REG_MASK_ENABLE,
                 WhichIntSources);

    EIP93_INSERTCODE_FUNCTION_EXIT_CODE;
}

/*----------------------------------------------------------------------------
 * EIP93_INT_IsRawActive
 */

EIP93_Status_t
EIP93_INT_IsRawActive(
        EIP93_IOArea_t * const IOArea_p,
        EIP93_INT_SourceBitmap_t * const PendingIntSources_p)
{

    EIP93_INSERTCODE_FUNCTION_ENTRY_CODE_SL;
    EIP93_CHECK_POINTER(PendingIntSources_p);
    *PendingIntSources_p = EIP93_Read32(
                                   Device_p->Device,
                                   EIP93_REG_INT_UNMASK_STAT);

    EIP93_INSERTCODE_FUNCTION_EXIT_CODE;


}


/*----------------------------------------------------------------------------
 * EIP93_INT_IsActive
 */

EIP93_Status_t
EIP93_INT_IsActive(
        EIP93_IOArea_t * const IOArea_p,
        EIP93_INT_SourceBitmap_t * const PendingIntSources_p)
{

    EIP93_INSERTCODE_FUNCTION_ENTRY_CODE_SL;
    EIP93_CHECK_POINTER(PendingIntSources_p);

    *PendingIntSources_p = EIP93_Read32(
                                   Device_p->Device,
                                   EIP93_REG_INT_MASK_STAT);

    EIP93_INSERTCODE_FUNCTION_EXIT_CODE;

}


/*----------------------------------------------------------------------------
 * EIP93_INT_Acknowledge
 */

EIP93_Status_t
EIP93_INT_Acknowledge(
        EIP93_IOArea_t * const IOArea_p,
        const EIP93_INT_SourceBitmap_t WhichIntSources)
{
    EIP93_INSERTCODE_FUNCTION_ENTRY_CODE_SL;
    EIP93_CHECK_VALID_INTERRUPT(WhichIntSources);

    EIP93_Write32(
                 Device_p->Device,
                 EIP93_REG_INT_CLR,
                 WhichIntSources);

    EIP93_INSERTCODE_FUNCTION_EXIT_CODE;

}


/*----------------------------------------------------------------------------
 * EIP93_INT_Configure
 */
EIP93_Status_t
EIP93_INT_Configure(
        EIP93_IOArea_t * const IOArea_p,
        const bool fPulsed,
    const bool fAutoClear)
{


    EIP93_INSERTCODE_FUNCTION_ENTRY_CODE_SL;

    EIP93_Write32_INT_CFG(
                    Device_p->Device,
                    fPulsed ? 1 : 0,
                    fAutoClear ? 1 : 0);

    EIP93_INSERTCODE_FUNCTION_EXIT_CODE;

}


