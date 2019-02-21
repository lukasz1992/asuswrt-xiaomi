#include <asm/mach-ralink/rt_mmap.h>
#include "mtk_baseDefs.h"        // uint8_t, IDENTIFIER_NOT_USED, etc.
#include "mtk_hwAccess.h"         // HW access API
#include "mtk_arm.h"       // the API we will implement
#include "mtk_cEip93.h"         // configration options
#include "mtk_armL0.h" // macros and functions to access EIP93 reg
#include "mtk_internal.h"  // internal API

#ifdef EIP93_STRICT_ARGS

#define EIP93_INSERTCODE_FUNCTION_ENTRY_CODE_ARM \
    EIP93_Status_t res = EIP93_STATUS_OK; \
    EIP93_Device_t* Device_p = NULL; \
    EIP93_ARM_Mode_t* ARM_p = NULL; \
    EIP93_CHECK_POINTER(IOArea_p); \
    Device_p = (EIP93_Device_t*)IOArea_p; \
    ARM_p = &Device_p->extras.ARM_mode; \
    IDENTIFIER_NOT_USED(ARM_p);

#define EIP93_CHECK_ARM_IS_READY \
    EIP93_CHECK_POINTER(Device_p); \
    if (Device_p->CurrentMode != EIP93_MODE_ARM) \
    { \
        res = EIP93_ERROR_UNSUPPORTED_IN_THIS_STATE; \
        goto FUNC_RETURN; \
    }

#else

#define EIP93_INSERTCODE_FUNCTION_ENTRY_CODE_ARM \
    EIP93_Status_t res = EIP93_STATUS_OK; \
    EIP93_Device_t* Device_p = (EIP93_Device_t*)IOArea_p; \
    EIP93_ARM_Mode_t* ARM_p = &Device_p->extras.ARM_mode; \
    IDENTIFIER_NOT_USED(ARM_p);

#define EIP93_CHECK_ARM_IS_READY

#endif //EIP93_STRICT_ARGS

#ifdef RT_EIP93_DRIVER
#define VPint *(volatile unsigned int *)
#include "mtk_csDriver.h"  //to include VDRIVER_INTERRUPTS macro
#endif


/*----------------------------------------------------------------------------
 * EIP93_WriteCB
 * A write callback for the Ring Helper
 */
static int
EIP93_WriteCB(
        void * const CallbackParam1_p,
        const int CallbackParam2,
        const unsigned int WriteIndex,
        const unsigned int WriteCount,
    const unsigned int AvailableSpace,
        const void * Descriptors_p,
    const int DescriptorCount,
        const unsigned DescriptorSkipCount)
{
    unsigned int nDescrSize;
    unsigned int i;
#ifdef RT_EIP93_DRIVER_DEBUG
    unsigned int *p2;
#endif
    int nWritten = 0;

    EIP93_IOArea_t * IOArea_p = (EIP93_IOArea_t *) CallbackParam1_p;
    EIP93_INSERTCODE_FUNCTION_ENTRY_CODE_ARM;
    EIP93_CHECK_ARM_IS_READY;
    EIP93_CHECK_POINTER(Descriptors_p);

    nDescrSize = ARM_p->Settings.nDescriptorSize; //8 (words)

    IDENTIFIER_NOT_USED(CallbackParam2);
    //IDENTIFIER_NOT_USED(fKeepTogether);

    /*** first we write all descriptors we can to CDR ***/

    if (ARM_p->RingHelper.fSeparate) // separate rings
    {
        for(i = WriteIndex; i < WriteIndex + WriteCount; i++)
        {
            // write without checking ownership bits
            EIP93_ARM_Level0_WriteDescriptor(
                    ARM_p->CommandRingHandle,
                    i*nDescrSize,
                    ((const EIP93_ARM_CommandDescriptor_t *)Descriptors_p) +
                    DescriptorSkipCount + nWritten);

            nWritten++;
        }
    }
    else // combined rings
    {
        for(i = WriteIndex; i < WriteIndex + WriteCount; i++)
        {
            // just always write it
            EIP93_ARM_Level0_WriteDescriptor(
                ARM_p->CommandRingHandle,
                i*nDescrSize,
                ((const EIP93_ARM_CommandDescriptor_t *)Descriptors_p) +
                DescriptorSkipCount + nWritten);
            nWritten++;
        }
    }

    // now we call PreDMA to provide descriptors written for
    // the EIP93 DMA Master
    if (nWritten > 0)
    {

        HWPAL_DMAResource_PreDMA(ARM_p->CommandRingHandle,
                                 WriteIndex*nDescrSize*4,
                                 nWritten*nDescrSize*4);
                                 
#ifdef RT_EIP93_DRIVER_DEBUG                                  
    p2 = (unsigned int*)(VPint(RALINK_CRYPTO_ENGINE_BASE+0x80) | (0xa0000000));
    printk("\n[EIP93_WriteCB], CD_BASE:0x%p (B4 kcik PE):\n", p2);
    printk("CD_Control:0x%08x\n", *p2);
    printk("CD_SrcPktAddr:0x%08x\n", *(p2+1));
    printk("CD_DstPktAddr:0x%08x\n", *(p2+2));
    printk("CD_SADataAddr:0x%08x\n", *(p2+3));
    printk("CD_SAStateAddr:0x%08x\n", *(p2+4));
    printk("CD_SAStateAddr:0x%08x\n", *(p2+5));
    printk("CD_UserIdAddr:0x%08x\n", *(p2+6));
    printk("CD_Length:0x%08x\n", *(p2+7));
    printk("\n");
#endif            

#ifdef RT_EIP93_DRIVER
    //EndianSwap Setting for C.L.'s new POF for fix no_word_alignment  (put right b4 kick CryptoEngine)
		VPint(RALINK_CRYPTO_ENGINE_BASE+0x100) = 0x00000700;	//0x00030700;
		VPint(RALINK_CRYPTO_ENGINE_BASE+0x1d0) = 0x00e400e4;    
#endif
  
    EIP93_Write32_PE_CD_COUNT(Device_p->Device,(uint32_t)nWritten);

    }

    goto FUNC_RETURN;
FUNC_RETURN:
    if (res)
    {
        return -res;
    }
    else
    {
        return nWritten;
    }
}


