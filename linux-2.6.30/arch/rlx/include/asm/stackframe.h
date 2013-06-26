/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1994, 95, 96, 99, 2001 Ralf Baechle
 * Copyright (C) 1994, 1995, 1996 Paul M. Antoine.
 * Copyright (C) 1999 Silicon Graphics, Inc.
 */
#ifndef _ASM_STACKFRAME_H
#define _ASM_STACKFRAME_H

#include <linux/threads.h>

#include <asm/asm.h>
#include <asm/asmmacro.h>
#include <asm/rlxregs.h>
#include <asm/asm-offsets.h>

#define STATMASK 0x3f

		.macro	SAVE_AT
		.set	push
		.set	noat
		LONG_S	$1, PT_R1(sp)
		.set	pop
		.endm

		.macro	SAVE_TEMP
		mfhi	v1
		LONG_S	v1, PT_HI(sp)
		mflo	v1
		LONG_S	v1, PT_LO(sp)
		LONG_S	$8, PT_R8(sp)
		LONG_S	$9, PT_R9(sp)
		LONG_S	$10, PT_R10(sp)
		LONG_S	$11, PT_R11(sp)
		LONG_S	$12, PT_R12(sp)
		LONG_S	$13, PT_R13(sp)
		LONG_S	$14, PT_R14(sp)
		LONG_S	$15, PT_R15(sp)
		LONG_S	$24, PT_R24(sp)
		.endm

		.macro	SAVE_STATIC
		LONG_S	$16, PT_R16(sp)
		LONG_S	$17, PT_R17(sp)
		LONG_S	$18, PT_R18(sp)
		LONG_S	$19, PT_R19(sp)
		LONG_S	$20, PT_R20(sp)
		LONG_S	$21, PT_R21(sp)
		LONG_S	$22, PT_R22(sp)
		LONG_S	$23, PT_R23(sp)
		LONG_S	$30, PT_R30(sp)
		.endm

        /*
         * Support RADIAX save and restore
         */
