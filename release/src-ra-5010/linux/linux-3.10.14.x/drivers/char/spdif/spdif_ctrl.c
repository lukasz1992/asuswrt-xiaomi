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
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
#include <asm/system.h> /* cli(), *_flags */
#endif
#include <asm/uaccess.h> /* copy_from/to_user */
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <asm/rt2880/surfboardint.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <sound/pcm.h>

#include "spdif_ctrl.h"

/******************************************
* Function declaration
******************************************/
#ifdef  CONFIG_DEVFS_FS
#include <linux/devfs_fs_kernel.h>
static  devfs_handle_t devfs_handle;
#endif

static int spdifdrv_major =  236;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
#else
static struct class *spdifmodule_class;
#endif

/* internal function declarations */
irqreturn_t spdif_dma_tx_handler(int irq, void *irqaction);

/* forward declarations for _fops */
static long spdif_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static int spdif_open(struct inode *inode, struct file *file);
static int spdif_release(struct inode *inode, struct file *file);
#if defined(CONFIG_SPDIF_MMAP)
static int spdif_mmap(struct file *file, struct vm_area_struct *vma);
#endif

/* global varable definitions */
spdif_config_type spdif_config;
spdif_status_type spdif_status;
spdif_config_type* pspdif_config = &spdif_config;
spdif_status_type* pspdif_status = &spdif_status;

#if defined(CONFIG_SPDIF_MMAP)
static dma_addr_t spdif_mmap_addr[MAX_SPDIF_PAGE+1];
#endif

#if defined(SPDIF_AC3_DEBUG)
typedef struct ac3_si
{
	uint16_t syncword: 16;
	uint16_t crc1: 16;
	//uint16_t frmsizecod: 8;
	uint16_t crc2: 16;
	uint16_t crc3: 16;
	uint16_t crc4: 16;
	uint16_t crc5: 16;
	uint16_t crc6: 16;
	uint16_t crc7: 16;

} ac3_si_t;
#endif

#if defined (CONFIG_RALINK_MT7621)
			/* 22.05k  44.1k  88.2k  176.4k   24k   48k   96k   192k */	
unsigned long mas_div[8] = { 48,     24,    12,     6,     48,   24,   12,    6}; 
unsigned long iec_div[8] = {  2,      2,     2,     2,      2,    2,    2,    2};
unsigned long bit_div[8] = {  4,      4,     4,     4,      4,    4,    4,    4};
#endif

static const struct file_operations spdif_fops = {
        owner           : THIS_MODULE,
        open            : spdif_open,
        release         : spdif_release,
#if defined(CONFIG_SPDIF_MMAP)
	mmap		: spdif_mmap,
#endif
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
        unlocked_ioctl	: spdif_ioctl,
#else
        ioctl           : spdif_ioctl,
#endif
};




int __init spdif_mod_init(void)
{
#ifdef CONFIG_DEVFS_FS
#else
	int result;
#endif	
        /* register device with kernel */
	printk("%s: start SPDIF module\n",__FUNCTION__);
	
#ifdef  CONFIG_DEVFS_FS
    	if(devfs_register_chrdev(spdifdrv_major, SPDIFDRV_DEVNAME , &spdif_fops)) {
                printk(KERN_WARNING " spdif: can't create device node - %s\n", SPDIFDRV_DEVNAME);
                return -EIO;
    	}

    	devfs_handle = devfs_register(NULL, SPDIFDRV_DEVNAME, DEVFS_FL_DEFAULT, spdifdrv_major, 0,
            	S_IFCHR | S_IRUGO | S_IWUGO, &spdif_fops, NULL);
#else
	result = register_chrdev(spdifdrv_major, SPDIFDRV_DEVNAME, &spdif_fops);
    	if (result < 0) {
		printk(KERN_WARNING "spdif: can't get major %d\n",spdifdrv_major);
        	return result;
    	}

    	if (spdifdrv_major == 0) {
                spdifdrv_major = result; /* dynamic */
    	}
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
#else
 	spdifmodule_class = class_create(THIS_MODULE, SPDIFDRV_DEVNAME);
        if (IS_ERR(spdifmodule_class))
                return -EFAULT;
        device_create(spdifmodule_class, NULL, MKDEV(spdifdrv_major, 0), SPDIFDRV_DEVNAME);
#endif

        return 0;
}




void spdif_mod_exit(void)
{
	printk("%s: stop SPDIF module\n",__FUNCTION__);

#ifdef  CONFIG_DEVFS_FS
    	devfs_unregister_chrdev(spdifdrv_major, SPDIFDRV_DEVNAME);
    	devfs_unregister(devfs_handle);
#else
    	unregister_chrdev(spdifdrv_major, SPDIFDRV_DEVNAME);
#endif
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
#else
        device_destroy(spdifmodule_class,MKDEV(spdifdrv_major, 0));
        class_destroy(spdifmodule_class);
#endif
        return ;
}




static int spdif_open(struct inode *inode, struct file *filp)
{
        int minor = iminor(inode);

        if (minor >= SPDIF_MAX_DEV)
                return -ENODEV;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
        MOD_INC_USE_COUNT;
#else
        try_module_get(THIS_MODULE);
#endif

        if (filp->f_flags & O_NONBLOCK) {
                MSG("filep->f_flags O_NONBLOCK set\n");
                return -EAGAIN;
        }

#if 0
        /* set spdif_config */
        pspdif_config = (spdif_config_type*)kmalloc(sizeof(spdif_config_type), GFP_KERNEL);
        if(pspdif_config == NULL)
                return -1;
#endif

	filp->private_data = pspdif_config;
        memset(pspdif_config, 0, sizeof(spdif_config_type));

#ifdef SPDIF_STATISTIC
#if 0
        pspdif_status = (spdif_status_type*)kmalloc(sizeof(spdif_status_type), GFP_KERNEL);
        if(pspdif_status == NULL)
                return -1;
#endif
        memset(pspdif_status, 0, sizeof(spdif_status_type));
#endif
	// register irq num
	spdif_irq_request();

	// Reset SPDIF block: first reset(1) this bit, then deasset the reset (0) 
	spdif_reset();

	/*  set share pins to spdif mode */
	spdif_share_pin_config();

	// Init IEC_CTRL regiater
	pspdif_config->data_src = 1;
	
	init_waitqueue_head(&(pspdif_config->spdif_tx_qh));
        
	return 0;
}


static int spdif_release(struct inode *inode, struct file *filp)
{
	spdif_config_type *pspdif_config;

	/* Decrement usage count */
	module_put(THIS_MODULE);

	pspdif_config = filp->private_data;

	if (pspdif_config == NULL) 
		goto EXIT;

#if defined(CONFIG_SPDIF_MMAP)
	spdif_mem_unmap(pspdif_config);
#else
	spdif_txPagebuf_free(pspdif_config);
#endif

	spdif_irq_free();

EXIT:
	MSG("spdif_release succeeds\n");
	return 0;
}

#if defined(CONFIG_SPDIF_MMAP)
static int spdif_mmap(struct file *file, struct vm_area_struct *vma)
{
	int nRet;
	unsigned long size = vma->vm_end - vma->vm_start;
	printk("buf_size=%d, size=%d\n", pspdif_config->buf_size, size);

	if((pspdif_config->pMMAPBufPtr[0]==NULL) && (pspdif_config->mmap_index!=0))
		pspdif_config->mmap_index = 0;

	if(pspdif_config->pMMAPBufPtr[pspdif_config->mmap_index]!=NULL)
		goto EXIT;

#if 1
	pspdif_config->pMMAPBufPtr[pspdif_config->mmap_index] = kmalloc(size, GFP_DMA);
	spdif_mmap_addr[pspdif_config->mmap_index] = (dma_addr_t)dma_map_single(NULL, pspdif_config->pMMAPBufPtr[pspdif_config->mmap_index], size, DMA_BIDIRECTIONAL);
#else
	pspdif_config->pMMAPBufPtr[pspdif_config->mmap_index] = dma_alloc_coherent(NULL, size, &spdif_mmap_addr[pspdif_config->mmap_index], GFP_KERNEL);
#endif

	MSG("MMAP[%d]=0x%08X, vm_start=0x%08X, vm_end=0x%08X, spdif_mmap_addr[%d]=0x%08x\n", pspdif_config->mmap_index, (u32)pspdif_config->pMMAPBufPtr[pspdif_config->mmap_index], (u32)vma->vm_start, (u32)vma->vm_end, pspdif_config->mmap_index, spdif_mmap_addr[pspdif_config->mmap_index]);

	if(pspdif_config->pMMAPBufPtr[pspdif_config->mmap_index] == NULL)
	{
		MSG("spdif_mmap failed\n");
		return -1;
	}
EXIT:
	memset(pspdif_config->pMMAPBufPtr[pspdif_config->mmap_index], 0, size);

	nRet = remap_pfn_range(vma, vma->vm_start, virt_to_phys((void *)pspdif_config->pMMAPBufPtr[pspdif_config->mmap_index]) >> PAGE_SHIFT, size, vma->vm_page_prot);

	if(nRet != 0)
	{
		MSG("spdif_mmap->remap_pfn_range failed\n");
		return -EIO;
	}
	pspdif_config->mmap_index++;

	if(pspdif_config->mmap_index > (MAX_SPDIF_PAGE+1))
		pspdif_config->mmap_index = 0;

	return 0;
}

