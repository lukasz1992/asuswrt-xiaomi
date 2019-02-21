/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     PCI init for Ralink RT2880 solution
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
 * Initial Release
 *
 * May 2009 Bruce Chang
 * support RT2880/RT3883 PCIe
 *
 * May 2011 Bruce Chang
 * support RT6855/MT7620 PCIe
 *
 **************************************************************************
 */

#include <linux/types.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <asm/pci.h>
#include <asm/io.h>
#include <asm/mach-ralink/eureka_ep430.h>
#include <linux/init.h>
#include <linux/mod_devicetable.h>
#include <linux/delay.h>
#include <asm/rt2880/surfboardint.h>

extern void pcie_phy_init(void);
extern void chk_phy_pll(void);

#ifdef CONFIG_PCI

/*
 * These functions and structures provide the BIOS scan and mapping of the PCI
 * devices.
 */

#if defined(CONFIG_RALINK_RT2883) || defined(CONFIG_RALINK_RT3883) || \
    defined(CONFIG_RALINK_RT6855) || defined(CONFIG_RALINK_MT7620) || \
    defined(CONFIG_RALINK_MT7628)
#define RALINK_PCI_MM_MAP_BASE	0x20000000
#define RALINK_PCI_IO_MAP_BASE	0x10160000
#elif defined(CONFIG_RALINK_MT7621)
#define RALINK_PCI_MM_MAP_BASE	0x60000000
#define RALINK_PCI_IO_MAP_BASE	0x1e160000
#else
#define RALINK_PCI_MM_MAP_BASE	0x20000000
#define RALINK_PCI_IO_MAP_BASE	0x00460000
#endif

#if defined(CONFIG_RALINK_MT7621)
#define RALINK_SYSTEM_CONTROL_BASE	0xbe000000
#define	PCIE_SHARE_PIN_SW	10

#define GPIO_PERST
#if defined GPIO_PERST
#define GPIO_PCIE_PORT0		19
#if defined CONFIG_RALINK_I2S || defined CONFIG_RALINK_I2S_MODULE
#define	UARTL3_SHARE_PIN_SW	PCIE_SHARE_PIN_SW
#define GPIO_PCIE_PORT1		GPIO_PCIE_PORT0
#define GPIO_PCIE_PORT2		GPIO_PCIE_PORT0
#else
#define	UARTL3_SHARE_PIN_SW	 3
#define GPIO_PCIE_PORT1		 8
#define GPIO_PCIE_PORT2		 7
#endif
#define RALINK_GPIO_CTRL0			*(unsigned int *)(RALINK_PIO_BASE + 0x00)
#define RALINK_GPIO_DATA0			*(unsigned int *)(RALINK_PIO_BASE + 0x20)
#endif

#define ASSERT_SYSRST_PCIE(val)		do {	\
						if ((*(unsigned int *)(0xbe00000c)&0xFFFF) == 0x0101)	\
							RALINK_RSTCTRL |= val;	\
						else	\
							RALINK_RSTCTRL &= ~val;	\
					} while(0)
#define DEASSERT_SYSRST_PCIE(val) 	do {	\
						if ((*(unsigned int *)(0xbe00000c)&0xFFFF) == 0x0101)	\
							RALINK_RSTCTRL &= ~val;	\
						else	\
							RALINK_RSTCTRL |= val;	\
					} while(0)
#else
#define RALINK_SYSTEM_CONTROL_BASE	0xb0000000
#endif
#define RALINK_SYSCFG1 			*(unsigned int *)(RALINK_SYSTEM_CONTROL_BASE + 0x14)
#define RALINK_CLKCFG1			*(unsigned int *)(RALINK_SYSTEM_CONTROL_BASE + 0x30)
#define RALINK_RSTCTRL			*(unsigned int *)(RALINK_SYSTEM_CONTROL_BASE + 0x34)
#define RALINK_GPIOMODE			*(unsigned int *)(RALINK_SYSTEM_CONTROL_BASE + 0x60)
#define RALINK_PCIE_CLK_GEN		*(unsigned int *)(RALINK_SYSTEM_CONTROL_BASE + 0x7c)
#define RALINK_PCIE_CLK_GEN1		*(unsigned int *)(RALINK_SYSTEM_CONTROL_BASE + 0x80)
#define PPLL_CFG1			*(unsigned int *)(RALINK_SYSTEM_CONTROL_BASE + 0x9c)
#define PPLL_DRV			*(unsigned int *)(RALINK_SYSTEM_CONTROL_BASE + 0xa0)
//RALINK_SYSCFG1 bit
#define RALINK_PCI_HOST_MODE_EN		(1<<7)
#define RALINK_PCIE_RC_MODE_EN		(1<<8)
//RALINK_RSTCTRL bit
#define RALINK_PCIE_RST			(1<<23)
#define RALINK_PCI_RST			(1<<24)
//RALINK_CLKCFG1 bit
#define RALINK_PCI_CLK_EN		(1<<19)
#define RALINK_PCIE_CLK_EN		(1<<21)
//RALINK_GPIOMODE bit
#define PCI_SLOTx2			(1<<11)
#define PCI_SLOTx1			(2<<11)
//MTK PCIE PLL bit
#define PDRV_SW_SET			(1<<31)
#define LC_CKDRVPD_			(1<<19)

#if defined(CONFIG_RALINK_RT2883) || defined(CONFIG_RALINK_RT3883) || \
    defined(CONFIG_RALINK_RT6855) ||  defined(CONFIG_RALINK_MT7620) || \
    defined(CONFIG_RALINK_MT7621) || defined(CONFIG_RALINK_MT7628)
#define MEMORY_BASE 0x0
#else
#define MEMORY_BASE 0x08000000
#endif
#if defined(CONFIG_RALINK_RT6855) ||  defined(CONFIG_RALINK_MT7620) || defined(CONFIG_RALINK_MT7628)
	//pcie_disable = 0 mean there is a card on this slot
	//pcie_disable = 1 mean there is no card on this slot
	int pcie0_disable =0;
	int pcie1_disable =0;
#elif defined(CONFIG_RALINK_MT7621)
	int pcie_link_status = 0;
#endif
//extern pci_probe_only;

void __inline__ read_config(unsigned long bus, unsigned long dev, unsigned long func, unsigned long reg, unsigned long *val);
void __inline__ write_config(unsigned long bus, unsigned long dev, unsigned long func, unsigned long reg, unsigned long val);

#if 1

#define PCI_ACCESS_READ_1  0
#define PCI_ACCESS_READ_2  1
#define PCI_ACCESS_READ_4  2
#define PCI_ACCESS_WRITE_1 3
#define PCI_ACCESS_WRITE_2 4
#define PCI_ACCESS_WRITE_4 5

static int config_access(unsigned char access_type, struct pci_bus *bus,
                         unsigned int devfn, unsigned int where,
                         u32 * data)
{
  unsigned int slot = PCI_SLOT(devfn);
  u8 func = PCI_FUNC(devfn);
  uint32_t address_reg, data_reg;
  unsigned int address;

  address_reg = RALINK_PCI_CONFIG_ADDR;
  data_reg = RALINK_PCI_CONFIG_DATA_VIRTUAL_REG;
#if defined(CONFIG_RALINK_RT3883)
  if(bus->number == 0) {
	  where&=0xff;
  }
#endif

  /* Setup address */
#if defined(CONFIG_RALINK_RT2883)
  address = (bus->number << 24) | (slot << 19) | (func << 16) | (where & 0xfc)| 0x1;
#elif defined (CONFIG_RALINK_RT3883)
  address = (bus->number << 16) | (slot << 11) | (func << 8) | (where & 0xfc) | 0x80000000;
#elif defined(CONFIG_RALINK_RT6855) ||  defined(CONFIG_RALINK_MT7620) || \
      defined(CONFIG_RALINK_MT7621) || defined(CONFIG_RALINK_MT7628)
  address = (((where&0xF00)>>8)<<24) |(bus->number << 16) | (slot << 11) | (func << 8) | (where & 0xfc) | 0x80000000;
#else
  address = (bus->number << 16) | (slot << 11) | (func << 8) | (where& 0xfc) | 0x80000000;
#endif
  /* start the configuration cycle */
  MV_WRITE(address_reg, address);

  switch(access_type) {
  case PCI_ACCESS_WRITE_1:
    MV_WRITE_8(data_reg+(where&0x3), *data);
    break;
  case PCI_ACCESS_WRITE_2:
    MV_WRITE_16(data_reg+(where&0x3), *data);
    break;
  case PCI_ACCESS_WRITE_4:
    MV_WRITE(data_reg, *data);
    break;
  case PCI_ACCESS_READ_1:
    MV_READ_8( data_reg+(where&0x3), data);
    break;
  case PCI_ACCESS_READ_2:
    MV_READ_16(data_reg+(where&0x3), data);
    break;
  case PCI_ACCESS_READ_4:
    MV_READ(data_reg, data);
    break;
  default:
    printk("no specify access type\n");
    break;
  }
  //if (bus->number==1&&where==0x30){
  //printk("%x->[%x][%x][%x][%x]=%x\n",access_type,bus->number, slot, func, where, *data);
  //}
  return 0;
}



static int read_config_byte(struct pci_bus *bus, unsigned int devfn,
                            int where, u8 * val)
{
  //u32 data;
  int ret;

  ret = config_access(PCI_ACCESS_READ_1, bus, devfn, (unsigned int)where, (u32 *)val);
  //*val = (data >> ((where & 3) << 3)) & 0xff;
  return ret;
}

static int read_config_word(struct pci_bus *bus, unsigned int devfn,
                            int where, u16 * val)
{
  //u32 data;
  int ret;

  ret = config_access(PCI_ACCESS_READ_2, bus, devfn, (unsigned int)where, (u32 *)val);
  //*val = (data >> ((where & 3) << 3)) & 0xffff;
  return ret;
}

static int read_config_dword(struct pci_bus *bus, unsigned int devfn,
                             int where, u32 * val)
{
  int ret;

  ret = config_access(PCI_ACCESS_READ_4, bus, devfn, (unsigned int)where, (u32 *)val);
  return ret;
}
static int
write_config_byte(struct pci_bus *bus, unsigned int devfn, int where,
                  u8 val)
{
  if (config_access(PCI_ACCESS_WRITE_1, bus, devfn, (unsigned int)where, (u32 *)&val))
    return -1;

  return PCIBIOS_SUCCESSFUL;
}

static int
write_config_word(struct pci_bus *bus, unsigned int devfn, int where,
                  u16 val)
{
  if (config_access(PCI_ACCESS_WRITE_2, bus, devfn, where, (u32 *)&val))
    return -1;


  return PCIBIOS_SUCCESSFUL;
}

