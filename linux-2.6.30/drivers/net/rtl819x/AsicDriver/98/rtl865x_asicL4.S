	.file	1 "rtl865x_asicL4.c"
	.section .mdebug.abi32
	.previous
#APP
	.macro _ssnop; sll $0, $0, 1; .endm
	.macro _ehb; sll $0, $0, 3; .endm
	.macro mtc0_tlbw_hazard; nop; nop; .endm
	.macro tlbw_use_hazard; nop; nop; nop; .endm
	.macro tlb_probe_hazard; nop; nop; nop; .endm
	.macro irq_enable_hazard; _ssnop; _ssnop; _ssnop;; .endm
	.macro irq_disable_hazard; nop; nop; nop; .endm
	.macro back_to_back_c0_hazard; _ssnop; _ssnop; _ssnop;; .endm
		.macro	raw_local_irq_enable				
	.set	push						
	.set	reorder						
	.set	noat						
	mfc0	$1,$12						
	ori	$1,0x1f						
	xori	$1,0x1e						
	mtc0	$1,$12						
	irq_enable_hazard					
	.set	pop						
	.endm
		.macro	raw_local_irq_disable
	.set	push						
	.set	noat						
	mfc0	$1,$12						
	ori	$1,0x1f						
	xori	$1,0x1f						
	.set	noreorder					
	mtc0	$1,$12						
	irq_disable_hazard					
	.set	pop						
	.endm							

		.macro	raw_local_save_flags flags			
	.set	push						
	.set	reorder						
	mfc0	\flags, $12					
	.set	pop						
	.endm							

		.macro	raw_local_irq_save result			
	.set	push						
	.set	reorder						
	.set	noat						
	mfc0	\result, $12					
	ori	$1, \result, 0x1f				
	xori	$1, 0x1f					
	.set	noreorder					
	mtc0	$1, $12						
	irq_disable_hazard					
	.set	pop						
	.endm							

		.macro	raw_local_irq_restore flags			
	.set	push						
	.set	noreorder					
	.set	noat						
	mfc0	$1, $12						
	andi	\flags, 1					
	ori	$1, 0x1f					
	xori	$1, 0x1f					
	or	\flags, $1					
	mtc0	\flags, $12					
	irq_disable_hazard					
	.set	pop						
	.endm							

#NO_APP
	.section	.text._rtl8651_NaptAgingToSec,"ax",@progbits
	.align	2
	.globl	_rtl8651_NaptAgingToSec
	.ent	_rtl8651_NaptAgingToSec
	.type	_rtl8651_NaptAgingToSec, @function
_rtl8651_NaptAgingToSec:
	.set	nomips16
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	move	$5,$0
	srl	$3,$4,3
	li	$6,1			# 0x1
	beq	$3,$0,$L7
	andi	$4,$4,0x7

$L4:
	sll	$2,$6,3
	addu	$5,$5,$2
	addiu	$3,$3,-1
	bne	$3,$0,$L4
	sll	$6,$6,2

$L7:
	addiu	$2,$4,1
	mult	$2,$6
	mflo	$2
	addu	$5,$5,$2
	sll	$2,$5,2
	addu	$2,$2,$5
	addiu	$2,$2,-5
	li	$3,-1431699456			# 0xaaaa0000
	ori	$3,$3,0xaaab
	multu	$2,$3
	mfhi	$2
	srl	$5,$2,1
	subu	$2,$5,$6
	sltu	$3,$6,2
	j	$31
	movn	$2,$5,$3		#conditional move

	.set	macro
	.set	reorder
	.end	_rtl8651_NaptAgingToSec
	.section	.text._rtl8651_NaptAgingToUnit,"ax",@progbits
	.align	2
	.globl	_rtl8651_NaptAgingToUnit
	.ent	_rtl8651_NaptAgingToUnit
	.type	_rtl8651_NaptAgingToUnit, @function
_rtl8651_NaptAgingToUnit:
	.set	nomips16
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	move	$6,$0
	addiu	$4,$4,1
	sll	$2,$4,1
	addu	$4,$4,$2
	li	$2,-859045888			# 0xcccc0000
	ori	$2,$2,0xcccd
	multu	$4,$2
	mfhi	$2
	srl	$4,$2,2
	sltu	$2,$4,8
	bne	$2,$0,$L15
	li	$5,1			# 0x1

$L11:
	sll	$2,$5,3
	subu	$4,$4,$2
	sll	$5,$5,2
	sll	$2,$5,3
	sltu	$2,$4,$2
	beq	$2,$0,$L11
	addiu	$6,$6,1

$L15:
	divu	$0,$4,$5
	mfhi	$3
	srl	$2,$5,1
	sltu	$2,$2,$3
	beq	$2,$0,$L12
	nop

	sll	$6,$6,3
	mflo	$2
	addu	$6,$6,$2
	j	$L13
	addiu	$6,$6,1

$L12:
	sll	$6,$6,3
	divu	$0,$4,$5
	mflo	$2
	addu	$6,$6,$2