#ifdef CONFIG_CPU_HAS_RADIAX
        .macro  SAVE_RADIAX
        .set push
        .set at
		mflxc0	$8,  $0
		mflxc0  $9,  $1
		mflxc0  $10, $2
		mfru    $11, $0
		mfru    $12, $1
		mfru    $13, $2
		sw      $8,  PT_ESTATUS(sp)
		sw      $9,  PT_ECAUSE(sp)
		sw      $10, PT_INTVEC(sp)
		sw      $11, PT_CBS0(sp)
		sw      $12, PT_CBS1(sp)
		sw      $13, PT_CBS2(sp)
		mfru 	$8,  $4
		mfru    $9,  $5
		mfru	$10, $6
		mfru    $11, $16
		mfru    $12, $17
		mfru    $13, $18
		mfru    $14, $24
		sw      $8,  PT_CBE0(sp)
		sw      $9,  PT_CBE1(sp)
		sw      $10, PT_CBE2(sp)
		sw      $11, PT_LPS0(sp)
		sw      $12, PT_LPE0(sp)
		sw      $13, PT_LPC0(sp)
		sw      $14, PT_MMD(sp)
		mfa     $8,  $1
		mfa     $9,  $1, 8
		mfa     $10, $2
		mfa     $11, $2, 8
		sw	    $8,  PT_M0LL(sp)
		srl     $12, $9,24
		sw	    $12, PT_M0LH(sp)
		sw	    $10, PT_M0HL(sp)
		srl     $12, $11,24
		sw	    $12, PT_M0HH(sp)
		mfa     $8,  $5
		mfa     $9,  $5,8
		mfa     $10, $6
		mfa     $11, $6,8
		sw	    $8,  PT_M1LL(sp)
		srl     $12, $9,24
		sw     	$12, PT_M1LH(sp)
		sw	    $10, PT_M1HL(sp)
		srl     $12, $11,24
		sw	    $12, PT_M1HH(sp)
		mfa     $8,  $9
		mfa     $9,  $9, 8
		mfa     $10, $10
		mfa     $11, $10, 8
		sw	    $8,  PT_M2LL(sp)
		srl     $12, $9, 24
		sw   	$12, PT_M2LH(sp)
		sw	    $10, PT_M2HL(sp)
		srl     $12, $11, 24
		sw  	$12, PT_M2HH(sp)
		mfa     $8,  $13
		mfa     $9,  $13, 8
		mfa     $10, $14
		mfa     $11, $14, 8
		sw	    $8,  PT_M3LL(sp)
		srl     $12, $9, 24
		sw	    $12, PT_M3LH(sp)
		sw	    $10, PT_M3HL(sp)
		srl     $12, $11, 24
		sw	    $12, PT_M3HH(sp)
        .set pop
        .endm

        .macro  RESTORE_RADIAX
        .set push
        .set at
    	lw      $8,  PT_ESTASUS(sp)
		lw      $9,  PT_ECAUSE(sp)
		lw      $10, PT_INTVEC(sp)
		lw      $11, PT_CBS0(sp)
		lw      $12, PT_CBS1(sp)
		lw      $13, PT_CBS2(sp)
		mtlxc0  $8,  $0
		mtlxc0  $10, $2
		mtru    $11, $0
		mtru    $12, $1
		mtru    $13, $2
		lw      $8,  PT_CBE0(sp)
		lw      $9,  PT_CBE1(sp)
		lw      $10, PT_CBE2(sp)
		lw      $11, PT_LPS0(sp)
		lw      $12, PT_LPE0(sp)
		lw      $13, PT_LPC0(sp)
		lw      $14, PT_MMD(sp)
		mtru    $8,  $4
		mtru    $9,  $5
		mtru    $10, $6
		mtru    $11, $16
		mtru    $12, $17
		mtru    $13, $18
		mtru    $14, $24
		lw	    $8,  PT_M0LL(sp)
		lw	    $9,  PT_M0LH(sp)
		lw      $10, PT_M0HL(sp)
		lw      $11, PT_M0HH(sp)
		mta2 	$8,  $1
		sll	    $12, $9, 24
		mta2.g	$12, $1
		mta2	$10, $2
		sll	    $12, $11, 24
		mta2.g	$12, $2
		lw      $8,  PT_M1LL(sp)
		lw      $9,  PT_M1LH(sp)
		lw      $10, PT_M1HL(sp)
		lw      $11, PT_M1HH(sp)
		mta2 	$8,  $5
		sll	    $12, $9, 24
		mta2.g	$12, $5
		mta2	$10, $6
		sll	    $12, $11, 24
		mta2.g	$12, $6
		lw	    $8,  PT_M2LL(sp)
		lw	    $9,  PT_M2LH(sp)
		lw	    $10, PT_M2HL(sp)
		lw	    $11, PT_M2HH(sp)
		mta2 	$8,  $9
		sll	    $12, $9, 24
		mta2.g	$12, $9
		mta2	$10, $10
		sll	    $12, $11, 24
		mta2.g	$12, $10
		lw	    $8,  PT_M3LL(sp)
		lw	    $9,  PT_M3LH(sp)
		lw	    $10, PT_M3HL(sp)
		lw	    $11, PT_M3HH(sp)	
		mta2 	$8,  $13
		sll	    $12, $9, 24
		mta2.g	$12, $13
		mta2	$10, $13
		sll	    $12, $11, 24
		mta2.g	$12, $13
        .set pop
        .endm
#endif

		.macro	get_saved_sp	/* Uniprocessor variation */
		lui	k1, %hi(kernelsp)
		LONG_L	k1, %lo(kernelsp)(k1)
		.endm

		.macro	set_saved_sp stackp temp temp2
		LONG_S	\stackp, kernelsp
		.endm

        .macro  SAVE_SP
        .set    push
        .set    noat
        .set    reorder
        mfc0    k0, CP0_STATUS
        sll k0, 3       /* extract cu0 bit */
        .set    noreorder
        bltz    k0, 8f
         move   k1, sp
        .set    reorder
        /* Called from user mode, new stack. */
        get_saved_sp
