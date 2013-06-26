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

KBUILD_DEFCONFIG := ip22_defconfig

#
# Select the object file format to substitute into the linker script.
#
32bit-tool-archpref	= mips
64bit-tool-archpref	= mips64
32bit-bfd		= elf32-tradbigmips
64bit-bfd		= elf64-tradbigmips
32bit-emul		= elf32btsmip
64bit-emul		= elf64btsmip

ifdef CONFIG_32BIT
tool-archpref		= $(32bit-tool-archpref)
UTS_MACHINE		:= mips
endif

ifneq ($(SUBARCH),$(ARCH))
  ifeq ($(CROSS_COMPILE),)
    CROSS_COMPILE := $(call cc-cross-prefix, $(tool-archpref)-linux-  $(tool-archpref)-linux-gnu-  $(tool-archpref)-unknown-linux-gnu-)
  endif
endif

cflags-y := -ffunction-sections
cflags-y += $(call cc-option, -mno-check-zero-division)

ifdef CONFIG_32BIT
ld-emul			= $(32bit-emul)
vmlinux-32		= vmlinux
vmlinux-64		= vmlinux.64

cflags-y		+= -mabi=32
endif

ifdef CONFIG_64BIT
ld-emul			= $(64bit-emul)
vmlinux-32		= vmlinux.32
vmlinux-64		= vmlinux

cflags-y		+= -mabi=64
endif

all-$(CONFIG_BOOT_ELF32)	:= $(vmlinux-32)
all-$(CONFIG_BOOT_ELF64)	:= $(vmlinux-64)

#
# GCC uses -G 0 -mabicalls -fpic as default.  We don't want PIC in the kernel
# code since it only slows down the whole thing.  At some point we might make
# use of global pointer optimizations but their use of $28 conflicts with
# the current pointer optimization.
#
# The DECStation requires an ECOFF kernel for remote booting, other MIPS
# machines may also.  Since BFD is incredibly buggy with respect to
# crossformat linking we rely on the elf2ecoff tool for format conversion.
#
cflags-y			+= -G 0 -mno-abicalls -fno-pic -pipe
cflags-y			+= -msoft-float
LDFLAGS_vmlinux			+= -G 0 -static -n -nostdlib
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
cflags-$(CONFIG_CPU_BIG_ENDIAN)		+= $(shell $(CC) -dumpmachine |grep -q 'mips.*el-.*' && echo -EB $(undef-all) $(predef-be))
cflags-$(CONFIG_CPU_LITTLE_ENDIAN)	+= $(shell $(CC) -dumpmachine |grep -q 'mips.*el-.*' || echo -EL $(undef-all) $(predef-le))

cflags-$(CONFIG_CPU_HAS_SMARTMIPS)	+= $(call cc-option,-msmartmips)

cflags-$(CONFIG_SB1XXX_CORELIS)	+= $(call cc-option,-mno-sched-prolog) \
				   -fno-omit-frame-pointer

#
# CPU-dependent compiler/assembler options for optimization.
#
cflags-$(CONFIG_CPU_R3000)	+= -march=5181
cflags-$(CONFIG_CPU_TX39XX)	+= -march=r3900
cflags-$(CONFIG_CPU_R6000)	+= -march=r6000 -Wa,--trap
cflags-$(CONFIG_CPU_R4300)	+= -march=r4300 -Wa,--trap
cflags-$(CONFIG_CPU_VR41XX)	+= -march=r4100 -Wa,--trap
cflags-$(CONFIG_CPU_R4X00)	+= -march=r4600 -Wa,--trap
cflags-$(CONFIG_CPU_TX49XX)	+= -march=r4600 -Wa,--trap
cflags-$(CONFIG_CPU_LOONGSON2)	+= -march=r4600 -Wa,--trap
cflags-$(CONFIG_CPU_MIPS32_R1)	+= $(call cc-option,-march=mips32,-mips32 -U_MIPS_ISA -D_MIPS_ISA=_MIPS_ISA_MIPS32) \
			-Wa,-mips32 -Wa,--trap
cflags-$(CONFIG_CPU_MIPS32_R2)	+= $(call cc-option,-march=mips32r2,-mips32r2 -U_MIPS_ISA -D_MIPS_ISA=_MIPS_ISA_MIPS32) \
			-Wa,-mips32r2 -Wa,--trap
