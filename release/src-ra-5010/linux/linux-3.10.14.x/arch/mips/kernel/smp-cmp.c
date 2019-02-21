/*
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
 * Copyright (C) 2007 MIPS Technologies, Inc.
 *    Chris Dearman (chris@mips.com)
 */

#undef DEBUG

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/smp.h>
#include <linux/cpumask.h>
#include <linux/interrupt.h>
#include <linux/compiler.h>

#include <linux/atomic.h>
#include <asm/cacheflush.h>
#include <asm/cpu.h>
#include <asm/processor.h>
#include <asm/hardirq.h>
#include <asm/mmu_context.h>
#include <asm/smp.h>
#include <asm/time.h>
#include <asm/mipsregs.h>
#include <asm/mipsmtregs.h>
#include <asm/mips_mt.h>
#include <asm/amon.h>
#include <asm/gic.h>
#include <asm/gcmpregs.h>
#include <asm/bootinfo.h>

static void ipi_call_function(unsigned int cpu)
{
	pr_debug("CPU%d: %s cpu %d status %08x\n",
		 smp_processor_id(), __func__, cpu, read_c0_status());

	gic_send_ipi(plat_ipi_call_int_xlate(cpu));
}


static void ipi_resched(unsigned int cpu)
{
	pr_debug("CPU%d: %s cpu %d status %08x\n",
		 smp_processor_id(), __func__, cpu, read_c0_status());

	gic_send_ipi(plat_ipi_resched_int_xlate(cpu));
}

/*
 * FIXME: This isn't restricted to CMP
 * The SMVP kernel could use GIC interrupts if available
 */
void cmp_send_ipi_single(int cpu, unsigned int action)
{
	unsigned long flags;

	local_irq_save(flags);

	switch (action) {
	case SMP_CALL_FUNCTION:
		ipi_call_function(cpu);
		break;

	case SMP_RESCHEDULE_YOURSELF:
		ipi_resched(cpu);
		break;
	}

	local_irq_restore(flags);
}

static void cmp_send_ipi_mask(const struct cpumask *mask, unsigned int action)
{
	unsigned int i;

	for_each_cpu(i, mask)
		cmp_send_ipi_single(i, action);
}

#ifdef CONFIG_EVA
extern int gcmp_present;
extern unsigned long _gcmp_base;
static unsigned long bev_location = -1;

static int rd_bev_location(char *p)
{
	if ((strlen(p) > 19) && (*(p + 18) == '=')) {
		p += 19;
		bev_location = memparse(p, &p);
	} else
		bev_location = 0xbfc00000;
	return 0;
}
early_param("force-bev-location", rd_bev_location);

static void BEV_overlay_segment_map_check(unsigned long excBase,
	unsigned long excMask, unsigned long excSize)
{
	unsigned long addr;

	if ((excBase == (IO_BASE + IO_SHIFT)) && (excSize == IO_SIZE))
		return;

	printk("WARNING: BEV overlay segment doesn't fit whole I/O reg space, NMI/EJTAG/sRESET may not work\n");

	if ((MAP_BASE < (excBase + excSize)) && (excBase < VMALLOC_END))
		panic("BEV Overlay segment overlaps VMALLOC area\n");
#ifdef CONFIG_HIGHMEM
	if ((PKMAP_BASE < (excBase + excSize)) &&
	    (excBase < (PKMAP_BASE + (PAGE_SIZE*(LAST_PKMAP-1)))))
		panic("BEV Overlay segment overlaps HIGHMEM/PKMAP area\n");
#endif
	for (addr = excBase; addr < (excBase + excSize); addr += PAGE_SIZE) {
		if (page_is_ram(__pa(addr>>PAGE_SHIFT)))
			panic("BEV Overlay segment overlaps memory at %lx\n",addr);
	}
}

