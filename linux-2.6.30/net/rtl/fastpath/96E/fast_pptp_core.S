	.file	1 "fast_pptp_core.c"
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
	.globl	fast_pptp_fw
	.data
	.align	2
	.type	fast_pptp_fw, @object
	.size	fast_pptp_fw, 4
fast_pptp_fw:
	.word	0
	.globl	pptp_tcp_finished
	.align	2
	.type	pptp_tcp_finished, @object
	.size	pptp_tcp_finished, 4
pptp_tcp_finished:
	.word	0
	.align	2
	.type	res_check_pptp, @object
	.size	res_check_pptp, 4
res_check_pptp:
	.word	0
	.align	2
	.type	res1, @object
	.size	res1, 4
res1:
	.word	0
	.align	2
	.type	pptp_conn_check, @object
	.size	pptp_conn_check, 4
pptp_conn_check:
	.word	0
	.section	.text.is_pptp_device,"ax",@progbits
	.align	2
	.globl	is_pptp_device
	.ent	is_pptp_device
	.type	is_pptp_device, @function
is_pptp_device:
	.set	nomips16
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-24
	sw	$16,16($sp)
	sw	$31,20($sp)
	lui	$2,%hi(pptpAccInfo+56)
	lw	$2,%lo(pptpAccInfo+56)($2)
	move	$16,$4
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
	nop
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

	.end	is_pptp_device
	.section	.text.set_pptp_device,"ax",@progbits
	.align	2
	.globl	set_pptp_device
	.ent	set_pptp_device
	.type	set_pptp_device, @function
set_pptp_device:
	.set	nomips16
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,16($sp)
	lui	$2,%hi(fast_pptp_fw)
	lw	$2,%lo(fast_pptp_fw)($2)
	nop
	beq	$2,$0,$L4
	nop

	jal	rtl_get_dev_by_name
	nop

	lui	$3,%hi(pptpAccInfo)
	addiu	$3,$3,%lo(pptpAccInfo)
	sw	$2,56($3)
$L4:
	lw	$31,16($sp)
	nop
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	set_pptp_device
	.section	.text.Check_GRE_rx_net_device,"ax",@progbits
	.align	2
	.globl	Check_GRE_rx_net_device
	.ent	Check_GRE_rx_net_device
	.type	Check_GRE_rx_net_device, @function
Check_GRE_rx_net_device:
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
	jal	rtl_ip_hdr
	move	$17,$4

	move	$16,$2
	jal	rtl_get_skb_data
	move	$4,$17

	lbu	$5,0($16)
	lui	$3,%hi(pptpAccInfo)
	addiu	$16,$3,%lo(pptpAccInfo)
	lw	$3,8($16)
	andi	$5,$5,0xf
	sll	$5,$5,2
	move	$4,$17
	addu	$17,$2,$5
	beq	$3,$0,$L6
	move	$5,$0

	jal	rtl_get_skb_dev
	nop

	lw	$3,8($16)
	nop
	bne	$3,$2,$L8
	li	$5,1			# 0x1

	lhu	$3,6($17)
	lhu	$2,30($16)
	nop
	beq	$3,$2,$L6
	nop

$L8:
	move	$5,$0
$L6:
	lw	$31,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	move	$2,$5
	j	$31
	addiu	$sp,$sp,32

	.set	macro
	.set	reorder
	.end	Check_GRE_rx_net_device
	.section	.text.fast_pptp_filter,"ax",@progbits
	.align	2
	.globl	fast_pptp_filter
	.ent	fast_pptp_filter
	.type	fast_pptp_filter, @function
fast_pptp_filter:
	.set	nomips16
	.frame	$sp,40,$31		# vars= 0, regs= 5/0, args= 16, gp= 0
	.mask	0x800f0000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-40
	sw	$19,28($sp)
	move	$19,$4
	sw	$31,32($sp)
	sw	$18,24($sp)
	sw	$17,20($sp)
	jal	rtl_ip_hdr
	sw	$16,16($sp)

	move	$4,$19
	jal	rtl_get_skb_len
	move	$18,$2

	sltu	$2,$2,56
	bne	$2,$0,$L11
	nop

	lbu	$2,0($18)
	lbu	$3,9($18)
	andi	$2,$2,0xf
	sll	$2,$2,2
	addu	$4,$18,$2
	li	$2,6			# 0x6
	bne	$3,$2,$L11
	li	$2,1723			# 0x6bb

	lhu	$3,0($4)
	nop
	bne	$3,$2,$L11
	nop

	lw	$2,12($4)
	nop
	srl	$2,$2,28
	sll	$2,$2,2
	addu	$4,$4,$2
	lhu	$7,2($4)
	li	$2,1			# 0x1
	addiu	$8,$4,8
	andi	$5,$7,0xffff
	bne	$5,$2,$L14
	addiu	$6,$4,12

	lhu	$3,8($4)
	li	$2,8			# 0x8
	bne	$3,$2,$L14
	nop

	lbu	$2,4($6)
	nop
	bne	$2,$5,$L14
	lui	$16,%hi(pptpAccInfo)

	addiu	$5,$16,%lo(pptpAccInfo)
	lw	$2,36($5)
	nop
	beq	$2,$0,$L15
	nop

	lw	$2,20($5)
	nop
	beq	$2,$0,$L15
	nop

	lw	$2,32($5)
	nop
	beq	$2,$0,$L15
	nop

	lhu	$3,12($4)
	lhu	$2,18($5)
	nop
	bne	$2,$3,$L11
	nop

	lhu	$3,30($5)
	lhu	$2,2($6)
	nop
	bne	$3,$2,$L11
	nop

