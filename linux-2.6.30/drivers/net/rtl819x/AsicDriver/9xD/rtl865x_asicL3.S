	.file	1 "rtl865x_asicL3.c"
	.section .mdebug.abi32
	.previous
	.gnu_attribute 4, 3
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
	.section	.text.rtl8651_ipMulticastTableIndex,"ax",@progbits
	.align	2
	.globl	rtl8651_ipMulticastTableIndex
	.set	nomips16
	.ent	rtl8651_ipMulticastTableIndex
	.type	rtl8651_ipMulticastTableIndex, @function
rtl8651_ipMulticastTableIndex:
	.frame	$sp,288,$31		# vars= 288, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-288
	addiu	$11,$sp,256
	addiu	$12,$sp,284
	move	$2,$11
$L2:
	sw	$0,0($2)
	addiu	$2,$2,4
	bne	$2,$12,$L2
	addiu	$7,$sp,128

	move	$6,$sp
	move	$2,$0
	li	$3,1			# 0x1
	li	$10,32			# 0x20
$L7:
	sll	$8,$3,$2
	and	$9,$8,$4
	move	$13,$0
	movn	$13,$3,$9
	sw	$13,0($6)
	and	$8,$8,$5
	move	$9,$0
	movn	$9,$3,$8
	sw	$9,0($7)
	addiu	$2,$2,1
	addiu	$7,$7,4
	bne	$2,$10,$L7
	addiu	$6,$6,4

	lw	$2,0($sp)
	lw	$3,28($sp)
	xor	$2,$3,$2
	lw	$3,56($sp)
	xor	$2,$2,$3
	lw	$3,84($sp)
	xor	$2,$2,$3
	lw	$3,112($sp)
	xor	$2,$2,$3
	lw	$3,132($sp)
	xor	$2,$2,$3
	lw	$3,160($sp)
	xor	$2,$2,$3
	lw	$3,188($sp)
	xor	$2,$2,$3
	lw	$3,216($sp)
	xor	$2,$2,$3
	lw	$3,244($sp)
	xor	$2,$2,$3
	sw	$2,256($sp)
	lw	$2,4($sp)
	lw	$3,32($sp)
	xor	$2,$3,$2
	lw	$3,60($sp)
	xor	$2,$2,$3
	lw	$3,88($sp)
	xor	$2,$2,$3
	lw	$3,116($sp)
	xor	$2,$2,$3
	lw	$3,136($sp)
	xor	$2,$2,$3
	lw	$3,164($sp)
	xor	$2,$2,$3
	lw	$3,192($sp)
	xor	$2,$2,$3
	lw	$3,220($sp)
	xor	$2,$2,$3
	lw	$3,248($sp)
	xor	$2,$2,$3
	sw	$2,260($sp)
	lw	$2,8($sp)
	lw	$3,36($sp)
	xor	$2,$3,$2
	lw	$3,64($sp)
	xor	$2,$2,$3
	lw	$3,92($sp)
	xor	$2,$2,$3
	lw	$3,120($sp)
	xor	$2,$2,$3
	lw	$3,140($sp)
	xor	$2,$2,$3
	lw	$3,168($sp)
	xor	$2,$2,$3
	lw	$3,196($sp)
	xor	$2,$2,$3
	lw	$3,224($sp)
	xor	$2,$2,$3
	lw	$3,252($sp)
	xor	$2,$2,$3
	sw	$2,264($sp)
	lw	$2,12($sp)
	lw	$3,40($sp)
	xor	$2,$3,$2
	lw	$3,68($sp)
	xor	$2,$2,$3
	lw	$3,96($sp)
	xor	$2,$2,$3
	lw	$3,124($sp)
	xor	$2,$2,$3
	lw	$3,144($sp)
	xor	$2,$2,$3
	lw	$3,172($sp)
	xor	$2,$2,$3
	lw	$3,200($sp)
	xor	$2,$2,$3
	lw	$3,228($sp)
	xor	$2,$2,$3
	sw	$2,268($sp)
	lw	$2,16($sp)
	lw	$3,44($sp)
	xor	$2,$3,$2
	lw	$3,72($sp)
	xor	$2,$2,$3
	lw	$3,100($sp)
	xor	$2,$2,$3
	lw	$3,148($sp)
	xor	$2,$2,$3
	lw	$3,176($sp)
	xor	$2,$2,$3
	lw	$3,204($sp)
	xor	$2,$2,$3
	lw	$3,232($sp)
	xor	$2,$2,$3
	sw	$2,272($sp)
	lw	$2,20($sp)
	lw	$3,48($sp)
	xor	$2,$3,$2
	lw	$3,76($sp)
	xor	$2,$2,$3
	lw	$3,104($sp)
	xor	$2,$2,$3
	lw	$3,152($sp)
	xor	$2,$2,$3
	lw	$3,180($sp)
	xor	$2,$2,$3
	lw	$3,208($sp)
	xor	$2,$2,$3
	lw	$3,236($sp)
	xor	$2,$2,$3
	sw	$2,276($sp)
	lw	$2,24($sp)
	lw	$3,52($sp)
	xor	$2,$3,$2
	lw	$3,80($sp)
	xor	$2,$2,$3
	lw	$3,108($sp)
	xor	$2,$2,$3
	lw	$3,128($sp)
	xor	$2,$2,$3
	lw	$3,156($sp)
	xor	$2,$2,$3
	lw	$3,184($sp)
	xor	$2,$2,$3
	lw	$3,212($sp)
	xor	$2,$2,$3
	lw	$3,240($sp)
	xor	$2,$2,$3
	sw	$2,280($sp)
	move	$2,$11
$L8:
	lw	$3,0($2)
	andi	$3,$3,0x1
	sw	$3,0($2)
	addiu	$2,$2,4
	bne	$12,$2,$L8
	move	$3,$0

	move	$2,$0
	li	$5,7			# 0x7
$L9:
	lw	$4,0($11)
	sll	$4,$4,$3
	addu	$2,$2,$4
	addiu	$3,$3,1
	bne	$3,$5,$L9
	addiu	$11,$11,4

	addiu	$sp,$sp,288
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	rtl8651_ipMulticastTableIndex
	.size	rtl8651_ipMulticastTableIndex, .-rtl8651_ipMulticastTableIndex
	.section	.text.rtl8651_setAsicMulticastEnable,"ax",@progbits
	.align	2
	.globl	rtl8651_setAsicMulticastEnable
	.set	nomips16
	.ent	rtl8651_setAsicMulticastEnable
	.type	rtl8651_setAsicMulticastEnable, @function
rtl8651_setAsicMulticastEnable:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	li	$2,1			# 0x1
	bne	$4,$2,$L16
	nop

	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x4428
	lw	$3,0($2)
	ori	$3,$3,0x8
	sw	$3,0($2)
	j	$31
	move	$2,$0

$L16:
	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x4428
	lw	$4,0($2)
	li	$3,-9			# 0xfffffff7
	and	$3,$4,$3
	sw	$3,0($2)
	move	$2,$0
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	rtl8651_setAsicMulticastEnable
	.size	rtl8651_setAsicMulticastEnable, .-rtl8651_setAsicMulticastEnable
	.section	.text.rtl8651_getAsicMulticastEnable,"ax",@progbits
	.align	2
	.globl	rtl8651_getAsicMulticastEnable
	.set	nomips16
	.ent	rtl8651_getAsicMulticastEnable
	.type	rtl8651_getAsicMulticastEnable, @function
rtl8651_getAsicMulticastEnable:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	bne	$4,$0,$L21
	nop

	j	$31
	li	$2,-1			# 0xffffffff

$L21:
	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x4428
	lw	$2,0($2)
	srl	$2,$2,3
	andi	$2,$2,0x1
	sw	$2,0($4)
	j	$31
	move	$2,$0

	.set	macro
	.set	reorder
	.end	rtl8651_getAsicMulticastEnable
	.size	rtl8651_getAsicMulticastEnable, .-rtl8651_getAsicMulticastEnable
	.section	.text.rtl8651_setAsicMulticastPortInternal,"ax",@progbits
	.align	2
	.globl	rtl8651_setAsicMulticastPortInternal
	.set	nomips16
	.ent	rtl8651_setAsicMulticastPortInternal
	.type	rtl8651_setAsicMulticastPortInternal, @function