$L13:
	j	$31
	move	$2,$6

	.set	macro
	.set	reorder
	.end	_rtl8651_NaptAgingToUnit
	.section	.text._Is4WayHashEnabled,"ax",@progbits
	.align	2
	.globl	_Is4WayHashEnabled
	.ent	_Is4WayHashEnabled
	.type	_Is4WayHashEnabled, @function
_Is4WayHashEnabled:
	.set	nomips16
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x441c
	lw	$2,0($2)
	j	$31
	andi	$2,$2,0x200

	.set	macro
	.set	reorder
	.end	_Is4WayHashEnabled
	.section	.text._set4WayHash,"ax",@progbits
	.align	2
	.globl	_set4WayHash
	.ent	_set4WayHash
	.type	_set4WayHash, @function
_set4WayHash:
	.set	nomips16
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x441c
	lw	$3,0($2)
	beq	$4,$0,$L18
	nop

	j	$L19
	ori	$3,$3,0x200

$L18:
	li	$2,-513			# 0xfffffdff
	and	$3,$3,$2
$L19:
	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x441c
	sw	$3,0($2)
	j	$31
	move	$2,$0

	.set	macro
	.set	reorder
	.end	_set4WayHash
	.section	.text._rtl8651_enableEnhancedHash1,"ax",@progbits
	.align	2
	.globl	_rtl8651_enableEnhancedHash1
	.ent	_rtl8651_enableEnhancedHash1
	.type	_rtl8651_enableEnhancedHash1, @function
_rtl8651_enableEnhancedHash1:
	.set	nomips16
	.frame	$sp,32,$31		# vars= 8, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$16,24($sp)
	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x441c
	lw	$16,0($2)
	jal	rtl865x_getAsicFun
	addiu	$4,$sp,16

	lw	$2,16($sp)
	andi	$2,$2,0x4
	beq	$2,$0,$L20
	li	$3,-1			# 0xffffffff

	ori	$16,$16,0x2000
	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x441c
	sw	$16,0($2)
	move	$3,$0
$L20:
	move	$2,$3
	lw	$31,28($sp)
	lw	$16,24($sp)
	j	$31
	addiu	$sp,$sp,32

	.set	macro
	.set	reorder
	.end	_rtl8651_enableEnhancedHash1
	.section	.text._rtl8651_disableEnhancedHash1,"ax",@progbits
	.align	2
	.globl	_rtl8651_disableEnhancedHash1
	.ent	_rtl8651_disableEnhancedHash1
	.type	_rtl8651_disableEnhancedHash1, @function
_rtl8651_disableEnhancedHash1:
	.set	nomips16
	.frame	$sp,32,$31		# vars= 8, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$16,24($sp)
	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x441c
	lw	$16,0($2)
	jal	rtl865x_getAsicFun
	addiu	$4,$sp,16

	lw	$2,16($sp)
	andi	$2,$2,0x4
	beq	$2,$0,$L22
	li	$3,-1			# 0xffffffff

	li	$2,-8193			# 0xffffdfff
	and	$16,$16,$2
	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x441c
	sw	$16,0($2)
	move	$3,$0
$L22:
	move	$2,$3
	lw	$31,28($sp)
	lw	$16,24($sp)
	j	$31
	addiu	$sp,$sp,32

	.set	macro
	.set	reorder
	.end	_rtl8651_disableEnhancedHash1
	.section	.text.rtl8651_naptTcpUdpTableIndex,"ax",@progbits
	.align	2
	.globl	rtl8651_naptTcpUdpTableIndex
	.ent	rtl8651_naptTcpUdpTableIndex
	.type	rtl8651_naptTcpUdpTableIndex, @function
rtl8651_naptTcpUdpTableIndex:
	.set	nomips16
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	move	$9,$5
	sll	$4,$4,24
	sra	$4,$4,24
	lhu	$8,18($sp)
	bne	$7,$0,$L25
	andi	$6,$6,0xffff

	bne	$8,$0,$L25
	andi	$10,$4,0x00ff

	andi	$2,$10,0x2
	bne	$2,$0,$L25
	srl	$3,$5,10

	xor	$3,$3,$5
	srl	$2,$5,20
	xor	$2,$2,$3
	xor	$2,$6,$2
	andi	$5,$2,0x3ff
	andi	$3,$10,0x1
	sll	$3,$3,8
	srl	$2,$9,24
	andi	$2,$2,0xc0
	or	$3,$3,$2
	srl	$2,$6,10
	or	$3,$3,$2
	j	$31
	xor	$2,$5,$3

