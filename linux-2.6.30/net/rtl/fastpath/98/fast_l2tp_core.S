	.file	1 "fast_l2tp_core.c"
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
	.data
	.align	2
	.type	res1, @object
	.size	res1, 4
res1:
	.word	0
	.align	2
	.type	l2tpInfo, @object
	.size	l2tpInfo, 40
l2tpInfo:
	.word	0
	.space	36
	.section	.text.is_l2tp_device,"ax",@progbits
	.align	2
	.globl	is_l2tp_device
	.ent	is_l2tp_device
	.type	is_l2tp_device, @function
is_l2tp_device:
	.set	nomips16
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-24
	lui	$2,%hi(l2tpInfo+4)
	sw	$16,16($sp)
	move	$16,$4
	sw	$31,20($sp)
	lw	$2,%lo(l2tpInfo+4)($2)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L2
	move	$4,$2
	.set	macro
	.set	reorder

	jal	rtl_get_ppp_dev_name
#APP
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
#NO_APP
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L1
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

$L2:
	move	$2,$0
$L1:
	lw	$31,20($sp)
	lw	$16,16($sp)
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,24
	.set	macro
	.set	reorder

	.end	is_l2tp_device
	.section	.text.set_l2tp_device,"ax",@progbits
	.align	2
	.globl	set_l2tp_device
	.ent	set_l2tp_device
	.type	set_l2tp_device, @function
set_l2tp_device:
	.set	nomips16
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	lui	$2,%hi(fast_l2tp_fw)
	sw	$31,16($sp)
	lw	$2,%lo(fast_l2tp_fw)($2)
	beq	$2,$0,$L4
	nop

	jal	rtl_get_dev_by_name
	nop

	lui	$3,%hi(l2tpInfo)
	addiu	$3,$3,%lo(l2tpInfo)
	sw	$2,4($3)
