
#include "mtk_cAdapter.h"
#include "mtk_AdapterInternal.h"
#include "mtk_interrupts.h"
#include "mtk_arm.h"

#include "mtk_addrTrans.h"      // AddrTrans_*
#include <asm/io.h>                 // for virt_to_bus

#define   K1_TO_PHYSICAL(x) (((u32)(x)) & 0x1fffffff)

EIP93_IOArea_t Adapter_EIP93_IOArea;

#ifdef MTK_CRYPTO_DRIVER
unsigned int *pCmdRingBase, *pResRingBase;
#endif

static DMABuf_Handle_t Adapter_EIP93_CmdRing_Handle;
#ifdef ADAPTER_EIP93_SEPARATE_RINGS
static DMABuf_Handle_t Adapter_EIP93_ResRing_Handle;
#endif

static const DMABuf_Properties_t Adapter_EIP93_RingProps =
{
    ADAPTER_EIP93_RINGSIZE_BYTES, //defined in cs_adaptor.h
    4,          // Alignment
    0,          // Bank
    false       // fCached
};

typedef enum
{
    ADAPTER_EIP93_MODE_DISABLED,
    ADAPTER_EIP93_MODE_IDLE,
    ADAPTER_EIP93_MODE_ARM,
    ADAPTER_EIP93_MODE_DHM
} Adapter_EIP93_Mode_t;

static Adapter_EIP93_Mode_t Adapter_EIP93_Mode;
unsigned int Adapter_EIP93_MaxDescriptorsInRing;


/*----------------------------------------------------------------------------
 * Adapter_EIP93_SetMode_Idle
 *
 * Return Value
 *     true  Success
 *     false Failed
 */
bool
Adapter_EIP93_SetMode_Idle(void)
{

   EIP93_Status_t res93 ;

    switch (Adapter_EIP93_Mode)
    {
        case ADAPTER_EIP93_MODE_DISABLED:
            // cannot use EIP93
            return false;

        case ADAPTER_EIP93_MODE_IDLE:
            // already idle
            return true;

        case ADAPTER_EIP93_MODE_DHM:
            res93 = EIP93_Deactivate(&Adapter_EIP93_IOArea);
            if (res93 != EIP93_STATUS_OK)
            {
                printk(
                    "Adapter_EIP93_SetMode_Idle: "
                    "EIP93_Deactivate returned %d\n",
                    res93);
            }


            printk("Adapter: Successfully deactivated EIP93 \n");
            Adapter_EIP93_Mode = ADAPTER_EIP93_MODE_IDLE;
            printk("Adapter: Successfully deactivated EIP93 \n");
            return true;

        case ADAPTER_EIP93_MODE_ARM:

            res93 = EIP93_Deactivate(&Adapter_EIP93_IOArea);
            if (res93 != EIP93_STATUS_OK)
            {
                printk(
                    "Adapter_EIP93_SetMode_Idle: "
                    "EIP93_Deactivate returned %d\n",
                     res93);
            }
            // free the ring memory blocks
            DMABuf_Release(Adapter_EIP93_CmdRing_Handle);
#ifdef ADAPTER_EIP93_SEPARATE_RINGS
            DMABuf_Release(Adapter_EIP93_ResRing_Handle);
#endif

            Adapter_EIP93_Mode = ADAPTER_EIP93_MODE_IDLE;
            printk("Adapter: Successfully deactivated EIP93v2\n");
            return true;


        default:
            break;
    } // switch

    printk("Adapter_EIP93_SetMode_Idle: Unexpected mode %d\n",
            Adapter_EIP93_Mode);

    return false;
}

/*----------------------------------------------------------------------------
 * Adapter_EIP93_SetMode_ARM
 *
 * Return Value
 *     true  Success
 *     false Failed
 */
