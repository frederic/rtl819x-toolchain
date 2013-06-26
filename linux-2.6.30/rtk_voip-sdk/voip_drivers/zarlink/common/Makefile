obj-y	+= zarlinkCommonInit.o
obj-y	+= zarlinkCommonSlic.o
obj-y	+= zarlinkCommonDaa.o

ifeq "$(CONFIG_RTK_VOIP_SLIC_ZARLINK_880_SERIES)" "y"
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/ve880/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/vp880_api/
endif

ifeq "$(CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES)" "y"
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/ve890/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/vp890_api/
endif

EXTRA_CFLAGS += -I$(DIR_VOIP)/include/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/common/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/includes/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/$(ZARLINK_PROFILE)/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/arch/rtl89xxb/

include $(DIR_VOIP)/rules.mak
