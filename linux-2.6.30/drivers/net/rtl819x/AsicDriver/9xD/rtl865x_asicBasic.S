	.file	1 "rtl865x_asicBasic.c"
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
	.section	.text.prom_putchar,"ax",@progbits
	.align	2
	.set	nomips16
	.ent	prom_putchar
	.type	prom_putchar, @function
prom_putchar:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	li	$2,1			# 0x1
	li	$5,-1207959552			# 0xb8000000
	ori	$5,$5,0x2014
	j	$L2
	li	$6,30000			# 0x7530

$L5:
	bne	$2,$6,$L3
	addiu	$2,$2,1

	li	$3,-60			# 0xffffffc4
	li	$2,-1207959552			# 0xb8000000
	ori	$2,$2,0x2008
	sb	$3,0($2)
	j	$31
	nop

$L3:
$L2:
	lbu	$3,0($5)
	andi	$3,$3,0x20
	beq	$3,$0,$L5
	nop

	andi	$4,$4,0x00ff
	li	$2,-1207959552			# 0xb8000000
	ori	$2,$2,0x2000
	sb	$4,0($2)
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	prom_putchar
	.size	prom_putchar, .-prom_putchar
	.section	.text.early_console_write,"ax",@progbits
	.align	2
	.set	nomips16
	.ent	early_console_write
	.type	early_console_write, @function
early_console_write:
	.frame	$sp,32,$31		# vars= 0, regs= 4/0, args= 16, gp= 0
	.mask	0x80070000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$18,24($sp)
	sw	$17,20($sp)
	sw	$16,16($sp)
	beq	$5,$0,$L11
	move	$16,$4

	lb	$2,0($4)
	beq	$2,$0,$L11
	addiu	$17,$5,-1

	li	$18,10			# 0xa
$L10:
	bne	$2,$18,$L9
	nop

	jal	prom_putchar
	li	$4,13			# 0xd

$L9:
	lb	$4,0($16)
	jal	prom_putchar
	nop

	beq	$17,$0,$L11
	addiu	$16,$16,1

	lb	$2,0($16)
	bne	$2,$0,$L10
	addiu	$17,$17,-1

$L11:
	lw	$31,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	j	$31
	addiu	$sp,$sp,32

	.set	macro
	.set	reorder
	.end	early_console_write
	.size	early_console_write, .-early_console_write
	.section	.text.rtl865x_initAsicFun,"ax",@progbits
	.align	2
	.globl	rtl865x_initAsicFun
	.set	nomips16
	.ent	rtl865x_initAsicFun
	.type	rtl865x_initAsicFun, @function
rtl865x_initAsicFun:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	sw	$0,0($4)
	li	$3,-1207959552			# 0xb8000000
	lw	$5,0($3)
	li	$2,-65536			# 0xffff0000
	and	$2,$5,$2
	ori	$3,$3,0xc
	lw	$3,0($3)
	li	$5,-1073741824			# 0xc0000000
	bne	$2,$5,$L13
	andi	$3,$3,0xf

	li	$2,9			# 0x9
	beq	$3,$2,$L29
	li	$2,46			# 0x2e

	andi	$2,$3,0x8
	bne	$2,$0,$L15
	li	$2,46			# 0x2e

$L29:
	sw	$2,0($4)
	j	$31
	move	$2,$0

$L15:
	li	$2,8			# 0x8
	bne	$3,$2,$L17
	li	$2,14			# 0xe

	sw	$2,0($4)
	j	$31
	move	$2,$0

$L17:
	addiu	$3,$3,-10
	sltu	$3,$3,2
	beq	$3,$0,$L18
	nop

	li	$2,10			# 0xa
	sw	$2,0($4)
	j	$31
	move	$2,$0

$L18:
	li	$2,14			# 0xe
	sw	$2,0($4)
	j	$31
	move	$2,$0

$L13:
	li	$5,-2147483648			# 0x80000000
	bne	$2,$5,$L19
	nop

	li	$2,7			# 0x7
	bne	$3,$2,$L20
	li	$2,46			# 0x2e

	sw	$2,0($4)
	j	$31
	move	$2,$0

$L20:
	li	$2,15			# 0xf
	bne	$3,$2,$L21
	li	$2,3			# 0x3

	sw	$2,0($4)
	j	$31
	move	$2,$0

$L21:
	bne	$3,$2,$L22
	nop

	sw	$2,0($4)
	j	$31
	move	$2,$0

$L22:
	sw	$2,0($4)
	j	$31
	move	$2,$0

$L19:
	li	$5,-2120810496			# 0x81970000
	beq	$2,$5,$L23
	li	$2,-1			# 0xffffffff

	j	$31
	nop

$L23:
	li	$2,15			# 0xf
	beq	$3,$2,$L30
	li	$2,3			# 0x3

	li	$2,8			# 0x8
	bne	$3,$2,$L25
	li	$2,3			# 0x3

$L30:
	sw	$2,0($4)
	j	$31
	move	$2,$0

$L25:
	li	$2,14			# 0xe
	beq	$3,$2,$L31
	li	$2,46			# 0x2e

	li	$2,9			# 0x9
	beq	$3,$2,$L26
	andi	$3,$3,0x8

	bne	$3,$0,$L27
	nop

