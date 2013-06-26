obj-y = 

obj-$(CONFIG_RTK_VOIP_IPC_ARCH) += ipc_arch_tx.o
obj-$(CONFIG_RTK_VOIP_IPC_ARCH) += ipc_arch_rx.o

obj-$(CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP)  += ipc_arch_help_dsp.o

obj-$(CONFIG_RTK_VOIP_ETHERNET_DSP)  += ethernet_dsp/
obj-$(CONFIG_RTK_VOIP_COPROCESS_DSP) += coprocessor_dsp/

EXTRA_CFLAGS += -I$(DIR_VOIP)/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/dsp_r1/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_manager
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/

ifeq ($(CONFIG_RTK_VOIP_T38),y)
ifneq ($(CONFIG_VOIP_SDK),y)
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/dsp_r1/t38/rly_src
endif
endif

ifeq ($(CONFIG_RTK_VOIP),m)
EXTRA_CFLAGS += -mlong-calls
EXTRA_CFLAGS += -DMODULE
endif

ifeq "$(CONFIG_AUDIOCODES_VOIP)" "y"
EXTRA_CFLAGS += -DAUDIOCODES_VOIP
EXTRA_CFLAGS += -DACMW_LX
EXTRA_CFLAGS += -DACL_BYPASS
EXTRA_CFLAGS += -DACL_RELEASE
EXTRA_CFLAGS += -DAUDIOCODES_VOIP_SIP
endif

EXTRA_CFLAGS += -DMODULE_NAME=\"IPC\"


hostprogs-y = ipc_arch_viewer 
always = $(hostprogs-y)
HOST_EXTRACFLAGS += -I$(DIR_VOIP)/include
HOST_EXTRACFLAGS += -I$(DIR_LINUX)/include

include $(DIR_VOIP)/rules.mak

