#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/init.h>
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
#include "i2c_wm8960.h"
#include "../core/i2s_ctrl.h"


#define BUF_SIZE		20


#if defined(CONFIG_ARCH_MT7623)
/*FIXME*/
//static struct i2c_board_info __initdata i2c_devs1 = { I2C_BOARD_INFO("codec_wm8960", (0X34>>1))};
static struct i2c_board_info __initdata i2c_devs1 = { I2C_BOARD_INFO("codec_wm8960", (0X34))};

#endif
unsigned long wm_reg_data[56];
struct wm8960_data *wmio;

struct wm8960_data {
	struct i2c_client	*client;
	struct device	    *dev;
	const char 			*name;
};


void i2c_WM8960_write(u32 reg, u32 data)
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

#if defined(CONFIG_ARCH_MT7623)
	/*FIXME*/
	//msg.addr = wmio->client->addr;
	msg.addr = wmio->client->addr>>1;

#else	
        msg.addr = wmio->client->addr>>1;
#endif
     	msg.flags = 0;
        msg.buf = (char *)buf;
	msg.len = 2;
#if defined(CONFIG_ARCH_MT7623)
	msg.timing = 80;
	msg.ext_flag = ext_flag & 0x7FFFFFFF;
#endif

        ret = i2c_transfer(wmio->client->adapter, &msg, 1);
	MSG("[WM8960(%02X)=0x%08X]\n",(unsigned int)reg,(unsigned int)data);	

        if (ret <= 0)
                printk("%s: i2c write error!\n", __func__);
}



// Reset and power up the WM8960 
void audiohw_preinit(void)
{
	memset(wm_reg_data, 0 , sizeof(unsigned long)*55);

	i2c_WM8960_write(RESET, RESET_RESET);    // Reset (0x0F)
	
 	mdelay(50);	
	wm_reg_data[RESET] = 0xFFFF;
	mdelay(50);	
}

void audiohw_set_apll(int srate)
{
	unsigned long data;

	if((srate==8000) || (srate==12000) || (srate==16000) || (srate==24000) || (srate==32000) || (srate==48000))
	{
		// Provide 12.288MHz SYSCLK 
		data = wm_reg_data[PLL1];	
        	i2c_WM8960_write(PLL1, data | PLL1_OPCLKDIV_1 | PLL1_SDM_FRACTIONAL | PLL1_PLLPRESCALE_1 | PLL1_PLLN(0x8));   // PLL1 (0x34)
        	
		i2c_WM8960_write(PLL2, PLL2_PLLK_23_16(0x31));  // PLL2 (0x35)
        	i2c_WM8960_write(PLL3, PLL3_PLLK_15_8(0x26));  // PLL3 (0x36)
		i2c_WM8960_write(PLL4, PLL4_PLLK_7_0(0xe9));  // PLL4 (0x37)
	}
	else if ((srate==11025) || (srate==22050) || (srate==44100))
	{
		//Provide 11.2896MHz SYSCLK 
		data = wm_reg_data[PLL1];	
        	i2c_WM8960_write(PLL1, data | PLL1_OPCLKDIV_1 | PLL1_SDM_FRACTIONAL | PLL1_PLLPRESCALE_1 | PLL1_PLLN(0x7));   //PLL1 (0x34)
        	
		i2c_WM8960_write(PLL2, PLL2_PLLK_23_16(0x86));  //PLL2 (0x35) 
        	i2c_WM8960_write(PLL3, PLL3_PLLK_15_8(0xc2));  //PLL3 (0x36)
		i2c_WM8960_write(PLL4, PLL4_PLLK_7_0(0x26));  //PLL4 (0x37)
	}
	else
	{
		printk("Not support this srate\n");
	}
	mdelay(3);
}


