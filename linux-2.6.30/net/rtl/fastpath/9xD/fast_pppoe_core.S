	.file	1 "fast_pppoe_core.c"
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
	.section	.text.find_pppoe_info,"ax",@progbits
	.align	2
	.set	nomips16
	.ent	find_pppoe_info
	.type	find_pppoe_info, @function
find_pppoe_info:
	.frame	$sp,64,$31		# vars= 8, regs= 10/0, args= 16, gp= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-64
	sw	$31,60($sp)
	sw	$fp,56($sp)
	sw	$23,52($sp)
	sw	$22,48($sp)
	sw	$21,44($sp)
	sw	$20,40($sp)
	sw	$19,36($sp)
	sw	$18,32($sp)
	sw	$17,28($sp)
	sw	$16,24($sp)
	lui	$16,%hi(fast_pppoe_info+54)
	andi	$6,$6,0xffff
	lui	$22,%hi(fast_pppoe_info)
	move	$20,$4
	addiu	$16,$16,%lo(fast_pppoe_info+54)
	lw	$19,80($sp)
	move	$18,$5
	addiu	$22,$22,%lo(fast_pppoe_info)
	lw	$17,84($sp)
	move	$21,$0
	sw	$6,16($sp)
	li	$23,88			# 0x58
$L10:
	lhu	$2,0($16)
	beq	$2,$0,$L2
	.set	noreorder
	.set	nomacro
	beq	$20,$0,$L3
	mult	$21,$23
	.set	macro
	.set	reorder

	move	$4,$20
	mflo	$2
	addiu	$3,$2,16
	addu	$3,$22,$3
#APP
 # 75 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/string.h" 1
	.set	noreorder
	.set	noat
	lbu	$5,($3)
1:	lbu	$1,($4)
	addiu	$3,1
	bne	$1,$5,2f
	addiu	$4,1
	bnez	$5,1b
	lbu	$5,($3)
	move	$5,$1
2:	subu	$5,$1
3:	.set	at
	.set	reorder
 # 0 "" 2
#NO_APP
	.set	noreorder
	.set	nomacro
	beq	$5,$0,$L4
	addu	$2,$22,$2
	.set	macro
	.set	reorder

$L3:
	.set	noreorder
	.set	nomacro
	beq	$18,$0,$L5
	mult	$21,$23
	.set	macro
	.set	reorder

	move	$4,$18
	mflo	$2
	addu	$2,$22,$2
	move	$3,$2
#APP
 # 75 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/string.h" 1
	.set	noreorder
	.set	noat
	lbu	$5,($3)
1:	lbu	$1,($4)
	addiu	$3,1
	bne	$1,$5,2f
	addiu	$4,1
	bnez	$5,1b
	lbu	$5,($3)
	move	$5,$1
2:	subu	$5,$1
3:	.set	at
	.set	reorder
 # 0 "" 2
#NO_APP
	beq	$5,$0,$L4
$L5:
	lw	$2,88($sp)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L6
	mult	$21,$23
	.set	macro
	.set	reorder

	move	$5,$2
	li	$6,6			# 0x6
	sw	$7,20($sp)
	mflo	$fp
	addu	$4,$22,$fp
	.set	noreorder
	.set	nomacro
	jal	memcmp
	addiu	$4,$4,38
	.set	macro
	.set	reorder

	lw	$7,20($sp)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L4
	addu	$2,$22,$fp
	.set	macro
	.set	reorder

$L6:
	.set	noreorder
	.set	nomacro
	beq	$17,$0,$L7
	mult	$21,$23
	.set	macro
	.set	reorder

	move	$5,$17
	li	$6,6			# 0x6
	sw	$7,20($sp)
	mflo	$fp
	addiu	$4,$fp,32
	.set	noreorder
	.set	nomacro
	jal	memcmp
	addu	$4,$22,$4
	.set	macro
	.set	reorder

	lw	$7,20($sp)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L4
	addu	$2,$22,$fp
	.set	macro
	.set	reorder

$L7:
	beq	$19,$0,$L8
	lw	$2,-6($16)
	.set	noreorder
	.set	nomacro
	beq	$2,$19,$L16
	li	$2,88			# 0x58
	.set	macro
	.set	reorder

$L8:
	beq	$7,$0,$L9
	lw	$2,-10($16)
	.set	noreorder
	.set	nomacro
	beq	$2,$7,$L16
	li	$2,88			# 0x58
	.set	macro
	.set	reorder