rtl8651_setAsicMulticastPortInternal:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	sll	$5,$5,24
	lui	$2,%hi(rtl8651_totalExtPortNum)
	lw	$2,%lo(rtl8651_totalExtPortNum)($2)
	addiu	$2,$2,6
	sltu	$2,$4,$2
	bne	$2,$0,$L25
	sra	$5,$5,24

	j	$31
	li	$2,-1			# 0xffffffff

$L25:
	li	$2,1			# 0x1
	bne	$5,$2,$L27
	nop

	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x4418
	lw	$3,0($2)
	li	$5,1			# 0x1
	sll	$4,$5,$4
	andi	$4,$4,0x1ff
	sll	$4,$4,5
	or	$4,$4,$3
	sw	$4,0($2)
	j	$31
	move	$2,$0

$L27:
	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x4418
	lw	$3,0($2)
	li	$5,1			# 0x1
	sll	$4,$5,$4
	andi	$4,$4,0x1ff
	sll	$4,$4,5
	nor	$4,$0,$4
	and	$4,$4,$3
	sw	$4,0($2)
	j	$31
	move	$2,$0

	.set	macro
	.set	reorder
	.end	rtl8651_setAsicMulticastPortInternal
	.size	rtl8651_setAsicMulticastPortInternal, .-rtl8651_setAsicMulticastPortInternal
	.section	.text.rtl8651_getAsicMulticastPortInternal,"ax",@progbits
	.align	2
	.globl	rtl8651_getAsicMulticastPortInternal
	.set	nomips16
	.ent	rtl8651_getAsicMulticastPortInternal
	.type	rtl8651_getAsicMulticastPortInternal, @function
rtl8651_getAsicMulticastPortInternal:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	beq	$5,$0,$L30
	nop

	lui	$2,%hi(rtl8651_totalExtPortNum)
	lw	$2,%lo(rtl8651_totalExtPortNum)($2)
	addiu	$2,$2,6
	sltu	$2,$4,$2
	beq	$2,$0,$L30
	nop

	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x4418
	lw	$2,0($2)
	li	$3,1			# 0x1
	sll	$4,$3,$4
	andi	$4,$4,0x1ff
	sll	$4,$4,5
	and	$4,$4,$2
	beq	$4,$0,$L31
	nop

	li	$2,1			# 0x1
	sb	$2,0($5)
	j	$31
	move	$2,$0

$L31:
	sb	$0,0($5)
	j	$31
	move	$2,$0

$L30:
	j	$31
	li	$2,-1			# 0xffffffff

	.set	macro
	.set	reorder
	.end	rtl8651_getAsicMulticastPortInternal
	.size	rtl8651_getAsicMulticastPortInternal, .-rtl8651_getAsicMulticastPortInternal
	.section	.text.rtl8651_setAsicMulticastMTU,"ax",@progbits
	.align	2
	.globl	rtl8651_setAsicMulticastMTU
	.set	nomips16
	.ent	rtl8651_setAsicMulticastMTU
	.type	rtl8651_setAsicMulticastMTU, @function
rtl8651_setAsicMulticastMTU:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	li	$2,-16384			# 0xffffc000
	and	$2,$4,$2
	beq	$2,$0,$L35
	nop

	j	$31
	li	$2,-1			# 0xffffffff

$L35:
	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x440c
	lw	$5,0($2)
	andi	$4,$4,0x3fff
	li	$3,-16384			# 0xffffc000
	and	$3,$5,$3
	or	$4,$4,$3
	sw	$4,0($2)
	j	$31
	move	$2,$0

	.set	macro
	.set	reorder
	.end	rtl8651_setAsicMulticastMTU
	.size	rtl8651_setAsicMulticastMTU, .-rtl8651_setAsicMulticastMTU
	.section	.text.rtl8651_getAsicMulticastMTU,"ax",@progbits
	.align	2
	.globl	rtl8651_getAsicMulticastMTU
	.set	nomips16
	.ent	rtl8651_getAsicMulticastMTU
	.type	rtl8651_getAsicMulticastMTU, @function
rtl8651_getAsicMulticastMTU:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	bne	$4,$0,$L39
	nop

	j	$31
	li	$2,-1			# 0xffffffff

$L39:
	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x440c
	lw	$2,0($2)
	andi	$2,$2,0x3fff
	sw	$2,0($4)
	j	$31
	move	$2,$0

	.set	macro
	.set	reorder
	.end	rtl8651_getAsicMulticastMTU
	.size	rtl8651_getAsicMulticastMTU, .-rtl8651_getAsicMulticastMTU
	.section	.text.rtl865x_setAsicMulticastAging,"ax",@progbits
	.align	2
	.globl	rtl865x_setAsicMulticastAging
	.set	nomips16
	.ent	rtl865x_setAsicMulticastAging
	.type	rtl865x_setAsicMulticastAging, @function
rtl865x_setAsicMulticastAging:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	li	$2,1			# 0x1
	bne	$4,$2,$L43
	nop

	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x4400
	lw	$4,0($2)
	li	$3,-9			# 0xfffffff7
	and	$3,$4,$3
	sw	$3,0($2)
	j	$31
	move	$2,$0

$L43:
	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x4400
	lw	$3,0($2)
	ori	$3,$3,0x8
	sw	$3,0($2)
	move	$2,$0
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	rtl865x_setAsicMulticastAging
	.size	rtl865x_setAsicMulticastAging, .-rtl865x_setAsicMulticastAging
	.section	.text.rtl8651_getAsicIpMulticastTable,"ax",@progbits
	.align	2
	.globl	rtl8651_getAsicIpMulticastTable
	.set	nomips16
	.ent	rtl8651_getAsicIpMulticastTable
	.type	rtl8651_getAsicIpMulticastTable, @function
rtl8651_getAsicIpMulticastTable:
	.frame	$sp,56,$31		# vars= 32, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-56
	sw	$31,52($sp)
	sw	$16,48($sp)
	move	$2,$4
	beq	$5,$0,$L48
	move	$16,$5

	li	$4,3			# 0x3
	move	$5,$2
	jal	_rtl8651_readAsicEntryStopTLU
	addiu	$6,$sp,16

	lw	$2,16($sp)
	sw	$2,0($16)
	lw	$2,24($sp)
	andi	$2,$2,0x2000
	beq	$2,$0,$L49
	li	$2,268369920			# 0xfff0000

	ori	$2,$2,0xffff
	lw	$3,20($sp)
	and	$2,$3,$2
	li	$3,-536870912			# 0xe0000000
	or	$2,$2,$3
	j	$L50
	sw	$2,4($16)

$L49:
	ori	$2,$2,0xffff
	lw	$3,20($sp)
	and	$2,$3,$2
	bne	$2,$0,$L51
	li	$2,268369920			# 0xfff0000

	ori	$2,$2,0xffff
	and	$2,$3,$2
	j	$L50
	sw	$2,4($16)

$L51:
	ori	$2,$2,0xffff
	lw	$3,20($sp)
	and	$3,$3,$2
	li	$2,-536870912			# 0xe0000000
	or	$2,$3,$2
	sw	$2,4($16)
$L50:
	sh	$0,8($16)
	lw	$2,20($sp)
	srl	$2,$2,28
	sh	$2,10($16)
	lw	$2,24($sp)
	andi	$3,$2,0x1ff
	sw	$3,12($16)
	srl	$3,$2,9
	andi	$3,$3,0xf
	sh	$3,20($16)
	srl	$3,$2,15
	andi	$3,$3,0x7
	sll	$4,$3,2
	addu	$3,$4,$3
	sh	$3,16($16)
	srl	$3,$2,14
	andi	$3,$3,0x1
	andi	$2,$2,0x2000
	beq	$2,$0,$L48
	sh	$3,18($16)

	j	$L52
	move	$2,$0

