/******************************************************************************
* Filename : gpio.c
* This part is used to control LED and detect button-press
* 
******************************************************************************/

#include <common.h>
#include "../autoconf.h"
#include <configs/rt2880.h>
#include <rt_mmap.h>
#include <gpio.h>

#define ARRAY_SIZE(x)		(sizeof(x) / sizeof((x)[0]))
#if defined (MT7620_MP)
#if defined(ASUS_RTN14U) || defined(ASUS_RTAC52U) || defined(ASUS_RTAC51U) || defined(ASUS_RTAC51UP) || defined(ASUS_RTN11P) || defined(ASUS_RTN54U)
 //asm/rt2880/rt_mmap.h
#define SYSCTR_ADDR	0xB0000000  //RALINK_SYSCTL_BASE
#define IRQ_ADDR 	0xB0000200  //RALINK_INTCL_BASE   
#define PRGIO_ADDR 	0xB0000600  //RALINK_PIO_BASE

//MT7620
#define RALINK_GPIOMODE_I2C             (1U << 0)	/* GPIO #1, #2 */
#define RALINK_GPIOMODE_UARTF		(7U << 2)	/* GPIO #7~#14 */
#define RALINK_GPIOMODE_UARTL		(1U << 5)	/* GPIO#15,#16 */
#define RALINK_GPIOMODE_MDIO		(3U << 7)	/* GPIO#22,#23 */
#define RALINK_GPIOMODE_RGMII1		(1U << 9)	/* GPIO#24~#35 */
#define RALINK_GPIOMODE_RGMII2		(1U << 10)	/* GPIO#60~#71 */
#define RALINK_GPIOMODE_SPI		(1U << 11)	/* GPIO#3~#6 */
#define RALINK_GPIOMODE_SPI_REFCLK	(1U << 12)	/* GPIO#37 */
#define RALINK_GPIOMODE_WLED		(1U << 13)	/* GPIO#72 */
#define RALINK_GPIOMODE_JTAG            (1U << 15)	/* GPIO#40~#44 */
#define RALINK_GPIOMODE_PERST		(3U << 16)	/* GPIO#36 */
#define RALINK_GPIOMODE_NAND_SD		(3U << 18)	/* GPIO#45~#59 */
#define RALINK_GPIOMODE_PA		(1U << 20)	/* GPIO#18~#21 */
#define RALINK_GPIOMODE_WDT		(3U << 21)	/* GPIO#17 */
#define RALINK_GPIOMODE_SUTIF		(3U << 30)	/* FIXME */
#else
#error Invalid Product!!
#endif   //#if defined(ASUS_RTN14U)  ...


#elif defined (MT7621_MP)
#if defined (ASUS_RPAC56) || defined(ASUS_RTAC1200GA1) || defined(ASUS_RTAC1200GU) || defined (ASUS_RPAC87) || defined (ASUS_RTAC85U) || defined (ASUS_RTAC85P) || defined (ASUS_RTN800HP) || defined (ASUS_RTACRH26)
//MT7621
//asm/rt2880/rt_mmap.h
#define SYSCTR_ADDR	0xBE000000   //RALINK_SYSCTL_BASE
#define IRQ_ADDR 	0xBE000200   //RALINK_INTCL_BASE   
#define PRGIO_ADDR 	0xBE000600   //RALINK_PIO_BASE

#define RALINK_GPIOMODE_UART1		0x02
#define RALINK_GPIOMODE_I2C		0x04
#define RALINK_GPIOMODE_UART3		0x08
#define RALINK_GPIOMODE_UART2		0x20
#define RALINK_GPIOMODE_JTAG		0x80
#define RALINK_GPIOMODE_WDT		0x100
#define RALINK_GPIOMODE_PERST		0x400
#define RALINK_GPIOMODE_MDIO		0x1000
#define RALINK_GPIOMODE_GE1		0x4000
#define RALINK_GPIOMODE_GE2		0x8000
#define RALINK_GPIOMODE_SPI		0x10000
#define RALINK_GPIOMODE_SDXC		0x40000
#define RALINK_GPIOMODE_ESWINT		0x100000
#else 
#error Invalid Product!!
#endif  //#if definedd(ASUS_RPAC56)


#elif defined (MT7628_MP)
#if defined(ASUS_RTAC1200)
//asm/rt2880/rt_mmap.h
#define SYSCTR_ADDR	0xB0000000  
#define IRQ_ADDR 	0xB0000200
#define PRGIO_ADDR 	0xB0000600

//MT7628
#define RALINK_GPIOMODE_GPIO		0x1
#define RALINK_GPIOMODE_SPI_SLAVE	0x4
#define RALINK_GPIOMODE_SPI_CS1		0x10
#define RALINK_GPIOMODE_I2S		0x40
#define RALINK_GPIOMODE_UART1		0x100
#define RALINK_GPIOMODE_SDXC		0x400
#define RALINK_GPIOMODE_SPI		0x1000
#define RALINK_GPIOMODE_WDT		0x4000
#define RALINK_GPIOMODE_PERST		0x10000
#define RALINK_GPIOMODE_REFCLK	0x40000
#define RALINK_GPIOMODE_I2C		0x100000
#define RALINK_GPIOMODE_EPHY		0x40000
#define RALINK_GPIOMODE_P0LED		0x100000
#define RALINK_GPIOMODE_WLED		0x400000
#define RALINK_GPIOMODE_UART2		0x1000000
#define RALINK_GPIOMODE_UART3		0x4000000
#define RALINK_GPIOMODE_PWM0		0x10000000
#define RALINK_GPIOMODE_PWM1		0x40000000
#else
#error Invalid Product!!
#endif //#if defined(ASUS_RTAC1200)

#else
#error Invalid Product!!
#endif //#if defined (MT7620_MP)

#define ra_inl(offset)		(*(volatile unsigned long *)(offset))
#define ra_outl(offset,val)	(*(volatile unsigned long *)(offset) = val)
#define ra_and(addr, value) ra_outl(addr, (ra_inl(addr) & (value)))
#define ra_or(addr, value) ra_outl(addr, (ra_inl(addr) | (value)))



//#define SYSCFG_OFFSET		0x10
//#define UARTF_PCM_MODE_SHIFT	6
/*
6 R/W UARTF_PCM_MODE
0: Set 4 of UART-Full pins as normal UART function
1: Set 4 of UART-Full pins as normal PCM function
1¡¯b0
*/
//#define GPIOMODE_OFFSET		0x60
//#define UARTF_GPIO_MODE_SHIFT	1

