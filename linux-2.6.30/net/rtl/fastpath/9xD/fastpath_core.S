	.file	1 "fastpath_core.c"
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
	.section	.text.cache_timeout,"ax",@progbits
	.align	2
	.set	nomips16
	.ent	cache_timeout
	.type	cache_timeout, @function
cache_timeout:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	lbu	$2,0($4)
	beq	$2,$0,$L7
	lui	$6,%hi(max_udp_frag_entry)

	lw	$2,4($4)
	lw	$5,8($4)
	lhu	$3,2($4)
	xor	$5,$5,$2
	lbu	$2,1($4)
	srl	$7,$5,16
	lw	$6,%lo(max_udp_frag_entry)($6)
	xor	$3,$3,$5
	lw	$5,40($4)
	xor	$3,$3,$7
	addiu	$6,$6,-1
	xor	$2,$3,$2
	sb	$0,0($4)
	srl	$7,$3,8
	lw	$3,44($4)
	xor	$2,$2,$7
	beq	$5,$0,$L3
	and	$2,$2,$6

	j	$L4
	sw	$3,44($5)

$L3:
	li	$5,12			# 0xc
	lui	$6,%hi(udp_cache_table)
	mult	$2,$5
	lw	$6,%lo(udp_cache_table)($6)
	lw	$6,0($6)
	mflo	$5
	addu	$5,$6,$5
	sw	$3,4($5)
$L4:
	lw	$3,44($4)
	li	$6,12			# 0xc
	lw	$5,40($4)
	mult	$2,$6
	sw	$5,0($3)
	lui	$3,%hi(udp_cache_table)
	lw	$6,%lo(udp_cache_table)($3)
	lui	$3,%hi(udp_cache_list_free)
	mflo	$2
	addiu	$3,$3,%lo(udp_cache_list_free)
	lw	$6,0($6)
	addiu	$5,$4,48
	addu	$2,$6,$2
	lw	$6,8($2)
	addiu	$6,$6,-1
	sw	$6,8($2)
	sw	$0,48($4)
	lw	$2,4($3)
	sw	$2,52($4)
	lw	$2,4($3)
	sw	$4,0($2)
	lw	$2,8($3)
	sw	$5,4($3)
	addiu	$2,$2,1
	sw	$2,8($3)
$L7:
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	cache_timeout
	.size	cache_timeout, .-cache_timeout
	.section	.text.rtk_addArp,"ax",@progbits
	.align	2
	.globl	rtk_addArp
	.set	nomips16
	.ent	rtk_addArp
	.type	rtk_addArp, @function
rtk_addArp:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	j	$31
	move	$2,$0

	.set	macro
	.set	reorder
	.end	rtk_addArp
	.size	rtk_addArp, .-rtk_addArp
	.section	.text.rtk_modifyArp,"ax",@progbits
	.align	2
	.globl	rtk_modifyArp
	.set	nomips16
	.ent	rtk_modifyArp
	.type	rtk_modifyArp, @function
rtk_modifyArp:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	j	$31
	move	$2,$0

	.set	macro
	.set	reorder
	.end	rtk_modifyArp
	.size	rtk_modifyArp, .-rtk_modifyArp
	.section	.text.rtk_delArp,"ax",@progbits
	.align	2
	.globl	rtk_delArp
	.set	nomips16
	.ent	rtk_delArp
	.type	rtk_delArp, @function
rtk_delArp:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	j	$31
	li	$2,-4			# 0xfffffffc

	.set	macro
	.set	reorder
	.end	rtk_delArp
	.size	rtk_delArp, .-rtk_delArp
	.section	.text.rtk_addRoute,"ax",@progbits
	.align	2
	.globl	rtk_addRoute
	.set	nomips16
	.ent	rtk_addRoute
	.type	rtk_addRoute, @function
rtk_addRoute:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	j	$31
	move	$2,$0

	.set	macro
	.set	reorder
	.end	rtk_addRoute
	.size	rtk_addRoute, .-rtk_addRoute
	.section	.text.rtk_modifyRoute,"ax",@progbits
	.align	2
	.globl	rtk_modifyRoute
	.set	nomips16
	.ent	rtk_modifyRoute
	.type	rtk_modifyRoute, @function
rtk_modifyRoute:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	j	$31
	move	$2,$0

	.set	macro
	.set	reorder
	.end	rtk_modifyRoute
	.size	rtk_modifyRoute, .-rtk_modifyRoute
	.section	.text.rtk_delRoute,"ax",@progbits
	.align	2
	.globl	rtk_delRoute
	.set	nomips16
	.ent	rtk_delRoute
	.type	rtk_delRoute, @function
rtk_delRoute:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	j	$31
	li	$2,-4			# 0xfffffffc

	.set	macro
	.set	reorder
	.end	rtk_delRoute
	.size	rtk_delRoute, .-rtk_delRoute
	.section	.text.rtk_addSession,"ax",@progbits
	.align	2
	.globl	rtk_addSession
	.set	nomips16
	.ent	rtk_addSession
	.type	rtk_addSession, @function
rtk_addSession:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	j	$31
	move	$2,$0

	.set	macro
	.set	reorder
	.end	rtk_addSession
	.size	rtk_addSession, .-rtk_addSession
	.section	.text.rtk_delSession,"ax",@progbits
	.align	2
	.globl	rtk_delSession
	.set	nomips16
	.ent	rtk_delSession
	.type	rtk_delSession, @function
rtk_delSession:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	j	$31
	move	$2,$0

	.set	macro
	.set	reorder
	.end	rtk_delSession
	.size	rtk_delSession, .-rtk_delSession
	.section	.text.FastPath_Route,"ax",@progbits
	.align	2
	.globl	FastPath_Route
	.set	nomips16
	.ent	FastPath_Route
	.type	FastPath_Route, @function
FastPath_Route:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	lui	$2,%hi(route_list_inuse)
	move	$6,$0
	lw	$3,%lo(route_list_inuse)($2)
	j	$L25
	move	$2,$0

$L28:
	lw	$5,8($3)
	sltu	$7,$5,$6
	bne	$7,$0,$L26
	and	$8,$4,$5

	lw	$7,4($3)
	bne	$8,$7,$L26
	nop

	j	$L27
	addiu	$2,$3,16

$L26:
	move	$5,$6
$L27:
	lw	$3,44($3)
	move	$6,$5
$L25:
	bne	$3,$0,$L28
	nop

	j	$31
	nop

	.set	macro
	.set	reorder
	.end	FastPath_Route
	.size	FastPath_Route, .-FastPath_Route
	.section	.text.mark_path_invalid,"ax",@progbits
	.align	2
	.set	nomips16
	.ent	mark_path_invalid
	.type	mark_path_invalid, @function
mark_path_invalid:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	srl	$8,$4,8
	lhu	$2,18($sp)
	lui	$3,%hi(path_table_list_max)
	xor	$8,$8,$4
	lw	$3,%lo(path_table_list_max)($3)
	lui	$10,%hi(table_path)
	srl	$11,$6,16
	lw	$10,%lo(table_path)($10)
	andi	$5,$5,0xffff
	addiu	$3,$3,-1
	xor	$8,$8,$6
	lw	$10,0($10)
	xor	$8,$8,$11
	srl	$11,$5,4
	xor	$8,$8,$11
	andi	$7,$7,0xffff
	xor	$8,$8,$7
	xor	$8,$8,$2
	srl	$11,$8,12
	xor	$8,$11,$8
	li	$11,12			# 0xc
	and	$8,$8,$3
	mult	$8,$11
	mflo	$8
	addu	$10,$10,$8
	lw	$10,0($10)
	j	$L31
	li	$9,255			# 0xff

$L35:
	lw	$8,12($10)
	bne	$8,$5,$L32
	nop

	lw	$8,8($10)
	bne	$8,$7,$L32
	nop

	lw	$8,16($10)
	bne	$8,$4,$L32
	nop

	lw	$8,20($10)
	bne	$8,$6,$L32
	nop

	lbu	$11,0($10)
	bne	$11,$9,$L32
	nop

	lbu	$9,2($10)
	li	$6,1			# 0x1
	bne	$9,$6,$L33
	sb	$0,0($10)

	lw	$6,24($10)
	lhu	$9,30($10)
	j	$L34
	nop

$L33:
	lw	$6,32($10)
	lhu	$9,38($10)
	j	$L34
	nop

$L32:
	lw	$10,64($10)
$L31:
	bne	$10,$0,$L35
	nop

	j	$31
	nop

$L34:
	beq	$6,$0,$L44
	nop

	lbu	$11,2($10)
	li	$10,1			# 0x1
	bne	$11,$10,$L37
	srl	$10,$4,16

	srl	$10,$8,8
	xor	$10,$10,$8
	xor	$2,$10,$2
	srl	$10,$7,4
	xor	$2,$2,$10
	xor	$2,$2,$6
	xor	$9,$2,$9
	j	$L43
	srl	$6,$6,16

$L37:
	xor	$10,$10,$4
	xor	$10,$10,$5
	xor	$2,$10,$2
	xor	$2,$2,$6
	srl	$6,$6,8
	xor	$6,$2,$6
	srl	$9,$9,4
$L43:
	xor	$9,$6,$9
	li	$2,12			# 0xc
	srl	$6,$9,12
	xor	$9,$6,$9
	and	$3,$9,$3
	mult	$3,$2
	lui	$2,%hi(table_path)
	lw	$2,%lo(table_path)($2)
	lw	$2,0($2)
	mflo	$3
	addu	$3,$2,$3
	lw	$2,0($3)
	j	$L39
	li	$6,255			# 0xff

$L41:
	lw	$3,24($2)
	bne	$3,$8,$L40
	nop

	lw	$3,28($2)
	bne	$3,$7,$L40
	nop

	lw	$3,32($2)
	bne	$3,$4,$L40
	nop

	lw	$3,36($2)
	bne	$3,$5,$L40
	nop

	lbu	$3,0($2)
	bne	$3,$6,$L40
	nop

	sb	$0,0($2)
$L40:
	lw	$2,64($2)
$L39:
	bne	$2,$0,$L41
	nop

$L44:
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	mark_path_invalid
	.size	mark_path_invalid, .-mark_path_invalid
	.section	.text.negative_cache_timeout,"ax",@progbits
	.align	2
	.set	nomips16
	.ent	negative_cache_timeout
	.type	negative_cache_timeout, @function
negative_cache_timeout:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	lw	$2,4($4)
	lui	$6,%hi(max_udp_frag_entry)
	lw	$5,8($4)
	lhu	$3,2($4)
	xor	$5,$5,$2
	lbu	$2,1($4)
	srl	$7,$5,16
	lw	$6,%lo(max_udp_frag_entry)($6)
	xor	$3,$3,$5
	lw	$5,36($4)
	xor	$3,$3,$7
	addiu	$6,$6,-1
	xor	$2,$3,$2
	sb	$0,0($4)
	srl	$7,$3,8
	lw	$3,40($4)
	xor	$2,$2,$7
	beq	$5,$0,$L47
	and	$2,$2,$6

	j	$L48
	sw	$3,40($5)

$L47:
	li	$5,12			# 0xc
	lui	$6,%hi(negative_cache_table)
	mult	$2,$5
	lw	$6,%lo(negative_cache_table)($6)
	mflo	$5
	addu	$5,$6,$5
	sw	$3,4($5)
$L48:
	li	$3,12			# 0xc
	addiu	$5,$4,44
	lw	$6,36($4)
	mult	$2,$3
	lw	$3,40($4)
	sw	$6,0($3)
	lui	$6,%hi(negative_cache_table)
	lw	$6,%lo(negative_cache_table)($6)
	mflo	$2
	lui	$3,%hi(negative_cache_list_free)
	addu	$2,$6,$2
	addiu	$3,$3,%lo(negative_cache_list_free)
	lw	$6,8($2)
	addiu	$6,$6,-1
	sw	$6,8($2)
	sw	$0,44($4)
	lw	$2,4($3)
	sw	$2,48($4)
	lw	$2,4($3)
	sw	$4,0($2)
	lw	$2,8($3)
	sw	$5,4($3)
	addiu	$2,$2,1
	j	$31
	sw	$2,8($3)

	.set	macro
	.set	reorder
	.end	negative_cache_timeout
	.size	negative_cache_timeout, .-negative_cache_timeout
	.section	.text.negative_fragCache_init,"ax",@progbits
	.align	2
	.globl	negative_fragCache_init
	.set	nomips16
	.ent	negative_fragCache_init
	.type	negative_fragCache_init, @function
negative_fragCache_init:
	.frame	$sp,48,$31		# vars= 8, regs= 6/0, args= 16, gp= 0
	.mask	0x801f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
$L51 = .
$L52 = .
	addiu	$sp,$sp,-48
	sw	$31,44($sp)
	sw	$20,40($sp)
	sw	$19,36($sp)
	sw	$18,32($sp)
	sw	$17,28($sp)
	sw	$16,24($sp)
	lui	$2,%hi(malloc_sizes+60)
	lw	$4,%lo(malloc_sizes+60)($2)
	jal	kmem_cache_alloc
	li	$5,32			# 0x20

	lui	$3,%hi(negative_cache_table)
	beq	$2,$0,$L53
	sw	$2,%lo(negative_cache_table)($3)

	lui	$4,%hi(negative_cache_list_free)
	move	$2,$0
	addiu	$6,$4,%lo(negative_cache_list_free)
	li	$5,12			# 0xc
	sw	$0,%lo(negative_cache_list_free)($4)
	li	$4,64			# 0x40
	sw	$6,4($6)
	sw	$0,8($6)
$L54:
	mult	$2,$5
	lw	$7,%lo(negative_cache_table)($3)
	addiu	$2,$2,1
	mflo	$6
	addu	$7,$7,$6
	sw	$0,8($7)
	lw	$7,%lo(negative_cache_table)($3)
	addu	$7,$7,$6
	sw	$0,0($7)
	lw	$7,%lo(negative_cache_table)($3)
	addu	$6,$7,$6
	bne	$2,$4,$L54
	sw	$6,4($6)

	lui	$20,%hi(malloc_sizes)
	lui	$19,%hi(negative_cache_timeout)
	lui	$16,%hi(negative_cache_list_free)
	addiu	$20,$20,%lo(malloc_sizes)
	addiu	$19,$19,%lo(negative_cache_timeout)
	addiu	$16,$16,%lo(negative_cache_list_free)
	move	$17,$0
	li	$18,64			# 0x40
$L55:
	lw	$4,12($20)
	jal	kmem_cache_alloc
	li	$5,32			# 0x20

	beq	$2,$0,$L53
	addiu	$4,$2,12

	move	$5,$0
	move	$6,$0
	addiu	$17,$17,1
	jal	init_timer_key
	sw	$2,16($sp)

	lw	$2,16($sp)
	sw	$2,28($2)
	addiu	$3,$2,44
	sw	$19,24($2)
	sw	$0,44($2)
	lw	$4,4($16)
	sw	$4,48($2)
	lw	$4,4($16)
	sw	$2,0($4)
	lw	$2,8($16)
	sw	$3,4($16)
	addiu	$2,$2,1
	bne	$17,$18,$L55
	sw	$2,8($16)

	j	$L56
	li	$2,1			# 0x1

$L53:
	move	$2,$0
$L56:
	lw	$31,44($sp)
	lw	$20,40($sp)
	lw	$19,36($sp)
	lw	$18,32($sp)
	lw	$17,28($sp)
	lw	$16,24($sp)
	j	$31
	addiu	$sp,$sp,48

	.set	macro
	.set	reorder
	.end	negative_fragCache_init
	.size	negative_fragCache_init, .-negative_fragCache_init
	.section	.text.init_table_path,"ax",@progbits
	.align	2
	.globl	init_table_path
	.set	nomips16
	.ent	init_table_path
	.type	init_table_path, @function
init_table_path:
	.frame	$sp,40,$31		# vars= 0, regs= 5/0, args= 16, gp= 0
	.mask	0x800f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
$L61 = .
$L62 = .
	addiu	$sp,$sp,-40
	sw	$31,36($sp)
	sw	$19,32($sp)
	sw	$18,28($sp)
	sw	$17,24($sp)
	sw	$16,20($sp)
	lui	$2,%hi(malloc_sizes+4)
	move	$17,$4
	lw	$4,%lo(malloc_sizes+4)($2)
	move	$19,$5
	li	$5,32			# 0x20
	jal	kmem_cache_alloc
	lui	$16,%hi(table_path)

	move	$18,$2
	beq	$2,$0,$L63
	sw	$2,%lo(table_path)($16)

