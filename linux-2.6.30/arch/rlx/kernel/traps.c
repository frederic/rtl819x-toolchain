/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1994 - 1999, 2000, 01, 06 Ralf Baechle
 * Copyright (C) 1995, 1996 Paul M. Antoine
 * Copyright (C) 1998 Ulf Carlsson
 * Copyright (C) 1999 Silicon Graphics, Inc.
 * Kevin D. Kissell, kevink@mips.com and Carsten Langgaard, carstenl@mips.com
 * Copyright (C) 2000, 01 MIPS Technologies, Inc.
 * Copyright (C) 2002, 2003, 2004, 2005, 2007  Maciej W. Rozycki
 */
#include <linux/bug.h>
#include <linux/compiler.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/smp.h>
#include <linux/spinlock.h>
#include <linux/kallsyms.h>
#include <linux/bootmem.h>
#include <linux/interrupt.h>
#include <linux/ptrace.h>
#include <linux/kgdb.h>
#include <linux/kdebug.h>

#include <asm/bootinfo.h>
#include <asm/branch.h>
#include <asm/break.h>
#include <asm/cpu.h>
#include <asm/rlxregs.h>
#include <asm/module.h>
#include <asm/pgtable.h>
#include <asm/ptrace.h>
#include <asm/sections.h>
#include <asm/system.h>
#include <asm/tlbdebug.h>
#include <asm/traps.h>
#include <asm/uaccess.h>
#include <asm/mmu_context.h>
#include <asm/types.h>
#include <asm/stacktrace.h>
#include <asm/irq.h>

#include <linux/time.h>
#include <linux/kernel_stat.h>
#include <asm/cputime.h>

#ifdef CONFIG_PANIC_PRINTK
#define printk panic_printk
#endif

extern asmlinkage void rlx_irq_dispatch(void);
extern asmlinkage void handle_tlbm(void);
extern asmlinkage void handle_tlbl(void);
extern asmlinkage void handle_tlbs(void);
extern asmlinkage void handle_adel(void);
extern asmlinkage void handle_ades(void);
extern asmlinkage void handle_sys(void);
extern asmlinkage void handle_bp(void);
extern asmlinkage void handle_ri(void);
extern asmlinkage void handle_cpu(void);
extern asmlinkage void handle_ov(void);
extern asmlinkage void handle_reserved(void);

static void show_raw_backtrace(unsigned long reg29)
{
	unsigned long *sp = (unsigned long *)(reg29 & ~3);
	unsigned long addr;

	printk("Call Trace:");
#ifdef CONFIG_KALLSYMS
	printk("\n");
#endif
	while (!kstack_end(sp)) {
		unsigned long __user *p =
			(unsigned long __user *)(unsigned long)sp++;
		if (__get_user(addr, p)) {
			printk(" (Bad stack address)");
			break;
		}
		if (__kernel_text_address(addr))
			print_ip_sym(addr);
	}
	printk("\n");
}

#ifdef CONFIG_KALLSYMS
int raw_show_trace;
static int __init set_raw_show_trace(char *str)
{
	raw_show_trace = 1;
	return 1;
}
__setup("raw_show_trace", set_raw_show_trace);
#endif

static void show_backtrace(struct task_struct *task, const struct pt_regs *regs)
{
	unsigned long sp = regs->regs[29];
	unsigned long ra = regs->regs[31];
	unsigned long pc = regs->cp0_epc;

	if (raw_show_trace || !__kernel_text_address(pc)) {
		show_raw_backtrace(sp);
		return;
	}
	printk("Call Trace:\n");
	do {
		print_ip_sym(pc);
		pc = unwind_stack(task, &sp, pc, &ra);
	} while (pc);
	printk("\n");
}

/*
 * This routine abuses get_user()/put_user() to reference pointers
 * with at least a bit of error checking ...
 */
static void show_stacktrace(struct task_struct *task, const struct pt_regs *regs)
{
	const int field = 2 * sizeof(unsigned long);
	long stackdata;
	int i;
	unsigned long __user *sp = (unsigned long __user *)regs->regs[29];

	printk("Stack :");
	i = 0;
	while ((unsigned long) sp & (PAGE_SIZE - 1)) {
		if (i && ((i % (64 / field)) == 0))
			printk("\n       ");
		if (i > 39) {
			printk(" ...");
			break;
		}

		if (__get_user(stackdata, sp++)) {
			printk(" (Bad stack address)");
			break;
		}

		printk(" %0*lx", field, stackdata);
		i++;
	}
	printk("\n");
	show_backtrace(task, regs);
}