void BEV_overlay_segment(void)
{
	unsigned long RExcBase;
	unsigned long RExcExtBase;
	unsigned long excBase;
	unsigned long excMask;
	unsigned long excSize;
	unsigned long addr;
	char *p;

	printk("IO: BASE = 0x%lx, SHIFT = 0x%lx, SIZE = 0x%lx\n",IO_BASE, IO_SHIFT, IO_SIZE);
	RExcBase = GCMPCLCB(RESETBASE);
	RExcExtBase = GCMPCLCB(RESETBASEEXT);
	printk("GCMP base addr = 0x%lx, CLB: ResetExcBase = 0x%lx, ResetExcExtBase = 0x%lx\n",
		_gcmp_base,RExcBase,RExcExtBase);
	if ( !(RExcExtBase & 0x1) )
		return;

	if (bev_location == -1) {
		if ((p = strstr(arcs_cmdline, "force-bev-location")))
			rd_bev_location(p);
	}
	if (bev_location != -1) {
		addr = fls((IO_BASE + IO_SHIFT) ^ bev_location);
nextSize:
		if (addr > 28)
			panic("enforced BEV location is too far from I/O reg space\n");

		excMask = (0xffffffffUL >> (32 - addr));
		excBase = bev_location & ~excMask;
		if (((IO_BASE + IO_SHIFT + IO_SIZE - 1) & ~excMask) != excBase) {
			addr++;
			goto nextSize;
		}
		excSize = ((excBase | excMask) + 1) - excBase;
		printk("Setting BEV = 0x%lx, Overlay segment = 0x%lx, size = 0x%lx\n",
			bev_location, excBase, excSize);

		BEV_overlay_segment_map_check(excBase, excMask, excSize);

		GCMPCLCB(RESETBASEEXT) = (GCMPCLCB(RESETBASEEXT) &
			~GCMP_CCB_RESETEXTBASE_BEV_MASK_MSK) |
			(excMask & GCMP_CCB_RESETEXTBASE_BEV_MASK_MSK);
		GCMPCLCB(RESETBASE) = (GCMPCLCB(RESETBASE) & ~GCMP_CCB_RESETBASE_BEV_MSK) |
			bev_location;
		RExcBase = GCMPCLCB(RESETBASE);
		RExcExtBase = GCMPCLCB(RESETBASEEXT);

		return;
	}

	excBase = RExcBase & GCMP_CCB_RESETBASE_BEV_MSK;
	excMask = (RExcExtBase & GCMP_CCB_RESETEXTBASE_BEV_MASK_MSK) |
		    GCMP_CCB_RESETEXTBASE_BEV_MASK_LOWBITS;
	excBase &= ~excMask;
	excSize = ((excBase | excMask) + 1) - excBase;
	printk("BEV Overlay segment = 0x%lx, size = 0x%lx\n",excBase, excSize);

	BEV_overlay_segment_map_check(excBase, excMask, excSize);
}
#endif

static void cmp_init_secondary(void)
{
	struct cpuinfo_mips *c = &current_cpu_data;

	/* Assume GIC is present */
#ifdef CONFIG_RALINK_MT7621
	if(cpu_has_veic) {
		change_c0_status(ST0_IM, 0);
	} else {
		change_c0_status(ST0_IM, STATUSF_IP2 | STATUSF_IP3 | STATUSF_IP4 | STATUSF_IP5 | STATUSF_IP6 | STATUSF_IP7);
	}
#else
	change_c0_status(ST0_IM, STATUSF_IP3 | STATUSF_IP4 | STATUSF_IP6 |
				 STATUSF_IP7);
#endif

	/* Enable per-cpu interrupts: platform specific */

	c->core = (read_c0_ebase() & 0x3ff) >> (fls(smp_num_siblings)-1);
#if defined(CONFIG_MIPS_MT_SMP) || defined(CONFIG_MIPS_MT_SMTC)
	if (cpu_has_mipsmt)
		c->vpe_id = (read_c0_tcbind() >> TCBIND_CURVPE_SHIFT) &
			TCBIND_CURVPE;
#endif
#ifdef CONFIG_MIPS_MT_SMTC
	c->tc_id  = (read_c0_tcbind() & TCBIND_CURTC) >> TCBIND_CURTC_SHIFT;
#endif

#ifdef CONFIG_EVA
	if (gcmp_present)
		BEV_overlay_segment();
#endif
}

