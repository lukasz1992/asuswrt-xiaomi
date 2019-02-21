/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright, Ralink Technology, Inc.
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
 ***************************************************************************
 *
 */

#include <linux/init.h>
#include <linux/version.h>
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>   
#include <linux/fs.h>       
#include <linux/errno.h>
#include <linux/slab.h> 
#include <linux/types.h>    
#include <linux/proc_fs.h>
#include <linux/fcntl.h>    
#include <linux/delay.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
#include <asm/system.h>
#endif
#include <linux/wireless.h>

#include "i2c_drv.h"
#include "ralink_gpio.h"

#ifdef  CONFIG_DEVFS_FS
#include <linux/devfs_fs_kernel.h>
#endif

#ifdef  CONFIG_DEVFS_FS
static devfs_handle_t devfs_handle;
#endif


#if defined(CONFIG_MTK_NFC_SUPPORT)
  #if defined(CONFIG_RALINK_RT3883)
    #define MT6605_GPIO_IND         9  // MT6605_IRQ
    #define MT6605_GPIO_VEN         13 // MT6605_VEN
    #define MT6605_GPIO_RESET       11 // MT6605_RESET
  #elif defined(CONFIG_RALINK_MT7621)
    #define MT6605_GPIO_IND         10 // MT6605_IRQ
    #define MT6605_GPIO_VEN         9  // MT6605_VEN
    #define MT6605_GPIO_RESET       6  // MT6605_RESET
  #else
    #error "chip is not support"
  #endif
#endif

#if defined(CONFIG_MTK_NFC_MT6605_SIM)
extern void mt6605_sim(void);
#endif

#if defined (CONFIG_FB_MEDIATEK_ILITEK) || defined (CONFIG_FB_MEDIATEK_TRULY)&& defined (CONFIG_RALINK_MT7621)
int TOUCH_GPIO_RESET = 12; //NRST ==>GPIO12
u32 value_user;
#endif
int i2cdrv_major =  218;
unsigned long i2cdrv_addr = ATMEL_ADDR;
//unsigned long i2cdrv_addr = 0x70;
unsigned long address_bytes= 2;
unsigned long clkdiv_value = CLKDIV_VALUE;

unsigned long switch_address_bytes(unsigned long addr_bytes)
{
	address_bytes = addr_bytes;
	//printk("I2C controller address bytes is %x\n", addr_bytes);
	return address_bytes;
}

/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*           	i2c_master_init                                         */
/*    INPUTS: None                                                      */
/*   RETURNS: None                                                      */
/*   OUTPUTS: None                                                      */
/*   NOTE(S): Initialize I2C block to desired state                     */
/*                                                                      */
/*----------------------------------------------------------------------*/
void i2c_master_init(void)
{
	u32 i;
#if defined (CONFIG_FB_MEDIATEK_ILITEK) || defined (CONFIG_FB_MEDIATEK_TRULY)&& defined (CONFIG_RALINK_MT7621)
    RT2880_REG(RALINK_REG_PIODIR) =0x1200 ;
    //printk("RALINK_REG_PIODIR = %x\n", RT2880_REG(RALINK_REG_PIODIR));
   // RT2880_REG(RALINK_REG_PIODIR) = (1 << TOUCH_GPIO_RESET);
		RT2880_REG(RALINK_REG_PIOSET) = (1 << TOUCH_GPIO_RESET);
		mdelay(10);
		RT2880_REG(RALINK_REG_PIORESET) = (1 << TOUCH_GPIO_RESET);
		mdelay(10);
		RT2880_REG(RALINK_REG_PIOSET) = (1 << TOUCH_GPIO_RESET);
		mdelay(120);	

#endif
#if defined (CONFIG_RALINK_MT7621)
	RT2880_REG(RALINK_SYSCTL_BASE + 0x60) &= ~0x4;  //MT7621 bit2
	udelay(500);
#elif defined (CONFIG_RALINK_MT7628)
	RT2880_REG(RALINK_SYSCTL_BASE + 0x60) &= ~(0x3<<20);//~0x3000;  //MT7628 bit20
	udelay(500);
#else
	RT2880_REG(RALINK_SYSCTL_BASE+0x60) &= ~0x1;	
#endif
	/* reset i2c block */
	i = RT2880_REG(RT2880_RSTCTRL_REG) | RALINK_I2C_RST;
	RT2880_REG(RT2880_RSTCTRL_REG) = i;
	RT2880_REG(RT2880_RSTCTRL_REG) = i & ~(RALINK_I2C_RST);

	udelay(500);
	
	RT2880_REG(RT2880_I2C_CONFIG_REG) = I2C_CFG_DEFAULT;

#if defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
        i = 1 << 31; // the output is pulled hight by SIF master 0
        i |= 1 << 28; // allow triggered in VSYNC pulse
        i |= clkdiv_value<<16;//CLKDIV_VALUE << 16; //clk div
        i |= 1 << 6; // output H when SIF master 0 is in WAIT state
        i |= 1 << 1; // Enable SIF master 0
        RT2880_REG(RT2880_I2C_SM0CTL0) = i;

        RT2880_REG(RT2880_I2C_SM0_IS_AUTOMODE) = 1; //auto mode
#else
	RT2880_REG(RT2880_I2C_CLKDIV_REG) = CLKDIV_VALUE;
#endif

	/*
	 * Device Address : 
	 *
	 * ATMEL 24C152 serial EEPROM
	 *       1|0|1|0|0|A1|A2|R/W
	 *      MSB              LSB
	 * 
	 * ATMEL 24C01A/02/04/08A/16A
	 *    	MSB               LSB	  
	 * 1K/2K 1|0|1|0|A2|A1|A0|R/W
	 * 4K            A2 A1 P0
	 * 8K            A2 P1 P0
	 * 16K           P2 P1 P0 
	 *
	 * so device address needs 7 bits 
	 * if device address is 0, 
	 * write 0xA0 >> 1 into DEVADDR(max 7-bits) REG  
	 */
	RT2880_REG(RT2880_I2C_DEVADDR_REG) = i2cdrv_addr;

	/*
	 * Use Address Disabled Transfer Options
	 * because it just support 8-bits, 
	 * ATMEL eeprom needs two 8-bits address
	 */
	RT2880_REG(RT2880_I2C_ADDR_REG) = 0;
}


