/*
#*******************************************************************************
# Copyright (C) 2006 PMC-Sierra Inc.  All Rights Reserved.
#-------------------------------------------------------------------------------
# This software embodies materials and concepts which are proprietary and
# confidential to PMC-Sierra, Inc.  PMC-Sierra distributes this software to
# its customers pursuant to the terms and conditions of the Software License
# Agreement contained in the text file software.lic that is distributed along
# with the software.  This software can only be utilized if all terms and
# conditions of the Software License Agreement are accepted.  If there are
# any questions, concerns, or if the Software License Agreement text file
# software.lic is missing, please contact PMC-Sierra for assistance.
#-------------------------------------------------------------------------------
# $RCSfile: dallas_util.c,v $
#
# $Date: 2008-07-07 02:43:17 $
#
# $Revision: 1.1 $
#-------------------------------------------------------------------------------
# 
#-------------------------------------------------------------------------------
*/

#include <linux/init.h>
#include <asm/processor.h>
#include <asm/uaccess.h>
#include <linux/ctype.h>
#include <linux/proc_fs.h>
#include <linux/completion.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
#include <linux/mm_types.h>
#include <trace/sched.h>
#endif
#include "dallas_util.h"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
static inline void clear_freeze_flag(struct task_struct *p)
{
clear_tsk_thread_flag(p, TIF_FREEZE);

}
#endif
/*
 *  exit_mm() is not exported by SDK.
 *  We had to copy the code here for implementing dallas_daemonize().
 */
static void dallas_exit_mm(struct task_struct * tsk)
{
	struct mm_struct *mm = tsk->mm;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
	struct core_state *core_state;
#endif
	
	if (!mm)
		return;
	
	down_read(&mm->mmap_sem);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
	core_state = mm->core_state;
	if (core_state) {
		struct core_thread self;
		up_read(&mm->mmap_sem);

		self.task = tsk;
		self.next = xchg(&core_state->dumper.next, &self);
		/*
		 * Implies mb(), the result of xchg() must be visible
		 * to core_state->dumper.
		 */
		if (atomic_dec_and_test(&core_state->nr_threads))
			complete(&core_state->startup);

		for (;;) {
			set_task_state(tsk, TASK_UNINTERRUPTIBLE);
			if (!self.task) /* see coredump_finish() */
				break;
			schedule();
		}
		__set_task_state(tsk, TASK_RUNNING);
		down_read(&mm->mmap_sem);
	}
#else
	if (mm->core_waiters) {
		up_read(&mm->mmap_sem);
		down_write(&mm->mmap_sem);
		if (!--mm->core_waiters)
			complete(mm->core_startup_done);
		up_write(&mm->mmap_sem);
		
		wait_for_completion(&mm->core_done);
		down_read(&mm->mmap_sem);
	}
#endif
	atomic_inc(&mm->mm_count);
	BUG_ON(mm != tsk->active_mm);
	/* more a memory barrier than a real lock */
	task_lock(tsk);
	tsk->mm = NULL;
	up_read(&mm->mmap_sem);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
clear_freeze_flag(tsk);
#endif
	task_unlock(tsk);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
mm_update_next_owner(mm);
#endif
	mmput(mm);
}

/*
 *      Put all the gunge required to become a kernel thread without
 *      attached user resources in one place where it belongs.
 *  We cannot call daemonize() here because we need share
 *  the FS and FILES w/ the init task.
 */
static void dallas_daemonize(const char *name, ...)
{
	va_list args;
	
	va_start(args, name);
	vsnprintf(current->comm, sizeof(current->comm), name, args);
	va_end(args);
	
	/*
	 * If we were started as result of loading a module, close all of the
	 * user space pages.  We don't need them, and if we didn't close them
	 * they would be locked into memory.
	 */
	dallas_exit_mm(current);
}
 
static int call_kernel_thread_rt(void *p)
{
	kthread_rt_data_t *param = (kthread_rt_data_t *)p;
	void *( *start_routine ) ( void * );
	void *arg = param->arg;
	
	start_routine = param->start_routine;
	
	dallas_daemonize("thr_%s", param->name);
	complete(&(param->done));
	
	return (int) start_routine(arg);
}

#if 1
static inline struct task_struct *find_process_by_pid(pid_t pid)
{
	return pid ? find_task_by_vpid(pid) : current;
}
static int
do_sched_setscheduler(pid_t pid, int policy, struct sched_param __user *param)
{
	struct sched_param lparam;
	struct task_struct *p;
	int retval;

	if (!param || pid < 0)
		return -EINVAL;
	if (copy_from_user(&lparam, param, sizeof(struct sched_param)))
		return -EFAULT;

	rcu_read_lock();
	retval = -ESRCH;
	p = find_process_by_pid(pid);
	if (p != NULL)
		retval = sched_setscheduler(p, policy, &lparam);
	rcu_read_unlock();

	return retval;
}
#endif

/*
 * Create real-time kernel thread
 */
int kernel_thread_rt(char *name, pid_t *tid, int prio, void *(*start_routine)(void*), void *arg)
{
	int ret;
	int name_len;
	kthread_rt_data_t data;
	struct sched_param sched_data;
	mm_segment_t oldfs = get_ds();
	
	data.start_routine = start_routine;
	data.arg = arg;
	
	if (name != NULL) {
		name_len = strlen(name);
		if (name_len > KTHREAD_RT_NAME_LEN)
		    name_len = KTHREAD_RT_NAME_LEN;
		memcpy(data.name, name, name_len); /* Can't use mem_cpy */
	} else {
		name_len = 7;
		memcpy(data.name, "Default", name_len);
	}
	data.name[name_len] = 0;
	
	init_completion(&(data.done));
	*tid = kernel_thread(call_kernel_thread_rt, &data, 0);
	if(*tid < 0)
		return -EAGAIN;
	wait_for_completion(&(data.done));
	sched_data.sched_priority = prio;
	
	set_fs(KERNEL_DS);
	ret = do_sched_setscheduler(*tid, SCHED_RR, &sched_data);
	set_fs(oldfs);
	
	return ret;
}

int set_hw_addr (char buf[], char *str)
{
	int i;
	char c, val;

	for(i = 0; i < 6; i++)
	{
		if (!(c = tolower(*str++)))
		{
			printk("Invalid hardware address\n");
			return (-1);
		} if (isdigit(c)){
			val = c - '0';
		} else if (c >= 'a' && c <= 'f') {
			val = c-'a'+10;
		} else {
			printk("Invalid hardware address\n");
			return (-1);
		}
		buf[i] = val << 4;
		if (!(c = tolower(*str++))) {
			printk("Invalid hardware address\n");
			return (-1);
		}
		if (isdigit(c)) {
			val = c - '0';
		} else if (c >= 'a' && c <= 'f') {
			val = c-'a'+10;
		} else {
			printk("Invalid hardware address\n");
			return (-1);
		}
		buf[i] |= val;
		if (*str == ':')
			str++;
	}
	return 0;
}
