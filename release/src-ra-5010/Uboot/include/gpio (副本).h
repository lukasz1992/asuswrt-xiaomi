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

#elif defined(ASUS_RTN54U)
#define RST_BTN		1	/* I2C_SD */
#define WPS_BTN		2	/* I2C_SCLK */
#define PWR_LED		9	/* CTS_N */
#define WIFI_2G_LED	72	/* WLAN_N */
#define USB_LED		14	/* RIN */

#elif defined(ASUS_RTAC54U)
#define RST_BTN		1	/* I2C_SD */
#define WPS_BTN		2	/* I2C_SCLK */
#define PWR_LED		9	/* CTS_N */
#define WIFI_2G_LED	72	/* WLAN_N */
#define USB_LED		14	/* RIN */
  
#elif defined(ASUS_RTN56UB1)
#define RST_BTN		6	/* I2C_SD */
#define WPS_BTN		18	/* I2C_SCLK */
#define PWR_LED		12	/* CTS_N */
#define WIFI_2G_LED	14	/* WLAN_N */
#define USB_LED		13	/* RIN */
#define WIFI_5G_LED	15	/* WLAN_N */
#define WAN_LED		16	/* EPHY_LED0 */
#define LAN_LED		7	/* EPHY_LED1 */

#elif defined(ASUS_RTAC1200HP)
#define RST_BTN		62	/* I2C_SD */
#define WPS_BTN		61	/* I2C_SCLK */
#define PWR_LED		65	/* CTS_N */
#define WIFI_2G_LED	72	/* WLAN_N */
#define USB_LED		67	/* RIN */
#define WAN_LED		68	/* EPHY_LED0 */
#define LAN_LED		69	/* EPHY_LED1 */
#define RADIO_ONOFF_BTN	66	/* DSR_N */
#define WIFI_5G_LED	70	/* WLAN_N */

#elif defined(ASUS_RTN11P)
#define RST_BTN		17	/* WDT_RST_N */
#define WIFI_2G_LED	72	/* WLAN_N */
#define WAN_LED		44	/* EPHY_LED4_N_JTRST_N */
#define LAN_LED		39	/* SPI_WP */

#else
#if defined(CONFIG_ASUS_PRODUCT)
#error Invalid product
#endif

#endif

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
#else
extern unsigned int mtk7621_get_gpio_reg_addr(unsigned short gpio_nr, enum gpio_reg_id id);
#endif
extern int mtk7620_set_gpio_dir(unsigned short gpio_nr, unsigned short gpio_dir);
extern int mtk7620_get_gpio_pin(unsigned short gpio_nr);
extern int mtk7620_set_gpio_pin(unsigned short gpio_nr, unsigned int val);
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