/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*           	i2c_WM8751_write                                               */
/*    INPUTS: 8-bit data                                                */
/*   RETURNS: None                                                      */
/*   OUTPUTS: None                                                      */
/*   NOTE(S): transfer 8-bit data to I2C                                */
/*                                                                      */
/*----------------------------------------------------------------------*/
#if defined (CONFIG_RALINK_I2S) || defined (CONFIG_RALINK_I2S_MODULE)
void i2c_WM8751_write(u32 address, u32 data)
{
	int i, j;
	unsigned long old_addr = i2cdrv_addr;
	
#if defined(CONFIG_SND_SOC_WM8960) || defined(CONFIG_I2S_WM8960)
	i2cdrv_addr = WM8960_ADDR;
#elif defined(CONFIG_SND_SOC_WM8750) || defined(CONFIG_I2S_WM8750) || defined(CONFIG_I2S_WM8751)
	i2cdrv_addr = WM8751_ADDR;
#else
	i2cdrv_addr = WM8960_ADDR;
#endif
	
	i2c_master_init();	
	
	/* two bytes data at least so NODATA = 0 */

	RT2880_REG(RT2880_I2C_BYTECNT_REG) = 1;
	
	RT2880_REG(RT2880_I2C_DATAOUT_REG) = (address<<1)|(0x01&(data>>8));

	RT2880_REG(RT2880_I2C_STARTXFR_REG) = WRITE_CMD;

	j = 0;
	do {
		if (IS_SDOEMPTY) {	
			RT2880_REG(RT2880_I2C_DATAOUT_REG) = (data&0x0FF);	
			break;
		}
	} while (++j<max_ee_busy_loop);
	
	i = 0;
	while(IS_BUSY && i<i2c_busy_loop){
		i++;
	};
	i2cdrv_addr = old_addr;
}
#endif

/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*           	i2c_write                                               */
/*    INPUTS: 8-bit data                                                */
/*   RETURNS: None                                                      */
/*   OUTPUTS: None                                                      */
/*   NOTE(S): transfer 8-bit data to I2C                                */
/*                                                                      */
/*----------------------------------------------------------------------*/
static void i2c_write(u32 address, u8 *data, u32 nbytes)
{
	int i, j;
	u32 n;

	/* two bytes data at least so NODATA = 0 */
	n = nbytes + address_bytes;
	RT2880_REG(RT2880_I2C_BYTECNT_REG) = n-1;
	if (address_bytes == 2)
		RT2880_REG(RT2880_I2C_DATAOUT_REG) = (address >> 8) & 0xFF;
	else
		RT2880_REG(RT2880_I2C_DATAOUT_REG) = address & 0xFF;

	RT2880_REG(RT2880_I2C_STARTXFR_REG) = WRITE_CMD;
	for (i=0; i<n-1; i++) {
		j = 0;
		do {
			if (IS_SDOEMPTY) {
				if (address_bytes == 2) {
					if (i==0) {
						RT2880_REG(RT2880_I2C_DATAOUT_REG) = address & 0xFF;
					} else {
						RT2880_REG(RT2880_I2C_DATAOUT_REG) = data[i-1];
					}								
				} else {
					RT2880_REG(RT2880_I2C_DATAOUT_REG) = data[i];
				}
 			break;
			}
		} while (++j<max_ee_busy_loop);
	}

	i = 0;
	while(IS_BUSY && i<i2c_busy_loop){
		i++;
	};

}

/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*           	i2c_read                                                */
/*    INPUTS: None                                                      */
/*   RETURNS: 8-bit data                                                */
/*   OUTPUTS: None                                                      */
/*   NOTE(S): get 8-bit data from I2C                                   */
/*                                                                      */
/*----------------------------------------------------------------------*/
static void i2c_read(u8 *data, u32 nbytes) 
{
	int i, j;

	RT2880_REG(RT2880_I2C_BYTECNT_REG) = nbytes-1;
	RT2880_REG(RT2880_I2C_STARTXFR_REG) = READ_CMD;
	for (i=0; i<nbytes; i++) {
		j = 0;
		do {
			if (IS_DATARDY) {
				data[i] = RT2880_REG(RT2880_I2C_DATAIN_REG);
				break;
			}
		} while(++j<max_ee_busy_loop);
	}

	i = 0;
	while(IS_BUSY && i<i2c_busy_loop){
		i++;
	};
	
}

static inline void random_read_block(u32 address, u8 *data)
{
	/* change page */
	if (address_bytes == 1) {
		int page;
		
		page = ((address >> 8) & 0x7) << 1;
		/* device id always 0 */
		RT2880_REG(RT2880_I2C_DEVADDR_REG) = (i2cdrv_addr | (page>>1));
	}

   	/* dummy write */
   	i2c_write(address, data, 0);
	i2c_read(data, READ_BLOCK);	
}

u8 random_read_one_byte(u32 address)
{	
	u8 data;
#ifdef EEPROM_1B_ADDRESS_2KB_SUPPORT
	/* change page */
	if (address_bytes == 1) {
		int page;
		
		page = ((address >> 8) & 0x7) << 1;
		/* device id always 0 */
		RT2880_REG(RT2880_I2C_DEVADDR_REG) = (i2cdrv_addr | (page>>1));
	}
#endif

   	/* dummy write */
	i2c_write(address, &data, 0);
	i2c_read(&data, 1);
	return (data);
}

void i2c_eeprom_read(u32 address, u8 *data, u32 nbytes)
{
	int i;
	int nblock = nbytes / READ_BLOCK;
	int rem = nbytes % READ_BLOCK;

	for (i=0; i<nblock; i++) {
		random_read_block(address+i*READ_BLOCK, &data[i*READ_BLOCK]);
	}

	if (rem) {
		int offset = nblock*READ_BLOCK;
		for (i=0; i<rem; i++) {
			data[offset+i] = random_read_one_byte(address+offset+i);
		}		
	}
}