/*
1 R/W UARTF_GPIO_MODE
0:Normal Mode
1:GPIO Mode
Control GPIO[6:3]
1¡¯b1
*/

#if 0
void uart_pcm_mode(unsigned x)
{
	ulong v = le32_to_cpu(*((volatile u_long *)(SYSCTR_ADDR + SYSCFG_OFFSET)));
   
	if (0 == x) {
		v &= ~(1 << UARTF_PCM_MODE_SHIFT);
		*((volatile u_long *)(SYSCTR_ADDR + SYSCFG_OFFSET)) = cpu_to_le32(v);
	}
	else if(1 == x) {
		v |= (1 << UARTF_PCM_MODE_SHIFT);
		*((volatile u_long *)(SYSCTR_ADDR + SYSCFG_OFFSET)) = cpu_to_le32(v);
	}

}


void uart_gpio_mode(unsigned x)
{
	ulong v = le32_to_cpu(*(volatile u_long *)(SYSCTR_ADDR + GPIOMODE_OFFSET));

	if (0 == x) {
		v &= ~(1 << UARTF_GPIO_MODE_SHIFT);
		*(volatile u_long *)(SYSCTR_ADDR + GPIOMODE_OFFSET) = cpu_to_le32(v);
	}
	else if(1 == x) {
		v |= (1 << UARTF_GPIO_MODE_SHIFT);
		*(volatile u_long *)(SYSCTR_ADDR + GPIOMODE_OFFSET) = cpu_to_le32(v);
	}
}
#endif

#if defined(MT7620_MP) || defined (MT7621_MP) || defined (MT7628_MP)
const static struct gpio_reg_offset_s {
	unsigned short min_nr, max_nr;
	unsigned short int_offset;
	unsigned short edge_offset;
	unsigned short rmask_offset;
	unsigned short fmask_offset;
	unsigned short data_offset;
	unsigned short dir_offset;
	unsigned short pol_offset;
	unsigned short set_offset;
	unsigned short reset_offset;
#if defined(MT7620_MP)	
	unsigned short tog_offset;
#endif	
} s_gpio_reg_offset[] = {
#if defined(MT7620_MP)   
	{  0, 23,  0x0,  0x4,  0x8,  0xc, 0x20, 0x24, 0x28, 0x2c, 0x30, 0x34 },
	{ 24, 39, 0x38, 0x3c, 0x40, 0x44, 0x48, 0x4c, 0x50, 0x54, 0x58, 0x5c },
	{ 40, 71, 0x60, 0x64, 0x68, 0x6c, 0x70, 0x74, 0x78, 0x7c, 0x80, 0x84 },
	{ 72, 72, 0x88, 0x8c, 0x90, 0x94, 0x98, 0x9c, 0xa0, 0xa4, 0xa8, 0xac },
#endif
#if defined(MT7621_MP) 
	{  0, 31, 0x90, 0xA0, 0x50, 0x60, 0x20, 0x0, 0x10, 0x30, 0x40},
	{ 32, 63, 0x94, 0xA4, 0x54, 0x64, 0x24, 0x4, 0x14, 0x34, 0x44},
	{ 64, 95, 0x98, 0xA8, 0x58, 0x68, 0x28, 0x8, 0x18, 0x38, 0x48},
#endif	
#if defined(MT7628_MP) 
	{  0, 31, 0x90, 0xA0, 0x50, 0x60, 0x20, 0x0, 0x10, 0x30, 0x40},
	{ 32, 63, 0x94, 0xA4, 0x54, 0x64, 0x24, 0x4, 0x14, 0x34, 0x44},
	{ 64, 95, 0x98, 0xA8, 0x58, 0x68, 0x28, 0x8, 0x18, 0x38, 0x48},
#endif	
};
#endif  //#if defined(MT7620_MP) || defined (MT7621_MP) || defined (MT7628_MP)

static int is_valid_gpio_nr(unsigned short gpio_nr)
{
#if defined(MT7620_MP)   
	return (gpio_nr > 72)? 0:1;
#else
	return (gpio_nr > 95)? 0:1;
#endif	
}

/* Query GPIO number belongs which item.
 * @gpio_nr:	GPIO number
 * @return:
 * 	NULL:	Invalid parameter.
 *  otherwise:	Pointer to a gpio_reg_offset_s instance.
 */
static const struct gpio_reg_offset_s *get_gpio_reg_item(unsigned short gpio_nr)
{
	int i;
	const struct gpio_reg_offset_s *p = &s_gpio_reg_offset[0], *ret = NULL;

	if (!is_valid_gpio_nr(gpio_nr))
		return ret;

	for (i = 0; !ret && i < ARRAY_SIZE(s_gpio_reg_offset); ++i, ++p) {
		if (gpio_nr < p->min_nr || gpio_nr > p->max_nr)
			continue;

		ret = p;
	}

	return ret;
}

/* Return bit-shift of a GPIO.
 * @gpio_nr:	GPIO number
 * @return:
 * 	0~31:	bit-shift of a GPIO pin in a register.
 *  	-1:	Invalid parameter.
 */
static int get_gpio_reg_bit_shift(unsigned short gpio_nr)
{
	const struct gpio_reg_offset_s *p;

	if (!(p = get_gpio_reg_item(gpio_nr)))
		return -1;

	return gpio_nr - p->min_nr;
}

/* Return specific GPIO register in accordance with GPIO number
 * @gpio_nr:	GPIO number
 * @return
 * 	0:	invalid parameter
 *  otherwise:	address of GPIO register
 */

#if defined(MT7620_MP)   
unsigned int mtk7620_get_gpio_reg_addr(unsigned short gpio_nr, enum gpio_reg_id id)
#elif defined(MT7621_MP) 
unsigned int mtk7621_get_gpio_reg_addr(unsigned short gpio_nr, enum gpio_reg_id id)
#else
unsigned int get_gpio_reg_addr(unsigned short gpio_nr, enum gpio_reg_id id)
#endif
{
	int ret = 0;
	const struct gpio_reg_offset_s *p;

	if (!(p = get_gpio_reg_item(gpio_nr)) || id < 0 || id >= GPIO_MAX_REG)
		return ret;

	switch (id) {
	case GPIO_INT:
		ret = p->int_offset;
		break;
	case GPIO_EDGE:
		ret = p->edge_offset;
		break;
	case GPIO_RMASK:
		ret = p->rmask_offset;
		break;
	case GPIO_MASK:
		ret = p->fmask_offset;
		break;
	case GPIO_DATA:
		ret = p->data_offset;
		break;
	case GPIO_DIR:
		ret = p->dir_offset;
		break;
	case GPIO_POL:
		ret = p->pol_offset;
		break;
	case GPIO_SET:
		ret = p->set_offset;
		break;
	case GPIO_RESET:
		ret = p->reset_offset;
		break;
#if defined(MT7620_MP)   
	case GPIO_TOG:
		ret = p->tog_offset;
		break;
#endif		
	default:
		return 0;
	}
	ret += PRGIO_ADDR;

	return ret;
}

