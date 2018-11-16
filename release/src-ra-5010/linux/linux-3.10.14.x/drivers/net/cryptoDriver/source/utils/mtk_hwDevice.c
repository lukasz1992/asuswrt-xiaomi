#include <asm/mach-ralink/rt_mmap.h>
#include "mtk_cHwPci.h"
#include "mtk_baseDefs.h"
#include "mtk_hwAccess.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/moduleparam.h>

#define HWPAL_FLAG_READ     BIT_0   // 1
#define HWPAL_FLAG_WRITE    BIT_1   // 2
#define HWPAL_FLAG_SWAP     BIT_2   // 4
#define HWPAL_FLAG_HA       BIT_5   // 32

// Device administration structure
typedef struct
{
#ifdef HWPAL_DEVICE_MAGIC
    // Magic value for detecting valid handles
    unsigned int ValidHandle;
#endif

    // Name string used in HWPAL_Device_Find
    const char * DeviceName_p;

    // device offset range inside PCI device
    unsigned int StartByteOffset;
    unsigned int LastByteOffset;

    char Flags;
} HWPAL_Device_Administration_t; 

#define HWPAL_MAGIC_PCICONFIGSPACE  0xFF434647      // 43 46 47 = C F G

// the cs_hwpal_linux_pci_x86.h file defines a HWPAL_DEVICES that
// depends on the following HWPAL_DEVICE_ADD
#ifdef HWPAL_DEVICE_MAGIC
#define HWPAL_DEVICE_ADD(_name, _start, _last, _flags) \
        { HWPAL_DEVICE_MAGIC, _name, _start, _last, _flags }
#else
#define HWPAL_DEVICE_ADD(_name, _start, _last, _flags) \
        { _name, _start, _last, _flags }
#endif

static const HWPAL_Device_Administration_t HWPAL_Devices[] =
{
    HWPAL_DEVICE_ADD(
        "PCI_CONFIG_SPACE",
        HWPAL_MAGIC_PCICONFIGSPACE,
        HWPAL_MAGIC_PCICONFIGSPACE + 1024,
        HWPAL_PCI_CONFIG_FLAGS),
    HWPAL_DEVICES
};

// number of devices supported calculated on HWPAL_DEVICES defined 
// in cs_linux_pci_x86.h
#define DEVICE_COUNT \
        (sizeof(HWPAL_Devices) \
         / sizeof(HWPAL_Device_Administration_t))

         
#ifdef RT_EIP93_DRIVER
//the base address of Crypto Engine registers
static uint32_t * HWPAL_MappedBaseAddr_p = (uint32_t *)RALINK_CRYPTO_ENGINE_BASE;
#else
// virtual address returned by ioremap()
static uint32_t * HWPAL_MappedBaseAddr_p = NULL;
#endif         


// declarations native to Linux kernel
static struct pci_dev * HWPAL_PCI_Device_p = NULL;

// checks that byte offset is in range as per the
#define IS_INVALID_OFFSET(_ofs, _devp) \
    (((_devp)->StartByteOffset + (_ofs) > (_devp)->LastByteOffset) || \
     (((_ofs) & 3) != 0))

#ifdef HWPAL_DEVICE_MAGIC

// checks that device handle is valid
#define IS_INVALID_DEVICE(_devp) \
    ((_devp) < HWPAL_Devices || \
     (_devp) >= HWPAL_Devices + DEVICE_COUNT || \
     (_devp)->ValidHandle != HWPAL_DEVICE_MAGIC)

#endif /* HWPAL_DEVICE_MAGIC */

#ifdef RT_EIP93_DRIVER
/*----------------------------------------------------------------------------
 * rt_dump_register
 *
 * This function dumps an Crypto Engine's register.
 * (define RT_DUMP_REGISTER in cs_hwpal_linux_pci.h before use it!)
 *
 * Use rt_dump_register(0xfff) to dump all registers.
 * Use rt_dump_register(register_offset) to dump a specific register.
 * The register_offset can be referred in Programmer-Manual.pdf
 */
