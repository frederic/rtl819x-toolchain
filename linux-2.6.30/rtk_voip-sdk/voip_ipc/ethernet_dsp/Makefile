obj-y = ethernet_dsp_tx.o ethernet_dsp_rx.o

obj-$(CONFIG_RTK_VOIP_ETHERNET_DSP_IS_DSP) += ethernet_dsp_help_dsp.o

EXTRA_CFLAGS += -I$(DIR_VOIP)/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_ipc/

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

include $(DIR_VOIP)/rules.mak