/* Set GPIO pin direction.
 * If a GPIO pin is multi-function pin, it would be configured as GPIO mode.
 * @gpio_nr:	GPIO number
 * @gpio_dir:	GPIO direction
 * 	0: 	output
 * 	1:	input
 *  otherwise:	input
 * @return:
 * 	0:	Success
 * 	-1:	Invalid parameter
 */
#if defined(MT7620_MP)   
int mtk7620_set_gpio_dir(unsigned short gpio_nr, unsigned short gpio_dir)
#elif defined(MT7621_MP)   
int mtk7621_set_gpio_dir(unsigned short gpio_nr, unsigned short gpio_dir)
#else   
int set_gpio_dir(unsigned short gpio_nr, unsigned short gpio_dir)
#endif   
{
	int shift;
	unsigned int mask, val;
	unsigned int reg;

	if (!is_valid_gpio_nr(gpio_nr))
		return -1;
#if defined(MT7620_MP)   
	reg = mtk7620_get_gpio_reg_addr(gpio_nr, GPIO_DIR);
#elif defined(MT7621_MP)
	reg = mtk7621_get_gpio_reg_addr(gpio_nr, GPIO_DIR);
#else
	reg = get_gpio_reg_addr(gpio_nr, GPIO_DIR);
#endif	
	shift = get_gpio_reg_bit_shift(gpio_nr);

	if (!gpio_dir) {
		/* output */
		ra_or(reg, 1U << shift);
	} else {
		/* input */
		ra_and(reg, ~(1U << shift));
	}

	/* Handle special GPIO pin */
	shift = -1;
	mask = val = 1;
#if defined(MT7620_MP)   
	if (gpio_nr >= 1 && gpio_nr <= 2) {
		/* I2C */
		shift = 0;
	} else if (gpio_nr >=  7 && gpio_nr <= 14) {
		/* UARTF */
		shift = 2;
		mask = 7;
		val = 7;
	} else if (gpio_nr >= 15 && gpio_nr <= 16) {
		/* UARTL */
		shift = 5;
	} else if (gpio_nr >= 22 && gpio_nr <= 23) {
		/* MDIO */
		shift = 7;
		mask = 3;
		val = 3;
	} else if (gpio_nr >= 24 && gpio_nr <= 35) {
		/* RGMII1 */
		shift = 9;
	} else if (gpio_nr >= 60 && gpio_nr <= 71) {
		/* RGMII2 */
		shift = 10;
	} else if (gpio_nr >= 3 && gpio_nr <= 6) {
		/* SPI */
		shift = 11;
	} else if (gpio_nr == 37) {
		/* SPI_REFCLK */
		shift = 12;
	} else if (gpio_nr == 72) {
		/* WLED */
		shift = 13;
	} else if (gpio_nr >= 40 && gpio_nr <= 44) {
		/* JTAG/EPHY_LED */
		shift = 15;
	} else if (gpio_nr == 36) {
		/* PERST */
		shift = 16;
		mask = 3;
		val = 3;
	} else if (gpio_nr >= 45 && gpio_nr <= 59) {
		/* NAND/SD_BT */
		shift = 18;
		mask = 3;
		val = 3;
	} else if (gpio_nr >= 18 && gpio_nr <= 21) {
		/* PA */
		shift = 20;
	} else if (gpio_nr == 17) {
		/* WDT */
		shift = 21;
		mask = 3;
		val = 3;
	}
#elif defined (MT7621_MP) //MT7621 
	if (gpio_nr >=  1 && gpio_nr <= 2) {
		/* UART1 */
		shift = 1;
	} else if (gpio_nr >= 3 && gpio_nr <= 4) {
		/* I2C */
		shift = 2;
	} else if (gpio_nr >= 5 && gpio_nr <= 8) {
		/* UART3 */
		shift = 3;
	} else if (gpio_nr >= 9 && gpio_nr <= 12) {
		/* UART2 */
		shift = 5;
	} else if (gpio_nr >= 13 && gpio_nr <= 17) {
		/* JTAG */
		shift = 7;
	} else if (gpio_nr == 18) {
		/* WDT */
		shift = 8;
	} else if (gpio_nr == 19) {
		/* RST */
		shift = 10;
	} else if (gpio_nr >= 20 && gpio_nr <= 21) {
		/* MDIO */
		shift = 12;
	} else if (gpio_nr >= 49 && gpio_nr <= 60) {
		/* RGMII1 */
		shift = 14;
	} else if (gpio_nr >= 22 && gpio_nr <= 33) {
		/* RGMII2 */
		shift = 15;
	} else if (gpio_nr >= 34 && gpio_nr <= 40) {
		/* SPI */
		shift = 16;
	} else if (gpio_nr >= 41 && gpio_nr <= 48) {
		/* SD */
		shift = 18;
	} else if (gpio_nr == 61) {
		/* ESW */
		shift = 20;
	}
#elif defined (MT7628_MP)
	if (gpio_nr >=  0 && gpio_nr <= 3) {
		/* I2S */
		shift = 6;  //0x40  1000000
	} else if (gpio_nr >= 4 && gpio_nr <= 5) {
		/* I2C */
		shift = 20;  //0x100000  100000000000000000000
	} else if (gpio_nr >= 6 && gpio_nr <= 6) {
		/* SPI_CS1 */
		shift = 4;  //0x10  1000000
	} else if (gpio_nr >= 7 && gpio_nr <= 10) {
		/* SPI */
		shift = 4;  //0x1000  10000
	} else if (gpio_nr >= 11 && gpio_nr <= 11) {
		/* GPIO */
		shift = 0;  //0x1  0001
	} else if (gpio_nr >= 12 && gpio_nr <= 13) {
		/* UART0 */
		//shift = 8;  
	} else if (gpio_nr == 36) {
		/* PERST */
		shift = 16; //0x10000  10000000000000000
	} else if (gpio_nr == 37) {
		/* REFCLK */
		shift = 18; //0x40000  1000000000000000000
	} else if (gpio_nr == 38) {
		/* WDT */
		shift = 14; //0x4000  100000000000000
	} else if (gpio_nr == 39) {
		/* P4_LED_AN */
		//shift = 18; 
	} else if (gpio_nr == 40) {
		/* P3_LED_AN */
		//shift = 18; 
	} else if (gpio_nr == 41) {
		/* P2_LED_AN */
		//shift = 18; 
	} else if (gpio_nr == 42) {
		/* P1_LED_AN */
		//shift = 18; 
	} else if (gpio_nr == 43) {
		/* P0_LED_AN */
		//shift = 18;  
	} else if (gpio_nr == 44) {
		/* WLED_AN */
		shift = 22; //0x400000  10000000000000000000000
	} else if (gpio_nr >= 45 && gpio_nr <= 46) {
		/* UART1 */
		shift = 8;  //0x100
	}
#endif	
	if (shift >= 0) {
		unsigned long old = ra_inl(RT2880_GPIOMODE_REG), new;

		ra_and(RT2880_GPIOMODE_REG, ~(mask << shift));
		if (val)
			ra_or(RT2880_GPIOMODE_REG, val << shift);
		if (old != (new = ra_inl(RT2880_GPIOMODE_REG))) {
			debug("GPIO#%d updated GPIOMODE register: %08lx -> %08lx\n",
				gpio_nr, old, new);
		}
	}
	return 0;
}

