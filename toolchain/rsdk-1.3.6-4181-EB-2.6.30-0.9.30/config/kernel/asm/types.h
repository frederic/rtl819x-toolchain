/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1994, 1995, 1996, 1999 by Ralf Baechle
 * Copyright (C) 2008 Wind River Systems,
 *   written by Ralf Baechle
 * Copyright (C) 1999 Silicon Graphics, Inc.
 */
#ifndef _ASM_TYPES_H
#define _ASM_TYPES_H

/*
 * We don't use int-l64.h for the kernel anymore but still use it for
 * userspace to avoid code changes.
 */
#include <asm-generic/int-ll64.h>

#ifndef __ASSEMBLY__

typedef unsigned short umode_t;

#endif /* __ASSEMBLY__ */

/*
 * These aren't exported outside the kernel to avoid name space clashes
 */
#ifdef __KERNEL__

//#define BITS_PER_LONG _MIPS_SZLONG

#ifndef __ASSEMBLY__

typedef u32 dma_addr_t;
typedef u64 dma64_addr_t;

/*
 * Don't use phys_t.  You've been warned.
 */
typedef unsigned long phys_t;

#endif /* __ASSEMBLY__ */

#endif /* __KERNEL__ */

#endif /* _ASM_TYPES_H */