8:      move    k0, sp
        PTR_SUBU sp, k1, PT_SIZE
        LONG_S  k0, PT_R29(sp)
        .set    pop
        .endm

        .macro  SAVE_SOME_BUT_SP
        .set    push
        .set    noat
        .set    reorder
        LONG_S  $3, PT_R3(sp)
        LONG_S  $0, PT_R0(sp)
        mfc0    v1, CP0_STATUS
        LONG_S  $2, PT_R2(sp)
        LONG_S  v1, PT_STATUS(sp)
        LONG_S  $4, PT_R4(sp)
        mfc0    v1, CP0_CAUSE
        LONG_S  $5, PT_R5(sp)
        LONG_S  v1, PT_CAUSE(sp)
        LONG_S  $6, PT_R6(sp)
        MFC0    v1, CP0_EPC
        LONG_S  $7, PT_R7(sp)
        LONG_S  v1, PT_EPC(sp)
        LONG_S  $25, PT_R25(sp)
        LONG_S  $28, PT_R28(sp)
        LONG_S  $31, PT_R31(sp)
        ori $28, sp, _THREAD_MASK
        xori    $28, _THREAD_MASK
        .set    pop
        .endm

        .macro  SAVE_ALL_BUT_SP
        SAVE_SOME_BUT_SP
        SAVE_AT
        SAVE_TEMP
        SAVE_STATIC
#ifdef CONFIG_CPU_HAS_RADIAX
        SAVE_RADIAX
#endif
        .endm

		.macro	SAVE_SOME
		.set	push
		.set	noat
		.set	reorder
		mfc0	k0, CP0_STATUS
		sll	k0, 3		/* extract cu0 bit */
		.set	noreorder
		bltz	k0, 8f
		 move	k1, sp
		.set	reorder
		/* Called from user mode, new stack. */
		get_saved_sp
8:		move	k0, sp
		PTR_SUBU sp, k1, PT_SIZE
		LONG_S	k0, PT_R29(sp)
		LONG_S	$3, PT_R3(sp)
		/*
		 * You might think that you don't need to save $0,
		 * but the FPU emulator and gdb remote debug stub
		 * need it to operate correctly
		 */
		LONG_S	$0, PT_R0(sp)
		mfc0	v1, CP0_STATUS
		LONG_S	$2, PT_R2(sp)
		LONG_S	v1, PT_STATUS(sp)
		LONG_S	$4, PT_R4(sp)
		mfc0	v1, CP0_CAUSE
		LONG_S	$5, PT_R5(sp)
		LONG_S	v1, PT_CAUSE(sp)
		LONG_S	$6, PT_R6(sp)
		MFC0	v1, CP0_EPC
		LONG_S	$7, PT_R7(sp)
		LONG_S	v1, PT_EPC(sp)
		LONG_S	$25, PT_R25(sp)
		LONG_S	$28, PT_R28(sp)
		LONG_S	$31, PT_R31(sp)
		ori	$28, sp, _THREAD_MASK
		xori	$28, _THREAD_MASK
		.set	pop
		.endm

		.macro	SAVE_ALL
		SAVE_SOME
		SAVE_AT
		SAVE_TEMP
		SAVE_STATIC
#ifdef CONFIG_CPU_HAS_RADIAX
        SAVE_RADIAX