bool
Adapter_EIP93_SetMode_ARM(
        const bool fEnableDynamicSA)
{
#ifndef ADAPTER_EIP93_PE_MODE_ARM
    IDENTIFIER_NOT_USED(fEnableDynamicSA);
    return false;
#else
    EIP93_ARM_Settings_t Settings = {0};
    EIP93_ARM_RingMemory_t RingMemory = {0};
    EIP93_Status_t res93;

    if (Adapter_EIP93_Mode != ADAPTER_EIP93_MODE_IDLE)
    {
        printk("Adapter_EIP93_SetMode_ARM: Not possible in mode %d\n",
            Adapter_EIP93_Mode);
        return false;
    }

/**** allocate memory for the ring ****/

    ZEROINIT(RingMemory);
    // bytes to words conversion from ADAPTER_EIP93_RINGSIZE_BYTES
    RingMemory.RingSizeInWords = Adapter_EIP93_RingProps.Size >> 2;

    {
        DMABuf_Status_t dmares;
        DMABuf_HostAddress_t HostAddr;

        dmares = DMABuf_Alloc(
                        Adapter_EIP93_RingProps,
                        &HostAddr,
                        &Adapter_EIP93_CmdRing_Handle);
						
 	#ifdef MTK_CRYPTO_DRIVER
		pCmdRingBase = (unsigned int*)HostAddr.p; //store command descriptor ring base address (uncached memory address)
	#endif

        if (dmares != DMABUF_STATUS_OK)
        {
            printk(
                "Adapter_EIP93_Init: "
                "DMABuf_Alloc (Command Ring) returned %d\n",
                dmares);

            return false;
        }

        printk(
            "Adapter_EIP93_Init: "
            "CmdRing_Handle=%p\n",
            Adapter_EIP93_CmdRing_Handle.p);

        Adapter_GetEIP93PhysAddr(
                Adapter_EIP93_CmdRing_Handle,
                &RingMemory.CommandRingHandle,
                &RingMemory.CommandRingAddr);

        if (RingMemory.CommandRingAddr.Addr == 0)
        {
            printk(
                "Adapter_EIP93_Init: "
                "Failed to get command ring physical address\n");

            // free the command ring memory
            DMABuf_Release(Adapter_EIP93_CmdRing_Handle);

            return false;       // ## RETURN ##
        }
    }

#ifndef ADAPTER_EIP93_SEPARATE_RINGS
    // not separate rings
    RingMemory.fSeparateRings = false;
#else
    // separat rings
    RingMemory.fSeparateRings = true;

    {
        DMABuf_Status_t dmares;
        DMABuf_HostAddress_t HostAddr;

        dmares = DMABuf_Alloc(
                        Adapter_EIP93_RingProps,
                        &HostAddr,
                        &Adapter_EIP93_ResRing_Handle);
 
 	#ifdef MTK_CRYPTO_DRIVER
		pResRingBase = (unsigned int*)HostAddr.p; //store result descriptor ring base address (uncached memory address)
 	#endif
		
        if (dmares != DMABUF_STATUS_OK)
        {
            printk(
                    "Adapter_EIP93_Init: "
                    "DMABuf_Alloc (Result Ring) returned %d\n",
                    dmares);

            // free the command ring memory
            DMABuf_Release(Adapter_EIP93_CmdRing_Handle);

            return false;
        }

        printk(
            "Adapter_EIP93_Init: "
            "ResRing_Handle=%p\n",
            Adapter_EIP93_ResRing_Handle.p);

        Adapter_GetEIP93PhysAddr(
                Adapter_EIP93_ResRing_Handle,
                &RingMemory.ResultRingHandle,
                &RingMemory.ResultRingAddr);
                
        if (RingMemory.ResultRingAddr.Addr == 0)
        {
            printk(
                "Adapter_EIP93_Init: "
                "Failed to get result ring physical address\n");

            // free the ring memories
            DMABuf_Release(Adapter_EIP93_CmdRing_Handle);
            DMABuf_Release(Adapter_EIP93_ResRing_Handle);

            return false;
        }
    }
#endif /* ADAPTER_EIP93_SEPARATE_RINGS */

    // create the engine settings block
    Settings.nPEInputThreshold = ADAPTER_EIP93_DMATHRESHOLD_INPUT;
    Settings.nPEOutputThreshold = ADAPTER_EIP93_DMATHRESHOLD_OUTPUT;
    Settings.nDescriptorDoneCount = ADAPTER_EIP93_DESCRIPTORDONECOUNT;
    Settings.nDescriptorDoneTimeout = ADAPTER_EIP93_DESCRIPTORDONETIMEOUT;
    Settings.nDescriptorPendingCount= ADAPTER_EIP93_DESCRIPTORPENDINGCOUNT;
    Settings.nDescriptorSize = 8 ;
//    Settings.nRingPollDivisor = ADAPTER_EIP93_RINGPOLLDIVISOR;

    Adapter_EIP93_MaxDescriptorsInRing =
            RingMemory.RingSizeInWords /
            EIP93_ARM_DESCRIPTOR_SIZE();

    res93 = EIP93_ARM_Activate(
                &Adapter_EIP93_IOArea,
                &Settings,
                &RingMemory);

    if (res93 != EIP93_STATUS_OK)
    {
        printk(
            "Adapter_EIP93_Init: "
            "EIP93_ARM_Activate returned %d\n",
            res93);

        // free the ring memory blocks
        DMABuf_Release(Adapter_EIP93_CmdRing_Handle);
#ifdef ADAPTER_EIP93_SEPARATE_RINGS
        DMABuf_Release(Adapter_EIP93_ResRing_Handle);
#endif
        return false;
    }

    Adapter_EIP93_Mode = ADAPTER_EIP93_MODE_ARM;

    printk("Adapter: Successfully initialized EIP93v2 in ARM mode\n");

    return true;
#endif /* ADAPTER_EIP93_PE_MODE_ARM */
}


