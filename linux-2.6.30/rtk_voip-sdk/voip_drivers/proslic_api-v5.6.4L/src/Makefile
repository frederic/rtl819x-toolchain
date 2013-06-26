obj-y := proslic.o proslic_ctrl_spi.o proslic_timer_linux.o si_voice_version.o si_voice.o

obj-$(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226) += si3226_intf.o si3226_userdef.o 

obj-$(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226x) += si3226x_intf.o si3226x_userdef.o

obj-$(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3217x) += si3217x_intf.o si3217x_userdef.o 

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_FXO),y)
ifeq ($(CONFIG_RTK_VOIP_DRIVERS_SI3050),y)
obj-y += vdaa.o
endif
endif

EXTRA_CFLAGS += -I$(DIR_VOIP)/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/proslic_api/inc
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/proslic_api/custom

ifeq "$(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3217x)" "y"
EXTRA_CFLAGS += -DSI3217X
endif
ifeq "$(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226)" "y"
EXTRA_CFLAGS += -DSI322X
endif
ifeq "$(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226x)" "y"
EXTRA_CFLAGS += -DSI3226X
endif

include $(DIR_VOIP)/rules.mak
