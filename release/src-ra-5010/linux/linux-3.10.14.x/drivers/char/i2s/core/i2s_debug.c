#include <linux/init.h>
#include <linux/version.h>
#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include "i2s_ctrl.h"
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <asm/uaccess.h> /* copy_from/to_user */

#if defined(CONFIG_SND_RALINK_SOC)
#include <sound/soc/mtk/mtk_audio_device.h>
#endif

#if defined(CONFIG_I2S_WM8750)
#include "../codec/i2c_wm8750.h"
#endif
#if defined(CONFIG_I2S_WM8751)
#include "../codec/i2c_wm8751.h"
#endif
#if defined(CONFIG_I2S_WM8960)
#include "../codec/i2c_wm8960.h"
#endif


//#define INTERNAL_LOOPBACK_DEBUG

extern unsigned long i2s_codec_12p288Mhz[11];
extern unsigned long i2s_codec_12Mhz[11]; 
#if defined(CONFIG_RALINK_MT7628) || defined(CONFIG_ARCH_MT7623)
extern unsigned long i2s_inclk_int_16bit[13];
extern unsigned long i2s_inclk_comp_16bit[13];
extern unsigned long i2s_inclk_int_24bit[13];
extern unsigned long i2s_inclk_comp_24bit[13];
#else
extern unsigned long i2s_inclk_int[11];
extern unsigned long i2s_inclk_comp[11];
#endif
extern int i2s_pll_config_mt7621(unsigned long index);
extern int i2s_pll_config_mt7623(unsigned long index);

#if defined(CONFIG_I2S_WM8960) || defined(CONFIG_I2S_WM8750) || defined(CONFIG_I2S_WM8751)
extern void audiohw_loopback(int fsel);
extern void audiohw_bypass(void);
extern int audiohw_set_lineout_vol(int Aout, int vol_l, int vol_r);
extern int audiohw_set_linein_vol(int vol_l, int vol_r);
#endif

#if defined(CONFIG_I2S_WM8960)
extern void audiohw_codec_exlbk(void);
#endif

