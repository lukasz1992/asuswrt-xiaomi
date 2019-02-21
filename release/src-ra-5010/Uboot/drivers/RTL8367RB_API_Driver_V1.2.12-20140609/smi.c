/*
* Copyright c                  Realtek Semiconductor Corporation, 2006
* All rights reserved.
*
* Program : Control  smi connected RTL8366
* Abstract :
* Author : Yu-Mei Pan (ympan@realtek.com.cn)
*  $Id: smi.c,v 1.2 2008-04-10 03:04:19 shiehyy Exp $
*/

#include <common.h>
#include <rtk_types.h>
#include <gpio.h>
#include <smi.h>
#include "rtk_error.h"

#define MDC_MDIO_DUMMY_ID           0
#define MDC_MDIO_CTRL0_REG          31
#define MDC_MDIO_START_REG          29
#define MDC_MDIO_CTRL1_REG          21
#define MDC_MDIO_ADDRESS_REG        23
#define MDC_MDIO_DATA_WRITE_REG     24
#define MDC_MDIO_DATA_READ_REG      25
#define MDC_MDIO_PREAMBLE_LEN       32

#define MDC_MDIO_START_OP          0xFFFF
#define MDC_MDIO_ADDR_OP           0x000E
#define MDC_MDIO_READ_OP           0x0001
#define MDC_MDIO_WRITE_OP          0x0003

#define SPI_READ_OP                 0x3
#define SPI_WRITE_OP                0x2
#define SPI_READ_OP_LEN             0x8
#define SPI_WRITE_OP_LEN            0x8
#define SPI_REG_LEN                 16
#define SPI_DATA_LEN                16

#define DELAY                        10000
#define CLK_DURATION(clk)            { int i; for(i=0; i<clk; i++); }
#define _SMI_ACK_RESPONSE(ok)        { /*if (!(ok)) return RT_ERR_FAILED; */}

#if defined(MDC_MDIO_OPERATION)
#include <asm/types.h>

enum {
	MDIO_ACT_READ=0,
	MDIO_ACT_WRITE,

	MDIO_ACT_MAX
};

extern void mii_mgr_init(void);
extern int mii_mgr_read(u32 phy_addr, u32 phy_register, u32 *read_data);
extern int mii_mgr_write(u32 phy_addr, u32 phy_register, u32 write_data);

static inline void mdc_mdio_init(void)
{
	mii_mgr_init();
}

static inline int mdc_mdio_read(rtk_uint32 preamble_len, rtk_uint32 phy_id, rtk_uint32 register_id, rtk_uint32 *data)
{
	int r;

	r = mii_mgr_read(phy_id, register_id, data);
	if (r == 1)
		return RT_ERR_OK;
	else
		return RT_ERR_FAILED;
}

static int mdc_mdio_write(rtk_uint32 preamble_len, rtk_uint32 phy_id, rtk_uint32 register_id, rtk_uint32 data)
{
	int r;
	r = mii_mgr_write(phy_id, register_id, data);
	if (r == 1)
		return RT_ERR_OK;
	else
		return RT_ERR_FAILED;
}

#define MDC_MDIO_READ			mdc_mdio_read
#define MDC_MDIO_WRITE			mdc_mdio_write
#define rtlglue_drvMutexLock()		do {} while (0)
#define rtlglue_drvMutexUnlock()	do {} while (0)
#elif defined(SPI_OPERATION)
#error YOU HAVE TO IMPLEMENT PLATFORM SPECIFIC FUNCTION FOR SPI_OPERATION
#endif

gpioID smi_SCK;        /* GPIO used for SMI Clock Generation */
gpioID smi_SDA;        /* GPIO used for SMI Data signal */
gpioID smi_RST;     /* GPIO used for reset swtich */


#define ack_timer                    5
#define max_register                0x018A

#if defined(MDC_MDIO_OPERATION) || defined(SPI_OPERATION)
    /* No local function in MDC/MDIO mode */
