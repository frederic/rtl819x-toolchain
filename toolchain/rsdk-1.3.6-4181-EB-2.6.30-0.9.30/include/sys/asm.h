/* Copyright (C) 1997, 1998, 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ralf Baechle <ralf@gnu.org>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _SYS_ASM_H
#define _SYS_ASM_H

#include <sgidefs.h>

#ifndef CAT
# ifdef __STDC__
#  define __CAT(str1,str2) str1##str2
# else
#  define __CAT(str1,str2) str1/**/str2
# endif
# define CAT(str1,str2) __CAT(str1,str2)
#endif

/*
 * Macros to handle different pointer/register sizes for 32/64-bit code
 *
 * 64 bit address space isn't used yet, so we may use the R3000 32 bit
 * defines for now.
 */
# define PTR .word
# define PTRSIZE 4
# define PTRLOG 2

/*
 * PIC specific declarations
 */
# ifdef __PIC__
#  define CPRESTORE(register) \
		.cprestore register
#  define CPLOAD(register) \
		.cpload register
# else
#  define CPRESTORE(register)
#  define CPLOAD(register)
# endif

# define CPADD(register) \
		.cpadd	register

/*
 * Set gp when at 1st instruction
 */
# define SETUP_GP					\
		.set noreorder;				\
		.cpload $25;				\
		.set reorder
/* Set gp when not at 1st instruction */
# define SETUP_GPX(r)					\
		.set noreorder;				\
		move r, $31;	 /* Save old ra.  */	\
		bal 10f; /* Find addr of cpload.  */	\
		nop;					\
10:							\
		.cpload $31;				\
		move $31, r;				\
		.set reorder
# define SETUP_GPX_L(r, l)				\
		.set noreorder;				\
		move r, $31;	 /* Save old ra.  */	\
		bal l;   /* Find addr of cpload.  */	\
		nop;					\
l:							\
		.cpload $31;				\
		move $31, r;				\
		.set reorder
# define SAVE_GP(x) \
		.cprestore x /* Save gp trigger t9/jalr conversion.	 */
# define SETUP_GP64(a, b)
# define SETUP_GPX64(a, b)
# define SETUP_GPX64_L(cp_reg, ra_save, l)
# define RESTORE_GP64
# define USE_ALT_CP(a)

/*
 * Stack Frame Definitions
 */
# define NARGSAVE 4 /* Space for 4 argument registers must be allocated.  */


/*
 * LEAF - declare leaf routine
 */
#define	LEAF(symbol)                                    \
		.globl	symbol;                         \
		.align	2;                              \
		.type	symbol,@function;               \
		.ent	symbol,0;                       \
symbol:		.frame	sp,0,ra

/*
 * NESTED - declare nested routine entry point
 */
#define	NESTED(symbol, framesize, rpc)                  \
		.globl	symbol;                         \
		.align	2;                              \
		.type	symbol,@function;               \
		.ent	symbol,0;                       \
symbol:		.frame	sp, framesize, rpc

/*
 * END - mark end of function
 */
#ifndef END
# define END(function)                                   \
		.end	function;		        \
		.size	function,.-function
#endif

/*
 * EXPORT - export definition of symbol
 */
#define	EXPORT(symbol)                                  \
		.globl	symbol;                         \
symbol:

/*
 * ABS - export absolute symbol
 */
#define	ABS(symbol,value)                               \
		.globl	symbol;                         \
symbol		=	value

#define	PANIC(msg)                                      \
		.set	push;				\
		.set	reorder;                        \
		la	a0,8f;                          \
		jal	panic;                          \
9:		b	9b;                             \
		.set	pop;				\
		TEXT(msg)

/*
 * Print formated string
 */
#define PRINT(string)                                   \
		.set	push;				\
		.set	reorder;                        \
		la	a0,8f;                          \
		jal	printk;                         \
		.set	pop;				\
		TEXT(string)

#define	TEXT(msg)                                       \
		.data;                                  \
8:		.asciiz	msg;                            \
		.previous;

/*
 * Build text tables
 */
#define TTABLE(string)                                  \
		.text;                                  \
		.word	1f;                             \
		.previous;                              \
		.data;                                  \
1:		.asciz	string;                         \
		.previous

/*
 * MIPS IV pref instruction.
 * Use with .set noreorder only!
 *
 * MIPS IV implementations are free to treat this as a nop.  The R5000
 * is one of them.  So we should have an option not to use this instruction.
 */
# define PREF
# define PREFX

#if 0
# define MOVN(rd,rs,rt)					\
		.set	push;				\
		.set	reorder;			\
		beqz	rt,9f;				\
		move	rd,rs;				\
		.set	pop;				\
9:
# define MOVZ(rd,rs,rt)					\
		.set	push;				\
		.set	reorder;			\
		bnez	rt,9f;				\
		move	rd,rt;				\
		.set	pop;				\
9:
#endif

# define MOVN(rd,rs,rt)					\
		movn	rd,rs,rt
# define MOVZ(rd,rs,rt)					\
		movz	rd,rs,rt

/*
 * Stack alignment
 */
# define ALSZ	7
# define ALMASK	~7

/*
 * Size of a register
 */
# define SZREG	4

/*
 * Use the following macros in assemblercode to load/store registers,
 * pointers etc.
 */
# define REG_S sw
# define REG_L lw

/*
 * How to add/sub/load/store/shift C int variables.
 */
# define INT_ADD	add
# define INT_ADDI	addi
# define INT_ADDU	addu
# define INT_ADDIU	addiu
# define INT_SUB	add
# define INT_SUBI	subi
# define INT_SUBU	subu
# define INT_SUBIU	subu
# define INT_L		lw
# define INT_S		sw

/*
 * How to add/sub/load/store/shift C long variables.
 */
# define LONG_ADD	add
# define LONG_ADDI	addi
# define LONG_ADDU	addu
# define LONG_ADDIU	addiu
# define LONG_SUB	add
# define LONG_SUBI	subi
# define LONG_SUBU	subu
# define LONG_SUBIU	subu
# define LONG_L		lw
# define LONG_S		sw
# define LONG_SLL	sll
# define LONG_SLLV	sllv
# define LONG_SRL	srl
# define LONG_SRLV	srlv
# define LONG_SRA	sra
# define LONG_SRAV	srav

/*
 * How to add/sub/load/store/shift pointers.
 */
# define PTR_ADD	add
# define PTR_ADDI	addi
# define PTR_ADDU	addu
# define PTR_ADDIU	addiu
# define PTR_SUB	add
# define PTR_SUBI	subi
# define PTR_SUBU	subu
# define PTR_SUBIU	subu
# define PTR_L		lw
# define PTR_LA		la
# define PTR_S		sw
# define PTR_SLL	sll
# define PTR_SLLV	sllv
# define PTR_SRL	srl
# define PTR_SRLV	srlv
# define PTR_SRA	sra
# define PTR_SRAV	srav

# define PTR_SCALESHIFT	2

/*
 * Some cp0 registers were extended to 64bit for MIPS III.
 */
# define MFC0	mfc0
# define MTC0	mtc0

#endif /* sys/asm.h */
