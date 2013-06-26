/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1996, 99, 2003 by Ralf Baechle
 */
#ifndef _ASM_BYTEORDER_H
#define _ASM_BYTEORDER_H

#ifdef CONFIG_CPU_BIG_ENDIAN
#include <linux/byteorder/big_endian.h>
#else
#include <linux/byteorder/little_endian.h>
#endif

#endif /* _ASM_BYTEORDER_H */
