#
# Realtek Semiconductor Corp.
#
# Tony Wu (tonywu@realtek.com)
# Dec. 07, 2008
#

DIR_ROOT ?= ..
DIR_BOARD ?= $(DIR_ROOT)/target

.EXPORT_ALL_VARIABLES:
.PHONY: all romfs clean prune

ifneq (.config,$(wildcard .config))
all: config
else
all: lib app

include .config
include $(DIR_ROOT)/.config
include $(DIR_LINUX)/.config

CONFIG_RTL_WAPI_LOCAL_AS=0
CONFIG_USER_CWMP_OPENSSL=0

ifdef CONFIG_USER_CWMP_OPENSSL_SUPPORT
CONFIG_USER_CWMP_OPENSSL=1
endif
# 8198 and POCKET ROUTER support both wapi psk and wapi cert
# 8196C (not include POCKET ROUTER) only support wapi psk
ifdef CONFIG_RTL_8198_GW
ifdef CONFIG_RTL_WAPI_SUPPORT
ifdef CONFIG_RTL_WAPI_LOCAL_AS_SUPPORT
CONFIG_RTL_WAPI_LOCAL_AS=1
endif
CONFIG_APP_OPENSSL=y
CONFIG_APP_WAPI=y
endif
else # !CONFIG_RTL_8198_GW

ifdef CONFIG_POCKET_ROUTER_SUPPORT
ifdef CONFIG_RTL_WAPI_SUPPORT
ifdef CONFIG_RTL_WAPI_LOCAL_AS_SUPPORT
CONFIG_RTL_WAPI_LOCAL_AS=1
endif
# ############################################
# Because rootfs + 64K wapi-cert is oversize at 2M/16M,
# close WAN apps here just for wapi logo test!!!!!!
CONFIG_APP_PPPD=n
CONFIG_APP_PPTP=n
CONFIG_APP_L2TPD=n
CONFIG_APP_NTPCLIENT=n
CONFIG_APP_DNRD=n
CONFIG_APP_IPROUTE=n
# ############################################
CONFIG_APP_OPENSSL=y
CONFIG_APP_WAPI=y
endif
endif

endif

ifdef CONFIG_RTL_8196C
ifdef CONFIG_RTL_WAPI_SUPPORT
CONFIG_APP_OPENSSL=y
CONFIG_APP_WAPI=y
endif
endif

APMIBINC=
APMIBLIB=
ifeq ($(CONFIG_APP_BOA),y)
APMIBINC = $(DIR_USERS)/boa/apmib/
APMIBLIB = $(DIR_USERS)/boa/apmib/libapmib.so
else
APMIBINC = $(DIR_USERS)/goahead-2.1.1/LINUX/
APMIBLIB = $(DIR_USERS)/goahead-2.1.1/LINUX/apmib.so
endif
	
ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
CONFIG_APP_OPENSSL=y
CONFIG_RTL_WAPI_LOCAL_AS=1
endif

ifeq ($(CONFIG_RTL_WAPI_LOCAL_AS),1)
ifeq ($(CONFIG_RSDK_rsdk-1.5.5-5281-EB-2.6.30-0.9.30.3-110714),y) 
CONFIG_APP_OPENSSL-0.9.8I=y
endif
ifeq ($(CONFIG_RSDK_rsdk-1.5.5-4181-EB-2.6.30-0.9.30.3-110225),y)
CONFIG_APP_OPENSSL-0.9.8I=y
endif
ifneq ($(CONFIG_APP_OPENSSL-0.9.8I),y)
CONFIG_APP_OPENSSL-0.9.8B=y
endif
endif

ifdef CONFIG_RTL_COMAPI_WLTOOLS
WLTOOLS=wireless_tools.29
else
WLTOOLS=wireless_tools.25
endif

#EXIST=$(wildcard $(DIR_RSDK)/bin/rsdk-uclibc-gcc)
#ifneq ($(EXIST),)
#CROSS_COMPILE ?= rsdk-uclibc-
#else
CROSS_COMPILE ?= rsdk-linux-
#endif

CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
RANLIB = $(CROSS_COMPILE)ranlib
STRIP = $(CROSS_COMPILE)strip
SSTRIP = $(CROSS_COMPILE)sstrip

ifeq ($(CONFIG_RSDK_rsdk-1.5.0-4181-EB-2.6.30-0.9.30.2),y)
CFLAGS += -mips16
endif
ifeq ($(CONFIG_RSDK_rsdk-1.5.0-4181-EB-2.6.30-0.9.30.3),y)
CFLAGS += -mips16
endif
CFLAGS += -I$(DIR_TMPFS)/include

CINIT=1

ifeq ($(CONFIG_APP_RTK_VOIP),y)
app_y = script/mdev
endif

app_n =
app_  =

lib_y = 
lib_n =
lib_  =

export CFLAGS

# application
app_$(CONFIG_APP_BUSYBOX)		+= busybox
app_$(CONFIG_APP_GOAHEAD)		+= goahead-2.1.1
app_$(CONFIG_APP_BOA)			+= boa
ifeq ($(CINIT), 1)
app_$(CONFIG_APP_SCRIPT)		+= script/cinit
else
app_$(CONFIG_APP_SCRIPT)		+= script
endif
app_$(CONFIG_APP_SQUASHFS40)		+= squashfs4.0/squashfs-tools
app_$(CONFIG_APP_IPROUTE)		+= iproute2-2.6.29-1
app_$(CONFIG_APP_IPTABLES)		+= iptables-1.4.4
app_$(CONFIG_APP_L7FILTER_PAT)		+= l7-protocols
app_$(CONFIG_APP_NTPCLIENT)		+= ntpclient
app_$(CONFIG_APP_DHCPV6)		+= wide-dhcpv6
app_$(CONFIG_APP_RADVD)			+= radvd-0.9.1
app_$(CONFIG_APP_ECMH)			+= ecmh
app_$(CONFIG_APP_GDBSERVER)             += gdb
app_$(CONFIG_APP_USBMOUNT)		+= usbmount
app_$(CONFIG_APP_VSFTPD)		+= vsftpd-2.3.2
app_$(CONFIG_APP_DOSFSCK)		+= dosfstools-2.11/dosfsck
app_$(CONFIG_APP_MKDOSFS)		+= dosfstools-2.11/mkdosfs
app_$(CONFIG_APP_NTFS3G)		+= ntfs-3g-2010.10.2
app_$(CONFIG_APP_MP)			+= mp-daemon
#CONFIG_APP_SAMBA-3.0.37 OR CONFIG_APP_SAMBA-3.0.24 at most one will be chose
#app_$(CONFIG_APP_SAMBA-3.0.37)		+= samba-3.0.37/source
app_$(CONFIG_APP_SAMBA)		+= samba
#app_$(CONFIG_APP_SMABA-3.2.15)		+= samba
#ifeq($(CONFIG_APP_SAMBA-3.0.24),y)
#else
#ifeq($(CONFIG_APP_SAMBA-3.0.37),y)
#else
#ifeq($(CONFIG_APP_SAMBA-3.2.15),y)
#endif

