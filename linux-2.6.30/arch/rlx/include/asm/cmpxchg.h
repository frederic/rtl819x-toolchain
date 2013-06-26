/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2003, 06, 07 by Ralf Baechle (ralf@linux-mips.org)
 */
#ifndef __ASM_CMPXCHG_H
#define __ASM_CMPXCHG_H

#include <linux/irqflags.h>

#define __HAVE_ARCH_CMPXCHG 1

#ifdef CONFIG_CPU_HAS_LLSC
#if defined(CONFIG_CPU_RLX4181) || defined(CONFIG_CPU_RLX5181) || defined(CONFIG_CPU_RLX5281)
#define __cmpxchg_asm(ld, st, m, old, new)                              \
({                                                                      \
        __typeof(*(m)) __ret;                                           \
                                                                        \
                __asm__ __volatile__(                                   \
                "       .set    push                            \n"     \
                "       .set    noat                            \n"     \
                "1:     " ld "  %0, %2          # __cmpxchg_asm \n"     \
		"	nop					\n"	\
                "       bne     %0, %z3, 2f                     \n"     \
                "       move    $1, %z4                         \n"     \
                "       " st "  $1, %1                          \n"     \
                "       beqz    $1, 3f                          \n"     \
                "2:                                             \n"     \
                "       .subsection 2                           \n"     \
                "3:     b       1b                              \n"     \
                "       .previous                               \n"     \
                "       .set    pop                             \n"     \
                : "=&r" (__ret), "=R" (*m)                              \
                : "R" (*m), "Jr" (old), "Jr" (new)                      \
                : "memory");                                            \
        __ret;                                                          \
})
#else
#define __cmpxchg_asm(ld, st, m, old, new)				\
({									\
	__typeof(*(m)) __ret;						\
									\
		__asm__ __volatile__(					\
		"	.set	push				\n"	\
		"	.set	noat				\n"	\
		"1:	" ld "	%0, %2		# __cmpxchg_asm	\n"	\
		"	bne	%0, %z3, 2f			\n"	\
		"	move	$1, %z4				\n"	\
		"	" st "	$1, %1				\n"	\
		"	beqz	$1, 3f				\n"	\
		"2:						\n"	\
		"	.subsection 2				\n"	\
		"3:	b	1b				\n"	\
		"	.previous				\n"	\
		"	.set	pop				\n"	\
		: "=&r" (__ret), "=R" (*m)				\
		: "R" (*m), "Jr" (old), "Jr" (new)			\
		: "memory");						\
	__ret;								\
})
#endif
#else
#define __cmpxchg_asm(ld, st, m, old, new)				\
({									\
	__typeof(*(m)) __ret;						\
	unsigned long __flags;					\
									\
	raw_local_irq_save(__flags);				\
	__ret = *m;						\
	if (__ret == old)					\
		*m = new;					\
	raw_local_irq_restore(__flags);				\
									\
	__ret;								\
})
#endif

#define __cmpxchg(ptr, old, new, barrier)				\
({									\
	__typeof__(ptr) __ptr = (ptr);					\
	__typeof__(*(ptr)) __old = (old);				\
	__typeof__(*(ptr)) __new = (new);				\
	__typeof__(*(ptr)) __res = 0;					\
									\
	barrier;							\
									\
	__res = __cmpxchg_asm("ll", "sc", __ptr, __old, __new);	\
									\
	barrier;							\
									\
	__res;								\
})

#define cmpxchg(ptr, old, new)		__cmpxchg(ptr, old, new, smp_llsc_mb())
#define cmpxchg_local(ptr, old, new)	__cmpxchg(ptr, old, new, )

#if 0
#define cmpxchg64(ptr, o, n)						\
  ({									\
	BUILD_BUG_ON(sizeof(*(ptr)) != 8);				\
	cmpxchg((ptr), (o), (n));					\
  })

#include <asm-generic/cmpxchg-local.h>
#define cmpxchg64_local(ptr, o, n) __cmpxchg64_local_generic((ptr), (o), (n))
#endif

#endif /* __ASM_CMPXCHG_H */
