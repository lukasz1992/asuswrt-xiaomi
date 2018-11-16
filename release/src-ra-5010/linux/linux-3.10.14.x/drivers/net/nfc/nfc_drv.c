/**
           NFC Madule 
 **/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/version.h>
#include <linux/uaccess.h>
// ---------------------------------------------------------------------------
// define
// ---------------------------------------------------------------------------
#define DEVICE_MAJOR 219
#define DEVICE_NAME "mt6605"
#define MAX_LENGTH 768

// gpio direction
#define GPIO_INPUT	0
#define GPIO_OUTPUT	1

// use non-blocking I/O to fix nfcsd can't be killed issue
#define NON_BLOCKING_IO

// ---------------------------------------------------------------------------
// enum (from mtk_nfc_sys.h)
// ---------------------------------------------------------------------------
typedef enum 
{
    MTK_NFC_GPIO_EN_B = 0x0,
    MTK_NFC_GPIO_SYSRST_B,
    MTK_NFC_GPIO_INT,
    MTK_NFC_GPIO_IRQ,
    MTK_NFC_GPIO_MAX_NUM
} MTK_NFC_GPIO_E;

typedef enum 
{
    MTK_NFC_PULL_LOW  = 0x0,
    MTK_NFC_PULL_HIGH,
    MTK_NFC_PULL_INVALID,
} MTK_NFC_PULL_E;

typedef enum
{
    MTK_NFC_IOCTL_READ = 0x0,
    MTK_NFC_IOCTL_WRITE,
    MTK_NFC_IOCTL_MAX_NUM
} MTK_NFC_IOCTL_E;

// ---------------------------------------------------------------------------
// global variables
// ---------------------------------------------------------------------------
static int major;
static char nfc_dev_buf[MAX_LENGTH];

#if 0 // old PCB
/*  
    ------------------------------------------------
    | MT6605 EVB GPIO Mapping on RT3883 AP TestBed |
    ------------------------------------------------
    |   MT6605   |   AP platform                   |
    ------------------------------------------------
    | Led GIO    |   WPS Led GIO                   |
    | VEN        |   GPIO7                         |
    | I2C data   |   I2C data PIN                  |
    | I2C clk    |   I2C clock PIN                 |
    | INT        |   GPIO2                         |
    | Reset Pin  |   use jumper default pull high  |
    -----------------------------------------------
*/
static int pin_mapping[MTK_NFC_GPIO_MAX_NUM] = 
{
	30, // MTK_NFC_GPIO_EN_B
	-1, // MTK_NFC_GPIO_SYSRST_B
	26	// MTK_NFC_GPIO_INT
};
#else // new PCB
/*  new PCB @ 2013/2/26  
    ------------------------------------------------
    | MT6605 EVB GPIO Mapping on RT3883 AP TestBed |
    ------------------------------------------------
    |   MT6605   |   AP platform                   |
    ------------------------------------------------
    | Led GIO    |   GPIO7                         |
    | VEN        |   GPIO13                        |
    | I2C data   |                                 |
    | I2C clk    |                                 |
    | IRQ        |   GPIO9                         |
    | INT        |   GPIO12                        |
    | Reset Pin  |   GPIO11                        |
    -----------------------------------------------
*/
static int pin_mapping[MTK_NFC_GPIO_MAX_NUM] = 
{
#ifdef CONFIG_RALINK_MT7621
	9, // MTK_NFC_GPIO_EN_B
	6, // MTK_NFC_GPIO_SYSRST_B
	12, // MTK_NFC_GPIO_INT
	10  // MTK_NFC_GPIO_IRQ
#elif CONFIG_ARCH_MT8590
	9, // MTK_NFC_GPIO_EN_B
	6, // MTK_NFC_GPIO_SYSRST_B
	12, // MTK_NFC_GPIO_INT
	10  // MTK_NFC_GPIO_IRQ	
#elif defined(CONFIG_RALINK_RT3883)
	13, // MTK_NFC_GPIO_EN_B
	11, // MTK_NFC_GPIO_SYSRST_B
	12, // MTK_NFC_GPIO_INT
	 9  // MTK_NFC_GPIO_IRQ
#else
#error "Chip not support"
#endif
};
#endif