$L25:
	andi	$5,$6,0x3ff
	srl	$3,$6,10
	andi	$2,$9,0xf
	sll	$2,$2,6
	or	$3,$3,$2
	xor	$5,$5,$3
	srl	$2,$9,4
	andi	$2,$2,0x3ff
	xor	$5,$5,$2
	srl	$2,$9,14
	andi	$2,$2,0x3ff
	xor	$5,$5,$2
	srl	$3,$9,24
	andi	$2,$4,0x1
	sll	$2,$2,8
	or	$3,$3,$2
	andi	$2,$8,0x1
	sll	$2,$2,9
	or	$3,$3,$2
	xor	$5,$5,$3
	srl	$2,$8,1
	andi	$2,$2,0x3ff
	xor	$5,$5,$2
	srl	$3,$8,11
	andi	$2,$7,0x1f
	sll	$2,$2,5
	or	$3,$3,$2
	xor	$5,$5,$3
	srl	$2,$7,5
	andi	$2,$2,0x3ff
	xor	$5,$5,$2
	srl	$2,$7,15
	andi	$2,$2,0x3ff
	xor	$5,$5,$2
	srl	$2,$7,25
	j	$31
	xor	$2,$5,$2

	.set	macro
	.set	reorder
	.end	rtl8651_naptTcpUdpTableIndex
	.section	.text.rtl8651_setAsicLiberalNaptTcpUdpTable,"ax",@progbits
	.align	2
	.globl	rtl8651_setAsicLiberalNaptTcpUdpTable
	.ent	rtl8651_setAsicLiberalNaptTcpUdpTable
	.type	rtl8651_setAsicLiberalNaptTcpUdpTable, @function
rtl8651_setAsicLiberalNaptTcpUdpTable:
	.set	nomips16
	.frame	$sp,96,$31		# vars= 40, regs= 10/0, args= 16, gp= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-96
	sw	$31,92($sp)
	sw	$fp,88($sp)
	sw	$23,84($sp)
	sw	$22,80($sp)
	sw	$21,76($sp)
	sw	$20,72($sp)
	sw	$19,68($sp)
	sw	$18,64($sp)
	sw	$17,60($sp)
	sw	$16,56($sp)
	sw	$6,104($sp)
	sll	$23,$4,24
	sra	$23,$23,24
	andi	$fp,$5,0xffff
	andi	$20,$7,0xffff
	lb	$22,115($sp)
	lhu	$19,118($sp)
	lb	$21,127($sp)
	lb	$2,131($sp)
	sw	$2,48($sp)
	lb	$16,135($sp)
	lb	$17,139($sp)
	lb	$18,143($sp)
	addiu	$4,$sp,16
	move	$5,$0
	jal	memset
	li	$6,32			# 0x20

	xori	$16,$16,0x1
	sltu	$16,$16,1
	sll	$16,$16,1
	lw	$2,20($sp)
	li	$3,-3			# 0xfffffffd
	and	$2,$2,$3
	or	$2,$2,$16
	xori	$17,$17,0x1
	sltu	$17,$17,1
	sll	$17,$17,14
	li	$3,-16385			# 0xffffbfff
	and	$2,$2,$3
	or	$2,$2,$17
	srl	$3,$19,10
	sll	$3,$3,8
	li	$4,-16129			# 0xffffc0ff
	and	$4,$2,$4
	or	$4,$4,$3
	ori	$2,$4,0x4002
	beq	$18,$0,$L32
	sw	$2,20($sp)

	ori	$2,$4,0xc003
	li	$3,65536			# 0x10000
	or	$2,$2,$3
	sw	$2,20($sp)
	lw	$2,104($sp)
	sw	$2,16($sp)
	sh	$20,26($sp)
	lw	$4,120($sp)
	jal	_rtl8651_NaptAgingToUnit
	nop

	andi	$2,$2,0x3f
	sll	$2,$2,2
	lw	$3,20($sp)
	li	$4,-253			# 0xffffff03
	and	$3,$3,$4
	or	$3,$3,$2
	sw	$3,20($sp)
	andi	$2,$21,0x00ff
	srl	$6,$2,4
	andi	$6,$6,0x1
	srl	$5,$2,3
	andi	$5,$5,0x1
	beq	$5,$0,$L33
	move	$7,$0

	srl	$2,$2,2
	andi	$2,$2,0x1
	li	$3,1			# 0x1
	movn	$7,$3,$2		#conditional move
$L33:
	andi	$4,$19,0x3ff
	sll	$4,$4,21
	lw	$3,20($sp)
	li	$2,-2145452032			# 0x801f0000
	ori	$2,$2,0xffff
	and	$3,$3,$2
	or	$3,$3,$4
	andi	$4,$22,0xf
	sll	$4,$4,17
	li	$2,-2031616			# 0xffe10000
	ori	$2,$2,0xffff
	and	$3,$3,$2
	or	$3,$3,$4
	sw	$3,20($sp)
	sll	$3,$6,2
	sll	$2,$5,1
	or	$3,$3,$2
	or	$3,$3,$7
	andi	$3,$3,0x7
	sll	$3,$3,16
	lw	$4,24($sp)
	li	$2,-524288			# 0xfff80000
	ori	$2,$2,0xffff
	and	$4,$4,$2
	or	$4,$4,$3
	lw	$2,48($sp)
	xori	$3,$2,0x1
	sltu	$3,$3,1
	sll	$3,$3,19
	li	$2,-589824			# 0xfff70000
	ori	$2,$2,0xffff
	and	$4,$4,$2
	or	$4,$4,$3
	j	$L34
	sw	$4,24($sp)

