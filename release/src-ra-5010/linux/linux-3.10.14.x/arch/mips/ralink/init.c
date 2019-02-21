/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     init setup for Ralink RT2880 solution
 *
 *  Copyright 2007 Ralink Inc. (bruce_chang@ralinktech.com.tw)
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
 * May 2007 Bruce Chang
 *
 * Initial Release
 *
 *
 *
 **************************************************************************
 */

#include <linux/init.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/serial_8250.h>
#include <linux/delay.h>
#include <asm/bootinfo.h>
#include <asm/io.h>
#include <asm/serial.h>
#include <asm/mach-ralink/prom.h>
#include <asm/mach-ralink/generic.h>
#include <asm/mach-ralink/surfboard.h>
#include <asm/mach-ralink/surfboardint.h>
#include <asm/mach-ralink/rt_mmap.h>
#include <asm/mach-ralink/serial_rt2880.h>
#if defined (CONFIG_IRQ_GIC)
#include <asm/gcmpregs.h>
#endif

#if !defined(CONFIG_RALINK_MT7620)
#define UART_BAUDRATE	CONFIG_MT7621_BAUDRATE
#else
#define UART_BAUDRATE	CONFIG_MT7620_BAUDRATE
#endif

extern unsigned long surfboard_sysclk;
extern void show_sdk_patch_info(void);
//extern unsigned long mips_machgroup;
u32 mips_cpu_feq;

/* Environment variable */
typedef struct {
	char *name;
	char *val;
} t_env_var;

int prom_argc;
int *_prom_argv, *_prom_envp;


/*
 * YAMON (32-bit PROM) pass arguments and environment as 32-bit pointer.
 * This macro take care of sign extension, if running in 64-bit mode.
 */
#define prom_envp(index) ((char *)(((int *)(int)_prom_envp)[(index)]))

int init_debug = 0;

char *prom_getenv(char *envname)
{
	/*
	 * Return a pointer to the given environment variable.
	 * In 64-bit mode: we're using 64-bit pointers, but all pointers
	 * in the PROM structures are only 32-bit, so we need some
	 * workarounds, if we are running in 64-bit mode.
	 */
	int i, index=0;
	// Dennis Lee +
	return NULL;
	// 
	i = strlen(envname);

	while (prom_envp(index)) {
		if(strncmp(envname, prom_envp(index), i) == 0) {
			return(prom_envp(index+1));
		}
		index += 2;
	}

	return NULL;
}

static inline unsigned char str2hexnum(unsigned char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	return 0; /* foo */
}

static inline void str2eaddr(unsigned char *ea, unsigned char *str)
{
	int i;

	for (i = 0; i < 6; i++) {
		unsigned char num;

		if((*str == '.') || (*str == ':'))
			str++;
		num = str2hexnum(*str++) << 4;
		num |= (str2hexnum(*str++));
		ea[i] = num;
	}
}

#if defined(CONFIG_RALINK_MT7620)
#define RALINK_SYSTEM_CONTROL_BASE	0xB0000000
#define REVID				*(unsigned int *)(RALINK_SYSTEM_CONTROL_BASE + 0x0c)
#define RALINK_CLKCFG1			*(unsigned int *)(RALINK_SYSTEM_CONTROL_BASE + 0x30)
#define RALINK_RSTCTRL			*(unsigned int *)(RALINK_SYSTEM_CONTROL_BASE + 0x34)
#define PPLL_CFG0			*(unsigned int *)(RALINK_SYSTEM_CONTROL_BASE + 0x98)
#define PPLL_CFG1			*(unsigned int *)(RALINK_SYSTEM_CONTROL_BASE + 0x9c)
#define PPLL_DRV			*(unsigned int *)(RALINK_SYSTEM_CONTROL_BASE + 0xa0)

/* PCI-E Phy read/write */
#define PCIEPHY0_CFG			(RALINK_PCI_BASE + 0x90)
#define BUSY			0x80000000
#define WAITRETRY_MAX		10
#define WRITE_MODE		(1UL<<23)
#define DATA_SHIFT		0
#define ADDR_SHIFT		8

