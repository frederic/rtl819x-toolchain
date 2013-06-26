/*
 * offset.c: Calculate pt_regs and task_struct offsets.
 *
 * Copyright (C) 1996 David S. Miller
 * Copyright (C) 1997, 1998, 1999, 2000, 2001, 2002, 2003 Ralf Baechle
 * Copyright (C) 1999, 2000 Silicon Graphics, Inc.
 *
 * Kevin Kissell, kevink@mips.com and Carsten Langgaard, carstenl@mips.com
 * Copyright (C) 2000 MIPS Technologies, Inc.
 */
#include <linux/compat.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/kbuild.h>
#include <asm/ptrace.h>
#include <asm/processor.h>

void output_ptreg_defines(void)
{
	COMMENT("MIPS pt_regs offsets.");
	OFFSET(PT_R0, pt_regs, regs[0]);
	OFFSET(PT_R1, pt_regs, regs[1]);
	OFFSET(PT_R2, pt_regs, regs[2]);
	OFFSET(PT_R3, pt_regs, regs[3]);
	OFFSET(PT_R4, pt_regs, regs[4]);
	OFFSET(PT_R5, pt_regs, regs[5]);
	OFFSET(PT_R6, pt_regs, regs[6]);
	OFFSET(PT_R7, pt_regs, regs[7]);
	OFFSET(PT_R8, pt_regs, regs[8]);
	OFFSET(PT_R9, pt_regs, regs[9]);
	OFFSET(PT_R10, pt_regs, regs[10]);
	OFFSET(PT_R11, pt_regs, regs[11]);
	OFFSET(PT_R12, pt_regs, regs[12]);
	OFFSET(PT_R13, pt_regs, regs[13]);
	OFFSET(PT_R14, pt_regs, regs[14]);
	OFFSET(PT_R15, pt_regs, regs[15]);
	OFFSET(PT_R16, pt_regs, regs[16]);
	OFFSET(PT_R17, pt_regs, regs[17]);
	OFFSET(PT_R18, pt_regs, regs[18]);
	OFFSET(PT_R19, pt_regs, regs[19]);
	OFFSET(PT_R20, pt_regs, regs[20]);
	OFFSET(PT_R21, pt_regs, regs[21]);
	OFFSET(PT_R22, pt_regs, regs[22]);
	OFFSET(PT_R23, pt_regs, regs[23]);
	OFFSET(PT_R24, pt_regs, regs[24]);
	OFFSET(PT_R25, pt_regs, regs[25]);
	OFFSET(PT_R26, pt_regs, regs[26]);
	OFFSET(PT_R27, pt_regs, regs[27]);
	OFFSET(PT_R28, pt_regs, regs[28]);
	OFFSET(PT_R29, pt_regs, regs[29]);
	OFFSET(PT_R30, pt_regs, regs[30]);
	OFFSET(PT_R31, pt_regs, regs[31]);
	OFFSET(PT_HI, pt_regs, hi);
	OFFSET(PT_LO, pt_regs, lo);
	OFFSET(PT_EPC, pt_regs, cp0_epc);
	OFFSET(PT_BVADDR, pt_regs, cp0_badvaddr);
	OFFSET(PT_STATUS, pt_regs, cp0_status);
	OFFSET(PT_CAUSE, pt_regs, cp0_cause);
#ifdef CONFIG_CPU_HAS_RADIAX
    OFFSET(PT_ESTATUS, pt_regs, estatus); 
    OFFSET(PT_ECAUSE, pt_regs, ecause);
    OFFSET(PT_INTVEC, pt_regs, intvec);
    OFFSET(PT_CBS0, pt_regs, radiax[0]);
    OFFSET(PT_CBS1, pt_regs, radiax[1]);
    OFFSET(PT_CBS2, pt_regs, radiax[2]);
    OFFSET(PT_CBE0, pt_regs, radiax[3]);
    OFFSET(PT_CBE1, pt_regs, radiax[4]);
    OFFSET(PT_CBE2, pt_regs, radiax[5]);
    OFFSET(PT_LPS0, pt_regs, radiax[6]);
    OFFSET(PT_LPE0, pt_regs, radiax[7]);
    OFFSET(PT_LPC0, pt_regs, radiax[8]);
    OFFSET(PT_MMD , pt_regs, radiax[9]);
    OFFSET(PT_M0LL, pt_regs, radiax[10]);
    OFFSET(PT_M0LH, pt_regs, radiax[11]);
    OFFSET(PT_M0HL, pt_regs, radiax[12]);
    OFFSET(PT_M0HH, pt_regs, radiax[13]);
    OFFSET(PT_M1LL, pt_regs, radiax[14]);
    OFFSET(PT_M1LH, pt_regs, radiax[15]);
    OFFSET(PT_M1HL, pt_regs, radiax[16]);
    OFFSET(PT_M1HH, pt_regs, radiax[17]);
    OFFSET(PT_M2LL, pt_regs, radiax[18]);
    OFFSET(PT_M2LH, pt_regs, radiax[19]);
    OFFSET(PT_M2HL, pt_regs, radiax[20]);
    OFFSET(PT_M2HH, pt_regs, radiax[21]);
    OFFSET(PT_M3LL, pt_regs, radiax[22]);
    OFFSET(PT_M3LH, pt_regs, radiax[23]);
    OFFSET(PT_M3HL, pt_regs, radiax[24]);
    OFFSET(PT_M3HH, pt_regs, radiax[25]);
#endif

	DEFINE(PT_SIZE, sizeof(struct pt_regs));
	BLANK();
}

