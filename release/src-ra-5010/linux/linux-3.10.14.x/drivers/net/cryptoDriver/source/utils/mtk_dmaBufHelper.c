
#include "mtk_baseDefs.h"
#include "mtk_dmaBuf.h"
#include "mtk_hwDmaAccess.h"
#include "mtk_cLib.h"               // memcmp
#include "mtk_AdapterInternal.h"

#ifndef ADAPTER_USER_DOMAIN_BUILD
// kernel domain headers.
#include <linux/slab.h>         // kmalloc
#include <linux/hardirq.h>      // in_atomic
#include <linux/dma-mapping.h>  // dma_get_cache_alignment
#else
// user domain headers
#include <stdlib.h>              // malloc
#endif  // ADAPTER_USER_DOMAIN_BUILD

// to check DMAResource record consistency
#define ADAPTER_DMABUF_MAGIC 0xD71A64

#define ADAPTER_DMABUF_MEMORYBANK_INTERNALLOC 42

const DMABuf_Handle_t Adapter_DMABuf_NullHandle = { 0 };

#ifdef RT_EIP93_DRIVER
#define   K1_TO_PHYSICAL(x) (((u32)(x)) & 0x1fffffff)
#endif

/*----------------------------------------------------------------------------
 * Adapter_DMABuf_IsSameHandle
 */
bool
Adapter_DMABuf_IsSameHandle(
        const DMABuf_Handle_t * const Handle1_p,
        const DMABuf_Handle_t * const Handle2_p)
{
    if (memcmp(Handle1_p, Handle2_p, sizeof(DMABuf_Handle_t)) == 0)
        return true;

    return false;
}


/*----------------------------------------------------------------------------
 * Adapter_DMABuf_IsValidHandle
 */
bool
Adapter_DMABuf_IsValidHandle(
        DMABuf_Handle_t Handle)
{
    HWPAL_DMAResource_Handle_t DMAHandle;
    HWPAL_DMAResource_Record_t * Rec_p;

    if (Handle.p == NULL)
        return false;

    DMAHandle = Adapter_DMABuf_Handle2DMAResourceHandle( Handle);

    Rec_p = HWPAL_DMAResource_Handle2RecordPtr(DMAHandle);

    if (Rec_p == NULL)
        return false;

    if (Rec_p->Magic != ADAPTER_DMABUF_MAGIC)
        return false;

    return true;
}


/*----------------------------------------------------------------------------
 * Adapter_DMABuf_Handle2DMAResourceHandle
 */
HWPAL_DMAResource_Handle_t
Adapter_DMABuf_Handle2DMAResourceHandle(
        DMABuf_Handle_t Handle)
{
    return (HWPAL_DMAResource_Handle_t)Handle.p;
}


/*----------------------------------------------------------------------------
 * Adapter_DMABuf_IsForeignAllocated
 */
bool
Adapter_DMABuf_IsForeignAllocated(
        DMABuf_Handle_t Handle)
{
    HWPAL_DMAResource_Handle_t DMAHandle =
            Adapter_DMABuf_Handle2DMAResourceHandle(Handle);

    if (HWPAL_DMAResource_IsValidHandle(DMAHandle))
    {
        HWPAL_DMAResource_Record_t * const Rec_p =
                HWPAL_DMAResource_Handle2RecordPtr(DMAHandle);

        // Buffer is allocated using DMABuf_Alloc()
        if (Rec_p->alloc.MemoryBank == ADAPTER_DMABUF_MEMORYBANK_INTERNALLOC)
            return false;
    }

    // buffer is not allocated using DMABuf_Alloc(), hence Foreign
    return true;
}


