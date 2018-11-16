/*
 * mtk_audio_drv.c
 *
 *  Created on: 2013/8/20
 *      Author: MTK04880
 */
#include <linux/init.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
#include <linux/sched.h>
#endif
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h> /* O_ACCMODE */
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36)
#include <asm/system.h> /* cli(), *_flags */
#endif
#include <asm/uaccess.h> /* copy_from/to_user */
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/dma-mapping.h>
#include <sound/core.h>
#include <linux/pci.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>
#include "drivers/char/ralink_gdma.h"
#include "mt76xx_i2s.h"

/****************************/
/*GLOBAL VARIABLE DEFINITION*/
/****************************/
extern i2s_config_type* pi2s_config;
static int g_openFlag = 0;

/****************************/
/*FUNCTION DECLRATION		*/
/****************************/
static int mt76xx_i2s_set_fmt(struct snd_soc_dai *cpu_dai,\
		unsigned int fmt);

static int  mt76xx_i2s_shutdown(struct snd_pcm_substream *substream,
		       struct snd_soc_dai *dai);
static int  mt76xx_i2s_startup(struct snd_pcm_substream *substream,
		       struct snd_soc_dai *dai);
static int mt76xx_i2s_hw_params(struct snd_pcm_substream *substream,\
				struct snd_pcm_hw_params *params,\
				struct snd_soc_dai *dai);
static int mt76xx_i2s_play_prepare(struct snd_pcm_substream *substream,struct snd_soc_dai *dai);
static int mt76xx_i2s_rec_prepare(struct snd_pcm_substream *substream,struct snd_soc_dai *dai);
static int mt76xx_i2s_hw_free(struct snd_pcm_substream *substream,struct snd_soc_dai *dai);
static int mt76xx_i2s_prepare(struct snd_pcm_substream *substream, struct snd_soc_dai *dai);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,20)
static int mt76xx_i2s_drv_probe(struct platform_device *pdev);
static void mt76xx_i2s_drv_remove(struct platform_device *pdev);
#endif
/****************************/
/*STRUCTURE DEFINITION		*/
/****************************/


