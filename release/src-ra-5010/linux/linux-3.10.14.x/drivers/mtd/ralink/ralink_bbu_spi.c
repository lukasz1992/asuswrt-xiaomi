/*
 * MTD SPI driver for ST M25Pxx flash chips
 *
 * Author: Mike Lavender, mike@steroidmicros.com
 *
 * Copyright (c) 2005, Intec Automation Inc.
 *
 * Some parts are based on lart.c by Abraham Van Der Merwe
 *
 * Cleaned up and generalized based on mtd_dataflash.c
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/interrupt.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/gen_probe.h>
#include <linux/mtd/partitions.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <linux/version.h>

#if defined (CONFIG_RALINK_RT6855A) || defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
#include "bbu_spiflash.h"
#else
#include "ralink_spi.h"
#endif
#include "../maps/ralink-flash.h"

//#define TWO_SPI_FLASH
#ifdef TWO_SPI_FLASH
#include <linux/mtd/concat.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,4)
#define add_mtd_partitions      mtd_device_register
#define del_mtd_partitions      mtd_device_unregister
#endif

#if defined(CONFIG_SUPPORT_OPENWRT)
static struct mtd_partition rt2880_partitions[] = {
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
	        offset:         MTDPART_OFS_APPEND,
	}
};
#else /* CONFIG_SUPPORT_OPENWRT */
#if 0 //asus
static struct mtd_partition rt2880_partitions[] = {
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
#ifdef CONFIG_RT2880_ROOTFS_IN_FLASH
        }, {
                name:           "Kernel",
                size:           MTD_KERN_PART_SIZE,
                offset:         MTDPART_OFS_APPEND,
        }, {
                name:           "RootFS",
                size:           MTD_ROOTFS_PART_SIZE,
                offset:         MTDPART_OFS_APPEND,
#ifdef CONFIG_ROOTFS_IN_FLASH_NO_PADDING
        }, {
                name:           "Kernel_RootFS",
                size:           MTD_KERN_PART_SIZE + MTD_ROOTFS_PART_SIZE,
                offset:         MTD_BOOT_PART_SIZE + MTD_CONFIG_PART_SIZE + MTD_FACTORY_PART_SIZE,
#endif
#else //CONFIG_RT2880_ROOTFS_IN_RAM
        }, {
                name:           "Kernel",
                size:           MTD_KERN_PART_SIZE,
                offset:         MTDPART_OFS_APPEND,
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
#ifdef CONFIG_EXTEND_NVRAM
        }, {
                name:           "Config2",
                size:           MTD_CONFIG_PART_SIZE,
                offset:         MTD_CONFIG2_PART_OFFSET,
#endif
        }
};
#endif //asus
#endif /* CONFIG_SUPPORT_OPENWRT */


//asus
#define MTD_RADIO_PART_SIZE	0x10000
#define	MTD_JFFS2_PART_SIZE	0x100000
static struct mtd_partition rt2880_partitions[] = {
	/* Put your own partition definitions here */
        {
                name:           "Bootloader",  /* mtdblock0 */
                size:           MTD_BOOT_PART_SIZE,
                offset:         0,
        }, {
                name:           "nvram", /* mtdblock1 */
                size:           MTD_CONFIG_PART_SIZE,
                offset:         MTDPART_OFS_APPEND
        }, {
                name:           "Factory", /* mtdblock2 */
                size:           MTD_FACTORY_PART_SIZE,
                offset:         MTDPART_OFS_APPEND
        }, {
                name:           "linux", /* mtdblock3 */
                size:           MTD_KERN_PART_SIZE,
                offset:         MTDPART_OFS_APPEND,
        }, {
                name:           "rootfs", /* mtdblock4 */
                size:           MTD_ROOTFS_PART_SIZE,
                offset:         MTDPART_OFS_APPEND,
#if defined(CONFIG_SOUND)
        }, {
                name:           "Radio", /* mtdblock5 */
                size:           MTD_RADIO_PART_SIZE,
                offset:         MTDPART_OFS_APPEND,
#else
        }, {
                name:           "jffs2", /* mtdblock5 */
                size:           MTD_JFFS2_PART_SIZE,
                offset:         MTDPART_OFS_APPEND,
#endif
        } ,{
                name:           "ALL", /* mtdblock6 */
                size:           MTDPART_SIZ_FULL,
                offset:         0,
        }
};

//#define TEST_CS1_FLASH
//#define RD_MODE_DIOR

#ifdef TWO_SPI_FLASH
static struct mtd_partition rt2880_partitions_2[] = {
	{
		name:           "second_all",
		size:           MTDPART_SIZ_FULL,
		offset:         0x0,
	}, {
		name:           "second_1",
		size:           0x50000,
		offset:         0x0,
	}, {
		name:           "second_2",
		size:           0x700000,
		offset:         MTDPART_OFS_APPEND,
	}
};
#endif

#if defined(RD_MODE_DIOR) || defined(RD_MODE_DOR)
#define RD_MODE_DUAL
#elif defined(RD_MODE_QIOR) || defined(RD_MODE_QOR)
#define RD_MODE_QUAD
#endif

/* Choose the SPI flash mode */
#if defined (CONFIG_RALINK_RT6855A) || defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
#define BBU_MODE		// BBU SPI flash controller
#define MORE_BUF_MODE

// check DUAL/QUAD and MORE_BUF_MODE, they can't be enabled together
#if defined(MORE_BUF_MODE) && (defined(RD_MODE_DIOR) || defined(RD_MODE_DOR) || defined(RD_MODE_QIOR) || defined(RD_MODE_QOR))
#error "DUAL/QUAD mode and MORE_BUF_MODE can't be enabled together\n"
#endif
#else
#define USER_MODE		// SPI flash user mode support by default
//#define COMMAND_MODE		// SPI flash command mode support
#endif

#if !defined USER_MODE && !defined COMMAND_MODE && !defined BBU_MODE
#error "Please choose the correct mode of SPI flash controller"
#endif

/******************************************************************************
 * SPI FLASH elementray definition and function
 ******************************************************************************/

#define FLASH_PAGESIZE		256

/* Flash opcodes. */
#define OPCODE_WREN		6	/* Write enable */
#define OPCODE_WRDI		4	/* Write disable */
#define OPCODE_RDSR		5	/* Read status register */
#define OPCODE_WRSR		1	/* Write status register */
#define OPCODE_READ		3	/* Read data bytes */
#define OPCODE_PP		2	/* Page program */
#define OPCODE_SE		0xD8	/* Sector erase */
#define OPCODE_RES		0xAB	/* Read Electronic Signature */
#define OPCODE_RDID		0x9F	/* Read JEDEC ID */
#define OPCODE_DOR			0x3B	/* Dual Output Read */
#define OPCODE_QOR			0x6B	/* Quad Output Read */
#define OPCODE_DIOR                     0xBB    /* Dual IO High Performance Read */
#define OPCODE_QIOR                     0xEB    /* Quad IO High Performance Read */

/* Status Register bits. */
#define SR_WIP			1	/* Write in progress */
#define SR_WEL			2	/* Write enable latch */
#define SR_BP0			4	/* Block protect 0 */
#define SR_BP1			8	/* Block protect 1 */
#define SR_BP2			0x10	/* Block protect 2 */
#define SR_EPE			0x20	/* Erase/Program error */
#define SR_SRWD			0x80	/* SR write protect */

#define OPCODE_BRRD		0x16
#define OPCODE_BRWR		0x17
#define OPCODE_RDCR		0x35

//#define SPI_DEBUG
#if !defined (SPI_DEBUG)

#define ra_inl(addr)  (*(volatile unsigned int *)(addr))
#define ra_outl(addr, value)  (*(volatile unsigned int *)(addr) = (value))
#define ra_dbg(args...) do {} while(0)
/*#define ra_dbg(args...) do { printk(args); } while(0)*/

#else

#define ra_dbg(args...) do { printk(args); } while(0)
#define _ra_inl(addr)  (*(volatile unsigned int *)(addr))
#define _ra_outl(addr, value)  (*(volatile unsigned int *)(addr) = (value))

u32 ra_inl(u32 addr)
{	
	u32 retval = _ra_inl(addr);
	printk("%s(%x) => %x \n", __func__, addr, retval);

	return retval;	
}

u32 ra_outl(u32 addr, u32 val)
{
	_ra_outl(addr, val);

	printk("%s(%x, %x) \n", __func__, addr, val);

	return val;	
}

#endif // SPI_DEBUG //

#define ra_aor(addr, a_mask, o_value)  ra_outl(addr, (ra_inl(addr) & (a_mask)) | (o_value))
#define ra_and(addr, a_mask)  ra_aor(addr, a_mask, 0)
#define ra_or(addr, o_value)  ra_aor(addr, -1, o_value)

#define SPIC_READ_BYTES (1<<0)
#define SPIC_WRITE_BYTES (1<<1)
#define SPIC_DEBUG (1 << 7)

#if 1
void usleep(unsigned int usecs)
{
        unsigned long timeout = usecs_to_jiffies(usecs);

        while (timeout)
                timeout = schedule_timeout_interruptible(timeout);
}
#endif


#if defined USER_MODE || defined COMMAND_MODE
static unsigned int spi_wait_nsec = 0;
static int spic_busy_wait(void)
{
	do {
		if ((ra_inl(RT2880_SPISTAT_REG) & 0x01) == 0)
			return 0;
	} while (spi_wait_nsec >> 1);

	printk("%s: fail \n", __func__);
	return -1;
}
#elif defined BBU_MODE
static int bbu_spic_busy_wait(void)
{
	int n = 100000;
	do {
		if ((ra_inl(SPI_REG_CTL) & SPI_CTL_BUSY) == 0)
			return 0;
		udelay(1);
	} while (--n > 0);

	printk("%s: fail \n", __func__);
	return -1;
}
#endif // USER_MODE || COMMAND_MODE //


