#ifndef __RALINK_FLASH_H__
#define __RALINK_FLASH_H__

#if defined (CONFIG_RT2880_FLASH_32M)
#define MTD_BOOT_PART_SIZE	0x40000
#define	MTD_CONFIG_PART_SIZE	0x20000
#define	MTD_FACTORY_PART_SIZE	0x20000
#else
#if defined (RECONFIG_PARTITION_SIZE)
#if !defined (MTD_BOOT_PART_SIZE)
#error "Please define MTD_BOOT_PART_SIZE"
#endif
#if !defined (MTD_CONFIG_PART_SIZE)
#error "Please define MTD_CONFIG_PART_SIZE"
#endif
#if !defined (MTD_FACTORY_PART_SIZE)
#error "Please define MTD_FACTORY_PART_SIZE"
#endif
#else
#define MTD_BOOT_PART_SIZE	0x30000
#define	MTD_CONFIG_PART_SIZE	0x10000
#define	MTD_FACTORY_PART_SIZE	0x10000
#endif
#endif


//#ifdef CONFIG_RT2880_ROOTFS_IN_FLASH
#ifdef CONFIG_ROOTFS_IN_FLASH_NO_PADDING
#define CONFIG_MTD_KERNEL_PART_SIZ 0
#endif
#define MTD_ROOTFS_PART_SIZE	IMAGE1_SIZE - (MTD_BOOT_PART_SIZE + MTD_CONFIG_PART_SIZE \
					+ MTD_FACTORY_PART_SIZE + CONFIG_MTD_KERNEL_PART_SIZ)
#define	MTD_KERN_PART_SIZE	CONFIG_MTD_KERNEL_PART_SIZ
//#else
//#if defined CONFIG_EXTEND_NVRAM
//#define MTD_KERN_PART_SIZE	IMAGE1_SIZE - (MTD_BOOT_PART_SIZE + MTD_CONFIG_PART_SIZE \
//				+ MTD_FACTORY_PART_SIZE + MTD_CONFIG_PART_SIZE)
//#else
//#define MTD_KERN_PART_SIZE	IMAGE1_SIZE - (MTD_BOOT_PART_SIZE + MTD_CONFIG_PART_SIZE \
//					+ MTD_FACTORY_PART_SIZE)
//#endif
//#endif


#ifdef CONFIG_DUAL_IMAGE
#if defined (CONFIG_RT2880_FLASH_2M)
#define IMAGE1_SIZE		0x100000
#elif defined (CONFIG_RT2880_FLASH_4M)
#define IMAGE1_SIZE		0x200000
#elif defined (CONFIG_RT2880_FLASH_8M)
#define IMAGE1_SIZE		0x400000
#elif defined (CONFIG_RT2880_FLASH_16M)
#define IMAGE1_SIZE		0x800000
#elif defined (CONFIG_RT2880_FLASH_32M)
#define IMAGE1_SIZE		0x1000000
#endif

#define MTD_KERN2_PART_SIZE	MTD_KERN_PART_SIZE
#define MTD_KERN2_PART_OFFSET	IMAGE1_SIZE + (MTD_BOOT_PART_SIZE + MTD_CONFIG_PART_SIZE \
					+ MTD_FACTORY_PART_SIZE)

#ifdef CONFIG_RT2880_ROOTFS_IN_FLASH
#define MTD_ROOTFS2_PART_SIZE 	MTD_ROOTFS_PART_SIZE	
#define MTD_ROOTFS2_PART_OFFSET (MTD_KERN2_PART_OFFSET + MTD_KERN2_PART_SIZE)
#endif

#else // Non Dual Image
#if defined (CONFIG_RT2880_FLASH_2M)
#define IMAGE1_SIZE		0x200000
#elif defined (CONFIG_RT2880_FLASH_4M)
#define IMAGE1_SIZE		0x400000
#elif defined (CONFIG_RT2880_FLASH_8M)
#define IMAGE1_SIZE		0x800000
#elif defined (CONFIG_RT2880_FLASH_16M)
#define IMAGE1_SIZE		0x1000000
#elif defined (CONFIG_RT2880_FLASH_32M)
#define IMAGE1_SIZE		0x2000000
#else
#define IMAGE1_SIZE		0x8000000	// ASUS EXT	//CONFIG_MTD_PHYSMAP_LEN
#endif
#endif

#if defined CONFIG_EXTEND_NVRAM
#define MTD_CONFIG2_PART_OFFSET	IMAGE1_SIZE - MTD_CONFIG_PART_SIZE
#endif

#define BOOT_FROM_NOR	0
#define BOOT_FROM_NAND	2
#define BOOT_FROM_SPI	3

int ra_check_flash_type(void);

 
#ifdef CONFIG_ROOTFS_IN_FLASH_NO_PADDING
#define IH_NMLEN		32	/* Image Name Length		*/

#define MAX_STRING 12
#define MAX_VER 4

/* If hw[i].kernel == ROOTFS_OFFSET_MAGIC,
 * rootfilesystem offset (uImage header size + kernel size)
 * can be calculated by following equation:
 * (hw[i].minor << 16) | (hw[i+1].major << 8) | (hw[i+1].minor)
 */
#define ROOTFS_OFFSET_MAGIC	0xA9	/* Occupy two version_t		*/

typedef struct {
	uint8_t major;
	uint8_t minor;
} version_t;

typedef struct {
	version_t kernel;
	version_t fs;
	char	  productid[MAX_STRING];
	version_t hw[MAX_VER*2];
} TAIL;

typedef struct image_header {
	uint32_t	ih_magic;	/* Image Header Magic Number	*/
	uint32_t	ih_hcrc;	/* Image Header CRC Checksum	*/
	uint32_t	ih_time;	/* Image Creation Timestamp	*/
	uint32_t	ih_size;	/* Image Data Size		*/
	uint32_t	ih_load;	/* Data	 Load  Address		*/
	uint32_t	ih_ep;		/* Entry Point Address		*/
	uint32_t	ih_dcrc;	/* Image Data CRC Checksum	*/
	uint8_t		ih_os;		/* Operating System		*/
	uint8_t		ih_arch;	/* CPU architecture		*/
	uint8_t		ih_type;	/* Image Type			*/
	uint8_t		ih_comp;	/* Compression Type		*/
	union {
		uint8_t		ih_name[IH_NMLEN];	/* Image Name		*/
		TAIL		tail;		/* ASUS firmware infomation	*/
	} u;
} image_header_t;
#endif
#endif //__RALINK_FLASH_H__
