/*
 * Processor capabilities determination functions.
 *
 * Copyright (C) xxxx  the Anonymous
 * Copyright (C) 1994 - 2006 Ralf Baechle
 * Copyright (C) 2003, 2004  Maciej W. Rozycki
 * Copyright (C) 2001, 2004  MIPS Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/ptrace.h>
#include <linux/stddef.h>

#include <asm/cpu.h>
#include <asm/rlxregs.h>
#include <asm/system.h>

const char *__cpu_name[NR_CPUS];

void __cpuinit cpu_probe(void)
{
	struct cpuinfo_mips *c = &current_cpu_data;

    c->processor_id = PRID_IMP_UNKNOWN;
    c->options = MIPS_CPU_TLB | MIPS_CPU_3K_CACHE | MIPS_CPU_NOFPUEX;
    c->tlbsize = cpu_tlb_entry;  /* defined in bspcpu.h */
    c->processor_id = read_c0_prid();
}

void __cpuinit cpu_report(void)
{
	struct cpuinfo_mips *c = &current_cpu_data;

    printk("CPU revision is: %08x\n", c->processor_id);
}