$L9:
	lw	$2,16($sp)
	beq	$2,$0,$L2
	lhu	$2,-2($16)
	lw	$3,16($sp)
	.set	noreorder
	.set	nomacro
	bne	$2,$3,$L17
	addiu	$21,$21,1
	.set	macro
	.set	reorder

	addiu	$21,$21,-1
	li	$2,88			# 0x58
$L16:
	mult	$21,$2
	lui	$2,%hi(fast_pppoe_info)
	addiu	$2,$2,%lo(fast_pppoe_info)
	mflo	$21
	.set	noreorder
	.set	nomacro
	j	$L4
	addu	$2,$2,$21
	.set	macro
	.set	reorder

$L2:
	addiu	$21,$21,1
$L17:
	li	$2,16			# 0x10
	.set	noreorder
	.set	nomacro
	bne	$21,$2,$L10
	addiu	$16,$16,88
	.set	macro
	.set	reorder

	move	$2,$0
$L4:
	lw	$31,60($sp)
	lw	$fp,56($sp)
	lw	$23,52($sp)
	lw	$22,48($sp)
	lw	$21,44($sp)
	lw	$20,40($sp)
	lw	$19,36($sp)
	lw	$18,32($sp)
	lw	$17,28($sp)
	lw	$16,24($sp)
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,64
	.set	macro
	.set	reorder

	.end	find_pppoe_info
	.size	find_pppoe_info, .-find_pppoe_info
	.section	.rodata.str1.4,"aMS",@progbits,1
	.align	2
$LC0:
	.ascii	"ppp\000"
	.section	.text.fast_pppoe_xmit,"ax",@progbits
	.align	2
	.globl	fast_pppoe_xmit
	.set	nomips16
	.ent	fast_pppoe_xmit
	.type	fast_pppoe_xmit, @function
fast_pppoe_xmit:
	.frame	$sp,56,$31		# vars= 0, regs= 5/0, args= 32, gp= 0
	.mask	0x800f0000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-56
	sw	$31,52($sp)
	sw	$19,48($sp)
	sw	$18,44($sp)
	sw	$17,40($sp)
	sw	$16,36($sp)
	lui	$2,%hi(fast_pppoe_fw)
	lw	$2,%lo(fast_pppoe_fw)($2)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L19
	move	$16,$4
	.set	macro
	.set	reorder

	lui	$2,%hi(gQosEnabled)
	lw	$2,%lo(gQosEnabled)($2)
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L20
	move	$2,$0
	.set	macro
	.set	reorder

	jal	rtl_get_skb_dev
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L20
	move	$2,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_dev
	move	$4,$16
	.set	macro
	.set	reorder

	lui	$3,%hi($LC0)
	addiu	$4,$3,%lo($LC0)
	li	$5,3			# 0x3
#APP
 # 106 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/string.h" 1
	.set	noreorder
	.set	noat
1:	lbu	$3,($2)
	beqz	$5,2f
	lbu	$1,($4)
	subu	$5,1
	bne	$1,$3,3f
	addiu	$2,1
	bnez	$3,1b
	addiu	$4,1
2:
	move	$3,$1
3:	subu	$3,$1
	.set	at
	.set	reorder
 # 0 "" 2
#NO_APP
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L20
	move	$2,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_skb_headroom
	move	$4,$16
	.set	macro
	.set	reorder

	sltu	$2,$2,22
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L20
	move	$2,$0
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
	bne	$2,$0,$L20
	move	$2,$0
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
	bne	$2,$0,$L20
	move	$2,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_dev
	move	$4,$16
	.set	macro
	.set	reorder

	move	$5,$0
	move	$4,$2
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

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L19
	move	$17,$2
	.set	macro
	.set	reorder

	lb	$2,16($2)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L20
	move	$2,$0
	.set	macro
	.set	reorder

	lb	$2,0($17)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L20
	move	$2,$0
	.set	macro
	.set	reorder

	lhu	$2,52($17)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L20
	move	$2,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_dev
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_len
	move	$4,$16
	.set	macro
	.set	reorder

	move	$4,$17
	.set	noreorder
	.set	nomacro
	jal	rtl_get_dev_by_name
	move	$19,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L19
	move	$18,$2
	.set	macro
	.set	reorder

	li	$5,2			# 0x2
	move	$4,$16
	.set	noreorder
	.set	nomacro
	jal	rtl_skb_push
	addiu	$19,$19,2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	move	$4,$16
	.set	macro
	.set	reorder

	li	$3,33			# 0x21
	move	$4,$16
	sh	$3,0($2)
	li	$5,6			# 0x6
	.set	noreorder
	.set	nomacro
	jal	rtl_skb_push
	andi	$19,$19,0xffff
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	move	$4,$16
	.set	macro
	.set	reorder

	li	$3,17			# 0x11
	move	$5,$18
	sb	$3,0($2)
	srl	$3,$19,8
	sb	$0,1($2)
	move	$4,$16
	lbu	$6,52($17)
	sb	$6,2($2)
	lbu	$6,53($17)
	sb	$3,4($2)
	sb	$6,3($2)
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_dev
	sb	$19,5($2)
	.set	macro
	.set	reorder

	li	$5,2			# 0x2
	.set	noreorder
	.set	nomacro
	jal	rtl_skb_push
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	move	$4,$16
	.set	macro
	.set	reorder

	li	$3,-30620			# 0xffff8864
	move	$4,$16
	sh	$3,0($2)
	.set	noreorder
	.set	nomacro
	jal	rtl_skb_push
	li	$5,12			# 0xc
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	move	$4,$16
	.set	macro
	.set	reorder

	addiu	$5,$17,38
	li	$6,6			# 0x6
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$4,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	move	$4,$16
	.set	macro
	.set	reorder

	addiu	$5,$18,320
	addiu	$4,$2,6
	.set	noreorder
	.set	nomacro
	jal	memcpy
	li	$6,6			# 0x6
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	ip_finish_output4
	move	$4,$16
	.set	macro
	.set	reorder

	lui	$2,%hi(jiffies)
	lw	$3,%lo(jiffies)($2)
	li	$2,1			# 0x1
	.set	noreorder
	.set	nomacro
	j	$L20
	sw	$3,60($17)
	.set	macro
	.set	reorder

