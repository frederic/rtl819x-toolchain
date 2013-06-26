	.file	1 "fast_pptp_core.c"
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
	.section	.text.get_fastpptp_lastxmit,"ax",@progbits
	.align	2
	.globl	get_fastpptp_lastxmit
	.set	nomips16
	.ent	get_fastpptp_lastxmit
	.type	get_fastpptp_lastxmit, @function
get_fastpptp_lastxmit:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	lui	$2,%hi(fast_pptp_fw)
	lw	$2,%lo(fast_pptp_fw)($2)
	beq	$2,$0,$L2
	nop

	lui	$2,%hi(pptpAccInfo)
	addiu	$3,$2,%lo(pptpAccInfo)
	lw	$4,4($3)
	li	$3,1			# 0x1
	bne	$4,$3,$L2
	nop

	lw	$2,%lo(pptpAccInfo)($2)
	j	$31
	nop

$L2:
	j	$31
	move	$2,$0

	.set	macro
	.set	reorder
	.end	get_fastpptp_lastxmit
	.size	get_fastpptp_lastxmit, .-get_fastpptp_lastxmit
	.section	.rodata.str1.4,"aMS",@progbits,1
	.align	2
$LC0:
	.ascii	"fast_pptp\000"
	.align	2
$LC1:
	.ascii	"pptp_conn_ck\000"
	.section	.exit.text,"ax",@progbits
	.align	2
	.globl	fast_pptp_exit
	.set	nomips16
	.ent	fast_pptp_exit
	.type	fast_pptp_exit, @function
fast_pptp_exit:
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$16,16($sp)
	lui	$16,%hi(res1)
	lw	$5,%lo(res1)($16)
	beq	$5,$0,$L7
	nop

	lui	$4,%hi($LC0)
	jal	remove_proc_entry
	addiu	$4,$4,%lo($LC0)

	sw	$0,%lo(res1)($16)
$L7:
	lui	$16,%hi(res_check_pptp)
	lw	$5,%lo(res_check_pptp)($16)
	beq	$5,$0,$L9
	nop

	lui	$4,%hi($LC1)
	jal	remove_proc_entry
	addiu	$4,$4,%lo($LC1)

	sw	$0,%lo(res_check_pptp)($16)
