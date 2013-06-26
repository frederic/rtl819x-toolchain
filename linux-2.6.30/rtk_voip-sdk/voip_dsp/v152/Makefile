EXTRA_AFLAGS += -I$(DIR_VOIP)/voip_dsp/dsp_r1/include  -fno-pic
		
obj-y    := v152_api.o v152_proc.o



EXTRA_CFLAGS += -I$(DIR_VOIP)/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/rtp

ifeq ($(CONFIG_RTK_VOIP),m)
EXTRA_CFLAGS += -mlong-calls
EXTRA_CFLAGS += -DMODULE
endif

include $(DIR_VOIP)/rules.mak