#ifdef USER_MODE
/*
 * @cmd: command and address
 * @n_cmd: size of command, in bytes
 * @buf: buffer into which data will be read/written
 * @n_buf: size of buffer, in bytes
 * @flag: tag as READ/WRITE
 *
 * @return: if write_onlu, -1 means write fail, or return writing counter.
 * @return: if read, -1 means read fail, or return reading counter.
 */
static int spic_transfer(const u8 *cmd, int n_cmd, u8 *buf, int n_buf, int flag)
{
	int retval = -1;
	ra_dbg("cmd(%x): %x %x %x %x , buf:%x len:%x, flag:%s \n",
			n_cmd, cmd[0], cmd[1], cmd[2], cmd[3],
			(buf)? (*buf) : 0, n_buf,
			(flag == SPIC_READ_BYTES)? "read" : "write");

	// assert CS and we are already CLK normal high
	ra_and(RT2880_SPICTL_REG, ~(SPICTL_SPIENA_HIGH));

	// write command
	for (retval = 0; retval < n_cmd; retval++) {
		ra_outl(RT2880_SPIDATA_REG, cmd[retval]);
		ra_or(RT2880_SPICTL_REG, SPICTL_STARTWR);
		if (spic_busy_wait()) {
			retval = -1;
			goto end_trans;
		}
	}

	// read / write  data
	if (flag & SPIC_READ_BYTES) {
		for (retval = 0; retval < n_buf; retval++) {
			ra_or(RT2880_SPICTL_REG, SPICTL_STARTRD);
			if (spic_busy_wait())
				goto end_trans;
			buf[retval] = (u8) ra_inl(RT2880_SPIDATA_REG);
		}

	}
	else if (flag & SPIC_WRITE_BYTES) {
		for (retval = 0; retval < n_buf; retval++) {
			ra_outl(RT2880_SPIDATA_REG, buf[retval]);
			ra_or(RT2880_SPICTL_REG, SPICTL_STARTWR);
			if (spic_busy_wait())
				goto end_trans;
		}
	}

end_trans:
	// de-assert CS and
	ra_or (RT2880_SPICTL_REG, (SPICTL_SPIENA_HIGH));

	return retval;
}

static int spic_read(const u8 *cmd, size_t n_cmd, u8 *rxbuf, size_t n_rx)
{
	return spic_transfer(cmd, n_cmd, rxbuf, n_rx, SPIC_READ_BYTES);
}

static int spic_write(const u8 *cmd, size_t n_cmd, const u8 *txbuf, size_t n_tx)
{
	return spic_transfer(cmd, n_cmd, (u8 *)txbuf, n_tx, SPIC_WRITE_BYTES);
}
#endif // USER_MODE //
#if defined (CONFIG_RALINK_MT7621) && (defined (CONFIG_FB_MEDIATEK_TRULY) || defined(CONFIG_FB_MEDIATEK_ILITEK))
spinlock_t  flash_lock;
#endif
void spic_init(void)
{
#if defined (CONFIG_RALINK_MT7621) && (defined (CONFIG_FB_MEDIATEK_TRULY) || defined(CONFIG_FB_MEDIATEK_ILITEK))
	spin_lock_init(&flash_lock);
#endif
#if defined USER_MODE || defined COMMAND_MODE

	// use normal(SPI) mode instead of GPIO mode
	ra_and(RALINK_REG_GPIOMODE, ~(1 << 1));

	// reset spi block
	ra_or(RT2880_RSTCTRL_REG, RSTCTRL_SPI_RESET);
	udelay(1);
	ra_and(RT2880_RSTCTRL_REG, ~RSTCTRL_SPI_RESET);

	// FIXME, clk_div should depend on spi-flash. 
	// mode 0 (SPICLKPOL = 0) & (RXCLKEDGE_FALLING = 0)
	// mode 3 (SPICLKPOL = 1) & (RXCLKEDGE_FALLING = 0)
	ra_outl(RT2880_SPICFG_REG, SPICFG_MSBFIRST | SPICFG_TXCLKEDGE_FALLING | CFG_CLK_DIV | SPICFG_SPICLKPOL );

	// set idle state
	ra_outl(RT2880_SPICTL_REG, SPICTL_HIZSDO | SPICTL_SPIENA_HIGH);

	spi_wait_nsec = (8 * 1000 / (128 / CFG_CLK_DIV) ) >> 1 ;
	//printk("spi_wait_nsec: %x \n", spi_wait_nsec);
#elif defined BBU_MODE
#if  defined(CONFIG_RALINK_MT7621) || defined(CONFIG_RALINK_MT7628) 
#else
	// enable SMC bank 0 alias addressing
	ra_or(RALINK_SYSCTL_BASE + 0x38, 0x80000000);
#endif
#endif
}


struct chip_info {
	char		*name;
	u8		id;
	u32		jedec_id;
	unsigned long	sector_size;
	unsigned int	n_sectors;
	char		addr4b;
};

static struct chip_info chips_data [] = {
	/* REVISIT: fill in JEDEC ids, for parts that have them */
	{ "AT25DF321",          0x1f, 0x47000000, 64 * 1024, 64,  0 },
	{ "AT26DF161",          0x1f, 0x46000000, 64 * 1024, 32,  0 },
	{ "FL016AIF",           0x01, 0x02140000, 64 * 1024, 32,  0 },
	{ "FL064AIF",           0x01, 0x02160000, 64 * 1024, 128, 0 },
	{ "MX25L1605D",         0xc2, 0x2015c220, 64 * 1024, 32,  0 },//MX25L1606E
	{ "MX25L3205D",         0xc2, 0x2016c220, 64 * 1024, 64,  0 },//MX25L3233F
	{ "MX25L6405D",         0xc2, 0x2017c220, 64 * 1024, 128, 0 },//MX25L6433F
	{ "MX25L12805D",        0xc2, 0x2018c220, 64 * 1024, 256, 0 },//MX25L12835F
	{ "MX25L25635E",        0xc2, 0x2019c220, 64 * 1024, 512, 1 },//MX25L25635F
	{ "MX25L51245G",        0xc2, 0x201ac220, 64 * 1024, 1024, 1 },
	{ "S25FL256S",          0x01, 0x02194D01, 64 * 1024, 512, 1 },
	{ "S25FL128P",          0x01, 0x20180301, 64 * 1024, 256, 0 },
	{ "S25FL129P",          0x01, 0x20184D01, 64 * 1024, 256, 0 },
	{ "S25FL164K",          0x01, 0x40170140, 64 * 1024, 128, 0 },
	{ "S25FL132K",          0x01, 0x40160140, 64 * 1024, 64,  0 },
	{ "S25FL032P",          0x01, 0x02154D00, 64 * 1024, 64,  0 },
	{ "S25FL064P",          0x01, 0x02164D00, 64 * 1024, 128, 0 },
	{ "S25FL116K",          0x01, 0x40150140, 64 * 1024, 32,  0 },
	{ "F25L64QA",           0x8c, 0x41170000, 64 * 1024, 128, 0 }, //ESMT
	{ "F25L32QA",           0x8c, 0x41168c41, 64 * 1024, 64,  0 }, //ESMT
	{ "EN25F16",            0x1c, 0x31151c31, 64 * 1024, 32,  0 },
	{ "EN25Q32B",           0x1c, 0x30161c30, 64 * 1024, 64,  0 },
	{ "EN25F32",            0x1c, 0x31161c31, 64 * 1024, 64,  0 },
	{ "EN25F64",            0x1c, 0x20171c20, 64 * 1024, 128, 0 },  // EN25P64
	{ "EN25Q64",            0x1c, 0x30171c30, 64 * 1024, 128, 0 },
	{ "W25Q32BV",           0xef, 0x40160000, 64 * 1024, 64,  0 },//W25Q32FV
	{ "W25X32VS",           0xef, 0x30160000, 64 * 1024, 64,  0 },
	{ "W25Q64BV",           0xef, 0x40170000, 64 * 1024, 128, 0 }, //S25FL064K //W25Q64FV
	{ "W25Q128BV",          0xef, 0x40180000, 64 * 1024, 256, 0 },//W25Q128FV
	{ "W25Q256FV",          0xef, 0x40190000, 64 * 1024, 512, 1 },
	{ "N25Q032A13ESE40F",   0x20, 0xba161000, 64 * 1024, 64,  0 },
	{ "N25Q064A13ESE40F",   0x20, 0xba171000, 64 * 1024, 128, 0 },
	{ "N25Q128A13ESE40F",   0x20, 0xba181000, 64 * 1024, 256, 0 },
	{ "N25Q256A",       	0x20, 0xba191000, 64 * 1024, 512, 1 },
	{ "MT25QL512AB",    	0x20, 0xba201044, 64 * 1024, 1024, 1 },
	{ "GD25Q32B",           0xC8, 0x40160000, 64 * 1024, 64,  0 },
	{ "GD25Q64B",           0xC8, 0x40170000, 64 * 1024, 128, 0 },
	{ "GD25Q128C",          0xC8, 0x40180000, 64 * 1024, 256, 0 },

};


struct flash_info {
	struct semaphore	lock;
	struct mtd_info		mtd;
	struct chip_info	*chip;
#ifdef TWO_SPI_FLASH
	struct chip_info        *chips[2];
	struct mtd_info         mtd2;
#endif
			 
	u8			command[5];
};

