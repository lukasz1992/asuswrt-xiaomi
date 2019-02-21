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
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,14)
#include <asm/system.h> /* cli(), *_flags */
#endif
#include <asm/uaccess.h> /* copy_from/to_user */
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <asm/rt2880/surfboardint.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include "../../ralink_gdma.h"
#if defined(CONFIG_I2S_WITH_AEC)
#include "../aec/aec_api.h"
#endif

#ifdef  CONFIG_DEVFS_FS
#include <linux/devfs_fs_kernel.h>
static	devfs_handle_t devfs_handle;
#endif

#include "i2s_ctrl.h"

#if defined(CONFIG_SND_MT76XX_SOC)
#include <sound/soc/mtk/mt76xx_machine.h>
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

static int i2sdrv_major =  191;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
#else
static struct class *i2smodule_class;
#endif

/* external functions declarations */
#if defined(CONFIG_I2S_WM8960)
extern void audiohw_set_frequency(int fsel, int codec_pll_en);
void audiohw_set_apll(int srate);
#elif defined(CONFIG_I2S_WM8750)||defined(CONFIG_I2S_WM8751)
extern void audiohw_set_frequency(int fsel);
#endif
#if defined(CONFIG_I2S_WM8960)||defined(CONFIG_I2S_WM8750)||defined(CONFIG_I2S_WM8751)
extern int audiohw_set_lineout_vol(int Aout, int vol_l, int vol_r);
extern int audiohw_set_master_vol(int vol_l, int vol_r);
extern int audiohw_set_linein_vol(int vol_l, int vol_r);
#endif

extern void audiohw_micboost(int boostgain);

extern int GdmaI2sTx(uint32_t Src, uint32_t Dst, uint8_t TxNo, uint16_t TransCount,
                void (*DoneIntCallback)(uint32_t data),
                void (*UnMaskIntCallback)(uint32_t data));

extern int GdmaI2sRx(uint32_t Src, uint32_t Dst, uint8_t RxNo, uint16_t TransCount,
                void (*DoneIntCallback)(uint32_t data),
                void (*UnMaskIntCallback)(uint32_t data));

extern int GdmaMaskChannel(uint32_t ChNum);

extern int GdmaUnMaskChannel(uint32_t ChNum);

/* internal functions declarations */
irqreturn_t i2s_irq_isr(int irq, void *irqaction);
int i2s_debug_cmd(unsigned int cmd, unsigned long arg);

/* forward declarations for _fops */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
static long i2s_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
#else
static int i2s_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
#endif
static int i2s_mmap(struct file *file, struct vm_area_struct *vma);
static int i2s_open(struct inode *inode, struct file *file);
static int i2s_release(struct inode *inode, struct file *file);
int i2s_mmap_alloc(unsigned long size);
int i2s_mmap_remap(struct vm_area_struct *vma, unsigned long size);

/* global varable definitions */
i2s_config_type i2s_config;
i2s_status_type i2s_status; 
i2s_config_type* pi2s_config = &i2s_config;;
i2s_status_type* pi2s_status = &i2s_status;;

#if defined(ARM_ARCH)
static dma_addr_t i2s_txdma_addr0, i2s_txdma_addr1;
static dma_addr_t i2s_rxdma_addr0, i2s_rxdma_addr1;
#define I2S_TX_FIFO_WREG_PHY (I2S_TX_FIFO_WREG & 0x1FFFFFFF)
#define I2S_RX_FIFO_RREG_PHY (I2S_RX_FIFO_RREG & 0x1FFFFFFF)
#else
static dma_addr_t i2s_txdma_addr, i2s_rxdma_addr;
#endif
static dma_addr_t i2s_mmap_addr[MAX_I2S_PAGE*2];
				      /* 8khz 11.025khz 12khz  16khz 22.05khz 24Khz  32khz 44.1khz 48khz 88.2khz 96khz*/
unsigned long i2s_inclk_15p625Mhz[11] = {60<<8, 43<<8,  40<<8, 30<<8, 21<<8,  19<<8, 14<<8, 10<<8, 9<<8,  7<<8,  4<<8};
unsigned long i2s_exclk_12p288Mhz[11] = {47<<8, 34<<8,  31<<8, 23<<8, 16<<8,  15<<8, 11<<8,  8<<8, 7<<8,  5<<8,  3<<8};
unsigned long i2s_exclk_12Mhz[11]     = {46<<8, 33<<8,  30<<8, 22<<8, 16<<8,  15<<8, 11<<8,  8<<8, 7<<8,  5<<8,  3<<8};
#if defined(CONFIG_I2S_WM8750) || defined(CONFIG_SND_SOC_WM8750)
					/* 8k  11.025k  12k   16k  22.05k  24k  32k   44.1k   48k  88.2k   96k*/
unsigned long i2s_codec_12p288Mhz[11]  = {0x0C,  0x00, 0x10, 0x14,  0x38, 0x38, 0x18,  0x20, 0x00,  0x00, 0x1C};
unsigned long i2s_codec_12Mhz[11]      = {0x0C,  0x32, 0x10, 0x14,  0x37, 0x38, 0x18,  0x22, 0x00,  0x3E, 0x1C};
unsigned long i2s_codec_24p576Mhz[11]  = {0x4C,  0x00, 0x50, 0x54,  0x00, 0x78, 0x58,  0x00, 0x40,  0x00, 0x5C};
unsigned long i2s_codec_18p432Mhz[11]  = {0x0e,  0x32, 0x12, 0x16,  0x36, 0x3a, 0x1a,  0x22, 0x02,  0x3e, 0x1e};
#endif
#if defined(CONFIG_I2S_WM8751) || defined(CONFIG_SND_SOC_WM8751)
unsigned long i2s_codec_12p288Mhz[11]  = {0x04,  0x00, 0x10, 0x14,  0x38, 0x38, 0x18,  0x20, 0x00,  0x00, 0x1C};
unsigned long i2s_codec_12Mhz[11]      = {0x04,  0x32, 0x10, 0x14,  0x37, 0x38, 0x18,  0x22, 0x00,  0x3E, 0x1C};
#endif
#if defined(CONFIG_I2S_WM8960) || defined(CONFIG_SND_SOC_WM8960)
unsigned long i2s_codec_12p288Mhz[11]  = {0x36,  0x24, 0x24, 0x1b,  0x12, 0x12, 0x09,  0x00, 0x00,  0x00, 0x00};
unsigned long i2s_codec_12Mhz[11]      = {0x36,  0x24, 0x24, 0x1b,  0x12, 0x12, 0x09,  0x00, 0x00,  0x00, 0x00};
#endif

#if defined(CONFIG_RALINK_RT6855A)
				   /* 8K  11.025k  12k   16k  22.05k   24k   32k  44.1K   48k  88.2k  96k */
unsigned long i2s_inclk_int[11]  = {  97,    70,    65,   48,    35,    32,   24,   17,    16,   12,    8};
unsigned long i2s_inclk_comp[11] = { 336,   441,    53,  424,   220,   282,  212,  366,   141,  185,   70};
#elif defined (CONFIG_RALINK_MT7621)
#ifdef MT7621_ASIC_BOARD
#if defined (CONFIG_I2S_MCLK_12P288MHZ)
unsigned long i2s_inclk_int[11]  = { 576,   384,    0,   288,   192,   192,  144,   96,    96,   48,   48};
unsigned long i2s_inclk_comp[11] = {   0,     0,    0,     0,     0,     0,    0,    0,     0,    0,    0};
#elif defined(CONFIG_I2S_MCLK_12MHZ)
unsigned long i2s_inclk_int[11] =  {1171,   850,    0,   585,   425,   390,  292,  212,   195,   106,   97};
unsigned long i2s_inclk_comp[11] = { 448,   174,    0,   480,    87,   320,  496,  299,   160,   149,  336};
#endif
#else //MT7621_FPGA_BOARD
unsigned long i2s_inclk_int[11] =  { 529,   384,    0,   264,   192,   176,  132,   96,    88,    48,   44};
unsigned long i2s_inclk_comp[11] = { 102,     0,    0,   307,     0,   204,  153,    0,   102,     0,   51};
#endif
#elif defined (CONFIG_RALINK_MT7628)
#ifdef MT7628_ASIC_BOARD
                                      /* 8K  11.025k 12k  16k 22.05k 24k  32k 44.1K  48k  88.2k 96k  176k 192k */
unsigned long i2s_inclk_int_16bit[13] = {937,  680,   0,  468,  340, 312, 234, 170,  156,   85, 78,   42,  39};
unsigned long i2s_inclk_comp_16bit[13]= {256,  139,   0,  384,   69, 256, 192,  34,  128,   17, 64,  267,  32};
unsigned long i2s_inclk_int_24bit[13] = {625,  404,   0,  312,  226, 208, 156, 113,  104,   56, 52,   28,  26};
unsigned long i2s_inclk_comp_24bit[13]= {  0,  404,   0,  256,  387, 170, 128, 193,   85,  352, 42,  176,  21};
#else
				      /* 8K  11.025k 12k  16k 22.05k 24k  32k 44.1K  48k  88.2k 96k  176k 192k */
unsigned long i2s_inclk_int_16bit[13] = {468,  340,   0,  234,  170, 156, 117,  85,   78,   42, 39,   21,  19};
unsigned long i2s_inclk_comp_16bit[13]= {384,   69,   0,  192,   34, 128,  96,  17,   64,  264, 32,  133, 272};
unsigned long i2s_inclk_int_24bit[13] = {312,  202,   0,  156,  113, 104,  78,  56,   52,   28, 26,   14,  13};
unsigned long i2s_inclk_comp_24bit[13]= {256,  202,   0,  128,  193,  85,  64, 352,   42,  176,  21,  88,  10};
#endif
#elif defined (CONFIG_ARCH_MT7623)
#if defined MT7623_ASIC_BOARD
				      /* 8K  11.025k 12k  16k 22.05k 24k  32k 44.1K  48k  88.2k 96k  176k 192k */
unsigned long i2s_inclk_int_16bit[13] = {576,  384,   0,  288,  192, 192, 144,  96,  96,    48, 48,   24,  24};
unsigned long i2s_inclk_comp_16bit[13]= { 0,    0,    0,    0,   0,    0,   0,   0,   0,     0,  0,    0,   0};
unsigned long i2s_inclk_int_24bit[13] = {384,  256,   0,  192,  128, 128,  96,  64,  64,    32, 32,   16,  16};
unsigned long i2s_inclk_comp_24bit[13]= { 0,    0,    0,    0,   0,    0,   0,   0,   0,     0,  0,    0,   0};
#else
				      /* 8K  11.025k 12k  16k 22.05k 24k  32k 44.1K  48k  88.2k 96k  176k 192k */
unsigned long i2s_inclk_int_16bit[13] = {72,   48,    0,   36,  24,   24,  18,  12,   12,    6,  6,    3,   3};
unsigned long i2s_inclk_comp_16bit[13]= { 0,    0,    0,    0,   0,    0,   0,   0,    0,    0,  0,    0,   0};
unsigned long i2s_inclk_int_24bit[13] = {48,   32,    0,   24,  16,   16,  12,   8,    8,    4,  4,    2,   2};
unsigned long i2s_inclk_comp_24bit[13]= { 0,    0,    0,    0,   0,    0,   0,   0,    0,    0,  0,    0,   0};
#endif
#else
				  /* 8K  11.025k 12k  16k  22.05k  24k  32k  44.1K  48k  88.2k  96k */
unsigned long i2s_inclk_int[11]  = { 78,    56,   52,  39,   28,    26,  19,   14,   13,   9,    6};
unsigned long i2s_inclk_comp[11] = { 64,   352,   42,  32,  176,    21, 272,   88,   10, 455,  261};
#endif

#if defined(CONFIG_I2S_WITH_AEC)
aecFuncTbl_t *aecFuncP;
#endif
/* USB mode 22.05Khz register value in datasheet is 0x36 but will cause slow clock, 0x37 is correct value */
/* USB mode 44.1Khz register value in datasheet is 0x22 but will cause slow clock, 0x23 is correct value */

struct tasklet_struct i2s_tx_tasklet;
struct tasklet_struct i2s_rx_tasklet;

char test_buf[I2S_PAGE_SIZE];
char test_buf_1[I2S_PAGE_SIZE];
char test_buf_2[I2S_PAGE_SIZE];

static const struct file_operations i2s_fops = {
	owner		: THIS_MODULE,
	mmap		: i2s_mmap,
	open		: i2s_open,
	release		: i2s_release,
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	unlocked_ioctl:     i2s_ioctl,
#else	
	ioctl		: i2s_ioctl,
#endif	
};

int __init i2s_mod_init(void)
{
	int result;

	printk("******* i2s module init **********\n");
	/* register device with kernel */
#ifdef  CONFIG_DEVFS_FS
    	if(devfs_register_chrdev(i2sdrv_major, I2SDRV_DEVNAME , &i2s_fops)) {
		printk(KERN_WARNING " i2s: can't create device node - %s\n", I2SDRV_DEVNAME);
		return -EIO;
    	}

    	devfs_handle = devfs_register(NULL, I2SDRV_DEVNAME, DEVFS_FL_DEFAULT, i2sdrv_major, 0, 
	    S_IFCHR | S_IRUGO | S_IWUGO, &i2s_fops, NULL);
#else
    	result = register_chrdev(i2sdrv_major, I2SDRV_DEVNAME, &i2s_fops);
    	if (result < 0) {
		printk(KERN_WARNING "i2s: can't get major %d\n",i2sdrv_major);
        	return result;
    	}

    	if (i2sdrv_major == 0) {
		i2sdrv_major = result; /* dynamic */
    	}
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
#else	
	i2smodule_class=class_create(THIS_MODULE, I2SDRV_DEVNAME);
	if (IS_ERR(i2smodule_class)) 
		return -EFAULT;
	device_create(i2smodule_class, NULL, MKDEV(i2sdrv_major, 0), I2SDRV_DEVNAME);
#endif	

#if defined(CONFIG_I2S_WITH_AEC)
	printk("AEC FuncP init \n");
	/*Add by mtk04880*/
	aecFuncP = kmalloc(sizeof(aecFuncTbl_t), GFP_KERNEL);
	/*If aecFuncP cannot request memory,it will be ignored in I2S module. Since AEC & I2S are independent
	 * when AEC module is inserted,It will return err message (but I2S will keep running without AEC support)
	 * */
	if(aecFuncP){
		memset(aecFuncP,0,sizeof(aecFuncTbl_t));
	}
#endif

	return 0;
}

void i2s_mod_exit(void)
{
	printk("************ i2s module exit *************\n");	
#ifdef  CONFIG_DEVFS_FS
    	devfs_unregister_chrdev(i2sdrv_major, I2SDRV_DEVNAME);
    	devfs_unregister(devfs_handle);
#else
    	unregister_chrdev(i2sdrv_major, I2SDRV_DEVNAME);
#endif
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
#else
	device_destroy(i2smodule_class,MKDEV(i2sdrv_major, 0));
	class_destroy(i2smodule_class); 
#endif	
	return ;
}


int i2s_open(struct inode *inode, struct file *filp)
{
#if defined(I2S_HW_INTERRUPT_EN)&&(I2S_SW_IRQ_EN)
	int Ret;
#endif
	int minor = iminor(inode);

	if (minor >= I2S_MAX_DEV)
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

	/* set i2s_config */
	filp->private_data = pi2s_config;
	memset(pi2s_config, 0, sizeof(i2s_config_type));
#ifdef I2S_STATISTIC
	memset(pi2s_status, 0, sizeof(i2s_status_type));	
#endif
	i2s_param_init(pi2s_config);

#if defined(I2S_HW_INTERRUPT_EN)&&(I2S_SW_IRQ_EN)	
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	Ret = request_irq(SURFBOARDINT_I2S, i2s_irq_isr, IRQF_DISABLED, "Ralink_I2S", NULL);
#else
	Ret = request_irq(SURFBOARDINT_I2S, i2s_irq_isr, SA_INTERRUPT, "Ralink_I2S", NULL);
#endif
	
	if(Ret){
		MSG("IRQ %d is not free.\n", SURFBOARDINT_I2S);
		i2s_release(inode, filp);
		return -1;
	}
#endif	
 
    	init_waitqueue_head(&(pi2s_config->i2s_tx_qh));
    	init_waitqueue_head(&(pi2s_config->i2s_rx_qh));
	spin_lock_init(&pi2s_config->lock);

	return 0;
}


static int i2s_release(struct inode *inode, struct file *filp)
{
	i2s_config_type* ptri2s_config;
	
	/* decrement usage count */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
	MOD_DEC_USE_COUNT;
#else
	module_put(THIS_MODULE);
#endif

#if defined(I2S_HW_INTERRUPT_EN)&&(I2S_SW_IRQ_EN)
	free_irq(SURFBOARDINT_I2S, NULL);
#endif
	
	ptri2s_config = filp->private_data;
	if(ptri2s_config==NULL)
		goto EXIT;
#ifdef CONFIG_I2S_MMAP	
	i2s_mem_unmap(ptri2s_config);
#else
	i2s_txbuf_free(ptri2s_config);
	i2s_rxbuf_free(ptri2s_config);
#endif	
	/* free buffer */
	i2s_txPagebuf_free(ptri2s_config);
	i2s_rxPagebuf_free(ptri2s_config);	
EXIT:			
	MSG("i2s_release succeeds\n");
	return 0;
}

int i2s_mmap_alloc(unsigned long size)
{
	int i;
	u32 page_size;
       	int first_index;

	page_size = I2S_PAGE_SIZE;

	if ((pi2s_config->mmap_index == 0) || (pi2s_config->mmap_index == MAX_I2S_PAGE))
	{
		MSG("mmap_index=%d\n", pi2s_config->mmap_index);

		first_index = pi2s_config->mmap_index;
	pi2s_config->pMMAPBufPtr[pi2s_config->mmap_index] = kmalloc(size, GFP_DMA);
	i2s_mmap_addr[pi2s_config->mmap_index] = (dma_addr_t)dma_map_single(NULL, pi2s_config->pMMAPBufPtr[pi2s_config->mmap_index], size, DMA_BIDIRECTIONAL);
	
	if( pi2s_config->pMMAPBufPtr[pi2s_config->mmap_index] == NULL ) 
	{
		MSG("i2s_mmap failed\n");
		return -1;
	}
	}
	else
	{
		printk("illegal index:%d\n", pi2s_config->mmap_index);
		return -1;	
	}
	
	printk("MMAP[%d]=0x%08X, i2s_mmap_addr[%d]=0x%08x\n",
		pi2s_config->mmap_index, (u32)pi2s_config->pMMAPBufPtr[pi2s_config->mmap_index], 
                pi2s_config->mmap_index, i2s_mmap_addr[pi2s_config->mmap_index]);
	
	memset(pi2s_config->pMMAPBufPtr[pi2s_config->mmap_index], 0, size);
	pi2s_config->mmap_index++;

	for (i=1; i<MAX_I2S_PAGE; i++)
	{
		i2s_mmap_addr[pi2s_config->mmap_index] = i2s_mmap_addr[first_index] + i*page_size;
		pi2s_config->pMMAPBufPtr[pi2s_config->mmap_index] = pi2s_config->pMMAPBufPtr[first_index] + i*page_size;

		printk("MMAP[%d]=0x%08X, i2s_mmap_addr[%d]=0x%08x\n",pi2s_config->mmap_index, (u32)pi2s_config->pMMAPBufPtr[pi2s_config->mmap_index], pi2s_config->mmap_index, i2s_mmap_addr[pi2s_config->mmap_index]);
	
		/* Notice: The last mmap_index's value should be MAX_I2S_PAGE or MAX_I2S_PAGE*2 */
		pi2s_config->mmap_index++;
	}

	return 0;
}