int wait_pciephy_busy(void)
{
	unsigned long reg_value = 0x0, retry = 0;
	while(1){
		//reg_value = rareg(READMODE, PCIEPHY0_CFG, 0);
		reg_value = (*((volatile u32 *)PCIEPHY0_CFG));

		if(reg_value & BUSY)
			mdelay(100);
		else
			break;
		if(retry++ > WAITRETRY_MAX){
			printk("PCIE-PHY retry failed.\n");
			return -1;
		}
	}
	return 0;
}

unsigned long pcie_phy(char rwmode, unsigned long addr, unsigned long val)
{
	unsigned long reg_value = 0x0;

	wait_pciephy_busy();
	if(rwmode == 'w'){
		reg_value |= WRITE_MODE;
		reg_value |= (val) << DATA_SHIFT;
	}
	reg_value |= (addr) << ADDR_SHIFT;

	// apply the action
	//rareg(WRITEMODE, PCIEPHY0_CFG, reg_value);
	(*((volatile u32 *)PCIEPHY0_CFG)) = reg_value;

	mdelay(1);
	wait_pciephy_busy();

	if(rwmode == 'r'){
		//reg_value = rareg(READMODE, PCIEPHY0_CFG, 0);
		reg_value = (*((volatile u32 *)PCIEPHY0_CFG));
		//printk("[%02x]=0x%02x\n", (unsigned int)addr, (unsigned int)(reg_value & 0xff));
		return reg_value;
	}
	return 0;
}


void Pcie_BypassDLL(void)
{
	pcie_phy('w', 0x0, 0x80);
	pcie_phy('w', 0x1, 0x04);
}

static void prom_pcieinit(void)
{
        printk(" PCIE: bypass PCIe DLL.\n");
        Pcie_BypassDLL();

	printk(" PCIE: Elastic buffer control: Addr:0x68 -> 0xB4\n");
	pcie_phy('w', 0x68, 0xB4);

	RALINK_RSTCTRL = (RALINK_RSTCTRL | RALINK_PCIE0_RST);
	RALINK_CLKCFG1 = (RALINK_CLKCFG1 & ~RALINK_PCIE0_CLK_EN);
	PPLL_DRV = (PPLL_DRV & ~(1<<19));
	PPLL_DRV = (PPLL_DRV | 1<<31);
	printk(" disable all power about PCIe\n");

	if(!( REVID & ((0x1UL)<<16))){
		/* Only MT7620N do this, not MT7620A */
		PPLL_CFG0 = (PPLL_CFG0 | (1UL << 31));
		PPLL_CFG1 = (PPLL_CFG1 | (1UL << 26));
		printk(" PCIE: PLL power down for MT7620N\n");
	}

}
#elif defined (CONFIG_MT7621_ASIC)
#define RALINK_SYSTEM_CONTROL_BASE	0xbe000000
#define RALINK_CLKCFG1			*(unsigned int *)(RALINK_SYSTEM_CONTROL_BASE + 0x30)
#define RALINK_RSTCTRL			*(unsigned int *)(RALINK_SYSTEM_CONTROL_BASE + 0x34)
#define RALINK_PCIE0_RST_CLK		(1<<24)
#define RALINK_PCIE1_RST_CLK		(1<<25)
#define RALINK_PCIE2_RST_CLK		(1<<26)
#define DEASSERT_SYSRST_PCIE(val) 	do {	\
						if ((*(unsigned int *)(0xbe00000c)&0xFFFF) == 0x0101)	\
							RALINK_RSTCTRL &= ~val;	\
						else	\
							RALINK_RSTCTRL |= val;	\
					} while(0)
