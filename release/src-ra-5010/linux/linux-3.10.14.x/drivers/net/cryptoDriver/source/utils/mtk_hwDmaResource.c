
#include "mtk_cHwPci.h"

#include "mtk_baseDefs.h"
#include "mtk_cLib.h"
#include "mtk_hwDmaAccess.h"
#include "mtk_hwAccess.h"

#include <linux/slab.h>         // kmalloc, kfree
#include <linux/dma-mapping.h>  // dma_sync_single_for_cpu

#ifdef HWPAL_LOCK_SLEEPABLE
#include <linux/mutex.h>        // mutex_*
#else
#include <linux/spinlock.h>     // spinlock_*
#endif


/*

 Requirements on the records:
  - pre-allocated array of records
  - valid between Create and Destroy
  - re-use on a least-recently-used basis to make sure accidental continued
    use after destroy does not cause crashes, allowing us to detect the
    situation instead of crashing quickly.
 
 Requirements on the handles:
  - one handle per record
  - valid between Create and Destroy
  - quickly find the ptr-to-record belonging to the handle
  - detect continued use of a handle after Destroy
  - caller-hidden admin/status, thus not inside the record
  - report leaking handles upon exit
 
 Solution:
  - handle cannot be a record number (no post-destroy use detection possible)
  - recnr/destroyed in separate memory location for each handle: Handles_p
  - Array of records: Records_p
  - free locations in Array1: Freelist1 (FreeHandles)
  - free record numbers list: Freelist2 (FreeRecords)
 */

typedef struct
{
    int ReadIndex;
    int WriteIndex;
    int * Nrs_p;
} HWPAL_FreeList_t;

static int HandlesCount = 0;        // remainder are valid only when this is != 0
static int * Handles_p;
static HWPAL_DMAResource_Record_t * Records_p;
static HWPAL_FreeList_t FreeHandles;
static HWPAL_FreeList_t FreeRecords;


#ifdef HWPAL_LOCK_SLEEPABLE
static struct mutex HWPAL_Lock;
#else
static spinlock_t HWPAL_SpinLock;
#endif

#define HWPAL_RECNR_DESTROYED  -1


/*----------------------------------------------------------------------------
 * HWPAL_FreeList_Get
 *
 * Gets the next entry from the freelist. Returns -1 when the list is empty.
 */
static inline int
HWPAL_FreeList_Get(
        HWPAL_FreeList_t * const List_p)
{
    int Nr = -1;
    int ReadIndex_Updated = List_p->ReadIndex + 1;

    if (ReadIndex_Updated >= HandlesCount)
        ReadIndex_Updated = 0;

    // if post-increment ReadIndex == WriteIndex, the list is empty
    if (ReadIndex_Updated != List_p->WriteIndex)
    {
        // grab the next number
        Nr = List_p->Nrs_p[List_p->ReadIndex];
        List_p->ReadIndex = ReadIndex_Updated;
    }

    return Nr;
}


/*----------------------------------------------------------------------------
 * HWPAL_FreeList_Add
 *
 * Adds an entry to the freelist.
 */
static inline void
HWPAL_FreeList_Add(
        HWPAL_FreeList_t * const List_p,
        int Nr)
{
    if (List_p->WriteIndex == List_p->ReadIndex)
    {
        printk(
            "HWPAL_FreeList_Add: "
            "Attempt to add value %d to full list\n",
            Nr);
        return;
    }

    if (Nr < 0 || Nr >= HandlesCount)
    {
        printk(
            "HWPAL_FreeList_Add: "
            "Attempt to put invalid value: %d\n",
            Nr);
        return;
    }

    {
        int WriteIndex_Updated = List_p->WriteIndex + 1;
        if (WriteIndex_Updated >= HandlesCount)
            WriteIndex_Updated = 0;

        // store the number
        List_p->Nrs_p[List_p->WriteIndex] = Nr;
        List_p->WriteIndex = WriteIndex_Updated;
    }
}


/*----------------------------------------------------------------------------
 * HWPAL_Hexdump
 *
 * This function hex-dumps an array of uint32_t.
 */
#ifdef HWPAL_TRACE_DMARESOURCE_READWRITE