/* Read GPIO pin value.
 * @gpio_nr:	GPIO number
 * @return:	GPIO value
 * 	0/1:	Success
 * 	-1:	Invalid parameter
 */

#if defined(MT7620_MP)   
int mtk7620_get_gpio_pin(unsigned short gpio_nr)
#elif defined(MT7621_MP) //MT7621
int mtk7621_get_gpio_pin(unsigned short gpio_nr)
#else
int get_gpio_pin(unsigned short gpio_nr)
#endif   
{
	int shift;
	unsigned int reg;
	unsigned long val = 0;

	if (!is_valid_gpio_nr(gpio_nr))
		return -1;

#if defined(MT7620_MP)   
	reg = mtk7620_get_gpio_reg_addr(gpio_nr, GPIO_DATA);
#elif defined(MT7621_MP)
	reg = mtk7621_get_gpio_reg_addr(gpio_nr, GPIO_DATA);
#else
	reg = get_gpio_reg_addr(gpio_nr, GPIO_DATA);
#endif
	shift = get_gpio_reg_bit_shift(gpio_nr);

	val = !!(ra_inl(reg) & (1U << shift));

	return val;
}

/* Set GPIO pin value
 * @gpio_nr:	GPIO number
 * @val:
 * 	0:	Write 0 to GPIO pin
 *  otherwise:	Write 1 to GPIO pin
 * @return:
 * 	0:	Success
 * 	-1:	Invalid parameter
 */

#if defined(MT7620_MP)   
int mtk7620_set_gpio_pin(unsigned short gpio_nr, unsigned int val)
#elif defined(MT7621_MP)
int mtk7621_set_gpio_pin(unsigned short gpio_nr, unsigned int val)
#else
int set_gpio_pin(unsigned short gpio_nr, unsigned int val)
#endif
{
	int shift;
	unsigned int reg;

	if (!is_valid_gpio_nr(gpio_nr))
		return -1;

#if defined(MT7620_MP)   
	reg = mtk7620_get_gpio_reg_addr(gpio_nr, GPIO_DATA);
#elif defined(MT7621_MP)
	reg = mtk7621_get_gpio_reg_addr(gpio_nr, GPIO_DATA);
#else
	reg = get_gpio_reg_addr(gpio_nr, GPIO_DATA);
#endif
	shift = get_gpio_reg_bit_shift(gpio_nr);

	if (!val)
		ra_and(reg, ~(1U << shift));
	else
		ra_or(reg, 1U << shift);

	return 0;
}


