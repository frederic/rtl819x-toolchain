EXTRA_AFLAGS += -I$(DIR_VOIP)/voip_dsp/dsp_r1/include  -fno-pic

obj-y    :=
#CONFIG_VOIP_SDK=y

ifeq ($(CONFIG_VOIP_SDK),y)
obj-y += ivr.obj 
ifeq ($(CONFIG_RTK_VOIP_IVR_DEV),y)
obj-y += ivr_dev.obj
endif
ifeq ($(CONFIG_RTK_VOIP_G7231),y)
ifneq ($(CONFIG_AUDIOCODES_VOIP),y)
obj-y += ivr_g723sup.obj 
endif
endif
ifeq ($(CONFIG_RTK_VOIP_G729AB),y)
ifneq ($(CONFIG_AUDIOCODES_VOIP),y)
obj-y += ivr_g729sup.obj 
endif
endif
ifneq ($(CONFIG_AUDIOCODES_VOIP),y)
obj-y += ivr_g711sup.obj 
endif
ifeq ($(CONFIG_RTK_VOIP_IVR_LINEAR_8K),y)
obj-y += ivr_linear8ksup.obj
endif
ifeq ($(CONFIG_RTK_VOIP_IVR_LINEAR_16K),y)
obj-y += ivr_linear16ksup.obj
endif
else
obj-y += ivr.o 
ifeq ($(CONFIG_RTK_VOIP_IVR_DEV),y)
obj-y += ivr_dev.o
endif
ifeq ($(CONFIG_RTK_VOIP_G7231),y)
ifneq ($(CONFIG_AUDIOCODES_VOIP),y)
obj-y += ivr_g723sup.o 
endif
endif
ifeq ($(CONFIG_RTK_VOIP_G729AB),y)
ifneq ($(CONFIG_AUDIOCODES_VOIP),y)
obj-y += ivr_g729sup.o 
endif
endif
ifneq ($(CONFIG_AUDIOCODES_VOIP),y)
obj-y += ivr_g711sup.o 
endif
ifeq ($(CONFIG_RTK_VOIP_IVR_LINEAR_8K),y)
obj-y += ivr_linear8ksup.o
endif
ifeq ($(CONFIG_RTK_VOIP_IVR_LINEAR_16K),y)
obj-y += ivr_linear16ksup.o
endif
endif


obj-y += ivr_dummy.o

ifeq ($(CONFIG_RTK_VOIP_IVR_TEXT),y)
obj-y += ivr_speech.o
endif

EXTRA_CFLAGS += -I$(DIR_VOIP)/include

ifneq ($(CONFIG_AUDIOCODES_VOIP),y)
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/dsp_r1
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/dsp_r1/include


ifeq ($(CONFIG_RTK_VOIP_G729AB),y)
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/dsp_r1/common/util
endif
endif
ifeq "$(CONFIG_AUDIOCODES_VOIP)" "y"
EXTRA_CFLAGS += -DAUDIOCODES_VOIP
EXTRA_CFLAGS += -DACMW_LX
EXTRA_CFLAGS += -DACL_BYPASS
EXTRA_CFLAGS += -DACL_RELEASE
EXTRA_CFLAGS += -DAUDIOCODES_VOIP_SIP
endif


ifeq ($(CONFIG_RTK_VOIP),m)
EXTRA_CFLAGS += -mlong-calls
EXTRA_CFLAGS += -DMODULE
endif

include $(DIR_VOIP)/rules.mak