/*----------------------------------------------------------------------------
 * EIP93_ReadCB
 * A read callback for the Ring Helper
 */
static int
EIP93_ReadCB(
        void * const CallbackParam1_p,
        const int CallbackParam2,
        const unsigned int ReadIndex,
        const unsigned int ReadLimit,
        void * Descriptors_p,
        const unsigned int DescriptorSkipCount)
{
    unsigned int nDescrSize;
    unsigned int i;
    int nRead = 0;
    EIP93_IOArea_t * const IOArea_p = (EIP93_IOArea_t *) CallbackParam1_p;
    EIP93_INSERTCODE_FUNCTION_ENTRY_CODE_ARM;
    EIP93_CHECK_ARM_IS_READY;
    EIP93_CHECK_POINTER(Descriptors_p);

    IDENTIFIER_NOT_USED(CallbackParam2);

    nDescrSize = ARM_p->Settings.nDescriptorSize;
    // now we read all descriptors we can from RDR

    for(i = ReadIndex; i < ReadIndex + ReadLimit; i++)
    {
        EIP93_ARM_ResultDescriptor_t * CurrentResultDesc_p =
            ((EIP93_ARM_ResultDescriptor_t *)Descriptors_p) +
            DescriptorSkipCount + nRead;

        // first we call PostDMA to obtain descriptors to be read from
        // the EIP93 DMA Master
        HWPAL_DMAResource_PostDMA(ARM_p->ResultRingHandle,
                                  i*nDescrSize*4,
                                  nDescrSize*4);

        // read it if ready
        if (EIP93_ARM_Level0_ReadDescriptor_IfReady(
                  CurrentResultDesc_p,
                  ARM_p->ResultRingHandle,
                  i*nDescrSize))
        {

            // just clear this descriptor
            EIP93_ARM_Level0_ClearDescriptor(
                    ARM_p->ResultRingHandle,
                    i*nDescrSize);

            // make sure our next "PostDMA" does not undo this
            HWPAL_DMAResource_PreDMA(
                    ARM_p->ResultRingHandle,
                    i*nDescrSize*4,
                    4);

#ifdef EIP93_ARM_NUM_OF_DESC_PADDING_WORDS
            CurrentResultDesc_p->fPaddingWordValuesAreValid =
                !ARM_p->RingHelper.fSeparate;
#endif //EIP93_ARM_NUM_OF_DESC_PADDING_WORDS

            nRead++;

        }
        else
        {
            break; // for
        }
    }


    if (nRead > 0)
    {
         EIP93_Write32_PE_RD_COUNT(Device_p->Device,(uint32_t)nRead);

#ifdef RT_EIP93_DRIVER
         //clear ResultInterrupt when PE_RD_COUNT==0
        if(VPint(RALINK_CRYPTO_ENGINE_BASE+0x94)==0)
                VPint(RALINK_CRYPTO_ENGINE_BASE+0x204) = 2;
#endif
    }

    goto FUNC_RETURN;
FUNC_RETURN:
    if (res)
    {
        return -res;
    }
    else
    {
        return nRead;
    }
}