void led_init(void)
{
#ifdef ASUS_RTN14U
	//led init
	//ephy_led 0/1/2/3 as gpio mode 40/41/42/43
	ra_or(RT2880_GPIOMODE_REG, (RALINK_GPIOMODE_JTAG));
	ra_or(PRGIO_ADDR+0x74, 0xf); // set LED as output
	ra_or(PRGIO_ADDR+0x70, 0x0); // turn on LED
	
	//wifi led init 
	ra_or(RT2880_GPIOMODE_REG, (RALINK_GPIOMODE_WLED));
	ra_or(PRGIO_ADDR+0x9c, 1); // set LED as output
	ra_or(PRGIO_ADDR+0x98, 0x0); // turn on LED
#elif defined(ASUS_RTAC52U)
	int i, led[] = { PWR_LED, WIFI_2G_LED, WAN_LED, LAN_LED, USB_LED };

	for (i = 0; i < ARRAY_SIZE(led); ++i) {
		mtk7620_set_gpio_dir(led[i], 0);	/* Set LED as output */
		mtk7620_set_gpio_pin(led[i], 0);	/* turn on LED */
	}
#elif defined(ASUS_RTAC51U)
	int i, led[] = { PWR_LED, WIFI_2G_LED, USB_LED };

	for (i = 0; i < ARRAY_SIZE(led); ++i) {
		mtk7620_set_gpio_dir(led[i], 0);	/* Set LED as output */
		mtk7620_set_gpio_pin(led[i], 0);	/* turn on LED */
	}
#elif defined(ASUS_RTAC51UP)
	int i, led[] = { PWR_LED, WIFI_2G_LED, USB_LED };

	for (i = 0; i < ARRAY_SIZE(led); ++i) {
		mtk7620_set_gpio_dir(led[i], 0);	/* Set LED as output */
		mtk7620_set_gpio_pin(led[i], 0);	/* turn on LED */
	}
#elif defined(ASUS_RTN54U)
	int i, led[] = { PWR_LED, WIFI_2G_LED, USB_LED };

	for (i = 0; i < ARRAY_SIZE(led); ++i) {
		mtk7620_set_gpio_dir(led[i], 0);	/* Set LED as output */
		mtk7620_set_gpio_pin(led[i], 0);	/* turn on LED */
	}
#elif defined(ASUS_RTN11P)
	int i, led[] = { WIFI_2G_LED, WAN_LED, LAN_LED };

	for (i = 0; i < ARRAY_SIZE(led); ++i) {
		mtk7620_set_gpio_dir(led[i], 0);	/* Set LED as output */
		mtk7620_set_gpio_pin(led[i], 0);	/* turn on LED */
	}
#elif defined(ASUS_RTAC1200GA1)	
	int i, led[] = { PWR_LED, WIFI_2G_LED, WAN_LED, LAN_LED, USB_LED ,WIFI_5G_LED};

	for (i = 0; i < ARRAY_SIZE(led); ++i) {
		mtk7621_set_gpio_dir(led[i], 0);	/* Set LED as output */
		mtk7621_set_gpio_pin(led[i], 0);	/* turn on LED */
	}
#elif defined(ASUS_RTAC1200GU)	
	int i, led[] = { PWR_LED, WIFI_2G_LED, WAN_LED, LAN_LED, USB_LED ,WIFI_5G_LED};

	for (i = 0; i < ARRAY_SIZE(led); ++i) {
		mtk7621_set_gpio_dir(led[i], 0);	/* Set LED as output */
		mtk7621_set_gpio_pin(led[i], 0);	/* turn on LED */
	}
#elif defined(ASUS_RTAC1200)
	int i, led[] = { PWR_LED, WIFI_2G_LED, USB_LED };

	for (i = 0; i < ARRAY_SIZE(led); ++i) {
		set_gpio_dir(led[i], 0);	/* Set LED as output */
		if (led[i] == USB_LED)
			set_gpio_pin(led[i], 1);	/* turn on LED for USB_LED */
		else
			set_gpio_pin(led[i], 0);	/* turn on LED */
	}
#elif defined(ASUS_RPAC56)
	int i, led[] = { PWR_LED_WHITE, WIFI_2G_LED_GREEN, WIFI_5G_LED_GREEN };

	for (i = 0; i < ARRAY_SIZE(led); ++i) {
		mtk7621_set_gpio_dir(led[i], 0);	/* Set LED as output */
		//mtk7621_set_gpio_pin(led[i], 1);	/* turn on LED */
	}
#elif defined(ASUS_RPAC87)
	int i, led[] = { PWR_LED, WIFI_2G_LED1, WIFI_2G_LED2, WIFI_2G_LED3, WIFI_2G_LED4, 
					WIFI_5G_LED1, WIFI_5G_LED2, WIFI_5G_LED3, WIFI_5G_LED4};

	for (i = 0; i < ARRAY_SIZE(led); ++i) {
		mtk7621_set_gpio_dir(led[i], 0);	/* Set LED as output */
		//mtk7621_set_gpio_pin(led[i], 1);	/* turn on LED */
	}
#elif defined(ASUS_RTAC85U)
	int i, led[] = { PWR_LED, WIFI_2G_LED, WAN_LED, LAN_LED, USB_LED, WIFI_5G_LED, WPS_LED };

	for (i = 0; i < ARRAY_SIZE(led); ++i) {
		mtk7621_set_gpio_dir(led[i], 0);	/* Set LED as output */
		mtk7621_set_gpio_pin(led[i], 1);
	}
#elif defined(ASUS_RTAC85P) || defined (ASUS_RTACRH26)
	int i, led[] = { PWR_LED, WIFI_2G_LED, WIFI_5G_LED };

	for (i = 0; i < ARRAY_SIZE(led); ++i) {
		mtk7621_set_gpio_dir(led[i], 0);	/* Set LED as output */
		mtk7621_set_gpio_pin(led[i], 1);
	}
#elif defined(ASUS_RTN800HP)
	int i, led[] = { PWR_LED, WIFI_2G_LED, WAN_LED };

	for (i = 0; i < ARRAY_SIZE(led); ++i) {
		mtk7621_set_gpio_dir(led[i], 0);	/* Set LED as output */
		mtk7621_set_gpio_pin(led[i], 1);
	}
#else
#error Invalid Product!!
#endif

#if defined(ALL_LED_OFF)
#if defined(MT7620_MP)	
	mtk7620_set_gpio_dir(ALL_LED_OFF_GPIO_NR, 0);	/* Set LED as output */
	mtk7620_set_gpio_pin(ALL_LED_OFF_GPIO_NR, 0);	/* turn on LED */
#elif defined(MT7621_MP)
	mtk7621_set_gpio_dir(ALL_LED_OFF_GPIO_NR, 0);	/* Set LED as output */
	mtk7621_set_gpio_pin(ALL_LED_OFF_GPIO_NR, 0);	/* turn on LED */
#else
	set_gpio_dir(ALL_LED_OFF_GPIO_NR, 0);	/* Set LED as output */
	set_gpio_pin(ALL_LED_OFF_GPIO_NR, 0);	/* turn on LED */
#endif	
#endif
}
#if defined(ASUS_RPAC56)
void led_all_off(){
	int i, led[] = { PWR_LED_WHITE, WIFI_2G_LED_GREEN, WIFI_5G_LED_GREEN };

	for (i = 0; i < ARRAY_SIZE(led); ++i) {
		mtk7621_set_gpio_pin(led[i], 0);	/* turn off LED */
	}
}

void led_all_on(){
	int i, led[] = { PWR_LED_WHITE, WIFI_2G_LED_GREEN, WIFI_5G_LED_GREEN };

	for (i = 0; i < ARRAY_SIZE(led); ++i) {
		mtk7621_set_gpio_pin(led[i], 1);	/* turn on LED */
	}
}
#elif defined(ASUS_RPAC87)
void led_all_off(){
	int i, led[] = { PWR_LED, WIFI_2G_LED1, WIFI_2G_LED2, WIFI_2G_LED3, WIFI_2G_LED4, 
					WIFI_5G_LED1, WIFI_5G_LED2, WIFI_5G_LED3, WIFI_5G_LED4};

	for (i = 0; i < ARRAY_SIZE(led); ++i) {
		mtk7621_set_gpio_pin(led[i], 1);	/* turn off LED */
	}
}

void led_all_on(){
	int i, led[] = { PWR_LED, WIFI_2G_LED1, WIFI_2G_LED2, WIFI_2G_LED3, WIFI_2G_LED4, 
					WIFI_5G_LED1, WIFI_5G_LED2, WIFI_5G_LED3, WIFI_5G_LED4};

	for (i = 0; i < ARRAY_SIZE(led); ++i) {
		mtk7621_set_gpio_pin(led[i], 0);	/* turn on LED */
	}
}