static void prom_pcieinit(void)
{
	u32 val = 0;

	/* aseert PCIe RC RST */
#if !defined (CONFIG_PCIE_PORT0)
	val |= RALINK_PCIE0_RST_CLK;
#endif
#if !defined (CONFIG_PCIE_PORT1)
	val |= RALINK_PCIE1_RST_CLK;
#endif
#if !defined (CONFIG_PCIE_PORT2)
	val |= RALINK_PCIE2_RST_CLK;
#endif
	DEASSERT_SYSRST_PCIE(val);

	/* disable PCIe clock */
	RALINK_CLKCFG1 &= ~val;

#if !defined (CONFIG_PCIE_PORT0) && !defined (CONFIG_PCIE_PORT1)
	/* set  PCIe PHY to 1.3mA for power saving */
	(*((volatile u32 *)(RALINK_PCI_BASE + 0x9000))) = 0x10;
#endif
#if !defined (CONFIG_PCIE_PORT2)
	/* set  PCIe PHY to 1.3mA for power saving */
	(*((volatile u32 *)(RALINK_PCI_BASE + 0xA000))) = 0x10;
#endif
}
#elif defined (CONFIG_RALINK_MT7628)
static void prom_pcieinit(void)
{
	u32 val;

	/* aseert PCIe RC RST */
	val = (*((volatile u32 *)(RALINK_SYSCTL_BASE + 0x34)));
	val |= (0x1<<26);
	(*((volatile u32 *)(RALINK_SYSCTL_BASE + 0x34))) = val;

	/* disable PCIe clock */
	val = (*((volatile u32 *)(RALINK_SYSCTL_BASE + 0x30)));
	val &= ~(0x1<<26);
	(*((volatile u32 *)(RALINK_SYSCTL_BASE + 0x30))) = val;

#if !defined (CONFIG_PCI)
	/* set  PCIe PHY to 1.3mA for power saving */
	(*((volatile u32 *)(RALINK_PCI_BASE + 0x9000))) = 0x10;
#endif
}
#else /* CONFIG_RALINK_MT7620 */
static void prom_pcieinit(void)
{
}
#endif /* CONFIG_RALINK_MT7620 */


static void prom_usbinit(void)
{
	u32 reg = 0;

	reg = reg;
#if defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) || \
    defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || \
    defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7628)
	reg = *(volatile u32 *)KSEG1ADDR((RALINK_SYSCTL_BASE + 0x34));
	reg = reg | RALINK_UDEV_RST | RALINK_UHST_RST;
	*(volatile u32 *)KSEG1ADDR((RALINK_SYSCTL_BASE + 0x34))= reg;

	reg = *(volatile u32 *)KSEG1ADDR((RALINK_SYSCTL_BASE + 0x30));
#if defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7628)
	reg = reg & ~(RALINK_UPHY0_CLK_EN);
#else
	reg = reg & ~(RALINK_UPHY0_CLK_EN | RALINK_UPHY1_CLK_EN);
#endif
	*(volatile u32 *)KSEG1ADDR((RALINK_SYSCTL_BASE + 0x30))= reg;

#elif defined (CONFIG_RALINK_RT3052)
	*(volatile u32 *)KSEG1ADDR((RALINK_USB_OTG_BASE + 0xE00)) = 0xF;	// power saving
#elif defined (CONFIG_RALINK_MT7621)

	/* TODO */

#endif

}

int get_ethernet_addr(char *ethernet_addr)
{
        char *ethaddr_str;

        ethaddr_str = prom_getenv("ethaddr");
	if (!ethaddr_str) {
	        printk("ethaddr not set in boot prom\n");
		return -1;
	}
	str2eaddr(ethernet_addr, ethaddr_str);

	if (init_debug > 1) {
	        int i;
		printk("get_ethernet_addr: ");
	        for (i=0; i<5; i++)
		        printk("%02x:", (unsigned char)*(ethernet_addr+i));
		printk("%02x\n", *(ethernet_addr+i));
	}

	return 0;
}