$L26:
	li	$2,46			# 0x2e
$L31:
	sw	$2,0($4)
	j	$31
	move	$2,$0

$L27:
	li	$2,3			# 0x3
	sw	$2,0($4)
	j	$31
	move	$2,$0

	.set	macro
	.set	reorder
	.end	rtl865x_initAsicFun
	.size	rtl865x_initAsicFun, .-rtl865x_initAsicFun
	.section	.text.rtl865x_getAsicFun,"ax",@progbits
	.align	2
	.globl	rtl865x_getAsicFun
	.set	nomips16
	.ent	rtl865x_getAsicFun
	.type	rtl865x_getAsicFun, @function
rtl865x_getAsicFun:
	.frame	$sp,32,$31		# vars= 0, regs= 3/0, args= 16, gp= 0
	.mask	0x80030000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$17,24($sp)
	sw	$16,20($sp)
	lui	$2,%hi(fun_enable)
	lw	$3,%lo(fun_enable)($2)
	sw	$3,0($4)
	lw	$2,%lo(fun_enable)($2)
	bne	$2,$0,$L35
	move	$17,$4

	lui	$16,%hi(fun_enable)
	jal	rtl865x_initAsicFun
	addiu	$4,$16,%lo(fun_enable)

	lw	$2,%lo(fun_enable)($16)
	sw	$2,0($17)
$L35:
	move	$2,$0
	lw	$31,28($sp)
	lw	$17,24($sp)
	lw	$16,20($sp)
	j	$31
	addiu	$sp,$sp,32

	.set	macro
	.set	reorder
	.end	rtl865x_getAsicFun
	.size	rtl865x_getAsicFun, .-rtl865x_getAsicFun
	.section	.rodata.str1.4,"aMS",@progbits,1
	.align	2
$LC0:
	.ascii	"init switch core failed!!!\012\000"
	.section	.text.bsp_swcore_init,"ax",@progbits
	.align	2
	.globl	bsp_swcore_init
	.set	nomips16
	.ent	bsp_swcore_init
	.type	bsp_swcore_init, @function
bsp_swcore_init:
	.frame	$sp,32,$31		# vars= 0, regs= 4/0, args= 16, gp= 0
	.mask	0x80070000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$18,24($sp)
	sw	$17,20($sp)
	sw	$16,16($sp)
	move	$16,$4
	li	$2,-1207959552			# 0xb8000000
	lw	$17,0($2)
	ori	$2,$2,0xc
	lw	$18,0($2)
	lui	$4,%hi(fun_enable)
	jal	rtl865x_initAsicFun
	addiu	$4,$4,%lo(fun_enable)

	li	$3,-1			# 0xffffffff
	beq	$2,$3,$L39
	li	$2,-1			# 0xffffffff

	li	$2,-65536			# 0xffff0000
	and	$17,$17,$2
	li	$2,-1073741824			# 0xc0000000
	bne	$17,$2,$L40
	andi	$18,$18,0xf

	li	$2,9			# 0x9
	bne	$16,$2,$L41
	nop

	beq	$18,$2,$L41
	andi	$18,$18,0x8

	beq	$18,$0,$L41
	lui	$4,%hi($LC0)

	addiu	$4,$4,%lo($LC0)
	jal	early_console_write
	li	$5,27			# 0x1b

	j	$L39
	li	$2,-1			# 0xffffffff

$L40:
	li	$2,-2147483648			# 0x80000000
	bne	$17,$2,$L42
	li	$2,-2120810496			# 0x81970000

	li	$2,7			# 0x7
	bne	$16,$2,$L43
	nop

	beq	$18,$2,$L41
	lui	$4,%hi($LC0)

	addiu	$4,$4,%lo($LC0)
	jal	early_console_write
	li	$5,27			# 0x1b

	j	$L39
	li	$2,-1			# 0xffffffff

$L43:
	li	$2,15			# 0xf
	bne	$16,$2,$L44
	nop

	beq	$18,$2,$L44
	li	$2,7			# 0x7

	beq	$18,$2,$L41
	li	$2,3			# 0x3

	beq	$18,$2,$L41
	li	$2,11			# 0xb

	beq	$18,$2,$L41
	lui	$4,%hi($LC0)

	addiu	$4,$4,%lo($LC0)
	jal	early_console_write
	li	$5,27			# 0x1b

	j	$L39
	li	$2,-1			# 0xffffffff

$L44:
	li	$2,3			# 0x3
	bne	$16,$2,$L41
	nop

	beq	$18,$2,$L41
	li	$2,7			# 0x7

	beq	$18,$2,$L41
	lui	$4,%hi($LC0)

	addiu	$4,$4,%lo($LC0)
	jal	early_console_write
	li	$5,27			# 0x1b

	j	$L39
	li	$2,-1			# 0xffffffff