int i2s_mmap_remap(struct vm_area_struct *vma, unsigned long size)
{
	int nRet;

	if((pi2s_config->pMMAPBufPtr[0]!=NULL) && (pi2s_config->mmap_index == MAX_I2S_PAGE))
	{
		MSG("i2s_mmap_remap:0\n");
		nRet = remap_pfn_range(vma, vma->vm_start, virt_to_phys((void *)pi2s_config->pMMAPBufPtr[0]) >> PAGE_SHIFT,  size, vma->vm_page_prot);

		if( nRet != 0 )
		{
			printk("i2s_mmap->remap_pfn_range failed\n");
			return -EIO;
		}
	}

	if((pi2s_config->pMMAPBufPtr[MAX_I2S_PAGE]!=NULL) && (pi2s_config->mmap_index == MAX_I2S_PAGE*2))
	{
		MSG("i2s_mmap_remap:%d\n", MAX_I2S_PAGE);

		nRet = remap_pfn_range(vma, vma->vm_start, virt_to_phys((void *)pi2s_config->pMMAPBufPtr[MAX_I2S_PAGE]) >> PAGE_SHIFT,  size, vma->vm_page_prot);
	
		if( nRet != 0 )
		{
			printk("i2s_mmap->remap_pfn_range failed\n");
			return -EIO;
		}
	}

	return 0;
}

static int i2s_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long size = vma->vm_end-vma->vm_start;
	printk("page_size=%d, ksize=%lu\n", I2S_PAGE_SIZE, size);

	if((pi2s_config->pMMAPBufPtr[0]==NULL)&&(pi2s_config->mmap_index!=0))
		pi2s_config->mmap_index = 0;
		
	printk("%s: vm_start=%08X,vm_end=%08X\n", __func__, (u32)vma->vm_start, (u32)vma->vm_end);
		
	/* Do memory allocate and dma sync */
	i2s_mmap_alloc(size);

	i2s_mmap_remap(vma, size);


	return 0;
}

int i2s_mem_unmap(i2s_config_type* ptri2s_config)
{
	u32 page_size;

	page_size = I2S_PAGE_SIZE;

	if(ptri2s_config->pMMAPBufPtr[0])
	{	
		printk("ummap MMAP[0]=0x%08X\n", (u32)ptri2s_config->pMMAPBufPtr[0]);
		dma_unmap_single(NULL, i2s_mmap_addr[0], MAX_I2S_PAGE*page_size, DMA_BIDIRECTIONAL);
		kfree(ptri2s_config->pMMAPBufPtr[0]);
	}

	if(ptri2s_config->pMMAPBufPtr[MAX_I2S_PAGE])
	{
		printk("ummap MMAP[%d]=0x%08X\n", MAX_I2S_PAGE, (u32)ptri2s_config->pMMAPBufPtr[MAX_I2S_PAGE]);
		dma_unmap_single(NULL, i2s_mmap_addr[MAX_I2S_PAGE], MAX_I2S_PAGE*page_size, DMA_BIDIRECTIONAL);
		kfree(ptri2s_config->pMMAPBufPtr[MAX_I2S_PAGE]);
	}

	ptri2s_config->mmap_index = 0;
	
	return 0;
}

int i2s_param_init(i2s_config_type* ptri2s_config)
{
	ptri2s_config->dmach = GDMA_I2S_TX0;
	ptri2s_config->tx_ff_thres = CONFIG_I2S_TFF_THRES;
	ptri2s_config->tx_ch_swap = CONFIG_I2S_CH_SWAP;
	ptri2s_config->rx_ff_thres = CONFIG_I2S_TFF_THRES;
	ptri2s_config->rx_ch_swap = CONFIG_I2S_CH_SWAP;
	ptri2s_config->slave_en = CONFIG_I2S_SLAVE_EN; 
	ptri2s_config->codec_pll_en = CONFIG_I2S_CODEC_PLL_EN;

	ptri2s_config->bRxDMAEnable = 0;
	ptri2s_config->bTxDMAEnable = 0;
	//ptri2s_config->bALSAEnable = 0;
	ptri2s_config->srate = 44100;
	ptri2s_config->txvol = 0;
	ptri2s_config->rxvol = 0;
	ptri2s_config->lbk = 0;
	ptri2s_config->extlbk = 0;
	ptri2s_config->txrx_coexist = 0;
	ptri2s_config->wordlen_24b = 0;
#if defined(CONFIG_RALINK_MT7628) || defined(CONFIG_ARCH_MT7623)
	ptri2s_config->sys_endian = 0;
	ptri2s_config->fmt = 0;
#endif
	ptri2s_config->micboost = 0;
	ptri2s_config->micin = 0;

	return 0;
}

int i2s_txbuf_alloc(i2s_config_type* ptri2s_config)
{
	int i;

	for( i = 0 ; i < MAX_I2S_PAGE ; i ++ )
        {
#if defined(CONFIG_I2S_MMAP)
		ptri2s_config->pMMAPTxBufPtr[i] = ptri2s_config->pMMAPBufPtr[i];
#else
                if(ptri2s_config->pMMAPTxBufPtr[i]==NULL)
                	ptri2s_config->pMMAPTxBufPtr[i] = kmalloc(I2S_PAGE_SIZE, GFP_KERNEL);
#endif
		memset(ptri2s_config->pMMAPTxBufPtr[i], 0, I2S_PAGE_SIZE);
	}

	return 0;
}

int i2s_rxbuf_alloc(i2s_config_type* ptri2s_config)
{
	int i;

	for( i = 0 ; i < MAX_I2S_PAGE ; i ++ )
        {
#if defined(CONFIG_I2S_MMAP)
        	ptri2s_config->pMMAPRxBufPtr[i] = ptri2s_config->pMMAPBufPtr[i+(ptri2s_config->mmap_index-MAX_I2S_PAGE)];
#else
                if(ptri2s_config->pMMAPRxBufPtr[i]==NULL)
			ptri2s_config->pMMAPRxBufPtr[i] = kmalloc(I2S_PAGE_SIZE, GFP_KERNEL);
#endif
		memset(ptri2s_config->pMMAPRxBufPtr[i], 0, I2S_PAGE_SIZE);
        }

	return 0;
}

int i2s_txPagebuf_alloc(i2s_config_type* ptri2s_config)
{
#if defined(ARM_ARCH)
	ptri2s_config->pPage0TxBuf8ptr = (u8*)pci_alloc_consistent(NULL, I2S_PAGE_SIZE , &i2s_txdma_addr0);
	ptri2s_config->pPage1TxBuf8ptr = (u8*)pci_alloc_consistent(NULL, I2S_PAGE_SIZE , &i2s_txdma_addr1);
	if(ptri2s_config->pPage0TxBuf8ptr==NULL)
        {
		MSG("Allocate Tx Page0 Buffer Failed\n");
                return -1;
        }
	if(ptri2s_config->pPage1TxBuf8ptr==NULL)
        {
		MSG("Allocate Tx Page1 Buffer Failed\n");
                return -1;
        }
#else
	ptri2s_config->pPage0TxBuf8ptr = (u8*)pci_alloc_consistent(NULL, I2S_PAGE_SIZE*2 , &i2s_txdma_addr);
        if(ptri2s_config->pPage0TxBuf8ptr==NULL)
        {
		MSG("Allocate Tx Page Buffer Failed\n");
                return -1;
        }
        ptri2s_config->pPage1TxBuf8ptr = ptri2s_config->pPage0TxBuf8ptr + I2S_PAGE_SIZE;
#endif
	return 0;
}

int i2s_rxPagebuf_alloc(i2s_config_type* ptri2s_config)
{
#if defined(ARM_ARCH)
	ptri2s_config->pPage0RxBuf8ptr = (u8*)pci_alloc_consistent(NULL, I2S_PAGE_SIZE, &i2s_rxdma_addr0);
	ptri2s_config->pPage1RxBuf8ptr = (u8*)pci_alloc_consistent(NULL, I2S_PAGE_SIZE, &i2s_rxdma_addr1);
	if(ptri2s_config->pPage0RxBuf8ptr==NULL)
	{
		MSG("Allocate Rx Page Buffer Failed\n");
		return -1;
	}
	if(ptri2s_config->pPage1RxBuf8ptr==NULL)
	{
		MSG("Allocate Rx Page Buffer Failed\n");
		return -1;
	}
#else
	ptri2s_config->pPage0RxBuf8ptr = (u8*)pci_alloc_consistent(NULL, I2S_PAGE_SIZE*2 , &i2s_rxdma_addr);
	if(ptri2s_config->pPage0RxBuf8ptr==NULL)
	{
		MSG("Allocate Rx Page Buffer Failed\n");
		return -1;
	}
	ptri2s_config->pPage1RxBuf8ptr = ptri2s_config->pPage0RxBuf8ptr + I2S_PAGE_SIZE;
#endif
	return 0;
}

int i2s_txbuf_free(i2s_config_type* ptri2s_config)
{
	int i;

	for(i = 0 ; i < MAX_I2S_PAGE ; i ++) 
	{
		if(ptri2s_config->pMMAPTxBufPtr[i] != NULL)
		{
#if defined(CONFIG_I2S_MMAP)
                        ptri2s_config->pMMAPTxBufPtr[i] = NULL;
#else
			kfree(ptri2s_config->pMMAPTxBufPtr[i]);
			ptri2s_config->pMMAPTxBufPtr[i] = NULL;
#endif
		}
	}
	return 0;
}

int i2s_rxbuf_free(i2s_config_type* ptri2s_config)
{
	int i;

	for(i = 0 ; i < MAX_I2S_PAGE ; i ++) 
	{
		if(ptri2s_config->pMMAPRxBufPtr[i] != NULL)
		{
#if defined(CONFIG_I2S_MMAP)
                        ptri2s_config->pMMAPRxBufPtr[i] = NULL;
#else
			kfree(ptri2s_config->pMMAPRxBufPtr[i]);
			ptri2s_config->pMMAPRxBufPtr[i] = NULL;
#endif
		}
	}
	
	return 0;
}

int i2s_txPagebuf_free(i2s_config_type* ptri2s_config)
{
#if defined(ARM_ARCH)
	if (ptri2s_config->pPage0TxBuf8ptr)
	{
		pci_free_consistent(NULL, I2S_PAGE_SIZE, ptri2s_config->pPage0TxBuf8ptr, i2s_txdma_addr0);
		ptri2s_config->pPage0TxBuf8ptr = NULL;
	}

	if (ptri2s_config->pPage1TxBuf8ptr)
	{
		pci_free_consistent(NULL, I2S_PAGE_SIZE, ptri2s_config->pPage1TxBuf8ptr, i2s_txdma_addr1);
		ptri2s_config->pPage1TxBuf8ptr = NULL;
	}
	printk("Free tx page buffer\n");
#else
	if (ptri2s_config->pPage0TxBuf8ptr)
	{
		pci_free_consistent(NULL, I2S_PAGE_SIZE*2, ptri2s_config->pPage0TxBuf8ptr, i2s_txdma_addr);
		ptri2s_config->pPage0TxBuf8ptr = NULL;
	}
#endif
	return 0;

}

int i2s_rxPagebuf_free(i2s_config_type* ptri2s_config)
{
#if defined(ARM_ARCH)
	if (ptri2s_config->pPage0RxBuf8ptr)
	{
		pci_free_consistent(NULL, I2S_PAGE_SIZE, ptri2s_config->pPage0RxBuf8ptr, i2s_rxdma_addr0);
		ptri2s_config->pPage0RxBuf8ptr = NULL;
	}
	if (ptri2s_config->pPage1RxBuf8ptr)
	{
		pci_free_consistent(NULL, I2S_PAGE_SIZE, ptri2s_config->pPage1RxBuf8ptr, i2s_rxdma_addr1);
		ptri2s_config->pPage1RxBuf8ptr = NULL;
	}
	printk("Free rx page buffer\n");
#else
	if (ptri2s_config->pPage0RxBuf8ptr)
	{
		pci_free_consistent(NULL, I2S_PAGE_SIZE*2, ptri2s_config->pPage0RxBuf8ptr, i2s_rxdma_addr);
		ptri2s_config->pPage0RxBuf8ptr = NULL;
	}
#endif
	return 0;
}

int i2s_reset_tx_param(i2s_config_type* ptri2s_config)
{
	ptri2s_config->tx_isr_cnt = 0;
	ptri2s_config->tx_w_idx = 0;
	ptri2s_config->tx_r_idx = 0;	
	ptri2s_config->enLable = 0;
	ptri2s_config->tx_pause_en = 0;
	ptri2s_config->end_cnt = 0;
	ptri2s_config->tx_stop_cnt = 0;

#ifdef I2S_STATISTIC
	pi2s_status->txbuffer_unrun = 0;
	pi2s_status->txbuffer_ovrun = 0;
	pi2s_status->txdmafault = 0;
	pi2s_status->txovrun = 0;
	pi2s_status->txunrun = 0;
	pi2s_status->txthres = 0;
	pi2s_status->txbuffer_len = 0;
#endif

	return 0;
}

int i2s_reset_rx_param(i2s_config_type* ptri2s_config)
{
	ptri2s_config->rx_isr_cnt = 0;
	ptri2s_config->rx_w_idx = 0;
	ptri2s_config->rx_r_idx = 0;	
	ptri2s_config->enLable = 0;
	ptri2s_config->rx_pause_en = 0;
	ptri2s_config->rx_stop_cnt = 0;

#ifdef I2S_STATISTIC
	pi2s_status->rxbuffer_unrun = 0;
	pi2s_status->rxbuffer_ovrun = 0;
	pi2s_status->rxdmafault = 0;
	pi2s_status->rxovrun = 0;
	pi2s_status->rxunrun = 0;
	pi2s_status->rxthres = 0;
	pi2s_status->rxbuffer_len = 0;
#endif

	return 0;
}	
#ifdef MT7621_ASIC_BOARD
int i2s_pll_config_mt7621(unsigned long index)
{
        unsigned long data;
	unsigned long regValue;
	bool xtal_20M_en = 0;
//	bool xtal_25M_en = 0;
	bool xtal_40M_en = 0;

	regValue = i2s_inw(RALINK_SYSCTL_BASE + 0x10);
       	regValue = (regValue >> 6) & 0x7;
	if (regValue < 3)
	{
		xtal_20M_en = 1;
		MSG("Xtal is 20MHz. \n");
	}
	else if (regValue < 6)
	{
		xtal_40M_en = 1;
		MSG("Xtal is 40M.\n");
	}
	else
	{
		//xtal_25M_en = 1;
		MSG("Xtal is 25M.\n");
	}

#if defined (CONFIG_I2S_MCLK_12P288MHZ)
	printk("MT7621 provide 12.288M/11.298MHz REFCLK\n");	
	/* Firstly, reset all required register to default value */
	i2s_outw(RALINK_ANA_CTRL_BASE, 0x00008000);
	i2s_outw(RALINK_ANA_CTRL_BASE+0x0014, 0x01001d61);//0x01401d61);
	i2s_outw(RALINK_ANA_CTRL_BASE+0x0018, 0x38233d0e);
	i2s_outw(RALINK_ANA_CTRL_BASE+0x001c, 0x80100004);//0x80120004);
	i2s_outw(RALINK_ANA_CTRL_BASE+0x0020, 0x1c7dbf48);

        /* toggle RG_XPTL_CHG */
        i2s_outw(RALINK_ANA_CTRL_BASE, 0x00008800);
        i2s_outw(RALINK_ANA_CTRL_BASE, 0x00008c00);

        data = i2s_inw(RALINK_ANA_CTRL_BASE+0x0014);
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
        	data |= REGBIT(0x1, 6);  /* for 40M */
	}
	i2s_outw(RALINK_ANA_CTRL_BASE+0x0014, data);


        data = i2s_inw(RALINK_ANA_CTRL_BASE+0x0018);
        data &= ~(0xf0773f00);
        data |= REGBIT(0x3, 28);
        data |= REGBIT(0x2, 20);
	if ((xtal_40M_en) || (xtal_20M_en))
	{
        	data |= REGBIT(0x3, 16); /* for 40M or 20M */
	}
	else
	{
        	data |= REGBIT(0x2, 16); /* for 25M */
	}
        data |= REGBIT(0x3, 12);
	if ((xtal_40M_en) || (xtal_20M_en))
	{
        	data |= REGBIT(0xd, 8);	/* for 40M or 20M */
	}
	else
	{
        	data |= REGBIT(0x7, 8);	/* for 25M */
	}
        i2s_outw(RALINK_ANA_CTRL_BASE+0x0018, data);

        if((index==1)|(index==4)|(index==7)|(index==9))// 270 MHz for 22.05K, 44.1K, 88.2K, 176.4K
        {
		if ((xtal_40M_en) || (xtal_20M_en))
		{
         	       	i2s_outw(RALINK_ANA_CTRL_BASE+0x0020, 0x1a18548a); /* for 40M or 20M */
		}
		else
		{
                	i2s_outw(RALINK_ANA_CTRL_BASE+0x0020, 0x14ad106e); /* for 25M */
		}
        }
        else if ((index==0)|(index==3)|(index==5)|(index==6)|(index==8)|(index==10))// 294 MHZ for 24K, 48K, 96K, 192K
        {
		if ((xtal_40M_en) || (xtal_20M_en))
		{
                	i2s_outw(RALINK_ANA_CTRL_BASE+0x0020, 0x1c7dbf48); /* for 40M or 20M */
		}
		else
		{
                	i2s_outw(RALINK_ANA_CTRL_BASE+0x0020, 0x1697cc39); /* for 25M */
		}
        }
	else if (index==2)
	{
		printk("Not support 12KHz sampling rate!\n");
		return -1;
	}
        else
        {
                printk("Wrong sampling rate!\n");
                return -1;
        }

        //*Common setting - Set PLLGP_CTRL_4 *//
	/* 1. Bit 31 */
        data = i2s_inw(RALINK_ANA_CTRL_BASE+0x001c);
        data &= ~(REGBIT(0x1, 31));
        i2s_outw(RALINK_ANA_CTRL_BASE+0x001c, data);
	ndelay(10);

        /* 2. Bit 0 */
        data = i2s_inw(RALINK_ANA_CTRL_BASE+0x001c);
        data |= REGBIT(0x1, 0);
        i2s_outw(RALINK_ANA_CTRL_BASE+0x001c, data);
	udelay(200);

        /* 3. Bit 3 */
        data = i2s_inw(RALINK_ANA_CTRL_BASE+0x001c);
        data |= REGBIT(0x1, 3);
        i2s_outw(RALINK_ANA_CTRL_BASE+0x001c, data);
	udelay(1);

        /* 4. Bit 8 */
        data = i2s_inw(RALINK_ANA_CTRL_BASE+0x001c);
        data |= REGBIT(0x1, 8);
        i2s_outw(RALINK_ANA_CTRL_BASE+0x001c, data);
	ndelay(40);

        /* 5. Bit 6 */
        data = i2s_inw(RALINK_ANA_CTRL_BASE+0x001c);
        data |= REGBIT(0x1, 6);
        i2s_outw(RALINK_ANA_CTRL_BASE+0x001c, data);
	ndelay(40);

        /* 6. Bit 5 & Bit 7*/
        data = i2s_inw(RALINK_ANA_CTRL_BASE+0x001c);
        data |= REGBIT(0x1, 5);
	data |= REGBIT(0x1, 7);
        i2s_outw(RALINK_ANA_CTRL_BASE+0x001c, data);
	udelay(1);

        /* 7. Bit 17 */
        data = i2s_inw(RALINK_ANA_CTRL_BASE+0x001c);
        data |= REGBIT(0x1, 17);
        i2s_outw(RALINK_ANA_CTRL_BASE+0x001c, data);