$L4:
	lw	$31,16($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	set_l2tp_device
	.section	.text.event_ppp_dev_down,"ax",@progbits
	.align	2
	.globl	event_ppp_dev_down
	.ent	event_ppp_dev_down
	.type	event_ppp_dev_down, @function
event_ppp_dev_down:
	.set	nomips16
	.frame	$sp,32,$31		# vars= 0, regs= 3/0, args= 16, gp= 0
	.mask	0x80030000,-8
	.fmask	0x00000000,0
	addiu	$sp,$sp,-32
	lui	$2,%hi(l2tpInfo)
	sw	$17,20($sp)
	addiu	$17,$2,%lo(l2tpInfo)
	lhu	$2,38($17)
	sw	$16,16($sp)
	move	$16,$4
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L6
	sw	$31,24($sp)
	.set	macro
	.set	reorder

	lw	$2,4($17)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L6
	move	$4,$2
	.set	macro
	.set	reorder

	jal	rtl_get_ppp_dev_name
#APP
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
#NO_APP
	move	$5,$0
	move	$4,$17
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L6
	li	$6,40			# 0x28
	.set	macro
	.set	reorder

	jal	memset
	lui	$2,%hi(state)
	sw	$0,%lo(state)($2)
$L6:
	lw	$31,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,32
	.set	macro
	.set	reorder

	.end	event_ppp_dev_down
	.rdata
	.align	2
$LC0:
	.ascii	"ppp0\000"
	.align	2
$LC1:
	.ascii	"eth1\000"
	.section	.text.l2tp_tx_id,"ax",@progbits
	.align	2
	.globl	l2tp_tx_id
	.ent	l2tp_tx_id
	.type	l2tp_tx_id, @function
l2tp_tx_id:
	.set	nomips16
	.frame	$sp,48,$31		# vars= 0, regs= 7/0, args= 16, gp= 0
	.mask	0x803f0000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-48
	sw	$20,32($sp)
	move	$20,$4
	sw	$31,40($sp)
	sw	$19,28($sp)
	sw	$18,24($sp)
	sw	$21,36($sp)
	sw	$17,20($sp)
	jal	rtl_get_skb_data
	sw	$16,16($sp)

	move	$4,$20
	jal	rtl_ip_hdr
	move	$18,$2

	move	$19,$2
	lbu	$2,0($2)
	andi	$2,$2,0xf
	sll	$2,$2,2
	addu	$4,$19,$2
	li	$2,1701			# 0x6a5
	lhu	$6,0($4)
	andi	$3,$6,0xffff
	bne	$3,$2,$L27
	addiu	$5,$4,8

	lhu	$2,2($4)
	bne	$2,$3,$L27
	li	$2,1701			# 0x6a5

	lhu	$3,8($4)
	sll	$2,$3,16
	sra	$2,$2,16
	bgez	$2,$L11
	addiu	$2,$4,12

	andi	$3,$3,0x4000
	addiu	$17,$4,10
	movn	$17,$2,$3		#conditional move  
	li	$2,4			# 0x4
	lhu	$16,14($17)
	bne	$16,$2,$L28
	li	$2,11			# 0xb

	lui	$2,%hi(l2tpInfo)
	addiu	$4,$2,%lo(l2tpInfo)
	lhu	$3,20($4)
	lhu	$2,0($17)
	bne	$3,$2,$L28
	li	$2,11			# 0xb

	move	$5,$0
	jal	memset
	li	$6,40			# 0x28

	lui	$4,%hi($LC0)
	addiu	$4,$4,%lo($LC0)
	jal	rtl865x_setNetifType
	li	$5,2			# 0x2

	lui	$2,%hi(state)
	sw	$0,%lo(state)($2)
	li	$2,11			# 0xb
$L28:
	bne	$16,$2,$L29
	li	$2,12			# 0xc

	lui	$21,%hi(state)
	lw	$2,%lo(state)($21)
	bne	$2,$0,$L29
	li	$2,12			# 0xc

	jal	rtl_get_skb_dev_name
	move	$4,$20

	lui	$5,%hi($LC1)
	move	$4,$2
	addiu	$5,$5,%lo($LC1)
	jal	memcmp
	li	$6,4			# 0x4

	bne	$2,$0,$L29
	li	$2,12			# 0xc

	jal	rtl_get_skb_dev
	move	$4,$20

	lui	$4,%hi(l2tpInfo+24)
	lui	$3,%hi(l2tpInfo)
	addiu	$4,$4,%lo(l2tpInfo+24)
	addiu	$5,$18,-8
	li	$6,6			# 0x6
	jal	memcpy
	sw	$2,%lo(l2tpInfo)($3)

	lui	$4,%hi(l2tpInfo+30)
	addiu	$4,$4,%lo(l2tpInfo+30)
	addiu	$5,$18,-14
	jal	memcpy
	li	$6,6			# 0x6

	lui	$4,%hi(l2tpInfo+36)
	addiu	$4,$4,%lo(l2tpInfo+36)
	addiu	$5,$18,-2
	jal	memcpy
	li	$6,2			# 0x2

	li	$2,1			# 0x1
	sw	$2,%lo(state)($21)
	li	$2,12			# 0xc
$L29:
	bne	$16,$2,$L10
	nop

	lhu	$2,2($17)
	beq	$2,$0,$L10
	lui	$18,%hi(state)

	lw	$2,%lo(state)($18)
	sltu	$2,$2,2
	beq	$2,$0,$L10
	lui	$16,%hi(l2tpInfo)

	lhu	$2,0($17)
	addiu	$16,$16,%lo(l2tpInfo)
	sh	$2,20($16)
	lhu	$2,2($17)
	sh	$2,22($16)
	lw	$2,12($19)
	sw	$2,16($16)
	lw	$2,16($19)
	move	$4,$2
	jal	filter_addconnect
	sw	$2,12($16)

	li	$2,1
	lui	$4,%hi($LC0)
	li	$5,4			# 0x4
	addiu	$4,$4,%lo($LC0)
	jal	rtl865x_setNetifType
	sh	$2,38($16)

	li	$2,2			# 0x2
	j	$L10
	sw	$2,%lo(state)($18)

$L11:
	andi	$3,$6,0xffff
	li	$2,1701			# 0x6a5
$L27:
	bne	$3,$2,$L10
	nop

	lhu	$2,2($4)
	bne	$2,$3,$L10
	lui	$3,%hi(l2tpInfo)

	addiu	$17,$5,2
	lhu	$2,0($5)
	addiu	$16,$3,%lo(l2tpInfo)
	lw	$4,12($16)
	addiu	$3,$5,4
	andi	$2,$2,0x4000
	movn	$17,$3,$2		#conditional move  
	jal	filter_checkConnect
	addiu	$18,$17,6

	beq	$2,$0,$L23
	nop

	lw	$4,12($16)
	jal	filter_addconnect
	nop

$L23:
	lhu	$3,6($17)
	li	$2,49185			# 0xc021
	bne	$3,$2,$L10
	li	$2,6			# 0x6

	lbu	$3,2($18)
	bne	$3,$2,$L10
	nop

	lhu	$3,20($16)
	lhu	$2,0($17)
	bne	$3,$2,$L10
	nop

	lhu	$3,2($17)
	lhu	$2,22($16)
	bne	$2,$3,$L10
	move	$4,$16

	move	$5,$0
	jal	memset
	li	$6,40			# 0x28

	lui	$2,%hi(state)
	sw	$0,%lo(state)($2)
$L10:
	lw	$31,40($sp)
	lw	$21,36($sp)
	lw	$20,32($sp)
	lw	$19,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	j	$31
	addiu	$sp,$sp,48

	.set	macro
	.set	reorder
	.end	l2tp_tx_id
	.section	.text.fast_l2tp_rx,"ax",@progbits
	.align	2
	.globl	fast_l2tp_rx
	.ent	fast_l2tp_rx
	.type	fast_l2tp_rx, @function
fast_l2tp_rx:
	.set	nomips16
	.frame	$sp,48,$31		# vars= 0, regs= 8/0, args= 16, gp= 0
	.mask	0x807f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-48
	sw	$31,44($sp)
	sw	$22,40($sp)
	sw	$20,32($sp)
	sw	$17,20($sp)
	move	$17,$4
	sw	$16,16($sp)
	sw	$21,36($sp)
	sw	$19,28($sp)
	jal	rtl_ip_hdr
	sw	$18,24($sp)

	move	$4,$17
	move	$20,$2
	lbu	$2,0($2)
	andi	$2,$2,0xf
	sll	$2,$2,2
	jal	rtl_get_skb_data
	addu	$16,$20,$2

	move	$4,$17
	jal	rtl_get_skb_len
	move	$22,$2

	sltu	$2,$2,40
	bne	$2,$0,$L31
	li	$2,1701			# 0x6a5

	lhu	$3,0($16)
	bne	$3,$2,$L31
	nop

	lhu	$2,2($16)
	bne	$2,$3,$L31
	lui	$21,%hi(l2tpInfo)

	lw	$2,%lo(l2tpInfo)($21)
	bne	$2,$0,$L34
	nop

	jal	l2tp_tx_id
	move	$4,$17

$L34:
	lbu	$2,28($22)
	addiu	$19,$21,%lo(l2tpInfo)
	lhu	$4,20($19)
	srl	$3,$2,5
	andi	$2,$2,0x2
	andi	$18,$3,0x2
	beq	$4,$0,$L31
	addu	$18,$18,$2

	lhu	$2,22($19)
	beq	$2,$0,$L31
	addu	$16,$22,$18

	lbu	$2,36($16)
	bne	$2,$0,$L31
	li	$2,33			# 0x21

	lbu	$3,37($16)
	bne	$3,$2,$L31
	nop

	lw	$2,%lo(l2tpInfo)($21)
	beq	$2,$0,$L31
	nop

	lw	$3,16($20)
	lw	$2,16($19)
	bne	$2,$3,$L31
	nop

	lw	$2,4($19)
	bne	$2,$0,$L38
	nop

	lui	$4,%hi($LC0)
	jal	rtl_get_dev_by_name
	addiu	$4,$4,%lo($LC0)

	sw	$2,4($19)
$L38:
	lw	$5,4($19)
	beq	$5,$0,$L31
	li	$2,6			# 0x6

	lbu	$3,47($16)
	beq	$3,$2,$L41
	li	$2,17			# 0x11

	bne	$3,$2,$L31
	nop

$L41:
	jal	rtl_set_skb_dev
	move	$4,$17

	addiu	$5,$18,38
	jal	skb_pull
	move	$4,$17

	jal	rtl_skb_reset_network_header
	move	$4,$17

	jal	rtl_skb_reset_transport_header
	move	$4,$17

	lw	$4,4($19)
	beq	$4,$0,$L31
	nop

	jal	rtl_get_ppp_dev_priv
	nop

	beq	$2,$0,$L31
	nop

	lw	$4,4($19)
	jal	rtl_get_ppp_dev_priv
	nop

	move	$4,$17
	jal	rtl_get_skb_len
	move	$16,$2

	move	$4,$16
	lw	$31,44($sp)
	move	$6,$2
	lw	$22,40($sp)
	move	$5,$0
	lw	$21,36($sp)
	lw	$20,32($sp)
	lw	$19,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	j	rtl_inc_ppp_stats
	addiu	$sp,$sp,48

$L31:
	lw	$31,44($sp)
	lw	$22,40($sp)
	lw	$21,36($sp)
	lw	$20,32($sp)
	lw	$19,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	j	$31
	addiu	$sp,$sp,48

	.set	macro
	.set	reorder
	.end	fast_l2tp_rx
	.section	.text.get_fast_l2tp_lastxmit,"ax",@progbits
	.align	2
	.globl	get_fast_l2tp_lastxmit
	.ent	get_fast_l2tp_lastxmit
	.type	get_fast_l2tp_lastxmit, @function
get_fast_l2tp_lastxmit:
	.set	nomips16
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	lui	$2,%hi(l2tpInfo)
	addiu	$5,$2,%lo(l2tpInfo)
	li	$2,1			# 0x1
	lhu	$3,38($5)
	bne	$3,$2,$L43
	move	$4,$0

	lw	$4,8($5)
$L43:
	j	$31
	move	$2,$4

	.set	macro
	.set	reorder
	.end	get_fast_l2tp_lastxmit
	.section	.text.check_for_fast_l2tp_to_wan,"ax",@progbits
	.align	2
	.globl	check_for_fast_l2tp_to_wan
	.ent	check_for_fast_l2tp_to_wan
	.type	check_for_fast_l2tp_to_wan, @function
check_for_fast_l2tp_to_wan:
	.set	nomips16
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$16,16($sp)
	sw	$31,20($sp)
	jal	rtl_ip_hdr
	move	$16,$4

	move	$4,$16
	lbu	$3,9($2)
	li	$2,1			# 0x1
	beq	$3,$2,$L45
	move	$5,$0

	jal	rtl_ip_hdr
	nop

	li	$3,-256			# 0xffffff00
	lw	$2,12($2)
	and	$2,$2,$3
	li	$3,171966464			# 0xa400000
	ori	$3,$3,0x4000
	xor	$2,$2,$3
	sltu	$5,$0,$2
$L45:
	lw	$31,20($sp)
	move	$2,$5
	lw	$16,16($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	check_for_fast_l2tp_to_wan
	.rdata
	.align	2
	.type	__func__.0, @object
	.size	__func__.0, 17
__func__.0:
	.ascii	"fast_l2tp_to_wan\000"
	.section	.text.fast_l2tp_to_wan,"ax",@progbits
	.align	2
	.globl	fast_l2tp_to_wan
	.ent	fast_l2tp_to_wan
	.type	fast_l2tp_to_wan, @function
fast_l2tp_to_wan:
	.set	nomips16
	.frame	$sp,88,$31		# vars= 48, regs= 6/0, args= 16, gp= 0
	.mask	0x801f0000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-88
	lui	$2,%hi(fast_l2tp_fw)
	sw	$17,68($sp)
	sw	$31,84($sp)
	sw	$20,80($sp)
	sw	$19,76($sp)
	sw	$18,72($sp)
	sw	$16,64($sp)
	lw	$2,%lo(fast_l2tp_fw)($2)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L50
	move	$17,$4
	.set	macro
	.set	reorder

	lui	$18,%hi(l2tpInfo)
	addiu	$3,$18,%lo(l2tpInfo)
	lhu	$2,20($3)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L48
	move	$5,$0
	.set	macro
	.set	reorder

	lhu	$2,22($3)
	beq	$2,$0,$L48
	lw	$2,%lo(l2tpInfo)($18)
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L49
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

$L50:
	.set	noreorder
	.set	nomacro
	j	$L48
	move	$5,$0
	.set	macro
	.set	reorder

$L49:
	lhu	$3,38($3)
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L48
	move	$5,$0
	.set	macro
	.set	reorder

	jal	rtl_ip_hdr
	move	$4,$17
	lhu	$20,2($2)
	.set	noreorder
	.set	nomacro
	jal	rtl_skb_headroom
	move	$16,$2
	.set	macro
	.set	reorder

	sltu	$2,$2,52
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L66
	li	$5,52			# 0x34
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
	bne	$2,$0,$L66
	li	$5,52			# 0x34
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
	beq	$2,$0,$L67
	move	$4,$17
	.set	macro
	.set	reorder

	li	$5,52			# 0x34
$L66:
	.set	noreorder
	.set	nomacro
	jal	skb_realloc_headroom
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L48
	move	$5,$0
	.set	macro
	.set	reorder

	move	$4,$17
	.set	noreorder
	.set	nomacro
	jal	consume_skb
	move	$17,$2
	.set	macro
	.set	reorder

	move	$4,$17
$L67:
	li	$5,52			# 0x34
	lbu	$16,1($16)
	.set	noreorder
	.set	nomacro
	jal	skb_push
	addiu	$19,$sp,16
	.set	macro
	.set	reorder

	lui	$5,%hi(l2tpInfo+24)
	move	$4,$2
	addiu	$5,$5,%lo(l2tpInfo+24)
	.set	noreorder
	.set	nomacro
	jal	memcpy
	li	$6,14			# 0xe
	.set	macro
	.set	reorder

	li	$2,268369920			# 0xfff0000
	lw	$3,16($sp)
	ori	$2,$2,0xffff
	addiu	$4,$18,%lo(l2tpInfo)
	move	$5,$0
	and	$3,$3,$2
	li	$2,1073741824			# 0x40000000
	or	$3,$3,$2
	li	$2,-251723776			# 0xf0ff0000
	ori	$2,$2,0xffff
	and	$3,$3,$2
	li	$2,83886080			# 0x5000000
	or	$3,$3,$2
	li	$2,16384
	lw	$7,16($4)
	lw	$6,12($4)
	move	$4,$17
	sh	$2,22($sp)
	li	$2,17
	sb	$2,25($sp)
	li	$2,64
	sw	$3,16($sp)
	sw	$6,32($sp)
	sw	$7,28($sp)
	sb	$2,24($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_ip_summed
	sb	$16,17($sp)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_len
	move	$4,$17
	.set	macro
	.set	reorder

	addiu	$6,$sp,32
	addiu	$2,$2,-14
	sh	$2,18($sp)
	lw	$3,16($sp)
	sh	$0,20($sp)
	lw	$4,20($sp)
	srl	$2,$3,22
	andi	$2,$2,0x3c
	addu	$5,$3,$4
	addu	$7,$19,$2
	sh	$0,26($sp)
	sltu	$4,$5,$4
	lw	$2,24($sp)
	addu	$5,$5,$4
	lw	$3,28($sp)
	addu	$5,$5,$2
	sltu	$4,$5,$2
	addu	$5,$5,$4
	addu	$5,$5,$3
	sltu	$4,$5,$3
	addu	$5,$5,$4
$L56:
	lw	$2,0($6)
	addiu	$6,$6,4
	addu	$5,$5,$2
	sltu	$4,$5,$2
	.set	noreorder
	.set	nomacro
	bne	$6,$7,$L56
	addu	$5,$5,$4
	.set	macro
	.set	reorder

	move	$4,$17
	addiu	$16,$18,%lo(l2tpInfo)
#APP
		.set	push		# csum_fold
	.set	noat		
	sll	$1, $5, 16	
	addu	$5, $1		
	sltu	$1, $5, $1	
	srl	$5, $5, 16	
	addu	$5, $1		
	xori	$5, 0xffff	
	.set	pop
#NO_APP
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	sh	$5,10($19)
	.set	macro
	.set	reorder

	addiu	$5,$sp,16
	addiu	$4,$2,14
	.set	noreorder
	.set	nomacro
	jal	memcpy
	li	$6,20			# 0x14
	.set	macro
	.set	reorder

	addiu	$2,$20,18
	lhu	$3,22($16)
	li	$6,1701
	lhu	$5,20($16)
	move	$4,$17
	sh	$2,44($sp)
	li	$2,2
	sh	$2,48($sp)
	li	$2,-253
	sh	$2,54($sp)
	li	$2,33
	sh	$3,52($sp)
	sh	$6,42($sp)
	sh	$5,50($sp)
	sh	$6,40($sp)
	sh	$2,56($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	sh	$0,46($sp)
	.set	macro
	.set	reorder

	li	$6,18			# 0x12
	addiu	$5,$sp,40
	.set	noreorder
	.set	nomacro
	jal	memcpy
	addiu	$4,$2,34
	.set	macro
	.set	reorder

	lw	$5,%lo(l2tpInfo)($18)
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_dev
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	timeoutCheck_skipp_pkt
	addiu	$4,$2,52
	.set	macro
	.set	reorder

	li	$3,1			# 0x1
	.set	noreorder
	.set	nomacro
	beq	$2,$3,$L61
	lui	$2,%hi(jiffies)
	.set	macro
	.set	reorder

	lw	$2,%lo(jiffies)($2)
	sw	$2,8($16)
$L61:
	lw	$4,4($16)
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L68
	lui	$2,%hi(gQosEnabled)
	.set	macro
	.set	reorder

	jal	rtl_get_ppp_dev_priv
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L68
	lui	$2,%hi(gQosEnabled)
	.set	macro
	.set	reorder

	lw	$4,4($16)
	jal	rtl_get_ppp_dev_priv
	move	$4,$17
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_len
	move	$16,$2
	.set	macro
	.set	reorder

	move	$4,$16
	move	$6,$2
	.set	noreorder
	.set	nomacro
	jal	rtl_inc_ppp_stats
	li	$5,1			# 0x1
	.set	macro
	.set	reorder

	lui	$2,%hi(gQosEnabled)
$L68:
	lw	$2,%lo(gQosEnabled)($2)
	beq	$2,$0,$L63
	.set	noreorder
	.set	nomacro
	jal	dev_queue_xmit
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L48
	li	$5,1			# 0x1
	.set	macro
	.set	reorder

$L63:
	.set	noreorder
	.set	nomacro
	jal	rtl_call_skb_ndo_start_xmit
	move	$4,$17
	.set	macro
	.set	reorder

	li	$5,1			# 0x1
$L48:
	lw	$31,84($sp)
	move	$2,$5
	lw	$20,80($sp)
	lw	$19,76($sp)
	lw	$18,72($sp)
	lw	$17,68($sp)
	lw	$16,64($sp)
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,88
	.set	macro
	.set	reorder

	.end	fast_l2tp_to_wan
	.rdata
	.align	2
$LC3:
	.ascii	"%c\n\000"
	.section	.text.l2tp_read_proc,"ax",@progbits
	.align	2
	.ent	l2tp_read_proc
	.type	l2tp_read_proc, @function
l2tp_read_proc:
	.set	nomips16
	.frame	$sp,40,$31		# vars= 0, regs= 5/0, args= 16, gp= 0
	.mask	0x800f0000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-40
	lui	$2,%hi(fast_l2tp_fw)
	sw	$19,28($sp)
	move	$19,$5
	sw	$18,24($sp)
	lui	$5,%hi($LC3)
	sw	$17,20($sp)
	addiu	$5,$5,%lo($LC3)
	sw	$16,16($sp)
	move	$17,$6
	sw	$31,32($sp)
	move	$16,$7
	lw	$2,%lo(fast_l2tp_fw)($2)
	move	$18,$4
	addu	$18,$18,$17
	jal	sprintf
	addiu	$6,$2,48

	move	$4,$2
	addu	$2,$17,$16
	slt	$2,$2,$4
	subu	$4,$4,$17
	slt	$3,$16,$4
	movn	$4,$16,$3		#conditional move
	bne	$2,$0,$L70
	slt	$5,$4,0

	lw	$2,56($sp)
	li	$3,1			# 0x1
	sw	$3,0($2)
$L70:
	sw	$18,0($19)
	move	$2,$0
	lw	$31,32($sp)
	movz	$2,$4,$5		#conditional move  
	lw	$19,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	j	$31
	addiu	$sp,$sp,40

	.set	macro
	.set	reorder
	.end	l2tp_read_proc
	.section	.text.l2tp_write_proc,"ax",@progbits
	.align	2
	.ent	l2tp_write_proc
	.type	l2tp_write_proc, @function
l2tp_write_proc:
	.set	nomips16
	.frame	$sp,32,$31		# vars= 8, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-32
	sltu	$2,$6,2
	sw	$16,24($sp)
	li	$3,-14			# 0xfffffff2
	sw	$31,28($sp)
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L73
	move	$16,$6
	.set	macro
	.set	reorder

	addiu	$2,$5,1
	.set	noreorder
	.set	nomacro
	beq	$5,$0,$L75
	or	$3,$5,$2
	.set	macro
	.set	reorder

	lw	$2,24($28)
	addiu	$4,$sp,16
	and	$2,$2,$3
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L75
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
	bne	$6,$0,$L73
	li	$3,-14			# 0xfffffff2
	.set	macro
	.set	reorder

	lb	$2,16($sp)
	lui	$3,%hi(fast_l2tp_fw)
	addiu	$2,$2,-48
	sw	$2,%lo(fast_l2tp_fw)($3)
	li	$3,1			# 0x1
	.set	noreorder
	.set	nomacro
	bne	$2,$3,$L83
	lui	$4,%hi(l2tp_tx_id_hook)
	.set	macro
	.set	reorder

	lui	$2,%hi(l2tp_tx_id)
	addiu	$2,$2,%lo(l2tp_tx_id)
	.set	noreorder
	.set	nomacro
	j	$L84
	sw	$2,%lo(l2tp_tx_id_hook)($4)
	.set	macro
	.set	reorder

$L83:
	lui	$2,%hi(l2tpInfo+20)
	sw	$0,%lo(l2tp_tx_id_hook)($4)
	sh	$0,%lo(l2tpInfo+20)($2)
$L84:
	lui	$4,%hi(l2tpInfo)
	move	$5,$0
	li	$6,40			# 0x28
	.set	noreorder
	.set	nomacro
	jal	memset
	addiu	$4,$4,%lo(l2tpInfo)
	.set	macro
	.set	reorder

	lui	$2,%hi(state)
	move	$3,$16
	.set	noreorder
	.set	nomacro
	j	$L73
	sw	$0,%lo(state)($2)
	.set	macro
	.set	reorder

$L75:
	li	$3,-14			# 0xfffffff2
$L73:
	lw	$31,28($sp)
	move	$2,$3
	lw	$16,24($sp)
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,32
	.set	macro
	.set	reorder

	.end	l2tp_write_proc
	.rdata
	.align	2
$LC4:
	.ascii	"fast_l2tp\000"
	.section	.init.text,"ax",@progbits
	.align	2
	.globl	fast_l2tp_init
	.ent	fast_l2tp_init
	.type	fast_l2tp_init, @function
fast_l2tp_init:
	.set	nomips16
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	lui	$4,%hi($LC4)
	addiu	$sp,$sp,-24
	move	$5,$0
	addiu	$4,$4,%lo($LC4)
	sw	$31,16($sp)
	jal	create_proc_entry
	move	$6,$0

	lui	$4,%hi(l2tpInfo)
	lui	$7,%hi(res1)
	move	$3,$2
	addiu	$4,$4,%lo(l2tpInfo)
	move	$5,$0
	li	$6,40			# 0x28
	beq	$2,$0,$L86
	sw	$2,%lo(res1)($7)

	lui	$2,%hi(l2tp_read_proc)
	addiu	$2,$2,%lo(l2tp_read_proc)
	sw	$2,64($3)
	lui	$2,%hi(l2tp_write_proc)
	lw	$3,%lo(res1)($7)
	addiu	$2,$2,%lo(l2tp_write_proc)
	sw	$2,68($3)
$L86:
	lui	$2,%hi(l2tp_tx_id_hook)
	sw	$0,%lo(l2tp_tx_id_hook)($2)
	lui	$2,%hi(fast_l2tp_fw)
	sw	$0,%lo(fast_l2tp_fw)($2)
	lui	$2,%hi(state)
	jal	memset
	sw	$0,%lo(state)($2)

	move	$2,$0
	lw	$31,16($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	fast_l2tp_init
	.section	.exit.text,"ax",@progbits
	.align	2
	.globl	fast_l2tp_exit
	.ent	fast_l2tp_exit
	.type	fast_l2tp_exit, @function
fast_l2tp_exit:
	.set	nomips16
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	lui	$4,%hi($LC4)
	sw	$16,16($sp)
	lui	$16,%hi(res1)
	lw	$2,%lo(res1)($16)
	addiu	$4,$4,%lo($LC4)
	sw	$31,20($sp)
	beq	$2,$0,$L87
	move	$5,$2

	jal	remove_proc_entry
	nop

	sw	$0,%lo(res1)($16)
$L87:
	lw	$31,20($sp)
	lw	$16,16($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	fast_l2tp_exit
	.globl	l2tp_tx_id_hook
	.section	.bss
	.align	2
	.type	l2tp_tx_id_hook, @object
	.size	l2tp_tx_id_hook, 4
l2tp_tx_id_hook:
	.space	4
	.globl	fast_l2tp_fw
	.align	2
	.type	fast_l2tp_fw, @object
	.size	fast_l2tp_fw, 4
fast_l2tp_fw:
	.space	4
	.globl	state
	.align	2
	.type	state, @object
	.size	state, 4
state:
	.space	4
	.ident	"GCC: (GNU) 3.4.6-1.3.6"
