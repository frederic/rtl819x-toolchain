
obj-y	:= ve880_int.o ve880_api.o 

ifneq "$(CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK_ON_NEW_ARCH)" "y"
obj-y	+= ve880_int_ext.o
endif

#EXTRA_CFLAGS += -Werror

ifeq "$(CONFIG_RTK_VOIP_SLIC_ZARLINK_880_SERIES)" "y"
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/vp880_api/
endif
ifeq "$(CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES)" "y"
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/vp890_api/
endif

EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/common/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/includes/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/$(ZARLINK_PROFILE)/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/arch/rtl89xxb/

EXTRA_CFLAGS += -I$(DIR_VOIP)/include/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/spi/

include $(DIR_VOIP)/rules.mak