$L32:
	sh	$0,26($sp)
	sw	$0,16($sp)
	lw	$4,20($sp)
	li	$2,-65536			# 0xffff0000
	ori	$2,$2,0x7fff
	and	$4,$4,$2
	li	$2,-131072			# 0xfffe0000
	ori	$2,$2,0xffff
	and	$4,$4,$2
	lw	$3,24($sp)
	li	$2,-524288			# 0xfff80000
	ori	$2,$2,0xffff
	and	$3,$3,$2
	sw	$3,24($sp)
	li	$2,-2031616			# 0xffe10000
	ori	$2,$2,0xffff
	and	$4,$4,$2
	li	$2,-2145452032			# 0x801f0000
	ori	$2,$2,0xffff
	and	$4,$4,$2
	li	$2,-253			# 0xffffff03
	and	$4,$4,$2
	sw	$4,20($sp)
$L34:
	li	$2,1			# 0x1
	bne	$23,$2,$L35
	li	$4,9			# 0x9

	move	$5,$fp
	jal	_rtl8651_forceAddAsicEntry
	addiu	$6,$sp,16

	j	$L28
	nop

$L35:
	move	$5,$fp
	jal	_rtl8651_addAsicEntry
	addiu	$6,$sp,16

	li	$3,-1			# 0xffffffff
	movz	$3,$0,$2		#conditional move
	move	$2,$3
$L28:
	lw	$31,92($sp)
	lw	$fp,88($sp)
	lw	$23,84($sp)
	lw	$22,80($sp)
	lw	$21,76($sp)
	lw	$20,72($sp)
	lw	$19,68($sp)
	lw	$18,64($sp)
	lw	$17,60($sp)
	lw	$16,56($sp)
	j	$31
	addiu	$sp,$sp,96

	.set	macro
	.set	reorder
	.end	rtl8651_setAsicLiberalNaptTcpUdpTable
	.section	.text.rtl8651_setAsicNaptTcpUdpTable,"ax",@progbits
	.align	2
	.globl	rtl8651_setAsicNaptTcpUdpTable
	.ent	rtl8651_setAsicNaptTcpUdpTable
	.type	rtl8651_setAsicNaptTcpUdpTable, @function
rtl8651_setAsicNaptTcpUdpTable:
	.set	nomips16
	.frame	$sp,64,$31		# vars= 32, regs= 4/0, args= 16, gp= 0
	.mask	0x80070000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-64
	sw	$31,60($sp)
	sw	$18,56($sp)
	sw	$17,52($sp)
	sw	$16,48($sp)
	move	$17,$5
	move	$16,$6
	sll	$18,$4,24
	sltu	$2,$5,1024
	beq	$2,$0,$L40
	sra	$18,$18,24

	bne	$6,$0,$L41
	addiu	$4,$sp,16

$L40:
	j	$L38
	li	$3,-1			# 0xffffffff

$L41:
	move	$5,$0
	jal	memset
	li	$6,32			# 0x20

	lw	$3,20($sp)
	ori	$3,$3,0x4002
	lw	$2,16($16)
	srl	$2,$2,10
	andi	$2,$2,0x1
	li	$4,-2			# 0xfffffffe
	and	$3,$3,$4
	or	$3,$3,$2
	lw	$2,0($16)
	sw	$2,16($sp)
	lhu	$2,4($16)
	sh	$2,26($sp)
	lbu	$2,13($16)
	andi	$2,$2,0x3f
	sll	$2,$2,8
	li	$4,-16129			# 0xffffc0ff
	and	$3,$3,$4
	or	$3,$3,$2
	sw	$3,20($sp)
	lw	$4,8($16)
	jal	_rtl8651_NaptAgingToUnit
	nop

	andi	$2,$2,0x3f
	sll	$2,$2,2
	lw	$3,20($sp)
	li	$4,-253			# 0xffffff03
	and	$3,$3,$4
	or	$3,$3,$2
	lw	$6,16($16)
	srl	$4,$6,13
	andi	$4,$4,0x1
	sll	$4,$4,16
	li	$2,-131072			# 0xfffe0000
	ori	$2,$2,0xffff
	and	$3,$3,$2
	or	$3,$3,$4
	srl	$4,$6,15
	andi	$4,$4,0x1
	sll	$4,$4,19
	lw	$5,24($sp)
	li	$2,-589824			# 0xfff70000
	ori	$2,$2,0xffff
	and	$5,$5,$2
	or	$5,$5,$4
	lbu	$4,12($16)
	andi	$4,$4,0x7
	sll	$4,$4,16
	li	$2,-524288			# 0xfff80000
	ori	$2,$2,0xffff
	and	$5,$5,$2
	or	$5,$5,$4
	sll	$4,$6,4
	andi	$4,$4,0x8000
	li	$2,-65536			# 0xffff0000
	ori	$2,$2,0x7fff
	and	$3,$3,$2
	or	$3,$3,$4
	lbu	$4,14($16)
	andi	$4,$4,0xf
	sll	$4,$4,17
	li	$2,-2031616			# 0xffe10000
	ori	$2,$2,0xffff
	and	$3,$3,$2
	or	$3,$3,$4
	lhu	$4,16($16)
	andi	$4,$4,0x3ff
	sll	$4,$4,21
	li	$2,-2145452032			# 0x801f0000
	ori	$2,$2,0xffff
	and	$3,$3,$2
	or	$3,$3,$4
	ori	$3,$3,0x4002
	sw	$3,20($sp)
	srl	$3,$6,9
	andi	$3,$3,0x1
	sll	$3,$3,20
	li	$2,-1114112			# 0xffef0000
	ori	$2,$2,0xffff
	and	$5,$5,$2
	or	$5,$5,$3
	srl	$6,$6,6
	andi	$6,$6,0x7
	sll	$6,$6,21
	li	$2,-14745600			# 0xff1f0000
	ori	$2,$2,0xffff
	and	$5,$5,$2
	or	$5,$5,$6
	li	$2,1			# 0x1
	bne	$18,$2,$L42
	sw	$5,24($sp)

	li	$4,9			# 0x9
	move	$5,$17
	jal	_rtl8651_forceAddAsicEntry
	addiu	$6,$sp,16

	j	$L38
	move	$3,$0

