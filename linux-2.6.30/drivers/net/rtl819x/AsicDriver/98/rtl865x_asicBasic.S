	.file	1 "rtl865x_asicBasic.c"
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
	.section	.text.prom_putchar,"ax",@progbits
	.align	2
	.ent	prom_putchar
	.type	prom_putchar, @function
prom_putchar:
	.set	nomips16
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	sll	$4,$4,24
	sra	$4,$4,24
	move	$3,$0
	li	$2,-1207959552			# 0xb8000000
	ori	$5,$2,0x2014
	move	$2,$3
$L6:
	sltu	$2,$2,30000
	bne	$2,$0,$L4
	addiu	$3,$3,1

	li	$3,-60
	li	$2,-1207959552			# 0xb8000000
	ori	$2,$2,0x2008
	sb	$3,0($2)
	j	$31
	nop

$L4:
	lbu	$2,0($5)
	andi	$2,$2,0x20
	beq	$2,$0,$L6
	move	$2,$3

	li	$2,-1207959552			# 0xb8000000
	ori	$2,$2,0x2000
	sb	$4,0($2)
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	prom_putchar
	.section	.text.early_console_write,"ax",@progbits
	.align	2
	.ent	early_console_write
	.type	early_console_write, @function
early_console_write:
	.set	nomips16
	.frame	$sp,40,$31		# vars= 0, regs= 5/0, args= 16, gp= 0
	.mask	0x800f0000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-40
	sw	$31,32($sp)
	sw	$19,28($sp)
	sw	$18,24($sp)
	sw	$17,20($sp)
	sw	$16,16($sp)
	move	$16,$4
	beq	$5,$0,$L9
	addiu	$17,$5,-1

	lb	$2,0($4)
	beq	$2,$0,$L9
	li	$19,10			# 0xa

	li	$18,-1			# 0xffffffff
	lb	$2,0($16)
$L13:
	bne	$2,$19,$L10
	nop

	jal	prom_putchar
	li	$4,13			# 0xd

$L10:
	lb	$4,0($16)
	jal	prom_putchar
	addiu	$16,$16,1

	addiu	$17,$17,-1
	beq	$17,$18,$L9
	nop

	lb	$2,0($16)
	bne	$2,$0,$L13
	nop

$L9:
	lw	$31,32($sp)
	lw	$19,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	j	$31
	addiu	$sp,$sp,40

	.set	macro
	.set	reorder
	.end	early_console_write
	.data
	.align	2
	.type	_rtl8651_asicTableSize, @object
	.size	_rtl8651_asicTableSize, 64
_rtl8651_asicTableSize:
	.word	2
	.word	1
	.word	2
	.word	3
	.word	5
	.word	3
	.word	3
	.word	3
	.word	4
	.word	3
	.word	3
	.word	1
	.word	8
	.word	1
	.word	3
	.word	1
	.section	.dram-fwd,"aw",@progbits
	.align	2
	.type	fun_enable, @object
	.size	fun_enable, 4
fun_enable:
	.word	0
	.section	.text.rtl865x_initAsicFun,"ax",@progbits
	.align	2
	.globl	rtl865x_initAsicFun
	.ent	rtl865x_initAsicFun
	.type	rtl865x_initAsicFun, @function
rtl865x_initAsicFun:
	.set	nomips16
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	move	$6,$4
	sw	$0,0($4)
	li	$2,-1207959552			# 0xb8000000
	lw	$4,0($2)
	li	$3,-65536			# 0xffff0000
	and	$4,$4,$3
	ori	$2,$2,0xc
	lw	$3,0($2)
	li	$2,-1073741824			# 0xc0000000
	bne	$4,$2,$L15
	andi	$5,$3,0xf

	li	$2,9			# 0x9
	beq	$5,$2,$L17
	nop

	andi	$2,$3,0x8
	bne	$2,$0,$L16
	nop

$L17:
	lw	$2,0($6)
	ori	$2,$2,0x2e
	j	$L23
	sw	$2,0($6)

$L16:
	li	$2,8			# 0x8
	bne	$5,$2,$L19
	nop

	lw	$2,0($6)
	ori	$2,$2,0xe
	j	$L23
	sw	$2,0($6)

$L19:
	addiu	$2,$5,-10
	sltu	$2,$2,2
	beq	$2,$0,$L21
	nop

	lw	$2,0($6)
	ori	$2,$2,0xa
	j	$L23
	sw	$2,0($6)

$L21:
	lw	$2,0($6)
	ori	$2,$2,0xe
	j	$L23
	sw	$2,0($6)

$L15:
	li	$2,-2147483648			# 0x80000000
	bne	$4,$2,$L23
	nop

	li	$2,7			# 0x7
	bne	$5,$2,$L25
	nop

	lw	$2,0($6)
	ori	$2,$2,0x2e
	j	$L23
	sw	$2,0($6)

$L25:
	li	$2,15			# 0xf
	bne	$5,$2,$L27
	nop

	lw	$2,0($6)
	ori	$2,$2,0x3
	j	$L23
	sw	$2,0($6)

$L27:
	li	$2,3			# 0x3
	bne	$5,$2,$L29
	nop

	lw	$2,0($6)
	ori	$2,$2,0x3
	j	$L23
	sw	$2,0($6)

