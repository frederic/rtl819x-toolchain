/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1994 by Waldorf GMBH, written by Ralf Baechle
 * Copyright (C) 1995, 96, 97, 98, 99, 2000, 01, 02, 03 by Ralf Baechle
 */
#ifndef _ASM_IRQ_H
#define _ASM_IRQ_H

#include <linux/linkage.h>
#include <linux/smp.h>
#include <irq.h>

#define irq_canonicalize(irq) (irq)	/* Sane hardware, sane code ... */

/*
 * do_IRQ handles all normal device IRQ's (the special
 * SMP cross-CPU interrupts have their own specific
 * handlers).
 *
 * Ideally there should be away to get this into kernel/irq/handle.c to
 * avoid the overhead of a call for just a tiny function ...
 */
#define do_IRQ(irq)							\
do {									\
	irq_enter();							\
	generic_handle_irq(irq);					\
	irq_exit();							\
} while (0)


// rock: add more detail spurious int info 2010-04-06
enum {
	SPURIOS_INT_CPU = 0,
	SPURIOS_INT_LOPI,
	SPURIOS_INT_CASCADE,
	SPURIOS_INT_MAX
};

extern void arch_init_irq(void);
#if defined(CONFIG_RTK_VOIP) || defined(CONFIG_RTL_819X)
extern void spurious_interrupt(int i); 
#else
extern void spurious_interrupt(void);
#endif

extern int allocate_irqno(void);
extern void alloc_legacy_irqno(void);
extern void free_irqno(unsigned int irq);

/*
 * Before R2 the timer and performance counter interrupts were both fixed to
 * IE7.  Since R2 their number has to be read from the c0_intctl register.
 */
#define CP0_LEGACY_COMPARE_IRQ 7

extern int cp0_compare_irq;
extern int cp0_perfcount_irq;

#endif /* _ASM_IRQ_H */
