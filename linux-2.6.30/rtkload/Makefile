# The kernel tree you do builds in.
# Uncomment if you're building for the emulator
include ../.config
include ../../.config	# to check toolchain CONFIG_RSDK_rsdk-1.5.4-5281-EB-2.6.30-0.9.30.3-uls-101110
DOQUIET = false
#DOQUIET = true 
#EMULATOR = true
#KERNEL_ROOT = /home/john/RealTek/linux-2.5/linux
KERNEL_ROOT = ..
ifeq ($(wildcard $(DIR_USERS)/boa/tools/cvimg),)
ifeq ($(wildcard $(DIR_USERS)/goahead-2.1.1/LINUX/cvimg),)
CVIMG=./cvimg
else
CVIMG=$(DIR_USERS)/goahead-2.1.1/LINUX/cvimg
endif
.NOTPARALLEL : all
else
CVIMG=$(DIR_USERS)/boa/tools/cvimg
endif
# ---- shouldn't need to modify below this line.

ifdef EMULATOR
EMUOPTS = -DEMULATOR
LDSCRIPT = ld-emu.script
else
EMUOPTS =
LDSCRIPT = ld.script
endif

COPTIONS = -DROM_MEMORY -DCOMPRESSED_KERNEL -D__KERNEL__

#SOURCES = vsprintf.c string.c ctype.c prom_printf.c misc.c hfload.c start.S read_memory.c
ifdef BZ2_COMPRESS
	SOURCES = misc.c hfload.c start.S cache.c
else
ifdef GZIP_COMPRESS
	SOURCES = string.c ctype.c misc.c hfload.c start.S read_memory.c cache.c
else
	SOURCES = string.c ctype.c misc.c hfload.c start.S read_memory.c cache.c LzmaDecode.c
endif	
endif

LOADER_FILES = hfload.o read_memory.o
#LOADER_FILES = hfload.o
ifdef BZ2_COMPRESS
	SUPPORT_FILES = misc.o cache.o
else
ifdef GZIP_COMPRESS
	SUPPORT_FILES = vsprintf.o prom_printf.o string.o ctype.o misc.o cache.o	
else
ifeq ($(strip $(DOQUIET)),true)
    SUPPORT_FILES = string.o ctype.o misc.o cache.o LzmaDecode.o 
else
    SUPPORT_FILES = vsprintf.o prom_printf.o string.o ctype.o misc.o cache.o LzmaDecode.o  
endif
endif	
endif
    
CFLAGS =-Os -g -fno-pic -mno-abicalls $(EMUOPTS)
ifeq ($(strip $(DOQUIET)),true)
CFLAGS  += $(WARNINGS)  -D__DO_QUIET__
endif
CFLAGS += -DEMBEDDED -I$(KERNEL_ROOT)/include/linux -I$(KERNEL_ROOT)/include -I$(KERNEL_ROOT)/lib $(COPTIONS) -G 0
CFLAGS += -I$(KERNEL_ROOT)/arch/rlx/bsp -I$(KERNEL_ROOT)/arch/rlx/include -I$(KERNEL_ROOT)/arch/rlx/include/asm/mach-generic 
#ASFLAGS = -g $(EMUOPTS) -DEMBEDDED -I$(KERNEL_ROOT)/include -I$(KERNEL_ROOT)/arch/rlx/include
#CFLAGS +=  -I$(KERNEL_ROOT)/arch/mips/include -I$(KERNEL_ROOT)/arch/mips/include/asm/mach-generic 
#ASFLAGS = -g $(EMUOPTS) -DEMBEDDED -I$(KERNEL_ROOT)/include -I$(KERNEL_ROOT)/arch/mips/include
ASFLAGS = -g -fno-pic -mno-abicalls $(EMUOPTS) -DEMBEDDED -I$(KERNEL_ROOT)/include -I$(KERNEL_ROOT)/arch/rlx/include

LDFLAGS=-static -nostdlib

ifdef BZ2_COMPRESS
CFLAGS += -DBZ2_COMPRESS
ASFLAGS += -DBZ2_COMPRESS
else
ifndef GZIP_COMPRESS
CFLAGS += -DLZMA_COMPRESS
ASFLAGS += -DLZMA_COMPRESS
endif
endif

START_FILE = start.o


#RTL_819X normal LOAD_START_ADDR and FLASH_OFFSET
ifdef CONFIG_RTL_819X
LOAD_START_ADDR=0x80500000
FLASH_OFFSET=30000
endif