$L42:
	bne	$17,$2,$L47
	move	$2,$0

	li	$2,8			# 0x8
	bne	$16,$2,$L48
	li	$2,11			# 0xb

	li	$2,15			# 0xf
	beq	$18,$2,$L45
	li	$2,8			# 0x8

	beq	$18,$2,$L41
	li	$2,14			# 0xe

	beq	$18,$2,$L41
	li	$2,9			# 0x9

	beq	$18,$2,$L41
	andi	$18,$18,0x8

	beq	$18,$0,$L41
	lui	$4,%hi($LC0)

	addiu	$4,$4,%lo($LC0)
	jal	early_console_write
	li	$5,27			# 0x1b

	j	$L39
	li	$2,-1			# 0xffffffff

$L45:
	li	$2,11			# 0xb
$L48:
	bne	$16,$2,$L47
	move	$2,$0

	li	$2,14			# 0xe
	beq	$18,$2,$L41
	li	$2,9			# 0x9

	beq	$18,$2,$L41
	andi	$18,$18,0x8

	beq	$18,$0,$L41
	lui	$4,%hi($LC0)

	addiu	$4,$4,%lo($LC0)
	jal	early_console_write
	li	$5,27			# 0x1b

	j	$L39
	li	$2,-1			# 0xffffffff

$L41:
	move	$2,$0
$L39:
$L47:
	lw	$31,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	j	$31
	addiu	$sp,$sp,32

	.set	macro
	.set	reorder
	.end	bsp_swcore_init
	.size	bsp_swcore_init, .-bsp_swcore_init
	.section	.text.rtl865x_accessAsicTable,"ax",@progbits
	.align	2
	.globl	rtl865x_accessAsicTable
	.set	nomips16
	.ent	rtl865x_accessAsicTable
	.type	rtl865x_accessAsicTable, @function
rtl865x_accessAsicTable:
	.frame	$sp,40,$31		# vars= 8, regs= 3/0, args= 16, gp= 0
	.mask	0x80030000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-40
	sw	$31,36($sp)
	sw	$17,32($sp)
	sw	$16,28($sp)
	move	$16,$4
	move	$17,$5
	jal	rtl865x_getAsicFun
	addiu	$4,$sp,16

	sltu	$2,$16,15
	beq	$2,$0,$L50
	li	$2,1			# 0x1

	sll	$16,$2,$16
	andi	$2,$16,0xe22
	bne	$2,$0,$L51
	andi	$2,$16,0x4000

	bne	$2,$0,$L53
	andi	$16,$16,0x8

	bne	$16,$0,$L52
	li	$2,1			# 0x1

	j	$L55
	sw	$2,0($17)

$L53:
	lw	$2,16($sp)
	andi	$2,$2,0x20
	beq	$2,$0,$L54
	li	$2,1			# 0x1

	j	$L55
	sw	$2,0($17)

$L54:
	j	$L55
	sw	$0,0($17)

$L51:
	lw	$2,16($sp)
	andi	$2,$2,0x4
	beq	$2,$0,$L56
	li	$2,1			# 0x1

	j	$L55
	sw	$2,0($17)

$L56:
	j	$L55
	sw	$0,0($17)

$L52:
	lw	$2,16($sp)
	andi	$2,$2,0x2
	beq	$2,$0,$L57
	li	$2,1			# 0x1

	j	$L55
	sw	$2,0($17)

$L57:
	j	$L55
	sw	$0,0($17)

$L50:
	sw	$2,0($17)
$L55:
	move	$2,$0
	lw	$31,36($sp)
	lw	$17,32($sp)
	lw	$16,28($sp)
	j	$31
	addiu	$sp,$sp,40

	.set	macro
	.set	reorder
	.end	rtl865x_accessAsicTable
	.size	rtl865x_accessAsicTable, .-rtl865x_accessAsicTable
	.section	.text._rtl865x_getForceAddMcastOpCnt,"ax",@progbits
	.align	2
	.globl	_rtl865x_getForceAddMcastOpCnt
	.set	nomips16
	.ent	_rtl865x_getForceAddMcastOpCnt
	.type	_rtl865x_getForceAddMcastOpCnt, @function
_rtl865x_getForceAddMcastOpCnt:
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
	.size	_rtl865x_getForceAddMcastOpCnt, .-_rtl865x_getForceAddMcastOpCnt
	.section	.text._rtl8651_delAsicEntry,"ax",@progbits
	.align	2
	.globl	_rtl8651_delAsicEntry
	.set	nomips16
	.ent	_rtl8651_delAsicEntry
	.type	_rtl8651_delAsicEntry, @function
_rtl8651_delAsicEntry:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	li	$3,-1149239296			# 0xbb800000
	ori	$3,$3,0x4d00
$L63:
	lw	$2,0($3)
	andi	$2,$2,0x1
	bne	$2,$0,$L63
	lui	$2,%hi(_rtl8651_asicTableSize)

	sll	$3,$4,2
	addiu	$2,$2,%lo(_rtl8651_asicTableSize)
	addu	$2,$3,$2
	lw	$7,0($2)
	beq	$7,$0,$L64
	move	$2,$0

	li	$8,-1149239296			# 0xbb800000
	ori	$8,$8,0x4d20
	sll	$3,$2,2