$L15:
	li	$17,1			# 0x1
	lui	$2,%hi(pptp_tcp_finished)
	sw	$17,%lo(pptp_tcp_finished)($2)
	lhu	$2,0($6)
	addiu	$16,$16,%lo(pptpAccInfo)
	move	$4,$19
	sh	$2,18($16)
	lhu	$2,2($6)
	nop
	sh	$2,30($16)
	lw	$2,16($18)
	nop
	sw	$2,20($16)
	lw	$2,12($18)
	sw	$17,36($16)
	jal	rtl_get_skb_dev
	sw	$2,32($16)

	sw	$2,8($16)
	jal	rtl_get_skb_data
	move	$4,$19

	lui	$4,%hi(pptpAccInfo+24)
	addiu	$5,$2,-8
	li	$6,6			# 0x6
	jal	memcpy
	addiu	$4,$4,%lo(pptpAccInfo+24)

	jal	rtl_get_skb_data
	move	$4,$19

	lui	$4,%hi(pptpAccInfo+12)
	addiu	$4,$4,%lo(pptpAccInfo+12)
	addiu	$5,$2,-14
	jal	memcpy
	li	$6,6			# 0x6

	j	$L11
	sw	$17,4($16)

$L14:
	andi	$3,$7,0xffff
	li	$2,1			# 0x1
	bne	$3,$2,$L11
	li	$2,13			# 0xd

	lhu	$3,0($8)
	nop
	bne	$3,$2,$L11
	lui	$4,%hi(pptpAccInfo)

	addiu	$3,$4,%lo(pptpAccInfo)
	lw	$2,36($3)
	nop
	beq	$2,$0,$L29
	lui	$2,%hi(pptp_tcp_finished)

	lw	$2,20($3)
	nop
	beq	$2,$0,$L29
	lui	$2,%hi(pptp_tcp_finished)

	lw	$2,32($3)
	nop
	beq	$2,$0,$L29
	lui	$2,%hi(pptp_tcp_finished)

	lhu	$3,18($3)
	lhu	$2,4($8)
	nop
	bne	$3,$2,$L11
	addiu	$3,$4,%lo(pptpAccInfo)

	lui	$2,%hi(pptp_tcp_finished)
$L29:
	sw	$0,%lo(pptp_tcp_finished)($2)
	sw	$0,4($3)
$L11:
	lw	$31,32($sp)
	lw	$19,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	j	$31
	addiu	$sp,$sp,40

	.set	macro
	.set	reorder
	.end	fast_pptp_filter
	.section	.text.fast_pptp_to_lan,"ax",@progbits
	.align	2
	.globl	fast_pptp_to_lan
	.ent	fast_pptp_to_lan
	.type	fast_pptp_to_lan, @function
fast_pptp_to_lan:
	.set	nomips16
	.frame	$sp,48,$31		# vars= 0, regs= 8/0, args= 16, gp= 0
	.mask	0x807f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-48
	sw	$22,40($sp)
	sw	$21,36($sp)
	sw	$20,32($sp)
	sw	$16,16($sp)
	sw	$31,44($sp)
	sw	$19,28($sp)
	sw	$18,24($sp)
	sw	$17,20($sp)
	lw	$18,0($4)
	lui	$20,%hi(pptpAccInfo)
	move	$21,$4
	addiu	$16,$20,%lo(pptpAccInfo)
	jal	rtl_get_skb_dev
	move	$4,$18

	lw	$3,8($16)
	nop
	bne	$2,$3,$L33
	move	$22,$0

	jal	rtl_ip_hdr
	move	$4,$18

	lbu	$3,9($2)
	li	$2,47			# 0x2f
	bne	$3,$2,$L30
	move	$7,$0

	jal	rtl_get_skb_len
	move	$4,$18

	sltu	$2,$2,20
	bne	$2,$0,$L30
	move	$7,$0

	jal	rtl_ip_hdr
	move	$4,$18

	move	$17,$2
	jal	rtl_get_skb_data
	move	$4,$18

	lbu	$3,0($17)
	nop
	andi	$3,$3,0xf
	sll	$3,$3,2
	addu	$5,$2,$3
	lbu	$2,1($5)
	li	$3,1			# 0x1
	andi	$2,$2,0x7
	bne	$2,$3,$L30
	move	$7,$0

	lhu	$3,2($5)
	li	$2,34827			# 0x880b
	bne	$3,$2,$L30
	li	$19,8			# 0x8

	lhu	$3,6($5)
	lhu	$2,30($16)
	nop
	bne	$3,$2,$L30
	move	$6,$0

	lbu	$2,0($5)
	nop
	andi	$2,$2,0x10
	beq	$2,$0,$L35
	nop

	lw	$2,8($5)
	li	$19,12			# 0xc
	sw	$2,40($16)
$L35:
	lb	$2,1($5)
	lhu	$4,4($5)
	addiu	$3,$19,4
	slt	$2,$2,0
	movn	$19,$3,$2		#RLX4181/RLX4281:conditional move  
	beq	$4,$0,$L33
	addu	$5,$5,$19

	lbu	$3,0($5)
	nop
	bne	$3,$0,$L38
	li	$2,255			# 0xff

	addiu	$5,$5,1
	j	$L39
	li	$6,1			# 0x1

$L38:
	bne	$3,$2,$L39
	li	$2,3			# 0x3

	lbu	$3,1($5)
	nop
	bne	$3,$2,$L39
	nop

	addiu	$5,$5,2
	lbu	$2,0($5)
	nop
	bne	$2,$0,$L39
	li	$6,2			# 0x2

	addiu	$5,$5,1
	li	$6,3			# 0x3
$L39:
	lbu	$3,0($5)
	li	$2,33			# 0x21
	beq	$3,$2,$L43
	li	$2,253			# 0xfd

	bne	$3,$2,$L30
	move	$7,$0

