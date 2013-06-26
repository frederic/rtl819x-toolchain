	.file	1 "fast_pppoe_core.c"
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
	.globl	fast_pppoe_proc
	.data
	.align	2
	.type	fast_pppoe_proc, @object
	.size	fast_pppoe_proc, 4
fast_pppoe_proc:
	.word	0
	.globl	fast_pppoe_fw
	.align	2
	.type	fast_pppoe_fw, @object
	.size	fast_pppoe_fw, 4
fast_pppoe_fw:
	.word	0
	.rdata
	.align	2
$LC0:
	.ascii	"fast pppoe enable:%c\n\000"
	.align	2
$LC1:
	.ascii	"[%d] null,null,0\n\000"
	.align	2
$LC2:
	.ascii	"[%d] %s,%s,%d,0x%x:%x:%x:%x:%x:%x\n\000"
	.section	.text.fast_pppoe_read_proc,"ax",@progbits
	.align	2
	.ent	fast_pppoe_read_proc
	.type	fast_pppoe_read_proc, @function
fast_pppoe_read_proc:
	.set	nomips16
	.frame	$sp,80,$31		# vars= 0, regs= 8/0, args= 48, gp= 0
	.mask	0x807f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-80
	sw	$22,72($sp)
	sw	$21,68($sp)
	sw	$20,64($sp)
	sw	$19,60($sp)
	sw	$18,56($sp)
	sw	$17,52($sp)
	sw	$16,48($sp)
	sw	$31,76($sp)
	lui	$2,%hi(fast_pppoe_fw)
	lw	$2,%lo(fast_pppoe_fw)($2)
	move	$22,$5
	lui	$5,%hi($LC0)
	move	$20,$6
	addiu	$5,$5,%lo($LC0)
	addiu	$6,$2,48
	move	$19,$7
	jal	sprintf
	move	$21,$4

	move	$16,$2
	move	$17,$0
	move	$18,$0
$L7:
	lui	$2,%hi(fast_pppoe_info)
	addiu	$2,$2,%lo(fast_pppoe_info)
	addu	$13,$18,$2
	lhu	$2,54($13)
	lui	$7,%hi(fast_pppoe_info+16)
	lui	$5,%hi($LC1)
	addiu	$7,$7,%lo(fast_pppoe_info+16)
	addu	$14,$21,$16
	addiu	$5,$5,%lo($LC1)
	addu	$7,$18,$7
	move	$6,$17
	bne	$2,$0,$L5
	move	$4,$14

	jal	sprintf
	addiu	$17,$17,1

	j	$L15
	addu	$16,$16,$2

$L5:
	lbu	$12,43($13)
	lhu	$2,52($13)
	lbu	$3,38($13)
	lbu	$8,39($13)
	lbu	$9,40($13)
	lbu	$10,41($13)
	lbu	$11,42($13)
	lui	$5,%hi($LC2)
	addiu	$5,$5,%lo($LC2)
	move	$6,$17
	sw	$13,16($sp)
	sw	$2,20($sp)
	sw	$3,24($sp)
	sw	$8,28($sp)
	sw	$9,32($sp)
	sw	$10,36($sp)
	sw	$11,40($sp)
	jal	sprintf
	sw	$12,44($sp)

	addiu	$17,$17,1
	addu	$16,$16,$2
$L15:
	slt	$2,$17,16
	bne	$2,$0,$L7
	addiu	$18,$18,88

	addu	$2,$20,$19
	slt	$2,$2,$16
	bne	$2,$0,$L8
	li	$3,1			# 0x1

	lw	$2,96($sp)
	nop
	sw	$3,0($2)
$L8:
	subu	$16,$16,$20
	slt	$2,$19,$16
	movn	$16,$19,$2		#RLX4181/RLX4281:conditional move
	addu	$2,$21,$20
	slt	$3,$16,0
	sw	$2,0($22)
	move	$2,$0
	movz	$2,$16,$3		#RLX4181/RLX4281:conditional move  
	lw	$31,76($sp)
	lw	$22,72($sp)
	lw	$21,68($sp)
	lw	$20,64($sp)
	lw	$19,60($sp)
	lw	$18,56($sp)
	lw	$17,52($sp)
	lw	$16,48($sp)
	j	$31
	addiu	$sp,$sp,80

	.set	macro
	.set	reorder
	.end	fast_pppoe_read_proc
	.section	.text.fast_pppoe_write_proc,"ax",@progbits
	.align	2
	.ent	fast_pppoe_write_proc
	.type	fast_pppoe_write_proc, @function