/*----------------------------------------------------------------------------
 * EIP93_StatusCB
 * A status callback for the Ring Helper
 */

static int
EIP93_StatusCB(
        void * const CallbackParam1_p,
        const int CallbackParam2,
        int * const DeviceReadPos_p)
{
    uint16_t CmdIndex;
    uint16_t ResIndex;

    EIP93_IOArea_t * IOArea_p = (EIP93_IOArea_t *) CallbackParam1_p;
    EIP93_INSERTCODE_FUNCTION_ENTRY_CODE_ARM;
    EIP93_CHECK_ARM_IS_READY;
    IDENTIFIER_NOT_USED(CallbackParam2);

    EIP93_Read32_PE_RING_PNTR(Device_p->Device,
            (uint16_t *)&CmdIndex,
            (uint16_t *)&ResIndex);
    *DeviceReadPos_p = (int)CmdIndex;


    goto FUNC_RETURN;
FUNC_RETURN:
    return 0;
}


/*----------------------------------------------------------------------------
 * EIP93_ARM_Activate
 *
 *  See header file for function specification.
 */
EIP93_Status_t
EIP93_ARM_Activate(
        EIP93_IOArea_t * const IOArea_p,
        const EIP93_ARM_Settings_t * const Settings_p,
        const EIP93_ARM_RingMemory_t * const Ring_p)
{
    unsigned int RingSizeInDescr = 0;
    bool fEnableSwap_PD = false;
    bool fEnableSwap_SA = false;
    bool fEnableSwap_Data = false;

    EIP93_INSERTCODE_FUNCTION_ENTRY_CODE_ARM;
    EIP93_CHECK_DEVICE_IS_READY;
    EIP93_CHECK_POINTER(Settings_p);
    EIP93_CHECK_POINTER(Ring_p);


    EIP93_CHECK_INT_INRANGE(
            Settings_p->nDescriptorDoneCount,
            EIP93_MIN_DESC_DONE_COUNT,
            EIP93_MAX_DESC_DONE_COUNT);

    EIP93_CHECK_INT_INRANGE(
            Settings_p->nDescriptorPendingCount,
            EIP93_MIN_DESC_PENDING_COUNT,
            EIP93_MAX_DESC_PENDING_COUNT);

    EIP93_CHECK_INT_INRANGE(
            Settings_p->nDescriptorDoneTimeout,
            EIP93_MIN_TIMEOUT_COUNT,
            EIP93_MAX_TIMEOUT_COUNT);

    EIP93_CHECK_INT_INRANGE(
            Settings_p->nPEInputThreshold,
            EIP93_MIN_PE_INPUT_THRESHOLD,
            EIP93_MAX_PE_INPUT_THRESHOLD);

    EIP93_CHECK_INT_INRANGE(
            Settings_p->nPEOutputThreshold,
            EIP93_MIN_PE_OUTPUT_THRESHOLD,
            EIP93_MAX_PE_OUTPUT_THRESHOLD);

    EIP93_CHECK_INT_INRANGE(
            Settings_p->nDescriptorSize,
            EIP93_MIN_PE_DESCRIPTOR_SIZE,
            EIP93_MAX_PE_DESCRIPTOR_SIZE);

      // first we configure the Ring Helper

    ARM_p->RingHelperCallbacks.WriteFunc_p = &EIP93_WriteCB;
    ARM_p->RingHelperCallbacks.ReadFunc_p = &EIP93_ReadCB;
    ARM_p->RingHelperCallbacks.StatusFunc_p = &EIP93_StatusCB;
    ARM_p->RingHelperCallbacks.CallbackParam1_p = IOArea_p;
    ARM_p->RingHelperCallbacks.CallbackParam2 = 0;

    RingSizeInDescr = Ring_p->RingSizeInWords / Settings_p->nDescriptorSize;

    EIP93_CHECK_INT_INRANGE(
            RingSizeInDescr,
            EIP93_MIN_PE_RING_SIZE,
            EIP93_MAX_PE_RING_SIZE);

    RingHelper_Init(&ARM_p->RingHelper,
                &ARM_p->RingHelperCallbacks,
                Ring_p->fSeparateRings,
                RingSizeInDescr,
                RingSizeInDescr);


    // now we initialize the EIP PE and ring registers

    ARM_p->CommandRingHandle = Ring_p->CommandRingHandle;
    EIP93_Write32_PE_CDR_BASE(Device_p->Device,
                              Ring_p->CommandRingAddr.Addr);

    if(Ring_p->fSeparateRings)
    {
        ARM_p->ResultRingHandle  = Ring_p->ResultRingHandle;
        EIP93_Write32_PE_RDR_BASE(Device_p->Device,
                                  Ring_p->ResultRingAddr.Addr);
    }
    else
    {
        ARM_p->ResultRingHandle  = Ring_p->CommandRingHandle;
        EIP93_Write32_PE_RDR_BASE(Device_p->Device,
                                  Ring_p->CommandRingAddr.Addr);
    }

    EIP93_Write32_PE_RING_SIZE(
            Device_p->Device,
            (uint16_t)Settings_p->nDescriptorSize,
            //(uint16_t)RingSizeInDescr);
            (uint16_t)RingSizeInDescr -1); /*for integration*/

    EIP93_Write32_PE_RING_THRESH(
            Device_p->Device,
            (uint16_t) Settings_p->nDescriptorDoneCount,
            (uint16_t) Settings_p->nDescriptorPendingCount,
        (uint16_t) Settings_p->nDescriptorDoneTimeout);

    EIP93_Write32_PE_IO_THRESHOLD(Device_p->Device,
                                 (uint16_t) Settings_p->nPEInputThreshold,
                                  (uint16_t)Settings_p->nPEOutputThreshold);



    // prepare the ring buffers

    // Initialize all descriptors with zero for command ring
    EIP93_ARM_Level0_ClearAllDescriptors(
            ARM_p->CommandRingHandle,
            Settings_p->nDescriptorSize,
            RingSizeInDescr);


    // Call PreDMA to make sure engine sees it
    HWPAL_DMAResource_PreDMA(ARM_p->CommandRingHandle,
                             0,
                             Ring_p->RingSizeInWords*4);

    if(Ring_p->fSeparateRings)
    {
        EIP93_ARM_Level0_ClearAllDescriptors(
                ARM_p->ResultRingHandle,
                Settings_p->nDescriptorSize,
                RingSizeInDescr);

        // we do PreDMA for the whole RDR buffer, to make sure
        // the EIP93 DMA Master gets the full control over the buffer
        // (for instance, dirty cache lines are flushed now,
        // so they will not overwrite possible new result descriptors
        // written by EIP93 Packet Engine later)
        HWPAL_DMAResource_PreDMA(ARM_p->ResultRingHandle,
                                 0,
                                 Ring_p->RingSizeInWords*4);
    }

#ifdef EIP93_ENABLE_SWAP_PD
        fEnableSwap_PD = true;
#endif //EIP93_ENABLE_SWAP_PD
#ifdef EIP93_ENABLE_SWAP_SA
        fEnableSwap_SA = true;
#endif //EIP93_ENABLE_SWAP_SA
#ifdef EIP93_ENABLE_SWAP_DATA
        fEnableSwap_Data = true;
#endif //EIP93_ENABLE_SWAP_DATA



    // now we initizalize and start up the PE

    EIP93_Write32_PE_CFG(Device_p->Device,
                             0, // Rst PE: no
                             0, // Reset PDR: no
                             3, // ARM mode on
                             fEnableSwap_PD,
                             fEnableSwap_SA,
                             fEnableSwap_Data,
                             1, // PDR Update is on
                             0); // target mode swap off


    // now PE is running and we are ready to accept command descriptors
    // and process packet data

    ARM_p->Settings = *Settings_p;

    Device_p->CurrentMode = EIP93_MODE_ARM;

    EIP93_INSERTCODE_FUNCTION_EXIT_CODE;
}


