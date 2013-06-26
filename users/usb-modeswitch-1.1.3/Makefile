CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
RANLIB = $(CROSS_COMPILE)ranlib
STRIP = $(CROSS_COMPILE)strip
SSTRIP = $(CROSS_COMPILE)sstrip

PROG        = usb_modeswitch
VERS        = 1.1.3
RM          = /bin/rm -f
OBJS        = usb_modeswitch.c
LIBUSB		= libusb
LIBUSB_V	= 0.1.12
LIB_USB		= ../$(LIBUSB)-$(LIBUSB_V)

CFLAGS += -Os
CFLAGS += $(EXTRA_CFLAGS)
CFLAGS += -I $(LIB_USB)

all: $(PROG).o
	make -C $(LIB_USB)
	$(CC) $(PROG).o -L$(LIB_USB) -lusb -o $(PROG)

romfs:
	$(ROMFSINST) /bin/$(PROG)

clean:
	$(RM) -f $(EXEC) *.elf *.o *.a
	$(RM) $(PROG)