fast_pppoe_write_proc:
	.set	nomips16
	.frame	$sp,16,$31		# vars= 8, regs= 1/0, args= 0, gp= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	addiu	$sp,$sp,-16
	sltu	$2,$6,2
	sw	$31,8($sp)
	move	$13,$6
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L16
	li	$4,-14			# 0xfffffffffffffff2
	.set	macro
	.set	reorder

	addiu	$2,$5,1
	.set	noreorder
	.set	nomacro
	beq	$5,$0,$L16
	or	$7,$5,$2
	.set	macro
	.set	reorder

	lw	$2,24($28)
	move	$4,$sp
	and	$2,$2,$7
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L18
	li	$6,1			# 0x1
	.set	macro
	.set	reorder

#APP
	.set	noreorder
	jal	__copy_user
	.set	noat
	addu	$1, $5, $6
	.set	at
	.set	reorder
#NO_APP
	.set	noreorder
	.set	nomacro
	bne	$6,$0,$L18
	move	$4,$13
	.set	macro
	.set	reorder

	lb	$2,0($sp)
	lui	$3,%hi(fast_pppoe_fw)
	addiu	$2,$2,-48
	.set	noreorder
	.set	nomacro
	j	$L16
	sw	$2,%lo(fast_pppoe_fw)($3)
	.set	macro
	.set	reorder

$L18:
	li	$4,-14			# 0xfffffffffffffff2
$L16:
	lw	$31,8($sp)
	move	$2,$4
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,16
	.set	macro
	.set	reorder

	.end	fast_pppoe_write_proc
	.rdata
	.align	2
$LC3:
	.ascii	"fast_pppoe\000"
	.section	.init.text,"ax",@progbits
	.align	2
	.globl	fast_pppoe_init
	.ent	fast_pppoe_init
	.type	fast_pppoe_init, @function
fast_pppoe_init:
	.set	nomips16
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	lui	$4,%hi($LC3)
	addiu	$sp,$sp,-24
	addiu	$4,$4,%lo($LC3)
	move	$5,$0
	sw	$31,16($sp)
	jal	create_proc_entry
	move	$6,$0

	lui	$4,%hi(fast_pppoe_info)
	lui	$7,%hi(fast_pppoe_proc)
	move	$3,$2
	addiu	$4,$4,%lo(fast_pppoe_info)
	move	$5,$0
	li	$6,1408			# 0x580
	beq	$2,$0,$L27
	sw	$2,%lo(fast_pppoe_proc)($7)

	lui	$2,%hi(fast_pppoe_read_proc)
	addiu	$2,$2,%lo(fast_pppoe_read_proc)
	sw	$2,64($3)
	lw	$3,%lo(fast_pppoe_proc)($7)
	lui	$2,%hi(fast_pppoe_write_proc)
	addiu	$2,$2,%lo(fast_pppoe_write_proc)
	sw	$2,68($3)
$L27:
	jal	memset
	nop

	lw	$31,16($sp)
	lui	$2,%hi(fast_pppoe_fw)
	li	$3,1			# 0x1
	sw	$3,%lo(fast_pppoe_fw)($2)
	addiu	$sp,$sp,24
	j	$31
	move	$2,$0

	.set	macro
	.set	reorder
	.end	fast_pppoe_init
	.section	.exit.text,"ax",@progbits
	.align	2
	.globl	fast_pppoe_exit
	.ent	fast_pppoe_exit
	.type	fast_pppoe_exit, @function
fast_pppoe_exit:
	.set	nomips16
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$16,16($sp)
	lui	$16,%hi(fast_pppoe_proc)
	lw	$2,%lo(fast_pppoe_proc)($16)
	lui	$4,%hi($LC3)
	sw	$31,20($sp)
	addiu	$4,$4,%lo($LC3)
	beq	$2,$0,$L29
	move	$5,$2

	jal	remove_proc_entry
	nop

	sw	$0,%lo(fast_pppoe_proc)($16)
$L29:
	lw	$31,20($sp)
	lw	$16,16($sp)
	move	$2,$0
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	fast_pppoe_exit
	.section	.text.clear_pppoe_info,"ax",@progbits
	.align	2
	.globl	clear_pppoe_info
	.ent	clear_pppoe_info
	.type	clear_pppoe_info, @function
clear_pppoe_info:
	.set	nomips16
	.frame	$sp,56,$31		# vars= 0, regs= 9/0, args= 16, gp= 0
	.mask	0x80ff0000,-8
	.fmask	0x00000000,0
	addiu	$sp,$sp,-56
	sw	$23,44($sp)
	sw	$22,40($sp)
	lw	$23,76($sp)
	lw	$22,80($sp)
	sw	$21,36($sp)
	sw	$20,32($sp)
	sw	$18,24($sp)
	sw	$17,20($sp)
	sw	$16,16($sp)
	sw	$31,48($sp)
	sw	$19,28($sp)
	move	$17,$4
	move	$16,$5
	andi	$21,$6,0xffff
	move	$18,$0
	li	$20,15			# 0xf