#elif defined(CONFIG_I2S_MCLK_12MHZ)
	printk("MT7621 provide 12MHz REFCLK\n");
	/* Firstly, reset all required register to default value */
	i2s_outw(RALINK_ANA_CTRL_BASE+0x0014, 0x01401d61);//0x01401d61);
	i2s_outw(RALINK_ANA_CTRL_BASE+0x001c, 0x80120004);//0x80100004);
	i2s_outw(RALINK_ANA_CTRL_BASE+0x0018, 0x38233d0e);

	if (xtal_40M_en)
	{
		data = i2s_inw(RALINK_ANA_CTRL_BASE+0x001c);
        	data &= ~REGBIT(0x1, 17);
        	i2s_outw(RALINK_ANA_CTRL_BASE+0x001c, data);

		data = i2s_inw(RALINK_ANA_CTRL_BASE+0x0014);
		data &= ~REGBIT(0x3, 4);
        	data |= REGBIT(0x1, 4);
        	i2s_outw(RALINK_ANA_CTRL_BASE+0x0014, data);

		data = i2s_inw(RALINK_ANA_CTRL_BASE+0x001c);
        	data &= ~REGBIT(0x1, 31);
        	i2s_outw(RALINK_ANA_CTRL_BASE+0x001c, data);
	}
	else if (xtal_20M_en)
	{
		data = i2s_inw(RALINK_ANA_CTRL_BASE+0x001c);
        	data &= ~REGBIT(0x1, 17);
        	i2s_outw(RALINK_ANA_CTRL_BASE+0x001c, data);

		data = i2s_inw(RALINK_ANA_CTRL_BASE+0x0014);
		data &= ~REGBIT(0x3, 6);
        	i2s_outw(RALINK_ANA_CTRL_BASE+0x0014, data);

		data = i2s_inw(RALINK_ANA_CTRL_BASE+0x0014);
		data &= ~REGBIT(0x3, 4);
        	data |= REGBIT(0x1, 4);
        	i2s_outw(RALINK_ANA_CTRL_BASE+0x0014, data);

		data = i2s_inw(RALINK_ANA_CTRL_BASE+0x001c);
        	data &= ~REGBIT(0x1, 31);
        	i2s_outw(RALINK_ANA_CTRL_BASE+0x001c, data);
	}
	else
	{
		data = i2s_inw(RALINK_ANA_CTRL_BASE+0x001c);
        	data &= ~REGBIT(0x1, 17);
        	i2s_outw(RALINK_ANA_CTRL_BASE+0x001c, data);

		data = i2s_inw(RALINK_ANA_CTRL_BASE+0x0014);
		data &= ~REGBIT(0x7f, 8);
        	data |= REGBIT(0x17, 8);
        	i2s_outw(RALINK_ANA_CTRL_BASE+0x0014, data);

		data = i2s_inw(RALINK_ANA_CTRL_BASE+0x0014);
		data &= ~REGBIT(0x3, 6);
        	i2s_outw(RALINK_ANA_CTRL_BASE+0x0014, data);

		data = i2s_inw(RALINK_ANA_CTRL_BASE+0x0018);
		data &= ~REGBIT(0x7, 16);
        	data |= REGBIT(0x2, 16);
        	i2s_outw(RALINK_ANA_CTRL_BASE+0x0018, data);

		data = i2s_inw(RALINK_ANA_CTRL_BASE+0x0018);
		data &= ~REGBIT(0xf, 8);
        	data |= REGBIT(0x7, 8);
        	i2s_outw(RALINK_ANA_CTRL_BASE+0x0018, data);


		data = i2s_inw(RALINK_ANA_CTRL_BASE+0x0014);
		data &= ~REGBIT(0x3, 4);
        	data |= REGBIT(0x1, 4);
        	i2s_outw(RALINK_ANA_CTRL_BASE+0x0014, data);

		data = i2s_inw(RALINK_ANA_CTRL_BASE+0x001c);
        	data &= ~REGBIT(0x1, 31);
        	i2s_outw(RALINK_ANA_CTRL_BASE+0x001c, data);

	}
#endif
        return 0;
}
#if defined(CONFIG_I2S_IN_MCLK)
int i2s_pll_refclk_set(void)
{
	unsigned long data;

	/* Set APLL register for REFCLK */
        data = i2s_inw(RALINK_SYSCTL_BASE+0x90);
        data &= ~(0x0000f000);
        data |= REGBIT(0x1, 12);
        i2s_outw(RALINK_SYSCTL_BASE+0x0090, data);

        data = i2s_inw(RALINK_SYSCTL_BASE+0x0090);
        data &= ~(0x00000300);
        i2s_outw(RALINK_SYSCTL_BASE+0x0090, data);
        MSG("Set 0x90 register\n");

	return 0;
}
#endif
#endif	

#ifdef MT7623_ASIC_BOARD
int i2s_pll_config_mt7623(unsigned long index)
{
	unsigned long data;

	/* xPLL PWR ON */
	data = i2s_inw(AUD2PLL_PWR_CON0);
	data |= 0x1;
	i2s_outw(AUD2PLL_PWR_CON0, data);
	udelay(5);

	/* xPLL ISO Disable */
	data = i2s_inw(AUD2PLL_PWR_CON0);
	data &= ~(0x2);
	i2s_outw(AUD2PLL_PWR_CON0, data);

	/* xPLL Frequency Set */
	data = i2s_inw(AUD2PLL_CON0);
	data |= 0x1;
	i2s_outw(AUD2PLL_CON0, data);

	/* AUD1PLL Frequency Set(change from 98.304MHz to 294.912MHz) */
	i2s_outw(AUD1PLL_CON0, 0x121);
	i2s_outw(AUD1PLL_CON1, 0xad5efee6);
	udelay(40);

	/* Audio clock setting */
	if((index==1)|(index==4)|(index==7)|(index==9)|(index==11))// for 22.05K, 44.1K, 88.2K, 176.4K
	{
		printk("\n*****%s:index=%d(270MHz)*****\n", __func__, (int)index);
		data = i2s_inw(0xFB00002c);
		//data &= ~REGBIT(0x8, 1);
		data &= ~(0x80);
		i2s_outw(0xFB00002C, data); /* AUD1PLL 270.9204MHz */
	}	
	else if ((index==0)|(index==3)|(index==5)|(index==6)|(index==8)|(index==10)|(index==12)) //for 24K, 48K, 96K, 192K
	{
		printk("\n*****%s:index=%d(294MHz)*****\n", __func__, (int)index);
		data = i2s_inw(0xFB00002c);
		//data |= REGBIT(0x8, 1);
        	data |= (0x80);
		i2s_outw(0xFB00002c, data); /* AUD1PLL 294.912MHz */
	}
	else if (index==2)
	{
		printk("Not support 12KHz sampling rate!\n");
		return -1;
	}
        else
        {
                printk("Wrong sampling rate!\n");
                return -1;
        }
	return 0;
}
#endif

#if defined(MT7628_ASIC_BOARD) || defined(CONFIG_ARCH_MT7623)
int i2s_driving_strength_adjust(void)
{
#if defined(MT7628_ASIC_BOARD)
        unsigned long data;

        MSG("Adjust MT7628 current's driving strngth\n");
        /* Adjust REFCLK0's driving strength of current which can avoid
         * the glitch of REFCKL0
         * E4 = 0xb0001354[5]; E8 = 0xb0001364[5]
         * (E4,E8)=(0,0)-> 4 mA;
         *        =(1,0)-> 8 mA;
         *        =(0,1)-> 12 mA;
         *        =(1,1)-> 16 mA*/

        /* Set to 12mA */
        data = i2s_inw(0xb0001354);
        data &= ~(0x1<<5);
        i2s_outw(0xb0001354, data);

        data = i2s_inw(0xb0001364);
        data |= (0x1<<5);
        i2s_outw(0xb0001364, data);
#endif
#if defined(CONFIG_ARCH_MT7623)
	MSG("Adjust MT7623 current's driving strngth\n");

	i2s_outw(0xF0005F80, 0x7777);
#endif

        return 0;
}
#endif

#if defined(CONFIG_I2S_IN_MCLK)
#if defined(CONFIG_I2S_MCLK_12MHZ)
int i2s_refclk_12m_enable(void)
{
	unsigned long data;
	
	MSG("Enable SoC MCLK 12Mhz\n");

#if defined(CONFIG_RALINK_RT6855A)
	data = i2s_inw(RALINK_SYSCTL_BASE+0x860);
	data |= (0x1<<17);
	data &= ~(0x7<<18);
	data |= (0x1<<18);
	i2s_outw(RALINK_SYSCTL_BASE+0x860, data);
#elif defined(CONFIG_RALINK_RT3350)
	data = i2s_inw(RALINK_SYSCTL_BASE+0x2c);
	data |= (0x1<<8);
	i2s_outw(RALINK_SYSCTL_BASE+0x2c, data);
#elif defined(CONFIG_RALINK_RT3883)
	data = i2s_inw(RALINK_SYSCTL_BASE+0x2c);
	data &= ~(0x03<<13);
	data |= (0x1<<13);	
	i2s_outw(RALINK_SYSCTL_BASE+0x2c, data);
#elif defined(CONFIG_RALINK_RT3352)||defined(CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) 
	data = i2s_inw(RALINK_SYSCTL_BASE+0x2c);
	data &= ~(0x0F<<8);
	data |= (0x3<<8);	
	i2s_outw(RALINK_SYSCTL_BASE+0x2c, data);
#elif defined(CONFIG_RALINK_MT7620)
	data = i2s_inw(RALINK_SYSCTL_BASE+0x2c);
	data &= ~(0x07<<9);
    	data |= (1<<9);
	i2s_outw(RALINK_SYSCTL_BASE+0x2c, data);
#elif defined(CONFIG_RALINK_MT7621)
	data = i2s_inw(RALINK_SYSCTL_BASE+0x2c);
	data &= ~(0x1f<<18);
	data |= REGBIT(0x19, 18);
	data &= ~(0x1f<<12);
	data |= REGBIT(0x1, 12);
	data &= ~(0x7<<9);
	data |= REGBIT(0x5, 9);
	i2s_outw(RALINK_SYSCTL_BASE+0x2c, data);
#elif defined(CONFIG_RALINK_MT7628)
	data = i2s_inw(RALINK_SYSCTL_BASE+0x2c);
	MSG("turn on REFCLK output for MCLK1\n");
	data &= ~(0x7<<9);
	data |= (0x1<<9);  /* output for MCLK */
	i2s_outw(RALINK_SYSCTL_BASE+0x2c, data);
#else	
	#error "This SoC does not provide 12MHz clock to audio codec\n");	
#endif
	i2s_refclk_gpio_out_config();

	return 0;
}
#endif

#if defined(CONFIG_I2S_MCLK_12P288MHZ)
int i2s_refclk_12p288m_enable(void)
{
	unsigned long data;
	MSG("Enable SoC MCLK 12.288Mhz\n");

#if defined(CONFIG_RALINK_RT3352)||defined(CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855)
	data = i2s_inw(RALINK_SYSCTL_BASE+0x2c);
	data &= ~(0x01F<<18);
	data |= 31<<18;
	data &= ~(0x01F<<12);
	data |= 1<<12;
	data |= (0xF<<8);
	i2s_outw(RALINK_SYSCTL_BASE+0x2c, data);
#elif defined(CONFIG_RALINK_MT7621)
	data = i2s_inw(RALINK_SYSCTL_BASE+0x2c);
	data &= ~(0x1f<<18);
	data |= REGBIT(0xc, 18);
	data &= ~(0x1f<<12);
	data |= REGBIT(0x1, 12);
	data &= ~(0x7<<9);
	data |= REGBIT(0x5, 9);
	i2s_outw(RALINK_SYSCTL_BASE+0x2c, data);
	printk("MT7621 provide REFCLK 12.288MHz/11.289MHz\n");
#elif defined(CONFIG_ARCH_MT7623)
	/* MT7623 does not need to set divider for REFCLK */
	/* GPIO126 - I2S0_MCLK */
	data = i2s_inw(0xF00058F0);
	data &= ~(0x7<<3);
	data |= (0x6<<3);
	i2s_outw(0xF00058F0, data);	
	/* GPIO_DIR8: OUT */
	data = i2s_inw(0xF0005070);
	data |= (0x1<<14);
	i2s_outw(0xF0005070, data);
#else
	#error "This SoC does not provide 12.288Mhz clock to audio codec\n");	
#endif
	
	return 0;
}
#endif

#if defined(CONFIG_I2S_MCLK_18P432MHZ)
int i2s_refclk_18p432m_enable(unsigned long index)
{
	unsigned long data;
	MSG("Enable SoC MCLK 18.432MHz/16.934MHz");

	if((index==1)|(index==4)|(index==7)|(index==9))// 16.934MHz for 22.05K, 44.1K, 88.2K, 176.4K
        {
		data = i2s_inw(ETHDMASYS_SYSCTL_BASE+0x2c);
		data &= ~(0x1<<7);
		i2s_outw(ETHDMASYS_SYSCTL_BASE+0x2c, data);
	}
	else if((index==0)|(index==3)|(index==5)|(index==6)|(index==8)|(index==10))// 18.432MHZ for 24K, 48K, 96K, 192K
        {
		data = i2s_inw(ETHDMASYS_SYSCTL_BASE+0x2c);
		data |= (0x1<<7);
		i2s_outw(ETHDMASYS_SYSCTL_BASE+0x2c, data);
	}

	data = i2s_inw(ETHDMASYS_SYSCTL_BASE+0x30);
	data |= (0x1<<17);
	i2s_outw(ETHDMASYS_SYSCTL_BASE+0x30, data);

	return 0;
}
#endif
#endif

int i2s_refclk_disable(void)
{
	unsigned long data;

#if defined(CONFIG_RALINK_RT6855A)
    	data = i2s_inw(RALINK_SYSCTL_BASE+0x860);
	data &= ~(1<<17);
	i2s_outw(RALINK_SYSCTL_BASE+0x860, data);
#elif defined(CONFIG_RALINK_RT3350)
	data = i2s_inw(RALINK_SYSCTL_BASE+0x2c);
	data &= ~(0x1<<8);
	i2s_outw(RALINK_SYSCTL_BASE+0x2c, data);
#elif defined(CONFIG_RALINK_RT3883)
	data = i2s_inw(RALINK_SYSCTL_BASE+0x2c);
	 data &= ~(0x0F<<13);	
	i2s_outw(RALINK_SYSCTL_BASE+0x2c, data);	
#elif defined(CONFIG_RALINK_RT3352)||defined(CONFIG_RALINK_RT5350)||defined (CONFIG_RALINK_RT6855)
	data = i2s_inw(RALINK_SYSCTL_BASE+0x2c);
	data &= ~(0x0F<<8);
	i2s_outw(RALINK_SYSCTL_BASE+0x2c, data);
#elif defined (CONFIG_RALINK_MT7620)||defined (CONFIG_RALINK_MT7621)||defined (CONFIG_RALINK_MT7628) 
	printk("turn off REFCLK output from internal CLK\n");
	data = i2s_inw(RALINK_SYSCTL_BASE+0x2c);
	data &= ~(0x07<<9);
	i2s_outw(RALINK_SYSCTL_BASE+0x2c, data);
#elif defined (CONFIG_ARCH_MT7623) /*FIXME:2*/
#ifdef MT7623_ASIC_BOARD
	printk("turn off REFCLK output from internal CLK\n");
	/* GPIO126 - I2S0_MCLK */
        data = i2s_inw(0xF00058F0);
        data &= ~(0x7<<3);
	//data |= (0x2<<3);
        i2s_outw(0xF00058F0, data);
	/* GPIO126 => GPIO_DIR8: IN */
	data = i2s_inw(0xF0005070);
	data &= ~(0x1<<14);
	i2s_outw(0xF0005070, data);
#else
	printk("turn off REFCLK output from internal CLK\n");
	data = i2s_inw(ETHDMASYS_SYSCTL_BASE+0x30);
	data &= ~(0x1<<17);
	i2s_outw(ETHDMASYS_SYSCTL_BASE+0x30, data);
#endif
#endif
	return 0;
}