unsigned long txbuffer[512] = {
				0x01020304, 0x05060708, 0x090a0b0c, 0x0d0e0f10, 0x11121314, 0x15161718, 0x191a1b1c, 0x1d1e1f20,
			       	0x21222324, 0x25262728, 0x292a2b2c, 0x2d2e2f30, 0x31323334, 0x35363738, 0x393a3b3c, 0x3d3e3f40,
				0x41424344, 0x45464748, 0x494a4b4c, 0x4d4e4f50, 0x51525354, 0x55565758, 0x595a5b5c, 0x5d5e5f60,
				0x61626364, 0x65666768, 0x696a6b6c, 0x6d6e6f70, 0x71727374, 0x75767778, 0x797a7b7c, 0x7d7e7f80,
				0x81828384, 0x85868788, 0x898a8b8c, 0x8d8e8f90, 0x91929394, 0x95969798, 0x999a9b9c, 0x9d9e9fa0,
				0xa1a2a3a4, 0xa5a6a7a8, 0xa9aaabac, 0xadaeafb0, 0xb1b2b3b4, 0xb5b6b7b8, 0xb9babbbc, 0xbdbebfc0,
				0xc1c2c3c4, 0xc5c6c7c8, 0xc9cacbcc, 0xcdcecfd0, 0xd1d2d3d4, 0xd5d6d7d8, 0xd9dadbdc, 0xdddedfe0,
				0xe1e2e3e4, 0xe5e6e7e8, 0xe9eaebec, 0xedeeeff0, 0xf1f2f3f4, 0xf5f6f7f8, 0xf9fafbfc, 0xfdfeff00, //round 1
0x01020304, 0x05060708, 0x090a0b0c, 0x0d0e0f10, 0x11121314, 0x15161718, 0x191a1b1c, 0x1d1e1f20,
			       	0x21222324, 0x25262728, 0x292a2b2c, 0x2d2e2f30, 0x31323334, 0x35363738, 0x393a3b3c, 0x3d3e3f40,
				0x41424344, 0x45464748, 0x494a4b4c, 0x4d4e4f50, 0x51525354, 0x55565758, 0x595a5b5c, 0x5d5e5f60,
				0x61626364, 0x65666768, 0x696a6b6c, 0x6d6e6f70, 0x71727374, 0x75767778, 0x797a7b7c, 0x7d7e7f80,
				0x81828384, 0x85868788, 0x898a8b8c, 0x8d8e8f90, 0x91929394, 0x95969798, 0x999a9b9c, 0x9d9e9fa0,
				0xa1a2a3a4, 0xa5a6a7a8, 0xa9aaabac, 0xadaeafb0, 0xb1b2b3b4, 0xb5b6b7b8, 0xb9babbbc, 0xbdbebfc0,
				0xc1c2c3c4, 0xc5c6c7c8, 0xc9cacbcc, 0xcdcecfd0, 0xd1d2d3d4, 0xd5d6d7d8, 0xd9dadbdc, 0xdddedfe0,
				0xe1e2e3e4, 0xe5e6e7e8, 0xe9eaebec, 0xedeeeff0, 0xf1f2f3f4, 0xf5f6f7f8, 0xf9fafbfc, 0xfdfeff00, //round 2
0x01020304, 0x05060708, 0x090a0b0c, 0x0d0e0f10, 0x11121314, 0x15161718, 0x191a1b1c, 0x1d1e1f20,
			       	0x21222324, 0x25262728, 0x292a2b2c, 0x2d2e2f30, 0x31323334, 0x35363738, 0x393a3b3c, 0x3d3e3f40,
				0x41424344, 0x45464748, 0x494a4b4c, 0x4d4e4f50, 0x51525354, 0x55565758, 0x595a5b5c, 0x5d5e5f60,
				0x61626364, 0x65666768, 0x696a6b6c, 0x6d6e6f70, 0x71727374, 0x75767778, 0x797a7b7c, 0x7d7e7f80,
				0x81828384, 0x85868788, 0x898a8b8c, 0x8d8e8f90, 0x91929394, 0x95969798, 0x999a9b9c, 0x9d9e9fa0,
				0xa1a2a3a4, 0xa5a6a7a8, 0xa9aaabac, 0xadaeafb0, 0xb1b2b3b4, 0xb5b6b7b8, 0xb9babbbc, 0xbdbebfc0,
				0xc1c2c3c4, 0xc5c6c7c8, 0xc9cacbcc, 0xcdcecfd0, 0xd1d2d3d4, 0xd5d6d7d8, 0xd9dadbdc, 0xdddedfe0,
				0xe1e2e3e4, 0xe5e6e7e8, 0xe9eaebec, 0xedeeeff0, 0xf1f2f3f4, 0xf5f6f7f8, 0xf9fafbfc, 0xfdfeff00, //round 3
0x01020304, 0x05060708, 0x090a0b0c, 0x0d0e0f10, 0x11121314, 0x15161718, 0x191a1b1c, 0x1d1e1f20,
			       	0x21222324, 0x25262728, 0x292a2b2c, 0x2d2e2f30, 0x31323334, 0x35363738, 0x393a3b3c, 0x3d3e3f40,
				0x41424344, 0x45464748, 0x494a4b4c, 0x4d4e4f50, 0x51525354, 0x55565758, 0x595a5b5c, 0x5d5e5f60,
				0x61626364, 0x65666768, 0x696a6b6c, 0x6d6e6f70, 0x71727374, 0x75767778, 0x797a7b7c, 0x7d7e7f80,
				0x81828384, 0x85868788, 0x898a8b8c, 0x8d8e8f90, 0x91929394, 0x95969798, 0x999a9b9c, 0x9d9e9fa0,
				0xa1a2a3a4, 0xa5a6a7a8, 0xa9aaabac, 0xadaeafb0, 0xb1b2b3b4, 0xb5b6b7b8, 0xb9babbbc, 0xbdbebfc0,
				0xc1c2c3c4, 0xc5c6c7c8, 0xc9cacbcc, 0xcdcecfd0, 0xd1d2d3d4, 0xd5d6d7d8, 0xd9dadbdc, 0xdddedfe0,
				0xe1e2e3e4, 0xe5e6e7e8, 0xe9eaebec, 0xedeeeff0, 0xf1f2f3f4, 0xf5f6f7f8, 0xf9fafbfc, 0xfdfeff00, //round 4
0x01020304, 0x05060708, 0x090a0b0c, 0x0d0e0f10, 0x11121314, 0x15161718, 0x191a1b1c, 0x1d1e1f20,
			       	0x21222324, 0x25262728, 0x292a2b2c, 0x2d2e2f30, 0x31323334, 0x35363738, 0x393a3b3c, 0x3d3e3f40,
				0x41424344, 0x45464748, 0x494a4b4c, 0x4d4e4f50, 0x51525354, 0x55565758, 0x595a5b5c, 0x5d5e5f60,
				0x61626364, 0x65666768, 0x696a6b6c, 0x6d6e6f70, 0x71727374, 0x75767778, 0x797a7b7c, 0x7d7e7f80,
				0x81828384, 0x85868788, 0x898a8b8c, 0x8d8e8f90, 0x91929394, 0x95969798, 0x999a9b9c, 0x9d9e9fa0,
				0xa1a2a3a4, 0xa5a6a7a8, 0xa9aaabac, 0xadaeafb0, 0xb1b2b3b4, 0xb5b6b7b8, 0xb9babbbc, 0xbdbebfc0,
				0xc1c2c3c4, 0xc5c6c7c8, 0xc9cacbcc, 0xcdcecfd0, 0xd1d2d3d4, 0xd5d6d7d8, 0xd9dadbdc, 0xdddedfe0,
				0xe1e2e3e4, 0xe5e6e7e8, 0xe9eaebec, 0xedeeeff0, 0xf1f2f3f4, 0xf5f6f7f8, 0xf9fafbfc, 0xfdfeff00, //round 5
0x01020304, 0x05060708, 0x090a0b0c, 0x0d0e0f10, 0x11121314, 0x15161718, 0x191a1b1c, 0x1d1e1f20,
			       	0x21222324, 0x25262728, 0x292a2b2c, 0x2d2e2f30, 0x31323334, 0x35363738, 0x393a3b3c, 0x3d3e3f40,
				0x41424344, 0x45464748, 0x494a4b4c, 0x4d4e4f50, 0x51525354, 0x55565758, 0x595a5b5c, 0x5d5e5f60,
				0x61626364, 0x65666768, 0x696a6b6c, 0x6d6e6f70, 0x71727374, 0x75767778, 0x797a7b7c, 0x7d7e7f80,
				0x81828384, 0x85868788, 0x898a8b8c, 0x8d8e8f90, 0x91929394, 0x95969798, 0x999a9b9c, 0x9d9e9fa0,
				0xa1a2a3a4, 0xa5a6a7a8, 0xa9aaabac, 0xadaeafb0, 0xb1b2b3b4, 0xb5b6b7b8, 0xb9babbbc, 0xbdbebfc0,
				0xc1c2c3c4, 0xc5c6c7c8, 0xc9cacbcc, 0xcdcecfd0, 0xd1d2d3d4, 0xd5d6d7d8, 0xd9dadbdc, 0xdddedfe0,
				0xe1e2e3e4, 0xe5e6e7e8, 0xe9eaebec, 0xedeeeff0, 0xf1f2f3f4, 0xf5f6f7f8, 0xf9fafbfc, 0xfdfeff00, //round 6
0x01020304, 0x05060708, 0x090a0b0c, 0x0d0e0f10, 0x11121314, 0x15161718, 0x191a1b1c, 0x1d1e1f20,
			       	0x21222324, 0x25262728, 0x292a2b2c, 0x2d2e2f30, 0x31323334, 0x35363738, 0x393a3b3c, 0x3d3e3f40,
				0x41424344, 0x45464748, 0x494a4b4c, 0x4d4e4f50, 0x51525354, 0x55565758, 0x595a5b5c, 0x5d5e5f60,
				0x61626364, 0x65666768, 0x696a6b6c, 0x6d6e6f70, 0x71727374, 0x75767778, 0x797a7b7c, 0x7d7e7f80,
				0x81828384, 0x85868788, 0x898a8b8c, 0x8d8e8f90, 0x91929394, 0x95969798, 0x999a9b9c, 0x9d9e9fa0,
				0xa1a2a3a4, 0xa5a6a7a8, 0xa9aaabac, 0xadaeafb0, 0xb1b2b3b4, 0xb5b6b7b8, 0xb9babbbc, 0xbdbebfc0,
				0xc1c2c3c4, 0xc5c6c7c8, 0xc9cacbcc, 0xcdcecfd0, 0xd1d2d3d4, 0xd5d6d7d8, 0xd9dadbdc, 0xdddedfe0,
				0xe1e2e3e4, 0xe5e6e7e8, 0xe9eaebec, 0xedeeeff0, 0xf1f2f3f4, 0xf5f6f7f8, 0xf9fafbfc, 0xfdfeff00, //round 7
0x01020304, 0x05060708, 0x090a0b0c, 0x0d0e0f10, 0x11121314, 0x15161718, 0x191a1b1c, 0x1d1e1f20,
			       	0x21222324, 0x25262728, 0x292a2b2c, 0x2d2e2f30, 0x31323334, 0x35363738, 0x393a3b3c, 0x3d3e3f40,
				0x41424344, 0x45464748, 0x494a4b4c, 0x4d4e4f50, 0x51525354, 0x55565758, 0x595a5b5c, 0x5d5e5f60,
				0x61626364, 0x65666768, 0x696a6b6c, 0x6d6e6f70, 0x71727374, 0x75767778, 0x797a7b7c, 0x7d7e7f80,
				0x81828384, 0x85868788, 0x898a8b8c, 0x8d8e8f90, 0x91929394, 0x95969798, 0x999a9b9c, 0x9d9e9fa0,
				0xa1a2a3a4, 0xa5a6a7a8, 0xa9aaabac, 0xadaeafb0, 0xb1b2b3b4, 0xb5b6b7b8, 0xb9babbbc, 0xbdbebfc0,
				0xc1c2c3c4, 0xc5c6c7c8, 0xc9cacbcc, 0xcdcecfd0, 0xd1d2d3d4, 0xd5d6d7d8, 0xd9dadbdc, 0xdddedfe0,
				0xe1e2e3e4, 0xe5e6e7e8, 0xe9eaebec, 0xedeeeff0, 0xf1f2f3f4, 0xf5f6f7f8, 0xf9fafbfc, 0xfdfeff00  //round 8
				};

