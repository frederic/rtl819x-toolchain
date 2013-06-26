obj-y = coprocessor_dsp_tx.o coprocessor_dsp_rx.o 

obj-$(CONFIG_RTK_VOIP_COPROCESS_DSP_IS_HOST) += cpu/
obj-$(CONFIG_RTK_VOIP_COPROCESS_DSP_IS_DSP) += dsp/

EXTRA_CFLAGS += -I$(DIR_VOIP)/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_ipc/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_ipc/coprocessor_dsp/include/

EXTRA_CFLAGS-$(CONFIG_RTK_VOIP_COPROCESS_DSP_IS_HOST) += -I$(DIR_VOIP)/voip_ipc/coprocessor_dsp/cpu/include/
EXTRA_CFLAGS-$(CONFIG_RTK_VOIP_COPROCESS_DSP_IS_DSP) += -I$(DIR_VOIP)/voip_ipc/coprocessor_dsp/dsp/include/

EXTRA_CFLAGS += $(EXTRA_CFLAGS-y)

EXTRA_LDFLAGS += -T$(DIR_VOIP)/voip_ipc/coprocessor_dsp/coprocessor_shm.lds

ifeq ($(CONFIG_RTK_VOIP),m)
EXTRA_CFLAGS += -mlong-calls
EXTRA_CFLAGS += -DMODULE
endif

EXTRA_CFLAGS += -DMODULE_NAME=\"IPC\"

include $(DIR_VOIP)/rules.mak