int i2s_refclk_gpio_out_config(void)
{
#ifndef CONFIG_ARCH_MT7623
	unsigned long data; /* FIXME */
#endif
	
	/* Set REFCLK GPIO pin as REFCLK mode*/
#if defined(CONFIG_RALINK_MT7620)
	data = i2s_inw(RALINK_SYSCTL_BASE+0x60);
    	data &= ~(0x03<<21);  /* WDT */
    	data |= (1<<21);
	//data &= ~(0x03<<16);  /* PERST */
	//data |= (1<<16);
    	i2s_outw(RALINK_SYSCTL_BASE+0x60, data);
#endif
#if defined(CONFIG_RALINK_MT7621)
	data = i2s_inw(RALINK_SYSCTL_BASE+0x60);
	//data &= ~(0x3<<10); /* PERST */
	//data |= (0x2<<10);
	data &= ~(0x3<<8); /* WDT */
	data |= (0x2<<8);
	i2s_outw(RALINK_SYSCTL_BASE+0x60, data);
	MSG("Set 0x60 register\n");
#endif
#if defined(CONFIG_RALINK_MT7628)
	data = i2s_inw(RALINK_SYSCTL_BASE+0x60);
	data &= ~(0x1<<18);
	i2s_outw(RALINK_SYSCTL_BASE+0x60, data);
#endif

	return 0;
} 

int i2s_refclk_gpio_in_config(void)
{
#ifndef CONFIG_ARCH_MT7623
	unsigned long data; /* FIXME */
#endif

#if defined (CONFIG_RALINK_MT7620)
	data = i2s_inw(RALINK_SYSCTL_BASE+0x60);
    	data &= ~(0x03<<21);  /* WDT */
    	data |= (1<<21);
	//data &= ~(0x03<<16);  /* PERST */
	//data |= (1<<16);
    	i2s_outw(RALINK_SYSCTL_BASE+0x60, data);

	data = i2s_inw(RALINK_PIO_BASE);
	data &= ~(0x1<<17); /* GPIO share ping 17 for WDT */
	i2s_outw(RALINK_PIO_BASE, data);

	//data = i2s_inw(RALINK_PIO_BASE+0x04);
	//data &= ~(0x1<<4); /* GPIO share ping 36 for PERST */
	//i2s_outw(RALINK_PIO_BASE+0x04, data);
#endif
#if defined (CONFIG_RALINK_MT7621)
	data = i2s_inw(RALINK_SYSCTL_BASE+0x60);
	//data &= ~(0x3<<10); /* PERST */
	//data |= (0x1<<10);
	data &= ~(0x3<<8); /* WDT */
	data |= (0x1<<8);
	i2s_outw(RALINK_SYSCTL_BASE+0x60, data);
	
	data = i2s_inw(RALINK_PIO_BASE);
	//data &= ~(0x1<<19); /* GPIO share ping 19 for RERST */
	data &= ~(0x1<<18); /* GPIO share ping 18 for WDT */
	i2s_outw(RALINK_PIO_BASE, data);
#endif
#if defined (CONFIG_RALINK_MT7628)
	/* To use external OSC, set REFCLK_GPIO ping as GPIO mode and set it as input direction */
	data = i2s_inw(RALINK_SYSCTL_BASE+0x60);
	data |= (0x1<<18);
	i2s_outw(RALINK_SYSCTL_BASE+0x60, data);

	data = i2s_inw(RALINK_PIO_BASE+0x04);
	data &= ~(0x1<<5); /* GPIO share ping 37*/
	i2s_outw(RALINK_PIO_BASE+0x04, data);
#endif

	return 0;
}

int i2s_slave_clock_gpio_in_mt7623(void)
{
	unsigned long data;

	/* GPIO74(I2S0_BCLK)=>GPIO_DIR5: IN */
	data = i2s_inw(0xF0005040);
	data &= ~(0x1<<10);
	i2s_outw(0xF0005040, data);

	/* GPIO73(I2S0_LRCK)=>GPIO_DIR5: IN */
	data = i2s_inw(0xF0005040);
	data &= ~(0x1<<9);
	i2s_outw(0xF0005040, data);

	printk("i2s_slave_clock_gpio_in_mt7623\n");

	return 0;
}

int i2s_master_clock_gpio_out_mt7623(void)
{
	unsigned long data;

	/* GPIO74(I2S0_BCLK)=>GPIO_DIR5: OUT */
	data = i2s_inw(0xF0005040);
	data |= (0x1<<10);
	i2s_outw(0xF0005040, data);

	/* GPIO73(I2S0_LRCK)=>GPIO_DIR5: OUT */
	data = i2s_inw(0xF0005040);
	data |= (0x1<<9);
	i2s_outw(0xF0005040, data);
	
	printk("i2s_master_clock_gpio_out_mt7623\n");

	return 0;
}

int i2s_share_pin_mt7623(i2s_config_type* ptri2s_config)
{
	unsigned long data;
	
	printk("\nConfig MT7623 I2S pinmux\n");
	/* GPIO74 - I2S0_BCLK */
	data = i2s_inw(0xF0005840);
	data &= ~(0x7<<12);
	data |= (0x6<<12);
	i2s_outw(0xF0005840, data);

	/* GPIO73 - I2S0_LRCK */
	data = i2s_inw(0xF0005840);
	data &= ~(0x7<<9);
	data |= (0x6<<9);
	i2s_outw(0xF0005840, data);

	if(ptri2s_config->slave_en==0)
		i2s_master_clock_gpio_out_mt7623();
	else
		i2s_slave_clock_gpio_in_mt7623();

	/* GPIO49 - I2S0_DATA */
	data = i2s_inw(0xF00057F0);
	data &= ~(0x7<<12);
	data |= (0x6<<12);
	i2s_outw(0xF00057F0, data);
	/* GPIO_DIR4: OUT */
	data = i2s_inw(0xF0005030);
	data |= (0x1<<1);
	i2s_outw(0xF0005030, data);

	/* GPIO72 - I2S0_DATA_IN */
	data = i2s_inw(0xF0005840);
	data &= ~(0x7<<6);
	data |= (0x6<<6);
	i2s_outw(0xF0005840, data);
	/* GPIO_DIR5: IN */
	data = i2s_inw(0xF0005040);
	data &= ~(0x1<<8);
	i2s_outw(0xF0005040, data);

	return 0;
}

int i2s_share_pin_config(i2s_config_type* ptri2s_config)
{
#ifndef CONFIG_ARCH_MT7623
	unsigned long data; /*FIXME*/
#endif
	
	/* set share pins to i2s/gpio mode and i2c mode */
#if defined(CONFIG_RALINK_RT6855A)
	data = i2s_inw(RALINK_SYSCTL_BASE+0x860);
	data |= 0x00008080;
	i2s_outw(RALINK_SYSCTL_BASE+0x860, data);
#elif defined(CONFIG_RALINK_MT7621)	
	data = i2s_inw(RALINK_SYSCTL_BASE+0x60); 
	data &= 0xFFFFFFE3;
	data |= 0x00000010;
	i2s_outw(RALINK_SYSCTL_BASE+0x60, data);
#elif defined(CONFIG_RALINK_MT7628)	
	data = i2s_inw(RALINK_SYSCTL_BASE+0x60); 
	data &= ~(0x3<<6);    /* I2S_MODE */ 
	data &= ~(0x3<<20);   /* I2C_MODE */
	i2s_outw(RALINK_SYSCTL_BASE+0x60, data);
#elif defined(CONFIG_ARCH_MT7623)
	i2s_share_pin_mt7623(ptri2s_config);
#else	
	data = i2s_inw(RALINK_SYSCTL_BASE+0x60); 
	data &= 0xFFFFFFE2;
	data |= 0x00000018;
	i2s_outw(RALINK_SYSCTL_BASE+0x60, data);
#endif
	return 0;
}

int i2s_ws_config(i2s_config_type* ptri2s_config, unsigned long index)
{
	unsigned long data;
	unsigned long* pTable;

#if defined(CONFIG_I2S_IN_CLK)
	/* REFCLK is 15.625Mhz or 40Mhz(fractional division) */
#if defined(CONFIG_I2S_FRAC_DIV)
	MSG("Internal REFCLK with fractional division\n");
#if defined(CONFIG_RALINK_MT7628) || defined(CONFIG_ARCH_MT7623)
	if (ptri2s_config->wordlen_24b == 1)
	{
		MSG("24 bit int table\n");
		pTable = i2s_inclk_int_24bit;
	}
	else
	{
		MSG("16 bit int table\n");
		pTable = i2s_inclk_int_16bit;
	}
#else
	pTable = i2s_inclk_int;
#endif /* CONFIG_RALINK_MT7628 */
	
	data = (unsigned long)(pTable[index]);
	i2s_outw(I2S_DIVINT_CFG, data);

#if defined(CONFIG_RALINK_MT7628) || defined(CONFIG_ARCH_MT7623)
	if (ptri2s_config->wordlen_24b == 1)
	{
		MSG("24 bit comp table\n");
		pTable = i2s_inclk_comp_24bit;
	}
	else
	{
		MSG("16 bit comp table\n");
		pTable = i2s_inclk_comp_16bit;
	}
#else
	pTable = i2s_inclk_comp;
#endif	/* CONFIG_RALINK_MT7628 */

	data = (unsigned long)(pTable[index]);
	data |= REGBIT(1, I2S_CLKDIV_EN); 
	i2s_outw(I2S_DIVCOMP_CFG, data);
#else
	MSG("Internal REFCLK 15.625Mhz \n");
	pTable = i2s_inclk_15p625Mhz;
	data = i2s_inw(RALINK_SYSCTL_BASE+0x30); 
	data &= 0xFFFF00FF;
	data |= (unsigned long)(pTable[index]);
	data |= 0x00008000;
	i2s_outw(RALINK_SYSCTL_BASE+0x30, data);  
#endif /* CONFIG_I2S_FRAC_DIV */
#else
#if defined(CONFIG_I2S_MCLK_12MHZ)
	/* REFCLK = MCLK = 12Mhz */
	MSG("External REFCLK 12Mhz \n");
	pTable = i2s_exclk_12Mhz;
	data = i2s_inw(RALINK_SYSCTL_BASE+0x30);
	data &= 0xFFFF00FF;
	data |= (unsigned long)(pTable[index]); 
	data |= 0x0000C000;
	i2s_outw(RALINK_SYSCTL_BASE+0x30, data);  	
#else
	/* REFCLK = MCLK = 12.288Mhz */
	pTable = i2s_exclk_12p288Mhz;
	MSG("External REFCLK 12.288Mhz \n");
	data = i2s_inw(RALINK_SYSCTL_BASE+0x30);
	data &= 0xFFFF00FF;
	data |= (unsigned long)(pTable[index]); 
	data |= 0x0000C000;
	i2s_outw(RALINK_SYSCTL_BASE+0x30, data); 					 
#endif /* CONFIG_I2S_MCLK_12MHZ */		
#endif /* Not CONFIG_I2S_IN_CLK */
	
#if defined(CONFIG_I2S_WS_EDGE)
	data = i2s_inw(I2S_I2SCFG);
	data |= REGBIT(0x1, I2S_WS_INV);
	i2s_outw(I2S_I2SCFG, data);
#endif

	return 0;
}

int i2s_mode_config(u32 slave_en)
{
	unsigned long data;
	
	if(slave_en==0)
	{
		/* Master mode*/
		printk("This SoC is in Master mode\n");
#if defined(CONFIG_RALINK_RT3052)
		data = i2s_inw(I2S_I2SCFG);
		data &= ~REGBIT(0x1, I2S_SLAVE_EN);
		data &= ~REGBIT(0x1, I2S_CLK_OUT_DIS);
		i2s_outw(I2S_I2SCFG, data);
#elif defined(CONFIG_RALINK_RT3883)||defined(CONFIG_RALINK_RT3352)||\
	defined(CONFIG_RALINK_RT5350)||defined(CONFIG_RALINK_RT6855)||\
	defined(CONFIG_RALINK_MT7620)||defined(CONFIG_RALINK_RT6855A)||\
	defined(CONFIG_RALINK_MT7621)||defined(CONFIG_RALINK_MT7628)||\
	defined(CONFIG_ARCH_MT7623)
		data = i2s_inw(I2S_I2SCFG);
		data &= ~REGBIT(0x1, I2S_SLAVE_MODE);
		i2s_outw(I2S_I2SCFG, data);
#else
	#error "a strange clock mode"	
#endif	
	}
	else
	{
		/* Slave mode */
		printk("This SoC is in Slave mode\n");
#if defined(CONFIG_RALINK_RT3052)
		data = i2s_inw(I2S_I2SCFG);
		data |= REGBIT(0x1, I2S_SLAVE_EN);
		data |= REGBIT(0x1, I2S_CLK_OUT_DIS);
		i2s_outw(I2S_I2SCFG, data);
#elif defined(CONFIG_RALINK_RT3883)||defined(CONFIG_RALINK_RT3352)||\
	defined(CONFIG_RALINK_RT5350)||defined(CONFIG_RALINK_RT6855)||\
	defined(CONFIG_RALINK_MT7620)||defined(CONFIG_RALINK_RT6855A)||\
	defined(CONFIG_RALINK_MT7621)||defined(CONFIG_RALINK_MT7628)||\
	defined(CONFIG_ARCH_MT7623)
		data = i2s_inw(I2S_I2SCFG);
		data |= REGBIT(0x1, I2S_SLAVE_MODE);
		i2s_outw(I2S_I2SCFG, data);
#else
		#error "a strange clock mode "	
#endif
	}

	return 0;
}

int i2s_codec_frequency_config(i2s_config_type* ptri2s_config, unsigned long index)
{
#if defined(CONFIG_I2S_WM8960)||defined(CONFIG_I2S_WM8750)||defined(CONFIG_I2S_WM8751)
	unsigned long data;
	unsigned long* pTable;
#endif

#if defined(CONFIG_I2S_MCLK_12MHZ)
#if defined(CONFIG_I2S_WM8960)||defined(CONFIG_I2S_WM8750)||defined(CONFIG_I2S_WM8751)
	pTable = i2s_codec_12Mhz;
	data = pTable[index];
#endif
#if defined(CONFIG_I2S_WM8960)
	audiohw_set_frequency(data, ptri2s_config->codec_pll_en);
#elif defined(CONFIG_I2S_WM8750)||defined(CONFIG_I2S_WM8751)
	audiohw_set_frequency(data|0x01);
#endif	
#else
#if defined(CONFIG_I2S_WM8960)||defined(CONFIG_I2S_WM8750)||defined(CONFIG_I2S_WM8751)
#if defined(MT7623_FPGA_BOARD) && defined(CONFIG_I2S_WM8750)
	pTable = i2s_codec_18p432Mhz;
#else
	pTable = i2s_codec_12p288Mhz;
#endif
	data = pTable[index];
#endif
#if defined(CONFIG_I2S_WM8960)
	audiohw_set_frequency(data, ptri2s_config->codec_pll_en);
#elif defined(CONFIG_I2S_WM8750)||defined(CONFIG_I2S_WM8751)
	audiohw_set_frequency(data);
#endif
#endif
	return 0;
}

/*
 *  Ralink Audio System Clock Enable
 *	
 *  I2S_WS : signal direction opposite to/same as I2S_CLK 
 *
 *  I2S_CLK : Integer division or fractional division
 *			  REFCLK from Internal or External (external REFCLK not support for fractional division)
 *			  Suppose external REFCLK always be the same as external MCLK
 * 		
 *  MCLK : External OSC or internal generation
 *
 */
int i2s_clock_enable(i2s_config_type* ptri2s_config)
{
	unsigned long index;
	/* audio sampling rate decision */
	switch(ptri2s_config->srate)
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
#if defined(CONFIG_RALINK_MT7628) || defined(CONFIG_ARCH_MT7623)
		case 176000:
			index = 11;
			break;
		case 192000:
			index = 12;
			break;
#endif
		default:
			index = 7;
	}
#ifdef MT7621_ASIC_BOARD
        /* Set pll config  */
        i2s_pll_config_mt7621(index);
#endif
#ifdef MT7623_ASIC_BOARD
        /* Set pll config  */
        i2s_pll_config_mt7623(index);
#endif

	/* enable internal MCLK */
#if defined(CONFIG_I2S_IN_MCLK)
#if defined(CONFIG_RALINK_MT7621)
	i2s_pll_refclk_set();
#endif
#if defined(CONFIG_I2S_MCLK_12MHZ)
#if defined(MT7628_ASIC_BOARD) || defined(CONFIG_ARCH_MT7623)
        i2s_driving_strength_adjust();
#endif
	i2s_refclk_12m_enable();
#endif /* MCLK_12MHZ */
#if defined(CONFIG_I2S_MCLK_12P288MHZ)
	i2s_refclk_12p288m_enable();
#endif /* MCLK_12P288MHZ */
#if defined(CONFIG_I2S_MCLK_18P432MHZ)
	i2s_refclk_18p432m_enable(index);
#endif
	i2s_refclk_gpio_out_config();

#else	
	MSG("Disable SoC MCLK, use external OSC\n");
	i2s_refclk_disable();
	i2s_refclk_gpio_in_config();
#endif /* CONFIG_I2S_IN_MCLK */	

	i2s_share_pin_config(ptri2s_config);	
	
	if(ptri2s_config->slave_en==0)
	{
		/* Setup I2S_WS and I2S_CLK */
		i2s_ws_config(ptri2s_config, index);	
	}

	i2s_mode_config(ptri2s_config->slave_en);

	if(!ptri2s_config->bALSAEnable)
	{
#if defined(CONFIG_I2S_WM8750) || defined(CONFIG_I2S_WM8751)|| defined(CONFIG_I2S_WM8960)
	i2s_codec_enable(ptri2s_config);
#endif
	i2s_codec_frequency_config(ptri2s_config,index);
	}

	return 0;
}	

int i2s_clock_disable(i2s_config_type* ptri2s_config)
{
	if(!ptri2s_config->bALSAEnable)
	{
#if defined(CONFIG_I2S_WM8960) || defined(CONFIG_I2S_WM8750) || defined(CONFIG_I2S_WM8751)
	i2s_codec_disable(ptri2s_config);
#endif
	}

	/* disable internal MCLK */
#if defined(CONFIG_I2S_IN_MCLK)	
	i2s_refclk_disable();
	i2s_refclk_gpio_in_config();
#endif
	return 0;
}	


