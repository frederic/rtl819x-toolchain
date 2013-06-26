#
# Makefile for the Linux/MIPS kernel.
#

extra-y		:= head.o init_task.o

obj-y		+= cpu-probe.o branch.o entry.o genex.o irq.o process.o \
		   ptrace.o reset.o setup.o signal.o  \
		   topology.o traps.o unaligned.o

obj-y      += rlx-switch.o syscall.o scall32-o32.o
obj-y      += rlx-time.o rlx-cevt.o

obj-$(CONFIG_STACKTRACE)	+= stacktrace.o
obj-$(CONFIG_MODULES)		+= mips_ksyms.o module.o
obj-$(CONFIG_IRQ_CPU)		+= irq_cpu.o
obj-$(CONFIG_IRQ_VEC)		+= irq_vec.o

obj-$(CONFIG_KGDB)		+= kgdb.o
obj-$(CONFIG_PROC_FS)		+= proc.o

obj-$(CONFIG_KEXEC)		+= machine_kexec.o relocate_kernel.o
obj-$(CONFIG_EARLY_PRINTK)	+= early_printk.o

EXTRA_CFLAGS += -Werror
EXTRA_CFLAGS += -I$(DIR_LINUX)/drivers/net/rtl819x/AsicDriver
EXTRA_CFLAGS += -I$(DIR_LINUX)/drivers/net/rtl819x/common
