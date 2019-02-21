#include <linux/version.h>
#include <linux/string.h>
#include <linux/module.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
#include <asm/rt2880/prom.h>
#define PRINT(fmt, args...) prom_printf(fmt, ##args)
#else
#if defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628) || defined (CONFIG_RALINK_RT5350)
#include <asm/mach-ralink/prom.h>
#define PRINT(fmt, args...) prom_printf(fmt, ##args)
#else
#define PRINT(fmt, args...)  printk(KERN_ALERT fmt, ##args)
#endif
#endif

#define SDK_VERSION "5.0.S.0"

char pathinfo[][10] = {	"",
};
			
void show_sdk_patch_info(void)
{
	int i = 0;

	PRINT("\nSDK %s\n", SDK_VERSION);
	while (strlen(pathinfo[i]) > 0)
		PRINT("%s\n", pathinfo[i++]);
}
EXPORT_SYMBOL(show_sdk_patch_info);
