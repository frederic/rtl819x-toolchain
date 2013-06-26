	.file	1 "rtl_nf_connGC.c"
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
	.section	.text.isReservedConntrack,"ax",@progbits
	.align	2
	.globl	isReservedConntrack
	.set	nomips16
	.ent	isReservedConntrack
	.type	isReservedConntrack, @function
isReservedConntrack:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	lbu	$2,38($4)
	li	$3,1			# 0x1
	beq	$2,$3,$L2
	li	$3,-268435456			# 0xf0000000

	lw	$6,20($4)
	li	$2,-536870912			# 0xe0000000
	and	$7,$6,$3
	lw	$8,0($4)
	beq	$7,$2,$L2
	li	$7,-1			# 0xffffffff

	beq	$6,$7,$L2
	nop

	beq	$8,$0,$L2
	nop

	j	$L28
	nop

$L20:
	beq	$6,$7,$L30
	nop

	beq	$8,$0,$L30
	nop

	j	$L29
	li	$2,80			# 0x50

$L21:
	bne	$3,$6,$L17
	nop

	j	$31
	li	$2,1			# 0x1

$L22:
	bne	$3,$4,$L18
	li	$3,80			# 0x50

	j	$31
	li	$2,1			# 0x1

$L23:
	bne	$2,$4,$L19
	li	$2,1			# 0x1

	j	$L30
	nop

$L24:
	xori	$2,$2,0x1f90
	j	$31
	sltu	$2,$2,1

$L2:
	li	$2,1			# 0x1
$L30:
	j	$31
	nop

$L28:
	lw	$6,20($5)
	lw	$8,0($5)
	and	$3,$6,$3
	bne	$3,$2,$L20
	li	$2,1			# 0x1

	j	$L30
	nop

$L29:
	lhu	$3,16($4)
	bne	$3,$2,$L21
	li	$6,8080			# 0x1f90

	j	$31
	li	$2,1			# 0x1

$L17:
	lhu	$3,36($4)
	bne	$3,$2,$L22
	li	$4,8080			# 0x1f90

	j	$31
	li	$2,1			# 0x1

$L18:
	lhu	$2,16($5)
	bne	$2,$3,$L23
	nop

	j	$31
	li	$2,1			# 0x1

$L19:
	lhu	$2,36($5)
	bne	$2,$3,$L24
	nop

	j	$31
	li	$2,1			# 0x1

	.set	macro
	.set	reorder
	.end	isReservedConntrack
	.size	isReservedConntrack, .-isReservedConntrack
	.section	.text.rtl_nf_conn_GC_init,"ax",@progbits
	.align	2
	.globl	rtl_nf_conn_GC_init
	.set	nomips16
	.ent	rtl_nf_conn_GC_init
	.type	rtl_nf_conn_GC_init, @function
rtl_nf_conn_GC_init:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	lui	$3,%hi(Tcp_State_Hash_Head+4)
	lui	$4,%hi(Tcp_State_Hash_Head+92)
	addiu	$3,$3,%lo(Tcp_State_Hash_Head+4)
	j	$L32
	addiu	$4,$4,%lo(Tcp_State_Hash_Head+92)

$L34:
	lw	$2,0($3)
	beq	$2,$0,$L32
	addiu	$3,$3,8

	sw	$2,4($2)
	sw	$2,0($2)
$L32:
	bne	$3,$4,$L34
	nop

	lui	$3,%hi(Udp_State_Hash_Head+4)
	lui	$4,%hi(Udp_State_Hash_Head+20)
	addiu	$3,$3,%lo(Udp_State_Hash_Head+4)
	j	$L35
	addiu	$4,$4,%lo(Udp_State_Hash_Head+20)

$L37:
	lw	$2,0($3)
	beq	$2,$0,$L35
	addiu	$3,$3,8

	sw	$2,4($2)
	sw	$2,0($2)
$L35:
	bne	$3,$4,$L37
	li	$6,12			# 0xc

	lui	$4,%hi(prot_limit)
	addiu	$4,$4,%lo(prot_limit)
	jal	memset
	move	$5,$0

	lui	$6,%hi(prot_counters)
	lui	$4,%hi(_prot_limit)
	addiu	$6,$6,%lo(prot_counters)
	addiu	$4,$4,%lo(_prot_limit)
	move	$2,$0
	li	$3,12			# 0xc
	addu	$8,$6,$2
$L46:
	addu	$7,$2,$4
	move	$5,$4
	addiu	$2,$2,4
	sw	$0,0($8)
	sw	$0,0($7)
	bne	$2,$3,$L46
	addu	$8,$6,$2

	li	$6,10			# 0xa
	lui	$3,%hi(drop_priority_max_idx)
	sw	$6,%lo(drop_priority_max_idx)($3)
	lui	$6,%hi(rtl_newGC_session_status_flags)
	lui	$4,%hi(prot_limit)
	li	$9,60			# 0x3c
	sw	$0,%lo(rtl_newGC_session_status_flags)($6)
	li	$6,2			# 0x2
	addiu	$2,$4,%lo(prot_limit)
	move	$3,$0
	sw	$6,%lo(prot_limit)($4)
	li	$6,90			# 0x5a
	move	$8,$2
	sw	$6,4($2)
	lui	$7,%hi(nf_conntrack_max)
	li	$4,100			# 0x64
	li	$6,12			# 0xc
	sw	$9,8($2)
$L39:
	addu	$2,$8,$3
	addu	$9,$3,$5
	lw	$10,0($2)
	addiu	$3,$3,4
	lw	$2,%lo(nf_conntrack_max)($7)
	mult	$10,$2
	mflo	$10
	divu	$0,$10,$4
	mflo	$10
	sw	$10,0($9)
	bne	$3,$6,$L39
	nop

	sll	$5,$2,2
	li	$4,5			# 0x5
	lui	$3,%hi(rtl_nf_conntrack_threshold)
	divu	$0,$5,$4
	mflo	$5
	subu	$4,$2,$5
	sltu	$4,$4,65
	bne	$4,$0,$L40
	sw	$5,%lo(rtl_nf_conntrack_threshold)($3)

	addiu	$4,$2,-64
	sw	$4,%lo(rtl_nf_conntrack_threshold)($3)