static struct snd_soc_dai_ops mt76xx_i2s_dai_ops = {
	.startup   = mt76xx_i2s_startup,
	.hw_params = mt76xx_i2s_hw_params,
	.hw_free   = mt76xx_i2s_hw_free,
	//.shutdown = mt76xx_i2s_shutdown,
	.prepare   = mt76xx_i2s_prepare,
	.set_fmt   = mt76xx_i2s_set_fmt,
	//.set_sysclk = mt76xx_i2s_set_sysclk,
};
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,20)
const struct snd_soc_component_driver mt76xx_i2s_component = {
	.name		= "mt76xx-i2s",
};
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38)
struct snd_soc_dai_driver mt76xx_i2s_dai = {
#else
struct snd_soc_dai mt76xx_i2s_dai = {
	.name = "mtk-i2s",
#endif
	.playback = {
		.channels_min = 1,
		.channels_max = 2,
		.rates = (SNDRV_PCM_RATE_8000|SNDRV_PCM_RATE_11025|\
		SNDRV_PCM_RATE_16000|SNDRV_PCM_RATE_22050|SNDRV_PCM_RATE_32000|\
		SNDRV_PCM_RATE_44100|SNDRV_PCM_RATE_48000),

		.formats = (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE |\
				SNDRV_PCM_FMTBIT_S24_LE),
	},
	.capture = {
		.channels_min = 1,
		.channels_max = 2,
		.rates = (SNDRV_PCM_RATE_8000|SNDRV_PCM_RATE_11025|\
				SNDRV_PCM_RATE_16000|SNDRV_PCM_RATE_22050|SNDRV_PCM_RATE_32000|\
				SNDRV_PCM_RATE_44100|SNDRV_PCM_RATE_48000),
		.formats = (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE |\
				SNDRV_PCM_FMTBIT_S24_LE),
	},
	.ops = &mt76xx_i2s_dai_ops,
	.symmetric_rates = 1,
};

/****************************/
/*FUNCTION BODY				*/
/****************************/

static int mt76xx_i2s_set_fmt(struct snd_soc_dai *cpu_dai,
		unsigned int fmt)
{//TODO

	//printk("******* %s *******\n", __func__);
	return 0;
}

static int mt76xx_i2s_play_prepare(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
	//printk("******* %s *******\n", __func__);
	i2s_config_type* rtd = (i2s_config_type*)substream->runtime->private_data;
	rtd->pss[SNDRV_PCM_STREAM_PLAYBACK] = substream;
	if(! rtd->i2sStat[SNDRV_PCM_STREAM_PLAYBACK]){
		i2s_reset_tx_param( rtd);
		i2s_tx_config( rtd);
		gdma_En_Switch(rtd, STREAM_PLAYBACK, GDMA_I2S_EN);

		if( rtd->bRxDMAEnable==0)
			i2s_clock_enable( rtd);
		
		i2s_tx_enable( rtd);
		rtd->i2sStat[SNDRV_PCM_STREAM_PLAYBACK] = 1;
		MSG("I2S_TXENABLE done\n");
	}

	return 0;
}

static int mt76xx_i2s_rec_prepare(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{

	//printk("******* %s *******\n", __func__);
	i2s_config_type* rtd = (i2s_config_type*)substream->runtime->private_data;
	rtd->pss[SNDRV_PCM_STREAM_CAPTURE] = substream;
	if(! rtd->i2sStat[SNDRV_PCM_STREAM_CAPTURE]) {
		i2s_reset_rx_param(rtd);
		i2s_rx_config(rtd);
		gdma_En_Switch(rtd, STREAM_CAPTURE, GDMA_I2S_EN);

		if(rtd->bTxDMAEnable==0)
			i2s_clock_enable(rtd);

		i2s_rx_enable(rtd);
		rtd->i2sStat[SNDRV_PCM_STREAM_CAPTURE] = 1;
	}
	return 0;
}

static int  mt76xx_i2s_shutdown(struct snd_pcm_substream *substream,
		       struct snd_soc_dai *dai)
{
	//i2s_config_type* rtd = (i2s_config_type*)substream->runtime->private_data;
	//printk("******* %s *******\n", __func__);
	return 0;
}

static int  mt76xx_i2s_startup(struct snd_pcm_substream *substream,
		       struct snd_soc_dai *dai)
{

	//printk("******* %s *******\n", __func__);
    	if((!pi2s_config->i2sStat[SNDRV_PCM_STREAM_PLAYBACK]) && (!pi2s_config->i2sStat[SNDRV_PCM_STREAM_CAPTURE])){
		i2s_startup();
    		if(!pi2s_config)
    			return -1;
    		i2s_reset_config(pi2s_config);
    	}
	substream->runtime->private_data = pi2s_config;
	return 0;
}

static int mt76xx_i2s_hw_params(struct snd_pcm_substream *substream,\
				struct snd_pcm_hw_params *params,\
				struct snd_soc_dai *dai){
	unsigned int srate = 0;
	unsigned long data;
	struct snd_pcm_runtime *runtime = substream->runtime;
	i2s_config_type* rtd = runtime->private_data;

	//printk("******* %s *******\n", __func__);
	switch(params_rate(params)){
	case 8000:
		srate = 8000;
		break;
	case 16000:
		srate = 16000;
		break;
	case 32000:
		srate = 32000;
		break;
	case 44100:
		srate = 44100;
		break;
	case 48000:
		srate = 48000;
		break;
	default:
		srate = 44100;
		//MSG("audio sampling rate %u should be %d ~ %d Hz\n", (u32)params_rate(params), MIN_SRATE_HZ, MAX_SRATE_HZ);
		break;
	}
	if(srate){
		if((rtd->bRxDMAEnable != GDMA_I2S_EN) && (rtd->bTxDMAEnable != GDMA_I2S_EN)){
			rtd->srate = srate;
			MSG("set audio sampling rate to %d Hz\n", rtd->srate);
		}
	}

	return 0;
}
static int mt76xx_i2s_hw_free(struct snd_pcm_substream *substream,struct snd_soc_dai *dai){

	//printk("******* %s *******\n", __func__);
	i2s_config_type* rtd = (i2s_config_type*)substream->runtime->private_data;
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK){
		if(rtd->i2sStat[SNDRV_PCM_STREAM_PLAYBACK]){
			MSG("I2S_TXDISABLE\n");
			i2s_reset_tx_param(rtd);

			if((rtd->bRxDMAEnable==0)&&(rtd->bTxDMAEnable==0)){
				i2s_clock_disable(rtd);
			}
			rtd->i2sStat[SNDRV_PCM_STREAM_PLAYBACK] = 0;
		}
	}
	else{
		if(rtd->i2sStat[SNDRV_PCM_STREAM_CAPTURE]){
			MSG("I2S_RXDISABLE\n");
			i2s_reset_rx_param(rtd);
			
			if((rtd->bRxDMAEnable==0)&&(rtd->bTxDMAEnable==0)){
				i2s_clock_disable(rtd);
			}
			rtd->i2sStat[SNDRV_PCM_STREAM_CAPTURE] = 0;
		}
	}
	return 0;
}
static int mt76xx_i2s_prepare(struct snd_pcm_substream *substream,struct snd_soc_dai *dai)
{

	//printk("******* %s *******\n", __func__);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		return mt76xx_i2s_play_prepare(substream, dai);
	else
		return mt76xx_i2s_rec_prepare(substream, dai);

	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,20)
static int mt76xx_i2s_drv_probe(struct platform_device *pdev)
{
	//printk("****** %s ******\n", __func__);
	return snd_soc_register_component(&pdev->dev, &mt76xx_i2s_component,
					&mt76xx_i2s_dai, 1);
}

static void mt76xx_i2s_drv_remove(struct platform_device *pdev)
{
	snd_soc_unregister_component(&pdev->dev);	
}

static struct platform_driver mt76xx_i2s_driver = {
	.probe  = mt76xx_i2s_drv_probe,
	.remove = mt76xx_i2s_drv_remove,
	.driver = {
		.name  = "mt76xx-i2s",
		.owner = THIS_MODULE,
	},
};

static int __init mt76xx_i2s_init(void)
{

	//printk("****** %s ******\n", __func__);
	return platform_driver_register(&mt76xx_i2s_driver);
}

static void __exit mt76xx_i2s_exit(void)
{
	//printk("****** %s ******\n", __func__);
	platform_driver_unregister(&mt76xx_i2s_driver);
}

module_init(mt76xx_i2s_init);
module_exit(mt76xx_i2s_exit);

MODULE_AUTHOR("Dora Chen");
MODULE_DESCRIPTION("Stretch MT76xx I2S Interface");
MODULE_LICENSE("GPL");
#endif