int spdif_mem_unmap(spdif_config_type* pspdif_config)
{
	int i;

	for(i=0; i<MAX_SPDIF_PAGE; i++)
	{
		if(pspdif_config->pMMAPBufPtr[i])
		{
			printk("unmap MMAP[%d]=0x%08X\n", i, (u32)pspdif_config->pMMAPBufPtr[i]);
			if(pspdif_config->buf_size >= SPDIF_MIN_PAGE_SIZE)
				dma_unmap_single(NULL, spdif_mmap_addr[i], pspdif_config->buf_size, DMA_BIDIRECTIONAL);
			else
				dma_unmap_single(NULL, spdif_mmap_addr[i], SPDIF_MIN_PAGE_SIZE, DMA_BIDIRECTIONAL);
		}
	}
	pspdif_config->mmap_index = 0;

	return 0;
}
#endif

#if defined(CONFIG_RALINK_MT7621)
int spdif_pll_config(unsigned long index)
{
	unsigned long data;
        unsigned long regValue;
        bool xtal_20M_en = 0;
//      bool xtal_25M_en = 0;
        bool xtal_40M_en = 0;

        regValue = spdif_inw(RALINK_SYSCTL_BASE + 0x10);
        regValue = (regValue >> 6) & 0x7;
        if (regValue < 3)
        {
                xtal_20M_en = 1;
                printk("Xtal is 20MHz. \n");
        }
        else if (regValue < 6)
        {
                xtal_40M_en = 1;
                printk("Xtal is 40M.\n");
        }
        else
        {
                //xtal_25M_en = 1;
                printk("Xtal is 25M.\n");
        }

	/* Firstly, reset all required register to default value */
	spdif_outw(RALINK_ANA_CTRL_BASE, 0x00008000);
	spdif_outw(RALINK_ANA_CTRL_BASE+0x0014, 0x01001d61);//0x01401d61);
	spdif_outw(RALINK_ANA_CTRL_BASE+0x0018, 0x38233d0e);
	spdif_outw(RALINK_ANA_CTRL_BASE+0x001c, 0x80100004);//0x80120004);
	spdif_outw(RALINK_ANA_CTRL_BASE+0x0020, 0x1c7dbf48);

	/* toggle RG_XPTL_CHG */
        spdif_outw(RALINK_ANA_CTRL_BASE, 0x00008800);
        spdif_outw(RALINK_ANA_CTRL_BASE, 0x00008c00);


        data = spdif_inw(RALINK_ANA_CTRL_BASE+0x0014);
        data &= ~(0x0000ffc0);
	if ((xtal_40M_en) || (xtal_20M_en))
	{
        	data |= REGBIT(0x1d, 8); /* for 40M or 20M */
	}
	else
	{
        	data |= REGBIT(0x17, 8); /* for 25M */
	}	
	if (xtal_40M_en)
	{
        	data |= REGBIT(0x1, 6); /* for 40M */
	}
        spdif_outw(RALINK_ANA_CTRL_BASE+0x0014, data);


        data = spdif_inw(RALINK_ANA_CTRL_BASE+0x0018);
        data &= ~(0xf0773f00);
        data |= REGBIT(0x3, 28);
        data |= REGBIT(0x2, 20);
	if ((xtal_40M_en) || (xtal_20M_en))
	{
        	data |= REGBIT(0x3, 16); /* for 40M or 20M */
	}
	else
	{
        	data |= REGBIT(0x2, 16); /* for 25M  */
	}
        data |= REGBIT(0x3, 12);
	if ((xtal_40M_en) || (xtal_20M_en))
	{
     	   	data |= REGBIT(0xd, 8);	/* for 40M or 20M */
	}
	else
	{
        	data |= REGBIT(0x7, 8); /* for 25M */
	}
        spdif_outw(RALINK_ANA_CTRL_BASE+0x0018, data);


        if(index <= 3)// 270 MHz for 22.05K, 44.1K, 88.2K, 176.4K
        {
		if ((xtal_40M_en) || (xtal_20M_en))
		{
                	spdif_outw(RALINK_ANA_CTRL_BASE+0x0020, 0x1a18548a); /* for 40M or 20M */
		}
		else
		{	
                	spdif_outw(RALINK_ANA_CTRL_BASE+0x0020, 0x14ad106e); /* for 25M */
		}
        }
        else if ((index > 3) && (index < 8))// 294 MHZ for 24K, 48K, 96K, 192K
        {
		if ((xtal_40M_en) || (xtal_20M_en))
		{
                	spdif_outw(RALINK_ANA_CTRL_BASE+0x0020, 0x1c7dbf48); /* for 40M or 20M */
		}
		else
		{
                	spdif_outw(RALINK_ANA_CTRL_BASE+0x0020, 0x1697cc39); /* for 25M */
		}
        }
        else
        {
                printk("Wrong sampling rate!\n");
                return -1;
        }

	/*Set PLLGP_CTRL_4 */
	/* 1. Bit 31 */
	data = spdif_inw(RALINK_ANA_CTRL_BASE+0x001c);
	data &= ~(REGBIT(0x1, 31));
  	spdif_outw(RALINK_ANA_CTRL_BASE+0x001c, data);
	ndelay(10);

	/* 2. Bit 0 */
	data = spdif_inw(RALINK_ANA_CTRL_BASE+0x001c);
	data |= REGBIT(0x1, 0);
	spdif_outw(RALINK_ANA_CTRL_BASE+0x001c, data);
	udelay(200);

	/* 3. Bit 3 */
	data = spdif_inw(RALINK_ANA_CTRL_BASE+0x001c);
	data |= REGBIT(0x1, 3);
	spdif_outw(RALINK_ANA_CTRL_BASE+0x001c, data);
	udelay(1);

	/* 4. Bit 8 */
	data = spdif_inw(RALINK_ANA_CTRL_BASE+0x001c);
	data |= REGBIT(0x1, 8);
	spdif_outw(RALINK_ANA_CTRL_BASE+0x001c, data);
	ndelay(40);

	/* 5. Bit 6 */
	data = spdif_inw(RALINK_ANA_CTRL_BASE+0x001c);
	data |= REGBIT(0x1, 6);
	spdif_outw(RALINK_ANA_CTRL_BASE+0x001c, data);
	ndelay(40);

	/* 6. Bit 5 & Bit 7*/
	data = spdif_inw(RALINK_ANA_CTRL_BASE+0x001c);
	data |= REGBIT(0x1, 5);
	data |= REGBIT(0x1, 7);
	spdif_outw(RALINK_ANA_CTRL_BASE+0x001c, data);
	udelay(1);

	/* 7. Bit 17 */
	data = spdif_inw(RALINK_ANA_CTRL_BASE+0x001c);
	data |= REGBIT(0x1, 17);
	spdif_outw(RALINK_ANA_CTRL_BASE+0x001c, data);

	return 0;
}
#endif

int spdif_clock_enable(spdif_config_type* pspdif_config)
{
        unsigned long index;
        
	/* audio sampling rate decision */
	printk("srate=%d\n", pspdif_config->srate);
        switch(pspdif_config->srate)
        {
                case 22050:
                        index = 0;
                        break;
                case 44100:
                        index = 1;
                        break;
		case 88200:
			index = 2;
			break;
		case 176400:
			index = 3;
			break;
		case 24000:
			index = 4;
			break;
                case 48000:
                        index = 5;
                        break;
                case 96000:
                        index = 6;
                        break;
                case 192000:
                        index = 7;
                        break;
                default:
                        index = 1;
        }

#if defined(CONFIG_RALINK_MT7621)
	/* PLL config  */
	spdif_pll_config(index);
#endif
	spdif_tx_clock_config();

	/* Set Audio clock divider */
	spdif_clock_divider_set(pspdif_config, index);

	return 0;	
}

int spdif_irq_request(void)
{
	int Ret;

	printk("Enter %s\n", __func__);
	// Init interrupt handler
	Ret = request_irq(SURFBOARDINT_SPDIF, spdif_dma_tx_handler, IRQF_DISABLED, "Ralink_SPDIF", NULL);

        if(Ret){
		printk("************SPDIF IRQ %d is not free.***********\n", SURFBOARDINT_SPDIF);
                return -1;
        }
	else
		printk("************Set SPDIF IRQ %d is done.***********\n", SURFBOARDINT_SPDIF);
	return 0;
}

int spdif_irq_free(void)
{
	printk("Enter %s\n", __func__);
	free_irq(SURFBOARDINT_SPDIF, NULL);

	return 0;
}

int spdif_clock_disable(spdif_config_type* pspdif_config)
{
	 unsigned long data;

        /* disable SPDIF TX clock */
        data = spdif_inw(RALINK_SYSCTL_BASE+0x30);
        data &= ~(0x1<<7);
        spdif_outw(RALINK_SYSCTL_BASE+0x30, data);
        
	return 0;
}

void spdif_reset(void)
{
	unsigned long data;

	// Reset SPDIF block: first reset(1) this bit, then deasset the reset (0)
        data = spdif_inw(RALINK_SYSCTL_BASE+0x34);
        data |= 0x00000080;
        spdif_outw(RALINK_SYSCTL_BASE+0x34, data);

        data = spdif_inw(RALINK_SYSCTL_BASE+0x34);
        data &= ~(0x00000080);
        spdif_outw(RALINK_SYSCTL_BASE+0x34, data);
}

void spdif_share_pin_config(void)
{
	unsigned long data;

	/*  set share pins to spdif mode */
#if 1 /* MT7621 EVB */
        data = spdif_inw(RALINK_SYSCTL_BASE+0x60);
        data &= 0xFFFFFFE7;
        data |= 0x00000018;
        spdif_outw(RALINK_SYSCTL_BASE+0x60, data);
#else /* MT7621 RFB */
	data = spdif_inw(RALINK_SYSCTL_BASE+0x60);
        data &= 0xFFFFFF3F;
        data |= 0x000000C0;
        spdif_outw(RALINK_SYSCTL_BASE+0x60, data);
#endif
}

