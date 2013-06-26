CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
RANLIB = $(CROSS_COMPILE)ranlib
STRIP = $(CROSS_COMPILE)strip
SSTRIP = $(CROSS_COMPILE)sstrip

PROG        = lsusb
VERS        = 0.86
RM          = /bin/rm -f
OBJS        = devtree.o  lsusb.o  lsusb-t.o  names.o  usbmisc.o
LIBUSB		= libusb
LIBUSB_V	= 0.1.12
LIB_USB		= ../$(LIBUSB)-$(LIBUSB_V)

CFLAGS += -Os
CFLAGS += $(EXTRA_CFLAGS)
CFLAGS += -I $(LIB_USB)
CFLAGS += -DDATADIR=$(shell pwd)

all: $(OBJS)
	make -C $(LIB_USB)
	$(CC) $(OBJS) -L$(LIB_USB) -lusb -o $(PROG)

romfs:
	$(ROMFSINST) /bin/$(PROG)

clean:
	$(RM) -f $(EXEC) *.elf *.o *.a
	$(RM) $(PROG)
