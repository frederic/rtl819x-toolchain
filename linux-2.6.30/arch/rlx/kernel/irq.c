/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Code to handle x86 style IRQs plus some generic interrupt stuff.
 *
 * Copyright (C) 1992 Linus Torvalds
 * Copyright (C) 1994 - 2000 Ralf Baechle
 */
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel_stat.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/random.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/kallsyms.h>
#include <linux/kgdb.h>

#include <asm/atomic.h>
#include <asm/system.h>
#include <asm/uaccess.h>

#ifdef CONFIG_KGDB
int kgdb_early_setup;
#endif

static unsigned long irq_map[NR_IRQS / BITS_PER_LONG];

int allocate_irqno(void)
{
	int irq;

again:
	irq = find_first_zero_bit(irq_map, NR_IRQS);

	if (irq >= NR_IRQS)
		return -ENOSPC;

	if (test_and_set_bit(irq, irq_map))
		goto again;

	return irq;
}

/*
 * Allocate the 16 legacy interrupts for i8259 devices.  This happens early
 * in the kernel initialization so treating allocation failure as BUG() is
 * ok.
 */
void __init alloc_legacy_irqno(void)
{
	int i;

	for (i = 0; i <= 16; i++)
		BUG_ON(test_and_set_bit(i, irq_map));
}

void free_irqno(unsigned int irq)
{
	smp_mb__before_clear_bit();
	clear_bit(irq, irq_map);
	smp_mb__after_clear_bit();
}

/*
 * 'what should we do if we get a hw irq event on an illegal vector'.
 * each architecture has to answer this themselves.
 */
void ack_bad_irq(unsigned int irq)
{
	printk("unexpected IRQ # %d\n", irq);
}

//by rock 2010-04-06
#if defined(CONFIG_RTK_VOIP) || defined(CONFIG_RTL_819X)
static atomic_t irq_err_count[SPURIOS_INT_MAX];
#else
static atomic_t irq_err_count;
#endif

#ifdef CONFIG_CPU_RLX5281

static void get_int_counts(u32 *hw_int, u32 *vec_int)
{
	// enable hardware, vercor interrupts 
	u32 mode = (0x2e << 0) | (0x2f << 8); 
	u64 cnt[4];

	__asm__ __volatile__ (
		// enable cp3
		"mfc0	$8, $12			\n\t"
		"nop					\n\t"
		"nop					\n\t"
		"or		$8, 0x80000000	\n\t"
		"mtc0	$8, $12			\n\t"
		"nop					\n\t"
		"nop					\n\t"
		// stop counter
		"ctc3 $0,$0				\n\t"
		"nop					\n\t"
		"nop					\n\t"
		// save counter
		"mfc3 %L[cnt0],$8		\n\t"
		"mfc3 %M[cnt0],$9		\n\t"
		"mfc3 %L[cnt1],$10		\n\t"
		"mfc3 %M[cnt1],$11		\n\t"
		"mfc3 %L[cnt2],$12		\n\t"
		"mfc3 %M[cnt2],$13		\n\t"
		"mfc3 %L[cnt3],$14		\n\t"
		"mfc3 %M[cnt3],$15		\n\t"
		"nop					\n\t"
		"nop					\n\t"
		// start counter
		"ctc3 %[mode],$0		\n\t"
		"nop					\n\t"
		"nop					\n\t"
		:[cnt0] "=&r" (cnt[0]), [cnt1] "=&r" (cnt[1]), [cnt2] "=&r" (cnt[2]), [cnt3] "=&r" (cnt[3])
		:[mode] "r" (mode)
	);

	*hw_int = (u32) cnt[0];
	*vec_int = (u32) cnt[1];
}

#endif

/*
 * Generic, controller-independent functions:
 */

