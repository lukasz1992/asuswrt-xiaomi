
#ifndef EIP93_ARM_H
#define EIP93_ARM_H

#include "mtk_eip93.h"
#include "mtk_hwDmaAccess.h"



/*----------------------------------------------------------------------------
 * Number of 32bit words for a descriptor used in ARM of EIP93
 *
 */
static inline unsigned int
EIP93_ARM_DESCRIPTOR_SIZE(void )
{
#ifdef EIP93_ARM_NUM_OF_DESC_PADDING_WORDS

        return (8+EIP93_ARM_NUM_OF_DESC_PADDING_WORDS);

#else

        return 8;

#endif
}


/*----------------------------------------------------------------------------
 * Interrupts of  EIP93 valid in ARM
 */
typedef enum
{
    EIP93_ARM_INT_CDR_THRESHOLD =         BIT_0,
    EIP93_ARM_INT_RDR_THRESHOLD =         BIT_1,

} EIP93_ARM_InterruptSource_t;


/*----------------------------------------------------------------------------
 * ARM settings
 * used in _ARM_Activate function as an input
 */
typedef struct
{
    // Indicate how many 32-bit words of free space (1-511) must be available
    // in the PE input Data RAM buffer before a  input transfer starts.
    // A value of 0x20 .. 0x80 generally gives a good performance but
    // the optimal value depends on the system and application.
    unsigned int nPEInputThreshold;

    // Indicates how many 32-bit words of data (1-432) must be available in
    // the PE output Data RAM buffer before a  output transfer starts and
    // the exact burst length that is actually used for the transfer.
    // The maximum threshold is  511 words, as this buffer is
    // also used to store up to 256 pad bytes and a possible 64 bytes ICV that
    // can be stripped for decrypt operations. The last data transfer that
    // completes the packet processing can differ to this transfer size.
    // A value of 0x20 .. 0x80 generally gives a good performance but
    // the optimal value depends on the system and application.
    unsigned int nPEOutputThreshold;

    // This parameter allows configuring the number of descriptors that must
    // be completed before issuing a EIP93_INT_PE_CDRTHRESH_REQ interrupt.
    unsigned int nDescriptorDoneCount;

    // This parameter allows configuring the minimum number of descriptors
    // that must be present in the command ring. reaching or falling below
    // will  issuing a EIP93_INT_PE_RDRTHRESH_REQ interrupt.
    unsigned int nDescriptorPendingCount;

    // This parameter allows configuring maximum number of clock cycles allowed
    // for a result descriptor to reside in the result ring. Extending this time
    // will result in issuing a EIP93_INT_PE_RDRTHRESH_REQ interrupt.
    // The rdr_thresh_irq interrupt activates when the RD
    // counter for the RDR is non-zero for more than 2(N+8) internal system
    // clock cycles, where 'N' is the value set in this field. Valid settings
    // range from 0 to 15.
    // The minimum time-out value for N=0 is 128 clock cycles and the
    // maximum timeout value for N=15 is 8388608 clock cycles.
    // At 100 MHz this is 1.28 us for N=0 and 83.89 ms for N=15.

    unsigned int nDescriptorDoneTimeout;

    // This parameter sets the spacing of a descriptor in 32bit words. The
    // actual size of the descriptor also depends on Padding Words.
    // This can be used to align each descriptor to a full cache
    // line. Valid range is 8-15.
    unsigned int nDescriptorSize;

} EIP93_ARM_Settings_t;


/*----------------------------------------------------------------------------
 * ARM descriptor rings settings
 * used in _ARM_Activate function as an input
 */
typedef struct
{
    // This parameter indicates whether the command and result rings share one
    // ring (one block of memory) or use separate rings (two memory blocks).
    bool fSeparateRings;

    // Command Ring DMA resource handle
    HWPAL_DMAResource_Handle_t CommandRingHandle;
    // Command Ring physical address, understandable by Device
    EIP93_DeviceAddress_t CommandRingAddr;

    // Result Ring DMA resource handle
    HWPAL_DMAResource_Handle_t ResultRingHandle;
    // Result Ring physical address, understandable by Device
    EIP93_DeviceAddress_t ResultRingAddr;

    // Ring Size, in words
    // This value is provided for both: Command and Result rings.
    unsigned int RingSizeInWords;

} EIP93_ARM_RingMemory_t;


