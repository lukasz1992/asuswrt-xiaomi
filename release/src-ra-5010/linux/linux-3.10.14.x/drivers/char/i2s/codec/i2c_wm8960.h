/* wm8960.h  --  WM8960 Soc Audio driver */
#ifndef _WM8960_H
#define _WM8960_H

#define bool	unsigned char
#define false 0
#define true 1

/* volume/balance/treble/bass interdependency */
#define VOLUME_MIN 	-730
#define VOLUME_MAX  	60


/* Register addresses and bits */
#define OUTPUT_MUTED                	0x2f
#define OUTPUT_0DB                  	0x79

#define LINV			    	0x00
#define LINV_IPVU		    	(1 << 8)  /* FIXME */
#define LINV_LINMUTE		    	(1 << 7)
#define LINV_LIZC                       (1 << 6)
#define LINV_LINVOL(x)              	((x) & 0x3f)

#define RINV			    	0x01
#define RINV_IPVU		    	(1 << 8) /* FIXME */
#define RINV_RINMUTE		    	(1 << 7)
#define RINV_RIZC                   	(1 << 6)
#define RINV_RINVOL(x)              	((x) & 0x3f)

#define LOUT1                       	0x02
#define LOUT1_LO1VU                 	(1 << 8)
#define LOUT1_LO1ZC                 	(1 << 7)
#define LOUT1_LOUT1VOL(x)           	((x) & 0x7f)

#define ROUT1                       	0x03
#define ROUT1_RO1VU                 	(1 << 8)
#define ROUT1_RO1ZC                 	(1 << 7)
#define ROUT1_ROUT1VOL(x)           	((x) & 0x7f)

#define CLOCKING1			0x04  /* FIXME */
#define CLOCKING1_ADCDIV(x)		(((x) & 0x7) << 6)
#define CLOCKING1_DACDIV(x)		(((x) & 0x7) << 3)
#define CLOCKING1_SYSCLKDIV_1		(0 << 1)
#define CLOCKING1_SYSCLKDIV_2		(2 << 1)
#define CLOCKING1_CLKSEL_MCLK		(0 << 0)
#define CLOCKING1_CLKSEL_PLL		(1 << 0)

#define DACCTRL1                     	0x05
#define DACCTRL1_DATTENUATE             (1 << 7)
#define DACCTRL1_DACMU               	(1 << 3)
#define DACCTRL1_DEEMPH_48           	(3 << 1)
#define DACCTRL1_DEEMPH_44           	(2 << 1)
#define DACCTRL1_DEEMPH_32           	(1 << 1)
#define DACCTRL1_DEEMPH_NONE         	(0 << 1)
#define DACCTRL1_DEEMPH(x)           	((x) & (0x3 << 1))

#define DACCTRL2			0x06

#define AINTFCE1                     	0x07
#define AINTFCE1_BCLKINV             	(1 << 7)
#define AINTFCE1_MS                  	(1 << 6)
#define AINTFCE1_LRSWAP              	(1 << 5)
#define AINTFCE1_LRP                 	(1 << 4)
#define AINTFCE1_WL_32               	(3 << 2)
#define AINTFCE1_WL_24                  (2 << 2)
#define AINTFCE1_WL_20                  (1 << 2)
#define AINTFCE1_WL_16               	(0 << 2)
#define AINTFCE1_WL(x)               	(((x) & 0x3) << 2)
#define AINTFCE1_FORMAT_DSP             (3 << 0)
#define AINTFCE1_FORMAT_I2S             (2 << 0)
#define AINTFCE1_FORMAT_LJUST           (1 << 0)
#define AINTFCE1_FORMAT_RJUST        	(0 << 0)
#define AINTFCE1_FORMAT(x)           	((x) & 0x3)

/* FIXME */
#define CLOCKING2                    	0x08
#define CLOCKING2_DCLKDIV(x)            (((x) & 0x7) << 6)
#define CLOCKING2_BCLKDIV(x)		(((x) & 0xf) << 0)

#define AINTFCE2			0x09
#define AINTFCE2_ALRCGPIO_ALRC		(0 << 6)
#define	AINTFCE2_ALRCGPIO_GPIO		(1 << 6)
#define	AINTFCE2_LOOPBACK		(1 << 0)

#define LEFTGAIN                    	0x0a
#define LEFTGAIN_LDVU                   (1 << 8)
#define LEFTGAIN_LDACVOL(x)         	((x) & 0xff)

#define RIGHTGAIN                   	0x0b
#define RIGHTGAIN_RDVU                  (1 << 8)
#define RIGHTGAIN_RDACVOL(x)        	((x) & 0xff)

#define RESET                       	0x0f
#define RESET_RESET                 	0x000

