obj-$(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226) += si3226_constants.o si3226_patch_C_2008MAY07.o si3226_patch_D_QCUK_2008NOV20.o

obj-$(CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3217x_BB) += si3217x_BKBT_constants.o si3217x_patch_B_BB_2009MAY22.o

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_FXO),y)
ifeq ($(CONFIG_RTK_VOIP_DRIVERS_SI3050),y)
obj-y += vdaa_constants.o
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
EXTRA_CFLAGS += -DSI32267
endif

include $(DIR_VOIP)/rules.mak