int i2s_codec_enable(i2s_config_type* ptri2s_config)
{
	
	int AIn = 0, AOut = 0;
#if 1
#if defined(CONFIG_I2S_WM8960) || defined(CONFIG_I2S_WM8750) || defined(CONFIG_I2S_WM8751)
	/* Codec initialization */
	audiohw_preinit();
#endif
#endif

#if defined(CONFIG_I2S_WM8960)
	if(ptri2s_config->codec_pll_en)
	{
		MSG("Codec PLL EN = %d\n", pi2s_config->codec_pll_en);
		audiohw_set_apll(ptri2s_config->srate);
	}
#endif

#if defined(CONFIG_I2S_TXRX)	
	if((ptri2s_config->bTxDMAEnable) || (ptri2s_config->txrx_coexist))
		AOut = 1;
	if((ptri2s_config->bRxDMAEnable) || (ptri2s_config->txrx_coexist))
		AIn = 1;
#if defined(CONFIG_I2S_WM8960)
	audiohw_postinit(!(ptri2s_config->slave_en), AIn, AOut, ptri2s_config->codec_pll_en, ptri2s_config->wordlen_24b);
	audiohw_micboost(ptri2s_config->micboost);	
	audiohw_micin(ptri2s_config->micin);
#elif defined(CONFIG_I2S_WM8750)
	audiohw_postinit(!(ptri2s_config->slave_en), AIn, AOut, ptri2s_config->wordlen_24b);
#endif
	MSG("AOut=%d, AIn=%d\n", AOut, AIn);
#else
#if defined(CONFIG_I2S_WM8750)
	audiohw_postinit(!(ptri2s_config->slave_en), 0, 1);
#elif defined(CONFIG_I2S_WM8960)	
	audiohw_postinit(!(ptri2s_config->slave_en), 1, 1, ptri2s_config->codec_pll_en);
#elif defined(CONFIG_I2S_WM8751)	
	if(ptri2s_config->slave_en==0)
		audiohw_postinit(1,1);
	else
		audiohw_postinit(0,1);
#endif		
#endif
	return 0;	
}

int i2s_codec_disable(i2s_config_type* ptri2s_config)
{
#if defined(CONFIG_I2S_WM8960) || defined(CONFIG_I2S_WM8750) || defined(CONFIG_I2S_WM8751)
	audiohw_close();
#endif
	return 0;
}	

int i2s_reset_config(i2s_config_type* ptri2s_config)
{
	unsigned long data;

	/* RESET bit: write 1 clear */
#if defined(CONFIG_RALINK_RT6855A)
	data = i2s_inw(RALINK_SYSCTL_BASE+0x834);
	data |= (1<<17);
	i2s_outw(RALINK_SYSCTL_BASE+0x834, data);

	data = i2s_inw(RALINK_SYSCTL_BASE+0x834);
	data &= ~(1<<17);
	i2s_outw(RALINK_SYSCTL_BASE+0x834, data);
#elif defined(CONFIG_ARCH_MT7623)
	data = i2s_inw(0xFB000000+0x34);
	data |= (1<<17);
	i2s_outw(0xFB000000+0x34, data);

	data = i2s_inw(0xFB000000+0x34);
	data &= ~(1<<17);
	i2s_outw(0xFB000000+0x34, data);
#else
	data = i2s_inw(RALINK_SYSCTL_BASE+0x34);
	data |= (1<<17);
	i2s_outw(RALINK_SYSCTL_BASE+0x34, data);

	data = i2s_inw(RALINK_SYSCTL_BASE+0x34);
	data &= ~(1<<17);
	i2s_outw(RALINK_SYSCTL_BASE+0x34, data);

#if 0  /* Reset GDMA */
	data = i2s_inw(RALINK_SYSCTL_BASE+0x34);
	data |= (1<<14);
	i2s_outw(RALINK_SYSCTL_BASE+0x34, data);

	data = i2s_inw(RALINK_SYSCTL_BASE+0x34);
	data &= ~(1<<14);
	i2s_outw(RALINK_SYSCTL_BASE+0x34, data);
#endif
#endif
	printk("I2S reset complete!!\n");	
	return 0;
}

int i2s_tx_config(i2s_config_type* ptri2s_config)
{
	unsigned long data;
	/* set I2S_I2SCFG */
	data = i2s_inw(I2S_I2SCFG);
	data &= 0xFFFFFF81;
	data |= REGBIT(ptri2s_config->tx_ff_thres, I2S_TX_FF_THRES);
	data |= REGBIT(ptri2s_config->tx_ch_swap, I2S_TX_CH_SWAP);
#if defined(CONFIG_RALINK_RT6855A)	
	data |= REGBIT(1, I2S_BYTE_SWAP);
#endif
#if defined(CONFIG_RALINK_MT7628) || defined(CONFIG_ARCH_MT7623)
	MSG("TX:wordLen=%d, sysEndian=%d\n", ptri2s_config->wordlen_24b, ptri2s_config->sys_endian);
	data |= REGBIT(ptri2s_config->wordlen_24b, I2S_DATA_24BIT);
	data |= REGBIT(ptri2s_config->sys_endian, I2S_SYS_ENDIAN);
	data |= REGBIT(ptri2s_config->little_edn, I2S_LITTLE_ENDIAN);
#endif	
	data &= ~REGBIT(1, I2S_TX_CH0_OFF);
	data &= ~REGBIT(1, I2S_TX_CH1_OFF);
	i2s_outw(I2S_I2SCFG, data);

	/* set I2S_I2SCFG1 */
	MSG("internal loopback: %d\n", ptri2s_config->lbk);
	data = i2s_inw(I2S_I2SCFG1);
	data |= REGBIT(ptri2s_config->lbk, I2S_LBK_EN);
	data |= REGBIT(ptri2s_config->extlbk, I2S_EXT_LBK_EN);
	data &= 0xFFFFFFFC;
#if defined(CONFIG_RALINK_MT7628) || defined(CONFIG_ARCH_MT7623)
	data |= REGBIT(ptri2s_config->fmt, I2S_DATA_FMT);
#endif
	i2s_outw(I2S_I2SCFG1, data);
	
	return 0;
}	

int i2s_rx_config(i2s_config_type* ptri2s_config)
{
	unsigned long data;
	/* set I2S_I2SCFG */
	data = i2s_inw(I2S_I2SCFG);
	data &= 0xFFFF81FF;
	data |= REGBIT(ptri2s_config->rx_ff_thres, I2S_RX_FF_THRES);
	data |= REGBIT(ptri2s_config->rx_ch_swap, I2S_RX_CH_SWAP);
	data &= ~REGBIT(1, I2S_RX_CH0_OFF);
	data &= ~REGBIT(1, I2S_RX_CH1_OFF);
#if defined(CONFIG_RALINK_MT7628) || defined(CONFIG_ARCH_MT7623)
	MSG("RX:wordLen=%d, sysEndian=%d\n", ptri2s_config->wordlen_24b, ptri2s_config->sys_endian);
	data |= REGBIT(ptri2s_config->wordlen_24b, I2S_DATA_24BIT);
	data |= REGBIT(ptri2s_config->sys_endian, I2S_SYS_ENDIAN);
	data |= REGBIT(ptri2s_config->little_edn, I2S_LITTLE_ENDIAN);
#endif	
	i2s_outw(I2S_I2SCFG, data);

	/* set I2S_I2SCFG1 */
	data = i2s_inw(I2S_I2SCFG1);
	data |= REGBIT(ptri2s_config->lbk, I2S_LBK_EN);
	data |= REGBIT(ptri2s_config->extlbk, I2S_EXT_LBK_EN);
#if defined(CONFIG_RALINK_MT7628) || defined(CONFIG_ARCH_MT7623)
	data &= 0xFFFFFFFC;
	data |= REGBIT(ptri2s_config->fmt, I2S_DATA_FMT);
#endif
	i2s_outw(I2S_I2SCFG1, data);
	
	return 0;	
}

/* Turn On Tx DMA and INT */
int i2s_tx_enable(i2s_config_type* ptri2s_config)
{
	unsigned long data;

#if defined(I2S_HW_INTERRUPT_EN)	
	data = i2s_inw(I2S_INT_EN);
	data |= REGBIT(0x1, I2S_TX_INT3_EN);  /* FIFO DMA fault */
	data |= REGBIT(0x1, I2S_TX_INT2_EN);  /* FIFO overrun */
	data |= REGBIT(0x1, I2S_TX_INT1_EN);  /* FIFO underrun */
	data |= REGBIT(0x1, I2S_TX_INT0_EN);  /* FIFO below threshold */
	i2s_outw(I2S_INT_EN, data);
#endif	

	data = i2s_inw(I2S_I2SCFG);
#if defined(CONFIG_I2S_TXRX)	
	data |= REGBIT(0x1, I2S_TX_EN);
#endif	
	data |= REGBIT(0x1, I2S_DMA_EN);
	i2s_outw(I2S_I2SCFG, data);
	
	data = i2s_inw(I2S_I2SCFG);
	data |= REGBIT(0x1, I2S_EN);
	i2s_outw(I2S_I2SCFG, data);
	
	MSG("i2s_tx_enable done\n");
	return I2S_OK;
}

/* Turn On Rx DMA and INT */
int i2s_rx_enable(i2s_config_type* ptri2s_config)
{
	unsigned long data;

#if defined(I2S_HW_INTERRUPT_EN)	
	data = i2s_inw(I2S_INT_EN);
	data |= REGBIT(0x1, I2S_RX_INT3_EN);  /* FIFO DMA fault */
	data |= REGBIT(0x1, I2S_RX_INT2_EN);  /* FIFO overrun */
	data |= REGBIT(0x1, I2S_RX_INT1_EN);  /* FIFO underrun */
	data |= REGBIT(0x1, I2S_RX_INT0_EN);  /* FIFO below threshold */
	i2s_outw(I2S_INT_EN, data);
#endif
	
	data = i2s_inw(I2S_I2SCFG);
#if defined(CONFIG_I2S_TXRX)	
	data |= REGBIT(0x1, I2S_RX_EN);
#endif	
	data |= REGBIT(0x1, I2S_DMA_EN);
	i2s_outw(I2S_I2SCFG, data);
	
	data = i2s_inw(I2S_I2SCFG);
	data |= REGBIT(0x1, I2S_EN);
	i2s_outw(I2S_I2SCFG, data);
	
	MSG("i2s_rx_enable done\n");
	return I2S_OK;
}
/* Turn Off Tx DMA and INT */
int i2s_tx_disable(i2s_config_type* ptri2s_config)
{
	unsigned long data;

#if defined(I2S_HW_INTERRUPT_EN)	
	data = i2s_inw(I2S_INT_EN);
	data &= ~REGBIT(0x1, I2S_TX_INT3_EN);
	data &= ~REGBIT(0x1, I2S_TX_INT2_EN);
	data &= ~REGBIT(0x1, I2S_TX_INT1_EN);
	data &= ~REGBIT(0x1, I2S_TX_INT0_EN);
	i2s_outw(I2S_INT_EN, data);
#endif	

	data = i2s_inw(I2S_I2SCFG);
#if defined(CONFIG_I2S_TXRX)	
	data &= ~REGBIT(0x1, I2S_TX_EN);
#endif	
	if(ptri2s_config->bRxDMAEnable==0)
	{
		ptri2s_config->bTxDMAEnable = 0;
		data &= ~REGBIT(0x1, I2S_DMA_EN);
                data &= ~REGBIT(0x1, I2S_EN);
	}
	i2s_outw(I2S_I2SCFG, data);
	return I2S_OK;
}
/* Turn Off Rx DMA and INT */	
int i2s_rx_disable(i2s_config_type* ptri2s_config)
{
	unsigned long data;

#if defined(I2S_HW_INTERRUPT_EN)	
	data = i2s_inw(I2S_INT_EN);
	data &= ~REGBIT(0x1, I2S_RX_INT3_EN);
	data &= ~REGBIT(0x1, I2S_RX_INT2_EN);
	data &= ~REGBIT(0x1, I2S_RX_INT1_EN);
	data &= ~REGBIT(0x1, I2S_RX_INT0_EN);
	i2s_outw(I2S_INT_EN, data);
#endif
	
	data = i2s_inw(I2S_I2SCFG);
#if defined(CONFIG_I2S_TXRX)	
	data &= ~REGBIT(0x1, I2S_RX_EN);
#endif
	if(ptri2s_config->bTxDMAEnable==0)
	{
		ptri2s_config->bRxDMAEnable = 0;
		data &= ~REGBIT(0x1, I2S_DMA_EN);
                data &= ~REGBIT(0x1, I2S_EN);
	}
	i2s_outw(I2S_I2SCFG, data);
	return I2S_OK;
}

int i2s_dma_tx_transf_data(i2s_config_type* ptri2s_config, u32 dma_ch)
{
	int tx_r_idx;
 
	if ((pi2s_config->bALSAEnable==1) && (pi2s_config->bALSAMMAPEnable==1))
		tx_r_idx = (pi2s_config->tx_r_idx + ALSA_MMAP_IDX_SHIFT)%MAX_I2S_PAGE;
	else
		tx_r_idx = pi2s_config->tx_r_idx;

	if(dma_ch==GDMA_I2S_TX0)
        {
#if defined(CONFIG_I2S_MMAP)
		dma_sync_single_for_device(NULL,  i2s_mmap_addr[tx_r_idx], I2S_PAGE_SIZE, DMA_TO_DEVICE);
#if defined(ARM_ARCH)
		GdmaI2sTx(i2s_mmap_addr[tx_r_idx], I2S_TX_FIFO_WREG_PHY, 0, I2S_PAGE_SIZE, i2s_dma_tx_handler, i2s_dma_tx_unmask_handler);
#else
                GdmaI2sTx((u32)(pi2s_config->pMMAPTxBufPtr[tx_r_idx]), I2S_TX_FIFO_WREG, 0, I2S_PAGE_SIZE, i2s_dma_tx_handler, i2s_dma_tx_unmask_handler);
#endif
#else
                memcpy(pi2s_config->pPage0TxBuf8ptr,  pi2s_config->pMMAPTxBufPtr[tx_r_idx], I2S_PAGE_SIZE);
#if defined(ARM_ARCH)
		GdmaI2sTx(i2s_txdma_addr0, I2S_TX_FIFO_WREG_PHY, 0, I2S_PAGE_SIZE, i2s_dma_tx_handler, i2s_dma_tx_unmask_handler);
#else
                GdmaI2sTx((u32)(pi2s_config->pPage0TxBuf8ptr), I2S_TX_FIFO_WREG, 0, I2S_PAGE_SIZE, i2s_dma_tx_handler, i2s_dma_tx_unmask_handler);
#endif
#endif
                pi2s_config->dmach = GDMA_I2S_TX0;
                pi2s_config->tx_r_idx = (pi2s_config->tx_r_idx+1)%MAX_I2S_PAGE;
	}
        else
        {
#if defined(CONFIG_I2S_MMAP)
		dma_sync_single_for_device(NULL,  i2s_mmap_addr[tx_r_idx], I2S_PAGE_SIZE, DMA_TO_DEVICE);
#if defined(ARM_ARCH)
		GdmaI2sTx(i2s_mmap_addr[tx_r_idx], I2S_TX_FIFO_WREG_PHY, 1, I2S_PAGE_SIZE, i2s_dma_tx_handler, i2s_dma_tx_unmask_handler);
#else
                GdmaI2sTx((u32)(pi2s_config->pMMAPTxBufPtr[tx_r_idx]), I2S_TX_FIFO_WREG, 1, I2S_PAGE_SIZE, i2s_dma_tx_handler, i2s_dma_tx_unmask_handler);
#endif
#else
                memcpy(pi2s_config->pPage1TxBuf8ptr,  pi2s_config->pMMAPTxBufPtr[tx_r_idx], I2S_PAGE_SIZE);
#if defined(ARM_ARCH)
		GdmaI2sTx(i2s_txdma_addr1, I2S_TX_FIFO_WREG_PHY, 1, I2S_PAGE_SIZE, i2s_dma_tx_handler, i2s_dma_tx_unmask_handler);
#else
                GdmaI2sTx((u32)(pi2s_config->pPage1TxBuf8ptr), I2S_TX_FIFO_WREG, 1, I2S_PAGE_SIZE, i2s_dma_tx_handler, i2s_dma_tx_unmask_handler);
#endif
#endif
                pi2s_config->dmach = GDMA_I2S_TX1;
                pi2s_config->tx_r_idx = (pi2s_config->tx_r_idx+1)%MAX_I2S_PAGE;
	}
#if defined(CONFIG_I2S_WITH_AEC)
	if(aecFuncP->AECFeEnq){
		aecFuncP->AECFeEnq(0,pi2s_config->pMMAPTxBufPtr[pi2s_config->tx_r_idx],I2S_PAGE_SIZE);
	}
#endif
	return 0;
}

int i2s_dma_tx_transf_zero(i2s_config_type* ptri2s_config, u32 dma_ch)
{
	if(dma_ch==GDMA_I2S_TX0)
        {
         	memset(pi2s_config->pPage0TxBuf8ptr, 0, I2S_PAGE_SIZE);
#if defined(ARM_ARCH)
		GdmaI2sTx(i2s_txdma_addr0, I2S_TX_FIFO_WREG_PHY, 0, I2S_PAGE_SIZE, i2s_dma_tx_handler, i2s_dma_tx_unmask_handler);
#else
                GdmaI2sTx((u32)pi2s_config->pPage0TxBuf8ptr, I2S_TX_FIFO_WREG, 0, I2S_PAGE_SIZE, i2s_dma_tx_handler, i2s_dma_tx_unmask_handler);
#endif
        }
        else
        {
                memset(pi2s_config->pPage1TxBuf8ptr, 0, I2S_PAGE_SIZE);
#if defined(ARM_ARCH)
		GdmaI2sTx(i2s_txdma_addr1, I2S_TX_FIFO_WREG_PHY, 1, I2S_PAGE_SIZE, i2s_dma_tx_handler, i2s_dma_tx_unmask_handler);
#else
                GdmaI2sTx((u32)pi2s_config->pPage1TxBuf8ptr, I2S_TX_FIFO_WREG, 1, I2S_PAGE_SIZE, i2s_dma_tx_handler, i2s_dma_tx_unmask_handler);
#endif
        }
	return 0;
}

