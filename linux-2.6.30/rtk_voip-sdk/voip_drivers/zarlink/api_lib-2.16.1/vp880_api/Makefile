
obj-y	:= 	apiCal.o 	\
			apicnt.o 	\
			apiInit.o 	\
			apiquery.o 	

# apiseq.c requires Zarlink NDA
ifeq ($(CONFIG_VOIP_SDK),y)

ifeq ($(CONFIG_RTL_89xxD),y)
obj-y += apiseq_rtl89xxD.obj
else
obj-y += apiseq.obj
endif

ifeq ($(CONFIG_RTK_VOIP_PACKAGE_867X),y)
ifeq ($(CONFIG_8676_IAD_ZARLINK),y)
	obj-y += apiseq_rtl8676.obj
else
	obj-y += apiseq.obj
endif
endif

else
obj-y += apiseq.o
endif

EXTRA_CFLAGS += -I$(DIR_VOIP)/include/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/vp880_api/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/includes/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/arch/rtl89xxb/

ifeq "$(CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES)" "y"
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/vp890_api/
endif

include $(DIR_VOIP)/rules.mak