ifdef CONFIG_RTL_8196B_GW
ifdef CONFIG_MTD_RTL_8196_SPI
LOAD_START_ADDR=0x80A00000
else
LOAD_START_ADDR=0x80500000
endif
ifdef CONFIG_RTL_8196B_GW_8M
FLASH_OFFSET=10000
else
ifeq ($(CONFIG_RTL_FLASH_MAPPING_ENABLE),y)
ifeq ($(CONFIG_RTL_LINUX_IMAGE_OFFSET),)
FLASH_OFFSET=30000
else
FLASH_OFFSET=$(CONFIG_RTL_LINUX_IMAGE_OFFSET)
endif
else
FLASH_OFFSET=30000
endif
endif
endif

ifdef CONFIG_RTL_8198_GW
ifdef CONFIG_MTD_RTL_8196_SPI
LOAD_START_ADDR=0x80A00000
else
LOAD_START_ADDR=0x80500000
endif
ifdef CONFIG_RTL_8196B_GW_8M
FLASH_OFFSET=10000
else
FLASH_OFFSET=30000
endif
endif

ifdef CONFIG_RTL_8196B_ICT
LOAD_START_ADDR=0x80500000
FLASH_OFFSET=30000
endif

ifdef CONFIG_RTL_865X_PANAHOST
LOAD_START_ADDR=0x80500000
FLASH_OFFSET=20000
endif

ifdef CONFIG_RTL_8197B_PANA
LOAD_START_ADDR=0x80700000
FLASH_OFFSET=30000
endif

ifdef CONFIG_RTL_8196B_AP_ROOT
LOAD_START_ADDR=0x80500000
FLASH_OFFSET=30000
endif

ifeq ($(CONFIG_RTL_FLASH_MAPPING_ENABLE),y)
ifeq ($(CONFIG_RTL_LINUX_IMAGE_OFFSET),)
FLASH_OFFSET=20000
else
FLASH_OFFSET=$(CONFIG_RTL_LINUX_IMAGE_OFFSET)
endif
else
FLASH_OFFSET=20000
endif

ifdef CONFIG_RTK_VOIP_BOARD
LOAD_START_ADDR=0x80C00000
endif

CV_SIGNATURE=
ifdef CONFIG_RTL_8197B_PANA
#CV_OPTION=linux
CV_OPTION=signature
CV_SIGNATURE=csys
else
ifdef CONFIG_SQUASHFS
CV_OPTION=linux-ro
else
CV_OPTION=linux
endif
endif

ifdef CONFIG_RTL_8198_NFBI_BOARD
LOAD_START_ADDR=0x80700000
FLASH_OFFSET=30000
CV_OPTION=signature
CV_SIGNATURE=csys
endif

#ifdef CONFIG_RTL_8196C_iNIC
#LOAD_START_ADDR=0x80500000
#FLASH_OFFSET=30000
#CV_OPTION=signature
#CV_SIGNATURE=cs6c
#endif

CROSS_COMPILE ?= rsdk-linux-

LD=$(CROSS_COMPILE)ld
CC=$(CROSS_COMPILE)gcc
CPP=$(CROSS_COMPILE)gcc
STRIP=$(CROSS_COMPILE)strip
AS=$(CROSS_COMPILE)as
NM=$(CROSS_COMPILE)nm
OBJCOPY=$(CROSS_COMPILE)objcopy

SEDFLAGS	= s/LOAD_ADDR/$(LOAD_START_ADDR)/;

ifdef CONFIG_RTK_VOIP_BOARD
STRIP-OPTIONS-$(CONFIG_RSDK_rsdk-1.5.4-5281-EB-2.6.30-0.9.30.3-uls-101110) = --remove-section=.bss
endif

# jffs2 related definitions 
DIR_ROMFS_BOOT = $(DIR_ROMFS)/boot/
MKJFFS2 = ./mkfs.jffs2
MKIMAGE_UBOOT = ./mkimage-uboot

O_TARGET := rtk
obj-y		:= vmlinux_img.o $(START_FILE) $(LOADER_FILES) $(SUPPORT_FILES)

ifdef CONFIG_ROOTFS_JFFS2
all: rtk-clean rtk-vmlinux_img vmlinux_img.gzip.uboot.jffs2
else
all: rtk-clean rtk-vmlinux_img $(START_FILE) $(LOADER_FILES) $(SUPPORT_FILES)
endif