/*----------------------------------------------------------------------------
 * Adapter_EIP93_Init
 */
bool
Adapter_EIP93_Init(void)
{
    HWPAL_Device_t Device; //void*
    EIP93_Status_t res93;

    Adapter_EIP93_Mode = ADAPTER_EIP93_MODE_DISABLED;

    if (!HWPAL_Device_Find("eip93", &Device))
    {
        printk("Adapter_EIP93_Init: Failed to find EIP93 device\n");
        return false;
    }

    res93 = EIP93_Initialize(
                &Adapter_EIP93_IOArea,
                Device);

    if (res93 != EIP93_STATUS_OK)
    {
        printk(
            "Adapter_EIP93_Init: "
            "EIP93_Initialize returned %d\n",
            res93);
        return false;
    }

#ifdef ADAPTER_EIP93PE_INTERRUPTS_ENABLE
    // Configure level of Interrupts
    res93 = EIP93_INT_Configure(
                &Adapter_EIP93_IOArea,
                false,
                false);

    if (res93 != EIP93_STATUS_OK)
    {
        printk(
            "Adapter_EIP93_Init: "
            "EIP93_INT_Configure returned %d\n",
            res93);

        return false;
    }

#endif // ADAPTER_EIP93PE_INTERRUPTS_ENABLE



    Adapter_EIP93_Mode = ADAPTER_EIP93_MODE_IDLE;

    return true;
}


/*----------------------------------------------------------------------------
 * Adapter_EIP93_UnInit
 */
void
Adapter_EIP93_UnInit(void)
{
    EIP93_Status_t res93;

    res93 = EIP93_Shutdown(&Adapter_EIP93_IOArea);

    if (res93 != EIP93_STATUS_OK)
    {
        printk(
            "Adapter_EIP93_UnInit: "
            "EIP93_Shutdown returned %d\n",
            res93);
    }

    if (Adapter_EIP93_Mode == ADAPTER_EIP93_MODE_ARM)
    {
        // free the ring memory blocks
        DMABuf_Release(Adapter_EIP93_CmdRing_Handle);
#ifdef ADAPTER_EIP93_SEPARATE_RINGS
        DMABuf_Release(Adapter_EIP93_ResRing_Handle);
#endif
    }

    Adapter_EIP93_Mode = ADAPTER_EIP93_MODE_DISABLED;
}


/*----------------------------------------------------------------------------
 * AddrTrans_Translate
 */
AddrTrans_Status_t
AddrTrans_Translate(
        const AddrTrans_Pair_t PairIn,
        const unsigned int AlternativeRef,
        AddrTrans_Domain_t DestDomain,
        AddrTrans_Pair_t * const PairOut_p)
{
    // we only support 1:1 translation from driver to device domain
    if (PairOut_p == NULL)
        return ADDRTRANS_ERROR_BAD_ARGUMENT;

    if (DestDomain != ADDRTRANS_DOMAIN_DEVICE_PE)
        return ADDRTRANS_ERROR_CANNOT_TRANSLATE;

    PairOut_p->Domain = DestDomain;

    if (PairIn.Domain == ADDRTRANS_DOMAIN_ALTERNATIVE &&
        AlternativeRef == ADAPTER_DMABUF_ALLOCATORREF_KMALLOC)
    {
#ifdef RT_EIP93_DRIVER
        PairOut_p->Address_p = (void *)K1_TO_PHYSICAL(PairIn.Address_p);
#else
        // linux kmalloc allocated address
        // we can use virt_to_bus on this one
        PairOut_p->Address_p = (void *)virt_to_bus(PairIn.Address_p);
#endif
        return ADDRTRANS_STATUS_OK;
    }

    PairOut_p->Domain = ADDRTRANS_DOMAIN_UNKNOWN;
    PairOut_p->Address_p = 0;

    return ADDRTRANS_ERROR_CANNOT_TRANSLATE;
}