$L19:
	move	$2,$0
$L20:
	lw	$31,52($sp)
	lw	$19,48($sp)
	lw	$18,44($sp)
	lw	$17,40($sp)
	lw	$16,36($sp)
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,56
	.set	macro
	.set	reorder

	.end	fast_pppoe_xmit
	.size	fast_pppoe_xmit, .-fast_pppoe_xmit
	.section	.text.get_pppoe_last_rx_tx,"ax",@progbits
	.align	2
	.globl	get_pppoe_last_rx_tx
	.set	nomips16
	.ent	get_pppoe_last_rx_tx
	.type	get_pppoe_last_rx_tx, @function
get_pppoe_last_rx_tx:
	.frame	$sp,40,$31		# vars= 0, regs= 1/0, args= 32, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-40
	sw	$31,36($sp)
	andi	$6,$6,0xffff
	lw	$2,56($sp)
	sw	$2,16($sp)
	lw	$2,60($sp)
	sw	$2,20($sp)
	lw	$2,64($sp)
	jal	find_pppoe_info
	sw	$2,24($sp)

	beq	$2,$0,$L23
	nop

	lw	$3,68($sp)
	lw	$4,56($2)
	sw	$4,0($3)
	lw	$3,60($2)
	lw	$2,72($sp)
	sw	$3,0($2)
$L23:
	lw	$31,36($sp)
	move	$2,$0
	j	$31
	addiu	$sp,$sp,40

	.set	macro
	.set	reorder
	.end	get_pppoe_last_rx_tx
	.size	get_pppoe_last_rx_tx, .-get_pppoe_last_rx_tx
	.section	.text.check_and_restore_pppoe_hdr,"ax",@progbits
	.align	2
	.globl	check_and_restore_pppoe_hdr
	.set	nomips16
	.ent	check_and_restore_pppoe_hdr
	.type	check_and_restore_pppoe_hdr, @function
check_and_restore_pppoe_hdr:
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
	lui	$2,%hi(fast_pppoe_fw)
	lw	$2,%lo(fast_pppoe_fw)($2)
	beq	$2,$0,$L27
	move	$16,$4

	jal	rtl_get_skb_pppoe_flag
	nop

	beq	$2,$0,$L27
	li	$5,8			# 0x8

	jal	rtl_skb_push
	move	$4,$16

	jal	rtl_get_skb_data
	move	$4,$16

	move	$4,$16
	jal	rtl_set_skb_network_header
	move	$5,$2

	jal	rtl_get_skb_data
	move	$4,$16

	move	$4,$16
	jal	rtl_set_skb_transport_header
	move	$5,$2

	jal	rtl_get_skb_dev
	move	$4,$16

	move	$4,$16
	lw	$3,80($2)
	addiu	$3,$3,-1
	jal	rtl_get_skb_dev
	sw	$3,80($2)

	move	$4,$16
	lw	$18,88($2)
	jal	rtl_get_skb_len
	move	$17,$2

	move	$4,$16
	subu	$2,$18,$2
	jal	rtl_get_skb_rx_dev
	sw	$2,88($17)

	move	$4,$16
	jal	rtl_set_skb_dev
	move	$5,$2

	move	$4,$16
	jal	rtl_set_skb_protocol
	li	$5,34916			# 0x8864

	move	$4,$16
	move	$5,$0
	lw	$31,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	j	rtl_set_skb_pppoe_flag
	addiu	$sp,$sp,32