$L76:
	addu	$3,$3,$8
	sw	$0,0($3)
	addiu	$2,$2,1
	sltu	$3,$2,$7
	bne	$3,$0,$L76
	sll	$3,$2,2

$L64:
	sltu	$2,$6,$5
	bne	$2,$0,$L66
	nop

	sll	$4,$4,16
	li	$2,-1157627904			# 0xbb000000
	addu	$4,$4,$2
	sll	$2,$5,5
	addu	$4,$4,$2
	li	$7,-1149239296			# 0xbb800000
	ori	$9,$7,0x4d08
	ori	$3,$7,0x4d00
	li	$8,9			# 0x9
	ori	$7,$7,0x4d04
$L70:
	sw	$4,0($9)
	sw	$8,0($3)
$L67:
	lw	$2,0($3)
	andi	$2,$2,0x1
	bne	$2,$0,$L67
	nop

	lw	$2,0($7)
	andi	$2,$2,0x1
	beq	$2,$0,$L68
	addiu	$5,$5,1

	addiu	$5,$5,-1
	j	$31
	li	$2,-1			# 0xffffffff

$L68:
	sltu	$2,$6,$5
	beq	$2,$0,$L70
	addiu	$4,$4,32

$L66:
	j	$31
	move	$2,$0

	.set	macro
	.set	reorder
	.end	_rtl8651_delAsicEntry
	.size	_rtl8651_delAsicEntry, .-_rtl8651_delAsicEntry
	.section	.rodata.str1.4
	.align	2
$LC1:
	.ascii	"\012Assert Fail: %s %d\000"
	.align	2
$LC2:
	.ascii	"drivers/net/rtl819x/rtl865x/../AsicDriver/rtl865x_asicBa"
	.ascii	"sic.c\000"
	.section	.text._rtl8651_readAsicEntryStopTLU,"ax",@progbits
	.align	2
	.globl	_rtl8651_readAsicEntryStopTLU
	.set	nomips16
	.ent	_rtl8651_readAsicEntryStopTLU
	.type	_rtl8651_readAsicEntryStopTLU, @function
_rtl8651_readAsicEntryStopTLU:
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
	move	$17,$4
	move	$18,$5
	move	$16,$6
	jal	rtl865x_accessAsicTable
	addiu	$5,$sp,16

	lw	$2,16($sp)
	beq	$2,$0,$L79
	li	$2,-1			# 0xffffffff

	bne	$16,$0,$L80
	sll	$3,$17,16

	lui	$4,%hi($LC1)
	addiu	$4,$4,%lo($LC1)
	lui	$5,%hi($LC2)
	addiu	$5,$5,%lo($LC2)
	jal	panic_printk
	li	$6,557			# 0x22d

$L81:
	j	$L81
	nop

$L80:
	li	$2,-1157627904			# 0xbb000000
	addu	$2,$3,$2
	sll	$18,$18,5
	addu	$18,$2,$18
	lui	$2,%hi(RtkHomeGatewayChipNameID)
	lw	$2,%lo(RtkHomeGatewayChipNameID)($2)
	li	$3,2			# 0x2
	bne	$2,$3,$L82
	li	$3,3			# 0x3

	lui	$2,%hi(RtkHomeGatewayChipRevisionID)
	lw	$2,%lo(RtkHomeGatewayChipRevisionID)($2)
	bgtz	$2,$L94
	li	$2,-1149239296			# 0xbb800000

	j	$L92
	sll	$3,$17,2

$L82:
	beq	$2,$3,$L83
	li	$3,4			# 0x4

	bne	$2,$3,$L92
	sll	$3,$17,2

$L83:
	li	$2,-1149239296			# 0xbb800000
$L94:
	ori	$2,$2,0x4418
	lw	$4,0($2)
	li	$3,262144			# 0x40000
	or	$3,$4,$3
	sw	$3,0($2)
	sll	$3,$17,2
$L92:
	lui	$2,%hi(_rtl8651_asicTableSize)
	addiu	$2,$2,%lo(_rtl8651_asicTableSize)
	addu	$2,$3,$2
	lw	$2,0($2)
	beq	$2,$0,$L85
	move	$2,$0

	move	$17,$3
	lui	$3,%hi(_rtl8651_asicTableSize)
	addiu	$3,$3,%lo(_rtl8651_asicTableSize)
	addu	$17,$17,$3
	lw	$4,0($17)
$L86:
	lw	$3,0($18)
	sw	$3,0($16)
	addiu	$2,$2,1
	addiu	$18,$18,4
	sltu	$3,$2,$4
	bne	$3,$0,$L86
	addiu	$16,$16,4

$L85:
	lui	$2,%hi(RtkHomeGatewayChipNameID)
	lw	$2,%lo(RtkHomeGatewayChipNameID)($2)
	li	$3,2			# 0x2
	bne	$2,$3,$L87
	nop

	lui	$2,%hi(RtkHomeGatewayChipRevisionID)
	lw	$2,%lo(RtkHomeGatewayChipRevisionID)($2)
	bgtz	$2,$L88
	nop

	j	$L93
	move	$2,$0

$L87:
	li	$3,3			# 0x3
	beq	$2,$3,$L88
	li	$3,4			# 0x4

	bne	$2,$3,$L89
	nop

