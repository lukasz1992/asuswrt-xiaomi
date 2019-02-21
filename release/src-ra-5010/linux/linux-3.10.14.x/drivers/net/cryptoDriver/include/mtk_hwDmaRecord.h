
#ifndef EIP93_HW_DMA_RECORD_H
#define EIP93_HW_DMA_RECORD_H

#include "mtk_dmaBuf.h"

typedef struct
{
    uint32_t Magic;     // signature used to validate handles

    struct
    {
        // for freeing the buffer
        void * AllocatedAddr_p;
        unsigned int AllocatedSize;     // in bytes

        void * Alternative_p;
        char AllocatorRef;

        // for separating SoC memory from main memory
        uint8_t MemoryBank;

    } alloc;

    struct
    {
        // alignment used for HostAddr_p
        uint8_t Alignment;

        // aligned start-address, data starts here
        void * HostAddr_p;

        // maximum data amount that can be stored from HostAddr_p
        unsigned int BufferSize;        // in bytes

        // true = memory is cached
        bool fCached;
    } host;

    struct
    {
        // used by Read/Write32[Array]
        bool fSwapEndianess;

        // address as seen by device
        // (must point to same buffer as HostAddr_p)
        // 0 = not yet translated
        uint32_t DeviceAddr32;

    } device;

#ifndef ADAPTER_REMOVE_BOUNCEBUFFERS
    struct
    {
        // bounce buffer for DMABuf_Register'ed buffers
        // note: used only when concurrency is impossible
        //       (PE source packets allow concurrency!!)
        DMABuf_Handle_t Bounce_Handle;
    } bounce;
#endif

} HWPAL_DMAResource_Record_t;

#endif