void prom_init_sysclk(void)
{
#if defined (CONFIG_RALINK_MT7621)
	int cpu_fdiv = 0;
	int cpu_ffrac = 0;
	int fbdiv = 0;
#endif

	u32 reg __maybe_unused;

#if defined(CONFIG_RT2880_FPGA)
        mips_cpu_feq = 25000000; 
#elif defined (CONFIG_RT3052_FPGA) || defined (CONFIG_RT3352_FPGA) || defined (CONFIG_RT2883_FPGA) || defined (CONFIG_RT3883_FPGA) || defined (CONFIG_RT5350_FPGA) 
        mips_cpu_feq = 40000000; 
#elif defined (CONFIG_RT6855_FPGA) || defined (CONFIG_MT7620_FPGA) || defined (CONFIG_MT7628_FPGA)
        mips_cpu_feq = 50000000; 
#elif defined (CONFIG_MT7621_FPGA)
        mips_cpu_feq = 50000000;
#else
        u8 clk_sel;

        reg = (*((volatile u32 *)(RALINK_SYSCTL_BASE + 0x10)));

#if defined (CONFIG_RT2880_ASIC)
        clk_sel = (reg>>20) & 0x03;
#elif defined (CONFIG_RT2883_ASIC) 
        clk_sel = (reg>>18) & 0x03;
#elif defined (CONFIG_RT3052_ASIC) 
        clk_sel = (reg>>18) & 0x01;
#elif defined (CONFIG_RT3352_ASIC) 
        clk_sel = (reg>>8) & 0x01;
#elif defined (CONFIG_RT5350_ASIC) 
	{
        u8 clk_sel2;
        clk_sel = (reg>>8) & 0x01;
        clk_sel2 = (reg>>10) & 0x01;
        clk_sel |= (clk_sel2 << 1 );
	}
#elif defined (CONFIG_RT3883_ASIC) 
        clk_sel = (reg>>8) & 0x03;
#elif defined (CONFIG_MT7620_ASIC) 
	reg = (*((volatile u32 *)(RALINK_SYSCTL_BASE + 0x58)));
	if( reg & ((0x1UL) << 24) ){
		clk_sel = 1;	/* clock from BBP PLL (480MHz ) */
	}else{
		clk_sel = 0;	/* clock from CPU PLL (600MHz) */


	}
#elif defined (CONFIG_MT7621_ASIC)
	clk_sel = 0;
	reg = (*((volatile u32 *)(RALINK_SYSCTL_BASE + 0x2C)));
	if( reg & ((0x1UL) << 30)) {
		clk_sel = 1; // CPU PLL
	} else {
		clk_sel = 0; // GPLL (500Mhz)
	}
#elif defined (CONFIG_RT6855_ASIC)
        clk_sel = 0;
#elif defined (CONFIG_MT7628_ASIC)
        clk_sel = 0;
#else
#error Please Choice System Type
#endif
        switch(clk_sel) {
#if defined (CONFIG_RALINK_RT2880_SHUTTLE)
	case 0:
		mips_cpu_feq = (233333333);
		break;
	case 1:
		mips_cpu_feq = (250000000);
		break;
	case 2:
		mips_cpu_feq = (266666666);
		break;
	case 3:
		mips_cpu_feq = (280000000);
		break;
#elif defined (CONFIG_RALINK_RT2880_MP)
	case 0:
		mips_cpu_feq = (250000000);
		break;
	case 1:
		mips_cpu_feq = (266666666);
		break;
	case 2:
		mips_cpu_feq = (280000000);
		break;
	case 3:
		mips_cpu_feq = (300000000);
		break;
#elif defined (CONFIG_RALINK_RT2883) 
	case 0:
		mips_cpu_feq = (380*1000*1000);
		break;
	case 1:
		mips_cpu_feq = (390*1000*1000);
		break;
	case 2:
		mips_cpu_feq = (400*1000*1000);
		break;
	case 3:
		mips_cpu_feq = (420*1000*1000);
		break;
#elif defined (CONFIG_RALINK_RT3052) 
#if defined (CONFIG_RALINK_RT3350)
		// MA10 is floating
	case 0:
	case 1:
		mips_cpu_feq = (320*1000*1000);
		break;
#else
	case 0:
		mips_cpu_feq = (320*1000*1000);
		break;
	case 1:
		mips_cpu_feq = (384*1000*1000); 
		break;
#endif
#elif defined (CONFIG_RALINK_RT3352) 
	case 0:
		mips_cpu_feq = (384*1000*1000);
		break;
	case 1:
		mips_cpu_feq = (400*1000*1000); 
		break;
#elif defined (CONFIG_RALINK_RT3883) 
	case 0:
		mips_cpu_feq = (250*1000*1000);
		break;
	case 1:
		mips_cpu_feq = (384*1000*1000); 
		break;
	case 2:
		mips_cpu_feq = (480*1000*1000); 
		break;
	case 3:
		mips_cpu_feq = (500*1000*1000); 
		break;
#elif defined(CONFIG_RALINK_RT5350)
	case 0:
		mips_cpu_feq = (360*1000*1000);
		break;
	case 1:
		//reserved
		break;
	case 2:
		mips_cpu_feq = (320*1000*1000); 
		break;
	case 3:
		mips_cpu_feq = (300*1000*1000); 
		break;
#elif defined (CONFIG_RALINK_RT6855) 
	case 0:
		mips_cpu_feq = (400*1000*100);
		break;
#elif defined (CONFIG_RALINK_MT7620)
	case 0:
		reg = (*((volatile u32 *)(RALINK_SYSCTL_BASE + 0x54)));
		if(!(reg & CPLL_SW_CONFIG)){
			mips_cpu_feq = (600*1000*1000);
		}else{
			/* read CPLL_CFG0 to determine real CPU clock */
			int mult_ratio = (reg & CPLL_MULT_RATIO) >> CPLL_MULT_RATIO_SHIFT;
			int div_ratio = (reg & CPLL_DIV_RATIO) >> CPLL_DIV_RATIO_SHIFT;
			mult_ratio += 24;	/* begin from 24 */
			if(div_ratio == 0)	/* define from datasheet */
				div_ratio = 2;
			else if(div_ratio == 1)
				div_ratio = 3;
			else if(div_ratio == 2)
				div_ratio = 4;
			else if(div_ratio == 3)
				div_ratio = 8;
			mips_cpu_feq = ((BASE_CLOCK * mult_ratio ) / div_ratio) * 1000 * 1000;
		}

		break;
	case 1:
		mips_cpu_feq = (480*1000*1000);
		break;
#elif defined (CONFIG_RALINK_MT7621)
        case 0:
		reg = (*(volatile u32 *)(RALINK_SYSCTL_BASE + 0x44));
		cpu_fdiv = ((reg >> 8) & 0x1F);
		cpu_ffrac = (reg & 0x1F);
                mips_cpu_feq = (500 * cpu_ffrac / cpu_fdiv) * 1000 * 1000;
                break;
        case 1: //CPU PLL
		reg = (*(volatile u32 *)(RALINK_MEMCTRL_BASE + 0x648));
		fbdiv = ((reg >> 4) & 0x7F) + 1;
		reg = (*(volatile u32 *)(RALINK_SYSCTL_BASE + 0x10)); 
		reg = (reg >> 6) & 0x7;
		if(reg >= 6) { //25Mhz Xtal
			mips_cpu_feq = 25 * fbdiv * 1000 * 1000;
		} else if(reg >=3) { //40Mhz Xtal
			mips_cpu_feq = 20 * fbdiv * 1000 * 1000;
		} else { // 20Mhz Xtal
			/* TODO */
		}
		break;
#elif defined (CONFIG_RALINK_MT7628)
	case 0:
		reg = (*(volatile u32 *)(RALINK_SYSCTL_BASE + 0x10));
		if (reg & 0x80)
		{
			/* 40MHz Xtal */
			mips_cpu_feq = 580 * 1000 * 1000;
		} else
		{
			/* 25MHZ Xtal */
			mips_cpu_feq = 575 * 1000 * 1000;
		}
		break;
#else
#error Please Choice Chip Type
#endif
	}

#endif
	
#if defined (CONFIG_RT3883_ASIC) 
	if ((reg>>17) & 0x1) { //DDR2
		switch (clk_sel) {
		case 0:
			surfboard_sysclk = (125*1000*1000);
			break;
		case 1:
			surfboard_sysclk = (128*1000*1000);
			break;
		case 2:
			surfboard_sysclk = (160*1000*1000);
			break;
		case 3:
			surfboard_sysclk = (166*1000*1000);
			break;
		}
	}
	else { //SDR
		switch (clk_sel) {
		case 0:
			surfboard_sysclk = (83*1000*1000);
			break;
		case 1:
			surfboard_sysclk = (96*1000*1000);
			break;
		case 2:
			surfboard_sysclk = (120*1000*1000);
			break;
		case 3:
			surfboard_sysclk = (125*1000*1000);
			break;
		}
	}

#elif defined(CONFIG_RT5350_ASIC)
	switch (clk_sel) {
	case 0:
		surfboard_sysclk = (120*1000*1000);
		break;
	case 1:
		//reserved
		break;
	case 2:
		surfboard_sysclk = (80*1000*1000);
		break;
	case 3:
		surfboard_sysclk = (100*1000*1000);
		break;
	}

#elif defined (CONFIG_RALINK_RT6855)
	surfboard_sysclk = mips_cpu_feq/4;
#elif defined (CONFIG_RALINK_MT7620)
	/* FIXME , SDR -> /4,   DDR -> /3, but currently "surfboard_sysclk" */
	surfboard_sysclk = mips_cpu_feq/4;
#elif defined (CONFIG_RALINK_MT7628)
	surfboard_sysclk = mips_cpu_feq/3;
#elif defined (CONFIG_RALINK_MT7621)
	surfboard_sysclk = mips_cpu_feq/4;
#elif defined (CONFIG_RALINK_RT2880)
	surfboard_sysclk = mips_cpu_feq/2;
#else
	surfboard_sysclk = mips_cpu_feq/3;
#endif
	printk("\n The CPU feqenuce set to %u MHz\n",mips_cpu_feq / 1000 / 1000);


#ifdef CONFIG_RALINK_CPUSLEEP
	printk("\n MIPS CPU sleep mode enabled.\n");
#if defined (CONFIG_RALINK_MT7620) 
#ifdef CONFIG_USB_SUPPORT
	reg = (*((volatile u32 *)(RALINK_SYSCTL_BASE + 0x3C)));
	reg &= ~(0x1F1F);
	reg |= 0x303;
	(*((volatile u32 *)(RALINK_SYSCTL_BASE + 0x3C))) = reg;
#endif /* CONFIG_USB_SUPPORT */
	reg = (*((volatile u32 *)(RALINK_SYSCTL_BASE + 0x40)));
	(*((volatile u32 *)(RALINK_SYSCTL_BASE + 0x40))) = (reg | 0x80000000);
#elif defined (CONFIG_RALINK_MT7621)
	reg = (*((volatile u32 *)(RALINK_RBUS_MATRIXCTL_BASE + 0x14)));
	(*((volatile u32 *)(RALINK_RBUS_MATRIXCTL_BASE + 0x14))) = (reg | 0xC0000000);
#elif defined (CONFIG_RALINK_MT7628)
#ifdef CONFIG_USB_SUPPORT
	reg = (*((volatile u32 *)(RALINK_SYSCTL_BASE + 0x440)));
	reg &= ~(0xf0f);
	reg |= 0x606;
	(*((volatile u32 *)(RALINK_SYSCTL_BASE + 0x440))) = reg;
#else
	reg = (*((volatile u32 *)(RALINK_SYSCTL_BASE + 0x440)));
	reg &= ~(0xf0f);
	reg |= 0xa0a;
	(*((volatile u32 *)(RALINK_SYSCTL_BASE + 0x440))) = reg;
#endif /* CONFIG_USB_SUPPORT */
	reg = (*((volatile u32 *)(RALINK_SYSCTL_BASE + 0x444)));
	(*((volatile u32 *)(RALINK_SYSCTL_BASE + 0x444))) = (reg | 0x80000000);
#endif /* MT7620 */
#endif /* CONFIG_RALINK_CPUSLEEP */

#if defined (CONFIG_RALINK_MT7628)
	reg = (*((volatile u32 *)(RALINK_RBUS_MATRIXCTL_BASE + 0x0)));
	(*((volatile u32 *)(RALINK_RBUS_MATRIXCTL_BASE + 0x0))) = (reg & ~(0x4000000));

	// MIPS reset apply to Andes
	reg = (*((volatile u32 *)(RALINK_SYSCTL_BASE + 0x38)));
	(*((volatile u32 *)(RALINK_SYSCTL_BASE + 0x38))) = (reg | 0x200);
#endif

}

