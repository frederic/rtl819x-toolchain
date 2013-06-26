#*******************************************************************************
# Copyright (C) 2006 PMC-Sierra Inc.  All Rights Reserved.
#-------------------------------------------------------------------------------
# This software embodies materials and concepts which are proprietary and
# confidential to PMC-Sierra, Inc.  PMC-Sierra distributes this software to
# its customers pursuant to the terms and conditions of the Software License
# Agreement contained in the text file software.lic that is distributed along
# with the software.  This software can only be utilized if all terms and
# conditions of the Software License Agreement are accepted.  If there are
# any questions, concerns, or if the Software License Agreement text file
# software.lic is missing, please contact PMC-Sierra for assistance.
#-------------------------------------------------------------------------------
# $RCSfile: user_rules.make,v $
#
# $Date: 2008-09-10 05:49:03 $
#
# $Revision: 1.4 $
#-------------------------------------------------------------------------------
# Makefile rules for use with making userland applications and libraries.
#-------------------------------------------------------------------------------

# Check for verbose mode
ifneq ($(V),1)
MAKEFLAGS += --no-print-directory
QUIET=@
else
QUIET=
endif

# Flags that user applications need to be built
CFLAGS += -Wall                \
         -O2                  \
         -mtune=5281           \
         -mno-split-addresses \
         -DEXPORT_SYMTAB      \
         -DCPU=MIPS32         \
         -DTOOL_FAMILY=gnu    \
         -DTOOL=m34k0         \
         -D__MIPSEB__         \
         -D_MIPS_SZLONG=32    \
         -DOSAL_DEBUG         \
         -fno-strict-aliasing

#         -mtune=4kc           \
#         -mips32r2            \

LDFLAGS = -r 
APPFLAGS += -lpthread
APPFLAGS += -lz -L../../zlib-1.2.5

# rules for making objects from source
OBJS    = $(SRCS-y:.c=.o)
ASMOBJS = $(ASMSRCS-y:.S=.o)

# directories that userland applications and/or libraries need
INCDIRS += -I./ -I./../inc -I../../zlib-1.2.5
INCDIRS += -I$(TOPDIR)/inc

all: $(OBJS) $(ASMOBJS)

TDIR_STR = $(subst /, ,$(shell pwd))
THISDIR = $(word $(words $(TDIR_STR)), $(TDIR_STR))

.PHONY: all tests deps depend testsclean depsclean clean distclean

tests:
	@$(foreach dir, $(TESTDIRS-y), $(MAKE) -C $(dir);)

deps:
	@$(foreach dir, $(DEPDIRS-y), $(MAKE) -C $(dir);)
	
depend:
	@echo "   DEPEND    $(THISDIR)"
	$(QUIET)$(MKDEP) $(CFLAGS) $(CSW) $(INCDIRS) $(addprefix src/, $(SRCS))

testsclean:
	@$(foreach dir, $(TESTDIRS-y), echo "CLEAN     $(dir)"; $(MAKE) -C $(dir) clean;)

depsclean:
	@$(foreach dir, $(DEPDIRS-y), echo "CLEAN      $(dir)"; $(MAKE) -C $(dir) clean;)
	
clean: testsclean depsclean
	@echo "   CLEAN     $(THISDIR)"
	$(QUIET)rm -f *.o $(APP)

distclean: clean
	@echo "   DISTCLEAN $(THISDIR)"
	$(QUIET)rm -f .depend

ifneq ($(wildcard .depend), )
include .depend
endif

%.o : $(if $(@D), , %.c)
	@echo "   CC        $(THISDIR)/$<"
	$(QUIET)$(CC) $(CFLAGS) $(INCDIRS) $(CSW) -o $(@F) -c $<

%.o : $(addprefix src/, %.c)
	@echo "   CC        $(THISDIR)/$<"
	$(QUIET)$(CC) $(CFLAGS) $(INCDIRS) $(CSW) -o $(@F) -c $<

%.o : $(addprefix src/, %.S)
	@echo "   CC        $(THISDIR)/$<"
	$(QUIET)$(CC) $(CFLAGS) $(INCDIRS) $(CSW) -o $@ -c $<