$L40:
	beq	$2,$0,$L47
	lui	$2,%hi(rtl_newGC_session_status_flags)

	jal	rtl_gc_threshold_check
	move	$4,$0

	bne	$2,$0,$L41
	lui	$2,%hi(rtl_newGC_session_status_flags)

	li	$3,3			# 0x3
	lw	$4,%lo(rtl_newGC_session_status_flags)($2)
	beq	$4,$3,$L42
	lui	$4,%hi(jiffies)

	lw	$4,%lo(jiffies)($4)
	sw	$3,%lo(rtl_newGC_session_status_flags)($2)
	lui	$2,%hi(rtl_newGC_session_status_time)
	addiu	$3,$4,1
	j	$L42
	sw	$3,%lo(rtl_newGC_session_status_time)($2)

$L41:
$L47:
	sw	$0,%lo(rtl_newGC_session_status_flags)($2)
$L42:
	jal	rtl_nf_conn_GC_init_hooks
	nop

	move	$2,$0
	lw	$31,20($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	rtl_nf_conn_GC_init
	.size	rtl_nf_conn_GC_init, .-rtl_nf_conn_GC_init
	.section	.text.__conntrack_drop_check,"ax",@progbits
	.align	2
	.globl	__conntrack_drop_check
	.set	nomips16
	.ent	__conntrack_drop_check
	.type	__conntrack_drop_check, @function
__conntrack_drop_check:
	.frame	$sp,56,$31		# vars= 0, regs= 9/0, args= 16, gp= 0
	.mask	0x80ff0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-56
	sw	$31,52($sp)
	sw	$23,48($sp)
	sw	$22,44($sp)
	sw	$21,40($sp)
	sw	$20,36($sp)
	sw	$19,32($sp)
	sw	$18,28($sp)
	sw	$17,24($sp)
	sw	$16,20($sp)
	move	$5,$0
	jal	rtl_new_gc_get_ct_protonum
	move	$16,$4

	move	$4,$16
	move	$5,$0
	jal	rtl_new_gc_get_ct_port_by_dir
	move	$6,$0

	move	$4,$16
	move	$5,$0
	li	$6,1			# 0x1
	jal	rtl_new_gc_get_ct_port_by_dir
	move	$20,$2

	move	$4,$16
	li	$5,1			# 0x1
	move	$6,$0
	jal	rtl_new_gc_get_ct_port_by_dir
	move	$19,$2

	move	$4,$16
	li	$5,1			# 0x1
	li	$6,1			# 0x1
	jal	rtl_new_gc_get_ct_port_by_dir
	move	$18,$2

	move	$4,$16
	move	$5,$0
	move	$6,$0
	jal	rtl_new_gc_get_ct_ip_by_dir
	move	$17,$2

	move	$4,$16
	move	$5,$0
	li	$6,1			# 0x1
	jal	rtl_new_gc_get_ct_ip_by_dir
	move	$23,$2

	move	$4,$16
	li	$5,1			# 0x1
	move	$6,$0
	jal	rtl_new_gc_get_ct_ip_by_dir
	move	$21,$2

	move	$4,$16
	li	$5,1			# 0x1
	li	$6,1			# 0x1
	jal	rtl_new_gc_get_ct_ip_by_dir
	move	$22,$2

	li	$4,-268435456			# 0xf0000000
	and	$5,$21,$4
	li	$3,-536870912			# 0xe0000000
	beq	$5,$3,$L49
	and	$4,$2,$4

	beq	$4,$3,$L49
	nop

	bne	$23,$2,$L53
	sltu	$2,$20,1024

	beq	$21,$22,$L51
	li	$2,-1			# 0xffffffff

	sltu	$2,$20,1024
$L53:
	bne	$2,$0,$L51
	li	$2,-1			# 0xffffffff

	sltu	$2,$19,1024
	bne	$2,$0,$L51
	li	$2,-1			# 0xffffffff

	sltu	$2,$18,1024
	bne	$2,$0,$L51
	li	$2,-1			# 0xffffffff

	sltu	$2,$17,1024
	bne	$2,$0,$L49
	li	$3,8080			# 0x1f90

	beq	$20,$3,$L49
	nop

	beq	$19,$3,$L49
	nop

	beq	$18,$3,$L51
	li	$2,-1			# 0xffffffff

	bne	$17,$3,$L51
	move	$2,$0

$L49:
	li	$2,-1			# 0xffffffff
$L51:
	lw	$31,52($sp)
	lw	$23,48($sp)
	lw	$22,44($sp)
	lw	$21,40($sp)
	lw	$20,36($sp)
	lw	$19,32($sp)
	lw	$18,28($sp)
	lw	$17,24($sp)
	lw	$16,20($sp)
	j	$31
	addiu	$sp,$sp,56

	.set	macro
	.set	reorder
	.end	__conntrack_drop_check
	.size	__conntrack_drop_check, .-__conntrack_drop_check
	.section	.text.__nf_ct_refresh_acct_proto,"ax",@progbits
	.align	2
	.globl	__nf_ct_refresh_acct_proto
	.set	nomips16
	.ent	__nf_ct_refresh_acct_proto
	.type	__nf_ct_refresh_acct_proto, @function
__nf_ct_refresh_acct_proto:
	.frame	$sp,56,$31		# vars= 8, regs= 6/0, args= 24, gp= 0
	.mask	0x801f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-56
	sw	$31,52($sp)
	sw	$20,48($sp)
	sw	$19,44($sp)
	sw	$18,40($sp)
	sw	$17,36($sp)
	sw	$16,32($sp)
	move	$16,$4
	move	$18,$5
	sw	$0,24($sp)
	move	$17,$6
	lbu	$19,79($sp)
	jal	local_bh_disable
	move	$20,$7

	li	$5,10			# 0xa
	jal	rtl_test_bit
	move	$4,$16

	beq	$2,$0,$L64
	move	$4,$16

	jal	rtl_test_bit
	li	$5,3			# 0x3

	li	$3,-1			# 0xffffffff
	bne	$2,$3,$L57
	move	$5,$20

	jal	rtl_new_gc_set_ct_timeout_expires
	move	$4,$16

	li	$2,8			# 0x8
	j	$L58
	sw	$2,24($sp)

$L57:
	jal	rtl_del_ct_timer
	move	$4,$16

	beq	$2,$0,$L58
	lui	$2,%hi(jiffies)

	move	$4,$16
	lw	$5,%lo(jiffies)($2)
	jal	rtl_new_gc_set_ct_timeout_expires
	addu	$5,$20,$5

	jal	rtl_add_ct_timer
	move	$4,$16

	li	$2,8			# 0x8
	sw	$2,24($sp)
	li	$2,6			# 0x6
	beq	$19,$2,$L59
	li	$2,17			# 0x11

	bne	$19,$2,$L58
	nop

	j	$L66
	nop

$L59:
	lw	$6,84($sp)
	li	$5,1			# 0x1
	j	$L63
	move	$4,$16

$L66:
	jal	rtl_new_gc_get_ct_udp_status
	move	$4,$16

	andi	$2,$2,0x2
	beq	$2,$0,$L61
	move	$4,$16

	li	$5,2			# 0x2
	j	$L63
	li	$6,1			# 0x1

$L61:
	li	$5,2			# 0x2
	move	$6,$0
$L63:
	jal	rtl_list_move_tail
	nop

$L58:
	lw	$7,72($sp)
	addiu	$2,$sp,24
	move	$4,$16
	move	$5,$18
	sw	$2,16($sp)
	jal	__nf_ct_refresh_acct_proto_hooks
	move	$6,$17

$L64:
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
	.size	__nf_ct_refresh_acct_proto, .-__nf_ct_refresh_acct_proto
	.section	.text.rtl_connGC_addList,"ax",@progbits
	.align	2
	.globl	rtl_connGC_addList
	.set	nomips16
	.ent	rtl_connGC_addList
	.type	rtl_connGC_addList, @function
rtl_connGC_addList:
	.frame	$sp,32,$31		# vars= 0, regs= 3/0, args= 16, gp= 0
	.mask	0x80030000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$17,24($sp)
	sw	$16,20($sp)
	move	$16,$5
	.set	noreorder
	.set	nomacro
	jal	rtl_new_gc_ip_hdr
	move	$17,$4
	.set	macro
	.set	reorder

	beq	$2,$0,$L68
	.set	noreorder
	.set	nomacro
	jal	rtl_new_gc_get_skb_protocol
	move	$4,$17
	.set	macro
	.set	reorder

	li	$3,6			# 0x6
	.set	noreorder
	.set	nomacro
	beq	$2,$3,$L70
	li	$3,17			# 0x11
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$3,$L71
	li	$3,1			# 0x1
	.set	macro
	.set	reorder

	bne	$2,$3,$L68
	j	$L79
$L70:
	.set	noreorder
	.set	nomacro
	jal	rtl_new_gc_get_ct_tcp_state
	move	$4,$16
	.set	macro
	.set	reorder

	lui	$3,%hi(Tcp_State_Hash_Head)
	sll	$2,$2,3
	addiu	$3,$3,%lo(Tcp_State_Hash_Head)
	addu	$2,$3,$2
	lw	$2,4($2)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L72
	move	$4,$16
	.set	macro
	.set	reorder

	li	$5,1			# 0x1
	.set	noreorder
	.set	nomacro
	jal	rtl_list_add_tail
	move	$6,$0
	.set	macro
	.set	reorder

$L72:
#APP
 # 56 "/backup_new/qian_yu/trunk_release_buildcode/96d/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
#NO_APP
	lui	$2,%hi(prot_counters)
	addiu	$2,$2,%lo(prot_counters)
#APP
 # 58 "/backup_new/qian_yu/trunk_release_buildcode/96d/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	1:	ll	$3, 4($2)		# atomic_add    
       nop                                             
	addu	$3, 1					
	sc	$3, 4($2)                      
	beqz	$3, 2f					
	.subsection 2					
2:	b	1b                          
	.previous                       

 # 0 "" 2
#NO_APP
	j	$L77
$L71:
	.set	noreorder
	.set	nomacro
	jal	rtl_new_gc_get_ct_udp_status
	move	$4,$16
	.set	macro
	.set	reorder

	andi	$2,$2,0x2
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L73
	move	$4,$16
	.set	macro
	.set	reorder

	li	$5,2			# 0x2
	.set	noreorder
	.set	nomacro
	j	$L76
	li	$6,1			# 0x1
	.set	macro
	.set	reorder

$L73:
	li	$5,2			# 0x2
	move	$6,$0
$L76:
	jal	rtl_list_add_tail
#APP
 # 56 "/backup_new/qian_yu/trunk_release_buildcode/96d/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
#NO_APP
	lui	$2,%hi(prot_counters)
	addiu	$2,$2,%lo(prot_counters)
#APP
 # 58 "/backup_new/qian_yu/trunk_release_buildcode/96d/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	1:	ll	$3, 8($2)		# atomic_add    
       nop                                             
	addu	$3, 1					
	sc	$3, 8($2)                      
	beqz	$3, 2f					
	.subsection 2					
2:	b	1b                          
	.previous                       

 # 0 "" 2
#NO_APP
	j	$L77
$L79:
#APP
 # 56 "/backup_new/qian_yu/trunk_release_buildcode/96d/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
#NO_APP
	lui	$2,%hi(prot_counters)
#APP
 # 58 "/backup_new/qian_yu/trunk_release_buildcode/96d/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	1:	ll	$3, %lo(prot_counters)($2)		# atomic_add    
       nop                                             
	addu	$3, 1					
	sc	$3, %lo(prot_counters)($2)                      
	beqz	$3, 2f					
	.subsection 2					
2:	b	1b                          
	.previous                       

 # 0 "" 2
#NO_APP
$L77:
#APP
 # 72 "/backup_new/qian_yu/trunk_release_buildcode/96d/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
#NO_APP
$L68:
	lw	$31,28($sp)
	move	$2,$0
	lw	$17,24($sp)
	lw	$16,20($sp)
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,32
	.set	macro
	.set	reorder

	.end	rtl_connGC_addList
	.size	rtl_connGC_addList, .-rtl_connGC_addList
	.section	.text.clean_from_lists,"ax",@progbits
	.align	2
	.globl	clean_from_lists
	.set	nomips16
	.ent	clean_from_lists
	.type	clean_from_lists, @function
clean_from_lists:
	.frame	$sp,32,$31		# vars= 0, regs= 3/0, args= 16, gp= 0
	.mask	0x80030000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$17,24($sp)
	sw	$16,20($sp)
	move	$16,$4
	move	$17,$5
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

	li	$3,6			# 0x6
	.set	noreorder
	.set	nomacro
	beq	$2,$3,$L83
	li	$3,17			# 0x11
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$3,$L84
	li	$3,1			# 0x1
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$3,$L92
	move	$4,$16
	.set	macro
	.set	reorder

	j	$L91
$L83:
#APP
 # 94 "/backup_new/qian_yu/trunk_release_buildcode/96d/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
#NO_APP
	lui	$2,%hi(prot_counters)
	addiu	$2,$2,%lo(prot_counters)
#APP
 # 96 "/backup_new/qian_yu/trunk_release_buildcode/96d/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	1:	ll	    $3, 4($2)		# atomic_sub		
       nop                                             
	subu	$3, 1					
	sc	    $3, 4($2)					
	beqz	$3, 2f					
	.subsection 2					
2:	b	1b					
	.previous					

 # 0 "" 2
#NO_APP
	j	$L89
$L84:
#APP
 # 94 "/backup_new/qian_yu/trunk_release_buildcode/96d/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
#NO_APP
	lui	$2,%hi(prot_counters)
	addiu	$2,$2,%lo(prot_counters)
#APP
 # 96 "/backup_new/qian_yu/trunk_release_buildcode/96d/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	1:	ll	    $3, 8($2)		# atomic_sub		
       nop                                             
	subu	$3, 1					
	sc	    $3, 8($2)					
	beqz	$3, 2f					
	.subsection 2					
2:	b	1b					
	.previous					

 # 0 "" 2
#NO_APP
	j	$L89
$L91:
#APP
 # 94 "/backup_new/qian_yu/trunk_release_buildcode/96d/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
#NO_APP
	lui	$2,%hi(prot_counters)
#APP
 # 96 "/backup_new/qian_yu/trunk_release_buildcode/96d/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	1:	ll	    $3, %lo(prot_counters)($2)		# atomic_sub		
       nop                                             
	subu	$3, 1					
	sc	    $3, %lo(prot_counters)($2)					
	beqz	$3, 2f					
	.subsection 2					
2:	b	1b					
	.previous					

 # 0 "" 2
#NO_APP
$L89:
#APP
 # 110 "/backup_new/qian_yu/trunk_release_buildcode/96d/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
#NO_APP
	move	$4,$16
$L92:
	.set	noreorder
	.set	nomacro
	jal	rtl_new_gc_get_ct_protonum
	move	$5,$0
	.set	macro
	.set	reorder

	li	$3,6			# 0x6
	.set	noreorder
	.set	nomacro
	beq	$2,$3,$L85
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
	bne	$2,$3,$L86
$L85:
	move	$5,$17
	.set	noreorder
	.set	nomacro
	jal	clean_from_lists_hooks
	move	$4,$16
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
	beq	$2,$0,$L93
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
	bne	$2,$0,$L87
	lui	$2,%hi(rtl_newGC_session_status_flags)
	.set	macro
	.set	reorder

	li	$3,3			# 0x3
	lw	$4,%lo(rtl_newGC_session_status_flags)($2)
	.set	noreorder
	.set	nomacro
	beq	$4,$3,$L86
	lui	$4,%hi(jiffies)
	.set	macro
	.set	reorder

	lw	$4,%lo(jiffies)($4)
	sw	$3,%lo(rtl_newGC_session_status_flags)($2)
	lui	$2,%hi(rtl_newGC_session_status_time)
	addiu	$3,$4,1
	.set	noreorder
	.set	nomacro
	j	$L86
	sw	$3,%lo(rtl_newGC_session_status_time)($2)
	.set	macro
	.set	reorder

$L87:
$L93:
	sw	$0,%lo(rtl_newGC_session_status_flags)($2)
$L86:
	jal	local_bh_enable
	move	$4,$16
	lw	$31,28($sp)
	lw	$17,24($sp)
	lw	$16,20($sp)
	.set	noreorder
	.set	nomacro
	j	nf_ct_remove_expectations
	addiu	$sp,$sp,32
	.set	macro
	.set	reorder

	.end	clean_from_lists
	.size	clean_from_lists, .-clean_from_lists
	.section	.text.rtl_death_action,"ax",@progbits
	.align	2
	.globl	rtl_death_action
	.set	nomips16
	.ent	rtl_death_action
	.type	rtl_death_action, @function
rtl_death_action:
	.frame	$sp,32,$31		# vars= 0, regs= 3/0, args= 16, gp= 0
	.mask	0x80030000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$17,24($sp)
	sw	$16,20($sp)
	lw	$2,200($4)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L97
	move	$16,$4
	.set	macro
	.set	reorder

	lbu	$3,8($2)
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L97
	addu	$2,$2,$3
	.set	macro
	.set	reorder

	beq	$2,$0,$L97
	lw	$2,0($2)
	beq	$2,$0,$L97
	lw	$2,64($2)
	beq	$2,$0,$L97
	jalr	$2
$L97:
	.set	noreorder
	.set	nomacro
	jal	local_bh_disable
	lui	$17,%hi(init_net)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_nf_ct_stat_inc
	addiu	$4,$17,%lo(init_net)
	.set	macro
	.set	reorder

	move	$4,$16
	.set	noreorder
	.set	nomacro
	jal	clean_from_lists
	addiu	$5,$17,%lo(init_net)
	.set	macro
	.set	reorder

	jal	local_bh_enable
	beq	$16,$0,$L99
#APP
 # 167 "/backup_new/qian_yu/trunk_release_buildcode/96d/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
 # 169 "/backup_new/qian_yu/trunk_release_buildcode/96d/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	1:	ll	$3, 0($16)		# atomic_sub_return	
       nop                                             
	subu	$2, $3, 1				
	sc	$2, 0($16)					
	beqz	$2, 2f					
	subu	$2, $3, 1				
	.subsection 2					
2:	b	1b					
	.previous					

 # 0 "" 2
 # 185 "/backup_new/qian_yu/trunk_release_buildcode/96d/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
#NO_APP
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L99
	move	$4,$16
	.set	macro
	.set	reorder

	lw	$31,28($sp)
	lw	$17,24($sp)
	lw	$16,20($sp)
	.set	noreorder
	.set	nomacro
	j	nf_conntrack_destroy
	addiu	$sp,$sp,32
	.set	macro
	.set	reorder

$L99:
	lw	$31,28($sp)
	lw	$17,24($sp)
	lw	$16,20($sp)
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,32
	.set	macro
	.set	reorder

	.end	rtl_death_action
	.size	rtl_death_action, .-rtl_death_action
	.section	.text.drop_one_conntrack,"ax",@progbits
	.align	2
	.globl	drop_one_conntrack
	.set	nomips16
	.ent	drop_one_conntrack
	.type	drop_one_conntrack, @function
drop_one_conntrack:
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
	lui	$2,%hi(_prot_limit)
	addiu	$2,$2,%lo(_prot_limit)
	lw	$3,4($2)
	lui	$2,%hi(prot_counters)
	addiu	$2,$2,%lo(prot_counters)
	lw	$2,4($2)
	slt	$2,$3,$2
	beq	$2,$0,$L170
	lui	$2,%hi(_prot_limit)

	lui	$19,%hi(drop_priority)
	jal	local_bh_disable
	lui	$23,%hi(Tcp_State_Hash_Head)

	addiu	$19,$19,%lo(drop_priority)
	addiu	$23,$23,%lo(Tcp_State_Hash_Head)
	move	$18,$0
	lui	$22,%hi(drop_priority_max_idx)
	j	$L102
	li	$21,-1			# 0xffffffff

$L110:
	lbu	$2,0($19)
	sltu	$3,$2,10
	beq	$3,$0,$L103
	sll	$2,$2,3

	addu	$2,$23,$2
	lw	$20,4($2)
	lw	$17,0($20)
$L160:
	beq	$17,$20,$L103
	addiu	$16,$17,-204

	lb	$2,212($16)
	bne	$2,$21,$L171
	nop

	jal	__conntrack_drop_check
	move	$4,$16

	sb	$2,212($16)
	lb	$2,212($16)
$L171:
	bne	$2,$0,$L106
	nop

	jal	rtl_del_ct_timer
	move	$4,$16

	bne	$2,$0,$L163
	nop

$L106:
	lw	$17,0($17)
	j	$L160
	nop

$L103:
	addiu	$18,$18,1
	addiu	$19,$19,4
$L102:
	lw	$2,%lo(drop_priority_max_idx)($22)
	slt	$2,$18,$2
	bne	$2,$0,$L110
	nop

	jal	local_bh_enable
	nop

	lui	$2,%hi(_prot_limit)
$L170:
	addiu	$2,$2,%lo(_prot_limit)
	lw	$3,8($2)
	lui	$2,%hi(prot_counters)
	addiu	$2,$2,%lo(prot_counters)
	lw	$2,8($2)
	slt	$2,$3,$2
	beq	$2,$0,$L112
	nop

	lui	$19,%hi(drop_priority)
	jal	local_bh_disable
	lui	$23,%hi(Udp_State_Hash_Head)

	addiu	$19,$19,%lo(drop_priority)
	addiu	$23,$23,%lo(Udp_State_Hash_Head)
	move	$18,$0
	lui	$22,%hi(drop_priority_max_idx)
	j	$L113
	li	$21,-1			# 0xffffffff

$L120:
	lbu	$2,0($19)
	sltu	$3,$2,11
	bne	$3,$0,$L114
	addiu	$2,$2,-11

	sll	$2,$2,3
	addu	$2,$23,$2
	lw	$20,4($2)
	lw	$17,0($20)
$L161:
	beq	$17,$20,$L114
	addiu	$16,$17,-204

	lb	$2,212($16)
	bne	$2,$21,$L172
	nop

	jal	__conntrack_drop_check
	move	$4,$16

	sb	$2,212($16)
	lb	$2,212($16)
$L172:
	bne	$2,$0,$L117
	nop

	jal	rtl_del_ct_timer
	move	$4,$16

	bne	$2,$0,$L163
	nop

$L117:
	lw	$17,0($17)
	j	$L161
	nop

$L114:
	addiu	$18,$18,1
	addiu	$19,$19,4
$L113:
	lw	$2,%lo(drop_priority_max_idx)($22)
	slt	$2,$18,$2
	bne	$2,$0,$L120
	nop

	jal	local_bh_enable
	nop

$L112:
	lui	$18,%hi(drop_priority+2)
	move	$17,$0
	addiu	$18,$18,%lo(drop_priority+2)
	jal	local_bh_disable
	li	$fp,-1			# 0xffffffff

	move	$19,$18
	lui	$23,%hi(jiffies)
	li	$22,2			# 0x2
	j	$L121
	li	$21,12			# 0xc

$L137:
	lbu	$2,-2($19)
	sltu	$3,$2,10
	beq	$3,$0,$L122
	lui	$3,%hi(Udp_State_Hash_Head)

	lui	$3,%hi(Tcp_State_Hash_Head)
	sll	$2,$2,3
	addiu	$3,$3,%lo(Tcp_State_Hash_Head)
	sw	$0,28($sp)
	addu	$2,$3,$2
	lw	$2,4($2)
	sw	$2,24($sp)
	lw	$20,0($2)
	bne	$20,$2,$L124
	nop

	j	$L167
	addiu	$17,$17,1

$L130:
	lw	$2,28($sp)
	addiu	$2,$2,1
	sw	$2,28($sp)
	lb	$2,212($16)
	bne	$2,$fp,$L173
	nop

	jal	__conntrack_drop_check
	move	$4,$16

	sb	$2,212($16)
	lb	$2,212($16)
$L173:
	bne	$2,$0,$L126
	nop

	jal	rtl_get_ct_timer_expires
	move	$4,$16

	lw	$4,%lo(jiffies)($23)
	lhu	$3,0($19)
	subu	$4,$2,$4
	srl	$4,$4,7
	sltu	$3,$3,$4
	bne	$3,$0,$L127
	nop

	jal	rtl_del_ct_timer
	move	$4,$16

	beq	$2,$0,$L127
	li	$3,3			# 0x3

	move	$4,$16
	move	$6,$0
	li	$7,1			# 0x1
	move	$5,$17
	jal	__drop_one_conntrack_process_hooks1
	sw	$3,16($sp)

	li	$3,1			# 0x1
	beq	$2,$3,$L163
	nop

	beq	$2,$22,$L129
	li	$2,3			# 0x3

	move	$4,$16
	move	$6,$0
	li	$7,1			# 0x1
	move	$5,$17
	jal	__drop_one_conntrack_process_hooks2
	sw	$2,16($sp)

$L127:
	lw	$3,28($sp)
	slt	$2,$3,129
	beq	$2,$0,$L123
	nop

$L126:
	lw	$20,0($20)
$L124:
	lw	$2,24($sp)
	bne	$20,$2,$L130
	addiu	$16,$20,-204

	j	$L167
	addiu	$17,$17,1

$L122:
	addiu	$2,$2,-11
	sll	$2,$2,3
	addiu	$3,$3,%lo(Udp_State_Hash_Head)
	sw	$0,28($sp)
	addu	$2,$3,$2
	lw	$2,4($2)
	sw	$2,24($sp)
	lw	$20,0($2)
	bne	$20,$2,$L131
	nop

	j	$L167
	addiu	$17,$17,1

$L136:
	lw	$2,28($sp)
	addiu	$16,$20,-204
	addiu	$2,$2,1
	sw	$2,28($sp)
	lb	$2,212($16)
	bne	$2,$fp,$L174
	nop

	jal	__conntrack_drop_check
	move	$4,$16

	sb	$2,212($16)
	lb	$2,212($16)
$L174:
	bne	$2,$0,$L133
	nop

	jal	rtl_get_ct_timer_expires
	move	$4,$16

	lw	$4,%lo(jiffies)($23)
	lhu	$3,0($19)
	subu	$4,$2,$4
	srl	$4,$4,7
	sltu	$3,$3,$4
	bne	$3,$0,$L134
	nop

	jal	rtl_del_ct_timer
	move	$4,$16

	beq	$2,$0,$L134
	move	$4,$16

	move	$6,$0
	li	$7,1			# 0x1
	move	$5,$17
	jal	__drop_one_conntrack_process_hooks1
	sw	$21,16($sp)

	li	$3,1			# 0x1
	beq	$2,$3,$L163
	nop

	beq	$2,$22,$L129
	move	$4,$16

	move	$6,$0
	li	$7,1			# 0x1
	move	$5,$17
	jal	__drop_one_conntrack_process_hooks2
	sw	$21,16($sp)

$L134:
	lw	$3,28($sp)
	slt	$2,$3,129
	beq	$2,$0,$L123
	nop

$L133:
	lw	$20,0($20)
$L131:
	lw	$2,24($sp)
	bne	$20,$2,$L136
	nop

$L123:
	addiu	$17,$17,1
$L167:
	addiu	$19,$19,4
$L121:
	lui	$3,%hi(drop_priority_max_idx)
	lw	$2,%lo(drop_priority_max_idx)($3)
	slt	$2,$17,$2
	bne	$2,$0,$L137
	nop

	jal	local_bh_enable
	move	$17,$0

	jal	local_bh_disable
	li	$fp,-1			# 0xffffffff

	lui	$23,%hi(jiffies)
	li	$22,1			# 0x1
	li	$21,12			# 0xc
	j	$L138
	li	$20,3			# 0x3

$L153:
	lbu	$2,-2($18)
	sltu	$3,$2,10
	beq	$3,$0,$L139
	lui	$3,%hi(Udp_State_Hash_Head)

	lui	$3,%hi(Tcp_State_Hash_Head)
	sll	$2,$2,3
	addiu	$3,$3,%lo(Tcp_State_Hash_Head)
	sw	$0,28($sp)
	addu	$2,$3,$2
	lw	$2,4($2)
	sw	$2,24($sp)
	lw	$19,0($2)
	bne	$19,$2,$L141
	nop

	j	$L168
	addiu	$17,$17,1

$L146:
	lw	$2,28($sp)
	addiu	$16,$19,-204
	addiu	$2,$2,1
	sw	$2,28($sp)
	lb	$2,212($16)
	bne	$2,$fp,$L175
	nop

	jal	__conntrack_drop_check
	move	$4,$16

	sb	$2,212($16)
	lb	$2,212($16)
$L175:
	bne	$2,$0,$L143
	nop

	jal	rtl_get_ct_timer_expires
	move	$4,$16

	lw	$4,%lo(jiffies)($23)
	lhu	$3,0($18)
	subu	$4,$2,$4
	srl	$4,$4,9
	sltu	$3,$3,$4
	bne	$3,$0,$L144
	nop

	jal	rtl_del_ct_timer
	move	$4,$16

	beq	$2,$0,$L144
	move	$4,$16

	li	$6,2			# 0x2
	move	$7,$0
	move	$5,$17
	jal	__drop_one_conntrack_process_hooks1
	sw	$20,16($sp)

	beq	$2,$22,$L163
	li	$3,2			# 0x2

	beq	$2,$3,$L129
	move	$4,$16

	li	$6,2			# 0x2
	move	$7,$0
	move	$5,$17
	jal	__drop_one_conntrack_process_hooks2
	sw	$20,16($sp)

$L144:
	lw	$3,28($sp)
	slt	$2,$3,129
	beq	$2,$0,$L140
	nop

$L143:
	lw	$19,0($19)
$L141:
	lw	$2,24($sp)
	bne	$19,$2,$L146
	nop

	j	$L168
	addiu	$17,$17,1

$L139:
	addiu	$2,$2,-11
	sll	$2,$2,3
	addiu	$3,$3,%lo(Udp_State_Hash_Head)
	sw	$0,28($sp)
	addu	$2,$3,$2
	lw	$2,4($2)
	sw	$2,24($sp)
	lw	$19,0($2)
	bne	$19,$2,$L147
	nop

	j	$L168
	addiu	$17,$17,1

$L152:
	lw	$2,28($sp)
	addiu	$16,$19,-204
	addiu	$2,$2,1
	sw	$2,28($sp)
	lb	$2,212($16)
	bne	$2,$fp,$L176
	nop

	jal	__conntrack_drop_check
	move	$4,$16

	sb	$2,212($16)
	lb	$2,212($16)
$L176:
	bne	$2,$0,$L149
	nop

	jal	rtl_get_ct_timer_expires
	move	$4,$16

	lw	$4,%lo(jiffies)($23)
	lhu	$3,0($18)
	subu	$4,$2,$4
	srl	$4,$4,9
	sltu	$3,$3,$4
	bne	$3,$0,$L150
	nop

	jal	rtl_del_ct_timer
	move	$4,$16

	beq	$2,$0,$L150
	move	$4,$16

	li	$6,2			# 0x2
	move	$7,$0
	move	$5,$17
	jal	__drop_one_conntrack_process_hooks1
	sw	$21,16($sp)

	bne	$2,$22,$L151
	li	$3,2			# 0x2

$L163:
	jal	local_bh_enable
	nop

	jal	rtl_death_action
	move	$4,$16

	j	$L169
	lui	$2,%hi(rtl_newGC_session_status_flags)

$L151:
	beq	$2,$3,$L129
	move	$4,$16

	li	$6,2			# 0x2
	move	$7,$0
	move	$5,$17
	jal	__drop_one_conntrack_process_hooks2
	sw	$21,16($sp)

$L150:
	lw	$3,28($sp)
	slt	$2,$3,129
	beq	$2,$0,$L140
	nop

$L149:
	lw	$19,0($19)
$L147:
	lw	$2,24($sp)
	bne	$19,$2,$L152
	nop

$L140:
	addiu	$17,$17,1
$L168:
	addiu	$18,$18,4
$L138:
	lui	$3,%hi(drop_priority_max_idx)
	lw	$2,%lo(drop_priority_max_idx)($3)
	slt	$2,$17,$2
	bne	$2,$0,$L153
	nop

	jal	local_bh_enable
	nop

	lui	$3,%hi(rtl_newGC_session_status_flags)
	lw	$5,%lo(rtl_newGC_session_status_flags)($3)
	li	$4,1			# 0x1
	bne	$5,$4,$L166
	move	$2,$0

	j	$L111
	nop

$L129:
	lui	$2,%hi(rtl_newGC_session_status_flags)
$L169:
	lw	$3,%lo(rtl_newGC_session_status_flags)($2)
	bne	$3,$0,$L108
	lui	$3,%hi(jiffies)

	lw	$4,%lo(jiffies)($3)
	li	$3,3			# 0x3
	sw	$3,%lo(rtl_newGC_session_status_flags)($2)
	li	$2,1			# 0x1
	j	$L164
	addiu	$4,$4,1

$L166:
	lui	$2,%hi(jiffies)
	lw	$5,%lo(jiffies)($2)
	move	$2,$0
	sw	$4,%lo(rtl_newGC_session_status_flags)($3)
	addiu	$4,$5,200
$L164:
	lui	$3,%hi(rtl_newGC_session_status_time)
	j	$L111
	sw	$4,%lo(rtl_newGC_session_status_time)($3)

$L108:
	li	$2,1			# 0x1
$L111:
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
	.end	drop_one_conntrack
	.size	drop_one_conntrack, .-drop_one_conntrack
	.section	.text.conntrack_dointvec_minmax,"ax",@progbits
	.align	2
	.globl	conntrack_dointvec_minmax
	.set	nomips16
	.ent	conntrack_dointvec_minmax
	.type	conntrack_dointvec_minmax, @function
conntrack_dointvec_minmax:
	.frame	$sp,40,$31		# vars= 0, regs= 3/0, args= 24, gp= 0
	.mask	0x80030000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-40
	sw	$31,36($sp)
	sw	$17,32($sp)
	sw	$16,28($sp)
	move	$16,$5
	lw	$2,56($sp)
	sw	$2,16($sp)
	lw	$2,60($sp)
	jal	proc_dointvec_minmax
	sw	$2,20($sp)

	bne	$2,$0,$L178
	move	$17,$2

	beq	$16,$0,$L178
	lui	$8,%hi(_prot_limit)

	lui	$7,%hi(prot_limit)
	addiu	$8,$8,%lo(_prot_limit)
	addiu	$7,$7,%lo(prot_limit)
	move	$3,$0
	lui	$6,%hi(nf_conntrack_max)
	li	$4,100			# 0x64
	li	$5,12			# 0xc
$L179:
	addu	$2,$7,$3
	addu	$9,$8,$3
	lw	$10,0($2)
	addiu	$3,$3,4
	lw	$2,%lo(nf_conntrack_max)($6)
	mult	$10,$2
	mflo	$10
	divu	$0,$10,$4
	mflo	$10
	sw	$10,0($9)
	bne	$3,$5,$L179
	nop

	sll	$5,$2,2
	li	$4,5			# 0x5
	lui	$3,%hi(rtl_nf_conntrack_threshold)
	divu	$0,$5,$4
	mflo	$5
	subu	$4,$2,$5
	sltu	$4,$4,65
	bne	$4,$0,$L180
	sw	$5,%lo(rtl_nf_conntrack_threshold)($3)

	addiu	$4,$2,-64
	sw	$4,%lo(rtl_nf_conntrack_threshold)($3)
$L180:
	beq	$2,$0,$L184
	lui	$2,%hi(rtl_newGC_session_status_flags)

	jal	rtl_gc_threshold_check
	move	$4,$0

	bne	$2,$0,$L181
	lui	$2,%hi(rtl_newGC_session_status_flags)

	li	$3,3			# 0x3
	lw	$4,%lo(rtl_newGC_session_status_flags)($2)
	beq	$4,$3,$L178
	lui	$4,%hi(jiffies)

	lw	$4,%lo(jiffies)($4)
	sw	$3,%lo(rtl_newGC_session_status_flags)($2)
	lui	$2,%hi(rtl_newGC_session_status_time)
	addiu	$3,$4,1
	j	$L178
	sw	$3,%lo(rtl_newGC_session_status_time)($2)

$L181:
$L184:
	sw	$0,%lo(rtl_newGC_session_status_flags)($2)
$L178:
	move	$2,$17
	lw	$31,36($sp)
	lw	$17,32($sp)
	lw	$16,28($sp)
	j	$31
	addiu	$sp,$sp,40

	.set	macro
	.set	reorder
	.end	conntrack_dointvec_minmax
	.size	conntrack_dointvec_minmax, .-conntrack_dointvec_minmax
	.section	.text.conntrack_dointvec,"ax",@progbits
	.align	2
	.globl	conntrack_dointvec
	.set	nomips16
	.ent	conntrack_dointvec
	.type	conntrack_dointvec, @function
conntrack_dointvec:
	.frame	$sp,40,$31		# vars= 0, regs= 3/0, args= 24, gp= 0
	.mask	0x80030000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-40
	sw	$31,36($sp)
	sw	$17,32($sp)
	sw	$16,28($sp)
	move	$16,$5
	lw	$2,56($sp)
	sw	$2,16($sp)
	lw	$2,60($sp)
	jal	proc_dointvec
	sw	$2,20($sp)

	bne	$2,$0,$L186
	move	$17,$2

	beq	$16,$0,$L186
	lui	$8,%hi(_prot_limit)

	lui	$7,%hi(prot_limit)
	addiu	$8,$8,%lo(_prot_limit)
	addiu	$7,$7,%lo(prot_limit)
	move	$3,$0
	lui	$6,%hi(nf_conntrack_max)
	li	$4,100			# 0x64
	li	$5,12			# 0xc
$L187:
	addu	$2,$7,$3
	addu	$9,$8,$3
	lw	$10,0($2)
	addiu	$3,$3,4
	lw	$2,%lo(nf_conntrack_max)($6)
	mult	$10,$2
	mflo	$10
	divu	$0,$10,$4
	mflo	$10
	sw	$10,0($9)
	bne	$3,$5,$L187
	nop

	sll	$5,$2,2
	li	$4,5			# 0x5
	lui	$3,%hi(rtl_nf_conntrack_threshold)
	divu	$0,$5,$4
	mflo	$5
	subu	$4,$2,$5
	sltu	$4,$4,65
	bne	$4,$0,$L188
	sw	$5,%lo(rtl_nf_conntrack_threshold)($3)

	addiu	$4,$2,-64
	sw	$4,%lo(rtl_nf_conntrack_threshold)($3)
$L188:
	beq	$2,$0,$L192
	lui	$2,%hi(rtl_newGC_session_status_flags)

	jal	rtl_gc_threshold_check
	move	$4,$0

	bne	$2,$0,$L189
	lui	$2,%hi(rtl_newGC_session_status_flags)

	li	$3,3			# 0x3
	lw	$4,%lo(rtl_newGC_session_status_flags)($2)
	beq	$4,$3,$L186
	lui	$4,%hi(jiffies)

	lw	$4,%lo(jiffies)($4)
	sw	$3,%lo(rtl_newGC_session_status_flags)($2)
	lui	$2,%hi(rtl_newGC_session_status_time)
	addiu	$3,$4,1
	j	$L186
	sw	$3,%lo(rtl_newGC_session_status_time)($2)

$L189:
$L192:
	sw	$0,%lo(rtl_newGC_session_status_flags)($2)
$L186:
	move	$2,$17
	lw	$31,36($sp)
	lw	$17,32($sp)
	lw	$16,28($sp)
	j	$31
	addiu	$sp,$sp,40

	.set	macro
	.set	reorder
	.end	conntrack_dointvec
	.size	conntrack_dointvec, .-conntrack_dointvec
	.globl	conntrack_min
	.section	.bss,"aw",@nobits
	.align	2
	.type	conntrack_min, @object
	.size	conntrack_min, 4
conntrack_min:
	.space	4
	.globl	conntrack_max
	.data
	.align	2
	.type	conntrack_max, @object
	.size	conntrack_max, 4
conntrack_max:
	.word	100
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
	.globl	drop_priority
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
	.local	prot_counters
	.comm	prot_counters,12,4
	.local	_prot_limit
	.comm	_prot_limit,12,4
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
	.data
	.align	2
	.type	syn_sent_list, @object
	.size	syn_sent_list, 8
syn_sent_list:
	.word	syn_sent_list
	.word	syn_sent_list
	.align	2
	.type	syn_recv_list, @object
	.size	syn_recv_list, 8
syn_recv_list:
	.word	syn_recv_list
	.word	syn_recv_list
	.align	2
	.type	established_list, @object
	.size	established_list, 8
established_list:
	.word	established_list
	.word	established_list
	.align	2
	.type	fin_wait_list, @object
	.size	fin_wait_list, 8
fin_wait_list:
	.word	fin_wait_list
	.word	fin_wait_list
	.align	2
	.type	close_wait_list, @object
	.size	close_wait_list, 8
close_wait_list:
	.word	close_wait_list
	.word	close_wait_list
	.align	2
	.type	last_ack_list, @object
	.size	last_ack_list, 8
last_ack_list:
	.word	last_ack_list
	.word	last_ack_list
	.align	2
	.type	time_wait_list, @object
	.size	time_wait_list, 8
time_wait_list:
	.word	time_wait_list
	.word	time_wait_list
	.align	2
	.type	close_list, @object
	.size	close_list, 8
close_list:
	.word	close_list
	.word	close_list
	.align	2
	.type	listen_list, @object
	.size	listen_list, 8
listen_list:
	.word	listen_list
	.word	listen_list
	.align	2
	.type	udp_unreply_list, @object
	.size	udp_unreply_list, 8
udp_unreply_list:
	.word	udp_unreply_list
	.word	udp_unreply_list
	.align	2
	.type	udp_assured_list, @object
	.size	udp_assured_list, 8
udp_assured_list:
	.word	udp_assured_list
	.word	udp_assured_list
	.ident	"GCC: (GNU) 4.4.5-1.5.5p4"
