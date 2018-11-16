/*******************************************************************************
 * Filename:
 * ---------
 *  mt6605.c
 *
 * Project:
 * --------
 *
 * Description:
 * ------------
 *
 * Author:
 * -------
 *  LiangChi Huang, ext 25609, LiangChi.Huang@mediatek.com, 2012-08-09
 *
 *******************************************************************************/
/*****************************************************************************
 * Include
 *****************************************************************************/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>
#include <linux/spinlock.h>
#include <linux/dma-mapping.h>

#include <mach/cust_gpio_usage.h>
#include <mach/cust_eint.h>
#include <mach/mt_gpio.h>
//#include <cust_eint_ext.h>
#include <mach/eint.h>


#include <linux/interrupt.h>
/*
#include <cust_gpio_usage.h>
#include <cust_eint.h>
#include <cust_i2c.h>

#include <mach/mt_gpio.h>
#include <mach/eint.h>
*/
/* #include <mach/mt6605.h>// */
/*
#include <linux/interrupt.h>

#ifdef CONFIG_MTK_MT6306_SUPPORT
#include <mach/dcl_sim_gpio.h>
#endif
*/
/*****************************************************************************
 * Define
 *****************************************************************************/
//#define pr_debug(fmt,arg...)  printk(KERN_ERR fmt,##arg)

#define NFC_I2C_BUSNUM  0/*I2C_NFC_CHANNEL*/

#define I2C_ID_NAME "mt6605"

#define MAX_BUFFER_SIZE	255

#define NFC_CLIENT_TIMING 100	/* I2C speed */
#define I2C_MASK_FLAG	(0x00ff)
/*****************************************************************************
 * GLobal Variable
 *****************************************************************************/
struct mt6605_dev *mt6605_dev_ptr = NULL;

static struct i2c_board_info nfc_board_info __initdata = { I2C_BOARD_INFO(I2C_ID_NAME, 0x28/*I2C_NFC_SLAVE_7_BIT_ADDR*/) };

/* For DMA */
static char *I2CDMAWriteBuf;
static unsigned int I2CDMAWriteBuf_pa;	/* = NULL; */
static char *I2CDMAReadBuf;
static unsigned int I2CDMAReadBuf_pa;	/* = NULL; */

static int fgNfcChip;
int forceExitBlockingRead = 0;

/*****************************************************************************
 * Function Prototype
 *****************************************************************************/
static int mt6605_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int mt6605_remove(struct i2c_client *client);
/* static int mt6605_detect(struct i2c_client *client, int kind, struct i2c_board_info *info); */
static int mt6605_dev_open(struct inode *inode, struct file *filp);
static long mt6605_dev_unlocked_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static ssize_t mt6605_dev_read(struct file *filp, char __user *buf, size_t count, loff_t *offset);
static ssize_t mt6605_dev_write(struct file *filp, const char __user *buf, size_t count,
				loff_t *offset);
void mt6605_dev_irq_handler(void);
/* static void mt6605_disable_irq(struct mt6605_dev *mt6605_dev); */
extern void mt_eint_soft_set(unsigned int eint_num);

/*****************************************************************************
 * Data Structure
 *****************************************************************************/

struct mt6605_dev {
	wait_queue_head_t read_wq;
	struct mutex read_mutex;
	struct i2c_client *client;
	struct miscdevice mt6605_device;
	unsigned int ven_gpio;
	unsigned int sysrstb_gpio;
	unsigned int irq_gpio;	/* Chip inform Host */
	unsigned int eint_gpio;	/* Host inform Chip */
	bool irq_enabled;

	struct mutex irq_enabled_lock;
	/* spinlock_t            irq_enabled_lock; */
};

struct mt6605_i2c_platform_data {
	unsigned int irq_gpio;	/* Chip inform Host */
	unsigned int ven_gpio;
	unsigned int sysrstb_gpio;
	unsigned int eint_gpio;	/* Host inform Chip */
};


static const struct i2c_device_id mt6605_id[] = {
	{I2C_ID_NAME, 0},
	{}
};

/* #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)) */
/* static struct i2c_client_address_data addr_data = { .forces = forces,}; */
/* #endif */
static struct i2c_driver mt6605_dev_driver = {
	.probe = mt6605_probe,
	.remove = mt6605_remove,
	.driver = {
		   .name = "mt6605",
		   .owner = THIS_MODULE,
		   },
	.id_table = mt6605_id,
};


