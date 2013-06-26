# Makefile for libcryptomodule.a
#
# David A. McGrew
# Cisco Systems, Inc.


obj-y += cipher/
obj-y += hash/
obj-y += replay/
obj-y += math/
obj-y += rng/
obj-y += kernel/

EXTRA_CFLAGS += -DGNU
EXTRA_CFLAGS += -I$(DIR_VOIP)/srtp/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/include
EXTRA_CFLAGS += -I$(DIR_VOIP)/srtp/crypto/include

ifeq ($(CONFIG_RTK_VOIP),m)
EXTRA_CFLAGS += -mlong-calls
EXTRA_CFLAGS += -DMODULE
endif

include $(DIR_VOIP)/rules.mak
