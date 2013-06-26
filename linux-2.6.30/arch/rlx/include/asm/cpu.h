/*
 * cpu.h: Values of the PRId register used to match up
 *        various MIPS cpu types.
 *
 * Copyright (C) 1996 David S. Miller (dm@engr.sgi.com)
 * Copyright (C) 2004  Maciej W. Rozycki
 */
#ifndef _ASM_CPU_H
#define _ASM_CPU_H

/* Assigned Company values for bits 23:16 of the PRId Register
   (CP0 register 15, select 0).  As of the MIPS32 and MIPS64 specs from
   MTI, the PRId register is defined in this (backwards compatible)
   way:

  +----------------+----------------+----------------+----------------+
  | Company Options| Company ID     | Processor ID   | Revision       |
  +----------------+----------------+----------------+----------------+
   31            24 23            16 15             8 7

   I don't have docs for all the previous processors, but my impression is
   that bits 16-23 have been 0 for all MIPS processors before the MIPS32/64
   spec.
*/

#define PRID_IMP_RLX4180   0xc100
#define PRID_IMP_RLX4181   0xcd00
#define PRID_IMP_RLX5181   0xcf00
#define PRID_IMP_RLX5280   0xc600
#define PRID_IMP_RLX5281   0xdc01
#define PRID_IMP_RLX4281   0xdc02

#define PRID_IMP_UNKNOWN   0xff00

/*
 * FPU implementation/revision register (CP1 control register 0).
 *
 * +---------------------------------+----------------+----------------+
 * | 0                               | Implementation | Revision       |
 * +---------------------------------+----------------+----------------+
 *  31                             16 15             8 7              0
 */

#define FPIR_IMP_NONE		0x0000

#define CPU_UNKNOWN         0
#define CPU_RLX4180         1
#define CPU_RLX4181         2
#define CPU_RLX5181         3
#define CPU_RLX5280         4
#define CPU_RLX5281         5
#define CPU_RLX4281         6
#define CPU_LAST            6

#if 0
enum cpu_type_enum {
	CPU_UNKNOWN,
	CPU_RLX4180,
	CPU_RLX4181,
	CPU_RLX5181,
	CPU_RLX5280,
	CPU_RLX5281,
	CPU_RLX4281,
	CPU_LAST
};
#endif

/*
 * ISA Level encodings
 *
 */
#define MIPS_CPU_ISA_I		0x00000001
#define MIPS_CPU_ISA_II		0x00000002
#define MIPS_CPU_ISA_III	0x00000004
#define MIPS_CPU_ISA_IV		0x00000008
#define MIPS_CPU_ISA_V		0x00000010
#define MIPS_CPU_ISA_M32R1	0x00000020
#define MIPS_CPU_ISA_M32R2	0x00000040
#define MIPS_CPU_ISA_M64R1	0x00000080
#define MIPS_CPU_ISA_M64R2	0x00000100

#define MIPS_CPU_ISA_32BIT (MIPS_CPU_ISA_I | MIPS_CPU_ISA_II | \
	MIPS_CPU_ISA_M32R1 | MIPS_CPU_ISA_M32R2 )
#define MIPS_CPU_ISA_64BIT (MIPS_CPU_ISA_III | MIPS_CPU_ISA_IV | \
	MIPS_CPU_ISA_V | MIPS_CPU_ISA_M64R1 | MIPS_CPU_ISA_M64R2)

/*
 * CPU Option encodings
 */
#define MIPS_CPU_TLB		0x00000001 /* CPU has TLB */
#define MIPS_CPU_4KEX		0x00000002 /* "R4K" exception model */
#define MIPS_CPU_3K_CACHE	0x00000004 /* R3000-style caches */
#define MIPS_CPU_4K_CACHE	0x00000008 /* R4000-style caches */
#define MIPS_CPU_TX39_CACHE	0x00000010 /* TX3900-style caches */
#define MIPS_CPU_FPU		0x00000020 /* CPU has FPU */
#define MIPS_CPU_32FPR		0x00000040 /* 32 dbl. prec. FP registers */
#define MIPS_CPU_COUNTER	0x00000080 /* Cycle count/compare */
#define MIPS_CPU_WATCH		0x00000100 /* watchpoint registers */
#define MIPS_CPU_DIVEC		0x00000200 /* dedicated interrupt vector */
#define MIPS_CPU_VCE		0x00000400 /* virt. coherence conflict possible */
#define MIPS_CPU_CACHE_CDEX_P	0x00000800 /* Create_Dirty_Exclusive CACHE op */
#define MIPS_CPU_CACHE_CDEX_S	0x00001000 /* ... same for seconary cache ... */
#define MIPS_CPU_MCHECK		0x00002000 /* Machine check exception */
#define MIPS_CPU_EJTAG		0x00004000 /* EJTAG exception */
#define MIPS_CPU_NOFPUEX	0x00008000 /* no FPU exception */
#define MIPS_CPU_LLSC		0x00010000 /* CPU has ll/sc instructions */
#define MIPS_CPU_INCLUSIVE_CACHES	0x00020000 /* P-cache subset enforced */
#define MIPS_CPU_PREFETCH	0x00040000 /* CPU has usable prefetch */
#define MIPS_CPU_VINT		0x00080000 /* CPU supports MIPSR2 vectored interrupts */
#define MIPS_CPU_VEIC		0x00100000 /* CPU supports MIPSR2 external interrupt controller mode */
#define MIPS_CPU_ULRI		0x00200000 /* CPU has ULRI feature */

/*
 * CPU ASE encodings
 */
#define MIPS_ASE_MIPS16		0x00000001 /* code compression */
#define MIPS_ASE_MDMX		0x00000002 /* MIPS digital media extension */
#define MIPS_ASE_MIPS3D		0x00000004 /* MIPS-3D */
#define MIPS_ASE_SMARTMIPS	0x00000008 /* SmartMIPS */
#define MIPS_ASE_DSP		0x00000010 /* Signal Processing ASE */
#define MIPS_ASE_MIPSMT		0x00000020 /* CPU supports MIPS MT */


#endif /* _ASM_CPU_H */