$L29:
	lw	$2,0($6)
	ori	$2,$2,0x3
	sw	$2,0($6)
$L23:
	lw	$3,0($6)
	li	$2,-1			# 0xffffffff
	j	$31
	movn	$2,$0,$3		#conditional move

	.set	macro
	.set	reorder
	.end	rtl865x_initAsicFun
	.section	.text.rtl865x_getAsicFun,"ax",@progbits
	.align	2
	.globl	rtl865x_getAsicFun
	.ent	rtl865x_getAsicFun
	.type	rtl865x_getAsicFun, @function
rtl865x_getAsicFun:
	.set	nomips16
	.frame	$sp,32,$31		# vars= 0, regs= 3/0, args= 16, gp= 0
	.mask	0x80030000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-32
	sw	$31,24($sp)
	sw	$17,20($sp)
	sw	$16,16($sp)
	move	$17,$4
	lui	$2,%hi(fun_enable)
	lw	$3,%lo(fun_enable)($2)
	sw	$3,0($4)
	lw	$2,%lo(fun_enable)($2)
	bne	$2,$0,$L33
	move	$3,$0

	lui	$16,%hi(fun_enable)
	jal	rtl865x_initAsicFun
	addiu	$4,$16,%lo(fun_enable)

	lw	$2,%lo(fun_enable)($16)
	sw	$2,0($17)
	move	$3,$0
$L33:
	move	$2,$3
	lw	$31,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	j	$31
	addiu	$sp,$sp,32

	.set	macro
	.set	reorder
	.end	rtl865x_getAsicFun
	.rdata
	.align	2
$LC0:
	.ascii	"init switch core failed!!!\n\000"
	.section	.text.bsp_swcore_init,"ax",@progbits
	.align	2
	.globl	bsp_swcore_init
	.ent	bsp_swcore_init
	.type	bsp_swcore_init, @function
bsp_swcore_init:
	.set	nomips16
	.frame	$sp,40,$31		# vars= 0, regs= 6/0, args= 16, gp= 0
	.mask	0x801f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-40
	sw	$31,36($sp)
	sw	$20,32($sp)
	sw	$19,28($sp)
	sw	$18,24($sp)
	sw	$17,20($sp)
	sw	$16,16($sp)
	move	$20,$4
	move	$19,$0
	li	$2,-1207959552			# 0xb8000000
	lw	$4,0($2)
	li	$3,-65536			# 0xffff0000
	and	$16,$4,$3
	ori	$2,$2,0xc
	lw	$17,0($2)
	andi	$18,$17,0xf
	lui	$4,%hi(fun_enable)
	jal	rtl865x_initAsicFun
	addiu	$4,$4,%lo(fun_enable)

	li	$3,-1			# 0xffffffff
	beq	$2,$3,$L36
	li	$4,-1			# 0xffffffff

	li	$2,-1073741824			# 0xc0000000
	bne	$16,$2,$L39
	li	$2,-2147483648			# 0x80000000

	li	$2,9			# 0x9
	bne	$20,$2,$L40
	nop

	beq	$18,$20,$L40
	andi	$2,$17,0x8

	beq	$2,$0,$L40
	lui	$4,%hi($LC0)

	addiu	$4,$4,%lo($LC0)
	jal	early_console_write
	li	$5,27			# 0x1b

	j	$L42
	li	$19,-1			# 0xffffffff

$L40:
	j	$L42
	move	$19,$0

$L39:
	bne	$16,$2,$L36
	move	$4,$19

	li	$2,7			# 0x7
	bne	$20,$2,$L50
	li	$2,15			# 0xf

	beq	$18,$20,$L50
	lui	$4,%hi($LC0)

	addiu	$4,$4,%lo($LC0)
	jal	early_console_write
	li	$5,27			# 0x1b

	j	$L42
	li	$19,-1			# 0xffffffff

$L50:
	bne	$20,$2,$L51
	li	$2,3			# 0x3

	beq	$18,$20,$L46
	li	$2,7			# 0x7

	beq	$18,$2,$L46
	li	$2,3			# 0x3

	beq	$18,$2,$L46
	li	$2,11			# 0xb

	beq	$18,$2,$L46
	lui	$4,%hi($LC0)

	addiu	$4,$4,%lo($LC0)
	jal	early_console_write
	li	$5,27			# 0x1b

	j	$L42
	li	$19,-1			# 0xffffffff

$L46:
	li	$2,3			# 0x3
$L51:
	bne	$20,$2,$L42
	move	$19,$0

	beq	$18,$20,$L42
	li	$2,7			# 0x7

	beq	$18,$2,$L52
	lui	$4,%hi($LC0)

	addiu	$4,$4,%lo($LC0)
	jal	early_console_write
	li	$5,27			# 0x1b

	li	$19,-1			# 0xffffffff
$L42:
$L52:
	move	$4,$19
$L38:
$L36:
	move	$2,$4
	lw	$31,36($sp)
	lw	$20,32($sp)
	lw	$19,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	j	$31
	addiu	$sp,$sp,40

	.set	macro
	.set	reorder
	.end	bsp_swcore_init
	.section	.text.rtl865x_accessAsicTable,"ax",@progbits
	.align	2
	.globl	rtl865x_accessAsicTable
	.ent	rtl865x_accessAsicTable
	.type	rtl865x_accessAsicTable, @function
