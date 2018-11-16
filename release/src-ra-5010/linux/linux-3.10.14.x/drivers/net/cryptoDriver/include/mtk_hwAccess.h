
#ifndef EIP93_HW_ACCESS_H
#define EIP93_HW_ACCESS_H

#include "mtk_baseDefs.h"     // bool, uint32_t, inline
#include "mtk_csHwPci.h"


/*----------------------------------------------------------------------------
 * rt_dump_register
 *
 * This function dumps an Crypto Engine's register.
 * (define RT_DUMP_REGISTER in mtk_csHwPci.h before use it!)
 *
 * Use rt_dump_register(0xfff) to dump all registers.
 * Use rt_dump_register(register_offset) to dump a specific register.
 * The register_offset can be referred in Programmer-Manual.pdf
 */
void
rt_dump_register(
		unsigned int offset);

/*----------------------------------------------------------------------------
 * HWPAL_Initialize
 *
 * This function must be called exactly once to initialize the HWPAL
 * implementation before any other API function may be used.
 *
 * CustomInitData_p
 *     This anonymous parameter can be used to pass information from the caller
 *     to the driver framework implementation.
 *
 * Return Value
 *     true   Success
 *     false  Failed to initialize
 */
bool
HWPAL_Initialize(
        void * CustomInitData_p);


/*----------------------------------------------------------------------------
 * HWPAL_UnInitialize
 *
 * This function can be called to shut down the HWPAL implementation. The
 * caller must make sure none of the other API functions are called after or
 * during the invokation of this UnInitialize function. After this call
 * returns the API state is back in "uninitialized" and the HWPAL_Initialize
 * function may be used anew.
 *
 * Return Value
 *     None
 */
void
HWPAL_UnInitialize(void);


/*----------------------------------------------------------------------------
 * HWPAL_SwapEndian32
 *
 * This function can be used to swap the byte order of a 32bit integer. The
 * implementation could use custom CPU instructions, if available.
 */
static inline uint32_t
HWPAL_SwapEndian32(
        const uint32_t Value)
{
#ifdef HWPAL_SWAP_SAFE
    return (((Value & 0x000000FFU) << 24) |
            ((Value & 0x0000FF00U) <<  8) |
            ((Value & 0x00FF0000U) >>  8) |
            ((Value & 0xFF000000U) >> 24));
#else
    // reduced typically unneeded AND operations
    return ((Value << 24) |
            ((Value & 0x0000FF00U) <<  8) |
            ((Value & 0x00FF0000U) >>  8) |
            (Value >> 24));
#endif
}


/*****************************************************************************
 * HWPAL_Device
 *
 * These functions can be used to transfer a single 32bit word, or an array of
 * such words to or from a device. The device, typically a single hardware
 * block within a chip, is represented by the handle.
 *
 * The handles are typically defined statically and can be retrieved by the
 * caller using the HWPAL_Device_Find function.
 *
 * The driver libraries use the handle to allow one chip to contain multiple
 * instances of a device type and reuse the driver for each instance.
 * The driver libraries will access the device resources (registers and RAM)
 * using offsets.
 *
 * Each device has its own configuration, including the endianess swapping
 * need for the words transferred. Endianess swapping can thus be performed on
 * the fly and transparent to the caller.
 */

typedef void * HWPAL_Device_t;


/*----------------------------------------------------------------------------
 * HWPAL_Device_Find
 *
 * This function must be used to retrieve a handle for a certain device that
 * is identified by a string. The exact strings supported is decided by the
 * implementation of driver framework and can differ from product to product.
 * Note that this function may be called more than once to retrieve the same
 * handle for the same device.
 *
 * DeviceName_p (input)
 *     Pointer to the (zero-terminated) string that represents the device.
 *
 * Device_p (output)
 *     Pointer to the memory location where the device handle will be written
 *     when it is found.
 *
 * Return Value
 *     true   Success, handle was written.
 *     false  Device is not supported.
 */
bool
HWPAL_Device_Find(
        const char * DeviceName_p,
        HWPAL_Device_t * const Device_p);


