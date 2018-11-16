/*
 * mt76xx_pcm.c
 *
 *  Created on: 2013/9/6
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

#define GDMA_PAGE_SIZE 		I2S_PAGE_SIZE
#define GDMA_PAGE_NUM 		MAX_I2S_PAGE
#define GDMA_TOTAL_PAGE_SIZE	I2S_TOTAL_PAGE_SIZE

dma_addr_t i2s_txdma_addr, i2s_rxdma_addr;
dma_addr_t i2s_mmap_addr[GDMA_PAGE_NUM*2];

extern struct tasklet_struct i2s_tx_tasklet;
extern struct tasklet_struct i2s_rx_tasklet;
extern int i2s_mmap_remap(struct vm_area_struct *vma, unsigned long size);
extern void i2s_tx_end_sleep_on(i2s_config_type* ptri2s_config);
extern void i2s_rx_end_sleep_on(i2s_config_type* ptri2s_config);

static int mt76xx_pcm_open(struct snd_pcm_substream *substream);
static int mt76xx_pcm_new(struct snd_card *card,\
	struct snd_soc_dai *dai, struct snd_pcm *pcm);
static void mt76xx_pcm_free(struct snd_pcm *pcm);
static int mt76xx_pcm_close(struct snd_pcm_substream *substream);
static snd_pcm_uframes_t mt76xx_pcm_pointer(struct snd_pcm_substream *substream);
static int mt76xx_pcm_trigger(struct snd_pcm_substream *substream, int cmd);
static int mt76xx_pcm_prepare(struct snd_pcm_substream *substream);
static int mt76xx_pcm_hw_params(struct snd_pcm_substream *substream,\
				 struct snd_pcm_hw_params *hw_params);
static int mt76xx_pcm_copy(struct snd_pcm_substream *substream, int channel,\
		snd_pcm_uframes_t pos,void __user *buf, snd_pcm_uframes_t count);
static int mt76xx_pcm_mmap(struct snd_pcm_substream *substream, struct vm_area_struct *vma);
static int mt76xx_pcm_hw_free(struct snd_pcm_substream *substream);

static int mt76xx_pcm_free_dma_buffer(struct snd_pcm_substream *substream,int stream);
static int mt76xx_pcm_allocate_dma_buffer(struct snd_pcm_substream *substream,int stream);

#if LINUX_VERSION_CODE > KERNEL_VERSION(3,10,20)
static int mt76xx_platform_drv_probe(struct platform_device *pdev);
static int mt76xx_platform_drv_remove(struct platform_device *pdev);
#endif

static const struct snd_pcm_hardware mt76xx_pcm_hwparam = {
#if defined(CONFIG_I2S_MMAP)
	.info			= (SNDRV_PCM_INFO_INTERLEAVED |
				SNDRV_PCM_INFO_PAUSE |
				SNDRV_PCM_INFO_RESUME |
				SNDRV_PCM_INFO_MMAP |
				SNDRV_PCM_INFO_MMAP_VALID),
#else
	.info			= (SNDRV_PCM_INFO_INTERLEAVED |
				SNDRV_PCM_INFO_PAUSE |
				SNDRV_PCM_INFO_RESUME),
#endif
	.formats		= SNDRV_PCM_FMTBIT_S16_LE,
	.period_bytes_min	= GDMA_PAGE_SIZE,
	.period_bytes_max	= GDMA_PAGE_SIZE,
	.periods_min		= 1,
	.periods_max		= GDMA_PAGE_NUM,
	.buffer_bytes_max	= GDMA_TOTAL_PAGE_SIZE,
};

static struct snd_pcm_ops mt76xx_pcm_ops = {

	.open = 	mt76xx_pcm_open,
	.ioctl = 	snd_pcm_lib_ioctl,
	.hw_params = 	mt76xx_pcm_hw_params,
	.hw_free = 	mt76xx_pcm_hw_free,
	.trigger =	mt76xx_pcm_trigger,
	.prepare = 	mt76xx_pcm_prepare,
	.pointer = 	mt76xx_pcm_pointer,
	.close = 	mt76xx_pcm_close,
#if defined(CONFIG_I2S_MMAP)
	.mmap = mt76xx_pcm_mmap,
#endif
	.copy = mt76xx_pcm_copy,
};
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,10,0)
struct snd_soc_platform_driver mt76xx_soc_platform = {
	.ops		= &mt76xx_pcm_ops,
	.pcm_new	= mt76xx_pcm_new,
	.pcm_free	= mt76xx_pcm_free,
};
#else
struct snd_soc_platform mt76xx_soc_platform = {
	.name		= "mtk-dma",
	.pcm_ops	= &mt76xx_pcm_ops,
	.pcm_new	= mt76xx_pcm_new,
	.pcm_free	= mt76xx_pcm_free,
};
#endif

static int mt76xx_pcm_close(struct snd_pcm_substream *substream){

	//printk("******* %s *********\n", __func__);
	return 0;
}

static snd_pcm_uframes_t mt76xx_pcm_pointer(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	i2s_config_type* rtd = runtime->private_data;
	unsigned int offset = 0;
	int buff_frame_bond = bytes_to_frames(runtime, GDMA_PAGE_SIZE);
	//printk("\n******* %s *********\n", __func__);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK){
		offset = bytes_to_frames(runtime, GDMA_PAGE_SIZE*rtd->tx_r_idx);
		//printk("r:%d w:%d (%d) \n",rtd->tx_r_idx,rtd->tx_w_idx,(runtime->control->appl_ptr/buff_frame_bond)%GDMA_PAGE_NUM);
	}
	else{
		offset = bytes_to_frames(runtime, GDMA_PAGE_SIZE*rtd->rx_w_idx);
		//printk("w:%d r:%d appl_ptr:%x\n",rtd->rx_w_idx,rtd->rx_r_idx,(runtime->control->appl_ptr/buff_frame_bond)%GDMA_PAGE_NUM);
	}
	return offset;
}


static int mt76xx_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
	int ret = 0;
	i2s_config_type* rtd = (i2s_config_type*)substream->runtime->private_data;
	struct snd_pcm_runtime *runtime= substream->runtime;

	//printk("******* %s *********\n", __func__);
	printk("trigger cmd:%s\n",(cmd==SNDRV_PCM_TRIGGER_START)?"START":\
			(cmd==SNDRV_PCM_TRIGGER_RESUME)?"RESUME":\
			(cmd==SNDRV_PCM_TRIGGER_PAUSE_RELEASE)?"PAUSE_RELEASE":\
			(cmd==SNDRV_PCM_TRIGGER_STOP)?"STOP":\
			(cmd==SNDRV_PCM_TRIGGER_SUSPEND)?"SUSPEND":\
			(cmd==SNDRV_PCM_TRIGGER_PAUSE_PUSH)?"PAUSE_PUSH":"default");

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			rtd->bTrigger[SNDRV_PCM_STREAM_PLAYBACK] = 1;
		} else {
			rtd->bTrigger[SNDRV_PCM_STREAM_CAPTURE] = 1;
		}
		break;
	case SNDRV_PCM_TRIGGER_STOP:
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			rtd->bTrigger[SNDRV_PCM_STREAM_PLAYBACK] = 0;
		} else {
			rtd->bTrigger[SNDRV_PCM_STREAM_CAPTURE] = 0;
		}
		break;
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK){
			rtd->tx_pause_en = 0;
		} else {
			rtd->rx_pause_en = 0;
		}
		break;

	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK){
			rtd->tx_pause_en = 1;
		} else {
			rtd->rx_pause_en = 1;
		}
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

static int mt76xx_pcm_copy(struct snd_pcm_substream *substream, int channel,\
		snd_pcm_uframes_t pos,void __user *buf, snd_pcm_uframes_t count)
{
	struct snd_pcm_runtime *runtime= substream->runtime;
	i2s_config_type* rtd = runtime->private_data;
	int tx_w_idx = 0;
        int rx_r_idx = 0;
        char *hwbuf = NULL;

	//printk("******* %s *********\n", __func__);
	hwbuf = runtime->dma_area + frames_to_bytes(runtime, pos);
	//MSG("%s bur:%x\n",__func__,hwbuf);
	//printk("hw_ptr:%d, buffer_size:%d, appl_prt:%d, boundary:%d\n", 
	//		runtime->status->hw_ptr, runtime->buffer_size, runtime->control->appl_ptr, runtime->boundary);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK){
		rtd->tx_w_idx = (rtd->tx_w_idx+1)%MAX_I2S_PAGE;
                tx_w_idx = rtd->tx_w_idx;
                //printk("put TB[%d - %x] for user write\n",rtd->tx_w_idx,pos);
                copy_from_user(rtd->pMMAPTxBufPtr[tx_w_idx], (char*)buf, I2S_PAGE_SIZE);	
	}
	else{
		rx_r_idx = rtd->rx_r_idx;
                rtd->rx_r_idx = (rtd->rx_r_idx+1)%MAX_I2S_PAGE;
                copy_to_user((char*)buf, rtd->pMMAPRxBufPtr[rx_r_idx], I2S_PAGE_SIZE);
	}
	return 0;
}

static int mt76xx_pcm_mmap(struct snd_pcm_substream *substream, struct vm_area_struct *vma)
{
        int ret;
        unsigned long size;

        size = vma->vm_end-vma->vm_start;
        printk("******* %s: size :%x end:%x start:%x *******\n", __func__,size,vma->vm_end,vma->vm_start);
        ret = i2s_mmap_remap(vma, size);

        return ret;
}


static int mt76xx_pcm_prepare(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime= substream->runtime;
	i2s_config_type *rtd = (i2s_config_type*)runtime->private_data;
	//runtime->buffer_size = GDMA_PAGE_NUM*GDMA_PAGE_SIZE;
	//runtime->boundary = (GDMA_PAGE_NUM*GDMA_PAGE_SIZE)/4;

	//printk("******* %s *******\n", __func__);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK){
		//printk("===== %s:%s:%d =====\n", __FILE__, __func__, __LINE__);
		mt76xx_pcm_allocate_dma_buffer(substream,SNDRV_PCM_STREAM_PLAYBACK);
		
		if(! rtd->dmaStat[SNDRV_PCM_STREAM_PLAYBACK]){
			i2s_page_prepare(rtd,STREAM_PLAYBACK);
			tasklet_init(&i2s_tx_tasklet, i2s_tx_task, (u32)rtd);
			rtd->dmaStat[SNDRV_PCM_STREAM_PLAYBACK] = 1;
			gdma_unmask_handler(GDMA_I2S_TX0);
		}
	} else {
		mt76xx_pcm_allocate_dma_buffer(substream,SNDRV_PCM_STREAM_CAPTURE);

		if(! rtd->dmaStat[SNDRV_PCM_STREAM_CAPTURE]){
			i2s_page_prepare(rtd,STREAM_CAPTURE); /* TX:enLabel=1; RX:enLabel=2 */
			tasklet_init(&i2s_rx_tasklet, i2s_rx_task, (u32)rtd);
			rtd->dmaStat[SNDRV_PCM_STREAM_CAPTURE] = 1;
			gdma_unmask_handler(GDMA_I2S_RX0);
		}
	}

	return 0;
}


