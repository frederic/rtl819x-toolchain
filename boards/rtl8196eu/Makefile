
# Realtek Semiconductor Corp.
#
# Makefile -- Build instructions for Realtek RLXOCP with initramfs
#
# Tony Wu (tonywu@realtek.com.tw)
# Dec. 12, 2007
#

VERSION = RTL8196EU v1.0
#DIRS = bin dev etc home lib mnt proc usr var sys tmp
DIRS = bin dev dev/voip etc home lib mnt proc usr var sys 

.PHONY: romfs image

include $(DIR_LINUX)/.config
include $(DIR_ROOT)/.config 
include $(DIR_USERS)/.config 
include $(DIR_USERS)/boa/src/svn_version.h

ifeq ($(CONFIG_APP_BOA), y)
MGBIN = $(DIR_USERS)/boa/tools/mgbin
else
MGBIN = $(DIR_USERS)/goahead-2.1.1/LINUX/mgbin
endif

FW_BIN = image/fw.bin

FW_NOWEB_BIN = image/fw_noweb.bin

ifdef CONFIG_ROOTFS_SQUASH
ROOT_BIN = image/root.bin
endif

ifndef CONFIG_ROOTFS_JFFS2
LINUX_BIN  = image/linux.bin
endif

ifeq ($(CONFIG_APP_BOA), y)
ifdef CONFIG_RTL_WEBPAGES_IN_ROOTFS
WEBPAGE_BIN = 
else
WEBPAGE_BIN = image/webpages.bin
WEBIMAGE_BIN = webpages.bin
endif # CONFIG_RTL_WEBPAGES_IN_ROOTFS
else
ifdef CONFIG_RTL_WEBPAGES_IN_ROOTFS
WEBPAGE_BIN = 
ifdef CONFIG_RTL_8198_AP_ROOT
WEBIMAGE_GZ = webpages-ap.gz
WEB_NAME = web-gw
else	#!CONFIG_RTL_8198_AP_ROOT
WEBIMAGE_GZ = webpages-gw.gz
WEB_NAME = web-gw
endif	#CONFIG_RTL_8198_AP_ROOT
else	#!CONFIG_RTL_WEBPAGES_IN_ROOTFS
ifdef CONFIG_RTL_8198_AP_ROOT
WEBPAGE_BIN = image/webpages-ap.bin
WEBIMAGE_BIN = webpages-ap.bin
else	#!CONFIG_RTL_8198_AP_ROOT
WEBPAGE_BIN = image/webpages-gw.bin
WEBIMAGE_BIN = webpages-gw.bin
endif	#CONFIG_RTL_8198_AP_ROOT
endif	#CONFIG_RTL_WEBPAGES_IN_ROOTFS
endif

ifdef CONFIG_APP_SAMBA
SAMBA_FLG = 1 
else
SAMBA_FLG = 0 
endif

ifeq ($(CONFIG_MODEL_RTL8196EU_MP), y)
ifeq ($(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN_D), y)
MP_NFJROM = 96eu_92c_92d_nfjrom
else
ifeq ($(CONFIG_RTL_92C_SUPPORT), y)
MP_NFJROM = 96eu_92c_nfjrom_26
else
ifeq ($(CONFIG_RTL_92D_SUPPORT), y)
MP_NFJROM = 96eu_92d_nfjrom_26
else
MP_NFJROM = 96eu_nfjrom_26
endif
endif
endif
else
MP_NFJROM = nfjrom
endif

romfs:
	rm -rf $(DIR_ROMFS)
	[ -d $(DIR_ROMFS)/$$i ] || mkdir -p $(DIR_ROMFS)
	for i in $(DIRS); do \
		[ -d $(DIR_ROMFS)/$$i ] || mkdir -p $(DIR_ROMFS)/$$i; \
	done
	mkdir $(DIR_ROMFS)/dev/misc
	mkdir $(DIR_ROMFS)/dev/pts
	#chmod 777 $(DIR_ROMFS)/tmp
	$(ROMFSINST) etc /etc