$L27:
	lw	$31,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	j	$31
	addiu	$sp,$sp,32

	.set	macro
	.set	reorder
	.end	check_and_restore_pppoe_hdr
	.size	check_and_restore_pppoe_hdr, .-check_and_restore_pppoe_hdr
	.section	.text.check_and_pull_pppoe_hdr,"ax",@progbits
	.align	2
	.globl	check_and_pull_pppoe_hdr
	.set	nomips16
	.ent	check_and_pull_pppoe_hdr
	.type	check_and_pull_pppoe_hdr, @function
check_and_pull_pppoe_hdr:
	.frame	$sp,64,$31		# vars= 8, regs= 10/0, args= 16, gp= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-64
	sw	$31,60($sp)
	sw	$fp,56($sp)
	sw	$23,52($sp)
	sw	$22,48($sp)
	sw	$21,44($sp)
	sw	$20,40($sp)
	sw	$19,36($sp)
	sw	$18,32($sp)
	sw	$17,28($sp)
	sw	$16,24($sp)
	lui	$2,%hi(fast_pppoe_fw)
	lw	$2,%lo(fast_pppoe_fw)($2)
	beq	$2,$0,$L29
	move	$16,$4

	jal	rtl_set_skb_pppoe_flag
	move	$5,$0

	jal	rtl_skb_mac_header
	move	$4,$16

	beq	$2,$0,$L29
	move	$17,$2

	lhu	$3,12($2)
	li	$2,34916			# 0x8864
	bne	$3,$2,$L33
	move	$2,$0

	lhu	$3,20($17)
	li	$2,33			# 0x21
	bne	$3,$2,$L33
	move	$2,$0

	lui	$19,%hi(fast_pppoe_info+54)
	lui	$21,%hi(fast_pppoe_info)
	lhu	$3,16($17)
	addiu	$19,$19,%lo(fast_pppoe_info+54)
	addiu	$21,$21,%lo(fast_pppoe_info)
	addiu	$fp,$17,6
	move	$18,$0
	li	$23,88			# 0x58
	li	$22,16			# 0x10
$L32:
	lhu	$2,0($19)
	beq	$2,$0,$L30
	nop

	lhu	$2,-2($19)
	bne	$2,$3,$L30
	mult	$18,$23

	move	$5,$fp
	li	$6,6			# 0x6
	sw	$3,16($sp)
	mflo	$20
	addu	$4,$21,$20
	jal	memcmp
	addiu	$4,$4,38

	lw	$3,16($sp)
	bne	$2,$0,$L37
	addiu	$18,$18,1

	addiu	$18,$18,-1
	addu	$20,$21,$20
	bne	$20,$0,$L36
	move	$2,$0

	j	$L33
	nop

$L30:
	addiu	$18,$18,1
$L37:
	bne	$18,$22,$L32
	addiu	$19,$19,88

	j	$L33
	move	$2,$0

$L36:
	jal	rtl_get_dev_by_name
	addiu	$4,$20,16

	beq	$2,$0,$L29
	move	$18,$2

	addiu	$17,$17,22
	move	$4,$16
	jal	rtl_set_skb_network_header
	move	$5,$17

	move	$5,$17
	jal	rtl_set_skb_transport_header
	move	$4,$16

	li	$5,8			# 0x8
	jal	rtl_skb_pull
	move	$4,$16

	jal	rtl_get_skb_dev
	move	$4,$16

	move	$4,$16
	jal	rtl_set_skb_rx_dev
	move	$5,$2

	move	$4,$16
	jal	rtl_set_skb_protocol
	li	$5,2048			# 0x800

	move	$5,$18
	jal	rtl_set_skb_dev
	move	$4,$16

	jal	rtl_get_skb_dev
	move	$4,$16

	move	$4,$16
	lw	$3,80($2)
	addiu	$3,$3,1
	jal	rtl_get_skb_dev
	sw	$3,80($2)

	move	$4,$16
	lw	$18,88($2)
	jal	rtl_get_skb_len
	move	$17,$2

	move	$4,$16
	li	$5,1			# 0x1
	addu	$2,$2,$18
	jal	rtl_set_skb_pppoe_flag
	sw	$2,88($17)

	lui	$2,%hi(jiffies)
	lw	$3,%lo(jiffies)($2)
	li	$2,1			# 0x1
	j	$L33
	sw	$3,56($20)