/*----------------------------------------------------------------------------
 * Logical Command Descriptor for PacketPut
 * and a helper function
 */
typedef struct
{
    // control fields for the command descriptor
    // EIP93_CommandDescriptor_Control_MakeWord helper function
    // can be used for obtaining this word
    uint32_t ControlWord;

    // source packet data, has to be provided by the caller:
    // physical address, understandable by Device
    EIP93_DeviceAddress_t SrcPacketAddr;
    // source packet data length, in bytes
    unsigned int SrcPacketByteCount;
    // length of source packet data in words that must bypass
    // the Packet Engine and are directly copied from
    // the source packet buffer to the destination packet buffer
    uint32_t BypassWordLength;

    // where to place the result data, has to be allocated by the caller:
    // physical address, understandable by Device
    EIP93_DeviceAddress_t DstPacketAddr;

    // SA data, has to be allocated and filled in by the caller:
    // physical address, understandable by Device
    EIP93_DeviceAddress_t SADataAddr;

    //SA State,has to be allocated and filled in by the caller:
    // physical address, understandable by Device
    EIP93_DeviceAddress_t SAStateDataAddr;

    // copy through content from command to result descriptor
    uint32_t UserId;

#ifdef EIP93_ARM_NUM_OF_DESC_PADDING_WORDS
    // 0 or more padding word values
    // these values will be placed by _PacketPut
    // to additional padding words in a command descriptor passed
    // to the device.
    uint32_t PaddingWords[EIP93_ARM_NUM_OF_DESC_PADDING_WORDS];
#endif

} EIP93_ARM_CommandDescriptor_t;


static inline void
EIP93_ARM_CommandDescriptor_InitMany(
        EIP93_ARM_CommandDescriptor_t * CommandDescriptors_p,
        const unsigned int CommandDescriptorsCount,
        const EIP93_DeviceAddress_t SADataAddr,
        const EIP93_DeviceAddress_t SAStateDataAddr,
        const uint32_t ControlWord)
{
    unsigned int i;
    if (!CommandDescriptors_p)
        return;
    for(i = 0; i <= CommandDescriptorsCount; i++)
    {
        CommandDescriptors_p[i].SADataAddr = SADataAddr;
        CommandDescriptors_p[i].SAStateDataAddr = SAStateDataAddr;
        CommandDescriptors_p[i].ControlWord = ControlWord;
    }
}

/*----------------------------------------------------------------------------
 * Logical Result Descriptor for PacketGet.
 */
typedef struct
{
    // status fields for the result descriptor
    // EIP93_ResultDescriptor_Status_InterpretWord helper function
    // can be used for obtaining this word
    uint32_t StatusWord;

    // result packet data length, in bytes
    unsigned int DstPacketByteCount;

    // length of packet data in words that must bypass the Packet Engine
    // and are directly copied from the source packet buffer to the
    // destination packet buffer
    uint32_t BypassWordLength;

    //copied through user ID
    uint32_t UserId;

#ifdef EIP93_ARM_NUM_OF_DESC_PADDING_WORDS
    // whether a number of padding words values present are valid
    bool fPaddingWordValuesAreValid;
    // 0 or more padding word values
    // these values could have been placed by _PacketPut to the command ring
    // if fPaddingWordValuesAreValid is FALSE,
    // then there are two reasons for that:
    //  - either these words were not provided by _PacketPut
    //  - or _PacketGet just could not retrieve them from the result ring
    //    (this is not possible for separate command and result rings)
    uint32_t PaddingWords[EIP93_ARM_NUM_OF_DESC_PADDING_WORDS];
#endif

} EIP93_ARM_ResultDescriptor_t;


/*----------------------------------------------------------------------------
 *                         ARM APIs
 *----------------------------------------------------------------------------
 */

/*----------------------------------------------------------------------------
 * EIP93_ARM_Activate
 * configures the EIP93 Packet Engine operational mode for Autonomous Ring.
 *
 * This function is only allowed to be called from the 'initialized' state,
 * to which the device can be put by _Initialize or _Deactivate functions.
 * This function puts the device to the 'ARM activated' state.
 *
 * (input)Settings_p - different settings for ARM mode that influence
 *                  various behaviour aspects of the EIP93 packet engine,
                    such as PDR polling, interrupts, SA format.
 *
 * (input)Ring_p - command and result descriptor rings memory settings:
 *                 DMA resources and physical base addresses.
 *
 * Returns EIP93_STATUS_OK if the EIP93 packet engine was successfully
 * activated in the ARM mode, in this case the EIP93 packet engine
 * starts polling for new command descriptors in PDR
 * (if Settings_p->nRingPollDivisor is not 0).
 */