$L42:
	lui	$19,%hi(fast_pppoe_info)
	addiu	$2,$19,%lo(fast_pppoe_info)
	addu	$5,$18,$2
	lhu	$2,54($5)
	#nop
	beq	$2,$0,$L33
	.set	noreorder
	.set	nomacro
	beq	$17,$0,$L35
	lui	$2,%hi(fast_pppoe_info+16)
	.set	macro
	.set	reorder

	addiu	$2,$2,%lo(fast_pppoe_info+16)
	addu	$2,$18,$2
	move	$4,$17
#APP
	.set	noreorder
	.set	noat
	lbu	$3,($2)
1:	lbu	$1,($4)
	addiu	$2,1
	bne	$1,$3,2f
	addiu	$4,1
	bnez	$3,1b
	lbu	$3,($2)
	nop
	move	$3,$1
2:	subu	$3,$1
3:	.set	at
	.set	reorder
#NO_APP
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L35
	move	$4,$5
	.set	macro
	.set	reorder

	li	$6,88			# 0x58
	.set	noreorder
	.set	nomacro
	jal	memset
	move	$5,$0
	.set	macro
	.set	reorder

$L35:
	.set	noreorder
	.set	nomacro
	beq	$16,$0,$L37
	addiu	$2,$19,%lo(fast_pppoe_info)
	.set	macro
	.set	reorder

	addu	$2,$18,$2
	move	$5,$2
	move	$4,$16
#APP
	.set	noreorder
	.set	noat
	lbu	$3,($2)
1:	lbu	$1,($4)
	addiu	$2,1
	bne	$1,$3,2f
	addiu	$4,1
	bnez	$3,1b
	lbu	$3,($2)
	nop
	move	$3,$1
2:	subu	$3,$1
3:	.set	at
	.set	reorder
#NO_APP
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L37
	move	$4,$5
	.set	macro
	.set	reorder

	li	$6,88			# 0x58
	.set	noreorder
	.set	nomacro
	jal	memset
	move	$5,$0
	.set	macro
	.set	reorder

$L37:
	.set	noreorder
	.set	nomacro
	beq	$22,$0,$L39
	lui	$4,%hi(fast_pppoe_info+38)
	.set	macro
	.set	reorder

	addiu	$4,$4,%lo(fast_pppoe_info+38)
	addu	$4,$18,$4
	move	$5,$22
	.set	noreorder
	.set	nomacro
	jal	memcmp
	li	$6,6			# 0x6
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L39
	lui	$4,%hi(fast_pppoe_info)
	.set	macro
	.set	reorder

	addiu	$4,$4,%lo(fast_pppoe_info)
	addu	$4,$18,$4
	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	memset
	li	$6,88			# 0x58
	.set	macro
	.set	reorder

$L39:
	.set	noreorder
	.set	nomacro
	beq	$23,$0,$L40
	lui	$4,%hi(fast_pppoe_info+32)
	.set	macro
	.set	reorder

	addiu	$4,$4,%lo(fast_pppoe_info+32)
	addu	$4,$18,$4
	move	$5,$23
	.set	noreorder
	.set	nomacro
	jal	memcmp
	li	$6,6			# 0x6
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L45
	lui	$2,%hi(fast_pppoe_info)
	.set	macro
	.set	reorder

	lui	$4,%hi(fast_pppoe_info)
	addiu	$4,$4,%lo(fast_pppoe_info)
	addu	$4,$18,$4
	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	memset
	li	$6,88			# 0x58
	.set	macro
	.set	reorder

$L40:
	lui	$2,%hi(fast_pppoe_info)
$L45:
	addiu	$2,$2,%lo(fast_pppoe_info)
	addu	$4,$18,$2
	lhu	$2,52($4)
	#nop
	.set	noreorder
	.set	nomacro
	bne	$2,$21,$L33
	move	$5,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	memset
	li	$6,88			# 0x58
	.set	macro
	.set	reorder

$L33:
	addiu	$20,$20,-1
	.set	noreorder
	.set	nomacro
	bgez	$20,$L42
	addiu	$18,$18,88
	.set	macro
	.set	reorder

	lw	$31,48($sp)
	lw	$23,44($sp)
	lw	$22,40($sp)
	lw	$21,36($sp)
	lw	$20,32($sp)
	lw	$19,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	move	$2,$0
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,56
	.set	macro
	.set	reorder

	.end	clear_pppoe_info
	.section	.text.find_pppoe_info,"ax",@progbits
	.align	2
	.ent	find_pppoe_info
	.type	find_pppoe_info, @function