$L48:
	li	$2,-1			# 0xffffffff
$L52:
	lw	$31,52($sp)
	lw	$16,48($sp)
	j	$31
	addiu	$sp,$sp,56

	.set	macro
	.set	reorder
	.end	rtl8651_getAsicIpMulticastTable
	.size	rtl8651_getAsicIpMulticastTable, .-rtl8651_getAsicIpMulticastTable
	.section	.text.rtl8651_getAsicArp,"ax",@progbits
	.align	2
	.globl	rtl8651_getAsicArp
	.set	nomips16
	.ent	rtl8651_getAsicArp
	.type	rtl8651_getAsicArp, @function
rtl8651_getAsicArp:
	.frame	$sp,56,$31		# vars= 32, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-56
	sw	$31,52($sp)
	sw	$16,48($sp)
	move	$2,$4
	sltu	$3,$4,512
	beq	$3,$0,$L55
	move	$16,$5

	beq	$5,$0,$L55
	li	$4,1			# 0x1

	move	$5,$2
	jal	_rtl8651_readAsicEntryStopTLU
	addiu	$6,$sp,16

	lw	$2,16($sp)
	andi	$2,$2,0x1
	beq	$2,$0,$L55
	nop

	lw	$2,16($sp)
	srl	$3,$2,1
	andi	$3,$3,0x3ff
	sra	$4,$3,2
	sw	$4,0($16)
	andi	$3,$3,0x3
	sw	$3,4($16)
	srl	$2,$2,11
	andi	$2,$2,0x1f
	sw	$2,8($16)
	j	$L56
	move	$2,$0

$L55:
	li	$2,-1			# 0xffffffff
$L56:
	lw	$31,52($sp)
	lw	$16,48($sp)
	j	$31
	addiu	$sp,$sp,56

	.set	macro
	.set	reorder
	.end	rtl8651_getAsicArp
	.size	rtl8651_getAsicArp, .-rtl8651_getAsicArp
	.section	.text.rtl8651_delAsicIpMulticastTable,"ax",@progbits
	.align	2
	.globl	rtl8651_delAsicIpMulticastTable
	.set	nomips16
	.ent	rtl8651_delAsicIpMulticastTable
	.type	rtl8651_delAsicIpMulticastTable, @function
rtl8651_delAsicIpMulticastTable:
	.frame	$sp,64,$31		# vars= 32, regs= 3/0, args= 16, gp= 0
	.mask	0x80030000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-64
	sw	$31,60($sp)
	sw	$17,56($sp)
	sw	$16,52($sp)
	move	$17,$4
	addiu	$16,$sp,16
	move	$4,$16
	move	$5,$0
	jal	memset
	li	$6,32			# 0x20

	li	$4,3			# 0x3
	move	$5,$17
	jal	_rtl8651_forceAddAsicEntry
	move	$6,$16

	lw	$31,60($sp)
	lw	$17,56($sp)
	lw	$16,52($sp)
	j	$31
	addiu	$sp,$sp,64

	.set	macro
	.set	reorder
	.end	rtl8651_delAsicIpMulticastTable
	.size	rtl8651_delAsicIpMulticastTable, .-rtl8651_delAsicIpMulticastTable
	.section	.text.rtl8651_setAsicIpMulticastTable,"ax",@progbits
	.align	2
	.globl	rtl8651_setAsicIpMulticastTable
	.set	nomips16
	.ent	rtl8651_setAsicIpMulticastTable
	.type	rtl8651_setAsicIpMulticastTable, @function
rtl8651_setAsicIpMulticastTable:
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
	lw	$3,4($4)
	srl	$3,$3,28
	li	$2,14			# 0xe
	bne	$3,$2,$L61
	move	$16,$4

	lhu	$3,10($4)
	lui	$2,%hi(rtl8651_totalExtPortNum)
	lw	$2,%lo(rtl8651_totalExtPortNum)($2)
	addiu	$2,$2,6
	slt	$2,$3,$2
	beq	$2,$0,$L61
	addiu	$17,$sp,16

	move	$4,$17
	move	$5,$0
	jal	memset
	li	$6,32			# 0x20

	lw	$4,0($16)
	sw	$4,16($sp)
	lw	$5,4($16)
	li	$18,268369920			# 0xfff0000
	ori	$18,$18,0xffff
	and	$3,$5,$18
	li	$2,-268435456			# 0xf0000000
	lw	$6,20($sp)
	and	$2,$6,$2
	or	$2,$2,$3
	jal	rtl8651_ipMulticastTableIndex
	sw	$2,20($sp)

	lbu	$3,11($16)
	sll	$3,$3,28
	lw	$4,20($sp)
	and	$18,$4,$18
	or	$18,$18,$3
	sw	$18,20($sp)
	lhu	$3,14($16)
	andi	$3,$3,0x1ff
	li	$4,-512			# 0xfffffe00
	lw	$5,24($sp)
	and	$5,$5,$4
	or	$5,$5,$3
	li	$3,-16385			# 0xffffbfff
	and	$5,$5,$3
	ori	$5,$5,0x2000
	lbu	$3,21($16)
	andi	$3,$3,0xf
	sll	$3,$3,9
	li	$4,-7681			# 0xffffe1ff
	and	$4,$5,$4
	or	$4,$4,$3
	li	$3,196608			# 0x30000
	ori	$3,$3,0x8000
	or	$3,$4,$3
	sw	$3,24($sp)
	li	$4,3			# 0x3
	move	$5,$2
	jal	_rtl8651_forceAddAsicEntry
	move	$6,$17

	j	$L62
	nop

$L61:
	li	$2,-1			# 0xffffffff
$L62:
	lw	$31,60($sp)
	lw	$18,56($sp)
	lw	$17,52($sp)
	lw	$16,48($sp)
	j	$31
	addiu	$sp,$sp,64

	.set	macro
	.set	reorder
	.end	rtl8651_setAsicIpMulticastTable
	.size	rtl8651_setAsicIpMulticastTable, .-rtl8651_setAsicIpMulticastTable
	.section	.text.rtl8651_delAsicArp,"ax",@progbits
	.align	2
	.globl	rtl8651_delAsicArp
	.set	nomips16
	.ent	rtl8651_delAsicArp
	.type	rtl8651_delAsicArp, @function
rtl8651_delAsicArp:
	.frame	$sp,64,$31		# vars= 32, regs= 3/0, args= 16, gp= 0
	.mask	0x80030000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-64
	sw	$31,60($sp)
	sw	$17,56($sp)
	sw	$16,52($sp)
	sltu	$2,$4,512
	bne	$2,$0,$L65
	move	$16,$4

	j	$L66
	li	$2,-1			# 0xffffffff

$L65:
	addiu	$17,$sp,16
	move	$4,$17
	move	$5,$0
	jal	memset
	li	$6,32			# 0x20

	li	$2,-2			# 0xfffffffe
	lw	$3,16($sp)
	and	$2,$3,$2
	sw	$2,16($sp)
	li	$4,1			# 0x1
	move	$5,$16
	jal	_rtl8651_forceAddAsicEntry
	move	$6,$17

$L66:
	lw	$31,60($sp)
	lw	$17,56($sp)
	lw	$16,52($sp)
	j	$31
	addiu	$sp,$sp,64

	.set	macro
	.set	reorder
	.end	rtl8651_delAsicArp
	.size	rtl8651_delAsicArp, .-rtl8651_delAsicArp
	.section	.text.rtl8651_setAsicArp,"ax",@progbits
	.align	2
	.globl	rtl8651_setAsicArp
	.set	nomips16
	.ent	rtl8651_setAsicArp
	.type	rtl8651_setAsicArp, @function