EIP93_Status_t
EIP93_ARM_Activate(
        EIP93_IOArea_t * const IOArea_p,
        const EIP93_ARM_Settings_t * const Settings_p,
        const EIP93_ARM_RingMemory_t * const Ring_p);



//NOTE:
//     The rest of the API functions in this header file can be called
//     only in the 'ARM activated' state, that is AFTER the EIP93_ARM_Activate
//     function was sucessfully called.



/*----------------------------------------------------------------------------
 * EIP93_ARM_FinalizeSA
 *
 * This function has to be called once for each SA data buffer BEFORE
 * it can be referenced in EIP93_ARM_CommandDescriptor_t instances, which
 * are passsed to EIP93_ARM_PE_PacketPut().
 * After this call the SA data buffer cannot be modified by Host,
 * but EIP93_ARM_PE_PacketPut() invocations using this SA can be done.
 *
 * (input)SADataHandle - DMA resource handle for the SA data buffer
 *
 */
EIP93_Status_t
EIP93_ARM_FinalizeSA(
        EIP93_IOArea_t * const IOArea_p,
        const HWPAL_DMAResource_Handle_t SADataHandle);


/*----------------------------------------------------------------------------
 * EIP93_ARM_PacketPut
 *
 * Inserts packet descriptors in the command descriptor ring.
 * May be called immediately after the EIP93_INT_PE_CDRTHRESH_REQ interrupt.
 *
 * EIP93_INT_PE_RDRTHRESH_REQ  interrupt can be triggered as a result
 * of this function's call.
 * In this case the EIP93_ARM_PacketGet function can be used
 * to fetch the results from the result descriptor ring.
 *
 * (input)CmdDescriptors_p - Pointer to 1st in the the array of descriptors.
 *
 * (input)CmdDescriptorCount - Number of descriptors stored
 * back-to-back in the array pointed to by CmdDescriptors_p.
 *
 * (output)DoneCount - Number of descriptors actually inserted
 *                     to the command descriptor ring.
 *
 * Returns EIP93_BUSY_RETRY_LATER  when  PE is busy to carry out the request,
 * in such situation call again after the above said criteria is meet.
 */
EIP93_Status_t
EIP93_ARM_PacketPut(
        EIP93_IOArea_t * const IOArea_p,
        const EIP93_ARM_CommandDescriptor_t *  CmdDescriptors_p,
        const unsigned int CmdDescriptorCount,
        unsigned int * const DoneCount_p);


/*----------------------------------------------------------------------------
 * EIP93_ARM_PacketGet
 *
 * May be called after the EIP93_INT_PE_RDRTHRESH_REQ interrupt.
 *
 * (input)ResDescriptors_p - Pointer to 1st in the the array of descriptors.
 *
 * (input)ResDescriptorLimit - Maximum number of descriptors that are expected
 * to be got from the result ring and for which it is enough space in
 * ResDescriptors_p.
 *
 * (output)DoneCount - Number of descriptors actually emptied from the
 * result ring and stored back-to-back in the
 * array pointed to by ResDescriptors_p.
 *
 * Returns EIP93_BUSY_RETRY_LATER  if  PE busy to carry out the
 * request, in such situation call again later.
 */
EIP93_Status_t
EIP93_ARM_PacketGet(
        EIP93_IOArea_t * const IOArea_p,
        EIP93_ARM_ResultDescriptor_t *  ResDescriptors_p,
        const unsigned int ResDescriptorLimit,
        unsigned int * const DoneCount_p);


/*----------------------------------------------------------------------------
 * EIP93_ARM_PacketExternalGet
 *
 * Should be called as a result of a corresponding EIP93_ARM_PacketGet()
 * call on an external Host. This function has to be called instead of
 * EIP93_ARM_PacketGet() in case another, external Host is used for external
 * completion.
 *
 * (input)ExternalDoneCount:Number of descriptors actually emptied from the
 * result ring by the external host.
 */
EIP93_Status_t
EIP93_ARM_PacketExternalGet(
        EIP93_IOArea_t * const IOArea_p,
        const unsigned int ExternalDoneCount);

#endif 