static inline void
HWPAL_DMAResource_Hexdump(
        const char * ArrayName_p,
        const uint16_t * Handle_p,
        const unsigned int Offset,
        const uint32_t * WordArray_p,
        const unsigned int WordCount,
        bool fSwapEndianness)
{
    unsigned int i;

    printk(
        "%s: "
        "Handle = %p: "
        "byte offsets %u - %u "
        "(swap=%d)\n"
        ArrayName_p,
        Handle_p,
        Offset,
        Offset + WordCount*4 - 1,
        fSwapEndianess);

    for (i = 1; i <= WordCount; i++)
    {
        uint32_t Value = WordArray_p[i - 1];

        if (fSwapEndianness)
            Value = HWPAL_SwapEndian32(Value);

        printk(" 0x%08x", Value);

        if ((i & 7) == 0)
            printk("\n  ");
    }

    if ((WordCount & 7) != 0)
        printk("\n");
}
#endif


bool
HWPAL_DMAResource_Init(
        const unsigned int MaxHandles)
{

#ifdef HWPAL_LOCK_SLEEPABLE
    mutex_init(&HWPAL_Lock);
#else
    spin_lock_init(&HWPAL_SpinLock);
#endif

    // already initialized?
    if (HandlesCount != 0)
        return false;

    // this implementation only supports MaxHandles != 0
    if (MaxHandles == 0)
        return false;

    Records_p = kmalloc(MaxHandles * sizeof(HWPAL_DMAResource_Record_t), GFP_KERNEL);
    Handles_p = kmalloc(MaxHandles * sizeof(int), GFP_KERNEL);
    FreeHandles.Nrs_p = kmalloc(MaxHandles * sizeof(int), GFP_KERNEL);
    FreeRecords.Nrs_p = kmalloc(MaxHandles * sizeof(int), GFP_KERNEL);

    // if any allocation failed, free the whole lot
    if (Records_p == NULL ||
        Handles_p == NULL ||
        FreeHandles.Nrs_p == NULL ||
        FreeRecords.Nrs_p == NULL)
    {
        if (Records_p)
            kfree(Records_p);

        if (Handles_p)
            kfree(Handles_p);

        if (FreeHandles.Nrs_p)
            kfree(FreeHandles.Nrs_p);

        if (FreeRecords.Nrs_p)
            kfree(FreeRecords.Nrs_p);

        Records_p = NULL;
        Handles_p = NULL;
        FreeHandles.Nrs_p = NULL;
        FreeRecords.Nrs_p = NULL;

        return false;
    }


    {
        unsigned int i;

        for (i = 0; i < MaxHandles; i++)
        {
            Handles_p[i] = HWPAL_RECNR_DESTROYED;
            FreeHandles.Nrs_p[i] = MaxHandles - 1 - i;
            FreeRecords.Nrs_p[i] = i;
        }

        FreeHandles.ReadIndex = 0;
        FreeHandles.WriteIndex = 0;

        FreeRecords.ReadIndex = 0;
        FreeRecords.WriteIndex = 0;
    }

    HandlesCount = MaxHandles;

    return true;
}



void
HWPAL_DMAResource_UnInit(void)
{
    // exit if not initialized
    if (HandlesCount == 0)
        return;


    HandlesCount = 0;

    kfree(FreeHandles.Nrs_p);
    kfree(FreeRecords.Nrs_p);
    kfree(Handles_p);
    kfree(Records_p);

    FreeHandles.Nrs_p = NULL;
    FreeRecords.Nrs_p = NULL;
    Handles_p = NULL;
    Records_p = NULL;
}


/*----------------------------------------------------------------------------
 * HWPAL_DMAResource_Create
 */
HWPAL_DMAResource_Handle_t
HWPAL_DMAResource_Create(void)
{
#ifndef HWPAL_LOCK_SLEEPABLE
    unsigned long flags;
#endif
    int HandleNr;
    int RecNr = 0;

    // return NULL when not initialized
    if (HandlesCount == 0)
        return NULL;

#ifdef HWPAL_LOCK_SLEEPABLE
    mutex_lock(&HWPAL_Lock);
#else
    spin_lock_irqsave(&HWPAL_SpinLock, flags);
#endif

    HandleNr = HWPAL_FreeList_Get(&FreeHandles);
    if (HandleNr != -1)
    {
        RecNr = HWPAL_FreeList_Get(&FreeRecords);
        if (RecNr == -1)
        {
            HWPAL_FreeList_Add(&FreeHandles, HandleNr);
            HandleNr = -1;
        }
    }

#ifdef HWPAL_LOCK_SLEEPABLE
    mutex_unlock(&HWPAL_Lock);
#else
    spin_unlock_irqrestore(&HWPAL_SpinLock, flags);
#endif

    // return NULL when reservation failed
    if (HandleNr == -1)
        return NULL;

    // initialize the record
    {
        HWPAL_DMAResource_Record_t * Rec_p = Records_p + RecNr;
        memset(Rec_p, 0, sizeof(HWPAL_DMAResource_Record_t));
    }

    // initialize the handle
    Handles_p[HandleNr] = RecNr;

    // fill in the handle position
    return Handles_p + HandleNr;
}