rtl8651_setAsicArp:
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
	move	$16,$4
	sltu	$2,$4,512
	beq	$2,$0,$L69
	move	$17,$5

	beq	$5,$0,$L69
	addiu	$18,$sp,16

	move	$4,$18
	move	$5,$0
	jal	memset
	li	$6,32			# 0x20

	lhu	$3,6($17)
	lhu	$2,2($17)
	andi	$3,$3,0x3
	sll	$2,$2,2
	or	$2,$3,$2
	andi	$2,$2,0x3ff
	sll	$2,$2,1
	li	$3,-2047			# 0xfffff801
	lw	$4,16($sp)
	and	$4,$4,$3
	or	$4,$4,$2
	ori	$4,$4,0x1
	lbu	$3,11($17)
	andi	$3,$3,0x1f
	sll	$3,$3,11
	li	$2,-65536			# 0xffff0000
	ori	$2,$2,0x7ff
	and	$2,$4,$2
	or	$2,$2,$3
	sw	$2,16($sp)
	li	$4,1			# 0x1
	move	$5,$16
	jal	_rtl8651_forceAddAsicEntry
	move	$6,$18

	j	$L70
	nop

$L69:
	li	$2,-1			# 0xffffffff
$L70:
	lw	$31,60($sp)
	lw	$18,56($sp)
	lw	$17,52($sp)
	lw	$16,48($sp)
	j	$31
	addiu	$sp,$sp,64

	.set	macro
	.set	reorder
	.end	rtl8651_setAsicArp
	.size	rtl8651_setAsicArp, .-rtl8651_setAsicArp
	.section	.text.rtl8651_delAsicRouting,"ax",@progbits
	.align	2
	.globl	rtl8651_delAsicRouting
	.set	nomips16
	.ent	rtl8651_delAsicRouting
	.type	rtl8651_delAsicRouting, @function
rtl8651_delAsicRouting:
	.frame	$sp,64,$31		# vars= 32, regs= 3/0, args= 16, gp= 0
	.mask	0x80030000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-64
	sw	$31,60($sp)
	sw	$17,56($sp)
	sw	$16,52($sp)
	sltu	$2,$4,8
	bne	$2,$0,$L73
	move	$16,$4

	j	$L74
	li	$2,-1			# 0xffffffff

$L73:
	addiu	$17,$sp,16
	move	$4,$17
	move	$5,$0
	jal	memset
	li	$6,32			# 0x20

	li	$2,-33			# 0xffffffdf
	lw	$3,20($sp)
	and	$2,$3,$2
	sw	$2,20($sp)
	li	$4,2			# 0x2
	move	$5,$16
	jal	_rtl8651_forceAddAsicEntry
	move	$6,$17

$L74:
	lw	$31,60($sp)
	lw	$17,56($sp)
	lw	$16,52($sp)
	j	$31
	addiu	$sp,$sp,64

	.set	macro
	.set	reorder
	.end	rtl8651_delAsicRouting
	.size	rtl8651_delAsicRouting, .-rtl8651_delAsicRouting
	.section	.text.rtl8651_setAsicRouting,"ax",@progbits
	.align	2
	.globl	rtl8651_setAsicRouting
	.set	nomips16
	.ent	rtl8651_setAsicRouting
	.type	rtl8651_setAsicRouting, @function
rtl8651_setAsicRouting:
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
	move	$16,$4
	sltu	$2,$4,8
	beq	$2,$0,$L77
	move	$17,$5

	beq	$5,$0,$L100
	li	$2,-1			# 0xffffffff

	lw	$4,4($5)
	bne	$4,$0,$L78
	andi	$2,$4,0x1

	j	$L79
	move	$18,$0

$L78:
	beq	$2,$0,$L80
	li	$5,1			# 0x1

	j	$L81
	move	$2,$0

$L80:
	li	$2,1			# 0x1
	li	$6,32			# 0x20
	sll	$3,$5,$2
$L102:
	and	$3,$3,$4
	bne	$3,$0,$L101
	li	$18,31			# 0x1f

	addiu	$2,$2,1
	bne	$2,$6,$L102
	sll	$3,$5,$2

$L81:
	li	$18,31			# 0x1f
$L101:
	subu	$18,$18,$2
$L79:
	addiu	$4,$sp,16
	move	$5,$0
	jal	memset
	li	$6,32			# 0x20

	lw	$2,0($17)
	sw	$2,16($sp)
	lw	$2,8($17)
	sltu	$2,$2,7
	beq	$2,$0,$L100
	li	$2,-1			# 0xffffffff

	lw	$3,8($17)
	sll	$3,$3,2
	lui	$2,%hi($L88)
	addiu	$2,$2,%lo($L88)
	addu	$2,$2,$3
	lw	$2,0($2)
	j	$2
	nop

	.rdata
	.align	2
	.align	2
$L88:
	.word	$L83
	.word	$L84
	.word	$L85
	.word	$L77
	.word	$L86
	.word	$L87
	.word	$L86
	.section	.text.rtl8651_setAsicRouting
$L83:
	lbu	$2,39($17)
	andi	$2,$2,0x7
	sll	$2,$2,24
	li	$3,-117506048			# 0xf8ff0000
	ori	$3,$3,0xffff
	lw	$5,20($sp)
	and	$5,$5,$3
	or	$5,$5,$2
	lhu	$3,30($17)
	sll	$3,$3,2
	lhu	$2,34($17)
	or	$2,$3,$2
	andi	$2,$2,0x3ff
	sll	$2,$2,14
	li	$3,-16777216			# 0xff000000
	ori	$3,$3,0x3fff
	and	$5,$5,$3
	or	$5,$5,$2
	andi	$18,$18,0x1f
	li	$2,-32			# 0xffffffe0
	and	$5,$5,$2
	or	$5,$5,$18
	lbu	$2,15($17)
	andi	$2,$2,0x7
	sll	$2,$2,11
	li	$3,-14337			# 0xffffc7ff
	and	$5,$5,$3
	or	$5,$5,$2
	lw	$2,60($17)
	srl	$3,$2,31
	sll	$3,$3,9
	li	$4,-513			# 0xfffffdff
	and	$4,$5,$4
	or	$4,$4,$3
	srl	$2,$2,20
	andi	$2,$2,0x400
	li	$3,-1025			# 0xfffffbff
	and	$4,$4,$3
	or	$4,$4,$2
	lbu	$3,11($17)
	andi	$3,$3,0x7
	sll	$3,$3,6
	li	$2,-449			# 0xfffffe3f
	and	$2,$4,$2
	or	$2,$2,$3
	ori	$2,$2,0x20
	j	$L89
	sw	$2,20($sp)

$L84:
	lhu	$3,30($17)
	sll	$3,$3,2
	lhu	$2,34($17)
	or	$2,$3,$2
	andi	$2,$2,0x3ff
	sll	$2,$2,14
	li	$3,-16777216			# 0xff000000
	ori	$3,$3,0x3fff
	lw	$5,20($sp)
	and	$5,$5,$3
	or	$5,$5,$2
	andi	$18,$18,0x1f
	li	$2,-32			# 0xffffffe0
	and	$5,$5,$2
	or	$5,$5,$18
	lbu	$2,15($17)
	andi	$2,$2,0x7
	sll	$2,$2,11
	li	$3,-14337			# 0xffffc7ff
	and	$5,$5,$3
	or	$5,$5,$2
	lw	$2,60($17)
	srl	$3,$2,31
	sll	$3,$3,9
	li	$4,-513			# 0xfffffdff
	and	$4,$5,$4
	or	$4,$4,$3
	srl	$2,$2,20
	andi	$2,$2,0x400
	li	$3,-1025			# 0xfffffbff
	and	$4,$4,$3
	or	$4,$4,$2
	lbu	$3,11($17)
	andi	$3,$3,0x7
	sll	$3,$3,6
	li	$2,-449			# 0xfffffe3f
	and	$2,$4,$2
	or	$2,$2,$3
	ori	$2,$2,0x20
	j	$L89
	sw	$2,20($sp)