/*
** This function sets up the local prom_rs_table used only for the fake console
** console (mainly prom_printf for debug display and no input processing)
** and also sets up the global rs_table used for the actual serial console.
** To get the correct baud_base value, prom_init_sysclk() must be called before
** this function is called.
*/
static struct uart_port serial_req[2];
__init int prom_init_serial_port(void)
{

  /*
   * baud rate = system clock freq / (CLKDIV * 16)
   * CLKDIV=system clock freq/16/baud rate
   */
  memset(serial_req, 0, 2*sizeof(struct uart_port));

  serial_req[0].type       = PORT_16550A;
  serial_req[0].line       = 0;
  serial_req[0].irq        = SURFBOARDINT_UART;
  serial_req[0].flags      = UPF_FIXED_TYPE;
#if defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) ||  defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7628)
  serial_req[0].uartclk    = 40000000;
#elif defined (CONFIG_RALINK_MT7621)
  serial_req[0].uartclk    = 50000000;
#else
  serial_req[0].uartclk    = surfboard_sysclk;
#endif

#if defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
  serial_req[0].iotype     = UPIO_MEM32;
#else
  serial_req[0].iotype     = UPIO_AU;
#endif
  serial_req[0].regshift   = 2;
  serial_req[0].mapbase    = RALINK_UART_BASE;
  serial_req[0].membase    = ioremap_nocache(RALINK_UART_BASE, PAGE_SIZE);

  serial_req[1].type       = PORT_16550A;
  serial_req[1].line       = 1;
  serial_req[1].irq        = SURFBOARDINT_UART1;
  serial_req[1].flags      = UPF_FIXED_TYPE;
#if defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) ||  defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7628)
  serial_req[1].uartclk    = 40000000;
#elif defined (CONFIG_RALINK_MT7621)
  serial_req[1].uartclk    = 50000000;
#else
  serial_req[1].uartclk    = surfboard_sysclk;
#endif

#if defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
  serial_req[1].iotype     = UPIO_MEM32;
#else
  serial_req[1].iotype     = UPIO_AU;
#endif
  serial_req[1].regshift   = 2;
  serial_req[1].mapbase    = RALINK_UART_LITE_BASE;
  serial_req[1].membase    = ioremap_nocache(RALINK_UART_LITE_BASE, PAGE_SIZE);

  early_serial_setup(&serial_req[0]);
  early_serial_setup(&serial_req[1]);

  return(0);
}