cflags-$(CONFIG_CPU_MIPS64_R1)	+= $(call cc-option,-march=mips64,-mips64 -U_MIPS_ISA -D_MIPS_ISA=_MIPS_ISA_MIPS64) \
			-Wa,-mips64 -Wa,--trap
cflags-$(CONFIG_CPU_MIPS64_R2)	+= $(call cc-option,-march=mips64r2,-mips64r2 -U_MIPS_ISA -D_MIPS_ISA=_MIPS_ISA_MIPS64) \
			-Wa,-mips64r2 -Wa,--trap
cflags-$(CONFIG_CPU_R5000)	+= -march=r5000 -Wa,--trap
cflags-$(CONFIG_CPU_R5432)	+= $(call cc-option,-march=r5400,-march=r5000) \
			-Wa,--trap
cflags-$(CONFIG_CPU_R5500)	+= $(call cc-option,-march=r5500,-march=r5000) \
			-Wa,--trap
cflags-$(CONFIG_CPU_NEVADA)	+= $(call cc-option,-march=rm5200,-march=r5000) \
			-Wa,--trap
cflags-$(CONFIG_CPU_RM7000)	+= $(call cc-option,-march=rm7000,-march=r5000) \
			-Wa,--trap
cflags-$(CONFIG_CPU_RM9000)	+= $(call cc-option,-march=rm9000,-march=r5000) \
			-Wa,--trap
cflags-$(CONFIG_CPU_SB1)	+= $(call cc-option,-march=sb1,-march=r5000) \
			-Wa,--trap
cflags-$(CONFIG_CPU_R8000)	+= -march=r8000 -Wa,--trap
cflags-$(CONFIG_CPU_R10000)	+= $(call cc-option,-march=r10000,-march=r8000) \
			-Wa,--trap
cflags-$(CONFIG_CPU_CAVIUM_OCTEON) += $(call cc-option,-march=octeon) -Wa,--trap
ifeq (,$(findstring march=octeon, $(cflags-$(CONFIG_CPU_CAVIUM_OCTEON))))
cflags-$(CONFIG_CPU_CAVIUM_OCTEON) += -Wa,-march=octeon
endif

cflags-$(CONFIG_CPU_R4000_WORKAROUNDS)	+= $(call cc-option,-mfix-r4000,)
cflags-$(CONFIG_CPU_R4400_WORKAROUNDS)	+= $(call cc-option,-mfix-r4400,)
cflags-$(CONFIG_CPU_DADDI_WORKAROUNDS)	+= $(call cc-option,-mno-daddi,)

ifdef CONFIG_CPU_SB1
ifdef CONFIG_SB1_PASS_1_WORKAROUNDS
MODFLAGS	+= -msb1-pass1-workarounds
endif
endif

#
# Firmware support
#
libs-$(CONFIG_ARC)		+= arch/mips/fw/arc/
libs-$(CONFIG_CFE)		+= arch/mips/fw/cfe/
libs-$(CONFIG_SNIPROM)		+= arch/mips/fw/sni/
libs-y				+= arch/mips/fw/lib/

#
# Board-dependent options and extra files
#

#
# Acer PICA 61, Mips Magnum 4000 and Olivetti M700.
#
# Common Alchemy Au1x00 stuff
#

#
# Cobalt Server

#
# DECstation family
#
core-$(CONFIG_MACH_DECSTATION)	+= arch/mips/dec/
cflags-$(CONFIG_MACH_DECSTATION)+= -I$(srctree)/arch/mips/include/asm/mach-dec
libs-$(CONFIG_MACH_DECSTATION)	+= arch/mips/dec/prom/
load-$(CONFIG_MACH_DECSTATION)	+= 0xffffffff80040000

#
# Wind River PPMC Board (4KC + GT64120)
#
#
# lemote fulong mini-PC board
#
#
# MIPS Malta board
#
core-$(CONFIG_MIPS_MALTA)	+= arch/mips/mti-malta/
cflags-$(CONFIG_MIPS_MALTA)	+= -I$(srctree)/arch/mips/include/asm/mach-malta
load-$(CONFIG_MIPS_MALTA)	+= 0xffffffff80100000
all-$(CONFIG_MIPS_MALTA)	:= vmlinux.bin

core-$(CONFIG_RTL_8196B)         += arch/mips/rtl8196b/
cflags-$(CONFIG_RTL_8196B)       += -I$(srctree)/arch/mips/include/asm/mach-realtek/rtl8196b -fno-builtin -D__KERNEL__
cflags-$(CONFIG_RTL_8196B)       += -march=5181
load-$(CONFIG_RTL_8196B)         += 0xffffffff80000000