#if 0
#if defined(CONFIG_SPDIF_MMAP)
int spdif_mmap_txbuf_alloc(spdif_config_type* pspdif_config)
{
	int i;
	
	for(i=0; i < MAX_SPDIF_PAGE; i++)
	{
		pspdif_config->pMMAPTxBufPtr[i] = pspdif_config->pMMAPBufPtr[i];
		memset(pspdif_config->pMMAPTxBufPtr[i], 0, pspdif_config->buf_size);
	}
	return 0;
}

int spdif_mmap_txbuf_free(spdif_config_type* pspdif_config)
{
	int i;

	for(i=0; i<MAX_SPDIF_PAGE; i++)
	{
		if(pspdif_config->pMMAPTxBufPtr[i] != NULL)
		{
			pspdif_config->pMMAPTxBufPtr[i] = NULL;
			kfree(pspdif_config->pMMAPTxBufPtr[i]);
		}
	}
	return 0;
}
#endif
#endif

int spdif_txPagebuf_free(spdif_config_type* pspdif_config)
{
	if (pspdif_config->pPageTxBuf8ptr[0])
        {
		pci_free_consistent(NULL, pspdif_config->buf_size*(MAX_SPDIF_PAGE+1), pspdif_config->pPageTxBuf8ptr[0], pspdif_config->txdma_addr_buf[0]);
		pspdif_config->pPageTxBuf8ptr[0] = NULL;
	}		
	return 0;
}

int spdif_txPagebuf_alloc(spdif_config_type* pspdif_config)
{
	int i;

	/* allocate IEC tx buffer -- IEC_BS_SBLK & IEC_BS_EBLK */
	pspdif_config->pPageTxBuf8ptr[0] = (u8*)pci_alloc_consistent(NULL, (pspdif_config->buf_size)*(MAX_SPDIF_PAGE+1) , &pspdif_config->txdma_addr_buf[0]);

	if(pspdif_config->pPageTxBuf8ptr[0] == NULL) 
	{
        	MSG("Allocate IEC Tx buffer FAILED!!\n");
                return -1;
        }

	for(i=0;i<MAX_SPDIF_PAGE;i++) {
		pspdif_config->pPageTxBuf8ptr[i+1] = pspdif_config->pPageTxBuf8ptr[i] + pspdif_config->buf_size;
		pspdif_config->txdma_addr_buf[i+1] = pspdif_config->txdma_addr_buf[i] + pspdif_config->buf_size;	
	}

	for(i=0;i<=MAX_SPDIF_PAGE;i++)
		printk("addr%d=0x%08x(0x%08x)\n", i, pspdif_config->txdma_addr_buf[i], pspdif_config->pPageTxBuf8ptr[i]);

	/* Init all buffers as 0 */
	for(i=0;i<=MAX_SPDIF_PAGE;i++) {
		memset(pspdif_config->pPageTxBuf8ptr[i], 0, pspdif_config->buf_size);
	}
	
	pspdif_config->page0_start = 1;	
	pspdif_config->page1_start = 1;
	
	pspdif_config->nsadr_buf0 =  pspdif_config->txdma_addr_buf[0]; // The first data buffer is 0 
	pspdif_config->nsadr_buf1 =  pspdif_config->txdma_addr_buf[1];

	return 0;
}

void spdif_tx_clock_config(void)
{
	unsigned long data;

	/* Enable SPDIF TX clock */
	data = spdif_inw(RALINK_SYSCTL_BASE+0x30);
	data |= 0x00000080;
	spdif_outw(RALINK_SYSCTL_BASE+0x30, data);
	printk("0xBE000F30=0x%08x\n", spdif_inw(RALINK_SYSCTL_BASE+0x0030));
}

int spdif_clock_divider_set(spdif_config_type* pspdif_config, unsigned long index)
{
	unsigned long data;
	unsigned long mas_value;
	unsigned long iec_value;
	unsigned long bit_value;
	unsigned long* pTable;

	MSG("Set Audio clock divider\n");
	pTable = mas_div;
        mas_value = (unsigned long)(pTable[index]);
        pTable = iec_div;
        iec_value = (unsigned long)(pTable[index]);
	pTable = bit_div;
	bit_value = (unsigned long)(pTable[index]);
	data = spdif_inw(IEC_ACLK_DIV);
	data &= (0x00000000);
	data |= REGBIT(mas_value, MAS_DIV);
	data |= REGBIT(iec_value, IEC_DIV);
	data |= REGBIT(bit_value, BIT_DIV);
	data |= REGBIT(0x1f, LRC_DIV);  /*FIXME*/
	spdif_outw(IEC_ACLK_DIV, data);
	printk("IEC_ACLK_DIV=0x%08x\n", data);

#if 0
	/* For FPGA mode, it's APLL select */
	if(index <= 3)
	{
		printk("%d:index=%ld\n", __LINE__, index);
		data = spdif_inw(IEC_APLL_CFG0);
		data &= 0x0;
		spdif_outw(IEC_APLL_CFG0, data);
		//printk("APLL_CFG0=0x%08x\n", data);
	}
	else if (index>3 && index <8)
	{
		printk("%d:index=%ld\n", __LINE__, index);
		data = spdif_inw(IEC_APLL_CFG0);
		data &= 0x0;
		data |= 0x00000001;
		spdif_outw(IEC_APLL_CFG0, data);
		//printk("APLL_CFG0=0x%08x\n", data);
	}
	else
	{
		MSG("Wrong sampling rate\n");
		return -1;
	}
	printk("APLL_CFG0=0x%08x\n", spdif_inw(IEC_APLL_CFG0));
#endif

	return 0;
}


int spdif_ch_status_init(spdif_config_type* pspdif_config)
{
	/* Set channel config, except sam_freq, digital_ch_stat, and word_len */
	pspdif_config->clk_accuracy = LEVEL_II;
	pspdif_config->ch1_num = 0; 
	pspdif_config->ch2_num = 0;
	pspdif_config->src_num = 0;
	pspdif_config->category_code = 0; 
	pspdif_config->ch_status_mode = 0;
	pspdif_config->add_info = 0;
	pspdif_config->cp_right = 0;
	pspdif_config->consumer_use = 0; 
	pspdif_config->cgms_a = 0; 
	pspdif_config->original_fs = ORIG_SFREQ_NOT_INDICATED;

	return 0;
}

int spdif_nsadr_set(spdif_config_type* pspdif_config, u32 dma_ch, u32 r_idx)
{
	if( r_idx > MAX_SPDIF_PAGE) {
		printk("Wrong buffer index!!\n");
		return -1;
	}

	if (dma_ch == 0) {
#if defined(CONFIG_SPDIF_MMAP)
		dma_sync_single_for_device(NULL, spdif_mmap_addr[r_idx], pspdif_config->buf_size, DMA_TO_DEVICE);
		pspdif_config->nsadr_buf0 = spdif_mmap_addr[r_idx]; 
#else
		pspdif_config->nsadr_buf0 = pspdif_config->txdma_addr_buf[r_idx];
#endif
	}
	else {
#if defined(CONFIG_SPDIF_MMAP)
		dma_sync_single_for_device(NULL, spdif_mmap_addr[r_idx], pspdif_config->buf_size, DMA_TO_DEVICE);
		pspdif_config->nsadr_buf1 = spdif_mmap_addr[r_idx];
#else
		pspdif_config->nsadr_buf1 = pspdif_config->txdma_addr_buf[r_idx];
#endif
	}

	return 0;
}

int spdif_ch_status_config(spdif_config_type* pspdif_config, u32 dma_ch)
{
	unsigned long data;

	if (dma_ch == SPDIF_DMA_BUF0)
	{
		data = spdif_inw(IEC_BUF0_CH_CFG0);
		data &= 0x0;
		data |= REGBIT(pspdif_config->clk_accuracy, CLK_ACCURACY);
		data |= REGBIT(pspdif_config->sam_freq, SAM_FREQ);
		data |= REGBIT(pspdif_config->ch1_num, CH1_NUM);
		data |= REGBIT(pspdif_config->src_num, SRC_NUM);
		data |= REGBIT(pspdif_config->category_code, CATEGORY_CODE);
		data |= REGBIT(pspdif_config->ch_status_mode, CH_MODE);
		data |= REGBIT(pspdif_config->add_info, ADD_INFO);
		data |= REGBIT(pspdif_config->cp_right, CP_RIGHT);
		data |= REGBIT(pspdif_config->digital_ch_stat, CH_DIGITAL);
		data |= REGBIT(pspdif_config->consumer_use, CH_CONSUMER);
		spdif_outw(IEC_BUF0_CH_CFG0, data);

		data = spdif_inw(IEC_BUF0_CH_CFG1);
		data &= 0x0;
		data |= REGBIT(pspdif_config->cgms_a, CGMS_A);
		data |= REGBIT(pspdif_config->original_fs, ORIGINAL_FS);
		data |= REGBIT(pspdif_config->word_len, WORD_LEN);
		spdif_outw(IEC_BUF0_CH_CFG1, data);

		data = spdif_inw(IEC_BUF0_CH_CFG_TRIG);
		data &= 0xff0fffff;
		data |= REGBIT(pspdif_config->ch2_num, CH2_NUM);
		spdif_outw(IEC_BUF0_CH_CFG_TRIG, data);
	}
	else
	{
		data = spdif_inw(IEC_BUF1_CH_CFG0);
		data &= 0x0;
		data |= REGBIT(pspdif_config->clk_accuracy, CLK_ACCURACY);
		data |= REGBIT(pspdif_config->sam_freq, SAM_FREQ);
		data |= REGBIT(pspdif_config->ch1_num, CH1_NUM);
		data |= REGBIT(pspdif_config->src_num, SRC_NUM);
		data |= REGBIT(pspdif_config->category_code, CATEGORY_CODE);
		data |= REGBIT(pspdif_config->ch_status_mode, CH_MODE);
		data |= REGBIT(pspdif_config->add_info, ADD_INFO);
		data |= REGBIT(pspdif_config->cp_right, CP_RIGHT);
		data |= REGBIT(pspdif_config->digital_ch_stat, CH_DIGITAL);
		data |= REGBIT(pspdif_config->consumer_use, CH_CONSUMER);
		spdif_outw(IEC_BUF1_CH_CFG0, data);

		data = spdif_inw(IEC_BUF1_CH_CFG1);
		data &= 0x0;
		data |= REGBIT(pspdif_config->cgms_a, CGMS_A);
		data |= REGBIT(pspdif_config->original_fs, ORIGINAL_FS);
		data |= REGBIT(pspdif_config->word_len, WORD_LEN);
		spdif_outw(IEC_BUF1_CH_CFG1, data);

		data = spdif_inw(IEC_BUF1_CH_CFG_TRIG);
		data &= 0xff0fffff;
		data |= REGBIT(pspdif_config->ch2_num, CH2_NUM);
		spdif_outw(IEC_BUF1_CH_CFG_TRIG, data);
	}	

	return 0;
}