#endif
		.endm

		.macro	RESTORE_AT
		.set	push
		.set	noat
		LONG_L	$1,  PT_R1(sp)
		.set	pop
		.endm

		.macro	RESTORE_TEMP
		LONG_L	$24, PT_LO(sp)
		mtlo	$24
		LONG_L	$24, PT_HI(sp)
		mthi	$24
		LONG_L	$8, PT_R8(sp)
		LONG_L	$9, PT_R9(sp)
		LONG_L	$10, PT_R10(sp)
		LONG_L	$11, PT_R11(sp)
		LONG_L	$12, PT_R12(sp)
		LONG_L	$13, PT_R13(sp)
		LONG_L	$14, PT_R14(sp)
		LONG_L	$15, PT_R15(sp)
		LONG_L	$24, PT_R24(sp)
		.endm

		.macro	RESTORE_STATIC
		LONG_L	$16, PT_R16(sp)
		LONG_L	$17, PT_R17(sp)
		LONG_L	$18, PT_R18(sp)
		LONG_L	$19, PT_R19(sp)
		LONG_L	$20, PT_R20(sp)
		LONG_L	$21, PT_R21(sp)
		LONG_L	$22, PT_R22(sp)
		LONG_L	$23, PT_R23(sp)
		LONG_L	$30, PT_R30(sp)
		.endm

		.macro	RESTORE_SOME
		.set	push
		.set	reorder
		.set	noat
		mfc0	a0, CP0_STATUS
		li	v1, 0xff00
		ori	a0, STATMASK
		xori	a0, STATMASK
		mtc0	a0, CP0_STATUS
		and	a0, v1
		LONG_L	v0, PT_STATUS(sp)
		nor	v1, $0, v1
		and	v0, v1
		or	v0, a0
		mtc0	v0, CP0_STATUS
		LONG_L	$31, PT_R31(sp)
		LONG_L	$28, PT_R28(sp)
		LONG_L	$25, PT_R25(sp)
		LONG_L	$7,  PT_R7(sp)
		LONG_L	$6,  PT_R6(sp)
		LONG_L	$5,  PT_R5(sp)
		LONG_L	$4,  PT_R4(sp)
		LONG_L	$3,  PT_R3(sp)
		LONG_L	$2,  PT_R2(sp)
		.set	pop
		.endm

		.macro	RESTORE_SP_AND_RET
		.set	push
		.set	noreorder
		LONG_L	k0, PT_EPC(sp)
		LONG_L	sp, PT_R29(sp)
		jr	k0
		 rfe
		.set	pop
		.endm

		.macro	RESTORE_SP
		LONG_L	sp, PT_R29(sp)
		.endm

		.macro	RESTORE_ALL
#ifdef CONFIG_CPU_HAS_RADIAX
        RESTORE_RADIAX
#endif
		RESTORE_TEMP
		RESTORE_STATIC
		RESTORE_AT
		RESTORE_SOME
		RESTORE_SP
		.endm

		.macro	RESTORE_ALL_AND_RET
#ifdef CONFIG_CPU_HAS_RADIAX
        RESTORE_RADIAX
#endif
		RESTORE_TEMP
		RESTORE_STATIC
		RESTORE_AT
		RESTORE_SOME
		RESTORE_SP_AND_RET
		.endm

/*
 * Move to kernel mode and disable interrupts.
 * Set cp0 enable bit as sign that we're running on the kernel stack
 */
		.macro	CLI
		mfc0	t0, CP0_STATUS
		li	t1, ST0_CU0 | STATMASK
		or	t0, t1
		xori	t0, STATMASK
		mtc0	t0, CP0_STATUS
		irq_disable_hazard
		.endm

/*
 * Move to kernel mode and enable interrupts.
 * Set cp0 enable bit as sign that we're running on the kernel stack
 */
		.macro	STI
		mfc0	t0, CP0_STATUS
		li	t1, ST0_CU0 | STATMASK
		or	t0, t1
		xori	t0, STATMASK & ~1
		mtc0	t0, CP0_STATUS
		irq_enable_hazard
		.endm

/*
 * Just move to kernel mode and leave interrupts as they are.  Note
 * for the R3000 this means copying the previous enable from IEp.
 * Set cp0 enable bit as sign that we're running on the kernel stack
 */
		.macro	KMODE
		mfc0	t0, CP0_STATUS
		li	t1, ST0_CU0 | (STATMASK & ~1)
		andi	t2, t0, ST0_IEP
		srl	t2, 2
		or	t0, t2
		or	t0, t1
		xori	t0, STATMASK & ~1
		mtc0	t0, CP0_STATUS
		irq_disable_hazard
		.endm

#endif /* _ASM_STACKFRAME_H */
