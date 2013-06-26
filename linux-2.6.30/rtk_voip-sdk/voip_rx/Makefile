obj-$(CONFIG_RTK_VOIP_RX)            += voip_rx.o

voip_rx-objs-y  := rtk_trap.o

EXTRA_CFLAGS += -I$(DIR_VOIP)/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/dsp_r1/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_tx

ifeq ($(CONFIG_RTK_VOIP_SRTP),y)
EXTRA_CFLAGS += -I$(DIR_VOIP)/srtp/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/srtp/crypto/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/cp3
#EXTRA_CFLAGS+=-DFEATURE_COP3_PROFILE
endif

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

# rock: add module name for PRINT
EXTRA_CFLAGS += -DMODULE_NAME=\"RX\"

voip_rx-objs := $(voip_rx-objs-y)

include $(DIR_VOIP)/rules.mak
