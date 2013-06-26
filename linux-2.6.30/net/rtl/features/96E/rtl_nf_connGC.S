	.file	1 "rtl_nf_connGC.c"
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
	.globl	drop_priority
	.data
	.align	2
	.type	drop_priority, @object
	.size	drop_priority, 40
drop_priority:
	.byte	8
	.space	1
	.half	60
	.byte	6
	.space	1
	.half	30
	.byte	5
	.space	1
	.half	60
	.byte	7
	.space	1
	.half	120
	.byte	4
	.space	1
	.half	120
	.byte	11
	.space	1
	.half	85
	.byte	1
	.space	1
	.half	110
	.byte	2
	.space	1
	.half	30
	.byte	12
	.space	1
	.half	10
	.byte	3
	.space	1
	.half	120
	.globl	Udp_State_Hash_Head
	.align	2
	.type	Udp_State_Hash_Head, @object
	.size	Udp_State_Hash_Head, 16
Udp_State_Hash_Head:
	.byte	11
	.space	3
	.word	udp_unreply_list
	.byte	12
	.space	3
	.word	udp_assured_list
	.align	2
	.type	udp_assured_list, @object
	.size	udp_assured_list, 8
udp_assured_list:
	.word	udp_assured_list
	.word	udp_assured_list
	.align	2
	.type	udp_unreply_list, @object
	.size	udp_unreply_list, 8
udp_unreply_list:
	.word	udp_unreply_list
	.word	udp_unreply_list
	.globl	Tcp_State_Hash_Head
	.align	2
	.type	Tcp_State_Hash_Head, @object
	.size	Tcp_State_Hash_Head, 88
Tcp_State_Hash_Head:
	.word	0
	.word	0
	.word	1
	.word	syn_sent_list
	.word	2
	.word	syn_recv_list
	.word	3
	.word	established_list
	.word	4
	.word	fin_wait_list
	.word	5
	.word	close_wait_list
	.word	6
	.word	last_ack_list
	.word	7
	.word	time_wait_list
	.word	8
	.word	close_list
	.word	9
	.word	listen_list
	.word	10
	.word	0
	.align	2
	.type	listen_list, @object
	.size	listen_list, 8
listen_list:
	.word	listen_list
	.word	listen_list
	.align	2
	.type	close_list, @object
	.size	close_list, 8
close_list:
	.word	close_list
	.word	close_list
	.align	2
	.type	time_wait_list, @object
	.size	time_wait_list, 8
time_wait_list:
	.word	time_wait_list
	.word	time_wait_list
	.align	2
	.type	last_ack_list, @object
	.size	last_ack_list, 8
last_ack_list:
	.word	last_ack_list
	.word	last_ack_list
	.align	2
	.type	close_wait_list, @object
	.size	close_wait_list, 8
close_wait_list:
	.word	close_wait_list
	.word	close_wait_list
	.align	2
	.type	fin_wait_list, @object
	.size	fin_wait_list, 8
fin_wait_list:
	.word	fin_wait_list
	.word	fin_wait_list
	.align	2
	.type	established_list, @object
	.size	established_list, 8
established_list:
	.word	established_list
	.word	established_list
	.align	2
	.type	syn_recv_list, @object
	.size	syn_recv_list, 8
syn_recv_list:
	.word	syn_recv_list
	.word	syn_recv_list
	.align	2
	.type	syn_sent_list, @object
	.size	syn_sent_list, 8
syn_sent_list:
	.word	syn_sent_list
	.word	syn_sent_list
	.globl	conntrack_max
	.align	2
	.type	conntrack_max, @object
	.size	conntrack_max, 4
conntrack_max:
	.word	100
	.globl	conntrack_min
	.align	2
	.type	conntrack_min, @object
	.size	conntrack_min, 4
conntrack_min:
	.word	0
	.section	.text.conntrack_dointvec,"ax",@progbits
	.align	2
	.globl	conntrack_dointvec
	.ent	conntrack_dointvec
	.type	conntrack_dointvec, @function
conntrack_dointvec:
	.set	nomips16
	.frame	$sp,32,$31		# vars= 0, regs= 2/0, args= 24, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-32
	lw	$2,48($sp)
	sw	$16,24($sp)
	sw	$31,28($sp)
	sw	$2,16($sp)
	lw	$2,52($sp)
	move	$16,$5
	jal	proc_dointvec
	sw	$2,20($sp)

	bne	$2,$0,$L1
	move	$3,$2

	beq	$16,$0,$L1
	move	$3,$0

	lui	$2,%hi(nf_conntrack_max)
	lw	$6,%lo(nf_conntrack_max)($2)
	lui	$2,%hi(_prot_limit)
	addiu	$9,$2,%lo(_prot_limit)
	lui	$2,%hi(prot_limit)
	addiu	$8,$2,%lo(prot_limit)
	li	$2,1374355456			# 0x51eb0000
	ori	$7,$2,0x851f
	move	$5,$0
	sll	$3,$5,2
$L15:
	addu	$2,$3,$8
	lw	$2,0($2)
	addiu	$5,$5,1
	addu	$3,$3,$9
	mult	$6,$2
	slt	$4,$5,3
	mflo	$2
	multu	$2,$7
	mfhi	$2
	srl	$2,$2,5
	sw	$2,0($3)
	bne	$4,$0,$L15
	sll	$3,$5,2

	li	$3,-859045888			# 0xffffffffcccc0000
	sll	$2,$6,2
	ori	$3,$3,0xcccd
	multu	$2,$3
	lui	$4,%hi(rtl_nf_conntrack_threshold)
	mfhi	$2
	srl	$2,$2,2
	subu	$3,$6,$2
	sltu	$3,$3,65
	bne	$3,$0,$L8
	sw	$2,%lo(rtl_nf_conntrack_threshold)($4)

	addiu	$2,$6,-64
	sw	$2,%lo(rtl_nf_conntrack_threshold)($4)
$L8:
	lui	$2,%hi(nf_conntrack_max)
	lw	$2,%lo(nf_conntrack_max)($2)
	nop
	beq	$2,$0,$L16
	lui	$2,%hi(rtl_newGC_session_status_flags)

	jal	rtl_gc_threshold_check
	move	$4,$0

	bne	$2,$0,$L9
	lui	$2,%hi(rtl_newGC_session_status_flags)

	lui	$4,%hi(rtl_newGC_session_status_flags)
	lw	$2,%lo(rtl_newGC_session_status_flags)($4)
	li	$3,3			# 0x3
	beq	$2,$3,$L3
	lui	$2,%hi(jiffies)

	lw	$2,%lo(jiffies)($2)
	sw	$3,%lo(rtl_newGC_session_status_flags)($4)
	lui	$3,%hi(rtl_newGC_session_status_time)
	addiu	$2,$2,1
	j	$L3
	sw	$2,%lo(rtl_newGC_session_status_time)($3)