static int
write_config_dword(struct pci_bus *bus, unsigned int devfn, int where,
                   u32 val)
{
  if (config_access(PCI_ACCESS_WRITE_4, bus, devfn, where, &val))
    return -1;

  return PCIBIOS_SUCCESSFUL;
}
#else //no used
#define PCI_ACCESS_READ  0
#define PCI_ACCESS_WRITE 1

static int config_access(unsigned char access_type, struct pci_bus *bus,
                         unsigned int devfn, unsigned char where,
                         u32 * data)
{
  unsigned int slot = PCI_SLOT(devfn);
  u8 func = PCI_FUNC(devfn);
  uint32_t address_reg, data_reg;
  unsigned int address;

  address_reg = RALINK_PCI_CONFIG_ADDR;
  data_reg = RALINK_PCI_CONFIG_DATA_VIRTUAL_REG;
#if defined(CONFIG_RALINK_RT3883)
  if(bus->number == 0) {
	  where&=0xff;
  }
#endif

  /* Setup address */
#if defined (CONFIG_RALINK_RT2883)
  address = (bus->number << 24) | (slot << 19) | (func << 16) | (where & 0xfc)| 0x1;
#elif defined (CONFIG_RALINK_RT3883)
  address = (bus->number << 16) | (slot << 11) | (func << 8) | (where & 0xfc) | 0x80000000;
#elif defined(CONFIG_RALINK_RT6855) || defined(CONFIG_RALINK_MT7620) || \
      defined(CONFIG_RALINK_MT7621) || defined(CONFIG_RALINK_MT7628)
  address = (((where&0xF00)>>8)<<24) |(bus->number << 16) | (slot << 11) | (func << 8) | (where & 0xfc) | 0x80000000;
#else
  address = (bus->number << 16) | (slot << 11) | (func << 8) | (where& 0xfc) | 0x80000000;
#endif
  /* start the configuration cycle */
  MV_WRITE(address_reg, address);

  if (access_type == PCI_ACCESS_WRITE){
    MV_WRITE(data_reg, *data);
  }else{
    MV_READ(data_reg, data);
  }
  //printk("%x->[%x][%x][%x][%x]=%x\n",access_type,bus->number, slot, func, where, *data);
  return 0;
}



static int read_config_byte(struct pci_bus *bus, unsigned int devfn,
                            int where, u8 * val)
{
  u32 data;
  int ret;

  ret = config_access(PCI_ACCESS_READ, bus, devfn, where, &data);
  *val = (data >> ((where & 3) << 3)) & 0xff;
  return ret;
}

static int read_config_word(struct pci_bus *bus, unsigned int devfn,
                            int where, u16 * val)
{
  u32 data;
  int ret;

  ret = config_access(PCI_ACCESS_READ, bus, devfn, where, &data);
  *val = (data >> ((where & 3) << 3)) & 0xffff;
  return ret;
}

static int read_config_dword(struct pci_bus *bus, unsigned int devfn,
                             int where, u32 * val)
{
  int ret;

  ret = config_access(PCI_ACCESS_READ, bus, devfn, where, val);
  return ret;
}
static int
write_config_byte(struct pci_bus *bus, unsigned int devfn, int where,
                  u8 val)
{
  u32 data = 0;

  if (config_access(PCI_ACCESS_READ, bus, devfn, where, &data))
    return -1;

  data = (data & ~(0xff << ((where & 3) << 3))) |
    (val << ((where & 3) << 3));

  if (config_access(PCI_ACCESS_WRITE, bus, devfn, where, &data))
    return -1;

  return PCIBIOS_SUCCESSFUL;
}

static int
write_config_word(struct pci_bus *bus, unsigned int devfn, int where,
                  u16 val)
{
  u32 data = 0;

  if (config_access(PCI_ACCESS_READ, bus, devfn, where, &data))
    return -1;

  data = (data & ~(0xffff << ((where & 3) << 3))) |
    (val << ((where & 3) << 3));

  if (config_access(PCI_ACCESS_WRITE, bus, devfn, where, &data))
    return -1;


  return PCIBIOS_SUCCESSFUL;
}

static int
write_config_dword(struct pci_bus *bus, unsigned int devfn, int where,
                   u32 val)
{
  if (config_access(PCI_ACCESS_WRITE, bus, devfn, where, &val))
    return -1;

  return PCIBIOS_SUCCESSFUL;
}
#endif

static int pci_config_read(struct pci_bus *bus, unsigned int devfn,
                       int where, int size, u32 * val)
{
   switch (size) {
  case 1:
    return read_config_byte(bus, devfn, where, (u8 *) val);
  case 2:
    return read_config_word(bus, devfn, where, (u16 *) val);
  default:
    return read_config_dword(bus, devfn, where, val);
  }
}

static int pci_config_write(struct pci_bus *bus, unsigned int devfn,
                        int where, int size, u32 val)
{
  switch (size) {
  case 1:
    return write_config_byte(bus, devfn, where, (u8) val);
  case 2:
    return write_config_word(bus, devfn, where, (u16) val);
  default:
    return write_config_dword(bus, devfn, where, val);
  }
}


/*
 *  General-purpose PCI functions.
 */

struct pci_ops rt2880_pci_ops= {
  .read =  pci_config_read,
  .write = pci_config_write,
};

static struct resource rt2880_res_pci_mem1 = {
  .name = "PCI MEM1",
  .start = RALINK_PCI_MM_MAP_BASE,
  .end = (u32)((RALINK_PCI_MM_MAP_BASE + (unsigned char *)0x0fffffff)),
  .flags = IORESOURCE_MEM,
};
static struct resource rt2880_res_pci_io1 = {
  .name = "PCI I/O1",
  .start = RALINK_PCI_IO_MAP_BASE,
  .end = (u32)((RALINK_PCI_IO_MAP_BASE + (unsigned char *)0x0ffff)),
  .flags = IORESOURCE_IO,
};

struct pci_controller rt2880_controller = {
  .pci_ops = &rt2880_pci_ops,
  .mem_resource = &rt2880_res_pci_mem1,
  .io_resource = &rt2880_res_pci_io1,
  .mem_offset     = 0x00000000UL,
  .io_offset      = 0x00000000UL,
  .io_map_base	= 0xa0000000,
};

void __inline__ read_config(unsigned long bus, unsigned long dev, unsigned long func, unsigned long reg, unsigned long *val)
{
	unsigned int address_reg, data_reg, address;

 	address_reg = RALINK_PCI_CONFIG_ADDR;
        data_reg = RALINK_PCI_CONFIG_DATA_VIRTUAL_REG;
#if defined(CONFIG_RALINK_RT3883)
	if(bus == 0) {
		reg&=0xff;
	}
#endif

        /* set addr */
#if defined (CONFIG_RALINK_RT2883)
	        address = (bus << 24) | (dev << 19) | (func << 16) | (reg & 0xfc);
#elif defined (CONFIG_RALINK_RT3883)
  		address = (bus << 16) | (dev << 11) | (func << 8) | (reg & 0xfc) | 0x80000000 ;
#elif defined(CONFIG_RALINK_RT6855) || defined(CONFIG_RALINK_MT7620) || \
      defined(CONFIG_RALINK_MT7621) || defined(CONFIG_RALINK_MT7628)
  		address = (((reg & 0xF00)>>8)<<24) | (bus << 16) | (dev << 11) | (func << 8) | (reg & 0xfc) | 0x80000000 ;
#else
		address = (bus << 16) | (dev << 11) | (func << 8) | (reg & 0xfc) | 0x80000000 ;
#endif

        /* start the configuration cycle */
        MV_WRITE(address_reg, address);
        /* read the data */
        MV_READ(data_reg, val);
	return;
}

void __inline__ write_config(unsigned long bus, unsigned long dev, unsigned long func, unsigned long reg, unsigned long val)
{
	unsigned int address_reg, data_reg, address;

 	address_reg = RALINK_PCI_CONFIG_ADDR;
        data_reg = RALINK_PCI_CONFIG_DATA_VIRTUAL_REG;
#if defined(CONFIG_RALINK_RT3883)
	if(bus == 0) {
		reg&=0xff;
	}
#endif

        /* set addr */
#if defined (CONFIG_RALINK_RT2883)
	        address = (bus << 24) | (dev << 19) | (func << 16) | (reg & 0xfc);
#elif defined (CONFIG_RALINK_RT3883)
  		address = (bus << 16) | (dev << 11) | (func << 8) | (reg& 0xfc) | 0x80000000 ;
#elif defined(CONFIG_RALINK_RT6855) || defined(CONFIG_RALINK_MT7620) || \
      defined(CONFIG_RALINK_MT7621) || defined(CONFIG_RALINK_MT7628)
  		address = (((reg & 0xF00)>>8)<<24) | (bus << 16) | (dev << 11) | (func << 8) | (reg & 0xfc) | 0x80000000 ;
#else
		address = (bus << 16) | (dev << 11) | (func << 8) | (reg & 0xfc) | 0x80000000 ;
#endif
        /* start the configuration cycle */
        MV_WRITE(address_reg, address);
        /* read the data */
        MV_WRITE(data_reg, val);
	return;
}