int spdif_ch_cfg_trig(spdif_config_type* pspdif_config, u32 dma_ch)
{
	unsigned long data;

	if(dma_ch == SPDIF_DMA_BUF0)
	{
		data = spdif_inw(IEC_BUF0_CH_CFG_TRIG);
		data &= 0x7fffffff;
		data |= REGBIT(0x1, CH_CFG_TRIG);
		spdif_outw(IEC_BUF0_CH_CFG_TRIG, data);
	}
	else
	{
		data = spdif_inw(IEC_BUF1_CH_CFG_TRIG);
		data &= 0x7fffffff;
		data |= REGBIT(0x1, CH_CFG_TRIG);
		spdif_outw(IEC_BUF1_CH_CFG_TRIG, data);
	}

	return 0;
}


int spdif_intr_status_config(spdif_config_type* pspdif_config)
{
	unsigned long data;

	data = spdif_inw(IEC_CTRL);
	//data &= 0xffff7fff;
	data |= REGBIT(0x1, IEC_INTR_STATUS);
	spdif_outw(IEC_CTRL, data);

	return 0;
}


int spdif_next_uadr_config(spdif_config_type* pspdif_config, u32 dma_ch)
{
	unsigned long data;

	/* Set IEC_NEXT_UADR */
	if(dma_ch == SPDIF_DMA_BUF0)
	{
		data = spdif_inw(IEC_BUF0_NEXT_UADR);
		data &= 0xe0000000;
		data |= pspdif_config->nusadr_buf0;
		spdif_outw(IEC_BUF0_NEXT_UADR, data);
	}
	else
	{
		data = spdif_inw(IEC_BUF1_NEXT_UADR);
		data &= 0xe0000000;
		data |= pspdif_config->nusadr_buf1;
		spdif_outw(IEC_BUF1_NEXT_UADR, data);
	}

	return 0;
}



int spdif_nsadr_config(spdif_config_type* pspdif_config, u32 dma_ch)
{
	unsigned long data;

	/* Set IEC_NSADR */
	if(dma_ch == SPDIF_DMA_BUF0)
	{
		data = spdif_inw(IEC_BUF0_NSADR);
		data &= 0xe0000000;
		data |= pspdif_config->nsadr_buf0;
		spdif_outw(IEC_BUF0_NSADR, data);
	}
	else
	{
		data = spdif_inw(IEC_BUF1_NSADR);
		data &= 0xe0000000;
		data |= pspdif_config->nsadr_buf1;
		spdif_outw(IEC_BUF1_NSADR, data);
	}

	return 0;
}

int spdif_pcm_bufsize_init(spdif_config_type* pspdif_config)
{
	pspdif_config->burst_sample = PCM_BURST_SAMPLE;
	pspdif_config->buf_size = pspdif_config->burst_sample * (pspdif_config->byte_num) * 2;
	pspdif_config->copy_size = pspdif_config->buf_size >> 3;
	printk("PCM buffer size: %d; copy size: %d\n", pspdif_config->buf_size, pspdif_config->copy_size);

	return 0;
}

int spdif_raw_bufsize_init(spdif_config_type* pspdif_config)
{
	pspdif_config->buf_size = pspdif_config->nb_len >> 3;
	printk("**********Raw data buffer size: %d byte**********\n", pspdif_config->buf_size);

	return 0;
}

int spdif_pcm_intr_nsnum_init(spdif_config_type* pspdif_config)
{
	pspdif_config->intr_size = 1;
	pspdif_config->nsnum = pspdif_config->burst_sample;

	return 0;
}

int spdif_raw_intr_nsnum_init(spdif_config_type* pspdif_config)
{
	pspdif_config->intr_size = 3; 
	pspdif_config->nsnum = pspdif_config->burst_sample; 
	
	return 0;
}

int spdif_intr_nsnum_config(spdif_config_type* pspdif_config, u32 dma_ch)
{
	unsigned long data; 

	if(dma_ch == SPDIF_DMA_BUF0)
	{
		data = spdif_inw(IEC_BUF0_INTR_NSNUM);
		data &= 0xe000e000;
        	data |= REGBIT(pspdif_config->intr_size, INTR_SIZE);
		data |= REGBIT(pspdif_config->nsnum, BURST_NSNUM); 
		spdif_outw(IEC_BUF0_INTR_NSNUM, data);	
	}
	else
	{
		data = spdif_inw(IEC_BUF1_INTR_NSNUM);
		data &= 0xe000e000;
        	data |= REGBIT(pspdif_config->intr_size, INTR_SIZE);
		data |= REGBIT(pspdif_config->nsnum, BURST_NSNUM); 
		spdif_outw(IEC_BUF1_INTR_NSNUM, data);		
	}

	return 0;
}	

int spdif_pcpd_pack_init(spdif_config_type* pspdif_config)
{
	/* Init PCPD PACK for buf0 */
	pspdif_config->err_flag_buf0 = VALID_BURST_PAYLOAD ;
	pspdif_config->dep_info_buf0 = GENERAL_DATA;
	pspdif_config->bstream_num_buf0 = 0;
	pspdif_config->nb_len_buf0 = pspdif_config->nb_len;  	

	/* Init PCPD PACK for buf1 */
	pspdif_config->err_flag_buf1 = VALID_BURST_PAYLOAD ;
	pspdif_config->dep_info_buf1 = GENERAL_DATA;
	pspdif_config->bstream_num_buf1 = 0;
	pspdif_config->nb_len_buf1 = pspdif_config->nb_len;  

	return 0;
}

int spdif_pcpd_pack_config(spdif_config_type* pspdif_config, u32 dma_ch)
{
	unsigned long data;

	if(dma_ch == SPDIF_DMA_BUF0)
	{
		data = spdif_inw(IEC_BUF0_PCPD_PACK);
		data &= 0xffffffff;
		data |= REGBIT(pspdif_config->bstream_num_buf0, BITSTREAM_NUM);
		data |= REGBIT(pspdif_config->dep_info_buf0, DEP_INFO);
		data |= REGBIT(pspdif_config->err_flag_buf0, ERR_FLAG);
		data |= REGBIT(pspdif_config->subdata_type, SUBDATA_TYPE);
		data |= REGBIT(pspdif_config->data_type, DATA_TYPE);
		data |= REGBIT(pspdif_config->nb_len_buf0, NB_LEN);
		spdif_outw(IEC_BUF0_PCPD_PACK, data);
	}
	else
	{
		data = spdif_inw(IEC_BUF1_PCPD_PACK);
		data &= 0xffffffff;
		data |= REGBIT(pspdif_config->bstream_num_buf1, BITSTREAM_NUM);
		data |= REGBIT(pspdif_config->dep_info_buf1, DEP_INFO);
		data |= REGBIT(pspdif_config->err_flag_buf1, ERR_FLAG);
		data |= REGBIT(pspdif_config->subdata_type, SUBDATA_TYPE);
		data |= REGBIT(pspdif_config->data_type, DATA_TYPE);
		data |= REGBIT(pspdif_config->nb_len_buf1, NB_LEN);
		spdif_outw(IEC_BUF1_PCPD_PACK, data);
	}
	return 0;
}


/* FIXME: Not finish yet!! */
int spdif_reset_tx_config(spdif_config_type* pspdif_config)
{
	pspdif_config->buf_undflow = 0;
	pspdif_config->dma_buf = SPDIF_DMA_BUF0;
	pspdif_config->pause_en = 0;
	
	pspdif_config->tx_r_idx = 0;
	pspdif_config->tx_w_idx = 0;

	pspdif_config->bTxDMAEnable=0;
pspdif_config->interrupt_cnt=0;

	return 0;
}


