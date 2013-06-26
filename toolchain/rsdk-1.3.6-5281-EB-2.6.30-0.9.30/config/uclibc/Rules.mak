# Rules.make for uClibc
#
# Copyright (C) 2000-2008 Erik Andersen <andersen@uclibc.org>
#
# Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
#

# check for proper make version
ifneq ($(findstring x3.7,x$(MAKE_VERSION)),)
$(error Your make is too old $(MAKE_VERSION). Go get at least 3.80)
endif

#-----------------------------------------------------------
# This file contains rules which are shared between multiple
# Makefiles.  All normal configuration options live in the
# file named ".config".  Don't mess with this file unless
# you know what you are doing.


#-----------------------------------------------------------
# If you are running a cross compiler, you will want to set
# 'CROSS' to something more interesting ...  Target
# architecture is determined by asking the CC compiler what
# arch it compiles things for, so unless your compiler is
# broken, you should not need to specify TARGET_ARCH.
#
# Most people will set this stuff on the command line, i.e.
#        make CROSS=arm-linux-
# will build uClibc for 'arm'.

ifndef CROSS
CROSS=rsdk-linux-
endif
CC         = $(CROSS)gcc
AR         = $(CROSS)ar
LD         = $(CROSS)ld
NM         = $(CROSS)nm
STRIPTOOL  = $(CROSS)strip

INSTALL    = install
LN         = ln
RM         = rm -f
TAR        = tar
SED        = sed
AWK        = awk

STRIP_FLAGS ?= -x -R .note -R .comment

UNIFDEF := $(top_builddir)extra/scripts/unifdef -UUCLIBC_INTERNAL

# Select the compiler needed to build binaries for your development system
HOSTCC     = gcc
BUILD_CFLAGS = -O2 -Wall
export ARCH := rlx

#---------------------------------------------------------
# Nothing beyond this point should ever be touched by mere
# mortals.  Unless you hang out with the gods, you should
# probably leave all this stuff alone.

# Pull in the user's uClibc configuration
ifeq ($(filter $(noconfig_targets),$(MAKECMDGOALS)),)
-include $(top_builddir).config
endif

