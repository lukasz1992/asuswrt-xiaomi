#include <linux/version.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/timer.h>

#include "rt_timer.h"

#define PERIOD0_INTERVAL		1000  /* unit: msec*/

void timer_handler(unsigned long data)
{
    struct timeval tv;
    do_gettimeofday(&tv);

    printk("Get timer0 periodic interrupt at  %02lu.%06lu\n", tv.tv_sec % 100, tv.tv_usec);
}

#if !defined (CONFIG_RALINK_TIMER_WDG) && !defined (CONFIG_RALINK_TIMER_WDG_MODULE)
#define PERIOD1_INTERVAL		2000  /* unit: msec*/
void timer1_handler(unsigned long data)
{
    struct timeval tv;
    do_gettimeofday(&tv);

    printk("Get timer1 periodic interrupt at  %02lu.%06lu\n", tv.tv_sec % 100, tv.tv_usec);
}
#endif

#if defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
#define PERIOD2_INTERVAL		3000  /* unit: msec*/
void timer2_handler(unsigned long data)
{
    struct timeval tv;
    do_gettimeofday(&tv);

    printk("Get timer2 periodic interrupt at  %02lu.%06lu\n", tv.tv_sec % 100, tv.tv_usec);
}
#endif

static int __init watchdog_init(void)
{
	request_tmr_service(PERIOD0_INTERVAL, &timer_handler, 0);
	
#if !defined (CONFIG_RALINK_TIMER_WDG) && !defined (CONFIG_RALINK_TIMER_WDG_MODULE)
	request_tmr1_service(PERIOD1_INTERVAL, &timer1_handler, 0);
#endif
#if defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
	request_tmr2_service(PERIOD2_INTERVAL, &timer2_handler, 0);
#endif
	return 0;
}

static void __exit watchdog_exit(void)
{
	unregister_tmr_service();

#if !defined (CONFIG_RALINK_TIMER_WDG) && !defined (CONFIG_RALINK_TIMER_WDG_MODULE)
	unregister_tmr1_service();
#endif
#if defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
	unregister_tmr2_service();
#endif
}

module_init(watchdog_init);
module_exit(watchdog_exit);

MODULE_AUTHOR("Steven Liu");
MODULE_DESCRIPTION("Ralink APSoC Hardware Periodic Timer Test Module");
MODULE_LICENSE("GPL");
