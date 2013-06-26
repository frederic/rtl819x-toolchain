	.file	1 "filter.c"
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
	.section	.text.str2hexnum,"ax",@progbits
	.align	2
	.globl	str2hexnum
	.set	nomips16
	.ent	str2hexnum
	.type	str2hexnum, @function
str2hexnum:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	j	$L2
	move	$2,$0

$L7:
	sltu	$2,$5,10
	bne	$2,$0,$L3
	addiu	$2,$3,-65

	addiu	$5,$3,-87
	bne	$6,$0,$L3
	sltu	$2,$2,6

	bne	$2,$0,$L5
	addiu	$5,$3,-55

	j	$31
	li	$2,-1			# 0xffffffff

$L5:
$L3:
	addu	$2,$5,$7
$L2:
	lbu	$3,0($4)
	sll	$7,$2,4
	addiu	$4,$4,1
	addiu	$6,$3,-97
	addiu	$5,$3,-48
	bne	$3,$0,$L7
	sltu	$6,$6,6

	j	$31
	nop

	.set	macro
	.set	reorder
	.end	str2hexnum
	.size	str2hexnum, .-str2hexnum
	.section	.text.strtoint,"ax",@progbits
	.align	2
	.globl	strtoint
	.set	nomips16
	.ent	strtoint
	.type	strtoint, @function
strtoint:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	move	$2,$0
	j	$L11
	li	$3,10			# 0xa

$L13:
	beq	$7,$0,$L15
	mflo	$5

	addu	$2,$6,$5
$L11:
	lb	$5,0($4)
	mult	$2,$3
	addiu	$4,$4,1
	addiu	$6,$5,-48
	bne	$5,$0,$L13
	sltu	$7,$6,10

$L15:
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	strtoint
	.size	strtoint, .-strtoint
	.section	.text.Del_Pattern_ACL_For_ContentFilter,"ax",@progbits
	.align	2
	.globl	Del_Pattern_ACL_For_ContentFilter
	.set	nomips16
	.ent	Del_Pattern_ACL_For_ContentFilter
	.type	Del_Pattern_ACL_For_ContentFilter, @function
Del_Pattern_ACL_For_ContentFilter:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	j	$31
	move	$2,$0

	.set	macro
	.set	reorder
	.end	Del_Pattern_ACL_For_ContentFilter
	.size	Del_Pattern_ACL_For_ContentFilter, .-Del_Pattern_ACL_For_ContentFilter
	.section	.text.filter_addconnect,"ax",@progbits
	.align	2
	.globl	filter_addconnect
	.set	nomips16
	.ent	filter_addconnect
	.type	filter_addconnect, @function
filter_addconnect:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	andi	$3,$4,0xff
	lui	$2,%hi(ConnectedIp)
	sll	$3,$3,2
	addiu	$2,$2,%lo(ConnectedIp)
	addu	$2,$3,$2
	j	$31
	sw	$4,0($2)

	.set	macro
	.set	reorder
	.end	filter_addconnect
	.size	filter_addconnect, .-filter_addconnect
	.section	.text.filter_delconnect,"ax",@progbits
	.align	2
	.globl	filter_delconnect
	.set	nomips16
	.ent	filter_delconnect
	.type	filter_delconnect, @function
filter_delconnect:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	andi	$4,$4,0x00ff
	lui	$2,%hi(ConnectedIp)
	sll	$4,$4,2
	addiu	$2,$2,%lo(ConnectedIp)
	addu	$4,$4,$2
	j	$31
	sw	$0,0($4)

	.set	macro
	.set	reorder
	.end	filter_delconnect
	.size	filter_delconnect, .-filter_delconnect
	.section	.text.filter_checkConnect,"ax",@progbits
	.align	2
	.globl	filter_checkConnect
	.set	nomips16
	.ent	filter_checkConnect
	.type	filter_checkConnect, @function
filter_checkConnect:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	andi	$5,$4,0xff
	lui	$3,%hi(ConnectedIp)
	sll	$5,$5,2
	addiu	$3,$3,%lo(ConnectedIp)
	addu	$3,$5,$3
	lw	$3,0($3)
	beq	$3,$0,$L26
	li	$2,1			# 0x1

	xor	$2,$3,$4
	sltu	$2,$0,$2
$L26:
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	filter_checkConnect
	.size	filter_checkConnect, .-filter_checkConnect
	.section	.text.register_customRspHook,"ax",@progbits
	.align	2
	.globl	register_customRspHook
	.set	nomips16
	.ent	register_customRspHook
	.type	register_customRspHook, @function
register_customRspHook:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	beq	$4,$0,$L28
	lui	$2,%hi(fast_path_cusRsp401_func)

	j	$L29
	sw	$4,%lo(fast_path_cusRsp401_func)($2)

$L28:
	lui	$3,%hi(GenerateHTTP401)
	addiu	$3,$3,%lo(GenerateHTTP401)
	sw	$3,%lo(fast_path_cusRsp401_func)($2)
$L29:
	beq	$5,$0,$L30
	lui	$2,%hi(fast_path_cusRspTCPFinAck_func)

	j	$L31
	sw	$5,%lo(fast_path_cusRspTCPFinAck_func)($2)

$L30:
	lui	$3,%hi(GenerateTCPFINACK)
	addiu	$3,$3,%lo(GenerateTCPFINACK)
	sw	$3,%lo(fast_path_cusRspTCPFinAck_func)($2)
$L31:
	beq	$6,$0,$L32
	lui	$2,%hi(fast_path_cusRspTCPEndAck_func)

	j	$31
	sw	$6,%lo(fast_path_cusRspTCPEndAck_func)($2)

$L32:
	lui	$3,%hi(GenerateTCPENDACK)
	addiu	$3,$3,%lo(GenerateTCPENDACK)
	j	$31
	sw	$3,%lo(fast_path_cusRspTCPEndAck_func)($2)

	.set	macro
	.set	reorder
	.end	register_customRspHook
	.size	register_customRspHook, .-register_customRspHook
	.section	.text.unregister_customRspHook,"ax",@progbits
	.align	2
	.globl	unregister_customRspHook
	.set	nomips16
	.ent	unregister_customRspHook
	.type	unregister_customRspHook, @function
unregister_customRspHook:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	lui	$2,%hi(fast_path_cusRsp401_func)
	sw	$0,%lo(fast_path_cusRsp401_func)($2)
	lui	$2,%hi(fast_path_cusRspTCPFinAck_func)
	sw	$0,%lo(fast_path_cusRspTCPFinAck_func)($2)
	lui	$2,%hi(fast_path_cusRspTCPEndAck_func)
	j	$31
	sw	$0,%lo(fast_path_cusRspTCPEndAck_func)($2)

	.set	macro
	.set	reorder
	.end	unregister_customRspHook
	.size	unregister_customRspHook, .-unregister_customRspHook
	.section	.text.rtl_schedule_filter_fastpath,"ax",@progbits
	.align	2
	.set	nomips16
	.ent	rtl_schedule_filter_fastpath
	.type	rtl_schedule_filter_fastpath, @function
rtl_schedule_filter_fastpath:
	.frame	$sp,40,$31		# vars= 16, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-40
	sw	$31,36($sp)
	addiu	$4,$sp,16
	jal	do_gettimeofday
	sw	$5,24($sp)

	li	$4,65536			# 0x10000
	lw	$3,16($sp)
	ori	$4,$4,0x5180
	li	$2,7			# 0x7
	lw	$5,24($sp)
	div	$0,$3,$4
	lw	$4,8($5)
	mflo	$6
	addiu	$6,$6,4
	div	$0,$6,$2
	li	$6,1			# 0x1
	mfhi	$2
	sll	$2,$6,$2
	ori	$2,$2,0x80
	and	$4,$2,$4
	beq	$4,$0,$L41
	move	$2,$0

	li	$6,3600			# 0xe10
	li	$2,60			# 0x3c
	div	$0,$3,$6
	li	$6,24			# 0x18
	lw	$4,0($5)
	mflo	$7
	div	$0,$3,$2
	mflo	$3
	div	$0,$7,$6
	mfhi	$6
	div	$0,$3,$2
	mfhi	$8
	mult	$6,$2
	mflo	$6
	bne	$4,$0,$L39
	addu	$8,$6,$8

	lw	$3,4($5)
	bne	$3,$0,$L40
	li	$2,1			# 0x1

	j	$L41
	nop

$L39:
	sltu	$4,$8,$4
	bne	$4,$0,$L38
	nop

$L40:
	lw	$2,4($5)
	sltu	$8,$2,$8
	j	$L41
	xori	$2,$8,0x1

$L38:
	move	$2,$0
$L41:
	lw	$31,36($sp)
	j	$31
	addiu	$sp,$sp,40

	.set	macro
	.set	reorder
	.end	rtl_schedule_filter_fastpath
	.size	rtl_schedule_filter_fastpath, .-rtl_schedule_filter_fastpath
	.section	.text.rtl_ip_range_filter_fastpath,"ax",@progbits
	.align	2
	.set	nomips16
	.ent	rtl_ip_range_filter_fastpath
	.type	rtl_ip_range_filter_fastpath, @function
rtl_ip_range_filter_fastpath:
	.frame	$sp,32,$31		# vars= 8, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	jal	rtl_ip_hdr
	sw	$5,16($sp)

	lw	$5,16($sp)
	lw	$2,12($2)
	lw	$4,0($5)
	sltu	$4,$2,$4
	bne	$4,$0,$L45
	move	$3,$0

	lw	$3,4($5)
	sltu	$3,$3,$2
	xori	$3,$3,0x1
$L45:
	lw	$31,28($sp)
	move	$2,$3
	j	$31
	addiu	$sp,$sp,32

	.set	macro
	.set	reorder
	.end	rtl_ip_range_filter_fastpath
	.size	rtl_ip_range_filter_fastpath, .-rtl_ip_range_filter_fastpath
	.section	.rodata.str1.4,"aMS",@progbits,1
	.align	2
$LC0:
	.ascii	"filter_table\000"
	.section	.text.filter_exit_fastpath,"ax",@progbits
	.align	2
	.globl	filter_exit_fastpath
	.set	nomips16
	.ent	filter_exit_fastpath
	.type	filter_exit_fastpath, @function
filter_exit_fastpath:
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$16,16($sp)
	lui	$16,%hi(res12)
	lw	$5,%lo(res12)($16)
	beq	$5,$0,$L50
	nop

	lui	$4,%hi($LC0)
	jal	remove_proc_entry
	addiu	$4,$4,%lo($LC0)

	sw	$0,%lo(res12)($16)
