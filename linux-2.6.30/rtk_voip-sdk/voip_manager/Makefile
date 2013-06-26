obj-$(CONFIG_RTK_VOIP_DRIVERS)  += voip_manager.o
voip-manager-mod-objs-y    := voip_mgr.o voip_mgr_netfilter.o 
voip-manager-mod-objs-y    += voip_mgr_do_protocol.o voip_mgr_do_dsp.o voip_mgr_do_driver.o 
voip-manager-mod-objs-y    += voip_mgr_do_misc.o voip_mgr_do_ipphone.o voip_mgr_do_debug.o

voip-manager-mod-objs-y    += voip_mgr_transfer_id.o voip_mgr_events.o 

voip-manager-mod-objs-$(CONFIG_REALTEK_VOIP) += voip_resource_check.o

ifeq ($(CONFIG_RTK_VOIP_IPC_ARCH),y)
ifeq ($(CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST),y)
voip-manager-mod-objs-y    += voip_mgr_help_host.o
endif
ifeq ($(CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP),y)
voip-manager-mod-objs-y    += voip_mgr_help_dsp.o
endif
endif

EXTRA_CFLAGS += -I$(DIR_VOIP)/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/cp3
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/dsp_r1/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/v152
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_ipc

ifeq "$(CONFIG_AUDIOCODES_VOIP)" "y"
EXTRA_CFLAGS += -Iacmw_lx/ac_drv
EXTRA_CFLAGS += -Iacmw_lx/include
EXTRA_CFLAGS += -DAUDIOCODES_VOIP
EXTRA_CFLAGS += -DACMW_LX
EXTRA_CFLAGS += -DACL_BYPASS
EXTRA_CFLAGS += -DACL_RELEASE
EXTRA_CFLAGS += -DAUDIOCODES_VOIP_SIP
endif

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_PCM8651),y)
EXTRA_CFLAGS += -Idrivers/net/re865x/rtl865x
endif

ifeq ($(CONFIG_RTK_VOIP_SRTP),y)
EXTRA_CFLAGS += -I$(DIR_VOIP)/srtp/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/srtp/crypto/include
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

voip_manager-objs := $(voip-manager-mod-objs-y)

# rock: add module name for PRINT
EXTRA_CFLAGS += -DMODULE_NAME=\"MGR\"

include $(DIR_VOIP)/rules.mak
