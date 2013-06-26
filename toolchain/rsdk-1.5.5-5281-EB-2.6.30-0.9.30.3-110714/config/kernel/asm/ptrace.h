/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1994, 95, 96, 97, 98, 99, 2000 by Ralf Baechle
 * Copyright (C) 1999, 2000 Silicon Graphics, Inc.
 */
#ifndef _ASM_PTRACE_H
#define _ASM_PTRACE_H

/* 0 - 31 are integer registers, 32 - 63 are fp registers.  */
#define MMHI		32
#define MMLO		33
#define PC		    34
#define BADVADDR	35
#define STATUS      36
#define CAUSE		37

/*
 * This struct defines the way the registers are stored on the stack during a
 * system call/exception. As usual the registers k0/k1 aren't being saved.
 */
struct pt_regs {
	/* Pad bytes for argument save space on the stack. */
	unsigned long pad0[6];

	/* Saved main processor registers. */
	unsigned long regs[32];

	/* Saved special registers. */
	unsigned long hi;
	unsigned long lo;
	unsigned long cp0_epc;
	unsigned long cp0_badvaddr;
	unsigned long cp0_status;
	unsigned long cp0_cause;

    unsigned long estatus;
    unsigned long ecause;
    unsigned long intvec;
#ifdef CONFIG_CPU_HAS_RADIAX
    unsigned long radiax[27];
#endif

} __attribute__ ((aligned (8)));

/* Arbitrarily choose the same ptrace numbers as used by the Sparc code. */
#define PTRACE_GETREGS		12
#define PTRACE_SETREGS		13

#define PTRACE_OLDSETOPTIONS	21

#define PTRACE_GET_THREAD_AREA	25
#define PTRACE_SET_THREAD_AREA	26

/* Calls to trace a 64bit program from a 32bit program.  */
#define PTRACE_PEEKTEXT_3264	0xc0
#define PTRACE_PEEKDATA_3264	0xc1
#define PTRACE_POKETEXT_3264	0xc2
#define PTRACE_POKEDATA_3264	0xc3
#define PTRACE_GET_THREAD_AREA_3264	0xc4


#endif /* _ASM_PTRACE_H */