#define ALC1			    	0x11
#define ALC1_ALCOFF		    	(0x0 << 7)
#define	ALC1_ALCRONLY		    	(0x1 << 7)
#define ALC1_ALCLONLY		    	(0x2 << 7)
#define ALC1_ALCSTEREO		    	(0x3 << 7)
#define ALC1_ALCSEL(x)		    	(((x) & 0x3) << 7)
#define ALC1_SET_MAXGAIN(x)	    	((x & 0x7) << 4)
#define ALC1_GET_MAXGAIN(x)	    	((x) & (0x7 << 4))
#define ALC1_ALCL(x)		    	((x) & 0x0f)	

#define ALC2			    	0x12
#define ALC2_MINGAIN(x)               	((x & 0x7) << 4)
#define ALC2_HLD(x)		    	((x) & 0x0f)

#define ALC3			    	0x13
#define ALC3_SET_DCY(x)		    	((x & 0x0f) << 4)
#define ALC3_GET_DCY(x)		    	((x) & (0x0f << 4))
#define ALC3_ATK(x)		    	((x) & 0x0f)

#define NOISEGATE		    	0x14
#define NOISEGATE_SET_NGTH(x)	    	((x & 0x1f) << 3)
#define NOISEGATE_GET_NGTH(x)	    	((x) & (0x1f << 3))
#define NOISEGATE_NGAT_ENABLE	    	1

#define LADCVOL			    	0x15
#define LADCVOL_LAVU_EN		    	(1 << 8)
#define LADCVOL_LADCVOL(x)	    	((x) & 0x0ff)

#define RADCVOL			    	0x16
#define RADCVOL_RAVU_EN		    	(1 << 8)
#define RADCVOL_RADCVOL(x)	    	((x) & 0x0ff)

#define ADDITIONAL1                 	0x17
#define ADDITIONAL1_TSDEN               (1 << 8)
#define ADDITIONAL1_VSEL_LOWEST     	(0 << 6)
#define ADDITIONAL1_VSEL_LOW        	(1 << 6)
#define ADDITIONAL1_VSEL_DEFAULT2   	(2 << 6)
#define ADDITIONAL1_VSEL_DEFAULT    	(3 << 6)
#define ADDITIONAL1_VSEL(x)         	(((x) & 0x3) << 6)
#define ADDITIONAL1_DMONOMIX_STEREO   	(0 << 4)
#define ADDITIONAL1_DMONOMIX_MONO   	(1 << 4)
#define ADDITIONAL1_DATSEL(x)         	(((x) & 0x3) << 2)
#define ADDITIONAL1_TOCLKSEL            (1 << 1)
#define ADDITIONAL1_TOEN                (1 << 0)

#define ADDITIONAL2                 	0x18
#define ADDITIONAL2_HPSWEN              (1 << 6)
#define ADDITIONAL2_HPSWPOL             (1 << 5)
#define ADDITIONAL2_TRIS                (1 << 3)
#define ADDITIONAL2_LRCM_ON         	(1 << 2)

#define PWRMGMT1                    	0x19
#define PWRMGMT1_VMIDSEL_DISABLED   	(0 << 7)
#define PWRMGMT1_VMIDSEL_50K        	(1 << 7)
#define PWRMGMT1_VMIDSEL_250K       	(2 << 7)
#define PWRMGMT1_VMIDSEL_5K         	(3 << 7)
#define PWRMGMT1_VREF                   (1 << 6)
#define PWRMGMT1_AINL                   (1 << 5)
#define PWRMGMT1_AINR                   (1 << 4)
#define PWRMGMT1_ADCL                   (1 << 3)
#define PWRMGMT1_ADCR                   (1 << 2)
#define PWRMGMT1_MICB                   (1 << 1)
#define PWRMGMT1_DIGENB                 (1 << 0)

#define PWRMGMT2                    	0x1a
#define PWRMGMT2_DACL               	(1 << 8)
#define PWRMGMT2_DACR                   (1 << 7)
#define PWRMGMT2_LOUT1                  (1 << 6)
#define PWRMGMT2_ROUT1                  (1 << 5)
#define PWRMGMT2_SPKL                   (1 << 4)
#define PWRMGMT2_SPKR                   (1 << 3)
#define PWRMGMT2_OUT3                   (1 << 1)
#define PWRMGMT2_PLL_EN                 (1 << 0)

#define ADDITIONAL3                 	0x1b
#define ADDITIONAL3_VROI            	(1 << 6)
#define ADDITIONAL3_OUT3CAP             (1 << 3)
#define ADDITIONAL3_ADC_ALC_SR(x)       ((x) & 0x7)

#define ANTIPOP1			0x1c
#define ANTIPOP2			0x1d

#define ADCLPATH			0x20
#define ADCLPATH_LMN1			(1 << 8)
#define ADCLPATH_LMP3                   (1 << 7)
#define ADCLPATH_LMP2                   (1 << 6)
#define ADCLPATH_LMICBOOST_29DB         (0x3 << 4)
#define ADCLPATH_LMICBOOST_20DB         (0x2 << 4)
#define ADCLPATH_LMICBOOST_13DB         (0x1 << 4)
#define ADCLPATH_SET_LMICBOOST(x)       ((x & 0x3) << 4)
#define ADCLPATH_LMIC2B                 (1 << 3)


