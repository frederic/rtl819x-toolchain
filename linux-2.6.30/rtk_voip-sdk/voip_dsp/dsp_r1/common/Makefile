#
# Makefile for dsp common routines
#
#	./basic_op	: ITU basic operations.
#	./util		: memory, stack, IMEM/DMEM operations.
#
EXTRA_AFLAGS += -I$(DIR_VOIP)/voip_dsp/dsp_r1/include -fno-pic -mno-abicalls

subdir-y := 
obj-y    :=

export-objs = 

ifeq ($(CONFIG_RTK_VOIP),m)
obj-y += 	util/dspfuncs.o util/dspfunc.o util/dspfunc_tab.o util/oper_32b.o util/oper_32bs.o util/pre_proc.o util/pre_procs.o \
			util/memset.o util/utility.o util/utilitys.o util/dmem_stack.o util/my_head.o
ifneq ($(CONFIG_VOIP_SDK),y)
obj-y +=	basic_op/basic_op.o basic_op/basic_ops.o basic_op/basop_inline.o
endif
else
obj-y += 	util/
ifneq ($(CONFIG_VOIP_SDK),y)
obj-y += 	basic_op/
endif
endif

EXTRA_CFLAGS += -DGNU
EXTRA_CFLAGS += -I$(DIR_VOIP)/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/dsp_r1
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/dsp_r1/include
ifneq ($(CONFIG_VOIP_SDK),y)
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/dsp_r1/common/basic_op
endif
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/dsp_r1/common/util

ifeq ($(CONFIG_RTK_VOIP),m)
EXTRA_CFLAGS += -mlong-calls
EXTRA_CFLAGS += -DMODULE
endif

include $(DIR_VOIP)/rules.mak