void show_stack(struct task_struct *task, unsigned long *sp)
{
	struct pt_regs regs;
	if (sp) {
		regs.regs[29] = (unsigned long)sp;
		regs.regs[31] = 0;
		regs.cp0_epc = 0;
	} else {
		if (task && task != current) {
			regs.regs[29] = task->thread.reg29;
			regs.regs[31] = 0;
			regs.cp0_epc = task->thread.reg31;
		} else {
			prepare_frametrace(&regs);
		}
	}
	show_stacktrace(task, &regs);
}

void dump_uptime(void)
{
	struct timespec uptime;
	struct timespec idle;
	int i;
	cputime_t idletime = cputime_zero;
	
	for_each_possible_cpu(i)
		idletime = cputime64_add(idletime, kstat_cpu(i).cpustat.idle);
	
	do_posix_clock_monotonic_gettime(&uptime);
	monotonic_to_bootbased(&uptime);
	cputime_to_timespec(idletime, &idle);
	printk("uptime: %lu.%02lu %lu.%02lu\n",
			(unsigned long) uptime.tv_sec,
			(uptime.tv_nsec / (NSEC_PER_SEC / 100)),
			(unsigned long) idle.tv_sec,
			(idle.tv_nsec / (NSEC_PER_SEC / 100)));
}

/*
 * The architecture-independent dump_stack generator
 */
void dump_stack(void)
{
	struct pt_regs regs;

	dump_uptime();
	prepare_frametrace(&regs);
	show_backtrace(current, &regs);
}

EXPORT_SYMBOL(dump_stack);

static void show_code(unsigned int __user *pc)
{
	long i;
	unsigned short __user *pc16 = NULL;

	printk("\nCode:");

	if ((unsigned long)pc & 1)
		pc16 = (unsigned short __user *)((unsigned long)pc & ~1);
	for(i = -3 ; i < 6 ; i++) {
		unsigned int insn;
		if (pc16 ? __get_user(insn, pc16 + i) : __get_user(insn, pc + i)) {
			printk(" (Bad address in epc)\n");
			break;
		}
		printk("%c%0*x%c", (i?' ':'<'), pc16 ? 4 : 8, insn, (i?' ':'>'));
	}
}

static void __show_regs(const struct pt_regs *regs)
{
	const int field = 2 * sizeof(unsigned long);
	unsigned int cause = regs->cp0_cause;
	int i;

	printk("Cpu %d\n", smp_processor_id());

	/*
	 * Saved main processor registers
	 */
	for (i = 0; i < 32; ) {
		if ((i % 4) == 0)
			printk("$%2d   :", i);
		if (i == 0)
			printk(" %0*lx", field, 0UL);
		else if (i == 26 || i == 27)
			printk(" %*s", field, "");
		else
			printk(" %0*lx", field, regs->regs[i]);

		i++;
		if ((i % 4) == 0)
			printk("\n");
	}

	printk("Hi    : %0*lx\n", field, regs->hi);
	printk("Lo    : %0*lx\n", field, regs->lo);

	/*
	 * Saved cp0 registers
	 */
	printk("epc   : %0*lx %pS\n", field, regs->cp0_epc,
	       (void *) regs->cp0_epc);
	printk("    %s\n", print_tainted());
	printk("ra    : %0*lx %pS\n", field, regs->regs[31],
	       (void *) regs->regs[31]);

	printk("Status: %08x    ", (uint32_t) regs->cp0_status);

	if (regs->cp0_status & ST0_KUO) printk("KUo ");
	if (regs->cp0_status & ST0_IEO) printk("IEo ");
	if (regs->cp0_status & ST0_KUP) printk("KUp ");
	if (regs->cp0_status & ST0_IEP) printk("IEp ");
	if (regs->cp0_status & ST0_KUC) printk("KUc ");
	if (regs->cp0_status & ST0_IEC) printk("IEc ");

	printk("\n");

	printk("Cause : %08x\n", cause);

	cause = (cause & CAUSEF_EXCCODE) >> CAUSEB_EXCCODE;
	if (1 <= cause && cause <= 5)
		printk("BadVA : %0*lx\n", field, regs->cp0_badvaddr);

	printk("PrId  : %08x (%s)\n", read_c0_prid(), cpu_name_string());
}