void i2c_eeprom_read_one(u32 address, u8 *data, u32 nbytes)
{
	int i;

	for (i=0; i<nbytes; i++) {
		data[i] = random_read_one_byte(address+i);
	}
}

static inline void random_write_block(u32 address, u8 *data)
{
	/* change page */
	if (address_bytes == 1) {
		int page;
		
		page = ((address >> 8) & 0x7) << 1;
		/* device id always 0 */
		RT2880_REG(RT2880_I2C_DEVADDR_REG) = (i2cdrv_addr | (page>>1));
	}


	i2c_write(address, data, WRITE_BLOCK);
	udelay(5000);
}

void random_write_one_byte(u32 address, u8 *data)
{	
#ifdef EEPROM_1B_ADDRESS_2KB_SUPPORT
	/* change page */
	if (address_bytes == 1) {
		int page;
		
		page = ((address >> 8) & 0x7) << 1;
		/* device id always 0 */
		RT2880_REG(RT2880_I2C_DEVADDR_REG) = (i2cdrv_addr | (page>>1));
	}
#endif

	i2c_write(address, data, 1);
	udelay(5000);
}

void i2c_eeprom_write(u32 address, u8 *data, u32 nbytes)
{
	int i;
	int nblock = nbytes / WRITE_BLOCK;
	int rem = nbytes % WRITE_BLOCK;

	for (i=0; i<nblock; i++) {
		random_write_block(address+i*WRITE_BLOCK, &data[i*WRITE_BLOCK]);
	}

	if (rem) {
		int offset = nblock*WRITE_BLOCK;

		for (i=0; i<rem; i++) {
			random_write_one_byte(address+offset+i, &data[offset+i]);
		}		
	}
}

void i2c_read_config(char *data, unsigned int len)
{
	i2c_master_init();
	i2c_eeprom_read(0, data, len);
}

void i2c_eeprom_dump(void)
{
	u32 a;
	u8 v;

	i2c_master_init();
	for (a = 0; a < 128; a++) {
		if (a % 16 == 0)
			printk("%4x : ", a);
		v = random_read_one_byte(a);
		printk("%02x ", v);
		if (a % 16 == 15)
			printk("\n");
	}
}
#if defined (CONFIG_FB_MEDIATEK_ILITEK) || defined (CONFIG_FB_MEDIATEK_TRULY)&& defined (CONFIG_RALINK_MT7621)
void i2c_write_touch_cmd(u8 address)
{
  u8 data;//dummy write	
	i2cdrv_addr = 0x48;
	switch_address_bytes(1);
	i2c_master_init();
	i2c_write(address, &data, 0);
}
void i2c_write_touch(u8 address, u8 data)
{
	i2cdrv_addr = 0x48;
	switch_address_bytes(1);
	i2c_master_init();
	i2c_write(address, &data, 1);
}
u8 i2c_read_touch(u8 address)
{
	u8 data;
	//i2cdrv_addr = 0x48;
	//switch_address_bytes(1);
	//i2c_master_init();
	i2c_write(address, &data, 0); //write CR
	i2c_read(&data, 1);
	return (data);
}

u32 i2c_read_touch_channel() 
{
	u32 data;
	//i2cdrv_addr = 0x48;
	//switch_address_bytes(1);
	//i2c_master_init();
	i2c_read(&data, 4);
	//i2c_read(&data, 1);
	return (data);
}
#endif
#if defined (CONFIG_MT7621_FPGA) || defined (CONFIG_MT7628_FPGA)
/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*         	pcie phy board initial					*/
/*   RETURNS: None                                                      */
/*   OUTPUTS: None                                                      */
/*   NOTE(S): transfer 32-bit data to I2C                                */
/*                                                                      */
/*----------------------------------------------------------------------*/
void i2c_pcie_phy_write(u32 address, u8 data, u8 bitmask, u32 bit_pos)
{
	u8 k; 
	
	i2cdrv_addr = 0x70;
	switch_address_bytes(1);
	i2c_master_init();	

	k = random_read_one_byte(address);
	k &= ~bitmask;
	k |= ((data << bit_pos) & bitmask);
	random_write_one_byte(address, &k);

	//i2cdrv_addr = 0x00;
	//switch_address_bytes(2);
}

void chk_phy_pll(void)
{
	u8 k; 
	i2cdrv_addr = 0x70;
	switch_address_bytes(1);
	i2c_master_init();	
	i2c_pcie_phy_write(0xff, 0x04, 0x01, 0);////0xFC[31:24]   0x04      //Change bank address to 0x04
	k = random_read_one_byte(0x30);
	printk("PE1_GLOBAL_RGS_0(bank4, 0x30) = %x\n", k);
	i2c_pcie_phy_write(0xff, 0x00, 0x01, 0);////0xFC[31:24]   0x00      //Change bank address to 0x00
}

void pcie_phy_cali(void)
{
	int i;
	unsigned long value; 
#if defined (CONFIG_RALINK_MT7621)
#define ASSERT_SYSRST_PCIE(val)		do {	\
						if ((RT2880_REG(RALINK_SYSCTL_BASE + 0xc)&0xFFFF) == 0x0101)	\
							RT2880_REG(RALINK_SYSCTL_BASE + 0x34) |= val;	\
						else	\
							RT2880_REG(RALINK_SYSCTL_BASE + 0x34) &= ~val;	\
					} while(0)
#define DEASSERT_SYSRST_PCIE(val) 	do {	\
						if ((RT2880_REG(RALINK_SYSCTL_BASE + 0xc)&0xFFFF) == 0x0101)	\
							RT2880_REG(RALINK_SYSCTL_BASE + 0x34) &= ~val;	\
						else	\
							RT2880_REG(RALINK_SYSCTL_BASE + 0x34) |= val;	\
					} while(0)
#endif

	i2cdrv_addr = 0x00;
	switch_address_bytes(2);
	i2c_master_init();	
#if defined (CONFIG_RALINK_MT7621)
	RT2880_REG(RALINK_SYSCTL_BASE + 0x60) &= ~0x00000c00; 	//PCIe reset GPIO mode bit[11:10]=2'b00
