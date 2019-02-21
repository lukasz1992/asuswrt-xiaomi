/*
 *  Kevin D. Kissell, kevink@mips and Carsten Langgaard, carstenl@mips.com
 *  Copyright (C) 2000 MIPS Technologies, Inc.	All rights reserved.
 *
 *  This program is free software; you can distribute it and/or modify it
 *  under the terms of the GNU General Public License (Version 2) as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 * Routines corresponding to Linux kernel FP context
 * manipulation primitives for the Algorithmics MIPS
 * FPU Emulator
 */
#include <linux/sched.h>
#include <asm/processor.h>
#include <asm/signal.h>
#include <asm/uaccess.h>

#include <asm/fpu.h>
#include <asm/fpu_emulator.h>

#define SIGNALLING_NAN      0x7ff800007ff80000LL
#define SIGNALLING_NAN2008  0x7ff000007fa00000LL

extern unsigned int fpu_fcr31 __read_mostly;
extern unsigned int system_has_fpu __read_mostly;
static int nan2008 __read_mostly = -1;

static int __init setup_nan2008(char *str)
{
	get_option (&str, &nan2008);

	return 1;
}

__setup("nan2008=", setup_nan2008);

void fpu_emulator_init_fpu(void)
{
	static int first = 1;
	int i;

	if (first) {
		first = 0;
		printk("Algorithmics/MIPS FPU Emulator v1.5\n");
	}

	if (system_has_fpu)
		current->thread.fpu.fcr31 = fpu_fcr31;
	else if (nan2008 < 0) {
		if (!test_thread_flag(TIF_32BIT_REGS))
			current->thread.fpu.fcr31 = FPU_CSR_DEFAULT|FPU_CSR_MAC2008|FPU_CSR_ABS2008|FPU_CSR_NAN2008;
		else
			current->thread.fpu.fcr31 = FPU_CSR_DEFAULT;
	} else {
		if (nan2008)
			current->thread.fpu.fcr31 = FPU_CSR_DEFAULT|FPU_CSR_MAC2008|FPU_CSR_ABS2008|FPU_CSR_NAN2008;
		else
			current->thread.fpu.fcr31 = FPU_CSR_DEFAULT;
	}

	if (current->thread.fpu.fcr31 & FPU_CSR_NAN2008)
		for (i = 0; i < 32; i++) {
			current->thread.fpu.fpr[i] = SIGNALLING_NAN2008;
		}
	else
		for (i = 0; i < 32; i++) {
			current->thread.fpu.fpr[i] = SIGNALLING_NAN;
		}
}


/*
 * Emulator context save/restore to/from a signal context
 * presumed to be on the user stack, and therefore accessed
 * with appropriate macros from uaccess.h
 */

inline int fpu_emulator_save_context(struct sigcontext __user *sc)
{
	int i;
	int err = 0;

	for (i = 0; i < 32; i++) {
		err |=
		    __put_user(current->thread.fpu.fpr[i], &sc->sc_fpregs[i]);
	}
	err |= __put_user(current->thread.fpu.fcr31, &sc->sc_fpc_csr);

	return err;
}

inline int fpu_emulator_restore_context(struct sigcontext __user *sc)
{
	int i;
	int err = 0;

	for (i = 0; i < 32; i++) {
		err |=
		    __get_user(current->thread.fpu.fpr[i], &sc->sc_fpregs[i]);
	}
	err |= __get_user(current->thread.fpu.fcr31, &sc->sc_fpc_csr);

	return err;
}

#ifdef CONFIG_64BIT
/*
 * This is the o32 version
 */

int fpu_emulator_save_context32(struct sigcontext32 __user *sc)
{
	int i;
	int err = 0;

	if (!test_thread_flag(TIF_32BIT_REGS)) {
		for (i = 0; i < 32; i++) {
			err |=
			    __put_user(current->thread.fpu.fpr[i], &sc->sc_fpregs[i]);
		}
		err |= __put_user(current->thread.fpu.fcr31, &sc->sc_fpc_csr);

		return err;

	}

	for (i = 0; i < 32; i+=2) {
		err |=
		    __put_user(current->thread.fpu.fpr[i], &sc->sc_fpregs[i]);
	}
	err |= __put_user(current->thread.fpu.fcr31, &sc->sc_fpc_csr);

	return err;
}

int fpu_emulator_restore_context32(struct sigcontext32 __user *sc)
{
	int i;
	int err = 0;

	if (!test_thread_flag(TIF_32BIT_REGS)) {
		for (i = 0; i < 32; i++) {
			err |=
			    __get_user(current->thread.fpu.fpr[i], &sc->sc_fpregs[i]);
		}
		err |= __get_user(current->thread.fpu.fcr31, &sc->sc_fpc_csr);

		return err;
	}

	for (i = 0; i < 32; i+=2) {
		err |=
		    __get_user(current->thread.fpu.fpr[i], &sc->sc_fpregs[i]);
	}
	err |= __get_user(current->thread.fpu.fcr31, &sc->sc_fpc_csr);

	return err;
}
#endif