static struct mt6605_i2c_platform_data mt6605_platform_data = {
	.irq_gpio = GPIO_IRQ_NFC_PIN,
	.ven_gpio = GPIO_NFC_VENB_PIN,
	.sysrstb_gpio = GPIO_NFC_RST_PIN,
	.eint_gpio = GPIO_NFC_EINT_PIN,
};


static const struct file_operations mt6605_dev_fops = {
	.owner = THIS_MODULE,
	.llseek = no_llseek,
	.read = mt6605_dev_read,
	.write = mt6605_dev_write,
	.open = mt6605_dev_open,
	.unlocked_ioctl = mt6605_dev_unlocked_ioctl,
};

/*****************************************************************************
 * Extern Area
 *****************************************************************************/



/*****************************************************************************
 * Function
 *****************************************************************************/


static void mt6605_disable_irq(struct mt6605_dev *mt6605_dev)
{
	/* unsigned long flags; */
	pr_debug("%s : irq %d, enable=%d\n", __func__, mt6605_dev->client->irq,
	       mt6605_dev->irq_enabled);
	mt_eint_mask(mt6605_dev->client->irq);
	pr_debug("mt6605_disable_irq\n");

}

struct mt6605_dev _gmt6605_dev;
static int mt6605_probe(struct i2c_client *client, const struct i2c_device_id *id)
{

	int ret;
	struct mt6605_i2c_platform_data *platform_data;
#if 0
	struct mt6605_dev *mt6605_dev;
#endif

	pr_debug("mt6605_dev_probe\n");

	platform_data = &mt6605_platform_data;

	if (platform_data == NULL) {
		pr_debug("%s : nfc probe fail\n", __func__);
		return -ENODEV;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_debug("%s : need I2C_FUNC_I2C\n", __func__);
		return -ENODEV;
	}

	mt_set_gpio_mode(platform_data->irq_gpio, GPIO_IRQ_NFC_PIN_M_EINT);
	mt_set_gpio_dir(platform_data->irq_gpio, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(platform_data->irq_gpio, GPIO_PULL_ENABLE);
	mt_set_gpio_pull_select(platform_data->irq_gpio, GPIO_PULL_DOWN);



#ifdef GPIO_NFC_VENB_PIN
	/* ven_gpio */
	mt_set_gpio_mode(platform_data->ven_gpio, GPIO_NFC_VENB_PIN_M_GPIO);
	mt_set_gpio_dir(platform_data->ven_gpio, GPIO_DIR_OUT);
	mt_set_gpio_out(platform_data->ven_gpio, GPIO_OUT_ONE);
	mt_set_gpio_pull_enable(platform_data->irq_gpio, GPIO_PULL_ENABLE);//
	mt_set_gpio_pull_select(platform_data->irq_gpio, GPIO_PULL_DOWN);//
	usleep_range(900, 1000);	/* debug */
#else
	pr_debug("%s : need define GPIO_NFC_VENB_PIN in codegen.dws", __func__);
	return -ENODEV;
#endif

#ifdef GPIO_NFC_RST_PIN
	/* firm_gpio */
	mt_set_gpio_mode(platform_data->sysrstb_gpio, GPIO_NFC_RST_PIN_M_GPIO);
	mt_set_gpio_dir(platform_data->sysrstb_gpio, GPIO_DIR_OUT);
	mt_set_gpio_out(platform_data->sysrstb_gpio, GPIO_OUT_ONE);
	mt_set_gpio_pull_enable(platform_data->irq_gpio, GPIO_PULL_ENABLE);//
	mt_set_gpio_pull_select(platform_data->irq_gpio, GPIO_PULL_DOWN);//	
	usleep_range(900, 1000);	/* debug */
#else
	pr_debug("%s : need define GPIO_NFC_RST_PIN in codegen.dws", __func__);
	return -ENODEV;
#endif


	/* EINT_gpio */
	mt_set_gpio_mode(platform_data->eint_gpio, GPIO_NFC_EINT_PIN_M_GPIO);
	mt_set_gpio_dir(platform_data->eint_gpio, GPIO_DIR_OUT);
	/* Set output High */
	mt_set_gpio_out(platform_data->eint_gpio, GPIO_OUT_ONE);
	mt_set_gpio_pull_enable(platform_data->irq_gpio, GPIO_PULL_ENABLE);//
	mt_set_gpio_pull_select(platform_data->irq_gpio, GPIO_PULL_DOWN);//

#if 0				/* following WCP2's prolicy "Not allow to use non-cached locks" */
	mt6605_dev = kzalloc(sizeof(*mt6605_dev), GFP_KERNEL);
	if (mt6605_dev == NULL) {
		pr_debug("pn544_dev: failed to allocate memory for module data\n");
		ret = -ENOMEM;
		goto err_exit;
	}
#endif

	_gmt6605_dev.irq_gpio = platform_data->irq_gpio;
	_gmt6605_dev.ven_gpio = platform_data->ven_gpio;
	_gmt6605_dev.sysrstb_gpio = platform_data->sysrstb_gpio;
	_gmt6605_dev.eint_gpio = platform_data->eint_gpio;
	_gmt6605_dev.client = client;

	/* init mutex and queues */
	init_waitqueue_head(&_gmt6605_dev.read_wq);
	mutex_init(&_gmt6605_dev.read_mutex);
	/* spin_lock_init(&mt6605_dev->irq_enabled_lock); */
	mutex_init(&_gmt6605_dev.irq_enabled_lock);

	_gmt6605_dev.mt6605_device.minor = MISC_DYNAMIC_MINOR;
	_gmt6605_dev.mt6605_device.name = "mt6605";
	_gmt6605_dev.mt6605_device.fops = &mt6605_dev_fops;

	ret = misc_register(&_gmt6605_dev.mt6605_device);
	if (ret) {
		pr_debug("%s : misc_register failed\n", __FILE__);
		goto err_misc_register;
	}
#ifdef CONFIG_64BIT
	I2CDMAWriteBuf =
	    (char *)dma_alloc_coherent(&client->dev, MAX_BUFFER_SIZE,
				       (dma_addr_t *) &I2CDMAWriteBuf_pa, GFP_KERNEL);
#else
	I2CDMAWriteBuf =
	    (char *)dma_alloc_coherent(NULL, MAX_BUFFER_SIZE, (dma_addr_t *) &I2CDMAWriteBuf_pa,
				       GFP_KERNEL);
#endif

	if (I2CDMAWriteBuf == NULL) {
		pr_debug("%s : failed to allocate dma buffer\n", __func__);
		goto err_request_irq_failed;
	}
#ifdef CONFIG_64BIT
	I2CDMAReadBuf =
	    (char *)dma_alloc_coherent(&client->dev, MAX_BUFFER_SIZE,
				       (dma_addr_t *) &I2CDMAReadBuf_pa, GFP_KERNEL);
#else
	I2CDMAReadBuf =
	    (char *)dma_alloc_coherent(NULL, MAX_BUFFER_SIZE, (dma_addr_t *) &I2CDMAReadBuf_pa,
				       GFP_KERNEL);
#endif

	if (I2CDMAReadBuf == NULL) {
		pr_debug("%s : failed to allocate dma buffer\n", __func__);
		goto err_request_irq_failed;
	}
	pr_debug("%s :I2CDMAWriteBuf_pa %d, I2CDMAReadBuf_pa,%d\n", __func__,
	       I2CDMAWriteBuf_pa, I2CDMAReadBuf_pa);

	/* request irq.  the irq is set whenever the chip has data available
	 * for reading.  it is cleared when all data has been read.
	 */
	client->irq = CUST_EINT_IRQ_NFC_NUM;

	pr_debug("%s : requesting IRQ %d\n", __func__, client->irq);

	pr_debug("mt6605_Prob2\n");

	mt_eint_set_hw_debounce(CUST_EINT_IRQ_NFC_NUM, CUST_EINT_IRQ_NFC_DEBOUNCE_CN);

	mt_eint_registration(CUST_EINT_IRQ_NFC_NUM, CUST_EINT_IRQ_NFC_TYPE, mt6605_dev_irq_handler,
			     0);
	/* LC debug,20121021 */
	/* mt_eint_unmask(CUST_EINT_IRQ_NFC_NUM); */
	/* mt6605_disable_irq(mt6605_dev); */
	mt_eint_mask(CUST_EINT_IRQ_NFC_NUM);

	i2c_set_clientdata(client, &_gmt6605_dev);

	forceExitBlockingRead = 0;

	return 0;

err_request_irq_failed:

	misc_deregister(&_gmt6605_dev.mt6605_device);

err_misc_register:

	mutex_destroy(&_gmt6605_dev.read_mutex);

	/* kfree(mt6605_dev); */
	/* err_exit: */

	gpio_free(platform_data->sysrstb_gpio);

	return ret;

}


static int mt6605_remove(struct i2c_client *client)
{
	/* struct mt6605_dev *mt6605_dev; */

	pr_debug("mt6605_remove\n");

	if (I2CDMAWriteBuf) {
#ifdef CONFIG_64BIT
		dma_free_coherent(&client->dev, MAX_BUFFER_SIZE, I2CDMAWriteBuf, I2CDMAWriteBuf_pa);
#else
		dma_free_coherent(NULL, MAX_BUFFER_SIZE, I2CDMAWriteBuf, I2CDMAWriteBuf_pa);
#endif
		I2CDMAWriteBuf = NULL;
		I2CDMAWriteBuf_pa = 0;
	}

	if (I2CDMAReadBuf) {
#ifdef CONFIG_64BIT
		dma_free_coherent(&client->dev, MAX_BUFFER_SIZE, I2CDMAReadBuf, I2CDMAReadBuf_pa);
#else
		dma_free_coherent(NULL, MAX_BUFFER_SIZE, I2CDMAReadBuf, I2CDMAReadBuf_pa);
#endif
		I2CDMAReadBuf = NULL;
		I2CDMAReadBuf_pa = 0;
	}
	/* mt6605_dev = i2c_get_clientdata(client); */
	free_irq(client->irq, &_gmt6605_dev);
	misc_deregister(&_gmt6605_dev.mt6605_device);
	mutex_destroy(&_gmt6605_dev.read_mutex);
	gpio_free(_gmt6605_dev.irq_gpio);
	gpio_free(_gmt6605_dev.ven_gpio);
	gpio_free(_gmt6605_dev.sysrstb_gpio);
	gpio_free(_gmt6605_dev.eint_gpio);
	/* kfree(mt6605_dev); */

	return 0;
}


void mt6605_dev_irq_handler(void)
{
	struct mt6605_dev *mt6605_dev = mt6605_dev_ptr;
	/* pr_debug("%s : &mt6605_dev=%p\n", __func__, mt6605_dev); */

	if (NULL == mt6605_dev) {
		pr_debug("mt6605_dev NULL\n");
		return;
	}
	mt6605_disable_irq(mt6605_dev);
	wake_up(&mt6605_dev->read_wq);
	/* wake_up_interruptible(&mt6605_dev->read_wq); */

	pr_debug("%s : wake_up &read_wq=%p, mt6605_dev=%p\n", __func__,
	       &mt6605_dev->read_wq, mt6605_dev);
	/* pr_debug("mt6605_dev_irq_handler\n"); */
}

static ssize_t mt6605_dev_read(struct file *filp, char __user *buf, size_t count, loff_t *offset)
{
	struct mt6605_dev *mt6605_dev = filp->private_data;
	/* char read_buf[MAX_BUFFER_SIZE]; */
	int ret = 0;
	/* int dir=0; */

	pr_debug("%s : %d, &mt6605_dev=%p", __func__, count, mt6605_dev); 

	if (count > MAX_BUFFER_SIZE)
		count = MAX_BUFFER_SIZE;

	/* pr_debug("%s : reading %zu bytes.\n", __func__, count); */


	if (!mt_get_gpio_in(mt6605_dev->irq_gpio)) {
		if (filp->f_flags & O_NONBLOCK) {
			ret = -EAGAIN;
			pr_debug("%s : goto fail\n", __func__);
			goto fail;
		}
		/* mutex_lock(&mt6605_dev->irq_enabled_lock); */
		/* mt6605_dev->irq_enabled = true; */
		/* mutex_unlock(&mt6605_dev->irq_enabled_lock); */

		mutex_lock(&mt6605_dev->read_mutex);
		if (forceExitBlockingRead == 1) {
			pr_debug("%s : forceExitBlockingRead.\n", __func__);
			forceExitBlockingRead = 0;	/* clear flag */
			mutex_unlock(&mt6605_dev->read_mutex);
			goto fail;
		}
		mutex_unlock(&mt6605_dev->read_mutex);

		mt_eint_unmask(mt6605_dev->client->irq);	/* Enable Interrupt */
		pr_debug("%s : mt_eint_unmask %d, IRQ, %d\n",
		       __func__, mt6605_dev->client->irq, mt_get_gpio_in(mt6605_dev->irq_gpio));

		ret =
		    wait_event_interruptible(mt6605_dev->read_wq,
					     (mt_get_gpio_in(mt6605_dev->irq_gpio)
					      || forceExitBlockingRead));
		pr_debug("%s : wait_event_interruptible ret %d,gpio,%d\n", __func__, ret,
		       mt_get_gpio_in(mt6605_dev->irq_gpio));

		if (ret || forceExitBlockingRead) {
			pr_debug("%s : goto fail\n", __func__);
			mutex_lock(&mt6605_dev->read_mutex);
			if (forceExitBlockingRead == 1) {
				pr_debug("%s : clear flag for forceExitBlockingRead\n",
				       __func__);
				forceExitBlockingRead = 0;	/* clear flag */
			}
			mutex_unlock(&mt6605_dev->read_mutex);
			goto fail;
		}

	}

	mt6605_dev->client->addr = (mt6605_dev->client->addr & I2C_MASK_FLAG);	/* | I2C_DMA_FLAG; */
//
	mt6605_dev->client->ext_flag |= I2C_DMA_FLAG;
	/* mt6605_dev->client->ext_flag |= I2C_DIRECTION_FLAG; */
	mt6605_dev->client->ext_flag |= I2C_A_FILTER_MSG;
	mt6605_dev->client->timing = NFC_CLIENT_TIMING;
	/* Read data */
	//ret =
	//    i2c_master_recv(mt6605_dev->client, (unsigned char *)(uintptr_t) I2CDMAReadBuf, count);
	ret = i2c_master_recv(mt6605_dev->client, (unsigned char *)I2CDMAReadBuf_pa, count);	
	/* mutex_unlock(&mt6605_dev->read_mutex); */

	pr_debug("%s : i2c_master_recv returned %d, IRQ, %d\n", __func__, ret,
	       mt_get_gpio_in(mt6605_dev->irq_gpio));

	if (ret < 0) {
		pr_debug("%s : i2c_master_recv returned %d\n", __func__, ret);
		return ret;
	}
	if (ret > count) {
		pr_debug("%s : received too many bytes from i2c (%d)\n", __func__, ret);
		return -EIO;
	}

	if (copy_to_user(buf, I2CDMAReadBuf, ret)) {
		pr_debug("%s : failed to copy to user space\n", __func__);
		return -EFAULT;
	}

	pr_debug("%s : return,ret,%d\n", __func__, ret);

	return ret;

fail:
	/* mutex_unlock(&mt6605_dev->read_mutex); */
	pr_debug("%s : return,ret2,%d\n", __func__, ret);
	return ret;

}

static ssize_t mt6605_dev_write(struct file *filp, const char __user *buf, size_t count,
				loff_t *offset)
{
	struct mt6605_dev *mt6605_dev;
	/* char write_buf[MAX_BUFFER_SIZE]; */
	int ret = 0, ret_tmp = 0, count_ori = 0, count_remain = 0, idx = 0;

	pr_debug("%s : %d", __func__, count);

	mt6605_dev = filp->private_data;

	count_ori = count;
	count_remain = count_ori;

	if (count > MAX_BUFFER_SIZE) {
		count = MAX_BUFFER_SIZE;
		count_remain -= count;
	}
	/* msleep(1); */

	do {

		if (copy_from_user(I2CDMAWriteBuf, &buf[(idx * 255)], count)) {
			pr_debug("%s : failed to copy from user space\n", __func__);
			return -EFAULT;
		}
		/* pr_debug("%s : writing %zu bytes, remain bytes %zu.\n",
		__func__, count, count_remain); */
		pr_debug("%s : writing %zu bytes, remain bytes %d.\n", __func__, count,
		       count_remain);

		/* Write data */
		mt6605_dev->client->addr = (mt6605_dev->client->addr & I2C_MASK_FLAG);	/* | I2C_DMA_FLAG; */

		 mt6605_dev->client->ext_flag |= I2C_DMA_FLAG; 
		/* mt6605_dev->client->ext_flag |= I2C_DIRECTION_FLAG; */
		 mt6605_dev->client->ext_flag |= I2C_A_FILTER_MSG;
		 mt6605_dev->client->timing = NFC_CLIENT_TIMING;
		//ret_tmp =
		//    i2c_master_send(mt6605_dev->client,
		//		    (unsigned char *)(uintptr_t) I2CDMAWriteBuf, count);
		ret_tmp = i2c_master_send(mt6605_dev->client, (unsigned char *)I2CDMAWriteBuf_pa, count);
		if (ret_tmp != count) {
			pr_debug("%s : i2c_master_send returned %d\n", __func__, ret);
			ret = -EIO;
			return ret;
		}

		ret += ret_tmp;
		/* pr_debug("%s : %d,%d,%d\n", __func__, ret_tmp,ret,count_ori); */

		if (ret == count_ori) {
			/* pr_debug("%s : ret== count_ori\n", __func__); */
			break;
		} else {
			if (count_remain > MAX_BUFFER_SIZE) {
				count = MAX_BUFFER_SIZE;
				count_remain -= MAX_BUFFER_SIZE;
			} else {
				count = count_remain;
				count_remain = 0;
			}
			idx++;

			/* pr_debug("%s :remain_bytes, %d,%d,%d,%d,%d\n",
			__func__, ret_tmp,ret,count,count_ori,idx); */
		}

	} while (1);

	pr_debug("%s : writing %d bytes. Status %d\n", __func__, count_ori, ret);
	/* pr_debug("%s : writing %zu bytes. Status %zu\n", __func__, count_ori,ret); */
	return ret;
}

static int mt6605_dev_open(struct inode *inode, struct file *filp)
{
	struct mt6605_dev *mt6605_dev =
	    container_of(filp->private_data, struct mt6605_dev, mt6605_device);

	pr_debug("mt6605_dev_open,%s : filp->private_data=%p\n",
	       __func__, filp->private_data);


	filp->private_data = mt6605_dev;
	mt6605_dev_ptr = mt6605_dev;

	pr_debug("mt6605_dev_open,%s : %d,%d, &mt6605_dev_open=%p, file=%p\n",
	       __func__, imajor(inode), iminor(inode), mt6605_dev_ptr, filp);

	/* 20121009,LiangChi add */
	/* mt_set_gpio_out(mt6605_dev->ven_gpio, GPIO_OUT_ZERO); */
	/* mt_set_gpio_out(mt6605_dev->sysrstb_gpio, GPIO_OUT_ONE); */

	forceExitBlockingRead = 0;
	return 0;
}


/* #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)) */
/* static int mt6605_dev_ioctl(struct inode *inode, struct file *filp, */
/* unsigned int cmd, unsigned long arg) */
/* { */
/* return 0; */
/* } */
/* #else */

typedef enum {
	MTK_NFC_GPIO_EN_B = 0x0,
	MTK_NFC_GPIO_SYSRST_B,
	MTK_NFC_GPIO_EINT,
	MTK_NFC_GPIO_IRQ,
	MTK_NFC_GPIO_MAX_NUM
} MTK_NFC_GPIO_E;

typedef enum {
	MTK_NFC_IOCTL_READ = 0x0,
	MTK_NFC_IOCTL_WRITE,
	MTK_NFC_IOCTL_MAX_NUM
} MTK_NFC_IOCTL;

typedef enum {
	MTK_NFC_PULL_LOW = 0x0,
	MTK_NFC_PULL_HIGH,
	MTK_NFC_PULL_INVALID,
} MTK_NFC_PULL_E;

int mt6605_get_gpio_dir(unsigned long pin)
{
	/* workaround for mt6306 bug */
	return GPIO_DIR_OUT;
}

static long mt6605_dev_unlocked_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{

	struct mt6605_dev *mt6605_dev = filp->private_data;
	int result = 0;
	int gpio_dir, gpio_num, tmp_gpio;

	int (*mt_set_gpio_out_ptr) (unsigned long, unsigned long);
	int (*mt_get_gpio_dir_ptr) (unsigned long);
	int (*mt_get_gpio_ptr) (unsigned long);

	mt_get_gpio_dir_ptr = mt_get_gpio_dir;
	mt_set_gpio_out_ptr = mt_set_gpio_out;
	mt_get_gpio_ptr = mt_get_gpio_in;


	pr_debug("mt6605_dev_unlocked_ioctl,cmd,%d,%lu\n", cmd, arg);
	/* (((arg & 0xFF00) >> 8) & 0x00FF), */
	/* (arg & 0x00FF)); */

	if ((cmd & 0xFFFF) == 0xFE00) {
		mt_eint_mask(CUST_EINT_IRQ_NFC_NUM);
		pr_debug("mt6605_dev_unlocked_ioctl,mask_IRQ");
		return 0;
	} else if ((cmd & 0xFFFF) == 0xFE01) {
		mt_set_gpio_mode(mt6605_dev->irq_gpio, GPIO_IRQ_NFC_PIN_M_EINT);
		mt_set_gpio_dir(mt6605_dev->irq_gpio, GPIO_DIR_IN);
		mt_set_gpio_pull_enable(mt6605_dev->irq_gpio, GPIO_PULL_ENABLE);
		mt_set_gpio_pull_select(mt6605_dev->irq_gpio, GPIO_PULL_DOWN);

		mt_eint_set_hw_debounce(CUST_EINT_IRQ_NFC_NUM, CUST_EINT_IRQ_NFC_DEBOUNCE_CN);
		mt_eint_registration(CUST_EINT_IRQ_NFC_NUM, CUST_EINT_IRQ_NFC_TYPE,
				     mt6605_dev_irq_handler, 0);
		mt_eint_mask(CUST_EINT_IRQ_NFC_NUM);
		pr_debug("mt6605_dev_unlocked_ioctl,Re-registered IRQ");
		return 0;
	} else if ((cmd & 0xFFFF) == 0xFEFF) {	/* EXIT EINT */
		mutex_lock(&mt6605_dev->read_mutex);
		forceExitBlockingRead = 1;
		mutex_unlock(&mt6605_dev->read_mutex);
		/* mt_eint_soft_set(CUST_EINT_EXT_IRQ_NFC_NUM); */
		/* mt65xx_eint_mask(CUST_EINT_EXT_IRQ_NFC_NUM); */
		wake_up_interruptible(&mt6605_dev->read_wq);
		pr_debug("mt6605_dev_unlocked_ioctl,SW Release IRQ");
		return 0;
	} else if ((cmd & 0xFFFF) == 0xFEFE) {	/* Get ChipID */
		return fgNfcChip;
	} else if ((cmd & 0xFFFF) == 0xFEFD) {	/*  */
		fgNfcChip = (arg & 0xFFFF);
		return 0;
	}

	tmp_gpio = (((arg & 0xFF00) >> 8) & 0x00FF);

	if (tmp_gpio == MTK_NFC_GPIO_EN_B) {
		gpio_num = mt6605_dev->ven_gpio;
#ifdef CONFIG_MTK_MT6306_SUPPORT
		mt_get_gpio_dir_ptr = mt6605_get_gpio_dir;
		mt_get_gpio_ptr = mt6306_get_gpio_out;
		mt_set_gpio_out_ptr = mt6306_set_gpio_out;
#endif
	} else if (tmp_gpio == MTK_NFC_GPIO_SYSRST_B) {
		gpio_num = mt6605_dev->sysrstb_gpio;
#ifdef CONFIG_MTK_MT6306_SUPPORT
		mt_get_gpio_dir_ptr = mt6605_get_gpio_dir;
		mt_get_gpio_ptr = mt6306_get_gpio_out;
		mt_set_gpio_out_ptr = mt6306_set_gpio_out;
#endif
	} else if (tmp_gpio == MTK_NFC_GPIO_EINT) {
		gpio_num = mt6605_dev->eint_gpio;
	} else if (tmp_gpio == MTK_NFC_GPIO_IRQ) {
		gpio_num = mt6605_dev->irq_gpio;
	} else {
		result = MTK_NFC_PULL_INVALID;
	}

	if (result != MTK_NFC_PULL_INVALID) {
		if (cmd == MTK_NFC_IOCTL_READ) {	/* Read */
			gpio_dir = mt_get_gpio_dir_ptr(gpio_num);

			if ((gpio_dir == GPIO_DIR_IN) || (gpio_dir == GPIO_DIR_OUT))
				result = mt_get_gpio_ptr(gpio_num);
			else
				result = MTK_NFC_PULL_INVALID;

		} else if (cmd == MTK_NFC_IOCTL_WRITE) {	/* WRITE */
			gpio_dir = mt_get_gpio_dir_ptr(gpio_num);
			if (gpio_dir == GPIO_DIR_OUT) {
				int gpio_pol = (arg & 0x00FF);

				if (gpio_pol == MTK_NFC_PULL_LOW)
					result = mt_set_gpio_out_ptr(gpio_num, GPIO_OUT_ZERO);
				else if (gpio_pol == MTK_NFC_PULL_HIGH)
					result = mt_set_gpio_out_ptr(gpio_num, GPIO_OUT_ONE);

			} else {
				result = MTK_NFC_PULL_INVALID;
			}
		} else {	/* ERROR */

			result = MTK_NFC_PULL_INVALID;
		}
	}

	pr_debug("mt6605_dev_unlocked_ioctl,%d\n", result);
	return result;

}

/* return 0, success; return <0, fail */
/* md_id        : modem id */
/* md_state   : 0, on ; 1, off ; */
/* vsim_state : 0, on ; 1, off; */
/*
int inform_nfc_vsim_change(int md_id, int md_state, int vsim_state)
{
    char send_data[] = {0xaa, 0x0f, 0x03, 0x00, 0x03, 0x00, 0x00, 0xaa, 0xf0};
    int ret = 0;
    int send_bytes = sizeof(send_data);
    int retry;

    //AA 0F 03 00 03 00 xx AA F0
    //where xx is defined:
    //Bit[7:4] : md_id
    //Bit[3:2] : md_state
    //Bit[1:0] : vsim_state

    send_data[6] |= (md_id<<4);
    send_data[6] |= (md_state<<2);
    send_data[6] |= (vsim_state);
	pr_debug("inform_nfc_vsim_change, md_id,%d, md_state,%d, vsim_state,%d , send_data[6],0x%X.\n",
	md_id, md_state, vsim_state, send_data[6]);

    //send to mt6605
    _gmt6605_dev.client->addr = (_gmt6605_dev.client->addr & I2C_MASK_FLAG);
    _gmt6605_dev.client->ext_flag |= I2C_DMA_FLAG;
    _gmt6605_dev.client->timing = 400;

    memcpy(I2CDMAWriteBuf, send_data, send_bytes);

    //eint pull high
    ret = mt_set_gpio_out(mt6605_platform_data.eint_gpio, GPIO_OUT_ONE);

    //sleep 5ms
    msleep(5);

    for (retry = 0; retry < 10; retry++)
    {
	//ret = i2c_master_send(_gmt6605_dev.client, (unsigned char *)I2CDMAWriteBuf_pa, send_bytes);
	    ret = i2c_master_send(_gmt6605_dev.client, (unsigned char *)(uintptr_t)I2CDMAWriteBuf_pa, send_bytes);//
	if (ret == send_bytes)
	{
	    pr_debug("inform_nfc_vsim_change, send to mt6605 OK. retry %d", retry);
	    break;
	}
	else
	{
	    pr_debug("inform_nfc_vsim_change, send to mt6605 fail. retry %d, ret %d", retry, ret);
	}
	//sleep 2ms
	msleep(2);
    }
    //eint pull low
    ret = mt_set_gpio_out(mt6605_platform_data.eint_gpio, GPIO_OUT_ZERO);

    return 0;
}
*/
#if 0
static int mt6605_detect(struct i2c_client *client, int kind, struct i2c_board_info *info)
{

	pr_debug("mt6605_detect\n");
	/* strcpy(info->type, "mt6605"); */
	return 0;
}
#endif
/*
 * module load/unload record keeping
 */

static int __init mt6605_dev_init(void)
{
	pr_debug("mt6605_dev_init\n");
	i2c_register_board_info(NFC_I2C_BUSNUM, &nfc_board_info, 1);
	pr_debug("mt6605_dev_init2\n");
	i2c_add_driver(&mt6605_dev_driver);
	return 0;
}

static void __exit mt6605_dev_exit(void)
{
	pr_debug("mt6605_dev_exit\n");
	i2c_del_driver(&mt6605_dev_driver);
	/* return 0; */
}


module_init(mt6605_dev_init);
module_exit(mt6605_dev_exit);

MODULE_AUTHOR("LiangChi Huang");
MODULE_DESCRIPTION("MTK NFC driver");
MODULE_LICENSE("GPL");