$L85:
	lw	$2,20($17)
	srl	$2,$2,3
	andi	$2,$2,0x3f
	sll	$2,$2,20
	li	$3,-66125824			# 0xfc0f0000
	ori	$3,$3,0xffff
	lw	$5,20($sp)
	and	$5,$5,$3
	or	$5,$5,$2
	lw	$2,16($17)
	srl	$2,$2,3
	andi	$2,$2,0x3f
	sll	$2,$2,14
	li	$3,-1048576			# 0xfff00000
	ori	$3,$3,0x3fff
	and	$5,$5,$3
	or	$5,$5,$2
	andi	$18,$18,0x1f
	li	$2,-32			# 0xffffffe0
	and	$5,$5,$2
	or	$5,$5,$18
	lbu	$2,15($17)
	andi	$2,$2,0x7
	sll	$2,$2,11
	li	$3,-14337			# 0xffffc7ff
	and	$5,$5,$3
	or	$5,$5,$2
	lw	$2,60($17)
	srl	$3,$2,31
	sll	$3,$3,9
	li	$4,-513			# 0xfffffdff
	and	$4,$5,$4
	or	$4,$4,$3
	srl	$2,$2,20
	andi	$2,$2,0x400
	li	$3,-1025			# 0xfffffbff
	and	$4,$4,$3
	or	$4,$4,$2
	lbu	$2,11($17)
	andi	$2,$2,0x7
	sll	$2,$2,6
	li	$3,-449			# 0xfffffe3f
	and	$4,$4,$3
	or	$4,$4,$2
	ori	$4,$4,0x20
	lbu	$3,27($17)
	andi	$3,$3,0x7
	sll	$3,$3,26
	li	$2,-469827584			# 0xe3ff0000
	ori	$2,$2,0xffff
	and	$2,$4,$2
	or	$2,$2,$3
	j	$L89
	sw	$2,20($sp)

$L86:
	lbu	$2,15($17)
	andi	$2,$2,0x7
	sll	$2,$2,11
	li	$3,-14337			# 0xffffc7ff
	lw	$4,20($sp)
	and	$4,$4,$3
	or	$4,$4,$2
	andi	$18,$18,0x1f
	li	$2,-32			# 0xffffffe0
	and	$4,$4,$2
	or	$4,$4,$18
	lbu	$2,11($17)
	andi	$2,$2,0x7
	sll	$2,$2,6
	li	$3,-449			# 0xfffffe3f
	and	$4,$4,$3
	or	$4,$4,$2
	ori	$4,$4,0x20
	lw	$3,60($17)
	srl	$3,$3,31
	sll	$3,$3,9
	li	$2,-513			# 0xfffffdff
	and	$2,$4,$2
	or	$2,$2,$3
	j	$L89
	sw	$2,20($sp)

$L87:
	lw	$3,40($17)
	srl	$3,$3,1
	andi	$3,$3,0xf
	sll	$3,$3,14
	li	$2,-262144			# 0xfffc0000
	ori	$2,$2,0x3fff
	lw	$4,20($sp)
	and	$2,$4,$2
	or	$2,$2,$3
	sw	$2,20($sp)
	lw	$2,44($17)
	sltu	$2,$2,33
	beq	$2,$0,$L100
	li	$2,-1			# 0xffffffff

	lw	$3,44($17)
	sll	$3,$3,2
	lui	$2,%hi($L95)
	addiu	$2,$2,%lo($L95)
	addu	$2,$2,$3
	lw	$2,0($2)
	j	$2
	nop

	.rdata
	.align	2
	.align	2
$L95:
	.word	$L77
	.word	$L77
	.word	$L90
	.word	$L77
	.word	$L91
	.word	$L77
	.word	$L77
	.word	$L77
	.word	$L92
	.word	$L77
	.word	$L77
	.word	$L77
	.word	$L77
	.word	$L77
	.word	$L77
	.word	$L77
	.word	$L93
	.word	$L77
	.word	$L77
	.word	$L77
	.word	$L77
	.word	$L77
	.word	$L77
	.word	$L77
	.word	$L77
	.word	$L77
	.word	$L77
	.word	$L77
	.word	$L77
	.word	$L77
	.word	$L77
	.word	$L77
	.word	$L94
	.section	.text.rtl8651_setAsicRouting
$L90:
	li	$2,-14337			# 0xffffc7ff
	lw	$3,20($sp)
	and	$2,$3,$2
	j	$L96
	sw	$2,20($sp)

$L91:
	li	$2,-14337			# 0xffffc7ff
	lw	$3,20($sp)
	and	$2,$3,$2
	ori	$2,$2,0x800
	j	$L96
	sw	$2,20($sp)

$L92:
	li	$2,-14337			# 0xffffc7ff
	lw	$3,20($sp)
	and	$2,$3,$2
	ori	$2,$2,0x1000
	j	$L96
	sw	$2,20($sp)

$L93:
	li	$2,-14337			# 0xffffc7ff
	lw	$3,20($sp)
	and	$2,$3,$2
	ori	$2,$2,0x1800
	j	$L96
	sw	$2,20($sp)

$L94:
	li	$2,-14337			# 0xffffc7ff
	lw	$3,20($sp)
	and	$2,$3,$2
	ori	$2,$2,0x2000
	sw	$2,20($sp)
$L96:
	lbu	$2,51($17)
	andi	$2,$2,0x1f
	sll	$2,$2,18
	li	$3,-8192000			# 0xff830000
	ori	$3,$3,0xffff
	lw	$5,20($sp)
	and	$5,$5,$3
	or	$5,$5,$2
	lbu	$2,55($17)
	andi	$2,$2,0x3
	sll	$2,$2,23
	li	$3,-25231360			# 0xfe7f0000
	ori	$3,$3,0xffff
	and	$5,$5,$3
	or	$5,$5,$2
	andi	$18,$18,0x1f
	li	$2,-32			# 0xffffffe0
	and	$5,$5,$2
	or	$5,$5,$18
	lbu	$2,11($17)
	andi	$2,$2,0x7
	sll	$2,$2,6
	li	$3,-449			# 0xfffffe3f
	and	$5,$5,$3
	or	$5,$5,$2
	ori	$5,$5,0x20
	lbu	$2,59($17)
	andi	$2,$2,0x7
	sll	$2,$2,25
	li	$3,-234946560			# 0xf1ff0000
	ori	$3,$3,0xffff
	and	$5,$5,$3
	or	$5,$5,$2
	lw	$2,60($17)
	srl	$3,$2,31
	sll	$3,$3,9
	li	$4,-513			# 0xfffffdff
	and	$4,$5,$4
	or	$4,$4,$3
	srl	$2,$2,20
	andi	$3,$2,0x400
	li	$2,-1025			# 0xfffffbff
	and	$2,$4,$2
	or	$2,$2,$3
	sw	$2,20($sp)
$L89:
	li	$4,2			# 0x2
	move	$5,$16
	jal	_rtl8651_forceAddAsicEntry
	addiu	$6,$sp,16

	j	$L97
	nop

$L77:
	li	$2,-1			# 0xffffffff
$L97:
$L100:
	lw	$31,60($sp)
	lw	$18,56($sp)
	lw	$17,52($sp)
	lw	$16,48($sp)
	j	$31
	addiu	$sp,$sp,64

	.set	macro
	.set	reorder
	.end	rtl8651_setAsicRouting
	.size	rtl8651_setAsicRouting, .-rtl8651_setAsicRouting
	.section	.text.rtl865x_initAsicL3,"ax",@progbits
	.align	2
	.globl	rtl865x_initAsicL3
	.set	nomips16
	.ent	rtl865x_initAsicL3
	.type	rtl865x_initAsicL3, @function