find_pppoe_info:
	.set	nomips16
	.frame	$sp,56,$31		# vars= 0, regs= 10/0, args= 16, gp= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-56
	sw	$fp,48($sp)
	sw	$23,44($sp)
	sw	$22,40($sp)
	lw	$fp,72($sp)
	lw	$23,76($sp)
	lw	$22,80($sp)
	sw	$21,36($sp)
	sw	$20,32($sp)
	sw	$19,28($sp)
	sw	$18,24($sp)
	sw	$17,20($sp)
	sw	$16,16($sp)
	sw	$31,52($sp)
	move	$17,$4
	move	$16,$5
	move	$21,$7
	andi	$20,$6,0xffff
	move	$19,$0
	move	$18,$0
$L60:
	lui	$6,%hi(fast_pppoe_info)
	addiu	$2,$6,%lo(fast_pppoe_info)
	addu	$5,$18,$2
	lhu	$2,54($5)
	#nop
	beq	$2,$0,$L49
	.set	noreorder
	.set	nomacro
	beq	$17,$0,$L51
	lui	$2,%hi(fast_pppoe_info+16)
	.set	macro
	.set	reorder

	addiu	$2,$2,%lo(fast_pppoe_info+16)
	addu	$2,$18,$2
	move	$4,$17
#APP
	.set	noreorder
	.set	noat
	lbu	$3,($2)
1:	lbu	$1,($4)
	addiu	$2,1
	bne	$1,$3,2f
	addiu	$4,1
	bnez	$3,1b
	lbu	$3,($2)
	nop
	move	$3,$1
2:	subu	$3,$1
3:	.set	at
	.set	reorder
#NO_APP
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L46
	move	$2,$5
	.set	macro
	.set	reorder

$L51:
	.set	noreorder
	.set	nomacro
	beq	$16,$0,$L53
	addiu	$2,$6,%lo(fast_pppoe_info)
	.set	macro
	.set	reorder

	addu	$2,$18,$2
	move	$5,$2
	move	$4,$16
#APP
	.set	noreorder
	.set	noat
	lbu	$3,($2)
1:	lbu	$1,($4)
	addiu	$2,1
	bne	$1,$3,2f
	addiu	$4,1
	bnez	$3,1b
	lbu	$3,($2)
	nop
	move	$3,$1
2:	subu	$3,$1
3:	.set	at
	.set	reorder
#NO_APP
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L46
	move	$2,$5
	.set	macro
	.set	reorder

$L53:
	.set	noreorder
	.set	nomacro
	beq	$22,$0,$L55
	lui	$4,%hi(fast_pppoe_info+38)
	.set	macro
	.set	reorder

	addiu	$4,$4,%lo(fast_pppoe_info+38)
	addu	$4,$18,$4
	move	$5,$22
	.set	noreorder
	.set	nomacro
	jal	memcmp
	li	$6,6			# 0x6
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L66
	lui	$2,%hi(fast_pppoe_info)
	.set	macro
	.set	reorder

$L55:
	.set	noreorder
	.set	nomacro
	beq	$23,$0,$L56
	lui	$4,%hi(fast_pppoe_info+32)
	.set	macro
	.set	reorder

	addiu	$4,$4,%lo(fast_pppoe_info+32)
	addu	$4,$18,$4
	move	$5,$23
	.set	noreorder
	.set	nomacro
	jal	memcmp
	li	$6,6			# 0x6
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L56
	lui	$2,%hi(fast_pppoe_info)
	.set	macro
	.set	reorder

$L66:
	addiu	$2,$2,%lo(fast_pppoe_info)
	.set	noreorder
	.set	nomacro
	j	$L46
	addu	$2,$18,$2
	.set	macro
	.set	reorder

$L56:
	.set	noreorder
	.set	nomacro
	beq	$fp,$0,$L57
	lui	$2,%hi(fast_pppoe_info)
	.set	macro
	.set	reorder

	addiu	$2,$2,%lo(fast_pppoe_info)
	addu	$4,$18,$2
	lw	$2,48($4)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$fp,$L46
	move	$2,$4
	.set	macro
	.set	reorder

$L57:
	.set	noreorder
	.set	nomacro
	beq	$21,$0,$L58
	lui	$2,%hi(fast_pppoe_info)
	.set	macro
	.set	reorder

	addiu	$2,$2,%lo(fast_pppoe_info)
	addu	$4,$18,$2
	lw	$2,44($4)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$21,$L46
	move	$2,$4
	.set	macro
	.set	reorder

