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

#------------------------------------------------------------------------------
ifdef CONFIG_RTL_8198_AP_ROOT
AP=1
SKUTAG=ap
else
GW=1
SKUTAG=gw
endif

#------------------------------------------------------------------------------
COMPWEB=../tools/compweb

#------------------------------------------------------------------------------
.PHONY: clean all

#------------------------------------------------------------------------------
#Record which files need not include
EXCLUDE_FILES = 
EXCLUDE_FILES += ! -path '*/.*'
EXCLUDE_FILES += ! -path '*/*.bak'
EXCLUDE_FILES += ! -name Makefile
EXCLUDE_FILES += ! -name www_files
EXCLUDE_FILES += ! -name www_files.tar.bz2
EXCLUDE_FILES += ! -name webpages.bin

# routing support
ifneq ($(CONFIG_APP_ROUTED),y)
EXCLUDE_FILES += ! -name route.htm ! -name routetbl.htm
endif

#for 11s mesh
ifneq ($(CONFIG_RTL_MESH_SUPPORT),y)
EXCLUDE_FILES += ! -name buffalo.htm ! -name bull.htm ! -name wlmesh.htm ! -name wlmeshinfo.htm ! -name wlmeshACL.htm
endif

ifneq ($(CONFIG_RTK_VLAN_SUPPORT),y)
EXCLUDE_FILES += ! -name vlan.htm
endif

ifneq ($(CONFIG_PCIE_POWER_SAVING),y)
EXCLUDE_FILES += ! -name powerConsumption.htm ! -name powerConsumption_data.htm ! -name powerConsumption_proc.sxsl
EXCLUDE_FILES += ! -name num-0.gif ! -name num-1.gif ! -name num-2.gif ! -name num-3.gif ! -name num-4.gif ! -name num-5.gif
EXCLUDE_FILES += ! -name num-6.gif ! -name num-7.gif ! -name num-8.gif ! -name num-9.gif ! -name dot.gif ! -name num-0.gif
EXCLUDE_FILES += ! -name xml_data.js
endif

ifneq ($(CONFIG_RTL_ULINKER),y)
EXCLUDE_FILES += ! -name ulinker_opmode.htm
endif

ifneq ($(CONFIG_HTTP_FILE_SERVER_SUPPORT),y)
EXCLUDE_FILES += ! -name http_files.htm
EXCLUDE_FILES += ! -name http_files.htm
EXCLUDE_FILES += ! -name usb_conninfo.htm
endif

ifneq ($(CONFIG_RTL_P2P_SUPPORT),y)
EXCLUDE_FILES += ! -name wlp2p.htm ! -name wlp2pscanlist.htm ! -name wlp2pstate.htm ! -name wlp2pwscconnect.htm ! -name wlp2pwscsel.htm
endif

ifneq ($(CONFIG_APP_CTORRENT), y)
EXCLUDE_FILES += ! -name bt.htm ! -name btnewtorrent.htm ! -name btclient.htm
endif

#ifneq ($(CONFIG_RTL_P2P_SUPPORT),y)
#EXCLUDE_FILES += ! -name wlwapiCertManagement.htm
#endif

ifneq ($(CONFIG_IPV6),y)
EXCLUDE_FILES += ! -name dhcp6s.htm  ! -name ipv6_basic.htm ! -name ip6filter.htm ! -name dnsv6.htm ! -name ipv6_wan.htm ! -name radvd.htm ! -name tunnel6.htm ! -name ip6_qos.htm ! -name portfilter6.htm
endif

ifeq ($(CONFIG_IPV6),y)
EXCLUDE_FILES += ! -name ipfilter.htm ! -name portfilter.htm ! -name ip_qos.htm
endif



ifeq ($(CONFIG_RTL_WAPI_SUPPORT),y)
ifneq ($(CONFIG_RTL_WAPI_LOCAL_AS_SUPPORT),y)
EXCLUDE_FILES += ! -name wlwapiCertManagement.htm ! -name  wlwapiDistribute.htm
endif
else
EXCLUDE_FILES += ! -name wlwapiCertManagement.htm ! -name  wlwapiDistribute.htm ! -name wlwapiRekey.htm ! -name  wlwapiinstallcert.htm
endif

# 8198 and POCKET ROUTER support both wapi psk and wapi cert
# 8196C (not include POCKET ROUTER) only support wapi psk
ifeq ($(CONFIG_RTL_8196C_GW),y)
ifneq ($(CONFIG_POCKET_ROUTER_SUPPORT),y)
#EXCLUDE_FILES += ! -name wlwapiCertManagement.htm ! -name  wlwapiDistribute.htm ! -name  wlwapiinstallcert.htm
EXCLUDE_FILES += ! -name wlwapiCertManagement.htm ! -name  wlwapiDistribute.htm 
endif
endif

ifneq ($(CONFIG_APP_TR069),y)
EXCLUDE_FILES += ! -name share.js ! -name tr069config.htm
endif

ifeq ($(CONFIG_APP_RTK_VOIP),y)
	EXCLUDE_FILES += ! -name vlan.htm
	EXCLUDE_FILES += ! -name route.htm ! -name routetbl.htm
	EXCLUDE_FILES += ! -name ip_qos.htm
endif
ifneq ($(CONFIG_RTK_VLAN_WAN_TAG_SUPPORT),y)
EXCLUDE_FILES += ! -name vlan_wan.htm
endif
#------------------------------------------------------------------------------
all:
	rm -f www_files www_files.tar.bz2 webpages.bin webpages.*
ifeq ($(CONFIG_APP_RTK_VOIP),y)
	rm -f voip*.asp voip*.js
	cp $(VOIP_APP)/web/voip*.asp .
	cp $(VOIP_APP)/web/voip*.js .
	sed -i "s/goform/boafrm/g" voip*.asp
	sed -i "s/method=\"get\"/method=\"post\"/g" voip*.asp
endif
	find ./ -type f $(EXCLUDE_FILES) -name "*.*" > www_files
ifdef CONFIG_RTL_WEBPAGES_IN_ROOTFS
	cat www_files | xargs tar jcvf www_files.tar.bz2
else
#	$(COMPWEB) signature www_files webpages.bin $(CV_SIGNATURE)
	$(COMPWEB) $(SKUTAG) www_files webpages.bin
endif

clean:
	rm -f www_files www_files.tar.bz2 webpages.bin webpages.*
ifeq ($(CONFIG_APP_RTK_VOIP),y)
	rm -f voip*.asp voip*.js
endif

