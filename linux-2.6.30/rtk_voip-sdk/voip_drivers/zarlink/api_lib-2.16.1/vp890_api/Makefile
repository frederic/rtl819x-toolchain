
obj-y	:= 	vp890_apiCal.o  	\
			vp890_common.o  	\
			vp890_config.o  	\
			vp890_control.o  	\
			vp890_init.o  		\
			vp890_query.o 

# vp890_seq.c requires Zarlink NDA
ifeq ($(CONFIG_VOIP_SDK),y)

ifeq ($(CONFIG_RTL_89xxD),y)
obj-y += vp890_seq_rtl89xxD.obj
else
obj-y += vp890_seq.obj
endif

ifeq ($(CONFIG_RTK_VOIP_PACKAGE_867X),y)
ifeq ($(CONFIG_8676_IAD_ZARLINK),y)
obj-y += vp890_seq_rtl8676.obj
else
obj-y += vp890_seq.obj
endif
endif

else
obj-y += vp890_seq.o
endif

EXTRA_CFLAGS += -I$(DIR_VOIP)/include/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/vp890_api/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/includes/
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_drivers/zarlink/$(ZARLINK_LIB)/arch/rtl89xxb/

include $(DIR_VOIP)/rules.mak