int show_interrupts(struct seq_file *p, void *v)
{
	int i = *(loff_t *) v, j;
	struct irqaction * action;
	unsigned long flags;
	struct irq_desc *rtl_irq_desc;

	if (i == 0) {
		seq_printf(p, "           ");
		for_each_online_cpu(j)
			seq_printf(p, "CPU%d       ", j);
		seq_putc(p, '\n');
	}

	if (i < NR_IRQS) {
		rtl_irq_desc = irq_to_desc(i);
		spin_lock_irqsave(rtl_irq_desc->lock, flags);
		action = rtl_irq_desc->action;
		if (!action)
			goto skip;
		seq_printf(p, "%3d: ", i);
		seq_printf(p, "%10u ", kstat_irqs(i));
		seq_printf(p, " %14s", rtl_irq_desc->chip->name);
		seq_printf(p, "  %s (0x%lx)", action->name, action->flags);

		for (action=action->next; action; action = action->next)
			seq_printf(p, ", %s (0x%lx)", action->name, action->flags);

		seq_putc(p, '\n');
skip:
		spin_unlock_irqrestore(rtl_irq_desc->lock, flags);
	} else if (i == NR_IRQS) {
#ifdef CONFIG_CPU_RLX5281
		u32 hw_int, vec_int;
		static int first_dump = 1;
		static int hw_int_off, vec_int_off;
#endif
#ifdef CONFIG_RTL_819X_SWCORE
		extern int cnt_swcore;
		extern int cnt_swcore_tx;
		extern int cnt_swcore_rx;
		extern int cnt_swcore_link;
		extern int cnt_swcore_err;
#endif

		seq_printf(p, "\n");
#if defined(CONFIG_RTK_VOIP) || defined(CONFIG_RTL_819X)
		for (j=0; j<ARRAY_SIZE(irq_err_count); j++)
			seq_printf(p, "ER%d: %10u\n", j, atomic_read(&irq_err_count[j]));
#else
		seq_printf(p, "ERR: %10u\n", atomic_read(&irq_err_count)); 
#endif

#ifdef CONFIG_CPU_RLX5281
		get_int_counts(&hw_int, &vec_int);

		if (first_dump)
		{
			first_dump = 0;
			hw_int_off = 0;
			vec_int_off = 0;
			for (j=0; j<NR_IRQS; j++)
			{
				rtl_irq_desc = irq_to_desc(j);
				if (rtl_irq_desc->action == NULL)
					continue;

				if (strcmp(rtl_irq_desc->chip->name, "RLX") == 0)
					hw_int_off += kstat_irqs(j);
				else if (strcmp(rtl_irq_desc->chip->name, "RLX LOPI") == 0)
					vec_int_off += kstat_irqs(j);
				else if (strcmp(rtl_irq_desc->chip->name, "ICTL") == 0)
					hw_int_off += kstat_irqs(j);
			}

			// rock: use sw counter to calc hw counter offset on first time
			// (first dump info is sw info)
			hw_int_off -= hw_int;
			vec_int_off -= vec_int;
		}

		seq_printf(p, "\n HW: %10u\n", hw_int + hw_int_off);
		seq_printf(p, "VEC: %10u\n", vec_int + vec_int_off);
#endif

#ifdef CONFIG_RTL_819X_SWCORE
		seq_printf(p, "\n SW: %10u\n", cnt_swcore);
		seq_printf(p, " TX: %10u\n", cnt_swcore_tx);
		seq_printf(p, " RX: %10u\n", cnt_swcore_rx);
		seq_printf(p, "LNK: %10u\n", cnt_swcore_link);
		seq_printf(p, "ERR: %10u\n", cnt_swcore_err);
#endif
	}
	return 0;
}

#if defined(CONFIG_RTK_VOIP) || defined(CONFIG_RTL_819X)
asmlinkage void spurious_interrupt(int i)
{
	atomic_inc(&irq_err_count[i]);
}
#else
asmlinkage void spurious_interrupt()
{
	atomic_inc(&irq_err_count);
}
#endif

void __init init_IRQ(void)
{
    extern void bsp_irq_init(void);
	int i;

#ifdef CONFIG_KGDB
	if (kgdb_early_setup)
		return;
#endif

	for (i = 0; i < NR_IRQS; i++)
		set_irq_noprobe(i);

	bsp_irq_init();

#ifdef CONFIG_KGDB
	if (!kgdb_early_setup)
		kgdb_early_setup = 1;
#endif

#if defined(CONFIG_RTK_VOIP) || defined(CONFIG_RTL_819X)
	for (i=0;i<SPURIOS_INT_MAX;i++) {
		atomic_set(&irq_err_count[i], 0);
	}
#endif
}
