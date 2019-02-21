/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id: //WIFI_SOC/TRUNK/RT288x_SDK/source/linux-2.6.21.x/drivers/char/i2s/wm8750.c#11 $
 *
 * Driver for WM8751 audio codec
 *
 * Based on code from the ipodlinux project - http://ipodlinux.org/
 * Adapted for Rockbox in December 2005
 *
 * Original file: linux/arch/armnommu/mach-ipod/audio.c
 *
 * Copyright (c) 2003-2005 Bernard Leach (leachbj@bouncycastle.org)
 *
 * All files in this archive are subject to the GNU General Public License.
 * See the file COPYING in the source tree root for full license agreement.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#if defined(CONFIG_ARCH_MT7623)
#include <linux/autoconf.h>
#else
#include <linux/config.h>
#endif
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/cdev.h>
#if defined(CONFIG_ARCH_MT7623)
#include <mt_i2c.h>
#include <mach/mt_gpio.h>
#endif
#include "i2c_wm8750.h"
#include "../core/i2s_ctrl.h"

#define BUF_SIZE		20

#if defined(CONFIG_ARCH_MT7623)
static struct i2c_board_info __initdata i2c_devs2 = { I2C_BOARD_INFO("codec_wm8750", (0X36>>1))};
#endif
unsigned long wm_reg_data[48];
struct wm8750_data *wmio;

struct wm8750_data {
        struct i2c_client   *client;
        struct device       *dev;
        const char          *name;
};


void i2c_WM8750_write(u32 reg, u32 data)
{
	int ret;
        struct i2c_msg msg;
	u8 buf[2]={0};

#if defined(CONFIG_ARCH_MT7623)
	unsigned int ext_flag = 0;
	
	ext_flag &= 0x7FFFFFFF;
	ext_flag |= I2C_A_FILTER_MSG;
	ext_flag |= I2C_POLLING_FLAG;
#endif

	wm_reg_data[reg] = data;

	buf[0]= (reg<<1)|(0x01&(data>>8));
        buf[1]= (data&0xFF);

        msg.addr = wmio->client->addr;
        msg.flags = 0;
        msg.buf = (char *)buf;
        msg.len = 2;
#if defined(CONFIG_ARCH_MT7623)
	msg.timing = 80;
	msg.ext_flag = ext_flag & 0x7FFFFFFF;
#endif

        ret = i2c_transfer(wmio->client->adapter, &msg, 1);
	MSG("[WM8750(%02X)=0x%08X]\n",(unsigned int)reg,(unsigned int)data);

        if (ret <= 0)
                printk("%s: i2c write error!\n", __func__);
}


/* convert tenth of dB volume (-730..60) to master volume register value */
int tenthdb2master(int db)
{
    /* +6 to -73dB 1dB steps (plus mute == 80levels) 7bits */
    /* 1111111 ==  +6dB  (0x7f)                            */
    /* 1111001 ==   0dB  (0x79)                            */
    /* 0110000 == -73dB  (0x30)                            */
    /* 0101111..0000000 == mute  (<= 0x2f)                 */
    if (db < VOLUME_MIN)
        return 0x0;
    else
        return (db / 10) + 73 + 0x30;
}

static int tone_tenthdb2hw(int value)
{
    /* -6.0db..+0db..+9.0db step 1.5db - translate -60..+0..+90 step 15
        to 10..6..0 step -1.
    */
    value = 10 - (value + 60) / 15;

    if (value == 6)
        value = 0xf; /* 0db -> off */

    return value;
}


#ifdef USE_ADAPTIVE_BASS
static int adaptivebass2hw(int value)
{
    /* 0 to 15 step 1 - step -1  0 = off is a 15 in the register */
    value = 15 - value;

    return value;
}
#endif

/* Reset and power up the WM8751 */
void audiohw_preinit(void)
{
	int i;
	/*
	* 1. Switch on power supplies.
	*    By default the WM8751 is in Standby Mode, the DAC is
	*    digitally muted and the Audio Interface, Line outputs
	*    and Headphone outputs are all OFF (DACMU = 1 Power
	*    Management registers 1 and 2 are all zeros).
	*/
	//if(wm_reg_data[RESET]!=0)
	//	return;
	
	memset(wm_reg_data, 0 , sizeof(unsigned long)*48);
	
	i2c_WM8750_write(RESET, RESET_RESET);    /*Reset*/
	
	for(i = 0; i < 1000*HZ; i++);
	
	wm_reg_data[RESET] = 0xFFFF;
	/* 2. Enable Vmid and VREF. */
	i2c_WM8750_write(PWRMGMT1, PWRMGMT1_VREF | PWRMGMT1_VMIDSEL_50K );
              
}