#if LINUX_VERSION_CODE > KERNEL_VERSION(3,10,0)
int __init pcibios_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
#else
int __init pcibios_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
#endif
{
  u16 cmd;
  u32 val;
  struct resource *res;
  int i, irq = -1;
#ifdef CONFIG_RALINK_RT2883	
  if (dev->bus->number > 1) {
    printk("bus>1\n");
    return 0;
  }
  if (slot > 0) {
    printk("slot=%d >0\n", slot);
    return 0;
  }
#elif defined (CONFIG_RALINK_RT2880)
  if (dev->bus->number != 0) {
    return 0;
  }
#else
#endif

  //printk("** bus= %x, slot=0x%x\n",dev->bus->number,  slot);
#if defined (CONFIG_RALINK_RT3883)
  if((dev->bus->number ==0) && (slot == 0)) {
	RALINK_PCI0_BAR0SETUP_ADDR = 0x03FF0001;	//open 3FF:64M; ENABLE
	RALINK_PCI0_BAR0SETUP_ADDR = 0x03FF0001;	//open 3FF:64M; ENABLE
	RALINK_PCI1_BAR0SETUP_ADDR = 0x03FF0001;	//open 3FF:64M; ENABLE
	RALINK_PCI1_BAR0SETUP_ADDR = 0x03FF0001;	//open 3FF:64M; ENABLE
  	write_config(0, 0, 0, PCI_BASE_ADDRESS_0, MEMORY_BASE);
  	read_config(0, 0, 0, PCI_BASE_ADDRESS_0, (unsigned long *)&val);
 	printk("BAR0 at slot 0 = %x\n", val);
//  	dev->irq = 0;
 	printk("bus=0, slot = 0x%x\n", slot);
   	res = &dev->resource[0];
    	res->start = MEMORY_BASE;
    	res->end   = MEMORY_BASE + 0x01ffffff;
	for(i=0;i<16;i++){
	read_config(0, 0, 0, i<<2, (unsigned long *)&val);
	printk("P2P(PCI) 0x%02x = %08x\n", i<<2, val);
	}
  	dev->irq = 0;
  }else if((dev->bus->number ==0) && (slot == 0x1)){
	write_config(0, 1, 0, 0x1c, 0x00000101);
	for(i=0;i<16;i++){
	read_config(0, 1, 0, i<<2, (unsigned long *)&val);
	printk("P2P(PCIe)  0x%02x = %08x\n", i<<2, val);
	}
  }else if((dev->bus->number ==0) && (slot == 0x11)){
 	printk("bus=0, slot = 0x%x\n", slot);
	for(i=0;i<16;i++){
	read_config(0, 0x11, 0, i<<2, (unsigned long *)&val);
	printk("dev I(PCI)  0x%02x = %08x\n", i<<2, val);
	}
	dev->irq = 2;
  }else if((dev->bus->number ==0) && (slot == 0x12)){
 	printk("bus=0, slot = 0x%x\n", slot);
	for(i=0;i<16;i++){
	read_config(0, 0x12, 0, i<<2, (unsigned long *)&val);
	printk("dev II(PCI)  0x%02x = %08x\n", i<<2, val);
	}
	dev->irq = 15;
  }else if((dev->bus->number ==1) ){
 	printk("bus=1, slot = 0x%x\n", slot);
	for(i=0;i<16;i++){
	read_config(1, 0, 0, i<<2, (unsigned long *)&val);
	printk("dev III(PCIe)  0x%02x = %08x\n", i<<2, val);
	}
	dev->irq = 16;
  }else{
  	return 0;
  }	
#elif defined (CONFIG_RALINK_RT6855)
  if((dev->bus->number ==0) && (slot == 0)) {
	RALINK_PCI0_BAR0SETUP_ADDR = 0x7FFF0001;	//open 7FFF:2G; ENABLE
  	write_config(0, 0, 0, PCI_BASE_ADDRESS_0, MEMORY_BASE);
  	read_config(0, 0, 0, PCI_BASE_ADDRESS_0, (unsigned long *)&val);
	//write_config(0, 0, 0, 0x1c, 0x00000101);
 	printk("BAR0 at slot 0 = %x\n", val);
 	printk("bus=0x%x, slot = 0x%x\n",dev->bus->number, slot);
#if 0
   	res = &dev->resource[0];
    	res->start = MEMORY_BASE;
    	res->end   = MEMORY_BASE + 0x03ffffff;
  	//dev->irq = RALINK_INT_PCIE0;
	for(i=0;i<16;i++){
	read_config(0, 0, 0, i<<2, &val);
	printk("P2P(PCIe0) 0x%02x = %08x\n", i<<2, val);
	}
#endif
  }else if((dev->bus->number ==0) && (slot == 0x1)){
	RALINK_PCI1_BAR0SETUP_ADDR = 0x7FFF0001;	//open 7FFF:2G; ENABLE
  	write_config(0, 1, 0, PCI_BASE_ADDRESS_0, MEMORY_BASE);
  	read_config(0, 1, 0, PCI_BASE_ADDRESS_0, (unsigned long *)&val);
	//write_config(0, 1, 0, 0x1c, 0x00000101);
 	printk("BAR0 at slot 1 = %x\n", val);
 	printk("bus=0x%x, slot = 0x%x\n",dev->bus->number, slot);
#if 0
   	res = &dev->resource[0];
    	res->start = MEMORY_BASE;
    	res->end   = MEMORY_BASE + 0x03ffffff;
  	//dev->irq = RALINK_INT_PCIE1;
	for(i=0;i<16;i++){
	read_config(0, 1, 0, i<<2, &val);
	printk("P2P(PCIe1)  0x%02x = %08x\n", i<<2, val);
	}
#endif
  }else if((dev->bus->number ==1) && (slot == 0x0)){
 	printk("bus=0x%x, slot = 0x%x\n",dev->bus->number, slot);
	//dev->irq = RALINK_INT_PCIE1;
#if 1 //James want to go back
	if(pcie0_disable!=1){
		dev->irq = RALINK_INT_PCIE0;
		printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	}else{
		dev->irq = RALINK_INT_PCIE1;
		printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	}
#endif
#if 0
	for(i=0;i<16;i++){
	read_config(1, 0, 0, i<<2, &val);
	printk("dev I(PCIe0)  0x%02x = %08x\n", i<<2, val);
	}
#endif
  }else if((dev->bus->number ==1) && (slot == 0x1)){
 	printk("bus=0x%x, slot = 0x%x\n",dev->bus->number, slot);
	dev->irq = RALINK_INT_PCIE1;
  }else if((dev->bus->number ==2) && (slot == 0x0)){
 	printk("bus=0x%x, slot = 0x%x\n",dev->bus->number, slot);
	dev->irq = RALINK_INT_PCIE1;
#if 0
	for(i=0;i<16;i++){
	read_config(2, 0, 0, i<<2, &val);
	printk("dev II(PCIe1)  0x%02x = %08x\n", i<<2, val);
	}
#endif
  }else if((dev->bus->number ==2) && (slot == 0x1)){
 	printk("bus=0x%x, slot = 0x%x\n",dev->bus->number, slot);
	dev->irq = RALINK_INT_PCIE1;
  }else{
 	printk("bus=0x%x, slot = 0x%x\n",dev->bus->number, slot);
  	return 0;
  }	
#elif defined(CONFIG_RALINK_MT7620) || defined(CONFIG_RALINK_MT7628)
  if((dev->bus->number ==0) && (slot == 0)) {
  	write_config(0, 0, 0, PCI_BASE_ADDRESS_0, MEMORY_BASE);
  	read_config(0, 0, 0, PCI_BASE_ADDRESS_0, (unsigned long *)&val);
	//write_config(0, 0, 0, 0x1c, 0x00000101);
 	printk("BAR0 at slot 0 = %x\n", val);
 	printk("bus=0x%x, slot = 0x%x\n",dev->bus->number, slot);
#if 0
   	res = &dev->resource[0];
    	res->start = MEMORY_BASE;
    	res->end   = MEMORY_BASE + 0x03ffffff;
  	//dev->irq = RALINK_INT_PCIE0;
	for(i=0;i<16;i++){
	read_config(0, 0, 0, i<<2, &val);
	printk("P2P(PCIe0) 0x%02x = %08x\n", i<<2, val);
	}
#endif
  }else if((dev->bus->number ==1) && (slot == 0x0)){
 	printk("bus=0x%x, slot = 0x%x\n",dev->bus->number, slot);
	irq = RALINK_INT_PCIE0;
#if 0 //James want to go back
	if(pcie0_disable!=1){
		dev->irq = RALINK_INT_PCIE0;
		printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	}else{
		dev->irq = RALINK_INT_PCIE1;
		printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	}
#endif
#if 0
	for(i=0;i<16;i++){
	read_config(1, 0, 0, i<<2, &val);
	printk("dev I(PCIe0)  0x%02x = %08x\n", i<<2, val);
	}
#endif
  }else{
 	printk("bus=0x%x, slot = 0x%x\n",dev->bus->number, slot);
  	return 0;
  }	
#elif defined(CONFIG_RALINK_MT7621)
  if((dev->bus->number ==0) && (slot == 0)) {
	// RALINK_PCI0_BAR0SETUP_ADDR = 0x7FFF0001;	//open 7FFF:2G; ENABLE
  	write_config(0, 0, 0, PCI_BASE_ADDRESS_0, MEMORY_BASE);
  	read_config(0, 0, 0, PCI_BASE_ADDRESS_0, (unsigned long *)&val);
	//write_config(0, 0, 0, 0x1c, 0x00000101);
 	printk("BAR0 at slot 0 = %x\n", val);
 	printk("bus=0x%x, slot = 0x%x\n",dev->bus->number, slot);
#if 0
   	res = &dev->resource[0];
    	res->start = MEMORY_BASE;
    	res->end   = MEMORY_BASE + 0x03ffffff;
  	//dev->irq = RALINK_INT_PCIE0;
	for(i=0;i<16;i++){
	read_config(0, 0, 0, i<<2, &val);
	printk("P2P(PCIe0) 0x%02x = %08x\n", i<<2, val);
	}
#endif
  }else if((dev->bus->number ==0) && (slot == 0x1)){
	// RALINK_PCI1_BAR0SETUP_ADDR = 0x7FFF0001;	//open 7FFF:2G
  	write_config(0, 1, 0, PCI_BASE_ADDRESS_0, MEMORY_BASE);
  	read_config(0, 1, 0, PCI_BASE_ADDRESS_0, (unsigned long *)&val);
	//write_config(0, 1, 0, 0x1c, 0x00000101);
 	printk("BAR0 at slot 1 = %x\n", val);
 	printk("bus=0x%x, slot = 0x%x\n",dev->bus->number, slot);
#if 0
   	res = &dev->resource[0];
    	res->start = MEMORY_BASE;
    	res->end   = MEMORY_BASE + 0x03ffffff;
  	//dev->irq = RALINK_INT_PCIE1;
	for(i=0;i<16;i++){
	read_config(0, 1, 0, i<<2, &val);
	printk("P2P(PCIe1)  0x%02x = %08x\n", i<<2, val);
	}
#endif
  }else if((dev->bus->number ==0) && (slot == 0x2)){
	//RALINK_PCI2_BAR0SETUP_ADDR = 0x7FFF0001;	//open 7FFF:2G; ENABLE
  	write_config(0, 2, 0, PCI_BASE_ADDRESS_0, MEMORY_BASE);
  	read_config(0, 2, 0, PCI_BASE_ADDRESS_0, (unsigned long *)&val);
	//write_config(0, 1, 0, 0x1c, 0x00000101);
 	printk("BAR0 at slot 2 = %x\n", val);
 	printk("bus=0x%x, slot = 0x%x\n",dev->bus->number, slot);
  }else if((dev->bus->number ==1) && (slot == 0x0)){
	switch (pcie_link_status) {
	case 2:
	case 6:
		irq = RALINK_INT_PCIE1;
		break;
	case 4:
		irq = RALINK_INT_PCIE2;
		break;
	default:
		irq = RALINK_INT_PCIE0;
	}
 	printk("bus=0x%x, slot = 0x%x, irq=0x%x\n",dev->bus->number, slot, irq);
#if 0
	for(i=0;i<16;i++){
	read_config(1, 0, 0, i<<2, &val);
	printk("dev I(PCIe0)  0x%02x = %08x\n", i<<2, val);
	}
#endif
  }else if((dev->bus->number ==2) && (slot == 0x0)){
	switch (pcie_link_status) {
	case 5:
	case 6:
		irq = RALINK_INT_PCIE2;
		break;
	default:
		irq = RALINK_INT_PCIE1;
	}
 	printk("bus=0x%x, slot = 0x%x, irq=0x%x\n",dev->bus->number, slot, irq);
  }else if((dev->bus->number ==2) && (slot == 0x1)){
	switch (pcie_link_status) {
	case 5:
	case 6:
		irq = RALINK_INT_PCIE2;
		break;
	default:
		irq = RALINK_INT_PCIE1;
	}
 	printk("bus=0x%x, slot = 0x%x, irq=0x%x\n",dev->bus->number, slot, irq);
  }else if((dev->bus->number ==3) && (slot == 0x0)){
	irq = RALINK_INT_PCIE2;
	printk("bus=0x%x, slot = 0x%x, irq=0x%x\n",dev->bus->number, slot, irq);
  }else if((dev->bus->number ==3) && (slot == 0x1)){
	irq = RALINK_INT_PCIE2;
	printk("bus=0x%x, slot = 0x%x, irq=0x%x\n",dev->bus->number, slot, irq);
  }else if((dev->bus->number ==3) && (slot == 0x2)){
	irq = RALINK_INT_PCIE2;
	printk("bus=0x%x, slot = 0x%x, irq=0x%x\n",dev->bus->number, slot, irq);
  }else{
 	printk("bus=0x%x, slot = 0x%x\n",dev->bus->number, slot);
  	return 0;
  }	
#elif defined (CONFIG_RALINK_RT2883)
  if((dev->bus->number ==0) && (slot == 0)) {
	RALINK_PCI_BAR0SETUP_ADDR = 0x01FF0001;	//open 1FF:32M; ENABLE
  	write_config(0, 0, 0, PCI_BASE_ADDRESS_0, MEMORY_BASE);
  	read_config(0, 0, 0, PCI_BASE_ADDRESS_0, &val);
 	printk("BAR0 at slot 0 = %x\n", val);
//  	dev->irq = 0;
 	printk("bus=0, slot = 0x%x\n", slot);
   	res = &dev->resource[0];
    	res->start = MEMORY_BASE;
    	res->end   = MEMORY_BASE + 0x01ffffff;
	for(i=0;i<16;i++){
	read_config(0, 0, 0, i<<2, &val);
	printk("pci-to-pci 0x%02x = %08x\n", i<<2, val);
	}
  	dev->irq = 0;
  }else if((dev->bus->number ==1)){
 	printk("bus=1, slot = 0x%x\n", slot);
	for(i=0;i<16;i++){
	read_config(1, slot, 0, (i)<<2, &val);
	printk("bus 1 dev %d fun 0: 0x%02x = %08x\n", slot, i<<2, val);
	}
	dev->irq = 2;
  }else{
  	return 0;
  }	
#else //RT2880
  if(slot == 0) {
	  printk("*************************************************************\n");
	RALINK_PCI_BAR0SETUP_ADDR = 0x07FF0001;	
 	printk("MEMORY_BASE = %x\n", MEMORY_BASE);
  	write_config(0, 0, 0, PCI_BASE_ADDRESS_0, MEMORY_BASE);
  	read_config(0, 0, 0, PCI_BASE_ADDRESS_0, &val);
  	dev->irq = 0;
    res = &dev->resource[0];
    res->start = 0x08000000;
    res->end   = 0x09ffffff;
 	printk("BAR0 at slot 0 = %x\n", val);
  }else if(slot ==0x11){
	dev->irq = 2;
  }else if(slot==0x12){
	dev->irq = 15;
  }else{
  	return 0;
  }	
#endif

  for(i=0;i<6;i++){
    res = (struct resource *) &dev->resource[i];
    printk("res[%d]->start = %x\n", i, res->start);
    printk("res[%d]->end = %x\n", i, res->end);
  }

  pci_write_config_byte(dev, PCI_CACHE_LINE_SIZE, 0x14);  //configure cache line size 0x14
  pci_write_config_byte(dev, PCI_LATENCY_TIMER, 0xFF);  //configure latency timer 0x10
  pci_read_config_word(dev, PCI_COMMAND, &cmd);
//FIXME
#if defined(CONFIG_RALINK_RT2883) || defined(CONFIG_RALINK_RT3883) || \
    defined(CONFIG_RALINK_RT6855) || defined(CONFIG_RALINK_MT7620) || \
    defined(CONFIG_RALINK_MT7621) || defined(CONFIG_RALINK_MT7628)
  cmd = cmd | PCI_COMMAND_MASTER | PCI_COMMAND_IO | PCI_COMMAND_MEMORY;
#else
  cmd = cmd | PCI_COMMAND_MASTER | PCI_COMMAND_IO | PCI_COMMAND_MEMORY |
  	PCI_COMMAND_INVALIDATE | PCI_COMMAND_FAST_BACK | PCI_COMMAND_SERR |
  	PCI_COMMAND_WAIT | PCI_COMMAND_PARITY;
#endif
  pci_write_config_word(dev, PCI_COMMAND, cmd);
  pci_write_config_byte(dev, PCI_INTERRUPT_LINE, irq);
  //pci_write_config_byte(dev, PCI_INTERRUPT_PIN, dev->irq);
  return irq;
}