$L9:
	lw	$31,20($sp)
	lw	$16,16($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	fast_pptp_exit
	.size	fast_pptp_exit, .-fast_pptp_exit
	.section	.init.text,"ax",@progbits
	.align	2
	.globl	fast_pptp_init
	.set	nomips16
	.ent	fast_pptp_init
	.type	fast_pptp_init, @function
fast_pptp_init:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	lui	$4,%hi($LC0)
	move	$5,$0
	addiu	$4,$4,%lo($LC0)
	jal	create_proc_entry
	move	$6,$0

	lui	$3,%hi(res1)
	beq	$2,$0,$L11
	sw	$2,%lo(res1)($3)

	lui	$3,%hi(ppfw_write_proc)
	addiu	$3,$3,%lo(ppfw_write_proc)
	sw	$3,68($2)
	lui	$3,%hi(ppfw_read_proc)
	addiu	$3,$3,%lo(ppfw_read_proc)
	sw	$3,64($2)
$L11:
	lui	$4,%hi($LC1)
	move	$5,$0
	addiu	$4,$4,%lo($LC1)
	jal	create_proc_entry
	move	$6,$0

	lui	$3,%hi(res_check_pptp)
	beq	$2,$0,$L12
	sw	$2,%lo(res_check_pptp)($3)

	lui	$3,%hi(pptpconn_write_proc)
	addiu	$3,$3,%lo(pptpconn_write_proc)
	sw	$3,68($2)
	lui	$3,%hi(pptpconn_read_proc)
	addiu	$3,$3,%lo(pptpconn_read_proc)
	sw	$3,64($2)
$L12:
	lui	$4,%hi(pptpAccInfo)
	move	$5,$0
	addiu	$4,$4,%lo(pptpAccInfo)
	jal	memset
	li	$6,64			# 0x40

	move	$2,$0
	lw	$31,20($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	fast_pptp_init
	.size	fast_pptp_init, .-fast_pptp_init
	.section	.text.pptpconn_write_proc,"ax",@progbits
	.align	2
	.globl	pptpconn_write_proc
	.set	nomips16
	.ent	pptpconn_write_proc
	.type	pptpconn_write_proc, @function
pptpconn_write_proc:
	.frame	$sp,208,$31		# vars= 200, regs= 1/0, args= 0, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-208
	sw	$31,204($sp)
	sltu	$4,$6,2
	bne	$4,$0,$L15
	beq	$5,$0,$L15
	addu	$4,$5,$6
	or	$3,$4,$5
	lw	$4,24($28)
	or	$2,$3,$6
	and	$2,$2,$4
	bne	$2,$0,$L15
	move	$4,$sp
#APP
 # 781 "net/rtl/fastpath/fast_pptp_core.c" 1
	.set	noreorder
	jal	__copy_user
	.set	noat
	addu	$1, $5, $6
	.set	at
	.set	reorder
 # 0 "" 2
#NO_APP
	.set	noreorder
	.set	nomacro
	bne	$6,$0,$L15
	move	$3,$sp
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L16
	li	$4,10			# 0xa
	.set	macro
	.set	reorder

$L17:
	mflo	$6
	addu	$2,$5,$6
$L16:
	lb	$5,0($3)
	mult	$2,$4
	addiu	$5,$5,-48
	andi	$6,$5,0x00ff
	sltu	$6,$6,10
	.set	noreorder
	.set	nomacro
	bne	$6,$0,$L17
	addiu	$3,$3,1
	.set	macro
	.set	reorder

	lui	$3,%hi(pptp_conn_check)
	sw	$2,%lo(pptp_conn_check)($3)
$L15:
	lw	$31,204($sp)
	li	$2,-14			# 0xfffffff2
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,208
	.set	macro
	.set	reorder

	.end	pptpconn_write_proc
	.size	pptpconn_write_proc, .-pptpconn_write_proc
	.section	.text.ppfw_write_proc,"ax",@progbits
	.align	2
	.globl	ppfw_write_proc
	.set	nomips16
	.ent	ppfw_write_proc
	.type	ppfw_write_proc, @function
ppfw_write_proc:
	.frame	$sp,32,$31		# vars= 0, regs= 4/0, args= 16, gp= 0
	.mask	0x80070000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$18,24($sp)
	sw	$17,20($sp)
	sw	$16,16($sp)
	sltu	$2,$6,2
	move	$17,$6
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L20
	move	$18,$5
	.set	macro
	.set	reorder

$L21 = .
	addiu	$4,$6,32
	.set	noreorder
	.set	nomacro
	jal	__kmalloc
	li	$5,32			# 0x20
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L20
	move	$16,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$18,$0,$L31
	move	$4,$16
	.set	macro
	.set	reorder

	addu	$2,$18,$17
	lw	$3,24($28)
	or	$2,$2,$18
	or	$2,$2,$17
	and	$2,$2,$3
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L31
	move	$5,$18
	.set	macro
	.set	reorder

	move	$4,$16
	move	$6,$17
#APP
 # 807 "net/rtl/fastpath/fast_pptp_core.c" 1
	.set	noreorder
	jal	__copy_user
	.set	noat
	addu	$1, $5, $6
	.set	at
	.set	reorder
 # 0 "" 2
#NO_APP
	.set	noreorder
	.set	nomacro
	bne	$6,$0,$L22
	move	$3,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L23
	li	$5,10			# 0xa
	.set	macro
	.set	reorder

$L24:
	addiu	$3,$3,1
	mflo	$2
	addu	$2,$4,$2
$L23:
	lb	$4,0($3)
	addiu	$4,$4,-48
	andi	$6,$4,0x00ff
	sltu	$6,$6,10
	.set	noreorder
	.set	nomacro
	bne	$6,$0,$L24
	mult	$2,$5
	.set	macro
	.set	reorder

	lui	$4,%hi(pptpAccInfo)
	lui	$18,%hi(fast_pptp_fw)
	addiu	$4,$4,%lo(pptpAccInfo)
	move	$5,$0
	li	$6,64			# 0x40
	.set	noreorder
	.set	nomacro
	jal	memset
	sw	$2,%lo(fast_pptp_fw)($18)
	.set	macro
	.set	reorder

	lw	$3,%lo(fast_pptp_fw)($18)
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L25
	lui	$2,%hi(sync_tx_pptp_gre_seqno_hook)
	.set	macro
	.set	reorder

	lui	$3,%hi(sync_tx_pptp_gre_seqno)
	addiu	$3,$3,%lo(sync_tx_pptp_gre_seqno)
	.set	noreorder
	.set	nomacro
	j	$L26
	sw	$3,%lo(sync_tx_pptp_gre_seqno_hook)($2)
	.set	macro
	.set	reorder

$L25:
	sw	$0,%lo(sync_tx_pptp_gre_seqno_hook)($2)
$L26:
	.set	noreorder
	.set	nomacro
	j	$L29
	move	$4,$16
	.set	macro
	.set	reorder

$L22:
	move	$4,$16
$L31:
	li	$17,-14			# 0xfffffff2
$L29:
	jal	kfree
	.set	noreorder
	.set	nomacro
	j	$L30
	move	$2,$17
	.set	macro
	.set	reorder

$L20:
	li	$17,-14			# 0xfffffff2
	move	$2,$17
$L30:
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

	.end	ppfw_write_proc
	.size	ppfw_write_proc, .-ppfw_write_proc
	.section	.rodata.str1.4
	.align	2
$LC2:
	.ascii	"%d\012\000"
	.section	.text.pptpconn_read_proc,"ax",@progbits
	.align	2
	.globl	pptpconn_read_proc
	.set	nomips16
	.ent	pptpconn_read_proc
	.type	pptpconn_read_proc, @function
pptpconn_read_proc:
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
	lui	$2,%hi(pptp_conn_check)
	move	$16,$6
	move	$17,$5
	lw	$6,%lo(pptp_conn_check)($2)
	lui	$5,%hi($LC2)
	addiu	$5,$5,%lo($LC2)
	move	$18,$4
	jal	sprintf
	sw	$7,16($sp)

	lw	$7,16($sp)
	addu	$3,$7,$16
	slt	$3,$3,$2
	bne	$3,$0,$L33
	li	$4,1			# 0x1

	lw	$3,56($sp)
	sw	$4,0($3)
$L33:
	subu	$2,$2,$16
	addu	$16,$18,$16
	slt	$3,$7,$2
	sw	$16,0($17)
	movz	$7,$2,$3
	slt	$2,$7,0
	lw	$31,36($sp)
	movn	$7,$0,$2
	lw	$18,32($sp)
	move	$2,$7
	lw	$17,28($sp)
	lw	$16,24($sp)
	j	$31
	addiu	$sp,$sp,40

	.set	macro
	.set	reorder
	.end	pptpconn_read_proc
	.size	pptpconn_read_proc, .-pptpconn_read_proc
	.section	.text.ppfw_read_proc,"ax",@progbits
	.align	2
	.globl	ppfw_read_proc
	.set	nomips16
	.ent	ppfw_read_proc
	.type	ppfw_read_proc, @function
ppfw_read_proc:
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
	lui	$2,%hi(fast_pptp_fw)
	move	$16,$6
	move	$17,$5
	lw	$6,%lo(fast_pptp_fw)($2)
	lui	$5,%hi($LC2)
	addiu	$5,$5,%lo($LC2)
	move	$18,$4
	jal	sprintf
	sw	$7,16($sp)

	lw	$7,16($sp)
	addu	$3,$7,$16
	slt	$3,$3,$2
	bne	$3,$0,$L36
	li	$4,1			# 0x1

	lw	$3,56($sp)
	sw	$4,0($3)
$L36:
	subu	$2,$2,$16
	addu	$16,$18,$16
	slt	$3,$7,$2
	sw	$16,0($17)
	movz	$7,$2,$3
	slt	$2,$7,0
	lw	$31,36($sp)
	movn	$7,$0,$2
	lw	$18,32($sp)
	move	$2,$7
	lw	$17,28($sp)
	lw	$16,24($sp)
	j	$31
	addiu	$sp,$sp,40

	.set	macro
	.set	reorder
	.end	ppfw_read_proc
	.size	ppfw_read_proc, .-ppfw_read_proc
	.section	.text.sync_tx_pptp_gre_seqno,"ax",@progbits
	.align	2
	.globl	sync_tx_pptp_gre_seqno
	.set	nomips16
	.ent	sync_tx_pptp_gre_seqno
	.type	sync_tx_pptp_gre_seqno, @function
sync_tx_pptp_gre_seqno:
	.frame	$sp,48,$31		# vars= 8, regs= 5/0, args= 16, gp= 0
	.mask	0x800f0000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-48
	sw	$31,44($sp)
	sw	$19,40($sp)
	sw	$18,36($sp)
	sw	$17,32($sp)
	sw	$16,28($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	move	$19,$4
	.set	macro
	.set	reorder

	move	$18,$2
	lui	$2,%hi(fast_pptp_fw)
	lw	$2,%lo(fast_pptp_fw)($2)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L47
	addiu	$16,$18,14
	.set	macro
	.set	reorder

#APP
 # 685 "net/rtl/fastpath/fast_pptp_core.c" 1
	raw_local_irq_save	$17
 # 0 "" 2
#NO_APP
	lbu	$3,9($16)
	li	$2,47			# 0x2f
	bne	$3,$2,$L40
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_len
	move	$4,$19
	.set	macro
	.set	reorder

	sltu	$2,$2,35
	bne	$2,$0,$L40
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	move	$4,$19
	.set	macro
	.set	reorder

	lbu	$3,14($18)
	andi	$3,$3,0xf
	sll	$3,$3,2
	addiu	$3,$3,14
	addu	$2,$2,$3
	lbu	$3,2($2)
	sb	$3,16($sp)
	lbu	$3,3($2)
	sb	$3,17($sp)
	li	$3,1			# 0x1
	lbu	$4,1($2)
	andi	$4,$4,0x7
	.set	noreorder
	.set	nomacro
	bne	$4,$3,$L40
	li	$3,34827			# 0x880b
	.set	macro
	.set	reorder

	lhu	$4,16($sp)
	bne	$4,$3,$L40
	lbu	$3,0($2)
	andi	$3,$3,0x10
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L42
	move	$5,$0
	.set	macro
	.set	reorder

	lui	$3,%hi(pptpAccInfo)
	addiu	$3,$3,%lo(pptpAccInfo)
	lhu	$5,4($16)
	lhu	$4,46($3)
	.set	noreorder
	.set	nomacro
	beq	$5,$4,$L42
	li	$5,1			# 0x1
	.set	macro
	.set	reorder

	lhu	$5,6($2)
	lhu	$4,18($3)
	.set	noreorder
	.set	nomacro
	bne	$5,$4,$L42
	li	$5,1			# 0x1
	.set	macro
	.set	reorder

	lw	$5,32($3)
	lw	$4,16($16)
	.set	noreorder
	.set	nomacro
	bne	$5,$4,$L43
	lui	$9,%hi(pptpAccInfo+48)
	.set	macro
	.set	reorder

	lw	$4,8($2)
	lw	$7,36($3)
	addiu	$8,$9,%lo(pptpAccInfo+48)
	sb	$4,3($8)
	srl	$13,$4,24
	addiu	$6,$2,8
	srl	$11,$4,16
	addiu	$12,$7,1
	move	$5,$0
	srl	$10,$7,24
	sb	$13,%lo(pptpAccInfo+48)($9)
	srl	$4,$4,8
	sw	$12,36($3)
	sw	$7,20($sp)
	sb	$11,1($8)
	sb	$4,2($8)
	sb	$10,8($2)
	lbu	$3,21($sp)
	sb	$3,1($6)
	lbu	$3,22($sp)
	sb	$3,2($6)
	lbu	$3,23($sp)
	.set	noreorder
	.set	nomacro
	j	$L42
	sb	$3,3($6)
	.set	macro
	.set	reorder

$L43:
	li	$5,1			# 0x1
$L42:
	lb	$3,1($2)
	.set	noreorder
	.set	nomacro
	bgez	$3,$L44
	lui	$3,%hi(pptpAccInfo)
	.set	macro
	.set	reorder

	addiu	$3,$3,%lo(pptpAccInfo)
	lhu	$6,6($2)
	lhu	$4,18($3)
	bne	$6,$4,$L40
	lw	$6,32($3)
	lw	$4,16($16)
	bne	$6,$4,$L40
	lw	$6,40($3)
	addiu	$4,$sp,20
	addiu	$3,$2,12
	srl	$7,$6,24
	sw	$6,20($sp)
	sb	$7,12($2)
	lbu	$2,1($4)
	sb	$2,1($3)
	lbu	$2,2($4)
	sb	$2,2($3)
	lbu	$2,3($4)
	sb	$2,3($3)
$L44:
	.set	noreorder
	.set	nomacro
	bne	$5,$0,$L40
	addiu	$2,$16,16
	.set	macro
	.set	reorder

	lui	$4,%hi(pptpAccInfo)
	addiu	$4,$4,%lo(pptpAccInfo)
	lhu	$3,44($4)
	addiu	$3,$3,1
	andi	$3,$3,0xffff
	sh	$3,44($4)
	sh	$3,4($16)
	sh	$0,10($16)
	lw	$5,14($18)
	lw	$6,4($16)
	lw	$4,8($16)
	srl	$7,$5,22
	addu	$5,$6,$5
	andi	$7,$7,0x3c
	addu	$3,$5,$4
	sltu	$6,$5,$6
	addu	$7,$16,$7
	lw	$5,12($16)
	addu	$6,$3,$6
	sltu	$4,$6,$4
	addu	$6,$6,$5
	addu	$4,$6,$4
	sltu	$3,$4,$5
	addu	$3,$3,$4
$L45:
	lw	$4,0($2)
	addiu	$2,$2,4
	addu	$3,$3,$4
	sltu	$4,$3,$4
	.set	noreorder
	.set	nomacro
	bne	$2,$7,$L45
	addu	$3,$4,$3
	.set	macro
	.set	reorder

	move	$2,$3
#APP
 # 83 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/checksum.h" 1
		.set	push		# csum_fold
	.set	noat		
	sll	$1, $2, 16	
	addu	$2, $1		
	sltu	$1, $2, $1	
	srl	$2, $2, 16	
	addu	$2, $1		
	xori	$2, 0xffff	
	.set	pop
 # 0 "" 2
#NO_APP
	sh	$2,10($16)
$L40:
#APP
 # 134 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/irqflags.h" 1
	raw_local_irq_restore	$17
 # 0 "" 2
#NO_APP
$L47:
	lw	$31,44($sp)
	lw	$19,40($sp)
	lw	$18,36($sp)
	lw	$17,32($sp)
	lw	$16,28($sp)
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,48
	.set	macro
	.set	reorder

	.end	sync_tx_pptp_gre_seqno
	.size	sync_tx_pptp_gre_seqno, .-sync_tx_pptp_gre_seqno
	.section	.text.fast_pptp_sync_rx_seq,"ax",@progbits
	.align	2
	.globl	fast_pptp_sync_rx_seq
	.set	nomips16
	.ent	fast_pptp_sync_rx_seq
	.type	fast_pptp_sync_rx_seq, @function
fast_pptp_sync_rx_seq:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	jal	rtl_ip_hdr
	nop

	li	$4,1			# 0x1
	addiu	$3,$2,20
	lbu	$5,1($3)
	andi	$5,$5,0x7
	bne	$5,$4,$L52
	li	$4,34827			# 0x880b

	lhu	$5,2($3)
	bne	$5,$4,$L52
	nop

	lbu	$2,20($2)
	andi	$2,$2,0x10
	beq	$2,$0,$L51
	lui	$2,%hi(pptpAccInfo)

	addiu	$2,$2,%lo(pptpAccInfo)
	lw	$4,52($2)
	addiu	$5,$4,1
	sw	$4,8($3)
	sw	$5,52($2)
$L51:
	lb	$2,1($3)
	bgez	$2,$L52
	lui	$2,%hi(pptpAccInfo+48)

	lw	$2,%lo(pptpAccInfo+48)($2)
	sw	$2,12($3)
$L52:
	lw	$31,20($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	fast_pptp_sync_rx_seq
	.size	fast_pptp_sync_rx_seq, .-fast_pptp_sync_rx_seq
	.section	.text.is_pptp_device,"ax",@progbits
	.align	2
	.globl	is_pptp_device
	.set	nomips16
	.ent	is_pptp_device
	.type	is_pptp_device, @function
is_pptp_device:
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$16,16($sp)
	lui	$3,%hi(pptpAccInfo+56)
	move	$16,$4
	lw	$4,%lo(pptpAccInfo+56)($3)
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L55
	move	$2,$0
	.set	macro
	.set	reorder

	jal	rtl_get_ppp_dev_name
#APP
 # 75 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/string.h" 1
	.set	noreorder
	.set	noat
	lbu	$3,($2)
1:	lbu	$1,($16)
	addiu	$2,1
	bne	$1,$3,2f
	addiu	$16,1
	bnez	$3,1b
	lbu	$3,($2)
	move	$3,$1
2:	subu	$3,$1
3:	.set	at
	.set	reorder
 # 0 "" 2
#NO_APP
	sltu	$2,$3,1
$L55:
	lw	$31,20($sp)
	lw	$16,16($sp)
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,24
	.set	macro
	.set	reorder

	.end	is_pptp_device
	.size	is_pptp_device, .-is_pptp_device
	.section	.rodata.str1.4
	.align	2
$LC3:
	.ascii	"ppp0\000"
	.align	2
$LC4:
	.ascii	"RTL\000"
	.section	.text.fast_pptp_to_wan,"ax",@progbits
	.align	2
	.globl	fast_pptp_to_wan
	.set	nomips16
	.ent	fast_pptp_to_wan
	.type	fast_pptp_to_wan, @function
fast_pptp_to_wan:
	.frame	$sp,88,$31		# vars= 48, regs= 5/0, args= 16, gp= 0
	.mask	0x800f0000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-88
	sw	$31,84($sp)
	sw	$19,80($sp)
	sw	$18,76($sp)
	sw	$17,72($sp)
	sw	$16,68($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_ip_hdr
	move	$18,$4
	.set	macro
	.set	reorder

	move	$4,$18
	lw	$17,12($2)
	li	$2,-16777216			# 0xff000000
	.set	noreorder
	.set	nomacro
	jal	rtl_ip_hdr
	and	$17,$17,$2
	.set	macro
	.set	reorder

	lw	$16,12($2)
	.set	noreorder
	.set	nomacro
	jal	rtl_ip_hdr
	move	$4,$18
	.set	macro
	.set	reorder

	lw	$3,12($2)
	li	$2,167772160			# 0xa000000
	.set	noreorder
	.set	nomacro
	bne	$17,$2,$L58
	li	$2,16711680			# 0xff0000
	.set	macro
	.set	reorder

	and	$16,$16,$2
	li	$2,4194304			# 0x400000
	.set	noreorder
	.set	nomacro
	bne	$16,$2,$L58
	andi	$3,$3,0xff00
	.set	macro
	.set	reorder

	li	$2,16384			# 0x4000
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L68
	lui	$16,%hi(pptpAccInfo)
	.set	macro
	.set	reorder

	lui	$2,%hi(pptp_conn_check)
	lw	$3,%lo(pptp_conn_check)($2)
	li	$2,3			# 0x3
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L62
	move	$3,$0
	.set	macro
	.set	reorder

$L58:
	lui	$16,%hi(pptpAccInfo)
$L68:
	addiu	$16,$16,%lo(pptpAccInfo)
	lw	$4,56($16)
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L62
	move	$3,$0
	.set	macro
	.set	reorder

	jal	rtl_get_ppp_dev_name
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L62
	move	$3,$0
	.set	macro
	.set	reorder

	lw	$4,56($16)
	jal	rtl_get_ppp_dev_name
	lui	$3,%hi($LC3)
	addiu	$4,$3,%lo($LC3)
#APP
 # 75 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/string.h" 1
	.set	noreorder
	.set	noat
	lbu	$3,($2)
1:	lbu	$1,($4)
	addiu	$2,1
	bne	$1,$3,2f
	addiu	$4,1
	bnez	$3,1b
	lbu	$3,($2)
	move	$3,$1
2:	subu	$3,$1
3:	.set	at
	.set	reorder
 # 0 "" 2
#NO_APP
	bne	$3,$0,$L59
	lw	$2,4($16)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L69
	lui	$2,%hi(pptpAccInfo+56)
	.set	macro
	.set	reorder

	lw	$4,56($16)
	beq	$4,$0,$L69
	jal	rtl_get_ppp_dev_priv
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L60
	lui	$2,%hi(pptpAccInfo+56)
	.set	macro
	.set	reorder

	lw	$4,56($16)
	jal	rtl_get_ppp_dev_priv
	move	$4,$2
	.set	noreorder
	.set	nomacro
	jal	rtl_ppp_vj_check
	move	$17,$2
	.set	macro
	.set	reorder

	li	$3,1			# 0x1
	.set	noreorder
	.set	nomacro
	beq	$2,$3,$L62
	move	$3,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_ip_hdr
	move	$4,$18
	.set	macro
	.set	reorder

	lbu	$19,1($2)
	.set	noreorder
	.set	nomacro
	jal	rtl_get_ppp_xmit_pending
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L59
	lui	$5,%hi($LC4)
	.set	macro
	.set	reorder

	li	$6,3			# 0x3
	move	$4,$18
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_cb
	addiu	$5,$5,%lo($LC4)
	.set	macro
	.set	reorder

	lw	$5,56($16)
	.set	noreorder
	.set	nomacro
	jal	rtl_ppp_start_xmit
	move	$4,$18
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_ppp_xmit_pending
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L62
	li	$3,1			# 0x1
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_ppp_xmit_pending
	move	$4,$17
	.set	macro
	.set	reorder

	move	$4,$17
	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	rtl_set_ppp_xmit_pending
	move	$16,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_skb_headroom
	move	$4,$16
	.set	macro
	.set	reorder

	sltu	$2,$2,50
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L70
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_skb_cloned
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L70
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_skb_shared
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L71
	li	$5,50			# 0x32
	.set	macro
	.set	reorder

	move	$4,$16
$L70:
	.set	noreorder
	.set	nomacro
	jal	skb_realloc_headroom
	li	$5,50			# 0x32
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L59
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	consume_skb
	sw	$2,56($sp)
	.set	macro
	.set	reorder

	lw	$2,56($sp)
	move	$16,$2
	li	$5,50			# 0x32
$L71:
	.set	noreorder
	.set	nomacro
	jal	skb_push
	move	$4,$16
	.set	macro
	.set	reorder

	lui	$5,%hi(pptpAccInfo+24)
	move	$4,$2
	li	$6,6			# 0x6
	addiu	$5,$5,%lo(pptpAccInfo+24)
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$17,$2
	.set	macro
	.set	reorder

	lui	$5,%hi(pptpAccInfo+12)
	addiu	$4,$17,6
	addiu	$5,$5,%lo(pptpAccInfo+12)
	.set	noreorder
	.set	nomacro
	jal	memcpy
	li	$6,6			# 0x6
	.set	macro
	.set	reorder

	li	$2,8			# 0x8
	move	$4,$16
	sb	$2,12($17)
	li	$6,268369920			# 0xfff0000
	sb	$0,13($17)
	ori	$6,$6,0xffff
	lui	$17,%hi(pptpAccInfo)
	lw	$2,32($sp)
	addiu	$17,$17,%lo(pptpAccInfo)
	li	$7,83886080			# 0x5000000
	move	$5,$0
	and	$6,$2,$6
	li	$2,1073741824			# 0x40000000
	or	$6,$6,$2
	li	$2,-251723776			# 0xf0ff0000
	ori	$2,$2,0xffff
	lw	$3,32($17)
	and	$6,$6,$2
	lw	$2,20($17)
	or	$6,$6,$7
	sw	$3,48($sp)
	sw	$6,32($sp)
	li	$6,16384			# 0x4000
	sh	$6,38($sp)
	li	$6,47			# 0x2f
	sw	$2,44($sp)
	li	$2,64			# 0x40
	sb	$6,41($sp)
	sb	$2,40($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_ip_summed
	sb	$19,33($sp)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_len
	move	$4,$16
	.set	macro
	.set	reorder

	addiu	$4,$sp,32
	lhu	$3,44($17)
	addiu	$2,$2,-14
	sh	$2,34($sp)
	addiu	$5,$sp,48
	addiu	$3,$3,1
	andi	$3,$3,0xffff
	sh	$0,42($sp)
	sh	$3,36($sp)
	lw	$8,4($4)
	lw	$7,32($sp)
	lw	$6,8($4)
	srl	$9,$7,22
	addu	$7,$8,$7
	addu	$2,$7,$6
	sltu	$8,$7,$8
	lw	$7,44($sp)
	addu	$8,$2,$8
	andi	$9,$9,0x3c
	sltu	$6,$8,$6
	addu	$4,$4,$9
	addu	$8,$8,$7
	addu	$6,$8,$6
	sltu	$2,$6,$7
	sh	$3,44($17)
	addu	$2,$2,$6
$L65:
	lw	$6,0($5)
	addiu	$5,$5,4
	addu	$2,$2,$6
	sltu	$6,$2,$6
	.set	noreorder
	.set	nomacro
	bne	$5,$4,$L65
	addu	$2,$6,$2
	.set	macro
	.set	reorder

	lui	$18,%hi(pptpAccInfo)
	move	$4,$16
	addiu	$17,$18,%lo(pptpAccInfo)
	sh	$3,46($17)
#APP
 # 83 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/checksum.h" 1
		.set	push		# csum_fold
	.set	noat		
	sll	$1, $2, 16	
	addu	$2, $1		
	sltu	$1, $2, $1	
	srl	$2, $2, 16	
	addu	$2, $1		
	xori	$2, 0xffff	
	.set	pop
 # 0 "" 2
#NO_APP
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	sh	$2,42($sp)
	.set	macro
	.set	reorder

	addiu	$5,$sp,32
	li	$6,20			# 0x14
	.set	noreorder
	.set	nomacro
	jal	memcpy
	addiu	$4,$2,14
	.set	macro
	.set	reorder

	li	$2,48			# 0x30
	move	$4,$16
	sb	$2,16($sp)
	li	$2,-127			# 0xffffff81
	sb	$2,17($sp)
	li	$2,-30709			# 0xffff880b
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_len
	sh	$2,18($sp)
	.set	macro
	.set	reorder

	move	$4,$16
	lw	$3,36($17)
	addiu	$2,$2,-50
	lhu	$7,18($17)
	addiu	$6,$3,1
	lw	$5,40($17)
	sw	$6,36($17)
	sw	$3,24($sp)
	sh	$7,22($sp)
	sw	$5,28($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	sh	$2,20($sp)
	.set	macro
	.set	reorder

	li	$6,16			# 0x10
	addiu	$4,$2,34
	.set	noreorder
	.set	nomacro
	jal	memcpy
	addiu	$5,$sp,16
	.set	macro
	.set	reorder

	lui	$2,%hi(jiffies)
	move	$4,$16
	lw	$2,%lo(jiffies)($2)
	lw	$5,8($17)
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_dev
	sw	$2,%lo(pptpAccInfo)($18)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	dev_queue_xmit
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L62
	li	$3,1			# 0x1
	.set	macro
	.set	reorder

$L60:
$L69:
	lw	$4,%lo(pptpAccInfo+56)($2)
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L62
	move	$3,$0
	.set	macro
	.set	reorder

	jal	rtl_get_ppp_dev_priv
$L59:
	move	$3,$0
$L62:
	lw	$31,84($sp)
	move	$2,$3
	lw	$19,80($sp)
	lw	$18,76($sp)
	lw	$17,72($sp)
	lw	$16,68($sp)
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,88
	.set	macro
	.set	reorder

	.end	fast_pptp_to_wan
	.size	fast_pptp_to_wan, .-fast_pptp_to_wan
	.section	.text.fast_pptp_filter,"ax",@progbits
	.align	2
	.globl	fast_pptp_filter
	.set	nomips16
	.ent	fast_pptp_filter
	.type	fast_pptp_filter, @function
fast_pptp_filter:
	.frame	$sp,40,$31		# vars= 0, regs= 5/0, args= 16, gp= 0
	.mask	0x800f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-40
	sw	$31,36($sp)
	sw	$19,32($sp)
	sw	$18,28($sp)
	sw	$17,24($sp)
	sw	$16,20($sp)
	jal	rtl_ip_hdr
	move	$17,$4

	move	$4,$17
	jal	rtl_get_skb_len
	move	$19,$2

	sltu	$2,$2,56
	bne	$2,$0,$L77
	li	$2,6			# 0x6

	lbu	$3,0($19)
	lbu	$4,9($19)
	bne	$4,$2,$L77
	andi	$3,$3,0xf

	sll	$3,$3,2
	li	$2,1723			# 0x6bb
	addu	$3,$19,$3
	lhu	$4,0($3)
	bne	$4,$2,$L77
	nop

	lw	$2,12($3)
	srl	$2,$2,28
	sll	$2,$2,2
	addu	$2,$3,$2
	li	$3,1			# 0x1
	lhu	$4,2($2)
	bne	$4,$3,$L77
	li	$6,8			# 0x8

	lhu	$3,8($2)
	bne	$3,$6,$L74
	addiu	$5,$2,8

	addiu	$3,$2,12
	lbu	$5,4($3)
	bne	$5,$4,$L77
	lui	$4,%hi(pptpAccInfo)

	addiu	$4,$4,%lo(pptpAccInfo)
	lw	$5,36($4)
	beq	$5,$0,$L78
	lui	$5,%hi(pptp_tcp_finished)

	lw	$5,20($4)
	beq	$5,$0,$L78
	lui	$5,%hi(pptp_tcp_finished)

	lw	$5,32($4)
	beq	$5,$0,$L78
	lui	$5,%hi(pptp_tcp_finished)

	lhu	$5,18($4)
	lhu	$4,12($2)
	bne	$5,$4,$L77
	lui	$4,%hi(pptpAccInfo+30)

	lhu	$5,%lo(pptpAccInfo+30)($4)
	lhu	$4,2($3)
	bne	$5,$4,$L77
	lui	$5,%hi(pptp_tcp_finished)

$L78:
	li	$18,1			# 0x1
	move	$4,$17
	sw	$18,%lo(pptp_tcp_finished)($5)
	lui	$16,%hi(pptpAccInfo)
	lhu	$2,12($2)
	addiu	$16,$16,%lo(pptpAccInfo)
	sh	$2,18($16)
	lhu	$2,2($3)
	sh	$2,30($16)
	lw	$2,16($19)
	sw	$2,20($16)
	lw	$2,12($19)
	sw	$18,36($16)
	jal	rtl_get_skb_dev
	sw	$2,32($16)

	move	$4,$17
	jal	rtl_get_skb_data
	sw	$2,8($16)

	lui	$4,%hi(pptpAccInfo+24)
	addiu	$5,$2,-8
	li	$6,6			# 0x6
	jal	memcpy
	addiu	$4,$4,%lo(pptpAccInfo+24)

	jal	rtl_get_skb_data
	move	$4,$17

	lui	$4,%hi(pptpAccInfo+12)
	addiu	$4,$4,%lo(pptpAccInfo+12)
	addiu	$5,$2,-14
	jal	memcpy
	li	$6,6			# 0x6

	j	$L77
	sw	$18,4($16)

$L74:
	li	$2,13			# 0xd
	bne	$3,$2,$L77
	lui	$2,%hi(pptpAccInfo)

	addiu	$2,$2,%lo(pptpAccInfo)
	lw	$3,36($2)
	beq	$3,$0,$L76
	nop

	lw	$3,20($2)
	beq	$3,$0,$L76
	nop

	lw	$3,32($2)
	beq	$3,$0,$L76
	nop

	lhu	$3,18($2)
	lhu	$2,4($5)
	bne	$3,$2,$L77
	nop

$L76:
	lui	$2,%hi(pptp_tcp_finished)
	sw	$0,%lo(pptp_tcp_finished)($2)
	lui	$2,%hi(pptpAccInfo+4)
	sw	$0,%lo(pptpAccInfo+4)($2)
$L77:
	lw	$31,36($sp)
	lw	$19,32($sp)
	lw	$18,28($sp)
	lw	$17,24($sp)
	lw	$16,20($sp)
	j	$31
	addiu	$sp,$sp,40

	.set	macro
	.set	reorder
	.end	fast_pptp_filter
	.size	fast_pptp_filter, .-fast_pptp_filter
	.section	.text.Check_GRE_rx_net_device,"ax",@progbits
	.align	2
	.globl	Check_GRE_rx_net_device
	.set	nomips16
	.ent	Check_GRE_rx_net_device
	.type	Check_GRE_rx_net_device, @function
Check_GRE_rx_net_device:
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
	lui	$16,%hi(pptpAccInfo)
	jal	rtl_ip_hdr
	move	$18,$4

	addiu	$16,$16,%lo(pptpAccInfo)
	move	$4,$18
	jal	rtl_get_skb_data
	move	$17,$2

	lbu	$20,0($17)
	move	$19,$2
	lw	$17,8($16)
	beq	$17,$0,$L80
	andi	$20,$20,0xf

	jal	rtl_get_skb_dev
	move	$4,$18

	bne	$17,$2,$L80
	sll	$20,$20,2

	lhu	$2,30($16)
	addu	$19,$19,$20
	lhu	$3,6($19)
	xor	$2,$3,$2
	j	$L81
	sltu	$2,$2,1

$L80:
	move	$2,$0
$L81:
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
	.end	Check_GRE_rx_net_device
	.size	Check_GRE_rx_net_device, .-Check_GRE_rx_net_device
	.section	.text.fast_pptp_to_lan,"ax",@progbits
	.align	2
	.globl	fast_pptp_to_lan
	.set	nomips16
	.ent	fast_pptp_to_lan
	.type	fast_pptp_to_lan, @function
fast_pptp_to_lan:
	.frame	$sp,48,$31		# vars= 8, regs= 5/0, args= 16, gp= 0
	.mask	0x800f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-48
	sw	$31,44($sp)
	sw	$19,40($sp)
	sw	$18,36($sp)
	sw	$17,32($sp)
	sw	$16,28($sp)
	lui	$18,%hi(pptpAccInfo)
	move	$17,$4
	lw	$16,0($4)
	addiu	$18,$18,%lo(pptpAccInfo)
	jal	rtl_get_skb_dev
	move	$4,$16

	lw	$3,8($18)
	bne	$3,$2,$L95
	move	$2,$0

	jal	rtl_ip_hdr
	move	$4,$16

	lbu	$3,9($2)
	li	$2,47			# 0x2f
	bne	$3,$2,$L95
	move	$2,$0

	jal	rtl_get_skb_len
	move	$4,$16

	sltu	$2,$2,20
	bne	$2,$0,$L95
	move	$2,$0

	jal	rtl_ip_hdr
	move	$4,$16

	move	$4,$16
	jal	rtl_get_skb_data
	move	$19,$2

	lbu	$3,0($19)
	andi	$3,$3,0xf
	sll	$3,$3,2
	addu	$3,$2,$3
	li	$2,1			# 0x1
	lbu	$4,1($3)
	andi	$4,$4,0x7
	bne	$4,$2,$L95
	move	$2,$0

	lhu	$4,2($3)
	li	$2,34827			# 0x880b
	bne	$4,$2,$L95
	move	$2,$0

	lhu	$4,6($3)
	lhu	$2,30($18)
	bne	$4,$2,$L84
	nop

	lbu	$4,0($3)
	andi	$4,$4,0x10
	beq	$4,$0,$L86
	li	$2,8			# 0x8

	lw	$4,8($3)
	li	$2,12			# 0xc
	sw	$4,40($18)
$L86:
	lb	$6,1($3)
	addiu	$5,$2,4
	lhu	$4,4($3)
	slt	$6,$6,0
	beq	$4,$0,$L84
	movn	$2,$5,$6

	addu	$3,$3,$2
	lbu	$4,0($3)
	bne	$4,$0,$L88
	nop

	addiu	$3,$3,1
	j	$L89
	li	$4,1			# 0x1

$L88:
	li	$5,255			# 0xff
	bne	$4,$5,$L89
	move	$4,$0

	lbu	$5,1($3)
	li	$4,3			# 0x3
	bne	$5,$4,$L90
	move	$4,$0

	addiu	$3,$3,2
	lbu	$5,0($3)
	bne	$5,$0,$L89
	li	$4,2			# 0x2

	addiu	$3,$3,1
	li	$4,3			# 0x3
$L90:
$L89:
	lbu	$18,0($3)
	li	$3,33			# 0x21
	beq	$18,$3,$L92
	li	$3,253			# 0xfd

	bne	$18,$3,$L84
	nop

$L92:
	lbu	$5,0($19)
	andi	$5,$5,0xf
	sll	$5,$5,2
	addiu	$5,$5,1
	addu	$5,$5,$2
	li	$2,33			# 0x21
	beq	$18,$2,$L93
	addu	$5,$5,$4

	lui	$19,%hi(pptpAccInfo)
	addiu	$19,$19,%lo(pptpAccInfo)
	lw	$4,56($19)
	beq	$4,$0,$L95
	move	$2,$0

	jal	rtl_get_ppp_dev_priv
	sw	$5,16($sp)

	lw	$5,16($sp)
	beq	$2,$0,$L84
	addiu	$5,$5,-2

	jal	skb_pull
	move	$4,$16

	jal	rtl_get_skb_data
	move	$4,$16

	sb	$18,1($2)
	sb	$0,0($2)
	lw	$4,56($19)
	jal	rtl_get_ppp_dev_priv
	nop

	move	$5,$16
	li	$6,1			# 0x1
	jal	rtl_ppp_receive_nonmp_frame
	move	$4,$2

	move	$16,$2
	beq	$16,$0,$L95
	li	$2,-2			# 0xfffffffe

	sw	$16,0($17)
	j	$L96
	li	$18,1			# 0x1

$L93:
	move	$4,$16
	jal	skb_pull
	move	$18,$0

$L96:
	jal	rtl_get_skb_data
	move	$4,$16

	andi	$17,$2,0x3
	beq	$17,$0,$L100
	move	$4,$16

	jal	rtl_skb_headroom
	move	$4,$16

	sltu	$2,$2,$17
	bne	$2,$0,$L97
	nop

	jal	rtl_get_skb_data
	move	$4,$16

	move	$4,$16
	jal	rtl_get_skb_data
	move	$19,$2

	move	$4,$16
	jal	rtl_get_skb_len
	sw	$2,16($sp)

	subu	$4,$19,$17
	lw	$5,16($sp)
	jal	memmove
	move	$6,$2

	move	$4,$16
	li	$6,1			# 0x1
	jal	rtl_set_skb_data
	move	$5,$17

	move	$4,$16
	li	$6,1			# 0x1
	jal	rtl_set_skb_tail
	move	$5,$17

$L97:
	move	$4,$16
$L100:
	jal	rtl_skb_reset_network_header
	lui	$17,%hi(pptpAccInfo)

	addiu	$17,$17,%lo(pptpAccInfo)
	jal	rtl_skb_reset_transport_header
	move	$4,$16

	lw	$4,56($17)
	beq	$4,$0,$L95
	move	$2,$0

	bne	$18,$0,$L101
	lui	$2,%hi(pptpAccInfo+56)

	jal	rtl_get_ppp_dev_priv
	nop

	beq	$2,$0,$L98
	nop

	lw	$4,56($17)
	jal	rtl_get_ppp_dev_priv
	nop

	move	$4,$16
	jal	rtl_get_skb_len
	move	$17,$2

	move	$5,$0
	move	$4,$17
	jal	rtl_inc_ppp_stats
	addiu	$6,$2,-2

$L98:
	lui	$2,%hi(pptpAccInfo+56)
$L101:
	lw	$5,%lo(pptpAccInfo+56)($2)
	jal	rtl_set_skb_dev
	move	$4,$16

	j	$L95
	li	$2,1			# 0x1

$L84:
	move	$2,$0
$L95:
	lw	$31,44($sp)
	lw	$19,40($sp)
	lw	$18,36($sp)
	lw	$17,32($sp)
	lw	$16,28($sp)
	j	$31
	addiu	$sp,$sp,48

	.set	macro
	.set	reorder
	.end	fast_pptp_to_lan
	.size	fast_pptp_to_lan, .-fast_pptp_to_lan
	.section	.text.set_pptp_device,"ax",@progbits
	.align	2
	.globl	set_pptp_device
	.set	nomips16
	.ent	set_pptp_device
	.type	set_pptp_device, @function
set_pptp_device:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	lui	$2,%hi(fast_pptp_fw)
	lw	$2,%lo(fast_pptp_fw)($2)
	beq	$2,$0,$L104
	nop

	jal	rtl_get_dev_by_name
	nop

	lui	$3,%hi(pptpAccInfo+56)
	sw	$2,%lo(pptpAccInfo+56)($3)
$L104:
	lw	$31,20($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	set_pptp_device
	.size	set_pptp_device, .-set_pptp_device
	.globl	pptp_tcp_finished
	.section	.bss,"aw",@nobits
	.align	2
	.type	pptp_tcp_finished, @object
	.size	pptp_tcp_finished, 4
pptp_tcp_finished:
	.space	4
	.globl	fast_pptp_fw
	.align	2
	.type	fast_pptp_fw, @object
	.size	fast_pptp_fw, 4
fast_pptp_fw:
	.space	4
	.local	res1
	.comm	res1,4,4
	.local	res_check_pptp
	.comm	res_check_pptp,4,4
	.local	pptp_conn_check
	.comm	pptp_conn_check,4,4
	.local	pptpAccInfo
	.comm	pptpAccInfo,64,4
	.ident	"GCC: (GNU) 4.4.5-1.5.5p4"
