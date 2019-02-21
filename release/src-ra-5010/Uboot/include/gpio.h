#ifndef GPIO_H
#define GPIO_H

#if defined(MT7620_MP) || defined (MT7621_MP)
/* LED, Button GPIO# definition */
#if defined(ASUS_RTN14U)
#define RST_BTN		1	/* I2C_SD */
#define WPS_BTN		2	/* I2C_SCLK */
#define PWR_LED		43	/* EPHY_LED3 */
#define WIFI_2G_LED	72	/* WLAN_N */
#define WAN_LED		40	/* EPHY_LED0 */
#define LAN_LED		41	/* EPHY_LED1 */
#define USB_LED		42	/* EPHY_LED2 */

#elif defined(ASUS_RTAC52U)
#define RST_BTN		1	/* I2C_SD */
#define WPS_BTN		2	/* I2C_SCLK */
#define RADIO_ONOFF_BTN	13	/* DSR_N */
#define PWR_LED		9	/* CTS_N */
#define WIFI_2G_LED	72	/* WLAN_N */
#define WAN_LED		8	/* TXD */
#define LAN_LED		12	/* DCD_N */
#define USB_LED		14	/* RIN */

#elif defined(ASUS_RTAC51U)
#define RST_BTN		1	/* I2C_SD */
#define WPS_BTN		2	/* I2C_SCLK */
#define PWR_LED		9	/* CTS_N */
#define WIFI_2G_LED	72	/* WLAN_N */
#define USB_LED		14	/* RIN */

#elif defined(ASUS_RTAC51UP)
#define RST_BTN		1	/* I2C_SD */
#define WPS_BTN		2	/* I2C_SCLK */
#define PWR_LED		9	/* CTS_N */
#define WIFI_2G_LED	72	/* WLAN_N */
#define USB_LED		14	/* RIN */

#elif defined(ASUS_RTN54U)
#define RST_BTN		1	/* I2C_SD */
#define WPS_BTN		2	/* I2C_SCLK */
#define PWR_LED		9	/* CTS_N */
#define WIFI_2G_LED	72	/* WLAN_N */
#define USB_LED		14	/* RIN */

#elif defined(ASUS_RTAC1200GA1)
#define RST_BTN		41	/* ND_WP */
#define WPS_BTN		43	/* ND_CLE */
#define PWR_LED		48	/* ND_D3 */
#define WIFI_2G_LED	14	/* WLAN_N */
#define USB_LED		47	/* ND_D2 */
#define WIFI_5G_LED	15	/* WLAN_N */
#define WAN_LED		16	/* EPHY_LED0 */
#define LAN_LED		7	/* EPHY_LED1 */

#elif defined(ASUS_RTAC1200GU)
#define RST_BTN		41	/* ND_WP */
#define WPS_BTN		43	/* ND_CLE */
#define PWR_LED		48	/* ND_D3 */
#define WIFI_2G_LED	14	/* WLAN_N */
#define USB_LED		47	/* ND_D2 */
#define WIFI_5G_LED	15	/* WLAN_N */
#define WAN_LED		16	/* EPHY_LED0 */
#define LAN_LED		7	/* EPHY_LED1 */

#elif defined(ASUS_RTN11P)
#define RST_BTN		17	/* WDT_RST_N */
#define WIFI_2G_LED	72	/* WLAN_N */
#define WAN_LED		44	/* EPHY_LED4_N_JTRST_N */
#define LAN_LED		39	/* SPI_WP */

#elif defined(ASUS_RPAC56)
#define RST_BTN				41	/* ND_WP*/
#define WPS_BTN				10	/* CTS2_N */
#define WIFI_2G_LED_RED		16	/* JTCLK */
#define WIFI_2G_LED_GREEN	13	/* JTDO */
#define WIFI_2G_LED_BLUE	12	/* RXD2 */
#define WIFI_5G_LED_RED		45	/* ND_D0 */
#define WIFI_5G_LED_GREEN	44	/* ND_ALE */
#define WIFI_5G_LED_BLUE	43	/* ND_CLE */
#define PWR_LED_WHITE		48	/* ND_D3 */
#define PWR_LED_ORANGE		47	/* ND_D2 */
#define PWR_LED_Blue		46	/* ND_D1 */