void set_pcie_phy(u32 *addr, int start_b, int bits, int val)
{
	//printk("0x%p:", addr);
	//printk(" %x", *addr);
	*(unsigned int *)(addr) &= ~(((1<<bits) - 1)<<start_b);
	*(unsigned int *)(addr) |= val << start_b;
	//printk(" -> %x\n", *addr);
}

#if defined (CONFIG_MT7621_ASIC)
void bypass_pipe_rst(void)
{
#if defined (CONFIG_PCIE_PORT0)
	/* PCIe Port 0 */
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x02c), 12, 1, 0x01);	// rg_pe1_pipe_rst_b
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x02c),  4, 1, 0x01);	// rg_pe1_pipe_cmd_frc[4]
#endif
#if defined (CONFIG_PCIE_PORT1)
	/* PCIe Port 1 */
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x12c), 12, 1, 0x01);	// rg_pe1_pipe_rst_b
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x12c),  4, 1, 0x01);	// rg_pe1_pipe_cmd_frc[4]
#endif
#if defined (CONFIG_PCIE_PORT2)
	/* PCIe Port 2 */
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x02c), 12, 1, 0x01);	// rg_pe1_pipe_rst_b
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x02c),  4, 1, 0x01);	// rg_pe1_pipe_cmd_frc[4]
#endif
}

void set_phy_for_ssc(void)
{
	unsigned long reg = (*(volatile u32 *)(RALINK_SYSCTL_BASE + 0x10));

	reg = (reg >> 6) & 0x7;
#if defined (CONFIG_PCIE_PORT0) || defined (CONFIG_PCIE_PORT1)
	/* Set PCIe Port0 & Port1 PHY to disable SSC */
	/* Debug Xtal Type */
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x400),  8, 1, 0x01);	// rg_pe1_frc_h_xtal_type
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x400),  9, 2, 0x00);	// rg_pe1_h_xtal_type
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x000),  4, 1, 0x01);	// rg_pe1_frc_phy_en               //Force Port 0 enable control
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x100),  4, 1, 0x01);	// rg_pe1_frc_phy_en               //Force Port 1 enable control
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x000),  5, 1, 0x00);	// rg_pe1_phy_en                   //Port 0 disable
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x100),  5, 1, 0x00);	// rg_pe1_phy_en                   //Port 1 disable
	if(reg <= 5 && reg >= 3) { 	// 40MHz Xtal
		printk("***** Xtal 40MHz *****\n");
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x490),  6, 2, 0x01);	// RG_PE1_H_PLL_PREDIV             //Pre-divider ratio (for host mode)
#if 1 /* SSC option tune from -5000ppm to -1000ppm */
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4a8),  0,12, 0x1a);	// RG_LC_DDS_SSC_DELTA
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4a8), 16,12, 0x1a);	// RG_LC_DDS_SSC_DELTA1
#endif
	} else {			// 25MHz | 20MHz Xtal
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x490),  6, 2, 0x00);	// RG_PE1_H_PLL_PREDIV             //Pre-divider ratio (for host mode)
		if (reg >= 6) { 	
			printk("***** Xtal 25MHz *****\n");
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4bc),  4, 2, 0x01);	// RG_PE1_H_PLL_FBKSEL             //Feedback clock select
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x49c),  0,31, 0x18000000);	// RG_PE1_H_LCDDS_PCW_NCPO         //DDS NCPO PCW (for host mode)
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4a4),  0,16, 0x18d);	// RG_PE1_H_LCDDS_SSC_PRD          //DDS SSC dither period control
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4a8),  0,12, 0x4a);	// RG_PE1_H_LCDDS_SSC_DELTA        //DDS SSC dither amplitude control
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4a8), 16,12, 0x4a);	// RG_PE1_H_LCDDS_SSC_DELTA1       //DDS SSC dither amplitude control for initial
#if 1 /* SSC option tune from -5000ppm to -1000ppm */
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4a8),  0,12, 0x11);	// RG_LC_DDS_SSC_DELTA
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4a8), 16,12, 0x11);	// RG_LC_DDS_SSC_DELTA1
#endif
		} else {
			printk("***** Xtal 20MHz *****\n");
#if 1 /* SSC option tune from -5000ppm to -1000ppm */
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4a8),  0,12, 0x1a);	// RG_LC_DDS_SSC_DELTA
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4a8), 16,12, 0x1a);	// RG_LC_DDS_SSC_DELTA1
#endif
		}
	}
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4a0),  5, 1, 0x01);	// RG_PE1_LCDDS_CLK_PH_INV         //DDS clock inversion
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x490), 22, 2, 0x02);	// RG_PE1_H_PLL_BC                 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x490), 18, 4, 0x06);	// RG_PE1_H_PLL_BP                 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x490), 12, 4, 0x02);	// RG_PE1_H_PLL_IR                 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x490),  8, 4, 0x01);	// RG_PE1_H_PLL_IC                 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x4ac), 16, 3, 0x00);	// RG_PE1_H_PLL_BR                 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x490),  1, 3, 0x02);	// RG_PE1_PLL_DIVEN                
	if(reg <= 5 && reg >= 3) { 	// 40MHz Xtal
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x414),  6, 2, 0x01);	// rg_pe1_mstckdiv		//value of da_pe1_mstckdiv when force mode enable
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x414),  5, 1, 0x01);	// rg_pe1_frc_mstckdiv          //force mode enable of da_pe1_mstckdiv      
	}