$L9:
$L16:
	sw	$0,%lo(rtl_newGC_session_status_flags)($2)
$L3:
	move	$3,$0
$L1:
	lw	$31,28($sp)
	lw	$16,24($sp)
	move	$2,$3
	j	$31
	addiu	$sp,$sp,32

	.set	macro
	.set	reorder
	.end	conntrack_dointvec
	.section	.text.conntrack_dointvec_minmax,"ax",@progbits
	.align	2
	.globl	conntrack_dointvec_minmax
	.ent	conntrack_dointvec_minmax
	.type	conntrack_dointvec_minmax, @function
conntrack_dointvec_minmax:
	.set	nomips16
	.frame	$sp,32,$31		# vars= 0, regs= 2/0, args= 24, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-32
	lw	$2,48($sp)
	sw	$16,24($sp)
	sw	$31,28($sp)
	sw	$2,16($sp)
	lw	$2,52($sp)
	move	$16,$5
	jal	proc_dointvec_minmax
	sw	$2,20($sp)

	bne	$2,$0,$L17
	move	$3,$2

	beq	$16,$0,$L17
	move	$3,$0

	lui	$2,%hi(nf_conntrack_max)
	lw	$6,%lo(nf_conntrack_max)($2)
	lui	$2,%hi(_prot_limit)
	addiu	$9,$2,%lo(_prot_limit)
	lui	$2,%hi(prot_limit)
	addiu	$8,$2,%lo(prot_limit)
	li	$2,1374355456			# 0x51eb0000
	ori	$7,$2,0x851f
	move	$5,$0
	sll	$3,$5,2
$L31:
	addu	$2,$3,$8
	lw	$2,0($2)
	addiu	$5,$5,1
	addu	$3,$3,$9
	mult	$6,$2
	slt	$4,$5,3
	mflo	$2
	multu	$2,$7
	mfhi	$2
	srl	$2,$2,5
	sw	$2,0($3)
	bne	$4,$0,$L31
	sll	$3,$5,2

	li	$3,-859045888			# 0xffffffffcccc0000
	sll	$2,$6,2
	ori	$3,$3,0xcccd
	multu	$2,$3
	lui	$4,%hi(rtl_nf_conntrack_threshold)
	mfhi	$2
	srl	$2,$2,2
	subu	$3,$6,$2
	sltu	$3,$3,65
	bne	$3,$0,$L24
	sw	$2,%lo(rtl_nf_conntrack_threshold)($4)

	addiu	$2,$6,-64
	sw	$2,%lo(rtl_nf_conntrack_threshold)($4)
$L24:
	lui	$2,%hi(nf_conntrack_max)
	lw	$2,%lo(nf_conntrack_max)($2)
	nop
	beq	$2,$0,$L32
	lui	$2,%hi(rtl_newGC_session_status_flags)

	jal	rtl_gc_threshold_check
	move	$4,$0

	bne	$2,$0,$L25
	lui	$2,%hi(rtl_newGC_session_status_flags)

	lui	$4,%hi(rtl_newGC_session_status_flags)
	lw	$2,%lo(rtl_newGC_session_status_flags)($4)
	li	$3,3			# 0x3
	beq	$2,$3,$L19
	lui	$2,%hi(jiffies)

	lw	$2,%lo(jiffies)($2)
	sw	$3,%lo(rtl_newGC_session_status_flags)($4)
	lui	$3,%hi(rtl_newGC_session_status_time)
	addiu	$2,$2,1
	j	$L19
	sw	$2,%lo(rtl_newGC_session_status_time)($3)

$L25:
$L32:
	sw	$0,%lo(rtl_newGC_session_status_flags)($2)
$L19:
	move	$3,$0
$L17:
	lw	$31,28($sp)
	lw	$16,24($sp)
	move	$2,$3
	j	$31
	addiu	$sp,$sp,32

	.set	macro
	.set	reorder
	.end	conntrack_dointvec_minmax
	.section	.text.clean_from_lists,"ax",@progbits
	.align	2
	.globl	clean_from_lists
	.ent	clean_from_lists
	.type	clean_from_lists, @function