#else
void _smi_start(void)
{

    /* change GPIO pin to Output only */
    _rtl865x_initGpioPin(smi_SDA, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
    _rtl865x_initGpioPin(smi_SCK, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);

    /* Initial state: SCK: 0, SDA: 1 */
    _rtl865x_setGpioDataBit(smi_SCK, 0);
    _rtl865x_setGpioDataBit(smi_SDA, 1);
    CLK_DURATION(DELAY);

    /* CLK 1: 0 -> 1, 1 -> 0 */
    _rtl865x_setGpioDataBit(smi_SCK, 1);
    CLK_DURATION(DELAY);
    _rtl865x_setGpioDataBit(smi_SCK, 0);
    CLK_DURATION(DELAY);

    /* CLK 2: */
    _rtl865x_setGpioDataBit(smi_SCK, 1);
    CLK_DURATION(DELAY);
    _rtl865x_setGpioDataBit(smi_SDA, 0);
    CLK_DURATION(DELAY);
    _rtl865x_setGpioDataBit(smi_SCK, 0);
    CLK_DURATION(DELAY);
    _rtl865x_setGpioDataBit(smi_SDA, 1);

}



void _smi_writeBit(rtk_uint16 signal, rtk_uint32 bitLen)
{
    for( ; bitLen > 0; bitLen--)
    {
        CLK_DURATION(DELAY);

        /* prepare data */
        if ( signal & (1<<(bitLen-1)) )
            _rtl865x_setGpioDataBit(smi_SDA, 1);
        else
            _rtl865x_setGpioDataBit(smi_SDA, 0);
        CLK_DURATION(DELAY);

        /* clocking */
        _rtl865x_setGpioDataBit(smi_SCK, 1);
        CLK_DURATION(DELAY);
        _rtl865x_setGpioDataBit(smi_SCK, 0);
    }
}



void _smi_readBit(rtk_uint32 bitLen, rtk_uint32 *rData)
{
    rtk_uint32 u;

    /* change GPIO pin to Input only */
    _rtl865x_initGpioPin(smi_SDA, GPIO_PERI_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);

    for (*rData = 0; bitLen > 0; bitLen--)
    {
        CLK_DURATION(DELAY);

        /* clocking */
        _rtl865x_setGpioDataBit(smi_SCK, 1);
        CLK_DURATION(DELAY);
        _rtl865x_getGpioDataBit(smi_SDA, &u);
        _rtl865x_setGpioDataBit(smi_SCK, 0);

        *rData |= (u << (bitLen - 1));
    }

    /* change GPIO pin to Output only */
    _rtl865x_initGpioPin(smi_SDA, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
}



void _smi_stop(void)
{

    CLK_DURATION(DELAY);
    _rtl865x_setGpioDataBit(smi_SDA, 0);
    _rtl865x_setGpioDataBit(smi_SCK, 1);
    CLK_DURATION(DELAY);
    _rtl865x_setGpioDataBit(smi_SDA, 1);
    CLK_DURATION(DELAY);
    _rtl865x_setGpioDataBit(smi_SCK, 1);
    CLK_DURATION(DELAY);
    _rtl865x_setGpioDataBit(smi_SCK, 0);
    CLK_DURATION(DELAY);
    _rtl865x_setGpioDataBit(smi_SCK, 1);

    /* add a click */
    CLK_DURATION(DELAY);
    _rtl865x_setGpioDataBit(smi_SCK, 0);
    CLK_DURATION(DELAY);
    _rtl865x_setGpioDataBit(smi_SCK, 1);


    /* change GPIO pin to Output only */
    _rtl865x_initGpioPin(smi_SDA, GPIO_PERI_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);
    _rtl865x_initGpioPin(smi_SCK, GPIO_PERI_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);


}

#endif /* End of #if defined(MDC_MDIO_OPERATION) || defined(SPI_OPERATION) */

rtk_int32 smi_reset(rtk_uint32 port, rtk_uint32 pinRST)
{
#if defined(MDC_MDIO_OPERATION) || defined(SPI_OPERATION)

#else
    gpioID gpioId;
    rtk_int32 res;

    /* Initialize GPIO port A, pin 7 as SMI RESET */
    gpioId = GPIO_ID(port, pinRST);
    res = _rtl865x_initGpioPin(gpioId, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
    if (res != RT_ERR_OK)
        return res;
    smi_RST = gpioId;

    _rtl865x_setGpioDataBit(smi_RST, 1);
    CLK_DURATION(1000000);
    _rtl865x_setGpioDataBit(smi_RST, 0);
    CLK_DURATION(1000000);
    _rtl865x_setGpioDataBit(smi_RST, 1);
    CLK_DURATION(1000000);

    /* change GPIO pin to Input only */
    _rtl865x_initGpioPin(smi_RST, GPIO_PERI_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);
#endif
    return RT_ERR_OK;
}


rtk_int32 smi_init(rtk_uint32 port, rtk_uint32 pinSCK, rtk_uint32 pinSDA)
{
#if defined(MDC_MDIO_OPERATION) || defined(SPI_OPERATION)

#else
    gpioID gpioId;
    rtk_int32 res;

    /* change GPIO pin to Input only */
    /* Initialize GPIO port C, pin 0 as SMI SDA0 */
    gpioId = GPIO_ID(port, pinSDA);
    res = _rtl865x_initGpioPin(gpioId, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
    if (res != RT_ERR_OK)
        return res;
    smi_SDA = gpioId;


    /* Initialize GPIO port C, pin 1 as SMI SCK0 */
    gpioId = GPIO_ID(port, pinSCK);
    res = _rtl865x_initGpioPin(gpioId, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
    if (res != RT_ERR_OK)
        return res;
    smi_SCK = gpioId;


    _rtl865x_setGpioDataBit(smi_SDA, 1);
    _rtl865x_setGpioDataBit(smi_SCK, 1);
#endif
    return RT_ERR_OK;
}




rtk_int32 smi_read(rtk_uint32 mAddrs, rtk_uint32 *rData)
{
#if defined(MDC_MDIO_OPERATION)

    /* Write Start command to register 29 */
    MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_DUMMY_ID, MDC_MDIO_START_REG, MDC_MDIO_START_OP);

    /* Write address control code to register 31 */
    MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_DUMMY_ID, MDC_MDIO_CTRL0_REG, MDC_MDIO_ADDR_OP);

    /* Write Start command to register 29 */
    MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_DUMMY_ID, MDC_MDIO_START_REG, MDC_MDIO_START_OP);

    /* Write address to register 23 */
    MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_DUMMY_ID, MDC_MDIO_ADDRESS_REG, mAddrs);

    /* Write Start command to register 29 */
    MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_DUMMY_ID, MDC_MDIO_START_REG, MDC_MDIO_START_OP);

    /* Write read control code to register 21 */
    MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_DUMMY_ID, MDC_MDIO_CTRL1_REG, MDC_MDIO_READ_OP);

    /* Write Start command to register 29 */
    MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_DUMMY_ID, MDC_MDIO_START_REG, MDC_MDIO_START_OP);

    /* Read data from register 25 */
    MDC_MDIO_READ(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_DUMMY_ID, MDC_MDIO_DATA_READ_REG, rData);

    return RT_ERR_OK;