void wifi_led_all_off(){
	int i, led[] = { WIFI_2G_LED1, WIFI_2G_LED2, WIFI_2G_LED3, WIFI_2G_LED4, 
					WIFI_5G_LED1, WIFI_5G_LED2, WIFI_5G_LED3, WIFI_5G_LED4};

	for (i = 0; i < ARRAY_SIZE(led); ++i) {
		mtk7621_set_gpio_pin(led[i], 1);	/* turn off LED */
	}
}

void wifi_led_all_on(){
	int i, led[] = {WIFI_2G_LED1, WIFI_2G_LED2, WIFI_2G_LED3, WIFI_2G_LED4, 
					WIFI_5G_LED1, WIFI_5G_LED2, WIFI_5G_LED3, WIFI_5G_LED4};

	for (i = 0; i < ARRAY_SIZE(led); ++i) {
		mtk7621_set_gpio_pin(led[i], 0);	/* turn on LED */
	}
}

#endif

#if defined(HWNAT_FIX)
void rst_fengine(void)
{
	printf("ppe reset\n");	
	ra_or(RT2880_RSTCTRL_REG, 0x1<<31); //rst ppe
	ra_or(RALINK_FRAME_ENGINE_BASE+0x4,0x1); //rst pse
	udelay(500000);
	ra_and(RT2880_RSTCTRL_REG, ~(0x1<<31)); 
	ra_or(RALINK_FRAME_ENGINE_BASE+0x4,~(0x1)); 
}
#endif

void gpio_init(void)
{
extern const char *model;
#ifdef ASUS_RTN14U
	printf("ASUS %s gpio init : wps / reset pin\n", model);
	//wps:gpio2 
	//rst:gpio1
	//set gpio1 & 2  as gpio mode
	ra_or(RT2880_GPIOMODE_REG, (RALINK_GPIOMODE_I2C));
	//set gpio1 & 2 as input
	ra_and(PRGIO_ADDR+0x24, ~0x3);
#elif defined(ASUS_RTAC52U)
	printf("ASUS %s gpio init : wps / reset / radio onoff pin\n", model);
	mtk7620_set_gpio_dir(WPS_BTN, 1);
	mtk7620_set_gpio_dir(RST_BTN, 1);
	mtk7620_set_gpio_dir(RADIO_ONOFF_BTN, 1);
#elif defined(ASUS_RTAC51U)
	printf("ASUS %s gpio init : wps / reset pin\n", model);
	mtk7620_set_gpio_dir(WPS_BTN, 1);
	mtk7620_set_gpio_dir(RST_BTN, 1);
#elif defined(ASUS_RTAC51UP)
	printf("ASUS %s gpio init : wps / reset pin\n", model);
	mtk7620_set_gpio_dir(WPS_BTN, 1);
	mtk7620_set_gpio_dir(RST_BTN, 1);
#elif defined(ASUS_RTN54U)
	printf("ASUS %s gpio init : wps / reset pin\n", model);
	mtk7620_set_gpio_dir(WPS_BTN, 1);
	mtk7620_set_gpio_dir(RST_BTN, 1);
#elif defined(ASUS_RTN11P)
	printf("ASUS %s gpio init : reset pin\n", model);
	mtk7620_set_gpio_dir(RST_BTN, 1);
#elif defined(ASUS_RPAC56)
	printf("ASUS %s gpio init : wps / reset pin\n", model);
	mtk7621_set_gpio_dir(WPS_BTN, 1);
	mtk7621_set_gpio_dir(RST_BTN, 1);
#elif defined(ASUS_RTAC1200)
	printf("ASUS %s gpio init : wps / reset pin\n", model);
	set_gpio_dir(WPS_BTN, 1);
	set_gpio_dir(RST_BTN, 1);
#elif defined(ASUS_RTAC1200GA1)
	printf("ASUS %s gpio init : wps / reset pin\n", model);
	mtk7621_set_gpio_dir(WPS_BTN, 1);
	mtk7621_set_gpio_dir(RST_BTN, 1);
#elif defined(ASUS_RTAC1200GU)
	printf("ASUS %s gpio init : wps / reset pin\n", model);
	mtk7621_set_gpio_dir(WPS_BTN, 1);
	mtk7621_set_gpio_dir(RST_BTN, 1);
#elif defined(ASUS_RPAC87)
	printf("ASUS %s gpio init : wps / reset pin\n", model);
	mtk7621_set_gpio_dir(WPS_BTN, 1);
	mtk7621_set_gpio_dir(RST_BTN, 1);
#elif defined(ASUS_RTAC85U)
	printf("ASUS %s gpio init : wps / reset pin\n", model);
	mtk7621_set_gpio_dir(WPS_BTN, 1);
	mtk7621_set_gpio_dir(RST_BTN, 1);
#elif defined(ASUS_RTAC85P) || defined (ASUS_RTACRH26)
	printf("ASUS %s gpio init : wps / reset pin\n", model);
	mtk7621_set_gpio_dir(WPS_BTN, 1);
	mtk7621_set_gpio_dir(RST_BTN, 1);
#elif defined(ASUS_RTN800HP)
	printf("ASUS %s gpio init : wps / reset pin\n", model);
	mtk7621_set_gpio_dir(WPS_BTN, 1);
	mtk7621_set_gpio_dir(RST_BTN, 1);
#else
#error Invalid Product!!
#endif
}

unsigned long DETECT(void)
{
	int key = 0;

#if defined(MT7620_MP)
	if(!mtk7620_get_gpio_pin(RST_BTN))
#elif defined(MT7621_MP)  //MT7621
	if(!mtk7621_get_gpio_pin(RST_BTN)) 
#else
	if(!get_gpio_pin(RST_BTN)) 
#endif	
	{   
		key = 1;
		printf("reset buootn pressed!\n");
	}
	return key;
}

unsigned long DETECT_WPS(void)
{
	int key = 0;

#ifdef WPS_BTN
#if defined(MT7620_MP)
	if(!mtk7620_get_gpio_pin(WPS_BTN)) 
#elif defined(MT7621_MP) //MT7621	   
	if(!mtk7621_get_gpio_pin(WPS_BTN)) 
#else
	if(!get_gpio_pin(WPS_BTN)) 
#endif	
	{   
		key = 1;
		printf("wps buootn pressed!\n");
	}
#endif
	return key;
}