void audiohw_set_frequency(int fsel, int pll_en)
{
        MSG("audiohw_set_frequency_=0x%08X\n",fsel);

	if (pll_en)
	{
		printk("PLL enable\n");
		i2c_WM8960_write(CLOCKING1, (fsel<<3) | CLOCKING1_SYSCLKDIV_2 | CLOCKING1_CLKSEL_PLL);  //CLOCKING (0x04)=>0x05 

	}
	else
	{
		printk("PLL disable\n");
		i2c_WM8960_write(CLOCKING1, (fsel<<3));//| CLOCKING1_SYSCLKDIV_2);  //CLOCKING (0x04) 
	}
	
}

//FIXME 
int audiohw_set_lineout_vol(int Aout, int vol_l, int vol_r)
{
	MSG("audiohw_set_lineout_vol_\n");
	switch(Aout)
	{
	case 1:
		//i2c_WM8960_write(LOUT1, LOUT1_LO1VU|LOUT1_LO1ZC|LOUT1_LOUT1VOL(0x7f)); //LOUT1(0x02) 
		//i2c_WM8960_write(ROUT1, ROUT1_RO1VU|ROUT1_RO1ZC|ROUT1_ROUT1VOL(0x7f)); //ROUT1(0x03) 
		i2c_WM8960_write(LOUT1, LOUT1_LO1VU|LOUT1_LO1ZC|LOUT1_LOUT1VOL(vol_l)); //LOUT1(0x02) 
		i2c_WM8960_write(ROUT1, ROUT1_RO1VU|ROUT1_RO1ZC|ROUT1_ROUT1VOL(vol_r)); //ROUT1(0x03) 
		break;
	case 2:
    		i2c_WM8960_write(LSPK, LSPK_SPKLVU|LSPK_SPKLZC| LSPK_SPKLVOL(vol_l));
    		i2c_WM8960_write(RSPK, RSPK_SPKRVU|RSPK_SPKRZC| RSPK_SPKRVOL(vol_r));
		break;
	default:
		break;
	}	
    	return 0;
}

//FIXME 
int audiohw_set_linein_vol(int vol_l, int vol_r)
{
	MSG("audiohw_set_linein_vol_\n");
	
    i2c_WM8960_write(LINV, LINV_IPVU|LINV_LINVOL(vol_l)); //LINV(0x00)=>0x12b 
	i2c_WM8960_write(RINV, RINV_IPVU|RINV_RINVOL(vol_r)); //LINV(0x01)=>0x12b 

    	return 0;
}