$L29:
	move	$2,$0
$L33:
	lw	$31,60($sp)
	lw	$fp,56($sp)
	lw	$23,52($sp)
	lw	$22,48($sp)
	lw	$21,44($sp)
	lw	$20,40($sp)
	lw	$19,36($sp)
	lw	$18,32($sp)
	lw	$17,28($sp)
	lw	$16,24($sp)
	j	$31
	addiu	$sp,$sp,64

	.set	macro
	.set	reorder
	.end	check_and_pull_pppoe_hdr
	.size	check_and_pull_pppoe_hdr, .-check_and_pull_pppoe_hdr
	.section	.text.set_pppoe_info,"ax",@progbits
	.align	2
	.globl	set_pppoe_info
	.set	nomips16
	.ent	set_pppoe_info
	.type	set_pppoe_info, @function
set_pppoe_info:
	.frame	$sp,72,$31		# vars= 0, regs= 9/0, args= 32, gp= 0
	.mask	0x80ff0000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-72
	sw	$31,68($sp)
	sw	$23,64($sp)
	sw	$22,60($sp)
	sw	$21,56($sp)
	sw	$20,52($sp)
	sw	$19,48($sp)
	sw	$18,44($sp)
	sw	$17,40($sp)
	sw	$16,36($sp)
	move	$19,$5
	move	$5,$0
	lw	$18,88($sp)
	andi	$22,$6,0xffff
	move	$6,$0
	lw	$16,92($sp)
	move	$20,$4
	lw	$17,96($sp)
	move	$21,$7
	sw	$18,16($sp)
	sw	$16,20($sp)
	.set	noreorder
	.set	nomacro
	jal	find_pppoe_info
	sw	$17,24($sp)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L39
	move	$23,$2
	.set	macro
	.set	reorder

	lui	$3,%hi(fast_pppoe_info+54)
	move	$2,$0
	addiu	$3,$3,%lo(fast_pppoe_info+54)
	li	$4,16			# 0x10
$L42:
	lhu	$5,0($3)
	.set	noreorder
	.set	nomacro
	bne	$5,$0,$L40
	addiu	$3,$3,88
	.set	macro
	.set	reorder

	li	$23,88			# 0x58
	mult	$2,$23
	lui	$23,%hi(fast_pppoe_info)
	addiu	$23,$23,%lo(fast_pppoe_info)
	mflo	$2
	addu	$23,$23,$2
	bne	$23,$0,$L39
	j	$L41
$L40:
	addiu	$2,$2,1
	.set	noreorder
	.set	nomacro
	bne	$2,$4,$L42
	move	$5,$0
	.set	macro
	.set	reorder

	lui	$23,%hi(fast_pppoe_info)
	addiu	$4,$23,%lo(fast_pppoe_info)
	.set	noreorder
	.set	nomacro
	jal	memset
	li	$6,1408			# 0x580
	.set	macro
	.set	reorder

	addiu	$23,$23,%lo(fast_pppoe_info)
$L39:
	beq	$22,$0,$L43
	sh	$22,52($23)
$L43:
	beq	$21,$0,$L44
	sw	$21,44($23)
$L44:
	beq	$18,$0,$L45
	sw	$18,48($23)
$L45:
	.set	noreorder
	.set	nomacro
	beq	$20,$0,$L46
	addiu	$2,$23,16
	.set	macro
	.set	reorder

#APP
 # 25 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/string.h" 1
	.set	noreorder
	.set	noat
1:	lbu	$1,($20)
	addiu	$20,1
	sb	$1,($2)
	bnez	$1,1b
	addiu	$2,1
	.set	at
	.set	reorder
 # 0 "" 2
#NO_APP
$L46:
	.set	noreorder
	.set	nomacro
	beq	$19,$0,$L47
	move	$2,$23
	.set	macro
	.set	reorder

#APP
 # 25 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/string.h" 1
	.set	noreorder
	.set	noat
1:	lbu	$1,($19)
	addiu	$19,1
	sb	$1,($2)
	bnez	$1,1b
	addiu	$2,1
	.set	at
	.set	reorder
 # 0 "" 2
#NO_APP
$L47:
	.set	noreorder
	.set	nomacro
	beq	$17,$0,$L48
	move	$5,$17
	.set	macro
	.set	reorder

	addiu	$4,$23,38
	.set	noreorder
	.set	nomacro
	jal	memcpy
	li	$6,6			# 0x6
	.set	macro
	.set	reorder

