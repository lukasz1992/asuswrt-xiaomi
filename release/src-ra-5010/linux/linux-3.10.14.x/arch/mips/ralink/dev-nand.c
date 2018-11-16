#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/i2c/at24.h>
#include <linux/i2c.h>
#include <asm/mach-ralink/rt_mmap.h>
#include <asm/mach-ralink/surfboardint.h>
#include <linux/mtd/mt6575_typedefs.h>

#if defined (CONFIG_MTK_MTD_NAND) || defined (CONFIG_MTD_ANY_RALINK) //RLT_TODO
#define NFI_base    RALINK_NAND_CTRL_BASE
#define NFIECC_base RALINK_NANDECC_CTRL_BASE
#define MT7621_NFI_IRQ_ID		SURFBOARDINT_NAND
#define MT7621_NFIECC_IRQ_ID	SURFBOARDINT_NAND_ECC

static struct resource MT7621_resource_nand[] = {
        {
                .start          = NFI_base,
                .end            = NFI_base + 0x1A0,
                .flags          = IORESOURCE_MEM,
        },
        {
                .start          = NFIECC_base,
                .end            = NFIECC_base + 0x150,
                .flags          = IORESOURCE_MEM,
        },
        {
                .start          = MT7621_NFI_IRQ_ID,
                .flags          = IORESOURCE_IRQ,
        },
        {
                .start          = MT7621_NFIECC_IRQ_ID,
                .flags          = IORESOURCE_IRQ,
        },
};

static struct platform_device MT7621_nand_dev = {
    .name = "MT7621-NAND",
    .id   = 0,
        .num_resources  = ARRAY_SIZE(MT7621_resource_nand),
        .resource               = MT7621_resource_nand,
    .dev            = {
        .platform_data = &mt7621_nand_hw,
    },
};
#endif


int __init mtk_nand_register(void)
{

	int retval = 0;

	retval = platform_device_register(&MT7621_nand_dev);
	if (retval != 0) {
		printk(KERN_ERR "register nand device fail\n");
		return retval;
	}


	return retval;
}
arch_initcall(mtk_nand_register);
