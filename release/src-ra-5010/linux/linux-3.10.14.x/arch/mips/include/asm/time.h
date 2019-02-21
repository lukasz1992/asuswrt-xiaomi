/*
 * Copyright (C) 2001, 2002, MontaVista Software Inc.
 * Author: Jun Sun, jsun@mvista.com or jsun@junsun.net
 * Copyright (c) 2003  Maciej W. Rozycki
 *
 * include/asm-mips/time.h
 *     header file for the new style time.c file and time services.
 *
 * This program is free software; you can redistribute	it and/or modify it
 * under  the terms of	the GNU General	 Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */
#ifndef _ASM_TIME_H
#define _ASM_TIME_H

#include <linux/rtc.h>
#include <linux/spinlock.h>
#include <linux/clockchips.h>
#include <linux/clocksource.h>
#include <asm/gic.h>

extern spinlock_t rtc_lock;

/*
 * RTC ops.  By default, they point to weak no-op RTC functions.
 *	rtc_mips_set_time - reverse the above translation and set time to RTC.
 *	rtc_mips_set_mmss - similar to rtc_set_time, but only min and sec need
 *			to be set.  Used by RTC sync-up.
 */
extern int rtc_mips_set_time(unsigned long);
extern int rtc_mips_set_mmss(unsigned long);

/*
 * board specific routines required by time_init().
 */
extern void plat_time_init(void);

/*
 * mips_hpt_frequency - must be set if you intend to use an R4k-compatible
 * counter as a timer interrupt source.
 */
extern unsigned int mips_hpt_frequency;

/*
 * The performance counter IRQ on MIPS is a close relative to the timer IRQ
 * so it lives here.
 */
extern int (*perf_irq)(void);

/*
 * Initialize the calling CPU's compare interrupt as clockevent device
 */
extern unsigned int __weak get_c0_compare_int(void);
extern int r4k_clockevent_init(void);
extern int smtc_clockevent_init(void);
extern int gic_clockevent_init(void);

static inline int mips_clockevent_init(void)
{
#ifdef CONFIG_MIPS_MT_SMTC
	return smtc_clockevent_init();
#elif defined(CONFIG_CEVT_GIC)
	return (gic_clockevent_init() | r4k_clockevent_init());
#elif defined(CONFIG_CEVT_R4K)
	return r4k_clockevent_init();
#else
	return -ENXIO;
#endif
}

/*
 * Initialize the count register as a clocksource
 */
extern int init_r4k_clocksource(void);

static inline int init_mips_clocksource(void)
{
#ifdef CONFIG_CSRC_R4K
	return init_r4k_clocksource();
#else
	return 0;
#endif
}

static inline void clockevent_set_clock(struct clock_event_device *cd,
					unsigned int clock)
{
	clockevents_calc_mult_shift(cd, clock, 4);
}

#ifdef CONFIG_MET
static inline unsigned long long mips_cyc2ns(u64 cyc, u32 __mult, u32 __shift)
{
#ifdef CONFIG_32BIT
	/*
	 * To balance the overhead of 128bit-arithematic and the precision
	 * lost, we choose a smaller shift to avoid the quick overflow as the
	 * X86 & ARM does. please refer to arch/x86/kernel/tsc.c and
	 * arch/arm/plat-orion/time.c
	 */
	return (cyc * __mult) >> __shift;
#else /* CONFIG_64BIT */
	/* 64-bit arithmatic can overflow, so use 128-bit */
	u64 t1, t2, t3;
	unsigned long long rv;
	u64 mult, shift;
	mult = __mult;
	shift = __shift;

	asm (
		"dmultu\t%[cyc],%[mult]\n\t"
		"nor\t%[t1],$0,%[shift]\n\t"
		"mfhi\t%[t2]\n\t"
		"mflo\t%[t3]\n\t"
		"dsll\t%[t2],%[t2],1\n\t"
		"dsrlv\t%[rv],%[t3],%[shift]\n\t"
		"dsllv\t%[t1],%[t2],%[t1]\n\t"
		"or\t%[rv],%[t1],%[rv]\n\t"
		: [rv] "=&r" (rv), [t1] "=&r" (t1), [t2] "=&r" (t2), [t3] "=&r" (t3)
		: [cyc] "r" (cyc), [mult] "r" (mult), [shift] "r" (shift)
		: "hi", "lo");
	return rv;
#endif
}
#endif

#endif /* _ASM_TIME_H */