/*----------------------------------------------------------------------------
 * HWPAL_DMAResource_Destroy
 */
void
HWPAL_DMAResource_Destroy(
        HWPAL_DMAResource_Handle_t Handle)
{
    if (HWPAL_DMAResource_IsValidHandle(Handle))
    {
        int * p = (int *)Handle;
        int RecNr = *p;

        if (RecNr >= 0 &&
            RecNr < HandlesCount)
        {
#ifndef HWPAL_LOCK_SLEEPABLE
            unsigned long flags;
#endif
            int HandleNr = p - Handles_p;

            // note handle is no longer value
            *p = HWPAL_RECNR_DESTROYED;

#ifdef HWPAL_LOCK_SLEEPABLE
            mutex_lock(&HWPAL_Lock);
#else
            spin_lock_irqsave(&HWPAL_SpinLock, flags);
#endif

            // add the HandleNr and RecNr to respective LRU lists
            HWPAL_FreeList_Add(&FreeHandles, HandleNr);
            HWPAL_FreeList_Add(&FreeRecords, RecNr);

#ifdef HWPAL_LOCK_SLEEPABLE
            mutex_unlock(&HWPAL_Lock);
#else
            spin_unlock_irqrestore(&HWPAL_SpinLock, flags);
#endif
        }
        else
        {
            printk(
                "HWPAL_DMAResource_Destroy: "
                "Handle %p was already destroyed\n",
                Handle);
        }
    }
    else
    {
        printk(
            "HWPAL_DMAResource_Destroy: "
            "Invalid handle %p\n",
            Handle);
    }
}


/*----------------------------------------------------------------------------
 * HWPAL_DMAResource_IsValidHandle
 */
bool
HWPAL_DMAResource_IsValidHandle(
        HWPAL_DMAResource_Handle_t Handle)
{
    int * p = (int *)Handle;

    if (p < Handles_p ||
        p >= Handles_p + HandlesCount)
    {
        return false;
    }

    // check that the handle has not been destroyed yet
    if (*p < 0 ||
        *p >= HandlesCount)
    {
        return false;
    }

    return true;
}


/*----------------------------------------------------------------------------
 * HWPAL_DMAResource_Handle2RecordPtr
 */
HWPAL_DMAResource_Record_t *
HWPAL_DMAResource_Handle2RecordPtr(
        HWPAL_DMAResource_Handle_t Handle)
{
    int * p = (int *)Handle;
   
    if (p != NULL)
    {
        int RecNr = *p;

        if (RecNr >= 0 &&
            RecNr < HandlesCount)
        {
            return Records_p + RecNr;           // ## RETURN ##
        }
    }

    return NULL;
}


/*----------------------------------------------------------------------------
 * HWPAL_DMAResource_Read32
 *
 * This function can be used to read one 32bit word from the DMA Resource
 * buffer.
 * If required (decided by HWPAL_DMAResource_Record_t.device.fSwapEndianess),
 * on the fly endianess swapping of the value read will be performed before it
 * is returned to the caller.
 *
 * Handle (input)
 *     Handle for the DMA Resource to access.
 *
 * WordOffset (input)
 *     Offset in 32bit words, from the start of the DMA Resource to read from.
 *
 * Return Value
 *     The value read.
 *
 * When the Handle and WordOffset parameters are not valid, the implementation
 * will return an unspecified value.
 */
