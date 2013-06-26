# Makefile for secure rtp 
#
# David A. McGrew
# Cisco Systems, Inc.

# targets:
#
# runtest       runs test applications 
# test		builds test applications
# libcrypt.a	static library implementing crypto engine
# libsrtp.a	static library implementing srtp
# clean		removes objects, libs, and executables
# distribution  cleans and builds a .tgz
# tags          builds etags file from all .c and .h files


obj-y += srtp/
obj-y += crypto/

EXTRA_CFLAGS += -DGNU
EXTRA_CFLAGS += -I$(DIR_VOIP)/srtp/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/srtp/crypto/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/voip_dsp/cp3

ifeq ($(CONFIG_RTK_VOIP),m)
EXTRA_CFLAGS += -mlong-calls
EXTRA_CFLAGS += -DMODULE
endif

# rock: add module name for PRINT
EXTRA_CFLAGS += -DMODULE_NAME=\"SRTP\"

include $(DIR_VOIP)/rules.mak