#ifdef TWO_SPI_FLASH
struct mtd_info *merged_mtd;
static struct mtd_info *ralink_mtd[2];
#endif
struct flash_info *flash = NULL;

const struct semaphore* FLASH_LOCK_GET(void){
	return &(flash->lock);
}

#ifdef BBU_MODE
#ifdef MORE_BUF_MODE
int bbu_mb_spic_trans(const u8 code, const u32 addr, u8 *buf, const size_t n_tx, const size_t n_rx, int flag, int lcd)
{
	if (lcd == LCD_USE){	

#if(1)
		u32 DATA0, DATA1, DATA2, DATA3, DATA4, DATA5, DATA6, DATA7, DATA8;
		sysRegWrite(SPI_REG_MASTER, 0x20038884);// CS1, morebuffer mode

		DATA0 = (*(buf + 2) << 24) | (*(buf + 1) << 16 ) | (*(buf + 0)  << 8 ) | (*(buf + 5));
		RT2880_REG(SPI_REG_OPCODE) = DATA0;

		ra_or(SPI_REG_MOREBUF, (32 << 24));
		DATA1 = (*(buf + 7)) << (3*8) | (*(buf + 8)) << (2*8) | (*(buf + 3)) << (1*8) | (*(buf + 4));
		RT2880_REG(SPI_REG_DATA(0)) = DATA1;
		DATA2 = (*(buf + 9)) << (3*8) | (*(buf + 10)) << (2*8) | (*(buf + 11)) << (1*8) | (*(buf + 6));
		RT2880_REG(SPI_REG_DATA(1)) = DATA2;
		DATA3 = (*(buf + 17)) << (3*8) | (*(buf + 12)) << (2*8) | (*(buf + 13)) << (1*8) | (*(buf + 14));
		RT2880_REG(SPI_REG_DATA(2)) = DATA3;
		DATA4 = (*(buf + 19)) << (3*8) | (*(buf + 20)) << (2*8) | (*(buf + 15)) << (1*8) | (*(buf + 16));
		RT2880_REG(SPI_REG_DATA(3)) = DATA4;
		DATA5 = (*(buf + 21)) << (3*8) | (*(buf + 22)) << (2*8) | (*(buf + 23)) << (1*8) | (*(buf + 18));
		RT2880_REG(SPI_REG_DATA(4)) = DATA5;
		DATA6 = (*(buf + 29)) << (3*8) | (*(buf + 24)) << (2*8) | (*(buf + 25)) << (1*8) | (*(buf + 26));
		RT2880_REG(SPI_REG_DATA(5)) = DATA6;
		DATA7 = (*(buf + 31)) << (3*8) | (*(buf + 32)) << (2*8) | (*(buf + 27)) << (1*8) | (*(buf + 28));
		RT2880_REG(SPI_REG_DATA(6)) = DATA7;
		DATA8 = (*(buf + 33)) << (3*8) | (*(buf + 34)) << (2*8) | (*(buf + 35)) << (1*8) | (*(buf + 30));
		RT2880_REG(SPI_REG_DATA(7)) = DATA8;
		ra_or(SPI_REG_MOREBUF, 256);
  
		ra_or(SPI_REG_CTL, SPI_CTL_START);	
		bbu_spic_busy_wait();
		ra_and(SPI_REG_MASTER, ~(1 << 2));		
		return 0;
#endif
		}else if (lcd == FLASH_USE){
#if defined (CONFIG_RALINK_MT7621) && (defined (CONFIG_FB_MEDIATEK_TRULY) || defined(CONFIG_FB_MEDIATEK_ILITEK))			
	spin_lock(&flash_lock);
#endif	
	u32 reg;
	int i, q, r;
	int rc = -1;
#if defined (CONFIG_RALINK_MT7621) && (defined (CONFIG_FB_MEDIATEK_TRULY) || defined(CONFIG_FB_MEDIATEK_ILITEK))
	bbu_spic_busy_wait();
  sysRegWrite(SPI_REG_MASTER, 0x58880);
#endif	  
	 
	if (flag != SPIC_READ_BYTES && flag != SPIC_WRITE_BYTES) {
		printk("we currently support more-byte-mode for reading and writing data only\n");
#if defined (CONFIG_RALINK_MT7621) && (defined (CONFIG_FB_MEDIATEK_TRULY) || defined(CONFIG_FB_MEDIATEK_ILITEK))
		spin_unlock(&flash_lock);
#endif
		return -1;
	}

	/* step 0. enable more byte mode */
	ra_or(SPI_REG_MASTER, (1 << 2));

	bbu_spic_busy_wait();

	/* step 1. set opcode & address, and fix cmd bit count to 32 (or 40) */
	if (flash && flash->chip->addr4b) {
		ra_and(SPI_REG_CTL, ~SPI_CTL_ADDREXT_MASK);
		ra_or(SPI_REG_CTL, (code << 24) & SPI_CTL_ADDREXT_MASK);
		ra_outl(SPI_REG_OPCODE, addr);
	}
	else
	{
		ra_outl(SPI_REG_OPCODE, (code << 24) & 0xff000000);
		ra_or(SPI_REG_OPCODE, (addr & 0xffffff));
	}
	ra_and(SPI_REG_MOREBUF, ~SPI_MBCTL_CMD_MASK);
	if (flash && flash->chip->addr4b)
		ra_or(SPI_REG_MOREBUF, (40 << 24));
	else
		ra_or(SPI_REG_MOREBUF, (32 << 24));

	/* step 2. write DI/DO data #0 ~ #7 */
	if (flag & SPIC_WRITE_BYTES) {
		if (buf == NULL) {
			printk("%s: write null buf\n", __func__);
			goto RET_MB_TRANS;
		}
		for (i = 0; i < n_tx; i++) {
			q = i / 4;
			r = i % 4;
			if (r == 0)
				ra_outl(SPI_REG_DATA(q), 0);
			ra_or(SPI_REG_DATA(q), (*(buf + i) << (r * 8)));
		}
	}

	/* step 3. set rx (miso_bit_cnt) and tx (mosi_bit_cnt) bit count */
	ra_and(SPI_REG_MOREBUF, ~SPI_MBCTL_TX_RX_CNT_MASK);
	ra_or(SPI_REG_MOREBUF, (n_rx << 3 << 12));
	ra_or(SPI_REG_MOREBUF, n_tx << 3);

	/* step 4. kick */
	ra_or(SPI_REG_CTL, SPI_CTL_START);

	/* step 5. wait spi_master_busy */
	bbu_spic_busy_wait();
	if (flag & SPIC_WRITE_BYTES) {
		rc = 0;
		goto RET_MB_TRANS;
	}

	/* step 6. read DI/DO data #0 */
	if (flag & SPIC_READ_BYTES) {
		if (buf == NULL) {
			printk("%s: read null buf\n", __func__);
#if defined (CONFIG_RALINK_MT7621) && (defined (CONFIG_FB_MEDIATEK_TRULY) || defined(CONFIG_FB_MEDIATEK_ILITEK))
			spin_unlock(&flash_lock);
#endif
			return -1;
		}
		for (i = 0; i < n_rx; i++) {
			q = i / 4;
			r = i % 4;
			reg = ra_inl(SPI_REG_DATA(q));
			*(buf + i) = (u8)(reg >> (r * 8));
		}
	}

	rc = 0;
RET_MB_TRANS:
	/* step #. disable more byte mode */
	ra_and(SPI_REG_MASTER, ~(1 << 2));
#if defined (CONFIG_RALINK_MT7621) && (defined (CONFIG_FB_MEDIATEK_TRULY) || defined(CONFIG_FB_MEDIATEK_ILITEK))
	spin_unlock(&flash_lock);
#endif
	return rc;
}

}
#endif // MORE_BUF_MODE //