rtl865x_initAsicL3:
	.frame	$sp,104,$31		# vars= 72, regs= 3/0, args= 16, gp= 0
	.mask	0x80030000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-104
	sw	$31,100($sp)
	sw	$17,96($sp)
	sw	$16,92($sp)
	jal	rtl865x_getAsicFun
	addiu	$4,$sp,84

	lw	$2,84($sp)
	andi	$2,$2,0x2
	beq	$2,$0,$L110
	li	$2,-1			# 0xffffffff

	li	$4,5			# 0x5
	jal	rtl8651_clearSpecifiedAsicTable
	li	$5,16			# 0x10

	li	$4,1			# 0x1
	jal	rtl8651_clearSpecifiedAsicTable
	li	$5,512			# 0x200

	li	$4,11			# 0xb
	jal	rtl8651_clearSpecifiedAsicTable
	li	$5,8			# 0x8

	li	$4,13			# 0xd
	jal	rtl8651_clearSpecifiedAsicTable
	li	$5,32			# 0x20

	li	$4,2			# 0x2
	jal	rtl8651_clearSpecifiedAsicTable
	li	$5,8			# 0x8

	li	$4,3			# 0x3
	jal	rtl8651_clearSpecifiedAsicTable
	li	$5,128			# 0x80

	jal	rtl8651_setAsicMulticastEnable
	li	$4,1			# 0x1

	li	$16,-1149239296			# 0xbb800000
	ori	$16,$16,0x440c
	lw	$3,0($16)
	li	$2,262144			# 0x40000
	or	$2,$3,$2
	sw	$2,0($16)
	jal	rtl8651_setAsicMulticastMTU
	li	$4,1522			# 0x5f2

	lw	$3,0($16)
	li	$2,65536			# 0x10000
	or	$2,$3,$2
	sw	$2,0($16)
	lui	$2,%hi(rtl8651_totalExtPortNum)
	lw	$2,%lo(rtl8651_totalExtPortNum)($2)
	addiu	$2,$2,6
	blez	$2,$L105
	move	$16,$0

	lui	$17,%hi(rtl8651_totalExtPortNum)
	move	$4,$16
$L111:
	jal	rtl8651_setAsicMulticastPortInternal
	li	$5,1			# 0x1

	bne	$2,$0,$L104
	addiu	$16,$16,1

	lw	$2,%lo(rtl8651_totalExtPortNum)($17)
	addiu	$2,$2,6
	slt	$2,$16,$2
	bne	$2,$0,$L111
	move	$4,$16

$L105:
	addiu	$16,$sp,16
	move	$4,$16
	move	$5,$0
	jal	memset
	li	$6,68			# 0x44

	li	$2,4			# 0x4
	sw	$2,24($sp)
	sw	$0,16($sp)
	sw	$0,20($sp)
	sw	$0,28($sp)
	li	$2,2147418112			# 0x7fff0000
	ori	$2,$2,0xffff
	lw	$3,76($sp)
	and	$2,$3,$2
	sw	$2,76($sp)
	li	$4,7			# 0x7
	jal	rtl8651_setAsicRouting
	move	$5,$16

	j	$L107
	move	$2,$0

$L104:
	li	$2,-1			# 0xffffffff
$L107:
$L110:
	lw	$31,100($sp)
	lw	$17,96($sp)
	lw	$16,92($sp)
	j	$31
	addiu	$sp,$sp,104

	.set	macro
	.set	reorder
	.end	rtl865x_initAsicL3
	.size	rtl865x_initAsicL3, .-rtl865x_initAsicL3
	.section	.text.rtl8651_setAsicNextHopTable,"ax",@progbits
	.align	2
	.globl	rtl8651_setAsicNextHopTable
	.set	nomips16
	.ent	rtl8651_setAsicNextHopTable
	.type	rtl8651_setAsicNextHopTable, @function
rtl8651_setAsicNextHopTable:
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
	move	$16,$4
	sltu	$2,$4,32
	beq	$2,$0,$L113
	move	$17,$5

	beq	$5,$0,$L113
	addiu	$18,$sp,16

	move	$4,$18
	move	$5,$0
	jal	memset
	li	$6,32			# 0x20

	lhu	$3,2($17)
	sll	$3,$3,2
	lhu	$2,6($17)
	or	$3,$3,$2
	andi	$3,$3,0x3ff
	sll	$3,$3,11
	li	$2,-2097152			# 0xffe00000
	ori	$2,$2,0x7ff
	lw	$4,16($sp)
	and	$2,$4,$2
	or	$2,$2,$3
	lbu	$3,11($17)
	andi	$3,$3,0x7
	sll	$3,$3,8
	li	$4,-1793			# 0xfffff8ff
	and	$2,$2,$4
	or	$2,$2,$3
	lbu	$3,15($17)
	andi	$3,$3,0x7
	sll	$3,$3,5
	li	$4,-225			# 0xffffff1f
	and	$2,$2,$4
	or	$2,$2,$3
	lbu	$3,19($17)
	andi	$3,$3,0xf
	sll	$3,$3,1
	li	$4,-31			# 0xffffffe1
	and	$2,$2,$4
	or	$2,$2,$3
	lw	$3,20($17)
	srl	$3,$3,31
	li	$4,-2			# 0xfffffffe
	and	$2,$2,$4
	or	$2,$2,$3
	sw	$2,16($sp)
	li	$4,13			# 0xd
	move	$5,$16
	jal	_rtl8651_forceAddAsicEntry
	move	$6,$18

	j	$L114
	nop

$L113:
	li	$2,-1			# 0xffffffff
$L114:
	lw	$31,60($sp)
	lw	$18,56($sp)
	lw	$17,52($sp)
	lw	$16,48($sp)
	j	$31
	addiu	$sp,$sp,64

	.set	macro
	.set	reorder
	.end	rtl8651_setAsicNextHopTable
	.size	rtl8651_setAsicNextHopTable, .-rtl8651_setAsicNextHopTable
	.section	.text.rtl8651_setAsicPppoe,"ax",@progbits
	.align	2
	.globl	rtl8651_setAsicPppoe
	.set	nomips16
	.ent	rtl8651_setAsicPppoe
	.type	rtl8651_setAsicPppoe, @function
rtl8651_setAsicPppoe:
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
	move	$16,$4
	sltu	$2,$4,8
	beq	$2,$0,$L117
	move	$17,$5

	beq	$5,$0,$L120
	li	$2,-1			# 0xffffffff

	lhu	$3,0($5)
	li	$2,65535			# 0xffff
	beq	$3,$2,$L117
	addiu	$18,$sp,16

	move	$4,$18
	move	$5,$0
	jal	memset
	li	$6,32			# 0x20

	lhu	$2,0($17)
	sh	$2,18($sp)
	lbu	$3,3($17)
	andi	$3,$3,0x7
	sll	$3,$3,16
	li	$2,-524288			# 0xfff80000
	ori	$2,$2,0xffff
	lw	$4,16($sp)
	and	$2,$4,$2
	or	$2,$2,$3
	sw	$2,16($sp)
	li	$4,11			# 0xb
	move	$5,$16
	jal	_rtl8651_forceAddAsicEntry
	move	$6,$18

	j	$L118
	nop

$L117:
	li	$2,-1			# 0xffffffff
$L118:
$L120:
	lw	$31,60($sp)
	lw	$18,56($sp)
	lw	$17,52($sp)
	lw	$16,48($sp)
	j	$31
	addiu	$sp,$sp,64

	.set	macro
	.set	reorder
	.end	rtl8651_setAsicPppoe
	.size	rtl8651_setAsicPppoe, .-rtl8651_setAsicPppoe
	.section	.text.rtl8651_delAsicExtIntIpTable,"ax",@progbits
	.align	2
	.globl	rtl8651_delAsicExtIntIpTable
	.set	nomips16
	.ent	rtl8651_delAsicExtIntIpTable
	.type	rtl8651_delAsicExtIntIpTable, @function
rtl8651_delAsicExtIntIpTable:
	.frame	$sp,64,$31		# vars= 32, regs= 3/0, args= 16, gp= 0
	.mask	0x80030000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-64
	sw	$31,60($sp)
	sw	$17,56($sp)
	sw	$16,52($sp)
	sltu	$2,$4,16
	bne	$2,$0,$L122
	move	$16,$4

	j	$L123
	li	$2,-1			# 0xffffffff