core-$(CONFIG_RTL_8196C)         += arch/mips/rtl8196c/
cflags-$(CONFIG_RTL_8196C)       += -I$(srctree)/arch/mips/include/asm/mach-realtek/rtl8196c -fno-builtin -D__KERNEL__
cflags-$(CONFIG_RTL_8196C)       += -march=4181
load-$(CONFIG_RTL_8196C)         += 0xffffffff80000000
#
# MIPS SIM
#
core-$(CONFIG_MIPS_SIM)		+= arch/mips/mipssim/
cflags-$(CONFIG_MIPS_SIM)	+= -I$(srctree)/arch/mips/include/asm/mach-mipssim
load-$(CONFIG_MIPS_SIM)		+= 0x80100000

#
# PMC-Sierra MSP SOCs
#
core-$(CONFIG_PMC_MSP)		+= arch/mips/pmc-sierra/msp71xx/
cflags-$(CONFIG_PMC_MSP)	+= -I$(srctree)/arch/mips/include/asm/pmc-sierra/msp71xx \
					-mno-branch-likely
load-$(CONFIG_PMC_MSP)		+= 0xffffffff80100000

#
# PMC-Sierra Yosemite
#
core-$(CONFIG_PMC_YOSEMITE)	+= arch/mips/pmc-sierra/yosemite/
cflags-$(CONFIG_PMC_YOSEMITE)	+= -I$(srctree)/arch/mips/include/asm/mach-yosemite
load-$(CONFIG_PMC_YOSEMITE)	+= 0xffffffff80100000

#
# Basler eXcite
#
#
# LASAT platforms
#

#
# Common VR41xx
#

#
# SGI IP22 (Indy/Indigo2)
#
# Set the load address to >= 0xffffffff88069000 if you want to leave space for
# symmon, 0xffffffff80002000 for production kernels.  Note that the value must
# be aligned to a multiple of the kernel stack size or the handling of the
# current variable will break so for 64-bit kernels we have to raise the start
# address by 8kb.
#

#
# SGI-IP27 (Origin200/2000)
#
# Set the load address to >= 0xc000000000300000 if you want to leave space for
# symmon, 0xc00000000001c000 for production kernels.  Note that the value must
# be 16kb aligned or the handling of the current variable will break.
#