$L88:
	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x4418
	lw	$4,0($2)
	li	$3,-327680			# 0xfffb0000
	ori	$3,$3,0xffff
	and	$3,$4,$3
	sw	$3,0($2)
	j	$L79
	move	$2,$0

$L89:
	move	$2,$0
$L79:
$L93:
	lw	$31,36($sp)
	lw	$18,32($sp)
	lw	$17,28($sp)
	lw	$16,24($sp)
	j	$31
	addiu	$sp,$sp,40

	.set	macro
	.set	reorder
	.end	_rtl8651_readAsicEntryStopTLU
	.size	_rtl8651_readAsicEntryStopTLU, .-_rtl8651_readAsicEntryStopTLU
	.section	.text._rtl8651_readAsicEntry,"ax",@progbits
	.align	2
	.globl	_rtl8651_readAsicEntry
	.set	nomips16
	.ent	_rtl8651_readAsicEntry
	.type	_rtl8651_readAsicEntry, @function
_rtl8651_readAsicEntry:
	.frame	$sp,104,$31		# vars= 72, regs= 4/0, args= 16, gp= 0
	.mask	0x80070000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-104
	sw	$31,100($sp)
	sw	$18,96($sp)
	sw	$17,92($sp)
	sw	$16,88($sp)
	move	$17,$4
	move	$18,$5
	move	$16,$6
	jal	rtl865x_accessAsicTable
	addiu	$5,$sp,16

	lw	$2,16($sp)
	beq	$2,$0,$L97
	li	$2,-1			# 0xffffffff

	bne	$16,$0,$L98
	sll	$3,$17,16

	lui	$4,%hi($LC1)
	addiu	$4,$4,%lo($LC1)
	lui	$5,%hi($LC2)
	addiu	$5,$5,%lo($LC2)
	jal	panic_printk
	li	$6,462			# 0x1ce

$L99:
	j	$L99
	nop

$L98:
	li	$2,-1157627904			# 0xbb000000
	addu	$2,$3,$2
	sll	$18,$18,5
	addu	$18,$2,$18
	li	$3,-1149239296			# 0xbb800000
	ori	$3,$3,0x4d00
$L100:
	lw	$2,0($3)
	andi	$2,$2,0x1
	bne	$2,$0,$L100
	li	$14,10			# 0xa

	addiu	$12,$18,4
	addiu	$11,$18,8
	addiu	$10,$18,12
	addiu	$9,$18,16
	addiu	$8,$18,20
	addiu	$7,$18,24
	addiu	$6,$18,28
	addiu	$13,$sp,56
	addiu	$5,$sp,84
$L104:
	lw	$2,0($18)
	sw	$2,20($sp)
	lw	$3,0($12)
	sw	$3,24($sp)
	lw	$3,0($11)
	sw	$3,28($sp)
	lw	$3,0($10)
	sw	$3,32($sp)
	lw	$3,0($9)
	sw	$3,36($sp)
	lw	$3,0($8)
	sw	$3,40($sp)
	lw	$3,0($7)
	sw	$3,44($sp)
	lw	$3,0($6)
	sw	$3,48($sp)
	lw	$3,0($18)
	sw	$3,52($sp)
	lw	$4,0($12)
	sw	$4,56($sp)
	lw	$4,0($11)
	sw	$4,60($sp)
	lw	$4,0($10)
	sw	$4,64($sp)
	lw	$4,0($9)
	sw	$4,68($sp)
	lw	$4,0($8)
	sw	$4,72($sp)
	lw	$4,0($7)
	sw	$4,76($sp)
	lw	$4,0($6)
	bne	$3,$2,$L101
	sw	$4,80($sp)

	move	$2,$13
$L102:
	lw	$4,0($2)
	lw	$3,-32($2)
	bne	$4,$3,$L101
	addiu	$2,$2,4

	bne	$2,$5,$L102
	sll	$3,$17,2

	j	$L113
	lui	$2,%hi(_rtl8651_asicTableSize)

$L101:
	addiu	$14,$14,-1
	bne	$14,$0,$L104
	sll	$3,$17,2

	lui	$2,%hi(_rtl8651_asicTableSize)
$L113:
	addiu	$2,$2,%lo(_rtl8651_asicTableSize)
	addu	$2,$3,$2
	lw	$2,0($2)
	beq	$2,$0,$L105
	move	$2,$0

	addiu	$3,$sp,52
	sll	$17,$17,2
	lui	$4,%hi(_rtl8651_asicTableSize)
	addiu	$4,$4,%lo(_rtl8651_asicTableSize)
	addu	$17,$17,$4
	lw	$5,0($17)
$L106:
	lw	$4,0($3)
	sw	$4,0($16)
	addiu	$2,$2,1
	addiu	$3,$3,4
	sltu	$4,$2,$5
	bne	$4,$0,$L106
	addiu	$16,$16,4

$L105:
	move	$2,$0