#memload-partial
ifdef CONFIG_ROOTFS_JFFS2
jffs2_no_this:
endif
	$(CC) -fno-pic -mno-abicalls -Os -fomit-frame-pointer -D__KERNEL__ -c vmlinux_img.c -o vmlinux_img.o	
	$(OBJCOPY) --add-section .vmlinux=vmlinux_img.gz vmlinux_img.o
	@sed "$(SEDFLAGS)" < ld.script.in > $(LDSCRIPT)
	$(LD) $(LDFLAGS) -G 0 -T $(LDSCRIPT) -o memload-partial $(START_FILE) $(LOADER_FILES) $(SUPPORT_FILES) vmlinux_img.o
	$(NM) memload-partial | grep -v '\(compiled\)\|\(\.o$$\)\|\( [aU] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)' | sort > system.map
	
	cp memload-partial memload-full
	$(OBJCOPY) -Obinary memload-full nfjrom
	$(CVIMG) $(CV_OPTION) nfjrom linux.bin $(LOAD_START_ADDR) $(FLASH_OFFSET) $(CV_SIGNATURE)
#	@./size_chk vmlinux_img $(LOAD_START_ADDR)
	@$(CVIMG) size_chk vmlinux_img $(LOAD_START_ADDR) || exit $$?
ifeq ($(CONFIG_BLK_DEV_INITRD),y)
	@echo "RAMFS no need to check flash size"
else
ifeq ($(CONFIG_RTL_FLASH_MAPPING_ENABLE),y)
	@$(CVIMG) flash_size_chk linux.bin || exit $$?
endif
endif

rtk-clean:
	rm -f *.o memload system.map nfjrom memload-partial memload-full vmlinux_img.gz target target.img strip1 linux.bin vmlinux-stripped  $(LDSCRIPT) vmlinux_img vmlinux_img.gzip.uboot.jffs2
	
rtk-vmlinux_img: 
	cp $(KERNEL_ROOT)/vmlinux vmlinux-stripped
	$(STRIP) vmlinux-stripped $(STRIP-OPTIONS-y)
	$(OBJCOPY) -Obinary vmlinux-stripped vmlinux_img
	rm -f vmlinux_img.gz
ifndef CONFIG_ROOTFS_JFFS2
ifdef BZ2_COMPRESS
	cat vmlinux_img | bzip2 -9v > vmlinux_img.gz
else
ifdef GZIP_COMPRESS
	cat vmlinux_img | gzip -9v > vmlinux_img.gz	
else
	./lzma e vmlinux_img vmlinux_img.gz
endif
endif
	$(CVIMG) vmlinuxhdr vmlinux_img.gz vmlinux_img.gz $(KERNEL_ROOT)/vmlinux
endif # CONFIG_ROOTFS_JFFS2

target:	nfjrom $(BOOT_CODE) $(EXT2_IMG)
# Pading boot code to 512 bytes long (but only 256 bytes are allowed, 512 is to make it page alignment)
	$(OBJCOPY) -Obinary --pad-to=0x80000200 $(BOOT_CODE) bootcode
	$(OBJCOPY) -Obinary --remove-section=.bss --remove-section=.data --remove-section=.mdebug  --pad-to=0x806FFE00 memload-full nandrom
	$(LD) -G0 -Ttarget.script -o target  -bbinary bootcode -bbinary nandrom -bbinary $(EXT2_IMG)
	$(OBJCOPY) -Obinary target target.img		 
	cp -f ./target.img /tftpboot
depend:
	rm -f .depend
	$(CC) $(CFLAGS) -MM $(SOURCES) >.depend

bz2:
	@make BZ2_COMPRESS=1

gzip:
	@make GZIP_COMPRESS=1

vmlinux_img.gzip.uboot.jffs2: 
	cat vmlinux_img | gzip -9v > vmlinux_img.gzip
	mkdir -p $(DIR_ROMFS_BOOT)
	$(MKIMAGE_UBOOT) -A mips -O linux -T kernel -C gzip -a 80000000 -e 80000000 -n 8954c_kernel -d vmlinux_img.gzip $(DIR_ROMFS_BOOT)/vmlinux_img.gzip.uboot
	$(MKJFFS2) -n -p -b -e $(CONFIG_JFFS2_ERASE_SIZE) -o $(DIR_IMAGE)/vmlinux_img.gzip.uboot.jffs2 -d $(DIR_ROMFS)

