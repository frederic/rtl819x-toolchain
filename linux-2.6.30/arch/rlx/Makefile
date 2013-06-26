#
# This file is subject to the terms and conditions of the GNU General Public
# License.  See the file "COPYING" in the main directory of this archive
# for more details.
#
# Copyright (C) 1994, 95, 96, 2003 by Ralf Baechle
# DECStation modifications by Paul M. Antoine, 1996
# Copyright (C) 2002, 2003, 2004  Maciej W. Rozycki
#
# This file is included by the global makefile so that you can add your own
# architecture-specific flags and dependencies. Remember to do have actions
# for "archclean" cleaning up for this architecture.
#

#
# Select the object file format to substitute into the linker script.
#
ifdef CONFIG_CPU_LITTLE_ENDIAN
32bit-bfd		= elf32-tradlittlemips
32bit-emul		= elf32ltsmip
else
32bit-bfd		= elf32-tradbigmips
32bit-emul		= elf32btsmip
endif

CROSS_COMPILE  := rsdk-linux-
UTS_MACHINE    := rlx

ld-emul			= $(32bit-emul)
vmlinux-32		= vmlinux

cflags-y := -ffunction-sections
cflags-y += $(call cc-option, -mno-check-zero-division)
cflags-y += -mabi=32

#
# GCC uses -G 0 -mabicalls -fpic as default.  We don't want PIC in the kernel
# code since it only slows down the whole thing.  At some point we might make
# use of global pointer optimizations but their use of $28 conflicts with
# the current pointer optimization.
#
cflags-y			+= -G 0 -mno-abicalls -fno-pic -pipe
cflags-y			+= -msoft-float
LDFLAGS_vmlinux     += -G 0 -static -n -nostdlib
MODFLAGS			+= -mlong-calls

cflags-y += -ffreestanding

#
# We explicitly add the endianness specifier if needed, this allows
# to compile kernels with a toolchain for the other endianness. We
# carefully avoid to add it redundantly because gcc 3.3/3.4 complains
# when fed the toolchain default!
#
# Certain gcc versions upto gcc 4.1.1 (probably 4.2-subversion as of
# 2006-10-10 don't properly change the predefined symbols if -EB / -EL
# are used, so we kludge that here.  A bug has been filed at
# http://gcc.gnu.org/bugzilla/show_bug.cgi?id=29413.
#
undef-all += -UMIPSEB -U_MIPSEB -U__MIPSEB -U__MIPSEB__
undef-all += -UMIPSEL -U_MIPSEL -U__MIPSEL -U__MIPSEL__
predef-be += -DMIPSEB -D_MIPSEB -D__MIPSEB -D__MIPSEB__
predef-le += -DMIPSEL -D_MIPSEL -D__MIPSEL -D__MIPSEL__
cflags-$(CONFIG_CPU_BIG_ENDIAN)		+= -EB $(undef-all) $(predef-be)
cflags-$(CONFIG_CPU_LITTLE_ENDIAN)	+= -EL $(undef-all) $(predef-le)

#
# BSP Makefile
#
include arch/rlx/bsp/Makefile

cflags-y                    += -I$(srctree)/arch/rlx/include/asm/mach-generic
drivers-$(CONFIG_PCI)       += arch/rlx/pci/

ifdef CONFIG_CPU_LITTLE_ENDIAN
JIFFIES			= jiffies_64
else
JIFFIES			= jiffies_64 + 4
endif

#
# Automatically detect the build format. By default we choose
# the elf format according to the load address.
# We can always force a build with a 64-bits symbol format by
# passing 'KBUILD_SYM32=no' option to the make's command line.
#
KBUILD_AFLAGS	+= $(cflags-y)
KBUILD_CFLAGS	+= $(cflags-y) \
			-D"VMLINUX_LOAD_ADDRESS=$(load-y)"

LDFLAGS			+= -m $(ld-emul)

CHECKFLAGS += $(shell $(CC) $(KBUILD_CFLAGS) -dM -E -xc /dev/null | \
	egrep -vw '__GNUC_(|MINOR_|PATCHLEVEL_)_' | \
	sed -e 's/^\#define /-D/' -e "s/ /='/" -e "s/$$/'/")

OBJCOPYFLAGS		+= --remove-section=.reginfo

#
# Choosing incompatible machines durings configuration will result in
# error messages during linking.  Select a default linkscript if
# none has been choosen above.
#

CPPFLAGS_vmlinux.lds := \
	$(KBUILD_CFLAGS) \
	-D"LOADADDR=$(load-y)" \
	-D"JIFFIES=$(JIFFIES)" \
	-D"DATAOFFSET=$(if $(dataoffset-y),$(dataoffset-y),0)" \
	-D"ZZ_DRAMSIZE_ZZ=8k" \
	-D"ZZ_IRAMSIZE_ZZ=8k" \
	-D"ZZ_L2RAMSIZE_ZZ=32k"

head-y := arch/rlx/kernel/head.o arch/rlx/kernel/init_task.o
libs-y += arch/rlx/lib/     # Library support
#libs-y += arch/rlx/fw/lib/  # Firmware support
core-y += arch/rlx/kernel/ arch/rlx/mm/

drivers-$(CONFIG_OPROFILE)	+= arch/rlx/oprofile/

#
# Targets
#
vmlinux.32: vmlinux
	$(OBJCOPY) -O $(32bit-bfd) $(OBJCOPYFLAGS) $< $@

makeboot =$(Q)$(MAKE) $(build)=arch/rlx/boot VMLINUX=$(vmlinux-32) $(1)

all: vmlinux.32

vmlinux.bin: $(vmlinux-32)
	+@$(call makeboot,$@)

archprepare:
#ifdef CONFIG_MIPS32_O32
#	@echo '  Checking missing-syscalls for O32'
#	$(Q)$(MAKE) $(build)=. missing-syscalls EXTRA_CFLAGS="-mabi=32"
#endif

install:
	$(Q)install -D -m 755 vmlinux $(INSTALL_PATH)/vmlinux-$(KERNELRELEASE)
	$(Q)install -D -m 644 .config $(INSTALL_PATH)/config-$(KERNELRELEASE)
	$(Q)install -D -m 644 System.map $(INSTALL_PATH)/System.map-$(KERNELRELEASE)

archclean:
	@$(MAKE) $(clean)=arch/rlx/boot

define archhelp
	echo '  install              - install kernel into $(INSTALL_PATH)'
	echo '  vmlinux.ecoff        - ECOFF boot image'
	echo '  vmlinux.bin          - Raw binary boot image'
	echo '  vmlinux.srec         - SREC boot image'
	echo
	echo '  These will be default as apropriate for a configured platform.'
endef

CLEAN_FILES += vmlinux.32 vmlinux.bin