uint32_t
HWPAL_DMAResource_Read32(
        HWPAL_DMAResource_Handle_t Handle,
        const uint32_t WordOffset)
{
    HWPAL_DMAResource_Record_t * Rec_p;
    
    Rec_p = HWPAL_DMAResource_Handle2RecordPtr(Handle);
    if (Rec_p == NULL)
    {
        printk(
            "HWPAL_DMAResource_Read32: "
            "Invalid handle %p\n",
            Handle);

        return 0;
    }

    if (WordOffset * 4 >= Rec_p->host.BufferSize)
    {
        printk(
            "HWPAL_DMAResource_Read32: "
            "Invalid WordOffset %u for Handle %p\n",
            WordOffset,
            Handle);

        return 0;
    }

    {
        uint32_t * Address_p = Rec_p->host.HostAddr_p;
        uint32_t Value = Address_p[WordOffset];

        // swap endianness, if required
        if (Rec_p->device.fSwapEndianess)
            Value = HWPAL_SwapEndian32(Value);

#ifdef HWPAL_TRACE_DMARESOURCE_READ
        printk(
            "HWPAL_DMAResource_Read32:  "
            "(handle %p) "
            "0x%08x = [%u] "
            "(swap=%d)\n",
            Handle,
            Value,
            WordOffset,
            Rec_p->device.fSwapEndianess);
#endif

        return Value;
    }
}


/*----------------------------------------------------------------------------
 * HWPAL_DMAResource_Write32
 *
 * This function can be used to write one 32bit word to the DMA Resource.
 * If required (decided by HWPAL_DMAResource_Record_t.device.fSwapEndianess),
 * on the fly endianess swapping of the value to be written will be performed.
 *
 * Handle (input)
 *     Handle for the DMA Resource to access.
 *
 * WordOffset (input)
 *     Offset in 32bit words, from the start of the DMA Resource to write to.
 *
 * Value (input)
 *     The 32bit value to write.
 *
 * Return Value
 *     None
 *
 * The write can only be successful when the Handle and WordOffset
 * parameters are valid.
 */
void
HWPAL_DMAResource_Write32(
        HWPAL_DMAResource_Handle_t Handle,
        const unsigned int WordOffset,
        const uint32_t Value)
{
    HWPAL_DMAResource_Record_t * Rec_p;
  
    Rec_p = HWPAL_DMAResource_Handle2RecordPtr(Handle);
    if (Rec_p == NULL)
    {
        printk(
            "HWPAL_DMAResource_Write32: "
            "Invalid handle %p\n",
            Handle);

        return;
    }

    if (WordOffset * 4 >= Rec_p->host.BufferSize)
    {
        printk(
            "HWPAL_DMAResource_Write32: "
            "Invalid WordOffset %u for Handle %p\n",
            WordOffset,
            Handle);

        return;
    }

#ifdef HWPAL_TRACE_DMARESOURCE_WRITE
    printk(
        "HWPAL_DMAResource_Write32: "
        "(handle %p) "
        "[%u] = 0x%08x "
        "(swap=%d)\n",
        Handle,
        WordOffset,
        Value,
        Rec_p->device.fSwapEndianess);
#endif

    {
        uint32_t * Address_p = Rec_p->host.HostAddr_p;
        uint32_t WriteValue = Value;

        // swap endianness, if required
        if (Rec_p->device.fSwapEndianess)
            WriteValue = HWPAL_SwapEndian32(WriteValue);

        Address_p[WordOffset] = WriteValue;
    }
}


/*----------------------------------------------------------------------------
 * HWPAL_DMAResource_Read32Array
 *
 * This function perform the same task as HWPAL_DMAResource_Read32 for a
 * consecutive array of 32bit words.
 *
 * See HWPAL_DMAResource_Read32 for a more detailed description.
 *
 * Handle (input)
 *     Handle for the DMA Resource to access.
 *
 * WordOffset (input)
 *     Offset in 32bit words, from the start of the DMA Resource to start
 *     reading from.
 *
 * WordCount (input)
 *     The number of 32bit words to transfer.
 *
 * Values_p (input)
 *     Memory location to write the retrieved values to.
 *     Note the ability to let Values_p point inside the DMAResource that is
 *     being read from, allowing for in-place endianess conversion.
 *
 * Return Value
 *     None.
 *
 * The read can only be successful when the Handle and WordOffset
 * parameters are valid.
 */