/*----------------------------------------------------------------------------
 * EIP93_ARM_FinalizeSA
 *
 *  See header file for function specification.
 */
EIP93_Status_t
EIP93_ARM_FinalizeSA(
        EIP93_IOArea_t * const IOArea_p,
        const HWPAL_DMAResource_Handle_t SADataHandle)
{
    EIP93_INSERTCODE_FUNCTION_ENTRY_CODE_ARM;
    EIP93_CHECK_ARM_IS_READY;
    EIP93_CHECK_HANDLE(SADataHandle);


    EIP93_INSERTCODE_FUNCTION_EXIT_CODE;
}



/*----------------------------------------------------------------------------
 * EIP93_ARM_PacketPut
 *
 *  See header file for function specification.
 */
EIP93_Status_t
EIP93_ARM_PacketPut(
        EIP93_IOArea_t * const IOArea_p,
        const EIP93_ARM_CommandDescriptor_t *  CmdDescriptors_p,
        const unsigned int CmdDescriptorCount,
        unsigned int * const DoneCount_p)
{
    uint32_t CDtodoCnt;
    uint16_t DescriptorOffset;
    uint16_t RingSizeInDescriptor;
    EIP93_INSERTCODE_FUNCTION_ENTRY_CODE_ARM;
    EIP93_CHECK_ARM_IS_READY;
    EIP93_CHECK_POINTER(CmdDescriptors_p);
    EIP93_CHECK_POINTER(DoneCount_p);

    EIP93_Read32_PE_CD_COUNT(
            Device_p->Device,
        &CDtodoCnt);
    EIP93_Read32_PE_RING_SIZE(
            Device_p->Device,
            &DescriptorOffset,
            &RingSizeInDescriptor);

                                    
    if(CDtodoCnt < (uint32_t)RingSizeInDescriptor)
    {
        *DoneCount_p = RingHelper_Put(
                               &ARM_p->RingHelper,
                               CmdDescriptors_p,
                               CmdDescriptorCount);
    }
    else
    {
        *DoneCount_p = 0;
    }
    EIP93_INSERTCODE_FUNCTION_EXIT_CODE;
}