$L42:
	li	$4,9			# 0x9
	move	$5,$17
	jal	_rtl8651_addAsicEntry
	addiu	$6,$sp,16

	bne	$2,$0,$L38
	li	$3,-1			# 0xffffffff

	move	$3,$0
$L38:
	move	$2,$3
	lw	$31,60($sp)
	lw	$18,56($sp)
	lw	$17,52($sp)
	lw	$16,48($sp)
	j	$31
	addiu	$sp,$sp,64

	.set	macro
	.set	reorder
	.end	rtl8651_setAsicNaptTcpUdpTable
	.section	.text.rtl8651_getAsicRawNaptTable,"ax",@progbits
	.align	2
	.globl	rtl8651_getAsicRawNaptTable
	.ent	rtl8651_getAsicRawNaptTable
	.type	rtl8651_getAsicRawNaptTable, @function
rtl8651_getAsicRawNaptTable:
	.set	nomips16
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$16,16($sp)
	move	$2,$4
	move	$16,$5
	li	$4,9			# 0x9
	move	$5,$2
	jal	_rtl8651_readAsicEntry
	move	$6,$16

	move	$4,$2
	lw	$3,4($16)
	xori	$2,$3,0x1
	andi	$2,$2,0x1
	beq	$2,$0,$L47
	move	$2,$4

	li	$2,65536			# 0x10000
	and	$2,$3,$2
	beq	$2,$0,$L47
	move	$2,$4

	ori	$2,$3,0x1
	sw	$2,4($16)
	move	$2,$4