$L43:
	addiu	$6,$6,1
	beq	$6,$0,$L33
	move	$16,$3

	lbu	$2,0($17)
	nop
	andi	$2,$2,0xf
	sll	$2,$2,2
	addu	$2,$2,$19
	addu	$19,$2,$6
	li	$2,33			# 0x21
	beq	$3,$2,$L46
	addiu	$17,$20,%lo(pptpAccInfo)

	lw	$4,56($17)
	nop
	beq	$4,$0,$L30
	move	$7,$0

	jal	rtl_get_ppp_dev_priv
	nop

	beq	$2,$0,$L30
	move	$7,$0

	addiu	$5,$19,-2
	jal	skb_pull
	move	$4,$18

	jal	rtl_get_skb_data
	move	$4,$18

	sb	$16,1($2)
	sb	$0,0($2)
	lw	$4,56($17)
	jal	rtl_get_ppp_dev_priv
	nop

	move	$4,$2
	move	$5,$18
	jal	rtl_ppp_receive_nonmp_frame
	li	$6,1			# 0x1

	move	$18,$2
	beq	$2,$0,$L30
	li	$7,-2			# 0xfffffffffffffffe

	li	$22,1			# 0x1
	j	$L50
	sw	$2,0($21)

$L46:
	move	$5,$19
	jal	skb_pull
	move	$4,$18

$L50:
	jal	rtl_get_skb_data
	move	$4,$18

	andi	$19,$2,0x3
	beq	$19,$0,$L59
	move	$4,$18

	jal	rtl_skb_headroom
	move	$4,$18

	sltu	$2,$2,$19
	bne	$2,$0,$L51
	nop

	jal	rtl_get_skb_data
	move	$4,$18

	move	$4,$18
	jal	rtl_get_skb_data
	subu	$17,$2,$19

	move	$4,$18
	jal	rtl_get_skb_len
	move	$16,$2

	move	$6,$2
	move	$4,$17
	jal	memmove
	move	$5,$16

	move	$4,$18
	move	$5,$19
	jal	rtl_set_skb_data
	li	$6,1			# 0x1

	move	$4,$18
	move	$5,$19
	jal	rtl_set_skb_tail
	li	$6,1			# 0x1

$L51:
	move	$4,$18
$L59:
	jal	rtl_skb_reset_network_header
	lui	$17,%hi(pptpAccInfo)

	addiu	$16,$17,%lo(pptpAccInfo)
	jal	rtl_skb_reset_transport_header
	move	$4,$18

	lw	$4,56($16)
	nop
	beq	$4,$0,$L30
	move	$7,$0

	bne	$22,$0,$L60
	addiu	$2,$17,%lo(pptpAccInfo)

	jal	rtl_get_ppp_dev_priv
	nop

	beq	$2,$0,$L60
	addiu	$2,$17,%lo(pptpAccInfo)

	lw	$4,56($16)
	jal	rtl_get_ppp_dev_priv
	nop

	move	$4,$18
	jal	rtl_get_skb_len
	move	$16,$2

	addiu	$6,$2,-2
	move	$4,$16
	jal	rtl_inc_ppp_stats
	move	$5,$0

	addiu	$2,$17,%lo(pptpAccInfo)
$L60:
	lw	$5,56($2)
	jal	rtl_set_skb_dev
	move	$4,$18

	j	$L30
	li	$7,1			# 0x1

$L33:
	move	$7,$0
$L30:
	lw	$31,44($sp)
	lw	$22,40($sp)
	lw	$21,36($sp)
	lw	$20,32($sp)
	lw	$19,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	move	$2,$7
	j	$31
	addiu	$sp,$sp,48

	.set	macro
	.set	reorder
	.end	fast_pptp_to_lan
	.section	.text.get_fastpptp_lastxmit,"ax",@progbits
	.align	2
	.globl	get_fastpptp_lastxmit
	.ent	get_fastpptp_lastxmit
	.type	get_fastpptp_lastxmit, @function
get_fastpptp_lastxmit:
	.set	nomips16
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	lui	$2,%hi(fast_pptp_fw)
	lw	$2,%lo(fast_pptp_fw)($2)
	lui	$5,%hi(pptpAccInfo)
	addiu	$3,$5,%lo(pptpAccInfo)
	beq	$2,$0,$L61
	move	$4,$0

	lw	$3,4($3)
	li	$2,1			# 0x1
	bne	$3,$2,$L61
	nop

	lw	$4,%lo(pptpAccInfo)($5)
$L61:
	j	$31
	move	$2,$4

	.set	macro
	.set	reorder
	.end	get_fastpptp_lastxmit
	.rdata
	.align	2
	.type	__func__.0, @object
	.size	__func__.0, 17
__func__.0:
	.ascii	"fast_pptp_to_wan\000"
	.align	2
$LC0:
	.ascii	"ppp0\000"
	.align	2
$LC2:
	.ascii	"RTL\000"
	.section	.text.fast_pptp_to_wan,"ax",@progbits
	.align	2
	.globl	fast_pptp_to_wan
	.ent	fast_pptp_to_wan
	.type	fast_pptp_to_wan, @function