$L50:
	lw	$31,20($sp)
	lw	$16,16($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	filter_exit_fastpath
	.size	filter_exit_fastpath, .-filter_exit_fastpath
	.section	.rodata.str1.4
	.align	2
$LC1:
	.ascii	"enable_dos\000"
	.section	.exit.text,"ax",@progbits
	.align	2
	.globl	filter_exit
	.set	nomips16
	.ent	filter_exit
	.type	filter_exit, @function
filter_exit:
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$16,16($sp)
	lui	$16,%hi(res3)
	lw	$5,%lo(res3)($16)
	beq	$5,$0,$L52
	nop

	lui	$4,%hi($LC1)
	jal	remove_proc_entry
	addiu	$4,$4,%lo($LC1)

	sw	$0,%lo(res3)($16)
$L52:
	lui	$4,%hi(dos_timer)
	jal	del_timer
	addiu	$4,$4,%lo(dos_timer)

	lw	$31,20($sp)
	lw	$16,16($sp)
	j	filter_exit_fastpath
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	filter_exit
	.size	filter_exit, .-filter_exit
	.section	.text.filter_table_head_init,"ax",@progbits
	.align	2
	.globl	filter_table_head_init
	.set	nomips16
	.ent	filter_table_head_init
	.type	filter_table_head_init, @function
filter_table_head_init:
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$16,16($sp)
	lui	$16,%hi(table_list_head)
	addiu	$4,$16,%lo(table_list_head)
	move	$5,$0
	jal	memset
	li	$6,32			# 0x20

	addiu	$3,$16,%lo(table_list_head)
	sw	$3,%lo(table_list_head)($16)
	lui	$4,%hi(table_list_head+8)
	lw	$31,20($sp)
	addiu	$4,$4,%lo(table_list_head+8)
	lw	$16,16($sp)
	move	$2,$0
	li	$5,1			# 0x1
	sw	$5,24($3)
	sw	$4,12($3)
	sw	$3,4($3)
	sw	$4,8($3)
	sw	$0,16($3)
	sw	$0,20($3)
	sw	$0,28($3)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	filter_table_head_init
	.size	filter_table_head_init, .-filter_table_head_init
	.section	.text.filter_table_init,"ax",@progbits
	.align	2
	.globl	filter_table_init
	.set	nomips16
	.ent	filter_table_init
	.type	filter_table_init, @function
filter_table_init:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	jal	filter_table_head_init
	nop

	lui	$4,%hi($LC0)
	addiu	$4,$4,%lo($LC0)
	move	$5,$0
	jal	create_proc_entry
	move	$6,$0

	lui	$3,%hi(res12)
	beq	$2,$0,$L57
	sw	$2,%lo(res12)($3)

	lui	$3,%hi(filter_table_write_proc)
	addiu	$3,$3,%lo(filter_table_write_proc)
	sw	$3,68($2)
	lui	$3,%hi(filter_table_read_proc)
	addiu	$3,$3,%lo(filter_table_read_proc)
	sw	$3,64($2)
$L57:
	lw	$31,20($sp)
	move	$2,$0
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	filter_table_init
	.size	filter_table_init, .-filter_table_init
	.section	.text.filter_init_fastpath,"ax",@progbits
	.align	2
	.globl	filter_init_fastpath
	.set	nomips16
	.ent	filter_init_fastpath
	.type	filter_init_fastpath, @function
filter_init_fastpath:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	li	$3,1			# 0x1
	lui	$2,%hi(filter_rule_num)
	jal	filter_table_init
	sw	$3,%lo(filter_rule_num)($2)

	move	$2,$0
	lw	$31,20($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	filter_init_fastpath
	.size	filter_init_fastpath, .-filter_init_fastpath
	.section	.init.text,"ax",@progbits
	.align	2
	.globl	filter_init
	.set	nomips16
	.ent	filter_init
	.type	filter_init, @function
filter_init:
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$16,16($sp)
	lui	$2,%hi(item)
	sw	$0,%lo(item)($2)
	lui	$4,%hi($LC1)
	lui	$2,%hi(none_dos_drop_pkt_cnt)
	addiu	$4,$4,%lo($LC1)
	sw	$0,%lo(none_dos_drop_pkt_cnt)($2)
	move	$5,$0
	lui	$2,%hi(url_count)
	move	$6,$0
	jal	create_proc_entry
	sb	$0,%lo(url_count)($2)

	lui	$3,%hi(res3)
	beq	$2,$0,$L62
	sw	$2,%lo(res3)($3)

	lui	$3,%hi(dos_write_proc)
	addiu	$3,$3,%lo(dos_write_proc)
	sw	$3,68($2)
	lui	$3,%hi(dos_read_proc)
	addiu	$3,$3,%lo(dos_read_proc)
	sw	$3,64($2)
$L62:
	lui	$16,%hi(dos_timer)
	move	$6,$0
	addiu	$4,$16,%lo(dos_timer)
	jal	init_timer_key
	move	$5,$0

	lui	$4,%hi(jiffies)
	lw	$3,%lo(jiffies)($4)
	addiu	$2,$16,%lo(dos_timer)
	lw	$5,%lo(jiffies)($4)
	lui	$16,%hi(_tcpDosScanBitmap)
	move	$4,$2
	addiu	$3,$3,100
	sw	$3,8($2)
	lui	$3,%hi(dos_timer_fn)
	addiu	$3,$3,%lo(dos_timer_fn)
	addiu	$5,$5,100
	sw	$3,12($2)
	jal	mod_timer
	sw	$0,16($2)

	addiu	$4,$16,%lo(_tcpDosScanBitmap)
	move	$5,$0
	jal	memset
	li	$6,64			# 0x40

	addiu	$3,$16,%lo(_tcpDosScanBitmap)
	li	$2,1			# 0x1
	sb	$2,3($3)
	sb	$2,63($3)
	sb	$2,58($3)
	sb	$2,41($3)
	sb	$2,40($3)
	sb	$2,33($3)
	sb	$2,32($3)
	sb	$2,9($3)
	sb	$2,8($3)
	jal	filter_init_fastpath
	sb	$2,%lo(_tcpDosScanBitmap)($16)

	jal	igmp_delete_init_netlink
	nop

	move	$2,$0
	lw	$31,20($sp)
	lw	$16,16($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	filter_init
	.size	filter_init, .-filter_init
	.section	.rodata.str1.4
	.align	2
$LC2:
	.ascii	"DoS: Whole System SYN Flood Attack\012\000"
	.align	2
$LC3:
	.ascii	"DoS: Whole System FIN Flood Attack\012\000"
	.align	2
$LC4:
	.ascii	"DoS: Whole System UDP Flood Attack\012\000"
	.align	2
$LC5:
	.ascii	"DoS: Whole System ICMP Flood Attack\012\000"
	.align	2
$LC6:
	.ascii	"DoS: Per-source SYN Flood Attack source=%u.%u.%u.%u dest"
	.ascii	"ination=%u.%u.%u.%u\012\000"
	.align	2
$LC7:
	.ascii	"DoS: Per-source FIN Flood Attack source=%u.%u.%u.%u dest"
	.ascii	"ination=%u.%u.%u.%u\012\000"
	.align	2
$LC8:
	.ascii	"DoS: Per-source UDP Flood Attack source=%u.%u.%u.%u dest"
	.ascii	"ination=%u.%u.%u.%u\012\000"
	.align	2
$LC9:
	.ascii	"DoS: Per-source ICMP Flood Attack source=%u.%u.%u.%u des"
	.ascii	"tination=%u.%u.%u.%u\012\000"
	.align	2
$LC10:
	.ascii	"DoS: Port Scan Attack source=%u.%u.%u.%u destination=%u."
	.ascii	"%u.%u.%u\012\000"
	.align	2
$LC11:
	.ascii	"DoS: Tcp Scan Attack source=%u.%u.%u.%u destination=%u.%"
	.ascii	"u.%u.%u\012\000"
	.align	2
$LC12:
	.ascii	"DoS: Tcp SYN With Data Attack source=%u.%u.%u.%u destina"
	.ascii	"tion=%u.%u.%u.%u\012\000"
	.align	2
$LC13:
	.ascii	"DoS: IP Land Attack source=%u.%u.%u.%u destination=%u.%u"
	.ascii	".%u.%u\012\000"
	.align	2
$LC14:
	.ascii	"DoS: UdpEchoChargen Attack source=%u.%u.%u.%u destinatio"
	.ascii	"n=%u.%u.%u.%u\012\000"
	.align	2
$LC15:
	.ascii	"DoS: UdpBomb Attack source=%u.%u.%u.%u destination=%u.%u"
	.ascii	".%u.%u\012\000"
	.align	2
$LC16:
	.ascii	"DoS: PingOfDeath Attack source=%u.%u.%u.%u destination=%"
	.ascii	"u.%u.%u.%u\012\000"
	.align	2
$LC17:
	.ascii	"DoS: IcmpSmurf Attack source=%u.%u.%u.%u destination=%u."
	.ascii	"%u.%u.%u\012\000"
	.align	2
$LC18:
	.ascii	"DoS: IpSpoof Attack source=%u.%u.%u.%u destination=%u.%u"
	.ascii	".%u.%u\012\000"
	.align	2
$LC19:
	.ascii	"DoS: TearDrop Attack source=%u.%u.%u.%u destination=%u.%"
	.ascii	"u.%u.%u\012\000"
	.section	.text.dos_timer_fn,"ax",@progbits
	.align	2
	.set	nomips16
	.ent	dos_timer_fn
	.type	dos_timer_fn, @function
dos_timer_fn:
	.frame	$sp,48,$31		# vars= 0, regs= 2/0, args= 40, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-48
	sw	$31,44($sp)
	sw	$16,40($sp)
	lui	$2,%hi(item)
	lw	$2,%lo(item)($2)
	beq	$2,$0,$L65
	lui	$3,%hi(block_count)

	lui	$4,%hi(block_time)
	lw	$4,%lo(block_time)($4)
	lw	$5,%lo(block_count)($3)
	slt	$4,$5,$4
	bne	$4,$0,$L66
	lui	$4,%hi(block)

	sw	$0,%lo(block)($4)
	sw	$0,%lo(block_count)($3)
$L66:
	lui	$3,%hi(block)
	lw	$4,%lo(block)($3)
	li	$3,1			# 0x1
	bne	$4,$3,$L106
	andi	$3,$2,0x1e

	lui	$3,%hi(block_count)
	lw	$4,%lo(block_count)($3)
	addiu	$4,$4,1
	sw	$4,%lo(block_count)($3)
	andi	$3,$2,0x1e
$L106:
	beq	$3,$0,$L68
	lui	$13,%hi(dos_pkt+36828)

	lui	$3,%hi(dos_pkt)
	andi	$14,$2,0x2
	addiu	$13,$13,%lo(dos_pkt+36828)
	addiu	$3,$3,%lo(dos_pkt)
	andi	$12,$2,0x4
	move	$4,$0
	move	$5,$0
	andi	$11,$2,0x8
	move	$6,$0
	move	$7,$0
	andi	$10,$2,0x10
	li	$9,1			# 0x1
$L74:
	lb	$8,0($3)
	bne	$8,$9,$L69
	nop

	beq	$14,$0,$L70
	nop

	lw	$8,4($3)
	slt	$15,$0,$8
	addu	$8,$7,$8
	j	$L70
	movn	$7,$8,$15

$L101:
	lw	$8,8($3)
	slt	$15,$0,$8
	addu	$8,$6,$8
	j	$L71
	movn	$6,$8,$15

$L102:
	lw	$8,12($3)
	slt	$15,$0,$8
	addu	$8,$5,$8
	j	$L72
	movn	$5,$8,$15

$L103:
	lw	$8,16($3)
	slt	$15,$0,$8
	addu	$8,$4,$8
	movn	$4,$8,$15
$L69:
	bne	$3,$13,$L74
	addiu	$3,$3,36

	lui	$3,%hi(whole_syn_threshold)
	lw	$3,%lo(whole_syn_threshold)($3)
	slt	$7,$3,$7
	beq	$7,$0,$L75
	andi	$7,$2,0x2

	beq	$7,$0,$L75
	nop

	blez	$3,$L75
	lui	$3,%hi(LogFlag)

	lw	$7,%lo(LogFlag)($3)
	ori	$7,$7,0x2
	sw	$7,%lo(LogFlag)($3)
$L75:
	lui	$3,%hi(whole_fin_threshold)
	lw	$3,%lo(whole_fin_threshold)($3)
	slt	$6,$3,$6
	beq	$6,$0,$L76
	andi	$6,$2,0x4

	beq	$6,$0,$L76
	nop

	blez	$3,$L76
	lui	$3,%hi(LogFlag)

	lw	$6,%lo(LogFlag)($3)
	ori	$6,$6,0x4
	sw	$6,%lo(LogFlag)($3)
$L76:
	lui	$3,%hi(whole_udp_threshold)
	lw	$3,%lo(whole_udp_threshold)($3)
	slt	$5,$3,$5
	beq	$5,$0,$L77
	andi	$5,$2,0x8

	beq	$5,$0,$L77
	nop

	blez	$3,$L77
	lui	$3,%hi(LogFlag)

	lw	$5,%lo(LogFlag)($3)
	ori	$5,$5,0x8
	sw	$5,%lo(LogFlag)($3)
$L77:
	lui	$3,%hi(whole_icmp_threshold)
	lw	$3,%lo(whole_icmp_threshold)($3)
	slt	$4,$3,$4
	beq	$4,$0,$L68
	andi	$2,$2,0x10

	beq	$2,$0,$L107
	lui	$2,%hi(LogFlag)

	blez	$3,$L107
	nop

	lw	$3,%lo(LogFlag)($2)
	ori	$3,$3,0x10
	sw	$3,%lo(LogFlag)($2)
$L68:
	lui	$2,%hi(LogFlag)
$L107:
	lw	$16,%lo(LogFlag)($2)
	andi	$2,$16,0x2
	beq	$2,$0,$L108
	andi	$2,$16,0x4

	lui	$4,%hi($LC2)
	jal	scrlog_printk
	addiu	$4,$4,%lo($LC2)

	andi	$2,$16,0x4
$L108:
	beq	$2,$0,$L109
	andi	$2,$16,0x8

	lui	$4,%hi($LC3)
	jal	scrlog_printk
	addiu	$4,$4,%lo($LC3)

	andi	$2,$16,0x8
$L109:
	beq	$2,$0,$L110
	andi	$2,$16,0x10

	lui	$4,%hi($LC4)
	jal	scrlog_printk
	addiu	$4,$4,%lo($LC4)

	andi	$2,$16,0x10
$L110:
	beq	$2,$0,$L111
	andi	$2,$16,0x20

	lui	$4,%hi($LC5)
	jal	scrlog_printk
	addiu	$4,$4,%lo($LC5)

	andi	$2,$16,0x20
$L111:
	beq	$2,$0,$L112
	andi	$2,$16,0x40

	lui	$3,%hi(attack_saddr)
	lui	$2,%hi(attack_daddr)
	addiu	$3,$3,%lo(attack_saddr)
	addiu	$2,$2,%lo(attack_daddr)
	lbu	$6,5($3)
	lui	$4,%hi(attack_saddr+4)
	lbu	$7,6($3)
	lbu	$10,7($3)
	lui	$3,%hi(attack_daddr+4)
	lbu	$8,5($2)
	lbu	$9,%lo(attack_daddr+4)($3)
	lbu	$5,%lo(attack_saddr+4)($4)
	lui	$4,%hi($LC6)
	lbu	$3,7($2)
	addiu	$4,$4,%lo($LC6)
	lbu	$2,6($2)
	sw	$10,16($sp)
	sw	$9,20($sp)
	sw	$8,24($sp)
	sw	$2,28($sp)
	jal	scrlog_printk
	sw	$3,32($sp)

	andi	$2,$16,0x40
$L112:
	beq	$2,$0,$L113
	andi	$2,$16,0x80

	lui	$3,%hi(attack_saddr)
	lui	$2,%hi(attack_daddr)
	addiu	$3,$3,%lo(attack_saddr)
	addiu	$2,$2,%lo(attack_daddr)
	lbu	$6,9($3)
	lui	$4,%hi(attack_saddr+8)
	lbu	$7,10($3)
	lbu	$10,11($3)
	lui	$3,%hi(attack_daddr+8)
	lbu	$8,9($2)
	lbu	$9,%lo(attack_daddr+8)($3)
	lbu	$5,%lo(attack_saddr+8)($4)
	lui	$4,%hi($LC7)
	lbu	$3,11($2)
	addiu	$4,$4,%lo($LC7)
	lbu	$2,10($2)
	sw	$10,16($sp)
	sw	$9,20($sp)
	sw	$8,24($sp)
	sw	$2,28($sp)
	jal	scrlog_printk
	sw	$3,32($sp)

	andi	$2,$16,0x80
$L113:
	beq	$2,$0,$L114
	andi	$2,$16,0x100

	lui	$3,%hi(attack_saddr)
	lui	$2,%hi(attack_daddr)
	addiu	$3,$3,%lo(attack_saddr)
	addiu	$2,$2,%lo(attack_daddr)
	lbu	$6,13($3)
	lui	$4,%hi(attack_saddr+12)
	lbu	$7,14($3)
	lbu	$10,15($3)
	lui	$3,%hi(attack_daddr+12)
	lbu	$8,13($2)
	lbu	$9,%lo(attack_daddr+12)($3)
	lbu	$5,%lo(attack_saddr+12)($4)
	lui	$4,%hi($LC8)
	lbu	$3,15($2)
	addiu	$4,$4,%lo($LC8)
	lbu	$2,14($2)
	sw	$10,16($sp)
	sw	$9,20($sp)
	sw	$8,24($sp)
	sw	$2,28($sp)
	jal	scrlog_printk
	sw	$3,32($sp)

	andi	$2,$16,0x100
$L114:
	beq	$2,$0,$L115
	andi	$2,$16,0x200

	lui	$3,%hi(attack_saddr)
	lui	$2,%hi(attack_daddr)
	addiu	$3,$3,%lo(attack_saddr)
	addiu	$2,$2,%lo(attack_daddr)
	lbu	$6,17($3)
	lui	$4,%hi(attack_saddr+16)
	lbu	$7,18($3)
	lbu	$10,19($3)
	lui	$3,%hi(attack_daddr+16)
	lbu	$8,17($2)
	lbu	$9,%lo(attack_daddr+16)($3)
	lbu	$5,%lo(attack_saddr+16)($4)
	lui	$4,%hi($LC9)
	lbu	$3,19($2)
	addiu	$4,$4,%lo($LC9)
	lbu	$2,18($2)
	sw	$10,16($sp)
	sw	$9,20($sp)
	sw	$8,24($sp)
	sw	$2,28($sp)
	jal	scrlog_printk
	sw	$3,32($sp)

	andi	$2,$16,0x200
$L115:
	beq	$2,$0,$L116
	andi	$2,$16,0x8000

	lui	$3,%hi(attack_saddr)
	lui	$2,%hi(attack_daddr)
	addiu	$3,$3,%lo(attack_saddr)
	addiu	$2,$2,%lo(attack_daddr)
	lbu	$6,21($3)
	lui	$4,%hi(attack_saddr+20)
	lbu	$7,22($3)
	lbu	$10,23($3)
	lui	$3,%hi(attack_daddr+20)
	lbu	$8,21($2)
	lbu	$9,%lo(attack_daddr+20)($3)
	lbu	$5,%lo(attack_saddr+20)($4)
	lui	$4,%hi($LC10)
	lbu	$3,23($2)
	addiu	$4,$4,%lo($LC10)
	lbu	$2,22($2)
	sw	$10,16($sp)
	sw	$9,20($sp)
	sw	$8,24($sp)
	sw	$2,28($sp)
	jal	scrlog_printk
	sw	$3,32($sp)

	andi	$2,$16,0x8000
$L116:
	beq	$2,$0,$L117
	li	$2,65536			# 0x10000

	lui	$3,%hi(attack_saddr)
	lui	$2,%hi(attack_daddr)
	addiu	$3,$3,%lo(attack_saddr)
	addiu	$2,$2,%lo(attack_daddr)
	lbu	$6,45($3)
	lui	$4,%hi(attack_saddr+44)
	lbu	$7,46($3)
	lbu	$10,47($3)
	lui	$3,%hi(attack_daddr+44)
	lbu	$8,45($2)
	lbu	$9,%lo(attack_daddr+44)($3)
	lbu	$5,%lo(attack_saddr+44)($4)
	lui	$4,%hi($LC11)
	lbu	$3,47($2)
	addiu	$4,$4,%lo($LC11)
	lbu	$2,46($2)
	sw	$10,16($sp)
	sw	$9,20($sp)
	sw	$8,24($sp)
	sw	$2,28($sp)
	jal	scrlog_printk
	sw	$3,32($sp)

	li	$2,65536			# 0x10000
$L117:
	and	$2,$16,$2
	beq	$2,$0,$L118
	andi	$2,$16,0x800

	lui	$3,%hi(attack_saddr)
	lui	$2,%hi(attack_daddr)
	addiu	$3,$3,%lo(attack_saddr)
	addiu	$2,$2,%lo(attack_daddr)
	lbu	$6,49($3)
	lui	$4,%hi(attack_saddr+48)
	lbu	$7,50($3)
	lbu	$10,51($3)
	lui	$3,%hi(attack_daddr+48)
	lbu	$8,49($2)
	lbu	$9,%lo(attack_daddr+48)($3)
	lbu	$5,%lo(attack_saddr+48)($4)
	lui	$4,%hi($LC12)
	lbu	$3,51($2)
	addiu	$4,$4,%lo($LC12)
	lbu	$2,50($2)
	sw	$10,16($sp)
	sw	$9,20($sp)
	sw	$8,24($sp)
	sw	$2,28($sp)
	jal	scrlog_printk
	sw	$3,32($sp)

	andi	$2,$16,0x800
$L118:
	beq	$2,$0,$L119
	li	$2,262144			# 0x40000

	lui	$3,%hi(attack_saddr)
	lui	$2,%hi(attack_daddr)
	addiu	$3,$3,%lo(attack_saddr)
	addiu	$2,$2,%lo(attack_daddr)
	lbu	$6,29($3)
	lui	$4,%hi(attack_saddr+28)
	lbu	$7,30($3)
	lbu	$10,31($3)
	lui	$3,%hi(attack_daddr+28)
	lbu	$8,29($2)
	lbu	$9,%lo(attack_daddr+28)($3)
	lbu	$5,%lo(attack_saddr+28)($4)
	lui	$4,%hi($LC13)
	lbu	$3,31($2)
	addiu	$4,$4,%lo($LC13)
	lbu	$2,30($2)
	sw	$10,16($sp)
	sw	$9,20($sp)
	sw	$8,24($sp)
	sw	$2,28($sp)
	jal	scrlog_printk
	sw	$3,32($sp)

	li	$2,262144			# 0x40000
$L119:
	and	$2,$16,$2
	beq	$2,$0,$L120
	li	$2,131072			# 0x20000

	lui	$3,%hi(attack_saddr)
	lui	$2,%hi(attack_daddr)
	addiu	$3,$3,%lo(attack_saddr)
	addiu	$2,$2,%lo(attack_daddr)
	lbu	$5,56($3)
	lui	$4,%hi($LC14)
	lbu	$6,57($3)
	addiu	$4,$4,%lo($LC14)
	lbu	$7,58($3)
	lbu	$10,59($3)
	lbu	$9,56($2)
	lbu	$3,59($2)
	lbu	$8,57($2)
	lbu	$2,58($2)
	sw	$10,16($sp)
	sw	$9,20($sp)
	sw	$8,24($sp)
	sw	$2,28($sp)
	jal	scrlog_printk
	sw	$3,32($sp)

	li	$2,131072			# 0x20000
$L120:
	and	$2,$16,$2
	beq	$2,$0,$L121
	andi	$2,$16,0x4000

	lui	$3,%hi(attack_saddr)
	lui	$2,%hi(attack_daddr)
	addiu	$3,$3,%lo(attack_saddr)
	addiu	$2,$2,%lo(attack_daddr)
	lbu	$6,53($3)
	lui	$4,%hi(attack_saddr+52)
	lbu	$7,54($3)
	lbu	$10,55($3)
	lui	$3,%hi(attack_daddr+52)
	lbu	$8,53($2)
	lbu	$9,%lo(attack_daddr+52)($3)
	lbu	$5,%lo(attack_saddr+52)($4)
	lui	$4,%hi($LC15)
	lbu	$3,55($2)
	addiu	$4,$4,%lo($LC15)
	lbu	$2,54($2)
	sw	$10,16($sp)
	sw	$9,20($sp)
	sw	$8,24($sp)
	sw	$2,28($sp)
	jal	scrlog_printk
	sw	$3,32($sp)

	andi	$2,$16,0x4000
$L121:
	beq	$2,$0,$L122
	andi	$2,$16,0x400

	lui	$3,%hi(attack_saddr)
	lui	$2,%hi(attack_daddr)
	addiu	$3,$3,%lo(attack_saddr)
	addiu	$2,$2,%lo(attack_daddr)
	lbu	$6,41($3)
	lui	$4,%hi(attack_saddr+40)
	lbu	$7,42($3)
	lbu	$10,43($3)
	lui	$3,%hi(attack_daddr+40)
	lbu	$8,41($2)
	lbu	$9,%lo(attack_daddr+40)($3)
	lbu	$5,%lo(attack_saddr+40)($4)
	lui	$4,%hi($LC16)
	lbu	$3,43($2)
	addiu	$4,$4,%lo($LC16)
	lbu	$2,42($2)
	sw	$10,16($sp)
	sw	$9,20($sp)
	sw	$8,24($sp)
	sw	$2,28($sp)
	jal	scrlog_printk
	sw	$3,32($sp)

	andi	$2,$16,0x400
$L122:
	beq	$2,$0,$L123
	andi	$2,$16,0x1000

	lui	$3,%hi(attack_saddr)
	lui	$2,%hi(attack_daddr)
	addiu	$3,$3,%lo(attack_saddr)
	addiu	$2,$2,%lo(attack_daddr)
	lbu	$6,25($3)
	lui	$4,%hi(attack_saddr+24)
	lbu	$7,26($3)
	lbu	$10,27($3)
	lui	$3,%hi(attack_daddr+24)
	lbu	$8,25($2)
	lbu	$9,%lo(attack_daddr+24)($3)
	lbu	$5,%lo(attack_saddr+24)($4)
	lui	$4,%hi($LC17)
	lbu	$3,27($2)
	addiu	$4,$4,%lo($LC17)
	lbu	$2,26($2)
	sw	$10,16($sp)
	sw	$9,20($sp)
	sw	$8,24($sp)
	sw	$2,28($sp)
	jal	scrlog_printk
	sw	$3,32($sp)

	andi	$2,$16,0x1000
$L123:
	beq	$2,$0,$L94
	lui	$3,%hi(attack_saddr)

	lui	$2,%hi(attack_daddr)
	addiu	$3,$3,%lo(attack_saddr)
	addiu	$2,$2,%lo(attack_daddr)
	lbu	$6,33($3)
	lui	$4,%hi(attack_saddr+32)
	lbu	$7,34($3)
	lbu	$10,35($3)
	lui	$3,%hi(attack_daddr+32)
	lbu	$8,33($2)
	lbu	$9,%lo(attack_daddr+32)($3)
	lbu	$5,%lo(attack_saddr+32)($4)
	lui	$4,%hi($LC18)
	lbu	$3,35($2)
	addiu	$4,$4,%lo($LC18)
	lbu	$2,34($2)
	sw	$10,16($sp)
	sw	$9,20($sp)
	sw	$8,24($sp)
	sw	$2,28($sp)
	jal	scrlog_printk
	sw	$3,32($sp)

$L94:
	andi	$16,$16,0x2000
	beq	$16,$0,$L124
	lui	$2,%hi(LogFlag)

	lui	$3,%hi(attack_saddr)
	lui	$2,%hi(attack_daddr)
	addiu	$3,$3,%lo(attack_saddr)
	addiu	$2,$2,%lo(attack_daddr)
	lbu	$6,37($3)
	lui	$4,%hi(attack_saddr+36)
	lbu	$7,38($3)
	lbu	$10,39($3)
	lui	$3,%hi(attack_daddr+36)
	lbu	$8,37($2)
	lbu	$9,%lo(attack_daddr+36)($3)
	lbu	$5,%lo(attack_saddr+36)($4)
	lui	$4,%hi($LC19)
	lbu	$3,39($2)
	addiu	$4,$4,%lo($LC19)
	lbu	$2,38($2)
	sw	$10,16($sp)
	sw	$9,20($sp)
	sw	$8,24($sp)
	sw	$2,28($sp)
	jal	scrlog_printk
	sw	$3,32($sp)

	lui	$2,%hi(LogFlag)
$L124:
	lui	$3,%hi(dos_pkt+36864)
	sw	$0,%lo(LogFlag)($2)
	lui	$2,%hi(dos_pkt)
	addiu	$3,$3,%lo(dos_pkt+36864)
	addiu	$2,$2,%lo(dos_pkt)
$L96:
	sb	$0,0($2)
	sw	$0,24($2)
	sw	$0,4($2)
	sw	$0,8($2)
	sw	$0,12($2)
	sw	$0,16($2)
	sw	$0,20($2)
	addiu	$2,$2,36
	bne	$2,$3,$L96
	nop

$L65:
	lui	$2,%hi(none_dos_drop_pkt_cnt)
	lw	$2,%lo(none_dos_drop_pkt_cnt)($2)
	sltu	$3,$2,3001
	bne	$3,$0,$L97
	lui	$3,%hi(item_backup)

	lui	$2,%hi(item)
	lw	$3,%lo(item)($2)
	beq	$3,$0,$L98
	lui	$4,%hi(item_backup)

	sw	$3,%lo(item_backup)($4)
	j	$L98
	sw	$0,%lo(item)($2)

$L97:
	lw	$4,%lo(item_backup)($3)
	beq	$4,$0,$L99
	lui	$5,%hi(item)

	sw	$4,%lo(item)($5)
	sw	$0,%lo(item_backup)($3)
$L99:
	beq	$2,$0,$L125
	lui	$2,%hi(jiffies)

$L98:
	lui	$2,%hi(none_dos_drop_pkt_cnt)
	sw	$0,%lo(none_dos_drop_pkt_cnt)($2)
	lui	$2,%hi(jiffies)
$L125:
	lui	$4,%hi(dos_timer)
	lw	$5,%lo(jiffies)($2)
	addiu	$4,$4,%lo(dos_timer)
	lw	$31,44($sp)
	lw	$16,40($sp)
	addiu	$5,$5,100
	j	mod_timer
	addiu	$sp,$sp,48

$L70:
	bne	$12,$0,$L101
	nop

$L71:
	bne	$11,$0,$L102
	nop

$L72:
	bne	$10,$0,$L103
	nop

	j	$L69
	nop

	.set	macro
	.set	reorder
	.end	dos_timer_fn
	.size	dos_timer_fn, .-dos_timer_fn
	.section	.rodata.str1.4
	.align	2
$LC20:
	.ascii	"%s\012\000"
	.section	.text.dos_read_proc,"ax",@progbits
	.align	2
	.set	nomips16
	.ent	dos_read_proc
	.type	dos_read_proc, @function
dos_read_proc:
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
	move	$16,$6
	lui	$6,%hi(dos_flag)
	move	$17,$5
	lui	$5,%hi($LC20)
	addiu	$6,$6,%lo(dos_flag)
	addiu	$5,$5,%lo($LC20)
	move	$18,$4
	jal	sprintf
	sw	$7,16($sp)

	lw	$7,16($sp)
	addu	$3,$7,$16
	slt	$3,$3,$2
	bne	$3,$0,$L127
	li	$4,1			# 0x1

	lw	$3,56($sp)
	sw	$4,0($3)
$L127:
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
	.end	dos_read_proc
	.size	dos_read_proc, .-dos_read_proc
	.section	.rodata.str1.4
	.align	2
$LC21:
	.ascii	"br0\000"
	.align	2
$LC22:
	.ascii	"ppp0\000"
	.align	2
$LC23:
	.ascii	"eth1\000"
	.align	2
$LC24:
	.ascii	"wlan0\000"
	.section	.text.need_filter,"ax",@progbits
	.align	2
	.globl	need_filter
	.set	nomips16
	.ent	need_filter
	.type	need_filter, @function
need_filter:
	.frame	$sp,32,$31		# vars= 8, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$16,24($sp)
	jal	rtl_ip_hdr
	lui	$4,%hi($LC21)
	addiu	$4,$4,%lo($LC21)
	.set	noreorder
	.set	nomacro
	jal	rtl_get_dev_by_name
	move	$16,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L153
	lui	$4,%hi($LC22)
	.set	macro
	.set	reorder

	lw	$2,292($2)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L153
	sw	$2,16($sp)
	.set	macro
	.set	reorder

	lui	$3,%hi($LC21)
	lw	$2,12($2)
	.set	noreorder
	.set	nomacro
	j	$L132
	addiu	$6,$3,%lo($LC21)
	.set	macro
	.set	reorder

$L134:
#APP
 # 75 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/string.h" 1
	.set	noreorder
	.set	noat
	lbu	$7,($4)
1:	lbu	$1,($5)
	addiu	$4,1
	bne	$1,$7,2f
	addiu	$5,1
	bnez	$7,1b
	lbu	$7,($4)
	move	$7,$1
2:	subu	$7,$1
3:	.set	at
	.set	reorder
 # 0 "" 2
#NO_APP
	beq	$7,$0,$L133
	lw	$2,0($2)
$L132:
	addiu	$5,$2,35
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L134
	move	$4,$6
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L153
	lui	$4,%hi($LC22)
	.set	macro
	.set	reorder

$L133:
	lw	$3,20($2)
	lw	$4,16($16)
	.set	noreorder
	.set	nomacro
	beq	$4,$3,$L135
	move	$2,$0
	.set	macro
	.set	reorder

	lui	$4,%hi($LC22)
$L153:
	.set	noreorder
	.set	nomacro
	jal	rtl_get_dev_by_name
	addiu	$4,$4,%lo($LC22)
	.set	macro
	.set	reorder

	beq	$2,$0,$L136
	lw	$2,292($2)
	sw	$2,16($sp)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L138
	move	$3,$2
	.set	macro
	.set	reorder

	lui	$2,%hi($LC22)
	lw	$3,12($3)
	.set	noreorder
	.set	nomacro
	j	$L139
	addiu	$6,$2,%lo($LC22)
	.set	macro
	.set	reorder

$L141:
#APP
 # 75 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/string.h" 1
	.set	noreorder
	.set	noat
	lbu	$7,($4)
1:	lbu	$1,($5)
	addiu	$4,1
	bne	$1,$7,2f
	addiu	$5,1
	bnez	$7,1b
	lbu	$7,($4)
	move	$7,$1
2:	subu	$7,$1
3:	.set	at
	.set	reorder
 # 0 "" 2
#NO_APP
	beq	$7,$0,$L140
	lw	$3,0($3)
$L139:
	addiu	$5,$3,35
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L141
	move	$4,$6
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L135
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

$L140:
	lw	$4,16($16)
	lw	$2,16($3)
	.set	noreorder
	.set	nomacro
	j	$L154
	xor	$2,$4,$2
	.set	macro
	.set	reorder

$L136:
	lui	$4,%hi($LC23)
	.set	noreorder
	.set	nomacro
	jal	rtl_get_dev_by_name
	addiu	$4,$4,%lo($LC23)
	.set	macro
	.set	reorder

	beq	$2,$0,$L142
	lw	$2,292($2)
	sw	$2,16($sp)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L138
	move	$3,$2
	.set	macro
	.set	reorder

	lui	$2,%hi($LC23)
	lw	$3,12($3)
	.set	noreorder
	.set	nomacro
	j	$L144
	addiu	$6,$2,%lo($LC23)
	.set	macro
	.set	reorder

$L146:
#APP
 # 75 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/string.h" 1
	.set	noreorder
	.set	noat
	lbu	$7,($4)
1:	lbu	$1,($5)
	addiu	$4,1
	bne	$1,$7,2f
	addiu	$5,1
	bnez	$7,1b
	lbu	$7,($4)
	move	$7,$1
2:	subu	$7,$1
3:	.set	at
	.set	reorder
 # 0 "" 2
#NO_APP
	beq	$7,$0,$L149
	lw	$3,0($3)
$L144:
	addiu	$5,$3,35
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L146
	move	$4,$6
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L135
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

$L142:
	lui	$4,%hi($LC24)
	.set	noreorder
	.set	nomacro
	jal	rtl_get_dev_by_name
	addiu	$4,$4,%lo($LC24)
	.set	macro
	.set	reorder

	beq	$2,$0,$L138
	lw	$2,292($2)
	sw	$2,16($sp)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L138
	move	$3,$2
	.set	macro
	.set	reorder

	lui	$2,%hi($LC24)
	lw	$3,12($3)
	.set	noreorder
	.set	nomacro
	j	$L148
	addiu	$6,$2,%lo($LC24)
	.set	macro
	.set	reorder

$L150:
#APP
 # 75 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/string.h" 1
	.set	noreorder
	.set	noat
	lbu	$7,($4)
1:	lbu	$1,($5)
	addiu	$4,1
	bne	$1,$7,2f
	addiu	$5,1
	bnez	$7,1b
	lbu	$7,($4)
	move	$7,$1
2:	subu	$7,$1
3:	.set	at
	.set	reorder
 # 0 "" 2
#NO_APP
	beq	$7,$0,$L149
	lw	$3,0($3)
$L148:
	addiu	$5,$3,35
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L150
	move	$4,$6
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L135
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

$L149:
	lw	$4,16($16)
	lw	$2,20($3)
	xor	$2,$4,$2
$L154:
	.set	noreorder
	.set	nomacro
	j	$L135
	sltu	$2,$0,$2
	.set	macro
	.set	reorder

$L138:
	li	$2,1			# 0x1
$L135:
	lw	$31,28($sp)
	lw	$16,24($sp)
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,32
	.set	macro
	.set	reorder

	.end	need_filter
	.size	need_filter, .-need_filter
	.section	.rodata.str1.4
	.align	2
$LC25:
	.ascii	"dos_filter: alloc skb fail!\012\000"
	.section	.text.GenerateTCPENDACK,"ax",@progbits
	.align	2
	.set	nomips16
	.ent	GenerateTCPENDACK
	.type	GenerateTCPENDACK, @function
GenerateTCPENDACK:
	.frame	$sp,112,$31		# vars= 56, regs= 10/0, args= 16, gp= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-112
	sw	$31,108($sp)
	sw	$fp,104($sp)
	sw	$23,100($sp)
	sw	$22,96($sp)
	sw	$21,92($sp)
	sw	$20,88($sp)
	sw	$19,84($sp)
	sw	$18,80($sp)
	sw	$17,76($sp)
	sw	$16,72($sp)
	move	$19,$4
	lui	$4,%hi($LC21)
	.set	noreorder
	.set	nomacro
	jal	rtl_get_dev_by_name
	addiu	$4,$4,%lo($LC21)
	.set	macro
	.set	reorder

	move	$4,$19
	.set	noreorder
	.set	nomacro
	jal	rtl_ip_hdr
	sw	$2,68($sp)
	.set	macro
	.set	reorder

	lbu	$18,0($2)
	move	$17,$2
	lui	$2,%hi(endStateClientPort)
	lhu	$3,%lo(endStateClientPort)($2)
	andi	$18,$18,0xf
	lw	$5,68($sp)
	sll	$18,$18,2
	addu	$18,$17,$18
	lhu	$2,0($18)
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L156
	lui	$2,%hi(endStateClientSeq)
	.set	macro
	.set	reorder

	lw	$3,%lo(endStateClientSeq)($2)
	lw	$2,4($18)
	bne	$3,$2,$L156
	.set	noreorder
	.set	nomacro
	jal	rtl_neigh_lookup
	addiu	$4,$17,12
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L156
	move	$16,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_hh_from_neigh
	move	$4,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L171
	li	$5,32			# 0x20
	.set	macro
	.set	reorder

	j	$L175
$L176:
	.set	noreorder
	.set	nomacro
	jal	neigh_destroy
	move	$4,$16
	.set	macro
	.set	reorder

	j	$L156
$L171:
	move	$4,$19
	move	$6,$0
	.set	noreorder
	.set	nomacro
	jal	skb_copy_expand
	li	$7,32			# 0x20
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L159
	move	$20,$2
	.set	macro
	.set	reorder

#APP
 # 167 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
 # 169 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	1:	ll	$3, 72($16)		# atomic_sub_return	
       nop                                             
	subu	$2, $3, 1				
	sc	$2, 72($16)					
	beqz	$2, 2f					
	subu	$2, $3, 1				
	.subsection 2					
2:	b	1b					
	.previous					

 # 0 "" 2
 # 185 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
#NO_APP
	bne	$2,$0,$L160
	.set	noreorder
	.set	nomacro
	jal	neigh_destroy
	move	$4,$16
	.set	macro
	.set	reorder

$L160:
	lui	$4,%hi($LC25)
	.set	noreorder
	.set	nomacro
	jal	scrlog_printk
	addiu	$4,$4,%lo($LC25)
	.set	macro
	.set	reorder

	j	$L156
$L159:
	.set	noreorder
	.set	nomacro
	jal	rtl_ip_hdr
	move	$4,$2
	.set	macro
	.set	reorder

	move	$4,$20
	lbu	$5,0($2)
	andi	$5,$5,0xf
	addiu	$5,$5,5
	.set	noreorder
	.set	nomacro
	jal	skb_trim
	sll	$5,$5,2
	.set	macro
	.set	reorder

	move	$4,$20
	.set	noreorder
	.set	nomacro
	jal	skb_put
	move	$5,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_hh_from_neigh
	move	$4,$16
	.set	macro
	.set	reorder

	addiu	$4,$sp,64
	move	$5,$2
	.set	noreorder
	.set	nomacro
	jal	rtl_get_lock_from_hh
	move	$21,$2
	.set	macro
	.set	reorder

	jal	local_bh_disable
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	move	$4,$20
	.set	macro
	.set	reorder

	move	$4,$21
	.set	noreorder
	.set	nomacro
	jal	rtl_get_data_from_hh
	move	$22,$2
	.set	macro
	.set	reorder

	li	$6,16			# 0x10
	addiu	$4,$22,-16
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$5,$2
	.set	macro
	.set	reorder

	jal	local_bh_enable
	.set	noreorder
	.set	nomacro
	jal	rtl_get_len_from_hh
	move	$4,$21
	.set	macro
	.set	reorder

	move	$4,$20
	.set	noreorder
	.set	nomacro
	jal	skb_push
	move	$5,$2
	.set	macro
	.set	reorder

#APP
 # 167 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
 # 169 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	1:	ll	$3, 72($16)		# atomic_sub_return	
       nop                                             
	subu	$2, $3, 1				
	sc	$2, 72($16)					
	beqz	$2, 2f					
	subu	$2, $3, 1				
	.subsection 2					
2:	b	1b					
	.previous					

 # 0 "" 2
 # 185 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
#NO_APP
	bne	$2,$0,$L161
	.set	noreorder
	.set	nomacro
	jal	neigh_destroy
	move	$4,$16
	.set	macro
	.set	reorder

$L161:
	move	$4,$20
	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_nfct
	addiu	$21,$sp,44
	.set	macro
	.set	reorder

	move	$4,$20
	li	$16,20			# 0x14
	lhu	$2,2($18)
	lw	$5,56($sp)
	sh	$2,44($sp)
	li	$2,268369920			# 0xfff0000
	lhu	$3,0($18)
	ori	$2,$2,0xffff
	and	$2,$5,$2
	li	$5,1342177280			# 0x50000000
	sh	$3,46($sp)
	or	$2,$2,$5
	li	$3,-327680			# 0xfffb0000
	lw	$5,4($18)
	ori	$3,$3,0xffff
	lw	$6,8($18)
	sw	$2,56($sp)
	addiu	$5,$5,1
	sb	$0,13($21)
	lui	$2,%hi(url_cache+12)
	lw	$7,56($sp)
	sw	$6,48($sp)
	li	$6,1048576			# 0x100000
	and	$3,$7,$3
	sw	$5,52($sp)
	li	$5,-131072			# 0xfffe0000
	lhu	$2,%lo(url_cache+12)($2)
	or	$3,$3,$6
	ori	$5,$5,0xffff
	sh	$0,62($sp)
	and	$3,$3,$5
	sw	$3,56($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_len
	sh	$2,58($sp)
	.set	macro
	.set	reorder

	li	$4,8			# 0x8
	lw	$3,12($18)
	addiu	$22,$2,-34
	srl	$3,$3,28
	.set	noreorder
	.set	nomacro
	j	$L172
	sll	$3,$3,2
	.set	macro
	.set	reorder

$L166:
	lbu	$5,0($7)
	sltu	$7,$5,2
	beq	$7,$0,$L163
	.set	noreorder
	.set	nomacro
	j	$L172
	move	$16,$2
	.set	macro
	.set	reorder

$L163:
	lbu	$2,0($6)
	.set	noreorder
	.set	nomacro
	bne	$5,$4,$L172
	addu	$16,$16,$2
	.set	macro
	.set	reorder

	subu	$16,$16,$2
	addu	$fp,$21,$fp
	addiu	$18,$sp,16
	addiu	$16,$16,6
	move	$4,$18
	addiu	$23,$sp,20
	move	$5,$fp
	addu	$21,$21,$16
	.set	noreorder
	.set	nomacro
	jal	memcpy
	li	$6,4			# 0x4
	.set	macro
	.set	reorder

	move	$4,$23
	li	$6,4			# 0x4
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$5,$21
	.set	macro
	.set	reorder

	move	$4,$fp
	li	$6,4			# 0x4
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$5,$23
	.set	macro
	.set	reorder

	move	$4,$21
	li	$6,4			# 0x4
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$5,$18
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L177
	addiu	$18,$sp,44
	.set	macro
	.set	reorder

$L172:
	addiu	$2,$16,1
	sltu	$5,$16,$3
	addu	$7,$21,$16
	addu	$6,$21,$2
	.set	noreorder
	.set	nomacro
	bne	$5,$0,$L166
	addiu	$fp,$16,2
	.set	macro
	.set	reorder

	addiu	$18,$sp,44
$L177:
	move	$5,$22
	move	$6,$0
	sh	$0,60($sp)
	move	$4,$18
	lui	$16,%hi(url_cache)
	lw	$23,16($17)
	andi	$22,$22,0xffff
	lw	$21,12($17)
	.set	noreorder
	.set	nomacro
	jal	csum_partial
	addiu	$22,$22,6
	.set	macro
	.set	reorder

	move	$4,$20
#APP
 # 141 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/checksum.h" 1
		.set	push		# csum_tcpudp_nofold
	.set	noat		
	addu	$23, $21		
	sltu	$1, $23, $21	
	addu	$23, $1		
	addu	$23, $22		
	sltu	$1, $23, $22	
	addu	$23, $1		
	addu	$23, $2		
	sltu	$1, $23, $2	
	addu	$23, $1		
	.set	pop
 # 0 "" 2
 # 83 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/checksum.h" 1
		.set	push		# csum_fold
	.set	noat		
	sll	$1, $23, 16	
	addu	$23, $1		
	sltu	$1, $23, $1	
	srl	$23, $23, 16	
	addu	$23, $1		
	xori	$23, 0xffff	
	.set	pop
 # 0 "" 2
#NO_APP
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	sh	$23,60($sp)
	.set	macro
	.set	reorder

	move	$5,$18
	li	$6,20			# 0x14
	.set	noreorder
	.set	nomacro
	jal	memcpy
	addiu	$4,$2,34
	.set	macro
	.set	reorder

	li	$2,268369920			# 0xfff0000
	move	$4,$19
	lw	$3,24($sp)
	ori	$2,$2,0xffff
	li	$5,1073741824			# 0x40000000
	li	$6,-251723776			# 0xf0ff0000
	and	$2,$3,$2
	addiu	$3,$16,%lo(url_cache)
	or	$2,$2,$5
	li	$5,6			# 0x6
	ori	$6,$6,0xffff
	sb	$5,33($sp)
	move	$5,$0
	and	$2,$2,$6
	li	$6,83886080			# 0x5000000
	or	$2,$2,$6
	lbu	$3,14($3)
	sh	$0,30($sp)
	sw	$2,24($sp)
	sb	$0,25($sp)
	lw	$2,12($17)
	sw	$2,40($sp)
	lw	$2,16($17)
	sb	$3,32($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_ip_summed
	sw	$2,36($sp)
	.set	macro
	.set	reorder

	li	$3,40			# 0x28
	lhu	$2,%lo(url_cache)($16)
	addiu	$4,$sp,24
	sh	$3,26($sp)
	addiu	$3,$sp,40
	addiu	$2,$2,1
	sh	$2,28($sp)
	sh	$0,34($sp)
	lw	$7,4($4)
	lw	$6,24($sp)
	lw	$5,8($4)
	srl	$8,$6,22
	addu	$6,$7,$6
	addu	$2,$6,$5
	sltu	$7,$6,$7
	lw	$6,36($sp)
	addu	$7,$2,$7
	andi	$8,$8,0x3c
	sltu	$5,$7,$5
	addu	$4,$4,$8
	addu	$7,$7,$6
	addu	$5,$7,$5
	sltu	$2,$5,$6
	addu	$2,$2,$5
$L167:
	lw	$5,0($3)
	addiu	$3,$3,4
	addu	$2,$2,$5
	sltu	$5,$2,$5
	.set	noreorder
	.set	nomacro
	bne	$3,$4,$L167
	addu	$2,$5,$2
	.set	macro
	.set	reorder

	move	$4,$20
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
	sh	$2,34($sp)
	.set	macro
	.set	reorder

	addiu	$5,$sp,24
	addiu	$4,$2,14
	.set	noreorder
	.set	nomacro
	jal	memcpy
	li	$6,20			# 0x14
	.set	macro
	.set	reorder

	lui	$2,%hi(url_cache+16)
	move	$4,$20
	.set	noreorder
	.set	nomacro
	jal	dev_queue_xmit
	sb	$0,%lo(url_cache+16)($2)
	.set	macro
	.set	reorder

$L156:
	lw	$31,108($sp)
	move	$2,$0
	lw	$fp,104($sp)
	lw	$23,100($sp)
	lw	$22,96($sp)
	lw	$21,92($sp)
	lw	$20,88($sp)
	lw	$19,84($sp)
	lw	$18,80($sp)
	lw	$17,76($sp)
	lw	$16,72($sp)
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,112
	.set	macro
	.set	reorder

$L175:
#APP
 # 167 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
 # 169 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	1:	ll	$3, 72($16)		# atomic_sub_return	
       nop                                             
	subu	$2, $3, 1				
	sc	$2, 72($16)					
	beqz	$2, 2f					
	subu	$2, $3, 1				
	.subsection 2					
2:	b	1b					
	.previous					

 # 0 "" 2
 # 185 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
#NO_APP
	bne	$2,$0,$L156
	j	$L176
	.end	GenerateTCPENDACK
	.size	GenerateTCPENDACK, .-GenerateTCPENDACK
	.section	.text.GenerateTCPFINACK,"ax",@progbits
	.align	2
	.set	nomips16
	.ent	GenerateTCPFINACK
	.type	GenerateTCPFINACK, @function
GenerateTCPFINACK:
	.frame	$sp,112,$31		# vars= 56, regs= 10/0, args= 16, gp= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-112
	sw	$31,108($sp)
	sw	$fp,104($sp)
	sw	$23,100($sp)
	sw	$22,96($sp)
	sw	$21,92($sp)
	sw	$20,88($sp)
	sw	$19,84($sp)
	sw	$18,80($sp)
	sw	$17,76($sp)
	sw	$16,72($sp)
	move	$19,$4
	lui	$4,%hi($LC21)
	.set	noreorder
	.set	nomacro
	jal	rtl_get_dev_by_name
	addiu	$4,$4,%lo($LC21)
	.set	macro
	.set	reorder

	move	$4,$19
	.set	noreorder
	.set	nomacro
	jal	rtl_ip_hdr
	sw	$2,68($sp)
	.set	macro
	.set	reorder

	lbu	$18,0($2)
	move	$17,$2
	lui	$2,%hi(finStateClientPort)
	lhu	$3,%lo(finStateClientPort)($2)
	andi	$18,$18,0xf
	lw	$5,68($sp)
	sll	$18,$18,2
	addu	$18,$17,$18
	lhu	$2,0($18)
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L179
	lui	$2,%hi(finStateClientSeq)
	.set	macro
	.set	reorder

	lw	$3,%lo(finStateClientSeq)($2)
	lw	$2,4($18)
	bne	$3,$2,$L179
	.set	noreorder
	.set	nomacro
	jal	rtl_neigh_lookup
	addiu	$4,$17,12
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L179
	move	$16,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_hh_from_neigh
	move	$4,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L194
	li	$5,32			# 0x20
	.set	macro
	.set	reorder

	j	$L198
$L199:
	.set	noreorder
	.set	nomacro
	jal	neigh_destroy
	move	$4,$16
	.set	macro
	.set	reorder

	j	$L179
$L194:
	move	$4,$19
	move	$6,$0
	.set	noreorder
	.set	nomacro
	jal	skb_copy_expand
	li	$7,32			# 0x20
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L182
	move	$20,$2
	.set	macro
	.set	reorder

#APP
 # 167 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
 # 169 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	1:	ll	$3, 72($16)		# atomic_sub_return	
       nop                                             
	subu	$2, $3, 1				
	sc	$2, 72($16)					
	beqz	$2, 2f					
	subu	$2, $3, 1				
	.subsection 2					
2:	b	1b					
	.previous					

 # 0 "" 2
 # 185 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
#NO_APP
	bne	$2,$0,$L183
	.set	noreorder
	.set	nomacro
	jal	neigh_destroy
	move	$4,$16
	.set	macro
	.set	reorder

$L183:
	lui	$4,%hi($LC25)
	.set	noreorder
	.set	nomacro
	jal	scrlog_printk
	addiu	$4,$4,%lo($LC25)
	.set	macro
	.set	reorder

	j	$L179
$L182:
	.set	noreorder
	.set	nomacro
	jal	rtl_ip_hdr
	move	$4,$2
	.set	macro
	.set	reorder

	move	$4,$20
	lbu	$5,0($2)
	andi	$5,$5,0xf
	addiu	$5,$5,5
	.set	noreorder
	.set	nomacro
	jal	skb_trim
	sll	$5,$5,2
	.set	macro
	.set	reorder

	move	$4,$20
	.set	noreorder
	.set	nomacro
	jal	skb_put
	move	$5,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_hh_from_neigh
	move	$4,$16
	.set	macro
	.set	reorder

	addiu	$4,$sp,64
	move	$5,$2
	.set	noreorder
	.set	nomacro
	jal	rtl_get_lock_from_hh
	move	$21,$2
	.set	macro
	.set	reorder

	jal	local_bh_disable
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	move	$4,$20
	.set	macro
	.set	reorder

	move	$4,$21
	.set	noreorder
	.set	nomacro
	jal	rtl_get_data_from_hh
	move	$22,$2
	.set	macro
	.set	reorder

	li	$6,16			# 0x10
	addiu	$4,$22,-16
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$5,$2
	.set	macro
	.set	reorder

	jal	local_bh_enable
	.set	noreorder
	.set	nomacro
	jal	rtl_get_len_from_hh
	move	$4,$21
	.set	macro
	.set	reorder

	move	$4,$20
	.set	noreorder
	.set	nomacro
	jal	skb_push
	move	$5,$2
	.set	macro
	.set	reorder

#APP
 # 167 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
 # 169 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	1:	ll	$3, 72($16)		# atomic_sub_return	
       nop                                             
	subu	$2, $3, 1				
	sc	$2, 72($16)					
	beqz	$2, 2f					
	subu	$2, $3, 1				
	.subsection 2					
2:	b	1b					
	.previous					

 # 0 "" 2
 # 185 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
#NO_APP
	bne	$2,$0,$L184
	.set	noreorder
	.set	nomacro
	jal	neigh_destroy
	move	$4,$16
	.set	macro
	.set	reorder

$L184:
	move	$4,$20
	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_nfct
	addiu	$21,$sp,44
	.set	macro
	.set	reorder

	li	$6,268369920			# 0xfff0000
	move	$4,$20
	lhu	$2,2($18)
	ori	$6,$6,0xffff
	li	$16,20			# 0x14
	sh	$2,44($sp)
	lhu	$3,0($18)
	lw	$2,56($sp)
	sh	$3,46($sp)
	and	$6,$2,$6
	li	$2,1342177280			# 0x50000000
	lw	$3,8($18)
	or	$6,$6,$2
	lui	$2,%hi(url_cache+12)
	lw	$5,4($18)
	lhu	$2,%lo(url_cache+12)($2)
	sw	$6,56($sp)
	sb	$0,13($21)
	lw	$6,56($sp)
	sw	$3,48($sp)
	li	$3,-327680			# 0xfffb0000
	ori	$3,$3,0xffff
	sw	$5,52($sp)
	li	$5,1048576			# 0x100000
	and	$3,$6,$3
	sh	$0,62($sp)
	or	$3,$3,$5
	li	$5,65536			# 0x10000
	or	$3,$3,$5
	sw	$3,56($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_len
	sh	$2,58($sp)
	.set	macro
	.set	reorder

	li	$4,8			# 0x8
	lw	$3,12($18)
	addiu	$22,$2,-34
	srl	$3,$3,28
	.set	noreorder
	.set	nomacro
	j	$L195
	sll	$3,$3,2
	.set	macro
	.set	reorder

$L189:
	lbu	$5,0($7)
	sltu	$7,$5,2
	beq	$7,$0,$L186
	.set	noreorder
	.set	nomacro
	j	$L195
	move	$16,$2
	.set	macro
	.set	reorder

$L186:
	lbu	$2,0($6)
	.set	noreorder
	.set	nomacro
	bne	$5,$4,$L195
	addu	$16,$16,$2
	.set	macro
	.set	reorder

	subu	$16,$16,$2
	addu	$fp,$21,$fp
	addiu	$18,$sp,16
	addiu	$16,$16,6
	move	$4,$18
	addiu	$23,$sp,20
	move	$5,$fp
	addu	$21,$21,$16
	.set	noreorder
	.set	nomacro
	jal	memcpy
	li	$6,4			# 0x4
	.set	macro
	.set	reorder

	move	$4,$23
	li	$6,4			# 0x4
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$5,$21
	.set	macro
	.set	reorder

	move	$4,$fp
	li	$6,4			# 0x4
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$5,$23
	.set	macro
	.set	reorder

	move	$4,$21
	li	$6,4			# 0x4
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$5,$18
	.set	macro
	.set	reorder

	j	$L188
$L195:
	addiu	$2,$16,1
	sltu	$5,$16,$3
	addu	$7,$21,$16
	addu	$6,$21,$2
	.set	noreorder
	.set	nomacro
	bne	$5,$0,$L189
	addiu	$fp,$16,2
	.set	macro
	.set	reorder

$L188:
	lhu	$3,46($sp)
	lui	$2,%hi(endStateClientPort)
	move	$5,$22
	addiu	$18,$sp,44
	move	$6,$0
	andi	$22,$22,0xffff
	sh	$3,%lo(endStateClientPort)($2)
	lui	$2,%hi(endStateClientSeq)
	move	$4,$18
	lw	$3,52($sp)
	addiu	$22,$22,6
	sh	$0,60($sp)
	sw	$3,%lo(endStateClientSeq)($2)
	lw	$23,16($17)
	lw	$21,12($17)
	.set	noreorder
	.set	nomacro
	jal	csum_partial
	lui	$16,%hi(url_cache)
	.set	macro
	.set	reorder

	move	$4,$20
#APP
 # 141 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/checksum.h" 1
		.set	push		# csum_tcpudp_nofold
	.set	noat		
	addu	$23, $21		
	sltu	$1, $23, $21	
	addu	$23, $1		
	addu	$23, $22		
	sltu	$1, $23, $22	
	addu	$23, $1		
	addu	$23, $2		
	sltu	$1, $23, $2	
	addu	$23, $1		
	.set	pop
 # 0 "" 2
 # 83 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/checksum.h" 1
		.set	push		# csum_fold
	.set	noat		
	sll	$1, $23, 16	
	addu	$23, $1		
	sltu	$1, $23, $1	
	srl	$23, $23, 16	
	addu	$23, $1		
	xori	$23, 0xffff	
	.set	pop
 # 0 "" 2
#NO_APP
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	sh	$23,60($sp)
	.set	macro
	.set	reorder

	move	$5,$18
	li	$6,20			# 0x14
	.set	noreorder
	.set	nomacro
	jal	memcpy
	addiu	$4,$2,34
	.set	macro
	.set	reorder

	li	$2,268369920			# 0xfff0000
	move	$4,$19
	lw	$3,24($sp)
	ori	$2,$2,0xffff
	li	$5,1073741824			# 0x40000000
	li	$6,-251723776			# 0xf0ff0000
	and	$2,$3,$2
	addiu	$3,$16,%lo(url_cache)
	or	$2,$2,$5
	li	$5,6			# 0x6
	ori	$6,$6,0xffff
	sb	$5,33($sp)
	move	$5,$0
	and	$2,$2,$6
	li	$6,83886080			# 0x5000000
	or	$2,$2,$6
	lbu	$3,14($3)
	sh	$0,30($sp)
	sw	$2,24($sp)
	sb	$0,25($sp)
	lw	$2,12($17)
	sw	$2,40($sp)
	lw	$2,16($17)
	sb	$3,32($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_ip_summed
	sw	$2,36($sp)
	.set	macro
	.set	reorder

	li	$3,40			# 0x28
	lhu	$2,%lo(url_cache)($16)
	addiu	$4,$sp,24
	sh	$3,26($sp)
	addiu	$3,$sp,40
	addiu	$2,$2,1
	sh	$2,28($sp)
	sh	$0,34($sp)
	lw	$7,4($4)
	lw	$6,24($sp)
	lw	$5,8($4)
	srl	$8,$6,22
	addu	$6,$7,$6
	addu	$2,$6,$5
	sltu	$7,$6,$7
	lw	$6,36($sp)
	addu	$7,$2,$7
	andi	$8,$8,0x3c
	sltu	$5,$7,$5
	addu	$4,$4,$8
	addu	$7,$7,$6
	addu	$5,$7,$5
	sltu	$2,$5,$6
	addu	$2,$2,$5
$L190:
	lw	$5,0($3)
	addiu	$3,$3,4
	addu	$2,$2,$5
	sltu	$5,$2,$5
	.set	noreorder
	.set	nomacro
	bne	$3,$4,$L190
	addu	$2,$5,$2
	.set	macro
	.set	reorder

	move	$4,$20
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
	sh	$2,34($sp)
	.set	macro
	.set	reorder

	addiu	$5,$sp,24
	addiu	$4,$2,14
	.set	noreorder
	.set	nomacro
	jal	memcpy
	li	$6,20			# 0x14
	.set	macro
	.set	reorder

	lui	$2,%hi(url_cache)
	move	$4,$20
	addiu	$2,$2,%lo(url_cache)
	li	$3,1			# 0x1
	sb	$3,16($2)
	.set	noreorder
	.set	nomacro
	jal	dev_queue_xmit
	sb	$0,15($2)
	.set	macro
	.set	reorder

$L179:
	lw	$31,108($sp)
	move	$2,$0
	lw	$fp,104($sp)
	lw	$23,100($sp)
	lw	$22,96($sp)
	lw	$21,92($sp)
	lw	$20,88($sp)
	lw	$19,84($sp)
	lw	$18,80($sp)
	lw	$17,76($sp)
	lw	$16,72($sp)
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,112
	.set	macro
	.set	reorder

$L198:
#APP
 # 167 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
 # 169 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	1:	ll	$3, 72($16)		# atomic_sub_return	
       nop                                             
	subu	$2, $3, 1				
	sc	$2, 72($16)					
	beqz	$2, 2f					
	subu	$2, $3, 1				
	.subsection 2					
2:	b	1b					
	.previous					

 # 0 "" 2
 # 185 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
#NO_APP
	bne	$2,$0,$L179
	j	$L199
	.end	GenerateTCPFINACK
	.size	GenerateTCPFINACK, .-GenerateTCPFINACK
	.section	.text.unregister_customRspStr,"ax",@progbits
	.align	2
	.globl	unregister_customRspStr
	.set	nomips16
	.ent	unregister_customRspStr
	.type	unregister_customRspStr, @function
unregister_customRspStr:
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$16,16($sp)
	lui	$16,%hi(block_message)
	lw	$4,%lo(block_message)($16)
	beq	$4,$0,$L202
	nop

	jal	kfree
	nop

	sw	$0,%lo(block_message)($16)
$L202:
	lw	$31,20($sp)
	lw	$16,16($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	unregister_customRspStr
	.size	unregister_customRspStr, .-unregister_customRspStr
	.section	.text.filter_table_flush,"ax",@progbits
	.align	2
	.globl	filter_table_flush
	.set	nomips16
	.ent	filter_table_flush
	.type	filter_table_flush, @function
filter_table_flush:
	.frame	$sp,48,$31		# vars= 0, regs= 8/0, args= 16, gp= 0
	.mask	0x807f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-48
	sw	$31,44($sp)
	sw	$22,40($sp)
	sw	$21,36($sp)
	sw	$20,32($sp)
	sw	$19,28($sp)
	sw	$18,24($sp)
	sw	$17,20($sp)
	sw	$16,16($sp)
	lui	$18,%hi(table_list_head)
	lw	$16,%lo(table_list_head)($18)
	li	$21,1048576			# 0x100000
	li	$20,2097152			# 0x200000
	ori	$21,$21,0x100
	addiu	$18,$18,%lo(table_list_head)
	lw	$17,0($16)
	j	$L204
	ori	$20,$20,0x200

$L207:
	lw	$2,8($16)
	lw	$19,0($2)
	j	$L205
	addiu	$22,$16,8

$L206:
	lw	$3,4($2)
	lw	$5,0($2)
	sw	$3,4($5)
	sw	$5,0($3)
	sw	$21,0($2)
	jal	kfree
	sw	$20,4($2)

	move	$2,$19
	lw	$3,24($16)
	addiu	$3,$3,-1
	sw	$3,24($16)
	lw	$19,0($19)
$L205:
	bne	$2,$22,$L206
	move	$4,$2

	lw	$2,4($16)
	move	$4,$16
	lw	$3,0($16)
	sw	$2,4($3)
	sw	$3,0($2)
	sw	$21,0($16)
	sw	$20,4($16)
	jal	kfree
	move	$16,$17

	lw	$2,24($18)
	addiu	$2,$2,-1
	sw	$2,24($18)
	lw	$17,0($17)
$L204:
	bne	$16,$18,$L207
	li	$3,1			# 0x1

	lui	$2,%hi(filter_rule_num)
	sw	$3,%lo(filter_rule_num)($2)
	lui	$3,%hi(log_enable)
	lw	$31,44($sp)
	move	$2,$0
	lw	$22,40($sp)
	lw	$21,36($sp)
	lw	$20,32($sp)
	lw	$19,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	sw	$0,%lo(log_enable)($3)
	lui	$3,%hi(url_filter_mode)
	sw	$0,%lo(url_filter_mode)($3)
	j	$31
	addiu	$sp,$sp,48

	.set	macro
	.set	reorder
	.end	filter_table_flush
	.size	filter_table_flush, .-filter_table_flush
	.section	.rodata.str1.4
	.align	2
$LC26:
	.ascii	"HTTP/1.1 401 The web site is blocked by administrator\015"
	.ascii	"\012Server: Embedded HTTP Server 2.00\015\012Connection:"
	.ascii	" close\015\012\015\012<HTML><HEAD><TITLE>401 The web sit"
	.ascii	"e is blocked by administrator</TITLE></HEAD>\012<BODY BG"
	.ascii	"COLOR=\"#ffffff\"><H4>401 The web site is blocked by adm"
	.ascii	"inistrator</H4></BODY></HTML>\012\000"
	.section	.text.GenerateHTTP401,"ax",@progbits
	.align	2
	.globl	GenerateHTTP401
	.set	nomips16
	.ent	GenerateHTTP401
	.type	GenerateHTTP401, @function
GenerateHTTP401:
	.frame	$sp,176,$31		# vars= 120, regs= 9/0, args= 16, gp= 0
	.mask	0x80ff0000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-176
	sw	$31,172($sp)
	sw	$23,168($sp)
	sw	$22,164($sp)
	sw	$21,160($sp)
	sw	$20,156($sp)
	sw	$19,152($sp)
	sw	$18,148($sp)
	sw	$17,144($sp)
	sw	$16,140($sp)
	move	$17,$4
	lui	$4,%hi($LC21)
	.set	noreorder
	.set	nomacro
	jal	rtl_get_dev_by_name
	addiu	$4,$4,%lo($LC21)
	.set	macro
	.set	reorder

	move	$4,$17
	.set	noreorder
	.set	nomacro
	jal	rtl_ip_hdr
	sw	$2,132($sp)
	.set	macro
	.set	reorder

	lw	$5,132($sp)
	.set	noreorder
	.set	nomacro
	jal	rtl_neigh_lookup
	addiu	$4,$2,12
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L210
	move	$16,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_hh_from_neigh
	move	$4,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L231
	lui	$2,%hi(block_message)
	.set	macro
	.set	reorder

	j	$L238
$L239:
	.set	noreorder
	.set	nomacro
	jal	neigh_destroy
	move	$4,$16
	.set	macro
	.set	reorder

	j	$L210
$L231:
	lw	$4,%lo(block_message)($2)
	bne	$4,$0,$L233
	lui	$4,%hi($LC26)
	addiu	$4,$4,%lo($LC26)
$L233:
	jal	strlen
	move	$4,$17
	li	$5,32			# 0x20
	move	$6,$2
	.set	noreorder
	.set	nomacro
	jal	skb_copy_expand
	li	$7,32			# 0x20
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L215
	move	$19,$2
	.set	macro
	.set	reorder

#APP
 # 167 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
 # 169 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	1:	ll	$3, 72($16)		# atomic_sub_return	
       nop                                             
	subu	$2, $3, 1				
	sc	$2, 72($16)					
	beqz	$2, 2f					
	subu	$2, $3, 1				
	.subsection 2					
2:	b	1b					
	.previous					

 # 0 "" 2
 # 185 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
#NO_APP
	bne	$2,$0,$L216
	.set	noreorder
	.set	nomacro
	jal	neigh_destroy
	move	$4,$16
	.set	macro
	.set	reorder

$L216:
	lui	$4,%hi($LC25)
	.set	noreorder
	.set	nomacro
	jal	scrlog_printk
	addiu	$4,$4,%lo($LC25)
	.set	macro
	.set	reorder

	j	$L210
$L215:
	.set	noreorder
	.set	nomacro
	jal	rtl_ip_hdr
	move	$4,$2
	.set	macro
	.set	reorder

	move	$4,$19
	move	$18,$2
	lbu	$2,0($2)
	andi	$2,$2,0xf
	sll	$20,$2,2
	addu	$20,$18,$20
	lw	$3,12($20)
	srl	$3,$3,28
	addu	$2,$3,$2
	.set	noreorder
	.set	nomacro
	jal	skb_trim
	sll	$5,$2,2
	.set	macro
	.set	reorder

	lui	$2,%hi(block_message)
	lw	$4,%lo(block_message)($2)
	bne	$4,$0,$L234
	lui	$4,%hi($LC26)
	addiu	$4,$4,%lo($LC26)
$L234:
	jal	strlen
	move	$4,$19
	.set	noreorder
	.set	nomacro
	jal	skb_put
	move	$5,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_get_hh_from_neigh
	move	$4,$16
	.set	macro
	.set	reorder

	addiu	$4,$sp,128
	move	$5,$2
	.set	noreorder
	.set	nomacro
	jal	rtl_get_lock_from_hh
	move	$21,$2
	.set	macro
	.set	reorder

	jal	local_bh_disable
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	move	$4,$19
	.set	macro
	.set	reorder

	move	$4,$21
	.set	noreorder
	.set	nomacro
	jal	rtl_get_data_from_hh
	move	$22,$2
	.set	macro
	.set	reorder

	li	$6,16			# 0x10
	addiu	$4,$22,-16
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$5,$2
	.set	macro
	.set	reorder

	jal	local_bh_enable
	.set	noreorder
	.set	nomacro
	jal	rtl_get_len_from_hh
	move	$4,$21
	.set	macro
	.set	reorder

	move	$4,$19
	.set	noreorder
	.set	nomacro
	jal	skb_push
	move	$5,$2
	.set	macro
	.set	reorder

#APP
 # 167 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
 # 169 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	1:	ll	$3, 72($16)		# atomic_sub_return	
       nop                                             
	subu	$2, $3, 1				
	sc	$2, 72($16)					
	beqz	$2, 2f					
	subu	$2, $3, 1				
	.subsection 2					
2:	b	1b					
	.previous					

 # 0 "" 2
 # 185 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
#NO_APP
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L241
	move	$5,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	neigh_destroy
	move	$4,$16
	.set	macro
	.set	reorder

	move	$5,$0
$L241:
	.set	noreorder
	.set	nomacro
	jal	rtl_set_skb_nfct
	move	$4,$19
	.set	macro
	.set	reorder

	lbu	$16,0($18)
	lw	$3,12($18)
	lw	$2,16($18)
	andi	$16,$16,0xf
	sll	$16,$16,2
	sw	$3,16($18)
	sw	$2,12($18)
	addu	$16,$18,$16
	lhu	$2,0($16)
	lhu	$3,2($16)
	lw	$22,12($16)
	sh	$2,2($16)
	lui	$2,%hi(block_message)
	sh	$3,0($16)
	srl	$22,$22,28
	lw	$5,%lo(block_message)($2)
	sll	$22,$22,2
	.set	noreorder
	.set	nomacro
	beq	$5,$0,$L220
	addu	$22,$16,$22
	.set	macro
	.set	reorder

	move	$4,$5
	.set	noreorder
	.set	nomacro
	jal	strlen
	sw	$5,132($sp)
	.set	macro
	.set	reorder

	move	$4,$22
	lw	$5,132($sp)
	.set	noreorder
	.set	nomacro
	j	$L235
	addiu	$6,$2,1
	.set	macro
	.set	reorder

$L220:
	lui	$21,%hi($LC26)
	.set	noreorder
	.set	nomacro
	jal	strlen
	addiu	$4,$21,%lo($LC26)
	.set	macro
	.set	reorder

	move	$4,$22
	addiu	$5,$21,%lo($LC26)
	move	$6,$2
$L235:
	.set	noreorder
	.set	nomacro
	jal	memcpy
	li	$21,-4			# 0xfffffffc
	.set	macro
	.set	reorder

	move	$4,$17
	lw	$2,8($16)
	li	$17,20			# 0x14
	lw	$22,4($16)
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_len
	sw	$2,4($16)
	.set	macro
	.set	reorder

	move	$4,$19
	lw	$5,12($20)
	lbu	$3,0($18)
	srl	$5,$5,28
	andi	$3,$3,0xf
	addu	$3,$5,$3
	mult	$3,$21
	mflo	$3
	addu	$22,$3,$22
	addu	$22,$22,$2
	lui	$2,%hi(url_cache+12)
	sw	$22,8($16)
	lhu	$2,%lo(url_cache+12)($2)
	sb	$0,13($16)
	sh	$2,14($16)
	li	$2,-327680			# 0xfffb0000
	lw	$3,12($16)
	ori	$2,$2,0xffff
	sh	$0,18($16)
	and	$2,$3,$2
	li	$3,1048576			# 0x100000
	or	$2,$2,$3
	li	$3,524288			# 0x80000
	or	$2,$2,$3
	li	$3,-131072			# 0xfffe0000
	ori	$3,$3,0xffff
	and	$2,$2,$3
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_len
	sw	$2,12($16)
	.set	macro
	.set	reorder

	li	$4,8			# 0x8
	lbu	$5,0($18)
	lw	$3,12($20)
	andi	$20,$5,0xf
	mult	$20,$21
	srl	$3,$3,28
	sll	$3,$3,2
	mflo	$20
	addiu	$20,$20,-14
	.set	noreorder
	.set	nomacro
	j	$L232
	addu	$20,$20,$2
	.set	macro
	.set	reorder

$L226:
	lbu	$5,0($7)
	sltu	$7,$5,2
	beq	$7,$0,$L223
	.set	noreorder
	.set	nomacro
	j	$L232
	move	$17,$2
	.set	macro
	.set	reorder

$L223:
	lbu	$2,0($6)
	.set	noreorder
	.set	nomacro
	bne	$5,$4,$L232
	addu	$17,$17,$2
	.set	macro
	.set	reorder

	subu	$17,$17,$2
	addu	$23,$16,$23
	addiu	$21,$sp,16
	addiu	$17,$17,6
	move	$4,$21
	addiu	$22,$sp,20
	move	$5,$23
	addu	$17,$16,$17
	.set	noreorder
	.set	nomacro
	jal	memcpy
	li	$6,4			# 0x4
	.set	macro
	.set	reorder

	move	$4,$22
	li	$6,4			# 0x4
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$5,$17
	.set	macro
	.set	reorder

	move	$4,$23
	li	$6,4			# 0x4
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$5,$22
	.set	macro
	.set	reorder

	move	$4,$17
	li	$6,4			# 0x4
	.set	noreorder
	.set	nomacro
	jal	memcpy
	move	$5,$21
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L240
	move	$5,$20
	.set	macro
	.set	reorder

$L232:
	addiu	$2,$17,1
	sltu	$5,$17,$3
	addu	$7,$16,$17
	addu	$6,$16,$2
	.set	noreorder
	.set	nomacro
	bne	$5,$0,$L226
	addiu	$23,$17,2
	.set	macro
	.set	reorder

	move	$5,$20
$L240:
	sh	$0,16($16)
	move	$6,$0
	move	$4,$16
	lw	$21,16($18)
	andi	$20,$20,0xffff
	lw	$17,12($18)
	.set	noreorder
	.set	nomacro
	jal	csum_partial
	addiu	$20,$20,6
	.set	macro
	.set	reorder

	move	$4,$19
#APP
 # 141 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/checksum.h" 1
		.set	push		# csum_tcpudp_nofold
	.set	noat		
	addu	$21, $17		
	sltu	$1, $21, $17	
	addu	$21, $1		
	addu	$21, $20		
	sltu	$1, $21, $20	
	addu	$21, $1		
	addu	$21, $2		
	sltu	$1, $21, $2	
	addu	$21, $1		
	.set	pop
 # 0 "" 2
 # 83 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/checksum.h" 1
		.set	push		# csum_fold
	.set	noat		
	sll	$1, $21, 16	
	addu	$21, $1		
	sltu	$1, $21, $1	
	srl	$21, $21, 16	
	addu	$21, $1		
	xori	$21, 0xffff	
	.set	pop
 # 0 "" 2
#NO_APP
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_len
	sh	$21,16($16)
	.set	macro
	.set	reorder

	lui	$3,%hi(url_cache)
	addiu	$2,$2,-14
	addiu	$4,$3,%lo(url_cache)
	sh	$2,2($18)
	addiu	$2,$18,16
	lbu	$4,14($4)
	sh	$0,6($18)
	sb	$4,8($18)
	lhu	$3,%lo(url_cache)($3)
	lw	$5,0($18)
	addiu	$3,$3,2
	sh	$3,4($18)
	srl	$7,$5,22
	lw	$6,4($18)
	andi	$7,$7,0x3c
	sh	$0,10($18)
	addu	$7,$18,$7
	lw	$4,8($18)
	addu	$5,$6,$5
	sltu	$6,$5,$6
	addu	$3,$5,$4
	lw	$5,12($18)
	addu	$6,$3,$6
	sltu	$4,$6,$4
	addu	$6,$6,$5
	addu	$4,$6,$4
	sltu	$3,$4,$5
	addu	$3,$3,$4
$L227:
	lw	$4,0($2)
	addiu	$2,$2,4
	addu	$3,$3,$4
	sltu	$4,$3,$4
	.set	noreorder
	.set	nomacro
	bne	$2,$7,$L227
	addu	$3,$4,$3
	.set	macro
	.set	reorder

	move	$2,$3
	li	$3,1			# 0x1
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
	move	$4,$19
	sh	$2,10($18)
	lui	$2,%hi(url_cache+15)
	.set	noreorder
	.set	nomacro
	jal	rtl_get_skb_data
	sb	$3,%lo(url_cache+15)($2)
	.set	macro
	.set	reorder

	addiu	$4,$sp,24
	move	$5,$2
	.set	noreorder
	.set	nomacro
	jal	memcpy
	li	$6,54			# 0x36
	.set	macro
	.set	reorder

	lui	$2,%hi(finStateClientPort)
	move	$4,$19
	lhu	$3,2($16)
	sh	$3,%lo(finStateClientPort)($2)
	lui	$2,%hi(finStateClientSeq)
	lw	$3,8($16)
	.set	noreorder
	.set	nomacro
	jal	dev_queue_xmit
	sw	$3,%lo(finStateClientSeq)($2)
	.set	macro
	.set	reorder

$L210:
	lw	$31,172($sp)
	move	$2,$0
	lw	$23,168($sp)
	lw	$22,164($sp)
	lw	$21,160($sp)
	lw	$20,156($sp)
	lw	$19,152($sp)
	lw	$18,148($sp)
	lw	$17,144($sp)
	lw	$16,140($sp)
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,176
	.set	macro
	.set	reorder

$L238:
#APP
 # 167 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
 # 169 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	1:	ll	$3, 72($16)		# atomic_sub_return	
       nop                                             
	subu	$2, $3, 1				
	sc	$2, 72($16)					
	beqz	$2, 2f					
	subu	$2, $3, 1				
	.subsection 2					
2:	b	1b					
	.previous					

 # 0 "" 2
 # 185 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/atomic.h" 1
	   

 # 0 "" 2
#NO_APP
	bne	$2,$0,$L210
	j	$L239
	.end	GenerateHTTP401
	.size	GenerateHTTP401, .-GenerateHTTP401
	.section	.rodata.str1.4
	.align	2
$LC27:
	.ascii	"%s\000"
	.section	.text.register_customRspStr,"ax",@progbits
	.align	2
	.globl	register_customRspStr
	.set	nomips16
	.ent	register_customRspStr
	.type	register_customRspStr, @function
register_customRspStr:
	.frame	$sp,32,$31		# vars= 0, regs= 3/0, args= 16, gp= 0
	.mask	0x80030000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$17,24($sp)
	sw	$16,20($sp)
	move	$16,$4
	jal	unregister_customRspStr
	lui	$17,%hi(block_message)

	beq	$16,$0,$L243
	nop

$L244 = .
	jal	strlen
	move	$4,$16

	li	$5,208			# 0xd0
	jal	__kmalloc
	addiu	$4,$2,1

	beq	$2,$0,$L246
	sw	$2,%lo(block_message)($17)

	lui	$5,%hi($LC27)
	move	$4,$2
	addiu	$5,$5,%lo($LC27)
	j	$L247
	move	$6,$16

$L243:
	lui	$16,%hi($LC26)
	jal	strlen
	addiu	$4,$16,%lo($LC26)

	li	$5,208			# 0xd0
	jal	__kmalloc
	move	$4,$2

	beq	$2,$0,$L246
	sw	$2,%lo(block_message)($17)

	lui	$5,%hi($LC27)
	move	$4,$2
	addiu	$6,$16,%lo($LC26)
	addiu	$5,$5,%lo($LC27)
$L247:
	lw	$31,28($sp)
	lw	$17,24($sp)
	lw	$16,20($sp)
	j	sprintf
	addiu	$sp,$sp,32

$L246:
	lw	$31,28($sp)
	lw	$17,24($sp)
	lw	$16,20($sp)
	j	$31
	addiu	$sp,$sp,32

	.set	macro
	.set	reorder
	.end	register_customRspStr
	.size	register_customRspStr, .-register_customRspStr
	.section	.text.filter_table_regist,"ax",@progbits
	.align	2
	.globl	filter_table_regist
	.set	nomips16
	.ent	filter_table_regist
	.type	filter_table_regist, @function
filter_table_regist:
	.frame	$sp,32,$31		# vars= 0, regs= 3/0, args= 16, gp= 0
	.mask	0x80030000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$17,24($sp)
	sw	$16,20($sp)
	lui	$2,%hi(table_list_head+24)
	move	$17,$4
	lw	$2,%lo(table_list_head+24)($2)
	bne	$2,$0,$L250
	move	$16,$5

	jal	filter_table_head_init
	nop

$L249 = .
$L250:
	lui	$2,%hi(malloc_sizes+4)
	lw	$4,%lo(malloc_sizes+4)($2)
	jal	kmem_cache_alloc
	li	$5,208			# 0xd0

	lui	$5,%hi(table_list_head)
	sw	$17,16($2)
	move	$3,$2
	addiu	$6,$2,8
	li	$2,1			# 0x1
	addiu	$4,$5,%lo(table_list_head)
	sw	$2,24($3)
	move	$2,$0
	sw	$3,0($3)
	sw	$3,4($3)
	sw	$16,28($3)
	sw	$6,12($3)
	sw	$0,20($3)
	sw	$6,8($3)
	lw	$6,%lo(table_list_head)($5)
	sw	$3,4($6)
	sw	$6,0($3)
	sw	$4,4($3)
	lw	$6,24($4)
	sw	$3,%lo(table_list_head)($5)
	addiu	$3,$6,1
	sw	$3,24($4)
	lw	$31,28($sp)
	lw	$17,24($sp)
	lw	$16,20($sp)
	j	$31
	addiu	$sp,$sp,32

	.set	macro
	.set	reorder
	.end	filter_table_regist
	.size	filter_table_regist, .-filter_table_regist
	.section	.text.strtok,"ax",@progbits
	.align	2
	.globl	strtok
	.set	nomips16
	.ent	strtok
	.type	strtok, @function
strtok:
	.frame	$sp,32,$31		# vars= 0, regs= 3/0, args= 16, gp= 0
	.mask	0x80030000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$17,24($sp)
	sw	$16,20($sp)
	move	$17,$5
	bne	$4,$0,$L254
	move	$16,$4

	lui	$2,%hi(___strtok)
	lw	$16,%lo(___strtok)($2)
	beq	$16,$0,$L259
	move	$2,$16

$L254:
	move	$4,$16
	jal	strspn
	move	$5,$17

	addu	$16,$16,$2
	lb	$2,0($16)
	bne	$2,$0,$L256
	lui	$2,%hi(___strtok)

	move	$16,$0
	j	$L255
	sw	$0,%lo(___strtok)($2)

$L256:
	move	$5,$17
	jal	strpbrk
	move	$4,$16

	beq	$2,$0,$L260
	lui	$3,%hi(___strtok)

	lb	$3,0($2)
	beq	$3,$0,$L260
	lui	$3,%hi(___strtok)

	sb	$0,0($2)
	addiu	$2,$2,1
$L260:
	sw	$2,%lo(___strtok)($3)
$L255:
	move	$2,$16
$L259:
	lw	$31,28($sp)
	lw	$17,24($sp)
	lw	$16,20($sp)
	j	$31
	addiu	$sp,$sp,32

	.set	macro
	.set	reorder
	.end	strtok
	.size	strtok, .-strtok
	.section	.rodata.str1.4
	.align	2
$LC28:
	.ascii	" \000"
	.section	.text.dos_write_proc,"ax",@progbits
	.align	2
	.set	nomips16
	.ent	dos_write_proc
	.type	dos_write_proc, @function
dos_write_proc:
	.frame	$sp,136,$31		# vars= 80, regs= 10/0, args= 16, gp= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-136
	sw	$31,132($sp)
	sw	$fp,128($sp)
	sw	$23,124($sp)
	sw	$22,120($sp)
	sw	$21,116($sp)
	sw	$20,112($sp)
	sw	$19,108($sp)
	sw	$18,104($sp)
	sw	$17,100($sp)
	sw	$16,96($sp)
	sltu	$3,$6,2
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L262
	move	$16,$6
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$5,$0,$L280
	li	$2,-14			# 0xfffffff2
	.set	macro
	.set	reorder

	addiu	$4,$5,80
	lw	$3,24($28)
	or	$2,$4,$5
	and	$2,$2,$3
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L262
	lui	$2,%hi(dos_flag)
	.set	macro
	.set	reorder

	li	$6,80			# 0x50
	addiu	$4,$2,%lo(dos_flag)
#APP
 # 3077 "net/rtl/fastpath/filter.c" 1
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
	bne	$6,$0,$L262
	addiu	$4,$sp,16
	.set	macro
	.set	reorder

	addiu	$2,$2,%lo(dos_flag)
	li	$5,80			# 0x50
	move	$3,$4
#APP
 # 50 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/string.h" 1
	.set	noreorder
	.set	noat
1:	lbu	$1,($2)
	subu	$5,1
	sb	$1,($3)
	beqz	$1,2f
	addiu	$3,1
	bnez	$5,1b
	addiu	$2,1
2:
	.set	at
	.set	reorder
 # 0 "" 2
#NO_APP
	li	$18,1			# 0x1
	lui	$fp,%hi(per_fin_threshold)
	lui	$23,%hi(per_syn_threshold)
	lui	$22,%hi(whole_icmp_threshold)
	lui	$21,%hi(whole_udp_threshold)
	lui	$20,%hi(whole_fin_threshold)
	.set	noreorder
	.set	nomacro
	j	$L263
	lui	$19,%hi(whole_syn_threshold)
	.set	macro
	.set	reorder

$L279:
	move	$4,$17
	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	simple_strtol
	move	$6,$0
	.set	macro
	.set	reorder

	addiu	$3,$18,-1
	andi	$3,$3,0x00ff
	sltu	$4,$3,13
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L264
	lui	$4,%hi($L278)
	.set	macro
	.set	reorder

	sll	$3,$3,2
	addiu	$4,$4,%lo($L278)
	addu	$3,$4,$3
	lw	$3,0($3)
	j	$3
	.rdata
	.align	2
	.align	2
$L278:
	.word	$L265
	.word	$L266
	.word	$L267
	.word	$L268
	.word	$L269
	.word	$L270
	.word	$L271
	.word	$L272
	.word	$L273
	.word	$L274
	.word	$L275
	.word	$L276
	.word	$L277
	.section	.text.dos_write_proc
$L265:
	lui	$3,%hi(op_mode)
	.set	noreorder
	.set	nomacro
	j	$L264
	sb	$2,%lo(op_mode)($3)
	.set	macro
	.set	reorder

$L266:
	move	$4,$17
	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	simple_strtol
	li	$6,16			# 0x10
	.set	macro
	.set	reorder

	lui	$4,%hi(lan_addr)
	.set	noreorder
	.set	nomacro
	j	$L264
	sw	$2,%lo(lan_addr)($4)
	.set	macro
	.set	reorder

$L267:
	move	$4,$17
	move	$5,$0
	.set	noreorder
	.set	nomacro
	jal	simple_strtol
	li	$6,16			# 0x10
	.set	macro
	.set	reorder

	lui	$3,%hi(lan_mask)
	.set	noreorder
	.set	nomacro
	j	$L264
	sw	$2,%lo(lan_mask)($3)
	.set	macro
	.set	reorder

$L268:
	lui	$4,%hi(item)
	sw	$2,%lo(item)($4)
	lui	$2,%hi(item_backup)
	.set	noreorder
	.set	nomacro
	j	$L264
	sw	$0,%lo(item_backup)($2)
	.set	macro
	.set	reorder

$L269:
	.set	noreorder
	.set	nomacro
	j	$L264
	sw	$2,%lo(whole_syn_threshold)($19)
	.set	macro
	.set	reorder

$L270:
	.set	noreorder
	.set	nomacro
	j	$L264
	sw	$2,%lo(whole_fin_threshold)($20)
	.set	macro
	.set	reorder

$L271:
	.set	noreorder
	.set	nomacro
	j	$L264
	sw	$2,%lo(whole_udp_threshold)($21)
	.set	macro
	.set	reorder

$L272:
	.set	noreorder
	.set	nomacro
	j	$L264
	sw	$2,%lo(whole_icmp_threshold)($22)
	.set	macro
	.set	reorder

$L273:
	.set	noreorder
	.set	nomacro
	j	$L264
	sw	$2,%lo(per_syn_threshold)($23)
	.set	macro
	.set	reorder

$L274:
	.set	noreorder
	.set	nomacro
	j	$L264
	sw	$2,%lo(per_fin_threshold)($fp)
	.set	macro
	.set	reorder

$L275:
	lui	$3,%hi(per_udp_threshold)
	.set	noreorder
	.set	nomacro
	j	$L264
	sw	$2,%lo(per_udp_threshold)($3)
	.set	macro
	.set	reorder

$L276:
	lui	$4,%hi(per_icmp_threshold)
	.set	noreorder
	.set	nomacro
	j	$L264
	sw	$2,%lo(per_icmp_threshold)($4)
	.set	macro
	.set	reorder

$L277:
	lui	$3,%hi(block_time)
	sw	$2,%lo(block_time)($3)
$L264:
	addiu	$18,$18,1
	move	$4,$0
	andi	$18,$18,0x00ff
$L263:
	lui	$5,%hi($LC28)
	.set	noreorder
	.set	nomacro
	jal	strtok
	addiu	$5,$5,%lo($LC28)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L279
	move	$17,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L280
	move	$2,$16
	.set	macro
	.set	reorder

$L262:
	li	$2,-14			# 0xfffffff2
$L280:
	lw	$31,132($sp)
	lw	$fp,128($sp)
	lw	$23,124($sp)
	lw	$22,120($sp)
	lw	$21,116($sp)
	lw	$20,112($sp)
	lw	$19,108($sp)
	lw	$18,104($sp)
	lw	$17,100($sp)
	lw	$16,96($sp)
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,136
	.set	macro
	.set	reorder

	.end	dos_write_proc
	.size	dos_write_proc, .-dos_write_proc
	.section	.text.Add_Pattern_ACL_For_ContentFilter,"ax",@progbits
	.align	2
	.globl	Add_Pattern_ACL_For_ContentFilter
	.set	nomips16
	.ent	Add_Pattern_ACL_For_ContentFilter
	.type	Add_Pattern_ACL_For_ContentFilter, @function
Add_Pattern_ACL_For_ContentFilter:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	j	rtl_Add_Pattern_ACL_For_ContentFilter
	nop

	.set	macro
	.set	reorder
	.end	Add_Pattern_ACL_For_ContentFilter
	.size	Add_Pattern_ACL_For_ContentFilter, .-Add_Pattern_ACL_For_ContentFilter
	.section	.rodata.str1.4
	.align	2
$LC29:
	.ascii	"type=%d num=%d\012\000"
	.align	2
$LC30:
	.ascii	"index=%d\011\000"
	.align	2
$LC31:
	.ascii	"mac=%02x:%02x:%02x:%02x:%02x:%02x\012\000"
	.align	2
$LC32:
	.ascii	"index=%d\011\011\000"
	.align	2
$LC33:
	.ascii	"url=%s\012\000"
	.align	2
$LC34:
	.ascii	"start=%x\011\000"
	.align	2
$LC35:
	.ascii	"end=%x\011\000"
	.align	2
$LC36:
	.ascii	"day=%x\012\000"
	.align	2
$LC37:
	.ascii	"unknow table type\012\000"
	.align	2
$LC38:
	.ascii	"Mode:%d %s\012\000"
	.section	.text.filter_table_read_proc,"ax",@progbits
	.align	2
	.set	nomips16
	.ent	filter_table_read_proc
	.type	filter_table_read_proc, @function
filter_table_read_proc:
	.frame	$sp,80,$31		# vars= 8, regs= 10/0, args= 32, gp= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
$L285 = .
$L286 = .
	addiu	$sp,$sp,-80
	sw	$31,76($sp)
	sw	$fp,72($sp)
	sw	$23,68($sp)
	sw	$22,64($sp)
	sw	$21,60($sp)
	sw	$20,56($sp)
	sw	$19,52($sp)
	sw	$18,48($sp)
	sw	$17,44($sp)
	sw	$16,40($sp)
	lui	$16,%hi(malloc_sizes)
	move	$22,$4
	sw	$5,84($sp)
	addiu	$16,$16,%lo(malloc_sizes)
	move	$21,$6
	lw	$4,52($16)
	li	$5,208			# 0xd0
	move	$20,$7
	jal	kmem_cache_alloc
	move	$18,$0

	beq	$2,$0,$L288
	move	$17,$2

$L289 = .
$L290 = .
	move	$4,$2
	move	$5,$0
	jal	memset
	li	$6,512			# 0x200

	lw	$4,44($16)
	jal	kmem_cache_alloc
	li	$5,208			# 0xd0

	move	$4,$17
	beq	$2,$0,$L308
	move	$16,$2

	move	$4,$2
	move	$5,$0
	jal	memset
	li	$6,256			# 0x100

	lui	$fp,%hi($LC32)
	lui	$2,%hi(table_list_head)
	lui	$23,%hi($LC30)
	addiu	$fp,$fp,%lo($LC32)
	lw	$18,%lo(table_list_head)($2)
	j	$L292
	addiu	$23,$23,%lo($LC30)

$L304:
	lw	$7,24($18)
	move	$4,$17
	lw	$6,16($18)
	addiu	$5,$5,%lo($LC29)
	jal	sprintf
	addiu	$7,$7,-1

	li	$3,3			# 0x3
	lw	$2,16($18)
	beq	$2,$3,$L295
	nop

	li	$3,4			# 0x4
	beq	$2,$3,$L296
	nop

	li	$3,2			# 0x2
	bne	$2,$3,$L307
	move	$4,$16

	lw	$19,8($18)
	j	$L297
	addiu	$3,$18,8

$L298:
	lw	$6,20($19)
	move	$5,$23
	jal	sprintf
	sw	$3,36($sp)

	move	$4,$17
	jal	strcat
	move	$5,$16

	lui	$5,%hi($LC31)
	move	$4,$16
	lbu	$2,30($19)
	addiu	$5,$5,%lo($LC31)
	lbu	$6,28($19)
	lbu	$7,29($19)
	sw	$2,16($sp)
	lbu	$2,31($19)
	sw	$2,20($sp)
	lbu	$2,32($19)
	sw	$2,24($sp)
	lbu	$2,33($19)
	jal	sprintf
	sw	$2,28($sp)

	move	$4,$17
	jal	strcat
	move	$5,$16

	lw	$19,0($19)
	lw	$3,36($sp)
$L297:
	bne	$19,$3,$L298
	move	$4,$16

	j	$L299
	nop

$L295:
	lw	$19,8($18)
	j	$L300
	addiu	$3,$18,8

$L301:
	lw	$6,20($19)
	move	$5,$fp
	jal	sprintf
	sw	$3,36($sp)

	move	$4,$17
	jal	strcat
	move	$5,$16

	lui	$5,%hi($LC33)
	move	$4,$16
	addiu	$6,$19,28
	jal	sprintf
	addiu	$5,$5,%lo($LC33)

	move	$4,$17
	jal	strcat
	move	$5,$16

	lw	$19,0($19)
	lw	$3,36($sp)
$L300:
	bne	$19,$3,$L301
	move	$4,$16

	j	$L299
	nop

$L296:
	lw	$19,8($18)
	j	$L302
	addiu	$3,$18,8

$L303:
	lw	$6,20($19)
	move	$4,$16
	move	$5,$23
	sw	$3,36($sp)
	jal	sprintf
	sw	$2,32($sp)

	move	$4,$17
	jal	strcat
	move	$5,$16

	lui	$5,%hi($LC34)
	move	$4,$16
	lw	$6,28($19)
	jal	sprintf
	addiu	$5,$5,%lo($LC34)

	move	$4,$17
	jal	strcat
	move	$5,$16

	lui	$5,%hi($LC35)
	move	$4,$16
	lw	$2,32($sp)
	lw	$6,4($2)
	jal	sprintf
	addiu	$5,$5,%lo($LC35)

	move	$4,$17
	jal	strcat
	move	$5,$16

	lui	$5,%hi($LC36)
	move	$4,$16
	lw	$2,32($sp)
	lw	$6,8($2)
	jal	sprintf
	addiu	$5,$5,%lo($LC36)

	move	$4,$17
	jal	strcat
	move	$5,$16

	lw	$19,0($19)
	lw	$3,36($sp)
$L302:
	bne	$19,$3,$L303
	addiu	$2,$19,28

	j	$L299
	nop

$L307:
	lui	$5,%hi($LC37)
	jal	sprintf
	addiu	$5,$5,%lo($LC37)

	move	$4,$17
	jal	strcat
	move	$5,$16

$L299:
	lw	$18,0($18)
$L292:
	lui	$3,%hi(table_list_head)
	addiu	$3,$3,%lo(table_list_head)
	bne	$18,$3,$L304
	lui	$5,%hi($LC29)

	lui	$2,%hi(url_filter_mode)
	lui	$5,%hi($LC38)
	move	$4,$22
	lw	$6,%lo(url_filter_mode)($2)
	addiu	$5,$5,%lo($LC38)
	jal	sprintf
	move	$7,$17

	addu	$3,$20,$21
	slt	$3,$3,$2
	bne	$3,$0,$L305
	li	$4,1			# 0x1

	lw	$3,96($sp)
	sw	$4,0($3)
$L305:
	subu	$2,$2,$21
	addu	$21,$22,$21
	move	$4,$17
	slt	$18,$20,$2
	lw	$3,84($sp)
	movz	$20,$2,$18
	slt	$18,$20,0
	sw	$21,0($3)
	jal	kfree
	movn	$20,$0,$18

	move	$18,$20
	move	$4,$16
$L308:
	jal	kfree
	nop

$L288:
	move	$2,$18
	lw	$31,76($sp)
	lw	$fp,72($sp)
	lw	$23,68($sp)
	lw	$22,64($sp)
	lw	$21,60($sp)
	lw	$20,56($sp)
	lw	$19,52($sp)
	lw	$18,48($sp)
	lw	$17,44($sp)
	lw	$16,40($sp)
	j	$31
	addiu	$sp,$sp,80

	.set	macro
	.set	reorder
	.end	filter_table_read_proc
	.size	filter_table_read_proc, .-filter_table_read_proc
	.section	.text.scan_table,"ax",@progbits
	.align	2
	.globl	scan_table
	.set	nomips16
	.ent	scan_table
	.type	scan_table, @function
scan_table:
	.frame	$sp,48,$31		# vars= 0, regs= 8/0, args= 16, gp= 0
	.mask	0x807f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-48
	sw	$31,44($sp)
	sw	$22,40($sp)
	sw	$21,36($sp)
	sw	$20,32($sp)
	sw	$19,28($sp)
	sw	$18,24($sp)
	sw	$17,20($sp)
	sw	$16,16($sp)
	lui	$2,%hi(url_filter_flag)
	move	$17,$6
	sw	$0,%lo(url_filter_flag)($2)
	move	$19,$4
	move	$22,$5
	beq	$6,$0,$L310
	move	$20,$7

	lw	$2,28($6)
	beq	$2,$0,$L310
	addiu	$21,$6,8

	lw	$16,8($6)
	j	$L311
	move	$18,$0

$L321:
	lw	$2,16($16)
	andi	$3,$2,0x1
	bne	$3,$0,$L312
	move	$18,$16

	lw	$3,20($16)
	bne	$22,$3,$L313
	nop

$L312:
	andi	$3,$2,0x200
	beq	$3,$0,$L314
	nop

	andi	$2,$2,0x2
	lw	$6,0($20)
	beq	$2,$0,$L313
	nop

	beq	$6,$0,$L310
	move	$4,$19

	lw	$5,20($16)
	jal	scan_table
	move	$7,$6

	bne	$2,$0,$L318
	li	$2,1			# 0x1

	j	$L313
	nop

$L314:
	andi	$2,$2,0x100
	bne	$2,$0,$L316
	move	$4,$19

	lw	$2,28($17)
	jalr	$2
	addiu	$5,$16,28

	blez	$2,$L313
	nop

$L316:
	lw	$2,16($16)
	andi	$3,$2,0x2
	bne	$3,$0,$L317
	andi	$3,$2,0x1

	beq	$3,$0,$L315
	srl	$2,$2,3

	j	$L325
	xori	$2,$2,0x1

$L317:
	lw	$6,0($20)
	beq	$6,$0,$L310
	move	$4,$19

	lw	$5,20($16)
	jal	scan_table
	move	$7,$6

	beq	$2,$0,$L313
	nop

	lw	$2,16($16)
	andi	$4,$2,0x1
	beq	$4,$0,$L319
	lui	$3,%hi(url_filter_mode)

	lw	$4,%lo(url_filter_mode)($3)
	srl	$3,$2,3
	bne	$4,$0,$L318
	andi	$2,$3,0x1

	j	$L325
	xori	$2,$3,0x1

$L319:
	lw	$2,%lo(url_filter_mode)($3)
	j	$L318
	sltu	$2,$2,1

$L313:
	lw	$16,0($16)
$L311:
	bne	$16,$21,$L321
	nop

	bne	$18,$0,$L322
	lui	$2,%hi(url_filter_flag)

	lui	$2,%hi(url_filter_mode)
	lw	$2,%lo(url_filter_mode)($2)
	bne	$2,$0,$L315
	li	$2,-1			# 0xffffffff

	j	$L318
	nop

$L322:
	lw	$2,%lo(url_filter_flag)($2)
	bne	$2,$0,$L315
	nop

	lw	$3,16($18)
	andi	$4,$3,0x1
	beq	$4,$0,$L318
	nop

	srl	$2,$3,3
$L325:
	j	$L318
	andi	$2,$2,0x1

$L310:
	j	$L318
	li	$2,-1			# 0xffffffff

$L315:
	li	$2,1			# 0x1
$L318:
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
	.end	scan_table
	.size	scan_table, .-scan_table
	.section	.text.filter_item_regist,"ax",@progbits
	.align	2
	.globl	filter_item_regist
	.set	nomips16
	.ent	filter_item_regist
	.type	filter_item_regist, @function
filter_item_regist:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, gp= 0
	.mask	0x80000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	lui	$2,%hi(table_list_head)
	addiu	$3,$2,%lo(table_list_head)
	lw	$2,%lo(table_list_head)($2)
	j	$L327
	nop

$L331:
	lw	$7,16($2)
	bne	$7,$4,$L328
	nop

	lw	$3,8($2)
	addiu	$6,$2,8
	sw	$5,4($3)
	sw	$3,0($5)
	sw	$6,4($5)
	lw	$3,24($2)
	sw	$5,8($2)
	addiu	$3,$3,1
	sw	$3,24($2)
	li	$2,3			# 0x3
	bne	$4,$2,$L330
	move	$2,$0

	li	$2,2			# 0x2
	bne	$3,$2,$L329
	nop

	jal	rtl_Add_Pattern_ACL_For_ContentFilter
	nop

	j	$L330
	move	$2,$0

$L328:
	move	$2,$6
$L327:
	lw	$6,0($2)
	bne	$2,$3,$L331
	nop

	j	$L330
	li	$2,-1			# 0xffffffff

$L329:
	move	$2,$0
$L330:
	lw	$31,20($sp)
	j	$31
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	filter_item_regist
	.size	filter_item_regist, .-filter_item_regist
	.section	.rodata.str1.4
	.align	2
$LC39:
	.ascii	"init\000"
	.align	2
$LC40:
	.ascii	"flush\000"
	.align	2
$LC41:
	.ascii	"enable_log\000"
	.align	2
$LC42:
	.ascii	"white\000"
	.align	2
$LC43:
	.ascii	"black\000"
	.align	2
$LC44:
	.ascii	"add:\000"
	.align	2
$LC45:
	.ascii	":\000"
	.align	2
$LC46:
	.ascii	";\000"
	.align	2
$LC47:
	.ascii	"#\000"
	.align	2
$LC48:
	.ascii	"-\000"
	.section	.text.filter_table_write_proc,"ax",@progbits
	.align	2
	.set	nomips16
	.ent	filter_table_write_proc
	.type	filter_table_write_proc, @function
filter_table_write_proc:
	.frame	$sp,88,$31		# vars= 32, regs= 10/0, args= 16, gp= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
$L334 = .
$L335 = .
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
	lui	$2,%hi(malloc_sizes+60)
	move	$19,$5
	lw	$4,%lo(malloc_sizes+60)($2)
	li	$5,208			# 0xd0
	move	$17,$6
	.set	noreorder
	.set	nomacro
	jal	kmem_cache_alloc
	li	$18,-14			# 0xfffffff2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L337
	move	$16,$2
	.set	macro
	.set	reorder

	lui	$20,%hi(get_info_from_usr_space)
	addiu	$4,$20,%lo(get_info_from_usr_space)
	.set	noreorder
	.set	nomacro
	jal	strlen
	sw	$2,28($sp)
	.set	macro
	.set	reorder

	addiu	$4,$20,%lo(get_info_from_usr_space)
	move	$6,$2
	.set	noreorder
	.set	nomacro
	jal	memset
	move	$5,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$19,$0,$L338
	addu	$2,$19,$17
	.set	macro
	.set	reorder

	move	$18,$17
	or	$2,$2,$19
	lw	$3,24($28)
	or	$2,$2,$17
	and	$2,$2,$3
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L340
	move	$6,$17
	.set	macro
	.set	reorder

	addiu	$4,$20,%lo(get_info_from_usr_space)
	move	$5,$19
#APP
 # 671 "net/rtl/fastpath/filter_v2.c" 1
	.set	noreorder
	jal	__copy_user
	.set	noat
	addu	$1, $5, $6
	.set	at
	.set	reorder
 # 0 "" 2
#NO_APP
$L340:
	.set	noreorder
	.set	nomacro
	bne	$6,$0,$L386
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$17,$0,$L341
	lui	$3,%hi(get_info_from_usr_space)
	.set	macro
	.set	reorder

	move	$2,$16
	addiu	$3,$3,%lo(get_info_from_usr_space)
#APP
 # 50 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/string.h" 1
	.set	noreorder
	.set	noat
1:	lbu	$1,($3)
	subu	$17,1
	sb	$1,($2)
	beqz	$1,2f
	addiu	$2,1
	bnez	$17,1b
	addiu	$3,1
2:
	.set	at
	.set	reorder
 # 0 "" 2
#NO_APP
$L341:
	lui	$4,%hi(get_info_from_usr_space)
	lui	$17,%hi($LC39)
	.set	noreorder
	.set	nomacro
	jal	strlen
	addiu	$4,$4,%lo(get_info_from_usr_space)
	.set	macro
	.set	reorder

	addu	$2,$16,$2
	sb	$0,-1($2)
	lw	$19,28($sp)
	.set	noreorder
	.set	nomacro
	jal	strlen
	addiu	$4,$17,%lo($LC39)
	.set	macro
	.set	reorder

	addiu	$5,$17,%lo($LC39)
	move	$4,$19
	.set	noreorder
	.set	nomacro
	jal	memcmp
	move	$6,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L342
	lui	$17,%hi($LC40)
	.set	macro
	.set	reorder

	lui	$20,%hi($LC28)
	addiu	$17,$sp,28
	addiu	$5,$20,%lo($LC28)
	move	$4,$17
	lui	$19,%hi(filter_tlb)
	.set	noreorder
	.set	nomacro
	jal	strsep
	move	$20,$5
	.set	macro
	.set	reorder

	addiu	$19,$19,%lo(filter_tlb)
	sw	$2,16($sp)
	move	$5,$20
$L387:
	.set	noreorder
	.set	nomacro
	jal	strsep
	move	$4,$17
	.set	macro
	.set	reorder

	move	$4,$2
	.set	noreorder
	.set	nomacro
	jal	str2hexnum
	sw	$2,16($sp)
	.set	macro
	.set	reorder

	sll	$3,$2,3
	addu	$3,$19,$3
	lw	$5,4($3)
	.set	noreorder
	.set	nomacro
	jal	filter_table_regist
	move	$4,$2
	.set	macro
	.set	reorder

	lw	$2,28($sp)
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L387
	move	$5,$20
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L385
	move	$4,$16
	.set	macro
	.set	reorder

$L342:
	lw	$19,28($sp)
	.set	noreorder
	.set	nomacro
	jal	strlen
	addiu	$4,$17,%lo($LC40)
	.set	macro
	.set	reorder

	addiu	$5,$17,%lo($LC40)
	move	$4,$19
	.set	noreorder
	.set	nomacro
	jal	memcmp
	move	$6,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L345
	lui	$17,%hi($LC41)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	filter_table_flush
	move	$4,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L385
	move	$4,$16
	.set	macro
	.set	reorder

$L345:
	lw	$19,28($sp)
	.set	noreorder
	.set	nomacro
	jal	strlen
	addiu	$4,$17,%lo($LC41)
	.set	macro
	.set	reorder

	addiu	$5,$17,%lo($LC41)
	move	$4,$19
	.set	noreorder
	.set	nomacro
	jal	memcmp
	move	$6,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L346
	li	$3,1			# 0x1
	.set	macro
	.set	reorder

	lui	$2,%hi(log_enable)
	.set	noreorder
	.set	nomacro
	j	$L344
	sw	$3,%lo(log_enable)($2)
	.set	macro
	.set	reorder

$L346:
	lui	$17,%hi($LC42)
	lw	$19,28($sp)
	.set	noreorder
	.set	nomacro
	jal	strlen
	addiu	$4,$17,%lo($LC42)
	.set	macro
	.set	reorder

	addiu	$5,$17,%lo($LC42)
	move	$4,$19
	.set	noreorder
	.set	nomacro
	jal	memcmp
	move	$6,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L347
	li	$3,1			# 0x1
	.set	macro
	.set	reorder

	lui	$2,%hi(url_filter_mode)
	.set	noreorder
	.set	nomacro
	j	$L344
	sw	$3,%lo(url_filter_mode)($2)
	.set	macro
	.set	reorder

$L347:
	lui	$17,%hi($LC43)
	lw	$19,28($sp)
	.set	noreorder
	.set	nomacro
	jal	strlen
	addiu	$4,$17,%lo($LC43)
	.set	macro
	.set	reorder

	addiu	$5,$17,%lo($LC43)
	move	$4,$19
	.set	noreorder
	.set	nomacro
	jal	memcmp
	move	$6,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L348
	lui	$2,%hi(url_filter_mode)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L344
	sw	$0,%lo(url_filter_mode)($2)
	.set	macro
	.set	reorder

$L348:
	lui	$17,%hi($LC44)
	lw	$19,28($sp)
	.set	noreorder
	.set	nomacro
	jal	strlen
	addiu	$4,$17,%lo($LC44)
	.set	macro
	.set	reorder

	addiu	$5,$17,%lo($LC44)
	move	$4,$19
	.set	noreorder
	.set	nomacro
	jal	memcmp
	move	$6,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L385
	move	$4,$16
	.set	macro
	.set	reorder

	lui	$5,%hi($LC45)
	addiu	$4,$sp,28
	addiu	$5,$5,%lo($LC45)
	.set	noreorder
	.set	nomacro
	jal	strsep
	lui	$19,%hi($LC28)
	.set	macro
	.set	reorder

	lui	$23,%hi(malloc_sizes)
	lui	$21,%hi(filter_rule_num)
	addiu	$19,$19,%lo($LC28)
	sw	$2,20($sp)
	addiu	$23,$23,%lo(malloc_sizes)
	addiu	$22,$sp,32
	lui	$5,%hi($LC46)
$L391:
	addiu	$4,$sp,28
	.set	noreorder
	.set	nomacro
	jal	strsep
	addiu	$5,$5,%lo($LC46)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L344
	sw	$2,20($sp)
	.set	macro
	.set	reorder

	move	$4,$2
	li	$6,1			# 0x1
	.set	noreorder
	.set	nomacro
	jal	memcmp
	move	$5,$19
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L388
	addiu	$17,$sp,20
	.set	macro
	.set	reorder

	lw	$4,20($sp)
	jal	strlen
	li	$3,1			# 0x1
	.set	noreorder
	.set	nomacro
	beq	$2,$3,$L385
	move	$4,$16
	.set	macro
	.set	reorder

$L349:
	addiu	$17,$sp,20
$L388:
	lui	$5,%hi($LC47)
	addiu	$5,$5,%lo($LC47)
	.set	noreorder
	.set	nomacro
	jal	strsep
	move	$4,$17
	.set	macro
	.set	reorder

	move	$4,$2
	.set	noreorder
	.set	nomacro
	jal	str2hexnum
	sw	$2,16($sp)
	.set	macro
	.set	reorder

	li	$3,-1			# 0xffffffff
	.set	noreorder
	.set	nomacro
	beq	$2,$3,$L344
	addiu	$20,$sp,16
	.set	macro
	.set	reorder

	lui	$5,%hi($LC47)
$L390:
	addiu	$4,$sp,20
	.set	noreorder
	.set	nomacro
	jal	strsep
	addiu	$5,$5,%lo($LC47)
	.set	macro
	.set	reorder

	move	$5,$19
	move	$4,$20
	.set	noreorder
	.set	nomacro
	jal	strsep
	sw	$2,16($sp)
	.set	macro
	.set	reorder

	move	$4,$2
	.set	noreorder
	.set	nomacro
	jal	str2hexnum
	sw	$2,24($sp)
	.set	macro
	.set	reorder

	move	$4,$20
	move	$5,$19
	.set	noreorder
	.set	nomacro
	jal	strsep
	move	$17,$2
	.set	macro
	.set	reorder

	move	$4,$2
	.set	noreorder
	.set	nomacro
	jal	str2hexnum
	sw	$2,24($sp)
	.set	macro
	.set	reorder

	move	$fp,$2
	li	$2,2			# 0x2
	.set	noreorder
	.set	nomacro
	beq	$17,$2,$L357
	slt	$2,$17,3
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L355
	li	$2,3			# 0x3
	.set	macro
	.set	reorder

	li	$2,1			# 0x1
	bne	$17,$2,$L350
	j	$L376
$L355:
	.set	noreorder
	.set	nomacro
	beq	$17,$2,$L374
	li	$2,4			# 0x4
	.set	macro
	.set	reorder

	bne	$17,$2,$L350
	j	$L375
$L376:
	lw	$4,20($23)
	.set	noreorder
	.set	nomacro
	jal	kmem_cache_alloc
	li	$5,208			# 0xd0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L344
	move	$17,$2
	.set	macro
	.set	reorder

	addiu	$2,$2,8
	move	$5,$19
	sw	$2,12($17)
	move	$4,$20
	sw	$17,0($17)
	sw	$17,4($17)
	sw	$2,8($17)
	sw	$0,24($17)
	lw	$2,%lo(filter_rule_num)($21)
	sw	$fp,16($17)
	.set	noreorder
	.set	nomacro
	jal	strsep
	sw	$2,20($17)
	.set	macro
	.set	reorder

	move	$4,$2
	.set	noreorder
	.set	nomacro
	jal	str2hexnum
	sw	$2,24($sp)
	.set	macro
	.set	reorder

	li	$6,4			# 0x4
	move	$5,$22
	addiu	$4,$17,28
	.set	noreorder
	.set	nomacro
	jal	memcpy
	sw	$2,32($sp)
	.set	macro
	.set	reorder

	move	$5,$19
	.set	noreorder
	.set	nomacro
	jal	strsep
	move	$4,$20
	.set	macro
	.set	reorder

	move	$4,$2
	.set	noreorder
	.set	nomacro
	jal	str2hexnum
	sw	$2,24($sp)
	.set	macro
	.set	reorder

	addiu	$4,$17,32
	move	$5,$22
	li	$6,4			# 0x4
	.set	noreorder
	.set	nomacro
	jal	memcpy
	sw	$2,32($sp)
	.set	macro
	.set	reorder

	move	$5,$17
	.set	noreorder
	.set	nomacro
	jal	filter_item_regist
	li	$4,1			# 0x1
	.set	macro
	.set	reorder

	lw	$2,16($sp)
	bne	$2,$0,$L376
	j	$L350
$L352 = .
$L357:
	lw	$4,20($23)
	.set	noreorder
	.set	nomacro
	jal	kmem_cache_alloc
	li	$5,208			# 0xd0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L344
	move	$17,$2
	.set	macro
	.set	reorder

	addiu	$2,$2,8
	move	$4,$20
	sw	$2,12($17)
	move	$5,$19
	sw	$17,0($17)
	sw	$17,4($17)
	sw	$2,8($17)
	sw	$0,24($17)
	lw	$2,%lo(filter_rule_num)($21)
	sw	$fp,16($17)
	sw	$2,20($17)
	.set	noreorder
	.set	nomacro
	jal	strsep
	sw	$17,44($sp)
	.set	macro
	.set	reorder

	lw	$3,44($sp)
	sw	$2,24($sp)
$L359:
	lui	$5,%hi($LC48)
	addiu	$4,$sp,24
	addiu	$5,$5,%lo($LC48)
	.set	noreorder
	.set	nomacro
	jal	strsep
	sw	$3,44($sp)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	str2hexnum
	move	$4,$2
	.set	macro
	.set	reorder

	lw	$3,44($sp)
	sb	$2,28($3)
	lw	$2,24($sp)
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L359
	addiu	$3,$3,1
	.set	macro
	.set	reorder

$L360 = .
	move	$5,$17
	.set	noreorder
	.set	nomacro
	jal	filter_item_regist
	li	$4,2			# 0x2
	.set	macro
	.set	reorder

	j	$L350
$L374:
	lw	$4,20($23)
	.set	noreorder
	.set	nomacro
	jal	kmem_cache_alloc
	li	$5,208			# 0xd0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L344
	move	$17,$2
	.set	macro
	.set	reorder

	move	$4,$20
	.set	noreorder
	.set	nomacro
	jal	strsep
	move	$5,$19
	.set	macro
	.set	reorder

	sw	$2,24($sp)
	lb	$2,0($2)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L362
	addiu	$2,$17,8
	.set	macro
	.set	reorder

	sw	$2,12($17)
	sw	$17,0($17)
	sw	$17,4($17)
	sw	$2,8($17)
	sw	$0,24($17)
	lw	$2,%lo(filter_rule_num)($21)
	sw	$fp,16($17)
	sw	$2,20($17)
	lw	$4,24($sp)
	jal	strlen
	sltu	$2,$2,41
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L364
	li	$3,39			# 0x27
	.set	macro
	.set	reorder

	lw	$4,24($sp)
	jal	strlen
	move	$3,$2
$L364:
	lw	$4,24($sp)
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L389
	addu	$2,$17,$3
	.set	macro
	.set	reorder

	addiu	$2,$17,28
	move	$5,$3
#APP
 # 50 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/string.h" 1
	.set	noreorder
	.set	noat
1:	lbu	$1,($4)
	subu	$5,1
	sb	$1,($2)
	beqz	$1,2f
	addiu	$2,1
	bnez	$5,1b
	addiu	$4,1
2:
	.set	at
	.set	reorder
 # 0 "" 2
#NO_APP
	addu	$2,$17,$3
$L389:
	move	$5,$17
	li	$4,3			# 0x3
	.set	noreorder
	.set	nomacro
	jal	filter_item_regist
	sb	$0,28($2)
	.set	macro
	.set	reorder

$L362:
	lw	$2,16($sp)
	bne	$2,$0,$L374
	j	$L350
$L375:
	lw	$4,20($23)
	.set	noreorder
	.set	nomacro
	jal	kmem_cache_alloc
	li	$5,208			# 0xd0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L344
	move	$17,$2
	.set	macro
	.set	reorder

	addiu	$3,$17,28
	addiu	$2,$2,8
	move	$5,$19
	sw	$3,40($sp)
	move	$4,$20
	sw	$2,12($17)
	sw	$17,0($17)
	sw	$17,4($17)
	sw	$2,8($17)
	sw	$0,24($17)
	lw	$2,%lo(filter_rule_num)($21)
	sw	$fp,16($17)
	.set	noreorder
	.set	nomacro
	jal	strsep
	sw	$2,20($17)
	.set	macro
	.set	reorder

	move	$4,$2
	.set	noreorder
	.set	nomacro
	jal	str2hexnum
	sw	$2,24($sp)
	.set	macro
	.set	reorder

	li	$6,4			# 0x4
	move	$5,$22
	lw	$4,40($sp)
	.set	noreorder
	.set	nomacro
	jal	memcpy
	sw	$2,32($sp)
	.set	macro
	.set	reorder

	move	$4,$20
	.set	noreorder
	.set	nomacro
	jal	strsep
	move	$5,$19
	.set	macro
	.set	reorder

	lw	$3,32($sp)
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L367
	sw	$2,24($sp)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	str2hexnum
	move	$4,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L384
	li	$2,1440			# 0x5a0
	.set	macro
	.set	reorder

$L367:
	lw	$4,24($sp)
	jal	str2hexnum
$L384:
	lw	$3,40($sp)
	li	$6,4			# 0x4
	move	$5,$22
	sw	$2,32($sp)
	.set	noreorder
	.set	nomacro
	jal	memcpy
	addiu	$4,$3,4
	.set	macro
	.set	reorder

	move	$5,$19
	.set	noreorder
	.set	nomacro
	jal	strsep
	move	$4,$20
	.set	macro
	.set	reorder

	move	$4,$2
	.set	noreorder
	.set	nomacro
	jal	str2hexnum
	sw	$2,24($sp)
	.set	macro
	.set	reorder

	move	$5,$22
	li	$6,4			# 0x4
	lw	$3,40($sp)
	addiu	$4,$3,8
	li	$3,255			# 0xff
	movn	$3,$2,$2
	.set	noreorder
	.set	nomacro
	jal	memcpy
	sw	$3,32($sp)
	.set	macro
	.set	reorder

	move	$5,$17
	.set	noreorder
	.set	nomacro
	jal	filter_item_regist
	li	$4,4			# 0x4
	.set	macro
	.set	reorder

	lw	$2,16($sp)
	bne	$2,$0,$L375
$L350:
	lw	$2,20($sp)
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L390
	lui	$5,%hi($LC47)
	.set	macro
	.set	reorder

	lw	$2,%lo(filter_rule_num)($21)
	addiu	$2,$2,1
	sw	$2,%lo(filter_rule_num)($21)
	lw	$2,28($sp)
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L391
	lui	$5,%hi($LC46)
	.set	macro
	.set	reorder

$L344:
	.set	noreorder
	.set	nomacro
	j	$L385
	move	$4,$16
	.set	macro
	.set	reorder

$L338:
	move	$4,$16
$L386:
	li	$18,-14			# 0xfffffff2
$L385:
	jal	kfree
$L337:
	move	$2,$18
	lw	$31,84($sp)
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

	.end	filter_table_write_proc
	.size	filter_table_write_proc, .-filter_table_write_proc
	.section	.rodata.str1.4
	.align	2
$LC49:
	.ascii	"[ip_range_filter]%d.%d.%d.%d ===> %d.%d.%d.%d  [Blocked]"
	.ascii	"\012\000"
	.align	2
$LC50:
	.ascii	"[mac_filter]%d.%d.%d.%d(%02x:%02x:%02x:%02x:%02x:%02x) ="
	.ascii	"==> %d.%d.%d.%d [Blocked]\012\000"
	.align	2
$LC51:
	.ascii	"[url_key_filter]%d.%d.%d.%d ==> %d.%d.%d.%d(%s)  [Blocke"
	.ascii	"d]\012\000"
	.align	2
$LC52:
	.ascii	"[schedule_filter]%d.%d.%d.%d ===> %d.%d.%d.%d  [Blocked]"
	.ascii	"\012\000"
	.section	.text.log_fastFilter,"ax",@progbits
	.align	2
	.globl	log_fastFilter
	.set	nomips16
	.ent	log_fastFilter
	.type	log_fastFilter, @function
log_fastFilter:
	.frame	$sp,184,$31		# vars= 8, regs= 9/0, args= 136, gp= 0
	.mask	0x80ff0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-184
	sw	$31,180($sp)
	sw	$23,176($sp)
	sw	$22,172($sp)
	sw	$21,168($sp)
	sw	$20,164($sp)
	sw	$19,160($sp)
	sw	$18,156($sp)
	sw	$17,152($sp)
	sw	$16,148($sp)
	move	$17,$5
	move	$18,$4
	jal	do_gettimeofday
	addiu	$4,$sp,136

	jal	rtl_ip_hdr
	move	$4,$17

	move	$4,$17
	jal	rtl_eth_hdr
	move	$16,$2

	li	$3,2			# 0x2
	beq	$18,$3,$L395
	nop

	slt	$2,$18,3
	beq	$2,$0,$L398
	li	$2,3			# 0x3

	li	$2,1			# 0x1
	bne	$18,$2,$L393
	nop

	j	$L416
	nop

$L398:
	beq	$18,$2,$L396
	li	$2,4			# 0x4

	bne	$18,$2,$L393
	nop

	j	$L417
	nop

$L416:
	beq	$16,$0,$L393
	lui	$4,%hi($LC49)

	lw	$2,16($16)
	lw	$3,12($16)
	j	$L412
	addiu	$4,$4,%lo($LC49)

$L395:
	beq	$2,$0,$L393
	nop

	beq	$16,$0,$L393
	nop

	lw	$18,12($16)
	jal	rtl_eth_hdr
	move	$4,$17

	move	$4,$17
	jal	rtl_eth_hdr
	move	$23,$2

	move	$4,$17
	addiu	$23,$23,84
	jal	rtl_eth_hdr
	move	$22,$2

	move	$4,$17
	jal	rtl_eth_hdr
	move	$21,$2

	move	$4,$17
	jal	rtl_eth_hdr
	move	$20,$2

	move	$4,$17
	jal	rtl_eth_hdr
	move	$19,$2

	move	$4,$0
	andi	$5,$18,0xff
	lw	$3,16($16)
	addiu	$8,$sp,20
	srl	$6,$18,16
	sw	$5,16($sp)
	srl	$7,$18,8
	andi	$6,$6,0xff
	andi	$7,$7,0xff
	j	$L400
	srl	$5,$18,24

$L399:
	lbu	$9,0($11)
	addiu	$4,$4,1
	sb	$9,0($10)
$L400:
	sltu	$9,$4,14
	addu	$11,$23,$4
	bne	$9,$0,$L399
	addu	$10,$8,$4

	addiu	$22,$22,98
	move	$4,$0
	j	$L402
	addiu	$8,$sp,36

$L401:
	lbu	$9,0($11)
	addiu	$4,$4,1
	sb	$9,0($10)
$L402:
	sltu	$9,$4,14
	addu	$11,$22,$4
	bne	$9,$0,$L401
	addu	$10,$8,$4

	addiu	$21,$21,112
	move	$4,$0
	j	$L404
	addiu	$8,$sp,52

$L403:
	lbu	$9,0($11)
	addiu	$4,$4,1
	sb	$9,0($10)
$L404:
	sltu	$9,$4,14
	addu	$11,$21,$4
	bne	$9,$0,$L403
	addu	$10,$8,$4

	addiu	$20,$20,126
	move	$4,$0
	j	$L406
	addiu	$8,$sp,68

$L405:
	lbu	$9,0($11)
	addiu	$4,$4,1
	sb	$9,0($10)
$L406:
	sltu	$9,$4,14
	addu	$11,$20,$4
	bne	$9,$0,$L405
	addu	$10,$8,$4

	addiu	$19,$19,140
	move	$4,$0
	j	$L408
	addiu	$8,$sp,84

$L407:
	lbu	$9,0($11)
	addiu	$4,$4,1
	sb	$9,0($10)
$L408:
	sltu	$9,$4,14
	addu	$11,$19,$4
	bne	$9,$0,$L407
	addu	$10,$8,$4

	addiu	$2,$2,154
	move	$4,$0
	j	$L410
	addiu	$8,$sp,100

$L409:
	lbu	$9,0($11)
	addiu	$4,$4,1
	sb	$9,0($10)
$L410:
	sltu	$9,$4,14
	addu	$11,$2,$4
	bne	$9,$0,$L409
	addu	$10,$8,$4

	srl	$2,$3,16
	lui	$4,%hi($LC50)
	srl	$8,$3,8
	addiu	$4,$4,%lo($LC50)
	andi	$2,$2,0xff
	andi	$8,$8,0xff
	sw	$2,120($sp)
	andi	$2,$3,0xff
	sw	$8,124($sp)
	srl	$3,$3,24
	sw	$2,128($sp)
	jal	scrlog_printk
	sw	$3,116($sp)

	j	$L393
	nop

$L396:
	beq	$16,$0,$L393
	lui	$5,%hi(log_info)

	lw	$2,16($16)
	lw	$3,12($16)
	lui	$4,%hi($LC51)
	addiu	$5,$5,%lo(log_info)
	srl	$8,$2,16
	addiu	$4,$4,%lo($LC51)
	srl	$6,$3,16
	j	$L413
	sw	$5,36($sp)

$L417:
	beq	$16,$0,$L393
	lui	$4,%hi($LC52)

	lw	$2,16($16)
	addiu	$4,$4,%lo($LC52)
	lw	$3,12($16)
$L412:
	srl	$8,$2,16
	srl	$6,$3,16
$L413:
	srl	$5,$2,8
	srl	$7,$3,8
	andi	$8,$8,0xff
	andi	$5,$5,0xff
	sw	$8,24($sp)
	sw	$5,28($sp)
	andi	$8,$2,0xff
	srl	$5,$3,24
	sw	$8,32($sp)
	andi	$3,$3,0xff
	srl	$2,$2,24
	sw	$3,16($sp)
	andi	$6,$6,0x00ff
	sw	$2,20($sp)
	jal	scrlog_printk
	andi	$7,$7,0x00ff

$L393:
	lw	$31,180($sp)
	move	$2,$0
	lw	$23,176($sp)
	lw	$22,172($sp)
	lw	$21,168($sp)
	lw	$20,164($sp)
	lw	$19,160($sp)
	lw	$18,156($sp)
	lw	$17,152($sp)
	lw	$16,148($sp)
	j	$31
	addiu	$sp,$sp,184

	.set	macro
	.set	reorder
	.end	log_fastFilter
	.size	log_fastFilter, .-log_fastFilter
	.section	.text.do_filter,"ax",@progbits
	.align	2
	.globl	do_filter
	.set	nomips16
	.ent	do_filter
	.type	do_filter, @function
do_filter:
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
	lui	$2,%hi(table_list_head)
	move	$17,$4
	li	$20,-1			# 0xffffffff
	addiu	$19,$2,%lo(table_list_head)
	lw	$16,%lo(table_list_head)($2)
	j	$L419
	li	$18,1			# 0x1

$L424:
	jal	scan_table
	move	$4,$17

	beq	$2,$20,$L420
	nop

	bne	$2,$18,$L421
	lui	$3,%hi(log_enable)

	lw	$3,%lo(log_enable)($3)
	beq	$3,$0,$L423
	li	$2,1			# 0x1

	lw	$4,16($16)
	jal	log_fastFilter
	move	$5,$17

	j	$L423
	li	$2,1			# 0x1

$L421:
	lw	$16,0($16)
$L419:
	move	$6,$16
	move	$5,$0
	bne	$16,$19,$L424
	move	$7,$16

$L420:
	move	$2,$0
$L423:
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
	.end	do_filter
	.size	do_filter, .-do_filter
	.section	.text.FilterWeb_v2,"ax",@progbits
	.align	2
	.globl	FilterWeb_v2
	.set	nomips16
	.ent	FilterWeb_v2
	.type	FilterWeb_v2, @function
FilterWeb_v2:
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, gp= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$16,16($sp)
	jal	rtl_ip_hdr
	move	$16,$4

	jal	need_filter
	move	$4,$16

	bne	$2,$0,$L427
	nop

	lw	$31,20($sp)
	lw	$16,16($sp)
	j	$31
	addiu	$sp,$sp,24

$L427:
	move	$4,$16
	lw	$31,20($sp)
	lw	$16,16($sp)
	j	do_filter
	addiu	$sp,$sp,24

	.set	macro
	.set	reorder
	.end	FilterWeb_v2
	.size	FilterWeb_v2, .-FilterWeb_v2
	.section	.text.filter_enter,"ax",@progbits
	.align	2
	.globl	filter_enter
	.set	nomips16
	.ent	filter_enter
	.type	filter_enter, @function
filter_enter:
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
	lui	$2,%hi(item)
	lw	$2,%lo(item)($2)
	bne	$2,$0,$L431
	move	$17,$4

	lui	$2,%hi(table_list_head+24)
	lw	$2,%lo(table_list_head+24)($2)
	sltu	$2,$2,2
	beq	$2,$0,$L431
	lui	$3,%hi(none_dos_drop_pkt_cnt)

	j	$L506
	li	$2,1			# 0x1

$L431:
	jal	rtl_get_skb_type
	move	$4,$17

	bne	$2,$0,$L432
	li	$2,4			# 0x4

	jal	rtl_get_skb_protocol
	move	$4,$17

	li	$3,2048			# 0x800
	bne	$2,$3,$L432
	li	$2,4			# 0x4

	lui	$2,%hi(table_list_head+24)
	lw	$2,%lo(table_list_head+24)($2)
	sltu	$2,$2,2
	bne	$2,$0,$L514
	lui	$2,%hi(item)

	jal	FilterWeb_v2
	move	$4,$17

	beq	$2,$0,$L435
	lui	$3,%hi(fast_path_cusRsp401_func)

	lw	$2,%lo(fast_path_cusRsp401_func)($3)
	j	$L512
	nop

$L435:
	lw	$2,%lo(fast_path_cusRsp401_func)($3)
	beq	$2,$0,$L434
	lui	$3,%hi(url_cache)

	li	$2,1			# 0x1
	addiu	$3,$3,%lo(url_cache)
	lbu	$4,15($3)
	bne	$4,$2,$L437
	nop

	lui	$2,%hi(fast_path_cusRspTCPFinAck_func)
	lw	$2,%lo(fast_path_cusRspTCPFinAck_func)($2)
	j	$L512
	nop

$L437:
	lbu	$3,16($3)
	bne	$3,$2,$L514
	lui	$2,%hi(item)

	lui	$2,%hi(fast_path_cusRspTCPEndAck_func)
	lw	$2,%lo(fast_path_cusRspTCPEndAck_func)($2)
$L512:
	beq	$2,$0,$L436
	nop

	jalr	$2
	move	$4,$17

	j	$L432
	move	$2,$0

$L434:
	lui	$2,%hi(item)
$L514:
	lw	$2,%lo(item)($2)
	andi	$2,$2,0x1
	beq	$2,$0,$L438
	move	$4,$17

	lui	$18,%hi(wan_ppp_dev)
	jal	rtl_ip_hdr
	lui	$19,%hi(wan_dev)

	lui	$4,%hi($LC22)
	addiu	$4,$4,%lo($LC22)
	lbu	$20,0($2)
	jal	rtl_get_dev_by_name
	move	$16,$2

	lui	$4,%hi($LC23)
	addiu	$4,$4,%lo($LC23)
	andi	$20,$20,0xf
	jal	rtl_get_dev_by_name
	sw	$2,%lo(wan_ppp_dev)($18)

	move	$4,$17
	jal	rtl_get_skb_dev
	sw	$2,%lo(wan_dev)($19)

	beq	$2,$0,$L438
	nop

	jal	rtl_get_skb_dev
	move	$4,$17

	lw	$3,%lo(wan_dev)($19)
	beq	$3,$2,$L515
	lui	$2,%hi(item)

	jal	rtl_get_skb_dev
	move	$4,$17

	lw	$3,%lo(wan_ppp_dev)($18)
	bne	$3,$2,$L438
	lui	$2,%hi(item)

$L515:
	sll	$20,$20,2
	lw	$3,%lo(item)($2)
	andi	$2,$3,0x3fe
	beq	$2,$0,$L440
	addu	$20,$16,$20

	lw	$2,12($16)
	li	$4,36			# 0x24
	andi	$5,$2,0x3ff
	mult	$5,$4
	andi	$5,$2,0xff
	lui	$4,%hi(ConnectedIp)
	sll	$5,$5,2
	addiu	$4,$4,%lo(ConnectedIp)
	addu	$4,$5,$4
	lw	$5,0($4)
	lui	$4,%hi(dos_pkt)
	addiu	$4,$4,%lo(dos_pkt)
	mflo	$6
	bne	$2,$5,$L441
	addu	$4,$4,$6

	lw	$5,24($4)
	bne	$5,$2,$L441
	nop

	j	$L440
	sb	$0,0($4)

$L441:
	lw	$6,24($4)
	lb	$5,0($4)
	beq	$6,$2,$L442
	nop

	bne	$5,$0,$L440
	lui	$5,%hi(cur_p_pkt)

	sw	$4,%lo(cur_p_pkt)($5)
	sw	$2,24($4)
	li	$2,1			# 0x1
	j	$L443
	sb	$2,0($4)

$L442:
	li	$2,1			# 0x1
	bne	$5,$2,$L465
	move	$2,$0

	lui	$2,%hi(cur_p_pkt)
	j	$L443
	sw	$4,%lo(cur_p_pkt)($2)

$L500:
	lui	$2,%hi(cur_p_pkt)
	lbu	$4,9($16)
	lw	$5,%lo(cur_p_pkt)($2)
	li	$2,6			# 0x6
	bne	$4,$2,$L446
	move	$2,$0

	lbu	$4,13($20)
	li	$2,2			# 0x2
	andi	$4,$4,0x3f
	bne	$4,$2,$L446
	move	$2,$0

	lui	$2,%hi(block)
	lw	$4,%lo(block)($2)
	li	$2,1			# 0x1
	bne	$4,$2,$L445
	lui	$4,%hi(attack_saddr+4)

	lw	$6,%lo(attack_saddr+4)($4)
	lw	$4,12($16)
	beq	$6,$4,$L446
	li	$2,-1			# 0xffffffff

$L445:
	lw	$4,4($5)
	lui	$2,%hi(per_syn_threshold)
	addiu	$4,$4,1
	sw	$4,4($5)
	lw	$2,%lo(per_syn_threshold)($2)
	slt	$4,$2,$4
	beq	$4,$0,$L444
	andi	$4,$3,0x20

	beq	$4,$0,$L444
	nop

	blez	$2,$L444
	lui	$4,%hi(LogFlag)

	lw	$5,12($16)
	lui	$2,%hi(attack_saddr+4)
	lui	$6,%hi(block)
	sw	$5,%lo(attack_saddr+4)($2)
	li	$8,1			# 0x1
	lw	$5,%lo(LogFlag)($4)
	move	$2,$0
	lw	$7,16($16)
	ori	$5,$5,0x20
	sw	$8,%lo(block)($6)
	lui	$6,%hi(attack_daddr+4)
	sw	$7,%lo(attack_daddr+4)($6)
	j	$L446
	sw	$5,%lo(LogFlag)($4)

$L444:
	move	$2,$0
$L446:
	andi	$4,$3,0x44
$L534:
	beq	$4,$0,$L516
	andi	$4,$3,0x88

	lui	$4,%hi(cur_p_pkt)
	lbu	$5,9($16)
	lw	$6,%lo(cur_p_pkt)($4)
	li	$4,6			# 0x6
	bne	$5,$4,$L450
	move	$4,$0

	lbu	$4,13($20)
	li	$5,1			# 0x1
	andi	$4,$4,0x3f
	bne	$4,$5,$L448
	lui	$5,%hi(block)

	lw	$5,%lo(block)($5)
	bne	$5,$4,$L449
	lui	$5,%hi(attack_saddr+8)

	lw	$7,%lo(attack_saddr+8)($5)
	lw	$5,12($16)
	beq	$7,$5,$L450
	li	$4,-1			# 0xffffffff

$L449:
	lw	$5,8($6)
	lui	$4,%hi(per_fin_threshold)
	addiu	$5,$5,1
	sw	$5,8($6)
	lw	$4,%lo(per_fin_threshold)($4)
	slt	$5,$4,$5
	beq	$5,$0,$L448
	andi	$5,$3,0x40

	beq	$5,$0,$L448
	nop

	blez	$4,$L448
	lui	$5,%hi(LogFlag)

	lw	$6,12($16)
	lui	$4,%hi(attack_saddr+8)
	lui	$7,%hi(block)
	sw	$6,%lo(attack_saddr+8)($4)
	li	$9,1			# 0x1
	lw	$6,%lo(LogFlag)($5)
	move	$4,$0
	lw	$8,16($16)
	ori	$6,$6,0x40
	sw	$9,%lo(block)($7)
	lui	$7,%hi(attack_daddr+8)
	sw	$8,%lo(attack_daddr+8)($7)
	j	$L450
	sw	$6,%lo(LogFlag)($5)

$L448:
	move	$4,$0
$L450:
	or	$2,$2,$4
	andi	$4,$3,0x88
$L516:
	beq	$4,$0,$L517
	andi	$4,$3,0x110

	lui	$4,%hi(cur_p_pkt)
	lbu	$5,9($16)
	lw	$6,%lo(cur_p_pkt)($4)
	li	$4,17			# 0x11
	bne	$5,$4,$L454
	move	$4,$0

	lui	$4,%hi(block)
	lw	$5,%lo(block)($4)
	li	$4,1			# 0x1
	bne	$5,$4,$L453
	lui	$5,%hi(attack_saddr+12)

	lw	$7,%lo(attack_saddr+12)($5)
	lw	$5,12($16)
	beq	$7,$5,$L454
	li	$4,-1			# 0xffffffff

$L453:
	lw	$5,12($6)
	lui	$4,%hi(per_udp_threshold)
	addiu	$5,$5,1
	sw	$5,12($6)
	lw	$4,%lo(per_udp_threshold)($4)
	slt	$5,$4,$5
	beq	$5,$0,$L452
	andi	$5,$3,0x80

	beq	$5,$0,$L452
	nop

	blez	$4,$L452
	lui	$5,%hi(LogFlag)

	lw	$6,12($16)
	lui	$4,%hi(attack_saddr+12)
	lui	$7,%hi(block)
	sw	$6,%lo(attack_saddr+12)($4)
	li	$9,1			# 0x1
	lw	$6,%lo(LogFlag)($5)
	move	$4,$0
	lw	$8,16($16)
	ori	$6,$6,0x80
	sw	$9,%lo(block)($7)
	lui	$7,%hi(attack_daddr+12)
	sw	$8,%lo(attack_daddr+12)($7)
	j	$L454
	sw	$6,%lo(LogFlag)($5)

$L452:
	move	$4,$0
$L454:
	or	$2,$2,$4
	andi	$4,$3,0x110
$L517:
	beq	$4,$0,$L518
	andi	$4,$3,0x200

	lui	$5,%hi(cur_p_pkt)
	lbu	$4,9($16)
	lw	$6,%lo(cur_p_pkt)($5)
	li	$5,1			# 0x1
	bne	$4,$5,$L456
	lui	$5,%hi(block)

	lw	$5,%lo(block)($5)
	bne	$5,$4,$L457
	lui	$5,%hi(attack_saddr+16)

	lw	$7,%lo(attack_saddr+16)($5)
	lw	$5,12($16)
	beq	$7,$5,$L458
	li	$4,-1			# 0xffffffff

$L457:
	lw	$5,16($6)
	lui	$4,%hi(per_icmp_threshold)
	addiu	$5,$5,1
	sw	$5,16($6)
	lw	$4,%lo(per_icmp_threshold)($4)
	slt	$5,$4,$5
	beq	$5,$0,$L456
	andi	$5,$3,0x100

	beq	$5,$0,$L456
	nop

	blez	$4,$L456
	lui	$5,%hi(LogFlag)

	lw	$6,12($16)
	lui	$4,%hi(attack_saddr+16)
	lui	$7,%hi(block)
	sw	$6,%lo(attack_saddr+16)($4)
	li	$9,1			# 0x1
	lw	$6,%lo(LogFlag)($5)
	move	$4,$0
	lw	$8,16($16)
	ori	$6,$6,0x100
	sw	$9,%lo(block)($7)
	lui	$7,%hi(attack_daddr+16)
	sw	$8,%lo(attack_daddr+16)($7)
	j	$L458
	sw	$6,%lo(LogFlag)($5)

$L456:
	move	$4,$0
$L458:
	or	$2,$2,$4
	andi	$4,$3,0x200
$L518:
	beq	$4,$0,$L519
	li	$4,4194304			# 0x400000

	lbu	$6,9($16)
	lui	$4,%hi(cur_p_pkt)
	li	$5,6			# 0x6
	lw	$4,%lo(cur_p_pkt)($4)
	bne	$6,$5,$L520
	li	$5,17			# 0x11

	lhu	$5,32($4)
	beq	$5,$0,$L521
	nop

	lhu	$6,2($20)
	beq	$6,$5,$L521
	nop

	lw	$5,20($4)
	addiu	$5,$5,1
	sw	$5,20($4)
	lhu	$5,32($4)
$L521:
	bne	$5,$0,$L460
	nop

	lhu	$5,2($20)
	sh	$5,32($4)
$L460:
	lbu	$6,9($16)
	li	$5,17			# 0x11
$L520:
	bne	$6,$5,$L522
	li	$5,8388608			# 0x800000

	lhu	$5,32($4)
	beq	$5,$0,$L523
	nop

	lhu	$6,2($20)
	beq	$6,$5,$L523
	nop

	lw	$5,20($4)
	addiu	$5,$5,1
	sw	$5,20($4)
	lhu	$5,32($4)
$L523:
	bne	$5,$0,$L522
	li	$5,8388608			# 0x800000

	lhu	$5,2($20)
	sh	$5,32($4)
	li	$5,8388608			# 0x800000
$L522:
	and	$5,$3,$5
	lw	$4,20($4)
	beq	$5,$0,$L464
	nop

	j	$L508
	slt	$4,$4,11

$L464:
	slt	$4,$4,201
$L508:
	bne	$4,$0,$L519
	li	$4,4194304			# 0x400000

	lui	$4,%hi(LogFlag)
	lw	$5,%lo(LogFlag)($4)
	ori	$5,$5,0x200
	sw	$5,%lo(LogFlag)($4)
	lui	$4,%hi(attack_saddr+20)
	lw	$5,12($16)
	sw	$5,%lo(attack_saddr+20)($4)
	lui	$4,%hi(attack_daddr+20)
	lw	$5,16($16)
	sw	$5,%lo(attack_daddr+20)($4)
	li	$4,4194304			# 0x400000
$L519:
	and	$4,$3,$4
	bne	$4,$0,$L524
	andi	$4,$3,0x8000

$L440:
	move	$2,$0
$L465:
	andi	$4,$3,0x8000
$L524:
	beq	$4,$0,$L525
	li	$5,65536			# 0x10000

	lbu	$5,9($16)
	li	$4,6			# 0x6
	bne	$5,$4,$L468
	move	$4,$0

	lbu	$5,13($20)
	lui	$4,%hi(_tcpDosScanBitmap)
	addiu	$4,$4,%lo(_tcpDosScanBitmap)
	andi	$5,$5,0x3f
	addu	$4,$5,$4
	lb	$4,0($4)
	beq	$4,$0,$L467
	lui	$5,%hi(LogFlag)

	li	$4,-1			# 0xffffffff
	lw	$6,%lo(LogFlag)($5)
	ori	$6,$6,0x8000
	sw	$6,%lo(LogFlag)($5)
	lui	$5,%hi(attack_saddr+44)
	lw	$6,12($16)
	sw	$6,%lo(attack_saddr+44)($5)
	lui	$5,%hi(attack_daddr+44)
	lw	$6,16($16)
	j	$L468
	sw	$6,%lo(attack_daddr+44)($5)

$L467:
	move	$4,$0
$L468:
	or	$2,$2,$4
	li	$5,65536			# 0x10000
$L525:
	and	$4,$3,$5
	beq	$4,$0,$L526
	andi	$4,$3,0x800

	lbu	$6,9($16)
	li	$4,6			# 0x6
	bne	$6,$4,$L472
	move	$4,$0

	lbu	$6,13($20)
	li	$4,2			# 0x2
	andi	$6,$6,0x3f
	bne	$6,$4,$L472
	move	$4,$0

	lbu	$6,0($16)
	lw	$4,12($20)
	andi	$6,$6,0xf
	lhu	$7,2($16)
	sll	$6,$6,2
	srl	$4,$4,28
	subu	$6,$7,$6
	sll	$4,$4,2
	bne	$6,$4,$L527
	lui	$6,%hi(LogFlag)

	lhu	$4,6($16)
	andi	$4,$4,0x2000
	beq	$4,$0,$L470
	nop

$L527:
	li	$4,-1			# 0xffffffff
	lw	$7,%lo(LogFlag)($6)
	or	$5,$7,$5
	sw	$5,%lo(LogFlag)($6)
	lui	$5,%hi(attack_saddr+48)
	lw	$6,12($16)
	sw	$6,%lo(attack_saddr+48)($5)
	lui	$5,%hi(attack_daddr+48)
	lw	$6,16($16)
	j	$L472
	sw	$6,%lo(attack_daddr+48)($5)

$L470:
	move	$4,$0
$L472:
	or	$2,$2,$4
	andi	$4,$3,0x800
$L526:
	beq	$4,$0,$L528
	li	$4,262144			# 0x40000

	lw	$6,12($16)
	lw	$5,16($16)
	bne	$6,$5,$L475
	move	$4,$0

	lui	$5,%hi(LogFlag)
	li	$4,-1			# 0xffffffff
	lw	$6,%lo(LogFlag)($5)
	ori	$6,$6,0x800
	sw	$6,%lo(LogFlag)($5)
	lui	$5,%hi(attack_saddr+28)
	lw	$6,12($16)
	sw	$6,%lo(attack_saddr+28)($5)
	lui	$5,%hi(attack_daddr+28)
	lw	$6,16($16)
	sw	$6,%lo(attack_daddr+28)($5)
$L475:
	or	$2,$2,$4
	li	$4,262144			# 0x40000
$L528:
	and	$4,$3,$4
	beq	$4,$0,$L529
	li	$6,131072			# 0x20000

	lbu	$5,9($16)
	li	$4,17			# 0x11
	bne	$5,$4,$L479
	move	$4,$0

	lhu	$4,2($20)
	li	$7,7			# 0x7
	beq	$4,$7,$L478
	nop

	beq	$4,$5,$L478
	li	$6,19			# 0x13

	beq	$4,$6,$L478
	nop

	lhu	$4,0($20)
	beq	$4,$7,$L478
	nop

	beq	$4,$5,$L530
	lui	$5,%hi(LogFlag)

	bne	$4,$6,$L477
	nop

$L478:
	lui	$5,%hi(LogFlag)
$L530:
	li	$6,262144			# 0x40000
	lw	$7,%lo(LogFlag)($5)
	li	$4,-1			# 0xffffffff
	or	$6,$7,$6
	sw	$6,%lo(LogFlag)($5)
	lui	$5,%hi(attack_saddr+56)
	lw	$6,12($16)
	sw	$6,%lo(attack_saddr+56)($5)
	lui	$5,%hi(attack_daddr+56)
	lw	$6,16($16)
	j	$L479
	sw	$6,%lo(attack_daddr+56)($5)

$L477:
	move	$4,$0
$L479:
	or	$2,$2,$4
	li	$6,131072			# 0x20000
$L529:
	and	$4,$3,$6
	beq	$4,$0,$L531
	andi	$4,$3,0x4000

	lbu	$5,9($16)
	li	$4,17			# 0x11
	bne	$5,$4,$L482
	move	$4,$0

	lhu	$4,6($16)
	andi	$4,$4,0x3fff
	bne	$4,$0,$L482
	move	$4,$0

	lbu	$4,0($16)
	lhu	$7,2($16)
	andi	$4,$4,0xf
	lhu	$5,4($20)
	sll	$4,$4,2
	subu	$4,$7,$4
	slt	$4,$5,$4
	beq	$4,$0,$L481
	lui	$5,%hi(LogFlag)

	li	$4,-1			# 0xffffffff
	lw	$7,%lo(LogFlag)($5)
	or	$6,$7,$6
	sw	$6,%lo(LogFlag)($5)
	lui	$5,%hi(attack_saddr+52)
	lw	$6,12($16)
	sw	$6,%lo(attack_saddr+52)($5)
	lui	$5,%hi(attack_daddr+52)
	lw	$6,16($16)
	j	$L482
	sw	$6,%lo(attack_daddr+52)($5)

$L481:
	move	$4,$0
$L482:
	or	$2,$2,$4
	andi	$4,$3,0x4000
$L531:
	beq	$4,$0,$L532
	andi	$4,$3,0x400

	lhu	$4,6($16)
	andi	$5,$4,0x2000
	bne	$5,$0,$L484
	andi	$4,$4,0x1fff

	beq	$4,$0,$L484
	sll	$4,$4,3

	lbu	$6,0($16)
	lhu	$5,2($16)
	andi	$6,$6,0xf
	sll	$6,$6,2
	subu	$4,$4,$6
	addu	$4,$4,$5
	li	$5,65536			# 0x10000
	sltu	$4,$4,$5
	bne	$4,$0,$L484
	lui	$5,%hi(LogFlag)

	li	$4,-1			# 0xffffffff
	lw	$6,%lo(LogFlag)($5)
	ori	$6,$6,0x4000
	sw	$6,%lo(LogFlag)($5)
	lui	$5,%hi(attack_saddr+40)
	lw	$6,12($16)
	sw	$6,%lo(attack_saddr+40)($5)
	lui	$5,%hi(attack_daddr+40)
	lw	$6,16($16)
	j	$L485
	sw	$6,%lo(attack_daddr+40)($5)

$L484:
	move	$4,$0
$L485:
	or	$2,$2,$4
	andi	$4,$3,0x400
$L532:
	beq	$4,$0,$L486
	li	$5,1			# 0x1

	lbu	$4,0($16)
	lbu	$6,9($16)
	bne	$6,$5,$L487
	andi	$4,$4,0xf

	sll	$4,$4,2
	addu	$4,$16,$4
	lbu	$5,0($4)
	li	$4,8			# 0x8
	bne	$5,$4,$L488
	move	$4,$0

	lui	$4,%hi(lan_addr)
	lui	$5,%hi(lan_mask)
	lw	$6,%lo(lan_addr)($4)
	lw	$4,12($16)
	lw	$5,%lo(lan_mask)($5)
	xor	$4,$6,$4
	and	$4,$4,$5
	bne	$4,$0,$L487
	lui	$5,%hi(LogFlag)

	li	$4,-1			# 0xffffffff
	lw	$6,%lo(LogFlag)($5)
	ori	$6,$6,0x400
	sw	$6,%lo(LogFlag)($5)
	lui	$5,%hi(attack_saddr+24)
	lw	$6,12($16)
	sw	$6,%lo(attack_saddr+24)($5)
	lui	$5,%hi(attack_daddr+24)
	lw	$6,16($16)
	j	$L488
	sw	$6,%lo(attack_daddr+24)($5)

$L487:
	move	$4,$0
$L488:
	or	$2,$2,$4
$L486:
	andi	$4,$3,0x1000
	beq	$4,$0,$L489
	lui	$4,%hi(lan_addr)

	lui	$5,%hi(lan_mask)
	lw	$6,%lo(lan_addr)($4)
	lw	$4,12($16)
	lw	$5,%lo(lan_mask)($5)
	xor	$4,$6,$4
	and	$4,$4,$5
	bne	$4,$0,$L491
	move	$4,$0

	lbu	$5,9($16)
	li	$4,1			# 0x1
	beq	$5,$4,$L490
	lui	$5,%hi(LogFlag)

	li	$4,-1			# 0xffffffff
	lw	$6,%lo(LogFlag)($5)
	ori	$6,$6,0x1000
	sw	$6,%lo(LogFlag)($5)
	lui	$5,%hi(attack_saddr+32)
	lw	$6,12($16)
	sw	$6,%lo(attack_saddr+32)($5)
	lui	$5,%hi(attack_daddr+32)
	lw	$6,16($16)
	j	$L491
	sw	$6,%lo(attack_daddr+32)($5)

$L490:
	move	$4,$0
$L491:
	or	$2,$2,$4
$L489:
	andi	$3,$3,0x2000
	beq	$3,$0,$L492
	li	$4,36			# 0x24

	lw	$3,12($16)
	andi	$5,$3,0x3ff
	mult	$5,$4
	andi	$5,$3,0xff
	lui	$4,%hi(ConnectedIp)
	sll	$5,$5,2
	addiu	$4,$4,%lo(ConnectedIp)
	addu	$4,$5,$4
	lw	$5,0($4)
	lui	$4,%hi(dos_pkt)
	addiu	$4,$4,%lo(dos_pkt)
	mflo	$6
	bne	$3,$5,$L493
	addu	$4,$4,$6

	lw	$5,24($4)
	bne	$5,$3,$L493
	nop

	j	$L494
	sb	$0,0($4)

$L493:
	lw	$6,24($4)
	lb	$5,0($4)
	beq	$6,$3,$L495
	nop

	bne	$5,$0,$L494
	lui	$5,%hi(cur_p_pkt)

	sw	$4,%lo(cur_p_pkt)($5)
	sw	$3,24($4)
	li	$3,1			# 0x1
	j	$L496
	sb	$3,0($4)

$L495:
	li	$3,1			# 0x1
	bne	$5,$3,$L494
	lui	$3,%hi(cur_p_pkt)

	j	$L533
	sw	$4,%lo(cur_p_pkt)($3)

$L501:
	bne	$4,$0,$L499
	move	$4,$0

$L502:
	lhu	$4,6($16)
	andi	$6,$4,0x3fff
	beq	$6,$0,$L494
	sll	$7,$4,3

	lhu	$6,30($3)
	andi	$7,$7,0xffff
	slt	$7,$7,$6
	bne	$7,$0,$L497
	nop

	andi	$4,$4,0x2000
	bne	$4,$0,$L498
	nop

	sh	$0,30($3)
	j	$L499
	sh	$0,28($3)

$L498:
	sh	$5,28($3)
	move	$4,$0
	lbu	$5,0($16)
	lhu	$7,2($16)
	andi	$5,$5,0xf
	addu	$6,$7,$6
	sll	$5,$5,2
	subu	$6,$6,$5
	j	$L499
	sh	$6,30($3)

$L497:
	lui	$3,%hi(LogFlag)
	li	$4,-1			# 0xffffffff
	lw	$5,%lo(LogFlag)($3)
	ori	$5,$5,0x2000
	sw	$5,%lo(LogFlag)($3)
	lui	$3,%hi(attack_saddr+36)
	lw	$5,12($16)
	sw	$5,%lo(attack_saddr+36)($3)
	lui	$3,%hi(attack_daddr+36)
	lw	$5,16($16)
	j	$L499
	sw	$5,%lo(attack_daddr+36)($3)

$L494:
	move	$4,$0
$L499:
	or	$2,$2,$4
$L492:
	bne	$2,$0,$L432
	move	$2,$0

$L438:
	jal	rtl_ip_hdr
	move	$4,$17

	lbu	$3,9($2)
	li	$2,1			# 0x1
	beq	$3,$2,$L433
	lui	$3,%hi(none_dos_drop_pkt_cnt)

$L506:
	lw	$4,%lo(none_dos_drop_pkt_cnt)($3)
	addiu	$4,$4,1
	j	$L432
	sw	$4,%lo(none_dos_drop_pkt_cnt)($3)

$L433:
	j	$L432
	li	$2,4			# 0x4

$L436:
	move	$2,$0
$L432:
	lw	$31,36($sp)
	lw	$20,32($sp)
	lw	$19,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	j	$31
	addiu	$sp,$sp,40

$L443:
	andi	$2,$3,0x22
	bne	$2,$0,$L500
	move	$2,$0

	j	$L534
	andi	$4,$3,0x44

$L496:
	lui	$3,%hi(cur_p_pkt)
$L533:
	lw	$3,%lo(cur_p_pkt)($3)
	lhu	$5,4($16)
	lhu	$4,28($3)
	bne	$5,$4,$L501
	nop

	j	$L502
	nop

	.set	macro
	.set	reorder
	.end	filter_enter
	.size	filter_enter, .-filter_enter
	.section	.text.rtl_mac_filter_fastpath,"ax",@progbits
	.align	2
	.globl	rtl_mac_filter_fastpath
	.set	nomips16
	.ent	rtl_mac_filter_fastpath
	.type	rtl_mac_filter_fastpath, @function
rtl_mac_filter_fastpath:
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
	move	$16,$0
	move	$18,$4
	li	$17,6			# 0x6
	addu	$2,$5,$16
$L541:
	move	$4,$18
	lbu	$19,0($2)
	jal	rtl_skb_mac_header
	sw	$5,16($sp)

	addu	$2,$2,$16
	lbu	$2,6($2)
	lw	$5,16($sp)
	beq	$19,$2,$L536
	addiu	$16,$16,1

	j	$L537
	move	$2,$0

$L536:
	bne	$16,$17,$L541
	addu	$2,$5,$16

	li	$2,1			# 0x1
$L537:
	lw	$31,44($sp)
	lw	$19,40($sp)
	lw	$18,36($sp)
	lw	$17,32($sp)
	lw	$16,28($sp)
	j	$31
	addiu	$sp,$sp,48

	.set	macro
	.set	reorder
	.end	rtl_mac_filter_fastpath
	.size	rtl_mac_filter_fastpath, .-rtl_mac_filter_fastpath
	.section	.text.T.854,"ax",@progbits
	.align	2
	.set	nomips16
	.ent	T.854
	.type	T.854, @function
T.854:
	.frame	$sp,72,$31		# vars= 16, regs= 10/0, args= 16, gp= 0
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
	move	$16,$0
	move	$19,$4
	lw	$20,88($sp)
	move	$21,$5
	move	$17,$0
	lw	$18,92($sp)
	li	$8,13			# 0xd
	move	$fp,$6
	li	$3,10			# 0xa
	subu	$23,$5,$7
	move	$22,$4
	sw	$0,0($18)
	j	$L543
	sw	$0,0($20)

$L553:
	lb	$2,0($22)
	bne	$2,$8,$L544
	nop

	andi	$2,$16,0x1
	beq	$2,$0,$L546
	addiu	$16,$16,1

	j	$L548
	move	$16,$0

$L544:
	bne	$2,$3,$L545
	andi	$2,$16,0x1

	beq	$2,$0,$L545
	addiu	$16,$16,1

$L546:
	slt	$2,$16,4
	beq	$2,$0,$L547
	move	$4,$22

	j	$L556
	sw	$3,24($sp)

$L545:
	move	$16,$0
$L548:
	move	$4,$22
	sw	$3,24($sp)
$L556:
	addiu	$22,$22,1
	move	$6,$7
	sw	$7,16($sp)
	move	$5,$fp
	jal	memcmp
	sw	$8,20($sp)

	lw	$3,24($sp)
	lw	$7,16($sp)
	lw	$8,20($sp)
	beq	$2,$0,$L549
	move	$2,$0

	j	$L543
	addiu	$17,$17,1

$L549:
	addu	$17,$17,$7
	sw	$17,0($20)
	addu	$19,$19,$17
	j	$L550
	li	$3,13			# 0xd

$L551:
	bne	$5,$0,$L547
	addiu	$2,$2,1

$L550:
	lb	$4,0($19)
	addu	$5,$2,$17
	sltu	$5,$21,$5
	bne	$4,$3,$L551
	addiu	$19,$19,1

	sw	$2,0($18)
	j	$L552
	li	$2,1			# 0x1

$L543:
	sltu	$2,$23,$17
	beq	$2,$0,$L553
	nop

$L547:
	move	$2,$0
$L552:
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
	.end	T.854
	.size	T.854, .-T.854
	.section	.rodata.str1.4
	.align	2
$LC53:
	.ascii	"GET \000"
	.align	2
$LC54:
	.ascii	"Host: \000"
	.section	.text.rtl_url_filter_fastpath,"ax",@progbits
	.align	2
	.globl	rtl_url_filter_fastpath
	.set	nomips16
	.ent	rtl_url_filter_fastpath
	.type	rtl_url_filter_fastpath, @function
rtl_url_filter_fastpath:
	.frame	$sp,72,$31		# vars= 16, regs= 7/0, args= 24, gp= 0
	.mask	0x803f0000,-4
	.fmask	0x00000000,0
	addiu	$sp,$sp,-72
	sw	$31,68($sp)
	sw	$21,64($sp)
	sw	$20,60($sp)
	sw	$19,56($sp)
	sw	$18,52($sp)
	sw	$17,48($sp)
	sw	$16,44($sp)
	move	$16,$4
	move	$4,$5
	.set	noreorder
	.set	nomacro
	jal	strlen
	move	$18,$5
	.set	macro
	.set	reorder

	sltu	$2,$2,2
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L562
	move	$17,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	rtl_ip_hdr
	move	$4,$16
	.set	macro
	.set	reorder

	lbu	$3,9($2)
	move	$19,$2
	li	$2,6			# 0x6
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L562
	move	$17,$0
	.set	macro
	.set	reorder

$L559 = .
$L560 = .
	lbu	$21,0($19)
	lui	$2,%hi(malloc_sizes+68)
	lw	$4,%lo(malloc_sizes+68)($2)
	andi	$21,$21,0xf
	sll	$21,$21,2
	addu	$21,$19,$21
	lw	$17,12($21)
	.set	noreorder
	.set	nomacro
	jal	kmem_cache_alloc
	li	$5,208			# 0xd0
	.set	macro
	.set	reorder

	srl	$17,$17,28
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L558
	move	$16,$2
	.set	macro
	.set	reorder

	sll	$17,$17,2
	lbu	$20,0($19)
	lui	$5,%hi($LC53)
	addu	$17,$21,$17
	addiu	$5,$5,%lo($LC53)
	move	$4,$17
	li	$6,4			# 0x4
	lhu	$21,2($19)
	.set	noreorder
	.set	nomacro
	jal	memcmp
	andi	$20,$20,0xf
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L565
	lui	$6,%hi($LC54)
	.set	macro
	.set	reorder

	li	$2,-8			# 0xfffffff8
	move	$4,$17
	mult	$20,$2
	addiu	$19,$sp,24
	addiu	$2,$sp,28
	addiu	$6,$6,%lo($LC54)
	li	$7,6			# 0x6
	sw	$2,20($sp)
	mflo	$20
	sw	$19,16($sp)
	addu	$20,$20,$21
	.set	noreorder
	.set	nomacro
	jal	T.854
	move	$5,$20
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L576
	move	$4,$16
	.set	macro
	.set	reorder

	lw	$4,28($sp)
	lw	$3,24($sp)
	.set	noreorder
	.set	nomacro
	beq	$4,$0,$L564
	addu	$3,$17,$3
	.set	macro
	.set	reorder

	move	$2,$16
#APP
 # 50 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/string.h" 1
	.set	noreorder
	.set	noat
1:	lbu	$1,($3)
	subu	$4,1
	sb	$1,($2)
	beqz	$1,2f
	addiu	$2,1
	bnez	$4,1b
	addiu	$3,1
2:
	.set	at
	.set	reorder
 # 0 "" 2
#NO_APP
$L564:
	lw	$2,28($sp)
	lui	$6,%hi($LC53)
	move	$5,$20
	addiu	$6,$6,%lo($LC53)
	move	$4,$17
	li	$7,4			# 0x4
	addu	$2,$16,$2
	sb	$0,0($2)
	addiu	$2,$sp,32
	sw	$19,16($sp)
	.set	noreorder
	.set	nomacro
	jal	T.854
	sw	$2,20($sp)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L576
	move	$4,$16
	.set	macro
	.set	reorder

	lw	$3,32($sp)
	addiu	$3,$3,-9
	.set	noreorder
	.set	nomacro
	bgtz	$3,$L566
	sw	$3,32($sp)
	.set	macro
	.set	reorder

$L565:
	move	$4,$16
$L576:
	.set	noreorder
	.set	nomacro
	jal	kfree
	move	$17,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L575
	move	$2,$17
	.set	macro
	.set	reorder

$L566:
	lw	$2,28($sp)
	lw	$4,24($sp)
	addu	$2,$16,$2
	addu	$17,$17,$4
#APP
 # 50 "/home199/qian_yu/branch-rtl819x-sdk-v3.2/linux-2.6.30/arch/rlx/include/asm/string.h" 1
	.set	noreorder
	.set	noat
1:	lbu	$1,($17)
	subu	$3,1
	sb	$1,($2)
	beqz	$1,2f
	addiu	$2,1
	bnez	$3,1b
	addiu	$17,1
2:
	.set	at
	.set	reorder
 # 0 "" 2
#NO_APP
	lw	$2,32($sp)
	move	$4,$16
	lw	$3,28($sp)
	addu	$2,$16,$2
	addu	$2,$2,$3
	.set	noreorder
	.set	nomacro
	jal	strlen
	sb	$0,0($2)
	.set	macro
	.set	reorder

	move	$4,$18
	.set	noreorder
	.set	nomacro
	jal	strlen
	move	$17,$2
	.set	macro
	.set	reorder

	sltu	$17,$17,$2
	.set	noreorder
	.set	nomacro
	bne	$17,$0,$L568
	move	$20,$2
	.set	macro
	.set	reorder

	addu	$19,$16,$2
	.set	noreorder
	.set	nomacro
	j	$L569
	move	$17,$16
	.set	macro
	.set	reorder

$L571:
	move	$4,$17
	addiu	$19,$19,1
	addiu	$17,$17,1
	.set	noreorder
	.set	nomacro
	jal	memcmp
	move	$6,$20
	.set	macro
	.set	reorder

	bne	$2,$0,$L569
	.set	noreorder
	.set	nomacro
	j	$L570
	li	$17,1			# 0x1
	.set	macro
	.set	reorder

$L569:
	lb	$2,0($19)
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L571
	move	$5,$18
	.set	macro
	.set	reorder

$L568:
	move	$17,$0
$L570:
	lui	$6,%hi($LC27)
	lui	$4,%hi(log_info)
	move	$7,$16
	addiu	$6,$6,%lo($LC27)
	addiu	$4,$4,%lo(log_info)
	.set	noreorder
	.set	nomacro
	jal	snprintf
	li	$5,32			# 0x20
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	kfree
	move	$4,$16
	.set	macro
	.set	reorder

	lui	$2,%hi(url_filter_mode)
	lw	$2,%lo(url_filter_mode)($2)
	li	$3,1			# 0x1
	bne	$2,$3,$L562
	.set	noreorder
	.set	nomacro
	beq	$17,$0,$L572
	lui	$3,%hi(url_filter_flag)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L562
	li	$17,1			# 0x1
	.set	macro
	.set	reorder

$L572:
	.set	noreorder
	.set	nomacro
	j	$L562
	sw	$2,%lo(url_filter_flag)($3)
	.set	macro
	.set	reorder

$L558:
	move	$17,$0
$L562:
	move	$2,$17
$L575:
	lw	$31,68($sp)
	lw	$21,64($sp)
	lw	$20,60($sp)
	lw	$19,56($sp)
	lw	$18,52($sp)
	lw	$17,48($sp)
	lw	$16,44($sp)
	.set	noreorder
	.set	nomacro
	j	$31
	addiu	$sp,$sp,72
	.set	macro
	.set	reorder

	.end	rtl_url_filter_fastpath
	.size	rtl_url_filter_fastpath, .-rtl_url_filter_fastpath
	.globl	filter_tlb
	.data
	.align	2
	.type	filter_tlb, @object
	.size	filter_tlb, 40
filter_tlb:
	.word	0
	.word	0
	.word	1
	.word	rtl_ip_range_filter_fastpath
	.word	2
	.word	rtl_mac_filter_fastpath
	.word	3
	.word	rtl_url_filter_fastpath
	.word	4
	.word	rtl_schedule_filter_fastpath
	.globl	fast_path_cusRsp401_func
	.section	.bss,"aw",@nobits
	.align	2
	.type	fast_path_cusRsp401_func, @object
	.size	fast_path_cusRsp401_func, 4
fast_path_cusRsp401_func:
	.space	4
	.globl	fast_path_cusRspTCPFinAck_func
	.align	2
	.type	fast_path_cusRspTCPFinAck_func, @object
	.size	fast_path_cusRspTCPFinAck_func, 4
fast_path_cusRspTCPFinAck_func:
	.space	4
	.globl	fast_path_cusRspTCPEndAck_func
	.align	2
	.type	fast_path_cusRspTCPEndAck_func, @object
	.size	fast_path_cusRspTCPEndAck_func, 4
fast_path_cusRspTCPEndAck_func:
	.space	4
	.local	res3
	.comm	res3,4,4
	.section	.dram-l34-fwd,"aw",@progbits
	.align	2
	.type	item, @object
	.size	item, 4
item:
	.space	4
	.align	2
	.type	none_dos_drop_pkt_cnt, @object
	.size	none_dos_drop_pkt_cnt, 4
none_dos_drop_pkt_cnt:
	.space	4
	.type	url_count, @object
	.size	url_count, 1
url_count:
	.space	1
	.local	block_count
	.comm	block_count,4,4
	.local	block_time
	.comm	block_time,4,4
	.local	block
	.comm	block,4,4
	.local	item_backup
	.comm	item_backup,4,4
	.local	url_cache
	.comm	url_cache,20,4
	.local	endStateClientPort
	.comm	endStateClientPort,2,2
	.local	endStateClientSeq
	.comm	endStateClientSeq,4,4
	.local	finStateClientPort
	.comm	finStateClientPort,2,2
	.local	finStateClientSeq
	.comm	finStateClientSeq,4,4
	.local	block_message
	.comm	block_message,4,4
	.local	res12
	.comm	res12,4,4
	.local	log_enable
	.comm	log_enable,4,4
	.local	url_filter_mode
	.comm	url_filter_mode,4,4
	.local	url_filter_flag
	.comm	url_filter_flag,4,4
	.local	get_info_from_usr_space
	.comm	get_info_from_usr_space,1024,4
	.local	log_info
	.comm	log_info,32,4
	.local	filter_rule_num
	.comm	filter_rule_num,4,4
	.local	LogFlag
	.comm	LogFlag,4,4
	.local	ConnectedIp
	.comm	ConnectedIp,1024,4
	.local	dos_pkt
	.comm	dos_pkt,36864,4
	.local	cur_p_pkt
	.comm	cur_p_pkt,4,4
	.local	dos_timer
	.comm	dos_timer,24,4
	.local	_tcpDosScanBitmap
	.comm	_tcpDosScanBitmap,64,4
	.local	op_mode
	.comm	op_mode,1,1
	.local	whole_syn_threshold
	.comm	whole_syn_threshold,4,4
	.local	whole_fin_threshold
	.comm	whole_fin_threshold,4,4
	.local	whole_udp_threshold
	.comm	whole_udp_threshold,4,4
	.local	whole_icmp_threshold
	.comm	whole_icmp_threshold,4,4
	.local	per_syn_threshold
	.comm	per_syn_threshold,4,4
	.local	per_fin_threshold
	.comm	per_fin_threshold,4,4
	.local	per_udp_threshold
	.comm	per_udp_threshold,4,4
	.local	per_icmp_threshold
	.comm	per_icmp_threshold,4,4
	.local	lan_addr
	.comm	lan_addr,4,4
	.local	lan_mask
	.comm	lan_mask,4,4
	.local	attack_saddr
	.comm	attack_saddr,56,4
	.local	attack_daddr
	.comm	attack_daddr,56,4
	.local	dos_flag
	.comm	dos_flag,80,4
	.globl	table_list_head
	.section	.bss
	.align	2
	.type	table_list_head, @object
	.size	table_list_head, 32
table_list_head:
	.space	32
	.globl	wan_dev
	.align	2
	.type	wan_dev, @object
	.size	wan_dev, 4
wan_dev:
	.space	4
	.globl	wan_ppp_dev
	.align	2
	.type	wan_ppp_dev, @object
	.size	wan_ppp_dev, 4
wan_ppp_dev:
	.space	4
	.globl	___strtok
	.align	2
	.type	___strtok, @object
	.size	___strtok, 4
___strtok:
	.space	4
	.ident	"GCC: (GNU) 4.4.5-1.5.5p4"