/* Enable DACs and audio output after a short delay */
void audiohw_postinit(int bSlave, int AIn, int AOut, int wordLen24b)
{
	int i;
	unsigned long data;
	
	if(wm_reg_data[RESET]!=0xFFFF)
    	return;
    
    /* BCLKINV=0(Dont invert BCLK) MS=1(Enable Master) LRSWAP=0 LRP=0 */
    /* IWL=00(16 bit) FORMAT=10(I2S format) */
    
	if(bSlave)
	{ 
		MSG("WM8750 slave.....\n");
		if (wordLen24b)
		{
			printk("24 bit word length\n");
			i2c_WM8750_write(AINTFCE, AINTFCE_WL_24 | AINTFCE_FORMAT_I2S);
		}
		else
		{
			printk("16 bit word length\n");
			i2c_WM8750_write(AINTFCE, AINTFCE_WL_16 | AINTFCE_FORMAT_I2S);
		}
	}	
	else
	{
		MSG("WM8750 master.....\n");
		/* AINTFCE_BCLKINV on or off depend on trying result */
		if (wordLen24b)
		{
			printk("24 bit word length\n");
			i2c_WM8750_write(AINTFCE, AINTFCE_MS | AINTFCE_WL_24 | AINTFCE_FORMAT_I2S);
		}
		else
		{
			printk("16 bit word length\n");
			i2c_WM8750_write(AINTFCE, AINTFCE_MS | AINTFCE_WL_16 | AINTFCE_FORMAT_I2S);	
		}
	}
    
	wm_reg_data[RESET] = 0x5A5A;
	
	/* From app notes: allow Vref to stabilize to reduce clicks */
	for(i = 0; i < 1000*HZ; i++);
    
	data = wm_reg_data[PWRMGMT1];
    
	if(AIn > 0)
		i2c_WM8750_write(PWRMGMT1, data | PWRMGMT1_ADCL | PWRMGMT1_ADCR | PWRMGMT1_AINL | PWRMGMT1_AINR |PWRMGMT1_MICB);
		//i2c_WM8750_write(PWRMGMT1, PWRMGMT1_VREF | PWRMGMT1_VMIDSEL_50K | PWRMGMT1_ADCL | PWRMGMT1_ADCR | PWRMGMT1_AINL | PWRMGMT1_AINR );
		
	/* 3. Enable DACs as required. */
	if(AOut > 0)
	{
		data = PWRMGMT2_DACL | PWRMGMT2_DACR;
		switch(AOut)
		{
		case 1:
			data |= PWRMGMT2_LOUT1 | PWRMGMT2_ROUT1;
			break;
		case 2:
			data |= PWRMGMT2_LOUT2 | PWRMGMT2_ROUT2;
			break;
		case 3:
			data |= PWRMGMT2_OUT3;
			break;
		default:
			break;	
		}	
		i2c_WM8750_write(PWRMGMT2,   data);
	}
	else
	{
		i2c_WM8750_write(PWRMGMT2, 0);
	}
		 
	/* 4. Enable line and / or headphone output buffers as required. */
	i2c_WM8750_write(ADDITIONAL1, ADDITIONAL1_TOEN | ADDITIONAL1_DMONOMIX_LLRR | ADDITIONAL1_VSEL_DEFAULT );
    
	if(AOut==2) 
	{               
		i2c_WM8750_write(ADDITIONAL2, ADDITIONAL2_HPSWEN|ADDITIONAL2_HPSWPOL);
	}
	else
	{
		i2c_WM8750_write(ADDITIONAL2, ADDITIONAL2_LRCM_ON);
	}
	
	i2c_WM8750_write(ADDITIONAL3, ADDITIONAL3_ADCLRM(0));
	
	i2c_WM8750_write(RIGHTMIX1, 0);
	i2c_WM8750_write(LEFTMIX2, 0);
	if(AOut > 0)
	{
		i2c_WM8750_write(LEFTMIX1, LEFTMIX1_LD2LO);
		i2c_WM8750_write(RIGHTMIX2, RIGHTMIX2_RD2RO);
		i2c_WM8750_write(LOUT1, 0x179);
		i2c_WM8750_write(ROUT1, 0x179);
		i2c_WM8750_write(LEFTGAIN, 0x1FF);
		i2c_WM8750_write(RIGHTGAIN, 0x1FF);
	}
	else
	{
		i2c_WM8750_write(LEFTMIX1, 0);
		i2c_WM8750_write(RIGHTMIX2, 0);
		i2c_WM8750_write(LOUT1, 0);
		i2c_WM8750_write(ROUT1, 0);
	}
	i2c_WM8750_write(MONOMIX1, 0);
	i2c_WM8750_write(MONOMIX2, 0);
	i2c_WM8750_write(ADCCTRL, ADCCTRL_ADCHPD);
	if(AIn>0)
	{
		//i2c_WM8750_write(LINV, LINV_LINVOL(0x00)|LINV_LIVU);
		//i2c_WM8750_write(RINV, RINV_RINVOL(0x00)|RINV_RIVU);
		//i2c_WM8750_write(LINV, 0x117);
		//i2c_WM8750_write(RINV, 0x117);
		i2c_WM8750_write(LINV, 0x100);
		i2c_WM8750_write(RINV, 0x100);
		
		i2c_WM8750_write(ALC1, 0);
		i2c_WM8750_write(NOISEGATE, 0x03);
		//i2c_WM8750_write(ALC1, ALC1_ALCL(0x01)|ALC1_SET_MAXGAIN(0x1)|ALC1_ALCSTEREO);
		//i2c_WM8750_write(NOISEGATE, NOISEGATE_SET_NGTH(0x1F)|NOISEGATE_SET_NGG(0x01)|NOISEGATE_NGAT_ENABLE);
		
		audiohw_sel_input(AIn);
	}
	else
	{
		i2c_WM8750_write(LINV, LINV_LINMUTE);
		i2c_WM8750_write(RINV, RINV_RINMUTE);
	}
	
	//i2c_WM8750_write(LEFTGAIN, 0x1FF);
	//i2c_WM8750_write(RIGHTGAIN, 0x1FF);
	if(AOut > 0)
		audiohw_mute(false);
	else
		audiohw_mute(true);
}
int audiohw_set_master_vol(int vol_l, int vol_r)
{
	/* +6 to -73dB 1dB steps (plus mute == 80levels) 7bits */
	/* 1111111 ==  +6dB                                    */
	/* 1111001 ==   0dB                                    */
	/* 0110000 == -73dB                                    */
	/* 0101111 == mute (0x2f)                              */
	//MSG("audiohw_set_master_vol\n");
	i2c_WM8750_write(LOUT1, LOUT1_BITS | LOUT1_LOUT1VOL(0x79));
	i2c_WM8750_write(ROUT1, ROUT1_BITS | ROUT1_ROUT1VOL(0x79));
	
	i2c_WM8750_write(LOUT2, LOUT2_BITS | LOUT2_LOUT2VOL(0x79));
	i2c_WM8750_write(ROUT2, ROUT2_BITS | ROUT2_ROUT2VOL(0x79));

	i2c_WM8750_write(LEFTGAIN, LEFTGAIN_LDVU | LEFTGAIN_LDACVOL(vol_l));
	i2c_WM8750_write(RIGHTGAIN, RIGHTGAIN_LDVU | RIGHTGAIN_LDACVOL(vol_r));
	return 0;
}