void PWR_LEDON(void)
{
#if defined(ASUS_RTN11P)	// power LED is controlled by HW. so we flash other LED instead.
	mtk7620_set_gpio_pin(WIFI_2G_LED, 0);
	mtk7620_set_gpio_pin(WAN_LED, 0);
	mtk7620_set_gpio_pin(LAN_LED, 0);
#else
#if defined(MT7620_MP)	
	mtk7620_set_gpio_pin(PWR_LED, 0);
#elif defined(MT7621_MP) //MT7621
	#if defined(ASUS_RPAC56)
	mtk7621_set_gpio_dir(PWR_LED_ORANGE, 0);
	mtk7621_set_gpio_pin(PWR_LED_ORANGE, 1);
	/*#elif defined(ASUS_RPAC87)
	mtk7621_set_gpio_dir(PWR_LED_ORANGE, 0);
	mtk7621_set_gpio_pin(PWR_LED_ORANGE, 1);*/
	#else
	mtk7621_set_gpio_pin(PWR_LED, 0);
	#endif
#else
	set_gpio_pin(PWR_LED, 0);
#endif
#endif
}

void LEDON(void)
{
#if defined(ASUS_RTN14U) || defined(ASUS_RTAC52U)
	mtk7620_set_gpio_pin(PWR_LED, 0);
	mtk7620_set_gpio_pin(WAN_LED, 0);
	mtk7620_set_gpio_pin(LAN_LED, 0);
	mtk7620_set_gpio_pin(USB_LED, 0);
	mtk7620_set_gpio_pin(WIFI_2G_LED, 0);
#elif defined(ASUS_RTAC51U)
	mtk7620_set_gpio_pin(PWR_LED, 0);
	mtk7620_set_gpio_pin(USB_LED, 0);
	mtk7620_set_gpio_pin(WIFI_2G_LED, 0);
#elif defined(ASUS_RTAC51UP)
	mtk7620_set_gpio_pin(PWR_LED, 0);
	mtk7620_set_gpio_pin(USB_LED, 0);
	mtk7620_set_gpio_pin(WIFI_2G_LED, 0);
#elif defined(ASUS_RTN54U)
	mtk7620_set_gpio_pin(PWR_LED, 0);
	mtk7620_set_gpio_pin(USB_LED, 0);
	mtk7620_set_gpio_pin(WIFI_2G_LED, 0);
#elif defined(ASUS_RTN11P)
	mtk7620_set_gpio_pin(WIFI_2G_LED, 0);
	mtk7620_set_gpio_pin(WAN_LED, 0);
	mtk7620_set_gpio_pin(LAN_LED, 0);
#elif defined(ASUS_RTAC1200)
	set_gpio_pin(PWR_LED, 0);
	set_gpio_pin(USB_LED, 1);
	set_gpio_pin(WIFI_2G_LED, 0);
#elif defined(ASUS_RPAC56)
	mtk7621_set_gpio_pin(WIFI_2G_LED_BLUE, 1);
	mtk7621_set_gpio_pin(WIFI_2G_LED_GREEN, 1);
	mtk7621_set_gpio_pin(WIFI_2G_LED_RED, 1);
	mtk7621_set_gpio_pin(PWR_LED_WHITE, 1);
	mtk7621_set_gpio_pin(PWR_LED_ORANGE, 1);
	mtk7621_set_gpio_pin(WIFI_5G_LED_BLUE, 1);
	mtk7621_set_gpio_pin(WIFI_5G_LED_GREEN, 1);
	mtk7621_set_gpio_pin(WIFI_5G_LED_RED, 1);
#elif defined(ASUS_RTAC1200GA1)	
	mtk7621_set_gpio_pin(PWR_LED, 0);
	mtk7621_set_gpio_pin(WAN_LED, 0);
	mtk7621_set_gpio_pin(LAN_LED, 0);
	mtk7621_set_gpio_pin(USB_LED, 0);
	mtk7621_set_gpio_pin(WIFI_2G_LED, 0);
	mtk7621_set_gpio_pin(WIFI_5G_LED, 0);
#elif defined(ASUS_RTAC1200GU)	
	mtk7621_set_gpio_pin(PWR_LED, 0);
	mtk7621_set_gpio_pin(WAN_LED, 0);
	mtk7621_set_gpio_pin(LAN_LED, 0);
	mtk7621_set_gpio_pin(USB_LED, 0);
	mtk7621_set_gpio_pin(WIFI_2G_LED, 0);
	mtk7621_set_gpio_pin(WIFI_5G_LED, 0);
#elif defined(ASUS_RPAC87)
	mtk7621_set_gpio_pin(PWR_LED, 0);
	mtk7621_set_gpio_pin(WIFI_2G_LED1, 0);
	mtk7621_set_gpio_pin(WIFI_2G_LED2, 0);
	mtk7621_set_gpio_pin(WIFI_2G_LED3, 0);
	mtk7621_set_gpio_pin(WIFI_2G_LED4, 0);
	mtk7621_set_gpio_pin(WIFI_5G_LED1, 0);
	mtk7621_set_gpio_pin(WIFI_5G_LED2, 0);
	mtk7621_set_gpio_pin(WIFI_5G_LED3, 0);
	mtk7621_set_gpio_pin(WIFI_5G_LED4, 0);
#elif defined(ASUS_RTAC85U)
	mtk7621_set_gpio_pin(PWR_LED, 0);
	mtk7621_set_gpio_pin(WAN_LED, 0);
	mtk7621_set_gpio_pin(LAN_LED, 0);
	mtk7621_set_gpio_pin(USB_LED, 0);
	mtk7621_set_gpio_pin(WIFI_2G_LED, 0);
	mtk7621_set_gpio_pin(WIFI_5G_LED, 0);
	mtk7621_set_gpio_pin(WPS_LED, 0);
#elif defined(ASUS_RTAC85P)  || defined (ASUS_RTACRH26)
	mtk7621_set_gpio_pin(PWR_LED, 0);
	mtk7621_set_gpio_pin(WIFI_2G_LED, 0);
	mtk7621_set_gpio_pin(WIFI_5G_LED, 0);
#elif defined(ASUS_RTN800HP)
	mtk7621_set_gpio_pin(PWR_LED, 0);
	mtk7621_set_gpio_pin(WAN_LED, 0);
	mtk7621_set_gpio_pin(WIFI_2G_LED, 0);
#else
#error Invalid product
#endif
}