static int mt76xx_pcm_hw_params(struct snd_pcm_substream *substream,
				 struct snd_pcm_hw_params *hw_params)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	i2s_config_type *rtd = (i2s_config_type*)runtime->private_data;
	int ret,i;
	ret = i = 0;

	//printk("******* %s *******\n", __func__);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK){
		//i2s_page_prepare(rtd,STREAM_PLAYBACK);
	} else {
		//i2s_page_prepare(rtd,STREAM_CAPTURE);
	}

	return ret;
}

static int mt76xx_pcm_hw_free(struct snd_pcm_substream *substream)
{
	i2s_config_type* rtd = (i2s_config_type*)substream->runtime->private_data;
	struct snd_dma_buffer *buf = &substream->dma_buffer;

	//printk("******* %s *******\n", __func__);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK){
		if(rtd->dmaStat[SNDRV_PCM_STREAM_PLAYBACK]){

			gdma_En_Switch(rtd,STREAM_PLAYBACK,GDMA_I2S_DIS);
			i2s_tx_end_sleep_on(rtd);
			tasklet_kill(&i2s_tx_tasklet);
			i2s_tx_disable(rtd);
			//mt76xx_pcm_free_dma_buffer(substream,substream->stream);
			i2s_page_release(rtd,STREAM_PLAYBACK);
			rtd->dmaStat[SNDRV_PCM_STREAM_PLAYBACK] = 0;
		}
		mt76xx_pcm_free_dma_buffer(substream,substream->stream);
	}
	else{
		if(rtd->dmaStat[SNDRV_PCM_STREAM_CAPTURE]){

			gdma_En_Switch(rtd,STREAM_CAPTURE,GDMA_I2S_DIS);
			i2s_tx_end_sleep_on(rtd);
			tasklet_kill(&i2s_rx_tasklet);
			i2s_rx_disable(rtd);
			//mt76xx_pcm_free_dma_buffer(substream,substream->stream);
			i2s_page_release(rtd,STREAM_CAPTURE);
			rtd->dmaStat[SNDRV_PCM_STREAM_CAPTURE] = 0;
		}
		mt76xx_pcm_free_dma_buffer(substream,substream->stream);
	}
	return 0;
}