int i2s_dma_rx_transf_data(i2s_config_type* ptri2s_config, u32 dma_ch)
{
	int rx_w_idx;

	pi2s_config->rx_w_idx = (pi2s_config->rx_w_idx+1)%MAX_I2S_PAGE;

	if ((pi2s_config->bALSAEnable==1) && (pi2s_config->bALSAMMAPEnable==1))
		rx_w_idx = (pi2s_config->rx_w_idx+ALSA_MMAP_IDX_SHIFT)%MAX_I2S_PAGE;
	else
		rx_w_idx = (pi2s_config->rx_w_idx)%MAX_I2S_PAGE;

	if(dma_ch==GDMA_I2S_RX0)
        {
                
#ifdef CONFIG_I2S_MMAP
                dma_sync_single_for_device(NULL,  i2s_mmap_addr[rx_w_idx+(pi2s_config->mmap_index-MAX_I2S_PAGE)], I2S_PAGE_SIZE, DMA_FROM_DEVICE);
#if defined(ARM_ARCH)
		GdmaI2sRx(I2S_RX_FIFO_RREG_PHY, (u32)i2s_mmap_addr[rx_w_idx+(pi2s_config->mmap_index-MAX_I2S_PAGE)], 0, I2S_PAGE_SIZE, i2s_dma_rx_handler, i2s_dma_rx_unmask_handler);
#else
                GdmaI2sRx(I2S_RX_FIFO_RREG, (u32)(pi2s_config->pMMAPRxBufPtr[rx_w_idx]), 0, I2S_PAGE_SIZE, i2s_dma_rx_handler, i2s_dma_rx_unmask_handler);
#endif
#else
                memcpy(pi2s_config->pMMAPRxBufPtr[rx_w_idx], pi2s_config->pPage0RxBuf8ptr, I2S_PAGE_SIZE);
#if defined(ARM_ARCH)
		GdmaI2sRx(I2S_RX_FIFO_RREG_PHY, i2s_rxdma_addr0, 0, I2S_PAGE_SIZE, i2s_dma_rx_handler, i2s_dma_rx_unmask_handler);
#else
                GdmaI2sRx(I2S_RX_FIFO_RREG, (u32)(pi2s_config->pPage0RxBuf8ptr), 0, I2S_PAGE_SIZE, i2s_dma_rx_handler, i2s_dma_rx_unmask_handler);
#endif
#endif
                pi2s_config->dmach = GDMA_I2S_RX0;
        }
	else
        {
                
#ifdef CONFIG_I2S_MMAP
                dma_sync_single_for_device(NULL,  i2s_mmap_addr[rx_w_idx+(pi2s_config->mmap_index-MAX_I2S_PAGE)], I2S_PAGE_SIZE, DMA_FROM_DEVICE);
#if defined(ARM_ARCH)
		GdmaI2sRx(I2S_RX_FIFO_RREG_PHY, (u32)i2s_mmap_addr[rx_w_idx+(pi2s_config->mmap_index-MAX_I2S_PAGE)], 1, I2S_PAGE_SIZE, i2s_dma_rx_handler, i2s_dma_rx_unmask_handler);
#else
                GdmaI2sRx(I2S_RX_FIFO_RREG, (u32)(pi2s_config->pMMAPRxBufPtr[rx_w_idx]), 1, I2S_PAGE_SIZE, i2s_dma_rx_handler, i2s_dma_rx_unmask_handler);
#endif
#else
                memcpy(pi2s_config->pMMAPRxBufPtr[rx_w_idx], pi2s_config->pPage1RxBuf8ptr, I2S_PAGE_SIZE);
#if defined(ARM_ARCH)
		GdmaI2sRx(I2S_RX_FIFO_RREG_PHY, i2s_rxdma_addr1, 1, I2S_PAGE_SIZE, i2s_dma_rx_handler, i2s_dma_rx_unmask_handler);
#else
                GdmaI2sRx(I2S_RX_FIFO_RREG, (u32)(pi2s_config->pPage1RxBuf8ptr), 1, I2S_PAGE_SIZE, i2s_dma_rx_handler, i2s_dma_rx_unmask_handler);
#endif
#endif
                pi2s_config->dmach = GDMA_I2S_RX1;

        }
#if defined(CONFIG_I2S_WITH_AEC)
		if(aecFuncP->AECNeEnq){
			aecFuncP->AECNeEnq(0,pi2s_config->pMMAPRxBufPtr[rx_w_idx],I2S_PAGE_SIZE);
		}
#endif
	return 0;
}

int i2s_dma_rx_transf_zero(i2s_config_type* ptri2s_config, u32 dma_ch)
{
	if(dma_ch==GDMA_I2S_RX0)
        {	
		memset(pi2s_config->pPage0RxBuf8ptr, 0, I2S_PAGE_SIZE);
#if defined(ARM_ARCH)
		GdmaI2sRx(I2S_RX_FIFO_RREG_PHY, i2s_rxdma_addr0, 0, I2S_PAGE_SIZE, i2s_dma_rx_handler, i2s_dma_rx_unmask_handler);
#else
        	GdmaI2sRx(I2S_RX_FIFO_RREG, (u32)pi2s_config->pPage0RxBuf8ptr, 0, I2S_PAGE_SIZE, i2s_dma_rx_handler, i2s_dma_rx_unmask_handler);
#endif
        }
        else
       	{
		memset(pi2s_config->pPage1RxBuf8ptr, 0, I2S_PAGE_SIZE);
#if defined(ARM_ARCH)
		GdmaI2sRx(I2S_RX_FIFO_RREG_PHY, i2s_rxdma_addr1, 1, I2S_PAGE_SIZE, i2s_dma_rx_handler, i2s_dma_rx_unmask_handler);
#else
                GdmaI2sRx(I2S_RX_FIFO_RREG, (u32)pi2s_config->pPage1RxBuf8ptr, 1, I2S_PAGE_SIZE, i2s_dma_rx_handler, i2s_dma_rx_unmask_handler);
#endif
        }
	return 0;
}

void i2s_dma_tx_handler(u32 dma_ch)
{
	pi2s_config->enLable = 1; /* TX:enLabel=1; RX:enLabel=2 */

	if(pi2s_config->bTxDMAEnable==0) 
	{
		if(pi2s_config->end_cnt != 0)
		{
			i2s_dma_tx_transf_data(pi2s_config, dma_ch);
			pi2s_config->end_cnt --;
	        	MSG("end_cnt = %d, r_idx = %d\n", pi2s_config->end_cnt, pi2s_config->tx_r_idx);
		}
		else
		{
			pi2s_config->tx_stop_cnt++;
			i2s_dma_tx_soft_stop(pi2s_config, dma_ch);
			MSG("tx_stop=%d, ch=%d\n", pi2s_config->tx_stop_cnt, dma_ch);
			if (pi2s_config->tx_stop_cnt == 3)
			{
                        	wake_up_interruptible(&(pi2s_config->i2s_tx_qh));
				printk("T:wake up!!\n");
			}
		}
		return;
	}
	
	pi2s_config->tx_isr_cnt++;

#ifdef 	I2S_STATISTIC
	i2s_int_status(dma_ch);
#endif
	/* FIXME */
	if(pi2s_config->bALSAEnable)
	{
		if(pi2s_config->dmaStat[STREAM_PLAYBACK])
		{
			if(!pi2s_config->bTrigger[STREAM_PLAYBACK]){
				//printk("trigger stop: rIdx:%d widx:%d\n", pi2s_config->tx_r_idx,pi2s_config->tx_w_idx);
                                i2s_dma_tx_transf_zero(pi2s_config, dma_ch);
                                if(pi2s_config->bPreTrigger[STREAM_PLAYBACK]){
                                        /* mtk04880 commented:
                                         * for corner case, there are cases which ALSA Trigger stop before disabling DMA.
                                         * For which case, it needs to keep call snd_pcm_elapased to keep ALSA hw ptr updating.
                                         * It is so called post stop handlment.
                                         */
                                        //printk("post-stop\n");
                                        goto EXIT;
                                }
                                else{
                                        //printk("pre-stop\n");
                                        wake_up_interruptible(&(pi2s_config->i2s_tx_qh));
                                        return;
                                }
                        }
                        else{
                                if(!pi2s_config->bPreTrigger[STREAM_PLAYBACK])
                                        pi2s_config->bPreTrigger[STREAM_PLAYBACK] = 1;

			}
		}	
	}
	else
	{
		if(pi2s_config->tx_r_idx==pi2s_config->tx_w_idx)
		{
			/* Buffer Empty */
			MSG("TXBE r=%d w=%d[i=%u,c=%u]\n",pi2s_config->tx_r_idx,pi2s_config->tx_w_idx,pi2s_config->tx_isr_cnt,dma_ch);
#ifdef I2S_STATISTIC		
			pi2s_status->txbuffer_unrun++;
#endif	
			i2s_dma_tx_transf_zero(pi2s_config, dma_ch);
			goto EXIT;	
		}
	}

	if(pi2s_config->pMMAPTxBufPtr[pi2s_config->tx_r_idx]==NULL)
	{
		MSG("mmap buf NULL [%d]\n",pi2s_config->tx_r_idx);
		i2s_dma_tx_transf_zero(pi2s_config, dma_ch);

		goto EXIT;	
	}

	if(pi2s_config->tx_pause_en == 1)
	{
		/* Enable PAUSE */
		MSG("TX pause now\n");
		i2s_dma_tx_transf_zero(pi2s_config, dma_ch);

		goto EXIT;	
	}

#ifdef I2S_STATISTIC	
	pi2s_status->txbuffer_len--;
#endif
	i2s_dma_tx_transf_data(pi2s_config, dma_ch);

EXIT:
#if defined(CONFIG_SND_MT76XX_SOC)
	if(pi2s_config->bALSAEnable == 1){
		if(pi2s_config->pss[STREAM_PLAYBACK])
			snd_pcm_period_elapsed(pi2s_config->pss[STREAM_PLAYBACK]);
	}
#endif
	wake_up_interruptible(&(pi2s_config->i2s_tx_qh));		
	return;
}

void i2s_dma_rx_handler(u32 dma_ch)
{
	pi2s_config->enLable = 2; /* TX:enLabel=1; RX:enLabel=2 */
#if defined(CONFIG_I2S_TXRX)
	if(pi2s_config->rx_isr_cnt==0)
	{
		pi2s_config->next_p0_idx = 0;
		pi2s_config->next_p1_idx = 1;
	}	
	pi2s_config->rx_isr_cnt++;
	
#ifdef  I2S_STATISTIC
	i2s_int_status(dma_ch);
#endif

	if (pi2s_config->bRxDMAEnable==0)
	{
		pi2s_config->rx_stop_cnt++;
		i2s_dma_rx_soft_stop(pi2s_config, dma_ch);
		MSG("rx_stop=%d\n", pi2s_config->rx_stop_cnt);

		if(pi2s_config->rx_stop_cnt == 2)
		{
			wake_up_interruptible(&(pi2s_config->i2s_rx_qh));
			printk("R:wake up!!\n");
		}
		return;	
	}

	if(pi2s_config->bALSAEnable)
	{
		 if(pi2s_config->dmaStat[STREAM_CAPTURE]){
			if(!pi2s_config->bTrigger[STREAM_CAPTURE]){
                                MSG("trigger stop: rIdx:%d widx:%d\n", pi2s_config->rx_r_idx,pi2s_config->rx_w_idx);
				i2s_dma_rx_transf_zero(pi2s_config, dma_ch);
                                wake_up_interruptible(&(pi2s_config->i2s_rx_qh));
                                return;
			}
		 }
	}
	else
	{
		if(((pi2s_config->rx_w_idx+1)%MAX_I2S_PAGE)==pi2s_config->rx_r_idx){
			/* Buffer Full */
			MSG("RXBF r=%d w=%d[i=%u,c=%u]\n",pi2s_config->rx_r_idx,pi2s_config->rx_w_idx,pi2s_config->rx_isr_cnt,dma_ch);
#ifdef I2S_STATISTIC		
			pi2s_status->rxbuffer_unrun++;
#endif	
			i2s_dma_rx_transf_zero(pi2s_config, dma_ch);
			goto EXIT;	
		}
	}

	if(pi2s_config->rx_pause_en == 1)
	{
		/* Enable PAUSE */
		i2s_dma_rx_transf_zero(pi2s_config, dma_ch);

		goto EXIT;	
	}

#ifdef I2S_STATISTIC	
	pi2s_status->rxbuffer_len++;
#endif
	i2s_dma_rx_transf_data(pi2s_config, dma_ch);

EXIT:
#if defined(CONFIG_SND_MT76XX_SOC)
	if(pi2s_config->bALSAEnable == 1){
		if(pi2s_config->pss[STREAM_CAPTURE])
			snd_pcm_period_elapsed(pi2s_config->pss[STREAM_CAPTURE]);
	}
#endif
	wake_up_interruptible(&(pi2s_config->i2s_rx_qh));
#endif	
	return;
}

#ifdef I2S_STATISTIC
void i2s_int_status(u32 dma_ch)
{
	u32 i2s_status;
	
	if((pi2s_config->tx_isr_cnt>0)||(pi2s_config->rx_isr_cnt>0))
	{
		i2s_status = i2s_inw(I2S_INT_STATUS);
		
		if(i2s_status&REGBIT(1, I2S_TX_DMA_FAULT))
		{
			pi2s_status->txdmafault++;
		}
		if(i2s_status&REGBIT(1, I2S_TX_OVRUN))
		{
			pi2s_status->txovrun++;
		}
		if(i2s_status&REGBIT(1, I2S_TX_UNRUN))
		{
			pi2s_status->txunrun++;
		}
		if(i2s_status&REGBIT(1, I2S_TX_THRES))
		{
			pi2s_status->txthres++;
		}
		if(i2s_status&REGBIT(1, I2S_RX_DMA_FAULT))
		{
			pi2s_status->rxdmafault++;
		}
		if(i2s_status&REGBIT(1, I2S_RX_OVRUN))
		{
			pi2s_status->rxovrun++;
		}
		if(i2s_status&REGBIT(1, I2S_RX_UNRUN))
		{
			pi2s_status->rxunrun++;
		}
		if(i2s_status&REGBIT(1, I2S_RX_THRES))
		{
			pi2s_status->rxthres++;
		}
	}
#if 0
	if(pi2s_config->enLable == 1)
	{
		if((pi2s_config->tx_isr_cnt>0) && (pi2s_config->tx_isr_cnt%40==0))
		{
			MSG("tisr i=%u,ch=%u,o=%u,u=%d,s=%X [r=%d,w=%d]\n",\
				pi2s_config->tx_isr_cnt,dma_ch,pi2s_status->txovrun,pi2s_status->txunrun,\
				i2s_inw(I2S_INT_STATUS),pi2s_config->tx_r_idx,pi2s_config->tx_w_idx);
		}
	}
	
	if(pi2s_config->enLable == 2)
	{
		if((pi2s_config->rx_isr_cnt>0) && (pi2s_config->rx_isr_cnt%40==0))
		{
			MSG("risr i=%u,ch=%u,o=%u,u=%d,s=%X [r=%d,w=%d]\n",\
				pi2s_config->rx_isr_cnt,dma_ch,pi2s_status->rxovrun,pi2s_status->rxunrun,\
				i2s_inw(I2S_INT_STATUS),pi2s_config->rx_r_idx,pi2s_config->rx_w_idx);
		}
	}
#endif
	
	*(unsigned long*)(I2S_INT_STATUS) = 0xFFFFFFFF;
}
#endif

#if defined(I2S_HW_INTERRUPT_EN)&&(I2S_SW_IRQ_EN)
irqreturn_t i2s_irq_isr(int irq, void *irqaction)
{
	u32 i2s_status;
	
	//MSG("i2s_irq_isr [0x%08X]\n",i2s_inw(I2S_INT_STATUS));
	if((pi2s_config->tx_isr_cnt>0)||(pi2s_config->rx_isr_cnt>0))
	{
		i2s_status = i2s_inw(I2S_INT_STATUS);
		MSG("i2s_irq_isr [0x%08X]\n",i2s_status);
	}
	else
		return IRQ_HANDLED;
		
	if(i2s_status&REGBIT(1, I2S_TX_DMA_FAULT))
	{
#ifdef I2S_STATISTIC
		pi2s_status->txdmafault++;
#endif
	}
	if(i2s_status&REGBIT(1, I2S_TX_OVRUN))
	{
#ifdef I2S_STATISTIC
		pi2s_status->txovrun++;
#endif
	}
	if(i2s_status&REGBIT(1, I2S_TX_UNRUN))
	{
#ifdef I2S_STATISTIC
		pi2s_status->txunrun++;
#endif
	}
	if(i2s_status&REGBIT(1, I2S_TX_THRES))
	{
#ifdef I2S_STATISTIC
		pi2s_status->txthres++;
#endif
	}
	if(i2s_status&REGBIT(1, I2S_RX_DMA_FAULT))
	{
#ifdef I2S_STATISTIC
		pi2s_status->rxdmafault++;
#endif
	}
	if(i2s_status&REGBIT(1, I2S_RX_OVRUN))
	{
#ifdef I2S_STATISTIC
		pi2s_status->rxovrun++;
#endif
	}
	if(i2s_status&REGBIT(1, I2S_RX_UNRUN))
	{
#ifdef I2S_STATISTIC
		pi2s_status->rxunrun++;
#endif
	}
	if(i2s_status&REGBIT(1, I2S_RX_THRES))
	{
#ifdef I2S_STATISTIC
		pi2s_status->rxthres++;
#endif
	}
	i2s_outw(I2S_INT_STATUS, 0xFFFFFFFF);
	return IRQ_HANDLED;
}
#endif

void i2s_tx_task(unsigned long pData)
{
	unsigned long flags;
	spin_lock_irqsave(&pi2s_config->lock, flags);
	//if (pi2s_config->bTxDMAEnable!=0)
	{	
		if (pi2s_config->tx_unmask_ch!=0)
		{
			u32 dmach = pi2s_config->tx_unmask_ch;
			u32 ch;
			for (ch = 0; ch < 16; ch++)
			{
				if (dmach& (1<<ch))
				{
					MSG("do unmask ch%d tisr=%d in tx_isr\n",ch,pi2s_config->tx_isr_cnt);
					GdmaUnMaskChannel(ch);
				}	
			}
			pi2s_config->tx_unmask_ch = 0;	
		}
	}	
	spin_unlock_irqrestore(&pi2s_config->lock, flags);
}

void i2s_rx_task(unsigned long pData)
{
	unsigned long flags;
	spin_lock_irqsave(&pi2s_config->lock, flags);
	//if (pi2s_config->bRxDMAEnable!=0)
	{	
		if (pi2s_config->rx_unmask_ch!=0)
		{
			u32 dmach = pi2s_config->rx_unmask_ch;
			u32 ch;
			for (ch = 0; ch < 16; ch++)
			{
				if (dmach& (1<<ch))
				{
					MSG("do unmask ch%d risr=%d in rx_isr\n",ch,pi2s_config->rx_isr_cnt);
					GdmaUnMaskChannel(ch);
				}	
			}
			pi2s_config->rx_unmask_ch = 0;	
	
		}
	}	
	spin_unlock_irqrestore(&pi2s_config->lock, flags);
}


void i2s_dma_unmask_handler(u32 dma_ch)
{
	MSG("i2s_dma_unmask_handler ch=%d\n",dma_ch);
	
	GdmaUnMaskChannel(dma_ch);

	return;
}

void i2s_dma_tx_unmask_handler(u32 dma_ch)
{
	MSG("i2s_dma_tx_unmask_handler ch=%d\n",dma_ch);
	pi2s_config->tx_unmask_ch |= (1<<dma_ch);
	tasklet_hi_schedule(&i2s_tx_tasklet);
	return;
}

