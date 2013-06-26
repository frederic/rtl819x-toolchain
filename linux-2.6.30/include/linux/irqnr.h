#ifndef _LINUX_IRQNR_H
#define _LINUX_IRQNR_H

/*
 * Generic irq_desc iterators:
 */
#ifdef __KERNEL__

#ifndef CONFIG_GENERIC_HARDIRQS
#include <asm/irq.h>
/*
 * Wrappers for non-genirq architectures:
 */
#define nr_irqs			NR_IRQS
#define irq_to_desc(irq)	(&irq_desc[irq])

# define for_each_irq_desc(irq, desc)		\
	for (irq = 0; irq < nr_irqs; irq++)

# define for_each_irq_desc_reverse(irq, desc)                          \
	for (irq = nr_irqs - 1; irq >= 0; irq--)

#else /* CONFIG_GENERIC_HARDIRQS */

extern int nr_irqs;
#if defined(CONFIG_RTL_819X) && !defined(CONFIG_RTL_8196C) && !defined(CONFIG_SPARSE_IRQ)
 //__MIPS16 is defined at include/net/rtl/rtl_types.h
 extern __attribute__((mips16))  struct irq_desc *irq_to_desc(unsigned int irq);
#else
 extern  struct irq_desc *irq_to_desc(unsigned int irq);
#endif

# define for_each_irq_desc(irq, desc)					\
	for (irq = 0, desc = irq_to_desc(irq); irq < nr_irqs;		\
	     irq++, desc = irq_to_desc(irq))				\
		if (!desc)						\
			;						\
		else


# define for_each_irq_desc_reverse(irq, desc)				\
	for (irq = nr_irqs - 1, desc = irq_to_desc(irq); irq >= 0;	\
	     irq--, desc = irq_to_desc(irq))				\
		if (!desc)						\
			;						\
		else

#endif /* CONFIG_GENERIC_HARDIRQS */

#define for_each_irq_nr(irq)                   \
       for (irq = 0; irq < nr_irqs; irq++)

#endif /* __KERNEL__ */

#endif