int bbu_spic_trans(const u8 code, const u32 addr, u8 *buf, const size_t n_tx, const size_t n_rx, int flag, int lcd)
{
		
		if (lcd == LCD_USE){
		
   sysRegWrite(SPI_REG_MASTER, 0x20038880); // CS1, not morebuffer mode
   
		sysRegWrite(SPI_REG_OPCODE, addr);
		ra_and(SPI_REG_CTL, ~SPI_CTL_TX_RX_CNT_MASK);
		ra_or(SPI_REG_CTL, 1);
	/* step 4. kick */
		ra_or(SPI_REG_CTL, SPI_CTL_START);
	/* step 5. wait spi_master_busy */
		bbu_spic_busy_wait();
		
		return 0;
	}
#if defined (CONFIG_RALINK_MT7621) && defined (CONFIG_FB_MEDIATEK_TRULY)
	else if(lcd ==TRULY_USE){
	 u32 tmp1, tmp2, tmp3, DATA0;
	      sysRegWrite(SPI_REG_MASTER, 0x20038880);     
	/* step 0. enable more byte mode */

	tmp1 = (*(buf + 2) & 0xFC) | ((*(buf + 1) & 0xC0) >> 6);
	tmp2 = ((*(buf + 1) & 0x3C) << 2) | ( (*(buf + 0) & 0xF0) >> 4);
	tmp3 =  (*(buf + 0) & 0x0F) << 4;
	
	
	DATA0 = (0x72) | (tmp1 << 24) | (tmp2 << 16) | (tmp3<<8);
  bbu_spic_busy_wait();
 	RT2880_REG(SPI_REG_OPCODE) = DATA0; 
	
	/* step 3. set mosi_byte_cnt */

	ra_and(SPI_REG_CTL, ~SPI_CTL_TX_RX_CNT_MASK);

	ra_or(SPI_REG_CTL, 4);
 
	/* step 4. kick */
	ra_or(SPI_REG_CTL, SPI_CTL_START);		
	return 0;
	}
#endif	
	else if (lcd == FLASH_USE){
#if defined (CONFIG_RALINK_MT7621) && defined (CONFIG_FB_MEDIATEK_TRULY)
		spin_lock(&flash_lock);
#endif			
	  u32 reg;
		

	bbu_spic_busy_wait();
#if defined (CONFIG_RALINK_MT7621) && defined (CONFIG_FB_MEDIATEK_TRULY) && defined(CONFIG_FB_MEDIATEK_ILITEK)
	sysRegWrite(SPI_REG_MASTER, 0x58880);
#endif
	/* step 1. set opcode & address */
	if (flash && flash->chip->addr4b) {
		ra_and(SPI_REG_CTL, ~SPI_CTL_ADDREXT_MASK);
		ra_or(SPI_REG_CTL, addr & SPI_CTL_ADDREXT_MASK);
	}
	ra_outl(SPI_REG_OPCODE, ((addr & 0xffffff) << 8));
	ra_or(SPI_REG_OPCODE, code);

#if defined(RD_MODE_QUAD) || defined(RD_MODE_DUAL)
	if (flag & SPIC_READ_BYTES)
		ra_outl(SPI_REG_DATA0, 0); // clear data bit for dummy bits in Dual/Quad IO Read
#endif
	/* step 2. write DI/DO data #0 */
	if (flag & SPIC_WRITE_BYTES) {
		if (buf == NULL) {
			printk("%s: write null buf\n", __func__);
#if defined (CONFIG_RALINK_MT7621) && defined (CONFIG_FB_MEDIATEK_TRULY)
			spin_unlock(&flash_lock);
#endif
			return -1;
		}
		ra_outl(SPI_REG_DATA0, 0);
		switch (n_tx) {
		case 8:
			ra_or(SPI_REG_DATA0, (*(buf+3) << 24));
		case 7:
			ra_or(SPI_REG_DATA0, (*(buf+2) << 16));
		case 6:
			ra_or(SPI_REG_DATA0, (*(buf+1) << 8));
		case 5:
			ra_or(SPI_REG_DATA0, *buf);
			break;
		case 3:
			reg = ra_inl(SPI_REG_CTL);
			if (((reg & (0x3<<19)) == (0x3 << 19)) && (flash && flash->chip->addr4b)) 
			{
				ra_and(SPI_REG_CTL, ~SPI_CTL_ADDREXT_MASK);
				ra_or(SPI_REG_CTL, (*buf << 24) & SPI_CTL_ADDREXT_MASK);
				ra_and(SPI_REG_OPCODE, 0xff);
				ra_or(SPI_REG_OPCODE, (*(buf+1) & 0xff) << 24);
			}
			else
			{
				ra_and(SPI_REG_OPCODE, 0xff);
				ra_or(SPI_REG_OPCODE, (*buf & 0xff) << 24);
				ra_or(SPI_REG_OPCODE, (*(buf+1) & 0xff) << 16);
			}
			break;
		case 2:
			reg = ra_inl(SPI_REG_CTL);
			if (((reg & (0x3<<19)) == (0x3 << 19)) && (flash && flash->chip->addr4b)) 
			{
				ra_and(SPI_REG_CTL, ~SPI_CTL_ADDREXT_MASK);
				ra_or(SPI_REG_CTL, (*buf << 24) & SPI_CTL_ADDREXT_MASK);
			}
			else
			{
				ra_and(SPI_REG_OPCODE, 0xff);
				ra_or(SPI_REG_OPCODE, (*buf & 0xff) << 24);
			}
			break;
		default:
			printk("%s: fixme, write of length %d\n", __func__, n_tx);
#if defined (CONFIG_RALINK_MT7621) && defined (CONFIG_FB_MEDIATEK_TRULY)
			spin_unlock(&flash_lock);
#endif
			return -1;
		}
	}


	/* step 3. set mosi_byte_cnt */
	ra_and(SPI_REG_CTL, ~SPI_CTL_TX_RX_CNT_MASK);
	ra_or(SPI_REG_CTL, (n_rx << 4));
	if (flash && flash->chip->addr4b && n_tx >= 4)
		ra_or(SPI_REG_CTL, (n_tx + 1));
	else
		ra_or(SPI_REG_CTL, n_tx);


	/* step 4. kick */
	ra_or(SPI_REG_CTL, SPI_CTL_START);

	/* step 5. wait spi_master_busy */
	bbu_spic_busy_wait();
	if (flag & SPIC_WRITE_BYTES){
#if defined (CONFIG_RALINK_MT7621) && defined (CONFIG_FB_MEDIATEK_TRULY)
		spin_unlock(&flash_lock);
#endif
		return 0;
  }
	/* step 6. read DI/DO data #0 */
	if (flag & SPIC_READ_BYTES) {
		if (buf == NULL) {
			printk("%s: read null buf\n", __func__);
#if defined (CONFIG_RALINK_MT7621) && defined (CONFIG_FB_MEDIATEK_TRULY)
			spin_unlock(&flash_lock);
#endif
			return -1;
		}
		reg = ra_inl(SPI_REG_DATA0);
		switch (n_rx) {
		case 4:
			*(buf+3) = (u8)(reg >> 24);
		case 3:
			*(buf+2) = (u8)(reg >> 16);
		case 2:
			*(buf+1) = (u8)(reg >> 8);
		case 1:
			*buf = (u8)reg;
			break;
		default:
			printk("%s: fixme, read of length %d\n", __func__, n_rx);
#if defined (CONFIG_RALINK_MT7621) && defined (CONFIG_FB_MEDIATEK_TRULY)
			spin_unlock(&flash_lock);
#endif
			return -1;
		}
	}
#if defined (CONFIG_RALINK_MT7621) && defined (CONFIG_FB_MEDIATEK_TRULY)
  spin_unlock(&flash_lock);
#endif
	return 0;
}
}
#endif // BBU_MODE //

static int raspi_read_rg(u8 code, u8 *val);
static int raspi_write_rg(u8 code, u8 *val);
static int raspi_wait_ready(int sleep_ms);
/*
 * read SPI flash device ID
 */
static int raspi_read_devid(u8 *rxbuf, int n_rx)
{
	u8 code = OPCODE_RDID;
	int retval;

#ifdef USER_MODE
	retval = spic_read(&code, 1, rxbuf, n_rx);
#elif defined BBU_MODE
	retval = bbu_spic_trans(code, 0, rxbuf, 1, 3, SPIC_READ_BYTES, FLASH_USE);
	if (!retval)
		retval = n_rx;
#endif
	if (retval != n_rx) {
		printk("%s: ret: %x\n", __func__, retval);
		return retval;
	}
	return retval;
}



static int raspi_read_sr(u8 *val)
{
	return raspi_read_rg(OPCODE_RDSR, val);
}

static int raspi_write_sr(u8 *val)
{
	return raspi_write_rg(OPCODE_WRSR, val);
}

/*
 * Read the status register, returning its value in the location
 */
static int raspi_read_rg(u8 code, u8 *val)
{
	ssize_t retval;

#ifdef USER_MODE
	retval = spic_read(&code, 1, val, 1);
#elif defined BBU_MODE
	retval = bbu_spic_trans(code, 0, val, 1, 1, SPIC_READ_BYTES, FLASH_USE);
	return retval;
#endif
	if (retval != 1) {
		printk("%s: ret: %x\n", __func__, retval);
		return -EIO;
	}


	return 0;
}

/*
 * write status register
 */
static int raspi_write_rg(u8 code, u8 *val)
{
	ssize_t retval;

#ifdef USER_MODE
	retval = spic_write(&code, 1, val, 1);
#elif defined BBU_MODE
	{
		// put the value to be written in address register, so it will be transfered
		u32 address = (*val) << 24;
		retval = bbu_spic_trans(code, address, val, 2, 0, SPIC_WRITE_BYTES, FLASH_USE);
	}
	return retval;
#endif
	if (retval != 1) {
		printk("%s: ret: %x\n", __func__, retval);
		return -EIO;
	}

	return 0;
}

#if defined(RD_MODE_DUAL) || defined(RD_MODE_QUAD)

static int raspi_write_rg16(u8 code, u8 *val)
{
	ssize_t retval;

#ifdef USER_MODE
	retval = spic_write(&code, 1, val, 2);
#elif defined BBU_MODE
	{
		// put the value to be written in address register, so it will be transfered
		u32 address = (*val) << 24;
		address |= (*(val+1)) << 16;
		retval = bbu_spic_trans(code, address, val, 3, 0, SPIC_WRITE_BYTES, FLASH_USE);
	}
	return retval;
#endif
	if (retval != 1) {
		printk("%s: ret: %x\n", __func__, retval);
		return -EIO;
	}

	return 0;
}
#endif