int audiohw_set_lineout_vol(int Aout, int vol_l, int vol_r)
{
	MSG("audiohw_set_lineout_vol\n");
	switch(Aout)
	{
	case 1:
		i2c_WM8750_write(LOUT1, LOUT1_BITS | LOUT1_LOUT1VOL(vol_l));
    		i2c_WM8750_write(ROUT1, ROUT1_BITS | ROUT1_ROUT1VOL(vol_r));
		break;
	case 2:
    		i2c_WM8750_write(LOUT2, LOUT2_BITS | LOUT2_LOUT2VOL(vol_l));
    		i2c_WM8750_write(ROUT2, ROUT2_BITS | ROUT2_ROUT2VOL(vol_r));
		break;
	case 3:
		i2c_WM8750_write(MONOOUT, MONOOUT_MOZC | MONOOUT_MONOOUTVOL(vol_l));	
		break;
	}	
    return 0;
}

int audiohw_set_linein_vol(int vol_l, int vol_r)
{
	MSG("audiohw_set_linein_vol\n");
    	i2c_WM8750_write(LINV, LINV_LINVOL(vol_l)|LINV_LIVU);
	i2c_WM8750_write(RINV, RINV_RINVOL(vol_r)|RINV_RIVU);
    	return 0;
}

void audiohw_set_bass(int value)
{
	i2c_WM8750_write(BASSCTRL, BASSCTRL_BITS |

#ifdef USE_ADAPTIVE_BASS
	BASSCTRL_BASS(adaptivebass2hw(value)));
#else
	BASSCTRL_BASS(tone_tenthdb2hw(value)));
