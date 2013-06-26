CC=gcc
DEBUG = -g
IFLAGS = -I. -I../apmib -I..
CFLAGS = $(SUPP) # -static
CFLAGS_AP = $(SUPP)
CFLAGS_GW = $(SUPP) -DHOME_GATEWAY -DDOS_SUPPORT -DENABLE_QOS -DQOS_BY_BANDWIDTH
CFLAGS_VPN = $(SUPP) -DHOME_GATEWAY -DVPN_SUPPORT -DDOS_SUPPORT -DENABLE_QOS -DQOS_BY_BANDWIDTH

ifndef COMMON_CFLAGS
$(error COMMON_CFLAGS is not defined)
endif

ifdef CONFIG_RTL_8198_AP_ROOT
all: compweb cvcfg-ap cvimg mgbin mkramdisk
else
all: compweb cvcfg-gw cvcfg-ap cvimg mgbin mkramdisk cvcfg-ap-8m cvcfg-vpn cvcfg-8m
endif

ifeq ($(wildcard $(DIR_LINUX)/.config),)
include ../../../linux-2.6.30/.config
else
include $(DIR_LINUX)/.config
endif

ifeq ($(wildcard $(DIR_USERS)/.config),)
include ../../.config
else
include $(DIR_USERS)/.config
endif

ifeq ($(CONFIG_RTK_VLAN_SUPPORT),y)
VLAN_CONFIG_SUPPORTED = 1
endif

SUPP = -DUNIVERSAL_REPEATER
SUPP += -DWIFI_SIMPLE_CONFIG
ifdef CONFIG_RTL_8198_AP_ROOT
else
SUPP += -DROUTE_SUPPORT
endif
SUPP+= $(COMMON_CFLAGS)
ifeq ($(CONFIG_RTL_819X),y)
	SUPP += -DCONFIG_RTL_819X
endif

ifeq ($(CONFIG_RTL_8196B_GW),y)
        SUPP += -DCONFIG_RTL_8196B
endif

ifeq ($(CONFIG_RTL_8196C_GW),y)
        SUPP += -DCONFIG_RTL_8196C
endif

ifeq ($(CONFIG_RTL_8196C_iNIC),y)
        SUPP += -DCONFIG_RTL_8196C
endif

ifeq ($(CONFIG_RTL_8198_GW),y)
        SUPP += -DCONFIG_RTL_8198
endif

ifeq ($(CONFIG_RTL_8198_AP_ROOT),y)
        SUPP += -DCONFIG_RTL_8198
endif

ifeq ($(CONFIG_RTL_8198_INBAND_AP),y)
        SUPP += -DCONFIG_RTL_8198
endif

ifeq ($(CONFIG_RTL_819XD),y)
        SUPP += -DCONFIG_RTL_819XD
endif

ifeq ($(CONFIG_RTL_8196E),y)
        SUPP += -DCONFIG_RTL_8196E
endif
 
ifeq ($(CONFIG_RTL8196B_GW_8M),y)
	SUPP += -DCONFIG_RTL8196B_GW_8M
endif

ifeq ($(VLAN_CONFIG_SUPPORTED),1)
SUPP +=-DVLAN_CONFIG_SUPPORTED
endif

ifeq ($(CONFIG_RTL_ULINKER),1)
SUPP +=-DCONFIG_RTL_ULINKER
endif

ifeq ($(CONFIG_RTL_92D_SUPPORT),y)
SUPP +=-DCONFIG_RTL_92D_SUPPORT
endif

ifeq ($(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN),y)
SUPP +=-DCONFIG_RTL_DUAL_PCIESLOT_BIWLAN
endif

# for 802.11s
SUPP += 
ifeq ($(CONFIG_MESH_ENABLE),y)
	SUPP += -DCONFIG_RTK_MESH -DSNMP_SUPPORT -D_MESH_ACL_ENABLE_ -DWIFI_SIMPLE_CONFIG 
	ifeq ($(CONFIG_11S_TEST_MODE),y)
		SUPP += -D_11s_TEST_MODE_
	endif	