void
HWPAL_DMAResource_Read32Array(
        HWPAL_DMAResource_Handle_t Handle,
        const unsigned int WordOffset,
        const unsigned int WordCount,
        uint32_t * Values_p)
{
    HWPAL_DMAResource_Record_t * Rec_p;

    if (WordCount == 0)
        return;

    Rec_p = HWPAL_DMAResource_Handle2RecordPtr(Handle);
    if (Rec_p == NULL)
    {
        printk(
            "HWPAL_DMAResource_Read32Array: "
            "Invalid handle %p\n",
            Handle);
        return;
    }

    if ((WordOffset + WordCount - 1) * 4 >= Rec_p->host.BufferSize)
    {
        printk(
            "HWPAL_DMAResource_Read32Array: "
            "Invalid range: %u - %u\n",
            WordOffset,
            WordOffset + WordCount - 1);
        return;
    }

    {
        uint32_t * Address_p = Rec_p->host.HostAddr_p;
        unsigned int i;

        for (i = 0; i < WordCount; i++)
        {
            uint32_t Value = Address_p[WordOffset + i];
            
            // swap endianness, if required
            if (Rec_p->device.fSwapEndianess)
                Value = HWPAL_SwapEndian32(Value);
                
            Values_p[i] = Value;
        } // for
    }

#ifdef HWPAL_TRACE_DMARESOURCE_READ
    {
        uint32_t * Address_p = Rec_p->host.HostAddr_p;
        if (Values_p == Address_p + WordOffset)
        {
            printk(
                "HWPAL_DMAResource_Read32Array: "
                "(handle %p) "
                "[%u..%u] IN-PLACE "
                "(swap=%d)\n",
                Handle,
                WordOffset,
                WordOffset + WordCount - 1,
                Rec_p->device.fSwapEndianess);
        }
        else
        {
            printk(
                "HWPAL_DMAResource_Read32Array: "
                "(handle %p) "
                "[%u..%u] "
                "(swap=%d)\n",
                Handle,
                WordOffset,
                WordOffset + WordCount - 1,
                Rec_p->device.fSwapEndianess);
        }
    }
#endif
}


/*----------------------------------------------------------------------------
 * HWPAL_DMAResource_Write32Array
 *
 * This function perform the same task as HWPAL_DMAResource_Write32 for a
 * consecutive array of 32bit words.
 *
 * See HWPAL_DMAResource_Write32 for a more detailed description.
 *
 * Handle (input)
 *     Handle for the DMA Resource to access.
 *
 * WordOffset (input)
 *     Offset in 32bit words, from the start of the DMA Resource to start
 *     writing from.
 *
 * WordCount (input)
 *     The number of 32bit words to transfer.
 *
 * Values_p (input)
 *     Pointer to the memory where the values to be written are located.
 *     Note the ability to let Values_p point inside the DMAResource that is
 *     being written to, allowing for in-place endianess conversion.
 *
 * Return Value
 *     None.
 *
 * The write can only be successful when the Handle and WordOffset
 * parameters are valid.
 */
void
HWPAL_DMAResource_Write32Array(
        HWPAL_DMAResource_Handle_t Handle,
        const unsigned int WordOffset,
        const unsigned int WordCount,
        const uint32_t * Values_p)
{
    HWPAL_DMAResource_Record_t * Rec_p;

    if (WordCount == 0)
        return;

    Rec_p = HWPAL_DMAResource_Handle2RecordPtr(Handle);
    if (Rec_p == NULL)
    {
        printk(
            "HWPAL_DMAResource_Write32Array: "
            "Invalid handle %p\n",
            Handle);
        return;
    }

    if ((WordOffset + WordCount - 1) * 4 >= Rec_p->host.BufferSize)
    {
        printk(
            "HWPAL_DMAResource_Write32Array: "
            "Invalid range: %u - %u\n",
            WordOffset,
            WordOffset + WordCount - 1);
        return;
    }

    {
        uint32_t * Address_p = Rec_p->host.HostAddr_p;
        unsigned int i;

        for (i = 0; i < WordCount; i++)
        {
            uint32_t Value = Values_p[i];
            
            // swap endianness, if required
            if (Rec_p->device.fSwapEndianess)
                Value = HWPAL_SwapEndian32(Value);

            Address_p[WordOffset + i] = Value;
        } // for
    }

#ifdef HWPAL_TRACE_DMARESOURCE_WRITE
    {
        uint32_t * Address_p = Rec_p->host.HostAddr_p;
        if (Values_p == Address_p + WordOffset)
        {
            printk(
                "HWPAL_DMAResource_Write32Array: "
                "(handle %p) "
                "[%u..%u] IN-PLACE "
                "(swap=%d)\n",
                Handle,
                WordOffset,
                WordOffset + WordCount - 1,
                Rec_p->device.fSwapEndianess);
        }
        else
        {
            printk(
                "HWPAL_DMAResource_Write32Array: "
                "(handle %p) "
                "[%u..%u] "
                "(swap=%d)\n",
                Handle,
                WordOffset,
                WordOffset + WordCount - 1,
                Rec_p->device.fSwapEndianess);
        }
    }
#endif
}


