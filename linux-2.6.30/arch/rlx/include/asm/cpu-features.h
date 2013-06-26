/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2003, 2004 Ralf Baechle
 * Copyright (C) 2004  Maciej W. Rozycki
 */
#ifndef __ASM_CPU_FEATURES_H
#define __ASM_CPU_FEATURES_H

#include <asm/cpu.h>
#include <asm/cpu-info.h>
#include <bspcpu.h>

#ifndef current_cpu_type
#define current_cpu_type()      current_cpu_data.cputype
#endif

#define cpu_has_tlb               1
#define cpu_has_3k_cache	      1
#define cpu_has_mips16            1

#ifdef CONFIG_CPU_HAS_EJTAG
#define cpu_has_ejtag		      1
#else
#define cpu_has_ejtag		      0
#endif

#ifdef CONFIG_CPU_HAS_LLSC
#define cpu_has_llsc	      	  1
#else
#define cpu_has_llsc		      0
#endif

#ifndef cpu_dcache_line_size
#define cpu_dcache_line_size()	cpu_dcache_line
#endif
#ifndef cpu_icache_line_size
#define cpu_icache_line_size()	cpu_icache_line
#endif
#ifndef cpu_scache_line_size
#define cpu_scache_line_size()	cpu_scache_line
#endif

#ifndef cpu_scache_line
#define cpu_scache_line 0
#endif

#ifndef cpu_dcache_line
#define cpu_dcache_line 0
#endif

#ifndef cpu_icache_line
#define cpu_icache_line 0
#endif

#ifndef cpu_scache_size
#define cpu_scache_size 0
#endif

#ifndef cpu_dcache_size
#define cpu_dcache_size 0
#endif

#ifndef cpu_icache_size
#define cpu_icache_size 0
#endif

#ifndef cpu_tlb_entry
#define cpu_tlb_entry 0
#endif

/* tonywu: below does not matter, should set to zero except icache snoop */
#define cpu_has_4kex             0
#define cpu_has_4k_cache         0
#define cpu_has_6k_cache         0
#define cpu_has_8k_cache         0
#define cpu_has_tx39_cache       0
#define cpu_has_octeon_cache     0
#define cpu_has_fpu              0
#define raw_cpu_has_fpu          0
#define cpu_has_32fpr            0
#define cpu_has_counter		     0
#define cpu_has_watch	     	 0
#define cpu_has_divec	     	 0
#define cpu_has_vce              0	
#define cpu_has_cache_cdex_p     0
#define cpu_has_cache_cdex_s     0
#define cpu_has_prefetch         0
#define cpu_has_mcheck           0
#define cpu_has_mdmx             0
#define cpu_has_mips3d           0
#define cpu_has_smartmips        0
#define cpu_has_vtag_icache	     0
#define cpu_has_dc_aliases	     0
#define cpu_has_ic_fills_f_dc	 0
#define cpu_has_pindexed_dcache	 0

/*
 * I-Cache snoops remote store.  This only matters on SMP.  Some multiprocessors
 * such as the R10000 have I-Caches that snoop local stores; the embedded ones
 * don't.  For maintaining I-cache coherency this means we need to flush the
 * D-cache all the way back to whever the I-cache does refills from, so the
 * I-cache has a chance to see the new data at all.  Then we have to flush the
 * I-cache also.
 * Note we may have been rescheduled and may no longer be running on the CPU
 * that did the store so we can't optimize this into only doing the flush on
 * the local CPU.
 */
#define cpu_icache_snoops_remote_store	1
#define cpu_has_mips32r1	0
#define cpu_has_mips32r2	0
#define cpu_has_mips64r1	0
#define cpu_has_mips64r2	0

/*
 * Shortcuts ...
 */
#define cpu_has_mips32	(cpu_has_mips32r1 | cpu_has_mips32r2)
#define cpu_has_mips64	(cpu_has_mips64r1 | cpu_has_mips64r2)
#define cpu_has_mips_r1	(cpu_has_mips32r1 | cpu_has_mips64r1)
#define cpu_has_mips_r2	(cpu_has_mips32r2 | cpu_has_mips64r2)
#define cpu_has_mips_r	(cpu_has_mips32r1 | cpu_has_mips32r2 | \
			 cpu_has_mips64r1 | cpu_has_mips64r2)

/*
 * MIPS32, MIPS64, VR5500, IDT32332, IDT32334 and maybe a few other
 * pre-MIPS32/MIPS53 processors have CLO, CLZ.  For 64-bit kernels
 * cpu_has_clo_clz also indicates the availability of DCLO and DCLZ.
 */
#define cpu_has_clo_clz	           0
#define cpu_has_dsp		           0
#define cpu_has_mipsmt             0
#define cpu_has_userlocal          0

#define cpu_has_nofpuex	           0
#define cpu_has_64bits             0 
#define cpu_has_64bit_zero_reg     0
#define cpu_has_64bit_gp_regs      0
#define cpu_has_64bit_addresses	   0

#define cpu_has_vint               0
#define cpu_has_veic               0
#define cpu_has_inclusive_pcaches  0

#endif /* __ASM_CPU_FEATURES_H */