$L48:
	.set	noreorder
	.set	nomacro
	beq	$16,$0,$L49
	move	$5,$16
	.set	macro
	.set	reorder

	addiu	$4,$23,32
	.set	noreorder
	.set	nomacro
	jal	memcpy
	li	$6,6			# 0x6
	.set	macro
	.set	reorder

$L49:
	li	$2,1			# 0x1
	sh	$2,54($23)
$L41:
	lw	$31,68($sp)
	move	$2,$0
	lw	$23,64($sp)
	lw	$22,60($sp)
	lw	$21,56($sp)
	lw	$20,52($sp)
	lw	$19,48($sp)
	lw	$18,44($sp)
	lw	$17,40($sp)
	lw	$16,36($sp)
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,72
	.set	macro
	.set	reorder

	.end	set_pppoe_info
	.size	set_pppoe_info, .-set_pppoe_info
	.section	.text.clear_pppoe_info,"ax",@progbits
	.align	2
	.globl	clear_pppoe_info
	.set	nomips16
	.ent	clear_pppoe_info
	.type	clear_pppoe_info, @function
clear_pppoe_info:
	.frame	$sp,56,$31		# vars= 0, regs= 10/0, args= 16, gp= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-56
	sw	$31,52($sp)
	sw	$fp,48($sp)
	sw	$23,44($sp)
	sw	$22,40($sp)
	sw	$21,36($sp)
	sw	$20,32($sp)
	sw	$19,28($sp)
	sw	$18,24($sp)
	sw	$17,20($sp)
	sw	$16,16($sp)
	lui	$16,%hi(fast_pppoe_info)
	move	$22,$4
	addiu	$16,$16,%lo(fast_pppoe_info)
	move	$17,$0
	move	$21,$5
	lw	$20,76($sp)
	li	$fp,16			# 0x10
	andi	$23,$6,0xffff
	lw	$19,80($sp)
	move	$18,$16
$L58:
	lhu	$2,54($16)
	beq	$2,$0,$L53
	.set	noreorder
	.set	nomacro
	beq	$22,$0,$L54
	li	$3,88			# 0x58
	.set	macro
	.set	reorder

	move	$2,$22
	mult	$17,$3
	mflo	$3
	addiu	$3,$3,16
	addu	$3,$18,$3
#APP
 # 75 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/string.h" 1
	.set	noreorder
	.set	noat
	lbu	$4,($3)
1:	lbu	$1,($2)
	addiu	$3,1
	bne	$1,$4,2f
	addiu	$2,1
	bnez	$4,1b
	lbu	$4,($3)
	move	$4,$1
2:	subu	$4,$1
3:	.set	at
	.set	reorder
 # 0 "" 2
#NO_APP
	.set	noreorder
	.set	nomacro
	bne	$4,$0,$L54
	move	$4,$16
	.set	macro
	.set	reorder

	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	memset
	li	$6,88			# 0x58
	.set	macro
	.set	reorder

$L54:
	.set	noreorder
	.set	nomacro
	beq	$21,$0,$L55
	li	$3,88			# 0x58
	.set	macro
	.set	reorder

	move	$2,$21
	mult	$17,$3
	mflo	$3
	addu	$3,$18,$3
#APP
 # 75 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/string.h" 1
	.set	noreorder
	.set	noat
	lbu	$4,($3)
1:	lbu	$1,($2)
	addiu	$3,1
	bne	$1,$4,2f
	addiu	$2,1
	bnez	$4,1b
	lbu	$4,($3)
	move	$4,$1
2:	subu	$4,$1
3:	.set	at
	.set	reorder
 # 0 "" 2
#NO_APP
	.set	noreorder
	.set	nomacro
	bne	$4,$0,$L55
	move	$4,$16
	.set	macro
	.set	reorder

	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	memset
	li	$6,88			# 0x58
	.set	macro
	.set	reorder

$L55:
	.set	noreorder
	.set	nomacro
	beq	$19,$0,$L56
	li	$4,88			# 0x58
	.set	macro
	.set	reorder

	move	$5,$19
	li	$6,6			# 0x6
	mult	$17,$4
	mflo	$4
	addu	$4,$18,$4
	.set	noreorder
	.set	nomacro
	jal	memcmp
	addiu	$4,$4,38
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L56
	move	$4,$16
	.set	macro
	.set	reorder

	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	memset
	li	$6,88			# 0x58
	.set	macro
	.set	reorder

$L56:
	.set	noreorder
	.set	nomacro
	beq	$20,$0,$L57
	li	$4,88			# 0x58
	.set	macro
	.set	reorder

	move	$5,$20
	li	$6,6			# 0x6
	mult	$17,$4
	mflo	$4
	addiu	$4,$4,32
	.set	noreorder
	.set	nomacro
	jal	memcmp
	addu	$4,$18,$4
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L57
	move	$4,$16
	.set	macro
	.set	reorder

	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	memset
	li	$6,88			# 0x58
	.set	macro
	.set	reorder