static int mt76xx_pcm_free_dma_buffer(struct snd_pcm_substream *substream,
	int stream)
{

	//struct snd_pcm_substream *substream = pcm->streams[stream].substream;
	struct snd_dma_buffer *buf = &substream->dma_buffer;
	i2s_config_type* rtd = (i2s_config_type*)substream->runtime->private_data;

	//printk("******* %s *******\n", __func__);
	if (!buf->area)
		return 0;
	if(substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		i2s_memPool_free(rtd,STREAM_PLAYBACK);
	else
		i2s_memPool_free(rtd,STREAM_CAPTURE);
	buf->area = NULL;
	snd_pcm_set_runtime_buffer(substream, NULL);
	return 0;
}

static int mt76xx_pcm_allocate_dma_buffer(struct snd_pcm_substream *substream,
	int stream)
{
	//struct snd_pcm_substream *substream = pcm->streams[stream].substream;
	struct snd_dma_buffer *buf = &substream->dma_buffer;
	i2s_config_type* rtd = (i2s_config_type*)substream->runtime->private_data;

	//printk("******* %s *******\n", __func__);
	if(!buf->area){
#if defined(CONFIG_I2S_MMAP)
		printk("\n############## MMAP ##############\n");
		buf->dev.type = SNDRV_DMA_TYPE_DEV;
#else
		buf->dev.type = SNDRV_DMA_TYPE_UNKNOWN;
#endif
		buf->dev.dev = NULL;
		buf->private_data = NULL;
		if(stream == SNDRV_PCM_STREAM_PLAYBACK)
			buf->area = i2s_memPool_Alloc(rtd,STREAM_PLAYBACK);
		else
			buf->area = i2s_memPool_Alloc(rtd,STREAM_CAPTURE);

		if (!buf->area)
			return -ENOMEM;
		buf->bytes = GDMA_TOTAL_PAGE_SIZE;
#if defined(CONFIG_I2S_MMAP)
		buf->addr = i2s_mmap_phys_addr(rtd);
#endif
		snd_pcm_set_runtime_buffer(substream, buf);
	} else{
		printk("Buffer have been allocated!\n");
	}

	return 0;
}

static int mt76xx_pcm_open(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime= substream->runtime;
	struct snd_dma_buffer *buf = &substream->dma_buffer;
	int stream = substream->stream;
	int ret = 0;

	//printk("******* %s *******\n", __func__);
	snd_soc_set_runtime_hwparams(substream, &mt76xx_pcm_hwparam);
	/* ensure that buffer size is a multiple of period size */
	ret = snd_pcm_hw_constraint_integer(runtime,
						SNDRV_PCM_HW_PARAM_PERIODS);
	if (ret < 0)
		goto out;

#if 1
	if(stream == SNDRV_PCM_STREAM_PLAYBACK){
		ret = mt76xx_pcm_allocate_dma_buffer(substream,
				SNDRV_PCM_STREAM_PLAYBACK);
	}
	else{
		ret = mt76xx_pcm_allocate_dma_buffer(substream,
				SNDRV_PCM_STREAM_CAPTURE);
	}
#endif

	if (ret)
		goto out;

	if(buf)
		memset(buf->area,0,sizeof(I2S_PAGE_SIZE*MAX_I2S_PAGE));

 out:
	return ret;
}



static int mt76xx_pcm_new(struct snd_card *card,
	struct snd_soc_dai *dai, struct snd_pcm *pcm)
{
	int ret = 0;

	//printk("******* %s *******\n", __func__);
	return 0;
}

static void mt76xx_pcm_free(struct snd_pcm *pcm)
{
	struct snd_pcm_substream *substream;
	struct snd_dma_buffer *buf;
	i2s_config_type* rtd;
	int stream;

	//printk("******* %s *******\n", __func__);
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,20)
static int mt76xx_platform_drv_probe(struct platform_device *pdev)
{
	//printk("******* %s *******\n", __func__);
	return snd_soc_register_platform(&pdev->dev, &mt76xx_soc_platform);
}

static int mt76xx_platform_drv_remove(struct platform_device *pdev)
{
	//printk("******* %s *******\n", __func__);
	snd_soc_unregister_platform(&pdev->dev);
	return 0;
}

static struct platform_driver mt76xx_pcm_driver = {
	.driver = {
		.name = "mt76xx-pcm",
		.owner = THIS_MODULE,
	},	

	.probe = mt76xx_platform_drv_probe,
	.remove = mt76xx_platform_drv_remove,
};	

static int __init mt76xx_pcm_init(void)
{

	printk("******* %s *******\n", __func__);
	return platform_driver_register(&mt76xx_pcm_driver);
}

static void __exit mt76xx_pcm_exit(void)
{
	platform_driver_unregister(&mt76xx_pcm_driver);
}
#else
static int __init mt76xx_pcm_init(void)
{

	printk("******* %s *******\n", __func__);
	return snd_soc_register_platform(&mt76xx_soc_platform);
}

static void __exit mt76xx_pcm_exit(void)
{
	printk("******* %s *******\n", __func__);
	snd_soc_unregister_platform(&mt76xx_soc_platform);
}
#endif
module_init(mt76xx_pcm_init);
module_exit(mt76xx_pcm_exit);

MODULE_AUTHOR("Dora Chen");
MODULE_DESCRIPTION("MTK APSoC I2S DMA driver");
MODULE_LICENSE("GPL");