/*
 * FIXME: really the generic show_regs should take a const pointer argument.
 */
void show_regs(struct pt_regs *regs)
{
	__show_regs((struct pt_regs *)regs);
}

void show_registers(const struct pt_regs *regs)
{
	const int field = 2 * sizeof(unsigned long);

	__show_regs(regs);
	print_modules();
	printk("Process %s (pid: %d, threadinfo=%p, task=%p, tls=%0*lx)\n",
	       current->comm, current->pid, current_thread_info(), current,
	      field, current_thread_info()->tp_value);

	show_stacktrace(current, regs);
	show_code((unsigned int __user *) regs->cp0_epc);
	printk("\n");
}

static DEFINE_SPINLOCK(die_lock);

void __noreturn die(const char * str, const struct pt_regs * regs)
{
	static int die_counter;

	console_verbose();
	spin_lock_irq(&die_lock);
	bust_spinlocks(1);
	printk("%s[#%d]:\n", str, ++die_counter);
	show_registers(regs);
	add_taint(TAINT_DIE);
#if defined(CONFIG_RTL_WTDOG)
	{ extern int is_fault; is_fault=1; } // set kernel fault flag	
#endif		
	spin_unlock_irq(&die_lock);

	if (in_interrupt())
		panic("Fatal exception in interrupt");

	if (panic_on_oops) {
		printk(KERN_EMERG "Fatal exception: panic in 5 seconds\n");
		ssleep(5);
		panic("Fatal exception");
	}

	do_exit(SIGSEGV);
}


#define OPCODE 0xfc000000
#define BASE   0x03e00000
#define RT     0x001f0000
#define OFFSET 0x0000ffff
#define LL     0xc0000000
#define SC     0xe0000000
#define SPEC0  0x00000000
#define SPEC3  0x7c000000
#define RD     0x0000f800
#define FUNC   0x0000003f
#define SYNC   0x0000000f
#define RDHWR  0x0000003b

/* ll/sc emulation */
#ifndef CONFIG_CPU_HAS_LLSC

/* The ll_bit is cleared by r*_switch.S */

unsigned long ll_bit;

static struct task_struct *ll_task = NULL;

static inline int simulate_ll(struct pt_regs *regs, unsigned int opcode)
{
	unsigned long value, __user *vaddr;
	long offset;

	/*
	 * analyse the ll instruction that just caused a ri exception
	 * and put the referenced address to addr.
	 */

	/* sign extend offset */
	offset = opcode & OFFSET;
	offset <<= 16;
	offset >>= 16;

	vaddr = (unsigned long __user *)
	        ((unsigned long)(regs->regs[(opcode & BASE) >> 21]) + offset);

	if ((unsigned long)vaddr & 3)
		return SIGBUS;
	if (get_user(value, vaddr))
		return SIGSEGV;

	preempt_disable();

	if (ll_task == NULL || ll_task == current) {
		ll_bit = 1;
	} else {
		ll_bit = 0;
	}
	ll_task = current;

	preempt_enable();

	regs->regs[(opcode & RT) >> 16] = value;

	return 0;
}

static inline int simulate_sc(struct pt_regs *regs, unsigned int opcode)
{
	unsigned long __user *vaddr;
	unsigned long reg;
	long offset;

	/*
	 * analyse the sc instruction that just caused a ri exception
	 * and put the referenced address to addr.
	 */

	/* sign extend offset */
	offset = opcode & OFFSET;
	offset <<= 16;
	offset >>= 16;

	vaddr = (unsigned long __user *)
	        ((unsigned long)(regs->regs[(opcode & BASE) >> 21]) + offset);
	reg = (opcode & RT) >> 16;

	if ((unsigned long)vaddr & 3)
		return SIGBUS;

	preempt_disable();

	if (ll_bit == 0 || ll_task != current) {
		regs->regs[reg] = 0;
		preempt_enable();
		return 0;
	}

	preempt_enable();

	if (put_user(regs->regs[reg], vaddr))
		return SIGSEGV;

	regs->regs[reg] = 1;

	return 0;
}

/*
 * ll uses the opcode of lwc0 and sc uses the opcode of swc0.  That is both
 * opcodes are supposed to result in coprocessor unusable exceptions if
 * executed on ll/sc-less processors.  That's the theory.  In practice a
 * few processors such as NEC's VR4100 throw reserved instruction exceptions
 * instead, so we're doing the emulation thing in both exception handlers.
 */