/*----------------------------------------------------------------------------
 * HWPAL_Device_Init
 *
 * This is an initialization point for the device instance. It must be called
 * exactly once before the device handle may be used to read or write the
 * device resources. It is typically called from the Driver Library.
 *
 * Device
 *     Handle for the device instance as returned by HWPAL_Device_Find.
 *
 * Return Value
 *     None
 */
void
HWPAL_Device_Init(
        HWPAL_Device_t Device);


/*----------------------------------------------------------------------------
 * HWPAL_Device_Read32
 *
 * This function can be used to read one 32bit resource inside a device
 * (typically a register or memory location). Since reading registers can have
 * side effects, the implementation must guarantee that the resource will be
 * read only once and no neighbouring resources will be accessed.
 *
 * If required (decided based on internal configuration), on the fly endianess
 * swapping of the value read will be performed before it is returned to the
 * caller.
 *
 * Device (input)
 *     Handle for the device instance as returned by HWPAL_Device_Find.
 *
 * ByteOffset (input)
 *     The byte offset within the device for the resource to read.
 *
 * Return Value
 *     The value read.
 *
 * When the Device or Offset parameters are invalid, the implementation will
 * return an unspecified value.
 */
uint32_t
HWPAL_Device_Read32(
        HWPAL_Device_t Device,
        const unsigned int ByteOffset);


/*----------------------------------------------------------------------------
 * HWPAL_Device_Write32
 *
 * This function can be used to write one 32bit resource inside the device
 * (typically a register or memory location). Since writing registers can
 * have side effects, the implementation must guarantee that the resource will
 * be written only once and no neighbouring resources will be accessed.
 *
 * If required (decided based on internal configuration), on the fly endianess
 * swapping of the value to be written will be performed.
 *
 * Device (input)
 *     Handle for the device instance as returned by HWPAL_Device_Find.
 *
 * ByteOffset (input)
 *     The byte offset within the device for the resource to write.
 *
 * Value (input)
 *     The 32bit value to write.
 *
 * Return Value
 *     None
 *
 * The write can only be successful when the Device and ByteOffset parameters
 * are valid.
 */
void
HWPAL_Device_Write32(
        HWPAL_Device_t Device,
        const unsigned int ByteOffset,
        const uint32_t Value);


/*----------------------------------------------------------------------------
 * HWPAL_Device_Read32Array
 *
 * This function perform the same task as HWPAL_Device_Read32 for a
 * consecutive array of 32bit words, allowing the implementation to use a
 * more optimal burst-read.
 *
 * See HWPAL_Device_Read32 for a more detailed description.
 *
 * Device (input)
 *     Handle for the device instance as returned by HWPAL_Device_Find.
 *
 * ByteOffset (input)
 *     Byte offset of the first resource to read. This value is incremented
 *     by 4 for each following resource.
 *
 * MemoryDst_p (output)
 *     Pointer to the memory where the retrieved words will be stored.
 *
 * Count (input)
 *     The number of 32bit words to transfer.
 *
 * Return Value
 *     None.
 */
void
HWPAL_Device_Read32Array(
        HWPAL_Device_t Device,
        unsigned int ByteOffset,
        uint32_t * MemoryDst_p,
        const int Count);


/*----------------------------------------------------------------------------
 * HWPAL_Device_Write32Array
 *
 * This function perform the same task as HWPAL_Device_Write32 for a
 * consecutive array of 32bit words, allowing the implementation to use a
 * more optimal burst-write.
 *
 * See HWPAL_Device_Write32 for a more detailed description.
 *
 * Device (input)
 *     Handle for the device instance as returned by HWPAL_Device_Find.
 *
 * ByteOffset (input)
 *     Byte offset of the first resource to write. This value is incremented
 *     by 4 for each following resource.
 *
 * MemorySrc_p (input)
 *     Pointer to the memory where the values to be written are located.
 *
 * Count (input)
 *     The number of 32bit words to transfer.
 *
 * Return Value
 *     None.
 */
void
HWPAL_Device_Write32Array(
        HWPAL_Device_t Device,
        unsigned int ByteOffset,
        const uint32_t * MemorySrc_p,
        const int Count);


#endif 

