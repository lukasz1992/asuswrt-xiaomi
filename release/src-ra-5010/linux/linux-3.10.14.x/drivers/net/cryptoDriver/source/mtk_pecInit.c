
#include "mtk_cAdapter.h"
#ifdef ADAPTER_EIP93_PE_MODE_ARM

#include "mtk_baseDefs.h"         // uint32_t
#include "mtk_pecApi.h"   // PEC_* (the API we implement here)
#include "mtk_dmaBuf.h"         // DMABuf_*
#include "mtk_hwDmaAccess.h"      // HWPAL_Resource_*
#include "mtk_cLib.h"               // memcpy
#include "mtk_AdapterInternal.h"
#include "mtk_eip93.h"
#include "mtk_interrupts.h"
#include "mtk_arm.h"        // driver library API we will use
#include "mtk_descp.h" // for parsing result descriptor
#include <linux/kernel.h>


static bool PEC_IsInitialized = false;


/*----------------------------------------------------------------------------
 * Adapter_PRNG_Init_ARM
 *
 * This function initializes the PE PRNG for the ARM mode.
 *
 * Return Value
 *      true: PRNG is initialized
 *     false: PRNG initialization failed
 */
static bool
Adapter_PRNG_Init_ARM(const bool fLongSA)
{
    int i;
    EIP93_Status_t res93;
    EIP93_ARM_CommandDescriptor_t EIP93_CmdDscr;
    EIP93_ARM_ResultDescriptor_t EIP93_ResDscr;
    DMABuf_Status_t dmares;
    DMABuf_HostAddress_t HostAddr;
    DMABuf_Properties_t DMAProp;
    DMABuf_Handle_t DMAHandle;
    EIP93_ResultDescriptor_Status_t EIP93ResDscrStatus;
    HWPAL_DMAResource_Handle_t DMAResHandle;
    unsigned int PutCount = 0;
    int LoopLimiter = 1000;
    EIP93_DeviceAddress_t EIP93PhysAddress = {0};

    DMAProp.Alignment = 4;        // used as uint32_t array
    DMAProp.Bank = 0;
    DMAProp.fCached = false;
    DMAProp.Size = 128;

    // Allocate DMA-safe buffer for SA record
    dmares = DMABuf_Alloc(DMAProp, &HostAddr, &DMAHandle);
    if (dmares != DMABUF_STATUS_OK)
    {
        printk(
            "Adapter_PRNG_Init_ARM: "
            "Failed to alloc DMA buffer (error %d)\n",
            dmares);

        return false;   // failure
    }

    // Fill in SA for PRNG Init
    *(((uint32_t*)HostAddr.p))   = 0x00001307;   // SA word 0
    *(((uint32_t*)HostAddr.p)+1) = 0x02000000;   // SA word 1
    if(fLongSA)
    {
        // 32-word SA
        const uint32_t PRNGKey[]      = {0xe0fc631d, 0xcbb9fb9a,
                                         0x869285cb, 0xcbb9fb9a,
                                         0, 0, 0, 0};
        const uint32_t PRNGSeed[]     = {0x758bac03, 0xf20ab39e,
                                         0xa569f104, 0x95dfaea6,
                                         0, 0, 0, 0};
        const uint32_t PRNGDateTime[] = {0, 0, 0, 0, 0, 0, 0, 0};

        for(i = 0; i < 8; i++)
        {
            *(((uint32_t*)HostAddr.p)+i+2)   = PRNGKey[i];
            *(((uint32_t*)HostAddr.p)+i+10)  = PRNGSeed[i];
            *(((uint32_t*)HostAddr.p)+i+18)  = PRNGDateTime[i];
        }// for
    }
    else
    {
        // 24-word SA
        const uint32_t PRNGKey[]      = {0xe0fc631d, 0xcbb9fb9a,
                                         0x869285cb, 0xcbb9fb9a,
                                         0, 0};
        const uint32_t PRNGSeed[]     = {0x758bac03, 0xf20ab39e,
                                         0xa569f104, 0x95dfaea6,
                                         0};
        const uint32_t PRNGDateTime[] = {0, 0, 0, 0, 0};

        // Write key data to SA
        for(i = 0; i < 6; i++)
        {

            *(((uint32_t*)HostAddr.p)+i+2)   = PRNGKey[i];
        }// for

        // Write Seed and Date&Time data to SA
        for(i = 0; i < 5; i++)
        {
            *(((uint32_t*)HostAddr.p)+i+8)   = PRNGSeed[i];
            *(((uint32_t*)HostAddr.p)+i+13)  = PRNGDateTime[i];
        }// for
    }

    Adapter_GetEIP93PhysAddr(DMAHandle, &DMAResHandle, &EIP93PhysAddress);

    // In-place copy to ensure correct endianness format
    {
        HWPAL_DMAResource_Record_t * const Rec_p =
            HWPAL_DMAResource_Handle2RecordPtr(DMAResHandle);

        HWPAL_DMAResource_Write32Array(
                    DMAResHandle,
                    0,
                    Rec_p->host.BufferSize / 4,
                    Rec_p->host.HostAddr_p);
    }

    // ask the EIP93 DrvLib to finalize the SA
    // (fill in some fields it is responsible for)
    res93 = EIP93_ARM_FinalizeSA(&Adapter_EIP93_IOArea, DMAResHandle);
    if (res93 != EIP93_STATUS_OK)
    {
        printk(
            "Adapter_PRNG_Init_ARM: "
            "EIP93_ARM_FinalizeSA returned %d\n",
            res93);

        goto fail;     // failure
    }

    // now use DMAResource to ensure the engine
    // can read the memory blocks using DMA
    HWPAL_DMAResource_PreDMA(DMAResHandle, 0, 0);     // 0,0 = "entire buffer"

    ZEROINIT(EIP93_CmdDscr);
    ZEROINIT(EIP93_ResDscr);

    // Fill in command descriptor
    EIP93_CmdDscr.ControlWord = 0x40;   // PRNG Init function
    EIP93_CmdDscr.SADataAddr.Addr = EIP93PhysAddress.Addr;

    res93 = EIP93_ARM_PacketPut(
                &Adapter_EIP93_IOArea,
                &EIP93_CmdDscr,
                1,
                &PutCount);
    if (res93 != EIP93_STATUS_OK)
    {
        printk(
            "Adapter_PRNG_Init_ARM: "
            "EIP93_ARM_PacketPut returned %d\n", res93);

        goto fail;       // failure
    }

    if (PutCount == 0)
        goto fail;       // failure

    // now wait for the result descriptor
    // normally this will we get the result descriptors in no-time
    while(LoopLimiter > 0)
    {
        unsigned int GetCount = 0;

        res93 = EIP93_ARM_PacketGet(
                    &Adapter_EIP93_IOArea,
                    &EIP93_ResDscr,
                    1,
                    &GetCount);
        if (res93 != EIP93_STATUS_OK)
        {
            printk(
                "Adapter_PRNG_Init_ARM: "
                "EIP93_ARM_PacketGet returned %d\n", res93);

            goto fail;       // failure
        }

        if (GetCount > 0)
            break;

        LoopLimiter--;
        // note: we might not be in a sleepable context
        // so no sleep call here!
    } // while

    if (LoopLimiter <= 0)
    {
        printk(
            "Adapter_PRNG_Init_ARM: "
            "EIP93_ARM_PacketGet could not retrieve a result descriptor\n");

        goto fail;       // failure
    }

    EIP93_ResultDescriptor_Status_InterpretWord(EIP93_ResDscr.StatusWord,
                                                &EIP93ResDscrStatus);

    if (EIP93ResDscrStatus.RawStatus != 0)
    {
        printk(
            "Adapter_PRNG_Init_ARM: "
            "EIP93_ARM_PacketGet returned with status code 0x%08x\n",
            EIP93_ResDscr.StatusWord);

        goto fail;       // failure
    }

    DMABuf_Release(DMAHandle);
    return true; // success

fail:
    DMABuf_Release(DMAHandle);
    return false;
}