static inline int raspi_write_enable(void);
static int raspi_4byte_mode(int enable)
{
	ssize_t retval;
	
	raspi_wait_ready(1);
	
	
	if (flash->chip->id == 0x1) // Spansion
	{
		u8 br, br_cfn; // bank register
#ifdef USER_MODE
		if (enable)
		{
			br = 0x81;
			ra_or(RT2880_SPICFG_REG, SPICFG_ADDRMODE);
		}
		else
		{
			br = 0x0;
			ra_and(RT2880_SPICFG_REG, ~(SPICFG_ADDRMODE));
		}
    	
#elif defined BBU_MODE
		if (enable)
		{
			ra_or(SPI_REG_CTL, 0x3 << 19);
			ra_or(SPI_REG_Q_CTL, 0x3 << 8);
			br = 0x81;
		}
		else
		{
			ra_and(SPI_REG_CTL, ~SPI_CTL_SIZE_MASK);
			ra_or(SPI_REG_CTL, 0x2 << 19);
			ra_and(SPI_REG_Q_CTL, ~(0x3 << 8));
			ra_or(SPI_REG_Q_CTL, 0x2 << 8);
			br = 0x0;
		}
#endif
		raspi_write_rg(OPCODE_BRWR, &br);
		raspi_wait_ready(1);
		raspi_read_rg(OPCODE_BRRD, &br_cfn);
		if (br_cfn != br)
		{
			printk("4B mode switch failed %d, %x, %x\n", enable, br_cfn, br);
			return -1;
		}

	}
	else
	{
		u8 code;
	
		code = enable? 0xB7 : 0xE9; /* B7: enter 4B, E9: exit 4B */

#ifdef USER_MODE
		if (enable)
			ra_or(RT2880_SPICFG_REG, SPICFG_ADDRMODE);
		else
			ra_and(RT2880_SPICFG_REG, ~(SPICFG_ADDRMODE));
		retval = spic_read(&code, 1, 0, 0);
#elif defined BBU_MODE
		if (enable) {
			ra_or(SPI_REG_CTL, 0x3 << 19);
			ra_or(SPI_REG_Q_CTL, 0x3 << 8);

		}
		else {
			ra_and(SPI_REG_CTL, ~SPI_CTL_SIZE_MASK);
			ra_or(SPI_REG_CTL, 0x2 << 19);
			ra_and(SPI_REG_Q_CTL, ~(0x3 << 8));
			ra_or(SPI_REG_Q_CTL, 0x2 << 8);
		}
		retval = bbu_spic_trans(code, 0, NULL, 1, 0, 0, FLASH_USE);
#endif
		// for Winbond's W25Q256FV, need to clear extend address register
		if ((!enable) && (flash->chip->id == 0xef))
		{
			code = 0x0;
			raspi_write_enable();
			raspi_write_rg(0xc5, &code);
		}

		if (retval != 0) {
			printk("%s: ret: %x\n", __func__, retval);
			return -1;
		}

	}

	return 0;
}

/*
 * Set write enable latch with Write Enable command.
 * Returns negative if error occurred.
 */
static inline int raspi_write_enable(void)
{
	u8 code = OPCODE_WREN;

#ifdef USER_MODE
	return spic_write(&code, 1, NULL, 0);
#elif defined BBU_MODE
	return bbu_spic_trans(code, 0, NULL, 1, 0, 0, FLASH_USE);
#endif
}

/*
 * Set all sectors (global) unprotected if they are protected.
 * Returns negative if error occurred.
 */
static inline int raspi_unprotect(void)
{
	u8 sr = 0;

	if (raspi_read_sr(&sr) < 0) {
		printk("%s: read_sr fail: %x\n", __func__, sr);
		return -1;
	}

	if ((sr & (SR_BP0 | SR_BP1 | SR_BP2)) != 0) {
		sr = 0;
		raspi_write_sr(&sr);
	}
	return 0;
}

/*
 * Service routine to read status register until ready, or timeout occurs.
 * Returns non-zero if error.
 */
static int raspi_wait_ready(int sleep_ms)
{
	int count;
	int sr = 0;

	/*int timeout = sleep_ms * HZ / 1000;
	while (timeout) 
		timeout = schedule_timeout (timeout);*/

	/* one chip guarantees max 5 msec wait here after page writes,
	 * but potentially three seconds (!) after page erase.
	 */
	for (count = 0; count < ((sleep_ms+1) *1000 * 500); count++) {
		if ((raspi_read_sr((u8 *)&sr)) < 0)
			break;
		else if (!(sr & SR_WIP)) {
			return 0;
		}

		udelay(1);
		/* REVISIT sometimes sleeping would be best */
	}

	printk("%s: read_sr fail: %x\n", __func__, sr);
	return -EIO;
}

static int raspi_wait_sleep_ready(int sleep_ms)
{
	int count;
	int sr = 0;

	/*int timeout = sleep_ms * HZ / 1000;
	while (timeout) 
		timeout = schedule_timeout (timeout);*/

	/* one chip guarantees max 5 msec wait here after page writes,
	 * but potentially three seconds (!) after page erase.
	 */
	for (count = 0; count < ((sleep_ms+1) *1000); count++) {
		if ((raspi_read_sr((u8 *)&sr)) < 0)
			break;
		else if (!(sr & SR_WIP)) {
			return 0;
		}

		usleep(1);
		/* REVISIT sometimes sleeping would be best */
	}

	printk("%s: read_sr fail: %x\n", __func__, sr);
	return -EIO;
}

/*
 * Erase one sector of flash memory at offset ``offset'' which is any
 * address within the sector which should be erased.
 *
 * Returns 0 if successful, non-zero otherwise.
 */
static int raspi_erase_sector(u32 offset)
{

	/* Wait until finished previous write command. */
	if (raspi_wait_ready(3))
		return -EIO;

	/* Send write enable, then erase commands. */
	raspi_write_enable();
	raspi_unprotect();

#ifdef USER_MODE
	if (flash->chip->addr4b) {
		raspi_4byte_mode(1);
		flash->command[0] = OPCODE_SE;
		flash->command[1] = offset >> 24;
		flash->command[2] = offset >> 16;
		flash->command[3] = offset >> 8;
		flash->command[4] = offset;
		spic_write(flash->command, 5, 0 , 0);
		raspi_wait_sleep_ready(950);
		raspi_4byte_mode(0);
		return 0;
	}

	/* Set up command buffer. */
	flash->command[0] = OPCODE_SE;
	flash->command[1] = offset >> 16;
	flash->command[2] = offset >> 8;
	flash->command[3] = offset;

	spic_write(flash->command, 4, 0 , 0);
	raspi_wait_sleep_ready(950);
#elif defined BBU_MODE
	if (flash->chip->addr4b)
		raspi_4byte_mode(1);
	raspi_write_enable();
	bbu_spic_trans(STM_OP_SECTOR_ERASE, offset, NULL, 4, 0, 0, FLASH_USE);
	//raspi_wait_ready(950);
	raspi_wait_sleep_ready(950);
	if (flash->chip->addr4b)
		raspi_4byte_mode(0);
#endif
	return 0;
}

/*
 * SPI device driver setup and teardown
 */
struct chip_info *chip_prob(void)
{
	struct chip_info *info, *match;
	u8 buf[5];
	u32 jedec, weight;
	int i;

	raspi_read_devid(buf, 5);
	jedec = (u32)((u32)(buf[1] << 24) | ((u32)buf[2] << 16) | ((u32)buf[3] <<8) | (u32)buf[4]);

#ifdef BBU_MODE
	printk("flash manufacture id: %x, device id %x %x\n", buf[0], buf[1], buf[2]);
#else
	printk("deice id : %x %x %x %x %x (%x)\n", buf[0], buf[1], buf[2], buf[3], buf[4], jedec);
#endif
	

	// FIXME, assign default as AT25D
	weight = 0xffffffff;
	match = &chips_data[0];
	for (i = 0; i < ARRAY_SIZE(chips_data); i++) {
		info = &chips_data[i];
		if (info->id == buf[0]) {
#ifdef BBU_MODE
			if ((u8)(info->jedec_id >> 24 & 0xff) == buf[1] &&
			    (u8)(info->jedec_id >> 16 & 0xff) == buf[2])
#else
			if (info->jedec_id == jedec)
#endif
				return info;

			if (weight > (info->jedec_id ^ jedec)) {
				weight = info->jedec_id ^ jedec;
				match = info;
			}
		}
	}
	printk("Warning: un-recognized chip ID, please update SPI driver!\n");

	return match;
}

#if 0
int raspi_set_lock (struct mtd_info *mtd, loff_t to, size_t len, int set)
{
	u32 page_offset, page_size;
	int retval;

	/*  */
	while (len > 0) {
		/* FIXME: 4b mode ? */
		/* write the next page to flash */
		flash->command[0] = (set == 0)? 0x39 : 0x36;
		flash->command[1] = to >> 16;
		flash->command[2] = to >> 8;
		flash->command[3] = to;

		raspi_wait_ready(1);
			
		raspi_write_enable();

#ifdef USER_MODE
		retval = spic_write(flash->command, 4, 0, 0);
#elif defined BBU_MODE
		retval = 0;
		//FIXME
#endif
			
		if (retval < 0) {
			return -EIO;
		}
			
		page_offset = (to & (mtd->erasesize-1));
		page_size = mtd->erasesize - page_offset;
				
		len -= mtd->erasesize;
		to += mtd->erasesize;
	}

	return 0;
	
}
#endif

static int ramtd_read(struct mtd_info *mtd, loff_t from, size_t len,
	size_t *retlen, u_char *buf);

/*
 * MTD implementation
 */

/*
 * Erase an address range on the flash chip.  The address range may extend
 * one or more erase sectors.  Return an error is there is a problem erasing.
 */
static int ramtd_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	u32 addr,len;

	//printk("%s: addr:%x len:%x\n", __func__, instr->addr, instr->len);
	/* sanity checks */
	if (instr->addr + instr->len > flash->mtd.size)
		return -EINVAL;

#if 0  //FIXME - undefined reference to `__umoddi3'
	if ((instr->addr % mtd->erasesize) != 0
			|| (instr->len % mtd->erasesize) != 0) {
		return -EINVAL;
	}
#endif

	addr = instr->addr;
	len = instr->len;

  	down(&flash->lock);
#ifdef TWO_SPI_FLASH
	if (mtd == ralink_mtd[1])
 	{
		ra_or(SPI_REG_MASTER, (1 << 29));
	}