void
rt_dump_register(
        unsigned int offset)
{
#ifdef RT_DUMP_REGISTER
    unsigned int register_base = RALINK_CRYPTO_ENGINE_BASE;
	unsigned int value = 0, i = 0;

	offset &= 0xfff;
	if(offset != 0xfff) /* print for a specific register */
	{
		value = ioread32((void __iomem *)(register_base+offset));
		printk("<address>\t<value>\n0x%08x\t0x%08x\n", register_base+offset, value);
	}
	else /* print for all registers */
	{
		printk("\n[Command Registers:]\n");
		printk("<address>\t<value>\n");
		for(i=0; i<=0x1c; i+=0x4){
			value = ioread32((void __iomem *)(register_base+i));
			printk("0x%08x\t0x%08x\n", register_base+i, value);
		}
        
		printk("\n[Descriptor Ring Configuration Registers:]\n");
        printk("<address>\t<value>\n");
        for(i=0x80; i<=0x9c; i+=0x4){
            value = ioread32((void __iomem *)(register_base+i));
            printk("0x%08x\t0x%08x\n", register_base+i, value);
        }
        
        printk("\n[Configuration Registers:]\n");
        printk("<address>\t<value>\n");
        for(i=0x100; i<=0x104; i+=0x4){
            value = ioread32((void __iomem *)(register_base+i));
            printk("0x%08x\t0x%08x\n", register_base+i, value);
        }
        for(i=0x10c; i<=0x118; i+=0x4){
            value = ioread32((void __iomem *)(register_base+i));
            printk("0x%08x\t0x%08x\n", register_base+i, value);
        }
            printk("0x%08x\t0x%08x\n", register_base+0x120, value);
            printk("0x%08x\t0x%08x\n", register_base+0x1d0, value);
            
        printk("\n[Clock Control and Debug Interface Registers:]\n");
        printk("<address>\t<value>\n");
            printk("0x%08x\t0x%08x\n", register_base+0x1e0, value);
            
        printk("\n[Device Revision and Options Registers:]\n");
        printk("<address>\t<value>\n");
        for(i=0x1f4; i<=0x1fc; i+=0x4){
            value = ioread32((void __iomem *)(register_base+i));
            printk("0x%08x\t0x%08x\n", register_base+i, value);
        }
        
        printk("\n[Interrupt Control Registers:]\n");
        printk("<address>\t<value>\n");
        for(i=0x200; i<=0x214; i+=0x4){
            value = ioread32((void __iomem *)(register_base+i));
            printk("0x%08x\t0x%08x\n", register_base+i, value);
        }
        
        printk("\n[SA Registers:]\n");
        printk("<address>\t<value>\n");
        for(i=0x400; i<=0x404; i+=0x4){
            value = ioread32((void __iomem *)(register_base+i));
            printk("0x%08x\t0x%08x\n", register_base+i, value);
        }
        for(i=0x420; i<=0x444; i+=0x4){
            value = ioread32((void __iomem *)(register_base+i));
            printk("0x%08x\t0x%08x\n", register_base+i, value);
        }
        for(i=0x468; i<=0x478; i+=0x4){
            value = ioread32((void __iomem *)(register_base+i));
            printk("0x%08x\t0x%08x\n", register_base+i, value);
        }
        for(i=0x500; i<=0x528; i+=0x4){
            value = ioread32((void __iomem *)(register_base+i));
            printk("0x%08x\t0x%08x\n", register_base+i, value);
        }       
	}
#endif 
}
#endif
 
/*----------------------------------------------------------------------------
 * HWPAL_Hexdump
 *
 * This function hex-dumps an array of uint32_t.
 */