/*----------------------------------------------------------------------------
 * PEC_Init
 */
PEC_Status_t
PEC_Init(
     const   PEC_InitBlock_t * const InitBlock_p)
{
    // ensure we init only once
    if (PEC_IsInitialized)
        return PEC_ERROR_BAD_USE_ORDER;


	if (!Adapter_EIP93_SetMode_ARM(0))
	{
        return PEC_ERROR_INTERNAL;      // ## RETURN ##
	}

    // Initialize PRNG if present
    {
        EIP93_Status_t res93;
        EIP93_Capabilities_t Capabilities;
        bool fLongSA = false;

        res93 = EIP93_HWRevision_Get(
                        &Adapter_EIP93_IOArea,
                        &Capabilities);
        if (res93 != EIP93_STATUS_OK)
        {
            printk(
                "PEC_Init: "
                "EIP93_HWRevision_Get returns error: %d\n",
                res93);

            return PEC_ERROR_INTERNAL;
        }

        if(Capabilities.fPrng)
        {
            if(Capabilities.fAes256 ||
               Capabilities.fSha224 ||
               Capabilities.fSha256)
                fLongSA = true;

            if (!Adapter_PRNG_Init_ARM(fLongSA))
            {
                // PRNG init failed, so shutdown and return an error code
                printk("PEC_Init: PRNG initialization failed!\n");

                Adapter_EIP93_SetMode_Idle();
                return PEC_ERROR_INTERNAL;
            }
            else
            {
                printk("PEC_Init: PRNG is initialized\n");
            }
        }
    }
	

    PEC_IsInitialized = true;


    return PEC_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * PEC_UnInit
 */
PEC_Status_t
PEC_UnInit(void)
{
    // ensure we un-init only once
    if (PEC_IsInitialized)
    {
        Adapter_EIP93_SetMode_Idle();

#ifdef ADAPTER_EIP93PE_INTERRUPTS_ENABLE
        Adapter_Interrupt_Disable(IRQ_RDR_THRESH_IRQ);
        Adapter_Interrupt_Disable(IRQ_CDR_THRESH_IRQ);
#endif

        PEC_IsInitialized = false;
    }

    return PEC_STATUS_OK;
}


#else
;       // avoids "empty translation unit" warning
#endif /* ADAPTER_EIP93_PE_MODE_ARM */

