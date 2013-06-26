#
# Makefile for the IEEE 802.1d ethernet bridging layer.
#

ifeq ($(CONFIG_RTL_MESH_SUPPORT),y)
        EXTRA_CFLAGS += -DCONFIG_RTK_MESH
endif

obj-$(CONFIG_BRIDGE) += bridge.o

bridge-y	:= br.o br_device.o br_fdb.o br_forward.o br_if.o br_input.o \
			br_ioctl.o br_notify.o br_stp.o br_stp_bpdu.o \
			br_stp_if.o br_stp_timer.o br_netlink.o 

bridge-$(CONFIG_SYSFS) += br_sysfs_if.o br_sysfs_br.o

bridge-$(CONFIG_RTL865X_LANPORT_RESTRICTION) += ../rtl/features/lan_restrict.o

bridge-$(CONFIG_BRIDGE_NETFILTER) += br_netfilter.o

obj-$(CONFIG_DOMAIN_NAME_QUERY_SUPPORT) += pocket_filter.o
ifeq ($(CONFIG_RTL_ULINKER),y)
        obj-y += pocket_filter.o
endif

obj-$(CONFIG_BRIDGE_NF_EBTABLES) += netfilter/

#EXTRA_CFLAGS += -I $(DIR_LINUX)/net/ipv4/fastpath/
#EXTRA_CFLAGS += -I$(DIR_LINUX)/drivers/net/rtl819x/