$L57:
	lhu	$2,52($16)
	.set	noreorder
	.set	nomacro
	bne	$2,$23,$L53
	move	$4,$16
	.set	macro
	.set	reorder

	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	memset
	li	$6,88			# 0x58
	.set	macro
	.set	reorder

$L53:
	addiu	$17,$17,1
	.set	noreorder
	.set	nomacro
	bne	$17,$fp,$L58
	addiu	$16,$16,88
	.set	macro
	.set	reorder

	lw	$31,52($sp)
	move	$2,$0
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

	.end	clear_pppoe_info
	.size	clear_pppoe_info, .-clear_pppoe_info
	.section	.rodata.str1.4
	.align	2
$LC1:
	.ascii	"fast_pppoe\000"
	.section	.exit.text,"ax",@progbits
	.align	2
	.globl	fast_pppoe_exit
	.set	nomips16
	.ent	fast_pppoe_exit
	.type	fast_pppoe_exit, @function
fast_pppoe_exit:
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$16,16($sp)
	lui	$16,%hi(fast_pppoe_proc)
	lw	$5,%lo(fast_pppoe_proc)($16)
	beq	$5,$0,$L62
	nop

	lui	$4,%hi($LC1)
	jal	remove_proc_entry
	addiu	$4,$4,%lo($LC1)

	sw	$0,%lo(fast_pppoe_proc)($16)
