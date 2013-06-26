obj-y += common/
obj-y += $(ZARLINK_PROFILE)/
obj-y += arch/rtl89xxb/

ifeq "$(CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES)" "y"
obj-y += vp890_api/
endif

ifeq "$(CONFIG_RTK_VOIP_SLIC_ZARLINK_880_SERIES)" "y"
obj-y += vp880_api/
endif
include $(DIR_VOIP)/rules.mak