#elif defined (CONFIG_RALINK_MT7628)
	RT2880_REG(RALINK_SYSCTL_BASE + 0x60) &= ~(0x1<<9);	//PCIe reset GPIO mode bit[11:10]=2'b00
#endif
#if defined (CONFIG_RALINK_MT7621)
	//port1&port2 pcie reset assert
	value = 1<<24;
#if defined (CONFIG_PCIE_PORT1)
	value |= 1<<25;
#endif
#if defined (CONFIG_PCIE_PORT2)
	value |= 1<<26;
#endif
	DEASSERT_SYSRST_PCIE(value);
#elif defined (CONFIG_RALINK_MT7628)
	//port0 pcie reset assert
	RT2880_REG(RALINK_SYSCTL_BASE + 0x34) &= ~(0x1<<26);	// port0 enable
#endif
	for (i=0;i<64;i++){
#if 0
		if( i>0x20 && i<0x30){
			printk("skip i = %x\n",i);
			continue;
		} else {
			printk("do cal i = %x\n",i);
		}
#endif
	//printk("before ***************************\n");
	//printk("top register:\n");
	//i2c_eeprom_dump();
	//printk("\nphyip register:\n");
	i2cdrv_addr = 0x70;
	switch_address_bytes(1);
	i2c_master_init();	
	//i2c_eeprom_dump();
	i2cdrv_addr = 0x00;
	switch_address_bytes(2);
	i2c_master_init();	
	printk("********************************\n");
		i2c_eeprom_read(0x40, &value, 4);
		value &= ~0x00007e7e;
		value |= i<<1; 
		value |= i<<1+8; 
		i2c_write(0x40, (u8 *)(&value), 4);
	//printk("after ***************************\n");
	//printk("top register:\n");
	//i2c_eeprom_dump();
	//printk("\nphyip register:\n");
	i2cdrv_addr = 0x70;
	switch_address_bytes(1);
	i2c_master_init();	
	//i2c_eeprom_dump();
	i2cdrv_addr = 0x00;
	switch_address_bytes(2);
	i2c_master_init();	
	printk("********************************\n");
		udelay(100000);
#if defined (CONFIG_RALINK_MT7621)
		ASSERT_SYSRST_PCIE(0x7<<24);
		udelay(1000);
		value = 1<<24;
#if defined (CONFIG_PCIE_PORT1)
		value |= 1<<25;
#endif
#if defined (CONFIG_PCIE_PORT2)
		value |= 1<<26;
#endif
		DEASSERT_SYSRST_PCIE(value);
#if 0
		//chk_phy_pll();
#endif
#elif defined (CONFIG_RALINK_MT7628)
		RT2880_REG(RALINK_SYSCTL_BASE + 0x34) |= 0x1<<26;	//port0 reset assert
		udelay(1000);
		RT2880_REG(RALINK_SYSCTL_BASE + 0x34) &= ~(0x1<<26);	//deassert port0
#endif
		udelay(1000);
		printk("system reset: %x\n", RT2880_REG(RALINK_SYSCTL_BASE + 0x34));
		RT2880_REG(RALINK_PCI_BASE) |= 0x00000002;	//assert PRST 
		udelay(1000);

		printk("%s: %s() %d\n", __FILE__, __FUNCTION__, __LINE__);
		RT2880_REG(RALINK_PCI_BASE) &= ~0x00000002;	//release PRST
		printk("%s: %s() %d\n", __FILE__, __FUNCTION__, __LINE__);
		//i2c_eeprom_read(0x40, &value, 4);
		//printk("TOP register 0x40 = 0x%x\n", value);
		udelay(100000);
		if(RT2880_REG(RALINK_PCI_BASE + 0x2050) & 0x1) {
			printk("pcie0 link up at 0x%x\n", i);
		}else{
			printk("pcie0 no link at 0x%x\n", i);
		}
#if defined (CONFIG_PCIE_PORT1)
		if(RT2880_REG(RALINK_PCI_BASE + 0x3050) & 0x1) {
			printk("pcie1 link up at 0x%x\n", i);
		}else{
			printk("pcie1 no link at 0x%x\n", i);
		}
#endif
#if defined (CONFIG_PCIE_PORT2)
		if(RT2880_REG(RALINK_PCI_BASE + 0x4050) & 0x1) {
			printk("pcie2 link up at 0x%x\n", i);
		}else{
			printk("pcie2 no link at 0x%x\n", i);
		}
#endif
	}
#if 1
		value &= ~0x00007e7e;
		i2c_write(0x40, (u8 *)(&value), 4);
#if defined (CONFIG_RALINK_MT7621)
		ASSERT_SYSRST_PCIE(0x7<<24);
#elif defined (CONFIG_RALINK_MT7628)
		RT2880_REG(RALINK_SYSCTL_BASE + 0x34) |= 0x1<<26;
#endif
		udelay(1000);
#endif
}
void pcie_top_init(void)
{
	unsigned long value;
	
	i2cdrv_addr = 0x00;
	switch_address_bytes(2);
	i2c_master_init();	
	i2c_eeprom_read(0x104, &value, 4);
	printk("TOP register 0x104 = 0x%x\n", value);
	value |= 0x01; //Enable I2C Smith Trigger
	printk("value= %x\n", value);
	i2c_write(0x104,(u8 *)(&value), 4);
	i2c_eeprom_read(0x104, &value, 4);
	printk("TOP register 0x104 = 0x%x\n", value);
#if 1
	i2c_eeprom_read(0x40, &value, 4);
	printk("TOP register 0x40 = 0x%x\n", value);
	value &= ~0x0000007e;
	value |= 3<<1;
	value |= 3<<1+8;
	i2c_write(0x40,(u8 *)(&value), 4);
	i2c_eeprom_read(0x40, &value, 4);
	printk("TOP register 0x40 = 0x%x\n", value);
#endif
	i2c_eeprom_dump();
	printk("TOP register initial done\n");
}