app_$(CONFIG_APP_IGMPPROXY)		+= igmpproxy
app_$(CONFIG_APP_WIRELESSTOOLS)		+= $(WLTOOLS)
app_$(CONFIG_APP_DNRD)			+= dnrd-2.12.1/src
app_$(CONFIG_APP_ROUTED)		+= routed
app_$(CONFIG_APP_RTK_CMD)		+= rtk_cmd 
app_$(CONFIG_APP_PPPD)			+= ppp-2.4.4/pppd
app_$(CONFIG_APP_PPTP)			+= pptp-1.7.2
app_$(CONFIG_APP_L2TPD)			+= l2tpd
app_$(CONFIG_APP_DNSMASQ)		+= dnsmasq-2.33
app_$(CONFIG_APP_MINI_UPNP)		+= mini_upnp
app_$(CONFIG_APP_WSC)			+= wsc 
app_$(CONFIG_APP_AUTH)			+= auth/src 
app_$(CONFIG_APP_IWCONTROL)		+= auth/src/dlisten
app_$(CONFIG_APP_UDHCPD)		+= udhcp-0.9.9-pre
app_$(CONFIG_APP_BRCTL)			+= bridge-utils
app_$(CONFIG_APP_MINIIGD)		+= miniigd
app_$(CONFIG_APP_RELOAD)		+= rc
app_$(CONFIG_APP_LLTDD)			+= lltdd/native-linux
app_$(CONFIG_APP_ORAYDDNS)			+= orayddns
app_$(CONFIG_APP_UPDATEDD)			+=  updatedd-2.5
app_$(CONFIG_APP_IAPP)			+= IAPP 
lib_$(CONFIG_APP_RTK_VOIP)		+= rtk_voip		# pkshih: build this before tr069
app_$(CONFIG_APP_RTK_VOIP)		+= rtk_voip		# pkshih: build this before tr069
app_$(CONFIG_APP_PMC_UTILS)		+= pmc_utils
app_$(CONFIG_APP_NFBI)			+= nfbi
app_$(CONFIG_APP_HOSTAPD_SLAVE)			+= hostapd_slave
app_$(CONFIG_APP_RTK_INBAND_CTL)	+= hcm
app_$(CONFIG_APP_RTK_INBAND_HOST)       += rtk_inbandHost
app_$(CONFIG_APP_SNMP)			+= net-snmp-5.4.1
app_$(CONFIG_APP_IOH)			+= ioh
app_$(CONFIG_APP_NFBI_HOST)		+= nfbihost/nfbi
app_$(CONFIG_APP_TR069)			+= cwmp-tr069
app_$(CONFIG_APP_UWIFI)		+= uWiFi
app_$(CONFIG_APP_NMBS)		+= nbserver
app_$(CONFIG_APP_AUTORUNIMG)    += autorun_img_tool

# USB3G
app_$(CONFIG_APP_COMGT)			+= comgt.0.32
app_$(CONFIG_APP_CHAT)			+= ppp-2.4.4/chat
app_$(CONFIG_APP_LIBUSB)		+= libusb-0.1.12
app_$(CONFIG_APP_USBMODESWITCH)		+= usb-modeswitch
app_$(CONFIG_APP_MBPK_EJECT)		+= mbpk_eject
app_$(CONFIG_APP_USBMODESWITCH_DATA)	+= usb-modeswitch-data
app_$(CONFIG_APP_USBUTILS)		+= usbutils-0.86
app_$(CONFIG_APP_HUBCTRL)		+= hub-ctrl
app_$(CONFIG_APP_MNET)		        += mnet

# debug & test
app_$(CONFIG_APP_CLE_SHELL)		+= cle-shell
app_$(CONFIG_APP_UTILS)			+= utils
app_$(CONFIG_APP_DHRYSTONE)		+= dhrystone
app_$(CONFIG_APP_LTP)			+= ltp

# wapi related
#app_$(CONFIG_APP_OPENSSL)		+= openssl-0.9.8b 
app_$(CONFIG_APP_OPENSSL)		+= 	

app_$(CONFIG_APP_OPENSSL-0.9.8B)	+= openssl-0.9.8b
app_$(CONFIG_APP_OPENSSL-0.9.8I)	+= openssl-0.9.8i

app_$(CONFIG_APP_WAPI)			+= wapi_utils/udp_sock wapi_utils/rwWapiFiles \
					  wapi_utils/other_tools wapi_utils/wapi_scripts
app_$(CONFIG_APP_HOSTAPD_HCM)           += hostapd-0.6.10/hostapd inband_lib
app_$(CONFIG_APP_DLNA_DMS)		+= dlna_dms