int i2s_debug_cmd(unsigned int cmd, unsigned long arg)
{
	unsigned long data, index;
	unsigned long *pTable;
	int i;

	switch(cmd)
	{
		case I2S_DEBUG_CLKGEN:
			MSG("I2S_DEBUG_CLKGEN\n");
#if defined(CONFIG_RALINK_RT3052)
			*(volatile unsigned long*)(0xB0000060) = 0x00000016;
			*(volatile unsigned long*)(0xB0000030) = 0x00009E00;
			*(volatile unsigned long*)(0xB0000A00) = 0xC0000040;
#elif defined(CONFIG_RALINK_RT3350)		
			*(volatile unsigned long*)(0xB0000060) = 0x00000018;
			*(volatile unsigned long*)(0xB000002C) = 0x00000100;
			*(volatile unsigned long*)(0xB0000030) = 0x00009E00;
			*(volatile unsigned long*)(0xB0000A00) = 0xC0000040;			
#elif defined(CONFIG_RALINK_RT3883)	
			*(volatile unsigned long*)(0xB0000060) = 0x00000018;
			*(volatile unsigned long*)(0xB000002C) = 0x00003000;
			*(volatile unsigned long*)(0xB0000A00) = 0xC1104040;
			*(volatile unsigned long*)(0xB0000A24) = 0x00000027;
			*(volatile unsigned long*)(0xB0000A20) = 0x80000020;
#elif (defined(CONFIG_RALINK_RT3352)||defined(CONFIG_RALINK_RT5350)) || defined (CONFIG_RALINK_RT6855)
			*(volatile unsigned long*)(0xB0000060) = 0x00000018;
			*(volatile unsigned long*)(0xB000002C) = 0x00000300;
			*(volatile unsigned long*)(0xB0000A00) = 0xC1104040;
			*(volatile unsigned long*)(0xB0000A24) = 0x00000027;
			*(volatile unsigned long*)(0xB0000A20) = 0x80000020;			
#elif defined(CONFIG_RALINK_RT6855A)
			*(volatile unsigned long*)(RALINK_SYSCTL_BASE+0x860) = 0x00008080;
			*(volatile unsigned long*)(RALINK_SYSCTL_BASE+0x82C) = 0x00000300;
			*(volatile unsigned long*)(RALINK_I2S_BASE+0x00) = 0xC1104040;
			*(volatile unsigned long*)(RALINK_I2S_BASE+0x24) = 0x00000027;
			*(volatile unsigned long*)(RALINK_I2S_BASE+0x20) = 0x80000020;	
#else
//#error "I2S debug mode not support this Chip"			
#endif			
			break;
		case I2S_DEBUG_INLBK:
			MSG("I2S_DEBUG_INLBK\n");
#if defined(CONFIG_RALINK_MT7621)
                        switch(96000)
                        {
                                case 8000:
                                        index = 0;
                                        break;
                                case 11025:
                                        index = 1;
                                        break;
                                case 12000:
                                        index = 2;
                                        break;
                                case 16000:
                                        index = 3;
                                        break;
                                case 22050:
                                        index = 4;
                                        break;
                                case 24000:
                                        index = 5;
                                        break;
                                case 32000:
                                        index = 6;
                                        break;
                                case 44100:
                                        index = 7;
                                        break;
                                case 48000:
                                        index = 8;
                                        break;
                                case 88200:
                                        index = 9;
                                        break;
                                case 96000:
                                        index = 10;
                                        break;
				case 192000:
                                        index = 11;
                                        break;
                                default:
                                        index = 7;
                        }
                        i2s_pll_config_mt7621(index);
#elif defined(CONFIG_ARCH_MT7623)
			i2s_pll_config_mt7623(11);
#endif


#if defined(CONFIG_RALINK_RT3052)
			break;
#endif
#if defined(CONFIG_RALINK_RT6855A)
			*(volatile unsigned long*)(RALINK_SYSCTL_BASE+0x834) |= 0x00020000;
			*(volatile unsigned long*)(RALINK_SYSCTL_BASE+0x834) &= 0xFFFDFFFF;	
			*(volatile unsigned long*)(RALINK_I2S_BASE+0x0) &= 0x7FFFFFFF;	//Rest I2S to default vaule	
			*(volatile unsigned long*)(RALINK_SYSCTL_BASE+0x860) |= 0x00008080;
			*(volatile unsigned long*)(RALINK_SYSCTL_BASE+0x82C) = 0x00000300;
#elif defined(CONFIG_RALINK_MT7621)
                        *(volatile unsigned long*)(RALINK_SYSCTL_BASE+0x34) |= 0x00020000;
                        *(volatile unsigned long*)(RALINK_SYSCTL_BASE+0x34) &= 0xFFFDFFFF;
                        *(volatile unsigned long*)(RALINK_I2S_BASE+0x0) &= 0x7FFFFFFF;   //Rest I2S to default vaule
                        *(volatile unsigned long*)(RALINK_SYSCTL_BASE+0x60) = 0x00000010;     //GPIO purpose selection
#elif defined(CONFIG_RALINK_MT7628)
                        *(volatile unsigned long*)(RALINK_SYSCTL_BASE+0x34) |= 0x00020000;
                        *(volatile unsigned long*)(RALINK_SYSCTL_BASE+0x34) &= 0xFFFDFFFF;
                        *(volatile unsigned long*)(RALINK_I2S_BASE+0x0) &= 0x7FFFFFFF;   //Rest I2S to default vaule
                        *(volatile unsigned long*)(RALINK_SYSCTL_BASE+0x60) &= ~((0x3)<<6);     //GPIO purpose selection /*FIXME*/
#elif defined(CONFIG_ARCH_MT7623)
			*(volatile unsigned long*)(0xFB000034) |= 0x00020000;
			*(volatile unsigned long*)(0xFB000034) &= 0xFFFDFFFF;
			*(volatile unsigned long*)(ETHDMASYS_I2S_BASE+0x0) &= 0x7FFFFFFF;   //Rest I2S to default vaule
			
			*(volatile unsigned long*)(0xF0005840) &= ~((0x7)<<12);
			*(volatile unsigned long*)(0xF0005840) |= ((0x6)<<12);
			*(volatile unsigned long*)(0xF0005840) &= ~((0x7)<<9);
			*(volatile unsigned long*)(0xF0005840) |= ((0x6)<<9);
			*(volatile unsigned long*)(0xF0005040) |= ((0x1)<<10);
			*(volatile unsigned long*)(0xF0005040) |= ((0x1)<<9);

			*(volatile unsigned long*)(0xF00057F0) &= ~((0x7)<<12);
			*(volatile unsigned long*)(0xF00057F0) |= ((0x6)<<12);
			*(volatile unsigned long*)(0xF0005030) |= ((0x1)<<1);

			*(volatile unsigned long*)(0xF0005840) &= ~((0x7)<<6);
			*(volatile unsigned long*)(0xF0005840) |= ((0x6)<<6);
			*(volatile unsigned long*)(0xF0005040) &= ~((0x1)<<8);

			*(volatile unsigned long*)(0xF00058F0) &= ~((0x7)<<3);
			*(volatile unsigned long*)(0xF00058F0) |= ((0x6)<<3);
			*(volatile unsigned long*)(0xF0005070) |= ((0x1)<<14);


#else	
			*(volatile unsigned long*)(0xB0000034) |= 0x00020000;
			*(volatile unsigned long*)(0xB0000034) &= 0xFFFDFFFF;	
			*(volatile unsigned long*)(0xB0000A00) &= 0x7FFFFFFF;	//Rest I2S to default vaule
			*(volatile unsigned long*)(0xB0000060) = 0x00000018;

#if defined(CONFIG_RALINK_RT3883)			
			*(volatile unsigned long*)(0xB000002C) = 0x00003000;
#elif defined(CONFIG_ARCH_MT7623)

#else
			*(volatile unsigned long*)(0xB000002C) = 0x00000300;
#endif
#endif			
#if defined(CONFIG_RALINK_MT7621)
                        *(volatile unsigned long*)(RALINK_I2S_BASE+0x18) = 0x80000000;
                        *(volatile unsigned long*)(RALINK_I2S_BASE+0x00) = 0xc1104040;

                        pTable = i2s_inclk_int;
                        data = pTable[index];
                        //*(volatile unsigned long*)(RALINK_I2S_BASE+0x24) = data;
                        i2s_outw(RALINK_I2S_BASE+0x24, data);

                        pTable = i2s_inclk_comp;
                        data = pTable[index];
                        //*(volatile unsigned long*)(RALINK_I2S_BASE+0x20) = data;
                        i2s_outw(RALINK_I2S_BASE+0x20, (data|0x80000000));
#elif defined(CONFIG_RALINK_MT7628)
			index =11;  /* SR: 192k */
			*(volatile unsigned long*)(RALINK_I2S_BASE+0x18) = 0x80000000;
                        *(volatile unsigned long*)(RALINK_I2S_BASE+0x00) = 0xc1104040;

                        pTable = i2s_inclk_int_16bit;
			//pTable = i2s_inclk_int_24bit;
                        data = pTable[index];
                        //*(volatile unsigned long*)(RALINK_I2S_BASE+0x24) = data;
                        i2s_outw(RALINK_I2S_BASE+0x24, data);

                        pTable = i2s_inclk_comp_16bit;
			//pTable = i2s_inclk_comp_24bit;
                        data = pTable[index];
                        //*(volatile unsigned long*)(RALINK_I2S_BASE+0x20) = data;
                        i2s_outw(RALINK_I2S_BASE+0x20, (data|0x80000000));
			mdelay(5);
#elif defined(CONFIG_ARCH_MT7623)
			index = 11;
			*(volatile unsigned long*)(I2S_I2SCFG1) = 0x80000000;
			*(volatile unsigned long*)(I2S_I2SCFG) = 0xE1104040;
			*(volatile unsigned long*)(ETHDMASYS_SYSCTL_BASE+0x30) |= 0x00020000;
			*(volatile unsigned long*)(ETHDMASYS_SYSCTL_BASE+0x2c) |= 0x00000080;

			pTable = i2s_inclk_int_16bit;
			//pTable = i2s_inclk_int_24bit;
                        data = pTable[index];
                        i2s_outw(I2S_DIVINT_CFG, data);

                        pTable = i2s_inclk_comp_16bit;
			//pTable = i2s_inclk_comp_24bit;
                        data = pTable[index];
                        i2s_outw(I2S_DIVCOMP_CFG, (data|0x80000000));
			mdelay(5);
#else
			*(volatile unsigned long*)(RALINK_I2S_BASE+0x18) = 0x80000000;
			*(volatile unsigned long*)(RALINK_I2S_BASE+0x00) = 0xC1104040;
			*(volatile unsigned long*)(RALINK_I2S_BASE+0x24) = 0x00000006;
			*(volatile unsigned long*)(RALINK_I2S_BASE+0x20) = 0x80000105;
#endif
			{
				int count = 0;
				int k=0;
				int enable_cnt=0;
				unsigned long param[4];
				unsigned long data;
				//unsigned long data_tmp;
				unsigned long ff_status;
			  	//unsigned long* txbuffer;
#if 0
				int j=0;
				int temp = 0;
#endif
#if defined (INTERNAL_LOOPBACK_DEBUG)
				int count2 = 0;
#endif
				memset(param, 0, 4*sizeof(unsigned long) );	
				copy_from_user(param, (unsigned long*)arg, sizeof(long)*2);
#if 0
				txbuffer = (unsigned long*)kcalloc(param[0], sizeof(unsigned long), GFP_KERNEL);
				if(txbuffer == NULL)
					return -1;
#endif

				//ff_status = *(volatile unsigned long*)(RALINK_I2S_BASE+0x0C);
				ff_status = *(volatile unsigned long*)(I2S_FF_STATUS);
				printk("ff status=[0x%08X]\n",(u32)ff_status);

#if 0
				for(i = 0; i < param[0]; i++)
				{
					if (i==0)
					{
						txbuffer[i] = 0x555A555A;
						printk("%d: 0x%8lx\n", i, txbuffer[i]);
					}
					else 
					{
						#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,14)
						srandom32(jiffies);
						txbuffer[i] = random32()%(0x555A555A)+1;
						//printk("%d: 0x%8x\n", i, txbuffer[i]);
						#else
						//TODO:do we need to implement random32()
						txbuffer[i] = 0x01010101;					
						#endif
					}
				}
#endif
	
				for( i = 0 ; i < param[0] ; i ++ )
				{
					ff_status = *(volatile unsigned long*)(I2S_FF_STATUS);
				#if defined(CONFIG_RALINK_MT7628) || defined(CONFIG_ARCH_MT7623)	
					if((ff_status&0xFF) > 0)
				#else
					if((ff_status&0x0F) > 0)
				#endif
					{
						*(volatile unsigned long*)(I2S_TX_FIFO_WREG) = txbuffer[i];
						mdelay(1);
					}
					else
					{
						mdelay(1);
						printk("[%d]NO TX FREE FIFO ST=[0x%08X]\n", i, (u32)ff_status);
						continue;	
					}

					//if(i >= 16)
					{

						ff_status = *(volatile unsigned long*)(I2S_FF_STATUS);
					#if defined(CONFIG_RALINK_MT7628)
						if(((ff_status>>8)&0xFF) > 0)
					#else
						if(((ff_status>>4)&0x0F) > 0)
					#endif
						{
							data = *(volatile unsigned long*)(I2S_RX_FIFO_RREG);
							//data_tmp = *(volatile unsigned long*)(I2S_RX_FIFO_RREG);
							//MSG("[0x%08X] vs [0x%08X]\n", (u32)data, (u32)data_tmp);
						}
						else
						{
							printk("*[%d]NO RX FREE FIFO ST=[0x%08X]\n", i, (u32)ff_status);
							continue;
						}
						
						if (data == txbuffer[0])
						{
							k = i;
							enable_cnt = 1;
						}
						if (enable_cnt==1)
						{
							if(data!= txbuffer[i-k])
							{
								MSG("[%d][0x%08X] vs [0x%08X]\n", (i-k), (u32)data, (u32)txbuffer[i-k]);
							}
							else
							{
								//MSG("**[%d][0x%08X] vs [0x%08X]\n" ,(i-k), (u32)data , (u32)txbuffer[i-k]);
								count++;
								data=0;
							}
						}

					}	
				}
#if 0	
				temp = i-k;
				for (j=0; j<k; j++)
				{

					//ff_status = *(volatile unsigned long*)(RALINK_I2S_BASE+0x0C);
					ff_status = *(volatile unsigned long*)(I2S_FF_STATUS);
				#if defined(CONFIG_RALINK_MT7628) || defined(CONFIG_ARCH_MT7623)
					if(((ff_status>>8)&0xFF) > 0)
				#else
					if(((ff_status>>4)&0x0F) > 0)
				#endif
					{
						//data = *(volatile unsigned long*)(RALINK_I2S_BASE+0x14);
						data = *(volatile unsigned long*)(I2S_RX_FIFO_RREG);
					}
					else
					{
						printk("*NO RX FREE FIFO ST=[0x%08X]\n", (u32)ff_status);
						continue;
					}

					if(data!= txbuffer[temp+j])
					{
						MSG("[%d][0x%08X] vs [0x%08X]\n", (temp+j), (u32)data, (u32)txbuffer[temp+j]);
					}
					else
					{
						//MSG("&&[%d][0x%08X] vs [0x%08X]\n" ,(temp+j), (u32)data , (u32)txbuffer[temp+j]);
						count++;
						data=0;
					}
					if ((temp+j)==128)
					{
						//ff_status = *(volatile unsigned long*)(RALINK_I2S_BASE+0x0C);
						ff_status = *(volatile unsigned long*)(I2S_FF_STATUS);
						//printk("[%d]FIFO ST=[0x%08X]\n", (temp+j), (u32)ff_status);
					}
				}
#endif

#if defined (INTERNAL_LOOPBACK_DEBUG)
				for( i = 0 ; i < param[0] ; i ++ )
				{
					//ff_status = *(volatile unsigned long*)(RALINK_I2S_BASE+0x0C);
					ff_status = *(volatile unsigned long*)(I2S_FF_STATUS);
				#if defined(CONFIG_RALINK_MT7628)|| defined(CONFIG_ARCH_MT7623)
					if((ff_status&0xFF) > 0)
				#else
					if((ff_status&0x0F) > 0)
				#endif
					{
						//*(volatile unsigned long*)(RALINK_I2S_BASE+0x10) = txbuffer[i];
						*(volatile unsigned long*)(I2S_TX_FIFO_WREG) = txbuffer[i];
						mdelay(1);
					}
					else
					{
						mdelay(1);
						printk("[%d]NO TX FREE FIFO ST=[0x%08X]\n", i, (u32)ff_status);
						continue;	
					}

					//if(i >= 16)
					{

						//ff_status = *(volatile unsigned long*)(RALINK_I2S_BASE+0x0C);
						ff_status = *(volatile unsigned long*)(I2S_FF_STATUS);
					#if defined(CONFIG_RALINK_MT7628)|| defined(CONFIG_ARCH_MT7623)
						if(((ff_status>>8)&0xFF) > 0)
					#else
						if(((ff_status>>4)&0x0F) > 0)
					#endif
						{
							//data = *(volatile unsigned long*)(RALINK_I2S_BASE+0x14);
							data = *(volatile unsigned long*)(I2S_RX_FIFO_RREG);
						}
						else
						{
							printk("*[%d]NO RX FREE FIFO ST=[0x%08X]\n", i, (u32)ff_status);
							continue;
						}
						
						{
							if(data!= txbuffer[i])
							{
								MSG("[%d][0x%08X] vs [0x%08X]\n", (i), (u32)data, (u32)txbuffer[i]);
							}
							else
							{
								MSG("**[%d][0x%08X] vs [0x%08X]\n" ,(i), (u32)data , (u32)txbuffer[i]);
								count2++;
								data=0;
							}
						}

					}	
				}
				printk("Pattern match done count2=%d.\n", count2);
#endif
				printk("Pattern match done count=%d.\n", count);

			}	
#if defined(CONFIG_ARCH_MT7623)
			*(volatile unsigned long*)(0xFB000034) |= 0x00020000;
			*(volatile unsigned long*)(0xFB000034) &= 0xFFFDFFFF;
			*(volatile unsigned long*)(ETHDMASYS_I2S_BASE+0x0) &= 0x7FFFFFFF;   //Rest I2S to default vaule
#endif	

#if !defined(CONFIG_RALINK_RT3052)
			break;
#endif
		case I2S_DEBUG_EXLBK:
			MSG("I2S_DEBUG_EXLBK\n");
#if !defined(CONFIG_ARCH_MT7623)
			switch(arg)
			{
				case 8000:
					index = 0;
					break;
				case 11025:
					index = 1;
					break;
				case 12000:
					index = 2;
					break;			
				case 16000:
					index = 3;
					break;
				case 22050:
					index = 4;
					break;
				case 24000:
					index = 5;
					break;	
				case 32000:
					index = 6;
					break;			
				case 44100:
					index = 7;
					break;
				case 48000:
					index = 8;
					break;
				case 88200:
					index = 9;
					break;	
				case 96000:
					index = 10;
					break;
				default:
					index = 7;
			}
#if defined(CONFIG_RALINK_RT3052)
			break;
#endif			
#if defined(CONFIG_RALINK_RT6855A)
			*(volatile unsigned long*)(RALINK_SYSCTL_BASE+0x860) = 0x00008080;
			//*(volatile unsigned long*)(RALINK_SYSCTL_BASE+0x82C) = 0x00000300;
#else			
			*(volatile unsigned long*)(RALINK_SYSCTL_BASE+0x60) = 0x00000018;
#if defined(CONFIG_RALINK_RT3883)
			*(volatile unsigned long*)(RALINK_SYSCTL_BASE+0x2C) = 0x00003000;			
#else
			*(volatile unsigned long*)(RALINK_SYSCTL_BASE+0x2C) = 0x00000300;
#endif
#endif
	
			*(volatile unsigned long*)(RALINK_I2S_BASE+0x18) = 0x40000000;
			*(volatile unsigned long*)(RALINK_I2S_BASE+0x00) = 0x81104040;
#if defined(CONFIG_RALINK_MT7628)
			pTable = i2s_inclk_int_16bit;
#else
			pTable = i2s_inclk_int;
#endif
			data = (volatile unsigned long)(pTable[index]);
			i2s_outw(I2S_DIVINT_CFG, data);
#if defined(CONFIG_RALINK_MT7628)
			pTable = i2s_inclk_comp_16bit;
#else
			pTable = i2s_inclk_comp;
#endif
			data = (volatile unsigned long)(pTable[index]);
			data |= REGBIT(1, I2S_CLKDIV_EN);
			i2s_outw(I2S_DIVCOMP_CFG, data);

		#if defined(CONFIG_I2S_MCLK_12MHZ)
			pTable = i2s_codec_12Mhz;
			#if defined(CONFIG_I2S_WM8960)
				data = pTable[index];
			#else
				data = pTable[index]|0x01;
			#endif
		#else
			pTable = i2s_codec_12p288Mhz;
			data = pTable[index];
		#endif

		#if defined(CONFIG_I2S_WM8960) || defined(CONFIG_I2S_WM8750) || defined(CONFIG_I2S_WM8751)
			audiohw_preinit();
		#endif


		#if defined (CONFIG_I2S_WM8960)
			audiohw_postinit(1, 1, 1, 1, 0); // for codec apll enable, 16 bit word length 
		#elif defined(CONFIG_I2S_WM8750) || defined(CONFIG_I2S_WM8751)
    			audiohw_postinit(1, 1, 1, 0); // for 16 bit word length 
		#endif


		#if defined (CONFIG_I2S_WM8960)
			audiohw_set_frequency(data, 1);	// for codec apll enable
		#elif defined(CONFIG_I2S_WM8750) || defined(CONFIG_I2S_WM8751)
            		audiohw_set_frequency(data|0x1);
		#endif


		#if defined(CONFIG_I2S_WM8960) || defined(CONFIG_I2S_WM8750) || defined(CONFIG_I2S_WM8751)
			audiohw_set_lineout_vol(1, 100, 100);
			audiohw_set_linein_vol(100, 100);
		#endif
		

		#if defined(CONFIG_I2S_TXRX)			
			//audiohw_loopback(data);
		#endif
		#if !defined(CONFIG_RALINK_RT3052)
			break;
		#endif
#endif
		case I2S_DEBUG_CODECBYPASS:			
		#if defined(CONFIG_I2S_TXRX)
		#if defined(CONFIG_RALINK_MT7628)	
			data = i2s_inw(RALINK_SYSCTL_BASE+0x60); 
			//data &= ~(0x3<<4);
			data &= ~(0x3<<6);
			data &= ~(0x3<<16);
			data &= ~(0x1<<14);
			i2s_outw(RALINK_SYSCTL_BASE+0x60, data);

			data = i2s_inw(RALINK_SYSCTL_BASE+0x2c);
			data &= ~(0x07<<9);
			i2s_outw(RALINK_SYSCTL_BASE+0x2c, data);
		#endif
		
		#if defined(CONFIG_I2S_WM8960) || defined(CONFIG_I2S_WM8750) || defined(CONFIG_I2S_WM8751)
			audiohw_bypass();	/* did not work */
		#endif
		#endif
			break;	
		case I2S_DEBUG_FMT:
			break;
		case I2S_DEBUG_RESET:
			break;
#if defined(CONFIG_I2S_WM8960)
		case I2S_DEBUG_CODEC_EXLBK:
			audiohw_codec_exlbk();
			break;
#endif	
		default:
			MSG("Not support this debug cmd [%d]\n", cmd);	
			break;				
	}
	
	return 0;	
}