$L122:
	addiu	$17,$sp,16
	move	$4,$17
	move	$5,$0
	jal	memset
	li	$6,32			# 0x20

	li	$2,-2			# 0xfffffffe
	lw	$3,24($sp)
	and	$2,$3,$2
	sw	$2,24($sp)
	li	$4,5			# 0x5
	move	$5,$16
	jal	_rtl8651_forceAddAsicEntry
	move	$6,$17

$L123:
	lw	$31,60($sp)
	lw	$17,56($sp)
	lw	$16,52($sp)
	j	$31
	addiu	$sp,$sp,64

	.set	macro
	.set	reorder
	.end	rtl8651_delAsicExtIntIpTable
	.size	rtl8651_delAsicExtIntIpTable, .-rtl8651_delAsicExtIntIpTable
	.section	.text.rtl8651_setAsicExtIntIpTable,"ax",@progbits
	.align	2
	.globl	rtl8651_setAsicExtIntIpTable
	.set	nomips16
	.ent	rtl8651_setAsicExtIntIpTable
	.type	rtl8651_setAsicExtIntIpTable, @function
rtl8651_setAsicExtIntIpTable:
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
	move	$16,$4
	sltu	$2,$4,16
	beq	$2,$0,$L126
	move	$17,$5

	beq	$5,$0,$L129
	li	$2,-1			# 0xffffffff

	lw	$3,12($5)
	li	$2,-1073741824			# 0xc0000000
	and	$3,$3,$2
	beq	$3,$2,$L126
	addiu	$18,$sp,16

	move	$4,$18
	move	$5,$0
	jal	memset
	li	$6,32			# 0x20

	lw	$2,0($17)
	sw	$2,20($sp)
	lw	$2,4($17)
	sw	$2,16($sp)
	lw	$2,12($17)
	srl	$3,$2,31
	sll	$3,$3,2
	li	$4,-5			# 0xfffffffb
	lw	$5,24($sp)
	and	$4,$5,$4
	or	$4,$4,$3
	srl	$2,$2,29
	andi	$2,$2,0x2
	li	$3,-3			# 0xfffffffd
	and	$4,$4,$3
	or	$4,$4,$2
	lbu	$3,11($17)
	andi	$3,$3,0x1f
	sll	$3,$3,3
	li	$2,-249			# 0xffffff07
	and	$2,$4,$2
	or	$2,$2,$3
	ori	$2,$2,0x1
	sw	$2,24($sp)
	li	$4,5			# 0x5
	move	$5,$16
	jal	_rtl8651_forceAddAsicEntry
	move	$6,$18

	j	$L127
	nop

$L126:
	li	$2,-1			# 0xffffffff
$L127:
$L129:
	lw	$31,60($sp)
	lw	$18,56($sp)
	lw	$17,52($sp)
	lw	$16,48($sp)
	j	$31
	addiu	$sp,$sp,64

	.set	macro
	.set	reorder
	.end	rtl8651_setAsicExtIntIpTable
	.size	rtl8651_setAsicExtIntIpTable, .-rtl8651_setAsicExtIntIpTable
	.section	.text.rtl8651_getAsicRouting,"ax",@progbits
	.align	2
	.globl	rtl8651_getAsicRouting
	.set	nomips16
	.ent	rtl8651_getAsicRouting
	.type	rtl8651_getAsicRouting, @function
rtl8651_getAsicRouting:
	.frame	$sp,56,$31		# vars= 32, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-56
	sw	$31,52($sp)
	sw	$16,48($sp)
	move	$2,$4
	sltu	$3,$4,8
	beq	$3,$0,$L131
	move	$16,$5

	beq	$5,$0,$L131
	li	$4,2			# 0x2

	move	$5,$2
	jal	_rtl8651_readAsicEntry
	addiu	$6,$sp,16

	lw	$2,20($sp)
	andi	$2,$2,0x20
	beq	$2,$0,$L149
	li	$2,-1			# 0xffffffff

	lw	$2,16($sp)
	sw	$2,0($16)
	lw	$2,20($sp)
	srl	$2,$2,6
	andi	$2,$2,0x7
	sw	$2,8($16)
	sw	$0,4($16)
	move	$2,$0
	li	$4,31			# 0x1f
	li	$3,1			# 0x1
$L132:
	subu	$5,$4,$2
	sll	$5,$3,$5
	lw	$6,4($16)
	or	$5,$6,$5
	sw	$5,4($16)
	addiu	$2,$2,1
	lbu	$5,23($sp)
	andi	$5,$5,0x1f
	sltu	$5,$5,$2
	beq	$5,$0,$L132
	nop

	lw	$2,20($sp)
	srl	$3,$2,11
	andi	$3,$3,0x7
	sw	$3,12($16)
	srl	$2,$2,9
	sll	$3,$2,31
	lw	$4,60($16)
	li	$2,2147418112			# 0x7fff0000
	ori	$2,$2,0xffff
	and	$2,$4,$2
	or	$2,$2,$3
	sw	$2,60($16)
	lw	$2,8($16)
	sltu	$2,$2,7
	beq	$2,$0,$L149
	li	$2,-1			# 0xffffffff

	lw	$3,8($16)
	sll	$3,$3,2
	lui	$2,%hi($L138)
	addiu	$2,$2,%lo($L138)
	addu	$2,$2,$3
	lw	$2,0($2)
	j	$2
	nop

	.rdata
	.align	2
	.align	2
$L138:
	.word	$L133
	.word	$L134
	.word	$L135
	.word	$L131
	.word	$L136
	.word	$L137
	.word	$L136
	.section	.text.rtl8651_getAsicRouting
$L133:
	sw	$0,16($16)
	sw	$0,20($16)
	lw	$2,20($sp)
	srl	$3,$2,24
	andi	$3,$3,0x7
	sw	$3,36($16)
	srl	$3,$2,14
	andi	$3,$3,0x3ff
	sra	$4,$3,2
	sw	$4,28($16)
	andi	$3,$3,0x3
	sw	$3,32($16)
	srl	$3,$2,10
	andi	$3,$3,0x1
	sll	$3,$3,30
	lw	$4,60($16)
	li	$2,-1073807360			# 0xbfff0000
	ori	$2,$2,0xffff
	and	$2,$4,$2
	or	$2,$2,$3
	sw	$2,60($16)
	j	$L139
	move	$2,$0

$L134:
	sw	$0,16($16)
	sw	$0,20($16)
	sw	$0,36($16)
	lw	$2,20($sp)
	srl	$3,$2,14
	andi	$3,$3,0x3ff
	sra	$4,$3,2
	sw	$4,28($16)
	andi	$3,$3,0x3
	sw	$3,32($16)
	srl	$3,$2,10
	andi	$3,$3,0x1
	sll	$3,$3,30
	lw	$4,60($16)
	li	$2,-1073807360			# 0xbfff0000
	ori	$2,$2,0xffff
	and	$2,$4,$2
	or	$2,$2,$3
	sw	$2,60($16)
	j	$L139
	move	$2,$0

$L135:
	lw	$2,20($sp)
	srl	$3,$2,20
	andi	$3,$3,0x3f
	sw	$3,20($16)
	srl	$3,$2,14
	andi	$3,$3,0x3f
	sw	$3,16($16)
	sw	$0,36($16)
	sw	$0,28($16)
	sw	$0,32($16)
	srl	$3,$2,26
	andi	$3,$3,0x7
	sw	$3,24($16)
	srl	$3,$2,10
	andi	$3,$3,0x1
	sll	$3,$3,30
	lw	$4,60($16)
	li	$2,-1073807360			# 0xbfff0000
	ori	$2,$2,0xffff
	and	$2,$4,$2
	or	$2,$2,$3
	sw	$2,60($16)
	j	$L139
	move	$2,$0

$L136:
	sw	$0,16($16)
	sw	$0,20($16)
	sw	$0,36($16)
	sw	$0,28($16)
	sw	$0,32($16)
	lw	$3,20($sp)
	srl	$3,$3,10
	andi	$3,$3,0x1
	sll	$3,$3,30
	lw	$4,60($16)
	li	$2,-1073807360			# 0xbfff0000
	ori	$2,$2,0xffff
	and	$2,$4,$2
	or	$2,$2,$3
	sw	$2,60($16)
	j	$L139
	move	$2,$0

