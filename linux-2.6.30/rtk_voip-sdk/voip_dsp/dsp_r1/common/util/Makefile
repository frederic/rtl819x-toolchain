EXTRA_AFLAGS += -I$(DIR_VOIP)/voip_dsp/dsp_r1/include -fno-pic -mno-abicalls

#obj-m += codec_mem_api.o codec_mem.o

# 2.6 SDK don't build these .o 
ifneq "$(CONFIG_VOIP_SDK)" "y"
obj-y += dspfuncs.o dspfunc.o dspfunc_tab.o oper_32b.o oper_32bs.o pre_proc.o pre_procs.o \
	memset.o utility.o utilitys.o dmem_stack.o my_head.o
endif


EXTRA_CFLAGS += -DGNU
EXTRA_CFLAGS += -I$(DIR_VOIP)/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/dsp_r1/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/dsp_r1/common/basic_op

include $(DIR_VOIP)/rules.mak