void output_task_defines(void)
{
	COMMENT("MIPS task_struct offsets.");
	OFFSET(TASK_STATE, task_struct, state);
	OFFSET(TASK_THREAD_INFO, task_struct, stack);
	OFFSET(TASK_FLAGS, task_struct, flags);
	OFFSET(TASK_MM, task_struct, mm);
	OFFSET(TASK_PID, task_struct, pid);
	DEFINE(TASK_STRUCT_SIZE, sizeof(struct task_struct));
	BLANK();
}

void output_thread_info_defines(void)
{
	COMMENT("MIPS thread_info offsets.");
	OFFSET(TI_TASK, thread_info, task);
	OFFSET(TI_EXEC_DOMAIN, thread_info, exec_domain);
	OFFSET(TI_FLAGS, thread_info, flags);
	OFFSET(TI_TP_VALUE, thread_info, tp_value);
	OFFSET(TI_CPU, thread_info, cpu);
	OFFSET(TI_PRE_COUNT, thread_info, preempt_count);
	OFFSET(TI_ADDR_LIMIT, thread_info, addr_limit);
	OFFSET(TI_RESTART_BLOCK, thread_info, restart_block);
	OFFSET(TI_REGS, thread_info, regs);
	DEFINE(_THREAD_SIZE, THREAD_SIZE);
	DEFINE(_THREAD_MASK, THREAD_MASK);
	BLANK();
}

void output_thread_defines(void)
{
	COMMENT("MIPS specific thread_struct offsets.");
	OFFSET(THREAD_REG16, task_struct, thread.reg16);
	OFFSET(THREAD_REG17, task_struct, thread.reg17);
	OFFSET(THREAD_REG18, task_struct, thread.reg18);
	OFFSET(THREAD_REG19, task_struct, thread.reg19);
	OFFSET(THREAD_REG20, task_struct, thread.reg20);
	OFFSET(THREAD_REG21, task_struct, thread.reg21);
	OFFSET(THREAD_REG22, task_struct, thread.reg22);
	OFFSET(THREAD_REG23, task_struct, thread.reg23);
	OFFSET(THREAD_REG29, task_struct, thread.reg29);
	OFFSET(THREAD_REG30, task_struct, thread.reg30);
	OFFSET(THREAD_REG31, task_struct, thread.reg31);
	OFFSET(THREAD_STATUS, task_struct, thread.cp0_status);

	OFFSET(THREAD_BVADDR, task_struct, thread.cp0_badvaddr);
	OFFSET(THREAD_BUADDR, task_struct, thread.cp0_baduaddr);
	OFFSET(THREAD_ECODE, task_struct, thread.error_code);
	OFFSET(THREAD_TRAPNO, task_struct, thread.trap_no);
	BLANK();
}

