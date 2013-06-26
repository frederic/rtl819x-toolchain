#ifndef __DMEM_STACK_H__
#define __DMEM_STACK_H__

#include "rtk_voip.h"

extern void entry_dmem_stack(unsigned long* orig_sp, unsigned long* dmem_sp);
extern void leave_dmem_stack(unsigned long* orig_sp);

#define SYS_DUMMY_SSIZE 2048
#define SYS_DMEM_SSIZE 1024

extern unsigned long sys_dmem_stack[SYS_DMEM_SSIZE];
extern unsigned long sys_orig_sp;
extern unsigned long sys_dmem_sp;

extern unsigned long __sys_dmem_start;


#endif /* __DMEM_STACK_H__ */