$L58:
	.set	noreorder
	.set	nomacro
	beq	$20,$0,$L49
	lui	$2,%hi(fast_pppoe_info)
	.set	macro
	.set	reorder

	addiu	$2,$2,%lo(fast_pppoe_info)
	addu	$4,$18,$2
	lhu	$2,52($4)
	#nop
	.set	noreorder
	.set	nomacro
	bne	$2,$20,$L67
	addiu	$19,$19,1
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L46
	move	$2,$4
	.set	macro
	.set	reorder

$L49:
	addiu	$19,$19,1
$L67:
	slt	$2,$19,16
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L60
	addiu	$18,$18,88
	.set	macro
	.set	reorder

	move	$2,$0
$L46:
	lw	$31,52($sp)
	lw	$fp,48($sp)
	lw	$23,44($sp)
	lw	$22,40($sp)
	lw	$21,36($sp)
	lw	$20,32($sp)
	lw	$19,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,56
	.set	macro
	.set	reorder

	.end	find_pppoe_info
	.section	.text.set_pppoe_info,"ax",@progbits
	.align	2
	.globl	set_pppoe_info
	.ent	set_pppoe_info
	.type	set_pppoe_info, @function
set_pppoe_info:
	.set	nomips16
	.frame	$sp,72,$31		# vars= 0, regs= 9/0, args= 32, gp= 0
	.mask	0x80ff0000,-8
	.fmask	0x00000000,0
	addiu	$sp,$sp,-72
	sw	$22,56($sp)
	sw	$21,52($sp)
	sw	$19,44($sp)
	lw	$22,92($sp)
	lw	$19,88($sp)
	lw	$21,96($sp)
	sw	$20,48($sp)
	sw	$18,40($sp)
	andi	$20,$6,0xffff
	move	$18,$5
	move	$6,$0
	move	$5,$0
	sw	$23,60($sp)
	sw	$17,36($sp)
	sw	$16,32($sp)
	sw	$31,64($sp)
	sw	$19,16($sp)
	sw	$22,20($sp)
	sw	$21,24($sp)
	move	$17,$4
	.set	noreorder
	.set	nomacro
	jal	find_pppoe_info
	move	$23,$7
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L69
	move	$16,$2
	.set	macro
	.set	reorder

	lui	$16,%hi(fast_pppoe_info)
	move	$5,$0
	addiu	$4,$16,%lo(fast_pppoe_info)
$L75:
	lhu	$2,54($4)
	addiu	$5,$5,1
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L88
	slt	$3,$5,16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L75
	addiu	$4,$4,88
	.set	macro
	.set	reorder

	addiu	$4,$16,%lo(fast_pppoe_info)
	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	memset
	li	$6,1408			# 0x580
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L69
	addiu	$16,$16,%lo(fast_pppoe_info)
	.set	macro
	.set	reorder

$L88:
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L68
	move	$16,$4
	.set	macro
	.set	reorder

$L69:
	beq	$20,$0,$L77
	sh	$20,52($16)
$L77:
	beq	$23,$0,$L78
	sw	$23,44($16)
$L78:
	beq	$19,$0,$L79
	sw	$19,48($16)
$L79:
	.set	noreorder
	.set	nomacro
	beq	$17,$0,$L80
	addiu	$2,$16,16
	.set	macro
	.set	reorder

#APP
	.set	noreorder
	.set	noat
1:	lbu	$1,($17)
	addiu	$17,1
	sb	$1,($2)
	bnez	$1,1b
	addiu	$2,1
	.set	at
	.set	reorder
#NO_APP
$L80:
	.set	noreorder
	.set	nomacro
	beq	$18,$0,$L82
	move	$3,$16
	.set	macro
	.set	reorder

#APP
	.set	noreorder
	.set	noat
1:	lbu	$1,($18)
	addiu	$18,1
	sb	$1,($3)
	bnez	$1,1b
	addiu	$3,1
	.set	at
	.set	reorder
#NO_APP
$L82:
	.set	noreorder
	.set	nomacro
	beq	$21,$0,$L84
	move	$5,$21
	.set	macro
	.set	reorder

	addiu	$4,$16,38
	.set	noreorder
	.set	nomacro
	jal	memcpy
	li	$6,6			# 0x6
	.set	macro
	.set	reorder

$L84:
	.set	noreorder
	.set	nomacro
	beq	$22,$0,$L85
	move	$5,$22
	.set	macro
	.set	reorder

	addiu	$4,$16,32
	.set	noreorder
	.set	nomacro
	jal	memcpy
	li	$6,6			# 0x6
	.set	macro
	.set	reorder