static void cmp_smp_finish(void)
{
	pr_debug("SMPCMP: CPU%d: %s\n", smp_processor_id(), __func__);

	/* CDFIXME: remove this? */
	write_c0_compare(read_c0_count() + (8 * mips_hpt_frequency / HZ));

#ifdef CONFIG_MIPS_MT_FPAFF
	/* If we have an FPU, enroll ourselves in the FPU-full mask */
	if (cpu_has_fpu)
		cpu_set(smp_processor_id(), mt_fpu_cpumask);
#endif /* CONFIG_MIPS_MT_FPAFF */

	local_irq_enable();
}

static void cmp_cpus_done(void)
{
	pr_debug("SMPCMP: CPU%d: %s\n", smp_processor_id(), __func__);
}

/*
 * Setup the PC, SP, and GP of a secondary processor and start it running
 * smp_bootstrap is the place to resume from
 * __KSTK_TOS(idle) is apparently the stack pointer
 * (unsigned long)idle->thread_info the gp
 */
static void cmp_boot_secondary(int cpu, struct task_struct *idle)
{
	struct thread_info *gp = task_thread_info(idle);
	unsigned long sp = __KSTK_TOS(idle);
	unsigned long pc = (unsigned long)&smp_bootstrap;
	unsigned long a0 = 0;

	pr_debug("SMPCMP: CPU%d: %s cpu %d\n", smp_processor_id(),
		__func__, cpu);

#if 0
	/* Needed? */
	local_flush_icache_range((unsigned long)gp,
			   (unsigned long)(gp + sizeof(struct thread_info)));
#endif

	amon_cpu_start(cpu, pc, sp, (unsigned long)gp, a0);
}

/*
 * Common setup before any secondaries are started
 */
void __init cmp_smp_setup(void)
{
	int i;
	int ncpu = 0;

	pr_debug("SMPCMP: CPU%d: %s\n", smp_processor_id(), __func__);

#ifdef CONFIG_MIPS_MT_FPAFF
	/* If we have an FPU, enroll ourselves in the FPU-full mask */
	if (cpu_has_fpu)
		cpu_set(0, mt_fpu_cpumask);
#endif /* CONFIG_MIPS_MT_FPAFF */

	for (i = 1; i < NR_CPUS; i++) {
		if (amon_cpu_avail(i)) {
			set_cpu_possible(i, true);
			__cpu_number_map[i]	= ++ncpu;
			__cpu_logical_map[ncpu] = i;
		}
	}

	if (cpu_has_mipsmt) {
		unsigned int nvpe = 1;
#ifdef CONFIG_MIPS_MT_SMP
		unsigned int mvpconf0 = read_c0_mvpconf0();

		nvpe = ((mvpconf0 & MVPCONF0_PVPE) >> MVPCONF0_PVPE_SHIFT) + 1;
#elif defined(CONFIG_MIPS_MT_SMTC)
		unsigned int mvpconf0 = read_c0_mvpconf0();

		nvpe = ((mvpconf0 & MVPCONF0_PTC) >> MVPCONF0_PTC_SHIFT) + 1;
#endif
		smp_num_siblings = nvpe;
	}
	pr_info("Detected %i available secondary CPU(s)\n", ncpu);
}

void __init cmp_prepare_cpus(unsigned int max_cpus)
{
	pr_debug("SMPCMP: CPU%d: %s max_cpus=%d\n",
		 smp_processor_id(), __func__, max_cpus);

	/*
	 * FIXME: some of these options are per-system, some per-core and
	 * some per-cpu
	 */
	mips_mt_set_cpuoptions();
}

struct plat_smp_ops cmp_smp_ops = {
	.send_ipi_single	= cmp_send_ipi_single,
	.send_ipi_mask		= cmp_send_ipi_mask,
	.init_secondary		= cmp_init_secondary,
	.smp_finish		= cmp_smp_finish,
	.cpus_done		= cmp_cpus_done,
	.boot_secondary		= cmp_boot_secondary,
	.smp_setup		= cmp_smp_setup,
	.prepare_cpus		= cmp_prepare_cpus,
};