__init int prom_get_ttysnum(void)
{
	char *argptr;
	int ttys_num = 0;       /* default */

	/* get ttys_num to use with the fake console/prom_printf */
	argptr = prom_getcmdline();

	if ((argptr = strstr(argptr, "console=ttyS")) != NULL)
	{
                argptr += strlen("console=ttyS");

                if (argptr[0] == '0')           /* ttyS0 */
                        ttys_num = 0;           /* happens to be rs_table[0] */
                else if (argptr[0] == '1')      /* ttyS1 */
                        ttys_num = 1;           /* happens to be rs_table[1] */
	}

	return (ttys_num);
}

static void serial_setbrg(unsigned long wBaud)
{
	unsigned int clock_divisor = 0;
#if defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) || \
    defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || \
    defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7628)
        clock_divisor =  (40000000 / SURFBOARD_BAUD_DIV / wBaud);
#elif defined (CONFIG_RALINK_MT7621)
        clock_divisor =  (50000000 / SURFBOARD_BAUD_DIV / wBaud);
#else
        clock_divisor =  (surfboard_sysclk / SURFBOARD_BAUD_DIV / wBaud);
#endif

        //fix at 57600 8 n 1 n
        IER(RALINK_SYSCTL_BASE + 0xC00) = 0;
        FCR(RALINK_SYSCTL_BASE + 0xC00) = 0;
        LCR(RALINK_SYSCTL_BASE + 0xC00) = (UART_LCR_WLEN8 | UART_LCR_DLAB);
        DLL(RALINK_SYSCTL_BASE + 0xC00) = clock_divisor & 0xff;
        DLM(RALINK_SYSCTL_BASE + 0xC00) = clock_divisor >> 8;
        LCR(RALINK_SYSCTL_BASE + 0xC00) = UART_LCR_WLEN8;