$L85:
	li	$2,1
	sh	$2,54($16)
$L68:
	lw	$31,64($sp)
	lw	$23,60($sp)
	lw	$22,56($sp)
	lw	$21,52($sp)
	lw	$20,48($sp)
	lw	$19,44($sp)
	lw	$18,40($sp)
	lw	$17,36($sp)
	lw	$16,32($sp)
	move	$2,$0
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,72
	.set	macro
	.set	reorder

	.end	set_pppoe_info
	.section	.text.get_pppoe_last_rx_tx,"ax",@progbits
	.align	2
	.globl	get_pppoe_last_rx_tx
	.ent	get_pppoe_last_rx_tx
	.type	get_pppoe_last_rx_tx, @function
get_pppoe_last_rx_tx:
	.set	nomips16
	.frame	$sp,40,$31		# vars= 0, regs= 1/0, args= 32, gp= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-40
	lw	$2,56($sp)
	andi	$6,$6,0xffff
	sw	$31,32($sp)
	sw	$2,16($sp)
	lw	$2,60($sp)
	nop
	sw	$2,20($sp)
	lw	$2,64($sp)
	jal	find_pppoe_info
	sw	$2,24($sp)

	beq	$2,$0,$L90
	move	$4,$2

	lw	$3,56($2)
	lw	$2,68($sp)
	nop
	sw	$3,0($2)
	lw	$3,60($4)
	lw	$2,72($sp)
	nop
	sw	$3,0($2)
$L90:
	lw	$31,32($sp)
	move	$2,$0
	j	$31
	addiu	$sp,$sp,40

	.set	macro
	.set	reorder
	.end	get_pppoe_last_rx_tx
	.section	.text.check_and_pull_pppoe_hdr,"ax",@progbits
	.align	2
	.globl	check_and_pull_pppoe_hdr
	.ent	check_and_pull_pppoe_hdr
	.type	check_and_pull_pppoe_hdr, @function
check_and_pull_pppoe_hdr:
	.set	nomips16
	.frame	$sp,48,$31		# vars= 0, regs= 8/0, args= 16, gp= 0
	.mask	0x807f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-48
	sw	$19,28($sp)
	sw	$31,44($sp)
	sw	$22,40($sp)
	sw	$21,36($sp)
	sw	$20,32($sp)
	sw	$18,24($sp)
	sw	$17,20($sp)
	sw	$16,16($sp)
	lui	$2,%hi(fast_pppoe_fw)
	lw	$2,%lo(fast_pppoe_fw)($2)
	move	$19,$4
	beq	$2,$0,$L91
	move	$3,$0

	jal	rtl_set_skb_pppoe_flag
	move	$5,$0

	jal	rtl_skb_mac_header
	move	$4,$19

	move	$17,$2
	beq	$2,$0,$L91
	move	$3,$0

	lhu	$3,12($2)
	li	$2,34916			# 0x8864
	bne	$3,$2,$L91
	move	$3,$0

	lhu	$3,20($17)
	li	$2,33			# 0x21
	bne	$3,$2,$L91
	move	$3,$0

	lhu	$22,16($17)
	addiu	$21,$17,6
	move	$20,$0
	move	$18,$0
$L102:
	lui	$2,%hi(fast_pppoe_info)
	addiu	$2,$2,%lo(fast_pppoe_info)
	addu	$16,$18,$2
	lhu	$2,54($16)
	nop
	beq	$2,$0,$L98
	addiu	$20,$20,1

	lhu	$2,52($16)
	lui	$4,%hi(fast_pppoe_info+38)
	addiu	$4,$4,%lo(fast_pppoe_info+38)
	addu	$4,$18,$4
	move	$5,$21
	bne	$2,$22,$L98
	li	$6,6			# 0x6

	jal	memcmp
	nop

	beq	$2,$0,$L107
	nop

$L98:
	slt	$2,$20,16
	bne	$2,$0,$L102
	addiu	$18,$18,88

	j	$L91
	move	$3,$0

$L107:
	bne	$16,$0,$L103
	move	$18,$16

	j	$L91
	move	$3,$0

