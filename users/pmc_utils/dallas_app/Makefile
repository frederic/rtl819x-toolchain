#******************************************************************************
# Copyright (C) 2006 PMC-Sierra Inc.  All Rights Reserved.
#------------------------------------------------------------------------------
# This software embodies materials and concepts which are proprietary and
# confidential to PMC-Sierra, Inc.  PMC-Sierra distributes this software to
# its customers pursuant to the terms and conditions of the Software License
# Agreement contained in the text file software.lic that is distributed along
# with the software.  This software can only be utilized if all terms and
# conditions of the Software License Agreement are accepted.  If there are
# any questions, concerns, or if the Software License Agreement text file
# software.lic is missing, please contact PMC-Sierra for assistance.
#------------------------------------------------------------------------------
# $RCSfile: Makefile,v $
#
# $Date: 2008-08-13 02:35:43 $
#
# $Revision: 1.3 $
#------------------------------------------------------------------------------
# Makefile for the Dallas Controller demo program
#------------------------------------------------------------------------------
# Name of the output executable
APP = dallas_ctrl

CURDIR = $(shell pwd)

ifneq ($(strip $(TOOLPREFIX)),)
CROSS:=$(TOOLPREFIX)
endif

CC              =$(CROSS)gcc
LD              = $(CROSS)ld
STRIP           =$(CROSS)strip
STRIPFLAGS      = --strip-all --remove-section=.note --remove-section=.comment

export TOPDIR = $(CURDIR)
 
# List of source files for this module that are common

SRCS-y += dallas_app_main.c 
SRCS-y += dallas_app_util.c 
SRCS-y += dallas_app_cmd.c 

OBJS    = $(SRCS-y:.c=.o)


################ do not modify below this line ###############
app: rmapp $(OBJS)
	@echo "   CC        $(THISDIR)/$(APP)"
	$(CC) $(CFLAGS) $(APPFLAGS) $(ALGLIBS) $(OBJS) -o $(APP) 


rmapp:
	@echo "   RMAPP     $(THISDIR)"
	$(QUIET)rm -f $(APP)

include $(TOPDIR)/user_rules.make

