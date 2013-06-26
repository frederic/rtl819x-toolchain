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
	move	$16,$5
	sw	$2,16($sp)
	lw	$2,52($sp)
	sw	$31,28($sp)
	jal	proc_dointvec
	sw	$2,20($sp)

	bne	$2,$0,$L1
	move	$3,$2

	beq	$16,$0,$L1
	move	$3,$0

	lui	$2,%hi(nf_conntrack_max)
	move	$5,$0
	lw	$6,%lo(nf_conntrack_max)($2)
	lui	$2,%hi(_prot_limit)
	addiu	$9,$2,%lo(_prot_limit)
	lui	$2,%hi(prot_limit)
	addiu	$8,$2,%lo(prot_limit)
	li	$2,1374355456			# 0x51eb0000
	ori	$7,$2,0x851f
	sll	$3,$5,2
$L15:
	addiu	$5,$5,1
	addu	$2,$3,$8
	slt	$4,$5,3
	lw	$2,0($2)
	addu	$3,$3,$9
	mult	$6,$2
	mflo	$2
	multu	$2,$7
	mfhi	$2
	srl	$2,$2,5
	sw	$2,0($3)
	bne	$4,$0,$L15
	sll	$3,$5,2

	li	$3,-859045888			# 0xcccc0000
	sll	$2,$6,2
	ori	$3,$3,0xcccd
	lui	$4,%hi(rtl_nf_conntrack_threshold)
	multu	$2,$3
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
	beq	$2,$0,$L16
	lui	$2,%hi(rtl_newGC_session_status_flags)

	jal	rtl_gc_threshold_check
	move	$4,$0

	bne	$2,$0,$L9
	lui	$2,%hi(rtl_newGC_session_status_flags)

	lui	$4,%hi(rtl_newGC_session_status_flags)
	li	$3,3			# 0x3
	lw	$2,%lo(rtl_newGC_session_status_flags)($4)
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
	move	$2,$3
	lw	$16,24($sp)
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
	move	$16,$5
	sw	$2,16($sp)
	lw	$2,52($sp)
	sw	$31,28($sp)
	jal	proc_dointvec_minmax
	sw	$2,20($sp)

	bne	$2,$0,$L17
	move	$3,$2

	beq	$16,$0,$L17
	move	$3,$0

	lui	$2,%hi(nf_conntrack_max)
	move	$5,$0
	lw	$6,%lo(nf_conntrack_max)($2)
	lui	$2,%hi(_prot_limit)
	addiu	$9,$2,%lo(_prot_limit)
	lui	$2,%hi(prot_limit)
	addiu	$8,$2,%lo(prot_limit)
	li	$2,1374355456			# 0x51eb0000
	ori	$7,$2,0x851f
	sll	$3,$5,2