#endif
}

void audiohw_set_treble(int value)
{
    	i2c_WM8750_write(TREBCTRL, TREBCTRL_BITS | TREBCTRL_TREB(tone_tenthdb2hw(value)));
}

void audiohw_mute(bool mute)
{
   	/* Mute:   Set DACMU = 1 to soft-mute the audio DACs. */
    	/* Unmute: Set DACMU = 0 to soft-un-mute the audio DACs. */
    	i2c_WM8750_write(DACCTRL, mute ? DACCTRL_DACMU : 0);
}

/* Nice shutdown of WM8751 codec */
void audiohw_close(void)
{
	/* 1. Set DACMU = 1 to soft-mute the audio DACs. */
	audiohw_mute(true);
	
	i2c_WM8750_write(LADCVOL, 0);
	i2c_WM8750_write(RADCVOL, 0);
	
	/* 2. Disable all output buffers. */
	i2c_WM8750_write(PWRMGMT2, 0x0);
	
	/* 3. Switch off the power supplies. */
	i2c_WM8750_write(PWRMGMT1, 0x0);
	
	memset(wm_reg_data, 0 , sizeof(unsigned long)*48);
}

/* Note: Disable output before calling this function */
void audiohw_set_frequency(int fsel)
{
	MSG("audiohw_set_frequency=0x%08X\n",fsel);
	//fsel |= 0x0180;	// BCLK = MCLK/16
	//fsel |= 0x0080;
	//fsel &= 0xFFFFFFFE;
	i2c_WM8750_write(CLOCKING, fsel);	
} 

void audiohw_set_MCLK(unsigned int bUsb)
{
	if(bUsb)
		i2c_WM8750_write(CLOCKING, CLOCKING_SR_USB);
	else
		i2c_WM8750_write(CLOCKING, 0);		
} 

void audiohw_sel_input(int ain)
{
	switch(ain)
	{
	case 1:
		i2c_WM8750_write(ADCLPATH, ADCLPATH_LINSEL_IN1);
		i2c_WM8750_write(ADCRPATH, ADCRPATH_RINSEL_IN1);
		//i2c_WM8750_write(ADCINMODE, ADCINMODE_RDCM_EN|ADCINMODE_LDCM_EN);	
		break;
	case 2:
		i2c_WM8750_write(ADCLPATH, ADCLPATH_LINSEL_IN2);
		i2c_WM8750_write(ADCRPATH, ADCRPATH_RINSEL_IN2);
		//i2c_WM8750_write(ADCLPATH, ADCLPATH_LINSEL_DS);
		//i2c_WM8750_write(ADCRPATH, ADCRPATH_RINSEL_DS);
		//i2c_WM8750_write(ADCINMODE, ADCINMODE_DS_IN2|ADCINMODE_RDCM_EN|ADCINMODE_LDCM_EN);						
		break;
	case 3:
		i2c_WM8750_write(ADCLPATH, ADCLPATH_LINSEL_IN3);
		i2c_WM8750_write(ADCRPATH, ADCRPATH_RINSEL_IN3);			
		break;	
	default:
		i2c_WM8750_write(ADCLPATH, ADCLPATH_LINSEL_IN1);
		i2c_WM8750_write(ADCRPATH, ADCRPATH_RINSEL_IN1);			
		break;		
	}
	return;
}

void audiohw_loopback(int fsel)
{
	int i;
	memset(wm_reg_data, 0 , sizeof(unsigned long)*48);
	
	i2c_WM8750_write(RESET, 0x000);    /*Reset*/
	  
	for(i = 0; i < 1000*HZ; i++);
	
	//i2c_WM8750_write(ADDITIONAL2, 0x0010);
	//i2c_WM8750_write(PWRMGMT1, 0x0FC );
	
	
	i2c_WM8750_write(AINTFCE, AINTFCE_WL_16|AINTFCE_FORMAT_I2S|AINTFCE_MS);
	i2c_WM8750_write(LINV, 0x117);
	i2c_WM8750_write(RINV, 0x117);     
	i2c_WM8750_write(LOUT1, 0x179);
	i2c_WM8750_write(ROUT1, 0x179);
	i2c_WM8750_write(ADCCTRL, 0);
	i2c_WM8750_write(CLOCKING, fsel);
	i2c_WM8750_write(LEFTGAIN, 0x01FF);   
	i2c_WM8750_write(RIGHTGAIN, 0x01FF);   
	i2c_WM8750_write(PWRMGMT1, 0x0FC );            
	i2c_WM8750_write(PWRMGMT2, 0x1E0 );
	i2c_WM8750_write(LEFTMIX1, 0x150 );
	i2c_WM8750_write(RIGHTMIX2, 0x150 );
}	