static int simulate_llsc(struct pt_regs *regs, unsigned int opcode)
{
	if ((opcode & OPCODE) == LL)
		return simulate_ll(regs, opcode);
	if ((opcode & OPCODE) == SC)
		return simulate_sc(regs, opcode);

	return -1;			/* Must be something else ... */
}
#endif

#ifndef CONFIG_CPU_HAS_SYNC
static int simulate_sync(struct pt_regs *regs, unsigned int opcode)
{
	if ((opcode & OPCODE) == SPEC0 && (opcode & FUNC) == SYNC)
		return 0;

	return -1;			/* Must be something else ... */
}
#endif

asmlinkage void do_ov(struct pt_regs *regs)
{
	siginfo_t info;

	die_if_kernel("Integer overflow", regs);

	info.si_code = FPE_INTOVF;
	info.si_signo = SIGFPE;
	info.si_errno = 0;
	info.si_addr = (void __user *) regs->cp0_epc;
	force_sig_info(SIGFPE, &info, current);
}

static void do_trap_or_bp(struct pt_regs *regs, unsigned int code, const char *str)
{
	siginfo_t info;
	char b[40];

	if (notify_die(DIE_TRAP, str, regs, code, 0, 0) == NOTIFY_STOP)
		return;

	/*
	 * A short test says that IRIX 5.3 sends SIGTRAP for all trap
	 * insns, even for trap and break codes that indicate arithmetic
	 * failures.  Weird ...
	 * But should we continue the brokenness???  --macro
	 */
	switch (code) {
	case BRK_OVERFLOW:
	case BRK_DIVZERO:
		scnprintf(b, sizeof(b), "%s instruction in kernel code", str);
		die_if_kernel(b, regs);
		if (code == BRK_DIVZERO)
			info.si_code = FPE_INTDIV;
		else
			info.si_code = FPE_INTOVF;
		info.si_signo = SIGFPE;
		info.si_errno = 0;
		info.si_addr = (void __user *) regs->cp0_epc;
		force_sig_info(SIGFPE, &info, current);
		break;
	case BRK_BUG:
		die_if_kernel("Kernel bug detected", regs);
		force_sig(SIGTRAP, current);
		break;
	default:
		scnprintf(b, sizeof(b), "%s instruction in kernel code", str);
		die_if_kernel(b, regs);
		force_sig(SIGTRAP, current);
	}
}

asmlinkage void do_bp(struct pt_regs *regs)
{
	unsigned int opcode, bcode;

	if (__get_user(opcode, (unsigned int __user *) exception_epc(regs)))
		goto out_sigsegv;

	/*
	 * There is the ancient bug in the MIPS assemblers that the break
	 * code starts left to bit 16 instead to bit 6 in the opcode.
	 * Gas is bug-compatible, but not always, grrr...
	 * We handle both cases with a simple heuristics.  --macro
	 */
	bcode = ((opcode >> 6) & ((1 << 20) - 1));
	if (bcode >= (1 << 10))
		bcode >>= 10;

	do_trap_or_bp(regs, bcode, "Break");
#if defined(CONFIG_RTL_WTDOG)
	if (bcode == 7){ // divided by zero
		die("Oops", regs);
		{ extern int is_fault; is_fault=1;} // set kernel fault flag
	}
#endif			
	return;

out_sigsegv:
	force_sig(SIGSEGV, current);
}

asmlinkage void do_ri(struct pt_regs *regs)
{
	unsigned int __user *epc = (unsigned int __user *)exception_epc(regs);
	unsigned long old_epc = regs->cp0_epc;
	unsigned int opcode = 0;
	int status = -1;

	if (notify_die(DIE_RI, "RI Fault", regs, SIGSEGV, 0, 0)
	    == NOTIFY_STOP)
		return;

	die_if_kernel("Reserved instruction in kernel code", regs);

	if (unlikely(compute_return_epc(regs) < 0))
		return;

	if (unlikely(get_user(opcode, epc) < 0))
		status = SIGSEGV;

#ifndef CONFIG_CPU_HAS_LLSC
	if (status < 0)
		status = simulate_llsc(regs, opcode);
#endif

#if 0
	if (status < 0)
		status = simulate_rdhwr(regs, opcode);
#endif

#ifndef CONFIG_CPU_HAS_SYNC 
	if (status < 0)
		status = simulate_sync(regs, opcode);
#endif

	if (status < 0)
		status = SIGILL;

	if (unlikely(status > 0)) {
		regs->cp0_epc = old_epc;		/* Undo skip-over.  */
		force_sig(status, current);
	}
}