ifdef CONFIG_APP_VSFTPD
	mkdir -p $(DIR_ROMFS)/usr/share/empty
endif
	$(ROMFSINST) -s /var/ppp /etc/ppp
ifdef CONFIG_APP_LOGIN_CONSOLE
	$(ROMFSINST) -s /var/shadow /etc/shadow
endif
	$(ROMFSINST) home /home
	$(ROMFSINST) -s bin/init /init
ifneq ($(CONFIG_APP_BOA), y)
ifdef CONFIG_RTL_WEBPAGES_IN_ROOTFS
	cp $(DIR_USERS)/goahead-2.1.1/LINUX/$(WEBIMAGE_GZ) ./ 
	tar xvfz $(WEBIMAGE_GZ)
	mv $(WEB_NAME) $(DIR_ROMFS)/web
	rm -rf $(WEBIMAGE_GZ)
	$(ROMFSINST) -s /var/config.dat /web/config.dat
else
	$(ROMFSINST) -s /var/web /web
endif
endif
	$(ROMFSINST) -s /var/tmp /tmp
ifndef CONFIG_MODEL_RTL8196EU_MP
ifdef CONFIG_RTL_SDRAM_GE_32M
	rm -f $(DIR_ROMFS)/etc/init.d/rcS_16M
	chmod 755 $(DIR_ROMFS)/etc/init.d/rcS_32M
	ln -s rcS_32M $(DIR_ROMFS)/etc/init.d/rcS
else
	rm -f $(DIR_ROMFS)/etc/init.d/rcS_32M
	chmod 755 $(DIR_ROMFS)/etc/init.d/rcS_16M
	ln -s rcS_16M $(DIR_ROMFS)/etc/init.d/rcS
endif
else
	chmod 755 $(DIR_ROMFS)/etc/init.d/rcS
endif

ifndef CONFIG_MODEL_RTL8196EU_MP
ifdef CONFIG_RTL_ULINKER
	rm -f $(DIR_ROMFS)/etc/init.d/rcS
	mv $(DIR_ROMFS)/etc/init.d/rcS_32M_ulinker $(DIR_ROMFS)/etc/init.d/rcS
else
	rm -f $(DIR_ROMFS)/etc/init.d/rcS_32M_ulinker
	rm -f $(DIR_ROMFS)/etc/autorun.img
endif
endif

ifdef CONFIG_RTL_WEBPAGES_IN_ROOTFS
	sed -i "s/flash extr \/web/#flash extr \/web/" $(DIR_ROMFS)/etc/init.d/rcS
endif
ifeq ($(CONFIG_APP_BOA), y)
	sed -i "s/webs&/boa/" $(DIR_ROMFS)/etc/init.d/rcS
endif


ifdef CONFIG_APP_LOGIN_CONSOLE
	sed -i 's/^::respawn:-\/bin\/sh/::respawn:\/bin\/getty -L ttyS0 38400 vt100/' $(DIR_ROMFS)/etc/inittab
endif
ifeq ($(CONFIG_RTL8192CD),m)
	sed -i "14i insmod /lib/modules/2.6.30.9/kernel/drivers/net/wireless/rtl8192cd/rtl8192cd.ko" $(DIR_ROMFS)/etc/init.d/rcS