void pcie_phyip_init(void)
{

	i2c_pcie_phy_write(0xff, 0x02, 0xff, 0);////0xFC[31:24]   0x02      //Change bank address to 0x02
	i2c_pcie_phy_write(0x06, 0x0e, 0x3c, 2);////0x04[21:18]   0x0e      //Increase the TX output amplitude
	i2c_pcie_phy_write(0xff, 0x03, 0xff, 0);////0xFC[31:24]   0x03      //Change bank address to 0x03
	i2c_pcie_phy_write(0x06, 0x0e, 0x3c, 2);////0x04[21:18]   0x0e      //Increase the TX output amplitude
	i2c_pcie_phy_write(0xff, 0x04, 0xff, 0);////0xFC[31:24]   0x04      //Change bank address to 0x04
	i2c_pcie_phy_write(0xa0, 0x01, 0x80, 7);////0xA0[07:07]   0x01      //DDS high frequency mode enable
	i2c_pcie_phy_write(0xa8, 0x74, 0xff, 0);////0xA8[11:00]   0x74      //Improve SSC deviation
	i2c_pcie_phy_write(0xa9, 0x00, 0x0f, 0);////0xA8[11:00]   0x74      //Improve SSC deviation
	i2c_pcie_phy_write(0xa9, 0x04, 0xf0, 4);////0xA8[23:12]   0x74      //Improve SSC deviation
	i2c_pcie_phy_write(0xaa, 0x07, 0xff, 0);////0xA8[23:12]   0x74      //Improve SSC deviation
	i2c_pcie_phy_write(0xa2, 0x00, 0x08, 3);////0xA0[19:19]   0x00      //Disable SSC
	i2c_pcie_phy_write(0xa2, 0x01, 0x08, 3);////0xA0[19:19]   0x01      //Enable SSC
	i2c_pcie_phy_write(0xff, 0x02, 0xff, 0);////0xFC[31:24]   0x02      //Change bank address to 0x02
	i2c_pcie_phy_write(0x0a, 0x07, 0x0f, 0);////0x08[19:16]   0x07      //Adjust the Rx sensitivity level for Port 0
	i2c_pcie_phy_write(0x0d, 0x0f, 0xff, 0);////0x0C[15:08]   0x0F      //Adjust the Rx sensitivity level for Port 0
	i2c_pcie_phy_write(0xff, 0x03, 0xff, 0);////0xFC[31:24]   0x03      //Change bank address to 0x03
	i2c_pcie_phy_write(0x0l, 0x07, 0x0f, 0);////0x08[19:16]   0x07      //Adjust the Rx sensitivity level for Port 1
	i2c_pcie_phy_write(0x0d, 0x0f, 0xff, 0);////0x0C[15:08]   0x0F      //Adjust the Rx sensitivity level for Port 1
	i2c_pcie_phy_write(0xff, 0x04, 0xff, 0);////0xFC[31:24]   0x04      //Change bank address to 0x04
	i2c_pcie_phy_write(0xac, 0x08, 0xf0, 4);////0xAC[07:04]   0x08      //Patch PLL Loop bandwidth issue (from 10MHz to 6MHz)
	i2c_pcie_phy_write(0xad, 0x05, 0x0f, 0);////0xAC[11:08]   0x05      //Patch PLL Loop bandwidth issue (from 10MHz to 6MHz)
	i2c_pcie_phy_write(0x02, 0x01, 0x01, 0);////0x00[16:16]   0x01      //Disable Digital probe signal output
	i2c_pcie_phy_write(0xff, 0x01, 0xff, 0);////0xFC[31:24]   0x01      //Change bank address to 0x01
	i2c_pcie_phy_write(0x02, 0x7f, 0x7f, 0);////0x00[22:16]   0x7f      //Disable Digital probe signal output
	i2c_pcie_phy_write(0x03, 0x7f, 0x7f, 0);////0x00[30:24]   0x7f      //Disable Digital probe signal output
	i2c_pcie_phy_write(0xff, 0x00, 0xff, 0);////0xFC[31:24]   0x00      //Change bank address to 0x00
	i2c_pcie_phy_write(0x10, 0x07, 0xf0, 4);////0x10[07:04]   0x07      //retry comma detection threshold value
	i2c_pcie_phy_write(0x10, 0x08, 0x0f, 0);////0x10[03:00]   0x08      //comma detection threshold value
	i2c_pcie_phy_write(0xff, 0x01, 0xff, 0);////0xFC[31:24]   0x01      //Change bank address to 0x01
	i2c_pcie_phy_write(0x10, 0x07, 0xf0, 4);////0x10[07:04]   0x07      //retry comma detection threshold value
	i2c_pcie_phy_write(0x10, 0x08, 0x0f, 0);////0x10[03:00]   0x08      //comma detection threshold value
	i2c_pcie_phy_write(0xff, 0x00, 0xff, 0);////0xFC[31:24]   0x00      //Change bank address to 0x00

	i2c_eeprom_dump();
	printk("PhyIP initial done\n");
}
void pcie_phy_init(void)
{
#if 0
		chk_phy_pll();
#endif
	pcie_top_init();
	pcie_phyip_init();
	//pcie_phy_cali();
#if 0
		chk_phy_pll();
#endif
}
#endif

#if defined(CONFIG_MTK_NFC_SUPPORT)

#define MAX_BLOCK       64
#define MAX_LOOP        1000
#define GPIO_INPUT      0
#define GPIO_OUTPUT     1


#if defined (CONFIG_RALINK_RT3883) 
void MT6605_gpio_init(u8 num, u8 dir)
{
        int reg = 0;
        if ((num >= 25) && (num <= 31))
        {
                reg = RT2880_REG(RALINK_SYSCTL_BASE+0x14);
                reg &= ~(0x3<<1); // GPIO 2, 3~7 as GPIO mode
                RT2880_REG(RALINK_SYSCTL_BASE+0x14) = reg;

                //set direction
                reg = RT2880_REG(RALINK_PIO_BASE+0x4c);
                if (dir == GPIO_INPUT)
                        reg &= ~(1<<(num-24)); // input
                else
                        reg |= (1<<(num-24)); // output
                RT2880_REG(RALINK_PIO_BASE+0x4c) = reg;
        }
        else if ((num >= 7) && (num <= 14))
        {
                reg = RT2880_REG(RALINK_SYSCTL_BASE+0x60);
                reg |= (0x7<<2); // UARTF as GPIO mode
                RT2880_REG(RALINK_SYSCTL_BASE+0x60) = reg;

                //set direction = input
                reg = RT2880_REG(RALINK_PIO_BASE+0x24);
                if (dir == GPIO_INPUT)
                        reg &= ~(1<<num); // input
                else
                        reg |= (1<<num); // output
                RT2880_REG(RALINK_PIO_BASE+0x24) = reg;
        }
        else
                printk("GPIO num not support\n");
}