#if defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
        IER(RALINK_SYSCTL_BASE + 0xD00) = 0;
        FCR(RALINK_SYSCTL_BASE + 0xD00) = 0;
        LCR(RALINK_SYSCTL_BASE + 0xD00) = (UART_LCR_WLEN8 | UART_LCR_DLAB);
        DLL(RALINK_SYSCTL_BASE + 0xD00) = clock_divisor & 0xff;
        DLM(RALINK_SYSCTL_BASE + 0xD00) = clock_divisor >> 8;
        LCR(RALINK_SYSCTL_BASE + 0xD00) = UART_LCR_WLEN8;
#else
	IER(RALINK_SYSCTL_BASE + 0x500) = 0;
        FCR(RALINK_SYSCTL_BASE + 0x500) = 0;
        LCR(RALINK_SYSCTL_BASE + 0x500) = (UART_LCR_WLEN8 | UART_LCR_DLAB);
        DLL(RALINK_SYSCTL_BASE + 0x500) = clock_divisor & 0xff;
        DLM(RALINK_SYSCTL_BASE + 0x500) = clock_divisor >> 8;
        LCR(RALINK_SYSCTL_BASE + 0x500) = UART_LCR_WLEN8;
#endif
}

int serial_init(unsigned long wBaud)
{
        serial_setbrg(wBaud);

        return (0);
}
__init void prom_init(void)
{
	//mips_machgroup = MACH_GROUP_RT2880;
	//mips_machtype = MACH_RALINK_ROUTER;
#if defined (CONFIG_IRQ_GIC)
	int result __maybe_unused;
#endif

#ifdef CONFIG_UBOOT_CMDLINE
	prom_argc = (int)fw_arg0;
	_prom_argv = (int *)fw_arg1;
	_prom_envp = (int *)fw_arg2;
#endif

	prom_init_cmdline();

	prom_init_sysclk();

	set_io_port_base(KSEG1);
	write_c0_wired(0);
	serial_init(UART_BAUDRATE);

	prom_init_serial_port();  /* Needed for Serial Console */

	prom_meminit();
	prom_usbinit();		/* USB power saving*/
	prom_pcieinit();	/* PCIe power saving*/
	prom_setup_printf(prom_get_ttysnum());
	prom_printf("\nLINUX started...\n");
#if defined(CONFIG_RT2880_FPGA) || defined(CONFIG_RT3052_FPGA) || defined(CONFIG_RT3352_FPGA) || defined(CONFIG_RT2883_FPGA) || defined(CONFIG_RT3883_FPGA) || defined(CONFIG_RT5350_FPGA) || defined (CONFIG_RT6855_FPGA) || defined(CONFIG_MT7620_FPGA) || defined (CONFIG_MT7621_FPGA) || defined (CONFIG_MT7628_FPGA)
	prom_printf("\n THIS IS FPGA\n");
#elif defined(CONFIG_RT2880_ASIC) || defined(CONFIG_RT3052_ASIC) || defined(CONFIG_RT3352_ASIC) || defined (CONFIG_RT2883_ASIC) || defined (CONFIG_RT3883_ASIC) || defined (CONFIG_RT5350_ASIC) || defined (CONFIG_RT6855_ASIC) || defined (CONFIG_MT7620_ASIC) || defined (CONFIG_MT7621_ASIC) || defined (CONFIG_MT7628_ASIC)
	prom_printf("\n THIS IS ASIC\n");
#endif
	show_sdk_patch_info();

#if defined (CONFIG_IRQ_GIC)


#if 0 /* for 2.6.36.x */
 	/* Early detection of CMP support */
        result = gcmp_probe(GCMP_BASE_ADDR, GCMP_ADDRSPACE_SZ);
#ifdef CONFIG_MIPS_CMP
        if (result)
                register_smp_ops(&cmp_smp_ops);
#endif // CONFIG_MIPS_CMP //
#ifdef CONFIG_MIPS_MT_SMP
#ifdef CONFIG_MIPS_CMP
        if (!result)
                register_smp_ops(&vsmp_smp_ops);
#else
	register_smp_ops(&vsmp_smp_ops);
#endif // CONFIG_MIPS_CMP //
#endif // CONFIG_MIPS_MT_SMP //
#endif /* if 0, for 2.6.36.x */

	/* Early detection of CMP support */
	if (gcmp_probe(GCMP_BASE_ADDR, GCMP_ADDRSPACE_SZ)){
#ifdef CONFIG_MIPS_CMP
		if (!register_cmp_smp_ops())
			return;
#endif
        }                                                        

#ifdef CONFIG_MIPS_MT_SMP
	if (!register_vsmp_smp_ops())
		return;
#endif

#endif // CONFIG_IRQ_GIC //
}

EXPORT_SYMBOL(mips_cpu_feq);