void output_mm_defines(void)
{
	COMMENT("Size of struct page");
	DEFINE(STRUCT_PAGE_SIZE, sizeof(struct page));
	BLANK();
	COMMENT("Linux mm_struct offsets.");
	OFFSET(MM_USERS, mm_struct, mm_users);
	OFFSET(MM_PGD, mm_struct, pgd);
	OFFSET(MM_CONTEXT, mm_struct, context);
	BLANK();
	DEFINE(_PAGE_SIZE, PAGE_SIZE);
	DEFINE(_PAGE_SHIFT, PAGE_SHIFT);
	BLANK();
	DEFINE(_PGD_T_SIZE, sizeof(pgd_t));
	DEFINE(_PMD_T_SIZE, sizeof(pmd_t));
	DEFINE(_PTE_T_SIZE, sizeof(pte_t));
	BLANK();
	DEFINE(_PGD_T_LOG2, PGD_T_LOG2);
	DEFINE(_PMD_T_LOG2, PMD_T_LOG2);
	DEFINE(_PTE_T_LOG2, PTE_T_LOG2);
	BLANK();
	DEFINE(_PGD_ORDER, PGD_ORDER);
	DEFINE(_PMD_ORDER, PMD_ORDER);
	DEFINE(_PTE_ORDER, PTE_ORDER);
	BLANK();
	DEFINE(_PMD_SHIFT, PMD_SHIFT);
	DEFINE(_PGDIR_SHIFT, PGDIR_SHIFT);
	BLANK();
	DEFINE(_PTRS_PER_PGD, PTRS_PER_PGD);
	DEFINE(_PTRS_PER_PMD, PTRS_PER_PMD);
	DEFINE(_PTRS_PER_PTE, PTRS_PER_PTE);
	BLANK();
}

void output_sc_defines(void)
{
	COMMENT("Linux sigcontext offsets.");
	OFFSET(SC_REGS, sigcontext, sc_regs);
	OFFSET(SC_MDHI, sigcontext, sc_mdhi);
	OFFSET(SC_MDLO, sigcontext, sc_mdlo);
	OFFSET(SC_PC, sigcontext, sc_pc);
	BLANK();
}

void output_signal_defined(void)
{
	COMMENT("Linux signal numbers.");
	DEFINE(_SIGHUP, SIGHUP);
	DEFINE(_SIGINT, SIGINT);
	DEFINE(_SIGQUIT, SIGQUIT);
	DEFINE(_SIGILL, SIGILL);
	DEFINE(_SIGTRAP, SIGTRAP);
	DEFINE(_SIGIOT, SIGIOT);
	DEFINE(_SIGABRT, SIGABRT);
	DEFINE(_SIGEMT, SIGEMT);
	DEFINE(_SIGFPE, SIGFPE);
	DEFINE(_SIGKILL, SIGKILL);
	DEFINE(_SIGBUS, SIGBUS);
	DEFINE(_SIGSEGV, SIGSEGV);
	DEFINE(_SIGSYS, SIGSYS);
	DEFINE(_SIGPIPE, SIGPIPE);
	DEFINE(_SIGALRM, SIGALRM);
	DEFINE(_SIGTERM, SIGTERM);
	DEFINE(_SIGUSR1, SIGUSR1);
	DEFINE(_SIGUSR2, SIGUSR2);
	DEFINE(_SIGCHLD, SIGCHLD);
	DEFINE(_SIGPWR, SIGPWR);
	DEFINE(_SIGWINCH, SIGWINCH);
	DEFINE(_SIGURG, SIGURG);
	DEFINE(_SIGIO, SIGIO);
	DEFINE(_SIGSTOP, SIGSTOP);
	DEFINE(_SIGTSTP, SIGTSTP);
	DEFINE(_SIGCONT, SIGCONT);
	DEFINE(_SIGTTIN, SIGTTIN);
	DEFINE(_SIGTTOU, SIGTTOU);
	DEFINE(_SIGVTALRM, SIGVTALRM);
	DEFINE(_SIGPROF, SIGPROF);
	DEFINE(_SIGXCPU, SIGXCPU);
	DEFINE(_SIGXFSZ, SIGXFSZ);
	BLANK();
}

void output_irq_cpustat_t_defines(void)
{
	COMMENT("Linux irq_cpustat_t offsets.");
	DEFINE(IC_SOFTIRQ_PENDING, offsetof(irq_cpustat_t, __softirq_pending));
	DEFINE(IC_IRQ_CPUSTAT_T, sizeof(irq_cpustat_t));
	BLANK();
}