void i2s_dma_rx_unmask_handler(u32 dma_ch)
{
	MSG("i2s_dma_rx_unmask_handler ch=%d\n",dma_ch);
	pi2s_config->rx_unmask_ch |= (1<<dma_ch);
	tasklet_hi_schedule(&i2s_rx_tasklet);
	return;
}

void i2s_dma_mask_handler(u32 dma_ch)
{
        MSG("i2s_dma_mask_handler ch=%d\n", dma_ch);
        GdmaMaskChannel(dma_ch);
        return;
}

void i2s_dma_tx_init(i2s_config_type* ptri2s_config)
{
	memset(pi2s_config->pPage0TxBuf8ptr, 0, I2S_PAGE_SIZE);
	memset(pi2s_config->pPage1TxBuf8ptr, 0, I2S_PAGE_SIZE);
#if defined(ARM_ARCH)
	GdmaI2sTx(i2s_txdma_addr0, I2S_TX_FIFO_WREG_PHY, 0, I2S_PAGE_SIZE, i2s_dma_tx_handler, i2s_dma_tx_unmask_handler);
	GdmaI2sTx(i2s_txdma_addr1, I2S_TX_FIFO_WREG_PHY, 1, I2S_PAGE_SIZE, i2s_dma_tx_handler, i2s_dma_tx_unmask_handler);
#else
	GdmaI2sTx((u32)ptri2s_config->pPage0TxBuf8ptr, I2S_FIFO_WREG, 0, I2S_PAGE_SIZE, i2s_dma_tx_handler, i2s_dma_tx_unmask_handler);
	GdmaI2sTx((u32)ptri2s_config->pPage1TxBuf8ptr, I2S_FIFO_WREG, 1, I2S_PAGE_SIZE, i2s_dma_tx_handler, i2s_dma_tx_unmask_handler);
#endif

	return;
}

void i2s_dma_rx_init(i2s_config_type* ptri2s_config)
{
	memset(pi2s_config->pPage0RxBuf8ptr, 0, I2S_PAGE_SIZE);
	memset(pi2s_config->pPage1RxBuf8ptr, 0, I2S_PAGE_SIZE);

#if defined(ARM_ARCH)
	GdmaI2sRx(I2S_RX_FIFO_RREG_PHY, i2s_rxdma_addr0, 0, I2S_PAGE_SIZE, i2s_dma_rx_handler, i2s_dma_rx_unmask_handler);
	GdmaI2sRx(I2S_RX_FIFO_RREG_PHY, i2s_rxdma_addr1, 1, I2S_PAGE_SIZE, i2s_dma_rx_handler, i2s_dma_rx_unmask_handler);
#else
	GdmaI2sRx(I2S_RX_FIFO_RREG, (u32)ptri2s_config->pPage0RxBuf8ptr, 0, I2S_PAGE_SIZE, i2s_dma_rx_handler, i2s_dma_rx_unmask_handler);
	GdmaI2sRx(I2S_RX_FIFO_RREG, (u32)ptri2s_config->pPage1RxBuf8ptr, 1, I2S_PAGE_SIZE, i2s_dma_rx_handler, i2s_dma_rx_unmask_handler);
#endif

	return;
}

void i2s_dma_tx_end_handle(i2s_config_type* ptri2s_config)
{
	if (ptri2s_config->tx_w_idx < ptri2s_config->tx_r_idx)
        {
        	ptri2s_config->end_cnt = (ptri2s_config->tx_w_idx + MAX_I2S_PAGE)-ptri2s_config->tx_r_idx;
                printk("case1: w=%d, r=%d, end=%d\n", ptri2s_config->tx_w_idx, ptri2s_config->tx_r_idx, ptri2s_config->end_cnt);
        }
        else if (ptri2s_config->tx_w_idx > ptri2s_config->tx_r_idx)
        {
                ptri2s_config->end_cnt = ptri2s_config->tx_w_idx-ptri2s_config->tx_r_idx;
                printk("case2: w=%d, r=%d, end=%d\n", ptri2s_config->tx_w_idx, ptri2s_config->tx_r_idx, ptri2s_config->end_cnt);
        }
	else
	{
		printk("case3: w=%d, r=%d, end=%d\n", ptri2s_config->tx_w_idx, ptri2s_config->tx_r_idx, ptri2s_config->end_cnt);
		
	}

	if (ptri2s_config->end_cnt > 0)
	{
		interruptible_sleep_on(&(ptri2s_config->i2s_tx_qh));
	}

	return;
}

void i2s_tx_end_sleep_on(i2s_config_type* ptri2s_config)
{
	while(ptri2s_config->tx_stop_cnt<3)
		interruptible_sleep_on(&(ptri2s_config->i2s_tx_qh));
	
	return;
}

void i2s_rx_end_sleep_on(i2s_config_type* ptri2s_config)
{
	while(ptri2s_config->rx_stop_cnt<2)
		interruptible_sleep_on(&(ptri2s_config->i2s_rx_qh));
	return;
}

int i2s_dma_tx_soft_stop(i2s_config_type* ptri2s_config, u32 dma_ch)
{
	if(dma_ch==GDMA_I2S_TX0)
        {
#if defined(ARM_ARCH)
		GdmaI2sTx(i2s_txdma_addr0, I2S_TX_FIFO_WREG_PHY, 0, 4, i2s_dma_tx_handler, i2s_dma_tx_unmask_handler);
#else
		GdmaI2sTx((u32)pi2s_config->pPage0TxBuf8ptr, I2S_TX_FIFO_WREG, 0, 4, i2s_dma_tx_handler, i2s_dma_tx_unmask_handler);
#endif
        }
        else
        {
#if defined(ARM_ARCH)
		GdmaI2sTx(i2s_txdma_addr1, I2S_TX_FIFO_WREG_PHY, 1, 4, i2s_dma_tx_handler, i2s_dma_tx_unmask_handler);
#else
                GdmaI2sTx((u32)pi2s_config->pPage1TxBuf8ptr, I2S_TX_FIFO_WREG, 1, 4, i2s_dma_tx_handler, i2s_dma_tx_unmask_handler);
#endif
        }

	return 0;
}

int i2s_dma_rx_soft_stop(i2s_config_type* ptri2s_config, u32 dma_ch)
{
	if(dma_ch==GDMA_I2S_RX0)
        {
		memset(pi2s_config->pPage0RxBuf8ptr, 0, I2S_PAGE_SIZE);
#if defined(ARM_ARCH)
		GdmaI2sRx(I2S_RX_FIFO_RREG_PHY, i2s_rxdma_addr0, 0, 4, i2s_dma_rx_handler, i2s_dma_rx_unmask_handler);
#else
		GdmaI2sRx(I2S_RX_FIFO_RREG, (u32)pi2s_config->pPage0RxBuf8ptr, 0, 4, i2s_dma_rx_handler, i2s_dma_rx_unmask_handler);
#endif
        }
        else
        {
		memset(pi2s_config->pPage1RxBuf8ptr, 0, I2S_PAGE_SIZE);
#if defined(ARM_ARCH)
		GdmaI2sRx(I2S_RX_FIFO_RREG_PHY, i2s_rxdma_addr1, 1, 4, i2s_dma_rx_handler, i2s_dma_rx_unmask_handler);
#else
                GdmaI2sRx(I2S_RX_FIFO_RREG, (u32)pi2s_config->pPage1RxBuf8ptr, 1, 4, i2s_dma_rx_handler, i2s_dma_rx_unmask_handler);
#endif
        }

	return 0;
}

void i2s_gen_test_pattern(void)
{
	int i;
	for (i=0; i<I2S_PAGE_SIZE; i++)
	{
		test_buf[i] = 0x5A;
		test_buf_1[i] = 0x11;
		test_buf_2[i] = 0x22;

	}
}

int i2s_put_audio(i2s_config_type* ptri2s_config, unsigned long arg)
{
	unsigned long flags;
	int tx_w_idx;

	do{
		spin_lock_irqsave(&ptri2s_config->lock, flags);

		if(((ptri2s_config->tx_w_idx+4)%MAX_I2S_PAGE)!=ptri2s_config->tx_r_idx)
		{
			ptri2s_config->tx_w_idx = (ptri2s_config->tx_w_idx+1)%MAX_I2S_PAGE;	
			tx_w_idx = ptri2s_config->tx_w_idx;
			spin_unlock_irqrestore(&ptri2s_config->lock, flags);
			//printk("put TB[%d] for user write\n",ptri2s_config->tx_w_idx);
#if defined(CONFIG_I2S_MMAP)
			put_user(tx_w_idx, (int*)arg);
#else
			copy_from_user(ptri2s_config->pMMAPTxBufPtr[tx_w_idx], (char*)arg, I2S_PAGE_SIZE);
#endif
			pi2s_status->txbuffer_len++;
			//spin_unlock_irqrestore(&ptri2s_config->lock, flags);
			break;
		}
		else
		{
			/* Buffer Full */
			//printk("TBF tr=%d, tw=%d\n", ptri2s_config->tx_r_idx, ptri2s_config->tx_w_idx);
			pi2s_status->txbuffer_ovrun++;
			spin_unlock_irqrestore(&ptri2s_config->lock, flags);
			interruptible_sleep_on(&(ptri2s_config->i2s_tx_qh));
			if (ptri2s_config->bTxDMAEnable==0 && ptri2s_config->end_cnt==0)
			{
				printk("wake up for exit i2s driver\n");
				put_user(-1, (int*)arg);
				break;
			}
		}
	}while(1);

	return 0;
}

int i2s_get_audio(i2s_config_type* ptri2s_config, unsigned long arg)
{
	unsigned long flags;
	int rx_r_idx;

	do{
		spin_lock_irqsave(&ptri2s_config->lock, flags);
		//printk("GA rr=%d, rw=%d,i=%d\n", ptri2s_config->rx_r_idx, ptri2s_config->rx_w_idx,ptri2s_config->rx_isr_cnt);
		if(((ptri2s_config->rx_r_idx+2)%MAX_I2S_PAGE)!=ptri2s_config->rx_w_idx)
		{			
			rx_r_idx = ptri2s_config->rx_r_idx;
			ptri2s_config->rx_r_idx = (ptri2s_config->rx_r_idx+1)%MAX_I2S_PAGE;
			spin_unlock_irqrestore(&ptri2s_config->lock, flags);
#if defined(CONFIG_I2S_MMAP)
			put_user(rx_r_idx, (int*)arg);
#else
			copy_to_user((char*)arg, ptri2s_config->pMMAPRxBufPtr[rx_r_idx], I2S_PAGE_SIZE);
#endif
			//printk("rx_r_idx=%d\n", ptri2s_config->rx_r_idx);
			//ptri2s_config->rx_r_idx = (ptri2s_config->rx_r_idx+1)%MAX_I2S_PAGE;
			pi2s_status->rxbuffer_len--;
			//spin_unlock_irqrestore(&ptri2s_config->lock, flags);
			break;
		}
		else
		{
			/* Buffer Full */
			//printk("RBF rr=%d, rw=%d\n", ptri2s_config->rx_r_idx, ptri2s_config->rx_w_idx);
			pi2s_status->rxbuffer_ovrun++;
			spin_unlock_irqrestore(&ptri2s_config->lock, flags);
			interruptible_sleep_on(&(ptri2s_config->i2s_rx_qh));
		}
#if defined(CONFIG_I2S_WITH_AEC)
		if(aecFuncP->AECECDeq){
			aecFuncP->AECECDeq(0,pi2s_config->pMMAPRxBufPtr[ptri2s_config->rx_r_idx],I2S_PAGE_SIZE);
		}
#endif
	}while(1);

	return 0;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
long i2s_ioctl (struct file *filp, unsigned int cmd, unsigned long arg)
#else
int i2s_ioctl (struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#endif
{
	int i ;
	i2s_config_type* ptri2s_config;
	unsigned long flags;
	    
	ptri2s_config = filp->private_data;
	switch (cmd) {
	case I2S_RESET:
		spin_lock_irqsave(&ptri2s_config->lock, flags);
		i2s_reset_config(ptri2s_config);
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);
		break;
	case I2S_SRATE:
		spin_lock_irqsave(&ptri2s_config->lock, flags);
#if defined(CONFIG_I2S_WM8960)
		if((arg>MAX_SRATE_HZ)||(arg<MIN_SRATE_HZ))
		{
			MSG("Audio sampling rate %u should be %d ~ %d Hz. Set SRate to 48000Hz\n", (u32)arg, MIN_SRATE_HZ, MAX_SRATE_HZ);
			ptri2s_config->srate = 48000;
			spin_unlock(&ptri2s_config->lock);
			break;
		}	
#elif defined(CONFIG_I2S_WM8750)
		if((arg>MAX_SRATE_HZ)||(arg<MIN_SRATE_HZ))
		{
			MSG("Audio sampling rate %u should be %d ~ %d Hz. Set SRate to 96000Hz\n", (u32)arg, MIN_SRATE_HZ, MAX_SRATE_HZ);
			ptri2s_config->srate = 96000;
			spin_unlock(&ptri2s_config->lock);
			break;
		}
#endif
		ptri2s_config->srate = arg;
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);
		MSG("set audio sampling rate to %d Hz\n", ptri2s_config->srate);
		break;
	case I2S_TX_VOL:
		spin_lock_irqsave(&ptri2s_config->lock, flags);
		
		if((int)arg > 127)
			ptri2s_config->txvol = 127;
		else if((int)arg < 48)
			ptri2s_config->txvol = 48;
		else
			ptri2s_config->txvol = arg;
		
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);
		
		spin_lock_irqsave(&ptri2s_config->lock, flags);
#if (defined(CONFIG_I2S_WM8750) || defined(CONFIG_I2S_WM8751))
		audiohw_set_master_vol(arg,arg);
#elif defined(CONFIG_I2S_WM8960)
		audiohw_set_lineout_vol(1, ptri2s_config->txvol, ptri2s_config->txvol);
#endif
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);
		break;
	case I2S_RX_VOL:
		spin_lock_irqsave(&ptri2s_config->lock, flags);
		
		if((int)arg > 63)
			ptri2s_config->rxvol = 63;
		else if((int)arg < 0)
			ptri2s_config->rxvol = 0;
		else
			ptri2s_config->rxvol = arg;
		
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);
		break;
#if defined (CONFIG_RALINK_MT7628) || defined(CONFIG_ARCH_MT7623)
	case I2S_WORD_LEN:
		spin_lock_irqsave(&ptri2s_config->lock, flags);
		if((int)arg == 16)
		{
			ptri2s_config->wordlen_24b = 0;
			MSG("Enable 16 bit word length.\n");
		}
		else if ((int)arg == 24)
		{
			ptri2s_config->wordlen_24b = 1;
			MSG("Enable 24 bit word length.\n");
		}
		else
		{
			MSG("MT7628 only support 16bit/24bit word length.\n");
			spin_unlock(&ptri2s_config->lock);
			break;
		}
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);
		break;
	case I2S_ENDIAN_FMT:
		spin_lock_irqsave(&ptri2s_config->lock, flags);
		if((int)arg == 1)
		{
			ptri2s_config->little_edn = 1;
			MSG("Little endian format.\n");
		}
		else 
		{
			ptri2s_config->little_edn = 0;
			MSG("Big endian format.\n");
		}
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);
		break;	
#endif
	case I2S_INTERNAL_LBK:
		spin_lock_irqsave(&ptri2s_config->lock, flags);
		if((int)arg == 1)
		{
			ptri2s_config->lbk = 1;
			MSG("Enable internal loopback.\n");
		}
		else 
		{
			ptri2s_config->lbk = 0;
			MSG("Disable internal loopback.\n");
		}
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);
		break;
	case I2S_EXTERNAL_LBK:
		spin_lock_irqsave(&ptri2s_config->lock, flags);
		if((int)arg == 1)
		{
			ptri2s_config->extlbk = 1;
			MSG("Enable external loopback.\n");
		}
		else 
		{
			ptri2s_config->extlbk = 0;
			MSG("Disable external loopback.\n");
		}
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);
		break;
	case I2S_TXRX_COEXIST:
		spin_lock_irqsave(&ptri2s_config->lock, flags);
		if((int)arg == 1)
		{
			ptri2s_config->txrx_coexist = 1;
			MSG("TX/RX coexist.\n");
		}
		else 
		{
			ptri2s_config->txrx_coexist = 0;
			MSG("TX/RX coexist.\n");
		}
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);
		break;

	case I2S_TX_ENABLE:
		spin_lock_irqsave(&ptri2s_config->lock, flags);
		MSG("I2S_TXENABLE\n");

		pi2s_config->tx_unmask_ch = 0;
		tasklet_init(&i2s_tx_tasklet, i2s_tx_task, (u32)pi2s_config);

		pi2s_config->dis_match = 0;
		pi2s_config->start_cnt = 0;
		i2s_gen_test_pattern();

		/* allocate tx buffer */
		i2s_txPagebuf_alloc(ptri2s_config);
		i2s_txbuf_alloc(ptri2s_config);
	
		/* Init two dma channels */
		i2s_dma_tx_init(ptri2s_config);
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);

		spin_lock_irqsave(&ptri2s_config->lock, flags);
		/* Init & config all tx param */
		i2s_reset_tx_param(ptri2s_config);
		ptri2s_config->bTxDMAEnable = 1;
		/* Clear all ALSA related config */
		ptri2s_config->bALSAEnable = 0;
		ptri2s_config->bALSAMMAPEnable = 0;

		i2s_tx_config(ptri2s_config);
	
		if(ptri2s_config->bRxDMAEnable==0)
			i2s_clock_enable(ptri2s_config);
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);
	
		spin_lock_irqsave(&ptri2s_config->lock, flags);
#if defined(CONFIG_I2S_WM8960)||defined(CONFIG_I2S_WM8750)||defined(CONFIG_I2S_WM8751)
		audiohw_set_lineout_vol(1, ptri2s_config->txvol, ptri2s_config->txvol);
