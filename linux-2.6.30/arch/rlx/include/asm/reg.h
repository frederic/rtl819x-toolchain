/*
 * Various register offset definitions for debuggers, core file
 * examiners and whatnot.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1995, 1999 Ralf Baechle
 * Copyright (C) 1995, 1999 Silicon Graphics
 */
#ifndef __ASM_MIPS_REG_H
#define __ASM_MIPS_REG_H


#define EF_R0			6
#define EF_R1			7
#define EF_R2			8
#define EF_R3			9
#define EF_R4			10
#define EF_R5			11
#define EF_R6			12
#define EF_R7			13
#define EF_R8			14
#define EF_R9			15
#define EF_R10			16
#define EF_R11			17
#define EF_R12			18
#define EF_R13			19
#define EF_R14			20
#define EF_R15			21
#define EF_R16			22
#define EF_R17			23
#define EF_R18			24
#define EF_R19			25
#define EF_R20			26
#define EF_R21			27
#define EF_R22			28
#define EF_R23			29
#define EF_R24			30
#define EF_R25			31

/*
 * k0/k1 unsaved
 */
#define EF_R26			32
#define EF_R27			33

#define EF_R28			34
#define EF_R29			35
#define EF_R30			36
#define EF_R31			37

/*
 * Saved special registers
 */
#define EF_LO			38
#define EF_HI			39

#define EF_CP0_EPC		40
#define EF_CP0_BADVADDR		41
#define EF_CP0_STATUS		42
#define EF_CP0_CAUSE		43
#define EF_UNUSED0		44

#define EF_SIZE			180

#endif /* __ASM_MIPS_REG_H */