$L137:
	lw	$2,20($sp)
	srl	$3,$2,13
	andi	$3,$3,0x1e
	sw	$3,40($16)
	srl	$2,$2,11
	andi	$2,$2,0x7
	sltu	$3,$2,5
	beq	$3,$0,$L131
	sll	$2,$2,2

	lui	$3,%hi($L145)
	addiu	$3,$3,%lo($L145)
	addu	$2,$3,$2
	lw	$2,0($2)
	j	$2
	nop

	.rdata
	.align	2
	.align	2
$L145:
	.word	$L140
	.word	$L141
	.word	$L142
	.word	$L143
	.word	$L144
	.section	.text.rtl8651_getAsicRouting
$L140:
	li	$2,2			# 0x2
	j	$L146
	sw	$2,44($16)

$L141:
	li	$2,4			# 0x4
	j	$L146
	sw	$2,44($16)

$L142:
	li	$2,8			# 0x8
	j	$L146
	sw	$2,44($16)

$L143:
	li	$2,16			# 0x10
	j	$L146
	sw	$2,44($16)

$L144:
	li	$2,32			# 0x20
	sw	$2,44($16)
$L146:
	lw	$2,20($sp)
	srl	$3,$2,18
	andi	$3,$3,0x1f
	sw	$3,48($16)
	srl	$3,$2,23
	andi	$3,$3,0x3
	sw	$3,52($16)
	srl	$3,$2,25
	andi	$3,$3,0x7
	sw	$3,56($16)
	srl	$3,$2,9
	sll	$3,$3,31
	lw	$5,60($16)
	li	$4,2147418112			# 0x7fff0000
	ori	$4,$4,0xffff
	and	$4,$5,$4
	or	$4,$4,$3
	srl	$3,$2,10
	andi	$3,$3,0x1
	sll	$3,$3,30
	li	$2,-1073807360			# 0xbfff0000
	ori	$2,$2,0xffff
	and	$2,$4,$2
	or	$2,$2,$3
	sw	$2,60($16)
	j	$L139
	move	$2,$0

$L131:
	li	$2,-1			# 0xffffffff
$L139:
$L149:
	lw	$31,52($sp)
	lw	$16,48($sp)
	j	$31
	addiu	$sp,$sp,56

	.set	macro
	.set	reorder
	.end	rtl8651_getAsicRouting
	.size	rtl8651_getAsicRouting, .-rtl8651_getAsicRouting
	.section	.text.rtl8651_getAsicNextHopTable,"ax",@progbits
	.align	2
	.globl	rtl8651_getAsicNextHopTable
	.set	nomips16
	.ent	rtl8651_getAsicNextHopTable
	.type	rtl8651_getAsicNextHopTable, @function
rtl8651_getAsicNextHopTable:
	.frame	$sp,56,$31		# vars= 32, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-56
	sw	$31,52($sp)
	sw	$16,48($sp)
	move	$2,$4
	sltu	$3,$4,32
	beq	$3,$0,$L151
	move	$16,$5

	beq	$5,$0,$L151
	li	$4,13			# 0xd

	move	$5,$2
	jal	_rtl8651_readAsicEntry
	addiu	$6,$sp,16

	lw	$2,16($sp)
	srl	$3,$2,11
	andi	$3,$3,0x3ff
	sra	$4,$3,2
	sw	$4,0($16)
	andi	$3,$3,0x3
	sw	$3,4($16)
	srl	$3,$2,8
	andi	$3,$3,0x7
	sw	$3,8($16)
	srl	$3,$2,5
	andi	$3,$3,0x7
	sw	$3,12($16)
	srl	$3,$2,1
	andi	$3,$3,0xf
	sw	$3,16($16)
	sll	$2,$2,31
	lw	$4,20($16)
	li	$3,2147418112			# 0x7fff0000
	ori	$3,$3,0xffff
	and	$3,$4,$3
	or	$2,$3,$2
	sw	$2,20($16)
	j	$L152
	move	$2,$0

$L151:
	li	$2,-1			# 0xffffffff
$L152:
	lw	$31,52($sp)
	lw	$16,48($sp)
	j	$31
	addiu	$sp,$sp,56

	.set	macro
	.set	reorder
	.end	rtl8651_getAsicNextHopTable
	.size	rtl8651_getAsicNextHopTable, .-rtl8651_getAsicNextHopTable
	.section	.text.rtl8651_getAsicPppoe,"ax",@progbits
	.align	2
	.globl	rtl8651_getAsicPppoe
	.set	nomips16
	.ent	rtl8651_getAsicPppoe
	.type	rtl8651_getAsicPppoe, @function
rtl8651_getAsicPppoe:
	.frame	$sp,56,$31		# vars= 32, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-56
	sw	$31,52($sp)
	sw	$16,48($sp)
	move	$2,$4
	sltu	$3,$4,8
	beq	$3,$0,$L155
	move	$16,$5

	beq	$5,$0,$L155
	li	$4,11			# 0xb

	move	$5,$2
	jal	_rtl8651_readAsicEntry
	addiu	$6,$sp,16

	lhu	$2,18($sp)
	sh	$2,0($16)
	lbu	$2,17($sp)
	andi	$2,$2,0x7
	sh	$2,2($16)
	j	$L156
	move	$2,$0

$L155:
	li	$2,-1			# 0xffffffff
$L156:
	lw	$31,52($sp)
	lw	$16,48($sp)
	j	$31
	addiu	$sp,$sp,56

	.set	macro
	.set	reorder
	.end	rtl8651_getAsicPppoe
	.size	rtl8651_getAsicPppoe, .-rtl8651_getAsicPppoe
	.section	.text.rtl8651_getAsicExtIntIpTable,"ax",@progbits
	.align	2
	.globl	rtl8651_getAsicExtIntIpTable
	.set	nomips16
	.ent	rtl8651_getAsicExtIntIpTable
	.type	rtl8651_getAsicExtIntIpTable, @function
rtl8651_getAsicExtIntIpTable:
	.frame	$sp,56,$31		# vars= 32, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-56
	sw	$31,52($sp)
	sw	$16,48($sp)
	move	$2,$4
	sltu	$3,$4,16
	beq	$3,$0,$L159
	move	$16,$5

	beq	$5,$0,$L159
	li	$4,5			# 0x5

	move	$5,$2
	jal	_rtl8651_readAsicEntry
	addiu	$6,$sp,16

	lw	$2,24($sp)
	andi	$2,$2,0x1
	beq	$2,$0,$L159
	li	$4,2147418112			# 0x7fff0000

	lw	$2,20($sp)
	sw	$2,0($16)
	lw	$2,16($sp)
	sw	$2,4($16)
	lw	$2,24($sp)
	srl	$3,$2,2
	sll	$3,$3,31
	lw	$5,12($16)
	ori	$4,$4,0xffff
	and	$5,$5,$4
	or	$5,$5,$3
	srl	$4,$2,1
	andi	$4,$4,0x1
	sll	$4,$4,30
	li	$3,-1073807360			# 0xbfff0000
	ori	$3,$3,0xffff
	and	$3,$5,$3
	or	$3,$3,$4
	sw	$3,12($16)
	srl	$2,$2,3
	andi	$2,$2,0x1f
	sw	$2,8($16)
	j	$L160
	move	$2,$0

$L159:
	li	$2,-1			# 0xffffffff
$L160:
	lw	$31,52($sp)
	lw	$16,48($sp)
	j	$31
	addiu	$sp,$sp,56

	.set	macro
	.set	reorder
	.end	rtl8651_getAsicExtIntIpTable
	.size	rtl8651_getAsicExtIntIpTable, .-rtl8651_getAsicExtIntIpTable
	.ident	"GCC: (GNU) 4.4.5-1.5.5p4"