#endif
	/* now erase those sectors */
	while (len > 0) {
		if (raspi_erase_sector(addr)) {
			instr->state = MTD_ERASE_FAILED;
#ifdef TWO_SPI_FLASH
			ra_and(SPI_REG_MASTER, ~(1 << 29));
#endif
			up(&flash->lock);
#if defined (CONFIG_RALINK_MT7621) && (defined (CONFIG_FB_MEDIATEK_TRULY) || defined(CONFIG_FB_MEDIATEK_ILITEK))
			spin_unlock(&flash_lock);
#endif
			return -EIO;
		}

		addr += mtd->erasesize;
		len -= mtd->erasesize;
	}

#ifdef TWO_SPI_FLASH
	ra_and(SPI_REG_MASTER, ~(1 << 29));
#endif
  	up(&flash->lock);

	instr->state = MTD_ERASE_DONE;
	mtd_erase_callback(instr);

	return 0;
}

/*
 * Read an address range from the flash chip.  The address range
 * may be any size provided it is within the physical boundaries.
 */
static int ramtd_read(struct mtd_info *mtd, loff_t from, size_t len,
	size_t *retlen, u_char *buf)
{
	size_t rdlen = 0;

	ra_dbg("%s: from:%x len:%x \n", __func__, from, len);

	/* sanity checks */
	if (len == 0)
		return 0;

	if (from + len > flash->mtd.size)
		return -EINVAL;

	/* Byte count starts at zero. */
	if (retlen)
		*retlen = 0;

	down(&flash->lock);

	/* Wait till previous write/erase is done. */
	if (raspi_wait_ready(1)) {
		/* REVISIT status return?? */
		up(&flash->lock);
		return -EIO;
	}
#ifdef TWO_SPI_FLASH
	if (mtd == ralink_mtd[1])
	{
		ra_or(SPI_REG_MASTER, (1 << 29));
	}
#endif

#ifdef USER_MODE

	/* Set up the write data buffer. */
	flash->command[0] = OPCODE_READ;
	if (flash->chip->addr4b) {
		raspi_4byte_mode(1);
		flash->command[1] = from >> 24;
		flash->command[2] = from >> 16;
		flash->command[3] = from >> 8;
		flash->command[4] = from;
		rdlen = spic_read(flash->command, 5, buf, len);
		raspi_4byte_mode(0);
	}
	else
	{
		flash->command[1] = from >> 16;
		flash->command[2] = from >> 8;
		flash->command[3] = from;
		rdlen = spic_read(flash->command, 4, buf, len);
	}
#elif defined BBU_MODE
	if (flash->chip->addr4b)
		raspi_4byte_mode(1);
#ifndef MORE_BUF_MODE
#if defined(RD_MODE_DUAL)
	// serial mode = dual
	if (flash->chip->id == 0x1) // Spansion
	{
		u8 reg[2], cr;
		raspi_read_rg(OPCODE_RDCR, &reg[1]);
		if (reg[1] & (1 << 1))
		{
			reg[1] &= ~(1 << 1);
			raspi_read_sr(&reg[0]);
                        raspi_write_enable();
                        raspi_write_rg16(OPCODE_WRSR, &reg[0]);
			raspi_wait_ready(1);
			raspi_read_rg(OPCODE_RDCR, &cr);
			if (reg[1] != cr)
				printk("warning: set quad failed %x %x\n", reg[1], cr);
		}
	}
	else // MXIC
	{
                u8 sr;
		raspi_read_sr(&sr);
                if ((sr & (1 << 6)))
                {
			u8 get_sr;
                        sr &= ~(1 << 6);
                        raspi_write_enable();
                        raspi_write_sr(&sr);
			raspi_wait_ready(1);
			raspi_read_sr(&get_sr);
			if (get_sr != sr)
				printk("warning: sr write failed %x %x\n", sr, get_sr);
                }
 	}
	ra_and(SPI_REG_MASTER, ~3);
	ra_or(SPI_REG_MASTER, 1);
#elif defined(RD_MODE_QUAD)
	// serial mode = quad
	if (flash->chip->id == 0x1) // Spansion
	{
		u8 reg[2], cr;
		raspi_read_rg(OPCODE_RDCR, &reg[1]);
		if ((reg[1] & (1 << 1)) == 0)
		{
			reg[1] |= (1 << 1);
			raspi_read_sr(&reg[0]);
                        raspi_write_enable();
                        raspi_write_rg16(OPCODE_WRSR, &reg[0]);
			raspi_wait_ready(1); 
			raspi_read_rg(OPCODE_RDCR, &cr);
			if (reg[1] != cr)
				printk("warning: set quad failed %x %x\n", reg[1], cr);
		}
	}
	else // MXIC
	{
                u8 sr, sr2;
		raspi_read_sr(&sr);
		sr2 = sr;
                if ((sr & (1 << 6)) == 0)
                {
			u8 get_sr;
                        sr |= (1 << 6);
                        raspi_write_enable();
                        raspi_write_sr(&sr);
			raspi_wait_ready(1);
			raspi_read_sr(&get_sr);
			if (get_sr != sr)
				printk("warning: quad sr write failed %x %x %x\n", sr, get_sr, sr2);
			
                }
 	}
	ra_and(SPI_REG_MASTER, ~3);
	ra_or(SPI_REG_MASTER, 2);

#endif
#endif
	do {
		int rc, more;
#ifdef MORE_BUF_MODE
		more = 32;
#else
		more = 4;
#endif
		if (len - rdlen <= more) {
#ifdef MORE_BUF_MODE
			rc = bbu_mb_spic_trans(STM_OP_RD_DATA, from, (buf+rdlen), 0, (len-rdlen), SPIC_READ_BYTES, FLASH_USE);
#else
#if defined(RD_MODE_DOR)
			rc = bbu_spic_trans(OPCODE_DOR, from, (buf+rdlen), 5, (len-rdlen), SPIC_READ_BYTES, FLASH_USE);
#elif defined(RD_MODE_DIOR)
			rc = bbu_spic_trans(OPCODE_DIOR, from, (buf+rdlen), 5, (len-rdlen), SPIC_READ_BYTES, FLASH_USE);
#elif defined(RD_MODE_QOR)
			rc = bbu_spic_trans(OPCODE_QOR, from, (buf+rdlen), 5, (len-rdlen), SPIC_READ_BYTES, FLASH_USE);
#elif defined(RD_MODE_QIOR)
			rc = bbu_spic_trans(OPCODE_QIOR, from, (buf+rdlen), 7, (len-rdlen), SPIC_READ_BYTES, FLASH_USE);
#else
			rc = bbu_spic_trans(STM_OP_RD_DATA, from, (buf+rdlen), 4, (len-rdlen), SPIC_READ_BYTES, FLASH_USE);
#endif
#endif
			if (rc != 0) {
				printk("%s: failed\n", __func__);
				break;
			}
			rdlen = len;
		}
		else {
#ifdef MORE_BUF_MODE
			rc = bbu_mb_spic_trans(STM_OP_RD_DATA, from, (buf+rdlen), 0, more, SPIC_READ_BYTES, FLASH_USE);
#else
#if defined(RD_MODE_DOR)
			rc = bbu_spic_trans(OPCODE_DOR, from, (buf+rdlen), 5, more, SPIC_READ_BYTES, FLASH_USE);
#elif defined(RD_MODE_DIOR)
			rc = bbu_spic_trans(OPCODE_DIOR, from, (buf+rdlen), 5, more, SPIC_READ_BYTES, FLASH_USE);
#elif defined(RD_MODE_QOR)
			rc = bbu_spic_trans(OPCODE_QOR, from, (buf+rdlen), 5, more, SPIC_READ_BYTES, FLASH_USE);
#elif defined(RD_MODE_QIOR)
			rc = bbu_spic_trans(OPCODE_QIOR, from, (buf+rdlen), 7, more, SPIC_READ_BYTES, FLASH_USE);
#else
			rc = bbu_spic_trans(STM_OP_RD_DATA, from, (buf+rdlen), 4, more, SPIC_READ_BYTES, FLASH_USE);
#endif
#endif
			if (rc != 0) {
				printk("%s: failed\n", __func__);
				break;
			}
			rdlen += more;
			from += more;
		}
	} while (rdlen < len);

#ifndef MORE_BUF_MODE
#if defined(RD_MODE_DUAL) || defined(RD_MODE_QUAD)
	// serial mode = normal
	ra_and(SPI_REG_MASTER, ~3);
#if defined(RD_MODE_QUAD)
	// serial mode = quad
	if (flash->chip->id == 0x1) // Spansion
	{
		u8 reg[2], cr;
		raspi_read_rg(OPCODE_RDCR, &reg[1]);
		if (reg[1] & (1 << 1))
		{
			reg[1] &= ~(1 << 1);
			raspi_read_sr(&reg[0]);
                        raspi_write_enable();
                        raspi_write_rg16(OPCODE_WRSR, &reg[0]);
			raspi_wait_ready(1);
			raspi_read_rg(OPCODE_RDCR, &cr);
			if (reg[1] != cr)
				printk("warning: set quad failed %x %x\n", reg[1], cr);
		}
	}
	else // MXIC
	{
                u8 sr;
		raspi_read_sr(&sr);
                if ((sr & (1 << 6)))
                {
                        sr &= ~(1 << 6);
                        raspi_write_enable();
                        raspi_write_sr(&sr);
			raspi_wait_ready(1);
                }
 	}

#endif
#endif
#endif // MORE_BUF_MODE

	if (flash->chip->addr4b)
		raspi_4byte_mode(0);
#endif
#ifdef TWO_SPI_FLASH
	ra_and(SPI_REG_MASTER, ~(1 << 29));
#endif
  	up(&flash->lock);

	if (retlen) 
		*retlen = rdlen;

	if (rdlen != len) 
		return -EIO;

	return 0;
}

