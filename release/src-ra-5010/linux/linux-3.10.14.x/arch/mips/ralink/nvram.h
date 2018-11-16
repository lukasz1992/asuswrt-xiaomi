#ifndef _NVRAM_H
#define _NVRAM_H 	1

#include <linux/autoconf.h>

#define ENV_BLK_SIZE 0x1000

#ifdef CONFIG_DUAL_IMAGE
#define ENV_UBOOT_SIZE 0x1000
#define FLASH_BLOCK_NUM	5
#define UBOOT_NVRAM	0
#define RT2860_NVRAM    1
#define RTDEV_NVRAM    	2
#define WIFI3_NVRAM    	3
#define CERT_NVRAM    	4
#else
#define FLASH_BLOCK_NUM	4
#define RT2860_NVRAM    0
#define RTDEV_NVRAM    	1
#define WIFI3_NVRAM    	2
#define CERT_NVRAM    	3
#endif

#define RALINK_NVRAM_DEVNAME "apsoc_nvram"
#define RALINK_NVRAM_MTDNAME "Config"
#define FB_2860_BLOCK_NAME "2860"
#define FB_CONFIG2_BLOCK_NAME "config2"

#if defined CONFIG_EXTEND_NVRAM
#define RALINK_NVRAM2_MTDNAME "Config2"
#define EXTEND_BLOCK_NUM 2
#define CONFIG2_NVRAM 		CERT_NVRAM+1
#if defined CONFIG_CONFIG_SHRINK
#define VOIP_NVRAM			RT2860_NVRAM
#else
#define VOIP_NVRAM			CONFIG2_NVRAM
#endif
#if defined CONFIG_WAPI_SUPPORT
#define WAPI_NVRAM    	CONFIG2_NVRAM+1
#else
#define TR069CERT_NVRAM    	CONFIG2_NVRAM+1
#endif
#else
#define EXTEND_BLOCK_NUM 0
#define VOIP_NVRAM			RT2860_NVRAM
#if defined CONFIG_CONFIG_SHRINK
#define CONFIG2_NVRAM           RT2860_NVRAM
#endif
#endif

#define RANV_PRINT(x, ...) do { if (ra_nvram_debug) printk("\n%s %d: " x, __FILE__, __LINE__, ## __VA_ARGS__); } while(0)
#define RANV_ERROR(x, ...) do { printk("%s %d: ERROR! " x, __FILE__, __LINE__, ## __VA_ARGS__); } while(0)

//x is the value returned if the check failed
#define RANV_CHECK_INDEX(x) do { \
        if (index < 0 || index >= FLASH_BLOCK_NUM+EXTEND_BLOCK_NUM) { \
                RANV_PRINT("index(%d) is out of range\n", index); \
		return x; \
        } \
} while (0)

#define RANV_CHECK_VALID() do { \
        if (!fb[index].valid) { \
                RANV_PRINT("fb[%d] invalid, init again\n", index); \
		init_nvram_block(index); \
        } \
} while (0)

typedef struct environment_s {
	unsigned long crc;		//CRC32 over data bytes
	char *data;
} env_t;

typedef struct cache_environment_s {
	char *name;
	char *value;
} cache_t;

#define MAX_CACHE_ENTRY 1000
typedef struct block_s {
	char *name;
	env_t env;			//env block
	cache_t	cache[MAX_CACHE_ENTRY];	//env cache entry by entry
	unsigned long flash_offset;
	unsigned long flash_max_len;	//ENV_BLK_SIZE

	char valid;
	char dirty;
} block_t;

#define MAX_NAME_LEN 128
#define MAX_VALUE_LEN (ENV_BLK_SIZE * 5)
typedef struct nvram_ioctl_s {
	int index;
	int ret;
	char *name;
	char *value;
} nvram_ioctl_t;

#define RALINK_NVRAM_IOCTL		0x1000
#define RALINK_NVRAM_IOCTL_GET		RALINK_NVRAM_IOCTL + 0x01
#define RALINK_NVRAM_IOCTL_GETALL	RALINK_NVRAM_IOCTL + 0x02
#define RALINK_NVRAM_IOCTL_SET		RALINK_NVRAM_IOCTL + 0x03
#define RALINK_NVRAM_IOCTL_COMMIT	RALINK_NVRAM_IOCTL + 0x04
#define RALINK_NVRAM_IOCTL_CLEAR	RALINK_NVRAM_IOCTL + 0x05

#endif