void audiohw_bypass(void)
{
	int i;
	memset(wm_reg_data, 0 , sizeof(unsigned long)*48);
	i2c_WM8750_write(RESET, 0x000);    /*Reset*/
	  
	for(i = 0; i < 1000*HZ; i++);
	
	i2c_WM8750_write(LINV, 0x117);
	i2c_WM8750_write(RINV, 0x117);  
	i2c_WM8750_write(LOUT1, 0x179);
	i2c_WM8750_write(ROUT1, 0x179);
	i2c_WM8750_write(DACCTRL, 0);
	i2c_WM8750_write(AINTFCE, 0x42);
	i2c_WM8750_write(CLOCKING, 0x23);            
	i2c_WM8750_write(LEFTGAIN, 0x1FF);
	i2c_WM8750_write(RIGHTGAIN, 0x1FF);
	i2c_WM8750_write(PWRMGMT1, 0x0F0);
	i2c_WM8750_write(PWRMGMT2, 0x060);
	i2c_WM8750_write(LEFTMIX1, 0xA0 );
	i2c_WM8750_write(RIGHTMIX2, 0xA0 );
}	


EXPORT_SYMBOL(audiohw_set_frequency);
EXPORT_SYMBOL(audiohw_close);
EXPORT_SYMBOL(audiohw_postinit);
EXPORT_SYMBOL(audiohw_preinit);
EXPORT_SYMBOL(audiohw_set_master_vol);
EXPORT_SYMBOL(audiohw_set_lineout_vol);
EXPORT_SYMBOL(audiohw_set_linein_vol);
EXPORT_SYMBOL(audiohw_set_bass);
EXPORT_SYMBOL(audiohw_set_treble);
EXPORT_SYMBOL(audiohw_mute);
EXPORT_SYMBOL(audiohw_set_MCLK);
EXPORT_SYMBOL(audiohw_sel_input);
EXPORT_SYMBOL(audiohw_loopback);
EXPORT_SYMBOL(audiohw_bypass);


static int codec_wm8750_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
        struct wm8750_data *wm;

	printk("*******Enter %s********\n", __func__);

        if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
                return -EIO;

        wm = devm_kzalloc(&client->dev, sizeof(struct wm8750_data), GFP_KERNEL);
        if (!wm)
                return -ENOMEM;

#if defined(CONFIG_ARCH_MT7623)
	mt_set_gpio_mode(GPIO242, GPIO_MODE_04);
	mt_set_gpio_mode(GPIO243, GPIO_MODE_04);
#endif

        wm->client = client;
        wm->dev = &client->dev;
        wm->name = id->name;
        i2c_set_clientdata(client, wm);
        wmio = wm;

        memset(wm_reg_data, 0 , sizeof(unsigned long)*48);

        return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0)
static int codec_wm8750_i2c_remove(struct i2c_client *client)
#else
static int __devexit codec_wm8750_i2c_remove(struct i2c_client *client)
#endif
{
        struct wm8750_data *wm = i2c_get_clientdata(client);
	kfree(wm);

        return 0;
}

static const struct i2c_device_id wm8750_id[] = {
        { "codec_wm8750", 0 },
        {}
};


static struct i2c_driver codec_wm8750_i2c_driver = {
        .driver = {
                .name   = "codec_wm8750"
        },
        .probe  = codec_wm8750_i2c_probe,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0)
        .remove = codec_wm8750_i2c_remove,
#else
        .remove = __devexit_p(codec_wm8750_i2c_remove),
#endif
        .id_table = wm8750_id,
};

static int __init wm8750_i2c_init(void)
{
#if defined(CONFIG_ARCH_MT7623)
	i2c_register_board_info(1, &i2c_devs2, 1);
#endif
        return i2c_add_driver(&codec_wm8750_i2c_driver);;
}

static void __exit wm8750_i2c_exit(void)
{
        i2c_del_driver(&codec_wm8750_i2c_driver);
}

module_init(wm8750_i2c_init);
module_exit(wm8750_i2c_exit);

MODULE_AUTHOR("Dora Chen <dora.chen@mediatek.com>");
MODULE_DESCRIPTION("WM8750 I2C client driver");


