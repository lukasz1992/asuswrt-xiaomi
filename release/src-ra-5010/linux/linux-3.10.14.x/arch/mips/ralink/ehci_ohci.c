/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     EHCI/OHCI init for MTK/Ralink APSoC platform
 *
 *  Copyright 2009 Ralink Inc. (yyhuang@ralinktech.com.tw)
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 **************************************************************************
 * March 2009 YYHuang Initial Release
 * Dec   2013 using linux 3.10.14 generic ehci/ohci platform code - yyhuang
 **************************************************************************
 */

#include <linux/types.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/usb/ehci_pdriver.h>
#include <linux/usb/ohci_pdriver.h>

#include <asm/mach-ralink/rt_mmap.h>
#include <linux/dma-mapping.h>

#if defined(CONFIG_USB_EHCI_HCD_PLATFORM) || defined(CONFIG_USB_OHCI_HCD_PLATFORM) || \
    defined(CONFIG_USB_EHCI_HCD_PLATFORM_MODULE) || defined(CONFIG_USB_OHCI_HCD_PLATFORM_MODULE)

#define SYSCFG1                 (RALINK_SYSCTL_BASE + 0x14)
#define USB0_HOST_MODE          (1UL<<10)

#define RT2880_CLKCFG1_REG      (RALINK_SYSCTL_BASE + 0x30)
#define RT2880_RSTCTRL_REG      (RALINK_SYSCTL_BASE + 0x34)
#define RALINK_UHST_RST         (1<<22)
#define RALINK_UDEV_RST         (1<<25)

#define IRQ_RT3XXX_USB 18

static struct resource rt3xxx_ehci_resources[] = {
	[0] = {
		.start  = 0x101c0000,
		.end    = 0x101c0fff,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = IRQ_RT3XXX_USB,
		.end    = IRQ_RT3XXX_USB,
		.flags  = IORESOURCE_IRQ,
	},
};

static struct resource rt3xxx_ohci_resources[] = {
	[0] = {
		.start  = 0x101c1000,
		.end    = 0x101c1fff,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = IRQ_RT3XXX_USB,
		.end    = IRQ_RT3XXX_USB,
		.flags  = IORESOURCE_IRQ,
	},
};

static u64 rt3xxx_ehci_dmamask = DMA_BIT_MASK(32);
static u64 rt3xxx_ohci_dmamask = DMA_BIT_MASK(32);
static atomic_t power_saving_bits = ATOMIC_INIT(0);

void static inline rt_writel(u32 val, unsigned long reg)
{
	*(volatile u32 *)(reg) = val;
}

static inline u32 rt_readl(unsigned long reg)
{
	return (*(volatile u32 *)reg);
}

static void rt_set_host(void)
{
	u32 val = rt_readl(SYSCFG1);
	// host mode
	val |= USB0_HOST_MODE;
	rt_writel(val, SYSCFG1);
}

static void try_wake_up(void)
{
        u32 val;

        val = le32_to_cpu(*(volatile u_long *)(RT2880_CLKCFG1_REG));
#if defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7628)
	val = val | (RALINK_UPHY0_CLK_EN | RALINK_UPHY1_CLK_EN) ;
#elif defined (CONFIG_RALINK_RT5350)
	/* one port only */
	val = val | (RALINK_UPHY0_CLK_EN) ;
#else
#error  "no define platform"
#endif
	*(volatile u_long *)(RT2880_CLKCFG1_REG) = cpu_to_le32(val);
	udelay(10000);  // enable port0 & port1 Phy clock

	val = le32_to_cpu(*(volatile u_long *)(RT2880_RSTCTRL_REG));
	val = val & ~(RALINK_UHST_RST | RALINK_UDEV_RST);
	*(volatile u_long *)(RT2880_RSTCTRL_REG) = cpu_to_le32(val);
	udelay(10000);  // toggle reset bit 25 & 22 to 0
}

inline static void try_sleep(void)
{
	u32 val;

	val = le32_to_cpu(*(volatile u_long *)(RT2880_CLKCFG1_REG));
#if defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7628)
	val = val & ~(RALINK_UPHY0_CLK_EN | RALINK_UPHY1_CLK_EN);
#elif defined (CONFIG_RALINK_RT5350)
	val = val & ~(RALINK_UPHY0_CLK_EN);
#else
#error  "no define platform"
#endif
	*(volatile u_long *)(RT2880_CLKCFG1_REG) = cpu_to_le32(val);
	udelay(10000);  // disable port0 & port1 Phy clock

	val = le32_to_cpu(*(volatile u_long *)(RT2880_RSTCTRL_REG));
	val = val | (RALINK_UHST_RST | RALINK_UDEV_RST);
	*(volatile u_long *)(RT2880_RSTCTRL_REG) = cpu_to_le32(val);
	udelay(10000);  // toggle reset bit 25 & 22 to 1
}

static int ra_power_on(struct platform_device *pdev)
{
	/* printk("ra_power_on\n"); */
	atomic_inc(&power_saving_bits);
	try_wake_up();
	rt_set_host();
	return 0;
}

static void ra_power_off(struct platform_device *pdev)
{
	/* printk("ra_power_of\n"); */
	if(atomic_dec_and_test(&power_saving_bits)){
		/* printk("enter sleep mode!!!\n"); */
		try_sleep();
	}
	return;
}

static struct usb_ehci_pdata ralink_ehci_pdata = {
	.caps_offset		= 0,
	.has_synopsys_hc_bug    = 1,
	.power_on		= ra_power_on,
	.power_off		= ra_power_off,
};

static struct usb_ohci_pdata ralink_ohci_pdata = {
	.power_on		= ra_power_on,
	.power_off		= ra_power_off,
};


int __init init_rt_ehci_ohci(void)
{
	struct platform_device *ehci_pdev, *ohci_pdev;
	printk("MTK/Ralink EHCI/OHCI init.\n");
	//platform_add_devices(rt3xxx_devices, ARRAY_SIZE(rt3xxx_devices));

	ehci_pdev = platform_device_register_resndata(NULL, "ehci-platform", -1,
			&rt3xxx_ehci_resources[0], ARRAY_SIZE(rt3xxx_ehci_resources),
			&ralink_ehci_pdata, sizeof(ralink_ehci_pdata));
	if (IS_ERR(ehci_pdev)) {
		pr_err("MTK/Ralink EHCI: unable to register USB, err=%d\n", (int) PTR_ERR(ehci_pdev));
		return -1;
	}
	ehci_pdev->dev.dma_mask = &rt3xxx_ehci_dmamask;
	ehci_pdev->dev.coherent_dma_mask = DMA_BIT_MASK(32);

	ohci_pdev = platform_device_register_resndata(NULL, "ohci-platform", -1,
			&rt3xxx_ohci_resources[0], ARRAY_SIZE(rt3xxx_ohci_resources),
			&ralink_ohci_pdata, sizeof(ralink_ohci_pdata));			
	if (IS_ERR(ohci_pdev)) {
		pr_err("MTK/Ralink OHCI: unable to register USB, err=%d\n", (int) PTR_ERR(ohci_pdev));
		goto ohci_fail;
	}
	ohci_pdev->dev.dma_mask = &rt3xxx_ohci_dmamask;
	ohci_pdev->dev.coherent_dma_mask = DMA_BIT_MASK(32);
	return 0;

ohci_fail:
	platform_device_unregister(ehci_pdev);
	return -1;
}

device_initcall(init_rt_ehci_ohci);
#endif