$L103:
	jal	rtl_get_dev_by_name
	addiu	$4,$16,16

	move	$20,$2
	beq	$2,$0,$L91
	move	$3,$0

	addiu	$16,$17,22
	move	$5,$16
	jal	rtl_set_skb_network_header
	move	$4,$19

	move	$5,$16
	jal	rtl_set_skb_transport_header
	move	$4,$19

	li	$5,8			# 0x8
	jal	rtl_skb_pull
	move	$4,$19

	jal	rtl_get_skb_dev
	move	$4,$19

	move	$5,$2
	jal	rtl_set_skb_rx_dev
	move	$4,$19

	move	$4,$19
	jal	rtl_set_skb_protocol
	li	$5,2048			# 0x800

	move	$5,$20
	jal	rtl_set_skb_dev
	move	$4,$19

	jal	rtl_get_skb_dev
	move	$4,$19

	lw	$3,80($2)
	move	$4,$19
	addiu	$3,$3,1
	jal	rtl_get_skb_dev
	sw	$3,80($2)

	move	$16,$2
	jal	rtl_get_skb_len
	move	$4,$19

	lw	$3,88($16)
	move	$4,$19
	li	$5,1			# 0x1
	addu	$3,$3,$2
	jal	rtl_set_skb_pppoe_flag
	sw	$3,88($16)

	lui	$2,%hi(jiffies)
	lw	$2,%lo(jiffies)($2)
	li	$3,1			# 0x1
	sw	$2,56($18)
$L91:
	lw	$31,44($sp)
	lw	$22,40($sp)
	lw	$21,36($sp)
	lw	$20,32($sp)
	lw	$19,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	move	$2,$3
	j	$31
	addiu	$sp,$sp,48

	.set	macro
	.set	reorder
	.end	check_and_pull_pppoe_hdr
	.section	.text.check_and_restore_pppoe_hdr,"ax",@progbits
	.align	2
	.globl	check_and_restore_pppoe_hdr
	.ent	check_and_restore_pppoe_hdr
	.type	check_and_restore_pppoe_hdr, @function
check_and_restore_pppoe_hdr:
	.set	nomips16
	.frame	$sp,32,$31		# vars= 0, regs= 3/0, args= 16, gp= 0
	.mask	0x80030000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-32
	sw	$17,20($sp)
	sw	$31,24($sp)
	sw	$16,16($sp)
	lui	$2,%hi(fast_pppoe_fw)
	lw	$2,%lo(fast_pppoe_fw)($2)
	nop
	beq	$2,$0,$L109
	move	$17,$4

	jal	rtl_get_skb_pppoe_flag
	nop

	li	$5,8			# 0x8
	beq	$2,$0,$L109
	move	$4,$17

	jal	rtl_skb_push
	nop

	jal	rtl_get_skb_data
	move	$4,$17

	move	$5,$2
	jal	rtl_set_skb_network_header
	move	$4,$17

	jal	rtl_get_skb_data
	move	$4,$17

	move	$5,$2
	jal	rtl_set_skb_transport_header
	move	$4,$17

	jal	rtl_get_skb_dev
	move	$4,$17

	lw	$3,80($2)
	move	$4,$17
	addiu	$3,$3,-1
	jal	rtl_get_skb_dev
	sw	$3,80($2)

	move	$16,$2
	jal	rtl_get_skb_len
	move	$4,$17

	lw	$3,88($16)
	move	$4,$17
	subu	$3,$3,$2
	jal	rtl_get_skb_rx_dev
	sw	$3,88($16)

	move	$5,$2
	jal	rtl_set_skb_dev
	move	$4,$17

	move	$4,$17
	jal	rtl_set_skb_protocol
	li	$5,34916			# 0x8864

	move	$4,$17
	lw	$31,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	move	$5,$0
	j	rtl_set_skb_pppoe_flag
	addiu	$sp,$sp,32

$L109:
	lw	$31,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	j	$31
	addiu	$sp,$sp,32

	.set	macro
	.set	reorder
	.end	check_and_restore_pppoe_hdr
	.rdata
	.align	2
$LC4:
	.ascii	"ppp\000"
	.section	.text.fast_pppoe_xmit,"ax",@progbits
	.align	2
	.globl	fast_pppoe_xmit
	.ent	fast_pppoe_xmit
	.type	fast_pppoe_xmit, @function
fast_pppoe_xmit:
	.set	nomips16
	.frame	$sp,56,$31		# vars= 0, regs= 5/0, args= 32, gp= 0
	.mask	0x800f0000,-8
	.fmask	0x00000000,0
	addiu	$sp,$sp,-56
	sw	$17,36($sp)
	sw	$31,48($sp)
	sw	$19,44($sp)
	sw	$18,40($sp)
	sw	$16,32($sp)
	lui	$2,%hi(fast_pppoe_fw)
	lw	$2,%lo(fast_pppoe_fw)($2)
	move	$17,$4
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L112
	move	$3,$0
	.set	macro
	.set	reorder

	lui	$2,%hi(gQosEnabled)
	lw	$2,%lo(gQosEnabled)($2)
	#nop
	bne	$2,$0,$L112
	jal	rtl_get_skb_dev
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L112
	move	$3,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_dev
	move	$4,$17
	.set	macro
	.set	reorder

	lui	$5,%hi($LC4)
	addiu	$4,$5,%lo($LC4)
	li	$3,3			# 0x3