$L31:
	addiu	$5,$5,1
	addu	$2,$3,$8
	slt	$4,$5,3
	lw	$2,0($2)
	addu	$3,$3,$9
	mult	$6,$2
	mflo	$2
	multu	$2,$7
	mfhi	$2
	srl	$2,$2,5
	sw	$2,0($3)
	bne	$4,$0,$L31
	sll	$3,$5,2

	li	$3,-859045888			# 0xcccc0000
	sll	$2,$6,2
	ori	$3,$3,0xcccd
	lui	$4,%hi(rtl_nf_conntrack_threshold)
	multu	$2,$3
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
	beq	$2,$0,$L32
	lui	$2,%hi(rtl_newGC_session_status_flags)

	jal	rtl_gc_threshold_check
	move	$4,$0

	bne	$2,$0,$L25
	lui	$2,%hi(rtl_newGC_session_status_flags)

	lui	$4,%hi(rtl_newGC_session_status_flags)
	li	$3,3			# 0x3
	lw	$2,%lo(rtl_newGC_session_status_flags)($4)
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
	move	$2,$3
	lw	$16,24($sp)
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
	move	$17,$5
	sw	$16,16($sp)
	move	$5,$0
	sw	$31,24($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_hlist_nulls_del_rcu
	move	$16,$4
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
	beq	$2,$0,$L46
	li	$2,17			# 0x11
	.set	macro
	.set	reorder

	li	$2,1			# 0x1
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L44
	move	$4,$16
	.set	macro
	.set	reorder

	j	$L55
$L46:
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L42
	move	$4,$16
	.set	macro
	.set	reorder

	j	$L55
$L40:
	lui	$2,%hi(prot_counters+4)
#APP
	   

	1:	ll	    $3, %lo(prot_counters+4)($2)		# atomic_sub		
       nop                                             
	subu	$3, 1					
	sc	    $3, %lo(prot_counters+4)($2)					
	beqz	$3, 2f					
	.subsection 2					
2:	b	1b					
	.previous					

#NO_APP
	j	$L54
$L42:
	lui	$2,%hi(prot_counters+8)
#APP
	   

	1:	ll	    $3, %lo(prot_counters+8)($2)		# atomic_sub		
       nop                                             
	subu	$3, 1					
	sc	    $3, %lo(prot_counters+8)($2)					
	beqz	$3, 2f					
	.subsection 2					
2:	b	1b					
	.previous					

#NO_APP
	j	$L54
$L44:
	lui	$2,%hi(prot_counters)
#APP
	   

	1:	ll	    $4, %lo(prot_counters)($2)		# atomic_sub		
       nop                                             
	subu	$4, $3					
	sc	    $4, %lo(prot_counters)($2)					
	beqz	$4, 2f					
	.subsection 2					
2:	b	1b					
	.previous					

#NO_APP
$L54:
#APP
	   

#NO_APP
	move	$4,$16
$L55:
	.set	noreorder
	.set	nomacro
	jal	rtl_new_gc_get_ct_protonum
	move	$5,$0
	.set	macro
	.set	reorder

	li	$3,6			# 0x6
	.set	noreorder
	.set	nomacro
	beq	$2,$3,$L56
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
	bne	$2,$3,$L53
	move	$4,$16
	.set	macro
	.set	reorder

$L56:
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
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L57
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
	bne	$2,$0,$L49
	lui	$2,%hi(rtl_newGC_session_status_flags)
	.set	macro
	.set	reorder

	lui	$4,%hi(rtl_newGC_session_status_flags)
	li	$3,3			# 0x3
	lw	$2,%lo(rtl_newGC_session_status_flags)($4)
	.set	noreorder
	.set	nomacro
	beq	$2,$3,$L53
	lui	$2,%hi(jiffies)
	.set	macro
	.set	reorder

	lw	$2,%lo(jiffies)($2)
	sw	$3,%lo(rtl_newGC_session_status_flags)($4)
	lui	$3,%hi(rtl_newGC_session_status_time)
	addiu	$2,$2,1
	.set	noreorder
	.set	nomacro
	j	$L53
	sw	$2,%lo(rtl_newGC_session_status_time)($3)
	.set	macro
	.set	reorder

$L49:
$L57:
	sw	$0,%lo(rtl_newGC_session_status_flags)($2)
$L53:
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
	lui	$2,%hi(init_net)
	sw	$17,20($sp)
	addiu	$17,$2,%lo(init_net)
	sw	$16,16($sp)
	sw	$31,24($sp)
	lw	$3,200($4)
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L65
	move	$16,$4
	.set	macro
	.set	reorder

	lbu	$2,8($3)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L65
	addu	$3,$3,$2
	.set	macro
	.set	reorder

	beq	$3,$0,$L65
	lw	$2,0($3)
	beq	$2,$0,$L65
	lw	$2,64($2)
	beq	$2,$0,$L65
	jal	$2
$L65:
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
	beq	$16,$0,$L58
#APP
	   

	1:	ll	$3, 0($16)		# atomic_sub_return	
       nop                                             
	subu	$2, $3, 1				
	sc	$2, 0($16)					
	beqz	$2, 2f					
	subu	$2, $3, 1				
	.subsection 2					
2:	b	1b					
	.previous					

	   

#NO_APP
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L58
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

$L58:
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
	move	$17,$5
	sw	$16,16($sp)
	sw	$31,24($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_new_gc_ip_hdr
	move	$16,$4
	.set	macro
	.set	reorder

	beq	$2,$0,$L85
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
	beq	$3,$2,$L88
	slt	$2,$3,7
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L97
	li	$2,17			# 0x11
	.set	macro
	.set	reorder

	li	$2,1			# 0x1
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L95
	lui	$2,%hi(prot_counters)
	.set	macro
	.set	reorder

	j	$L85
$L97:
	beq	$3,$2,$L91
	j	$L85
$L88:
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
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L100
	lui	$2,%hi(prot_counters+4)
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

	lui	$2,%hi(prot_counters+4)
$L100:
#APP
	   

	1:	ll	$3, %lo(prot_counters+4)($2)		# atomic_add    
       nop                                             
	addu	$3, 1					
	sc	$3, %lo(prot_counters+4)($2)                      
	beqz	$3, 2f					
	.subsection 2					
2:	b	1b                          
	.previous                       

#NO_APP
	j	$L99
$L91:
	.set	noreorder
	.set	nomacro
	jal	rtl_new_gc_get_ct_udp_status
	move	$4,$17
	.set	macro
	.set	reorder

	andi	$2,$2,0x2
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L92
	move	$4,$17
	.set	macro
	.set	reorder

	li	$5,2			# 0x2
	.set	noreorder
	.set	nomacro
	j	$L98
	li	$6,1			# 0x1
	.set	macro
	.set	reorder

$L92:
	li	$5,2			# 0x2
	move	$6,$0
$L98:
	jal	rtl_list_add_tail
	lui	$2,%hi(prot_counters+8)
#APP
	   

	1:	ll	$3, %lo(prot_counters+8)($2)		# atomic_add    
       nop                                             
	addu	$3, 1					
	sc	$3, %lo(prot_counters+8)($2)                      
	beqz	$3, 2f					
	.subsection 2					
2:	b	1b                          
	.previous                       

#NO_APP
	j	$L99
$L95:
#APP
	   

	1:	ll	$4, %lo(prot_counters)($2)		# atomic_add    
       nop                                             
	addu	$4, $3					
	sc	$4, %lo(prot_counters)($2)                      
	beqz	$4, 2f					
	.subsection 2					
2:	b	1b                          
	.previous                       

#NO_APP
$L99:
#APP
	   

#NO_APP
$L85:
	lw	$31,24($sp)
	move	$2,$0
	lw	$17,20($sp)
	lw	$16,16($sp)
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
	move	$20,$6
	sw	$19,44($sp)
	move	$19,$5
	sw	$18,40($sp)
	sw	$17,36($sp)
	sw	$31,52($sp)
	sw	$0,24($sp)
	lbu	$17,79($sp)
	jal	local_bh_disable
	move	$18,$7

	move	$4,$16
	jal	rtl_test_bit
	li	$5,10			# 0xa

	beq	$2,$0,$L121
	move	$4,$16

	jal	rtl_test_bit
	li	$5,3			# 0x3

	li	$3,-1			# 0xffffffff
	bne	$2,$3,$L108
	move	$5,$18

	jal	rtl_new_gc_set_ct_timeout_expires
	move	$4,$16

	li	$2,8			# 0x8
	j	$L109
	sw	$2,24($sp)

$L108:
	jal	rtl_del_ct_timer
	move	$4,$16

	beq	$2,$0,$L109
	lui	$2,%hi(jiffies)

	move	$4,$16
	lw	$5,%lo(jiffies)($2)
	jal	rtl_new_gc_set_ct_timeout_expires
	addu	$5,$5,$18

	jal	rtl_add_ct_timer
	move	$4,$16

	li	$2,8			# 0x8
	sw	$2,24($sp)
	li	$2,6			# 0x6
	beq	$17,$2,$L112
	li	$2,17			# 0x11

	beq	$17,$2,$L113
	nop

	j	$L109
	nop

$L112:
	lw	$6,84($sp)
	move	$4,$16
	j	$L120
	li	$5,1			# 0x1

$L113:
	jal	rtl_new_gc_get_ct_udp_status
	move	$4,$16

	andi	$2,$2,0x2
	beq	$2,$0,$L114
	move	$4,$16

	li	$5,2			# 0x2
	j	$L120
	li	$6,1			# 0x1

$L114:
	li	$5,2			# 0x2
	move	$6,$0
$L120:
	jal	rtl_list_move_tail
	nop

$L109:
	lw	$7,72($sp)
	addiu	$2,$sp,24
	move	$4,$16
	move	$5,$19
	move	$6,$20
	jal	__nf_ct_refresh_acct_proto_hooks
	sw	$2,16($sp)

$L121:
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
	move	$5,$0
	sw	$16,16($sp)
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

	li	$5,-268435456			# 0xf0000000
	move	$3,$2
	li	$4,-536870912			# 0xe0000000
	and	$2,$17,$5
	beq	$2,$4,$L124
	and	$2,$3,$5

	beq	$2,$4,$L122
	li	$2,-1			# 0xffffffff

	bne	$19,$3,$L127
	sltu	$2,$23,1024

	beq	$17,$18,$L124
	nop

$L127:
	bne	$2,$0,$L122
	li	$2,-1			# 0xffffffff

	sltu	$2,$22,1024
	bne	$2,$0,$L122
	li	$2,-1			# 0xffffffff

	sltu	$2,$21,1024
	bne	$2,$0,$L122
	li	$2,-1			# 0xffffffff

	sltu	$2,$20,1024
	bne	$2,$0,$L124
	li	$3,8080			# 0x1f90

	beq	$23,$3,$L124
	nop

	beq	$22,$3,$L124
	nop

	beq	$21,$3,$L122
	li	$2,-1			# 0xffffffff

	bne	$20,$3,$L122
	move	$2,$0

$L124:
	li	$2,-1			# 0xffffffff
$L122:
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
	beq	$3,$2,$L128
	li	$7,1			# 0x1

	lw	$6,20($4)
	li	$8,-268435456			# 0xf0000000
	li	$9,-536870912			# 0xe0000000
	and	$2,$6,$8
	lw	$3,0($4)
	beq	$2,$9,$L139
	li	$7,-1			# 0xffffffff

	beq	$6,$7,$L139
	nop

	beq	$3,$0,$L139
	nop

	lw	$3,20($5)
	lw	$6,0($5)
	and	$2,$3,$8
	beq	$2,$9,$L139
	nop

	beq	$3,$7,$L139
	nop

	beq	$6,$0,$L139
	li	$3,80			# 0x50

	lhu	$2,16($4)
	beq	$2,$3,$L139
	li	$6,8080			# 0x1f90

	beq	$2,$6,$L139
	nop

	lhu	$4,36($4)
	beq	$4,$3,$L139
	nop

	beq	$4,$6,$L139
	nop

	lhu	$2,16($5)
	beq	$2,$3,$L139
	nop

	beq	$2,$6,$L139
	nop

	lhu	$5,36($5)
	beq	$5,$3,$L156
	li	$7,1			# 0x1

	bne	$5,$6,$L128
	move	$7,$0

$L139:
	li	$7,1			# 0x1
$L128:
$L156:
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
	lui	$2,%hi(_prot_limit+4)
	sw	$31,60($sp)
	lui	$3,%hi(prot_counters+4)
	sw	$fp,56($sp)
	sw	$23,52($sp)
	sw	$22,48($sp)
	sw	$21,44($sp)
	sw	$20,40($sp)
	sw	$19,36($sp)
	sw	$18,32($sp)
	sw	$17,28($sp)
	sw	$16,24($sp)
	lw	$2,%lo(_prot_limit+4)($2)
	lw	$3,%lo(prot_counters+4)($3)
	slt	$2,$2,$3
	beq	$2,$0,$L343
	lui	$2,%hi(_prot_limit+8)

	lui	$20,%hi(drop_priority_max_idx)
	jal	local_bh_disable
	move	$19,$0

	lw	$2,%lo(drop_priority_max_idx)($20)
	blez	$2,$L304
	lui	$3,%hi(drop_priority)

$L344:
	sll	$2,$19,2
	addiu	$3,$3,%lo(drop_priority)
	addu	$2,$2,$3
	lbu	$3,0($2)
	sltu	$2,$3,10
	beq	$2,$0,$L164
	lui	$2,%hi(Tcp_State_Hash_Head)

	sll	$3,$3,3
	addiu	$2,$2,%lo(Tcp_State_Hash_Head)
	addu	$3,$3,$2
	lw	$18,4($3)
	lw	$2,0($18)
	beq	$2,$18,$L164
	move	$17,$2

$L332:
	beq	$17,$18,$L164
	addiu	$16,$17,-204

	li	$2,-1			# 0xffffffff
	lb	$3,212($16)
	bne	$3,$2,$L171
	move	$4,$16

	jal	__conntrack_drop_check
	nop

	sb	$2,212($16)
$L171:
	lb	$2,212($16)
	bne	$2,$0,$L170
	move	$4,$16

	jal	rtl_del_ct_timer
	nop

	bne	$2,$0,$L318
	nop

$L170:
	lw	$17,0($17)
	j	$L332
	nop

$L164:
	lw	$2,%lo(drop_priority_max_idx)($20)
	addiu	$19,$19,1
	slt	$2,$19,$2
	bne	$2,$0,$L344
	lui	$3,%hi(drop_priority)

$L304:
	jal	local_bh_enable
	nop

	lui	$2,%hi(_prot_limit+8)
$L343:
	lui	$3,%hi(prot_counters+8)
	lw	$2,%lo(_prot_limit+8)($2)
	lw	$3,%lo(prot_counters+8)($3)
	slt	$2,$2,$3
	beq	$2,$0,$L182
	lui	$fp,%hi(drop_priority_max_idx)

	lui	$20,%hi(drop_priority_max_idx)
	jal	local_bh_disable
	move	$19,$0

	lw	$2,%lo(drop_priority_max_idx)($20)
	blez	$2,$L307
	lui	$3,%hi(drop_priority)

$L345:
	sll	$2,$19,2
	addiu	$3,$3,%lo(drop_priority)
	addu	$2,$2,$3
	lbu	$3,0($2)
	sltu	$2,$3,11
	bne	$2,$0,$L188
	lui	$2,%hi(Udp_State_Hash_Head)

	sll	$3,$3,3
	addiu	$2,$2,%lo(Udp_State_Hash_Head)
	addu	$3,$3,$2
	lw	$18,-84($3)
	lw	$2,0($18)
	beq	$2,$18,$L188
	move	$17,$2

$L333:
	beq	$17,$18,$L188
	addiu	$16,$17,-204

	li	$2,-1			# 0xffffffff
	lb	$3,212($16)
	bne	$3,$2,$L195
	move	$4,$16

	jal	__conntrack_drop_check
	nop

	sb	$2,212($16)
$L195:
	lb	$2,212($16)
	bne	$2,$0,$L194
	move	$4,$16

	jal	rtl_del_ct_timer
	nop

	bne	$2,$0,$L318
	nop

$L194:
	lw	$17,0($17)
	j	$L333
	nop

$L188:
	lw	$2,%lo(drop_priority_max_idx)($20)
	addiu	$19,$19,1
	slt	$2,$19,$2
	bne	$2,$0,$L345
	lui	$3,%hi(drop_priority)

$L307:
	jal	local_bh_enable
	lui	$fp,%hi(drop_priority_max_idx)

	j	$L341
	nop

$L318:
	jal	local_bh_enable
	nop

	jal	rtl_death_action
	move	$4,$16

	j	$L342
	lui	$4,%hi(rtl_newGC_session_status_flags)

$L182:
$L341:
	jal	local_bh_disable
	move	$19,$0

	lw	$2,%lo(drop_priority_max_idx)($fp)
	blez	$2,$L310
	lui	$22,%hi(drop_priority)

$L346:
	sll	$21,$19,2
	addiu	$2,$22,%lo(drop_priority)
	addu	$2,$21,$2
	lbu	$2,0($2)
	sltu	$3,$2,10
	beq	$3,$0,$L214
	sll	$4,$2,3

	lui	$2,%hi(Tcp_State_Hash_Head)
	addiu	$2,$2,%lo(Tcp_State_Hash_Head)
	addu	$2,$4,$2
	lw	$18,4($2)
	lw	$2,0($18)
	beq	$2,$18,$L235
	li	$23,3			# 0x3

	move	$17,$2
	move	$20,$0
$L334:
	beq	$17,$18,$L235
	addiu	$16,$17,-204

	li	$2,-1			# 0xffffffff
	lb	$3,212($16)
	move	$4,$16
	bne	$3,$2,$L221
	addiu	$20,$20,1

	jal	__conntrack_drop_check
	nop

	sb	$2,212($16)
$L221:
	lb	$2,212($16)
	bne	$2,$0,$L220
	move	$4,$16

	jal	rtl_get_ct_timer_expires
	nop

	lui	$3,%hi(jiffies)
	lw	$5,%lo(jiffies)($3)
	addiu	$3,$22,%lo(drop_priority)
	addu	$3,$21,$3
	subu	$2,$2,$5
	lhu	$3,2($3)
	srl	$2,$2,7
	sltu	$3,$3,$2
	bne	$3,$0,$L223
	move	$4,$16

	jal	rtl_del_ct_timer
	nop

	move	$4,$16
	move	$5,$19
	move	$6,$0
	beq	$2,$0,$L223
	li	$7,1			# 0x1

	jal	__drop_one_conntrack_process_hooks1
	sw	$23,16($sp)

	move	$4,$16
	move	$3,$2
	li	$2,1			# 0x1
	move	$5,$19
	move	$6,$0
	beq	$3,$2,$L319
	li	$7,1			# 0x1

	li	$2,2			# 0x2
	beq	$3,$2,$L246
	li	$3,1			# 0x1

	jal	__drop_one_conntrack_process_hooks2
	sw	$23,16($sp)

$L223:
	slt	$2,$20,129
	beq	$2,$0,$L246
	move	$3,$0

$L220:
	lw	$17,0($17)
	j	$L334
	nop

$L319:
	jal	local_bh_enable
	nop

	jal	rtl_death_action
	move	$4,$16

	j	$L246
	li	$3,1			# 0x1

$L214:
	lui	$2,%hi(Udp_State_Hash_Head)
	addiu	$2,$2,%lo(Udp_State_Hash_Head)
	addu	$2,$4,$2
	lw	$18,-84($2)
	lw	$2,0($18)
	beq	$2,$18,$L235
	li	$23,12			# 0xc

	move	$17,$2
	move	$20,$0
$L336:
	beq	$17,$18,$L235
	addiu	$16,$17,-204

	li	$2,-1			# 0xffffffff
	lb	$3,212($16)
	move	$4,$16
	bne	$3,$2,$L240
	addiu	$20,$20,1

	jal	__conntrack_drop_check
	nop

	sb	$2,212($16)
$L240:
	lb	$2,212($16)
	bne	$2,$0,$L239
	move	$4,$16

	jal	rtl_get_ct_timer_expires
	nop

	lui	$3,%hi(jiffies)
	lw	$5,%lo(jiffies)($3)
	addiu	$3,$22,%lo(drop_priority)
	addu	$3,$21,$3
	subu	$2,$2,$5
	lhu	$3,2($3)
	srl	$2,$2,7
	sltu	$3,$3,$2
	bne	$3,$0,$L242
	move	$4,$16

	jal	rtl_del_ct_timer
	nop

	move	$4,$16
	move	$5,$19
	move	$6,$0
	beq	$2,$0,$L242
	li	$7,1			# 0x1

	jal	__drop_one_conntrack_process_hooks1
	sw	$23,16($sp)

	move	$4,$16
	move	$3,$2
	li	$2,1			# 0x1
	move	$5,$19
	move	$6,$0
	beq	$3,$2,$L319
	li	$7,1			# 0x1

	li	$2,2			# 0x2
	beq	$3,$2,$L246
	li	$3,1			# 0x1

	jal	__drop_one_conntrack_process_hooks2
	sw	$23,16($sp)

$L242:
	slt	$2,$20,129
	beq	$2,$0,$L246
	move	$3,$0

$L239:
	lw	$17,0($17)
	j	$L336
	nop

$L235:
	move	$3,$0
$L246:
	li	$2,1			# 0x1
	beq	$3,$2,$L207
	addiu	$19,$19,1

	lw	$2,%lo(drop_priority_max_idx)($fp)
	slt	$2,$19,$2
	bne	$2,$0,$L346
	lui	$22,%hi(drop_priority)

$L310:
	jal	local_bh_enable
	lui	$fp,%hi(drop_priority_max_idx)

	jal	local_bh_disable
	move	$19,$0

	lw	$2,%lo(drop_priority_max_idx)($fp)
	blez	$2,$L314
	lui	$22,%hi(drop_priority)

$L347:
	sll	$21,$19,2
	addiu	$2,$22,%lo(drop_priority)
	addu	$2,$21,$2
	lbu	$2,0($2)
	sltu	$3,$2,10
	beq	$3,$0,$L260
	sll	$4,$2,3

	lui	$2,%hi(Tcp_State_Hash_Head)
	addiu	$2,$2,%lo(Tcp_State_Hash_Head)
	addu	$2,$4,$2
	lw	$18,4($2)
	lw	$2,0($18)
	beq	$2,$18,$L281
	li	$23,3			# 0x3

	move	$17,$2
	move	$20,$0
$L337:
	beq	$17,$18,$L281
	addiu	$16,$17,-204

	li	$2,-1			# 0xffffffff
	lb	$3,212($16)
	move	$4,$16
	bne	$3,$2,$L267
	addiu	$20,$20,1

	jal	__conntrack_drop_check
	nop

	sb	$2,212($16)
$L267:
	lb	$2,212($16)
	bne	$2,$0,$L266
	move	$4,$16

	jal	rtl_get_ct_timer_expires
	nop

	lui	$3,%hi(jiffies)
	lw	$5,%lo(jiffies)($3)
	addiu	$3,$22,%lo(drop_priority)
	addu	$3,$21,$3
	subu	$2,$2,$5
	lhu	$3,2($3)
	srl	$2,$2,9
	sltu	$3,$3,$2
	bne	$3,$0,$L269
	move	$4,$16

	jal	rtl_del_ct_timer
	nop

	move	$4,$16
	move	$5,$19
	li	$6,2			# 0x2
	beq	$2,$0,$L269
	move	$7,$0

	jal	__drop_one_conntrack_process_hooks1
	sw	$23,16($sp)

	move	$4,$16
	move	$3,$2
	li	$2,1			# 0x1
	move	$5,$19
	li	$6,2			# 0x2
	beq	$3,$2,$L324
	move	$7,$0

	li	$2,2			# 0x2
	beq	$3,$2,$L292
	li	$3,1			# 0x1

	jal	__drop_one_conntrack_process_hooks2
	sw	$23,16($sp)

$L269:
	slt	$2,$20,129
	beq	$2,$0,$L292
	move	$3,$0

$L266:
	lw	$17,0($17)
	j	$L337
	nop

$L324:
	jal	local_bh_enable
	nop

	jal	rtl_death_action
	move	$4,$16

	j	$L292
	li	$3,1			# 0x1

$L260:
	lui	$2,%hi(Udp_State_Hash_Head)
	addiu	$2,$2,%lo(Udp_State_Hash_Head)
	addu	$2,$4,$2
	lw	$18,-84($2)
	lw	$2,0($18)
	beq	$2,$18,$L281
	li	$23,12			# 0xc

	move	$17,$2
	move	$20,$0
$L339:
	beq	$17,$18,$L281
	addiu	$16,$17,-204

	li	$2,-1			# 0xffffffff
	lb	$3,212($16)
	move	$4,$16
	bne	$3,$2,$L286
	addiu	$20,$20,1

	jal	__conntrack_drop_check
	nop

	sb	$2,212($16)
$L286:
	lb	$2,212($16)
	bne	$2,$0,$L285
	move	$4,$16

	jal	rtl_get_ct_timer_expires
	nop

	lui	$3,%hi(jiffies)
	lw	$5,%lo(jiffies)($3)
	addiu	$3,$22,%lo(drop_priority)
	addu	$3,$21,$3
	subu	$2,$2,$5
	lhu	$3,2($3)
	srl	$2,$2,9
	sltu	$3,$3,$2
	bne	$3,$0,$L288
	move	$4,$16

	jal	rtl_del_ct_timer
	nop

	move	$4,$16
	move	$5,$19
	li	$6,2			# 0x2
	beq	$2,$0,$L288
	move	$7,$0

	jal	__drop_one_conntrack_process_hooks1
	sw	$23,16($sp)

	move	$4,$16
	move	$3,$2
	li	$2,1			# 0x1
	move	$5,$19
	li	$6,2			# 0x2
	beq	$3,$2,$L324
	move	$7,$0

	li	$2,2			# 0x2
	beq	$3,$2,$L292
	li	$3,1			# 0x1

	jal	__drop_one_conntrack_process_hooks2
	sw	$23,16($sp)

$L288:
	slt	$2,$20,129
	beq	$2,$0,$L292
	move	$3,$0

$L285:
	lw	$17,0($17)
	j	$L339
	nop

$L281:
	move	$3,$0
$L292:
	li	$2,1			# 0x1
	beq	$3,$2,$L207
	addiu	$19,$19,1

	lw	$2,%lo(drop_priority_max_idx)($fp)
	slt	$2,$19,$2
	bne	$2,$0,$L347
	lui	$22,%hi(drop_priority)

	j	$L314
	nop

$L207:
	lui	$4,%hi(rtl_newGC_session_status_flags)
$L342:
	lw	$2,%lo(rtl_newGC_session_status_flags)($4)
	bne	$2,$0,$L300
	nop

	lui	$2,%hi(jiffies)
	lw	$3,%lo(jiffies)($2)
	li	$2,3			# 0x3
	sw	$2,%lo(rtl_newGC_session_status_flags)($4)
	lui	$2,%hi(rtl_newGC_session_status_time)
	addiu	$3,$3,1
	sw	$3,%lo(rtl_newGC_session_status_time)($2)
$L300:
	j	$L157
	li	$2,1			# 0x1

$L314:
	jal	local_bh_enable
	nop

	lui	$4,%hi(rtl_newGC_session_status_flags)
	lw	$2,%lo(rtl_newGC_session_status_flags)($4)
	li	$3,1			# 0x1
	beq	$2,$3,$L302
	lui	$2,%hi(jiffies)

	lw	$2,%lo(jiffies)($2)
	sw	$3,%lo(rtl_newGC_session_status_flags)($4)
	lui	$3,%hi(rtl_newGC_session_status_time)
	addiu	$2,$2,200
	sw	$2,%lo(rtl_newGC_session_status_time)($3)
$L302:
	move	$2,$0
$L157:
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
	li	$5,10			# 0xa
	sw	$31,24($sp)
	addiu	$2,$2,4
	sw	$17,20($sp)
	sw	$16,16($sp)
$L354:
	lw	$3,0($2)
	addiu	$5,$5,-1
	beq	$3,$0,$L351
	addiu	$2,$2,8

	sw	$3,4($3)
	sw	$3,0($3)
$L351:
	bgez	$5,$L354
	nop

	lui	$2,%hi(Udp_State_Hash_Head)
	li	$5,1			# 0x1
	addiu	$2,$2,%lo(Udp_State_Hash_Head)
	addiu	$2,$2,4
$L360:
	lw	$3,0($2)
	addiu	$5,$5,-1
	beq	$3,$0,$L357
	addiu	$2,$2,8

	sw	$3,4($3)
	sw	$3,0($3)
$L357:
	bgez	$5,$L360
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
$L383:
	addiu	$5,$5,1
	addu	$4,$2,$6
	slt	$3,$5,3
	addu	$2,$2,$7
	sw	$0,0($2)
	sw	$0,0($4)
	bne	$3,$0,$L383
	sll	$2,$5,2

	lui	$2,%hi(nf_conntrack_max)
	li	$3,10			# 0xa
	lw	$6,%lo(nf_conntrack_max)($2)
	lui	$2,%hi(_prot_limit)
	addiu	$9,$2,%lo(_prot_limit)
	lui	$2,%hi(drop_priority_max_idx)
	sw	$3,%lo(drop_priority_max_idx)($2)
	li	$2,2			# 0x2
	sw	$2,%lo(prot_limit)($16)
	li	$2,1374355456			# 0x51eb0000
	addiu	$4,$16,%lo(prot_limit)
	ori	$7,$2,0x851f
	li	$2,90			# 0x5a
	lui	$17,%hi(rtl_newGC_session_status_flags)
	sw	$2,4($4)
	li	$2,60			# 0x3c
	move	$8,$4
	move	$5,$0
	sw	$2,8($4)
	sw	$0,%lo(rtl_newGC_session_status_flags)($17)
	sll	$3,$5,2
$L384:
	addiu	$5,$5,1
	addu	$2,$3,$8
	slt	$4,$5,3
	lw	$2,0($2)
	addu	$3,$3,$9
	mult	$6,$2
	mflo	$2
	multu	$2,$7
	mfhi	$2
	srl	$2,$2,5
	sw	$2,0($3)
	bne	$4,$0,$L384
	sll	$3,$5,2

	li	$3,-859045888			# 0xcccc0000
	sll	$2,$6,2
	ori	$3,$3,0xcccd
	lui	$4,%hi(rtl_nf_conntrack_threshold)
	multu	$2,$3
	mfhi	$2
	srl	$2,$2,2
	subu	$3,$6,$2
	sltu	$3,$3,65
	bne	$3,$0,$L370
	sw	$2,%lo(rtl_nf_conntrack_threshold)($4)

	addiu	$2,$6,-64
	sw	$2,%lo(rtl_nf_conntrack_threshold)($4)
$L370:
	lui	$2,%hi(nf_conntrack_max)
	lw	$2,%lo(nf_conntrack_max)($2)
	beq	$2,$0,$L371
	nop

	jal	rtl_gc_threshold_check
	move	$4,$0

	bne	$2,$0,$L371
	li	$3,3			# 0x3

	lw	$2,%lo(rtl_newGC_session_status_flags)($17)
	beq	$2,$3,$L374
	lui	$2,%hi(jiffies)

	lw	$2,%lo(jiffies)($2)
	sw	$3,%lo(rtl_newGC_session_status_flags)($17)
	lui	$3,%hi(rtl_newGC_session_status_time)
	addiu	$2,$2,1
	j	$L374
	sw	$2,%lo(rtl_newGC_session_status_time)($3)

$L371:
	sw	$0,%lo(rtl_newGC_session_status_flags)($17)
$L374:
	jal	rtl_nf_conn_GC_init_hooks
	nop

	move	$2,$0
	lw	$31,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
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
