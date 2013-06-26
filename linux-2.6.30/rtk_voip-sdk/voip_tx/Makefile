obj-$(CONFIG_RTK_VOIP_TX) += voip_tx.o 

voip_tx-objs-y := direct_send_mgr.o 

EXTRA_CFLAGS += -I$(DIR_VOIP)/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/dsp_r1/include
ifeq ($(CONFIG_RTK_VOIP_PACKAGE_865X),y)
ifeq ($(CONFIG_RTK_VOIP_DRIVERS_PCM8651)$(CONFIG_RTK_VOIP_DRIVERS_PCM865xC),y)
EXTRA_CFLAGS += -Idrivers/net/re865x/rtl865x
endif
endif

ifeq ($(CONFIG_RTK_VOIP_SRTP),y)
EXTRA_CFLAGS +=  -I$(DIR_VOIP)/voip_dsp/cp3
#EXTRA_CFLAGS+=-DFEATURE_COP3_PROFILE
EXTRA_CFLAGS += -I$(DIR_VOIP)/srtp/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/srtp/crypto/include
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
EXTRA_CFLAGS += -DMODULE_NAME=\"TX\"

voip_tx-objs := $(voip_tx-objs-y)

include $(DIR_VOIP)/rules.mak