clean_from_lists:
	.set	nomips16
	.frame	$sp,32,$31		# vars= 0, regs= 3/0, args= 16, gp= 0
	.mask	0x80030000,-8
	.fmask	0x00000000,0
	addiu	$sp,$sp,-32
	sw	$17,20($sp)
	sw	$16,16($sp)
	move	$17,$5
	move	$16,$4
	sw	$31,24($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_hlist_nulls_del_rcu
	move	$5,$0
	.set	macro
	.set	reorder

	move	$4,$16
	.set	noreorder
	.set	nomacro
	jal	rtl_hlist_nulls_del_rcu
	li	$5,1			# 0x1
	.set	macro
	.set	reorder

	jal	local_bh_disable
	move	$4,$16
	.set	noreorder
	.set	nomacro
	jal	rtl_new_gc_get_ct_protonum
	move	$5,$0
	.set	macro
	.set	reorder

	move	$3,$2
	li	$2,6			# 0x6
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L40
	slt	$2,$3,7
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L49
	li	$2,17			# 0x11
	.set	macro
	.set	reorder

	li	$2,1			# 0x1
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L46
	move	$4,$16
	.set	macro
	.set	reorder

	j	$L58
$L49:
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L43
	move	$4,$16
	.set	macro
	.set	reorder

	j	$L58
$L40:
	lui	$4,%hi(prot_counters+4)
#APP
	raw_local_irq_save	$3
#NO_APP
	lw	$2,%lo(prot_counters+4)($4)
	#nop
	addiu	$2,$2,-1
	sw	$2,%lo(prot_counters+4)($4)
	j	$L57
$L43:
	lui	$4,%hi(prot_counters+8)
#APP
	raw_local_irq_save	$3
#NO_APP
	lw	$2,%lo(prot_counters+8)($4)
	#nop
	addiu	$2,$2,-1
	sw	$2,%lo(prot_counters+8)($4)
	j	$L57
$L46:
	lui	$4,%hi(prot_counters)
#APP
	raw_local_irq_save	$3
#NO_APP
	lw	$2,%lo(prot_counters)($4)
	#nop
	addiu	$2,$2,-1
	sw	$2,%lo(prot_counters)($4)
$L57:
#APP
	raw_local_irq_restore	$3
#NO_APP
	move	$4,$16
$L58:
	.set	noreorder
	.set	nomacro
	jal	rtl_new_gc_get_ct_protonum
	move	$5,$0
	.set	macro
	.set	reorder

	li	$3,6			# 0x6
	.set	noreorder
	.set	nomacro
	beq	$2,$3,$L59
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_new_gc_get_ct_protonum
	move	$5,$0
	.set	macro
	.set	reorder

	li	$3,17			# 0x11
	.set	noreorder
	.set	nomacro
	bne	$2,$3,$L56
	move	$4,$16
	.set	macro
	.set	reorder

$L59:
	.set	noreorder
	.set	nomacro
	jal	clean_from_lists_hooks
	move	$5,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_list_del
	move	$4,$16
	.set	macro
	.set	reorder

	lui	$2,%hi(nf_conntrack_max)
	lw	$2,%lo(nf_conntrack_max)($2)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L60
	lui	$2,%hi(rtl_newGC_session_status_flags)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_gc_threshold_check
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L52
	lui	$2,%hi(rtl_newGC_session_status_flags)
	.set	macro
	.set	reorder

	lui	$4,%hi(rtl_newGC_session_status_flags)
	lw	$2,%lo(rtl_newGC_session_status_flags)($4)
	li	$3,3			# 0x3
	.set	noreorder
	.set	nomacro
	beq	$2,$3,$L56
	lui	$2,%hi(jiffies)
	.set	macro
	.set	reorder

	lw	$2,%lo(jiffies)($2)
	sw	$3,%lo(rtl_newGC_session_status_flags)($4)
	lui	$3,%hi(rtl_newGC_session_status_time)
	addiu	$2,$2,1
	.set	noreorder
	.set	nomacro
	j	$L56
	sw	$2,%lo(rtl_newGC_session_status_time)($3)
	.set	macro
	.set	reorder

$L52:
$L60:
	sw	$0,%lo(rtl_newGC_session_status_flags)($2)
$L56:
	jal	local_bh_enable
	move	$4,$16
	lw	$31,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	.set	noreorder
	.set	nomacro
	j	nf_ct_remove_expectations
	addiu	$sp,$sp,32
	.set	macro
	.set	reorder

	.end	clean_from_lists
	.section	.text.rtl_death_action,"ax",@progbits
	.align	2
	.globl	rtl_death_action
	.ent	rtl_death_action
	.type	rtl_death_action, @function
rtl_death_action:
	.set	nomips16
	.frame	$sp,32,$31		# vars= 0, regs= 3/0, args= 16, gp= 0
	.mask	0x80030000,-8
	.fmask	0x00000000,0
	addiu	$sp,$sp,-32
	sw	$17,20($sp)
	sw	$16,16($sp)
	sw	$31,24($sp)
	lw	$3,200($4)
	lui	$2,%hi(init_net)
	move	$16,$4
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L68
	addiu	$17,$2,%lo(init_net)
	.set	macro
	.set	reorder

	lbu	$2,8($3)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L68
	addu	$3,$3,$2
	.set	macro
	.set	reorder

	beq	$3,$0,$L68
	lw	$2,0($3)
	#nop
	beq	$2,$0,$L68
	lw	$2,64($2)
	#nop
	beq	$2,$0,$L68
	jal	$2
$L68:
	jal	local_bh_disable
	.set	noreorder
	.set	nomacro
	jal	rtl_nf_ct_stat_inc
	move	$4,$17
	.set	macro
	.set	reorder

	move	$4,$16
	.set	noreorder
	.set	nomacro
	jal	clean_from_lists
	move	$5,$17
	.set	macro
	.set	reorder

	jal	local_bh_enable
	beq	$16,$0,$L61
#APP
	raw_local_irq_save	$3
#NO_APP
	lw	$2,0($16)
	#nop
	addiu	$2,$2,-1
	sw	$2,0($16)
#APP
	raw_local_irq_restore	$3
#NO_APP
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L61
	move	$4,$16
	.set	macro
	.set	reorder

	lw	$31,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	.set	noreorder
	.set	nomacro
	j	nf_conntrack_destroy
	addiu	$sp,$sp,32
	.set	macro
	.set	reorder

$L61:
	lw	$31,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,32
	.set	macro
	.set	reorder

	.end	rtl_death_action
	.section	.text.rtl_connGC_addList,"ax",@progbits
	.align	2
	.globl	rtl_connGC_addList
	.ent	rtl_connGC_addList
	.type	rtl_connGC_addList, @function
rtl_connGC_addList:
	.set	nomips16
	.frame	$sp,32,$31		# vars= 0, regs= 3/0, args= 16, gp= 0
	.mask	0x80030000,-8
	.fmask	0x00000000,0
	addiu	$sp,$sp,-32
	sw	$17,20($sp)
	sw	$16,16($sp)
	sw	$31,24($sp)
	move	$17,$5
	.set	noreorder
	.set	nomacro
	jal	rtl_new_gc_ip_hdr
	move	$16,$4
	.set	macro
	.set	reorder

	beq	$2,$0,$L89
	.set	noreorder
	.set	nomacro
	jal	rtl_new_gc_get_skb_protocol
	move	$4,$16
	.set	macro
	.set	reorder

	move	$3,$2
	li	$2,6			# 0x6
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L92
	slt	$2,$3,7
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L104
	li	$2,17			# 0x11
	.set	macro
	.set	reorder

	li	$2,1			# 0x1
	beq	$3,$2,$L101
	j	$L89
$L104:
	beq	$3,$2,$L96
	j	$L89
$L92:
	.set	noreorder
	.set	nomacro
	jal	rtl_new_gc_get_ct_tcp_state
	move	$4,$17
	.set	macro
	.set	reorder

	lui	$3,%hi(Tcp_State_Hash_Head)
	sll	$2,$2,3
	addiu	$3,$3,%lo(Tcp_State_Hash_Head)
	addu	$2,$2,$3
	lw	$2,4($2)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L107
	lui	$4,%hi(prot_counters+4)
	.set	macro
	.set	reorder

	move	$4,$17
	li	$5,1			# 0x1
	.set	noreorder
	.set	nomacro
	jal	rtl_list_add_tail
	move	$6,$0
	.set	macro
	.set	reorder

	lui	$4,%hi(prot_counters+4)
$L107:
#APP
	raw_local_irq_save	$3
#NO_APP
	lw	$2,%lo(prot_counters+4)($4)
	#nop
	addiu	$2,$2,1
	sw	$2,%lo(prot_counters+4)($4)
	j	$L106
$L96:
	.set	noreorder
	.set	nomacro
	jal	rtl_new_gc_get_ct_udp_status
	move	$4,$17
	.set	macro
	.set	reorder

	andi	$2,$2,0x2
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L97
	move	$4,$17
	.set	macro
	.set	reorder

	li	$5,2			# 0x2
	.set	noreorder
	.set	nomacro
	j	$L105
	li	$6,1			# 0x1
	.set	macro
	.set	reorder

$L97:
	li	$5,2			# 0x2
	move	$6,$0
$L105:
	jal	rtl_list_add_tail
	lui	$4,%hi(prot_counters+8)
#APP
	raw_local_irq_save	$3
#NO_APP
	lw	$2,%lo(prot_counters+8)($4)
	#nop
	addiu	$2,$2,1
	sw	$2,%lo(prot_counters+8)($4)
	j	$L106
$L101:
	lui	$4,%hi(prot_counters)
#APP
	raw_local_irq_save	$3
#NO_APP
	lw	$2,%lo(prot_counters)($4)
	#nop
	addiu	$2,$2,1
	sw	$2,%lo(prot_counters)($4)
$L106:
#APP
	raw_local_irq_restore	$3
#NO_APP
$L89:
	lw	$31,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	move	$2,$0
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,32
	.set	macro
	.set	reorder

	.end	rtl_connGC_addList
	.section	.text.__nf_ct_refresh_acct_proto,"ax",@progbits
	.align	2
	.globl	__nf_ct_refresh_acct_proto
	.ent	__nf_ct_refresh_acct_proto
	.type	__nf_ct_refresh_acct_proto, @function
__nf_ct_refresh_acct_proto:
	.set	nomips16
	.frame	$sp,56,$31		# vars= 8, regs= 6/0, args= 24, gp= 0
	.mask	0x801f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-56
	sw	$16,32($sp)
	move	$16,$4
	sw	$20,48($sp)
	sw	$19,44($sp)
	sw	$18,40($sp)
	sw	$17,36($sp)
	move	$19,$5
	sw	$31,52($sp)
	move	$20,$6
	sw	$0,24($sp)
	lbu	$17,79($sp)
	jal	local_bh_disable
	move	$18,$7

	move	$4,$16
	jal	rtl_test_bit
	li	$5,10			# 0xa

	beq	$2,$0,$L128
	move	$4,$16

	jal	rtl_test_bit
	li	$5,3			# 0x3

	li	$3,-1			# 0xffffffffffffffff
	bne	$2,$3,$L115
	move	$5,$18

	jal	rtl_new_gc_set_ct_timeout_expires
	move	$4,$16

	li	$2,8			# 0x8
	j	$L116
	sw	$2,24($sp)

$L115:
	jal	rtl_del_ct_timer
	move	$4,$16

	beq	$2,$0,$L116
	lui	$2,%hi(jiffies)

	lw	$5,%lo(jiffies)($2)
	move	$4,$16
	jal	rtl_new_gc_set_ct_timeout_expires
	addu	$5,$5,$18

	jal	rtl_add_ct_timer
	move	$4,$16

	li	$2,8			# 0x8
	sw	$2,24($sp)
	li	$2,6			# 0x6
	beq	$17,$2,$L119
	li	$2,17			# 0x11

	beq	$17,$2,$L120
	nop

	j	$L116
	nop

$L119:
	lw	$6,84($sp)
	move	$4,$16
	j	$L127
	li	$5,1			# 0x1

$L120:
	jal	rtl_new_gc_get_ct_udp_status
	move	$4,$16

	andi	$2,$2,0x2
	beq	$2,$0,$L121
	move	$4,$16

	li	$5,2			# 0x2
	j	$L127
	li	$6,1			# 0x1

$L121:
	li	$5,2			# 0x2
	move	$6,$0
$L127:
	jal	rtl_list_move_tail
	nop

$L116:
	lw	$7,72($sp)
	addiu	$2,$sp,24
	move	$4,$16
	move	$5,$19
	move	$6,$20
	jal	__nf_ct_refresh_acct_proto_hooks
	sw	$2,16($sp)

$L128:
	jal	local_bh_enable
	nop

	lw	$31,52($sp)
	lw	$20,48($sp)
	lw	$19,44($sp)
	lw	$18,40($sp)
	lw	$17,36($sp)
	lw	$16,32($sp)
	j	$31
	addiu	$sp,$sp,56

	.set	macro
	.set	reorder
	.end	__nf_ct_refresh_acct_proto
	.section	.text.__conntrack_drop_check,"ax",@progbits
	.align	2
	.globl	__conntrack_drop_check
	.ent	__conntrack_drop_check
	.type	__conntrack_drop_check, @function
__conntrack_drop_check:
	.set	nomips16
	.frame	$sp,56,$31		# vars= 0, regs= 9/0, args= 16, gp= 0
	.mask	0x80ff0000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-56
	sw	$16,16($sp)
	move	$5,$0
	move	$16,$4
	sw	$31,48($sp)
	sw	$23,44($sp)
	sw	$22,40($sp)
	sw	$21,36($sp)
	sw	$20,32($sp)
	sw	$19,28($sp)
	sw	$18,24($sp)
	jal	rtl_new_gc_get_ct_protonum
	sw	$17,20($sp)

	move	$4,$16
	move	$5,$0
	jal	rtl_new_gc_get_ct_port_by_dir
	move	$6,$0

	move	$4,$16
	move	$5,$0
	li	$6,1			# 0x1
	jal	rtl_new_gc_get_ct_port_by_dir
	move	$23,$2

	move	$4,$16
	li	$5,1			# 0x1
	move	$6,$0
	jal	rtl_new_gc_get_ct_port_by_dir
	move	$22,$2

	move	$4,$16
	li	$5,1			# 0x1
	li	$6,1			# 0x1
	jal	rtl_new_gc_get_ct_port_by_dir
	move	$21,$2

	move	$4,$16
	move	$5,$0
	move	$6,$0
	jal	rtl_new_gc_get_ct_ip_by_dir
	move	$20,$2

	move	$4,$16
	move	$5,$0
	li	$6,1			# 0x1
	jal	rtl_new_gc_get_ct_ip_by_dir
	move	$19,$2

	move	$4,$16
	li	$5,1			# 0x1
	move	$6,$0
	jal	rtl_new_gc_get_ct_ip_by_dir
	move	$17,$2

	move	$4,$16
	li	$5,1			# 0x1
	li	$6,1			# 0x1
	jal	rtl_new_gc_get_ct_ip_by_dir
	move	$18,$2

	li	$5,-268435456			# 0xfffffffff0000000
	move	$3,$2
	li	$4,-536870912			# 0xffffffffe0000000
	and	$2,$17,$5
	beq	$2,$4,$L131
	and	$2,$3,$5

	beq	$2,$4,$L129
	li	$2,-1			# 0xffffffffffffffff

	bne	$19,$3,$L134
	sltu	$2,$23,1024

	beq	$17,$18,$L131
	nop

$L134:
	bne	$2,$0,$L129
	li	$2,-1			# 0xffffffffffffffff

	sltu	$2,$22,1024
	bne	$2,$0,$L129
	li	$2,-1			# 0xffffffffffffffff

	sltu	$2,$21,1024
	bne	$2,$0,$L129
	li	$2,-1			# 0xffffffffffffffff

	sltu	$2,$20,1024
	bne	$2,$0,$L131
	li	$3,8080			# 0x1f90

	beq	$23,$3,$L131
	nop

	beq	$22,$3,$L131
	nop

	beq	$21,$3,$L129
	li	$2,-1			# 0xffffffffffffffff

	bne	$20,$3,$L129
	move	$2,$0

$L131:
	li	$2,-1			# 0xffffffffffffffff
$L129:
	lw	$31,48($sp)
	lw	$23,44($sp)
	lw	$22,40($sp)
	lw	$21,36($sp)
	lw	$20,32($sp)
	lw	$19,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	j	$31
	addiu	$sp,$sp,56

	.set	macro
	.set	reorder
	.end	__conntrack_drop_check
	.section	.text.isReservedConntrack,"ax",@progbits
	.align	2
	.globl	isReservedConntrack
	.ent	isReservedConntrack
	.type	isReservedConntrack, @function
isReservedConntrack:
	.set	nomips16
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	lbu	$3,38($4)
	li	$2,1			# 0x1
	beq	$3,$2,$L135
	li	$7,1			# 0x1

	lw	$6,20($4)
	li	$8,-268435456			# 0xfffffffff0000000
	li	$9,-536870912			# 0xffffffffe0000000
	and	$2,$6,$8
	lw	$3,0($4)
	beq	$2,$9,$L146
	li	$7,-1			# 0xffffffffffffffff

	beq	$6,$7,$L146
	nop

	beq	$3,$0,$L146
	nop

	lw	$3,20($5)
	lw	$6,0($5)
	and	$2,$3,$8
	beq	$2,$9,$L146
	nop

	beq	$3,$7,$L146
	nop

	beq	$6,$0,$L146
	li	$3,80			# 0x50

	lhu	$2,16($4)
	nop
	beq	$2,$3,$L146
	li	$6,8080			# 0x1f90

	beq	$2,$6,$L146
	nop

	lhu	$4,36($4)
	nop
	beq	$4,$3,$L146
	nop

	beq	$4,$6,$L146
	nop

	lhu	$2,16($5)
	nop
	beq	$2,$3,$L146
	nop

	beq	$2,$6,$L146
	nop

	lhu	$5,36($5)
	nop
	beq	$5,$3,$L163
	li	$7,1			# 0x1

	bne	$5,$6,$L135
	move	$7,$0

$L146:
	li	$7,1			# 0x1
$L135:
$L163:
	j	$31
	move	$2,$7

	.set	macro
	.set	reorder
	.end	isReservedConntrack
	.section	.text.drop_one_conntrack,"ax",@progbits
	.align	2
	.globl	drop_one_conntrack
	.ent	drop_one_conntrack
	.type	drop_one_conntrack, @function
drop_one_conntrack:
	.set	nomips16
	.frame	$sp,64,$31		# vars= 0, regs= 10/0, args= 24, gp= 0
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
	lui	$2,%hi(_prot_limit+4)
	lui	$3,%hi(prot_counters+4)
	lw	$2,%lo(_prot_limit+4)($2)
	lw	$3,%lo(prot_counters+4)($3)
	nop
	slt	$2,$2,$3
	beq	$2,$0,$L350
	lui	$2,%hi(_prot_limit+8)

	jal	local_bh_disable
	lui	$20,%hi(drop_priority_max_idx)

	lw	$2,%lo(drop_priority_max_idx)($20)
	nop
	blez	$2,$L311
	move	$19,$0

	lui	$3,%hi(drop_priority)
$L351:
	addiu	$3,$3,%lo(drop_priority)
	sll	$2,$19,2
	addu	$2,$2,$3
	lbu	$3,0($2)
	nop
	sltu	$2,$3,10
	beq	$2,$0,$L171
	lui	$2,%hi(Tcp_State_Hash_Head)

	addiu	$2,$2,%lo(Tcp_State_Hash_Head)
	sll	$3,$3,3
	addu	$3,$3,$2
	lw	$18,4($3)
	nop
	lw	$2,0($18)
	nop
	beq	$2,$18,$L171
	move	$17,$2

$L339:
	beq	$17,$18,$L171
	addiu	$16,$17,-204

	lb	$3,212($16)
	li	$2,-1			# 0xffffffffffffffff
	bne	$3,$2,$L178
	move	$4,$16

	jal	__conntrack_drop_check
	nop

	sb	$2,212($16)
$L178:
	lb	$2,212($16)
	nop
	bne	$2,$0,$L177
	move	$4,$16

	jal	rtl_del_ct_timer
	nop

	bne	$2,$0,$L325
	nop

$L177:
	lw	$17,0($17)
	j	$L339
	nop

$L171:
	lw	$2,%lo(drop_priority_max_idx)($20)
	addiu	$19,$19,1
	slt	$2,$19,$2
	bne	$2,$0,$L351
	lui	$3,%hi(drop_priority)

$L311:
	jal	local_bh_enable
	nop

	lui	$2,%hi(_prot_limit+8)
$L350:
	lui	$3,%hi(prot_counters+8)
	lw	$2,%lo(_prot_limit+8)($2)
	lw	$3,%lo(prot_counters+8)($3)
	nop
	slt	$2,$2,$3
	beq	$2,$0,$L189
	lui	$fp,%hi(drop_priority_max_idx)

	jal	local_bh_disable
	lui	$20,%hi(drop_priority_max_idx)

	lw	$2,%lo(drop_priority_max_idx)($20)
	nop
	blez	$2,$L314
	move	$19,$0

	lui	$3,%hi(drop_priority)
$L352:
	addiu	$3,$3,%lo(drop_priority)
	sll	$2,$19,2
	addu	$2,$2,$3
	lbu	$3,0($2)
	nop
	sltu	$2,$3,11
	bne	$2,$0,$L195
	lui	$2,%hi(Udp_State_Hash_Head)

	addiu	$2,$2,%lo(Udp_State_Hash_Head)
	sll	$3,$3,3
	addu	$3,$3,$2
	lw	$18,-84($3)
	nop
	lw	$2,0($18)
	nop
	beq	$2,$18,$L195
	move	$17,$2

$L340:
	beq	$17,$18,$L195
	addiu	$16,$17,-204

	lb	$3,212($16)
	li	$2,-1			# 0xffffffffffffffff
	bne	$3,$2,$L202
	move	$4,$16

	jal	__conntrack_drop_check
	nop

	sb	$2,212($16)
$L202:
	lb	$2,212($16)
	nop
	bne	$2,$0,$L201
	move	$4,$16

	jal	rtl_del_ct_timer
	nop

	bne	$2,$0,$L325
	nop

$L201:
	lw	$17,0($17)
	j	$L340
	nop

$L195:
	lw	$2,%lo(drop_priority_max_idx)($20)
	addiu	$19,$19,1
	slt	$2,$19,$2
	bne	$2,$0,$L352
	lui	$3,%hi(drop_priority)

$L314:
	jal	local_bh_enable
	lui	$fp,%hi(drop_priority_max_idx)

	j	$L348
	nop

$L325:
	jal	local_bh_enable
	nop

	jal	rtl_death_action
	move	$4,$16

	j	$L349
	lui	$4,%hi(rtl_newGC_session_status_flags)

$L189:
$L348:
	jal	local_bh_disable
	move	$19,$0

	lw	$2,%lo(drop_priority_max_idx)($fp)
	nop
	blez	$2,$L317
	lui	$22,%hi(drop_priority)

$L353:
	sll	$21,$19,2
	addiu	$2,$22,%lo(drop_priority)
	addu	$2,$21,$2
	lbu	$2,0($2)
	nop
	sltu	$3,$2,10
	beq	$3,$0,$L221
	sll	$4,$2,3

	lui	$2,%hi(Tcp_State_Hash_Head)
	addiu	$2,$2,%lo(Tcp_State_Hash_Head)
	addu	$2,$4,$2
	lw	$18,4($2)
	nop
	lw	$2,0($18)
	nop
	beq	$2,$18,$L242
	li	$23,3			# 0x3

	move	$17,$2
	move	$20,$0
$L341:
	beq	$17,$18,$L242
	addiu	$16,$17,-204

	lb	$3,212($16)
	li	$2,-1			# 0xffffffffffffffff
	move	$4,$16
	bne	$3,$2,$L228
	addiu	$20,$20,1

	jal	__conntrack_drop_check
	nop

	sb	$2,212($16)
$L228:
	lb	$2,212($16)
	nop
	bne	$2,$0,$L227
	move	$4,$16

	jal	rtl_get_ct_timer_expires
	nop

	lui	$3,%hi(jiffies)
	lw	$4,%lo(jiffies)($3)
	addiu	$3,$22,%lo(drop_priority)
	addu	$3,$21,$3
	lhu	$3,2($3)
	subu	$2,$2,$4
	srl	$2,$2,7
	sltu	$3,$3,$2
	bne	$3,$0,$L230
	move	$4,$16

	jal	rtl_del_ct_timer
	nop

	move	$4,$16
	move	$5,$19
	move	$6,$0
	beq	$2,$0,$L230
	li	$7,1			# 0x1

	jal	__drop_one_conntrack_process_hooks1
	sw	$23,16($sp)

	move	$3,$2
	li	$2,1			# 0x1
	move	$4,$16
	move	$5,$19
	move	$6,$0
	beq	$3,$2,$L326
	li	$7,1			# 0x1

	li	$2,2			# 0x2
	beq	$3,$2,$L253
	li	$3,1			# 0x1

	jal	__drop_one_conntrack_process_hooks2
	sw	$23,16($sp)

$L230:
	slt	$2,$20,129
	beq	$2,$0,$L253
	move	$3,$0

$L227:
	lw	$17,0($17)
	j	$L341
	nop

$L326:
	jal	local_bh_enable
	nop

	jal	rtl_death_action
	move	$4,$16

	j	$L253
	li	$3,1			# 0x1

$L221:
	lui	$2,%hi(Udp_State_Hash_Head)
	addiu	$2,$2,%lo(Udp_State_Hash_Head)
	addu	$2,$4,$2
	lw	$18,-84($2)
	nop
	lw	$2,0($18)
	nop
	beq	$2,$18,$L242
	li	$23,12			# 0xc

	move	$17,$2
	move	$20,$0
$L343:
	beq	$17,$18,$L242
	addiu	$16,$17,-204

	lb	$3,212($16)
	li	$2,-1			# 0xffffffffffffffff
	move	$4,$16
	bne	$3,$2,$L247
	addiu	$20,$20,1

	jal	__conntrack_drop_check
	nop

	sb	$2,212($16)
$L247:
	lb	$2,212($16)
	nop
	bne	$2,$0,$L246
	move	$4,$16

	jal	rtl_get_ct_timer_expires
	nop

	lui	$3,%hi(jiffies)
	lw	$4,%lo(jiffies)($3)
	addiu	$3,$22,%lo(drop_priority)
	addu	$3,$21,$3
	lhu	$3,2($3)
	subu	$2,$2,$4
	srl	$2,$2,7
	sltu	$3,$3,$2
	bne	$3,$0,$L249
	move	$4,$16

	jal	rtl_del_ct_timer
	nop

	move	$4,$16
	move	$5,$19
	move	$6,$0
	beq	$2,$0,$L249
	li	$7,1			# 0x1

	jal	__drop_one_conntrack_process_hooks1
	sw	$23,16($sp)

	move	$3,$2
	li	$2,1			# 0x1
	move	$4,$16
	move	$5,$19
	move	$6,$0
	beq	$3,$2,$L326
	li	$7,1			# 0x1

	li	$2,2			# 0x2
	beq	$3,$2,$L253
	li	$3,1			# 0x1

	jal	__drop_one_conntrack_process_hooks2
	sw	$23,16($sp)

$L249:
	slt	$2,$20,129
	beq	$2,$0,$L253
	move	$3,$0

$L246:
	lw	$17,0($17)
	j	$L343
	nop

$L242:
	move	$3,$0
$L253:
	li	$2,1			# 0x1
	beq	$3,$2,$L214
	addiu	$19,$19,1

	lw	$2,%lo(drop_priority_max_idx)($fp)
	nop
	slt	$2,$19,$2
	bne	$2,$0,$L353
	lui	$22,%hi(drop_priority)

$L317:
	jal	local_bh_enable
	lui	$fp,%hi(drop_priority_max_idx)

	jal	local_bh_disable
	move	$19,$0

	lw	$2,%lo(drop_priority_max_idx)($fp)
	nop
	blez	$2,$L321
	lui	$22,%hi(drop_priority)

$L354:
	sll	$21,$19,2
	addiu	$2,$22,%lo(drop_priority)
	addu	$2,$21,$2
	lbu	$2,0($2)
	nop
	sltu	$3,$2,10
	beq	$3,$0,$L267
	sll	$4,$2,3

	lui	$2,%hi(Tcp_State_Hash_Head)
	addiu	$2,$2,%lo(Tcp_State_Hash_Head)
	addu	$2,$4,$2
	lw	$18,4($2)
	nop
	lw	$2,0($18)
	nop
	beq	$2,$18,$L288
	li	$23,3			# 0x3

	move	$17,$2
	move	$20,$0
$L344:
	beq	$17,$18,$L288
	addiu	$16,$17,-204

	lb	$3,212($16)
	li	$2,-1			# 0xffffffffffffffff
	move	$4,$16
	bne	$3,$2,$L274
	addiu	$20,$20,1

	jal	__conntrack_drop_check
	nop

	sb	$2,212($16)
$L274:
	lb	$2,212($16)
	nop
	bne	$2,$0,$L273
	move	$4,$16

	jal	rtl_get_ct_timer_expires
	nop

	lui	$3,%hi(jiffies)
	lw	$4,%lo(jiffies)($3)
	addiu	$3,$22,%lo(drop_priority)
	addu	$3,$21,$3
	lhu	$3,2($3)
	subu	$2,$2,$4
	srl	$2,$2,9
	sltu	$3,$3,$2
	bne	$3,$0,$L276
	move	$4,$16

	jal	rtl_del_ct_timer
	nop

	move	$4,$16
	move	$5,$19
	li	$6,2			# 0x2
	beq	$2,$0,$L276
	move	$7,$0

	jal	__drop_one_conntrack_process_hooks1
	sw	$23,16($sp)

	move	$3,$2
	li	$2,1			# 0x1
	move	$4,$16
	move	$5,$19
	li	$6,2			# 0x2
	beq	$3,$2,$L331
	move	$7,$0

	li	$2,2			# 0x2
	beq	$3,$2,$L299
	li	$3,1			# 0x1

	jal	__drop_one_conntrack_process_hooks2
	sw	$23,16($sp)

$L276:
	slt	$2,$20,129
	beq	$2,$0,$L299
	move	$3,$0

$L273:
	lw	$17,0($17)
	j	$L344
	nop

$L331:
	jal	local_bh_enable
	nop

	jal	rtl_death_action
	move	$4,$16

	j	$L299
	li	$3,1			# 0x1

$L267:
	lui	$2,%hi(Udp_State_Hash_Head)
	addiu	$2,$2,%lo(Udp_State_Hash_Head)
	addu	$2,$4,$2
	lw	$18,-84($2)
	nop
	lw	$2,0($18)
	nop
	beq	$2,$18,$L288
	li	$23,12			# 0xc

	move	$17,$2
	move	$20,$0
$L346:
	beq	$17,$18,$L288
	addiu	$16,$17,-204

	lb	$3,212($16)
	li	$2,-1			# 0xffffffffffffffff
	move	$4,$16
	bne	$3,$2,$L293
	addiu	$20,$20,1

	jal	__conntrack_drop_check
	nop

	sb	$2,212($16)
$L293:
	lb	$2,212($16)
	nop
	bne	$2,$0,$L292
	move	$4,$16

	jal	rtl_get_ct_timer_expires
	nop

	lui	$3,%hi(jiffies)
	lw	$4,%lo(jiffies)($3)
	addiu	$3,$22,%lo(drop_priority)
	addu	$3,$21,$3
	lhu	$3,2($3)
	subu	$2,$2,$4
	srl	$2,$2,9
	sltu	$3,$3,$2
	bne	$3,$0,$L295
	move	$4,$16

	jal	rtl_del_ct_timer
	nop

	move	$4,$16
	move	$5,$19
	li	$6,2			# 0x2
	beq	$2,$0,$L295
	move	$7,$0

	jal	__drop_one_conntrack_process_hooks1
	sw	$23,16($sp)

	move	$3,$2
	li	$2,1			# 0x1
	move	$4,$16
	move	$5,$19
	li	$6,2			# 0x2
	beq	$3,$2,$L331
	move	$7,$0

	li	$2,2			# 0x2
	beq	$3,$2,$L299
	li	$3,1			# 0x1

	jal	__drop_one_conntrack_process_hooks2
	sw	$23,16($sp)

$L295:
	slt	$2,$20,129
	beq	$2,$0,$L299
	move	$3,$0

$L292:
	lw	$17,0($17)
	j	$L346
	nop

$L288:
	move	$3,$0
$L299:
	li	$2,1			# 0x1
	beq	$3,$2,$L214
	addiu	$19,$19,1

	lw	$2,%lo(drop_priority_max_idx)($fp)
	nop
	slt	$2,$19,$2
	bne	$2,$0,$L354
	lui	$22,%hi(drop_priority)

	j	$L321
	nop

$L214:
	lui	$4,%hi(rtl_newGC_session_status_flags)
$L349:
	lw	$2,%lo(rtl_newGC_session_status_flags)($4)
	nop
	bne	$2,$0,$L307
	nop

	lui	$2,%hi(jiffies)
	lw	$3,%lo(jiffies)($2)
	li	$2,3			# 0x3
	sw	$2,%lo(rtl_newGC_session_status_flags)($4)
	addiu	$3,$3,1
	lui	$2,%hi(rtl_newGC_session_status_time)
	sw	$3,%lo(rtl_newGC_session_status_time)($2)
$L307:
	j	$L164
	li	$2,1			# 0x1

$L321:
	jal	local_bh_enable
	nop

	lui	$4,%hi(rtl_newGC_session_status_flags)
	lw	$2,%lo(rtl_newGC_session_status_flags)($4)
	li	$3,1			# 0x1
	beq	$2,$3,$L309
	lui	$2,%hi(jiffies)

	lw	$2,%lo(jiffies)($2)
	sw	$3,%lo(rtl_newGC_session_status_flags)($4)
	lui	$3,%hi(rtl_newGC_session_status_time)
	addiu	$2,$2,200
	sw	$2,%lo(rtl_newGC_session_status_time)($3)
$L309:
	move	$2,$0
$L164:
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
	.end	drop_one_conntrack
	.section	.text.rtl_nf_conn_GC_init,"ax",@progbits
	.align	2
	.globl	rtl_nf_conn_GC_init
	.ent	rtl_nf_conn_GC_init
	.type	rtl_nf_conn_GC_init, @function
rtl_nf_conn_GC_init:
	.set	nomips16
	.frame	$sp,32,$31		# vars= 0, regs= 3/0, args= 16, gp= 0
	.mask	0x80030000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	lui	$2,%hi(Tcp_State_Hash_Head)
	addiu	$sp,$sp,-32
	addiu	$2,$2,%lo(Tcp_State_Hash_Head)
	sw	$31,24($sp)
	sw	$17,20($sp)
	sw	$16,16($sp)
	addiu	$2,$2,4
	li	$5,10			# 0xa
$L361:
	lw	$3,0($2)
	addiu	$5,$5,-1
	beq	$3,$0,$L358
	addiu	$2,$2,8

	sw	$3,4($3)
	sw	$3,0($3)
$L358:
	bgez	$5,$L361
	nop

	lui	$2,%hi(Udp_State_Hash_Head)
	addiu	$2,$2,%lo(Udp_State_Hash_Head)
	addiu	$2,$2,4
	li	$5,1			# 0x1
$L367:
	lw	$3,0($2)
	addiu	$5,$5,-1
	beq	$3,$0,$L364
	addiu	$2,$2,8

	sw	$3,4($3)
	sw	$3,0($3)
$L364:
	bgez	$5,$L367
	lui	$16,%hi(prot_limit)

	move	$5,$0
	li	$6,12			# 0xc
	jal	memset
	addiu	$4,$16,%lo(prot_limit)

	lui	$2,%hi(prot_counters)
	addiu	$7,$2,%lo(prot_counters)
	lui	$2,%hi(_prot_limit)
	addiu	$6,$2,%lo(_prot_limit)
	move	$5,$0
	sll	$2,$5,2
$L390:
	addiu	$5,$5,1
	addu	$4,$2,$6
	slt	$3,$5,3
	addu	$2,$2,$7
	sw	$0,0($2)
	sw	$0,0($4)
	bne	$3,$0,$L390
	sll	$2,$5,2

	lui	$2,%hi(nf_conntrack_max)
	lw	$6,%lo(nf_conntrack_max)($2)
	li	$3,10			# 0xa
	lui	$2,%hi(drop_priority_max_idx)
	sw	$3,%lo(drop_priority_max_idx)($2)
	li	$2,2			# 0x2
	addiu	$4,$16,%lo(prot_limit)
	sw	$2,%lo(prot_limit)($16)
	li	$2,90			# 0x5a
	sw	$2,4($4)
	li	$2,60			# 0x3c
	sw	$2,8($4)
	lui	$2,%hi(_prot_limit)
	lui	$17,%hi(rtl_newGC_session_status_flags)
	addiu	$9,$2,%lo(_prot_limit)
	li	$2,1374355456			# 0x51eb0000
	sw	$0,%lo(rtl_newGC_session_status_flags)($17)
	move	$8,$4
	ori	$7,$2,0x851f
	move	$5,$0
	sll	$3,$5,2
$L391:
	addu	$2,$3,$8
	lw	$2,0($2)
	addiu	$5,$5,1
	addu	$3,$3,$9
	mult	$6,$2
	slt	$4,$5,3
	mflo	$2
	multu	$2,$7
	mfhi	$2
	srl	$2,$2,5
	sw	$2,0($3)
	bne	$4,$0,$L391
	sll	$3,$5,2

	li	$3,-859045888			# 0xffffffffcccc0000
	sll	$2,$6,2
	ori	$3,$3,0xcccd
	multu	$2,$3
	lui	$4,%hi(rtl_nf_conntrack_threshold)
	mfhi	$2
	srl	$2,$2,2
	subu	$3,$6,$2
	sltu	$3,$3,65
	bne	$3,$0,$L377
	sw	$2,%lo(rtl_nf_conntrack_threshold)($4)

	addiu	$2,$6,-64
	sw	$2,%lo(rtl_nf_conntrack_threshold)($4)
$L377:
	lui	$2,%hi(nf_conntrack_max)
	lw	$2,%lo(nf_conntrack_max)($2)
	nop
	beq	$2,$0,$L378
	nop

	jal	rtl_gc_threshold_check
	move	$4,$0

	bne	$2,$0,$L378
	li	$3,3			# 0x3

	lw	$2,%lo(rtl_newGC_session_status_flags)($17)
	nop
	beq	$2,$3,$L381
	lui	$2,%hi(jiffies)

	lw	$2,%lo(jiffies)($2)
	sw	$3,%lo(rtl_newGC_session_status_flags)($17)
	lui	$3,%hi(rtl_newGC_session_status_time)
	addiu	$2,$2,1
	j	$L381
	sw	$2,%lo(rtl_newGC_session_status_time)($3)

$L378:
	sw	$0,%lo(rtl_newGC_session_status_flags)($17)
$L381:
	jal	rtl_nf_conn_GC_init_hooks
	nop

	lw	$31,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	move	$2,$0
	j	$31
	addiu	$sp,$sp,32

	.set	macro
	.set	reorder
	.end	rtl_nf_conn_GC_init
	.globl	rtl_nf_conntrack_threshold
	.section	.bss
	.align	2
	.type	rtl_nf_conntrack_threshold, @object
	.size	rtl_nf_conntrack_threshold, 4
rtl_nf_conntrack_threshold:
	.space	4
	.globl	drop_priority_max_idx
	.align	2
	.type	drop_priority_max_idx, @object
	.size	drop_priority_max_idx, 4
drop_priority_max_idx:
	.space	4
	.globl	rtl_newGC_session_status_flags
	.section	.dram-fwd,"aw",@progbits
	.align	2
	.type	rtl_newGC_session_status_flags, @object
	.size	rtl_newGC_session_status_flags, 4
rtl_newGC_session_status_flags:
	.space	4
	.globl	rtl_newGC_session_status_time
	.align	2
	.type	rtl_newGC_session_status_time, @object
	.size	rtl_newGC_session_status_time, 4
rtl_newGC_session_status_time:
	.space	4
	.globl	prot_limit
	.section	.bss
	.align	2
	.type	prot_limit, @object
	.size	prot_limit, 12
prot_limit:
	.space	12
	.local	prot_counters
	.comm	prot_counters,12,4
	.local	_prot_limit
	.comm	_prot_limit,12,4
	.ident	"GCC: (GNU) 3.4.6-1.3.6"