/*----------------------------------------------------------------------------
 * DMABuf_Alloc
 *
 * Allocate a buffer of requested size that can be used for device DMA.
 *
 * RequestedProperties
 *     Requested properties of the buffer that will be allocated, including
 *     the size, start address alignment, etc. See above.
 *
 * Buffer_p (output)
 *     Pointer to the memory location where the address of the buffer will be
 *     written by this function when allocation is successful. This address
 *     can then be used to access the driver on the host in the domain of the
 *     driver.
 *
 * Handle_p (output)
 *     Pointer to the memory location when the handle will be returned.
 *
 * Return Values
 *     DMABUF_STATUS_OK: Success, Handle_p was written.
 *     DMABUF_ERROR_BAD_ARGUMENT
 *     DMABUF_ERROR_OUT_OF_MEMORY: Failed to allocate a buffer.
 */
DMABuf_Status_t
DMABuf_Alloc(
        const DMABuf_Properties_t RequestedProperties,
        DMABuf_HostAddress_t * const Buffer_p,
        DMABuf_Handle_t * const Handle_p)
{
    HWPAL_DMAResource_Handle_t DMAHandle;
    HWPAL_DMAResource_Record_t * Rec_p;

#ifndef ADAPTER_USER_DOMAIN_BUILD
    unsigned int AlignTo = dma_get_cache_alignment();
#else
    unsigned int AlignTo = 1; // Not needed in user domain
#endif  // ADAPTER_USER_DOMAIN_BUILD


    if (Handle_p == NULL ||
        Buffer_p == NULL)
    {
        return DMABUF_ERROR_BAD_ARGUMENT;
    }

    // initialize the output parameters
    Handle_p->p = NULL;
    Buffer_p->p = NULL;

    // validate the properties
    if (RequestedProperties.Size == 0)
        return DMABUF_ERROR_BAD_ARGUMENT;

    // we support up to 1 megabyte buffers
    if (RequestedProperties.Size >= 1*1024*1024)
        return DMABUF_ERROR_BAD_ARGUMENT;

    // alignment must be a power of two, up to 32
    if (RequestedProperties.Alignment != 1 &&
        RequestedProperties.Alignment != 2 &&
        RequestedProperties.Alignment != 4 &&
        RequestedProperties.Alignment != 8 &&
        RequestedProperties.Alignment != 16 &&
        RequestedProperties.Alignment != 32 &&
        RequestedProperties.Alignment != 64 &&
        RequestedProperties.Alignment != 128)
    {
        return DMABUF_ERROR_BAD_ARGUMENT;
    }

    if (RequestedProperties.Alignment > AlignTo)
        AlignTo = RequestedProperties.Alignment;

    // we only support one memory bank: number zero
    if (RequestedProperties.Bank != 0)
        return DMABUF_ERROR_BAD_ARGUMENT;

    // create a record
    DMAHandle = HWPAL_DMAResource_Create();
    if (DMAHandle == NULL)
        return DMABUF_ERROR_OUT_OF_MEMORY;

    Rec_p = HWPAL_DMAResource_Handle2RecordPtr(DMAHandle);
    if (Rec_p == NULL)
    {
        // panic...
        goto DESTROY_HANDLE;
    }

    // allocate the memory
    {
        unsigned int n;
        void * p;
#ifdef RT_EIP93_DRIVER
        dma_addr_t dma_handle;
#endif
        n = AlignTo + RequestedProperties.Size;  

        {
#ifndef ADAPTER_USER_DOMAIN_BUILD
#ifdef RT_EIP93_DRIVER
            int flags=0;
#else
            int flags = GFP_DMA;
#endif

            if (in_atomic())
                flags |= GFP_ATOMIC;    // non-sleepable
            else
                flags |= GFP_KERNEL;    // sleepable

#ifdef RT_EIP93_DRIVER
            p = dma_alloc_coherent(NULL, n, &dma_handle, flags);
#else
            p = kmalloc(n, flags);
#endif
#else
            p = malloc(n);
#endif  // ADAPTER_USER_DOMAIN_BUILD
        }

        if (p == NULL)
            goto DESTROY_HANDLE;

        // fill in the record fields
        Rec_p->Magic = ADAPTER_DMABUF_MAGIC;

        Rec_p->alloc.AllocatedAddr_p = p;
        Rec_p->alloc.AllocatedSize = n;
        Rec_p->alloc.MemoryBank = ADAPTER_DMABUF_MEMORYBANK_INTERNALLOC;
        Rec_p->alloc.AllocatorRef = ADAPTER_DMABUF_ALLOCATORREF_KMALLOC;
        // used by AddrTrans

        Rec_p->host.fCached = true;
        Rec_p->host.Alignment = (uint8_t)AlignTo;
        Rec_p->host.HostAddr_p = p;
        Rec_p->host.BufferSize = RequestedProperties.Size;
        // note: not the allocated "n"

        // align the address
        {
            // create an alignment mask
            unsigned int Mask = AlignTo - 1;
            unsigned int A = (unsigned int)Rec_p->alloc.AllocatedAddr_p;
            unsigned int MaskedBits = A & Mask;
            if (MaskedBits != 0)
            {
                // calculate the alignment error
                unsigned int Err = AlignTo - MaskedBits;
                uint8_t * p = Rec_p->host.HostAddr_p;
                p += Err;
                Rec_p->host.HostAddr_p = p;
            }
        }       
    }

    // set the output parameters
    Handle_p->p = DMAHandle;
    Buffer_p->p = Rec_p->host.HostAddr_p;

    return DMABUF_STATUS_OK;

DESTROY_HANDLE:
    HWPAL_DMAResource_Destroy(DMAHandle);
    return DMABUF_ERROR_OUT_OF_MEMORY;
}
EXPORT_SYMBOL(DMABuf_Alloc); /*for integration*/