app_$(CONFIG_APP_MT_DAAPD)		+= mt-daapd-0.2.4.2

# libraries
#lib_$(CONFIG_LIBNL) += iw-0.9.17
lib_$(CONFIG_APP_HOSTAPD_HCM)		+= inband_lib
lib_$(CONFIG_APP_HOSTAPD_SLAVE)		+= inband_lib
lib_$(CONFIG_APP_RTK_INBAND_CTL)	+= rtk_inbandLib
lib_$(CONFIG_APP_RTK_INBAND_HOST)	+= rtk_inbandLib
lib_$(CONFIG_LIB_NL) += libnl-1.1
lib_$(CONFIG_APP_FLEX)			+= flex-2.5.33
lib_$(CONFIG_LIB_Z)			+= zlib-1.2.5
lib_$(CONFIG_APP_PPPD)			+= ppp-2.4.4/pppd/plugins/rp-pppoe
ifeq ($(CONFIG_APP_SNMP),y)
lib_$(CONFIG_APP_GOAHEAD)		+= goahead-2.1.1
endif
ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
#lib_$(CONFIG_APP_OPENSSL)		+= openssl-0.9.8b
lib_$(CONFIG_APP_OPENSSL-0.9.8B)	+= openssl-0.9.8b
lib_$(CONFIG_APP_OPENSSL-0.9.8I)	+= openssl-0.9.8i
endif

lib:
ifneq ($(lib_y),)
	$(MAKE) $(lib_y) || exit $$?
else
	@echo "No libs specified"
endif
app:
	$(MAKE) $(app_y) || exit $$?
endif
#
# add directory dependancies here
#
.PHONY: ulibs romfs $(lib_y) $(app_y)

# We create the target directory of the symlink if it does
# not exist so the test in check-symlink works and we have a
# directory for generated filesas used by some architectures.
define create-symlink
	if [ ! -L cle-shell ]; then                                      \
			echo '  SYMLINK cle-shell -> ../../../ICTest/users/cle-shell/'; \
			ln -fsn ../../../ICTest/users/cle-shell/ cle-shell;                         \
	fi
endef

prepare:
ifeq ($(CONFIG_APP_CLE_SHELL),y)
	@$(create-symlink)
endif
ifeq ($(CONFIG_APP_UTILS),y)
	@if [ ! -L utils ]; then \
			echo '  SYMLINK utils -> ../../../ICTest/users/utils/'; \
			ln -fsn ../../../ICTest/users/utils/ utils; \
	fi
endif
#ifeq ($(CONFIG_APP_OPENSSL),y)
ifeq ($(CONFIG_APP_OPENSSL-0.9.8B),y)
	@cd ./openssl-0.9.8b; \
	if [ $(CONFIG_RTL_WAPI_LOCAL_AS) = 1 -o $(CONFIG_USER_CWMP_OPENSSL) = y ]; then \
		if [ ! -f dlib_config_done ]; then \
			./Configure shared rsdk-linux; make depend; \
			rm -f slib_config_done; touch dlib_config_done; \
			make clean; \
			cp Makefile.DLibForWapi Makefile; \
			cp test/Makefile.wapi test/Makefile; \
			fi;\
	else\
		if [ ! -f slib_config_done ]; then \
			./Configure no-threads no-hw no-shared rsdk-linux; make depend; \
			rm -f dlib_config_done; touch slib_config_done;\
			make clean; \
			cp Makefile.SLibForWapi Makefile; \
			cp test/Makefile.wapi test/Makefile; \
		fi;\
	fi;\
	cd -;