int MT6605_gpio_read(u8 num)
{
        int reg = 0;

        if ((num >= 25) && (num <= 31))
        {
                reg = RT2880_REG(0x48+RALINK_PIO_BASE) & (1<<(num-24));

        }
        else if ((num >= 7) && (num <= 14))
        {
                reg = RT2880_REG(0x20+RALINK_PIO_BASE) & (1<<(num));
        }
        else
                printk("GPIO num not support\n");

        return reg;
}

void MT6605_gpio_write(u8 num, u8 val)
{
        int reg = 0;
        if ((num >= 25) && (num <= 31))
        {
                reg = RT2880_REG(0x48+RALINK_PIO_BASE);
                if (val)
                        reg |= (1<<(num-24));
                else
                        reg &= ~(1<<(num-24));
                RT2880_REG(0x48+RALINK_PIO_BASE) = reg;
        }
        else if ((num >= 7) && (num <= 14))
        {
                reg = RT2880_REG(0x20+RALINK_PIO_BASE);
                if (val)
                        reg |= (1<<(num));
                else
                        reg &= ~(1<<(num));
                RT2880_REG(0x20+RALINK_PIO_BASE) = reg;
        }
        else
                printk("GPIO num not support\n");
}
#elif defined(CONFIG_RALINK_MT7621)
void MT6605_gpio_init(u8 num, u8 dir)
{
	int reg = 0;

	reg = RT2880_REG(RALINK_SYSCTL_BASE+0x60);
	reg &= ~(0xf << 3);
	reg |= (0x5 << 3); // UART2 and UART3
	RT2880_REG(RALINK_SYSCTL_BASE+0x60) = reg;

	//set direction = input
	reg = RT2880_REG(RALINK_REG_PIODIR);
	if (dir == GPIO_INPUT)
		reg &= ~(1<<num); // input
	else
		reg |= (1<<num); // output
	RT2880_REG(RALINK_REG_PIODIR) = reg;
}

int MT6605_gpio_read(u8 num)
{
	return RT2880_REG(RALINK_REG_PIODATA) & (1<<(num));
}

void MT6605_gpio_write(u8 num, u8 val)
{
	int reg;

	reg = RT2880_REG(RALINK_REG_PIODATA);
	if (val)
		reg |= (1<<(num));
	else
		reg &= ~(1<<(num));
	RT2880_REG(RALINK_REG_PIODATA) = reg;
 
}
#else
#error "Chip not Support NFC"
#endif

EXPORT_SYMBOL(MT6605_gpio_read);
EXPORT_SYMBOL(MT6605_gpio_init);
EXPORT_SYMBOL(MT6605_gpio_write);

void i2c_read_MT6605(char *data, unsigned int len)
{
        int i, j;
        unsigned long old_addr = i2cdrv_addr;
        int nblock = len / MAX_BLOCK;
        int rem = len % MAX_BLOCK;
        int reg;

        for (j = 0; j < MAX_LOOP; j++)
        {
                reg = MT6605_gpio_read(MT6605_GPIO_IND); // RT2880_REG(0x48+RALINK_PIO_BASE) & (1<<2);
                if (reg)
                        break;
                udelay(1);
        }

        if (j >= MAX_LOOP)
        {
                printk("GPIO Read Timeout\n");
                return;
        }


        i2cdrv_addr = (0x28 >> 0);
        i2c_master_init();

        for (i=0; i<len; i++) {
                if ((i & (MAX_BLOCK-1)) == 0)
                {
                        if (nblock > 0)
                        {
                                RT2880_REG(RT2880_I2C_BYTECNT_REG) = MAX_BLOCK - 1;
                                nblock--;
                        }
                        else
                                RT2880_REG(RT2880_I2C_BYTECNT_REG) = rem - 1;

                        RT2880_REG(RT2880_I2C_STARTXFR_REG) = READ_CMD;
                }

                j = 0;
                do {
                        if (IS_DATARDY) {
                                data[i] = RT2880_REG(RT2880_I2C_DATAIN_REG);
                                break;
                        }
                } while(++j<max_ee_busy_loop);
        }

        i = 0;
        while(IS_BUSY && i<i2c_busy_loop){
                i++;
        };

        i2cdrv_addr = old_addr;

}

void i2c_write_MT6605(char *data, unsigned int len)
{
        int i, j;
        unsigned long old_addr = i2cdrv_addr;
        int nblock = len / MAX_BLOCK;
        int rem = len % MAX_BLOCK;
        int reg;

        for (j = 0; j < MAX_LOOP; j++)
        {
                reg = MT6605_gpio_read(MT6605_GPIO_IND); // RT2880_REG(0x48+RALINK_PIO_BASE) & (1<<2);
                if (!reg)
                        break;
                udelay(1);
        }

        if (j >= MAX_LOOP)
        {
                printk("GPIO Write Timeout\n");
                return;
        }


        i2cdrv_addr = (0x28 >> 0);
        i2c_master_init();


        for (i=0; i<len; i++) {

                if ((i & (MAX_BLOCK-1)) == 0)
                {

                        j = 0;
                        while(IS_BUSY && j<i2c_busy_loop){
                                j++;
                        };


                        if (nblock > 0)
                        {
                                RT2880_REG(RT2880_I2C_BYTECNT_REG) = MAX_BLOCK - 1;
                                nblock--;
                        }
                        else
                        {
                                RT2880_REG(RT2880_I2C_BYTECNT_REG) = rem - 1;
                        }
                }

                j = 0;
                do {
                        if (IS_SDOEMPTY) {
                                RT2880_REG(RT2880_I2C_DATAOUT_REG) = data[i];
                                break;
                        }
                } while (++j<max_ee_busy_loop);

                if ((i & (MAX_BLOCK-1)) == 0)
                {
                        RT2880_REG(RT2880_I2C_STARTXFR_REG) = WRITE_CMD;
                }

        }

        i = 0;
        while(IS_BUSY && i<i2c_busy_loop){
                i++;
        };

        i2cdrv_addr = old_addr;
}

