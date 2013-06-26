export CONFIG_VOIP_SDK=y

ifeq "$(VERSION).$(PATCHLEVEL)" "2.4"
DIR_VOIP := $(shell /bin/pwd)
O_TARGET := rtk_voip.o
else
DIR_VOIP := $(src)
endif

export DIR_VOIP

obj-y := main.o

ifneq "$(CONFIG_RTK_VOIP_DRIVERS_11N_MP)" "y"

obj-$(CONFIG_RTK_VOIP_RX) += voip_rx/
obj-$(CONFIG_RTK_VOIP_TX) += voip_tx/
obj-$(CONFIG_RTK_VOIP_SRTP) += srtp/
obj-ipchost-y-sdk-y += voip_dsp/cpu_setup.obj
obj-ipchost-y-sdk-  += voip_dsp/cpu_setup.o
obj-ipchost--ac-y-sdk-y += voip_dsp/dsp_r1/t38.obj voip_dsp/cpu_setup.obj voip_dsp/Ntt_sRing_det.obj
obj-ipchost--ac-y-sdk-  += voip_dsp/dsp_r1/t38/    voip_dsp/cpu_setup.o   voip_dsp/fsk_det/
obj-ipchost--ac-y += voip_dsp/ivr/ voip_dsp/cp3/
obj-ipchost--ac-  += voip_dsp/
obj-$(CONFIG_RTK_VOIP_DSP) += $(obj-ipchost-$(CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST)-ac-$(CONFIG_AUDIOCODES_VOIP))
obj-$(CONFIG_RTK_VOIP_MANAGER) += voip_manager/
obj-$(CONFIG_RTK_VOIP_DRIVERS) += voip_drivers/

obj-y += voip_fs/
obj-$(CONFIG_RTK_VOIP_IPC_ARCH) += voip_ipc/

endif	# CONFIG_RTK_VOIP_DRIVERS_11N_MP

EXTRA_CFLAGS += -I$(DIR_VOIP)/include

ifeq ($(CONFIG_RTK_VOIP),m)
EXTRA_CFLAGS += -mlong-calls
EXTRA_CFLAGS += -DMODULE
endif

ifdef CONFIG_PRINTK
# no need wrap
else
EXTRA_LDFLAGS += --wrap=printk
endif

ifeq "$(CONFIG_RTK_VOIP_PACKAGE_867X)" "y"
ifeq "$(CONFIG_RTK_VOIP_DRIVERS_PCM8672)" "y"
CFLAGS_KERNEL += -march=5181
AFLAGS_KERNEL += -march=5181
endif
ifeq "$(CONFIG_RTK_VOIP_DRIVERS_PCM8676)" "y"
CFLAGS_KERNEL += -march=5281
AFLAGS_KERNEL += -march=5281
endif
endif

include $(DIR_VOIP)/rules.mak
