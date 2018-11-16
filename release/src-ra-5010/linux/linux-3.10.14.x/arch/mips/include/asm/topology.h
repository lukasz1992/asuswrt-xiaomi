/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2007 by Ralf Baechle
 * Copyright (C) 2012 by Leonid Yegoshin
 */
#ifndef __ASM_TOPOLOGY_H
#define __ASM_TOPOLOGY_H

#include <topology.h>

#ifdef CONFIG_SMP
#define smt_capable()	(smp_num_siblings > 1)
#define topology_thread_cpumask(cpu)    (&per_cpu(cpu_sibling_map, cpu))
#define topology_core_id(cpu)           (cpu_data[cpu].core)
#define topology_core_cpumask(cpu)      ((void)(cpu), cpu_online_mask)
#define topology_physical_package_id(cpu)   ((void)cpu, 0)
#endif

#endif /* __ASM_TOPOLOGY_H */