rtl865x_accessAsicTable:
	.set	nomips16
	.frame	$sp,40,$31		# vars= 8, regs= 3/0, args= 16, gp= 0
	.mask	0x80030000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-40
	sw	$31,32($sp)
	sw	$17,28($sp)
	sw	$16,24($sp)
	move	$16,$4
	move	$17,$5
	jal	rtl865x_getAsicFun
	addiu	$4,$sp,16

	sltu	$2,$16,15
	beq	$2,$0,$L69
	li	$2,1			# 0x1

	li	$3,1			# 0x1
	sll	$4,$3,$16
	andi	$2,$4,0xe22
	bne	$2,$0,$L62
	andi	$2,$4,0x8

	bne	$2,$0,$L65
	andi	$2,$4,0x4000

	beq	$2,$0,$L68
	li	$2,1			# 0x1

	lw	$2,16($sp)
	andi	$2,$2,0x20
	movz	$3,$0,$2		#conditional move
	j	$L54
	sw	$3,0($17)

$L62:
	lw	$2,16($sp)
	andi	$2,$2,0x4
	beq	$2,$0,$L63
	li	$2,1			# 0x1

	j	$L54
	sw	$2,0($17)

$L63:
	j	$L54
	sw	$0,0($17)

$L65:
	lw	$2,16($sp)
	andi	$2,$2,0x2
	beq	$2,$0,$L66
	li	$2,1			# 0x1

	j	$L54
	sw	$2,0($17)

$L66:
	j	$L54
	sw	$0,0($17)

$L68:
$L69:
	sw	$2,0($17)
$L54:
	move	$2,$0
	lw	$31,32($sp)
	lw	$17,28($sp)
	lw	$16,24($sp)
	j	$31
	addiu	$sp,$sp,40

	.set	macro
	.set	reorder
	.end	rtl865x_accessAsicTable
	.rdata
	.align	2
$LC1:
	.ascii	"\n"
	.ascii	"Assert Fail: %s %d\000"
	.align	2
$LC2:
	.ascii	"drivers/net/rtl819x/rtl865x/../AsicDriver/rtl865x_asicBa"
	.ascii	"sic.c\000"
	.section	.text._rtl8651_asicTableAccessForward,"ax",@progbits
	.align	2
	.ent	_rtl8651_asicTableAccessForward
	.type	_rtl8651_asicTableAccessForward, @function
_rtl8651_asicTableAccessForward:
	.set	nomips16
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	bne	$6,$0,$L71
	sw	$31,16($sp)

	lui	$4,%hi($LC1)
	addiu	$4,$4,%lo($LC1)
	lui	$5,%hi($LC2)
	addiu	$5,$5,%lo($LC2)
	jal	panic_printk
	li	$6,324			# 0x144

$L72:
	j	$L72
	nop

$L71:
	li	$2,-1149239296			# 0xbb800000
	ori	$3,$2,0x4d00
$L74:
	lw	$2,0($3)
	andi	$2,$2,0x1
	bne	$2,$0,$L74
	sll	$2,$4,2

	move	$8,$2
	lui	$3,%hi(_rtl8651_asicTableSize)
	addiu	$3,$3,%lo(_rtl8651_asicTableSize)
	addu	$2,$2,$3
	lw	$2,0($2)
	beq	$2,$0,$L81
	move	$7,$0

	li	$9,-1149239296			# 0xbb800000
	addu	$8,$8,$3
	sll	$2,$7,2
$L82:
	addu	$3,$2,$9
	addu	$2,$2,$6
	lw	$2,0($2)
	sw	$2,19744($3)
	addiu	$7,$7,1
	lw	$2,0($8)
	sltu	$2,$7,$2
	bne	$2,$0,$L82
	sll	$2,$7,2