#if 0 /* Disable Port0&Port1 SSC */
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x414), 28, 2, 0x1);      // rg_pe1_frc_lcdds_ssc_en              //value of da_pe1_mstckdiv when force mode enable
#else
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x414), 28, 2, 0x0);      // rg_pe1_frc_lcdds_ssc_en              //value of da_pe1_mstckdiv when force mode enable
#endif
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x040), 17, 4, 0x07);	// rg_pe1_crtmsel                   //value of da[x]_pe1_crtmsel when force mode enable for Port 0
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x040), 16, 1, 0x01);	// rg_pe1_frc_crtmsel               //force mode enable of da[x]_pe1_crtmsel for Port 0
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x140), 17, 4, 0x07);	// rg_pe1_crtmsel                   //value of da[x]_pe1_crtmsel when force mode enable for Port 1
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x140), 16, 1, 0x01);	// rg_pe1_frc_crtmsel               //force mode enable of da[x]_pe1_crtmsel for Port 1
	/* Enable PHY and disable force mode */
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x000),  5, 1, 0x01);	// rg_pe1_phy_en                   //Port 0 enable
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x100),  5, 1, 0x01);	// rg_pe1_phy_en                   //Port 1 enable
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x000),  4, 1, 0x00);	// rg_pe1_frc_phy_en               //Force Port 0 disable control
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0P1_CTL_OFFSET + 0x100),  4, 1, 0x00);	// rg_pe1_frc_phy_en               //Force Port 1 disable control
#endif
#if defined (CONFIG_PCIE_PORT2)
	/* Set PCIe Port2 PHY to disable SSC */
	/* Debug Xtal Type */
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x400),  8, 1, 0x01);	// rg_pe1_frc_h_xtal_type
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x400),  9, 2, 0x00);	// rg_pe1_h_xtal_type
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x000),  4, 1, 0x01);	// rg_pe1_frc_phy_en               //Force Port 0 enable control
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x000),  5, 1, 0x00);	// rg_pe1_phy_en                   //Port 0 disable
	if(reg <= 5 && reg >= 3) { 	// 40MHz Xtal
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x490),  6, 2, 0x01);	// RG_PE1_H_PLL_PREDIV             //Pre-divider ratio (for host mode)
#if 1 /* SSC option tune from -5000ppm to -1000ppm */
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4a8),  0,12, 0x1a);	// RG_LC_DDS_SSC_DELTA
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4a8), 16,12, 0x1a);	// RG_LC_DDS_SSC_DELTA1
#endif
	} else {			// 25MHz | 20MHz Xtal
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x490),  6, 2, 0x00);	// RG_PE1_H_PLL_PREDIV             //Pre-divider ratio (for host mode)
		if (reg >= 6) { 	// 25MHz Xtal
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4bc),  4, 2, 0x01);	// RG_PE1_H_PLL_FBKSEL             //Feedback clock select
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x49c),  0,31, 0x18000000);	// RG_PE1_H_LCDDS_PCW_NCPO         //DDS NCPO PCW (for host mode)
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4a4),  0,16, 0x18d);	// RG_PE1_H_LCDDS_SSC_PRD          //DDS SSC dither period control
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4a8),  0,12, 0x4a);	// RG_PE1_H_LCDDS_SSC_DELTA        //DDS SSC dither amplitude control
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4a8), 16,12, 0x4a);	// RG_PE1_H_LCDDS_SSC_DELTA1       //DDS SSC dither amplitude control for initial
#if 1 /* SSC option tune from -5000ppm to -1000ppm */
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4a8),  0,12, 0x11);	// RG_LC_DDS_SSC_DELTA
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4a8), 16,12, 0x11);	// RG_LC_DDS_SSC_DELTA1
#endif
		} else { 		// 20MHz Xtal
#if 1 /* SSC option tune from -5000ppm to -1000ppm */
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4a8),  0,12, 0x1a);	// RG_LC_DDS_SSC_DELTA
			set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4a8), 16,12, 0x1a);	// RG_LC_DDS_SSC_DELTA1
#endif
		}
	}
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4a0),  5, 1, 0x01);	// RG_PE1_LCDDS_CLK_PH_INV         //DDS clock inversion
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x490), 22, 2, 0x02);	// RG_PE1_H_PLL_BC                 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x490), 18, 4, 0x06);	// RG_PE1_H_PLL_BP                 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x490), 12, 4, 0x02);	// RG_PE1_H_PLL_IR                 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x490),  8, 4, 0x01);	// RG_PE1_H_PLL_IC                 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x4ac), 16, 3, 0x00);	// RG_PE1_H_PLL_BR                 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x490),  1, 3, 0x02);	// RG_PE1_PLL_DIVEN                
	if(reg <= 5 && reg >= 3) { 	// 40MHz Xtal
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x414),  6, 2, 0x01);	// rg_pe1_mstckdiv		//value of da_pe1_mstckdiv when force mode enable
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x414),  5, 1, 0x01);	// rg_pe1_frc_mstckdiv          //force mode enable of da_pe1_mstckdiv      
	}
#if 0 /* Disable Port2 SSC */
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x414), 28, 2, 0x1);        // rg_pe1_frc_lcdds_ssc_en              //value of da_pe1_mstckdiv when force mode enable
#else
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x414), 28, 2, 0x0);        // rg_pe1_frc_lcdds_ssc_en              //value of da_pe1_mstckdiv when force mode enable
#endif
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x040), 17, 4, 0x07);	// rg_pe1_crtmsel                   //value of da[x]_pe1_crtmsel when force mode enable for Port 0
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x040), 16, 1, 0x01);	// rg_pe1_frc_crtmsel               //force mode enable of da[x]_pe1_crtmsel for Port 0
	/* Enable PHY and disable force mode */
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x000),  5, 1, 0x01);	// rg_pe1_phy_en                   //Port 0 enable
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P2_CTL_OFFSET + 0x000),  4, 1, 0x00);	// rg_pe1_frc_phy_en               //Force Port 0 disable control
#endif
}
#elif defined (CONFIG_MT7628_ASIC)
void pcie_phy_config(void)
{
	unsigned long reg = (*(volatile u32 *)(RALINK_SYSCTL_BASE + 0x10));

	reg = (reg >> 6) & 0x1;
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x400), 8, 1, 0x01);		// [rg_pe1_frc_h_xtal_type]: Enable Crystal type force mode
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x400), 9, 2, 0x00);		// [rg_pe1_h_xtal_type]: Force Crystal type = 20MHz 
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x000), 4, 1, 0x01);		// [rg_pe1_frc_phy_en]: Enable Port 0 force mode
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x000), 5, 1, 0x00);		// [rg_pe1_phy_en]: Port 0 disable
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x4AC),16, 3, 0x03);		// [RG_PE1_H_PLL_BR]
	if(reg == 1) {
		printk("***** Xtal 40MHz *****\n");
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x4BC),24, 8, 0x7D);	// [RG_PE1_H_PLL_FBKDIV]
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x490),12, 4, 0x08);	// [RG_PE1_H_PLL_IR]
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x490), 6, 2, 0x01);	// [RG_PE1_H_PLL_PREDIV]: Pre-divider ratio (for host mode)
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x4C0), 0,32, 0x1F400000);	// [RG_PE1_H_LCDDS_PCW_NCPO]: For 40MHz crystal input
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x4A4), 0,16, 0x013D);	// [RG_PE1_H_LCDDS_SSC_PRD]
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x4A8),16,16, 0x74);	// [RG_PE1_H_LCDDS_SSC_DELTA1]: For SSC=4500ppm
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x4A8), 0,16, 0x74);	// [RG_PE1_H_LCDDS_SSC_DELTA]: For SSC=4500ppm
	} else {
		printk("***** Xtal 25MHz *****\n");
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x4BC),24, 8, 0x64);	// [RG_PE1_H_PLL_FBKDIV]
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x490),12, 4, 0x0A);	// [RG_PE1_H_PLL_IR]
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x490), 6, 2, 0x00);	// [RG_PE1_H_PLL_PREDIV]: Pre-divider ratio (for host mode)
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x4C0), 0,32, 0x19000000);	// [RG_PE1_H_LCDDS_PCW_NCPO]: For 25MHz crystal input
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x4A4), 0,16, 0x018D);	// [RG_PE1_H_LCDDS_SSC_PRD]
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x4A8),16,16, 0x4A);	// [RG_PE1_H_LCDDS_SSC_DELTA1]: For SSC=4500ppm
		set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x4A8), 0,16, 0x4A);	// [RG_PE1_H_LCDDS_SSC_DELTA]: For SSC=4500ppm
	}
	/* MT7628 PCIe PHY LDO setting: 0x1 -> 0x5 (1.0V -> 1.2V) */
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x498), 0, 8, 0x05);

	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x000), 5, 1, 0x01);	// Port 0 enable			[rg_pe1_phy_en]
	set_pcie_phy((u32 *)(RALINK_PCIEPHY_P0_CTL_OFFSET + 0x000), 4, 1, 0x00);	// Disable Port 0 force mode		[rg_pe1_frc_phy_en]
}
#endif