/*----------------------------------------------------------------------------
 * DMABuf_Register
 *
 * This function must be used to register an "alien" buffer that was allocated
 * somewhere else. The caller guarantees that this buffer can be used for DMA.
 *
 * ActualProperties (input)
 *     Properties that describe the buffer that is being registered.
 *
 * Buffer_p (input)
 *     Pointer to the buffer. This pointer must be valid to use on the host
 *     in the domain of the driver.
 *
 * Alternative_p (input)
 *     Some allocators return two addresses. This parameter can be used to
 *     pass this second address to the driver. The type is pointer to ensure
 *     it is always large enough to hold a system address, also in LP64
 *     architecture. Set to NULL if not used.
 *
 * AllocatorRef (input)
 *     Number to describe the source of this buffer. The exact numbers
 *     supported is implementation specitic. This provides some flexibility
 *     for a specific implementation to support a number of "alien" buffers
 *     from different allocator and propertly interpret and use the
 *     Alternative_p parameter when translating the address to the device
 *     memory map. Set to zero if not used.
 *
 * Handle_p (output)
 *     Pointer to the memory location when the handle will be returned.
 *
 * Return Values
 *     DMABUF_STATUS_OK: Success, Handle_p was written.
 *     DMABUF_ERROR_BAD_ARGUMENT
 */