// ---------------------------------------------------------------------------
// extern functions (from i2c_drv.h)
// ---------------------------------------------------------------------------
// I2C & GPIO Utilities
extern void i2c_read_MT6605(char *data, unsigned int len);
extern void i2c_write_MT6605(char *data, unsigned int len);
extern void i2c_check_MT6605(unsigned char *data, unsigned char *expected, unsigned int len);
extern void MT6605_gpio_init(u8 num, u8 dir);
extern int MT6605_gpio_read(u8 num);
extern void MT6605_gpio_write(u8 num, u8 val);

// ---------------------------------------------------------------------------
// function prototypes
// ---------------------------------------------------------------------------
static int mt6605_dev_open(struct inode *, struct file *);
static int mt6605_dev_release(struct inode *, struct file *);
static ssize_t mt6605_dev_read(struct file *filp, char __user *buf, size_t count, loff_t *offset);
static ssize_t mt6605_dev_write(struct file *filp, const char __user *buf, size_t count, loff_t *offset);
//static int mt6605_dev_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
static long mt6605_dev_unlocked_ioctl (struct file *filp, unsigned int cmd, unsigned long arg);
#else
static int mt6605_dev_unlocked_ioctl (struct inode *inode, struct file *filp, unsigned int cmd, unsigned long *arg);
#endif

// ---------------------------------------------------------------------------
// data structure
// ---------------------------------------------------------------------------
struct ioctl_arg
{
    int pin;
    int highlow;
};

static struct file_operations nfc_dev_fops = 
{
    .owner   = THIS_MODULE,
    .read    = mt6605_dev_read,
    .write   = mt6605_dev_write,
    .open    = mt6605_dev_open,
    .release = mt6605_dev_release,
    //.ioctl   = mt6605_dev_ioctl,
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
    .unlocked_ioctl = mt6605_dev_unlocked_ioctl,
#else
    .ioctl   = mt6605_dev_unlocked_ioctl,
#endif

};

// ---------------------------------------------------------------------------
// functions
// ---------------------------------------------------------------------------
static int nfc_init(void)
{
    printk("nfc initialized!\n");
        
    major = register_chrdev(DEVICE_MAJOR, DEVICE_NAME, &nfc_dev_fops);
    if (major < 0) {
        printk(KERN_ALERT "Register Fail\n");
        return major;
    }
	
    return 0;
}

static void nfc_exit(void)
{
    printk("nfc exited\n");
    
    unregister_chrdev(major, DEVICE_NAME); 
}

static int mt6605_dev_open(struct inode *inode, struct file *filp)
{
    printk(KERN_ALERT "open() was called\n");

    // Initialize MT6605 GPIO
    MT6605_gpio_init(pin_mapping[MTK_NFC_GPIO_EN_B]    , GPIO_OUTPUT);
    MT6605_gpio_init(pin_mapping[MTK_NFC_GPIO_SYSRST_B], GPIO_OUTPUT);
    MT6605_gpio_init(pin_mapping[MTK_NFC_GPIO_INT]     , GPIO_OUTPUT);
    MT6605_gpio_init(pin_mapping[MTK_NFC_GPIO_IRQ]     , GPIO_INPUT);
    MT6605_gpio_write(pin_mapping[MTK_NFC_GPIO_SYSRST_B], 1);    
    MT6605_gpio_write(pin_mapping[MTK_NFC_GPIO_EN_B]    , 0);

    return 0;	
}

static int mt6605_dev_release(struct inode *inod, struct file *filp)
{
    printk(KERN_ALERT "close() was called\n");

    // Power off MT6605 (To advoid abnormal exit!!!)
    MT6605_gpio_write(pin_mapping[MTK_NFC_GPIO_SYSRST_B], 0);    
    MT6605_gpio_write(pin_mapping[MTK_NFC_GPIO_EN_B]    , 1);

    return 0;
}