$L81:
	sll	$3,$4,16
	sll	$2,$5,5
	addu	$3,$3,$2
	li	$2,-1157627904			# 0xbb000000
	addu	$3,$3,$2
	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x4d08
	sw	$3,0($2)
	lw	$31,16($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	_rtl8651_asicTableAccessForward
	.section	.text._rtl8651_addAsicEntry,"ax",@progbits
	.align	2
	.globl	_rtl8651_addAsicEntry
	.ent	_rtl8651_addAsicEntry
	.type	_rtl8651_addAsicEntry, @function
_rtl8651_addAsicEntry:
	.set	nomips16
	.frame	$sp,40,$31		# vars= 8, regs= 4/0, args= 16, gp= 0
	.mask	0x80070000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-40
	sw	$31,36($sp)
	sw	$18,32($sp)
	sw	$17,28($sp)
	sw	$16,24($sp)
	move	$16,$4
	move	$17,$5
	move	$18,$6
	jal	rtl865x_accessAsicTable
	addiu	$5,$sp,16

	lw	$2,16($sp)
	beq	$2,$0,$L83
	li	$3,-1			# 0xffffffff

	move	$4,$16
	move	$5,$17
	jal	_rtl8651_asicTableAccessForward
	move	$6,$18

	lui	$2,%hi(RtkHomeGatewayChipNameID)
	lw	$3,%lo(RtkHomeGatewayChipNameID)($2)
	li	$2,2			# 0x2
	bne	$3,$2,$L99
	lui	$2,%hi(RtkHomeGatewayChipNameID)

	lui	$2,%hi(RtkHomeGatewayChipRevisionID)
	lw	$2,%lo(RtkHomeGatewayChipRevisionID)($2)
	bgtz	$2,$L100
	li	$3,-1149239296			# 0xbb800000

	lui	$2,%hi(RtkHomeGatewayChipNameID)
$L99:
	lw	$3,%lo(RtkHomeGatewayChipNameID)($2)
	li	$2,3			# 0x3
	beq	$3,$2,$L86
	li	$2,4			# 0x4

	bne	$3,$2,$L101
	li	$2,3			# 0x3

$L86:
	li	$3,-1149239296			# 0xbb800000
$L100:
	ori	$3,$3,0x4418
	lw	$2,0($3)
	li	$4,262144			# 0x40000
	or	$2,$2,$4
	sw	$2,0($3)
	move	$4,$3
	li	$3,524288			# 0x80000
$L88:
	lw	$2,0($4)
	and	$2,$2,$3
	beq	$2,$0,$L88
	li	$2,3			# 0x3

$L101:
	li	$3,-1149239296			# 0xbb800000
	ori	$3,$3,0x4d00
	sw	$2,0($3)
$L90:
	lw	$2,0($3)
	andi	$2,$2,0x1
	bne	$2,$0,$L90
	li	$2,-1149239296			# 0xbb800000

	ori	$2,$2,0x4d04
	lw	$2,0($2)
	andi	$2,$2,0x1
	beq	$2,$0,$L92
	lui	$2,%hi(RtkHomeGatewayChipNameID)

	lw	$3,%lo(RtkHomeGatewayChipNameID)($2)
	li	$2,2			# 0x2
	bne	$3,$2,$L102
	lui	$2,%hi(RtkHomeGatewayChipNameID)

	lui	$2,%hi(RtkHomeGatewayChipRevisionID)
	lw	$2,%lo(RtkHomeGatewayChipRevisionID)($2)
	bgtz	$2,$L94
	lui	$2,%hi(RtkHomeGatewayChipNameID)

$L102:
	lw	$3,%lo(RtkHomeGatewayChipNameID)($2)
	li	$2,3			# 0x3
	beq	$3,$2,$L94
	li	$2,4			# 0x4

	bne	$3,$2,$L93
	nop

$L94:
	li	$4,-1149239296			# 0xbb800000
	ori	$4,$4,0x4418
	lw	$3,0($4)
	li	$2,-327680			# 0xfffb0000
	ori	$2,$2,0xffff
	and	$3,$3,$2
	sw	$3,0($4)
$L93:
	j	$L83
	li	$3,-1			# 0xffffffff

$L92:
	lw	$3,%lo(RtkHomeGatewayChipNameID)($2)
	li	$2,2			# 0x2
	bne	$3,$2,$L103
	lui	$2,%hi(RtkHomeGatewayChipNameID)

	lui	$2,%hi(RtkHomeGatewayChipRevisionID)
	lw	$2,%lo(RtkHomeGatewayChipRevisionID)($2)
	bgtz	$2,$L104
	li	$4,-1149239296			# 0xbb800000

	lui	$2,%hi(RtkHomeGatewayChipNameID)
$L103:
	lw	$3,%lo(RtkHomeGatewayChipNameID)($2)
	li	$2,3			# 0x3
	beq	$3,$2,$L97
	li	$2,4			# 0x4

	bne	$3,$2,$L83
	move	$3,$0

$L97:
	li	$4,-1149239296			# 0xbb800000
$L104:
	ori	$4,$4,0x4418
	lw	$3,0($4)
	li	$2,-327680			# 0xfffb0000
	ori	$2,$2,0xffff
	and	$3,$3,$2
	sw	$3,0($4)
	move	$3,$0
$L83:
	move	$2,$3
	lw	$31,36($sp)
	lw	$18,32($sp)
	lw	$17,28($sp)
	lw	$16,24($sp)
	j	$31
	addiu	$sp,$sp,40

	.set	macro
	.set	reorder
	.end	_rtl8651_addAsicEntry
	.data
	.align	2
	.type	mcastForceAddOpCnt, @object
	.size	mcastForceAddOpCnt, 4
mcastForceAddOpCnt:
	.word	0
	.section	.text._rtl865x_getForceAddMcastOpCnt,"ax",@progbits
	.align	2
	.globl	_rtl865x_getForceAddMcastOpCnt
	.ent	_rtl865x_getForceAddMcastOpCnt
	.type	_rtl865x_getForceAddMcastOpCnt, @function
_rtl865x_getForceAddMcastOpCnt:
	.set	nomips16
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	lui	$2,%hi(mcastForceAddOpCnt)
	lw	$2,%lo(mcastForceAddOpCnt)($2)
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	_rtl865x_getForceAddMcastOpCnt
	.section	.text._rtl8651_forceAddAsicEntry,"ax",@progbits
	.align	2
	.globl	_rtl8651_forceAddAsicEntry
	.ent	_rtl8651_forceAddAsicEntry
	.type	_rtl8651_forceAddAsicEntry, @function
_rtl8651_forceAddAsicEntry:
	.set	nomips16
	.frame	$sp,40,$31		# vars= 8, regs= 4/0, args= 16, gp= 0
	.mask	0x80070000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-40
	sw	$31,36($sp)
	sw	$18,32($sp)
	sw	$17,28($sp)
	sw	$16,24($sp)
	move	$16,$4
	move	$17,$5
	move	$18,$6
	jal	rtl865x_accessAsicTable
	addiu	$5,$sp,16

	lw	$2,16($sp)
	beq	$2,$0,$L106
	li	$3,-1			# 0xffffffff

	li	$2,3			# 0x3
	bne	$16,$2,$L119
	lui	$2,%hi(RtkHomeGatewayChipNameID)

	lui	$3,%hi(mcastForceAddOpCnt)
	lw	$2,%lo(mcastForceAddOpCnt)($3)
	addiu	$2,$2,1
	sw	$2,%lo(mcastForceAddOpCnt)($3)
	lui	$2,%hi(RtkHomeGatewayChipNameID)
$L119:
	lw	$3,%lo(RtkHomeGatewayChipNameID)($2)
	li	$2,2			# 0x2
	bne	$3,$2,$L120
	lui	$2,%hi(RtkHomeGatewayChipNameID)

	lui	$2,%hi(RtkHomeGatewayChipRevisionID)
	lw	$2,%lo(RtkHomeGatewayChipRevisionID)($2)
	bgtz	$2,$L121
	li	$3,-1149239296			# 0xbb800000

	lui	$2,%hi(RtkHomeGatewayChipNameID)
$L120:
	lw	$3,%lo(RtkHomeGatewayChipNameID)($2)
	li	$2,3			# 0x3
	beq	$3,$2,$L110
	li	$2,4			# 0x4

	bne	$3,$2,$L122
	move	$4,$16

$L110:
	li	$3,-1149239296			# 0xbb800000
$L121:
	ori	$3,$3,0x4418
	lw	$2,0($3)
	li	$4,262144			# 0x40000
	or	$2,$2,$4
	sw	$2,0($3)
	move	$4,$3
	li	$3,524288			# 0x80000
$L112:
	lw	$2,0($4)
	and	$2,$2,$3
	beq	$2,$0,$L112
	nop

	move	$4,$16
$L122:
	move	$5,$17
	jal	_rtl8651_asicTableAccessForward
	move	$6,$18

	li	$2,9			# 0x9
	li	$3,-1149239296			# 0xbb800000
	ori	$3,$3,0x4d00
	sw	$2,0($3)
$L114:
	lw	$2,0($3)
	andi	$2,$2,0x1
	bne	$2,$0,$L114
	lui	$2,%hi(RtkHomeGatewayChipNameID)

	lw	$3,%lo(RtkHomeGatewayChipNameID)($2)
	li	$2,2			# 0x2
	bne	$3,$2,$L123
	lui	$2,%hi(RtkHomeGatewayChipNameID)

	lui	$2,%hi(RtkHomeGatewayChipRevisionID)
	lw	$2,%lo(RtkHomeGatewayChipRevisionID)($2)
	bgtz	$2,$L124
	li	$4,-1149239296			# 0xbb800000

	lui	$2,%hi(RtkHomeGatewayChipNameID)
$L123:
	lw	$3,%lo(RtkHomeGatewayChipNameID)($2)
	li	$2,3			# 0x3
	beq	$3,$2,$L117
	li	$2,4			# 0x4

	bne	$3,$2,$L106
	move	$3,$0

$L117:
	li	$4,-1149239296			# 0xbb800000
$L124:
	ori	$4,$4,0x4418
	lw	$3,0($4)
	li	$2,-327680			# 0xfffb0000
	ori	$2,$2,0xffff
	and	$3,$3,$2
	sw	$3,0($4)
	move	$3,$0
$L106:
	move	$2,$3
	lw	$31,36($sp)
	lw	$18,32($sp)
	lw	$17,28($sp)
	lw	$16,24($sp)
	j	$31
	addiu	$sp,$sp,40

	.set	macro
	.set	reorder
	.end	_rtl8651_forceAddAsicEntry
	.section	.text._rtl8651_readAsicEntry,"ax",@progbits
	.align	2
	.globl	_rtl8651_readAsicEntry
	.ent	_rtl8651_readAsicEntry
	.type	_rtl8651_readAsicEntry, @function
_rtl8651_readAsicEntry:
	.set	nomips16
	.frame	$sp,112,$31		# vars= 72, regs= 5/0, args= 16, gp= 0
	.mask	0x800f0000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-112
	sw	$31,104($sp)
	sw	$19,100($sp)
	sw	$18,96($sp)
	sw	$17,92($sp)
	sw	$16,88($sp)
	move	$17,$4
	move	$18,$5
	move	$19,$6
	li	$16,10			# 0xa
	jal	rtl865x_accessAsicTable
	addiu	$5,$sp,80

	lw	$2,80($sp)
	beq	$2,$0,$L125
	li	$3,-1			# 0xffffffff

	bne	$19,$0,$L127
	sll	$6,$17,16

	lui	$4,%hi($LC1)
	addiu	$4,$4,%lo($LC1)
	lui	$5,%hi($LC2)
	addiu	$5,$5,%lo($LC2)
	jal	panic_printk
	li	$6,462			# 0x1ce

$L128:
	j	$L128
	nop

$L127:
	sll	$2,$18,5
	addu	$6,$6,$2
	li	$2,-1157627904			# 0xbb000000
	addu	$6,$6,$2
	li	$2,-1149239296			# 0xbb800000
	ori	$3,$2,0x4d00
$L130:
	lw	$2,0($3)
	andi	$2,$2,0x1
	bne	$2,$0,$L130
	addiu	$10,$sp,16

	move	$12,$10
	li	$11,1			# 0x1
$L132:
	move	$8,$0
	sll	$3,$8,5
$L160:
	addu	$3,$3,$10
	lw	$2,0($6)
	sw	$2,0($3)
	lw	$2,4($6)
	sw	$2,4($3)
	lw	$2,8($6)
	sw	$2,8($3)
	lw	$2,12($6)
	sw	$2,12($3)
	lw	$2,16($6)
	sw	$2,16($3)
	lw	$2,20($6)
	sw	$2,20($3)
	lw	$2,24($6)
	sw	$2,24($3)
	lw	$2,28($6)
	sw	$2,28($3)
	addiu	$8,$8,1
	sltu	$2,$8,2
	bne	$2,$0,$L160
	sll	$3,$8,5

	move	$9,$0
	li	$8,1			# 0x1
	move	$5,$12
$L148:
	move	$4,$0
	sll	$7,$8,3
	addu	$3,$7,$4
$L161:
	sll	$3,$3,2
	addu	$3,$3,$5
	sll	$2,$4,2
	addu	$2,$2,$5
	lw	$3,0($3)
	lw	$2,0($2)
	beq	$3,$2,$L144
	addiu	$4,$4,1

	addiu	$4,$4,-1
	j	$L146
	li	$9,1			# 0x1

$L144:
	slt	$2,$4,8
	bne	$2,$0,$L161
	addu	$3,$7,$4

	addiu	$8,$8,1
	sltu	$2,$8,2
	bne	$2,$0,$L148
	nop

$L146:
	bne	$9,$11,$L133
	addiu	$16,$16,-1

	bne	$16,$0,$L132
	nop

$L133:
	li	$2,1			# 0x1
	sll	$2,$2,5
	addiu	$3,$sp,16
	addu	$6,$3,$2
	sll	$3,$17,2
	lui	$2,%hi(_rtl8651_asicTableSize)
	addiu	$2,$2,%lo(_rtl8651_asicTableSize)
	addu	$3,$3,$2
	lw	$2,0($3)
	beq	$2,$0,$L159
	move	$5,$0

	move	$4,$3
	sll	$2,$5,2
$L162:
	addu	$3,$2,$19
	addu	$2,$2,$6
	lw	$2,0($2)
	sw	$2,0($3)
	addiu	$5,$5,1
	lw	$2,0($4)
	sltu	$2,$5,$2
	bne	$2,$0,$L162
	sll	$2,$5,2

$L159:
	move	$3,$0
$L125:
	move	$2,$3
	lw	$31,104($sp)
	lw	$19,100($sp)
	lw	$18,96($sp)
	lw	$17,92($sp)
	lw	$16,88($sp)
	j	$31
	addiu	$sp,$sp,112

	.set	macro
	.set	reorder
	.end	_rtl8651_readAsicEntry
	.section	.text._rtl8651_readAsicEntryStopTLU,"ax",@progbits
	.align	2
	.globl	_rtl8651_readAsicEntryStopTLU
	.ent	_rtl8651_readAsicEntryStopTLU
	.type	_rtl8651_readAsicEntryStopTLU, @function
_rtl8651_readAsicEntryStopTLU:
	.set	nomips16
	.frame	$sp,40,$31		# vars= 8, regs= 4/0, args= 16, gp= 0
	.mask	0x80070000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-40
	sw	$31,36($sp)
	sw	$18,32($sp)
	sw	$17,28($sp)
	sw	$16,24($sp)
	move	$16,$4
	move	$17,$5
	move	$18,$6
	jal	rtl865x_accessAsicTable
	addiu	$5,$sp,16

	lw	$2,16($sp)
	beq	$2,$0,$L163
	li	$3,-1			# 0xffffffff

	bne	$18,$0,$L165
	sll	$6,$16,16

	lui	$4,%hi($LC1)
	addiu	$4,$4,%lo($LC1)
	lui	$5,%hi($LC2)
	addiu	$5,$5,%lo($LC2)
	jal	panic_printk
	li	$6,557			# 0x22d

$L166:
	j	$L166
	nop

$L165:
	sll	$2,$17,5
	addu	$6,$6,$2
	li	$2,-1157627904			# 0xbb000000
	addu	$6,$6,$2
	lui	$2,%hi(RtkHomeGatewayChipNameID)
	lw	$3,%lo(RtkHomeGatewayChipNameID)($2)
	li	$2,2			# 0x2
	bne	$3,$2,$L180
	lui	$2,%hi(RtkHomeGatewayChipNameID)

	lui	$2,%hi(RtkHomeGatewayChipRevisionID)
	lw	$2,%lo(RtkHomeGatewayChipRevisionID)($2)
	bgtz	$2,$L181
	li	$2,-1149239296			# 0xbb800000

	lui	$2,%hi(RtkHomeGatewayChipNameID)
$L180:
	lw	$3,%lo(RtkHomeGatewayChipNameID)($2)
	li	$2,3			# 0x3
	beq	$3,$2,$L169
	li	$2,4			# 0x4

	bne	$3,$2,$L182
	sll	$2,$16,2

$L169:
	li	$2,-1149239296			# 0xbb800000
$L181:
	ori	$2,$2,0x4418
	lw	$3,0($2)
	li	$4,262144			# 0x40000
	or	$3,$3,$4
	sw	$3,0($2)
	sll	$2,$16,2
$L182:
	move	$4,$2
	lui	$3,%hi(_rtl8651_asicTableSize)
	addiu	$3,$3,%lo(_rtl8651_asicTableSize)
	addu	$2,$2,$3
	lw	$2,0($2)
	beq	$2,$0,$L179
	move	$5,$0

	addu	$4,$4,$3
	sll	$2,$5,2
$L183:
	addu	$3,$2,$18
	addu	$2,$2,$6
	lw	$2,0($2)
	sw	$2,0($3)
	addiu	$5,$5,1
	lw	$2,0($4)
	sltu	$2,$5,$2
	bne	$2,$0,$L183
	sll	$2,$5,2

$L179:
	lui	$2,%hi(RtkHomeGatewayChipNameID)
	lw	$3,%lo(RtkHomeGatewayChipNameID)($2)
	li	$2,2			# 0x2
	bne	$3,$2,$L184
	lui	$2,%hi(RtkHomeGatewayChipNameID)

	lui	$2,%hi(RtkHomeGatewayChipRevisionID)
	lw	$2,%lo(RtkHomeGatewayChipRevisionID)($2)
	bgtz	$2,$L176
	lui	$2,%hi(RtkHomeGatewayChipNameID)

$L184:
	lw	$3,%lo(RtkHomeGatewayChipNameID)($2)
	li	$2,3			# 0x3
	beq	$3,$2,$L176
	li	$2,4			# 0x4

	bne	$3,$2,$L163
	move	$3,$0

$L176:
	li	$4,-1149239296			# 0xbb800000
	ori	$4,$4,0x4418
	lw	$3,0($4)
	li	$2,-327680			# 0xfffb0000
	ori	$2,$2,0xffff
	and	$3,$3,$2
	sw	$3,0($4)
	move	$3,$0
$L163:
	move	$2,$3
	lw	$31,36($sp)
	lw	$18,32($sp)
	lw	$17,28($sp)
	lw	$16,24($sp)
	j	$31
	addiu	$sp,$sp,40

	.set	macro
	.set	reorder
	.end	_rtl8651_readAsicEntryStopTLU
	.section	.text._rtl8651_delAsicEntry,"ax",@progbits
	.align	2
	.globl	_rtl8651_delAsicEntry
	.ent	_rtl8651_delAsicEntry
	.type	_rtl8651_delAsicEntry, @function
_rtl8651_delAsicEntry:
	.set	nomips16
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	li	$2,-1149239296			# 0xbb800000
	ori	$3,$2,0x4d00
$L186:
	lw	$2,0($3)
	andi	$2,$2,0x1
	bne	$2,$0,$L186
	sll	$2,$4,2

	move	$7,$2
	lui	$3,%hi(_rtl8651_asicTableSize)
	addiu	$3,$3,%lo(_rtl8651_asicTableSize)
	addu	$2,$2,$3
	lw	$2,0($2)
	beq	$2,$0,$L199
	move	$3,$0

	li	$8,-1149239296			# 0xbb800000
	lui	$2,%hi(_rtl8651_asicTableSize)
	addiu	$2,$2,%lo(_rtl8651_asicTableSize)
	addu	$7,$7,$2
	sll	$2,$3,2
$L202:
	addu	$2,$2,$8
	sw	$0,19744($2)
	addiu	$3,$3,1
	lw	$2,0($7)
	sltu	$2,$3,$2
	bne	$2,$0,$L202
	sll	$2,$3,2

$L199:
	sltu	$2,$6,$5
	bne	$2,$0,$L201
	nop

	li	$2,-1149239296			# 0xbb800000
	ori	$10,$2,0x4d08
	sll	$4,$4,16
	li	$9,-1157627904			# 0xbb000000
	ori	$3,$2,0x4d00
	li	$8,9			# 0x9
	ori	$7,$2,0x4d04
$L197:
	sll	$2,$5,5
	addu	$2,$4,$2
	addu	$2,$2,$9
	sw	$2,0($10)
	sw	$8,0($3)
$L194:
	lw	$2,0($3)
	andi	$2,$2,0x1
	bne	$2,$0,$L194
	nop

	lw	$2,0($7)
	andi	$2,$2,0x1
	beq	$2,$0,$L196
	addiu	$5,$5,1

	addiu	$5,$5,-1
	j	$31
	li	$2,-1			# 0xffffffff

$L196:
	sltu	$2,$6,$5
	beq	$2,$0,$L197
	nop

$L201:
	j	$31
	move	$2,$0

	.set	macro
	.set	reorder
	.end	_rtl8651_delAsicEntry
	.data
	.align	2
	.type	rtl819x_lastTxDesc, @object
	.size	rtl819x_lastTxDesc, 4
rtl819x_lastTxDesc:
	.word	0
	.align	2
	.type	rtl819x_lastRxDesc, @object
	.size	rtl819x_lastRxDesc, 4
rtl819x_lastRxDesc:
	.word	0
	.align	2
	.type	rtl819x_swHangCnt, @object
	.size	rtl819x_swHangCnt, 4
rtl819x_swHangCnt:
	.word	0
	.align	2
	.type	rtl819x_checkSwCoreTimer, @object
	.size	rtl819x_checkSwCoreTimer, 4
rtl819x_checkSwCoreTimer:
	.word	0
	.rdata
	.align	2
$LC3:
	.ascii	".........................................\n\000"
	.section	.text.rtl819x_poll_sw,"ax",@progbits
	.align	2
	.globl	rtl819x_poll_sw
	.ent	rtl819x_poll_sw
	.type	rtl819x_poll_sw, @function
rtl819x_poll_sw:
	.set	nomips16
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,16($sp)
	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x610c
	lw	$3,0($2)
	li	$2,65536			# 0x10000
	and	$8,$3,$2
	li	$3,-1207894016			# 0xb8010000
	ori	$2,$3,0x4
	lw	$2,0($2)
	li	$4,-4			# 0xfffffffc
	and	$6,$2,$4
	ori	$3,$3,0x20
	lw	$2,0($3)
	and	$7,$2,$4
	lui	$5,%hi(rtl819x_checkSwCoreTimer)
	lw	$4,%lo(rtl819x_checkSwCoreTimer)($5)
	li	$2,-859045888			# 0xcccc0000
	ori	$2,$2,0xcccd
	multu	$4,$2
	mfhi	$2
	srl	$2,$2,4
	sll	$3,$2,2
	addu	$3,$3,$2
	sll	$3,$3,2
	subu	$3,$4,$3
	addiu	$4,$4,1
	bne	$3,$0,$L203
	sw	$4,%lo(rtl819x_checkSwCoreTimer)($5)

	bne	$8,$0,$L205
	lui	$2,%hi(rtl819x_lastTxDesc)

	lui	$2,%hi(rtl819x_swHangCnt)
	j	$L206
	sw	$0,%lo(rtl819x_swHangCnt)($2)

$L205:
	lw	$2,%lo(rtl819x_lastTxDesc)($2)
	beq	$2,$0,$L213
	lui	$2,%hi(rtl819x_lastRxDesc)

	lw	$2,%lo(rtl819x_lastRxDesc)($2)
	bne	$2,$0,$L207
	lui	$2,%hi(rtl819x_lastRxDesc)

$L213:
	sw	$6,%lo(rtl819x_lastRxDesc)($2)
	lui	$2,%hi(rtl819x_lastTxDesc)
	sw	$7,%lo(rtl819x_lastTxDesc)($2)
	lui	$2,%hi(rtl819x_swHangCnt)
	j	$L206
	sw	$0,%lo(rtl819x_swHangCnt)($2)

$L207:
	lw	$2,%lo(rtl819x_lastRxDesc)($2)
	bne	$2,$6,$L214
	lui	$2,%hi(rtl819x_swHangCnt)

	lui	$2,%hi(rtl819x_lastTxDesc)
	lw	$2,%lo(rtl819x_lastTxDesc)($2)
	bne	$2,$7,$L210
	lui	$2,%hi(rtl819x_swHangCnt)

	lui	$3,%hi(rtl819x_swHangCnt)
	lw	$2,%lo(rtl819x_swHangCnt)($3)
	addiu	$2,$2,1
	j	$L206
	sw	$2,%lo(rtl819x_swHangCnt)($3)

$L210:
$L214:
	sw	$0,%lo(rtl819x_swHangCnt)($2)
$L206:
	lui	$2,%hi(rtl819x_lastRxDesc)
	sw	$6,%lo(rtl819x_lastRxDesc)($2)
	lui	$2,%hi(rtl819x_lastTxDesc)
	sw	$7,%lo(rtl819x_lastTxDesc)($2)
	lui	$3,%hi(rtl819x_swHangCnt)
	lw	$2,%lo(rtl819x_swHangCnt)($3)
	sltu	$2,$2,3
	bne	$2,$0,$L203
	nop

	sw	$0,%lo(rtl819x_swHangCnt)($3)
	lui	$4,%hi($LC3)
	jal	panic_printk
	addiu	$4,$4,%lo($LC3)

	jal	machine_restart
	move	$4,$0

$L203:
	lw	$31,16($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	rtl819x_poll_sw
	.globl	RtkHomeGatewayChipName
	.section	.bss
	.align	2
	.type	RtkHomeGatewayChipName, @object
	.size	RtkHomeGatewayChipName, 16
RtkHomeGatewayChipName:
	.space	16
	.globl	RtkHomeGatewayChipNameID
	.align	2
	.type	RtkHomeGatewayChipNameID, @object
	.size	RtkHomeGatewayChipNameID, 4
RtkHomeGatewayChipNameID:
	.space	4
	.globl	RtkHomeGatewayChipRevisionID
	.align	2
	.type	RtkHomeGatewayChipRevisionID, @object
	.size	RtkHomeGatewayChipRevisionID, 4
RtkHomeGatewayChipRevisionID:
	.space	4
	.ident	"GCC: (GNU) 3.4.6-1.3.6"
