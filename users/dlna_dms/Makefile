#DLNA_DMS_TOOLCHAIN_PATH=/home/matthewhui/dlna/rsdk-1.3.5-b39/linux/uclibc/bin
DLNA_DMS_INSTALL_PREFIX=$(shell pwd)/tmp
#DLNA_DMS_INSTALL_PREFIX=$(DIR_USERS)/libupnp-1.6.5/tmp
DLNA_DMS_ARCH=mips
DLNA_DMS_CROSS_COMPILER=rsdk-linux-

UPNP_FLAGS=--prefix=$(DLNA_DMS_INSTALL_PREFIX) CC=$(DLNA_DMS_CROSS_COMPILER)gcc --host=$(DLNA_DMS_ARCH)-linux --enable-static --enable-shared
#FFMPEG_FLAGS=--cross-prefix=$(DLNA_DMS_CROSS_COMPILER) --arch=$(DLNA_DMS_ARCH) --prefix=$(DLNA_DMS_INSTALL_PREFIX) --enable-cross-compile  --enable-gpl  --disable-ffplay --disable-ffserver --enable-static --enable-shared --disable-debug
#DLNA_FLAGS=--prefix=$(DLNA_DMS_INSTALL_PREFIX) --disable-shared --with-ffmpeg-dir=$(DLNA_DMS_INSTALL_PREFIX)/lib --cross-prefix=$(DLNA_DMS_CROSS_COMPILER) --cross-compile
DMS_FLAGS=--prefix=$(DLNA_DMS_INSTALL_PREFIX) --disable-dlna --cross-prefix=$(DLNA_DMS_CROSS_COMPILER) --cross-compile --with-libupnp-dir=$(DLNA_DMS_INSTALL_PREFIX)/lib --with-libdlna-dir=$(DLNA_DMS_INSTALL_PREFIX)/lib --enable-debug --disable-nls

#ICONV_FLAGS=--prefix=$(DLNA_DMS_INSTALL_PREFIX)/new CC=$(DLNA_DMS_CROSS_COMPILER)gcc --host=$(DLNA_DMS_ARCH)-linux --disable-nls --enable-static=yes --enable-shared=no --enable-fast-install=no


#PATH :=$(PATH):$(DLNA_DMS_TOOLCHAIN_PATH)
export PKG_CONFIG_PATH=$(DLNA_DMS_INSTALL_PREFIX)/lib/pkgconfig
export C_INCLUDE_PATH=$(DLNA_DMS_INSTALL_PREFIX)/include
export LD_LIBRARY_PATH=$(DLNA_DMS_INSTALL_PREFIX)/tmp/lib
#export LD_LIBRARY_PATH_LIBICONV=$(DLNA_DMS_INSTALL_PREFIX)/tmp/

.PHONY: all upnp ffmpeg dlna ushare libiconv
all: upnp ffmpeg dlna libiconv ushare
	

romfs:
	#$(ROMFSINST) ./tmp/bin/ffmpeg /bin/ffmpeg
	$(ROMFSINST) ./ushare/src/ushare /bin/ushare
	#$(ROMFSINST) ./tmp/etc/ushare.conf /etc/ushare.conf
test:
	@echo "$(DLNA_DMS_TOOLCHAIN)"
	@echo "$(DLNA_DMS_INSTALL_PREFIX)"
	@echo "$(DLNA_DMS_ARCH)"
	@echo "$(DLNA_DMS_CROSS_COMPILER)"
	env

upnp:
	cd libupnp-1.6.5 && ./configure $(UPNP_FLAGS)
	$(MAKE) -C libupnp-1.6.5 
	$(MAKE) -C libupnp-1.6.5 install

#ffmpeg:
#	cd ffmpeg_SVN-r13200 && ./configure $(FFMPEG_FLAGS)
#	$(MAKE) -C ffmpeg_SVN-r13200
#	$(MAKE) -C ffmpeg_SVN-r13200 install

#dlna:
#	cd libdlna-0.2.3 && ./configure $(DLNA_FLAGS)
#	$(MAKE) -C libdlna-0.2.3
#	$(MAKE) -C libdlna-0.2.3 install

libiconv:
#	cd ushare/libiconv-1.14 && ./configure $(ICONV_FLAGS)
#	cd ushare/libiconv-1.14/preload && cp Makefile.libiconv Makefile
#	$(MAKE) -C ushare/libiconv-1.14
#	cd ushare/libiconv-1.14/lib/.libs && cp libiconv.a $(DLNA_DMS_INSTALL_PREFIX)/lib/libiconv.a
		
ushare:
	cd ushare && ./configure $(DMS_FLAGS)
	$(MAKE) -C ushare
#	$(MAKE) -C ushare install
#	$(DLNA_DMS_CROSS_COMPILER)strip $(DLNA_DMS_INSTALL_PREFIX)/bin/ushare
	

clean:
	-$(MAKE) -C libupnp-1.6.5 clean
#	-$(MAKE) -C ffmpeg_SVN-r13200 clean
#	-$(MAKE) -C libdlna-0.2.3 clean
	-$(MAKE) -C ushare clean
#	-$(MAKE) -C ushare/libiconv-1.14 clean
	$(RM) -r $(DLNA_DMS_INSTALL_PREFIX)/*

distclean:
	-$(MAKE) -C libupnp-1.6.5 distclean
#	-$(MAKE) -C ffmpeg_SVN-r13200 distclean
#	-$(MAKE) -C libdlna-0.2.3 distclean
	-$(MAKE) -C ushare distclean
#	-$(MAKE) -C ushare/libiconv-1.14 distclean
	$(RM) -r $(DLNA_DMS_INSTALL_PREFIX)/*
	
