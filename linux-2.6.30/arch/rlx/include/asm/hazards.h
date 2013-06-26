/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2003, 04, 07 Ralf Baechle <ralf@linux-mips.org>
 * Copyright (C) MIPS Technologies, Inc.
 *   written by Ralf Baechle <ralf@linux-mips.org>
 */
#ifndef _ASM_HAZARDS_H
#define _ASM_HAZARDS_H

#ifdef __ASSEMBLY__
#define ASMMACRO(name, code...) .macro name; code; .endm
#else

#include <asm/cpu-features.h>

#define ASMMACRO(name, code...)						\
__asm__(".macro " #name "; " #code "; .endm");				\
									\
static inline void name(void)						\
{									\
	__asm__ __volatile__ (#name);					\
}

#endif

ASMMACRO(_ssnop,
	 sll	$0, $0, 1
	)

ASMMACRO(_ehb,
	 sll	$0, $0, 3
	)

/*
 * Finally the catchall case for all other processors including R4000, R4400,
 * R4600, R4700, R5000, RM7000, NEC VR41xx etc.
 *
 * The taken branch will result in a two cycle penalty for the two killed
 * instructions on R4000 / R4400.  Other processors only have a single cycle
 * hazard so this is nice trick to have an optimal code for a range of
 * processors.
 */
ASMMACRO(mtc0_tlbw_hazard,
	nop; nop
	)
ASMMACRO(tlbw_use_hazard,
	nop; nop; nop
	)
ASMMACRO(tlb_probe_hazard,
	 nop; nop; nop
	)
ASMMACRO(irq_enable_hazard,
	 _ssnop; _ssnop; _ssnop;
	)
ASMMACRO(irq_disable_hazard,
	nop; nop; nop
	)
ASMMACRO(back_to_back_c0_hazard,
	 _ssnop; _ssnop; _ssnop;
	)
#define instruction_hazard() do { } while (0)

#endif /* _ASM_HAZARDS_H */