#APP
	.set	noreorder
	.set	noat
1:	lbu	$5,($2)
	beqz	$3,2f
	lbu	$1,($4)
	subu	$3,1
	bne	$1,$5,3f
	addiu	$2,1
	bnez	$5,1b
	addiu	$4,1
2:
	nop
	move	$5,$1
3:	subu	$5,$1
	.set	at
	.set	reorder
#NO_APP
	.set	noreorder
	.set	nomacro
	bne	$5,$0,$L112
	move	$3,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_skb_headroom
	move	$4,$17
	.set	macro
	.set	reorder

	sltu	$2,$2,22
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L112
	move	$3,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_skb_cloned
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L112
	move	$3,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_skb_shared
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L112
	move	$3,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_dev
	move	$4,$17
	.set	macro
	.set	reorder

	move	$4,$2
	move	$5,$0
	move	$6,$0
	move	$7,$0
	sw	$0,16($sp)
	sw	$0,20($sp)
	.set	noreorder
	.set	nomacro
	jal	find_pppoe_info
	sw	$0,24($sp)
	.set	macro
	.set	reorder

	move	$16,$2
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L112
	move	$3,$0
	.set	macro
	.set	reorder

	lb	$2,16($2)
	#nop
	beq	$2,$0,$L112
	lb	$2,0($16)
	#nop
	beq	$2,$0,$L112
	lhu	$2,52($16)
	#nop
	beq	$2,$0,$L112
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_dev
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_len
	move	$4,$17
	.set	macro
	.set	reorder

	move	$4,$16
	.set	noreorder
	.set	nomacro
	jal	rtl_get_dev_by_name
	move	$19,$2
	.set	macro
	.set	reorder

	move	$18,$2
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L112
	move	$3,$0
	.set	macro
	.set	reorder

	li	$5,2			# 0x2
	.set	noreorder
	.set	nomacro
	jal	rtl_skb_push
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	move	$4,$17
	.set	macro
	.set	reorder

	li	$3,33
	sh	$3,0($2)
	li	$5,6			# 0x6
	.set	noreorder
	.set	nomacro
	jal	rtl_skb_push
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	move	$4,$17
	.set	macro
	.set	reorder

	li	$3,17			# 0x11
	sb	$3,0($2)
	sb	$0,1($2)
	lhu	$6,52($16)
	addiu	$3,$19,2
	andi	$3,$3,0xffff
	srl	$4,$6,8
	srl	$5,$3,8
	sb	$6,3($2)
	sb	$3,5($2)
	sb	$4,2($2)
	sb	$5,4($2)
	move	$4,$17
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_dev
	move	$5,$18
	.set	macro
	.set	reorder

	li	$5,2			# 0x2
	.set	noreorder
	.set	nomacro
	jal	rtl_skb_push
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	move	$4,$17
	.set	macro
	.set	reorder

	li	$3,-30620
	sh	$3,0($2)
	li	$5,12			# 0xc
	.set	noreorder
	.set	nomacro
	jal	rtl_skb_push
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	move	$4,$17
	.set	macro
	.set	reorder

	move	$4,$2
	addiu	$5,$16,38
	.set	noreorder
	.set	nomacro
	jal	memcpy
	li	$6,6			# 0x6
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	move	$4,$17
	.set	macro
	.set	reorder

	addiu	$4,$2,6
	addiu	$5,$18,320
	.set	noreorder
	.set	nomacro
	jal	memcpy
	li	$6,6			# 0x6
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	ip_finish_output4
	move	$4,$17
	.set	macro
	.set	reorder

	lui	$2,%hi(jiffies)
	lw	$2,%lo(jiffies)($2)
	li	$3,1			# 0x1
	sw	$2,60($16)
$L112:
	lw	$31,48($sp)
	lw	$19,44($sp)
	lw	$18,40($sp)
	lw	$17,36($sp)
	lw	$16,32($sp)
	move	$2,$3
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,56
	.set	macro
	.set	reorder

	.end	fast_pppoe_xmit
	.globl	fast_pppoe_info
	.section	.bss
	.align	2
	.type	fast_pppoe_info, @object
	.size	fast_pppoe_info, 1408
fast_pppoe_info:
	.space	1408
	.ident	"GCC: (GNU) 3.4.6-1.3.6"