/*----------------------------------------------------------------------------
 * HWPAL_DMAResource_PreDMA
 */
void
HWPAL_DMAResource_PreDMA(
        HWPAL_DMAResource_Handle_t Handle,
        const unsigned int ByteOffset,
        const unsigned int ByteCount)
{
    HWPAL_DMAResource_Record_t * Rec_p;
    dma_addr_t pa;
    size_t size;

    Rec_p = HWPAL_DMAResource_Handle2RecordPtr(Handle);
    if (Rec_p == NULL)
    {
        printk(
            "HWPAL_DMAResource_PreDMA: "
            "Invalid handle %p\n",
            Handle);

        return;
    }

    // dma_sync_single_for_cpu wants the bus address
    pa = Rec_p->device.DeviceAddr32;
    size = 0;

    if (ByteCount == 0)
    {
#ifdef HWPAL_TRACE_DMARESOURCE_PREPOSTDMA
        printk(
            "HWPAL_DMAResource_PreDMA: "
            "Handle=%p, "
            "Range=ALL (%u, %u)\n",
            Handle,
            ByteOffset,
            ByteCount);
#endif
        size = Rec_p->host.BufferSize;
    }
    else
    {
#ifdef HWPAL_TRACE_DMARESOURCE_PREPOSTDMA
        printk(
                "HWPAL_DMAResource_PreDMA: "
                "Handle=%p, "
                "Range=%u-%u\n",
                Handle,
                ByteOffset,
                ByteOffset + ByteCount -1);
#endif
        pa += ByteOffset;
        size = ByteCount;
    }

#ifdef HWPAL_TRACE_DMARESOURCE_PREPOSTDMA
    printk(
        "HWPAL_DMAResource_PreDMA: "
        "pa=0x%08x, size=0x%08x\n",
        pa, size);
#endif

    dma_sync_single_for_cpu(NULL, pa, size, DMA_TO_DEVICE);
}


/*----------------------------------------------------------------------------
 * HWPAL_DMAResource_PostDMA
 */
void
HWPAL_DMAResource_PostDMA(
        HWPAL_DMAResource_Handle_t Handle,
        const unsigned int ByteOffset,
        const unsigned int ByteCount)
{
    HWPAL_DMAResource_Record_t * Rec_p;
    dma_addr_t pa;
    size_t size;

    Rec_p = HWPAL_DMAResource_Handle2RecordPtr(Handle);
    if (Rec_p == NULL)
    {
        printk(
            "HWPAL_DMAResource_PostDMA: "
            "Invalid handle %p\n",
            Handle);

        return;
    }

    // dma_sync_single_for_cpu wants the bus address
    pa = Rec_p->device.DeviceAddr32;
    size = 0;

    if (ByteCount == 0)
    {
#ifdef HWPAL_TRACE_DMARESOURCE_PREPOSTDMA
        printk(
                "HWPAL_DMAResource_PostDMA: "
                "Handle=%p, "
                "Range=ALL (%u, %u)\n",
                Handle,
                ByteOffset,
                ByteCount);
#endif
        size = Rec_p->host.BufferSize;
    }
    else
    {
#ifdef HWPAL_TRACE_DMARESOURCE_PREPOSTDMA
        printk(
                "HWPAL_DMAResource_PostDMA: "
                "Handle=%p, "
                "Range=%u-%u\n",
                Handle,
                ByteOffset,
                ByteOffset + ByteCount -1);
#endif
        pa += ByteOffset;
        size = ByteCount;
    }

#ifdef HWPAL_TRACE_DMARESOURCE_PREPOSTDMA
    printk(
        "HWPAL_DMAResource_PostDMA: "
        "pa=0x%08x, size=0x%08x\n",
        pa, size);
#endif

    dma_sync_single_for_cpu(NULL, pa, size, DMA_FROM_DEVICE);
}