int spdif_tx_hw_config(spdif_config_type* pspdif_config)
{
        unsigned long data;
	u32 dma_ch0, dma_ch1;

	dma_ch0 = SPDIF_DMA_BUF0;
	dma_ch1 = SPDIF_DMA_BUF1;
	
	// Config IEC_CTRL
	data = spdif_inw(IEC_CTRL);
	data &= 0xffffff0c;
	data |= REGBIT(pspdif_config->mute_spdif, MUTE_SPDF);
	data |= REGBIT(pspdif_config->byte_swap, BYTE_SWAP);
	data |= REGBIT(pspdif_config->raw_swap, RAW_SWAP);
	data |= REGBIT(pspdif_config->raw_24, RAW_24);
	data |= REGBIT(pspdif_config->data_fmt, DATA_FMT);
	data |= REGBIT(pspdif_config->data_src, DATA_SRC);
	spdif_outw(IEC_CTRL, data);
	
	/* Config IEC_BUF0_BS_SBLK */
	data = spdif_inw(IEC_BUF0_BS_SBLK);
	data &= 0xf8000000;
#if defined(CONFIG_SPDIF_MMAP)
	data |= (spdif_mmap_addr[0] >> 2); // unit is double word!!
#else
	data |= (pspdif_config->txdma_addr_buf[0] >> 2); // unit is double word!!
#endif
	spdif_outw(IEC_BUF0_BS_SBLK, data);
		
	/* Config IEC_BUF0_BS_EBLK */
	data = spdif_inw(IEC_BUF0_BS_EBLK);
	data &= 0xf8000000;
#if defined(CONFIG_SPDIF_MMAP)
	data |= (spdif_mmap_addr[MAX_SPDIF_PAGE+1] >> 2); // unit is double word!!
#else
	data |= ((pspdif_config->txdma_addr_buf[0] + pspdif_config->buf_size*(MAX_SPDIF_PAGE+1)) >> 2);
#endif
	spdif_outw(IEC_BUF0_BS_EBLK, data);
	
	/* Config IEC_BUF1_BS_SBLK */
	data = spdif_inw(IEC_BUF1_BS_SBLK);
	data &= 0xf8000000;
#if defined(CONFIG_SPDIF_MMAP)
	data |= (spdif_mmap_addr[0] >> 2); // unit is double word!!
#else
	data |= (pspdif_config->txdma_addr_buf[0] >> 2); // unit is double word!!
#endif
	spdif_outw(IEC_BUF1_BS_SBLK, data);
		
	/* Config IEC_BUF1_BS_EBLK */
	data = spdif_inw(IEC_BUF1_BS_EBLK);
	data &= 0xf8000000;
#if defined(CONFIG_SPDIF_MMAP)
	data |= (spdif_mmap_addr[MAX_SPDIF_PAGE+1] >> 2); // unit is double word!!
#else
	data |= ((pspdif_config->txdma_addr_buf[0] + pspdif_config->buf_size*(MAX_SPDIF_PAGE+1)) >> 2);
#endif
	spdif_outw(IEC_BUF1_BS_EBLK, data);
	
	/* Config IEC_BUF0_NSADR & IEC_BUF1_NSADR */
	spdif_nsadr_config(pspdif_config, dma_ch0);
	spdif_nsadr_config(pspdif_config, dma_ch1);

	/* Config IEC_BUF0_INTR_NSNUM & IEC_BUF1_INTR_NSNUM */
	spdif_intr_nsnum_config(pspdif_config, dma_ch0);
	spdif_intr_nsnum_config(pspdif_config, dma_ch1);

	/* Config IEC_BUF0_CH_CFG0 & IEC_BUF0_CH_CFG1 */
	spdif_ch_status_config(pspdif_config, dma_ch0);

	/* Config IEC_BUF1_CH_CFG0 & IEC_BUF1_CH_CFG1 */
	spdif_ch_status_config(pspdif_config, dma_ch1);

	/* Config IEC_BUF0_CH_CFG_TRIG & IEC_BUF1_CH_CFG_TRIG */
	spdif_ch_cfg_trig(pspdif_config, dma_ch0);
	spdif_ch_cfg_trig(pspdif_config, dma_ch1);

	/* Config IEC_BUF0_NEXT_UADR & IEC_BUF1_NEXT_UADR */
	if (pspdif_config->udata_en == 1)
	{
		spdif_next_uadr_config(pspdif_config, dma_ch0);
		spdif_next_uadr_config(pspdif_config, dma_ch1);
	}

	/* Config IEC_BUF0_PCPD_PACK & IEC_BUF1_PCPD_PACK */
	if (pspdif_config->data_fmt == 1) 
	{
		/* Config IEC_BUF0_PCPD_PACK */
		spdif_pcpd_pack_config(pspdif_config, dma_ch0);
		spdif_pcpd_pack_config(pspdif_config, dma_ch1);
	}

        return 0;
}

/* Turn On Tx DMA and INT */
int spdif_tx_enable(spdif_config_type* pspdif_config)
{
        unsigned long data;

	data = spdif_inw(IEC_CTRL);
	spdif_outw(IEC_CTRL, data);
	/* Eable SPDIF HW & Interrupt */
	data = spdif_inw(IEC_CTRL);
	data |= REGBIT(0x1, RAW_EN);
	data |= REGBIT(0x1, RAW_EN_OPT);
	data |= REGBIT(0x1, INTR_EN);
	data |= REGBIT(0x0, MUTE_SPDF);
	spdif_outw(IEC_CTRL, data);

        printk("spdif_tx_enable done\n");
        return SPDIF_OK;
}


/* FIXME: Not finish yet!! */
/* Turn Off Tx DMA and INT */
int spdif_tx_disable(spdif_config_type* pspdif_config)
{
        unsigned long data;

	data = spdif_inw(IEC_CTRL);
	//data |= REGBIT(0x1, MUTE_SAMPLE);
	//data |= REGBIT(0x1, MUTE_SPDF);
	//data ~= REGBIT(0x1, INTR_EN);
	data &= ~REGBIT(0x1, RAW_EN);
	spdif_outw(IEC_CTRL, data);

	// Reset SPDIF block: first reset(1) this bit, then deasset the reset (0) 
	spdif_reset();

        return SPDIF_OK;
}

int spdif_buffer_underflow_occur(spdif_config_type* pspdif_config)
{
	/* For raw data, take buffer under flow  as "PAUSE_DATA" state, fill the buffer with 0 */
	pspdif_config->data_type_tmp = pspdif_config->data_type;
	pspdif_config->subdata_type_tmp = pspdif_config->subdata_type;

	pspdif_config->data_type = PAUSE_DATA;
	pspdif_config->subdata_type = 0;
	pspdif_config->buf_size = PAUSE_BURST_SAMPLE;

	return 0;
}

int spdif_buffer_underflow_recover(spdif_config_type* pspdif_config)
{
	pspdif_config->data_type = pspdif_config->data_type_tmp;
	pspdif_config->subdata_type = pspdif_config->subdata_type_tmp;

	return 0;
}

void spdif_mute_config(void)
{
	unsigned long data;

	data = spdif_inw(IEC_CTRL);
	data |= REGBIT(0x1, MUTE_SAMPLE);
	spdif_outw(IEC_CTRL, data);
}

void spdif_demute_config(void)
{
	unsigned long data;
	
	data = spdif_inw(IEC_CTRL);
	data &= ~REGBIT(0x1, MUTE_SAMPLE);
	spdif_outw(IEC_CTRL, data);
}

#if defined(SPDIF_AC3_DEBUG)
void spdif_ac3_header_check(spdif_config_type* pspdif_config)
{
	ac3_si_t* pac3six;
	pac3six = pspdif_config->pPageTxBuf8ptr[pspdif_config->tx_r_idx];
	printk("**** %d: syncword=0x%4x, crc1=0x%4x, frame_num=0x%4x\n", pspdif_config->tx_r_idx, htons(pac3six->syncword), htons(pac3six->crc1), htons(pac3six->crc7));
}
#endif

void spdif_nb_len_config(spdif_config_type* pspdif_config, u32 dma_ch)
{
	if (dma_ch == SPDIF_DMA_BUF0)
	{
		pspdif_config->nb_len_buf0 = pspdif_config->nb_len; 
		spdif_pcpd_pack_config(pspdif_config, dma_ch);
	}
	else
	{
		pspdif_config->nb_len_buf1 = pspdif_config->nb_len; 
		spdif_pcpd_pack_config(pspdif_config, dma_ch);
	}
}

int spdif_rdone_bit_handle(spdif_config_type* pspdif_config, u32 dma_ch)
{
	if(dma_ch == SPDIF_DMA_BUF0)
	{
		//if((pspdif_config->page0_start == 1) && (pspdif_config->rdone_bit[0] == 0))
		if((pspdif_config->page0_start == 1))
		{
			pspdif_config->page0_start = 0;
			//pspdif_config->rdone_bit[0] = 1;
		}
		
		pspdif_config->rdone_bit[pspdif_config->tx_r_idx] = 1;
	}
	else
	{
		//if((pspdif_config->page1_start == 1) && (pspdif_config->rdone_bit[1] == 0))
		if((pspdif_config->page1_start == 1))
		{
			pspdif_config->page1_start = 0;
			//pspdif_config->rdone_bit[1] = 1;
		}

		pspdif_config->rdone_bit[pspdif_config->tx_r_idx] = 1;
	}

	return 0;
}

int spdif_rdone_bit_init(spdif_config_type* pspdif_config)
{
	int i;

	for (i=0; i<MAX_SPDIF_PAGE; i++)
	{
		if (i <= pspdif_config->tx_w_idx)
			pspdif_config->rdone_bit[i] = 0;
		else
			pspdif_config->rdone_bit[i] = 1;
	}
	return 0;
}

int spdif_tx_transf_zero_set(spdif_config_type* pspdif_config, u32 dma_ch)
{
	spdif_nsadr_set(pspdif_config, dma_ch, MAX_SPDIF_PAGE);
	spdif_nsadr_config(pspdif_config, dma_ch);

	if (pspdif_config->data_fmt == 1) // raw data
	{
		spdif_nb_len_config(pspdif_config, dma_ch);
	}

	return 0;
}

