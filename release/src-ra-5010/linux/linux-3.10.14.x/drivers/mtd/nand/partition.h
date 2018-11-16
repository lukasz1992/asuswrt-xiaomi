#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>

#define RECONFIG_PARTITION_SIZE 1

#define MTD_BOOT_PART_SIZE  0x80000
#define MTD_CONFIG_PART_SIZE    0x20000
#define MTD_FACTORY_PART_SIZE   0x20000

extern unsigned int  CFG_BLOCKSIZE;
#if defined(CONFIG_DUAL_TRX)	/* ASUS_EXT */
#define LARGE_MTD_BOOT_PART_SIZE	(CFG_BLOCKSIZE * 7)
#define LARGE_MTD_CONFIG_PART_SIZE	(CFG_BLOCKSIZE * 8)
#define LARGE_MTD_FACTORY_PART_SIZE	(CFG_BLOCKSIZE * 8)
#define TRX_FIRMWARE_SIZE		(50 * 1024 * 1024) 	//50 MB
#define TRX_FW_NUM			2
#else
#define LARGE_MTD_BOOT_PART_SIZE       (CFG_BLOCKSIZE<<2)
#define LARGE_MTD_CONFIG_PART_SIZE     (CFG_BLOCKSIZE<<2)
#define LARGE_MTD_FACTORY_PART_SIZE    (CFG_BLOCKSIZE<<1)
#define TRX_FW_NUM			1
#endif

#ifdef CONFIG_RT2880_ROOTFS_IN_FLASH
#define MTD_ROOTFS_RESERVED_BLOCK	0x80000  // (CFG_BLOCKSIZE<<2)
#endif

#include "../maps/ralink-flash.h"

/*=======================================================================*/
/* NAND PARTITION Mapping                                                  */
/*=======================================================================*/
#if defined(CONFIG_SUPPORT_OPENWRT)
static struct mtd_partition g_pasStatic_Partition[] = {
	{
	        name:           "ALL",
	        size:           MTDPART_SIZ_FULL,
	        offset:         0,
	},
	/* Put your own partition definitions here */
	{
	        name:           "Bootloader",
	        size:           MTD_BOOT_PART_SIZE,
	        offset:         0,
	}, {
	        name:           "Config",
	        size:           MTD_CONFIG_PART_SIZE,
	        offset:         MTDPART_OFS_APPEND
	}, {
	        name:           "Factory",
	        size:           MTD_FACTORY_PART_SIZE,
	        offset:         MTDPART_OFS_APPEND
	}, {
	        name:           "firmware",
	        size:           MTDPART_SIZ_FULL,
	        offset:         0x140000,
	},
};
#else /* CONFIG_SUPPORT_OPENWRT */
//#ifdef CONFIG_MTD_PARTITIONS
#define MTD_JFFS2_PART_SIZE     0x1400000	/* 20MB for JFFS */
static struct mtd_partition g_pasStatic_Partition[] = {

        /* Put your own partition definitions here */
        {
                name:           "Bootloader",
                size:           MTD_BOOT_PART_SIZE,
                offset:         0,
        }, {
                name:           "nvram",
                size:           MTD_CONFIG_PART_SIZE,
                offset:         MTDPART_OFS_APPEND
        }, {
                name:           "Factory",
                size:           MTD_FACTORY_PART_SIZE,
                offset:         MTDPART_OFS_APPEND
#ifdef CONFIG_MTK_MTD_NAND
        }, {
                name:           "Factory2",
                size:           MTD_FACTORY_PART_SIZE,
                offset:         MTDPART_OFS_APPEND
#endif	/* CONFIG_MTK_MTD_NAND */
#ifdef CONFIG_RT2880_ROOTFS_IN_FLASH
        }, {
                name:           "Kernel",
                size:           MTD_KERN_PART_SIZE,
                offset:         MTDPART_OFS_APPEND,
        }, {
                name:           "RootFS",
                size:           MTD_ROOTFS_PART_SIZE,
                offset:         MTDPART_OFS_APPEND,
        }, {
                name:           "RootFS_reserved",
                size:           MTD_ROOTFS_RESERVED_BLOCK,
                offset:         MTDPART_OFS_APPEND,
#ifdef CONFIG_ROOTFS_IN_FLASH_NO_PADDING
        }, {
                name:           "Kernel_RootFS",
                size:           MTD_KERN_PART_SIZE + MTD_ROOTFS_PART_SIZE,
                offset:         MTD_BOOT_PART_SIZE + MTD_CONFIG_PART_SIZE + MTD_FACTORY_PART_SIZE,
#endif
#else //CONFIG_RT2880_ROOTFS_IN_RAM
        }, {
                name:           "linux",
                size:           MTD_KERN_PART_SIZE,
                offset:         MTDPART_OFS_APPEND,
        }, {
                name:           "RootFS",
                size:           MTD_ROOTFS_PART_SIZE,
                offset:         MTDPART_OFS_APPEND,
#ifdef CONFIG_DUAL_TRX
        }, {
                name:           "linux2",
                size:           MTD_KERN_PART_SIZE,
                offset:         MTDPART_OFS_APPEND,
        }, {
                name:           "RootFS2",
                size:           MTD_ROOTFS_PART_SIZE,
                offset:         MTDPART_OFS_APPEND,
#endif	/* CONFIG_DUAL_TRX */
#endif
#ifdef CONFIG_DUAL_IMAGE
        }, {
                name:           "Kernel2",
                size:           MTD_KERN2_PART_SIZE,
                offset:         MTD_KERN2_PART_OFFSET,
#ifdef CONFIG_RT2880_ROOTFS_IN_FLASH
        }, {
                name:           "RootFS2",
                size:           MTD_ROOTFS2_PART_SIZE,
                offset:         MTD_ROOTFS2_PART_OFFSET,
#endif
#endif
        },{
                name:           "jffs2",
                size:           MTD_JFFS2_PART_SIZE,
                offset:         MTDPART_OFS_APPEND,
        } ,{
                name:           "ALL",
                size:           MTDPART_SIZ_FULL,
                offset:         0,
        }

};

#endif /* CONFIG_SUPPORT_OPENWRT */
#define NUM_PARTITIONS ARRAY_SIZE(g_pasStatic_Partition)
extern int part_num;	// = NUM_PARTITIONS;
//#endif
#undef RECONFIG_PARTITION_SIZE