$L64 = .
	li	$4,12			# 0xc
	lui	$2,%hi(path_list_inuse)
	mult	$17,$4
	lui	$7,%hi(path_list_free)
	addiu	$6,$2,%lo(path_list_inuse)
	addiu	$3,$7,%lo(path_list_free)
	li	$5,32			# 0x20
	sw	$0,%lo(path_list_inuse)($2)
	lui	$2,%hi(path_table_list_max)
	mflo	$4
	sw	$6,4($6)
	sw	$3,4($3)
	sw	$0,8($6)
	sw	$0,8($3)
	sw	$0,%lo(path_list_free)($7)
	jal	__kmalloc
	sw	$17,%lo(path_table_list_max)($2)

	sw	$2,0($18)
	lw	$2,%lo(table_path)($16)
	lw	$2,0($2)
	beq	$2,$0,$L63
	move	$2,$0

	j	$L65
	li	$3,12			# 0xc

$L66:
	lw	$5,%lo(table_path)($16)
	addiu	$2,$2,1
	lw	$5,0($5)
	mflo	$4
	addu	$5,$5,$4
	sw	$0,8($5)
	lw	$5,%lo(table_path)($16)
	lw	$5,0($5)
	addu	$5,$5,$4
	sw	$0,0($5)
	lw	$5,%lo(table_path)($16)
	lw	$5,0($5)
	addu	$4,$5,$4
	sw	$4,4($4)
$L65:
	slt	$4,$2,$17
	bne	$4,$0,$L66
	mult	$2,$3

	lui	$18,%hi(malloc_sizes)
	lui	$16,%hi(path_list_free)
	addiu	$18,$18,%lo(malloc_sizes)
	addiu	$16,$16,%lo(path_list_free)
	j	$L67
	move	$17,$0

$L68:
$L69 = .
	lw	$4,20($18)
	jal	kmem_cache_alloc
	li	$5,32			# 0x20

	bne	$2,$0,$L70
	addiu	$3,$2,72

	j	$L71
	li	$2,-2			# 0xfffffffe

$L70:
	sw	$0,72($2)
	lw	$4,4($16)
	addiu	$17,$17,1
	sw	$4,76($2)
	lw	$4,4($16)
	sw	$2,0($4)
	lw	$2,8($16)
	sw	$3,4($16)
	addiu	$2,$2,1
	sw	$2,8($16)
$L67:
	slt	$2,$17,$19
	bne	$2,$0,$L68
	move	$2,$0

	j	$L71
	nop

$L63:
	li	$2,-1			# 0xffffffff
$L71:
	lw	$31,36($sp)
	lw	$19,32($sp)
	lw	$18,28($sp)
	lw	$17,24($sp)
	lw	$16,20($sp)
	j	$31
	addiu	$sp,$sp,40

	.set	macro
	.set	reorder
	.end	init_table_path
	.size	init_table_path, .-init_table_path
	.section	.text.init_table_napt,"ax",@progbits
	.align	2
	.globl	init_table_napt
	.set	nomips16
	.ent	init_table_napt
	.type	init_table_napt, @function
init_table_napt:
	.frame	$sp,40,$31		# vars= 0, regs= 5/0, args= 16, gp= 0
	.mask	0x800f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
$L74 = .
$L75 = .
	addiu	$sp,$sp,-40
	sw	$31,36($sp)
	sw	$19,32($sp)
	sw	$18,28($sp)
	sw	$17,24($sp)
	sw	$16,20($sp)
	lui	$2,%hi(malloc_sizes+4)
	move	$17,$4
	lw	$4,%lo(malloc_sizes+4)($2)
	move	$19,$5
	li	$5,32			# 0x20
	jal	kmem_cache_alloc
	lui	$16,%hi(table_napt)

	move	$18,$2
	beq	$2,$0,$L76
	sw	$2,%lo(table_napt)($16)

$L77 = .
	li	$4,12			# 0xc
	lui	$2,%hi(napt_list_inuse)
	mult	$17,$4
	lui	$7,%hi(napt_list_free)
	addiu	$6,$2,%lo(napt_list_inuse)
	addiu	$3,$7,%lo(napt_list_free)
	li	$5,32			# 0x20
	sw	$0,%lo(napt_list_inuse)($2)
	lui	$2,%hi(napt_table_list_max)
	mflo	$4
	sw	$6,4($6)
	sw	$3,4($3)
	sw	$0,8($6)
	sw	$0,8($3)
	sw	$0,%lo(napt_list_free)($7)
	jal	__kmalloc
	sw	$17,%lo(napt_table_list_max)($2)

	sw	$2,0($18)
	lw	$2,%lo(table_napt)($16)
	lw	$2,0($2)
	beq	$2,$0,$L76
	move	$2,$0

	j	$L78
	li	$3,12			# 0xc

$L79:
	lw	$5,%lo(table_napt)($16)
	addiu	$2,$2,1
	lw	$5,0($5)
	mflo	$4
	addu	$5,$5,$4
	sw	$0,8($5)
	lw	$5,%lo(table_napt)($16)
	lw	$5,0($5)
	addu	$5,$5,$4
	sw	$0,0($5)
	lw	$5,%lo(table_napt)($16)
	lw	$5,0($5)
	addu	$4,$5,$4
	sw	$4,4($4)
$L78:
	slt	$4,$2,$17
	bne	$4,$0,$L79
	mult	$2,$3

	lui	$18,%hi(malloc_sizes)
	lui	$16,%hi(napt_list_free)
	addiu	$18,$18,%lo(malloc_sizes)
	addiu	$16,$16,%lo(napt_list_free)
	j	$L80
	move	$17,$0

$L81:
$L82 = .
	lw	$4,12($18)
	jal	kmem_cache_alloc
	li	$5,32			# 0x20

	bne	$2,$0,$L83
	addiu	$3,$2,44

	j	$L84
	li	$2,-2			# 0xfffffffe

$L83:
	sw	$0,44($2)
	lw	$4,4($16)
	addiu	$17,$17,1
	sw	$4,48($2)
	lw	$4,4($16)
	sw	$2,0($4)
	lw	$2,8($16)
	sw	$3,4($16)
	addiu	$2,$2,1
	sw	$2,8($16)
$L80:
	slt	$2,$17,$19
	bne	$2,$0,$L81
	move	$2,$0

	j	$L84
	nop

$L76:
	li	$2,-1			# 0xffffffff
$L84:
	lw	$31,36($sp)
	lw	$19,32($sp)
	lw	$18,28($sp)
	lw	$17,24($sp)
	lw	$16,20($sp)
	j	$31
	addiu	$sp,$sp,40

	.set	macro
	.set	reorder
	.end	init_table_napt
	.size	init_table_napt, .-init_table_napt
	.section	.text.udp_fragCache_init,"ax",@progbits
	.align	2
	.globl	udp_fragCache_init
	.set	nomips16
	.ent	udp_fragCache_init
	.type	udp_fragCache_init, @function
udp_fragCache_init:
	.frame	$sp,48,$31		# vars= 8, regs= 6/0, args= 16, gp= 0
	.mask	0x801f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
$L87 = .
$L88 = .
	addiu	$sp,$sp,-48
	sw	$31,44($sp)
	sw	$20,40($sp)
	sw	$19,36($sp)
	sw	$18,32($sp)
	sw	$17,28($sp)
	sw	$16,24($sp)
	lui	$2,%hi(malloc_sizes+4)
	move	$17,$4
	lw	$4,%lo(malloc_sizes+4)($2)
	li	$5,32			# 0x20
	jal	kmem_cache_alloc
	lui	$16,%hi(udp_cache_table)

	move	$18,$2
	beq	$2,$0,$L89
	sw	$2,%lo(udp_cache_table)($16)

$L90 = .
	li	$4,12			# 0xc
	lui	$2,%hi(udp_cache_list_free)
	mult	$17,$4
	addiu	$3,$2,%lo(udp_cache_list_free)
	sw	$0,%lo(udp_cache_list_free)($2)
	sw	$3,4($3)
	lui	$2,%hi(max_udp_frag_entry)
	sw	$0,8($3)
	li	$5,32			# 0x20
	mflo	$4
	jal	__kmalloc
	sw	$17,%lo(max_udp_frag_entry)($2)

	move	$3,$0
	sw	$2,0($18)
	lw	$4,%lo(udp_cache_table)($16)
	lw	$4,0($4)
	bne	$4,$0,$L100
	li	$2,-1			# 0xffffffff

	j	$L92
	nop

$L94:
	lw	$5,%lo(udp_cache_table)($16)
	addiu	$3,$3,1
	lw	$5,0($5)
	mflo	$4
	addu	$5,$5,$4
	sw	$0,8($5)
	lw	$5,%lo(udp_cache_table)($16)
	lw	$5,0($5)
	addu	$5,$5,$4
	sw	$0,0($5)
	lw	$5,%lo(udp_cache_table)($16)
	lw	$5,0($5)
	addu	$4,$5,$4
	j	$L99
	sw	$4,4($4)

$L100:
	li	$2,12			# 0xc
$L99:
	slt	$4,$3,$17
	bne	$4,$0,$L94
	mult	$3,$2

	lui	$20,%hi(malloc_sizes)
	lui	$19,%hi(cache_timeout)
	lui	$16,%hi(udp_cache_list_free)
	addiu	$20,$20,%lo(malloc_sizes)
	addiu	$19,$19,%lo(cache_timeout)
	addiu	$16,$16,%lo(udp_cache_list_free)
	j	$L95
	move	$18,$0

$L96:
$L97 = .
	lw	$4,12($20)
	jal	kmem_cache_alloc
	li	$5,32			# 0x20

	beq	$2,$0,$L89
	addiu	$4,$2,16

	move	$5,$0
	move	$6,$0
	addiu	$18,$18,1
	jal	init_timer_key
	sw	$2,16($sp)

	lw	$2,16($sp)
	sw	$2,32($2)
	addiu	$3,$2,48
	sw	$19,28($2)
	sw	$0,48($2)
	lw	$4,4($16)
	sw	$4,52($2)
	lw	$4,4($16)
	sw	$2,0($4)
	lw	$2,8($16)
	sw	$3,4($16)
	addiu	$2,$2,1
	sw	$2,8($16)
$L95:
	slt	$2,$18,$17
	bne	$2,$0,$L96
	li	$2,1			# 0x1

	j	$L92
	nop

$L89:
	move	$2,$0
$L92:
	lw	$31,44($sp)
	lw	$20,40($sp)
	lw	$19,36($sp)
	lw	$18,32($sp)
	lw	$17,28($sp)
	lw	$16,24($sp)
	j	$31
	addiu	$sp,$sp,48

	.set	macro
	.set	reorder
	.end	udp_fragCache_init
	.size	udp_fragCache_init, .-udp_fragCache_init
	.section	.rodata.str1.4,"aMS",@progbits,1
	.align	2
$LC0:
	.ascii	"%s:%s\012\000"
	.align	2
$LC1:
	.ascii	"Realtek FastPath\000"
	.align	2
$LC2:
	.ascii	"v1.03\000"
	.section	.text.get_fastpath_module_info,"ax",@progbits
	.align	2
	.globl	get_fastpath_module_info
	.set	nomips16
	.ent	get_fastpath_module_info
	.type	get_fastpath_module_info, @function
get_fastpath_module_info:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	beq	$4,$0,$L104
	nop

	lui	$5,%hi($LC0)
	lui	$6,%hi($LC1)
	lui	$7,%hi($LC2)
	addiu	$5,$5,%lo($LC0)
	addiu	$6,$6,%lo($LC1)
	addiu	$7,$7,%lo($LC2)
	j	sprintf
	nop

$L104:
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	get_fastpath_module_info
	.size	get_fastpath_module_info, .-get_fastpath_module_info
	.section	.rodata.str1.4
	.align	2
$LC3:
	.ascii	"napt num: %d, path num: %d.\012\000"
	.section	.text.fastpath_dump_napt_entry_num,"ax",@progbits
	.align	2
	.globl	fastpath_dump_napt_entry_num
	.set	nomips16
	.ent	fastpath_dump_napt_entry_num
	.type	fastpath_dump_napt_entry_num, @function
fastpath_dump_napt_entry_num:
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$16,16($sp)
	lui	$2,%hi(path_list_inuse)
	move	$16,$5
	lw	$2,%lo(path_list_inuse)($2)
	j	$L106
	move	$7,$0

$L107:
	lw	$2,72($2)
	addiu	$7,$7,1
$L106:
	bne	$2,$0,$L107
	move	$6,$0

	lui	$2,%hi(napt_list_inuse)
	lw	$2,%lo(napt_list_inuse)($2)
	j	$L108
	nop

$L109:
	lw	$2,44($2)
	addiu	$6,$6,1