#endif
		GdmaUnMaskChannel(GDMA_I2S_TX0);

		i2s_tx_enable(ptri2s_config);
	
		/* Kick off dma channel */	
		//GdmaUnMaskChannel(GDMA_I2S_TX0);

		MSG("I2S_TXENABLE done\n");
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);
		break;
	case I2S_TX_DISABLE:
		spin_lock_irqsave(&ptri2s_config->lock, flags);
		MSG("I2S_TXDISABLE\n");

		//tasklet_kill(&i2s_tx_tasklet);

		/* Handle tx end data */
		ptri2s_config->bTxDMAEnable = 0;
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);

		i2s_tx_end_sleep_on(ptri2s_config);
	
		tasklet_kill(&i2s_tx_tasklet);

		spin_lock_irqsave(&ptri2s_config->lock, flags);
		i2s_reset_tx_param(ptri2s_config);
		i2s_tx_disable(ptri2s_config);
		if((ptri2s_config->bRxDMAEnable==0)&&(ptri2s_config->bTxDMAEnable==0))
			i2s_clock_disable(ptri2s_config);
	
		i2s_txbuf_free(ptri2s_config);		
		if(ptri2s_config->mmap_index <= MAX_I2S_PAGE)
			ptri2s_config->mmap_index = 0;
		
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);
		break;
	case I2S_RX_ENABLE:
		spin_lock_irqsave(&ptri2s_config->lock, flags);
		MSG("I2S_RXENABLE\n");
		pi2s_config->rx_unmask_ch = 0;
		tasklet_init(&i2s_rx_tasklet, i2s_rx_task, (u32)pi2s_config);
		
		/* allocate rx buffer */
		i2s_rxPagebuf_alloc(ptri2s_config);
		i2s_rxbuf_alloc(ptri2s_config);	

		/* Init two dma channels */
		i2s_dma_rx_init(ptri2s_config);	
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);

		spin_lock_irqsave(&ptri2s_config->lock, flags);
		/* Init & config all rx param */
		i2s_reset_rx_param(ptri2s_config);
		ptri2s_config->bRxDMAEnable = 1;
		ptri2s_config->bALSAEnable = 0;
		ptri2s_config->bALSAMMAPEnable = 0;

		i2s_rx_config(ptri2s_config);

		if(ptri2s_config->bTxDMAEnable==0)
			i2s_clock_enable(ptri2s_config);

#if defined(CONFIG_I2S_TXRX)
#if defined(CONFIG_I2S_WM8960)||defined(CONFIG_I2S_WM8750)||defined(CONFIG_I2S_WM8751)
		audiohw_set_linein_vol(ptri2s_config->rxvol,  ptri2s_config->rxvol);
#endif
#endif
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);

		spin_lock_irqsave(&ptri2s_config->lock, flags);
		/* Kick off dma channel */
		GdmaUnMaskChannel(GDMA_I2S_RX0);

		i2s_rx_enable(ptri2s_config);
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);
		break;
	case I2S_RX_DISABLE:
		spin_lock_irqsave(&ptri2s_config->lock, flags);
		MSG("I2S_RXDISABLE\n");
		//tasklet_kill(&i2s_rx_tasklet);

		ptri2s_config->bRxDMAEnable = 0;
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);

		i2s_rx_end_sleep_on(ptri2s_config);		
		tasklet_kill(&i2s_rx_tasklet);

		spin_lock_irqsave(&ptri2s_config->lock, flags);
		i2s_reset_rx_param(ptri2s_config);
		i2s_rx_disable(ptri2s_config);
		if((ptri2s_config->bRxDMAEnable==0)&&(ptri2s_config->bTxDMAEnable==0))
			i2s_clock_disable(ptri2s_config);
		
		i2s_rxbuf_free(ptri2s_config);
		if(ptri2s_config->mmap_index <= MAX_I2S_PAGE)
			ptri2s_config->mmap_index = 0;	
		//i2s_rxPagebuf_free(ptri2s_config);
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);
		break;
	case I2S_PUT_AUDIO:
		i2s_put_audio(ptri2s_config, arg);		
		break;
	case I2S_GET_AUDIO:
		i2s_get_audio(ptri2s_config, arg);
		break;
	case I2S_TX_STOP:
		spin_lock_irqsave(&ptri2s_config->lock, flags);
		MSG("TxGDMA STOP\n");
		ptri2s_config->bTxDMAEnable = 0;
		ptri2s_config->end_cnt = 0;
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);

		while(ptri2s_config->tx_stop_cnt<3)
                        interruptible_sleep_on(&(ptri2s_config->i2s_tx_qh));

		spin_lock_irqsave(&ptri2s_config->lock, flags);
		i2s_reset_tx_param(ptri2s_config);
		i2s_tx_disable(ptri2s_config);
		if((ptri2s_config->bRxDMAEnable==0)&&(ptri2s_config->bTxDMAEnable==0))
			i2s_clock_disable(ptri2s_config);
		
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);

		spin_lock_irqsave(&ptri2s_config->lock, flags);
		i2s_txbuf_free(ptri2s_config);		
		if(ptri2s_config->mmap_index <= MAX_I2S_PAGE)
			ptri2s_config->mmap_index = 0;
		//i2s_txPagebuf_free(ptri2s_config);
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);
		break;
	case I2S_TX_PAUSE:
		spin_lock_irqsave(&ptri2s_config->lock, flags);
                ptri2s_config->tx_pause_en = 1;
		MSG("* tx_pause_en = 1 *\n");
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);
                break;
	case I2S_TX_RESUME:
		spin_lock_irqsave(&ptri2s_config->lock, flags);
                ptri2s_config->tx_pause_en = 0;
		MSG("# tx_pause_en = 0 #\n");
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);
                break;
	case I2S_RX_STOP:
		spin_lock_irqsave(&ptri2s_config->lock, flags);
		MSG("I2S_RX_STOP\n");
		ptri2s_config->bRxDMAEnable = 0;
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);

		while(ptri2s_config->rx_stop_cnt<2)
                        interruptible_sleep_on(&(ptri2s_config->i2s_rx_qh));

		spin_lock_irqsave(&ptri2s_config->lock, flags);
		i2s_reset_rx_param(ptri2s_config);
		i2s_rx_disable(ptri2s_config);
		if((ptri2s_config->bRxDMAEnable==0)&&(ptri2s_config->bTxDMAEnable==0))
			i2s_clock_disable(ptri2s_config);
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);

		spin_lock_irqsave(&ptri2s_config->lock, flags);
		i2s_rxbuf_free(ptri2s_config);
		if(ptri2s_config->mmap_index <= MAX_I2S_PAGE)
			ptri2s_config->mmap_index = 0;	
		//i2s_rxPagebuf_free(ptri2s_config);
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);
		break;
	case I2S_RX_PAUSE:
		spin_lock_irqsave(&ptri2s_config->lock, flags);
                ptri2s_config->rx_pause_en = 1;
		MSG("* rx_pause_en = 1 *\n");
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);
                break;
	case I2S_RX_RESUME:
		spin_lock_irqsave(&ptri2s_config->lock, flags);
                ptri2s_config->rx_pause_en = 0;
		MSG("# rx_pause_en = 0 #\n");
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);
                break;
	case I2S_CODEC_MIC_BOOST:
		spin_lock_irqsave(&ptri2s_config->lock, flags);
		if((int)arg > 3)
			ptri2s_config->micboost = 3;
		else if((int)arg < 0)
			ptri2s_config->micboost = 0;
		else
			ptri2s_config->micboost = arg;
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);
		break;
	case I2S_CODEC_MIC_IN:
		spin_lock_irqsave(&ptri2s_config->lock, flags);
		if((int)arg == 1)
			ptri2s_config->micin = 1;
		else
			ptri2s_config->micin = 0;
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);
		break;
        case I2S_CLOCK_ENABLE:
                spin_lock_irqsave(&ptri2s_config->lock, flags);
                i2s_clock_disable(ptri2s_config);
#if defined(CONFIG_RALINK_MT7628) || defined(CONFIG_ARCH_MT7623)
                ptri2s_config->wordlen_24b = 1;
#endif
                i2s_tx_config(ptri2s_config);
                i2s_clock_enable(ptri2s_config);
                i2s_tx_enable(ptri2s_config);
                spin_unlock_irqrestore(&ptri2s_config->lock, flags);
                break;
	case I2S_DEBUG_CODEC:
		spin_lock_irqsave(&ptri2s_config->lock, flags);
		for (i=0; i<10; i++)
		{
			printk("### i=%d ###\n", i);
			i2s_clock_enable(ptri2s_config);
			i2s_clock_disable(ptri2s_config);
		}
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);
	      	break;
#if defined(CONFIG_I2S_MS_CTRL)
	case I2S_MS_MODE_CTRL:
		spin_lock_irqsave(&ptri2s_config->lock, flags);
		if((int)arg == 1)
		{
			ptri2s_config->slave_en = 1;
			printk("I2S in slave mode.\n");
		}
		else 
		{
			ptri2s_config->slave_en = 0;
			printk("I2S in master mode.\n");
		}
		spin_unlock_irqrestore(&ptri2s_config->lock, flags);
		break;
#endif
	case I2S_DEBUG_CLKGEN:
	case I2S_DEBUG_INLBK:
	case I2S_DEBUG_EXLBK:
	case I2S_DEBUG_CODECBYPASS:	
	case I2S_DEBUG_FMT:
#if defined(CONFIG_I2S_WM8960)
	case I2S_DEBUG_CODEC_EXLBK:
#endif
	case I2S_DEBUG_RESET:
		i2s_debug_cmd(cmd, arg);
		break;							
	default :
		MSG("i2s_ioctl: command format error\n");
	}

	return 0;
}

/************************
 *      API for ALSA    *
 *                      *
 ************************/
char* i2s_memPool_Alloc(i2s_config_type* ptri2s_config,int dir)
{
        //printk("%s\n",__func__);
        if(!ptri2s_config)
                return NULL;
        if(dir == STREAM_PLAYBACK){
#if defined(CONFIG_I2S_MMAP)
                i2s_mmap_alloc(I2S_TOTAL_PAGE_SIZE);
#endif
                i2s_txbuf_alloc(ptri2s_config);
		return ptri2s_config->pMMAPTxBufPtr[0];
        }else{
#if defined(CONFIG_I2S_MMAP)
                i2s_mmap_alloc(I2S_TOTAL_PAGE_SIZE);
#endif
		i2s_rxbuf_alloc(ptri2s_config);	
		return ptri2s_config->pMMAPRxBufPtr[0];
	}
        return NULL;
}

void i2s_memPool_free(i2s_config_type* ptri2s_config,int dir)
{
        if(!ptri2s_config)
                return;
        if(dir == STREAM_PLAYBACK){
#if defined(CONFIG_I2S_MMAP)
		i2s_mem_unmap(ptri2s_config);
#endif
		i2s_txbuf_free(ptri2s_config);
        }else{
#if defined(CONFIG_I2S_MMAP)
		i2s_mem_unmap(ptri2s_config);
#endif
		i2s_rxbuf_free(ptri2s_config);
        }

        return;
}

int i2s_page_prepare(i2s_config_type* ptri2s_config,int dir)
{
        if(dir == STREAM_PLAYBACK){
                /* allocate tx buffer */
                i2s_txPagebuf_alloc(ptri2s_config);
		i2s_dma_tx_init(ptri2s_config);
	}else{
                /* allocate rx buffer */
		i2s_rxPagebuf_alloc(ptri2s_config);
		i2s_dma_rx_init(ptri2s_config);
        }
        return 0;
}

int i2s_page_release(i2s_config_type* ptri2s_config,int dir)
{
        if(!ptri2s_config)
                return (-1);
        if(dir == STREAM_PLAYBACK)
		i2s_txPagebuf_free(ptri2s_config);
        else
		i2s_rxPagebuf_free(ptri2s_config);
        
	return 0;
}

int i2s_startup(void)
{
	memset(pi2s_config, 0, sizeof(i2s_config_type));
	
#ifdef I2S_STATISTIC
	memset(pi2s_status, 0, sizeof(i2s_status_type));	
#endif

	i2s_param_init(pi2s_config);
	pi2s_config->bALSAEnable = 1;
#if defined(CONFIG_I2S_MMAP)
	pi2s_config->bALSAMMAPEnable = 1;
#endif

#if defined (CONFIG_RALINK_MT7628) || defined(CONFIG_ARCH_MT7623)
	pi2s_config->little_edn = 1;
#endif

    	init_waitqueue_head(&(pi2s_config->i2s_tx_qh));
    	init_waitqueue_head(&(pi2s_config->i2s_rx_qh));
	spin_lock_init(&pi2s_config->lock);

	return 0;
}

int gdma_En_Switch(i2s_config_type* ptri2s_config,int dir,int enabled){
        if(!ptri2s_config)
                return (-1);
        if(dir == STREAM_PLAYBACK){
                ptri2s_config->bTxDMAEnable = enabled;
                //MSG("%s:%d\n",__func__,ptri2s_config->bTxDMAEnable);
        }else{
                ptri2s_config->bRxDMAEnable = enabled;
        }
        return 0;
}

int i2s_audio_exchange(i2s_config_type* ptri2s_config,int dir,unsigned long arg)
{
        //MSG("I2S_PUT_AUDIO\n");
        if(!ptri2s_config)
                return (-1);
        if(dir == STREAM_PLAYBACK){
        	i2s_put_audio(ptri2s_config, arg);
	}else{
		i2s_get_audio(ptri2s_config, arg);
        }
        return 0;
}

void gdma_mask_handler(u32 dma_ch)
{
	i2s_dma_mask_handler(dma_ch);
        return;
}

void gdma_unmask_handler(u32 dma_ch)
{
        i2s_dma_unmask_handler(dma_ch);
	return;
}

u32 i2s_mmap_phys_addr(i2s_config_type* ptri2s_config)
{
	if((ptri2s_config->pMMAPBufPtr[0]!=NULL) && (ptri2s_config->mmap_index == MAX_I2S_PAGE))
		return (dma_addr_t)i2s_mmap_addr[0];
	else if((ptri2s_config->pMMAPBufPtr[MAX_I2S_PAGE]!=NULL) && (ptri2s_config->mmap_index == MAX_I2S_PAGE*2))
		return (dma_addr_t)i2s_mmap_addr[MAX_I2S_PAGE];
	else
		return -1;
}

EXPORT_SYMBOL(i2s_startup);
EXPORT_SYMBOL(i2s_mem_unmap);
EXPORT_SYMBOL(i2s_mmap_alloc);
EXPORT_SYMBOL(i2s_mmap_remap);
EXPORT_SYMBOL(i2s_param_init);
EXPORT_SYMBOL(i2s_txbuf_alloc);
EXPORT_SYMBOL(i2s_rxbuf_alloc);
EXPORT_SYMBOL(i2s_txPagebuf_alloc);
EXPORT_SYMBOL(i2s_rxPagebuf_alloc);
EXPORT_SYMBOL(i2s_txbuf_free);
EXPORT_SYMBOL(i2s_rxbuf_free);
EXPORT_SYMBOL(i2s_txPagebuf_free);
EXPORT_SYMBOL(i2s_rxPagebuf_free);
EXPORT_SYMBOL(i2s_rx_disable);
EXPORT_SYMBOL(i2s_tx_disable);
EXPORT_SYMBOL(i2s_rx_enable);
EXPORT_SYMBOL(i2s_tx_enable);
EXPORT_SYMBOL(i2s_rx_config);
EXPORT_SYMBOL(i2s_tx_config);
EXPORT_SYMBOL(i2s_reset_config);
EXPORT_SYMBOL(i2s_clock_disable);
EXPORT_SYMBOL(i2s_clock_enable);
EXPORT_SYMBOL(i2s_reset_rx_param);
EXPORT_SYMBOL(i2s_reset_tx_param);
EXPORT_SYMBOL(i2s_dma_rx_handler);
EXPORT_SYMBOL(i2s_dma_tx_handler);
EXPORT_SYMBOL(i2s_dma_unmask_handler);
EXPORT_SYMBOL(i2s_dma_tx_unmask_handler);
EXPORT_SYMBOL(i2s_dma_rx_unmask_handler);
EXPORT_SYMBOL(i2s_dma_mask_handler);
EXPORT_SYMBOL(i2s_dma_tx_init);
EXPORT_SYMBOL(i2s_dma_rx_init);
EXPORT_SYMBOL(i2s_tx_end_sleep_on);
EXPORT_SYMBOL(i2s_rx_end_sleep_on);
EXPORT_SYMBOL(i2s_mmap_phys_addr);
EXPORT_SYMBOL(i2s_open);
EXPORT_SYMBOL(pi2s_config);
#if defined(CONFIG_I2S_IN_MCLK)
#if defined(CONFIG_I2S_MCLK_12MHZ)
EXPORT_SYMBOL(i2s_refclk_12m_enable);
#endif
#if defined(CONFIG_I2S_MCLK_12P288MHZ)
EXPORT_SYMBOL(i2s_refclk_12p288m_enable);
#endif
#endif
#if defined(MT7628_ASIC_BOARD) || defined(CONFIG_ARCH_MT7623)
EXPORT_SYMBOL(i2s_driving_strength_adjust);
#endif
EXPORT_SYMBOL(i2s_refclk_disable);
EXPORT_SYMBOL(i2s_refclk_gpio_out_config);
EXPORT_SYMBOL(i2s_refclk_gpio_in_config);
EXPORT_SYMBOL(i2s_share_pin_config);
EXPORT_SYMBOL(i2s_share_pin_mt7623);
EXPORT_SYMBOL(i2s_ws_config);
EXPORT_SYMBOL(i2s_mode_config);
EXPORT_SYMBOL(i2s_codec_frequency_config);
EXPORT_SYMBOL(i2s_dma_tx_transf_data);
EXPORT_SYMBOL(i2s_dma_tx_transf_zero);
EXPORT_SYMBOL(i2s_dma_rx_transf_data);
EXPORT_SYMBOL(i2s_dma_rx_transf_zero);
EXPORT_SYMBOL(i2s_dma_tx_end_handle);
EXPORT_SYMBOL(i2s_dma_tx_soft_stop);
EXPORT_SYMBOL(i2s_dma_rx_soft_stop);
EXPORT_SYMBOL(i2s_tx_task);
EXPORT_SYMBOL(i2s_rx_task);

EXPORT_SYMBOL(i2s_memPool_Alloc);
EXPORT_SYMBOL(i2s_memPool_free);
EXPORT_SYMBOL(i2s_page_prepare);
EXPORT_SYMBOL(i2s_page_release);
EXPORT_SYMBOL(gdma_En_Switch);
EXPORT_SYMBOL(i2s_audio_exchange);
EXPORT_SYMBOL(gdma_mask_handler);
EXPORT_SYMBOL(gdma_unmask_handler);
#if defined(CONFIG_I2S_WITH_AEC)
EXPORT_SYMBOL(aecFuncP);
#endif
module_init(i2s_mod_init);
module_exit(i2s_mod_exit);

MODULE_DESCRIPTION("Ralink SoC I2S Controller Module");
MODULE_AUTHOR("Qwert Chin <qwert.chin@ralinktech.com.tw>");
MODULE_SUPPORTED_DEVICE("I2S");
MODULE_VERSION(I2S_MOD_VERSION);
MODULE_LICENSE("GPL");
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,12)
MODULE_PARM (i2sdrv_major, "i");
#else
module_param (i2sdrv_major, int, 0);
#endif