$L97:
	lw	$31,100($sp)
	lw	$18,96($sp)
	lw	$17,92($sp)
	lw	$16,88($sp)
	j	$31
	addiu	$sp,$sp,104

	.set	macro
	.set	reorder
	.end	_rtl8651_readAsicEntry
	.size	_rtl8651_readAsicEntry, .-_rtl8651_readAsicEntry
	.section	.text._rtl8651_asicTableAccessForward,"ax",@progbits
	.align	2
	.set	nomips16
	.ent	_rtl8651_asicTableAccessForward
	.type	_rtl8651_asicTableAccessForward, @function
_rtl8651_asicTableAccessForward:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	bne	$6,$0,$L115
	li	$3,-1149239296			# 0xbb800000

	lui	$4,%hi($LC1)
	addiu	$4,$4,%lo($LC1)
	lui	$5,%hi($LC2)
	addiu	$5,$5,%lo($LC2)
	jal	panic_printk
	li	$6,324			# 0x144

$L116:
	j	$L116
	nop

$L115:
	ori	$3,$3,0x4d00
$L120:
	lw	$2,0($3)
	andi	$2,$2,0x1
	bne	$2,$0,$L120
	lui	$2,%hi(_rtl8651_asicTableSize)

	sll	$3,$4,2
	addiu	$2,$2,%lo(_rtl8651_asicTableSize)
	addu	$2,$3,$2
	lw	$2,0($2)
	beq	$2,$0,$L117
	move	$2,$0

	li	$9,-1149239296			# 0xbb800000
	ori	$9,$9,0x4d20
	move	$7,$3
	lui	$3,%hi(_rtl8651_asicTableSize)
	addiu	$3,$3,%lo(_rtl8651_asicTableSize)
	addu	$3,$7,$3
	lw	$8,0($3)
$L118:
	sll	$3,$2,2
	addu	$3,$3,$9
	lw	$7,0($6)
	sw	$7,0($3)
	addiu	$2,$2,1
	sltu	$3,$2,$8
	bne	$3,$0,$L118
	addiu	$6,$6,4