endif

#For flash Mapping
ifeq ($(CONFIG_RTL_FLASH_MAPPING_ENABLE),y)
        SUPP += -DCONFIG_RTL_FLASH_MAPPING_ENABLE

ifdef CONFIG_RTL_HW_SETTING_OFFSET
        SUPP += -DCONFIG_RTL_HW_SETTING_OFFSET=$(CONFIG_RTL_HW_SETTING_OFFSET)
endif

ifdef CONFIG_RTL_DEFAULT_SETTING_OFFSET
        SUPP += -DCONFIG_RTL_DEFAULT_SETTING_OFFSET=$(CONFIG_RTL_DEFAULT_SETTING_OFFSET)
endif

ifdef CONFIG_RTL_CURRENT_SETTING_OFFSET
        SUPP += -DCONFIG_RTL_CURRENT_SETTING_OFFSET=$(CONFIG_RTL_CURRENT_SETTING_OFFSET)
endif

ifdef CONFIG_RTL_WEB_PAGES_OFFSET
        SUPP += -DCONFIG_RTL_WEB_PAGES_OFFSET=$(CONFIG_RTL_WEB_PAGES_OFFSET)
endif

ifdef CONFIG_RTL_LINUX_IMAGE_OFFSET
        SUPP += -DCONFIG_RTL_CODE_IMAGE_OFFSET=$(CONFIG_RTL_LINUX_IMAGE_OFFSET)
endif

ifdef CONFIG_RTL_ROOT_IMAGE_OFFSET
        SUPP += -DCONFIG_RTL_ROOT_IMAGE_OFFSET=$(CONFIG_RTL_ROOT_IMAGE_OFFSET)
endif
endif

SUPP += -DSTAND_ALONE_MINIUPNP
	
ifeq ($(CONFIG_APP_RTK_VOIP),y)
SUPP += -DVOIP_SUPPORT -I$(VOIP_APP)/includes
ifdef CONFIG_RTK_VOIP_IP_PHONE
SUPP += -I$(VOIP_APP)/ui/include
endif
endif

ifeq ($(CONFIG_IPV6),y)
SUPP += -DCONFIG_IPV6
endif

ifeq ($(CONFIG_APP_CTORRENT),y)
SUPP += -DCONFIG_RTL_BT_CLIENT
endif
 
ifeq ($(CONFIG_RTL_WAPI_SUPPORT),y)
SUPP += -DCONFIG_RTL_WAPI_SUPPORT
ifeq ($(CONFIG_RTL_WAPI_LOCAL_AS_SUPPORT),y)
SUPP += -DCONFIG_RTL_WAPI_LOCAL_AS_SUPPORT
endif
endif

ifeq ($(CONFIG_RTL_802_1X_CLIENT_SUPPORT),y)
SUPP += -DCONFIG_RTL_802_1X_CLIENT_SUPPORT
endif

ifeq ($(GW),1)
ifeq ($(CONFIG_NET_QOS),y)
        CFLAGS_GW += -DENABLE_QOS
        ifeq ($(CONFIG_QOS_UI_BY_BANDWIDTH),y)
                CFLAGS_GW += -DQOS_BY_BANDWIDTH
        endif
endif
endif

CVCFG_SRCS = cvcfg.c ../apmib/mibtbl.c
ifeq ($(CONFIG_APP_RTK_VOIP),y)
CVCFG_SRCS += $(VOIP_APP)/flash/voip_flash_mib.c
endif

CVCFG_AP_OBJS = $(CVCFG_SRCS:.c=-ap.o)
CVCFG_AP_8M_OBJS = $(CVCFG_SRCS:.c=-ap-8m.o)
CVCFG_GW_OBJS = $(CVCFG_SRCS:.c=-gw.o)
CVCFG_GW_8M_OBJS = $(CVCFG_SRCS:.c=-gw-8m.o)
CVCFG_VPN_OBJS = $(CVCFG_SRCS:.c=-vpn.o)