int spdif_tx_transf_data_set(spdif_config_type* pspdif_config, u32 dma_ch)
{
	spdif_nsadr_set(pspdif_config, dma_ch, pspdif_config->tx_r_idx);		
	spdif_nsadr_config(pspdif_config, dma_ch);
	if (pspdif_config->data_fmt == 1)
	{
		spdif_nb_len_config(pspdif_config, dma_ch);
	}

	return 0;
}

int spdif_tx_transf_zero_handle(spdif_config_type* pspdif_config)
{
	u32 dma_ch;

	if(pspdif_config->dma_buf == SPDIF_DMA_BUF0) 
		dma_ch = SPDIF_DMA_BUF0;
	else
		dma_ch = SPDIF_DMA_BUF1;

	spdif_tx_transf_zero_set(pspdif_config, dma_ch);
	spdif_ch_status_config(pspdif_config, dma_ch);
	spdif_ch_cfg_trig(pspdif_config, dma_ch);
	spdif_intr_status_config(pspdif_config); // Write one clear
		
	/* Set next dma buffer number */
	if(pspdif_config->dma_buf == SPDIF_DMA_BUF0) 
		pspdif_config->dma_buf = SPDIF_DMA_BUF1; // next buffer
	else
		pspdif_config->dma_buf = SPDIF_DMA_BUF0;
	
	return 0;
}

int spdif_tx_transf_data_handle(spdif_config_type* pspdif_config)
{
	u32 dma_ch;

	if(pspdif_config->dma_buf == SPDIF_DMA_BUF0)
		dma_ch = SPDIF_DMA_BUF0;
	else
		dma_ch = SPDIF_DMA_BUF1;

#if defined(SPDIF_AC3_DEBUG)
	if (pspdif_config->data_fmt == 1 && pspdif_config->data_type == AC3_DATA) {
		spdif_ac3_header_check(pspdif_config);
	}
#endif 

	spdif_tx_transf_data_set(pspdif_config, dma_ch);
	spdif_rdone_bit_handle(pspdif_config, dma_ch);
	spdif_ch_status_config(pspdif_config, dma_ch);
	spdif_ch_cfg_trig(pspdif_config, dma_ch);  // Update channel status
	spdif_intr_status_config(pspdif_config); // Write one clear

	/* Update next dma buffer number */
	if(pspdif_config->dma_buf == SPDIF_DMA_BUF0)
		pspdif_config->dma_buf = SPDIF_DMA_BUF1;
	else
		pspdif_config->dma_buf = SPDIF_DMA_BUF0;

	pspdif_config->tx_r_idx = (pspdif_config->tx_r_idx + 1) % MAX_SPDIF_PAGE;

	return 0;
}

int spdif_pcm_put_audio(spdif_config_type* pspdif_config, unsigned long arg)
{
	do{
		spin_lock(&pspdif_config->lock);
		if ((((pspdif_config->tx_w_idx + 3) % MAX_SPDIF_PAGE) != pspdif_config-> tx_r_idx) 
			&& (pspdif_config->rdone_bit[(pspdif_config->tx_w_idx + 1) % MAX_SPDIF_PAGE] ==1)
			&& (pspdif_config->rdone_bit[(pspdif_config->tx_w_idx + 2) % MAX_SPDIF_PAGE] ==1)
			&& (pspdif_config->rdone_bit[(pspdif_config->tx_w_idx + 3) % MAX_SPDIF_PAGE] ==1)
			)
		{
                	pspdif_config->tx_w_idx = (pspdif_config->tx_w_idx+1)%MAX_SPDIF_PAGE;
                        //printk("put TB[%d] for user write\n",pspdif_config->tx_w_idx);
				
			pspdif_config->rdone_bit[pspdif_config->tx_w_idx] = 0;
#if defined(CONFIG_SPDIF_MMAP)
			put_user(pspdif_config->tx_w_idx, (int*)arg);
#else
			//copy_from_user(pspdif_config->pPageTxBuf8ptr[pspdif_config->tx_w_idx], (char*)arg, pspdif_config->buf_size);
			/* Two move */
			copy_from_user(pspdif_config->pPageTxBuf8ptr[pspdif_config->tx_w_idx], (char*)arg, (pspdif_config->buf_size >> 1));
			copy_from_user((pspdif_config->pPageTxBuf8ptr[pspdif_config->tx_w_idx]+(pspdif_config->buf_size >> 1)), (char*)(arg+ (pspdif_config->buf_size >> 1)), (pspdif_config->buf_size >> 1));

			dma_cache_sync(NULL, pspdif_config->pPageTxBuf8ptr[pspdif_config->tx_w_idx], pspdif_config->buf_size, DMA_TO_DEVICE);
#endif                           
			pspdif_status->txbuffer_len++;
                        spin_unlock(&pspdif_config->lock);
                        break;
		}
                else
                {
                	/* Buffer Full */
                        //printk("TBF tr=%d, tw=%d\n", pspdif_config->tx_r_idx, pspdif_config->tx_w_idx);
                        pspdif_status->txbuffer_ovrun++;
                        spin_unlock(&pspdif_config->lock);

                        interruptible_sleep_on(&(pspdif_config->spdif_tx_qh));

                }
	}while(1);

	return 0;
}

int spdif_raw_put_audio(spdif_config_type* pspdif_config, unsigned long arg)
{
	do{
        	spin_lock(&pspdif_config->lock);
			
		if ((((pspdif_config->tx_w_idx + 4) % MAX_SPDIF_PAGE) != pspdif_config-> tx_r_idx) 
			&& (pspdif_config->rdone_bit[(pspdif_config->tx_w_idx + 1) % MAX_SPDIF_PAGE] ==1)
			&& (pspdif_config->rdone_bit[(pspdif_config->tx_w_idx + 2) % MAX_SPDIF_PAGE] ==1)
			&& (pspdif_config->rdone_bit[(pspdif_config->tx_w_idx + 3) % MAX_SPDIF_PAGE] ==1)
			)
		{
                	pspdif_config->tx_w_idx = (pspdif_config->tx_w_idx+1)%MAX_SPDIF_PAGE;
                        //printk("put TB[%d] for user write\n",pspdif_config->tx_w_idx);

#if defined(SPDIF_AC3_DEBUG)
			spdif_ac3_header_check(pspdif_config);
#endif
			pspdif_config->rdone_bit[pspdif_config->tx_w_idx] = 0;
#if defined(CONFIG_SPDIF_MMAP)
			put_user(pspdif_config->tx_w_idx, (int*)arg);
#else
			copy_from_user(pspdif_config->pPageTxBuf8ptr[pspdif_config->tx_w_idx], (char*)arg, pspdif_config->buf_size);
#endif                          
			pspdif_status->txbuffer_len++;
                        spin_unlock(&pspdif_config->lock);
                        break;
		}
                else
                {
                        /* Buffer Full */
                        //printk("TBF tr=%d, tw=%d\n", pspdif_config->tx_r_idx, pspdif_config->tx_w_idx);
                        pspdif_status->txbuffer_ovrun++;
                        spin_unlock(&pspdif_config->lock);

                        interruptible_sleep_on(&(pspdif_config->spdif_tx_qh));
                }
	}while(1);

	return 0;
}


irqreturn_t spdif_dma_tx_handler(int irq, void *irqaction)
{
	unsigned long data;
	bool interrupt_status;

	data = spdif_inw(IEC_CTRL);
	//printk("%s:iec_ctrl=0x%08x\n", __func__, data);

	interrupt_status = data & (0x8000); 
	
	if (interrupt_status == 1)
	{
		if(pspdif_config->pause_en == 1)
		{
			spdif_tx_transf_zero_handle(pspdif_config);	
			goto EXIT;
		}

		/* Buffer under flow case */
		if (pspdif_config->tx_r_idx == pspdif_config->tx_w_idx)
		{
			if (pspdif_config->buf_undflow == 0)
			{
				/* For raw data, take buffer under flow  as "PAUSE_DATA" state, 
				 * fill the buffer with 0 */
				if ((pspdif_config->data_fmt == 1) && (pspdif_config->data_type != PAUSE_DATA)) {
					spdif_buffer_underflow_occur(pspdif_config);
				}
#if 0
				/* For RAW data, take buffer under flow as "MUTE" */
				if (pspdif_config->data_fmt == 1){
					spdif_mute_config();
				}
#endif
				/* For PCM data, take buffer under flow as "MUTE" */
				if (pspdif_config->data_fmt == 0){
					spdif_mute_config();
				}
				pspdif_config->buf_undflow = 1;
			}

			spdif_tx_transf_zero_handle(pspdif_config);
			MSG("Buffer under flow\n");
			
			goto EXIT;
		}
		else
		{
			if (pspdif_config->buf_undflow == 1)
			{
				pspdif_config->buf_undflow = 0;
				if (pspdif_config->data_fmt == 1){
					spdif_buffer_underflow_recover(pspdif_config);	
				}
				else{
					spdif_demute_config();
				}
			}
		}

		if (pspdif_config->rdone_bit[pspdif_config->tx_r_idx] != 0)
		{
			printk("[%d] read done is in wrong state\n", pspdif_config->tx_r_idx);
			goto EXIT;
		}

		/* Normal case */
		spdif_tx_transf_data_handle(pspdif_config);
		pspdif_config->interrupt_cnt++;
		MSG("Normal case\n\n");
	}
	else
	{
		MSG("Interrupt is in wrong status\n");
		return IRQ_HANDLED;
	}
EXIT:
#if defined(CONFIG_SND_ALSA_SPDIF)
	spdif_tx_idx_update(pspdif_config);
#endif
	wake_up_interruptible(&(pspdif_config->spdif_tx_qh));
	return IRQ_HANDLED;
}