#elif defined(SPI_OPERATION)
    /* Write 8 bits READ OP_CODE */
    SPI_WRITE(SPI_READ_OP, SPI_READ_OP_LEN);

    /* Write 16 bits register address */
    SPI_WRITE(mAddrs, SPI_REG_LEN);

    /* Read 16 bits data */
    SPI_READ(rData, SPI_DATA_LEN);

#else
    rtk_uint32 rawData=0, ACK;
    rtk_uint8  con;
    rtk_uint32 ret = RT_ERR_OK;

    /*Disable CPU interrupt to ensure that the SMI operation is atomic.
      The API is based on RTL865X, rewrite the API if porting to other platform.*/
    rtlglue_drvMutexLock();

    _smi_start();                                /* Start SMI */

    _smi_writeBit(0x0b, 4);                     /* CTRL code: 4'b1011 for RTL8370 */

    _smi_writeBit(0x4, 3);                        /* CTRL code: 3'b100 */

    _smi_writeBit(0x1, 1);                        /* 1: issue READ command */

    con = 0;
    do {
        con++;
        _smi_readBit(1, &ACK);                    /* ACK for issuing READ command*/
    } while ((ACK != 0) && (con < ack_timer));

    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_writeBit((mAddrs&0xff), 8);             /* Set reg_addr[7:0] */

    con = 0;
    do {
        con++;
        _smi_readBit(1, &ACK);                    /* ACK for setting reg_addr[7:0] */
    } while ((ACK != 0) && (con < ack_timer));

    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_writeBit((mAddrs>>8), 8);                 /* Set reg_addr[15:8] */

    con = 0;
    do {
        con++;
        _smi_readBit(1, &ACK);                    /* ACK by RTL8369 */
    } while ((ACK != 0) && (con < ack_timer));
    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_readBit(8, &rawData);                    /* Read DATA [7:0] */
    *rData = rawData&0xff;

    _smi_writeBit(0x00, 1);                        /* ACK by CPU */

    _smi_readBit(8, &rawData);                    /* Read DATA [15: 8] */

    _smi_writeBit(0x01, 1);                        /* ACK by CPU */
    *rData |= (rawData<<8);

    _smi_stop();

    rtlglue_drvMutexUnlock();/*enable CPU interrupt*/

    return ret;