int init_rt2880pci(void)
{
	unsigned long val = 0;
#if 0 /* CONFIG_RALINK_RT6855 */
	int i = 0;
#endif

#if defined(CONFIG_RALINK_MT7621)
#if defined(CONFIG_MT7621_FPGA)
	pcie_phy_init();
	//chk_phy_pll();
#endif
	ASSERT_SYSRST_PCIE(RALINK_PCIE0_RST | RALINK_PCIE1_RST | RALINK_PCIE2_RST);
	//printk("pull PCIe RST: RALINK_RSTCTRL = %x\n", RALINK_RSTCTRL);
#if defined GPIO_PERST /* use GPIO control instead of PERST_N */
	//printk("RALINK_GPIOMODE = %x\n", RALINK_GPIOMODE);
	//printk("GPIO0~31 DIR = %x\n", RALINK_GPIO_CTRL0);
	//printk("GPIO0~31 DATA = %x\n", RALINK_GPIO_DATA0);
	RALINK_GPIOMODE &= ~(0x3<<PCIE_SHARE_PIN_SW | 0x3<<UARTL3_SHARE_PIN_SW);
	RALINK_GPIOMODE |= 0x1<<PCIE_SHARE_PIN_SW | 0x1<<UARTL3_SHARE_PIN_SW;
	mdelay(100);
	//printk("RALINK_GPIOMODE (sharing pin: [11:10]/[4:3] to 1) = %x\n", RALINK_GPIOMODE);
#if defined (CONFIG_PCIE_PORT0)
	val = 0x1<<GPIO_PCIE_PORT0;
#endif
#if defined (CONFIG_PCIE_PORT1)
	val |= 0x1<<GPIO_PCIE_PORT1;
#endif
#if defined (CONFIG_PCIE_PORT2)
	val |= 0x1<<GPIO_PCIE_PORT2;
#endif
	RALINK_GPIO_CTRL0 |= val;		// switch output mode
	mdelay(100);
	//printk("RALINK_GPIOMODE (sharing pin: [11:10]/[4:3] to 1) = %x\n", RALINK_GPIOMODE);
	RALINK_GPIO_DATA0 &= ~(val);		// clear DATA
	mdelay(100);
	//printk("GPIO0~31 DIR (output 19/8/7) = %x\n", RALINK_GPIO_CTRL0);
	//printk("GPIO0~31 DATA (clear 19/8/7) = %x\n", RALINK_GPIO_DATA0);
#else
	RALINK_GPIOMODE &= ~(0x3<<PCIE_SHARE_PIN_SW);
	//printk("RALINK_GPIOMODE (sharing pin: [11:10] to 0) = %x\n", RALINK_GPIOMODE);
#endif
#if defined (CONFIG_PCIE_PORT0)
	val = RALINK_PCIE0_RST;
#endif
#if defined (CONFIG_PCIE_PORT1)
	val |= RALINK_PCIE1_RST;
#endif
#if defined (CONFIG_PCIE_PORT2)
	val |= RALINK_PCIE2_RST;
#endif
	DEASSERT_SYSRST_PCIE(val);
	printk("release PCIe RST: RALINK_RSTCTRL = %x\n", RALINK_RSTCTRL);

	printk("PCIE PHY initialize\n");
#if defined (CONFIG_MT7621_ASIC)
	if ((*(unsigned int *)(0xbe00000c)&0xFFFF) == 0x0101) // MT7621 E2
		bypass_pipe_rst();
	set_phy_for_ssc();
#endif
#endif

#if defined(CONFIG_RALINK_MT7628)
#if defined (CONFIG_MT7628_FPGA)
	printk("PCIE PHY initialize\n");
	pcie_phy_init();
	//chk_phy_pll();
#endif
	printk("RALINK_GPIOMODE = %x \n", RALINK_GPIOMODE);
	RALINK_GPIOMODE &= ~(0x1<<16);	//PCIe RESET GPIO mode
	printk("RALINK_GPIOMODE = %x \n", RALINK_GPIOMODE);
	
	RALINK_RSTCTRL &= ~RALINK_PCIE0_RST;
	RALINK_CLKCFG1 |= RALINK_PCIE0_CLK_EN;
	mdelay(100);
#if defined (CONFIG_MT7628_ASIC)
	pcie_phy_config();
#endif
#endif
#if defined(CONFIG_RALINK_MT7620)

	printk("RALINK_GPIOMODE = %x\n", RALINK_GPIOMODE);
	RALINK_GPIOMODE = (RALINK_GPIOMODE & ~(0x3<<16));	//PERST_GPIO_MODE = 2'b00
	printk("RALINK_GPIOMODE = %x\n", RALINK_GPIOMODE);

	/* enable it since bsp will disable by default for power saving */
	RALINK_RSTCTRL = (RALINK_RSTCTRL & ~RALINK_PCIE0_RST);
	RALINK_CLKCFG1 = (RALINK_CLKCFG1 | RALINK_PCIE0_CLK_EN);

	//mdelay(100);

#if defined(CONFIG_MT7620_ASIC)
	printk("PPLL_CFG1=0x%x\n", PPLL_CFG1);
	if(PPLL_CFG1 & 1<<23){
		printk("MT7620 PPLL lock\n");
	}else{
		printk("MT7620 PPLL unlock\n");
		/* for power saving */
		RALINK_RSTCTRL |= (1<<26);
		RALINK_CLKCFG1 &= ~(1<<26);
		return 0;
	}
	PPLL_DRV |= 0x1 << 19;		//PCIe clock driver power ON
	PPLL_DRV &= ~(0x1<<18);		//Reference PCIe Output clock mode enable
	PPLL_DRV &= ~(0x1<<17);		//PCIe PHY clock enable
	PPLL_DRV |= 0x1 << 31;		//PDRV SW Set
	printk("PPLL_DRV =0x%x\n", PPLL_DRV);

	//mdelay(100);
#endif
#endif
#if defined(CONFIG_PCIE_ONLY) || defined(CONFIG_PCIE_PCI_CONCURRENT)
	RALINK_RSTCTRL = (RALINK_RSTCTRL | RALINK_PCIE_RST);
	RALINK_SYSCFG1 &= ~(0x30);
	RALINK_SYSCFG1 |= (2<<4);
	RALINK_PCIE_CLK_GEN &= 0x7fffffff;
	RALINK_PCIE_CLK_GEN1 &= 0x80ffffff;
	RALINK_PCIE_CLK_GEN1 |= 0xa << 24;
	RALINK_PCIE_CLK_GEN |= 0x80000000;
	mdelay(50);
	RALINK_RSTCTRL = (RALINK_RSTCTRL & ~RALINK_PCIE_RST);
#endif
	
#ifdef CONFIG_RALINK_RT3883
#if 0
	printk("before\n");
	printk("RALINK_GPIOMODE = %x\n", RALINK_GPIOMODE);
	printk("RALINK_SYSCFG1 = %x\n", RALINK_SYSCFG1);
	printk("RALINK_RSTCTRL = %x\n", RALINK_RSTCTRL);
	printk("RALINK_CLKCFG1 = %x\n", RALINK_CLKCFG1);
	printk("RALINK_PCIE_CLK_GEN= %x\n", RALINK_PCIE_CLK_GEN);
	printk("RALINK_PCIE_CLK_GEN1= %x\n", RALINK_PCIE_CLK_GEN1);
	printk("**************************\n");
#endif

#ifdef CONFIG_PCI_ONLY
//PCI host only, 330T
	RALINK_GPIOMODE = ((RALINK_GPIOMODE & (~(0x3800))) | PCI_SLOTx2);
	RALINK_SYSCFG1 = (RALINK_SYSCFG1 | RALINK_PCI_HOST_MODE_EN | RALINK_PCIE_RC_MODE_EN);
	RALINK_RSTCTRL = (RALINK_RSTCTRL | RALINK_PCIE_RST);
	RALINK_CLKCFG1 = (RALINK_CLKCFG1 & ~RALINK_PCIE_CLK_EN);
#elif defined (CONFIG_PCIE_ONLY)
//PCIe RC only, 220T
	RALINK_SYSCFG1 = (RALINK_SYSCFG1 | RALINK_PCIE_RC_MODE_EN | RALINK_PCI_HOST_MODE_EN);
	RALINK_RSTCTRL = (RALINK_RSTCTRL | RALINK_PCI_RST);
	RALINK_CLKCFG1 = (RALINK_CLKCFG1 & ~RALINK_PCI_CLK_EN);
#elif defined (CONFIG_PCIE_PCI_CONCURRENT)
//PCIe PCI co-exist
	RALINK_GPIOMODE = ((RALINK_GPIOMODE & ~(0x3800)) | PCI_SLOTx2);
	RALINK_SYSCFG1 = (RALINK_SYSCFG1 | RALINK_PCI_HOST_MODE_EN | RALINK_PCIE_RC_MODE_EN);
#endif
	mdelay(500);

#if 0
	printk("after\n");
	printk("RALINK_GPIOMODE = %x\n", RALINK_GPIOMODE);
	printk("RALINK_SYSCFG1 = %x\n", RALINK_SYSCFG1);
	printk("RALINK_RSTCTRL = %x\n", RALINK_RSTCTRL);
	printk("RALINK_CLKCFG1 = %x\n", RALINK_CLKCFG1);
	printk("RALINK_PCIE_CLK_GEN= %x\n", RALINK_PCIE_CLK_GEN);
	printk("RALINK_PCIE_CLK_GEN1= %x\n", RALINK_PCIE_CLK_GEN1);
	printk("**************************\n");
#endif
#endif

#ifdef CONFIG_RALINK_RT2880
	//pci_probe_only = 1;
	RALINK_PCI_PCICFG_ADDR = 0;
#elif defined (CONFIG_RALINK_RT2883)
	RALINK_PCI_PCICFG_ADDR = 0;
#elif defined (CONFIG_RALINK_RT3883)

#ifdef CONFIG_PCIE_ONLY
	RALINK_PCI_PCICFG_ADDR = 0;
	//RALINK_PCI_PCICFG_ADDR |= (1<<16);
#elif defined (CONFIG_PCI_ONLY)
	RALINK_PCI_PCICFG_ADDR = 0;
	RALINK_PCI_PCICFG_ADDR |= (1<<16);
#elif defined (CONFIG_PCIE_PCI_CONCURRENT)
	RALINK_PCI_PCICFG_ADDR = 0;
	RALINK_PCI_PCICFG_ADDR |= (1<<16);
#endif
#elif defined(CONFIG_RALINK_RT6855) || defined(CONFIG_RALINK_MT7620) || defined(CONFIG_RALINK_MT7628)
	//RALINK_PCI_PCICFG_ADDR = 0;
	//RALINK_PCI_PCICFG_ADDR |= (1<<20); //DEV0 = 0; DEV1 = 1
	printk("start PCIe register access\n");
	RALINK_PCI_PCICFG_ADDR &= ~(1<<1); //de-assert PERST
	//printk("RALINK_PCI_PCICFG_ADDR= %x\n", RALINK_PCI_PCICFG_ADDR);
	//RALINK_RSTCTRL = (RALINK_RSTCTRL | RALINK_PCIE1_RST);
	//printk("RALINK_RSTCTRL= %x\n", RALINK_RSTCTRL);
#elif defined(CONFIG_RALINK_MT7621)
	printk("start MT7621 PCIe register access\n");
#if defined GPIO_PERST /* add GPIO control instead of PERST_N */
#if defined (CONFIG_PCIE_PORT0)
	val = 0x1<<GPIO_PCIE_PORT0;
#endif
#if defined (CONFIG_PCIE_PORT1)
	val |= 0x1<<GPIO_PCIE_PORT1;
#endif
#if defined (CONFIG_PCIE_PORT2)
	val |= 0x1<<GPIO_PCIE_PORT2;
#endif
	//printk("GPIO0~31 DIR (output 19/8/7) = %x\n", *(unsigned int *)(0xbe000600));
	RALINK_GPIO_DATA0 |= val;		// set DATA
	mdelay(100);
	//printk("GPIO0~31 DATA (set 19/8/7) = %x\n", *(unsigned int *)(0xbe000620));
#else
	printk("RALINK_PCI_PCICFG_ADDR= %x\n", RALINK_PCI_PCICFG_ADDR);
	RALINK_PCI_PCICFG_ADDR &= ~(1<<1); //de-assert PERST
	printk("RALINK_PCI_PCICFG_ADDR = %x\n", RALINK_PCI_PCICFG_ADDR);
#endif
#endif
	mdelay(500);

	printk("RALINK_RSTCTRL = %x\n", RALINK_RSTCTRL);
	printk("RALINK_CLKCFG1 = %x\n", RALINK_CLKCFG1);

#ifdef CONFIG_RALINK_RT3883
	printk("\n*************** Ralink PCIe RC mode *************\n");
	mdelay(500);
	if(RALINK_SYSCFG1 & RALINK_PCIE_RC_MODE_EN){
		if(( RALINK_PCI1_STATUS & 0x1) == 0)
		{
			printk(" RALINK_PCI1_STATUS = %x\n", RALINK_PCI1_STATUS );
			for(i=0;i<16;i++){
				read_config(0, 1, 0, i<<2, &val);
				printk("pci-to-pci 0x%02x = %08x\n", i<<2, (int)val);
			}
#ifdef CONFIG_PCIE_ONLY
			printk("reset PCIe and turn off PCIe clock\n");
			RALINK_RSTCTRL = (RALINK_RSTCTRL | RALINK_PCIE_RST);
			RALINK_RSTCTRL = (RALINK_RSTCTRL & ~RALINK_PCIE_RST);
			RALINK_CLKCFG1 = (RALINK_CLKCFG1 & ~RALINK_PCIE_CLK_EN);
			printk("RALINK_CLKCFG1 = %x\n", RALINK_CLKCFG1);
			//cgrstb, cgpdb, pexdrven0, pexdrven1, cgpllrstb, cgpllpdb, pexclken
			RALINK_PCIE_CLK_GEN &= 0x0fff3f7f;
			printk("RALINK_PCIE_CLK_GEN= %x\n", RALINK_PCIE_CLK_GEN);
			return 0;
#else
			RALINK_CLKCFG1 = (RALINK_CLKCFG1 & ~RALINK_PCIE_CLK_EN);
#endif
		}
	}
	if(RALINK_SYSCFG1 & RALINK_PCI_HOST_MODE_EN){
		RALINK_PCI_ARBCTL = 0x79;
	}

#elif defined (CONFIG_RALINK_RT6855)
	printk("\n*************** RT6855 PCIe RC mode *************\n");
	mdelay(500);
	if(( RALINK_PCI0_STATUS & 0x1) == 0)
	{
		//RALINK_RSTCTRL = (RALINK_RSTCTRL | RALINK_PCIE0_RST);
		RALINK_CLKCFG1 = (RALINK_CLKCFG1 & ~RALINK_PCIE0_CLK_EN);
		printk("PCIE0 no card, disable it(RST&CLK)\n");
		pcie0_disable=1;
	}
	if(( RALINK_PCI1_STATUS & 0x1) == 0)
	{
		//RALINK_RSTCTRL = (RALINK_RSTCTRL | RALINK_PCIE1_RST);
		RALINK_CLKCFG1 = (RALINK_CLKCFG1 & ~RALINK_PCIE1_CLK_EN);
		printk("PCIE1 no card, disable it(RST&CLK)\n");
		pcie1_disable=1;
	}else{
		if(pcie0_disable==1){
			/* pcie0 no card, pcie1 has card */
			//James want to go back, next two line
			//RALINK_PCI_PCICFG_ADDR &= ~(0xff<<16);
			//RALINK_PCI_PCICFG_ADDR |= 1<<16;
			//printk("***RALINK_PCI_PCICFG_ADDR= %x\n", RALINK_PCI_PCICFG_ADDR);
		}
	}
#elif defined(CONFIG_RALINK_MT7620)
	printk("\n*************** MT7620 PCIe RC mode *************\n");
	mdelay(500);
	if(( RALINK_PCI0_STATUS & 0x1) == 0)
	{
		RALINK_RSTCTRL = (RALINK_RSTCTRL | RALINK_PCIE0_RST);
		RALINK_CLKCFG1 = (RALINK_CLKCFG1 & ~RALINK_PCIE0_CLK_EN);
		PPLL_DRV = (PPLL_DRV & ~LC_CKDRVPD_);
		PPLL_DRV = (PPLL_DRV | PDRV_SW_SET);
		printk("PCIE0 no card, disable it(RST&CLK)\n");
		pcie0_disable=1;
	}
#elif defined (CONFIG_RALINK_MT7621)
	printk("\n*************** MT7621 PCIe RC mode *************\n");
	mdelay(500);
#if defined (CONFIG_PCIE_PORT0)
	if(( RALINK_PCI0_STATUS & 0x1) == 0)
	{
		printk("PCIE0 no card, disable it(RST&CLK)\n");
		ASSERT_SYSRST_PCIE(RALINK_PCIE0_RST);
		RALINK_CLKCFG1 = (RALINK_CLKCFG1 & ~RALINK_PCIE0_CLK_EN);
		pcie_link_status &= ~(1<<0);
	} else {
		pcie_link_status |= 1<<0;
		RALINK_PCI_PCIMSK_ADDR |= (1<<20); // enable pcie1 interrupt
	}
#endif
#if defined (CONFIG_PCIE_PORT1)
	if(( RALINK_PCI1_STATUS & 0x1) == 0)
	{
		printk("PCIE1 no card, disable it(RST&CLK)\n");
		ASSERT_SYSRST_PCIE(RALINK_PCIE1_RST);
		RALINK_CLKCFG1 = (RALINK_CLKCFG1 & ~RALINK_PCIE1_CLK_EN);
		pcie_link_status &= ~(1<<1);
	} else {
		pcie_link_status |= 1<<1;
		RALINK_PCI_PCIMSK_ADDR |= (1<<21); // enable pcie1 interrupt
	}
#endif
#if defined (CONFIG_PCIE_PORT2)
	if (( RALINK_PCI2_STATUS & 0x1) == 0) {
		printk("PCIE2 no card, disable it(RST&CLK)\n");
		ASSERT_SYSRST_PCIE(RALINK_PCIE2_RST);
		RALINK_CLKCFG1 = (RALINK_CLKCFG1 & ~RALINK_PCIE2_CLK_EN);
		pcie_link_status &= ~(1<<2);
	} else {
		pcie_link_status |= 1<<2;
		RALINK_PCI_PCIMSK_ADDR |= (1<<22); // enable pcie2 interrupt
	}
#endif
	printk("pcie_link status = 0x%x\n", pcie_link_status);
	printk("RALINK_RSTCTRL= %x\n", RALINK_RSTCTRL);
	if (pcie_link_status == 0)
		return 0;

	printk("*** Configure Device number setting of Virtual PCI-PCI bridge ***\n");
/*
pcie(2/1/0) link status	pcie2_num	pcie1_num	pcie0_num
3'b000			x		x		x
3'b001			x		x		0
3'b010			x		0		x
3'b011			x		1		0
3'b100			0		x		x
3'b101			1		x		0
3'b110			1		0		x
3'b111			2		1		0
*/
	printk("RALINK_PCI_PCICFG_ADDR = %x", RALINK_PCI_PCICFG_ADDR);
	switch(pcie_link_status) {
	case 2:
		RALINK_PCI_PCICFG_ADDR &= ~0x00ff0000;
		RALINK_PCI_PCICFG_ADDR |= 0x1 << 16;	//port0
		RALINK_PCI_PCICFG_ADDR |= 0x0 << 20;	//port1
		break;
	case 4:
		RALINK_PCI_PCICFG_ADDR &= ~0x0fff0000;
		RALINK_PCI_PCICFG_ADDR |= 0x1 << 16;	//port0
		RALINK_PCI_PCICFG_ADDR |= 0x2 << 20;	//port1
		RALINK_PCI_PCICFG_ADDR |= 0x0 << 24;	//port2
		break;
	case 5:
		RALINK_PCI_PCICFG_ADDR &= ~0x0fff0000;
		RALINK_PCI_PCICFG_ADDR |= 0x0 << 16;	//port0
		RALINK_PCI_PCICFG_ADDR |= 0x2 << 20;	//port1
		RALINK_PCI_PCICFG_ADDR |= 0x1 << 24;	//port2
		break;
	case 6:
		RALINK_PCI_PCICFG_ADDR &= ~0x0fff0000;
		RALINK_PCI_PCICFG_ADDR |= 0x2 << 16;	//port0
		RALINK_PCI_PCICFG_ADDR |= 0x0 << 20;	//port1
		RALINK_PCI_PCICFG_ADDR |= 0x1 << 24;	//port2
		break;
	}
	printk(" -> %x\n", RALINK_PCI_PCICFG_ADDR);
#elif defined(CONFIG_RALINK_MT7628)
	printk("\n*************** MT7628 PCIe RC mode *************\n");
	mdelay(500);
	if(( RALINK_PCI0_STATUS & 0x1) == 0)
	{
		RALINK_RSTCTRL = (RALINK_RSTCTRL | RALINK_PCIE0_RST);
		RALINK_CLKCFG1 = (RALINK_CLKCFG1 & ~RALINK_PCIE0_CLK_EN);
		printk("PCIE0 no card, disable it(RST&CLK)\n");
		pcie0_disable=1;
		return 0;
	}
#elif defined (CONFIG_RALINK_RT2883)
	printk("\n*************** Ralink PCIe RC mode *************\n");
	mdelay(500);
	if(( RALINK_PCI_STATUS & 0x1) == 0)
	{
		printk(" RALINK_PCI_STATUS = %x\n", RALINK_PCI_STATUS );
		printk("************No PCIE device**********\n");
		for(i=0;i<16;i++){
			read_config(0, 0, 0, i<<2, &val);
			printk("pci-to-pci 0x%02x = %08x\n", i<<2, val);
		}
		return 0;
	}
#else
	for(i=0;i<0xfffff;i++);
	RALINK_PCI_ARBCTL = 0x79;
#endif	
	//printk(" RALINK_PCI_ARBCTL = %x\n", RALINK_PCI_ARBCTL);

/*
	ioport_resource.start = rt2880_res_pci_io1.start;
  	ioport_resource.end = rt2880_res_pci_io1.end;
*/

	RALINK_PCI_MEMBASE = 0xffffffff; //RALINK_PCI_MM_MAP_BASE;
	RALINK_PCI_IOBASE = RALINK_PCI_IO_MAP_BASE;

#ifdef CONFIG_RALINK_RT2880
	RALINK_PCI_BAR0SETUP_ADDR = 0x07FF0000;	//open 1FF:32M; DISABLE
	RALINK_PCI_IMBASEBAR0_ADDR = MEMORY_BASE;
	RALINK_PCI_ID = 0x08021814;
	RALINK_PCI_CLASS = 0x00800001;
	RALINK_PCI_SUBID = 0x28801814;
#elif defined (CONFIG_RALINK_RT2883)
	RALINK_PCI_BAR0SETUP_ADDR = 0x01FF0000;	//open 1FF:32M; DISABLE
	RALINK_PCI_IMBASEBAR0_ADDR = MEMORY_BASE;
	RALINK_PCI_ID = 0x08021814;
	RALINK_PCI_CLASS = 0x06040001;
	RALINK_PCI_SUBID = 0x28801814;
#elif defined (CONFIG_RALINK_RT3883)
	//PCI
	RALINK_PCI0_BAR0SETUP_ADDR = 0x03FF0000;	//open 3FF:64M; DISABLE
	RALINK_PCI0_IMBASEBAR0_ADDR = MEMORY_BASE;
	RALINK_PCI0_ID = 0x08021814;
	RALINK_PCI0_CLASS = 0x00800001;
	RALINK_PCI0_SUBID = 0x28801814;
	//PCIe
	RALINK_PCI1_BAR0SETUP_ADDR = 0x03FF0000;	//open 3FF:64M; DISABLE
	RALINK_PCI1_IMBASEBAR0_ADDR = MEMORY_BASE;
	RALINK_PCI1_ID = 0x08021814;
	RALINK_PCI1_CLASS = 0x06040001;
	RALINK_PCI1_SUBID = 0x28801814;
#elif defined (CONFIG_RALINK_RT6855) 
	//PCIe0
	//if(pcie0_disable!=1){
	RALINK_PCI0_BAR0SETUP_ADDR = 0x03FF0000;	//open 3FF:64M; DISABLE
	RALINK_PCI0_IMBASEBAR0_ADDR = MEMORY_BASE;
	RALINK_PCI0_ID = 0x08021814;
	RALINK_PCI0_CLASS = 0x06040001;
	RALINK_PCI0_SUBID = 0x28801814;
	//}
	//PCIe1
	//if(pcie1_disable!=1){
	RALINK_PCI1_BAR0SETUP_ADDR = 0x03FF0000;	//open 3FF:64M; DISABLE
	RALINK_PCI1_IMBASEBAR0_ADDR = MEMORY_BASE;
	RALINK_PCI1_ID = 0x08021814;
	RALINK_PCI1_CLASS = 0x06040001;
	RALINK_PCI1_SUBID = 0x28801814;
	//}
#elif defined(CONFIG_RALINK_MT7620) || defined(CONFIG_RALINK_MT7628)
	//PCIe0
	//if(pcie0_disable!=1){
	RALINK_PCI0_BAR0SETUP_ADDR = 0x7FFF0001;	//open 7FFF:2G; ENABLE
	RALINK_PCI0_IMBASEBAR0_ADDR = MEMORY_BASE;
	//RALINK_PCI0_ID = 0x08021814;
	RALINK_PCI0_CLASS = 0x06040001;
	//RALINK_PCI0_SUBID = 0x28801814;
	//}
	printk("PCIE0 enabled\n");
#elif defined(CONFIG_RALINK_MT7621)
#if defined (CONFIG_PCIE_PORT0)
	//PCIe0
	if((pcie_link_status & 0x1) != 0) {
		RALINK_PCI0_BAR0SETUP_ADDR = 0x7FFF0001;	//open 7FFF:2G; ENABLE
		RALINK_PCI0_IMBASEBAR0_ADDR = MEMORY_BASE;
		RALINK_PCI0_CLASS = 0x06040001;
		printk("PCIE0 enabled\n");
	}
#endif
#if defined (CONFIG_PCIE_PORT1)
	//PCIe1
	if ((pcie_link_status & 0x2) != 0) {
		RALINK_PCI1_BAR0SETUP_ADDR = 0x7FFF0001;	//open 7FFF:2G; ENABLE
		RALINK_PCI1_IMBASEBAR0_ADDR = MEMORY_BASE;
		RALINK_PCI1_CLASS = 0x06040001;
		printk("PCIE1 enabled\n");
	}
#endif
#if defined (CONFIG_PCIE_PORT2)
	//PCIe2
	if ((pcie_link_status & 0x4) != 0) {
		RALINK_PCI2_BAR0SETUP_ADDR = 0x7FFF0001;	//open 7FFF:2G; ENABLE
		RALINK_PCI2_IMBASEBAR0_ADDR = MEMORY_BASE;
		RALINK_PCI2_CLASS = 0x06040001;
		printk("PCIE2 enabled\n");
	}
#endif
#endif

#if defined (CONFIG_RALINK_RT3883)
	RALINK_PCI_PCIMSK_ADDR = 0x001c0000; // enable pcie/pci interrupt
#elif defined (CONFIG_RALINK_RT6855)
	//if(pcie0_disable!=1){
	RALINK_PCI_PCIMSK_ADDR |= (1<<20); // enable pcie0 interrupt
	//}
	//if(pcie1_disable!=1){
	RALINK_PCI_PCIMSK_ADDR |= (1<<21); // enable pcie1 interrupt
	//}
#elif defined(CONFIG_RALINK_MT7620) || defined(CONFIG_RALINK_MT7628)
	//if(pcie0_disable!=1){
	RALINK_PCI_PCIMSK_ADDR |= (1<<20); // enable pcie0 interrupt
	//}
#elif defined (CONFIG_RALINK_MT7621)
	printk("interrupt enable status: %x\n", RALINK_PCI_PCIMSK_ADDR);
#else
	RALINK_PCI_PCIMSK_ADDR = 0x000c0000; // enable pci interrupt
#endif

#if defined (CONFIG_RALINK_RT3883)
	//PCIe
	read_config(0, 1, 0, 0x4, &val);
	write_config(0, 1, 0, 0x4, val|0x7);
	//PCI
	read_config(0, 0, 0, 0x4, &val);
	write_config(0, 0, 0, 0x4, val|0x7);
#elif defined (CONFIG_RALINK_RT6855)
	//PCIe0
	//if(pcie0_disable==0 || pcie1_disable==0){
	read_config(0, 0, 0, 0x4, &val);
	write_config(0, 0, 0, 0x4, val|0x7);
	//printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	//}
	//PCIe1
	//if(pcie0_disable==0 && pcie1_disable==0){
	read_config(0, 1, 0, 0x4, &val);
	write_config(0, 1, 0, 0x4, val|0x7);
	//printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	//}
#elif defined(CONFIG_RALINK_MT7620) || defined(CONFIG_RALINK_MT7628)
	//PCIe0
	//if(pcie0_disable==0 || pcie1_disable==0){
	read_config(0, 0, 0, 0x4, &val);
	write_config(0, 0, 0, 0x4, val|0x7);
	//printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	//}
	read_config(0, 0, 0, 0x70c, &val);
	val &= ~(0xff)<<8;
	val |= 0x50<<8;
	write_config(0, 0, 0, 0x70c, val);
	read_config(0, 0, 0, 0x70c, &val);
	printk("Port 0 N_FTS = %x\n", (unsigned int)val);
#elif defined (CONFIG_RALINK_MT7621)
	switch(pcie_link_status) {
	case 7:
		read_config(0, 2, 0, 0x4, &val);
		write_config(0, 2, 0, 0x4, val|0x4);
		// write_config(0, 1, 0, 0x4, val|0x7);
		read_config(0, 2, 0, 0x70c, &val);
		val &= ~(0xff)<<8;
		val |= 0x50<<8;
		write_config(0, 2, 0, 0x70c, val);
		read_config(0, 2, 0, 0x70c, &val);
		printk("Port 2 N_FTS = %x\n", (unsigned int)val);
	case 3:
	case 5:
	case 6:
		read_config(0, 1, 0, 0x4, &val);
		write_config(0, 1, 0, 0x4, val|0x4);
		// write_config(0, 1, 0, 0x4, val|0x7);
		read_config(0, 1, 0, 0x70c, &val);
		val &= ~(0xff)<<8;
		val |= 0x50<<8;
		write_config(0, 1, 0, 0x70c, val);
		read_config(0, 1, 0, 0x70c, &val);
		printk("Port 1 N_FTS = %x\n", (unsigned int)val);
	default:
		read_config(0, 0, 0, 0x4, &val);
		write_config(0, 0, 0, 0x4, val|0x4); //bus master enable
		// write_config(0, 0, 0, 0x4, val|0x7); //bus master enable
		read_config(0, 0, 0, 0x70c, &val);
		val &= ~(0xff)<<8;
		val |= 0x50<<8;
		write_config(0, 0, 0, 0x70c, val);
		read_config(0, 0, 0, 0x70c, &val);
		printk("Port 0 N_FTS = %x\n", (unsigned int)val);
	}
	printk("config reg done\n");
#elif defined (CONFIG_RALINK_RT2883)
	read_config(0, 0, 0, 0x4, &val);
	write_config(0, 0, 0, 0x4, val|0x7);
	//FIXME
	////write_config(0, 0, 0, 0x18, 0x10100);
	//write_config(0, 0, 0, PCI_BASE_ADDRESS_0, MEMORY_BASE);
	//read_config(0, 0, 0, PCI_BASE_ADDRESS_0, &val); 
	////printk("BAR0 at slot 0 = %x\n", val); 
#else 
	write_config(0, 0, 0, PCI_BASE_ADDRESS_0, MEMORY_BASE); 
	read_config(0, 0, 0, PCI_BASE_ADDRESS_0, &val);
	printk("BAR0 at slot 0 = %x\n", val);
#endif
#if 0 /*CONFIG_RALINK_RT6855*/

	for(i=0;i<16;i++){
	read_config(0, 0, 0, i<<2, &val);
	printk("PCI-to-PCI bridge0 0x%02x = %08x\n", i<<2, (unsigned int)val);
	}
	for(i=0;i<16;i++){
	read_config(0, 1, 0, i<<2, &val);
	printk("PCI-to-PCI bridge1 0x%02x = %08x\n", i<<2, (unsigned int)val);
	}
#endif
	printk("init_rt2880pci done\n");
	register_pci_controller(&rt2880_controller);
	return 0;

}
#ifndef CONFIG_PCIE_PCI_NONE
arch_initcall(init_rt2880pci);
#endif

/* Do platform specific device initialization at pci_enable_device() time */
int pcibios_plat_dev_init(struct pci_dev *dev)
{
	return 0;
}

struct pci_fixup pcibios_fixups[] = {
//	{PCI_ANY_ID, PCI_ANY_ID, pcibios_fixup_resources },
	{0}
};
//DECLARE_PCI_FIXUP_FINAL(PCI_ANY_ID, PCI_ANY_ID, pcibios_fixup_resources)
#endif	/* CONFIG_PCI */