#elif defined(ASUS_RPAC87)
#define RST_BTN			3	/* I2C_SD */
#define WPS_BTN			18	/* WDT_RST_N */
#define PWR_LED			4	/* I2C_SCLK */
#define WIFI_2G_LED1		42	/* ND_RB_N */
#define WIFI_2G_LED2		43	/* ND_CLE */
#define WIFI_2G_LED3		41	/* ND_WP */
#define WIFI_2G_LED4		44	/* ND_ALE */
#define WIFI_5G_LED1		45	/* ND_D0 */
#define WIFI_5G_LED2		46	/* ND_D1 */
#define WIFI_5G_LED3		47	/* ND_D2 */
#define WIFI_5G_LED4		48	/* ND_D3 */

#elif defined(ASUS_RTAC85U)
#define RST_BTN		16	/* JTCLK */
#define WPS_BTN		18	/* WDT_RST_N */
#define PWR_LED		6	/* CTS3_N */
#define WIFI_2G_LED	4	/* I2C_SCLK */
#define USB_LED		15	/* JTMS */
#define WIFI_5G_LED	7	/* TXD3 */
#define WAN_LED		13	/* JTDO */
#define LAN_LED		14	/* JTDI */
#define WPS_LED		12	/* RXD2 */

#elif defined(ASUS_RTAC85P) || defined(ASUS_RTACRH26)
#define RST_BTN		3	/* JTCLK */
#define WPS_BTN		6	/* WDT_RST_N */
#define PWR_LED		4	/* CTS3_N */
#define WIFI_2G_LED	10	/* I2C_SCLK */
#define WIFI_5G_LED	8	/* TXD3 */

#elif defined(ASUS_RTN800HP)
#define RST_BTN		16	/* JTCLK */
#define WPS_BTN		18	/* WDT_RST_N */
#define PWR_LED		12	/* RXD2 */
#define WIFI_2G_LED	14	/* JTDI */
#define WAN_LED		13	/* JTDO */
#else

#if defined(CONFIG_ASUS_PRODUCT)
#error Invalid product
#endif //#if defined(CONFIG_ASUS_PRODUCT)
#endif //#if defined(ASUS_RTN14U)
#endif  //#if defined(MT7620_MP) || defined (MT7621_MP)

#if defined(MT7628_MP)
#if defined(ASUS_RTAC1200)
#define RST_BTN		5	/* I2C_SD */
#define WPS_BTN		11	/* GPIO0 */
#define PWR_LED		37	/* REF_CLKO */
#define WIFI_2G_LED	44	/* WLED_N */
#define USB_LED		6	/* SPI_CS1 */
#endif  // #if defined(ASUS_RTAC1200)
#endif  // #if defined(MT7628_MP)


enum gpio_reg_id {
	GPIO_INT = 0,
	GPIO_EDGE,
	GPIO_RMASK,
	GPIO_MASK,
	GPIO_DATA,
	GPIO_DIR,
	GPIO_POL,
	GPIO_SET,
	GPIO_RESET,
#if defined(MT7620_MP) 
	GPIO_TOG,
#endif	
	GPIO_MAX_REG
};

#if defined(MT7620_MP) 
extern unsigned int mtk7620_get_gpio_reg_addr(unsigned short gpio_nr, enum gpio_reg_id id);
extern int mtk7620_set_gpio_dir(unsigned short gpio_nr, unsigned short gpio_dir);
extern int mtk7620_get_gpio_pin(unsigned short gpio_nr);
extern int mtk7620_set_gpio_pin(unsigned short gpio_nr, unsigned int val);
#elif defined(MT7621_MP)
extern unsigned int mtk7621_get_gpio_reg_addr(unsigned short gpio_nr, enum gpio_reg_id id);
#else
extern unsigned int get_gpio_reg_addr(unsigned short gpio_nr, enum gpio_reg_id id);
extern int set_gpio_dir(unsigned short gpio_nr, unsigned short gpio_dir);
extern int get_gpio_pin(unsigned short gpio_nr);
extern int set_gpio_pin(unsigned short gpio_nr, unsigned int val);
#if defined(ASUS_RPAC56) || defined(ASUS_RPAC87)
extern void led_all_off();
extern void led_all_on();
#endif
#if defined(ASUS_RPAC87)
extern void wifi_led_all_off();
extern void wifi_led_all_on();
#endif
#endif



extern void led_init(void);
extern void gpio_init(void);
extern void LEDON(void);
extern void LEDOFF(void);
extern unsigned long DETECT(void);
extern unsigned long DETECT_WPS(void);
extern void rst_fengine(void);

#if defined(ALL_LED_OFF)
extern void ALL_LEDON(void);
extern void ALL_LEDOFF(void);
#endif

#endif