DMABuf_Status_t
DMABuf_Register(
        const DMABuf_Properties_t ActualProperties,
        void * Buffer_p,
        void * Alternative_p,
        const char AllocatorRef,
        DMABuf_Handle_t * const Handle_p)
{
    HWPAL_DMAResource_Handle_t DMAHandle;
    HWPAL_DMAResource_Record_t * Rec_p;

    if (Handle_p == NULL ||
        Buffer_p == NULL)
    {
        return DMABUF_ERROR_BAD_ARGUMENT;
    }

    // initialize the output parameter
    Handle_p->p = NULL;

    // validate the properties
    if (ActualProperties.Size == 0)
        return DMABUF_ERROR_BAD_ARGUMENT;

    // alignment must be a power of two, up to 32
    if (ActualProperties.Alignment != 1 &&
        ActualProperties.Alignment != 2 &&
        ActualProperties.Alignment != 4 &&
        ActualProperties.Alignment != 8 &&
        ActualProperties.Alignment != 16 &&
        ActualProperties.Alignment != 32)
    {
        return DMABUF_ERROR_BAD_ARGUMENT;
    }

    if (ActualProperties.Bank == ADAPTER_DMABUF_MEMORYBANK_INTERNALLOC)
        return DMABUF_ERROR_BAD_ARGUMENT;

    // create a record
    DMAHandle = HWPAL_DMAResource_Create();
    if (DMAHandle == NULL)
        return DMABUF_ERROR_OUT_OF_MEMORY;

    Rec_p = HWPAL_DMAResource_Handle2RecordPtr(DMAHandle);
    if (Rec_p == NULL)
    {
        // panic...
        goto DESTROY_HANDLE;
    }

    // allocate the memory
    {
        // fill in the record fields
        Rec_p->Magic = ADAPTER_DMABUF_MAGIC;

        Rec_p->alloc.AllocatedAddr_p = Buffer_p;
        Rec_p->alloc.AllocatedSize = ActualProperties.Size;
        Rec_p->alloc.Alternative_p = Alternative_p;
        Rec_p->alloc.AllocatorRef = AllocatorRef;
        Rec_p->alloc.MemoryBank = ActualProperties.Bank;

        Rec_p->host.fCached = ActualProperties.fCached;

        Rec_p->host.Alignment = ActualProperties.Alignment;
        Rec_p->host.HostAddr_p = Rec_p->alloc.AllocatedAddr_p;
        Rec_p->host.BufferSize = Rec_p->alloc.AllocatedSize;
    }

    // set the output parameters
    Handle_p->p = DMAHandle;

    return DMABUF_STATUS_OK;

DESTROY_HANDLE:
    HWPAL_DMAResource_Destroy(DMAHandle);
    return DMABUF_ERROR_OUT_OF_MEMORY;
}
EXPORT_SYMBOL(DMABuf_Register); /*for integration*/

/*----------------------------------------------------------------------------
 * DMABuf_Release
 *
 * This function will close the handle that was returned by DMABuf_Alloc or
 * DMABuf_Register, meaning it must not be used anymore.
 * If the buffer was allocated through DMABuf_Alloc, this function will also
 * free the buffer, meaning it must not be accessed anymore.
 *
 * Handle (input)
 *     The handle that may be released.
 *
 * Return Values
 *     DMABUF_STATUS_OK
 *     DMABUF_ERROR_INVALID_HANDLE
 */
DMABuf_Status_t
DMABuf_Release(
        DMABuf_Handle_t Handle)
{
    HWPAL_DMAResource_Handle_t DMAHandle = Handle.p;

    if (HWPAL_DMAResource_IsValidHandle(DMAHandle))
    {
        HWPAL_DMAResource_Record_t * Rec_p;

        Rec_p = HWPAL_DMAResource_Handle2RecordPtr(DMAHandle);

        if (Rec_p->alloc.MemoryBank == ADAPTER_DMABUF_MEMORYBANK_INTERNALLOC)
        {
#ifndef ADAPTER_USER_DOMAIN_BUILD
#ifdef RT_EIP93_DRIVER
            dma_free_coherent(NULL, Rec_p->alloc.AllocatedSize, Rec_p->alloc.AllocatedAddr_p, K1_TO_PHYSICAL(Rec_p->alloc.AllocatedAddr_p));
#else
            kfree(Rec_p->alloc.AllocatedAddr_p);
#endif
#else
            free(Rec_p->alloc.AllocatedAddr_p);
#endif  // ADAPTER_USER_DOMAIN_BUILD
            Rec_p->alloc.AllocatedAddr_p = NULL;
        }

        Rec_p->Magic = 0;

        HWPAL_DMAResource_Destroy(DMAHandle);

        return DMABUF_STATUS_OK;
    }

    return DMABUF_ERROR_INVALID_HANDLE;
}
EXPORT_SYMBOL(DMABuf_Release); /*for integration*/

/* end of file adapter_dmabuf.c */