$L62:
	lw	$31,20($sp)
	move	$2,$0
	lw	$16,16($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	fast_pppoe_exit
	.size	fast_pppoe_exit, .-fast_pppoe_exit
	.section	.init.text,"ax",@progbits
	.align	2
	.globl	fast_pppoe_init
	.set	nomips16
	.ent	fast_pppoe_init
	.type	fast_pppoe_init, @function
fast_pppoe_init:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	lui	$4,%hi($LC1)
	move	$5,$0
	addiu	$4,$4,%lo($LC1)
	jal	create_proc_entry
	move	$6,$0

	lui	$3,%hi(fast_pppoe_proc)
	beq	$2,$0,$L65
	sw	$2,%lo(fast_pppoe_proc)($3)

	lui	$4,%hi(fast_pppoe_read_proc)
	addiu	$4,$4,%lo(fast_pppoe_read_proc)
	sw	$4,64($2)
	lw	$2,%lo(fast_pppoe_proc)($3)
	lui	$3,%hi(fast_pppoe_write_proc)
	addiu	$3,$3,%lo(fast_pppoe_write_proc)
	sw	$3,68($2)
$L65:
	lui	$4,%hi(fast_pppoe_info)
	move	$5,$0
	addiu	$4,$4,%lo(fast_pppoe_info)
	jal	memset
	li	$6,1408			# 0x580

	move	$2,$0
	lw	$31,20($sp)
	li	$4,1			# 0x1
	lui	$3,%hi(fast_pppoe_fw)
	sw	$4,%lo(fast_pppoe_fw)($3)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	fast_pppoe_init
	.size	fast_pppoe_init, .-fast_pppoe_init
	.section	.text.fast_pppoe_write_proc,"ax",@progbits
	.align	2
	.set	nomips16
	.ent	fast_pppoe_write_proc
	.type	fast_pppoe_write_proc, @function
fast_pppoe_write_proc:
	.frame	$sp,16,$31		# vars= 8, regs= 1/0, args= 0, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-16
	sw	$31,12($sp)
	sltu	$4,$6,2
	.set	noreorder
	.set	nomacro
	bne	$4,$0,$L68
	move	$2,$6
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$5,$0,$L68
	addiu	$6,$5,1
	.set	macro
	.set	reorder

	lw	$4,24($28)
	or	$3,$6,$5
	and	$3,$3,$4
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L68
	move	$4,$sp
	.set	macro
	.set	reorder

	li	$6,1			# 0x1
#APP
 # 105 "net/rtl/fastpath/fast_pppoe_core.c" 1
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
	bne	$6,$0,$L68
	lui	$3,%hi(fast_pppoe_fw)
	.set	macro
	.set	reorder

	lb	$4,0($sp)
	addiu	$4,$4,-48
	.set	noreorder
	.set	nomacro
	j	$L69
	sw	$4,%lo(fast_pppoe_fw)($3)
	.set	macro
	.set	reorder

$L68:
	li	$2,-14			# 0xfffffff2
$L69:
	lw	$31,12($sp)
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,16
	.set	macro
	.set	reorder

	.end	fast_pppoe_write_proc
	.size	fast_pppoe_write_proc, .-fast_pppoe_write_proc
	.section	.rodata.str1.4
	.align	2
$LC2:
	.ascii	"fast pppoe enable:%c\012\000"
	.align	2
$LC3:
	.ascii	"[%d] null,null,0\012\000"
	.align	2
$LC4:
	.ascii	"[%d] %s,%s,%d,0x%x:%x:%x:%x:%x:%x\012\000"
	.section	.text.fast_pppoe_read_proc,"ax",@progbits
	.align	2
	.set	nomips16
	.ent	fast_pppoe_read_proc
	.type	fast_pppoe_read_proc, @function
fast_pppoe_read_proc:
	.frame	$sp,88,$31		# vars= 0, regs= 10/0, args= 48, gp= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
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
	lui	$2,%hi(fast_pppoe_fw)
	move	$23,$5
	lw	$2,%lo(fast_pppoe_fw)($2)
	lui	$5,%hi($LC2)
	move	$20,$6
	addiu	$5,$5,%lo($LC2)
	move	$19,$7
	lui	$16,%hi(fast_pppoe_info+54)
	addiu	$6,$2,48
	move	$21,$4
	jal	sprintf
	lui	$22,%hi(fast_pppoe_info)

	move	$18,$0
	move	$17,$2
	addiu	$16,$16,%lo(fast_pppoe_info+54)
	addiu	$22,$22,%lo(fast_pppoe_info)
	li	$fp,16			# 0x10
$L74:
	li	$2,88			# 0x58
	lui	$5,%hi($LC3)
	move	$6,$18
	mult	$18,$2
	lhu	$3,0($16)
	addu	$2,$21,$17
	addiu	$5,$5,%lo($LC3)
	move	$4,$2
	bne	$3,$0,$L72
	mflo	$13

	jal	sprintf
	addiu	$18,$18,1

	j	$L80
	addu	$17,$17,$2

$L72:
	addiu	$7,$13,16
	move	$4,$2
	lui	$5,%hi($LC4)
	lhu	$12,-2($16)
	addu	$13,$22,$13
	move	$6,$18
	lbu	$11,-16($16)
	addiu	$5,$5,%lo($LC4)
	lbu	$10,-15($16)
	addu	$7,$22,$7
	lbu	$9,-14($16)
	lbu	$8,-13($16)
	lbu	$3,-12($16)
	lbu	$2,-11($16)
	sw	$13,16($sp)
	sw	$12,20($sp)
	sw	$11,24($sp)
	sw	$10,28($sp)
	sw	$9,32($sp)
	sw	$8,36($sp)
	sw	$3,40($sp)
	jal	sprintf
	sw	$2,44($sp)

	addiu	$18,$18,1
	addu	$17,$17,$2
$L80:
	bne	$18,$fp,$L74
	addiu	$16,$16,88

	addu	$2,$19,$20
	slt	$2,$2,$17
	bne	$2,$0,$L75
	li	$3,1			# 0x1

	lw	$2,104($sp)
	sw	$3,0($2)
$L75:
	subu	$17,$17,$20
	addu	$20,$21,$20
	slt	$2,$19,$17
	sw	$20,0($23)
	movz	$19,$17,$2
	slt	$2,$19,0
	lw	$31,84($sp)
	movn	$19,$0,$2
	move	$2,$19
	lw	$fp,80($sp)
	lw	$23,76($sp)
	lw	$22,72($sp)
	lw	$21,68($sp)
	lw	$20,64($sp)
	lw	$19,60($sp)
	lw	$18,56($sp)
	lw	$17,52($sp)
	lw	$16,48($sp)
	j	$31
	addiu	$sp,$sp,88

	.set	macro
	.set	reorder
	.end	fast_pppoe_read_proc
	.size	fast_pppoe_read_proc, .-fast_pppoe_read_proc
	.globl	fast_pppoe_fw
	.section	.bss,"aw",@nobits
	.align	2
	.type	fast_pppoe_fw, @object
	.size	fast_pppoe_fw, 4
fast_pppoe_fw:
	.space	4
	.globl	fast_pppoe_proc
	.align	2
	.type	fast_pppoe_proc, @object
	.size	fast_pppoe_proc, 4
fast_pppoe_proc:
	.space	4
	.globl	fast_pppoe_info
	.align	2
	.type	fast_pppoe_info, @object
	.size	fast_pppoe_info, 1408
fast_pppoe_info:
	.space	1408
	.ident	"GCC: (GNU) 4.4.5-1.5.5p4"
