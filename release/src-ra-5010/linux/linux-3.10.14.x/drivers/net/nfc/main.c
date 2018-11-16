/**
           NFC Madule 
 **/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/cdev.h>

// ---------------------------------------------------------------------------
// define
// ---------------------------------------------------------------------------
#define DEVICE_MAJOR 219
#define DEVICE_NAME "mt6605"
#define MAX_LENGTH 768

// ---------------------------------------------------------------------------
// enum (from mtk_nfc_sys.h)
// ---------------------------------------------------------------------------
typedef enum 
{
    MTK_NFC_GPIO_EN_B = 0x0,
    MTK_NFC_GPIO_SYSRST_B,
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
static int pin_mapping[MTK_NFC_GPIO_MAX_NUM] = 
{
	30, // MTK_NFC_GPIO_EN_B
	26	// MTK_NFC_GPIO_SYSRST_B
};

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
static int mt6605_dev_unlocked_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

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
    .unlocked_ioctl = mt6605_dev_unlocked_ioctl
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
    //MT6605_gpio_init(26, 1); // IND
	  //MT6605_gpio_init(30, 0); // VEN	  
	  
	  // Initialize IND & VEN default value
	  //MT6605_gpio_write(30, 1);
	  //MT6605_gpio_write(30, 0);

    printk("[Init2]IND,GPIO[26]=%d\n", MT6605_gpio_read(26));
    printk("[Init2]VEN,GPIO[30]=%d\n", MT6605_gpio_read(30));

    return 0;	
}

static int mt6605_dev_release(struct inode *inod, struct file *filp)
{
    printk(KERN_ALERT "close() was called\n");
#if 1
    MT6605_gpio_write(30, 1);
#endif    
    return 0;
}

static int mt6605_dev_read(struct file *filp, char __user *buf, size_t count, loff_t *offset)
{
    int j;
    	
    if (count >= MAX_LENGTH)
    {
        printk(KERN_ALERT "Unexpected read size\n");
        return 0;
    }

    // check i2c indicator
#if 0 // debug
    printk("\n\n");
    printk("[1]IND,GPIO[26]=%d\n", MT6605_gpio_read(26));
    printk("[1]VEN,GPIO[30]=%d\n", MT6605_gpio_read(30));
#endif    
    while(!MT6605_gpio_read(26))
    {
    	  msleep(1);
    }
#if 0 // debug    
    printk("[2]IND,GPIO[26]=%d\n", MT6605_gpio_read(26));
    printk("[2]VEN,GPIO[30]=%d\n", MT6605_gpio_read(30));    
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
    int j;
    	
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

// test 

static int mt6605_dev_unlocked_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{   
	  //struct ioctl_arg *ioctl_data = filp->private_data;
	  struct ioctl_arg data;
	  int ret = 0;
	  
	  printk("mt6605_dev_unlocked_ioctl : cmd %d\n", cmd);
	  
	  memset(&data, 0, sizeof(data));	 	  
	  
	  switch (cmd) 
	  {
	      case MTK_NFC_IOCTL_READ: //read
	      	  if (copy_from_user(&data, (int __user *)arg, sizeof(arg))) {
                ret = -1;
            }
            else
            {
                printk("IOCTL READ arg : pin %d\n", data.pin);
                data.highlow = MT6605_gpio_read(pin_mapping[data.pin]);
                printk("IOCTL READ arg : pin %d, highlow %d\n", data.pin, data.highlow);
                if (copy_to_user((int __user *)arg, &data, sizeof(data)))
                {
                    ret = -1;
                }
            }
		        break;
		
	      case MTK_NFC_IOCTL_WRITE: //write
            if (copy_from_user(&data, (int __user *)arg, sizeof(arg))) {
                ret = -1;
            }
            else
            {
                printk("IOCTL WRITE arg : pin %d, highlow %d\n", data.pin, data.highlow);
                MT6605_gpio_write(pin_mapping[data.pin], data.highlow);                
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

MODULE_AUTHOR(Hiki Chen);
MODULE_DESCRIPTION(MTK NFC Driver);
MODULE_LICENSE("GPL");