//Set signal path
int audiohw_postinit(int bSlave, int AIn, int AOut, int pll_en, int wordLen24b)
{

	int i;
	unsigned long data;

	if(wm_reg_data[RESET]!=0xFFFF)
    	return 0;
	
	if(bSlave)
	{ 
		MSG("WM8960 slave.....\n");
		if(wordLen24b)
		{
			printk("24 bit word length\n");
			i2c_WM8960_write(AINTFCE1, AINTFCE1_WL_24 | AINTFCE1_FORMAT_I2S); //AINTFCE1(0x07) 
		}
		else
		{
			printk("16 bit word length\n");
			i2c_WM8960_write(AINTFCE1, AINTFCE1_WL_16 | AINTFCE1_FORMAT_I2S); //AINTFCE1(0x07) 
		}
	}	
	else
	{
		MSG("WM8960 master.....\n");
		i2c_WM8960_write(CLOCKING2, 0x1c4);//CLOCKING2_BCLKDIV(0x1c4));  //CLOCKING2(0x08) 

		if(wordLen24b)
		{
			printk("24 bit word length\n");
			i2c_WM8960_write(AINTFCE1, AINTFCE1_MS | AINTFCE1_WL_24 | AINTFCE1_FORMAT_I2S); //AINTFCE1(0x07) 
		}
		else
		{
			printk("16 bit word length\n");
			i2c_WM8960_write(AINTFCE1, AINTFCE1_MS | AINTFCE1_WL_16 | AINTFCE1_FORMAT_I2S); //AINTFCE1(0x07) 
		}
		mdelay(5);
	}

	
	//From app notes: allow Vref to stabilize to reduce clicks 
	for(i = 0; i < 1000*HZ; i++);
	
	if(AIn > 0)
	{
       		data = wm_reg_data[PWRMGMT1];
   		i2c_WM8960_write(PWRMGMT1, data|PWRMGMT1_ADCL|PWRMGMT1_ADCR|PWRMGMT1_AINL |PWRMGMT1_AINR);//|PWRMGMT1_MICB);//PWRMGMT1(0x19) 

		data = wm_reg_data[ADDITIONAL1];
		i2c_WM8960_write(ADDITIONAL1, data|ADDITIONAL1_DATSEL(0x01)); //ADDITIONAL1(0x17) 
		i2c_WM8960_write(LADCVOL, LADCVOL_LAVU_EN|LADCVOL_LADCVOL(0xc3)); //LADCVOL(0x15) 
		i2c_WM8960_write(RADCVOL, RADCVOL_RAVU_EN|RADCVOL_RADCVOL(0xc3)); //RADCVOL(0x16) 
		i2c_WM8960_write(ADCLPATH, ADCLPATH_LMN1|ADCLPATH_LMIC2B);//|ADCLPATH_LMICBOOST_13DB); //ADCLPATH(0x20)=>(0x108)
		i2c_WM8960_write(ADCRPATH, ADCRPATH_RMN1|ADCRPATH_RMIC2B);//|ADCRPATH_RMICBOOST_13DB); //ADCRPATH(0x21)=>(0x108)
		i2c_WM8960_write(PWRMGMT3, PWRMGMT3_LMIC|PWRMGMT3_RMIC); //PWRMGMT3(0x2f) 
	
		//i2c_WM8960_write(LINBMIX, 0x000); //LINBMIX(0x2B) 
		
		if (AOut<=0)
		{
			i2c_WM8960_write(AINTFCE2, 0x40); //FIXME:(0x09) 

			data = wm_reg_data[PWRMGMT2];
			if(pll_en)
			{
				i2c_WM8960_write(PWRMGMT2, data|PWRMGMT2_PLL_EN|PWRMGMT2_DACL|PWRMGMT2_DACR); //PWRMGMT2(0x1a) 
			}
			else
			{
				i2c_WM8960_write(PWRMGMT2, data|PWRMGMT2_DACL|PWRMGMT2_DACR); //PWRMGMT2(0x1a) 

			}
		}
	}
	if(AOut>0)
	{
		//Power management 2 setting 
		data = wm_reg_data[PWRMGMT2];

		if(pll_en)
		{
			i2c_WM8960_write(PWRMGMT2, data|PWRMGMT2_PLL_EN|PWRMGMT2_DACL|PWRMGMT2_DACR|PWRMGMT2_LOUT1|PWRMGMT2_ROUT1|PWRMGMT2_SPKL|PWRMGMT2_SPKR); //PWRMGMT2(0x1a) 
		}
		else
		{
			i2c_WM8960_write(PWRMGMT2, data|PWRMGMT2_DACL|PWRMGMT2_DACR|PWRMGMT2_LOUT1|PWRMGMT2_ROUT1|PWRMGMT2_SPKL|PWRMGMT2_SPKR); //PWRMGMT2(0x1a) 

		}
		
		mdelay(10);

		i2c_WM8960_write(AINTFCE2, 0x40); //FIXME:(0x09) 

		i2c_WM8960_write(LEFTGAIN, LEFTGAIN_LDVU|LEFTGAIN_LDACVOL(0xff)); //LEFTGAIN(0x0a) 
		i2c_WM8960_write(RIGHTGAIN, RIGHTGAIN_RDVU|RIGHTGAIN_RDACVOL(0xff)); //RIGHTGAIN(0x0b)

		i2c_WM8960_write(LEFTMIX1, 0x100);  //LEFTMIX1(0x22) 
		i2c_WM8960_write(RIGHTMIX2, 0x100); //RIGHTMIX2(0x25) 

		data = wm_reg_data[PWRMGMT3]; //FIXME
		i2c_WM8960_write(PWRMGMT3, data|PWRMGMT3_ROMIX|PWRMGMT3_LOMIX); //PWRMGMT3(0x2f) 

		data = wm_reg_data[CLASSDCTRL1]; //CLASSDCTRL1(0x31) SPEAKER FIXME
		i2c_WM8960_write(CLASSDCTRL1, 0xf7);//data|CLASSDCTRL1_OP_LRSPK);

		data = wm_reg_data[CLASSDCTRL3];	//CLASSDCTRL3(0x33) 
		i2c_WM8960_write(CLASSDCTRL3, 0xad);//data|(0x1b));
	}

	i2c_WM8960_write(DACCTRL1, 0x000);  //DACCTRL1(0x05) 

	data = wm_reg_data[PWRMGMT1];
	i2c_WM8960_write(PWRMGMT1, data|0x1c0); //FIXME:PWRMGMT1(0x19)
	

	printk("WM8960 All initial ok!\n");

	return 0;
	
}