$L117:
	sll	$4,$4,16
	li	$2,-1157627904			# 0xbb000000
	addu	$4,$4,$2
	sll	$5,$5,5
	addu	$5,$4,$5
	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x4d08
	sw	$5,0($2)
	lw	$31,20($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	_rtl8651_asicTableAccessForward
	.size	_rtl8651_asicTableAccessForward, .-_rtl8651_asicTableAccessForward
	.section	.text._rtl8651_forceAddAsicEntry,"ax",@progbits
	.align	2
	.globl	_rtl8651_forceAddAsicEntry
	.set	nomips16
	.ent	_rtl8651_forceAddAsicEntry
	.type	_rtl8651_forceAddAsicEntry, @function
_rtl8651_forceAddAsicEntry:
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
	move	$18,$4
	move	$16,$5
	move	$17,$6
	jal	rtl865x_accessAsicTable
	addiu	$5,$sp,16

	lw	$2,16($sp)
	beq	$2,$0,$L125
	li	$2,-1			# 0xffffffff

	li	$2,3			# 0x3
	bne	$18,$2,$L140
	lui	$2,%hi(RtkHomeGatewayChipNameID)

	lui	$2,%hi(mcastForceAddOpCnt)
	lw	$3,%lo(mcastForceAddOpCnt)($2)
	addiu	$3,$3,1
	sw	$3,%lo(mcastForceAddOpCnt)($2)
	lui	$2,%hi(RtkHomeGatewayChipNameID)
$L140:
	lw	$2,%lo(RtkHomeGatewayChipNameID)($2)
	li	$3,2			# 0x2
	bne	$2,$3,$L127
	li	$3,3			# 0x3

	lui	$2,%hi(RtkHomeGatewayChipRevisionID)
	lw	$2,%lo(RtkHomeGatewayChipRevisionID)($2)
	bgtz	$2,$L141
	li	$2,-1149239296			# 0xbb800000

	j	$L138
	move	$4,$18

$L127:
	beq	$2,$3,$L128
	li	$3,4			# 0x4

	bne	$2,$3,$L138
	move	$4,$18

$L128:
	li	$2,-1149239296			# 0xbb800000
$L141:
	ori	$2,$2,0x4418
	lw	$4,0($2)
	li	$3,262144			# 0x40000
	or	$3,$4,$3
	sw	$3,0($2)
	move	$4,$2
	li	$3,524288			# 0x80000
$L130:
	lw	$2,0($4)
	and	$2,$2,$3
	beq	$2,$0,$L130
	nop

	move	$4,$18
$L138:
	move	$5,$16
	jal	_rtl8651_asicTableAccessForward
	move	$6,$17

	li	$3,9			# 0x9
	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x4d00
	sw	$3,0($2)
	move	$3,$2
$L131:
	lw	$2,0($3)
	andi	$2,$2,0x1
	bne	$2,$0,$L131
	lui	$2,%hi(RtkHomeGatewayChipNameID)

	lw	$2,%lo(RtkHomeGatewayChipNameID)($2)
	li	$3,2			# 0x2
	bne	$2,$3,$L132
	nop

	lui	$2,%hi(RtkHomeGatewayChipRevisionID)
	lw	$2,%lo(RtkHomeGatewayChipRevisionID)($2)
	bgtz	$2,$L133
	nop

	j	$L139
	move	$2,$0

$L132:
	li	$3,3			# 0x3
	beq	$2,$3,$L133
	li	$3,4			# 0x4

	bne	$2,$3,$L134
	nop

$L133:
	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x4418
	lw	$4,0($2)
	li	$3,-327680			# 0xfffb0000
	ori	$3,$3,0xffff
	and	$3,$4,$3
	sw	$3,0($2)
	j	$L125
	move	$2,$0

$L134:
	move	$2,$0
$L125:
$L139:
	lw	$31,36($sp)
	lw	$18,32($sp)
	lw	$17,28($sp)
	lw	$16,24($sp)
	j	$31
	addiu	$sp,$sp,40

	.set	macro
	.set	reorder
	.end	_rtl8651_forceAddAsicEntry
	.size	_rtl8651_forceAddAsicEntry, .-_rtl8651_forceAddAsicEntry
	.section	.text._rtl8651_addAsicEntry,"ax",@progbits
	.align	2
	.globl	_rtl8651_addAsicEntry
	.set	nomips16
	.ent	_rtl8651_addAsicEntry
	.type	_rtl8651_addAsicEntry, @function
_rtl8651_addAsicEntry:
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
	move	$18,$4
	move	$16,$5
	move	$17,$6
	jal	rtl865x_accessAsicTable
	addiu	$5,$sp,16

	lw	$2,16($sp)
	beq	$2,$0,$L161
	li	$2,-1			# 0xffffffff

	move	$4,$18
	move	$5,$16
	jal	_rtl8651_asicTableAccessForward
	move	$6,$17

	lui	$2,%hi(RtkHomeGatewayChipNameID)
	lw	$4,%lo(RtkHomeGatewayChipNameID)($2)
	li	$2,2			# 0x2
	bne	$4,$2,$L144
	li	$2,3			# 0x3

	lui	$2,%hi(RtkHomeGatewayChipRevisionID)
	lw	$2,%lo(RtkHomeGatewayChipRevisionID)($2)
	bgtz	$2,$L162
	li	$2,-1149239296			# 0xbb800000

	j	$L164
	li	$3,3			# 0x3

$L144:
	beq	$4,$2,$L145
	li	$2,4			# 0x4

	bne	$4,$2,$L159
	li	$3,3			# 0x3

$L145:
	li	$2,-1149239296			# 0xbb800000
$L162:
	ori	$2,$2,0x4418
	lw	$5,0($2)
	li	$3,262144			# 0x40000
	or	$3,$5,$3
	sw	$3,0($2)
	move	$5,$2
	li	$3,524288			# 0x80000
$L147:
	lw	$2,0($5)
	and	$2,$2,$3
	beq	$2,$0,$L147
	nop

	li	$3,3			# 0x3
$L159:
	li	$2,-1149239296			# 0xbb800000
$L164:
	ori	$2,$2,0x4d00
	sw	$3,0($2)
	move	$3,$2
$L148:
	lw	$2,0($3)
	andi	$2,$2,0x1
	bne	$2,$0,$L148
	li	$2,-1149239296			# 0xbb800000

	ori	$2,$2,0x4d04
	lw	$2,0($2)
	andi	$2,$2,0x1
	beq	$2,$0,$L149
	li	$2,2			# 0x2

	bne	$4,$2,$L150
	li	$2,3			# 0x3

	lui	$2,%hi(RtkHomeGatewayChipRevisionID)
	lw	$2,%lo(RtkHomeGatewayChipRevisionID)($2)
	blez	$2,$L143
	nop

	j	$L160
	li	$2,-1149239296			# 0xbb800000

$L150:
	beq	$4,$2,$L160
	li	$2,-1149239296			# 0xbb800000

	li	$2,4			# 0x4
	bne	$4,$2,$L143
	nop

	li	$2,-1149239296			# 0xbb800000
$L160:
	ori	$2,$2,0x4418
	lw	$4,0($2)
	li	$3,-327680			# 0xfffb0000
	ori	$3,$3,0xffff
	and	$3,$4,$3
	sw	$3,0($2)
	j	$L152
	li	$2,-1			# 0xffffffff

$L149:
	bne	$4,$2,$L153
	li	$2,3			# 0x3

	lui	$2,%hi(RtkHomeGatewayChipRevisionID)
	lw	$2,%lo(RtkHomeGatewayChipRevisionID)($2)
	bgtz	$2,$L154
	nop

	j	$L161
	move	$2,$0

$L153:
	beq	$4,$2,$L163
	li	$2,-1149239296			# 0xbb800000

	li	$2,4			# 0x4
	bne	$4,$2,$L155
	nop

$L154:
	li	$2,-1149239296			# 0xbb800000
$L163:
	ori	$2,$2,0x4418
	lw	$4,0($2)
	li	$3,-327680			# 0xfffb0000
	ori	$3,$3,0xffff
	and	$3,$4,$3
	sw	$3,0($2)
	j	$L152
	move	$2,$0

$L143:
	j	$L152
	li	$2,-1			# 0xffffffff

$L155:
	move	$2,$0
$L152:
$L161:
	lw	$31,36($sp)
	lw	$18,32($sp)
	lw	$17,28($sp)
	lw	$16,24($sp)
	j	$31
	addiu	$sp,$sp,40

	.set	macro
	.set	reorder
	.end	_rtl8651_addAsicEntry
	.size	_rtl8651_addAsicEntry, .-_rtl8651_addAsicEntry
	.section	.rodata.str1.4
	.align	2
$LC3:
	.ascii	".........................................\012\000"
	.section	.text.rtl819x_poll_sw,"ax",@progbits
	.align	2
	.globl	rtl819x_poll_sw
	.set	nomips16
	.ent	rtl819x_poll_sw
	.type	rtl819x_poll_sw, @function
rtl819x_poll_sw:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	li	$2,-1149239296			# 0xbb800000
	ori	$2,$2,0x610c
	lw	$4,0($2)
	li	$2,-1207894016			# 0xb8010000
	ori	$3,$2,0x4
	lw	$6,0($3)
	ori	$2,$2,0x20
	lw	$5,0($2)
	lui	$3,%hi(rtl819x_checkSwCoreTimer)
	lw	$2,%lo(rtl819x_checkSwCoreTimer)($3)
	addiu	$7,$2,1
	sw	$7,%lo(rtl819x_checkSwCoreTimer)($3)
	li	$3,-859045888			# 0xcccc0000
	ori	$3,$3,0xcccd
	multu	$2,$3
	mfhi	$3
	srl	$3,$3,4
	sll	$7,$3,2
	sll	$3,$3,4
	addu	$3,$7,$3
	bne	$2,$3,$L173
	li	$2,-4			# 0xfffffffc

	and	$6,$6,$2
	and	$5,$5,$2
	li	$2,65536			# 0x10000
	and	$4,$4,$2
	bne	$4,$0,$L167
	lui	$2,%hi(rtl819x_lastTxDesc)

	lui	$2,%hi(rtl819x_swHangCnt)
	j	$L168
	sw	$0,%lo(rtl819x_swHangCnt)($2)

$L167:
	lw	$2,%lo(rtl819x_lastTxDesc)($2)
	beq	$2,$0,$L169
	nop

	lui	$3,%hi(rtl819x_lastRxDesc)
	lw	$3,%lo(rtl819x_lastRxDesc)($3)
	bne	$3,$0,$L170
	nop

$L169:
	lui	$2,%hi(rtl819x_swHangCnt)
	j	$L168
	sw	$0,%lo(rtl819x_swHangCnt)($2)

$L170:
	bne	$6,$3,$L171
	nop

	bne	$5,$2,$L175
	lui	$2,%hi(rtl819x_swHangCnt)

	lui	$3,%hi(rtl819x_swHangCnt)
	lw	$2,%lo(rtl819x_swHangCnt)($3)
	addiu	$2,$2,1
	sw	$2,%lo(rtl819x_swHangCnt)($3)
	lui	$3,%hi(rtl819x_lastRxDesc)
	sw	$6,%lo(rtl819x_lastRxDesc)($3)
	lui	$3,%hi(rtl819x_lastTxDesc)
	sltu	$2,$2,3
	bne	$2,$0,$L173
	sw	$5,%lo(rtl819x_lastTxDesc)($3)

	j	$L174
	lui	$2,%hi(rtl819x_swHangCnt)

$L171:
	lui	$2,%hi(rtl819x_swHangCnt)
$L175:
	j	$L168
	sw	$0,%lo(rtl819x_swHangCnt)($2)

$L174:
	sw	$0,%lo(rtl819x_swHangCnt)($2)
	lui	$4,%hi($LC3)
	jal	panic_printk
	addiu	$4,$4,%lo($LC3)

	jal	machine_restart
	move	$4,$0

	j	$L173
	nop

$L168:
	lui	$2,%hi(rtl819x_lastRxDesc)
	sw	$6,%lo(rtl819x_lastRxDesc)($2)
	lui	$2,%hi(rtl819x_lastTxDesc)
	sw	$5,%lo(rtl819x_lastTxDesc)($2)
$L173:
	lw	$31,20($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	rtl819x_poll_sw
	.size	rtl819x_poll_sw, .-rtl819x_poll_sw
	.local	rtl819x_checkSwCoreTimer
	.comm	rtl819x_checkSwCoreTimer,4,4
	.local	rtl819x_swHangCnt
	.comm	rtl819x_swHangCnt,4,4
	.local	rtl819x_lastTxDesc
	.comm	rtl819x_lastTxDesc,4,4
	.local	rtl819x_lastRxDesc
	.comm	rtl819x_lastRxDesc,4,4
	.rdata
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
	.local	mcastForceAddOpCnt
	.comm	mcastForceAddOpCnt,4,4
	.section	.dram-fwd,"aw",@progbits
	.align	2
	.type	fun_enable, @object
	.size	fun_enable, 4
fun_enable:
	.space	4
	.globl	RtkHomeGatewayChipName
	.section	.bss,"aw",@nobits
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
	.ident	"GCC: (GNU) 4.4.5-1.5.5p4"