endif
	echo "$(VERSION) -- " `date` > $(DIR_ROMFS)/etc/version
	echo "The SDK version is: $(CURRENT_VERSION)" >> $(DIR_ROMFS)/etc/version
	echo "Ethernet driver version is: $(NIC_DRIVER_VERSION)" >> $(DIR_ROMFS)/etc/version
	echo "Wireless driver version is: $(WLAN_DRIVER_VERSION)" >> $(DIR_ROMFS)/etc/version
	echo "Fastpath source version is: $(FASTPATH_VERSION)" >> $(DIR_ROMFS)/etc/version
	echo "Feature support version is: $(FEATURE_VERSION)" >> $(DIR_ROMFS)/etc/version

	# add by pk
	$(DIR_TOOLS)/mkimg USE_SAMBA=$(SAMBA_FLG) USERS_DIR="$(DIR_USERS)" LINUX_DIR="$(DIR_LINUX)" TOOLS_DIR="$(DIR_TOOLS)" TOOLCHAIN_DIR="$(DIR_RSDK)" || exit $$?
	$(DIR_TOOLS)/mkromfs USERS_DIR="$(DIR_USERS)" LINUX_DIR="$(DIR_LINUX)" TOOLS_DIR="$(DIR_TOOLS)" TOOLCHAIN_DIR="$(DIR_RSDK)" || exit $$?

image:
	[ -d $(DIR_IMAGE) ] || mkdir -p $(DIR_IMAGE)
	rm -rf $(DIR_IMAGE)/* root.bin
	$(DIR_TOOLS)/mkimg USE_SAMBA=$(SAMBA_FLG) USERS_DIR="$(DIR_USERS)" LINUX_DIR="$(DIR_LINUX)" TOOLS_DIR="$(DIR_TOOLS)" TOOLCHAIN_DIR="$(DIR_RSDK)" USE_SAMBA=1 || exit $$?
	$(MAKE) -C $(DIR_LINUX)/rtkload
	cp $(DIR_LINUX)/vmlinux $(DIR_IMAGE)/vmlinux.elf
ifndef CONFIG_ROOTFS_JFFS2
	cp $(DIR_LINUX)/rtkload/linux.bin $(DIR_IMAGE)/linux.bin
	cp $(DIR_LINUX)/rtkload/nfjrom $(DIR_IMAGE)/$(MP_NFJROM)
endif
ifdef CONFIG_RTL_8198_INBAND_AP
	[ ! -f root.bin ] || cp -f root.bin $(DIR_IMAGE)/root.bin; \
	$(MGBIN) -c -o $(FW_NOWEB_BIN) $(ROOT_BIN) $(LINUX_BIN);
else
	@egrep "^CONFIG_MODEL_RTL8196EU_MP=y" $(DIR_ROOT)/.config > MP.test; \
	if [ -s MP.test ] ; then \
		echo "It it MP image"; \
	else \
		[ ! -f root.bin ] || cp -f root.bin $(DIR_IMAGE)/root.bin; \
		egrep "^CONFIG_APP_BOA=y" $(DIR_USERS)/.config > BOA.test; \
		if [ -s BOA.test ] ; then \
			cp $(DIR_USERS)/boa/html/$(WEBIMAGE_BIN) $(DIR_IMAGE)/$(WEBIMAGE_BIN); \
			$(MGBIN) -c -o $(FW_BIN) $(ROOT_BIN) $(WEBPAGE_BIN) $(LINUX_BIN); \
			cd $(DIR_USERS)/boa/defconfig; \
			mv *.dat $(DIR_ROOT)/boards/rtl8196eu/image; \
			cd -; \
		else \
			egrep "^CONFIG_APP_GOAHEAD=y" $(DIR_USERS)/.config > GOAHEAD.test; \
			if [ -s GOAHEAD.test ] ; then \
				cp $(DIR_USERS)/goahead-2.1.1/LINUX/$(WEBIMAGE_BIN) $(DIR_IMAGE)/$(WEBIMAGE_BIN); \
				$(MGBIN) -c -o $(FW_BIN) $(ROOT_BIN) $(WEBPAGE_BIN) $(LINUX_BIN); \
				cd $(DIR_USERS)/goahead-2.1.1/LINUX; \
				mv *.dat $(DIR_ROOT)/boards/rtl8196eu/image; \
				cd -; \
			fi; \
		fi; \
	fi; \
	rm *.test
endif
