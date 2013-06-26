/*
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file "COPYING" in the main directory of this
 * archive for more details.
 *
 * Copyright (C) 2000 - 2001 by Kanoj Sarcar (kanoj@sgi.com)
 * Copyright (C) 2000 - 2001 by Silicon Graphics, Inc.
 * Copyright (C) 2000, 2001, 2002 Ralf Baechle
 * Copyright (C) 2000, 2001 Broadcom Corporation
 */
#ifndef __ASM_SMP_OPS_H
#define __ASM_SMP_OPS_H

struct plat_smp_ops;

static inline void register_smp_ops(struct plat_smp_ops *ops)
{
}

extern struct plat_smp_ops up_smp_ops;
extern struct plat_smp_ops cmp_smp_ops;
extern struct plat_smp_ops vsmp_smp_ops;

#endif /* __ASM_SMP_OPS_H */