inline int ramtd_lock (struct mtd_info *mtd, loff_t to, uint64_t len)
{
	//return raspi_set_lock(mtd, to, len, 1);
	return 0;
}

inline int ramtd_unlock (struct mtd_info *mtd, loff_t to, uint64_t len)
{
	//return raspi_set_lock(mtd, to, len, 0);
	return 0;
}


/*
 * Write an address range to the flash chip.  Data must be written in
 * FLASH_PAGESIZE chunks.  The address range may be any size provided
 * it is within the physical boundaries.
 */
static int ramtd_write(struct mtd_info *mtd, loff_t to, size_t len,
	size_t *retlen, const u_char *buf)
{
	u32 page_offset, page_size;
	int rc = 0;
#if defined BBU_MODE
	int wrto, wrlen, more;
	char *wrbuf;
#endif
	int count = 0;

	ra_dbg("%s: to:%x len:%x \n", __func__, to, len);
	if (retlen)
		*retlen = 0;

	/* sanity checks */
	if (len == 0)
		return 0;
	if (to + len > flash->mtd.size)
		return -EINVAL;

  	down(&flash->lock);

	/* Wait until finished previous write command. */
	if (raspi_wait_ready(2)) {
		up(&flash->lock);
		return -1;
	}
#ifdef TWO_SPI_FLASH
	if (mtd == ralink_mtd[1])
	{
		ra_or(SPI_REG_MASTER, (1 << 29));
	}
#endif
#ifdef USER_MODE
	/* Set up the opcode in the write buffer. */
	flash->command[0] = OPCODE_PP;
	if (flash->chip->addr4b) {
		flash->command[1] = to >> 24;
		flash->command[2] = to >> 16;
		flash->command[3] = to >> 8;
		flash->command[4] = to;
	}
	else
	{
		flash->command[1] = to >> 16;
		flash->command[2] = to >> 8;
		flash->command[3] = to;
	}
#endif

	/* what page do we start with? */
	page_offset = to % FLASH_PAGESIZE;

	if (flash->chip->addr4b)
		raspi_4byte_mode(1);

	/* write everything in PAGESIZE chunks */
	while (len > 0) {
		page_size = min_t(size_t, len, FLASH_PAGESIZE-page_offset);
		page_offset = 0;

		/* write the next page to flash */
#ifdef USER_MODE
		if (flash->chip->addr4b) {
			flash->command[1] = to >> 24;
			flash->command[2] = to >> 16;
			flash->command[3] = to >> 8;
			flash->command[4] = to;
		}
		else
		{
			flash->command[1] = to >> 16;
			flash->command[2] = to >> 8;
			flash->command[3] = to;
		}
#endif

		raspi_wait_ready(3);
		raspi_write_enable();
		raspi_unprotect();

#ifdef USER_MODE

		if (flash->chip->addr4b)
			rc = spic_write(flash->command, 5, buf, page_size);
		else
			rc = spic_write(flash->command, 4, buf, page_size);

#elif defined BBU_MODE
		wrto = to;
		wrlen = page_size;
		wrbuf = (char *)buf;
		rc = wrlen;
		do {
#ifdef MORE_BUF_MODE
			more = 32;
#else
			more = 4;
#endif
			if (wrlen <= more) {
#ifdef MORE_BUF_MODE
				bbu_mb_spic_trans(STM_OP_PAGE_PGRM, wrto, wrbuf, wrlen, 0, SPIC_WRITE_BYTES, FLASH_USE);
#else
				bbu_spic_trans(STM_OP_PAGE_PGRM, wrto, wrbuf, wrlen+4, 0, SPIC_WRITE_BYTES,FLASH_USE);
#endif
				wrlen = 0;
			}
			else {
#ifdef MORE_BUF_MODE
				bbu_mb_spic_trans(STM_OP_PAGE_PGRM, wrto, wrbuf, more, 0, SPIC_WRITE_BYTES, FLASH_USE);
#else
				bbu_spic_trans(STM_OP_PAGE_PGRM, wrto, wrbuf, more+4, 0, SPIC_WRITE_BYTES,FLASH_USE);
#endif
				wrto += more;
				wrlen -= more;
				wrbuf += more;
			}
			if (wrlen > 0) {
				raspi_wait_ready(100);
				raspi_write_enable();
			}
		} while (wrlen > 0);
#endif
		//printk("%s : to:%llx page_size:%x ret:%x\n", __func__, to, page_size, rc);

		if (rc > 0) {
			if (retlen)
				*retlen += rc;
			if (rc < page_size) {
#ifdef TWO_SPI_FLASH
				ra_and(SPI_REG_MASTER, ~(1 << 29));
#endif
				up(&flash->lock);
				printk("%s: rc:%x return:%x page_size:%x \n", 
				       __func__, rc, rc, page_size);
				return -EIO;
			}
		}

		len -= page_size;
		to += page_size;
		buf += page_size;
		count++;
		if ((count & 0xf) == 0)
			raspi_wait_sleep_ready(1);
			
	}

	raspi_wait_ready(100);

	if (flash->chip->addr4b)
		raspi_4byte_mode(0);
#ifdef TWO_SPI_FLASH
	ra_and(SPI_REG_MASTER, ~(1 << 29));
#endif
	up(&flash->lock);

	return 0;
}

#ifdef TWO_SPI_FLASH
static int ramtd_erase_2(struct mtd_info *mtd, struct erase_info *instr)
{
	int ret;
	ra_or(SPI_REG_MASTER, (1 << 29));
	flash->chip = flash->chips[1];
	ret = ramtd_erase(mtd, instr);
	flash->chip = flash->chips[0];
	ra_and(SPI_REG_MASTER, ~(1 << 29));
	return ret;
}

static int ramtd_read_2(struct mtd_info *mtd, loff_t from, size_t len,
		        size_t *retlen, u_char *buf)
{
	int ret;
	ra_or(SPI_REG_MASTER, (1 << 29));
	flash->chip = flash->chips[1];
	ret = ramtd_read(mtd, from, len, retlen, buf);
	flash->chip = flash->chips[0];
	ra_and(SPI_REG_MASTER, ~(1 << 29));
	return ret;
}

static int ramtd_write_2(struct mtd_info *mtd, loff_t to, size_t len,
		        size_t *retlen, const u_char *buf)
{
	int ret;
	ra_or(SPI_REG_MASTER, (1 << 29));
	flash->chip = flash->chips[1];
	ret = ramtd_write(mtd, to, len, retlen, buf);
	flash->chip = flash->chips[0];
	ra_and(SPI_REG_MASTER, ~(1 << 29));
	return ret;
}
#endif



/*
 * board specific setup should have ensured the SPI clock used here
 * matches what the READ command supports, at least until this driver
 * understands FAST_READ (for clocks over 25 MHz).
 */
static struct mtd_info *raspi_probe(struct map_info *map)
{
	struct chip_info		*chip;
	unsigned			i;
#ifdef CONFIG_ROOTFS_IN_FLASH_NO_PADDING
	loff_t offs;
#if 0  //asus
	struct __image_header {
		uint8_t unused[60];
		uint32_t ih_ksz;
	} hdr;
#else
//asus
  image_header_t hdr;
	version_t *hw = &hdr.u.tail.hw[0];
	uint32_t rfs_offset = 0;
	union {
		uint32_t rfs_offset_net_endian;
		uint8_t p[4];
	} u;
#endif //asus
#endif

#if  0
	spic_init();
#endif	
	if(ra_check_flash_type()!=BOOT_FROM_SPI) { /* SPI */
	    return 0;
	}

#if defined(CONFIG_RALINK_MT7621)
	// set default clock to hclk/5
        ra_and(SPI_REG_MASTER, ~(0xfff << 16));
        ra_or(SPI_REG_MASTER, (0x5 << 16));	//work-around 3-wire SPI issue (3 for RFB, 5 for EVB)
#elif defined(CONFIG_RALINK_MT7628)
	// set default clock to hclk/8
        ra_and(SPI_REG_MASTER, ~(0xfff << 16));
        ra_or(SPI_REG_MASTER, (0x6 << 16));
#endif

#ifdef TEST_CS1_FLASH
	ra_and(RALINK_SYSCTL_BASE + 0x60, ~(3 << 4));
        ra_or(SPI_REG_MASTER, (1 << 29));
#endif


	chip = chip_prob();
	
	flash = kzalloc(sizeof *flash, GFP_KERNEL);
	if (!flash)
		return NULL;

	flash->chip = chip;
	flash->mtd.name = "raspi";

	flash->mtd.type = MTD_NORFLASH;
	flash->mtd.writesize = 1;
	flash->mtd.flags = MTD_CAP_NORFLASH;
	flash->mtd.size = chip->sector_size * chip->n_sectors;
	flash->mtd.erasesize = chip->sector_size;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,4)
	flash->mtd._erase = ramtd_erase;
	flash->mtd._read = ramtd_read;
	flash->mtd._write = ramtd_write;
	flash->mtd._lock = ramtd_lock;
	flash->mtd._unlock = ramtd_unlock;
	sema_init(&flash->lock,1);
#else
	flash->mtd.erase = ramtd_erase;
	flash->mtd.read = ramtd_read;
	flash->mtd.write = ramtd_write;
	flash->mtd.lock = ramtd_lock;
	flash->mtd.unlock = ramtd_unlock;
	init_MUTEX(&flash->lock);