void audiohw_micboost(int boostgain)
{
	unsigned long data;

	data =  wm_reg_data[ADCLPATH];
	i2c_WM8960_write(ADCLPATH, data|(boostgain << 4));

	data =  wm_reg_data[ADCRPATH];
	i2c_WM8960_write(ADCRPATH, data|(boostgain << 4));
}

void audiohw_micin(int enableMic)
{
	unsigned long data;
	
	if (enableMic==1)
	{
		data =  wm_reg_data[PWRMGMT1];
		i2c_WM8960_write(PWRMGMT1, data|PWRMGMT1_MICB);
	}
#if 1
	else
	{
		data =  wm_reg_data[PWRMGMT1];
		i2c_WM8960_write(PWRMGMT1, data & (~(PWRMGMT1_MICB)));
	}
#endif
}

void audiohw_mute( bool mute)
{
    //Mute:   Set DACMU = 1 to soft-mute the audio DACs. 
    //Unmute: Set DACMU = 0 to soft-un-mute the audio DACs. 
    i2c_WM8960_write(DACCTRL1, mute ? DACCTRL1_DACMU : 0);
}


//Nice shutdown of WM8960 codec 
void audiohw_close(void)
{
	i2c_WM8960_write(DACCTRL1,DACCTRL1_DACMU); //0x05->0x08 
	i2c_WM8960_write(PWRMGMT1, 0x000); //0x19->0x000 
	mdelay(400);
	i2c_WM8960_write(PWRMGMT2, 0x000); //0x1a->0x000 

}

void audiohw_loopback(int fsel)
{
}

void audiohw_codec_exlbk(void)
{
	memset(wm_reg_data, 0 , sizeof(unsigned long)*55);

	i2c_WM8960_write(LINV, 0x117); //0x00->0x117 
	i2c_WM8960_write(RINV, 0x117); //0x01->0x117 
	i2c_WM8960_write(LOUT1, 0x179); //0x02->0x179 
	i2c_WM8960_write(ROUT1, 0x179); //0x03->0x179 
	i2c_WM8960_write(CLOCKING1, 0x00); //0x04->0x00 
	//i2c_WM8960_write(CLOCKING1, 0x40); //0x04->0x00 
	i2c_WM8960_write(DACCTRL1, 0x00); //0x05->0x00 
	i2c_WM8960_write(AINTFCE2, 0x41); //0x09->0x41 
	i2c_WM8960_write(LADCVOL, 0x1c3); //0x15->0x1c3 
	i2c_WM8960_write(RADCVOL, 0x1c3); //0x16->0x1c3 
	i2c_WM8960_write(PWRMGMT1, 0xfc); //0x19->0xfc 
	i2c_WM8960_write(PWRMGMT2, 0x1e0); //0x1a->0x1e0 
	i2c_WM8960_write(ADCLPATH, 0x108); //0x20->0x108 
	i2c_WM8960_write(ADCRPATH, 0x108); //0x21->0x108 
	i2c_WM8960_write(LEFTMIX1, 0x150); //0x22->0x150 
	i2c_WM8960_write(RIGHTMIX2, 0x150); //0x25->0x150 
	i2c_WM8960_write(BYPASS1, 0x00); //0x2d->0x00 
	i2c_WM8960_write(BYPASS2, 0x00); //0x2e->0x00 
	i2c_WM8960_write(PWRMGMT3, 0x3c); //0x2f->0x3c 
}