asmlinkage void do_cpu(struct pt_regs *regs)
{
	unsigned int __user *epc;
	unsigned long old_epc;
	unsigned int opcode;
	unsigned int cpid;
	int status;
	unsigned long __maybe_unused flags;

	die_if_kernel("do_cpu invoked from kernel context!", regs);

	cpid = (regs->cp0_cause >> CAUSEB_CE) & 3;
    if (cpid == 0) 
      {
		epc = (unsigned int __user *)exception_epc(regs);
		old_epc = regs->cp0_epc;
		opcode = 0;
		status = -1;

		if (unlikely(compute_return_epc(regs) < 0))
			return;

		if (unlikely(get_user(opcode, epc) < 0))
			status = SIGSEGV;

#ifndef CONFIG_CPU_HAS_LLSC
		if (status < 0)
			status = simulate_llsc(regs, opcode);
#endif

#if 0
		if (status < 0)
			status = simulate_rdhwr(regs, opcode);
#endif

		if (status < 0)
			status = SIGILL;

		if (unlikely(status > 0)) {
			regs->cp0_epc = old_epc;	/* Undo skip-over.  */
			force_sig(status, current);
		}

		return;
      }

	force_sig(SIGILL, current);
}

asmlinkage void do_reserved(struct pt_regs *regs)
{
	/*
	 * Game over - no way to handle this if it ever occurs.  Most probably
	 * caused by a new unknown cpu type or after another deadly
	 * hard/software error.
	 */
	show_regs(regs);
	panic("Caught reserved exception %ld - should not happen.",
	      (regs->cp0_cause & 0x7f) >> 2);
}

unsigned long ebase;
unsigned long exception_handlers[32];

/*
 * As a side effect of the way this is implemented we're limited
 * to interrupt handlers in the address range from
 * KSEG0 <= x < KSEG0 + 256mb on the Nevada.  Oh well ...
 */
void *set_except_vector(int n, void *addr)
{
	unsigned long handler = (unsigned long) addr;
	unsigned long old_handler = exception_handlers[n];

	exception_handlers[n] = handler;
	return (void *)old_handler;
}

extern void cpu_cache_init(void);
extern void tlb_init(void);
extern void flush_tlb_handlers(void);

void __cpuinit per_cpu_trap_init(void)
{
	unsigned int cpu = smp_processor_id();
	unsigned int status_set = ST0_CU0;

    /* clear CU* and BEV, enable CU0 */
	change_c0_status(ST0_CU|ST0_BEV, status_set);

	cpu_data[cpu].asid_cache = ASID_FIRST_VERSION;
	TLBMISS_HANDLER_SETUP();

	atomic_inc(&init_mm.mm_count);
	current->active_mm = &init_mm;
	BUG_ON(current->mm);
	enter_lazy_tlb(&init_mm, current);

    cpu_cache_init();
    tlb_init();
}

#define RLX_TRAP_VEC_BASE    0x80000080
#define RLX_TRAP_VEC_SIZE    0x80

void __init trap_init(void)
{
	extern char rlx_trap_dispatch;
	unsigned long i;

#if defined(CONFIG_KGDB)
	if (kgdb_early_setup)
		return;	/* Already done */
#endif

    ebase = CAC_BASE;

	per_cpu_trap_init();

	/*
	 * Setup default vectors
	 */
	for (i = 0; i <= 31; i++)
		set_except_vector(i, handle_reserved);

	/*
	 * Initialise interrupt handlers
	 */
	set_except_vector(0, rlx_irq_dispatch);
	set_except_vector(1, handle_tlbm);
	set_except_vector(2, handle_tlbl);
	set_except_vector(3, handle_tlbs);

	set_except_vector(4, handle_adel);
	set_except_vector(5, handle_ades);

	set_except_vector(8, handle_sys);
	set_except_vector(9, handle_bp);
	set_except_vector(10, handle_ri);
	set_except_vector(11, handle_cpu);
	set_except_vector(12, handle_ov);

    memcpy ((void *) (RLX_TRAP_VEC_BASE), &rlx_trap_dispatch, RLX_TRAP_VEC_SIZE);

    flush_icache_range (RLX_TRAP_VEC_BASE, RLX_TRAP_VEC_BASE+RLX_TRAP_VEC_SIZE);
    flush_tlb_handlers ();
}