void LEDOFF(void)
{
#if defined(ASUS_RTN14U) || defined(ASUS_RTAC52U)
	mtk7620_set_gpio_pin(PWR_LED, 1);
	mtk7620_set_gpio_pin(WAN_LED, 1);
	mtk7620_set_gpio_pin(LAN_LED, 1);
	mtk7620_set_gpio_pin(USB_LED, 1);
	mtk7620_set_gpio_pin(WIFI_2G_LED, 1);
#elif defined(ASUS_RTAC51U)
	mtk7620_set_gpio_pin(PWR_LED, 1);
	mtk7620_set_gpio_pin(USB_LED, 1);
	mtk7620_set_gpio_pin(WIFI_2G_LED, 1);
#elif defined(ASUS_RTAC51UP)
	mtk7620_set_gpio_pin(PWR_LED, 1);
	mtk7620_set_gpio_pin(USB_LED, 1);
	mtk7620_set_gpio_pin(WIFI_2G_LED, 1);
#elif defined(ASUS_RTN54U)
	mtk7620_set_gpio_pin(PWR_LED, 1);
	mtk7620_set_gpio_pin(USB_LED, 1);
	mtk7620_set_gpio_pin(WIFI_2G_LED, 1);
#elif defined(ASUS_RTN11P)
	mtk7620_set_gpio_pin(WIFI_2G_LED, 1);
	mtk7620_set_gpio_pin(WAN_LED, 1);
	mtk7620_set_gpio_pin(LAN_LED, 1);
#elif defined(ASUS_RTAC1200)
	set_gpio_pin(PWR_LED, 1);
	set_gpio_pin(USB_LED, 0);
	set_gpio_pin(WIFI_2G_LED, 1);
#elif defined(ASUS_RPAC56)
	mtk7621_set_gpio_pin(WIFI_2G_LED_BLUE, 0);
	mtk7621_set_gpio_pin(WIFI_2G_LED_GREEN, 0);
	mtk7621_set_gpio_pin(WIFI_2G_LED_RED, 0);
	mtk7621_set_gpio_pin(PWR_LED_WHITE, 0);
	mtk7621_set_gpio_pin(PWR_LED_ORANGE, 0);
	mtk7621_set_gpio_pin(WIFI_5G_LED_BLUE, 0);
	mtk7621_set_gpio_pin(WIFI_5G_LED_GREEN, 0);
	mtk7621_set_gpio_pin(WIFI_5G_LED_RED, 0);
#elif defined(ASUS_RTAC1200GA1)
	mtk7621_set_gpio_pin(PWR_LED, 1);
	mtk7621_set_gpio_pin(WAN_LED, 1);
	mtk7621_set_gpio_pin(LAN_LED, 1);
	mtk7621_set_gpio_pin(USB_LED, 1);
	mtk7621_set_gpio_pin(WIFI_2G_LED, 1);
	mtk7621_set_gpio_pin(WIFI_5G_LED, 1);
#elif defined(ASUS_RTAC1200GU)
	mtk7621_set_gpio_pin(PWR_LED, 1);
	mtk7621_set_gpio_pin(WAN_LED, 1);
	mtk7621_set_gpio_pin(LAN_LED, 1);
	mtk7621_set_gpio_pin(USB_LED, 1);
	mtk7621_set_gpio_pin(WIFI_2G_LED, 1);
	mtk7621_set_gpio_pin(WIFI_5G_LED, 1);
#elif defined(ASUS_RPAC87)
	mtk7621_set_gpio_pin(PWR_LED, 1);
	mtk7621_set_gpio_pin(WIFI_2G_LED1, 1);
	mtk7621_set_gpio_pin(WIFI_2G_LED2, 1);
	mtk7621_set_gpio_pin(WIFI_2G_LED3, 1);
	mtk7621_set_gpio_pin(WIFI_2G_LED4, 1);
	mtk7621_set_gpio_pin(WIFI_5G_LED1, 1);
	mtk7621_set_gpio_pin(WIFI_5G_LED2, 1);
	mtk7621_set_gpio_pin(WIFI_5G_LED3, 1);
	mtk7621_set_gpio_pin(WIFI_5G_LED4, 1);
#elif defined(ASUS_RTAC85U)
	mtk7621_set_gpio_pin(PWR_LED, 1);
	mtk7621_set_gpio_pin(WAN_LED, 1);
	mtk7621_set_gpio_pin(LAN_LED, 1);
	mtk7621_set_gpio_pin(USB_LED, 1);
	mtk7621_set_gpio_pin(WIFI_2G_LED, 1);
	mtk7621_set_gpio_pin(WIFI_5G_LED, 1);
	mtk7621_set_gpio_pin(WPS_LED, 1);
#elif defined(ASUS_RTAC85P) || defined (ASUS_RTACRH26)
	mtk7621_set_gpio_pin(PWR_LED, 1);
	mtk7621_set_gpio_pin(WIFI_2G_LED, 1);
	mtk7621_set_gpio_pin(WIFI_5G_LED, 1);
#elif defined(ASUS_RTN800HP)
	mtk7621_set_gpio_pin(PWR_LED, 1);
	mtk7621_set_gpio_pin(WAN_LED, 1);
	mtk7621_set_gpio_pin(WIFI_2G_LED, 1);
#else
#error Invalid product
#endif
}

#if defined(ALL_LED_OFF)
void ALL_LEDON(void)
{
#if defined(MT7620_MP)   
	mtk7620_set_gpio_pin(ALL_LED_OFF_GPIO_NR, 0);	/* turn on LED */
#elif defined(MT7621_MP)
	mtk7621_set_gpio_pin(ALL_LED_OFF_GPIO_NR, 0);	/* turn on LED */
#else
	set_gpio_pin(ALL_LED_OFF_GPIO_NR, 0);
#endif	
}

void ALL_LEDOFF(void)
{
#if defined(MT7620_MP)   
	mtk7620_set_gpio_pin(ALL_LED_OFF_GPIO_NR, 1);	/* turn off LED */
#elif defined(MT7621_MP)
	mtk7621_set_gpio_pin(ALL_LED_OFF_GPIO_NR, 1);	/* turn off LED */
#else
	set_gpio_pin(ALL_LED_OFF_GPIO_NR, 1);
#endif
}
#endif //#if defined(ALL_LED_OFF)

void flash_led_once(void)
{
#if defined(ASUS_RTAC85U)
	int i, led[] = { LAN_LED, WAN_LED };

	for (i = 0; i < ARRAY_SIZE(led); ++i)
		mtk7621_set_gpio_pin(led[i], 0);	/* turn on LED */
	udelay(500000);
	for (i = 0; i < ARRAY_SIZE(led); ++i)
		mtk7621_set_gpio_pin(led[i], 1);	/* turn off LED */
#endif
}