#define ADCRPATH			0x21
#define ADCRPATH_RMN1			(1 << 8)
#define ADCRPATH_RMP3                   (1 << 7)
#define ADCRPATH_RMP2                   (1 << 6)
#define ADCRPATH_RMICBOOST_29DB         (0x3 << 4)
#define ADCRPATH_RMICBOOST_20DB         (0x2 << 4)
#define ADCRPATH_RMICBOOST_13DB         (0x1 << 4)
#define ADCRPATH_SET_RMICBOOST(x)       ((x & 0x3) << 4)
#define ADCRPATH_RMIC2B                 (1 << 3)


#define LEFTMIX1                    	0x22
#define LEFTMIX1_LD2LO              	(1 << 8)
#define LEFTMIX1_LI2LO                  (1 << 7)
#define LEFTMIX1_LI2LO_DEFAULT          (5 << 4)
#define LEFTMIX1_LI2LOVOL(x)            (((x) & 0x7) << 4)

#define RIGHTMIX2                   	0x25
#define RIGHTMIX2_RD2RO             	(1 << 8)
#define RIGHTMIX2_RI2RO                 (1 << 7)
#define RIGHTMIX2_RI2RO_DEFAULT         (5 << 4)
#define RIGHTMIX2_RI2ROVOL(x)           (((x) & 0x7) << 4)

#define MONOMIX1                    	0x26
#define MONOMIX1_L2MO              	(1 << 7)

#define MONOMIX2                    	0x27
#define MONOMIX2_R2MO              	(1 << 7)

#define LSPK                       	0x28
#define LSPK_SPKLVU                 	(1 << 8)
#define LSPK_SPKLZC                     (1 << 7)
#define LSPK_SPKLVOL(x)                 ((x) & 0x7f)

#define RSPK                       	0x29
#define RSPK_SPKRVU                 	(1 << 8)
#define RSPK_SPKRZC                     (1 << 7)
#define RSPK_SPKRVOL(x)                 ((x) & 0x7f)

#define OUT3V                     	0x2a
#define LINBMIX				0x2b
#define RINBMIX				0x2c
#define BYPASS1 			0x2d
#define BYPASS2				0x2e

#define PWRMGMT3 			0x2f
#define PWRMGMT3_LMIC			(1<<5)
#define PWRMGMT3_RMIC                   (1<<4)
#define PWRMGMT3_LOMIX                  (1<<3)
#define PWRMGMT3_ROMIX                  (1<<2)

#define ADDITIONAL4 			0x30

#define CLASSDCTRL1 			0x31
#define CLASSDCTRL1_OP_OFF		(0<<6)
#define CLASSDCTRL1_OP_LSPK		(1<<6)
#define CLASSDCTRL1_OP_RSPK		(2<<6)
#define CLASSDCTRL1_OP_LRSPK		(3<<6)

#define CLASSDCTRL3			0x33

#define PLL1				0x34
#define PLL1_OPCLKDIV_1			(0<<6)
#define PLL1_OPCLKDIV_2			(1<<6)
#define PLL1_OPCLKDIV_3			(2<<6)
#define PLL1_OPCLKDIV_4			(3<<6)
#define	PLL1_OPCLKDIV_5p5		(4<<6)
#define	PLL1_OPCLKDIV_6			(5<<6)
#define PLL1_SDM_INTERGER		(0<<5)
#define PLL1_SDM_FRACTIONAL		(1<<5)
#define PLL1_PLLPRESCALE_1		(0<<4)
#define PLL1_PLLPRESCALE_2		(1<<4)
#define PLL1_PLLN(x)			((x) & 0xf)

#define PLL2				0x35
#define	PLL2_PLLK_23_16(x)		((x) & 0x1ff)

#define PLL3				0x36
#define PLL3_PLLK_15_8(x)		((x) & 0x1ff)

#define PLL4				0x37
#define PLL4_PLLK_7_0(x) 		((x) & 0x1ff)

/* codec API */
void audiohw_preinit(void);
int audiohw_postinit(int bSlave, int AIn, int AOut, int pll_en, int wordLen24b);
void audiohw_close(void);
void audiohw_set_frequency(int fsel, int pll_en);
void audiohw_mute(bool mute);
void audiohw_micboost(int boostgain);
void audiohw_micin(int enableMic);
void audiohw_set_apll(int srate);
int audiohw_set_lineout_vol(int Aout, int vol_l, int vol_r);
int audiohw_set_linein_vol(int vol_l, int vol_r);
void audiohw_mute( bool mute);
void audiohw_loopback(int fsel);
void audiohw_codec_exlbk(void);
void audiohw_bypass(void);

#endif /* _WM875x_H */
