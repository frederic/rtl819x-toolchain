ifeq ($(DIR_LINUX),)
include ../../linux-2.6.30/.config
else
include $(DIR_LINUX)/.config
endif

RSDK_LIB_DIR=$(shell  echo $(PATH) | cut -f1 -d:)

SUBDIR_NTFS_3G := ntfs-3g-2010.10.2


all:
	if [ ! -e ./$(SUBDIR_NTFS_3G)/Makefile ]; then \
		cd ./$(SUBDIR_NTFS_3G); \
		LD="mips-linux-ld" CC="rsdk-linux-gcc -fpic -O3" ./configure --build=i686-pc-linux-gnu --host=mips-linux; \
	fi
	make -C $(SUBDIR_NTFS_3G)
	rm -f $(RSDK_LIB_DIR)/../lib/libntfs-3g.so
	rm -f $(RSDK_LIB_DIR)/../lib/libntfs-3g.so.80
	rm -f $(RSDK_LIB_DIR)/../lib/libntfs-3g.so.80.0.0
	cp -p ./$(SUBDIR_NTFS_3G)/libntfs-3g/.libs/libntfs-3g.so.80.0.0 $(RSDK_LIB_DIR)/../lib/libntfs-3g.so.80.0.0
	ln -s libntfs-3g.so.80.0.0 $(RSDK_LIB_DIR)/../lib/libntfs-3g.so
	ln -s libntfs-3g.so.80.0.0 $(RSDK_LIB_DIR)/../lib/libntfs-3g.so.80

	
romfs:
	$(ROMFSINST) $(SUBDIR_NTFS_3G)/src/.libs/ntfs-3g /bin/ntfs-3g


clean:
	make -C $(SUBDIR_NTFS_3G) distclean
	
distclean:
	make -C $(SUBDIR_NTFS_3G) distclean