endif
ifeq ($(CONFIG_APP_OPENSSL-0.9.8I),y)
	@cd ./openssl-0.9.8i; \
	if [ $(CONFIG_RTL_WAPI_LOCAL_AS) = 1 -o $(CONFIG_USER_CWMP_OPENSSL) = y ]; then \
		if [ ! -f dlib_config_done ]; then \
			./Configure shared rsdk-linux; make depend; \
			rm -f slib_config_done; touch dlib_config_done; \
			make clean; \
			cp Makefile.DLibForWapi Makefile; \
			cp test/Makefile.wapi test/Makefile; \
			fi;\
	else\
		if [ ! -f slib_config_done ]; then \
			./Configure no-threads no-hw no-shared rsdk-linux; make depend; \
			rm -f dlib_config_done; touch slib_config_done;\
			make clean; \
			cp Makefile.SLibForWapi Makefile; \
			cp test/Makefile.wapi test/Makefile; \
		fi;\
	fi;\
	cd -;
endif

ifeq ($(CONFIG_APP_RTK_VOIP),y)
	# add voip support
	@if [ ! -e rtk_voip/.linux_config ]; then	\
		ln -fs $(DIR_LINUX)/.config rtk_voip/.linux_config;	\
	fi
	make -C rtk_voip prepare
endif

ifeq ($(CONFIG_APP_IPTABLES),y)
	@cd ./iptables-1.4.4; \
	if [ ! -f iptables_config_done ]; then \
		./configure --host=mips-linux --with-ksource=../../linux-2.6.30 --enable-static --disable-shared --disable-ipv6 CFLAGS=-Os; make depend; \
		rm -f iptables_config_done; touch iptables_config_done; \
		cp Makefile.RSDK Makefile; \
		cp ./extensions/GNUmakefile.RSDK ./extensions/GNUmakefile; \
	fi; \
	cd -
endif

ifeq ($(CONFIG_APP_GDBSERVER),y)
	echo 'make gdbserver';
	cd ./gdb; \
	if [ ! -f gdb_config_done ]; then \
		cd gdb-6.8; ./configure --target=mips-linux --host=i686-pc-linux-gnu --prefix=$(shell pwd)/gdb/gdb-host;  \
		cp Makefile.RSDK Makefile; \
		cd gdb/gdbserver; \
		./configure --target=mips-linux --host=mips-linux --prefix=$(shell pwd)/gdb/gdbserver-6.8; \
		cd ../../../; \
		rm -f gdb_config_done; touch gdb_config_done; \
	fi; \
	cd ..
endif

	@if ! grep "libz.a" -q ./zlib-1.2.5/Makefile; then \
		cd ./zlib-1.2.5/; \
		CC=rsdk-linux-gcc ./configure --prefix=$(DIR_USERS)/zlib-1.2.5/install; \
	fi

ifeq ($(CONFIG_APP_MT_DAAPD),y)
	@echo 'make zlib-1.2.3';
	cd ./zlib-1.2.3; \
	if [ ! -f zlib-1.2.3_config_done ]; then \
		./configure; \
		cp Makefile.zlib Makefile; \
		rm -f zlib-1.2.3_config_done; touch zlib-1.2.3_config_done; \
	fi; \
	make; \
	cd ..

	@echo 'make gdbm-1.8.3'; 
	cd ./gdbm-1.8.3; \
	if [ ! -f gdbm-1.8.3_config_done ]; then \
		./configure --host=mips-linux --build=i686-pc-linux-gnu; \
		cp Makefile.gdbm Makefile; \
		cp libtool.gdbm libtool; \
		rm -f gdbm-1.8.3_config_done; touch gdbm-1.8.3_config_done; \
	fi; \
	make; \
	cd ..

	@echo 'make libid3tag';
	cd ./libid3tag-0.15.1b; \
	if [ ! -f libid3tag_config_done ]; then \
		./configure --build=mips-linux --host=i686-pc-linux-gnu \
		--enable-shared \
		--disable-static \
		CPPFLAGS="-I../zlib-1.2.3/" \
		LDFLAGS="-L../zlib-1.2.3/"; \
		cp Makefile.id3tag Makefile; \
		cp libtool.id3tag libtool; \
		cd ./msvc++; \
		cp Makefile.id3tag Makefile; \
		cd ..;\
		rm -f libid3tag_config_done; touch libid3tag_config_done; \
	fi; \
	make; \
	cd ..

	@echo 'make mt-daapd'; \
	cd ./mt-daapd-0.2.4.2; \
	if [ ! -f mt-daapd_config_done ]; then \
		./configure --host=mips-linux \
		--with-gdbm-includes=../gdbm-1.8.3/ \
		--with-gdbm-libs=../gdbm-1.8.3/ \
		--with-id3tag=../libid3tag-0.15.1b/ \
		--with-id3tag-libs=../libid3tag-0.15.1b/.libs/ \
		LIBS="-lgdbm -L$(DIR_ROOT)/users/gdbm-1.8.3/ -lid3tag -L$(DIR_ROOT)/users/libid3tag-0.15.1b/.libs/ -lz -L$(DIR_ROOT)/users/zlib-1.2.3/" \
		ac_cv_func_setpgrp_void=yes; \
		cp Makefile.daapd Makefile; \
		cd ./src; cp Makefile.daapd Makefile; cd ..; \
		cd ./admin-root; cp Makefile.daapd Makefile; cd ..; \
		cd ./contrib; cp Makefile.daapd Makefile; cd ..; \
		cd ./debian; cp Makefile.daapd Makefile; cd ..; \
		rm -f mt-daapd_config_done; touch mt-daapd_config_done; \
	fi; \
	make; \
	cd ..
