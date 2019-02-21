/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2007 by Ralf Baechle
 */
#include <linux/clocksource.h>
#ifdef CONFIG_MET
#include <linux/cnt32_to_63.h>
#endif
#include <linux/init.h>
#ifdef CONFIG_MET
#include <linux/timer.h>
#endif

#include <asm/time.h>

static cycle_t c0_hpt_read(struct clocksource *cs)
{
	return read_c0_count();
}

static struct clocksource clocksource_mips = {
	.name		= "MIPS",
	.read		= c0_hpt_read,
	.mask		= CLOCKSOURCE_MASK(32),
	.flags		= CLOCK_SOURCE_IS_CONTINUOUS,
};

#ifdef CONFIG_MET
#ifdef CONFIG_CPU_SUPPORTS_HR_SCHED_CLOCK
/*
 * MIPS sched_clock implementation.
 *
 * Because the hardware timer period is quite short and because cnt32_to_63()
 * needs to be called at least once per half period to work properly, a kernel
 * timer is set up to ensure this requirement is always met.
 *
 * Please refer to include/linux/cnt32_to_63.h, arch/arm/plat-orion/time.c and
 * arch/mips/include/asm/time.h (mips_cyc2ns)
 */

#define CYC2NS_SHIFT 8
static u32 mult __read_mostly;
static u32 shift __read_mostly;

unsigned long long notrace sched_clock(void)
{
	u64 cyc = cnt32_to_63(read_c0_count());

#ifdef CONFIG_64BIT
	/* For we have used 128bit arithmatic to cope with the overflow
	 * problem, the method to clear the top bit with an event value doesn't
	 * work now, therefore, clear it at run-time is needed.
	 */
	if (cyc & 0x8000000000000000)
		cyc &= 0x7fffffffffffffff;
#endif
	return mips_cyc2ns(cyc, mult, shift);
}

static struct timer_list cnt32_to_63_keepwarm_timer;

static void notrace cnt32_to_63_keepwarm(unsigned long data)
{
	mod_timer(&cnt32_to_63_keepwarm_timer, round_jiffies(jiffies + data));
	sched_clock();
}
#endif

static inline void setup_hres_sched_clock(unsigned long clock)
{
#ifdef CONFIG_CPU_SUPPORTS_HR_SCHED_CLOCK
	unsigned long data;

#ifdef CONFIG_32BIT
	unsigned long long v;

	v = NSEC_PER_SEC;
	v <<= CYC2NS_SHIFT;
	v += clock/2;
	do_div(v, clock);
	mult = v;
	shift = CYC2NS_SHIFT;
	/*
	 * We want an even value to automatically clear the top bit
	 * returned by cnt32_to_63() without an additional run time
	 * instruction. So if the LSB is 1 then round it up.
	 */
	if (mult & 1)
		mult++;
#else
	mult = clocksource_mips.mult;
	shift = clocksource_mips.shift;
#endif

	data = 0x80000000UL / clock * HZ;
	setup_timer(&cnt32_to_63_keepwarm_timer, cnt32_to_63_keepwarm, data);
	mod_timer(&cnt32_to_63_keepwarm_timer, round_jiffies(jiffies + data));
#endif
}
#endif


int __init init_r4k_clocksource(void)
{
	if (!cpu_has_counter || !mips_hpt_frequency)
		return -ENXIO;

	/* Calculate a somewhat reasonable rating value */
	clocksource_mips.rating = 200 + mips_hpt_frequency / 10000000;
#ifdef CONFIG_MET
	setup_hres_sched_clock(mips_hpt_frequency);
#endif
	clocksource_register_hz(&clocksource_mips, mips_hpt_frequency);

	return 0;
}