#endif

	printk("%s(%02x %04x) (%d Kbytes)\n", 
	       chip->name, chip->id, chip->jedec_id, (int)(flash->mtd.size / 1024));

	printk("mtd .name = %s, .size = 0x%.8x (%uM) "
			".erasesize = 0x%.8x (%uK) .numeraseregions = %d\n",
		flash->mtd.name,
		(unsigned int)flash->mtd.size, (unsigned int)(flash->mtd.size / (1024*1024)),
		(unsigned int)flash->mtd.erasesize, (unsigned int)(flash->mtd.erasesize / 1024),
		(int)flash->mtd.numeraseregions);

	if (flash->mtd.numeraseregions)
		for (i = 0; i < flash->mtd.numeraseregions; i++)
			printk("mtd.eraseregions[%d] = { .offset = 0x%.8x, "
				".erasesize = 0x%.8x (%uK), "
				".numblocks = %d }\n",
				i, (unsigned int)flash->mtd.eraseregions[i].offset,
				(unsigned int)flash->mtd.eraseregions[i].erasesize,
				(unsigned int)(flash->mtd.eraseregions[i].erasesize / 1024),
				(int)flash->mtd.eraseregions[i].numblocks);

//asus #if defined (CONFIG_RT2880_ROOTFS_IN_FLASH) && defined (CONFIG_ROOTFS_IN_FLASH_NO_PADDING)
#if defined (CONFIG_ROOTFS_IN_FLASH_NO_PADDING) //asus
	offs = MTD_BOOT_PART_SIZE + MTD_CONFIG_PART_SIZE + MTD_FACTORY_PART_SIZE;
	ramtd_read(NULL, offs, sizeof(hdr), (size_t *)&i, (u_char *)(&hdr));
#if 0	//sdud
	if (hdr.ih_ksz != 0) {
		rt2880_partitions[4].size = ntohl(hdr.ih_ksz);
		rt2880_partitions[5].size = IMAGE1_SIZE - (MTD_BOOT_PART_SIZE +
				MTD_CONFIG_PART_SIZE + MTD_FACTORY_PART_SIZE +
				ntohl(hdr.ih_ksz));
	}
#else  //asus
	/* Looking for rootfilesystem offset */
	u.rfs_offset_net_endian = 0;
	for (i = 0; i < (MAX_VER*2); ++i, ++hw) {
		if (hw->major != ROOTFS_OFFSET_MAGIC)
			continue;
		u.p[1] = hw->minor;
		hw++;
		u.p[2] = hw->major;
		u.p[3] = hw->minor;
		rfs_offset = ntohl(u.rfs_offset_net_endian);
	}

	if (rfs_offset != 0) {
		rt2880_partitions[4].offset = MTD_BOOT_PART_SIZE + MTD_CONFIG_PART_SIZE + MTD_FACTORY_PART_SIZE + rfs_offset;
		rt2880_partitions[4].mask_flags |= MTD_WRITEABLE;
		if (flash->mtd.size > 0x800000) {
#if defined(CONFIG_SOUND)
			rt2880_partitions[3].size = flash->mtd.size - (MTD_BOOT_PART_SIZE + MTD_CONFIG_PART_SIZE + MTD_FACTORY_PART_SIZE) - MTD_RADIO_PART_SIZE;
			rt2880_partitions[4].size = flash->mtd.size - rt2880_partitions[4].offset - MTD_RADIO_PART_SIZE;
			rt2880_partitions[5].offset = flash->mtd.size - MTD_RADIO_PART_SIZE;
#else 
			rt2880_partitions[3].size = flash->mtd.size - (MTD_BOOT_PART_SIZE + MTD_CONFIG_PART_SIZE + MTD_FACTORY_PART_SIZE) - MTD_JFFS2_PART_SIZE;
			rt2880_partitions[4].size = flash->mtd.size - rt2880_partitions[4].offset - MTD_JFFS2_PART_SIZE;
			rt2880_partitions[5].offset = flash->mtd.size - MTD_JFFS2_PART_SIZE;
#endif
		} else {
			rt2880_partitions[3].size = flash->mtd.size - (MTD_BOOT_PART_SIZE + MTD_CONFIG_PART_SIZE + MTD_FACTORY_PART_SIZE);
			rt2880_partitions[4].size = flash->mtd.size - rt2880_partitions[4].offset;
			rt2880_partitions[5].name = rt2880_partitions[6].name;
			rt2880_partitions[5].size = rt2880_partitions[6].size;
			rt2880_partitions[5].offset = rt2880_partitions[6].offset;
		}
		printk(KERN_NOTICE "partion 3: %x %x\n", (unsigned int)rt2880_partitions[3].offset, (unsigned int)rt2880_partitions[3].size);
		printk(KERN_NOTICE "partion 4: %x %x\n", (unsigned int)rt2880_partitions[4].offset, (unsigned int)rt2880_partitions[4].size);
	}
#endif
#endif //asus

#ifdef TWO_SPI_FLASH
	flash->chips[0] = chip;

	ra_and(RALINK_SYSCTL_BASE + 0x60, ~(3 << 4));
	ra_or(SPI_REG_MASTER, (1 << 29));
	chip = chip_prob();
	flash->chips[1] = chip;
	ra_and(SPI_REG_MASTER, ~(1 << 29));
	flash->mtd2.name = "raspi2";

	flash->mtd2.type = MTD_NORFLASH;
	flash->mtd2.writesize = 1;
	flash->mtd2.flags = MTD_CAP_NORFLASH;
	flash->mtd2.size = chip->sector_size * chip->n_sectors;
	flash->mtd2.erasesize = chip->sector_size;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,4)
	flash->mtd2._erase = ramtd_erase_2;
	flash->mtd2._read = ramtd_read_2;
	flash->mtd2._write = ramtd_write_2;
	flash->mtd2._lock = ramtd_lock;
	flash->mtd2._unlock = ramtd_unlock;
#else
	flash->mtd2.erase = ramtd_erase_2;
	flash->mtd2.read = ramtd_read_2;
	flash->mtd2.write = ramtd_write_2;
	flash->mtd2.lock = ramtd_lock;
	flash->mtd2.unlock = ramtd_unlock;
#endif
	printk("%s(%02x %04x) (%d Kbytes)\n",
		chip->name, chip->id, chip->jedec_id, (int)(flash->mtd2.size / 1024));

	printk("mtd .name = %s, .size = 0x%.8x (%uM) "
		".erasesize = 0x%.8x (%uK) .numeraseregions = %d\n",
				flash->mtd2.name,
				(unsigned int)flash->mtd2.size, (unsigned int)(flash->mtd2.size / (1024*1024)),
				(unsigned int)flash->mtd2.erasesize, (unsigned int)(flash->mtd2.erasesize / 1024),
				(int)flash->mtd2.numeraseregions);

	if (flash->mtd2.numeraseregions)
		for (i = 0; i < flash->mtd2.numeraseregions; i++)
			printk("mtd.eraseregions[%d] = { .offset = 0x%.8x, "
					        ".erasesize = 0x%.8x (%uK), "
						".numblocks = %d }\n",
						i, (unsigned int)flash->mtd2.eraseregions[i].offset,
						(unsigned int)flash->mtd2.eraseregions[i].erasesize,
						(unsigned int)(flash->mtd2.eraseregions[i].erasesize / 1024),
						(int)flash->mtd2.eraseregions[i].numblocks);
	ralink_mtd[0] = &flash->mtd;
	ralink_mtd[1] = &flash->mtd2;
	merged_mtd = mtd_concat_create(ralink_mtd, 2, "Ralink Merged Flash");
	add_mtd_partitions(&flash->mtd, rt2880_partitions, ARRAY_SIZE(rt2880_partitions));
	add_mtd_partitions(&flash->mtd2, rt2880_partitions_2, ARRAY_SIZE(rt2880_partitions_2));
	return merged_mtd;
#else
	//asus
printk("#add mtd partition#\n");
	if (flash->mtd.size > 0x800000)
		add_mtd_partitions(&flash->mtd, rt2880_partitions, ARRAY_SIZE(rt2880_partitions));
	else
		add_mtd_partitions(&flash->mtd, rt2880_partitions, ARRAY_SIZE(rt2880_partitions)-1);

	//add_mtd_partitions(&flash->mtd, rt2880_partitions, ARRAY_SIZE(rt2880_partitions));
	//asus

	return &flash->mtd;
#endif
}


static void raspi_destroy(struct mtd_info *mtd)
{
	int			status;

	/* Clean up MTD stuff. */
	status = del_mtd_partitions(&flash->mtd);
	if (status == 0) {
		kfree(flash);
	}
}

static struct mtd_chip_driver raspi_chipdrv = {
        .probe   = raspi_probe,
        .destroy = raspi_destroy,
        .name    = "raspi_probe",
        .module  = THIS_MODULE
};

static int __init raspi_init(void)
{
#if defined (CONFIG_RT6855A_FPGA)
	// only for FPGA: restore to registers since PCI initiation changes them.
	ra_outl(0xbfbc0028, 0x68880);
	ra_outl(0xbfbc0004, 0xe9);
	ra_outl(0xbfbc0008, 0xffffffff);
	ra_outl(0xbfbc0000, 0x160001);
#endif

	register_mtd_chip_driver(&raspi_chipdrv);
	
        raspi_chipdrv.probe(NULL);

	return 0;
}

static void __exit raspi_exit(void)
{
    unregister_mtd_chip_driver(&raspi_chipdrv);
}


module_init(raspi_init);
module_exit(raspi_exit);

EXPORT_SYMBOL(bbu_mb_spic_trans);
EXPORT_SYMBOL(bbu_spic_trans);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Steven Liu");
MODULE_DESCRIPTION("MTD SPI driver for Ralink flash chips");
EXPORT_SYMBOL(FLASH_LOCK_GET);