#if ((defined(HWPAL_TRACE_DEVICE_READ)) || (defined(HWPAL_TRACE_DEVICE_WRITE)))
static void
HWPAL_Hexdump(
        const char * ArrayName_p,
        const char * DeviceName_p,
        const unsigned int ByteOffset,
        const uint32_t * WordArray_p,
        const unsigned int WordCount,
        bool fSwapEndianness)
{
    unsigned int i;

    printk(
        "%s: "
        "byte offsets 0x%x - 0x%x"
        " (%s)\n"
        "  ",
        ArrayName_p,
        ByteOffset,
        ByteOffset + WordCount*4 -1,
        DeviceName_p);

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
 

/*----------------------------------------------------------------------------
 * HWPAL_Device_Find
 */
bool
HWPAL_Device_Find(
        const char * DeviceName_p,
        HWPAL_Device_t * const Device_p)
{
    uint32_t i;

    if (Device_p == NULL)
        return false;

    *Device_p = NULL;

    for (i = 0; i < DEVICE_COUNT; i++)
    {
        // protect again potential empty records
        // caused by incomplete initializers
        if (HWPAL_Devices[i].DeviceName_p == NULL)
            continue;

        if (strcmp(DeviceName_p, HWPAL_Devices[i].DeviceName_p) == 0)
        {
            *Device_p = (HWPAL_Device_t)&HWPAL_Devices[i];

#ifdef HWPAL_TRACE_DEVICE_FIND
            printk(
                     "HWPAL_Device_Find: "
                     "Returned %p for device %s\n",
                     *Device_p,
                     DeviceName_p);
#endif
            return true;
        }
    }
    
#ifdef HWPAL_TRACE_DEVICE_FIND
    printk(
            "HWPAL_Device_Find: Failed to locate device %s\n", 
            DeviceName_p);
#endif

    return false;
}


/*----------------------------------------------------------------------------
 * HWPAL_Device_Init
 */
void
HWPAL_Device_Init(
        HWPAL_Device_t Device)
{
    IDENTIFIER_NOT_USED(Device);
}


/*----------------------------------------------------------------------------
 * HWPAL_RemapDeviceAddress
 *
 * This function remaps certain device addresses (relative within the whole
 * device address map) to other addresses. This is needed when the integration
 * has remapped some EIP device registers to other addresses. The EIP Driver
 * Libraries assume the devices always have the same internal layout.
 */

// the cs_hwpal_linux_pci_x86.h file defines a HWPAL_REMAP_ADDRESSES that
// depends on the following HWPAL_REMAP_ONE

#define HWPAL_REMAP_ONE(_old, _new) \
    case _old: \
        DeviceByteOffset = _new; \
        break;

static inline unsigned int
HWPAL_RemapDeviceAddress(
        unsigned int DeviceByteOffset)
{
    switch(DeviceByteOffset)
    {
        // include the remap statements
        HWPAL_REMAP_ADDRESSES

        default:
            break;
    }

    return DeviceByteOffset;
}


/*----------------------------------------------------------------------------
 * HWPAL_Device_Read32
 */
uint32_t 
HWPAL_Device_Read32(
        HWPAL_Device_t Device,
        const unsigned int ByteOffset)
{
    HWPAL_Device_Administration_t * Device_p;
    uint32_t Value=0;

    Device_p = (HWPAL_Device_Administration_t *)Device;
    if (Device_p == NULL)
        return 0xEEEEEEEE;

#ifdef HWPAL_DEVICE_MAGIC
    if (IS_INVALID_DEVICE(Device_p))
    {
        printk(
                "HWPAL_Device_Read32: "
                "Invalid device handle provided.\n");

        return 0xEEEEEEEE;
    }
#endif /* HWPAL_DEVICE_MAGIC */

#ifdef HWPAL_STRICT_ARGS_CHECK
    if (IS_INVALID_OFFSET(ByteOffset, Device_p)) 
    {
        printk(
                "HWPAL_Device_Read32: "
                "Invalid ByteOffset 0x%x (device %s)\n",
                ByteOffset,
                Device_p->DeviceName_p);

        return 0xEEEEEEEE;
    }
#endif /* HWPAL_STRICT_ARGS_CHECK */

#ifdef HWPAL_ENABLE_HA_SIMULATION
    if (Device_p->Flags & HWPAL_FLAG_HA)
    {
        // HA simulation mode
        // disable access to PKA_MASTER_SEQ_CTRL
        if (ByteOffset == 0x3FC8)
        {
            Value = 0;
            goto HA_SKIP;
        }
    }
#endif

    if (Device_p->StartByteOffset == HWPAL_MAGIC_PCICONFIGSPACE)
    {
#ifndef RT_EIP93_DRIVER
        pci_read_config_dword(HWPAL_PCI_Device_p, ByteOffset, &Value);
#endif
    }
    else
    {
        unsigned int DeviceByteOffset = Device_p->StartByteOffset + ByteOffset;
          
        DeviceByteOffset = HWPAL_RemapDeviceAddress(DeviceByteOffset);
          
        Value = ioread32(HWPAL_MappedBaseAddr_p + (DeviceByteOffset / 4));
    }

#ifdef HWPAL_ENABLE_HA_SIMULATION
HA_SKIP:
#endif

#ifdef HWPAL_DEVICE_ENABLE_SWAP
    if (Device_p->Flags & HWPAL_FLAG_SWAP)
        Value = HWPAL_SwapEndian32(Value);
#endif

#ifdef HWPAL_TRACE_DEVICE_READ
    if (Device_p->Flags & HWPAL_FLAG_READ)
    {
        unsigned int DeviceByteOffset = Device_p->StartByteOffset + ByteOffset;
        unsigned int DeviceByteOffset2 = HWPAL_RemapDeviceAddress(DeviceByteOffset);
        if (DeviceByteOffset2 != DeviceByteOffset)
        {
            DeviceByteOffset2 -= Device_p->StartByteOffset;
            printk(
                    "HWPAL_Device_Read32: "
                    "0x%x(was 0x%x) = 0x%08x (%s)\n",
                    DeviceByteOffset2,
                    ByteOffset,
                    (unsigned int)Value,
                    Device_p->DeviceName_p);
        }
        else
        {
            printk(
                    "HWPAL_Device_Read32: "
                    "0x%x = 0x%08x (%s)\n",
                    ByteOffset,
                    (unsigned int)Value,
                    Device_p->DeviceName_p);
        }
    }
#endif /* HWPAL_TRACE_DEVICE_READ */

    return Value;
}


/*----------------------------------------------------------------------------
 * HWPAL_Device_Write32
 */
void
HWPAL_Device_Write32(
        HWPAL_Device_t Device,
        const unsigned int ByteOffset,
        const uint32_t ValueIn)
{
    HWPAL_Device_Administration_t * Device_p;
    uint32_t Value = ValueIn;

    Device_p = (HWPAL_Device_Administration_t *)Device;
    if (Device_p == NULL)
        return;

#ifdef HWPAL_DEVICE_MAGIC
    if (IS_INVALID_DEVICE(Device_p))
    {
        printk(
                "HWPAL_Device_Write32 :"
                "Invalid device handle provided.\n");

        return;
    }
#endif /* HWPAL_DEVICE_MAGIC */

#ifdef HWPAL_STRICT_ARGS_CHECK
    if (IS_INVALID_OFFSET(ByteOffset, Device_p))
    {
        printk(
                "HWPAL_Device_Write32: "
                "Invalid ByteOffset 0x%x (device %s)\n",
                ByteOffset,
                Device_p->DeviceName_p);
        return;
    }
#endif /* HWPAL_STRICT_ARGS_CHECK */

#ifdef HWPAL_TRACE_DEVICE_WRITE
    if (Device_p->Flags & HWPAL_FLAG_WRITE)
    {
        printk(
                "HWPAL_Device_Write32: "
                "0x%x = 0x%08x (%s)\n",
                ByteOffset,
                (unsigned int)Value,
                Device_p->DeviceName_p);
    }
#endif /* HWPAL_TRACE_DEVICE_WRITE*/

#ifdef HWPAL_DEVICE_ENABLE_SWAP
    if (Device_p->Flags & HWPAL_FLAG_SWAP)
        Value = HWPAL_SwapEndian32(Value);
#endif

#ifdef HWPAL_ENABLE_HA_SIMULATION
    if (Device_p->Flags & HWPAL_FLAG_HA)
    {
        // HA simulation mode
        // disable access to PKA_MASTER_SEQ_CTRL
        if (ByteOffset == 0x3FC8)
        {
            printk(
                "HWPAL_Device_Write32: "
                "Unexpected write to PKA_MASTER_SEQ_CTRL\n");
            return;
        }
    }
#endif

    if (Device_p->StartByteOffset == HWPAL_MAGIC_PCICONFIGSPACE)
    {
#ifndef RT_EIP93_DRIVER
        pci_write_config_dword(HWPAL_PCI_Device_p, ByteOffset, Value);
#endif
    } 
    else
    {
        uint32_t DeviceByteOffset = Device_p->StartByteOffset + ByteOffset;

        DeviceByteOffset = HWPAL_RemapDeviceAddress(DeviceByteOffset);

        iowrite32(Value, HWPAL_MappedBaseAddr_p + (DeviceByteOffset / 4));
    }
}


/*----------------------------------------------------------------------------
 * HWPAL_Device_Read32Array
 *
 * Not supported for PCI Configuration space!
 */
void
HWPAL_Device_Read32Array(
        HWPAL_Device_t Device,
        unsigned int Offset,            // read starts here, +4 increments
        uint32_t * MemoryDst_p,         // writing starts here
        const int Count)                // number of uint32's to transfer
{
    HWPAL_Device_Administration_t * Device_p;
    unsigned int DeviceByteOffset;

    Device_p = (HWPAL_Device_Administration_t *)Device;

    if (Device_p == NULL ||
        MemoryDst_p == NULL ||
        Count <= 0)
    {
        return;
    }

    if (IS_INVALID_OFFSET(Offset, Device_p))
    {
        printk("HWPAL_Device_Read32Array: "
               "Invalid ByteOffset 0x%x (device %s)\n",
               Offset,
               Device_p->DeviceName_p);
        return;
    }

#ifdef HWPAL_ENABLE_HA_SIMULATION
    if (Device_p->Flags & HWPAL_FLAG_HA)
    {
        // HA simulation mode
        // disable access to PKA_MASTER_SEQ_CTRL
        return;
    }
#endif

    DeviceByteOffset = Device_p->StartByteOffset + Offset;

    {
        unsigned int RemappedOffset;
        uint32_t Value;
        int i;

#ifdef HWPAL_DEVICE_ENABLE_SWAP
        bool fSwap = false;
        if (Device_p->Flags & HWPAL_FLAG_SWAP)
            fSwap = true;
#endif
        for (i = 0; i < Count; i++)
        {
            RemappedOffset = HWPAL_RemapDeviceAddress(DeviceByteOffset);

            Value = ioread32(HWPAL_MappedBaseAddr_p + (RemappedOffset / 4));

#ifdef HWPAL_DEVICE_ENABLE_SWAP
            // swap endianness if required
            if (fSwap)
                Value = HWPAL_SwapEndian32(Value);
#endif

            MemoryDst_p[i] = Value;
            DeviceByteOffset +=  4;
        } // for
    }

#ifdef HWPAL_TRACE_DEVICE_READ
    if (Device_p->Flags & HWPAL_FLAG_READ)
    {
        HWPAL_Hexdump(
            "HWPAL_Device_Read32Array",
            Device_p->DeviceName_p,
            Device_p->StartByteOffset + Offset,
            MemoryDst_p,
            Count,
            false);     // already swapped during read above
    }
#endif /* HWPAL_TRACE_DEVICE_READ */
}


/*----------------------------------------------------------------------------
 * HWPAL_Device_Write32Array
 *
 * Not supported for PCI Configuration space!
 */
void
HWPAL_Device_Write32Array(
        HWPAL_Device_t Device,
        unsigned int Offset,            // write starts here, +4 increments
        const uint32_t * MemorySrc_p,   // reading starts here
        const int Count)                // number of uint32's to transfer
{
    HWPAL_Device_Administration_t * Device_p;
    unsigned int DeviceByteOffset;

    Device_p = (HWPAL_Device_Administration_t *)Device;

    if (Device_p == NULL ||
        MemorySrc_p == NULL ||
        Count <= 0)
    {
        return;     // ## RETURN ##
    }

    if (IS_INVALID_OFFSET(Offset, Device_p))
    {
        printk(
            "HWPAL_Device_Write32Array: "
            "Invalid ByteOffset 0x%x (device %s)\n",
            Offset,
            Device_p->DeviceName_p);
        return;
    }

    DeviceByteOffset = Device_p->StartByteOffset + Offset;

#ifdef HWPAL_ENABLE_HA_SIMULATION
    if (Device_p->Flags & HWPAL_FLAG_HA)
    {
        // HA simulation mode
        // disable access to PKA_MASTER_SEQ_CTRL
        return;
    }
#endif

#ifdef HWPAL_TRACE_DEVICE_WRITE
    if (Device_p->Flags & HWPAL_FLAG_WRITE)
    {
        bool fSwap = false;
#ifdef HWPAL_DEVICE_ENABLE_SWAP
        if (Device_p->Flags & HWPAL_FLAG_SWAP)
            fSwap = true;
#endif

        HWPAL_Hexdump(
            "HWPAL_Device_Write32Array",
            Device_p->DeviceName_p,
            DeviceByteOffset,
            MemorySrc_p,
            Count,
            fSwap);
    }
#endif /* HWPAL_TRACE_DEVICE_WRITE */

    {
        unsigned int RemappedOffset;
        uint32_t Value;
        int i;

#ifdef HWPAL_DEVICE_ENABLE_SWAP
        bool fSwap = false;
        if (Device_p->Flags & HWPAL_FLAG_SWAP)
            fSwap = true;
#endif

        for (i = 0; i < Count; i++)
        {
            RemappedOffset = HWPAL_RemapDeviceAddress(DeviceByteOffset);
            Value = MemorySrc_p[i];
#ifdef HWPAL_DEVICE_ENABLE_SWAP
            if (fSwap)
                Value = HWPAL_SwapEndian32(Value);
#endif
            iowrite32(Value, HWPAL_MappedBaseAddr_p + (RemappedOffset / 4));

            DeviceByteOffset += 4;
        } // for
    }
}


/*----------------------------------------------------------------------------
 * HWPAL_Probe
 */
static int 
HWPAL_Probe(
        struct pci_dev * PCI_Device_p, 
        const struct pci_device_id * id)
{
    const int BAR_ID = 0;
    resource_size_t BaseAddrHwRd;
 
    {
        // enable the device
        // this also looks up the IRQ
        int res = pci_enable_device(PCI_Device_p);

        if (res)
        {
            printk(
                "HWPAL_Probe: "
                "Failed to enable PCI device %s\n",
                pci_name(PCI_Device_p));

            return res;
        }
    }

    // remember the device reference
    // we need when access the configuration space
    HWPAL_PCI_Device_p = PCI_Device_p;

    // now map the chip into kernel memory
    // so we can access the EIP static resources
    BaseAddrHwRd = pci_resource_start(PCI_Device_p, BAR_ID);
    BaseAddrHwRd &= ~0xf; // Chop off the control bits

    // note: ioremap is uncached by default
    HWPAL_MappedBaseAddr_p = ioremap(
                                 BaseAddrHwRd,
                                 pci_resource_len(PCI_Device_p, BAR_ID));
  
    if (!HWPAL_MappedBaseAddr_p)
    {
        printk(
            "HWPAL_Probe: "
            "Failed to ioremap PCI device %s\n",
            pci_name(PCI_Device_p));

        return 1;
    }

    printk(
        "HWPAL_Probe: "
        "Mapped base address is: %p, sizeof(resource_size_t)=%d\n"
        "  start=0x%x, end=0x%x, len=0x%x, flags=0x%x, irq=%d\n",
        HWPAL_MappedBaseAddr_p,
        sizeof(resource_size_t),
        (unsigned int)pci_resource_start(PCI_Device_p, 0),
        (unsigned int)pci_resource_end(PCI_Device_p, 0),
        (unsigned int)pci_resource_len(PCI_Device_p, 0),
        (unsigned int)pci_resource_flags(PCI_Device_p, 0),
        PCI_Device_p->irq);

    IDENTIFIER_NOT_USED(id);

    // return 0 to indicate "we decided to take ownership"
    return 0;
}


/*----------------------------------------------------------------------------
 * HWPAL_Remove
 */
static void
HWPAL_Remove(
        struct pci_dev * PCI_Device_p)
{
    printk(
        "HWPAL_Remove: "
        "HWPAL_MappedBaseAddr_p=%p\n",
        HWPAL_MappedBaseAddr_p);

    if (HWPAL_MappedBaseAddr_p)
    {
        iounmap(HWPAL_MappedBaseAddr_p);
        HWPAL_MappedBaseAddr_p = NULL;
    }

    pci_disable_device(PCI_Device_p);
}


/*----------------------------------------------------------------------------
 * Declarations native to Linux kernel
 */

static char HWPAL_Module_Name[] = HWPAL_PCI_DRIVER_NAME;

// Safenet PCI vendor ID
#define HWPAL_PCI_VENDOR_ID_SAFENET  0x16AE

const struct pci_device_id DeviceIDs[] =
{
    {PCI_DEVICE(HWPAL_PCI_VENDOR_ID_SAFENET, HWPAL_PCI_DEVICE_ID), },
    {0, }
};

static struct pci_driver HWPAL_PCI_Driver =
{
#ifdef _MSC_VER
    // microsoft compiler does not support partial initializers
    // NOTE: struct must have fields in this order
    HWPAL_Module_Name,
    DeviceIDs,
    HWPAL_Probe,
    HWPAL_Remove,
#else
    .name = HWPAL_Module_Name,
    .id_table = DeviceIDs,
    .probe = HWPAL_Probe,
    .remove = HWPAL_Remove,
#endif
};


/*----------------------------------------------------------------------------
 * HWPAL_Initialize
 */
bool
HWPAL_Initialize(
        void * CustomInitData_p)
{
    int Status;

    Status = pci_register_driver(&HWPAL_PCI_Driver);
    if (Status < 0)
    {
        printk(
            "HWPAL_Initialize: "
            "Failed to register the PCI device\n");

        return false;
    }

    // if provided, CustomInitData_p points to an "int"
    // we return the "irq" number via this output parameter
    if (CustomInitData_p)
    {
        int * p = (int *)CustomInitData_p;
        *p = HWPAL_PCI_Device_p->irq;
    }
    
    return true;
}


/*----------------------------------------------------------------------------
 * HWPAL_Unitialize
 */
void
HWPAL_UnInitialize(void)
{
    printk(
        "HWPAL_UnInitialize: "
        "calling pci_unregister_driver\n");

    pci_unregister_driver(&HWPAL_PCI_Driver);
}


MODULE_DEVICE_TABLE(pci, DeviceIDs);