endif

ifeq ($(CONFIG_APP_RADVD),y)
	@if [ ! -e ./radvd-0.9.1/Makefile ]; then \
		cd ./radvd-0.9.1; \
		./configure --host=mips-linux; \
		touch Makefile.in; \
	fi
endif

prepare_lib:
ifeq ($(CONFIG_APP_LIBICONV),y)
	cd ./libiconv-1.14; \
	if [ ! -f libiconv_config_done ]; then \
		./configure --host=mips-linux --disable-nls --enable-static=yes --enable-shared=no --enable-fast-install=no; \
		cp Makefile.libiconv Makefile; \
		cp ./preload/Makefile.libiconv ./preload/Makefile; \
		rm -f libiconv_config_done; touch libiconv_config_done; \
	fi; \
	make; \
	cd ..
endif

prepare_app:
ifeq ($(CONFIG_APP_SNMP),y)
	cd ./net-snmp-5.4.1; \
	./configure.rtk-snmp CROSS_COMPILE=$(CROSS_COMPILE) APMIBINC=$(APMIBINC) APMIBLIB=$(APMIBLIB); \
	cd -
endif

$(lib_y): prepare prepare_lib
	[ ! -d "$@" ] || $(MAKE) -C $@ || exit $$?

$(app_y): prepare prepare_app
	[ ! -d "$@" ] || $(MAKE) -C $@ || exit $$?

#%_only:
#	$(MAKE) -C $(@:_only=)
#
#%_clean:
#	$(MAKE) -C $(@:_clean=) clean; true
#
#%_romfs:
#	$(MAKE) -C $(@:_romfs=) romfs

ulibs:
	[ -e $(DIR_ROMFS)/lib ] || mkdir -p $(DIR_ROMFS)/lib
	for i in $(lib_y) ; do \
		[ ! -d $$i ] || $(MAKE) -C $$i romfs || exit $$? ; \
	done

romfs:
	[ -e $(DIR_ROMFS)/bin ] || mkdir -p $(DIR_ROMFS)/bin
	for i in $(app_y) ; do \
		[ ! -d $$i ] || $(MAKE) -C $$i romfs || exit $$? ; \
	done

clean:
	-for i in $(app_y) $(app_n) $(app_) $(lib_y) $(lib_n) $(lib_) ; do \
		$(MAKE) -C $$i clean ; \
	done

prune:
	-for i in $(sort $(app_n) $(app_)) ; do \
		found=0; \
		for j in $(sort $(app_y)) ; do \
			if [ $$i = $$j ]; then \
				found=1; \
			fi; \
		done; \
	done

include Kconfig.mk