/*----------------------------------------------------------------------------
 * Adapter_GetEIP93PhysAddr
 *
 * This routine checks if we have already translated the address for EIP93
 * DMA. If not, we translate it now and cache it in the DMAResource Record.
 * We then return the address.
 *
 * Handle
 *     Must be a valid handle.
 *
 * Return Value
 *     Physical address for the start of the buffer referred to by Handle,
 *     or zero if address translation was not possible.
 */
void
Adapter_GetEIP93PhysAddr(
        DMABuf_Handle_t Handle,
        HWPAL_DMAResource_Handle_t * const DMAHandle_p,
        EIP93_DeviceAddress_t * const EIP93PhysAddr_p)
{
    HWPAL_DMAResource_Handle_t DMAHandle;
    HWPAL_DMAResource_Record_t * Rec_p = NULL;

    if (EIP93PhysAddr_p == NULL)
    {
        printk("Adapter_GetEIP93PhysAddr: PANIC\n");
        return;
    }

    // initialize the output parameters
    if (DMAHandle_p)
        *DMAHandle_p = NULL;

    EIP93PhysAddr_p->Addr = 0;
    EIP93PhysAddr_p->UpperAddr = 0;

    if (!Adapter_DMABuf_IsValidHandle(Handle))
        return;

    {
        // translate the handle
        DMAHandle = Adapter_DMABuf_Handle2DMAResourceHandle(Handle);

        // get the record belonging to this handle
        Rec_p = HWPAL_DMAResource_Handle2RecordPtr(DMAHandle);
    }

    if (Rec_p == NULL)
        return;

    if (DMAHandle_p)
        *DMAHandle_p = DMAHandle;

    if (Rec_p->device.DeviceAddr32 > 1)
        goto PHYS_DONE;

    if (0 == Rec_p->device.DeviceAddr32)
    {
        // first time, so translate
        AddrTrans_Pair_t PairIn;
        AddrTrans_Pair_t PairOut;
        AddrTrans_Status_t res;

        // assume address translation will fail
        Rec_p->device.DeviceAddr32 = 1;
                 // value used inside this function only

        if (Rec_p->alloc.AllocatorRef == ADAPTER_DMABUF_ALLOCATORREF_KMALLOC)
        {
            // kmalloc-allocated buffer

            // address translator needs to know this through alternative-ref
            PairIn.Address_p = Rec_p->host.HostAddr_p;
            PairIn.Domain = ADDRTRANS_DOMAIN_ALTERNATIVE;

            res = AddrTrans_Translate(
                            PairIn,
                            ADAPTER_DMABUF_ALLOCATORREF_KMALLOC,
                            ADDRTRANS_DOMAIN_DEVICE_PE,
                            &PairOut);

            if (res == ADDRTRANS_STATUS_OK)
            {
                // support for 32bit addresses only

                // unsigned long will be 64bit only on 64bit systems
                unsigned long Addr64or32 = (unsigned long)PairOut.Address_p;
                unsigned long HighAddr = Addr64or32;
                HighAddr >>= 16;  // shift in two steps avoids
                HighAddr >>= 16;  // side-effect with failing 32bit shift
                if (HighAddr)
                {
                    printk(
                        "Adapter_EIP93: "
                        "Physical Address too > 4GB not supported!"
                        " (handle=%p)\n",
                        Handle.p);
                    return;     // ## RETURN ##
                }

                Rec_p->device.DeviceAddr32 = (uint32_t)Addr64or32;

#ifdef ADAPTER_EIP93_ARMRING_ENABLE_SWAP
                Rec_p->device.fSwapEndianess = true;
#endif

                goto PHYS_DONE;
            }
            else
            {
                printk(
                "Adapter_GetEIP93PhysAddr: AddrTrans_Translate returned %d\n",
                res);
            }
        }

        if (1 == Rec_p->device.DeviceAddr32)
        {
            printk(
            "Adapter_GetEIP93PhysAddr: Address translation not possible\n");
        }
    }

    if (1 == Rec_p->device.DeviceAddr32)
    {
        // address translation tried before and failed
        return;
    }

PHYS_DONE:
    EIP93PhysAddr_p->Addr = Rec_p->device.DeviceAddr32;
}