#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
long spdif_ioctl (struct file *filp, unsigned int cmd, unsigned long arg)
#else
int spdif_ioctl (struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#endif
{
        spdif_config_type* pspdif_config;

        pspdif_config = filp->private_data;
        switch (cmd) {
        case SPDIF_SAMPLE_FREQ:
                spin_lock(&pspdif_config->lock);
                if((arg>MAX_SFREQ_HZ)||(arg<MIN_SFREQ_HZ))
                {
                        MSG("audio srate %u should be %d ~ %d Hz\n", (u32)arg, MIN_SFREQ_HZ, MAX_SFREQ_HZ);
			spin_unlock(&pspdif_config->lock);
                        break;
                }

		if (arg == 22050)
		{
			pspdif_config->sam_freq = SFREQ_22050HZ;
			pspdif_config->srate = 22050; 
		}
		else if (arg == 24000)
		{
			pspdif_config->sam_freq = SFREQ_24000HZ;
			pspdif_config->srate = 24000;
		}
		else if (arg == 32000)
		{
			pspdif_config->sam_freq = SFREQ_32000HZ;
			pspdif_config->srate = 32000;
		}
		else if (arg == 44100)
		{
			pspdif_config->sam_freq = SFREQ_44100HZ;
			pspdif_config->srate = 44100;
		}
		else if (arg == 48000)
		{
			pspdif_config->sam_freq = SFREQ_48000HZ;
			pspdif_config->srate = 48000;
		}
		else if (arg == 88200)
		{
			pspdif_config->sam_freq = SFREQ_88200HZ;
			pspdif_config->srate = 88200;
		}
		else if (arg == 96000)
		{
			pspdif_config->sam_freq = SFREQ_96000HZ;
			pspdif_config->srate = 96000;
		}
		else if (arg == 176400)
		{
			pspdif_config->sam_freq = SFREQ_176400HZ;
			pspdif_config->srate = 176400;
		}
		else if (arg == 192000)
		{
			pspdif_config->sam_freq = SFREQ_192000HZ;
			pspdif_config->srate = 192000;
		}
		else
			printk("This SoC doesn't support this SR.\n");

                MSG("set audio sampling rate to %d (%d)Hz\n", arg, pspdif_config->sam_freq);
                spin_unlock(&pspdif_config->lock);
                break;

	case SPDIF_WORD_LEN:  // Only for PCM data
		spin_lock(&pspdif_config->lock);

		if (arg == 16)
		{
			pspdif_config->word_len = WLEN_16_20BIT;
			pspdif_config->raw_24 = 0;
			pspdif_config->raw_swap = 0;
			pspdif_config->byte_num= BIT_MODE_16;
		}
		else if (arg == 17)
		{
			pspdif_config->word_len = WLEN_17_20BIT;
		}
		else if (arg == 18)
		{
			pspdif_config->word_len = WLEN_18_20BIT;
		}
		else if (arg == 19)
		{
			pspdif_config->word_len = WLEN_19_20BIT;
		}
		else if (arg == 20)
		{
			pspdif_config->word_len = WLEN_20_24BIT;
		}
		else if (arg == 21)
		{
			pspdif_config->word_len = WLEN_21_24BIT;
		}
		else if (arg == 22)
		{
			pspdif_config->word_len = WLEN_22_24BIT;
		}
		else if (arg == 23)
		{
			pspdif_config->word_len = WLEN_23_24BIT;
		}
		else if (arg == 24) 
		{
			pspdif_config->word_len = WLEN_24_24BIT;
			pspdif_config->raw_24 = 1;
			pspdif_config->raw_swap = 1;
			pspdif_config->byte_num = BIT_MODE_24;
		}
		else
		{
			MSG("Sample word len %u should be %d ~ %d bits\n", (u32)arg, MIN_WORD_LEN, MAX_WORD_LEN);
			spin_unlock(&pspdif_config->lock);
			break;
		}

		MSG("set sample word length  %d (%d)bits\n", arg, pspdif_config->word_len);
		spin_unlock(&pspdif_config->lock);
		break;

	case SPDIF_RAW_DATA_TYPE:
		spin_lock(&pspdif_config->lock);

		if (arg == 0)
		{
			pspdif_config->data_type = NULL_DATA;
			pspdif_config->subdata_type = 0;
			pspdif_config->burst_sample = NULL_BURST_SAMPLE;
		}
		else if (arg == 1)
		{
			pspdif_config->data_type = AC3_DATA;
			pspdif_config->subdata_type = 0;
			pspdif_config->burst_sample = AC3_BURST_SAMPLE;
		}
		else if (arg == 3)
		{
			pspdif_config->data_type = PAUSE_DATA;
			pspdif_config->subdata_type = 0;
			pspdif_config->burst_sample = PAUSE_BURST_SAMPLE;
		}
		else
		{
			MSG("Data type should be %d~%d, and can't be %d\n",  MIN_DATA_TYPE, MAX_DATA_TYPE, RESERVE_DATA_TYPE);
			spin_unlock(&pspdif_config->lock);
			break;
		}

		MSG("set audio data type to %d (%d)\n", pspdif_config->data_type, pspdif_config->burst_sample);
		spin_unlock(&pspdif_config->lock);
		break;

	case SPDIF_NB_LEN:
		spin_lock(&pspdif_config->lock);
		pspdif_config->nb_len = arg;
		MSG("********set nb length to %d*********\n", pspdif_config->nb_len);
		spin_unlock(&pspdif_config->lock);
		break;

        case SPDIF_TX_PCM_ENABLE:
                spin_lock(&pspdif_config->lock);
                MSG("SPDIF_TX_PCM_ENABLE\n");

                //spdif_reset_tx_config(pspdif_config);

		/* Set IEC_CTRL parameters */
		pspdif_config->data_fmt = 0;   

		/* Init channel status config */
		pspdif_config->digital_ch_stat = 0;
		spdif_ch_status_init(pspdif_config);

		/* Init interrupt size and next burst num for IEC_INTR_NSNUM (unit: sample)*/
		spdif_pcm_intr_nsnum_init(pspdif_config);
		pspdif_config->byte_swap = 0;	
			
		/* Enable clk before any hw config */
		spdif_clock_enable(pspdif_config);

		spdif_tx_hw_config(pspdif_config);
                spdif_tx_enable(pspdif_config);

		MSG("SPDIF_TX_PCM_ENABLE done\n");
                spin_unlock(&pspdif_config->lock);
                break;

	case SPDIF_TX_RAW_ENABLE:
		spin_lock(&pspdif_config->lock);
                MSG("SPDIF_TX_RAW_ENABLE\n");
			
		/* Set IEC_CTRL parameters */
		pspdif_config->data_fmt = 1;    
	
		/* Raw data's word length is 16 bit */
		pspdif_config->word_len = WLEN_16_20BIT;
		pspdif_config->raw_24 = 0;
		pspdif_config->raw_swap = 0;
		pspdif_config->byte_num= BIT_MODE_16;
		
		/* Init channel status config */
		pspdif_config->digital_ch_stat = 1; /*FIXME*/
		spdif_ch_status_init(pspdif_config);

		/* Init buffer setting for IEC_BS_SBLK, IEC_BS_EBLK */
		spdif_raw_bufsize_init(pspdif_config);
#if defined(CONFIG_SPDIF_MMAP)
		//spdif_mmap_txbuf_alloc(pspdif_config);
#else
		spdif_txPagebuf_alloc(pspdif_config);  
#endif

		/* Init interrupt size and next burst num for IEC_INTR_NSNUM (unit: sample)*/
		spdif_raw_intr_nsnum_init(pspdif_config);
		spdif_pcpd_pack_init(pspdif_config);
		//pspdif_config->byte_swap = 1; /* FIXME */		

		/* Enable clk before any hw config */	
		spdif_clock_enable(pspdif_config);

		spdif_reset_tx_config(pspdif_config);
		spdif_tx_hw_config(pspdif_config);
		spdif_tx_enable(pspdif_config);

		spdif_rdone_bit_init(pspdif_config);

		MSG("SPDIF_TX_RAW_ENABLE done\n");
                spin_unlock(&pspdif_config->lock);
		break;

        case SPDIF_TX_DISABLE:
                spin_lock(&pspdif_config->lock);
                MSG("SPDIF_TX_DISABLE\n");

		printk("TX disable\n");
                spdif_tx_disable(pspdif_config);
		spdif_clock_disable(pspdif_config);

#if defined(CONFIG_SPDIF_MMAP)
		//spdif_mmap_txbuf_free(pspdif_config);
		if(pspdif_config->mmap_index <= MAX_SPDIF_PAGE)
			pspdif_config->mmap_index = 0;
#else
                spdif_txPagebuf_free(pspdif_config);
#endif
		spin_unlock(&pspdif_config->lock);
                break;
        case SPDIF_PCM_PUT_AUDIO:
                spdif_pcm_put_audio(pspdif_config, arg);
		break;
	case SPDIF_RAW_PUT_AUDIO:
                spdif_raw_put_audio(pspdif_config, arg);
		break;

	case SPDIF_BYTE_SWAP_SET:
	        spin_lock(&pspdif_config->lock);
		pspdif_config->byte_swap = arg;
		MSG("********set byte_swap to %d*********\n", pspdif_config->byte_swap);
		spin_unlock(&pspdif_config->lock);
		break;
	case SPDIF_INIT_PARAMETER:
		spin_lock(&pspdif_config->lock);
		spdif_pcm_bufsize_init(pspdif_config);
#if defined(CONFIG_SPDIF_MMAP)
		//spdif_mmap_txbuf_alloc(pspdif_config);
#else
		spdif_txPagebuf_alloc(pspdif_config);
#endif
		
		spdif_reset_tx_config(pspdif_config);
		spdif_rdone_bit_init(pspdif_config);
		spin_unlock(&pspdif_config->lock);
		break;
	case SPDIF_DOWN_SAMPLE:
		spin_lock(&pspdif_config->lock);
		if (arg == 1)
		{
			pspdif_config->down_sample = SPDIF_NO_DOWN_SAMPLE;
			MSG("set down sample to No down sample\n");
		}
		else if (arg == 2)
		{
			pspdif_config->down_sample = SPDIF_2X_DOWN_SAMPLE;
			MSG("set down sample to 2x down sample\n");
		}
		else if (arg == 4)
		{
			pspdif_config->down_sample = SPDIF_4X_DOWN_SAMPLE;
			MSG("set down sample to 4x down sample\n");
		}
		else
		{
			MSG("Down sample should be 1: no down sample; 2: 2x down sample; 4: 4x down sample\n");
			spin_unlock(&pspdif_config->lock);
			break;
		}

		spin_unlock(&pspdif_config->lock);
		break;
	case SPDIF_TX_STOP:
		spin_lock(&pspdif_config->lock);
                MSG("SPDIF_TX_DISABLE\n");

printk("TX disable\n");
                spdif_tx_disable(pspdif_config);
		spdif_clock_disable(pspdif_config);

#if defined(CONFIG_SPDIF_MMAP)
		//spdif_mmap_txbuf_free(pspdif_config);
		if(pspdif_config->mmap_index <= MAX_SPDIF_PAGE)
			pspdif_config->mmap_index = 0;
#else
		spdif_txPagebuf_free(pspdif_config);
#endif
     		spin_unlock(&pspdif_config->lock);
		break;
	case SPDIF_TX_PAUSE:
		spin_lock(&pspdif_config->lock);
		pspdif_config->pause_en = 1;
		printk("* pause_en = 1 *\n");
		spin_unlock(&pspdif_config->lock);
		break;
	case SPDIF_TX_RESUME:
		spin_lock(&pspdif_config->lock);
		pspdif_config->pause_en = 0;
		printk("# pause_en = 0 #\n");
		spin_unlock(&pspdif_config->lock);
		break;
        default :
                MSG("spdif_ioctl: command format error\n");
        }

        return 0;
}


/***********************
 * SPDIF API for ALSA  *
 ***********************/
#if defined(CONFIG_SND_ALSA_SPDIF)
char* spdif_tx_dma_alloc(spdif_config_type* pspdif_config)
{
	int ret;

	if (!pspdif_config)
		return NULL;

	ret = spdif_txPagebuf_alloc(pspdif_config);
	if (ret < 0)
	{
		printk("Alloc TX buffer failed!\n");
		return NULL;
	}
	return pspdif_config->pPageTxBuf8ptr[0]; 
}

int spdif_tx_dma_free(spdif_config_type* pspdif_config)
{
	if (!pspdif_config)
		return -1;

	spdif_txPagebuf_free(pspdif_config);
	return 0;
}


int spdif_bufsize_init(spdif_config_type* pspdif_config)
{

	spdif_pcm_bufsize_init(pspdif_config);

	return pspdif_config->buf_size;	
}

int spdif_startup(void)
{
	memset(pspdif_config, 0, sizeof(spdif_config_type));

#ifdef SPDIF_STATISTIC
     	memset(pspdif_status, 0, sizeof(spdif_status_type));
#endif

	// Reset SPDIF block: first reset(1) this bit, then deasset the reset (0) 
	spdif_reset();

	/*  set share pins to spdif mode */
	spdif_share_pin_config();

	// Init IEC_CTRL regiater
	pspdif_config->data_src = 1;
	
	init_waitqueue_head(&(pspdif_config->spdif_tx_qh));
        
	return 0;
}

int spdif_shutdown(void)
{
	spdif_config_type *pspdif_config;

	//free_irq(SURFBOARDINT_SPDIF, NULL);
	spdif_irq_release();
printk("Enter %s: free irq\n", __func__);

#if defined(CONFIG_SPDIF_MMAP)
	spdif_mem_unmap(pspdif_config);
#else
	//spdif_txPagebuf_free(pspdif_config);
#endif

	kfree(pspdif_config);
	printk("spdif_release succeeds\n");
	
	return 0;
}

int spdif_tx_idx_update(spdif_config_type* pspdif_config)
{
	if(pspdif_config->substream[SUBSTREAM_PLAYBACK])
		snd_pcm_period_elapsed(pspdif_config->substream[SUBSTREAM_PLAYBACK]);
	return 0;
}

int spdif_tx_dma_ctrl_enable(spdif_config_type* pspdif_config)
{
	pspdif_config->bTxDMAEnable=1;
	return 0;
}

int spdif_tx_dma_ctrl_disable(spdif_config_type* pspdif_config)
{
	pspdif_config->bTxDMAEnable=0;
	return 0;
}

int spdif_irq_init(void)
{
	printk("Enter %s\n", __func__);
	spdif_irq_request();

	return 0;
}

int spdif_irq_release(void)
{
	printk("Enter %s\n", __func__);
	spdif_irq_free();

	return 0;
}
#endif

#if defined(CONFIG_RALINK_MT7621)
EXPORT_SYMBOL(spdif_pll_config);
#endif
EXPORT_SYMBOL(spdif_clock_enable);
EXPORT_SYMBOL(spdif_clock_disable);
EXPORT_SYMBOL(spdif_reset);
EXPORT_SYMBOL(spdif_irq_request);
EXPORT_SYMBOL(spdif_irq_free);
EXPORT_SYMBOL(spdif_share_pin_config);
EXPORT_SYMBOL(spdif_txPagebuf_free);
EXPORT_SYMBOL(spdif_txPagebuf_alloc);
EXPORT_SYMBOL(spdif_tx_clock_config);
EXPORT_SYMBOL(spdif_clock_divider_set);
EXPORT_SYMBOL(spdif_ch_status_init);
EXPORT_SYMBOL(spdif_nsadr_set);
EXPORT_SYMBOL(spdif_ch_status_config);
EXPORT_SYMBOL(spdif_ch_cfg_trig);
EXPORT_SYMBOL(spdif_intr_status_config);
EXPORT_SYMBOL(spdif_next_uadr_config);
EXPORT_SYMBOL(spdif_nsadr_config);
EXPORT_SYMBOL(spdif_pcm_bufsize_init);
EXPORT_SYMBOL(spdif_raw_bufsize_init);
EXPORT_SYMBOL(spdif_pcm_intr_nsnum_init);
EXPORT_SYMBOL(spdif_raw_intr_nsnum_init);
EXPORT_SYMBOL(spdif_intr_nsnum_config);
EXPORT_SYMBOL(spdif_pcpd_pack_init);
EXPORT_SYMBOL(spdif_pcpd_pack_config);
EXPORT_SYMBOL(spdif_reset_tx_config);
EXPORT_SYMBOL(spdif_tx_hw_config);
EXPORT_SYMBOL(spdif_tx_enable);
EXPORT_SYMBOL(spdif_tx_disable);
EXPORT_SYMBOL(spdif_buffer_underflow_occur);
EXPORT_SYMBOL(spdif_buffer_underflow_recover);
EXPORT_SYMBOL(spdif_mute_config);
EXPORT_SYMBOL(spdif_demute_config);
#if defined(SPDIF_AC3_DEBUG)
EXPORT_SYMBOL(spdif_ac3_header_check);
#endif
EXPORT_SYMBOL(spdif_nb_len_config);
EXPORT_SYMBOL(spdif_rdone_bit_handle);
EXPORT_SYMBOL(spdif_rdone_bit_init);
EXPORT_SYMBOL(spdif_tx_transf_zero_set);
EXPORT_SYMBOL(spdif_tx_transf_data_set);
EXPORT_SYMBOL(spdif_tx_transf_zero_handle);
EXPORT_SYMBOL(spdif_tx_transf_data_handle);
EXPORT_SYMBOL(spdif_pcm_put_audio);
EXPORT_SYMBOL(spdif_raw_put_audio);
EXPORT_SYMBOL(spdif_dma_tx_handler);

#if defined(CONFIG_SND_ALSA_SPDIF)
EXPORT_SYMBOL(spdif_tx_dma_alloc);
EXPORT_SYMBOL(spdif_tx_dma_free);
EXPORT_SYMBOL(spdif_bufsize_init);
EXPORT_SYMBOL(spdif_startup);
EXPORT_SYMBOL(spdif_shutdown);
EXPORT_SYMBOL(spdif_tx_idx_update);
EXPORT_SYMBOL(spdif_tx_dma_ctrl_enable);
EXPORT_SYMBOL(spdif_tx_dma_ctrl_disable);
EXPORT_SYMBOL(spdif_irq_init);
EXPORT_SYMBOL(spdif_irq_release);
#endif

module_init(spdif_mod_init);
module_exit(spdif_mod_exit);

MODULE_DESCRIPTION("Ralink SoC SPDIFTX  Module");
MODULE_AUTHOR("Dora Chen <dora.chen@mediatek.com>");
MODULE_SUPPORTED_DEVICE("SPDIF");
MODULE_VERSION(SPDIF_MOD_VERSION);
MODULE_LICENSE("GPL");
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,12)
MODULE_PARM (spdifdrv_major, "i");
#else
module_param (spdifdrv_major, int, 0);
#endif