#
# SGI IP28 (Indigo2 R10k)
#
# Set the load address to >= 0xa800000020080000 if you want to leave space for
# symmon, 0xa800000020004000 for production kernels ?  Note that the value must
# be 16kb aligned or the handling of the current variable will break.
# Simplified: what IP22 does at 128MB+ in ksegN, IP28 does at 512MB+ in xkphys
#
ifdef CONFIG_SGI_IP28
  ifeq ($(call cc-option-yn,-mr10k-cache-barrier=store), n)
      $(error gcc doesn't support needed option -mr10k-cache-barrier=store)
  endif
endif

#
# SGI-IP32 (O2)
#
# Set the load address to >= 80069000 if you want to leave space for symmon,
# 0xffffffff80004000 for production kernels.  Note that the value must be aligned to
# a multiple of the kernel stack size or the handling of the current variable
# will break.
#

#
# Sibyte SB1250/BCM1480 SOC
#
# This is a LIB so that it links at the end, and initcalls are later
# the sequence; but it is built as an object so that modules don't get
# removed (as happens, even if they have __initcall/module_init)
#

#
# Sibyte BCM91120x (Carmel) board
# Sibyte BCM91120C (CRhine) board
# Sibyte BCM91125C (CRhone) board
# Sibyte BCM91125E (Rhone) board
# Sibyte SWARM board
# Sibyte BCM91x80 (BigSur) board
#

#
# Broadcom BCM47XX boards
#
#
# SNI RM
#

#
# Cavium Octeon
#

cflags-y			+= -I$(srctree)/arch/mips/include/asm/mach-generic
drivers-$(CONFIG_PCI)		+= arch/mips/pci/

ifdef CONFIG_32BIT
ifdef CONFIG_CPU_LITTLE_ENDIAN
JIFFIES			= jiffies_64
else
JIFFIES			= jiffies_64 + 4
endif
else
JIFFIES			= jiffies_64
endif

#
# Automatically detect the build format. By default we choose
# the elf format according to the load address.
# We can always force a build with a 64-bits symbol format by
# passing 'KBUILD_SYM32=no' option to the make's command line.
#
ifdef CONFIG_64BIT
  ifndef KBUILD_SYM32
    ifeq ($(shell expr $(load-y) \< 0xffffffff80000000), 0)
      KBUILD_SYM32 = y
    endif
  endif

  ifeq ($(KBUILD_SYM32)$(call cc-option-yn,-msym32), yy)
    cflags-y += -msym32 -DKBUILD_64BIT_SYM32
  else
    ifeq ($(CONFIG_CPU_DADDI_WORKAROUNDS), y)
      $(error CONFIG_CPU_DADDI_WORKAROUNDS unsupported without -msym32)
    endif
  endif
endif

KBUILD_AFLAGS	+= $(cflags-y)
KBUILD_CFLAGS	+= $(cflags-y) \
			-D"VMLINUX_LOAD_ADDRESS=$(load-y)"

LDFLAGS			+= -m $(ld-emul)

ifdef CONFIG_MIPS
CHECKFLAGS += $(shell $(CC) $(KBUILD_CFLAGS) -dM -E -xc /dev/null | \
	egrep -vw '__GNUC_(|MINOR_|PATCHLEVEL_)_' | \
	sed -e 's/^\#define /-D/' -e "s/ /='/" -e "s/$$/'/")
ifdef CONFIG_64BIT
CHECKFLAGS		+= -m64
endif
endif

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
	-D"DATAOFFSET=$(if $(dataoffset-y),$(dataoffset-y),0)"

head-y := arch/mips/kernel/head.o arch/mips/kernel/init_task.o

libs-y			+= arch/mips/lib/

core-y			+= arch/mips/kernel/ arch/mips/mm/ arch/mips/math-emu/

drivers-$(CONFIG_OPROFILE)	+= arch/mips/oprofile/

ifdef CONFIG_LASAT
rom.bin rom.sw: vmlinux
	$(Q)$(MAKE) $(build)=arch/mips/lasat/image $@
endif

#
# Some machines like the Indy need 32-bit ELF binaries for booting purposes.
# Other need ECOFF, so we build a 32-bit ELF binary for them which we then
# convert to ECOFF using elf2ecoff.
#
vmlinux.32: vmlinux
	$(OBJCOPY) -O $(32bit-bfd) $(OBJCOPYFLAGS) $< $@

#
# The 64-bit ELF tools are pretty broken so at this time we generate 64-bit
# ELF files from 32-bit files by conversion.
#
vmlinux.64: vmlinux
	$(OBJCOPY) -O $(64bit-bfd) $(OBJCOPYFLAGS) $< $@

makeboot =$(Q)$(MAKE) $(build)=arch/mips/boot VMLINUX=$(vmlinux-32) $(1)

all:	$(all-y)

vmlinux.bin: $(vmlinux-32)
	+@$(call makeboot,$@)

vmlinux.ecoff: $(vmlinux-32)
	+@$(call makeboot,$@)

vmlinux.srec: $(vmlinux-32)
	+@$(call makeboot,$@)

CLEAN_FILES += vmlinux.ecoff \
	       vmlinux.srec

archprepare:
ifdef CONFIG_MIPS32_N32
	@echo '  Checking missing-syscalls for N32'
	$(Q)$(MAKE) $(build)=. missing-syscalls EXTRA_CFLAGS="-mabi=n32"
endif
ifdef CONFIG_MIPS32_O32
	@echo '  Checking missing-syscalls for O32'
	$(Q)$(MAKE) $(build)=. missing-syscalls EXTRA_CFLAGS="-mabi=32"
endif

install:
	$(Q)install -D -m 755 vmlinux $(INSTALL_PATH)/vmlinux-$(KERNELRELEASE)
	$(Q)install -D -m 644 .config $(INSTALL_PATH)/config-$(KERNELRELEASE)
	$(Q)install -D -m 644 System.map $(INSTALL_PATH)/System.map-$(KERNELRELEASE)

archclean:
	@$(MAKE) $(clean)=arch/mips/boot
	#@$(MAKE) $(clean)=arch/mips/lasat

define archhelp
	echo '  install              - install kernel into $(INSTALL_PATH)'
	echo '  vmlinux.ecoff        - ECOFF boot image'
	echo '  vmlinux.bin          - Raw binary boot image'
	echo '  vmlinux.srec         - SREC boot image'
	echo
	echo '  These will be default as apropriate for a configured platform.'
endef

CLEAN_FILES += vmlinux.32 \
	       vmlinux.64 \
	       vmlinux.ecoff