# Make certain these contain a final "/", but no "//"s.
TARGET_ARCH := rlx
RUNTIME_PREFIX:=$(strip $(subst //,/, $(subst ,/, $(subst ",, $(strip $(RUNTIME_PREFIX))))))
DEVEL_PREFIX:=$(strip $(subst //,/, $(subst ,/, $(subst ",, $(strip $(DEVEL_PREFIX))))))
KERNEL_HEADERS:=$(strip $(subst //,/, $(subst ,/, $(subst ",, $(strip $(KERNEL_HEADERS))))))
export RUNTIME_PREFIX DEVEL_PREFIX KERNEL_HEADERS


# Now config hard core
MAJOR_VERSION := 0
MINOR_VERSION := 9
SUBLEVEL      := 30
EXTRAVERSION  :=
VERSION       := $(MAJOR_VERSION).$(MINOR_VERSION).$(SUBLEVEL)
ifneq ($(EXTRAVERSION),)
VERSION       := $(VERSION)$(EXTRAVERSION)
endif
# Ensure consistent sort order, 'gcc -print-search-dirs' behavior, etc.
LC_ALL := C
export MAJOR_VERSION MINOR_VERSION SUBLEVEL VERSION LC_ALL

LIBC := libc
SHARED_MAJORNAME := $(LIBC).so.$(MAJOR_VERSION)
UCLIBC_LDSO_NAME := ld-uClibc
ARCH_NATIVE_BIT := 32
UCLIBC_LDSO := $(UCLIBC_LDSO_NAME).so.$(MAJOR_VERSION)
NONSHARED_LIBNAME := uclibc_nonshared.a
libc := $(top_builddir)lib/$(SHARED_MAJORNAME)
libc.depend := $(top_builddir)lib/$(SHARED_MAJORNAME:.$(MAJOR_VERSION)=)
interp := $(top_builddir)lib/interp.os
ldso := $(top_builddir)lib/$(UCLIBC_LDSO)
headers_dep := $(top_builddir)include/bits/sysnum.h
sub_headers := $(headers_dep)

#LIBS :=$(interp) -L$(top_builddir)lib -lc
LIBS := $(interp) -L$(top_builddir)lib $(libc:.$(MAJOR_VERSION)=)

# Make sure DESTDIR and PREFIX can be used to install
# PREFIX is a uClibcism while DESTDIR is a common GNUism
ifndef PREFIX
PREFIX = $(DESTDIR)
endif

ifneq ($(HAVE_SHARED),y)
libc :=
interp :=
ldso :=
endif

comma:=,
space:= #

ifndef CROSS
CROSS=$(strip $(subst ",, $(CROSS_COMPILER_PREFIX)))
endif

# A nifty macro to make testing gcc features easier
check_gcc=$(shell \
	if $(CC) $(1) -S -o /dev/null -xc /dev/null > /dev/null 2>&1; \
	then echo "$(1)"; else echo "$(2)"; fi)
check_as=$(shell \
	if $(CC) -Wa,$(1) -Wa,-Z -c -o /dev/null -xassembler /dev/null > /dev/null 2>&1; \
	then echo "-Wa,$(1)"; fi)
check_ld=$(shell \
	if $(LD) $(1) -o /dev/null -b binary /dev/null > /dev/null 2>&1; \
	then echo "$(1)"; fi)

ARFLAGS:=cr


GCC_MAJOR_VER?=$(shell $(CC) -dumpversion | cut -d . -f 1)
GCC_MINOR_VER?=$(shell $(CC) -dumpversion | cut -d . -f 2)

# Flags in OPTIMIZATION are used only for non-debug builds
OPTIMIZATION:=

OPTIMIZATION+=$(call check_gcc,-Os,-O2)
OPTIMIZATION+=$(call check_gcc,-funit-at-a-time,)

ifeq ($(GCC_MAJOR_VER),4)
# shrinks code, results are from 4.0.2
# 0.36%
OPTIMIZATION+=$(call check_gcc,-fno-tree-loop-optimize,)
# 0.34%
OPTIMIZATION+=$(call check_gcc,-fno-tree-dominator-opts,)
# 0.1%
OPTIMIZATION+=$(call check_gcc,-fno-strength-reduce,)
endif

CPU_CFLAGS-$(DOPIC) += -fPIC

CPU_LDFLAGS-$(ARCH_LITTLE_ENDIAN)+=-Wl,-EL -Wl,-melf32ltsmip
CPU_LDFLAGS-$(ARCH_BIG_ENDIAN)+=-Wl,-EB -Wl,-melf32btsmip
CPU_CFLAGS-$(CONFIG_MIPS_ISA_1)+=-mabi=32

# Check for --as-needed support in linker
ifndef LD_FLAG_ASNEEDED
_LD_FLAG_ASNEEDED:=$(shell $(LD) --help 2>/dev/null | grep -- --as-needed)
ifneq ($(_LD_FLAG_ASNEEDED),)
export LD_FLAG_ASNEEDED:=--as-needed
endif
endif
ifndef LD_FLAG_NO_ASNEEDED
ifdef LD_FLAG_ASNEEDED
export LD_FLAG_NO_ASNEEDED:=--no-as-needed
endif
endif
ifndef CC_FLAG_ASNEEDED
ifdef LD_FLAG_ASNEEDED
export CC_FLAG_ASNEEDED:=-Wl,$(LD_FLAG_ASNEEDED)
endif
endif
ifndef CC_FLAG_NO_ASNEEDED
ifdef LD_FLAG_NO_ASNEEDED
export CC_FLAG_NO_ASNEEDED:=-Wl,$(LD_FLAG_NO_ASNEEDED)
endif
endif
link.asneeded = $(if $(and $(CC_FLAG_ASNEEDED),$(CC_FLAG_NO_ASNEEDED)),$(CC_FLAG_ASNEEDED) $(1) $(CC_FLAG_NO_ASNEEDED))

# Check for AS_NEEDED support in linker script (binutils>=2.16.1 has it)
ifndef ASNEEDED
export ASNEEDED:=$(shell $(LD) --help 2>/dev/null | grep -q -- --as-needed && echo "AS_NEEDED ( $(UCLIBC_LDSO) )" || echo "$(UCLIBC_LDSO)")
endif

# Add a bunch of extra pedantic annoyingly strict checks
XWARNINGS=$(subst ",, $(strip $(WARNINGS))) -Wstrict-prototypes -fno-strict-aliasing
ifeq ($(EXTRA_WARNINGS),y)
XWARNINGS+=-Wnested-externs -Wshadow -Wmissing-noreturn -Wmissing-format-attribute -Wformat=2
XWARNINGS+=-Wmissing-prototypes -Wmissing-declarations
XWARNINGS+=-Wnonnull -Wundef
# works only w/ gcc-3.4 and up, can't be checked for gcc-3.x w/ check_gcc()
#XWARNINGS+=-Wdeclaration-after-statement
endif
XARCH_CFLAGS=$(subst ",, $(strip $(ARCH_CFLAGS)))
CPU_CFLAGS=$(subst ",, $(strip $(CPU_CFLAGS-y)))

SSP_DISABLE_FLAGS ?= $(call check_gcc,-fno-stack-protector,)
ifeq ($(UCLIBC_BUILD_SSP),y)
SSP_CFLAGS := $(call check_gcc,-fno-stack-protector-all,)
SSP_CFLAGS += $(call check_gcc,-fstack-protector,)
SSP_ALL_CFLAGS ?= $(call check_gcc,-fstack-protector-all,)
else
SSP_CFLAGS := $(SSP_DISABLE_FLAGS)
endif

NOSTDLIB_CFLAGS:=$(call check_gcc,-nostdlib,)
# Some nice CFLAGS to work with
CFLAGS := -include $(top_builddir)include/libc-symbols.h \
	$(XWARNINGS) $(CPU_CFLAGS) $(SSP_CFLAGS) \
	-fno-builtin -nostdinc -I$(top_builddir)include -I. \
	-I$(top_srcdir)libc/sysdeps/linux/$(TARGET_ARCH)

# Make sure that we can be built with non-C99 compilers, too.
# Use __\1__ instead.
CFLAGS += $(call check_gcc,-fno-asm,)
ifneq ($(strip $(UCLIBC_EXTRA_CFLAGS)),"")
CFLAGS += $(subst ",, $(UCLIBC_EXTRA_CFLAGS))
endif

LDADD_LIBFLOAT=
ifeq ($(UCLIBC_HAS_SOFT_FLOAT),y)
# If -msoft-float isn't supported, we want an error anyway.
# Hmm... might need to revisit this for arm since it has 2 different
# soft float encodings.
CFLAGS += -msoft-float
endif

# Please let us see private headers' parts
CFLAGS += -DUCLIBC_INTERNAL

# We need this to be checked within libc-symbols.h
ifneq ($(HAVE_SHARED),y)
CFLAGS += -DSTATIC
endif

CFLAGS += $(call check_gcc,-std=gnu99,)

LDFLAGS_NOSTRIP:=$(CPU_LDFLAGS-y) -Wl,-shared \
	-Wl,--warn-common -Wl,--warn-once -Wl,-z,combreloc
# binutils-2.16.1 warns about ignored sections, 2.16.91.0.3 and newer are ok
#LDFLAGS_NOSTRIP+=$(call check_ld,--gc-sections)

ifeq ($(UCLIBC_BUILD_RELRO),y)
LDFLAGS_NOSTRIP+=-Wl,-z,relro
endif

ifeq ($(UCLIBC_BUILD_NOW),y)
LDFLAGS_NOSTRIP+=-Wl,-z,now
endif

ifeq ($(LDSO_GNU_HASH_SUPPORT),y)
# Be sure that binutils support it
LDFLAGS_GNUHASH:=$(call check_ld,--hash-style=gnu)
ifeq ($(LDFLAGS_GNUHASH),)
ifneq ($(filter-out install_headers,$(MAKECMDGOALS)),)
$(error Your binutils don't support --hash-style option, while you want to use it)
endif
else
LDFLAGS_NOSTRIP += -Wl,$(LDFLAGS_GNUHASH)
endif
endif

LDFLAGS:=$(LDFLAGS_NOSTRIP) -Wl,-z,defs
ifeq ($(DODEBUG),y)
CFLAGS += -O0 -g3
else
CFLAGS += $(OPTIMIZATION) $(XARCH_CFLAGS)
endif
ifeq ($(DOSTRIP),y)
LDFLAGS += -Wl,-s
else
STRIPTOOL := true -Stripping_disabled
endif

ifeq ($(DOMULTI),y)
# we try to compile all sources at once into an object (IMA), but
# gcc-3.3.x does not support it
# gcc-3.4.x supports it, but does not need and support --combine. though fails on many sources
# gcc-4.0.x supports it, supports the --combine flag, but does not need it
# gcc-4.1(200506xx) supports it, but needs the --combine flag, else libs are useless
ifeq ($(GCC_MAJOR_VER),3)
DOMULTI:=n
else
CFLAGS+=$(call check_gcc,--combine,)
endif
else
DOMULTI:=n
endif

ifneq ($(strip $(UCLIBC_EXTRA_LDFLAGS)),"")
LDFLAGS += $(subst ",, $(UCLIBC_EXTRA_LDFLAGS))
endif

ifeq ($(UCLIBC_HAS_THREADS),y)
ifeq ($(UCLIBC_HAS_THREADS_NATIVE),y)
	PTNAME := nptl
else
ifeq ($(LINUXTHREADS_OLD),y)
	PTNAME := linuxthreads.old
else
	PTNAME := linuxthreads
endif
endif
PTDIR := $(top_builddir)libpthread/$(PTNAME)
# set up system dependencies include dirs (NOTE: order matters!)
ifeq ($(UCLIBC_HAS_THREADS_NATIVE),y)
PTINC:=	-I$(PTDIR)						\
	-I$(PTDIR)/sysdeps/unix/sysv/linux/$(TARGET_ARCH)	\
	-I$(PTDIR)/sysdeps/$(TARGET_ARCH)			\
	-I$(PTDIR)/sysdeps/unix/sysv/linux			\
	-I$(PTDIR)/sysdeps/pthread				\
	-I$(PTDIR)/sysdeps/pthread/bits				\
	-I$(PTDIR)/sysdeps/generic				\
	-I$(top_srcdir)ldso/ldso/$(TARGET_ARCH)			\
	-I$(top_srcdir)ldso/include
#
# Test for TLS if NPTL support was selected.
#
GCC_HAS_TLS=$(shell \
	echo "extern __thread int foo;" | $(CC) -o /dev/null -S -xc - 2>&1)
ifneq ($(GCC_HAS_TLS),)
gcc_tls_test_fail:
	@echo "####";
	@echo "#### Your compiler does not support TLS and you are trying to build uClibc";
	@echo "#### with NPTL support. Upgrade your binutils and gcc to versions which";
	@echo "#### support TLS for your architecture. Do not contact uClibc maintainers";
	@echo "#### about this problem.";
	@echo "####";
	@echo "#### Exiting...";
	@echo "####";
	@exit 1;
endif
else
PTINC := \
	-I$(PTDIR)/sysdeps/unix/sysv/linux/$(TARGET_ARCH) \
	-I$(PTDIR)/sysdeps/$(TARGET_ARCH) \
	-I$(PTDIR)/sysdeps/unix/sysv/linux \
	-I$(PTDIR)/sysdeps/pthread \
	-I$(PTDIR) \
	-I$(top_builddir)libpthread
endif
CFLAGS+=$(PTINC)
else
	PTNAME :=
	PTINC  :=
endif
CFLAGS += -I$(KERNEL_HEADERS)

#CFLAGS += -iwithprefix include-fixed -iwithprefix include
CC_IPREFIX:=$(shell $(CC) --print-file-name=include)
CFLAGS += -I$(dir $(CC_IPREFIX))/include-fixed -I$(CC_IPREFIX)

ifneq ($(DOASSERTS),y)
CFLAGS+=-DNDEBUG
endif

ifeq ($(SYMBOL_PREFIX),_)
CFLAGS+=-D__UCLIBC_UNDERSCORES__
endif

# Keep the check_as from being needlessly executed
ifndef ASFLAGS_NOEXEC
ifeq ($(UCLIBC_BUILD_NOEXECSTACK),y)
export ASFLAGS_NOEXEC := $(call check_as,--noexecstack)
else
export ASFLAGS_NOEXEC :=
endif
endif
ASFLAGS = $(ASFLAGS_NOEXEC)

LIBGCC_CFLAGS ?= $(CFLAGS) $(CPU_CFLAGS-y)
LIBGCC:=$(shell $(CC) $(LIBGCC_CFLAGS) -print-libgcc-file-name)
LIBGCC_DIR:=$(dir $(LIBGCC))

# moved from libpthread/linuxthreads
ifeq ($(UCLIBC_CTOR_DTOR),y)
SHARED_START_FILES:=$(top_builddir)lib/crti.o $(LIBGCC_DIR)crtbeginS.o
SHARED_END_FILES:=$(LIBGCC_DIR)crtendS.o $(top_builddir)lib/crtn.o
endif

LOCAL_INSTALL_PATH := install_dir