$L47:
	lw	$31,20($sp)
	lw	$16,16($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	rtl8651_getAsicRawNaptTable
	.section	.text.rtl8651_setAsicRawNaptTable,"ax",@progbits
	.align	2
	.globl	rtl8651_setAsicRawNaptTable
	.ent	rtl8651_setAsicRawNaptTable
	.type	rtl8651_setAsicRawNaptTable, @function
rtl8651_setAsicRawNaptTable:
	.set	nomips16
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,16($sp)
	move	$7,$4
	sll	$6,$6,24
	sra	$6,$6,24
	lw	$2,4($5)
	ori	$2,$2,0x4002
	sw	$2,4($5)
	li	$2,1			# 0x1
	bne	$6,$2,$L49
	move	$3,$5

	li	$4,9			# 0x9
	move	$5,$7
	jal	_rtl8651_forceAddAsicEntry
	move	$6,$3

	j	$L48
	nop

$L49:
	li	$4,9			# 0x9
	move	$5,$7
	jal	_rtl8651_addAsicEntry
	move	$6,$3

	li	$3,-1			# 0xffffffff
	movz	$3,$0,$2		#conditional move
	move	$2,$3
$L48:
	lw	$31,16($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	rtl8651_setAsicRawNaptTable
	.section	.text.rtl8651_getAsicNaptTcpUdpTable,"ax",@progbits
	.align	2
	.globl	rtl8651_getAsicNaptTcpUdpTable
	.ent	rtl8651_getAsicNaptTcpUdpTable
	.type	rtl8651_getAsicNaptTcpUdpTable, @function
rtl8651_getAsicNaptTcpUdpTable:
	.set	nomips16
	.frame	$sp,56,$31		# vars= 32, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-56
	sw	$31,52($sp)
	sw	$16,48($sp)
	sltu	$2,$4,1024
	beq	$2,$0,$L54
	move	$16,$5

	bne	$5,$0,$L53
	nop

$L54:
	j	$L52
	li	$2,-1			# 0xffffffff

$L53:
	jal	rtl8651_getAsicRawNaptTable
	addiu	$5,$sp,16

	lw	$2,16($sp)
	sw	$2,0($16)
	lhu	$2,26($sp)
	sh	$2,4($16)
	lw	$4,20($sp)
	srl	$2,$4,8
	andi	$2,$2,0x3f
	sb	$2,13($16)
	srl	$4,$4,2
	jal	_rtl8651_NaptAgingToSec
	andi	$4,$4,0x3f

	sw	$2,8($16)
	lw	$5,20($sp)
	srl	$4,$5,3
	andi	$4,$4,0x2000
	lw	$3,16($16)
	li	$2,-8193			# 0xffffdfff
	and	$3,$3,$2
	or	$3,$3,$4
	lw	$6,24($sp)
	srl	$4,$6,4
	andi	$4,$4,0x8000
	li	$2,-65536			# 0xffff0000
	ori	$2,$2,0x7fff
	and	$3,$3,$2
	or	$3,$3,$4
	sll	$2,$5,13
	andi	$2,$2,0x4000
	li	$4,-16385			# 0xffffbfff
	and	$3,$3,$4
	or	$3,$3,$2
	srl	$2,$5,2
	andi	$2,$2,0x1000
	li	$4,-4097			# 0xffffefff
	and	$3,$3,$4
	or	$3,$3,$2
	andi	$2,$5,0x1
	sll	$2,$2,10
	li	$4,-1025			# 0xfffffbff
	and	$3,$3,$4
	or	$3,$3,$2
	srl	$2,$5,4
	andi	$2,$2,0x800
	li	$4,-2049			# 0xfffff7ff
	and	$3,$3,$4
	or	$3,$3,$2
	sw	$3,16($16)
	srl	$2,$5,17
	andi	$2,$2,0xf
	sb	$2,14($16)
	srl	$5,$5,21
	andi	$5,$5,0x3ff
	sh	$5,16($16)
	srl	$2,$6,16
	andi	$2,$2,0x7
	sb	$2,12($16)
	srl	$4,$6,11
	andi	$4,$4,0x200
	lw	$2,16($16)
	li	$3,-513			# 0xfffffdff
	and	$2,$2,$3
	or	$2,$2,$4
	srl	$6,$6,15
	andi	$6,$6,0x1c0
	li	$3,-449			# 0xfffffe3f
	and	$2,$2,$3
	or	$2,$2,$6
	sw	$2,16($16)
	move	$2,$0
$L52:
	lw	$31,52($sp)
	lw	$16,48($sp)
	j	$31
	addiu	$sp,$sp,56

	.set	macro
	.set	reorder
	.end	rtl8651_getAsicNaptTcpUdpTable
	.section	.text.rtl8651_delAsicNaptTcpUdpTable,"ax",@progbits
	.align	2
	.globl	rtl8651_delAsicNaptTcpUdpTable
	.ent	rtl8651_delAsicNaptTcpUdpTable
	.type	rtl8651_delAsicNaptTcpUdpTable, @function
rtl8651_delAsicNaptTcpUdpTable:
	.set	nomips16
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,16($sp)
	move	$3,$4
	sltu	$2,$4,1024
	beq	$2,$0,$L57
	move	$6,$5

	sltu	$2,$5,1024
	bne	$2,$0,$L56
	li	$4,9			# 0x9

$L57:
	j	$L55
	li	$2,-1			# 0xffffffff

$L56:
	jal	_rtl8651_delAsicEntry
	move	$5,$3

	move	$2,$0
$L55:
	lw	$31,16($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	rtl8651_delAsicNaptTcpUdpTable
	.section	.text.rtl8651_setAsicNaptIcmpTimeout,"ax",@progbits
	.align	2
	.globl	rtl8651_setAsicNaptIcmpTimeout
	.ent	rtl8651_setAsicNaptIcmpTimeout
	.type	rtl8651_setAsicNaptIcmpTimeout, @function
rtl8651_setAsicNaptIcmpTimeout:
	.set	nomips16
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,16($sp)
	jal	_rtl8651_NaptAgingToUnit
	nop

	move	$5,$2
	sltu	$2,$2,1024
	beq	$2,$0,$L58
	li	$3,-1			# 0xffffffff

	li	$4,-1149239296			# 0xbb800000
	ori	$4,$4,0x4404
	lw	$3,0($4)
	li	$2,1056964608			# 0x3f000000
	nor	$2,$0,$2
	and	$3,$3,$2
	sll	$2,$5,24
	or	$3,$3,$2
	sw	$3,0($4)
	move	$3,$0
$L58:
	move	$2,$3
	lw	$31,16($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	rtl8651_setAsicNaptIcmpTimeout
	.section	.text.rtl8651_getAsicNaptIcmpTimeout,"ax",@progbits
	.align	2
	.globl	rtl8651_getAsicNaptIcmpTimeout
	.ent	rtl8651_getAsicNaptIcmpTimeout
	.type	rtl8651_getAsicNaptIcmpTimeout, @function
rtl8651_getAsicNaptIcmpTimeout:
	.set	nomips16
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$16,16($sp)
	move	$16,$4
	beq	$4,$0,$L60
	li	$2,-1			# 0xffffffff

	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x4404
	lw	$4,0($2)
	srl	$4,$4,24
	jal	_rtl8651_NaptAgingToSec
	andi	$4,$4,0x3f

	sw	$2,0($16)
	move	$2,$0
$L60:
	lw	$31,20($sp)
	lw	$16,16($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	rtl8651_getAsicNaptIcmpTimeout
	.section	.text.rtl8651_setAsicNaptUdpTimeout,"ax",@progbits
	.align	2
	.globl	rtl8651_setAsicNaptUdpTimeout
	.ent	rtl8651_setAsicNaptUdpTimeout
	.type	rtl8651_setAsicNaptUdpTimeout, @function
rtl8651_setAsicNaptUdpTimeout:
	.set	nomips16
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,16($sp)
	jal	_rtl8651_NaptAgingToUnit
	nop

	move	$5,$2
	sltu	$2,$2,1024
	beq	$2,$0,$L62
	li	$3,-1			# 0xffffffff

	li	$4,-1149239296			# 0xbb800000
	ori	$4,$4,0x4404
	lw	$3,0($4)
	li	$2,16515072			# 0xfc0000
	nor	$2,$0,$2
	and	$3,$3,$2
	sll	$2,$5,18
	or	$3,$3,$2
	sw	$3,0($4)
	move	$3,$0
$L62:
	move	$2,$3
	lw	$31,16($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	rtl8651_setAsicNaptUdpTimeout
	.section	.text.rtl8651_getAsicNaptUdpTimeout,"ax",@progbits
	.align	2
	.globl	rtl8651_getAsicNaptUdpTimeout
	.ent	rtl8651_getAsicNaptUdpTimeout
	.type	rtl8651_getAsicNaptUdpTimeout, @function
rtl8651_getAsicNaptUdpTimeout:
	.set	nomips16
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$16,16($sp)
	move	$16,$4
	beq	$4,$0,$L64
	li	$2,-1			# 0xffffffff

	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x4404
	lw	$4,0($2)
	srl	$4,$4,18
	jal	_rtl8651_NaptAgingToSec
	andi	$4,$4,0x3f

	sw	$2,0($16)
	move	$2,$0
$L64:
	lw	$31,20($sp)
	lw	$16,16($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	rtl8651_getAsicNaptUdpTimeout
	.section	.text.rtl8651_setAsicNaptTcpLongTimeout,"ax",@progbits
	.align	2
	.globl	rtl8651_setAsicNaptTcpLongTimeout
	.ent	rtl8651_setAsicNaptTcpLongTimeout
	.type	rtl8651_setAsicNaptTcpLongTimeout, @function
rtl8651_setAsicNaptTcpLongTimeout:
	.set	nomips16
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,16($sp)
	jal	_rtl8651_NaptAgingToUnit
	nop

	move	$5,$2
	sltu	$2,$2,1024
	beq	$2,$0,$L66
	li	$3,-1			# 0xffffffff

	li	$4,-1149239296			# 0xbb800000
	ori	$4,$4,0x4404
	lw	$3,0($4)
	li	$2,196608			# 0x30000
	ori	$2,$2,0xf000
	nor	$2,$0,$2
	and	$3,$3,$2
	sll	$2,$5,12
	or	$3,$3,$2
	sw	$3,0($4)
	move	$3,$0
$L66:
	move	$2,$3
	lw	$31,16($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	rtl8651_setAsicNaptTcpLongTimeout
	.section	.text.rtl8651_getAsicNaptTcpLongTimeout,"ax",@progbits
	.align	2
	.globl	rtl8651_getAsicNaptTcpLongTimeout
	.ent	rtl8651_getAsicNaptTcpLongTimeout
	.type	rtl8651_getAsicNaptTcpLongTimeout, @function
rtl8651_getAsicNaptTcpLongTimeout:
	.set	nomips16
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$16,16($sp)
	move	$16,$4
	beq	$4,$0,$L68
	li	$2,-1			# 0xffffffff

	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x4404
	lw	$4,0($2)
	srl	$4,$4,12
	jal	_rtl8651_NaptAgingToSec
	andi	$4,$4,0x3f

	sw	$2,0($16)
	move	$2,$0
$L68:
	lw	$31,20($sp)
	lw	$16,16($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	rtl8651_getAsicNaptTcpLongTimeout
	.section	.text.rtl8651_setAsicNaptTcpMediumTimeout,"ax",@progbits
	.align	2
	.globl	rtl8651_setAsicNaptTcpMediumTimeout
	.ent	rtl8651_setAsicNaptTcpMediumTimeout
	.type	rtl8651_setAsicNaptTcpMediumTimeout, @function
rtl8651_setAsicNaptTcpMediumTimeout:
	.set	nomips16
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,16($sp)
	jal	_rtl8651_NaptAgingToUnit
	nop

	move	$5,$2
	sltu	$2,$2,1024
	beq	$2,$0,$L70
	li	$3,-1			# 0xffffffff

	li	$4,-1149239296			# 0xbb800000
	ori	$4,$4,0x4404
	lw	$2,0($4)
	li	$3,-4033			# 0xfffff03f
	and	$2,$2,$3
	sll	$3,$5,6
	or	$2,$2,$3
	sw	$2,0($4)
	move	$3,$0
$L70:
	move	$2,$3
	lw	$31,16($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	rtl8651_setAsicNaptTcpMediumTimeout
	.section	.text.rtl8651_getAsicNaptTcpMediumTimeout,"ax",@progbits
	.align	2
	.globl	rtl8651_getAsicNaptTcpMediumTimeout
	.ent	rtl8651_getAsicNaptTcpMediumTimeout
	.type	rtl8651_getAsicNaptTcpMediumTimeout, @function
rtl8651_getAsicNaptTcpMediumTimeout:
	.set	nomips16
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$16,16($sp)
	move	$16,$4
	beq	$4,$0,$L72
	li	$2,-1			# 0xffffffff

	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x4404
	lw	$4,0($2)
	srl	$4,$4,6
	jal	_rtl8651_NaptAgingToSec
	andi	$4,$4,0x3f

	sw	$2,0($16)
	move	$2,$0
$L72:
	lw	$31,20($sp)
	lw	$16,16($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	rtl8651_getAsicNaptTcpMediumTimeout
	.section	.text.rtl8651_setAsicNaptTcpFastTimeout,"ax",@progbits
	.align	2
	.globl	rtl8651_setAsicNaptTcpFastTimeout
	.ent	rtl8651_setAsicNaptTcpFastTimeout
	.type	rtl8651_setAsicNaptTcpFastTimeout, @function
rtl8651_setAsicNaptTcpFastTimeout:
	.set	nomips16
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,16($sp)
	jal	_rtl8651_NaptAgingToUnit
	nop

	move	$5,$2
	sltu	$2,$2,1024
	beq	$2,$0,$L74
	li	$3,-1			# 0xffffffff

	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x4404
	lw	$3,0($2)
	li	$4,-64			# 0xffffffc0
	and	$3,$3,$4
	or	$3,$3,$5
	sw	$3,0($2)
	move	$3,$0
$L74:
	move	$2,$3
	lw	$31,16($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	rtl8651_setAsicNaptTcpFastTimeout
	.section	.text.rtl8651_getAsicNaptTcpFastTimeout,"ax",@progbits
	.align	2
	.globl	rtl8651_getAsicNaptTcpFastTimeout
	.ent	rtl8651_getAsicNaptTcpFastTimeout
	.type	rtl8651_getAsicNaptTcpFastTimeout, @function
rtl8651_getAsicNaptTcpFastTimeout:
	.set	nomips16
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$16,16($sp)
	move	$16,$4
	beq	$4,$0,$L76
	li	$2,-1			# 0xffffffff

	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x4404
	lw	$4,0($2)
	jal	_rtl8651_NaptAgingToSec
	andi	$4,$4,0x3f

	sw	$2,0($16)
	move	$2,$0
$L76:
	lw	$31,20($sp)
	lw	$16,16($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	rtl8651_getAsicNaptTcpFastTimeout
	.section	.text.rtl8651_setAsicNaptAutoAddDelete,"ax",@progbits
	.align	2
	.globl	rtl8651_setAsicNaptAutoAddDelete
	.ent	rtl8651_setAsicNaptAutoAddDelete
	.type	rtl8651_setAsicNaptAutoAddDelete, @function
rtl8651_setAsicNaptAutoAddDelete:
	.set	nomips16
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	sll	$4,$4,24
	sra	$4,$4,24
	sll	$5,$5,24
	li	$2,1			# 0x1
	bne	$4,$2,$L79
	sra	$5,$5,24

	li	$3,-1149239296			# 0xbb800000
	ori	$3,$3,0x4418
	lw	$2,0($3)
	ori	$2,$2,0x2
	sw	$2,0($3)
	j	$L83
	li	$2,1			# 0x1

$L79:
	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x4418
	lw	$3,0($2)
	li	$4,-3			# 0xfffffffd
	and	$3,$3,$4
	sw	$3,0($2)
	li	$2,1			# 0x1
$L83:
	bne	$5,$2,$L81
	li	$3,-1149239296			# 0xbb800000

	ori	$3,$3,0x4418
	lw	$2,0($3)
	ori	$2,$2,0x4
	sw	$2,0($3)
	j	$31
	move	$2,$0

$L81:
	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x4418
	lw	$3,0($2)
	li	$4,-5			# 0xfffffffb
	and	$3,$3,$4
	sw	$3,0($2)
	move	$2,$0
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	rtl8651_setAsicNaptAutoAddDelete
	.section	.text.rtl865x_initAsicL4,"ax",@progbits
	.align	2
	.globl	rtl865x_initAsicL4
	.ent	rtl865x_initAsicL4
	.type	rtl865x_initAsicL4, @function
rtl865x_initAsicL4:
	.set	nomips16
	.frame	$sp,32,$31		# vars= 8, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-32
	sw	$31,24($sp)
	jal	rtl865x_getAsicFun
	addiu	$4,$sp,16

	lw	$2,16($sp)
	andi	$2,$2,0x4
	beq	$2,$0,$L85
	li	$3,-1			# 0xffffffff

	li	$4,9			# 0x9
	jal	rtl8651_clearSpecifiedAsicTable
	li	$5,1024			# 0x400

	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x4418
	lw	$3,0($2)
	li	$4,-2			# 0xfffffffe
	and	$3,$3,$4
	sw	$3,0($2)
	move	$4,$0
	jal	rtl8651_setAsicNaptAutoAddDelete
	li	$5,1			# 0x1

	move	$3,$0
$L85:
	move	$2,$3
	lw	$31,24($sp)
	j	$31
	addiu	$sp,$sp,32

	.set	macro
	.set	reorder
	.end	rtl865x_initAsicL4
	.ident	"GCC: (GNU) 3.4.6-1.3.6"