$L108:
	bne	$2,$0,$L109
	lui	$5,%hi($LC3)

	addu	$4,$4,$16
	jal	sprintf
	addiu	$5,$5,%lo($LC3)

	addu	$2,$2,$16
	lw	$31,20($sp)
	lw	$16,16($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	fastpath_dump_napt_entry_num
	.size	fastpath_dump_napt_entry_num, .-fastpath_dump_napt_entry_num
	.section	.rodata.str1.4
	.align	2
$LC4:
	.ascii	"lo\000"
	.section	.text.enter_fast_path,"ax",@progbits
	.align	2
	.globl	enter_fast_path
	.set	nomips16
	.ent	enter_fast_path
	.type	enter_fast_path, @function
enter_fast_path:
	.frame	$sp,88,$31		# vars= 24, regs= 10/0, args= 24, gp= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-88
	sw	$31,84($sp)
	sw	$fp,80($sp)
	sw	$23,76($sp)
	sw	$22,72($sp)
	sw	$21,68($sp)
	sw	$20,64($sp)
	sw	$19,60($sp)
	sw	$18,56($sp)
	sw	$17,52($sp)
	sw	$16,48($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_dev_name
	move	$20,$4
	.set	macro
	.set	reorder

	move	$4,$20
	.set	noreorder
	.set	nomacro
	jal	rtl_ip_hdr
	move	$17,$2
	.set	macro
	.set	reorder

	move	$16,$2
	lbu	$2,8($2)
	sltu	$2,$2,2
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L112
	li	$2,6			# 0x6
	.set	macro
	.set	reorder

	lbu	$22,9($16)
	.set	noreorder
	.set	nomacro
	beq	$22,$2,$L113
	li	$2,17			# 0x11
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$22,$2,$L114
	move	$4,$0
	.set	macro
	.set	reorder

$L113:
	lbu	$19,0($16)
	move	$4,$16
	move	$6,$20
	andi	$19,$19,0xf
	sll	$19,$19,2
	addu	$19,$16,$19
	.set	noreorder
	.set	nomacro
	jal	fast_path_pre_process_check
	move	$5,$19
	.set	macro
	.set	reorder

	move	$4,$2
	li	$2,6			# 0x6
	.set	noreorder
	.set	nomacro
	bne	$4,$2,$L114
	lui	$3,%hi($LC4)
	.set	macro
	.set	reorder

	addiu	$2,$3,%lo($LC4)
#APP
 # 75 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/string.h" 1
	.set	noreorder
	.set	noat
	lbu	$3,($17)
1:	lbu	$1,($2)
	addiu	$17,1
	bne	$1,$3,2f
	addiu	$2,1
	bnez	$3,1b
	lbu	$3,($17)
	move	$3,$1
2:	subu	$3,$1
3:	.set	at
	.set	reorder
 # 0 "" 2
#NO_APP
	beq	$3,$0,$L112
	lhu	$5,0($19)
	lhu	$7,2($19)
	lhu	$17,6($16)
	sw	$5,36($sp)
	lw	$21,12($16)
	sw	$7,32($sp)
	lw	$23,16($16)
	.set	noreorder
	.set	nomacro
	bne	$22,$4,$L115
	andi	$2,$17,0x3fff
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L116
	li	$2,8192			# 0x2000
	.set	macro
	.set	reorder

	bne	$17,$2,$L117
	j	$L116
$L243:
	move	$4,$21
	move	$6,$23
	.set	noreorder
	.set	nomacro
	jal	mark_path_invalid
	sw	$2,16($sp)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L114
	move	$4,$0
	.set	macro
	.set	reorder

$L208:
	and	$2,$2,$3
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L114
	move	$4,$0
	.set	macro
	.set	reorder

$L117:
	lw	$3,12($19)
	li	$2,1048576			# 0x100000
	and	$2,$3,$2
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L115
	li	$2,40			# 0x28
	.set	macro
	.set	reorder

	lhu	$3,2($16)
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L115
	move	$4,$20
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	skb_trim
	li	$5,40			# 0x28
	.set	macro
	.set	reorder

$L115:
	lhu	$2,6($16)
	andi	$2,$2,0x3fff
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L118
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

	li	$2,8192			# 0x2000
	.set	noreorder
	.set	nomacro
	bne	$17,$2,$L119
	move	$8,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L120
	li	$fp,1			# 0x1
	.set	macro
	.set	reorder

$L119:
	andi	$2,$17,0x2000
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L120
	move	$fp,$0
	.set	macro
	.set	reorder

	andi	$17,$17,0x1fff
	.set	noreorder
	.set	nomacro
	bne	$17,$0,$L120
	li	$8,1			# 0x1
	.set	macro
	.set	reorder

	move	$8,$0
	move	$fp,$0
$L120:
	lw	$6,12($16)
	lui	$3,%hi(max_udp_frag_entry)
	lw	$2,16($16)
	lhu	$5,4($16)
	xor	$9,$2,$6
	lbu	$4,9($16)
	xor	$7,$5,$9
	lw	$3,%lo(max_udp_frag_entry)($3)
	srl	$11,$9,16
	lui	$9,%hi(negative_cache_table)
	xor	$11,$7,$11
	addiu	$3,$3,-1
	xor	$7,$11,$4
	lw	$9,%lo(negative_cache_table)($9)
	srl	$11,$11,8
	xor	$7,$7,$11
	li	$11,12			# 0xc
	and	$7,$7,$3
	mult	$7,$11
	mflo	$11
	addu	$9,$9,$11
	lw	$9,0($9)
	.set	noreorder
	.set	nomacro
	j	$L122
	li	$10,3			# 0x3
	.set	macro
	.set	reorder

$L124:
	lhu	$11,2($9)
	bne	$11,$5,$L123
	lw	$11,4($9)
	bne	$11,$6,$L123
	lw	$11,8($9)
	bne	$11,$2,$L123
	lbu	$11,1($9)
	bne	$11,$4,$L123
	lbu	$11,0($9)
	beq	$11,$10,$L112
$L123:
	lw	$9,36($9)
$L122:
	bne	$9,$0,$L124
	.set	noreorder
	.set	nomacro
	j	$L241
	li	$9,12			# 0xc
	.set	macro
	.set	reorder

$L128:
	lhu	$7,2($18)
	bne	$7,$5,$L126
	lw	$7,4($18)
	bne	$7,$6,$L126
	lw	$7,8($18)
	bne	$7,$2,$L126
	lbu	$7,1($18)
	bne	$7,$4,$L126
	lbu	$7,0($18)
	beq	$7,$10,$L127
$L126:
	lw	$18,40($18)
$L203:
	bne	$18,$0,$L128
	j	$L242
$L127:
	lhu	$12,12($18)
	sw	$0,24($sp)
	sw	$12,36($sp)
	lhu	$2,14($18)
	.set	noreorder
	.set	nomacro
	beq	$fp,$0,$L131
	sw	$2,32($sp)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L244
	li	$fp,1			# 0x1
	.set	macro
	.set	reorder

$L204:
	lui	$6,%hi(negative_cache_list_free)
	lw	$2,%lo(negative_cache_list_free)($6)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L112
	li	$7,3			# 0x3
	.set	macro
	.set	reorder

	lw	$8,44($2)
	sb	$7,0($2)
	sh	$5,2($2)
	sw	$21,4($2)
	sw	$23,8($2)
	sb	$4,1($2)
	lw	$7,48($2)
	.set	noreorder
	.set	nomacro
	beq	$8,$0,$L132
	addiu	$6,$6,%lo(negative_cache_list_free)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L133
	sw	$7,48($8)
	.set	macro
	.set	reorder

$L132:
	sw	$7,4($6)
$L133:
	lw	$7,48($2)
	xor	$21,$23,$21
	lui	$8,%hi(negative_cache_list_free)
	lw	$9,44($2)
	addiu	$8,$8,%lo(negative_cache_list_free)
	xor	$5,$21,$5
	srl	$21,$21,16
	lui	$6,%hi(negative_cache_table)
	sw	$9,0($7)
	xor	$5,$5,$21
	addiu	$7,$2,36
	lw	$9,8($8)
	xor	$10,$5,$4
	addiu	$4,$2,12
	srl	$5,$5,8
	xor	$5,$10,$5
	addiu	$9,$9,-1
	sw	$9,8($8)
	and	$3,$5,$3
	li	$8,12			# 0xc
	mult	$3,$8
	sw	$0,36($2)
	lw	$5,%lo(negative_cache_table)($6)
	mflo	$3
	addu	$5,$5,$3
	lw	$5,4($5)
	sw	$5,40($2)
	lw	$5,%lo(negative_cache_table)($6)
	addu	$5,$5,$3
	lw	$5,4($5)
	sw	$2,0($5)
	lw	$5,%lo(negative_cache_table)($6)
	addu	$5,$5,$3
	sw	$7,4($5)
	lw	$5,%lo(negative_cache_table)($6)
	addu	$3,$5,$3
	lw	$5,8($3)
	addiu	$5,$5,1
	sw	$5,8($3)
	lui	$3,%hi(jiffies)
	lw	$3,%lo(jiffies)($3)
	addiu	$3,$3,1000
	.set	noreorder
	.set	nomacro
	jal	add_timer
	sw	$3,20($2)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L114
	move	$4,$0
	.set	macro
	.set	reorder

$L118:
	move	$18,$0
	move	$8,$0
	move	$fp,$0
	.set	noreorder
	.set	nomacro
	j	$L131
	sw	$2,24($sp)
	.set	macro
	.set	reorder

$L244:
	sw	$fp,24($sp)
$L131:
	srl	$2,$21,8
	lw	$11,32($sp)
	lui	$3,%hi(path_table_list_max)
	xor	$9,$23,$21
	lw	$12,36($sp)
	li	$7,255			# 0xff
	srl	$4,$23,16
	lw	$10,%lo(path_table_list_max)($3)
	li	$6,2			# 0x2
	xor	$9,$9,$2
	lui	$2,%hi(table_path)
	li	$5,1			# 0x1
	xor	$9,$9,$4
	addiu	$10,$10,-1
	xor	$9,$9,$22
	lw	$3,%lo(table_path)($2)
	xor	$9,$9,$11
	sw	$4,28($sp)
	li	$4,17			# 0x11
	srl	$11,$12,4
	lw	$2,0($3)
	xor	$9,$9,$11
	srl	$11,$9,12
	xor	$9,$11,$9
	and	$9,$9,$10
	li	$10,12			# 0xc
	mult	$9,$10
	mflo	$9
	addu	$2,$2,$9
	lw	$17,0($2)
	.set	noreorder
	.set	nomacro
	j	$L134
	li	$3,6			# 0x6
	.set	macro
	.set	reorder

$L200:
	lw	$2,12($17)
	lw	$11,36($sp)
	bne	$2,$11,$L135
	lw	$2,8($17)
	lw	$12,32($sp)
	bne	$2,$12,$L135
	lw	$2,16($17)
	bne	$2,$21,$L135
	lw	$9,20($17)
	bne	$9,$23,$L135
	lbu	$2,0($17)
	bne	$2,$7,$L135
	lw	$2,4($17)
	bne	$2,$6,$L136
	bne	$22,$3,$L135
	j	$L137
$L136:
	bne	$2,$5,$L135
	bne	$22,$4,$L135
$L137:
	lw	$5,32($17)
	move	$4,$20
	lbu	$7,1($16)
	move	$6,$21
	sw	$8,40($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_ip_route_input
	sw	$9,44($sp)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L114
	move	$4,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_skb_dst_check
	move	$4,$20
	.set	macro
	.set	reorder

	li	$3,-1			# 0xffffffff
	lw	$8,40($sp)
	lw	$9,44($sp)
	.set	noreorder
	.set	nomacro
	beq	$2,$3,$L143
	li	$2,6			# 0x6
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$22,$2,$L140
	addiu	$3,$19,16
	.set	macro
	.set	reorder

	lhu	$3,6($19)
	addiu	$2,$19,6
	movz	$2,$0,$3
	move	$3,$2
$L140:
	.set	noreorder
	.set	nomacro
	beq	$fp,$0,$L142
	lui	$5,%hi(udp_cache_list_free)
	.set	macro
	.set	reorder

	lui	$4,%hi(max_udp_frag_entry)
	lw	$2,%lo(udp_cache_list_free)($5)
	lw	$11,%lo(max_udp_frag_entry)($4)
	lhu	$7,4($16)
	beq	$2,$0,$L143
	lw	$4,36($sp)
	lw	$6,48($2)
	sh	$4,12($2)
	li	$4,1			# 0x1
	lw	$12,32($sp)
	sb	$4,0($2)
	sh	$12,14($2)
	sh	$7,2($2)
	sw	$21,4($2)
	sw	$9,8($2)
	sb	$22,1($2)
	lw	$4,52($2)
	.set	noreorder
	.set	nomacro
	beq	$6,$0,$L144
	addiu	$5,$5,%lo(udp_cache_list_free)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L145
	sw	$4,52($6)
	.set	macro
	.set	reorder

$L144:
	sw	$4,4($5)
$L145:
	lw	$4,52($2)
	xor	$9,$9,$21
	lui	$8,%hi(udp_cache_list_free)
	lw	$5,48($2)
	addiu	$8,$8,%lo(udp_cache_list_free)
	xor	$7,$7,$9
	srl	$9,$9,16
	addiu	$11,$11,-1
	addiu	$6,$2,40
	sw	$5,0($4)
	xor	$7,$7,$9
	lui	$5,%hi(udp_cache_table)
	lw	$10,8($8)
	srl	$12,$7,8
	addiu	$4,$2,16
	xor	$7,$7,$22
	xor	$7,$7,$12
	addiu	$10,$10,-1
	sw	$10,8($8)
	and	$7,$7,$11
	li	$8,12			# 0xc
	sw	$0,40($2)
	mult	$7,$8
	lw	$8,%lo(udp_cache_table)($5)
	lw	$8,0($8)
	mflo	$7
	addu	$8,$8,$7
	lw	$8,4($8)
	sw	$8,44($2)
	lw	$8,%lo(udp_cache_table)($5)
	lw	$8,0($8)
	addu	$8,$8,$7
	lw	$8,4($8)
	sw	$2,0($8)
	lw	$8,%lo(udp_cache_table)($5)
	lw	$8,0($8)
	addu	$8,$8,$7
	sw	$6,4($8)
	lw	$5,%lo(udp_cache_table)($5)
	lw	$5,0($5)
	addu	$7,$5,$7
	lw	$5,8($7)
	addiu	$5,$5,1
	sw	$5,8($7)
	lui	$5,%hi(jiffies)
	lw	$5,%lo(jiffies)($5)
	addiu	$5,$5,1000
	sw	$5,24($2)
	.set	noreorder
	.set	nomacro
	jal	add_timer
	sw	$3,40($sp)
	.set	macro
	.set	reorder

	lw	$3,40($sp)
	.set	noreorder
	.set	nomacro
	j	$L245
	move	$4,$20
	.set	macro
	.set	reorder

$L143:
	.set	noreorder
	.set	nomacro
	jal	rtl_dst_release
	move	$4,$20
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L114
	move	$4,$0
	.set	macro
	.set	reorder

$L142:
	.set	noreorder
	.set	nomacro
	beq	$8,$0,$L245
	move	$4,$20
	.set	macro
	.set	reorder

	addiu	$4,$18,16
	.set	noreorder
	.set	nomacro
	jal	del_timer
	sw	$3,40($sp)
	.set	macro
	.set	reorder

	lui	$6,%hi(max_udp_frag_entry)
	lw	$2,4($18)
	lw	$5,8($18)
	lhu	$4,2($18)
	xor	$5,$5,$2
	lbu	$2,1($18)
	srl	$7,$5,16
	lw	$6,%lo(max_udp_frag_entry)($6)
	xor	$4,$4,$5
	lw	$5,40($18)
	xor	$4,$4,$7
	addiu	$6,$6,-1
	xor	$2,$4,$2
	sb	$0,0($18)
	srl	$7,$4,8
	lw	$3,40($sp)
	xor	$2,$2,$7
	lw	$4,44($18)
	.set	noreorder
	.set	nomacro
	beq	$5,$0,$L147
	and	$2,$2,$6
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L148
	sw	$4,44($5)
	.set	macro
	.set	reorder

$L147:
	li	$5,12			# 0xc
	lui	$6,%hi(udp_cache_table)
	mult	$2,$5
	lw	$6,%lo(udp_cache_table)($6)
	lw	$6,0($6)
	mflo	$5
	addu	$5,$6,$5
	sw	$4,4($5)
$L148:
	lw	$4,44($18)
	li	$6,12			# 0xc
	lw	$5,40($18)
	mult	$2,$6
	sw	$5,0($4)
	lui	$4,%hi(udp_cache_table)
	lw	$6,%lo(udp_cache_table)($4)
	lui	$4,%hi(udp_cache_list_free)
	mflo	$2
	addiu	$4,$4,%lo(udp_cache_list_free)
	lw	$6,0($6)
	addiu	$5,$18,48
	addu	$2,$6,$2
	lw	$6,8($2)
	addiu	$6,$6,-1
	sw	$6,8($2)
	sw	$0,48($18)
	lw	$2,4($4)
	sw	$2,52($18)
	lw	$2,4($4)
	sw	$18,0($2)
	lw	$2,8($4)
	sw	$5,4($4)
	addiu	$2,$2,1
	sw	$2,8($4)
	move	$4,$20
$L245:
	sw	$3,40($sp)
	move	$5,$16
	.set	noreorder
	.set	nomacro
	jal	fast_path_before_nat_check
	move	$6,$22
	.set	macro
	.set	reorder

	move	$4,$2
	li	$2,1			# 0x1
	lw	$3,40($sp)
	beq	$4,$2,$L114
	lbu	$2,1($17)
	sltu	$4,$2,13
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L149
	lui	$4,%hi($L155)
	.set	macro
	.set	reorder

	sll	$2,$2,2
	addiu	$4,$4,%lo($L155)
	addu	$2,$4,$2
	lw	$2,0($2)
	lhu	$5,10($16)
	j	$2
	.rdata
	.align	2
	.align	2
$L155:
	.word	$L150
	.word	$L151
	.word	$L152
	.word	$L152
	.word	$L153
	.word	$L149
	.word	$L149
	.word	$L149
	.word	$L154
	.word	$L149
	.word	$L149
	.word	$L149
	.word	$L154
	.section	.text.enter_fast_path
$L151:
	lw	$2,24($17)
	srl	$4,$21,16
	andi	$21,$21,0xffff
	srl	$6,$2,16
	subu	$6,$0,$6
	andi	$2,$2,0xffff
	subu	$2,$6,$2
	addu	$2,$2,$4
	addu	$21,$2,$21
	addu	$5,$21,$5
	.set	noreorder
	.set	nomacro
	bgez	$5,$L156
	andi	$4,$5,0xffff
	.set	macro
	.set	reorder

	subu	$5,$0,$5
	andi	$2,$5,0xffff
	sra	$5,$5,16
	addu	$5,$2,$5
	sra	$2,$5,16
	addu	$5,$2,$5
	.set	noreorder
	.set	nomacro
	j	$L210
	nor	$5,$0,$5
	.set	macro
	.set	reorder

$L156:
	sra	$5,$5,16
	addu	$5,$4,$5
	sra	$2,$5,16
	addu	$5,$2,$5
$L210:
	sh	$5,10($16)
	lw	$2,24($sp)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L165
	li	$2,17			# 0x11
	.set	macro
	.set	reorder

	bne	$22,$2,$L159
	beq	$3,$0,$L165
	lhu	$4,0($3)
	beq	$4,$0,$L165
	lw	$6,24($17)
	lw	$5,12($16)
	srl	$2,$6,16
	subu	$2,$0,$2
	andi	$6,$6,0xffff
	srl	$7,$5,16
	subu	$2,$2,$6
	addu	$2,$2,$7
	andi	$5,$5,0xffff
	addu	$2,$2,$5
	addu	$2,$4,$2
	.set	noreorder
	.set	nomacro
	bgez	$2,$L160
	andi	$4,$2,0xffff
	.set	macro
	.set	reorder

	subu	$2,$0,$2
	andi	$4,$2,0xffff
	sra	$2,$2,16
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$4,$2
	.set	noreorder
	.set	nomacro
	j	$L212
	nor	$2,$0,$2
	.set	macro
	.set	reorder

$L160:
	sra	$2,$2,16
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$4,$2
$L212:
	.set	noreorder
	.set	nomacro
	j	$L165
	sh	$2,0($3)
	.set	macro
	.set	reorder

$L159:
	lw	$5,24($17)
	lw	$4,12($16)
	srl	$6,$5,16
	lhu	$2,0($3)
	subu	$6,$0,$6
	andi	$5,$5,0xffff
	subu	$5,$6,$5
	srl	$6,$4,16
	addu	$5,$5,$6
	andi	$4,$4,0xffff
	addu	$4,$5,$4
	addu	$2,$2,$4
	.set	noreorder
	.set	nomacro
	bgez	$2,$L161
	andi	$4,$2,0xffff
	.set	macro
	.set	reorder

	subu	$2,$0,$2
	andi	$4,$2,0xffff
	sra	$2,$2,16
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$4,$2
	.set	noreorder
	.set	nomacro
	j	$L211
	nor	$2,$0,$2
	.set	macro
	.set	reorder

$L161:
	sra	$2,$2,16
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$4,$2
$L211:
	sh	$2,0($3)
	li	$2,1048576			# 0x100000
	lw	$3,12($19)
	and	$2,$3,$2
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L165
	li	$2,40			# 0x28
	.set	macro
	.set	reorder

	lhu	$3,2($16)
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L165
	move	$4,$20
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	skb_trim
	li	$5,40			# 0x28
	.set	macro
	.set	reorder

	j	$L165
$L152:
	lw	$2,24($17)
	srl	$4,$21,16
	andi	$21,$21,0xffff
	srl	$6,$2,16
	subu	$6,$0,$6
	andi	$2,$2,0xffff
	subu	$2,$6,$2
	addu	$2,$2,$4
	addu	$21,$2,$21
	addu	$5,$21,$5
	.set	noreorder
	.set	nomacro
	bgez	$5,$L163
	andi	$4,$5,0xffff
	.set	macro
	.set	reorder

	subu	$5,$0,$5
	andi	$2,$5,0xffff
	sra	$5,$5,16
	addu	$5,$2,$5
	sra	$2,$5,16
	addu	$5,$2,$5
	.set	noreorder
	.set	nomacro
	j	$L213
	nor	$5,$0,$5
	.set	macro
	.set	reorder

$L163:
	sra	$5,$5,16
	addu	$5,$4,$5
	sra	$2,$5,16
	addu	$5,$2,$5
$L213:
	sh	$5,10($16)
	lw	$4,24($sp)
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L165
	li	$2,17			# 0x11
	.set	macro
	.set	reorder

	bne	$22,$2,$L166
	beq	$3,$0,$L167
	lhu	$4,0($3)
	beq	$4,$0,$L167
	lw	$6,24($17)
	lw	$5,12($16)
	srl	$2,$6,16
	lhu	$8,0($19)
	subu	$2,$0,$2
	andi	$6,$6,0xffff
	srl	$7,$5,16
	subu	$2,$2,$6
	addu	$2,$2,$7
	andi	$5,$5,0xffff
	addu	$2,$2,$5
	lw	$7,28($17)
	addu	$2,$2,$8
	subu	$2,$2,$7
	addu	$2,$4,$2
	.set	noreorder
	.set	nomacro
	bgez	$2,$L168
	andi	$4,$2,0xffff
	.set	macro
	.set	reorder

	subu	$2,$0,$2
	andi	$4,$2,0xffff
	sra	$2,$2,16
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$4,$2
	.set	noreorder
	.set	nomacro
	j	$L215
	nor	$2,$0,$2
	.set	macro
	.set	reorder

$L168:
	sra	$2,$2,16
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$4,$2
$L215:
	.set	noreorder
	.set	nomacro
	j	$L167
	sh	$2,0($3)
	.set	macro
	.set	reorder

$L166:
	lw	$5,24($17)
	lw	$4,12($16)
	srl	$2,$5,16
	lhu	$6,0($19)
	subu	$2,$0,$2
	andi	$5,$5,0xffff
	srl	$7,$4,16
	subu	$2,$2,$5
	addu	$2,$2,$7
	andi	$4,$4,0xffff
	addu	$2,$2,$4
	lw	$7,28($17)
	addu	$2,$2,$6
	lhu	$6,0($3)
	subu	$2,$2,$7
	addu	$2,$6,$2
	.set	noreorder
	.set	nomacro
	bgez	$2,$L169
	andi	$4,$2,0xffff
	.set	macro
	.set	reorder

	subu	$2,$0,$2
	andi	$4,$2,0xffff
	sra	$2,$2,16
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$4,$2
	.set	noreorder
	.set	nomacro
	j	$L214
	nor	$2,$0,$2
	.set	macro
	.set	reorder

$L169:
	sra	$2,$2,16
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$4,$2
$L214:
	sh	$2,0($3)
	li	$2,1048576			# 0x100000
	lw	$3,12($19)
	and	$2,$3,$2
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L167
	li	$2,40			# 0x28
	.set	macro
	.set	reorder

	lhu	$3,2($16)
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L167
	move	$4,$20
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	skb_trim
	li	$5,40			# 0x28
	.set	macro
	.set	reorder

$L167:
	lw	$2,28($17)
	sh	$2,0($19)
$L165:
	lw	$2,24($17)
	.set	noreorder
	.set	nomacro
	j	$L150
	sw	$2,12($16)
	.set	macro
	.set	reorder

$L153:
	lw	$2,32($17)
	andi	$23,$23,0xffff
	lw	$9,28($sp)
	andi	$4,$2,0xffff
	addu	$21,$23,$9
	srl	$2,$2,16
	subu	$4,$21,$4
	subu	$2,$4,$2
	addu	$5,$2,$5
	.set	noreorder
	.set	nomacro
	bgez	$5,$L171
	andi	$4,$5,0xffff
	.set	macro
	.set	reorder

	subu	$5,$0,$5
	andi	$2,$5,0xffff
	sra	$5,$5,16
	addu	$5,$2,$5
	sra	$2,$5,16
	addu	$5,$2,$5
	.set	noreorder
	.set	nomacro
	j	$L216
	nor	$5,$0,$5
	.set	macro
	.set	reorder

$L171:
	sra	$5,$5,16
	addu	$5,$4,$5
	sra	$2,$5,16
	addu	$5,$2,$5
$L216:
	sh	$5,10($16)
	lw	$11,24($sp)
	.set	noreorder
	.set	nomacro
	beq	$11,$0,$L225
	li	$2,17			# 0x11
	.set	macro
	.set	reorder

	bne	$22,$2,$L174
	beq	$3,$0,$L225
	lhu	$2,0($3)
	beq	$2,$0,$L225
	lw	$4,32($17)
	andi	$23,$4,0xffff
	subu	$23,$21,$23
	srl	$4,$4,16
	subu	$23,$23,$4
	.set	noreorder
	.set	nomacro
	j	$L231
	addu	$23,$2,$23
	.set	macro
	.set	reorder

$L174:
	lw	$2,32($17)
	lhu	$23,0($3)
	andi	$4,$2,0xffff
	subu	$21,$21,$4
	srl	$2,$2,16
	subu	$21,$21,$2
	addu	$23,$23,$21
$L231:
	.set	noreorder
	.set	nomacro
	bgez	$23,$L176
	andi	$2,$23,0xffff
	.set	macro
	.set	reorder

	subu	$23,$0,$23
	andi	$2,$23,0xffff
	sra	$23,$23,16
	addu	$23,$2,$23
	sra	$2,$23,16
	addu	$23,$2,$23
	.set	noreorder
	.set	nomacro
	j	$L217
	nor	$23,$0,$23
	.set	macro
	.set	reorder

$L176:
	sra	$23,$23,16
	addu	$23,$2,$23
	sra	$2,$23,16
	addu	$23,$2,$23
$L217:
	.set	noreorder
	.set	nomacro
	j	$L225
	sh	$23,0($3)
	.set	macro
	.set	reorder

$L154:
	lw	$2,32($17)
	andi	$23,$23,0xffff
	lw	$12,28($sp)
	andi	$4,$2,0xffff
	addu	$23,$23,$12
	srl	$2,$2,16
	subu	$4,$23,$4
	subu	$4,$4,$2
	addu	$4,$4,$5
	.set	noreorder
	.set	nomacro
	bgez	$4,$L177
	andi	$2,$4,0xffff
	.set	macro
	.set	reorder

	subu	$4,$0,$4
	andi	$2,$4,0xffff
	sra	$4,$4,16
	addu	$4,$2,$4
	sra	$2,$4,16
	addu	$4,$2,$4
	.set	noreorder
	.set	nomacro
	j	$L218
	nor	$4,$0,$4
	.set	macro
	.set	reorder

$L177:
	sra	$4,$4,16
	addu	$4,$2,$4
	sra	$2,$4,16
	addu	$4,$2,$4
$L218:
	sh	$4,10($16)
	lw	$2,24($sp)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L225
	li	$2,17			# 0x11
	.set	macro
	.set	reorder

	bne	$22,$2,$L180
	beq	$3,$0,$L181
	lhu	$4,0($3)
	beq	$4,$0,$L181
	lw	$5,32($17)
	lhu	$7,2($19)
	andi	$2,$5,0xffff
	lw	$6,36($17)
	subu	$2,$23,$2
	srl	$5,$5,16
	subu	$2,$2,$5
	addu	$2,$2,$7
	subu	$2,$2,$6
	.set	noreorder
	.set	nomacro
	j	$L233
	addu	$2,$4,$2
	.set	macro
	.set	reorder

$L180:
	lw	$4,32($17)
	lhu	$5,2($19)
	andi	$2,$4,0xffff
	lw	$6,36($17)
	subu	$2,$23,$2
	srl	$4,$4,16
	subu	$2,$2,$4
	addu	$2,$2,$5
	lhu	$5,0($3)
	subu	$2,$2,$6
	addu	$2,$5,$2
$L233:
	.set	noreorder
	.set	nomacro
	bgez	$2,$L183
	andi	$4,$2,0xffff
	.set	macro
	.set	reorder

	subu	$2,$0,$2
	andi	$4,$2,0xffff
	sra	$2,$2,16
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$4,$2
	.set	noreorder
	.set	nomacro
	j	$L219
	nor	$2,$0,$2
	.set	macro
	.set	reorder

$L183:
	sra	$2,$2,16
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$4,$2
$L219:
	sh	$2,0($3)
$L181:
	lw	$2,36($17)
	.set	noreorder
	.set	nomacro
	j	$L225
	sh	$2,2($19)
	.set	macro
	.set	reorder

$L149:
	lw	$4,24($17)
	srl	$2,$21,16
	andi	$21,$21,0xffff
	lhu	$5,10($16)
	addu	$21,$21,$2
	andi	$2,$4,0xffff
	subu	$2,$21,$2
	srl	$4,$4,16
	subu	$2,$2,$4
	addu	$2,$2,$5
	.set	noreorder
	.set	nomacro
	bgez	$2,$L184
	andi	$4,$2,0xffff
	.set	macro
	.set	reorder

	subu	$2,$0,$2
	andi	$4,$2,0xffff
	sra	$2,$2,16
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$4,$2
	.set	noreorder
	.set	nomacro
	j	$L220
	nor	$2,$0,$2
	.set	macro
	.set	reorder

$L184:
	sra	$2,$2,16
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$4,$2
$L220:
	sh	$2,10($16)
	andi	$23,$23,0xffff
	lw	$2,32($17)
	lw	$4,28($sp)
	andi	$5,$2,0xffff
	addu	$23,$23,$4
	srl	$2,$2,16
	lhu	$4,10($16)
	subu	$5,$23,$5
	subu	$5,$5,$2
	addu	$5,$5,$4
	.set	noreorder
	.set	nomacro
	bgez	$5,$L186
	andi	$2,$5,0xffff
	.set	macro
	.set	reorder

	subu	$5,$0,$5
	andi	$2,$5,0xffff
	sra	$5,$5,16
	addu	$5,$2,$5
	sra	$2,$5,16
	addu	$5,$2,$5
	.set	noreorder
	.set	nomacro
	j	$L221
	nor	$5,$0,$5
	.set	macro
	.set	reorder

$L186:
	sra	$5,$5,16
	addu	$5,$2,$5
	sra	$2,$5,16
	addu	$5,$2,$5
$L221:
	sh	$5,10($16)
	lw	$9,24($sp)
	.set	noreorder
	.set	nomacro
	beq	$9,$0,$L188
	li	$2,17			# 0x11
	.set	macro
	.set	reorder

	bne	$22,$2,$L189
	beq	$3,$0,$L190
	lhu	$2,0($3)
	beq	$2,$0,$L190
	lw	$5,24($17)
	lhu	$7,0($19)
	andi	$4,$5,0xffff
	lw	$6,28($17)
	subu	$4,$21,$4
	srl	$5,$5,16
	subu	$4,$4,$5
	addu	$4,$4,$7
	subu	$4,$4,$6
	.set	noreorder
	.set	nomacro
	j	$L237
	addu	$4,$2,$4
	.set	macro
	.set	reorder

$L189:
	lw	$2,24($17)
	lhu	$5,0($19)
	andi	$4,$2,0xffff
	lw	$6,28($17)
	subu	$4,$21,$4
	srl	$2,$2,16
	subu	$4,$4,$2
	addu	$4,$4,$5
	lhu	$5,0($3)
	subu	$4,$4,$6
	addu	$4,$5,$4
$L237:
	.set	noreorder
	.set	nomacro
	bgez	$4,$L194
	andi	$2,$4,0xffff
	.set	macro
	.set	reorder

	subu	$4,$0,$4
	andi	$2,$4,0xffff
	sra	$4,$4,16
	addu	$4,$2,$4
	sra	$2,$4,16
	addu	$4,$2,$4
	.set	noreorder
	.set	nomacro
	j	$L223
	nor	$4,$0,$4
	.set	macro
	.set	reorder

$L194:
	sra	$4,$4,16
	addu	$4,$2,$4
	sra	$2,$4,16
	addu	$4,$2,$4
$L223:
	sh	$4,0($3)
	lw	$4,32($17)
	lhu	$5,2($19)
	andi	$2,$4,0xffff
	lw	$6,36($17)
	subu	$2,$23,$2
	srl	$4,$4,16
	subu	$2,$2,$4
	addu	$2,$2,$5
	lhu	$5,0($3)
	subu	$2,$2,$6
	addu	$2,$5,$2
	.set	noreorder
	.set	nomacro
	bgez	$2,$L196
	andi	$4,$2,0xffff
	.set	macro
	.set	reorder

	subu	$2,$0,$2
	andi	$4,$2,0xffff
	sra	$2,$2,16
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$4,$2
	.set	noreorder
	.set	nomacro
	j	$L224
	nor	$2,$0,$2
	.set	macro
	.set	reorder

$L196:
	sra	$2,$2,16
	addu	$2,$4,$2
	sra	$4,$2,16
	addu	$2,$4,$2
$L224:
	sh	$2,0($3)
$L190:
	lw	$2,28($17)
	sh	$2,0($19)
	lw	$2,36($17)
	sh	$2,2($19)
$L188:
	lw	$2,24($17)
	sw	$2,12($16)
$L225:
	lw	$2,32($17)
	sw	$2,16($16)
$L150:
	lbu	$2,8($16)
	lbu	$3,9($16)
	addiu	$4,$2,-1
	sll	$2,$2,8
	andi	$4,$4,0x00ff
	lhu	$5,10($16)
	or	$2,$2,$3
	sb	$4,8($16)
	sll	$4,$4,8
	addu	$2,$2,$5
	or	$3,$4,$3
	subu	$2,$2,$3
	.set	noreorder
	.set	nomacro
	bgez	$2,$L197
	andi	$3,$2,0xffff
	.set	macro
	.set	reorder

	subu	$2,$0,$2
	andi	$3,$2,0xffff
	sra	$2,$2,16
	addu	$2,$3,$2
	sra	$3,$2,16
	addu	$2,$3,$2
	.set	noreorder
	.set	nomacro
	j	$L226
	nor	$2,$0,$2
	.set	macro
	.set	reorder

$L197:
	sra	$2,$2,16
	addu	$2,$3,$2
	sra	$3,$2,16
	addu	$2,$3,$2
$L226:
	sh	$2,10($16)
	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_ip_summed
	move	$4,$20
	.set	macro
	.set	reorder

	move	$4,$20
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_dev
	move	$5,$0
	.set	macro
	.set	reorder

	lw	$5,48($17)
	lw	$6,52($17)
	.set	noreorder
	.set	nomacro
	jal	fastpath_set_qos_mark
	move	$4,$20
	.set	macro
	.set	reorder

	lui	$2,%hi(jiffies)
	move	$4,$16
	lw	$2,%lo(jiffies)($2)
	move	$5,$19
	move	$6,$20
	.set	noreorder
	.set	nomacro
	jal	fast_path_post_process_xmit_check
	sw	$2,40($17)
	.set	macro
	.set	reorder

	li	$3,1			# 0x1
	.set	noreorder
	.set	nomacro
	beq	$2,$3,$L114
	li	$4,1			# 0x1
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	ip_finish_output3
	move	$4,$20
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L114
	li	$4,1			# 0x1
	.set	macro
	.set	reorder

$L135:
	lw	$17,64($17)
$L134:
	bne	$17,$0,$L200
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_inDev
	move	$4,$20
	.set	macro
	.set	reorder

	move	$4,$16
	move	$5,$19
	lw	$31,84($sp)
	move	$6,$20
	lw	$fp,80($sp)
	lw	$23,76($sp)
	lw	$22,72($sp)
	lw	$21,68($sp)
	lw	$20,64($sp)
	lw	$19,60($sp)
	lw	$18,56($sp)
	lw	$17,52($sp)
	lw	$16,48($sp)
	.set	noreorder
	.set	nomacro
	j	fast_path_post_process_return_check
	addiu	$sp,$sp,88
	.set	macro
	.set	reorder

$L112:
	move	$4,$0
$L114:
	lw	$31,84($sp)
	move	$2,$4
	lw	$fp,80($sp)
	lw	$23,76($sp)
	lw	$22,72($sp)
	lw	$21,68($sp)
	lw	$20,64($sp)
	lw	$19,60($sp)
	lw	$18,56($sp)
	lw	$17,52($sp)
	lw	$16,48($sp)
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,88
	.set	macro
	.set	reorder

$L116:
	lw	$2,12($19)
	li	$3,65536			# 0x10000
	and	$3,$2,$3
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L208
	li	$3,393216			# 0x60000
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L243
	li	$2,6			# 0x6
	.set	macro
	.set	reorder

$L241:
	mult	$7,$9
	lui	$9,%hi(udp_cache_table)
	lw	$9,%lo(udp_cache_table)($9)
	lw	$9,0($9)
	mflo	$7
	addu	$7,$9,$7
	lw	$18,0($7)
	.set	noreorder
	.set	nomacro
	j	$L203
	li	$10,1			# 0x1
	.set	macro
	.set	reorder

$L242:
	.set	noreorder
	.set	nomacro
	beq	$fp,$0,$L204
	li	$fp,1			# 0x1
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L131
	sw	$fp,24($sp)
	.set	macro
	.set	reorder

	.end	enter_fast_path
	.size	enter_fast_path, .-enter_fast_path
	.section	.text.rtl_fp_mark_invalid,"ax",@progbits
	.align	2
	.globl	rtl_fp_mark_invalid
	.set	nomips16
	.ent	rtl_fp_mark_invalid
	.type	rtl_fp_mark_invalid, @function
rtl_fp_mark_invalid:
	.frame	$sp,72,$31		# vars= 8, regs= 10/0, args= 24, gp= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-72
	sw	$31,68($sp)
	sw	$fp,64($sp)
	sw	$23,60($sp)
	sw	$22,56($sp)
	sw	$21,52($sp)
	sw	$20,48($sp)
	sw	$19,44($sp)
	sw	$18,40($sp)
	sw	$17,36($sp)
	sw	$16,32($sp)
	li	$5,1			# 0x1
	move	$20,$4
	jal	rtl_get_ct_ip_by_dir
	move	$6,$0

	move	$4,$20
	li	$5,1			# 0x1
	li	$6,1			# 0x1
	jal	rtl_get_ct_ip_by_dir
	move	$23,$2

	move	$4,$20
	move	$5,$0
	move	$6,$0
	jal	rtl_get_ct_ip_by_dir
	move	$21,$2

	move	$4,$20
	move	$5,$0
	li	$6,1			# 0x1
	jal	rtl_get_ct_ip_by_dir
	move	$19,$2

	move	$4,$20
	li	$5,1			# 0x1
	move	$6,$0
	jal	rtl_get_ct_port_by_dir
	move	$17,$2

	move	$4,$20
	li	$5,1			# 0x1
	li	$6,1			# 0x1
	jal	rtl_get_ct_port_by_dir
	move	$22,$2

	move	$4,$20
	move	$5,$0
	move	$6,$0
	jal	rtl_get_ct_port_by_dir
	sw	$2,24($sp)

	li	$6,1			# 0x1
	move	$4,$20
	move	$5,$0
	jal	rtl_get_ct_port_by_dir
	move	$18,$2

	move	$4,$20
	li	$5,1			# 0x1
	jal	rtl_get_ct_protonum
	move	$16,$2

	move	$5,$0
	move	$4,$20
	jal	rtl_get_ct_protonum
	move	$fp,$2

	lw	$7,24($sp)
	move	$4,$23
	move	$5,$22
	sw	$fp,16($sp)
	move	$6,$21
	jal	mark_path_invalid
	move	$20,$2

	move	$4,$19
	sw	$20,16($sp)
	move	$5,$18
	move	$6,$17
	jal	mark_path_invalid
	move	$7,$16

	lw	$31,68($sp)
	lw	$fp,64($sp)
	lw	$23,60($sp)
	lw	$22,56($sp)
	lw	$21,52($sp)
	lw	$20,48($sp)
	lw	$19,44($sp)
	lw	$18,40($sp)
	lw	$17,36($sp)
	lw	$16,32($sp)
	j	$31
	addiu	$sp,$sp,72

	.set	macro
	.set	reorder
	.end	rtl_fp_mark_invalid
	.size	rtl_fp_mark_invalid, .-rtl_fp_mark_invalid
	.section	.text.rtl_br_fdb_time_update,"ax",@progbits
	.align	2
	.globl	rtl_br_fdb_time_update
	.set	nomips16
	.ent	rtl_br_fdb_time_update
	.type	rtl_br_fdb_time_update, @function
rtl_br_fdb_time_update:
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
	move	$17,$4
	move	$4,$0
	move	$16,$5
	move	$5,$6
	jal	rtl_get_hw_fdb_age
	li	$6,2			# 0x2

	li	$3,300			# 0x12c
	beq	$2,$3,$L251
	li	$3,450			# 0x1c2

	beq	$2,$3,$L252
	li	$3,150			# 0x96

	bne	$2,$3,$L259
	move	$4,$16

	j	$L260
	lui	$2,%hi(jiffies)

$L252:
	lui	$2,%hi(jiffies)
	move	$4,$16
	lw	$5,%lo(jiffies)($2)
	j	$L256
	nop

$L251:
	lui	$2,%hi(jiffies)
	move	$4,$16
	lw	$5,%lo(jiffies)($2)
	j	$L256
	addiu	$5,$5,-15000

$L260:
	lw	$5,%lo(jiffies)($2)
	addiu	$5,$5,-30000
$L256:
	jal	rtl_set_fdb_aging
	nop

	move	$4,$16
$L259:
	lui	$2,%hi(jiffies)
	lw	$16,%lo(jiffies)($2)
	jal	rtl_get_fdb_aging
	nop

	move	$4,$17
	jal	rtl_hold_time
	move	$18,$2

	li	$3,-1			# 0xffffffff
	addu	$2,$2,$18
	subu	$2,$16,$2
	slt	$2,$2,0
	lw	$31,28($sp)
	movn	$3,$0,$2
	lw	$18,24($sp)
	move	$2,$3
	lw	$17,20($sp)
	lw	$16,16($sp)
	j	$31
	addiu	$sp,$sp,32

	.set	macro
	.set	reorder
	.end	rtl_br_fdb_time_update
	.size	rtl_br_fdb_time_update, .-rtl_br_fdb_time_update
	.section	.text.rtk_idleNaptConnection,"ax",@progbits
	.align	2
	.globl	rtk_idleNaptConnection
	.set	nomips16
	.ent	rtk_idleNaptConnection
	.type	rtk_idleNaptConnection, @function
rtk_idleNaptConnection:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	li	$15,6			# 0x6
	lw	$3,4($4)
	li	$2,17			# 0x11
	lw	$6,20($4)
	li	$8,-16777216			# 0xff000000
	lw	$7,24($4)
	srl	$11,$3,8
	xor	$14,$6,$3
	lw	$10,0($4)
	xor	$14,$14,$7
	lw	$9,8($4)
	xor	$14,$14,$11
	lui	$11,%hi(path_table_list_max)
	lw	$13,%lo(path_table_list_max)($11)
	lui	$11,%hi(table_path)
	srl	$12,$6,16
	lw	$11,%lo(table_path)($11)
	xor	$14,$14,$12
	xori	$24,$10,0x2
	addiu	$13,$13,-1
	lw	$12,0($11)
	srl	$11,$9,4
	movz	$2,$15,$24
	xor	$14,$14,$11
	lui	$11,%hi(jiffies)
	xor	$14,$14,$2
	lw	$11,%lo(jiffies)($11)
	srl	$15,$14,12
	xor	$14,$15,$14
	li	$15,12			# 0xc
	and	$14,$14,$13
	mult	$14,$15
	ori	$15,$8,0x100
	mflo	$14
	addu	$14,$12,$14
	lw	$14,0($14)
	j	$L264
	ori	$8,$8,0xff00

$L268:
	lw	$24,4($14)
	bne	$24,$10,$L265
	nop

	lw	$24,0($14)
	and	$24,$24,$8
	bne	$24,$15,$L265
	nop

	lw	$24,16($14)
	bne	$24,$3,$L265
	nop

	lw	$24,12($14)
	bne	$24,$9,$L265
	nop

	lw	$25,24($14)
	lw	$24,12($4)
	bne	$25,$24,$L265
	nop

	lw	$25,28($14)
	lw	$24,16($4)
	bne	$25,$24,$L265
	nop

	lw	$24,20($14)
	bne	$24,$6,$L265
	nop

	lw	$24,8($14)
	bne	$24,$7,$L265
	nop

	lw	$8,40($14)
	subu	$15,$11,$5
	subu	$15,$15,$8
	bgez	$15,$L266
	nop

	lw	$4,56($14)
	j	$L274
	addu	$5,$5,$8

$L265:
	lw	$14,64($14)
$L264:
	bne	$14,$0,$L268
	nop

$L266:
	lw	$8,12($4)
	srl	$25,$6,8
	lw	$14,16($4)
	srl	$24,$7,4
	li	$4,-16777216			# 0xff000000
	xor	$15,$8,$6
	xor	$15,$15,$14
	xor	$15,$15,$25
	srl	$25,$8,16
	xor	$15,$15,$25
	xor	$15,$15,$24
	xor	$2,$15,$2
	srl	$15,$2,12
	xor	$2,$15,$2
	li	$15,12			# 0xc
	and	$13,$2,$13
	mult	$13,$15
	ori	$2,$4,0x200
	mflo	$13
	addu	$12,$12,$13
	lw	$12,0($12)
	j	$L269
	ori	$4,$4,0xff00

$L272:
	lw	$13,4($12)
	bne	$13,$10,$L270
	nop

	lw	$13,0($12)
	and	$13,$13,$4
	bne	$13,$2,$L270
	nop

	lw	$13,20($12)
	bne	$13,$8,$L270
	nop

	lw	$13,8($12)
	bne	$13,$14,$L270
	nop

	lw	$13,24($12)
	bne	$13,$6,$L270
	nop

	lw	$13,28($12)
	bne	$13,$7,$L270
	nop

	lw	$13,32($12)
	bne	$13,$3,$L270
	nop

	lw	$13,36($12)
	bne	$13,$9,$L270
	nop

	lw	$2,40($12)
	subu	$11,$11,$5
	subu	$11,$11,$2
	bgez	$11,$L271
	nop

	lw	$4,56($12)
	addu	$5,$5,$2
$L274:
	jal	rtl_set_ct_timeout_expires
	nop

	j	$L267
	li	$2,-1			# 0xffffffff

$L270:
	lw	$12,64($12)
$L269:
	bne	$12,$0,$L272
	nop

$L271:
	move	$2,$0
$L267:
	lw	$31,20($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	rtk_idleNaptConnection
	.size	rtk_idleNaptConnection, .-rtk_idleNaptConnection
	.section	.text.rtl_fpTimer_update,"ax",@progbits
	.align	2
	.globl	rtl_fpTimer_update
	.set	nomips16
	.ent	rtl_fpTimer_update
	.type	rtl_fpTimer_update, @function
rtl_fpTimer_update:
	.frame	$sp,72,$31		# vars= 32, regs= 6/0, args= 16, gp= 0
	.mask	0x801f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-72
	sw	$31,68($sp)
	sw	$20,64($sp)
	sw	$19,60($sp)
	sw	$18,56($sp)
	sw	$17,52($sp)
	sw	$16,48($sp)
	lui	$2,%hi(jiffies)
	move	$16,$4
	move	$5,$0
	lw	$19,%lo(jiffies)($2)
	jal	rtl_get_ct_protonum
	nop

	li	$3,17			# 0x11
	bne	$2,$3,$L276
	move	$4,$16

	jal	rtl_get_ct_udp_status
	move	$4,$16

	andi	$2,$2,0x2
	beq	$2,$0,$L277
	lui	$2,%hi(nf_ct_udp_timeout)

	lui	$2,%hi(nf_ct_udp_timeout_stream)
	lw	$17,%lo(nf_ct_udp_timeout_stream)($2)
	j	$L278
	li	$18,1			# 0x1

$L277:
	lw	$17,%lo(nf_ct_udp_timeout)($2)
	j	$L278
	li	$18,1			# 0x1

$L276:
	jal	rtl_get_ct_protonum
	move	$5,$0

	li	$3,6			# 0x6
	bne	$2,$3,$L282
	li	$2,-1			# 0xffffffff

	jal	rtl_get_ct_tcp_state
	move	$4,$16

	sltu	$2,$2,6
	beq	$2,$0,$L282
	li	$2,-1			# 0xffffffff

	move	$4,$16
	jal	rtl_tcp_get_timeouts
	li	$18,2			# 0x2

	move	$17,$2
$L278:
	move	$5,$0
	move	$6,$0
	jal	rtl_get_ct_ip_by_dir
	move	$4,$16

	move	$4,$16
	li	$5,1			# 0x1
	li	$6,1			# 0x1
	jal	rtl_get_ct_ip_by_dir
	move	$20,$2

	bne	$20,$2,$L280
	li	$6,1			# 0x1

	move	$4,$16
	li	$5,1			# 0x1
	move	$6,$0
	jal	rtl_get_ct_ip_by_dir
	sw	$18,16($sp)

	move	$4,$16
	li	$5,1			# 0x1
	move	$6,$0
	jal	rtl_get_ct_port_by_dir
	sw	$2,20($sp)

	move	$4,$16
	move	$5,$0
	li	$6,1			# 0x1
	jal	rtl_get_ct_ip_by_dir
	sw	$2,24($sp)

	move	$4,$16
	move	$5,$0
	li	$6,1			# 0x1
	jal	rtl_get_ct_port_by_dir
	sw	$2,28($sp)

	move	$4,$16
	move	$5,$0
	move	$6,$0
	jal	rtl_get_ct_ip_by_dir
	sw	$2,32($sp)

	move	$4,$16
	j	$L284
	move	$5,$0

$L280:
	move	$5,$0
	jal	rtl_get_ct_ip_by_dir
	move	$4,$16

	move	$4,$16
	li	$5,1			# 0x1
	move	$6,$0
	jal	rtl_get_ct_ip_by_dir
	move	$20,$2

	bne	$20,$2,$L282
	li	$2,-1			# 0xffffffff

	move	$4,$16
	move	$5,$0
	move	$6,$0
	jal	rtl_get_ct_ip_by_dir
	sw	$18,16($sp)

	move	$4,$16
	move	$5,$0
	move	$6,$0
	jal	rtl_get_ct_port_by_dir
	sw	$2,20($sp)

	move	$4,$16
	li	$5,1			# 0x1
	li	$6,1			# 0x1
	jal	rtl_get_ct_ip_by_dir
	sw	$2,24($sp)

	move	$4,$16
	li	$5,1			# 0x1
	li	$6,1			# 0x1
	jal	rtl_get_ct_port_by_dir
	sw	$2,28($sp)

	move	$4,$16
	li	$5,1			# 0x1
	move	$6,$0
	jal	rtl_get_ct_ip_by_dir
	sw	$2,32($sp)

	move	$4,$16
	li	$5,1			# 0x1
$L284:
	move	$6,$0
	jal	rtl_get_ct_port_by_dir
	sw	$2,36($sp)

	addiu	$4,$sp,16
	move	$5,$17
	jal	rtk_idleNaptConnection
	sw	$2,40($sp)

	beq	$2,$0,$L279
	addu	$5,$17,$19

	jal	rtl_check_for_acc
	move	$4,$16

	j	$L282
	move	$2,$0

$L279:
	li	$2,-1			# 0xffffffff
$L282:
	lw	$31,68($sp)
	lw	$20,64($sp)
	lw	$19,60($sp)
	lw	$18,56($sp)
	lw	$17,52($sp)
	lw	$16,48($sp)
	j	$31
	addiu	$sp,$sp,72

	.set	macro
	.set	reorder
	.end	rtl_fpTimer_update
	.size	rtl_fpTimer_update, .-rtl_fpTimer_update
	.section	.text.rtk_delNaptConnection,"ax",@progbits
	.align	2
	.globl	rtk_delNaptConnection
	.set	nomips16
	.ent	rtk_delNaptConnection
	.type	rtk_delNaptConnection, @function
rtk_delNaptConnection:
	.frame	$sp,32,$31		# vars= 0, regs= 4/0, args= 16, gp= 0
	.mask	0x80070000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$18,24($sp)
	sw	$17,20($sp)
	sw	$16,16($sp)
	li	$2,6			# 0x6
	move	$16,$4
	lw	$3,0($4)
	li	$18,17			# 0x11
	xori	$3,$3,0x2
	movz	$18,$2,$3
#APP
 # 1244 "net/rtl/fastpath/fastpath_core.c" 1
	raw_local_irq_save	$17
 # 0 "" 2
#NO_APP
	lw	$4,20($4)
	jal	filter_delconnect
	lui	$9,%hi(napt_table_list_max)
	lw	$3,4($16)
	lw	$6,8($16)
	lw	$2,20($16)
	srl	$7,$3,16
	lw	$5,24($16)
	xor	$8,$6,$3
	xor	$8,$8,$2
	lw	$4,0($16)
	xor	$8,$8,$5
	lw	$10,%lo(napt_table_list_max)($9)
	xor	$8,$8,$7
	lui	$7,%hi(table_napt)
	srl	$11,$2,16
	addiu	$10,$10,-1
	andi	$9,$4,0xffff
	lw	$7,%lo(table_napt)($7)
	xor	$8,$8,$11
	xor	$8,$8,$9
	lw	$7,0($7)
	srl	$9,$8,12
	xor	$9,$9,$8
	and	$9,$9,$10
	li	$10,12			# 0xc
	mult	$9,$10
	mflo	$9
	addu	$7,$7,$9
	lw	$8,0($7)
	j	$L288
$L312:
	lw	$7,4($8)
	bne	$7,$4,$L289
	lw	$7,8($8)
	bne	$7,$3,$L289
	lw	$7,12($8)
	bne	$7,$6,$L289
	lw	$10,16($8)
	lw	$7,12($16)
	bne	$10,$7,$L289
	lw	$10,20($8)
	lw	$7,16($16)
	bne	$10,$7,$L289
	lw	$7,24($8)
	bne	$7,$2,$L289
	lw	$7,28($8)
	bne	$7,$5,$L289
	lw	$3,36($8)
	sb	$0,0($8)
	lw	$2,40($8)
	beq	$3,$0,$L290
	.set	noreorder
	.set	nomacro
	j	$L291
	sw	$2,40($3)
	.set	macro
	.set	reorder

$L290:
	lui	$3,%hi(table_napt)
	lw	$3,%lo(table_napt)($3)
	lw	$3,0($3)
	addu	$3,$3,$9
	sw	$2,4($3)
$L291:
	lw	$2,40($8)
	lw	$3,36($8)
	sw	$3,0($2)
	lui	$2,%hi(table_napt)
	lw	$2,%lo(table_napt)($2)
	lw	$2,0($2)
	addu	$9,$2,$9
	lw	$2,8($9)
	addiu	$2,$2,-1
	sw	$2,8($9)
	lw	$3,44($8)
	lw	$2,48($8)
	beq	$3,$0,$L292
	.set	noreorder
	.set	nomacro
	j	$L293
	sw	$2,48($3)
	.set	macro
	.set	reorder

$L292:
	lui	$3,%hi(napt_list_inuse+4)
	sw	$2,%lo(napt_list_inuse+4)($3)
$L293:
	lw	$2,48($8)
	lui	$3,%hi(napt_list_inuse)
	lw	$5,44($8)
	addiu	$3,$3,%lo(napt_list_inuse)
	addiu	$4,$8,44
	sw	$5,0($2)
	lui	$2,%hi(napt_list_free)
	lw	$5,8($3)
	addiu	$2,$2,%lo(napt_list_free)
	addiu	$5,$5,-1
	sw	$5,8($3)
	sw	$0,44($8)
	lw	$3,4($2)
	sw	$3,48($8)
	lw	$3,4($2)
	sw	$8,0($3)
	lui	$8,%hi(path_table_list_max)
	lw	$3,8($2)
	sw	$4,4($2)
	addiu	$3,$3,1
	sw	$3,8($2)
	lw	$3,4($16)
	lw	$2,20($16)
	lw	$5,24($16)
	srl	$6,$3,8
	xor	$9,$2,$3
	lw	$4,8($16)
	xor	$9,$9,$5
	lw	$10,%lo(path_table_list_max)($8)
	xor	$9,$9,$6
	lui	$6,%hi(table_path)
	srl	$11,$2,16
	addiu	$10,$10,-1
	srl	$8,$4,4
	lw	$6,%lo(table_path)($6)
	xor	$9,$9,$11
	xor	$9,$9,$8
	lw	$6,0($6)
	xor	$9,$9,$18
	srl	$8,$9,12
	xor	$8,$8,$9
	and	$8,$8,$10
	li	$10,12			# 0xc
	mult	$8,$10
	mflo	$8
	addu	$6,$6,$8
	lw	$6,0($6)
	.set	noreorder
	.set	nomacro
	j	$L294
	li	$7,1			# 0x1
	.set	macro
	.set	reorder

$L301:
	lw	$10,4($6)
	lw	$9,0($16)
	bne	$10,$9,$L295
	lbu	$9,2($6)
	bne	$9,$7,$L295
	lw	$9,16($6)
	bne	$9,$3,$L295
	lw	$9,12($6)
	bne	$9,$4,$L295
	lw	$10,24($6)
	lw	$9,12($16)
	bne	$10,$9,$L295
	lw	$10,28($6)
	lw	$9,16($16)
	bne	$10,$9,$L295
	lw	$9,20($6)
	bne	$9,$2,$L295
	lw	$9,8($6)
	bne	$9,$5,$L295
	lw	$3,64($6)
	sb	$0,0($6)
	lw	$2,68($6)
	beq	$3,$0,$L296
	.set	noreorder
	.set	nomacro
	j	$L297
	sw	$2,68($3)
	.set	macro
	.set	reorder

$L296:
	lui	$3,%hi(table_path)
	lw	$3,%lo(table_path)($3)
	lw	$3,0($3)
	addu	$3,$3,$8
	sw	$2,4($3)
$L297:
	lw	$2,68($6)
	lw	$3,64($6)
	sw	$3,0($2)
	lui	$2,%hi(table_path)
	lw	$2,%lo(table_path)($2)
	lw	$2,0($2)
	addu	$8,$2,$8
	lw	$2,8($8)
	addiu	$2,$2,-1
	sw	$2,8($8)
	lw	$3,72($6)
	lw	$2,76($6)
	beq	$3,$0,$L298
	.set	noreorder
	.set	nomacro
	j	$L299
	sw	$2,76($3)
	.set	macro
	.set	reorder

$L298:
	lui	$3,%hi(path_list_inuse+4)
	sw	$2,%lo(path_list_inuse+4)($3)
$L299:
	lw	$2,76($6)
	lui	$3,%hi(path_list_inuse)
	lw	$5,72($6)
	addiu	$3,$3,%lo(path_list_inuse)
	addiu	$4,$6,72
	sw	$5,0($2)
	lui	$2,%hi(path_list_free)
	lw	$5,8($3)
	addiu	$2,$2,%lo(path_list_free)
	addiu	$5,$5,-1
	sw	$5,8($3)
	sw	$0,72($6)
	lw	$3,4($2)
	sw	$3,76($6)
	lw	$3,4($2)
	sw	$6,0($3)
	lw	$3,8($2)
	sw	$4,4($2)
	addiu	$3,$3,1
	.set	noreorder
	.set	nomacro
	j	$L300
	sw	$3,8($2)
	.set	macro
	.set	reorder

$L295:
	lw	$6,64($6)
$L294:
	bne	$6,$0,$L301
$L300:
	lw	$3,20($16)
	lui	$8,%hi(path_table_list_max)
	lw	$2,12($16)
	lw	$5,16($16)
	srl	$6,$3,8
	xor	$9,$2,$3
	lw	$4,24($16)
	xor	$9,$9,$5
	lw	$8,%lo(path_table_list_max)($8)
	xor	$9,$9,$6
	lui	$6,%hi(table_path)
	srl	$10,$2,16
	addiu	$8,$8,-1
	xor	$9,$9,$10
	lw	$6,%lo(table_path)($6)
	srl	$10,$4,4
	xor	$9,$9,$10
	lw	$6,0($6)
	xor	$18,$9,$18
	srl	$9,$18,12
	xor	$18,$9,$18
	and	$18,$18,$8
	li	$8,12			# 0xc
	mult	$18,$8
	mflo	$18
	addu	$6,$6,$18
	lw	$6,0($6)
	.set	noreorder
	.set	nomacro
	j	$L302
	li	$7,2			# 0x2
	.set	macro
	.set	reorder

$L309:
	lw	$9,4($6)
	lw	$8,0($16)
	bne	$9,$8,$L303
	lbu	$8,2($6)
	bne	$8,$7,$L303
	lw	$8,20($6)
	bne	$8,$2,$L303
	lw	$8,8($6)
	bne	$8,$5,$L303
	lw	$8,24($6)
	bne	$8,$3,$L303
	lw	$8,28($6)
	bne	$8,$4,$L303
	lw	$9,32($6)
	lw	$8,4($16)
	bne	$9,$8,$L303
	lw	$9,36($6)
	lw	$8,8($16)
	bne	$9,$8,$L303
	lw	$3,64($6)
	sb	$0,0($6)
	lw	$2,68($6)
	beq	$3,$0,$L304
	.set	noreorder
	.set	nomacro
	j	$L305
	sw	$2,68($3)
	.set	macro
	.set	reorder

$L304:
	lui	$3,%hi(table_path)
	lw	$3,%lo(table_path)($3)
	lw	$3,0($3)
	addu	$3,$3,$18
	sw	$2,4($3)
$L305:
	lw	$2,68($6)
	lw	$3,64($6)
	sw	$3,0($2)
	lui	$2,%hi(table_path)
	lw	$2,%lo(table_path)($2)
	lw	$2,0($2)
	addu	$18,$2,$18
	lw	$2,8($18)
	addiu	$2,$2,-1
	sw	$2,8($18)
	lw	$3,72($6)
	lw	$2,76($6)
	beq	$3,$0,$L306
	.set	noreorder
	.set	nomacro
	j	$L307
	sw	$2,76($3)
	.set	macro
	.set	reorder

$L306:
	lui	$3,%hi(path_list_inuse+4)
	sw	$2,%lo(path_list_inuse+4)($3)
$L307:
	lw	$2,76($6)
	lui	$3,%hi(path_list_inuse)
	lw	$5,72($6)
	addiu	$3,$3,%lo(path_list_inuse)
	addiu	$4,$6,72
	sw	$5,0($2)
	lui	$2,%hi(path_list_free)
	lw	$5,8($3)
	addiu	$2,$2,%lo(path_list_free)
	addiu	$5,$5,-1
	sw	$5,8($3)
	sw	$0,72($6)
	lw	$3,4($2)
	sw	$3,76($6)
	lw	$3,4($2)
	sw	$6,0($3)
	lw	$3,8($2)
	sw	$4,4($2)
	addiu	$3,$3,1
	.set	noreorder
	.set	nomacro
	j	$L308
	sw	$3,8($2)
	.set	macro
	.set	reorder

$L303:
	lw	$6,64($6)
$L302:
	bne	$6,$0,$L309
$L308:
	andi	$2,$17,0x1
	bne	$2,$0,$L310
#APP
 # 134 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/irqflags.h" 1
	raw_local_irq_restore	$17
 # 0 "" 2
#NO_APP
	j	$L311
$L310:
#APP
 # 134 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/irqflags.h" 1
	raw_local_irq_restore	$17
 # 0 "" 2
#NO_APP
	.set	noreorder
	.set	nomacro
	j	$L311
	move	$2,$0
	.set	macro
	.set	reorder

$L289:
	lw	$8,36($8)
$L288:
	bne	$8,$0,$L312
#APP
 # 134 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/irqflags.h" 1
	raw_local_irq_restore	$17
 # 0 "" 2
#NO_APP
	li	$2,-4			# 0xfffffffc
$L311:
	lw	$31,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,32
	.set	macro
	.set	reorder

	.end	rtk_delNaptConnection
	.size	rtk_delNaptConnection, .-rtk_delNaptConnection
	.section	.text.rtk_addNaptConnection,"ax",@progbits
	.align	2
	.globl	rtk_addNaptConnection
	.set	nomips16
	.ent	rtk_addNaptConnection
	.type	rtk_addNaptConnection, @function
rtk_addNaptConnection:
	.frame	$sp,144,$31		# vars= 88, regs= 10/0, args= 16, gp= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-144
	sw	$31,140($sp)
	sw	$fp,136($sp)
	sw	$23,132($sp)
	sw	$22,128($sp)
	sw	$21,124($sp)
	sw	$20,120($sp)
	sw	$19,116($sp)
	sw	$18,112($sp)
	sw	$17,108($sp)
	sw	$16,104($sp)
	move	$17,$5
	move	$16,$4
	sw	$6,152($sp)
	.set	noreorder
	.set	nomacro
	beq	$5,$0,$L316
	move	$18,$7
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_eth_hdr
	move	$4,$5
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L316
	li	$2,6			# 0x6
	.set	macro
	.set	reorder

	lw	$3,0($16)
	lw	$4,20($16)
	li	$fp,17			# 0x11
	xori	$3,$3,0x2
	.set	noreorder
	.set	nomacro
	jal	filter_addconnect
	movz	$fp,$2,$3
	.set	macro
	.set	reorder

#APP
 # 818 "net/rtl/fastpath/fastpath_core.c" 1
	raw_local_irq_save	$23
 # 0 "" 2
#NO_APP
	lw	$4,4($16)
	lui	$8,%hi(napt_table_list_max)
	lw	$6,8($16)
	lw	$3,20($16)
	srl	$7,$4,16
	lw	$5,24($16)
	xor	$9,$6,$4
	xor	$9,$9,$3
	lw	$2,0($16)
	xor	$9,$9,$5
	lw	$10,%lo(napt_table_list_max)($8)
	xor	$9,$9,$7
	lui	$7,%hi(table_napt)
	srl	$11,$3,16
	addiu	$10,$10,-1
	andi	$8,$2,0xffff
	lw	$7,%lo(table_napt)($7)
	xor	$9,$9,$11
	xor	$9,$9,$8
	lw	$7,0($7)
	srl	$8,$9,12
	xor	$8,$8,$9
	and	$8,$8,$10
	li	$10,12			# 0xc
	mult	$8,$10
	mflo	$8
	addu	$7,$7,$8
	lw	$7,0($7)
	j	$L319
$L323:
	lw	$9,4($7)
	bne	$9,$2,$L320
	lw	$9,8($7)
	bne	$9,$4,$L320
	lw	$9,12($7)
	bne	$9,$6,$L320
	lw	$10,16($7)
	lw	$9,12($16)
	bne	$10,$9,$L320
	lw	$10,20($7)
	lw	$9,16($16)
	bne	$10,$9,$L320
	lw	$9,24($7)
	bne	$9,$3,$L320
	lw	$9,28($7)
	bne	$9,$5,$L320
	lw	$4,152($sp)
	move	$5,$fp
	.set	noreorder
	.set	nomacro
	jal	rtl_conntrack_drop_check_hook
	move	$6,$4
	.set	macro
	.set	reorder

	andi	$2,$23,0x1
	bne	$2,$0,$L321
#APP
 # 134 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/irqflags.h" 1
	raw_local_irq_restore	$23
 # 0 "" 2
#NO_APP
	j	$L322
$L321:
#APP
 # 134 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/irqflags.h" 1
	raw_local_irq_restore	$23
 # 0 "" 2
#NO_APP
	.set	noreorder
	.set	nomacro
	j	$L322
	move	$2,$0
	.set	macro
	.set	reorder

$L320:
	lw	$7,36($7)
$L319:
	bne	$7,$0,$L323
	lui	$4,%hi(napt_list_free)
	lw	$3,%lo(napt_list_free)($4)
	beq	$3,$0,$L324
	sw	$2,4($3)
	lw	$2,4($16)
	lw	$5,44($3)
	sw	$2,8($3)
	lw	$6,8($16)
	lw	$2,48($3)
	sw	$6,12($3)
	lw	$6,12($16)
	sw	$6,16($3)
	lw	$6,16($16)
	sw	$6,20($3)
	lw	$6,20($16)
	sw	$6,24($3)
	lw	$6,24($16)
	sw	$18,32($3)
	sw	$6,28($3)
	li	$6,-1			# 0xffffffff
	.set	noreorder
	.set	nomacro
	beq	$5,$0,$L325
	sb	$6,0($3)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L326
	sw	$2,48($5)
	.set	macro
	.set	reorder

$L325:
	addiu	$4,$4,%lo(napt_list_free)
	sw	$2,4($4)
$L326:
	lw	$2,48($3)
	lui	$5,%hi(napt_list_free)
	lw	$4,44($3)
	addiu	$5,$5,%lo(napt_list_free)
	addiu	$7,$3,44
	addiu	$6,$3,36
	sw	$4,0($2)
	lui	$2,%hi(napt_list_inuse)
	lw	$9,8($5)
	addiu	$2,$2,%lo(napt_list_inuse)
	lui	$4,%hi(table_napt)
	addiu	$9,$9,-1
	sw	$9,8($5)
	sw	$0,44($3)
	lw	$5,4($2)
	sw	$5,48($3)
	lw	$5,4($2)
	sw	$3,0($5)
	lw	$5,8($2)
	sw	$7,4($2)
	addiu	$5,$5,1
	sw	$5,8($2)
	sw	$0,36($3)
	lw	$2,%lo(table_napt)($4)
	lw	$2,0($2)
	addu	$2,$2,$8
	lw	$2,4($2)
	sw	$2,40($3)
	lw	$2,%lo(table_napt)($4)
	lw	$2,0($2)
	addu	$2,$2,$8
	lw	$2,4($2)
	sw	$3,0($2)
	lw	$2,%lo(table_napt)($4)
	lw	$2,0($2)
	addu	$2,$2,$8
	sw	$6,4($2)
	lw	$2,%lo(table_napt)($4)
	lw	$2,0($2)
	addu	$8,$2,$8
	lw	$2,8($8)
	addiu	$2,$2,1
	sw	$2,8($8)
	li	$2,33			# 0x21
	sb	$2,17($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl865x_getLanDev
	sb	$0,16($sp)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl865x_getWanDev
	move	$20,$2
	.set	macro
	.set	reorder

	move	$4,$17
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_protocol
	move	$19,$2
	.set	macro
	.set	reorder

	move	$4,$17
	.set	noreorder
	.set	nomacro
	jal	rtl_ip_hdr
	sw	$2,64($sp)
	.set	macro
	.set	reorder

	move	$4,$17
	lbu	$22,0($2)
	move	$21,$2
	andi	$22,$22,0xf
	sll	$22,$22,2
	.set	noreorder
	.set	nomacro
	jal	rtl_eth_hdr
	addu	$22,$2,$22
	.set	macro
	.set	reorder

	li	$6,6			# 0x6
	addiu	$5,$2,6
	.set	noreorder
	.set	nomacro
	jal	memcpy
	addiu	$4,$sp,20
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_eth_hdr
	move	$4,$17
	.set	macro
	.set	reorder

	li	$6,6			# 0x6
	move	$5,$2
	.set	noreorder
	.set	nomacro
	jal	memcpy
	addiu	$4,$sp,28
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_mark
	move	$4,$17
	.set	macro
	.set	reorder

	move	$4,$17
	sw	$2,80($sp)
	lw	$2,12($21)
	sw	$2,96($sp)
	lhu	$3,0($22)
	sw	$3,88($sp)
	lw	$5,16($21)
	sw	$5,92($sp)
	lhu	$2,2($22)
	.set	noreorder
	.set	nomacro
	jal	rtl_store_skb_dst
	sw	$2,84($sp)
	.set	macro
	.set	reorder

	lui	$2,%hi(path_list_free)
	lw	$18,%lo(path_list_free)($2)
	lui	$2,%hi(path_table_list_max)
	lw	$2,%lo(path_table_list_max)($2)
	sw	$2,76($sp)
	lw	$3,4($16)
	sw	$3,60($sp)
	lw	$4,8($16)
	sw	$4,72($sp)
	lw	$5,20($16)
	sw	$5,56($sp)
	lw	$2,24($16)
	.set	noreorder
	.set	nomacro
	beq	$18,$0,$L327
	sw	$2,68($sp)
	.set	macro
	.set	reorder

	lw	$2,0($16)
	move	$4,$0
	sw	$2,4($18)
	lw	$6,4($16)
	sw	$6,16($18)
	lw	$2,8($16)
	sw	$2,12($18)
	lw	$2,20($16)
	sw	$2,20($18)
	lw	$2,24($16)
	sw	$2,8($18)
	lw	$5,12($16)
	sw	$5,24($18)
	lw	$2,16($16)
	sw	$2,28($18)
	lw	$8,20($16)
	sw	$8,32($18)
	lw	$2,24($16)
	sw	$2,36($18)
	lui	$2,%hi(route_list_inuse)
	lw	$2,%lo(route_list_inuse)($2)
	.set	noreorder
	.set	nomacro
	j	$L328
	move	$7,$0
	.set	macro
	.set	reorder

$L331:
	lw	$3,8($2)
	sltu	$9,$3,$4
	.set	noreorder
	.set	nomacro
	bne	$9,$0,$L329
	and	$10,$3,$8
	.set	macro
	.set	reorder

	lw	$9,4($2)
	bne	$10,$9,$L329
	.set	noreorder
	.set	nomacro
	j	$L330
	addiu	$7,$2,16
	.set	macro
	.set	reorder

$L329:
	move	$3,$4
$L330:
	lw	$2,44($2)
	move	$4,$3
$L328:
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L331
	li	$3,-1			# 0xffffffff
	.set	macro
	.set	reorder

	li	$2,1			# 0x1
	sw	$7,44($18)
	sb	$3,0($18)
	sb	$2,2($18)
	.set	noreorder
	.set	nomacro
	beq	$6,$5,$L332
	sb	$0,1($18)
	.set	macro
	.set	reorder

	sb	$2,1($18)
$L332:
	lw	$3,12($18)
	lw	$2,28($18)
	beq	$3,$2,$L333
	lbu	$2,1($18)
	ori	$2,$2,0x2
	sb	$2,1($18)
$L333:
	sw	$0,48($18)
	addiu	$3,$sp,36
	sw	$0,52($18)
	move	$4,$3
	move	$5,$0
	lw	$2,4($16)
	li	$6,14			# 0xe
	sw	$2,12($21)
	lw	$2,8($16)
	sh	$2,0($22)
	lw	$2,20($16)
	sw	$2,16($21)
	lw	$2,24($16)
	sh	$2,2($22)
	.set	noreorder
	.set	nomacro
	jal	memset
	sw	$3,100($sp)
	.set	macro
	.set	reorder

	move	$4,$17
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_mark
	move	$5,$0
	.set	macro
	.set	reorder

	lw	$3,100($sp)
	.set	noreorder
	.set	nomacro
	beq	$20,$0,$L334
	move	$6,$3
	.set	macro
	.set	reorder

	lw	$4,4($16)
	.set	noreorder
	.set	nomacro
	jal	rtl_arp_req_get_ha
	move	$5,$20
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L334
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_ppp_proto_check
	addiu	$5,$sp,16
	.set	macro
	.set	reorder

	move	$3,$2
	li	$2,1			# 0x1
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L335
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_skb_mac_header_was_set
	sw	$3,100($sp)
	.set	macro
	.set	reorder

	lw	$3,100($sp)
	bne	$2,$3,$L335
	.set	noreorder
	.set	nomacro
	jal	rtl_eth_hdr
	move	$4,$17
	.set	macro
	.set	reorder

	lbu	$3,12($2)
	lbu	$2,13($2)
	sll	$3,$3,8
	or	$3,$2,$3
	li	$2,2048			# 0x800
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L335
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_skb_set_mac_header
	li	$5,-22			# 0xffffffea
	.set	macro
	.set	reorder

$L335:
	.set	noreorder
	.set	nomacro
	jal	rtl_eth_hdr
	move	$4,$17
	.set	macro
	.set	reorder

	li	$6,6			# 0x6
	addiu	$4,$2,6
	.set	noreorder
	.set	nomacro
	jal	memcpy
	addiu	$5,$sp,36
	.set	macro
	.set	reorder

	move	$4,$17
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_dmac
	move	$5,$20
	.set	macro
	.set	reorder

	li	$2,2048			# 0x800
	lw	$3,64($sp)
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L334
	lui	$2,%hi(nf_hooks+128)
	.set	macro
	.set	reorder

	lw	$3,%lo(nf_hooks+128)($2)
	addiu	$2,$2,%lo(nf_hooks+128)
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L336
	move	$4,$17
	.set	macro
	.set	reorder

	move	$5,$0
	move	$6,$20
	.set	noreorder
	.set	nomacro
	jal	rtl_ipt_do_table
	move	$7,$19
	.set	macro
	.set	reorder

$L336:
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_mark
	move	$4,$17
	.set	macro
	.set	reorder

	beq	$2,$0,$L337
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_mark
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L378
	sw	$2,48($18)
	.set	macro
	.set	reorder

$L337:
	lw	$2,48($18)
	sw	$2,48($18)
$L378:
	lui	$2,%hi(nf_hooks+160)
	lw	$3,%lo(nf_hooks+160)($2)
	addiu	$2,$2,%lo(nf_hooks+160)
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L339
	move	$4,$17
	.set	macro
	.set	reorder

	li	$5,4			# 0x4
	move	$6,$20
	.set	noreorder
	.set	nomacro
	jal	rtl_ipt_do_table
	move	$7,$19
	.set	macro
	.set	reorder

$L339:
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_mark
	move	$4,$17
	.set	macro
	.set	reorder

	beq	$2,$0,$L340
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_mark
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L334
	sw	$2,52($18)
	.set	macro
	.set	reorder

$L340:
	lw	$2,52($18)
	sw	$2,52($18)
$L334:
	lw	$2,12($16)
	addiu	$3,$sp,36
	move	$4,$3
	move	$5,$0
	li	$6,14			# 0xe
	sw	$2,12($21)
	lw	$2,16($16)
	sh	$2,0($22)
	lw	$2,20($16)
	sw	$2,16($21)
	lw	$2,24($16)
	sh	$2,2($22)
	.set	noreorder
	.set	nomacro
	jal	memset
	sw	$3,100($sp)
	.set	macro
	.set	reorder

	move	$4,$17
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_mark
	move	$5,$0
	.set	macro
	.set	reorder

	lw	$3,100($sp)
	.set	noreorder
	.set	nomacro
	beq	$19,$0,$L342
	move	$6,$3
	.set	macro
	.set	reorder

	lw	$4,20($16)
	.set	noreorder
	.set	nomacro
	jal	rtl_arp_req_get_ha
	move	$5,$19
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L381
	li	$2,6			# 0x6
	.set	macro
	.set	reorder

	move	$4,$17
	.set	noreorder
	.set	nomacro
	jal	rtl_ppp_proto_check
	addiu	$5,$sp,16
	.set	macro
	.set	reorder

	move	$3,$2
	li	$2,1			# 0x1
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L343
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_skb_mac_header_was_set
	sw	$3,100($sp)
	.set	macro
	.set	reorder

	lw	$3,100($sp)
	bne	$2,$3,$L343
	.set	noreorder
	.set	nomacro
	jal	rtl_eth_hdr
	move	$4,$17
	.set	macro
	.set	reorder

	lbu	$3,12($2)
	lbu	$2,13($2)
	sll	$3,$3,8
	or	$3,$2,$3
	li	$2,2048			# 0x800
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L343
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_skb_set_mac_header
	li	$5,-22			# 0xffffffea
	.set	macro
	.set	reorder

$L343:
	.set	noreorder
	.set	nomacro
	jal	rtl_eth_hdr
	move	$4,$17
	.set	macro
	.set	reorder

	li	$6,6			# 0x6
	move	$4,$2
	.set	noreorder
	.set	nomacro
	jal	memcpy
	addiu	$5,$sp,36
	.set	macro
	.set	reorder

	move	$4,$17
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_smac
	move	$5,$19
	.set	macro
	.set	reorder

	li	$2,2048			# 0x800
	lw	$4,64($sp)
	.set	noreorder
	.set	nomacro
	bne	$4,$2,$L381
	li	$2,6			# 0x6
	.set	macro
	.set	reorder

	lui	$2,%hi(nf_hooks+160)
	lw	$3,%lo(nf_hooks+160)($2)
	addiu	$2,$2,%lo(nf_hooks+160)
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L344
	move	$4,$17
	.set	macro
	.set	reorder

	li	$5,4			# 0x4
	move	$6,$20
	.set	noreorder
	.set	nomacro
	jal	rtl_ipt_do_table
	move	$7,$19
	.set	macro
	.set	reorder

$L344:
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_mark
	move	$4,$17
	.set	macro
	.set	reorder

	beq	$2,$0,$L345
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_mark
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L342
	sw	$2,52($18)
	.set	macro
	.set	reorder

$L345:
	lw	$2,52($18)
	sw	$2,52($18)
$L342:
	li	$2,6			# 0x6
$L381:
	.set	noreorder
	.set	nomacro
	bne	$fp,$2,$L347
	lui	$2,%hi(nf_ct_udp_timeout_stream)
	.set	macro
	.set	reorder

	lw	$4,152($sp)
	jal	rtl_tcp_get_timeouts
	.set	noreorder
	.set	nomacro
	j	$L379
	sw	$2,60($18)
	.set	macro
	.set	reorder

$L347:
	lw	$2,%lo(nf_ct_udp_timeout_stream)($2)
	sw	$2,60($18)
$L379:
	lui	$2,%hi(jiffies)
	lw	$5,152($sp)
	lw	$3,72($18)
	sw	$5,56($18)
	lw	$2,%lo(jiffies)($2)
	sw	$2,40($18)
	lw	$2,76($18)
	beq	$3,$0,$L349
	.set	noreorder
	.set	nomacro
	j	$L350
	sw	$2,76($3)
	.set	macro
	.set	reorder

$L349:
	lui	$3,%hi(path_list_free+4)
	sw	$2,%lo(path_list_free+4)($3)
$L350:
	lw	$3,56($sp)
	lui	$8,%hi(path_list_free)
	lw	$4,60($sp)
	addiu	$8,$8,%lo(path_list_free)
	lw	$5,68($sp)
	xor	$6,$3,$4
	lw	$2,76($18)
	lw	$4,72($18)
	xor	$6,$6,$5
	lw	$5,60($sp)
	srl	$3,$5,8
	lw	$5,76($sp)
	sw	$4,0($2)
	xor	$6,$6,$3
	lui	$2,%hi(path_list_inuse)
	lw	$4,56($sp)
	addiu	$2,$2,%lo(path_list_inuse)
	lw	$10,8($8)
	addiu	$9,$5,-1
	srl	$3,$4,16
	lw	$4,72($sp)
	addiu	$5,$18,72
	addiu	$10,$10,-1
	xor	$6,$6,$3
	lui	$3,%hi(table_path)
	sw	$10,8($8)
	srl	$7,$4,4
	addiu	$4,$18,64
	sw	$0,72($18)
	xor	$6,$6,$7
	lw	$8,4($2)
	xor	$6,$6,$fp
	srl	$7,$6,12
	sw	$8,76($18)
	xor	$7,$7,$6
	li	$8,12			# 0xc
	lw	$6,4($2)
	and	$7,$7,$9
	mult	$7,$8
	sw	$18,0($6)
	lw	$6,8($2)
	sw	$5,4($2)
	addiu	$5,$6,1
	mflo	$7
	sw	$5,8($2)
	sw	$0,64($18)
	lw	$2,%lo(table_path)($3)
	lw	$2,0($2)
	addu	$2,$2,$7
	lw	$2,4($2)
	sw	$2,68($18)
	lw	$2,%lo(table_path)($3)
	lw	$2,0($2)
	addu	$2,$2,$7
	lw	$2,4($2)
	sw	$18,0($2)
	lw	$2,%lo(table_path)($3)
	lw	$2,0($2)
	addu	$2,$2,$7
	sw	$4,4($2)
	lw	$2,%lo(table_path)($3)
	lw	$2,0($2)
	addu	$7,$2,$7
	lw	$2,8($7)
	addiu	$2,$2,1
	sw	$2,8($7)
$L327:
	lui	$2,%hi(path_list_free)
	lw	$18,%lo(path_list_free)($2)
	lui	$2,%hi(path_table_list_max)
	lw	$2,%lo(path_table_list_max)($2)
	sw	$2,76($sp)
	lw	$2,20($16)
	sw	$2,60($sp)
	lw	$3,24($16)
	sw	$3,72($sp)
	lw	$4,12($16)
	sw	$4,56($sp)
	lw	$5,16($16)
	.set	noreorder
	.set	nomacro
	beq	$18,$0,$L351
	sw	$5,68($sp)
	.set	macro
	.set	reorder

	lw	$2,0($16)
	move	$5,$0
	sw	$2,4($18)
	lw	$2,20($16)
	sw	$2,16($18)
	lw	$2,24($16)
	sw	$2,12($18)
	lw	$6,12($16)
	sw	$6,20($18)
	lw	$2,16($16)
	sw	$2,8($18)
	lw	$2,20($16)
	sw	$2,24($18)
	lw	$2,24($16)
	sw	$2,28($18)
	lw	$4,4($16)
	sw	$4,32($18)
	lw	$2,8($16)
	sw	$2,36($18)
	lui	$2,%hi(route_list_inuse)
	lw	$2,%lo(route_list_inuse)($2)
	.set	noreorder
	.set	nomacro
	j	$L352
	move	$7,$0
	.set	macro
	.set	reorder

$L355:
	lw	$3,8($2)
	sltu	$8,$3,$5
	.set	noreorder
	.set	nomacro
	bne	$8,$0,$L353
	and	$9,$3,$4
	.set	macro
	.set	reorder

	lw	$8,4($2)
	bne	$9,$8,$L353
	.set	noreorder
	.set	nomacro
	j	$L354
	addiu	$7,$2,16
	.set	macro
	.set	reorder

$L353:
	move	$3,$5
$L354:
	lw	$2,44($2)
	move	$5,$3
$L352:
	bne	$2,$0,$L355
	li	$2,2			# 0x2
	sb	$2,2($18)
	li	$2,-1			# 0xffffffff
	sw	$7,44($18)
	sb	$2,0($18)
	.set	noreorder
	.set	nomacro
	beq	$6,$4,$L356
	sb	$0,1($18)
	.set	macro
	.set	reorder

	li	$2,4			# 0x4
	sb	$2,1($18)
$L356:
	lw	$3,8($18)
	lw	$2,36($18)
	beq	$3,$2,$L357
	lbu	$2,1($18)
	ori	$2,$2,0x8
	sb	$2,1($18)
$L357:
	sw	$0,48($18)
	addiu	$3,$sp,36
	sw	$0,52($18)
	move	$4,$3
	move	$5,$0
	lw	$2,20($16)
	li	$6,14			# 0xe
	sw	$2,12($21)
	lw	$2,24($16)
	sh	$2,0($22)
	lw	$2,12($16)
	sw	$2,16($21)
	lw	$2,16($16)
	sh	$2,2($22)
	.set	noreorder
	.set	nomacro
	jal	memset
	sw	$3,100($sp)
	.set	macro
	.set	reorder

	move	$4,$17
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_mark
	move	$5,$0
	.set	macro
	.set	reorder

	lw	$3,100($sp)
	.set	noreorder
	.set	nomacro
	beq	$19,$0,$L358
	move	$6,$3
	.set	macro
	.set	reorder

	lw	$4,20($16)
	.set	noreorder
	.set	nomacro
	jal	rtl_arp_req_get_ha
	move	$5,$19
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L358
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_ppp_proto_check
	addiu	$5,$sp,16
	.set	macro
	.set	reorder

	move	$3,$2
	li	$2,1			# 0x1
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L359
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_skb_mac_header_was_set
	sw	$3,100($sp)
	.set	macro
	.set	reorder

	lw	$3,100($sp)
	bne	$2,$3,$L359
	.set	noreorder
	.set	nomacro
	jal	rtl_eth_hdr
	move	$4,$17
	.set	macro
	.set	reorder

	lbu	$3,12($2)
	lbu	$2,13($2)
	sll	$3,$3,8
	or	$3,$2,$3
	li	$2,2048			# 0x800
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L359
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_skb_set_mac_header
	li	$5,-22			# 0xffffffea
	.set	macro
	.set	reorder

$L359:
	.set	noreorder
	.set	nomacro
	jal	rtl_eth_hdr
	move	$4,$17
	.set	macro
	.set	reorder

	li	$6,6			# 0x6
	addiu	$4,$2,6
	.set	noreorder
	.set	nomacro
	jal	memcpy
	addiu	$5,$sp,36
	.set	macro
	.set	reorder

	move	$4,$17
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_dmac
	move	$5,$19
	.set	macro
	.set	reorder

	li	$2,2048			# 0x800
	lw	$3,64($sp)
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L360
	lui	$2,%hi(nf_hooks+128)
	.set	macro
	.set	reorder

	lw	$3,%lo(nf_hooks+128)($2)
	addiu	$2,$2,%lo(nf_hooks+128)
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L360
	move	$4,$17
	.set	macro
	.set	reorder

	move	$5,$0
	move	$6,$19
	.set	noreorder
	.set	nomacro
	jal	rtl_ipt_do_table
	move	$7,$20
	.set	macro
	.set	reorder

$L360:
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_mark
	move	$4,$17
	.set	macro
	.set	reorder

	beq	$2,$0,$L361
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_mark
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L358
	sw	$2,48($18)
	.set	macro
	.set	reorder

$L361:
	lw	$2,48($18)
	sw	$2,48($18)
$L358:
	lw	$2,20($16)
	addiu	$3,$sp,36
	move	$4,$3
	move	$5,$0
	li	$6,14			# 0xe
	sw	$2,12($21)
	lw	$2,24($16)
	sh	$2,0($22)
	lw	$2,4($16)
	sw	$2,16($21)
	lw	$2,8($16)
	sh	$2,2($22)
	.set	noreorder
	.set	nomacro
	jal	memset
	sw	$3,100($sp)
	.set	macro
	.set	reorder

	move	$4,$17
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_mark
	move	$5,$0
	.set	macro
	.set	reorder

	lw	$3,100($sp)
	.set	noreorder
	.set	nomacro
	beq	$20,$0,$L363
	move	$6,$3
	.set	macro
	.set	reorder

	lw	$4,4($16)
	.set	noreorder
	.set	nomacro
	jal	rtl_arp_req_get_ha
	move	$5,$20
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L363
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_ppp_proto_check
	addiu	$5,$sp,16
	.set	macro
	.set	reorder

	move	$16,$2
	li	$2,1			# 0x1
	bne	$16,$2,$L364
	.set	noreorder
	.set	nomacro
	jal	rtl_skb_mac_header_was_set
	move	$4,$17
	.set	macro
	.set	reorder

	bne	$2,$16,$L364
	.set	noreorder
	.set	nomacro
	jal	rtl_eth_hdr
	move	$4,$17
	.set	macro
	.set	reorder

	lbu	$3,12($2)
	lbu	$2,13($2)
	sll	$3,$3,8
	or	$3,$2,$3
	li	$2,2048			# 0x800
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L364
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_skb_set_mac_header
	li	$5,-22			# 0xffffffea
	.set	macro
	.set	reorder

$L364:
	.set	noreorder
	.set	nomacro
	jal	rtl_eth_hdr
	move	$4,$17
	.set	macro
	.set	reorder

	li	$6,6			# 0x6
	move	$4,$2
	.set	noreorder
	.set	nomacro
	jal	memcpy
	addiu	$5,$sp,36
	.set	macro
	.set	reorder

	move	$4,$17
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_smac
	move	$5,$20
	.set	macro
	.set	reorder

	li	$2,2048			# 0x800
	lw	$4,64($sp)
	.set	noreorder
	.set	nomacro
	bne	$4,$2,$L363
	lui	$2,%hi(nf_hooks+160)
	.set	macro
	.set	reorder

	lw	$3,%lo(nf_hooks+160)($2)
	addiu	$2,$2,%lo(nf_hooks+160)
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L363
	move	$4,$17
	.set	macro
	.set	reorder

	li	$5,4			# 0x4
	move	$6,$19
	.set	noreorder
	.set	nomacro
	jal	rtl_ipt_do_table
	move	$7,$20
	.set	macro
	.set	reorder

$L363:
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_mark
	move	$4,$17
	.set	macro
	.set	reorder

	sw	$2,52($18)
	li	$2,6			# 0x6
	.set	noreorder
	.set	nomacro
	bne	$fp,$2,$L365
	lui	$2,%hi(nf_ct_udp_timeout_stream)
	.set	macro
	.set	reorder

	lw	$4,152($sp)
	jal	rtl_tcp_get_timeouts
	.set	noreorder
	.set	nomacro
	j	$L380
	sw	$2,60($18)
	.set	macro
	.set	reorder

$L365:
	lw	$2,%lo(nf_ct_udp_timeout_stream)($2)
	sw	$2,60($18)
$L380:
	lui	$2,%hi(jiffies)
	lw	$5,152($sp)
	lw	$3,72($18)
	sw	$5,56($18)
	lw	$2,%lo(jiffies)($2)
	sw	$2,40($18)
	lw	$2,76($18)
	beq	$3,$0,$L367
	.set	noreorder
	.set	nomacro
	j	$L368
	sw	$2,76($3)
	.set	macro
	.set	reorder

$L367:
	lui	$3,%hi(path_list_free+4)
	sw	$2,%lo(path_list_free+4)($3)
$L368:
	lw	$3,56($sp)
	lui	$8,%hi(path_list_free)
	lw	$4,60($sp)
	addiu	$8,$8,%lo(path_list_free)
	lw	$5,68($sp)
	xor	$6,$3,$4
	lw	$2,76($18)
	lw	$4,72($18)
	xor	$6,$6,$5
	lw	$5,60($sp)
	srl	$3,$5,8
	lw	$5,76($sp)
	sw	$4,0($2)
	xor	$6,$6,$3
	lui	$2,%hi(path_list_inuse)
	lw	$4,56($sp)
	addiu	$2,$2,%lo(path_list_inuse)
	lw	$10,8($8)
	addiu	$9,$5,-1
	srl	$3,$4,16
	lw	$4,72($sp)
	addiu	$5,$18,72
	addiu	$10,$10,-1
	xor	$6,$6,$3
	lui	$3,%hi(table_path)
	sw	$10,8($8)
	srl	$7,$4,4
	addiu	$4,$18,64
	sw	$0,72($18)
	xor	$6,$6,$7
	lw	$8,4($2)
	xor	$6,$6,$fp
	srl	$7,$6,12
	sw	$8,76($18)
	xor	$7,$7,$6
	li	$8,12			# 0xc
	lw	$6,4($2)
	and	$7,$7,$9
	mult	$7,$8
	sw	$18,0($6)
	lw	$6,8($2)
	sw	$5,4($2)
	addiu	$5,$6,1
	mflo	$7
	sw	$5,8($2)
	sw	$0,64($18)
	lw	$2,%lo(table_path)($3)
	lw	$2,0($2)
	addu	$2,$2,$7
	lw	$2,4($2)
	sw	$2,68($18)
	lw	$2,%lo(table_path)($3)
	lw	$2,0($2)
	addu	$2,$2,$7
	lw	$2,4($2)
	sw	$18,0($2)
	lw	$2,%lo(table_path)($3)
	lw	$2,0($2)
	addu	$2,$2,$7
	sw	$4,4($2)
	lw	$2,%lo(table_path)($3)
	lw	$2,0($2)
	addu	$7,$2,$7
	lw	$2,8($7)
	addiu	$2,$2,1
	sw	$2,8($7)
$L351:
	.set	noreorder
	.set	nomacro
	jal	rtl_eth_hdr
	move	$4,$17
	.set	macro
	.set	reorder

	addiu	$5,$sp,20
	li	$6,6			# 0x6
	.set	noreorder
	.set	nomacro
	jal	memcpy
	addiu	$4,$2,6
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_eth_hdr
	move	$4,$17
	.set	macro
	.set	reorder

	li	$6,6			# 0x6
	move	$4,$2
	.set	noreorder
	.set	nomacro
	jal	memcpy
	addiu	$5,$sp,28
	.set	macro
	.set	reorder

	lw	$5,80($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_mark
	move	$4,$17
	.set	macro
	.set	reorder

	move	$4,$17
	lw	$2,96($sp)
	sw	$2,12($21)
	lw	$3,88($sp)
	sh	$3,0($22)
	lw	$5,92($sp)
	sw	$5,16($21)
	lw	$2,84($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_dst
	sh	$2,2($22)
	.set	macro
	.set	reorder

	beq	$20,$0,$L369
#APP
 # 94 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
 # 96 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	1:	ll	    $2, 452($20)		# atomic_sub		
       nop                                             
	subu	$2, 1					
	sc	    $2, 452($20)					
	beqz	$2, 2f					
	.subsection 2					
2:	b	1b					
	.previous					

 # 0 "" 2
 # 110 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
#NO_APP
$L369:
	beq	$19,$0,$L370
#APP
 # 94 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
 # 96 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	1:	ll	    $2, 452($19)		# atomic_sub		
       nop                                             
	subu	$2, 1					
	sc	    $2, 452($19)					
	beqz	$2, 2f					
	.subsection 2					
2:	b	1b					
	.previous					

 # 0 "" 2
 # 110 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
#NO_APP
$L370:
#APP
 # 134 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/irqflags.h" 1
	raw_local_irq_restore	$23
 # 0 "" 2
#NO_APP
	lw	$4,152($sp)
	move	$5,$fp
	.set	noreorder
	.set	nomacro
	jal	rtl_conntrack_drop_check_hook
	move	$6,$4
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L322
	move	$2,$0
	.set	macro
	.set	reorder

$L324:
#APP
 # 134 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/irqflags.h" 1
	raw_local_irq_restore	$23
 # 0 "" 2
#NO_APP
	.set	noreorder
	.set	nomacro
	j	$L322
	li	$2,-1			# 0xffffffff
	.set	macro
	.set	reorder

$L316:
	li	$2,-1			# 0xffffffff
$L322:
	lw	$31,140($sp)
	lw	$fp,136($sp)
	lw	$23,132($sp)
	lw	$22,128($sp)
	lw	$21,124($sp)
	lw	$20,120($sp)
	lw	$19,116($sp)
	lw	$18,112($sp)
	lw	$17,108($sp)
	lw	$16,104($sp)
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,144
	.set	macro
	.set	reorder

	.end	rtk_addNaptConnection
	.size	rtk_addNaptConnection, .-rtk_addNaptConnection
	.local	napt_table_list_max
	.comm	napt_table_list_max,4,4
	.local	path_table_list_max
	.comm	path_table_list_max,4,4
	.local	max_udp_frag_entry
	.comm	max_udp_frag_entry,4,4
	.globl	route_list_inuse
	.section	.bss,"aw",@nobits
	.align	2
	.type	route_list_inuse, @object
	.size	route_list_inuse, 12
route_list_inuse:
	.space	12
	.globl	route_list_free
	.align	2
	.type	route_list_free, @object
	.size	route_list_free, 12
route_list_free:
	.space	12
	.globl	napt_list_inuse
	.align	2
	.type	napt_list_inuse, @object
	.size	napt_list_inuse, 12
napt_list_inuse:
	.space	12
	.globl	napt_list_free
	.align	2
	.type	napt_list_free, @object
	.size	napt_list_free, 12
napt_list_free:
	.space	12
	.globl	table_napt
	.align	2
	.type	table_napt, @object
	.size	table_napt, 4
table_napt:
	.space	4
	.globl	path_list_inuse
	.align	2
	.type	path_list_inuse, @object
	.size	path_list_inuse, 12
path_list_inuse:
	.space	12
	.globl	path_list_free
	.align	2
	.type	path_list_free, @object
	.size	path_list_free, 12
path_list_free:
	.space	12
	.globl	table_path
	.align	2
	.type	table_path, @object
	.size	table_path, 4
table_path:
	.space	4
	.globl	udp_cache_list_free
	.align	2
	.type	udp_cache_list_free, @object
	.size	udp_cache_list_free, 12
udp_cache_list_free:
	.space	12
	.globl	udp_cache_table
	.align	2
	.type	udp_cache_table, @object
	.size	udp_cache_table, 4
udp_cache_table:
	.space	4
	.globl	negative_cache_list_free
	.align	2
	.type	negative_cache_list_free, @object
	.size	negative_cache_list_free, 12
negative_cache_list_free:
	.space	12
	.globl	negative_cache_table
	.align	2
	.type	negative_cache_table, @object
	.size	negative_cache_table, 4
negative_cache_table:
	.space	4
	.ident	"GCC: (GNU) 4.4.5-1.5.5p4"