EXPORT_SYMBOL(i2c_read_MT6605);
EXPORT_SYMBOL(i2c_write_MT6605);

#endif

static int i2c_auth_cp_handler(u32 reg, u8* buf, u32 len, u32 bWOP)
{
	u32 ret = len;
	char *i2cbuf;
	u32 old_addr = i2cdrv_addr;
	u32 old_addr_bytes = address_bytes;
	i2cdrv_addr = MFI_AUTHIC_ADDR;
	switch_address_bytes(1);
	i2c_master_init();

	i2cbuf = kmalloc(len, GFP_KERNEL);	
	memset(i2cbuf, 0x0, len);
	
	udelay(10000);
	
	if (bWOP)
	{
			ret = copy_from_user(i2cbuf, buf ,len);
			if (ret)
				printk("i2c_auth_cp_handler copy_from_user ret=%d\n",ret);
			
			i2c_write(reg, buf, len);		
			udelay(5000);
	}
	else
	{
			int rlen, totallen;

			totallen = len;
			ret = 0;
			do 
			{
				if (totallen > 64)
					rlen = 64;
				else
					rlen = totallen;	
				i2c_read(i2cbuf+ret, rlen);
				udelay(5000);
				ret+=rlen;
				totallen-=rlen;
			}while(totallen >0);	
			
			if (ret<len)
			{
					printk("!!!!!! i2c_auth_cp_read read %d (need %d)!!!!!!!\n",ret,len);
					printk("I2CSTATUS=%08X\n",RT2880_REG(RT2880_I2C_STATUS_REG));
			}		
			copy_to_user(buf, i2cbuf, len);
	}
	kfree(i2cbuf);
	i2cdrv_addr = old_addr;
	switch_address_bytes(old_addr_bytes);
	return ret;
}	

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
long i2cdrv_ioctl(struct file *filp, unsigned int cmd, 
		unsigned long arg)
#else
int i2cdrv_ioctl(struct inode *inode, struct file *filp, \
                     unsigned int cmd, unsigned long arg)
#endif
{
	//unsigned char w_byte[4];
	unsigned int address, size;
	u8 value, *tmp;
	I2C_WRITE *s_i2c_write;
#if defined (CONFIG_FB_MEDIATEK_ILITEK) || defined (CONFIG_FB_MEDIATEK_TRULY)&& defined (CONFIG_RALINK_MT7621)
    u32 value32;
  #endif
	switch (cmd) {
	case RT2880_PCIE_PHY_READ:
		value = 0; address = 0;
		address = (unsigned int)arg;
		i2c_master_init();
		value = random_read_one_byte(address);
		printk("0x%04x : 0x%x\n", address, (unsigned char)value);
		break;
	case RT2880_PCIE_PHY_WRITE:
		s_i2c_write = (I2C_WRITE*)arg;
		address = s_i2c_write->address;
		value   = s_i2c_write->value;
		size    = s_i2c_write->size;
		tmp = &value;
		i2c_master_init();
		//random_write_one_byte(address, &value);
		i2c_write((unsigned long)address, tmp, 4);
		break;
	case RT2880_I2C_DUMP:
		i2c_eeprom_dump();
		break;
	case RT2880_I2C_READ:
		value = 0; address = 0;
		address = (unsigned int)arg;
#if defined(CONFIG_MTK_NFC_MT6605_SIM)
		// to test NFC, build i2ccmd, use the following command to test
		// i2ccmd read 4444
		// i2ccmd read 3333
		// i2ccmd read 2222
		if (address == 0x2222)
		{
			mt6605_sim();
		}
		else if (address == 0x3333)
		{
                        MT6605_gpio_write(MT6605_GPIO_VEN, 0);
                        MT6605_gpio_write(MT6605_GPIO_RESET, 1);
		}
		else if (address == 0x4444)
		{
                        MT6605_gpio_write(MT6605_GPIO_RESET, 0);
                        MT6605_gpio_write(MT6605_GPIO_VEN, 1);
		}
#else
		i2c_master_init();
		i2c_eeprom_read(address, (unsigned char*)&value, 4);
		printk("0x%04x : 0x%08x\n", address, (unsigned int)value);
#endif
		break;
	case RT2880_I2C_WRITE:
		s_i2c_write = (I2C_WRITE*)arg;
		address = s_i2c_write->address;
		value   = s_i2c_write->value;
		size    = s_i2c_write->size;
		i2c_master_init();
		i2c_eeprom_write(address, (unsigned char*)&value, size);
#if 0
		memcpy(w_byte, (unsigned char*)&value, 4);
		if ( size == 4) {
			i2c_eeprom_write(address, w_byte[0], 1);
			i2c_eeprom_write(address+1, w_byte[1], 1 );
			i2c_eeprom_write(address+2, w_byte[2], 1 );
			i2c_eeprom_write(address+3, w_byte[3], 1 );
		} else if (size == 2) {
			i2c_eeprom_write(address, w_byte[0], 1);
			i2c_eeprom_write(address+1, w_byte[1], 1 );
		} else if (size == 1) {
			i2c_eeprom_write(address, w_byte[0], 1);
		} else {
			printk("i2c_drv -- size error, %d\n", size);
			return 0;
		}
#endif
		break;
	case RT2880_I2C_SET_ADDR:
		i2cdrv_addr = (unsigned long)arg;
		break;
	case RT2880_I2C_SET_ADDR_BYTES:
		value = switch_address_bytes( (unsigned long)arg);
		printk("i2c addr bytes = %x\n", value);
		break;
	case RT2880_I2C_SET_CLKDIV:
		clkdiv_value  =  40*1000/(unsigned long)arg;
		printk("i2c clkdiv = %d\n",clkdiv_value);
		break;
	case RT2880_I2C_AUTHCP_CMD:
		{
			u32 param[4];
			__get_user(param[0], (int __user *)(long*)arg);
			__get_user(param[1], (int __user *)(long*)arg+1);
			__get_user(param[2], (int __user *)(long*)arg+2);
			__get_user(param[3], (int __user *)(long*)arg+3);

    	if ((int)param[0] < 0)
    	{
#if 1
					//Should redefined based on MFi Auth IC pin    		
    		  printk("===RST AUTH IC ===\n"); 
    		  RT2880_REG(RALINK_SYSCTL_BASE + 0x60)&= ~0x7<<2;
					RT2880_REG(RALINK_SYSCTL_BASE + 0x60)|= 0x6<<2;
    			RT2880_REG(RALINK_SYSCTL_BASE + 0x60)|= RALINK_GPIOMODE_I2C;
    			RT2880_REG(0xB0000620)|= (0x3<<1); //Set to HIGH
    			RT2880_REG(0xB0000624)|= (0x3<<1);
    			RT2880_REG(0xB0000620)|= (1<<11);
    			RT2880_REG(0xB0000624)|= (1<<11);
    			udelay(10000);
    			RT2880_REG(RALINK_SYSCTL_BASE + 0x60)&= ~(7<<2);
    			RT2880_REG(RALINK_SYSCTL_BASE + 0x60)|= (6<<2);
    			RT2880_REG(0xB0000620)&= ~(1<<11);
    			RT2880_REG(0xB0000624)|= (1<<11);
    			udelay(11000*2);
    			//RT2880_REG(0xB0000620)|= (1<<11);	
    			RT2880_REG(RALINK_SYSCTL_BASE + 0x60)&= ~RALINK_GPIOMODE_I2C;
#endif    			
    	}
    	else
    	{
    			RT2880_REG(RALINK_SYSCTL_BASE + 0x60) &= ~RALINK_GPIOMODE_I2C;
					param[3] = i2c_auth_cp_handler(param[0],(char *)param[1],param[2],param[3]);
					__put_user(param[3], (int __user *)(long*)arg+3);
			}
		}
		break;	
		#if defined (CONFIG_FB_MEDIATEK_ILITEK) || defined (CONFIG_FB_MEDIATEK_TRULY)&& defined (CONFIG_RALINK_MT7621)
	case Touch_Router_Read:
		value = 0; address = 0;
		address = (unsigned int)arg;
		value = i2c_read_touch(address);
		value_user = value;
		
		//printk("0x%0x : 0x%x\n", address, (unsigned char)value);
		break;

	case Touch_Router_Write:
		s_i2c_write = (I2C_WRITE*)arg;
		address = s_i2c_write->address;
		value   = s_i2c_write->value;
		i2c_write_touch(address, value);
		break;
	case Touch_Router_Read_Channel:
		value32 = 0;
		value32 = i2c_read_touch_channel();
		value_user=value32;
		mdelay(20);
		break;
	case Touch_Router_Write_CMD:
		s_i2c_write = (I2C_WRITE*)arg;
		address = s_i2c_write->address;
		i2c_write_touch_cmd(address);
		break;
	case Touch_Router_Get:
		put_user(value_user, (int __user *)arg);
		break;	
#endif
	default :
		printk("i2c_drv: command format error\n");
	}

	return 0;
}

