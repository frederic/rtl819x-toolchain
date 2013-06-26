/*
 * r2300_switch.S: R2300 specific task switching code.
 *
 * Copyright (C) 1994, 1995, 1996, 1999 by Ralf Baechle
 * Copyright (C) 1994, 1995, 1996 by Andreas Busse
 *
 * Multi-cpu abstraction and macros for easier reading:
 * Copyright (C) 1996 David S. Miller (dm@engr.sgi.com)
 *
 * Further modifications to make this work:
 * Copyright (c) 1998-2000 Harald Koerfgen
 */
#include <asm/asm.h>
#include <asm/cachectl.h>
#include <asm/rlxregs.h>
#include <asm/asm-offsets.h>
#include <asm/page.h>
#include <asm/regdef.h>
#include <asm/stackframe.h>
#include <asm/thread_info.h>

#include <asm/asmmacro.h>

	.set	mips1
	.align	5

/*
 * Offset to the current process status flags, the first 32 bytes of the
 * stack are not used.
 */
#define ST_OFF (_THREAD_SIZE - 32 - PT_SIZE + PT_STATUS)

/*
 * task_struct *resume(task_struct *prev, task_struct *next,
 *                     struct thread_info *next_ti) )
 */
LEAF(resume)
#ifndef CONFIG_CPU_HAS_LLSC
	sw      zero, ll_bit
#endif
	mfc0	t1, CP0_STATUS
	sw	t1, THREAD_STATUS(a0)
	cpu_save_nonscratch a0
	sw	ra, THREAD_REG31(a0)

	/*
	 * The order of restoring the registers takes care of the race
	 * updating $28, $29 and kernelsp without disabling ints.
	 */
	move	$28, a2
	cpu_restore_nonscratch a1

	addiu	t1, $28, _THREAD_SIZE - 32
	sw	t1, kernelsp

	mfc0	t1, CP0_STATUS		/* Do we really need this? */
	li	a3, 0xff01
	and	t1, a3
	lw	a2, THREAD_STATUS(a1)
	nor	a3, $0, a3
	and	a2, a3
	or	a2, t1
	mtc0	a2, CP0_STATUS
	move	v0, a0
	jr	ra
	END(resume)