static int mt6605_dev_read(struct file *filp, char __user *buf, size_t count, loff_t *offset)
{
    //int j;
    	
    if (count >= MAX_LENGTH)
    {
        printk(KERN_ALERT "Unexpected read size\n");
        return 0;
    }

    // check i2c indicator
#ifdef NON_BLOCKING_IO
    if (!MT6605_gpio_read(pin_mapping[MTK_NFC_GPIO_IRQ]))
    {
        return 0;
    }
#else    
    while(!MT6605_gpio_read(pin_mapping[MTK_NFC_GPIO_IRQ]))
    {
    	  msleep(1);
    }
#endif    

    i2c_read_MT6605(nfc_dev_buf, count);
copy_to_user(buf, nfc_dev_buf, count);
    
#if 0 // debug info
    printk("\n\nread length : %d\n", count);
    printk("read data : ");
    for (j = 0; j < count; j++)
		  printk("0x%02x ", buf[j]);
		printk("\n\n\n");
#endif		
    
    return count;
}

static int mt6605_dev_write(struct file *filp, const char __user *buf, size_t count, loff_t *offset)
{
    //int j;
    	
    if (count >= MAX_LENGTH)
    {
        printk(KERN_ALERT "Unexpected write size\n");
        return 0;
    }
	  
#if 0	// debug info
    printk("\n\nwrite length : %d\n", count);
    printk("write data : ");
    for (j = 0; j < count; j++)
		  printk("0x%02x ", buf[j]);
		printk("\n\n\n");
#endif       
		
copy_from_user(nfc_dev_buf, buf, count);
    i2c_write_MT6605(nfc_dev_buf, count);    
    
    return count;
}

//static int mt6605_dev_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
//{
//}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
long mt6605_dev_unlocked_ioctl (struct file *filp, unsigned int cmd, unsigned long arg)
#else
int mt6605_dev_unlocked_ioctl (struct inode *inode, struct file *filp, unsigned int cmd, unsigned long *arg)
#endif
{   
	  //struct ioctl_arg *ioctl_data = filp->private_data;
	  struct ioctl_arg data;
	  int ret = 0;
	  
//	  printk("mt6605_dev_unlocked_ioctl : cmd %d\n", cmd);
	  
	  memset(&data, 0, sizeof(data));	 	  
	  
	  switch (cmd) 
	  {
	      case MTK_NFC_IOCTL_READ: //read
	      	  if (copy_from_user(&data, (int __user *)arg, sizeof(struct ioctl_arg))) {
                ret = -1;
            }
            else
            {
               // printk("IOCTL READ arg : pin %d\n", data.pin);
                if (pin_mapping[data.pin] != -1)
                {
                    data.highlow = MT6605_gpio_read(pin_mapping[data.pin]);
                    //printk("IOCTL READ arg : pin %d, highlow %d\n", data.pin, data.highlow);
                    if (copy_to_user((int __user *)arg, &data, sizeof(data)))
                    {
                        ret = -1;
                    }
                }
            }
		        break;
		
	      case MTK_NFC_IOCTL_WRITE: //write
            if (copy_from_user(&data, (int __user *)arg, sizeof(struct ioctl_arg))) {
                ret = -1;
            }
            else
            {
             //   printk("IOCTL WRITE arg : pin %d, highlow %d\n", data.pin, data.highlow);
                if (pin_mapping[data.pin] != -1)
                {
                    MT6605_gpio_write(pin_mapping[data.pin], data.highlow);
                }
            }
            break;
		
	      default:	      	  
		        printk("mt6605_dev_unlocked_ioctl: invalid command type\n");
		        ret = -1;
	  }
	  
	  return ret;
}

module_init(nfc_init);
module_exit(nfc_exit);

MODULE_AUTHOR("Hiki Chen");
MODULE_DESCRIPTION("MTK NFC Driver");
MODULE_LICENSE("GPL");