ifdef CONFIG_DEFAULTS_KERNEL_2_6
CFLAGS  += -DCONFIG_DEFAULTS_KERNEL_2_6
endif

compweb: compweb.o
	$(CC) -o compweb $(DEBUG) \
	$(CFLAGS) $(IFLAGS) \
	compweb.o $(LDFLAGS)

cvcfg-ap: $(CVCFG_AP_OBJS)
	$(CC) -o cvcfg-ap $(DEBUG) \
	$(CFLAGS) $(IFLAGS) \
	$(CVCFG_AP_OBJS) $(LDFLAGS)

cvcfg-ap-8m: $(CVCFG_AP_8M_OBJS)
	$(CC) -o cvcfg-ap-8m $(DEBUG) \
	$(CFLAGS) $(IFLAGS) \
	$(CVCFG_AP_8M_OBJS) $(LDFLAGS)

cvcfg-gw: $(CVCFG_GW_OBJS)
	$(CC) -o cvcfg-gw $(DEBUG) \
	$(CFLAGS) $(IFLAGS) \
	$(CVCFG_GW_OBJS) $(LDFLAGS)

cvcfg-vpn: $(CVCFG_VPN_OBJS)
	$(CC) -o cvcfg-vpn $(DEBUG) \
	$(CFLAGS) $(IFLAGS) \
	$(CVCFG_VPN_OBJS) $(LDFLAGS)

cvcfg-8m: $(CVCFG_GW_8M_OBJS)
	$(CC) -o cvcfg-8m $(DEBUG) \
	$(CFLAGS) $(IFLAGS) \
	$(CVCFG_GW_8M_OBJS) $(LDFLAGS)

cvimg: cvimg.o
	$(CC) -o cvimg $(DEBUG) \
	$(CFLAGS) $(IFLAGS) \
	cvimg.o $(LDFLAGS)

mgbin: mgbin.o
	$(CC) -o mgbin $(DEBUG) \
	$(CFLAGS) $(IFLAGS) \
	mgbin.o $(LDFLAGS)

mkramdisk: ramdisk.o
	$(CC) -o $@ $(DEBUG) \
	$(CFLAGS) $(IFLAGS) \
	ramdisk.o $(LDFLAGS)

clean:
	rm compweb.o cvimg.o mgbin.o ramdisk.o $(CVCFG_AP_OBJS) $(CVCFG_AP_8M_OBJS) \
	$(CVCFG_GW_OBJS) $(CVCFG_GW_8M_OBJS) $(CVCFG_VPN_OBJS) -f
	rm compweb cvcfg-gw cvcfg-ap cvcfg-vpn cvimg mgbin mkramdisk cvcfg-ap-8m.o \
	mibtbl-ap-8m.o cvcfg-ap-8m cvcfg-8m -f

#
#	Dependencies
#
%-ap.o: %.c
	$(CC) -c -o $@ $(DEBUG) $(CFLAGS_AP) $(IFLAGS) $<

%-ap-8m.o: %.c
	$(CC) -c -o $@ $(DEBUG) $(CFLAGS_AP) -DCOMPACK_SIZE $(IFLAGS) $<

%-gw.o: %.c
	$(CC) -c -o $@ $(DEBUG) $(CFLAGS_GW) $(IFLAGS) $<

%-gw-8m.o: %.c
	$(CC) -c -o $@ $(DEBUG) $(CFLAGS_GW) -DCOMPACK_SIZE $(IFLAGS) $<

%-vpn.o: %.c
	$(CC) -c -o $@ $(DEBUG) $(CFLAGS_VPN) $(IFLAGS) $<

#
#	Transition rules (add -o to put object in right directory)
#
.c.o:
	$(CC) -c -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) $<