struct file_operations i2cdrv_fops = {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	unlocked_ioctl: i2cdrv_ioctl,
#else
	ioctl:  i2cdrv_ioctl,
#endif
};

int i2cdrv_init(void)
{
#if !defined (CONFIG_DEVFS_FS)
	int result=0;
#endif

	/* configure i2c to normal mode */
	RT2880_REG(RALINK_SYSCTL_BASE + 0x60) &= ~RALINK_GPIOMODE_I2C;

#ifdef  CONFIG_DEVFS_FS
	if(devfs_register_chrdev(i2cdrv_major, I2C_DEV_NAME , &i2cdrv_fops)) {
		printk(KERN_WARNING " i2cdrv: can't create device node\n");
		return -EIO;
	}

	devfs_handle = devfs_register(NULL, I2C_DEV_NAME, DEVFS_FL_DEFAULT, i2cdrv_major, 0, \
			S_IFCHR | S_IRUGO | S_IWUGO, &i2cdrv_fops, NULL);
#else
	result = register_chrdev(i2cdrv_major, I2C_DEV_NAME, &i2cdrv_fops);
	if (result < 0) {
		printk(KERN_WARNING "i2c_drv: can't get major %d\n",i2cdrv_major);
		return result;
	}

	if (i2cdrv_major == 0) {
		i2cdrv_major = result; /* dynamic */
	}
#endif

#if defined(CONFIG_MTK_NFC_SUPPORT)

        MT6605_gpio_init(MT6605_GPIO_IND, GPIO_INPUT); // IND
        MT6605_gpio_init(MT6605_GPIO_VEN, GPIO_OUTPUT); // VEN
        MT6605_gpio_init(MT6605_GPIO_RESET, GPIO_OUTPUT); // Reset

        MT6605_gpio_write(MT6605_GPIO_RESET, 0);
        MT6605_gpio_write(MT6605_GPIO_VEN, 1);
#endif



	printk("i2cdrv_major = %d\n", i2cdrv_major);
	return 0;
}


static void i2cdrv_exit(void)
{
	printk("i2c_drv exit\n");

#ifdef  CONFIG_DEVFS_FS
	devfs_unregister_chrdev(i2cdrv_major, I2C_DEV_NAME);
	devfs_unregister(devfs_handle);
#else
	unregister_chrdev(i2cdrv_major, I2C_DEV_NAME);
#endif

}

#if defined (CONFIG_RALINK_I2S) || defined (CONFIG_RALINK_I2S_MODULE)
EXPORT_SYMBOL(i2c_WM8751_write);
#endif

#if defined (CONFIG_MT7621_FPGA) || defined (CONFIG_MT7628_FPGA)
EXPORT_SYMBOL(pcie_phy_init);
EXPORT_SYMBOL(chk_phy_pll);
#endif

module_init(i2cdrv_init);
module_exit(i2cdrv_exit);

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,12)
MODULE_PARM (i2cdrv_major, "i");
#else
module_param (i2cdrv_major, int, 0);
#endif

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Ralink I2C Driver");

