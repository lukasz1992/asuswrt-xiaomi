/*
 * Copyright (C) 2002 MontaVista Software Inc.
 * Author: Jun Sun, jsun@mvista.com or jsun@junsun.net
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */
#ifndef _ASM_FPU_H
#define _ASM_FPU_H

#include <linux/sched.h>
#include <linux/thread_info.h>
#include <linux/bitops.h>

#include <asm/mipsregs.h>
#include <asm/cpu.h>
#include <asm/cpu-features.h>
#include <asm/hazards.h>
#include <asm/processor.h>
#include <asm/current.h>

#ifdef CONFIG_MIPS_MT_FPAFF
#include <asm/mips_mt.h>
#endif

struct sigcontext;
struct sigcontext32;

extern void fpu_emulator_init_fpu(void);
extern int _init_fpu(void);
extern void _save_fp(struct task_struct *);
extern void _restore_fp(struct task_struct *);

/*
 * This macro is used only to obtain FIR from FPU and it seems
 * like a BUG in 34K with single FPU affinity to VPE0.
 */
#define __enable_fpu()                                                  \
do {									\
	set_c0_status(ST0_CU1);						\
	enable_fpu_hazard();						\
} while (0)

#define clear_fpu_owner()	clear_thread_flag(TIF_USEDFPU)

static inline int __is_fpu_owner(void)
{
	return test_thread_flag(TIF_USEDFPU);
}

static inline int is_fpu_owner(void)
{
	return cpu_has_fpu && __is_fpu_owner();
}

static inline int __own_fpu(void)
{
	int ret = 0;

#if defined(CONFIG_CPU_MIPS32_R2) || defined(CONFIG_MIPS64)
	if (test_thread_flag(TIF_32BIT_REGS)) {
		change_c0_status(ST0_CU1|ST0_FR,ST0_CU1);
		KSTK_STATUS(current) |= ST0_CU1;
		KSTK_STATUS(current) &= ~ST0_FR;
		enable_fpu_hazard();
		if (read_c0_status() & ST0_FR)
		    ret = SIGFPE;
	} else {
		set_c0_status(ST0_CU1|ST0_FR);
		KSTK_STATUS(current) |= ST0_CU1|ST0_FR;
		enable_fpu_hazard();
		if (!(read_c0_status() & ST0_FR))
		    ret = SIGFPE;
	}
#else
	if (!test_thread_flag(TIF_32BIT_REGS))
		return SIGFPE;  /* core has no 64bit FPU, so ... */

	set_c0_status(ST0_CU1);
	KSTK_STATUS(current) |= ST0_CU1;
	enable_fpu_hazard();
#endif
	set_thread_flag(TIF_USEDFPU);
	return ret;
}

static inline int own_fpu_inatomic(int restore)
{
	int ret = 0;

	if (cpu_has_fpu && !__is_fpu_owner()) {
		ret =__own_fpu();
		if (restore && !ret)
			_restore_fp(current);
	}
	return ret;
}

static inline int own_fpu(int restore)
{
	int ret;

	preempt_disable();
	ret = own_fpu_inatomic(restore);
	preempt_enable();

	return ret;
}

static inline void lose_fpu(int save)
{
	preempt_disable();
	if (is_fpu_owner()) {
		if (save)
			_save_fp(current);
		KSTK_STATUS(current) &= ~ST0_CU1;
		clear_thread_flag(TIF_USEDFPU);
		clear_c0_status(ST0_CU1);
		disable_fpu_hazard();
	}
	preempt_enable();
}

static inline int init_fpu(void)
{
	int ret = 0;

	preempt_disable();
	if (cpu_has_fpu && !(ret = __own_fpu()))
		_init_fpu();
	else
		fpu_emulator_init_fpu();

	preempt_enable();

	return ret;
}

static inline void save_fp(struct task_struct *tsk)
{
	if (cpu_has_fpu)
		_save_fp(tsk);
}

static inline void restore_fp(struct task_struct *tsk)
{
	if (cpu_has_fpu)
		_restore_fp(tsk);
}

static inline fpureg_t *get_fpu_regs(struct task_struct *tsk)
{
	if (tsk == current) {
		preempt_disable();
		if (is_fpu_owner())
			_save_fp(current);
		preempt_enable();
	}

	return tsk->thread.fpu.fpr;
}

#endif /* _ASM_FPU_H */
