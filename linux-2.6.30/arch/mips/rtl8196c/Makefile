#
# Makefile for the RTL8652 specific parts of the kernel
#
DIR_RTLASIC = $(DIR_LINUX)/drivers/net/rtl819x/

obj-y	 	:= setup.o printf.o int.o mem.o 
obj-$(CONFIG_PCI) += pci.o
EXTRA_AFLAGS := $(CFLAGS)
EXTRA_CFLAGS += -I$(DIR_RTLASIC)
