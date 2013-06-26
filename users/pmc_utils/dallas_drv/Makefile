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
# $RCSfile: Makefile,v $
#
# $Date: 2009-01-14 06:50:27 $
#
# $Revision: 1.5 $
#-------------------------------------------------------------------------------
# Makefile for the Dallas Controller kernel module
#-------------------------------------------------------------------------------

# Generic Makefile for building a LKM for Linux 2.6

ifneq ($(strip $(TOOLPREFIX)),)
CROSS:=$(TOOLPREFIX)
endif

ifeq ($(strip $(DALLAS_WORK_MODE)), non-slave_mode)
DALLAS_IMAGE=dallas.bin
else
DALLAS_IMAGE=isr.bin
endif

CC              =$(CROSS)gcc
STRIP           =$(CROSS)strip
STRIPFLAGS      = --strip-all --remove-section=.note --remove-section=.comment

#ARCH         = mips
#CROSS        = /pkg/xgcc/xgcc_mips-sdelinux-v6.03.01-1.dist/xgcc/bin/mips-linux-

EXTRA_CFLAGS += -I. -Wall -Wno-strict-prototypes
#EXTRA_CFLAGS += -Wall -pipe -Os -mips32r2
EXTRA_CFLAGS += -Wall -pipe -Os
EXTRA_CFLAGS += -DCPU=MIPS32 -DTOOL_FAMILY=gnu -DTOOL=m34k0
EXTRA_CFLAGS += -D__MIPSEB__ -D_MIPS_SZLONG=32 -DEXPORT_SYMTAB -DOSAL
EXTRA_CFLAGS += -D__VOSMP_TASKLIST_LOCK_REPLACED_BY_RCULOCK__ -D__VOSMP_SCHED_SETAFFINITY_ENABLE__ -DVAPP_MAIN
EXTRA_CFLAGS += -DVAPP_MGMT_PRINT_TO_CONSOLE -DOSAL_LKM
EXTRA_CFLAGS += -DPMC_LINUX -DPMC_VERSION  -fno-strict-aliasing -fno-common -fomit-frame-pointer -fno-pic -G 0 -nostdinc -mno-abicalls -mlong-calls
EXTRA_CFLAGS += -I$(PWD)/inc
EXTRA_CFLAGS += -I$(PWD)/../inc


all: modules

obj-m        := dallas_ctrl.o
#socket_test-objs  := denver2.o
dallas_ctrl-objs  := dallas_controller.o dallas_util.o pbrc_protocol.o ksocket.o 

EXTRA_CFLAGS += 

modules:
#	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) -C $(KERNELPATH) M=`pwd` modules
#	$(MAKE) CROSS_COMPILE=$(CROSS) -C $(KERNELPATH) M=`pwd` modules
	make -C $(KERNELPATH) M=`pwd` modules
	@rm -rf ../dallas
	@mkdir ../dallas
	@cp *.ko ../dallas
	@cp ../dallas_lib/$(DALLAS_IMAGE) ../dallas

clean:
	@rm -rf *.o *.ko Module.symvers *.mod.c .*.cmd .tmp_versions src/*.o src/.*.cmd



