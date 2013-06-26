CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
RANLIB = $(CROSS_COMPILE)ranlib
STRIP = $(CROSS_COMPILE)strip
SSTRIP = $(CROSS_COMPILE)sstrip

CFLAGS += -Os
CFLAGS += $(EXTRA_CFLAGS)

romfs:
	$(CC) -c -fPIC error.c descriptors.c linux.c usb.c
	$(CC) -shared -Wl,-soname,libusb.so.0 -o libusb.so.0.1.12 error.o descriptors.o linux.o usb.o
	mkdir -p ../../romfs/usr/lib/
	ln -sf libusb.so.0.1.12 ./libusb.so.0
	ln -sf libusb.so.0 ./libusb.so
	cp libusb.so.0.1.12 ../../romfs/usr/lib/
	ln -sf libusb.so.0.1.12 ../../romfs/usr/lib/libusb.so.0
	ln -sf libusb.so.0 ../../romfs/usr/lib/libusb.so

clean:
	-rm -f $(EXEC) *.elf *.o *.a libusb.so*