#endif /* end of #if defined(MDC_MDIO_OPERATION) */
}



rtk_int32 smi_write(rtk_uint32 mAddrs, rtk_uint32 rData)
{
#if defined(MDC_MDIO_OPERATION)

    /* Write Start command to register 29 */
    MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_DUMMY_ID, MDC_MDIO_START_REG, MDC_MDIO_START_OP);

    /* Write address control code to register 31 */
    MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_DUMMY_ID, MDC_MDIO_CTRL0_REG, MDC_MDIO_ADDR_OP);

    /* Write Start command to register 29 */
    MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_DUMMY_ID, MDC_MDIO_START_REG, MDC_MDIO_START_OP);

    /* Write address to register 23 */
    MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_DUMMY_ID, MDC_MDIO_ADDRESS_REG, mAddrs);

    /* Write Start command to register 29 */
    MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_DUMMY_ID, MDC_MDIO_START_REG, MDC_MDIO_START_OP);

    /* Write data to register 24 */
    MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_DUMMY_ID, MDC_MDIO_DATA_WRITE_REG, rData);

    /* Write Start command to register 29 */
    MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_DUMMY_ID, MDC_MDIO_START_REG, MDC_MDIO_START_OP);

    /* Write data control code to register 21 */
    MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, MDC_MDIO_DUMMY_ID, MDC_MDIO_CTRL1_REG, MDC_MDIO_WRITE_OP);

    return RT_ERR_OK;

#elif defined(SPI_OPERATION)
    /* Write 8 bits WRITE OP_CODE */
    SPI_WRITE(SPI_WRITE_OP, SPI_WRITE_OP_LEN);

    /* Write 16 bits register address */
    SPI_WRITE(mAddrs, SPI_REG_LEN);

    /* Write 16 bits data */
    SPI_WRITE(rData, SPI_DATA_LEN);
#else

/*
    if ((mAddrs > 0x018A) || (rData > 0xFFFF))  return    RT_ERR_FAILED;
*/
    rtk_int8 con;
    rtk_uint32 ACK;
    rtk_uint32 ret = RT_ERR_OK;

    /*Disable CPU interrupt to ensure that the SMI operation is atomic.
      The API is based on RTL865X, rewrite the API if porting to other platform.*/
       rtlglue_drvMutexLock();

    _smi_start();                                /* Start SMI */

    _smi_writeBit(0x0b, 4);                     /* CTRL code: 4'b1011 for RTL8370*/

    _smi_writeBit(0x4, 3);                        /* CTRL code: 3'b100 */

    _smi_writeBit(0x0, 1);                        /* 0: issue WRITE command */

    con = 0;
    do {
        con++;
        _smi_readBit(1, &ACK);                    /* ACK for issuing WRITE command*/
    } while ((ACK != 0) && (con < ack_timer));
    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_writeBit((mAddrs&0xff), 8);             /* Set reg_addr[7:0] */

    con = 0;
    do {
        con++;
        _smi_readBit(1, &ACK);                    /* ACK for setting reg_addr[7:0] */
    } while ((ACK != 0) && (con < ack_timer));
    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_writeBit((mAddrs>>8), 8);                 /* Set reg_addr[15:8] */

    con = 0;
    do {
        con++;
        _smi_readBit(1, &ACK);                    /* ACK for setting reg_addr[15:8] */
    } while ((ACK != 0) && (con < ack_timer));
    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_writeBit(rData&0xff, 8);                /* Write Data [7:0] out */

    con = 0;
    do {
        con++;
        _smi_readBit(1, &ACK);                    /* ACK for writting data [7:0] */
    } while ((ACK != 0) && (con < ack_timer));
    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_writeBit(rData>>8, 8);                    /* Write Data [15:8] out */

    con = 0;
    do {
        con++;
        _smi_readBit(1, &ACK);                        /* ACK for writting data [15:8] */
    } while ((ACK != 0) && (con < ack_timer));
    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_stop();

    rtlglue_drvMutexUnlock();/*enable CPU interrupt*/

    return ret;
#endif /* end of #if defined(MDC_MDIO_OPERATION) */
}