/*----------------------------------------------------------------------------
 * EIP93_ARM_PacketGet
 *
 *  See header file for function specification.
 */
EIP93_Status_t
EIP93_ARM_PacketGet(
        EIP93_IOArea_t * const IOArea_p,
        EIP93_ARM_ResultDescriptor_t *  ResDescriptors_p,
        const unsigned int ResDescriptorLimit,
        unsigned int * const DoneCount_p)
{   
    uint32_t DoneCnt=0;
    EIP93_INSERTCODE_FUNCTION_ENTRY_CODE_ARM;
    EIP93_CHECK_ARM_IS_READY;
    EIP93_CHECK_POINTER(ResDescriptors_p);
    EIP93_CHECK_POINTER(DoneCount_p);

#ifndef VDRIVER_INTERRUPTS
#ifdef RT_EIP93_DRIVER
/* When EIP93 receives a big packet, it can't provide result packet
 * right away. So we have to poll until EIP93 provides the result
 * packet. The polling mechanism has to come with the spinlock
 * "eip93_lock", otherwise the polling will fail on SMP environment.
 */
    while(DoneCnt==0)
        EIP93_Read32_PE_RD_COUNT(Device_p->Device,&DoneCnt);    
#else
    EIP93_Read32_PE_RD_COUNT(Device_p->Device,&DoneCnt);
#endif

#else
    EIP93_Read32_PE_RD_COUNT(Device_p->Device,&DoneCnt);
#endif

    if(DoneCnt)
    {
    *DoneCount_p = RingHelper_Get(
                       &ARM_p->RingHelper,
                       -1,                 /* ReadyCount we don't know*/
                       ResDescriptors_p,
                       ResDescriptorLimit);
     }
     else
     {
      *DoneCount_p = 0;
     }
          
    EIP93_INSERTCODE_FUNCTION_EXIT_CODE;
}



/*----------------------------------------------------------------------------
 * EIP93_ARM_PacketExternalGet
 *
 *  See header file for function specification.
 */
EIP93_Status_t
EIP93_ARM_PacketExternalGet(
        EIP93_IOArea_t * const IOArea_p,
        const unsigned int ExternalDoneCount)
{
    EIP93_INSERTCODE_FUNCTION_ENTRY_CODE_ARM;
    EIP93_CHECK_ARM_IS_READY;

    #if 0
    RingHelper_Notify(
            &ARM_p->RingHelper,
            ExternalDoneCount);
    #endif

    EIP93_INSERTCODE_FUNCTION_EXIT_CODE;
}