void audiohw_bypass(void)
{
	int i;

	memset(wm_reg_data, 0 , sizeof(unsigned long)*55);
	i2c_WM8960_write(RESET, 0x000);    //0x0f(R15)->0x000 
	
	for(i = 0; i < 1000*HZ; i++);

	i2c_WM8960_write(PWRMGMT1, 0xf0); //0x19(R25)->0xf0 
	i2c_WM8960_write(PWRMGMT2, 0x60); //0x1a(R26)->0x60 
	i2c_WM8960_write(PWRMGMT3, 0x3c); //0x2f(R47)->0x3c 
	i2c_WM8960_write(LINV, 0x117); // 0x00(R0)->0x117 
	i2c_WM8960_write(RINV, 0x117); // 0x01(R1)->0x117 
	i2c_WM8960_write(ADCLPATH, 0x108); //0x20(R32)->0x108 
	i2c_WM8960_write(ADCRPATH, 0x108); //0x21(R33)->0x108 
	i2c_WM8960_write(BYPASS1, 0x80); //0x2d(R45)->0x80 
	i2c_WM8960_write(BYPASS2, 0x80); //0x2e(R46)->0x80 
	i2c_WM8960_write(LOUT1, 0x179); // 0x02(R2)->0x179 
	i2c_WM8960_write(ROUT1, 0x179); // 0x03(R3)->0x179 
}
EXPORT_SYMBOL(audiohw_set_frequency);
EXPORT_SYMBOL(audiohw_close);
EXPORT_SYMBOL(audiohw_postinit);
EXPORT_SYMBOL(audiohw_preinit);
EXPORT_SYMBOL(audiohw_set_apll);
EXPORT_SYMBOL(audiohw_codec_exlbk);
EXPORT_SYMBOL(audiohw_bypass);
EXPORT_SYMBOL(audiohw_set_lineout_vol);
EXPORT_SYMBOL(audiohw_set_linein_vol);
EXPORT_SYMBOL(audiohw_micin);
EXPORT_SYMBOL(audiohw_mute);
EXPORT_SYMBOL(audiohw_loopback);
EXPORT_SYMBOL(audiohw_micboost);

static int codec_wm8960_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct wm8960_data *wm;

printk("*******Enter %s********\n", __func__);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
		return -EIO;
	
	wm = devm_kzalloc(&client->dev, sizeof(struct wm8960_data), GFP_KERNEL);	
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

	memset(wm_reg_data, 0 , sizeof(unsigned long)*55);
	
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0)
static int codec_wm8960_i2c_remove(struct i2c_client *client)
#else
static int __devexit codec_wm8960_i2c_remove(struct i2c_client *client)
#endif
{
	struct wm8960_data *wm = i2c_get_clientdata(client);
	kfree(wm);

	return 0;
}

static const struct i2c_device_id wm8960_id[] = {
	{ "codec_wm8960", 0 },
	{}
};

static struct i2c_driver codec_wm8960_i2c_driver = {
	.driver = {
		.name	= "codec_wm8960"               
	},
	.probe	= codec_wm8960_i2c_probe,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0)
        .remove = codec_wm8960_i2c_remove,
#else
	.remove	= __devexit_p(codec_wm8960_i2c_remove),
#endif
	.id_table = wm8960_id,
};
static int __init wm8960_i2c_init(void)
{
#if defined(CONFIG_ARCH_MT7623)
	i2c_register_board_info(1, &i2c_devs1, 1);
#endif	
	return i2c_add_driver(&codec_wm8960_i2c_driver);; 
}

static void __exit wm8960_i2c_exit(void)
{
	i2c_del_driver(&codec_wm8960_i2c_driver);
}

module_init(wm8960_i2c_init);
module_exit(wm8960_i2c_exit);

MODULE_AUTHOR("Ryder Lee <ryder.lee@mediatek.com>");
MODULE_DESCRIPTION("WM8960 I2C client driver");
MODULE_LICENSE("GPL");