fast_pptp_to_wan:
	.set	nomips16
	.frame	$sp,80,$31		# vars= 40, regs= 6/0, args= 16, gp= 0
	.mask	0x801f0000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-80
	sw	$31,76($sp)
	sw	$18,64($sp)
	sw	$17,60($sp)
	sw	$16,56($sp)
	move	$17,$4
	sw	$20,72($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_ip_hdr
	sw	$19,68($sp)
	.set	macro
	.set	reorder

	lw	$16,12($2)
	move	$4,$17
	li	$2,-16777216			# 0xffffffffff000000
	.set	noreorder
	.set	nomacro
	jal	rtl_ip_hdr
	and	$16,$16,$2
	.set	macro
	.set	reorder

	lw	$3,12($2)
	move	$4,$17
	li	$2,16711680			# 0xff0000
	.set	noreorder
	.set	nomacro
	jal	rtl_ip_hdr
	and	$18,$3,$2
	.set	macro
	.set	reorder

	lw	$2,12($2)
	#nop
	andi	$3,$2,0xff00
	li	$2,167772160			# 0xa000000
	.set	noreorder
	.set	nomacro
	bne	$16,$2,$L64
	li	$2,4194304			# 0x400000
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$18,$2,$L64
	li	$2,16384			# 0x4000
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L89
	lui	$19,%hi(pptpAccInfo)
	.set	macro
	.set	reorder

	lui	$2,%hi(pptp_conn_check)
	lw	$3,%lo(pptp_conn_check)($2)
	li	$2,3			# 0x3
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L63
	move	$4,$0
	.set	macro
	.set	reorder

$L64:
	lui	$19,%hi(pptpAccInfo)
$L89:
	addiu	$18,$19,%lo(pptpAccInfo)
	lw	$4,56($18)
	#nop
	beq	$4,$0,$L83
	jal	rtl_get_ppp_dev_name
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L63
	move	$4,$0
	.set	macro
	.set	reorder

	lw	$4,56($18)
	jal	rtl_get_ppp_dev_name
	lui	$4,%hi($LC0)
	addiu	$3,$4,%lo($LC0)
#APP
	.set	noreorder
	.set	noat
	lbu	$4,($2)
1:	lbu	$1,($3)
	addiu	$2,1
	bne	$1,$4,2f
	addiu	$3,1
	bnez	$4,1b
	lbu	$4,($2)
	nop
	move	$4,$1
2:	subu	$4,$1
3:	.set	at
	.set	reorder
#NO_APP
	.set	noreorder
	.set	nomacro
	bne	$4,$0,$L63
	move	$4,$0
	.set	macro
	.set	reorder

	lw	$2,4($18)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L90
	addiu	$2,$19,%lo(pptpAccInfo)
	.set	macro
	.set	reorder

	lw	$4,56($18)
	#nop
	beq	$4,$0,$L90
	jal	rtl_get_ppp_dev_priv
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L68
	addiu	$2,$19,%lo(pptpAccInfo)
	.set	macro
	.set	reorder

	lw	$4,56($18)
	jal	rtl_get_ppp_dev_priv
	move	$4,$2
	.set	noreorder
	.set	nomacro
	jal	rtl_ppp_vj_check
	move	$16,$2
	.set	macro
	.set	reorder

	li	$3,1			# 0x1
	.set	noreorder
	.set	nomacro
	beq	$2,$3,$L63
	move	$4,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_ip_hdr
	move	$4,$17
	.set	macro
	.set	reorder

	lbu	$20,1($2)
	.set	noreorder
	.set	nomacro
	jal	rtl_get_ppp_xmit_pending
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L63
	move	$4,$0
	.set	macro
	.set	reorder

	lui	$5,%hi($LC2)
	li	$6,3			# 0x3
	addiu	$5,$5,%lo($LC2)
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_cb
	move	$4,$17
	.set	macro
	.set	reorder

	lw	$5,56($18)
	.set	noreorder
	.set	nomacro
	jal	rtl_ppp_start_xmit
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_ppp_xmit_pending
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L63
	li	$4,1			# 0x1
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_ppp_xmit_pending
	move	$4,$16
	.set	macro
	.set	reorder

	move	$17,$2
	move	$4,$16
	.set	noreorder
	.set	nomacro
	jal	rtl_set_ppp_xmit_pending
	move	$5,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_skb_headroom
	move	$4,$17
	.set	macro
	.set	reorder

	sltu	$2,$2,50
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L91
	move	$4,$17
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
	bne	$2,$0,$L91
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_skb_shared
	move	$4,$17
	.set	macro
	.set	reorder

	beq	$2,$0,$L74
	move	$4,$17
$L91:
	.set	noreorder
	.set	nomacro
	jal	skb_realloc_headroom
	li	$5,50			# 0x32
	.set	macro
	.set	reorder

	move	$16,$2
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L63
	move	$4,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	consume_skb
	move	$4,$17
	.set	macro
	.set	reorder

	move	$17,$16
$L74:
	move	$4,$17
	.set	noreorder
	.set	nomacro
	jal	skb_push
	li	$5,50			# 0x32
	.set	macro
	.set	reorder

	lui	$5,%hi(pptpAccInfo+24)
	move	$4,$2
	addiu	$5,$5,%lo(pptpAccInfo+24)
	li	$6,6			# 0x6
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$16,$2
	.set	macro
	.set	reorder

	lui	$5,%hi(pptpAccInfo+12)
	addiu	$4,$16,6
	addiu	$5,$5,%lo(pptpAccInfo+12)
	.set	noreorder
	.set	nomacro
	jal	memcpy
	li	$6,6			# 0x6
	.set	macro
	.set	reorder

	li	$2,8			# 0x8
	sb	$2,12($16)
	sb	$0,13($16)
	lw	$3,16($sp)
	li	$2,268369920			# 0xfff0000
	ori	$2,$2,0xffff
	and	$3,$3,$2
	li	$2,1073741824			# 0x40000000
	or	$3,$3,$2
	li	$2,-251723776			# 0xfffffffff0ff0000
	ori	$2,$2,0xffff
	addiu	$16,$19,%lo(pptpAccInfo)
	and	$3,$3,$2
	li	$2,83886080			# 0x5000000
	lw	$6,32($16)
	lw	$7,20($16)
	or	$3,$3,$2
	li	$2,16384
	sh	$2,22($sp)
	li	$2,47
	move	$5,$0
	move	$4,$17
	sb	$2,25($sp)
	li	$2,64
	sw	$3,16($sp)
	sw	$6,32($sp)
	sw	$7,28($sp)
	sb	$2,24($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_ip_summed
	sb	$20,17($sp)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_len
	move	$4,$17
	.set	macro
	.set	reorder

	lhu	$5,44($16)
	addiu	$2,$2,-14
	sh	$2,18($sp)
	addiu	$5,$5,1
	sh	$5,20($sp)
	lw	$3,20($sp)
	lw	$2,16($sp)
	sh	$0,26($sp)
	lw	$4,24($sp)
	addu	$6,$2,$3
	sltu	$7,$6,$3
	addu	$6,$6,$7
	lw	$3,28($sp)
	addu	$6,$6,$4
	sltu	$7,$6,$4
	addu	$6,$6,$7
	addu	$6,$6,$3
	srl	$2,$2,22
	sltu	$7,$6,$3
	andi	$2,$2,0x3c
	addiu	$8,$sp,16
	sh	$5,44($16)
	addu	$6,$6,$7
	addu	$4,$8,$2
	addiu	$3,$sp,32
$L78:
	lw	$2,0($3)
	addiu	$3,$3,4
	addu	$6,$6,$2
	sltu	$7,$6,$2
	.set	noreorder
	.set	nomacro
	bne	$3,$4,$L78
	addu	$6,$6,$7
	.set	macro
	.set	reorder

	lhu	$2,4($8)
	addiu	$16,$19,%lo(pptpAccInfo)
	move	$4,$17
	sh	$2,46($16)
#APP
		.set	push		# csum_fold
	.set	noat		
	sll	$1, $6, 16	
	addu	$6, $1		
	sltu	$1, $6, $1	
	srl	$6, $6, 16	
	addu	$6, $1		
	xori	$6, 0xffff	
	.set	pop
#NO_APP
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	sh	$6,10($8)
	.set	macro
	.set	reorder

	addiu	$4,$2,14
	addiu	$5,$sp,16
	.set	noreorder
	.set	nomacro
	jal	memcpy
	li	$6,20			# 0x14
	.set	macro
	.set	reorder

	li	$2,48
	sb	$2,40($sp)
	li	$2,-127
	move	$4,$17
	sb	$2,41($sp)
	li	$2,-30709
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_len
	sh	$2,42($sp)
	.set	macro
	.set	reorder

	lw	$3,36($16)
	lhu	$6,18($16)
	lw	$7,40($16)
	move	$5,$3
	addiu	$2,$2,-50
	addiu	$3,$3,1
	sh	$6,46($sp)
	move	$4,$17
	sh	$2,44($sp)
	sw	$3,36($16)
	sw	$7,52($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	sw	$5,48($sp)
	.set	macro
	.set	reorder

	addiu	$4,$2,34
	li	$6,16			# 0x10
	.set	noreorder
	.set	nomacro
	jal	memcpy
	addiu	$5,$sp,40
	.set	macro
	.set	reorder

	lui	$2,%hi(jiffies)
	lw	$2,%lo(jiffies)($2)
	lw	$5,8($16)
	move	$4,$17
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_dev
	sw	$2,%lo(pptpAccInfo)($19)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	dev_queue_xmit
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L63
	li	$4,1			# 0x1
	.set	macro
	.set	reorder

$L68:
$L90:
	lw	$4,56($2)
	#nop
	beq	$4,$0,$L83
	jal	rtl_get_ppp_dev_priv
$L83:
	move	$4,$0
$L63:
	lw	$31,76($sp)
	lw	$20,72($sp)
	lw	$19,68($sp)
	lw	$18,64($sp)
	lw	$17,60($sp)
	lw	$16,56($sp)
	move	$2,$4
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,80
	.set	macro
	.set	reorder

	.end	fast_pptp_to_wan
	.section	.text.fast_pptp_sync_rx_seq,"ax",@progbits
	.align	2
	.globl	fast_pptp_sync_rx_seq
	.ent	fast_pptp_sync_rx_seq
	.type	fast_pptp_sync_rx_seq, @function
fast_pptp_sync_rx_seq:
	.set	nomips16
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,16($sp)
	jal	rtl_ip_hdr
	nop

	addiu	$4,$2,20
	move	$5,$2
	lbu	$2,1($4)
	li	$3,1			# 0x1
	andi	$2,$2,0x7
	bne	$2,$3,$L92
	li	$2,34827			# 0x880b

	lhu	$3,2($4)
	nop
	bne	$3,$2,$L92
	lui	$2,%hi(pptpAccInfo)

	addiu	$6,$2,%lo(pptpAccInfo)
	lbu	$2,20($5)
	nop
	andi	$2,$2,0x10
	beq	$2,$0,$L94
	nop

	lw	$2,52($6)
	nop
	move	$3,$2
	addiu	$2,$2,1
	sw	$2,52($6)
	sw	$3,8($4)
$L94:
	lb	$2,1($4)
	nop
	bgez	$2,$L92
	lui	$2,%hi(pptpAccInfo+48)

	lw	$2,%lo(pptpAccInfo+48)($2)
	nop
	sw	$2,12($4)
$L92:
	lw	$31,16($sp)
	nop
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	fast_pptp_sync_rx_seq
	.section	.text.sync_tx_pptp_gre_seqno,"ax",@progbits
	.align	2
	.globl	sync_tx_pptp_gre_seqno
	.ent	sync_tx_pptp_gre_seqno
	.type	sync_tx_pptp_gre_seqno, @function
sync_tx_pptp_gre_seqno:
	.set	nomips16
	.frame	$sp,48,$31		# vars= 8, regs= 5/0, args= 16, gp= 0
	.mask	0x800f0000,-8
	.fmask	0x00000000,0
	addiu	$sp,$sp,-48
	sw	$18,32($sp)
	sw	$17,28($sp)
	sw	$16,24($sp)
	sw	$31,40($sp)
	sw	$19,36($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	move	$18,$4
	.set	macro
	.set	reorder

	move	$17,$2
	lui	$2,%hi(fast_pptp_fw)
	lw	$2,%lo(fast_pptp_fw)($2)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L96
	addiu	$16,$17,14
	.set	macro
	.set	reorder

#APP
	raw_local_irq_save	$19
#NO_APP
	lbu	$3,9($16)
	li	$2,47			# 0x2f
	bne	$3,$2,$L99
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_len
	move	$4,$18
	.set	macro
	.set	reorder

	sltu	$2,$2,35
	bne	$2,$0,$L99
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	move	$4,$18
	.set	macro
	.set	reorder

	lbu	$3,14($17)
	move	$10,$0
	addiu	$8,$sp,16
	andi	$3,$3,0xf
	sll	$3,$3,2
	addu	$2,$2,$3
	addiu	$7,$2,16
	addiu	$6,$2,14
	move	$5,$0
$L103:
	addu	$2,$7,$5
	lbu	$4,0($2)
	addu	$3,$8,$5
	addiu	$5,$5,1
	slt	$2,$5,2
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L103
	sb	$4,0($3)
	.set	macro
	.set	reorder

	lbu	$2,1($6)
	li	$3,1			# 0x1
	andi	$2,$2,0x7
	.set	noreorder
	.set	nomacro
	bne	$2,$3,$L99
	li	$2,34827			# 0x880b
	.set	macro
	.set	reorder

	lhu	$3,16($sp)
	#nop
	bne	$3,$2,$L99
	lbu	$2,0($6)
	#nop
	andi	$2,$2,0x10
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L106
	lui	$9,%hi(pptpAccInfo)
	.set	macro
	.set	reorder

	addiu	$4,$9,%lo(pptpAccInfo)
	lhu	$3,4($16)
	lhu	$2,46($4)
	#nop
	beq	$3,$2,$L107
	lhu	$3,6($6)
	lhu	$2,18($4)
	#nop
	bne	$3,$2,$L107
	lw	$3,32($4)
	lw	$2,16($16)
	#nop
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L107
	lui	$2,%hi(pptpAccInfo+48)
	.set	macro
	.set	reorder

	lw	$3,8($6)
	addiu	$7,$2,%lo(pptpAccInfo+48)
	sw	$3,20($sp)
	addiu	$8,$sp,20
	move	$5,$0
$L114:
	addu	$2,$8,$5
	lbu	$4,0($2)
	addu	$3,$7,$5
	addiu	$5,$5,1
	slt	$2,$5,4
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L114
	sb	$4,0($3)
	.set	macro
	.set	reorder

	addiu	$2,$9,%lo(pptpAccInfo)
	lw	$3,36($2)
	addiu	$7,$6,8
	move	$5,$0
	move	$4,$3
	addiu	$3,$3,1
	sw	$3,36($2)
	sw	$4,20($sp)
$L119:
	addu	$2,$8,$5
	lbu	$4,0($2)
	addu	$3,$7,$5
	addiu	$5,$5,1
	slt	$2,$5,4
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L119
	sb	$4,0($3)
	.set	macro
	.set	reorder

	j	$L106
$L107:
	li	$10,1			# 0x1
$L106:
	lb	$2,1($6)
	#nop
	.set	noreorder
	.set	nomacro
	bgez	$2,$L122
	lui	$2,%hi(pptpAccInfo)
	.set	macro
	.set	reorder

	addiu	$4,$2,%lo(pptpAccInfo)
	lhu	$3,6($6)
	lhu	$2,18($4)
	#nop
	bne	$3,$2,$L99
	lw	$3,32($4)
	lw	$2,16($16)
	#nop
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L99
	move	$5,$0
	.set	macro
	.set	reorder

	lw	$2,40($4)
	addiu	$6,$6,12
	addiu	$7,$sp,20
	sw	$2,20($sp)
$L129:
	addu	$2,$7,$5
	lbu	$4,0($2)
	addu	$3,$6,$5
	addiu	$5,$5,1
	slt	$2,$5,4
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L129
	sb	$4,0($3)
	.set	macro
	.set	reorder

$L122:
	bne	$10,$0,$L99
	lui	$3,%hi(pptpAccInfo)
	addiu	$3,$3,%lo(pptpAccInfo)
	lhu	$2,44($3)
	addiu	$7,$16,16
	addiu	$2,$2,1
	sh	$2,44($3)
	sh	$2,4($16)
	lw	$3,4($16)
	lw	$2,0($16)
	sh	$0,10($16)
	lw	$4,8($16)
	addu	$5,$2,$3
	sltu	$6,$5,$3
	addu	$5,$5,$6
	lw	$3,12($16)
	addu	$5,$5,$4
	sltu	$6,$5,$4
	addu	$5,$5,$6
	addu	$5,$5,$3
	srl	$2,$2,22
	sltu	$6,$5,$3
	andi	$2,$2,0x3c
	addu	$5,$5,$6
	addu	$3,$16,$2
$L132:
	lw	$2,0($7)
	addiu	$7,$7,4
	addu	$5,$5,$2
	sltu	$6,$5,$2
	.set	noreorder
	.set	nomacro
	bne	$7,$3,$L132
	addu	$5,$5,$6
	.set	macro
	.set	reorder

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
	sh	$5,10($16)
$L99:
#APP
	raw_local_irq_restore	$19
#NO_APP
$L96:
	lw	$31,40($sp)
	lw	$19,36($sp)
	lw	$18,32($sp)
	lw	$17,28($sp)
	lw	$16,24($sp)
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,48
	.set	macro
	.set	reorder

	.end	sync_tx_pptp_gre_seqno
	.rdata
	.align	2
$LC7:
	.ascii	"%d\n\000"
	.section	.text.ppfw_read_proc,"ax",@progbits
	.align	2
	.globl	ppfw_read_proc
	.ent	ppfw_read_proc
	.type	ppfw_read_proc, @function
ppfw_read_proc:
	.set	nomips16
	.frame	$sp,40,$31		# vars= 0, regs= 5/0, args= 16, gp= 0
	.mask	0x800f0000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-40
	sw	$19,28($sp)
	sw	$18,24($sp)
	sw	$17,20($sp)
	sw	$16,16($sp)
	sw	$31,32($sp)
	lui	$2,%hi(fast_pptp_fw)
	move	$17,$6
	lw	$6,%lo(fast_pptp_fw)($2)
	move	$19,$5
	lui	$5,%hi($LC7)
	addiu	$5,$5,%lo($LC7)
	move	$16,$7
	jal	sprintf
	move	$18,$4

	subu	$4,$2,$17
	addu	$3,$17,$16
	slt	$3,$3,$2
	slt	$2,$16,$4
	movn	$4,$16,$2		#RLX4181/RLX4281:conditional move
	addu	$18,$18,$17
	bne	$3,$0,$L151
	slt	$5,$4,0

	lw	$2,56($sp)
	li	$3,1			# 0x1
	sw	$3,0($2)
$L151:
	sw	$18,0($19)
	lw	$31,32($sp)
	lw	$19,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	move	$2,$0
	movz	$2,$4,$5		#RLX4181/RLX4281:conditional move  
	j	$31
	addiu	$sp,$sp,40

	.set	macro
	.set	reorder
	.end	ppfw_read_proc
	.section	.text.atoi_dec,"ax",@progbits
	.align	2
	.ent	atoi_dec
	.type	atoi_dec, @function
atoi_dec:
	.set	nomips16
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	lbu	$5,0($4)
	move	$6,$4
	addiu	$2,$5,-48
	andi	$2,$2,0x00ff
	sltu	$2,$2,10
	beq	$2,$0,$L159
	move	$7,$0

$L157:
	addiu	$6,$6,1
	sll	$2,$5,24
	lbu	$5,0($6)
	sll	$3,$7,2
	addu	$3,$3,$7
	addiu	$4,$5,-48
	sra	$2,$2,24
	sll	$3,$3,1
	andi	$4,$4,0x00ff
	addu	$3,$3,$2
	sltu	$4,$4,10
	bne	$4,$0,$L157
	addiu	$7,$3,-48

$L159:
	j	$31
	move	$2,$7

	.set	macro
	.set	reorder
	.end	atoi_dec
	.section	.text.pptpconn_read_proc,"ax",@progbits
	.align	2
	.globl	pptpconn_read_proc
	.ent	pptpconn_read_proc
	.type	pptpconn_read_proc, @function
pptpconn_read_proc:
	.set	nomips16
	.frame	$sp,40,$31		# vars= 0, regs= 5/0, args= 16, gp= 0
	.mask	0x800f0000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-40
	sw	$19,28($sp)
	sw	$18,24($sp)
	sw	$17,20($sp)
	sw	$16,16($sp)
	sw	$31,32($sp)
	lui	$2,%hi(pptp_conn_check)
	move	$17,$6
	lw	$6,%lo(pptp_conn_check)($2)
	move	$19,$5
	lui	$5,%hi($LC7)
	addiu	$5,$5,%lo($LC7)
	move	$16,$7
	jal	sprintf
	move	$18,$4

	subu	$4,$2,$17
	addu	$3,$17,$16
	slt	$3,$3,$2
	slt	$2,$16,$4
	movn	$4,$16,$2		#RLX4181/RLX4281:conditional move
	addu	$18,$18,$17
	bne	$3,$0,$L161
	slt	$5,$4,0

	lw	$2,56($sp)
	li	$3,1			# 0x1
	sw	$3,0($2)
$L161:
	sw	$18,0($19)
	lw	$31,32($sp)
	lw	$19,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	move	$2,$0
	movz	$2,$4,$5		#RLX4181/RLX4281:conditional move  
	j	$31
	addiu	$sp,$sp,40

	.set	macro
	.set	reorder
	.end	pptpconn_read_proc
	.section	.text.pptpconn_write_proc,"ax",@progbits
	.align	2
	.globl	pptpconn_write_proc
	.ent	pptpconn_write_proc
	.type	pptpconn_write_proc, @function
pptpconn_write_proc:
	.set	nomips16
	.frame	$sp,224,$31		# vars= 200, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	addiu	$sp,$sp,-224
	sltu	$2,$6,2
	sw	$31,216($sp)
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L164
	move	$3,$6
	.set	macro
	.set	reorder

	addu	$2,$5,$6
	or	$2,$5,$2
	.set	noreorder
	.set	nomacro
	beq	$5,$0,$L164
	or	$7,$2,$6
	.set	macro
	.set	reorder

	lw	$2,24($28)
	addiu	$13,$sp,16
	and	$2,$2,$7
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L167
	move	$4,$13
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
	move	$3,$6
$L167:
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L164
	move	$4,$13
	.set	macro
	.set	reorder

	jal	atoi_dec
	lui	$3,%hi(pptp_conn_check)
	sw	$2,%lo(pptp_conn_check)($3)
$L164:
	lw	$31,216($sp)
	li	$2,-14			# 0xfffffffffffffff2
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,224
	.set	macro
	.set	reorder

	.end	pptpconn_write_proc
	.section	.text.ppfw_write_proc,"ax",@progbits
	.align	2
	.globl	ppfw_write_proc
	.ent	ppfw_write_proc
	.type	ppfw_write_proc, @function
ppfw_write_proc:
	.set	nomips16
	.frame	$sp,40,$31		# vars= 8, regs= 4/0, args= 16, gp= 0
	.mask	0x80070000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-40
	sltu	$2,$6,2
	sw	$18,32($sp)
	sw	$16,24($sp)
	sw	$31,36($sp)
	sw	$17,28($sp)
	move	$18,$6
	move	$16,$5
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L174
	li	$3,-14			# 0xfffffffffffffff2
	.set	macro
	.set	reorder

$L180:
$L221:
	addiu	$4,$6,32
	.set	noreorder
	.set	nomacro
	jal	__kmalloc
	li	$5,32			# 0x20
	.set	macro
	.set	reorder

	move	$17,$2
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L174
	li	$3,-14			# 0xfffffffffffffff2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$16,$0,$L225
	addu	$2,$16,$18
	.set	macro
	.set	reorder

	lw	$3,24($28)
	or	$2,$16,$2
	or	$2,$2,$18
	and	$3,$3,$2
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L226
	move	$6,$18
	.set	macro
	.set	reorder

	move	$5,$16
	move	$4,$17
#APP
	.set	noreorder
	jal	__copy_user
	.set	noat
	addu	$1, $5, $6
	.set	at
	.set	reorder
#NO_APP
$L226:
	bne	$6,$0,$L225
	.set	noreorder
	.set	nomacro
	jal	atoi_dec
	move	$4,$17
	.set	macro
	.set	reorder

	lui	$4,%hi(pptpAccInfo)
	lui	$16,%hi(fast_pptp_fw)
	addiu	$4,$4,%lo(pptpAccInfo)
	move	$5,$0
	li	$6,64			# 0x40
	.set	noreorder
	.set	nomacro
	jal	memset
	sw	$2,%lo(fast_pptp_fw)($16)
	.set	macro
	.set	reorder

	lw	$2,%lo(fast_pptp_fw)($16)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L233
	lui	$3,%hi(sync_tx_pptp_gre_seqno_hook)
	.set	macro
	.set	reorder

	lui	$2,%hi(sync_tx_pptp_gre_seqno)
	addiu	$2,$2,%lo(sync_tx_pptp_gre_seqno)
	.set	noreorder
	.set	nomacro
	j	$L234
	sw	$2,%lo(sync_tx_pptp_gre_seqno_hook)($3)
	.set	macro
	.set	reorder

$L233:
	sw	$0,%lo(sync_tx_pptp_gre_seqno_hook)($3)
$L234:
	.set	noreorder
	.set	nomacro
	jal	kfree
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L174
	move	$3,$18
	.set	macro
	.set	reorder

$L225:
	.set	noreorder
	.set	nomacro
	jal	kfree
	move	$4,$17
	.set	macro
	.set	reorder

	li	$3,-14			# 0xfffffffffffffff2
$L174:
	lw	$31,36($sp)
	lw	$18,32($sp)
	lw	$17,28($sp)
	lw	$16,24($sp)
	move	$2,$3
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,40
	.set	macro
	.set	reorder

	.end	ppfw_write_proc
	.rdata
	.align	2
$LC8:
	.ascii	"fast_pptp\000"
	.align	2
$LC9:
	.ascii	"pptp_conn_ck\000"
	.section	.init.text,"ax",@progbits
	.align	2
	.globl	fast_pptp_init
	.ent	fast_pptp_init
	.type	fast_pptp_init, @function
fast_pptp_init:
	.set	nomips16
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	lui	$4,%hi($LC8)
	addiu	$sp,$sp,-24
	addiu	$4,$4,%lo($LC8)
	move	$5,$0
	sw	$31,16($sp)
	jal	create_proc_entry
	move	$6,$0

	lui	$4,%hi($LC9)
	lui	$7,%hi(res1)
	move	$3,$2
	addiu	$4,$4,%lo($LC9)
	move	$5,$0
	move	$6,$0
	beq	$2,$0,$L236
	sw	$2,%lo(res1)($7)

	lui	$2,%hi(ppfw_read_proc)
	addiu	$2,$2,%lo(ppfw_read_proc)
	sw	$2,64($3)
	lw	$3,%lo(res1)($7)
	lui	$2,%hi(ppfw_write_proc)
	addiu	$2,$2,%lo(ppfw_write_proc)
	sw	$2,68($3)
$L236:
	jal	create_proc_entry
	nop

	lui	$4,%hi(pptpAccInfo)
	lui	$7,%hi(res_check_pptp)
	move	$3,$2
	addiu	$4,$4,%lo(pptpAccInfo)
	move	$5,$0
	li	$6,64			# 0x40
	beq	$2,$0,$L237
	sw	$2,%lo(res_check_pptp)($7)

	lui	$2,%hi(pptpconn_read_proc)
	addiu	$2,$2,%lo(pptpconn_read_proc)
	sw	$2,64($3)
	lw	$3,%lo(res_check_pptp)($7)
	lui	$2,%hi(pptpconn_write_proc)
	addiu	$2,$2,%lo(pptpconn_write_proc)
	sw	$2,68($3)
$L237:
	jal	memset
	nop

	lw	$31,16($sp)
	move	$2,$0
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	fast_pptp_init
	.section	.exit.text,"ax",@progbits
	.align	2
	.globl	fast_pptp_exit
	.ent	fast_pptp_exit
	.type	fast_pptp_exit, @function
fast_pptp_exit:
	.set	nomips16
	.frame	$sp,32,$31		# vars= 0, regs= 3/0, args= 16, gp= 0
	.mask	0x80030000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-32
	sw	$16,16($sp)
	lui	$16,%hi(res1)
	lw	$2,%lo(res1)($16)
	lui	$4,%hi($LC8)
	sw	$17,20($sp)
	sw	$31,24($sp)
	lui	$17,%hi(res_check_pptp)
	addiu	$4,$4,%lo($LC8)
	beq	$2,$0,$L239
	move	$5,$2

	jal	remove_proc_entry
	nop

	sw	$0,%lo(res1)($16)
$L239:
	lw	$2,%lo(res_check_pptp)($17)
	lui	$4,%hi($LC9)
	addiu	$4,$4,%lo($LC9)
	beq	$2,$0,$L238
	move	$5,$2

	jal	remove_proc_entry
	nop

	sw	$0,%lo(res_check_pptp)($17)
$L238:
	lw	$31,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	j	$31
	addiu	$sp,$sp,32

	.set	macro
	.set	reorder
	.end	fast_pptp_exit
	.local	pptpAccInfo
	.comm	pptpAccInfo,64,4
	.ident	"GCC: (GNU) 3.4.6-1.3.6"
